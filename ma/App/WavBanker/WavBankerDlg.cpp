// WavBankerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WavBanker.h"
#include "WavBankerDlg.h"
#include "fang.h"
#include "pickdir.h"
#include "FileListItemInfo.h"
#include "fversion.h"
#include "Settings.h"
#include "WavBank.h"
#include "fclib.h"
#include "WavFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
// CWavBankerDlg dialog

CWavBankerDlg::CWavBankerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWavBankerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWavBankerDlg)
	m_sCurrentDir = _T("");
	m_sCurrentBank = _T("");
	m_bRecurseSubDirs = FALSE;
	m_sNumFilesShown = _T("");
	m_sVersion = _T("");
	m_sNumBankFilesShown = _T("");
	m_sBankSize = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_WavFiles.RemoveAll();

	m_WavBank.CreateEmpty();

	m_bPromptForSave = FALSE;
}

void CWavBankerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWavBankerDlg)
	DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFileList);
	DDX_Control(pDX, IDC_BANK_LIST,	m_ctrlBankList);
	DDX_Text(pDX, IDC_CURRENT_DIR, m_sCurrentDir);
	DDX_Text(pDX, IDC_CURRENT_BANK, m_sCurrentBank);
	DDX_Check(pDX, IDC_SUB_DIRS_CHECK, m_bRecurseSubDirs);
	DDX_Text(pDX, IDC_NUM_FILES, m_sNumFilesShown);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Text(pDX, IDC_NUM_BANK_FILES, m_sNumBankFilesShown);
	DDX_Text(pDX, IDC_BANK_SIZE, m_sBankSize);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWavBankerDlg, CDialog)
	//{{AFX_MSG_MAP(CWavBankerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHOOSE_CURRENT_DIR, OnChooseCurrentDir)
	ON_BN_CLICKED(IDC_SUB_DIRS_CHECK, OnSubDirsCheck)
	ON_BN_CLICKED(IDC_CHOOSE_CURRENT_BANK, OnChooseCurrentBank)
	ON_BN_CLICKED(IDC_REFRESH_BUTTON, OnRefreshButton)
	ON_NOTIFY(NM_RCLICK, IDC_FILE_LIST, OnRclickFileList)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_LIST, OnDblclkFileList)
	ON_COMMAND(IDC_ADD_TO_BANK, OnAddToBank)
	ON_NOTIFY(NM_RCLICK, IDC_BANK_LIST, OnRclickBankList)
	ON_COMMAND(IDC_SAVE_BANK, OnSaveButton)
	ON_COMMAND(IDC_UPDATE_ENTRIES, OnUpdateButton)
	ON_COMMAND(IDC_DELETE_ENTRIES, OnDeleteButton)
	ON_COMMAND(IDC_REFRESH_FILE_LIST, OnRefreshButton)
	ON_BN_CLICKED(IDC_CREATE_NEW_BANK, OnCreateNewBank)
	ON_COMMAND(IDC_REMOVE_ALL_ENTRIES, OnRemoveAllEntries)
	ON_COMMAND(IDC_VIEW_FILE_STATS, OnViewFileStats)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWavBankerDlg message handlers

BOOL CWavBankerDlg::OnInitDialog()
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

	::SetProp( GetSafeHwnd(), WavBanker_pszPropName, (HANDLE)1 );


	////////////////////////////
	// SET OUR SETTINGS FILENAME
	CSettings::SetSettingsFilename( "wav banker.ini" ); 
	
	////////////////////////////////////////////////
	// GET THE DEFAULT VALUES FROM OUR SETTINGS FILE
	CSettings& Settings = CSettings::GetCurrent();
	
	m_sCurrentDir = Settings.GetLastWavDir();
	m_sCurrentBank = Settings.GetLastBankFile();
	m_bRecurseSubDirs = Settings.GetRecurseSubDirs();
	
	BOOL bBankAscSort = Settings.GetBankAscendingSort();
	BOOL bFileAscSort = Settings.GetFileAscendingSort();
	u32 nBankSortCol = Settings.GetBankSortColumn();
	u32 nFileSortCol = Settings.GetFileSortColumn();

	/////////////////////
	// LOAD OUR BANK FILE
	LoadBankFile( FALSE );
	if( !m_WavBank.IsEmpty() ) {
		// we loaded a bank file, use the settings from there instead of from the ini file
		WavBank_ToolSettings_t *pSettings = m_WavBank.GetFileSettings();
		m_sCurrentDir = pSettings->szLastDirUsed;
		m_bRecurseSubDirs = pSettings->bRecurseSubDirs;
		bBankAscSort = pSettings->bBankAscendingSort;
		bFileAscSort = pSettings->bFileAscendingSort;
		nBankSortCol = pSettings->nBankSortColumn;
		nFileSortCol = pSettings->nFileSortColumn;
	}
	
	//////////////////////
	// SCAN FOR WAVE FILES
	ScanForWavFiles();

	///////////////////////////////////
	// SETUP THE FILE VIEW LIST CONTROL
	CRect Rect;
	m_ctrlFileList.GetClientRect( &Rect );
	int nColLen = Rect.Width() - 16;
	int nColX = nColLen / 7;// subtract 16 to account for the right scroll bar width
	m_ctrlFileList.InsertColumn( 0, "Name",		LVCFMT_LEFT, nColX * 4 );
	m_ctrlFileList.InsertColumn( 1, "Size",		LVCFMT_RIGHT, nColX );
	m_ctrlFileList.InsertColumn( 2, "Modified",	LVCFMT_LEFT, nColX * 2 );
		
	UpdateFileListBox( FALSE );

	// set some properties for our list box
	m_ctrlFileList.SetFullRowSel( TRUE );
	m_ctrlFileList.SortColumn( nFileSortCol, bFileAscSort );	
	m_ctrlFileList.EnableSeparatorLines( TRUE );

	///////////////////////////////////
	// SETUP THE BANK VIEW LIST CONTROL
	Rect;
	m_ctrlBankList.GetClientRect( &Rect );
	nColLen = Rect.Width() - 16;
	nColX = nColLen / 7;// subtract 16 to account for the right scroll bar width
	m_ctrlBankList.InsertColumn( 0, "Name",		LVCFMT_LEFT, nColX * 4 );
	m_ctrlBankList.InsertColumn( 1, "Size",		LVCFMT_RIGHT, nColX );
	m_ctrlBankList.InsertColumn( 2, "Modified",	LVCFMT_LEFT, nColX * 2 );
		
	UpdateBankListBox( FALSE );

	// set some properties for our list box
	m_ctrlBankList.SetFullRowSel( TRUE );
	m_ctrlBankList.SortColumn( nBankSortCol, bBankAscSort );	
	m_ctrlBankList.EnableSeparatorLines( TRUE );

	////////////////////////
	// display the version #
	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );
	
	UpdateData( VARS_TO_CONTROLS );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWavBankerDlg::OnSysCommand( UINT nID, LPARAM lParam ) {

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

void CWavBankerDlg::OnPaint() 
{
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

HCURSOR CWavBankerDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

void CWavBankerDlg::OnChooseCurrentDir() {
	CString sNewDir;

	UpdateData( CONTROLS_TO_VARS );
	
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Wav Directory", m_sCurrentDir ) == PICKDIR_RET_SUCCESS ) {
		// store our new dir
		m_sCurrentDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );

		// prompt to save next time
		m_bPromptForSave = TRUE;

		// re-scan the new dir
		ScanForWavFiles();

		// setup the file list box
		UpdateFileListBox( TRUE );			
	}	
}

void CWavBankerDlg::OnSubDirsCheck() {
	
	UpdateData( CONTROLS_TO_VARS );

	// prompt to save next time
	m_bPromptForSave = TRUE;

	// re-scan the new dir
	ScanForWavFiles();
	
	// setup the file list box
	UpdateFileListBox( TRUE );
}

void CWavBankerDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );

	// writeout our settings before we exit
	CSettings& Settings = CSettings::GetCurrent();
	
	Settings.m_sLastWavDir = m_sCurrentDir;
	Settings.m_sLastBankFile = m_sCurrentBank;
	Settings.m_bRecurseSubDirs = m_bRecurseSubDirs;
	Settings.m_nFileSortColumn = m_ctrlFileList.GetSortedColumn();
	Settings.m_bFileAscendingSort = m_ctrlFileList.IsAscSorted();
	Settings.m_nBankSortColumn = m_ctrlBankList.GetSortedColumn();
	Settings.m_bBankAscendingSort = m_ctrlBankList.IsAscSorted();
	
	Settings.SaveCommonDataOutToFile();

	// see if we should ask the user to save the current wav bank
	PromptToSaveBank();
		
	::RemoveProp( GetSafeHwnd(), WavBanker_pszPropName );

	CDialog::OnCancel();
}

void CWavBankerDlg::OnOK() {
	//OnCancel();
}

BOOL CWavBankerDlg::PreTranslateMessage( MSG* pMsg ) {

	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the app, do nothing
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage( pMsg );
}

void CWavBankerDlg::OnChooseCurrentBank() {
	CString sOldBankFile;

	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".wvb",
					 m_sCurrentBank,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Wav Bank Files (*.wvb)|*.wvb||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Wav Bank File";
	if( dlg.DoModal() == IDOK ) {
		// store the old bank name
		sOldBankFile = m_sCurrentBank;

		if( sOldBankFile.CompareNoCase( dlg.GetPathName() ) == 0 ) {
			// we have chosen the same bank that is already selected, do we want to revert
			CString sError;
			sError.Format( "The current bank file is already:\n"
						   "%s\n"
						   "Do you want to reload the file and lose\n"
						   "any changes that you may have made?", sOldBankFile );
			if( MessageBox( sError, _T("WavBanker"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2) == IDNO ) {
				// don't reload the bank file and lose changes, do nothing
				return;	
			}
		} else {
			// we have selected a new bank, prompt to save the existing one
			PromptToSaveBank();
		}

		// save the new bank name
		m_sCurrentBank = dlg.GetPathName();
		UpdateData( VARS_TO_CONTROLS );

		// load the new bank
		LoadBankFile( TRUE );
		
		// setup the bank list box
		UpdateBankListBox( TRUE );
	}	
}

void CWavBankerDlg::ScanForWavFiles() {

	m_WavFiles.RemoveAll();
	
	if( m_sCurrentDir.IsEmpty() ) {
		return;
	}

	CString sRootDir = m_sCurrentDir;
	u32 nDirLen = sRootDir.GetLength();
	if( sRootDir.GetAt( nDirLen-1 ) != '\\' ) {
		sRootDir += '\\';
	}

	// gather file info on every wav file in sRootDir and below (NOT SORTED, TOO SLOW)
	m_WavFiles.AddDir( sRootDir, "*.wav", m_bRecurseSubDirs, 
					   CFileInfoArray::AP_NOSORT,
					   FALSE );

	// scan & remove any filename that are too long to be put into a wav bank
	int i, nNumFiles, nNumTooLong;
	CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData;
	CWaveFile WavFile;

	BeginWaitCursor();

	nNumTooLong = 0;
	pFileInfoData = m_WavFiles.GetData();
	nNumFiles = m_WavFiles.GetSize();
	for( i=0; i < nNumFiles;  ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];

		// test the length of the filename (not counting path or extension)
		if( pFileInfo->GetFileTitle().GetLength() >= WAV_BANK_MAX_WAV_FILENAME_LEN ) {
			// remove the ith entry
			m_WavFiles.RemoveAt( i, 1 );
			nNumTooLong++;
			nNumFiles--;
			continue;
		}
		
		// advance to the next entry
		i++;
	}

	EndWaitCursor();

	if( nNumTooLong ) {
		// tell the user how many files have been removed
		CString sError, sTemp;
		if( nNumTooLong ) {
			sTemp.Format( "Wav filenames must be 11.3 chars or less in length.\n"
						  "There were %d files that had names that exceeded this limit.\n\n", nNumTooLong );						  
			sError += sTemp;
		}

		sError += "The invalid files have been removed from the displayed wav list.\n"; 

		MessageBox( sError, _T("WavBanker: Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
	}
}

void CWavBankerDlg::UpdateFileListBox( BOOL bReSortList ) {
	int i, nNumFiles;
	u32 nDirLen;
	CString sFilename;
	CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData;
	CFileListItemInfo *pItemInfo;
	
	// empty the file list box
	m_ctrlFileList.DeleteAllItems();

	nDirLen = m_sCurrentDir.GetLength() + 1;

	// add all of the wav files in our list
	nNumFiles = m_WavFiles.GetSize();
	pFileInfoData = m_WavFiles.GetData();
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];
		
		sFilename = pFileInfo->GetFilePath();
		sFilename.Delete( 0, nDirLen );
		if( sFilename.Find( '\\' ) >= 0 ) {
			// since this filename has a path, add \ to the beginning of the name so that it sorts to the top
			sFilename.Insert( 0, '\\' );
		}

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

	m_sNumFilesShown.Format( "%d object(s)", nNumFiles );
			
	UpdateData( VARS_TO_CONTROLS );
}

void CWavBankerDlg::OnRefreshButton() {
	
	UpdateData( CONTROLS_TO_VARS );

	// re-scan the new dir
	ScanForWavFiles();

	// setup the file list box
	UpdateFileListBox( TRUE );
}

void CWavBankerDlg::OnRclickFileList( NMHDR* pNMHDR, LRESULT* pResult ) {
	
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

void CWavBankerDlg::OnDblclkFileList( NMHDR* pNMHDR, LRESULT* pResult ) {
	
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

void CWavBankerDlg::OnAddToBank() {
	
	if( m_sCurrentBank.IsEmpty() ) {
		MessageBox( _T("Before you can add files to a bank, you\nmust either create a new bank or load\nan existing one."), 
					_T("WavBanker"),
					MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return;
	}

	u32 nNumFileSelected = m_ctrlFileList.GetSelectedCount();
	if( !nNumFileSelected ) {
		return;
	}
	BeginWaitCursor();

	// walk the selected files
	int nIndex, nFilesAdded;
	POSITION Pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	CFileListItemInfo *pItemInfo;
	CFileInfo *pFileInfo;
	WavBank_Entry_t *pEntry;
	CString sMsg, sProblemFiles;

	sProblemFiles.Empty();
	nFilesAdded = 0;
	while( Pos ) {
		nIndex = m_ctrlFileList.GetNextSelectedItem( Pos );
		pItemInfo = (CFileListItemInfo *)m_ctrlFileList.GetItemData( nIndex );
		FASSERT( pItemInfo );	

		pFileInfo = pItemInfo->GetFileInfo();

		// see if this file already has an entry in the bank
		pEntry = m_WavBank.GetEntryFromFileInfo( pFileInfo );
		if( pEntry ) {
			// see if pEntry needs updating
			if( !m_WavBank.DoesFileMatchEntry( pFileInfo, pEntry ) ) {
				CTime FileTime = pFileInfo->GetLastWriteTime();
				CTime BankTime = (time_t)pEntry->nModifiedTime;

				sMsg.Format( "This bank already contains a file named\n"
							 "'%s'.\n\n"
							 "Would you like to replace the existing file\n\n"
							 "\t%.2f KB\n"
							 "\tModified: %s\n\n"
							 "with this one?\n\n"
							 "\t%.2f KB\n"
							 "\tModified: %s\n",
							 pFileInfo->GetFileName(),
							 pEntry->nNumBytes * (1.0f/1024.0f),
							 BankTime.Format( "%#x, %#I:%M:%S %p" ),
							 pFileInfo->GetLength() * (1.0f/1024.0f),
							 FileTime.Format( "%#x, %#I:%M:%S %p" ) );
				if( MessageBox( sMsg, 
								_T("WavBanker: Replace File?"), 
								MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1 ) == IDNO ) {
					// don't update the entry, move to the next selection
					continue;					
				}
			} else {
				// the files match exact, move to the next selection
				continue;
			}
		}
		// add pFileInfo (or update if it already exists)
		if( m_WavBank.AddEntry( pFileInfo ) ) {
			nFilesAdded++;
		} else {
			sProblemFiles += pFileInfo->GetFilePath();
			sProblemFiles += '\n';						
		}
	}

	if( !sProblemFiles.IsEmpty() ) {
		// we encountered problems, tell the user
		sMsg.Format( "Trouble adding the following files to the bank:\n"
					 "%s\n"
					 "The most likely cause is that the wav file are an invalid format.\n\n"
					 "Wav files must be:\n"
					 "   1 or 2 channels\n"
					 "   16-bit samples\n"
					 "   Uncompressed PCM Windows wav files.\n", sProblemFiles );
		MessageBox( sMsg, _T("WavBanker: Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
	}

	EndWaitCursor();

	if( nFilesAdded ) {
		m_bPromptForSave = TRUE;
		// setup the bank list box
		UpdateBankListBox( TRUE );	
	}
}

void CWavBankerDlg::OnRclickBankList( NMHDR* pNMHDR, LRESULT* pResult ) {

	UpdateData( CONTROLS_TO_VARS );

	// get the list box's screen rect
	CPoint Point;
	::GetCursorPos( &Point ); 
	m_ctrlBankList.ScreenToClient( &Point );
	
	// select the item under the mouse cursor
	int nIndex = m_ctrlBankList.HitTest( Point );
	if( nIndex >= 0 ) {
		m_ctrlBankList.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED );
	}
	// create the popup menu
	CMenu menu;
	menu.LoadMenu( IDR_BANK_POPUP_MENU );
	CMenu *pContextMenu = menu.GetSubMenu( 0 ); 
	m_ctrlBankList.ClientToScreen( &Point );

	// display the popup menu
	pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this );
	
	*pResult = 0;	
}

void CWavBankerDlg::OnSaveButton() {
	
	UpdateData( CONTROLS_TO_VARS );

	SaveBankFile();
}

void CWavBankerDlg::OnUpdateButton() {
	
	UpdateData( CONTROLS_TO_VARS );

	u32 nNumFileSelected = m_ctrlBankList.GetSelectedCount();
	if( !nNumFileSelected ) {
		return;
	}
	BeginWaitCursor();
	
	// walk the selected files
	int nIndex, nFilesUpdated, nNumWaves, i;
	POSITION Pos = m_ctrlBankList.GetFirstSelectedItemPosition();
	CBankListItemInfo *pItemInfo;
	WavBank_Entry_t *pEntry;
	CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData = m_WavFiles.GetData();;
	nNumWaves = m_WavFiles.GetSize();
	CTime ModTime;
	CString sMsg, sProblemFiles;
	
	sProblemFiles.Empty();
	nFilesUpdated = 0;	
	while( Pos ) {
		nIndex = m_ctrlBankList.GetNextSelectedItem( Pos );
		pItemInfo = (CBankListItemInfo *)m_ctrlBankList.GetItemData( nIndex );
		FASSERT( pItemInfo );	

		pEntry = pItemInfo->GetEntry();
		ModTime = (time_t)pEntry->nModifiedTime;

		// find a wav file with the same name
		for( i=0; i < nNumWaves; i++ ) {
			pFileInfo = (CFileInfo *)&pFileInfoData[i];
			if( pFileInfo->GetFileTitle().CompareNoCase( pEntry->szFilename ) == 0 ) {
				// the name matched, is the modified time newer than the bank entry
				if( pFileInfo->GetLastWriteTime() != ModTime ) {
					// update with this file
					break;
				}
			}
		}
		if( i < nNumWaves ) {
			// we found a file to update with
			if( m_WavBank.UpdateEntry( pFileInfo, pEntry ) ) {
				nFilesUpdated++;
			} else {
				sProblemFiles += pFileInfo->GetFilePath();
				sProblemFiles += '\n';				
			}
		}
	}

	if( !sProblemFiles.IsEmpty() ) {
		// we encountered problems, tell the user
		sMsg.Format( "Trouble updating the following files in the bank:\n"
					 "%s\n"
					 "The most likely cause is that the wav file are an invalid format.\n\n"
					 "Wav files must be:\n"
					 "   1 or 2 channels\n"
					 "   16-bit samples\n"
					 "   Uncompressed PCM Windows wav files.\n", sProblemFiles );
		MessageBox( sMsg, _T("WavBanker: Error"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
	}

	EndWaitCursor();

	if( nFilesUpdated ) {
		m_bPromptForSave = TRUE;
		// setup the bank list box
		UpdateBankListBox( TRUE );	
	}
}

void CWavBankerDlg::OnDeleteButton() {
		
	UpdateData( CONTROLS_TO_VARS );

	u32 nNumFileSelected = m_ctrlBankList.GetSelectedCount();
	if( !nNumFileSelected ) {
		return;
	}
	// walk the selected files
	int nIndex, nFilesDeleted;
	POSITION Pos = m_ctrlBankList.GetFirstSelectedItemPosition();
	CBankListItemInfo *pItemInfo;
	WavBank_Entry_t *pEntry;
	nFilesDeleted = 0;
	while( Pos ) {
		nIndex = m_ctrlBankList.GetNextSelectedItem( Pos );
		pItemInfo = (CBankListItemInfo *)m_ctrlBankList.GetItemData( nIndex );
		FASSERT( pItemInfo );	

		pEntry = pItemInfo->GetEntry();

		if( m_WavBank.DeleteEntry( pEntry ) ) {
			nFilesDeleted++;
		}
	}
	if( nFilesDeleted ) {
		m_bPromptForSave = TRUE;
		// setup the bank list box
		UpdateBankListBox( TRUE );	
	}
}

void CWavBankerDlg::OnCreateNewBank() {
	
	UpdateData( CONTROLS_TO_VARS );

	CString sDefaultName = m_sCurrentBank;
	if( !sDefaultName.IsEmpty() ) {
		int nIndex, nLettersToDelete;
		nIndex = sDefaultName.ReverseFind( '\\' );
		if( nIndex >= 0 ) {
			nLettersToDelete = sDefaultName.GetLength() - (nIndex + 1);
			if( nLettersToDelete > 0 ) {
				sDefaultName.Delete( nIndex + 1, nLettersToDelete );
			}
		}
	}
	sDefaultName += "new_bank.wvb";

	CFileDialog dlg( FALSE,
					 ".wvb",
					 "new_bank.wvb",
					 OFN_EXTENSIONDIFFERENT | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
					 "Wav Bank Files (*.wvb)|*.wvb||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Wav Bank File To Create";
	if( dlg.DoModal() == IDOK ) {
		// make sure that the new name is ok
		if( dlg.GetFileTitle().GetLength() > 11 ) {
			MessageBox( _T("Bank names must be 11 or fewer characters.\nPlease select a new bank name."), 
						_T("WavBanker"),
						MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
			return;
		}
		// make sure that the user picked the correct extension
		if( dlg.GetFileExt().CompareNoCase( "wvb" ) != 0 ) {
			MessageBox( _T("Wav banks must have the following file\nextension:\n*.wvb\nPlease select a new filename."), 
						_T("WavBanker"),
						MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			return;
		}
		PromptToSaveBank();

		// save the new bank name
		m_sCurrentBank = dlg.GetPathName();
		UpdateData( VARS_TO_CONTROLS );
		
		// create an empty file
		m_WavBank.CreateEmpty();

		// setup the bank list box
		UpdateBankListBox( TRUE );
	}
}

void CWavBankerDlg::PromptToSaveBank() {

	if( m_sCurrentBank.IsEmpty() ) {
		return;
	}
	if( !m_bPromptForSave ) {
		return;
	}
	CString sMsg;
	sMsg.Format( "Save %s?", m_sCurrentBank );
	if( MessageBox( sMsg, _T("WavBanker"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON1 ) == IDYES ) {
		SaveBankFile();
	}
}

void CWavBankerDlg::SaveBankFile() {

	if( m_sCurrentBank.IsEmpty() ) {
		return;
	}
	// capture the settings
	WavBank_ToolSettings_t Settings;
	Settings.bRecurseSubDirs = m_bRecurseSubDirs;
	Settings.bBankAscendingSort = m_ctrlBankList.IsAscSorted();
	Settings.bFileAscendingSort = m_ctrlFileList.IsAscSorted();
	Settings.nBankSortColumn = m_ctrlBankList.GetSortedColumn();
	Settings.nFileSortColumn = m_ctrlFileList.GetSortedColumn();
	fclib_strncpy( Settings.szLastDirUsed, m_sCurrentDir, WAV_BANK_STRING_LEN );
	
	// update the bank's settings
	m_WavBank.SetFileSettings( &Settings );

	// write out the bank file
	if( !m_WavBank.Write( m_sCurrentBank ) ) {
		MessageBox( _T("Error writing the bank file to disk."), 
					_T("WavBanker"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
	} else {
		m_bPromptForSave = FALSE;
	}
}

void CWavBankerDlg::LoadBankFile( BOOL bUseBankToolSettings ) {
	
	m_WavBank.CreateEmpty();

	m_bPromptForSave = FALSE;

	if( m_sCurrentBank.IsEmpty() ) {
		return;
	}
	if( !m_WavBank.Read( m_sCurrentBank ) ) {
		CString sError;
		sError.Format( "Error reading\n%s\nfrom disk.\nStarting a new file.", m_sCurrentBank );
		MessageBox( sError, 
					_T("WavBanker"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		bUseBankToolSettings = FALSE;
		m_WavBank.CreateEmpty();
		m_bPromptForSave = TRUE;
	}

	if( bUseBankToolSettings ) {
		WavBank_ToolSettings_t *pSettings = m_WavBank.GetFileSettings();
		m_sCurrentDir = pSettings->szLastDirUsed;
		m_bRecurseSubDirs = pSettings->bRecurseSubDirs;
		m_ctrlBankList.SortColumn( pSettings->nBankSortColumn, pSettings->bBankAscendingSort );
		m_ctrlFileList.SortColumn( pSettings->nFileSortColumn, pSettings->bFileAscendingSort );
		
		UpdateData( VARS_TO_CONTROLS );

		// rescan the new directories for wav files
		ScanForWavFiles();
		UpdateFileListBox( TRUE );
	}
}

void CWavBankerDlg::UpdateBankListBox( BOOL bReSortList ) {
	u32 i, nNumEntries;
	CBankListItemInfo *pItemInfo;
	WavBank_Entry_t *pEntry;
	
	// empty the bank list box
	m_ctrlBankList.DeleteAllItems();

	// add all of the entries to our list
	nNumEntries = m_WavBank.GetNumEntries();
	for( i=0; i < nNumEntries; i++ ) {
		pEntry = m_WavBank.GetEntryByIndex( i );

		// add a new item to the list box control
		m_ctrlBankList.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CBankListItemInfo( i, pEntry );
		FASSERT( pItemInfo );
		m_ctrlBankList.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlBankList.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlBankList.SetItemText( i, 2, LPSTR_TEXTCALLBACK );		
	}

	// resort the file list
	if( bReSortList ) {
		m_ctrlBankList.ReSort();
	}

	m_sNumBankFilesShown.Format( "%d object(s)", nNumEntries );
	m_sBankSize.Format( "bank size: %.02f KB", m_WavBank.GetBankSize() * (1.0f/1024.0f) );
			
	UpdateData( VARS_TO_CONTROLS );	
}

void CWavBankerDlg::OnRemoveAllEntries() {
	
	UpdateData( CONTROLS_TO_VARS );

	int i, nFilesDeleted, nItemCount;
	nItemCount = m_ctrlBankList.GetItemCount();
	CBankListItemInfo *pItemInfo;
	WavBank_Entry_t *pEntry;
	nFilesDeleted = 0;
	for( i=0; i < nItemCount; i++ ) {
		pItemInfo = (CBankListItemInfo *)m_ctrlBankList.GetItemData( i );
		FASSERT( pItemInfo );

		pEntry = pItemInfo->GetEntry();

		if( m_WavBank.DeleteEntry( pEntry ) ) {
			nFilesDeleted++;
		}
	}
	if( nFilesDeleted ) {
		m_bPromptForSave = TRUE;
		// setup the bank list box
		UpdateBankListBox( TRUE );	
	}
}

void CWavBankerDlg::OnViewFileStats() {
	
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
