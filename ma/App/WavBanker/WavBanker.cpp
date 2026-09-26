//////////////////////////////////////////////////////////////////////////////////////
// WavBanker.cpp - Defines the class behaviors for the application.
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 06/04/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "WavBanker.h"
#include "WavBankerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

cchar *WavBanker_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CWavBankerApp

BEGIN_MESSAGE_MAP(CWavBankerApp, CWinApp)
	//{{AFX_MSG_MAP(CWavBankerApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWavBankerApp construction

CWavBankerApp::CWavBankerApp() {
	WavBanker_pszPropName = APP_NAMES_WAV_BANKER;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWavBankerApp object

CWavBankerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWavBankerApp initialization

BOOL CWavBankerApp::InitInstance() {

	AfxEnableControlContainer();

	// Standard initialization

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, WavBanker_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), WavBanker_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "WavBanker is already running.\nIt could not be found to activate it, however.", "WavBanker Already Running", MB_OK );
		return FALSE;
	}

	CWavBankerDlg dlg;
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

int CWavBankerApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
