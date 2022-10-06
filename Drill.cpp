#include "pch.h"
#include "Drill.h"
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

Drill::Drill() {
	CString q200 = _T("");
	CString q201 = _T("");
	CString q202 = _T("");
	CString q203 = _T("");
	CString q204 = _T("");
	CString q206 = _T("");
	CString q210 = _T("");
	CString q211 = _T("");
}

void Drill::fillDrill(CString lineq00, CString lineq01, CString lineq06, CString lineq02, CString lineq10, CString lineq03, CString lineq04, CString lineq11) {
	q200 = fillQValues(lineq00);
	q201 = fillQValues(lineq01);
	q206 = fillQValues(lineq06);
	q202 = fillQValues(lineq02);
	q210 = fillQValues(lineq10);
	q203 = fillQValues(lineq03);
	q204 = fillQValues(lineq04);
	q211 = fillQValues(lineq11);
}

CString Drill::fillQValues(CString line) {

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