// EdgePropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "k9.h"
#include "EdgePropsDlg.h"
#include "MaiEditGraph.h"
#include "DlgStringInput.h"
#include "Maimain.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEdgePropsDlg dialog
void _InitHazardCombo(CComboBox* pCombo, u8 uCurHazardId);


CEdgePropsDlg::CEdgePropsDlg(int nVertId, GraphEdge* pEdge, CWnd* pParent /*=NULL*/)
	: CDialog(CEdgePropsDlg::IDD, pParent)
{


	//{{AFX_DATA_INIT(CEdgePropsDlg)
	m_bHazardDoor =	((pEdge->m_nProps & EDGEPROP_HAZARD_DOOR) !=0);
	m_bHazardLift =	((pEdge->m_nProps & EDGEPROP_HAZARD_LIFT) !=0); 
	m_bCustom =	((pEdge->m_nProps & EDGEPROP_CUSTOM_VOL) !=0);
	m_bDownOnly = ((pEdge->m_nProps & EDGEPROP_DOWN_ONLY) !=0);
	m_nHazardId = pEdge->m_nHazardId;
	m_fCustomHeight = pEdge->m_fMaxSafeHeight;
	m_fCustomWidth = pEdge->m_fHalfWidth;
	m_bUpOnly = ((pEdge->m_nProps & EDGEPROP_UP_ONLY) !=0);
	m_bHazardBridge = ((pEdge->m_nProps & EDGEPROP_HAZARD_BRIDGE) !=0);
	m_bHazardDestructable = ((pEdge->m_nProps & EDGEPROP_HAZARD_DESTRUCTABLE) !=0);
	//}}AFX_DATA_INIT
	m_bCustomVol = m_bHadCustomVol = m_bCustom;
	m_pGraphEdge = pEdge;
	m_uJumpType  = pEdge->m_nProps & EDGEPROP_JUMP;
}


void CEdgePropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEdgePropsDlg)
	DDX_Check(pDX, IDC_CHECKDOOR, m_bHazardDoor);
	DDX_Check(pDX, IDC_CHECKLIFT, m_bHazardLift);
	DDX_Text(pDX, IDC_EBHAZARDID, m_nHazardId);
	DDX_Text(pDX, IDC_EBHEIGHT, m_fCustomHeight);
	DDX_Text(pDX, IDC_EBWIDTH, m_fCustomWidth);
	DDX_Check(pDX, IDC_CUSTOMVOL, m_bCustom);
	DDX_Check(pDX, IDC_DROPOFF, m_bDownOnly);
	DDX_Check(pDX, IDC_UPONLY, m_bUpOnly);
	DDX_Check(pDX, IDC_DESTRUCT, m_bHazardDestructable);
	DDX_Check(pDX, IDC_BRIDGE, m_bHazardBridge);
	DDX_Control(pDX, IDC_JUMPTYPECOMBO, m_JumpTypesCombo);
	DDX_Control(pDX, IDC_HAZARDCOMBO, m_HazardCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEdgePropsDlg, CDialog)
	//{{AFX_MSG_MAP(CEdgePropsDlg)
	ON_BN_CLICKED(IDC_CUSTOMVOL, OnCustomVol)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SELECTHAZARDBUTTON, OnBnClickedSelecthazardbutton)
	ON_CBN_SELCHANGE(IDC_HAZARDCOMBO, OnCbnEditchangeHazardcombo)
END_MESSAGE_MAP()


BOOL CEdgePropsDlg::NeedToRecalcVol(void)
{
	return m_bHadCustomVol && !m_bCustomVol;
}

/////////////////////////////////////////////////////////////////////////////
// CEdgePropsDlg message handlers

BOOL CEdgePropsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CEdit* pEBWidth = (CEdit *) GetDlgItem( IDC_EBWIDTH);
	CEdit* pEBHeight = (CEdit *) GetDlgItem( IDC_EBHEIGHT);
	pEBWidth->SetReadOnly(1-m_bCustomVol);
	pEBHeight->SetReadOnly(1-m_bCustomVol);

	m_JumpTypesCombo.ResetContent();
	m_JumpTypesCombo.SetItemData(m_JumpTypesCombo.AddString("No Jump"), 0);
	m_JumpTypesCombo.SetItemData(m_JumpTypesCombo.AddString("Jump Obstacle"), JUMPTYPE_OBSTACLE);
	m_JumpTypesCombo.SetItemData(m_JumpTypesCombo.AddString("Jump Ledge"), JUMPTYPE_LEDGE);
	m_JumpTypesCombo.SetItemData(m_JumpTypesCombo.AddString("Jump Vert To Vert"), JUMPTYPE_VERT_2_VERT);
	m_JumpTypesCombo.SetItemData(m_JumpTypesCombo.AddString("Hover Vert To Vert"), JUMPTYPE_HOVER);

	u32 uItem = 0;
	for (s32 i = 0; i < m_JumpTypesCombo.GetCount(); i++)
	{
		if (m_JumpTypesCombo.GetItemData(i) == (m_pGraphEdge->m_nProps & EDGEPROP_JUMP))
		{
			m_JumpTypesCombo.SetCurSel(i);
			break;
		}
	}

	_InitHazardCombo(&m_HazardCombo, m_nHazardId);


	if (this->m_pGraphEdge->Is3D())
	{
		int anDisableWnds[] = {IDC_CHECKDOOR, IDC_CHECKLIFT,IDC_BRIDGE   ,IDC_DESTRUCT   ,IDC_DROPOFF,  IDC_UPONLY  ,IDC_JUMPTYPECOMBO};

		for (u32 j = 0; j < sizeof(anDisableWnds)/sizeof(int); j++)
		{
			CWnd* pWnd = GetDlgItem( anDisableWnds[j]);
			if (pWnd)
			{
				pWnd->EnableWindow(FALSE);
			}
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void _InitHazardCombo(CComboBox* pCombo, u8 uCurHazardId)
{
	pCombo->ResetContent();
	u32 uICur = 0;
	for (s32 i = 0; i < maimain_pEditGraph->GetNumHazards(); i++)
	{
		pCombo->SetItemData(pCombo->AddString(maimain_pEditGraph->GetHazardName(i)), i);
		if (i == uCurHazardId)
		{
			uICur = i;
		}
	}
	pCombo->SetCurSel(uICur);
}


void CEdgePropsDlg::OnOK(void)
{
	CDialog::OnOK();
	if (m_JumpTypesCombo.GetCurSel() >= 0)
	{
		m_uJumpType = (u16) m_JumpTypesCombo.GetItemData(m_JumpTypesCombo.GetCurSel());
	}
}

void CEdgePropsDlg::OnCustomVol() 
{

	CEdit* pEBWidth = (CEdit *) GetDlgItem( IDC_EBWIDTH);
	CEdit* pEBHeight = (CEdit *) GetDlgItem( IDC_EBHEIGHT);
	m_bCustomVol = 1-m_bCustomVol;
	pEBWidth->SetReadOnly(1-m_bCustomVol);
	pEBHeight->SetReadOnly(1-m_bCustomVol);
}


void CEdgePropsDlg::OnBnClickedSelecthazardbutton()
{
	DlgStringInput Dlg("HAZ01", "Enter Hazard Name", this);
	if (Dlg.DoModal())
	{
		maimain_pEditGraph->AddHazard(Dlg.m_String);
		_InitHazardCombo(&m_HazardCombo, m_nHazardId);   //might be new options to add to the list
		UpdateData(FALSE);
	}

}

void CEdgePropsDlg::OnCbnEditchangeHazardcombo()
{
	this->m_nHazardId = m_HazardCombo.GetCurSel();
	UpdateData(FALSE);
}
