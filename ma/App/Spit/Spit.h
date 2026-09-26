// Spit.h : main header file for the SPIT application
//

#if !defined(AFX_SPIT_H__4D39763F_44A3_4AF0_945E_B23A3A04EC8A__INCLUDED_)
#define AFX_SPIT_H__4D39763F_44A3_4AF0_945E_B23A3A04EC8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "fang.h"
#include "resource.h"		// main symbols
#include "InterProcessData.h"

class CSpitDlg;

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

extern cchar *Spit_pszPropName;
extern CSpitDlg *Spit_pSpitDlg;

/////////////////////////////////////////////////////////////////////////////
// CSpitApp:
// See Spit.cpp for the implementation of this class
//

class CSpitApp : public CWinApp
{
public:
	CSpitApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpitApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSpitApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};

extern CSpitApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPIT_H__4D39763F_44A3_4AF0_945E_B23A3A04EC8A__INCLUDED_)
