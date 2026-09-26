// GCSndBankUtilDlg.h : header file
//

#if !defined(AFX_GCSNDBANKUTILDLG_H__0BB7A89B_CE71_4A22_B58D_F02345611756__INCLUDED_)
#define AFX_GCSNDBANKUTILDLG_H__0BB7A89B_CE71_4A22_B58D_F02345611756__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fang.h"
#include "FileInfo.h"
#include "FileListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CGCSndBankUtilDlg dialog

class CGCSndBankUtilDlg : public CDialog
{
// Construction
public:
	CGCSndBankUtilDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CGCSndBankUtilDlg)
	enum { IDD = IDD_GCSNDBANKUTIL_DIALOG };
	CFileListCtrl m_ctrlBankFiles;
	CFileListCtrl m_ctrlMusyXFiles;
	CString	m_sVersion;
	CString	m_sOutputDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCSndBankUtilDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGCSndBankUtilDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnConvertButton();
	afx_msg void OnChooseOutputDir();
	afx_msg void OnDropFiles( HDROP hDropInfo );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CFileInfoArray m_BankFiles;
	CFileInfo m_ProjFile;
	CFileInfo m_SampFile;
	CFileInfo m_HFile;
	CFileInfo m_PoolFile;
	CFileInfo m_SDirFile;

	static BOOL IsStringAFilename( const CString &rsFilename, CString &rsExt );
	void UpdateBankListBox( BOOL bReSortList );
	void UpdateMusyXListBox( BOOL bReSortList );
	void GetOldestMusyXFile( CTime &rOldestTime );
	void GetNewestBankFile( CTime &rNewestTime );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCSNDBANKUTILDLG_H__0BB7A89B_CE71_4A22_B58D_F02345611756__INCLUDED_)
