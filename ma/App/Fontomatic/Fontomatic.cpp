// Fontomatic.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fang.h"
#include "Fontomatic.h"
#include "FontomaticDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR Fontomatic_pszExeName;

/////////////////////////////////////////////////////////////////////////////
// CFontomaticApp

BEGIN_MESSAGE_MAP(CFontomaticApp, CWinApp)
	//{{AFX_MSG_MAP(CFontomaticApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontomaticApp construction

CFontomaticApp::CFontomaticApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFontomaticApp object

CFontomaticApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFontomaticApp initialization

BOOL CFontomaticApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	Fontomatic_pszExeName = m_pszExeName;
	// make sure that only one instance of this app runs at a time
	m_hMutex = ::CreateMutex( NULL, TRUE, Fontomatic_pszExeName );
	if( GetLastError() == ERROR_ALREADY_EXISTS ) {
		CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow(GW_CHILD);
		while( pPrevWnd ) {
			if( ::GetProp(pPrevWnd->GetSafeHwnd(), Fontomatic_pszExeName ) ) {
				if( pPrevWnd->IsIconic() ) {
					pPrevWnd->ShowWindow( SW_RESTORE );
				}
				pPrevWnd->SetForegroundWindow();
				pPrevWnd->GetLastActivePopup()->SetForegroundWindow();
				return FALSE;
			}

			pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
		}

		::MessageBox( NULL, _T( "Fontomatic is already running.\nIt could not be found to activate it, however." ), _T( "Fontomatic Running" ), MB_OK );
		return FALSE;
	}

	CFontomaticDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CFontomaticApp::ExitInstance() {
	CloseHandle( m_hMutex );
	return CWinApp::ExitInstance();
}