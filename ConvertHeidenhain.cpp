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
/// <summary>
/// Leerer Konstruktor, wird nur in der NW.CL ConvertDlg.cpp Klasse genutzt um auf die Methoden deser Klasse
/// zuzugreifen
/// </summary>
ConvertHeidenhain::ConvertHeidenhain() {};

/// <summary>
/// @startConverting wird als Verzweigungsmethode genutzt. Je nach Verzweigung gelang der eingelesene String in eine andere Methode in
/// der dieser dann übersetzt wird.
/// </summary>
/// @param [fileContent] der Übergebene CStringArray der alle Zeilen der .tap File beinhaltet.
/// @param [labelIndex] enthält den Array index ab dem die LBLs anfangen
/// @param [filePath] enthält den Dateipfad der geöffneten Datei
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
	readConfigFile();
	file.Copy(fileContent);
	bool foundOpCycle = false;

	int indexOfFirstToolCall = initialComment();
	
	for (int i = indexOfFirstToolCall; i < fileContent.GetSize(); i++) {
		if (fileContent.GetAt(i).Find(_T("M129")) != -1) {
			findOtherLine(fileContent.GetAt(i));
			foundRTCPOFF = true;
		}
		else if (fileContent.GetAt(i).Find(_T("M127")) != -1) {
			findOtherLine(fileContent.GetAt(i),';');
		}
		else if (fileContent.GetAt(i).Find(_T("L X")) != -1 || fileContent.GetAt(i).Find(_T("L Y")) != -1 || fileContent.GetAt(i).Find(_T("L Z")) != -1||
			fileContent.GetAt(i).Find(_T("L  X")) != -1 || fileContent.GetAt(i).Find(_T("L  Y")) != -1 || fileContent.GetAt(i).Find(_T("L  Z")) != -1) {
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
		else if (fileContent.GetAt(i).Find(_T("CYCL DEF")) != -1) {
			findCycleDef(fileContent.GetAt(i+1), fileContent.GetAt(i + 2), fileContent.GetAt(i + 3));
			outputTransform(fileContent.GetAt(i));
			i += 3;
		}else if (fileContent.GetAt(i).Find(_T(";")) != -1) {
			//nichts!
		}
		else if (fileContent.GetAt(i).Find(_T("PGM ENDE")) != -1) {
			break;
		}else{
			//findOtherLine(fileContent.GetAt(i));
		}
	}

	for (int i = 0; i < moveLines.GetSize(); i++) {
		convertedFileContent.Add(moveLines.GetAt(i));
	}
	convertedFileContent.Add(mw_op_end);
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
int ConvertHeidenhain::initialComment() {
	int indexOfFirstToolCall = 0;
	convertedFileContent.Add(_T("MW_CL_VERSION 1.2"));
	convertedFileContent.Add(_T("MW_UNITS_METRIC 1"));
	convertedFileContent.Add(_T("<!-- || ====================================================================== || -->"));
	convertedFileContent.Add(_T("MW_OP_START"));
	convertedFileContent.Add(_T("MW_OP_COMMENT \"INITIAL\""));
	convertedFileContent.Add(_T("MW_HEADERAXESNAME \"G-Code\""));

	for (int i = 0; i < file.GetSize(); i++) {
		if (file.GetAt(i).Find(_T("M129")) != -1) {
			findOtherLine(file.GetAt(i),';');
			foundRTCPOFF = true;
		}
		else if (file.GetAt(i).Find(_T("M127")) != -1) {
			findOtherLine(file.GetAt(i),';');
		}
		else if (file.GetAt(i).Find(_T("L X")) != -1 || file.GetAt(i).Find(_T("L Y")) != -1 || file.GetAt(i).Find(_T("L Z")) != -1 ||
			file.GetAt(i).Find(_T("L  X")) != -1 || file.GetAt(i).Find(_T("L  Y")) != -1 || file.GetAt(i).Find(_T("L  Z")) != -1) {
			findMovement(file.GetAt(i), i);
		}
		else if (file.GetAt(i).Find(_T("* -")) != -1 && file.GetAt(i).GetAt(file.GetAt(i).GetLength() - 1) == '-') {
			findComment(file.GetAt(i));

		}
		else if (file.GetAt(i).Find(_T("FN")) != -1) {
			findFeedRate(file.GetAt(i));
		}
		else if (file.GetAt(i).Find(_T("CC")) != -1) {
			findCircle(file.GetAt(i), file.GetAt(i + 1));
			i++;
		}
		else if (file.GetAt(i).Find(_T("CYCL DEF")) != -1) {
			//findCycleDef(file.GetAt(i + 1), file.GetAt(i + 2), file.GetAt(i + 3));
			//outputTransform(file.GetAt(i));
			findOtherLine(file.GetAt(i));
			findOtherLine(file.GetAt(i + 1));
			findOtherLine(file.GetAt(i + 2));
			findOtherLine(file.GetAt(i + 3));
			i += 3;
		}
		else if (file.GetAt(i).Find(_T(";")) != -1) {
			//nichts!
		}
		else if (file.GetAt(i).Find(_T("TOOL CALL")) != -1) {
			indexOfFirstToolCall = i;
			break;
		}		
	}

	for (int i = 0; i < moveLines.GetSize(); i++) {
		convertedFileContent.Add(moveLines.GetAt(i));
		CString test = moveLines.GetAt(i);
	}
	moveLines.RemoveAll();
	return indexOfFirstToolCall;
	//---------> here
}

/// <summary>
/// @findMovement filtert die Zeile nach den Veränderungen der X,Y und Z koordinaten aus und speichert diese in den membervariablen der Klasse ConvertHeidenhain.cpp
/// </summary>
/// @param[line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @param[index] für testzweck wird später rausgenommen
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
	line = cutAtSpace(line, 1);
	moveLines.Add(convertedLine+_T(" #")+line);
}

/// <summary>
/// @fillCoordinates die membervariablen für die koordinaten werden aktuallisiert.
/// Die Methode sucht im String nach dem gesuchten Zeichen und befüllt die Koordinatenvariable neu.
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
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

/// <summary>
/// @addDecimalPlace falls der String keine Nachkommastelle enthält wird hier 
/// ein .0 am ende des Strings hinzugefügt
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
void ConvertHeidenhain::addDecimalPlace(CString& line) {
	if (line.Find(_T(".")) == -1) {
		line.Append(_T(".000"));
	}
}

/// <summary>
/// @findLineNr filtert die Zeilenummer der Zeile aus und return diese Zurück.
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @returns [lineNr] enthält die gefilterte Zeilennummer
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

/// <summary>
/// @findFeedRate filtert den Wert für die Spindle aus diese wird dann in der Variable [feedRate] gespeichert 
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
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

/// <summary>
/// @findToolCall einzelne Information werden für den Machine Zyklus gefiltert, gesammelt und
/// in class Variablen abgespeichert
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind , dieser enthält den SubString "TOOL CALL"
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
}

/// <summary>
/// @openSubFiles öffnet die Nebenfiles welche für die vollständige Übersetzung benötigt werden
/// </summary>
/// @param [path] beinhaltet den Pfad des .tap Files welches zuvor über den "Open" Button geöffnet wurde
/// @param [subFileContent] ist die Adresse eines leerer CStringArrays in dem der Inhalt der ausgewählten Datei Zeilenweise eingespeichert werden
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
/// @findSubFilesPath Schneided den Datei Pfad des .tap files aus, wird benötigt um die Subfiles automatisch zu öffnen.
/// </summary>
/// @param [fileName] der Dateiname der neuen Datei
/// @returns [newFilePath] Der neue Pfad für die SubFiles
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

/// <summary>
/// @findToolName filtert den namen des Werkzeugstückes aus 
/// </summary>
/// @param [toolNameComment] 
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
	mw_tool_comment.Append(tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) + 1));
	mw_tool_name.Append(tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) - 2));
	mw_tool_name.Append(_T(" USE_NUMBER "));
	substring = tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) - 1).Right(tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) - 1).GetLength() -
		(tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) - 1).Find(_T(" ")) + 1));
	mw_tool_name.Append(substring);
}

/// <summary>
/// @findComment markiert die Zeilen als Kommentare aus
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
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

/// <summary>
/// @startMachineCycle fügt den Anfangskopf einer Machinenoperation hinzu
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @param [foundOpCycle] 
/// @param [indexString] 
void ConvertHeidenhain::startMachineCycle(CString line,bool &foundOpCycle,CString indexString) {
	findToolCall(line);
	if (foundOpCycle == true) {
		for (int i = 0; i < moveLines.GetSize(); i++) {
			convertedFileContent.Add(moveLines.GetAt(i));
		}
		moveLines.RemoveAll();
		convertedFileContent.Add(mw_op_end);
		convertedFileContent.Add(_T("============================================================================================="));
	}
	convertedFileContent.Add(mw_op_start);
	op_number_index++;
	indexString.Format(_T("%d"), op_number_index);
	convertedFileContent.Add(mw_op_number + _T(" ") + indexString);
	convertedFileContent.Add(mw_op_comment);
	convertedFileContent.Add(mw_tool_name);
	convertedFileContent.Add(mw_tool_comment);
	if (foundRTCPOFF == true) {
		convertedFileContent.Add(_T("MW_RTCP OFF"));
	}
	foundOpCycle = true;
}

/// <summary>
/// @findCircle konvertiert ein CIRCLE Befehl um, dieser Befehl besteht aus zwei Zeilen
/// </summary>
/// @param [lineCC] die erste Zeile im MCD
/// @param [lineC] die zweite Zeile aus dem MCD
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
	lineCC = cutAtSpace(lineCC, 1);
	convertedLineOne = _T("MW_RELMOVE FEED  ") + CClineX + _T(" ") + CClineY + _T(" ") +feedRate +_T(" MOVE=")+lineNr+ _T("#")+lineCC;
	lineNr = findLineNr(lineC);
	lineC = cutAtSpace(lineC, 1);
	convertedLineTwo = _T("MW_RELARCMOVE FEED  ")+ ClineX + _T(" ") + ClineY + _T(" ")+rotationDirection+resultString+_T(" NI0. NJ0. NK1. ")+feedRate+ _T(" MOVE=") + lineNr+_T("#") + lineC;

	moveLines.Add(convertedLineOne);
	moveLines.Add(convertedLineTwo);
}

/// <summary>
/// @findOtherLine wird aktuell genutzt für alle anderen Zeilen für die der Algorithmus noch nicht geschrieben wurde
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
void ConvertHeidenhain::findOtherLine(CString line) {
	CString convertedLine = _T("");
	convertedLine.Append(mw_other_line);
	CString lineNumber = findLineNr(line);
	line = cutAtSpace(line, 1);
	convertedLine.Append(lineNumber);
	convertedLine.Append(_T("#"));
	line = cutAtSpace(line, 1);
	convertedLine.Append(line);
	moveLines.Add(convertedLine);
}
/// <summary>
/// 
/// </summary>
/// <param name="line"></param>
/// <param name="c"></param>
void ConvertHeidenhain::findOtherLine(CString line, char c) {
	CString convertedLine = _T("");
	convertedLine.Append(mw_other_line);
	CString lineNumber = findLineNr(line);
	line = cutAtSpace(line, 1);
	convertedLine.Append(lineNumber);
	convertedLine.Append(_T("#"));
	line = cutAtSpace(line, 1,c);
	convertedLine.Append(line);
	moveLines.Add(convertedLine);
}

/// <summary>
/// @findLabelName findet den namen des Labels raus, dieser wird benötigt damit das Programm bei einem CALL zum richtig Label springen kann
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @param [spaces] enthält die anzahl an leerzeichen welche die Zeile beinhalten darf bis der Labelname anfängt,
/// da die Methode sowohl für das finden des Namens im "CALL" als auch nach dem Programmende genutzt wird für einen Vergleich
/// haben wir verschiedene Zeilen und somit verschiedene Anzahlen von Leerzeichen die wir beachten müssen
/// @returns [labelName] ist das Ergebnis des Filterrungsprozess beinhaltet den Label namen
CString ConvertHeidenhain::cutAtSpace(CString line,int spaces) {
	int spaceCount = 0;
	CString labelName = _T("");
	for (int i = 0; i < line.GetLength(); i++) {
		if (line.GetAt(i) == ' ') {
			spaceCount++;
		}
		if (spaceCount >= spaces) {
			labelName.AppendChar(line.GetAt(i));
		}
	}
	return labelName;
}
/// <summary>
/// 
/// </summary>
/// <param name="line"></param>
/// <param name="spaces"></param>
/// <param name="c"></param>
/// <returns></returns>
CString ConvertHeidenhain::cutAtSpace(CString line, int spaces, char c) {
	int spaceCount = 0;
	CString labelName = _T("");
	for (int i = 0; i < line.GetLength(); i++) {
		if (line.GetAt(i) == c) {
			break;
		}
		if (line.GetAt(i) == ' ') {
			spaceCount++;
		}
		if (spaceCount >= spaces) {
			labelName.AppendChar(line.GetAt(i));
		}
		
	}
	return labelName;
}

/// <summary>
/// @jumpToLabel diese Methode wird genutzt um bei einem "CALL LBL" zu dem richtigen label zu springen
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
void ConvertHeidenhain::jumpToLabel(CString line) {
	findOtherLine(line);
	CString labelName = cutAtSpace(line,2);
	bool foundLabel = false;
	CString indexString;
	bool foundOpCycle = false;
	int size = file.GetSize();
	CString compareLabel;
	for (int i = label_index; i < size; i++) {
		if (file.GetAt(i).Find(labelName) != -1) {
			compareLabel = cutAtSpace(file.GetAt(i),1);
			if (compareLabel.GetLength() == labelName.GetLength()) {
				foundLabel = true;
			}
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

/// <summary>
/// 
/// </summary>
/// @param [line]
void ConvertHeidenhain::outputTransform(CString line) {
	CString mw_transform = _T("MW_TRANSFORM holder_transform (1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)");
	mw_toolpath_transform = _T("MW_TOOLPATH_TRANSFORM (") + xx + _T(".,") + xy + _T(".,") + xz + _T(".,") + tx + _T(".,") + 
		yx + _T(",") + yy + _T(".,") + yz + _T(".,") + ty + _T(".,") + zx + _T(".,") + zy + _T(".,") + zz + _T(".,") + tz+_T(",0,0,0,1)");
	moveLines.Add(mw_transform);
	moveLines.Add(mw_toolpath_transform);
}

/// <summary>
/// 
/// </summary>
/// @param [line]
void ConvertHeidenhain::findCycleDef(CString lineX,CString lineY,CString lineZ) {
	CString coordinateX = cutAtSpace(lineX, 4);
	CString coordinateY = cutAtSpace(lineY, 4);
	CString coordinateZ = cutAtSpace(lineZ, 4);
	CString transPos = _T("");
	
	x_cycle = coordinateX;
	y_cycle = coordinateY;
	z_cycle = coordinateZ;
	
	bool foundTransform = false;
	for (int i = 0; i < creoConfiContent.GetSize(); i++) {
		if (foundTransform == true) {
			if (creoConfiContent.GetAt(i).GetAt(3) != '0') {
				transPos.AppendChar(creoConfiContent.GetAt(i).GetAt(0));
				transPos.AppendChar(creoConfiContent.GetAt(i).GetAt(1));
				updateTrans(transPos,creoConfiContent.GetAt(i));
				transPos = _T("");
			}
		}
		if (creoConfiContent.GetAt(i).Find(_T("TRANS")) != -1) {
			foundTransform = true;
		}
		
	}
}

/// <summary>
/// 
/// </summary>
/// @param [trans]
/// @param [line]
void ConvertHeidenhain::updateTrans(CString trans, CString line) {
	if (trans == _T("xx")) {
		xx = _T("");
		xx = fillPosition(line);
	}
	else if (trans == _T("xy")) {
		xy = _T("");
		xy = fillPosition(line);
	}
	else if (trans == _T("xz")) {
		xz = _T("");
		xz = fillPosition(line);
	}
	else if (trans == _T("tx")) {
		tx = _T("");
		tx = fillPosition(line);
	}
	else if (trans == _T("yx")) {
		yx = _T("");
		yx = fillPosition(line);
	}
	else if (trans == _T("yy")) {
		yy = _T("");
		yy = fillPosition(line);
	}
	else if (trans == _T("yz")) {
		yz= _T("");
		yz = fillPosition(line);
	}
	else if (trans == _T("ty")) {
		ty = _T("");
		ty = fillPosition(line);
	}
	else if (trans == _T("zx")) {
		zx= _T("");
		zx = fillPosition(line);
	}
	else if (trans == _T("zy")) {
		zy = _T("");
		zy = fillPosition(line);
	}
	else if (trans == _T("zz")) {
		zz = _T("");
		zz = fillPosition(line);
	}
	else if (trans == _T("tz")) {
		tz = _T("");
		tz = fillPosition(line);
	}
}

/// <summary>
/// 
/// </summary>
/// @param [line]
/// @returns[filling]
CString ConvertHeidenhain::fillPosition(CString line) {
	CString filling = _T("");
	for (int i = 3; i < line.GetLength(); i++) {
		filling.AppendChar(line.GetAt(i));
	}
	return filling;
}

void ConvertHeidenhain::readConfigFile() {
	CString configPath = findSubFilesPath(_T("config.txt"));
	openSubFiles(configPath, configFile);

}