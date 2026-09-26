//////////////////////////////////////////////////////////////////////////////////////
// MyListCtrl.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 06/08/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "MyListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl

CMyListCtrl::CMyListCtrl() : CSortedListCtrl( TRUE, TRUE )
{
}

CMyListCtrl::~CMyListCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyListCtrl, CSortedListCtrl)
	//{{AFX_MSG_MAP(CMyListCtrl)
		ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl message handlers

void CMyListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) {
	LV_DISPINFO *pDispInfo = (LV_DISPINFO*)pNMHDR;
	
	if( pDispInfo->item.mask & LVIF_TEXT ) {
		CMyItemInfo *pAppItem = (CMyItemInfo *)(pDispInfo->item.lParam);
		
		switch( pDispInfo->item.iSubItem ) {
		case 0:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetFilename() );
			break;
		case 1:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetFileExtension() );	
			break;
		case 2:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetLibTime() );
			break;
		case 3:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetLocTime() );
			break;
		case 4:
			lstrcpy( pDispInfo->item.pszText, pAppItem->GetMasterTime() );
			break;
		}
	}
	*pResult = 0;
}

int CMyListCtrl::CompareItems( CItemInfo *pItemInfo1, CItemInfo *pItemInfo2 ) {
	int nResult;
	CMyItemInfo *pInfo1 = (CMyItemInfo *)(pItemInfo1);
	CMyItemInfo *pInfo2 = (CMyItemInfo *)(pItemInfo2);
		
	switch( GetSortedColumn() ) {
	case 0: // Sort on column 'Filename'
		nResult = pInfo1->GetFilename().CompareNoCase( pInfo2->GetFilename() );
		break;
	case 1: // Sort on column 'Ext'
		nResult = pInfo1->GetFileExtension().CompareNoCase( pInfo2->GetFileExtension() );
		break;
	case 2: // Sort on column 'Lib'
		nResult = pInfo1->GetLibTime().CompareNoCase( pInfo2->GetLibTime() );
		break;
	case 3: // Sort on column 'Loc'
		nResult = pInfo1->GetLocTime().CompareNoCase( pInfo2->GetLocTime() );
		break;
	case 4: // Sort on column 'Master'
		nResult = pInfo1->GetMasterTime().CompareNoCase( pInfo2->GetMasterTime() );
		break;
	default :
		nResult = 0;
		break;
	}
	return IsAscSorted() ? nResult : -nResult;
}
