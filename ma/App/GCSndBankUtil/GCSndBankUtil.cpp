// GCSndBankUtil.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GCSndBankUtil.h"
#include "GCSndBankUtilDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

cchar *GCSndBankUtil_pszPropName;

/////////////////////////////////////////////////////////////////////////////
// CGCSndBankUtilApp

BEGIN_MESSAGE_MAP(CGCSndBankUtilApp, CWinApp)
	//{{AFX_MSG_MAP(CGCSndBankUtilApp)
	//}}AFX_MSG
	//ON_COMMAND(ID_HELP, CWinApp::OnHelp) removed to disable F1 trying to bring up help file
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCSndBankUtilApp construction

CGCSndBankUtilApp::CGCSndBankUtilApp() {
	GCSndBankUtil_pszPropName = APP_NAMES_GC_BANK_CONVERTER;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGCSndBankUtilApp object

CGCSndBankUtilApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGCSndBankUtilApp initialization

BOOL CGCSndBankUtilApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

	// limit this app to 1 instance, if it is already running bring it forward
	m_hMutex = ::CreateMutex( NULL, TRUE, GCSndBankUtil_pszPropName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
		while( pPrevWnd ) {
			if( ::GetProp( pPrevWnd->GetSafeHwnd(), GCSndBankUtil_pszPropName )) {
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

	CGCSndBankUtilDlg dlg;
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

int CGCSndBankUtilApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}

