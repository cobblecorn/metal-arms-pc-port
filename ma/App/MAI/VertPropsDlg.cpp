//
// VertPropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "k9.h"
#include "VertPropsDlg.h"
#include "MaiEditGraph.h"
#include "maimain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVertPropsDlg dialog


CVertPropsDlg::CVertPropsDlg(GraphVert* pV, CWnd* pParent /*=NULL*/)
	: CDialog(CVertPropsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVertPropsDlg)
	m_nVertId = maimain_pEditGraph->GetVertId(pV);
	m_bCustom = ((pV->m_nProps & VERTPROP_CUSTOM_VOL));
	m_fCustomHeight = pV->m_fHeightClearance;
	m_fCustomRadius = pV->GetRad();
	//}}AFX_DATA_INIT
	m_bCustomVol = m_bHadCustomVol = m_bCustom;
}


BOOL CVertPropsDlg::NeedToRecalcVol(void)
{
	return m_bHadCustomVol && !m_bCustomVol;
}

void CVertPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVertPropsDlg)
	DDX_Text(pDX, IDC_EBVERTID, m_nVertId);
	DDX_Text(pDX, IDC_EBHEIGHT, m_fCustomHeight);
	DDX_Text(pDX, IDC_EBWIDTH, m_fCustomRadius);
	DDX_Check(pDX, IDC_CUSTOMVOL, m_bCustom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVertPropsDlg, CDialog)
	//{{AFX_MSG_MAP(CVertPropsDlg)
	ON_BN_CLICKED(IDC_CUSTOMVOL, OnCustomvol)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVertPropsDlg message handlers

BOOL CVertPropsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CEdit* pEBWidth = (CEdit *) GetDlgItem( IDC_EBWIDTH);
	CEdit* pEBHeight = (CEdit *) GetDlgItem( IDC_EBHEIGHT);
	pEBWidth->SetReadOnly(1-m_bCustomVol);
	pEBHeight->SetReadOnly(1-m_bCustomVol);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVertPropsDlg::OnCustomvol() 
{
	CEdit* pEBWidth = (CEdit *) GetDlgItem( IDC_EBWIDTH);
	CEdit* pEBHeight = (CEdit *) GetDlgItem( IDC_EBHEIGHT);
	m_bCustomVol = 1-m_bCustomVol;
	pEBWidth->SetReadOnly(1-m_bCustomVol);
	pEBHeight->SetReadOnly(1-m_bCustomVol);
	
}
