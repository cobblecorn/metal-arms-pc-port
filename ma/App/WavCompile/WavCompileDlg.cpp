// WavCompileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fang.h"
#include "WavCompile.h"
#include "WavCompileDlg.h"
#include "fversion.h"
#include "pickdir.h"
#include "settings.h"
#include "WavFile.h"
#include "WavToADPCM_Xbox.h"
#include "WavToADPCM_GC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define _MIN_WAV_BYTES		4096

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
// CWavCompileDlg dialog

CWavCompileDlg::CWavCompileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWavCompileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWavCompileDlg)
	m_sNumFiles = _T("");
	m_sVersion = _T("");
	m_sGCDir = _T("");
	m_sXBDir = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_WavFiles.RemoveAll();
}

void CWavCompileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWavCompileDlg)
	DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFileList);
	DDX_Text(pDX, IDC_NUM_FILES, m_sNumFiles);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Text(pDX, IDC_GC_DIR, m_sGCDir);
	DDX_Text(pDX, IDC_XB_DIR, m_sXBDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWavCompileDlg, CDialog)
	//{{AFX_MSG_MAP(CWavCompileDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_COMMAND(IDC_CLEAR_FILE_LIST, OnClearFileList)
	ON_BN_CLICKED(IDC_COMPILE_BUTTON, OnCompileButton)
	ON_NOTIFY(NM_RCLICK, IDC_FILE_LIST, OnRclickFileList)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_LIST, OnDblclkFileList)
	ON_BN_CLICKED(IDC_CHOOSE_GC_DIR, OnChooseGcDir)
	ON_BN_CLICKED(IDC_CHOOSE_XB_DIR, OnChooseXbDir)
	ON_COMMAND(IDC_VIEW_FILE_STATS, OnViewFileStats)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWavCompileDlg message handlers

BOOL CWavCompileDlg::OnInitDialog()
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

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	::SetProp( GetSafeHwnd(), WavCompile_pszPropName, (HANDLE)1 );

	////////////////////////////
	// SET OUR SETTINGS FILENAME
	CSettings::SetSettingsFilename( "wav compile.ini" ); 
	
	////////////////////////////////////////////////
	// GET THE DEFAULT VALUES FROM OUR SETTINGS FILE
	CSettings& Settings = CSettings::GetCurrent();
	
	m_sXBDir = Settings.GetXBDir();
	m_sGCDir = Settings.GetGCDir();
	BOOL bFileAscSort = Settings.GetFileAscendingSort();
	u32 nFileSortCol = Settings.GetFileSortColumn();

	///////////////////////////////////
	// SETUP THE FILE VIEW LIST CONTROL
	CRect Rect;
	m_ctrlFileList.GetClientRect( &Rect );
	int nColLen = Rect.Width() - 16;
	int nColX = nColLen / 7;// subtract 16 to account for the right scroll bar width
	m_ctrlFileList.InsertColumn( 0, "Name",		LVCFMT_LEFT, nColX * 4 );
	m_ctrlFileList.InsertColumn( 1, "Size",		LVCFMT_RIGHT, nColX );
	m_ctrlFileList.InsertColumn( 2, "Modified",	LVCFMT_LEFT, nColX * 2 );
		
	// set some properties for our list box
	m_ctrlFileList.SetFullRowSel( TRUE );
	m_ctrlFileList.SortColumn( nFileSortCol, bFileAscSort );	
	m_ctrlFileList.EnableSeparatorLines( TRUE );

	// since there are no files yet, update our var
	m_sNumFiles = "0 object(s)";

	////////////////////////
	// display the version #
	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );
	
	UpdateData( VARS_TO_CONTROLS );

	fang_Init();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWavCompileDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWavCompileDlg::OnPaint() {

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

HCURSOR CWavCompileDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

void CWavCompileDlg::OnCancel() {

	UpdateData( CONTROLS_TO_VARS );

	// writeout our settings before we exit
	CSettings& Settings = CSettings::GetCurrent();
	
	Settings.m_sXBDir = m_sXBDir;
	Settings.m_sGCDir = m_sGCDir;
	Settings.m_nFileSortColumn = m_ctrlFileList.GetSortedColumn();
	Settings.m_bFileAscendingSort = m_ctrlFileList.IsAscSorted();
		
	Settings.SaveCommonDataOutToFile();

	::RemoveProp( GetSafeHwnd(), WavCompile_pszPropName );
		
	CDialog::OnCancel();
}

void CWavCompileDlg::OnOK() {
	//CDialog::OnCancel();
}

void CWavCompileDlg::OnDropFiles( HDROP hdrop ) {

	u32 nNumFiles, i, nNumFileInfos, j;
	TCHAR szNextFile [MAX_PATH];
	CString sFilename;
	int nIndex;
	u32 nFilesAdded = 0;
	const CFileInfo *pFileInfoData;
	CFileInfo *pFileInfo1, *pFileInfo2;
	
    // Get the # of files being dropped.
    nNumFiles = DragQueryFile( hdrop, -1, NULL, 0 );

    for( i=0; i < nNumFiles; i++ ) {
		 // Get the next filename from the HDROP info.
        if( DragQueryFile( hdrop, i, szNextFile, MAX_PATH ) > 0 ) {
			sFilename = szNextFile;
			sFilename.MakeLower();
			if( sFilename.Find( ".wav" ) >= 0 ) {
				// most likely this is a wav file
				nIndex = m_WavFiles.AddFile( szNextFile, CFileInfoArray::AP_NOSORT );
				if( nIndex >= 0 ) {
					// a file was added
					nFilesAdded++;
					
					// make sure we haven't dropped a duplicate file
					if( nIndex > 1 ) {
						nNumFileInfos = m_WavFiles.GetSize() - 1;
						pFileInfoData = m_WavFiles.GetData();
						pFileInfo1 = (CFileInfo *)&pFileInfoData[nIndex];
						sFilename = pFileInfo1->GetFileName();
						for( j=0; j < nNumFileInfos; j++ ) {
							pFileInfo2 = (CFileInfo *)&pFileInfoData[j];

							if( pFileInfo2->GetFileName().CompareNoCase( sFilename ) == 0 ) {
								// we already have a file with this name, don't allow duplicates
								m_WavFiles.RemoveAt( nIndex, 1 );
								nFilesAdded--;
								break;
							}
						}
					}
				}
			}
        }
    }

    // Free up memory.
    DragFinish( hdrop );
	
	if( nFilesAdded ) {
		UpdateFileListBox( TRUE );		
	}
}

void CWavCompileDlg::UpdateFileListBox( BOOL bReSortList ) {
	int i, nNumFiles;
	CString sFilename;
	CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData;
	CFileListItemInfo *pItemInfo;
	
	// empty the file list box
	m_ctrlFileList.DeleteAllItems();

	// add all of the wav files in our list
	nNumFiles = m_WavFiles.GetSize();
	pFileInfoData = m_WavFiles.GetData();
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];
		
		sFilename = pFileInfo->GetFilePath();
		
		// add a new item to the list box control
		m_ctrlFileList.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CFileListItemInfo( i, sFilename, pFileInfo );
		FASSERT( pItemInfo );
		m_ctrlFileList.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlFileList.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlFileList.SetItemText( i, 2, LPSTR_TEXTCALLBACK );		
	}

	// resort the file list
	if( bReSortList ) {
		m_ctrlFileList.ReSort();
	}

	m_sNumFiles.Format( "%d object(s)", nNumFiles );
			
	UpdateData( VARS_TO_CONTROLS );
}

void CWavCompileDlg::OnClearFileList() {
	
	UpdateData( CONTROLS_TO_VARS );

	m_WavFiles.RemoveAll();

	UpdateFileListBox( TRUE );
}

void CWavCompileDlg::OnCompileButton() {
	
	UpdateData( CONTROLS_TO_VARS );

	int nNumFiles, i, nNumNotCorrectWavFormat, nNumXBConverted, nNumGCConverted, nWavOpenErrors, nXBConvertErrors, nGCConvertErrors;
	CWaveFile WavFile;
	CFileListItemInfo *pItemInfo;
	CFileInfo *pFileInfo;
	CString sError, sTemp;

	// make sure that there are files to compile
	nNumFiles = m_ctrlFileList.GetItemCount();
	if( nNumFiles < 1 ) {
		// no files to compile
		return;
	}

	// make sure that the output paths are set
	if( m_sXBDir.IsEmpty() ||
		m_sGCDir.IsEmpty() ||
		m_sXBDir.CompareNoCase( m_sGCDir ) == 0 ) {
		MessageBox( "You must first select an Xbox and GC output directory.\n"
					"The 2 directories cannot be the same.", 
					_T("Wav Compiler"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		return;
	}

	BeginWaitCursor();

	// walk the files
	nNumNotCorrectWavFormat = 0;
	nNumXBConverted = 0;
	nNumGCConverted = 0;
	nWavOpenErrors = 0;
	nXBConvertErrors = 0;
	nGCConvertErrors = 0;
	for( i=0; i < nNumFiles; i++ ) {
		pItemInfo = (CFileListItemInfo *)m_ctrlFileList.GetItemData( i );
		FASSERT( pItemInfo );

		pFileInfo = pItemInfo->GetFileInfo();

		// load wav file
		if( !WavFile.Open( (char *)((cchar *)pFileInfo->GetFilePath()), NULL, WAVEFILE_READ ) ) {
			nWavOpenErrors++;
			continue;
		}
		
		// validate the wav file
		if( WavFile.m_pwfx->wBitsPerSample != 16 ||
			(WavFile.m_pwfx->nChannels != 1 && WavFile.m_pwfx->nChannels != 2) ||			
			WavFile.m_pwfx->wFormatTag != WAVE_FORMAT_PCM ||
			WavFile.m_dwSize < _MIN_WAV_BYTES ) {
			nNumNotCorrectWavFormat++;
			WavFile.Close();
			continue;
		}

		// convert the wav file to an xbox usable file
		if( !ConvertWavFile_XB( pFileInfo, WavFile ) ) {
			nXBConvertErrors++;
		} else {
			nNumXBConverted++;
		}
		Sleep(0);

		// reset the wav file so that the next read won't fail
		WavFile.ResetFile();

		// convert the wav file to a gc usable file
		if( !ConvertWavFile_GC( pFileInfo, WavFile ) ) {
			nGCConvertErrors++;
		} else {
			nNumGCConverted++;
		}
		Sleep(0);

		// close the wav file
		WavFile.Close();
	}

	EndWaitCursor();

	// prepare a status messagebox
	if( nNumXBConverted != nNumFiles ||
		nNumGCConverted != nNumFiles ) {
		// tell the user about the problems
		sTemp.Format( "%d of %d files compiled ok for xbox.\n", nNumXBConverted, nNumFiles );
		sError += sTemp;
		sTemp.Format( "%d of %d files compiled ok for gc.\n\n", nNumGCConverted, nNumFiles );
		sError += sTemp;

		if( nNumNotCorrectWavFormat ) {
			sTemp.Format( "Wav files must be:\n"
						   "1 or 2 channels\n"
						   "16-bit samples\n"
						   "Uncompressed PCM windows wav files\n"
						   "At least %dk bytes in size\n"
						   "There were %d file that were not of this type.\n\n", _MIN_WAV_BYTES, nNumNotCorrectWavFormat );
			sError += sTemp;
		}

		if( nXBConvertErrors ) {
			sTemp.Format( "There was a problem converting %d files into the Xbox format\n"
						  "Most likely the XBADPCM codec isn't installed.\n\n", nXBConvertErrors );
			sError += sTemp;
		}
		
		if( nGCConvertErrors ) {
			sTemp.Format( "There was a problem converting %d files into the GC format\n"
						  "Most likely the required dll 'dsptool.dll' isn't installed.\n\n", nGCConvertErrors );
			sError += sTemp;
		}

		MessageBox( sError, _T("Wav Compiler: Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
	} else {
		sTemp.Format( "%d file(s) successfully compiled.", nNumFiles );
		MessageBox( sTemp, _T("Wav Compiler"), MB_ICONINFORMATION|MB_OK|MB_DEFBUTTON1);
	}
}

void CWavCompileDlg::OnRclickFileList( NMHDR* pNMHDR, LRESULT* pResult ) {

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
	menu.LoadMenu( IDR_FILE_POPUP_MENU );
	CMenu *pContextMenu = menu.GetSubMenu( 0 ); 
	m_ctrlFileList.ClientToScreen( &Point );

	// display the popup menu
	pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this );
	
	*pResult = 0;
}

void CWavCompileDlg::OnDblclkFileList( NMHDR* pNMHDR, LRESULT* pResult ) {

	UpdateData( CONTROLS_TO_VARS );

	CPoint Point;
	::GetCursorPos( &Point );
	m_ctrlFileList.ScreenToClient( &Point );
	
	int nIndex = m_ctrlFileList.HitTest( Point );
	if( nIndex >= 0 ) {
		CFileListItemInfo *pItemInfo = (CFileListItemInfo *)m_ctrlFileList.GetItemData( nIndex );
		FASSERT( pItemInfo );
		CFileInfo *pFileInfo = pItemInfo->GetFileInfo();
		if( pFileInfo ) {
			ShellExecute( NULL, "open", pFileInfo->GetFilePath(), NULL, NULL, SW_SHOWNORMAL );
		}
	}
		
	*pResult = 0;
}

void CWavCompileDlg::OnChooseGcDir() {
	CString sNewDir;

	UpdateData( CONTROLS_TO_VARS );
	
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select GameCube Output Directory", m_sGCDir ) == PICKDIR_RET_SUCCESS ) {
		// store our new dir
		m_sGCDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );		
	}	
}

void CWavCompileDlg::OnChooseXbDir() {
	CString sNewDir;

	UpdateData( CONTROLS_TO_VARS );
	
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Xbox Output Directory", m_sXBDir ) == PICKDIR_RET_SUCCESS ) {
		// store our new dir
		m_sXBDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );		
	}
}

void CWavCompileDlg::OnViewFileStats() {
	
	UpdateData( CONTROLS_TO_VARS );

	int nIndex;
	CFileInfo *pFileInfo;
	CString sMsg, sTemp;
	CTime ModifiedTime;

	// grab the first selected file
	POSITION Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	if( !Pos ) { 
		// no file is selected
		return;
	}
	nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );

	CFileListItemInfo *pItemInfo = (CFileListItemInfo *)m_ctrlFileList.GetItemData( nIndex );
	FASSERT( pItemInfo );

	sMsg = "File Properties:\n";
	pFileInfo = pItemInfo->GetFileInfo();
	sTemp.Format( "\t%s\t\n", pFileInfo->GetFilePath() );
	sMsg += sTemp;
	ModifiedTime = pFileInfo->GetLastWriteTime();
	sTemp.Format( "\t%s\t\n", ModifiedTime.Format( "%#c" ) );
	sMsg += sTemp;
	sTemp.Format( "\t(%d bytes)\t\n\n", pFileInfo->GetLength() );
	sMsg += sTemp;
	
	// read the wav file
	CWaveFile WavFile;
	sMsg += "Wav Properties:\n";
	if( !WavFile.Open( (char *)((const char *)(pFileInfo->GetFilePath())), NULL, WAVEFILE_READ ) ) {
		sMsg += "Could not read wav file properites.\n";
	} else {
		//WavFile.m_pwfx->wFormatTag = WAVE_FORMAT_PCM
		sTemp.Format( "\tChannels = %d\n\tSamples Per Sec = %d\n\tBits Per Sample = %d\n\tLength = %.2f secs", 
					  WavFile.m_pwfx->nChannels,
					  WavFile.m_pwfx->nSamplesPerSec,
					  WavFile.m_pwfx->wBitsPerSample,
					  WavFile.m_fSecsOfData );
		sMsg += sTemp;		
	}
	WavFile.Close();

	sTemp.Format( "%s Properties", pFileInfo->GetFileName() );
	this->MessageBox( sMsg, sTemp, MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1);
}

BOOL CWavCompileDlg::ConvertWavFile_XB( const CFileInfo *pFileInfo, CWaveFile &rWavFile ) {
	CString sError, sFilename;

	// convert wav file to xbADPCM 
	CWavToADPCM_XBox XBWav;
	if( !XBWav.ConvertWavFile( rWavFile, pFileInfo->GetFilePath() ) ) {
		return FALSE;
	}

	// create an output filename
	sFilename = m_sXBDir;
	if( sFilename.GetAt( sFilename.GetLength()-1 ) != '\\' ) {
		sFilename += '\\';
	}
	sFilename += pFileInfo->GetFileTitle();
	sFilename += ".wvs";

	// open the file
	FILE *pFileStream = _tfopen( (cchar *)sFilename, _T("wb") );
	if( !pFileStream ) {
		return FALSE;
	}

	// create an dx8 wvs file header
	FDX8Data_WvsFile_Header_t Header;
	Header.fFreqHz = (f32)rWavFile.m_pwfx->nSamplesPerSec;
	Header.fSecs = rWavFile.m_fSecsOfData;
	Header.nNumBytes = XBWav.m_nDataSize;
	Header.WavFormat = XBWav.m_XBWavFormat;

	// write the file to disk
	fwrite( &Header, sizeof( FDX8Data_WvsFile_Header_t ), 1, pFileStream );
	fwrite( XBWav.m_pData, XBWav.m_nDataSize, 1, pFileStream );

	// close the file
	fclose( pFileStream );

	return TRUE;
}

BOOL CWavCompileDlg::ConvertWavFile_GC( const CFileInfo *pFileInfo, CWaveFile &rWavFile ) {
	CString sError, sFilename;

	// convert wav file to gcADPCM 
	CWavToADPCM_GC GCWav;
	if( !GCWav.ConvertWavFile( rWavFile, pFileInfo->GetFilePath() ) ) {
		return FALSE;
	}

	// create an output filename
	sFilename = m_sGCDir;
	if( sFilename.GetAt( sFilename.GetLength()-1 ) != '\\' ) {
		sFilename += '\\';
	}
	sFilename += pFileInfo->GetFileTitle();
	sFilename += ".wvs";

	// open the file
	FILE *pFileStream = _tfopen( (cchar *)sFilename, _T("wb") );
	if( !pFileStream ) {
		return FALSE;
	}

	// write the file to disk
	fwrite( GCWav.m_pData, GCWav.m_nDataSize, 1, pFileStream );

	// close the file
	fclose( pFileStream );

	return TRUE;
}

