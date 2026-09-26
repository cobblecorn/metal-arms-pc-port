// LodDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Spit.h"
#include "LodDlg.h"
#include "SpitDlg.h"

#if 0
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
#endif

static const double _fMinDistance = 1.0;
static const double _fMinEmulationDistance = 0.0;

/////////////////////////////////////////////////////////////////////////////
// CLodDlg dialog

IMPLEMENT_DYNCREATE(CLodDlg, CDialog)

CLodDlg::CLodDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLodDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLodDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLodDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLodDlg)
	DDX_Control(pDX, IDC_EDIT_CULL_DISTANCE, m_ctrlDistance);
	DDX_Control(pDX, IDC_EDIT_EMULATION_DISTANCE, m_ctrlEmulationDistance);
	//}}AFX_DATA_MAP
	DDX_SliderButtonCtrl(pDX, IDC_EDIT_LOD_PERCENT, m_ctrlLODPercent, 0);
}


BEGIN_MESSAGE_MAP(CLodDlg, CDialog)
	//{{AFX_MSG_MAP(CLodDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_CULL_DISTANCE, OnKillfocusEditCullDistance)
	ON_EN_KILLFOCUS(IDC_EDIT_LOD_PERCENT, OnKillfocusEditLodPercent)
	ON_EN_CHANGE(IDC_EDIT_LOD_PERCENT, OnChangeEditLodPercent)
	//}}AFX_MSG_MAP
	ON_MESSAGE(CSP_CLOSEUP, OnSliderClose)
	ON_EN_KILLFOCUS(IDC_EDIT_EMULATION_DISTANCE, OnEnKillfocusEditEmulationdistance)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLodDlg message handlers

void CLodDlg::OnKillfocusEditCullDistance() {
	f32 fCullDistance = (f32)m_ctrlDistance.GetTextAsDouble( &_fMinDistance, NULL );
	
	m_GameData.fCullDist = fCullDistance;
	m_GameData.fStartSkipDrawDist = (f32)m_ctrlLODPercent.GetPos();
	m_GameData.fStartSkipDrawDist *= (1.0f/100.0f);
	m_GameData.fStartSkipDrawDist *= fCullDistance;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLodDlg::OnKillfocusEditLodPercent() {
	f32 fLodPercent = (f32)m_ctrlLODPercent.GetPos();
	fLodPercent *= (1.0f/100.0f);
		
	m_GameData.fStartSkipDrawDist = fLodPercent * m_GameData.fCullDist;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

void CLodDlg::OnEnKillfocusEditEmulationdistance()
{
	f32 fEmulationDistance = (f32)m_ctrlEmulationDistance.GetTextAsDouble( &_fMinEmulationDistance, NULL );
	m_GameData.fEmulationDist = fEmulationDistance;
	Spit_pSpitDlg->DataChanged( &m_GameData );
}

BOOL CLodDlg::PreTranslateMessage( MSG* pMsg ) {
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

void CLodDlg::OnOK() {
	//OnCancel();
}

BOOL CLodDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlLODPercent.SetRange( 1, 99 );
	
	m_ctrlDistance.SetMaxWholeDigits( 6 );
	m_ctrlDistance.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlDistance.AllowNegative( FALSE );// do this last

	m_ctrlEmulationDistance.SetMaxWholeDigits( 6 );
	m_ctrlEmulationDistance.SetMaxDecimalPlaces( SPIT_MAX_DECIMAL_PLACES );
	m_ctrlEmulationDistance.AllowNegative( FALSE );// do this last

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LONG CLodDlg::OnSliderClose( UINT lParam, LONG wParam ) {

	if( wParam == m_ctrlLODPercent.GetDlgCtrlID() ) {
		OnKillfocusEditLodPercent();	
	}

	return 0;
}

void CLodDlg::SetData( CLodDlg_Data_t *pData ) {

	CString sValue;
	
	sValue.Format( "%f", pData->fCullDist );
	m_ctrlDistance.SetText( sValue );

	sValue.Format( "%f", pData->fEmulationDist );
	m_ctrlEmulationDistance.SetText( sValue );

	f32 fPercent = pData->fStartSkipDrawDist / pData->fCullDist;
	m_ctrlLODPercent.SetPos( AccuCast( fPercent * 100.0f ) );

	// copy our current settings
	m_GameData = *pData;
}

void CLodDlg::OnChangeEditLodPercent() {
	OnKillfocusEditLodPercent();	
}
