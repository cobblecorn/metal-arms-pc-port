// CopyGoDlg.h : header file
//

#if !defined(AFX_COPYGODLG_H__CDF71547_E997_11D0_9906_0060977F3DB0__INCLUDED_)
#define AFX_COPYGODLG_H__CDF71547_E997_11D0_9906_0060977F3DB0__INCLUDED_

#include "dlgbars.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CCopyGoDlg dialog

class CCopyGoDlg : public CDialog
{
// Construction
public:
	CCopyGoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCopyGoDlg)
	enum { IDD = IDD_COPYGO_DIALOG };
	CListCtrl	m_dependencies;
	CString	m_program_file;
	CString	m_args;
	CString	m_wkdir;
	CString	m_sVersion;
	//}}AFX_DATA

	CString			m_cr_filename;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyGoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	void	Load();
	void	Save();

	int				m_dependency_item;

	BOOL			m_modified;

	CDlgToolBar		m_toolBar;

	// Generated message map functions
	//{{AFX_MSG(CCopyGoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnProgramBrowse();
	afx_msg void OnAddDependency();
	afx_msg void OnRemoveDependency();
	afx_msg void OnItemchangedDependencies(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnEdit();
	afx_msg void OnChangeProgramFile();
	afx_msg void OnWkdirBrowse();
	afx_msg void OnChangeArgs();
	afx_msg void OnChangeWkdir();
	afx_msg void OnDblclkDependencies(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYGODLG_H__CDF71547_E997_11D0_9906_0060977F3DB0__INCLUDED_)
