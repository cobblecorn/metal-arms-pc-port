// ma_winDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ma_win.h"
#include "ma_winDlg.h"
#include "Settings.h"
#include "fang.h"
#include "pickdir.h"
#include "VidMode.h"
#include "floop.h"
#include "fversion.h"
#include "pickasset.h"
#include "ffile.h"
#include "screenshot.h"
#include "gameloop.h"
#include "inputemulation.h"
#include "bot.h"
#include "ai/aibrain.h"
#include "fRenderSort.h"
#include "flight.h"
#include "Game.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _MASTER_MOVIE_PATH		"C:\\art\\MOVIES\\"

// color conversion (windows to game)
#define _COLORRGB_TO_COLORREF( pColorRGB ) ( (u32)((pColorRGB)->fRed * 255.0f) | ((u32)((pColorRGB)->fGreen * 255.0f) << 8) | ((u32)((pColorRGB)->fBlue * 255.0f) << 16) )
#define _COLORREF_TO_COLORRGB( pColorRGB, nColorRef ) (pColorRGB)->Set( (f32)((nColorRef) & 0xff) * (1.0f/255.0f), (f32)(((nColorRef)>>8) & 0xff) * (1.0f/255.0f), (f32)(((nColorRef)>>16) & 0xff) * (1.0f/255.0f) )

#define _GAMELOOP_EXIT		(WM_USER + 300)

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
// CMa_winDlg dialog

CMa_winDlg::CMa_winDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMa_winDlg::IDD, pParent) {
	//{{AFX_DATA_INIT(CMa_winDlg)
	m_sMasterFile = _T("");
	m_sInputFilename = _T("");
	m_sScreenshotDir = _T("");
	m_sVersion = _T("");
	m_bSkipLevelSelect = FALSE;
	m_bDebugAI = FALSE;
	m_bPlayerDeath = TRUE;
	m_sMemcardDir = _T("");
	m_bShowEntityInfo = FALSE;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bFangOK = FALSE;
	m_bValidVideoMode = FALSE;
	m_bRunning = FALSE;
	m_pMainDialog = this;

	m_bDebugAI = TRUE;
	m_bPlayerDeath = TRUE;
}

void CMa_winDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMa_winDlg)
	DDX_Text(pDX, IDC_MASTER_FILE, m_sMasterFile);
	DDX_Text(pDX, IDC_INPUT_NAME, m_sInputFilename);
	DDX_Text(pDX, IDC_SCREENSHOT_DIR, m_sScreenshotDir);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Check(pDX, IDC_SKIP_LEVEL_SELECT, m_bSkipLevelSelect);
	DDX_Check(pDX, IDC_CBDEBUG_AI, 	m_bDebugAI);
	DDX_Check(pDX, IDC_CBPLAYER_DEATH, 	m_bPlayerDeath);
	DDX_Text(pDX, IDC_MEMCARD_DIR, m_sMemcardDir);
	DDX_Check(pDX, IDC_CHECK_SHOW_ENTITY_INFO, m_bShowEntityInfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMa_winDlg, CDialog)
	//{{AFX_MSG_MAP(CMa_winDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_CHOOSE_MASTER_FILE, OnChooseMasterFile)
	ON_BN_CLICKED(IDC_VIDEO_MODE, OnVideoMode)
	ON_BN_CLICKED(IDC_BUTTON_RUN, OnButtonRun)
	ON_BN_CLICKED(IDC_3D_COLOR, On3dColor)
	ON_BN_CLICKED(IDC_CHOOSE_SCREENSHOT_DIR, OnChooseScreenshotDir)
	ON_BN_CLICKED(IDC_SERIES, OnSeries)
	ON_BN_CLICKED(IDC_SINGLE, OnSingle)
	ON_BN_CLICKED(IDC_VIEW_BOUNDS, OnViewBounds)
	ON_BN_CLICKED(IDC_DONT_RENDER_SURFACE, OnDontRenderSurface)
	ON_BN_CLICKED(IDC_DONT_RENDER_SPECULAR, OnDontRenderSpecular)
	ON_BN_CLICKED(IDC_DONT_RENDER_TRANSLUCENCIES, OnDontRenderTranslucencies)
	ON_BN_CLICKED(IDC_CAMERA_POV_VISIBILITY, OnCameraPOVVisibility)
	ON_BN_CLICKED(IDC_CBPLAYER_DEATH, OnPlayerDeath )
	ON_BN_CLICKED(IDC_CBDEBUG_AI, OnDebugAI)
	ON_BN_CLICKED(IDC_CBBDADAI, OnBDADAI)
	ON_BN_CLICKED(IDC_30FPS, On30fps)
	ON_BN_CLICKED(IDC_60FPS, On60fps)
	ON_BN_CLICKED(IDC_REALTIME_FPS, OnRealtimeFps)
	ON_BN_CLICKED(IDC_BUTTON_INPUT_EMULATION, OnInputMapper)
	ON_BN_CLICKED(IDC_JPEG, OnJpegClick)
	ON_BN_CLICKED(IDC_TGA, OnTgaClick)
	ON_BN_CLICKED(IDC_XBOX, OnXbox)
	ON_BN_CLICKED(IDC_GAMECUBE, OnGamecube)
	ON_BN_CLICKED(IDC_DISPLAY_FPS, OnDisplayFps)
	ON_BN_CLICKED(IDC_CHOOSE_MEMCARD_DIR, OnChooseMemcardDir)
	ON_BN_CLICKED(IDC_SLOT1, OnSlot1)
	ON_BN_CLICKED(IDC_SLOT2, OnSlot2)
	ON_BN_CLICKED(IDC_DISPLAY_SAFE_SCREEN_BOX, OnDisplaySafeScreenBox)
	ON_BN_CLICKED(IDC_LEGSERVO_DEFAULT, OnLegServoDefault)
	ON_BN_CLICKED(IDC_LEGSERVO_L1, OnLegServoL1)
	ON_BN_CLICKED(IDC_LEGSERVO_L2, OnLegServoL2)
	ON_BN_CLICKED(IDC_LEGSERVO_L3, OnLegServoL3)
	ON_BN_CLICKED(IDC_ARMSERVO_DEFAULT, OnArmServoDefault)
	ON_BN_CLICKED(IDC_ARMSERVO_L1, OnArmServoL1)
	ON_BN_CLICKED(IDC_ARMSERVO_L2, OnArmServoL2)
	ON_BN_CLICKED(IDC_ARMSERVO_L3, OnArmServoL3)
	ON_MESSAGE(_GAMELOOP_EXIT, OnGameloopExit)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ENTITY_INFO, OnCheckShowEntityInfo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DONT_DRAW_HUD, OnBnClickedDontDrawHud)
	ON_BN_CLICKED(IDC_DONT_DRAW_DEBUG, OnBnClickedDontDrawDebug)
	ON_BN_CLICKED(IDC_LIGHT_SHOWRANGE, OnBnClickedLightShowrange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMa_winDlg message handlers

static CWinPrintf *_pWinPrintf;
CMa_winDlg *CMa_winDlg::m_pMainDialog = NULL;

static void _DevPrintf( cchar *pszFormat, FANG_VA_LIST Args ) {
	_pWinPrintf->Printf( pszFormat, Args );
}

long CMa_winDlg::OnGameloopExit( WPARAM wParam, LPARAM lParam ) {
	
	EnableControls( TRUE );

	m_bRunning = FALSE;

	if( Mawin_bAutoRun ) {
		// if we autoran, go ahead and kill the app
		PostMessage( WM_COMMAND, IDCANCEL, 0 ); 
	}
	return 0;
}

void CMa_winDlg::EnableControls( BOOL bEnable ) {
	
	((CButton *)GetDlgItem( IDC_VIDEO_MODE ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_BUTTON_LOAD))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_BUTTON_INPUT_EMULATION ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_CHOOSE_SCREENSHOT_DIR ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_SKIP_LEVEL_SELECT ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_CHOOSE_MASTER_FILE ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_3D_COLOR ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_XBOX ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_GAMECUBE ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_CHOOSE_MEMCARD_DIR ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_SLOT1 ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_SLOT2 ))->EnableWindow( bEnable );	
	((CButton *)GetDlgItem( IDC_INSTALL_AUDIO ))->EnableWindow( bEnable );	
}

static void _GameloopExitFunction( void ) {
	CMa_winDlg::m_pMainDialog->PostMessage( _GAMELOOP_EXIT );	
}

static void _GameloopMinimizeFunction( void ) {
	CMa_winDlg::m_pMainDialog->SetForegroundWindow();
}


BOOL CMa_winDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//_CrtSetBreakAlloc(104);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if( pSysMenu != NULL ) {
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if( !strAboutMenu.IsEmpty() ) {
			pSysMenu->InsertMenu( 0, MF_STRING, IDM_ABOUTBOX, strAboutMenu );
		//	pSysMenu->AppendMenu(MF_SEPARATOR);
		//	pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		pSysMenu->EnableMenuItem( 2, MF_BYPOSITION | MF_GRAYED );
		pSysMenu->EnableMenuItem( 4, MF_BYPOSITION | MF_GRAYED );
	}
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	::SetProp( GetSafeHwnd(), Mawin_pszExeName, (HANDLE)1 );

	CSettings::SetSettingsFilename( "ma_win.ini" );
	CSettings::SetApplicationName( "ma_win" );

	// get the default values from our settings file
	CSettings& Settings = CSettings::GetCurrent();

	m_sMasterFile = Settings.GetArtPath();
	m_sInputFilename = Settings.GetLastFile();
	m_3DBGColorRGB.fRed = Settings.Get3dBGRed();
	m_3DBGColorRGB.fGreen = Settings.Get3dBGGreen();
	m_3DBGColorRGB.fBlue = Settings.Get3dBGBlue();
	m_sScreenshotDir = Settings.GetScreenshotPath();
	m_sMemcardDir = Settings.GetMemCardPath();
	
	m_bRunning = FALSE;

	UpdateLegServoCheckbox( Settings.m_nServoLegLevel );
	UpdateArmServoCheckbox( Settings.m_nServoArmLevel );

	// set the default screenshot radio button state
	((CButton *)GetDlgItem( IDC_SINGLE ))->SetCheck( TRUE );
	((CButton *)GetDlgItem( IDC_TGA ))->SetCheck( TRUE );
	
	// set the default animation playback radio button state
	((CButton *)GetDlgItem( IDC_REALTIME_FPS ))->SetCheck( TRUE );
	
	// set the default view bounds state
	((CButton *)GetDlgItem( IDC_VIEW_BOUNDS ))->SetCheck( FALSE );

	// Set render options
	((CButton *)GetDlgItem( IDC_DONT_RENDER_SURFACE ))->SetCheck( !(FRS_bRenderFlags & FRS_RENDER_PASS_SURFACE) );
	((CButton *)GetDlgItem( IDC_DONT_RENDER_SPECULAR ))->SetCheck( !(FRS_bRenderFlags & FRS_RENDER_PASS_SPECULAR) );
	((CButton *)GetDlgItem( IDC_DONT_RENDER_TRANSLUCENCIES ))->SetCheck( !(FRS_bRenderFlags & FRS_RENDER_TRANSLUCENCIES) );

	// Set Camera POV
	((CButton *)GetDlgItem( IDC_CAMERA_POV_VISIBILITY ))->SetCheck( Game_bCameraPOVVisibility );
	
	// set the dev skip radio buttons
	m_bSkipLevelSelect = Settings.m_bSkipLevelSelect;
	((CButton *)GetDlgItem( IDC_SKIP_LEVEL_SELECT ))->SetCheck( m_bSkipLevelSelect );

	m_bPlayerDeath = Settings.m_bPlayerDeath;
	((CButton *)GetDlgItem( IDC_CBPLAYER_DEATH))->SetCheck( m_bPlayerDeath );

	m_bDebugAI = Settings.m_bDebugAI;
	((CButton *)GetDlgItem( IDC_CBDEBUG_AI))->SetCheck( m_bDebugAI );
	
	if( Settings.GetXbox() ) {
		((CButton *)GetDlgItem( IDC_XBOX ))->SetCheck( TRUE );
	} else if( Settings.GetGameCube() ) {
		((CButton *)GetDlgItem( IDC_GAMECUBE ))->SetCheck( TRUE );
	}

	if( Settings.GetMemSlot1() ) {
		((CButton *)GetDlgItem( IDC_SLOT1 ))->SetCheck( TRUE );
	}
	if( Settings.GetMemSlot2() ) {
		((CButton *)GetDlgItem( IDC_SLOT2 ))->SetCheck( TRUE );
	}

	((CButton *)GetDlgItem( IDC_DISPLAY_FPS ))->SetCheck( Settings.GetShowFPS() );
	gameloop_ShowFPS( Settings.GetShowFPS() );
	
	((CButton *)GetDlgItem( IDC_INSTALL_AUDIO ))->SetCheck( Settings.GetInstallAudio() );

	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );

	UpdateData( VARS_TO_CONTROLS );

	// make sure that a master file is selected, if not, give the user a chance to select one now
	if( m_sMasterFile.IsEmpty() ) {
		MessageBox( _T("You must select a master file before MAwin\ncan be used.  Click OK and then select\na master file."), 
					_T("MAwin Error: No Master File"),
					MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		CFileDialog dlg( TRUE,
						 ".mst",
						 m_sMasterFile,
						 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
						 "Master Files (*.mst)|*.mst||",
						 this );
		dlg.m_ofn.lpstrTitle = "Select Master File";
		if( dlg.DoModal() == IDOK ) {
			m_sMasterFile = dlg.GetPathName();
			m_sMasterFile.MakeLower();
			UpdateData( VARS_TO_CONTROLS );
		} else {
			// no master file, no MAwin
			PostMessage( WM_COMMAND, IDCANCEL, 0 );  
			return TRUE;
		}
	}

	// init the fang system
	_pWinPrintf = &m_WinPrintf;
	m_WinPrintf.AttachToEditControl( IDC_EDIT_PRINTBOX, this, "" );
	m_WinPrintf.Enable( TRUE );

	// Print sign-on message...
	_pWinPrintf->Printf( "Welcome to the Mettle Arms development system for Windows!\n" );

	#if FANG_DEBUG_BUILD
		_pWinPrintf->Printf( "DEBUG Build\n" );
	#elif FANG_TEST_BUILD
		_pWinPrintf->Printf( "TEST Build\n" );
	#elif FANG_RELEASE_BUILD
		_pWinPrintf->Printf( "RELEASE Build\n" );
	#elif FANG_PRODUCTION_BUILD
		_pWinPrintf->Printf( "PRODUCTION Build\n" );
	#elif
		#error <Undefined Fang build>
	#endif

	_pWinPrintf->Printf( "\n" );

	fang_Init();
	Fang_ConfigDefs.nRes_HeapBytes = ( (64)*1024*1024 );// add 24 extra megs for windows uncompressed audio
	Fang_ConfigDefs.pszFile_MasterFilePathName = m_sMasterFile;
	Fang_ConfigDefs.pszMovie_BasePathName = _MASTER_MOVIE_PATH;
	Fang_ConfigDefs.pFang_FcnPrintf = _DevPrintf;
	Fang_ConfigDefs.nAudio_MaxSoundBytes = (1024 * ( 36 * 1024));
	Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 4048000;
	Fang_ConfigDefs.bCheckPoint_StartupSystem = TRUE;
	Fang_ConfigDefs.nText_MaxCharsPerFrame = 550;
	Fang_ConfigDefs.nText_MaxCharsPerPrintf = 190;

	Fang_ConfigDefs.nMaxParticleEmitters = 500;
	Fang_ConfigDefs.nMaxParticles = 3000;
	Fang_ConfigDefs.nMaxParticleEmitterSprites = 1000;

	m_bFangOK = fang_Startup();
	if( !m_bFangOK ) {
		// could not install the fang engine, tell the user and exit
		MessageBox( "Fang Engine failed to install.", "Fang Install Failure", MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 );  
		return TRUE;
	}

	// figure out what video mode has been selected, if none tell the user to select one
	CVidMode VidMode;
	if( !VidMode.GetVidMode( &m_VidWin ) ) {
		// try to auto select the best video mode
		CVidMode VidModeDlg( TRUE );
		VidModeDlg.DoModal();
		if( !VidModeDlg.GetVidMode( &m_VidWin ) ) {
			// Video mode not selected...
			MessageBox( "You need to set the video mode before mawin may be used.\nClick Ok and then click the Video button.", "Mawin Error: Set Video Mode" );
			m_bValidVideoMode = FALSE;
			((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( FALSE );
		} else {
			m_bValidVideoMode = TRUE;
			((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( TRUE );	
		}
	} else {
		m_bValidVideoMode = TRUE;
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( TRUE );
	}
	m_VidWin.bAllowPowerSuspend = TRUE;
	m_VidWin.hInstance = theApp.m_hInstance;
	m_VidWin.hWnd = 0;
	m_VidWin.nIconIDI = IDR_MAINFRAME;
	m_VidWin.pFcnSuspend = NULL;
	strcpy( m_VidWin.szWindowTitle, "Mettle Arms Render Window" );

	{
		CString sPath = Mawin_pszFullAppPath;
		int nIndex = sPath.ReverseFind( '.' );
		if( nIndex >= 0 ) {
			sPath.Delete( nIndex, sPath.GetLength() - nIndex );
		}
		sPath.MakeLower();
		sPath += "_asset_log.txt";

		ffile_LogSetFilename( sPath );
	}

	if( Mawin_bAutoRun && m_bValidVideoMode ) {
		// post a run message
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->PostMessage( BM_CLICK );
		// minimize the main window
		WINDOWPLACEMENT WinPlacement;
		GetWindowPlacement( &WinPlacement );
		WinPlacement.showCmd = SW_MINIMIZE;
		SetWindowPlacement( &WinPlacement ); 
	}
		
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMa_winDlg::UpdateLegServoCheckbox( u32 nServoLevel ) {
	FASSERT( nServoLevel < 4 );

	((CButton *)GetDlgItem( IDC_LEGSERVO_DEFAULT ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_LEGSERVO_L1 ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_LEGSERVO_L2 ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_LEGSERVO_L3 ))->SetCheck( FALSE );

	switch( nServoLevel ) {
	case 0:
		((CButton *)GetDlgItem( IDC_LEGSERVO_DEFAULT ))->SetCheck( TRUE );
		break;

	case 1:
		((CButton *)GetDlgItem( IDC_LEGSERVO_L1 ))->SetCheck( TRUE );
		break;

	case 2:
		((CButton *)GetDlgItem( IDC_LEGSERVO_L2 ))->SetCheck( TRUE );
		break;

	case 3:
		((CButton *)GetDlgItem( IDC_LEGSERVO_L3 ))->SetCheck( TRUE );
		break;
	};
}

void CMa_winDlg::UpdateArmServoCheckbox( u32 nServoLevel ) {
	FASSERT( nServoLevel < 4 );

	((CButton *)GetDlgItem( IDC_ARMSERVO_DEFAULT ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_ARMSERVO_L1 ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_ARMSERVO_L2 ))->SetCheck( FALSE );
	((CButton *)GetDlgItem( IDC_ARMSERVO_L3 ))->SetCheck( FALSE );

	switch( nServoLevel ) {
	case 0:
		((CButton *)GetDlgItem( IDC_ARMSERVO_DEFAULT ))->SetCheck( TRUE );
		break;

	case 1:
		((CButton *)GetDlgItem( IDC_ARMSERVO_L1 ))->SetCheck( TRUE );
		break;

	case 2:
		((CButton *)GetDlgItem( IDC_ARMSERVO_L2 ))->SetCheck( TRUE );
		break;

	case 3:
		((CButton *)GetDlgItem( IDC_ARMSERVO_L3 ))->SetCheck( TRUE );
		break;
	};
}

u32 CMa_winDlg::GetLegServoLevel( void ) const {
	if( ((CButton *)GetDlgItem( IDC_LEGSERVO_DEFAULT ))->GetCheck() == 1 ) {
		return 0;
	} else if( ((CButton *)GetDlgItem( IDC_LEGSERVO_L1 ))->GetCheck() == 1 ) {
		return 1;
	} else if( ((CButton *)GetDlgItem( IDC_LEGSERVO_L2 ))->GetCheck() == 1 ) {
		return 2;
	}

	return 3;
}

u32 CMa_winDlg::GetArmServoLevel( void ) const {
	if( ((CButton *)GetDlgItem( IDC_ARMSERVO_DEFAULT ))->GetCheck() == 1 ) {
		return 0;
	} else if( ((CButton *)GetDlgItem( IDC_ARMSERVO_L1 ))->GetCheck() == 1 ) {
		return 1;
	} else if( ((CButton *)GetDlgItem( IDC_ARMSERVO_L2 ))->GetCheck() == 1 ) {
		return 2;
	}

	return 3;
}

void CMa_winDlg::OnSysCommand( UINT nID, LPARAM lParam ) {
	if( (nID & 0xFFF0) == IDM_ABOUTBOX ) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CMa_winDlg::OnPaint() {

	if( IsIconic() ) {
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
	} else {
		CDialog::OnPaint();
	}
}

HCURSOR CMa_winDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

void CMa_winDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_bRunning ) {
		OnButtonRun();
		// wait till the game loop has stop running and shutdown properly (it will post an exit msg when it has shutdown)
		MSG Msg;
		do {
			Sleep( 15 );
		} while( !PeekMessage( &Msg, this->m_hWnd, _GAMELOOP_EXIT, _GAMELOOP_EXIT, PM_NOREMOVE ) );
	}
		
	if( m_bFangOK ) {
		// shutdown fang
		fang_Shutdown();
		m_bFangOK = FALSE;
	}
	
	// writeout our settings before we exit
	CSettings& Settings = CSettings::GetCurrent();
	Settings.m_sArtPath = m_sMasterFile;
	Settings.m_sLastFile = m_sInputFilename;
	Settings.m_f3dBGRed = m_3DBGColorRGB.fRed;
	Settings.m_f3dBGGreen = m_3DBGColorRGB.fGreen;
	Settings.m_f3dBGBlue = m_3DBGColorRGB.fBlue;
	Settings.m_sScreenshotPath = m_sScreenshotDir;
	Settings.m_bSkipLevelSelect = m_bSkipLevelSelect;
	Settings.m_bPlayerDeath = m_bPlayerDeath;
	Settings.m_bDebugAI = m_bDebugAI;
	Settings.m_bXbox = ( ((CButton *)GetDlgItem( IDC_XBOX ))->GetCheck() == 1 );
	Settings.m_bGameCube = ( ((CButton *)GetDlgItem( IDC_GAMECUBE ))->GetCheck() == 1 );
	Settings.m_bMemSlot1 = ( ((CButton *)GetDlgItem( IDC_SLOT1 ))->GetCheck() == 1 );
	Settings.m_bMemSlot2 = ( ((CButton *)GetDlgItem( IDC_SLOT2 ))->GetCheck() == 1 );
	Settings.m_bShowFPS = ( ((CButton *)GetDlgItem( IDC_DISPLAY_FPS ))->GetCheck() == 1 );
	Settings.m_sMemCardPath = m_sMemcardDir;
	Settings.m_bInstallAudio = ( ((CButton *)GetDlgItem( IDC_INSTALL_AUDIO ))->GetCheck() == 1 );
	Settings.m_nServoLegLevel = GetLegServoLevel();
	Settings.m_nServoArmLevel = GetArmServoLevel();

	Settings.SaveCommonDataOutToFile();

	::RemoveProp( GetSafeHwnd(), Mawin_pszExeName );

	CDialog::OnCancel();
}

void CMa_winDlg::OnOK() {
	//OnCancel();
}

BOOL CMa_winDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CMa_winDlg::OnButtonLoad() {
	CPickAsset PickAssetDlg;

	UpdateData( CONTROLS_TO_VARS );

	PickAssetDlg.m_nOptions = CPickAsset::PICK_ASSET_WORLDS;
	PickAssetDlg.m_sInputMasterFile = m_sMasterFile;
	PickAssetDlg.m_bReadOnly = TRUE;
	PickAssetDlg.m_sInitialSelection = m_sInputFilename;
	PickAssetDlg.m_sInitialSelection += ".wld";
	if( PickAssetDlg.DoModal() == IDOK ) {
		m_sInputFilename = PickAssetDlg.GetFileTitle();		
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CMa_winDlg::OnChooseMasterFile() {
	CString sOldMasterFile;

	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".mst",
					 m_sMasterFile,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Master Files (*.mst)|*.mst||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Master File";
	if( dlg.DoModal() == IDOK ) {
		// grab the old master file name
		sOldMasterFile = m_sMasterFile;
		// grab the new master file name
		m_sMasterFile = dlg.GetPathName();
		m_sMasterFile.MakeLower();

		if( sOldMasterFile != m_sMasterFile ) {
			m_sInputFilename.Empty();			
		}
		
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CMa_winDlg::OnVideoMode() {
	CVidMode VidMode;

	UpdateData( CONTROLS_TO_VARS );

	VidMode.DoModal();

	UpdateData( VARS_TO_CONTROLS );
	
	if( VidMode.GetVidMode( &m_VidWin ) ) {
		m_bValidVideoMode = TRUE;
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( TRUE );
	}
	UpdateData( VARS_TO_CONTROLS );
}

// this button is the RUN or STOP button, depending on what state we are currently in
void CMa_winDlg::OnButtonRun() {
	
	if( !m_bValidVideoMode ) {
		// Just in case...
		return;
	}
	UpdateData( CONTROLS_TO_VARS );

	if( !m_bRunning ) {
		// Start up...
		if( m_sInputFilename.IsEmpty() ) {
			// no file selected, bring the window forward and then warn
			WINDOWPLACEMENT WinPlacement;
			GetWindowPlacement( &WinPlacement );
			WinPlacement.showCmd = SW_SHOWNORMAL;
			SetWindowPlacement( &WinPlacement );

			MessageBox( "You need to select a file first.\nClick Ok and then click the Load button.", "Mawin Error: No File Selected" );
			return;
		}
		
		EnableControls( FALSE );

		// go ahead and re-read the master file, just in case things were compiled in since launching the app
		Fang_ConfigDefs.pszFile_MasterFilePathName = m_sMasterFile;
		Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes = 4048000;
		ffile_SetupFileMode();
		
		/////////////////////////////////////////////////////////////
		// fill in a gameloop init parms struct to start the gameloop
		GameloopInitParm_t GameloopParms;
		GameloopParms.bLoadWorldOnly = FALSE;
		GameloopParms.fTargetFPS = GAMELOOP_DEFAULT_TARGET_FPS;
		GameloopParms.bSkipLevelSelect = m_bSkipLevelSelect;
		GameloopParms.bViewBounds = ( ((CButton *)GetDlgItem( IDC_VIEW_BOUNDS ))->GetCheck() == 1 );
		GameloopParms.bShowFPS = ( ((CButton *)GetDlgItem( IDC_DISPLAY_FPS ))->GetCheck() == 1 );
		GameloopParms.nAnimPlaybackRate = gameloop_GetAnimPlaybackRate();
		GameloopParms.bDrawScreenSafeArea = ( ((CButton *)GetDlgItem( IDC_DISPLAY_SAFE_SCREEN_BOX ))->GetCheck() == 1 );;
		GameloopParms.nMaxSoundMgrSounds = GAMELOOP_DEFAULT_MAX_SOUNDS;
		GameloopParms.bPlayerDeath = m_bPlayerDeath;
		GameloopParms.bDebugAI = m_bDebugAI;
		if( ((CButton *)GetDlgItem( IDC_XBOX ))->GetCheck() == 1 ) {
			GameloopParms.nPlatform = GAMELOOP_PLATFORM_XB;
		} else {
			GameloopParms.nPlatform = GAMELOOP_PLATFORM_GC;
		}
		GameloopParms.pszInputFilename = m_sInputFilename;
		GameloopParms.pszScreenShotDir = m_sScreenshotDir;
		GameloopParms.VidWin = m_VidWin;
		GameloopParms.BGColorRGB = m_3DBGColorRGB; 
		GameloopParms.pExitFunc = _GameloopExitFunction;
		GameloopParms.pMinFunc = _GameloopMinimizeFunction;
		GameloopParms.nMemCardUsageFlags = GAMELOOP_MEMCARD_NONE;
		if( ((CButton *)GetDlgItem( IDC_SLOT1 ))->GetCheck() == 1 ) {
			GameloopParms.nMemCardUsageFlags |= GAMELOOP_MEMCARD_SLOT1;
		}
		if( ((CButton *)GetDlgItem( IDC_SLOT2 ))->GetCheck() == 1 ) {
			GameloopParms.nMemCardUsageFlags |= GAMELOOP_MEMCARD_SLOT2;
		}
		GameloopParms.pszMemCardDir = m_sMemcardDir;

		CSettings& oSettings = CSettings::GetCurrent();
		GameloopParms.pauInputEmulationMap = oSettings.m_auInputEmulationMap;
		GameloopParms.pszInputEmulationDevName = (u8 *)(LPCSTR)( oSettings.m_sInputEmulationDevName );
		GameloopParms.bInstallAudio = ( ((CButton *)GetDlgItem( IDC_INSTALL_AUDIO ))->GetCheck() == 1 );

		GameloopParms.nLegServoLevel = GetLegServoLevel();
		GameloopParms.nArmServoLevel = GetArmServoLevel();
		GameloopParms.bShowEntityInfo = m_bShowEntityInfo;
		GameloopParms.bDemoLaunched = FALSE;
		GameloopParms.uTimeoutInterval = 0;
		//
		///////////////////////////////////////////////

		if( !gameloop_Start( &GameloopParms ) ) {
			EnableControls( TRUE );
		}
		m_bRunning = TRUE;
	} else {
		// Shut down...
		gameloop_End();
	}
}

// sPathname should be in the form: C:\hello\mike.txt and this function will
// fill sFilename with: mike.  Returns TRUE if sFilename was filled in, FALSE 
// if there was a problem and the filename could not be computed.
BOOL CMa_winDlg::GetFilenameFromFullPath( CString &sPathname, CString &sFilename ) {
	
	if( sPathname.IsEmpty() ) {
		return FALSE;
	}
	int nSlashIndex, nPeriodIndex;
	nSlashIndex = sPathname.ReverseFind( '\\' );
	nSlashIndex++;// this may be zero if there was no \ found
	nPeriodIndex = sPathname.ReverseFind( '.' );
	if( nPeriodIndex < 0 ) {
		// no period found, just copy from the slash to the end
		sFilename = sPathname.Mid( nSlashIndex );
	} else {
		// there was a period, I'll assume that is the extension seperator and 
		// copy from the slash to the period
		sFilename = sPathname.Mid( nSlashIndex, nPeriodIndex-nSlashIndex );
	}
	return TRUE;
}

void CMa_winDlg::On3dColor() {
	UpdateData( CONTROLS_TO_VARS );

	CColorDialog ColorDlg( _COLORRGB_TO_COLORREF( &m_3DBGColorRGB ), CC_ANYCOLOR | CC_FULLOPEN, this );

	if( ColorDlg.DoModal() == IDOK ) {
		CFColorRGB Temp;
		_COLORREF_TO_COLORRGB( &Temp, ColorDlg.GetColor() );
		m_3DBGColorRGB.fRed = Temp.fRed;
		m_3DBGColorRGB.fGreen = Temp.fGreen;
		m_3DBGColorRGB.fBlue = Temp.fBlue;
	}	
	UpdateData( VARS_TO_CONTROLS );		
}

void CMa_winDlg::OnChooseScreenshotDir() {
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Screenshot Directory", m_sScreenshotDir ) == PICKDIR_RET_SUCCESS ) {
		m_sScreenshotDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CMa_winDlg::OnSeries() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_SERIES ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotMode( FALSE );
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnSingle() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_SINGLE ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotMode( TRUE );
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnDontRenderSurface() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_DONT_RENDER_SURFACE ))->GetCheck() == 1 );
	if ( !bState )
	{
		FRS_bRenderFlags |= FRS_RENDER_PASS_SURFACE;
	}
	else
	{
		FRS_bRenderFlags &= ~FRS_RENDER_PASS_SURFACE;
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnDontRenderSpecular() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_DONT_RENDER_SPECULAR ))->GetCheck() == 1 );
	if ( !bState )
	{
		FRS_bRenderFlags |= FRS_RENDER_PASS_SPECULAR;
	}
	else
	{
		FRS_bRenderFlags &= ~FRS_RENDER_PASS_SPECULAR;
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnDontRenderTranslucencies() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_DONT_RENDER_TRANSLUCENCIES ))->GetCheck() == 1 );
	if ( !bState )
	{
		FRS_bRenderFlags |= FRS_RENDER_TRANSLUCENCIES;
	}
	else
	{
		FRS_bRenderFlags &= ~FRS_RENDER_TRANSLUCENCIES;
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnCameraPOVVisibility() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_CAMERA_POV_VISIBILITY ))->GetCheck() == 1 );
	Game_bCameraPOVVisibility = bState;
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnViewBounds() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_VIEW_BOUNDS ))->GetCheck() == 1 );
	gameloop_SetViewBounds( bState );
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnPlayerDeath() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_CBPLAYER_DEATH ))->GetCheck() == 1 );

	gameloop_SetPlayerDeath(bState);
	CBot::m_bAllowPlayerDeath = bState;

	UpdateData( VARS_TO_CONTROLS );
}


void CMa_winDlg::OnDebugAI() {
	UpdateData( CONTROLS_TO_VARS );
	
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_CBDEBUG_AI))->GetCheck() == 1 );
	gameloop_SetDebugAI(bState);
	
	UpdateData( VARS_TO_CONTROLS );
}


void CMa_winDlg::OnBDADAI() {
	UpdateData( CONTROLS_TO_VARS );
	
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_CBBDADAI))->GetCheck() == 1 );
	CAIBrain::m_bGlobalIgnoreMode = bState;
	
	UpdateData( VARS_TO_CONTROLS );
}


void CMa_winDlg::On30fps() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_30FPS ))->GetCheck() == 1 );
	if( bState ) {
		gameloop_SetAnimPlaybackRate( GAMELOOP_ANIM_PLAYBACK_30FPS );
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::On60fps() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_60FPS ))->GetCheck() == 1 );
	if( bState ) {
		gameloop_SetAnimPlaybackRate( GAMELOOP_ANIM_PLAYBACK_60FPS );
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnRealtimeFps() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_FPS ))->GetCheck() == 1 );
	if( bState ) {
		gameloop_SetAnimPlaybackRate( GAMELOOP_ANIM_PLAYBACK_REALTIME );
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CMa_winDlg::OnInputMapper() {
	CInputEmulation oInputEmulation;
	oInputEmulation.DoModal();
}

void CMa_winDlg::OnTgaClick() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_TGA ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotType( SCREENSHOT_OUTPUT_TYPE_TGA );
	}
}

void CMa_winDlg::OnJpegClick() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_JPEG ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotType( SCREENSHOT_OUTPUT_TYPE_JPEG );
	}
}

void CMa_winDlg::OnXbox() {
	UpdateData( CONTROLS_TO_VARS );		
}

void CMa_winDlg::OnGamecube() {
	UpdateData( CONTROLS_TO_VARS );		
}

void CMa_winDlg::OnDisplayFps() {
	UpdateData( CONTROLS_TO_VARS );	
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_DISPLAY_FPS ))->GetCheck() == 1 );
	gameloop_ShowFPS( bState );
}

void CMa_winDlg::OnChooseMemcardDir() {
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Memory Card Directory", m_sMemcardDir ) == PICKDIR_RET_SUCCESS ) {
		m_sMemcardDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CMa_winDlg::OnSlot1() {
	UpdateData( CONTROLS_TO_VARS );	
}

void CMa_winDlg::OnSlot2() {
	UpdateData( CONTROLS_TO_VARS );	
}

void CMa_winDlg::OnDisplaySafeScreenBox() {
	UpdateData( CONTROLS_TO_VARS );	
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_DISPLAY_SAFE_SCREEN_BOX ))->GetCheck() == 1 );
	gameloop_ShowScreenSafeArea( bState );	
}

void CMa_winDlg::OnLegServoDefault() {
	UpdateLegServoCheckbox( 0 );
	gameloop_SetLegServoLevel( 0 );
}

void CMa_winDlg::OnLegServoL1() {
	UpdateLegServoCheckbox( 1 );
	gameloop_SetLegServoLevel( 1 );
}

void CMa_winDlg::OnLegServoL2() {
	UpdateLegServoCheckbox( 2 );
	gameloop_SetLegServoLevel( 2 );
}

void CMa_winDlg::OnLegServoL3() {
	UpdateLegServoCheckbox( 3 );
	gameloop_SetLegServoLevel( 3 );
}

void CMa_winDlg::OnArmServoDefault() {
	UpdateArmServoCheckbox( 0 );
	gameloop_SetArmServoLevel( 0 );
}

void CMa_winDlg::OnArmServoL1() {
	UpdateArmServoCheckbox( 1 );
	gameloop_SetArmServoLevel( 1 );
}

void CMa_winDlg::OnArmServoL2() {
	UpdateArmServoCheckbox( 2 );
	gameloop_SetArmServoLevel( 2 );
}

void CMa_winDlg::OnArmServoL3() {
	UpdateArmServoCheckbox( 3 );
	gameloop_SetArmServoLevel( 3 );
}

void CMa_winDlg::OnCheckShowEntityInfo() {
	UpdateData( CONTROLS_TO_VARS );
	
	gameloop_SetShowEntityInfo( m_bShowEntityInfo );
}

void CMa_winDlg::OnBnClickedDontDrawHud() {
	UpdateData( CONTROLS_TO_VARS );	
	BOOL bDebugChecked = ( ((CButton *)GetDlgItem( IDC_DONT_DRAW_DEBUG ))->GetCheck() == 1 );
	BOOL bHudChecked = ( ((CButton *)GetDlgItem( IDC_DONT_DRAW_HUD ))->GetCheck() == 1 );
	gameloop_EnableOverlayDrawElements( !bHudChecked, !bDebugChecked );
}

void CMa_winDlg::OnBnClickedDontDrawDebug() {
	UpdateData( CONTROLS_TO_VARS );	
	BOOL bDebugChecked = ( ((CButton *)GetDlgItem( IDC_DONT_DRAW_DEBUG ))->GetCheck() == 1 );
	BOOL bHudChecked = ( ((CButton *)GetDlgItem( IDC_DONT_DRAW_HUD ))->GetCheck() == 1 );
	gameloop_EnableOverlayDrawElements( !bHudChecked, !bDebugChecked );
}

void CMa_winDlg::OnBnClickedLightShowrange()
{
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_LIGHT_SHOWRANGE ))->GetCheck() == 1 );

	FLight_bShowLightRange = bState;

	UpdateData( VARS_TO_CONTROLS );	
}
