#include "pch.h"
#include "UniversalDrill.h"
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
#include "ConvertHeidenhain.h"

UniversalDrill::UniversalDrill() {
	CString q200 = _T("");
	CString q201 = _T("");
	CString q206 = _T("");
	CString q202 = _T("");
	CString q210 = _T("");
	CString q203 = _T("");
	CString q204 = _T("");
	CString q212 = _T("");
	CString q213 = _T("");
	CString q205 = _T("");
	CString q211 = _T("");
	CString q208 = _T("");
	CString q256 = _T("");
}

void UniversalDrill::fillDrill(CString lineq00, CString lineq01, CString lineq06, CString lineq02, CString lineq10, CString lineq03, CString lineq04, CString lineq12, CString lineq13, CString lineq05, CString lineq11, CString lineq08, CString lineq56) {
	q200 = fillQValues(lineq00);
	q201 = fillQValues(lineq01);
	q206 = fillQValues(lineq06);
	q202 = fillQValues(lineq02);
	q210 = fillQValues(lineq10);
	q203 = fillQValues(lineq03);
	q204 = fillQValues(lineq04);
	q212 = fillQValues(lineq12);
	q213 = fillQValues(lineq13);
	q205 = fillQValues(lineq05);
	q211 = fillQValues(lineq11);
	q208 = fillQValues(lineq08);
	q256 = fillQValues(lineq56);
}

CString UniversalDrill::fillQValues(CString line) {

	CString value = _T("");
	bool foundEqualOperator = false;
	for (int i = 0; i < line.GetLength(); i++) {
		if (foundEqualOperator == true && line.GetAt(i) != ' ') {
			value.AppendChar(line.GetAt(i));
		}
		if (line.GetAt(i) == '=') {
			foundEqualOperator = true;

		}
	}
	return value;
}

int UniversalDrill::stringAsInt(CString line) {
	
	return 0;
}
