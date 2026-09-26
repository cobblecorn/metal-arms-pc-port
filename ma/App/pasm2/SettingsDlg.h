//////////////////////////////////////////////////////////////////////////////////////
// SettingsDlg.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/20/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_SETTINGSDLG_H__9961A1CF_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
#define AFX_SETTINGSDLG_H__9961A1CF_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

class CSettingsDlg : public CDialog
{
// Construction
public:
	CSettingsDlg(CWnd* pParent = NULL);   // standard constructor

	static void CreateFilename( CString &rDest, const CString &rPath, const CString &rFilename );

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_SETTINGS_DIALOG };
	CString	m_sXBMasterFileDir;
	CString	m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
    CString m_sPS2MasterFileDir;    // KJ
#endif
	CString	m_sLogDir;
	CString	m_sExtractDir;
	CString	m_sConfigDir;
	CString	m_sIgnoreExtensions;
	CString m_sConfigFilename;
	CString	m_sMasterFilename;
	CString	m_sLibDir;
	CString	m_sLocalDir;
	CString	m_sLibraryLockDir;
	CString	m_sXClientDir;
	BOOL	m_bAutoRunXClient;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	//variables
	
// Implementation
protected:
	HACCEL m_hAccel;

	// Generated message map functions
	//{{AFX_MSG(CSettingsDlg)
	afx_msg void OnChooseXBMasterFileDir();
	afx_msg void OnChooseGCMasterFileDir();
#ifdef _MMI_TARGET_PS2
	afx_msg void OnChoosePS2MasterFileDir(); // KJ
#endif
	afx_msg void OnChooseLogOutputDir();
	afx_msg void OnChooseExtractDir();
	afx_msg void OnChooseConfigDir();
	afx_msg void OnNewConfigFile();
	afx_msg void OnChooseConfigFile();
	afx_msg void OnNewMasterFile();
	afx_msg void OnChooseMasterFile();
	afx_msg void OnChooseLibDir();
	afx_msg void OnChooseLocalDir();	
	virtual BOOL OnInitDialog();
	afx_msg void OnChooseLibLockDir();
	afx_msg void OnChooseXclientDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL ParseUserEnteredString( CString &rUserEnteredString, const CString &rOldString, BOOL bRemoveExtension );
public:
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeLmapMaxsize();
	afx_msg void OnBnClickedFilter();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__9961A1CF_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
