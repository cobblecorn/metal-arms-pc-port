// NumParticlesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Spit.h"
#include "NumParticlesDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

static const double _Min = 0.001f;

/////////////////////////////////////////////////////////////////////////////
// CNumParticlesDlg dialog

IMPLEMENT_DYNCREATE(CNumParticlesDlg, CDialog)

CNumParticlesDlg::CNumParticlesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNumParticlesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNumParticlesDlg)
	//}}AFX_DATA_INIT
}


void CNumParticlesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNumParticlesDlg)
	DDX_Control(pDX, IDC_EDIT_MAX_BURSTS_PER_SEC, m_ctrlMaxBurstsPerSec);
	DDX_Control(pDX, IDC_EDIT_SECS_TO_EMIT, m_ctrlSecsToEmit);
	DDX_Control(pDX, IDC_EDIT_MIN_BURSTS_PER_SEC, m_ctrlMinBurstsPerSec);
	DDX_Control(pDX, IDC_EDIT_MIN_DELAY_SECS, m_ctrlMinDelaySecs);
	DDX_Control(pDX, IDC_EDIT_MAX_DELAY_SECS, m_ctrlMaxDelaySecs);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_MAX_PARTICLES, m_ctrlMaxParticles, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_MIN_PARTICLES, m_ctrlMinParticles, 0);
}


BEGIN_MESSAGE_MAP(CNumParticlesDlg, CDialog)
	//{{AFX_MSG_MAP(CNumParticlesDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_SECS_TO_EMIT, OnKillfocusEditSecsToEmit)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_PARTICLES, OnKillfocusEditMinParticles)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_PARTICLES, OnKillfocusEditMaxParticles)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_BURSTS_PER_SEC, OnKillfocusEditMinBurstsPerSec)
	ON_EN_CHANGE(IDC_EDIT_MAX_PARTICLES, OnChangeEditMaxParticles)
	ON_EN_CHANGE(IDC_EDIT_MIN_PARTICLES, OnChangeEditMinParticles)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_BURSTS_PER_SEC, OnKillfocusEditMaxBurstsPerSec)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_DELAY_SECS, OnKillfocusEditMinDelaySec)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_DELAY_SECS, OnKillfocusEditMaxDelaySec)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumParticlesDlg message handlers

BOOL CNumParticlesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlMinParticles.SetRange( 1, 128 );
	
	m_ctrlMaxParticles.SetRange( 1, 128 );

	m_ctrlSecsToEmit.SetMaxWholeDigits( 4 );
	m_ctrlSecsToEmit.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlSecsToEmit.AllowNegative( FALSE );// do this last
		
	m_ctrlMinBurstsPerSec.SetMaxWholeDigits( 2 );
	m_ctrlMinBurstsPerSec.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMinBurstsPerSec.AllowNegative( FALSE );// do this last

	m_ctrlMaxBurstsPerSec.SetMaxWholeDigits( 2 );
	m_ctrlMaxBurstsPerSec.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMaxBurstsPerSec.AllowNegative( FALSE );// do this last

	m_ctrlMinDelaySecs.SetMaxWholeDigits( 4 );
	m_ctrlMinDelaySecs.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMinDelaySecs.AllowNegative( FALSE );// do this last

	m_ctrlMaxDelaySecs.SetMaxWholeDigits( 4 );
	m_ctrlMaxDelaySecs.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMaxDelaySecs.AllowNegative( FALSE );// do this last
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CNumParticlesDlg::PreTranslateMessage( MSG* pMsg ) {
	// Detect the Enter (Return) key, and pass the message to the dialog.
	// This disables the "default button" functionality normally
	// found in dialogs and property sheets, and allows the user to hit
	// Enter to "commit" the changes (s)he is making to an edit field.
	if( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN ) {
		CWnd* pControl = GetFocus();
		if( pControl ) {
			char pszString[256];
			if( ::GetClassName( pControl->m_hWnd, pszString, 256 ) ) {
				// see if we have an edit control
				CString s;
				s = "edit";
				if( !s.CompareNoCase( pszString ) ) {
					// the strings matched, we have an edit box, highlight the entire line and kill focus
					
					// Tell the dialog that Enter was pressed, and pass the ID
					// of the edit control which has the focus.
					PostMessage(WM_COMMAND, MAKELONG(pControl->GetDlgCtrlID(), EN_KILLFOCUS), (LPARAM) pControl->m_hWnd);
					
					// Highlight all text in the edit control.
					pControl->PostMessage( EM_SETSEL, 0, -1 );

					return TRUE;
				} 
			}
		}
	}
	if( pMsg->message == WM_KEYDOWN ) {
		if( pMsg->wParam == VK_ESCAPE ) {
			// Esc key should not kill the dialog, do nothing
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CNumParticlesDlg::OnOK() {
	//OnCancel();
}

void CNumParticlesDlg::OnKillfocusEditSecsToEmit() {
	f32 fSecsToEmit = (f32)m_ctrlSecsToEmit.GetTextAsDouble();

	m_GameData.fSecsToEmit = fSecsToEmit;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CNumParticlesDlg::OnKillfocusEditMinParticles() {
	int nMinNumParticles = m_ctrlMinParticles.GetPos();
	int nMaxNumParticles = m_ctrlMaxParticles.GetPos();

	if( nMinNumParticles > nMaxNumParticles ) {
		nMaxNumParticles = nMinNumParticles;
		m_ctrlMaxParticles.SetPos( nMaxNumParticles );
	}
	
	m_GameData.NumPerBurst.x = (f32)nMinNumParticles;
	m_GameData.NumPerBurst.y = (f32)(nMaxNumParticles - nMinNumParticles);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CNumParticlesDlg::OnKillfocusEditMaxParticles() {
	int nMinNumParticles = m_ctrlMinParticles.GetPos();
	int nMaxNumParticles = m_ctrlMaxParticles.GetPos();

	if( nMaxNumParticles < nMinNumParticles ) {
		nMinNumParticles = nMaxNumParticles;
		m_ctrlMinParticles.SetPos( nMinNumParticles );
	}	
	
	m_GameData.NumPerBurst.x = (f32)nMinNumParticles;
	m_GameData.NumPerBurst.y = (f32)(nMaxNumParticles - nMinNumParticles);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

LONG CNumParticlesDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlMaxParticles.GetDlgCtrlID() ) {
		OnKillfocusEditMaxParticles();	
	} else if( wParam == m_ctrlMinParticles.GetDlgCtrlID() ) {
		OnKillfocusEditMinParticles();
	}

	return 0;
}

void CNumParticlesDlg::SetData( CNumParticlesDlg_Data_t *pData ) {

	CString sValue;
	
	m_ctrlMinParticles.SetPos( AccuCast( pData->NumPerBurst.x ) );
	m_ctrlMaxParticles.SetPos( AccuCast(pData->NumPerBurst.x + pData->NumPerBurst.y) );
	
	sValue.Format( "%f", pData->fSecsToEmit );
	m_ctrlSecsToEmit.SetText( sValue );

	sValue.Format( "%f", 1.0f/pData->SecsBetweenBursts.x );
	m_ctrlMinBurstsPerSec.SetText( sValue );

	sValue.Format( "%f", 1.0f/pData->SecsBetweenBursts.y );
	m_ctrlMaxBurstsPerSec.SetText( sValue );

	sValue.Format( "%f", pData->EmitPause.x );
	m_ctrlMinDelaySecs.SetText( sValue );

	sValue.Format( "%f", pData->EmitPause.y );
	m_ctrlMaxDelaySecs.SetText( sValue );

	// copy our current settings
	m_GameData = *pData;
}

void CNumParticlesDlg::OnChangeEditMaxParticles() {
	OnKillfocusEditMaxParticles();	
}

void CNumParticlesDlg::OnChangeEditMinParticles() {
	OnKillfocusEditMinParticles();
}

void CNumParticlesDlg::OnKillfocusEditMinBurstsPerSec() {
	f32 fMinBurstsPerSec = (f32)m_ctrlMinBurstsPerSec.GetTextAsDouble( &_Min );	
	f32 fMaxBurstsPerSec = (f32)m_ctrlMaxBurstsPerSec.GetTextAsDouble( &_Min );

	if( fMinBurstsPerSec > fMaxBurstsPerSec ) {
		fMaxBurstsPerSec = fMinBurstsPerSec;

		CString sValue;
		sValue.Format( "%f", fMaxBurstsPerSec );
		m_ctrlMaxBurstsPerSec.SetText( sValue );
	}

	m_GameData.SecsBetweenBursts.Set( 1.0f/fMinBurstsPerSec, 1.0f/fMaxBurstsPerSec );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CNumParticlesDlg::OnKillfocusEditMaxBurstsPerSec() {
	f32 fMinBurstsPerSec = (f32)m_ctrlMinBurstsPerSec.GetTextAsDouble( &_Min );	
	f32 fMaxBurstsPerSec = (f32)m_ctrlMaxBurstsPerSec.GetTextAsDouble( &_Min );

	if( fMaxBurstsPerSec < fMinBurstsPerSec ) {
		fMinBurstsPerSec = fMaxBurstsPerSec;

		CString sValue;
		sValue.Format( "%f", fMinBurstsPerSec );
		m_ctrlMinBurstsPerSec.SetText( sValue );
	}

	m_GameData.SecsBetweenBursts.Set( 1.0f/fMinBurstsPerSec, 1.0f/fMaxBurstsPerSec );
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CNumParticlesDlg::OnKillfocusEditMinDelaySec() {
	f32 fMinDelay = (f32)m_ctrlMinDelaySecs.GetTextAsDouble();	
	f32 fMaxDelay = (f32)m_ctrlMaxDelaySecs.GetTextAsDouble();

	if( fMinDelay > fMaxDelay ) {
		fMaxDelay = fMinDelay;

		CString sValue;
		sValue.Format( "%f", fMaxDelay );
		m_ctrlMaxDelaySecs.SetText( sValue );
	}

	m_GameData.EmitPause.Set( fMinDelay, fMaxDelay );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CNumParticlesDlg::OnKillfocusEditMaxDelaySec() {
	f32 fMinDelay = (f32)m_ctrlMinDelaySecs.GetTextAsDouble();	
	f32 fMaxDelay = (f32)m_ctrlMaxDelaySecs.GetTextAsDouble();

	if( fMaxDelay < fMinDelay ) {
		fMinDelay = fMaxDelay;

		CString sValue;
		sValue.Format( "%f", fMinDelay );
		m_ctrlMinDelaySecs.SetText( sValue );
	}

	m_GameData.EmitPause.Set( fMinDelay, fMaxDelay );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

