//////////////////////////////////////////////////////////////////////////////////////
// pasm.h - main header file for the PASM application
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
#if !defined(AFX_PASM_H__9961A1C4_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
#define AFX_PASM_H__9961A1C4_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "fang.h"
#ifdef _MMI_TARGET_PS2
#include "ps2\resource.h"
#else
#include "resource.h"
#endif
#include "InterProcessData.h"
#include "CSharedStruct.h"

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

extern cchar *Pasm_pszPropName;
extern CSharedStruct<InterProcessData_t> Pasm_SharedAppData;

/////////////////////////////////////////////////////////////////////////////
// CPasmApp:
// See pasm.cpp for the implementation of this class
//

class CPasmApp : public CWinApp
{
public:
	CPasmApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPasmApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPasmApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PASM_H__9961A1C4_54F9_11D5_AEBF_000102CDD4F3__INCLUDED_)
