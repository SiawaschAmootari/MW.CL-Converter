
// MW.CL Converter.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "pch.h"
#include "framework.h"
#include "MW.CL Converter.h"
#include "MW.CL ConverterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMWCLConverterApp

BEGIN_MESSAGE_MAP(CMWCLConverterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMWCLConverterApp-Erstellung

CMWCLConverterApp::CMWCLConverterApp()
{
	// TODO: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance positionieren
}


// Das einzige CMWCLConverterApp-Objekt

CMWCLConverterApp theApp;


// CMWCLConverterApp-Initialisierung

BOOL CMWCLConverterApp::InitInstance()
{
	CWinApp::InitInstance();


	// Shell-Manager erstellen, falls das Dialogfeld
	// Shellbaumansicht- oder Shelllistenansicht-Steuerelemente enthält.
	CShellManager *pShellManager = new CShellManager;

	//Visuellen Manager "Windows Native" aktivieren, um Designs für MFC-Steuerelemente zu aktivieren
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standardinitialisierung
	// Wenn Sie diese Funktionen nicht verwenden und die Größe
	// der ausführbaren Datei verringern möchten, entfernen Sie
	// die nicht erforderlichen Initialisierungsroutinen.
	// Ändern Sie den Registrierungsschlüssel, unter dem Ihre Einstellungen gespeichert sind.
	// TODO: Ändern Sie diese Zeichenfolge entsprechend,
	// z.B. zum Namen Ihrer Firma oder Organisation.
	SetRegistryKey(_T("Vom lokalen Anwendungs-Assistenten generierte Anwendungen"));

	CMWCLConverterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Fügen Sie hier Code ein, um das Schließen des
		//  Dialogfelds über "OK" zu steuern
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Fügen Sie hier Code ein, um das Schließen des
		//  Dialogfelds über "Abbrechen" zu steuern
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warnung: Fehler bei der Dialogfelderstellung, unerwartetes Beenden der Anwendung.\n");
		TRACE(traceAppMsg, 0, "Warnung: Wenn Sie MFC-Steuerelemente im Dialogfeld verwenden, ist #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS nicht möglich.\n");
	}

	// Den oben erstellten Shell-Manager löschen.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Da das Dialogfeld geschlossen wurde, FALSE zurückliefern, sodass wir die
	//  Anwendung verlassen, anstatt das Nachrichtensystem der Anwendung zu starten.
	return FALSE;
}

void	CMWCLConverterApp::ArrToVal(const CStringArray& sValArr, CString& sVal)
{
	return ArrToVal(_T("\r\n"), sValArr, sVal);
}

// #sia
void	CMWCLConverterApp::ArrToVal(const CString sDel, const CStringArray& sValArr, CString& sVal)
{
	// Call:

	sVal = _T("");
	if (sValArr.IsEmpty() == TRUE)	return;

	for (int iLine = 0; iLine < sValArr.GetSize(); iLine++)
	{
		sVal += sValArr[iLine];
		if (iLine < sValArr.GetSize() - 1) sVal += sDel;
	}
}

