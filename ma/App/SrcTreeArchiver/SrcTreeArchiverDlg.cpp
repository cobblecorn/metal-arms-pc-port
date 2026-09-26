// SrcTreeArchiverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SrcTreeArchiver.h"
#include "SrcTreeArchiverDlg.h"
#include "Settings.h"
#include "pickdir.h"
#include "FileInfo.h"
#include "ZipArchive.h"

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
// CSrcTreeArchiverDlg dialog

CSrcTreeArchiverDlg::CSrcTreeArchiverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSrcTreeArchiverDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSrcTreeArchiverDlg)
	m_sIgnoreExtList = _T("");
	m_sOutputFilename = _T("");
	m_sProgressText = _T("");
	m_sSrcTreeDir = _T("");
	m_bRecurseSubDirs = FALSE;
	m_bClickNGo = FALSE;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nCurrentProfile = 0;
}

void CSrcTreeArchiverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSrcTreeArchiverDlg)
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_ctrlDatePicker);
	DDX_Control(pDX, IDC_SPAN_COMBO, m_ctrlSpanDisks);
	DDX_Control(pDX, IDC_PROFILE_COMBO, m_ctrlProfile);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ctrlProgressBar);
	DDX_Text(pDX, IDC_IGNORE_EXT, m_sIgnoreExtList);
	DDX_Text(pDX, IDC_OUTPUT_FILENAME, m_sOutputFilename);
	DDX_Text(pDX, IDC_PROGRESS_TEXT, m_sProgressText);
	DDX_Text(pDX, IDC_SOURCE_TREE, m_sSrcTreeDir);
	DDX_Check(pDX, IDC_SUB_DIRS_CHECK, m_bRecurseSubDirs);
	DDX_Check(pDX, IDC_CLICK_N_GO, m_bClickNGo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSrcTreeArchiverDlg, CDialog)
	//{{AFX_MSG_MAP(CSrcTreeArchiverDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PICK_SOURCE_TREE, OnButtonPickSourceTree)
	ON_BN_CLICKED(IDC_RUN_BUTTON, OnRunButton)
	ON_CBN_SELCHANGE(IDC_PROFILE_COMBO, OnSelchangeProfileCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrcTreeArchiverDlg message handlers

BOOL CSrcTreeArchiverDlg::OnInitDialog()
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
	::SetProp( GetSafeHwnd(), SrcTreeArchiver_pszPropName, (HANDLE)1 );

	CSettings::SetSettingsFilename( "src_tree_archiver.ini" );
	CSettings::SetApplicationName( "src_tree_archiver" );

	// get the default values from our settings file
	CSettings& Settings = CSettings::GetCurrent();

	m_nCurrentProfile = Settings.GetProfileIndex();
	m_sIgnoreExtList = Settings.GetIgnoreExtList( m_nCurrentProfile );
	m_sOutputFilename = Settings.GetDestFilename( m_nCurrentProfile );
	m_sSrcTreeDir = Settings.GetSrcPath( m_nCurrentProfile );
	m_bRecurseSubDirs = Settings.GetRecurseSubDirs( m_nCurrentProfile );
	m_bClickNGo = Settings.GetClickNGo();

	m_sProgressText.Empty();
	m_ctrlProgressBar.SetRange( 0, 1 );
	m_ctrlProgressBar.SetPos( 0 );

	int nIndex;
	m_ctrlSpanDisks.ResetContent();
	// no disk spanning
	nIndex = m_ctrlSpanDisks.AddString( "No Disk Spanning" );
	m_ctrlSpanDisks.SetItemData( nIndex, 0 );
	// 1.44MB disks
	nIndex = m_ctrlSpanDisks.AddString( "1.44 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, 1440000 );
	// 10MB disks
	nIndex = m_ctrlSpanDisks.AddString( "10 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (10 * 1024 * 1024) );
	// 25MB disks
	nIndex = m_ctrlSpanDisks.AddString( "25 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (25 * 1024 * 1024) );
	// 50MB disks
	nIndex = m_ctrlSpanDisks.AddString( "50 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (50 * 1024 * 1024) );
	// 100MB disks
	nIndex = m_ctrlSpanDisks.AddString( "100 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (93 * 1024 * 1024) );
	// 200MB disks
	nIndex = m_ctrlSpanDisks.AddString( "200 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (190 * 1024 * 1024) );
	// 250MB disks
	nIndex = m_ctrlSpanDisks.AddString( "250 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (230 * 1024 * 1024) );
	// 600MB disks
	nIndex = m_ctrlSpanDisks.AddString( "600 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (590 * 1024 * 1024) );
	// 650MB disks
	nIndex = m_ctrlSpanDisks.AddString( "650 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (640 * 1024 * 1024) );
	// 700MB disks
	nIndex = m_ctrlSpanDisks.AddString( "700 MB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (690 * 1024 * 1024) );
	// 1GB disks
	nIndex = m_ctrlSpanDisks.AddString( "1 GB Disks" );
	m_ctrlSpanDisks.SetItemData( nIndex, (990 * 1024 * 1024) );

	m_ctrlSpanDisks.SetCurSel( Settings.GetDiskSpanIndex( m_nCurrentProfile ) );

	m_ctrlProfile.ResetContent();
	CString sItem;
	u32 i;
	for( i=0; i < SETTINGS_MAX_PROFILES; i++ ) {
		sItem.Format( "Profile #%d", i+1 );
		m_ctrlProfile.AddString( sItem );
	}
	m_ctrlProfile.SetCurSel( m_nCurrentProfile );

	CTime Date = Settings.GetDate( m_nCurrentProfile );
	if( (u32)Date.GetTime() != 0 ) {
		m_ctrlDatePicker.SetTime( &Date );
	} else {
		m_ctrlDatePicker.SetTime();
	}

	UpdateData( VARS_TO_CONTROLS );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSrcTreeArchiverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSrcTreeArchiverDlg::OnPaint() 
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

HCURSOR CSrcTreeArchiverDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSrcTreeArchiverDlg::OnButtonPickSourceTree() {
	UpdateData( CONTROLS_TO_VARS );
	CString sNewDir;
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select Source Tree Directory", m_sSrcTreeDir ) == PICKDIR_RET_SUCCESS ) {
		m_sSrcTreeDir = sNewDir;
		UpdateData( VARS_TO_CONTROLS );
	}
}

void CSrcTreeArchiverDlg::OnRunButton() {
	UpdateData( CONTROLS_TO_VARS );

	CTime Date;

	// save our current settings, just in case we crash
	SaveCurrentProfile();

	if( !m_bClickNGo ) {
		u32 nBytesPerDisk = m_ctrlSpanDisks.GetItemData( m_ctrlSpanDisks.GetCurSel() );
		BOOL bUseDate = (m_ctrlDatePicker.GetTime( Date ) != GDT_NONE) ? TRUE : FALSE;

		CreateZipFile( FALSE, m_sIgnoreExtList, m_sOutputFilename, m_sSrcTreeDir, m_bRecurseSubDirs, Date, nBytesPerDisk, bUseDate );	
	} else {
		u32 i, nBytesPerDisk;
		CString sIgnoreExtList, sOutputFilename, sSrcTreeDir;
		BOOL bRecurseSubDirs, bUseDate;

		CSettings& Settings = CSettings::GetCurrent();

		for( i=0; i < SETTINGS_MAX_PROFILES; i++ ) {
			sIgnoreExtList = Settings.GetIgnoreExtList( i );
			sOutputFilename = Settings.GetDestFilename( i );
			sSrcTreeDir = Settings.GetSrcPath( i );
			bRecurseSubDirs = Settings.GetRecurseSubDirs( i );
			Date = Settings.GetDate( i );
			bUseDate = ( (u32)Date.GetTime() == 0 ) ? FALSE : TRUE;
			nBytesPerDisk = m_ctrlSpanDisks.GetItemData( Settings.GetDiskSpanIndex( i ) );

			CreateZipFile( TRUE, sIgnoreExtList, sOutputFilename, sSrcTreeDir, bRecurseSubDirs, Date, nBytesPerDisk, bUseDate );
			
			UpdateData( CONTROLS_TO_VARS );
		}
	}
}

void CSrcTreeArchiverDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );

	// writeout our settings before we exit
	CSettings& Settings = CSettings::GetCurrent();

	Settings.m_nProfileIndex = m_nCurrentProfile;
	Settings.m_bClickNGo = m_bClickNGo;
	SaveCurrentProfile();	

	::RemoveProp( GetSafeHwnd(), SrcTreeArchiver_pszPropName );
	
	CDialog::OnCancel();
}

void CSrcTreeArchiverDlg::OnOK() {
	//OnCancel();
}

void CSrcTreeArchiverDlg::OnSelchangeProfileCombo() {
	
	UpdateData( CONTROLS_TO_VARS );

	// save the settings for the last profile
	CSettings& Settings = CSettings::GetCurrent();
	SaveCurrentProfile();

	// grab the settings for the new profile
	m_nCurrentProfile = m_ctrlProfile.GetCurSel();
	m_sIgnoreExtList = Settings.GetIgnoreExtList( m_nCurrentProfile );
	m_sOutputFilename = Settings.GetDestFilename( m_nCurrentProfile );
	m_sSrcTreeDir = Settings.GetSrcPath( m_nCurrentProfile );
	m_bRecurseSubDirs = Settings.GetRecurseSubDirs( m_nCurrentProfile );
	CTime Date = Settings.GetDate( m_nCurrentProfile );
	if( (u32)Date.GetTime() != 0 ) {
		m_ctrlDatePicker.SetTime( &Date );
	} else {
		m_ctrlDatePicker.SetTime();
	}
	m_ctrlSpanDisks.SetCurSel( Settings.GetDiskSpanIndex( m_nCurrentProfile ) );

	UpdateData( VARS_TO_CONTROLS );
}

void CSrcTreeArchiverDlg::CreateZipFile( BOOL bNoMsgBoxes, const CString &rsIgnoreExtList,
										 const CString &rsOutputFilename, const CString &rsSrcTreeDir,
										 BOOL bRecurseSubDirs, CTime &rDate, u32 nBytesPerDisk, BOOL bUseDate ) {
	CFileInfoArray SourceFiles;
	CFileInfo *pFileInfo;
	const CFileInfo *pFileInfoData;
	int i, nNumFiles;
	BOOL bFileOK;
	CZipArchive ZipFile;
	CTime FileTime;
	CString sRootDir;

	// test input parameters
	if( rsSrcTreeDir.IsEmpty() ||
		rsOutputFilename.IsEmpty() ) {

		if( !bNoMsgBoxes ) {
			this->MessageBox( _T("You must select both a source tree dir\nand an output name before compressing."), 
							  _T("Source Tree Archiver Error"),
							  MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		}
		return;
	}

	m_sProgressText = "Scanning the source tree...";
	UpdateData( VARS_TO_CONTROLS );

	sRootDir = rsSrcTreeDir;
	sRootDir += '\\';

	SourceFiles.AddDir( sRootDir, "*.*", bRecurseSubDirs, CFileInfoArray::AP_NOSORT, FALSE );
	SourceFiles.RemoveExtFiles( rsIgnoreExtList );
	
	nNumFiles = SourceFiles.GetSize();
	
	if( !nNumFiles ) {
		if( !bNoMsgBoxes ) {
			this->MessageBox( _T("No source files were found, nothing to compress."), 
							  _T("Source Tree Archiver Error"),
							  MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		}
		return;
	}

	// see if we should remove any file based on date
	if( bUseDate ) {
		// a date has been supplied
		pFileInfoData = SourceFiles.GetData();
		for( i=nNumFiles-1; i > 0; i-- ) {
			pFileInfo = (CFileInfo *)&pFileInfoData[i];

			FileTime = pFileInfo->GetLastWriteTime();

			if( FileTime < rDate ) {
				// remove this file
				SourceFiles.RemoveAt( i, 1 );
				nNumFiles--;
			}
		}		
	}
	if( !nNumFiles ) {
		if( !bNoMsgBoxes ) {
			this->MessageBox( _T("No source files were found, nothing to compress."), 
							  _T("Source Tree Archiver Error"),
							  MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		}
		return;
	}


	// setup the progress bar
	m_ctrlProgressBar.SetRange( 0, nNumFiles );
	m_ctrlProgressBar.SetPos( 0 );
	m_ctrlProgressBar.SetStep( 1 );

	// create the zip file
	if( !nBytesPerDisk ) {
		// no disk spanning
		ZipFile.Open( rsOutputFilename, CZipArchive::create );
	} else {
		// span the disks
		ZipFile.Open( rsOutputFilename, CZipArchive::createSpan, nBytesPerDisk );
	}

	// run through each file
	pFileInfoData = SourceFiles.GetData();
	for( i=0; i < nNumFiles; i++ ) {
		pFileInfo = (CFileInfo *)&pFileInfoData[i];
	
		m_sProgressText.Format( "Compressing file %d of %d...", i+1, nNumFiles );
		UpdateData( VARS_TO_CONTROLS );

		bFileOK = ZipFile.AddNewFile( pFileInfo->GetFilePath(), Z_BEST_SPEED, TRUE, NULL, NULL, 0x3FFFC );
		if( !bFileOK ) {
			break;
		}
		m_ctrlProgressBar.StepIt();
	}
	ZipFile.Close();

	if( bFileOK ) {
		m_sProgressText.Format( "The zip file was created successfully." );			
	} else {
		m_sProgressText.Format( "Trouble during the zip process, the zip file has been deleted." );				
		::DeleteFile( rsOutputFilename );		
	}
	m_ctrlProgressBar.SetPos( 0 );
	UpdateData( VARS_TO_CONTROLS );	
}

void CSrcTreeArchiverDlg::SaveCurrentProfile() {
	CSettings& Settings = CSettings::GetCurrent();

	Settings.m_asIgnoreExtList[m_nCurrentProfile] = m_sIgnoreExtList;
	Settings.m_asDestFilename[m_nCurrentProfile] = m_sOutputFilename;
	Settings.m_asSrcPath[m_nCurrentProfile] = m_sSrcTreeDir;
	Settings.m_abRecurseSubDirs[m_nCurrentProfile] = m_bRecurseSubDirs;
	CTime Date;
	if( m_ctrlDatePicker.GetTime( Date ) == GDT_NONE ) {
		Settings.m_aDate[m_nCurrentProfile] = (CTime)0;
	} else {
		Settings.m_aDate[m_nCurrentProfile] = Date;
	}
	Settings.m_anDiskSpanIndex[m_nCurrentProfile] = m_ctrlSpanDisks.GetCurSel();

	// write out to disk
	Settings.SaveCommonDataOutToFile();
}
