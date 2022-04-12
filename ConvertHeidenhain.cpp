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

void ConvertHeidenhain::startConverting(CStringArray& fileContent,int &labelIndex  ,CString filePath)
{
	label_index = labelIndex;
	path = filePath;
	CString tool_repositoryName=_T("tool_repository.cl");
	CString creoConfiName = _T("creo2mw.ini");
	CString tool_repositoryPath = findSubFilesPath(tool_repositoryName);
	CString creoConfiPath = findSubFilesPath(creoConfiName);
	CString indexString;
	openSubFiles(tool_repositoryPath ,tool_repositoryContent);
	openSubFiles(creoConfiPath,creoConfiContent);
	file.Copy(fileContent);
	bool foundOpCycle = false;

	for (int i = 0; i < fileContent.GetSize(); i++) {
		if (fileContent.GetAt(i).Find(_T("L X")) != -1 || fileContent.GetAt(i).Find(_T("L Y")) != -1 || fileContent.GetAt(i).Find(_T("L Z")) != -1) {
			findMovement(fileContent.GetAt(i),i);
		}
		else if (fileContent.GetAt(i).Find(_T("* -")) != -1 && fileContent.GetAt(i).GetAt(fileContent.GetAt(i).GetLength()-1) =='-') {
			findComment(fileContent.GetAt(i));
			
		}
		else if (fileContent.GetAt(i).Find(_T("TOOL CALL")) != -1) {
			
			startMachineCycle(fileContent.GetAt(i),foundOpCycle,indexString);
		
		}
		else if (fileContent.GetAt(i).Find(_T("FN")) != -1) {
			findFeedRate(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T("CC")) != -1) {
			findCircle(fileContent.GetAt(i), fileContent.GetAt(i+1));
			i++;
		}
		else if (fileContent.GetAt(i).Find(_T("CALL LBL")) != -1) {
			jumpToLabel(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T("PGM ENDE")) != -1) {
			break;
		}else{
			findOtherLine(fileContent.GetAt(i));
		}
		//textFilter(fileContent.GetAt(i), i);
	}

	for (int i = 0; i < moveLines.GetSize(); i++) {
		convertedFileContent.Add(moveLines.GetAt(i));
	}
	convertedFileContent.Add(mw_op_end);
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
	moveLines.Add(convertedLine+_T(" #")+line);


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
	feedRate = _T("F");
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
	spindle = _T("F");
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

	findToolName(toolNameComment);
	//convertedFileContent.Add(mw_tool_name);
	//convertedFileContent.Add(mw_tool_comment);

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
			CStringArray firstHundredLines;
			/*for (int i = 0; i < subFileContent.GetSize(); i++) {
				firstHundredLines.Add(subFileContent.GetAt(i));

			}
			theApp.ArrToVal(firstHundredLines, sFilecontent);
			m_EDIT_FILE_OUTPUT.SetWindowText(sFilecontent);*/
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

CString ConvertHeidenhain::findSubFilesPath(CString fileName) {	
	CString newFilePath = path;
	int index=0;
	for (int i = newFilePath.GetLength() - 1; i > 0; i--) {
		if (newFilePath.GetAt(i) == '\\') {
			break;
		}		//Pfad des Hauptprogramm wird genutzt um den Pfad des Subprogramms zu erstellen
		newFilePath.Delete(i, 1);
	}
	//newFilePath.Delete(index, 1);
	newFilePath.Append(fileName);
	return newFilePath;
}

void ConvertHeidenhain::findToolName(CString toolNameComment) {
	mw_tool_name = _T("");
	mw_tool_comment = _T("");
	CString substring;
	int indexToolNameComment=0;
	for (int i = 0; i < tool_repositoryContent.GetSize(); i++) {
		if (tool_repositoryContent.GetAt(i).Find(toolNameComment)!=-1) {
			indexToolNameComment = i;
			break;
		}
	}
	mw_tool_comment.Append(tool_repositoryContent.GetAt(indexToolNameComment + 1));

	mw_tool_name.Append(tool_repositoryContent.GetAt(indexToolNameComment - 2));
	mw_tool_name.Append(_T(" USE_NUMBER "));
	substring = tool_repositoryContent.GetAt(indexToolNameComment - 1).Right(tool_repositoryContent.GetAt(indexToolNameComment - 1).GetLength() - 
																		(tool_repositoryContent.GetAt(indexToolNameComment - 1).Find(_T(" "))+1));
	mw_tool_name.Append(substring);
}

void ConvertHeidenhain::findComment(CString line) {
	mw_op_comment = _T("MW_OP_COMMENT ");
	bool foundComment = false;
	for (int i = 0; i < line.GetLength(); i++) {
		if (line.GetAt(i)=='-'  && foundComment == false) {
			foundComment = true;
			mw_op_comment.AppendChar('\"');
			i++;
		}
		if (foundComment == true) {
			mw_op_comment.AppendChar(line.GetAt(i));
		}
		if (foundComment == true && line.GetAt(i) == '-'|| i == line.GetAllocLength()-1) {
			mw_op_comment.AppendChar('\"');
			break;
		}
	}
}

void ConvertHeidenhain::startMachineCycle(CString line,bool &foundOpCycle,CString indexString) {
	findToolCall(line);
	if (foundOpCycle == true) {
		for (int i = 0; i < moveLines.GetSize(); i++) {
			convertedFileContent.Add(moveLines.GetAt(i));
		}
		moveLines.RemoveAll();
		convertedFileContent.Add(mw_op_end);
	}
	convertedFileContent.Add(mw_op_start);
	op_number_index++;
	indexString.Format(_T("%d"), op_number_index);
	convertedFileContent.Add(mw_op_number + _T(" ") + indexString);
	convertedFileContent.Add(mw_op_comment);
	convertedFileContent.Add(mw_tool_name);
	convertedFileContent.Add(mw_tool_comment);
	foundOpCycle = true;
}

void ConvertHeidenhain::findCircle(CString lineCC, CString lineC) {
	for (int i = 0; i < lineCC.GetLength(); i++) {
		//Refactor fillCoordinate
		fillCoordinates(lineCC, 'X', i, x_coordinate);
		fillCoordinates(lineCC, 'Y', i, y_coordinate);
	}

	addDecimalPlace(x_coordinate);
	addDecimalPlace(y_coordinate);

	double ccX = _wtof(x_coordinate);
	double ccY = _wtof(y_coordinate);
	//Zeile für C
	CString CClineX = x_coordinate;
	CString CClineY = y_coordinate;
	CString gotoLine = _T("");

	for (int i = 0; i < lineC.GetLength(); i++) {
		//Refactor fillCoordinate
		fillCoordinates(lineC, 'X', i, x_coordinate);
		fillCoordinates(lineC, 'Y', i, y_coordinate);
	}

	addDecimalPlace(x_coordinate);
	addDecimalPlace(y_coordinate);

	double cX = _wtof(x_coordinate);
	double cY = _wtof(y_coordinate);
	CString ClineX = x_coordinate;
	CString ClineY = y_coordinate;
	double result = sqrt(((cX - ccX) * (cX - ccX)) + ((cY - ccY) * (cY - ccY)));//?

	CString rotationDirection;
	if (lineC.Find(_T("DR+")) != -1 || lineC.Find(_T("DR +")) != -1) {
		rotationDirection = _T("R+");
	}
	else if (lineC.Find(_T("DR-")) != -1 || lineC.Find(_T("DR -")) != -1) {
		rotationDirection = _T("R-");
	}

	CString resultString;
	resultString.Format(_T("%f"), result);
	addDecimalPlace(resultString);
	CString convertedLineOne;
	CString convertedLineTwo;
	CString lineNr;
	lineNr = findLineNr(lineCC);
	convertedLineOne = _T("MW_RELMOVE FEED  ") + CClineX + _T(" ") + CClineY + _T(" ") +feedRate +_T(" MOVE=")+lineNr+ _T("#")+lineCC;
	lineNr = findLineNr(lineC);
	convertedLineTwo = _T("MW_RELARCMOVE FEED  ")+ ClineX + _T(" ") + ClineY + _T(" ")+rotationDirection+resultString+_T(" NI0. NJ0. NK1. ")+feedRate+ _T(" MOVE=") + lineNr+_T("#") + lineC;

	moveLines.Add(convertedLineOne);
	moveLines.Add(convertedLineTwo);

}

void ConvertHeidenhain::findOtherLine(CString line) {
	CString convertedLine = _T("");
	convertedLine.Append(mw_other_line);
	CString lineNumber = findLineNr(line);
	convertedLine.Append(lineNumber);
	convertedLine.Append(_T("#"));
	convertedLine.Append(line);
	moveLines.Add(convertedLine);
}

void ConvertHeidenhain::jumpToLabel(CString line) {
	findOtherLine(line);
	int spaceCount = 0;
	CString labelName = _T("");
	for (int i = 0; i < line.GetLength(); i++) {
		if(line.Find(_T(" ")) != -1) {
			spaceCount++;
		}
		if (spaceCount >= 2) {
			labelName.AppendChar(line.GetAt(i));
		}
	}
	bool foundLabel = false;
	CString indexString;
	bool foundOpCycle = false;
	int size = file.GetSize();
	for (int i = label_index; i < size; i++) {
		
		if (file.GetAt(i).Find(labelName) != -1) {
			foundLabel = true;
		}

		if (foundLabel == true) {
			if (file.GetAt(i).Find(_T("L X")) != -1 || file.GetAt(i).Find(_T("L Y")) != -1 || file.GetAt(i).Find(_T("L Z")) != -1) {
				findMovement(file.GetAt(i), i);
			}
			else if (file.GetAt(i).Find(_T("* -")) != -1 && file.GetAt(i).GetAt(file.GetAt(i).GetLength() - 1) == '-') {
				findComment(file.GetAt(i));

			}
			else if (file.GetAt(i).Find(_T("TOOL CALL")) != -1) {

				startMachineCycle(file.GetAt(i), foundOpCycle, indexString);

			}
			else if (file.GetAt(i).Find(_T("FN")) != -1) {
				findFeedRate(file.GetAt(i));
			}
			else if (file.GetAt(i).Find(_T("CC")) != -1) {
				findCircle(file.GetAt(i), file.GetAt(i + 1));
				i++;
			}
			else if (file.GetAt(i).Find(_T("LBL 0")) != -1) {
				break;
			}
			else {
				findOtherLine(file.GetAt(i));
			}
		}
	}
}

/*void ConvertHeidenhain::textFilter(CString line, int& index) {
	bool foundOpCycle = false;
	CString indexString;
	if (line.Find(_T("L X")) != -1 || line.Find(_T("L Y")) != -1 || line.Find(_T("L Z")) != -1) {
		findMovement(line, index);
	}
	else if (line.Find(_T("* -")) != -1 && line.GetAt(line.GetLength() - 1) == '-') {
		findComment(line);

	}
	else if (line.Find(_T("TOOL CALL")) != -1) {

		startMachineCycle(line, foundOpCycle, indexString);

	}
	else if (line.Find(_T("FN")) != -1) {
		findFeedRate(line);
	}
	else if (line.Find(_T("CC")) != -1) {
		findCircle(line, fileContent.GetAt(index + 1));
		index++;
	}
	else if (line.Find(_T("PGM ENDE")) != -1) {
		
	}
	else {
		findOtherLine(line);
	}
}*/
