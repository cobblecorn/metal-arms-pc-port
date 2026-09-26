// PoiPropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "k9.h"
#include "PoiPropsDlg.h"
#include "AIGraph.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPoiPropsDlg dialog


CPoiPropsDlg::CPoiPropsDlg(CGraphPoi* pPoi, CWnd* pParent /*=NULL*/)
	: CDialog(CPoiPropsDlg::IDD, pParent),
	m_pPoi(pPoi)
{
	//{{AFX_DATA_INIT(CPoiPropsDlg)
	m_bOffensivePoi = ((pPoi->m_uFlags & CGraphPoi::POIFLAG_OFFENSIVE_HINT) !=0);
	m_bCustom = ((pPoi->m_uFlags & CGraphPoi::POIFLAG_CUSTOM_LOS) !=0);
	
	m_bN = pPoi->UnpackVisRating(0)>0;
	m_bNE = pPoi->UnpackVisRating(1)>0;
	m_bE = pPoi->UnpackVisRating(2)>0;
	m_bSE = pPoi->UnpackVisRating(3)>0;
	m_bS = pPoi->UnpackVisRating(4)>0;
	m_bSW = pPoi->UnpackVisRating(5)>0;
	m_bW = pPoi->UnpackVisRating(6)>0;
	m_bNW = pPoi->UnpackVisRating(7)>0;
	m_nRange = pPoi->DistToVisRange(pPoi->GetMax360Range());
	//}}AFX_DATA_INIT
}


void CPoiPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPoiPropsDlg)
	DDX_Check(pDX, IDCB_OFFENSIVEPOI, m_bOffensivePoi);
	DDX_Check(pDX, IDCB_N, m_bN);
	DDX_Check(pDX, IDC_NE, m_bNE);
	DDX_Check(pDX, IDC_E, m_bE);
	DDX_Check(pDX, IDC_SE, m_bSE);
	DDX_Check(pDX, IDC_S, m_bS);
	DDX_Check(pDX, IDC_SW, m_bSW);
	DDX_Check(pDX, IDC_W, m_bW);
	DDX_Check(pDX, IDC_NW, m_bNW);
	DDX_Check(pDX, IDCB_CUSTOM, m_bCustom);
	DDX_Radio(pDX, IDRB_SHORTRANGE, m_nRange);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPoiPropsDlg, CDialog)
	//{{AFX_MSG_MAP(CPoiPropsDlg)
	ON_BN_CLICKED(IDCB_CUSTOM, OnCustom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CPoiPropsDlg::OnCustom() 
{
	m_bCustom = 1-m_bCustom;

	CButton* pEdit = NULL;
	pEdit = (CButton*) GetDlgItem(IDCB_N);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_NE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_E); 
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_SE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_S); 
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_SW);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_W); 
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_NW);
	pEdit->EnableWindow(m_bCustom);

	pEdit = (CButton*) GetDlgItem(IDRB_SHORTRANGE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDRB_MEDRANGE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDRB_LONGRANGE);
	pEdit->EnableWindow(m_bCustom);

}


BOOL CPoiPropsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CButton* pEdit = NULL;
	pEdit = (CButton*) GetDlgItem(IDCB_N);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_NE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_E); 
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_SE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_S); 
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_SW);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_W); 
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDC_NW);
	pEdit->EnableWindow(m_bCustom);

	pEdit = (CButton*) GetDlgItem(IDRB_SHORTRANGE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDRB_MEDRANGE);
	pEdit->EnableWindow(m_bCustom);
	pEdit = (CButton*) GetDlgItem(IDRB_LONGRANGE);
	pEdit->EnableWindow(m_bCustom);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}
