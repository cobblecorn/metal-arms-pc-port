//////////////////////////////////////////////////////////////////////////////////////
// GoodieBag.h - Goodie bag class for Mettle Arms.
//
// Author: Justine Lank
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
// 05/16/02	Link		Created.
// 10.28.03 pgm			fixed up swarmer type goodies 
// 10.28.03 pgm			moved some creation time funcs from header file into cpp file

//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GOODIEBAG_H_
#define _GOODIEBAG_H_ 1

#include "fang.h"

class CEntity;

const u32 GoodieBag_uMaxNumGoodieInsts = 4;

// NKM
// A GoodieType_e is now just a collectable type
typedef u32 GoodieType_e;

/*enum GoodieType_e
{
	GOODIETYPE_WASHER = 0,
	GOODIETYPE_CORINGCHARGE,
	GOODIETYPE_CHIP,
	GOODIETYPE_SWARMER,
//	GOODIETYPE_AMMO1,
	GOODIETYPE_WEAPON1,
	GOODIETYPE_PUP_ENERGY,

	GOODIETYPE_COUNT
};*/



enum GoodieBagAngVelType_e
{
	GOODIEBAGANGVELTYPE_NONE = 0,
	GOODIEBAGANGVELTYPE_RANDOM,

	GOODIEBAGANGVELTYPE_COUNT
};



FCLASS_NOALIGN_PREFIX class CGoodieInst
{
public:
	static GoodieType_e GetGoodieTypeFromString(cchar *pszGoodieName);

	CGoodieInst() { m_bCreated = FALSE; m_papEntities = NULL; m_eAngVelType = GOODIEBAGANGVELTYPE_NONE; }
	~CGoodieInst();

	f32 GetProb() { return(m_fProb); }
	GoodieType_e GetGoodieType() { return(m_eGoodieType); }

	CEntity *GetEntity(u32 uNum);

	// WARNING: This function will not necessarily return the same value on successive calls!
	// Call it once, and save the value that it returns for you own uses.
	u32 GetQuant();

	// This creates a simple GoodieInst that always gives a fixed number of
	//   a certain item.
	void Create(GoodieType_e eGoodieType, u32 uQuantity);

	// This is the most likely create to use.
	void Create(GoodieType_e eGoodieType, u32 uQuantity, f32 fProb);

	// This creates a slightly less simple GoodieInst that always gives off its
	//   goodie but in varying quantities.
	void Create(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2);

	void Create(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2, f32 fProb, GoodieBagAngVelType_e eAngVelType = GOODIEBAGANGVELTYPE_NONE);

	void CloneFromOther(CGoodieInst *pGoodieInst);

	// Call this once the goodie inst is in it's final resting spot.
	BOOL CreateEntities();

	void ClearEntities();

	GoodieType_e m_eGoodieType;
	u32 m_auQuantity[2];
	f32 m_fProb;
	GoodieBagAngVelType_e m_eAngVelType;

private:

	BOOL m_bCreated;

	CEntity **m_papEntities;		// These get created by CreateEntities() if appropriate.
								// If created, there will be m_auQuantity[1] of them.

	FCLASS_STACKMEM_NOALIGN(CGoodieInst);
} FCLASS_NOALIGN_SUFFIX;


enum GoodieBagBrain_e
{
	GOODIEBAGBRAIN_ANY = 0,
	GOODIEBAGBRAIN_ONE,

	GOODIEBAGBRAIN_COUNT
};


FCLASS_NOALIGN_PREFIX class CGoodieBag
{
public:
	CGoodieBag();

	void Reset();

	// Call this once the goodie bag is in it's final resting spot.
	void CreateEntities();
	u32 NumGoodies() { return(m_uNumGoodies); }
	void ReleaseIntoWorld(CEntity *pOwner);
	void CloneOther(CGoodieBag *pGoodieBag);
	f32 GetProb() { return(m_fProb); }
	void SetProb(f32 fProb);

	void SetGoodieScale( f32 fScale );
	f32 GetGoodieScale( void ) const	{ return m_fGoodieScale; }
	
	BOOL AddGoodie(GoodieType_e eGoodieType, u32 uQuantity);
	BOOL AddGoodie(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2, f32 fProb, GoodieBagAngVelType_e eAngVelType = GOODIEBAGANGVELTYPE_NONE);
	BOOL AddGoodie(GoodieType_e eGoodieType, u32 uQuantity, f32 fProb);

	GoodieBagBrain_e m_eBrain;
//	GoodieBagAngVelType_e m_eAngVelType;

	CGoodieInst m_aGoodies[GoodieBag_uMaxNumGoodieInsts];

private:

	// This needs to be called when an item is picked up.
	static void PickupCallback(CEntity *pPickedUp, CGoodieBag *pGoodieBag, GoodieType_e eGoodieType);

	u32 m_uNumGoodies;
	f32 m_fProb;
	f32 m_fGoodieScale;

	FCLASS_STACKMEM_NOALIGN(CGoodieBag);
} FCLASS_NOALIGN_SUFFIX;

#endif
