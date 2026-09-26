// BotTalkinator.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "BotTalkinator.h"
#include "BotTalkinatorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// public vars
cchar *BotTalkinator_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CBotTalkinatorApp

BEGIN_MESSAGE_MAP(CBotTalkinatorApp, CWinApp)
	//{{AFX_MSG_MAP(CBotTalkinatorApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBotTalkinatorApp construction

CBotTalkinatorApp::CBotTalkinatorApp() {
	BotTalkinator_pszPropName = APP_NAMES_BOT_TALK;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBotTalkinatorApp object

CBotTalkinatorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBotTalkinatorApp initialization

BOOL CBotTalkinatorApp::InitInstance() {

	// used to find memory leaks
	//_CrtSetBreakAlloc(377);

	AfxEnableControlContainer();

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, BotTalkinator_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), BotTalkinator_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();			

				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "BotTalkinator is already running.\nIt could not be found to activate it, however.", "App Already Running", MB_OK );
		return FALSE;
	}

	CBotTalkinatorDlg dlg;
	m_pMainWnd = &dlg;

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CBotTalkinatorApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
