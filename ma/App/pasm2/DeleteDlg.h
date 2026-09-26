//////////////////////////////////////////////////////////////////////////////////////
// DeleteDlg.h - 
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
#if !defined(AFX_DELETEDLG_H__F5A0DA5D_128E_4892_86BE_189C012ED1A3__INCLUDED_)
#define AFX_DELETEDLG_H__F5A0DA5D_128E_4892_86BE_189C012ED1A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeleteDlg.h : header file
//

#include "FileInfo.h"
#include "fdata.h"

/////////////////////////////////////////////////////////////////////////////
// CDeleteDlg dialog

class CDeleteDlg : public CDialog
{
// Construction
public:
	CDeleteDlg(CWnd* pParent = NULL);   // standard constructor

	/////////////////////////////////////
	// must be set before calling DoModal
	const CFileInfo *m_pLibFileInfo;
	const CFileInfo *m_pLocFileInfo;
	const cchar *m_pszEntryName;
	BOOL m_bLibStartCondition;// should the lib check start checked
	BOOL m_bLocStartCondition;// should the loc check start checked
	BOOL m_bMasterStartCondition;// should the master check start checked
	/////////////////////////////////////

	// DoModal() returns IDCANCEL if we should stop deleting files (nothing was deleted)
	// DoModal() returns IDOK if we should move on to the next file, will
	// set the following variable if the delete all was selected.
	BOOL m_bDeleteAll;
	u32 m_nFilesToDelete;
	BOOL m_bDeleteLibFile;
	BOOL m_bDeleteLocFile;
	BOOL m_bDeleteMasterFile;

// Dialog Data
	//{{AFX_DATA(CDeleteDlg)
	enum { IDD = IDD_DELETE_FILE_DIALOG };
	BOOL	m_bDeleteFromLib;
	BOOL	m_bDeleteFromLoc;
	BOOL	m_bDeleteFromMaster;
	CString	m_sFilename;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeleteDlg)
	afx_msg void OnDelete();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDeleteAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DELETEDLG_H__F5A0DA5D_128E_4892_86BE_189C012ED1A3__INCLUDED_)
