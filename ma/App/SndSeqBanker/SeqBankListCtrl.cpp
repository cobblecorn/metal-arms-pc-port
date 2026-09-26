//////////////////////////////////////////////////////////////////////////////////////
// SeqBankListCtrl.cpp - an app specific sortable list control
//
// Author: Michael Starich   
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
// 06/04/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "SeqBankListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeqBankListCtrl

CSeqBankListCtrl::CSeqBankListCtrl() : CSortedListCtrl( TRUE, TRUE )
{
}

CSeqBankListCtrl::~CSeqBankListCtrl()
{
}


BEGIN_MESSAGE_MAP(CSeqBankListCtrl, CSortedListCtrl)
	//{{AFX_MSG_MAP(CSeqBankListCtrl)
		ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSeqBankListCtrl message handlers

void CSeqBankListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) {
	LV_DISPINFO *pDispInfo = (LV_DISPINFO*)pNMHDR;
	
	if( pDispInfo->item.mask & LVIF_TEXT ) {
		CSeqBankListItemInfo *pAppItem = (CSeqBankListItemInfo *)(pDispInfo->item.lParam);
		
		switch( pDispInfo->item.iSubItem ) {
		case 0:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetIdString() );
			break;
		case 1:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetNameString() );
			break;
		case 2:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetNumCmdsString() );	
			break;
		}
	}
	*pResult = 0;
}

int CSeqBankListCtrl::CompareItems( CItemInfo *pItemInfo1, CItemInfo *pItemInfo2 ) {
	int nResult;
	u32 nA, nB;
	CSeqBankListItemInfo *pInfo1 = (CSeqBankListItemInfo *)(pItemInfo1);
	CSeqBankListItemInfo *pInfo2 = (CSeqBankListItemInfo *)(pItemInfo2);
		
	switch( GetSortedColumn() ) {

	case 0: // Sort on column 'Id'
		nA = pInfo1->GetId();
		nB = pInfo2->GetId();
		if( nA < nB ) {
			nResult = -1;
		} else if( nA > nB ) {
			nResult = 1;
		} else {
			nResult = 0;
		}
		break;
	case 1: // Sort on column 'Name'
		nResult = pInfo1->GetNameString().CompareNoCase( pInfo2->GetNameString() );
		break;
	case 2: // Sort on column '# Commands'
		nA = pInfo1->GetNumCmds();
		nB = pInfo2->GetNumCmds();
		if( nA < nB ) {
			nResult = -1;
		} else if( nA > nB ) {
			nResult = 1;
		} else {
			nResult = 0;
		}
		break;	
	default :
		nResult = 0;
		break;
	}
	return IsAscSorted() ? nResult : -nResult;
}
