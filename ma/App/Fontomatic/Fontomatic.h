// Fontomatic.h : main header file for the FONTOMATIC application
//

#if !defined(AFX_FONTOMATIC_H__2DB80474_49CF_11D3_973D_005004427DE5__INCLUDED_)
#define AFX_FONTOMATIC_H__2DB80474_49CF_11D3_973D_005004427DE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "fang.h"

extern LPCTSTR Fontomatic_pszExeName;

/////////////////////////////////////////////////////////////////////////////
// CFontomaticApp:
// See Fontomatic.cpp for the implementation of this class
//

class CFontomaticApp : public CWinApp
{
public:
	CFontomaticApp();

protected:
	HANDLE m_hMutex;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontomaticApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFontomaticApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CFontomaticApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTOMATIC_H__2DB80474_49CF_11D3_973D_005004427DE5__INCLUDED_)
