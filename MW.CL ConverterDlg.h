
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
	CEdit m_EDIT_FILE_INPUT;
	CEdit m_EDIT_FILE_OUTPUT;
	CListBox m_LIST_MESSAGES;
	CStringArray m_sFilecontent;
	CStringArray m_sFileConverted;
	CStringArray g_conversionHistory;
	CStringArray g_pprintList;
	CStringArray g_toolList;

	CString g_sFilePath;
	CString m_FILE_NAME;
	CString g_x;
	CString g_y;
	CString g_z;
	CString g_fedRat;
	CString g_diameter;

	afx_msg void OnBnClickedButtonOpen();
	CComboBox m_COMBO_FILE_PATH;
	afx_msg void OnBnClickedButtonConvert();
};