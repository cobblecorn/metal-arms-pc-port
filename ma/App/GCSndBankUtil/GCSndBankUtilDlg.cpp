// GCSndBankUtilDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fang.h"
#include "GCSndBankUtil.h"
#include "GCSndBankUtilDlg.h"
#include "fversion.h"
#include "pickdir.h"
#include "Settings.h"
#include "ParseHeader.h"
#include "WavBank.h"
#include "MusyXData.h"
#include "GCBank.h"
#include <io.h>

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
// CGCSndBankUtilDlg dialog

CGCSndBankUtilDlg::CGCSndBankUtilDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGCSndBankUtilDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGCSndBankUtilDlg)
	m_sVersion = _T("");
	m_sOutputDir = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGCSndBankUtilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGCSndBankUtilDlg)
	DDX_Control(pDX, IDC_BANK_FILE_LIST, m_ctrlBankFiles);
	DDX_Control(pDX, IDC_MUSYX_FILE_LIST, m_ctrlMusyXFiles);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Text(pDX, IDC_OUTPUT_DIR, m_sOutputDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGCSndBankUtilDlg, CDialog)
	//{{AFX_MSG_MAP(CGCSndBankUtilDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_CONVERT_BUTTON, OnConvertButton)
	ON_BN_CLICKED(IDC_CHOOSE_OUTPUT_DIR, OnChooseOutputDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCSndBankUtilDlg message handlers

BOOL CGCSndBankUtilDlg::OnInitDialog()
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

	::SetProp( GetSafeHwnd(), GCSndBankUtil_pszPropName, (HANDLE)1 );

	////////////////////////////
	// SET OUR SETTINGS FILENAME
	CSettings::SetSettingsFilename( "gc_snd_bank_util.ini" ); 
	
	////////////////////////////////////////////////
	// GET THE DEFAULT VALUES FROM OUR SETTINGS FILE
	CSettings& Settings = CSettings::GetCurrent();
	
	m_sOutputDir = Settings.GetOutputDir();
	BOOL bBankAscSort = Settings.GetBankAscendingSort();
	u32 nBankSortCol = Settings.GetBankSortColumn();
	BOOL bMusyXAscSort = Settings.GetMusyXAscendingSort();
	u32 nMusyXSortCol = Settings.GetMusyXSortColumn();

	/////////////////////////////////////////
	// SETUP THE BANK FILE VIEW LIST CONTROL
	CRect Rect;
	m_ctrlBankFiles.GetClientRect( &Rect );
	int nColLen = Rect.Width() - 16;
	int nColX = nColLen / 7;// subtract 16 to account for the right scroll bar width
	m_ctrlBankFiles.InsertColumn( 0, "Name",		LVCFMT_LEFT, nColX * 4 );
	m_ctrlBankFiles.InsertColumn( 1, "Size",		LVCFMT_RIGHT, nColX );
	m_ctrlBankFiles.InsertColumn( 2, "Modified",	LVCFMT_LEFT, nColX * 2 );
		
	// set some properties for our list box
	m_ctrlBankFiles.SetFullRowSel( TRUE );
	m_ctrlBankFiles.SortColumn( nBankSortCol, bBankAscSort );	
	m_ctrlBankFiles.EnableSeparatorLines( TRUE );

	/////////////////////////////////////////
	// SETUP THE MUSYX FILE VIEW LIST CONTROL
	m_ctrlMusyXFiles.GetClientRect( &Rect );
	nColLen = Rect.Width() - 16;
	nColX = nColLen / 7;// subtract 16 to account for the right scroll bar width
	m_ctrlMusyXFiles.InsertColumn( 0, "Name",		LVCFMT_LEFT, nColX * 4 );
	m_ctrlMusyXFiles.InsertColumn( 1, "Size",		LVCFMT_RIGHT, nColX );
	m_ctrlMusyXFiles.InsertColumn( 2, "Modified",	LVCFMT_LEFT, nColX * 2 );
		
	// set some properties for our list box
	m_ctrlMusyXFiles.SetFullRowSel( TRUE );
	m_ctrlMusyXFiles.SortColumn( nMusyXSortCol, bMusyXAscSort );	
	m_ctrlMusyXFiles.EnableSeparatorLines( TRUE );	

	////////////////////////
	// display the version #
	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );
	
	UpdateData( VARS_TO_CONTROLS );

	fang_Init();

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGCSndBankUtilDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGCSndBankUtilDlg::OnPaint() 
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

HCURSOR CGCSndBankUtilDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CGCSndBankUtilDlg::OnOK() {
	//CDialog::OnOK();
}

void CGCSndBankUtilDlg::OnCancel() {

	UpdateData( CONTROLS_TO_VARS );

	// writeout our settings before we exit
	CSettings& Settings = CSettings::GetCurrent();
	
	Settings.m_sOutputDir = m_sOutputDir;
	Settings.m_nBankSortColumn = m_ctrlBankFiles.GetSortedColumn();
	Settings.m_bBankAscendingSort = m_ctrlBankFiles.IsAscSorted();
	Settings.m_nMusyXSortColumn = m_ctrlMusyXFiles.GetSortedColumn();
	Settings.m_bMusyXAscendingSort = m_ctrlMusyXFiles.IsAscSorted();
		
	Settings.SaveCommonDataOutToFile();

	::RemoveProp( GetSafeHwnd(), GCSndBankUtil_pszPropName );
		
	CDialog::OnCancel();
}

void CGCSndBankUtilDlg::OnConvertButton() {
	CString sMsg;
	CTime OldestMusyXFile, NewestBankFile;
	int i, nNumFiles, j, nNumEntries;
	CFileInfo *pFileInfo, NewSampFile;
	const CFileInfo *pFileInfoData;
	CString sMissingFiles, sFilename;
	WavBank_Entry_t *pEntry;
	CMusyXData MusyXData;
	CGCBank GCBank;
	u32 nBytesToPad;
	FILE *pFileStream;

	UpdateData( CONTROLS_TO_VARS );

	// MAKE SURE THAT ALL FILE REQUIREMENTS ARE MET
	if( m_sOutputDir.IsEmpty() ||
		m_ctrlBankFiles.GetItemCount() < 1 ||
		m_ctrlMusyXFiles.GetItemCount() != 5 ) {
		// the min file requirements are not met
		sMsg = "You must provide all of the following before you can\n"
			   "convert Xbox banks into GameCube banks.\n"
			   "\n   From the MusyX Sound Tool:\n"
			   "\t1 .proj\n"
			   "\t1 .samp\n"
			   "\t1 .h\n"
			   "\t1 .sdir\n"
			   "\t1 .pool\n"
			   "\n   From the WavBanker Tool:\n"
			   "\tAll of the .wvb files created for the Xbox\n"
			   "\n   And an output directory to put converted files.\n\n";
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return;
	}

	// MAKE SURE THAT THE MUSYX FILES ARE NEWER THAN ALL OF THE BANKS
	GetOldestMusyXFile( OldestMusyXFile );
	GetNewestBankFile( NewestBankFile );
	if( NewestBankFile > OldestMusyXFile ) {
		sMsg = "The newest Xbox bank file is newer than\n"
			   "the oldest MusyX generated file.\n\n"
			   "You must create all of your Xbox banks\n"
			   "before running the MusyX Sound Tools.\n\n"
			   "Please update and regenerate the MusyX sound files.\n";
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return;
	}

	// PARSE THE MUSYX HEADER FILE AND VERIFY THAT THERE ARE THE SAME # OF BANKS AS MUSYX GROUPS
	CParseHeader Header;
	if( !Header.Parse( m_HFile.GetFilePath() ) ||
		Header.m_nNumGRPs != (u32)m_ctrlBankFiles.GetItemCount() ) {
		sMsg.Format( "There were %d MusyX groups found and %d bank files\n"
					 "passed in.\n"
					 "There should be the same number of MusyX groups as\n"
					 "there are Xbox bank files.\n"
					 "Please check your data and try again.\n", Header.m_nNumGRPs, m_ctrlBankFiles.GetItemCount() );
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return;
	}

	// COMPARE THE GROUP NAMES AND BANK NAMES, THEY MUST MATCH
	nNumFiles = m_BankFiles.GetSize();
	pFileInfoData = m_BankFiles.GetData();
	sMissingFiles.Empty();
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];

		if( Header.FindGroupIndex( pFileInfo->GetFileTitle() ) == -1 ) {
			// we have a bank that doesn't exist as a MusyX group
			sMissingFiles += "\t";
			sMissingFiles += pFileInfo->GetFileTitle();
			sMissingFiles += "\n";			
		}
	}
	if( !sMissingFiles.IsEmpty() ) {
		sMsg = "The following MusyX groups could not be found:\n\n";
		sMsg += sMissingFiles;
		sMsg += "\n";
		sMsg += "These groups must exist before conversion is possible.\n";
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return;
	}

	// READ ALL OF THE BANK HEADERS
	CWavBank *paWavBanks;
	paWavBanks = new CWavBank[nNumFiles];
	if( !paWavBanks ) {
		sMsg = "Could not read in all of the wav banks.\n";
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		return;
	}
	sMissingFiles.Empty();
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];
		
		if( !paWavBanks[i].ReadEntriesOnly( pFileInfo->GetFilePath() ) ) {
			sMsg = "Could not read in all of the wav banks.\n";
			MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			delete [] paWavBanks;
			return;
		}
		// go ahead and check for the sound ids in the MusyX file, record any missing ones
		nNumEntries = paWavBanks[i].GetNumEntries();
		for( j=0; j < nNumEntries; j++ ) {
			pEntry = paWavBanks[i].GetEntryByIndex( j );		
			FASSERT( pEntry );

			if( Header.FindSfxIndex( pEntry->szFilename ) == -1 ) {
				// we have a bank that doesn't exist as a MusyX group
				sMissingFiles += "\t";
				sMissingFiles += pEntry->szFilename;
				sMissingFiles += "\n";			
			}
		}
	}
	// if we were missing wav files report it now
	if( !sMissingFiles.IsEmpty() ) {
		sMsg = "The following MusyX sample files could not be found:\n\n";
		sMsg += sMissingFiles;
		sMsg += "\n";
		sMsg += "These sample files are used in the bank files and\nmust exist before conversion is possible.\n";
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		delete [] paWavBanks;
		return;
	}

	// COPY AND RENAME THE .SAMP FILE
	sFilename.Format( "%s\\snd_smpls.rdg", m_sOutputDir );
	if( !CopyFile( m_SampFile.GetFilePath(), sFilename, FALSE ) ) {
		sMsg.Format( "Error copying file.\n"
					 "From:\n"
					 "%s\n"
					 "To:\n"
					 "%s\n", m_SampFile.GetFilePath(), sFilename );
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		delete [] paWavBanks;
		return;
	}
	// ensure that the sample filesize is a multiple of 32
	NewSampFile.Create( sFilename );
	nBytesToPad = (NewSampFile.GetLength() & 0x1F);
	if( nBytesToPad ) {
		// the filesize is not a multiple of 32, pad the end of the file with 0
		pFileStream = fopen( sFilename, "a+b" );
		if( !pFileStream ) {
			sMsg.Format( "Error padding file:\n"
						 "%s\n"
						 "to be a multiple of 32 bytes.\n", sFilename );
			MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			delete [] paWavBanks;
			::DeleteFile( sFilename );
			return;
		}
		// write out nBytesToPad 0s
		nBytesToPad = 32 - nBytesToPad;
		i = _fileno( pFileStream );
		if( _chsize( i, NewSampFile.GetLength() + nBytesToPad ) == -1 ) {
			sMsg.Format( "Error padding file:\n"
						 "%s\n"
						 "to be a multiple of 32 bytes.\n", sFilename );
			MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			fclose( pFileStream );
			delete [] paWavBanks;
			::DeleteFile( sFilename );
			return;	
		}
		fclose( pFileStream );
	}
  
	// CREATE THE MUSYX DATA FILE
	sFilename.Format( "%s\\snd_init.rdg", m_sOutputDir );
	if( !MusyXData.CreateMusyXDataFile( &m_ProjFile, &m_SDirFile, &m_PoolFile, sFilename ) ) {
		sMsg.Format( "Error creating the MusyX data file:\n"
					 "%s\n", sFilename );
		MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		delete [] paWavBanks;
		return;
	}

	// CREATE THE BANK FILES
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];

		sFilename.Format( "%s\\%s.rdg", m_sOutputDir, pFileInfo->GetFileTitle() );
		if( !GCBank.CreateGCBank( paWavBanks[i],
								  Header,
								  pFileInfo->GetFileTitle(),
								  sFilename ) ) {
			sMsg.Format( "Error creating the GC bank file:\n"
						 "%s\n", sFilename );
			MessageBox( sMsg, _T("GC Sound Bank Utility"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			delete [] paWavBanks;
			return;
		}
	}

	// SUCCESS!!!
	delete [] paWavBanks;
}

void CGCSndBankUtilDlg::OnChooseOutputDir() {
	CString sNewDir;

	UpdateData( CONTROLS_TO_VARS );
	
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Output Directory", m_sOutputDir ) == PICKDIR_RET_SUCCESS ) {
		// store our new dir
		m_sOutputDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );
	}	
}

void CGCSndBankUtilDlg::OnDropFiles( HDROP hdrop ) {

	u32 nNumFiles, i, nNumFileInfos, j;
	TCHAR szNextFile [MAX_PATH];
	CString sFilename, sExt;
	int nIndex;
	u32 nBanksAdded = 0, nMusyXAdded = 0;
	const CFileInfo *pFileInfoData;
	CFileInfo *pFileInfo1, *pFileInfo2;
	
    // Get the # of files being dropped.
    nNumFiles = DragQueryFile( hdrop, -1, NULL, 0 );
    for( i=0; i < nNumFiles; i++ ) {
		 // Get the next filename from the HDROP info.
        if( DragQueryFile( hdrop, i, szNextFile, MAX_PATH ) > 0 ) {
			sFilename = szNextFile;
			sFilename.MakeLower();
			if( IsStringAFilename( sFilename, sExt ) ) {
				// sFilename is a file, find out what type

				if( sExt.CompareNoCase( ".wvb" ) == 0 ) {
					// most likely this is a wav bank file
					nIndex = m_BankFiles.AddFile( szNextFile, CFileInfoArray::AP_NOSORT );
					if( nIndex >= 0 ) {
						// a file was added
						nBanksAdded++;
						
						// make sure we haven't dropped a duplicate file
						if( nIndex > 1 ) {
							nNumFileInfos = m_BankFiles.GetSize() - 1;
							pFileInfoData = m_BankFiles.GetData();
							pFileInfo1 = (CFileInfo *)&pFileInfoData[nIndex];
							sFilename = pFileInfo1->GetFileName();
							for( j=0; j < nNumFileInfos; j++ ) {
								pFileInfo2 = (CFileInfo *)&pFileInfoData[j];

								if( pFileInfo2->GetFileName().CompareNoCase( sFilename ) == 0 ) {
									// we already have a file with this name, don't allow duplicates
									m_BankFiles.RemoveAt( nIndex, 1 );
									nBanksAdded--;
									break;
								}
							}
						}
					}
				} else if( sExt.CompareNoCase( ".proj" ) == 0 ) {
					m_ProjFile.Create( sFilename );
					nMusyXAdded++;
				} else if( sExt.CompareNoCase( ".h" ) == 0 ) {
					m_HFile.Create( sFilename );
					nMusyXAdded++;
				} else if( sExt.CompareNoCase( ".pool" ) == 0 ) {
					m_PoolFile.Create( sFilename );
					nMusyXAdded++;
				} else if( sExt.CompareNoCase( ".sdir" ) == 0 ) {
					m_SDirFile.Create( sFilename );
					nMusyXAdded++;
				} else if( sExt.CompareNoCase( ".samp" ) == 0 ) {
					m_SampFile.Create( sFilename );
					nMusyXAdded++;
				}
			} else {
				// we have a directory
				m_sOutputDir = sFilename;
				UpdateData( VARS_TO_CONTROLS );
			}
        }
    }
    // Free up memory.
    DragFinish( hdrop );

	if( nBanksAdded ) {
		UpdateBankListBox( TRUE );		
	}
	if( nMusyXAdded ) {
		UpdateMusyXListBox( TRUE );
	}
}

BOOL CGCSndBankUtilDlg::IsStringAFilename( const CString &rsFilename, CString &rsExt ) {
	CFileInfo FileInfo;

	FileInfo.Create( rsFilename );
	if( FileInfo.IsDirectory() ) {
		return FALSE;
	}
	rsExt = FileInfo.GetFileExt();

	return TRUE;
}

void CGCSndBankUtilDlg::UpdateBankListBox( BOOL bReSortList ) {
	int i, nNumFiles;
	CString sFilename;
	CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData;
	CFileListItemInfo *pItemInfo;
	
	// empty the bank file list box
	m_ctrlBankFiles.DeleteAllItems();

	// add all of the bank files in our list
	nNumFiles = m_BankFiles.GetSize();
	pFileInfoData = m_BankFiles.GetData();
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];
		
		sFilename = pFileInfo->GetFilePath();
		
		// add a new item to the list box control
		m_ctrlBankFiles.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CFileListItemInfo( i, sFilename, pFileInfo );
		FASSERT( pItemInfo );
		m_ctrlBankFiles.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlBankFiles.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlBankFiles.SetItemText( i, 2, LPSTR_TEXTCALLBACK );		
	}

	// resort the file list
	if( bReSortList ) {
		m_ctrlBankFiles.ReSort();
	}
			
	UpdateData( VARS_TO_CONTROLS );
}

void CGCSndBankUtilDlg::UpdateMusyXListBox( BOOL bReSortList ) {
	int i = 0;
	CString sFilename;
	CFileListItemInfo *pItemInfo;
	
	// empty the bank file list box
	m_ctrlMusyXFiles.DeleteAllItems();

	sFilename = m_ProjFile.GetFilePath();
	if( !sFilename.IsEmpty() ) {
		// add a new item to the list box control
		m_ctrlMusyXFiles.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CFileListItemInfo( i, sFilename, &m_ProjFile );
		FASSERT( pItemInfo );
		m_ctrlMusyXFiles.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlMusyXFiles.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlMusyXFiles.SetItemText( i, 2, LPSTR_TEXTCALLBACK );
		i++;
	}

	sFilename = m_SampFile.GetFilePath();
	if( !sFilename.IsEmpty() ) {
		// add a new item to the list box control
		m_ctrlMusyXFiles.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CFileListItemInfo( i, sFilename, &m_SampFile );
		FASSERT( pItemInfo );
		m_ctrlMusyXFiles.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlMusyXFiles.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlMusyXFiles.SetItemText( i, 2, LPSTR_TEXTCALLBACK );
		i++;
	}

	sFilename = m_HFile.GetFilePath();
	if( !sFilename.IsEmpty() ) {
		// add a new item to the list box control
		m_ctrlMusyXFiles.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CFileListItemInfo( i, sFilename, &m_HFile );
		FASSERT( pItemInfo );
		m_ctrlMusyXFiles.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlMusyXFiles.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlMusyXFiles.SetItemText( i, 2, LPSTR_TEXTCALLBACK );
		i++;
	}

	sFilename = m_PoolFile.GetFilePath();
	if( !sFilename.IsEmpty() ) {
		// add a new item to the list box control
		m_ctrlMusyXFiles.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CFileListItemInfo( i, sFilename, &m_PoolFile );
		FASSERT( pItemInfo );
		m_ctrlMusyXFiles.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlMusyXFiles.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlMusyXFiles.SetItemText( i, 2, LPSTR_TEXTCALLBACK );
		i++;
	}

	sFilename = m_SDirFile.GetFilePath();
	if( !sFilename.IsEmpty() ) {
		// add a new item to the list box control
		m_ctrlMusyXFiles.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CFileListItemInfo( i, sFilename, &m_SDirFile );
		FASSERT( pItemInfo );
		m_ctrlMusyXFiles.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlMusyXFiles.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlMusyXFiles.SetItemText( i, 2, LPSTR_TEXTCALLBACK );
		i++;
	}

	// resort the file list
	if( bReSortList ) {
		m_ctrlMusyXFiles.ReSort();
	}
			
	UpdateData( VARS_TO_CONTROLS );	
}

void CGCSndBankUtilDlg::GetOldestMusyXFile( CTime &rOldestTime ) {
	CTime Time;

	rOldestTime = m_ProjFile.GetLastWriteTime();

	Time = m_SampFile.GetLastWriteTime();
	if( Time < rOldestTime ) {
		rOldestTime = Time;
	}
	
	Time = m_HFile.GetLastWriteTime();
	if( Time < rOldestTime ) {
		rOldestTime = Time;
	}
	
	Time = m_PoolFile.GetLastWriteTime();
	if( Time < rOldestTime ) {
		rOldestTime = Time;
	}
	
	Time = m_SDirFile.GetLastWriteTime();
	if( Time < rOldestTime ) {
		rOldestTime = Time;
	}
}

void CGCSndBankUtilDlg::GetNewestBankFile( CTime &rNewestTime ) {
	int i, nNumFiles;
	const CFileInfo *pFileInfoData;
	CFileInfo *pFileInfo;
	CTime Time;
		
	nNumFiles = m_BankFiles.GetSize();
	pFileInfoData = m_BankFiles.GetData();
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];
		if( i > 0 ) {
			Time = pFileInfo->GetLastWriteTime();
			if( Time > rNewestTime ) {
				rNewestTime = Time;
			}
		} else {
			rNewestTime = pFileInfo->GetLastWriteTime();
		}
	}
}