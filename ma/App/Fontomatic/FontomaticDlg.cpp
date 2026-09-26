//////////////////////////////////////////////////////////////////////////////////////
// FontomaticDlg.cpp
//
// Author: Albert Yale
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
// 09/17/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "stdafx.h"
#include <io.h>
#include "fang.h"
#include "Fontomatic.h"
#include "FontomaticDlg.h"
#include "Settings.h"
#include "VidMode.h"
#include "UnicodeFile.h"

#include "tga.h"
#include "fontcut.h"
#include "fontpack.h"
#include "fontscript.h"
#include "fclib.h"

#include <stdlib.h>
#include <errno.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// private prototypes
static BOOL _GameInit( void *pParameter );
static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter );
static BOOL _GameMain( BOOL bExitRequest, void *pParameter );

// static variables
static ImageData_t *_pImage, _ImageData;
static FontData_t *_pFont, _FontData;
static PackData_t *_pPack;

#define _FONT_HANDLE	'1'

static BOOL _bMainLoopExecuted;
static FResFrame_t _oResFrame;

TCHAR _szTempExpanded[ 204 ];
TCHAR _szTempCRLF[ 201 ];
TCHAR _szTemp[ 201 ];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

}; // CAboutDlg

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT

} // CAboutDlg

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP

} // DoDataExchange

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CFontomaticDlg::CFontomaticDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFontomaticDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFontomaticDlg)
	m_oSliderItalicSlant = 0;
	m_oSliderNumberLines = 0;
	m_oSliderSpacingLetter = 0;
	m_oSliderSpacingLine = 0;
	m_oSliderSpacingWord = 0;
	m_oCheckDrawGuides = FALSE;
	m_oCheckFixedWidth = FALSE;
	m_oCheckItalic = FALSE;
	m_oEditPreview = _T("");
	m_oOneToOne = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD));

	m_bValidVideoMode = FALSE;
	m_bExiting = FALSE;

} // CFontomaticDlg

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFontomaticDlg)
	DDX_Control(pDX, IDC_OUTPUT, m_oOutput);
	DDX_Control(pDX, IDC_OUTPUT_DIR, m_oOutputDir);
	DDX_Control(pDX, IDC_EDIT_MASTER_FILE, m_oEditMasterFile);
	DDX_Control(pDX, IDC_RUN_SCRIPT, m_oRunScript);
	DDX_Control(pDX, IDC_RUN, m_oRun);
	DDX_Control(pDX, IDC_OUTPUT_SCRIPT_FILE_BROWSE, m_oOutputScriptFileBrowse);
	DDX_Control(pDX, IDC_LOAD, m_oLoad);
	DDX_Control(pDX, IDC_INPUT_FILE_BROWSE, m_oInputFileBrowse);
	DDX_Control(pDX, IDC_INPUT_FILE, m_oInputFile);
	DDX_Control(pDX, IDC_INPUT_CHARS_BROWSE, m_oInputCharsBrowse);
	DDX_Control(pDX, IDC_EDIT_SPACING_WORD, m_oEditSpacingWord);
	DDX_Control(pDX, IDC_EDIT_SPACING_LINE, m_oEditSpacingLine);
	DDX_Control(pDX, IDC_EDIT_SPACING_LETTER, m_oEditSpacingLetter);
	DDX_Control(pDX, IDC_EDIT_NUMBER_LINES, m_oEditNumberLines);
	DDX_Control(pDX, IDC_EDIT_ITALIC_SLANT, m_oEditItalicSlant);
	DDX_Control(pDX, IDC_BUTTON_VIDMODE, m_oButtonVidMode);
	DDX_Control(pDX, IDC_BUTTON_PREVIEW, m_oButtonPreview);
	DDX_Control(pDX, IDC_BUTTON_MASTER_FILE, m_oButtonMasterFile);
	DDX_Control(pDX, IDC_CHARACTER_STRING, m_oCharString);
	DDX_Slider(pDX, IDC_SLIDER_ITALIC_SLANT, m_oSliderItalicSlant);
	DDX_Slider(pDX, IDC_SLIDER_NUMBER_LINES, m_oSliderNumberLines);
	DDX_Slider(pDX, IDC_SLIDER_SPACING_LETTER, m_oSliderSpacingLetter);
	DDX_Slider(pDX, IDC_SLIDER_SPACING_LINE, m_oSliderSpacingLine);
	DDX_Slider(pDX, IDC_SLIDER_SPACING_WORD, m_oSliderSpacingWord);
	DDX_Check(pDX, IDC_CHECK_DRAW_GUIDES, m_oCheckDrawGuides);
	DDX_Check(pDX, IDC_CHECK_FIXED_WIDTH, m_oCheckFixedWidth);
	DDX_Check(pDX, IDC_CHECK_ITALIC, m_oCheckItalic);
	DDX_Text(pDX, IDC_EDIT_PREVIEW, m_oEditPreview);
	DDX_Check(pDX, IDC_CHECK_ONE_TO_ONE, m_oOneToOne);
	//}}AFX_DATA_MAP

} // DoDataExchange

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BEGIN_MESSAGE_MAP(CFontomaticDlg, CDialog)
	//{{AFX_MSG_MAP(CFontomaticDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_INPUT_FILE_BROWSE, OnInputFileBrowse)
	ON_BN_CLICKED(IDC_RUN, OnRun)
	ON_BN_CLICKED(IDC_OUTPUT_SCRIPT_FILE_BROWSE, OnOutputScriptFileBrowse)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_BUTTON_VIDMODE, OnButtonVidmode)
	ON_BN_CLICKED(IDC_BUTTON_MASTER_FILE, OnButtonMasterFile)
	ON_BN_CLICKED(IDC_BUTTON_PREVIEW, OnButtonPreview)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_INPUT_CHARS_BROWSE, OnInputCharsBrowse)
	ON_BN_CLICKED(IDC_RUN_SCRIPT, OnRunScript) 
	ON_BN_CLICKED(IDC_CHECK_FIXED_WIDTH, OnCheckFixedWidth)
	ON_EN_CHANGE(IDC_EDIT_PREVIEW, OnChangeEditPreview)
	ON_BN_CLICKED(IDC_CHECK_DRAW_GUIDES, OnCheckDrawGuides)
	ON_BN_CLICKED(IDC_CHECK_ITALIC, OnCheckItalic)
	ON_BN_CLICKED(IDC_CHECK_ONE_TO_ONE, OnCheckOneToOne)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CFontomaticDlg::OnInitDialog()
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
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	::SetProp( GetSafeHwnd(), Fontomatic_pszExeName, (HANDLE)1 );

	m_oTempString.Format( _T( "version # %d.%d.%d" ), fversion_GetToolMajorVer(), 
												      fversion_GetToolMinorVer(),
												      fversion_GetToolSubVer() );
	SetDlgItemText( IDC_STATIC_VERSION, m_oTempString );	

	SetDlgItemText( IDC_EDIT_PREVIEW, _T( "The quick brown fox jumps over the lazy dog!" ) );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->SetRange( 0, 65 );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->SetTicFreq( 5 );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->SetRange( 0, 65 );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->SetTicFreq( 5 );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LINE ))->SetRange( 0, 65 );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LINE ))->SetTicFreq( 5 );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->SetRange( 1, 65 );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->ClearTics();
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->SetTic( 1 );
	for( u32 uIndex = 5; uIndex <= 65; uIndex += 5 ) ((CSliderCtrl *)GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->SetTic( uIndex );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->SetRange( -65, 65 );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->SetTicFreq( 5 );

	CSettings::SetSettingsFilename( _T( "fontomatic.ini" ) );

	//// get the default values from our settings file
	//
	CSettings &Settings = CSettings::GetCurrent();
	CSettings::SetApplicationName( _T( "fontomatic" ) );

	m_sMasterfileNameAndPath = Settings.m_sMasterFile;
	SetDlgItemText( IDC_EDIT_MASTER_FILE, m_sMasterfileNameAndPath );
	SetDlgItemText( IDC_INPUT_FILE, Settings.m_sInputFile );
	SetDlgItemText( IDC_OUTPUT_DIR, Settings.m_sOutputDir );
	SetDlgItemText( IDC_CHARACTER_STRING, Settings.m_sCharSet );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->SetPos( (u32)Settings.m_fLetterSpacing );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->SetPos( (u32)Settings.m_fWordSpacing );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LINE ))->SetPos( (u32)Settings.m_fLineSpacing );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->SetPos( (u32)Settings.m_fNumberOfLines );
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->SetPos( (s32)Settings.m_fItalicSlant );

	((CButton *)GetDlgItem( IDC_CHECK_ONE_TO_ONE ))->SetCheck( Settings.m_bOneToOne );
	((CButton *)GetDlgItem( IDC_CHECK_DRAW_GUIDES ))->SetCheck( Settings.m_bDrawGuides );
	((CButton *)GetDlgItem( IDC_CHECK_FIXED_WIDTH ))->SetCheck( Settings.m_bFixedWidthFont );
	((CButton *)GetDlgItem( IDC_CHECK_ITALIC ))->SetCheck( Settings.m_bItalicFont );

	SetDlgItemInt( IDC_EDIT_SPACING_LETTER, (u32)Settings.m_fLetterSpacing, FALSE );
	SetDlgItemInt( IDC_EDIT_SPACING_WORD, (u32)Settings.m_fWordSpacing, FALSE );
	SetDlgItemInt( IDC_EDIT_SPACING_LINE, (u32)Settings.m_fLineSpacing, FALSE );
	SetDlgItemInt( IDC_EDIT_NUMBER_LINES, (u32)Settings.m_fNumberOfLines, FALSE );

	TCHAR szTemp[ 5 ];
	_stprintf( szTemp, _T( "%d" ), (s32)Settings.m_fItalicSlant );
	SetDlgItemText( IDC_EDIT_ITALIC_SLANT, szTemp );

	m_fOriginalLetterSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->GetPos();
	m_fOriginalWordSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->GetPos();
	m_fOriginalItalicSlant = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->GetPos();
	//
	////

	m_GameloopParm.pDlg = this;
	_bMainLoopExecuted = FALSE;

	TCHAR pszComputerName[256];
	DWORD nSize = 256;
	GetComputerName( pszComputerName, &nSize );
	m_sComputerName = pszComputerName;
	nSize = 256;
	GetUserName( pszComputerName, &nSize );
	m_sUserName = pszComputerName;

	SetDlgItemText( IDC_OUTPUT, _T( "Please enter the input filenames and character list, then press Generate" ) );
	m_bUseLoadedValues = FALSE;

	if( m_sMasterfileNameAndPath.IsEmpty() )
	{
		MessageBox( _T("You must select a master file before Fontomatic\ncan be used.  Click OK and then select\na master file."), 
					_T("Fontomatic Error: No Master File"),
					MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		CFileDialog dlg( TRUE,
						 _T( ".mst" ),
						 NULL,
						 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
						 _T( "Master Files (*.mst)|*.mst||" ),
						 this );
		dlg.m_ofn.lpstrTitle = _T( "Select Master File" );
		if( dlg.DoModal() == IDOK ) {
			m_sMasterfileNameAndPath = dlg.GetPathName();
			SetDlgItemText( IDC_EDIT_MASTER_FILE, m_sMasterfileNameAndPath );
		} else {
			// no master file, no Fontomatic
			PostMessage( WM_COMMAND, IDCANCEL, 0 );
			return TRUE;
		}
	}

	m_Color = 13382497;

	// init static variables
	_pImage = NULL;
	_pFont = NULL;
	_pPack = NULL;

	// init systems
	tga_ModuleInit();
	fontcut_ModuleInit();
	fontpack_ModuleInit();
	fontscript_ModuleInit();

	fang_Init();
	Fang_ConfigDefs.nRes_HeapBytes = ( 4 * 1024 * 1024 );
	
#ifdef _UNICODE
	fclib_WideToChar( m_szMasterfileNameAndPath, m_sMasterfileNameAndPath );
#else
	strcpy( m_szMasterfileNameAndPath, m_sMasterfileNameAndPath );
#endif

	Fang_ConfigDefs.pszFile_MasterFilePathName = m_szMasterfileNameAndPath;

	if( ! fang_Startup() )
	{
		// could not install the fang engine, tell the user and exit
		this->MessageBox( _T( "Fang Engine failed to install." ), 
						  _T( "Install Failure" ),
						  MB_OK | MB_ICONSTOP );
		PostMessage( WM_COMMAND, IDCANCEL, 0 );  
		return TRUE;
	}

	GetDlgItemText( IDC_OUTPUT_DIR, m_oTempString );
	if( m_oTempString.IsEmpty() )
	{
		// Output directory not selected...
		MessageBox( _T( "You need to set an output directory before Fontomatic may be used.\nClick Ok to set the output directory." ), _T( "Fontomatic Error: Set Output Directory" ) );
		OnOutputScriptFileBrowse();
		GetDlgItemText( IDC_OUTPUT_DIR, m_oTempString );
		if( m_oTempString.IsEmpty() )
		{
			// no output directory, no Fontomatic
			PostMessage( WM_COMMAND, IDCANCEL, 0 );
			return TRUE;
		}
	}

	CVidMode VidMode;
	if( !VidMode.GetVidMode( &m_GameloopParm.VidWin ) )
	{
		// Video mode not selected...
		MessageBox( _T( "You need to set the video mode before Fontomatic may be used.\nClick Ok to set the video mode." ), _T( "Fontomatic Error: Set Video Mode" ) );
		OnButtonVidmode();
		if( ! m_bValidVideoMode )
		{
			// no video mode, no Fontomatic
			PostMessage( WM_COMMAND, IDCANCEL, 0 );
			return TRUE;
		}
	} else {
		m_bValidVideoMode = TRUE;
	}

	GetDlgItemText( IDC_INPUT_FILE, m_oTempString );
	if( ! m_oTempString.IsEmpty() )
	{
		((CButton *)GetDlgItem( IDC_BUTTON_PREVIEW ))->EnableWindow( TRUE );
		((CButton *)GetDlgItem( IDC_RUN ))->EnableWindow( TRUE );
	}

	m_GameloopParm.VidWin.bAllowPowerSuspend = TRUE;
	m_GameloopParm.VidWin.hInstance = theApp.m_hInstance;
	m_GameloopParm.VidWin.hWnd = 0;
	m_GameloopParm.VidWin.nIconIDI = IDR_MAINFRAME;
	m_GameloopParm.VidWin.pFcnSuspend = NULL;
	strcpy( m_GameloopParm.VidWin.szWindowTitle, "fontomatic" );

	CString sTitleString;
	GetWindowText( sTitleString );
#if _DEBUG
	sTitleString.AppendFormat( _T( " (DEBUG" ) );
	#if _UNICODE
		sTitleString.AppendFormat( _T( " UNICODE" ) );
	#endif //_UNICODE
	sTitleString.AppendFormat( _T( ")" ) );
#else if _UNICODE
	sTitleString.AppendFormat( _T( " (UNICODE)" ) );
#endif
	SetWindowText( sTitleString );
	UpdateData( TRUE );

	return TRUE;  // return TRUE  unless you set the focus to a control

} // OnInitDialog

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

} // OnSysCommand

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnPaint()
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
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

} // OnPaint

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

HCURSOR CFontomaticDlg::OnQueryDragIcon()
// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
{
	return (HCURSOR) m_hIcon;

} // OnQueryDragIcon

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnCancel()
{
	m_bExiting = TRUE;

	PromptUserToSaveFont( TRUE );

	// Writeout settings before exit.
	SaveSettingsToDisk();

	if( floop_IsGameloopRunning() )
	{
		floop_UninstallGameloop();
	}

	// close systems down
	FreeAllocatedFontMem();

	if( fang_IsStartedUp() )
	{
		// shutdown fang
		fang_Shutdown();
	}

	::RemoveProp( GetSafeHwnd(), Fontomatic_pszExeName );

	CDialog::OnCancel();

} // OnCancel

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::SaveSettingsToDisk()
{
	CSettings &Settings = CSettings::GetCurrent();

	Settings.m_bOneToOne = m_oOneToOne;
	Settings.m_bDrawGuides = m_oCheckDrawGuides;
	Settings.m_bFixedWidthFont = m_oCheckFixedWidth;
	Settings.m_bItalicFont = m_oCheckItalic;

	m_oEditMasterFile.GetWindowText( m_sMasterfileNameAndPath );
	Settings.m_sMasterFile = m_sMasterfileNameAndPath;
	m_oInputFile.GetWindowText( Settings.m_sInputFile );
	m_oOutputDir.GetWindowText( Settings.m_sOutputDir );
	m_oCharString.GetWindowText( Settings.m_sCharSet );

	Settings.m_fLetterSpacing = (f32)m_oSliderSpacingLetter;
	Settings.m_fWordSpacing = (f32)m_oSliderSpacingWord;
	Settings.m_fLineSpacing = (f32)m_oSliderSpacingLine;
	Settings.m_fNumberOfLines = (f32)m_oSliderNumberLines;
	Settings.m_fItalicSlant = (f32)m_oSliderItalicSlant;
	Settings.m_bFirstRun = FALSE;

	Settings.SaveCommonDataOutToFile();

} // SaveSettingsToDisk

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::FreeAllocatedFontMem()
{
	if( _pImage ) {
		tga_FreeImageData( _pImage );
		_pImage = NULL;
	}
	if( _pFont ) {
		fontcut_FreeFontData( _pFont );
		_pFont = NULL;
	}
	if( _pPack ) {
		fontpack_FreePackData( _pPack );
		_pPack = NULL;
	}

} // FreeAllocatedFontMem

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::GenerateFont( BOOL bScriptOnly )
{
	int nIndex;
	CString oTempInputFilenameAndPath, oTempInputFilenameOnly, oTempOutputDir, oTempCharsSet;

	UpdateData( TRUE );

	////
	//
	m_oInputFile.GetWindowText( oTempInputFilenameAndPath );
	if( oTempInputFilenameAndPath.IsEmpty() )
	{
		m_oOutput.SetWindowText( _T( "Please Enter a source TGA filename!" ) );
		return;
	}

	m_oOutputDir.GetWindowText( oTempOutputDir );
	if( oTempOutputDir.IsEmpty() )
	{
		m_oOutput.SetWindowText( _T( "Please select a destination directory!" ) );
		return;
	}
	oTempOutputDir += "\\";

	m_oCharString.GetWindowText( oTempCharsSet );
	if( oTempCharsSet.IsEmpty() )
	{
		m_oOutput.SetWindowText( _T( "Please Enter the characters in the font!" ) );
		return;
	}
	//
	////

	nIndex = oTempInputFilenameAndPath.ReverseFind( _T( '\\' ) );
	if( nIndex == -1 )
	{
		m_oOutput.SetWindowText( _T( "Please Enter a valid source TGA filename!" ) );
		return;
	}
	m_oTempString = oTempInputFilenameAndPath;
	m_oTempString.Delete( 0, nIndex + 1 );
	nIndex = m_oTempString.ReverseFind( '.' );
	if( nIndex == -1 )
	{
		m_oOutput.SetWindowText( _T( "Please Enter a valid source TGA filename!" ) );
		return;
	}
	m_oTempString.Delete( nIndex, m_oTempString.GetLength() - nIndex );

	CString oOutputScriptFile = oTempOutputDir + m_oTempString + _T( ".fnt" );
	CString TGAPath = oTempOutputDir;
	oTempInputFilenameOnly = m_oTempString;

	if( oTempInputFilenameOnly.GetLength() > 10 ) {
		m_oOutput.SetWindowText( _T( "TGA filename must be 10 characters or less!" ) );
		return;
	}

	//////////////////////////////
	// finally we can do some work
	//////////////////////////////

	// free all of the old stuff first
	FreeAllocatedFontMem();

	m_oInputFile.GetWindowText( m_oTempString );
	_pImage = tga_Get32BitImageDataFromFile( m_oTempString, &_ImageData );
	if( !_pImage ) {
		m_oOutput.SetWindowText( tga_GetLastError() );
		return;
	}

	_pFont = fontcut_CutUpImageIntoAFont( _pImage, &_FontData );
	if( !_pFont ) {
		FreeAllocatedFontMem();
		m_oOutput.SetWindowText( fontcut_GetLastError() );
		return;
	}
	_pPack = fontpack_CreatePackedTGAs( ! bScriptOnly, oTempInputFilenameOnly, TGAPath, _pImage, _pFont );
	if( !_pPack ) {
		FreeAllocatedFontMem();
		m_oOutput.SetWindowText( fontpack_GetLastError() );
		return;
	}
	// make sure that we have a character string whose length is equal to the number of letters there are
	nIndex = oTempCharsSet.GetLength();
	if( (u32)nIndex != _pFont->nNumLetters )
	{
		m_oTempString.Format( _T( "The character list (%d letters) and TGA (%d letters) don't match!" ), nIndex, _pFont->nNumLetters );
		m_oOutput.SetWindowText( m_oTempString );
		FreeAllocatedFontMem();
		return;
	}
	// use the spacing values obtained from the script file, we must have loaded an existing file
	if( m_bUseLoadedValues ) {
		m_bUseLoadedValues = FALSE;
	}
	if( !fontscript_WriteScriptFile( oOutputScriptFile, oTempInputFilenameAndPath, _pPack,
									 m_sComputerName, m_sUserName, oTempCharsSet,
									 TGAPath,
									 (f32)m_oSliderSpacingLetter,
									 (f32)m_oSliderSpacingWord,
									 (f32)m_oSliderItalicSlant ) )
	{
		FreeAllocatedFontMem();
		m_oOutput.SetWindowText( fontscript_GetLastError() );
		return;
	}	
	// tell the user that everything is cool
	if( bScriptOnly )
	{
		m_oTempString.Format( _T( "Success, 1 Script file written containing %d letters" ), _pPack->pOriginalData->nNumLetters );
	}
	else
	{
		m_oTempString.Format( _T( "Success, 1 Script file and %d TGA files written containing %d letters" ), _pPack->nNumPages, _pPack->pOriginalData->nNumLetters );
	}
	m_oOutput.SetWindowText( m_oTempString );

	m_fOriginalLetterSpacing = (f32)m_oSliderSpacingLetter;
	m_fOriginalWordSpacing = (f32)m_oSliderSpacingWord;
	m_fOriginalItalicSlant = (f32)m_oSliderItalicSlant;

} // GenerateFont

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::ReadFnt( BOOL bImporting )
{
	CString Filename, SourceTGA, OutputTGA, OutputPath, Letters, Temp2;
	CStringW Buffer, Temp; //WIDE FORMAT BUFFER!

	u32 i;
	s32 nIndex, nIndex2;
	f32 fLetterSpace, fWordSpace, fItalicSlant;

	if( bImporting )
	{
		CFileDialog dlg(TRUE, _T( "fnt" ), NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, _T( "Script Files (*.fnt)|*.fnt||" ), this );
		if( dlg.DoModal() != IDOK ) return;

		Filename = dlg.GetPathName();
	}
	else
	{
		GetDlgItemText( IDC_INPUT_FILE, m_oTempString );
		nIndex = m_oTempString.ReverseFind( '\\' );
		if( nIndex != -1 )
		{
			m_oTempString = m_oTempString.Mid( nIndex + 1 );
		}
		nIndex = m_oTempString.ReverseFind( '.' );
		if( nIndex != -1 )
		{
			m_oTempString = m_oTempString.Left( nIndex );
		}

		GetDlgItemText( IDC_OUTPUT_DIR, Temp2 );
		Filename = Temp2 + "\\" + m_oTempString + ".fnt";

		FILE *poFile = _tfopen( Filename, _T( "rb" ) );
		if( poFile )
		{
			fclose( poFile );
		}
		else
		{
			m_fOriginalLetterSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->GetPos();
			m_fOriginalWordSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->GetPos();
			m_fOriginalItalicSlant = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->GetPos();

			Temp2.Format( _T( "Could not read %s.fnt, using current slider values." ), m_oTempString );
			SetDlgItemText( IDC_OUTPUT, Temp2 );

			return;
		}
	}

//	CStdioFile file( Filename, CFile::modeRead | CFile::typeBinary );
	CUnicodeFile file;
	if( !file.Open( Filename, UNICODEFILE_OPENMODE_READ ) ) {
		m_oTempString.Format( _T( "Error reading data from %s, the file may not be a unicode format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}

	// find out the source TGA name
	for( i=0; i < 4; i++ ) {
		if( !file.ReadString( Buffer ) ) {
			m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
			SetDlgItemText( IDC_OUTPUT, m_oTempString );
			return;
		}
	}
	nIndex = Buffer.Find( L"FROM: " );
	if( nIndex == -1 ) {
		m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}
	SourceTGA = Buffer.Mid( nIndex + wcslen( L"FROM: " ) );

	// find out the output TGA name
	for( i=0; i < 3; i++ ) {
		if( !file.ReadString( Buffer ) ) {
			m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
			SetDlgItemText( IDC_OUTPUT, m_oTempString );
			return;
		}
	}
	nIndex = Buffer.Find( L"TGA 1: " );
	if( nIndex == -1 ) {
		m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}
	OutputTGA = Buffer.Mid( nIndex + wcslen( L"TGA 1: " ) );
	nIndex = OutputTGA.ReverseFind( '.' );
	if( nIndex == -1 )
	{
		m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}
	OutputTGA.Delete( nIndex - 1, OutputTGA.GetLength() - nIndex + 1 );
	OutputTGA += _T( ".tga" );
	nIndex = OutputTGA.ReverseFind( _T( '\\' ) );
	OutputPath = OutputTGA.Mid( 0, nIndex );

	// find the letter string
	nIndex = -1;
	while( nIndex == -1 ) {
		if( !file.ReadString( Buffer ) ) {
			m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
			SetDlgItemText( IDC_OUTPUT, m_oTempString );
			return;
		}
		nIndex = Buffer.Find( L"TEXT \"" );
	}
	nIndex2 = Buffer.ReverseFind( L'"' );
	if( nIndex2 == -1 ) {
		m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}
	Letters = Buffer.Mid( nIndex + wcslen( L"TEXT \"" ), nIndex2 - ( nIndex + wcslen( L"TEXT \"" ) ) );

	// find the letter spacing
	nIndex = -1;
	while( nIndex == -1 ) {
		if( !file.ReadString( Buffer ) ) {
			m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
			SetDlgItemText( IDC_OUTPUT, m_oTempString );
			return;
		}
		nIndex = Buffer.Find( L"COMMAND FONT_SET_LETTER_SPACING " );
	}
	Temp = Buffer.Mid( nIndex + wcslen( L"COMMAND FONT_SET_LETTER_SPACING " ) );
	swscanf( Temp, L"%f", &fLetterSpace );
	if( ( 0.0f > fLetterSpace ) || ( 128.0f < fLetterSpace ) ) {
		m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}

	// find the word spacing
	nIndex = -1;
	while( nIndex == -1 ) {
		if( !file.ReadString( Buffer ) ) {
			m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
			SetDlgItemText( IDC_OUTPUT, m_oTempString );
			return;
		}
		nIndex = Buffer.Find( L"COMMAND FONT_SET_WORD_SPACING " );
	}
	Temp = Buffer.Mid( nIndex + wcslen( L"COMMAND FONT_SET_WORD_SPACING ") );
	swscanf( Temp, L"%f", &fWordSpace );
	if( ( 0.0f > fWordSpace ) || ( 128.0f < fWordSpace ) ) {
		m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}

	// find the italic slant
	nIndex = -1;
	while( nIndex == -1 ) {
		if( !file.ReadString( Buffer ) ) {
			m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
			SetDlgItemText( IDC_OUTPUT, m_oTempString );
			return;
		}
		nIndex = Buffer.Find( L"COMMAND FONT_SET_ITALIC_SLANT " );
	}
	Temp = Buffer.Mid( nIndex + wcslen( L"COMMAND FONT_SET_ITALIC_SLANT " ) );
	swscanf( Temp, L"%f", &fItalicSlant );
	if( ( -128.0f > fItalicSlant ) || ( 128.0f < fItalicSlant ) ) {
		m_oTempString.Format( _T( "Error reading data from %s, the file may be the wrong format" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		return;
	}

	// ok, now update all our vars

	////
	//
	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->SetPos( (u32)fLetterSpace );
	SetDlgItemInt( IDC_EDIT_SPACING_LETTER, (u32)fLetterSpace, FALSE );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->SetPos( (u32)fWordSpace );
	SetDlgItemInt( IDC_EDIT_SPACING_WORD, (u32)fWordSpace, FALSE );

	((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->SetPos( (s32)fItalicSlant );
	TCHAR szTemp[ 5 ];
	_stprintf( szTemp, _T( "%d" ), (s32)fItalicSlant );
	SetDlgItemText( IDC_EDIT_ITALIC_SLANT, szTemp );

	m_fOriginalLetterSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->GetPos();
	m_fOriginalWordSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->GetPos();
	m_fOriginalItalicSlant = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->GetPos();
	//
	////

	m_bUseLoadedValues = TRUE;
	SetDlgItemText( IDC_CHARACTER_STRING, Letters );
	SetDlgItemText( IDC_INPUT_FILE, SourceTGA );
	SetDlgItemText( IDC_OUTPUT_DIR, OutputPath );

	nIndex = Filename.ReverseFind( _T( '\\' ) );
	if( nIndex != -1 )
	{
		Filename = Filename.Mid( nIndex + 1 );
	}

	if( bImporting )
	{
		m_oTempString.Format( _T( "Data read from %s, you may now press Generate to create a font" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
		((CButton *)GetDlgItem( IDC_BUTTON_PREVIEW ))->EnableWindow( TRUE );
		((CButton *)GetDlgItem( IDC_RUN ))->EnableWindow( TRUE );
	}
	else
	{
		m_oTempString.Format( _T( "Read %s" ), Filename );
		SetDlgItemText( IDC_OUTPUT, m_oTempString );
	}

} // ReadFnt

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnInputFileBrowse()
{
	CFileDialog dlg(TRUE, _T( "tga" ), NULL, 0, _T( "Image Files (*.tga)|*.tga||" ), this );

	if( dlg.DoModal() != IDOK ) return;

	SetDlgItemText( IDC_INPUT_FILE, dlg.GetPathName() );

	TCHAR szTemp[ _MAX_PATH + 1 ];
	fang_MemZero( szTemp, sizeof( szTemp ) );

	_tcscpy( szTemp, dlg.GetPathName() );
	TCHAR *pszTemp = _tcsstr( szTemp, _T( ".tga" ) );

	if( ! pszTemp ) return;

	_tcscpy( pszTemp, _T( ".txt" ) );

	FILE *poFile = _tfopen( szTemp, _T( "rb" ) );
	if( ! poFile ) return;

	fang_MemZero( szTemp, sizeof( szTemp ) );
	if( (u32)filelength( fileno( poFile ) ) != fread( szTemp, 1, filelength( fileno( poFile ) ), poFile ) ) return;
	SetDlgItemText( IDC_CHARACTER_STRING, szTemp );

	((CButton *)GetDlgItem( IDC_BUTTON_PREVIEW ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_RUN ))->EnableWindow( TRUE );

} // OnInputFileBrowse

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnRun()
{
	GenerateFont( FALSE );

} // OnRun

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnOutputScriptFileBrowse()
{
	BROWSEINFO oBrowseInfo;
	TCHAR szDir[ MAX_PATH ];
	LPITEMIDLIST poItemIdList;
	LPMALLOC poMalloc;

	if( SUCCEEDED( SHGetMalloc( &poMalloc ) ) )
	{
		fang_MemZero( &oBrowseInfo, sizeof( oBrowseInfo ) );

		oBrowseInfo.hwndOwner = GetSafeHwnd();
		oBrowseInfo.lpszTitle = TEXT( "Select an output directory" );
		oBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;

		poItemIdList = SHBrowseForFolder( &oBrowseInfo );

		if( poItemIdList )
		{
			if( SHGetPathFromIDList( poItemIdList, szDir ) )
			{
				SetDlgItemText( IDC_OUTPUT_DIR, szDir );
			}

			poMalloc->Free( poItemIdList );
		}

		poMalloc->Release();
	}

} // OnOutputScriptFileBrowse

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnLoad()
{
	ReadFnt( TRUE );

} // OnLoad

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnButtonVidmode()
{
	CVidMode VidMode;

	VidMode.DoModal();

	if( VidMode.GetVidMode( &m_GameloopParm.VidWin ) )
	{
		m_bValidVideoMode = TRUE;
		SaveSettingsToDisk();
	}

} // OnButtonVidmode

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnButtonMasterFile()
{
	CFileDialog oFileDlg( TRUE,
					 _T( ".mst" ),
					 NULL,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 _T( "Master Files (*.mst)|*.mst||" ),
					 this );
	oFileDlg.m_ofn.lpstrTitle = _T( "Select Master File" );

	if( IDOK == oFileDlg.DoModal() )
	{
		// grab the new master file name
		SetDlgItemText( IDC_EDIT_MASTER_FILE, oFileDlg.GetPathName() );
		m_sMasterfileNameAndPath = oFileDlg.GetPathName();
	}

} // OnButtonMasterFile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnButtonPreview()
{
	if( ! m_bValidVideoMode )
	{
		OnButtonVidmode();
		if( ! m_bValidVideoMode )
		{
			return;
		}
	}

	int nIndex;

	GetDlgItemText( IDC_INPUT_FILE, m_oTempString );
	if( m_oTempString.IsEmpty() )
	{
		SetDlgItemText( IDC_OUTPUT, _T( "Please Enter a source TGA filename!" ) );
		return;
	}

	// we need to get the base output tga file name (no more than 10 chars)
	nIndex = m_oTempString.ReverseFind( _T( '\\' ) );
	if( nIndex != -1 ) {
		m_sFontName = m_oTempString.Mid( nIndex + 1 );
	} else {
		m_sFontName = m_oTempString;
	}
	nIndex = m_sFontName.ReverseFind( _T( '.' ) );
	if( nIndex != -1 ) {
		m_sFontName.Delete( nIndex, m_sFontName.GetLength() - nIndex );
	}
	else
	{
		SetDlgItemText( IDC_OUTPUT, _T( "Please Enter a valid source TGA filename!" ) );
		return;
	}

	if( m_sFontName.GetLength() > 10 ) {
		SetDlgItemText( IDC_OUTPUT, _T( "Output TGA filename must be less than 10 characters!" ) );
		return;
	}

	// Start up...
	GetDlgItemText( IDC_EDIT_MASTER_FILE, m_sMasterfileNameAndPath );
	if( m_sMasterfileNameAndPath.IsEmpty() )
	{
		// no file selected
		MessageBox( _T( "You need to select a master file first.\nClick Ok and then click the Browse button." ), 
			        _T( "Fontomatic Error: No Master File Selected" ) );
		return;
	}
	SaveSettingsToDisk();

	((CButton *)GetDlgItem( IDC_RUN ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDC_RUN_SCRIPT ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_EDIT_SPACING_LETTER ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_EDIT_SPACING_WORD ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->EnableWindow( ! ((CButton *)GetDlgItem( IDC_CHECK_FIXED_WIDTH ))->GetCheck() );
	((CButton *)GetDlgItem( IDC_EDIT_SPACING_LINE ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_SLIDER_SPACING_LINE ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_EDIT_NUMBER_LINES ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_EDIT_ITALIC_SLANT ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_EDIT_PREVIEW ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_CHECK_FIXED_WIDTH ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_CHECK_ONE_TO_ONE ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_CHECK_DRAW_GUIDES ))->EnableWindow( TRUE );
	((CButton *)GetDlgItem( IDC_CHECK_ITALIC ))->EnableWindow( TRUE );

	((CButton *)GetDlgItem( IDC_LOAD ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDC_INPUT_FILE_BROWSE ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDC_INPUT_CHARS_BROWSE ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDC_OUTPUT_SCRIPT_FILE_BROWSE ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDC_BUTTON_PREVIEW ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDC_BUTTON_MASTER_FILE ))->EnableWindow( FALSE );
	((CButton *)GetDlgItem( IDC_BUTTON_VIDMODE ))->EnableWindow( FALSE );

	((CEdit *)GetDlgItem( IDC_CHARACTER_STRING ))->SetReadOnly( TRUE );
	((CEdit *)GetDlgItem( IDC_INPUT_FILE ))->SetReadOnly( TRUE );

	ReadFnt( FALSE );

	m_fOriginalLetterSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->GetPos();
	m_fOriginalWordSpacing = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->GetPos();
	m_fOriginalItalicSlant = (f32)((CSliderCtrl *)GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->GetPos();

	// go ahead and re-read the master file, just in case things were compiled in since launching the app
#ifdef _UNICODE
	fclib_WideToChar( m_szMasterfileNameAndPath, m_sMasterfileNameAndPath );
#else
	strcpy( m_szMasterfileNameAndPath, m_sMasterfileNameAndPath );
#endif

	Fang_ConfigDefs.pszFile_MasterFilePathName = m_szMasterfileNameAndPath;

	ffile_SetupFileMode();

	UpdateData( TRUE );

	floop_InstallGameloop( _GameInit, _GameMain, _GameTerm, &m_GameloopParm, 60 );

} // OnButtonPreview

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnRunScript()
{
	UpdateData( TRUE );
	GenerateFont( TRUE );

} // OnRunScript

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnInputCharsBrowse()
{
	CFileDialog dlg(TRUE, _T( "txt" ), NULL, 0, _T( "Text Files (*.txt)|*.txt||" ), this );

	if( dlg.DoModal() == IDOK )
	{
		FILE *poFile = _tfopen( dlg.GetPathName(), _T( "rb" ) );
		if( ! poFile ) return;

		TCHAR szTemp[ 256 + 1 ];
		fang_MemZero( szTemp, sizeof( szTemp ) );
		if( (u32)filelength( fileno( poFile ) ) != fread( szTemp, 1, filelength( fileno( poFile ) ), poFile ) ) return;
		SetDlgItemText( IDC_CHARACTER_STRING, szTemp );
	}

} // OnInputCharsBrowse

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( GetDlgItem( IDC_SLIDER_SPACING_LETTER ) == pScrollBar )
	{
		SetDlgItemInt( IDC_EDIT_SPACING_LETTER, ((CSliderCtrl *)pScrollBar)->GetPos(), FALSE );
		UpdateData( TRUE );
	}
	else if( GetDlgItem( IDC_SLIDER_SPACING_WORD ) == pScrollBar )
	{
		SetDlgItemInt( IDC_EDIT_SPACING_WORD, ((CSliderCtrl *)pScrollBar)->GetPos(), FALSE );
		UpdateData( TRUE );
	}
	else if( GetDlgItem( IDC_SLIDER_SPACING_LINE ) == pScrollBar )
	{
		SetDlgItemInt( IDC_EDIT_SPACING_LINE, ((CSliderCtrl *)pScrollBar)->GetPos(), FALSE );
		UpdateData( TRUE );
	}
	else if( GetDlgItem( IDC_SLIDER_NUMBER_LINES ) == pScrollBar )
	{
		SetDlgItemInt( IDC_EDIT_NUMBER_LINES, ((CSliderCtrl *)pScrollBar)->GetPos(), FALSE );
		UpdateData( TRUE );
	}
	else if( GetDlgItem( IDC_SLIDER_ITALIC_SLANT ) == pScrollBar )
	{
		TCHAR szTemp[ 5 ];
		_stprintf( szTemp, _T( "%d" ), ((CSliderCtrl *)pScrollBar)->GetPos() );
		SetDlgItemText( IDC_EDIT_ITALIC_SLANT, szTemp );
		UpdateData( TRUE );
	}

	CDialog::OnHScroll( nSBCode, nPos, pScrollBar );

} // OnHScroll

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _GameInit( void *pParameter )
{
	floop_SetTargetFramesPerSec( 30.0f );
	floop_EnableGovernor( TRUE );

	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CFontomaticDlg *pDlg = pParm->pDlg;

	_bMainLoopExecuted = FALSE;

	if( !fvid_CreateWindow( &pParm->VidWin ) )
	{
		pDlg->MessageBox( _T( "Could not use the current video mode.\nTry selecting a different video mode." ), 
			              _T( "Fontomatic: fvid_CreateWindow() Failed" ) );
		return FALSE;
	}

	fdx8vid_EnableCursor( TRUE );
	fdx8vid_SetCursor( LoadCursor( NULL, IDC_ARROW ) );

	_oResFrame = fres_GetFrame();

	///////////////////////
	// init the text system
	if( FTEXT_ERROR == ftext_Install() )
	{
		fvid_DestroyWindow();
		pDlg->MessageBox( _T( "Could not init the text system." ), _T( "Text System Problem" ) );
		return FALSE;
	}

	ftext_GetAttributes( 0 )->ohFont = _FONT_HANDLE;
	ftext_GetAttributes( 0 )->oColorBorder.OpaqueWhite();

	char szFontName[255];
#ifdef _UNICODE
	fclib_WideToChar( szFontName, pDlg->m_sFontName );
#else
	strcpy( szFontName, pDlg->m_sFontName );
#endif
	if( FTEXT_ERROR == ftext_Load( _FONT_HANDLE, szFontName ) )
	{
		fvid_DestroyWindow();
		pDlg->MessageBox( _T( "Could not load font.\nTry PASM'ing first." ), _T( "Text System Problem" ) );
		return FALSE;
	}

	_szTempExpanded[ 203 ] = 0;
	_szTempCRLF[ 200 ] = 0;
	_szTemp[ 200 ] = 0;

	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;

	return TRUE;

} // _GameInit

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _GameTerm( FLoopTermCode_t nTermCode, void *pParameter )
{
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CFontomaticDlg *pDlg = pParm->pDlg;

	// shutdown the text system
	ftext_Uninstall();

	fvid_DestroyWindow();

	if( ! pDlg->m_bExiting )
	{
		((CButton *)pDlg->GetDlgItem( IDC_RUN ))->EnableWindow( TRUE );
		((CButton *)pDlg->GetDlgItem( IDC_RUN_SCRIPT ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_EDIT_SPACING_LETTER ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_SLIDER_SPACING_LETTER ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_EDIT_SPACING_WORD ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_SLIDER_SPACING_WORD ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_EDIT_SPACING_LINE ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_SLIDER_SPACING_LINE ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_EDIT_NUMBER_LINES ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_EDIT_ITALIC_SLANT ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_SLIDER_ITALIC_SLANT ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_EDIT_PREVIEW ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_CHECK_FIXED_WIDTH ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_CHECK_ONE_TO_ONE ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_CHECK_DRAW_GUIDES ))->EnableWindow( FALSE );
		((CButton *)pDlg->GetDlgItem( IDC_CHECK_ITALIC ))->EnableWindow( FALSE );

		((CButton *)pDlg->GetDlgItem( IDC_LOAD ))->EnableWindow( TRUE );
		((CButton *)pDlg->GetDlgItem( IDC_INPUT_FILE_BROWSE ))->EnableWindow( TRUE );
		((CButton *)pDlg->GetDlgItem( IDC_INPUT_CHARS_BROWSE ))->EnableWindow( TRUE );
		((CButton *)pDlg->GetDlgItem( IDC_OUTPUT_SCRIPT_FILE_BROWSE ))->EnableWindow( TRUE );
		((CButton *)pDlg->GetDlgItem( IDC_BUTTON_PREVIEW ))->EnableWindow( TRUE );
		((CButton *)pDlg->GetDlgItem( IDC_BUTTON_MASTER_FILE ))->EnableWindow( TRUE );
		((CButton *)pDlg->GetDlgItem( IDC_BUTTON_VIDMODE ))->EnableWindow( TRUE );

		((CEdit *)pDlg->GetDlgItem( IDC_CHARACTER_STRING ))->SetReadOnly( FALSE );
		((CEdit *)pDlg->GetDlgItem( IDC_INPUT_FILE ))->SetReadOnly( FALSE );

		pDlg->PromptUserToSaveFont( FALSE );
	}

} // _GameTerm

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _GameMain( BOOL bExitRequest, void *pParameter )
{
	_GameloopParm_t *pParm = (_GameloopParm_t *)pParameter;
	CFontomaticDlg *pDlg = pParm->pDlg;
	BOOL bKeyDown, bHaveFocus;

	if( bExitRequest || fvid_HasUserClosedWindow() ) { 
		return FALSE;
	}

	_bMainLoopExecuted = TRUE;

	bHaveFocus = fdx8vid_HaveFocus();

	if( !bHaveFocus ) {
		// allow other windows to get some processing
		Sleep( 0 );
	}

	bKeyDown = bHaveFocus && ( GetAsyncKeyState( VK_ESCAPE ) < 0 );
	if( bKeyDown ) {
		return FALSE;
	}

	bKeyDown = bHaveFocus && ( GetAsyncKeyState( VK_BACK ) < 0 );
	if( bKeyDown ) {
		pDlg->SetForegroundWindow();
	}

	// reset the camera position
	bKeyDown = bHaveFocus && (GetAsyncKeyState( VK_SPACE ) < 0);

	fvid_Begin();

	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

	ftext_SetLetterSpacing( _FONT_HANDLE, (f32)pDlg->m_oSliderSpacingLetter );
	ftext_SetWordSpacing( _FONT_HANDLE, (f32)pDlg->m_oSliderSpacingWord );
	ftext_SetItalicSlant( _FONT_HANDLE, (f32)pDlg->m_oSliderItalicSlant );
	ftext_GetAttributes( 0 )->fLineSpacing = ( (f32)pDlg->m_oSliderSpacingLine * ( 1.0f / 1000.0f ) );
	ftext_GetAttributes( 0 )->fNumberOfLines = (f32)pDlg->m_oSliderNumberLines;
	ftext_GetAttributes( 0 )->bDrawGuides = pDlg->m_oCheckDrawGuides;
	ftext_GetAttributes( 0 )->bFixedWidthFont = pDlg->m_oCheckFixedWidth;
	ftext_GetAttributes( 0 )->bItalicFont = pDlg->m_oCheckItalic;

	_tcsncpy( _szTempCRLF, pDlg->m_oEditPreview, 200 );


	for( u32 uIndex = 0, uTemp = 0; uIndex < 200; ++uIndex )
	{
		if( _szTempCRLF[ uIndex ] )
		{
			if( '\r' != _szTempCRLF[ uIndex ] )
			{
				_szTemp[ uTemp++ ] = _szTempCRLF[ uIndex ];
			}
		}
		else
		{
			_szTemp[ uTemp ] = 0;
			break;
		}
	}

	if( pDlg->m_oOneToOne )
	{
		ftext_DebugPrintString( 0.0f, 0.0f, _szTemp );
	}
	else
	{
		ftext_PrintString( 0, _szTemp );
	}

	fvid_End();

	if( ! fvid_Swap() ) return FALSE;

	return TRUE;

} // _GameMain

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::PromptUserToSaveFont( BOOL bSaveToDisk )
{
	static BOOL _bAlreadyInFunction = FALSE;

	if( !_bAlreadyInFunction ) {
		// make thread friendly
		_bAlreadyInFunction = TRUE;

		if( ( (s32)m_fOriginalWordSpacing   != m_oSliderSpacingWord   ) ||
			( (s32)m_fOriginalLetterSpacing != m_oSliderSpacingLetter ) ||
			( (s32)m_fOriginalItalicSlant   != m_oSliderItalicSlant   ) )
		{
			// ask if they want to save
			if( MessageBox( _T( "Save changes to script?" ), _T( "Fontomatic" ), MB_YESNO | MB_ICONEXCLAMATION ) == IDYES )
			{
				if( bSaveToDisk ) {
					GenerateFont( TRUE ); 
				} else {
					PostMessage( WM_COMMAND, IDC_RUN_SCRIPT, 0 );
				}
			}
		}
		
		_bAlreadyInFunction = FALSE;
	}

} // PromptUserToSaveFont

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// [mike] added so that all tools are consistant with each other, don't remove.
BOOL CFontomaticDlg::PreTranslateMessage( MSG *pMsg )
{
	if( m_hAccel != NULL )
	{
		if( ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) )
		{
			return TRUE;
		}
	}
	if( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_ESCAPE )
		{
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);

} // PreTranslateMessage

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// [mike] added so that all tools are consistant with each other, don't remove.
void CFontomaticDlg::OnOK( void )
{
	//OnCancel();

} // OnOK

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnCheckFixedWidth()
{
	((CButton *)GetDlgItem( IDC_SLIDER_SPACING_WORD ))->EnableWindow( ! ((CButton *)GetDlgItem( IDC_CHECK_FIXED_WIDTH ))->GetCheck() );
	UpdateData( TRUE );

} // OnCheckFixedWidth

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnChangeEditPreview()
{
	UpdateData( TRUE );

} // OnChangeEditPreview

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnCheckDrawGuides()
{
	UpdateData( TRUE );

} // OnCheckDrawGuides

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnCheckItalic()
{
	UpdateData( TRUE );

} // OnCheckItalic

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFontomaticDlg::OnCheckOneToOne()
{
	((CButton *)GetDlgItem( IDC_SLIDER_NUMBER_LINES ))->EnableWindow( ! ((CButton *)GetDlgItem( IDC_CHECK_ONE_TO_ONE ))->GetCheck() );
	UpdateData( TRUE );

} // OnCheckOneToOne

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
