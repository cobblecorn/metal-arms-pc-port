//////////////////////////////////////////////////////////////////////////////////////
// Damage.h - Damage module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/06/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DAMAGE_H_
#define _DAMAGE_H_ 1

#include "fang.h"
#include "fmath.h"
#include "flinklist.h"
#include "fgamedata.h"
#include "gcoll.h"


#define _DAMAGE_ENTITY_POOL_COUNT 80

#define DAMAGE_CAUSE_STRING_IMPACT			"Impact"	// Maps to DAMAGE_CAUSE_IMPACT
#define DAMAGE_CAUSE_STRING_BLAST			"Blast"		// Maps to DAMAGE_CAUSE_BLAST
#define DAMAGE_CAUSE_STRING_HEAT			"Heat"		// Maps to DAMAGE_CAUSE_HEAT
#define DAMAGE_CAUSE_STRING_SEVER			"Sever"		// Maps to DAMAGE_CAUSE_SEVER
#define DAMAGE_CAUSE_STRING_COLLISION		"Collision"	// Maps to DAMAGE_CAUSE_COLLISION
#define DAMAGE_CAUSE_STRING_CRUSH			"Crush"		// Maps to DAMAGE_CAUSE_CRUSH
#define DAMAGE_CAUSE_STRING_AMBIENT			"Ambient"	// Maps to DAMAGE_CAUSE_AMBIENT


typedef enum {
	DAMAGE_CAUSE_IMPACT,				// Damage was caused by a projectile impact
	DAMAGE_CAUSE_BLAST,					// Damage was caused by a blast
	DAMAGE_CAUSE_HEAT,					// Damage was caused by directed heat
	DAMAGE_CAUSE_SEVER,					// Damage was caused by a severing weapon (like a spinning saw blade)
	DAMAGE_CAUSE_COLLISION,				// Damage was caused by a collision (falling from a great height, hitting a wall at high speed, etc.)
	DAMAGE_CAUSE_CRUSH,					// Damage was caused by an omni-present crushing force
	DAMAGE_CAUSE_AMBIENT,				// Damage was caused by some omni-present ambient condition (acid, lava, punishment, etc.)

	DAMAGE_CAUSE_COUNT
} DamageCause_e;


typedef enum {
	DAMAGE_HITPOINT_TYPE_PROJECTILE,	// Hitpoint type is "projectile"
	DAMAGE_HITPOINT_TYPE_BLAST,			// Hitpoint type is "blast"
	DAMAGE_HITPOINT_TYPE_FLAME,			// Hitpoint type is "flame"

	DAMAGE_HITPOINT_TYPE_COUNT
} DamageHitpointType_e;


typedef enum {
	DAMAGE_RETURN_NORMAL,				// No special instructions for 
	DAMAGE_RETURN_REFLECT,

	DAMAGE_RETURN_COUNT
} DamageReturn_e;

class CEntity;
class CBot;
class CWeapon;
class CFWorldMesh;
class CDamage;
class CDamageData;
class CArmorProfile;
class CFWorldTracker;
class CFWorldMesh;
struct FVisVolume_t;
struct FCollImpact_t;

typedef struct {
	CEntity *pEntity;					// The last entity damaged.
	DamageHitpointType_e eDamageType;	// The type of hit point damage.
	CWeapon *pWeapon;					// The weapon (if any) that caused the damage.
	f32 fDeltaHitPoints;				// The amount of damage inflicted.
	u32 nTimeStamp;						// What frame was this damage dealt? Current uses FVid_nFrameCounter.
} LastEntityDamage_t;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageRange - Utility class for ranged, interpolated data.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CDamageRange {
public:

	f32 m_fInnerRadius;							// Ranges smaller than this value will use m_fMinValue
	f32 m_fInnerValue;							// The value to use when the range is <= m_fMinRadius

	f32 m_fOuterRadius;							// Ranges greater than this value will use 0
	f32 m_fOuterValue;							// The value to use when the range is == m_fMaxRadius


	void GenerateFromLerp( f32 fUnitVal, const CDamageRange *pDamageRange0, const CDamageRange *pDamageRange1 );


	FCLASS_STACKMEM_NOALIGN( CDamageRange );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageProfileDef - CSV data used to build a CDamageProfile.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CDamageProfileDef {
public:

	union {
		cchar *m_pszDamageCause;				// CSV: Maps to DamageCause_e (string need not persist)
		DamageCause_e m_nDamageCause;			// CDamageProfile: DamageCause_e
	};

	f32 m_fDamagePeriod;						// How long between damage inflictions (0=every frame; not recommended)

	f32 m_afUnitHitpoints[DAMAGE_HITPOINT_TYPE_COUNT];	// 0=No hitpoints applied, 1=all hitpoints applied

	CDamageRange m_HitpointRange;				// Defines the spherical range for hitpoints
	CDamageRange m_ImpulseRange;				// Defines the spherical range for impulse
	CDamageRange m_RumbleRange;					// Defines the spherical range for rumble

	f32 m_fRumbleSecs;							// Duration of the rumble in seconds
	f32 m_fAIAudibleRadius;						// The audible range the AI can hear the sound from


	void GenerateFromLerp( f32 fUnitVal, const CDamageProfileDef *pDamageProfileDef0, const CDamageProfileDef *pDamageProfileDef1 );


	FCLASS_STACKMEM_NOALIGN( CDamageProfileDef );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CArmorProfileDef - CSV data used to build a CArmorProfile.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CArmorProfileDef {
public:

	f32 m_afUnitProtection[DAMAGE_HITPOINT_TYPE_COUNT];		// 0=no protection, 1=invincible

	f32 m_fUnitProtectionFromImpulse;						// 0=receive full force from impulse, 1=receive no force from impulse
	f32 m_fUnitProtectionFromRumble;						// 0=receive full rumble, 1=receive no rumble

	f32 m_fMinBlastHitpointFilter;							// Blast hitpoints must be greater than or equal to this value, or they will be clamped to 0


	FCLASS_STACKMEM_NOALIGN( CArmorProfileDef );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageProfile - Damage profile data.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CDamageProfile : public CDamageProfileDef {
public:

	cchar *m_pszName;							// Profile name

	f32 m_fOODeltaRadius_Hitpoint;				// 1/(m_HitpointRange.m_fOuterRadius - m_HitpointRange.m_fInnerRadius), 0.0=No interpolation region
	f32 m_fOODeltaRadius_Impulse;				// 1/(m_ImpulseRange.m_fOuterRadius - m_ImpulseRange.m_fInnerRadius), 0.0=No interpolation region
	f32 m_fOODeltaRadius_Rumble;				// 1/(m_RumbleRange.m_fOuterRadius - m_RumbleRange.m_fInnerRadius), 0.0=No interpolation region

	BOOL8 m_abZeroInnerAndOuterValues[3];		// [0]=hitpoint, [1]=impulse, [2]=rumble
												// TRUE if m_HitpointRange.m_fInnerValue and m_HitpointRange.m_fOuterValue are both zero


	void ComputeValuesFromProfileDef( void );
	void GenerateFromLerp( f32 fUnitVal, const CDamageProfile *pDamageProfile0, const CDamageProfile *pDamageProfile1 );


	FCLASS_STACKMEM_NOALIGN( CDamageProfile );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CArmorProfile - Armor profile data.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CArmorProfile : public CArmorProfileDef {
public:

	cchar *m_pszName;							// Profile name


	FCLASS_STACKMEM_NOALIGN( CArmorProfile );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageResult
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CDamageResult {
public:

	// Info:
	const CDamageData *m_pDamageData;					// The damage data associated with these results
	const CArmorProfile *m_pArmorProfile;				// Armor used to generate these results

	// Hitpoints:
	f32 m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_COUNT];	// Resultant delta hitpoint breakdown
	f32 m_fDeltaHitpoints;								// Sum of all delta hitpoints in m_afDeltaHitpoints[]

	// Impulse:
	f32 m_fImpulseMag;									// Resultant impulse intensity
	CFVec3A m_Impulse_WS;								// Resultant impulse vector (m_AttackUnitDir_WS * m_fImpulseMag, or Zero if DAMAGE_LOCALE_AMBIENT)

	// Rumble:
	f32 m_fRumbleUnitIntensity;							// Resultant unit rumble intensity


	FCLASS_STACKMEM_ALIGN( CDamageResult );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageForm - Given to the damage system by a module that wishes to inflict damage upon other entities.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CDamageForm {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Damage callback function. Return FALSE to abort Damage, or potentially modify the damage result.
	typedef BOOL DamageInflictingCallback_t( CDamageForm *pDamageForm, CEntity *pDamageeEntity, void *pUserArgument );


	typedef enum {
		DAMAGE_LOCALE_AMBIENT,	// Damage is inflicted internally, with no directional nor triangle information (e.g. pit of lava)
		DAMAGE_LOCALE_BLAST,	// Damage is inflicted directionally from a distant blast, with no triangle information (requires m_Epicenter_WS)
		DAMAGE_LOCALE_IMPACT,	// Damage is inflicted upon a specific triangle (e.g. projectile impact) (requires m_Epicenter_WS, m_AttackUnitDir_WS and m_TriData) (delivery must be DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY)

		DAMAGE_LOCALE_COUNT
	} DamageLocale_e;


	typedef enum {
		DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY,					// Deliver damage to m_pDamageeEntity
		DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS,		// Deliver damage to all entities within the radius specified by the profile (requires m_Epicenter_WS)
		DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_VOLUME,	// Deliver damage to all entities within m_pVisVolume
		DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE,	// Deliver damage to all entities within m_pTripwireEntity

		DAMAGE_DELIVERY_COUNT
	} DamageDelivery_e;


	typedef struct {
		CEntity *pEntity;					// The entity that's actually causing the damage (e.g. CEProj) (NULL for none or unknown)
		CWeapon *pWeapon;					// The weapon that spawned m_pDamagerEntity (NULL for none of unknown)
		CBot *pBot;							// The bot that's responsible for unleashing m_pDamagerEntity upon its victims (NULL for none or unknown)
		s8 nDamagerPlayerIndex;				// The player index that's responsible for dealing the damage (-1 = not a player, or unknown)
	} Damager_t;


	typedef struct {
		CFVec3A ImpactUnitNormal_WS;		// Unit normal of the triangle that was hit (when no tri data, this is simply -m_AttackUnitDir_WS)
		CFVec3A aTriVtx_WS[3];				// The 3 vertices in world space of the triangle that was hit
		s32 nDamageeBoneIndex;				// Which bone in a segmented mesh this impact is describing
		CFWorldMesh *pDamageeWorldMesh;		// Points to the world mesh that was hit (never NULL when m_nInfoLevel==DAMAGE_INFO_LEVEL_TRIANGLE)
		GCollSurfType_e eSurfaceType;		// The surface type of the impacted triangle
	} TriData_t;


	// Valid damage configurations:
	//
	//  DAMAGE_LOCALE_AMBIENT supports all DamageDelivery_e types.
	//  DAMAGE_LOCALE_BLAST supports all DamageDelivery_e types.
	//  DAMAGE_LOCALE_IMPACT supports only DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY.
	//
	//  DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY supports all DamageLocale_e types.
	//  DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS supports only DAMAGE_LOCALE_AMBIENT and DAMAGE_LOCALE_BLAST.
	//  DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_VOLUME supports only DAMAGE_LOCALE_AMBIENT and DAMAGE_LOCALE_BLAST.
	//  DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE supports only DAMAGE_LOCALE_AMBIENT and DAMAGE_LOCALE_BLAST.
	//
	//
	// Notes:
	//
	//  DAMAGE_LOCALE_AMBIENT and DAMAGE_LOCALE_IMPACT use the inner-radius values from the profile.
	//  DAMAGE_LOCALE_BLAST interpolates values between the inner and outer radius from the profile.
	//
	//  DAMAGE_LOCALE_BLAST performs a LOS test for hitpoints and impulse, but not for rumble.
	//  DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS uses the largest of the profile's radii: hitpoint, impulse, and rumble.
	//
	//  Hitpoints are used to inflict health gain or loss.
	//  Impulse is used to generate a physical impact reaction in the damagee.
	//  Rumble is used to generate camera shake, spawn debris, fumble footing, etc.
	//
	//  Hitpoints are normalized. There are 1.0 hitpoints in each battery cell. Hitpoints can range from -infinity to +infinity.
	//  Impulse is the magnitude of the impulse force. The larger the value, the greater the impulse force
	//  Rumble is a unit value. A value of 1.0 means a large rumble, like being close to a huge explosion.
	//




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		_BLAST_POINT_COUNT		= 5
	};


	typedef enum {
		LIST_MEMBER_NONE,						// This form is not a part of any list
		LIST_MEMBER_FREE_POOL,					// This form is currently in the free pool
		LIST_MEMBER_QUEUED_POOL,				// This form is currently in the queued pool
		LIST_MEMBER_INSTANT_POOL,				// This form is currently in the instant pool

		LIST_MEMBER_COUNT
	} ListMember_e;


	enum {
		DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL	= 0x04,	// FALSE=This form was constructed via fnew, TRUE=This form was obtained from the free pool

		DAMAGE_FLAG_NONE						= 0x00
	};



	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static Damager_t m_TempDamager;			// Used as a temporary place to construct damager data


	// Information:
	DamageLocale_e m_nDamageLocale;			// The damage locale (ambient, blast, or impact)
	DamageDelivery_e m_nDamageDelivery;		// The damage delivery type
	const CDamageProfile *m_pDamageProfile;	// Damage profile (must not be NULL)

	f32 m_fNormIntensity;					// Normalized intenstiy (multiplied by intensity data in the profile)
	CFVec3A m_Epicenter_WS;					// Damage epicenter (required for DAMAGE_LOCALE_BLAST and DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS)
											//   DAMAGE_LOCALE_BLAST: Epicenter is where the blast detonated
											//   DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS: Epicenter is the origin of the test sphere

	// Tri Data:
	TriData_t m_TriData;					// Triangle data (required by DAMAGE_LOCALE_IMPACT)
	CFVec3A m_ImpactPoint_WS;				// Point on the triangle of where the projectile hit (required by DAMAGE_LOCALE_IMPACT)
	CFVec3A m_AttackUnitDir_WS;				// Unit vector representing the direction from which the damage came (from damager to damagee) (required by DAMAGE_LOCALE_IMPACT)

	// Delivery:
	union {
		CEntity *m_pDamageeEntity;			// The entity that's receiving the damage (used by DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY) (If NULL, the damage form will be ignored)
		CEntity *m_pTripwireEntity;			// The tripwire defining the damage region (used by DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE)
		FVisVolume_t *m_pVisVolume;			// The FVis volume defining the damage region (used by DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_VOLUME)
	};

	// Deliverer:
	Damager_t m_Damager;					// Who's issuing the damage? See Damager_t for details.

	// Callback:
	DamageInflictingCallback_t *m_pFcnDamageBeingInflicted;	// Called before damage is applied
	void *m_pUserArgument;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	ListMember_e m_nListMember;				// Describes which list this form is currently a member of

	u32 m_nAISoundType;						// Built from profile data string. See AISOUNDTYPE_* in aienviro.h for details

	f32 m_fSecsUntilFormRetired;			// Number of seconds remaining until this form is to be retired (0=this frame only, <0=infinite, >0=duration in seconds)
	f32 m_fSecsUntilIssuingNextDamage;		// Number of seconds remaining until damage is issued again

	f32 m_fLargestOuterRadius;				// The largest value between the hitpoint, impulse, and rumble ranges

	FLink_t m_Link;							// Link to other CDamageForm objects in pool

	u8 m_nFlags;							// See DAMAGE_FLAG_* for info

	static CFWorldTracker **m_apEntityDamageQueue;		// Holds trackers that damage will be applied to
	static u32 m_uEntityDamageQueueCount;				// Number of trackers currently in m_apEntityDamageQueue

	static CEntity *m_pOneSpecificEntity;				// The specific entity we're looking for
	static CFVec3A m_aBlastPoint[_BLAST_POINT_COUNT];	// Used to determine whether a blast hit a tracker



//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CDamageForm() { m_nFlags = CDamageForm::DAMAGE_FLAG_NONE; }
	void Init( void );
	void InitTriDataFromCollImpact( CFWorldMesh *pImpactedWorldMesh, const FCollImpact_t *pImpact, const CFVec3A *pAttackUnitDir_WS );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL _PrepareForSubmission( f32 fDuration );

	BOOL _Work( void );
	void _Work_OneSpecificEntity( void );
	void _Work_ProfileRadius( void );
	void _Work_SpecificVolume( void );
	void _Work_SpecificTripwire( void );

	void _InflictDamage_Impact( CEntity *pEntity );
	void _InflictDamage_Ambient( CEntity *pEntity, const CFVec3A *pEpicenter_WS );
	void _InflictDamage_Blast( CFWorldTracker *pTracker );

	static FINLINE void _AddTrackerToDamageQueue( CFWorldTracker* pTracker );

	static BOOL _FindTrackersCallback_OneSpecificEntity_Blast( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	static BOOL _FindTrackersCallback_Blast( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _FindTrackersCallback_Impact( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _FindTrackersCallback_Ambient( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	static BOOL _DoesDamageDataHaveNonZeroIntensity( void );
	void _SetupDamagerInfo( void );

	f32 _ComputeRadialIntensity_Rumble( f32 fDist );
	f32 _ComputeRadialIntensity_Impulse( f32 fDist );
	f32 _ComputeRadialIntensity_Hitpoint( f32 fDist );


	friend class CDamage;


	FCLASS_STACKMEM_ALIGN( CDamageForm );
} FCLASS_ALIGN_SUFFIX;


FINLINE void CDamageForm::_AddTrackerToDamageQueue( CFWorldTracker* pTracker ) {
	if( m_uEntityDamageQueueCount >= _DAMAGE_ENTITY_POOL_COUNT ) {
		DEVPRINTF("CDamageForm::_AddTrackerToDamageQueue(): Cannot queue damage request, all entity pointers were used.\n");
		return;
	}

	m_apEntityDamageQueue[ m_uEntityDamageQueueCount ] = pTracker;
	m_uEntityDamageQueueCount++;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageData - Class given to a CEntity by the damage system to inflict damage onto the entity.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CDamageData {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// General info:
	CDamageForm::DamageLocale_e m_nDamageLocale;	// The damage locale (ambient, blast, or impact)
	CEntity *m_pDamageeEntity;						// The entity that's receiving the damage
	const CDamageProfile *m_pDamageProfile;			// The damage profile associated with this damage data

	const CFVec3A *m_pEpicenter_WS;					// Damage epicenter:
													// (valid only for DAMAGE_LOCALE_BLAST and DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS)
													// (NULL otherwise)
													//   DAMAGE_LOCALE_BLAST: Epicenter is where the blast detonated
													//   DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS: Epicenter is the origin of the test sphere

	CFVec3A m_ImpactPoint_WS;						// The point of impact (always valid):
													//   DAMAGE_LOCALE_IMPACT: Point on the triangle where the projectile hit
													//   DAMAGE_LOCALE_BLAST: Blast point somewhere within the bounding sphere
													//   DAMAGE_LOCALE_AMBIENT: m_MtxToWorld.m_vPos

	CFVec3A m_AttackUnitDir_WS;						// Unit vector representing the direction from which the damage came (from damager to damagee):
													// (valid only for DAMAGE_LOCALE_IMPACT and DAMAGE_LOCALE_BLAST)
													// ( <0,0,1> otherwise )
													//   DAMAGE_LOCALE_IMPACT: Attack dir is the direction of travel of the projectile
													//   DAMAGE_LOCALE_BLAST: Attack dir is the vector from m_pEpicenter_WS to the blast point

	const CDamageForm::TriData_t *m_pTriData;		// Triangle data (valid only for DAMAGE_LOCALE_IMPACT, NULL otherwise)


	// Damager info:
	CEntity *m_pDamagerTripwireEntity;				// The tripwire defining the damage region (NULL=tripwire did not cause the damage)
	FVisVolume_t *m_pDamagerVisVolume;				// The FVis volume defining the damage region (NULL=FVis volume did not cause the damage)
	CDamageForm::Damager_t m_Damager;				// See CDamageForm::Damager_t for details


	// AI data:
	f32 m_fAIAudibleRadius;							// The audible range the AI can hear the sound from


	// IMPORTANT: Typically, you don't want to use the following
	//            fields directly. Instead, compute a damage result
	//            via the ComputeDamageResult() function and use
	//            the values in the CDamageResult class for
	//            the hitpoints, etc.

	// Hitpoint data:
	f32 m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_COUNT];	// Delta hitpoints to be applied


	// Impulse data:									// (valid only for DAMAGE_LOCALE_IMPACT and DAMAGE_LOCALE_BLAST, 0 otherwise)
	f32 m_fImpulseMag;									// Magnitude of the impulse (0=no impulse) (always 0 if DAMAGE_LOCALE_AMBIENT)


	// Rumble data:
	f32 m_fRumbleUnitIntensity;							// Rumble unit intensity (0=no rumble, 1=max rumble)

	f32 m_fDamageFormOrigNormIntensity;					// Original m_fNormIntensity from the CDamageForm
	f32 m_fLargestOuterRadius;							// The largest value between the hitpoint, impulse, and rumble ranges




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	void ComputeDamageResult( const CArmorProfile *pArmorProfile, CDamageResult *pDamageResult, f32 fArmorModifier=0.0f, BOOL bDontApplyArmorModifierToHitpointsIfHitpointProtectionIsInfinite=FALSE, f32 fDamageNormIntensity=1.0f ) const;
	void InitCollImpactFromTriData( FCollImpact_t *pImpact ) const;

	friend class CDamageForm;


	FCLASS_STACKMEM_ALIGN( CDamageData );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamage
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CDamage {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static CDamageProfile m_DamageProfileNone;
	static CArmorProfile m_ArmorProfileNone;
	static CArmorProfile m_ArmorProfileInfinite;

	static LastEntityDamage_t m_LastEntityDamaged;


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;
	static BOOL m_bBusyProcessingInstantForms;
	static CDamageForm *m_pPoolFreeDamageForm;
	static FLinkRoot_t m_RootFreeDamageForm;
	static FLinkRoot_t m_RootQueuedDamageForm;
	static FLinkRoot_t m_RootInstantDamageForm;

	static CDamageProfile *m_pDamageProfileArray;
	static CArmorProfile *m_pArmorProfileArray;
	static u32 m_nDamageProfileCount;
	static u32 m_nArmorProfileCount;
	static BOOL m_bDamageDataSwapped;	// If TRUE, single and multi-player data have been swapped
	static const FGameData_TableEntry_t m_aVocab_DamageProfile[];
	static const FGameData_TableEntry_t m_aVocab_ArmorProfile[];




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitSystem( void );
	static  void InitLevel( void );
	static void UninitSystem( void );

	static void SwapMultiplayerData( BOOL bIsMultiPlayer );

	static u32 GetEmptyDamageFormCount( void ) { FASSERT( m_bSystemInitialized ); return m_RootFreeDamageForm.nCount; }

	static CDamageForm *GetEmptyDamageFormFromPool( void );
	static CDamageForm *BuildNewEmptyDamageForm( void );
	static CDamageForm *BuildNewEmptyDamageForm( u32 nArrayCount );

	static void SubmitDamageForm( CDamageForm *pDamageForm, f32 fDuration=0.0f );
	static void KillDamageForm( CDamageForm *pDamageForm );
	static BOOL CancelDamageForm( CDamageForm *pDamageForm );
	static BOOL ResubmitDamageForm( CDamageForm *pDamageForm, f32 fDuration=0.0f );

	static CDamageProfile *FindDamageProfile( cchar *pszProfileName, BOOL bReturnValidPointerIfNotFound=TRUE, BOOL bDisplayErrorIfNotFound=TRUE );
	static CArmorProfile *FindArmorProfile( cchar *pszProfileName, BOOL bReturnValidPointerIfNotFound=TRUE, BOOL bDisplayErrorIfNotFound=TRUE );

	static void Work( void );

	static f32 ComputeRadialIntensity_Rumble( f32 fDist, const CDamageProfile *pDamageProfile, f32 fNormIntensity=1.0f );
	static f32 ComputeRadialIntensity_Impulse( f32 fDist, const CDamageProfile *pDamageProfile, f32 fNormIntensity=1.0f );
	static f32 ComputeRadialIntensity_Hitpoint( f32 fDist, const CDamageProfile *pDamageProfile, f32 fNormIntensity=1.0f );
	static f32 ComputeRadialIntensity( f32 fDist, f32 fInnerValue, f32 fOuterValue, f32 fInnerRadius, f32 fOuterRadius, f32 fOODeltaRadius, BOOL bClampToUnitFloat, f32 fNormIntensity );
    



//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _InitPools( void );
	static BOOL _LoadDamageAndArmorData( void );
	static void *_GameDataDamageConverterCallback( cchar *pszDamageProfileName );
	static void *_GameDataArmorConverterCallback( cchar *pszArmorProfileName );


	FCLASS_STACKMEM_ALIGN( CDamage );
} FCLASS_ALIGN_SUFFIX;





#endif

