//////////////////////////////////////////////////////////////////////////////////////
// FileListCtrl.h - an app specific sortable list control
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
#ifndef _FILE_LIST_CTRL_H_
#define _FILE_LIST_CTRL_H_ 1

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SortedListCtrl.h"
#include "FileListItemInfo.h"


class CFileListCtrl : public CSortedListCtrl
{
// Construction
public:
	CFileListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFileListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFileListCtrl)
	    afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	int CompareItems( CItemInfo *pItemInfo1, CItemInfo *pItemInfo2 );

	DECLARE_MESSAGE_MAP()
};

#endif
