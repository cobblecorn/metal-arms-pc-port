// LifeSpanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "LifeSpanDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

const static double _fMinLifeSpan = 0.01f;

/////////////////////////////////////////////////////////////////////////////
// CLifeSpanDlg dialog

IMPLEMENT_DYNCREATE(CLifeSpanDlg, CDialog)

CLifeSpanDlg::CLifeSpanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLifeSpanDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLifeSpanDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLifeSpanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLifeSpanDlg)
	DDX_Control(pDX, IDC_EDIT_MIN_LIFE_SECS, m_ctrlMinLifeSecs);
	DDX_Control(pDX, IDC_EDIT_MAX_LIFE_SECS, m_ctrlMaxLifeSecs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLifeSpanDlg, CDialog)
	//{{AFX_MSG_MAP(CLifeSpanDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_LIFE_SECS, OnKillfocusEditMinLifeSecs)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_LIFE_SECS, OnKillfocusEditMaxLifeSecs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLifeSpanDlg message handlers

void CLifeSpanDlg::OnKillfocusEditMinLifeSecs() {
	f32 fMax = (f32)m_ctrlMaxLifeSecs.GetTextAsDouble();
	f32 fMin = (f32)m_ctrlMinLifeSecs.GetTextAsDouble( &_fMinLifeSpan );

	if( fMin > fMax ) {
		fMax = fMin;
		CString s;
		s.Format( "%f", fMax );
		m_ctrlMaxLifeSecs.SetText( s );
	}

	m_GameData.OOLifeSecs.Set( 1.0f/fMin, 1.0f/fMax );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLifeSpanDlg::OnKillfocusEditMaxLifeSecs() {
	f32 fMin = (f32)m_ctrlMinLifeSecs.GetTextAsDouble();
	f32 fMax = (f32)m_ctrlMaxLifeSecs.GetTextAsDouble( &_fMinLifeSpan );		

	if( fMax < fMin ) {
		fMin = fMax;
		CString s;
		s.Format( "%f", fMin );
		m_ctrlMinLifeSecs.SetText( s );	
	}

	m_GameData.OOLifeSecs.Set( 1.0f/fMin, 1.0f/fMax );
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

BOOL CLifeSpanDlg::PreTranslateMessage( MSG* pMsg ) {
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

void CLifeSpanDlg::OnOK() {
	//OnCancel();
}

BOOL CLifeSpanDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString sValue;

	m_ctrlMinLifeSecs.SetMaxWholeDigits( 2 );
	m_ctrlMinLifeSecs.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMinLifeSecs.AllowNegative( FALSE );// do this last
	
	m_ctrlMaxLifeSecs.SetMaxWholeDigits( 2 );
	m_ctrlMaxLifeSecs.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlMaxLifeSecs.AllowNegative( FALSE );// do this last

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLifeSpanDlg::SetData( CLifeSpanDlg_Data_t *pData ) {

	CString sValue;

	sValue.Format( "%f", 1.0f/pData->OOLifeSecs.x );
	m_ctrlMinLifeSecs.SetText( sValue );

	sValue.Format( "%f", 1.0f/pData->OOLifeSecs.y );
	m_ctrlMaxLifeSecs.SetText( sValue );

	// copy our current settings
	m_GameData = *pData;
}
