//////////////////////////////////////////////////////////////////////////////////////
// weapon_ripper.h - Small Projectile Emitter Weapon.
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

#ifndef _WEAPON_RIPPER_H_
#define _WEAPON_RIPPER_H_ 1

#include "fang.h"
#include "weapon.h"
#include "eproj.h"
#include "fforce.h"

class CBot;
class CRipperBeam;
class CDamageProfile;
class CFAnimCombiner;
class CFAnimMeshRest;
class CMeshEntity;



FCLASS_ALIGN_PREFIX class CWeaponRipper : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_MESH_WEAPON,
		_MESH_CLIP,
		_MESH_SAW,

		_MESH_COUNT
	} _Mesh_e;


	typedef enum {
		_BONE_PRIMARY_FIRE,					// Where the hurt comes out of
		_BONE_CLIP_ATTACH,					// Where the clip attaches to
		_BONE_L3_SPIN_CARTRIDGE,			// L3 only: The cartridge to spin

		_BONE_COUNT
	} _Bone_e;


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

		u32 nProjectilesInPoolCount;		// The number of projectiles in the projectile pool

		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera
		f32 fClipCullDist;					// Don't draw the clip if it's this far from the camera

		f32 fSawCullDist;					// Don't draw the saw if it's this far from the camera
		f32 fSawScale;						// The scale at which to draw the saw
		f32 fSawSpeed;						// The speed at which the saw projectile travels

		f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing
		f32 fUnitRecoil;					// amount of recoil this weapon generates

		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		f32 fShotSpreadFactor;				// Shot spread factor
		f32 afScopeSpreadMult[EUK_COUNT_SCOPE];	// Spread multiplier when this weapon is used with the scope

		cchar *pszImpactDamageProfile;		// Impact damage profile string (references a damage profile in damage.csv)
		cchar *pszCarveDamageProfile;		// Carve damage profile string (references a damage profile in damage.csv) (used only for L3)

		f32 fSawCarveDamageSecs;			// Proj: Amount of time carving lasts
		f32 fSawCarveInitialSpeed;			// Proj: Initial speed of carving saw
		u32 nMinRicochetCount;				// Proj: Minimum guaranteed ricochet count
		u32 nMaxRicochetCount;				// Proj: Maximum ricochet count

		cchar *pszStreamerTexName;			// Streamer texture name
		f32 fStreamerAlpha;					// Streamer alpha
		f32 fStreamerWidth;					// Streamer width
		u32 nStreamerVtxCount;				// Streamer vertex count
		f32 fStreamerSamplesPerSec;			// Streamer samples per second

		f32 fDamageSphereRadius;			// Used to damage nearby entities
		f32 fMuzzleScale_Impact;			// Scale of impact flash
		f32 fMuzzleAlpha_Impact;			// Alpha of impact flash
		f32 fMuzzleOffset_Impact;			// Offset of impact flash

		f32 fL3_CartridgeMaxRevsPerSec;		// L3: The maximum revolutions per second of the cartridge
		f32 fL3_CartridgeOOMaxRevsPerSec;	// L3: 1.0f / fL3_CartridgeMaxRevsPerSec
		f32 fL3_CartridgeRotAccel;			// L3: The cartridge rotation acceleration
		f32 fL3_CartridgeRotDecel;			// L3: The cartridge rotation deceleration

		FExplosion_GroupHandle_t hExplosionGroupShatter;	// Explosion group used when saw blade shatters

		FMesh_t *pMeshEjectClip;			// Eject clip mesh

		CFSoundGroup *pSoundGroupFire;			// Fire sound
		CFSoundGroup *pSoundGroupEmpty;			// Empty sound
		CFSoundGroup *pSoundGroupEjectClip;		// Eject clip sound
		CFSoundGroup *pSoundGroupAttachClip;	// Attach clip sound
		CFSoundGroup *pSoundGroupSlapInClip;	// Slap-in clip sound

		CFSoundGroup *pSoundGroupRicochet;		// Blade ricochet sound
		CFSoundGroup *pSoundGroupStick;			// Blade stick sound
		CFSoundGroup *pSoundGroupCutStart;		// Blade cut start sound
		CFSoundGroup *pSoundGroupCutStop;		// Blade cut stop sound
		CFSoundGroup *pSoundGroupCutLoop;		// Blade cut loop sound
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;			// This EUK's world mesh
		CFAnimCombiner *m_pAnimCombiner;	// This EUK's animation combiner
		CFAnimMeshRest *m_pAnimRest;		// Rest Animation
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static _UserProps_t m_aUserProps[EUK_COUNT_RIPPER];
	static CDamageProfile *m_apImpactDamageProfile[EUK_COUNT_RIPPER];
	static CDamageProfile *m_apCarveDamageProfile[EUK_COUNT_RIPPER];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static CEProjPool::PoolHandle_t m_ahProjPool[EUK_COUNT_RIPPER];
	static CFTexInst _aStreamerTexInst[EUK_COUNT_RIPPER];
	static CEProjExt::CEProj_Saw_Params_t m_aSawParms[EUK_COUNT_RIPPER];

	static CWeaponRipper *m_pCallbackThis;

	static f32 m_afSecsBetweenEmptySounds[EUK_COUNT_RIPPER];	// Seconds between the playing of empty sounds

	_ResourceData_t m_aResourceData[EUK_COUNT_RIPPER];
	_ResourceData_t *m_pResourceData;
	_UserProps_t *m_pUserProps;

	CMeshEntity *m_pClipMeshEntity;					// The clip mesh entity (never NULL)

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
	f32 m_fPlayEmptySoundTimer;						// Used to make sure we don't spawn too many empty sounds
	BOOL m_bFireThisFrame;							// TRUE when we should fire a round on the next call to ClassHierarchyWork()
	CFVec3A m_TargetPos_WS;							// targeted point
	const CFVec3A *m_pBuddyFirePos_WS;				// The buddy's fire position

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponRipper();
	virtual ~CWeaponRipper();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPoint_WS, const CFVec3A *pBuddyFirePos_WS = NULL );


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
	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );

	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );

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
	void _AddClipToWorld( void );
	void _RemoveClipFromWorld( void );
	static void _CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _FireRound( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fJitterFactor, BOOL bBuddy );
	static void _BuildProjSkipList( CEProj *pProj );


	FCLASS_STACKMEM_ALIGN( CWeaponRipper );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRipperBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponRipperBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponRipperBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponRipperBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
