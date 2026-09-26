// WavCompile.h : main header file for the WAVCOMPILE application
//

#if !defined(AFX_WAVCOMPILE_H__30F4B64D_998F_4622_AD09_4786A922F6A1__INCLUDED_)
#define AFX_WAVCOMPILE_H__30F4B64D_998F_4622_AD09_4786A922F6A1__INCLUDED_

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

extern cchar *WavCompile_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CWavCompileApp:
// See WavCompile.cpp for the implementation of this class
//

class CWavCompileApp : public CWinApp
{
public:
	CWavCompileApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWavCompileApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWavCompileApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVCOMPILE_H__30F4B64D_998F_4622_AD09_4786A922F6A1__INCLUDED_)
