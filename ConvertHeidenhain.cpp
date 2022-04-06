#include "pch.h"
#include "framework.h"
#include "MW.CL Converter.h"
#include "MW.CL ConverterDlg.h"
#include "ConvertHeidenhain.h"
#include "afxdialogex.h"
#include <afxcoll.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <atlstr.h>


using namespace std;

ConvertHeidenhain::ConvertHeidenhain() {};

void ConvertHeidenhain::startConverting(CStringArray& fileContent)
{
	CString tool_repositoryName=_T("tool_repository.cl");
	CString creoConfiName = _T("creo2mw.ini");
	CString tool_repositoryPath = findSubFilesPath(tool_repositoryName);
	CString creoConfiPath = findSubFilesPath(creoConfiName);
	
	openSubFiles(tool_repositoryPath ,tool_repositoryContent);
	openSubFiles(creoConfiPath,creoConfiContent);

	for (int i = 0; i < fileContent.GetSize(); i++) {
		if (fileContent.GetAt(i).Find(_T("L X")) != -1 || fileContent.GetAt(i).Find(_T("L Y")) != -1 || fileContent.GetAt(i).Find(_T("L Z")) != -1) {
			findMovement(fileContent.GetAt(i),i);
		}
		else if (fileContent.GetAt(i).Find(_T("TOOL CALL")) != -1) {
			findToolCall(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T("FN")) != -1) {
			findFeedRate(fileContent.GetAt(i));
		}
	}
}

		
void ConvertHeidenhain::findMovement(CString line, int index) {
	
	CString convertedLine=_T("");
	
	if (line.Find(_T("FMAX")) != -1) {
		foundFMAX = true;
		foundFQ = false;
		convertedLine = mw_relmove_rapid;
	}
	else if (line.Find(_T("FQ1")) != -1) {
		foundFMAX = false;
		foundFQ = true;
		convertedLine = mw_relmove_feed;
	}
	else if(foundFQ == true && foundFMAX == false){
		convertedLine = mw_relmove_feed;
	}

	for (int i = 0; i < line.GetLength(); i++) {
		//Refactor fillCoordinates
		fillCoordinates(line, 'X', i, x_coordinate);
		fillCoordinates(line, 'Y', i, y_coordinate);
		fillCoordinates(line, 'Z', i, z_coordinate);
		//findFedRat(line, i, g_fedRat);
	}

	addDecimalPlace(x_coordinate);
	addDecimalPlace(y_coordinate);
	addDecimalPlace(z_coordinate);

	convertedLine.Append(_T(" X")+x_coordinate);
	convertedLine.Append(_T(" "));
	convertedLine.Append(_T("Y") + y_coordinate);
	convertedLine.Append(_T(" "));
	convertedLine.Append(_T("Z") + z_coordinate);
	convertedLine.Append(_T(" "));
	
	if (foundFMAX == true) {
		convertedLine.Append(spindle);
	}
	else if (foundFQ == true) {
		convertedLine.Append(feedRate);
	}

	convertedLine.Append(time_move);
	CString lineNr = findLineNr(line);
	convertedLine.Append(lineNr);
	convertedFileContent.Add(convertedLine+_T(" #")+line);
}

void ConvertHeidenhain::fillCoordinates(CString line, char c, int index, CString& g_coordinate) {

	if (line.GetAt(index) == c && (line.GetAt(index + 1) == '+' || line.GetAt(index + 1) == '-')) {
		g_coordinate = _T("");
		for (int j = index + 1; j < line.GetLength(); j++) {
			if (line.GetAt(j) != ' ') {
				g_coordinate.AppendChar(line.GetAt(j));
			}
			else {
				break;
			}
		}
		g_coordinate.Replace(',', '.');
	}
}

void ConvertHeidenhain::addDecimalPlace(CString& line) {
	if (line.Find(_T(".")) == -1) {
		line.Append(_T(".000"));
	}
}

CString ConvertHeidenhain::findLineNr(CString line) {
	CString lineNr=_T("");
	for (int i = 0; i < line.GetLength(); i++) {
		if (line.GetAt(i) == ' ') {
			break;
		}
		lineNr.AppendChar(line.GetAt(i));

	}
	return lineNr;
}

void ConvertHeidenhain::findFeedRate(CString line) {
	bool foundFeedRate = false;
	for (int i = 0; i < line.GetAt(i); i++) {
		if (line.GetAt(i) == ';') {
			break;
		}
		if (foundFeedRate == true) {
			feedRate.AppendChar(line.GetAt(i));
		}
		if (line.GetAt(i) == '=') {
			i++;
			foundFeedRate = true;
		}
	}
}

void ConvertHeidenhain::findToolCall(CString line) {
	CString toolName = _T("");
	CString toolNameComment = _T("");
	bool foundToolNameComment = false;
	bool foundSpindl = false;

	for (int i = 0; i < line.GetLength(); i++) {
		//TOOLNAMECOMMENT
		if (line.GetAt(i) == ' ' && foundToolNameComment==true) {
			foundToolNameComment = false;
		}
		if (line.GetAt(i) == '"' && foundToolNameComment==false) {
			foundToolNameComment = true;
		}
		if (foundToolNameComment == true) {
			toolNameComment.AppendChar(line.GetAt(i));
		}
		//SPINDLFEEDRATE
		if (line.GetAt(i) == ' ' && foundSpindl == true) {
			foundSpindl = false;
		}
		if (foundSpindl == true) {
			spindle.AppendChar(line.GetAt(i));
		}
		if (line.GetAt(i) == 'S' && foundSpindl == false) {
			foundSpindl = true;
		}
		
	}



}

void ConvertHeidenhain::openSubFiles(CString path, CStringArray& subFileContent) {
	
	CStdioFile csfFile;
	if (std::ifstream(path).good())
	{
		try
		{
			csfFile.Open(path, CStdioFile::modeRead);
			CString sLine = _T("");
			bool bRead;
			CString sFilecontent = _T("");

			while (true)
			{
				bRead = csfFile.ReadString(sLine);
				if (bRead == false)
				{
					break;
				}
				subFileContent.Add(sLine);
			}
			csfFile.Close();
		}
		catch (const std::out_of_range& )
		{
			m_LIST_MESSAGES.InsertString(0, _T("No file selected"));
		}
		catch (const std::invalid_argument& )
		{
			m_LIST_MESSAGES.InsertString(0, _T("Invalid file"));
		}
	}
	else
	{
		m_LIST_MESSAGES.InsertString(0, _T("Error: filepath is wrong"));
		m_LIST_MESSAGES.InsertString(0, path);
	}
}
/// <summary>
/// ////////////////<--------------------------Fehler finden ---------------------------------->
/// File pfad korrekt ausfiltern
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
CString ConvertHeidenhain::findSubFilesPath(CString fileName) {	
	CString newFilePath = _T("");
	int index=0;
	for (int i = newFilePath.GetLength() - 1; i > 0; i--) {
		if (newFilePath.GetAt(i) == '\\') {
			index = i;
			break;
		}
		//newFilePath.Delete(i, 1);		//Pfad des Hauptprogramm wird genutzt um den Pfad des Subprogramms zu erstellen
	}
	newFilePath.Delete(index, 1);
	newFilePath.Append(_T("..\\")+fileName);
	return newFilePath;
}

