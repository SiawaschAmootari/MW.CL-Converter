#include "pch.h"
#include "framework.h"
#include "MW.CL Converter.h"
#include "MW.CL ConverterDlg.h"
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
class ConvertHeidenhain{
	ConvertHeidenhain::ConvertHeidenhain() {};

	void ConvertHeidenhain::startConverting(CStringArray &fileContent) {
		fileContent.Add(_T("I was in Heidenhein Class"));
	}

		

};