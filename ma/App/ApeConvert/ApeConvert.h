// ApeConvert.h : main header file for the APECONVERT application
//

#if !defined(AFX_APECONVERT_H__4648C7C4_3523_11D5_AEBC_000102CDD4F3__INCLUDED_)
#define AFX_APECONVERT_H__4648C7C4_3523_11D5_AEBC_000102CDD4F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define VARS_TO_CONTROLS	FALSE
#define CONTROLS_TO_VARS	TRUE

/////////////////////////////////////////////////////////////////////////////
// CApeConvertApp:
// See ApeConvert.cpp for the implementation of this class
//

class CApeConvertApp : public CWinApp
{
public:
	CApeConvertApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApeConvertApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CApeConvertApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APECONVERT_H__4648C7C4_3523_11D5_AEBC_000102CDD4F3__INCLUDED_)
