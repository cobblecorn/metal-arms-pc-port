// ApeLaunchPad.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ApeLaunchPad.h"
#include "ApeLaunchPadDlg.h"
#include "fclib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// public vars
cchar *ApeLaunchPad_pszPropName;
CSharedStruct<InterProcessData_t> ApeLaunchPad_SharedAppData( INTER_PROCESS_SHARED_FILE_NAME );

/////////////////////////////////////////////////////////////////////////////
// CApeLaunchPadApp

BEGIN_MESSAGE_MAP(CApeLaunchPadApp, CWinApp)
	//{{AFX_MSG_MAP(CApeLaunchPadApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApeLaunchPadApp construction

CApeLaunchPadApp::CApeLaunchPadApp() {
	ApeLaunchPad_pszPropName = APP_NAMES_APE_APP_LAUNCHER;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CApeLaunchPadApp object

CApeLaunchPadApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CApeLaunchPadApp initialization

BOOL CApeLaunchPadApp::InitInstance() {

	AfxEnableControlContainer();

	// Standard initialization
	BOOL bLauncherBusy = TRUE;

	// see if we should copy our command line param into our shared mem-mapped file
	if( ApeLaunchPad_SharedAppData.m_bCreated ||
		!ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_APP_LAUNCHER].bDataInUse ) {
		// copy this applications command line param into our string data
		fclib_strncpy( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_APP_LAUNCHER].szString,
					   m_lpCmdLine, APP_STRING_LEN );
		// mark the slot as used
		ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_APP_LAUNCHER].bDataInUse = TRUE;

		bLauncherBusy = FALSE;
	}

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, ApeLaunchPad_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), ApeLaunchPad_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				
				// post a message to the other window
				if( bLauncherBusy ) {
					pPrevWnd->PostMessage( APP_MSG_CODES_LAUNCHER_ALREADY_BUSY );
				} else {
					pPrevWnd->PostMessage( APP_MSG_CODES_LAUNCHER_2ND_APP_RUN );
				}

				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "ApeAppLauncher is already running.\nIt could not be found to activate it, however.", "App Already Running", MB_OK );
		return FALSE;
	}
	
	CApeLaunchPadDlg dlg;
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

int CApeLaunchPadApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
