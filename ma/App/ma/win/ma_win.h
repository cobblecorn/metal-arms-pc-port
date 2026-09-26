//////////////////////////////////////////////////////////////////////////////////////
// ma_win.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/22/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_MA_WIN_H__AA648B62_0044_4171_9233_1873317B772F__INCLUDED_)
#define AFX_MA_WIN_H__AA648B62_0044_4171_9233_1873317B772F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "fang.h"
#include "resource.h"		// main symbols

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

extern cchar *Mawin_pszExeName;
extern BOOL Mawin_bAutoRun;
extern char Mawin_pszFullAppPath[256];

/////////////////////////////////////////////////////////////////////////////
// CMa_winApp:
// See ma_win.cpp for the implementation of this class
//

class CMa_winApp : public CWinApp
{
public:
	CMa_winApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMa_winApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMa_winApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};

extern CMa_winApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MA_WIN_H__AA648B62_0044_4171_9233_1873317B772F__INCLUDED_)
