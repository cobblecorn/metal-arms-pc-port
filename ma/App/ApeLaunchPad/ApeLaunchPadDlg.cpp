// ApeLaunchPadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ApeLaunchPad.h"
#include "ApeLaunchPadDlg.h"
#include "fang.h"
#include "Settings.h"
#include "fversion.h"
#include "InterProcessData.h"
#include "fclib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _MAX_LOG_LINES						100
#define _MSECS_TO_WAIT_FOR_APP_TO_EXECUTE	1234

// uses an undocumented windows call to bring windows to the foreground under win2k
//typedef void (WINAPI *PROCSWITCHTOTHISWINDOW)( HWND, BOOL );
//PROCSWITCHTOTHISWINDOW SwitchToThisWindow;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApeLaunchPadDlg dialog

CApeLaunchPadDlg::CApeLaunchPadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CApeLaunchPadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApeLaunchPadDlg)
	m_sFontomaticLoc = _T("");
	m_sJawsLoc = _T("");
	m_sK9Loc = _T("");
	m_sPasmLoc = _T("");
	m_sVersion = _T("");
	m_sCopyGoLoc = _T("");
	m_sMawinLoc = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// uses an undocumented windows call to bring windows to the foreground under win2k
//	HMODULE hUser32 = GetModuleHandle("user32");
//	SwitchToThisWindow = ( PROCSWITCHTOTHISWINDOW )GetProcAddress( hUser32, "SwitchToThisWindow" );
}

void CApeLaunchPadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApeLaunchPadDlg)
	DDX_Control(pDX, IDC_LIST, m_ctrlLogList);
	DDX_Text(pDX, IDC_FONTOMATIC_LOC, m_sFontomaticLoc);
	DDX_Text(pDX, IDC_JAWS_LOC, m_sJawsLoc);
	DDX_Text(pDX, IDC_K9_LOC, m_sK9Loc);
	DDX_Text(pDX, IDC_PASM_LOC, m_sPasmLoc);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Text(pDX, IDC_COPYGO_LOC, m_sCopyGoLoc);
	DDX_Text(pDX, IDC_MAWIN_LOC, m_sMawinLoc);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CApeLaunchPadDlg, CDialog)
	//{{AFX_MSG_MAP(CApeLaunchPadDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PICK_FONTOMATIC_LOC, OnButtonPickFontomaticLoc)
	ON_BN_CLICKED(IDC_BUTTON_PICK_JAWS_LOC, OnButtonPickJawsLoc)
	ON_BN_CLICKED(IDC_BUTTON_PICK_K9_LOC, OnButtonPickK9Loc)
	ON_BN_CLICKED(IDC_BUTTON_PICK_PASM_LOC, OnButtonPickPasmLoc)
	ON_BN_CLICKED(IDC_BUTTON_RUN_FONTOMATIC, OnButtonRunFontomatic)
	ON_BN_CLICKED(IDC_BUTTON_RUN_JAWS, OnButtonRunJaws)
	ON_BN_CLICKED(IDC_BUTTON_RUN_K9, OnButtonRunK9)
	ON_BN_CLICKED(IDC_BUTTON_RUN_PASM, OnButtonRunPasm)
	ON_BN_CLICKED(IDC_BUTTON_PICK_COPYGO_LOC, OnButtonPickCopyGoLoc)
	ON_BN_CLICKED(IDC_BUTTON_RUN_COPYGO, OnButtonRunCopyGo)
	ON_BN_CLICKED(IDC_BUTTON_PICK_MAWIN_LOC, OnButtonPickMawinLoc)
	ON_BN_CLICKED(IDC_BUTTON_RUN_MAWIN, OnButtonRunMawin)
	ON_MESSAGE(APP_MSG_CODES_LAUNCHER_2ND_APP_RUN, OnMsgThat2ndAppWasRun)
	ON_MESSAGE(APP_MSG_CODES_LAUNCHER_ALREADY_BUSY, OnMsgThatLauncherWasBusy)
	ON_MESSAGE(APP_MSG_CODES_LAUNCHER_PASM_DONE, OnMsgThatPasmIsDone)
	ON_BN_CLICKED(IDC_BUTTON_INFO, OnButtonInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApeLaunchPadDlg message handlers

BOOL CApeLaunchPadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	::SetProp( GetSafeHwnd(), ApeLaunchPad_pszPropName, (HANDLE)1 ); 

	CSettings::SetSettingsFilename( "ApeLaunchPad.ini" );

	// get the default values from our settings file
	CSettings& Settings = CSettings::GetCurrent();
	CSettings::SetApplicationName( "ApeLaunchPad" );

	m_sFontomaticLoc = Settings.GetFontomaticLoc();
	m_sJawsLoc = Settings.GetJawsLoc();
	m_sK9Loc = Settings.GetK9Loc();
	m_sPasmLoc = Settings.GetPasmLoc();
	m_sCopyGoLoc = Settings.GetCopyGoLoc();
	m_sMawinLoc = Settings.GetMawinLoc();
	
	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );

	m_ctrlLogList.ResetContent();
	AddStringToLog( "Welcome to the Ape Launch Pad!", FALSE );

	RunPasm();

	UpdateData( VARS_TO_CONTROLS );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CApeLaunchPadDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CApeLaunchPadDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CApeLaunchPadDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CApeLaunchPadDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );

	// mark our data as unused
	ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_APP_LAUNCHER].bDataInUse = FALSE;

	CSettings& Settings = CSettings::GetCurrent();
	
	Settings.m_sFontomaticLoc = m_sFontomaticLoc;
	Settings.m_sJawsLoc = m_sJawsLoc;
	Settings.m_sK9Loc = m_sK9Loc;
	Settings.m_sPasmLoc = m_sPasmLoc;
	Settings.m_sCopyGoLoc = m_sCopyGoLoc;
	Settings.m_sMawinLoc = m_sMawinLoc;

	Settings.SaveCommonDataOutToFile();
	
	::RemoveProp( GetSafeHwnd(), ApeLaunchPad_pszPropName );

	CDialog::OnCancel();
}

void CApeLaunchPadDlg::OnOK() {
	//OnCancel();
}

BOOL CApeLaunchPadDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CApeLaunchPadDlg::OnButtonPickCopyGoLoc() {
	
	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".exe",
					 m_sCopyGoLoc,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Exe Files (*.exe)|*.exe||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select CopyGo File";
	if( dlg.DoModal() == IDOK ) {
		m_sCopyGoLoc = dlg.GetPathName();
		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CApeLaunchPadDlg::OnButtonPickFontomaticLoc() {
	
	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".exe",
					 m_sFontomaticLoc,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Exe Files (*.exe)|*.exe|Copy Go Files (*.cr)|*.cr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Fontomatic App File";
	if( dlg.DoModal() == IDOK ) {
		m_sFontomaticLoc = dlg.GetPathName();
		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CApeLaunchPadDlg::OnButtonPickJawsLoc() {
	
	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".exe",
					 m_sJawsLoc,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Exe Files (*.exe)|*.exe|Copy Go Files (*.cr)|*.cr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Jaws App File";
	if( dlg.DoModal() == IDOK ) {
		m_sJawsLoc = dlg.GetPathName();
		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CApeLaunchPadDlg::OnButtonPickK9Loc() {
	
	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".exe",
					 m_sK9Loc,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Exe Files (*.exe)|*.exe|Copy Go Files (*.cr)|*.cr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select K9 App File";
	if( dlg.DoModal() == IDOK ) {
		m_sK9Loc = dlg.GetPathName();
		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CApeLaunchPadDlg::OnButtonPickPasmLoc() {
	
	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".exe",
					 m_sPasmLoc,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Exe Files (*.exe)|*.exe|Copy Go Files (*.cr)|*.cr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select PASM App File";
	if( dlg.DoModal() == IDOK ) {
		m_sPasmLoc = dlg.GetPathName();
		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CApeLaunchPadDlg::OnButtonPickMawinLoc() {

	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".exe",
					 m_sMawinLoc,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Exe Files (*.exe)|*.exe|Copy Go Files (*.cr)|*.cr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Mawin App File";
	if( dlg.DoModal() == IDOK ) {
		m_sMawinLoc = dlg.GetPathName();
		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CApeLaunchPadDlg::OnButtonRunFontomatic() {
	BOOL bMinimize;

	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Attempting to run Fontomatic...", TRUE );

	if( !RunAFile( m_sFontomaticLoc ) ) {
		bMinimize = FALSE;
		AddStringToLog( "Cannot run Fontomatic. Make sure the path is set correctly." );
	} else {
		bMinimize = TRUE;
		AddStringToLog( "Fontomatic launched OK." );
	}
	if( bMinimize ) {
		ShowWindow( SW_MINIMIZE );
	}
}

void CApeLaunchPadDlg::OnButtonRunJaws() {
	BOOL bMinimize;

	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Attempting to run Jaws..." );
	
	if( !RunAFile( m_sJawsLoc ) ) {
		bMinimize = FALSE;
		AddStringToLog( "Cannot run Jaws. Make sure the path is set correctly." );
	} else {
		bMinimize = TRUE;
		AddStringToLog( "Jaws launched OK." );
	}

	if( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].bDataInUse ) {
		// allow Jaws time to execute
		Sleep( _MSECS_TO_WAIT_FOR_APP_TO_EXECUTE );
		// send a message to launch the new file
		SendLaunchMsgToJaws();
		AddStringToLog( " " );

		ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].bDataInUse = FALSE;
	}
	if( bMinimize ) {
		ShowWindow( SW_MINIMIZE );
	}
}

void CApeLaunchPadDlg::OnButtonRunK9() {
	BOOL bMinimize;

	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Attempting to run K9..." );
	
	if( !RunAFile( m_sK9Loc ) ) {
		bMinimize = FALSE;
		AddStringToLog( "Cannot run K9. Make sure the path is set correctly." );
	} else {
		bMinimize = TRUE;
		AddStringToLog( "K9 launched OK." );
	}

	if( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_K9].bDataInUse ) {
		// allow K9 time to execute
		Sleep( _MSECS_TO_WAIT_FOR_APP_TO_EXECUTE );
		// send a message to launch the new file
		SendLaunchMsgToK9();
		AddStringToLog( " " );
		
		ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_K9].bDataInUse = FALSE;		
	}
	if( bMinimize ) {
		ShowWindow( SW_MINIMIZE );
	}
}

void CApeLaunchPadDlg::OnButtonRunPasm() {
	BOOL bMinimize;
	
	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Attempting to run PASM..." );
	
	if( !RunAFile( m_sPasmLoc ) ) {
		bMinimize = FALSE;
		AddStringToLog( "Cannot run PASM. Make sure the path is set correctly." );
	} else {
		bMinimize = TRUE;
		AddStringToLog( "PASM launched OK." );
	}

	if( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_PASM].bDataInUse ) {
		// allow PASM time to execute
		Sleep( _MSECS_TO_WAIT_FOR_APP_TO_EXECUTE );
		// send a message to rescan & compile local files
		SendRescanAndCompileMsgToPASM();
	}
	if( bMinimize ) {
		ShowWindow( SW_MINIMIZE );
	}
}

void CApeLaunchPadDlg::OnButtonRunCopyGo() {
	BOOL bMinimize;

	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Attempting to run CopyGo..." );

	if( !RunAFile( m_sCopyGoLoc ) ) {
		bMinimize = FALSE;
		AddStringToLog( "Cannot run CopyGo. Make sure the path is set correctly." );
	} else {
		bMinimize = TRUE;
		AddStringToLog( "CopyGo launched OK." );
	}
	if( bMinimize ) {
		ShowWindow( SW_MINIMIZE );
	}
}

void CApeLaunchPadDlg::OnButtonRunMawin() {
	BOOL bMinimize;

	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Attempting to run Mawin..." );

	if( !RunAFile( m_sMawinLoc ) ) {
		bMinimize = FALSE;
		AddStringToLog( "Cannot run Mawin. Make sure the path is set correctly." );
	} else {
		bMinimize = TRUE;
		AddStringToLog( "Mawin launched OK." );
	}
	if( bMinimize ) {
		ShowWindow( SW_MINIMIZE );
	}
}

BOOL CApeLaunchPadDlg::GetPathFromString( const CString &rsFullFilename, CString &rsPath ) const {
	
	if( rsFullFilename.IsEmpty() ) {
		return FALSE;
	}
	int nSlashIndex;
	nSlashIndex = rsFullFilename.ReverseFind( '\\' );
	if( nSlashIndex < 0 ) {
		return FALSE;
	}
	
	rsPath = rsFullFilename.Mid( 0, nSlashIndex );
	
	return TRUE;
}

BOOL CApeLaunchPadDlg::IsFileACopyGoFile( const CString &rsFilename ) const {

	if( rsFilename.IsEmpty() ) {
		return FALSE;
	}
	int nPeriodIndex = rsFilename.ReverseFind( '.' );
	if( nPeriodIndex < 0 ) {
		return FALSE;
	}
	CString sExt = rsFilename.Mid( nPeriodIndex );
	if( sExt.CompareNoCase( ".cr" ) == 0 ) {
		return TRUE;
	}
	return FALSE;
}

BOOL CApeLaunchPadDlg::RunAFile( const CString &rsFilename ) {
	int nRetVal;

	if( rsFilename.IsEmpty() ) {
		return FALSE;
	}

	if( IsFileACopyGoFile( rsFilename ) ) {
		if( m_sCopyGoLoc.IsEmpty() ) {
			AddStringToLog( "Cannot run a CopyGo (*.cr) file until you pick the CopyGo application location." );
			return FALSE;
		} else {
			CString sParam;
			sParam.Format( "/s %s", rsFilename );
			nRetVal = (int)ShellExecute( NULL, "open", m_sCopyGoLoc, sParam, NULL, SW_SHOWNORMAL );	
		}
	} else {
		nRetVal = (int)ShellExecute( NULL, "open", rsFilename, NULL, NULL, SW_SHOWNORMAL );	
	}
	
	return (nRetVal > 32);
}

long CApeLaunchPadDlg::OnMsgThat2ndAppWasRun( WPARAM wParam, LPARAM lParam ) {
	
	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Another instance was run and the launcher is idle." );
	
	RunPasm();
	
	return 0;
}

long CApeLaunchPadDlg::OnMsgThatLauncherWasBusy( WPARAM wParam, LPARAM lParam ) {
	
	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "Another instance was run, but the launcher is busy." );
	
	return 0;
}

void CApeLaunchPadDlg::RunPasm() {

	// print out the command line param
	AddStringToLog( " " );
	CString s;
	s.Format( "Command Line Parameter = '%s'", ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_APP_LAUNCHER].szString );
	AddStringToLog( s );

	if( !m_sPasmLoc.IsEmpty() ) {
		// go ahead and launch PASM, all commands need it
		PostMessage( WM_COMMAND, IDC_BUTTON_RUN_PASM, 0 );

		// mark the PASM data as in use so that PASM knows to send us a message back
		ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_PASM].bDataInUse = TRUE;
	}	
}

BOOL CApeLaunchPadDlg::SendRescanAndCompileMsgToPASM() {

	// find the PASM app
	CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
	while( pPrevWnd ) {
		if( ::GetProp( pPrevWnd->GetSafeHwnd(), APP_NAMES_PASM )) {
//			SwitchToThisWindow( pPrevWnd->m_hWnd, TRUE ); 
			
			// post a message to PASM
			pPrevWnd->PostMessage( APP_MSG_CODES_PASM_RESCAN_AND_COMPILE_LOCAL );

			AddStringToLog( "Commanding PASM to 'Rescan & Compile Local Files'." );
			
			return TRUE;
		}
		// grab the next window
		pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
	}
	// if we got here we couldn't find a PASM app running
	return FALSE;
}

BOOL CApeLaunchPadDlg::SendLaunchMsgToK9() {
	
	// find the K9 app
	CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
	while( pPrevWnd ) {
		if( ::GetProp( pPrevWnd->GetSafeHwnd(), APP_NAMES_K9 )) {
//			SwitchToThisWindow( pPrevWnd->m_hWnd, TRUE );
			
			// post a message to K9
			pPrevWnd->PostMessage( APP_MSG_CODES_K9_LAUNCH );

			AddStringToLog( "Commanding K9 to 'Launch' the passed in world file." );
			
			return TRUE;
		}
		// grab the next window
		pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
	}
	// if we got here we couldn't find a K9 app running
	return FALSE;
}

BOOL CApeLaunchPadDlg::SendLaunchMsgToJaws() {
	
	// find the Jaws app
	CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
	while( pPrevWnd ) {
		if( ::GetProp( pPrevWnd->GetSafeHwnd(), APP_NAMES_JAWS )) {
//			SwitchToThisWindow( pPrevWnd->m_hWnd, TRUE );
			
			// post a message to Jaws
			pPrevWnd->PostMessage( APP_MSG_CODES_JAWS_LAUNCH );

			AddStringToLog( "Commanding Jaws to 'Launch' the passed in files." );
			
			return TRUE;
		}
		// grab the next window
		pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
	}
	// if we got here we couldn't find a Jaws app running
	return FALSE;
}

long CApeLaunchPadDlg::OnMsgThatPasmIsDone( WPARAM wParam, LPARAM lParam ) {
	CString sWldFile, sMtxFile1, sMtxFile2, sApeFile, sPackage, sLog;
	u32 nNumMtxFiles;
	
	UpdateData( CONTROLS_TO_VARS );

	AddStringToLog( "PASM has finished, parsing command line..." );

	// figure out what app to run
	CString sParam = ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_APP_LAUNCHER].szString;
	if( !sParam.IsEmpty() ) {
		// not empty
		sParam.MakeLower();
		if( sParam.Find( "/k9" ) >= 0 ) {

			////////////
			// launch k9
			PostMessage( WM_COMMAND, IDC_BUTTON_RUN_K9, 0 );
			
			// parse the cmd line for the filename
			if( ExtractWorldFilename( sParam, sWldFile ) ) {
				// package the filename for k9
				ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_K9].bDataInUse = TRUE;
				fclib_strncpy( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_K9].szString, sWldFile, APP_STRING_LEN );
				ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_K9].nNumStrings = 1;

				sLog.Format( "Found the world file = '%s'", sWldFile );
				AddStringToLog( sLog );
			}
			
		} else if( sParam.Find( "/jaws" ) >= 0 ) {
			
			//////////////
			// launch jaws
			PostMessage( WM_COMMAND, IDC_BUTTON_RUN_JAWS, 0 );

			// parse the cmd line for the filename
			if( ExtractWorldFilename( sParam, sWldFile ) ) {
				// found a .wld file

				// package the filename for jaws
				ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].bDataInUse = TRUE;
				fclib_strncpy( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].szString, sWldFile, APP_STRING_LEN );
				ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].nNumStrings = 1;
				
				sLog.Format( "Found the world file = '%s'", sWldFile );
				AddStringToLog( sLog );

			} else {
				// search for a .ape file
				if( ExtractApeFilename( sParam, sApeFile ) ) {
					// found a .ape file
					
					// package the filename for jaws
					ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].bDataInUse = TRUE;
					sPackage = sApeFile;				
					ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].nNumStrings = 1;

					sLog.Format( "Found the ape file = '%s'", sApeFile );
					AddStringToLog( sLog );
					
					// see if there are any .mtx files
					nNumMtxFiles = ExtractMtxFilenames( sParam, sMtxFile1, sMtxFile2 );
					switch( nNumMtxFiles ) {
					case 1:
						sPackage += ':';
						sPackage += sMtxFile1;
						ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].nNumStrings = 2;

						sLog.Format( "Found the animation file = '%s'", sMtxFile1 );
						AddStringToLog( sLog );
						break;
					case 2:
						sPackage += ':';
						sPackage += sMtxFile1;
						sPackage += ':';
						sPackage += sMtxFile2;
						ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].nNumStrings = 3;

						sLog.Format( "Found the animation file = '%s'", sMtxFile1 );
						AddStringToLog( sLog );
						sLog.Format( "Found the animation file = '%s'", sMtxFile2 );
						AddStringToLog( sLog );
						break;
					}
					fclib_strncpy( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_JAWS].szString, sPackage, APP_STRING_LEN );					
				}
			}

		} else if( sParam.Find( "/mawin" ) >= 0 ) {

			///////////////
			// launch mawin
			PostMessage( WM_COMMAND, IDC_BUTTON_RUN_MAWIN, 0 );

			// parse the cmd line for the filename
			if( ExtractWorldFilename( sParam, sWldFile ) ) {
				// package the filename for mawin
				ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_MAWIN].bDataInUse = TRUE;
				fclib_strncpy( ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_MAWIN].szString, sWldFile, APP_STRING_LEN );
				ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_MAWIN].nNumStrings = 1;	
				
				sLog.Format( "Found the world file = '%s'", sWldFile );
				AddStringToLog( sLog );
			}

		} else if( sParam.Find( "/fontomatic" ) >= 0 ) {
			// launch fontomatic
			PostMessage( WM_COMMAND, IDC_BUTTON_RUN_FONTOMATIC, 0 );
		} else {
			// unknown app
			AddStringToLog( "Unknown command line parameters, don't know which app to launch." );
		}
	} else {
		AddStringToLog( "No command line passed in, don't know which app to launch." );
	}
	// mark the launcher as available
	ApeLaunchPad_SharedAppData->aAppData[APP_TYPE_APP_LAUNCHER].bDataInUse = FALSE;

	return 0;
}

void CApeLaunchPadDlg::AddStringToLog( cchar *pszString, BOOL bEnsureVisible/*=TRUE*/ ) {

	int nIndex = m_ctrlLogList.AddString( pszString );

	// make sure that there are not more than _MAX_LOG_LINES lines viewable
	int nCount = m_ctrlLogList.GetCount();
	if( nCount > _MAX_LOG_LINES ) {
		int nNumToDelete = nCount - _MAX_LOG_LINES;
		int i;
		for( i=0; i < nNumToDelete; i++ ) {
			m_ctrlLogList.DeleteString( 0 );
		}
		// fixup the index of the element just added
		nIndex -= nNumToDelete;
	}
	if( bEnsureVisible ) {
		m_ctrlLogList.SetCurSel( nIndex );	
	}
}

BOOL CApeLaunchPadDlg::ExtractWorldFilename( const CString &rsCmdParam, CString &rsFilename ) {

	CString sTemp;
	sTemp = rsCmdParam;
	sTemp.MakeLower();

	// find the first ':'
	int nIndex = sTemp.Find( ':' );
	while( !sTemp.IsEmpty() && nIndex >= 0 ) {
		// found a ':', delete everything up to it
		sTemp.Delete( 0, nIndex+1 );
		// find the first file extension
		nIndex = sTemp.Find( '.' );
		if( nIndex > 0 ) {
			// see if we have the correct file extension
			if( sTemp[nIndex+1] == 'w' &&
				sTemp[nIndex+2] == 'l' &&
				sTemp[nIndex+3] == 'd' ) {
				// copy the letters upto the end of the extension
				rsFilename = sTemp.Mid( 0, nIndex+4 );
				// make sure that the filename doesn't contain invalid characters
				if( DoesFilenameContainInvalidChars( rsFilename ) ) {
					sTemp.Format( "Error: the filename '%s' contains invalid characters.", rsFilename );
					AddStringToLog( sTemp );
					return FALSE;
				}
				return TRUE;
			}
		}
		nIndex = sTemp.Find( ':' );
	}
	return FALSE;
}


BOOL CApeLaunchPadDlg::ExtractApeFilename( const CString &rsCmdParam, CString &rsFilename ) {

	CString sTemp;
	sTemp = rsCmdParam;
	sTemp.MakeLower();

	// find the first ':'
	int nIndex = sTemp.Find( ':' );
	while( !sTemp.IsEmpty() && nIndex >= 0 ) {
		// found a ':', delete everything up to it
		sTemp.Delete( 0, nIndex+1 );
		// find the first file extension
		nIndex = sTemp.Find( '.' );
		if( nIndex > 0 ) {
			// see if we have the correct file extension
			if( sTemp[nIndex+1] == 'a' &&
				sTemp[nIndex+2] == 'p' &&
				sTemp[nIndex+3] == 'e' ) {	
				// copy the letters upto the end of the extension
				rsFilename = sTemp.Mid( 0, nIndex+4 );
				// make sure that the filename doesn't contain invalid characters
				if( DoesFilenameContainInvalidChars( rsFilename ) ) {
					sTemp.Format( "Error: the filename '%s' contains invalid characters.", rsFilename );
					AddStringToLog( sTemp );
					return FALSE;
				}
				return TRUE;
			}
		}
		nIndex = sTemp.Find( ':' );
	}
	return FALSE;
}

// returns the number of valid files found
u32 CApeLaunchPadDlg::ExtractMtxFilenames( const CString &rsCmdParam, CString &rsFilename1, CString &rsFilename2 ) {
	u32 nFileNum = 0;
	
	CString sTemp;
	sTemp = rsCmdParam;
	sTemp.MakeLower();

	// find the first ':'
	int nIndex = sTemp.Find( ':' );
	while( !sTemp.IsEmpty() && nIndex >= 0 ) {
		// found a ':', delete everything up to it
		sTemp.Delete( 0, nIndex+1 );
		// find the first file extension
		nIndex = sTemp.Find( '.' );
		if( nIndex > 0 ) {
			// see if we have the correct file extension
			if( sTemp[nIndex+1] == 'm' &&
				sTemp[nIndex+2] == 't' &&
				sTemp[nIndex+3] == 'x' ) {	
				// copy the letters upto the end of the extension
				if( nFileNum == 0 ) {
					// the 1st filename
					rsFilename1 = sTemp.Mid( 0, nIndex+4 );

					// make sure that the filename doesn't contain invalid characters
					if( DoesFilenameContainInvalidChars( rsFilename1 ) ) {
						sTemp.Format( "Error: the filename '%s' contains invalid characters.", rsFilename1 );
						AddStringToLog( sTemp );
						return 0;
					}
					nFileNum++;
				} else {
					// the 2nd filename
					rsFilename2 = sTemp.Mid( 0, nIndex+4 );
					// make sure that the filename doesn't contain invalid characters
					if( DoesFilenameContainInvalidChars( rsFilename2 ) ) {
						sTemp.Format( "Error: the filename '%s' contains invalid characters.", rsFilename2 );
						AddStringToLog( sTemp );
						return 0;
					}
					return 2;		
				}
			}
		}
		nIndex = sTemp.Find( ':' );
	}
	return nFileNum;
}

BOOL CApeLaunchPadDlg::DoesFilenameContainInvalidChars( const CString &rsFilename ) {

	return ( rsFilename.FindOneOf( "\\/:*?\"<>|" ) > 0 );
}

void CApeLaunchPadDlg::OnButtonInfo() {
	CString sMsg;

	sMsg =  "The Ape Launch Pad is a utility that will launch Swingin' Ape Tools.       \n";
	sMsg += "To use, first setup the path to each tool, you can select either CopyGo    \n";
	sMsg += "or EXE files.  Then run the Launch Pad, passing in command line parameters.\n";
	sMsg += "It will run PASM, rescan & compile local file, and then launch the requested\n";
	sMsg += "application, running the optionally supplied file(s).                       \n\n";
	sMsg += "Here are the command line parameters:\n";
	sMsg += "( All files should contain their extension and are seperated by a colon ':' )\n\n";
	sMsg += "/k9:wld_filename\n";
	sMsg += "/jaws:wld_or_ape_filename:animation_file1:animation_file2\n";
	sMsg += "/mawin:wld_filename\n";
	sMsg += "/fontomatic:tga_or_fnt_filename\n\n";

	this->MessageBox( sMsg, _T("Ape Launch Pad"), MB_ICONINFORMATION|MB_OK|MB_DEFBUTTON1);
}
