//////////////////////////////////////////////////////////////////////////////////////
// weapon_mortar.h - Small Projectile Emitter Weapon.
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
// 08/14/02 Ranck		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_MORTAR_H_
#define _WEAPON_MORTAR_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fforce.h"
#include "tracer.h"


class CMeshEntity;
class CFAnimCombiner;
class CFAnimMeshRest;
class CFAnimCombinerConfig;
class CFAnimSource;



FCLASS_ALIGN_PREFIX class CWeaponMortar : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_FORK,
		_MESH_SLING,

		_MESH_COUNT
	} _Mesh_e;


	typedef enum {
		_SLING_ANIM_TAKE_UP_SLACK,
		_SLING_ANIM_AIM,
		_SLING_ANIM_FIRE,

		_SLING_ANIM_COUNT
	} _SlingAnim_e;


	typedef enum {
		_ANIM_TAP_REST,
		_ANIM_TAP_RELOAD,
		_ANIM_TAP_AIMFIRE,

		_ANIM_TAP_COUNT
	} _AnimTap_e;


	typedef enum {
		_ANIM_CONTROL_RELOAD,
		_ANIM_CONTROL_AIMFIRE,

		_ANIM_CONTROL_COUNT
	} _AnimControl_e;



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		_MAX_SAMPLE_COUNT = 16
	};


	typedef enum {
		SLING_MODE_IDLE,								// Sling is paused at the beginning of the Taking-Up-Slack anim
		SLING_MODE_TAKING_UP_SLACK,						// Sling is playing the Taking-Up-Slack anim (will enter AIMING when done)
		SLING_MODE_AIMING,								// Sling's aim anim is being driven by m_fUnitStretch
		SLING_MODE_FIRING,								// Sling's firing anim is playing (will enter IDLE when done)

		SLING_MODE_COUNT
	} _SlingMode_e;


	typedef struct {			// NOTE: If you change this structure, you must also change _Mesh_e, _SlingAnim_e and m_aUserPropVocab
		cchar *apszMeshName[_MESH_COUNT];				// Mesh names
		cchar *apszAnimName[_SLING_ANIM_COUNT];			// Anim names

		f32 fWeaponCullDist;							// Don't draw the weapon if it's this far from the camera
		f32 fDistFromWeaponOrigToMuzzle;				// Distance from the weapon's origin to where the projectiles come out

		// Sound Effects:
		CFSoundGroup *pSoundGroupFire;					// Fire sound
		CFSoundGroup *pSoundGroupPullBack;				// Pulling back sound
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pForkWorldMesh;					// This EUK's world mesh for the fork
		CFAnimCombiner *m_pForkAnimCombiner;			// Our animation combiner for the fork mesh
		CFAnimMeshRest *m_pForkAnimMeshRest;			// The rest anim for our fork
		s32 m_anForkTapID[_ANIM_TAP_COUNT];				// Tap IDs for the fork anim combiner
		s32 m_anForkControlID[_ANIM_CONTROL_COUNT];		// Control IDs for the fork anim combiner

		CMeshEntity *m_pSlingMeshEntity;				// The sling's mesh entity
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_MORTAR];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static FTexDef_t *m_apTexDef[EUK_COUNT_MORTAR];

	static CFAnimCombinerConfig *m_pAnimCombinerConfig;

	_ResourceData_t m_aResourceData[EUK_COUNT_MORTAR];
	_ResourceData_t *m_pResourceData;

	BOOL m_bAbortForkAnims;							// TRUE = Blend out fork anim controls
	_SlingMode_e m_nSlingMode;						// The current sling mode
	f32 m_fUnitStretch;								// How much the bot is stretching back the bungie
	f32 m_fUnitLaunchSpeed;							// When launched, this indicates the projectile's unit speed (1=fastest)

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.

	CFAudioEmitter *m_pSoundEmitter;

	CFVec3A m_UnitFireDir_WS;

	u32 m_nNextFreeTriggerSampleIndex;				// Next free index into m_afTriggerSample[]
	u32 m_nTriggerSampleCount;						// Number of valid entries in m_afTriggerSample[]
	f32 m_afTriggerValue[_MAX_SAMPLE_COUNT];		// Trigger value samples
	f32 m_afTriggerDeltaTime[_MAX_SAMPLE_COUNT];	// Trigger delta time samples

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponMortar();
	virtual ~CWeaponMortar();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	// Targeting
	virtual f32 GetProjectileSpeed(void) const;


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS = NULL );


	// Rumble:
	virtual void KillRumble( void );


	// Item inst:
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );


	// Mortar:
	BOOL Mortar_IsReadyToFire( void );
	FINLINE f32 Mortar_GetUnitStretch( void ) { FASSERT( IsCreated() ); return m_fUnitStretch; }
	FINLINE f32 Mortar_GetUnitLaunchSpeed( void ) { FASSERT( IsCreated() ); return m_fUnitLaunchSpeed; }

	void Mortar_Fork_Idle( void );
	void Mortar_Fork_Abort( void );
	void Mortar_Fork_AttachReloadAnim( CFAnimSource *pAnimSource );
	void Mortar_Fork_AttachAimFireAnim( CFAnimSource *pAnimSource );
	void Mortar_Fork_SetReloadAnimControl( f32 fUnitControl );
	void Mortar_Fork_SetAimFireAnimControl( f32 fUnitControl );

	void Mortar_Sling_Idle( void );
	void Mortar_Sling_TakeUpSlack( void );
	void Mortar_Sling_Release( BOOL bPlayFireSound );




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
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	virtual void ClassHierarchyResetToState( void );

	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );
	virtual void NotifyAmmoMightHaveChanged( void );

	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	static void _TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );

	FCLASS_STACKMEM_ALIGN( CWeaponMortar );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponMortarBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponMortarBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponMortarBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponMortarBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
