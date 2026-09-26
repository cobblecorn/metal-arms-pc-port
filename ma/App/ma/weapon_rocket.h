//////////////////////////////////////////////////////////////////////////////////////
// weapon_rocket.h - Rocket launcher weapon.
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

#ifndef _WEAPON_ROCKET_H_
#define _WEAPON_ROCKET_H_ 1

#include "fang.h"
#include "weapon.h"
#include "eproj.h"
#include "fforce.h"
#include "smoketrail.h"
#include "explosion.h"

struct FAnim_t;
class CBot;
class CDamageProfile;
class CFAnimCombiner;
class CFAnimInst;
class CFSoundGroup;



FCLASS_ALIGN_PREFIX class CWeaponRocket : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_WEAPON,
		_MESH_ROCKET,

		_MESH_COUNT
	} _Mesh_e;


	typedef enum {
		_ANIM_RELOAD_OPEN,
		_ANIM_RELOAD_CLOSE,
		_ANIM_DEPLOY,
		_ANIM_STOW,

		_ANIM_COUNT
	} _Anim_e;





//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef struct {						// NOTE: If you change this structure, you must also change _Anim_e and m_aUserPropVocab
		cchar *apszMeshName[_MESH_COUNT];	// Mesh names
		cchar *apszAnimName[_ANIM_COUNT];	// Animation names
		f32 afTimeMul[_ANIM_COUNT];			// The animation time multiplier

		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fReserveAmmoMax;				// Maximum number of reserve rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec

		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fShotSpreadFactor;				// Shot spread factor
		f32 afScopeSpreadMult[EUK_COUNT_SCOPE];	// Spread multiplier when this weapon is used with the scope

		f32 fMaxLiveRange;					// The maximum distance the rocket can travel before self-destructing
		f32 fRocketCullDist;				// Don't draw the rocket if it's this far from the camera
		f32 fRocketSpeed;					// Rocket's linear speed
		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		cchar *pszTubeBoneName;				// The tube's bone name

		cchar *pszRL23_CartridgeBoneName;		// RL23: The rotating cartridge's bone name
		f32 fRL23_AngleBetweenCartridgeSlots;	// RL23: The angle between the rocket slots on the cartridge
		f32 fRL23_NoAmmoBaseAngle;				// RL23: The angle of the cartridge when it is empty
		f32 fRL23_CartridgeRocketScale;			// RL23: The scale factor of the rockets sitting on the cartridge
		f32 fRL23_CartridgeRocketPosOffsetZ;	// RL23: The rockets' positional Z component relative to the cartridge bone's origin
		f32 fRL23_CartridgeRocketPosRadius;		// RL23: The rockets' positional radius relative to the cartridge bone's Z axis
		f32 fRL23_ReloadTimeForOneRound;		// RL23: The reload time for one round
		f32 fRL23_OOReloadTimeForOneRound;		// RL23: 1/fRL2_ReloadTimeForOneRound

		FExplosion_GroupHandle_t hExplosionGroupImpact;	// Explosion profile to use when the rocket impacts
		FExplosion_GroupHandle_t hExplosionGroupFire;	// Explosion profile to use when the rocket is fired

		cchar *pszCoronaTexName;			// Corona texture
		f32 fCoronaScale;					// Corona scale
		f32 fCoronaUnitScreenspaceScale;	// 0=worldspace scale is constant, 1=screenspace scale is constant
		f32 fCoronaColorRed;				// Corona color red
		f32 fCoronaColorGreen;				// Corona color green
		f32 fCoronaColorBlue;				// Corona color blue

		f32 fLightColorRed;					// Light color red
		f32 fLightColorGreen;				// Light color green
		f32 fLightColorBlue;				// Light color blue
		f32 fLightRadius;					// Dynamic light radius

		f32 fProjectileMeshScale;			// Scale to use for the projectile itself

		CFSoundGroup *pSoundGroupFire;
		CFSoundGroup *pSoundGroupReload;
		CFSoundGroup *pSoundGroupLoop;

		CEProjExt::CEProj_Merv_StaticParams_t MervStaticParams;
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;						// This EUK's world mesh
		CFAnimCombiner *m_pAnimCombiner;				// This EUK's animation combiner
		CFAnimInst *m_apAnimInstArray[_ANIM_COUNT];		// This EUK's array of CFAnimInst objects (null=none)
		s8 m_nCombinerTapID;							// The combiner's tap ID
		s8 m_nCombinerTapID2;							// 2nd tap ID (for RL2 and RL3 only)
		s8 m_nTubeBoneIndex;							// Index of the tube bone
		CEProjPool::PoolHandle_t m_hProjPool;			// Projectile pool
		CFWorldMesh *m_pCartridgeRocketWorldMeshArray;	// Rockets that are in the cartridge and ready to fire (for visuals only) (NULL=none)
	} _ResourceData_t;


	typedef enum {
		_RL23_CARTMODE_IDLE,
		_RL23_CARTMODE_FIRE,
		_RL23_CARTMODE_RELOAD,
		_RL23_CARTMODE_SWINGDOWN,
		_RL23_CARTMODE_SWINGUP,

		_RL23_CARTMODE_COUNT
	} _RL23_CartMode_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_ROCKET_LAUNCHER];
	static CDamageProfile *m_apDamageProfile[EUK_COUNT_ROCKET_LAUNCHER];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static CFTexInst m_aCoronaTexInst[EUK_COUNT_ROCKET_LAUNCHER];

	static CFMtx43A *m_pCartridgeSlotMtxArray;

	static SmokeTrailAttrib_t m_aSmokeTrailAttrib[EUK_COUNT_ROCKET_LAUNCHER];
//	static SmokeTrailAttrib_t m_RearTubeSmokeTrailAttrib;

	_ResourceData_t m_aResourceData[EUK_COUNT_ROCKET_LAUNCHER];
	_ResourceData_t *m_pResourceData;

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
	f32 m_fAnimInstTimeMul;							// The time scale of the currently playing animation
	CFAnimInst *m_pAnimInstCurrentlyPlaying;		// The CFAnimInst in _ResourceData_t::m_apAnimInstArray that is currently playing (NULL=none)

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to


	// EUK-level-specific:
	static CFMtx43A m_RL23_MtxRotZ;
	static CWeaponRocket *m_pRL23_CallbackThis;
	_RL23_CartMode_e m_nRL23_CartMode;				// Cartridge mode
	s32 m_nRL23_CartridgeBoneIndex;					// Index of cartridge bone
	f32 m_fRL23_CartridgeRotationAngle;				// Rotation angle of rocket cartridge
	f32 m_fRL23_CartridgeRotAngleStart;				// When rotating, this is the start angle
	f32 m_fRL23_CartridgeRotAngleEnd;				// When rotating, this is the destination angle
	f32 m_fRL23_CartridgeSwingAngle;				// Cartridge-empty swing angle
	BOOL m_bRL23_ReloadRequest;						// TRUE when we're waiting to begin the reload process
//	SmokeTrailHandle_t m_hTubeSmokeTrail;			// Smoke coming out of the tube (SMOKETRAIL_NULLHANDLE = none)
//	f32 m_fRL23_TubeSmokeSecs;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponRocket();
	virtual ~CWeaponRocket();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual void ComputeMuzzleUnitDir_WS( CFVec3A *pMuzzleUnitDir_WS ) const;

	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS = NULL );


	// Rumble:
	virtual void KillRumble( void );


	// Item inst:
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); if( m_pResourceData ) return m_pResourceData->m_pWorldMesh; else return NULL; }



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

	virtual void BeginStateChange( void );
	virtual void ClassHierarchyResetToState( void );
	virtual void BeginReload( void );

	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );
	virtual void NotifyAmmoMightHaveChanged( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _RL23_ComputeCartridgeRocketMatrices( void );

	void _ClearDataMembers( void );
	void _SetAnimToCurrentFinalState( void );
	static void _CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _ComputeMtxPalette( void );
	_Anim_e _TransitionStateToAnimIndex( State_e nState ) const;
	_Anim_e _FinalStateToAnimInfo( State_e nState, f32 *pfControlStartUnitValue ) const;
	static void _BuildRocketSkipList( CEProj *pProj );

	CFMtx43A *_GetTubeMtx( void ) const;
	void _InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight );

	void _SpawnTubeEffects( void );
	void _KillTubeEffects( void );
	void _TubeEffectsWork( void );

	void _RL1_Work( void );
	void _RL23_Work( void );

	static void _RL1_SetSmokeTrailAttributes( u32 uUpgradeLevel );		// set this upgrade level to use the RL1 style smoke
	static void _RL2_SetSmokeTrailAttributes( void );
	static void _RL3_SetSmokeTrailAttributes( void );


	FCLASS_STACKMEM_ALIGN( CWeaponRocket );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRocketBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponRocketBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponRocketBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponRocketBuilder );
} FCLASS_ALIGN_SUFFIX;








#endif

