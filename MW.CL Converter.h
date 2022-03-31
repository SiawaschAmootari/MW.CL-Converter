
// MW.CL Converter.h: Hauptheaderdatei für die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'pch.h' vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"		// Hauptsymbole


// CMWCLConverterApp:
// Siehe MW.CL Converter.cpp für die Implementierung dieser Klasse
//

class CMWCLConverterApp : public CWinApp
{
public:
	CMWCLConverterApp();
	void	ArrToVal(const CStringArray& sValArr, CString& sVal);
	void	ArrToVal(const CString sDel, const CStringArray& sValArr, CString& sVal);
// Überschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CMWCLConverterApp theApp;
