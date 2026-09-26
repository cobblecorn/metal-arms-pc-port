//////////////////////////////////////////////////////////////////////////////////////
// jaws.h - main header file for the JAWS application
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
// 07/19/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_JAWS_H__1BDF7364_FE98_11D4_A008_000102CDD45D__INCLUDED_)
#define AFX_JAWS_H__1BDF7364_FE98_11D4_A008_000102CDD45D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "InterProcessData.h"
#include "CSharedStruct.h"

#define JAWS_REG_KEY		"Software\\SwinginApe\\Jaws"

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

extern cchar *Jaws_pszPropName;
extern CSharedStruct<InterProcessData_t> Jaws_SharedAppData;

/////////////////////////////////////////////////////////////////////////////
// CJawsApp:
// See jaws.cpp for the implementation of this class
//

class CJawsApp : public CWinApp
{
public:
	CJawsApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJawsApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJawsApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};

extern CJawsApp theApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JAWS_H__1BDF7364_FE98_11D4_A008_000102CDD45D__INCLUDED_)
