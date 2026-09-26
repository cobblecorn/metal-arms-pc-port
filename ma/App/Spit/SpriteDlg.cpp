// SpriteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "SpriteDlg.h"
#include "fmath.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

const static double _fMinUnitFloat = 0.0;
const static double _fMaxUnitFloat = 1.0;

const static double _fMinVel = 0.0001;

/////////////////////////////////////////////////////////////////////////////
// CSpriteDlg dialog

IMPLEMENT_DYNCREATE(CSpriteDlg, CDialog)

CSpriteDlg::CSpriteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpriteDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpriteDlg)
	//}}AFX_DATA_INIT
}


void CSpriteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpriteDlg)
	DDX_Control(pDX, IDC_EDIT_SIZE_PERCENT_PER_SPRITE, m_ctrlSizePercent);
	DDX_Control(pDX, IDC_EDIT_POS_PER_SPRITE, m_ctrlPosStep);
	DDX_Control(pDX, IDC_EDIT_MAX_SPRITE_VEL, m_ctrlSpriteVel);
	DDX_Control(pDX, IDC_EDIT_ALPHA_PERCENT_PER_SPRITE, m_ctrlAlphaPercent);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_NUM_SPRITES, m_ctrlNumSprites, 0);
}


BEGIN_MESSAGE_MAP(CSpriteDlg, CDialog)
	//{{AFX_MSG_MAP(CSpriteDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_ALPHA_PERCENT_PER_SPRITE, OnKillfocusEditAlphaPercentPerSprite)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_SPRITE_VEL, OnKillfocusEditMaxSpriteVel)
	ON_EN_KILLFOCUS(IDC_EDIT_NUM_SPRITES, OnKillfocusEditNumSprites)
	ON_EN_KILLFOCUS(IDC_EDIT_POS_PER_SPRITE, OnKillfocusEditPosPerSprite)
	ON_EN_KILLFOCUS(IDC_EDIT_SIZE_PERCENT_PER_SPRITE, OnKillfocusEditSizePercentPerSprite)
	ON_EN_CHANGE(IDC_EDIT_NUM_SPRITES, OnChangeEditNumSprites)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpriteDlg message handlers

BOOL CSpriteDlg::PreTranslateMessage( MSG* pMsg ) {
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

void CSpriteDlg::OnOK() {
	//OnCancel();
}

BOOL CSpriteDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlSizePercent.SetMaxWholeDigits( 1 );
	m_ctrlSizePercent.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlSizePercent.AllowNegative( FALSE );// do this last
	
	m_ctrlPosStep.SetMaxWholeDigits( 1 );
	m_ctrlPosStep.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlPosStep.AllowNegative( FALSE );// do this last
		
	m_ctrlNumSprites.SetRange( 1, 10 );
	
	m_ctrlSpriteVel.SetMaxWholeDigits( 4 );
	m_ctrlSpriteVel.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlSpriteVel.AllowNegative( FALSE );// do this last
	
	m_ctrlAlphaPercent.SetMaxWholeDigits( 1 );
	m_ctrlAlphaPercent.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlAlphaPercent.AllowNegative( FALSE );// do this last
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpriteDlg::OnKillfocusEditAlphaPercentPerSprite() {
	f32 fAlphaStep = (f32)m_ctrlAlphaPercent.GetTextAsDouble( &_fMinUnitFloat, &_fMaxUnitFloat );
	
	// update our copy of the data and then sent it off to the main dlg
	m_GameData.fAlphaStepPerSprite = fAlphaStep;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CSpriteDlg::OnKillfocusEditMaxSpriteVel() {
	f32 fMaxVel = (f32)m_ctrlSpriteVel.GetTextAsDouble( &_fMinVel, NULL );	

	// update our copy of the data and then sent it off to the main dlg
	m_GameData.fMaxVel2 = fMaxVel * fMaxVel;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CSpriteDlg::OnKillfocusEditNumSprites() {
	u32 nMinNumParticles = (u32)m_ctrlNumSprites.GetPos();

	// update our copy of the data and then sent it off to the main dlg
	m_GameData.fNumSprites = (f32)nMinNumParticles;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CSpriteDlg::OnKillfocusEditPosPerSprite() {
	f32 fPosStep = (f32)m_ctrlPosStep.GetTextAsDouble( &_fMinUnitFloat, &_fMaxUnitFloat );	

	// update our copy of the data and then sent it off to the main dlg
	m_GameData.fPosStepPerSprite = fPosStep;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CSpriteDlg::OnKillfocusEditSizePercentPerSprite() {
	f32 fSizeStep = (f32)m_ctrlSizePercent.GetTextAsDouble( &_fMinUnitFloat, &_fMaxUnitFloat );		

	// update our copy of the data and then sent it off to the main dlg
	m_GameData.fSizeStepPerSprite = fSizeStep;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

LONG CSpriteDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlPosStep.GetDlgCtrlID() ) {
		OnKillfocusEditPosPerSprite();	
	}

	return 0;
}

void CSpriteDlg::SetData( CSpriteDlg_Data_t *pData ) {

	CString sValue;
	
	sValue.Format( "%f", pData->fSizeStepPerSprite );
	m_ctrlSizePercent.SetText( sValue );

	sValue.Format( "%f", pData->fPosStepPerSprite );
	m_ctrlPosStep.SetText( sValue );
	
	m_ctrlNumSprites.SetPos( AccuCast( pData->fNumSprites ) );
	
	sValue.Format( "%f", fmath_Sqrt( pData->fMaxVel2 ) );
	m_ctrlSpriteVel.SetText( sValue );

	sValue.Format( "%f", pData->fAlphaStepPerSprite );
	m_ctrlAlphaPercent.SetText( sValue );

	// copy our current settings
	m_GameData = *pData;
}

void CSpriteDlg::OnChangeEditNumSprites() {
	OnKillfocusEditNumSprites();
}
