//////////////////////////////////////////////////////////////////////////////////////
// QueryDetailsDlg.cpp - 
//
// Author: Pat MacKellar 
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/10/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "k9.h"
#include "QueryDetailsDlg.h"
#include "DlgStringInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueryDetailsDlg dialog
CQueryDetails *  CNiIterator<class CQueryDetails *>::s_ReturnError;


CQueryDetailsDlg::CQueryDetailsDlg(CNiList<CQueryDetails*> *pSaveList, CWnd* pParent /*=NULL*/)
	: CDialog(CQueryDetailsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQueryDetailsDlg)
	m_bCanJump = FALSE;
	m_b3DMover = FALSE;
	m_bSurfaceMover = FALSE;
	m_fQueryHeight = 0.0f;
	m_fQueryWidth = 0.0f;
	m_fQuerySlopeCost = 0.0f;
	//}}AFX_DATA_INIT
	m_pSaveList = pSaveList;

	m_pCurOnDestroy = NULL;
}


CQueryDetailsDlg::~CQueryDetailsDlg(void)
{
}


void CQueryDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryDetailsDlg)
	DDX_Control(pDX, IDC_CBSAVES, m_SavesComboBox);
	DDX_Check(pDX, IDC_CANJUMP, m_bCanJump);
	DDX_Check(pDX, IDC_3DMOVER, m_b3DMover);
	DDX_Check(pDX, IDC_SURFACEMOVER, m_bSurfaceMover);
	DDX_Text(pDX, IDC_EBQUERYHEIGHT, m_fQueryHeight);
	DDX_Text(pDX, IDC_EBQUERYWIDTH, m_fQueryWidth);
	DDX_Text(pDX, IDC_EBQUERYSLOPECOST, m_fQuerySlopeCost);
	DDV_MinMaxFloat(pDX, m_fQuerySlopeCost, -20.f, 20.f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryDetailsDlg, CDialog)
	//{{AFX_MSG_MAP(CQueryDetailsDlg)
	ON_BN_CLICKED(IDC_BDELETESAVE, OnBDeleteSave)
	ON_BN_CLICKED(IDC_BNEWSAVE, OnBNewSave)
	ON_EN_CHANGE(IDC_EBQUERYWIDTH, OnChangeEbQueryWidth)
	ON_EN_CHANGE(IDC_EBQUERYHEIGHT, OnChangeEbQueryHeight)
	ON_BN_CLICKED(IDC_CANJUMP, OnCanJump)
	ON_CBN_SELCHANGE(IDC_CBSAVES, OnSelchangeCbsaves)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EBQUERYSLOPECOST, OnChangeEbQuerySlopeCost)
	ON_BN_CLICKED(IDC_3DMOVER, On3DMover)
	ON_BN_CLICKED(IDC_SURFACEMOVER, OnSurfaceMover)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CQueryDetailsDlg::UpdateDialog(void)
{
	//defaults, incase something goes wrong
	m_fQueryHeight = 0.0f;
	m_fQueryWidth = 0.0f;
	m_bCanJump = 0;
	m_b3DMover = 0;
	m_bSurfaceMover = 0;

	int nCurSel = m_SavesComboBox.GetCurSel();
	if (nCurSel >= 0)
	{	
		CQueryDetails* pDetails = (CQueryDetails*) m_SavesComboBox.GetItemData(nCurSel);
		FASSERT(pDetails);
		if (pDetails)
		{
			m_fQueryHeight = pDetails->m_fHeight;
			m_fQueryWidth = pDetails->m_fWidth;
			m_fQuerySlopeCost = pDetails->m_fSlopeCost;
			m_bCanJump = pDetails->m_bCanJump;
			m_b3DMover = pDetails->m_b3DMover;
			m_bSurfaceMover = pDetails->m_bSurfaceMover;
		}
	}
	UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CQueryDetailsDlg message handlers

void CQueryDetailsDlg::OnBDeleteSave() 
{
	int nCurSel = m_SavesComboBox.GetCurSel();
	if (nCurSel > 0)
	{	//never delete item 0, it is the default!
		CQueryDetails* pDetails = (CQueryDetails*) m_SavesComboBox.GetItemData(nCurSel);
		FASSERT(pDetails);
		if (pDetails)
		{
			BOOL bDidIt = m_pSaveList->Remove(pDetails);
			FASSERT(bDidIt);
			FASSERT(m_pSaveList->Size()>0);
			m_SavesComboBox.DeleteString(nCurSel);
			m_SavesComboBox.SetCurSel(nCurSel-1);//o.k., because 0 can never be removed
			UpdateDialog();
		}
	}
	else
	{
		MessageBox("Can't delete that!", "Whoops", MB_OK);
	}
	
}


void CQueryDetailsDlg::OnBNewSave() 
{
	DlgStringInput Dlg("NewNameHere", "Enter Config Name", this);

	if (Dlg.DoModal())
	{
		CQueryDetails* pDetails = new CQueryDetails();
		pDetails->Init(Dlg.m_String, 2.0f, 3.0f, CQueryDetails::CAN_JUMP, CQueryDetails::IS_SURFACE_MOVER, !CQueryDetails::IS_3D_MOVER);
		s32 nIndex = m_SavesComboBox.AddString(pDetails->GetName());
		if (nIndex <=0)
		{
			delete pDetails;
		}
		else
		{
			m_SavesComboBox.SetItemData(nIndex, (unsigned long) pDetails);
			UpdateData(FALSE);

			BOOL bDidIt = m_pSaveList->PushTail(pDetails);

			m_SavesComboBox.SetCurSel(nIndex);
			UpdateDialog();
		
		}
	}
}


BOOL CQueryDetailsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (m_pSaveList)
	{
		CNiIterator<CQueryDetails*> it = m_pSaveList->Begin();
		while (it.IsValid())
		{
			CQueryDetails* pDetails = it.Get();
			s32 nIndex = m_SavesComboBox.AddString(pDetails->GetName());
			if (nIndex >=0)
			{
				m_SavesComboBox.SetItemData(nIndex, (unsigned long) pDetails);
			}
			else
			{
				FASSERT(0);
			}
			it.Next();
		}
	}
	
	m_SavesComboBox.SetCurSel(0);

	UpdateDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


CQueryDetails* CQueryDetailsDlg::GetCurSelectedDetails(void)
{
	int nCurSel = m_SavesComboBox.GetCurSel();
	if (nCurSel >= 0)
	{	//never delete item 0, it is the default!
		CQueryDetails* pDetails = (CQueryDetails*) m_SavesComboBox.GetItemData(nCurSel);
		return pDetails;
	}
	return NULL;
}

void CQueryDetailsDlg::OnChangeEbQueryWidth() 
{
	CQueryDetails* pDetails = GetCurSelectedDetails();
	UpdateData(TRUE);
	if (pDetails)
	{
		pDetails->m_fWidth = m_fQueryWidth;
	}
	
}

void CQueryDetailsDlg::OnChangeEbQueryHeight() 
{
	CQueryDetails* pDetails = GetCurSelectedDetails();
	UpdateData(TRUE);
	if (pDetails)
	{
		pDetails->m_fHeight = m_fQueryHeight;
	}
}

void CQueryDetailsDlg::OnChangeEbQuerySlopeCost() 
{
	CQueryDetails* pDetails = GetCurSelectedDetails();
	UpdateData(TRUE);
	if (pDetails)
	{
		pDetails->m_fSlopeCost = m_fQuerySlopeCost;
	}
}

void CQueryDetailsDlg::OnCanJump() 
{
	CQueryDetails* pDetails = GetCurSelectedDetails();
	UpdateData(TRUE);
	if (pDetails)
	{
		pDetails->m_bCanJump = m_bCanJump;
	}
}

void CQueryDetailsDlg::OnSelchangeCbsaves() 
{
	UpdateDialog();
}

void CQueryDetailsDlg::OnDestroy() 
{
	// Called just before window is to be destroyed
	m_pCurOnDestroy = GetCurSelectedDetails();

	CDialog::OnDestroy();
	
	
}


void CQueryDetailsDlg::On3DMover() 
{
	CQueryDetails* pDetails = GetCurSelectedDetails();
	UpdateData(TRUE);
	if (pDetails)
	{
		pDetails->m_b3DMover = m_b3DMover;
	}
	
}

void CQueryDetailsDlg::OnSurfaceMover() 
{
	CQueryDetails* pDetails = GetCurSelectedDetails();
	UpdateData(TRUE);
	if (pDetails)
	{
		pDetails->m_bSurfaceMover = m_bSurfaceMover;
	}
	
}

