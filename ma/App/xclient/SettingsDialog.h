#if !defined(AFX_SETTINGSDIALOG_H__708D1BC6_34E0_45DC_8ECF_762974072033__INCLUDED_)
#define AFX_SETTINGSDIALOG_H__708D1BC6_34E0_45DC_8ECF_762974072033__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsDialog.h : header file
//
#include "fang.h"
/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog

class CSettingsDialog : public CDialog
{
// Construction
public:
	CSettingsDialog(CWnd* pParent = NULL);   // standard constructor

	void SetXboxName(const CString &sStr)		 {m_sXboxName = sStr;}
	void SetUserName(const CString &sStr)		 {m_sUserName = sStr;}
	void SetLocalMasterFile(const CString &sStr) {m_sLocalMasterFile = sStr;}
	void SetGameFile(const CString &sStr)        {m_sGameFile = sStr;}

	const CString &GetXboxName(void) {return m_sXboxName;}
	const CString &GetUserName(void) {return m_sUserName;}
	const CString &GetLocalMasterFile(void) {return m_sLocalMasterFile;}
	const CString &GetGameFile(void) {return m_sGameFile;}

// Dialog Data
	//{{AFX_DATA(CSettingsDialog)
	enum { IDD = IDD_SETTINGS };
	CEdit	m_EditGameFile;
	CEdit	m_EditMasterFile;
	CEdit	m_EditUserName;
	CEdit	m_EditXboxName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// !!!!! Must set before DoModal()
	CString m_sXboxName;		// Xbox to connect to
	CString m_sUserName;		// User name to connect to the Xbox with
	CString m_sLocalMasterFile; // Path to the local master file
	CString m_sGameFile;
	// !!!!!

	// Generated message map functions
	//{{AFX_MSG(CSettingsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	virtual void OnOK();
	afx_msg void OnReboot();
	afx_msg void OnBrowse2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDIALOG_H__708D1BC6_34E0_45DC_8ECF_762974072033__INCLUDED_)
