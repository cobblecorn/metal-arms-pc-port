//////////////////////////////////////////////////////////////////////////////////////
// site_WallSentry.h
//
// Author: Michael Scholz
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
// 11/05/02 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SITEWALLSENTRY_H_
#define _SITEFLOORSENTRY_H_ 1

#include "site_botWeapon.h"
#include "fdecal.h"

FCLASS_ALIGN_PREFIX class CWallSentry : public CBotSiteWeapon::CSiteWeaponAnimator
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
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
		FDecalDefHandle_t hBulletDecal;

	} _WallSpewProps_t;

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
		ANIMCONTROL_POWER_DOWN,
		ANIMCONTROL_DEATH,
		ANIMCONTROL_AIM_SUMMER,
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;

	// Animation taps:
	typedef enum
	{
		ANIMTAP_REST,
		ANIMTAP_POWER_DOWN,
		ANIMTAP_DEATH,
		ANIMTAP_AIM_SUMMER,
		ANIMTAP_BASE_COUNT
	} AnimTap_e;

	// Base animations:
	typedef enum
	{
		ANIM_POWER_DOWN,
		ANIM_DEATH,
		ANIM_BASE_COUNT,
	} Anim_e;

	typedef enum
	{
		BONE_SWIVEL,	
		BONE_ARM_U,  	
		BONE_ARM_L,    
		BONE_TORSO,	
		BONE_BARREL_L,
		BONE_BARREL_R,
		BONE_PRIMARY_FIRE_L,	
		BONE_PRIMARY_FIRE_R,	
		BONE_COUNT
	} Bone_e;	



//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static const FGameData_TableEntry_t m_aSpewPropVocab[]; 
    static const FGameDataMap_t m_aGameDataMap[];
	static _WallSpewProps_t	m_pSpewProps;
	static CDamageProfile *m_apDamageProfile;

	// Bot animation stack data:
	static CBotAnimStackDef m_AliveAnimStackDef;						// Animation stack definition data
	static CBotAnimStackDef m_DeadAnimStackDef;							// Animation stack definition data
	
	static cchar *m_apszBoneNameList[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBoneNameList2[4];								// Array of bone names

	static const u8 m_aBoneEnableIndices_Death[];
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animations
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT];

	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];
	
	CFAudioEmitter *m_apAudioEmitterFire[MAX_SIMULTANEOUS_FIRE_SOUNDS];	// Audio emitter used when we're playing the bullet firing sound (NULL=not playing forming sound)
	u32 m_nAudioEmitterIndex;						// Index of next audio emitter in m_apAudioEmitterFire[] to use

	CBotAnimStack m_DeadAnim;												// Animation stack

	// Mesh handles for geometry
	CFWorldMesh m_AliveMesh;
	CFWorldMesh m_DeadMesh;

	// rest anim sources
	CFAnimMeshRest		m_AliveMeshRestAnimSource;
	CFAnimMeshRest		m_DeadMeshRestAnimSource;

	CFMtx43A		m_MtxRecoil;

	CFMtx43A* m_apMuzzlePts[2];
	CFMtx43A* m_pTorsoBone;

	BOOL m_bFirePrimaryThisFrame;
	
	u16 m_nSpewClipAmmo;							// Number of rounds in the clip (if infinite clip ammo, this is INFINITE_AMMO)
	u16 m_nSpewReserveAmmo;							// Number of rounds outside the clip (if infinite reserve ammo, this is INFINITE_AMMO)
	f32 m_fSpewUnitShotSpread;
	f32 m_fSpewSecondsUntilNextTracer;
	f32 m_fSpewSecondsCountdownTimer;
	
	f32 m_fRecoilDistance;
	f32 m_fRecoilReturnRate;

	f32		m_fRecoil0;
	BOOL	m_bFireLeftBarrel;
	CBot*	m_pOwnerBot;

	CFAnimManMtx	m_AliveAnimManMtxAim;
	CFAnimManMtx	m_DeadAnimManMtxAim;

	const CFVec3A *m_apTagPoint_WS[TAG_POINT_COUNT];	// Tag points in world space
	CFWorldMesh*	m_pActiveMesh;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	virtual ~CWallSentry();
	virtual BOOL	Create( CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 userTypeBits );
	virtual void	RemoveFromWorld(void);
	virtual void	AddToWorld(void);
	virtual void	Destroy(void);

	virtual u32		GetTagPointCount( void ) const;
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;

	virtual CFWorldMesh* GetWorldMesh(void){return m_pActiveMesh;}

	virtual CFMtx43A* AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );

	virtual u32		TriggerWorkPrimary( f32 fUnitTriggerVal1);
	virtual u32		TriggerWorkSecondary( f32 fUnitTriggerVal2);

	virtual void 	Work();
	virtual void 	CueNewState();

	virtual void	ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );
	virtual BOOL	CheckpointSave( void );
	virtual void	CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData );
	virtual void	AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual BOOL	CanOccupyStation(CBot * pBot);

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
			void 	_PowerUpWork(void);						// my other tick
			void 	_PowerDownWork(void);					// my other other tick
			void 	_DeathWork(void);						// my other other other tick
			void 	_CuePowerUp(void);							// turn me on
			void 	_CuePowerDown(void);// WHIRRRWhiRRwwwhir.
			void	_CueDeath(void);		// time to die
			void	_SetNewWorldMesh( CFWorldMesh *pSomeMesh );
			void	_HandleFireThisFrame(void);
			void	_KillAudioEmitters(void);

	FCLASS_STACKMEM_ALIGN( CWallSentry );
} FCLASS_ALIGN_SUFFIX;

#endif