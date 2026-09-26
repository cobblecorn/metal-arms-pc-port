//////////////////////////////////////////////////////////////////////////////////////
// K9.h - 
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
// 06/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_K9_H__677E0106_1965_11D5_AEBA_000102CDD4F3__INCLUDED_)
#define AFX_K9_H__677E0106_1965_11D5_AEBA_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "fang.h"
#include "resource.h"		// main symbols
#include "InterProcessData.h"
#include "CSharedStruct.h"

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

extern cchar *K9_pszPropName;
extern CSharedStruct<InterProcessData_t> K9_SharedAppData;

/////////////////////////////////////////////////////////////////////////////
// CK9App:
// See K9.cpp for the implementation of this class
//

class CK9App : public CWinApp
{
public:
	CK9App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CK9App)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CK9App)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};

extern CK9App theApp;
const char* K9_GetBackupDirPath(void);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_K9_H__677E0106_1965_11D5_AEBA_000102CDD4F3__INCLUDED_)
