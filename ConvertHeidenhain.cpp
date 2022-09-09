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
#include "SearchAlgorithms.h"


using namespace std;

/// <summary>
/// Leerer Konstruktor, wird nur in der NW.CL ConvertDlg.cpp Klasse genutzt um auf die Methoden deser Klasse
/// zuzugreifen
/// </summary>
/// 
ConvertHeidenhain::ConvertHeidenhain() {}


/// <summary>
/// @startConverting wird als Verzweigungsmethode genutzt. Je nach Verzweigung gelang der eingelesene String in eine andere Methode in
/// der dieser dann übersetzt wird.
/// </summary>
/// @param [fileContent] der Übergebene CStringArray der alle Zeilen der .tap File beinhaltet.
/// @param [labelIndex] enthält den Array index ab dem die LBLs anfangen
/// @param [filePath] enthält den Dateipfad der geöffneten Datei
void ConvertHeidenhain::startConverting(CStringArray& fileContent,int &labelIndex  ,CString &filePath)
{
	label_index = labelIndex;
	path = filePath;
	readConfigFile(path);
	CString tool_repositoryName=_T("tool_repository.cl");
	CString creoConfiName = _T("creo2mw.ini");
	CString tool_repositoryPath = ConversionAlgorithms::findSubFilesPath(tool_repositoryName,path);
	CString creoConfiPath = ConversionAlgorithms::findSubFilesPath(creoConfiName,path);
	CString indexString;
	CString testString;
	openSubFiles(_T("C:\\ESCmachine\\mwMachineSimulator_SimulationFiles\\tool_repository.cl"), tool_repositoryContent);
	openSubFiles(_T("C:\\ESCmachine\\mwMachineSimulator_SimulationFiles\\creo2mw.ini"), creoConfiContent);
	file.Copy(fileContent);
	bool foundOpCycle = false;
	int indexOfFirstToolCall = initialComment();
	
	for (int i = indexOfFirstToolCall; i < fileContent.GetSize(); i++) {

		if (fileContent.GetAt(i).Find(_T("M129")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(fileContent.GetAt(i), mw_other_line));
			foundRTCPOFF = true;
		}
		else if (fileContent.GetAt(i).Find(_T("M05")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(fileContent.GetAt(i),mw_other_line));
			moveLines.Add(configFile.getToolChangePoint_z());
			moveLines.Add(configFile.getToolChangePoint_xy());
		}
		else if (fileContent.GetAt(i).Find(_T("PGM ENDE")) != -1) {
			moveLines.Add(_T("MW_MACHMOVE RAPID  TIME.1 MOVE=91# END PGM planen MM"));
			break;
		}
		else if (fileContent.GetAt(i).Find(_T("M127")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(fileContent.GetAt(i), ';',mw_other_line));
		}
		else if (fileContent.GetAt(i).Find(_T("FN")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(fileContent.GetAt(i),mw_other_line));
			feedRate = SearchAlgorithms::findFeedRate(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T(";")) != -1) {
			//ignore line
		}
		else if (fileContent.GetAt(i).Find(_T("L X")) != -1 || fileContent.GetAt(i).Find(_T("L Y")) != -1 || fileContent.GetAt(i).Find(_T("L Z")) != -1||
			fileContent.GetAt(i).Find(_T("L  X")) != -1 || fileContent.GetAt(i).Find(_T("L  Y")) != -1 || fileContent.GetAt(i).Find(_T("L  Z")) != -1) {
			filterMovement(fileContent.GetAt(i),i,false);
		}
		else if (fileContent.GetAt(i).Find(_T("L A")) != -1|| fileContent.GetAt(i).Find(_T("L  A")) != -1) {
			fillacCoordinates(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T("* -")) != -1 && fileContent.GetAt(i).GetAt(fileContent.GetAt(i).GetLength()-1) =='-') {
			
			sequenceWithoutToolChange(fileContent.GetAt(i));
			if (SearchAlgorithms::searchForToolChange(i,file) == false) {
				CString testString = fileContent.GetAt(i);
				op_number_index++;
				indexString.Format(_T("%d"), op_number_index);
				mw_op_number_list.Add(mw_op_number + _T(" ") + indexString);
				indexString = _T("");
				startMachineCycle(_T("try catch"));
				i++;
			}
		}
		else if (fileContent.GetAt(i).Find(_T("* -")) != -1 && fileContent.GetAt(i).GetAt(fileContent.GetAt(i).GetLength() - 1) != '-') {
			//comment ignore line
		}
		else if (fileContent.GetAt(i).Find(_T("TOOL CALL")) != -1) {
			CString testString = fileContent.GetAt(i);
			findToolCall(fileContent.GetAt(i));
			moveLines.Add(SearchAlgorithms::findOtherLine(fileContent.GetAt(i),mw_other_line));
			op_number_index++;
			indexString.Format(_T("%d"), op_number_index);
			mw_op_number_list.Add(mw_op_number + _T(" ") + indexString);
			indexString = _T("");
			if (foundOpCycle == true) {
				startMachineCycle(_T("hello"));
			}
			foundOpCycle = true;

		}
		else if (fileContent.GetAt(i).Find(_T("CC")) != -1) {
			findCircle(fileContent.GetAt(i), fileContent.GetAt(i+1));
			i++;
		}
		else if (fileContent.GetAt(i).Find(_T("CALL LBL")) != -1) {
			jumpToLabel(fileContent.GetAt(i));
		}
		else{
			moveLines.Add(SearchAlgorithms::findOtherLine(fileContent.GetAt(i), mw_other_line));
		}
	}
	
	startMachineCycle(indexString);
	convertedFileContent.Add(_T("MW_STOP"));
}

/// <summary>
/// 
/// </summary>
/// <param name="indexString"></param>
void ConvertHeidenhain::startMachineCycle(CString indexString) {

	if (indexString.Find(_T("try catch")) == -1) {
		convertedFileContent.Add(mw_op_start);
		convertedFileContent.Add(mw_op_number_list.GetAt(mw_list_counter));
		convertedFileContent.Add(mw_op_comment.GetAt(sequenceCounter));
		convertedFileContent.Add(configFile.getHeadadapter());
		convertedFileContent.Add(configFile.getPostconfig());
		convertedFileContent.Add(configFile.getOutputname());
		convertedFileContent.Add(mw_tool_name_list.GetAt(toolListCounter));
		CString mapKey = ConversionAlgorithms::cutAtSpace(mw_tool_name_list.GetAt(toolListCounter), 3);
		convertedFileContent.Add(toolRepositoryMap.at(mapKey));
		convertedFileContent.Add(mw_transform);
		convertedFileContent.Add(mw_toolpath_transform);
		convertedFileContent.Add(configFile.getShortestpath());
		sequenceCounter++;
		toolListCounter++;

	}
	else {
		convertedFileContent.Add(mw_op_start);
		convertedFileContent.Add(mw_op_number_list.GetAt(mw_list_counter));
		convertedFileContent.Add(mw_op_comment.GetAt(sequenceCounter));
		convertedFileContent.Add(configFile.getHeadadapter());
		convertedFileContent.Add(configFile.getPostconfig());
		convertedFileContent.Add(configFile.getOutputname());
		convertedFileContent.Add(mw_tool_name_list.GetAt(toolListCounter));
		CString mapKey = ConversionAlgorithms::cutAtSpace(mw_tool_name_list.GetAt(toolListCounter), 3);
		convertedFileContent.Add(toolRepositoryMap.at(mapKey));
		convertedFileContent.Add(mw_transform);
		convertedFileContent.Add(mw_toolpath_transform);
		convertedFileContent.Add(configFile.getShortestpath());
		sequenceCounter++;
	}
	if (foundRTCPOFF == true) {
		convertedFileContent.Add(_T("MW_RTCP OFF"));
	}
	if (indexString.Find(_T("try catch")) == -1) {
		convertedFileContent.Add(configFile.getToolChangeTime());
	}

	convertedFileContent.Add(_T("MW_USE_PREVIOUS_OPERATION_AXES_AS_REFERENCE"));
	if (configFile.getMw_toolCall().GetLength() > 1) {
		convertedFileContent.Add(configFile.getMw_toolCall());
	}
	for (int i = 0; i < moveLines.GetSize() - 1; i++) {
		convertedFileContent.Add(moveLines.GetAt(i));
	}
	configFile.setMw_toolCall(moveLines.GetAt(moveLines.GetSize() - 1));
	moveLines.RemoveAll();
	convertedFileContent.Add(mw_op_end);
	mw_list_counter++;
}

void ConvertHeidenhain::sequenceWithoutToolChange(CString line) {
	CString sequenceNamewotc = _T("MW_OP_COMMENT ");
	CString cuttedLine = ConversionAlgorithms::cutAtSpace(line, 3, ' ');
	CString sequenceName = _T("");
	
	sequenceName.AppendChar(_T('"'));
	sequenceName.Append(cuttedLine);
	sequenceName.AppendChar(_T('"'));

	sequenceNamewotc.Append(sequenceName);
	mw_op_comment.Add(sequenceNamewotc);
}

void ConvertHeidenhain::fillacCoordinates(CString line) {
	bool aIsFilled = false;
	bool cIsFilled = false;
	CString newA=_T("");
	CString newC=_T("");
	CString convertedLine = _T("MW_MACHMOVE RAPID");
	for (int i = 0; i < line.GetLength()-1; i++) {
		
		if (line.GetAt(i) == 'A'&& (line.GetAt(i+1) == '+'|| line.GetAt(i + 1) == '-') && aIsFilled == false) {
			newA = ConversionAlgorithms::fillCoordinates(line, 'A', i);
			coordinates.setA_coordinate(ConversionAlgorithms::addDecimalPlace(newA));
			aIsFilled = true;
		}
		if (line.GetAt(i) == 'C' && cIsFilled == false) {
			newC = ConversionAlgorithms::fillCoordinates(line, 'C', i);
			coordinates.setC_coordinate(ConversionAlgorithms::addDecimalPlace(newC));
			cIsFilled = true;
		}
	}

	/*coordinates.setA_coordinate(ConversionAlgorithms::addDecimalPlace(newA));
	coordinates.setC_coordinate(ConversionAlgorithms::addDecimalPlace(newC));
	*/
	convertedLine.Append(_T(" A") + coordinates.getA_coordinate());
	convertedLine.Append(_T(" "));
	convertedLine.Append(_T("C") + coordinates.getC_coordinate());
	convertedLine.Append(_T(" "));

	CString lineNr = ConversionAlgorithms::findLineNr(line);
	convertedLine.Append(_T(" TIME.1 MOVE="));
	convertedLine.Append(lineNr);
	line = ConversionAlgorithms::cutAtSpace(line, 1);
	moveLines.Add(convertedLine + _T(" #") + line);
}

/// <summary>
/// Die Methode ist für die "INITIAL" Operation des .cl files. Ähnlich wie startConverting geht es Zeile für Zeile durch das
/// ausgesuchte file und convertiert Zeile für Zeile bis der erste TOOL CALL eingelesen wird, dann wird die Methode beendet.
/// Der Bereich der Schleife geht von der ersten Zeile der ausgesuchten Datei bis zum ersten TOOL CALL.
/// </summary>
/// @returns indexOfFirstToolCall beschreibt den index mit dem die Methode startConverting arbeitet.
/// Der Sinn dahinter ist das startConverting nicht wieder von Anfang der Datei startet sondern vor dem
/// ersten TOOL CALL anfängt
int ConvertHeidenhain::initialComment() {
	Coordinates coordinates;
	int indexOfFirstToolCall = 0;
	convertedFileContent.Add(configFile.getVersion());
	convertedFileContent.Add(configFile.getUnit());
	convertedFileContent.Add(_T("<!-- || ====================================================================== || -->"));
	convertedFileContent.Add(_T("MW_OP_START"));
	convertedFileContent.Add(configFile.getInitial());
	convertedFileContent.Add(configFile.getOutputname());
	convertedFileContent.Add(configFile.getSafepoint() + _T("# INITIAL"));

	for (int i = 0; i < file.GetSize(); i++) {
		if (file.GetAt(i).Find(_T("PLANE RESET STAY")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i),mw_other_line));
		}
		if (file.GetAt(i).Find(_T("FUNCTION MODE MILL")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), mw_other_line));
		}
		if (file.GetAt(i).Find(_T("M129")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i),';', mw_other_line));
			foundRTCPOFF = true;
		}
		else if (file.GetAt(i).Find(_T("M127")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), ';', mw_other_line));
		}
		else if (file.GetAt(i).Find(_T("L A+")) != -1 || file.GetAt(i).Find(_T("L  A+")) != -1 || file.GetAt(i).Find(_T("L A-")) != -1 || file.GetAt(i).Find(_T("L  A-")) != -1) {
			fillacCoordinates(file.GetAt(i));
		}
		else if (file.GetAt(i).Find(_T("L X")) != -1 || file.GetAt(i).Find(_T("L Y")) != -1 || file.GetAt(i).Find(_T("L Z")) != -1 ||
			file.GetAt(i).Find(_T("L  X")) != -1 || file.GetAt(i).Find(_T("L  Y")) != -1 || file.GetAt(i).Find(_T("L  Z")) != -1) {
			filterMovement(file.GetAt(i), i, false);
		} 
		else if (file.GetAt(i).Find(_T("* -")) != -1 && file.GetAt(i).GetAt(file.GetAt(i).GetLength() - 1) == '-') {
			indexOfFirstToolCall = i;
			break;
		}
		else if (file.GetAt(i).Find(_T("FN")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i),mw_other_line));
			feedRate = SearchAlgorithms::findFeedRate(file.GetAt(i));
			
		}
		else if (file.GetAt(i).Find(_T("CC")) != -1) {
			findCircle(file.GetAt(i), file.GetAt(i + 1));
			i++;
		}
		else if (file.GetAt(i).Find(_T("CYCL DEF")) != -1) {
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), mw_other_line));
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i+1), mw_other_line));
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i+2), mw_other_line));
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i+3), mw_other_line));
			i += 3;
		}
		else if (file.GetAt(i).Find(_T(";")) != -1) {
			//nothing
		}
		else if (file.GetAt(i).Find(_T("TOOL CALL")) != -1) {
			indexOfFirstToolCall = i;
			break;
		}
		else if (file.GetAt(i).Find(_T("* -")) != -1 || file.GetAt(i).Find(_T("BLK FORM") || file.GetAt(i).Find(_T("BEGIN")) != -1)){
		//nothing
		}
		else {
			moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), mw_other_line));
		}
	}

	for (int i = 0; i < moveLines.GetSize(); i++) {
		convertedFileContent.Add(moveLines.GetAt(i));
	}
	convertedFileContent.Add(_T("MW_RTCP OFF"));
	convertedFileContent.Add(mw_op_end);
	moveLines.RemoveAll();
	return indexOfFirstToolCall;
}

/// <summary>
/// @findMovement filtert die Zeile nach den Veränderungen der X,Y und Z koordinaten aus und speichert diese in den membervariablen der Klasse ConvertHeidenhain.cpp
/// </summary>
/// @param[line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @param[index] für testzweck wird später rausgenommen
void ConvertHeidenhain::filterMovement(CString line, int index, bool isMachMove) {
	bool isM91 = false;
	bool xIsFilled = false;
	bool yIsFilled = false;
	bool zIsFilled = false;
	CString convertedLine=_T("");
	
	if (isMachMove == false) {
		if (line.Find(_T("FMAX")) != -1 && line.Find(_T("M91")) == -1) {
			foundFMAX = true;
			foundFQ = false;
			convertedLine = mw_relmove_rapid;
		}
		else if (line.Find(_T("FQ1")) != -1) {
			foundFMAX = false;
			foundFQ = true;
			convertedLine = mw_relmove_feed;
		}
		else if (foundFQ == true && foundFMAX == false) {
			convertedLine = mw_relmove_feed;
		}
		else if((line.Find(_T("FMAX")) != -1 && line.Find(_T("M91")) != -1)) {
			convertedLine = _T("MW_MACHMOVE RAPID  X+20.000 Y+400.000 Z+500.000 F10000");
			isM91 = true;
		}
		else {
			convertedLine = _T("MW_RELMOVE FEED ");
		}
	}
	
	if (isM91 == false) {
		for (int i = 0; i < line.GetLength(); i++) {
			//Refactor fillCoordinates
			if (line.GetAt(i) == 'X' && xIsFilled == false) {
				coordinates.setX_coordinate(ConversionAlgorithms::fillCoordinates(line, 'X', i));
				coordinates.setX_coordinate(ConversionAlgorithms::addDecimalPlace(coordinates.getX_coordinate()));
				xIsFilled = true;
			}
			if (line.GetAt(i) == 'Y'&& yIsFilled == false) {
				coordinates.setY_coordinate(ConversionAlgorithms::fillCoordinates(line, 'Y', i));
				coordinates.setY_coordinate(ConversionAlgorithms::addDecimalPlace(coordinates.getY_coordinate()));
				yIsFilled = true;
			}
			if (line.GetAt(i) == 'Z' && zIsFilled == false) {
				coordinates.setZ_coordinate(ConversionAlgorithms::fillCoordinates(line, 'Z', i));
				coordinates.setZ_coordinate(ConversionAlgorithms::addDecimalPlace(coordinates.getZ_coordinate()));
				zIsFilled = true;
				break;
			}
		}

		/*coordinates.setX_coordinate(ConversionAlgorithms::addDecimalPlace(coordinates.getX_coordinate()));
		coordinates.setY_coordinate(ConversionAlgorithms::addDecimalPlace(coordinates.getY_coordinate()));
		coordinates.setZ_coordinate(ConversionAlgorithms::addDecimalPlace(coordinates.getZ_coordinate()));
		*/
		convertedLine.Append(_T(" X") + coordinates.getX_coordinate());
		convertedLine.Append(_T(" "));
		convertedLine.Append(_T("Y") + coordinates.getY_coordinate());
		convertedLine.Append(_T(" "));
		convertedLine.Append(_T("Z") + coordinates.getZ_coordinate());
		convertedLine.Append(_T(" "));

		if (foundFMAX == true) {
			convertedLine.Append(spindle);
		}
		else if (foundFQ == true) {
			convertedLine.Append(feedRate);
		}
	}
		//convertedLine.Append(time_move);
		CString lineNr = ConversionAlgorithms::findLineNr(line);
		convertedLine.Append(_T(" MOVE="));
		convertedLine.Append(lineNr);
		line = ConversionAlgorithms::cutAtSpace(line, 1);
		if (line.Find(_T("M3")) != -1) {
			line.Append(_T(" # MW_MACHMOVE Z+500.000"));
		}
		moveLines.Add(convertedLine + _T(" #") + line);
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
		if (foundToolNameComment == true && line.GetAt(i) != '\"') {
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
	filterToolName(toolNameComment);
}

/// <summary>
/// @findToolName filtert den namen des Werkzeugstückes aus 
/// </summary>
/// @param [toolNameComment] 
void ConvertHeidenhain::filterToolName(CString toolNameComment) {
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
	mw_tool_name.Append(tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment)-2));
	mw_tool_name.Append(_T(" USE_NUMBER "));
	substring = tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) - 1).Right(tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) - 1).GetLength() -
		(tool_repositoryContent.GetAt(static_cast<INT_PTR>(indexToolNameComment) - 1).Find(_T(" ")) + 1));
	mw_tool_name.Append(substring);
	mw_tool_name_list.Add(mw_tool_name);
	mw_tool_comment_list.Add(mw_tool_comment);
	toolRepositoryMap.insert(pair<CString,CString>(substring, mw_tool_comment));
}

/// <summary>
/// @findCircle konvertiert ein CIRCLE Befehl um, dieser Befehl besteht aus zwei Zeilen
/// </summary>
/// @param [lineCC] die erste Zeile im MCD
/// @param [lineC] die zweite Zeile aus dem MCD
void ConvertHeidenhain::findCircle(CString lineCC, CString lineC) {
	bool xIsFilled = false;
	bool yIsFilled = false;
	CString newX = _T("");
	CString newY = _T("");
	for (int i = 0; i < lineCC.GetLength(); i++) {
		//Refactor fillCoordinate
		if (lineCC.GetAt(i) == 'X' && xIsFilled == false) {
			newX = ConversionAlgorithms::fillCoordinates(lineCC, 'X', i);
			coordinates.setX_coordinate(ConversionAlgorithms::addDecimalPlace(newX));
			coordinates.setCx(coordinates.getX_coordinate());
			xIsFilled = true;
		}
		if (lineCC.GetAt(i) == 'Y' && yIsFilled == false) {
			newY = ConversionAlgorithms::fillCoordinates(lineCC, 'Y', i);
			coordinates.setY_coordinate(ConversionAlgorithms::addDecimalPlace(newY));
			coordinates.setCy(coordinates.getY_coordinate());
			yIsFilled = true;
			break;
		}
	}
	/*
	coordinates.setX_coordinate(ConversionAlgorithms::addDecimalPlace(newX));
	coordinates.setY_coordinate(ConversionAlgorithms::addDecimalPlace(newY));
	*/

	double ccX = _wtof(coordinates.getX_coordinate());
	double ccY = _wtof(coordinates.getY_coordinate());
	//Zeile für C
	CString CClineX = coordinates.getX_coordinate();
	CString CClineY = coordinates.getY_coordinate();
	CString gotoLine = _T("");
	
	/*coordinates.setCx(newX);
	coordinates.setCy(newY);
	*/
	coordinates.setCz(coordinates.getZ_coordinate());
	
	newX = _T("");
	newY = _T("");
	yIsFilled = false;
	xIsFilled = false;
	for (int i = 0; i < lineC.GetLength(); i++) {
		//Refactor fillCoordinate
		if (lineC.GetAt(i) == 'X' && xIsFilled == false) {
			newX = ConversionAlgorithms::fillCoordinates(lineC, 'X', i);
			coordinates.setX_coordinate(ConversionAlgorithms::addDecimalPlace(newX));
			//coordinates.setCx(coordinates.getX_coordinate());
			xIsFilled = true;
		}
		if (lineC.GetAt(i) == 'Y' && yIsFilled == false) {
			newY = ConversionAlgorithms::fillCoordinates(lineC, 'Y', i);
			coordinates.setY_coordinate(ConversionAlgorithms::addDecimalPlace(newY));
			//coordinates.setCy(coordinates.getY_coordinate());
			yIsFilled = true;
			break;
		}
	}
	/*coordinates.setX_coordinate(ConversionAlgorithms::addDecimalPlace(newX));
	coordinates.setY_coordinate(ConversionAlgorithms::addDecimalPlace(newY));
	*/
	double cX = _wtof(coordinates.getX_coordinate());
	double cY = _wtof(coordinates.getY_coordinate());

	CString ClineX = coordinates.getX_coordinate();
	CString ClineY = coordinates.getX_coordinate();

	double result = sqrt(((cX - ccX) * (cX - ccX)) + ((cY - ccY) * (cY - ccY)));//?

	CString ni_nj_nk = _T("");
	CString rotationDirection;
	if (lineC.Find(_T("DR+")) != -1 || lineC.Find(_T("DR +")) != -1) {
		rotationDirection = _T("R+");
		ni_nj_nk = _T(" NI0. NJ0. NK1. ");
	}
	else if (lineC.Find(_T("DR-")) != -1 || lineC.Find(_T("DR -")) != -1) {
		rotationDirection = _T("R-");
		ni_nj_nk = _T(" NI0. NJ0. NK-1. ");
	}

	CString resultString;
	resultString.Format(_T("%f"), result);
	resultString = ConversionAlgorithms::addDecimalPlace(resultString);
	CString convertedLineOne;
	CString convertedLineTwo;
	CString lineNr;
	lineNr = ConversionAlgorithms::findLineNr(lineCC);
	lineCC = ConversionAlgorithms::cutAtSpace(lineCC, 1);
	convertedLineOne = _T("MW_RELMOVE FEED  X") + CClineX + _T(" Y") + CClineY + _T(" ") +feedRate +_T(" MOVE=")+lineNr+ _T("#")+lineCC;
	lineNr = ConversionAlgorithms::findLineNr(lineC);
	lineC = ConversionAlgorithms::cutAtSpace(lineC, 1);
	convertedLineTwo = _T("MW_RELARCMOVE FEED  X")+ coordinates.getX_coordinate() + _T(" Y") + coordinates.getY_coordinate() + _T(" Z") + coordinates.getZ_coordinate() + _T(" CX") + coordinates.getCx() + _T(" CY") + coordinates.getCy() + _T(" CZ") + coordinates.getCz() + _T(" ")
	 + ni_nj_nk + _T("A+0.") + feedRate + _T(" MOVE=") + lineNr + _T("#") + lineC;

	//moveLines.Add(convertedLineOne);
	moveLines.Add(convertedLineTwo);
}

/// <summary>
/// @jumpToLabel diese Methode wird genutzt um bei einem "CALL LBL" zu dem richtigen label zu springen
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
void ConvertHeidenhain::jumpToLabel(CString line) {
	moveLines.Add(SearchAlgorithms::findOtherLine(line, mw_other_line));
	CString labelName = ConversionAlgorithms::cutAtSpace(line,2);
	bool foundLabel = false;
	CString indexString;
	bool foundOpCycle = false;
	int size = file.GetSize();
	CString compareLabel;

	for (int i = label_index; i < size; i++) {
		if (file.GetAt(i).Find(labelName) != -1) {
			compareLabel = ConversionAlgorithms::cutAtSpace(file.GetAt(i),1);
			if (compareLabel.GetLength() == labelName.GetLength()) {
				foundLabel = true;
			}
		}
		if (foundLabel == true) {
			if (file.GetAt(i).Find(_T("L X")) != -1 || file.GetAt(i).Find(_T("L Y")) != -1 || file.GetAt(i).Find(_T("L Z")) != -1) {
				filterMovement(file.GetAt(i), i,false);
			}
			else if (file.GetAt(i).Find(_T("A+")) != -1 || file.GetAt(i).Find(_T("C+")) != -1 || file.GetAt(i).Find(_T("A-")) != -1 || file.GetAt(i).Find(_T("C-")) != -1) {
				moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), mw_other_line));
				fillacCoordinates(file.GetAt(i));
			}
			else if (file.GetAt(i).Find(_T("* -")) != -1 && file.GetAt(i).GetAt(file.GetAt(i).GetLength() - 1) == '-') {
				 sequenceWithoutToolChange(file.GetAt(i));
				//findSequenceName(file.GetAt(i));
			}
			else if (file.GetAt(i).Find(_T("TOOL CALL")) != -1) {
				//startMachineCycle(file.GetAt(i), foundOpCycle, indexString);
			}
			else if (file.GetAt(i).Find(_T("FN")) != -1) {
				CString testLine = ConversionAlgorithms::cutAtSpace(file.GetAt(i), 5);
				feedRate=SearchAlgorithms::findFeedRate(testLine);
			}
			else if (file.GetAt(i).Find(_T("CC")) != -1) {
				findCircle(file.GetAt(i), file.GetAt(i + 1));
				i++;
			}
			else if (file.GetAt(i).Find(_T("CYCL DEF")) != -1) {
				moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), mw_other_line));
				moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i + 1), mw_other_line));
				moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i + 2), mw_other_line));
				moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i + 3), mw_other_line));
				filterCycleDef(file.GetAt(i + 1), file.GetAt(i + 2), file.GetAt(i + 3));
				outputTransform(file.GetAt(i));
				findMatrix(file.GetAt(i + 4));
				i += 4;
			}
			else if (file.GetAt(i).Find(_T("LBL 0")) != -1) {
				moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), mw_other_line));
				break;
			}
			else if (file.GetAt(i).Find(_T("PGM ENDE")) != -1) {
				break;
			}
			else {
				moveLines.Add(SearchAlgorithms::findOtherLine(file.GetAt(i), mw_other_line));
			}
		}
	}
}

/// <summary>
/// Ausgabe für die Zeilen MW_TOOLPATH_TRANSFORM und MW_TRANSFORM
/// Bsp aus dem .cl
/// MW_TRANSFORM holder_transform (1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)
///	MW_TOOLPATH_TRANSFORM(1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 100., 0, 0, 0, 1)
/// </summary>
/// @param [line]
void ConvertHeidenhain::outputTransform(CString line) {
	mw_transform = _T("MW_TRANSFORM holder_transform (1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)");
	mw_toolpath_transform = _T("MW_TOOLPATH_TRANSFORM (") + transformation.getXx() + _T(".,") + transformation.getXy() + _T(".,") + transformation.getXz() + _T(".,") + transformation.getTx() + _T(".,") +
		transformation.getYx() + _T(".,") + transformation.getYy() + _T(".,") + transformation.getYz() + _T(".,") + transformation.getTy() + _T(".,") + transformation.getZx() + _T(".,") + transformation.getZy()
		+ _T(".,") + transformation.getZz() + _T(".,") + transformation.getTz() +_T(".,0,0,0,1)");
	//convertedFileContent.Add(mw_transform);
	//convertedFileContent.Add(mw_toolpath_transform);
}

/// <summary>
/// 
/// </summary>
/// @param [line]
void ConvertHeidenhain::filterCycleDef(CString lineX,CString lineY,CString lineZ) {
	CString coordinateX = ConversionAlgorithms::cutAtSpace(lineX, 4);
	CString coordinateY = ConversionAlgorithms::cutAtSpace(lineY, 4);
	CString coordinateZ = ConversionAlgorithms::cutAtSpace(lineZ, 4);
	CString transPos = _T("");
	
	coordinates.setX_cycle(ConversionAlgorithms::cutCoordinateChar(coordinateX));
	coordinates.setY_cycle(ConversionAlgorithms::cutCoordinateChar(coordinateY));
	coordinates.setZ_cycle(ConversionAlgorithms::cutCoordinateChar(coordinateZ));

	bool foundTransform = false;
	
	for (int i = 0; i < creoConfiContent.GetSize(); i++) {
		if (foundTransform == true) {
			if (creoConfiContent.GetAt(i).GetAt(3) != '0') {
				transPos.AppendChar(creoConfiContent.GetAt(i).GetAt(0)); 
				transPos.AppendChar(creoConfiContent.GetAt(i).GetAt(1));
				transformation = Transformation::updateTrans(transPos, creoConfiContent.GetAt(i), transformation);
				transPos = _T("");
			}
		}
		if (creoConfiContent.GetAt(i).Find(_T("TRANS")) != -1) {
			foundTransform = true;
		}	
	}
	

}

/// <summary>
/// Liest die config.txt Datei und Speichert alle Informationen in die unten angezeigten Variablen ein.
/// </summary>
void ConvertHeidenhain::readConfigFile(CString path) {
	CString configPath = ConversionAlgorithms::findSubFilesPath(_T("config.txt"),path);
	openSubFiles(_T("C:\\ESCmachine\\mwMachineSimulator_Machines\\HERMLE_C32U_AC_cfg.txt"), configFile.configFileList);
	configFile.setVersion(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(0),1));
	configFile.setUnit(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(1), 1));
	configFile.setInitial(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(2), 1));
	configFile.setOutputname(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(3), 1));
	configFile.setSafepoint(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(4), 1));
	configFile.setHeadadapter(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(5), 1));
	configFile.setPostconfig(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(6), 1));
	configFile.setShortestpath(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(7), 1));
	configFile.setToolChangeTime(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(8), 1));
	configFile.setToolChangePoint_x(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(9), 1));
	configFile.setToolChangePoint_z(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(10), 1));
	configFile.setToolChangePoint_xy(ConversionAlgorithms::cutAtSpace(configFile.configFileList.GetAt(11), 1));
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
		catch (const std::out_of_range&)
		{
			m_LIST_MESSAGES.InsertString(0, _T("No file selected"));
		}
		catch (const std::invalid_argument&)
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

void ConvertHeidenhain::findMatrix(CString line) {

	CString cuttedLine = ConversionAlgorithms::cutAtSpace(line, 3);

	coordinates.setA_matrix(ConversionAlgorithms::fillMatrix(cuttedLine,coordinates.getA_matrix(), 'A'));
	coordinates.setC_matrix(ConversionAlgorithms::fillMatrix(cuttedLine, coordinates.getC_matrix(), 'C'));

	double ra = _wtof(coordinates.getA_matrix());
	double rc = _wtof(coordinates.getC_matrix());

	if (ra == 0 && rc == 0) {
		moveLines.Add(SearchAlgorithms::findOtherLine(line, mw_other_line));
	}
	else {
		moveLines.Add(ConversionAlgorithms::calculateMatrix(ra, 0.00, rc,transformation,coordinates));
		moveLines.Add(_T("MW_MACHMOVE C+180 SUBMOVE"));
	}
}
