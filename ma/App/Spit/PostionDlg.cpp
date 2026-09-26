// PostionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spit.h"
#include "PostionDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CPostionDlg dialog

IMPLEMENT_DYNCREATE(CPostionDlg, CDialog)

CPostionDlg::CPostionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPostionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPostionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPostionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPostionDlg)
	DDX_Control(pDX, IDC_EDIT_JITTER_Z, m_ctrlJitterZ);
	DDX_Control(pDX, IDC_EDIT_JITTER_Y, m_ctrlJitterY);
	DDX_Control(pDX, IDC_EDIT_JITTER_X, m_ctrlJitterX);
	DDX_Control(pDX, IDC_EDIT_INITIAL_POS_RADIUS, m_ctrlPosRadius);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPostionDlg, CDialog)
	//{{AFX_MSG_MAP(CPostionDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_INITIAL_POS_RADIUS, OnKillfocusEditInitialPosRadius)
	ON_EN_KILLFOCUS(IDC_EDIT_JITTER_X, OnKillfocusEditJitterX)
	ON_EN_KILLFOCUS(IDC_EDIT_JITTER_Y, OnKillfocusEditJitterY)
	ON_EN_KILLFOCUS(IDC_EDIT_JITTER_Z, OnKillfocusEditJitterZ)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPostionDlg message handlers

void CPostionDlg::OnKillfocusEditInitialPosRadius() {
	f32 fRadius = (f32)m_ctrlPosRadius.GetTextAsDouble();
	
	m_GameData.fRandomInitPosRadius = fRadius;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

BOOL CPostionDlg::PreTranslateMessage( MSG* pMsg ) {
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

void CPostionDlg::OnOK() {
	//OnCancel();
}

BOOL CPostionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlPosRadius.SetMaxWholeDigits( 4 );
	m_ctrlPosRadius.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlPosRadius.AllowNegative( FALSE );// do this last

	m_ctrlJitterX.SetMaxWholeDigits( 4 );
	m_ctrlJitterX.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlJitterX.AllowNegative( FALSE );// do this last

	m_ctrlJitterY.SetMaxWholeDigits( 4 );
	m_ctrlJitterY.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlJitterY.AllowNegative( FALSE );// do this last

	m_ctrlJitterZ.SetMaxWholeDigits( 4 );
	m_ctrlJitterZ.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlJitterZ.AllowNegative( FALSE );// do this last
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPostionDlg::SetData( CPositionDlg_Data_t *pData ) {

	CString sValue;
	
	sValue.Format( "%f", pData->fRandomInitPosRadius );
	m_ctrlPosRadius.SetText( sValue );

	sValue.Format( "%f", pData->JitterOffset.x );
	m_ctrlJitterX.SetText( sValue );

	sValue.Format( "%f", pData->JitterOffset.y );
	m_ctrlJitterY.SetText( sValue );

	sValue.Format( "%f", pData->JitterOffset.z );
	m_ctrlJitterZ.SetText( sValue );

	// copy our current settings
	m_GameData = *pData;
}

void CPostionDlg::OnKillfocusEditJitterX() {
	f32 fX = (f32)m_ctrlJitterX.GetTextAsDouble();
	
	m_GameData.JitterOffset.x = fX;
	Spit_pSpitDlg->DataChanged( &m_GameData );	
}

void CPostionDlg::OnKillfocusEditJitterY() {
	f32 fY = (f32)m_ctrlJitterY.GetTextAsDouble();
	
	m_GameData.JitterOffset.y = fY;
	Spit_pSpitDlg->DataChanged( &m_GameData );		
}

void CPostionDlg::OnKillfocusEditJitterZ() {
	f32 fZ = (f32)m_ctrlJitterZ.GetTextAsDouble();
	
	m_GameData.JitterOffset.z = fZ;
	Spit_pSpitDlg->DataChanged( &m_GameData );		
}
