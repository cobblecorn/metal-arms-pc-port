//////////////////////////////////////////////////////////////////////////////////////
// jawsDlg.cpp - 
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
// 07/19/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "jaws.h"
#include "jawsDlg.h"
#include "vidmode.h"
#include "settings.h"
#include "pickdir.h"
#include "fversion.h"
#include "PickAsset.h"

#include "fang.h"
#include "floop.h"
#include "fviewport.h"
#include "fmath.h"
#include "fresload.h"
#include "fres.h"
#include "fmesh.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "ftimer.h"
#include "fboxfilter.h"
#include "ftext.h"
#include "fanim.h"
#include "fcoll.h"
#include "fperf.h"
#include "fdraw.h"
#include "fclib.h"
#include "frenderer.h"
#include "fpsprite.h"	//xxxxxxxxxx
#include "dx/fdx8mesh.h"	//xxxxxxxxxx
#include "fshadow.h"	//xxxxxxxxxx
#include "fmovie.h"
#include "ffile.h"
#include "fvis.h"
#include "frendersort.h"

#include "arcball.h"

#include "dx/fdx8vid.h"
#include "dx/fdx8.h"
#include "dx/fdx8mesh.h"
#include "dx/fdx8xfm.h"
#include "screenshot.h"

//====================
// private definitions

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

#define _MM_GAMELOOP_RUN		(WM_USER + 173)
#define _MM_GAMELOOP_TERM		(WM_USER + 174)
#define _MM_GAMELOOP_LOADED		(WM_USER + 175)
#define _MM_GAMELOOP_LOADERR	(WM_USER + 176)
#define _MM_GAMELOOP_LOAD		(WM_USER + 177)
#define _MM_GAMELOOP_LOADANIM1	(WM_USER + 179)
#define _MM_GAMELOOP_LOADANIM2	(WM_USER + 180)
#define _MM_GAMELOOP_LOADANIM3	(WM_USER + 181)
#define _MM_GAMELOOP_LOADANIM4	(WM_USER + 182)

#define _REGKEY_CONFIG			JAWS_REG_KEY "\\" "Config"
#define _REGKEY_CONFIGDISK		JAWS_REG_KEY "\\" "ConfigDisk"

#define _PICKRAY_LENGTH			100000.0f

#define _COLORRGB_TO_COLORREF( pColorRGB ) ( (u32)((pColorRGB)->fRed * 255.0f) | ((u32)((pColorRGB)->fGreen * 255.0f) << 8) | ((u32)((pColorRGB)->fBlue * 255.0f) << 16) )
#define _COLORREF_TO_COLORRGB( pColorRGB, nColorRef ) (pColorRGB)->Set( (f32)((nColorRef) & 0xff) * (1.0f/255.0f), (f32)(((nColorRef)>>8) & 0xff) * (1.0f/255.0f), (f32)(((nColorRef)>>16) & 0xff) * (1.0f/255.0f) )

#define _SERIALIZE_VERSION		2

#define _MIN_FOV				30
#define _MAX_FOV				90
#define _NUM_FOV_SETTINGS		( _MAX_FOV - _MIN_FOV )


typedef enum {
	_ANIM_PLAYBACK_30FPS = 0,
	_ANIM_PLAYBACK_60FPS,
	_ANIM_PLAYBACK_REALTIME,
	_ANIM_PLAYBACK_SLOWMOTION,

	_ANIM_PLAYBACK_COUNT
} _AnimPlaybackRates_e;

typedef enum {
	_MESH_DRAW_MODES_REGULAR = 0,
	_MESH_DRAW_MODES_ROOT_DOPS,
	_MESH_DRAW_MODES_ROOT_DOPS_PLUS_LEAF_TRIS,
	_MESH_DRAW_MODES_LEAF_DOPS,
	_MESH_DRAW_MODES_LEAF_TRIS,

	_MESH_DRAW_MODES_COUNT
} _MeshDrawModes_e;

//=================
// public variables

//==================
// private variables

static f32 _fAvgFrameSecs;
static FBoxFilterHandle_t _hBoxFilter_Potential;
static FBoxFilterHandle_t _hBoxFilter_Governed;
static BOOL _bFrameRate = FALSE;

static FViewport_t *_pViewport;
static f32 _fNewFOV, _fLastFOV;
//static FResFrame_t _ResFrame_MeshAnim;
static FResFrame_t _ResFrame_Anim;
static FMesh_t *_pMeshRes;
static CFMeshInst _MeshInst;
static FVisData_t *_pWorldRes;
static BOOL _bWorldMode;
static BOOL _bMainLoopExecuted;
static u32 _nMeshDrawMode = _MESH_DRAW_MODES_REGULAR;

static CFVec3 _WorldCamPos;
static f32 _fWorldCamHeading;
static f32 _fWorldCamPitch;

static BOOL _bFirstMouseActivity;
static u32 _nLastMouseX;
static u32 _nLastMouseY;

static CArcBall *_pArcBall;

static CFXfm _XfmCam;
static CFXfm _XfmModelOrigin;
static CFLight _LightDir;

static CFAnimCombinerConfig *_pCombinerConfig;
static CFAnimCombiner *_pCombiner;
static s32 _anControlID[3];
static s32 _anTapID[4];
static FAnim_t *_apResAnim[4];
static CFAnimInst *_apAnimInst[4];
static BOOL _abAnimRunning[4];

static BOOL _bLastLeftButton;
static BOOL _bLastRightButton;

static _AnimPlaybackRates_e _nAnimPlaybackRate = _ANIM_PLAYBACK_REALTIME;
static f32 _fFrameRateMultiplier = 1.0f;

static u32 _anPartnerIndex[4] = { 1, 0, 3, 2 };

static CWinPrintf *_pWinPrintf;

//===================
// private prototypes

static BOOL _GameInit( void *pParameter );
static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter );
static BOOL _GameMain( BOOL bExitRequest, void *pParameter );

static void _GameMouseCallback( u32 nX, u32 nY, s32 nDeltaWheel, BOOL bLeftButton, BOOL bRightButton, void *pParameter );
static void _AdjustMixer( s32 nControlID, s32 nDeltaWheel );

static void _FreeAnimResources( void );
static BOOL _LoadAnimFiles( CJawsDlg *pDlg );
static void _DisplayMixerAdjust( s32 nControlID );
static void _UpdateAnimations( CJawsDlg *pDlg, f32 fSecondsSinceLastCall );
static void _TriggerOneShotAnim( s32 nTapID, CJawsDlg *pDlg );
static void _DisplayPerf( void );



static void _DevPrintf( cchar *pszFormat, FANG_VA_LIST Args );

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
// CJawsDlg dialog

CJawsDlg::CJawsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJawsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJawsDlg)
	m_sVersion = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJawsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text( pDX, IDC_FORCE_LOD, m_sForceLOD );
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJawsDlg)
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CJawsDlg, CDialog)
	//{{AFX_MSG_MAP(CJawsDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_VIDMODE, OnButtonVidmode)
	ON_BN_CLICKED(IDC_BUTTON_RUN, OnButtonRun)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, OnButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_BACKGROUND, OnButtonBackground)
	ON_BN_CLICKED(IDC_BUTTON_AMBIENT, OnButtonAmbient)
	ON_BN_CLICKED(IDC_BUTTON_DIRECTIONAL, OnButtonDirectional)
	ON_BN_CLICKED(IDC_CHECK_AMBIENT, OnCheckAmbient)
	ON_BN_CLICKED(IDC_CHECK_BACKGROUND, OnCheckBackground)
	ON_BN_CLICKED(IDC_CHECK_DIRECTIONAL, OnCheckDirectional)
	ON_BN_CLICKED(IDC_BUTTON_ANIM1, OnButtonAnim1)
	ON_BN_CLICKED(IDC_BUTTON_ANIM2, OnButtonAnim2)
	ON_BN_CLICKED(IDC_BUTTON_ANIM3, OnButtonAnim3)
	ON_BN_CLICKED(IDC_BUTTON_ANIM4, OnButtonAnim4)
	ON_BN_CLICKED(IDC_BUTTON_X1, OnButtonX1)
	ON_BN_CLICKED(IDC_CHECK_LINK1, OnCheckLink1)
	ON_BN_CLICKED(IDC_CHECK_LINK2, OnCheckLink2)
	ON_BN_CLICKED(IDC_BUTTON_X2, OnButtonX2)
	ON_BN_CLICKED(IDC_BUTTON_X3, OnButtonX3)
	ON_BN_CLICKED(IDC_BUTTON_X4, OnButtonX4)
	ON_BN_CLICKED(IDC_CHECK_LOOP1, OnCheckLoop1)
	ON_BN_CLICKED(IDC_CHECK_LOOP2, OnCheckLoop2)
	ON_BN_CLICKED(IDC_CHECK_LOOP3, OnCheckLoop3)
	ON_BN_CLICKED(IDC_CHECK_LOOP4, OnCheckLoop4)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_ANIM, OnCheckEnableAnim)
	ON_BN_CLICKED(IDC_CHECK_PAUSE_ANIM, OnCheckPauseAnim)
	ON_BN_CLICKED(IDC_BUTTON_SAVECONFIG, OnButtonSaveconfig)
	ON_BN_CLICKED(IDC_BUTTON_LOADCONFIG, OnButtonLoadconfig)
	ON_BN_CLICKED(IDC_CHECK_PERF, OnCheckPerf)
	ON_BN_CLICKED(IDC_CHECK_SHOW_BOUNDS, OnCheckShowBounds)
	ON_BN_CLICKED(IDC_HIDE_CURSOR, OnHideCursorClick)
	ON_BN_CLICKED(IDC_30FPS, On30fpsClick)
	ON_BN_CLICKED(IDC_60FPS, On60fpsClick)
	ON_BN_CLICKED(IDC_REALTIME_FPS, OnRealtimeFpsClick)
	ON_BN_CLICKED(IDC_SERIES, OnSeriesClick)
	ON_BN_CLICKED(IDC_SINGLE, OnSingleClick)
	ON_BN_CLICKED(IDC_CHECK_FPS, OnCheckFpsClick)
	ON_BN_CLICKED(IDC_TGA, OnTgaClick)
	ON_BN_CLICKED(IDC_JPEG, OnJpegClick)
	ON_BN_CLICKED(IDC_DONT_RENDER_SURFACE, OnDontRenderSurface)
	ON_BN_CLICKED(IDC_DONT_RENDER_SPECULAR, OnDontRenderSpecular)
	ON_BN_CLICKED(IDC_REALTIME_SLOW10, OnRealtimeSlow10)
	ON_BN_CLICKED(IDC_REALTIME_SLOW20, OnRealtimeSlow20)
	ON_BN_CLICKED(IDC_REALTIME_SLOW30, OnRealtimeSlow30)
	ON_BN_CLICKED(IDC_REALTIME_SLOW40, OnRealtimeSlow40)
	ON_BN_CLICKED(IDC_REALTIME_SLOW50, OnRealtimeSlow50)
	ON_BN_CLICKED(IDC_REALTIME_SLOW75, OnRealtimeSlow75)
	ON_BN_CLICKED(IDC_REALTIME_SLOW100, OnRealtimeSlow100)
	ON_MESSAGE(_MM_GAMELOOP_RUN, OnGameloopRun)
	ON_MESSAGE(_MM_GAMELOOP_TERM, OnGameloopTerm)
	ON_MESSAGE(_MM_GAMELOOP_LOADED, OnGameloopLoaded)
	ON_MESSAGE(_MM_GAMELOOP_LOADERR, OnGameloopLoadErr)
	ON_MESSAGE(_MM_GAMELOOP_LOAD, OnGameloopLoad)
	ON_MESSAGE(_MM_GAMELOOP_LOADANIM1, OnGameloopLoadAnim1)
	ON_MESSAGE(_MM_GAMELOOP_LOADANIM2, OnGameloopLoadAnim2)
	ON_MESSAGE(_MM_GAMELOOP_LOADANIM3, OnGameloopLoadAnim3)
	ON_MESSAGE(_MM_GAMELOOP_LOADANIM4, OnGameloopLoadAnim4)
	ON_MESSAGE(APP_MSG_CODES_JAWS_LAUNCH, OnLaunchMsg)
	ON_BN_CLICKED(IDC_CHECK_SHOW_BONES, OnCheckShowBones)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LOD_SLIDER, OnNMCustomDrawLODSlider)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_FOV_SLIDER, OnNMCustomdrawFovSlider)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CJawsDlg message handlers

BOOL CJawsDlg::OnInitDialog()
{
	u32 i;

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	//LightMap Param
	CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_LOD_SLIDER );
	pSlider->SetRange(0, FDATA_MAX_LOD_MESH_COUNT);
	pSlider->SetPos( 0 );
	m_sForceLOD = "Force LOD disabled";

	// the FOV slider
	pSlider = (CSliderCtrl *)GetDlgItem( IDC_FOV_SLIDER );
	pSlider->SetRange(0, _NUM_FOV_SETTINGS);
	pSlider->SetPos( _NUM_FOV_SETTINGS );
    _fNewFOV = FMATH_DEG2RAD( 90.0f );
	_fLastFOV = _fNewFOV;

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->InsertMenu( 0, MF_STRING, IDM_ABOUTBOX, strAboutMenu );
			//pSysMenu->AppendMenu(MF_SEPARATOR);
			//pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		pSysMenu->EnableMenuItem( 2, MF_BYPOSITION | MF_GRAYED );
		pSysMenu->EnableMenuItem( 4, MF_BYPOSITION | MF_GRAYED );
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	::SetProp( GetSafeHwnd(), Jaws_pszPropName, (HANDLE)1 );

	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );

	UpdateData( VARS_TO_CONTROLS );

	_bFrameRate = TRUE;

	_bLastLeftButton = FALSE;
	_bLastRightButton = FALSE;
	_pArcBall = NULL;
	m_bReady = FALSE;
	m_bRunning = FALSE;
	m_bLoad = FALSE;
	m_bOrientSet = FALSE;
	m_bDataBeingUpdated = FALSE;
	m_bReloadMasterFile = FALSE;
	m_GameloopParm.pDlg = this;
	_bMainLoopExecuted = FALSE;
	m_bShowBound = FALSE;
	m_bShowBones = FALSE;

	m_bEnableAnim = TRUE;
	m_bPauseAnim = FALSE;
	m_abLink[0] = FALSE;
	m_abLink[1] = FALSE;

	for( i=0; i<4; i++ ) {
		m_abLoadAnim[i] = FALSE;
		m_abLoop[i] = TRUE;
		m_asAnimResName[i].Empty();
		_abAnimRunning[i] = FALSE;
	}

	// set the default animation playback rate
	_nAnimPlaybackRate = _ANIM_PLAYBACK_REALTIME;
	_fFrameRateMultiplier = 1.0f;
	((CButton *)GetDlgItem( IDC_REALTIME_FPS ))->SetCheck( TRUE );

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

	// init the fang system
	_pWinPrintf = &m_WinPrintf;
	m_WinPrintf.AttachToEditControl( IDC_EDIT_PRINTBOX, this, "" );
	m_WinPrintf.Enable( TRUE );

	fang_Init();
	Fang_ConfigDefs.nRes_HeapBytes = (16*1024*1024);

	CSettings SettingsDlg;
	SettingsDlg.GetCurrentSettings( &m_Settings );
	
	if( m_Settings.sMasterFile.IsEmpty() ) {
		MessageBox( _T("You must select a master file before Jaws\ncan be used.  Click OK and then select\na master file."), 
					_T("Jaws Error: No Master File"),
					MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);	
		CSettings SettingsDlg;
		
		if( SettingsDlg.DoModal() == IDOK ) {
			SettingsDlg.GetCurrentSettings( &m_Settings );
		} else {
			// no master file, no Jaws
			PostMessage( WM_COMMAND, IDCANCEL, 0 );  
			return TRUE;
		}		
	}

	Fang_ConfigDefs.pszFile_MasterFilePathName = m_Settings.sMasterFile;
	Fang_ConfigDefs.pFang_FcnPrintf = _DevPrintf;

	GetSettingsFromRegistry( FALSE );

	m_bReady = fang_Startup();

	if( !m_bReady ) {
		// could not install the fang engine, tell the user and exit
		MessageBox( "Could not start Fang engine.", "Jaws: fang_Startup() Error", MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 );  
		return TRUE;
	} else {
		CVidMode VidMode;
		if( !VidMode.GetVidMode( &m_GameloopParm.VidWin ) ) {
			// Video mode not selected...
			MessageBox( "You need to set the video mode before Jaws may be used.\nClick Ok and then click the Video button.", "Jaws: Set Video Mode" );
			m_bReady = FALSE;
		}

		m_GameloopParm.VidWin.bAllowPowerSuspend = TRUE;
		m_GameloopParm.VidWin.hInstance = theApp.m_hInstance;
		m_GameloopParm.VidWin.hWnd = 0;
		m_GameloopParm.VidWin.nIconIDI = IDR_MAINFRAME;
		m_GameloopParm.VidWin.pFcnSuspend = NULL;
		strcpy( m_GameloopParm.VidWin.szWindowTitle, "Jaws Art Utility" );

		_MeshInst.m_Xfm.Identity();
		_XfmModelOrigin.Identity();
		_XfmCam.Identity();
	}

	frenderer_DrawBound_Enable( m_bShowBound );
	frenderer_DrawBones_Enable( m_bShowBones );

	((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( m_bReady );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CJawsDlg::OnSysCommand( UINT nID, LPARAM lParam ) {

	if( (nID & 0xFFF0) == IDM_ABOUTBOX ) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialog::OnSysCommand( nID, lParam );
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CJawsDlg::OnPaint() {

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CJawsDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

void CJawsDlg::OnCancel() {
	if( m_bRunning ) {
		OnButtonRun();
	}

	StoreSettingsIntoRegistry( FALSE );

	fang_Shutdown();

	::RemoveProp( GetSafeHwnd(), Jaws_pszPropName );

	CDialog::OnCancel();
}

void CJawsDlg::OnOK() {
	//OnCancel();
}

BOOL CJawsDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CJawsDlg::OnButtonVidmode() {
	CVidMode VidMode;

	((CButton *)GetDlgItem( IDC_BUTTON_VIDMODE ))->EnableWindow( FALSE );
	VidMode.DoModal();
	((CButton *)GetDlgItem( IDC_BUTTON_VIDMODE ))->EnableWindow( TRUE );

	if( VidMode.GetVidMode( &m_GameloopParm.VidWin ) ) {
		if( fang_IsStartedUp() ) {
			m_bReady = TRUE;
			((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( TRUE );
		}
	}
}

long CJawsDlg::OnLaunchMsg( WPARAM wParam, LPARAM lParam ) {
	u32 i;

	DEVPRINTF( "\nJAWS: WE HAVE RECEIVED A LAUNCH MSG FROM ANOTHER PROCESS.\n" );

	if( Jaws_SharedAppData->aAppData[APP_TYPE_JAWS].nNumStrings ) {
		
		if( m_bRunning ) {
			// we are currently running, stop it from running
			DEVPRINTF( "JAWS: Stopping the currently running file.\n" );
			OnButtonRun();// we can't post a message since the stopping process also posts messages (they would be out of order)
		}

		// change the input filename
		CString sTemp = Jaws_SharedAppData->aAppData[APP_TYPE_JAWS].szString;
		int nIndex = sTemp.Find( '.' );
		CString sExt = sTemp.Mid( nIndex+1 );
		// determine what type of file we have
		if( sExt[0] == 'a' && sExt[1] == 'p' && sExt[2] == 'e' ) {
			// we have a mesh file
			m_sResName = sTemp.Left( nIndex );
			m_sResType = FMESH_RESTYPE;
			DEVPRINTF( "JAWS: Changing to the mesh file: %s\n", m_sResName );

			// see if there are optional animation filenames too
			if( Jaws_SharedAppData->aAppData[APP_TYPE_JAWS].nNumStrings > 1 ) {	
				// reset the animation vars
				m_bEnableAnim = TRUE;
				m_bPauseAnim = FALSE;
				m_abLink[0] = TRUE;
				m_abLink[1] = FALSE;
				for( i=0; i<4; i++ ) {
					m_abLoadAnim[i] = FALSE;
					m_abLoop[i] = TRUE;
					m_asAnimResName[i].Empty();
					_abAnimRunning[i] = FALSE;
				}
				// fill in the first animation file
				m_asAnimResName[0] = sExt.Mid( 4 );
				m_abLoadAnim[0] = TRUE;
				nIndex = m_asAnimResName[0].Find( ':' );
				if( nIndex > 0 ) {
					m_asAnimResName[1] = m_asAnimResName[0].Mid( nIndex + 1 );
					m_abLoadAnim[1] = TRUE;					
				}
				// remove the ext off of the animation file names
				nIndex = m_asAnimResName[0].Find( '.' );
				if( nIndex > 0 ) {
					m_asAnimResName[0].Delete( nIndex, m_asAnimResName[0].GetLength() - nIndex );
				}
				nIndex = m_asAnimResName[1].Find( '.' );
				if( nIndex > 0 ) {
					m_asAnimResName[1].Delete( nIndex, m_asAnimResName[1].GetLength() - nIndex );
				}
				// print out the filenames
				if( !m_asAnimResName[0].IsEmpty() ) {
					DEVPRINTF( "JAWS: Changing animation 1 file: %s\n", m_asAnimResName[0] );
				}
				if( !m_asAnimResName[1].IsEmpty() ) {
					DEVPRINTF( "JAWS: Changing animation 2 file: %s\n", m_asAnimResName[1] );
				}
			} else {
				// no animation files passed in, disable the animation vars
				m_bEnableAnim = FALSE;
				m_bPauseAnim = FALSE;
				m_abLink[0] = FALSE;
				m_abLink[1] = FALSE;
#if 0// don't disable the animation settings, just turn off the enable checkbox
				for( i=0; i<4; i++ ) {
					m_abLoadAnim[i] = FALSE;
					m_abLoop[i] = TRUE;
					m_asAnimResName[i].Empty();
					_abAnimRunning[i] = FALSE;
				}
#endif
			}
		} else if( sExt[0] == 'w' && sExt[1] == 'l' && sExt[2] == 'd' ) {
			// we have a world file
			m_sResName = sTemp.Left( nIndex );
			m_sResType = FWORLD_RESTYPE;
			DEVPRINTF( "JAWS: Changing to the world file: %s\n", m_sResName );

			// don't do anything to the current animation settings
		}
		
		UpdateData( VARS_TO_CONTROLS );

		// fix up the control names
		UpdateControls();

		// do the same thing as clicking run
		DEVPRINTF( "JAWS: Posting a msg to run the new world file.\n" );
		PostMessage( WM_COMMAND, IDC_BUTTON_RUN, 0 );
	} else {
		DEVPRINTF( "JAWS: No filename was sent, don't know which file to run.\n" );
	}

	return 0;
}

void CJawsDlg::OnButtonRun() {
	if( !m_bReady ) {
		// Just in case...
		return;
	}

	if( !m_bRunning ) {
		// Start up...
		if( !m_sResName.IsEmpty() ) {
			m_bLoad = TRUE;
		}

		((CButton *)GetDlgItem( IDC_BUTTON_VIDMODE ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( FALSE );
		// go ahead and re-read the master file, just in case things were compiled in since launching the app
		Fang_ConfigDefs.pszFile_MasterFilePathName = m_Settings.sMasterFile;
		ffile_SetupFileMode();
		floop_InstallGameloop( _GameInit, _GameMain, _GameTerm, &m_GameloopParm, 60.0f );
	} else {
		// Shut down...
		((CButton *)GetDlgItem( IDC_BUTTON_RUN ))->EnableWindow( FALSE );
		((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( FALSE );
		((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( FALSE );
		floop_UninstallGameloop();
	}
}

long CJawsDlg::OnGameloopRun(WPARAM wParam, LPARAM lParam) {
	m_bRunning = TRUE;
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->SetWindowText( "Stop" );
	((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( TRUE );
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( TRUE );
	return 0;
}

long CJawsDlg::OnGameloopTerm( WPARAM wParam, LPARAM lParam ) {
	m_bRunning = FALSE;
	m_bLoad = FALSE;
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->SetWindowText( "Run" );
	((CButton *)GetDlgItem(IDC_BUTTON_RUN))->EnableWindow( TRUE );
	((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( TRUE );
	((CButton *)GetDlgItem(IDC_BUTTON_VIDMODE))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDCANCEL ))->EnableWindow( TRUE );
	return 0;
}

long CJawsDlg::OnGameloopLoaded( WPARAM wParam, LPARAM lParam ) {
	((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( TRUE );
	return 0;
}

long CJawsDlg::OnGameloopLoadErr( WPARAM wParam, LPARAM lParam ) {
	CString sErrMsg;
	u32 i;

	SetForegroundWindow();

	((CButton *)GetDlgItem(IDC_BUTTON_LOAD))->EnableWindow( TRUE );

	if( !m_sResName.IsEmpty() && _pMeshRes==NULL ) {
		sErrMsg.Format( "Trouble loading object '%s'.", m_sResName );
		MessageBox( sErrMsg, "Jaws Load Error" );
	} else {
		for( i=0; i<4; i++ ) {
			if( !m_asAnimResName[i].IsEmpty() && _apResAnim[i]==NULL ) {
				sErrMsg.Format( "Trouble loading anim object '%s'.", m_asAnimResName[i] );
				MessageBox( sErrMsg, "Jaws Load Error" );
				m_asAnimResName[i].Empty();
			}
		}
	}

	UpdateControls();

	return 0;
}

long CJawsDlg::OnGameloopLoad( WPARAM wParam, LPARAM lParam ) {
	OnButtonLoad();
	return 0;
}

long CJawsDlg::OnGameloopLoadAnim1( WPARAM wParam, LPARAM lParam ) {
	OnButtonAnim1();
	return 0;
}

long CJawsDlg::OnGameloopLoadAnim2( WPARAM wParam, LPARAM lParam ) {
	OnButtonAnim2();
	return 0;
}

long CJawsDlg::OnGameloopLoadAnim3( WPARAM wParam, LPARAM lParam ) {
	OnButtonAnim3();
	return 0;
}

long CJawsDlg::OnGameloopLoadAnim4( WPARAM wParam, LPARAM lParam ) {
	OnButtonAnim4();
	return 0;
}


static BOOL _GameInit( void *pParameter ) {
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CJawsDlg *pDlg = pParm->pDlg;
	u32 i;

//	Test();

	_pCombinerConfig = NULL;
	_pCombiner = NULL;
	_bLastLeftButton = FALSE;
	_bLastRightButton = FALSE;
	_pArcBall = NULL;
	_bMainLoopExecuted = FALSE;
	
	_nMeshDrawMode = _MESH_DRAW_MODES_REGULAR;
	FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_DISABLED;
//	FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_LEAF_TRIS;

	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;

	for( i=0; i<4; i++ ) {
		_apResAnim[i] = NULL;
		_apAnimInst[i] = NULL;
	}

	if( !fvid_CreateWindow( &pParm->VidWin ) ) {
		pDlg->MessageBox( "Could not use the current video mode.\nTry selecting a different video mode.", "Jaws: fvid_CreateWindow() Failed" );
		goto _ExitInitWithError;
	}

	if( !_bFrameRate ) {
		fvid_DestroyWindow();
		pDlg->MessageBox( "Could not use the current video mode.\nTry selecting a different video mode.", "Jaws: fvid_CreateWindow() Failed." );
		return FALSE;
	}

	fdx8vid_SetCursor( LoadCursor( NULL, IDC_SIZEALL ) );
	fdx8vid_EnableCursor( pDlg->m_bDrawCursor );

	// Init combiner config...
	_pCombinerConfig = fnew CFAnimCombinerConfig;
	if( _pCombinerConfig == NULL ) {
		goto _ExitInitWithError;
	}

	if( !_pCombinerConfig->BeginCreation( "Mixer2", CFAnimMixer::TYPE_BLENDER ) ) {
		goto _ExitInitWithError;
	}
	if( !_pCombinerConfig->AddMixer( "Mixer0", CFAnimMixer::TYPE_BLENDER, "Mixer2", 0 ) ) {
		goto _ExitInitWithError;
	}
	if( !_pCombinerConfig->AddMixer( "Mixer1", CFAnimMixer::TYPE_BLENDER, "Mixer2", 1 ) ) {
		goto _ExitInitWithError;
	}
	if( !_pCombinerConfig->AddTap( "Tap0", "Mixer0", 0 ) ) {
		goto _ExitInitWithError;
	}
	if( !_pCombinerConfig->AddTap( "Tap1", "Mixer0", 1 ) ) {
		goto _ExitInitWithError;
	}
	if( !_pCombinerConfig->AddTap( "Tap2", "Mixer1", 0 ) ) {
		goto _ExitInitWithError;
	}
	if( !_pCombinerConfig->AddTap( "Tap3", "Mixer1", 1 ) ) {
		goto _ExitInitWithError;
	}
	if( !_pCombinerConfig->EndCreation() ) {
		goto _ExitInitWithError;
	}

	_pViewport = fviewport_Create();
	if( _pViewport == NULL ) {
		goto _ExitInitWithError;
	}

#if FANG_USE_PORTAL
	FVis_bDrawVisibility = TRUE;
	FVis_bShowDrawStats = TRUE;
	FVis_bDrawCellsOnly = FALSE;
#endif

	_fLastFOV = _fNewFOV;
	fviewport_InitPersp( _pViewport, _fLastFOV*0.5f, 0.1f, 5000.0f );
	_XfmCam.BuildInvRotYXZ_XlatFromPoint( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
	_fWorldCamHeading = 0.0f;
	_fWorldCamPitch = 0.0f;
	_WorldCamPos.Zero();

	if( FTEXT_ERROR == ftext_Install() ) {
		pDlg->MessageBox( "Could not install the text system.", "Jaws: Text System Problem" );
		goto _ExitInitWithError;
	}

	_pArcBall = fnew CArcBall;
	if( _pArcBall == NULL ) {
		goto _ExitInitWithError;
	}
	_pArcBall->SetRadius( 2.0f, 10.0f );

	_hBoxFilter_Potential = fboxfilter_Create_f32( 30 );
	_hBoxFilter_Governed = fboxfilter_Create_f32( 30 );

//	_ResFrame_MeshAnim = FRES_NULLHANDLE;
	_ResFrame_Anim = FRES_NULLHANDLE;
//	_ResFrame_MeshAnim = fres_GetFrame();
//	_ResFrame_Anim = fres_GetFrame();

	screenshot_Init( pDlg->m_Settings.sScreenShotPath );

	_bFirstMouseActivity = TRUE;
	fdx8vid_RegisterMouseCallback( _GameMouseCallback, pParm );

	fperf_Reset();
	_fAvgFrameSecs = 0.0f;

	// Success...
	pDlg->PostMessage( _MM_GAMELOOP_RUN );

	return TRUE;

_ExitInitWithError:
	if( _pCombinerConfig ) {
		fdelete( _pCombinerConfig );
		_pCombinerConfig = NULL;
	}

	return FALSE;
}

static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter ) {
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CJawsDlg *pDlg = pParm->pDlg;

	if( !_bFrameRate ) {
		pDlg->PostMessage( _MM_GAMELOOP_TERM );
		return;
	}

	fdx8vid_RegisterMouseCallback( NULL, NULL );

	_FreeAnimResources();

	if( _pCombinerConfig ) {
		fdelete( _pCombinerConfig );
		_pCombinerConfig = NULL;
	}

	if( _pArcBall ) {
		fdelete( _pArcBall );
		_pArcBall = NULL;
	}

	screenshot_Shutdown();

	ftext_Uninstall();

	fvid_DestroyWindow();
	pDlg->PostMessage( _MM_GAMELOOP_TERM );
}

static BOOL _GameMain( BOOL bExitRequest, void *pParameter ) 
{
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CJawsDlg *pDlg = pParm->pDlg;
	BOOL bKeyDown, bHaveFocus;
	f32 fFrameSecs;
	CFTimer Timer;
	CFVec3A V2, V1;
	CFVec3 NewOrigin_WS, NewOrigin_MS;
	CFXfm XfmNewOrigin;
	char szTemp[32];
	CFVec3A vDiff;
	CFSphere spPos;

	if ( bExitRequest || fvid_HasUserClosedWindow() ) 
	{
		return FALSE;
	}

	if ( pDlg->m_bDataBeingUpdated ) 
	{
		return TRUE;
	}

	if ( !_bFrameRate ) 
	{
		return FALSE;
	}

	if ( _nAnimPlaybackRate == _ANIM_PLAYBACK_SLOWMOTION ) {
		FLoop_fPreviousLoopSecs *= _fFrameRateMultiplier;
		FLoop_fPreviousLoopOOSecs = 1.0f/FLoop_fPreviousLoopSecs;
	}

//	if ( pDlg->m_bReloadMasterFile ) 
//	{
//		pDlg->m_bReloadMasterFile = FALSE;
//		ffile_SetupFileMode();
//	}

	Timer.Reset();

	_bMainLoopExecuted = TRUE;

	bHaveFocus = fdx8vid_HaveFocus();

	if ( !bHaveFocus ) 
	{
		// allow other windows to get some processing
		Sleep( 0 );
	}

	if ( pDlg->m_bLoad ) 
	{
		pDlg->m_bLoad = FALSE;

		if ( !pDlg->m_sResName.IsEmpty() ) 
		{
			fdx8vid_RestoreWindow();

			// Free all of the animation resources
			if ( _ResFrame_Anim != FRES_NULLHANDLE )
			{
				fres_ReleaseFrame( _ResFrame_Anim );
			}
			_ResFrame_Anim = fres_GetFrame();

			if( pDlg->m_sResType == FMESH_RESTYPE ) 
			{
				_bWorldMode = FALSE;

				ffile_SetupFileMode();
				_pMeshRes = (FMesh_t *)fresload_Load( pDlg->m_sResType, pDlg->m_sResName );
				if ( _pMeshRes == NULL ) 
				{
					pDlg->PostMessage( _MM_GAMELOOP_LOADERR );
					return TRUE;
				}

				if ( pDlg->m_bOrientSet ) 
				{
					CFXfm XfmSave = _MeshInst.m_Xfm;
					_MeshInst.Init( _pMeshRes );
					_MeshInst.m_Xfm = XfmSave;
				} 
				else 
				{
					_MeshInst.Init( _pMeshRes );
				//	_MeshInst.m_Xfm.BuildTranslation( 0.0f, 0.0f, 100.0f );

					// Mike's fix
					_MeshInst.m_Xfm.BuildTranslation( -_pMeshRes->BoundSphere_MS.m_Pos.x,
													  -_pMeshRes->BoundSphere_MS.m_Pos.y,
													  -_pMeshRes->BoundSphere_MS.m_Pos.z + (_pMeshRes->BoundSphere_MS.m_fRadius * 2.0f) );										
				}
				NewOrigin_WS.Set( 0.0f, 0.0f, (_pMeshRes->BoundSphere_MS.m_fRadius * 2.0f) );
				V1.Set( NewOrigin_WS );
				_MeshInst.m_Xfm.m_MtxR.MulPoint( V2, V1 );
				NewOrigin_MS = V2.v3;
				XfmNewOrigin.BuildTranslation( NewOrigin_MS );
				_XfmModelOrigin.ReceiveInverseOf( XfmNewOrigin );

				_XfmCam.BuildInvRotYXZ_XlatFromPoint( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
				_fWorldCamHeading = 0.0f;
				_fWorldCamPitch = 0.0f;
				_WorldCamPos.Zero();

				_pArcBall->m_Mtx = _MeshInst.m_Xfm.m_MtxF;
				_pArcBall->m_Mtx.m_vPos.v3 = NewOrigin_WS;
				_pArcBall->SetRadius( 2.0f, _MeshInst.m_pMesh->BoundSphere_MS.m_fRadius );

				_ResFrame_Anim = fres_GetFrame();
				if( !_LoadAnimFiles( pDlg ) ) 
				{
					pDlg->PostMessage( _MM_GAMELOOP_LOADERR );
					return TRUE;
				}
			} 
			else 
			{
				_bWorldMode = TRUE;

				_pWorldRes = (FVisData_t *)fresload_Load( pDlg->m_sResType, pDlg->m_sResName );

				if( _pWorldRes == NULL ) 
				{
					pDlg->PostMessage( _MM_GAMELOOP_LOADERR );
					return TRUE;
				}

				_ResFrame_Anim = fres_GetFrame();
			}

			pDlg->PostMessage( _MM_GAMELOOP_LOADED );
		}
	}

	if( _pCombiner ) 
	{
		if( pDlg->m_abLoadAnim[0] || pDlg->m_abLoadAnim[1] || pDlg->m_abLoadAnim[2] || pDlg->m_abLoadAnim[3] ) 
		{
			pDlg->m_abLoadAnim[0] = pDlg->m_abLoadAnim[1] = pDlg->m_abLoadAnim[2] = pDlg->m_abLoadAnim[3] = FALSE;

			fdx8vid_RestoreWindow();

			if ( !_LoadAnimFiles( pDlg ) ) 
			{
				pDlg->PostMessage( _MM_GAMELOOP_LOADERR );
				return TRUE;
			}
		}
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_ESCAPE ) < 0);
	if( bKeyDown ) {
		return FALSE;
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_NUMPAD9 ) < 0);//VK_ADD
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
		pDlg->PostMessage( _MM_GAMELOOP_LOAD );
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_CONTROL ) < 0) && (GetAsyncKeyState( '1' ) < 0);
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
		pDlg->PostMessage( _MM_GAMELOOP_LOADANIM1 );
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_CONTROL ) < 0) && (GetAsyncKeyState( '2' ) < 0);
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
		pDlg->PostMessage( _MM_GAMELOOP_LOADANIM2 );
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_CONTROL ) < 0) && (GetAsyncKeyState( '3' ) < 0);
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
		pDlg->PostMessage( _MM_GAMELOOP_LOADANIM3 );
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_CONTROL ) < 0) && (GetAsyncKeyState( '4' ) < 0);
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
		pDlg->PostMessage( _MM_GAMELOOP_LOADANIM4 );
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_BACK ) < 0);
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
	}

	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_HOME ) < 0);
	if( bKeyDown ) {
		if( _bWorldMode ) {
			_XfmCam.BuildInvRotYXZ_XlatFromPoint( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
			_fWorldCamHeading = 0.0f;
			_fWorldCamPitch = 0.0f;
			_WorldCamPos.Zero();
		} else {
		//	_pArcBall->m_Mtx.Identity();
		//	_pArcBall->m_Mtx.m_vPos.Set( 0.0f, 0.0f, 100.0f );
			_MeshInst.m_Xfm.BuildTranslation( -_pMeshRes->BoundSphere_MS.m_Pos.x,
											  -_pMeshRes->BoundSphere_MS.m_Pos.y,
											  -_pMeshRes->BoundSphere_MS.m_Pos.z + (_pMeshRes->BoundSphere_MS.m_fRadius * 2.0f) );
			NewOrigin_WS.Set( 0.0f, 0.0f, (_pMeshRes->BoundSphere_MS.m_fRadius * 2.0f) );
			V1.Set( NewOrigin_WS );
			_MeshInst.m_Xfm.m_MtxR.MulPoint( V2, V1 );
			NewOrigin_MS = V2.v3;
			XfmNewOrigin.BuildTranslation( NewOrigin_MS );
			_XfmModelOrigin.ReceiveInverseOf( XfmNewOrigin );

			_XfmCam.BuildInvRotYXZ_XlatFromPoint( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
			_fWorldCamHeading = 0.0f;
			_fWorldCamPitch = 0.0f;
			_WorldCamPos.Zero();

			_pArcBall->m_Mtx = _MeshInst.m_Xfm.m_MtxF;
			_pArcBall->m_Mtx.m_vPos.v3 = NewOrigin_WS;
			_pArcBall->SetRadius( 2.0f, _MeshInst.m_pMesh->BoundSphere_MS.m_fRadius );
		}
	}

	// go ahead and set the framerate governing level, it's cheap
	switch( _nAnimPlaybackRate ) {
	case _ANIM_PLAYBACK_30FPS:
		if( floop_GetTargetFramesPerSec() != 30 || !floop_IsFixedLoopTimeModeEnabled() ) {
			floop_SetTargetFramesPerSec( 30 );
			floop_EnableFixedLoopTimeMode( TRUE );
		}
		break;
	case _ANIM_PLAYBACK_60FPS:
		if( floop_GetTargetFramesPerSec() != 60 || !floop_IsFixedLoopTimeModeEnabled() ) {
			floop_SetTargetFramesPerSec( 60 );
			floop_EnableFixedLoopTimeMode( TRUE );
		}
		break;
	default:
	case _ANIM_PLAYBACK_SLOWMOTION:
	case _ANIM_PLAYBACK_REALTIME:
		if( floop_IsFixedLoopTimeModeEnabled() ) {
			floop_SetTargetFramesPerSec( 60 );
			floop_EnableFixedLoopTimeMode( FALSE );
		}
		break;
	}
	// enable/disable the drawing of the cursor
	fdx8vid_EnableCursor( pDlg->m_bDrawCursor );

	fvid_Begin();

	fmesh_ResetLightList();

	fmesh_Ambient_Set( &pDlg->m_AmbientColorRGB, pDlg->m_bAmbientOn ? 1.0f : 0.0f );

//	if ( pDlg->m_bDirOn ) {
		_LightDir.InitDirLight( &pDlg->m_DirLightVector );
		_LightDir.SetColor( &pDlg->m_DirColorRGB );
//		fmesh_AddLight( &_LightDir );
//	}

	if( _fLastFOV != _fNewFOV ) {
		fviewport_InitPersp( _pViewport, _fNewFOV*0.5f, 0.1f, 5000.0f );
		_fLastFOV = _fNewFOV;
	}

	fviewport_SetActive( _pViewport );

	if( pDlg->m_bBackgroundClear ) {
		fviewport_Clear(
			FVIEWPORT_CLEARFLAG_ALL,
			pDlg->m_BackColorRGB.fRed,
			pDlg->m_BackColorRGB.fGreen,
			pDlg->m_BackColorRGB.fBlue,
			1.0f,
			0
		);
	}

	if( _bWorldMode ) {
		// World mode:

		if( _pWorldRes ) {
			FVisVolume_t *pVolume;
			f32 fXlatSpeed = 0.75f * (FLoop_fPreviousLoopSecs * 60.0f);
			FMATH_CLAMPMIN( fXlatSpeed, 0.5f );

			bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_ADD ) < 0);
			if( bKeyDown ) {
				// super fast travel mode
				fXlatSpeed *= 4.0f;
			} else {
				bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_SUBTRACT ) < 0);
				if( bKeyDown ) {
					// super slow travel mode
					fXlatSpeed *= 0.25f;
				}
			} 

			_XfmCam.BuildInvRotYXZ_XlatFromPoint( _fWorldCamHeading, _fWorldCamPitch, 0.0f, _WorldCamPos.x, _WorldCamPos.y, _WorldCamPos.z );

			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'W' ) < 0);
			if( bKeyDown ) {
				// Forward...
				_WorldCamPos += _XfmCam.m_MtxR.m_vFront.v3 * fXlatSpeed;
			}

			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'S' ) < 0);
			if( bKeyDown ) {
				// Backwards...
				_WorldCamPos -= _XfmCam.m_MtxR.m_vFront.v3 * fXlatSpeed;
			}

			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'A' ) < 0);
			if( bKeyDown ) {
				// Strafe Left...
				_WorldCamPos -= _XfmCam.m_MtxR.m_vRight.v3 * fXlatSpeed;
			}

			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'D' ) < 0);
			if( bKeyDown ) {
				// Strafe Right...
				_WorldCamPos += _XfmCam.m_MtxR.m_vRight.v3 * fXlatSpeed;
			}

			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'R' ) < 0);
			if( bKeyDown ) {
				// Vertical Up...
				_WorldCamPos.y += fXlatSpeed;
			}

			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'F' ) < 0);
			if( bKeyDown ) {
				// Vertical Down...
				_WorldCamPos.y -= fXlatSpeed;
			}
			
			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'C' ) & 0x1);
			if( bKeyDown ) {
				// toggle between:
				// geo + cells
				// cells only
				// geo only
				if( FVis_bDrawVisibility && !FVis_bDrawCellsOnly ) {
					// switch to drawing the cells only
					FVis_bDrawCellsOnly = TRUE;
				} else if( FVis_bDrawCellsOnly ) {
					// switch to drawing geo only
					FVis_bDrawCellsOnly = FALSE;
					FVis_bDrawVisibility = FALSE;
				} else {
					// switch to drawing the geo + cells
					FVis_bDrawVisibility = TRUE;
				}
			}

			fworld_GetVolumeContainingPoint( &_XfmCam.m_MtxR.m_vPos, &pVolume );
			_XfmCam.InitStackWithView();
			fvis_FrameBegin();
			fvis_Draw( NULL, FALSE );
			fvis_FrameEnd();
		}

		if( pDlg->m_bFPSOn ) {
			ftext_DebugPrintf( 0.99f, 0.01f, "~w1~arCam XYZ: %7.1f %7.1f %7.1f", _XfmCam.m_MtxR.m_vPos.x, _XfmCam.m_MtxR.m_vPos.y, _XfmCam.m_MtxR.m_vPos.z );
		}

	} else {
		// Mesh mode:

		if( _pMeshRes ) {
			if( _pCombiner && pDlg->m_bEnableAnim ) {
				if( !pDlg->m_bPauseAnim ) {
					if( bHaveFocus ) {
						if( GetAsyncKeyState( VK_NUMPAD1 ) < 0 ) {
							_DisplayMixerAdjust( 0 );
						} else if( GetAsyncKeyState( VK_NUMPAD2 ) < 0 ) {
							_DisplayMixerAdjust( 1 );
						} else if( GetAsyncKeyState( VK_NUMPAD3 ) < 0 ) {
							_DisplayMixerAdjust( 2 );
						}

						if( GetAsyncKeyState( '1' ) < 0 ) {
							_TriggerOneShotAnim( 0, pDlg );
						}
						if( GetAsyncKeyState( '2' ) < 0 ) {
							_TriggerOneShotAnim( 1, pDlg );
						}
						if( GetAsyncKeyState( '3' ) < 0 ) {
							_TriggerOneShotAnim( 2, pDlg );
						}
						if( GetAsyncKeyState( '4' ) < 0 ) {
							_TriggerOneShotAnim( 3, pDlg );
						}
					}

					_UpdateAnimations( pDlg, FLoop_fPreviousLoopSecs );
				}

				_pCombiner->ComputeMtxPalette();
			}

			if( _pCombiner==NULL || !pDlg->m_bEnableAnim ) {
				_MeshInst.m_nFlags |= FMESHINST_FLAG_NOBONES;
			} else {
				_MeshInst.m_nFlags &= ~FMESHINST_FLAG_NOBONES;
			}

			_XfmCam.InitStackWithView();

			CFXfm XfmArcBall;
			XfmArcBall.BuildFromMtx( _pArcBall->m_Mtx, FALSE );
			_MeshInst.m_Xfm = XfmArcBall * _XfmModelOrigin;

			frenderer_Push( FRENDERER_MESH );
			_MeshInst.ResetLightList();
			if ( pDlg->m_bDirOn ) {
				_MeshInst.ConsiderLightForRender( &_LightDir );
			}
			_MeshInst.Draw( FVIEWPORT_PLANESMASK_ALL );
			frenderer_Pop();

			if( pDlg->m_bFPSOn ) {
				ftext_DebugPrintf( 0.01f, 0.01f, "~w1Obj XYZ: %5.1f %5.1f %5.1f\nFOV: %d Degrees", _MeshInst.m_Xfm.m_MtxF.m_vPos.x, _MeshInst.m_Xfm.m_MtxF.m_vPos.y, _MeshInst.m_Xfm.m_MtxF.m_vPos.z, (u32)FMATH_RAD2DEG( _fLastFOV ) );
				// draw the unit percent of each animation
				f32 afPercent[4];
				u32 i;
				for( i=0; i < 4; i++ ) {
					if( _apAnimInst[i] ) {
						afPercent[i] = _apAnimInst[i]->GetUnitTime() * 100.0f;
					} else {
						afPercent[i] = 0.0f;
					}
				}
				ftext_DebugPrintf( 0.01f, 0.05f, "~w1Anim1 = %.2f\nAnim2 = %.2f\nAnim3 = %.2f\nAnim4 = %.2f\n", afPercent[0], afPercent[1], afPercent[2], afPercent[3] );
			}

			if( pDlg->m_bPerfOn ) 
			{
				if ( _MeshInst.m_pMesh->nLODCount > 1 )
				{
					ftext_DebugPrintf( 0.01f, 0.025f, "~w1Current LOD :  %d", _MeshInst.m_nCurrentLOD );
					CString csSwitches = "~w1LOD Switches: ";
					for ( u32 nLOD = 0; nLOD < _MeshInst.m_pMesh->nLODCount; nLOD++ )
					{
						if ( nLOD != 0 )
						{
							sprintf( szTemp, ", %5.1f", _MeshInst.m_pMesh->afLODDistance[nLOD] );
						}
						else
						{
							sprintf( szTemp, "%5.1f", _MeshInst.m_pMesh->afLODDistance[nLOD] );
						}
						csSwitches += szTemp;
					}
/*
					ftext_DebugPrintf( 0.01f, 0.04f, csSwitches.GetBuffer(0), _MeshInst.m_nCurrentLOD );
					spPos = _MeshInst.m_pMesh->BoundSphere_MS;
					spPos.m_Pos = _MeshInst.m_Xfm.m_MtxF.m44.MultPoint( spPos.m_Pos );
					vDiff.Sub( _XfmCam.m_MtxR.m_vPos, spPos.m_Pos );
					if ( vDiff.MagSq() < (_MeshInst.m_pMesh->BoundSphere_MS.m_fRadius * _MeshInst.m_pMesh->BoundSphere_MS.m_fRadius) )
					{
						ftext_DebugPrintf( 0.01f, 0.055f, "~w1Obj Dist (- Radius):  0");
					}
					else
					{
						ftext_DebugPrintf( 0.01f, 0.055f, "~w1Obj Dist (- Radius):  %5.2f", vDiff.Mag() - _MeshInst.m_pMesh->BoundSphere_MS.m_fRadius );
					}
*/
				}
			}

			if( pDlg->m_bFPSOn ) {
			}

			bKeyDown = bHaveFocus && (GetAsyncKeyState( 'C' ) & 0x1);
			if( bKeyDown ) {
				// toggle between:
				// geo only
				// root dops & leaf tris
				// leaf dops & leaf tris
				// leaf tris
				_nMeshDrawMode++;
				if( _nMeshDrawMode >= _MESH_DRAW_MODES_COUNT ) {
					_nMeshDrawMode = 0;
				}
				switch( _nMeshDrawMode ) {
				case _MESH_DRAW_MODES_REGULAR:
					FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_DISABLED;
					break;
				case _MESH_DRAW_MODES_ROOT_DOPS:
					FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_ROOT_DOPS;
					break;
				case _MESH_DRAW_MODES_LEAF_DOPS:
					FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_LEAF_DOPS | FMESH_DRAW_COLL_GEO_LEAF_TRIS;
					break;
				case _MESH_DRAW_MODES_LEAF_TRIS:
					FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_LEAF_TRIS;
					break;
				case _MESH_DRAW_MODES_ROOT_DOPS_PLUS_LEAF_TRIS:
					FMesh_nDrawCollisionGeoFlags = FMESH_DRAW_COLL_GEO_ROOT_DOPS | FMESH_DRAW_COLL_GEO_LEAF_TRIS;
					break;
				}
			}

		}
	}

	fvid_End();
	if( !fvid_Swap() ) {
		return FALSE;
	}

	screenshot_Work();

	fFrameSecs = Timer.SampleSeconds( TRUE );
	fboxfilter_Add_f32( _hBoxFilter_Potential, fFrameSecs );
	fboxfilter_Get_f32( _hBoxFilter_Potential, NULL, &_fAvgFrameSecs, NULL, NULL );
	fboxfilter_Add_f32( _hBoxFilter_Governed, FLoop_fPreviousLoopSecs );
	fboxfilter_Get_f32( _hBoxFilter_Governed, NULL, &fFrameSecs, NULL, NULL );

	if( pDlg->m_bFPSOn ) {
		// Print last frame's info...
		ftext_DebugPrintf( 0.01f, 0.695f,	"~w1"						\
											"   Actual FPS: %5.1f\n"	\
											"Potential FPS: %5.1f",
											1.0f / fFrameSecs,
											1.0f / _fAvgFrameSecs );
	}

	if( pDlg->m_bPerfOn ) {
		_DisplayPerf();
	}

	return TRUE;
}

void CJawsDlg::OnButtonSettings() {
	Settings_t Settings;
	CSettings SettingsDlg;
	CString sOldMasterFile;

	if( SettingsDlg.DoModal() == IDOK ) {
		// grab the old master file
		sOldMasterFile = m_Settings.sMasterFile;
		// grab the new settings
		SettingsDlg.GetCurrentSettings( &Settings );

		m_bDataBeingUpdated = TRUE;
		m_Settings = Settings;
		if( sOldMasterFile != m_Settings.sMasterFile ) {
			Fang_ConfigDefs.pszFile_MasterFilePathName = m_Settings.sMasterFile;
			m_bReloadMasterFile = TRUE;
			// since the master file changed, reset the selected assets
			m_sResName.Empty();
			m_asAnimResName[0].Empty();
			m_asAnimResName[1].Empty();
			m_asAnimResName[2].Empty();
			m_asAnimResName[3].Empty();
			UpdateControls();
		}
		// since we changed
		m_bDataBeingUpdated = FALSE;
	}
}

void CJawsDlg::OnButtonLoad() {
	static u32 __nInFcnCount = 0;

	__nInFcnCount++;

	if( __nInFcnCount != 1 ) {
		__nInFcnCount--;
		return;
	}

	// use the pick asset dialog to pick assets from the master file
	CPickAsset PickAssetDlg;

	PickAssetDlg.m_nOptions = CPickAsset::PICK_ASSET_WORLDS | CPickAsset::PICK_ASSET_MESHES;
	PickAssetDlg.m_sInputMasterFile = m_Settings.sMasterFile;
	PickAssetDlg.m_bReadOnly = TRUE;
	PickAssetDlg.m_sInitialSelection = m_sResName;
	if( m_sResType == FWORLD_RESTYPE ) {
		PickAssetDlg.m_sInitialSelection += ".wld";
	} else {
		PickAssetDlg.m_sInitialSelection += ".ape";
	}
	if( PickAssetDlg.DoModal() == IDOK ) {
		m_sResName = PickAssetDlg.GetFileTitle();
		
		m_sDefaultLoadExt = PickAssetDlg.GetFileExt();
		m_sDefaultLoadExt.MakeLower();
		if( m_sDefaultLoadExt == "wld" ) {
			m_sResType = FWORLD_RESTYPE;
		} else {
			m_sResType = FMESH_RESTYPE;
		}

		StoreSettingsIntoRegistry( FALSE );
		UpdateControls();

		m_bOrientSet = FALSE;
		m_bLoad = TRUE;
//		m_bReloadMasterFile = TRUE;

		if( !m_bRunning && m_Settings.bAutoRunOnLoad ) {
			OnButtonRun();
		}
	}

	__nInFcnCount--;
}

void CJawsDlg::GetSettingsFromRegistry( BOOL bDiskKey ) {
	long nRetCode;
	HKEY hKey;
	DWORD nBufLength, nValueType, nValue;
	char szResName[FRES_NAMELEN+1];
	char szExt[MAX_PATH+1];
	f32 fValue;
	BOOL bValidDirVector = TRUE;
	u32 i;

	// Set defaults:
	m_sResName.Empty();
	m_sResType = FMESH_RESTYPE;
	m_sDefaultLoadExt = "bin";
	m_BackColorRGB.Black();
	m_AmbientColorRGB.Set( 0.4f );
	m_DirColorRGB.White();
	m_bBackgroundClear = TRUE;
	m_bAmbientOn = TRUE;
	m_bDirOn = TRUE;
	m_bPerfOn = FALSE;
	m_bFPSOn = TRUE;
	m_bDrawCursor = TRUE;
	m_bShowBound = FALSE;
	m_bShowBones = FALSE;

	for( i=0; i<4; i++ ) {
		m_asAnimResName[i].Empty();
		m_abLoop[i] = TRUE;
	}
	m_bEnableAnim = FALSE;
	m_bPauseAnim = FALSE;
	m_abLink[0] = FALSE;
	m_abLink[1] = FALSE;

	if( bDiskKey ) {
		nRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _REGKEY_CONFIGDISK, 0, KEY_READ, &hKey );
	} else {
		nRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _REGKEY_CONFIG, 0, KEY_READ, &hKey );
	}

	if( nRetCode != ERROR_SUCCESS ) {
		goto _ExitGetSettings;
	}

	nBufLength = FRES_NAMELEN + 1;
	nRetCode = RegQueryValueEx( hKey, "ResName", 0, &nValueType, (BYTE *)szResName, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		m_sResName = szResName;
	}

	nBufLength = FRES_TYPELEN + 1;
	nRetCode = RegQueryValueEx( hKey, "ResType", 0, &nValueType, (BYTE *)szResName, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		m_sResType = szResName;
	}

	nBufLength = MAX_PATH + 1;
	nRetCode = RegQueryValueEx( hKey, "DefLoadExt", 0, &nValueType, (BYTE *)szExt, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		m_sDefaultLoadExt = szExt;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "BackColor", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		_COLORREF_TO_COLORRGB( &m_BackColorRGB, nValue );
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "AmbientColor", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		_COLORREF_TO_COLORRGB( &m_AmbientColorRGB, nValue );
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "DirectionalColor", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		_COLORREF_TO_COLORRGB( &m_DirColorRGB, nValue );
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "BackClear", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bBackgroundClear = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "AmbientOn", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bAmbientOn = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "DirOn", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bDirOn = nValue;
	}

	nBufLength = sizeof(f32);
	nRetCode = RegQueryValueEx( hKey, "DirX", 0, &nValueType, (BYTE *)&fValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_BINARY ) {
		m_DirLightVector.x = fValue;
	} else {
		bValidDirVector = FALSE;
	}

	nBufLength = sizeof(f32);
	nRetCode = RegQueryValueEx( hKey, "DirY", 0, &nValueType, (BYTE *)&fValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_BINARY ) {
		m_DirLightVector.y = fValue;
	} else {
		bValidDirVector = FALSE;
	}

	nBufLength = sizeof(f32);
	nRetCode = RegQueryValueEx( hKey, "DirZ", 0, &nValueType, (BYTE *)&fValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_BINARY ) {
		m_DirLightVector.z = fValue;
	} else {
		bValidDirVector = FALSE;
	}

	if( !bValidDirVector ) {
		m_DirLightVector.Set( -1.0f, -1.0f, 1.0f );
	}

	nBufLength = FRES_NAMELEN + 1;
	nRetCode = RegQueryValueEx( hKey, "AnimResName0", 0, &nValueType, (BYTE *)szResName, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		m_asAnimResName[0] = szResName;
	}

	nBufLength = FRES_NAMELEN + 1;
	nRetCode = RegQueryValueEx( hKey, "AnimResName1", 0, &nValueType, (BYTE *)szResName, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		m_asAnimResName[1] = szResName;
	}

	nBufLength = FRES_NAMELEN + 1;
	nRetCode = RegQueryValueEx( hKey, "AnimResName2", 0, &nValueType, (BYTE *)szResName, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		m_asAnimResName[2] = szResName;
	}

	nBufLength = FRES_NAMELEN + 1;
	nRetCode = RegQueryValueEx( hKey, "AnimResName3", 0, &nValueType, (BYTE *)szResName, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_SZ ) {
		m_asAnimResName[3] = szResName;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LoopAnim0", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLoop[0] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LoopAnim1", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLoop[1] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LoopAnim2", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLoop[2] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LinkAnim3", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLink[3] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LinkAnim0", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLink[0] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LinkAnim1", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLink[1] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LinkAnim2", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLink[2] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "LinkAnim3", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_abLink[3] = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "EnableAnim", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bEnableAnim = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "PauseAnim", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bPauseAnim = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "PerfOn", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bPerfOn = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "FPSOn", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bFPSOn = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "DrawCursor", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bDrawCursor = nValue;
	}	

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "ShowBound", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bShowBound = nValue;
	}

	nBufLength = sizeof(DWORD);
	nRetCode = RegQueryValueEx( hKey, "ShowBones", 0, &nValueType, (BYTE *)&nValue, &nBufLength );
	if( nRetCode==ERROR_SUCCESS && nValueType==REG_DWORD ) {
		m_bShowBones = nValue;
	}

	RegCloseKey( hKey );

_ExitGetSettings:
	UpdateControls();
}

BOOL CJawsDlg::StoreSettingsIntoRegistry( BOOL bDiskKey ) {
	long nRetCode;
	DWORD nDisposition, nValue;
	HKEY hKey;
	BOOL bSuccess = TRUE;
	f32 fValue;

	if( bDiskKey ) {
		nRetCode = RegCreateKeyEx( HKEY_LOCAL_MACHINE, _REGKEY_CONFIGDISK, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &nDisposition );
	} else {
		nRetCode = RegCreateKeyEx( HKEY_LOCAL_MACHINE, _REGKEY_CONFIG, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &nDisposition );
	}

	if( nRetCode != ERROR_SUCCESS ) {
		return FALSE;
	}

	nRetCode = RegSetValueEx( hKey, "ResName", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)m_sResName, m_sResName.GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nRetCode = RegSetValueEx( hKey, "ResType", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)m_sResType, m_sResType.GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nRetCode = RegSetValueEx( hKey, "DefLoadExt", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)m_sDefaultLoadExt, m_sDefaultLoadExt.GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = _COLORRGB_TO_COLORREF( &m_BackColorRGB );
	nRetCode = RegSetValueEx( hKey, "BackColor", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = _COLORRGB_TO_COLORREF( &m_AmbientColorRGB );
	nRetCode = RegSetValueEx( hKey, "AmbientColor", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = _COLORRGB_TO_COLORREF( &m_DirColorRGB );
	nRetCode = RegSetValueEx( hKey, "DirectionalColor", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bBackgroundClear;
	nRetCode = RegSetValueEx( hKey, "BackClear", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bAmbientOn;
	nRetCode = RegSetValueEx( hKey, "AmbientOn", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bDirOn;
	nRetCode = RegSetValueEx( hKey, "DirOn", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	fValue = m_DirLightVector.x;
	nRetCode = RegSetValueEx( hKey, "DirX", 0, REG_BINARY, (CONST BYTE *)&fValue, sizeof(f32) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	fValue = m_DirLightVector.y;
	nRetCode = RegSetValueEx( hKey, "DirY", 0, REG_BINARY, (CONST BYTE *)&fValue, sizeof(f32) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	fValue = m_DirLightVector.z;
	nRetCode = RegSetValueEx( hKey, "DirZ", 0, REG_BINARY, (CONST BYTE *)&fValue, sizeof(f32) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nRetCode = RegSetValueEx( hKey, "AnimResName0", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)m_asAnimResName[0], m_asAnimResName[0].GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nRetCode = RegSetValueEx( hKey, "AnimResName1", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)m_asAnimResName[1], m_asAnimResName[1].GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nRetCode = RegSetValueEx( hKey, "AnimResName2", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)m_asAnimResName[2], m_asAnimResName[2].GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nRetCode = RegSetValueEx( hKey, "AnimResName3", 0, REG_SZ, (CONST BYTE *)(LPCTSTR)m_asAnimResName[3], m_asAnimResName[3].GetLength() + 1 );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLoop[0];
	nRetCode = RegSetValueEx( hKey, "LoopAnim0", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLoop[1];
	nRetCode = RegSetValueEx( hKey, "LoopAnim1", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLoop[2];
	nRetCode = RegSetValueEx( hKey, "LoopAnim2", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLoop[3];
	nRetCode = RegSetValueEx( hKey, "LoopAnim3", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLink[0];
	nRetCode = RegSetValueEx( hKey, "LinkAnim0", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLink[1];
	nRetCode = RegSetValueEx( hKey, "LinkAnim1", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLink[2];
	nRetCode = RegSetValueEx( hKey, "LinkAnim2", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_abLink[3];
	nRetCode = RegSetValueEx( hKey, "LinkAnim3", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bEnableAnim;
	nRetCode = RegSetValueEx( hKey, "EnableAnim", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bPauseAnim;
	nRetCode = RegSetValueEx( hKey, "PauseAnim", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bPerfOn;
	nRetCode = RegSetValueEx( hKey, "PerfOn", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bFPSOn;
	nRetCode = RegSetValueEx( hKey, "FPSOn", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}	
	
	nValue = m_bDrawCursor;
	nRetCode = RegSetValueEx( hKey, "DrawCursor", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}	

	nValue = m_bShowBound;
	nRetCode = RegSetValueEx( hKey, "ShowBound", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	nValue = m_bShowBones;
	nRetCode = RegSetValueEx( hKey, "ShowBones", 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD) );
	if( nRetCode != ERROR_SUCCESS ) {
		bSuccess = FALSE;
	}

	RegCloseKey( hKey );

	return bSuccess;
}

void CJawsDlg::UpdateControls() {
	CString sTypeName;

	sTypeName.Format( "%s (%s)", m_sResName, m_sResType );
	((CStatic *)GetDlgItem(IDC_BUTTON_LOAD))->SetWindowText( sTypeName );

	((CStatic *)GetDlgItem(IDC_BUTTON_ANIM1))->SetWindowText( m_asAnimResName[0] );
	((CStatic *)GetDlgItem(IDC_BUTTON_ANIM2))->SetWindowText( m_asAnimResName[1] );
	((CStatic *)GetDlgItem(IDC_BUTTON_ANIM3))->SetWindowText( m_asAnimResName[2] );
	((CStatic *)GetDlgItem(IDC_BUTTON_ANIM4))->SetWindowText( m_asAnimResName[3] );

	((CButton *)GetDlgItem(IDC_CHECK_LOOP1))->SetCheck( m_abLoop[0] );
	((CButton *)GetDlgItem(IDC_CHECK_LOOP2))->SetCheck( m_abLoop[1] );
	((CButton *)GetDlgItem(IDC_CHECK_LOOP3))->SetCheck( m_abLoop[2] );
	((CButton *)GetDlgItem(IDC_CHECK_LOOP4))->SetCheck( m_abLoop[3] );

	((CButton *)GetDlgItem(IDC_CHECK_LINK1))->SetCheck( m_abLink[0] );
	((CButton *)GetDlgItem(IDC_CHECK_LINK2))->SetCheck( m_abLink[1] );

	((CButton *)GetDlgItem(IDC_CHECK_ENABLE_ANIM))->SetCheck( m_bEnableAnim );
	((CButton *)GetDlgItem(IDC_CHECK_PAUSE_ANIM))->SetCheck( m_bPauseAnim );

	((CButton *)GetDlgItem(IDC_CHECK_AMBIENT))->SetCheck( m_bAmbientOn );
	((CButton *)GetDlgItem(IDC_CHECK_DIRECTIONAL))->SetCheck( m_bDirOn );
	((CButton *)GetDlgItem(IDC_CHECK_BACKGROUND))->SetCheck( m_bBackgroundClear );

	((CButton *)GetDlgItem(IDC_CHECK_PERF))->SetCheck( m_bPerfOn );
	((CButton *)GetDlgItem(IDC_HIDE_CURSOR))->SetCheck( !m_bDrawCursor );
	((CButton *)GetDlgItem(IDC_CHECK_FPS))->SetCheck( m_bFPSOn );	
	((CButton *)GetDlgItem(IDC_CHECK_SHOW_BOUNDS))->SetCheck( m_bShowBound );
	((CButton *)GetDlgItem(IDC_CHECK_SHOW_BONES))->SetCheck( m_bShowBones );
}

static void _GameMouseCallback( u32 nX, u32 nY, s32 nDeltaWheel, BOOL bLeftButton, BOOL bRightButton, void *pParameter ) {
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CJawsDlg *pDlg = pParm->pDlg;
	BOOL bShiftKeyDown, bHaveFocus, bMixerMode;
	s32 nDeltaX, nDeltaY;
	f32 fPixelsAcross, fPixelsDown;
	static u32 __nCursorOffX, __nCursorOffY;

	if( !_bMainLoopExecuted ) {
		return;
	}

	_bMainLoopExecuted = FALSE;

	if( _bFirstMouseActivity ) {
		_bFirstMouseActivity = FALSE;
		nDeltaX = 0;
		nDeltaY = 0;
	} else {
		_bFirstMouseActivity = TRUE;
		nDeltaX = (s32)nX - (s32)_nLastMouseX;
		nDeltaY = (s32)nY - (s32)_nLastMouseY;
	}

	bHaveFocus = fdx8vid_HaveFocus();

	if( bHaveFocus && (GetAsyncKeyState( 'L' ) < 0) ) {
		fPixelsAcross = (f32)FVid_Mode.nPixelsAcross;
		fPixelsDown = (f32)FVid_Mode.nPixelsDown;
		pDlg->m_DirLightVector.x = (fPixelsAcross*0.5f - (f32)nX) / (fPixelsAcross*0.5f);
		pDlg->m_DirLightVector.y = ((f32)nY - fPixelsDown*0.5f) / (fPixelsDown*0.5f);
		pDlg->m_DirLightVector.z = 1.0f;
	}

	bShiftKeyDown = bHaveFocus && (GetAsyncKeyState( VK_SHIFT ) < 0);

	if( _bWorldMode ) {
		if( nDeltaWheel ) {
			_WorldCamPos.y += ((f32)nDeltaWheel * 0.005f);
		}

		if( bLeftButton ) {
			if( !_bLastLeftButton ) {
				__nCursorOffX = nX;
				__nCursorOffY = nY;
			}

			_fWorldCamHeading += ((f32)nDeltaX * 0.01f);
			_fWorldCamPitch += ((f32)nDeltaY * 0.01f);
			FMATH_CLAMP( _fWorldCamPitch, -FMATH_HALF_PI, FMATH_HALF_PI );
		} else {
			if( _bLastLeftButton ) {

			}
		}
	} else {
		bMixerMode = FALSE;

		if( _pCombiner && bHaveFocus ) {
			if( GetAsyncKeyState( VK_NUMPAD1 ) < 0 ) {
				_AdjustMixer( 0, nDeltaWheel );
				bMixerMode = TRUE;
			} else if( GetAsyncKeyState( VK_NUMPAD2 ) < 0 ) {
				_AdjustMixer( 1, nDeltaWheel );
				bMixerMode = TRUE;
			} else if( GetAsyncKeyState( VK_NUMPAD3 ) < 0 ) {
				_AdjustMixer( 2, nDeltaWheel );
				bMixerMode = TRUE;
			}
		}

		if( bHaveFocus && (GetAsyncKeyState( 'O' ) < 0) ) {
			// Origin selection mode...
			CFCollInfo CollInfo;
			const CFMtx44A *pProjMtx;
			CFVec3 PickRayDir_VS, PickRayDir_WS, ImpactDistVec, NewOrigin_WS, NewOrigin_MS;
			CFXfm XfmNewOrigin;
			f32 fImpactDist, fClosestImpactDist;
			u32 i, nClosestImpactIndex;

			pProjMtx = fviewport_GetPlatformSpecificProjectionMatrix( fviewport_GetActive() );
			if( pProjMtx ) {
				PickRayDir_VS.x = ( ( ( 2.0f * (f32)nX ) / (f32)FVid_Mode.nPixelsAcross ) - 1.0f ) / pProjMtx->aa[1][1];
				PickRayDir_VS.y = -( ( ( 2.0f * (f32)nY ) / (f32)FVid_Mode.nPixelsDown ) - 1.0f ) / pProjMtx->aa[2][2];
				PickRayDir_VS.z = 1.0f;

				PickRayDir_WS.x = PickRayDir_VS.x * _XfmCam.m_MtxR.aa[0][0]
								+ PickRayDir_VS.y * _XfmCam.m_MtxR.aa[1][0]
								+ PickRayDir_VS.z * _XfmCam.m_MtxR.aa[2][0];
				PickRayDir_WS.y = PickRayDir_VS.x * _XfmCam.m_MtxR.aa[0][1]
								+ PickRayDir_VS.y * _XfmCam.m_MtxR.aa[1][1]
								+ PickRayDir_VS.z * _XfmCam.m_MtxR.aa[2][1];
				PickRayDir_WS.z = PickRayDir_VS.x * _XfmCam.m_MtxR.aa[0][2]
								+ PickRayDir_VS.y * _XfmCam.m_MtxR.aa[1][2]
								+ PickRayDir_VS.z * _XfmCam.m_MtxR.aa[2][2];

				CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
				CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
				CollInfo.bFindClosestImpactOnly = FALSE;
				CollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
				CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
				CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
				CollInfo.bCullBacksideCollisions = TRUE;
				CollInfo.Ray.Init( &_XfmCam.m_MtxR.m_vPos.v3, &(_XfmCam.m_MtxR.m_vPos.v3 + (PickRayDir_WS * _PICKRAY_LENGTH)) );

				fClosestImpactDist = -1;
				fcoll_Clear();
				if( _MeshInst.CollideWithMeshTris( &CollInfo ) ) {
					for( i=0; i<FColl_nImpactCount; i++ ) {
						ImpactDistVec = FColl_aImpactBuf[i].ImpactPoint.v3 - CollInfo.Ray.vStart_WS.v3;
						fImpactDist = ImpactDistVec.Mag();

						if( (fClosestImpactDist == -1) || (fImpactDist < fClosestImpactDist) ) {
							fClosestImpactDist = fImpactDist;
							nClosestImpactIndex = i;
						}
					}

					if( fClosestImpactDist != -1 ) {
//						fdx8mesh_DrawWhiteCookie( &FColl_aImpactBuf[nClosestImpactIndex].Cookie );

						if( bLeftButton ) {
							NewOrigin_WS = FColl_aImpactBuf[nClosestImpactIndex].aTriVtx[0].v3
										 + FColl_aImpactBuf[nClosestImpactIndex].aTriVtx[1].v3
										 + FColl_aImpactBuf[nClosestImpactIndex].aTriVtx[2].v3;
							NewOrigin_WS *= (1.0f/3.0f);
							CFVec3A V1, V2;
							V1.Set( NewOrigin_WS );
							_MeshInst.m_Xfm.m_MtxR.MulPoint( V2, V1 );
							NewOrigin_MS = V2.v3;
							XfmNewOrigin.BuildTranslation( NewOrigin_MS );
							_XfmModelOrigin.ReceiveInverseOf( XfmNewOrigin );
							_pArcBall->m_Mtx.m_vPos.v3 = NewOrigin_WS;
						} else {
						}
					}
				}
			}
		} else {
			// Move mode...
			if( bLeftButton ) {
				if( _bLastLeftButton ) {
					_pArcBall->Rotate( nX, nY );
				} else {
					_pArcBall->BeginRotate( nX, nY );
				}
			} else {
				if( _bLastLeftButton ) {
					_pArcBall->EndRotate( nX, nY );
				}
			}

			if( bShiftKeyDown ) {
				if( bRightButton ) {
					if( _bLastRightButton ) {
						_pArcBall->TranslateZ( nX, nY );
					} else {
						_pArcBall->BeginTranslateZ( nX, nY );
					}
				} else {
					if( _bLastRightButton ) {
						_pArcBall->EndTranslateZ( nX, nY );
					}
				}
			} else {
				if( bRightButton ) {
					if( _bLastRightButton ) {
						_pArcBall->Translate( nX, nY );
					} else {
						_pArcBall->BeginTranslate( nX, nY );
					}
				} else {
					if( _bLastRightButton ) {
						_pArcBall->EndTranslate( nX, nY );
					} else {
						if( !bMixerMode && nDeltaWheel ) {
							_pArcBall->m_Mtx.m_vPos.z -= ((f32)nDeltaWheel * (1.0f/20.0f));
						}
					}
				}
			}
		}
	}

	_nLastMouseX = nX;
	_nLastMouseY = nY;
	_bLastLeftButton = bLeftButton;
	_bLastRightButton = bRightButton;
}

static void _AdjustMixer( s32 nControlID, s32 nDeltaWheel ) {
	f32 fMixerValue, fDeltaWheel;

	fDeltaWheel = -(f32)nDeltaWheel;

	fMixerValue = _pCombiner->GetControlValue( _anControlID[nControlID] );
	fMixerValue += (fDeltaWheel * 0.0005f);
	FMATH_CLAMP( fMixerValue, 0.0f, 1.0f );
	_pCombiner->SetControlValue( _anControlID[nControlID], fMixerValue );
}

void CJawsDlg::OnButtonBackground() {
	CColorDialog ColorDlg( _COLORRGB_TO_COLORREF( &m_BackColorRGB ), CC_ANYCOLOR | CC_FULLOPEN, this );

	if( ColorDlg.DoModal() == IDOK ) {
		_COLORREF_TO_COLORRGB( &m_BackColorRGB, ColorDlg.GetColor() );
	}
}

void CJawsDlg::OnButtonAmbient() {
	CColorDialog ColorDlg( _COLORRGB_TO_COLORREF( &m_AmbientColorRGB ), CC_ANYCOLOR | CC_FULLOPEN, this );

	if( ColorDlg.DoModal() == IDOK ) {
		_COLORREF_TO_COLORRGB( &m_AmbientColorRGB, ColorDlg.GetColor() );
	}
}

void CJawsDlg::OnButtonDirectional() {
	CColorDialog ColorDlg( _COLORRGB_TO_COLORREF( &m_DirColorRGB ), CC_ANYCOLOR | CC_FULLOPEN, this );

	if( ColorDlg.DoModal() == IDOK ) {
		_COLORREF_TO_COLORRGB( &m_DirColorRGB, ColorDlg.GetColor() );
	}
}

void CJawsDlg::OnCheckAmbient() {
	m_bAmbientOn = ((CButton *)GetDlgItem(IDC_CHECK_AMBIENT))->GetCheck();
}

void CJawsDlg::OnCheckBackground() {
	m_bBackgroundClear = ((CButton *)GetDlgItem(IDC_CHECK_BACKGROUND))->GetCheck();
}

void CJawsDlg::OnCheckDirectional() {
	m_bDirOn = ((CButton *)GetDlgItem(IDC_CHECK_DIRECTIONAL))->GetCheck();
}

void CJawsDlg::OnButtonAnim1() {
	OnButtonAnim(0);
}

void CJawsDlg::OnButtonAnim2() {
	OnButtonAnim(1);
}

void CJawsDlg::OnButtonAnim3() {
	OnButtonAnim(2);
}

void CJawsDlg::OnButtonAnim4() {
	OnButtonAnim(3);
}

void CJawsDlg::OnCheckLink1() {
	m_abLink[0] = ((CButton *)GetDlgItem(IDC_CHECK_LINK1))->GetCheck();
}

void CJawsDlg::OnCheckLink2() {
	m_abLink[1] = ((CButton *)GetDlgItem(IDC_CHECK_LINK2))->GetCheck();
}

void CJawsDlg::OnButtonX1() {
	m_asAnimResName[0].Empty();
	UpdateControls();
	m_abLoadAnim[0] = TRUE;
}

void CJawsDlg::OnButtonX2() {
	m_asAnimResName[1].Empty();
	UpdateControls();
	m_abLoadAnim[1] = TRUE;
}

void CJawsDlg::OnButtonX3() {
	m_asAnimResName[2].Empty();
	UpdateControls();
	m_abLoadAnim[2] = TRUE;
}

void CJawsDlg::OnButtonX4() {
	m_asAnimResName[3].Empty();
	UpdateControls();
	m_abLoadAnim[3] = TRUE;
}

void CJawsDlg::OnCheckLoop1() {
	m_abLoop[0] = ((CButton *)GetDlgItem(IDC_CHECK_LOOP1))->GetCheck();
}

void CJawsDlg::OnCheckLoop2() {
	m_abLoop[1] = ((CButton *)GetDlgItem(IDC_CHECK_LOOP2))->GetCheck();
}

void CJawsDlg::OnCheckLoop3() {
	m_abLoop[2] = ((CButton *)GetDlgItem(IDC_CHECK_LOOP3))->GetCheck();
}

void CJawsDlg::OnCheckLoop4() {
	m_abLoop[3] = ((CButton *)GetDlgItem(IDC_CHECK_LOOP4))->GetCheck();
}

void CJawsDlg::OnCheckEnableAnim() {
	m_bEnableAnim = ((CButton *)GetDlgItem(IDC_CHECK_ENABLE_ANIM))->GetCheck();
}

void CJawsDlg::OnCheckPauseAnim() {
	m_bPauseAnim = ((CButton *)GetDlgItem(IDC_CHECK_PAUSE_ANIM))->GetCheck();
}

static void _FreeAnimResources( void ) {
	fres_ReleaseFrame( _ResFrame_Anim );
	_ResFrame_Anim = fres_GetFrame();
}

static BOOL _LoadAnimFiles( CJawsDlg *pDlg ) {
	u32 i;
	BOOL bSuccess;
	FResFrame_t ResFrame;

	_FreeAnimResources();

	// Create combiner...
	_pCombiner = fnew CFAnimCombiner;
	if( _pCombiner == NULL ) {
		return FALSE;
	}

	if( !_pCombiner->Create( _pCombinerConfig, &_MeshInst ) ) {
		fdelete( _pCombiner );
		_pCombiner = NULL;
		return FALSE;
	}

	_anControlID[0] = _pCombiner->GetControlID( "Mixer0" );
	_anControlID[1] = _pCombiner->GetControlID( "Mixer1" );
	_anControlID[2] = _pCombiner->GetControlID( "Mixer2" );
	_anTapID[0] = _pCombiner->GetTapID( "Tap0" );
	_anTapID[1] = _pCombiner->GetTapID( "Tap1" );
	_anTapID[2] = _pCombiner->GetTapID( "Tap2" );
	_anTapID[3] = _pCombiner->GetTapID( "Tap3" );

	FASSERT( _anControlID[0] != -1 );
	FASSERT( _anControlID[1] != -1 );
	FASSERT( _anControlID[2] != -1 );
	FASSERT( _anTapID[0] != -1 );
	FASSERT( _anTapID[1] != -1 );
	FASSERT( _anTapID[2] != -1 );
	FASSERT( _anTapID[3] != -1 );

	bSuccess = TRUE;

	ffile_SetupFileMode();
	for( i=0; i<4; i++ ) {
		ResFrame = fres_GetFrame();

		if( !pDlg->m_asAnimResName[i].IsEmpty() ) {
			_apResAnim[i] = (FAnim_t *)fresload_Load( FANIM_RESNAME, pDlg->m_asAnimResName[i] );
			if( _apResAnim[i] == NULL ) {
				_apAnimInst[i] = NULL;
				bSuccess = FALSE;
				continue;
			}

			_apAnimInst[i] = fnew CFAnimInst;
			if( _apAnimInst[i] == NULL ) {
				fres_ReleaseFrame( ResFrame );
				bSuccess = FALSE;
				continue;
			}

			if( !_apAnimInst[i]->Create( _apResAnim[i] ) ) {
				fdelete( _apAnimInst[i] );
				_apAnimInst[i] = NULL;
				fres_ReleaseFrame( ResFrame );
				bSuccess = FALSE;
				continue;
			}
		}

		_pCombiner->AttachToTap( _anTapID[i], _apAnimInst[i] );

		_abAnimRunning[i] = pDlg->m_abLoop[i];
	}

	return bSuccess;
}

void CJawsDlg::OnButtonAnim( u32 nTapID ) {
	static u32 __nInFcnCount = 0;

	__nInFcnCount++;

	if( __nInFcnCount != 1 ) {
		__nInFcnCount--;
		return;
	}

	// use the pick asset dialog to pick an animation file from the master file
	CPickAsset PickAssetDlg;

	PickAssetDlg.m_nOptions = CPickAsset::PICK_ASSET_ANIMATIONS;
	PickAssetDlg.m_sInputMasterFile = m_Settings.sMasterFile;
	PickAssetDlg.m_bReadOnly = TRUE;
	PickAssetDlg.m_sInitialSelection = m_asAnimResName[nTapID];
	PickAssetDlg.m_sInitialSelection += ".mtx";
	if( PickAssetDlg.DoModal() == IDOK ) {
		m_asAnimResName[nTapID] = PickAssetDlg.GetFileTitle();	
		
		UpdateControls();

//		m_bReloadMasterFile = TRUE;
		m_abLoadAnim[nTapID] = TRUE;
	}
	
	__nInFcnCount--;
}

static void _DisplayMixerAdjust( s32 nControlID ) {
	ftext_DebugPrintf( 0.5f, 0.01f, "~w1Mixer %i = %3.0f%% Top", nControlID+1, (1.0f - _pCombiner->GetControlValue(_anControlID[nControlID]))*100.0f );
}

static void _UpdateAnimations( CJawsDlg *pDlg, f32 fSecondsSinceLastCall ) {
	BOOL bOverflowed;
	u32 i, nMixer, nPartner;
	f32 fTimeRatio;

	for( i=0; i<4; i++ ) {
		nMixer = i >> 1;
		nPartner = _anPartnerIndex[i];

		if( pDlg->m_abLoop[i] ) {
			_abAnimRunning[i] = TRUE;
		}

		if( _apAnimInst[i] && _abAnimRunning[i] ) {
			if( pDlg->m_abLoop[i] && pDlg->m_abLink[nMixer] && _apAnimInst[nPartner] && pDlg->m_abLoop[nPartner] && (i > nPartner) ) {
				// We're the higher tap of a linked, 2-party looping animation system.
				// Our animation will be driven by the lower tap...
				continue;
			}

			if( pDlg->m_abLoop[i] && pDlg->m_abLink[nMixer] && _apAnimInst[nPartner] && pDlg->m_abLoop[nPartner] && (i < nPartner) ) {
				// We're the lower tap of a linked, 2-party looping animation system...

				// Update our animation...
				fTimeRatio = _apAnimInst[i]->GetTotalTime() * _apAnimInst[nPartner]->GetOOTotalTime();
				_apAnimInst[i]->DeltaTime( fSecondsSinceLastCall * FMATH_FPOT( _pCombiner->GetControlValue(_anControlID[nMixer]), 1.0f, fTimeRatio ) );

				// Update our partner's animation...
				_apAnimInst[nPartner]->UpdateUnitTime( _apAnimInst[i]->GetUnitTime() );
				continue;
			}

			bOverflowed = _apAnimInst[i]->DeltaTime( fSecondsSinceLastCall );

			if( bOverflowed ) {
				// We have gone past the end of the animation...

				if( pDlg->m_abLoop[i] ) {
					// We're a looping animation...

					if( pDlg->m_abLink[nMixer] && _apAnimInst[nPartner] && !pDlg->m_abLoop[nPartner] ) {
						// We're linked to our partner who is a one-shot...
						_apAnimInst[nPartner]->UpdateUnitTime( _apAnimInst[i]->GetUnitTime() );
						_abAnimRunning[nPartner] = TRUE;
					}
				} else {
					// We're a one-shot...
					_apAnimInst[i]->UpdateUnitTime( 1.0f );
					_abAnimRunning[i] = FALSE;
				}
			}
		}
	}
}

static void _TriggerOneShotAnim( s32 nTapID, CJawsDlg *pDlg ) {
	u32 nMixer, nPartner;

	if( _apAnimInst[nTapID] && !pDlg->m_abLoop[nTapID] ) {
		nMixer = nTapID >> 1;
		nPartner = _anPartnerIndex[nTapID];

		_apAnimInst[nTapID]->UpdateUnitTime( 0.0f );
		_abAnimRunning[nTapID] = TRUE;

		if( pDlg->m_abLink[nMixer] && _apAnimInst[nPartner] && !pDlg->m_abLoop[nPartner] ) {
			// We're linked to our partner who is a one-shot...
			_apAnimInst[nPartner]->UpdateUnitTime( 0.0f );
			_abAnimRunning[nPartner] = TRUE;
		}
	}
}

void CJawsDlg::OnButtonSaveconfig() {
	int nRetCode;
	CString sFilename;
	CFile *pFile;
	CArchive *pArchive;
	BOOL bSuccess = FALSE;

	pFile = NULL;
	pArchive = NULL;

	CFileDialog FileDlg( FALSE, "jaw", NULL, OFN_ENABLESIZING | OFN_OVERWRITEPROMPT, "Jaw (*.jaw)|*.jaw|All Files (*.*)|*.*||", this );
	nRetCode = FileDlg.DoModal();
	if( nRetCode != IDOK ) {
		return;
	}

	sFilename = FileDlg.GetPathName();

	pFile = new CFile;
	if( pFile == NULL ) {
		goto _ExitSaveConfig;
	}

	if( !pFile->Open( sFilename, CFile::modeWrite | CFile::modeCreate ) ) {
		goto _ExitSaveConfig;
	}

	pArchive = new CArchive( pFile, CArchive::store );
	if( pArchive == NULL ) {
		goto _ExitSaveConfig;
	}

	try {
		*pArchive << (int)_SERIALIZE_VERSION;
		*pArchive << m_sResName;
		*pArchive << m_sResType;
		*pArchive << m_sDefaultLoadExt;
		*pArchive << (int)_COLORRGB_TO_COLORREF( &m_BackColorRGB );
		*pArchive << (int)_COLORRGB_TO_COLORREF( &m_AmbientColorRGB );
		*pArchive << (int)_COLORRGB_TO_COLORREF( &m_DirColorRGB );
		*pArchive << m_bBackgroundClear;
		*pArchive << m_bAmbientOn;
		*pArchive << m_bDirOn;
		*pArchive << m_DirLightVector.x;
		*pArchive << m_DirLightVector.y;
		*pArchive << m_DirLightVector.z;
		*pArchive << m_asAnimResName[0];
		*pArchive << m_asAnimResName[1];
		*pArchive << m_asAnimResName[2];
		*pArchive << m_asAnimResName[3];
		*pArchive << m_abLoop[0];
		*pArchive << m_abLoop[1];
		*pArchive << m_abLoop[2];
		*pArchive << m_abLoop[3];
		*pArchive << m_abLink[0];
		*pArchive << m_abLink[1];
		*pArchive << m_abLink[2];
		*pArchive << m_abLink[3];
		*pArchive << m_bEnableAnim;
		*pArchive << m_bPauseAnim;

		SerializeXfm( pArchive, &_MeshInst.m_Xfm );
		SerializeXfm( pArchive, &_XfmCam );

		*pArchive << m_bShowBound;
		*pArchive << m_bShowBones;
	}
	catch( CException * ) {
		goto _ExitSaveConfig;
	}

	bSuccess = TRUE;

_ExitSaveConfig:
	if( pArchive ) {
		pArchive->Close();
		delete pArchive;
	}

	if( pFile ) {
		pFile->Close();
		delete pFile;
	}

	if( !bSuccess ) {
		MessageBox( "Trouble saving configuration data.", "Jaws Error" );
	}
}

void CJawsDlg::OnButtonLoadconfig() {
	int nRetCode;
	CString sFilename;
	CFile *pFile;
	CArchive *pArchive;
	BOOL bSuccess;
	int nSerializeVersion, nColorBack, nColorAmb, nColorDir;

	pFile = NULL;
	pArchive = NULL;
	bSuccess = FALSE;

	CFileDialog FileDlg( TRUE, NULL, NULL, OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, "Jaw (*.jaw)|*.jaw|All Files (*.*)|*.*||", this );
	nRetCode = FileDlg.DoModal();
	if( nRetCode != IDOK ) {
		return;
	}

	sFilename = FileDlg.GetPathName();

	pFile = new CFile;
	if( pFile == NULL ) {
		goto _ExitLoadConfig;
	}

	if( !pFile->Open( sFilename, CFile::modeRead ) ) {
		goto _ExitLoadConfig;
	}

	pArchive = new CArchive( pFile, CArchive::load );
	if( pArchive == NULL ) {
		goto _ExitLoadConfig;
	}

	try {
		*pArchive >> nSerializeVersion;

		switch( nSerializeVersion ) {
		case 1:
			*pArchive >> m_sResName;
			*pArchive >> m_sResType;
			*pArchive >> m_sDefaultLoadExt;
			*pArchive >> nColorBack;
			*pArchive >> nColorAmb;
			*pArchive >> nColorDir;
			*pArchive >> m_bBackgroundClear;
			*pArchive >> m_bAmbientOn;
			*pArchive >> m_bDirOn;
			*pArchive >> m_DirLightVector.x;
			*pArchive >> m_DirLightVector.y;
			*pArchive >> m_DirLightVector.z;
			*pArchive >> m_asAnimResName[0];
			*pArchive >> m_asAnimResName[1];
			*pArchive >> m_asAnimResName[2];
			*pArchive >> m_asAnimResName[3];
			*pArchive >> (int)m_abLoop[0];
			*pArchive >> (int)m_abLoop[1];
			*pArchive >> (int)m_abLoop[2];
			*pArchive >> (int)m_abLoop[3];
			*pArchive >> (int)m_abLink[0];
			*pArchive >> (int)m_abLink[1];
			*pArchive >> (int)m_abLink[2];
			*pArchive >> (int)m_abLink[3];
			*pArchive >> (int)m_bEnableAnim;
			*pArchive >> (int)m_bPauseAnim;

			DeserializeXfm( pArchive, &_MeshInst.m_Xfm );
			DeserializeXfm( pArchive, &_XfmCam );

			_COLORREF_TO_COLORRGB( &m_BackColorRGB, nColorBack );
			_COLORREF_TO_COLORRGB( &m_AmbientColorRGB, nColorAmb );
			_COLORREF_TO_COLORRGB( &m_DirColorRGB, nColorDir );

			break;

		case 2:
			*pArchive >> m_sResName;
			*pArchive >> m_sResType;
			*pArchive >> m_sDefaultLoadExt;
			*pArchive >> nColorBack;
			*pArchive >> nColorAmb;
			*pArchive >> nColorDir;
			*pArchive >> m_bBackgroundClear;
			*pArchive >> m_bAmbientOn;
			*pArchive >> m_bDirOn;
			*pArchive >> m_DirLightVector.x;
			*pArchive >> m_DirLightVector.y;
			*pArchive >> m_DirLightVector.z;
			*pArchive >> m_asAnimResName[0];
			*pArchive >> m_asAnimResName[1];
			*pArchive >> m_asAnimResName[2];
			*pArchive >> m_asAnimResName[3];
			*pArchive >> (int)m_abLoop[0];
			*pArchive >> (int)m_abLoop[1];
			*pArchive >> (int)m_abLoop[2];
			*pArchive >> (int)m_abLoop[3];
			*pArchive >> (int)m_abLink[0];
			*pArchive >> (int)m_abLink[1];
			*pArchive >> (int)m_abLink[2];
			*pArchive >> (int)m_abLink[3];
			*pArchive >> (int)m_bEnableAnim;
			*pArchive >> (int)m_bPauseAnim;

			DeserializeXfm( pArchive, &_MeshInst.m_Xfm );
			DeserializeXfm( pArchive, &_XfmCam );

			*pArchive >> (int)m_bShowBound;
			*pArchive >> (int)m_bShowBones;

			_COLORREF_TO_COLORRGB( &m_BackColorRGB, nColorBack );
			_COLORREF_TO_COLORRGB( &m_AmbientColorRGB, nColorAmb );
			_COLORREF_TO_COLORRGB( &m_DirColorRGB, nColorDir );

			break;
		default:
			goto _ExitLoadConfig;
		}
	}
	catch( CException * ) {
		goto _ExitLoadConfig;
	}

	bSuccess = TRUE;

_ExitLoadConfig:
	if( pArchive ) {
		pArchive->Close();
		delete pArchive;
	}

	if( pFile ) {
		pFile->Close();
		delete pFile;
	}

	if( !bSuccess ) {
		MessageBox( "Trouble reading configuration data.", "Jaws Error" );
	}

	UpdateControls();

	if( bSuccess ) {
		m_bOrientSet = TRUE;

		if( m_bRunning ) {
			m_bLoad = TRUE;
		} else {
			OnButtonRun();
		}
	}
}

void CJawsDlg::SerializeXfm( CArchive *pArchive, CFXfm *pXfm ) {
	u32 i;

	for( i=0; i<12; i++ ) {
		*pArchive << pXfm->m_MtxF.a[i];
		*pArchive << pXfm->m_MtxR.a[i];
	}
	*pArchive << pXfm->m_fScaleF;
	*pArchive << pXfm->m_fScaleR;
	*pArchive << 0;
}

void CJawsDlg::DeserializeXfm( CArchive *pArchive, CFXfm *pXfm ) {
	int i, nValue;

	for( i=0; i<12; i++ ) {
		*pArchive >> pXfm->m_MtxF.a[i];
		*pArchive >> pXfm->m_MtxR.a[i];
	}
	*pArchive >> pXfm->m_fScaleF;
	*pArchive >> pXfm->m_fScaleR;
	*pArchive >> nValue;
}

void CJawsDlg::OnCheckPerf() {
	m_bPerfOn = ((CButton *)GetDlgItem(IDC_CHECK_PERF))->GetCheck();
}

void CJawsDlg::OnCheckFpsClick() {
	m_bFPSOn = ((CButton *)GetDlgItem(IDC_CHECK_FPS))->GetCheck();	
}

void CJawsDlg::OnHideCursorClick() {
	m_bDrawCursor = !( ((CButton *)GetDlgItem(IDC_HIDE_CURSOR))->GetCheck() );	
}

void CJawsDlg::OnCheckShowBounds() {
	m_bShowBound = ((CButton *)GetDlgItem(IDC_CHECK_SHOW_BOUNDS))->GetCheck();

	if( m_bReady ) {
		frenderer_DrawBound_Enable( m_bShowBound );
	}
}

void CJawsDlg::OnCheckShowBones() {
	m_bShowBones = ((CButton *)GetDlgItem(IDC_CHECK_SHOW_BONES))->GetCheck();

	if( m_bReady ) {
		frenderer_DrawBones_Enable( m_bShowBones );
	}
}

static void _DisplayPerf( void ) {
	f32 fFPS;

	fFPS = 1.0f / _fAvgFrameSecs;

	ftext_DebugPrintf( 0.99f, 0.425f, "~w1~arHeap: %d", Fheap_nTotalMemTracked );
	ftext_DebugPrintf( 0.99f, 0.440f, "~w1~arObjs: %6u", FPerf_nMeshInstDrawnCount );
//	ftext_DebugPrintf( 0.99f, 0.465f, "~w1~arNodes: %6u", FPerf_nMeshNodeDrawnCount );
	ftext_DebugPrintf( 0.99f, 0.490f, "~w1~arMtls: %6u", FPerf_nMeshMtlDrawnCount );
	ftext_DebugPrintf( 0.99f, 0.515f, "~w1~arTris: %6u", FPerf_nRawStripTriCount + FPerf_nRawListTriCount );
	ftext_DebugPrintf( 0.99f, 0.540f, "~w1~arVtxs: %6u", FPerf_nRawVertexCount );
	ftext_DebugPrintf( 0.99f, 0.565f, "~w1~arKT/s: %6u", (u32)( (f32)(FPerf_nRawStripTriCount + FPerf_nRawListTriCount) * fFPS * (1.0f/1000.0f) ) );
	ftext_DebugPrintf( 0.99f, 0.590f, "~w1~arKV/s: %6u", (u32)( (f32)FPerf_nRawVertexCount * fFPS * (1.0f/1000.0f) ) );
	ftext_DebugPrintf( 0.99f, 0.615f, "~w1~arV/T: %6.1f", (FPerf_nRawStripTriCount + FPerf_nRawListTriCount) ? (f32)FPerf_nRawVertexCount / (f32)(FPerf_nRawStripTriCount + FPerf_nRawListTriCount) : 0.0f );
//	ftext_DebugPrintf( 0.99f, 0.640f, "~w1~arMB/s: %6u", (u32)( (f32)FPerf_nRawGeoByteCount * fFPS * (1.0f/1000000.0f) ) );
	ftext_DebugPrintf( 0.99f, 0.665f, "~w1~arTex Switch: %4u/%3u", FPerf_nRawTexSwitchCount, FPerf_nUniqueTexturesUsed );
	ftext_DebugPrintf( 0.99f, 0.690f, "~w1~arVB Switch: %6u", FPerf_nRawVBSwitchCount );
	ftext_DebugPrintf( 0.99f, 0.715f, "~w1~arSha Switch: %6u", FPerf_nRawShSwitchCount );
}




static void _DevPrintf( cchar *pszFormat, FANG_VA_LIST Args ) {
	_pWinPrintf->Printf( pszFormat, Args );
}

void CJawsDlg::On30fpsClick() {

	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_30FPS ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_30FPS;
		_fFrameRateMultiplier = 1.0f;
	}
}

void CJawsDlg::On60fpsClick() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_60FPS ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_60FPS;
		_fFrameRateMultiplier = 1.0f;
	}
}

void CJawsDlg::OnRealtimeFpsClick() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_FPS ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_REALTIME;
		_fFrameRateMultiplier = 1.0f;
	}
}

void CJawsDlg::OnRealtimeSlow10() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_SLOW10 ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_SLOWMOTION;
		_fFrameRateMultiplier = (1.0f/2.0f);
	}
}

void CJawsDlg::OnRealtimeSlow20() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_SLOW20 ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_SLOWMOTION;
		_fFrameRateMultiplier = (1.0f/3.0f);
	}
}

void CJawsDlg::OnRealtimeSlow30() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_SLOW30 ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_SLOWMOTION;
		_fFrameRateMultiplier = (1.0f/4.0f);
	}
}

void CJawsDlg::OnRealtimeSlow40() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_SLOW40 ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_SLOWMOTION;
		_fFrameRateMultiplier = (1.0f/5.0f);
	}
}

void CJawsDlg::OnRealtimeSlow50() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_SLOW50 ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_SLOWMOTION;
		_fFrameRateMultiplier = (1.0f/10.0f);
	}
}

void CJawsDlg::OnRealtimeSlow75() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_SLOW75 ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_SLOWMOTION;
		_fFrameRateMultiplier = (1.0f/20.0f);
	}
}

void CJawsDlg::OnRealtimeSlow100() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_REALTIME_SLOW100 ))->GetCheck() == 1 );
	if( bState ) {
		_nAnimPlaybackRate = _ANIM_PLAYBACK_SLOWMOTION;
		_fFrameRateMultiplier = (1.0f/60.0f);
	}
}

void CJawsDlg::OnSeriesClick() {

	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_SERIES ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotMode( FALSE );
	}	
}

void CJawsDlg::OnSingleClick() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_SINGLE ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotMode( TRUE );
	}
}

void CJawsDlg::OnTgaClick() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_TGA ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotType( SCREENSHOT_OUTPUT_TYPE_TGA );
	}
}

void CJawsDlg::OnJpegClick() {
	
	UpdateData( CONTROLS_TO_VARS );
	BOOL bState = ( ((CButton *)GetDlgItem( IDC_JPEG ))->GetCheck() == 1 );
	if( bState ) {
		screenshot_SetScreenShotType( SCREENSHOT_OUTPUT_TYPE_JPEG );
	}
}


void CJawsDlg::OnDontRenderSurface() {
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

void CJawsDlg::OnDontRenderSpecular() {
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

void CJawsDlg::OnNMCustomDrawLODSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	u32 nPos = ((CSliderCtrl*)GetDlgItem( IDC_LOD_SLIDER ))->GetPos();
	if ( nPos == 0 )
	{
		m_sForceLOD = "Force LOD disabled";
		FMesh_nForceLOD = -1;
	}
	else
	{
		FMesh_nForceLOD = nPos - 1;
		m_sForceLOD.Format( "Force LOD to %d", FMesh_nForceLOD );
	}
	UpdateData( FALSE );
}


void CJawsDlg::OnNMCustomdrawFovSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	u32 nPos = ((CSliderCtrl*)GetDlgItem( IDC_FOV_SLIDER ))->GetPos();
	nPos += _MIN_FOV;
	FMATH_CLAMP( nPos, _MIN_FOV, _MAX_FOV );
	_fNewFOV = FMATH_DEG2RAD( (f32)nPos );

	*pResult = 0;
}
