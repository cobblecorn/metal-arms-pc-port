//////////////////////////////////////////////////////////////////////////////////////
// StringInput.h - general class for getting a users string input
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
// 05/31/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_STRINGINPUT_H__044DEAA6_9C5A_11D3_975F_005004427DE5__INCLUDED_)
#define AFX_STRINGINPUT_H__044DEAA6_9C5A_11D3_975F_005004427DE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fang.h"

/////////////////////////////////////////////////////////////////////////////
// CStringInput dialog

class CStringInput : public CDialog
{
// Construction
public:
	CStringInput( CWnd* pParent = NULL );   // standard constructor
	BOOL GetUserString( cchar *pszDefault, cchar *pszTitle, cchar *pszInstructions, CString& sReturn );
// Dialog Data
	//{{AFX_DATA(CStringInput)
	enum { IDD = IDD_ENTER_STRING_DLG };
	CString	m_sInstructions;
	CString	m_sUserString;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringInput)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStringInput)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CString m_sTitle;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGINPUT_H__044DEAA6_9C5A_11D3_975F_005004427DE5__INCLUDED_)
