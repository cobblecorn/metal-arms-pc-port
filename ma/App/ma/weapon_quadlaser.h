//////////////////////////////////////////////////////////////////////////////////////
// weapon_quadlaser.h - Predator's quad laser
//
// Author: Mike Elliott
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
// 12/30/02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_QUADLASER_H_
#define _WEAPON_QUADLASER_H_ 1

#include "fang.h"
#include "fparticle.h"
#include "weapon.h"
#include "tracer.h"
#include "fforce.h"
#include "smoketrail.h"
#include "fexplosion.h"
#include "fdecal.h"

class CEParticle;
class CFSoundGroup;



FCLASS_ALIGN_PREFIX class CWeaponQuadLaser : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef enum {
		ARM_TL		= 0,
		ARM_TR		= 1,
		ARM_BL		= 2,
		ARM_BR		= 3,

		ARM_COUNT	= 4
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
	typedef struct {
		cchar *pszMeshName;

		f32 fOORoundsPerSecPrimary;			// OO fire rate (for the weapon, not per barrel)
		f32 fOORoundsPerSecSecondary;		// OO fire rate for secondary attack

		f32 fOOAimSpreadTime;				// OO how long with continued firing to get to max shot spread
		f32 fOOAimRecoveryTime;				// OO time to recover our aim
		f32 fMaxShotSpread;

		cchar *pszFireParticle;				// muzzle smoke particle effect
		cchar *pszPriTracerTex;				// Texture to use for the tracer.
		cchar *pszSecTracerTex;				// Texture to use for the tracer.

		f32	fMaxLiveRange;					// max distance a tracer can fire before it's destroyed (pri & sec)

		f32 fPriTracerLength;
		f32 fPriTracerWidth;
		f32 fPriTracerSpeed;

		f32 fSecTracerLength;
		f32 fSecTracerWidth;
		f32 fSecTracerSpeed;

		f32 fMinTargetAssistDist;
		f32 fMaxTargetAssistDist;

		f32 fRechargeTime;
		f32	fPrimaryDischargeTime;
		f32 fSecondaryDischargeTime;

		f32 fSecondaryFireRechargeTime;
		f32 fOOSecondaryFireRechargeTime;
		f32 fSecondaryFireBarrageTime;
		f32 fOOSecondaryFireBarrageTime;

		CDamageProfile *pDamageProfile1;	// Damage profile for primary weapon
		CDamageProfile *pDamageProfile2;	// Damage profile for secondary weapon

		FExplosion_GroupHandle_t hExplosionGroup;	// Explosion profile to use for secondary weapon impact

		CFSoundGroup *pSoundGroupFire1;		// Sound group for primary weapon
		CFSoundGroup *pSoundGroupFire2;		// Sound group for secondary weapon
		FDecalDefHandle_t hDecalDef;

	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;						// This EUK's world mesh
	} _ResourceData_t;

	typedef enum {
		SECONDARY_RECHARGING,
		SECONDARY_READY,
		SECONDARY_FIRING
	} SecondaryFireStatus_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
    s32 m_nArmFiredThisFrame;



//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL			m_bSystemInitialized;
	static u32			m_uWpnClassClientCount;					// Number of this class using the class-wide resources

	static _UserProps_t		m_aUserProps[EUK_COUNT_QUADLASER];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t			m_aUserPropMapTable[];

	static SmokeTrailAttrib_t m_SmokeTrailAttrib;

	static TracerDef_t	m_aTracerDefPri[EUK_COUNT_QUADLASER];			// Tracer definition
	static TracerDef_t	m_aTracerDefSec[EUK_COUNT_QUADLASER];		// Tracer definition for special attack
	static CFTexInst	m_aTracerTexInstPri[EUK_COUNT_QUADLASER];
	static CFTexInst	m_aTracerTexInstSec[EUK_COUNT_QUADLASER];

	static FParticle_DefHandle_t m_ahFireParticleDef[EUK_COUNT_QUADLASER];

	_ResourceData_t m_aResourceData[EUK_COUNT_QUADLASER];
	_ResourceData_t *m_pResourceData;

	_UserProps_t *m_pUserProps;

	static TracerGroupHandle_t m_hTracerGroupPri[EUK_COUNT_QUADLASER];	// Our primary fire tracer group
	static TracerGroupHandle_t m_hTracerGroupSec[EUK_COUNT_QUADLASER];	// Our secondary fire tracer group

	f32 m_fUnitCharge;								// used for HUD display
	f32 m_fSecondaryFireTimer;						// countdown timer for secondary fire
	f32 m_fFireTimer;								// countdown timer for shooting
	f32 m_fEnergyTimer;								// countdown timer for handling charge (ammo)

	f32 m_fUnitAimSpread;							// amount our aim has drifted because of continued firing
	f32	m_fUnitRecoilAdjust;						// adjust our desired final recoil
	u32 m_uDisabledArms;							// used to disable arms, if a bit is set, arm is broken and will no longer fire

	u32 m_uSoundCounter;

	CFMtx43A*	m_apMtxFireBones[ARM_COUNT];		// matrices for our fire points
	u32			m_uCurrentFireSeg;					// which segment is firing next
	CFVec3A		m_avAimPoints[ARM_COUNT];			// each arm can aim at a different spot
	CFVec3A		m_avDesiredAimPoints[ARM_COUNT];	// where we want each arm to aim. (m_avAimPoints chases m_avDesiredAimPoints which usually chases the bot's aimpoint)
	u32			m_auFirePos[ARM_COUNT];				// sets our fire order

	f32			m_afUnitRecoilTime[ARM_COUNT];		// handles recoil for shoulder joints
	BOOL		m_abRecoilOn[ARM_COUNT];			// are we in the process of recoiling?

	CFVec3A		m_avAimSpread[ARM_COUNT];			// spread factor for aiming
	CFVec3A		m_avLastSecFirePoint[ARM_COUNT];	// the point this arm fired at last time it fired
	f32			m_afLastSecFireTimer[ARM_COUNT];	// reset when one of the guns fires a sec shot
	
	f32			m_fUnitMuzzleSmokeIntensity;

	CEParticle	*m_apMuzzleSmokeParticles[ARM_COUNT];

	BOOL		m_bTrigger1Down;					// is trigger 1 down?
	BOOL		m_bTrigger2Down;					// is trigger 2 down?
	BOOL		m_bEnableArmLag;					// if true, the arms take time to travel to their position, otherwise they're just set there

	CFQuatA		m_aqArm0[ARM_COUNT];
	CFQuatA		m_aqArm1[ARM_COUNT];
	CFQuatA		m_aqArm2[ARM_COUNT];

	u64 m_uLastSoundTime;

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to

	SecondaryFireStatus_e m_eSecondaryStatus;
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponQuadLaser();
	virtual ~CWeaponQuadLaser();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	// Targeting
	virtual f32 GetProjectileSpeed(void) const;


	// Firing:
	void ComputeMuzzlePoint_WS( u32 uArm, CFVec3A *pMuzzlePoint_WS ) const;
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS = NULL );

	void SetFireBones( const CFWorldMesh *pWorldMesh, cchar *pszTLBone, cchar *pszTRBone, cchar *pszBLBone, cchar *pszBRBone );
	
	FINLINE void EnableArmLag( BOOL bEnable ) { FASSERT(IsCreated() ); m_bEnableArmLag = bEnable; };
	void ResetAimPoints( const CFVec3A &vTgtPoint );

	//FNs for aiming the weapon's arms:
	void AimSeg0( u32 uIdx, f32 fUnitCtl, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AimSeg1( u32 uIdx, f32 fUnitCtl, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void AimSeg2( u32 uIdx, f32 fUnitCtl, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	// Rumble:
	virtual void KillRumble( void );
	virtual void CheckpointRestore( void );


	//DEBUG:
	void DebugDraw( void );

	FINLINE BOOL IsArmDisabled( u32 uArm ) const { FASSERT( IsCreated() ); FASSERT( uArm < ARM_COUNT ); return ((m_uDisabledArms >> uArm) & 0x01); };
	FINLINE void DisableArm( u32 uArm )			 { FASSERT( IsCreated() ); FASSERT( uArm < ARM_COUNT ); FMATH_SETBIT( m_uDisabledArms, uArm ); };
	FINLINE void EnableAllArms( void )			 { FASSERT( IsCreated() ); m_uDisabledArms = 0; };
	FINLINE	void AdvanceFireArm( void )			 { FASSERT( IsCreated() ); if( ++m_uCurrentFireSeg > (ARM_COUNT-1) ) { m_uCurrentFireSeg = 0; } };

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); if( m_pResourceData ) return m_pResourceData->m_pWorldMesh; else return NULL; }

	FINLINE f32* GetUnitSecondaryChargeTimerPtr( void ) { FASSERT( IsCreated() ); return &m_fUnitCharge; }

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual void ClassHierarchyResetToState( void );
	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );

	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _FirePrimary( void );
	void _FireSecondary( void );
	void _ClearDataMembers( void );
	static void _TracerMovedSecondary( void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied);
	static void _TracerKilledCallbackPrimary( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );
	static void _TracerKilledCallbackSecondary( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );
	static void _TracerBuildTrackerSkipList( void *pThisWeapon );
	void _ComputeAimPoints( void );

	void _StartSecondaryFire( void );

	void _StartRecoil( u32 uArm );
	void _HandleRecoil( void );
	static BOOL _InitSmokeTrail( void );
	static void _InitDefaultUserProps( void );

	FCLASS_STACKMEM_ALIGN( CWeaponQuadLaser );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponLaserBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponQuadLaserBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponQuadLaserBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponQuadLaserBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
