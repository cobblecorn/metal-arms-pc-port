// SpitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fang.h"
#include "Spit.h"
#include "SpitDlg.h"

#include "fmath.h"
#include "fxfm.h"
#include "fversion.h"
#include "settings.h"
#include "fviewport.h"
#include "fvis.h"
#include "fboxfilter.h"
#include "floop.h"
#include "dx/fdx8vid.h"
#include "fperf.h"
#include "VidMode.h"
#include "DefFileIO.h"
#include "fdraw.h"
#include "fclib.h"
#include "frenderer.h"
#include "fresload.h"
#include "ffile.h"
#include "ftex.h"
#include "flightgroup.h"

#include <afxmt.h>

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

#define _MSG_STARTRENDERING		( WM_USER + 400 )
#define _MSG_LOAD_DEF_FILE		( WM_USER + 401 )
#define _UPDATE_CONTROLS		( WM_USER + 402 )

#define _FLOOR_Y				-5.0f
#define _EMITTER_DELTA_Y		100.0f

#define _WORLD_TO_LOAD			"wldparticl1"
// camera defines
#define _CAM_XLAT_SPEED			0.40f//0.30f//0.5f
#define _CAM_SPIN_SPEED			0.035f

#define _WINDOW_TITLE			"SPIT - Super Particle Interface Tool"

typedef enum {
	_LO = 0,
	_HIGH,

	_COUNT
} _EditableLevel_t;

enum {
    _EMIT_FLAGS_FREEZE					= 0x00000001,
    _EMIT_FLAGS_UN_FREEZE				= 0x00000002,
	_EMIT_FLAGS_SLOW_MO					= 0x00000004,
	_EMIT_FLAGS_AID						= 0x00000008,
	_EMIT_FLAGS_BOUNDS					= 0x00000010,
	_EMIT_FLAGS_SPAWN					= 0x00000020,
	_EMIT_FLAGS_LOOKAT					= 0x00000040,
	_EMIT_FLAGS_AMBIENT					= 0x00000080,
	_EMIT_FLAGS_STOP_ALL				= 0x00000100,
	_EMIT_FLAGS_LIGHT_BS				= 0x00000200,
	_EMIT_FLAGS_LIGHT_SETTINGS_CHANGED	= 0x00000400,
	_EMIT_FLAGS_KILL_ALL				= 0x00000800,

	_EMIT_FLAGS_NONE		= 0x00000000
};

// STATIC VARS:
static CFXfm _XfmCam;
static CFVec3 _CamPos;
static FViewport_t *_pViewport;
static FVisData_t *_pWorldRes;
static FBoxFilterHandle_t _hBoxFilterFrame;
static f32 _fFrameSecs;
static f32 _fCamHeading;
static f32 _fCamPitch;

static BOOL _bMouseCaptured = FALSE;
static BOOL _bRightButtonDown = FALSE;
static u32 _nLastMouseX;
static u32 _nLastMouseY;
static s32 _nDeltaWheel;
static s32 _nDeltaMouseX;
static s32 _nDeltaMouseY;

static FParticleDef_t *_pFangParticleDef;
static FResFrame_t _ResFrame;
static CString _sLastTextureLoaded;
static FTexDef_t *_pTexDef;
static CFTexInst *_pTexInst;
static FParticle_DefHandle_t _hParticleDef;
static FParticle_EmitterHandle_t _hEmitter = FPARTICLE_INVALID_HANDLE;

static CFVec3 _EmitterPos;
static CFVec3 _EmitterDir;
static f32 _fEmitterIntensity;
static CFVec3 _LookAtPoint;

/////////////////////////////////////////
// critical section vars
static CCriticalSection _CriticalSection;
static BOOL _bInitingRollups = FALSE;
static u32 _nEmitFlags = _EMIT_FLAGS_NONE;
static CFColorRGB _AmbientColorRGB;
/////////////////////////////////////////

// STATIC FUNCTIONS:
static BOOL _GameInit( void *pParameter );
static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter );
static BOOL _GameMain( BOOL bExitRequest, void *pParameter );
static void _FreeModeCameraWork( BOOL bHaveFocus, f32 fXlatSpeed, f32 fSpinSpeed, BOOL bControlKeyDown );
static void _LookAtModeCameraWork( BOOL bHaveFocus, f32 fXlatSpeed, f32 fSpinSpeed, BOOL bControlKeyDown );
static void _SetToDefaults( FParticleDef_t *pDef );

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
// CSpitDlg dialog

CSpitDlg::CSpitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpitDlg)
	m_sVersion = _T("");
	m_nCurrentEditableIntensity = -1;
	m_sStats = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hMenu = ::LoadMenu(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAIN_MENU));
}

void CSpitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpitDlg)
	DDX_Control(pDX, IDC_BUTTON_PICK_AMBIENT_LIGHT, m_ctrlAmbientLight);
	DDX_Control(pDX, IDC_BUTTON_COPY_SETTINGS, m_ctrlCopyIntensity);
	DDX_Control(pDX, IDC_BUTTON_COPY_CURRENT_SETTINGS, m_ctrlCopyCurrent);
	DDX_Control(pDX, IDC_STATIC_VIEWFRAME, m_ctrlViewFrame);
	DDX_Control(pDX, IDC_STATIC_FRAME, m_ctrlRollupArea);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Radio(pDX, IDC_RADIO_MIN, m_nCurrentEditableIntensity);
	DDX_Text(pDX, IDC_STATS, m_sStats);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_EMIT_INTENSITY, m_ctrlEmitIntensity, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_EMIT_Y, m_ctrlEmitY, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_EMIT_ROT, m_ctrlEmitRot, 0);
}

BEGIN_MESSAGE_MAP(CSpitDlg, CDialog)
	//{{AFX_MSG_MAP(CSpitDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_NEW_FILE, OnNewFile)
	ON_COMMAND(ID_OPEN_FILE, OnOpenFile)
	ON_COMMAND(ID_SAVE_FILE, OnSaveFile)
	ON_COMMAND(ID_SAVEAS, OnSaveAs)
	ON_COMMAND(ID_SELECT_MASTERFILE, OnSelectMasterFile)
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_RADIO_MAX, OnRadioMax)
	ON_BN_CLICKED(IDC_RADIO_MIN, OnRadioMin)
	ON_BN_CLICKED(IDC_BUTTON_COPY_SETTINGS, OnButtonCopySettings)
	ON_BN_CLICKED(IDC_BUTTON_SET_TO_DEFAULTS, OnButtonSetToDefaults)
	ON_BN_CLICKED(IDC_BUTTON_EMIT_SHOT, OnButtonEmitShot)
	ON_EN_KILLFOCUS(IDC_EDIT_EMIT_INTENSITY, OnKillfocusEditEmitIntensity)
	ON_EN_KILLFOCUS(IDC_EDIT_EMIT_ROT, OnKillfocusEditEmitRot)
	ON_EN_KILLFOCUS(IDC_EDIT_EMIT_Y, OnKillfocusEditEmitY)
	ON_BN_CLICKED(IDC_CHECK_PAUSE, OnCheckPause)
	ON_BN_CLICKED(IDC_CHECK_DRAW_BOUNDING_SPHERE, OnCheckDrawBoundingSphere)
	ON_BN_CLICKED(IDC_CHECK_SLOW_MO, OnCheckSlowMo)
	ON_BN_CLICKED(IDC_CHECK_DRAW_AID, OnCheckDrawAid)
	ON_EN_CHANGE(IDC_EDIT_EMIT_Y, OnChangeEditEmitY)
	ON_EN_CHANGE(IDC_EDIT_EMIT_INTENSITY, OnChangeEditEmitIntensity)
	ON_EN_CHANGE(IDC_EDIT_EMIT_ROT, OnChangeEditEmitRot)
	ON_BN_CLICKED(IDC_CHECK_LOOK_AT_CAMERA, OnCheckLookAtCamera)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_CHECK_AMBIENT_LIGHT_ON, OnCheckAmbientLightOn)
	ON_BN_CLICKED(IDC_BUTTON_PICK_AMBIENT_LIGHT, OnButtonPickAmbientLight)
	ON_MESSAGE(_MSG_STARTRENDERING, OnStartRenderWindow)
	ON_MESSAGE(_MSG_LOAD_DEF_FILE, OnLoadDefFile)
	ON_MESSAGE(_UPDATE_CONTROLS, UpdateControls)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
	ON_COMMAND(ID_FILE_UPDATEVERSION, OnFileUpdateversion)
	ON_BN_CLICKED(IDC_BUTTON_COPY_CURRENT_SETTINGS, OnBnClickedButtonCopyCurrentSettings)
	ON_BN_CLICKED(IDC_CHECK_DRAW_LIGHT_BS, OnBnClickedCheckDrawLightBS)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpitDlg message handlers

BOOL CSpitDlg::OnInitDialog()
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
			pSysMenu->InsertMenu( 0, MF_STRING, IDM_ABOUTBOX, strAboutMenu );
			//pSysMenu->AppendMenu(MF_SEPARATOR);
			//pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		pSysMenu->EnableMenuItem( 2, MF_BYPOSITION | MF_GRAYED );
		pSysMenu->EnableMenuItem( 4, MF_BYPOSITION | MF_GRAYED );
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Set the menu bar for the window.
	::SetMenu(m_hWnd, m_hMenu);
	
	// TODO: Add extra initialization here
	::SetProp( GetSafeHwnd(), Spit_pszPropName, (HANDLE)1 ); 

	CSettings::SetSettingsFilename( "spit.ini" );

	// get the default values from our settings file
	CSettings& Settings = CSettings::GetCurrent();
	CSettings::SetApplicationName( "spit" );

	// init our vars from the settings file
	m_sCurrentFile = Settings.GetLastFileWorkedOn();
	m_sMasterFile = Settings.GetMasterFileLoc();
	Settings.GetCameraPos( _CamPos );
	_fCamHeading = Settings.GetCameraHeading();
	_fCamPitch = Settings.GetCameraPitch();
	_XfmCam.BuildInvRotYXZ_XlatFromPoint( _fCamHeading, _fCamPitch, 0.0f, _CamPos.x, _CamPos.y, _CamPos.z );

	_nEmitFlags = _EMIT_FLAGS_SPAWN | _EMIT_FLAGS_LOOKAT;
	if( Settings.m_bAmbientOn ) {
		_nEmitFlags |= _EMIT_FLAGS_AMBIENT;
		((CButton *)GetDlgItem( IDC_CHECK_AMBIENT_LIGHT_ON ))->SetCheck( TRUE );
	}
	((CButton *)GetDlgItem( IDC_CHECK_LOOK_AT_CAMERA ))->SetCheck( TRUE );
	_AmbientColorRGB.fRed = Settings.m_fAmbientRed;
	_AmbientColorRGB.fGreen = Settings.m_fAmbientGreen;
	_AmbientColorRGB.fBlue = Settings.m_fAmbientBlue;
	
	// set the particle data to their default values
	_SetToDefaults( &m_ParticleData );
	m_bDataNeedsSaving = FALSE;
	m_bDataHasBeenUpdated = FALSE;

	// set the editable intensity level
	m_nCurrentEditableIntensity = _HIGH;
	m_ctrlCopyIntensity.SetWindowText( "Copy Low  -->  Hi" );
	m_ctrlCopyCurrent.SetWindowText( "Copy Current --> Hi" );

	_bInitingRollups = TRUE;

	// init the edit controls for cam y, cam rot, and intensity
	m_ctrlEmitRot.SetRange( Settings.GetEmitRot(), 0, 359 );
	m_ctrlEmitY.SetRange( Settings.GetEmitY(), 0, 100 );
	m_ctrlEmitIntensity.SetRange( 100, 0, 100 );

	_bInitingRollups = FALSE;

	// create the rollup control
	CRect ClientRect;
	m_ctrlRollupArea.GetWindowRect( &ClientRect );
	ScreenToClient( &ClientRect );
	m_ctrlRollup.Create( WS_VISIBLE|WS_CHILD, ClientRect, this, 1234 );

	// add dialogs to the rollup control
	u32 nIndex = 0;
	m_ctrlGlobal.Create( MAKEINTRESOURCE(IDD_GLOBAL), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Global", &m_ctrlGlobal, FALSE );

	nIndex++;
	m_ctrlNumParticles.Create( MAKEINTRESOURCE(IDD_NUM_PARTICLES), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Particle Count", &m_ctrlNumParticles, FALSE );

	nIndex++;
	m_ctrlForces.Create( MAKEINTRESOURCE(IDD_FORCES), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Forces", &m_ctrlForces, FALSE );

	nIndex++;
	m_ctrlColors.Create( MAKEINTRESOURCE(IDD_COLOR), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Color", &m_ctrlColors, FALSE );

	nIndex++;
	m_ctrlLifeSpan.Create( MAKEINTRESOURCE(IDD_LIFESPAN), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Lifespan", &m_ctrlLifeSpan, FALSE );

	nIndex++;
	m_ctrlScale.Create( MAKEINTRESOURCE(IDD_SCALE), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Scale", &m_ctrlScale, FALSE );
	
	nIndex++;
	m_ctrlVelocity.Create( MAKEINTRESOURCE(IDD_VELOCITY), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Velocity", &m_ctrlVelocity, FALSE );

	nIndex++;
	m_ctrlLod.Create( MAKEINTRESOURCE(IDD_LOD), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "LOD", &m_ctrlLod, FALSE );

	nIndex++;
	m_ctrlPosition.Create( MAKEINTRESOURCE(IDD_POSITION), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Position", &m_ctrlPosition, FALSE );

	nIndex++;
	m_nSpriteRollupIndex = nIndex;
	m_ctrlSprite.Create( MAKEINTRESOURCE(IDD_SPRITES), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Sprites", &m_ctrlSprite, FALSE );
	
	nIndex++;
	m_ctrlEmissive.Create( MAKEINTRESOURCE(IDD_EMISSIVE), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Emissive", &m_ctrlEmissive, FALSE );

	nIndex++;
	m_nLightRollupIndex = nIndex;
	m_ctrlLights.Create( MAKEINTRESOURCE(IDD_LIGHT), &m_ctrlRollup );
	m_ctrlRollup.InsertPage( "Light && Sound", &m_ctrlLights, FALSE );

	// expand all of the pages
	m_ctrlRollup.ExpandAllPages( FALSE );

	// init the gameloop param
	m_GameloopParams.bGameloopRunning = FALSE;
	m_GameloopParams.pDlg = this;

	// make sure we have a master file
	if( m_sMasterFile.IsEmpty() ) {
		MessageBox(_T("You must select a master file before SPIT\ncan be used.  Click OK and then select\na master file."), 
					_T("SPIT Error: No Master File"),
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
			Settings.m_sMasterFileLocation = m_sMasterFile;
			Settings.SaveCommonDataOutToFile();
		} else 	{
			// no master file, no SPIT
			PostMessage( WM_QUIT );
			m_bDataNeedsSaving = FALSE;
			m_bDataHasBeenUpdated = FALSE;
			return TRUE;
		}
	}
		
	// init fang
	fang_Init();
	Fang_ConfigDefs.pszFile_MasterFilePathName = m_sMasterFile;
	Fang_ConfigDefs.nRes_HeapBytes = (16*1024*1024);
	Fang_ConfigDefs.nMaxParticleEmitters = 128;
	Fang_ConfigDefs.nMaxParticles = 4096;
	Fang_ConfigDefs.nMaxParticleEmitterSprites = 2048;

	if( !fang_Startup() ) {
		// could not install the fang engine, tell the user and exit
		this->MessageBox( "Fang Engine failed to install.", 
						  "SPIT: Init Failure",
						  MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 ); 
		m_bDataNeedsSaving = FALSE;
		m_bDataHasBeenUpdated = FALSE;
		return TRUE;
	}
	// don't near cull any particles in this tool
	fparticle_NoNearCulling();

	if( !ffile_IsInMasterMode() ) {
		// not in master file mode, quit
		this->MessageBox( "Could not load master file, check pasm and your master file path.", 
						  "Master File Failure",
						  MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 );  
		m_bDataNeedsSaving = FALSE;
		m_bDataHasBeenUpdated = FALSE;
		return TRUE;
	}

	// select a video mode
	CVidMode VidMode;
	if( !VidMode.GetVidMode( &m_GameloopParams.VidSettings ) ) {
		// Video mode not selected... 
		MessageBox( "You need to set the video mode before SPIT may be used.\nClick Ok and then select a Video button.", "SPIT Error: Set Video Mode" );
		if( VidMode.DoModal() == IDOK ) {
			VidMode.GetVidMode( &m_GameloopParams.VidSettings );
		} else 	{
			// no video mode, no SPIT
			PostMessage(WM_QUIT);
			m_bDataNeedsSaving = FALSE;
			m_bDataHasBeenUpdated = FALSE;
			return TRUE;
		}
	}
	m_GameloopParams.VidSettings.bAllowPowerSuspend = TRUE;
	m_GameloopParams.VidSettings.hInstance = theApp.m_hInstance;
	m_GameloopParams.VidSettings.hWnd = m_ctrlViewFrame.m_hWnd;
	m_GameloopParams.VidSettings.nIconIDI = IDR_MAINFRAME;
	m_GameloopParams.VidSettings.pFcnSuspend = NULL;
	strcpy( m_GameloopParams.VidSettings.szWindowTitle, "SPIT Render Window" );
	
	// display the version
	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );
	// set the window text
	SetWindowText( _WINDOW_TITLE );

	UpdateData( VARS_TO_CONTROLS );

	// now that fang is inited, we can rotate vecs
	m_fEmitY = (f32)Settings.GetEmitY();
	m_EmitDir = CFVec3::m_UnitAxisY;
	m_EmitDir.RotateX( FMATH_DEG2RAD( (f32)Settings.GetEmitRot() ) );
	m_fEmitIntensity = 1.0f;

	// load the last particle data file from disk
	if( !m_sCurrentFile.IsEmpty() ) {
		// post a message to load the current def file
		PostMessage( _MSG_LOAD_DEF_FILE );
	} else {
		// even though there is no file loaded, go ahead and init the rollups with the default values
		_SetToDefaults( &m_ParticleData );
		InitRollupDialogs();
	}

	// post a message to start up the fang renderer
	PostMessage( _MSG_STARTRENDERING );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSpitDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSpitDlg::OnPaint() 
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

HCURSOR CSpitDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSpitDlg::OnNewFile() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	// ask to save the current file if need be
	SaveData( TRUE );

	// erase the current filename
	m_sCurrentFile.Empty();

	// set the window text
	SetWindowText( _WINDOW_TITLE );

	// go ahead and behave like the user clicked the "Set To Defaults" button
	OnButtonSetToDefaults();
}

void CSpitDlg::OnOpenFile() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	// ask to save the current file if need be
	SaveData( TRUE );

	// pick the new file
	CFileDialog dlg( TRUE,
					 ".fpr",
					 m_sCurrentFile,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Master Files (*.fpr)|*.fpr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Fang Particle Resource File";
	if( dlg.DoModal() == IDOK ) {
		m_sCurrentFile = dlg.GetPathName();

		OnLoadDefFile( 0, 0 );
	}
}

void CSpitDlg::OnSaveFile() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	// save the current file to disk
	if( m_sCurrentFile.IsEmpty() ) {
		// actually handle this like a save as
		OnSaveAs();
	} else {
		SaveData( FALSE );
	}
}

void CSpitDlg::OnSaveAs() {

	UpdateData( CONTROLS_TO_VARS );
	
	// pick the new filename
	CFileDialog dlg( FALSE,
					 ".fpr",
					 "*.fpr",
					 OFN_EXTENSIONDIFFERENT | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
					 "Fang Particle Resource Files (*.fpr)|*.fpr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Save As";
	if( dlg.DoModal() == IDOK ) {
		// make sure that the new name is ok
		if( dlg.GetFileTitle().GetLength() > 11 ) {
			MessageBox( _T("Valid names must be 11 or fewer characters.\nPlease select a new name."), 
						_T("SPIT"),
						MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
			return;
		}
		// make sure that the user picked the correct extension
		if( dlg.GetFileExt().CompareNoCase( "fpr" ) != 0 ) {
			MessageBox( _T("All names must have an extension of '.fpr'.\nPlease select a new filename."), 
						_T("SPIT"),
						MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			return;
		}
				
		// save the new file name
		m_sCurrentFile = dlg.GetPathName();

		// grab the data lock
		_CriticalSection.Lock();

		// mark that the file needs saving
		m_bDataNeedsSaving = TRUE;

		SaveData( FALSE );

		// unlock the data
		_CriticalSection.Unlock();

		// set the window text
		CString sTitle;
		sTitle.Format( "%s - [%s]", _WINDOW_TITLE, m_sCurrentFile );
		SetWindowText( sTitle );
	}	
}

void CSpitDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	if( m_GameloopParams.bGameloopRunning ) {
		floop_UninstallGameloop();
	}
	while( m_GameloopParams.bGameloopRunning ) {
		Sleep( 15 );
	}
	if( fang_IsStartedUp() ) {
		fang_Shutdown();
	}
	
	// prompt the user to save their work
	SaveData( TRUE );

	SaveSettingsToDisk();

	::RemoveProp( GetSafeHwnd(), Spit_pszPropName );
	
	CDialog::OnCancel();
}

void CSpitDlg::SaveSettingsToDisk() {

	UpdateData( CONTROLS_TO_VARS );

	CSettings& Settings = CSettings::GetCurrent();

	Settings.m_sLastFileWorkedOn = m_sCurrentFile;
	Settings.m_sMasterFileLocation = m_sMasterFile;
	Settings.m_fCameraX = _XfmCam.m_MtxR.m_vPos.v3.x;
	Settings.m_fCameraY = _XfmCam.m_MtxR.m_vPos.v3.y;
	Settings.m_fCameraZ = _XfmCam.m_MtxR.m_vPos.v3.z;
	Settings.m_fCameraHeading = _fCamHeading;
	Settings.m_fCameraPitch = _fCamPitch;
	Settings.m_nEmitRot = m_ctrlEmitRot.GetPos();
	Settings.m_nEmitY = m_ctrlEmitY.GetPos();
	Settings.m_bAmbientOn = (_nEmitFlags & _EMIT_FLAGS_AMBIENT) ? TRUE : FALSE;
	Settings.m_fAmbientRed = _AmbientColorRGB.fRed;
	Settings.m_fAmbientGreen = _AmbientColorRGB.fGreen;
	Settings.m_fAmbientBlue = _AmbientColorRGB.fBlue;
		
	Settings.SaveCommonDataOutToFile();
}

void CSpitDlg::OnOK() {
	//OnCancel();
}

BOOL CSpitDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		switch( pMsg->wParam ) {
		case VK_RETURN:
			{
				// Detect the Enter (Return) key, and pass the message to the dialog.
				// This disables the "default button" functionality normally
				// found in dialogs and property sheets, and allows the user to hit
				// Enter to "commit" the changes (s)he is making to an edit field.
				CWnd* pControl = GetFocus();
				if( pControl ) {
					char pszString[256];
					if( ::GetClassName( pControl->m_hWnd, pszString, 256 ) ) {
						// see if we have an edit control
						CString s;
						s = "edit";
						if( !s.CompareNoCase( pszString ) ) {
							// the strings matched, we have an edit box, highlight the entire line and kill focus
							
							// Tell the dialog that Enter was pressed, and pass the ID
							// of the edit control which has the focus.
							PostMessage(WM_COMMAND, MAKELONG(pControl->GetDlgCtrlID(), EN_KILLFOCUS), (LPARAM) pControl->m_hWnd);
							
							// Highlight all text in the edit control.
							pControl->PostMessage( EM_SETSEL, 0, -1 );

							return TRUE;
						} 
					}
				}
			}
			break;
		case VK_ESCAPE:
			// Esc key should not kill the app, do nothing
			return TRUE;
			break;
		case VK_HOME:
		case 'A':
		case 'S':
		case 'W':
		case 'D':
		case 'R':
		case 'F':
		case 'Q':
		case 'E':
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_CONTROL:
			// only if we are capturing the mouse do we want to ignore this messages
			if( _bMouseCaptured ) {
				return TRUE;
			}
			break;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CSpitDlg::OnSelectMasterFile() {
	CString sOldMasterFile;
	
	UpdateData( CONTROLS_TO_VARS );

	// select the master file
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
			// the master file name switched, prompt the user to save the current file
			
		}

		UpdateData( VARS_TO_CONTROLS );
	}	
}

BOOL CSpitDlg::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt ) {
	
	// zDelta is neg toward the user, pos away from
	
	if( m_GameloopParams.bGameloopRunning ) {
		
		// make sure that pt is in the render window
		CRect Rect1;
		m_ctrlViewFrame.GetWindowRect( &Rect1 );
		if( Rect1.PtInRect( pt ) ) {
			_nDeltaWheel += zDelta;
		}		
	}
	
	return CDialog::OnMouseWheel( nFlags, zDelta, pt );
}

void CSpitDlg::OnMouseMove( UINT nFlags, CPoint point ) {
	
	if( _bMouseCaptured ) {
		s32 nX, nY;

		nX = point.x - _nLastMouseX;
		nY = point.y - _nLastMouseY;

		_nDeltaMouseX = nX;
		_nDeltaMouseY = nY;
		_nLastMouseX = point.x;
		_nLastMouseY = point.y;
	}

	CDialog::OnMouseMove( nFlags, point );
}

void CSpitDlg::OnCaptureChanged( CWnd *pWnd ) {

	if( _bMouseCaptured && (pWnd != this) ) {
		// uncapture the mouse
		_bMouseCaptured = FALSE;
		ReleaseCapture();
	}
		
	CDialog::OnCaptureChanged( pWnd );
}

void CSpitDlg::OnLButtonUp( UINT nFlags, CPoint point ) {
		
	if( _bMouseCaptured ) {
		// uncapture the mouse
		_bMouseCaptured = FALSE;
		ReleaseCapture();
	}

	CDialog::OnLButtonUp( nFlags, point );
}

void CSpitDlg::OnLButtonDown(UINT nFlags, CPoint point) {
	
	if( m_GameloopParams.bGameloopRunning ) {
		// make sure that pt is in the render window
		CRect Rect1;
		CPoint ScreenPt( point );
		m_ctrlViewFrame.GetWindowRect( &Rect1 );
		ClientToScreen( &ScreenPt );
		if( Rect1.PtInRect( ScreenPt ) ) {
			SetFocus();
			SetCapture();
			_nLastMouseX = point.x;
			_nLastMouseY = point.y;
			_nDeltaMouseX = 0;
			_nDeltaMouseY = 0;
			_bMouseCaptured = TRUE;
		}
	}

	CDialog::OnLButtonDown( nFlags, point );
}

void CSpitDlg::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSpitDlg::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	
	CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CSpitDlg::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags ) {	

	CDialog::OnChar( nChar, nRepCnt, nFlags );
}

// only called from initdlg function
long CSpitDlg::OnLoadDefFile( WPARAM wParam, LPARAM lParam ) {
	CString sError;

	// lock the data
	_CriticalSection.Lock();

	// kill all current particles
	_nEmitFlags |= (_EMIT_FLAGS_KILL_ALL | _EMIT_FLAGS_SPAWN);
	
	BOOL bVersionChanged;
	BOOL bReadOK = CDefFileIO::Read( m_sCurrentFile, &m_ParticleData, bVersionChanged );

	if( !bReadOK ) {
		_CriticalSection.Unlock();

		_SetToDefaults( &m_ParticleData );
		InitRollupDialogs();

		// couldn't load the file, warn the user
		sError.Format( "Could not load\n%s,\ncheck the file path.", m_sCurrentFile );
		MessageBox( sError, _T("SPIT Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);

		m_sCurrentFile.Empty();

		SetWindowText( _WINDOW_TITLE );

		return 0;
	}
		
	// init the rollups
	InitRollupDialogs();

	if( bVersionChanged ) {
		// tell the user that the data was converted
		sError.Format( "The following file\n%s,\nwas an older version and was converted to the newest version.", m_sCurrentFile );
		MessageBox( sError, _T("SPIT Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);

		m_bDataNeedsSaving = TRUE;
	} else {
		// we just loaded the file, it won't need saving
		m_bDataNeedsSaving = FALSE;
	}
	m_bDataHasBeenUpdated = TRUE;

	_CriticalSection.Unlock();

	// set the window text
	CString sTitle;
	sTitle.Format( "%s - [%s]", _WINDOW_TITLE, m_sCurrentFile );
	SetWindowText( sTitle );
	
	return 0;
}

long CSpitDlg::OnStartRenderWindow( WPARAM wParam, LPARAM lParam ) {
	m_GameloopParams.bGameloopRunning = FALSE;

	// We need to install the game loop after the dialog has finished initializing.
	if( !floop_InstallGameloop( _GameInit, _GameMain, _GameTerm, (void *)&m_GameloopParams, 60 ) ) {
		switch( m_GameloopParams.nErrorCode ) {
		case GAMELOOP_ERROR_COULD_NOT_LOAD_WORLD:
			MessageBox(_T("Could not load the world file, make sure it is compiled into the master file."), _T("SPIT Error"), MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
			break;
		case GAMELOOP_ERROR_COULD_NOT_START_LOOP:
			MessageBox(_T("Could not start the gameloop."), _T("SPIT Error"), MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
			break;
		case GAMELOOP_ERROR_COULD_NOT_CREATE_RENDER_WINDOW:
		case GAMELOOP_ERROR_COULD_NOT_GET_A_VIEWPORT:
		case GAMELOOP_ERROR_COULD_NOT_ALLOCATE_PARTICLE_MEMORY:
			MessageBox(_T("General gameloop init error."), _T("SPIT Error"), MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
			break;
		}
		PostMessage( WM_QUIT );
		return 0;
	}

	// Wait until GameInit has completed its operations.
	while( !m_GameloopParams.bGameloopRunning ) {
		Sleep( 15 );// don't overload the system
	}

	return 0;
}

static BOOL _GameInit( void *pParameter ) {
	GameloopParams_t *pParm = (GameloopParams_t *)pParameter;
	
	if( !fvid_CreateWindow( &pParm->VidSettings ) ) {
		pParm->nErrorCode = GAMELOOP_ERROR_COULD_NOT_CREATE_RENDER_WINDOW;
		return FALSE;
	}

	fdx8vid_SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	fdx8vid_EnableCursor( TRUE );	

	_pViewport = fviewport_Create();
	if( _pViewport == NULL ) {
		pParm->nErrorCode = GAMELOOP_ERROR_COULD_NOT_GET_A_VIEWPORT;	
		return FALSE;
	}

	// grab the coordinates of the screen render window so that we can create an exact viewport
	CRect Rect;
	pParm->pDlg->m_ctrlViewFrame.GetClientRect( &Rect );
	fviewport_InitPersp( _pViewport, FMATH_DEG2RAD( 30.0f ), 0.01f, 10000.0f, 0, 0, Rect.Width(), Rect.Height() );
	
	//////////////////////
	// load the world file
	_pWorldRes = (FVisData_t *)fresload_Load( FWORLD_RESTYPE, _WORLD_TO_LOAD );
	if( _pWorldRes == NULL ) {
		pParm->nErrorCode = GAMELOOP_ERROR_COULD_NOT_LOAD_WORLD;
		return FALSE;
	}

	///////////////////////////////////////////
	// allocate space for our particle def file
	_pFangParticleDef = (FParticleDef_t *)fres_AllocAndZero( sizeof( FParticleDef_t ) );
	if( !_pFangParticleDef ) {
		pParm->nErrorCode = GAMELOOP_ERROR_COULD_NOT_ALLOCATE_PARTICLE_MEMORY;
		return FALSE;
	}
	
	// init the fang particle def file that we will be editing
	fang_MemCopy( _pFangParticleDef, &pParm->pDlg->m_ParticleData, sizeof( FParticleDef_t ) );
	_hParticleDef = fparticle_LoadDefFileFromMem( _pFangParticleDef );
		
	_EmitterPos.Set( 0.0f, _FLOOR_Y + pParm->pDlg->m_fEmitY, 30.0f );
	_LookAtPoint = _EmitterPos;
	_EmitterDir = CFVec3::m_UnitAxisY;
	_fEmitterIntensity = 1.0f;

	/////////////////////
	// set the mouse vars
	_bMouseCaptured = FALSE;
	_bRightButtonDown = FALSE;
	_nLastMouseX = 0;
	_nLastMouseY = 0;
	_nDeltaMouseX = 0;
	_nDeltaMouseY = 0;
	_nDeltaWheel = 0;

	fperf_Reset();
	_fFrameSecs = 0.0f;
	_hBoxFilterFrame = fboxfilter_Create_f32( 45 );

	// turn off memory bars
	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;
	FVis_bDrawCellsOnly = FALSE;
	FVis_bDrawVisibility = FALSE;

	floop_SetTargetFramesPerSec( 60.0f );
	frenderer_DrawBound_Enable( FALSE );

	// grab a frame for loading and releasing textures
	_ResFrame = fres_GetFrame();
	_sLastTextureLoaded.Empty();
	_pTexDef = NULL;
	_pTexInst = NULL;

	// Success...
	pParm->bGameloopRunning = TRUE;

	return TRUE;
}

static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter ) {
	GameloopParams_t *pParm = (GameloopParams_t *)pParameter;	

	CFLightGroupMgr::ReleaseAllLights();

	fvid_DestroyWindow();
	pParm->bGameloopRunning = FALSE;
}

static BOOL _GameMain( BOOL bExitRequest, void *pParameter ) {
	GameloopParams_t *pParm = (GameloopParams_t *)pParameter;
	CFTimer Timer;
	BOOL bHaveFocus, bKeyDown, bDrawAid, bLookAt, bDrawLightBS, bDrawPartBS;
	f32 fDist2Emitter;
	CFVec3 TempVec, TempVec2;
	FParticleKeyFrame_t *pKeyFrame;
	u32 nNumParticles, nNumSprites;
	u32 nNumRealLights=0, nNumVLights=0;
	
	if( bExitRequest || fvid_HasUserClosedWindow() ) {
		return FALSE;
	}

	{
		// see if we have new data to update
		_CriticalSection.Lock();
		
		// copy the pos, dir, and intensity
		_EmitterPos.y = (_FLOOR_Y + pParm->pDlg->m_fEmitY);
		_EmitterDir = pParm->pDlg->m_EmitDir;
		_fEmitterIntensity = pParm->pDlg->m_fEmitIntensity;

		if( _nEmitFlags & _EMIT_FLAGS_KILL_ALL ) {
			_nEmitFlags &= ~_EMIT_FLAGS_KILL_ALL;
			fparticle_KillAllEmitters();
			_hEmitter = FPARTICLE_INVALID_HANDLE;
		}

		if( pParm->pDlg->m_bDataHasBeenUpdated ) {
			// there is new data to be copied to the fang memory
			pParm->pDlg->m_bDataHasBeenUpdated = FALSE;

			const FParticleDef_t *pSrc = &pParm->pDlg->m_ParticleData;

			// the loaded texture and the new texture name don't match release our frame
			if( _sLastTextureLoaded.CompareNoCase( pSrc->szTextureName ) != 0 ) {
				//fdelete _pTexInst;
				_pTexInst = NULL;

				fres_ReleaseFrame( _ResFrame );
				
				_pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pSrc->szTextureName );
				if( _pTexDef ) {
					_sLastTextureLoaded = pSrc->szTextureName;

					_pTexInst = fnew CFTexInst;
					if( _pTexInst ) {
						_pTexInst->SetTexDef( _pTexDef );
					} else {
						fres_ReleaseFrame( _ResFrame );
						_pTexDef = NULL;
					}
				} else {
					_sLastTextureLoaded.Empty();
				}				
			}

			// copy the entire particle def struct to our fang memory
			FLink_t Link;
			FLinkRoot_t EmitterList;
			u32 nIndex;
			BOOL bResetParticles;

			Link = _pFangParticleDef->Link;
			EmitterList = _pFangParticleDef->EmitterList;
			nIndex = _pFangParticleDef->nIndexInList;

			bResetParticles = ( (_pFangParticleDef->nFlags & FPARTICLE_DEF_FLAGS_BUBBLE_MOTION) != (pSrc->nFlags & FPARTICLE_DEF_FLAGS_BUBBLE_MOTION) );

			fang_MemCopy( _pFangParticleDef, pSrc, sizeof( FParticleDef_t ) );
			if( (_pFangParticleDef->nFlags & FPARTICLE_DEF_FLAGS_TEXTURED) && 
				_pTexInst && 
				_pTexDef ) {
				// textured
				if( _pFangParticleDef->pTexDef != _pTexDef ) {
					_pFangParticleDef->pTexDef = _pTexDef;
				}
			} else {
				// not textured
				_pFangParticleDef->pTexDef = NULL;
			}
			_pFangParticleDef->Link = Link;
			_pFangParticleDef->EmitterList = EmitterList;
			_pFangParticleDef->nIndexInList = nIndex;

			// reset the particle group flags
			fparticle_ResetEmitterFlags( _pFangParticleDef, bResetParticles );
		}

		if( _nEmitFlags & _EMIT_FLAGS_LIGHT_SETTINGS_CHANGED ) {
			// the lighting settings changed, kill all particle lights, they will be back next frame
			fparticle_StopAllEmitterLights( _pFangParticleDef );
			_nEmitFlags &= ~_EMIT_FLAGS_LIGHT_SETTINGS_CHANGED;
		}
		if( _nEmitFlags & _EMIT_FLAGS_STOP_ALL ) {
			_nEmitFlags &= (~_EMIT_FLAGS_STOP_ALL);
			fparticle_StopAllEmitters();
			_hEmitter = FPARTICLE_INVALID_HANDLE;
		}
		if( _nEmitFlags & _EMIT_FLAGS_SPAWN ) {
			_nEmitFlags &= (~_EMIT_FLAGS_SPAWN);
			
			_hEmitter = fparticle_SpawnToolEmitter( _hParticleDef, 
													&_EmitterPos,
													&_EmitterDir,
													&_fEmitterIntensity,
													NULL );			
		}
		// see if we should pause or unpause the emitters
		if( _nEmitFlags & _EMIT_FLAGS_FREEZE ) {
			// freeze all of the particles
			fparticle_PauseAllEmitter( TRUE );
			_nEmitFlags &= ~(_EMIT_FLAGS_FREEZE | _EMIT_FLAGS_UN_FREEZE);
		} else if( _nEmitFlags & _EMIT_FLAGS_UN_FREEZE ) {
			// unfreeze all of the particles
			fparticle_PauseAllEmitter( FALSE );
			_nEmitFlags &= ~(_EMIT_FLAGS_FREEZE | _EMIT_FLAGS_UN_FREEZE);
		}
		if( _nEmitFlags & _EMIT_FLAGS_SLOW_MO ) {
			FLoop_fPreviousLoopSecs *= (1.0f/15.0f);
			FLoop_fPreviousLoopOOSecs = 1.0f/FLoop_fPreviousLoopSecs;
		}

		bDrawAid = _nEmitFlags & _EMIT_FLAGS_AID;

		bLookAt = _nEmitFlags & _EMIT_FLAGS_LOOKAT;

		//frenderer_DrawBound_Enable( (_nEmitFlags & _EMIT_FLAGS_BOUNDS) );
		bDrawPartBS = _nEmitFlags & _EMIT_FLAGS_BOUNDS;

		bDrawLightBS = _nEmitFlags & _EMIT_FLAGS_LIGHT_BS;
		
		_CriticalSection.Unlock();
	}

	Timer.Reset();

	fvid_Begin();

	if( _nEmitFlags & _EMIT_FLAGS_AMBIENT ) {
		fworld_Ambient_Set( &_AmbientColorRGB, 1.0f );
	} else {
		fworld_Ambient_Set( &_AmbientColorRGB, 0.0f );
	}
	
	fviewport_SetActive( _pViewport );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.55f, 0.25f, 0.55f, 1.0f, 0 );

	bHaveFocus = TRUE;
	
	if( _pWorldRes ) {
		
		/////////////////////
		// Do our view's work
		f32 fXlatSpeed = (_CAM_XLAT_SPEED * 60.0f) * FLoop_fRealPreviousLoopSecs;
		FMATH_CLAMPMIN( fXlatSpeed, (_CAM_XLAT_SPEED * 0.20f) );

		f32 fSpinSpeed = (_CAM_SPIN_SPEED * 60.0f) * FLoop_fRealPreviousLoopSecs;
		FMATH_CLAMPMIN( fSpinSpeed, (_CAM_SPIN_SPEED * 0.20f) );

		if( _bRightButtonDown ) {
			fXlatSpeed *= 0.5f;
			fSpinSpeed *= 0.5f;
		}

		// move the emitter
		BOOL bControlKeyDown = FALSE;
		if( _bMouseCaptured ) {
			bControlKeyDown = bHaveFocus && (GetAsyncKeyState( VK_CONTROL ) < 0);
			if( bControlKeyDown ) {	
				// use the mouse to move the emitter in xz space
				_EmitterPos.x += ( (f32)_nDeltaMouseX * 0.25f * fXlatSpeed );
				_EmitterPos.z += ( (f32)_nDeltaMouseY * -0.25f * fXlatSpeed );
				
				// clear out the mouse controls
				_nDeltaWheel = 0;
				_nDeltaMouseX = 0;
				_nDeltaMouseY = 0;	
			}
		}

		if( bLookAt ) {
			_LookAtModeCameraWork( bHaveFocus, fXlatSpeed, fSpinSpeed, bControlKeyDown );
		} else {
			_FreeModeCameraWork( bHaveFocus, fXlatSpeed, fSpinSpeed, bControlKeyDown );
		}

		// see if we should reset the camera position
		bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_HOME ) < 0);
		if( bKeyDown ) {
			// reset camera position
			_fCamHeading = 0.0f;
			_fCamPitch = 0.0f;
			_CamPos.Zero();
		}

		///////////////////
		// setup the camera
		if( bLookAt ) {
			// calculate a new lookat point
#if 0
			if( !fparticle_GetFirstEmitterBSpherePos( _pFangParticleDef, TempVec ) ) {
				_LookAtPoint = _EmitterPos;
			} else {
				TempVec2 = _LookAtPoint;
				_LookAtPoint.ReceiveLerpOf( 0.005f, TempVec2, TempVec ); 
			}
#else
			_LookAtPoint = _EmitterPos;
			_LookAtPoint.y += 1.0f;
#endif
			_XfmCam.BuildLookat( _CamPos, _LookAtPoint );
		} else {
			_XfmCam.BuildInvRotYXZ_XlatFromPoint( _fCamHeading, _fCamPitch, 0.0f, _CamPos.x, _CamPos.y, _CamPos.z );
			_LookAtPoint = _EmitterPos;
		}
		_XfmCam.InitStackWithView();

		//////////////////////////////
		// do the particle system work
		fparticle_Work();

		CFLightGroupMgr::Work();

		/////////////////
		// draw the world
		fworld_EnableFog( TRUE );
		fvis_FrameBegin();
		fvis_Draw( NULL, TRUE );
		fvis_FrameEnd();

        if( bDrawAid || bDrawLightBS || bDrawPartBS ) {
			frenderer_Push( FRENDERER_DRAW, NULL );
			fdraw_Depth_EnableWriting( FALSE );
			fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
			fdraw_SetTexture( NULL );
			fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
			fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

			pKeyFrame = (pParm->pDlg->m_nCurrentEditableIntensity == _LO) ? &_pFangParticleDef->Min : &_pFangParticleDef->Max;
			
			f32 fRadius;
			CFVec3 RotVec;

			if( bDrawAid ) {
				f32 fLen = (pKeyFrame->VelocityZ.x * (1.0f/10.0f));
				TempVec = _EmitterPos;
				TempVec += ( _EmitterDir * fLen );
				CFColorRGBA Color( 1.0f, 1.0f, 1.0f, 1.0f );					
				fdraw_SolidLine( &TempVec, &_EmitterPos, &Color );

				RotVec.Set( 0.0f, 0.0f, fLen );
				RotVec.RotateY( pKeyFrame->InitDirAngle.x );
				fRadius = fmath_Sin( pKeyFrame->InitDirAngle.x ) * fLen;

				fdraw_FacetedCircle( &_EmitterDir, &TempVec, fRadius * 1.5f, &Color );

				fLen += (pKeyFrame->VelocityZ.y * (1.0f/10.0f));
				TempVec2 = TempVec;
				TempVec2 += ( _EmitterDir * (pKeyFrame->VelocityZ.y * (1.0f/10.0f)) );
				Color.Set( 0.0f, 0.0f, 1.0f, 1.0f );
				fdraw_SolidLine( &TempVec, &TempVec2, &Color );

				RotVec.Set( 0.0f, 0.0f, fLen );
				RotVec.RotateY( (pKeyFrame->InitDirAngle.x + pKeyFrame->InitDirAngle.y) );
				fRadius = fmath_Sin( (pKeyFrame->InitDirAngle.x + pKeyFrame->InitDirAngle.y) ) * fLen;	

				fdraw_FacetedCircle( &_EmitterDir, &TempVec2, fRadius * 1.5f, &Color );
			}

			if( bDrawLightBS ) {
				nNumRealLights = CFLightGroupMgr::DrawAllLightBoundingSpheres( FColor_MotifGreen, FColor_MotifBlue, nNumVLights );				
			}

			if( bDrawPartBS ) {
				fparticle_DrawEmitterBoundingSpheres( _pFangParticleDef );
			}

			frenderer_Pop();
		}
		if( !bDrawLightBS ) {
			CFLightGroupMgr::GetCounts( nNumRealLights, nNumVLights );
		}
		
		if( (FVid_nFrameCounter & 0xF) == 0xF ) {
			// calculate how far from the camera we are
			TempVec = _CamPos;
			TempVec -= _EmitterPos;
			fDist2Emitter = TempVec.Mag();

			f32 fDistanceToBoundingSphereCenter = -1.0f;
			f32 fBSRadius = 0.0f;
			BOOL bDrawEmulated = FALSE;
			if( _hEmitter != FPARTICLE_INVALID_HANDLE ) {
				const CFSphere *pSphere = fparticle_GetBoundingSpherePtr( _hEmitter );
				if( !pSphere ) {
					_hEmitter = FPARTICLE_INVALID_HANDLE;	
				} else {
					TempVec = _CamPos;
					TempVec -= pSphere->m_Pos;
					fDistanceToBoundingSphereCenter = TempVec.Mag();
					fBSRadius = pSphere->m_fRadius;

					f32 fEmulationDist = FMATH_FPOT( pParm->pDlg->m_fEmitIntensity, _pFangParticleDef->Min.fEmulationDist, _pFangParticleDef->Max.fEmulationDist );
					if( fEmulationDist > 0.0f && 
						fDistanceToBoundingSphereCenter < fEmulationDist ) {
						bDrawEmulated = TRUE;
					}
				}
			}

			// get the current particle counts
			fparticle_GetCounts( nNumParticles, nNumSprites );

			//////////////////////
			// print some text out
			pParm->pDlg->m_sStats.Format( "FPS: %.1f\n"
										  //"Pitch: %.1f, Heading: %.1f\n"
										  "Cam Pos: %.1f, %.1f, %.1f\n"
										  "Cam To Emitter Dist: %.1f Feet\n"
										  "Cam To BS Dist: %.1f (%.1f Radius)\n"
										  "Num Particles: %d, Num Sprites: %d\n"
										  "Num Lights: %d, Num VLights: %d\n"
										  "Draw SW Emulated: %s",
										  1.0f / _fFrameSecs,
										  //FMATH_RAD2DEG( _fCamPitch ),
										  //FMATH_RAD2DEG( _fCamHeading ),
										  _CamPos.x, _CamPos.y, _CamPos.z,
										  fDist2Emitter,
										  fDistanceToBoundingSphereCenter, fBSRadius,
										  nNumParticles, nNumSprites,
										  nNumRealLights, nNumVLights,
										  bDrawEmulated ? "YES" : "NO" );
			pParm->pDlg->PostMessage( _UPDATE_CONTROLS );	
		}
	}

	fvid_End();
	if( !fvid_Swap() ) {
		return FALSE;
	}

	_fFrameSecs = Timer.SampleSeconds( TRUE );
	fboxfilter_Add_f32( _hBoxFilterFrame, _fFrameSecs );
	fboxfilter_Get_f32( _hBoxFilterFrame, NULL, &_fFrameSecs, NULL, NULL );

	// give windows some time to deal with things
	Sleep( 0 );

	return TRUE;
}

void CSpitDlg::OnRButtonUp(UINT nFlags, CPoint point) {
	
	_bRightButtonDown = FALSE;
	
	CDialog::OnRButtonUp(nFlags, point);
}

void CSpitDlg::OnRButtonDown(UINT nFlags, CPoint point) {
	
	_bRightButtonDown = TRUE;
	
	CDialog::OnRButtonDown(nFlags, point);
}

void CSpitDlg::OnRadioMax() {
	
	UpdateData( CONTROLS_TO_VARS );

	m_ctrlCopyIntensity.SetWindowText( "Copy Low  -->  Hi" );
	m_ctrlCopyCurrent.SetWindowText( "Copy Current --> Hi" );

	m_ctrlEmitIntensity.SetPos( 100 ); 
	_CriticalSection.Lock();
	m_fEmitIntensity = 1.0f;
	_CriticalSection.Unlock();

	// init the rollups
	InitRollupDialogs();
}

void CSpitDlg::OnRadioMin() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	m_ctrlCopyIntensity.SetWindowText( "Copy Hi  -->  Low" );
	m_ctrlCopyCurrent.SetWindowText( "Copy Current --> Low" );

	m_ctrlEmitIntensity.SetPos( 0 ); 
	_CriticalSection.Lock();
	m_fEmitIntensity = 0.0f;
	_CriticalSection.Unlock();

	// init the rollups
	InitRollupDialogs();
}

void CSpitDlg::OnButtonCopySettings() {
	
	UpdateData( CONTROLS_TO_VARS );

	// grab the data lock
	_CriticalSection.Lock();

	// copy the data
	if( m_nCurrentEditableIntensity == _LO ) {
		m_ParticleData.Min = m_ParticleData.Max;
	} else {
		m_ParticleData.Max = m_ParticleData.Min;
	}

	// init the rollups
	InitRollupDialogs();

	m_bDataNeedsSaving = TRUE;

	// unlock the data
	_CriticalSection.Unlock();
}

void CSpitDlg::OnBnClickedButtonCopyCurrentSettings(){
	
	UpdateData( CONTROLS_TO_VARS );

	// grab the data lock
	_CriticalSection.Lock();

	// copy the data
	FParticleKeyFrame_t KeyFrame;
	fparticle_CalculateKeyFrame( &KeyFrame, _pFangParticleDef, m_fEmitIntensity );

	if( m_nCurrentEditableIntensity == _LO ) {
		m_ParticleData.Min = KeyFrame;
	} else {
		m_ParticleData.Max = KeyFrame;
	}

	// init the rollups
	InitRollupDialogs();

	m_bDataNeedsSaving = TRUE;

	// unlock the data
	_CriticalSection.Unlock();
}

void CSpitDlg::SaveData( BOOL bPrompt ) {

	if( !m_bDataNeedsSaving ) {
		return;
	}

	if( bPrompt ) {
		CString sMsg;
		if( m_sCurrentFile.IsEmpty() ) {
			sMsg.Format( "The data in Untitled has changed.\n\nDo you want to save the changes?" );
		} else {
			sMsg.Format( "The data in %s has changed.\n\nDo you want to save the changes?", m_sCurrentFile );
		}
		if( MessageBox( sMsg, _T("SPIT"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON1 ) == IDNO ) {
			return;
		}

		// yes was answered, see if we need a filename
		if( m_sCurrentFile.IsEmpty() ) {
			CFileDialog dlg( FALSE,
							 ".fpr",
							 "*.fpr",
							 OFN_EXTENSIONDIFFERENT | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
							 "Fang Particle Resource Files (*.fpr)|*.fpr||",
							 this );
			dlg.m_ofn.lpstrTitle = "Save As";
			if( dlg.DoModal() == IDOK ) {
				// make sure that the new name is ok
				if( dlg.GetFileTitle().GetLength() > 11 ) {
					MessageBox( _T("Valid names must be 11 or fewer characters.\nPlease select a new name."), 
								_T("SPIT"),
								MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
					return;
				}
				// make sure that the user picked the correct extension
				if( dlg.GetFileExt().CompareNoCase( "fpr" ) != 0 ) {
					MessageBox( _T("All names must have an extension of '.fpr'.\nPlease select a new filename."), 
								_T("SPIT"),
								MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
					return;
				}
				
				// save the new file name
				m_sCurrentFile = dlg.GetPathName();

				// set the window text
				CString sTitle;
				sTitle.Format( "%s - [%s]", _WINDOW_TITLE, m_sCurrentFile );
				SetWindowText( sTitle );
			} else {
				return;
			}
		}
	}

	if( m_sCurrentFile.IsEmpty() ) {
		return;
	}

	// grab the data lock
	_CriticalSection.Lock();
	
	// ok, save the file and then mark it as saved
	BOOL bWriteOK = CDefFileIO::Write( m_sCurrentFile, &m_ParticleData );

	if( bWriteOK ) {
		m_bDataNeedsSaving = FALSE;
	}

	// unlock the data
	_CriticalSection.Unlock();

	// warn the user if the write didn't work
	if( !bWriteOK ) {
		CString sError;
		sError.Format( "Could not write\n%s\ncheck the file path and make sure the file is not read only.", m_sCurrentFile );
		MessageBox( sError, _T("SPIT Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
	}
}

void CSpitDlg::InitRollupDialogs() {

	// grab the data lock
	_CriticalSection.Lock();

	_bInitingRollups = TRUE;

	// create structs from the data
	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	// init the sprite dialog
	CSpriteDlg_Data_t SpriteDlgData;
	SpriteDlgData.fNumSprites = pKeyFrame->fNumSprites;
	SpriteDlgData.fMaxVel2 = pKeyFrame->fMaxVel2;
	SpriteDlgData.fAlphaStepPerSprite = pKeyFrame->fAlphaStepPerSprite;
	SpriteDlgData.fSizeStepPerSprite = pKeyFrame->fSizeStepPerSprite;
	SpriteDlgData.fPosStepPerSprite = pKeyFrame->fPosStepPerSprite;	
	m_ctrlSprite.SetData( &SpriteDlgData );

	// init the velocity dialog
	CVelocityDlg_Data_t VelocityDlgData;
	VelocityDlgData.InitDirAngle = pKeyFrame->InitDirAngle;
	VelocityDlgData.VelocityZ = pKeyFrame->VelocityZ;
	m_ctrlVelocity.SetData( &VelocityDlgData );

	// init the scale dialog
	CScaleDlg_Data_t ScaleDlgData;
	ScaleDlgData.FinalScale = pKeyFrame->FinalScale;
	ScaleDlgData.InitScale = pKeyFrame->InitScale;
	m_ctrlScale.SetData( &ScaleDlgData );

	// init the pos dialog
	CPositionDlg_Data_t PositionDlgData;
	PositionDlgData.fRandomInitPosRadius = pKeyFrame->fRandomInitPosRadius;
	PositionDlgData.JitterOffset = pKeyFrame->JitterOffset;
	m_ctrlPosition.SetData( &PositionDlgData );
	
	// init the num particles dialog
	CNumParticlesDlg_Data_t NumParticlesDlgData;
	NumParticlesDlgData.NumPerBurst = pKeyFrame->NumPerBurst;
	NumParticlesDlgData.SecsBetweenBursts = pKeyFrame->SecsBetweenBursts;
	NumParticlesDlgData.fSecsToEmit = pKeyFrame->fSecsToEmit;
	NumParticlesDlgData.EmitPause = pKeyFrame->EmitPause;
	m_ctrlNumParticles.SetData( &NumParticlesDlgData );

	// init the lod dialog
	CLodDlg_Data_t LodDlgData;
	LodDlgData.fCullDist = pKeyFrame->fCullDist;
	LodDlgData.fStartSkipDrawDist = pKeyFrame->fStartSkipDrawDist;
	LodDlgData.fEmulationDist = pKeyFrame->fEmulationDist;
	m_ctrlLod.SetData( &LodDlgData );

	// init the light dialog
	CLightDlg_Data_t LightDlgData;
	LightDlgData.StartLightRGB = pKeyFrame->StartLightRGB;
	LightDlgData.fStartLightIntensity = pKeyFrame->fStartLightIntensity;
	LightDlgData.fStartLightRadiusMultiplier = pKeyFrame->fStartLightRadiusMultiplier;
	LightDlgData.EndLightRGB = pKeyFrame->EndLightRGB;
	LightDlgData.fEndLightIntensity = pKeyFrame->fEndLightIntensity;
	LightDlgData.fEndLightRadiusMultiplier = pKeyFrame->fEndLightRadiusMultiplier;
	LightDlgData.fMaxLightRadius = pKeyFrame->fMaxLightRadius;
	LightDlgData.fSndUnitVolume = pKeyFrame->fSndUnitVolume;
	LightDlgData.fSndPitchMultiplier = pKeyFrame->fSndPitchMultiplier;
	m_ctrlLights.SetData( &LightDlgData );

	// init the lifespan dialog
	CLifeSpanDlg_Data_t LifeSpanDlgData;
	LifeSpanDlgData.OOLifeSecs = pKeyFrame->OOLifeSecs;
	m_ctrlLifeSpan.SetData( &LifeSpanDlgData );

	// init the global dialog
	CGlobalDlg_Data_t GlobalDlgData;
	GlobalDlgData.nFlags = m_ParticleData.nFlags;
	fclib_strncpy( GlobalDlgData.szTextureName, m_ParticleData.szTextureName, (FDATA_TEXNAME_LEN+1) );  
	fclib_strncpy( GlobalDlgData.szCoronaTextureName, m_ParticleData.szCoronaTextureName, (FDATA_TEXNAME_LEN+1) );  
	fclib_strncpy( GlobalDlgData.szSoundFxName, m_ParticleData.szSoundFxName, (FDATA_TEXNAME_LEN+1) );  
	GlobalDlgData.AnimAlphaInfo = m_ParticleData.AnimAlphaInfo;
	GlobalDlgData.AnimScaleInfo = m_ParticleData.AnimScaleInfo;
	GlobalDlgData.AnimColorInfo = m_ParticleData.AnimColorInfo;
	GlobalDlgData.nLightMotifIndex = m_ParticleData.nLightMotifIndex;
	GlobalDlgData.fSecsBetweenLightSamples = m_ParticleData.fSecsBetweenLightSamples;
	GlobalDlgData.SampledLightScale = m_ParticleData.SampledLightScale;
	GlobalDlgData.fCoronaScale = m_ParticleData.fCoronaScale;
	GlobalDlgData.nFlags2 = m_ParticleData.nFlags2;
	m_ctrlGlobal.SetData( &GlobalDlgData );

	// enable the multiple sprite dialog
	if( GlobalDlgData.nFlags & FPARTICLE_DEF_FLAGS_MULTIPLE_SPRITES ) {
		m_ctrlRollup.EnablePage( m_nSpriteRollupIndex, TRUE );
	} else {
		m_ctrlRollup.EnablePage( m_nSpriteRollupIndex, FALSE );
	}
	// enable the light dialog
	if( GlobalDlgData.nFlags & (FPARTICLE_DEF_FLAGS_EMIT_LIGHT | FPARTICLE_DEF_FLAGS_PLAY_SOUND) ) {
		m_ctrlRollup.EnablePage( m_nLightRollupIndex, TRUE );
	} else {
		m_ctrlRollup.EnablePage( m_nLightRollupIndex, FALSE );
	}

	// init the forces dialog
	CForcesDlg_Data_t ForcesDlgData;
	ForcesDlgData.fDragMultiplierPerSec = pKeyFrame->fDragMultiplierPerSec;
	ForcesDlgData.fGravityPerSec = pKeyFrame->fGravityPerSec;
	ForcesDlgData.MaxBubbleXYZ = pKeyFrame->MaxBubbleXYZ;
	ForcesDlgData.fBubbleUpdateChance = pKeyFrame->fBubbleUpdateChance;
	ForcesDlgData.fBubbleDragPerSec = pKeyFrame->fBubbleDragPerSec;
	m_ctrlForces.SetData( &ForcesDlgData );

	// init the emissive dialog
	CEmissiveDlg_Data_t EmissiveDlgData;
	EmissiveDlgData.EmissiveIntensity = pKeyFrame->EmissiveIntensity;
	m_ctrlEmissive.SetData( &EmissiveDlgData );

	// init the color dialog
	CColorDlg_Data_t ColorDlgData;
	ColorDlgData.InitRed = pKeyFrame->InitRed;
	ColorDlgData.InitGreen = pKeyFrame->InitGreen;
	ColorDlgData.InitBlue = pKeyFrame->InitBlue;	
	ColorDlgData.FinalRed = pKeyFrame->FinalRed;
	ColorDlgData.FinalGreen = pKeyFrame->FinalGreen;
	ColorDlgData.FinalBlue = pKeyFrame->FinalBlue;
	ColorDlgData.InitAlpha = pKeyFrame->InitAlpha;	
	ColorDlgData.FinalAlpha = pKeyFrame->FinalAlpha;
	m_ctrlColors.SetData( &ColorDlgData );

	m_bDataHasBeenUpdated = TRUE;

	_bInitingRollups = FALSE;

	// unlock the data
	_CriticalSection.Unlock();
}

void CSpitDlg::OnButtonSetToDefaults() {
	
	_CriticalSection.Lock();

	if( m_bDataNeedsSaving ) {
		// prompt the user before moving on
		if( MessageBox( _T("There have been changes made to the current\nfile.  Are you sure you want to over write them\nwith the default settings?"), 
						_T("SPIT"),
						MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1 ) == IDNO ) {
			_CriticalSection.Unlock();
			return;
		}
	}

	// kill all current particles
	_nEmitFlags |= (_EMIT_FLAGS_KILL_ALL | _EMIT_FLAGS_SPAWN);

	// load the defaults
	_SetToDefaults( &m_ParticleData );

	// mark that this file needs to be saved
	m_bDataNeedsSaving = TRUE;

	// init the rollups
	InitRollupDialogs();

	_CriticalSection.Unlock();	
}

void CSpitDlg::DataChanged( CSpriteDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->fNumSprites != pData->fNumSprites ||
		pKeyFrame->fMaxVel2 != pData->fMaxVel2 ||
		pKeyFrame->fAlphaStepPerSprite != pData->fAlphaStepPerSprite ||
		pKeyFrame->fSizeStepPerSprite != pData->fSizeStepPerSprite ||
		pKeyFrame->fPosStepPerSprite != pData->fPosStepPerSprite ) {

		// something changed, record that the data has changed and copy the new data	
		pKeyFrame->fNumSprites = pData->fNumSprites;
		pKeyFrame->fMaxVel2 = pData->fMaxVel2;
		pKeyFrame->fAlphaStepPerSprite = pData->fAlphaStepPerSprite;
		pKeyFrame->fSizeStepPerSprite = pData->fSizeStepPerSprite;
		pKeyFrame->fPosStepPerSprite = pData->fPosStepPerSprite;	

		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CVelocityDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->InitDirAngle != pData->InitDirAngle ||
		pKeyFrame->VelocityZ != pData->VelocityZ ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->InitDirAngle = pData->InitDirAngle;
		pKeyFrame->VelocityZ = pData->VelocityZ;

		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CScaleDlg_Data_t *pData ) {
	
	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->FinalScale != pData->FinalScale ||
		pKeyFrame->InitScale != pData->InitScale ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->FinalScale = pData->FinalScale;
		pKeyFrame->InitScale = pData->InitScale;
		
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CPositionDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->fRandomInitPosRadius != pData->fRandomInitPosRadius ||
		pKeyFrame->JitterOffset != pData->JitterOffset ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->fRandomInitPosRadius = pData->fRandomInitPosRadius;
		pKeyFrame->JitterOffset = pData->JitterOffset;
		
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();	
}

void CSpitDlg::DataChanged( CNumParticlesDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->NumPerBurst != pData->NumPerBurst ||
		pKeyFrame->SecsBetweenBursts != pData->SecsBetweenBursts ||
		pKeyFrame->fSecsToEmit != pData->fSecsToEmit ||
		pKeyFrame->EmitPause != pData->EmitPause ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->NumPerBurst = pData->NumPerBurst;
		pKeyFrame->SecsBetweenBursts = pData->SecsBetweenBursts;
		pKeyFrame->fSecsToEmit = pData->fSecsToEmit;
		pKeyFrame->EmitPause = pData->EmitPause;
	
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CLodDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->fCullDist != pData->fCullDist ||
		pKeyFrame->fStartSkipDrawDist != pData->fStartSkipDrawDist ||
		pKeyFrame->fEmulationDist != pData->fEmulationDist ) {		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->fCullDist = pData->fCullDist;
		pKeyFrame->fStartSkipDrawDist = pData->fStartSkipDrawDist;
		pKeyFrame->fEmulationDist = pData->fEmulationDist;
			
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CLightDlg_Data_t *pData ) {
	
	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	BOOL bLightRelatedFieldsChanged;

	// test the lighting settings for change
	if( pKeyFrame->StartLightRGB != pData->StartLightRGB ||
		pKeyFrame->fStartLightIntensity != pData->fStartLightIntensity ||
		pKeyFrame->fStartLightRadiusMultiplier != pData->fStartLightRadiusMultiplier ||		
		pKeyFrame->EndLightRGB != pData->EndLightRGB ||
		pKeyFrame->fEndLightIntensity != pData->fEndLightIntensity ||
		pKeyFrame->fEndLightRadiusMultiplier != pData->fEndLightRadiusMultiplier ||
		pKeyFrame->fMaxLightRadius != pData->fMaxLightRadius ) {
		// at least 1 lighting setting changed
		bLightRelatedFieldsChanged = TRUE;
		_nEmitFlags |= _EMIT_FLAGS_LIGHT_SETTINGS_CHANGED;
	} else {
		bLightRelatedFieldsChanged = FALSE;
	}

	// test the other features for change
	if( bLightRelatedFieldsChanged ||
		pKeyFrame->fSndPitchMultiplier != pData->fSndPitchMultiplier ||
		pKeyFrame->fSndUnitVolume != pData->fSndUnitVolume ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->StartLightRGB = pData->StartLightRGB;
		pKeyFrame->fStartLightIntensity = pData->fStartLightIntensity;
		pKeyFrame->fStartLightRadiusMultiplier = pData->fStartLightRadiusMultiplier;
		pKeyFrame->EndLightRGB = pData->EndLightRGB;
		pKeyFrame->fEndLightIntensity = pData->fEndLightIntensity;
		pKeyFrame->fEndLightRadiusMultiplier = pData->fEndLightRadiusMultiplier;
		pKeyFrame->fSndPitchMultiplier = pData->fSndPitchMultiplier;
		pKeyFrame->fSndUnitVolume = pData->fSndUnitVolume;
		pKeyFrame->fMaxLightRadius = pData->fMaxLightRadius;
			
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CLifeSpanDlg_Data_t *pData ) {
	
	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->OOLifeSecs != pData->OOLifeSecs ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->OOLifeSecs = pData->OOLifeSecs;
					
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();	
}

void CSpitDlg::DataChanged( CGlobalDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	BOOL bLightRelatedFieldsChanged;

	// test the lighting settings for change
	if( (m_ParticleData.nFlags & FPARTICLE_DEF_FLAGS_EMIT_LIGHT) != (pData->nFlags & FPARTICLE_DEF_FLAGS_EMIT_LIGHT) ||
		(m_ParticleData.nFlags2 & FPARTICLE_DEF_FLAG2_LIGHTING_SETTINGS) != (pData->nFlags2 & FPARTICLE_DEF_FLAG2_LIGHTING_SETTINGS) ||
		m_ParticleData.nLightMotifIndex != pData->nLightMotifIndex ||
		fclib_stricmp( m_ParticleData.szCoronaTextureName, pData->szCoronaTextureName ) != 0 ||
		m_ParticleData.fCoronaScale != pData->fCoronaScale ) {
		// at least 1 lighting setting changed
		bLightRelatedFieldsChanged = TRUE;
		_nEmitFlags |= _EMIT_FLAGS_LIGHT_SETTINGS_CHANGED;
	} else {
		bLightRelatedFieldsChanged = FALSE;
	}

	// test the other features for change
	if( bLightRelatedFieldsChanged ||
		m_ParticleData.nFlags != pData->nFlags ||
		m_ParticleData.nFlags2 != pData->nFlags2 ||
		m_ParticleData.fSecsBetweenLightSamples != pData->fSecsBetweenLightSamples ||
		m_ParticleData.SampledLightScale != pData->SampledLightScale ||
		fclib_stricmp( m_ParticleData.szSoundFxName, pData->szSoundFxName ) != 0 ||
		fclib_stricmp( m_ParticleData.szTextureName, pData->szTextureName ) != 0 ||
		fparticle_AreAnimFunctionsEqual( &m_ParticleData.AnimAlphaInfo, &pData->AnimAlphaInfo ) == FALSE ||
		fparticle_AreAnimFunctionsEqual( &m_ParticleData.AnimScaleInfo, &pData->AnimScaleInfo ) == FALSE ||
		fparticle_AreAnimFunctionsEqual( &m_ParticleData.AnimColorInfo, &pData->AnimColorInfo ) == FALSE ) {
		
		// something changed, record that the data has changed and copy the new data
		m_ParticleData.nFlags = pData->nFlags;
		m_ParticleData.nFlags2 = pData->nFlags2;
		m_ParticleData.fCoronaScale = pData->fCoronaScale;
        m_ParticleData.nLightMotifIndex = pData->nLightMotifIndex;
		m_ParticleData.fSecsBetweenLightSamples = pData->fSecsBetweenLightSamples;
		m_ParticleData.SampledLightScale = pData->SampledLightScale;
		fclib_strncpy( m_ParticleData.szTextureName, pData->szTextureName, (FDATA_TEXNAME_LEN+1) );
		fclib_strncpy( m_ParticleData.szSoundFxName, pData->szSoundFxName, (FDATA_TEXNAME_LEN+1) );
		fclib_strncpy( m_ParticleData.szCoronaTextureName, pData->szCoronaTextureName, (FDATA_TEXNAME_LEN+1) );
		m_ParticleData.AnimAlphaInfo = pData->AnimAlphaInfo;
		m_ParticleData.AnimColorInfo = pData->AnimColorInfo;
		m_ParticleData.AnimScaleInfo = pData->AnimScaleInfo;

		// enable the multiple sprite dialog
		if( m_ParticleData.nFlags & FPARTICLE_DEF_FLAGS_MULTIPLE_SPRITES ) {
			m_ctrlRollup.EnablePage( m_nSpriteRollupIndex, TRUE );
		} else {
			m_ctrlRollup.EnablePage( m_nSpriteRollupIndex, FALSE );
		}
		// enable the light dialog
		if( m_ParticleData.nFlags & (FPARTICLE_DEF_FLAGS_EMIT_LIGHT | FPARTICLE_DEF_FLAGS_PLAY_SOUND) ) {
			m_ctrlRollup.EnablePage( m_nLightRollupIndex, TRUE );
		} else {
			m_ctrlRollup.EnablePage( m_nLightRollupIndex, FALSE );
		}
			
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();	
}

void CSpitDlg::DataChanged( CForcesDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->fDragMultiplierPerSec != pData->fDragMultiplierPerSec ||
		pKeyFrame->fGravityPerSec != pData->fGravityPerSec ||
		pKeyFrame->MaxBubbleXYZ != pData->MaxBubbleXYZ ||
		pKeyFrame->fBubbleUpdateChance != pData->fBubbleUpdateChance ||
		pKeyFrame->fBubbleDragPerSec != pData->fBubbleDragPerSec ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->fDragMultiplierPerSec = pData->fDragMultiplierPerSec;
		pKeyFrame->fGravityPerSec = pData->fGravityPerSec;
		pKeyFrame->MaxBubbleXYZ = pData->MaxBubbleXYZ;
		pKeyFrame->fBubbleUpdateChance = pData->fBubbleUpdateChance;
		pKeyFrame->fBubbleDragPerSec = pData->fBubbleDragPerSec;
			
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CEmissiveDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->EmissiveIntensity != pData->EmissiveIntensity ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->EmissiveIntensity = pData->EmissiveIntensity;
					
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

void CSpitDlg::DataChanged( CColorDlg_Data_t *pData ) {

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	FParticleKeyFrame_t *pKeyFrame;
	if( m_nCurrentEditableIntensity == _LO ) {
		pKeyFrame = &m_ParticleData.Min;
	} else {
		pKeyFrame = &m_ParticleData.Max;
	}

	if( pKeyFrame->InitRed != pData->InitRed ||
		pKeyFrame->InitGreen != pData->InitGreen ||
		pKeyFrame->InitBlue != pData->InitBlue ||
		pKeyFrame->FinalRed != pData->FinalRed ||
		pKeyFrame->FinalGreen != pData->FinalGreen ||
		pKeyFrame->FinalBlue != pData->FinalBlue ||
		pKeyFrame->InitAlpha != pData->InitAlpha ||
		pKeyFrame->FinalAlpha != pData->FinalAlpha ) {
		
		// something changed, record that the data has changed and copy the new data
		pKeyFrame->InitRed = pData->InitRed;
		pKeyFrame->InitGreen = pData->InitGreen;
		pKeyFrame->InitBlue = pData->InitBlue;
		pKeyFrame->FinalRed = pData->FinalRed;
		pKeyFrame->FinalGreen = pData->FinalGreen;
		pKeyFrame->FinalBlue = pData->FinalBlue;
		pKeyFrame->InitAlpha = pData->InitAlpha;
		pKeyFrame->FinalAlpha = pData->FinalAlpha;
			
		m_bDataHasBeenUpdated = TRUE;
		m_bDataNeedsSaving = TRUE;
	}

	_CriticalSection.Unlock();
}

// this is so that the gameloop thread can send _UPDATE_CONTROLS messages
// to the parent thread and have the controls updated
long CSpitDlg::UpdateControls( WPARAM wParam, LPARAM lParam ) {
	UpdateData( VARS_TO_CONTROLS );	
	return 0;
}

void CSpitDlg::OnButtonEmitShot() {
	
	_CriticalSection.Lock();
	_nEmitFlags |= _EMIT_FLAGS_SPAWN;
	_CriticalSection.Unlock();
}

void CSpitDlg::OnKillfocusEditEmitIntensity() {
	f32 fIntensity = (f32)m_ctrlEmitIntensity.GetPos(); 
	fIntensity *= (1.0f/100.0f);

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}
	
	m_fEmitIntensity = fIntensity;

	_CriticalSection.Unlock();
}

void CSpitDlg::OnKillfocusEditEmitRot() {
	f32 fRot = (f32)m_ctrlEmitRot.GetPos();
	fRot = FMATH_DEG2RAD( fRot );

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}

	m_EmitDir = CFVec3::m_UnitAxisY;
	m_EmitDir.RotateX( fRot );

	_CriticalSection.Unlock();
}

void CSpitDlg::OnKillfocusEditEmitY() {
	f32 fEmitY = (f32)m_ctrlEmitY.GetPos();
	fEmitY *= (1.0f/100.0f);
	fEmitY *= _EMITTER_DELTA_Y;

	_CriticalSection.Lock();

	if( _bInitingRollups ) {
		// we are initing the rollups, we already have the latest data
		_CriticalSection.Unlock();
		return;
	}
	
	m_fEmitY = fEmitY;

	_CriticalSection.Unlock();
}

LONG CSpitDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlEmitRot.GetDlgCtrlID() ) {
		OnKillfocusEditEmitRot();	
	} else if( wParam == m_ctrlEmitY.GetDlgCtrlID() ) {
		OnKillfocusEditEmitY();
	} else if( wParam == m_ctrlEmitIntensity.GetDlgCtrlID() ) {
		OnKillfocusEditEmitIntensity();
	}

	return 0;
}

void CSpitDlg::OnCheckPause() {
	
	BOOL bPause = ((CButton *)GetDlgItem( IDC_CHECK_PAUSE ))->GetCheck();

	_CriticalSection.Lock();
	if( bPause ) {
		_nEmitFlags |= _EMIT_FLAGS_FREEZE;
	} else {
		_nEmitFlags |= _EMIT_FLAGS_UN_FREEZE;
	}
	_CriticalSection.Unlock();
	
}

void CSpitDlg::OnCheckDrawBoundingSphere() {
	
	BOOL bDraw = ((CButton *)GetDlgItem( IDC_CHECK_DRAW_BOUNDING_SPHERE ))->GetCheck();

	_CriticalSection.Lock();
	if( bDraw ) {
		_nEmitFlags |= _EMIT_FLAGS_BOUNDS;
	} else {
		_nEmitFlags &= ~_EMIT_FLAGS_BOUNDS;
	}
	_CriticalSection.Unlock();
}

void CSpitDlg::OnCheckSlowMo() {
	
	BOOL bSlowMo = ((CButton *)GetDlgItem( IDC_CHECK_SLOW_MO ))->GetCheck();

	_CriticalSection.Lock();
	if( bSlowMo ) {
		_nEmitFlags |= _EMIT_FLAGS_SLOW_MO;
	} else {
		_nEmitFlags &= ~_EMIT_FLAGS_SLOW_MO;
	}
	_CriticalSection.Unlock();
}

void CSpitDlg::OnCheckDrawAid() {
	
	BOOL bDrawAid = ((CButton *)GetDlgItem( IDC_CHECK_DRAW_AID ))->GetCheck();

	_CriticalSection.Lock();
	if( bDrawAid ) {
		_nEmitFlags |= _EMIT_FLAGS_AID;
	} else {
		_nEmitFlags &= ~_EMIT_FLAGS_AID;
	}
	_CriticalSection.Unlock();
}

void CSpitDlg::OnChangeEditEmitY() {
	OnKillfocusEditEmitY();		
}

void CSpitDlg::OnChangeEditEmitIntensity() {
	OnKillfocusEditEmitIntensity();	
}

void CSpitDlg::OnChangeEditEmitRot() {
	OnKillfocusEditEmitRot();	
}

static void _FreeModeCameraWork( BOOL bHaveFocus, f32 fXlatSpeed, f32 fSpinSpeed, BOOL bControlKeyDown ) {
	BOOL bKeyDown;

	if( _bMouseCaptured && !bControlKeyDown ) {
		/////////////////////////
		// process keyboard input
		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'W' ) < 0) || (GetAsyncKeyState( VK_UP ) < 0) );
		if( bKeyDown ) {
			// Forward...
			_CamPos += _XfmCam.m_MtxR.m_vFront.v3 * fXlatSpeed;
		}
		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'S' ) < 0) || (GetAsyncKeyState( VK_DOWN ) < 0) );
		if( bKeyDown ) {
			// Backwards...
			_CamPos -= _XfmCam.m_MtxR.m_vFront.v3 * fXlatSpeed;
		}
		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'A' ) < 0) || (GetAsyncKeyState( VK_LEFT ) < 0) );
		if( bKeyDown ) {
			// Strafe Left...
			_CamPos -= _XfmCam.m_MtxR.m_vRight.v3 * fXlatSpeed;
		}
		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'D' ) < 0) || (GetAsyncKeyState( VK_RIGHT ) < 0) );
		if( bKeyDown ) {
			// Strafe Right...
			_CamPos += _XfmCam.m_MtxR.m_vRight.v3 * fXlatSpeed;
		}
		bKeyDown = bHaveFocus && (GetAsyncKeyState( 'R' ) < 0);
		if( bKeyDown ) {
			// Vertical Up...
			_CamPos.y += fXlatSpeed;
		}
		bKeyDown = bHaveFocus && (GetAsyncKeyState( 'F' ) < 0);
		if( bKeyDown ) {
			// Vertical Down...
			_CamPos.y -= fXlatSpeed;
		}
		bKeyDown = bHaveFocus && (GetAsyncKeyState( 'Q' ) < 0);
		if( bKeyDown ) {
			// Spin Left...
			_fCamHeading -= fSpinSpeed;
		}
		bKeyDown = bHaveFocus && (GetAsyncKeyState( 'E' ) < 0);
		if( bKeyDown ) {
			// Spin Right...
			_fCamHeading += fSpinSpeed;
		}
		//////////////////////
		// process mouse input
		_fCamHeading += ( (f32)_nDeltaMouseX * 0.4f * fSpinSpeed );
		_nDeltaMouseX = 0;
		_fCamPitch += ( (f32)_nDeltaMouseY * 0.4f * fSpinSpeed );
		_nDeltaMouseY = 0;
		FMATH_CLAMP( _fCamPitch, -FMATH_HALF_PI, FMATH_HALF_PI );			
	}
	//////////////////////
	// process mouse wheel
	if( _nDeltaWheel ) {
		_CamPos.y += ( (f32)_nDeltaWheel * 0.075f * fXlatSpeed );
		_nDeltaWheel = 0;
	}

	// renormalize the heading 
	while( _fCamHeading < 0.0f ) {
		_fCamHeading += FMATH_2PI;
	}
	while( _fCamHeading > FMATH_2PI ) {
		_fCamHeading -= FMATH_2PI;
	}
}

static void _LookAtModeCameraWork( BOOL bHaveFocus, f32 fXlatSpeed, f32 fSpinSpeed, BOOL bControlKeyDown ) {
	BOOL bKeyDown;
	CFVec3 TempVec;
	f32 fDist2Camera, fOpposite;
	BOOL bReDistance = FALSE;

	TempVec = _CamPos;
	TempVec -= _EmitterPos;
	fDist2Camera = TempVec.Mag();

	if( _bMouseCaptured && !bControlKeyDown ) {
		
		/////////////////////////
		// process keyboard input
		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'W' ) < 0) || (GetAsyncKeyState( VK_UP ) < 0) );
		if( bKeyDown ) {
			// Forward...
			_CamPos += _XfmCam.m_MtxR.m_vFront.v3 * fXlatSpeed;

			if( (fDist2Camera - fXlatSpeed) <= 1.0f ) {
				// make sure that we are at least 1 foot from the target
				TempVec.Unitize();
				_CamPos = _EmitterPos;
				_CamPos += TempVec;

				fDist2Camera = 1.0f;
			} else {
				fDist2Camera -= fXlatSpeed;
			}
		}
		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'S' ) < 0) || (GetAsyncKeyState( VK_DOWN ) < 0) );
		if( bKeyDown ) {
			// Backwards...
			_CamPos -= _XfmCam.m_MtxR.m_vFront.v3 * fXlatSpeed;
			
			fDist2Camera += fXlatSpeed;
		}

		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'A' ) < 0) || (GetAsyncKeyState( VK_LEFT ) < 0) );
		if( bKeyDown ) {
			// Strafe Left...
			_CamPos -= _XfmCam.m_MtxR.m_vRight.v3 * fXlatSpeed;
			bReDistance = TRUE;
		}
		bKeyDown = bHaveFocus && ( (GetAsyncKeyState( 'D' ) < 0) || (GetAsyncKeyState( VK_RIGHT ) < 0) );
		if( bKeyDown ) {
			// Strafe Right...
			_CamPos += _XfmCam.m_MtxR.m_vRight.v3 * fXlatSpeed;
			bReDistance = TRUE;
		}
		
		bKeyDown = bHaveFocus && (GetAsyncKeyState( 'R' ) < 0);
		if( bKeyDown ) {
			// Vertical Up...
			_CamPos.y += fXlatSpeed;
			bReDistance = TRUE;
		}
		bKeyDown = bHaveFocus && (GetAsyncKeyState( 'F' ) < 0);
		if( bKeyDown ) {
			// Vertical Down...
			_CamPos.y -= fXlatSpeed;
			bReDistance = TRUE;
		}
		//////////////////////
		// process mouse input
		if( _nDeltaMouseX ) {
			_CamPos += _XfmCam.m_MtxR.m_vRight.v3 * ( (f32)_nDeltaMouseX * -0.4f * fXlatSpeed );
			_nDeltaMouseX = 0;
			bReDistance = TRUE;
		}
		if( _nDeltaMouseY ) {
			_CamPos.y += ( (f32)_nDeltaMouseY * 0.4f * fXlatSpeed );
			_nDeltaMouseY = 0;
			bReDistance = TRUE;
		}
	}
	//////////////////////
	// process mouse wheel
	if( _nDeltaWheel ) {
		_CamPos.y += ( (f32)_nDeltaWheel * 0.075f * fXlatSpeed );
		_nDeltaWheel = 0;
		bReDistance = TRUE;
	}

	// maintain the same distance to the target
	if( bReDistance ) {
		TempVec = _CamPos;
		TempVec -= _EmitterPos;
		TempVec.Unitize();
		TempVec *= fDist2Camera;
		TempVec += _EmitterPos;
		_CamPos = TempVec;
	}

	// recalculate the heading 
	TempVec = _EmitterPos - _CamPos;
	TempVec.y = 0.0f;
	TempVec *= 200.0f;// make sure that there is something to unitize
	TempVec.Unitize();
	fDist2Camera = TempVec.Dot( CFVec3::m_UnitAxisZ );

	if( fDist2Camera >= 0.999f ) {
		_fCamHeading = 0.0f;
	} else if( fDist2Camera <= -0.999f ) {
		_fCamHeading = FMATH_PI;
	} else {
		fOpposite = fmath_Sqrt( (1.0f * 1.0f) - (fDist2Camera * fDist2Camera) );
		_fCamHeading = fmath_Atan( fOpposite, fDist2Camera );

		// determine what side we are on
		if( TempVec.Dot( CFVec3::m_UnitAxisX ) < 0.0f ) {
			_fCamHeading = FMATH_2PI - _fCamHeading;
		}
	}
}

void CSpitDlg::OnCheckLookAtCamera() {
	
	BOOL bLookAt = ((CButton *)GetDlgItem( IDC_CHECK_LOOK_AT_CAMERA ))->GetCheck();

	_CriticalSection.Lock();
	if( bLookAt ) {
		_nEmitFlags |= _EMIT_FLAGS_LOOKAT;
	} else {
		_nEmitFlags &= ~_EMIT_FLAGS_LOOKAT;
	}
	_CriticalSection.Unlock();	
}

void CSpitDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
	
	if( nIDCtl == m_ctrlAmbientLight.GetDlgCtrlID() ) {
		// draw only a color in the button
		CDC dc;
		dc.Attach( lpDrawItemStruct->hDC );     //Get device context object
		CRect rt;
		rt = lpDrawItemStruct->rcItem;        //Get button rect
		dc.FillSolidRect( rt, RGB( (u8)(_AmbientColorRGB.fRed * 255.0f), (u8)(_AmbientColorRGB.fGreen * 255.0f), (u8)(_AmbientColorRGB.fBlue * 255.0f) ) );
		dc.DrawEdge( rt, EDGE_SUNKEN, BF_RECT );    // Draw a sunken face
		dc.Detach();
		return;
	}
	
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CSpitDlg::OnCheckAmbientLightOn() {
	BOOL bAmbient = ((CButton *)GetDlgItem( IDC_CHECK_AMBIENT_LIGHT_ON ))->GetCheck();

	_CriticalSection.Lock();
	if( bAmbient ) {
		_nEmitFlags |= _EMIT_FLAGS_AMBIENT;
	} else {
		_nEmitFlags &= ~_EMIT_FLAGS_AMBIENT;
	}
	_CriticalSection.Unlock();	
}

void CSpitDlg::OnButtonPickAmbientLight() {
	u8 nR = (u8)(_AmbientColorRGB.fRed * 255.0f);
	u8 nG = (u8)(_AmbientColorRGB.fGreen * 255.0f);
	u8 nB = (u8)(_AmbientColorRGB.fBlue * 255.0f);

	COLORREF Color = RGB( nR, nG, nB );

	CColorDialog ColorDlg( Color, CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT, this );
	if( ColorDlg.DoModal() == IDOK ) {
		Color = ColorDlg.GetColor();

		_CriticalSection.Lock();
		
		_AmbientColorRGB.Set( GetRValue( Color ) * (1.0f/255.0f), 
							  GetGValue( Color ) * (1.0f/255.0f),
							  GetBValue( Color ) * (1.0f/255.0f) );
		_CriticalSection.Unlock();	
		
		m_ctrlAmbientLight.Invalidate();		
	}	
}

void CSpitDlg::OnButtonStop() {
	
	_CriticalSection.Lock();
	_nEmitFlags |= _EMIT_FLAGS_STOP_ALL;
	_CriticalSection.Unlock();		
}

static void _SetToDefaults( FParticleDef_t *pDef ) {
	FASSERT( 1148 == sizeof( FParticleDef_t ) );

	// Generated by BreakPoint Software's Hex Workshop v3.11
	//   http://www.hexworkshop.com
	//
	// Source: default.fpr
	//   Time: 4/5/2003 7:35 PM
	// Offset: 0 / 0x00000000
	// Length: 1148 / 0x0000047C
	static const u8 _nRawData[1148] = {
		0x07, 0x00, 0x00, 0x00, 0x58, 0x81, 0x00, 0x00, 0x00, 0x00, 0x7A, 0x43, 0x00, 0x00, 0x48, 0x42, 
		0x0A, 0xD7, 0x23, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0xA0, 0x40, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x20, 0x41, 
		0x00, 0x00, 0x40, 0x40, 0xCD, 0xCC, 0xCC, 0x3D, 0xCD, 0xCC, 0xCC, 0x3D, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCC, 0xCC, 0x3D, 
		0x00, 0x00, 0x00, 0x00, 0x9A, 0x99, 0x19, 0x3E, 0x00, 0x00, 0x00, 0x40, 0xCD, 0xCC, 0x4C, 0x3F, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0xC8, 0x42, 0x8F, 0xC2, 0x75, 0x3F, 0x48, 0xE1, 0x7A, 0x3F, 
		0x0A, 0xD7, 0x23, 0x3C, 0xCD, 0xCC, 0x4C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCC, 0xCC, 0x3D, 
		0x66, 0x66, 0x66, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0xAD, 0xAC, 0x2C, 0x3E, 
		0xD6, 0xD4, 0x54, 0x3F, 0xF4, 0xF2, 0x72, 0x3F, 0xD2, 0xD0, 0x50, 0x3D, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x00, 0x00, 0xF4, 0xF2, 0xF2, 0x3E, 0xF4, 0xF2, 0xF2, 0xBE, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0xE8, 0xE6, 0x66, 0x3F, 0xCA, 0xC8, 0xC8, 0x3D, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x2E, 0xBA, 0xE8, 0x3F, 0x00, 0x00, 0xA0, 0x3F, 0x1E, 0x85, 0x6B, 0x3F, 
		0x0A, 0xD7, 0xA3, 0x3D, 0x35, 0xFA, 0x8E, 0x3C, 0x00, 0x00, 0x00, 0x00, 0xE5, 0xF2, 0x7F, 0x3F, 
		0x2C, 0xF6, 0xFF, 0x3F, 0x6F, 0x12, 0x83, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7A, 0x43, 0x00, 0x00, 0x52, 0x42, 0x0A, 0xD7, 0x23, 0x3C, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0xA0, 0x40, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x41, 0x00, 0x00, 0xC0, 0x40, 
		0xD9, 0x89, 0x9D, 0x3D, 0x89, 0x88, 0x88, 0x3D, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCD, 0xCC, 0xCC, 0x3D, 0x00, 0x00, 0x00, 0x00, 
		0x9A, 0x99, 0x19, 0x3E, 0x00, 0x00, 0x00, 0x40, 0xCD, 0xCC, 0x4C, 0x3F, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0xC8, 0x42, 0x8F, 0xC2, 0x75, 0x3D, 0x48, 0xE1, 0x7A, 0x3F, 0x0A, 0xD7, 0x23, 0x3C, 
		0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x41, 0x00, 0x00, 0x40, 0x40, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x81, 0x80, 0x80, 0xBB, 
		0xFC, 0xFA, 0x7A, 0x3F, 0xA5, 0xA4, 0x24, 0xBF, 0xF6, 0xF4, 0x74, 0x3F, 0xF1, 0xEF, 0x6F, 0xBF, 
		0xE8, 0xE6, 0x66, 0x3F, 0xCA, 0xC8, 0xC8, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x2E, 0xBA, 0xE8, 0x3F, 0x00, 0x00, 0xA0, 0x3F, 0x5C, 0x8F, 0x02, 0x3F, 0x47, 0xE1, 0xFA, 0x3E, 
		0x00, 0x00, 0x00, 0x00, 0xDD, 0x35, 0xFA, 0x3D, 0x51, 0xDA, 0x6F, 0x41, 0xB3, 0x8C, 0x0B, 0x42, 
		0x17, 0xB7, 0xD1, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x74, 0x77, 0x64, 0x6D, 0x66, 0x6C, 0x61, 0x6D, 0x65, 0x30, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x02, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x02, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x02, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0xCD, 0xCC, 0xCC, 0x3D, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	};

	// copy the data into our struct
	fang_MemCopy( pDef, _nRawData, sizeof( FParticleDef_t ) );
}


void CSpitDlg::OnFileUpdateversion() {
	FParticleDef_t PartDef;
	BOOL bFileChanged, bReadOK, bWriteOK;
	u32 nCouldNotRead = 0, nCouldNotWrite = 0, nConverted = 0, nFilesProcessed = 0, nFilesNotNeedingConverting = 0;

	#define _NUM_FILE_SELECTION_BYTES	16384

	char _szFileBuf[_NUM_FILE_SELECTION_BYTES];

	UpdateData( CONTROLS_TO_VARS );

	// multi file selector	
	ZeroMemory( _szFileBuf, _NUM_FILE_SELECTION_BYTES );
	strncpy( _szFileBuf, (const char *)m_sCurrentFile, m_sCurrentFile.GetLength() );

	CFileDialog dlg( TRUE, NULL, m_sCurrentFile,
					 OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
					 "Fpr Files (*.fpr)|*.fpr||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Files To Convert";
	dlg.m_ofn.lpstrFile = _szFileBuf;
	dlg.m_ofn.nMaxFile = _NUM_FILE_SELECTION_BYTES;
	if( dlg.DoModal() == IDOK ) {
		CString sFilename = dlg.GetFileTitle();
		if( sFilename.IsEmpty() ) {
			CString sPathname = _szFileBuf;
			sPathname += "\\";
			char *pszFilename = &_szFileBuf[dlg.m_ofn.nFileOffset];
			CString sFullFilename;
	
			while( *pszFilename ) {
				sFilename = pszFilename;
				pszFilename += sFilename.GetLength() + 1;
				sFullFilename.Format( "%s%s", sPathname, sFilename );

				///////////////////
				// convert the file
				
				// open the file
				bReadOK = CDefFileIO::Read( sFullFilename, &PartDef, bFileChanged );

				nFilesProcessed++;

				if( bReadOK ) {
					if( bFileChanged ) {
						// try to write out the file
						bWriteOK = CDefFileIO::Write( sFullFilename, &PartDef );
						if( !bWriteOK ) {
							nCouldNotWrite++;
						} else {
							nConverted++;
						}
					} else {
						nFilesNotNeedingConverting++;
					}
				} else {
					nCouldNotRead++;
				}
			};
		} else {
			// only 1 file hase been selected
			sFilename = dlg.GetPathName();

			// open the file
			bReadOK = CDefFileIO::Read( sFilename, &PartDef, bFileChanged );
			nFilesProcessed++;

			if( bReadOK ) {
				if( bFileChanged ) {
					// try to write out the file
					bWriteOK = CDefFileIO::Write( sFilename, &PartDef );
					if( !bWriteOK ) {
						nCouldNotWrite++;
					} else {
						nConverted++;
					}
				} else {
					nFilesNotNeedingConverting++;
				}
			} else {
				nCouldNotRead++;
			}			
		}
		UpdateData( VARS_TO_CONTROLS );

		// display a message box with some stats
		CString sMsg;
		sMsg.Format( "Of the %d files processed:\n"
					 "\t%d did not need conversion\n"
					 "\t%d were converted OK\n"
					 "\t%d had problems being read\n"
					 "\t%d had problems being written\n",
					 nFilesProcessed,
					 nFilesNotNeedingConverting,
					 nConverted,
                     nCouldNotRead,
					 nCouldNotWrite );
		MessageBox( sMsg, _T("SPIT Conversion Info"), MB_ICONINFORMATION|MB_OK|MB_DEFBUTTON1);
	}
}

void CSpitDlg::OnBnClickedCheckDrawLightBS() {

	BOOL bDrawLightBS = ((CButton *)GetDlgItem( IDC_CHECK_DRAW_LIGHT_BS ))->GetCheck();

	_CriticalSection.Lock();
	if( bDrawLightBS ) {
		_nEmitFlags |= _EMIT_FLAGS_LIGHT_BS;
	} else {
		_nEmitFlags &= ~_EMIT_FLAGS_LIGHT_BS;
	}
	_CriticalSection.Unlock();	
}
