//////////////////////////////////////////////////////////////////////////////////////
// BoneGroup.cpp - 
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
#include "BoneGroup.h"

#include "fanim.h"

CBoneGroup::CBoneGroup() {
	m_astrBoneNames.RemoveAll();
	m_strGroupName.Empty();
}

CBoneGroup::~CBoneGroup() {
	m_astrBoneNames.RemoveAll();
	m_strGroupName.Empty();
}

BOOL CBoneGroup::Init( CString strGroupName ) {
	m_strGroupName = strGroupName;
	m_astrBoneNames.RemoveAll();

	return(TRUE);
}

void CBoneGroup::AddBone( CString strBoneName ) {
	m_astrBoneNames.Add(strBoneName);
}

void CBoneGroup::AddCSVBones( CString strBoneNames ) {
	u32 auCharIdx[2];
	auCharIdx[0] = 0;
	auCharIdx[1] = strBoneNames.Find(',', 0);

	CString strBoneName;

	while(auCharIdx[1] != -1)
	{
		strBoneName = strBoneNames.Mid(auCharIdx[0], auCharIdx[1] - auCharIdx[0]);
		AddBone(strBoneName);

		auCharIdx[0] = auCharIdx[1] + 1;
		auCharIdx[1] = strBoneNames.Find(',', auCharIdx[0]);
	}
}

void CBoneGroup::EnableBones( CFAnimCombiner *pAnimCombiner, s32 nTapID ) {
	CFAnimSource *pAnimSource = pAnimCombiner->GetSourceDrivingTap(nTapID);
	
	s32 nBoneNameIdx;
	for( nBoneNameIdx = 0; nBoneNameIdx < m_astrBoneNames.GetSize(); ++nBoneNameIdx ) {
		pAnimCombiner->Mask_UpdateTapBoneMask( nTapID, m_astrBoneNames.GetAt(nBoneNameIdx), TRUE );
	}
}
