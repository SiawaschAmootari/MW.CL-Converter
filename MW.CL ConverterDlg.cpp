
// MW.CL ConverterDlg.cpp: Implementierungsdatei
//

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
#include <windows.h>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg-Dialogfeld für Anwendungsbefehl "Info"

class CAboutDlg : public CDialogEx 
{
public:
	CAboutDlg();

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

// Implementierung
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMWCLConverterDlg-Dialogfeld



CMWCLConverterDlg::CMWCLConverterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MWCL_CONVERTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CMWCLConverterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	/*DDX_Control(pDX, IDC_EDIT_FILE_INPUT, m_EDIT_FILE_INPUT);
	DDX_Control(pDX, IDC_EDIT_FILE_OUTPUT, m_EDIT_FILE_OUTPUT);
	DDX_Control(pDX, IDC_LIST_MESSAGES, m_LIST_MESSAGES);
	DDX_Control(pDX, IDC_COMBO_FILE_PATH, m_COMBO_FILE_PATH);*/
	DDX_Control(pDX, m_progress, m_progressbar);
}

BEGIN_MESSAGE_MAP(CMWCLConverterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CMWCLConverterDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CONVERT, &CMWCLConverterDlg::OnBnClickedButtonConvert)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CMWCLConverterDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CMWCLConverterDlg-Meldungshandler

BOOL CMWCLConverterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//SendMessage();
	// Hinzufügen des Menübefehls "Info..." zum Systemmenü.

	// IDM_ABOUTBOX muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Symbol für dieses Dialogfeld festlegen.  Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	
	// TODO: Hier zusätzliche Initialisierung einfügen
	quickOpen();
	quickConvert();
	quickSave();
	
	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void CMWCLConverterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}	
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie
//  den nachstehenden Code, um das Symbol zu zeichnen.  Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CMWCLConverterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}

	quickConvert();
	quickSave();
	for (int i = 0; i < 100; i++) {
		m_progressbar.SetPos(i);
		Sleep(0001);
	}
	exit(0);
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CMWCLConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//Öffnet das Programm
void CMWCLConverterDlg::OnBnClickedButtonOpen()
{
	try
	{
		CFileDialog cFileDialog(true, NULL, NULL, NULL, _T("All-files(*.*)|*.*;|"));

		int iId;
		iId = (int)cFileDialog.DoModal();
		bool bOk = true;
		CFileStatus filestatus;
		//CString m_sInputfile;
		CStdioFile file;
		//int fileSize;
		if (iId == IDOK)
		{
			g_sFilePath = cFileDialog.GetPathName();
			if (std::ifstream(g_sFilePath).good())
			{
				CString sNewName = g_sFilePath + "_backup";
				m_COMBO_FILE_PATH.InsertString(0, g_sFilePath);
				//rename(m_sInputfile, newName);
				CStdioFile file;
				file.Open(g_sFilePath, CStdioFile::modeRead);

				CString sLine = _T("");
				bool bRead;
				CString sFilecontent = _T("");
				int i = 0;

				m_sFilecontent.RemoveAll();
				m_FILE_NAME = g_sFilePath;

				while (true)
				{
					bRead = file.ReadString(sLine);
					if (bRead == false)
					{
						break;
					}
					else {
						m_sFilecontent.Add(sLine);
					}
				}
				//theApp.ArrToVal(m_sFilecontent, sFilecontent);
				CStringArray firstHundredLines;
				for (int i = 0; i < m_sFilecontent.GetSize(); i++) {
					firstHundredLines.Add(m_sFilecontent.GetAt(i));
					if (m_sFilecontent.GetAt(i).Find(_T("PGM ENDE")) != -1) {
						labelIndex = i+2;
					}

				}
				theApp.ArrToVal(firstHundredLines, sFilecontent);
				m_EDIT_FILE_INPUT.SetWindowText(sFilecontent);

				file.Close();
				//findToolCycle();
			}
			if (m_FILE_NAME.GetLength() <= 0)
			{
				m_LIST_MESSAGES.InsertString(0, _T("No file selected"));
			}
			else
			{
				UpdateData(false);
			}
		}
	}
	catch (const std::out_of_range& )
	{
		m_LIST_MESSAGES.InsertString(0, _T("No file selected"));
	}
	catch (const std::invalid_argument& )
	{
		m_LIST_MESSAGES.InsertString(0, _T("Invalid file"));
	}
}

void CMWCLConverterDlg::quickOpen()
{		bool bOk = true;
		CFileStatus filestatus;
		//CString m_sInputfile;
		CStdioFile file;
		//int fileSize;
		 g_sFilePath = _T("C:\\Users\\samootari\\OneDrive\\Desktop\\MW.CL Konverter\\op010.tap");
			if (std::ifstream(g_sFilePath).good())
			{
				CString sNewName = g_sFilePath + "_backup";
				//m_COMBO_FILE_PATH.InsertString(0, g_sFilePath);
				//rename(m_sInputfile, newName);
				CStdioFile file;
				file.Open(g_sFilePath, CStdioFile::modeRead);

				CString sLine = _T("");
				bool bRead;
				CString sFilecontent = _T("");
				int i = 0;

				m_sFilecontent.RemoveAll();
				m_FILE_NAME = g_sFilePath;

				while (true)
				{
					bRead = file.ReadString(sLine);
					if (bRead == false)
					{
						break;
					}
					else {
						m_sFilecontent.Add(sLine);
					}
				}
				//theApp.ArrToVal(m_sFilecontent, sFilecontent);
				CStringArray firstHundredLines;
				for (int i = 0; i < m_sFilecontent.GetSize(); i++) {
					firstHundredLines.Add(m_sFilecontent.GetAt(i));
					if (m_sFilecontent.GetAt(i).Find(_T("PGM ENDE")) != -1) {
						labelIndex = i+2;
					}

				}
				theApp.ArrToVal(firstHundredLines, sFilecontent);
				//m_EDIT_FILE_INPUT.SetWindowText(sFilecontent);

				file.Close();
				//findToolCycle();
			}
			if (m_FILE_NAME.GetLength() <= 0)
			{
				m_LIST_MESSAGES.InsertString(0, _T("No file selected"));
			}
			else
			{
				UpdateData(false);
			}
		}
	



//Übersetzer
void CMWCLConverterDlg::OnBnClickedButtonConvert()
{
	ConvertHeidenhain convert;

	convert.startConverting(m_sFilecontent, labelIndex,g_sFilePath);
	CString sFilecontent;
	CStringArray firstHundredLines;
	for (int i = 0; i < convert.convertedFileContent.GetSize(); i++) {
		firstHundredLines.Add(convert.convertedFileContent.GetAt(i));

	}
	theApp.ArrToVal(firstHundredLines, sFilecontent);
	//m_EDIT_FILE_OUTPUT.SetWindowText(sFilecontent);

	//for (int i = 0; i < convert.mw_op_number_list.GetSize(); i++) {
	//	firstHundredLines.Add(convert.mw_op_number_list.GetAt(i));
	//}
	//for (int i = 0; i < convert.creoConfiContent.GetSize(); i++) {
	//	firstHundredLines.Add(convert.creoConfiContent.GetAt(i));
	//}
	//firstHundredLines.Add(convert.mw_tool_name);
	theApp.ArrToVal(firstHundredLines, sFilecontent);
	//m_EDIT_FILE_OUTPUT.SetWindowText(sFilecontent); 

	m_sFileConverted.Copy(firstHundredLines);
	m_progressbar.SetPos(100);
}

void CMWCLConverterDlg::quickConvert()
{
	ConvertHeidenhain convert;

	convert.startConverting(m_sFilecontent, labelIndex, g_sFilePath);
	CString sFilecontent;
	CStringArray firstHundredLines;
	for (int i = 0; i < convert.convertedFileContent.GetSize(); i++) {
		firstHundredLines.Add(convert.convertedFileContent.GetAt(i));

	}
	theApp.ArrToVal(firstHundredLines, sFilecontent);
	//m_EDIT_FILE_OUTPUT.SetWindowText(sFilecontent);

	//for (int i = 0; i < convert.mw_op_number_list.GetSize(); i++) {
	//	firstHundredLines.Add(convert.mw_op_number_list.GetAt(i));
	//}
	//for (int i = 0; i < convert.creoConfiContent.GetSize(); i++) {
	//	firstHundredLines.Add(convert.creoConfiContent.GetAt(i));
	//}
	//firstHundredLines.Add(convert.mw_tool_name);
	theApp.ArrToVal(firstHundredLines, sFilecontent);
	//m_EDIT_FILE_OUTPUT.SetWindowText(sFilecontent);

	m_sFileConverted.Copy(firstHundredLines);
}

void CMWCLConverterDlg::OnBnClickedButtonSave()
{
	CFileDialog cFileDialog(false, _T("mpf"), m_FILE_NAME, OFN_OVERWRITEPROMPT, _T("cl-file (*.cl)|*.cl;"));
	int iId;
	iId = (int)cFileDialog.DoModal();
	bool bOk = true;
	CString m_sSavefile;

	if (iId == IDOK)
	{
		m_sSavefile = cFileDialog.GetPathName();
		CStdioFile file(cFileDialog.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		for (int iIndexM_sFilecontentNew = 0; iIndexM_sFilecontentNew < m_sFileConverted.GetSize(); iIndexM_sFilecontentNew++)
		{
			file.WriteString(m_sFileConverted.GetAt(iIndexM_sFilecontentNew).GetString());
			file.WriteString(_T("\n"));
		}
		if (m_sFilecontent.GetSize() <= 0)
		{
			m_LIST_MESSAGES.InsertString(0, _T("File is empty!"));
		}
		file.Flush();
		file.Close();
	}
}

void CMWCLConverterDlg::quickSave()
{
	//CFileDialog cFileDialog(false, _T("mpf"), m_FILE_NAME, OFN_OVERWRITEPROMPT, _T("cl-file (*.cl)|*.cl;"));
	bool bOk = true;
	CString m_sSavefile;

		m_sSavefile = _T("C:\\Users\\samootari\\OneDrive\\Desktop\\MW.CL Konverter\\mw.cl");
		CStdioFile file(_T("C:\\Users\\samootari\\OneDrive\\Desktop\\MW.CL Konverter\\mw.cl"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);

		for (int iIndexM_sFilecontentNew = 0; iIndexM_sFilecontentNew < m_sFileConverted.GetSize(); iIndexM_sFilecontentNew++)
		{
			file.WriteString(m_sFileConverted.GetAt(iIndexM_sFilecontentNew).GetString());
			file.WriteString(_T("\n"));
		}
		if (m_sFilecontent.GetSize() <= 0)
		{
			m_LIST_MESSAGES.InsertString(0, _T("File is empty!"));
		}
		file.Flush();
		file.Close();
	
}