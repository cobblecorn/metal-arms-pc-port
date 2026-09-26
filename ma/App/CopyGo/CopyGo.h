// CopyGo.h : main header file for the COPYGO application
//

#if !defined(AFX_COPYGO_H__CDF71545_E997_11D0_9906_0060977F3DB0__INCLUDED_)
#define AFX_COPYGO_H__CDF71545_E997_11D0_9906_0060977F3DB0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

/////////////////////////////////////////////////////////////////////////////
// CCopyGoApp:
// See CopyGo.cpp for the implementation of this class
//

class CCopyGoApp : public CWinApp
{
public:
	CCopyGoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyGoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCopyGoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void Spawn( const char * cr_filename, const char *pszArg=NULL );
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYGO_H__CDF71545_E997_11D0_9906_0060977F3DB0__INCLUDED_)
