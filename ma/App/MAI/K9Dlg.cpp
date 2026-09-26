//////////////////////////////////////////////////////////////////////////////////////
// K9Dlg.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 06/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "K9.h"
#include "K9Dlg.h"
#include "Settings.h"
#include "fang.h"
#include "pickdir.h"
#include "VidMode.h"
#include "fviewport.h"
#include "fworld.h"
#include "floop.h"
#include "fresload.h"
#include "dx/fdx8vid.h"
#include "ftext.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fperf.h"
#include "ftimer.h"
#include "fversion.h"
#include "PickAsset.h"
#include "ffile.h"
#include "screenshot.h"
#include "maitoolbar.h"
#include "maimain.h"
#include "maiEditGraph.h"
#include "fvis.h"
#include "fangalign.h"
#include <io.h>  //_access


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// custom messages
#define _MM_GAMELOOP_RUN		(WM_USER + 173)
#define _MM_GAMELOOP_TERM		(WM_USER + 174)
#define _MM_GAMELOOP_LOADERR	(WM_USER + 176)

// camera defines
#define _MIN_CAM_ZOOM			0.25f
#define _MAX_CAM_ZOOM			30.0f
#define _DEFAULT_CAM_ZOOM		4.0f
f32  fCamXLatSpeed = 0.5f*60.0f;
#define _CAM_SPIN_SPEED			0.035f

// color conversion (windows to game)
#define _COLORRGB_TO_COLORREF( pColorRGB ) ( (u32)((pColorRGB)->fRed * 255.0f) | ((u32)((pColorRGB)->fGreen * 255.0f) << 8) | ((u32)((pColorRGB)->fBlue * 255.0f) << 16) )
#define _COLORREF_TO_COLORRGB( pColorRGB, nColorRef ) (pColorRGB)->Set( (f32)((nColorRef) & 0xff) * (1.0f/255.0f), (f32)(((nColorRef)>>8) & 0xff) * (1.0f/255.0f), (f32)(((nColorRef)>>16) & 0xff) * (1.0f/255.0f) )

// private variables
static BOOL _bMainLoopExecuted;
static FViewport_t *_pViewport;
CFXfm k9dlg_XfmCam;
static CFXfm _XfmModelOrigin;
CFVec3 _WorldCamPos;
f32 _fCamPitch;
static FResFrame_t _ResFrame_Mesh;
static FVisData_t *_pWorldRes;
static BOOL _bLastLeftOnDown;
static BOOL _bLastRightOnDown;
static BOOL _bLastLeftOnUp;
static BOOL _bLastRightOnUp;
static BOOL _bLastLeftButton;
static BOOL _bLastRightButton;
static BOOL _bOldLastLeftButton;
static BOOL _bOldLastRightButton;
static u32 _nLastMouseX;
static u32 _nLastMouseY;
static s32 _nDeltaMouseX;
static s32 _nDeltaWheel;
static s32 _nDeltaMouseY;
static f32 _fCamZoom;
static u32 _nRadius;
static BOOL _bProcessLButton;
static BOOL _bProcessRButton;
static f32 _fCamHeading;
static BOOL _b2dMode;
static BOOL _nFirstMouseActivity;
static f32 _fOrthoCamY;
static f32 _fFrameSecs;
static CWinPrintf *_pWinPrintf;
static BOOL _bBookmarkValid;
static CFVec3 _BookmarkPos;
static f32 _fBookmarkZoom;
static f32 _fBookmarkHeading;
static f32 _fBookmarkPitch;
static BOOL _b3DCam_KillYAxis = FALSE;

// private prototypes
static BOOL _GameInit( void *pParameter );
static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter );
static BOOL _GameMain( BOOL bExitRequest, void *pParameter );
static void _GameMouseCallback( u32 nX, u32 nY, s32 nDeltaWheel, BOOL bLeftButton, BOOL bRightButton, void *pParameter );
static void _2DWork( BOOL bHaveFocus );
static void _3DWork( BOOL bHaveFocus );
static void _DisplayPerf( void );
static void _DevPrintf( cchar *pszFormat, FANG_VA_LIST Args );
static void _SetCameraBookmark(void);

static char szGraphFileName[256];
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

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
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
// CK9Dlg dialog

CK9Dlg::CK9Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CK9Dlg::IDD, pParent) {
	//{{AFX_DATA_INIT(CK9Dlg)
	m_sMasterFile = _T("");
	m_sInputFilename = _T("");
	m_sDisplayListDir = _T("");
	m_sVersion = _T("");
	m_sScreenshotDir = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD));

	m_bFangOK = FALSE;
	m_bValidVideoMode = FALSE;
	m_bRunning = FALSE;
	m_bLoad = FALSE;
	m_bDrawPerfStats = FALSE;
	m_bDrawFPS = TRUE;
	m_bDrawCamInfo = TRUE;	
}

CK9Dlg::~CK9Dlg()
{
}
void CK9Dlg::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CK9Dlg)
	DDX_Text(pDX, IDC_ART_DIR, m_sMasterFile);
	DDX_Text(pDX, IDC_INPUT_NAME, m_sInputFilename);
	DDX_Text(pDX, IDC_DISPLAY_LIST_DIR, m_sDisplayListDir);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Text(pDX, IDC_SCREENSHOT_DIR, m_sScreenshotDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( CK9Dlg, CDialog ) 
	//{{AFX_MSG_MAP(CK9Dlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_CHOOSE_ART_DIR, OnChooseMasterFile)
	ON_BN_CLICKED(IDC_VIDEO_MODE, OnVideoMode)
	ON_BN_CLICKED(IDC_BUTTON_RUN, OnButtonRun)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_CHOOSE_DISPLAY_LIST_DIR, OnChooseDisplayListDir)
	ON_BN_CLICKED(IDC_3D_COLOR, On3dColor)
	ON_BN_CLICKED(IDC_DRAW_STATS, OnDrawStats)
	ON_BN_CLICKED(IDC_DRAW_FPS, OnDrawFpsClick)
	ON_BN_CLICKED(IDC_DRAW_CAM_STATS, OnDrawCamStatsClick)
	ON_BN_CLICKED(IDC_CHOOSE_SCREENSHOT_DIR, OnChooseScreenshotDir)
	ON_BN_CLICKED(IDC_SERIES, OnSeries)
	ON_BN_CLICKED(IDC_SINGLE, OnSingle)
	ON_BN_CLICKED(IDC_HIDE_CURSOR, OnHideCursorClick)
	ON_MESSAGE(_MM_GAMELOOP_RUN, OnGameloopRun)
	ON_MESSAGE(_MM_GAMELOOP_TERM, OnGameloopTerm)
	ON_MESSAGE(_MM_GAMELOOP_LOADERR, OnGameloopLoadErr)
	ON_BN_CLICKED(IDC_TGA, OnTgaClick)
	ON_BN_CLICKED(IDC_JPEG, OnJpegClick)
	ON_MESSAGE(APP_MSG_CODES_K9_LAUNCH, OnLaunchMsg)
	ON_COMMAND(ID_SELECTTOOL, OnSelectTool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CK9Dlg message handlers
int CK9Dlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int r = CDialog::OnCreate(lpCreateStruct);

	return r;
}


void CK9Dlg::OnDestroy()
{
	CDialog::OnDestroy();

}

BOOL CK9Dlg::OnInitDialog() {

	CDialog::OnInitDialog();

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
			//pSysMenu->AppendMenu(MF_SEPARATOR);
			//pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		pSysMenu->EnableMenuItem( 2, MF_BYPOSITION | MF_GRAYED );
		pSysMenu->EnableMenuItem( 4, MF_BYPOSITION | MF_GRAYED );
	}
	SetIcon( m_hIcon, TRUE );		// Set big icon
	SetIcon( m_hIcon, FALSE );		// Set small icon
	
	// TODO: Add extra initialization here
	::SetProp( GetSafeHwnd(), K9_pszPropName, (HANDLE)1 ); 


	char szWorkingDir[256];
	_fullpath(szWorkingDir, ".", 240);

	char szFullPath[256];
	strcpy(szFullPath, szWorkingDir);
	strcat(szFullPath, "\\Mai.ini");

	if (_access(szFullPath, 6)==0)
	{  //found an ini that exists in current working directory. use it!
		CSettings::SetSettingsFilename( szFullPath );
	}
	else
	{  //nope, go ahead and read/write to the one in the windows directory.. even create it there if necessary.
		CSettings::SetSettingsFilename( "Mai.ini" );
	}

	// get the default values from our settings file
	ReadSettingsFromDisk(NULL);

	m_bRunning = FALSE;
	m_GameloopParm.pDlg = this;
	m_bLoad = FALSE;
	m_AmbientColorRGB.Set( 0.4f );
	_bMainLoopExecuted = FALSE;
	_pViewport = NULL;
	_bLastLeftOnDown = FALSE;
	_bLastRightOnDown = FALSE;
	_bLastLeftOnUp = FALSE;
	_bLastRightOnUp = FALSE;
	_bLastLeftButton = FALSE;
	_bLastRightButton = FALSE;
	_bOldLastLeftButton = FALSE;
	_bOldLastRightButton = FALSE;
	_bProcessLButton = FALSE;
	_bProcessRButton = FALSE;
	_b2dMode = TRUE;
	_nLastMouseX = 0;
	_nLastMouseY = 0;
	_nDeltaMouseX = 0;
	_nDeltaMouseY = 0;

	((CButton *)GetDlgItem( IDC_BUTTON_SAVE ))->EnableWindow( FALSE );

	((CButton *)GetDlgItem(IDC_DRAW_STATS))->SetCheck( m_bDrawPerfStats );
	((CButton *)GetDlgItem(IDC_DRAW_FPS))->SetCheck( m_bDrawFPS );
	((CButton *)GetDlgItem(IDC_DRAW_CAM_STATS))->SetCheck( m_bDrawCamInfo );

	((CButton *)GetDlgItem(IDC_HIDE_CURSOR))->SetCheck( !m_bDrawCursor );

	// set the default screenshot radio button state
	if( screenshot_GetScreenShotMode() ) {
		((CButton *)GetDlgItem( IDC_SINGLE ))->SetCheck( TRUE );
	} else {
		((CButton *)GetDlgItem( IDC_SERIES ))->SetCheck( TRUE );
	}
	switch( screenshot_GetScreenShotType() ) {
	default:
	case SCREENSHOT_OUTPUT_TYPE_TGA:
		((CButton *)GetDlgItem( IDC_TGA ))->SetCheck( TRUE );
		break;
	case SCREENSHOT_OUTPUT_TYPE_JPEG:
		((CButton *)GetDlgItem( IDC_JPEG ))->SetCheck( TRUE );
		break;
	}

	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );

	UpdateData( VARS_TO_CONTROLS );

	if( m_sMasterFile.IsEmpty() ) {
		MessageBox( _T("You must select a master file before K9\ncan be used.  Click OK and then select\na master file."), 
					_T("K9 Error: No Master File"),
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
			// no master file, no K9
			PostMessage( WM_COMMAND, IDCANCEL, 0 );  
			return TRUE;
		}
	}
	// init the fang system
	_pWinPrintf = &m_WinPrintf;
	m_WinPrintf.AttachToEditControl( IDC_EDIT_PRINTBOX, this, "" );
	m_WinPrintf.Enable( TRUE );

	fang_Init();
	Fang_ConfigDefs.nRes_HeapBytes = (16*1024*1024);
	Fang_ConfigDefs.pszFile_MasterFilePathName = m_sMasterFile;
	Fang_ConfigDefs.pFang_FcnPrintf = _DevPrintf;

	m_bFangOK = fang_Startup();
	if( !m_bFangOK ) {
		// could not install the fang engine, tell the user and exit
		this->MessageBox( "Fang Engine failed to install.", 
						  "Install Failure",
						  MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 );  
		return TRUE;
	}

	CVidMode VidMode;
	if( !VidMode.GetVidMode( &m_GameloopParm.VidWin ) ) {
		// Video mode not selected... 
		MessageBox( "You need to set the video mode before K9 may be used.\nClick Ok and then click the Video button.", "K9 Error: Set Video Mode" );
		m_bValidVideoMode = FALSE;
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( FALSE );
	} else {
		m_bValidVideoMode = TRUE;
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( TRUE );
	}	
	m_GameloopParm.VidWin.bAllowPowerSuspend = TRUE;
	m_GameloopParm.VidWin.hInstance = theApp.m_hInstance;
	m_GameloopParm.VidWin.hWnd = 0;
	m_GameloopParm.VidWin.nIconIDI = IDR_MAINFRAME;
	m_GameloopParm.VidWin.pFcnSuspend = NULL;
	strcpy( m_GameloopParm.VidWin.szWindowTitle, "Mai AI Utility" );

	_XfmModelOrigin.Identity();
	k9dlg_XfmCam.Identity();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CK9Dlg::OnSysCommand( UINT nID, LPARAM lParam ) {
	
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
void CK9Dlg::OnPaint() {

	if( IsIconic() ) {
		CPaintDC dc( this ); // device context for painting

		SendMessage( WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0 );

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics( SM_CXICON );
		int cyIcon = GetSystemMetrics( SM_CYICON );
		CRect rect;
		GetClientRect( &rect );
		int x = ( rect.Width() - cxIcon + 1 ) / 2;
		int y = ( rect.Height() - cyIcon + 1 ) / 2;

		// Draw the icon
		dc.DrawIcon( x, y, m_hIcon );
	} else {
		CDialog::OnPaint();
	}
}

HCURSOR CK9Dlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

void CK9Dlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_bRunning ) {
		OnButtonRun();
	}
	
	if( m_bFangOK ) {
		// shutdown fang
		fang_Shutdown();
		m_bFangOK = FALSE;
	}
	
	// writeout our settings before we exit
	SaveSettingsToDisk();

	::RemoveProp( GetSafeHwnd(), K9_pszPropName );

	CDialog::OnCancel();
}

void CK9Dlg::ReadSettingsFromDisk(cchar* pszLevelName) {

	CSettings& Settings = CSettings::GetCurrent();
	Settings.GetCommonDataFromFile(pszLevelName);
	CSettings::SetApplicationName( "Mai" );

	m_sMasterFile = Settings.GetArtPath();
	m_sInputFilename = Settings.GetLastFile();
	m_sDisplayListDir = Settings.GetDisplayListPath();
	m_3DBGColorRGB.fRed = Settings.Get3dBGRed();
	m_3DBGColorRGB.fGreen = Settings.Get3dBGGreen();
	m_3DBGColorRGB.fBlue = Settings.Get3dBGBlue();
	m_bDrawPerfStats = Settings.GetDrawPerfStats();
	m_bDrawFPS = Settings.GetDrawFPS();
	m_bDrawCamInfo = Settings.GetDrawCamInfo();
	m_sScreenshotDir = Settings.GetScreenshotPath();
	_bBookmarkValid = Settings.GetBookmarkValid();
	if( _bBookmarkValid ) {
		_BookmarkPos.Set( Settings.GetBookmarkCamX(), Settings.GetBookmarkCamY(), Settings.GetBookmarkCamZ() );
		_fBookmarkZoom = Settings.GetBookmarkCamZoom();
		_fBookmarkHeading = Settings.GetBookmarkCamHeading();
		_fBookmarkPitch = Settings.GetBookmarkCamPitch();

		_WorldCamPos = _BookmarkPos;
		_fCamZoom = _fBookmarkZoom;
		_fCamHeading = _fBookmarkHeading;
		_fCamPitch = _fBookmarkPitch;

	}
	m_bDrawCursor = Settings.GetDrawCursor();


}
void CK9Dlg::SaveSettingsToDisk() {
	UpdateData( CONTROLS_TO_VARS );

	CSettings& Settings = CSettings::GetCurrent();

	_SetCameraBookmark();
	
	Settings.m_sArtPath = m_sMasterFile;
	Settings.m_sLastFile = m_sInputFilename;
	Settings.m_sDisplayListPath = m_sDisplayListDir;
	Settings.m_f3dBGRed = m_3DBGColorRGB.fRed;
	Settings.m_f3dBGGreen = m_3DBGColorRGB.fGreen;
	Settings.m_f3dBGBlue = m_3DBGColorRGB.fBlue;
	Settings.m_bDrawPerfStats = m_bDrawPerfStats;
	Settings.m_bDrawFPS = m_bDrawFPS;
	Settings.m_bDrawCamInfo = m_bDrawCamInfo;
	Settings.m_sScreenshotPath = m_sScreenshotDir;
	if( _bBookmarkValid ) {
		Settings.m_bBookmarkValid = _bBookmarkValid;
		Settings.m_fBookmarkCamX = _BookmarkPos.x;
		Settings.m_fBookmarkCamY = _BookmarkPos.y;
		Settings.m_fBookmarkCamZ = _BookmarkPos.z;
		Settings.m_fBookmarkCamZoom = _fBookmarkZoom;
		Settings.m_fBookmarkCamHeading = _fBookmarkHeading;
		Settings.m_fBookmarkCamPitch = _fBookmarkPitch;
	}
	Settings.m_bDrawCursor = m_bDrawCursor;

	Settings.SaveCommonDataOutToFile();
}

void CK9Dlg::OnOK() {
	//OnCancel();
}

BOOL CK9Dlg::PreTranslateMessage( MSG* pMsg ) {

	if( m_hAccel != NULL ) {
		if( ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) {
			return TRUE;
		}
	}
	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CK9Dlg::OnButtonLoad() {
	CPickAsset PickAssetDlg;

	UpdateData( CONTROLS_TO_VARS );

	PickAssetDlg.m_nOptions = CPickAsset::PICK_ASSET_WORLDS;
	PickAssetDlg.m_sInputMasterFile = m_sMasterFile;
	PickAssetDlg.m_bReadOnly = TRUE;
	PickAssetDlg.m_sInitialSelection = m_sInputFilename;
	PickAssetDlg.m_sInitialSelection += ".wld";
	if( PickAssetDlg.DoModal() == IDOK ) {
		SaveSettingsToDisk();
		m_sInputFilename = PickAssetDlg.GetFileTitle();		
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CK9Dlg::OnChooseMasterFile() {
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

		ReadSettingsFromDisk(NULL);	 //pgm: why is this here?

	}
}

void CK9Dlg::OnVideoMode() {
	CVidMode VidMode;

	VidMode.DoModal();
	
	if( VidMode.GetVidMode( &m_GameloopParm.VidWin ) ) {
		m_bValidVideoMode = TRUE;
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( TRUE );
	}
}

long CK9Dlg::OnLaunchMsg( WPARAM wParam, LPARAM lParam ) {

	UpdateData( CONTROLS_TO_VARS );

	DEVPRINTF( "\nK9: WE HAVE RECEIVED A LAUNCH MSG FROM ANOTHER PROCESS.\n" );

	if( K9_SharedAppData->aAppData[APP_TYPE_K9].nNumStrings ) {
		
		if( m_bRunning ) {
			// we are currently running, stop it from running
			DEVPRINTF( "K9: Stopping the currently running world.\n" );
			OnButtonRun();// we can't post a message since the stopping process also posts messages (they would be out of order)
		}

		// change the input filename
		DEVPRINTF( "K9: Changing the world file to %s\n", K9_SharedAppData->aAppData[APP_TYPE_K9].szString );
		CString sTemp = K9_SharedAppData->aAppData[APP_TYPE_K9].szString;
		// strip off the file extension
		int nIndex = sTemp.Find( '.' );
		m_sInputFilename = sTemp.Left( nIndex );
		UpdateData( VARS_TO_CONTROLS );

		// do the same thing as clicking run
		DEVPRINTF( "K9: Posting a msg to run the new world file.\n" );
		PostMessage( WM_COMMAND, IDC_BUTTON_RUN, 0 );
	} else {
		DEVPRINTF( "K9: No filename was sent, don't know which world to run.\n" );
	}

	return 0;
}

// this button is the RUN or STOP button, depending on what state we are currently in
void CK9Dlg::OnButtonRun() {
	
	if( !m_bValidVideoMode ) {
		// Just in case...
		return;
	}

	if( !m_bRunning ) {
		// Start up...
		if( m_sInputFilename.IsEmpty() ) {
			// no file selected
			MessageBox( "You need to select a file first.\nClick Ok and then click the Load button.", "K9 Error: No File Selected" );
			return;
		}
		ReadSettingsFromDisk(m_sInputFilename);	 //now that m_sInputFilename is set

		m_bLoad = TRUE;
		((CButton *)GetDlgItem( IDC_BUTTON_SAVE ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_VIDEO_MODE ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_BUTTON_LOAD))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( FALSE );
		// go ahead and re-read the master file, just in case things were compiled in since launching the app
		Fang_ConfigDefs.pszFile_MasterFilePathName = m_sMasterFile;
		ffile_SetupFileMode();
		floop_InstallGameloop( _GameInit, _GameMain, _GameTerm, &m_GameloopParm, 60 );
	} else {
		// Shut down...
		((CButton *)GetDlgItem( IDC_BUTTON_SAVE ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( FALSE );
		floop_UninstallGameloop();
		// see if the user wants to save their work
//		SavePromptBeforeExit(); this doesn't help mai.  All game data is gone by now anyway
	}
}

long CK9Dlg::OnGameloopRun(WPARAM wParam, LPARAM lParam) {
	m_bRunning = TRUE;
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->SetWindowText( "Stop" );
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_BUTTON_SAVE ))->EnableWindow( TRUE );
	return 0;
}

long CK9Dlg::OnGameloopTerm( WPARAM wParam, LPARAM lParam ) {

	// see if the user wants to save their work
	SaveSettingsToDisk();

	SavePromptBeforeExit();

	m_bRunning = FALSE;
	m_bLoad = FALSE;
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->SetWindowText( "Run" );
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->EnableWindow( TRUE );
	((CButton *)GetDlgItem(IDC_VIDEO_MODE))->EnableWindow( TRUE );
	((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_BUTTON_SAVE ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( TRUE );
	return 0;
}

long CK9Dlg::OnGameloopLoadErr( WPARAM wParam, LPARAM lParam ) {
	CString sErrMsg;

	SetForegroundWindow();

	((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( TRUE );

	sErrMsg.Format( "Trouble loading object '%s'.", m_sInputFilename );
	MessageBox( sErrMsg, "Mai Load Error" );

	UpdateData( VARS_TO_CONTROLS );

	return 0;
}

// sPathname should be in the form: C:\hello\mike.txt and this function will
// fill sFilename with: mike.  Returns TRUE if sFilename was filled in, FALSE 
// if there was a problem and the filename could not be computed.
BOOL CK9Dlg::GetFilenameFromFullPath( CString &sPathname, CString &sFilename ) {
	
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

static BOOL _GameInit( void *pParameter ) {
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CK9Dlg *pDlg = pParm->pDlg;

	_bMainLoopExecuted = FALSE;

	if( !fvid_CreateWindow( &pParm->VidWin ) ) {
		pDlg->MessageBox( "Could not use the current video mode.\nTry selecting a different video mode.", "K9: fvid_CreateWindow() Failed" );
		return FALSE;
	}

	fdx8vid_SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	fdx8vid_EnableCursor( pDlg->m_bDrawCursor );	

	_pViewport = fviewport_Create();
	if( _pViewport == NULL ) {
		return FALSE;
	}
	fviewport_InitOrtho3D( _pViewport, 0.1f, 5000.0f );
	k9dlg_XfmCam.BuildInvRotYXZ_XlatFromPoint( 0.0f, FMATH_HALF_PI, 0.0f, 0.0f, 100.0f, 0.0f );
	_WorldCamPos.Set( 0.0f, 100.0f, 0.0f );
	_fCamZoom = _DEFAULT_CAM_ZOOM;
	_fCamHeading = 0.0f;
	_fCamPitch = 0.0f;

	if (_bBookmarkValid)
	{
		_WorldCamPos = _BookmarkPos;
		_fCamZoom = _fBookmarkZoom;
		_fCamHeading = _fBookmarkHeading;
		_fCamPitch = _fBookmarkPitch;
	}
	k9dlg_XfmCam.BuildInvRotYXZ_XlatFromPoint( _fCamHeading, _fCamPitch, 0.0f, _WorldCamPos.x, _WorldCamPos.y, _WorldCamPos.z );


	if( FTEXT_ERROR == ftext_Install() ) {
		pDlg->MessageBox( "Could not install the text system.", "K9: Text System Problem" );
		return FALSE;
	}

	_ResFrame_Mesh = fres_GetFrame();
	
	_nLastMouseX = 0;
	_nLastMouseY = 0;
	_nDeltaMouseX = 0;
	_nDeltaMouseY = 0;
	_bLastLeftButton = FALSE;
	_bLastRightButton = FALSE;
	_bOldLastLeftButton = FALSE;
	_bOldLastRightButton = FALSE;
	_bProcessLButton = FALSE;
	_bProcessRButton = FALSE;
	_b2dMode = TRUE;

	screenshot_Init( pDlg->m_sScreenshotDir );

	fdx8vid_RegisterMouseCallback( _GameMouseCallback, pParm );

	fperf_Reset();
	_fFrameSecs = 0.0f;


	//////////// Graph editor initialization

	// Success...
	pDlg->PostMessage( _MM_GAMELOOP_RUN );

	return TRUE;
}

static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter ) {
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CK9Dlg *pDlg = pParm->pDlg;

	// shutdown the screen grab system
	screenshot_Shutdown();

	ftext_Uninstall();

	// change our button states
	pDlg->SavePromptBeforeExit();
	maimain_UninitSystem();


	fdx8vid_RegisterMouseCallback( NULL, NULL );
	fvid_DestroyWindow();
	pDlg->PostMessage( _MM_GAMELOOP_TERM );

}


static BOOL _GameMain( BOOL bExitRequest, void *pParameter ) {
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CK9Dlg *pDlg = pParm->pDlg;
	BOOL bKeyDown, bHaveFocus, bResetCam = FALSE;
//	f32 fNearPlane, fFarPlane;
//	FVisVolume_t *pVisVolume;
	CFTimer Timer;
	
	if( bExitRequest || fvid_HasUserClosedWindow() ) {
		return FALSE;
	}

	Timer.Reset();

	_bMainLoopExecuted = TRUE;

	bHaveFocus = fdx8vid_HaveFocus();

	if (!bHaveFocus)
		_nFirstMouseActivity = 1;

	if( pDlg->m_bLoad ) {
		//////////////////////
		// load the world file
		pDlg->m_bLoad = FALSE;

		if( !pDlg->m_sInputFilename.IsEmpty() ) {
			fdx8vid_RestoreWindow();

			fres_ReleaseFrame( _ResFrame_Mesh );
			_ResFrame_Mesh = fres_GetFrame();

			fworld_SetShapeCreateCallback( maimain_WorldShapeCreateCallback );

			_pWorldRes = (FVisData_t *)fresload_Load( FWORLD_RESTYPE, pDlg->m_sInputFilename );
			if( _pWorldRes == NULL ) {
				pDlg->PostMessage( _MM_GAMELOOP_LOADERR );
				return FALSE;
			}
//pgm		bResetCam = TRUE;  can't figure out how to start in 3d at a bookmark and still have the 2d cam work?
//pgm		_b2dMode = TRUE;

			bResetCam = FALSE;
			_b2dMode = FALSE;

			_nLastMouseX = 0;
			_nLastMouseY = 0;
			_nDeltaMouseX = 0;
			_nDeltaMouseY = 0;
			_nFirstMouseActivity = 1;						
		}

		strcpy(szGraphFileName, pDlg->m_sInputFilename);
		strcat(szGraphFileName, ".gt");
		if (!maimain_InitSystem(pDlg, pDlg->m_sDisplayListDir, szGraphFileName) ) {  // FINDFIX: extract this path from the dialog someday
			return FALSE;
		}
	
	}

	if ( GetAsyncKeyState( VK_CAPITAL ) & 1 )
	{
		_b3DCam_KillYAxis ^=1;
	}


	bKeyDown = bHaveFocus && ( GetAsyncKeyState( VK_ESCAPE ) < 0 );
	if( bKeyDown ) {
		return FALSE;
	}

	bKeyDown = bHaveFocus && ( GetAsyncKeyState( VK_BACK ) < 0 );
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
	}

	_bLastLeftOnDown = (_bLastLeftButton && !_bOldLastLeftButton);
	_bLastRightOnDown = (_bLastRightButton && !_bOldLastRightButton);
	_bLastLeftOnUp = (!_bLastLeftButton && _bOldLastLeftButton);
	_bLastRightOnUp = (!_bLastRightButton && _bOldLastRightButton);
	_bOldLastLeftButton = _bLastLeftButton;
	_bOldLastRightButton = _bLastRightButton;

	// see if we have changed modes
	if( bHaveFocus ) {
		if( _b2dMode ) {
			// look for the 3 key to change to 3d mode
			if( GetAsyncKeyState( '3' ) < 0 ) {
				_b2dMode = FALSE;
				fdx8vid_SetCursor( LoadCursor( NULL, IDC_SIZEALL ) );
			}
		} else {
			// look for the 2 key to change to 2d mode
			if( GetAsyncKeyState( '2' ) < 0 ) {
			//	_b2dMode = TRUE;
			//	fdx8vid_SetCursor( LoadCursor( NULL, IDC_ARROW ) );
			}
		}
	}
	// enable/disable the drawing of the cursor
	fdx8vid_EnableCursor( pDlg->m_bDrawCursor );

	// reset the camera position
	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_HOME ) < 0);
	if( bResetCam || bKeyDown ) {
#if 0
		// calculate the center of the quad field
		_WorldCamPos.Set( _pWorldRes->RootNodeCornerMin_WS.x + (_pWorldRes->fRootDim_WS * 0.5f),
						  _pWorldRes->fTerrainBoxBaseY_WS + ((_pWorldRes->fNodeBoxMaxY_WS - _pWorldRes->fTerrainBoxBaseY_WS) * 0.5f),
						  _pWorldRes->RootNodeCornerMin_WS.y + (_pWorldRes->fRootDim_WS * 0.5f) );
		// now move to the center of the actual used nodes
		f32 fDimOver128 = ( _pWorldRes->fRootDim_WS * (1.0f/128.0f) );
		_WorldCamPos.x += fDimOver128 * (f32)( (_pWorldRes->pRootNode->TerrainBox.nMinX >> 1) + (_pWorldRes->pRootNode->TerrainBox.nMaxX >> 1) );
		_WorldCamPos.z += fDimOver128 * (f32)( (_pWorldRes->pRootNode->TerrainBox.nMinZ >> 1) + (_pWorldRes->pRootNode->TerrainBox.nMaxZ >> 1) );
#endif
		_WorldCamPos.Set(0.0f,0.0f,0.0f);
		_fCamZoom = _DEFAULT_CAM_ZOOM;
		_fCamHeading = 0.0f;
		_fCamPitch = 0.0f;
	}
	
	// see if we should use the camera bookmark
	bKeyDown = bHaveFocus && (GetAsyncKeyState( 'C' ) < 0);
	if( bKeyDown ) {
		if( _bBookmarkValid ) {
			_WorldCamPos = _BookmarkPos;
			_fCamZoom = _fBookmarkZoom;
			_fCamHeading = _fBookmarkHeading;
			_fCamPitch = _fBookmarkPitch;
		}
	} else {
		// see if we should set a camera bookmark
		bKeyDown = bHaveFocus && (GetAsyncKeyState( 'B' ) < 0);
		if( bKeyDown ) {
			_SetCameraBookmark();
		}
	}

	CMaiUserInput MaiUi;
	MaiUi.bLastLeftOnDown = _bLastLeftOnDown;
	MaiUi.bLastRightOnDown = _bLastRightOnDown;
	MaiUi.bLastLeftOnUp = _bLastLeftOnUp;
	MaiUi.bLastRightOnUp = _bLastRightOnUp;
	MaiUi.bLastLeftButton = _bLastLeftButton ;
	MaiUi.bLastRightButton = _bLastRightButton;
	MaiUi.nLastMouseX = _nLastMouseX;
	MaiUi.nLastMouseY = _nLastMouseY;
	MaiUi.nDeltaMouseX  = _nDeltaMouseX;
	MaiUi.nDeltaMouseY = _nDeltaMouseY;
	maimain_Work(MaiUi);
	
	if( !fvid_Swap() ) {
		return FALSE;
	}

	fvid_Begin();

	//////////////////////////////////////////////////////
	// Init our viewport, depending on what mode we are in
/*
	if( _b2dMode ) {
		// init the ortho 3d viewport
		fNearPlane = ( (_pWorldRes->fNodeBoxMaxY_WS + 5.0f) - _fOrthoCamY ) * _fCamZoom;
		if( fNearPlane < 0.0f ) {
			fNearPlane *= -1.0f;
		}
		FMATH_CLAMPMIN( fNearPlane, 0.1f );
		fFarPlane = ( (_pWorldRes->fNodeBoxMinY_WS - 5.0f) - _fOrthoCamY ) * _fCamZoom;
		if( fFarPlane < 0.0f ) {
			fFarPlane *= -1.0f;
		}
		FMATH_CLAMPMIN( fFarPlane, (fNearPlane + 5.0f) );
		fviewport_InitOrtho3D( _pViewport, fNearPlane, fFarPlane );
	} 
	else 
*/
	{
		fviewport_InitPersp( _pViewport, FMATH_QUARTER_PI, 0.1f, 5000.0f );	
	}
	fviewport_SetActive( _pViewport );
	if( _b2dMode ) {
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );
	} else {
		fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, pDlg->m_3DBGColorRGB.fRed, pDlg->m_3DBGColorRGB.fGreen, pDlg->m_3DBGColorRGB.fBlue, 1.0f, 0 );
	}

	if( _pWorldRes ) {
		
		/////////////////////
		// Do our view's work
		if( _b2dMode ) {
			_2DWork( bHaveFocus );
		} else {
			_3DWork( bHaveFocus );
		}

		//////////////////////////////////////////////
		// grab the current node that the camera is in
//		pWorldNode = fworld_GetNodeIntersectingPoint( _pWorldRes->nLeafLevelNum, &k9dlg_XfmCam.m_MtxR.m_vPos.v3 );
		
		k9dlg_XfmCam.InitStackWithView();
		
		/////////////////
		// draw the world
		fvis_FrameBegin();
		if( _b2dMode ) {
			fworld_EnableFog( FALSE );
			fvis_Draw( NULL, TRUE );
		} else {
			fworld_EnableFog( FALSE );
			fvis_Draw( NULL, FALSE );
		}
		fvis_FrameEnd();
				
		///////////////////////
		//  Draw the graph nodes and edges etc
		maimain_Draw(MaiUi);

		
		//////////////////////
		// print some text out
		s32 sTopPixel = 18;

		if( pDlg->m_bDrawFPS ) {
			ftext_DebugPrintf( 0.01f, 0.715f, "FPS: ~w1%.1f", 1.0f / _fFrameSecs );
		}
		if( pDlg->m_bDrawCamInfo ) {
			if( _b2dMode ) {
				ftext_DebugPrintf( 0.99f, 0.01f, "~arCam XZ: ~w1%7.1f %7.1f", _WorldCamPos.x, _WorldCamPos.z );
				ftext_DebugPrintf( 0.99f, 0.035f, "~arScale: ~w1%6.1f", _fCamZoom );
			} else {
				f32 fHeadingDeg = FMATH_RAD2DEG( _fCamHeading );
				while( fHeadingDeg >= 360.0f ) {
					fHeadingDeg -= 360.0f;
				}
				while( fHeadingDeg < 0.0f ) {
					fHeadingDeg += 360.0f;
				}
				ftext_DebugPrintf( 0.99f, 0.01f, "~arCam XYZ: ~w1%7.1f %7.1f %7.1f", _WorldCamPos.x, _WorldCamPos.y, _WorldCamPos.z );
				ftext_DebugPrintf( 0.99f, 0.035f, "~arHeading: ~w1%7.1f ~w0Pitch: ~w1%7.1f", fHeadingDeg, FMATH_RAD2DEG( _fCamPitch ) );
			}
		}				
	}

	fvid_End();

	_fFrameSecs = Timer.SampleSeconds( TRUE );

	if( pDlg->m_bDrawPerfStats && !_b2dMode ) {
		_DisplayPerf();	
	}

	screenshot_Work();

	_nDeltaMouseX = 0;
	_nDeltaMouseY = 0;
	_nDeltaWheel = 0;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// WARNING...
// THIS IS VERY TOUCHY BECAUSE IT CAN BE CALLED AT ANY TIME, EVEN WHILE YOUR MAIN IS EXECUTING!!!
// KEEP WORK TO A BARE MIN, AND DON'T ACCESS RENDERING DATA
/////////////////////////////////////////////////////////////////////////////////////////////////
static void _GameMouseCallback( u32 nX, u32 nY, s32 nDeltaWheel, BOOL bLeftButton, BOOL bRightButton, void *pParameter ) {
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CK9Dlg *pDlg = pParm->pDlg;
		
	if( !_bMainLoopExecuted ) {
		return;
	}
	if( _nFirstMouseActivity ==1) {
		_nFirstMouseActivity = 2;
		_nDeltaMouseX = 0;
		_nDeltaMouseY = 0;
	} else {
	   _nFirstMouseActivity = 0;
		_nDeltaMouseX = (s32)nX - (s32)_nLastMouseX;
		_nDeltaMouseY = (s32)nY - (s32)_nLastMouseY;
	}
	_nDeltaWheel = nDeltaWheel;
	
	if( bLeftButton ) {
		// set flag to process during next main gameloop
		_bProcessLButton = TRUE;
	}
	if( bRightButton ) {
		// set flag to process during next main gameloop
		_bProcessRButton = TRUE;
	}

	// record the last mouse state for use later on
	_nLastMouseX = nX;
	_nLastMouseY = nY;
	_bLastLeftButton = bLeftButton;
	_bLastRightButton = bRightButton;
}

void CK9Dlg::OnButtonSave() {
	CString sError, sFilename;

	if( m_bRunning ) {
		floop_PauseGameloop( TRUE );
		Sleep( 100 );// hack until gameloop is fixed
		
		// do save work here...

		floop_PauseGameloop( FALSE );
	}
}

void CK9Dlg::OnChooseDisplayListDir() {

	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Display List Directory", m_sDisplayListDir ) == PICKDIR_RET_SUCCESS ) {
		m_sDisplayListDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );
	}
}

static void _2DWork( BOOL bHaveFocus ) {
}

static void _3DWork( BOOL bHaveFocus ) {
	BOOL bKeyDown;

	///////////////////
	// build our camera
	k9dlg_XfmCam.BuildInvRotYXZ_XlatFromPoint( _fCamHeading, _fCamPitch, 0.0f, _WorldCamPos.x, _WorldCamPos.y, _WorldCamPos.z );
	
	//////////////////////
	// process mouse input
	if( _bProcessRButton ) {
		_fCamHeading += ((f32)_nDeltaMouseX * 0.01f);
		_fCamPitch += ((f32)_nDeltaMouseY * 0.01f);
		FMATH_CLAMP( _fCamPitch, -FMATH_HALF_PI, FMATH_HALF_PI );

		_bProcessRButton = FALSE;
	}
	if( _bProcessLButton ) {
		
		_bProcessLButton = FALSE;
	}
	/////////////////////////
	// process keyboard input

	CFVec3 KillAxis(1.0f, 1.0f, 1.0f);
	if ( _b3DCam_KillYAxis)
	{
		KillAxis.y = 0.0f;
	}
	BOOL bDetailMode = bHaveFocus && ( (GetAsyncKeyState( ' ' ) < 0));

	f32 fCamXlatScale = 1.0f - bDetailMode*0.8f;


	bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'W' ) < 0) || (GetAsyncKeyState( VK_UP ) < 0) );
	if( bKeyDown ) {
		// Forward...
		_WorldCamPos += k9dlg_XfmCam.m_MtxR.m_vFront.v3 * KillAxis * fCamXLatSpeed * fCamXlatScale* FLoop_fPreviousLoopSecs;
	}
	bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'S' ) < 0) || (GetAsyncKeyState( VK_DOWN ) < 0) );
	if( bKeyDown ) {
		// Backwards...
		_WorldCamPos -= k9dlg_XfmCam.m_MtxR.m_vFront.v3 * KillAxis * fCamXLatSpeed * fCamXlatScale*FLoop_fPreviousLoopSecs;
	}
	bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'A' ) < 0) || (GetAsyncKeyState( VK_LEFT ) < 0) );
	if( bKeyDown ) {
		// Strafe Left...
		_WorldCamPos -= k9dlg_XfmCam.m_MtxR.m_vRight.v3 * fCamXLatSpeed * fCamXlatScale*FLoop_fPreviousLoopSecs;
	}
	bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'D' ) < 0) || (GetAsyncKeyState( VK_RIGHT ) < 0) );
	if( bKeyDown ) {
		// Strafe Right...
		_WorldCamPos += k9dlg_XfmCam.m_MtxR.m_vRight.v3 * fCamXLatSpeed * fCamXlatScale*FLoop_fPreviousLoopSecs;
	}
	bKeyDown = bHaveFocus && (GetAsyncKeyState( 'R' ) < 0);
	if( bKeyDown ) {
		// Vertical Up...
		_WorldCamPos +=  k9dlg_XfmCam.m_MtxR.m_vUp.v3 * fCamXLatSpeed * fCamXlatScale*FLoop_fPreviousLoopSecs;
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( 'F' ) < 0);
	if( bKeyDown ) {
		// Vertical Down...
		_WorldCamPos -= k9dlg_XfmCam.m_MtxR.m_vUp.v3 * fCamXLatSpeed * fCamXlatScale*FLoop_fPreviousLoopSecs;
	}
	bKeyDown = bHaveFocus && (GetAsyncKeyState( 'Q' ) < 0);
	if( bKeyDown ) {
		// Spin Left...
		_fCamHeading -= _CAM_SPIN_SPEED;
	}
	bKeyDown = bHaveFocus && (GetAsyncKeyState( 'E' ) < 0);
	if( bKeyDown ) {
		// Spin Right...
		_fCamHeading += _CAM_SPIN_SPEED;
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_ADD ) < 0);
	if( bKeyDown ) {
		//camp speed up
		fCamXLatSpeed += 0.5f;
	}
	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_SUBTRACT ) < 0);
	if( bKeyDown ) {
		//camp speed down
		fCamXLatSpeed -= 0.5f;
		if (fCamXLatSpeed < 0.0f)
		{
			fCamXLatSpeed = 0.0f;
		}
	}

	if( _nDeltaWheel ) {
		_WorldCamPos.y += ( _nDeltaWheel* 0.005f );
	}
	
}

void CK9Dlg::On3dColor() {
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

void CK9Dlg::SavePromptBeforeExit() {

	// ask if they want to save
	if( maimain_pEditGraph &&
		this->MessageBox( "Save AI data file?", "MAI -- AI Utility", MB_YESNO | MB_ICONEXCLAMATION ) == IDYES )
	{
		CString sFilename, sError;
		// do save work here...
		maimain_MakeBackupCopy(maimain_szGraphFileFullPath, m_sScreenshotDir);

		BOOL bDidSave = maimain_pEditGraph->SaveToAsciiFile(maimain_szGraphFileFullPath);
		while (!bDidSave && MessageBox("    Save Failed ", "File Error", MB_RETRYCANCEL) == IDRETRY)
		{
			bDidSave = maimain_pEditGraph->SaveToAsciiFile(maimain_szGraphFileFullPath);
		}
	}
}

static void _DisplayPerf( void ) {
	f32 fFPS;

	fFPS = 1.0f / _fFrameSecs;
/*
	ftext_DebugPrintf( 0.99f, 0.44f,	"~arObjs:~w1%6u", FPerf_nMeshInstDrawnCount );
	ftext_DebugPrintf( 0.99f, 0.465f,	"~arNodes:~w1%6u", FPerf_nMeshNodeDrawnCount );
	ftext_DebugPrintf( 0.99f, 0.49f,	"~arMtls:~w1%6u", FPerf_nMeshMtlDrawnCount );
	ftext_DebugPrintf( 0.99f, 0.515f,	"~arTris:~w1%6u", FPerf_nRawTriangleCount );
	ftext_DebugPrintf( 0.99f, 0.54f,	"~arVtxs:~w1%6u", FPerf_nRawVertexCount );
	ftext_DebugPrintf( 0.99f, 0.565f,	"~arKT/s:~w1%6u", (u32)( (f32)FPerf_nRawTriangleCount * fFPS * (1.0f/1000.0f) ) );
	ftext_DebugPrintf( 0.99f, 0.59f,	"~arKV/s:~w1%6u", (u32)( (f32)FPerf_nRawVertexCount * fFPS * (1.0f/1000.0f) ) );
	ftext_DebugPrintf( 0.99f, 0.615f,	"~arV/T:~w1%6.1f", FPerf_nRawTriangleCount ? (f32)FPerf_nRawVertexCount / (f32)FPerf_nRawTriangleCount : 0.0f );
	ftext_DebugPrintf( 0.99f, 0.64f,	"~arMB/s:~w1%6u", (u32)( (f32)FPerf_nRawGeoByteCount * fFPS * (1.0f/1000000.0f) ) );
	ftext_DebugPrintf( 0.99f, 0.665f,	"~arTex Switch:~w1%4u/%3u", FPerf_nRawTexSwitchCount, FPerf_nUniqueTexturesUsed );
	ftext_DebugPrintf( 0.99f, 0.69f,	"~arVB Switch:~w1%6u", FPerf_nRawVBSwitchCount );
	ftext_DebugPrintf( 0.99f, 0.715f,	"~arSha Switch:~w1%6u", FPerf_nRawShSwitchCount );	
*/
}

void CK9Dlg::OnDrawStats() {
	UpdateData( CONTROLS_TO_VARS );
	m_bDrawPerfStats = ((CButton *)GetDlgItem(IDC_DRAW_STATS))->GetCheck();
	UpdateData( VARS_TO_CONTROLS );	
}

static void _DevPrintf( cchar *pszFormat, FANG_VA_LIST Args ) {
	_pWinPrintf->Printf( pszFormat, Args );
}

void CK9Dlg::OnDrawFpsClick() {
	UpdateData( CONTROLS_TO_VARS );
	m_bDrawFPS = ((CButton *)GetDlgItem(IDC_DRAW_FPS))->GetCheck();
	UpdateData( VARS_TO_CONTROLS );
}

void CK9Dlg::OnDrawCamStatsClick() {
	UpdateData( CONTROLS_TO_VARS );
	m_bDrawCamInfo = ((CButton *)GetDlgItem(IDC_DRAW_CAM_STATS))->GetCheck();
	UpdateData( VARS_TO_CONTROLS );
}


void CK9Dlg::OnChooseScreenshotDir() {
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Screenshot Directory", m_sScreenshotDir ) == PICKDIR_RET_SUCCESS ) {
		m_sScreenshotDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CK9Dlg::OnSeries() {
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_SERIES ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotMode( FALSE );
	}
	UpdateData( VARS_TO_CONTROLS );		
}

void CK9Dlg::OnSingle() { 
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_SINGLE ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotMode( TRUE );
	}
	UpdateData( VARS_TO_CONTROLS );		
}

void CK9Dlg::OnHideCursorClick() {
	m_bDrawCursor = !( ((CButton *)GetDlgItem(IDC_HIDE_CURSOR))->GetCheck() );		
}

void CK9Dlg::OnTgaClick() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_TGA ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotType( SCREENSHOT_OUTPUT_TYPE_TGA );
	}
}

void CK9Dlg::OnJpegClick() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_JPEG ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotType( SCREENSHOT_OUTPUT_TYPE_JPEG );
	}
}

void CK9Dlg::OnSelectTool()
{
}


void _SetCameraBookmark(void)
{
	_bBookmarkValid = TRUE;
	_BookmarkPos = _WorldCamPos;
	_fBookmarkZoom = _fCamZoom;
	_fBookmarkHeading = _fCamHeading;
	_fBookmarkPitch = _fCamPitch;
}


