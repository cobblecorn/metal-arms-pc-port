//////////////////////////////////////////////////////////////////////////////////////
// pasmDlg.cpp - 
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
// 07/20/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "pasm.h"
#include "pasmDlg.h"
#include "fang.h"
#include "Settings.h"
#include "fmath.h"
#include "fversion.h"
#include "SettingsDlg.h"
#include "ConfigFile.h"
#include "ErrorLog.h"
#include "MasterFileCompile.h"
#include "CompileDlg.h"
#include "DeleteDlg.h"
#include "CShellFileOp.h"
#include "InterProcessData.h"
#include "ZipArchive.h"
#include "ProgressWnd.h"
#include "utils.h"
#include "fversion.h"
// !!Nate
#include "SyncDialog.h"
#include "stringinput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _MAX_TOOLTIP_WIDTH			175
#define _INITIAL_TOOLTIP_MSECS		250
#define _RESHOW_TOOLTIP_MSECS		25
#define _TOOLTIP_VISIBLE_MSECS		6000

#define _LOCK_FILENAME				"\\pasm_net.loc"
#define _DELETE_LIST_FILENAME		"\\dellist.txt"

#define _NO_LIBRARY_SUPPORT			TRUE 

// uses an undocumented windows call to bring windows to the foreground under win2k
//typedef void (WINAPI *PROCSWITCHTOTHISWINDOW)( HWND, BOOL );
//PROCSWITCHTOTHISWINDOW SwitchToThisWindow;

static bool _ZipCompressCallback( int , int nBytesSoFar, void *pData );

CString *CPasmDlg::m_pStaticLocalDir=NULL;
CString CPasmDlg::m_csPASMTempDirectory;

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
// CPasmDlg dialog

CPasmDlg::CPasmDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPasmDlg::IDD, pParent), m_MapLocalFiles( 50 ), m_MapLibFiles( 50 )
{
	//{{AFX_DATA_INIT(CPasmDlg)
	m_bDownsizeTextures = FALSE;
	m_bDisableStripping = FALSE;
	m_bDetailedLightingInfo = FALSE;
	m_bDontUseVIS = FALSE;
	m_bReusePerfectVISMatchOnly = FALSE;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hMenu = ::LoadMenu(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAIN_MENU));

	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD));

	m_bFangOK = FALSE;

	m_bMustRunConfig = TRUE;
	m_sXBMasterFileDir = _T("");
	m_sGCMasterFileDir = _T("");
#ifdef _MMI_TARGET_PS2
    m_sPS2MasterFileDir = _T("");
#endif
	m_sLogDir = _T("");
	m_sExtractDir = _T("");
	m_sConfigDir = _T("");
	m_sIgnoreExtensions = _T("");
	m_sConfigFilename = _T("");
	m_sMasterFilename = _T("");
	m_sLibDir = _T("");
	m_sLibLockDir = _T("");
	m_sLocalDir = _T("");
	// !!Nate
	m_sXClientFileDir = _T("");
	m_bAutoRunXClient = FALSE;
	m_nLibFileCount = 0;
	m_nLocalFileCount = 0;
	m_nMasterFileCount = 0;
	m_bXBoxFormat = FALSE;
	m_bPCFormat = FALSE;
	m_bGCFormat = FALSE;
#ifdef _MMI_TARGET_PS2
    m_bPS2Format = FALSE;
#endif
	m_bOutputDebugPics = FALSE;
	m_sCurrentMasterFilename = _T("");
	m_LocalFiles.RemoveAll();
	m_LibFiles.RemoveAll();
	m_bShowDifferences = FALSE;
	m_bHoldCompileStats = FALSE;
	m_bDisableLibrarySupport = FALSE;
	m_bHideNonCompliableFiles = FALSE;
	m_nLastSelectedFilesCount = 0xFFFFFFF6;// bogus number so that the cache works the first time thru
	m_bUserQuitNetLockWait = FALSE;
	//m_LastDeleteTime;
	m_DeleteListFileTime = CTime::GetCurrentTime();

	m_bMasterFileVersionsChecked = FALSE;

	m_nLightingQuality = 0;
	m_fMaxLMapSize = 2.0f;
	m_fSubSample = 0;

	AfxInitRichEdit();

	char szWindowsTempPath[512];
	GetTempPath( 512, szWindowsTempPath );
	m_csPASMTempDirectory =  szWindowsTempPath;
	m_csPASMTempDirectory += "\\PASMTEMP\\";
	CreateDirectory( m_csPASMTempDirectory, NULL ); // This will fail if the directory already exists

//	m_bFilterLightMaps = FALSE;

	// uses an undocumented windows call to bring windows to the foreground under win2k
//	HMODULE hUser32 = GetModuleHandle("user32");
//	SwitchToThisWindow = ( PROCSWITCHTOTHISWINDOW )GetProcAddress( hUser32, "SwitchToThisWindow" );
}

void CPasmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPasmDlg)
	DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFileList);
	DDX_Control(pDX, IDC_COMPILE, m_CompileButton);
	DDX_Control(pDX, IDC_SYNC_DOWN, m_SyncDownButton);
	DDX_Control(pDX, IDC_SYNC_UP, m_SyncUpButton);
	DDX_Control(pDX, IDC_SYNC_DOWN_AND_COMPILE, m_SyncDownAndCompileButton);
	DDX_Control(pDX, IDC_NEWER_THAN_LOCAL, m_NewerThanLocButton);
	DDX_Control(pDX, IDC_NEWER_THAN_LIB, m_NewerThanLibButton);
	DDX_Control(pDX, IDC_NEWER_THAN_MASTER, m_NewerThanMasterButton);
	DDX_Control(pDX, IDC_RESCAN, m_RescanButton);
	DDX_Control(pDX, IDC_SCAN_AND_COMPILE, m_ScanAndCompileButton);
	DDX_Check(pDX, IDC_AUTO_DOWNSIZE_TEXTURES, m_bDownsizeTextures);
	DDX_Check(pDX, IDC_DISABLE_STRIPPING, m_bDisableStripping );
	DDX_Check(pDX, IDC_DETAILED_LIGHTING_INFO, m_bDetailedLightingInfo );
	DDX_Check(pDX, IDC_DONT_USE_VIS, m_bDontUseVIS );
	DDX_Check(pDX, IDC_REUSE_PERFECT_MATCH_ONLY, m_bReusePerfectVISMatchOnly );
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasmDlg, CDialog)
	//{{AFX_MSG_MAP(CPasmDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(IDC_CONFIG, OnPressConfigButton)
	ON_BN_CLICKED(ID_PLATFORM_XBOX, OnXBoxFormat)
	ON_BN_CLICKED(ID_PLATFORM_PC, OnPCFormat)
	ON_BN_CLICKED(ID_PLATFORM_GC, OnGCFormat)
#ifdef _MMI_TARGET_PS2
	ON_BN_CLICKED(ID_PLATFORM_PS2, OnPS2Format)
#endif
	ON_BN_CLICKED(ID_OUTPUT_DEBUG_PICS, OnOutputDebugPics)
	ON_BN_CLICKED(ID_VIEW_SHOWDIFFERENCESONLY, OnShowDifferences)
	ON_BN_CLICKED(ID_VIEW_HIDE_NON_COMPILABLE_FILES, OnHideNoncompilableFiles)
	ON_BN_CLICKED(ID_VIEW_HOLD_COMPILE_STATS, OnHoldCompileStats)
	ON_BN_CLICKED(ID_DISABLE_LIB, OnDisableLibrarySupport)
	ON_BN_CLICKED(IDC_NEWER_THAN_MASTER, OnSelectNewerThanMaster)
	ON_BN_CLICKED(IDC_NEWER_THAN_LOCAL, OnSelectNewerThanLocal)
	ON_BN_CLICKED(IDC_NEWER_THAN_LIB, OnSelectNewerThanLib)
	ON_COMMAND(IDC_COMPILE, OnCompile)
	ON_COMMAND(IDC_FORCE_COMPILE, OnForcedCompile)
	ON_COMMAND(IDC_CLEAR_ERROR_LOG, OnClearErrorLog)
	ON_COMMAND(IDC_REBUILD_MASTER_FILE, OnRebuildMasterFile)
	ON_BN_CLICKED(IDC_RESCAN, OnRescan)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_COMMAND(IDC_OPTIMIZE_MASTERFILE, OnOptimizeMasterfile)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_LIST, OnDblclkFileList)
	ON_NOTIFY(NM_CLICK, IDC_FILE_LIST, OnClickFileList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_LIST, OnItemchangedFileList)
	ON_BN_CLICKED(IDC_SCAN_AND_COMPILE, OnScanAndCompile)
	ON_NOTIFY(NM_RCLICK, IDC_FILE_LIST, OnRclickFileList)
	ON_BN_CLICKED(IDC_SYNC_DOWN, OnSyncDownClick)
	ON_BN_CLICKED(IDC_SYNC_UP, OnSyncUpClick)
	ON_BN_CLICKED(IDC_SYNC_DOWN_AND_COMPILE, OnSyncDownAndCompile)
	ON_COMMAND(IDC_MIRROR_LIBRARY, OnMirrorLibrary)
	ON_COMMAND(IDC_SHOW_FILESTATS, OnClickDetailedFileStats)
	ON_COMMAND(ID_OPEN_ERRORLOG, OnClickOpenErrorLog)
	ON_COMMAND(IDC_DUMP_FILENAMES, OnDumpFilenames)
	ON_COMMAND(ID_TOOLS_CREATE_ZIP_FILE, OnToolsCreateZipFile)
	ON_MESSAGE(APP_MSG_CODES_PASM_CHECK_MASTER_FILE_VERSION, OnCheckMasterFileVersion)
	ON_MESSAGE(APP_MSG_CODES_PASM_RESCAN_AND_COMPILE_LOCAL, OnExternalAppRequestingRescanAndCompile)
	ON_COMMAND(IDC_COMPACT_MASTER_FILE, OnCompactMasterFile)
	ON_COMMAND(ID_TOOLS_GENERATE_FILE_USAGE_FILE, OnGenerateFileTypeUsageFile)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnNMCustomdrawSlider1)
	ON_EN_CHANGE(IDC_LMAP_MAXSIZE, OnEnChangeLmapMaxsize)
//	ON_BN_CLICKED(IDC_FILTER, OnBnClickedFilter)
	ON_EN_CHANGE(IDC_SUBSAMPLE, OnEnChangeSubsample)
	ON_COMMAND(ID_TOOLS_GENERATE_MASTER_FILE_LIST, OnToolsGenerateMasterFileList)
	ON_COMMAND(ID_TOOLS_CREATENEWMASTERFILEWITHONLYLOGGEDASSETS, OnToolsCreateNewMasterFileWithOnlyLoggedAssets)
	ON_COMMAND(ID_TOOLS_GENERATELISTOFUNUSEDASSETS, OnToolsGenerateListOfUnusedAssets)
	ON_COMMAND(59397, OnOptimizeInternationalMasterFile)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPasmDlg message handlers

BOOL CPasmDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//	_CrtSetBreakAlloc( 50450 );
//	_CrtSetBreakAlloc( 50451 );

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	//LightMap Param
	CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_SLIDER1 );
	pSlider->SetRange(0, 10);

	((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->SetPos(m_nLightingQuality);

	char szTemp[80];
	sprintf(szTemp, "%2.2f", m_fMaxLMapSize);
	((CEdit*)GetDlgItem( IDC_LMAP_MAXSIZE ))->SetWindowText(szTemp);

	sprintf(szTemp, "%2.2f", m_fSubSample);
	((CEdit*)GetDlgItem( IDC_SUBSAMPLE ))->SetWindowText(szTemp);

//	((CButton*)GetDlgItem( IDC_FILTER ))->SetCheck( m_bFilterLightMaps );
	//

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
	::SetProp( GetSafeHwnd(), Pasm_pszPropName, (HANDLE)1 );

	////////////////////////////
	// SET OUR SETTINGS FILENAME
	CSettings::SetSettingsFilename( "pasm.ini" ); 
	
	////////////////////////////////////////////////
	// GET THE DEFAULT VALUES FROM OUR SETTINGS FILE
	CSettings& Settings = CSettings::GetCurrent();
	if( Settings.GetMustRunConfig() ) {
		// make sure the default settings are written out to disk
		Settings.SaveCommonDataOutToFile();
	}

	m_bXBoxFormat = Settings.GetXBoxFormatOn();
	m_bGCFormat = Settings.GetGCFormatOn();
	m_bPCFormat = Settings.GetPCFormatOn();
#ifdef _MMI_TARGET_PS2
    m_bPS2Format = Settings.GetPS2FormatOn();
#endif
	m_bShowDifferences = Settings.GetShowDifferences();
	m_bHoldCompileStats = Settings.GetHoldCompileStats();
	m_bDisableLibrarySupport = Settings.GetDisableLibrarySupport();
#if _NO_LIBRARY_SUPPORT
	m_bDisableLibrarySupport = TRUE;
#endif
	m_bMustRunConfig = Settings.GetMustRunConfig();
	m_sXBMasterFileDir = Settings.GetXBMasterFileDir();
	m_sGCMasterFileDir = Settings.GetGCMasterFileDir();
#ifdef _MMI_TARGET_PS2
    m_sPS2MasterFileDir = Settings.GetPS2MasterFileDir();
#endif
	// !!Nate
	m_sXClientFileDir = Settings.GetXClientFileDir();
	m_bAutoRunXClient = Settings.GetXClientAutoRun();

	m_sLogDir = Settings.GetLogDir();
	m_sExtractDir = Settings.GetExtractDir();
	m_sConfigDir = Settings.GetConfigDir();
	m_sIgnoreExtensions = Settings.GetIgnoreExtString();
	m_sConfigFilename = Settings.GetConfigFilename();
	m_bHideNonCompliableFiles = FALSE;//Settings.GetHideNonCompliableFiles();
	
	::CheckMenuItem( m_hMenu, ID_VIEW_SHOWDIFFERENCESONLY, m_bShowDifferences ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( m_hMenu, ID_VIEW_HOLD_COMPILE_STATS, m_bHoldCompileStats ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( m_hMenu, ID_DISABLE_LIB, m_bDisableLibrarySupport ? MF_CHECKED : MF_UNCHECKED );
#if _NO_LIBRARY_SUPPORT
	::EnableMenuItem( m_hMenu, ID_DISABLE_LIB, MF_GRAYED );
#endif
	::CheckMenuItem( m_hMenu, ID_VIEW_HIDE_NON_COMPILABLE_FILES, m_bHideNonCompliableFiles ? MF_CHECKED : MF_UNCHECKED );

	m_bOutputDebugPics = FALSE;
	::CheckMenuItem( m_hMenu, ID_OUTPUT_DEBUG_PICS, m_bOutputDebugPics ? MF_CHECKED : MF_UNCHECKED );

	EnableLibraryButtons( !m_bDisableLibrarySupport );

	FixIgnoreExtString( m_sIgnoreExtensions );
	
	///////////////////////////////////////////////////////////////
	// SET THE FORMAT CHECKBOX (MAKING SURE THAT ONLY 1 IS CHECKED)	
#if FANG_WINGC
	// force gc config
	m_bGCFormat = TRUE;
	m_bXBoxFormat = FALSE;
	m_bPCFormat = FALSE;

	::CheckMenuItem( m_hMenu, ID_PLATFORM_XBOX, m_bXBoxFormat ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PC, m_bPCFormat ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( m_hMenu, ID_PLATFORM_GC, m_bGCFormat ? MF_CHECKED : MF_UNCHECKED );

	::EnableMenuItem( m_hMenu, ID_PLATFORM_XBOX, MF_GRAYED );
	::EnableMenuItem( m_hMenu, ID_PLATFORM_PC, MF_GRAYED );
#else
	// don't allow GC
	m_bGCFormat = FALSE;
	if( m_bXBoxFormat ) {
		m_bPCFormat = FALSE;			
	} else {
		// default to xbox
		m_bPCFormat = FALSE;
#ifndef _MMI_TARGET_PS2
		m_bXBoxFormat = TRUE;		
#else
        m_bPS2Format = TRUE;
        m_bXBoxFormat = FALSE;
#endif
	}
	::CheckMenuItem( m_hMenu, ID_PLATFORM_XBOX, m_bXBoxFormat ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PC, m_bPCFormat ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( m_hMenu, ID_PLATFORM_GC, m_bGCFormat ? MF_CHECKED : MF_UNCHECKED );
#ifdef _MMI_TARGET_PS2
    ::CheckMenuItem( m_hMenu, ID_PLATFORM_PS2, m_bPS2Format ? MF_CHECKED : MF_UNCHECKED );
#endif

	::EnableMenuItem( m_hMenu, ID_PLATFORM_GC, MF_GRAYED );
#endif
	
	
	// Set the window title
#if FANG_WINGC
	SetWindowText( "PASM - Asset File Compiler (GameCube Version) - (c) Swingin' Ape Studios, Inc." );
#else
	SetWindowText( "PASM - Asset File Compiler (DirectX Version) - (c) Swingin' Ape Studios, Inc." );
#endif

	UpdateData( VARS_TO_CONTROLS );

	///////////////////////
	// LOAD THE CONFIG FILE
	CConfigFile ConfigFile;
	CString sFilename;

	// try to load a config file
	if( !m_bMustRunConfig && 
		!m_sConfigDir.IsEmpty() &&
		!m_sConfigFilename.IsEmpty() ) {
		
		CSettingsDlg::CreateFilename( sFilename, m_sConfigDir, m_sConfigFilename );

		if( !ConfigFile.Read( sFilename ) ) {
			this->MessageBox( "Trouble reading config file.\nPlease enter new settings.", 
							  "PASM ERROR",
							  MB_OK | MB_ICONSTOP );
			m_bMustRunConfig = TRUE;
		} else {
			m_sMasterFilename = ConfigFile.m_sMasterFilename;
			m_sLibDir = ConfigFile.m_sLibDir;
			m_sLibLockDir = ConfigFile.m_sLibraryLockDir;
			m_sLocalDir = ConfigFile.m_sLocalDir;

            m_pStaticLocalDir = &m_sLocalDir;

			if( m_sLibLockDir.IsEmpty() ) {
				m_bMustRunConfig = TRUE;;
			}
		}
		UpdateData( VARS_TO_CONTROLS );
	}

	///////////////////////////////////////////////////////////////////
	// RUN THE SETTINGS DIALOG IF THIS IS THE FIRST TIME WE'VE RUN PASM
	if( m_bMustRunConfig ) {
		CSettingsDlg SettingsDlg;
				
		SettingsDlg.m_sXBMasterFileDir = m_sXBMasterFileDir;
		SettingsDlg.m_sGCMasterFileDir = m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
        SettingsDlg.m_sPS2MasterFileDir = m_sPS2MasterFileDir;
#endif
		SettingsDlg.m_sLogDir = m_sLogDir;
		SettingsDlg.m_sExtractDir = m_sExtractDir;
		SettingsDlg.m_sConfigDir = m_sConfigDir;
		SettingsDlg.m_sIgnoreExtensions = m_sIgnoreExtensions;
		SettingsDlg.m_sConfigFilename = m_sConfigFilename;
		SettingsDlg.m_sMasterFilename = m_sMasterFilename;
		SettingsDlg.m_sLibDir = m_sLibDir;
		SettingsDlg.m_sLibraryLockDir = m_sLibLockDir;
		SettingsDlg.m_sLocalDir = m_sLocalDir;
		// !!Nate
		SettingsDlg.m_sXClientDir = m_sXClientFileDir;
		SettingsDlg.m_bAutoRunXClient = m_bAutoRunXClient;

		while( m_bMustRunConfig ) {
			if( SettingsDlg.DoModal() == IDOK ) {
				// record the new values
				m_sXBMasterFileDir = SettingsDlg.m_sXBMasterFileDir;
				m_sGCMasterFileDir = SettingsDlg.m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
                m_sPS2MasterFileDir = SettingsDlg.m_sPS2MasterFileDir;
#endif
				m_sLogDir = SettingsDlg.m_sLogDir;
				m_sExtractDir = SettingsDlg.m_sExtractDir;
				m_sConfigDir = SettingsDlg.m_sConfigDir;
				m_sIgnoreExtensions = SettingsDlg.m_sIgnoreExtensions;
				FixIgnoreExtString( m_sIgnoreExtensions );
				m_sConfigFilename = SettingsDlg.m_sConfigFilename;
				m_sMasterFilename = SettingsDlg.m_sMasterFilename;
				m_sLibDir = SettingsDlg.m_sLibDir; 
				m_sLibLockDir = SettingsDlg.m_sLibraryLockDir;
				m_sLocalDir = SettingsDlg.m_sLocalDir;
				// !!Nate
				m_sXClientFileDir = SettingsDlg.m_sXClientDir;
				m_bAutoRunXClient = SettingsDlg.m_bAutoRunXClient;

				UpdateData( VARS_TO_CONTROLS );
				
				// make sure that we have valid vars
				if( m_sXBMasterFileDir.IsEmpty() ||
					m_sGCMasterFileDir.IsEmpty() ||
#ifdef _MMI_TARGET_PS2
                    m_sPS2MasterFileDir.IsEmpty() ||
#endif
					m_sLogDir.IsEmpty() ||
					//m_sExtractDir.IsEmpty() ||
					m_sConfigDir.IsEmpty() ||
					m_sConfigFilename.IsEmpty() ||
					m_sMasterFilename.IsEmpty() ||
					m_sLibDir.IsEmpty() ||
					m_sLibLockDir.IsEmpty() ||
					m_sLocalDir.IsEmpty() ||
					// !!Nate
					m_sXClientFileDir.IsEmpty()) {

					this->MessageBox( "You must enter all configuration\nsettings before you can use PASM.", 
									  "PASM Settings Failure",
									  MB_OK | MB_ICONSTOP );	
				} else {
					SaveSettingsOutToDisk( TRUE, FALSE );
					
					m_bMustRunConfig = FALSE;
				}				
			} else {
				this->MessageBox( "You must enter your configuration\nsettings before you can use PASM.", 
								  "PASM Settings Failure",
								  MB_OK | MB_ICONSTOP );	
			}
		}		
		UpdateData( VARS_TO_CONTROLS );
	}

	//////////////////////
	// SETUP THE ERROR LOG
	SetupLogFile();

	//////////////////////
	// SETUP THE LOCK FILE
	sFilename = m_sLibLockDir;
	sFilename += _LOCK_FILENAME;
	m_FileLock.SetFilename( sFilename );

	////////////////////////////////////////////////////////////////////////////////////////////
	// SET THE NAME OF THE CURRENT MASTER FILE (DEPENDS ON WHICH PLATFORM IS CURRENTLY SELECTED)
	SetupCurrentMasterFilename();

	///////////////////
	// ADDING STATUSBAR
	int nTotWide;		// total width of status bar
	CRect rect;
	this->GetWindowRect(&rect);
	rect.top = rect.bottom- 25;
    	
	BOOL bRvStatOk = m_StatusBar.Create( WS_CHILD | WS_VISIBLE, rect, this, IDC_MAIN_STATUSBAR );// | WS_BORDER
                             
    if( bRvStatOk == NULL ) {
		this->MessageBox( _T("Status bar could not be created."), 
						  _T("PASM ERROR"),
						  MB_ICONHAND | MB_OK | MB_DEFBUTTON1 );
	} else {
		//	get size of window, use to configure the status
		//	bar with four separate parts
		CRect rWin;
   		this->GetWindowRect(&rWin);
   		nTotWide = rWin.right-rWin.left;
		
		// Make 5 sections from the total width of the window.
		int nWidths[5];
		f32 fWidth = (f32)nTotWide;
		nWidths[0] = (int)( fWidth * 0.21f );// set equal to right edge
   		nWidths[1] = nWidths[0] + (int)( fWidth * 0.21f );
   		nWidths[2] = nWidths[1] + (int)( fWidth * 0.21f );
		nWidths[3] = nWidths[2] + (int)( fWidth * 0.22f );
		nWidths[4] = -1;// uses remaining space

		m_StatusBar.SetMinHeight( 25 );
		m_StatusBar.SetParts( 5, nWidths ); 
		
		// now lets put some text into the sections of the status bar
		CString s( " 0 Files Selected" );
		m_StatusBar.SetText( s, 0, 0 );

		s.Format( "ver # %d.%d.%d", fversion_GetToolMajorVer(), 
									fversion_GetToolMinorVer(),
									fversion_GetToolSubVer() );
		m_StatusBar.SetText( s, 4, 0 );
	}

	/////////////////////////////////////
	// SCAN THE LIB, LOCAL, & MASTER FILE
	RescanAll( FALSE );

	if( m_bUserQuitNetLockWait ) {
		// the user quit the net wait at this stage, they probably don't have
		// a network connection, let's save the fact that it has been turned off
		// and not make them cancel the wait each time they start PASM.
		m_bUserQuitNetLockWait = FALSE;
	}

	///////////////////////////////////
	// SETUP THE FILE VIEW LIST CONTROL
	CRect Rect;
	m_ctrlFileList.GetClientRect( &Rect );
	
#if _NO_LIBRARY_SUPPORT
	int nColX = (Rect.Width()-16) / 10;// subtract 16 to account for the right scroll bar width
	m_ctrlFileList.InsertColumn( 0, "Filenames",		LVCFMT_LEFT, nColX * 3 );
	m_ctrlFileList.InsertColumn( 1, "Ext",				LVCFMT_LEFT, nColX );
	m_ctrlFileList.InsertColumn( 2, "Library Files",	LVCFMT_LEFT, 0 );
	m_ctrlFileList.InsertColumn( 3, "Local Files",		LVCFMT_LEFT, nColX * 3 );
	m_ctrlFileList.InsertColumn( 4, "Master Files",		LVCFMT_LEFT, nColX * 3 );	
#else
	int nColX = (Rect.Width()-16) / 13;// subtract 16 to account for the right scroll bar width
	m_ctrlFileList.InsertColumn( 0, "Filenames",		LVCFMT_LEFT, nColX * 3 );
	m_ctrlFileList.InsertColumn( 1, "Ext",				LVCFMT_LEFT, nColX );
	m_ctrlFileList.InsertColumn( 2, "Library Files",	LVCFMT_LEFT, nColX * 3 );
	m_ctrlFileList.InsertColumn( 3, "Local Files",		LVCFMT_LEFT, nColX * 3 );
	m_ctrlFileList.InsertColumn( 4, "Master Files",		LVCFMT_LEFT, nColX * 3 );
#endif
	
	UpdateFileListBox( FALSE );

	// set some properties for our list box
	m_ctrlFileList.SetFullRowSel( TRUE );
	m_ctrlFileList.SortColumn( Settings.GetSortColumn(), Settings.GetAscending() );	
	m_ctrlFileList.EnableSeparatorLines( TRUE );

	UpdateData( VARS_TO_CONTROLS );

	// save our settings out to disk
	SaveSettingsOutToDisk( TRUE, TRUE );

	////////////////////////
	// INIT THE FANG SYSTEM
	fang_Init();
	Fang_ConfigDefs.nRes_HeapBytes = (16*1024*1024);
	m_bFangOK = fang_Startup();
	if( !m_bFangOK ) {
		// could not install the fang engine, tell the user and exit
		this->MessageBox( "Fang Engine failed to install.", 
						  "PASM Install Failure",
						  MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 );  
		return TRUE;
	}

	fmesh_InitNormalSphere();

	///////////////////////////////
	// SET THE TOOLTIPS FOR OUR APP
	CString sToolTipText( "Compile selected files into master file (Alt+C)" );
	m_CompileButton.SetToolTipText( &sToolTipText );
	m_CompileButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_CompileButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_CompileButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_CompileButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );

	sToolTipText = "Checks OUT selected files from library (Alt+O)";
	m_SyncDownButton.SetToolTipText( &sToolTipText );
	m_SyncDownButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_SyncDownButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_SyncDownButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_SyncDownButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );

	sToolTipText = "Checks IN selected files to library (Alt+I)";
	m_SyncUpButton.SetToolTipText( &sToolTipText );
	m_SyncUpButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_SyncUpButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_SyncUpButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_SyncUpButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );
	
	sToolTipText = "Copies new/newer files from library then compiles them into master file";
	m_SyncDownAndCompileButton.SetToolTipText( &sToolTipText );
	m_SyncDownAndCompileButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_SyncDownAndCompileButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_SyncDownAndCompileButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_SyncDownAndCompileButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );

	sToolTipText = "Selects all library files that are new/newer than local files";
	m_NewerThanLocButton.SetToolTipText( &sToolTipText );
	m_NewerThanLocButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_NewerThanLocButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_NewerThanLocButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_NewerThanLocButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );
	
	sToolTipText = "Selects all local files that are new/newer than library files";
	m_NewerThanLibButton.SetToolTipText( &sToolTipText );
	m_NewerThanLibButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_NewerThanLibButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_NewerThanLibButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_NewerThanLibButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );
	 
	sToolTipText = "Selects all local files that are new/newer than master file";
	m_NewerThanMasterButton.SetToolTipText( &sToolTipText );
	m_NewerThanMasterButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_NewerThanMasterButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_NewerThanMasterButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_NewerThanMasterButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );
	
	sToolTipText = "Rescans both the library and local drives";
	m_RescanButton.SetToolTipText( &sToolTipText );
	m_RescanButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_RescanButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_RescanButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_RescanButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );
	
	sToolTipText = "Compiles new/newer local files into master file (Alt+R)";
	m_ScanAndCompileButton.SetToolTipText( &sToolTipText );
	m_ScanAndCompileButton.m_ToolTip.SetMaxTipWidth( _MAX_TOOLTIP_WIDTH );
	m_ScanAndCompileButton.m_ToolTip.SetDelayTime( TTDT_AUTOPOP, _TOOLTIP_VISIBLE_MSECS );
	m_ScanAndCompileButton.m_ToolTip.SetDelayTime( TTDT_INITIAL, _INITIAL_TOOLTIP_MSECS );
	m_ScanAndCompileButton.m_ToolTip.SetDelayTime( TTDT_RESHOW, _RESHOW_TOOLTIP_MSECS );

	///////////////////////////////////////////////////////////
	// TEST THE VERSION INFOMATION CONTAINED IN THE MASTER FILE
//	PostMessage( APP_MSG_CODES_PASM_CHECK_MASTER_FILE_VERSION );
 
	return TRUE;  // return TRUE  unless you set the focus to a control
}

long CPasmDlg::OnCheckMasterFileVersion( WPARAM wParam, LPARAM lParam ) {

	CheckMasterFileCompilerVersion();

	return 0;
}

// returns FALSE if versions didn't match and user didn't sync
// TRUE if versions matched or user sync'ed them up
BOOL CPasmDlg::CheckMasterFileCompilerVersion() {

	u32 nTgaVersion, nApeVersion, nMtxVersion, nCsvVersion, nFntVersion, nSmaVersion, nGtVersion, nWvbVersion, nFprVersion, nCamVersion;
	BOOL bFixTGAFiles, bFixApeFiles, bFixMtxFiles, bFixCsvFiles, bFixFntFiles, bFixSmaFiles, bFixGtFiles, bFixWvbFiles, bFixFprFiles, bFixCamFiles;

	if ( m_bMasterFileVersionsChecked )
	{
		// We've already checked the master file versions so there is no need to check again
		return TRUE;
	}

	nTgaVersion = m_MasterFile.GetTgaVersionFromHeader();
	nApeVersion = m_MasterFile.GetApeVersionFromHeader();
	nMtxVersion = m_MasterFile.GetMtxVersionFromHeader();
	nCsvVersion = m_MasterFile.GetCsvVersionFromHeader();
	nFntVersion = m_MasterFile.GetFntVersionFromHeader();
	nSmaVersion = m_MasterFile.GetSmaVersionFromHeader();
	nGtVersion = m_MasterFile.GetGtVersionFromHeader();
	nWvbVersion = m_MasterFile.GetWvbVersionFromHeader();
	nFprVersion = m_MasterFile.GetFprVersionFromHeader();
	nCamVersion = m_MasterFile.GetCamVersionFromHeader();

	if( nTgaVersion ||
		nApeVersion || 
		nMtxVersion || 
		nCsvVersion || 
		nFntVersion || 
		nSmaVersion || 
		nGtVersion ||
		nWvbVersion ||
		nFprVersion ||
		nCamVersion ) {
		// we got a valid version, so the file must exist, check the versions
		bFixMtxFiles = !( nMtxVersion == FDATA_PRJFILE_MTX_VERSION );
		bFixCsvFiles = !( nCsvVersion == FDATA_PRJFILE_CSV_VERSION );
		bFixFntFiles = !( nFntVersion == FDATA_PRJFILE_FNT_VERSION );
		bFixSmaFiles = !( nSmaVersion == FDATA_PRJFILE_SMA_VERSION );
		bFixGtFiles = !( nGtVersion == FDATA_PRJFILE_GT_VERSION );
		bFixWvbFiles = !( nWvbVersion == FDATA_PRJFILE_WVB_VERSION );
		bFixFprFiles = !( nFprVersion == FDATA_PRJFILE_FPR_VERSION );
		bFixCamFiles = !( nCamVersion == FDATA_PRJFILE_CAM_VERSION );

		if( m_bXBoxFormat ) {
			bFixTGAFiles = !( nTgaVersion == FDATA_PRJFILE_XB_TGA_VERSION );
			bFixApeFiles = !( nApeVersion == FDATA_PRJFILE_XB_APE_VERSION );			
		} else if( m_bGCFormat ) {
			bFixTGAFiles = !( nTgaVersion == FDATA_PRJFILE_GC_TGA_VERSION );
			bFixApeFiles = !( nApeVersion == FDATA_PRJFILE_GC_APE_VERSION );
		} else {
			bFixTGAFiles = bFixApeFiles = bFixMtxFiles = bFixCsvFiles = bFixFntFiles = bFixSmaFiles = FALSE;
		}

		if( bFixTGAFiles ||
			bFixApeFiles ||
			bFixMtxFiles || 
			bFixCsvFiles ||
			bFixFntFiles || 
			bFixSmaFiles ||
			bFixGtFiles ||
			bFixWvbFiles ||
			bFixFprFiles ||
			bFixCamFiles ) {
			// we need to fix one or more of the file formats
						
			// construct the message box text
			CString sMsg;
			sMsg = "The compiler version has changed for the following file types:\n\n";
			if( bFixTGAFiles ) {
				sMsg += " *.TGA ";	
			}
			if( bFixApeFiles ) {
				sMsg += " *.APE ";
				sMsg += " *.WLD ";
			}
			if( bFixMtxFiles ) {
				sMsg += " *.mtx ";
			}
			if( bFixCsvFiles ) {
				sMsg += " *.CSV ";
			}
			if( bFixFntFiles ) {
				sMsg += " *.FNT ";
			}
			if( bFixSmaFiles ) {
				sMsg += " *.SMA ";
			}
			if( bFixGtFiles ) {
				sMsg += " *.GT ";
			}
			if( bFixWvbFiles ) {
				sMsg += " *.WVB ";
			}
			if( bFixFprFiles ) {
				sMsg += " *.FPR ";
			}
			if( bFixCamFiles ) {
				sMsg += " *.CAM ";
			}
			sMsg += "\n\nFiles must be recompiled to bring the master file up-to-date.\n";
			sMsg += "This process can be lengthy, but will only need to occur once.\n";
			sMsg += "Start recompiling? (NO will cancel current operation and leave\n";
			sMsg += "master file out-of-date.)";

			if( MessageBox( sMsg, _T("PASM"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON1 ) == IDYES ) {
				// make sure that show differences is off, otherwise there is nothing actually in the list box to select
				if( m_bShowDifferences ) {
					m_bShowDifferences = FALSE;
					// rebuild the list box, no need to sort the items though
					UpdateFileListBox( FALSE );
					// restore the show differences state
					m_bShowDifferences = TRUE;
				}
				// select all files that we need to recompile
				int i, nItemCount;
				CMyItemInfo *pItemInfo;
				CFileInfo *pFileInfo;
				CString sFileExt;

				nItemCount = m_ctrlFileList.GetItemCount();
				for( i=0; i < nItemCount; i++ ) {
					pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( i );
					FASSERT( pItemInfo );
					
					pFileInfo = pItemInfo->GetLocFileInfo();
					if( pFileInfo ) {
						sFileExt = pFileInfo->GetFileExt();
						if( ( bFixTGAFiles && (sFileExt.CompareNoCase( ".tga" ) == 0) ) ||
							( bFixApeFiles && (sFileExt.CompareNoCase( ".ape" ) == 0) ) ||
							( bFixApeFiles && (sFileExt.CompareNoCase( ".wld" ) == 0) ) ||
							( bFixMtxFiles && (sFileExt.CompareNoCase( ".mtx" ) == 0) ) ||
							( bFixCsvFiles && (sFileExt.CompareNoCase( ".csv" ) == 0) ) ||
							( bFixFntFiles && (sFileExt.CompareNoCase( ".fnt" ) == 0) ) ||
							( bFixSmaFiles && (sFileExt.CompareNoCase( ".sma" ) == 0) ) ||
							( bFixGtFiles && (sFileExt.CompareNoCase( ".gt" ) == 0) ) ||
							( bFixWvbFiles && (sFileExt.CompareNoCase( ".wvb" ) == 0) ) ||
							( bFixFprFiles && (sFileExt.CompareNoCase( ".fpr" ) == 0) ) ||
							( bFixCamFiles && (sFileExt.CompareNoCase( ".cam" ) == 0) ) ) {
							// select
							m_ctrlFileList.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
						} else {	
							// unselect
							m_ctrlFileList.SetItemState( i, 0, LVIS_SELECTED );
						}
					} else {
						// unselect	
						m_ctrlFileList.SetItemState( i, 0, LVIS_SELECTED );
					}
				}
				UpdateData( VARS_TO_CONTROLS );
				UpdateStatusBarFileCounts();
				Compile( FALSE, TRUE, TRUE );
			} else {
//				PostMessage( WM_COMMAND, IDCANCEL, 0 );	
				return FALSE;
			}
			m_bMasterFileVersionsChecked = TRUE;
			return TRUE;
		}
	}

	m_bMasterFileVersionsChecked = TRUE;
	return TRUE;
}

void CPasmDlg::OnSysCommand( UINT nID, LPARAM lParam ) {
	
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
void CPasmDlg::OnPaint() {
	
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

HCURSOR CPasmDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

void CPasmDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_bFangOK ) {
		// shutdown fang
		fang_Shutdown();
		m_bFangOK = FALSE;
	}
	
	// writeout our settings before we exit
	SaveSettingsOutToDisk( FALSE, TRUE );
	
	::RemoveProp( GetSafeHwnd(), Pasm_pszPropName );

	CDialog::OnCancel();
}

void CPasmDlg::OnOK() {
	//OnCancel();
}

BOOL CPasmDlg::PreTranslateMessage( MSG* pMsg ) {

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

	return CDialog::PreTranslateMessage( pMsg );
}

// sPathname should be in the form: C:\hello\mike.txt and this function will
// fill sFilename with: mike.  Returns TRUE if sFilename was filled in, FALSE 
// if there was a problem and the filename could not be computed.
BOOL CPasmDlg::GetFilenameFromFullPath( const CString &sPathname, CString &sFilename ) {
	
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

void CPasmDlg::StripTrailingBackSlashFromPathname( CString &rsPathname ) {
	
	if( rsPathname.IsEmpty() ) {
		return;
	}
	int nSlashIndex, nLength;
	nSlashIndex = rsPathname.ReverseFind( '\\' );
	if( nSlashIndex < 0 ) {
		// there are no '\' in rsPathname
		return;
	}
	nLength = rsPathname.GetLength();
	if( nSlashIndex != (nLength-1) ) {
		// rsPathname doesn't have a trailing '\'
		return;
	}
	rsPathname.Delete( nSlashIndex, 1 );
}

//sets up the master filename using our settings string vars (those need to be setup before calling)
void CPasmDlg::SetupCurrentMasterFilename() {
	CString sFilename;

	if( m_bGCFormat ) {
		m_sCurrentMasterFilename = m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
    } else if( m_bPS2Format ) {
        m_sCurrentMasterFilename = m_sPS2MasterFileDir;
#endif
	} else {
		m_sCurrentMasterFilename = m_sXBMasterFileDir;
	}
	StripTrailingBackSlashFromPathname( m_sCurrentMasterFilename );

	if( !m_sCurrentMasterFilename.IsEmpty() ) {
		m_sCurrentMasterFilename += '\\';	
	}

	GetFilenameFromFullPath( m_sMasterFilename, sFilename );

	m_sCurrentMasterFilename += sFilename;
	
	if( m_bXBoxFormat ) {
		m_sCurrentMasterFilename += "_xb";
	} else if( m_bGCFormat ) {
		m_sCurrentMasterFilename += "_gc";
#ifdef _MMI_TARGET_PS2
	} else if( m_bPS2Format ) {
		m_sCurrentMasterFilename += "_ps2";
#endif
	} else {
		m_sCurrentMasterFilename += "_pc";
	}
	m_sCurrentMasterFilename += ".mst";
}

// sets up the log filename using our settings string vars (those need to be setup before calling)
void CPasmDlg::SetupLogFile() {
	CString sLogFilename;

	sLogFilename = m_sLogDir;
	StripTrailingBackSlashFromPathname( sLogFilename );

	if( !sLogFilename.IsEmpty() ) {
		sLogFilename += '\\';
	}
	sLogFilename += "pasm.txt";
		
	CErrorLog::SetFilename( sLogFilename );
}

void CPasmDlg::OnPressConfigButton() {
	CSettingsDlg SettingsDlg;
	CString sFilename;
	
	UpdateData( CONTROLS_TO_VARS );

	// setup initial settings dialog vars
	SettingsDlg.m_sXBMasterFileDir = m_sXBMasterFileDir;
	SettingsDlg.m_sGCMasterFileDir = m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
    SettingsDlg.m_sPS2MasterFileDir = m_sPS2MasterFileDir;
#endif
	SettingsDlg.m_sLogDir = m_sLogDir;
	SettingsDlg.m_sExtractDir = m_sExtractDir;
	SettingsDlg.m_sConfigDir = m_sConfigDir;
	SettingsDlg.m_sIgnoreExtensions = m_sIgnoreExtensions;
	SettingsDlg.m_sConfigFilename = m_sConfigFilename;
	SettingsDlg.m_sMasterFilename = m_sMasterFilename;
	SettingsDlg.m_sLibDir = m_sLibDir;
	SettingsDlg.m_sLibraryLockDir = m_sLibLockDir;
	SettingsDlg.m_sLocalDir = m_sLocalDir;
	// !!Nate
	SettingsDlg.m_sXClientDir = m_sXClientFileDir;
	SettingsDlg.m_bAutoRunXClient = m_bAutoRunXClient;

	if( SettingsDlg.DoModal() == IDOK ) {
		// record the new values
		m_sXBMasterFileDir = SettingsDlg.m_sXBMasterFileDir;
		m_sGCMasterFileDir = SettingsDlg.m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
        m_sPS2MasterFileDir = SettingsDlg.m_sPS2MasterFileDir;
#endif
		m_sLogDir = SettingsDlg.m_sLogDir;
		m_sExtractDir = SettingsDlg.m_sExtractDir;
		m_sConfigDir = SettingsDlg.m_sConfigDir;
		m_sIgnoreExtensions = SettingsDlg.m_sIgnoreExtensions;
		FixIgnoreExtString( m_sIgnoreExtensions );
		m_sConfigFilename = SettingsDlg.m_sConfigFilename;
		m_sMasterFilename = SettingsDlg.m_sMasterFilename;
		m_sLibDir = SettingsDlg.m_sLibDir;
		m_sLibLockDir = SettingsDlg.m_sLibraryLockDir;
		m_sLocalDir = SettingsDlg.m_sLocalDir;
		// !!Nate
		m_sXClientFileDir = SettingsDlg.m_sXClientDir;
		m_bAutoRunXClient = SettingsDlg.m_bAutoRunXClient;

		// just in case the log dir changed, setup the log file again
		SetupLogFile();
		SetupCurrentMasterFilename();

		RescanAll( TRUE );

//		CheckMasterFileCompilerVersion();
		
		SaveSettingsOutToDisk( TRUE, TRUE );		
	}
}

void CPasmDlg::OnXBoxFormat() {
	
	UpdateData( CONTROLS_TO_VARS );
	// set the correct menu item as checked
	::CheckMenuItem( m_hMenu, ID_PLATFORM_XBOX, MF_CHECKED );
	m_bXBoxFormat = TRUE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PC, MF_UNCHECKED );
	m_bPCFormat = FALSE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_GC, MF_UNCHECKED );
	m_bGCFormat = FALSE;
#ifdef _MMI_TARGET_PS2
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PS2, MF_UNCHECKED );	// KJ
	m_bPS2Format = FALSE;										// KJ
#endif
	// re setup our master file
	SetupCurrentMasterFilename();
	ReadMasterFileEntries();
	UpdateData( VARS_TO_CONTROLS );	

	m_bMasterFileVersionsChecked = FALSE;

//	if( !CheckMasterFileCompilerVersion() ) 
	{
		// re setup our list box
		UpdateFileListBox( TRUE );
	}
}

#ifdef _MMI_TARGET_PS2
// KJ Begin
void CPasmDlg::OnPS2Format() {

	UpdateData( CONTROLS_TO_VARS );
	// set the correct menu item as checked
	::CheckMenuItem( m_hMenu, ID_PLATFORM_XBOX, MF_UNCHECKED );
	m_bXBoxFormat = FALSE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PC, MF_UNCHECKED );
	m_bPCFormat = FALSE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_GC, MF_UNCHECKED );
	m_bGCFormat = FALSE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PS2, MF_CHECKED );	// KJ
	m_bPS2Format = TRUE;										// KJ
	// re setup our master file
	SetupCurrentMasterFilename();
	ReadMasterFileEntries();
	UpdateData( VARS_TO_CONTROLS );
	if( !CheckMasterFileCompilerVersion() ) {
		// re setup our list box
		UpdateFileListBox( TRUE );
	}
}
// KJ End
#endif

void CPasmDlg::OnPCFormat()  {
	
	UpdateData( CONTROLS_TO_VARS );
	// set the correct menu item as checked
	::CheckMenuItem( m_hMenu, ID_PLATFORM_XBOX, MF_UNCHECKED );
	m_bXBoxFormat = FALSE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PC, MF_CHECKED );
	m_bPCFormat = TRUE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_GC, MF_UNCHECKED );
	m_bGCFormat = FALSE;
#ifdef _MMI_TARGET_PS2
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PS2, MF_UNCHECKED );	// KJ
	m_bPS2Format = FALSE;										// KJ
#endif
	// re setup our master file
	SetupCurrentMasterFilename();
	ReadMasterFileEntries();
	UpdateData( VARS_TO_CONTROLS );	

	m_bMasterFileVersionsChecked = FALSE;

//	if( !CheckMasterFileCompilerVersion() ) 
	{
		// re setup our list box
		UpdateFileListBox( TRUE );
	}
}

void CPasmDlg::OnGCFormat()  {
	
	UpdateData( CONTROLS_TO_VARS );
	// set the correct menu item as checked
	::CheckMenuItem( m_hMenu, ID_PLATFORM_XBOX, MF_UNCHECKED );
	m_bXBoxFormat = FALSE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PC, MF_UNCHECKED );
	m_bPCFormat = FALSE;
	::CheckMenuItem( m_hMenu, ID_PLATFORM_GC, MF_CHECKED );
	m_bGCFormat = TRUE;
#ifdef _MMI_TARGET_PS2
	::CheckMenuItem( m_hMenu, ID_PLATFORM_PS2, MF_UNCHECKED );	// KJ
	m_bPS2Format = FALSE;										// KJ
#endif
	// re setup our master file
	SetupCurrentMasterFilename();
	ReadMasterFileEntries();
	UpdateData( VARS_TO_CONTROLS );	

	m_bMasterFileVersionsChecked = FALSE;

//	if( !CheckMasterFileCompilerVersion() ) 
	{
		// re setup our list box
		UpdateFileListBox( TRUE );
	}
}

void CPasmDlg::OnOutputDebugPics() {

	UpdateData( CONTROLS_TO_VARS );
	m_bOutputDebugPics = !m_bOutputDebugPics;
	::CheckMenuItem( m_hMenu, ID_OUTPUT_DEBUG_PICS, m_bOutputDebugPics ? MF_CHECKED : MF_UNCHECKED );
}
	

void CPasmDlg::SaveSettingsOutToDisk( BOOL bSaveConfigFile, BOOL bSaveColumnSortVars ) {
	UpdateData( CONTROLS_TO_VARS );

	CSettings& Settings = CSettings::GetCurrent();
#if !FANG_WINGC	
	// don't save out the platform if we are a gc build
	Settings.m_bXBoxFormat = m_bXBoxFormat;
	Settings.m_bPCFormat = m_bPCFormat;
	Settings.m_bGCFormat = m_bGCFormat;
#ifdef _MMI_TARGET_PS2
    Settings.m_bPS2Format = m_bPS2Format;
#endif
#endif
	Settings.m_bShowDifferences = m_bShowDifferences;
	Settings.m_bHoldCompileStats = m_bHoldCompileStats;
	if( !m_bUserQuitNetLockWait ) {
		Settings.m_bDisableLibrarySupport = m_bDisableLibrarySupport;
	}
	Settings.m_bMustRunConfig = m_bMustRunConfig;
	Settings.m_sXBMasterFileDir = m_sXBMasterFileDir;
	Settings.m_sGCMasterFileDir = m_sGCMasterFileDir;
#ifdef _MMI_TARGET_PS2
    Settings.m_sPS2MasterFileDir = m_sPS2MasterFileDir;
#endif
	Settings.m_sLogDir = m_sLogDir;
	Settings.m_sExtractDir = m_sExtractDir;
	Settings.m_sConfigDir = m_sConfigDir;
	FixIgnoreExtString( m_sIgnoreExtensions );
	Settings.m_sIgnoreExtensions = m_sIgnoreExtensions;	
	Settings.m_sConfigFilename = m_sConfigFilename;
	// !!Nate
	Settings.m_sXClientFileDir = m_sXClientFileDir;
	Settings.m_bAutoRunXClient = m_bAutoRunXClient;

	if( bSaveColumnSortVars ) {
		Settings.m_nSortColumn = m_ctrlFileList.GetSortedColumn();
		Settings.m_bAscendingSort = m_ctrlFileList.IsAscSorted();
	}
	Settings.m_bHideNonCompliableFiles = m_bHideNonCompliableFiles;
	
	Settings.SaveCommonDataOutToFile();
	
	if( bSaveConfigFile ) {
		CConfigFile ConfigFile;
		CString sFilename;

		ConfigFile.m_sMasterFilename = m_sMasterFilename;
		ConfigFile.m_sLibDir = m_sLibDir;
		ConfigFile.m_sLibraryLockDir = m_sLibLockDir;
		ConfigFile.m_sLocalDir = m_sLocalDir;

		CSettingsDlg::CreateFilename( sFilename, m_sConfigDir, m_sConfigFilename );

		if( !ConfigFile.Write( sFilename ) ) {
			this->MessageBox( "Trouble writing out config file.", 
							  "PASM ERROR",
							  MB_OK | MB_ICONSTOP );	
		}
	}
}

BOOL CPasmDlg::ScanLocalDrive() {

	m_LocalFiles.RemoveAll();
	m_MapLocalFiles.RemoveAll();

	if( m_sLocalDir.IsEmpty() ) {
		return FALSE;
	}

	CString sRootDir = m_sLocalDir;
	StripTrailingBackSlashFromPathname( sRootDir );
	sRootDir += '\\';

	// gather file info on every file in sRootDir and below (NOT SORTED, TOO SLOW)
	m_LocalFiles.AddDir( sRootDir, "*.*", TRUE, 
						 CFileInfoArray::AP_NOSORT,
					     FALSE );
	return TRUE;
}

BOOL CPasmDlg::ScanLibDrive() {
	
	m_LibFiles.RemoveAll();
	m_MapLibFiles.RemoveAll();

	if( m_sLibDir.IsEmpty() ) {
		return FALSE;
	} 

	CString sRootDir = m_sLibDir;
	StripTrailingBackSlashFromPathname( sRootDir );
	sRootDir += '\\';

	EnableWindow( FALSE );
	if( m_FileLock.WaitToLockFile( this ) ) {
		// gather file info on every file in sRootDir and below (NOT SORTED, TOO SLOW) 
		m_LibFiles.AddDir( sRootDir, "*.*", TRUE, 
						   CFileInfoArray::AP_NOSORT,
						   FALSE );

		// while we are here, try to read the dellist file
		ReadDeleteListFile();
		m_FileLock.UnlockFile();
	} else {		
		UserQuitWaitingForLibraryLock( FALSE );
	}
	SetForegroundWindow();
	EnableWindow( TRUE );

	return TRUE;
}

void CPasmDlg::UpdateFileListBox( BOOL bReSortList ) {
	int i, nNumFiles;
	u32 nIndex;
	CString sFilename, sExt;
	CFileInfo *pLocFileInfo, *pLibFileInfo;
	const FDataPrjFile_Entry_t *pEntry;	
	const CFileInfo *pFileInfoData;
	BOOL bFileIsInIgnoreList;

	UpdateData( CONTROLS_TO_VARS );
	
	m_ctrlFileList.DeleteAllItems();

	nIndex = 0;
	// add all of the master files
	nNumFiles = m_MasterFile.GetNumEntries();
	for( i=0; i < nNumFiles; i++ ) {
		pEntry = m_MasterFile.GetEntryByIndex( i );
		if( pEntry ) {
			pLocFileInfo = EntryToFileInfo( pEntry, sFilename, TRUE, TRUE );
			pLibFileInfo = EntryToFileInfo( pEntry, sFilename, FALSE, FALSE );

			if( m_bShowDifferences && !pLocFileInfo && !pLibFileInfo ) {
				if( IsFilenameALightmap( pEntry->szFilename ) ) {
					// this is a lightmap file, ignore it
					continue;
				}
			}

			// add pEntry to our list control
			AddElementToListBox( sFilename, pLibFileInfo, pLocFileInfo, (FDataPrjFile_Entry_t *)pEntry, nIndex );			
		}
	}

	// add all of the local files that didn't have a master file entry
	nNumFiles = m_LocalFiles.GetSize();
	pFileInfoData = m_LocalFiles.GetData();
	for( i=0; i < nNumFiles; i++ ) {
		pLocFileInfo = (CFileInfo *)&pFileInfoData[i];

		sExt = pLocFileInfo->GetFileExt();
		sExt.MakeLower();
		// make sure that the extension is unique and not confused with other file extensions
		// add a trailing ';' to sExt
		sExt += ';';

		bFileIsInIgnoreList = IsExtInIgnoreList( sExt );
		if( bFileIsInIgnoreList && m_bHideNonCompliableFiles ) {
			continue;
		}
		
		pEntry = FileInfoToEntry( pLocFileInfo, sFilename );
		if( !pEntry ) {
			// this entry wasn't added in the previous pass, add it now
			pLibFileInfo = EntryToFileInfo( NULL, sFilename, FALSE, FALSE );
			AddElementToListBox( sFilename, pLibFileInfo, pLocFileInfo, (FDataPrjFile_Entry_t *)pEntry, nIndex, bFileIsInIgnoreList );			
		}
	}

	// add all of the library files that didn't have a master file entry or local file
	nNumFiles = m_LibFiles.GetSize();
	pFileInfoData = m_LibFiles.GetData();
	for( i=0; i < nNumFiles; i++ ) {
		pLibFileInfo = (CFileInfo *)&pFileInfoData[i];

		sExt = pLibFileInfo->GetFileExt();
		sExt.MakeLower();
		// make sure that the extension is unique and not confused with other file extensions
		// add a trailing ';' to sExt
		sExt += ';';

		bFileIsInIgnoreList = IsExtInIgnoreList( sExt );
		if( bFileIsInIgnoreList && m_bHideNonCompliableFiles ) {
			continue;
		}
		
		pEntry = FileInfoToEntry( pLibFileInfo, sFilename );
		if( !pEntry ) {
			// this entry doesn't have a master file entry, does it have a local file?
			pLocFileInfo = EntryToFileInfo( NULL, sFilename, TRUE, FALSE );
			if( !pLocFileInfo ) {
				// this entry wasn't added before now, add it now
				AddElementToListBox( sFilename, pLibFileInfo, pLocFileInfo, (FDataPrjFile_Entry_t *)pEntry, nIndex, bFileIsInIgnoreList );
			}
		}
	}

	// resort the file list
	if( bReSortList ) {
		m_ctrlFileList.ReSort();
	}
	UpdateStatusBarFileCounts();
		
	UpdateData( VARS_TO_CONTROLS );
}

// returns TRUE if the element was added, FALSE if it was not (most likely show differences is checked)
BOOL CPasmDlg::AddElementToListBox( CString &rsFilename, CFileInfo *pLib, CFileInfo *pLoc, FDataPrjFile_Entry_t *pEntry,
								    u32 &rnIndex, BOOL bFileIsInIgnoreList/*=FALSE*/ ) {
	BOOL bAddItem;
	CTime LibTime, LocTime, MasTime;
	CMyItemInfo *pItemInfo;
	FDataPrjFile_SupportEntry_t *pSupportEntry;

	if( m_bDisableLibrarySupport ) {
		// fool our functions into doing the right thing without duplicating alot of code :-)
		pLib = pLoc;
	}
	if( !pLib || !pLoc || !pEntry ) {
		// one of the files doesn't exist, this element should be added regardless of "show differences" setting
		bAddItem = TRUE;
		
		// oophs, make sure that pEntry isn't a support file or non-compilable file with "show differences" = on
		if( pLib && pLoc && !pEntry && m_bShowDifferences ) {
			// could be a non-compilable file
			if( bFileIsInIgnoreList ) {
				// only display files where the loc & lib times don't match
				LocTime = pLoc->GetLastWriteTime();
				LibTime = pLib->GetLastWriteTime();
				if( AreTimesEqual( LocTime, LibTime ) ) {				
					// the times match, don't display this element
					bAddItem = FALSE;
				}
			} else { 
				// could be a support file
				if( rsFilename.Find( ".aid", 0 ) >= 0 ) {
					// grab the support entry
					pSupportEntry = m_MasterFile.GetSupportEntryByName( rsFilename );
					if( pSupportEntry ) {
						// see if time's match
						LocTime = pLoc->GetLastWriteTime();
						LibTime = pLib->GetLastWriteTime();
						MasTime = (time_t)pSupportEntry->nModifiedTime;
						if( AreTimesEqual( LocTime, LibTime ) &&
							AreTimesEqual( LocTime, MasTime ) ) {
							// the times match, don't display this element
							bAddItem = FALSE;
						}		
					}
				}
			}
		}
	} else {
		bAddItem = (m_bShowDifferences) ? FALSE : TRUE;

		if( !bAddItem ) {
			// test the file times for differences
			LocTime = pLoc->GetLastWriteTime();
			LibTime = pLib->GetLastWriteTime();
			MasTime = (time_t)pEntry->nModifiedTime;
			
			if( !AreTimesEqual( LocTime, LibTime ) ||
				!AreTimesEqual( LocTime, MasTime ) ) {
				// the don't times match, display this element
				bAddItem = TRUE;
			}
		}
	}

	if( bAddItem ) {
		// add a new item to the list box control
		m_ctrlFileList.InsertItem( rnIndex, LPSTR_TEXTCALLBACK );
		if( m_bDisableLibrarySupport ) {
			pLib = NULL;
		}
		pItemInfo = new CMyItemInfo( rnIndex, rsFilename, pLib, pLoc, pEntry );
		FASSERT( pItemInfo );
		m_ctrlFileList.SetItemData( rnIndex, (DWORD)pItemInfo );
		m_ctrlFileList.SetItemText( rnIndex, 1, LPSTR_TEXTCALLBACK );
		m_ctrlFileList.SetItemText( rnIndex, 2, LPSTR_TEXTCALLBACK );
		m_ctrlFileList.SetItemText( rnIndex, 3, LPSTR_TEXTCALLBACK );
		m_ctrlFileList.SetItemText( rnIndex, 4, LPSTR_TEXTCALLBACK );

		rnIndex++;
	}

	return bAddItem;
}

void CPasmDlg::OnShowDifferences() {
	
	UpdateData( CONTROLS_TO_VARS );
	// toggle the show difference check mark & var
	m_bShowDifferences = !m_bShowDifferences;
	::CheckMenuItem( m_hMenu, ID_VIEW_SHOWDIFFERENCESONLY, m_bShowDifferences ? MF_CHECKED : MF_UNCHECKED );
	UpdateFileListBox( TRUE );
	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::OnHideNoncompilableFiles() {
	
	UpdateData( CONTROLS_TO_VARS );
	// toggle the hide non compilable files vars & check mark
	m_bHideNonCompliableFiles = !m_bHideNonCompliableFiles;
	::CheckMenuItem( m_hMenu, ID_VIEW_HIDE_NON_COMPILABLE_FILES, m_bHideNonCompliableFiles ? MF_CHECKED : MF_UNCHECKED );
	UpdateFileListBox( TRUE );
	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::OnHoldCompileStats() {
	UpdateData( CONTROLS_TO_VARS );
	// toggle the hold compile stats check mark & var
	m_bHoldCompileStats = !m_bHoldCompileStats;
	::CheckMenuItem( m_hMenu, ID_VIEW_HOLD_COMPILE_STATS, m_bHoldCompileStats ? MF_CHECKED : MF_UNCHECKED );
	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::OnDisableLibrarySupport() {
	
	UpdateData( CONTROLS_TO_VARS );
	// toggle the disable library support check mark & var
	m_bDisableLibrarySupport = !m_bDisableLibrarySupport;
	::CheckMenuItem( m_hMenu, ID_DISABLE_LIB, m_bDisableLibrarySupport ? MF_CHECKED : MF_UNCHECKED );
	m_LibFiles.RemoveAll();
	m_MapLibFiles.RemoveAll();
	if( !m_bDisableLibrarySupport ) {
		// the user has enabled the library system
		m_bUserQuitNetLockWait = FALSE;
	}
	EnableLibraryButtons( !m_bDisableLibrarySupport );
	RescanAll( TRUE );
	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::UserQuitWaitingForLibraryLock( BOOL bRescanAll ) {

	this->MessageBox( _T("Library support will be disabled until\n" 
						 "PASM is able to lock the library lock file.\n"
						 "You can re-enable it by clicking the\n"
						 "'Edit->Disable Library Support' checkbox." ),
						 _T("PASM: Disabling Library Support"),
						 MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON1 );
	m_bDisableLibrarySupport = TRUE;
	m_bUserQuitNetLockWait = TRUE;
	::CheckMenuItem( m_hMenu, ID_DISABLE_LIB, m_bDisableLibrarySupport ? MF_CHECKED : MF_UNCHECKED );
	m_LibFiles.RemoveAll();
	m_MapLibFiles.RemoveAll();
	EnableLibraryButtons( !m_bDisableLibrarySupport );
	if( bRescanAll ) {
		RescanAll( TRUE );
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CPasmDlg::EnableLibraryButtons( BOOL bEnable ) {
	
	((CButton *)GetDlgItem( IDC_SYNC_DOWN_AND_COMPILE ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_NEWER_THAN_LOCAL ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_NEWER_THAN_LIB ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_SYNC_DOWN ))->EnableWindow( bEnable );
	((CButton *)GetDlgItem( IDC_SYNC_UP ))->EnableWindow( bEnable );
}

void CPasmDlg::SelectAll() {
	int i, nItemCount;

	nItemCount = m_ctrlFileList.GetItemCount();
	for( i=0; i < nItemCount; i++ ) {
		m_ctrlFileList.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );	
	}	
}

void CPasmDlg::UnSelectAll() {
	POSITION Pos;
	int i;

	Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	while( Pos ) { 
		i = m_ctrlFileList.GetNextSelectedItem( Pos );
		m_ctrlFileList.SetItemState( i, 0, LVIS_SELECTED );
	}
}

void CPasmDlg::SelectNewAndNewer( Location_e nFrom, Location_e nTo ) {
	int i, nItemCount, nFirstSelectedItem = -1;
	CMyItemInfo *pItemInfo;
	CFileInfo *pFromFile, *pToFile;
	FDataPrjFile_Entry_t *pToEntry;
	CTime FromTime, ToTime;
	CString sEntryName;
	FDataPrjFile_SupportEntry_t *pSupportEntry;

	if( nFrom == nTo ) {
		// we should unselect everything, nothing will be new/newer
		UnSelectAll();
		return;
	}
	
	// walk all of the items in the list control
	nItemCount = m_ctrlFileList.GetItemCount();
	for( i=0; i < nItemCount; i++ ) {
		// grab the user data
		pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( i );
		FASSERT( pItemInfo );

		switch( nFrom ) {
		case LOCAL:
			pFromFile = pItemInfo->GetLocFileInfo();
			break;
		case LIBRARY:
			pFromFile = pItemInfo->GetLibFileInfo();
			break;
		case MASTER:
			// not supported as a FROM, it doesn't make sense
			pFromFile = NULL;
		default:
			FASSERT_NOW;
			break;
		}
		if( !pFromFile ) {
			// there is no FROM file, so we should not select this item
			m_ctrlFileList.SetItemState( i, 0, LVIS_SELECTED );
			continue;
		}
		// record the FROM time
		FromTime = pFromFile->GetLastWriteTime();

		switch( nTo ) {
		case LOCAL:
			pToFile = pItemInfo->GetLocFileInfo();
			pToEntry = NULL;
			break;
		case LIBRARY:
			pToFile = pItemInfo->GetLibFileInfo();
			pToEntry = NULL;
			break;
		case MASTER:
			pToEntry = pItemInfo->GetMasterFileEntry();
			pToFile = NULL;
			break;
		default:
			FASSERT_NOW;
			break;
		}

		if( !pToEntry && !pToFile ) {
			pSupportEntry = NULL;
			// if the TO == MASTER and pToFile == NULL we may have an .aid file
			// which don't get compiled into the master file, but do have
			// support entries to compare the dates to
			if( (nTo == MASTER) && (pToFile == NULL) ) {
				sEntryName = pFromFile->GetFileName();
				sEntryName.MakeLower();
				if( sEntryName.Find( ".aid", 0 ) >= 0 ) {
					pSupportEntry = m_MasterFile.GetSupportEntryByName( sEntryName );					
				}
			}
			if( !pSupportEntry ) {
				// there is no TO file, so we should select this item, it is NEW
				m_ctrlFileList.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
				if( nFirstSelectedItem < 0 ) {
					nFirstSelectedItem = i;
				}
				continue;
			} else {
				// grab the TO time from the support entry
				ToTime = (time_t)pSupportEntry->nModifiedTime;
			}			
		} else {
			// record the TO time
			if( pToFile ) {
				ToTime = pToFile->GetLastWriteTime();
			} else {
				ToTime = (time_t )pToEntry->nModifiedTime;
			}
		}

		// compare times
		if( FromTime > ToTime ) {
			// the FROM time is NEWER than the TO time, this item should be selected
			m_ctrlFileList.SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
			if( nFirstSelectedItem < 0 ) {
				nFirstSelectedItem = i;
			}
			continue;
		}

		// this item should not be selected
		m_ctrlFileList.SetItemState( i, 0, LVIS_SELECTED );
	}
	if( nFirstSelectedItem >= 0 ) {
		m_ctrlFileList.EnsureVisible( nFirstSelectedItem, FALSE );
		m_ctrlFileList.SetItemState( nFirstSelectedItem, LVIS_FOCUSED, LVIS_FOCUSED );
	}
	UpdateStatusBarFileCounts();
}

// select the local files that are new/newer than the master files
void CPasmDlg::OnSelectNewerThanMaster() {
	
	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}
	UpdateData( CONTROLS_TO_VARS );
	SelectNewAndNewer( LOCAL, MASTER );	
	UpdateData( VARS_TO_CONTROLS );	
}

// select the lib files that are new/newer than the local files
void CPasmDlg::OnSelectNewerThanLocal()  {

	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}
	UpdateData( CONTROLS_TO_VARS );
	SelectNewAndNewer( LIBRARY, LOCAL );	
	UpdateData( VARS_TO_CONTROLS );	
}

// select the local files that are new/newer than the lib files
void CPasmDlg::OnSelectNewerThanLib() {

	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}
	UpdateData( CONTROLS_TO_VARS );
	SelectNewAndNewer( LOCAL, LIBRARY );	
	UpdateData( VARS_TO_CONTROLS );	
}

void CPasmDlg::OnCompile() {
	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}
	Compile( FALSE );
}

void CPasmDlg::OnForcedCompile() {
	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}
	Compile( FALSE, TRUE );
}

void CPasmDlg::ReadMasterFileEntries() {
	MasterFileCompile_ErrorCodes_e nReturnCode;
	
	//nReturnCode = m_MasterFile.ReadHeaderAndEntries( m_sCurrentMasterFilename );
	// !!Nate
	nReturnCode = m_MasterFile.ReadHeaderAndEntries( m_sCurrentMasterFilename, FALSE, TRUE );

	if( nReturnCode != MASTER_FILE_COMPILE_ERROR_NONE ) {
		CString s;

		switch( nReturnCode ) {
		case MASTER_FILE_COMPILE_ERROR_OPENING:
		case MASTER_FILE_COMPILE_ERROR_NONE:
			// either the file doesn't exist or we opened just fine, either way it is an ok return value
			break;
		case MASTER_FILE_COMPILE_ERROR_READING:
			// prompt to delete
			s.Format( "Could not read the current master file:\n%s\nDo you want to delete that file so\n a new one can be built?", m_sCurrentMasterFilename );
			if( this->MessageBox( s, _T("PASM ERROR"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON1 ) == IDYES ) {
				::DeleteFile( m_sCurrentMasterFilename );		
			}
			break;
		case MASTER_FILE_COMPILE_ERROR_INVALID_SIGNATURE:
			// prompt to delete
			s.Format( "The current master file:\n%s\nis an invalid file.\nDo you want to delete that file so\n a new one can be built?", m_sCurrentMasterFilename );
			if( this->MessageBox( s, _T("PASM ERROR"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON1 ) == IDYES ) {
				::DeleteFile( m_sCurrentMasterFilename );		
			}
			break;
		case MASTER_FILE_COMPILE_ERROR_WRONG_VERSION:
			// prompt to delete
			s.Format( "The current master file:\n%s\nis an old version.\nDo you want to delete that file so\n a new one can be built?", m_sCurrentMasterFilename );
			if( this->MessageBox( s, _T("PASM ERROR"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON1 ) == IDYES ) {
				::DeleteFile( m_sCurrentMasterFilename );		
			}	
			break;
		case MASTER_FILE_COMPILE_ERROR_ALLOCATING_MEMORY:
			// prompt to free some memory, app
			if( this->MessageBox( _T("Could not allocate needed memory.\nTo free memory, try closing some\nwindows and then re-run PASM.\nPress Yes to exit PASM."),
								  _T("PASM ERROR"),
								  MB_ICONHAND|MB_YESNO|MB_DEFBUTTON1 ) == IDYES ) {
				this->PostMessage( WM_COMMAND, IDCANCEL, 0 );			
				return;
			}
			break;
		}
	}
	m_nMasterFileCount = m_MasterFile.GetNumEntries();
	UpdateStatusBarFileCounts();
	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::UpdateStatusBarFileCounts() {
	CString s;
	
	// cache the number of selected files
	m_nLastSelectedFilesCount = m_ctrlFileList.GetSelectedCount();
	s.Format( " Selected Files: %d", m_nLastSelectedFilesCount );
	m_StatusBar.SetText( s, 0, 0 );

	if( m_bDisableLibrarySupport ) {
		s = " LIBRARY DISABLED";
	} else {
		s.Format( " Library: %d File(s)", m_nLibFileCount ); 
	}
	m_StatusBar.SetText( s, 1, 0 );
	
	s.Format( " Local: %d File(s)", m_nLocalFileCount );
	m_StatusBar.SetText( s, 2, 0 );
	
	if( m_bXBoxFormat ) {
		s.Format( " Master(XB): %d File(s)", m_nMasterFileCount );
	} else if( m_bGCFormat ) {
		s.Format( " Master(GC): %d File(s)", m_nMasterFileCount );
#ifdef _MMI_TARGET_PS2
    } else if( m_bPS2Format ) {
        s.Format( " Master(PS2): %d File(s)", m_nMasterFileCount );
#endif
	} else {
		s.Format( " Master(PC): %d File(s)", m_nMasterFileCount );
	}
	m_StatusBar.SetText( s, 3, 0 );

	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::OnClearErrorLog() {
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	rErrorLog.ClearErrorLog();	
}

CFileInfo *CPasmDlg::EntryToFileInfo( const FDataPrjFile_Entry_t *pEntry, CString &rsFilename, BOOL bLocalFile, BOOL bUpdateFilename/*=TRUE*/ ) {
	void *pLookup;
	CFileInfo *pFileInfo;
		
	if( bUpdateFilename ) {
		FASSERT( pEntry != NULL );
		rsFilename = pEntry->szFilename;
		rsFilename.MakeLower();
	}

	// search for the filename
	if( bLocalFile ) {
		// search for a local file
		if( m_MapLocalFiles.Lookup( rsFilename, pLookup ) ) {
			pFileInfo = (CFileInfo *)pLookup;
		} else {
			pFileInfo = NULL;
		}
	} else {
		// search for a library file
		if( m_MapLibFiles.Lookup( rsFilename, pLookup ) ) {
			pFileInfo = (CFileInfo *)pLookup;
		} else {
			pFileInfo = NULL;
		}
	}

	return pFileInfo;
}

const FDataPrjFile_Entry_t *CPasmDlg::FileInfoToEntry( CFileInfo *pFileInfo, CString &rsFilename ) {
	const FDataPrjFile_Entry_t *pEntry;
	
	rsFilename = pFileInfo->GetFileName();
	rsFilename.MakeLower();

	pEntry = m_MasterFile.GetEntryByName( rsFilename );
		
	return pEntry;
}

void CPasmDlg::OnRebuildMasterFile() {
	// warn the user about the time
	if( this->MessageBox( _T( "Rebuilding the entire master file\ncan be VERY time consuming,\nproceed?" ),
						  _T( "PASM Warning" ),
						  MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1 ) == IDYES ) {
		RescanAll( TRUE );
		Compile( TRUE );
	}	
}

void CPasmDlg::Compile( BOOL bRebuildAll, BOOL bForceCompilation/*=FALSE*/, BOOL bUpdateMasterFileVersionNums/*=FALSE*/ ) {
	CCompileDlg CompileDlg;
	CFileInfo *pFileInfo, *pAidFileInfo;
	const CFileInfo *pFileInfoData;
	POSITION Pos;
	int nIndex, nNumFiles;
	CMyItemInfo *pItemInfo;
	CString sFileExt, s;
	
	UpdateData( CONTROLS_TO_VARS );

	if( !bRebuildAll ) {
		if( m_ctrlFileList.GetSelectedCount() <= 0 &&
			!bForceCompilation &&
			!bUpdateMasterFileVersionNums ) {
			// nothing selected and there is no reason to force a compile
			return;
		}
	}

	// save our settings before starting a long process (also CompileDlg will read the settings to get the platform)
	SaveSettingsOutToDisk( FALSE, TRUE );

	// VERY IMPORTANT**********************************
	// SETUP INITIAL COMPILE DIALOG VARS
	CompileDlg.m_FilesToCompile.RemoveAll();
	CompileDlg.m_FilesToCompile.SetSize( 0, 100 );

	CompileDlg.m_AidFiles.RemoveAll();
	CompileDlg.m_AidFiles.SetSize( 0, 50 );

	CompileDlg.m_sMasterFilename = m_sCurrentMasterFilename;
	CompileDlg.m_bRebuildAll = bRebuildAll;
	CompileDlg.m_bForceCompile = (bRebuildAll || bForceCompilation) ? TRUE : FALSE;
	CompileDlg.m_bUpdateMasterFileVersion = (bRebuildAll || bUpdateMasterFileVersionNums) ? TRUE : FALSE;
	CompileDlg.m_bOutputDebugPics = m_bOutputDebugPics;	
	CompileDlg.m_bHoldCompilationStats = m_bHoldCompileStats;
	CompileDlg.m_bDownsizeTextures = m_bDownsizeTextures;
#ifdef _MMI_TARGET_PS2
    CompileDlg.m_bGenerateMeshStrips = FALSE;   //NS Disable PASM's stripping for PS2
#else
	CompileDlg.m_bGenerateMeshStrips = !m_bDisableStripping;
#endif
	CompileDlg.m_bDetailedLightingInfo = m_bDetailedLightingInfo;
	CompileDlg.m_bIgnoreVIS = m_bDontUseVIS;
	CompileDlg.m_bReusePerfectVISMatchOnly = m_bReusePerfectVISMatchOnly;
	if( m_bXBoxFormat ) {
		CompileDlg.m_nTargetPlatform = PASM_TARGET_XB;
	} else if( m_bGCFormat ) {
		CompileDlg.m_nTargetPlatform = PASM_TARGET_GC;
	} else if( m_bPCFormat ) {
		CompileDlg.m_nTargetPlatform = PASM_TARGET_PC;
#ifdef _MMI_TARGET_PS2
	} else if( m_bPS2Format ) {
        CompileDlg.m_nTargetPlatform = PASM_TARGET_PS2;
#endif
	} else {
		FASSERT_NOW;
		return;
	}
	//*******************************************************
	
	if( !bRebuildAll ) {
		// add all selected that have local files
		Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
		while( Pos ) {
			nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
			pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
			FASSERT( pItemInfo );
			
			pFileInfo = pItemInfo->GetLocFileInfo();
			if( pFileInfo ) {
				// if pFileInfo is a support file (.aid), then
				// don't add it, but see if there is an asset file
				// (.ape or .tga) file with the same name and add 
				// that file instead, duplicates will be removed later
				sFileExt = pFileInfo->GetFileExt();
				if( sFileExt.CompareNoCase( ".aid" ) == 0 ) {
					// grab a compilable file that goes with this aid file
					pFileInfo = FindCompilableLocalFileFromAidFile( pFileInfo );
					if( !pFileInfo ) {
						// nothing to compile
						continue;
					}
				}
				// grab any aid file that may be present
				pAidFileInfo = GetAidFileInfo( pFileInfo, TRUE );
				
				// add pFileInfo to our compile list
				CompileDlg.m_FilesToCompile.Add( *pFileInfo );
				if( pAidFileInfo ) {
					CompileDlg.m_AidFiles.Add( *pAidFileInfo );
				}
			}
		}
	} else {
		// select all local files (except for *.aid)
		nNumFiles = m_LocalFiles.GetSize();
		pFileInfoData = m_LocalFiles.GetData();
		for( nIndex=0; nIndex < nNumFiles; nIndex++ ) {
			pFileInfo = (CFileInfo *)&pFileInfoData[ nIndex ];	

			// if pFileInfo is a support file (.aid), then don't add it
			sFileExt = pFileInfo->GetFileExt();
			if( sFileExt.CompareNoCase( ".aid" ) == 0 ) {
				continue;
			}
			// grab any aid file that may be present
			pAidFileInfo = GetAidFileInfo( pFileInfo, TRUE );

			// add pFileInfo to our compile list
			CompileDlg.m_FilesToCompile.Add( *pFileInfo );
			if( pAidFileInfo ) {
				CompileDlg.m_AidFiles.Add( *pAidFileInfo );
			}
		}
	}

	if( CompileDlg.m_FilesToCompile.GetSize() <= 0 &&
		!CompileDlg.m_bForceCompile && 
		!CompileDlg.m_bUpdateMasterFileVersion ) {
		// none of the selected files had local files, and we don't have to update the master file
		return;
	}

	if( CompileDlg.m_FilesToCompile.GetSize() > 0 ) {
		// remove any duplicate names that may have been added
		u32 nNumTooLong, nNumZeroByte, nNumDups;
		RemoveBogusFiles( CompileDlg.m_FilesToCompile, FALSE, FALSE, FALSE, FALSE, TRUE, nNumTooLong, nNumZeroByte, nNumDups );
		RemoveBogusFiles( CompileDlg.m_AidFiles, FALSE, FALSE, FALSE, FALSE, TRUE, nNumTooLong, nNumZeroByte, nNumDups );
		 
		// remove files with extensions in our ignore list
		CString sPlatformExtIgnoreList( m_sIgnoreExtensions );
		if( m_bXBoxFormat || m_bPCFormat ) {
			sPlatformExtIgnoreList += "*.rdg;";
		} else if( m_bGCFormat ) {
			sPlatformExtIgnoreList += "*.rdx;";
			sPlatformExtIgnoreList += "*.wvb;";
		}
		int nNumRemainingFiles = CompileDlg.m_FilesToCompile.RemoveExtFiles( sPlatformExtIgnoreList );
		if( nNumRemainingFiles <= 0 &&
			!CompileDlg.m_bForceCompile &&
			!CompileDlg.m_bUpdateMasterFileVersion ) {
			// there are no compilable files to compile, and we don't have to update the master file
			return;
		}
		
		// change the master file count status bar text to "Compiling: x File(s)"
		s.Format( " Compiling: %d File(s)", nNumRemainingFiles );
		m_StatusBar.SetText( s, 3, 0 );
	}

	if( CompileDlg.DoModal() == IDOK ||
		CompileDlg.m_bForceCompile || 
		CompileDlg.m_bUpdateMasterFileVersion ) {

		// !!Nate
		if (m_bXBoxFormat && m_bAutoRunXClient && CompileDlg.m_uNumCompiledFiles > 0) {
			CSyncDialog SyncDialog;

			if( bForceCompilation ) {
				SyncDialog.SetFilesToForceCompile( CompileDlg.m_FilesToCompile );
			}

			SyncDialog.SetXClientName(m_sXClientFileDir);
			SyncDialog.DoModal();
		}

		// re-read the master  file and update our list box
		ReadMasterFileEntries();
		UpdateFileListBox( TRUE );				
	} else {
		// we didn't compile anything, change the master file count status bar text
		if( m_bXBoxFormat ) {
			s.Format( " Master(XB): %d File(s)", m_nMasterFileCount );
		} else if( m_bGCFormat ) {
			s.Format( " Master(GC): %d File(s)", m_nMasterFileCount );
#ifdef _MMI_TARGET_PS2
        } else if( m_bPS2Format ) {
            s.Format( " Master(PS2): %d File(s)", m_nMasterFileCount );
#endif
		} else {
			s.Format( " Master(PC): %d File(s)", m_nMasterFileCount );
		}
		m_StatusBar.SetText( s, 3, 0 );
	}
	
	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::OnRescan() {
	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}
	RescanAll( TRUE );
}

void CPasmDlg::RescanAll( BOOL bSetupListBox ) {
	u32 nNumDups;
	u32 nNumTooLong;
	u32 nNumZeroByte;
	BOOL bPromptToViewErrorLog;
	CString sOne, sTwo, sThree, sMsg;
	CStringList sFilesDeleted;

	UpdateData( CONTROLS_TO_VARS );

	BeginWaitCursor();

	bPromptToViewErrorLog = FALSE;
	sMsg.Empty();

	if( !m_bDisableLibrarySupport ) {
		// rescan the library
		ScanLibDrive();
		RemoveBogusFiles( m_LibFiles, TRUE, FALSE, TRUE, TRUE, TRUE, nNumTooLong, nNumZeroByte, nNumDups );
		if( nNumDups || nNumTooLong || nNumZeroByte ) {
			bPromptToViewErrorLog = TRUE;

			if( nNumDups == 1 ) {
				sOne.Format( "There is a duplicate file in the library.\n" );
			} else if( nNumDups > 1 ) {
				sOne.Format( "There are %d duplicate files in the library.\n", nNumDups );
			}
			if( nNumTooLong == 1 ) {
				sTwo.Format( "There is a filename in the library which is too long.\n" );
			} else if( nNumTooLong > 1 ) {
				sTwo.Format( "There are %d filenames in the library that are too long.\n", nNumTooLong );
			}
			if( nNumZeroByte == 1 ) {
				sThree.Format( "There is a zero byte file in the library.\n" );
			} else if( nNumZeroByte > 1 ) {
				sThree.Format( "There are %d zero byte files in the library.\n", nNumZeroByte );
			}
			sMsg += sOne;
			sMsg += sTwo;
			sMsg += sThree;
		}
		m_nLibFileCount = m_LibFiles.GetSize();
		CFileInfoArray::BuildMapFromFileInfoArray( m_MapLibFiles, m_LibFiles, 
												   CFileInfoArray::AP_IGNORE_CASE | CFileInfoArray::AP_COMPARE_FILENAME_ONLY );
	} else {
		// disable library support
		m_nLibFileCount = 0;
	}
	 
	// rescan the local drive
	ScanLocalDrive();
	if( !bSetupListBox ) {
		// we only need to do this the very 1st time we run
		DeleteAnyTempPasmFiles();
	}
	DeleteLocalFilesListedInTheDeleteListFile( FALSE, sFilesDeleted );
	RemoveBogusFiles( m_LocalFiles, TRUE, FALSE, TRUE, TRUE, TRUE, nNumTooLong, nNumZeroByte, nNumDups );
	if( nNumDups || nNumTooLong || nNumZeroByte ) {
		bPromptToViewErrorLog = TRUE;
		
		if( nNumDups == 1 ) {
			sOne.Format( "There is a duplicate file in the local path.\n" );
		} else if( nNumDups > 1 ) {
			sOne.Format( "There are %d duplicate files in the local path.\n", nNumDups );
		}
		if( nNumTooLong == 1 ) {
			sTwo.Format( "There is a filename in the local path which is too long.\n" );
		} else if( nNumTooLong > 1 ) {
			sTwo.Format( "There are %d filenames in the local path that are too long.\n", nNumTooLong );
		}
		if( nNumZeroByte == 1 ) {
			sThree.Format( "There is a zero byte file in the local path.\n" );
		} else if( nNumZeroByte > 1 ) {
			sThree.Format( "There are %d zero byte files in the local path.\n", nNumZeroByte );
		}		
		sMsg += sOne;
		sMsg += sTwo;
		sMsg += sThree;		
	}
	m_nLocalFileCount = m_LocalFiles.GetSize();
	CFileInfoArray::BuildMapFromFileInfoArray( m_MapLocalFiles, m_LocalFiles, 
											   CFileInfoArray::AP_IGNORE_CASE | CFileInfoArray::AP_COMPARE_FILENAME_ONLY );

	// prompt the user about lib & loc file errors
	if( bPromptToViewErrorLog ) {
		sMsg += "Open the error log file?";

		if( this->MessageBox( sMsg, "PASM ERROR", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			OnClickOpenErrorLog();
		}
	}

	// rescan the master file 
	// will also update the status bars stats
	// (ALWAYS DO THIS LAST!!!)
	ReadMasterFileEntries();
	if( DeleteMasterFilesListedInTheDeleteListFile( TRUE, sFilesDeleted ) ) {
		m_nMasterFileCount = m_MasterFile.GetNumEntries();
		UpdateStatusBarFileCounts();
	}

	if( bSetupListBox ) {
		UpdateFileListBox( TRUE );
	}
	EndWaitCursor();

	UpdateData( VARS_TO_CONTROLS );
}

void CPasmDlg::DeleteSelectedFiles( BOOL bAutoDeleteLocFiles/*=FALSE*/, BOOL bAutoDeleteMasEntries/*=FALSE*/ ) {
	POSITION Pos;
	CMyItemInfo *pItemInfo;
	int nIndex, nReturn;
	CDeleteDlg DeleteDlg;
	CStringList asMasterFileEntriesToDelete, asLibFilesToDelete, asLocFilesToDelete;
	u32 nFilesDeleted = 0;
	CString sEntry;
	BOOL bLibChecked, bLocChecked, bMasterChecked, bDeleteAll;
	FDataPrjFile_Entry_t *pEntry;

	UpdateData( CONTROLS_TO_VARS );
	
	if( m_ctrlFileList.GetSelectedCount() <= 0 ) {
		// nothing selected
		return;
	}

	asMasterFileEntriesToDelete.RemoveAll();
	asLibFilesToDelete.RemoveAll();
	asLocFilesToDelete.RemoveAll();

	bLibChecked = FALSE;
	bMasterChecked = bAutoDeleteMasEntries;
	bLocChecked = bAutoDeleteLocFiles;
	bDeleteAll = (bAutoDeleteLocFiles || bAutoDeleteMasEntries);
	
	BeginWaitCursor();

	Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	while( Pos ) {
		nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
		pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
		FASSERT( pItemInfo );

		if( !bDeleteAll ) {
			// the user is still going 1 file at a time
			DeleteDlg.m_pLibFileInfo = pItemInfo->GetLibFileInfo();
			DeleteDlg.m_pLocFileInfo = pItemInfo->GetLocFileInfo();
			pEntry = pItemInfo->GetMasterFileEntry();
			DeleteDlg.m_pszEntryName = ( pEntry ) ? pEntry->szFilename : NULL;
			DeleteDlg.m_bLibStartCondition = bLibChecked;
			DeleteDlg.m_bLocStartCondition = bLocChecked;
			DeleteDlg.m_bMasterStartCondition = bMasterChecked;

			nReturn = DeleteDlg.DoModal();
			if( nReturn == IDOK ) {
				if( DeleteDlg.m_bDeleteMasterFile ) {
					asMasterFileEntriesToDelete.AddTail( pEntry->szFilename );	
				}
				if( DeleteDlg.m_bDeleteLibFile ) {
					asLibFilesToDelete.AddTail( DeleteDlg.m_pLibFileInfo->GetFilePath() );
				}
				if( DeleteDlg.m_bDeleteLocFile ) {
					asLocFilesToDelete.AddTail( DeleteDlg.m_pLocFileInfo->GetFilePath() );
				}
			} else if( nReturn == IDCANCEL ) {
				// we won't delete anything if the user cancels
				EndWaitCursor();
				return;
			}

			// cache the checkbox settings for the next file
			bLibChecked = DeleteDlg.m_bDeleteFromLib;
			bLocChecked = DeleteDlg.m_bDeleteFromLoc;
			bMasterChecked = DeleteDlg.m_bDeleteFromMaster;
			bDeleteAll = DeleteDlg.m_bDeleteAll;			
		} else {
			// grab our pointers
			DeleteDlg.m_pLibFileInfo = pItemInfo->GetLibFileInfo();
			DeleteDlg.m_pLocFileInfo = pItemInfo->GetLocFileInfo();
			pEntry = pItemInfo->GetMasterFileEntry();

			if( bLibChecked && DeleteDlg.m_pLibFileInfo ) {
				asLibFilesToDelete.AddTail( DeleteDlg.m_pLibFileInfo->GetFilePath() );	
			}
			if( bLocChecked && DeleteDlg.m_pLocFileInfo ) {
				asLocFilesToDelete.AddTail( DeleteDlg.m_pLocFileInfo->GetFilePath() );	
			}
			if( bMasterChecked && pEntry ) {
				asMasterFileEntriesToDelete.AddTail( pEntry->szFilename );	
			}
		}
	}
	
	// delete any master file entries
	if( asMasterFileEntriesToDelete.GetCount() > 0 ) {
		
		// we need to open the master file so that we can delete files from it
		m_MasterFile.ReadHeaderAndEntries( m_sCurrentMasterFilename, FALSE, FALSE );

		for( Pos = asMasterFileEntriesToDelete.GetHeadPosition(); Pos != NULL; ) {
			sEntry = asMasterFileEntriesToDelete.GetNext( Pos );
			m_MasterFile.DeleteEntry( sEntry );
			nFilesDeleted++;
		}
			
		// close the master file, writes out everything to disk and closes the file
		if( !m_MasterFile.EndCompilation() ) {
			this->MessageBox( _T("Error closing the master file after deletion."), 
							  _T("PASM ERROR"),
							  MB_ICONHAND | MB_OK | MB_DEFBUTTON1 );
		}
	}
	
	CShellFileOp ShellFileOp;
	BOOL bAPICalled, bAborted;
	int  nAPIReturn;
	
	// delete any local files
	if( asLocFilesToDelete.GetCount() > 0 ) {
		ShellFileOp.Reset();
		if( !bAutoDeleteLocFiles ) {
			ShellFileOp.SetOperationFlags( FO_DELETE, this, FOF_FILESONLY );
		} else {
			// we are auto deleting, only display a simple progress bar and auto answering yes to any dialogs
			ShellFileOp.SetOperationFlags( FO_DELETE, this, FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS );			
		}
		ShellFileOp.SetProgressDlgTitle( "Deleting local files..." );
		bAPICalled = FALSE, bAborted = FALSE;
		nAPIReturn = 0;
		nIndex = 0;

		for( Pos = asLocFilesToDelete.GetHeadPosition(); Pos != NULL; ) {
			sEntry = asLocFilesToDelete.GetNext( Pos );
			ShellFileOp.AddSourceFile( sEntry );
			nIndex++;			
		}

		this->EnableWindow( FALSE );
		if( !ShellFileOp.Go( &bAPICalled, &nAPIReturn, &bAborted ) ) {
			if( !bAborted ) {
				CString sError;
				sError.Format( "Error (%d) occured while deleting local files.", nAPIReturn );
				this->MessageBox( sError, 
								  "PASM File Copy Error",
								  MB_OK | MB_ICONSTOP );	
			}
		}
		this->SetForegroundWindow();
		if( bAPICalled ) {
			// files got deleted
			nFilesDeleted += nIndex;
		}
		this->EnableWindow( TRUE );
	}

	// delete any lib files
	nIndex = asLibFilesToDelete.GetCount();
	if( nIndex > 0 ) {
		// warn about library deletions
		sEntry.Format( "You are about to delete %d file(s) from \nthe LIBRARY.  Are you ABSOLUTELY\nsure you want to delete from the \nLIBRARY?", nIndex );
		if( this->MessageBox( sEntry, 
							  _T("PASM: Confirm File Deletion"),
							  MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2 ) == IDYES ) {
			// the user gave us the go ahead
			ShellFileOp.Reset();
			ShellFileOp.SetOperationFlags( FO_DELETE, this, FOF_FILESONLY );
			ShellFileOp.SetProgressDlgTitle( "Deleting library files..." );
			bAPICalled = FALSE, bAborted = FALSE;
			nAPIReturn = 0;
			nIndex = 0;

			for( Pos = asLibFilesToDelete.GetHeadPosition(); Pos != NULL; ) {
				sEntry = asLibFilesToDelete.GetNext( Pos );
				ShellFileOp.AddSourceFile( sEntry );
				nIndex++;			
			}

			this->EnableWindow( FALSE );

			if( m_FileLock.WaitToLockFile( this ) ) {
				if( !ShellFileOp.Go( &bAPICalled, &nAPIReturn, &bAborted ) ) {
					if( !bAborted ) {
						CString sError;
						sError.Format( "Error (%d) occured while deleting local files.", nAPIReturn );
						this->MessageBox( sError, 
										  "PASM File Copy Error",
										  MB_OK | MB_ICONSTOP );	
					}
				}
				m_FileLock.UnlockFile();
			} else {
				UserQuitWaitingForLibraryLock( FALSE );
				nFilesDeleted++;// to force a rescan
			}
			this->SetForegroundWindow();
			if( bAPICalled ) {
				// files got deleted
				nFilesDeleted += nIndex;
			}
			this->EnableWindow( TRUE );
		}
	}
	
	if( nFilesDeleted ) {
		RescanAll( TRUE );
	}

	EndWaitCursor();

	UpdateData( VARS_TO_CONTROLS );	
}

void CPasmDlg::OnDelete() {
	DeleteSelectedFiles( FALSE, FALSE );	
}

CFileInfo *CPasmDlg::GetAidFileInfo( const CFileInfo *pFileInfo, BOOL bLocalFile ) {
	CString sName;
	void *pLookup;

	sName.Format( "%s.aid", pFileInfo->GetFileTitle() );
	sName.MakeLower();
	if( bLocalFile ) {
		// search the local drive
		if( m_MapLocalFiles.Lookup( sName, pLookup ) ) {
			return (CFileInfo *)pLookup;
		}
	} else {
		// search the library drive
		if( m_MapLibFiles.Lookup( sName, pLookup ) ) {
			return (CFileInfo *)pLookup;
		}	
	}
	return NULL;
}

BOOL CPasmDlg::IsFilenameALightmap( const CString &rsFilename ) {

	if( rsFilename.GetLength() < 3 ) {
		// all lightmaps have a length of at least 3 characters
		return FALSE;
	}
	if( rsFilename[0] >= '0' && rsFilename[0] <= '9' &&
		rsFilename[1] >= '0' && rsFilename[1] <= '9' &&
		rsFilename[2] >= '0' && rsFilename[2] <= '9' ) {
		return TRUE;
	}
	return FALSE;
}

BOOL CPasmDlg::IsFilenameALightmap( cchar *pszFilename ) {

	if( fclib_strlen( pszFilename ) < 3 ) {
		// all lightmaps have a length of at least 3 characters
		return FALSE;
	}
	if( pszFilename[0] >= '0' && pszFilename[0] <= '9' &&
		pszFilename[1] >= '0' && pszFilename[1] <= '9' &&
		pszFilename[2] >= '0' && pszFilename[2] <= '9' ) {
		return TRUE;
	}
	return FALSE;
}

void CPasmDlg::OnGenerateFileTypeUsageFile() {

	UpdateData( CONTROLS_TO_VARS );
	
	CStringInput StringInput;
	CString sExtToLog;
	CString sDefault = ".sfb";
	
	if( StringInput.GetUserString( sDefault, "What To Log?", "Enter a file extension to log, include a period", sExtToLog ) ) {
		sExtToLog.MakeLower();
		DEVPRINTF( "searching load log for files of type '%s'...\n", sExtToLog );
		GenerateFileTypeUsageFile( sExtToLog );	
		UpdateData( VARS_TO_CONTROLS );
	}
}

BOOL CPasmDlg::GenerateFileTypeUsageFile( cchar *pszExtensionWithPeriod ) {
	CString sExt, sLogFile, sTemp;

	UpdateData( CONTROLS_TO_VARS );

	// have the user select a log file 
	sExt = "Text Files (*.txt)|*.txt|Log Files (*.log)|*.log|All Files (*.*)|*.*||";	
	sTemp = m_sCurrentMasterFilename;
	sTemp += ".txt";
	CFileDialog LogFileDlg( TRUE, NULL, sTemp,
							OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
							(cchar *)sExt, this );
	LogFileDlg.m_ofn.lpstrTitle = "Select FANG Log File";
	if( LogFileDlg.DoModal() == IDOK ) {
		sLogFile = LogFileDlg.GetPathName();		
	} else {
		MessageBox( _T("Must select a log file before a\nfile type listing can be generated."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;
	}

	DEVPRINTF( "searching load log '%s'...\n", sLogFile );

	// read the log file into memory
	FILE *pLogFile = fopen( sLogFile, "rb" );
	if( !pLogFile ) {
		MessageBox( _T("Could not open the log file for reading."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;
	}
	fseek( pLogFile, 0, SEEK_END );
	int nLogFileBytes = ftell( pLogFile );
	fseek( pLogFile, 0, SEEK_SET );
	char *pFileBuffer = new char[nLogFileBytes];
	if( !pFileBuffer ) {
		fclose( pLogFile );
		MessageBox( _T("Could not allocate enough\nmemory to hold the log file."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;	
	}
	if( fread( pFileBuffer, nLogFileBytes, 1, pLogFile ) != 1 ) {
		delete [] pFileBuffer;
		fclose( pLogFile );
		MessageBox( _T("Could not read the log file."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;	
	}      
	fclose( pLogFile );

	// create a CStringList of the filenames from the log file
	CStringList asFilenames;
	char *pszLine;
	int i, nLen;

	// replace all CR/LF with NULL
	for( i=0; i < nLogFileBytes; i++ ) {
		if( pFileBuffer[i] == 0x1A ||
			pFileBuffer[i] == 0xA ||
			pFileBuffer[i] == 0xD ) {
			pFileBuffer[i] = 0;
		}
	}

	asFilenames.RemoveAll();

	sExt = pszExtensionWithPeriod;
	sExt.MakeLower();
	
	pszLine = pFileBuffer;
	i = 0;
	while( i < nLogFileBytes ) {
		sTemp = pszLine;	
		if( i == 0 ) {
			// to validate this log file, make sure that the first line starts with:
			// #FANG***
			if( sTemp.Find( "#FANG***" ) != 0 ) {
				// not a valid fang log file
				delete [] pFileBuffer;
				MessageBox( _T("The log file does not appear\nto be a valid log file." ), 
							_T("PASM ERROR"),
							MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
				return FALSE;
			}
		}
		nLen = sTemp.GetLength();
		if( nLen > 0 ) {
			if( sTemp[0] == '#' ) {
				// log all comment lines
				asFilenames.AddTail( sTemp );
			} else { 
				// not a comment line
				sTemp.MakeLower();

				if( sTemp.Find( sExt ) >= 0 ) {
					asFilenames.AddTail( sTemp );		
				}
			}
			i += nLen;
		} else {
			i++;
		}
		pszLine = &pFileBuffer[i];
	}
	
	// free our allocated memory
	delete [] pFileBuffer;

	DEVPRINTF( "found %d files in load log...\n", asFilenames.GetCount() );

	if( asFilenames.GetCount() <= 0 ) {
		// we didn't find any valid names, nothing to optimize
		MessageBox( _T("The log file does not contain any\nvalid lines, nothing to generated." ), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;
	}

	// open the output file
	sTemp.Format( "%s\\log of used %s files.txt", m_sXBMasterFileDir, pszExtensionWithPeriod );
	pLogFile = fopen( sTemp, "wt" );
	if( !pLogFile ) {
		return FALSE;
	}

	POSITION Pos = asFilenames.GetHeadPosition();
	while( Pos ) {
		sTemp = asFilenames.GetNext( Pos );
        fprintf( pLogFile, "%s\n", sTemp );
	}
	fclose( pLogFile );

	return TRUE;
}

void CPasmDlg::OnOptimizeMasterfile() {
	OptimizeMasterFile( FALSE, FALSE );
}

void CPasmDlg::OptimizeMasterFile( BOOL bLoggedAssetOnly, BOOL bInternationalOptimize ) {
	CString sExt, sLogFile, sTemp;
	
	if ( !CheckMasterFileCompilerVersion() ) {
		return;
	}

	UpdateData( CONTROLS_TO_VARS );

	// warn about the time this might take
	if( this->MessageBox( _T( "Optimizing the master file\ncan be VERY time consuming,\nproceed?" ),
						  _T( "PASM Warning" ),
						  MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1 ) == IDNO ) {
		return;
	}

	// have the user select a log file 
	sExt = "Text Files (*.txt)|*.txt|Log Files (*.log)|*.log|All Files (*.*)|*.*||";	
	sTemp = m_sCurrentMasterFilename;
	sTemp += ".txt";
	CFileDialog LogFileDlg( TRUE, NULL, sTemp,
							OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
							(cchar *)sExt, this );
	LogFileDlg.m_ofn.lpstrTitle = "Select FANG Log File";
	if( LogFileDlg.DoModal() == IDOK ) {
		sLogFile = LogFileDlg.GetPathName();		
	} else {
		MessageBox( _T("Must select a log file before the\nmaster file can be optimized."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}

	DEVPRINTF( "optimizing master file using load log '%s'...\n", sLogFile );

	BeginWaitCursor();

	CStringList asFilenames;
	if( !CreateUniqueListOfLogggedFiles( sLogFile, asFilenames, bInternationalOptimize ) ) {
		EndWaitCursor();
		MessageBox( _T("Trouble reading the log file and creating a file list, the masterfile was not optimized."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}	

	if( asFilenames.GetCount() <= 0 ) {
		// we didn't find any valid names, nothing to optimize
		EndWaitCursor();
		MessageBox( _T("The log file does not contain any\nvalid lines, nothing to optimize." ), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}

	DEVPRINTF( "total file count to optimize masterfile with is %d...\n", asFilenames.GetCount() );

	if( !m_MasterFile.RearrangeMasterFile( m_sCurrentMasterFilename, &asFilenames, bLoggedAssetOnly ) ) {
		EndWaitCursor();
		// problem optimizing the master file
		MessageBox( _T("There was a problem during the optimization process.\nThe master file has not been optimzied." ), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}
	ReadMasterFileEntries();
	UpdateFileListBox( TRUE );
	EndWaitCursor();

	UpdateData( VARS_TO_CONTROLS );
}

// rsExt should be in the following format: '.ext;'
// in all lowercase.
BOOL CPasmDlg::IsExtInIgnoreList( const CString &rsExt ) {

	if( rsExt.IsEmpty() ) {
		// if there is no extension, then return FALSE
		return FALSE;
	}
	
	if( m_sIgnoreExtensions.Find( rsExt ) >= 0 ) {
		return TRUE;
	}

	// do our platform specific tests here
	if( m_bXBoxFormat || m_bPCFormat ) {
		// ignore other platform's raw data files
		if( rsExt.Find( ".rdg" ) >= 0 ) {
			return TRUE;
		}
	} else if( m_bGCFormat ) {
		// ignore other platform's raw data files
		if( rsExt.Find( ".rdx" ) >= 0 ) {
			return TRUE;
		}
		// ignore wav banks file
		if( rsExt.Find( ".wvb" ) >= 0 ) {
			return TRUE;
		}		
	}

	return FALSE;
}

// allows double clicks to open up the file (tries the local first, then the library)
void CPasmDlg::OnDblclkFileList(NMHDR* pNMHDR, LRESULT* pResult) {
	
	UpdateData( CONTROLS_TO_VARS );

	CPoint Point;
	::GetCursorPos( &Point );
	m_ctrlFileList.ScreenToClient( &Point );
	
	int nIndex = m_ctrlFileList.HitTest( Point );
	if( nIndex >= 0 ) {
		CMyItemInfo *pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
		FASSERT( pItemInfo );
		CFileInfo *pFileInfo = pItemInfo->GetLocFileInfo();
		if( !pFileInfo ) {
			// no local file, try the library file
			pFileInfo = pItemInfo->GetLibFileInfo();
		}
		if( pFileInfo ) {
			ShellExecute( NULL, "open", pFileInfo->GetFilePath(), NULL, NULL, SW_SHOWNORMAL );
		}
	}
	if( m_ctrlFileList.GetSelectedCount() != m_nLastSelectedFilesCount ) {
		UpdateStatusBarFileCounts();
	}
	
	*pResult = 0;
}

void CPasmDlg::OnClickFileList(NMHDR* pNMHDR, LRESULT* pResult) {
	
	UpdateData( CONTROLS_TO_VARS );
	
	if( m_ctrlFileList.GetSelectedCount() != m_nLastSelectedFilesCount ) {
		UpdateStatusBarFileCounts();
	}

	*pResult = 0;
}

void CPasmDlg::OnItemchangedFileList(NMHDR* pNMHDR, LRESULT* pResult) {
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_ctrlFileList.GetSelectedCount() != m_nLastSelectedFilesCount ) {
		UpdateStatusBarFileCounts();
	}
	
	*pResult = 0;
}

void CPasmDlg::OnScanAndCompile() {
	
	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}
	RescanAll( TRUE );
	SelectNewAndNewer( LOCAL, MASTER );
	Compile( FALSE );
}

long CPasmDlg::OnExternalAppRequestingRescanAndCompile( WPARAM wParam, LPARAM lParam ) {
	
	if ( !CheckMasterFileCompilerVersion() )
	{
		return 0;
	}

	// cache and disable the hold compile stats var
	BOOL bCacheHoldCompileStats = m_bHoldCompileStats;
	m_bHoldCompileStats = FALSE;

	// do the same thing as clicking rescan & compile
	OnScanAndCompile();

	// send a message back to the app launcher
	CWnd *pPrevWnd = CWnd::GetDesktopWindow()->GetWindow( GW_CHILD );
	while( pPrevWnd ) {
		if( ::GetProp( pPrevWnd->GetSafeHwnd(), APP_NAMES_APE_APP_LAUNCHER )) {
//			SwitchToThisWindow( pPrevWnd->m_hWnd, TRUE );
			
			// post a message to the launcher
			pPrevWnd->PostMessage( APP_MSG_CODES_LAUNCHER_PASM_DONE );
		}
		// grab the next window
		pPrevWnd = pPrevWnd->GetWindow( GW_HWNDNEXT );
	}

	// Clear the inter process data flag
	Pasm_SharedAppData->aAppData[APP_TYPE_PASM].bDataInUse = FALSE;

	// restore the cached vars
	m_bHoldCompileStats = bCacheHoldCompileStats;

	// minimize PASM
	ShowWindow( SW_MINIMIZE );

	return 0;
}

// make sure that rsStr is lower case and ends with a ';'
void CPasmDlg::FixIgnoreExtString( CString &rsStr ) {
	
	if( rsStr.IsEmpty() ) {
		// nothing to do
		return;
	}
	// make lowercase
	rsStr.MakeLower();
	// add a semi-colon to the end of the string
	char c = rsStr.GetAt( rsStr.GetLength() - 1 );
	if( c != ';' ) {
		rsStr += ';';
	}
	// replace some common mistakes with a semi-colon
	rsStr.Replace( ',', ';' );
	rsStr.Replace( ':', ';' );

	// make sure that the ignore string has *.inc
	if( rsStr.Find( "*.inc" ) < 0 ) {
		rsStr += "*.inc;";
	}

	// make sure that the ignore string has *.scc
	if( rsStr.Find( "*.scc" ) < 0 ) {
		rsStr += "*.scc;";
	}

	// make sure that the ignore string has *.mat
	if( rsStr.Find( "*.mat" ) < 0 ) {
		rsStr += "*.mat;";
	}

	// make sure that the ignore string has *.kng
	if( rsStr.Find( "*.kng" ) < 0 ) {
		rsStr += "*.kng;";
	}

	// make sure that the ignore string has *.vis
	if( rsStr.Find( "*.vis" ) < 0 ) {
		rsStr += "*.vis;";
	}

	// make sure that the ignore string has *.rmp
	if( rsStr.Find( "*.rmp" ) < 0 ) {
		rsStr += "*.rmp;";
	}

	// make sure that the ignore string has *.bat
	if( rsStr.Find( "*.bat" ) < 0 ) {
		rsStr += "*.bat;";
	}
}

void CPasmDlg::OnRclickFileList(NMHDR* pNMHDR, LRESULT* pResult) {
	
	UpdateData( CONTROLS_TO_VARS );

	// get the list box's screen rect
	CPoint Point;
	::GetCursorPos( &Point ); 
	m_ctrlFileList.ScreenToClient( &Point );
	
	// select the item under the mouse cursor
	int nIndex = m_ctrlFileList.HitTest( Point );
	if( nIndex >= 0 ) {
		m_ctrlFileList.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED );
	}
	// create the popup menu
	CMenu menu;
	menu.LoadMenu( IDR_POPUP_MENU );
	CMenu *pContextMenu = menu.GetSubMenu( 0 ); 
	m_ctrlFileList.ClientToScreen( &Point );

	// check some of the menu entries
	::CheckMenuItem( pContextMenu->m_hMenu, ID_PLATFORM_XBOX, m_bXBoxFormat ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( pContextMenu->m_hMenu, ID_PLATFORM_PC, m_bPCFormat ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( pContextMenu->m_hMenu, ID_PLATFORM_GC, m_bGCFormat ? MF_CHECKED : MF_UNCHECKED );	 
#ifdef _MMI_TARGET_PS2
// NS
    ::CheckMenuItem( pContextMenu->m_hMenu, ID_PLATFORM_PS2, m_bPS2Format ? MF_CHECKED : MF_UNCHECKED );
#endif
	::CheckMenuItem( pContextMenu->m_hMenu, ID_VIEW_SHOWDIFFERENCESONLY, m_bShowDifferences ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( pContextMenu->m_hMenu, ID_VIEW_HOLD_COMPILE_STATS, m_bHoldCompileStats ? MF_CHECKED : MF_UNCHECKED );
	::CheckMenuItem( pContextMenu->m_hMenu, ID_VIEW_HIDE_NON_COMPILABLE_FILES, m_bHideNonCompliableFiles ? MF_CHECKED : MF_UNCHECKED );

	// disable some of the menu entries
#if FANG_WINGC
	::EnableMenuItem( pContextMenu->m_hMenu, ID_PLATFORM_XBOX, MF_GRAYED );
	::EnableMenuItem( pContextMenu->m_hMenu, ID_PLATFORM_PC, MF_GRAYED );
#else
	::EnableMenuItem( pContextMenu->m_hMenu, ID_PLATFORM_GC, MF_GRAYED );
#endif

	// display the popup menu
	pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this );
	
	*pResult = 0;
}

// move the selected files: library -> local
void CPasmDlg::CopySelectedLibraryFiles( BOOL bNoPrompts/*=FALSE*/ ) {
	POSITION Pos;
	int nIndex;
	CMyItemInfo *pItemInfo;
	CFileInfo *pLibFileInfo, *pLocFileInfo;
	CString sFileSrcLocation, sFileDestLocation, sLowerCaseLibDir, sLowerCaseLocDir;
	CShellFileOp ShellFileOp;
	u32 nNumFilesToCopy = 0;
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_bDisableLibrarySupport ) {
		// no library support currently
		return;
	}
	if( m_ctrlFileList.GetSelectedCount() <= 0 ) {
		// nothing selected
		return;
	}
	
	// save our settings before starting a long process
	SaveSettingsOutToDisk( FALSE, TRUE );
	
	sLowerCaseLibDir = m_sLibDir;
	sLowerCaseLibDir.MakeLower();

	sLowerCaseLocDir = m_sLocalDir;
	sLowerCaseLocDir.MakeLower();

	if( !bNoPrompts ) {
		ShellFileOp.SetOperationFlags( FO_COPY, this, FOF_FILESONLY | FOF_NOCONFIRMMKDIR | FOF_MULTIDESTFILES );
	} else {
		ShellFileOp.SetOperationFlags( FO_COPY, this, FOF_FILESONLY | FOF_NOCONFIRMMKDIR | FOF_MULTIDESTFILES | FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS );
	}
	ShellFileOp.SetProgressDlgTitle( "Copying from library..." );

	Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	while( Pos ) {
		nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
		pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
		FASSERT( pItemInfo );
		
		pLibFileInfo = pItemInfo->GetLibFileInfo();
		if( pLibFileInfo ) {
			// grab the file source location
			sFileSrcLocation = pLibFileInfo->GetFilePath();
			sFileSrcLocation.MakeLower();

			// see if we will be copying over an existing file (if so make sure to handle the paths not matching)
			pLocFileInfo = pItemInfo->GetLocFileInfo();
			if( pLocFileInfo ) {
				// see if the files have different modified times
				if( AreTimesEqual( pLocFileInfo->GetLastWriteTime(), pLibFileInfo->GetLastWriteTime() ) ) {
					// these 2 files don't need to be copied, they are the same already
					continue;
				}
				// keep the local file location
				sFileDestLocation = pLocFileInfo->GetFilePath();
				sFileDestLocation.MakeLower();
			} else {
				// create a local file destination from the source's location
				sFileDestLocation = sFileSrcLocation;

				// remove the library directory
				nIndex = sFileDestLocation.Find( sLowerCaseLibDir );
				FASSERT( nIndex >= 0 );

				sFileDestLocation.Delete( nIndex, sLowerCaseLibDir.GetLength() );

				sFileDestLocation.Insert( 0, sLowerCaseLocDir );
			}
			
			nNumFilesToCopy++;
			ShellFileOp.AddDestFile( sFileDestLocation );
			ShellFileOp.AddSourceFile( sFileSrcLocation );
		}
	}

	if( nNumFilesToCopy ) {
		// prepare to copy the files, disable keyboard & mouse input
		this->EnableWindow( FALSE );
		// copy the files
		BOOL bAPICalled = FALSE, bAborted = FALSE;
		int  nAPIReturn = 0;
		// get a lock on the library
		if( m_FileLock.WaitToLockFile( this ) ) {
			if( !ShellFileOp.Go( &bAPICalled, &nAPIReturn, &bAborted ) ) {
				if( !bAborted ) {
					CString sError;
					sError.Format( "Error (%d) occured while copying files from the library.", nAPIReturn );
					this->MessageBox( sError, 
									  "PASM File Copy Error",
									  MB_OK | MB_ICONSTOP );	
				}
			}
			m_FileLock.UnlockFile();
		} else {
			UserQuitWaitingForLibraryLock( FALSE );
			bAPICalled = TRUE;// to force a rescan
		}
		this->SetForegroundWindow();
		if( bAPICalled ) {
			// files got copied, we need to rescan
			RescanAll( TRUE );
		}
		this->EnableWindow( TRUE );		
	}
}

// move files: library -> local 
void CPasmDlg::OnSyncDownClick() {
	CopySelectedLibraryFiles( FALSE );		
}

// move files: local -> library
void CPasmDlg::OnSyncUpClick() {
	POSITION Pos;
	int nIndex;
	CMyItemInfo *pItemInfo;
	CFileInfo *pLibFileInfo, *pLocFileInfo;
	CString sFileSrcLocation, sFileDestLocation, sLowerCaseLibDir, sLowerCaseLocDir;
	CShellFileOp ShellFileOp;
	u32 nNumFilesToCopy = 0;
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_bDisableLibrarySupport ) {
		// no library support currently
		return;
	}
	if( m_ctrlFileList.GetSelectedCount() <= 0 ) {
		// nothing selected
		return;
	}
	
	// save our settings before starting a long process
	SaveSettingsOutToDisk( FALSE, TRUE );
	
	sLowerCaseLibDir = m_sLibDir;
	sLowerCaseLibDir.MakeLower();

	sLowerCaseLocDir = m_sLocalDir;
	sLowerCaseLocDir.MakeLower();

	ShellFileOp.SetOperationFlags( FO_COPY, this, FOF_FILESONLY | FOF_NOCONFIRMMKDIR | FOF_MULTIDESTFILES );
	ShellFileOp.SetProgressDlgTitle( "Copying to library..." );

	Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	while( Pos ) {
		nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
		pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
		FASSERT( pItemInfo );
		
		pLocFileInfo = pItemInfo->GetLocFileInfo();
		if( pLocFileInfo ) {
			// grab the file source location
			sFileSrcLocation = pLocFileInfo->GetFilePath();
			sFileSrcLocation.MakeLower();

			// see if we will be copying over an existing file (if so make sure to handle the paths not matching)
			pLibFileInfo = pItemInfo->GetLibFileInfo();
			if( pLibFileInfo ) {
				// see if the files have different modified times
				if( AreTimesEqual( pLocFileInfo->GetLastWriteTime(), pLibFileInfo->GetLastWriteTime() ) ) {
					// these 2 files don't need to be copied, they are the same already
					continue;
				}
				// keep the lib file location 
				sFileDestLocation = pLibFileInfo->GetFilePath();
				sFileDestLocation.MakeLower();
			} else {
				// create a lib file destination from the source's location
				sFileDestLocation = sFileSrcLocation;

				// remove the local directory
				nIndex = sFileDestLocation.Find( sLowerCaseLocDir );
				FASSERT( nIndex >= 0 );

				sFileDestLocation.Delete( nIndex, sLowerCaseLocDir.GetLength() );

				sFileDestLocation.Insert( 0, sLowerCaseLibDir );
			}
			
			nNumFilesToCopy++;
			ShellFileOp.AddDestFile( sFileDestLocation );
			ShellFileOp.AddSourceFile( sFileSrcLocation );
		}
	}

	if( nNumFilesToCopy ) {
		// prepare to copy the files, disable keyboard & mouse input
		this->EnableWindow( FALSE );
		// copy the files
		BOOL bAPICalled = FALSE, bAborted = FALSE;
		int  nAPIReturn = 0;
		// get a lock on the library
		if( m_FileLock.WaitToLockFile( this ) ) {
			if( !ShellFileOp.Go( &bAPICalled, &nAPIReturn, &bAborted ) ) {
				if( !bAborted ) {
					CString sError;
					sError.Format( "Error (%d) occured while copying files from the local path.", nAPIReturn );
					this->MessageBox( sError, 
									  "PASM File Copy Error",
									  MB_OK | MB_ICONSTOP );	
				}
			}
			m_FileLock.UnlockFile();
		} else {
			UserQuitWaitingForLibraryLock( FALSE );
			bAPICalled = TRUE;// to force a rescan
		}
		this->SetForegroundWindow();
		if( bAPICalled ) {
			// files got copied, we need to rescan
			RescanAll( TRUE );
		}
		this->EnableWindow( TRUE );		
	}	
}

void CPasmDlg::OnSyncDownAndCompile() {
	
	RescanAll( TRUE );

	SelectNewAndNewer( LIBRARY, LOCAL );
	UpdateData( VARS_TO_CONTROLS );	
	
	CopySelectedLibraryFiles( FALSE );

	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}

	UpdateData( CONTROLS_TO_VARS );
	
	SelectNewAndNewer( LOCAL, MASTER );
	UpdateData( VARS_TO_CONTROLS );	
	
	Compile( FALSE );	
}

void CPasmDlg::OnMirrorLibrary() {

	UpdateData( CONTROLS_TO_VARS );

	if( m_bDisableLibrarySupport ) {
		// no library support currently
		return;
	}

	// warn the user
	if( this->MessageBox( _T("Making a local mirror of the library will DELETE all\nnew or newer local files.  This process make take \na long time, do you want to proceed anyway?"), 
						  _T("PASM Warning"),
						  MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2 ) == IDNO ) {
		return;
	}

	// cache some user settings
	BOOL bShowDiffs, bHideNonCompileableFiles, bHoldOnCompile;
	bShowDiffs = m_bShowDifferences;
	m_bShowDifferences = FALSE;
	bHideNonCompileableFiles = m_bHideNonCompliableFiles;
	m_bHideNonCompliableFiles = FALSE;
	bHoldOnCompile = m_bHoldCompileStats;
	m_bHoldCompileStats = FALSE;
	
	// rescan everything
	RescanAll( TRUE ); 

	// select all local files that are new/newer than the library
	SelectNewAndNewer( LOCAL, LIBRARY );
	UpdateData( VARS_TO_CONTROLS );

	// delete all selected files from the local drive
	DeleteSelectedFiles( TRUE, FALSE );

	// select all library files that are new/newer than the local
	SelectNewAndNewer( LIBRARY, LOCAL );
	UpdateData( VARS_TO_CONTROLS );

	// copy all selected files from the library drive
	CopySelectedLibraryFiles( TRUE );

	// set the settings so that we can select everything to compile
	m_bShowDifferences = TRUE;
	m_bHideNonCompliableFiles = TRUE; 

	RescanAll( TRUE );

	// select all shown files
	SelectAll();
	
	// compile the selected files
	Compile( FALSE );

	// the only thing left to do is to delete any master file entries for files that have been deleted
	SelectAll();

	DeleteSelectedFiles( FALSE, TRUE );

	// restore the cached settings
	m_bShowDifferences = bShowDiffs;
	m_bHideNonCompliableFiles = bHideNonCompileableFiles;
	m_bHoldCompileStats = bHoldOnCompile;
	
	// rescan everything
	RescanAll( TRUE );
}

BOOL CPasmDlg::AreTimesEqual( const CTime &rTime1, const CTime &rTime2 ) {
	CTimeSpan DeltaTime = rTime1 - rTime2;
	int nDeltaSecs = (int)DeltaTime.GetTotalSeconds();

	return ( nDeltaSecs <= 2 && nDeltaSecs >= -2 );
}

void CPasmDlg::OnClickDetailedFileStats() {

	UpdateData( CONTROLS_TO_VARS );

	int nIndex;
	CFileInfo *pLibFileInfo, *pLocFileInfo;
	FDataPrjFile_Entry_t *pEntry;

	// grab the first selected file
	POSITION Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	if( !Pos ) { 
		// no file is selected
		return;
	}
	nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );

	CMyItemInfo *pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
	FASSERT( pItemInfo );

	pLibFileInfo = pItemInfo->GetLibFileInfo();
	pLocFileInfo = pItemInfo->GetLocFileInfo();
	pEntry = pItemInfo->GetMasterFileEntry();
	
	CString s, sTemp;
	CTime ModifiedTime;

	s = "Library File:\n";
	if( pLibFileInfo ) {
		sTemp.Format( "\t%s\t\n", pLibFileInfo->GetFilePath() );
		s += sTemp;
		ModifiedTime = pLibFileInfo->GetLastWriteTime();
		sTemp.Format( "\t%s\t\n", ModifiedTime.Format( "%#c" ) );
		s += sTemp;
		sTemp.Format( "\t(%d bytes)\t\n\n", pLibFileInfo->GetLength() );
		s += sTemp;
	} else {
		s += "\tn/a\n\n";
	}

	s += "Local File:\n";
	if( pLocFileInfo ) {
		sTemp.Format( "\t%s\t\n", pLocFileInfo->GetFilePath() );
		s += sTemp;
		ModifiedTime = pLocFileInfo->GetLastWriteTime();
		sTemp.Format( "\t%s\t\n", ModifiedTime.Format( "%#c" ) );
		s += sTemp;
		sTemp.Format( "\t(%d bytes)\t\n\n", pLocFileInfo->GetLength() );
		s += sTemp;
	} else {
		s += "\tn/a\n\n";
	}
	
	s += "Master File Entry:\n";
	if( pEntry ) {
		sTemp.Format( "\t%s\t\n", pEntry->szFilename );
		s += sTemp;
		ModifiedTime = (time_t )pEntry->nModifiedTime;
		sTemp.Format( "\t%s\t\n", ModifiedTime.Format( "%#c" ) );
		s += sTemp;
		sTemp.Format( "\t(%d bytes)\t\n", pEntry->nNumBytes );
		s += sTemp;
	} else {
		s += "\tn/a\n";
	}

	this->MessageBox( s, _T("PASM: File Details"), MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1);
}

void CPasmDlg::OnClickOpenErrorLog() {
	ShellExecute( NULL, "open", CErrorLog::GetErrorFilename(), NULL, NULL, SW_SHOWNORMAL );	
}

void CPasmDlg::RemoveBogusFiles( CFileInfoArray &rFileInfoArray, BOOL bLogErrors, BOOL bTestExcludedExtensions,
								 BOOL bCheckForLengthyFilenames, BOOL bCheckForZeroByteFiles, BOOL bCheckForDuplicateFiles, 
								 u32 &rnNumLengthyFilenames, u32 &rnNumZeroByteFiles, u32 &rnNumDuplicateFiles ) {
	int i, nNumFileInfos, nFileLen, nTitleLen;
	const CFileInfo *pFileInfo, *pFileInfo2;
	const CFileInfo *pFileInfoData;
	CStringList asZeroLenFiles, asLengthyFilenames, asDuplicateFiles, asKeeperFiles;
	CString sExt, sFilename;
	BOOL bFilenameIsTooLong;
	CMapStringToOb MapNameToFileInfoPtr( (rFileInfoArray.GetSize() <= 5) ? 5 : rFileInfoArray.GetSize() );
	void *pLookup;
	
	asZeroLenFiles.RemoveAll();
	rnNumZeroByteFiles = 0;

	asLengthyFilenames.RemoveAll();
	rnNumLengthyFilenames = 0;

	asDuplicateFiles.RemoveAll();
	asKeeperFiles.RemoveAll();
	rnNumDuplicateFiles = 0;

	pFileInfoData = rFileInfoArray.GetData();
	nNumFileInfos = rFileInfoArray.GetSize();
	for( i=0; i < nNumFileInfos;  ) {
		pFileInfo = &pFileInfoData[i];
		sFilename = pFileInfo->GetFileName();

		// check for zero byte files
		if( bCheckForZeroByteFiles && pFileInfo->GetLength() <= 0 ) {
			if( bLogErrors ) {
				asZeroLenFiles.AddTail( pFileInfo->GetFilePath() );	
			}
			// remove the ith entry
			rFileInfoArray.RemoveAt( i, 1 );
			rnNumZeroByteFiles++;
			nNumFileInfos--;
			continue;
		}

		// check for excluded extensions
		if( !bTestExcludedExtensions ) {
			sExt = pFileInfo->GetFileExt();
			sExt.MakeLower();
			
			// make sure that the extension is unique and not confused with other file extensions
			// add a trailing ';' to sExt
			sExt += ';';

			if( IsExtInIgnoreList( sExt ) ) {
				// remove the ith entry
				rFileInfoArray.RemoveAt( i, 1 );
				nNumFileInfos--;
				continue;
			}
		}
		
		// check for lenghty filenames
		if( bCheckForLengthyFilenames ) {
			nFileLen = sFilename.GetLength();
			nTitleLen = pFileInfo->GetFileTitle().GetLength();
			bFilenameIsTooLong = ( nFileLen >= FDATA_PRJFILE_FILENAME_LEN || nTitleLen >= (FDATA_PRJFILE_FILENAME_LEN-4) );
			if( bFilenameIsTooLong ) {
				if( bLogErrors ) {
					asLengthyFilenames.AddTail( pFileInfo->GetFilePath() );
				}
				// remove the ith entry
				rFileInfoArray.RemoveAt( i, 1 );
				rnNumLengthyFilenames++;
				nNumFileInfos--;
				continue;
			}		
		}

		// check for duplicate filenames
		if( bCheckForDuplicateFiles ) {
			sFilename.MakeLower();

			// don't count any .scc file, there will be lots of duplicates
			if( sFilename.Find( ".scc" ) > 0 ) {
				rFileInfoArray.RemoveAt( i, 1 );
				nNumFileInfos--;
				continue;
			}

			if( MapNameToFileInfoPtr.Lookup( sFilename, (CObject *&)pLookup ) == 0 ) {
				// add an entry for sFilename
				MapNameToFileInfoPtr.SetAt( sFilename, (CObject *)pFileInfo );				
			} else {
				// we found a previous entry for sFilename
				pFileInfo2 = (CFileInfo *)pLookup;

				if( bLogErrors ) {
					asKeeperFiles.AddTail( pFileInfo2->GetFilePath() );
					asDuplicateFiles.AddTail( pFileInfo->GetFilePath() );
				}
				// remove the ith entry
				rFileInfoArray.RemoveAt( i, 1 );
				rnNumDuplicateFiles++;
				nNumFileInfos--;
				continue;
			}	
		}

		// advance to the next entry
		i++;
	}

	// log the errors to the logfile if requested
	if( bLogErrors ) {
		POSITION Pos, Pos2;
		CString s;

		CErrorLog &rErrorLog = CErrorLog::GetCurrent();

		if( rnNumZeroByteFiles ) {
			rErrorLog.WriteErrorHeader( "The Following Files Are Zero Byte: " );
			for( Pos = asZeroLenFiles.GetHeadPosition(); Pos != NULL; ) {
				s.Format( "Skipping %s", asZeroLenFiles.GetNext( Pos ) );
				rErrorLog.WriteErrorLine( s );
			}
		}
		
		if( rnNumLengthyFilenames ) {
			rErrorLog.WriteErrorHeader( "The Following Filenames Are Too Long: " );
			for( Pos = asLengthyFilenames.GetHeadPosition(); Pos != NULL; ) {
				s.Format( "Skipping %s", asLengthyFilenames.GetNext( Pos ) );
				rErrorLog.WriteErrorLine( s );
			}			
		}
		
		if( rnNumDuplicateFiles ) {
			FASSERT( asKeeperFiles.GetCount() == asDuplicateFiles.GetCount() );
			for( Pos2 = asKeeperFiles.GetHeadPosition(), Pos = asDuplicateFiles.GetHeadPosition(); Pos != NULL;  ) {
				s.Format( "Duplicate Files Found, keeping %s", asKeeperFiles.GetNext( Pos2 ) );	
				rErrorLog.WriteErrorHeader( s );
				
				s.Format( "Skipping %s", asDuplicateFiles.GetNext( Pos ) );
				rErrorLog.WriteErrorLine( s );
			}
		}
	}
}

void CPasmDlg::ReadDeleteListFile() {
	CStdioFile File;
	CString sFilename, sLine;
 
	m_asFilesToDelete.RemoveAll();

	sFilename = m_sLibLockDir;
	sFilename += _DELETE_LIST_FILENAME;

	if( !File.Open( sFilename, CFile::modeRead | 
							   CFile::typeText |
							   CFile::modeNoTruncate |
							   CFile::shareDenyNone )  ) {
		return;
	}

	while( File.ReadString( sLine ) ) {
		// see if sLine has any comment operators
		if( !sLine.IsEmpty() && sLine.Find( '#' ) < 0 ) {
			sLine.TrimLeft();
			sLine.TrimRight();
			if( sLine.Find( '\\' ) >= 0 ) {
				// a path was supplied, make sure that the first
				// character is a '\' and then append the user's
				// local path name to the string.
				if( sLine[0] != '\\' ) {
					sLine.Insert( 0, '\\' );
				}
				sLine.Insert( 0, m_sLocalDir );
			}
			if( !sLine.IsEmpty() ) {
				m_asFilesToDelete.AddTail( sLine );
			}
		}
	}

	CFileStatus FileStatus;
	File.GetStatus( FileStatus );
	m_DeleteListFileTime = FileStatus.m_mtime;

	File.Close();
}

// returns the number of local files actually deleted
u32 CPasmDlg::DeleteLocalFilesListedInTheDeleteListFile( BOOL bUpdateLastDeleteTime, CStringList &rasFilesDeleted ) {
	
	u32 nFileCountToDelete = m_asFilesToDelete.GetCount();
	if( m_LastDeleteTime == m_DeleteListFileTime || 
		nFileCountToDelete == 0 ||
		m_LocalFiles.GetSize() <= 0 ) {
		// we have already probably deleted these files, don't do it again till the file count changes
		return 0;
	}

	rasFilesDeleted.RemoveAll();
	
	POSITION Pos;
	CString s, sPath, sFilename;
	int nIndex, i, nNumFileInfos, nFilesDeleted;
	const CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData;
	BOOL bWildCard;

	nFilesDeleted = 0;
	for( Pos = m_asFilesToDelete.GetHeadPosition(); Pos != NULL; ) {
		s = m_asFilesToDelete.GetNext( Pos );
		
		// break up s into a path string and a filename string
		nIndex = s.ReverseFind( '\\' );
		if( nIndex >= 0 ) {
			// get the path from the string
			sPath = s.Mid( 0, nIndex+1 );
			sPath.MakeLower();
			sFilename = s.Mid( nIndex+1 );
		} else {
			sPath.Empty();
			sFilename = s;
		}

		nIndex = sFilename.Find( ".*" );
		if( nIndex >= 0 ) {
			bWildCard = TRUE;
			sFilename.Delete( nIndex, 2 );
		} else {
			bWildCard = FALSE;
		}

		pFileInfoData = m_LocalFiles.GetData();
		nNumFileInfos = m_LocalFiles.GetSize();
		for( i=0; i < nNumFileInfos;  ) {
			pFileInfo = &pFileInfoData[i];
			s = ( !bWildCard ) ? pFileInfo->GetFileName() : pFileInfo->GetFileTitle();
			
			// see if the file name matches
			if( s.CompareNoCase( sFilename ) != 0 ) {
				// no match
				i++;
				continue;
			}
			if( !sPath.IsEmpty() ) {
				// see if the path matches
				s = pFileInfo->GetFileRoot();
				s.MakeLower();
				if( s.Find( sPath ) < 0 ) {
					// no match
					i++;
					continue;
				}
			}

			// delete the file described by pFileInfo
			DeleteFile( pFileInfo->GetFilePath() );
			// record the just deleted file
			rasFilesDeleted.AddTail( pFileInfo->GetFileName() );
			// remove pFileInfo from the local file array
			m_LocalFiles.RemoveAt( i, 1 );
			nFilesDeleted++;
			nNumFileInfos--;
		}
	}

	// update our cache variable, so that we don't do these same files again
	if( bUpdateLastDeleteTime ) {
		m_LastDeleteTime = m_DeleteListFileTime;
	}
	
	return nFilesDeleted;
}

// returns the number of master files deleted
u32 CPasmDlg::DeleteMasterFilesListedInTheDeleteListFile( BOOL bUpdateLastDeleteTime, const CStringList &rasExtraFilesToDelete ) {
	
	u32 nFileCountToDelete = m_asFilesToDelete.GetCount();
	if( m_LastDeleteTime == m_DeleteListFileTime || 
		nFileCountToDelete == 0 ||
		m_LocalFiles.GetSize() <= 0 ) {
		// we have already probably deleted these files, don't do it again till the file count changes
		return 0;
	}
	
	POSITION Pos;
	CString sFilename;
	int nIndex, nFilesDeleted;	

	// delete files from the delete list
	nFilesDeleted = 0;
	for( Pos = m_asFilesToDelete.GetHeadPosition(); Pos != NULL; ) {
		sFilename = m_asFilesToDelete.GetNext( Pos );
		
		// break up s into a filename string
		nIndex = sFilename.ReverseFind( '\\' );
		if( nIndex >= 0 ) {
			sFilename.Delete( 0, nIndex+1 );			
		}

		// delete the master file entry named sFilename
		m_MasterFile.DeleteEntry( sFilename );
		nFilesDeleted++;		
	}
	// delete files from the extra files delete list
	for( Pos = rasExtraFilesToDelete.GetHeadPosition(); Pos != NULL; ) {
		sFilename = rasExtraFilesToDelete.GetNext( Pos );
		
		// break up s into a filename string
		nIndex = sFilename.ReverseFind( '\\' );
		if( nIndex >= 0 ) {
			sFilename.Delete( 0, nIndex+1 );			
		}

		// delete the master file entry named sFilename
		m_MasterFile.DeleteEntry( sFilename );
		nFilesDeleted++;		
	}

	if( nFilesDeleted ) {
		// close the master file
		m_MasterFile.EndCompilation();

		// re-read the master file
		//m_MasterFile.ReadHeaderAndEntries( m_sCurrentMasterFilename );
		m_MasterFile.ReadHeaderAndEntries( m_sCurrentMasterFilename, FALSE, TRUE );
	}

	// update our cache variable, so that we don't do these same files again
	if( bUpdateLastDeleteTime ) {
		m_LastDeleteTime = m_DeleteListFileTime;
	}
	
	return nFilesDeleted;
}

u32 CPasmDlg::DeleteAnyTempPasmFiles() {
	int i, nNumFileInfos, nFilesDeleted;
	const CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData;
	
	nFilesDeleted = 0;
	pFileInfoData = m_LocalFiles.GetData();
	nNumFileInfos = m_LocalFiles.GetSize();
	for( i=0; i < nNumFileInfos;  ) {
		pFileInfo = &pFileInfoData[i];
		if( pFileInfo->GetFileExt().CompareNoCase( UTILS_PASM_TEMP_FILENAME_TAG ) != 0 ) {
			i++;
		} else {
			// delete the file described by pFileInfo
			DeleteFile( pFileInfo->GetFilePath() );
			
			// remove pFileInfo from the local file array
			m_LocalFiles.RemoveAt( i, 1 );
			nFilesDeleted++;
			nNumFileInfos--;
		}
	}
	return nFilesDeleted;
}

void CPasmDlg::OnDumpFilenames() {
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_ctrlFileList.GetSelectedCount() <= 0 ) {
		this->MessageBox( _T("There are no files currently selected.\nNothing to dump to a .csv file."), 
						  _T("PASM"),
						  MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON1 );
		return;
	}

	CSettings& Settings = CSettings::GetCurrent();
	CString str = Settings.GetLastOutputName();
	
	CFileDialog dlg( FALSE,
					 ".csv",
					 str,
					 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					 "Comma Separated Files (*.csv)|*.csv||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select CSV Dump File";
	if( dlg.DoModal() == IDOK ) {
		str = dlg.GetPathName();
		Settings.m_sLastOutputName = str;		
		
		// open the output file
		CStdioFile File;	
		if( !File.Open( str, CFile::modeCreate | CFile::modeWrite | 
							 CFile::typeText | CFile::shareDenyNone )  ) {
			str.Format( "Could not open:\n%s\nfor writting.\nNo file could be written to disk.\n", Settings.m_sLastOutputName ); 
			this->MessageBox( str, 
							  _T("PASM Error"),
							  MB_ICONHAND | MB_OK | MB_DEFBUTTON1 );
			return;
		}

		// output a header row
		str = "Path, Filename, Day, Date, Time, Size (bytes),\n";
		File.WriteString( str );

		// walk all of the selected files
		POSITION Pos;
		int nIndex;
		CMyItemInfo *pItemInfo;
		CFileInfo *pFileInfo; 
		CTime ModifiedTime;
		
		// write out the local file info
		Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
		while( Pos ) { 
			nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
			// grab the user data
			pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
			pFileInfo = pItemInfo->GetLocFileInfo();
			if( pFileInfo ) { 
				ModifiedTime = pFileInfo->GetLastWriteTime();
				str.Format( "%s,%s,%s,\"%s\",%s,%d,\n", pFileInfo->GetFileRoot(),
													 pFileInfo->GetFileName(),
													 ModifiedTime.Format( "%a" ),
													 ModifiedTime.Format( "%b %d, %Y" ),
													 ModifiedTime.Format( "%H:%M:%S" ),
													 pFileInfo->GetLength() );
				File.WriteString( str );
			}
		}
		// write out an extra line
		str.Format( "\n" );
		File.WriteString( str );

		// write out the network file info
		Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
		while( Pos ) { 
			nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
			// grab the user data
			pItemInfo = (CMyItemInfo *)m_ctrlFileList.GetItemData( nIndex );
			pFileInfo = pItemInfo->GetLibFileInfo();
			if( pFileInfo ) { 
				ModifiedTime = pFileInfo->GetLastWriteTime();
				str.Format( "%s,%s,%s,\"%s\",%s,%d,\n", pFileInfo->GetFileRoot(),
													 pFileInfo->GetFileName(),
													 ModifiedTime.Format( "%a" ),
													 ModifiedTime.Format( "%b %d, %Y" ),
													 ModifiedTime.Format( "%H:%M:%S" ),
													 pFileInfo->GetLength() );
				File.WriteString( str );
			}
		}

		File.Close();
	}
} 

CFileInfo *CPasmDlg::FindCompilableLocalFileFromAidFile( CFileInfo *pAidFileInfo ) {
	CString sAssetName;
	void *pLookup;

	// try to find a matching .tga file
	sAssetName.Format( "%s.tga", pAidFileInfo->GetFileTitle() );
	sAssetName.MakeLower();
	if( m_MapLocalFiles.Lookup( sAssetName, pLookup ) ) {
		// found a .tga file with the same name
		return (CFileInfo *)pLookup;
	}

	// try to find a matching .wld file
	sAssetName.Format( "%s.wld", pAidFileInfo->GetFileTitle() );
	sAssetName.MakeLower();
	if( m_MapLocalFiles.Lookup( sAssetName, pLookup ) ) {
		// found a .wld file with the same name
		return (CFileInfo *)pLookup;
	}

	// try to find a matching .ape file
	sAssetName.Format( "%s.ape", pAidFileInfo->GetFileTitle() );
	sAssetName.MakeLower();
	if( m_MapLocalFiles.Lookup( sAssetName, pLookup ) ) {
		// found a .ape file with the same name
		return (CFileInfo *)pLookup;
	}

	// try to find a matching .mtx file
	sAssetName.Format( "%s.mtx", pAidFileInfo->GetFileTitle() );
	sAssetName.MakeLower();
	if( m_MapLocalFiles.Lookup( sAssetName, pLookup ) ) {
		// found a .mtx file with the same name
		return (CFileInfo *)pLookup;
	}

	return NULL;
}

void CPasmDlg::OnToolsCreateZipFile() {

	UpdateData( CONTROLS_TO_VARS );

	if( m_sCurrentMasterFilename.IsEmpty() ) {
		return;
	}
	CString str, sZipFile = m_sCurrentMasterFilename;
	sZipFile += ".zip";

	str.Format( "Creating the zip file:\n%s\ncould take a long time,\nproceed?", sZipFile );
	if( this->MessageBox( str, _T( "PASM Warning" ), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1 ) == IDYES ) {
		// close the master file, the zip archive needs to open it for reading
		this->EnableWindow( FALSE );
		
		m_MasterFile.EndCompilation();

		CFileStatus FileStatus;
		CFile::GetStatus( m_sCurrentMasterFilename, FileStatus );
		
		CProgressWnd WndProgress( this, "PASM: Creating Zip File...", TRUE, FALSE );
		WndProgress.SetRange( 0, (u32)FileStatus.m_size >> 9, 512 );
		WndProgress.SetText( "Zipping the master file, creating:\n"
							 "%s\n\n"
							 "Click Cancel to quit waiting.", sZipFile );
		
		CZipArchive ZipFile;
		ZipFile.Open( sZipFile, CZipArchive::create );
		BOOL bFileOK = ZipFile.AddNewFile( m_sCurrentMasterFilename, Z_BEST_SPEED, TRUE, _ZipCompressCallback, &WndProgress, 0x3FFFC );
		ZipFile.Close();

		SetForegroundWindow();
		this->EnableWindow( TRUE );

		if( WndProgress.Cancelled() ) {
			bFileOK = FALSE;
		}
		
		if( bFileOK ) {
			str.Format( "The zip file:\n%s\nwas created successfully.\n", sZipFile ); 
			this->MessageBox( str, _T("PASM Message"), MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1 );
		} else {
			str.Format( "Trouble creating the zip file:\n%s\nThe zip file has been deleted.\n", sZipFile ); 
			this->MessageBox( str, _T("PASM ERROR"), MB_ICONHAND | MB_OK | MB_DEFBUTTON1 );
			::DeleteFile( sZipFile );		
		}		
	}
	// no matter what happened, we should rescan all so that we can reopen the master file
	RescanAll( TRUE );
}

static bool _ZipCompressCallback( int , int nBytesSoFar, void *pData ) {
	CProgressWnd *pWndProgress = (CProgressWnd *)pData;

	pWndProgress->PeekAndPump( FALSE );

	if( pWndProgress->Cancelled() ) {
		// the user got tired of waiting
		return false;
	}

	pWndProgress->SetPos( (nBytesSoFar >> 9) );
	
	return true;
}

void CPasmDlg::OnCompactMasterFile() {
	CString sExt, sLogFile;
	
	if ( !CheckMasterFileCompilerVersion() )
	{
		return;
	}

	UpdateData( CONTROLS_TO_VARS );

	// warn about the time this might take
	if( this->MessageBox( _T( "Compacting the master file\ncan be VERY time consuming,\nproceed?" ),
						  _T( "PASM Warning" ),
						  MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1 ) == IDNO ) {
		return;
	}

	BeginWaitCursor();
	if( !m_MasterFile.RearrangeMasterFile( m_sCurrentMasterFilename, NULL, FALSE ) ) {
		EndWaitCursor();
		// problem compacting the master file
		MessageBox( _T("There was a problem during the compacting process.\nThe master file has not been compacted." ), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}
	ReadMasterFileEntries();
	UpdateFileListBox( TRUE );
	EndWaitCursor();

	UpdateData( VARS_TO_CONTROLS );	
}

void CPasmDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here

	m_nLightingQuality = ((CSliderCtrl*)GetDlgItem( IDC_SLIDER1 ))->GetPos();

	*pResult = 0;
}

void CPasmDlg::OnEnChangeLmapMaxsize()
{
	char szString[80];

	CEdit *edit = (CEdit*)GetDlgItem( IDC_LMAP_MAXSIZE );
	edit->GetWindowText(szString, 80);
	m_fMaxLMapSize = (f32)atof(szString);
}

/*
void CPasmDlg::OnBnClickedFilter()
{
	// TODO: Add your control notification handler code here

	m_bFilterLightMaps = !m_bFilterLightMaps;
}
*/

void CPasmDlg::OnEnChangeSubsample()
{
	char szString[80];

	CEdit *edit = (CEdit*)GetDlgItem( IDC_SUBSAMPLE );
	edit->GetWindowText(szString, 80);
	m_fSubSample = (f32)atof(szString);
}

void CPasmDlg::OnToolsGenerateMasterFileList() {

	UpdateData( CONTROLS_TO_VARS );
	
	CStringInput StringInput;
	CString sExtToLog;
	CString sDefault = ".sfb";
	
	if( StringInput.GetUserString( sDefault, "What To Log?", "Enter a file extension to log, include a period\n(.* to log everything)", sExtToLog ) ) {

		DEVPRINTF( "searching for master file entries of type '%s'...\n", sExtToLog );
		sExtToLog.MakeLower();
		u32 nNumFilesLogged;
		CStringList Files;
		if( sExtToLog.Find( ".*" ) >= 0 ) {
			nNumFilesLogged = m_MasterFile.LogAllRegularFileEntries( Files, NULL );
			sExtToLog = "(all)";
		} else {
			nNumFilesLogged = m_MasterFile.LogAllRegularFileEntries( Files, sExtToLog );
		}

		DEVPRINTF( "found %d files of type '%s'...\n", nNumFilesLogged, sExtToLog );

		if( nNumFilesLogged ) {
			// open the output file
			CString sTemp;
			sTemp.Format( "%s\\log of masterfile %s files.txt", m_sXBMasterFileDir, sExtToLog );
			FILE *pLogFile = fopen( sTemp, "wt" );
			if( !pLogFile ) {
				return;
			}

			fprintf( pLogFile, "# Log file contains %d files\n", nNumFilesLogged );

			POSITION Pos = Files.GetHeadPosition();
			while( Pos ) {
				sTemp = Files.GetNext( Pos );
				if ( sExtToLog.CompareNoCase( ".wld" ) == 0 )
				{
					FDataPrjFile_Entry_t *pMFEntry = m_MasterFile.GetEntryByName( sTemp );

					if ( !pMFEntry )
					{
						fprintf( pLogFile, "%s\n", sTemp );
					}
					else if ( pMFEntry->nFlags & 0x0001 )
					{
						fprintf( pLogFile, "%s,%d,Reused\n", sTemp, (pMFEntry->nFlags >> 12) );
					}
					else
					{
						fprintf( pLogFile, "%s,%d,Perfect Match\n", sTemp, (pMFEntry->nFlags >> 12) );
					}
				}
				else if ( sExtToLog.CompareNoCase( ".ape" ) == 0 )
				{
					FDataPrjFile_Entry_t *pMFEntry = m_MasterFile.GetEntryByName( sTemp );
					if ( !pMFEntry )
					{
						fprintf( pLogFile, "%s\n", sTemp );
					}
					else
					{
						CString csOut = sTemp + ",";
						if ( pMFEntry->nFlags & APE_ENTRY_FLAGS_LIGHTMAPUV )
						{
							csOut += "Lightmap UV,";
						}
						else
						{
							csOut += ",";
						}
						if ( pMFEntry->nFlags & APE_ENTRY_FLAGS_VERTRAD )
						{
							csOut += "Vert Rad,";
						}
						fprintf( pLogFile, "%s\n", csOut );
					}
				}
				else
				{
					fprintf( pLogFile, "%s\n", sTemp );
				}
			}
			fclose( pLogFile );	
		}		

		UpdateData( VARS_TO_CONTROLS );
	}		
}

void CPasmDlg::OnToolsCreateNewMasterFileWithOnlyLoggedAssets() {
	OptimizeMasterFile( TRUE, FALSE );
}

BOOL CPasmDlg::CreateUniqueListOfLogggedFiles( const CString &rsLogFile, CStringList &rasFilenames, BOOL bReplaceUSWithInternationalNames ) {
	CString sLogFile, sTemp;
	
	// read the log file into memory
	FILE *pLogFile = fopen( rsLogFile, "rb" );
	if( !pLogFile ) {
		MessageBox( _T("Could not open the log file for reading."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;
	}
	fseek( pLogFile, 0, SEEK_END );
	int nLogFileBytes = ftell( pLogFile );
	fseek( pLogFile, 0, SEEK_SET );
	char *pFileBuffer = new char[nLogFileBytes];
	if( !pFileBuffer ) {
		fclose( pLogFile );
		MessageBox( _T("Could not allocate enough\nmemory to hold the log file."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;	
	}
	if( fread( pFileBuffer, nLogFileBytes, 1, pLogFile ) != 1 ) {
		delete [] pFileBuffer;
		fclose( pLogFile );
		MessageBox( _T("Could not read the log file."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return FALSE;	
	}      
	fclose( pLogFile );

	// create a CStringList of the filenames from the log file
	CStringList asLightmaps;
	char *pszLine;
	int i, nLen, nStartingIndex, nDollarSignIndex;

	// replace all CR/LF with NULL
	for( i=0; i < nLogFileBytes; i++ ) {
		if( pFileBuffer[i] == 0x1A ||
			pFileBuffer[i] == 0xA ||
			pFileBuffer[i] == 0xD ) {
			pFileBuffer[i] = 0;
		}
	}

	rasFilenames.RemoveAll();
	asLightmaps.RemoveAll();

	// add a few platform specific required files to the beginning of the list
	if( m_bXBoxFormat ) {
		// add the rdx files		
		rasFilenames.AddTail( "dsp_sfx.rdx" );
	} else if( m_bGCFormat ) {
		rasFilenames.AddTail( "snd_smpls.rdg" );
		rasFilenames.AddTail( "snd_init.rdg" );
	}

	pszLine = pFileBuffer;
	i = 0;
	while( i < nLogFileBytes ) {
		sTemp = pszLine;	
		if( i == 0 ) {
			// to validate this log file, make sure that the first line starts with:
			// #FANG***
			if( sTemp.Find( "#FANG***" ) != 0 ) {
				// not a valid fang log file
				delete [] pFileBuffer;
				MessageBox( _T("The log file does not appear\nto be a valid log file." ), 
							_T("PASM ERROR"),
							MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
				return FALSE;
			}
		}
		nLen = sTemp.GetLength();
		if( nLen > 0 ) {
			if( sTemp[0] != '#' ) {
				// not a comment line
				sTemp.MakeLower();

				// First, check to see if this is an audio file...
				BOOL bIsAudioFile = FALSE;
				nStartingIndex = sTemp.Find( ".wvb" );
				if( nStartingIndex >= 0 ) {
					bIsAudioFile = TRUE;
					if( m_bGCFormat ) {
						// remap .wvb files to .rdg files
						sTemp.Delete( nStartingIndex, 4 );
						sTemp += ".rdg";
					}
				}

				BOOL bAlreadyAddedToList = FALSE;
				// Now, if this is an international optimize and we are 
				// working with an audio file...
				if( bIsAudioFile && bReplaceUSWithInternationalNames ) {
					// We want to do some special processing here...
					// First, add a 0 before the period....
					sTemp.Insert( nStartingIndex, "0" );
					
					// Now, see if we can find this audio name...
					if( rasFilenames.Find( sTemp ) == NULL ) {
						// This name is NOT yet in the list..., so add it...
						// Also Add all the names of the other international wave bank permutations.
						rasFilenames.AddTail( sTemp );		
						sTemp.SetAt( nStartingIndex, '1' );
						rasFilenames.AddTail( sTemp );		
						sTemp.SetAt( nStartingIndex, '2' );
						rasFilenames.AddTail( sTemp );		
						sTemp.SetAt( nStartingIndex, '3' );
						rasFilenames.AddTail( sTemp );		
						sTemp.SetAt( nStartingIndex, '4' );
						rasFilenames.AddTail( sTemp );		
						bAlreadyAddedToList = TRUE;
					}
				}

				if( !IsFilenameALightmap( sTemp ) ) {

					// Check to see if this filename has a dollar sign in it...
					nDollarSignIndex = sTemp.Find( "$" );
					if( ( nDollarSignIndex >= 0 ) && bReplaceUSWithInternationalNames ) {
						// This is an international file, replace the dollar sign with all of our possible
						// international characters...
						sTemp.SetAt( nDollarSignIndex, '!' );
						if( rasFilenames.Find( sTemp ) == NULL ) {
							// If we don't find this sign, then NONE of the international character filenames are in there, 
							// so add ALL of them...
							rasFilenames.AddTail( sTemp );		
							sTemp.SetAt( nDollarSignIndex, '%' );
							rasFilenames.AddTail( sTemp );		
							sTemp.SetAt( nDollarSignIndex, '&' );
							rasFilenames.AddTail( sTemp );		
							sTemp.SetAt( nDollarSignIndex, '^' );
							rasFilenames.AddTail( sTemp );		
							sTemp.SetAt( nDollarSignIndex, '~' );
							rasFilenames.AddTail( sTemp );		
						}
					}
					// make sure that this name hasn't already appeared in the list
					else if( !bAlreadyAddedToList && ( rasFilenames.Find( sTemp ) == NULL ) ) {
						rasFilenames.AddTail( sTemp );		
					}
				} else {
					if( asLightmaps.Find( sTemp ) == NULL ) {
						asLightmaps.AddTail( sTemp );
					}
				}
			}
			i += nLen;
		} else {
			i++;
		}
		pszLine = &pFileBuffer[i];
	}
	
	// free our allocated memory
	delete [] pFileBuffer;

	DEVPRINTF( "found %d regular files and %d lightmaps file in the load log...\n", rasFilenames.GetCount(), asLightmaps.GetCount() );

	// append the lightmap list to the end of the regular file list
	if( asLightmaps.GetCount() > 0 ) {
		rasFilenames.AddTail( &asLightmaps );
	}

	// we must ensure that the platfrom specific files are added to the list
	CStringList PlatformSpecificFiles;
	PlatformSpecificFiles.RemoveAll();
	if( m_bXBoxFormat ) {
		// add the rdx files		
		m_MasterFile.LogAllRegularFileEntries( PlatformSpecificFiles, ".rdx" );
	} else if( m_bGCFormat ) {
		// add the rdg files
		m_MasterFile.LogAllRegularFileEntries( PlatformSpecificFiles, ".rdg" );
	}
	// add any of the platform specific files that are not already in the list
	POSITION Pos = PlatformSpecificFiles.GetHeadPosition();
	while( Pos ) {
		sTemp = PlatformSpecificFiles.GetNext( Pos );
		if( rasFilenames.Find( sTemp ) == NULL ) {
			rasFilenames.AddTail( sTemp );		
		}
	}

	return TRUE;
}

void CPasmDlg::OnToolsGenerateListOfUnusedAssets() {
	CStringList UnusedFiles, LogFile;
	CString sExt, sLogFile, sTemp;
	
	if ( !CheckMasterFileCompilerVersion() ) {
		return;
	}

	UpdateData( CONTROLS_TO_VARS );

	// have the user select a log file 
	sExt = "Text Files (*.txt)|*.txt|Log Files (*.log)|*.log|All Files (*.*)|*.*||";	
	sTemp = m_sCurrentMasterFilename;
	sTemp += ".txt";
	CFileDialog LogFileDlg( TRUE, NULL, sTemp,
							OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
							(cchar *)sExt, this );
	LogFileDlg.m_ofn.lpstrTitle = "Select FANG Log File";
	if( LogFileDlg.DoModal() == IDOK ) {
		sLogFile = LogFileDlg.GetPathName();		
	} else {
		MessageBox( _T("Must select a log file before the\nunused assets can be logged."), 
					_T("PASM ERROR"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}

	BeginWaitCursor();
	
	// read the log file
	if( !CreateUniqueListOfLogggedFiles( sLogFile, LogFile, FALSE ) ) {
		EndWaitCursor();
		return;
	}

	// log all of the assets in the 
	u32 nUnusedFiles = m_MasterFile.LogAllUnusedRegularFileEntries( LogFile, UnusedFiles );

	sTemp.Format( "%s\\log of unused files in masterfile.txt", m_sXBMasterFileDir );
	FILE *pLogFile = fopen( sTemp, "wt" );
	if( !pLogFile ) {
		EndWaitCursor();
		return;
	}

	fprintf( pLogFile, "# Log file contains %d files that can be deleted from the masterfile.\n", nUnusedFiles );
	fprintf( pLogFile, "# Generated from the log file: %s\n", sLogFile );
	fprintf( pLogFile, "# Format: Asset name, size in masterfile\n" );

	FDataPrjFile_Entry_t *pMasterFileEntry;
	u32 nTotalBytes = 0, nBytes;
	
	POSITION Pos = UnusedFiles.GetHeadPosition();
	while( Pos ) {
		sTemp = UnusedFiles.GetNext( Pos );
		pMasterFileEntry = m_MasterFile.GetEntryByName( sTemp );
		nBytes = (pMasterFileEntry) ? pMasterFileEntry->nNumBytes : 0;
			
		fprintf( pLogFile, "%s,%d,\n", sTemp, nBytes );
		nTotalBytes += nBytes;
	}
	fprintf( pLogFile, "# Num of Bytes that can be saved by cutting these files out of the masterfile:,%d\n", nTotalBytes );
	
	fclose( pLogFile );	
	
	EndWaitCursor();
}

void CPasmDlg::OnOptimizeInternationalMasterFile()
{
	OptimizeMasterFile( FALSE, TRUE );
}
