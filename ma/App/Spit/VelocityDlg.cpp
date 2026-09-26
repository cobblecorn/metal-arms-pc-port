// VelocityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "VelocityDlg.h"
#include "SpitDlg.h"
#include <math.h>

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CVelocityDlg dialog

IMPLEMENT_DYNCREATE(CVelocityDlg, CDialog)

CVelocityDlg::CVelocityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVelocityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVelocityDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVelocityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVelocityDlg)
	DDX_Control(pDX, IDC_EDIT_MIN_INIT_VEL, m_ctrlMinInitVel);
	DDX_Control(pDX, IDC_EDIT_MAX_INIT_VEL, m_ctrlMaxInitVel);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_MIN_SPREAD_ANGLE, m_ctrlMinSpreadAngle, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_MAX_SPREAD_ANGLE, m_ctrlMaxSpreadAngle, 0);
}


BEGIN_MESSAGE_MAP(CVelocityDlg, CDialog)
	//{{AFX_MSG_MAP(CVelocityDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_INIT_VEL, OnKillfocusEditMaxInitVel)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_SPREAD_ANGLE, OnKillfocusEditMaxSpreadAngle)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_INIT_VEL, OnKillfocusEditMinInitVel)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_SPREAD_ANGLE, OnKillfocusEditMinSpreadAngle)
	ON_EN_CHANGE(IDC_EDIT_MAX_SPREAD_ANGLE, OnChangeEditMaxSpreadAngle)
	ON_EN_CHANGE(IDC_EDIT_MIN_SPREAD_ANGLE, OnChangeEditMinSpreadAngle)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVelocityDlg message handlers

BOOL CVelocityDlg::PreTranslateMessage( MSG* pMsg ) {
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

void CVelocityDlg::OnOK() {
	//OnCancel();
}

BOOL CVelocityDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_ctrlMinInitVel.SetMaxWholeDigits( 3 );
	m_ctrlMinInitVel.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMinInitVel.AllowNegative( FALSE );// do this last
		
	m_ctrlMaxInitVel.SetMaxWholeDigits( 3 );
	m_ctrlMaxInitVel.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMaxInitVel.AllowNegative( FALSE );// do this last
		
	m_ctrlMinSpreadAngle.SetRange( 0, 89 );
		
	m_ctrlMaxSpreadAngle.SetRange( 0, 89 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVelocityDlg::OnKillfocusEditMaxInitVel() {
	f32 fMinVel = (f32)m_ctrlMinInitVel.GetTextAsDouble();

	f32 fMaxInitVel = (f32)m_ctrlMaxInitVel.GetTextAsDouble();	
	FMATH_CLAMPMIN( fMaxInitVel, 0.01f );
	if( fMaxInitVel < fMinVel ) {
		fMinVel = fMaxInitVel;
		// set the min equal to the max
		CString sValue;
		sValue.Format( "%f", fMinVel );
		m_ctrlMinInitVel.SetText( sValue );
	}

	// update our copy of the data and then sent it off to the main dlg
	GuiToGameData( &m_GameData,
				   m_ctrlMinSpreadAngle.GetPos(), m_ctrlMaxSpreadAngle.GetPos(),
				   fMinVel, fMaxInitVel );
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CVelocityDlg::OnKillfocusEditMinInitVel() {
	f32 fMaxVel = (f32)m_ctrlMaxInitVel.GetTextAsDouble();

	f32 fMinInitVel = (f32)m_ctrlMinInitVel.GetTextAsDouble();	
	FMATH_CLAMPMIN( fMinInitVel, 0.01f );
	if( fMinInitVel > fMaxVel ) {
		fMaxVel = fMinInitVel;

		// set the max equal to the min
		CString sValue;
		sValue.Format( "%f", fMaxVel );
		m_ctrlMaxInitVel.SetText( sValue );
	}

	// update our copy of the data and then sent it off to the main dlg
	GuiToGameData( &m_GameData,
				   m_ctrlMinSpreadAngle.GetPos(), m_ctrlMaxSpreadAngle.GetPos(),
				   fMinInitVel, fMaxVel );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CVelocityDlg::OnKillfocusEditMaxSpreadAngle() {
	int nMinAngle = m_ctrlMinSpreadAngle.GetPos();
	int nMaxAngle = m_ctrlMaxSpreadAngle.GetPos();
#if 0// allow the min and max to be independant of each other
	if( nMaxAngle < nMinAngle ) {
		nMinAngle = nMaxAngle;
		m_ctrlMinSpreadAngle.SetPos( nMinAngle );
	}
#endif
	// update our copy of the data and then sent it off to the main dlg
	GuiToGameData( &m_GameData, 
				   nMinAngle, nMaxAngle,
				   (f32)m_ctrlMinInitVel.GetTextAsDouble(), (f32)m_ctrlMaxInitVel.GetTextAsDouble() );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CVelocityDlg::OnKillfocusEditMinSpreadAngle() {
	int nMinAngle = m_ctrlMinSpreadAngle.GetPos();
	int nMaxAngle = m_ctrlMaxSpreadAngle.GetPos();
#if 0// allow the min and max to be independant of each other
	if( nMinAngle > nMaxAngle ) {
		nMaxAngle = nMinAngle;
		m_ctrlMaxSpreadAngle.SetPos( nMaxAngle );
	}
#endif
	// update our copy of the data and then sent it off to the main dlg
	GuiToGameData( &m_GameData, 
				   nMinAngle, nMaxAngle,
				   (f32)m_ctrlMinInitVel.GetTextAsDouble(), (f32)m_ctrlMaxInitVel.GetTextAsDouble() );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

LONG CVelocityDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlMaxSpreadAngle.GetDlgCtrlID() ) {
		OnKillfocusEditMaxSpreadAngle();	
	} else if( wParam == m_ctrlMinSpreadAngle.GetDlgCtrlID() ) {
		OnKillfocusEditMinSpreadAngle();
	}

	return 0;
}

void CVelocityDlg::GuiToGameData( CVelocityDlg_Data_t *pData, 
								  int nMinAngle, int nMaxAngle,
								  f32 fMinVel, f32 fMaxVel ) {

	pData->InitDirAngle.x = FMATH_DEG2RAD( (f32)nMinAngle );
	pData->InitDirAngle.y = FMATH_DEG2RAD( (f32)(nMaxAngle - nMinAngle) );
	pData->VelocityZ.x = fMinVel;
	pData->VelocityZ.y = fMaxVel - fMinVel;
}

void CVelocityDlg::SetData( CVelocityDlg_Data_t *pData ) {
	CString sValue;

	int nAngle = AccuCast( FMATH_RAD2DEG( pData->InitDirAngle.x ) );
	FMATH_CLAMP( nAngle, 0, 89 );
	m_ctrlMinSpreadAngle.SetPos( nAngle );

	nAngle = AccuCast( FMATH_RAD2DEG( pData->InitDirAngle.x + pData->InitDirAngle.y ) );
	FMATH_CLAMP( nAngle, 0, 89 );
	m_ctrlMaxSpreadAngle.SetPos( nAngle );
	
	sValue.Format( "%f", pData->VelocityZ.x );
	m_ctrlMinInitVel.SetText( sValue );

	sValue.Format( "%f", pData->VelocityZ.x + pData->VelocityZ.y );
	m_ctrlMaxInitVel.SetText( sValue );

	// copy our current settings
	m_GameData = *pData;
}

void CVelocityDlg::OnChangeEditMaxSpreadAngle() {
	OnKillfocusEditMaxSpreadAngle();	
}

void CVelocityDlg::OnChangeEditMinSpreadAngle() {
	OnKillfocusEditMinSpreadAngle();	
}
