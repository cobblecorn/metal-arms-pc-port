// ScaleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "ScaleDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

static const double _fMinScale = 0.0001;

/////////////////////////////////////////////////////////////////////////////
// CScaleDlg dialog

IMPLEMENT_DYNCREATE(CScaleDlg, CDialog)

CScaleDlg::CScaleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScaleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScaleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CScaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScaleDlg)
	DDX_Control(pDX, IDC_EDIT_MIN_INIT_SCALE, m_ctrlMinInitScale);
	DDX_Control(pDX, IDC_EDIT_MIN_FINAL_SCALE, m_ctrlMinFinalScale);
	DDX_Control(pDX, IDC_EDIT_MAX_INIT_SCALE, m_ctrlMaxInitScale);
	DDX_Control(pDX, IDC_EDIT_MAX_FINAL_SCALE, m_ctrlMaxFinalScale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScaleDlg, CDialog)
	//{{AFX_MSG_MAP(CScaleDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_INIT_SCALE, OnKillfocusEditMinInitScale)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_FINAL_SCALE, OnKillfocusEditMinFinalScale)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_INIT_SCALE, OnKillfocusEditMaxInitScale)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_FINAL_SCALE, OnKillfocusEditMaxFinalScale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScaleDlg message handlers

void CScaleDlg::OnKillfocusEditMinInitScale() {
	f32 fMax = (f32)m_ctrlMaxInitScale.GetTextAsDouble();
	f32 fMin = (f32)m_ctrlMinInitScale.GetTextAsDouble( &_fMinScale, NULL );	

	if( fMin > fMax ) {
		fMax = fMin;
		CString s;
		s.Format( "%f", fMax );
		m_ctrlMaxInitScale.SetText( s );
	}

	m_GameData.InitScale.Set( fMin, fMax-fMin );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CScaleDlg::OnKillfocusEditMaxInitScale() {
	f32 fMin = (f32)m_ctrlMinInitScale.GetTextAsDouble();
	f32 fMax = (f32)m_ctrlMaxInitScale.GetTextAsDouble( &_fMinScale, NULL );	

	if( fMax < fMin ) {
		fMin = fMax;
		CString s;
		s.Format( "%f", fMin );
		m_ctrlMinInitScale.SetText( s );
	}

	m_GameData.InitScale.Set( fMin, fMax-fMin );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CScaleDlg::OnKillfocusEditMinFinalScale() {
	f32 fMax = (f32)m_ctrlMaxFinalScale.GetTextAsDouble();
	f32 fMin = (f32)m_ctrlMinFinalScale.GetTextAsDouble( &_fMinScale, NULL );
	
	if( fMin > fMax ) {
		fMax = fMin;
		CString s;
		s.Format( "%f", fMax );
		m_ctrlMaxFinalScale.SetText( s );
	}

	m_GameData.FinalScale.Set( fMin, fMax-fMin );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CScaleDlg::OnKillfocusEditMaxFinalScale() {
	f32 fMin = (f32)m_ctrlMinFinalScale.GetTextAsDouble();
	f32 fMax = (f32)m_ctrlMaxFinalScale.GetTextAsDouble( &_fMinScale, NULL );		

	if( fMax < fMin ) {
		fMin = fMax;
		CString s;
		s.Format( "%f", fMin );
		m_ctrlMinFinalScale.SetText( s );
	}

	m_GameData.FinalScale.Set( fMin, fMax-fMin );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CScaleDlg::OnOK() {
	//OnCancel();
}

BOOL CScaleDlg::PreTranslateMessage( MSG* pMsg ) {
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

BOOL CScaleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlMinInitScale.SetMaxWholeDigits( 4 );
	m_ctrlMinInitScale.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMinInitScale.AllowNegative( FALSE );// do this last
	
	m_ctrlMaxInitScale.SetMaxWholeDigits( 4 );
	m_ctrlMaxInitScale.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMaxInitScale.AllowNegative( FALSE );// do this last
	
	m_ctrlMinFinalScale.SetMaxWholeDigits( 4 );
	m_ctrlMinFinalScale.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMinFinalScale.AllowNegative( FALSE );// do this last
	
	m_ctrlMaxFinalScale.SetMaxWholeDigits( 4 );
	m_ctrlMaxFinalScale.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMaxFinalScale.AllowNegative( FALSE );// do this last
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScaleDlg::SetData( CScaleDlg_Data_t *pData ) {

	CString sValue;
	
	sValue.Format( "%f", pData->InitScale.x );
	m_ctrlMinInitScale.SetText( sValue );

	sValue.Format( "%f", (pData->InitScale.x + pData->InitScale.y) );
	m_ctrlMaxInitScale.SetText( sValue );
	
	sValue.Format( "%f", pData->FinalScale.x );
	m_ctrlMinFinalScale.SetText( sValue );

	sValue.Format( "%f", (pData->FinalScale.x + pData->FinalScale.y) );
	m_ctrlMaxFinalScale.SetText( sValue );

	// copy our current settings
	m_GameData = *pData;
}
