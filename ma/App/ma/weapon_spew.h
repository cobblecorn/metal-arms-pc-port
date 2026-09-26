//////////////////////////////////////////////////////////////////////////////////////
// weapon_spew.h - Small Projectile Emitter Weapon.
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

#ifndef _WEAPON_SPEW_H_
#define _WEAPON_SPEW_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fforce.h"
#include "fdecal.h"


class CBot;
class CDamageProfile;
class CFSoundGroup;
class CEParticle;
class CFAudioEmitter;



FCLASS_ALIGN_PREFIX class CWeaponSpew : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_WEAPON,
		_MESH_CLIP,

		_MESH_COUNT
	} _Mesh_e;


	typedef enum {
		_BONE_PRIMARY_FIRE,					// Where the hurt comes out of
		_BONE_CLIP_ATTACH,					// Where the clip attaches to
		_BONE_L3_SPIN_CARTRIDGE,			// L3 only: The cartridge to spin
		_BONE_L3_SPIN_BARREL,				// L3 only: The barrel to spin

		_BONE_COUNT
	} _Bone_e;


	enum {
		MAX_SIMULTANEOUS_FIRE_SOUNDS	= 3
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

	typedef struct {						// NOTE: If you change this structure, you must also change _Anim_e and m_aUserPropVocab
		cchar *apszMeshName[_MESH_COUNT];	// Mesh names
		cchar *apszBoneName[_BONE_COUNT];	// Bone names

		f32 fClipAmmoMax;					// Maximum number of clip rounds (<0 means infinite)
		f32 fReserveAmmoMax;				// Maximum number of reserve rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec

		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera
		f32 fClipCullDist;					// Don't draw the clip if it's this far from the camera

		f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing
		f32 fUnitRecoil;					// amount of recoil this weapon generates

		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		f32 fUnitCamVibration;				// Amount camera vibrates when firing

		f32 fBestShotSpreadFactor;			// Tightest shot spread factor
		f32 fWorstShotSpreadFactor;			// Widest shot spread factor
		f32 afScopeSpreadMult[EUK_COUNT_SCOPE];	// Spread multiplier when this weapon is used with the scope

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

		f32 fL3_CartridgeMaxRevsPerSec;		// L3: The maximum revolutions per second of the cartridge
		f32 fL3_CartridgeOOMaxRevsPerSec;	// L3: 1.0f / fL3_CartridgeMaxRevsPerSec
		f32 fL3_CartridgeRotAccel;			// L3: The cartridge rotation acceleration
		f32 fL3_CartridgeRotDecel;			// L3: The cartridge rotation deceleration

		FParticle_DefHandle_t hParticleSmoke;	// Smoke particle
		f32 fHeatUpSpeed;						// Amount of time it takes to go from no smoke to min smoke, and from min smoke to max smoke
		f32 fCoolDownSpeed;						// Vice-versa of fHeatUpSpeed
		f32 fMaxSmokeUnitIntensity;				// Max smoke unit intensity

		CDamageProfile *pDamageProfile;			// Damage profile

		FMesh_t *pMeshEjectClip;				// Eject clip mesh

		// Sound Effects:
		CFSoundGroup *pSoundGroupFire;			// Fire sound
		CFSoundGroup *pSoundGroupEmpty;			// Empty sound
		CFSoundGroup *pSoundGroupEjectClip;		// Eject clip sound
		CFSoundGroup *pSoundGroupAttachClip;	// Attach clip sound
		CFSoundGroup *pSoundGroupSlapInClip;	// Slap-in clip sound
		FDecalDefHandle_t hDecalDef;			// Decal Def
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;			// This EUK's world mesh
		CFAnimCombiner *m_pAnimCombiner;	// This EUK's animation combiner
		CFAnimMeshRest *m_pAnimRest;		// Rest Animation
		const CFMtx43A *m_pFireMtx_WS;		// The fire bone matrix
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_SPEW];

	static BOOL m_bSystemInitialized;
	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];
	static CWeaponSpew *m_pCallbackThis;

	_ResourceData_t m_aResourceData[EUK_COUNT_SPEW];
	_ResourceData_t *m_pResourceData;
	_UserProps_t *m_pUserProps;

	CMeshEntity *m_pClipMeshEntity;					// The clip mesh entity (never NULL)

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
	f32 m_fDebrisSecondsCountdownTimer;				// Used to determine when we can spawn more debris
	f32 m_fUnitShotSpread;							// 0.0=tightest, 1.0=widest
	f32 m_fSecondsUntilNextTracer;					// Seconds remaining until we can draw another tracer
	BOOL m_bFireThisFrame;							// TRUE when we should fire a round on the next call to ClassHierarchyWork()
	CFVec3A m_TargetPos_WS;							// The target position to fire at
	const CFVec3A *m_pBuddyFirePos_WS;				// The buddy's fire position

	CEParticle *m_pParticleSmoke;					// Smoke particle entity (NULL=none)
	f32 m_fUnitHeatup;								// Amount gun is heated up

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to
	CFAudioEmitter *m_apAudioEmitterFire[MAX_SIMULTANEOUS_FIRE_SOUNDS];	// Audio emitter used when we're playing the bullet firing sound (NULL=not playing forming sound)
	u32 m_nAudioEmitterIndex;						// Index of next audio emitter in m_apAudioEmitterFire[] to use


	// EUK-level-specific:
	f32 m_fCartridgeSpinAngle;						// The cartridge spin angle
	f32 m_fCartridgeRotSpeed;						// The cartridge rotational speed




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );


	// Creation:
	CWeaponSpew();
	virtual ~CWeaponSpew();
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS = NULL );


	// Rumble:
	virtual void KillRumble( void );


	// Checkpoint save/restore
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual void CheckpointRestore( void );


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
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );

	virtual void ClassHierarchyResetToState( void );
	virtual void BeginReload( void );

	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );
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

	void _ClearDataMembers( void );
	void _KillAudioEmitters( void );
	void _PickRandomDebrisChunkTime( void );
	void _AddClipToWorld( void );
	void _RemoveClipFromWorld( void );
	static void _CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _FireRound( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fJitterFactor, BOOL bBuddy );

	void _SpawnImpactChunks( const CGCollMaterial *pCollMaterial, const FCollImpact_t *pCollImpact );
	void _UpdateSmoke( void );
	void _KillSmoke( void );


	FCLASS_STACKMEM_ALIGN( CWeaponSpew );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponSpewBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponSpewBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponSpewBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponSpewBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
