// PickSfxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "bottalkinator.h"
#include "PickSfxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPickSfxDlg dialog


CPickSfxDlg::CPickSfxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPickSfxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPickSfxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPickSfxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPickSfxDlg)
	DDX_Control(pDX, IDC_SFX_COMBO, m_ctrlSfxCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPickSfxDlg, CDialog)
	//{{AFX_MSG_MAP(CPickSfxDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPickSfxDlg message handlers

void CPickSfxDlg::OnOK() {
	
	// grab the current selected text
	m_ctrlSfxCombo.GetLBText( m_ctrlSfxCombo.GetCurSel(), m_sUserSelection );
	
	CDialog::OnOK();
}

BOOL CPickSfxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	m_ctrlSfxCombo.ResetContent();
	u32 nNumSfxs = fsndfx_GetNumFxsInBank( m_hSfxBank );
	FASSERT( nNumSfxs );
	u32 i;
	FSndFx_FxHandle_t hSfx;
	cchar *pszSfxname;
	for( i=0; i < nNumSfxs; i++ ) {
		hSfx = fsndfx_GetFxHandle( m_hSfxBank, i );
		pszSfxname = fsndfx_ConvertFxHandleToName( hSfx );
		FASSERT( pszSfxname );
		m_ctrlSfxCombo.AddString( pszSfxname );
	}

	// set the current selection
	if( m_sUserSelection.IsEmpty() ) {
		m_ctrlSfxCombo.SetCurSel( 0 );
	} else {
		s32 nIndex = m_ctrlSfxCombo.FindString( -1, m_sUserSelection );
		if( nIndex < 0 ) {
			nIndex = 0;
		}
		m_ctrlSfxCombo.SetCurSel( nIndex );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CPickSfxDlg::DoModal() {
	
	if( m_hSfxBank == FSNDFX_INVALID_BANK_HANDLE ) {
		return IDCANCEL;
	}
		
	return CDialog::DoModal();
}
