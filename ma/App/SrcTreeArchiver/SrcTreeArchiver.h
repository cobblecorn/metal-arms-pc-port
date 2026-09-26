// SrcTreeArchiver.h : main header file for the SRCTREEARCHIVER application
//

#if !defined(AFX_SRCTREEARCHIVER_H__74B30589_2025_48DB_93CA_861C3B6198FC__INCLUDED_)
#define AFX_SRCTREEARCHIVER_H__74B30589_2025_48DB_93CA_861C3B6198FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

extern const char *SrcTreeArchiver_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CSrcTreeArchiverApp:
// See SrcTreeArchiver.cpp for the implementation of this class
//

class CSrcTreeArchiverApp : public CWinApp
{
public:
	CSrcTreeArchiverApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSrcTreeArchiverApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSrcTreeArchiverApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SRCTREEARCHIVER_H__74B30589_2025_48DB_93CA_861C3B6198FC__INCLUDED_)
