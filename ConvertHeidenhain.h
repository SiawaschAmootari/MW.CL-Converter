#pragma once
#include <afxcoll.h>


using namespace std;

class ConvertHeidenhain
{



public:
	CStringArray fileContent;
	ConvertHeidenhain(void);
	void startConverting(CStringArray &fileContent);
	 
	

};

