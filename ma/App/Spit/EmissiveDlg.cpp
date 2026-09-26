// EmissiveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "EmissiveDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmissiveDlg dialog

IMPLEMENT_DYNCREATE(CEmissiveDlg, CDialog)

CEmissiveDlg::CEmissiveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEmissiveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmissiveDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEmissiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmissiveDlg)
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_MIN_EMISSIVE, m_ctrlMin, 0);
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_MAX_EMISSIVE, m_ctrlMax, 0);
}


BEGIN_MESSAGE_MAP(CEmissiveDlg, CDialog)
	//{{AFX_MSG_MAP(CEmissiveDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_MAX_EMISSIVE, OnKillfocusEditMaxEmissive)
	ON_EN_KILLFOCUS(IDC_EDIT_MIN_EMISSIVE, OnKillfocusEditMinEmissive)	
	ON_EN_CHANGE(IDC_EDIT_MAX_EMISSIVE, OnChangeEditMaxEmissive)
	ON_EN_CHANGE(IDC_EDIT_MIN_EMISSIVE, OnChangeEditMinEmissive)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmissiveDlg message handlers

BOOL CEmissiveDlg::PreTranslateMessage( MSG* pMsg ) {
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

BOOL CEmissiveDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlMin.SetRange( 0, 100 );
	
	m_ctrlMax.SetRange( 0, 100 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmissiveDlg::OnOK() {
	//OnCancel();
}

void CEmissiveDlg::OnKillfocusEditMaxEmissive() {
	
	// if max is less than min, adjust the min value
	int nMin = m_ctrlMin.GetPos();
	int nMax = m_ctrlMax.GetPos();

	if( nMax < nMin ) {
		nMin = nMax;
		m_ctrlMin.SetPos( nMin );
	}
	
	m_GameData.EmissiveIntensity.x = (f32)nMin;
	m_GameData.EmissiveIntensity.x *= (1.0f/100.0f);

	m_GameData.EmissiveIntensity.y = (f32)(nMax - nMin);
	m_GameData.EmissiveIntensity.y *= (1.0f/100.0f);
	
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CEmissiveDlg::OnKillfocusEditMinEmissive() {

	// if min is greater than max, adjust the max value
	int nMin = m_ctrlMin.GetPos();
	int nMax = m_ctrlMax.GetPos();

	if( nMin > nMax ) {
		nMax = nMin;
		m_ctrlMax.SetPos( nMax );
	}

	m_GameData.EmissiveIntensity.x = (f32)nMin;
	m_GameData.EmissiveIntensity.x *= (1.0f/100.0f);

	m_GameData.EmissiveIntensity.y = (f32)(nMax - nMin);
	m_GameData.EmissiveIntensity.y *= (1.0f/100.0f);
	
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

LONG CEmissiveDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlMax.GetDlgCtrlID() ) {
		OnKillfocusEditMaxEmissive();	
	} else if( wParam == m_ctrlMin.GetDlgCtrlID() ) {
		OnKillfocusEditMinEmissive();
	}

	return 0;
}

void CEmissiveDlg::SetData( CEmissiveDlg_Data_t *pData ) {

	m_ctrlMin.SetPos( AccuCast( pData->EmissiveIntensity.x * 100.0f ) );

	m_ctrlMax.SetPos( AccuCast( (pData->EmissiveIntensity.x + pData->EmissiveIntensity.y) * 100.0f ) );

	// copy our current settings
	m_GameData = *pData;
}

void CEmissiveDlg::OnChangeEditMaxEmissive() {
	OnKillfocusEditMaxEmissive();
}

void CEmissiveDlg::OnChangeEditMinEmissive() {
	OnKillfocusEditMinEmissive();
}
