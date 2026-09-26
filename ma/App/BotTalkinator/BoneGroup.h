//////////////////////////////////////////////////////////////////////////////////////
// BoneGroup.h - 
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
#ifndef _BONEGROUP_H_
#define _BONEGROUP_H_ 1

#include "fang.h"
#include "stdafx.h"

class CFAnimCombiner;

class CBoneGroup
{
public:
	CBoneGroup();
	~CBoneGroup();

	BOOL Init(CString strGroupName);
	void AddBone(CString strBoneName);
	void AddCSVBones(CString strBoneNames);

	void EnableBones(CFAnimCombiner *pAnimCombiner, s32 nTapID);

	CString m_strGroupName;
	CStringArray m_astrBoneNames;
};

#endif

