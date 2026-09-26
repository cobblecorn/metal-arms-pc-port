//////////////////////////////////////////////////////////////////////////////////////
// CharacterInfo.h - 
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
// 09/27/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHARACTERINFO_H_
#define _CHARACTERINFO_H_ 1

#include "fang.h"
#include "fanim.h"
#include "BoneGroupList.h"
#include "fsndfx.h"

class CGesture;
class CGestureInst;

#define NUM_SUPPORTED_SLOTS		4

// This is a class that contains all of the information for a specific character type.
// A character type has a mesh and a set of associated animations.
class CCharacterInfo
{
public:
	CCharacterInfo();
	~CCharacterInfo();

	BOOL Init1(u32 uMaxGestures);
	BOOL InsertGesture(CString strName, CString strSrcFile, u32 uFlags);
	BOOL FindGesture(CString strKeyName, CGesture **ppoRetGesture);
	void SetMesh(CString strMeshFileName);
	BOOL Init2();

	void Draw();

	void UnInit();

	CString m_strCharName;

	FSndFx_BankHandle_t m_hBankHandle;// if a bank was listed for this character, here is a handle to the loaded bank

	CString m_strMeshFileName;
	CFAnimMeshRest m_animRest;
	CFMeshInst *m_pMeshInst;
	CFAnimInst *m_pTalkLoopAnim;
	CFAnimCombiner m_oAnimCombiner;
	CFAnimCombinerConfig m_oAnimCombinerConfig;
	// JUSTIN: These will need to change if you add more user slots.
	s32 m_anTapId[NUM_SUPPORTED_SLOTS];
	s32 m_anControlId[NUM_SUPPORTED_SLOTS];

	CBoneGroupList m_oBoneGroups;

	BOOL m_bIsInitialized;
	CGesture *m_aoGestures;
	u32 m_uNumGestures, m_uMaxGestures;
};

// This class is merely a container for a list of character infos
class CCharacterInfoList
{
public:
	CCharacterInfoList();
	~CCharacterInfoList();

	BOOL Init(u32 uMaxCharacters, u32 uMaxGesturePerChar);
	void Uninit();
	CCharacterInfo *FindCharacterInfo(CString strCharName);
	BOOL LoadFromFile(CString strFileName);
	BOOL InitMeshes();

private:
	void Clear();

	BOOL m_bIsInitted;
	CCharacterInfo *m_paCharInfo;
	u32 m_uListSize;				// This is the size of the array that we allocated.
	u32 m_uCharInfoCnt;				// This is the number of slots that we have used.
};

#endif

