#include "pch.h"
#include "SearchAlgorithms.h"
#include "ConfigFile.h"
#include "Transformation.h"
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

bool SearchAlgorithms::searchForToolChange(int index, CStringArray &file) {
	bool foundToolCall = false;
	for (int i = index; i < file.GetSize(); i++) {
		if (file.GetAt(i).Find(_T("TOOL CALL")) != -1) {
			foundToolCall = true;
			break;
		}
		else if (file.GetAt(i).Find(_T("CALL LBL")) != -1) {
			break;
		}
	}

	return foundToolCall;
}

CString SearchAlgorithms::findFeedRate(CString line) {
	CString feedRate = _T("F");
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
	return feedRate;
}

CString SearchAlgorithms::findSequenceName(CString line) {
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
		//test der klassen 
		if (foundComment == true) {
			mw_op_comment_string.AppendChar(line.GetAt(i));
		}
	}
	return mw_op_comment_string;
}

CString SearchAlgorithms::findOtherLine(CString line,CString mw_other_line) {
	CString convertedLine = _T("");
	convertedLine.Append(mw_other_line);
	CString lineNumber = ConversionAlgorithms::findLineNr(line);
	line = ConversionAlgorithms::cutAtSpace(line, 1);
	convertedLine.Append(lineNumber);
	convertedLine.Append(_T("# "));
	line = ConversionAlgorithms::cutAtSpace(line, 0);
	convertedLine.Append(line);
	return convertedLine;
}

CString SearchAlgorithms::findOtherLine(CString line, char c, CString mw_other_line) {
	CString convertedLine = _T("");
	convertedLine.Append(mw_other_line);
	CString lineNumber = ConversionAlgorithms::findLineNr(line);
	convertedLine.Append(lineNumber);
	convertedLine.Append(_T("# "));
	line = ConversionAlgorithms::cutAtSpace(line, 1, c);
	convertedLine.Append(line);
	return convertedLine;
}