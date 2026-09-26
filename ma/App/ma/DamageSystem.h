//////////////////////////////////////////////////////////////////////////////////////
// DamageSystem.h - Damage system for Mettle Arms.
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
// 06/05/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DAMAGESYSTEM_H_
#define _DAMAGESYSTEM_H_ 1

#if 0

#include "fang.h"
#include "fmath.h"
#include "fworld.h"



class CEntity;
class CFWorldTracker;



// Defined as follows: Two damagers should have the same DamagerClass_e iff they should both
//   cause the same kinds of effects and responses (relative to each other) on everything they
//   hit.
enum DamagerClass_e
{
	DAMAGERCLASS_GLITCHHAND,
	DAMAGERCLASS_LASER,
	DAMAGERCLASS_RIVET,
	DAMAGERCLASS_GRENADE,
	DAMAGERCLASS_BLASTERPELLET,
	DAMAGERCLASS_ROCKET,
	DAMAGERCLASS_SPEWPELLET,
	DAMAGERCLASS_FLAME,
	DAMAGERCLASS_SAW,
	DAMAGERCLASS_EXPLOSION_GENERIC,

	DAMAGERCLASS_COUNT
};



// Defined as follows: Two damagees should have the same DamageeClass_e iff they should both
//   have the same effects and responses (relative to each other) to every kind of damager that
//   might hit them.
// (In other words, if a rock and a wall both do the same thing when hit by a laser, and both do
//   the same thing when hit by a grenade, etc, then they can/should share damagee types (but the
//   laser and the grenade should probably have different damager types)).
enum DamageeClass_e
{
	DAMAGEECLASS_DEFAULT,		// This can probably get used for most things, like bots, walls, etc.

	DAMAGEECLASS_COUNT
};



static const u32 _uHitEntityListSize = 10;



FCLASS_ALIGN_PREFIX class CDamageInfo
{
public:
	CEntity *m_pEDamager;			// The actual CEntity that is causing the damage (ie, the missile, rivet, whatever).  Can be NULL.
	CEntity *m_pEDamagerOwner;		// The owner of the damager that is causing the damage.
	DamagerClass_e m_eDamagerClass; // 
	// Note that, if we do decide to go with a CEDamager entity, then the above lines will probably be replaced by the below one,
	//   as they would probably all be in the CEDamager.
//	CEDamager *pEDamager;			// The actual CEDamager that is causing the damage (ie, the missile, rivet, whatever).  Can be NULL.

	CFVec3A m_vecNormal_WS;			// Normal to the plane of the surface on which the collision occured.
	CFVec3A m_vecPos_WS;			// Where the collision occured.
	CFVec3A m_vecDir_WS;			// Unit vector representing the direction from which the damage came (directed toward
									//   the damagee).

	f32 m_fUnitExplosionDamage;		// This value corresponds to the amount of explosion damage that has occurred.
	f32 m_fUnitFlameDamage;			// This value corresponds to the amount of flame damage that has occurred.
	f32 m_fUnitBulletDamage;		// This value corresponds to the amount of bullet damage that has occurred.

	CEntity *m_pEDamagee;			// The guy getting damaged.

	s32 m_nBoneIdx;					// Index of the bone that got hit.  Can be -1 in not applicable or unknown.

	FCLASS_STACKMEM_ALIGN(CDamageInfo);
} FCLASS_ALIGN_SUFFIX;



FCLASS_ALIGN_PREFIX class CDamageSystem
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	// Use this function when you (a CEntity) want to let another CEntity know that you are
	//   causing damage to it.
	// Will return TRUE if both the damagee and the damage system accept your request.
	static BOOL SubmitDamageRequest(CDamageInfo *pDamageInfo);

	// For these functions, you do *not* need to set up the m_pEDamagee field of pDamageInfo.  It will
	//   be taken care of by the system.
	static void SubmitDamageRequest(CDamageInfo *pDamageInfo, const CFVec3A &vecPos, f32 fRadius);
	// For this function, you also do *not* need to set the m_fUnitIntensity field of pDamageInfo.  It
	//   will be taken care of by the system.
	static void SubmitDamageRequest(CDamageInfo *pDamageInfo, const CFVec3A &vecPos, f32 fRadius1, f32 fRadius2, f32 fUnitIntensity1, f32 fUnitIntensity2);

private:
	CDamageSystem();		// You should never instantiate one of these.
	~CDamageSystem();

	static void _ProcessEntityList();

	static BOOL m_bSystemInitted;

	static CFVec3A s_vecBlastPos;
	static f32 s_fRadius1, s_fRadius2;
	static f32 s_fUnitBulletDamage1, s_fUnitBulletDamage2;
	static f32 s_fUnitExplosionDamage1, s_fUnitExplosionDamage2;
	static f32 s_fUnitFlameDamage1, s_fUnitFlameDamage2;
	static CDamageInfo *s_pCurDamageInfo;
//	static CFWorldTracker *s_apTrackerList[_uTrackerListSize];
	static CEntity *s_apHitEntities[_uHitEntityListSize];
	static u32 s_uEntityListCount;

	static void _DoEffects(CDamageInfo *pDamageInfo, DamageeClass_e eDamageeClass);
	static BOOL DamageSphereCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );

} FCLASS_ALIGN_SUFFIX;

#endif

#endif
