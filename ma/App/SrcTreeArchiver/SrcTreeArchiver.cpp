// SrcTreeArchiver.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SrcTreeArchiver.h"
#include "SrcTreeArchiverDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// public vars
const char *SrcTreeArchiver_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CSrcTreeArchiverApp

BEGIN_MESSAGE_MAP(CSrcTreeArchiverApp, CWinApp)
	//{{AFX_MSG_MAP(CSrcTreeArchiverApp)
	//}}AFX_MSG
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up the help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrcTreeArchiverApp construction

CSrcTreeArchiverApp::CSrcTreeArchiverApp() {
	SrcTreeArchiver_pszPropName = "SourceTreeArchiver - C8C30735-1A02-42dd-AAD0-A671C0FF9309";
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSrcTreeArchiverApp object

CSrcTreeArchiverApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSrcTreeArchiverApp initialization

BOOL CSrcTreeArchiverApp::InitInstance()
{
	AfxEnableControlContainer();

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, SrcTreeArchiver_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), SrcTreeArchiver_pszPropName )) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return FALSE;
			}
			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}
		::MessageBox( NULL, "SrcTreeArchiver is already running.\nIt could not be found to activate it, however.", "SrcTreeArchiver Already Running", MB_OK );
		return FALSE;
	}

	CSrcTreeArchiverDlg dlg;
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

int CSrcTreeArchiverApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}
