// ForcesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Spit.h"
#include "ForcesDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CForcesDlg dialog

IMPLEMENT_DYNCREATE(CForcesDlg, CDialog)

CForcesDlg::CForcesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CForcesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CForcesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CForcesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForcesDlg)
	DDX_Control(pDX, IDC_EDIT_BUBBLE_Z, m_ctrlBubbleZ);
	DDX_Control(pDX, IDC_EDIT_BUBBLE_Y, m_ctrlBubbleY);
	DDX_Control(pDX, IDC_EDIT_BUBBLE_X, m_ctrlBubbleX);
	DDX_Control(pDX, IDC_EDIT_BUBBLE_DRAG, m_ctrlBubbleDrag);
	DDX_Control(pDX, IDC_EDIT_GRAVITY, m_ctrlGravity);
	DDX_Control(pDX, IDC_EDIT_DRAG, m_ctrlDrag);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_BUBBLE_UPDATE_CHANCE, m_ctrlBubbleUpdateChance, 0);
}


BEGIN_MESSAGE_MAP(CForcesDlg, CDialog)
	//{{AFX_MSG_MAP(CForcesDlg)
	ON_BN_CLICKED(IDC_RESET_DRAG, OnResetDrag)
	ON_BN_CLICKED(IDC_RESET_GRAVITY, OnResetGravity)
	ON_EN_KILLFOCUS(IDC_EDIT_DRAG, OnKillfocusEditDrag)
	ON_EN_KILLFOCUS(IDC_EDIT_GRAVITY, OnKillfocusEditGravity)
	ON_BN_CLICKED(IDC_EARTH_GRAVITY, OnEarthGravity)
	ON_EN_KILLFOCUS(IDC_EDIT_BUBBLE_DRAG, OnKillfocusEditBubbleDrag)
	ON_EN_KILLFOCUS(IDC_EDIT_BUBBLE_X, OnKillfocusEditBubbleX)
	ON_EN_KILLFOCUS(IDC_EDIT_BUBBLE_Y, OnKillfocusEditBubbleY)
	ON_EN_KILLFOCUS(IDC_EDIT_BUBBLE_Z, OnKillfocusEditBubbleZ)
	ON_EN_KILLFOCUS(IDC_EDIT_BUBBLE_UPDATE_CHANCE, OnKillfocusEditBubbleUpdateChance)
	ON_EN_CHANGE(IDC_EDIT_BUBBLE_UPDATE_CHANCE, OnChangeEditBubbleUpdateChance)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CForcesDlg message handlers

BOOL CForcesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_ctrlGravity.SetMaxWholeDigits( 4 );
	m_ctrlGravity.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	
	m_ctrlDrag.SetMaxWholeDigits( 4 );
	m_ctrlDrag.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );

	m_ctrlBubbleUpdateChance.SetRange( 0, 100 );
	
	m_ctrlBubbleX.SetMaxWholeDigits( 4 );
	m_ctrlBubbleX.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlBubbleX.AllowNegative( FALSE );// do this last

	m_ctrlBubbleY.SetMaxWholeDigits( 4 );
	m_ctrlBubbleY.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlBubbleY.AllowNegative( FALSE );// do this last

	m_ctrlBubbleZ.SetMaxWholeDigits( 4 );
	m_ctrlBubbleZ.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlBubbleZ.AllowNegative( FALSE );// do this last

	m_ctrlBubbleDrag.SetMaxWholeDigits( 4 );
	m_ctrlBubbleDrag.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CForcesDlg::PreTranslateMessage( MSG* pMsg ) {
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

void CForcesDlg::OnOK() {
	//OnCancel();
}

void CForcesDlg::OnResetDrag() {
	CString s;
	
	s = "0.0f";
	m_ctrlDrag.SetText( s );	

	m_GameData.fDragMultiplierPerSec = 0.0f;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::OnResetGravity() {
	CString s;

	s = "0.0f";
	m_ctrlGravity.SetText( s );		

	m_GameData.fGravityPerSec = 0.0f;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::OnKillfocusEditDrag() {
	f32 fDrag = (f32)m_ctrlDrag.GetTextAsDouble();		

	m_GameData.fDragMultiplierPerSec = fDrag;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::OnKillfocusEditGravity() {
	f32 fGravity = (f32)m_ctrlGravity.GetTextAsDouble();
	
	m_GameData.fGravityPerSec = fGravity;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::OnEarthGravity() {
	CString s;

	s = "-32.0f";
	m_ctrlGravity.SetText( s );	

	m_GameData.fGravityPerSec = -32.0f;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::SetData( CForcesDlg_Data_t *pData ) {
	
	CString sValue;

	sValue.Format( "%f", pData->fGravityPerSec );
	m_ctrlGravity.SetText( sValue );

	sValue.Format( "%f", pData->fDragMultiplierPerSec );
	m_ctrlDrag.SetText( sValue );

	m_ctrlBubbleUpdateChance.SetPos( AccuCast( pData->fBubbleUpdateChance * 100.0f ) );
	
	sValue.Format( "%f", pData->fBubbleDragPerSec );
	m_ctrlBubbleDrag.SetText( sValue );

	sValue.Format( "%f", pData->MaxBubbleXYZ.x );
	m_ctrlBubbleX.SetText( sValue );

	sValue.Format( "%f", pData->MaxBubbleXYZ.y );
	m_ctrlBubbleY.SetText( sValue );

	sValue.Format( "%f", pData->MaxBubbleXYZ.z );
	m_ctrlBubbleZ.SetText( sValue );

	// copy our current settings
	m_GameData = *pData;
}

void CForcesDlg::OnKillfocusEditBubbleDrag() {
	f32 fDrag = (f32)m_ctrlBubbleDrag.GetTextAsDouble();		

	m_GameData.fBubbleDragPerSec = fDrag;
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CForcesDlg::OnKillfocusEditBubbleX() {
	f32 fBubbleX = (f32)m_ctrlBubbleX.GetTextAsDouble();		

	m_GameData.MaxBubbleXYZ.x = fBubbleX;
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CForcesDlg::OnKillfocusEditBubbleY() {
	f32 fBubbleY = (f32)m_ctrlBubbleY.GetTextAsDouble();		

	m_GameData.MaxBubbleXYZ.y = fBubbleY;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::OnKillfocusEditBubbleZ() {
	f32 fBubbleZ = (f32)m_ctrlBubbleZ.GetTextAsDouble();		

	m_GameData.MaxBubbleXYZ.z = fBubbleZ;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::OnKillfocusEditBubbleUpdateChance() {
	f32 fChance = (f32)m_ctrlBubbleUpdateChance.GetPos();
	
	m_GameData.fBubbleUpdateChance = fChance * (1.0f/100.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CForcesDlg::OnChangeEditBubbleUpdateChance() {
	OnKillfocusEditBubbleUpdateChance();	
}

LONG CForcesDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlBubbleUpdateChance.GetDlgCtrlID() ) {
		OnKillfocusEditBubbleUpdateChance();	
	}

	return 0;
}
