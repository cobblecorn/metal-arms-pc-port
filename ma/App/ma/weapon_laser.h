//////////////////////////////////////////////////////////////////////////////////////
// weapon_laser.h - Mining Laser weapon.
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
// 06/19/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_LASER_H_
#define _WEAPON_LASER_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fworld.h"
#include "fmesh.h"
#include "tracer.h"
#include "eproj.h"
#include "fforce.h"
#include "MuzzleFlash.h"
#include "fdecal.h"
#if FANG_WINGC
#include "pc_laser_charge.h"
#include "fparticle.h"
#endif


class CBot;
class CLaserBeam;
class CDamageProfile;
class CFSoundGroup;







FCLASS_ALIGN_PREFIX class CWeaponLaser : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_WEAPON,

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

		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fReserveAmmoMax;				// Maximum number of reserve rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec

		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fRocketsInPoolCount;			// The number of rockets in the projectile pool
		f32 fMaxLiveRange;					// The maximum distance the rocket can travel before self-destructing
		f32 fUnitRecoil;					// amount of recoil this weapon generates

		f32 fRocketCullDist;				// Don't draw the rocket if it's this far from the camera

		f32 fRechargeRate;					// Rate at which the weapon will recharge (this % per second)
		f32 fOORechargeRate;

		f32 fRocketSpeed;					// Rocket's linear speed
		f32 fTracerLength;					// The length of the tracer in the world.
		f32 fTracerWidth;					// The width of the tracer in the world.

		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		cchar *pszL12_TexName;				// Texture to use for the tracer.

		cchar *pszMuzzleTex;				// Texture to use for the muzzle flash.
		f32 fMuzzleWidth;					// Width of the muzzle flash.
		f32 fMuzzleHeight;					// Height of the muzzle flash.

		cchar *pszDamageProfile;			// Damage profile string (references a damage profile in damage.csv)

		// Sound Effects
		FSndFx_FxHandle_t hFiringSound;		// The sound made when firing

		FDecalDefHandle_t hDecalDef;		// decal def
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;						// This EUK's world mesh
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_LASER];
	static CDamageProfile *m_apDamageProfile[EUK_COUNT_LASER];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static TracerDef_t m_aoTracerDef[EUK_COUNT_LASER];		// Tracer definition

	static CFTexInst m_aTracerTexInst[EUK_COUNT_LASER];

	_ResourceData_t m_aResourceData[EUK_COUNT_LASER];
	_ResourceData_t *m_pResourceData;

	TracerGroupHandle_t m_hTracerGroup;				// Our tracer group for all 3 EUKs

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
													// AKA: seconds until next fire.

	CFAudioEmitter *m_pSoundEmitter;

	f32 m_fRechargeCntDn;							// This much time until the next energy recharge.
	f32 m_L3_fDrainCntDn;							// This much time until the next energy drain (used when firing).

	//CFVec3A m_vecFireUnitDir;
	CFVec3A m_vTargetPos_WS;
	const CFVec3A *m_pvecBuddyFirePos_WS;

	BOOL m_bFireThisFrame;
	BOOL m_bRechargeThisFrame;
	BOOL m_bFiredLastFrame;

	//////////////////////////////////////////////////////////////////////
	// Special things that are only used for the level 3 laser.
	CFVec3A m_vecL3_EndPoint;
	CLaserBeam *m_apL3_LaserBeam[2];
	//
	//////////////////////////////////////////////////////////////////////

#if FANG_WINGC
	s32 m_anMuzzleBoneIndex[EUK_COUNT_LASER];		// Retail muzzle bone per EUK mesh (-1 = use fDistFromWeaponOrigToMuzzle)
	struct _RetailCharge_t {
		f32 fDelay, fChargeSeconds, fReadyEnergy, fEnergyAfterBurst;
		f32 fSpeed, fLength, fWidth, fRange, afColor[3], fPitchStart, fPitchEnd;
		FParticle_DefHandle_t hChargeParticle, hBurstParticle, hEndParticle;
		cchar *apszDamage[3], *pszChargeSound, *pszBurstSound, *pszBurstSound2, *pszReadySound, *pszRicochetSound, *pszImpact, *pszRicochetImpact, *pszDecal;
		CDamageProfile *apDamage[3];
		CFSoundGroup *pChargeSound, *pBurstSound, *pBurstSound2, *pReadySound, *pRicochetSound;
	};
	static _RetailCharge_t m_aRetailCharge[EUK_COUNT_LASER];
	PCLaserCharge m_PlayerCharge;
	CFAudioEmitter *m_pChargeSoundEmitter;
	BOOL m_bChargedShotThisFrame, m_bChargeReadySoundPlayed;
	BOOL m_bSecondaryTapThisFrame;
	u32 m_nChargedShotTier;
	FParticle_EmitterHandle_t m_hChargeParticle;
	CFVec3A m_vChargeMuzzle_WS;
	f32 m_fChargePitchBase;
#endif

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to

	// EUK-level-specific:


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponLaser();
	virtual ~CWeaponLaser();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	// Targeting
	virtual f32 GetProjectileSpeed(void) const;


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS = NULL );
#if FANG_WINGC
	void CancelPlayerCharge( void );
	BOOL SupportsPlayerCharge( void ) const;
	BOOL UsesMouseChargeControls( void ) const;
	FINLINE BOOL PlayerSecondaryTap( void ) const { return m_bSecondaryTapThisFrame; }
#endif


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
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );

	virtual void ClassHierarchyResetToState( void );

	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );
	virtual void NotifyAmmoMightHaveChanged( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	static void _TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );
#if FANG_WINGC
	static BOOL _ReadRetailProperties( void );
	u32 _PlayerChargeTrigger( f32 fTrigger, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS );
	u16 _PlayerChargeCost( void ) const;
	void _StopChargeSound( void );
	void _ConfigureNormalTracer( TracerDef_t *pTracer ) const;
	void _ConfigureChargedTracer( TracerDef_t *pTracer ) const;
	static CDamageProfile *_ChargedDamageProfile( u16 nShot );
	static void _ChargedTracerCallback( TracerDef_t *pTracer, TracerKillReason_e nReason, const FCollImpact_t *pImpact );
	static void _ChargedSkipList( void *pWeapon );
	static void _ChargeParticleKilled( FParticle_EmitterHandle_t hEmitter, void *pUser );
	void _StopChargeParticle( void );
	friend void port_LaserRetailRegression( CBot *, int &, u32 &, u32 );
#endif

	FCLASS_STACKMEM_ALIGN( CWeaponLaser );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponLaserBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponLaserBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponLaserBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponLaserBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
