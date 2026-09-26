// SndSeqBanker.h : main header file for the SNDSEQBANKER application
//

#if !defined(AFX_SNDSEQBANKER_H__A5F0F573_9E92_404B_8ACA_E6F2EAAF4BB6__INCLUDED_)
#define AFX_SNDSEQBANKER_H__A5F0F573_9E92_404B_8ACA_E6F2EAAF4BB6__INCLUDED_

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

extern cchar *SndSeqBanker_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CSndSeqBankerApp:
// See SndSeqBanker.cpp for the implementation of this class
//

class CSndSeqBankerApp : public CWinApp
{
public:
	CSndSeqBankerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSndSeqBankerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSndSeqBankerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNDSEQBANKER_H__A5F0F573_9E92_404B_8ACA_E6F2EAAF4BB6__INCLUDED_)
