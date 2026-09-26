// AssetScanDialog.cpp : implementation file
//

#include "stdafx.h"
#include "pasm.h"
#include "pasmdlg.h"
#include "AssetScanDialog.h"




// AssetScanDialog dialog

#define _UPDATE_CONTROLS				(WM_USER + 700)


BEGIN_MESSAGE_MAP(AssetScanDialog, CDialog)
	ON_MESSAGE( _UPDATE_CONTROLS, UpdateBar )
END_MESSAGE_MAP()


//
//
//
IMPLEMENT_DYNAMIC(AssetScanDialog, CDialog)
AssetScanDialog::AssetScanDialog(CWnd* pParent /*=NULL*/)
	: CDialog(AssetScanDialog::IDD, pParent)
{
	m_pWorkThread = NULL;
	m_pPasmDlg = NULL;
	m_bSetupListBox = FALSE;
}

//
//
//
AssetScanDialog::~AssetScanDialog( void )
{
}

//
//
//
void AssetScanDialog::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCAN_PROGRESS_BAR, m_ScanProgressBar);
}


//
//
//
int AssetScanDialog::DoModal( void ) 
{
	// spawn a thread that is asleep till init dialog is done
	m_pWorkThread = AfxBeginThread( WorkerThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );

	return CDialog::DoModal();
}


//
//
//
BOOL AssetScanDialog::OnInitDialog( void ) 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_ScanProgressBar.SetRange( 0, 100 );
	m_ScanProgressBar.SetPos( 0 );
	m_ScanProgressBar.SetStep( 1 );

	// the very last thing is to wake up the worker thread
	m_pWorkThread->ResumeThread();

	return TRUE;
}


//
//
//
void AssetScanDialog::SetProgressPercent( f32 fUnitPercent )
{
    m_ScanProgressBar.SetPos( (s32)(100 * fUnitPercent) );
}


//
//
//
long AssetScanDialog::UpdateBar( WPARAM wParam, LPARAM lParam )
{
	UpdateData( VARS_TO_CONTROLS );
	return 0;
}


//
//
//
UINT AssetScanDialog::SpinnerThread( LPVOID pParam ) 
{
	AssetScanDialog *pDlg = (AssetScanDialog *)pParam;

	f32 fPercent = 0.f;
	while ( pDlg->m_nSpinnerState == 0 )
	{
		fPercent += 0.01f;
		pDlg->SetProgressPercent( fPercent );
        if ( fPercent >= 1.0f )
		{
			fPercent = 0.f;
		}

		pDlg->PostMessage( _UPDATE_CONTROLS );
//		pDlg->UpdateData( VARS_TO_CONTROLS );

		Sleep( 16 );
	}

	pDlg->m_nSpinnerState = 2;

	return 0;
}


//
//
//
UINT AssetScanDialog::WorkerThread( LPVOID pParam ) 
{
	AssetScanDialog *pDlg = (AssetScanDialog *)pParam;
	u32 nNumDups;
	u32 nNumTooLong;
	u32 nNumZeroByte;
	BOOL bPromptToViewErrorLog;
	CString sOne, sTwo, sThree, sMsg;
	CStringList sFilesDeleted;

	pDlg->BeginWaitCursor();

	pDlg->m_pSpinnerThread = AfxBeginThread( SpinnerThread, pDlg, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );
	pDlg->m_nSpinnerState = 0;
	pDlg->m_pSpinnerThread->ResumeThread();

	bPromptToViewErrorLog = FALSE;
	sMsg.Empty();

	if( !pDlg->m_pPasmDlg->m_bDisableLibrarySupport ) {
		// rescan the library
		pDlg->m_pPasmDlg->ScanLibDrive();
		pDlg->m_pPasmDlg->RemoveBogusFiles( pDlg->m_pPasmDlg->m_LibFiles, TRUE, FALSE, TRUE, TRUE, TRUE, nNumTooLong, nNumZeroByte, nNumDups );
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
		pDlg->m_pPasmDlg->m_nLibFileCount = pDlg->m_pPasmDlg->m_LibFiles.GetSize();
		CFileInfoArray::BuildMapFromFileInfoArray( pDlg->m_pPasmDlg->m_MapLibFiles, pDlg->m_pPasmDlg->m_LibFiles, 
												   CFileInfoArray::AP_IGNORE_CASE | CFileInfoArray::AP_COMPARE_FILENAME_ONLY );
	} else {
		// disable library support
		pDlg->m_pPasmDlg->m_nLibFileCount = 0;
	}

	// rescan the local drive
	pDlg->m_pPasmDlg->ScanLocalDrive();
	if( pDlg->m_bSetupListBox == FALSE ) {
		// we only need to do this the very 1st time we run
		pDlg->m_pPasmDlg->DeleteAnyTempPasmFiles();
	}
	pDlg->m_pPasmDlg->DeleteLocalFilesListedInTheDeleteListFile( FALSE, sFilesDeleted );
	pDlg->m_pPasmDlg->RemoveBogusFiles( pDlg->m_pPasmDlg->m_LocalFiles, TRUE, FALSE, TRUE, TRUE, TRUE, nNumTooLong, nNumZeroByte, nNumDups );
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
	pDlg->m_pPasmDlg->m_nLocalFileCount = pDlg->m_pPasmDlg->m_LocalFiles.GetSize();
	CFileInfoArray::BuildMapFromFileInfoArray( pDlg->m_pPasmDlg->m_MapLocalFiles, pDlg->m_pPasmDlg->m_LocalFiles, 
											   CFileInfoArray::AP_IGNORE_CASE | CFileInfoArray::AP_COMPARE_FILENAME_ONLY );

	// prompt the user about lib & loc file errors
	if( bPromptToViewErrorLog ) {
		sMsg += "Open the error log file?";

		if( pDlg->m_pPasmDlg->MessageBox( sMsg, "PASM ERROR", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			pDlg->m_pPasmDlg->OnClickOpenErrorLog();
		}
	}

	// rescan the master file 
	// will also update the status bars stats
	// (ALWAYS DO THIS LAST!!!)
	pDlg->m_pPasmDlg->ReadMasterFileEntries();
	if( pDlg->m_pPasmDlg->DeleteMasterFilesListedInTheDeleteListFile( TRUE, sFilesDeleted ) ) {
		pDlg->m_pPasmDlg->m_nMasterFileCount = pDlg->m_pPasmDlg->m_MasterFile.GetNumEntries();
		pDlg->m_pPasmDlg->UpdateStatusBarFileCounts();
	}

	if( pDlg->m_bSetupListBox ) {
		pDlg->m_pPasmDlg->UpdateFileListBox( TRUE );
	}
	pDlg->EndWaitCursor();

	pDlg->m_nSpinnerState = 1;

//	pDlg->m_pSpinnerThread->Delete();
//	while ( pDlg->m_nSpinnerState != 2 )
//	{
//	}

    pDlg->PostMessage( WM_COMMAND, IDOK, 0 );
	
	return 0;
}



// AssetScanDialog message handlers
