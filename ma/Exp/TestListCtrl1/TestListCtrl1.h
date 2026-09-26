// TestListCtrl1.h : main header file for the TESTLISTCTRL1 application
//

#if !defined(AFX_TESTLISTCTRL1_H__6DA14595_BAED_4689_914B_0042D0B2ADE2__INCLUDED_)
#define AFX_TESTLISTCTRL1_H__6DA14595_BAED_4689_914B_0042D0B2ADE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestListCtrl1App:
// See TestListCtrl1.cpp for the implementation of this class
//

class CTestListCtrl1App : public CWinApp
{
public:
	CTestListCtrl1App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestListCtrl1App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestListCtrl1App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTLISTCTRL1_H__6DA14595_BAED_4689_914B_0042D0B2ADE2__INCLUDED_)
