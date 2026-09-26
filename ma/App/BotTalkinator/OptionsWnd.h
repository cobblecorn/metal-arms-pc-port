#if !defined(AFX_OPTIONSWND_H__E52C080B_073A_4911_ADD1_964DEA80754D__INCLUDED_)
#define AFX_OPTIONSWND_H__E52C080B_073A_4911_ADD1_964DEA80754D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsWnd.h : header file
//

#include "BotTalkinatorDlg.h"
#include "vidmode.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsWnd dialog

class COptionsWnd : public CDialog
{
// Construction
public:
	COptionsWnd(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptionsWnd)
	enum { IDD = IDD_DIALOG_OPTIONS };
	CString	m_sInputDir;
	CString	m_sMasterFile;
	CString	m_sOutputDir;
	//}}AFX_DATA

	
	CBotTalkinatorDlg *m_pParentDlg;

	BOOL m_bValidVideoMode;
	CVidMode m_oVidMode;

	BOOL m_bMasterFileChanged;
	BOOL m_bInputDirChanged;
	BOOL m_bOutputDirChanged;

	BOOL DoGetOptions( CBotTalkinatorDlg *pParentDlg );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsWnd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptionsWnd)
	afx_msg void OnButtonVideomode();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonBrowseInputDir();
	afx_msg void OnButtonBrowseOutputDir();
	afx_msg void OnButtonBrowsemaster();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSWND_H__E52C080B_073A_4911_ADD1_964DEA80754D__INCLUDED_)
