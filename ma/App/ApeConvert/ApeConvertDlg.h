// ApeConvertDlg.h : header file
//

#if !defined(AFX_APECONVERTDLG_H__4648C7C6_3523_11D5_AEBC_000102CDD4F3__INCLUDED_)
#define AFX_APECONVERTDLG_H__4648C7C6_3523_11D5_AEBC_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CApeConvertDlg dialog

class CApeConvertDlg : public CDialog
{
// Construction
public:
	CApeConvertDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CApeConvertDlg)
	enum { IDD = IDD_APECONVERT_DIALOG };
	CButton	m_buttonExit;
	CProgressCtrl	m_ctrlProgressBar;
	CString	m_sInputFilename;
	CString	m_sProgressText;
	CListBox m_listboxOutput;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApeConvertDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CApeConvertDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnChooseInputFile();
	afx_msg void OnConvert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_bFangOK;
	CStringArray m_asFilesToCompile;
	BOOL ConvertApeFile( CString &rOutputString );
	BOOL GetFilenameFromFullPath( CString &sPathname, CString &sFilename );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APECONVERTDLG_H__4648C7C6_3523_11D5_AEBC_000102CDD4F3__INCLUDED_)
