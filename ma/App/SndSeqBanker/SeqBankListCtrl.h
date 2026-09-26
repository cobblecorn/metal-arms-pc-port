//////////////////////////////////////////////////////////////////////////////////////
// SeqBankListCtrl.h - an app specific sortable list control
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
#ifndef _SEQ_BANK_LIST_CTRL_H_
#define _SEQ_BANK_LIST_CTRL_H_ 1

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SortedListCtrl.h"
#include "SeqBankListItemInfo.h"


class CSeqBankListCtrl : public CSortedListCtrl
{
// Construction
public:
	CSeqBankListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSeqBankListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSeqBankListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSeqBankListCtrl)
	    afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	int CompareItems( CItemInfo *pItemInfo1, CItemInfo *pItemInfo2 );

	DECLARE_MESSAGE_MAP()
};

#endif
