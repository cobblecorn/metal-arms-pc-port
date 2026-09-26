//////////////////////////////////////////////////////////////////////////////////////
// pasm.cpp - Defines the class behaviors for the application.
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
// 07/20/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "pasm.h"
#include "pasmDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// public vars
cchar *Pasm_pszPropName;
CSharedStruct<InterProcessData_t> Pasm_SharedAppData( INTER_PROCESS_SHARED_FILE_NAME );

/////////////////////////////////////////////////////////////////////////////
// CPasmApp

BEGIN_MESSAGE_MAP(CPasmApp, CWinApp)
	//{{AFX_MSG_MAP(CPasmApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPasmApp construction

CPasmApp::CPasmApp() {
	Pasm_pszPropName = APP_NAMES_PASM;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPasmApp object

CPasmApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPasmApp initialization

BOOL CPasmApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	
	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, Pasm_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), Pasm_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "PASM is already running.\nIt could not be found to activate it, however.", "PASM Already Running", MB_OK );
		return FALSE;
	}
	CPasmDlg dlg;
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

int CPasmApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
