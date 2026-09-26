//////////////////////////////////////////////////////////////////////////////////////
// WavBanker.h - main header file for the WAVBANKER application
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
// 06/04/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_WAVBANKER_H__6E335309_7D3C_4EEC_A18A_C6792CB1EBEE__INCLUDED_)
#define AFX_WAVBANKER_H__6E335309_7D3C_4EEC_A18A_C6792CB1EBEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "fang.h"
#include "resource.h"		// main symbols
#include "InterProcessData.h"

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

extern cchar *WavBanker_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CWavBankerApp:
// See WavBanker.cpp for the implementation of this class
//

class CWavBankerApp : public CWinApp
{
public:
	CWavBankerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWavBankerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWavBankerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVBANKER_H__6E335309_7D3C_4EEC_A18A_C6792CB1EBEE__INCLUDED_)
