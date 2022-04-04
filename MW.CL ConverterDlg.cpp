
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
	DDX_Control(pDX, IDC_EDIT_FILE_INPUT, m_EDIT_FILE_INPUT);
	DDX_Control(pDX, IDC_EDIT_FILE_OUTPUT, m_EDIT_FILE_OUTPUT);
	DDX_Control(pDX, IDC_LIST_MESSAGES, m_LIST_MESSAGES);
	DDX_Control(pDX, IDC_COMBO_FILE_PATH, m_COMBO_FILE_PATH);
}

BEGIN_MESSAGE_MAP(CMWCLConverterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CMWCLConverterDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CONVERT, &CMWCLConverterDlg::OnBnClickedButtonConvert)
END_MESSAGE_MAP()


// CMWCLConverterDlg-Meldungshandler

BOOL CMWCLConverterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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
		CFileDialog cFileDialog(true, NULL, NULL, NULL, _T("h-files(*.h)|*.h;|All-files(*.*)|*.*;|"));

		int iId;
		iId = (int)cFileDialog.DoModal();
		bool bOk = true;
		CFileStatus filestatus;
		//CString m_sInputfile;
		CStdioFile file;
		int fileSize;
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
				for (int i = 0; i < 100; i++) {
					firstHundredLines.Add(m_sFilecontent.GetAt(i));

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
	catch (const std::out_of_range& e)
	{
		m_LIST_MESSAGES.InsertString(0, _T("No file selected"));
	}
	catch (const std::invalid_argument& e)
	{
		m_LIST_MESSAGES.InsertString(0, _T("Invalid file"));
	}
}

//Übersetzer
void CMWCLConverterDlg::OnBnClickedButtonConvert()
{
	ConvertHeidenhain convert;
	convert.startConverting(m_sFilecontent);

}
