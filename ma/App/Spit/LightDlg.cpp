// LightDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Spit.h"
#include "LightDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

const static double _fMinRadiusMultiplier = 0.1;
const static double _fZero = 0.0f;

/////////////////////////////////////////////////////////////////////////////
// CLightDlg dialog

IMPLEMENT_DYNCREATE(CLightDlg, CDialog)

CLightDlg::CLightDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLightDlg::IDD, pParent)
	, m_fSoundVolume(0)
	, m_fPitchMultiplier(0)
{
	//{{AFX_DATA_INIT(CLightDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightDlg)
	DDX_Control(pDX, IDC_BUTTON_PICK_COLOR, m_ctrlPickStartColor);
	DDX_Control(pDX, IDC_EDIT_RADIUS_PERCENT, m_ctrlStartRadiusMultiplier);
	DDX_Control(pDX, IDC_BUTTON_PICK_COLOR2, m_ctrlPickEndColor);
	DDX_Control(pDX, IDC_EDIT_RADIUS_PERCENT2, m_ctrlEndRadiusMultiplier);
	DDX_Control(pDX, IDC_EDIT_MAX_RADIUS, m_ctrlMaxRadius);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_RED, m_ctrlStartRed, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_GREEN, m_ctrlStartGreen, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_BLUE, m_ctrlStartBlue, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INTENSITY, m_ctrlStartIntensity, 0);
	DDX_Text(pDX, IDC_EDIT_SOUND_VOL, m_fSoundVolume);
	DDX_Text(pDX, IDC_EDIT_SOUND_PITCH, m_fPitchMultiplier);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_RED2, m_ctrlEndRed, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_GREEN2, m_ctrlEndGreen, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_BLUE2, m_ctrlEndBlue, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INTENSITY2, m_ctrlEndIntensity, 0);
}


BEGIN_MESSAGE_MAP(CLightDlg, CDialog)
	//{{AFX_MSG_MAP(CLightDlg)
	ON_BN_CLICKED(IDC_BUTTON_PICK_COLOR, OnButtonPickStartColor)
	ON_EN_KILLFOCUS(IDC_EDIT_BLUE, OnKillfocusEditStartBlue)
	ON_EN_KILLFOCUS(IDC_EDIT_GREEN, OnKillfocusEditStartGreen)
	ON_EN_KILLFOCUS(IDC_EDIT_INTENSITY, OnKillfocusEditStartIntensity)
	ON_EN_KILLFOCUS(IDC_EDIT_RADIUS_PERCENT, OnKillfocusEditStartRadiusPercent)
	ON_EN_KILLFOCUS(IDC_EDIT_RED, OnKillfocusEditStartRed)
	ON_WM_DRAWITEM()
	ON_EN_CHANGE(IDC_EDIT_RED, OnChangeEditStartRed)
	ON_EN_CHANGE(IDC_EDIT_INTENSITY, OnChangeEditStartIntensity)
	ON_EN_CHANGE(IDC_EDIT_GREEN, OnChangeEditStartGreen)
	ON_EN_CHANGE(IDC_EDIT_BLUE, OnChangeEditStartBlue)
	ON_BN_CLICKED(IDC_BUTTON_PICK_COLOR2, OnButtonPickEndColor)
	ON_EN_KILLFOCUS(IDC_EDIT_BLUE2, OnKillfocusEditEndBlue)
	ON_EN_KILLFOCUS(IDC_EDIT_GREEN2, OnKillfocusEditEndGreen)
	ON_EN_KILLFOCUS(IDC_EDIT_INTENSITY2, OnKillfocusEditEndIntensity)
	ON_EN_KILLFOCUS(IDC_EDIT_RADIUS_PERCENT2, OnKillfocusEditEndRadiusPercent)
	ON_EN_KILLFOCUS(IDC_EDIT_RED2, OnKillfocusEditEndRed)
	ON_EN_CHANGE(IDC_EDIT_RED2, OnChangeEditEndRed)
	ON_EN_CHANGE(IDC_EDIT_INTENSITY2, OnChangeEditEndIntensity)
	ON_EN_CHANGE(IDC_EDIT_GREEN2, OnChangeEditEndGreen)
	ON_EN_CHANGE(IDC_EDIT_BLUE2, OnChangeEditEndBlue)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
	ON_EN_KILLFOCUS(IDC_EDIT_SOUND_VOL, OnEnKillfocusEditSoundVol)
	ON_EN_KILLFOCUS(IDC_EDIT_SOUND_PITCH, OnEnKillfocusEditSoundPitch)
	ON_BN_CLICKED(IDC_MAKE_INTENSITY_EQUAL, OnBnClickedMakeIntensityEqual)
	ON_BN_CLICKED(IDC_MAKE_RADIUS_MULTIPIER_EQUAL, OnBnClickedMakeRadiusMultipierEqual)
	ON_BN_CLICKED(IDC_MAKE_LIGHT_COLORS_EQUAL, OnBnClickedMakeLightColorsEqual)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_RADIUS, OnKillfocusEditMaxRadius)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightDlg message handlers

BOOL CLightDlg::PreTranslateMessage( MSG* pMsg ) {
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

void CLightDlg::OnOK() {
	//OnCancel();
}

BOOL CLightDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
		
	m_ctrlStartRed.SetRange( 0, 255 );		
	m_ctrlStartGreen.SetRange( 0, 255 );
	m_ctrlStartBlue.SetRange( 0, 255 );
	m_ctrlStartRadiusMultiplier.SetMaxWholeDigits( 3 );
	m_ctrlStartRadiusMultiplier.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlStartRadiusMultiplier.AllowNegative( FALSE );// do this last
	m_ctrlStartIntensity.SetRange( 0, 200 );

	m_ctrlEndRed.SetRange( 0, 255 );		
	m_ctrlEndGreen.SetRange( 0, 255 );
	m_ctrlEndBlue.SetRange( 0, 255 );
	m_ctrlEndRadiusMultiplier.SetMaxWholeDigits( 3 );
	m_ctrlEndRadiusMultiplier.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlEndRadiusMultiplier.AllowNegative( FALSE );// do this last
	m_ctrlEndIntensity.SetRange( 0, 200 );

	m_ctrlMaxRadius.SetMaxWholeDigits( 3 );
	m_ctrlMaxRadius.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMaxRadius.AllowNegative( FALSE );// do this last

	m_fPitchMultiplier = 1.0f;
	m_fSoundVolume = 1.0f;
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightDlg::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct ) {
	
	if( nIDCtl == m_ctrlPickStartColor.GetDlgCtrlID() ||
		nIDCtl == m_ctrlPickEndColor.GetDlgCtrlID() ) {
		// draw only a color in the button

		CDC dc;
		dc.Attach( lpDrawItemStruct->hDC );     //Get device context object
		CRect rt;
		rt = lpDrawItemStruct->rcItem;        //Get button rect

		if( nIDCtl == m_ctrlPickStartColor.GetDlgCtrlID() ) {
            dc.FillSolidRect( rt, RGB( m_ctrlStartRed.GetPos(), m_ctrlStartGreen.GetPos(), m_ctrlStartBlue.GetPos() ) );
		} else {
			dc.FillSolidRect( rt, RGB( m_ctrlEndRed.GetPos(), m_ctrlEndGreen.GetPos(), m_ctrlEndBlue.GetPos() ) );
		}

#if 0
		UINT state = lpDrawItemStruct->itemState; //Get state of the button
		if( (state & ODS_SELECTED) ) {           // If it is pressed
			dc.DrawEdge(rt,EDGE_SUNKEN,BF_RECT);    // Draw a sunken face
		} else {
			dc.DrawEdge(rt,EDGE_RAISED,BF_RECT);    // Draw a raised face
		}

		dc.SetTextColor(RGB(255,255,120)); 		// Set the color of the caption to be yellow
		CString strTemp;
		GetWindowText(strTemp); 
							// Get the caption which have been set
		dc.DrawText(strTemp,rt,DT_CENTER|DT_VCENTER|DT_SINGLELINE); 
								  // Draw out the caption
		if( (state & ODS_FOCUS ) ) {       // If the button is focused
			// Draw a focus rect which indicates the user 
			// that the button is focused
			int iChange = 3;
			rt.top += iChange;
			rt.left += iChange;
			rt.right -= iChange;
			rt.bottom -= iChange;
			dc.DrawFocusRect(rt);
		}
#else
		dc.DrawEdge( rt, EDGE_SUNKEN, BF_RECT );    // Draw a sunken face
#endif
		dc.Detach();

		return;
	}

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

LONG CLightDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlStartRed.GetDlgCtrlID() ) {
		OnKillfocusEditStartRed();	
	} else if( wParam == m_ctrlStartGreen.GetDlgCtrlID() ) {
		OnKillfocusEditStartGreen();
	} else if( wParam == m_ctrlStartBlue.GetDlgCtrlID() ) {
		OnKillfocusEditStartBlue();
	} else if( wParam == m_ctrlStartIntensity.GetDlgCtrlID() ) {
		OnKillfocusEditStartIntensity();
	} else if( wParam == m_ctrlEndRed.GetDlgCtrlID() ) {
		OnKillfocusEditEndRed();	
	} else if( wParam == m_ctrlEndGreen.GetDlgCtrlID() ) {
		OnKillfocusEditEndGreen();
	} else if( wParam == m_ctrlEndBlue.GetDlgCtrlID() ) {
		OnKillfocusEditEndBlue();
	} else if( wParam == m_ctrlEndIntensity.GetDlgCtrlID() ) {
		OnKillfocusEditEndIntensity();
	}

	return 0;
}

void CLightDlg::SetData( CLightDlg_Data_t *pData ) {

	CString sValue;

	// the start properties
	m_ctrlStartIntensity.SetPos( AccuCast( pData->fStartLightIntensity * 100.0f ) );
	m_ctrlStartRed.SetPos( AccuCast( pData->StartLightRGB.fRed * 255.0f ) );
	m_ctrlStartGreen.SetPos( AccuCast( pData->StartLightRGB.fGreen * 255.0f ) );
	m_ctrlStartBlue.SetPos( AccuCast( pData->StartLightRGB.fBlue * 255.0f ) );
	sValue.Format( "%f", pData->fStartLightRadiusMultiplier );
	m_ctrlStartRadiusMultiplier.SetText( sValue );
	m_ctrlPickStartColor.Invalidate();

	// the end properties
	m_ctrlEndIntensity.SetPos( AccuCast( pData->fEndLightIntensity * 100.0f ) );
	m_ctrlEndRed.SetPos( AccuCast( pData->EndLightRGB.fRed * 255.0f ) );
	m_ctrlEndGreen.SetPos( AccuCast( pData->EndLightRGB.fGreen * 255.0f ) );
	m_ctrlEndBlue.SetPos( AccuCast( pData->EndLightRGB.fBlue * 255.0f ) );
	sValue.Format( "%f", pData->fEndLightRadiusMultiplier );
	m_ctrlEndRadiusMultiplier.SetText( sValue );
	m_ctrlPickEndColor.Invalidate();

	sValue.Format( "%f", pData->fMaxLightRadius );
	m_ctrlMaxRadius.SetText( sValue );

	m_fPitchMultiplier = pData->fSndPitchMultiplier;
	m_fSoundVolume = pData->fSndUnitVolume;

	UpdateData( VARS_TO_CONTROLS );

	// copy our current settings
	m_GameData = *pData;
}

void CLightDlg::OnChangeEditStartRed() {
	OnKillfocusEditStartRed();
}

void CLightDlg::OnChangeEditStartIntensity() {
	OnKillfocusEditStartIntensity();
}

void CLightDlg::OnChangeEditStartGreen() {
	OnKillfocusEditStartGreen();
}

void CLightDlg::OnChangeEditStartBlue() {
	OnKillfocusEditStartBlue();
}

void CLightDlg::OnButtonPickStartColor() {
	u8 nR = (u8)m_ctrlStartRed.GetPos();
	u8 nG = (u8)m_ctrlStartGreen.GetPos();
	u8 nB = (u8)m_ctrlStartBlue.GetPos();
	u8 nNew;

	COLORREF Color = RGB( nR, nG, nB );

	CColorDialog ColorDlg( Color, CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT, this );
	if( ColorDlg.DoModal() == IDOK ) {
		Color = ColorDlg.GetColor();
		nNew = GetRValue( Color );
		if( nR != nNew ) {
			m_ctrlStartRed.SetPos( nNew );
			m_GameData.StartLightRGB.fRed = (f32)nNew * (1.0f/255.0f);
		}
		nNew = GetGValue( Color );
		if( nG != nNew ) {
			m_ctrlStartGreen.SetPos( nNew );
			m_GameData.StartLightRGB.fGreen = (f32)nNew * (1.0f/255.0f);
		}
		nNew = GetBValue( Color );
		if( nB != nNew ) {
			m_ctrlStartBlue.SetPos( nNew );
			m_GameData.StartLightRGB.fBlue = (f32)nNew * (1.0f/255.0f);
		}

		m_ctrlPickStartColor.Invalidate();		

		Spit_pSpitDlg->DataChanged( &m_GameData );
	}			
}

void CLightDlg::OnKillfocusEditStartBlue() {
	f32 fBlue = (f32)m_ctrlStartBlue.GetPos();
	fBlue *= (1.0f/255.0f);

	m_ctrlPickStartColor.Invalidate();

	m_GameData.StartLightRGB.fBlue = fBlue;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditStartGreen() {
	f32 fGreen = (f32)m_ctrlStartGreen.GetPos();
	fGreen *= (1.0f/255.0f);
	
	m_ctrlPickStartColor.Invalidate();

	m_GameData.StartLightRGB.fGreen = fGreen;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditStartIntensity() {
	f32 fIntensity = (f32)m_ctrlStartIntensity.GetPos();
	fIntensity *= (1.0f/100.0f);

	m_GameData.fStartLightIntensity = fIntensity;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditStartRadiusPercent() {
	f32 fRadiusMultiplier = (f32)m_ctrlStartRadiusMultiplier.GetTextAsDouble( &_fMinRadiusMultiplier, NULL );
	
	m_GameData.fStartLightRadiusMultiplier = fRadiusMultiplier;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditStartRed() {
	f32 fRed = (f32)m_ctrlStartRed.GetPos();
	fRed *= (1.0f/255.0f);
	
	m_ctrlPickStartColor.Invalidate();

	m_GameData.StartLightRGB.fRed = fRed;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnEnKillfocusEditSoundVol() {
	
	UpdateData( CONTROLS_TO_VARS );	
	FMATH_CLAMP( m_fSoundVolume, 0.0f, 1.0f );
	UpdateData( VARS_TO_CONTROLS );

	m_GameData.fSndUnitVolume = m_fSoundVolume;
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CLightDlg::OnEnKillfocusEditSoundPitch() {
	
	UpdateData( CONTROLS_TO_VARS );	
	FMATH_CLAMP( m_fPitchMultiplier, 0.1f, 4.0f );
	UpdateData( VARS_TO_CONTROLS );

	m_GameData.fSndPitchMultiplier = m_fPitchMultiplier;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnChangeEditEndRed() {
	OnKillfocusEditEndRed();
}

void CLightDlg::OnChangeEditEndIntensity() {
	OnKillfocusEditEndIntensity();
}

void CLightDlg::OnChangeEditEndGreen() {
	OnKillfocusEditEndGreen();
}

void CLightDlg::OnChangeEditEndBlue() {
	OnKillfocusEditEndBlue();
}

void CLightDlg::OnButtonPickEndColor() {
	u8 nR = (u8)m_ctrlEndRed.GetPos();
	u8 nG = (u8)m_ctrlEndGreen.GetPos();
	u8 nB = (u8)m_ctrlEndBlue.GetPos();
	u8 nNew;

	COLORREF Color = RGB( nR, nG, nB );

	CColorDialog ColorDlg( Color, CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT, this );
	if( ColorDlg.DoModal() == IDOK ) {
		Color = ColorDlg.GetColor();
		nNew = GetRValue( Color );
		if( nR != nNew ) {
			m_ctrlEndRed.SetPos( nNew );
			m_GameData.EndLightRGB.fRed = (f32)nNew * (1.0f/255.0f);
		}
		nNew = GetGValue( Color );
		if( nG != nNew ) {
			m_ctrlEndGreen.SetPos( nNew );
			m_GameData.EndLightRGB.fGreen = (f32)nNew * (1.0f/255.0f);
		}
		nNew = GetBValue( Color );
		if( nB != nNew ) {
			m_ctrlEndBlue.SetPos( nNew );
			m_GameData.EndLightRGB.fBlue = (f32)nNew * (1.0f/255.0f);
		}

		m_ctrlPickEndColor.Invalidate();		

		Spit_pSpitDlg->DataChanged( &m_GameData );
	}			
}

void CLightDlg::OnKillfocusEditEndBlue() {
	f32 fBlue = (f32)m_ctrlEndBlue.GetPos();
	fBlue *= (1.0f/255.0f);

	m_ctrlPickEndColor.Invalidate();

	m_GameData.EndLightRGB.fBlue = fBlue;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditEndGreen() {
	f32 fGreen = (f32)m_ctrlEndGreen.GetPos();
	fGreen *= (1.0f/255.0f);
	
	m_ctrlPickEndColor.Invalidate();

	m_GameData.EndLightRGB.fGreen = fGreen;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditEndIntensity() {
	f32 fIntensity = (f32)m_ctrlEndIntensity.GetPos();
	fIntensity *= (1.0f/100.0f);

	m_GameData.fEndLightIntensity = fIntensity;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditEndRadiusPercent() {
	f32 fRadiusMultiplier = (f32)m_ctrlEndRadiusMultiplier.GetTextAsDouble( &_fMinRadiusMultiplier, NULL );
	
	m_GameData.fEndLightRadiusMultiplier = fRadiusMultiplier;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditEndRed() {
	f32 fRed = (f32)m_ctrlEndRed.GetPos();
	fRed *= (1.0f/255.0f);
	
	m_ctrlPickEndColor.Invalidate();

	m_GameData.EndLightRGB.fRed = fRed;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnBnClickedMakeIntensityEqual() {
	f32 fIntensity = (f32)m_ctrlStartIntensity.GetPos();
	fIntensity *= (1.0f/100.0f);

	m_GameData.fEndLightIntensity = fIntensity;
	m_ctrlEndIntensity.SetPos( AccuCast( fIntensity * 100.0f ) );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnBnClickedMakeRadiusMultipierEqual() {
	f32 fRadiusMultiplier = (f32)m_ctrlStartRadiusMultiplier.GetTextAsDouble( &_fMinRadiusMultiplier, NULL );
	
	m_GameData.fEndLightRadiusMultiplier = fRadiusMultiplier;	
	CString sValue;
	sValue.Format( "%f", fRadiusMultiplier );
	m_ctrlEndRadiusMultiplier.SetText( sValue );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnBnClickedMakeLightColorsEqual() {
	
	f32 fRed = (f32)m_ctrlStartRed.GetPos();
	fRed *= (1.0f/255.0f);
	f32 fGreen = (f32)m_ctrlStartGreen.GetPos();
	fGreen *= (1.0f/255.0f);
	f32 fBlue = (f32)m_ctrlStartBlue.GetPos();
	fBlue *= (1.0f/255.0f);
	
	m_GameData.EndLightRGB.fRed = fRed;
	m_GameData.EndLightRGB.fGreen = fGreen;
	m_GameData.EndLightRGB.fBlue = fBlue;

	m_ctrlEndRed.SetPos( AccuCast( fRed * 255.0f ) );
	m_ctrlEndGreen.SetPos( AccuCast( fGreen * 255.0f ) );
	m_ctrlEndBlue.SetPos( AccuCast( fBlue * 255.0f ) );

	m_ctrlPickEndColor.Invalidate();
	
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLightDlg::OnKillfocusEditMaxRadius() {
	f32 fMaxR = (f32)m_ctrlMaxRadius.GetTextAsDouble( &_fZero, NULL );
	
	m_GameData.fMaxLightRadius = fMaxR;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}
