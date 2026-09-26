//////////////////////////////////////////////////////////////////////////////////////
// weapon_flamer.h - Small Projectile Emitter Weapon.
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

#ifndef _WEAPON_FLAMER_H_
#define _WEAPON_FLAMER_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fforce.h"



class CFlamer;
class CMeshEntity;
class CBot;
class CFlamerBeam;
class CDamageProfile;
class CFAnimCombiner;
class CFAnimMeshRest;




FCLASS_ALIGN_PREFIX class CWeaponFlamer : public CWeapon {
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

		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera
		f32 fClipCullDist;					// Don't draw the clip if it's this far from the camera

		f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing
		f32 fUnitRecoil;					// amount of recoil this weapon generates

		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		cchar *pszDamageProfileHit;			// Damage profile string (references a damage profile in damage.csv)
		cchar *pszDamageProfileBurn;		// Damage profile string (references a damage profile in damage.csv)

		FMesh_t *pMeshEjectClip;			// Eject clip mesh

		f32 fSoundRadius;					// Distance sound from this weapon travels

		// Sound Effects
		FSndFx_FxHandle_t hFiringSound;		// The looping sound made when firing
		FSndFx_FxHandle_t hFadeOutSound;	// The sound made when done firing
		FSndFx_FxHandle_t hAttachClipSound;	// The sound made when the clip is attached
		FSndFx_FxHandle_t hSlapInSound;		// The sound made when the clip is slapped in
		FSndFx_FxHandle_t hEjectSound;		// The sound made when the clip is ejected
		FSndFx_FxHandle_t hEmptyClickSound;	// The sound made when the trigger is pulled while there is no ammo
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

	static _UserProps_t m_aUserProps[EUK_COUNT_FLAMER];
	static CDamageProfile *m_apDamageProfileHit[EUK_COUNT_FLAMER];
	static CDamageProfile *m_apDamageProfileBurn[EUK_COUNT_FLAMER];

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	_ResourceData_t m_aResourceData[EUK_COUNT_FLAMER];
	_ResourceData_t *m_pResourceData;
	_UserProps_t *m_pUserProps;

	CMeshEntity *m_pClipMeshEntity;					// The clip mesh entity (never NULL)

	CFlamer *m_pFlamer;								// Our flamer

	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
	f32 m_fTrigThisFrame;							// >0.0 when we should fire a round on the next call to ClassHierarchyWork()
	CFVec3A m_TargetPos_WS;							// target point
	const CFVec3A *m_apFirePos_WS[EUK_COUNT_FLAMER];// The fire position
	const CFVec3A *m_pBuddyFirePos_WS;				// The buddy's fire position

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponFlamer();
	virtual ~CWeaponFlamer();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS = NULL );


	// Rumble:
	virtual void KillRumble( void );


	// Item inst:
	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );

	virtual void CheckpointRestore( void );

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
	void _AddClipToWorld( void );
	void _RemoveClipFromWorld( void );


	FCLASS_STACKMEM_ALIGN( CWeaponFlamer );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponFlamerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponFlamerBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponFlamerBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponFlamerBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
