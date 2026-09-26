//////////////////////////////////////////////////////////////////////////////////////
// BoneGroupList.cpp - 
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
#include "stdafx.h"
#include "fang.h"
#include "BoneGroupList.h"
#include "BoneGroup.h"


CBoneGroupList::CBoneGroupList() {
	m_uGroupCnt = 0;
	m_uMaxGroups = 0;

	m_paBoneGroup = NULL;
}

CBoneGroupList::~CBoneGroupList() {
	m_uGroupCnt = 0;
	m_uMaxGroups = 0;

	delete [] m_paBoneGroup;
	m_paBoneGroup = NULL;
}

BOOL CBoneGroupList::Init( u32 uMaxBoneGroups ) {

	m_paBoneGroup = new CBoneGroup[uMaxBoneGroups];
	if( m_paBoneGroup == NULL ) {
		AfxMessageBox("CBoneGroupList::Init() : Error: Out of memory.\n");
		return FALSE;
	}

	m_uGroupCnt = 0;
	m_uMaxGroups = uMaxBoneGroups;

	return TRUE;
}

CBoneGroup *CBoneGroupList::AddGroup() {

	if( m_uGroupCnt == m_uMaxGroups ) {
		return NULL;
	}

	CBoneGroup *pBG = &m_paBoneGroup[m_uGroupCnt];
	++m_uGroupCnt;

	return pBG;
}

CBoneGroup *CBoneGroupList::FindGroup( CString strGroupName ) {
	u32 uCurGroupIdx;

	for( uCurGroupIdx = 0; uCurGroupIdx < m_uGroupCnt; ++uCurGroupIdx ) {
		if( m_paBoneGroup[uCurGroupIdx].m_strGroupName.CompareNoCase( strGroupName ) == 0 ) {
			return &m_paBoneGroup[uCurGroupIdx];
		}
	}

	return NULL;
}
