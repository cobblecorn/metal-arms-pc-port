//////////////////////////////////////////////////////////////////////////////////////
// site_RatGun.h
//
// Author: Michael Scholz
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/12/03 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SITERATGUN_H_
#define _SITERATGUN_H_ 1

#include "site_botWeapon.h"
#include "fAntenna.h"
#include "Hud2.h"
#include "fforce.h"
#include "fdecal.h"

FCLASS_ALIGN_PREFIX class CRatGun : public CBotSiteWeapon::CSiteWeaponAnimator
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct
	{
		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fReserveAmmoMax;				// Maximum number of reserve rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec
		f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing
		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fBestShotSpreadFactor;			// Tightest shot spread factor
		f32 fWorstShotSpreadFactor;			// Widest shot spread factor
		f32 fShotSpreadSpeed;				// Speed at which shot spreads out
		f32 fShotSpreadRecoverySpeed;		// Speed at which shot spreads in

		f32 fMuzzleWidth_Z;					// Width of the muzzle flash Z
		f32 fMuzzleHeight_Z;				// Height of the muzzle flash Z
		f32 fMuzzleAlpha_Z;					// Alpha of muzzle flash Z

		f32 fMuzzleScale_XY;				// Scale of muzzle flash XY
		f32 fMuzzleAlpha_XY;				// Alpha of muzzle flash XY

		f32 fMuzzleScale_Mesh;				// Scale of muzzle flash mesh
		f32 fMuzzleRot_Mesh;				// Rotation of muzzle flash mesh

		f32 fMuzzleScale_Impact;			// Scale of impact flash
		f32 fMuzzleAlpha_Impact;			// Alpha of impact flash
		f32 fMuzzleOffset_Impact;			// Offset of impact flash

		f32 fBulletDamage;
		f32 fExplosionDamage;
		f32 fFlameDamage;

		cchar *pszDamageProfile;			// Damage profile string (references a damage profile in damage.csv)

		f32 fSoundRadius;					// Distance sound from this weapon travels

		f32 fCameraShakeIntensity;
		f32 fCameraShakeDuration;
		FDecalDefHandle_t hBulletDecal;

	} _RatGunSpewProps_t;

	typedef enum
	{
		BONE_BALLTURRETBASE,
		BONE_BALLTURRET,
		BONE_GUN_BARREL,
		BONE_PRIMARY_FIRE,
		BONE_ATTACHPOINT_GUNNER,
		BONE_BALLTURRETRIBS,
		BONE_SHELL_EJECT,
		BONE_COUNT
	} Bone_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum
	{
		ANIM_IDLE_COUNT = 0,
		TAG_POINT_COUNT = 4,						// Number of tag points on guy
		MAX_SIMULTANEOUS_FIRE_SOUNDS	= 3
	};

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_REST,
		ANIMCONTROL_AIM_SUMMER,
		ANIMCONTROL_OPEN,
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;

	// Animation taps:
	typedef enum
	{
		ANIMTAP_REST,
		ANIMTAP_AIM_SUMMER,
		ANIMTAP_OPEN,
		ANIMTAP_BASE_COUNT
	} AnimTap_e;

	// Base animations:
	typedef enum
	{
		ANIM_OPEN,
		ANIM_BASE_COUNT,
	} Anim_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CFAntenna m_Antenna;												// antenna animator
	static cchar *m_apszBoneNameList[BONE_COUNT];						// Array of bone names

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static const FGameData_TableEntry_t m_aSpewPropVocab[];
	static const FGameDataMap_t m_aGameDataMap[];
	static _RatGunSpewProps_t	m_aSpewProps[2];
		// Bot animation stack data:
	static CBotAnimStackDef m_AliveAnimStackDef;						// Animation stack definition data
	
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_OpenClose[];
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT];

	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	static CDamageProfile *m_aDamageProfile[2];

	static CFDebrisGroup*  m_pDebrisGroup;
	static FMesh_t*		   m_pShellMesh;
	static CFDebrisMesh    m_DebrisShellMesh;

	static s32 m_nBoneIndexSwivel;

	CFAudioEmitter *m_apAudioEmitterFire[MAX_SIMULTANEOUS_FIRE_SOUNDS];	// Audio emitter used when we're playing the bullet firing sound (NULL=not playing forming sound)
	u32 m_nAudioEmitterIndex;						// Index of next audio emitter in m_apAudioEmitterFire[] to use

	f32 m_fOpenVelocity;
	f32 m_fOldNormX;
	f32 m_fOldNormY;
	HudMode_e m_OldHudMode;
	CReticle::Type_e m_OldReticleType;
	u32 m_uOldHudFlags;

	// Mesh handles for geometry
	CFWorldMesh m_AliveMesh;

	// rest anim sources
	CFAnimMeshRest		m_AliveMeshRestAnimSource;

	CFMtx43A	m_MtxRecoil;
	CFMtx43A*	m_pMuzzlePtArray[1];
	BOOL		m_bFirePrimaryThisFrame;

	u16 m_nSpewClipAmmo;							// Number of rounds in the clip (if infinite clip ammo, this is INFINITE_AMMO)
	u16 m_nSpewReserveAmmo;							// Number of rounds outside the clip (if infinite reserve ammo, this is INFINITE_AMMO)
	f32 m_fSpewFiringDelay;
	f32 m_fSpewUnitShotSpread;
	f32 m_fSpewSecondsUntilNextTracer;
	f32 m_fSpewSecondsCountdownTimer;

	f32 m_fRecoilDistance;
	f32 m_fRecoilReturnRate;

	f32		m_fRecoil0;
	CBot*	m_pOwnerBot;

	CFAnimManMtx	m_AliveAnimManMtxAim;

	CFMtx43A* pmtxCameraAttachPtWS;

	CBotSiteWeapon::State_e m_ePreviousState;
	_RatGunSpewProps_t* m_pSpewProps;
	CDamageProfile* m_pDamageProfile;
	f32			m_fNormHealth;
	FForceHandle_t m_hForce[2];	// Force feedback handle so we can kill it when we need to
	f32			m_fIdlingVelocity;
	f32			m_fIdlingVelocityDamp;
	f32			m_fIdlingAcceleration;

	BOOL m_bPossessed;
	s32 m_nAttachBotPtBoneIdx;
	s32 m_nShellEjectPtBoneIdx;
	const CFVec3A *m_apTagPoint_WS[TAG_POINT_COUNT];	// Tag points in world space
	BOOL m_bResettingTurret;						// TRUE if turret is returning to neutral position after driver exits
	FParticle_DefHandle_t m_hDustParticle;			// handle to rat gun impact dust
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	virtual ~CRatGun();
	virtual BOOL	Create( CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 userTypeBits );
	virtual void	RemoveFromWorld(void);
	virtual void	AddToWorld(void);
	virtual void	Destroy(void);

	virtual u32		GetTagPointCount( void ) const;
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;

	virtual CFMtx43A* AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );

	virtual u32		TriggerWorkPrimary( f32 fUnitTriggerVal1 );
	virtual u32		TriggerWorkSecondary( f32 fUnitTriggerVal2 );

	virtual void 	Work();
	virtual void 	CueNewState();

	virtual void	ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual BOOL	CheckpointSave( void );
	virtual void	CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData );
	virtual void	AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual BOOL	CanOccupyStation(CBot * pBot);

			BOOL	GetOpenOrClosed(void);
			BOOL	SetOpenOrClosed(BOOL bOpen, f32 fHowLongItShouldTakeSecs );
	
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
			void 	_IdlingWork(void);						// my other tick
			void	_EnteringPossessedWork(void);
			void 	_PossessedPowerUpWork(void);						// my other tick
			void 	_PowerDownWork(void);					// my other other tick
			void 	_DeathWork(void);						// my other other other tick
			void 	_CuePowerUp(void);							// turn me on
			void 	_CuePowerDown(void);// WHIRRRWhiRRwwwhir.
			void	_CueDeath(void);		// time to die
			void	_CuePossessed(BOOL bGoPosed);		// transform!
			void	_HandleFireThisFrame(void);
			void	_EjectShell( void );
			void	_KillAudioEmitters(void);

	FCLASS_STACKMEM_ALIGN( CRatGun );
} FCLASS_ALIGN_SUFFIX;

#endif