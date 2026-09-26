//////////////////////////////////////////////////////////////////////////////////////
// Settings.h - 
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
#if !defined(AFX_SETTINGS_H__F0A83D43_011B_11D5_A008_000102CDD45D__INCLUDED_)
#define AFX_SETTINGS_H__F0A83D43_011B_11D5_A008_000102CDD45D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Settings.h : header file
//

#include "fcolor.h"

typedef struct {
	CString sMasterFile;
	BOOL bAutoRunOnLoad;
	CString sScreenShotPath;
} Settings_t;

/////////////////////////////////////////////////////////////////////////////
// CSettings dialog

class CSettings : public CDialog
{
// Construction
public:
	void GetCurrentSettings( Settings_t *pDestSettings );
	CSettings(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettings)
	enum { IDD = IDD_DIALOG_SETTINGS };
	CString	m_sMasterFile;
	BOOL	m_bAutoRunOnLoad;
	CString	m_sScreenShotDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void GetSettingsFromRegistry( Settings_t *pDestSettings );
	BOOL StoreSettingsIntoRegistry( const Settings_t *pSettings );
	Settings_t m_CurrentSettings;
	
	// Generated message map functions
	//{{AFX_MSG(CSettings)
	afx_msg void OnButtonPickMasterFile();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChooseScreenshotDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGS_H__F0A83D43_011B_11D5_A008_000102CDD45D__INCLUDED_)
