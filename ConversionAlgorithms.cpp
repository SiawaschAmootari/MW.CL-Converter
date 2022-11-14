#include "ConversionAlgorithms.h"
#include "pch.h"
#include "ConfigFile.h"
#include "pch.h"
#include "Transformation.h"
#include "pch.h"
#include "Coordinates.h"
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

ConversionAlgorithms::ConversionAlgorithms() {};

/// <summary>
/// @findLabelName findet den namen des Labels raus, dieser wird benötigt damit das Programm bei einem CALL zum richtig Label springen kann
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @param [spaces] enthält die anzahl an leerzeichen welche die Zeile beinhalten darf bis der Labelname anfängt,
/// da die Methode sowohl für das finden des Namens im "CALL" als auch nach dem Programmende genutzt wird für einen Vergleich
/// haben wir verschiedene Zeilen und somit verschiedene Anzahlen von Leerzeichen die wir beachten müssen
/// @returns [labelName] ist das Ergebnis des Filterrungsprozess beinhaltet den Label namen
CString ConversionAlgorithms::cutAtSpace(CString line, int spaces)
{
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

	if (labelName.GetAt(0) == ' ') {
		CString label = _T("");
		for (int i = 1; i < labelName.GetLength(); i++) {
			label.AppendChar(labelName.GetAt(i));
		}
		return label;
	}
	return labelName;
}

/// <summary>
/// Schneidet den Eingegebenen String ab und returned das Ergebnis zurück.
/// </summary>
/// <param name="line"></param>
/// <param name="spaces"></param>
/// <param name="c"></param>
/// <returns></returns
CString ConversionAlgorithms::cutAtSpace(CString line, int spaces, char c)
{
	int spaceCount = 0;
	CString labelName = _T("");
	for (int i = 0; i < line.GetLength(); i++) {
		if (spaceCount >= spaces && line.GetAt(i) == ' ') {
			break;
		}
		if (spaceCount >= spaces) {
			labelName.AppendChar(line.GetAt(i));
		}
		if (line.GetAt(i) == ' ') {
			spaceCount++;
		}

	}


	if (labelName.GetAt(0) == ' ') {
		CString label = _T("");
		for (int i = 1; i < labelName.GetLength(); i++) {
			label.AppendChar(labelName.GetAt(i));
		}
		return label;
	}


	return labelName;
}

/// <summary>
/// @findSubFilesPath Schneided den Datei Pfad des .tap files aus, wird benötigt um die Subfiles automatisch zu öffnen.
/// </summary>
/// @param [fileName] der Dateiname der neuen Datei
/// @returns [newFilePath] Der neue Pfad für die SubFiles
CString ConversionAlgorithms::findSubFilesPath(CString fileName, CString path)
{
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

CString ConversionAlgorithms::cutCoordinateChar(CString coordinate)
{
	CString coordinateNumber = _T("");
	int counter;
	if (coordinate.GetAt(1) == ' ') {
		counter = 3;
	}
	else if (coordinate.GetAt(1) == '+' && coordinate.GetAt(0) != ' ') {
		counter = 2;
	}
	else { counter = 1; }

	for (int i = counter; i < coordinate.GetLength(); i++) {
		coordinateNumber.AppendChar(coordinate.GetAt(i));
	}

	return coordinateNumber;
}

CString ConversionAlgorithms::addTwoStrings(CString numberOne, CString numberTwo)
{
	double numberOneAsDouble = _wtof(numberOne);
	double numberTwoAsDouble = _wtof(numberTwo);
	double result = numberOneAsDouble + numberTwoAsDouble;
	CString resultAsString;
	resultAsString.Format(_T("%lf"), result);
	
	if (resultAsString.GetAt(0) != '-') {
		return _T("+") + resultAsString;
	}
	return resultAsString;
}

CString ConversionAlgorithms::substractTwoStrings(CString numberOne, CString numberTwo)
{
	double numberOneAsDouble = _wtof(numberOne);
	double numberTwoAsDouble = _wtof(numberTwo);
	double result = numberOneAsDouble - numberTwoAsDouble;
	CString resultAsString;
	resultAsString.Format(_T("%lf"), result);

	if (resultAsString.GetAt(0) != '-') {
		return _T("+") + resultAsString;
	}
	return resultAsString;
}

double ConversionAlgorithms::substractTwoStringsReturnDouble(CString numberOne, CString numberTwo)
{
	double numberOneAsDouble = _wtof(numberOne);
	double numberTwoAsDouble = _wtof(numberTwo);
	double result = numberOneAsDouble - numberTwoAsDouble;
	CString resultAsString;
	resultAsString.Format(_T("%lf"), result);

	
	return result;
}

CString ConversionAlgorithms::divideTwoStrings(CString numberOne, CString numberTwo)
{
	double numberOneAsDouble = _wtof(numberOne);
	double numberTwoAsDouble = _wtof(numberTwo);
	double result = numberOneAsDouble / numberTwoAsDouble;
	int check = int(result);
	
	if (result-check!=0) {
		result = check + 1;
	}
	CString resultAsString;
	resultAsString.Format(_T("%lf"), result);

	if (resultAsString.GetAt(0) != '-') {
		return _T("+") + resultAsString;
	}
	return resultAsString;
}

CString ConversionAlgorithms::calculateMatrix(double a, double b, double c,Transformation transformation, Coordinates coordinates)
{
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

	CStringArray matrixInString;
	int counter = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			CString str;
			str.Format(_T("%.10lf"), ((fs[(j + 1) % 3][(i + 1) % 3] * fs[(j + 2) % 3][(i + 2) % 3]) - (fs[(j + 1) % 3][(i + 2) % 3] * fs[(j + 2) % 3][(i + 1) % 3])) / determinant);
			matrixInString.Add(str);
		}
	}
	CString convertedLine = _T("MW_TOOLPATH_TRANSFORM (") + matrixInString.GetAt(0) + _T(",") + matrixInString.GetAt(1) + _T(",") + matrixInString.GetAt(2) + _T(",") + addTwoStrings(transformation.getTx(), coordinates.getX_cycle()) + _T(",") + matrixInString.GetAt(3) + _T(",") +
		matrixInString.GetAt(4) + _T(",") + matrixInString.GetAt(5) + _T(",") + addTwoStrings(transformation.getTy(), coordinates.getY_cycle()) + _T(",") + matrixInString.GetAt(6) + _T(",") + matrixInString.GetAt(7) + _T(",") + matrixInString.GetAt(8) + _T(",") + addTwoStrings(transformation.getTz(), coordinates.getZ_cycle()) + _T(".,0,0,0,1") + _T(")");

	//moveLines.Add(convertedLine);
	return convertedLine;
}

CString ConversionAlgorithms::fillMatrix(CString line, CString axis, char axisChar)
{
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
	return axis;
}

/// <summary>
/// @fillCoordinates die membervariablen für die koordinaten werden aktuallisiert.
/// Die Methode sucht im String nach dem gesuchten Zeichen und befüllt die Koordinatenvariable neu.
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
CString ConversionAlgorithms::fillCoordinates(CString line, char c, int index)
{
	CString g_coordinate = _T("");
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
	return g_coordinate;
}

/// <summary>
/// @addDecimalPlace falls der String keine Nachkommastelle enthält wird hier 
/// ein .0 am ende des Strings hinzugefügt
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
CString ConversionAlgorithms::addDecimalPlace(CString line)
{
	if (line.Find(_T(".")) == -1) {
		line.Append(_T(".000"));
	}
	return line;
}

/// <summary>
/// @findLineNr filtert die Zeilenummer der Zeile aus und return diese Zurück.
/// </summary>
/// @param [line] enthält die übergebene Zeile der .tap Datei welche im fileContent Array gespeichert sind
/// @returns [lineNr] enthält die gefilterte Zeilennummer
CString ConversionAlgorithms::findLineNr(CString line)
{
	CString lineNr = _T("");
	for (int i = 0; i < line.GetLength(); i++) {
		if (line.GetAt(i) == ' ') {
			break;
		}
		lineNr.AppendChar(line.GetAt(i));
	}
	return lineNr;
}
