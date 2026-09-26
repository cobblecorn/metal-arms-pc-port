// ApeConvert.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ApeConvert.h"
#include "ApeConvertDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CApeConvertApp

BEGIN_MESSAGE_MAP(CApeConvertApp, CWinApp)
	//{{AFX_MSG_MAP(CApeConvertApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApeConvertApp construction

CApeConvertApp::CApeConvertApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CApeConvertApp object

CApeConvertApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CApeConvertApp initialization

BOOL CApeConvertApp::InitInstance()
{
	// Standard initialization

	CApeConvertDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
