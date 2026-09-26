//////////////////////////////////////////////////////////////////////////////////////
// BankListCtrl.h - an app specific sortable list control
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
#ifndef _BANK_LIST_CTRL_H_
#define _BANK_LIST_CTRL_H_ 1

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SortedListCtrl.h"
#include "BankListItemInfo.h"


class CBankListCtrl : public CSortedListCtrl
{
// Construction
public:
	CBankListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBankListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBankListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBankListCtrl)
	    afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	int CompareItems( CItemInfo *pItemInfo1, CItemInfo *pItemInfo2 );

	DECLARE_MESSAGE_MAP()
};

#endif
