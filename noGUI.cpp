#include "pch.h"
#include "framework.h"
#include "MW.CL Converter.h"
#include "MW.CL ConverterDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include "ConvertHeidenhain.h"

using namespace std;

void openFile(CString path, CStringArray& subFileContent) {
	CStdioFile csfFile;
	if (std::ifstream(path).good())
	{
		try
		{
			csfFile.Open(path, CStdioFile::modeRead);
			CString line = _T("");
			bool bRead;
			CString filecontent = _T("");

			while (true)
			{
				bRead = csfFile.ReadString(line);
				if (bRead == false)
				{
					break;
				}
				subFileContent.Add(line);
			}
			csfFile.Close();
		
		}
		catch (const std::out_of_range&)
		{
			cout << "ERROR OUT OF RANGE" << endl;
		}
		catch (const std::invalid_argument&)
		{
			cout << "ERROR INVALID ARGUMENT" << endl;
		}
	}
	else
	{
		cout << "ERROR INVALID PATH" << endl << path << endl;
	}
}

void startConverting() {
	CString path = _T("C:\\Users\\samootari\\OneDrive\Desktop\\MW.CL Konverter\\planen.tap");
	int labelIndex = 0;
	CStringArray filecontent;
	openFile(path,filecontent);

	CStringArray firstHundredLines;
	for (int i = 0; i < m_sFilecontent.GetSize(); i++) {
		firstHundredLines.Add(m_sFilecontent.GetAt(i));
		if (m_sFilecontent.GetAt(i).Find(_T("PGM ENDE")) != -1) {
			labelIndex = i + 2;
		}
	}

	ConvertHeidenhain convert;

	convert.startConverting(filecontent, labelIndex, path);
	CString sFilecontent;
	CStringArray convertedFile;
	for (int i = 0; i < convert.convertedFileContent.GetSize(); i++) {
		convertedFile.Add(convert.convertedFileContent.GetAt(i));
	}

	string filename("mw.tap");
	fstream file_out;

	file_out.open(filename, std::ios_base::out);
	if (!file_out.is_open()) {
		cout << "failed to open " << filename << '\n';
	}
	else {
		for (int i = 0; i < convertedFile.GetSize(); i++) {
			file_out << convertedFile.GetAt(i)<< endl;
		}
	}
}


int main() {
	startConverting();
}