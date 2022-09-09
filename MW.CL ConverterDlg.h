
// MW.CL ConverterDlg.h: Headerdatei
//

#pragma once


// CMWCLConverterDlg-Dialogfeld
class CMWCLConverterDlg : public CDialogEx
{
// Konstruktion
public:
	CMWCLConverterDlg(CWnd* pParent = nullptr);	// Standardkonstruktor

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MWCL_CONVERTER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//CEdit m_EDIT_FILE_INPUT;
	//CEdit m_EDIT_FILE_OUTPUT;
	CListBox m_LIST_MESSAGES;
	CStringArray m_sFilecontent;
	CStringArray m_sFileConverted;
	CStringArray g_conversionHistory;
	CStringArray g_pprintList;
	CStringArray g_toolList;
	CStringArray save;
	CString g_sFilePath;
	CString m_FILE_NAME;
	int labelIndex=0;
	//ConvertHeidenhain convert;

	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonConvert();
	afx_msg void OnBnClickedButtonSave();
	void quickOpen();
	void quickConvert();
	void quickSave();
	CProgressCtrl progress;
	
};

