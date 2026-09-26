// ma_win.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ma_win.h"
#include "ma_winDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// public vars
cchar *Mawin_pszExeName;
BOOL Mawin_bAutoRun;
char Mawin_pszFullAppPath[256];

/////////////////////////////////////////////////////////////////////////////
// CMa_winApp

BEGIN_MESSAGE_MAP(CMa_winApp, CWinApp)
	//{{AFX_MSG_MAP(CMa_winApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up the help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMa_winApp construction

CMa_winApp::CMa_winApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMa_winApp object

CMa_winApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMa_winApp initialization

BOOL CMa_winApp::InitInstance()
{
//	afxMemDF |= (checkAlwaysMemDF);

	AfxEnableControlContainer();

	// Standard initialization
	Mawin_pszExeName = m_pszExeName;
	Mawin_bAutoRun = FALSE;

	// grab the full module filename
	GetModuleFileName( NULL, Mawin_pszFullAppPath, 256 ); 

	// see if the autorun parameter has been passed in
	CString sAutoRun = "autorun";
	CString sCmdLine = m_lpCmdLine;
	sCmdLine.MakeLower();
	if( sCmdLine.Find( sAutoRun ) != -1 ) {
		Mawin_bAutoRun = TRUE;
	}

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, Mawin_pszExeName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), Mawin_pszExeName )) {
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

	CMa_winDlg dlg;
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

int CMa_winApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
