// Spit.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Spit.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

// public vars
cchar *Spit_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CSpitApp

BEGIN_MESSAGE_MAP(CSpitApp, CWinApp)
	//{{AFX_MSG_MAP(CSpitApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpitApp construction

CSpitApp::CSpitApp() {

	Spit_pszPropName = APP_NAMES_SPIT;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSpitApp object

CSpitApp theApp;
CSpitDlg *Spit_pSpitDlg;

/////////////////////////////////////////////////////////////////////////////
// CSpitApp initialization

BOOL CSpitApp::InitInstance()
{
	// Standard initialization

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, Spit_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), Spit_pszPropName )) {
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

	CSpitDlg dlg;
	m_pMainWnd = &dlg;
	Spit_pSpitDlg = &dlg;
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

int CSpitApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
