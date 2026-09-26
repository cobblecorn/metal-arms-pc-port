//////////////////////////////////////////////////////////////////////////////////////
// BankListCtrl.cpp - an app specific sortable list control
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
#include "BankListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBankListCtrl

CBankListCtrl::CBankListCtrl() : CSortedListCtrl( TRUE, TRUE )
{
}

CBankListCtrl::~CBankListCtrl()
{
}


BEGIN_MESSAGE_MAP(CBankListCtrl, CSortedListCtrl)
	//{{AFX_MSG_MAP(CBankListCtrl)
		ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBankListCtrl message handlers

void CBankListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) {
	LV_DISPINFO *pDispInfo = (LV_DISPINFO*)pNMHDR;
	
	if( pDispInfo->item.mask & LVIF_TEXT ) {
		CBankListItemInfo *pAppItem = (CBankListItemInfo *)(pDispInfo->item.lParam);
		
		switch( pDispInfo->item.iSubItem ) {
		case 0:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetIdString() );
			break;
		case 1:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetFilenameString() );
			break;
		case 2:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetPlayTimeString() );	
			break;
		case 3:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetChannelsString() );	
			break;
		case 4:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetFreqString() );	
			break;		
		}
	}
	*pResult = 0;
}

int CBankListCtrl::CompareItems( CItemInfo *pItemInfo1, CItemInfo *pItemInfo2 ) {
	int nResult;
	u32 nA, nB;
	f32 fA, fB;
	CBankListItemInfo *pInfo1 = (CBankListItemInfo *)(pItemInfo1);
	CBankListItemInfo *pInfo2 = (CBankListItemInfo *)(pItemInfo2);
		
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
	case 1: // Sort on column 'Wav Name'
		nResult = pInfo1->GetFilenameString().CompareNoCase( pInfo2->GetFilenameString() );
		break;
	case 2: // Sort on column 'Play Length'
		fA = pInfo1->GetPlaySecs();
		fB = pInfo2->GetPlaySecs();
		if( fA < fB ) {
			nResult = -1;
		} else if( fA > fB ) {
			nResult = 1;
		} else {
			nResult = 0;
		}
		break;
	case 3: // Sort on column 'Channels'
		nA = pInfo1->GetNumChannels();
		nB = pInfo2->GetNumChannels();
		if( nA < nB ) {
			nResult = -1;
		} else if( nA > nB ) {
			nResult = 1;
		} else {
			nResult = 0;
		}
		break;
	case 4:
		nA = pInfo1->GetFrequency();
		nB = pInfo2->GetFrequency();
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
