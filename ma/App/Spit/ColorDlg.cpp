// ColorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "ColorDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorDlg dialog

IMPLEMENT_DYNCREATE(CColorDlg, CDialog)

CColorDlg::CColorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColorDlg)
	//}}AFX_DATA_INIT
}


void CColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorDlg)
	DDX_Control(pDX, IDC_BUTTON_PICK_MIN_INIT_COLOR, m_ctrlPickMinInitColor);
	DDX_Control(pDX, IDC_BUTTON_PICK_MIN_FINAL_COLOR, m_ctrlPickMinFinalColor);
	DDX_Control(pDX, IDC_BUTTON_PICK_MAX_INIT_COLOR, m_ctrlPickMaxInitColor);
	DDX_Control(pDX, IDC_BUTTON_PICK_MAX_FINAL_COLOR, m_ctrlPickMaxFinalColor);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MIN_RED, m_ctrlInitMinRed, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MIN_GREEN, m_ctrlInitMinGreen, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MIN_BLUE, m_ctrlInitMinBlue, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MIN_ALPHA, m_ctrlInitMinAlpha, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MAX_RED, m_ctrlInitMaxRed, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MAX_GREEN, m_ctrlInitMaxGreen, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MAX_BLUE, m_ctrlInitMaxBlue, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_INIT_MAX_ALPHA, m_ctrlInitMaxAlpha, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MIN_RED, m_ctrlFinalMinRed, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MIN_GREEN, m_ctrlFinalMinGreen, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MIN_BLUE, m_ctrlFinalMinBlue, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MIN_ALPHA, m_ctrlFinalMinAlpha, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MAX_RED, m_ctrlFinalMaxRed, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MAX_GREEN, m_ctrlFinalMaxGreen, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MAX_ALPHA, m_ctrlFinalMaxAlpha, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_FINAL_MAX_BLUE, m_ctrlFinalMaxBlue, 0);
}


BEGIN_MESSAGE_MAP(CColorDlg, CDialog)
	//{{AFX_MSG_MAP(CColorDlg)
	ON_BN_CLICKED(IDC_BUTTON_PICK_MAX_FINAL_COLOR, OnButtonPickMaxFinalColor)
	ON_BN_CLICKED(IDC_BUTTON_PICK_MAX_INIT_COLOR, OnButtonPickMaxInitColor)
	ON_BN_CLICKED(IDC_BUTTON_PICK_MIN_FINAL_COLOR, OnButtonPickMinFinalColor)
	ON_BN_CLICKED(IDC_BUTTON_PICK_MIN_INIT_COLOR, OnButtonPickMinInitColor)
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MAX_ALPHA, OnKillfocusEditFinalMaxAlpha)
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MIN_ALPHA, OnKillfocusEditFinalMinAlpha)
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MAX_BLUE, OnKillfocusEditFinalMaxRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MIN_BLUE, OnKillfocusEditFinalMinRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MAX_ALPHA, OnKillfocusEditInitMaxAlpha)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MIN_ALPHA, OnKillfocusEditInitMinAlpha)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MAX_BLUE, OnKillfocusEditInitMaxRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MIN_BLUE, OnKillfocusEditInitMinRGB)
	ON_WM_DRAWITEM()
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MAX_GREEN, OnKillfocusEditFinalMaxRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MAX_RED, OnKillfocusEditFinalMaxRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MIN_GREEN, OnKillfocusEditFinalMinRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_FINAL_MIN_RED, OnKillfocusEditFinalMinRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MAX_GREEN, OnKillfocusEditInitMaxRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MAX_RED, OnKillfocusEditInitMaxRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MIN_GREEN, OnKillfocusEditInitMinRGB)
	ON_EN_KILLFOCUS(IDC_EDIT_INIT_MIN_RED, OnKillfocusEditInitMinRGB)
	ON_EN_CHANGE(IDC_EDIT_INIT_MIN_RED, OnChangeEditInitMinRed)
	ON_EN_CHANGE(IDC_EDIT_INIT_MIN_GREEN, OnChangeEditInitMinGreen)
	ON_EN_CHANGE(IDC_EDIT_INIT_MIN_BLUE, OnChangeEditInitMinBlue)
	ON_EN_CHANGE(IDC_EDIT_INIT_MIN_ALPHA, OnChangeEditInitMinAlpha)
	ON_EN_CHANGE(IDC_EDIT_INIT_MAX_RED, OnChangeEditInitMaxRed)
	ON_EN_CHANGE(IDC_EDIT_INIT_MAX_GREEN, OnChangeEditInitMaxGreen)
	ON_EN_CHANGE(IDC_EDIT_INIT_MAX_BLUE, OnChangeEditInitMaxBlue)
	ON_EN_CHANGE(IDC_EDIT_INIT_MAX_ALPHA, OnChangeEditInitMaxAlpha)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MIN_RED, OnChangeEditFinalMinRed)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MIN_GREEN, OnChangeEditFinalMinGreen)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MIN_BLUE, OnChangeEditFinalMinBlue)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MIN_ALPHA, OnChangeEditFinalMinAlpha)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MAX_RED, OnChangeEditFinalMaxRed)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MAX_GREEN, OnChangeEditFinalMaxGreen)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MAX_BLUE, OnChangeEditFinalMaxBlue)
	ON_EN_CHANGE(IDC_EDIT_FINAL_MAX_ALPHA, OnChangeEditFinalMaxAlpha)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
	ON_BN_CLICKED(IDC_MAKE_INIT_COLORS_EQUAL, OnBnClickedMakeInitColorsEqual)
	ON_BN_CLICKED(IDC_MAKE_FINAL_COLORS_EQUAL, OnBnClickedMakeFinalColorsEqual)
	ON_BN_CLICKED(IDC_MAKE_INIT_ALPHA_EQUAL, OnBnClickedMakeInitAlphaEqual)
	ON_BN_CLICKED(IDC_MAKE_FINAL_ALPHA_EQUAL, OnBnClickedMakeFinalAlphaEqual)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorDlg message handlers

BOOL CColorDlg::PreTranslateMessage( MSG* pMsg ) {
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

BOOL CColorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlInitMinRed.SetRange( 0, 255 );
	
	m_ctrlInitMinGreen.SetRange( 0, 255 );
	
	m_ctrlInitMinBlue.SetRange( 0, 255 );

	m_ctrlInitMinAlpha.SetRange( 0, 255 );

	m_ctrlInitMaxRed.SetRange( 0, 255 );

	m_ctrlInitMaxGreen.SetRange( 0, 255 );

	m_ctrlInitMaxBlue.SetRange( 0, 255 );

	m_ctrlInitMaxAlpha.SetRange( 0, 255 );

	m_ctrlFinalMinRed.SetRange( 0, 255 );

	m_ctrlFinalMinGreen.SetRange( 0, 255 );

	m_ctrlFinalMinBlue.SetRange( 0, 255 );

	m_ctrlFinalMinAlpha.SetRange( 0, 255 );

	m_ctrlFinalMaxRed.SetRange( 0, 255 );

	m_ctrlFinalMaxGreen.SetRange( 0, 255 );

	m_ctrlFinalMaxAlpha.SetRange( 0, 255 );

	m_ctrlFinalMaxBlue.SetRange( 0, 255 );	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CColorDlg::OnOK() {
	//OnCancel();
}

void CColorDlg::OnButtonPickMaxFinalColor() {
	ColorPicker( &m_ctrlFinalMaxRed, &m_ctrlFinalMaxGreen, &m_ctrlFinalMaxBlue, &m_ctrlPickMaxFinalColor );		

	OnKillfocusEditFinalMaxRGB();
}

void CColorDlg::OnButtonPickMaxInitColor() {
	ColorPicker( &m_ctrlInitMaxRed, &m_ctrlInitMaxGreen, &m_ctrlInitMaxBlue, &m_ctrlPickMaxInitColor );			

	OnKillfocusEditInitMaxRGB();
}

void CColorDlg::OnButtonPickMinFinalColor() {
	ColorPicker( &m_ctrlFinalMinRed, &m_ctrlFinalMinGreen, &m_ctrlFinalMinBlue, &m_ctrlPickMinFinalColor );			

	OnKillfocusEditFinalMinRGB();
}

void CColorDlg::OnButtonPickMinInitColor() {
	ColorPicker( &m_ctrlInitMinRed, &m_ctrlInitMinGreen, &m_ctrlInitMinBlue, &m_ctrlPickMinInitColor );				

	OnKillfocusEditInitMinRGB();
}

void CColorDlg::OnKillfocusEditFinalMaxAlpha() {
	s32 nMin = m_ctrlFinalMinAlpha.GetPos();
	s32 nMax = m_ctrlFinalMaxAlpha.GetPos();

	if( nMax < nMin ) {
		nMin = nMax;
		m_ctrlFinalMinAlpha.SetPos( nMin );
	}

	m_GameData.FinalAlpha.Set( (f32)nMin, (f32)(nMax-nMin) ); 
	m_GameData.FinalAlpha *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CColorDlg::OnKillfocusEditFinalMinAlpha() {
	s32 nMin = m_ctrlFinalMinAlpha.GetPos();
	s32 nMax = m_ctrlFinalMaxAlpha.GetPos();

	if( nMin > nMax ) {
		nMax = nMin;
		m_ctrlFinalMaxAlpha.SetPos( nMax );
	}

	m_GameData.FinalAlpha.Set( (f32)nMin, (f32)(nMax-nMin) ); 
	m_GameData.FinalAlpha *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CColorDlg::OnKillfocusEditInitMaxAlpha() {
	s32 nMin = m_ctrlInitMinAlpha.GetPos();
	s32 nMax = m_ctrlInitMaxAlpha.GetPos();

	if( nMax < nMin ) {
		nMin = nMax;
		m_ctrlInitMinAlpha.SetPos( nMin );
	}

	m_GameData.InitAlpha.Set( (f32)nMin, (f32)(nMax-nMin) ); 
	m_GameData.InitAlpha *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CColorDlg::OnKillfocusEditInitMinAlpha() {
	s32 nMin = m_ctrlInitMinAlpha.GetPos();
	s32 nMax = m_ctrlInitMaxAlpha.GetPos();

	if( nMin > nMax ) {
		nMax = nMin;
		m_ctrlInitMaxAlpha.SetPos( nMax );
	}
	
	m_GameData.InitAlpha.Set( (f32)nMin, (f32)(nMax-nMin) ); 
	m_GameData.InitAlpha *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CColorDlg::OnKillfocusEditFinalMaxRGB() {
	s32 nMinRed = m_ctrlFinalMinRed.GetPos();	
	s32 nMinGreen = m_ctrlFinalMinGreen.GetPos();		
	s32 nMinBlue = m_ctrlFinalMinBlue.GetPos();

	s32 nMaxRed = m_ctrlFinalMaxRed.GetPos();
	s32 nMaxGreen = m_ctrlFinalMaxGreen.GetPos();		
	s32 nMaxBlue = m_ctrlFinalMaxBlue.GetPos();
	
	m_ctrlPickMaxFinalColor.Invalidate();

	m_GameData.FinalRed.Set( (f32)nMinRed, (f32)(nMaxRed - nMinRed) );
	m_GameData.FinalRed *= (1.0f/255.0f);
	m_GameData.FinalGreen.Set( (f32)nMinGreen, (f32)(nMaxGreen - nMinGreen) );
	m_GameData.FinalGreen *= (1.0f/255.0f);
	m_GameData.FinalBlue.Set( (f32)nMinBlue, (f32)(nMaxBlue - nMinBlue) );
	m_GameData.FinalBlue *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CColorDlg::OnKillfocusEditFinalMinRGB() {
	s32 nMinRed = m_ctrlFinalMinRed.GetPos();	
	s32 nMinGreen = m_ctrlFinalMinGreen.GetPos();		
	s32 nMinBlue = m_ctrlFinalMinBlue.GetPos();
	
	s32 nMaxRed = m_ctrlFinalMaxRed.GetPos();
	s32 nMaxGreen = m_ctrlFinalMaxGreen.GetPos();		
	s32 nMaxBlue = m_ctrlFinalMaxBlue.GetPos();

	m_ctrlPickMinFinalColor.Invalidate();

	m_GameData.FinalRed.Set( (f32)nMinRed, (f32)(nMaxRed - nMinRed) );
	m_GameData.FinalRed *= (1.0f/255.0f);
	m_GameData.FinalGreen.Set( (f32)nMinGreen, (f32)(nMaxGreen - nMinGreen) );
	m_GameData.FinalGreen *= (1.0f/255.0f);
	m_GameData.FinalBlue.Set( (f32)nMinBlue, (f32)(nMaxBlue - nMinBlue) );
	m_GameData.FinalBlue *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CColorDlg::OnKillfocusEditInitMaxRGB() {
	s32 nMinRed = m_ctrlInitMinRed.GetPos();
	s32 nMinGreen = m_ctrlInitMinGreen.GetPos();
	s32 nMinBlue = m_ctrlInitMinBlue.GetPos();

	s32 nMaxRed = m_ctrlInitMaxRed.GetPos();
	s32 nMaxGreen = m_ctrlInitMaxGreen.GetPos();	
	s32 nMaxBlue = m_ctrlInitMaxBlue.GetPos();				

	m_ctrlPickMaxInitColor.Invalidate();

	m_GameData.InitRed.Set( (f32)nMinRed, (f32)(nMaxRed - nMinRed) );
	m_GameData.InitRed *= (1.0f/255.0f);
	m_GameData.InitGreen.Set( (f32)nMinGreen, (f32)(nMaxGreen - nMinGreen) );
	m_GameData.InitGreen *= (1.0f/255.0f);
	m_GameData.InitBlue.Set( (f32)nMinBlue, (f32)(nMaxBlue - nMinBlue) );
	m_GameData.InitBlue *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CColorDlg::OnKillfocusEditInitMinRGB() {
	s32 nMinRed = m_ctrlInitMinRed.GetPos();
	s32 nMinGreen = m_ctrlInitMinGreen.GetPos();
	s32 nMinBlue = m_ctrlInitMinBlue.GetPos();	

	s32 nMaxRed = m_ctrlInitMaxRed.GetPos();
	s32 nMaxGreen = m_ctrlInitMaxGreen.GetPos();
	s32 nMaxBlue = m_ctrlInitMaxBlue.GetPos();	
	
	m_ctrlPickMinInitColor.Invalidate();

	m_GameData.InitRed.Set( (f32)nMinRed, (f32)(nMaxRed - nMinRed) );
	m_GameData.InitRed *= (1.0f/255.0f);
	m_GameData.InitGreen.Set( (f32)nMinGreen, (f32)(nMaxGreen - nMinGreen) );
	m_GameData.InitGreen *= (1.0f/255.0f);
	m_GameData.InitBlue.Set( (f32)nMinBlue, (f32)(nMaxBlue - nMinBlue) );
	m_GameData.InitBlue *= (1.0f/255.0f);
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

FINLINE void CColorDlg::ColorPicker( wcSliderButton *pRIntEdit,
									 wcSliderButton *pGIntEdit,
									 wcSliderButton *pBIntEdit,
									 CButton *pButton ) {
	u8 nR = pRIntEdit->GetPos();
	u8 nG = pGIntEdit->GetPos();
	u8 nB = pBIntEdit->GetPos();
	u8 nNew;

	COLORREF Color = RGB( nR, nG, nB );

	CColorDialog ColorDlg( Color, CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT, this );
	if( ColorDlg.DoModal() == IDOK ) {
		
		Color = ColorDlg.GetColor();
		nNew = GetRValue( Color );
		if( nR != nNew ) {
			pRIntEdit->SetPos( nNew );			
		}
		nNew = GetGValue( Color );
		if( nG != nNew ) {
			pGIntEdit->SetPos( nNew );
		}
		nNew = GetBValue( Color );
		if( nB != nNew ) {
			pBIntEdit->SetPos( nNew );
		}

		pButton->Invalidate();
	}
}

void CColorDlg::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct ) {
	BOOL bDraw = FALSE;
	u8 nR, nG, nB;

	if( nIDCtl == m_ctrlPickMinInitColor.GetDlgCtrlID() ) {
		// draw only a color in the button
		bDraw = TRUE;
		nR = m_ctrlInitMinRed.GetPos();
		nG = m_ctrlInitMinGreen.GetPos();
		nB = m_ctrlInitMinBlue.GetPos();
	} else if( nIDCtl == m_ctrlPickMinFinalColor.GetDlgCtrlID() ) {
		// draw only a color in the button
		bDraw = TRUE;
		nR = m_ctrlFinalMinRed.GetPos();
		nG = m_ctrlFinalMinGreen.GetPos();
		nB = m_ctrlFinalMinBlue.GetPos();
	} else if( nIDCtl == m_ctrlPickMaxInitColor.GetDlgCtrlID() ) {
		// draw only a color in the button
		bDraw = TRUE;
		nR = m_ctrlInitMaxRed.GetPos();
		nG = m_ctrlInitMaxGreen.GetPos();
		nB = m_ctrlInitMaxBlue.GetPos();
	} else if( nIDCtl == m_ctrlPickMaxFinalColor.GetDlgCtrlID() ) {
		// draw only a color in the button
		bDraw = TRUE;
		nR = m_ctrlFinalMaxRed.GetPos();
		nG = m_ctrlFinalMaxGreen.GetPos();
		nB = m_ctrlFinalMaxBlue.GetPos();
	}

	if( bDraw ) {
		CDC dc;
		dc.Attach( lpDrawItemStruct->hDC );     //Get device context object
		CRect rt;
		rt = lpDrawItemStruct->rcItem;        //Get button rect

		dc.FillSolidRect( rt, RGB( nR, nG, nB ) );

		dc.DrawEdge( rt, EDGE_SUNKEN, BF_RECT );    // Draw a sunken face

		dc.Detach();

		return;
	}

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

LONG CColorDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlInitMinRed.GetDlgCtrlID() ) {
		OnKillfocusEditInitMinRGB();	
	} else if( wParam == m_ctrlInitMinGreen.GetDlgCtrlID() ) {
		OnKillfocusEditInitMinRGB();
	} else if( wParam == m_ctrlInitMinBlue.GetDlgCtrlID() ) {
		OnKillfocusEditInitMinRGB();
	} else if( wParam == m_ctrlInitMinAlpha.GetDlgCtrlID() ) {
		OnKillfocusEditInitMinAlpha();
	} else if( wParam == m_ctrlInitMaxRed.GetDlgCtrlID() ) {
		OnKillfocusEditInitMaxRGB();	
	} else if( wParam == m_ctrlInitMaxGreen.GetDlgCtrlID() ) {
		OnKillfocusEditInitMaxRGB();
	} else if( wParam == m_ctrlInitMaxBlue.GetDlgCtrlID() ) {
		OnKillfocusEditInitMaxRGB();
	} else if( wParam == m_ctrlInitMaxAlpha.GetDlgCtrlID() ) {
		OnKillfocusEditInitMaxAlpha();
	} else if( wParam == m_ctrlFinalMinRed.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMinRGB();
	} else if( wParam == m_ctrlFinalMinGreen.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMinRGB();
	} else if( wParam == m_ctrlFinalMinBlue.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMinRGB();
	} else if( wParam == m_ctrlFinalMinAlpha.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMinAlpha();
	} else if( wParam == m_ctrlFinalMaxRed.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMaxRGB();
	} else if( wParam == m_ctrlFinalMaxGreen.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMaxRGB();
	} else if( wParam == m_ctrlFinalMaxBlue.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMaxRGB();
	} else if( wParam == m_ctrlFinalMaxAlpha.GetDlgCtrlID() ) {
		OnKillfocusEditFinalMaxAlpha();
	}

	return 0;
}

void CColorDlg::SetData( CColorDlg_Data_t *pData ) {

	m_ctrlInitMinRed.SetPos( AccuCast( pData->InitRed.x * 255.0f ) );
	m_ctrlInitMinGreen.SetPos( AccuCast(pData->InitGreen.x * 255.0f) );
	m_ctrlInitMinBlue.SetPos( AccuCast(pData->InitBlue.x * 255.0f) );
	m_ctrlInitMinAlpha.SetPos( AccuCast(pData->InitAlpha.x * 255.0f) );

	m_ctrlInitMaxRed.SetPos( AccuCast( (pData->InitRed.x + pData->InitRed.y) * 255.0f ) );
	m_ctrlInitMaxGreen.SetPos( AccuCast( (pData->InitGreen.x + pData->InitGreen.y) * 255.0f ) );
	m_ctrlInitMaxBlue.SetPos( AccuCast( (pData->InitBlue.x + pData->InitBlue.y) * 255.0f ) );
	m_ctrlInitMaxAlpha.SetPos( AccuCast( (pData->InitAlpha.x + pData->InitAlpha.y) * 255.0f ) );
	
	m_ctrlFinalMinRed.SetPos( AccuCast(pData->FinalRed.x * 255.0f) );
	m_ctrlFinalMinGreen.SetPos( AccuCast(pData->FinalGreen.x * 255.0f) );
	m_ctrlFinalMinBlue.SetPos( AccuCast(pData->FinalBlue.x * 255.0f) );
	m_ctrlFinalMinAlpha.SetPos( AccuCast(pData->FinalAlpha.x * 255.0f) );

	m_ctrlFinalMaxRed.SetPos( AccuCast( (pData->FinalRed.x + pData->FinalRed.y) * 255.0f ) );
	m_ctrlFinalMaxGreen.SetPos( AccuCast( (pData->FinalGreen.x + pData->FinalGreen.y) * 255.0f ) );
	m_ctrlFinalMaxBlue.SetPos( AccuCast( (pData->FinalBlue.x + pData->FinalBlue.y) * 255.0f ) );
	m_ctrlFinalMaxAlpha.SetPos( AccuCast( (pData->FinalAlpha.x + pData->FinalAlpha.y) * 255.0f ) );

	m_ctrlPickMinInitColor.Invalidate();
	m_ctrlPickMinFinalColor.Invalidate();
	m_ctrlPickMaxInitColor.Invalidate();
	m_ctrlPickMaxFinalColor.Invalidate();
	
	// copy our current settings
	m_GameData = *pData;
}

void CColorDlg::OnChangeEditInitMinRed() {
	OnKillfocusEditInitMinRGB();	
}

void CColorDlg::OnChangeEditInitMinGreen() {
	OnKillfocusEditInitMinRGB();
}

void CColorDlg::OnChangeEditInitMinBlue() {
	OnKillfocusEditInitMinRGB();	
}

void CColorDlg::OnChangeEditInitMinAlpha() {
	OnKillfocusEditInitMinAlpha();
}

void CColorDlg::OnChangeEditInitMaxRed() {
	OnKillfocusEditInitMaxRGB();
}

void CColorDlg::OnChangeEditInitMaxGreen() {
	OnKillfocusEditInitMaxRGB();
}

void CColorDlg::OnChangeEditInitMaxBlue() {
	OnKillfocusEditInitMaxRGB();
}

void CColorDlg::OnChangeEditInitMaxAlpha() {
	OnKillfocusEditInitMaxAlpha();
}

void CColorDlg::OnChangeEditFinalMinRed() {
	OnKillfocusEditFinalMinRGB();
}

void CColorDlg::OnChangeEditFinalMinGreen() {
	OnKillfocusEditFinalMinRGB();
}

void CColorDlg::OnChangeEditFinalMinBlue() {
	OnKillfocusEditFinalMinRGB();
}

void CColorDlg::OnChangeEditFinalMinAlpha() {
	OnKillfocusEditFinalMinAlpha();
}

void CColorDlg::OnChangeEditFinalMaxRed() {
	OnKillfocusEditFinalMaxRGB();
}

void CColorDlg::OnChangeEditFinalMaxGreen() {
	OnKillfocusEditFinalMaxRGB();
}

void CColorDlg::OnChangeEditFinalMaxBlue() {
	OnKillfocusEditFinalMaxRGB();
}

void CColorDlg::OnChangeEditFinalMaxAlpha() {
	OnKillfocusEditFinalMaxAlpha();
}

void CColorDlg::OnBnClickedMakeInitColorsEqual() {

	m_ctrlInitMaxRed.SetPos( m_ctrlInitMinRed.GetPos() );
	m_ctrlInitMaxGreen.SetPos( m_ctrlInitMinGreen.GetPos() );
	m_ctrlInitMaxBlue.SetPos( m_ctrlInitMinBlue.GetPos() );
	
	m_ctrlPickMaxInitColor.Invalidate();
}

void CColorDlg::OnBnClickedMakeFinalColorsEqual() {
	
	m_ctrlFinalMaxRed.SetPos( m_ctrlFinalMinRed.GetPos() );
	m_ctrlFinalMaxGreen.SetPos( m_ctrlFinalMinGreen.GetPos() );
	m_ctrlFinalMaxBlue.SetPos( m_ctrlFinalMinBlue.GetPos() );
	
	m_ctrlPickMaxFinalColor.Invalidate();
}

void CColorDlg::OnBnClickedMakeInitAlphaEqual() {
	
	m_ctrlInitMaxAlpha.SetPos( m_ctrlInitMinAlpha.GetPos() );
}

void CColorDlg::OnBnClickedMakeFinalAlphaEqual() {

	m_ctrlFinalMaxAlpha.SetPos( m_ctrlFinalMinAlpha.GetPos() );
}
