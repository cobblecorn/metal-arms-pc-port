// xclient.h : main header file for the XCLIENT application
//

#if !defined(AFX_XCLIENT_H__5BA3BFB4_ECDE_4663_83D5_D3CBABFF4B65__INCLUDED_)
#define AFX_XCLIENT_H__5BA3BFB4_ECDE_4663_83D5_D3CBABFF4B65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#include "fang.h"
#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CXClientApp:
// See xclient.cpp for the implementation of this class
//

class CXClientApp : public CWinApp
{
public:
	CXClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CXClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XCLIENT_H__5BA3BFB4_ECDE_4663_83D5_D3CBABFF4B65__INCLUDED_)
