// SrcTreeArchiverDlg.h : header file
//

#if !defined(AFX_SRCTREEARCHIVERDLG_H__CE736965_1FCC_4B5E_800C_5D7D37A93794__INCLUDED_)
#define AFX_SRCTREEARCHIVERDLG_H__CE736965_1FCC_4B5E_800C_5D7D37A93794__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fang.h"

/////////////////////////////////////////////////////////////////////////////
// CSrcTreeArchiverDlg dialog

class CSrcTreeArchiverDlg : public CDialog
{
// Construction
public:
	CSrcTreeArchiverDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSrcTreeArchiverDlg)
	enum { IDD = IDD_SRCTREEARCHIVER_DIALOG };
	CDateTimeCtrl	m_ctrlDatePicker;
	CComboBox	m_ctrlSpanDisks;
	CComboBox	m_ctrlProfile;
	CProgressCtrl	m_ctrlProgressBar;
	CString	m_sIgnoreExtList;
	CString	m_sOutputFilename;
	CString	m_sProgressText;
	CString	m_sSrcTreeDir;
	BOOL	m_bRecurseSubDirs;
	BOOL	m_bClickNGo;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSrcTreeArchiverDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	u32 m_nCurrentProfile;

	// Generated message map functions
	//{{AFX_MSG(CSrcTreeArchiverDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonPickSourceTree();
	afx_msg void OnRunButton();
	virtual void OnCancel();
	virtual void OnOK();// short circuits the default behavior of enter key, now enter has no effect
	afx_msg void OnSelchangeProfileCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void CreateZipFile( BOOL bNoMsgBoxes, const CString &rsIgnoreExtList, const CString &rsOutputFilename,
						const CString &rsSrcTreeDir, BOOL bRecurseSubDirs, CTime &rDate, u32 nBytesPerDisk, BOOL bUseDate );
	void SaveCurrentProfile();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SRCTREEARCHIVERDLG_H__CE736965_1FCC_4B5E_800C_5D7D37A93794__INCLUDED_)
