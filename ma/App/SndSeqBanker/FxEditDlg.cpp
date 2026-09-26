// FxEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SndSeqBanker.h"
#include "FxEditDlg.h"
#include "BankListItemInfo.h"
#include "fclib.h"
#include "fmath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFxEditDlg dialog


CFxEditDlg::CFxEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFxEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFxEditDlg)
	m_sFxName = _T("");
	m_nNumLoops = 0;
	m_sWavInfo = _T("");
	m_nVolume = 0;
	m_nPitch = 0;
	//}}AFX_DATA_INIT
}


void CFxEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFxEditDlg)
	DDX_Control(pDX, IDC_PITCH_SPIN, m_ctrlPitchSpinner);
	DDX_Control(pDX, IDC_VOLUME_SPIN, m_ctrlVolumeSpinner);
	DDX_Control(pDX, IDC_LOOP_SPIN, m_ctrlLoopSpinner);
	DDX_Control(pDX, IDC_WAV_COMBO, m_ctrlWavList);
	DDX_Text(pDX, IDC_FX_NAME, m_sFxName);
	DDV_MaxChars(pDX, m_sFxName, 15);
	DDX_Text(pDX, IDC_LOOPS, m_nNumLoops);
	DDX_Text(pDX, IDC_WAV_INFO, m_sWavInfo);
	DDX_Text(pDX, IDC_VOLUME, m_nVolume);
	DDX_Text(pDX, IDC_PITCH, m_nPitch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFxEditDlg, CDialog)
	//{{AFX_MSG_MAP(CFxEditDlg)
	ON_CBN_SELCHANGE(IDC_WAV_COMBO, OnSelchangeWavCombo)
	ON_EN_KILLFOCUS(IDC_VOLUME, OnKillfocusVolume)
	ON_EN_KILLFOCUS(IDC_LOOPS, OnKillfocusLoops)
	ON_EN_KILLFOCUS(IDC_PITCH, OnKillfocusPitch)
	ON_EN_KILLFOCUS(IDC_FX_NAME, OnKillfocusFxName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFxEditDlg message handlers

void CFxEditDlg::OnCancel() {
		
	CDialog::OnCancel();
}

void CFxEditDlg::OnOK() {

	UpdateData( CONTROLS_TO_VARS );

	// make sure that the new name doesn't already exist in the seq bank
	m_sFxName.TrimLeft();
	m_sFxName.TrimRight();
	SeqBank_Fx_t *pExistingFx = m_pSeqBank->GetFxByName( m_sFxName );
	if( pExistingFx &&
		(u32)pExistingFx != (u32)m_pFx ) {
		// the name already exists
		CString sError;
		sError.Format( "The name:\n"
					   "%s\n"
					   "already exists in this bank.\n\n"
					   "Sound fx names must be unique.\n", m_sFxName );
		MessageBox( sError, _T("Error: Duplicate Name"), MB_ICONEXCLAMATION|MB_OK|MB_DEFBUTTON1);
		
		((CWnd *)GetDlgItem( IDC_FX_NAME ))->SetFocus();
		return;
	}

	CString sCurWav;
	m_ctrlWavList.GetLBText( m_ctrlWavList.GetCurSel(), sCurWav );
	
	// update the fx
	fclib_strncpy( m_pFx->szFxName, m_sFxName, SEQ_BANK_MAX_FX_NAME_LEN );
	m_pFx->paCmds->TypeData.Play.nNumLoops = m_nNumLoops;
	fclib_strncpy( m_pFx->paCmds->TypeData.Play.szFilename, sCurWav, WAV_BANK_MAX_WAV_FILENAME_LEN );
	m_pFx->paCmds->TypeData.Play.nVolPercent = m_nVolume;
	m_pFx->paCmds->TypeData.Play.nPitchPercent = m_nPitch;
		
	CDialog::OnOK();
}

int CFxEditDlg::DoModal() {
		
	return CDialog::DoModal();
}

void CFxEditDlg::OnSelchangeWavCombo() {

	UpdateData( CONTROLS_TO_VARS );

	// update the wav info
	int nCurSel = m_ctrlWavList.GetCurSel();
	CString sWavName;
	m_ctrlWavList.GetLBText( nCurSel, sWavName );

	WavBank_Entry_t *pEntry = m_pWavBank->GetEntryByName( sWavName );
	if( pEntry ) {
		UpdateWavInfo( pEntry );
	}

	UpdateData( VARS_TO_CONTROLS );
}

BOOL CFxEditDlg::OnInitDialog() {

	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_ctrlWavList.ResetContent();
	u32 i, nNumWavs;
	nNumWavs = m_pWavBank->GetNumEntries();
	WavBank_Entry_t *pEntry;
	for( i=0; i < nNumWavs; i++ ) {
		pEntry = m_pWavBank->GetEntryByIndex( i );
		m_ctrlWavList.AddString( pEntry->szFilename );
	}
	
	// set the current wav selection
	pEntry = m_pWavBank->GetEntryByName( m_pFx->paCmds->TypeData.Play.szFilename );
	m_ctrlWavList.SelectString( 0, pEntry->szFilename );

	// update our info field
	UpdateWavInfo( pEntry );

	// setup our fx name
	m_sFxName = m_pFx->szFxName;

	// setup our number of loops
	m_nNumLoops = m_pFx->paCmds->TypeData.Play.nNumLoops;

	// setup our loop spinner	
	m_ctrlLoopSpinner.SetRange32( 0, 65535 );
	m_ctrlLoopSpinner.SetPos( m_nNumLoops );

	// setup our volume
	m_nVolume = m_pFx->paCmds->TypeData.Play.nVolPercent;

	// setup our volume spinner	
	m_ctrlVolumeSpinner.SetRange32( 1, 100 );
	m_ctrlVolumeSpinner.SetPos( m_nVolume );

	// setup our pitch
	m_nPitch = m_pFx->paCmds->TypeData.Play.nPitchPercent;

	// setup our pitch spinner	
	m_ctrlPitchSpinner.SetRange32( -1000, 1000 );
	m_ctrlPitchSpinner.SetPos( m_nPitch );

	// set the window heading text
	CString sHeading;
	sHeading.Format( "Edit Fx - %s", m_sFxName );
	SetWindowText( sHeading );

	UpdateData( VARS_TO_CONTROLS );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFxEditDlg::UpdateWavInfo( WavBank_Entry_t *pEntry ) {

	CString sSecs;
	CBankListItemInfo::ConvertSecsToString( pEntry->fSecsOfAudio, sSecs );

	m_sWavInfo.Format( "%d Hz\n"
					   "%d channels\n"
					   "%s", pEntry->nSamplesPerSec, pEntry->nNumChannels, sSecs );
}

void CFxEditDlg::OnKillfocusVolume() {
	
	UpdateData( CONTROLS_TO_VARS );
	
	FMATH_CLAMP( m_nVolume, 1, 100 );

	UpdateData( VARS_TO_CONTROLS );
}

void CFxEditDlg::OnKillfocusLoops() {
	
	UpdateData( CONTROLS_TO_VARS );

	FMATH_CLAMP( m_nNumLoops, 0, 65535 );

	UpdateData( VARS_TO_CONTROLS );	
}

void CFxEditDlg::OnKillfocusPitch() {
	
	UpdateData( CONTROLS_TO_VARS );

	FMATH_CLAMP( m_nPitch, -1000, 1000 );

	UpdateData( VARS_TO_CONTROLS );	
}

void CFxEditDlg::OnKillfocusFxName() {
	
	UpdateData( CONTROLS_TO_VARS );
}
