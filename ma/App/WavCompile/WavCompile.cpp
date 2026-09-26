// WavCompile.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WavCompile.h"
#include "WavCompileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

cchar *WavCompile_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CWavCompileApp

BEGIN_MESSAGE_MAP(CWavCompileApp, CWinApp)
	//{{AFX_MSG_MAP(CWavCompileApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWavCompileApp construction

CWavCompileApp::CWavCompileApp() {
	WavCompile_pszPropName = APP_NAMES_WAV_COMPILER;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWavCompileApp object

CWavCompileApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWavCompileApp initialization

BOOL CWavCompileApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, WavCompile_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), WavCompile_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "WavCompiler is already running.\nIt could not be found to activate it, however.", "WavCompiler Already Running", MB_OK );
		return FALSE;
	}

	CWavCompileDlg dlg;
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

int CWavCompileApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
