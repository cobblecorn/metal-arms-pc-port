//////////////////////////////////////////////////////////////////////////////////////
// weapon_rivet.h - Mining rivet implementation.
//
// Author: Steve Ranck     
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
// 03/28/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_RIVET_H_
#define _WEAPON_RIVET_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fworld.h"
#include "tracer.h"
#include "fxfm.h"
#include "fviewport.h"
#include "fforce.h"
#include "fmath.h"
#include "fmesh.h"
#include "eproj.h"
#include "explosion.h"


class CBot;
class CDamageProfile;
class CFSoundGroup;
class CEProj;




FCLASS_ALIGN_PREFIX class CWeaponRivet : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_WEAPON,						// Weapon mesh
		_MESH_CLIP,							// Clip mesh
		_MESH_ARROW,						// Arrow projectile mesh

		_MESH_COUNT
	} _Mesh_e;


	typedef enum {
		_BONE_PRIMARY_FIRE,					// Where the hurt comes out of
		_BONE_CLIP_ATTACH,					// Where the clip attaches to

		_BONE_COUNT
	} _Bone_e;


	enum {
		MAX_TRACER_COUNT = 10,
		DAMAGE_FORM_COUNT = 3
	};


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef enum {
		RIVET_FLAG_NONE					= 0x00
	};


	typedef struct {
		cchar *apszMeshName[_MESH_COUNT];		// Mesh names
		cchar *apszBoneName[_BONE_COUNT];		// Bone names
		cchar *pszTracerTexName;				// Tracer texture name

		f32 fClipAmmoMax;						// Maximum number of clip rounds (<0 means infinite)
		f32 fReserveAmmoMax;					// Maximum number of reserve rounds (<0 means infinite)
		f32 fRoundsPerSec;						// Maximum fire rate

		f32 fWeaponCullDist;					// Don't draw the weapon if it's this far from the camera
		f32 fClipCullDist;						// Don't draw the clip if it's this far from the camera

		f32 fShotSpreadFactor;					// Shot spread factor
		f32 afScopeSpreadMult[EUK_COUNT_SCOPE];	// Spread multiplier when this weapon is used with the scope

		u32 nProjInPoolCount;					// The number of projectiles in the projectile pool
		f32 fMaxLiveRange;						// The maximum distance the projectile can travel before self-destructing
		f32 fUnitRecoil;						// amount of recoil this weapon generates

		f32 fProjCullDist;						// Don't draw the projectile if it's this far from the camera
		f32 fProjSpeed;							// Projectile's linear speed
		f32 fMinTargetAssistDist;				// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;				// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		FParticle_DefHandle_t hParticleImpact;	// Impact particle (FPARTICLE_INVALID_HANDLE if none)
		CDamageProfile *pDamageProfileImpact;	// Impact damage profile

		FMesh_t *pMeshEjectClip;				// Eject clip mesh

		// Sound Effects:
		CFSoundGroup *pSoundGroupFire;			// Fire sound
		CFSoundGroup *pSoundGroupEmpty;			// Empty sound
		CFSoundGroup *pSoundGroupEjectClip;		// Eject clip sound
		CFSoundGroup *pSoundGroupAttachClip;	// Attach clip sound
		CFSoundGroup *pSoundGroupSlapInClip;	// Slap-in clip sound
		CFSoundGroup *pSoundGroupRicochet;		// Arrow ricochet sound

		CEProjExt::CEProj_Arrow_StaticParams_t ArrowProjStaticParams;
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;				// This EUK's world mesh
		CFAnimCombiner *m_pAnimCombiner;		// This EUK's animation combiner
		CFAnimMeshRest *m_pAnimRest;			// Rest Animation
		const CFMtx43A *m_pFireMtx_WS;			// The fire bone matrix
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;

	static _UserProps_t m_aUserProps[EUK_COUNT_RIVET];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];
	static CWeaponRivet *m_pCallbackThis;

	static CDamageForm *m_apDamageFormArray[DAMAGE_FORM_COUNT];
	static u32 m_nDamageFormCount;
	static CEntity *m_pHitEntity;
	static CFCollInfo m_CollInfoArrowRayTest;
	static CFTrackerCollideRayInfo m_TrackerCollideRayInfo;
	static const CFVec3A *m_pAttackUnitDir_WS;

	static CEProjPool::PoolHandle_t m_hArrowProjPool;

	static SmokeTrailAttrib_t m_SmokeTrailAttrib;

	static TracerGroupHandle_t m_hTracerGroup;		// Tracer group
	static CFTexInst m_TracerTexInst;				// Tracer texture
	static TracerDef_t m_TracerDef;					// Tracer definition

	_ResourceData_t m_aResourceData[EUK_COUNT_RIVET];
	_ResourceData_t *m_pResourceData;
	_UserProps_t *m_pUserProps;

	CMeshEntity *m_pClipMeshEntity;					// The clip mesh entity (never NULL)

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to
	f32 m_fSecondsCountdownTimer;					// Times the time between shots

	u16 m_nNextTracerID;							// The next tracer ID to store in the tracer's nUser field

	u8 m_nRivetFlags;								// See RIVET_FLAG_* for info
	BOOL8 m_bFireThisFrame;							// TRUE if we should fire a round the next time our work function is called
	CFVec3A m_TargetPos_WS;							// Target position
	const CFVec3A *m_pBuddyFirePos_WS;				// Position the projectile is to emit from the fire buddy (NULL=none)

	u16 m_nChargingTracerID;						// ID of the tracer being charged (0=none)

	CEProj *m_pChargingArrowProj1;					// Points to the arrow projectile being charged (NULL=none)
	CEProj *m_pChargingArrowProj2;					// Points to the arrow projectile being charged (from buddy) (NULL=none)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );


	// Creation:
	CWeaponRivet();
	virtual ~CWeaponRivet();
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS = NULL );
	BOOL IsArrowSuspendedByTriggerDown( CEProj *pProj );


	// Rumble:
	virtual void KillRumble( void );


	// Checkpoint save/restore
	virtual void CheckpointSaveSelect( s32 nCheckpoint );


	// Misc:
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );
	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); if( m_pResourceData ) { return m_pResourceData->m_pWorldMesh; } else { return NULL; } }




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );
	virtual void ClassHierarchyResetToState( void );

	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );

	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );

	virtual void BeginReload( void );
	virtual void NotifyAmmoMightHaveChanged( void );

	virtual void Clip_AttachToOwnerBotBone( cchar *pszBoneName );
	virtual void Clip_AttachToWeapon( void );
	virtual void Clip_DiscardAttachedToOwnerBotBone( void );
	virtual void Clip_Eject( void );
	virtual void Clip_SlapIn( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _SetSmokeTrailAttributes( void );	
	static void _BuildTracerSkipListCallback( void *pUser );
	static void _TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pCollImpact );

	void _ClearDataMembers( void );
	void _AddClipToWorld( void );
	void _RemoveClipFromWorld( void );
	void _FireRound( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fJitterFactor, BOOL bBuddy );
	void _StickArrowProjectile( TracerDef_t *pTracerDef, const FCollImpact_t *pCollImpact );
	void _InflictDamageToEntityArrowStruck( TracerDef_t *pTracerDef, const FCollImpact_t *pCollImpact );

	void _DamageNearbyEntities( const CFMtx43A *pArrowUnitMtx_WS, f32 fArrowRadius, const CDamageForm::Damager_t *pDamagerData );
	static BOOL _FindTrackersIntersectingArrowCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );

	void _DetonateChargingArrow( void );

	static BOOL _ArrowDetonatedCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact );


	FCLASS_STACKMEM_ALIGN( CWeaponRivet );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRivetBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponRivetBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponRivetBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponRivetBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif

