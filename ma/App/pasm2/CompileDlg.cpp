//////////////////////////////////////////////////////////////////////////////////////
// CompileDlg.cpp -
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
#include "fang.h"
#include "pasm.h"
#include "CompileDlg.h"
#include "ConvertMtxFiles.h"
#include "ErrorLog.h"
#include "CreateXBoxTgaFile.h"
#include "CreateGCTgaFile.h"
#include "GenMipMaps.h"
#include "TGAFileLoader.h"
#include "KongToMLMesh.h"
#include "ApeToWorldFile.h"
#include "Settings.h"
#include "ConvertCsvFiles.h"
#include "ConvertFntFiles.h"
#include "ConvertSmaFiles.h"
#include "ConvertGtFiles.h"
#include "aidfile.h"
#include "ConvertWvbFile_XBox.h"
#include "ConvertSfbFiles.h"
#include "ConvertFprFiles.h"
#include "ConvertCamFiles.h"
#include "NormalMapGen.h"
#include "PasmDlg.h"
#include "KongDef.h"

// From NVidia stripping library
#include "NvTriStrip.h"

#ifdef _MMI_TARGET_PS2
#include "MMI_CreatePS2TgaFile.h"   // KJ
#include "MMI_ApeToPS2File.h"		// KJ
#include "MMI_ApeToPS2WorldFile.h"	// KJ

	u32 nCollisionMeshBytes;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define _ENABLE_PASM_FILE_LOG		FALSE

#define _SAVE_KONG_FILES			TRUE

#define _MAX_PRECOMPILE_FILES		8192
#define _PRECOMPILE_DEPENDENCIES	TRUE
#define _FORCE_RECOMPILE_ALSO_FORCES_COMPILATION_OF_DEPENDENCIES	FALSE

extern CPasmApp theApp;

CMasterFileCompile CCompileDlg::m_MasterFile;

u32 CompileDlg_nWorldStartTime;

HANDLE _hFileLog = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS TO COMPILE ASSETS:

UINT WorkerThread( LPVOID pParam );

// xbox converters
BOOL ConvertTgaFile_XB( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,	CAidFile &rAidFile, CFileInfo *pAidFileInfo );
BOOL ConvertWvbFile_XB( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );

// gc converters
BOOL ConvertTgaFile_GC( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,	CAidFile &rAidFile, CFileInfo *pAidFileInfo );

#ifdef _MMI_TARGET_PS2
// PS2 converters
BOOL ConvertTgaFile_PS2( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
									 CAidFile &rAidFile, CFileInfo *pAidFileInfo );
#endif

// generic converters (where a file type does not need to get compiled by a platform specific function)
BOOL ConvertMtxFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename, CAidFile &rAidFile, CFileInfo *pAidFileInfo );
BOOL ConvertGenericFiles( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );
BOOL ConvertCsvFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );
BOOL ConvertFntFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );
BOOL ConvertSmaFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );
BOOL ConvertGtFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );
BOOL ConvertSfbFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );
BOOL ConvertFprFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename );
BOOL ConvertApeFile( TargetPlatform_e nPlatform, const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
								CKongToVisFile &rVisFile, CAidFile &AidFile, const CFileInfo *pAidFileInfo );
BOOL ConvertWldFile( TargetPlatform_e nPlatform, const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
								CKongToVisFile &rVisFile, CAidFile &AidFile, const CFileInfo *pAidFileInfo );
BOOL ConvertCamFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
    				 CAidFile &rAidFile, CFileInfo *pAidFileInfo );

// support routines
BOOL ApePreConvert( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CString &rsOutputFilename, CKongToVisFile &rVisFile,
								BOOL &rbNeedsCompiling, CFileInfo **ppAidFileInfo, CAidFile &AidFile );
BOOL WldPreConvert( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CString &rsOutputFilename, CKongToVisFile &rVisFile,
								BOOL &rbNeedsCompiling, CFileInfo **ppAidFileInfo, CAidFile &AidFile );
BOOL PreConvert( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CString &rsOutputFilename, CAidFile &rAidFile, BOOL &rbNeedsCompiling, CFileInfo **ppAidFileInfo );
BOOL DoesAidFileNeedCompiled( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CFileInfo **ppAidFileInfo );
void CreateAidEntryName( const CFileInfo *pFileInfo, CString &rsEntryName );
void StripExtension( CString &sString );



/////////////////////////////////////////////////////////////////////////////
// CCompileDlg dialog


// Static class vars
CString	CCompileDlg::m_sClickAnywhere;
CString	CCompileDlg::m_sOpName;
CString	CCompileDlg::m_sSubOpName;
CProgressCtrl CCompileDlg::m_ctrlOpProgressBar;
CProgressCtrl CCompileDlg::m_ctrlSubOpProgressBar;
CRichEditCtrl CCompileDlg::m_TextStatus;
CHARFORMAT2 CCompileDlg::m_DefaultCharFormat;
u32 CCompileDlg::m_nCurrentSubOp;

CCompileDlg::CCompileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCompileDlg::IDD, pParent), m_MapAidFiles( 25 ) {

	m_pWorkThread = NULL;
	m_nNumLeftClicks = 0;
	m_bAbortCompile = FALSE;
	m_bGenerateLMSTs = FALSE;
	m_bGenerateVertRad = FALSE;
	m_bBuildingLightMaps = FALSE;
	m_bCheckLibraryKNGFirst = FALSE;
	m_bIgnoreVIS = FALSE;
	m_bReusePerfectVISMatchOnly = FALSE;

	// !!Nate
	m_uNumCompiledFiles = 0;

	m_bCompilingPASMGeneratedAsset = FALSE;
}


void CCompileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ctrlProgressBar);
	DDX_Control( pDX, IDC_OP_PROGRESS, m_ctrlOpProgressBar );
	DDX_Control( pDX, IDC_SUBOP_PROGRESS, m_ctrlSubOpProgressBar );
	DDX_Control( pDX, IDC_COMPILE_RESULTS, m_TextStatus );
	DDX_Text( pDX, IDC_CLICK_ANYWHERE, m_sClickAnywhere );
	DDX_Text( pDX, IDC_OP_TEXT, m_sOpName );
	DDX_Text( pDX, IDC_SUBOP_TEXT, m_sSubOpName );
}


BEGIN_MESSAGE_MAP(CCompileDlg, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	ON_MESSAGE( CDLG_UPDATE_CONTROLS, UpdateControls )
	ON_MESSAGE( CDLG_PROCESSING_COMPLETE, ProcessingComplete )
	ON_MESSAGE( CDLG_INCREMENT_OP_PROGRESS, IncrementOpProgress )
	ON_MESSAGE( CDLG_INCREMENT_SUBOP_PROGRESS, IncrementSubOpProgress )
	ON_MESSAGE( CDLG_SET_SUBOP_PROGRESS, SetSubOpProgress )
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CCompileDlg message handlers

BOOL CCompileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here

	int nNumFiles = m_FilesToCompile.GetSize();
	m_ctrlProgressBar.SetRange( 0, nNumFiles );
	m_ctrlProgressBar.SetPos( 0 );
	m_ctrlProgressBar.SetStep( 1 );

	///////////////////
	// Adding STATUSBAR
	///////////////////
	int nTotWide;		// total width of status bar
	CRect rect;
	this->GetWindowRect(&rect);
	rect.top = rect.bottom- 25;

	BOOL bRvStatOk = m_StatusBar.Create( WS_CHILD | WS_VISIBLE, rect, this, IDC_PROGRESS_STATUSBAR );// | WS_BORDER

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

		// set each section's width
		int nWidths[3];
		nWidths[0] = (int)( (f32)nTotWide * 0.6f );// where is the right edge
   		nWidths[1] = (nTotWide/5) * 4;
   		nWidths[2] = -1;

		m_StatusBar.SetMinHeight( 25 );
		m_StatusBar.SetParts( 3, nWidths );

		// now lets put some text, different styles into parts of status bar
		CString s;
		s.Format( " Preparing to compile..." );
		m_StatusBar.SetText( s, 0, 0 );
		s.Format( " 0%% Done" );
		m_StatusBar.SetText( s, 1, 0 );
		switch( m_nTargetPlatform ) {
		case PASM_TARGET_XB:
			s.Format( " XBOX" );
			break;
		case PASM_TARGET_PC:
			s.Format( " PC" );
			break;
		case PASM_TARGET_GC:
			s.Format( " GameCube" );
			break;
#ifdef _MMI_TARGET_PS2
        case PASM_TARGET_PS2:                           // KJ
			s.Format( " PS2" );				// KJ
			break;							// KJ
#endif
		default:
			s.Format( " UNKNOWN" );
			break;
		}
		m_StatusBar.SetText( s, 2, 0 );
	}

	m_nNumLeftClicks = 0;
	m_bAbortCompile = FALSE;

	m_DefaultCharFormat.cbSize = sizeof( CHARFORMAT2 );
	m_DefaultCharFormat.dwMask = CFM_COLOR|CFM_FACE|CFM_SIZE|CFM_BOLD;
	strcpy( m_DefaultCharFormat.szFaceName, "Courier New" );
	m_DefaultCharFormat.crTextColor = RGB( 10, 10, 10 );
	m_DefaultCharFormat.yHeight = 135;
	m_DefaultCharFormat.dwEffects = 0;
	BOOL bResult = m_TextStatus.SetDefaultCharFormat( m_DefaultCharFormat );
	FASSERT( bResult );

	if( m_bUpdateMasterFileVersion ) {
		// disable the abort button if we are compiling to update the master file version for some asset types
		((CButton *)GetDlgItem( IDC_ABORT ))->EnableWindow( FALSE );
	}

	UpdateData( VARS_TO_CONTROLS );

	// the very last thing is to wake up the worker thread
	m_pWorkThread->ResumeThread();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CCompileDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage( pMsg );
}

int CCompileDlg::DoModal() {

	////////////////////////////////////////
	// grab some vars from the settings file
	CSettings& Settings = CSettings::GetCurrent();
	////////////////////////////////////////////////

	m_sOpName = "";
	m_sSubOpName = "";

	// build our DL & AID map vars so that we can lookup these files very quickly
	CFileInfoArray::BuildMapFromFileInfoArray( m_MapAidFiles, m_AidFiles, CFileInfoArray::AP_IGNORE_CASE | CFileInfoArray::AP_COMPARE_FILENAME_ONLY );

	m_sClickAnywhere.Empty();

	// spawn a thread that is asleep till init dialog is done
	m_pWorkThread = AfxBeginThread( WorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );

	return CDialog::DoModal();
}

// this is so that our worker thread can send _UPDATE_CONTROLS messages
// to the parent thread and have the controls updated
long CCompileDlg::UpdateControls( WPARAM wParam, LPARAM lParam ) {
	UpdateData( VARS_TO_CONTROLS );
	return 0;
}

void CCompileDlg::OnLButtonDown( UINT nFlags, CPoint point ) {

	m_nNumLeftClicks++;

	CDialog::OnLButtonDown(nFlags, point);
}

void CCompileDlg::OnAbort() {
	m_bAbortCompile = TRUE;
	m_nNumLeftClicks++;
}

//
//
//
void CCompileDlg::ResetOp( char *pszOpName, u32 nSubOpCount )
{
	m_sOpName = pszOpName;

	WriteToLog( "DLG OP: %s\n", pszOpName );

	m_ctrlOpProgressBar.SetRange( 0, nSubOpCount );
	m_ctrlOpProgressBar.SetPos( 0 );
	m_ctrlOpProgressBar.SetStep( 1 );
}

//
//
//
void CCompileDlg::ResetSubOp( char *pszSubOpName, u32 nSubOpUpdateCount )
{
	m_sSubOpName = pszSubOpName;

	WriteToLog( "DLG SUB-OP: %s\n", pszSubOpName );

	m_ctrlSubOpProgressBar.SetRange( 0, nSubOpUpdateCount );
	m_ctrlSubOpProgressBar.SetPos( 0 );
	m_ctrlSubOpProgressBar.SetStep( 1 );
	m_nCurrentSubOp = m_ctrlOpProgressBar.StepIt();
}

//
//
//
long CCompileDlg::IncrementOpProgress( WPARAM wParam, LPARAM lParam )
{
	m_ctrlOpProgressBar.StepIt();
	UpdateData( FALSE );
	return 0;
}

//
//
//
long CCompileDlg::ProcessingComplete( WPARAM wParam, LPARAM lParam )
{
	m_ctrlSubOpProgressBar.SetRange( 0, 1 );
	m_ctrlSubOpProgressBar.SetPos( 1 );
	m_ctrlOpProgressBar.SetRange( 0, 1 );
	m_ctrlOpProgressBar.SetPos( 1 );
	UpdateData( FALSE );
	return 0;
}


//
//
//
long CCompileDlg::IncrementSubOpProgress( WPARAM wParam, LPARAM lParam )
{
	if ( (u32)wParam == m_nCurrentSubOp )
	{
		m_ctrlSubOpProgressBar.StepIt();
		UpdateData( FALSE );
	}
	return 0;
}

//
//
//
long CCompileDlg::SetSubOpProgress( WPARAM wParam, LPARAM lParam )
{
	if ( (u32)wParam == m_nCurrentSubOp )
	{
		m_ctrlSubOpProgressBar.SetPos( (u32)lParam );
		UpdateData( FALSE );
	}
	return 0;
}


//
//
//
void CCompileDlg::InfoString( const char *pszFormat, ... )
{
	FANG_VA_LIST args;
	FANG_VA_START( args, pszFormat );
	char szFormattedString[512];
	char szFinalText[512];

	_vstprintf( szFormattedString, pszFormat, args );
	sprintf( szFinalText, "%s\n", szFormattedString );

	// Send it also to the log
	WriteToLog( szFinalText );

	CHARRANGE cr = {-1, -2};
	m_TextStatus.SendMessage(EM_EXSETSEL, 0, (LPARAM) &cr);
	m_TextStatus.SendMessage(EM_REPLACESEL, 0, (LONG) (LPCTSTR) szFinalText);
	m_TextStatus.SendMessage(EM_SCROLLCARET, 0, 0L);

	va_end( args );
}


//
//
//
void CCompileDlg::ErrorString( const char *pszFormat, ... )
{
	FANG_VA_LIST args;
	FANG_VA_START( args, pszFormat );
	char szFormattedString[512];
	char szFinalText[512];

//	static CHARFORMAT2 CharFormat;
//	CharFormat.cbSize = sizeof( CHARFORMAT2 );
//	CharFormat.dwMask = CFM_COLOR|CFM_FACE|CFM_SIZE;
//	strcpy( CharFormat.szFaceName, "Courier New" );
//	CharFormat.crTextColor = RGB( 255, 0, 0 );
//	CharFormat.yHeight = 135;
//	CharFormat.dwEffects = 0;
//	BOOL bResult = m_TextStatus.SetDefaultCharFormat( CharFormat );
//	FASSERT( bResult );

	_vstprintf( szFormattedString, pszFormat, args );
	sprintf( szFinalText, "%s\n", szFormattedString );

	// Send it also to the log
	WriteToLog( szFinalText );

	CHARRANGE cr = {-1, -2};
	m_TextStatus.SendMessage(EM_EXSETSEL, 0, (LPARAM) &cr);
	m_TextStatus.SendMessage(EM_REPLACESEL, 0, (LONG) (LPCTSTR) szFinalText);
	m_TextStatus.SendMessage(EM_SCROLLCARET, 0, 0L);

	va_end( args );
}


//
//
//
void CCompileDlg::WriteToLog( const char *pszFormat, ... )
{
	if ( !_hFileLog )
	{
		return;
	}

	FANG_VA_LIST args;
	FANG_VA_START( args, pszFormat );
	char szFormattedString[512];
//	char szFinalText[512];

	_vstprintf( szFormattedString, pszFormat, args );
//	sprintf( szFinalText, "%s\xd0", szFormattedString );

//	fwrite( szFormattedString, strlen( szFormattedString ), 1, _pFileLog );

	u32 nBytesWritten = 0;
	WriteFile( _hFileLog, szFormattedString, strlen( szFormattedString ), (LPDWORD)&nBytesWritten, NULL );
	FlushFileBuffers( _hFileLog );

	va_end( args );
}


///////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS TO COMPILE ASSETS:
///////////////////////////////////////////////////////////////////////////////////////////////


UINT WorkerThread( LPVOID pParam ) {
	CCompileDlg *pDlg = (CCompileDlg *)pParam;
	const CFileInfo *pFileInfoData, *pFileInfo;
	u32 nFilesLeft, nFilesCompiled = 0, nConversionErrors = 0;
	CString s, sFilename, csAllFiles;
	BOOL bReturn, bFileResult;
	WIN32_FIND_DATA FileData;
	HANDLE hFile;

	int nNumFiles = pDlg->m_FilesToCompile.GetSize();

	// prepare the master file for compilation
	switch( pDlg->m_nTargetPlatform ) {
	case PASM_TARGET_XB:
		bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_XBOX, pDlg->m_bRebuildAll );
		break;
	case PASM_TARGET_GC:
		bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_GC, pDlg->m_bRebuildAll );
		break;
	case PASM_TARGET_PC:
		bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_PC, pDlg->m_bRebuildAll );
		break;
#ifdef _MMI_TARGET_PS2
    case PASM_TARGET_PS2:   // KJ
		bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_PS2, pDlg->m_bRebuildAll );			// KJ
		break;	// KJ
#endif
	default:
		bReturn = FALSE;
		break;
	}
	if( !bReturn ) {
		// ask the user if they would like to start a new master file
		if( pDlg->MessageBox( _T("Error opening the master file, it is probably corrupted.\nWould you like to start a new master file?"),
							  _T("PASM ERROR"),
							  MB_ICONHAND | MB_YESNO | MB_DEFBUTTON1 ) == IDYES ) {
			// try again with the rebuild all == true
			switch( pDlg->m_nTargetPlatform ) {
			case PASM_TARGET_XB:
				bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_XBOX, TRUE );
				break;
			case PASM_TARGET_GC:
				bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_GC, TRUE );
				break;
			case PASM_TARGET_PC:
				bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_PC, TRUE );
				break;
#ifdef _MMI_TARGET_PS2
            case PASM_TARGET_PS2:
				bReturn = pDlg->m_MasterFile.BeginCompilation( pDlg->m_sMasterFilename, nNumFiles, FVERSION_PLATFORM_FLAG_PS2, TRUE );
				break;
#endif
			default:
				bReturn = FALSE;
				break;
			}
			if( !bReturn ) {
				// still unable to open the master file
				pDlg->MessageBox( _T("Still cannot open the master file, there might be a serious disk drive problem." ),
								  _T("PASM ERROR"),
								  MB_ICONHAND | MB_OK | MB_DEFBUTTON1 );
				pDlg->PostMessage( WM_COMMAND, IDCANCEL, 0 );
				return 0;
			}
		} else {
			pDlg->PostMessage( WM_COMMAND, IDCANCEL, 0 );
			return 0;
		}
	}

	// !!Nate
	pDlg->m_uNumCompiledFiles = 0;

	pDlg->m_bAbortCompile = FALSE;

//	_pFileLog = fopen( "PASM_LOG.TXT", "w" );
#if _ENABLE_PASM_FILE_LOG
	_hFileLog = CreateFile( "PASM_LOG.DOC", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
#endif

	// iterate through the files to compile
	pFileInfoData = pDlg->m_FilesToCompile.GetData();
	for( int i=0; i < nNumFiles; i++ ) {
		pFileInfo = &pFileInfoData[i];

		nFilesLeft = nNumFiles - i;
		sFilename = pFileInfo->GetFileName();
		sFilename.MakeLower();

		CString csTemp;
		csTemp = "Processing " + sFilename + "...";
		pDlg->ResetOp( csTemp.GetBuffer(0), 14 );
		pDlg->ResetSubOp( "Preparing for compilation...", 1 );
		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );

		if( nFilesLeft == 1 ) {
			s.Format( " Compiling: %s...", (cchar *)sFilename );
		} else {
			s.Format( " (%d More) Compiling: %s...", nFilesLeft, (cchar *)sFilename );
		}

		pDlg->m_StatusBar.SetText( s, 0, 0 );
		pDlg->InfoString( "\n-------------------------------------------------------------------" );
		pDlg->InfoString( s );
		pDlg->InfoString( "-------------------------------------------------------------------" );

		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );

		u32 nStartTime = timeGetTime();

		// do conversion here
		if( !CallConverter( pFileInfo, pDlg, nFilesCompiled ) ) {
			nConversionErrors++;

			if( pDlg->m_bUpdateMasterFileVersion ) {
				// since this is a version update and we weren't able to compile this file
				// remove any old compiled version from the master file
				pDlg->m_MasterFile.DeleteEntry( pFileInfo->GetFileName() );
			}
		}
		else
		{
			// !!Nate
			++pDlg->m_uNumCompiledFiles;
		}

		f32 fTimeToCompile = (timeGetTime() - nStartTime) / 1000.f;
		if ( fTimeToCompile > 3600 )
		{
			pDlg->InfoString( "Time to compile: %d hours, %d minutes", (u32)(fTimeToCompile / 3600.f), (u32)(fTimeToCompile / 60.f) % 60 );
		}
		else if ( fTimeToCompile > 60 )
		{
			pDlg->InfoString( "Time to compile: %d minutes, %d seconds", (u32)(fTimeToCompile / 60.f), (u32)fTimeToCompile % 60 );
		}
		else
		{
			pDlg->InfoString( "\nTime to compile: %5.2f seconds", (f32)(timeGetTime() - nStartTime) / 1000.f );
		}

		// update the % done
		s.Format( " %d%% Done", (u32)( (f32)(i+1)/(f32)nNumFiles * 100) );
		pDlg->m_StatusBar.SetText( s, 1, 0 );

		// advance progress bar
		pDlg->m_ctrlProgressBar.StepIt();

		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );

		// allow other windows apps to still function
		Sleep( 0 );

		pDlg->ResetSubOp( "Compilation Complete!", 1 );
		pDlg->PostMessage( CDLG_PROCESSING_COMPLETE );

		if( pDlg->m_bAbortCompile ) {
			break;
		}
	}

	if( pDlg->m_bUpdateMasterFileVersion ) {
		// update the master file version numbers
		switch( pDlg->m_nTargetPlatform ) {
		case PASM_TARGET_XB:
		case PASM_TARGET_PC:
			pDlg->m_MasterFile.SetApeVersion( FDATA_PRJFILE_XB_APE_VERSION );
			pDlg->m_MasterFile.SetTgaVersion( FDATA_PRJFILE_XB_TGA_VERSION );
			break;
		case PASM_TARGET_GC:
			pDlg->m_MasterFile.SetApeVersion( FDATA_PRJFILE_GC_APE_VERSION );
			pDlg->m_MasterFile.SetTgaVersion( FDATA_PRJFILE_GC_TGA_VERSION );
			break;
#ifdef _MMI_TARGET_PS2
        case PASM_TARGET_PS2:   // KJ
			pDlg->m_MasterFile.SetApeVersion( FDATA_PRJFILE_PS_APE_VERSION );	// KJ
			pDlg->m_MasterFile.SetTgaVersion( FDATA_PRJFILE_PS_TGA_VERSION );	// KJ
			break;	// KJ
#endif
		}
		pDlg->m_MasterFile.SetMtxVersion( FDATA_PRJFILE_MTX_VERSION );
		pDlg->m_MasterFile.SetCsvVersion( FDATA_PRJFILE_CSV_VERSION );
		pDlg->m_MasterFile.SetFntVersion( FDATA_PRJFILE_FNT_VERSION );
		pDlg->m_MasterFile.SetSmaVersion( FDATA_PRJFILE_SMA_VERSION );
		pDlg->m_MasterFile.SetGtVersion( FDATA_PRJFILE_GT_VERSION );
		pDlg->m_MasterFile.SetWvbVersion( FDATA_PRJFILE_WVB_VERSION );
		pDlg->m_MasterFile.SetFprVersion( FDATA_PRJFILE_FPR_VERSION );
		pDlg->m_MasterFile.SetCamVersion( FDATA_PRJFILE_CAM_VERSION );
	}

	// write out everything to the master file
	if( !pDlg->m_MasterFile.EndCompilation() ) {
		pDlg->MessageBox( _T("Error closing the master file after compilation."),
						  _T("PASM ERROR"),
						  MB_ICONHAND | MB_OK | MB_DEFBUTTON1 );
		pDlg->PostMessage( WM_COMMAND, IDCANCEL, 0 );

		if ( _hFileLog )
		{
			CloseHandle( _hFileLog );
			_hFileLog = NULL;
		}

		goto _REMOVETEMPDIRECTORY;
	}

	if( !pDlg->m_bAbortCompile ) {
		s.Format( " %d file(s) compiled OK, %d had errors", nNumFiles - nConversionErrors, nConversionErrors );
		pDlg->m_StatusBar.SetText( s, 0, 0 );

		if( nConversionErrors ) {
			if( nConversionErrors == 1 ) {
				s.Format( "Error compiling a file.\nOpen the error log file?" );
			} else {
				s.Format( "Error compiling %d files.\nOpen the error log file?", nConversionErrors );
			}
			if( pDlg->MessageBox( s, "PASM ERROR", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
				ShellExecute( NULL, "open", CErrorLog::GetErrorFilename(), NULL, NULL, SW_SHOWNORMAL );
			}
		}

		if( pDlg->m_bHoldCompilationStats ) {
			pDlg->m_sClickAnywhere = "*** Click anywhere inside this window to close. ***";
			pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
			// wait for the user to click the window
			u32 nOldLClickCount = pDlg->m_nNumLeftClicks;
			do {
				// wait a half sec
				Sleep( 500 );
			} while( pDlg->m_nNumLeftClicks <= nOldLClickCount );
		} else {
			// hold for 1/4 sec, it just looks better
			Sleep( 250 );
		}
	}

	if ( _hFileLog )
	{
		CloseHandle( _hFileLog );
		_hFileLog = NULL;
	}

	pDlg->PostMessage( WM_COMMAND, IDOK, 0 );

_REMOVETEMPDIRECTORY:

	// Remove the Interim data
	bFileResult = TRUE;
	csAllFiles = CPasmDlg::m_csPASMTempDirectory + "*.*";
	hFile = FindFirstFile( csAllFiles, &FileData );
	while ( bFileResult && hFile != INVALID_HANDLE_VALUE )
	{
		sFilename = CPasmDlg::m_csPASMTempDirectory + FileData.cFileName;
		bFileResult = DeleteFile( sFilename );
		bFileResult = FindNextFile( hFile, &FileData );
	}
	FindClose( hFile );

	return 0;
}

FileType_e MapFileInfoToFileType( const CFileInfo *pFileInfo, CCompileDlg *pDlg ) {

	CString sFileExt( pFileInfo->GetFileExt() );

	if( sFileExt.CompareNoCase( ".ape" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_APE;
	}
	if( sFileExt.CompareNoCase( ".wld" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_WLD;
	}
	if( sFileExt.CompareNoCase( ".tga" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_TGA;
	}
	if( sFileExt.CompareNoCase( ".mtx" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_MTX;
	}
	if( sFileExt.CompareNoCase( ".csv" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_CSV;
	}
	if( sFileExt.CompareNoCase( ".fnt" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_FNT;
	}
	if( sFileExt.CompareNoCase( ".sma" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_SMA;
	}
	if( sFileExt.CompareNoCase( ".gt" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_GT;
	}
	if( sFileExt.CompareNoCase( ".wvb" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_WVB;
	}
	if( sFileExt.CompareNoCase( ".rdg" ) == 0 ||
		sFileExt.CompareNoCase( ".rdx" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_RAW_DATA;
	}
	if( sFileExt.CompareNoCase( ".sfb" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_SFB;
	}
	if( sFileExt.CompareNoCase( ".fpr" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_FPR;
	}
	if( sFileExt.CompareNoCase( ".cam" ) == 0 ) {
		return COMPDIAG_FILE_TYPE_CAM;
	}

	return COMPDIAG_FILE_TYPE_GENERIC;
}

BOOL CallConverter( const CFileInfo *pFileInfo, CCompileDlg *pDlg, u32 &rnActuallyCompiled ) {
	FileType_e nFileType;
	BOOL bNeedsCompiling, bReturnValue = FALSE;
	CString sOutputFilename, sError;
	CKongToVisFile VisFile;
	CFileInfo *pAidFileInfo;
	CAidFile AidFile;

	if( !CMasterFileCompile::IsEntryNameValid( pFileInfo->GetFileName() ) ) {
		// the filename is too long
		pDlg->ErrorString( "Error converting %s, filename is too long. ", (cchar *)pFileInfo->GetFileName() );
		pDlg->m_nCompileResult = CDLG_RESULT_ASSET_NAME_TOO_LONG;
		return FALSE;
	}

	nFileType = MapFileInfoToFileType( pFileInfo, pDlg );

	// only compile certain file types on GC right now
	if( pDlg->m_nTargetPlatform == PASM_TARGET_GC ) {

		switch( nFileType ) {
		case COMPDIAG_FILE_TYPE_TGA:
		case COMPDIAG_FILE_TYPE_APE:
		case COMPDIAG_FILE_TYPE_WLD:
		case COMPDIAG_FILE_TYPE_GT:
		case COMPDIAG_FILE_TYPE_CSV:
		case COMPDIAG_FILE_TYPE_MTX:
		case COMPDIAG_FILE_TYPE_FNT:
		case COMPDIAG_FILE_TYPE_RAW_DATA:
		case COMPDIAG_FILE_TYPE_SFB:
		case COMPDIAG_FILE_TYPE_SMA:
		case COMPDIAG_FILE_TYPE_FPR:
		case COMPDIAG_FILE_TYPE_CAM:
			// supported file types
			break;
		default:
			// unsupported file types
			pDlg->ErrorString( "Error converting %s, only a limited number of filetypes are supported on GameCube. ", (cchar *)pFileInfo->GetFileName() );
			pDlg->m_nCompileResult = CDLG_RESULT_ASSET_TYPE_UNSUPPORTED;
			return FALSE;
			break;
		}
	}

	// grab a memory frame
	FResFrame_t MemFrame = fres_GetFrame();

	if( nFileType == COMPDIAG_FILE_TYPE_GENERIC ||
		nFileType == COMPDIAG_FILE_TYPE_RAW_DATA ) {
		sOutputFilename = pFileInfo->GetFileName();
		sOutputFilename.MakeLower();
		bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
		if( bNeedsCompiling ) {
			bReturnValue = ConvertGenericFiles( pFileInfo, pDlg, sOutputFilename );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_MTX ) {
		sOutputFilename.Format( "%s.mtx", pFileInfo->GetFileTitle() );
		bReturnValue = PreConvert( pFileInfo, pDlg, sOutputFilename, AidFile, bNeedsCompiling, &pAidFileInfo );
		if( bReturnValue ) {
			bReturnValue = ConvertMtxFile( pFileInfo, pDlg, sOutputFilename, AidFile, pAidFileInfo );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_CSV ) {
		sOutputFilename.Format( "%s.csv", pFileInfo->GetFileTitle() );
		sOutputFilename.MakeLower();
		bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
		if( bNeedsCompiling ) {
			bReturnValue = ConvertCsvFile( pFileInfo, pDlg, sOutputFilename );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_FNT ) {
		sOutputFilename.Format( "%s.fnt", pFileInfo->GetFileTitle() );
		sOutputFilename.MakeLower();
		bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
		if( bNeedsCompiling ) {
			bReturnValue = ConvertFntFile( pFileInfo, pDlg, sOutputFilename );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_SMA ) {
		sOutputFilename.Format( "%s.sma", pFileInfo->GetFileTitle() );
		sOutputFilename.MakeLower();
		bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
		if( bNeedsCompiling ) {
			bReturnValue = ConvertSmaFile( pFileInfo, pDlg, sOutputFilename );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_GT ) {
		sOutputFilename.Format( "%s.gt", pFileInfo->GetFileTitle() );
		sOutputFilename.MakeLower();
		bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
		if( bNeedsCompiling ) {
			bReturnValue = ConvertGtFile( pFileInfo, pDlg, sOutputFilename );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_SFB ) {
		sOutputFilename.Format( "%s.sfb", pFileInfo->GetFileTitle() );
		sOutputFilename.MakeLower();
		bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
		if( bNeedsCompiling ) {
			bReturnValue = ConvertSfbFile( pFileInfo, pDlg, sOutputFilename );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_FPR ) {
		sOutputFilename.Format( "%s.fpr", pFileInfo->GetFileTitle() );
		sOutputFilename.MakeLower();
		bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
		if( bNeedsCompiling ) {
			bReturnValue = ConvertFprFile( pFileInfo, pDlg, sOutputFilename );
		}
	} else if( nFileType == COMPDIAG_FILE_TYPE_CAM ) {
		sOutputFilename.Format( "%s.cam", pFileInfo->GetFileTitle() );
		sOutputFilename.MakeLower();
		bReturnValue = PreConvert( pFileInfo, pDlg, sOutputFilename, AidFile, bNeedsCompiling, &pAidFileInfo );
		if( bReturnValue ) {
			bReturnValue = ConvertCamFile( pFileInfo, pDlg, sOutputFilename, AidFile, pAidFileInfo );
		}
	} else {

		switch( pDlg->m_nTargetPlatform ) {

		case PASM_TARGET_XB:
		case PASM_TARGET_PC:
			switch( nFileType ) {

			case COMPDIAG_FILE_TYPE_WLD:
			{
				bReturnValue = WldPreConvert( pFileInfo, pDlg, sOutputFilename, VisFile, bNeedsCompiling, &pAidFileInfo, AidFile );
				if( bReturnValue ) {
					bReturnValue = ConvertWldFile( pDlg->m_nTargetPlatform, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
				}
				break;
			}
			case COMPDIAG_FILE_TYPE_APE:
				bReturnValue = ApePreConvert( pFileInfo, pDlg, sOutputFilename, VisFile, bNeedsCompiling, &pAidFileInfo, AidFile );
				if(	bReturnValue ) {
					bReturnValue = ConvertApeFile( pDlg->m_nTargetPlatform, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
				}
				break;

			case COMPDIAG_FILE_TYPE_WVB:
				sOutputFilename.Format( "%s.wvb", pFileInfo->GetFileTitle() );
				sOutputFilename.MakeLower();
				bNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, sOutputFilename );
				if( bNeedsCompiling ) {
					bReturnValue = ConvertWvbFile_XB( pFileInfo, pDlg, sOutputFilename );
				}
				break;

			case COMPDIAG_FILE_TYPE_TGA:
				sOutputFilename.Format( "%s.tga", pFileInfo->GetFileTitle() );
				bReturnValue = PreConvert( pFileInfo, pDlg, sOutputFilename, AidFile, bNeedsCompiling, &pAidFileInfo );
				if( bReturnValue ) {
					bReturnValue = ConvertTgaFile_XB( pFileInfo, pDlg, sOutputFilename, AidFile, pAidFileInfo );
				}
				break;

			default:
				FASSERT_NOW;
				break;
			}
			break;

		case PASM_TARGET_GC:
			switch( nFileType ) {

			case COMPDIAG_FILE_TYPE_TGA:
				sOutputFilename.Format( "%s.tga", pFileInfo->GetFileTitle() );
				bReturnValue = PreConvert( pFileInfo, pDlg, sOutputFilename, AidFile, bNeedsCompiling, &pAidFileInfo );
				if( bReturnValue ) {
					bReturnValue = ConvertTgaFile_GC( pFileInfo, pDlg, sOutputFilename, AidFile, pAidFileInfo );
				}
				break;

			case COMPDIAG_FILE_TYPE_WLD:
				bReturnValue = WldPreConvert( pFileInfo, pDlg, sOutputFilename, VisFile, bNeedsCompiling, &pAidFileInfo, AidFile );
				if( bReturnValue ) {
					bReturnValue = ConvertWldFile( pDlg->m_nTargetPlatform, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
				}
				break;
			case COMPDIAG_FILE_TYPE_APE:
				bReturnValue = ApePreConvert( pFileInfo, pDlg, sOutputFilename, VisFile, bNeedsCompiling, &pAidFileInfo, AidFile );
				if(	bReturnValue ) {
					bReturnValue = ConvertApeFile( pDlg->m_nTargetPlatform, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
				}
				break;

			default:
				FASSERT_NOW;
				break;
			}
			break;

#ifdef _MMI_TARGET_PS2
		// Begin KJ
        case PASM_TARGET_PS2:
			switch( nFileType ) {

            case COMPDIAG_FILE_TYPE_TGA:
				sOutputFilename.Format( "%s.tga", pFileInfo->GetFileTitle() );
				bReturnValue = PreConvert( pFileInfo, pDlg, sOutputFilename, AidFile, bNeedsCompiling, &pAidFileInfo );
				if( bReturnValue ) {
                    bReturnValue = ConvertTgaFile_PS2( pFileInfo, pDlg, sOutputFilename, AidFile, pAidFileInfo );
				}
				break;

			case COMPDIAG_FILE_TYPE_WLD:
				bReturnValue = WldPreConvert( pFileInfo, pDlg, sOutputFilename, VisFile, bNeedsCompiling, &pAidFileInfo, AidFile );
				nCollisionMeshBytes = 0;	// NS clear collsision mesh total before processing world mesh
				if( bReturnValue ) {

#define LOG_ENVMAP 1
#if LOG_ENVMAP
					{
						FILE *tf = fopen("\\pasm2-environmentmap.txt", "a+");
				        fprintf(tf, "-------------------------------------------------\n");
						fprintf(tf, "Processing %s\n", sOutputFilename);
						fclose(tf);
				        tf = NULL;
					}
#endif


//					bReturnValue = ConvertWldFile( CONVERTAPE_PLATFORM_PS2, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
					bReturnValue = ConvertWldFile( pDlg->m_nTargetPlatform, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
				}

#if LOG_ENVMAP
				{
					FILE *tf = fopen("\\pasm2-environmentmap.txt", "a+");
			        fprintf(tf, "Finished %s\n", sOutputFilename);
			        fprintf(tf, "-------------------------------------------------\n");
					fclose(tf);
			        tf = NULL;
				}
#endif
				break;
			case COMPDIAG_FILE_TYPE_APE:
				bReturnValue = ApePreConvert( pFileInfo, pDlg, sOutputFilename, VisFile, bNeedsCompiling, &pAidFileInfo, AidFile );
				if(	bReturnValue ) {
//					bReturnValue = ConvertApeFile( CONVERTAPE_PLATFORM_PS2, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
					bReturnValue = ConvertApeFile( pDlg->m_nTargetPlatform, pFileInfo, pDlg, sOutputFilename, VisFile, AidFile, pAidFileInfo );
				}
				break;

			default:
				FASSERT_NOW;
				break;
			}
#endif
			break;
			// End KJ
		}
	}
	// free any fang memory used
	fres_ReleaseFrame( MemFrame );

	if( !bNeedsCompiling && nFileType != COMPDIAG_FILE_TYPE_APE ) {
		sOutputFilename = pFileInfo->GetFileName();
		sOutputFilename.MakeLower();
		pDlg->InfoString( "%s does not need compiling, it is up to date already", (cchar *)sOutputFilename );
		pDlg->m_nCompileResult = CDLG_RESULT_UP_TO_DATE_MASTERFILE;
		return TRUE;
	}
	if( bReturnValue ) {
		rnActuallyCompiled++;
	}

	return bReturnValue;
}

BOOL ConvertMtxFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
								  CAidFile &rAidFile, CFileInfo *pAidFileInfo ) {
	CString sError, sFilename;
	BOOL bAnimCompress = TRUE, bTimeCompress = TRUE;
	f32 fThreshold = -1.f;
	CConvertMtxFile MtxData;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// check the aid file for user supplied settings
	if( rAidFile.m_bDataValid ) {
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].bValueWasFound ) {
			bAnimCompress = rAidFile.m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].bValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_TIME_COMPRESS].bValueWasFound ) {
			bTimeCompress = rAidFile.m_aNonStrings[AID_FILE_FLOATS_TIME_COMPRESS].bValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_THRESHOLD].bValueWasFound ) {
			fThreshold = rAidFile.m_aNonStrings[AID_FILE_FLOATS_THRESHOLD].fValue;
		}
	}

	// set our compression vars
	MtxData.SetCompressionVars( bTimeCompress, bAnimCompress, fThreshold );

	// convert the .mtx file
	if( !MtxData.ConvertFile( pFileInfo, (pDlg->m_nTargetPlatform == PASM_TARGET_GC) ? TRUE : FALSE ) ) {
		pDlg->ErrorString( "Error converting %s to animation data file", (cchar *)sFilename );
		return FALSE;
	}
	// grab a file pointer into the master file
	u32 nFileCRC = MtxData.GetDataCRC();
	u32 nConvertedSize = MtxData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nFileCRC );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !MtxData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}

	// update the support entries
	if( pAidFileInfo ) {
		pDlg->m_MasterFile.UpdateOrAddSupportEntry( pAidFileInfo );
	}
	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );
	pDlg->InfoString( "Total bone count: %d", MtxData.m_nBoneCount );
	pDlg->InfoString( "Total frame count: %d", MtxData.m_nTotalFrameCount );

	return TRUE;
}

BOOL ConvertGenericFiles( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// grab a file pointer into the master file
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, 0 );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// open the source file for copying
	FILE *pSrcFile = fopen( pFileInfo->GetFilePath(), "rb" );
	if( !pSrcFile ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error opening %s for reading", (cchar *)sFilename );
		return FALSE;
	}
	// copy the source file
	if( !pDlg->m_MasterFile.CopyFile( pDstFile, pSrcFile, 0 ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error copying %s into master file", (cchar *)sFilename );
		fclose( pSrcFile );
		return FALSE;
	}
	// close the source file
	fclose( pSrcFile );

	// update our status fields
	pDlg->InfoString( "%s copied OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", pFileInfo->GetLength() );

	return TRUE;
}

BOOL ConvertCsvFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// convert the .csv file
	CConvertCsvFile CsvData;
	if( !CsvData.ConvertFile( pFileInfo, (pDlg->m_nTargetPlatform == PASM_TARGET_GC) ? TRUE : FALSE ) ) {
		pDlg->ErrorString( "Error converting %s into game usable data", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nFileCRC = CsvData.GetDataCRC();
	u32 nConvertedSize = CsvData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nFileCRC );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !CsvData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );
	pDlg->InfoString( "Number of valid tables: %d", CsvData.m_nNumTables );
	pDlg->InfoString( "Number of valid fields: %d (%d floats, %d strings)", CsvData.m_nNumFields, CsvData.m_nNumFloatFields, CsvData.m_nNumStringFields );

	return TRUE;
}

BOOL ConvertFntFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// convert the .fnt file
	CConvertFntFile FntData;
	if( !FntData.ConvertFile( pFileInfo, (pDlg->m_nTargetPlatform == PASM_TARGET_GC) ? TRUE : FALSE ) ) {
		pDlg->ErrorString( "Error converting %s into usable font data", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nConvertedSize = FntData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !FntData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );

	return TRUE;
}

BOOL ConvertSmaFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// convert the .sma file
	CConvertSmaFile SmaData;
	if( !SmaData.ConvertFile( pFileInfo ) ) {
		pDlg->ErrorString( "Error converting %s into usable SMALL script data", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nFileCRC = SmaData.GetDataCRC();
	u32 nConvertedSize = SmaData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nFileCRC );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !SmaData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );

	return TRUE;
}

BOOL ConvertGtFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// convert the .gt file
	CConvertGtFile GtData;
	if( !GtData.ConvertFile( pFileInfo, (pDlg->m_nTargetPlatform == PASM_TARGET_GC) ? TRUE : FALSE ) ) {
		pDlg->ErrorString( "Error converting %s into usable graph description file", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nConvertedSize = GtData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !GtData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );

	return TRUE;
}

BOOL ConvertSfbFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// convert the .sfb file
	CConvertSfbFile SfbData;
	if( !SfbData.ConvertFile( pFileInfo, (pDlg->m_nTargetPlatform == PASM_TARGET_GC) ? TRUE : FALSE ) ) {
		pDlg->ErrorString( "Error converting %s into usable sound fx bank", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nFileCRC = SfbData.GetDataCRC();
	u32 nConvertedSize = SfbData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nFileCRC );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !SfbData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );
	pDlg->InfoString( "The referenced wav bank file is: '%s'", SfbData.m_sBasedOnWavBank );
	pDlg->InfoString( "The bank contains %d sound fxs and %d sequencer commands", SfbData.m_nNumSfxs, SfbData.m_nNumCmds );

	return TRUE;
}

BOOL ConvertFprFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// convert the .fpr file
	CConvertFprFile FprData;
	if( !FprData.ConvertFile( pFileInfo, (pDlg->m_nTargetPlatform == PASM_TARGET_GC) ? TRUE : FALSE ) ) {
		pDlg->ErrorString( "Error converting %s into usable particle def file", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nFileCRC = FprData.GetDataCRC();
	u32 nConvertedSize = FprData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nFileCRC );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !FprData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );

	return TRUE;
}

BOOL ConvertCamFile( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
    				 CAidFile &rAidFile, CFileInfo *pAidFileInfo ) {

	CString sError, sFilename;
	CConvertCamFile CamData;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	BOOL bAnimCompress = TRUE;

	// check the aid file for user supplied settings
	if( rAidFile.m_bDataValid ) {
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].bValueWasFound ) {
			bAnimCompress = rAidFile.m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].bValue;
		}
	}

	// set our compression vars
	CamData.SetCompressionVars( bAnimCompress );

	// convert the .cam file
	if( !CamData.ConvertFile( pFileInfo, (pDlg->m_nTargetPlatform == PASM_TARGET_GC) ? TRUE : FALSE ) ) {
		pDlg->ErrorString( "Error converting %s into game usable data", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nConvertedSize = CamData.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}
	// write out our data to the master file
	if( !CamData.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}

	// update the support entries
	if( pAidFileInfo ) {
		pDlg->m_MasterFile.UpdateOrAddSupportEntry( pAidFileInfo );
	}

	// update our status fields
	pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in the master file", nConvertedSize );

	return TRUE;
}


BOOL ConvertApeFile( TargetPlatform_e nPlatform, const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
									 CKongToVisFile &rVisFile, CAidFile &rAidFile, const CFileInfo *pAidFileInfo ) {
	CString sError, sFilename, sRawAssetName;
	CKongToMLMesh ConvertedMesh;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	FASSERT( rVisFile.m_bA2KConverted );

	sRawAssetName = rsOutputFilename;
	StripExtension( sRawAssetName );

	// Determine static lighting settings
	BOOL bGenerateLightmaps = pDlg->m_bGenerateLMSTs;
	BOOL bVertRadiosity = pDlg->m_bGenerateVertRad;

	// If we still don't have the lightmap flag set, check the AID file
	if ( !bGenerateLightmaps )
	{
		if ( rAidFile.m_bDataValid && rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_LMS].bValueWasFound )
		{
			bGenerateLightmaps = rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_LMS].bValue;
		}
	}

	// Make sure static lighting supports existing APE settings in the masterfile
	FDataPrjFile_Entry_t *pMFEntry = pDlg->m_MasterFile.GetEntryByName( rsOutputFilename );
	if ( pMFEntry )
	{
		if ( pMFEntry->nFlags & APE_ENTRY_FLAGS_LIGHTMAPUV )
		{
			if ( pDlg->m_bDetailedLightingInfo )
			{
				pDlg->InfoString( "    Preserving existing lightmap UV setting for %s.", sRawAssetName );
			}
			bGenerateLightmaps = TRUE;
		}
		else if ( bGenerateLightmaps && pDlg->m_bDetailedLightingInfo )
		{
			pDlg->InfoString( "    Adding lightmap UV information to %s.", sRawAssetName );
		}
		if ( pMFEntry->nFlags & APE_ENTRY_FLAGS_VERTRAD )
		{
			if ( pDlg->m_bDetailedLightingInfo )
			{
				pDlg->InfoString( "    Preserving existing vertex radiosity setting for %s.", sRawAssetName );
			}
			bVertRadiosity = TRUE;
		}
		else if ( bVertRadiosity && pDlg->m_bDetailedLightingInfo )
		{
			pDlg->InfoString( "    Adding vertex radiosity information to %s.", sRawAssetName );
		}
	}
	else if ( pDlg->m_bDetailedLightingInfo )
	{
		if ( bGenerateLightmaps )
		{
			pDlg->InfoString( "    Adding lightmap UV information to %s.", sRawAssetName );
		}
		if ( bVertRadiosity )
		{
			pDlg->InfoString( "    Adding vertex radiosity information to %s.", sRawAssetName );
		}
	}
	
	// Generate lightmap UV's if they are needed
	if ( bGenerateLightmaps )
	{
		ConvertedMesh.GenerateLMSTs( pDlg, rVisFile.m_pKongMesh, sRawAssetName );
	}

	if ( pDlg->m_bBuildingLightMaps )
	{
		pDlg->WriteToLog( "Saving Kong for later use in lightmapping.\n" );
		rVisFile.m_pKongMesh->CreateFileAndSaveKong( sRawAssetName );
	}

	ResetStripperTimers();

	///////////////////////
	// Convert to MLMesh
	pDlg->WriteToLog( "Converting APE to FANG format.\n" );
	if( !ConvertedMesh.Convert( pDlg, rVisFile, rsOutputFilename, &rAidFile, nPlatform, pDlg->m_bGenerateMeshStrips, FALSE, bVertRadiosity ) ) {
		pDlg->ErrorString( "Error creating ML mesh from %s", (cchar *)sFilename );
		pDlg->m_nCompileResult = CDLG_RESULT_COMPILE_FAILED;
		return FALSE;
	}

	pDlg->WriteToLog( "APE conversion complete.\n" );

	ResetStripperTimers();

	// Set the masterfile entry flags
	u16 nEntryFlags = 0;
	if ( bGenerateLightmaps )
	{
		nEntryFlags |= APE_ENTRY_FLAGS_LIGHTMAPUV;
	}
	if ( bVertRadiosity )
	{
		nEntryFlags |= APE_ENTRY_FLAGS_VERTRAD;
	}

	// grab a file pointer into the master file
	u32 nCRCValue = 0;
	nCRCValue = ConvertedMesh.GetCRC( nCRCValue );
	u32 nConvertedSize = ConvertedMesh.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nCRCValue, nEntryFlags );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		pDlg->m_nCompileResult = CDLG_RESULT_COMPILE_FAILED;
		return FALSE;
	}

	// write out our data to the master file
	if( !ConvertedMesh.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		pDlg->m_nCompileResult = CDLG_RESULT_COMPILE_FAILED;
		return FALSE;
	}

	pDlg->m_nCompileResult = CDLG_RESULT_COMPILE_SUCCESSFUL;

	if ( !pDlg->m_bBuildingLightMaps )
	{
		// update our status fields
		pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
		pDlg->InfoString( "Used %d bytes in master file", nConvertedSize );
		pDlg->InfoString( "Segs: %d, Bones: %d, Mats: %d", ConvertedMesh.m_LibResults.nSegmentCount, ConvertedMesh.m_LibResults.nBoneCount, ConvertedMesh.m_LibResults.nMaterialCount );
		if ( ConvertedMesh.m_nStripTriCount )
		{
			pDlg->InfoString( "%d%% of total tris (%d of %d) stripped.", (100 * ConvertedMesh.m_nStripTriCount) / (ConvertedMesh.m_nListTriCount + ConvertedMesh.m_nStripTriCount), ConvertedMesh.m_nStripTriCount, (ConvertedMesh.m_nListTriCount + ConvertedMesh.m_nStripTriCount) );
			pDlg->InfoString( "Average strip size of %3.1f tris.", (f32)ConvertedMesh.m_nStripTriCount / (f32)ConvertedMesh.m_nStripCount );
		}
	}

	// update the support entries
	if( pAidFileInfo ) {
		pDlg->m_MasterFile.UpdateOrAddSupportEntry( pAidFileInfo );
	}

	return TRUE;
}

BOOL ConvertWldFile( TargetPlatform_e nPlatform, const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
									 CKongToVisFile &rVisFile, CAidFile &rAidFile, const CFileInfo *pAidFileInfo ) {
	CString csAllFiles, sError, sFilename, sRawAssetName;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	FASSERT( rVisFile.m_bA2KConverted );

	sRawAssetName = rsOutputFilename;
	StripExtension( sRawAssetName );

	////////////////////////
	// Convert to world
	CApeToWorldFile ConvertedWorld;

	ResetStripperTimers();

	if ( rVisFile.m_nVisFileStatus == VIS_FILE_UP_TO_DATE_LOCALLY || rVisFile.m_nVisFileStatus == VIS_FILE_UP_TO_DATE_LIBRARY )
	{
		if ( !ConvertedWorld.ConvertToWorldFileFromVis( pDlg, rVisFile, rsOutputFilename, nPlatform ) )
		{
			pDlg->ErrorString( "Error creating world mesh from .vis file for %s", (cchar *)sFilename );
			goto _CONVERSIONFAILED;
		}
	}
	else if( !ConvertedWorld.ConvertToWorldFile( pDlg, rVisFile, rAidFile, rsOutputFilename, nPlatform ) ) {
		pDlg->ErrorString( "Error creating world mesh from %s", (cchar *)sFilename );
		goto _CONVERSIONFAILED;
	}

	ResetStripperTimers();

	u16 nEntryFlags = rVisFile.m_nStaticLightingQualityLevel << 12;
	if ( rVisFile.m_bUsedOldVisData )
	{
		nEntryFlags |= 0x0001; // This flag should be added to fdata.h, but I don't want to force the recompile on MS weekend.
	}

	// grab a file pointer into the master file
	u32 nCRCValue = ConvertedWorld.GetWorldCRC( rVisFile );
	u32 nConvertedSize = ConvertedWorld.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nCRCValue, nEntryFlags );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		goto _CONVERSIONFAILED;
	}

	pDlg->ResetSubOp( "Saving data...", 1 );
	pDlg->PostMessage( CDLG_PROCESSING_COMPLETE );

	// write out our data to the master file
	if( !ConvertedWorld.WriteWorldFile( pDlg, (cchar *)rsOutputFilename, rVisFile, nPlatform, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		goto _CONVERSIONFAILED;
	}

	// update our status fields
	pDlg->InfoString( "\n%s compiled OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in master file", nConvertedSize );

	if ( rVisFile.m_nVisFileStatus != VIS_FILE_UP_TO_DATE_LOCALLY && rVisFile.m_nVisFileStatus != VIS_FILE_UP_TO_DATE_LIBRARY )
	{
		f32 fTotalTris = (f32)rVisFile.m_pKongMesh->nTriangleCount;
		pDlg->InfoString( "There were %d tris, %d needed clipped (%.2f%%), %d didn't, yielding %d total tris", (u32)fTotalTris,
																													rVisFile.m_nStatsCrossingTris,
																													(f32)rVisFile.m_nStatsCrossingTris/fTotalTris * 100.0f,
																													rVisFile.m_nStatsNonCrossingTris,
																													rVisFile.m_nStatsFinalTriCount );
		pDlg->InfoString( "Volumes = %d, Cells = %d, Planes = %d, Portals = %d (%d Auto generated)", rVisFile.m_nStatsVolumes,
																											rVisFile.m_nStatsCells,
																											rVisFile.m_nStatsCellPlanes,
																											rVisFile.m_nStatsPortals,
																											rVisFile.m_nStatsAutoPortals );
		pDlg->InfoString( "Avg Tris/Vol: %.2f, Max Tris in a Vol: %d", rVisFile.m_fStatsAvgTrisPerVol, rVisFile.m_nStatsMaxVolTris );
		pDlg->InfoString( "Shapes: %d, Shape Init data: %d bytes", rVisFile.m_InitFile.m_nStatsNumShapes,
																		rVisFile.m_InitFile.m_nStatsShapeBytes );
	}
	if ( ConvertedWorld.m_nStripTriCount )
	{
		pDlg->InfoString( "%d%% of total tris (%d of %d) stripped.", (100 * ConvertedWorld.m_nStripTriCount) / (ConvertedWorld.m_nListTriCount + ConvertedWorld.m_nStripTriCount), ConvertedWorld.m_nStripTriCount, (ConvertedWorld.m_nListTriCount + ConvertedWorld.m_nStripTriCount) );
		pDlg->InfoString( "Average strip size of %3.1f tris.", (f32)ConvertedWorld.m_nStripTriCount / (f32)ConvertedWorld.m_nStripCount );
	}

	// update the support entries
	if( pAidFileInfo )
	{
		pDlg->m_MasterFile.UpdateOrAddSupportEntry( pAidFileInfo );
	}

	return TRUE;

_CONVERSIONFAILED:

	return FALSE;
}

BOOL ConvertTgaFile_XB( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
									 CAidFile &rAidFile, CFileInfo *pAidFileInfo ) {
	CTgaFileLoader TgaFile;
	CGenMipMaps MipMaps;
	CString sError, sFilename, sRawAssetName, sDesiredFormat, sTempBumpFile;
	BOOL bGenOnlyTheTopLOD, bForceCutout, bMipToZero, bDither, bDetailMap, bBumpMap, bBumpWraps;
	u32 nLargestDim, nSmallestDim;
	f32 fBumpScale;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	sRawAssetName = rsOutputFilename;
	StripExtension( sRawAssetName );

	// set the settings to their default values
	bGenOnlyTheTopLOD = FALSE;
	bForceCutout = FALSE;
	bMipToZero = TRUE;
	bDither = FALSE;
	sDesiredFormat = "auto";
	bDetailMap = FALSE;
	bBumpMap = FALSE;
	bBumpWraps = TRUE;
	fBumpScale = 0.0f;

	// check the aid file for user supplied settings
	if( rAidFile.m_bDataValid ) {
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].bValueWasFound ) {
			if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "detail" ) == 0 ) {
				bDetailMap = TRUE;
			} else if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "bump" ) == 0 ) {
				bBumpMap = TRUE;
			}
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValueWasFound ) {
			bGenOnlyTheTopLOD = !rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValue;
		}
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].bValueWasFound ) {
			// only 'cutout' means anything on the xbox side
			if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "cutout" ) == 0 ) {
				bForceCutout = TRUE;
			}
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValueWasFound ) {
			bMipToZero = !rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_DITHER].bValueWasFound ) {
			bDither = rAidFile.m_aNonStrings[AID_FILE_FLOATS_DITHER].bValue;
		}
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_XB_TEX_FORMAT].bValueWasFound ) {
			sDesiredFormat = rAidFile.m_aStrings[AID_FILE_STRINGS_XB_TEX_FORMAT].sValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].bValueWasFound ) {
			fBumpScale = rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].fValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValueWasFound ) {
			bBumpWraps = rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValue;
		}
	}

	if( !bBumpMap ) {
		// load file
		if( !TgaFile.LoadTgaFile( (cchar *)pFileInfo->GetFilePath(), FALSE, FALSE/*bDetailMap*/ ) ) {
			pDlg->ErrorString( "Error loading %s for conversion", (cchar *)sFilename );
			return FALSE;
		}
	} else {
		// create a new normal map file from our source image
		sTempBumpFile.Format( "%s%s - temp bump map file.tga", pFileInfo->GetFileRoot(), pFileInfo->GetFileTitle() );
		if( ConvertTgaToNormMap( (cchar *)pFileInfo->GetFilePath(), sTempBumpFile, KERNEL_3x3, bBumpWraps, fBumpScale ) == false ) {
			pDlg->ErrorString( "Error creating the bump map format of file %s", pFileInfo->GetFilePath() );
			return FALSE;
		}
		// load file
		if( !TgaFile.LoadTgaFile( (cchar *)sTempBumpFile, FALSE, FALSE ) ) {
			pDlg->ErrorString( "Error loading the temp bump map file %s for conversion", (cchar *)sTempBumpFile );
			// delete the temp file
			DeleteFile( sTempBumpFile );
			return FALSE;
		}
		// delete the temp file
		DeleteFile( sTempBumpFile );
	}

	nLargestDim = TgaFile.GetLargestDim();
	nSmallestDim = TgaFile.GetSmallestDim();
	if( nSmallestDim >= 32 ) {
		// only think about reducing texture size if the smallest dimension is >= 64
		if( pDlg->m_bDownsizeTextures && nLargestDim > 64 ) {
			nLargestDim >>= 1;// cut in half
		}
	}

	// generate mipmaps
	CCreateXBoxTgaFile::Xb_Format_e nFormat;
	if( !bDetailMap ) {
		BOOL bMipsWillBeACutout = MipMaps.WillTGABeACutout( TgaFile, bForceCutout );
		nFormat = CCreateXBoxTgaFile::GetFormatFromString( bMipsWillBeACutout, sDesiredFormat );
	} else {
		bForceCutout = FALSE;
		//nFormat = CCreateXBoxTgaFile::XB_FORMAT_DXT3;// force dxt3 compression
		nFormat = CCreateXBoxTgaFile::GetFormatFromString( FALSE, sDesiredFormat );
	}

	if( nFormat == CCreateXBoxTgaFile::XB_FORMAT_DXT1 ||
		nFormat == CCreateXBoxTgaFile::XB_FORMAT_DXT3 ||
		nFormat == CCreateXBoxTgaFile::XB_FORMAT_DXT5 ) {
		// don't go down smaller than 4 in any dimension, and maintain the aspect ratio
		if( !MipMaps.GenerateMipMaps( TgaFile, nLargestDim, 4, TRUE, bGenOnlyTheTopLOD, bForceCutout, bMipToZero ) ) {
			pDlg->ErrorString( "Error creating mipmaps from %s", (cchar *)sFilename );
			return FALSE;
		}
	} else {
		// go down to 1x1 not maintaining the aspect ratio
		if( !MipMaps.GenerateMipMaps( TgaFile, nLargestDim, 1, FALSE, bGenOnlyTheTopLOD, bForceCutout, bMipToZero ) ) {
			pDlg->ErrorString( "Error creating mipmaps from %s", (cchar *)sFilename );
			return FALSE;
		}
	}

	if( bDetailMap ) {
		MipMaps.BiasLowerLodsToValue( 255 );
		MipMaps.CutAllPixelValuesByHalf();
	}

	// convert mipmaps
	CCreateXBoxTgaFile XBoxTga;
	if( !XBoxTga.ConvertTgaFile( MipMaps, (cchar *)sRawAssetName, bDither, nFormat ) ) {
		pDlg->ErrorString( "Error creating Xbox texture from %s", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nCRCValue = XBoxTga.GetDataCRC();
	u32 nConvertedSize = XBoxTga.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nCRCValue );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}

	// write out our data to the master file
	if( !XBoxTga.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile, pDlg->m_bOutputDebugPics ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	if ( !pDlg->m_bBuildingLightMaps )
	{
		pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
		pDlg->InfoString( "Used %d bytes in master file", nConvertedSize );
	}

	// update the support entries
	if( pAidFileInfo ) {
		pDlg->m_MasterFile.UpdateOrAddSupportEntry( pAidFileInfo );
	}

	return TRUE;
}

BOOL ConvertWvbFile_XB( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename ) {
	CString sError, sFilename;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	// convert the bank file on disk to an xbox usable bank file
	CConvertWvbFile_XBox ConvertWvbFile;
	if( !ConvertWvbFile.ConvertFile( pFileInfo ) ) {
		pDlg->ErrorString( "Error creating Xbox sound bank file from %s", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nFileCRC = ConvertWvbFile.GetDataCRC();
	u32 nConvertedSize = ConvertWvbFile.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nFileCRC );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}

	// write out our data to the master file
	if( !ConvertWvbFile.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}

	// update our status fields
	pDlg->InfoString( "%s copied OK", (cchar *)sFilename );
	pDlg->InfoString( "Used %d bytes in master file", nConvertedSize );
	pDlg->InfoString( "%d wav files in the bank.", ConvertWvbFile.m_nNumWavs );
	pDlg->InfoString( "The largest is %d bytes, smallest %d bytes.", ConvertWvbFile.m_nLargestWav, ConvertWvbFile.m_nSmallestWav );

	return TRUE;
}

BOOL ConvertTgaFile_GC( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
									 CAidFile &rAidFile, CFileInfo *pAidFileInfo ) {
	CTgaFileLoader TgaFile;
	CGenMipMaps MipMaps;
	CString sError, sFilename, sRawAssetName, sDesiredFormat;
	BOOL bGenOnlyTheTopLOD, bForceCutout, bMipToZero, bDither, bDetailMap, bBumpMap, bBumpWraps;
	u32 nLargestDim, nSmallestDim;
	f32 fBumpScale;
	s32 nDownSize = -1;
    
	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	sRawAssetName = rsOutputFilename;
	StripExtension( sRawAssetName );

	// set the settings to their default values
	bGenOnlyTheTopLOD = FALSE;
	bForceCutout = FALSE;
	bMipToZero = TRUE;
	bDither = FALSE;
	sDesiredFormat = "auto";
	bDetailMap = FALSE;
	bBumpMap = FALSE;
	bBumpWraps = TRUE;
	fBumpScale = 0.0f;

	// check the aid file for user supplied settings
	if( rAidFile.m_bDataValid ) {
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].bValueWasFound ) {
			if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "detail" ) == 0 ) {
				bDetailMap = TRUE;
			} else if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "bump" ) == 0 ) {
				bBumpMap = TRUE;
			}
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValueWasFound ) {
			bGenOnlyTheTopLOD = !rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValue;
		}
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].bValueWasFound ) {
			// only 'cutout' means anything on the xbox side
			if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "cutout" ) == 0 ) {
				bForceCutout = TRUE;
			}
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValueWasFound ) {
			bMipToZero = !rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_DITHER].bValueWasFound ) {
			bDither = rAidFile.m_aNonStrings[AID_FILE_FLOATS_DITHER].bValue;
		}
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_GC_TEX_FORMAT].bValueWasFound ) {
			sDesiredFormat = rAidFile.m_aStrings[AID_FILE_STRINGS_GC_TEX_FORMAT].sValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].bValueWasFound ) {
			nDownSize = rAidFile.m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].nValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].bValueWasFound ) {
			fBumpScale = rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].fValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValueWasFound ) {
			bBumpWraps = rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValue;
		}
	}

	// load file
	if( !TgaFile.LoadTgaFile( (cchar *)pFileInfo->GetFilePath(), FALSE, FALSE/*bDetailMap*/ ) ) {
		pDlg->ErrorString( "Error loading %s for conversion", (cchar *)sFilename );
		return FALSE;
	}

	if( bBumpMap ) {
		// make sure that we have a 32bit image to bump
		if( !TgaFile.Convert32bitAlphaImageIntoUnitFloat() ) {
			pDlg->ErrorString( "Error creating bump map from %s, the tga must be 32 bit", (cchar *)sFilename );
			return FALSE;
		}
		u32 nW = TgaFile.GetWidth();
		u32 nH = TgaFile.GetHeight();
		f32 *pfSrcPixels = (f32 *)TgaFile.GetPixelData();
		f32 fTex00, fTex01, fTex10, fDs, fDt, fValue;

		// allocate some working memory to store the computed I and A values
		u8 *pDestMem, *pAlloctedMem;
		pAlloctedMem = new u8[nW * nH * 2];
		if( !pAlloctedMem ) {
			pDlg->ErrorString( "Error creating bump map from %s, could not allocate working memory", (cchar *)sFilename );
			return FALSE;
		}
		pDestMem = pAlloctedMem;
		u32 i, j, nIndex;
		for( i=0; i < nH; i++ ) {
			for( j=0; j < nW; j++ ) {
				nIndex = (i * nW) + j;
				// look up the 3 pixels used in the calculations
				fTex00 = pfSrcPixels[nIndex];

				if( (j+1) < nW ) {
					// grab the pixel to the right
					fTex01 = pfSrcPixels[nIndex+1];
				} else {
					if( bBumpWraps ) {
						// wrap to the pixel on the left side of the texture
						fTex01 = pfSrcPixels[nIndex+1-nW];
					} else {
						// clamp to the last pixel
						fTex01 = fTex00;
					}
				}

				if( (i+1) < nH ) {
					// grab the pixel below
					fTex10 = pfSrcPixels[nIndex+nW];
				} else {
					if( bBumpWraps ) {
						// grab the pixel on the top of the texture
						fTex10 = pfSrcPixels[j];
					} else {
						fTex10 = fTex00;
					}
				}

				// compute the delta values
				fDs = fTex00 - fTex01;
				fDt = fTex00 - fTex10;

				// calculate the intensity
				fValue = (fDs * 128.0f) + 127.0f;
				FMATH_CLAMP( fValue, 0.0f, 255.0f );
				pDestMem[0] = (u8)fValue;

				// calculate the alpha
				fValue = (fDt * 128.0f) + 127.0f;
				FMATH_CLAMP( fValue, 0.0f, 255.0f );
				pDestMem[1] = (u8)fValue;

				pDestMem += 2;
			}
		}

		// copy our I and A values back into the tga, over the float values
		u8 *pI = pAlloctedMem;
		u8 *pA = &pI[1];
		pDestMem = (u8 *)pfSrcPixels;
		for( i=0; i < nH; i++ ) {
			for( j=0; j < nW; j++ ) {
				pDestMem[0] = pDestMem[1] = pDestMem[2] = *pI;
				pDestMem[3] = *pA;

				pI += 2;
				pA += 2;
				pDestMem += 4;
			}
		}

		// free our temp memory
		delete [] pAlloctedMem;
	}

	u32 nOrigLargestDim = TgaFile.GetLargestDim();
	nLargestDim = TgaFile.GetLargestDim();
	nSmallestDim = TgaFile.GetSmallestDim();
	if( nSmallestDim >= 32 ) {
		// only think about reducing texture size if the smallest dimension is >= 64
		if( nLargestDim > 1024 ) {
			// we can't have any texture bigger than 1024 on GC
			nLargestDim = 1024;
		} else if( nDownSize > 0 ) {
			if ( (nSmallestDim >> nDownSize) >= 8 ) {
				nLargestDim >>= nDownSize;
			} else {
				nLargestDim = nLargestDim / (nSmallestDim / 8);
			}
		} else if( nDownSize == -1 ) {
			if ( nLargestDim > 64 ) {
				nLargestDim >>= 1;// Automatically cut in half
			} else if ( pDlg->m_bDownsizeTextures && nLargestDim > 32 ) {
				nLargestDim >>= 1;// Cut in half only if the dialog indicated we should
			}
		}
	}

	// generate mipmaps
	CCreateGCTgaFile::GC_Format_e nFormat;
	if( !bDetailMap && !bBumpMap ) {
		BOOL bMipsWillBeACutout = MipMaps.WillTGABeACutout( TgaFile, bForceCutout );
		nFormat = CCreateGCTgaFile::GetFormatFromString( bMipsWillBeACutout, sDesiredFormat, TgaFile.GetBitsPerPixel() );
	} else if( bDetailMap ) {
		bForceCutout = FALSE;
		nFormat = CCreateGCTgaFile::GetFormatFromString( FALSE, sDesiredFormat, TgaFile.GetBitsPerPixel() );//CCreateGCTgaFile::GC_FORMAT_S3TC;// force dxt1 compression for detail maps
	} else if( bBumpMap ) {
		bForceCutout = FALSE;
		nFormat	= CCreateGCTgaFile::GC_FORMAT_IA8;// Jeremy: force IA8 compression for bump maps
		if ( CCreateGCTgaFile::GetFormatFromString( FALSE, sDesiredFormat, TgaFile.GetBitsPerPixel() ) == CCreateGCTgaFile::GC_FORMAT_IA4)
		{
			nFormat	= CCreateGCTgaFile::GC_FORMAT_IA4;
		}
	}

	if( nFormat == CCreateGCTgaFile::GC_FORMAT_S3TC ||
		nFormat == CCreateGCTgaFile::GC_FORMAT_S3TCA1 ||
		nFormat == CCreateGCTgaFile::GC_FORMAT_S3TCx2 ) {
		// don't go down smaller than 8 in any dimension, and maintain the aspect ratio
		if( !MipMaps.GenerateMipMaps( TgaFile, nLargestDim, 8, TRUE, bGenOnlyTheTopLOD, bForceCutout, bMipToZero ) ) {
			pDlg->ErrorString( "Error creating mipmaps from %s", (cchar *)sFilename );
			return FALSE;
		}
	} else {
		// don't go down smaller than 4 in any dimension, and maintain the aspect ratio
		if( !MipMaps.GenerateMipMaps( TgaFile, nLargestDim, 4, TRUE, bGenOnlyTheTopLOD, bForceCutout, bMipToZero ) ) {
			pDlg->ErrorString( "Error creating mipmaps from %s", (cchar *)sFilename );
			return FALSE;
		}
	}

	if( bDetailMap ) {
		MipMaps.BiasLowerLodsToValue( 255 );
		MipMaps.CutAllPixelValuesByHalf();
	}

	// convert mipmaps
	CCreateGCTgaFile GCTga;
	if( !GCTga.ConvertTgaFile( MipMaps, (cchar *)sRawAssetName, bDither, nFormat ) ) {
		pDlg->ErrorString( "Error creating GC textures from %s", (cchar *)sFilename );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nCRCValue = GCTga.GetDataCRC();
	u32 nConvertedSize = GCTga.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize, nCRCValue );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
		return FALSE;
	}

	// write out our data to the master file
	if( !GCTga.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
		return FALSE;
	}
	// update our status fields
	if ( !pDlg->m_bBuildingLightMaps )
	{
		pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
		pDlg->InfoString( "Used %d bytes in master file", nConvertedSize );
	}

	// update the support entries
	if( pAidFileInfo ) {
		pDlg->m_MasterFile.UpdateOrAddSupportEntry( pAidFileInfo );
	}

	return TRUE;
}


#ifdef _MMI_TARGET_PS2
// ========================= Playstation 2 =================================================

#define MAX_TEXTURE_BYTES	(256*1024)
#define _MMI_PS2_MAXDIM	512
// KJ Begin

BOOL bAlpha;

BOOL ConvertTgaFile_PS2( const CFileInfo *pFileInfo, CCompileDlg *pDlg, const CString &rsOutputFilename,
									 CAidFile &rAidFile, CFileInfo *pAidFileInfo ) {
	CTgaFileLoader TgaFile;
	CGenMipMaps MipMaps;
	CString sError, sFilename, sRawAssetName, sDesiredFormat;
	BOOL bGenOnlyTheTopLOD, bForceCutout, bMipToZero, bDither, bDownSize, bDetailMap, bBumpMap, bBumpWraps, bLayer;
	s32	bKBias = -104;	// NS Default values to use
	u8	bMipMapFilter = 5, bLBias = 0;	// NS Default values to use
	u32 nLargestDim, nSmallestDim;	//ARG
	f32 fBumpScale;

	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	sRawAssetName = rsOutputFilename;
	StripExtension( sRawAssetName );

	// set the settings to their default values
	bGenOnlyTheTopLOD = FALSE;
	bForceCutout = FALSE;
	bMipToZero = TRUE;
    bDither = TRUE;
	sDesiredFormat = "auto";
	bDownSize = TRUE;
	bDetailMap = FALSE;
	bBumpMap = FALSE;
	bBumpWraps = TRUE;
	fBumpScale = 0.0f;
	bLayer = FALSE;

	// check the aid file for user supplied settings
	if( rAidFile.m_bDataValid ) {
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].bValueWasFound ) {
			if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "detail" ) == 0 ) {
				bDetailMap = TRUE;
                return (FALSE);         // NS skip detail maps
            } else if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "bump" ) == 0 ) {
				bBumpMap = TRUE;
                return (FALSE);         // NS skip bump maps
			}
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValueWasFound ) {
			bGenOnlyTheTopLOD = !rAidFile.m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValue;
		}
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].bValueWasFound ) {
			// only 'cutout' means anything on the xbox side
			if( rAidFile.m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue.CompareNoCase( "cutout" ) == 0 ) {
				bForceCutout = TRUE;
			}
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValueWasFound ) {
			bMipToZero = !rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_DITHER].bValueWasFound ) {
			bDither = rAidFile.m_aNonStrings[AID_FILE_FLOATS_DITHER].bValue;
		}
		if( rAidFile.m_aStrings[AID_FILE_STRINGS_PS2_TEX_FORMAT].bValueWasFound ) {
			sDesiredFormat = rAidFile.m_aStrings[AID_FILE_STRINGS_PS2_TEX_FORMAT].sValue;
		}
/* KJ
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].bValueWasFound ) {
			bDownSize = rAidFile.m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].bValue;
		}
*/
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].bValueWasFound ) {
			fBumpScale = rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].fValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValueWasFound ) {
			bBumpWraps = rAidFile.m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIPMAP_FILTER].bValueWasFound ) {
			bMipMapFilter = rAidFile.m_aNonStrings[AID_FILE_FLOATS_MIPMAP_FILTER].nValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LBIAS].bValueWasFound ) {
			bLBias = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LBIAS].nValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_KBIAS].bValueWasFound ) {
			bKBias = rAidFile.m_aNonStrings[AID_FILE_FLOATS_KBIAS].nValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_DOWNSIZE].bValueWasFound ) {
			bDownSize = rAidFile.m_aNonStrings[AID_FILE_FLOATS_DOWNSIZE].bValue;
		}
		if( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LAYER].bValueWasFound ) {
			bLayer = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LAYER].bValue;
		}
	}

	// load file
#ifdef _MMI_TARGET_PS2
	if( !TgaFile.LoadTgaFile( (cchar *)pFileInfo->GetFilePath(), FALSE, FALSE ) ) {
		pDlg->ErrorString( "Error loading %s for conversion", (cchar *)sFilename );
//		ErrorString( sError, pDlg );
		return FALSE;
	}
#else
	if( !TgaFile.LoadTgaFile( (cchar *)pFileInfo->GetFilePath(), FALSE, bDetailMap ) ) {
		pDlg->ErrorString( "Error loading %s for conversion", (cchar *)sFilename );
//		ErrorString( sError, pDlg );
		return FALSE;
	}
//#ifndef _MMI_TARGET_PS2
	if( bBumpMap ) {
		// make sure that we have a 32bit image to bump
		if( !TgaFile.Convert32bitAlphaImageIntoUnitFloat() ) {
			pDlg->ErrorString( "Error creating bump map from %s, the tga must be 32 bit", (cchar *)sFilename );
//			SetErrorString( sError, pDlg );
			return FALSE;
		}
		u32 nW = TgaFile.GetWidth();
		u32 nH = TgaFile.GetHeight();
		f32 *pfSrcPixels = (f32 *)TgaFile.GetPixelData();
		f32 fTex00, fTex01, fTex10, fDs, fDt, fValue;

		// allocate some working memory to store the computed I and A values
		u8 *pDestMem, *pAlloctedMem;
		pAlloctedMem = new u8[nW * nH * 2];
		if( !pAlloctedMem ) {
			pDlg->ErrorString( "Error creating bump map from %s, could not allocate working memory", (cchar *)sFilename );
//			SetErrorString( sError, pDlg );
			return FALSE;
		}
		pDestMem = pAlloctedMem;
		u32 i, j, nIndex;
		for( i=0; i < nH; i++ ) {
			for( j=0; j < nW; j++ ) {
				nIndex = (i * nW) + j;
				// look up the 3 pixels used in the calculations
				fTex00 = pfSrcPixels[nIndex];

				if( (j+1) < nW ) {
					// grab the pixel to the right
					fTex01 = pfSrcPixels[nIndex+1];
				} else {
					if( bBumpWraps ) {
						// wrap to the pixel on the left side of the texture
						fTex01 = pfSrcPixels[nIndex+1-nW];
					} else {
						// clamp to the last pixel
						fTex01 = fTex00;
					}
				}

				if( (i+1) < nH ) {
					// grab the pixel below
					fTex10 = pfSrcPixels[nIndex+nW];
				} else {
					if( bBumpWraps ) {
						// grab the pixel on the top of the texture
						fTex10 = pfSrcPixels[j];
					} else {
						fTex10 = fTex00;
					}
				}

				// compute the delta values
				fDs = fTex00 - fTex01;
				fDt = fTex00 - fTex10;

				// calculate the intensity
				fValue = (fDs * 128.0f) + 127.0f;
				FMATH_CLAMP( fValue, 0.0f, 255.0f );
				pDestMem[0] = (u8)fValue;

				// calculate the alpha
				fValue = (fDt * 128.0f) + 127.0f;
				FMATH_CLAMP( fValue, 0.0f, 255.0f );
				pDestMem[1] = (u8)fValue;

				pDestMem += 2;
			}
		}

		// copy our I and A values back into the tga, over the float values
		u8 *pI = pAlloctedMem;
		u8 *pA = &pI[1];
		pDestMem = (u8 *)pfSrcPixels;
		for( i=0; i < nH; i++ ) {
			for( j=0; j < nW; j++ ) {
				pDestMem[0] = pDestMem[1] = pDestMem[2] = *pI;
				pDestMem[3] = *pA;

				pI += 2;
				pA += 2;
				pDestMem += 4;
			}
		}

		// free our temp memory
		delete [] pAlloctedMem;
	}
#endif

	nLargestDim = TgaFile.GetLargestDim();
//ARG	if( (pDlg->m_bDownsizeTextures && nLargestDim > 64) ||
//ARG		(bDownSize && nLargestDim > _MMI_PS2_MAXDIM) ) {
//ARG
//ARG		do {
//ARG				nLargestDim >>= 1;// cut in half
//ARG		}while ( nLargestDim > _MMI_PS2_MAXDIM);
//ARG	} else {
//ARG        nLargestDim = _MMI_PS2_MAXDIM;
//ARG	}
//ARG - >>>>>
//ARG - from GC
	nSmallestDim = TgaFile.GetSmallestDim();
	if( nSmallestDim >= 32 ) {
		// only think about reducing texture size if the smallest dimension is >= 32
		if( nLargestDim > _MMI_PS2_MAXDIM ) {
			// we can't have any texture bigger than 1024 on GC
			nLargestDim = _MMI_PS2_MAXDIM;
		} else if( (pDlg->m_bDownsizeTextures && nLargestDim > 64) || (bDownSize && nLargestDim > 64) ) {
			nLargestDim >>= 1;// cut in half
		} else {
			nLargestDim = _MMI_PS2_MAXDIM;
		}
	}
//ARG - <<<<<

	// generate mipmaps
	CCreatePS2TgaFile::PS2_Format_e nFormat;
    if( !bDetailMap && !bBumpMap && !bLayer) {
		BOOL bMipsWillBeACutout = MipMaps.WillTGABeACutout( TgaFile, bForceCutout );
		nFormat = CCreatePS2TgaFile::GetFormatFromString( bMipsWillBeACutout, sDesiredFormat /* KJ, TgaFile.GetBitsPerPixel() */);
	}
/* KJ
	else if( bDetailMap ) {
		bForceCutout = FALSE;
		nFormat = CCreatePS2TgaFile::GC_FORMAT_S3TC;// force dxt1 compression for detail maps
	}
*/
	else if( bBumpMap ) {
		bForceCutout = FALSE;
		nFormat	= CCreatePS2TgaFile::PS2_FORMAT_I8;// Jeremy: force IA8 compression for bump maps
	}
	else if( bLayer ) {
		bForceCutout = FALSE;
		if(bAlpha)
			nFormat	=  CCreatePS2TgaFile::PS2_FORMAT_8888;
		else
			nFormat	=  CCreatePS2TgaFile::PS2_FORMAT_1555;
	}

/* KJ
	if( nFormat == CCreateGCTgaFile::GC_FORMAT_S3TC ||
		nFormat == CCreateGCTgaFile::GC_FORMAT_S3TCA1 ||
		nFormat == CCreateGCTgaFile::GC_FORMAT_S3TCx2 ) {
		// don't go down smaller than 8 in any dimension, and maintain the aspect ratio
		if( !MipMaps.GenerateMipMaps( TgaFile, nLargestDim, 8, TRUE, bGenOnlyTheTopLOD, bForceCutout, bMipToZero ) ) {
			sError.Format( "Error creating mipmaps from %s", (cchar *)sFilename );
			SetErrorString( sError, pDlg );
			return FALSE;
		}
	} else
*/
	{
		// don't go down smaller than 4 in any dimension, and maintain the aspect ratio
		// NS - On PS2 don't go down smaller than 8 in any dimension, and maintain the aspect ratio

		if( !MipMaps.GenerateMipMaps( TgaFile, nLargestDim, 8, TRUE, bGenOnlyTheTopLOD, bForceCutout, bMipToZero ) ) {
			pDlg->ErrorString( "Error creating mipmaps from %s", (cchar *)sFilename );
//			SetErrorString( sError, pDlg );
			return FALSE;
		}
	}

	if( bDetailMap ) {
		MipMaps.BiasLowerLodsToValue( 255 );
		MipMaps.CutAllPixelValuesByHalf();
	}

	// convert mipmaps
	CCreatePS2TgaFile PS2Tga;

	PS2Tga.SetKBias( bKBias );
    PS2Tga.SetLBias( bLBias );
    PS2Tga.SetMipMapFilter( bMipMapFilter );

	if( !PS2Tga.ConvertTgaFile( MipMaps, (cchar *)sRawAssetName, bDither, nFormat ) ) {
		pDlg->ErrorString( "Error creating PS2 textures from %s", (cchar *)sFilename );
//		SetErrorString( sError, pDlg );
		return FALSE;
	}

	// grab a file pointer into the master file
	u32 nConvertedSize = PS2Tga.GetSizeOfConvertedFile();
	FILE *pDstFile = pDlg->m_MasterFile.GetCompilationDataFilePtr( pFileInfo, rsOutputFilename, nConvertedSize );
	if( !pDstFile ) {
		pDlg->ErrorString( "Error creating master file entry for %s", (cchar *)sFilename );
//		SetErrorString( sError, pDlg );
		return FALSE;
	}

	// write out our data to the master file
	if( !PS2Tga.WriteConvertedFile( (cchar *)rsOutputFilename, pDstFile ) ) {
		pDlg->m_MasterFile.DeleteEntry( rsOutputFilename );
		pDlg->ErrorString( "Error writing %s to master file", (cchar *)sFilename );
//		SetErrorString( sError, pDlg );
		return FALSE;
	}
	// update our status fields
//    ClearStatusStrings( pDlg );
    pDlg->InfoString( "%s compiled OK", (cchar *)sFilename );
    pDlg->InfoString( "Used %d bytes in master file", nConvertedSize );

	// update the support entries
	if( pAidFileInfo ) {
		pDlg->m_MasterFile.UpdateOrAddSupportEntry( pAidFileInfo );
	}

	return TRUE;
}
#endif

//
//
//
BOOL _SavedKNGIsUpToDate( CCompileDlg *pDlg, cchar *pszKNGFileName, const CFileInfo *pFileInfo, const CFileInfo *pAidFileInfo )
{
	FASSERT( pFileInfo );

	KongFileHeader_t TestKHeader;
	SYSTEMTIME TestTime;
	u32 nBytesRead;

	if ( !ReadKongHeader( pszKNGFileName, &TestKHeader ) )
	{
		return FALSE;
	}

	if ( pDlg->m_bGenerateLMSTs && !TestKHeader.bHasLightmapsSTs )
	{
		// Requestor needs lightmaps, but this KNG doesn't have any
		return FALSE;
	}

	if ( pDlg->m_bGenerateVertRad )
	{
		// Check for up to date remap file
		CString csName = pszKNGFileName;
		csName.MakeLower();
		if ( pDlg->m_nTargetPlatform == PASM_TARGET_GC )
		{
			csName.Replace( ".kng", "_gc.rmp" );
		}
		else if ( pDlg->m_nTargetPlatform == PASM_TARGET_PC )
		{
			csName.Replace( ".kng", "_pc.rmp" );
		}
		else if ( pDlg->m_nTargetPlatform == PASM_TARGET_XB )
		{
			csName.Replace( ".kng", "_pc.rmp" );
		}
//ARG - >>>>>
#ifdef _MMI_TARGET_PS2
		else if ( pDlg->m_nTargetPlatform == PASM_TARGET_PS2 )
		{
			csName.Replace( ".kng", "_ps2.rmp" );
		}
#endif
//ARG - <<<<<
		HANDLE hFile = CreateFile( csName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			return FALSE;
		}
		ReadFile( hFile, &TestTime, sizeof( SYSTEMTIME ), (LPDWORD)&nBytesRead, NULL );
		CloseHandle( hFile );
		if ( nBytesRead != sizeof( SYSTEMTIME ) )
		{
			return FALSE;
		}

		// Compare the remap file time and the APE file time
		if ( memcmp( &TestTime, &TestKHeader.APELastWriteTime, sizeof(SYSTEMTIME) ) != 0 )
		{
			// Remap file time does not match APE file time
			return FALSE;
		}
	}

	pFileInfo->GetLastWriteTime().GetAsSystemTime( TestTime );
	if ( memcmp( &TestTime, &TestKHeader.APELastWriteTime, sizeof(SYSTEMTIME) ) == 0 )
	{
		if ( pAidFileInfo )
		{
			pAidFileInfo->GetLastWriteTime().GetAsSystemTime( TestTime );
			if ( memcmp( &TestTime, &TestKHeader.AIDLastWriteTime, sizeof(SYSTEMTIME) ) == 0)
			{
				// Signature, version, and file time stamps all match, so we are up to date, already
				return TRUE;
			}
		}
		else if ( !TestKHeader.bHasAidFile )
		{
			// Signature, version and APE file time stamps match (and both do not have an AID file), so we are up to date already.
			return TRUE;
		}
	}

	return FALSE;
}


// call this function before calling the ConvertApeFile_ functions, it will return the
// correct output filename and will fill in rVisFile if successfull.
BOOL ApePreConvert( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CString &rsOutputFilename, CKongToVisFile &rVisFile,
				   BOOL &rbNeedsCompiling, CFileInfo **ppAidFileInfo, CAidFile &AidFile )
{
	CString sError, sFilename;
	ApeMesh_t MeshInfo;
	BOOL bDoesAidNeedCompiled;

	/////////////////////////////////////////////////////////
	// determine if this file needs compiling, default to yes
	rbNeedsCompiling = TRUE;
	if( !CApeFileLoader::LoadAndValidateApeHeader( (cchar *)pFileInfo->GetFilePath(), MeshInfo, &sError, TRUE ) ) {
		// could not load the ape file, fail
		pDlg->m_nCompileResult = CDLG_RESULT_ASSET_LOAD_UNSUCCESSFUL;
		return FALSE;
	}

	// create the entry name from the file name
	rsOutputFilename.Format( "%s.ape", pFileInfo->GetFileTitle() );
	bDoesAidNeedCompiled = FALSE;// no aid file with meshes
	bDoesAidNeedCompiled = DoesAidFileNeedCompiled( pFileInfo, pDlg, ppAidFileInfo );
//	*ppAidFileInfo = NULL;

	pDlg->WriteToLog( "Pre-converting %s.\n", rsOutputFilename );

	rsOutputFilename.MakeLower();
	// see if the master file is already up to date for this file
	rbNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, rsOutputFilename );

	if( !rbNeedsCompiling && !bDoesAidNeedCompiled && !pDlg->m_bBuildingLightMaps )
	{
		// doesn't need compiling
		pDlg->m_nCompileResult = CDLG_RESULT_UP_TO_DATE_MASTERFILE;
		return FALSE;
	}

	rbNeedsCompiling = TRUE;

	// setup the filename (just used for error messages)
	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	if( *ppAidFileInfo ) {
		// the aid file exists
		if( AidFile.Load( *ppAidFileInfo ) ) {
			if( AidFile.m_bDataValid ) {
				// look for specific data here
			}
		} else {
			// the aid file had errors
			pDlg->ErrorString( "Error in aid file %s, please fix errors before recompiling.", (cchar *)(*ppAidFileInfo)->GetFileName() );
			pDlg->m_nCompileResult = CDLG_RESULT_COMPILE_FAILED;
			return FALSE;
		}
	}

/*
	Here we need to check for a valid KNG file
	if ( nFileType == COMPDIAG_FILE_TYPE_WLD )
	{
		// Check to see if there is a valid .vis file to use
		if ( rVisFile.CheckForVisFile( pDlg, pFileInfo, *ppAidFileInfo ) )
		{
			return TRUE;
		}
	}
*/
	rVisFile.m_nAIDFileCRCValue = AidFile.m_nFileCRCValue;

	if ( !rVisFile.LoadApeFile( (cchar *)pFileInfo->GetFilePath() ) )
	{
		pDlg->ErrorString( "Unable to load APE file %s", (cchar *)sFilename );
		return FALSE;
	}

	// Convert our file (convert it from ape->kong->vis (the last step only if need be))
	if( !rVisFile.ConvertKongFile( pDlg, (cchar *)pFileInfo->GetFilePath() ) ) {
		pDlg->ErrorString( "Error during ape->kong->vis conversion of %s", (cchar *)sFilename );
		pDlg->m_nCompileResult = CDLG_RESULT_COMPILE_FAILED;
		return FALSE;
	}

	// Record file times for later comparison
	pFileInfo->GetLastWriteTime().GetAsSystemTime( rVisFile.m_pKongMesh->KHeader.APELastWriteTime );
	rVisFile.m_pKongMesh->KHeader.bHasAidFile = FALSE;
	if ( *ppAidFileInfo )
	{
		rVisFile.m_pKongMesh->KHeader.bHasAidFile = TRUE;
		(*ppAidFileInfo)->GetLastWriteTime().GetAsSystemTime( rVisFile.m_pKongMesh->KHeader.AIDLastWriteTime );
	}

	return TRUE;
}


// call this function before calling the ConvertWldFile_ functions, it will return the
// correct output filename and will fill in rVisFile if successfull.
BOOL WldPreConvert( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CString &rsOutputFilename, CKongToVisFile &rVisFile,
				    BOOL &rbNeedsCompiling, CFileInfo **ppAidFileInfo, CAidFile &AidFile )
{
	CString sError, sFilename;
	ApeMesh_t MeshInfo;
	BOOL bDoesAidNeedCompiled;

	/////////////////////////////////////////////////////////
	// determine if this file needs compiling, default to yes
	rbNeedsCompiling = TRUE;
	if( !CApeFileLoader::LoadAndValidateApeHeader( (cchar *)pFileInfo->GetFilePath(), MeshInfo, &sError, TRUE ) ) {
		// could not load the ape file, fail
		return FALSE;
	}

	// create the entry name from the file name
	CompileDlg_nWorldStartTime = timeGetTime();
	rsOutputFilename.Format( "%s.wld", pFileInfo->GetFileTitle() );
	bDoesAidNeedCompiled = DoesAidFileNeedCompiled( pFileInfo, pDlg, ppAidFileInfo );

	rsOutputFilename.MakeLower();
	// see if the master file is already up to date for this file
	rbNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, rsOutputFilename );

	if( !rbNeedsCompiling && !bDoesAidNeedCompiled )
	{
		// doesn't need compiling
		return FALSE;
	}

	rbNeedsCompiling = TRUE;

	// setup the filename (just used for error messages)
	sFilename = pFileInfo->GetFileName();
	sFilename.MakeLower();

	if( *ppAidFileInfo ) {
		// the aid file exists
		if( AidFile.Load( *ppAidFileInfo ) ) {
			if( AidFile.m_bDataValid ) {
				// look for specific data here
			}
		} else {
			// the aid file had errors
			pDlg->ErrorString( "Error in aid file %s, please fix errors before recompiling.", (cchar *)(*ppAidFileInfo)->GetFileName() );
			return FALSE;
		}
	}

	// Check to see if there is a valid .vis file to use
	u32 nLightMapDetail = ((CPasmDlg *)theApp.m_pMainWnd)->m_nLightingQuality;
	f32 fLightMapMemory = ((CPasmDlg *)theApp.m_pMainWnd)->m_fMaxLMapSize;
	f32 fLightMapSubSample = ((CPasmDlg *)theApp.m_pMainWnd)->m_fSubSample;
	if ( AidFile.m_bDataValid )
	{
		if ( AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].bValueWasFound )
		{
			pDlg->InfoString( "Overriding PASM lightmap detail of %d with AID file detail of %d.", nLightMapDetail, AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].nValue );
			nLightMapDetail = AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].nValue;
		}
		if ( AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_MBS].bValueWasFound )
		{
			pDlg->InfoString( "Overriding PASM lightmap memory of %5.2f MBs with AID file of %5.2f MBs.", fLightMapMemory, AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_MBS].fValue );
			fLightMapMemory = AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_MBS].fValue;
		}
		if ( AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE].bValueWasFound )
		{
			pDlg->InfoString( "Overriding PASM lightmap subsample of %5.2f with AID file of %5.2f.", fLightMapSubSample, AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE].fValue );
			fLightMapSubSample = AidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE].fValue;
		}
	}

	if ( !rVisFile.LoadApeFile( (cchar *)pFileInfo->GetFilePath() ) )
	{
		pDlg->ErrorString( "Unable to load WLD file %s", (cchar *)sFilename );
		return FALSE;
	}

	rVisFile.m_nAIDFileCRCValue = AidFile.m_nFileCRCValue;

	if ( !pDlg->m_bIgnoreVIS && rVisFile.CheckForVisFile( pDlg, pFileInfo, *ppAidFileInfo, nLightMapDetail, fLightMapMemory ) )
	{
		return TRUE;
	}
	// Convert our file (convert it from ape->kong->vis (the last step only if need be))
	if( !rVisFile.ConvertKongFile( pDlg, (cchar *)pFileInfo->GetFilePath() ) )
	{
		pDlg->ErrorString( "Error during ape->kong->vis conversion of %s", (cchar *)sFilename );
		return FALSE;
	}

	// Record file times for later comparison
	pFileInfo->GetLastWriteTime().GetAsSystemTime( rVisFile.m_pKongMesh->KHeader.APELastWriteTime );
	rVisFile.m_pKongMesh->KHeader.bHasAidFile = FALSE;
	if ( *ppAidFileInfo )
	{
		rVisFile.m_pKongMesh->KHeader.bHasAidFile = TRUE;
		(*ppAidFileInfo)->GetLastWriteTime().GetAsSystemTime( rVisFile.m_pKongMesh->KHeader.AIDLastWriteTime );
	}

	return TRUE;
}

// call this function before calling the Convert... functions, it will return the
// correct output filename and will fill in rAidFile if it exists if successfull.
BOOL PreConvert( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CString &rsOutputFilename,
							  CAidFile &rAidFile, BOOL &rbNeedsCompiling, CFileInfo **ppAidFileInfo ) {
	BOOL bDoesAidNeedCompiled;

	// make sure that the output filename is lowercase
	rsOutputFilename.MakeLower();

	// see if the tga file needs to be compiled
	rbNeedsCompiling = (pDlg->m_bForceCompile) ? TRUE : pDlg->m_MasterFile.DoesFileInfoNeedCompiling( pFileInfo, rsOutputFilename );

	// see if there is an aid file
	bDoesAidNeedCompiled = DoesAidFileNeedCompiled( pFileInfo, pDlg, ppAidFileInfo );

	if( !rbNeedsCompiling && !bDoesAidNeedCompiled ) {
		// doesn't need compiling
		return FALSE;
	}
	rbNeedsCompiling = TRUE;

	if( *ppAidFileInfo ) {
		// the aid file exists, go ahead and parse it
		if( !rAidFile.Load( *ppAidFileInfo ) ) {
			// the aid file had errors
			CString sError;
			pDlg->ErrorString( "Error in aid file: '%s', please fix errors before recompiling.", (cchar *)(*ppAidFileInfo)->GetFileName() );
			return FALSE;
		}
	}

	return TRUE;
}


// Takes a pFileInfo and determines if the aid file (if it exists) needs to be compiled.
// If the AID file doesn't exist, this function will return FALSE.
// TRUE will only be returned if the AID file exists and (if a support entry exists) the times don't match.
// Regardless of the return value *ppAidFileInfo will be filled in (either NULL or a valid CFileInfo)
BOOL DoesAidFileNeedCompiled( const CFileInfo *pFileInfo, CCompileDlg *pDlg, CFileInfo **ppAidFileInfo ) {
	CString sAidName;
	void *pLookup;
	FDataPrjFile_SupportEntry_t *pSupportEntry;

	*ppAidFileInfo = NULL;

	if( !pFileInfo ) {
		return FALSE;
	}
	// create the AID filename
	CreateAidEntryName( pFileInfo, sAidName );

	// lookup the actual AID file
	if( !pDlg->m_MapAidFiles.Lookup( sAidName, pLookup ) ) {

		if ( pDlg->m_bCompilingPASMGeneratedAsset )
		{
			// Lightmap TGA's might exist but not be in the map, yet
			static CFileInfo FileInfo;
			sAidName = pFileInfo->GetFileDrive() + pFileInfo->GetFileDir() + sAidName;
			FileInfo.Create( sAidName );

			if ( !FileInfo.GetLength() ) {
				// the AID file doesn't exist, no need to compile
				return FALSE;
			}

			pLookup = &FileInfo;
		} else {
			// the AID file doesn't exist, no need to compile
			return FALSE;
		}
	}
	*ppAidFileInfo = (CFileInfo *)pLookup;

	// grab the support entry for the AID
	pSupportEntry = pDlg->m_MasterFile.GetSupportEntryByName( sAidName );
	if( !pSupportEntry ) {
		// a support entry doesn't exist yet, this AID needs to be compiled
		return TRUE;
	}
	if( pDlg->m_MasterFile.DoModifiedTimesMatch( pSupportEntry, *ppAidFileInfo ) ) {
		// the time in the master file match the AID file time, no need to compile
		return FALSE;
	}

	return TRUE;
}

void CreateAidEntryName( const CFileInfo *pFileInfo, CString &rsEntryName ) {

	rsEntryName = pFileInfo->GetFileTitle();
	rsEntryName.MakeLower();
	rsEntryName += ".aid";
}

void StripExtension( CString &sString ) {

	int nIndex = sString.ReverseFind( '.' );
	if( nIndex >= 0 ) {
		sString.Delete( nIndex, sString.GetLength() - nIndex );
	}
}

