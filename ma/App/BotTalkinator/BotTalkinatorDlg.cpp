// BotTalkinatorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fang.h"
#include "faudio.h"
#include "fmath.h"
#include "fperf.h"
#include "fsndfx.h"
#include "settings.h"
#include "vidmode.h"
#include "OptionsWnd.h"
#include "pickdir.h"
#include "BotTalkinator.h"
#include "BotTalkinatorDlg.h"
#include "NewDialog.h"
#include "GestureDialog.h"
#include "PickSfxDlg.h"
#include "io.h"
#include "fresload.h"
#include "mmsystem.h"
#include "frenderer.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

static const f32 fScrollRes = 20.0f;
static const f32 fSliderRes = 100.0f;

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
// CBotTalkinatorDlg dialog

CBotTalkinatorDlg::CBotTalkinatorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBotTalkinatorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBotTalkinatorDlg)
	m_strVersion = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bDataNeedsSaving = FALSE;
	m_pViewport = NULL;
	m_pLastSoundPlayed = NULL;
	m_bWavStarted = FALSE;
}

void CBotTalkinatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBotTalkinatorDlg)
	DDX_Control(pDX, IDC_OFFSET_STATIC, m_staticOffset);
	DDX_Control(pDX, IDC_EDIT_OFFSET, m_ctrlOffsetEdit);
	DDX_Control(pDX, IDC_EDIT_LEN, m_ctrlLenEdit);
	DDX_Control(pDX, IDC_LEN_STATIC, m_staticLen);
	DDX_Control(pDX, IDC_PICK_CSV, m_buttonPickCsv);
	DDX_Control(pDX, IDC_STATIC_SOUNDFILE, m_oSoundFileTitle);
	DDX_Control(pDX, IDC_STATIC_SEL_BOTRACE, m_oSelBotRaceText);
	DDX_Control(pDX, IDC_PICK_SFX, m_buttonPickSfx);
	DDX_Control(pDX, IDC_BUTTON_SAVE, m_buttonSave);
	DDX_Control(pDX, IDC_BUTTON_EXPORT, m_oButtonExport);
	DDX_Control(pDX, IDC_EDIT_EXFILE, m_oEditExFile);
	DDX_Control(pDX, IDC_TEXT_EXFILE, m_oTextExFile);
	DDX_Control(pDX, IDC_CHECK_TALKLOOP, m_oCheckTalkLoop);
	DDX_Control(pDX, IDC_STATIC_EDITOR2, m_oEditorBorder2);
	DDX_Control(pDX, IDC_STATIC_VIEWFRAME, m_oViewStaticFrame);
	DDX_Control(pDX, IDC_STATIC_MILVOICE, m_oEditorStaticMilVoice);
	DDX_Control(pDX, IDC_COMBO_MILVOICE, m_oEditorComboMilVoice);
	DDX_Control(pDX, IDC_CHECK_EDITOR_EXTDRAG, m_oEditorCheckExtDrag);
	DDX_Control(pDX, IDC_BUTTON_SETPLAYFROM, m_oEditorButtonSetPlayFrom);
	DDX_Control(pDX, IDC_BUTTON_PLAYFROM, m_oEditorButtonPlayFrom);
	DDX_Control(pDX, IDC_STATIC_BANK2, m_oEditorStaticBank2);
	DDX_Control(pDX, IDC_STATIC_BANK1, m_oEditorStaticBank1);
	DDX_Control(pDX, IDC_SCROLLBAR_GEST, m_oEditorScrollGest);
	DDX_Control(pDX, IDC_SCROLLBAR_BANK, m_oEditorScrollBank);
	DDX_Control(pDX, IDC_LIST1, m_oEditorListGesture);
	DDX_Control(pDX, IDC_EDIT_NAME, m_oEditorEditName);
	DDX_Control(pDX, IDC_STATIC_EDITOR_NAME, m_oEditorStaticName);
	DDX_Control(pDX, IDC_COMBO_BOTNAME, m_oSelectorComboBotName);
	DDX_Control(pDX, IDC_EDIT_EDITOR_WAVFILE, m_oEditorEditWAVFile);
	DDX_Control(pDX, IDC_CHECK_EDITOR_USEWAV, m_oEditorCheckUseWAV);
	DDX_Control(pDX, IDC_CHECK_EDITOR_IS2D, m_oEditorCheckIs2d);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_oViewPlay);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_oViewPause);
	DDX_Control(pDX, IDC_STATIC_CURTIMEPOS, m_oEditorCurTimePos);
	DDX_Control(pDX, IDC_SLIDER_TIMEPOS, m_oEditorTimePos);
	DDX_Control(pDX, IDC_BUTTON_OPTIONS, m_oMainOptions);
	DDX_Control(pDX, IDC_STATIC_SEL_TITLE, m_oSelTitleText);
	DDX_Control(pDX, IDC_STATIC_EDITOR_TITLE, m_oEditorTitleText);
	DDX_Control(pDX, IDC_STATIC_EDITOR_DIALOGUE, m_oEditorDialogueText);
	DDX_Control(pDX, IDC_STATIC_SELECTOR, m_oSelBorder);
	DDX_Control(pDX, IDC_STATIC_EDITOR, m_oEditorBorder);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_oEditorTitle);
	DDX_Control(pDX, IDC_EDIT_DIALOGUE, m_oEditorDialogue);
	DDX_Control(pDX, IDCANCEL, m_oMainExit);
	DDX_Control(pDX, IDC_BUTTON_SELECT, m_oSelSelect);
	DDX_Control(pDX, IDC_BUTTON_DELETE, m_oSelDelete);
	DDX_Control(pDX, IDC_BUTTON_NEW, m_oSelNew);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_oSelNext);
	DDX_Control(pDX, IDC_BUTTON_PREVIOUS, m_oSelPrev);
	DDX_Control(pDX, IDC_BUTTON_CANCELSEL, m_oEditorCancel);
	DDX_Control(pDX, IDC_BUTTON_OKAYSEL, m_oEditorOK);
	DDX_Control(pDX, IDC_COMBO_TITLE, m_oTitle);
	DDX_Text(pDX, IDC_STATIC_VERSION, m_strVersion);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBotTalkinatorDlg, CDialog)
	//{{AFX_MSG_MAP(CBotTalkinatorDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS, OnButtonPrevious)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, OnButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_CANCELSEL, OnButtonCancelsel)
	ON_BN_CLICKED(IDC_BUTTON_OKAYSEL, OnButtonOkaysel)
	ON_CBN_SELCHANGE(IDC_COMBO_TITLE, OnSelchangeComboTitle)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_OPTIONS, OnButtonOptions)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnButtonPause)
	ON_BN_CLICKED(IDC_CHECK_EDITOR_USEWAV, OnCheckEditorUsewav)
	ON_BN_CLICKED(IDC_CHECK_EDITOR_IS2D,   OnCheckEditorIs2d)
	ON_CBN_SELCHANGE(IDC_COMBO_BOTNAME, OnSelchangeComboBotname)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_PLAYFROM, OnButtonPlayfrom)
	ON_BN_CLICKED(IDC_BUTTON_SETPLAYFROM, OnButtonSetplayfrom)
	ON_BN_CLICKED(IDC_CHECK_EDITOR_EXTDRAG, OnCheckEditorExtdrag)
	ON_BN_CLICKED(IDC_CHECK_TALKLOOP, OnCheckTalkloop)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, OnButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_PICK_SFX, OnPickSfx)
	ON_EN_KILLFOCUS(IDC_EDIT_EXFILE, OnKillfocusEditExfile)
	ON_BN_CLICKED(IDC_PICK_CSV, OnPickCsv)
	ON_EN_KILLFOCUS(IDC_EDIT_LEN, OnKillfocusEditLen)
	ON_EN_KILLFOCUS(IDC_EDIT_OFFSET, OnKillfocusEditOffset)
	ON_EN_KILLFOCUS(IDC_EDIT_TITLE, OnKillfocusEditTitle)
	ON_EN_KILLFOCUS(IDC_EDIT_DIALOGUE, OnKillfocusEditDialogue)
	ON_MESSAGE(BOTTALKINATOR_MSG_INITCOMPLETE, OnInitComplete)
	ON_EN_KILLFOCUS(IDC_EDIT_EDITOR_WAVFILE, OnKillfocusEditEditorWavfile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBotTalkinatorDlg message handlers

BOOL CBotTalkinatorDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if( pSysMenu != NULL ) {
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if( !strAboutMenu.IsEmpty() ) {
			pSysMenu->InsertMenu( 0, MF_STRING, IDM_ABOUTBOX, strAboutMenu );
			//pSysMenu->AppendMenu(MF_SEPARATOR);
			//pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		pSysMenu->EnableMenuItem( 2, MF_BYPOSITION | MF_GRAYED );
		pSysMenu->EnableMenuItem( 4, MF_BYPOSITION | MF_GRAYED );
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// limit to just 1 bot talk at any given time
	::SetProp( GetSafeHwnd(), BotTalkinator_pszPropName, (HANDLE)1 );

	// display the version number
	m_strVersion.Format( "version # %d.%d.%d", fversion_GetToolMajorVer(), 
											   fversion_GetToolMinorVer(),
											   fversion_GetToolSubVer() );

	// Set up our critical section object.
	InitializeCriticalSection(&m_CriticalSection);
	
	// Obtain our settings object.
	CSettings::SetSettingsFilename( "BotTalkinator.ini" );
	
	UpdateData( VARS_TO_CONTROLS );

	return InitApplication();
}

void CBotTalkinatorDlg::OnSysCommand( UINT nID, LPARAM lParam ) {

	if( (nID & 0xFFF0) == IDM_ABOUTBOX ) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CBotTalkinatorDlg::OnPaint() {

	if( IsIconic() ) {
		CPaintDC dc( this ); // device context for painting

		SendMessage( WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0 );

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon( x, y, m_hIcon );
	} else {
		CDialog::OnPaint();
	}
}

void CBotTalkinatorDlg::Shutdown() {

	if( floop_IsGameloopRunning() ) {
		floop_UninstallGameloop();
	}

	if( fang_IsStartedUp() ) {
		faudio_Uninstall();
		fang_Shutdown();
	}

	m_oDialogueList.UnInit();
}

void CBotTalkinatorDlg::RefreshBotNameCombo() {
	POSITION pPos = m_oDialogueList.m_lpDialogueEntries.GetHeadPosition();

	m_oSelectorComboBotName.ResetContent();
	while( pPos != NULL ) {
		CDialogueEntryN *poDE = (CDialogueEntryN *)m_oDialogueList.m_lpDialogueEntries.GetNext( pPos );
		FASSERT(poDE != NULL);
		s32 nIndex = m_oSelectorComboBotName.AddString(poDE->m_poCharInfo->m_strCharName);
		m_oSelectorComboBotName.SetItemDataPtr(nIndex, poDE);
	}
	UpdateData( VARS_TO_CONTROLS );
}

void CBotTalkinatorDlg::RefreshTitleCombo( CDialogueEntryN *poDE ) {
	CDialogueInstN *poDI;

	//// Add the dialogue titles to the appropriate combo box.
	u32 uComboIdx;
	POSITION pPos = poDE->m_lpDialogues.GetHeadPosition();
	m_oTitle.ResetContent();
	while( pPos != NULL ) {
		poDI = (CDialogueInstN *)(poDE->m_lpDialogues.GetNext(pPos));
		uComboIdx = m_oTitle.AddString(poDI->m_strTitle);
		m_oTitle.SetItemDataPtr(uComboIdx, poDI);
	}	
}

void CBotTalkinatorDlg::RefreshSlider( CDialogueInstN *pDialogueInst ) {
		CString strTemp;

		strTemp.Format("%2.2f/%2.2fs", m_fPlayTimePos, pDialogueInst->m_fTotalTime);
		m_oEditorCurTimePos.SetWindowText(strTemp);
		m_oEditorTimePos.SetRange(0, (int)(m_fBankWidth * fSliderRes), TRUE);
		m_oEditorTimePos.SetTicFreq((int)(fSliderRes * (1.0f / 10.0f)));		
}

void CBotTalkinatorDlg::SetAreaActive( Area_e eArea, BOOL bActive ) {

	switch( eArea ) {

	case AREA_SELECTOR:
		// Handle everything in the top section.
		m_oSelectorComboBotName.EnableWindow(bActive);
		m_oTitle.EnableWindow(bActive);
		m_oSelPrev.EnableWindow(bActive);
		m_oSelNext.EnableWindow(bActive);
		m_oSelNew.EnableWindow(bActive);
		m_oSelDelete.EnableWindow(bActive);
		m_oSelSelect.EnableWindow(bActive && m_bValidVideoMode);
		m_oMainExit.EnableWindow(bActive);
		m_buttonSave.EnableWindow(bActive);
		m_oSelBorder.EnableWindow(bActive);
		m_oSelTitleText.EnableWindow(bActive);
		m_oSelBotRaceText.EnableWindow(bActive);
		m_oMainOptions.EnableWindow(bActive);
		break;

	case AREA_EDITOR:
		// Handle the buttons in the lower section.
		m_oEditorOK.EnableWindow(bActive);
		m_oEditorCancel.EnableWindow(bActive);
		m_oEditorDialogue.EnableWindow(bActive);
		m_oEditorEditName.EnableWindow(bActive);
		m_oEditorTitle.EnableWindow(bActive);
		m_oEditorBorder.EnableWindow(bActive);
		m_oEditorBorder2.EnableWindow(bActive);
		m_oEditorTitleText.EnableWindow(bActive);
		m_oEditorStaticName.EnableWindow(bActive);
		m_oEditorDialogueText.EnableWindow(bActive);
		m_oEditorTimePos.EnableWindow(bActive);
		m_oEditorCurTimePos.EnableWindow(bActive);
		m_oViewPlay.EnableWindow(bActive);
		m_oEditorEditWAVFile.EnableWindow(bActive && m_oTempDI.m_bUseWAV);
		m_buttonPickSfx.EnableWindow( bActive && m_oTempDI.m_bUseWAV );
		m_buttonPickCsv.EnableWindow( bActive );
		m_oEditorCheckUseWAV.EnableWindow(bActive);
		m_oEditorCheckIs2d.EnableWindow(bActive);
		m_oEditorListGesture.EnableWindow(bActive);
		m_oEditorScrollBank.EnableWindow(bActive);
		m_oEditorScrollGest.EnableWindow(bActive);
		m_oEditorStaticBank1.EnableWindow(bActive);
		m_oEditorStaticBank2.EnableWindow(bActive);
		m_oEditorButtonPlayFrom.EnableWindow(bActive);
		m_oEditorButtonSetPlayFrom.EnableWindow(bActive);
		m_oEditorCheckExtDrag.EnableWindow(bActive);
		m_oEditorStaticMilVoice.EnableWindow(bActive);
		m_oSoundFileTitle.EnableWindow(bActive);
		m_ctrlLenEdit.EnableWindow(bActive && m_oTempDI.m_bUseWAV);
		m_ctrlOffsetEdit.EnableWindow(bActive && m_oTempDI.m_bUseWAV);
		m_staticLen.EnableWindow(bActive);
		m_staticOffset.EnableWindow( bActive );
		m_oEditorComboMilVoice.EnableWindow(bActive && !m_oTempDI.m_bUseWAV);
		m_oCheckTalkLoop.EnableWindow(bActive);
		m_oEditExFile.EnableWindow(bActive);
		m_oTextExFile.EnableWindow(bActive);
		m_oButtonExport.EnableWindow(bActive);
		break;
	}
}

void CBotTalkinatorDlg::SetCurActiveArea( Area_e eActiveArea ) {
	BOOL bArea1, bArea2, bArea3;

	switch( eActiveArea ) {

	case AREA_SELECTOR:
		bArea1 = TRUE;
		bArea2 = FALSE;
		bArea3 = FALSE;
		break;
	
	case AREA_EDITOR:
		bArea1 = FALSE;
		bArea2 = TRUE;
		bArea3 = FALSE;
		break;
	}

	SetAreaActive( AREA_SELECTOR, bArea1 );
	SetAreaActive( AREA_EDITOR, bArea2 );
}

void CBotTalkinatorDlg::ShowPreviewData(CDialogueInstN *pDialogueInst)
{
	m_oEditorTitle.SetWindowText(pDialogueInst->m_strTitle);
	m_oEditorEditName.SetWindowText(pDialogueInst->m_strName);
	m_oEditorDialogue.SetWindowText(pDialogueInst->m_strDialogue);
	m_oEditorCheckUseWAV.SetCheck(pDialogueInst->m_bUseWAV);
	m_oEditorCheckIs2d.SetCheck(pDialogueInst->m_bIs2d);
	m_oEditorEditWAVFile.SetWindowText(pDialogueInst->m_strWAVFile);
	m_oEditExFile.SetWindowText(pDialogueInst->m_strExportFile);
	m_oCheckTalkLoop.SetCheck(pDialogueInst->m_bUseTalkLoop);
	m_oEditorComboMilVoice.SetCurSel( pDialogueInst->m_uMilVoice );

	CString sValue;
	sValue.Format( "%.3f", pDialogueInst->m_fMinTotalTime );
	m_ctrlLenEdit.SetWindowText( sValue );
	
	sValue.Format( "%.3f", pDialogueInst->m_fWavStartOffset );
	m_ctrlOffsetEdit.SetWindowText( sValue );

	SCROLLINFO oScrollInfo;
	oScrollInfo.cbSize = sizeof(SCROLLINFO);
	oScrollInfo.fMask = SIF_PAGE | SIF_RANGE;
	oScrollInfo.nMin = 0;
	oScrollInfo.nMax = (s32)((pDialogueInst->m_fTotalTime) * fScrollRes);
	oScrollInfo.nPage = (s32)(m_fBankWidth * fScrollRes);
	m_oEditorScrollBank.SetScrollInfo(&oScrollInfo, TRUE);
	m_oEditorListGesture.SetDialogueInst(pDialogueInst);

	RefreshSlider(pDialogueInst);
	OnUpdateBankPos();
	UpdateData( VARS_TO_CONTROLS );
}

void CBotTalkinatorDlg::OnUpdateBankPos() {
	CString strTemp;

	strTemp.Format("%2.2f", m_fBankStart);
	m_oEditorStaticBank1.SetWindowText(strTemp);
	strTemp.Format("%2.2f", m_fBankStart + m_fBankWidth);
	m_oEditorStaticBank2.SetWindowText(strTemp);
	m_oEditorListGesture.SetBankPos(m_fBankStart);
	UpdateData( VARS_TO_CONTROLS );
}

BOOL CBotTalkinatorDlg::GameInit( void *pParameter ) {
	GameLoopParam_s *pGameLoopData = (GameLoopParam_s *)(pParameter);
	CBotTalkinatorDlg *pTheDlg = pGameLoopData->pDlg;
	FVidWin_t oVidSettings = pGameLoopData->oVidSettings;

	if( !fvid_CreateWindow(&oVidSettings) ) {
		AfxMessageBox("Window creation failed.");
		pTheDlg->m_bGameInitComplete = TRUE;
		return FALSE;
	}

	if( fresload_Load(FWORLD_RESTYPE, "WED1default") == NULL ) {
		AfxMessageBox("Could not load default world.");
		pTheDlg->m_bGameInitComplete = TRUE;
		return FALSE;
	}

	pTheDlg->m_pViewport = fviewport_Create();
	
	// grab the coordinates of the screen render window so that we can create an exact viewport
	CRect Rect;
	pTheDlg->m_oViewStaticFrame.GetClientRect( &Rect );	
	fviewport_InitPersp( pTheDlg->m_pViewport, FMATH_DEG2RAD( 42.0f ), 0.01f, 10000.0f, 0, 0, Rect.Width(), Rect.Height() );

	pTheDlg->m_pLastSoundPlayed = NULL;
	pTheDlg->m_bWavStarted = FALSE;

	// We need to init the meshes after the window has been created.
	pTheDlg->m_oDialogueList.m_oCharInfoList.InitMeshes();

	pTheDlg->m_bGameInitComplete = TRUE;

	return TRUE;
}

void CBotTalkinatorDlg::GameTerm( FLoopTermCode_t nTermCode, void *pParameter ) {
	GameLoopParam_s *pGameLoopData = (GameLoopParam_s *)(pParameter);
	CBotTalkinatorDlg *pTheDlg = pGameLoopData->pDlg;

	fvid_DestroyWindow();
}

BOOL CBotTalkinatorDlg::GameMain( BOOL bExitRequest, void *pParameter ) {
	GameLoopParam_s *pGameLoopData = (GameLoopParam_s *)(pParameter);
	CBotTalkinatorDlg *pTheDlg = pGameLoopData->pDlg;

	// After this point, we don't want any of our flags that we will use to be changed.
	EnterCriticalSection(&pTheDlg->m_CriticalSection);

	// Check to see if we have any reason to quit.
	if( bExitRequest ) {
		LeaveCriticalSection(&pTheDlg->m_CriticalSection);
		return FALSE;
	}
		
	// If they went into PlayFrom mode, let's just update the current position
	//   and then act as if we were in Play mode.
	if( pTheDlg->m_eViewMode == VIEWMODE_PLAYFROM ) {
		pTheDlg->m_fPlayTimePos = pTheDlg->m_fPlayFromPos;
		pTheDlg->m_eViewMode = VIEWMODE_PLAY;
	}
	
	// Cache some variables for use.
	f32 fCurPlayPos = pTheDlg->m_fPlayTimePos;
	f32 fDialogueLength = pTheDlg->m_oTempDI.m_fTotalTime;
	
	// Stuff to do only if the play cursor is changing.
	if( pTheDlg->m_eViewMode == VIEWMODE_PLAY ) {
		// Increment our play cursor position.
		fCurPlayPos += FLoop_fPreviousLoopSecs;
		if( fCurPlayPos >= fDialogueLength ) {
			fCurPlayPos = fDialogueLength;
			pTheDlg->OnButtonPause();
		}
		
		// Update the UI elements.

		// Tell the custom gesture list control where to draw the current play position.
		pTheDlg->m_oEditorListGesture.SetPtrPos( fCurPlayPos );

		// Tell the slider control to move itself to the current play position.
		pTheDlg->m_oEditorTimePos.SetPos( (int)((fCurPlayPos - pTheDlg->m_fBankStart) * fSliderRes) );

		// This should get changed to a call that only updates the slider of the view section.
		pTheDlg->RefreshSlider( &pTheDlg->m_oTempDI );
		
		// Search for any new sounds that might need to be started.
		if( !pTheDlg->m_oTempDI.m_bUseWAV ) {
			// MilSpeak mode
			CSoundInst *pCurSound;

			if( pTheDlg->m_oTempDI.FindSoundAtTime(fCurPlayPos, &pCurSound) ) {
				if( pCurSound != pTheDlg->m_pLastSoundPlayed ) {
					if( !pCurSound->m_pSound->Play(fCurPlayPos - pCurSound->m_fStartTime) ) {
						DEVPRINTF("Error playing this sound (probably not anything to worry about).\n");
					}
				}
			} else {
				pCurSound = NULL;
			}
			pTheDlg->m_pLastSoundPlayed = pCurSound;
		} else {
			// Non MilSpeak mode
			if( !pTheDlg->m_bWavStarted ) {

				// only start playing if the play cursor is very near or ahead of the play point
				f32 fSecsTillWavStart = fCurPlayPos - pTheDlg->m_oTempDI.m_fWavStartOffset;

				if( fSecsTillWavStart >= 0.0f && fSecsTillWavStart <= (1.0f/60.0f) ) {
					if( pTheDlg->m_oTempDI.m_strWAVFile.Right(4) == ".wav" ) {
						PlaySound( pTheDlg->m_oTempDI.m_strWAVFile, NULL, SND_FILENAME | SND_ASYNC );
					} else {
						FSndFx_FxHandle_t hFXHandle = fsndfx_GetFxHandle( pTheDlg->m_oTempDI.m_strWAVFile );
						fsndfx_Play2D( hFXHandle );
					}
					pTheDlg->m_bWavStarted = TRUE;
				}
			}
		}

		pTheDlg->m_fPlayTimePos = fCurPlayPos;
	}

	fvid_Begin();

	fviewport_SetActive( pTheDlg->m_pViewport );
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.1f, 1.0f, 0 );

	CFXfm oCamera;
	oCamera.Identity();
	oCamera.InitStackWithView();

	frenderer_Push( FRENDERER_MESH, NULL );

	fmesh_Ambient_Set( 1.0f, 1.0f, 1.0f, 1.0f );

	pTheDlg->m_oTempDI.SetCurTimePos( fCurPlayPos );
	pTheDlg->m_oTempDI.Draw();

	frenderer_Pop();
	fvid_End();
		
	LeaveCriticalSection( &pTheDlg->m_CriticalSection );
	
	if( !fvid_Swap() ) {
		return FALSE;
	}

	return TRUE;
}

// this is only called after on init dlg is called
long CBotTalkinatorDlg::OnInitComplete( WPARAM wParam, LPARAM lParam ) {
	
	m_bGameInitComplete = FALSE;

	// We need to install the game loop after the dialog has finished initializing.
	if( !floop_InstallGameloop( CBotTalkinatorDlg::GameInit,
								CBotTalkinatorDlg::GameMain,
								CBotTalkinatorDlg::GameTerm,
								(void *)&m_oGameLoopData, 60 ) ) {
		AfxMessageBox( "CBotTalkinatorDlg::OnInitComplete() : Error installing gameloop.\n" );
		PostMessage( WM_QUIT );
		return 0;
	}

	// Wait until GameInit has completed its operations.
	while( !m_bGameInitComplete ) {
		// give the other threads some time
		Sleep( 33 );
	}

	return 0;
}

// The system calls this to obtain the cursor to display while the user drags the minimized window.
HCURSOR CBotTalkinatorDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

void CBotTalkinatorDlg::OnButtonNew() {
	CNewDialog oDialog;

	m_oTempDI.Clear();
	// Get the name of the currently selected robot
	m_oSelectorComboBotName.GetLBText( m_oSelectorComboBotName.GetCurSel(), m_oTempDI.m_strName );
	m_oTempDI.m_strTitle = "<Enter title>";
	m_oTempDI.m_strDialogue = "";
	m_oTempDI.m_strRace = "";
	m_oTempDI.m_bUseWAV = FALSE;
	m_oTempDI.m_strWAVFile = "";
	m_oTempDI.m_uMilVoice = 0;
	m_oTempDI.m_uNumGestures = 0;
	m_oTempDI.m_uNumSounds = 0;
	m_oTempDI.m_strExportFile = "";
	if( oDialog.DoGetNewDialog(&m_oTempDI) ) {
		// create a dialog inst for our new entry
		m_oDialogueList.AddDialogueInst( &m_oTempDI );

		CString sTitle = m_oTempDI.m_strTitle;
		OnSelchangeComboBotname();
			
		int nFoundPos = m_oTitle.FindString(-1, sTitle );
		FASSERT(nFoundPos != -1);
		m_oTitle.SetCurSel(nFoundPos);
		OnSelchangeComboTitle();

		UpdateData( VARS_TO_CONTROLS );
		m_oTempDI.RecalcUserAnimSlots();

		// go ahead and "press the select button"
		((CButton *)GetDlgItem( IDC_BUTTON_SELECT ))->PostMessage( BM_CLICK );

		m_bDataNeedsSaving = TRUE;
	}
}

void CBotTalkinatorDlg::OnButtonPrevious() {

	u32 uCurTitleSel = m_oTitle.GetCurSel();

	if( uCurTitleSel == 0 ) {
		uCurTitleSel = m_oTitle.GetCount() - 1;
	} else {
		--uCurTitleSel;
	}
	m_oTitle.SetCurSel(uCurTitleSel);

	UpdateData( VARS_TO_CONTROLS );
	OnSelchangeComboTitle();
}

void CBotTalkinatorDlg::OnButtonNext() {

	u32 uCurTitleSel = m_oTitle.GetCurSel();
	if( uCurTitleSel == (u32)(m_oTitle.GetCount() - 1) ) {
		uCurTitleSel = 0;
	} else {
		++uCurTitleSel;
	}
	m_oTitle.SetCurSel(uCurTitleSel);

	UpdateData( VARS_TO_CONTROLS );
	OnSelchangeComboTitle();
}

void CBotTalkinatorDlg::OnButtonEdit() {

	u32 uCurTitleIdx = m_oTitle.GetCurSel();
	CDialogueInstN *poDI = (CDialogueInstN *)m_oTitle.GetItemDataPtr( uCurTitleIdx );
	m_oTempDI.Copy( poDI );
	m_oTempDI.RecalcUserAnimSlots();
	m_oEditorListGesture.SetDialogueInst( &m_oTempDI );
	SetCurActiveArea( AREA_EDITOR );

	CDialogueListN::s_pCurDE = m_oTempDI.m_pDE;
}

// Called when the 'Cancel' button is clicked in the editor section.
void CBotTalkinatorDlg::OnButtonCancelsel() {

	//// Get the data from the controls.
	UpdateData( CONTROLS_TO_VARS );
	m_oEditorEditName.GetWindowText( m_oTempDI.m_strName );
	m_oEditorTitle.GetWindowText( m_oTempDI.m_strTitle );
	m_oEditorDialogue.GetWindowText( m_oTempDI.m_strDialogue );
	m_oEditExFile.GetWindowText( m_oTempDI.m_strExportFile );
	m_oTempDI.m_uMilVoice = m_oEditorComboMilVoice.GetCurSel();

	if( m_oEditorCheckUseWAV.GetCheck() ) {
		m_oTempDI.m_bUseWAV = TRUE;
	} else {
		m_oTempDI.m_bUseWAV = FALSE;
	}
	if( m_oEditorCheckIs2d.GetCheck() ) {
		m_oTempDI.m_bIs2d = TRUE;
	} else {
		m_oTempDI.m_bIs2d = FALSE;
	}

	m_oEditorEditWAVFile.GetWindowText(m_oTempDI.m_strWAVFile);
	
	// Find the old entry.
	u32 uCurTitleIdx = m_oTitle.GetCurSel();
	CDialogueInstN *poDI = (CDialogueInstN *)(m_oTitle.GetItemDataPtr(uCurTitleIdx));
	FASSERT(poDI != NULL);
	
	if( m_oTempDI.IsDifferent(poDI) ) {
		if( AfxMessageBox( "Changes were made and are about to be lost.\nLose changes?", MB_YESNO) == IDNO ) {
			return;
		}
	}

	m_oTempDI.Copy(poDI);
	ShowPreviewData(&m_oTempDI);
	ResetPointers();
	SetCurActiveArea(AREA_SELECTOR);
	UpdateData( VARS_TO_CONTROLS );
}

// Called when the user clicks 'OK' in the editor section.
void CBotTalkinatorDlg::OnButtonOkaysel() {

	//// Get the data from the controls.
	UpdateData( CONTROLS_TO_VARS );
	m_oEditorEditName.GetWindowText( m_oTempDI.m_strName );
	m_oEditorTitle.GetWindowText( m_oTempDI.m_strTitle );
	m_oEditorDialogue.GetWindowText( m_oTempDI.m_strDialogue );
	m_oEditExFile.GetWindowText( m_oTempDI.m_strExportFile );
	m_oTempDI.m_uMilVoice = m_oEditorComboMilVoice.GetCurSel();

	if( m_oEditorCheckUseWAV.GetCheck() ) {
		m_oTempDI.m_bUseWAV = TRUE;
	} else {
		m_oTempDI.m_bUseWAV = FALSE;
	}
	if( m_oEditorCheckIs2d.GetCheck() ) {
		m_oTempDI.m_bIs2d = TRUE;
	} else {
		m_oTempDI.m_bIs2d = FALSE;
	}
	m_oEditorEditWAVFile.GetWindowText( m_oTempDI.m_strWAVFile );
	
	// Find the old entry.
	u32 uCurTitleIdx = m_oTitle.GetCurSel();
	CDialogueInstN *poDI = (CDialogueInstN *)m_oTitle.GetItemDataPtr( uCurTitleIdx );
	FASSERT(poDI != NULL);
	
	if( m_oTempDI.IsDifferent( poDI ) ) {
		// Delete the old entry.
		CDialogueEntryN * poDE = m_oDialogueList.FindDialogueEntry( poDI->m_strName );
		FASSERT(poDE != NULL);
		if( !poDE->DeleteEntry(poDI) ) {
			AfxMessageBox("Internal Error: Report this to Mike.");
		}
		delete poDI;
		
		//// Prune any gestures that fall completely off the right edge of the dialogue.
		u32 uGestIdx;
		for( uGestIdx = 0 ; uGestIdx < m_oTempDI.m_uNumGestures; ++uGestIdx ) {
			if( m_oTempDI.m_aoGestureInst[uGestIdx].m_fStartTime >= m_oTempDI.m_fTotalTime ) {
				m_oTempDI.RemoveGesture( &m_oTempDI.m_aoGestureInst[uGestIdx] );
			}
		}
		
		//// Insert our new entry.
		m_oDialogueList.AddDialogueInst( &m_oTempDI );
		
		// We save off the title here because OnSelchangeComboBotName will change the m_oTempDI.
		CString strTitle = m_oTempDI.m_strTitle;
		OnSelchangeComboBotname();

		// Find the entry that we just inserted and select it.
		s32 nIndex = m_oTitle.FindStringExact( 0, strTitle );
		FASSERT( nIndex != -1 );
		m_oTitle.SetCurSel( nIndex );
		CDialogueInstN *pNewDI = (CDialogueInstN *)m_oTitle.GetItemDataPtr( nIndex );
		m_oTempDI.Copy( pNewDI );

		// Inform the list control of the new dialogue instance.
		m_oEditorListGesture.SetDialogueInst( &m_oTempDI );
		ShowPreviewData( &m_oTempDI );

		// mark that the data now needs to be saved
		m_bDataNeedsSaving = TRUE;
	}
	ResetPointers();
	SetCurActiveArea( AREA_SELECTOR );
	UpdateData( VARS_TO_CONTROLS );
}

// Callback for when the Dialogue Title ComboBox gets changed.
// Can also be called directly, but the current selection should be set *before* this function gets executed.
void CBotTalkinatorDlg::OnSelchangeComboTitle() {
	UpdateData( CONTROLS_TO_VARS );

	s32 nIndex = m_oTitle.GetCurSel();
	if( nIndex == -1 ) {
		OnButtonNew();
	}
	while( nIndex == -1 ) {
		OnButtonNew();
	}
	CDialogueInstN *poDI = (CDialogueInstN *)m_oTitle.GetItemDataPtr( nIndex );
	m_pCurDE = poDI->m_pDE;
	m_oTempDI.Copy( poDI );
	m_oTempDI.CalculateTime();
	ShowPreviewData( &m_oTempDI );
}

void CBotTalkinatorDlg::OnButtonDelete() {

	UpdateData( CONTROLS_TO_VARS );

	// Find the old entry and delete it.	
	u32 uCurTitleIdx = m_oTitle.GetCurSel();
	CDialogueInstN *poDI = (CDialogueInstN *)m_oTitle.GetItemDataPtr( uCurTitleIdx );
	FASSERT(poDI != NULL);

	CDialogueEntryN *poDE = m_oDialogueList.FindDialogueEntry( poDI->m_strName );
	FASSERT( poDE != NULL );
	if( !poDE->DeleteEntry( poDI ) ) {
		AfxMessageBox("Internal Error: Report this to Mike.");
	}
	delete poDI;
	
	// This may need to get changed.
	OnSelchangeComboBotname();

	// restore the item in the previously select items spot, unless past the end, then select the last one
	u32 nNumTitleItems = m_oTitle.GetCount();
	if( uCurTitleIdx >= nNumTitleItems ) {
		uCurTitleIdx = nNumTitleItems - 1;
	}
	m_oTitle.SetCurSel( uCurTitleIdx );
	UpdateData( VARS_TO_CONTROLS );
	OnSelchangeComboTitle();

	m_bDataNeedsSaving = TRUE;
}

void CBotTalkinatorDlg::OnOK() {
	//OnCancel();
}

void CBotTalkinatorDlg::OnButtonSave() {
	
	UpdateData( CONTROLS_TO_VARS );

	if( m_oDialogueList.SaveToFile( m_sDialogueFile ) ) {
		// the file was saved successfully
		m_bDataNeedsSaving = FALSE;
	}
}

void CBotTalkinatorDlg::OnButtonOptions() {
	COptionsWnd oOptDialog;

	// ask the user if they would like to save their data before entering the options menu
	if( m_bDataNeedsSaving ) {
		if( AfxMessageBox( "Changes have been made.\nIf the settings are changed, you will lose your work.\nDo you want to save?", MB_YESNO ) == IDYES ) {
			OnButtonSave();			
		}
	}

	if( oOptDialog.DoGetOptions( this ) ) {
		// changes were made, they were saved into the settings file and our vars were updated, now shutdown and restart the app gracefully
		
		// shutdown the gameloop and free all of our existing data
		Shutdown();

		UpdateData( CONTROLS_TO_VARS );

		// reset all major parts of the app
		InitApplication();	
	}
}

void CBotTalkinatorDlg::OnCancel() {

	if( m_bDataNeedsSaving ) {
		if( AfxMessageBox( "Changes have been made.\nDo you want to save?", MB_YESNO ) == IDYES ) {
			OnButtonSave();			
		}
		m_bDataNeedsSaving = FALSE;
	}

	Shutdown();

	::RemoveProp( GetSafeHwnd(), BotTalkinator_pszPropName );

	DeleteCriticalSection( &m_CriticalSection );

	// save the name of the current bot and current title
	CSettings &Settings = CSettings::GetCurrent();

	int nCurSel = m_oSelectorComboBotName.GetCurSel();
	if( nCurSel >= 0 ) {
		m_oSelectorComboBotName.GetLBText( nCurSel, Settings.m_sLastSelectedBot );
	}
	nCurSel = m_oTitle.GetCurSel();
	if( nCurSel >= 0 ) {
		m_oTitle.GetLBText( nCurSel, Settings.m_sLastSelectedTitle );
	}

	Settings.SaveCommonDataOutToFile();		

	CDialog::OnCancel();
}

void CBotTalkinatorDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar ) {

	if( pScrollBar == GetDlgItem(IDC_SLIDER_TIMEPOS) ) 	{
		m_fPlayTimePos = ((CSliderCtrl *)pScrollBar)->GetPos() * (1.0f / fSliderRes) + m_fBankStart;
		m_oEditorListGesture.SetPtrPos(m_fPlayTimePos);
		RefreshSlider(&m_oTempDI);

	} else if( pScrollBar == GetDlgItem(IDC_SCROLLBAR_BANK) ) {

		switch( nSBCode ) {

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			// nPos is only valid if nSBCode is one of the above values.
			m_oEditorScrollBank.SetScrollPos(nPos, TRUE);
			m_fBankStart = (f32)(nPos) * (1.0f / fScrollRes);
			if( m_fPlayTimePos < m_fBankStart ) {
				m_oEditorTimePos.SetPos(0);
			} else if( m_fPlayTimePos > (m_fBankStart + m_fBankWidth) ) {
				m_oEditorTimePos.SetPos((int)(m_fBankWidth * fSliderRes));
			} else {
				m_oEditorTimePos.SetPos((int)((m_fPlayTimePos - m_fBankStart) * (fSliderRes)));
			}
			UpdateData( VARS_TO_CONTROLS );
			OnUpdateBankPos();
			break;
			
		case SB_LINELEFT:
			nPos = m_oEditorScrollBank.GetScrollPos();
			if( nPos > 0 ) {
				--nPos;
				m_oEditorScrollBank.SetScrollPos(nPos, TRUE);
				m_fBankStart = (f32)(nPos) * (1.0f / fScrollRes);
				
				UpdateData( VARS_TO_CONTROLS );
				OnUpdateBankPos();
			}
			break;

		case SB_LINERIGHT:
			nPos = m_oEditorScrollBank.GetScrollPos();
			if( nPos <= (u32)((m_oTempDI.m_fTotalTime - m_fBankWidth) * fScrollRes) ) {
				++nPos;
				m_oEditorScrollBank.SetScrollPos(nPos, TRUE);
				m_fBankStart = (f32)(nPos) * (1.0f / fScrollRes);
				UpdateData( VARS_TO_CONTROLS );
				OnUpdateBankPos();
			}
			break;
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CBotTalkinatorDlg::OnButtonPlay()
{
	UpdateData( CONTROLS_TO_VARS );
	m_oEditorDialogue.GetWindowText( m_oTempDI.m_strDialogue );
	m_oEditorEditWAVFile.GetWindowText( m_oTempDI.m_strWAVFile );
	m_oTempDI.m_uMilVoice = m_oEditorComboMilVoice.GetCurSel();
	m_oTempDI.CalculateTime();
	ShowPreviewData( &m_oTempDI );
	if( m_oTempDI.m_bUseWAV ) {
		// only start playing if the play cursor is very near or ahead of the play point
		f32 fSecsTillWavStart = m_fPlayTimePos - m_oTempDI.m_fWavStartOffset;

		if( fSecsTillWavStart >= 0.0f && fSecsTillWavStart <= (1.0f/60.0f) ) {
			if( m_oTempDI.m_strWAVFile.Right(4) == ".wav" ) {
				PlaySound( m_oTempDI.m_strWAVFile, NULL, SND_FILENAME | SND_ASYNC );
			} else {
				FSndFx_FxHandle_t hFXHandle = fsndfx_GetFxHandle( m_oTempDI.m_strWAVFile );
				fsndfx_Play2D(hFXHandle);
			}
			m_bWavStarted = TRUE;
		}
	}
	m_pLastSoundPlayed = NULL;
	SetAreaActive( AREA_EDITOR, FALSE );
	m_oViewPause.EnableWindow( TRUE );
	m_oEditorButtonPlayFrom.EnableWindow( TRUE );
	m_eViewMode = VIEWMODE_PLAY;
}

void CBotTalkinatorDlg::OnButtonPause() {

	if( m_oTempDI.m_bUseWAV ) {
		PlaySound( NULL, NULL, NULL );
		CFAudioEmitter::DestroyAll();
		m_bWavStarted = FALSE;
	}
	m_eViewMode = VIEWMODE_PAUSE;
	SetAreaActive( AREA_EDITOR, TRUE );
	m_oViewPause.EnableWindow( FALSE );
}

void CBotTalkinatorDlg::OnCheckEditorUsewav() {

	if( m_oEditorCheckUseWAV.GetCheck() ) {
		m_oTempDI.m_bUseWAV = TRUE;
		m_oEditorEditWAVFile.EnableWindow( TRUE );
		m_oEditorComboMilVoice.EnableWindow( FALSE );
		m_buttonPickSfx.EnableWindow( TRUE );
		m_ctrlLenEdit.EnableWindow( TRUE );
		m_ctrlOffsetEdit.EnableWindow( TRUE );
	} else {
		m_oTempDI.m_bUseWAV = FALSE;
		m_oEditorEditWAVFile.EnableWindow( FALSE );
		m_oEditorComboMilVoice.EnableWindow( TRUE );
		m_buttonPickSfx.EnableWindow( FALSE );
		m_ctrlLenEdit.EnableWindow( FALSE );
		m_ctrlOffsetEdit.EnableWindow( FALSE );
	}
}

// Gets called when the Bot Name ComboBox gets changed.
// Selection should be set *before* this function gets executed.
// This function reloads and initializes with its first entry the title combo box, then refreshes
//   the preview data display.
void CBotTalkinatorDlg::OnSelchangeComboBotname() {
	UpdateData( CONTROLS_TO_VARS );

	s32 nIndex = m_oSelectorComboBotName.GetCurSel();
	FASSERT(nIndex != -1);
	CDialogueEntryN *poDE = (CDialogueEntryN *)m_oSelectorComboBotName.GetItemDataPtr( nIndex );

	m_pCurDE = poDE;
	RefreshTitleCombo(poDE);
	m_oTitle.SetCurSel( 0 );

	UpdateData( VARS_TO_CONTROLS );
	OnSelchangeComboTitle();
}

void CBotTalkinatorDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( pScrollBar == GetDlgItem(IDC_SCROLLBAR_GEST) ) {
		m_oEditorListGesture.OnVScrollExt(nSBCode, nPos, pScrollBar);
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CBotTalkinatorDlg::OnButtonPlayfrom() {

	UpdateData( CONTROLS_TO_VARS );
	m_fPlayTimePos = m_fPlayFromPos;
	m_oEditorDialogue.GetWindowText(m_oTempDI.m_strDialogue);
	m_oEditorEditWAVFile.GetWindowText(m_oTempDI.m_strWAVFile);
	m_oTempDI.m_uMilVoice = m_oEditorComboMilVoice.GetCurSel();
	m_oTempDI.CalculateTime();
	ShowPreviewData(&m_oTempDI);
	if( m_oTempDI.m_bUseWAV && ((m_eViewMode == VIEWMODE_PLAYFROM) || (m_eViewMode == VIEWMODE_PLAY)) ) {
		PlaySound(NULL, NULL, NULL);
		CFAudioEmitter::DestroyAll();
		m_bWavStarted = FALSE;
	}

	if( m_oTempDI.m_bUseWAV ) {

		// only start playing if the play cursor is very near or ahead of the play point
		f32 fSecsTillWavStart = m_fPlayTimePos - m_oTempDI.m_fWavStartOffset;

		if( fSecsTillWavStart >= 0.0f && fSecsTillWavStart <= (1.0f/60.0f) ) {
			if( m_oTempDI.m_strWAVFile.Right(4) == ".wav" ) {
				PlaySound(m_oTempDI.m_strWAVFile, NULL, SND_FILENAME | SND_ASYNC);
			} else {
				FSndFx_FxHandle_t hFXHandle = fsndfx_GetFxHandle(m_oTempDI.m_strWAVFile);
				fsndfx_Play2D( hFXHandle );
			}
			m_bWavStarted = TRUE;
		}
	}
	m_pLastSoundPlayed = NULL;
	SetAreaActive( AREA_EDITOR, FALSE );
	m_oViewPause.EnableWindow( TRUE );
	m_oEditorButtonPlayFrom.EnableWindow( TRUE );
	m_eViewMode = VIEWMODE_PLAYFROM;
}

void CBotTalkinatorDlg::OnButtonSetplayfrom() {
	m_fPlayFromPos = m_fPlayTimePos;
	m_oEditorListGesture.SetPlayFromPos(m_fPlayFromPos);
}

void CBotTalkinatorDlg::OnCheckEditorExtdrag() {
	UpdateData( CONTROLS_TO_VARS );
	m_oEditorListGesture.SetExtendedDrag(m_oEditorCheckExtDrag.GetCheck());
}

void CBotTalkinatorDlg::ResetPointers() {
	// JUSTIN: Check this.
	m_fPlayTimePos = 0.0f;
	m_oEditorTimePos.SetPos(0);
	m_oEditorListGesture.SetPtrPos(0.0f);
	m_fPlayFromPos = 0.0f;
	m_oEditorListGesture.SetPlayFromPos(0.0f);
	m_fBankStart = 0.0f;
	m_oEditorScrollBank.SetScrollPos(0, TRUE);
	m_oEditorListGesture.SetBankPos(0.0f);
}

void CBotTalkinatorDlg::OnCheckTalkloop() {

	if( m_oCheckTalkLoop.GetCheck() ) {
		m_oTempDI.m_bUseTalkLoop = TRUE;
	} else {
		m_oTempDI.m_bUseTalkLoop = FALSE;
	}
}

void CBotTalkinatorDlg::OnButtonExport() {

	CString strExportFileName;
	m_oEditExFile.GetWindowText( strExportFileName );
	m_oTempDI.ExportToFile( strExportFileName );
}

void CBotTalkinatorDlg::OnPickSfx() {
	
	CPickSfxDlg Dlg;

	Dlg.m_sUserSelection = m_oTempDI.m_strWAVFile;
	Dlg.m_hSfxBank = m_oTempDI.m_pDE->m_poCharInfo->m_hBankHandle;
	if( Dlg.DoModal() == IDOK ) {
		m_oTempDI.m_strWAVFile = Dlg.m_sUserSelection;
		m_oEditorEditWAVFile.SetWindowText( m_oTempDI.m_strWAVFile );

		// update the total time from the wav file
		m_oTempDI.CalculateTime();
		// make sure that the wav offset is ok, based on the new total time
		FMATH_CLAMP( m_oTempDI.m_fWavStartOffset, 0.0f, m_oTempDI.m_fTotalTime - m_oTempDI.m_fWavSecs );
		CString sValue;
		sValue.Format( "%.3f", m_oTempDI.m_fWavStartOffset );
		m_ctrlOffsetEdit.SetWindowText( sValue );

		ShowPreviewData( &m_oTempDI );
	}
}

void CBotTalkinatorDlg::OnKillfocusEditExfile() {
	
	UpdateData( CONTROLS_TO_VARS );
	m_oEditExFile.GetWindowText( m_oTempDI.m_strExportFile );

	CString sTemp = m_oTempDI.m_strExportFile;
	sTemp.MakeLower();
	BOOL bUpdateWindowText = FALSE;
	// make sure the user didn't type in .csv
	s32 nIndex = sTemp.Find( ".csv" );
	if( nIndex >= 0 ) {
		u32 nCharsToDelete = sTemp.GetLength() - nIndex;
		sTemp.Delete( nIndex, nCharsToDelete );
		m_oTempDI.m_strExportFile.Delete( nIndex, nCharsToDelete );
		bUpdateWindowText = TRUE;
	}
	// see if the length is too long
	if( m_oTempDI.m_strExportFile.GetLength() > 11 ) {
		MessageBox( _T("The .csv export name must be 11 characters or less.\nOnly the first 11 characters will be used."), 
					_T("Filename Too Long"),
					MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1 );
		m_oTempDI.m_strExportFile.Delete( 11, m_oTempDI.m_strExportFile.GetLength() - 11 );
		
		bUpdateWindowText = TRUE;
	}

	if( bUpdateWindowText ) {
		m_oEditExFile.SetWindowText( m_oTempDI.m_strExportFile );
	}
}

void CBotTalkinatorDlg::OnPickCsv() {
	
	UpdateData( CONTROLS_TO_VARS );

	CSettings &rSettings = CSettings::GetCurrent();
	CString sDefaultName = rSettings.GetOutputDir();
	if( m_oTempDI.m_strExportFile.IsEmpty() ) {
		sDefaultName += "\\export.csv";
	} else {
		sDefaultName += '\\';
		sDefaultName += m_oTempDI.m_strExportFile;
		sDefaultName += ".csv";
	}
	
	CFileDialog dlg( FALSE,
					 ".csv",
					 sDefaultName,
					 OFN_EXTENSIONDIFFERENT | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
					 "CSV Files (*.csv)|*.csv||",
					 this );
	dlg.m_ofn.lpstrTitle = "Select CSV Filename To Export To";
	if( dlg.DoModal() == IDOK ) {
		// make sure that the user picked the correct extension
		if( dlg.GetFileExt().CompareNoCase( "csv" ) != 0 ) {
			MessageBox( _T("All exported files will be CSV files.\nYour typed extension will be ignored." ), 
						_T("BotTalkinator"),
						MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		}
		// see if path has changed
		CString sPath = dlg.GetPathName();
		s32 nIndex = sPath.ReverseFind( '\\' );
		if( nIndex >= 0 ) {
			sPath.Delete( nIndex, sPath.GetLength() - nIndex );
		}
		if( sPath.CompareNoCase( rSettings.m_sOutputDir ) != 0 ) {
			// the paths are different, tell the user
			MessageBox( _T("All exported CSV files will be saved to the output directory.\nYour typed path will be ignored." ), 
						_T("BotTalkinator"),
						MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		}

		// save the new name
		m_oTempDI.m_strExportFile = dlg.GetFileTitle();

		// see if the length is too long
		if( m_oTempDI.m_strExportFile.GetLength() > 11 ) {
			MessageBox( _T("The .csv export name must be 11 characters or less.\nOnly the first 11 characters will be used."), 
						_T("Filename Too Long"),
						MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1 );
			m_oTempDI.m_strExportFile.Delete( 11, m_oTempDI.m_strExportFile.GetLength() - 11 );
		}

		// update the edit box with the new name
		m_oEditExFile.SetWindowText( m_oTempDI.m_strExportFile );
	}
}

BOOL CBotTalkinatorDlg::InitApplication() {
	CString strTemp;

	CSettings &oSettings = CSettings::GetCurrent();
	
	// Get the master file setting set up properly.
	m_sMasterFile = oSettings.GetMasterFile();
	if( m_sMasterFile.IsEmpty() ) {
		MessageBox( _T("You must select a master file before BotTalkinator\ncan be used.  Click OK and then select\na master file."), 
					_T("BotTalkinator Error: No Master File"),
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
			m_sMasterFile.MakeLower();
			oSettings.m_sMasterFile = m_sMasterFile;
			oSettings.SaveCommonDataOutToFile();
		} else {
			// no master file, no BotTalkinator
			PostMessage( WM_QUIT );
			return TRUE;
		}
	}

	// Do our Fang Engine set up here.
	fang_Init();
	Fang_ConfigDefs.pszFile_MasterFilePathName = m_sMasterFile;
	Fang_ConfigDefs.nRes_HeapBytes = (64 * 1024 * 1024);
	Fang_ConfigDefs.nAudio_MaxSoundBytes = (96 * 1024 * 1024);

	if( !fang_Startup() ) {
		AfxMessageBox( "Error on fang_Startup()!" );
		PostMessage( WM_QUIT );
		return TRUE;
	}

	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;
	
	// Get the video mode settings set up properly.
	CVidMode VidMode;
	if( !VidMode.GetVidMode( &m_oGameLoopData.oVidSettings ) ) {
		// Video mode not selected... 
		MessageBox( "You need to set the video mode before BotTalkinator may be used.\nClick Ok and then click the Video button.", "BotTalkinator Error: Set Video Mode" );
		m_bValidVideoMode = FALSE;
		if( VidMode.DoModal() == IDOK ) {
			VidMode.GetVidMode( &m_oGameLoopData.oVidSettings );
		} else {
			// no video mode, no BotTalkinator
			PostMessage( WM_QUIT );
			return TRUE;
		}
	}

	// If we get here, our video mode is okay.
	strcpy( m_oGameLoopData.oVidSettings.szWindowTitle, "BotTalkinator Viewing Window" );
	m_oGameLoopData.oVidSettings.bAllowPowerSuspend = TRUE;
	m_oGameLoopData.oVidSettings.hInstance = theApp.m_hInstance;
	m_oGameLoopData.oVidSettings.hWnd = m_oViewStaticFrame.m_hWnd;// Change this to NULL to use a separate window.
	m_oGameLoopData.oVidSettings.nIconIDI = IDR_MAINFRAME;
	m_oGameLoopData.oVidSettings.pFcnSuspend = NULL;
	m_oGameLoopData.pDlg = this;
	m_bValidVideoMode = TRUE;
	
	// Init the audio system.
	FAudio_Init_t oAudioInit;
	fang_MemZero( &oAudioInit, sizeof( oAudioInit ) );

	oAudioInit.uMaxListeners      = 1;
	oAudioInit.uMaxEmitters       = 25;
	oAudioInit.uMaxStreams        = 2;
	oAudioInit.uMaxBanks          = 20;
	oAudioInit.uMaxPriorityLevels = 1;
	oAudioInit.uPriorityLimits    = 0;
	oAudioInit.paoPriorityLimits  = NULL;
	oAudioInit.DX8ONLY_ohWnd      = (u32)m_hWnd;
	oAudioInit.DX8ONLY_ohInstance = (u32)theApp.m_hInstance;
	
	if( FAUDIO_ERROR == faudio_Install( &oAudioInit ) ) {
		AfxMessageBox( "Error installing sound system." );
		return FALSE;
	}

	// setup our paths
	m_sInputPath = oSettings.GetInputDir();
	CString sOutputPath = oSettings.GetOutputDir();
	m_sAnimFile = m_sInputPath + BOT_TALK_GESTURES_FILENAME;
	m_sSoundFile = m_sInputPath + BOT_TALK_VOCAB_FILENAME;
	m_sDialogueFile = m_sInputPath + BOT_TALK_DIALOGUES_FILENAME;
	if( m_sInputPath.IsEmpty() ||
		sOutputPath.IsEmpty() ||
		(_access( m_sAnimFile, 0) != 0) ||
		(_access( m_sSoundFile, 0) != 0) ||
		(_access( m_sDialogueFile, 0) != 0) ) {
		
		strTemp.Format( "The application settings need to be setup properly.\n\n"
						"Please set an input directory (which must contain the 3 input files)\n"
						"and an output directory for exporting to.\n" );
		MessageBox( strTemp, "Settings Error", MB_OK );

		COptionsWnd oOptDialog;
		if( oOptDialog.DoGetOptions( this ) == FALSE ) {
			// the user decided not to setup the app, quit out
			PostMessage( WM_COMMAND, IDCANCEL, 0 );
			return TRUE;
		}
	}

	// Get the CDialogueList member set up properly.
	if( !m_oDialogueList.Init( 512, 512 ) ) {		// 256 gestures max, 256 sounds max (all characters in both cases)
		AfxMessageBox("Error initializing CDialogueList system.");
		// There's no way to recover from this error.
		PostMessage( WM_COMMAND, IDCANCEL, 0 );
		return TRUE;
	}

	// read the gestures file
	if( !m_oDialogueList.LoadCharInfoFromFile( m_sAnimFile ) ) {
		strTemp.Format( "File '%s' is not a valid BotTalk Gestures file.\nPlease check the file and try again.", m_sAnimFile);
		MessageBox( strTemp, "Error reading file", MB_OK );
		// No gestures file, no BotTalkinator.
		PostMessage( WM_COMMAND, IDCANCEL, 0 );
		return TRUE;
	}

	// read the sounds file
	if( !m_oDialogueList.LoadSoundsFromFile(m_sSoundFile) ) {
		strTemp.Format("File '%s' is not a valid BotTalk Vocabulary file.\nPlease check the file and try again.", m_sSoundFile);
		MessageBox(strTemp, "Error reading file", MB_OK);
		// No gestures file, no BotTalkinator.
		PostMessage( WM_COMMAND, IDCANCEL, 0 );
		return TRUE;
	}

	// Ensure that the sound instances can find where we're keeping our sound lists.
	CDialogueInstN::m_apoSoundList[0] = &m_oDialogueList.m_aoSoundList[0];
	CDialogueInstN::m_apoSoundList[1] = &m_oDialogueList.m_aoSoundList[1];
	CDialogueInstN::m_apoSoundList[2] = &m_oDialogueList.m_aoSoundList[2];

	// read the dialogues file
	if( !m_oDialogueList.LoadDialoguesFromFile( m_sDialogueFile ) ) {
		strTemp.Format("File '%s' is not a valid BotTalk Dialogues file.\nPlease check the file and try again.", m_sDialogueFile);
		MessageBox(strTemp, "Error reading file", MB_OK);
		// No gestures file, no BotTalkinator.
		PostMessage(WM_COMMAND, IDCANCEL, 0);
		return(TRUE);
	}
	
	// Initialize system values.
	ResetPointers();
	m_fBankWidth = 10.0f;
	m_bDataNeedsSaving = FALSE;
	
	//  Initialize our controls.
	m_oEditorComboMilVoice.ResetContent();
	m_oEditorComboMilVoice.AddString( "Mil Voice 1" );
	m_oEditorComboMilVoice.AddString( "Mil Voice 2" );
	m_oEditorComboMilVoice.AddString( "Mil Voice 3" );
	m_oEditorComboMilVoice.SetCurSel( 0 );

	RefreshBotNameCombo();

	// set the current bot
	int nCurSel = 0;
	if( !oSettings.m_sLastSelectedBot.IsEmpty() ) {
		nCurSel = m_oSelectorComboBotName.FindString( -1, oSettings.m_sLastSelectedBot );
		if( nCurSel < 0 ) {
			nCurSel = 0;
		}
	}
	m_oSelectorComboBotName.SetCurSel( nCurSel );
	OnSelchangeComboBotname();

	// set the current title
	if( !oSettings.m_sLastSelectedTitle.IsEmpty() ) {
		nCurSel = m_oTitle.FindString( -1, oSettings.m_sLastSelectedTitle );
		if( nCurSel >= 0 ) {
			// only change the selection if we found the last string
			m_oTitle.SetCurSel( nCurSel );
			UpdateData( VARS_TO_CONTROLS );
			OnSelchangeComboTitle();		
		}
	}	

	m_oEditorListGesture.SetBankWidth( m_fBankWidth );
	
	m_eViewMode = VIEWMODE_PAUSE;

	// post a message to start the gameloop running
	PostMessage( BOTTALKINATOR_MSG_INITCOMPLETE );

	return TRUE;
}

void CBotTalkinatorDlg::OnKillfocusEditLen() {

	UpdateData( CONTROLS_TO_VARS );

	CString sValue;
	m_ctrlLenEdit.GetWindowText( sValue );

	if( sValue.IsEmpty() ) {
		sValue = "0.000";
	} 

	f32 fValue = (f32)atof( sValue );
	FMATH_CLAMP( fValue, 0.0f, (30.0f * 60.0f) );
	sValue.Format( "%.3f", fValue );
	m_ctrlLenEdit.SetWindowText( sValue );
	// grab the float value from our new string
	fValue = (f32)atof( sValue );

	m_oTempDI.m_fMinTotalTime = fValue;
	// update the total time from the wav file
	m_oTempDI.CalculateTime();
	// make sure that the wav offset is ok, based on the new total time
	FMATH_CLAMP( m_oTempDI.m_fWavStartOffset, 0.0f, m_oTempDI.m_fTotalTime - m_oTempDI.m_fWavSecs );
	sValue.Format( "%.3f", m_oTempDI.m_fWavStartOffset );
	m_ctrlOffsetEdit.SetWindowText( sValue );

	ShowPreviewData( &m_oTempDI );
}

void CBotTalkinatorDlg::OnKillfocusEditOffset() {
	
	UpdateData( CONTROLS_TO_VARS );

	CString sValue;
	m_ctrlOffsetEdit.GetWindowText( sValue );

	if( sValue.IsEmpty() ) {
		sValue = "0.000";
	} 

	f32 fValue = (f32)atof( sValue );
	FMATH_CLAMP( fValue, 0.0f, m_oTempDI.m_fTotalTime - m_oTempDI.m_fWavSecs );
	sValue.Format( "%.3f", fValue );
	m_ctrlOffsetEdit.SetWindowText( sValue );
	// grab the float value from our new string
	fValue = (f32)atof( sValue );

	m_oTempDI.m_fWavStartOffset = fValue;
}

void CBotTalkinatorDlg::OnKillfocusEditTitle() {
	
	UpdateData( CONTROLS_TO_VARS );
	m_oEditorTitle.GetWindowText( m_oTempDI.m_strTitle );	
}

void CBotTalkinatorDlg::OnKillfocusEditDialogue() {

	UpdateData( CONTROLS_TO_VARS );
	m_oEditorDialogue.GetWindowText( m_oTempDI.m_strDialogue );
}

void CBotTalkinatorDlg::OnKillfocusEditEditorWavfile() {
	
	UpdateData( CONTROLS_TO_VARS );
	m_oEditorEditWAVFile.GetWindowText( m_oTempDI.m_strWAVFile );
	
	// update the total time from the wav file
	m_oTempDI.CalculateTime();
	ShowPreviewData( &m_oTempDI );
}

void CBotTalkinatorDlg::OnCheckEditorIs2d()
{
	if( m_oEditorCheckIs2d.GetCheck() ) {
		m_oTempDI.m_bIs2d = TRUE;
	} else {
		m_oTempDI.m_bIs2d = FALSE;
	}
}

