//////////////////////////////////////////////////////////////////////////////////////
// BoneGroupList.h - 
//
// Author: Justin Link      
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
// 08/29/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BONEGROUPLIST_H_
#define _BONEGROUPLIST_H_ 1

#include "fang.h"
#include "stdafx.h"


static const CBoneGroupList_uMaxBoneGroups = 10;

class CBoneGroup;

class CBoneGroupList
{
public:
	CBoneGroupList();
	~CBoneGroupList();

	BOOL Init(u32 uMaxBoneGroups);
	CBoneGroup *AddGroup();
	CBoneGroup *FindGroup(CString strGroupName);

	CBoneGroup *m_paBoneGroup;
	u32 m_uGroupCnt;

private:
	u32 m_uMaxGroups;
};

#endif

