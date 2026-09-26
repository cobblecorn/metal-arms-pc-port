//////////////////////////////////////////////////////////////////////////////////////
// MenuTypes.cpp - Menu classes for Mettle Arms.
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
// 02/11/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _ITEM_H_
#define _ITEM_H_ 1

#include "fang.h"
#include "fmath.h"
#include "ftex.h"
#include "flight.h"

class CWeapon;
struct FMesh_t;
class CFMeshInst;

enum// ItemFlags_e
{
	ITEMFLAG_NONE = 0,
	ITEMFLAG_PRIMARY = 1,
	ITEMFLAG_SECONDARY = 2,
	ITEMFLAG_DUAL = ITEMFLAG_PRIMARY | ITEMFLAG_SECONDARY
};

struct ItemInit_s
{
	cchar *pszTag;
	cchar *pszCodeName;
	cwchar *pwszDisplayName;
	cwchar *pwszLongDesc;
	cwchar *pwszUpgradeDesc;
	cchar *pszIconTGAFile;
	cchar *pszMeshFile;

	CFVec2 vecST1, vecST2;

	u32 uInventoryPos_x, uInventoryPos_y;
	u32 uFlags;
	u32 uCurLevel, uMaxLevel;
	u32 uItemType;
	f32 fScale, fYOffset, fZOffset, fPitch_DEG;
	BOOL bPoster;
};


FCLASS_NOALIGN_PREFIX class CItem
{
public:
	CItem();
	~CItem();
	BOOL Init(ItemInit_s *pItemInit);
	void DrawMesh(f32 fThetaY);
	CWeapon *MakeWeapon();
	static cchar *GetWeaponName( const CWeapon *pWeapon );
	BOOL IsDual();
	BOOL IsPrimary();
	BOOL IsSecondary();
	BOOL IsPrimaryOnly();
	BOOL IsSecondaryOnly();

	cchar *m_pszTag;
	cchar *m_pszCodeName;			
	cwchar *m_pwszDisplayName;		
	cwchar *m_pwszLongDesc;
	cwchar *m_pwszUpgradeDesc;

	CFTexInst m_texIcon;
	CFVec2 m_vecST1;
	CFVec2 m_vecST2;
	FMesh_t *m_pMesh;
	CFMeshInst *m_pMI;
	u32 m_uInventoryPos_x;
	u32 m_uInventoryPos_y;
	u32 m_uFlags;
	u32 m_uCurLevel;
	u32 m_uMaxLevel;
	CItem *m_pUpgraded;
	CItem *m_pDowngraded;
	u32 m_uItemType;					// 0..31 are reserved for weapon types.
	f32 m_fScale;
	f32 m_fYOffset;
	f32 m_fZOffset;
	f32 m_fPitch;
//	static CFLight m_ItemLight;

	FCLASS_STACKMEM_NOALIGN(CItem);
} FCLASS_NOALIGN_SUFFIX;
#endif
