//////////////////////////////////////////////////////////////////////////////////////
// K9.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 06/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "K9.h"
#include "K9Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// public vars
cchar *K9_pszPropName;
CSharedStruct<InterProcessData_t> K9_SharedAppData( INTER_PROCESS_SHARED_FILE_NAME );

/////////////////////////////////////////////////////////////////////////////
// CK9App

BEGIN_MESSAGE_MAP(CK9App, CWinApp)
	//{{AFX_MSG_MAP(CK9App)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CK9App construction

CK9App::CK9App() {

	K9_pszPropName = APP_NAMES_K9;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CK9App object

CK9App theApp;

/////////////////////////////////////////////////////////////////////////////
// CK9App initialization
int YourAllocHook( int allocType, void *userData, size_t size, int blockType, 
   long requestNumber, const unsigned char *filename, int lineNumber)
{
	if (size == 6000)
	{
		int i = 2;
	}
	return 1;
}

BOOL CK9App::InitInstance()
{
	// Standard initialization
//	_CrtSetBreakAlloc(1983);
//	_CrtSetAllocHook(YourAllocHook);

int YourAllocHook( int allocType, void *userData, size_t size, int blockType, 
   long requestNumber, const unsigned char *filename, int lineNumber);

	
	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, K9_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), K9_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "This app is already running.\nIt could not be found to activate it, however.", "App Already Running", MB_OK );
		return FALSE;
	}

	CK9Dlg dlg;
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

int CK9App::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}


const char* K9_GetBackupDirPath(void)
{
	CK9Dlg* pDlg = (CK9Dlg*) (theApp.m_pMainWnd);

	return pDlg->m_sScreenshotDir;
	
}

