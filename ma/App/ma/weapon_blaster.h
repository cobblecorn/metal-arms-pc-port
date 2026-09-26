//////////////////////////////////////////////////////////////////////////////////////
// weapon_blaster.h - Blaster weapon.
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

#ifndef _WEAPON_BLASTER_H_
#define _WEAPON_BLASTER_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fworld.h"
#include "fmesh.h"
#include "fforce.h"
#include "smoketrail.h"
#include "tracer.h"
#include "muzzleflash.h"
#include "fdecal.h"


class CDamageProfile;
class CFSoundGroup;
class CEParticle;





FCLASS_ALIGN_PREFIX class CWeaponBlaster : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum
	{
		ABSOLUTE_MAX_PELLET_COUNT = 16
	};


	typedef enum {
		_MESH_WEAPON,						//mesh of the weapon  (can be different for each euk level)
		_MESH_SHELL,						//mesh of the shells kicked out by the blaster (can be different for each euk level)
		_MESH_COUNT
	} _Mesh_e;


	typedef enum {
		_BONE_PRIMARY_FIRE,					// Where the hurt comes out of
		_BONE_CARTRIDGE,					// Cartridge where shells are mounted on
		_BONE_BARREL,						// Barrel of the gun

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
		cchar *apszMeshName[_MESH_COUNT];	// Mesh names used for the blaster
		cchar *apszBoneName[_BONE_COUNT];	// Bone names used for the blaster
		f32 fClipAmmoMax;					// Maximum number of rounds in the clip
		f32 fReserveAmmoMax;				// Maximum number of rounds in reserve
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec
		u32 nPelletCountPerRound;			// Number of pellets per round
		f32 fShotSpreadFactor;				// Used to scatter the pellets

		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera
		f32 fShellCullDist;					// Don't draw the shells if it's this far from the camera
		f32 fMaxLiveRange;					// The maximum distance the pellets can travel before self-destructing
		f32 fUnitRecoil;					// amount of recoil this weapon generates
		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)
			
		f32 fCartridgeXRange;				// how much the cartridge from max clip to empty clip
		f32 fCartridgeXPos;					// initial position of the cartridge
		f32 fShellXPos;						// initial position of shell 0 (last to be fired)
		f32 fShellScale;					// scale of teh shell model
		f32 fDoubleStacked;					// are shells double stacked on the cartridge?
		f32 fShellStackYOffset;				// y offset of the bottom row of shells
		f32 fReloadTimeForOneRound;			// time to re-load one shell (two if dbl barrel)
		f32 fBarrel2DelayTime;				// how long after first barrel fires, does second fire. (seconds)
		cchar* pszTracerTexName;			// name of the texture used for the the texture on the lines that shoot out of the barrel apon firing
		cchar* pszMuzzleFlashTexName;		// name of the texture used for the muzzle flash

		f32 fMuzzleWidth_Z;					// Width of the muzzle flash Z
		f32 fMuzzleHeight_Z;				// Height of the muzzle flash Z
		f32 fMuzzleAlpha_Z;					// Alpha of muzzle flash Z

		f32 fMuzzleScale_Impact;			// Scale of impact flash
		f32 fMuzzleAlpha_Impact;			// Alpha of impact flash
		f32 fMuzzleOffset_Impact;			// Offset of impact flash

		f32 fShowShellsOnCartridge;			// Wether to Show any shells on the cartridge.
		f32 fUnitCamVibration;				// Amount of camera vibration when firing

		FParticle_DefHandle_t hParticleSmoke;	// Muzzle smoke particle
		f32 fSmokeUnitIntensity;				// Muzzle smoke intensity
		f32 fSmokeCoolDownSpeed;				// Muzzle smoke cool-down speed

		CDamageProfile *pDamageProfile;		// Damage profile for each pellet

		// Sound Effects
		CFSoundGroup *pSoundGroupFire;		// Fire sound
		CFSoundGroup *pSoundGroupEmpty;		// Empty sound
		CFSoundGroup *pSoundGroupCock;		// Cock sound
		CFSoundGroup *pSoundGroupRound;

		FDecalDefHandle_t hDecalDef;		// decal def
	} _UserProps_t;


	typedef struct {
		CFWorldMesh *m_pWorldMesh;						// This EUK's world mesh
		CFAnimCombiner *m_pAnimCombiner;				// This EUK's animation combiner
		s32 m_nCombinerTapID;							// The combiner's tap ID
		CFWorldMesh *m_paShellWorldMeshArray;			// Shells that are in the cartridge and ready to fire (for visuals only) (NULL=none)
		CFAnimMeshRest *m_pAnimInstRest;				// Special thing needed for animcombiners when there is no animation data
	} _ResourceData_t;


	typedef struct {
		CFMtx43A* paCartridgeSlotMtxArray;
		s8 nPrimFireBoneIndex;
		s8 nCartridgeBoneIndex;
		s8 nBarrelBoneIndex;
		f32 fOOReloadTimeForOneRound;
		f32 fShellSlotDist;
		BOOL bDoubleStacked;				// are shells double stacked on the cartridge?
		BOOL bShowShellsOnCartridge;		// Wether to Show any shells on the cartridge.											
	} _SystemData_t;


	typedef enum
	{
		BLASTERSTATE_NORMAL = 0x0000,
		BLASTERSTATE_RELOADING = 0x0001,
		BLASTERSTATE_COUNT
	} BlasterState_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t m_aUserPropMapTable[];

	static _UserProps_t m_aUserProps[EUK_COUNT_BLASTER];
	static CDamageProfile *m_apDamageProfile[EUK_COUNT_BLASTER];
	static _SystemData_t m_aSystemData[EUK_COUNT_BLASTER];
	static CWeaponBlaster *m_pCallbackThis;

//	static SmokeTrailAttrib_t m_SmokeTrailAttrib;

	_ResourceData_t m_aResourceData[EUK_COUNT_BLASTER];
	_ResourceData_t *m_pResourceData;
	_UserProps_t *m_pUserProps;

	f32 m_fReloadCockTimerSecs;						// Used to measure time before cock.
	f32 m_fSecondsCountdownTimer;					// Used to measure time between rounds, reloads, etc.
	f32 m_fAnimInstTimeMul;							// The time scale of the currently playing animation
	CFAnimInst *m_pAnimInstCurrentlyPlaying;		// The CFAnimInst in _ResourceData_t::m_apAnimInstArray that is currently playing (NULL=none)
	f32	m_fFireAgainTime;

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to

	CEParticle *m_pParticleSmoke;					// Muzzle smoke
	f32 m_fSmokeUnitIntensity;						// Current muzzle smoke particle intensity

	BOOL m_bReloadRequested;
	f32 m_fCartridgeOffsetStart;
	f32 m_fCartridgeOffsetEnd;
	f32 m_fCartridgeOffset;
	BlasterState_e m_nBlasterState;

	f32 m_fUnitTriggerVal1;
	BOOL m_bFireThisFrame;
	BOOL m_bBoolBarrel2Fire;
	CFVec3A m_vTargetPos_WS;
	const CFVec3A *m_pvFireBuddyPos_WS;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static u32 GetAvailableLevelCount( void );
#if FANG_WINGC
	static BOOL _ReadRetailProperties( void );
#endif
	static void UninitSystem( void );

	CWeaponBlaster();
	virtual ~CWeaponBlaster();


	// Creation:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );


	// Collision:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);


	// Firing:
	virtual void ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const;
	virtual u32 TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS = NULL );


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
	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );
	virtual void ClassHierarchyResetToState( void );
	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );
	virtual void BeginReload( void );


	void _ScatterBlast(const CFVec3A& BlastOrigin, const CFVec3A& BlastDirUnit, u32 uNumPellets, f32 fJitterFactor);




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	void _SetAnimToCurrentFinalState( void );
	void _ComputeMtxPalette( void );
	static void _SetSmokeTrailAttributes( void );

	u32 _L1_TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS );
	u32 _L23_TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS );

	void _L1_Fire( const CFVec3A &vUnitFireDir );
	void _L23_Fire( const CFVec3A &vUnitFireDir );

	void _UpdateSmoke( void );
	void _KillSmoke( void );
	void _DrawMuzzleEffects( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bBuddy );

	static void _ComputeCartridgeShellMatrices(void);
	static void _BarrelBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	static void _CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );


	FCLASS_STACKMEM_ALIGN( CWeaponBlaster );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponBlasterBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponBlasterBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponBlasterBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponBlasterBuilder );
} FCLASS_ALIGN_SUFFIX;








#endif

