// GCSndBankUtil.h : main header file for the GCSNDBANKUTIL application
//

#if !defined(AFX_GCSNDBANKUTIL_H__48E27B76_B830_4A41_8645_7954FADA8045__INCLUDED_)
#define AFX_GCSNDBANKUTIL_H__48E27B76_B830_4A41_8645_7954FADA8045__INCLUDED_

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

extern cchar *GCSndBankUtil_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CGCSndBankUtilApp:
// See GCSndBankUtil.cpp for the implementation of this class
//

class CGCSndBankUtilApp : public CWinApp
{
public:
	CGCSndBankUtilApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCSndBankUtilApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGCSndBankUtilApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCSNDBANKUTIL_H__48E27B76_B830_4A41_8645_7954FADA8045__INCLUDED_)
