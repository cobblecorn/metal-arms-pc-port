// ApeLaunchPad.h : main header file for the APELAUNCHPAD application
//

#if !defined(AFX_APELAUNCHPAD_H__A02DF3EB_2CBC_410F_9604_312CA9FB840C__INCLUDED_)
#define AFX_APELAUNCHPAD_H__A02DF3EB_2CBC_410F_9604_312CA9FB840C__INCLUDED_

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

extern cchar *ApeLaunchPad_pszPropName;
extern CSharedStruct<InterProcessData_t> ApeLaunchPad_SharedAppData;

/////////////////////////////////////////////////////////////////////////////
// CApeLaunchPadApp:
// See ApeLaunchPad.cpp for the implementation of this class
//

class CApeLaunchPadApp : public CWinApp
{
public:
	CApeLaunchPadApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApeLaunchPadApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CApeLaunchPadApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APELAUNCHPAD_H__A02DF3EB_2CBC_410F_9604_312CA9FB840C__INCLUDED_)
