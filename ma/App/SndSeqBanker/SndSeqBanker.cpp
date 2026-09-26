// SndSeqBanker.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fang.h"
#include "SndSeqBanker.h"
#include "SndSeqBankerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

cchar *SndSeqBanker_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CSndSeqBankerApp

BEGIN_MESSAGE_MAP(CSndSeqBankerApp, CWinApp)
	//{{AFX_MSG_MAP(CSndSeqBankerApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSndSeqBankerApp construction

CSndSeqBankerApp::CSndSeqBankerApp() {
	SndSeqBanker_pszPropName = APP_NAMES_SEQUENCER;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSndSeqBankerApp object

CSndSeqBankerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSndSeqBankerApp initialization

BOOL CSndSeqBankerApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, SndSeqBanker_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), SndSeqBanker_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "SndSeqBanker is already running.\nIt could not be found to activate it, however.", "SndSeqBanker Already Running", MB_OK );
		return FALSE;
	}

	CSndSeqBankerDlg dlg;
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

int CSndSeqBankerApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}