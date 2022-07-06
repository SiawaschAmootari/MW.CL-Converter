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
void ConvertHeidenhain::startConverting(CStringArray& fileContent, int& labelIndex, CString filePath)
{
	readConfigFile();
	label_index = labelIndex;
	path = filePath;
	CString tool_repositoryName = _T("tool_repository.cl");
	CString creoConfiName = _T("creo2mw.ini");
	CString tool_repositoryPath = findSubFilesPath(tool_repositoryName);
	CString creoConfiPath = findSubFilesPath(creoConfiName);
	CString indexString;

	openSubFiles(tool_repositoryPath, tool_repositoryContent);
	openSubFiles(creoConfiPath, creoConfiContent);
	file.Copy(fileContent);
	bool foundOpCycle = false;

	int indexOfFirstToolCall = initialComment();

	for (int i = indexOfFirstToolCall; i < fileContent.GetSize(); i++) {

		if (fileContent.GetAt(i).Find(_T("M129")) != -1) {
			findOtherLine(fileContent.GetAt(i));
			foundRTCPOFF = true;
		}
		else if (fileContent.GetAt(i).Find(_T("M05")) != -1) {
			findOtherLine(fileContent.GetAt(i));
			moveLines.Add(toolChangePoint_z);
			moveLines.Add(toolChangePoint_xy);
		}
		else if (fileContent.GetAt(i).Find(_T("PGM ENDE")) != -1) {
			moveLines.Add(_T("MW_MACHMOVE RAPID  TIME.1 MOVE=91# END PGM planen MM"));
			break;
		}
		/*else if (fileContent.GetAt(i).Find(_T("NP 1 -->")) != -1) {
			findMatrix(fileContent.GetAt(i));
		}*/
		else if (fileContent.GetAt(i).Find(_T("M127")) != -1) {
			findOtherLine(fileContent.GetAt(i), ';');
		}
		else if (fileContent.GetAt(i).Find(_T("FN")) != -1) {
			findOtherLine(fileContent.GetAt(i));
			findFeedRate(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T(";")) != -1) {
			//ignore line
		}
		else if (fileContent.GetAt(i).Find(_T("L X")) != -1 || fileContent.GetAt(i).Find(_T("L Y")) != -1 || fileContent.GetAt(i).Find(_T("L Z")) != -1 ||
			fileContent.GetAt(i).Find(_T("L  X")) != -1 || fileContent.GetAt(i).Find(_T("L  Y")) != -1 || fileContent.GetAt(i).Find(_T("L  Z")) != -1) {
			findMovement(fileContent.GetAt(i), i, false);
		}
		else if (fileContent.GetAt(i).Find(_T("L A")) != -1 || fileContent.GetAt(i).Find(_T("L  A")) != -1) {
			fillacCoordinates(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T("* -")) != -1 && fileContent.GetAt(i).GetAt(fileContent.GetAt(i).GetLength() - 1) == '-') {
			findSequenceName(fileContent.GetAt(i));
		}
		else if (fileContent.GetAt(i).Find(_T("* -")) != -1 && fileContent.GetAt(i).GetAt(fileContent.GetAt(i).GetLength() - 1) != '-') {
			//comment ignore line
		}
		else if (fileContent.GetAt(i).Find(_T("TOOL CALL")) != -1) {

			findToolCall(fileContent.GetAt(i));
			findOtherLine(fileContent.GetAt(i));
			op_number_index++;
			indexString.Format(_T("%d"), op_number_index);
			mw_op_number_list.Add(mw_op_number + _T(" ") + indexString);
			indexString = _T("");
			if (foundOpCycle == true) {
				startMachineCycle(fileContent.GetAt(i));
			}
			foundOpCycle = true;
		}
		else if (fileContent.GetAt(i).Find(_T("CC")) != -1) {
			findCircle(fileContent.GetAt(i), fileContent.GetAt(i + 1));
			i++;
		}
		else if (fileContent.GetAt(i).Find(_T("CALL LBL")) != -1) {
			jumpToLabel(fileContent.GetAt(i));
		}
		else {
			findOtherLine(fileContent.GetAt(i));
		}
	}

	startMachineCycle(indexString);
	convertedFileContent.Add(_T("MW_STOP"));
}

void ConvertHeidenhain::findMatrix(CString line) {

	CString cuttedLine = cutAtSpace(line, 3);

	fillMatrix(cuttedLine, a_matrix, 'A');
	fillMatrix(cuttedLine, c_matrix, 'C');

	double ra = _wtof(a_matrix);
	double rc = _wtof(c_matrix);

	if (ra == 0 && rc == 0) {
		findOtherLine(line);
	}
	else {
		calculateMatrix(ra, 0.00, rc);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConvertHeidenhain::fillMatrix(CString line, CString& axis, char axisChar) {
	axis = _T("");

	bool foundSpace = false;
	bool foundAxis = false;
	bool foundFloatingpoint = false;

	for (int i = 0; i < line.GetLength(); i++) {

		if (foundAxis == true) {
			if (line.GetAt(i) == ' ') {
				if (foundFloatingpoint == false) {
					axis.Append(_T(".0"));
				}
				break;
			}
			if (line.GetAt(i) == '.') {
				foundFloatingpoint = true;
			}
			axis.AppendChar(line.GetAt(i));
		}

		if (line.GetAt(i) == axisChar) {
			foundAxis = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConvertHeidenhain::calculateMatrix(double a, double b, double c) {
	double determinant = 0;

	a = a * -1;
	b = b * -1;
	c = c * -1;

	double degreeFactor = 0.01745329252;

	double ra[3][3];
	double rb[3][3];
	double rc[3][3];

	double fs[3][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
	double fr[3][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };

	ra[0][0] = 1;
	ra[0][1] = 0;
	ra[0][2] = 0;

	ra[1][0] = 0;
	ra[1][1] = cos(a * degreeFactor);
	ra[1][2] = sin(a * degreeFactor);

	ra[2][0] = 0;
	ra[2][1] = -sin(a * degreeFactor);
	ra[2][2] = cos(a * degreeFactor);

	////////////////////////////////////////////////////////////////////////////
	rb[0][0] = cos(b * degreeFactor);
	rb[0][1] = 0;
	rb[0][2] = -sin(b * degreeFactor);

	rb[1][0] = 0;
	rb[1][1] = 1;
	rb[1][2] = 0;

	rb[2][0] = sin(b * degreeFactor);
	rb[2][1] = 0;
	rb[2][2] = cos(b * degreeFactor);

	////////////////////////////////////////////////////////////////////////////
	rc[0][0] = cos(c * degreeFactor);
	rc[0][1] = sin(c * degreeFactor);
	rc[0][2] = 0;

	rc[1][0] = -sin(c * degreeFactor);
	rc[1][1] = cos(c * degreeFactor);
	rc[1][2] = 0;

	rc[2][0] = 0;
	rc[2][1] = 0;
	rc[2][2] = 1;

	////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			double sum = 0;
			for (int k = 0; k < 3; ++k) {
				sum += ra[k][i] * rc[j][k];
			}
			fs[i][j] = sum;
		}
	}

	cout << "matrix A" << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%.10lf |", ra[i][j]);
		}
		cout << endl;
	}
	cout << "matrix C" << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%.10lf |", rc[i][j]);
		}
		cout << endl;
	}
	cout << "Result" << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%.10lf |", fs[i][j]);
		}
		cout << endl;
	}

	cout << "inverse matrix" << endl;
	for (int i = 0; i < 3; i++)
		determinant = determinant + (fs[0][i] * (fs[1][(i + 1) % 3] * fs[2][(i + 2) % 3] - fs[1][(i + 2) % 3] * fs[2][(i + 1) % 3]));
	cout << "\n\ndeterminant: " << determinant;
	cout << "\n\nInverse of matrix is: \n";
	double test[9];
	int testCounter = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++)
			//printf("%.10lf |", ((fs[(j + 1) % 3][(i + 1) % 3] * fs[(j + 2) % 3][(i + 2) % 3]) - (fs[(j + 1) % 3][(i + 2) % 3] * fs[(j + 2) % 3][(i + 1) % 3])) / determinant);
			test[testCounter] = ((fs[(j + 1) % 3][(i + 1) % 3] * fs[(j + 2) % 3][(i + 2) % 3]) - (fs[(j + 1) % 3][(i + 2) % 3] * fs[(j + 2) % 3][(i + 1) % 3])) / determinant;
		testCounter++;
		cout << "\n";
	}
	cout << "\n";
	for (int i = 0; i < 9; i++) {
		CString str;
		str.Format(_T("%lf"), test[i]);
		cout << str;

	}
	//HERE -> </> <-

	CStringArray matrixInString;
	int counter = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			CString str;
			str.Format(_T("%lf"), ((fs[(j + 1) % 3][(i + 1) % 3] * fs[(j + 2) % 3][(i + 2) % 3]) - (fs[(j + 1) % 3][(i + 2) % 3] * fs[(j + 2) % 3][(i + 1) % 3])) / determinant);
			matrixInString.Add(str);
		}
	}

	CString convertedLine = _T("MW_TOOLPATH_TRANSFORM (") + matrixInString.GetAt(0) + _T(",") + matrixInString.GetAt(1) + _T(",") + matrixInString.GetAt(2) + _T(",") + matrixInString.GetAt(3) + _T(",") +
		matrixInString.GetAt(4) + _T(",") + matrixInString.GetAt(5) + _T(",") + matrixInString.GetAt(6) + _T(",") + matrixInString.GetAt(7) + _T(",") + matrixInString.GetAt(8) + _T(")");


	moveLines.Add(convertedLine);

}


void ConvertHeidenhain::fillacCoordinates(CString line) {
	CString convertedLine = _T("MW_MACHMOVE RAPID");
	for (int i = 0; i < line.GetLength(); i++) {
		fillCoordinates(line, 'A', i, a_coordinate);
		fillCoordinates(line, 'C', i, c_coordinate);
	}

	addDecimalPlace(a_coordinate);
	addDecimalPlace(c_coordinate);

	convertedLine.Append(_T(" A") + a_coordinate);
	convertedLine.Append(_T(" "));
	convertedLine.Append(_T("C") + c_coordinate);
	convertedLine.Append(_T(" "));

	CString lineNr = findLineNr(line);
	convertedLine.Append(_T(" TIME.1 MOVE="));
	convertedLine.Append(lineNr);
	line = cutAtSpace(line, 1);
	moveLines.Add(convertedLine + _T(" #") + line);
}

/// <summary>
/// @startMachineCycle fügt den Anfangskopf einer Machinenoperation hinzu, diese Methode wird im laufe der Entwicklung durch die untere Methode ersetzt!!!!
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @param [foundOpCycle] 
/// @param [indexString] 
/*void ConvertHeidenhain::startMachineCycle(CString line, bool& foundOpCycle, CString indexString) {

	convertedFileContent.Add(mw_op_start);
	convertedFileContent.Add(mw_op_number_list.GetAt(mw_list_counter));
	convertedFileContent.Add(mw_op_comment.GetAt(mw_list_counter));
	convertedFileContent.Add(headadapter);
	convertedFileContent.Add(postconfig);
	convertedFileContent.Add(outputname);
	//convertedFileContent.Add(safepoint);
	convertedFileContent.Add(mw_tool_name_list.GetAt(mw_list_counter));  //////////////////////////
	convertedFileContent.Add(mw_tool_comment);
	convertedFileContent.Add(mw_transform);
	convertedFileContent.Add(mw_toolpath_transform);
	convertedFileContent.Add(shortestpath);
	if (foundRTCPOFF == true) {
		convertedFileContent.Add(_T("MW_RTCP OFF"));
	}
	convertedFileContent.Add(toolChangeTime);
	convertedFileContent.Add(_T("MW_USE_PREVIOUS_OPERATION_AXES_AS_REFERENCE"));

	for (int i = 0; i < moveLines.GetSize(); i++) {
		convertedFileContent.Add(moveLines.GetAt(i));
	}
	moveLines.RemoveAll();
	convertedFileContent.Add(mw_op_end);
	mw_list_counter++;
}*/

/// <summary>
/// 
/// </summary>
/// <param name="indexString"></param>
void ConvertHeidenhain::startMachineCycle(CString indexString) {
	convertedFileContent.Add(mw_op_start);
	convertedFileContent.Add(mw_op_number_list.GetAt(mw_list_counter));
	convertedFileContent.Add(mw_op_comment.GetAt(mw_list_counter));
	convertedFileContent.Add(headadapter);
	convertedFileContent.Add(postconfig);
	convertedFileContent.Add(outputname);
	//convertedFileContent.Add(safepoint);
	convertedFileContent.Add(mw_tool_name_list.GetAt(mw_list_counter));  //////////////////////////
	convertedFileContent.Add(mw_tool_comment);
	convertedFileContent.Add(mw_transform);
	convertedFileContent.Add(mw_toolpath_transform);
	convertedFileContent.Add(shortestpath);

	if (foundRTCPOFF == true) {
		convertedFileContent.Add(_T("MW_RTCP OFF"));
	}
	convertedFileContent.Add(toolChangeTime);
	convertedFileContent.Add(_T("MW_USE_PREVIOUS_OPERATION_AXES_AS_REFERENCE"));

	for (int i = 0; i < moveLines.GetSize(); i++) {
		convertedFileContent.Add(moveLines.GetAt(i));
	}
	moveLines.RemoveAll();
	convertedFileContent.Add(mw_op_end);
	mw_list_counter++;
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
	int indexOfFirstToolCall = 0;
	convertedFileContent.Add(version);
	convertedFileContent.Add(unit);
	convertedFileContent.Add(_T("<!-- || ====================================================================== || -->"));
	convertedFileContent.Add(_T("MW_OP_START"));
	convertedFileContent.Add(initial);
	convertedFileContent.Add(outputname);
	convertedFileContent.Add(safepoint + _T("# INITIAL"));

	for (int i = 0; i < file.GetSize(); i++) {
		if (file.GetAt(i).Find(_T("PLANE RESET STAY")) != -1) {
			findOtherLine(file.GetAt(i));
		}
		if (file.GetAt(i).Find(_T("FUNCTION MODE MILL")) != -1) {
			findOtherLine(file.GetAt(i));
		}
		if (file.GetAt(i).Find(_T("M129")) != -1) {
			findOtherLine(file.GetAt(i), ';');
			foundRTCPOFF = true;
		}
		else if (file.GetAt(i).Find(_T("M127")) != -1) {
			findOtherLine(file.GetAt(i), ';');
		}
		else if (file.GetAt(i).Find(_T("L A+")) != -1 || file.GetAt(i).Find(_T("L  A+")) != -1 || file.GetAt(i).Find(_T("L A-")) != -1 || file.GetAt(i).Find(_T("L  A-")) != -1) {
			fillacCoordinates(file.GetAt(i));
		}
		else if (file.GetAt(i).Find(_T("L X")) != -1 || file.GetAt(i).Find(_T("L Y")) != -1 || file.GetAt(i).Find(_T("L Z")) != -1 ||
			file.GetAt(i).Find(_T("L  X")) != -1 || file.GetAt(i).Find(_T("L  Y")) != -1 || file.GetAt(i).Find(_T("L  Z")) != -1) {
			findMovement(file.GetAt(i), i, false);
		}
		else if (file.GetAt(i).Find(_T("* -")) != -1 && file.GetAt(i).GetAt(file.GetAt(i).GetLength() - 1) == '-') {
			//findOtherLine(file.GetAt(i));
			findSequenceName(file.GetAt(i));
		}
		else if (file.GetAt(i).Find(_T("FN")) != -1) {
			findOtherLine(file.GetAt(i));
			findFeedRate(file.GetAt(i));
		}
		else if (file.GetAt(i).Find(_T("CC")) != -1) {
			findCircle(file.GetAt(i), file.GetAt(i + 1));
			i++;
		}
		else if (file.GetAt(i).Find(_T("CYCL DEF")) != -1) {
			findOtherLine(file.GetAt(i));
			findOtherLine(file.GetAt(i + 1));
			findOtherLine(file.GetAt(i + 2));
			findOtherLine(file.GetAt(i + 3));
			i += 3;
		}
		else if (file.GetAt(i).Find(_T(";")) != -1) {
			//nothing
		}
		else if (file.GetAt(i).Find(_T("TOOL CALL")) != -1) {
			indexOfFirstToolCall = i;
			break;
		}
		else if (file.GetAt(i).Find(_T("* -")) != -1 || file.GetAt(i).Find(_T("BLK FORM") || file.GetAt(i).Find(_T("BEGIN")) != -1)) {
			//nothing
		}
		else {
			findOtherLine(file.GetAt(i));
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
void ConvertHeidenhain::findMovement(CString line, int index, bool isMachMove) {
	bool isM91 = false;
	CString convertedLine = _T("");

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
		else if ((line.Find(_T("FMAX")) != -1 && line.Find(_T("M91")) != -1)) {
			convertedLine = _T("MW_MACHMOVE RAPID  X+20.000 Y+400.000 Z+500.000 F10000");
			isM91 = true;
		}
	}

	if (isM91 == false) {
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

		convertedLine.Append(_T(" X") + x_coordinate);
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
	}
	//convertedLine.Append(time_move);
	CString lineNr = findLineNr(line);
	convertedLine.Append(_T(" MOVE="));
	convertedLine.Append(lineNr);
	line = cutAtSpace(line, 1);
	if (line.Find(_T("M3")) != -1) {
		line.Append(_T(" # MW_MACHMOVE Z+500.000"));
	}
	moveLines.Add(convertedLine + _T(" #") + line);
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

CString ConvertHeidenhain::addTwoStrings(CString numberOne, CString numberTwo) {


	double numberOneAsDouble = _wtof(numberOne);
	double numberTwoAsDouble = _wtof(numberTwo);
	double result = numberOneAsDouble + numberTwoAsDouble;
	CString resultAsString;
	resultAsString.Format(_T("%lf"), result);

	return resultAsString;
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
	CString lineNr = _T("");
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
		if (line.GetAt(i) == ' ' && foundToolNameComment == true) {
			foundToolNameComment = false;
		}
		if (line.GetAt(i) == '"' && foundToolNameComment == false) {
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
/// @findToolName filtert den namen des Werkzeugstückes aus 
/// </summary>
/// @param [toolNameComment] 
void ConvertHeidenhain::findToolName(CString toolNameComment) {
	mw_tool_name = _T("");
	mw_tool_comment = _T("");
	CString substring;
	int indexToolNameComment = 0;
	for (int i = 0; i < tool_repositoryContent.GetSize(); i++) {
		if (tool_repositoryContent.GetAt(i).Find(toolNameComment) != -1) {
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
	mw_tool_name_list.Add(mw_tool_name);
}

/// <summary>
/// @findComment markiert die Zeilen als Kommentare aus
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
void ConvertHeidenhain::findSequenceName(CString line) {
	CString mw_op_comment_string = _T("MW_OP_COMMENT ");
	bool foundComment = false;
	for (int i = 0; i < line.GetLength(); i++) {
		if (foundComment == true && line.GetAt(i) == ' ' || i == line.GetAllocLength() - 2) {
			mw_op_comment_string.AppendChar('\"');
			break;
		}
		if (line.GetAt(i) == '-' && foundComment == false) {
			foundComment = true;
			mw_op_comment_string.AppendChar('\"');
			i = i + 2;
		}
		if (foundComment == true) {
			mw_op_comment_string.AppendChar(line.GetAt(i));
		}
	}

	mw_op_comment.Add(mw_op_comment_string);
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
	convertedLineOne = _T("MW_RELMOVE FEED  X") + CClineX + _T(" Y") + CClineY + _T(" ") + feedRate + _T(" MOVE=") + lineNr + _T("#") + lineCC;
	lineNr = findLineNr(lineC);
	lineC = cutAtSpace(lineC, 1);
	convertedLineTwo = _T("MW_RELARCMOVE FEED  X") + ClineX + _T(" Y") + ClineY + _T(" ") + rotationDirection + resultString + _T(" NI0. NJ0. NK1. ") + feedRate + _T(" MOVE=") + lineNr + _T("#") + lineC;

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
	convertedLine.Append(_T("# "));
	line = cutAtSpace(line, 0);
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
	convertedLine.Append(lineNumber);
	convertedLine.Append(_T("# "));
	line = cutAtSpace(line, 1, c);
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
CString ConvertHeidenhain::cutAtSpace(CString line, int spaces) {
	int spaceCount = 0;
	CString labelName = _T("");
	for (int i = 0; i < line.GetLength(); i++) {
		if (spaceCount >= spaces) {
			labelName.AppendChar(line.GetAt(i));
		}
		if (line.GetAt(i) == ' ') {
			spaceCount++;
		}
	}
	return labelName;
}

/// <summary>
/// Schneidet den Eingegebenen String ab und returned das Ergebnis zurück.
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
	CString labelName = cutAtSpace(line, 2);
	bool foundLabel = false;
	CString indexString;
	bool foundOpCycle = false;
	int size = file.GetSize();
	CString compareLabel;

	for (int i = label_index; i < size; i++) {
		if (file.GetAt(i).Find(labelName) != -1) {
			compareLabel = cutAtSpace(file.GetAt(i), 1);
			if (compareLabel.GetLength() == labelName.GetLength()) {
				foundLabel = true;
			}
		}
		if (foundLabel == true) {
			if (file.GetAt(i).Find(_T("L X")) != -1 || file.GetAt(i).Find(_T("L Y")) != -1 || file.GetAt(i).Find(_T("L Z")) != -1) {
				findMovement(file.GetAt(i), i, false);
			}
			else if (file.GetAt(i).Find(_T("A+")) != -1 || file.GetAt(i).Find(_T("C+")) != -1 || file.GetAt(i).Find(_T("A-")) != -1 || file.GetAt(i).Find(_T("C-")) != -1) {
				fillacCoordinates(file.GetAt(i));
			}
			else if (file.GetAt(i).Find(_T("* -")) != -1 && file.GetAt(i).GetAt(file.GetAt(i).GetLength() - 1) == '-') {
				findSequenceName(file.GetAt(i));
			}
			else if (file.GetAt(i).Find(_T("TOOL CALL")) != -1) {
				//startMachineCycle(file.GetAt(i), foundOpCycle, indexString);
			}
			else if (file.GetAt(i).Find(_T("FN")) != -1) {
				CString testLine = cutAtSpace(file.GetAt(i), 5);
				findFeedRate(testLine);
			}
			else if (file.GetAt(i).Find(_T("CC")) != -1) {
				findCircle(file.GetAt(i), file.GetAt(i + 1));
				i++;
			}
			else if (file.GetAt(i).Find(_T("CYCL DEF")) != -1) {
				findOtherLine(file.GetAt(i));
				findOtherLine(file.GetAt(i + 1));
				findOtherLine(file.GetAt(i + 2));
				findOtherLine(file.GetAt(i + 3));
				findCycleDef(file.GetAt(i + 1), file.GetAt(i + 2), file.GetAt(i + 3));
				outputTransform(file.GetAt(i));
				findMatrix(file.GetAt(i + 4));
				////////////////////////////////////////////////////////////////////////////////////////////////
			   //Here                                                                                        //
			  ////////////////////////////////////////////////////////////////////////////////////////////////
				i += 4;
			}
			else if (file.GetAt(i).Find(_T("LBL 0")) != -1) {
				findOtherLine(file.GetAt(i));
				break;
			}
			else if (file.GetAt(i).Find(_T("PGM ENDE")) != -1) {
				break;
			}
			else {
				findOtherLine(file.GetAt(i));
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
	mw_toolpath_transform = _T("MW_TOOLPATH_TRANSFORM (") + xx + _T(".,") + xy + _T(".,") + xz + _T(".,") + tx + _T(".,") +
		yx + _T(".,") + yy + _T(".,") + yz + _T(".,") + ty + _T(".,") + zx + _T(".,") + zy + _T(".,") + zz + _T(".,") + tz + _T(".,0,0,0,1)");
	//convertedFileContent.Add(mw_transform);
	//convertedFileContent.Add(mw_toolpath_transform);
}

/// <summary>
/// 
/// </summary>
/// @param [line]
void ConvertHeidenhain::findCycleDef(CString lineX, CString lineY, CString lineZ) {
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
				updateTrans(transPos, creoConfiContent.GetAt(i));
				transPos = _T("");
			}
		}
		if (creoConfiContent.GetAt(i).Find(_T("TRANS")) != -1) {
			foundTransform = true;
		}
	}
}

/// <summary>
/// Befüllt String Variablen welche eine Zahl representieren.
/// Dies Können die X,Y und Z Koordinaten sein oder die Transformationspositionen
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

/// <summary>
/// Liest die config.txt Datei und Speichert alle Informationen in die unten angezeigten Variablen ein.
/// </summary>
void ConvertHeidenhain::readConfigFile() {
	CString configPath = findSubFilesPath(_T("config.txt"));
	openSubFiles(configPath, configFile);
	version = cutAtSpace(configFile.GetAt(0), 1);
	unit = cutAtSpace(configFile.GetAt(1), 1);
	initial = cutAtSpace(configFile.GetAt(2), 1);
	outputname = cutAtSpace(configFile.GetAt(3), 1);
	safepoint = cutAtSpace(configFile.GetAt(4), 1);
	headadapter = cutAtSpace(configFile.GetAt(5), 1);
	postconfig = cutAtSpace(configFile.GetAt(6), 1);
	shortestpath = cutAtSpace(configFile.GetAt(7), 1);
	toolChangeTime = cutAtSpace(configFile.GetAt(8), 1);
	toolChangePoint_x = cutAtSpace(configFile.GetAt(9), 1);
	toolChangePoint_z = cutAtSpace(configFile.GetAt(10), 1);
	toolChangePoint_xy = cutAtSpace(configFile.GetAt(11), 1);
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

/// <summary>
/// @findSubFilesPath Schneided den Datei Pfad des .tap files aus, wird benötigt um die Subfiles automatisch zu öffnen.
/// </summary>
/// @param [fileName] der Dateiname der neuen Datei
/// @returns [newFilePath] Der neue Pfad für die SubFiles
CString ConvertHeidenhain::findSubFilesPath(CString fileName) {
	CString newFilePath = path;
	int index = 0;
	for (int i = newFilePath.GetLength() - 1; i > 0; i--) {
		if (newFilePath.GetAt(i) == '\\') {
			break;
		}//Pfad des Hauptprogramm wird genutzt um den Pfad des Subprogramms zu erstellen
		newFilePath.Delete(i, 1);
	}
	//newFilePath.Delete(index, 1);
	newFilePath.Append(fileName);
	return newFilePath;
}

/// <summary>
/// Befüllt alle Variablen die für das toolpathtransform nötig sind.
/// Die Daten für die Befüllung stehen in der creo2mw.ini
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
		yz = _T("");
		yz = fillPosition(line);
	}
	else if (trans == _T("ty")) {
		ty = _T("");
		ty = fillPosition(line);
	}
	else if (trans == _T("zx")) {
		zx = _T("");
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