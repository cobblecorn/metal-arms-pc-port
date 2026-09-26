// SndSeqBankerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fang.h"
#include "SndSeqBanker.h"
#include "SndSeqBankerDlg.h"
#include "fversion.h"
#include "fclib.h"
#include "Settings.h"
#include "FxEditDlg.h"
#include "PickDir.h"
#include "io.h"

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
// CSndSeqBankerDlg dialog

CSndSeqBankerDlg::CSndSeqBankerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSndSeqBankerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSndSeqBankerDlg)
	m_sVersion = _T("");
	m_sNumSeqBankFiles = _T("");
	m_sNumWavBankFiles = _T("");
	m_sCurrentSeqBank = _T("");
	m_sCurrentWavBank = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bPromptForSave = FALSE;
}

void CSndSeqBankerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSndSeqBankerDlg)
	DDX_Control(pDX, IDC_SEQ_BANK_LIST, m_ctrlSeqList);
	DDX_Control(pDX, IDC_WAV_BANK_LIST, m_ctrlWavList);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	DDX_Text(pDX, IDC_NUM_SEQ_BANK_FILES, m_sNumSeqBankFiles);
	DDX_Text(pDX, IDC_NUM_WAV_BANK_FILES, m_sNumWavBankFiles);
	DDX_Text(pDX, IDC_CURRENT_SEQ_BANK, m_sCurrentSeqBank);
	DDX_Text(pDX, IDC_CURRENT_WAV_BANK, m_sCurrentWavBank);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSndSeqBankerDlg, CDialog)
	//{{AFX_MSG_MAP(CSndSeqBankerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_RCLICK, IDC_WAV_BANK_LIST, OnRclickWavBankList)
	ON_NOTIFY(NM_DBLCLK, IDC_WAV_BANK_LIST, OnDblclkWavBankList)
	ON_NOTIFY(NM_DBLCLK, IDC_SEQ_BANK_LIST, OnDblclkSeqBankList)
	ON_NOTIFY(NM_RCLICK, IDC_SEQ_BANK_LIST, OnRclickSeqBankList)
	ON_BN_CLICKED(IDC_CHOOSE_CURRENT_WAV_BANK, OnChooseCurrentWavBank)
	ON_BN_CLICKED(IDC_CHOOSE_CURRENT_SEQ_BANK, OnChooseCurrentSeqBank)
	ON_BN_CLICKED(IDC_CREATE_NEW_BANK, OnCreateNewBank)
	ON_COMMAND(IDC_CREATE_SFX, OnCreateSfx)
	ON_COMMAND(IDC_DELETE_FX, OnDeleteFx)
	ON_COMMAND(IDC_REMOVE_ALL_FXS, OnRemoveAllFxs)
	ON_COMMAND(IDC_SAVE_BANK, OnSaveBank)
	ON_COMMAND(IDC_EDIT_FX, EditFx)
	ON_BN_CLICKED(IDC_CHECK_SFX_NAMES, OnCheckSfxNames)
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_KEYDOWN, IDC_SEQ_BANK_LIST, OnLvnKeydownSeqBankList)	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSndSeqBankerDlg message handlers

BOOL CSndSeqBankerDlg::OnInitDialog()
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

	::SetProp( GetSafeHwnd(), SndSeqBanker_pszPropName, (HANDLE)1 );

	////////////////////////////
	// SET OUR SETTINGS FILENAME
	CSettings::SetSettingsFilename( "SndSeqBanker.ini" ); 

	////////////////////////////////////////////////
	// GET THE DEFAULT VALUES FROM OUR SETTINGS FILE
	CSettings& Settings = CSettings::GetCurrent();
	
	m_sCurrentSeqBank = Settings.GetLastSeqBank();
	BOOL bSeqAsc = TRUE;
	BOOL bBankAsc = TRUE;
	u32 nSeqCol = 0;
	u32 nBankCol = 0;

	if( !m_sCurrentSeqBank.IsEmpty() ) {
		LoadSeqBank( FALSE );
	}
	if( !m_SeqBank.IsEmpty() ) {
		SeqBank_ToolSettings_t *pSettings = m_SeqBank.GetFileSettings();
		m_sCurrentWavBank = pSettings->szBankFileUsed;
		nSeqCol = pSettings->nSeqSortColumn;
		bSeqAsc = pSettings->bSeqAscendingSort;
		nBankCol = pSettings->nBankSortColumn;
		bBankAsc = pSettings->bBankAscendingSort;	
	}

	LoadWavBank();

	// purge any bad fxs
	u32 nPurgedFxs = PurgeFxWithBadWavRefs();
	if( nPurgedFxs ) {
		CString sError;
		sError.Format( "The sequence bank:\n\n"
					   "%s\n\n"
					   "referenced %d wavs that no longer exist in the wav bank.\n"
					   "They have been removed from the sequence bank.\n", m_sCurrentSeqBank, nPurgedFxs );
		MessageBox( sError, 
					_T("Sound Sequencer Banker"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		m_bPromptForSave = TRUE;
	}

	///////////////////////////////////
	// SETUP THE WAV BANK LIST CONTROL
	CRect Rect;
	m_ctrlWavList.GetClientRect( &Rect );
	int nColLen = Rect.Width() - 16;
	int nColX = nColLen / 13;// subtract 16 to account for the right scroll bar width
	m_ctrlWavList.InsertColumn( 0, "Id",		LVCFMT_LEFT, nColX );
	m_ctrlWavList.InsertColumn( 1, "Wav Name",	LVCFMT_LEFT, nColX * 4 );
	m_ctrlWavList.InsertColumn( 2, "Play Length",LVCFMT_LEFT, nColX * 3 );
	m_ctrlWavList.InsertColumn( 3, "Channels",	LVCFMT_LEFT, nColX * 2 );
	m_ctrlWavList.InsertColumn( 4, "Freq (Hz)",	LVCFMT_LEFT, nColX * 3 );
		
	UpdateWavBankListBox( FALSE );

	// set some properties for our list box
	m_ctrlWavList.SetFullRowSel( TRUE );
	m_ctrlWavList.SortColumn( nBankCol, bBankAsc );	
	m_ctrlWavList.EnableSeparatorLines( TRUE );

	///////////////////////////////////
	// SETUP THE WAV BANK LIST CONTROL
	Rect;
	m_ctrlSeqList.GetClientRect( &Rect );
	nColLen = Rect.Width() - 16;
	nColX = nColLen / 7;// subtract 16 to account for the right scroll bar width
	m_ctrlSeqList.InsertColumn( 0, "Id",		LVCFMT_LEFT, nColX );
	m_ctrlSeqList.InsertColumn( 1, "Name",		LVCFMT_LEFT, nColX * 4 );
	m_ctrlSeqList.InsertColumn( 2, "# Commands",LVCFMT_LEFT, nColX * 2);
			
	UpdateSeqBankListBox( FALSE );

	// set some properties for our list box
	m_ctrlSeqList.SetFullRowSel( TRUE );
	m_ctrlSeqList.SortColumn( nSeqCol, bSeqAsc );	
	m_ctrlSeqList.EnableSeparatorLines( TRUE );
	
	////////////////////////
	// display the version #
	m_sVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											 fversion_GetToolMinorVer(),
											 fversion_GetToolSubVer() );

	UpdateData( VARS_TO_CONTROLS );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSndSeqBankerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSndSeqBankerDlg::OnPaint() 
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

HCURSOR CSndSeqBankerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSndSeqBankerDlg::OnOK() {
	//CDialog::OnOK();
}

void CSndSeqBankerDlg::OnCancel() {
	
	UpdateData( CONTROLS_TO_VARS );
	// see if we should ask the user to save the current wav bank
	PromptToSaveBank();

	// writeout our settings before we exit
	CSettings& Settings = CSettings::GetCurrent();
	
	Settings.m_sLastSeqBank = m_sCurrentSeqBank;
		
	Settings.SaveCommonDataOutToFile();

	::RemoveProp( GetSafeHwnd(), SndSeqBanker_pszPropName );
	
	CDialog::OnCancel();
}

void CSndSeqBankerDlg::OnRclickWavBankList(NMHDR* pNMHDR, LRESULT* pResult) {
	UpdateData( CONTROLS_TO_VARS );

	// get the list box's screen rect
	CPoint Point;
	::GetCursorPos( &Point ); 
	m_ctrlWavList.ScreenToClient( &Point );
	
	// select the item under the mouse cursor
	int nIndex = m_ctrlWavList.HitTest( Point );
	if( nIndex >= 0 ) {
		m_ctrlWavList.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED );
	}
	// create the popup menu
	CMenu menu;
	menu.LoadMenu( IDR_WAV_POPUP_MENU );
	CMenu *pContextMenu = menu.GetSubMenu( 0 ); 
	m_ctrlWavList.ClientToScreen( &Point );

	// display the popup menu
	pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this );
	
	*pResult = 0;
}

void CSndSeqBankerDlg::OnDblclkWavBankList(NMHDR* pNMHDR, LRESULT* pResult) {
		
	*pResult = 0;
}

void CSndSeqBankerDlg::OnDblclkSeqBankList(NMHDR* pNMHDR, LRESULT* pResult) {

	EditFx();
		
	*pResult = 0;
}

void CSndSeqBankerDlg::OnRclickSeqBankList(NMHDR* pNMHDR, LRESULT* pResult) {
		
	UpdateData( CONTROLS_TO_VARS );

	// get the list box's screen rect
	CPoint Point;
	::GetCursorPos( &Point ); 
	m_ctrlSeqList.ScreenToClient( &Point );
	
	// select the item under the mouse cursor
//	int nIndex = m_ctrlSeqList.HitTest( Point );
//	if( nIndex >= 0 ) {
//		m_ctrlSeqList.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED );
//	}
	// create the popup menu
	CMenu menu;
	menu.LoadMenu( IDR_SEQ_POPUP_MENU );
	CMenu *pContextMenu = menu.GetSubMenu( 0 ); 
	m_ctrlSeqList.ClientToScreen( &Point );

	// display the popup menu
	pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this );
	
	*pResult = 0;
}

void CSndSeqBankerDlg::OnChooseCurrentWavBank() {
	CString sOldBankFile;

	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".wvb",
					 m_sCurrentWavBank,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Wav Bank Files (*.wvb)|*.wvb||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Wav Bank File";
	if( dlg.DoModal() == IDOK ) {
		// store the old bank name
		sOldBankFile = m_sCurrentWavBank;

		if( sOldBankFile.CompareNoCase( dlg.GetPathName() ) != 0 ) {
			// we have chosen a new wav bank, should we start a new bank file
			m_sCurrentWavBank = dlg.GetPathName();
			UpdateData( VARS_TO_CONTROLS );

			// load the new wav bank
			LoadWavBank();
			
			// setup the wav bank list box
			UpdateWavBankListBox( TRUE );
		}
	}		
}

void CSndSeqBankerDlg::OnChooseCurrentSeqBank() {
	
	UpdateData( CONTROLS_TO_VARS );

	CFileDialog dlg( TRUE,
					 ".sfb",
					 m_sCurrentSeqBank,
					 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
					 "Sound Fx Bank Files (*.sfb)|*.sfb||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Sound Fx Bank File";
	if( dlg.DoModal() == IDOK ) {
		
		PromptToSaveBank();

		m_sCurrentSeqBank = dlg.GetPathName();
		UpdateData( VARS_TO_CONTROLS );

		// load the new seq bank
		LoadSeqBank( TRUE );
			
		// setup the seq bank list box
		UpdateSeqBankListBox( TRUE );
	}	
}

void CSndSeqBankerDlg::LoadWavBank() {

	m_WavBank.CreateEmpty();

	if( m_sCurrentWavBank.IsEmpty() ) {
		return;
	}
	if( !m_WavBank.Read( m_sCurrentWavBank ) ) {
		CString sError;
		sError.Format( "Error reading the wav bank:\n%s\nfrom disk.\n", m_sCurrentWavBank );
		MessageBox( sError, 
					_T("Sound Sequencer Banker"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		m_WavBank.CreateEmpty();		
	}	
}

void CSndSeqBankerDlg::UpdateWavBankListBox( BOOL bReSortList ) {
	u32 i, nNumEntries;
	CBankListItemInfo *pItemInfo;
	WavBank_Entry_t *pEntry;
	
	// empty the list box
	m_ctrlWavList.DeleteAllItems();

	// add all of the entries to our list
	nNumEntries = m_WavBank.GetNumEntries();
	for( i=0; i < nNumEntries; i++ ) {
		pEntry = m_WavBank.GetEntryByIndex( i );

		// add a new item to the list box control
		m_ctrlWavList.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CBankListItemInfo( i, pEntry );
		FASSERT( pItemInfo );
		m_ctrlWavList.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlWavList.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlWavList.SetItemText( i, 2, LPSTR_TEXTCALLBACK );
		m_ctrlWavList.SetItemText( i, 3, LPSTR_TEXTCALLBACK );
		m_ctrlWavList.SetItemText( i, 4, LPSTR_TEXTCALLBACK );
	}

	// resort the file list
	if( bReSortList ) {
		m_ctrlWavList.ReSort();
	}

	m_sNumWavBankFiles.Format( "%d object(s)", nNumEntries );
				
	UpdateData( VARS_TO_CONTROLS );
}

void CSndSeqBankerDlg::LoadSeqBank( BOOL bUseBankToolSettings ) {
	CString sError;

	if( !m_SeqBank.Read( m_sCurrentSeqBank ) ) {
		sError.Format( "Error reading the sequence bank:\n%s\nfrom disk.\n", m_sCurrentSeqBank );
		MessageBox( sError, 
					_T("Sound Sequencer Banker"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
		m_SeqBank.CreateEmpty();
		bUseBankToolSettings = FALSE;
		m_bPromptForSave = FALSE;
		m_sCurrentSeqBank.Empty();
	}

	if( bUseBankToolSettings ) {
		SeqBank_ToolSettings_t *pSettings = m_SeqBank.GetFileSettings();
		m_sCurrentWavBank = pSettings->szBankFileUsed;
		m_ctrlSeqList.SortColumn( pSettings->nSeqSortColumn, pSettings->bSeqAscendingSort );
		m_ctrlWavList.SortColumn( pSettings->nBankSortColumn, pSettings->bBankAscendingSort );

		UpdateData( VARS_TO_CONTROLS );

		// reload the wav bank
		LoadWavBank();

		// purge any bad fxs
		u32 nPurgedFxs = PurgeFxWithBadWavRefs();
		if( nPurgedFxs ) {
			sError.Format( "The sequence bank:\n\n"
						   "%s\n\n"
						   "referenced %d wavs that no longer exist in the wav bank.\n"
						   "They have been removed from the sequence bank.\n", m_sCurrentSeqBank, nPurgedFxs );
			MessageBox( sError, 
						_T("Sound Sequencer Banker"),
						MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
			m_bPromptForSave = TRUE;
		}

		UpdateWavBankListBox( TRUE );
	}
}

void CSndSeqBankerDlg::UpdateSeqBankListBox( BOOL bReSortList ) {
	u32 i, nNumFxs;
	CSeqBankListItemInfo *pItemInfo;
	SeqBank_Fx_t *pFx;
	
	// empty the list box
	m_ctrlSeqList.DeleteAllItems();

	// add all of the Fxs to our list
	nNumFxs = m_SeqBank.GetNumFxs();
	for( i=0; i < nNumFxs; i++ ) {
		pFx = m_SeqBank.GetFxByIndex( i );

		// add a new item to the list box control
		m_ctrlSeqList.InsertItem( i, LPSTR_TEXTCALLBACK );
		pItemInfo = new CSeqBankListItemInfo( i, pFx );
		FASSERT( pItemInfo );
		m_ctrlSeqList.SetItemData( i, (DWORD)pItemInfo );
		m_ctrlSeqList.SetItemText( i, 1, LPSTR_TEXTCALLBACK );
		m_ctrlSeqList.SetItemText( i, 2, LPSTR_TEXTCALLBACK );		
	}

	// resort the file list
	if( bReSortList ) {
		m_ctrlSeqList.ReSort();
	}

	m_sNumSeqBankFiles.Format( "%d object(s)", nNumFxs );
				
	UpdateData( VARS_TO_CONTROLS );
}

void CSndSeqBankerDlg::OnCreateNewBank() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	CFileDialog dlg( FALSE,
					 ".sfb",
					 "new_bank.sfb",
					 OFN_EXTENSIONDIFFERENT | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
					 "Sequence Bank Files (*.sfb)|*.sfb||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select Sequence Bank File To Create";
	if( dlg.DoModal() == IDOK ) {
		// make sure that the new name is ok
		if( dlg.GetFileTitle().GetLength() > 11 ) {
			MessageBox( _T("Bank names must be 11 or fewer characters.\nPlease select a new bank name."), 
						_T("Sound Sequencer Banker"),
						MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
			return;
		}
		// make sure that the user picked the correct extension
		if( dlg.GetFileExt().CompareNoCase( "sfb" ) != 0 ) {
			MessageBox( _T("Sequence banks must have the following file\nextension:\n*.sfb\nPlease select a new filename."), 
						_T("Sound Sequencer Banker"),
						MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			return;
		}
		
		// create a default bank name, use the same path as the currently loaded bank but with the name of the just picked sfb file
		CString sDefault;
		if( m_sCurrentWavBank.IsEmpty() ) {
			sDefault = dlg.GetPathName();
			int nIndex = sDefault.ReverseFind( '.' );
			if( nIndex >= 0 ) {
				sDefault.Delete( nIndex, 4 );
			}			
		} else {
			sDefault = m_sCurrentWavBank;
			int nIndex = sDefault.ReverseFind( '\\' );
			if( nIndex >= 0 ) {
				sDefault.Delete( nIndex, sDefault.GetLength() - nIndex );
			}
			sDefault += '\\';
			sDefault += dlg.GetFileTitle();
		}
		sDefault += ".wvb";			

		// pick a bank to base our new sfb file
		CFileDialog dlg2( TRUE,
						  ".wvb",
						  sDefault,
						  OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
						  "Wav Bank Files (*.wvb)|*.wvb||",
						  this );
		dlg2.m_ofn.lpstrTitle = "Select Wav Bank File";
		if( dlg2.DoModal() == IDOK ) {
			PromptToSaveBank();
			
			m_sCurrentWavBank = dlg2.GetPathName();
			m_sCurrentSeqBank = dlg.GetPathName();
			UpdateData( VARS_TO_CONTROLS );

			// load the new wav bank
			LoadWavBank();

			// start a new seq bank
			m_SeqBank.CreateEmpty();

			m_bPromptForSave = TRUE;
				
			// setup the wav bank list box
			UpdateWavBankListBox( TRUE );
			
			// setup the seq bank list box
			UpdateSeqBankListBox( TRUE );	
		} else {
			MessageBox( _T("Sequence banks must be based on an existing wav bank file.\nA new bank can't be created."), 
						_T("Sound Sequencer Banker"),
						MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
			return;
		}
	}
}

void CSndSeqBankerDlg::OnCreateSfx() {
	
	u32 nNumWavsSelected = m_ctrlWavList.GetSelectedCount();
	if( !nNumWavsSelected ) {
		return;
	}	
	// walk the selected files
	int nIndex, nFxsAdded;
	POSITION Pos = m_ctrlWavList.GetFirstSelectedItemPosition();
	CBankListItemInfo *pItemInfo;
	WavBank_Entry_t *pEntry;
	CString sNewName;
	SeqBank_Fx_t *pFx;
	SeqBank_Cmd_t *pCmd;
	
	nFxsAdded = 0;
	while( Pos ) {
		nIndex = m_ctrlWavList.GetNextSelectedItem( Pos );
		pItemInfo = (CBankListItemInfo *)m_ctrlWavList.GetItemData( nIndex );
		FASSERT( pItemInfo );	

		pEntry = pItemInfo->GetEntry();

		// create a unique name
		CreateUniqueSeqName( pItemInfo->GetFilenameString(), sNewName );

		pFx = m_SeqBank.AddFx( sNewName );
		if( !pFx ) {
			// TBD...
			continue;
		}
		// add a play cmd
		pCmd = m_SeqBank.AddNewCmd( pFx, SEQ_BANK_CMD_TYPE_PLAY );
		if( !pCmd ) {
			// TBD...
			m_SeqBank.DeleteFx( pFx );
			continue;
		}

		// fill in the play cmd
		fclib_strncpy( pCmd->TypeData.Play.szFilename, pEntry->szFilename, WAV_BANK_MAX_WAV_FILENAME_LEN );
		pCmd->TypeData.Play.nNumLoops = 1;
		pCmd->TypeData.Play.nVolPercent = 100;
		pCmd->TypeData.Play.nPitchPercent = 0;

		nFxsAdded++;
	}
	
	if( nFxsAdded ) {
		// update the list box
		UpdateSeqBankListBox( TRUE );
		m_bPromptForSave = TRUE;
	}
}

void CSndSeqBankerDlg::OnDeleteFx() {
	
	u32 nNumSeqsSelected = m_ctrlSeqList.GetSelectedCount();
	if( !nNumSeqsSelected ) {
		return;
	}	
	// walk the selected files
	int nIndex, nSeqsDeleted;
	POSITION Pos = m_ctrlSeqList.GetFirstSelectedItemPosition();
	CSeqBankListItemInfo *pItemInfo;
	SeqBank_Fx_t *pFx;
		
	nSeqsDeleted = 0;
	while( Pos ) {
		nIndex = m_ctrlSeqList.GetNextSelectedItem( Pos );
		pItemInfo = (CSeqBankListItemInfo *)m_ctrlSeqList.GetItemData( nIndex );
		FASSERT( pItemInfo );	

		pFx = pItemInfo->GetFx();

		m_SeqBank.DeleteFx( pFx );
		
		nSeqsDeleted++;
	}
	
	if( nSeqsDeleted ) {
		// update the list box
		UpdateSeqBankListBox( TRUE );
		m_bPromptForSave = TRUE;
	}	
}

void CSndSeqBankerDlg::OnRemoveAllFxs() {
	
	m_SeqBank.CreateEmpty();

	UpdateSeqBankListBox( TRUE );
	m_bPromptForSave = TRUE;
}

#define FILE_CANWRITE	02
#define YEPITWORKED		0
#define FILE_EXISTS		00

void CSndSeqBankerDlg::OnSaveBank() {
	
	if( m_sCurrentSeqBank.IsEmpty() ) {
		return;
	}
	// capture the settings
	SeqBank_ToolSettings_t Settings;
	Settings.bBankAscendingSort = m_ctrlWavList.IsAscSorted();
	Settings.nBankSortColumn = m_ctrlWavList.GetSortedColumn();
	Settings.bSeqAscendingSort = m_ctrlSeqList.IsAscSorted();
	Settings.nSeqSortColumn = m_ctrlSeqList.GetSortedColumn();
	fclib_strncpy( Settings.szBankFileUsed, m_sCurrentWavBank, WAV_BANK_STRING_LEN );
	
	// update the bank's settings
	m_SeqBank.SetFileSettings( &Settings );

	// see if the file exists
	if( _access( m_sCurrentSeqBank, FILE_EXISTS ) == YEPITWORKED ) {
		// file exists, check for file being read only
		if( _access( m_sCurrentSeqBank, FILE_CANWRITE ) != YEPITWORKED ) {
			// can't write, must be read only
			MessageBox( _T("Can't save file, it is read-only."), 
						_T("Sound Sequencer Banker"),
						MB_ICONHAND|MB_OK|MB_DEFBUTTON1);			
			return;
		}
	}

	// write out the bank file
	if( !m_SeqBank.Write( m_sCurrentSeqBank ) ) {
		MessageBox( _T("Error writing the bank file to disk."), 
					_T("Sound Sequencer Banker"),
					MB_ICONHAND|MB_OK|MB_DEFBUTTON1);
	} else {
		m_bPromptForSave = FALSE;
	}	
}

void CSndSeqBankerDlg::CreateUniqueSeqName( const CString &rsWavName, CString &rsSeqName ) {
	SeqBank_Fx_t *pFx;
	u32 i = 0;

	// try to create a seq name that is the same as the wav name
	rsSeqName = rsWavName;
	pFx = m_SeqBank.GetFxByName( rsSeqName );
	while( pFx ) {
		rsSeqName.Format( "%s_%02d", rsWavName, i );
		pFx = m_SeqBank.GetFxByName( rsSeqName );
		i++;
	};	
}

void CSndSeqBankerDlg::PromptToSaveBank() {

	if( m_sCurrentSeqBank.IsEmpty() ) {
		return;
	}
	if( !m_bPromptForSave ) {
		return;
	}
	CString sMsg;
	sMsg.Format( "Save %s?", m_sCurrentSeqBank );
	if( MessageBox( sMsg, _T("Sound Sequencer Banker"), MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON1 ) == IDYES ) {
		OnSaveBank();
	}
}

u32 CSndSeqBankerDlg::PurgeFxWithBadWavRefs() {
	u32 i, j, nNumWavs, nNumFxs, nNumPurged, nNumCmds;
	SeqBank_Fx_t *pFx;
	WavBank_Entry_t *pEntry;

	nNumWavs = m_WavBank.GetNumEntries();
	nNumFxs = m_SeqBank.GetNumFxs();
	nNumPurged = 0;

	if( !nNumWavs ) {
		// there are no wavs, remove all Fxs
		m_SeqBank.CreateEmpty();
		return nNumFxs;
	}
	for( i=0; i < nNumFxs;  ) {
		pFx = m_SeqBank.GetFxByIndex( i );
		
		nNumCmds = pFx->nNumCmds;
		for( j=0; j < nNumCmds; j++ ) {
			pEntry = m_WavBank.GetEntryByName( pFx->paCmds[j].TypeData.Play.szFilename );
			if( !pEntry ) {
				m_SeqBank.DeleteFx( pFx );
				nNumFxs--;
				nNumPurged++;
				break;
			}
		}
		if( j < nNumCmds ) {
			continue;
		}
		i++;
	}
	return nNumPurged;
}

void CSndSeqBankerDlg::OnCheckSfxNames() {
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_SeqBank.GetNumFxs() == 0 ) {
		return;
	}
	
	CString sNewDir;
	CFileInfoArray Files;
	if( pickdir_PickDir( this->m_hWnd, &sNewDir, "Select SFX Bank Directory", m_sCurrentSeqBank ) == PICKDIR_RET_SUCCESS ) {
		// search for .sfb files
		sNewDir += '\\';
		Files.AddDir( sNewDir, "*.sfb", TRUE, CFileInfoArray::AP_NOSORT, FALSE );

		u32 i, j, nNumFxs, nNumFiles;
		CFileInfo *pFileInfo;
		const CFileInfo *pFileInfoData;
		CSeqBank SeqBank;
		SeqBank_Fx_t *pFx;
		CString sBadNames, sFailedToRead, sMsg;

		nNumFxs = m_SeqBank.GetNumFxs();

		sBadNames.Empty();
		sFailedToRead.Empty();
		
		pFileInfoData = Files.GetData();
		nNumFiles = Files.GetSize();
		for( i=0; i < nNumFiles; i++ ) {
			pFileInfo = (CFileInfo *)&pFileInfoData[i];

			if( pFileInfo->GetFilePath().CompareNoCase( m_sCurrentSeqBank ) != 0 ) {
				if( SeqBank.Read( pFileInfo->GetFilePath() ) ) {
					// see if we have duplicate names
					for( j=0; j < nNumFxs; j++ ) {
						pFx = m_SeqBank.GetFxByIndex( j );
						if( pFx ) {
							if( SeqBank.GetFxByName( pFx->szFxName ) ) {
								sBadNames += "\t";
								sBadNames += pFx->szFxName;
								sBadNames += " ( ";
								sBadNames += pFileInfo->GetFileName();
								sBadNames += " )\n";
							}
						}
					}					
				} else {
					sFailedToRead += "\t";
					sFailedToRead += pFileInfo->GetFilePath();
					sFailedToRead += '\n';
				}
			}
		}
		
		sMsg.Empty();
		if( !sFailedToRead.IsEmpty() ||
			!sBadNames.IsEmpty() ) {

			// we had errors, construct the message box text			
						
			if( !sFailedToRead.IsEmpty() ) {
				sMsg += "The following files failed to load:            \n\n";
				sMsg += sFailedToRead;
				sMsg += "\n\n";
			}

			if( !sBadNames.IsEmpty() ) {
				sMsg += "The following names exist in other banks:        \n\n";
				sMsg += sBadNames;
				sMsg += '\n';
				sMsg += "Please rename these sound fxs.\n";					
			}

			MessageBox( sMsg, _T("Error Duplicate Names"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		}
	}
}

void CSndSeqBankerDlg::EditFx() {

	POSITION Pos = m_ctrlSeqList.GetFirstSelectedItemPosition();
	if( Pos ) {
		int nIndex = m_ctrlSeqList.GetNextSelectedItem( Pos );
		CSeqBankListItemInfo *pItemInfo = (CSeqBankListItemInfo *)m_ctrlSeqList.GetItemData( nIndex );
		FASSERT( pItemInfo );	

		CFxEditDlg FxEditDlg;
		FxEditDlg.m_pFx = pItemInfo->GetFx();
		FxEditDlg.m_pSeqBank = &m_SeqBank;
		FxEditDlg.m_pWavBank = &m_WavBank;		
		
		if( FxEditDlg.DoModal() == IDOK ) {
			// update the list box
			UpdateSeqBankListBox( TRUE );
			m_bPromptForSave = TRUE;

			// set the active selection to whatever was selected
			m_ctrlSeqList.SetItemState( nIndex, LVIS_SELECTED, LVIS_SELECTED );
			m_ctrlSeqList.EnsureVisible( nIndex, FALSE );
		}
	}
}

void CSndSeqBankerDlg::OnLvnKeydownSeqBankList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	if( pLVKeyDow->wVKey == VK_SPACE ) {
        EditFx();
	}
	
	*pResult = 0;
}

