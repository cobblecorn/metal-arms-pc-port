// BotTalkinator.h : main header file for the BOTTALKINATOR application
//

#if !defined(AFX_BOTTALKINATOR_H__B8BA3289_83AE_4F0D_A68E_CFAF3B4C2D1D__INCLUDED_)
#define AFX_BOTTALKINATOR_H__B8BA3289_83AE_4F0D_A68E_CFAF3B4C2D1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "InterProcessData.h"

extern cchar *BotTalkinator_pszPropName;

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

/////////////////////////////////////////////////////////////////////////////
// CBotTalkinatorApp:
// See BotTalkinator.cpp for the implementation of this class
//

class CBotTalkinatorApp : public CWinApp
{
public:
	CBotTalkinatorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBotTalkinatorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBotTalkinatorApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};

extern CBotTalkinatorApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOTTALKINATOR_H__B8BA3289_83AE_4F0D_A68E_CFAF3B4C2D1D__INCLUDED_)
