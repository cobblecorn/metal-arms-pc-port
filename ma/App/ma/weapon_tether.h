//////////////////////////////////////////////////////////////////////////////////////
// weapon_tether.h - Tether weapon.
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
// 06/05/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_TETHER_H_
#define _WEAPON_TETHER_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fworld.h"
#include "fmesh.h"
#include "fanim.h"
#include "eproj.h"
#include "fforce.h"
#include "tether.h"



class CBot;
class CMeshEntity;
class CReticle;
class CFCamera;
class CFSoundGroup;




FCLASS_ALIGN_PREFIX class CWeaponTether : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_WEAPON,
		_MESH_BOLT,

		_MESH_COUNT
	} _Mesh_e;


	typedef enum {
		_BONE_BARREL,
		_BONE_PRIMARY_FIRE,

		_BONE_COUNT
	} _Bone_e;

	enum {
		RL3_CHAMBER_COUNT = 3,			// Number of tethers RL3 can fire simultaneously

		KRUNKS_EUK_INDEX = 3,			// The EUK index of Krunks tether weapon
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF


	typedef enum {
		DATAPORTTEST_TARGETABLE,			// The data port is targetable
		DATAPORTTEST_HOLDABLE,				// The data port is not targetable, but we can wait a bit until it is
		DATAPORTTEST_CANCEL,				// The data port is not targetable and we can't wait for it to be

		DATAPORTTEST_COUNT
	} DataPortTest_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:


	typedef struct {						// NOTE: If you change this structure, you must also change m_aUserPropVocab
		cchar *apszMeshName[_MESH_COUNT];	// Mesh names
		cchar *apszBoneName[_BONE_COUNT];	// Bone names
		cchar *pszPumpAnimName;				// Pump animation name

		f32 fReserveAmmoMax;				// Maximum reserve ammo
		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fMaxLiveRange;					// The maximum distance the tether can travel before self-destructing
		f32 fBoltCullDist;					// Don't draw the bolt if it's this far from the camera
		f32 fBoltSpeed;						// Bolt's linear speed
		f32 fMuzzleRecoilDistance;			// The distance to recoil the muzzle
		f32 fBoltRestOffsetZ;				// The offset along the barrel's bone Z axis of where the bolt rests
		f32 fBoltScale;						// The bolt's scale

		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		f32 fTetherLength;					// The length of the tether cable
		f32 fStretchLength;					// The length the tether cable can stretch before snapping
		f32 fMaxFlyDist;					// The maximum distance the tether can fly before auto-destructing

		f32 fPossessSpeed;					// Possession speed multiplier
		f32 fLockOnSpeed;					// 1/seconds to lock onto a target
		u32 nPumpCount;						// Number of times the weapon must be pumped up in order to be reloaded

		CFSoundGroup *pSoundGroupFire;			// Fire sound
		CFSoundGroup *pSoundGroupEmpty;			// Empty sound
		CFSoundGroup *pSoundGroupPump;			// Pump sound
		CFSoundGroup *pSoundGroupAttachBolt;	// Attach bolt sound

		CTether::UserProps_t TetherUserProps;	// CTether's user props
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;			// This EUK's world mesh
		CFAnimCombiner *m_pAnimCombiner;	// This EUK's animation combiner
		CFAnimInst *m_pPumpAnimInst;		// Pump animation instance
		s32 m_nPrimaryFireBoneIndex;		// Bone index of the Primary Fire bone (Never -1)
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_TETHER];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static CWeaponTether *m_pCallbackTether;

	_ResourceData_t m_aResourceData[EUK_COUNT_TETHER];
	_ResourceData_t *m_pResourceData;
	_UserProps_t *m_pUserProps;

	CMeshEntity *m_pBoltMeshEntity;					// The bolt mesh entity (never NULL)

	BOOL m_bFireThisFrame;							// TRUE to fire the weapon on the next work function

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
	u32 m_nPumpsNeededCount;						// The number of pumps needed to reload
	f32 m_fPumpAnimTimeScale;						// Time scale multiplier used on the play animation (0 = not playing animation)
	f32 m_fUnitMuzzleRecoilDist;					// Unit amount of muzzle recoil distance

	CTether *m_pTether;								// The tether wire
	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponTether();
	virtual ~CWeaponTether();


	// Creation/Destruction:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );
	void DestroyTether( BOOL bSupressEffects=FALSE );


	// Info:
	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); if( m_pResourceData ) { return m_pResourceData->m_pWorldMesh; } else { return NULL; } }


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	// Targeting:
	virtual void HumanTargeting_FindDesiredTargetPoint_WS( CFVec3A *pTargetPoint_WS );


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS = NULL );

	virtual void Trigger_AttachRoundToOwnerBotBone( cchar *pszBoneName );
	virtual void Trigger_AttachRoundToWeapon( void );
	virtual void Trigger_RemoveRoundFromWorld( void );

	BOOL Pump( f32 fTimeScale );

	FINLINE BOOL IsTetherDestroyed( void ) const { FASSERT( IsCreated() ); return m_pTether->IsDestroyed(); }
	FINLINE BOOL IsTetherAttachedToTarget( void ) const { FASSERT( IsCreated() ); return m_pTether->IsAttachedToTarget(); }
	FINLINE BOOL IsTetherFiringZapPulse( void ) const { FASSERT( IsCreated() ); return m_pTether->IsFiringZapPulse(); }
	FINLINE BOOL IsTetherZapping( void ) const { FASSERT( IsCreated() ); return m_pTether->IsZapping(); }
	FINLINE BOOL IsTetherDoneZapping( void ) const { FASSERT( IsCreated() ); return m_pTether->IsDoneZapping(); }
	FINLINE BOOL IsTetherDetachedFromSource( void ) const { FASSERT( IsCreated() ); return m_pTether->IsDetachedFromSource(); }

	FINLINE u32 GetNumTetherPumpsNeeded( void ) const { FASSERT( IsCreated() ); return m_nPumpsNeededCount; }

	// Rumble:
	virtual void KillRumble( void );


	// Item inst:
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );




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
	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );
	virtual void ClassHierarchyResetToState( void );

	virtual void BeginReload( void );

	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );
	virtual void NotifyAmmoMightHaveChanged( void );
	virtual void ReticleTypeSet( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static BOOL _FindTrackersInRangeCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	void _FindUnlockedTargetPoint( CFCamera *pCamera, CFVec3A *pTargetPoint_WS );
	void _LaunchTetherCable( CBot *pTargetBot, const CFVec3A *pTargetPos_WS );

	void _ClearDataMembers( void );
	DataPortTest_e _TestDataPortTargetable( CBot *pBot, const FViewport_t *pViewport, CReticle *pReticle, f32 *pfDistFromMuzzleToDataPort=NULL );

	void _FireTether( void );
	void _SetReticleLockOnSpeed( void );
	void _RemoveBoltFromWorld( void );
	void _AddBoltToWorld( void );


	FCLASS_STACKMEM_ALIGN( CWeaponTether );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponTetherBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponTetherBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponTetherBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponTetherBuilder );
} FCLASS_ALIGN_SUFFIX;








#endif

