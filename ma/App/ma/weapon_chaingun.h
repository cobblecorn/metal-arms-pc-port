//////////////////////////////////////////////////////////////////////////////////////
// weapon_chaingun.h - Titan's chain gun
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
// 10/29/02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_CHAINGUN_H_
#define _WEAPON_CHAINGUN_H_ 1

#include "fang.h"
#include "weapon.h"
#include "fforce.h"
#include "eparticle.h"
#include "fdecal.h"

class CBot;
class CDamageProfile;
class CFAnimCombiner;
class CFAnimMeshRest;



FCLASS_ALIGN_PREFIX class CWeaponChaingun : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:


	typedef enum {
		_BONE_PRIMARY_FIRE,					// Where the hurt comes out of
		_BONE_SPIN_BARRELS,					// The barrels to spin
		_BONE_SPIN_CARTRIDGE,				// The cartridge to spin

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

	typedef enum {
		_STATE_OFF,							// shut off
		_STATE_IDLE,						// just spinning slowly
		_STATE_SPINUP,						// getting ready to fire
		_STATE_FIRING,						// at max speed, firing away
		_STATE_SPINDOWN,					// spinning back down to idle
		_STATE_OVERHEAT_FORCED_COOLDOWN,	// overheated.  Barrels stopped while gun slowly cools
	} _ChaingunState_e;


	typedef struct {						// NOTE: If you change this structure, you must also change _Anim_e and m_aUserPropVocab
		cchar *pszMeshName;				// Mesh names
		cchar *apszBoneName[_BONE_COUNT];	// Bone names

		f32 fMaxAmmo;						// Maximum number of clip rounds (<0 means infinite)
		f32 fRoundsPerSec;					// Maximum fire rate
		f32 fOORoundsPerSec;				// 1.0f / fRoundsPerSec

		f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera

		f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing

		f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)

		f32 fUnitCamVibration;				// Amount camera vibrates when firing

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

		f32 fBarrelsMaxRevsPerSec;			// The maximum revolutions per second of the barrels
		f32 fBarrelsOOMaxRevsPerSec;		// 1.0f / fBarrelsMaxRevsPerSec
		f32 fSpinupTime;					// How long it takes the barrels to spin up
		f32 fOOSpinupTime;					// 1.0f / fSpinupTime
		f32 fSpindownTime;					// How long it takes the barrels to spin down
		f32 fOOSpindownTime;				// 1.0f / fSpindownTime
		cchar *pszDamageProfile;			// new damage profile

		f32	fMuzzleLightRadius;				// radius of muzzle flash light
		f32 fCartridgeRPS;					// rev/sec of the cartridge
		f32 fBarrelIdleRPS;					// barrel idle rev/sec

		f32	fHeatUpRate;					// amount to heat / sec while firing
		f32	fCoolDownRate;					// amount to cool / sec while not firing
		f32 fOverheatPenalty;				// multiplier to fCoolDownRate if the gun overheats
		f32 fCoolDownThreshold;				// temp at which the gun starts operating again
		
		cchar *pszSmokeParticleName;		// the heat smoke particle to use
		cchar *pszShellMeshName;			// the shell mesh
		u32 uBarrelHeatTexID;				// the ID of our overheat tex layer (side)
		u32 uBarrelFrontHeatTexID;			// the ID of our overheat tex layer (front)

		f32 fSoundRadius;					// Distance sound from this weapon travels
		

		// Sound Effects
		FSndFx_FxHandle_t hSndFire;
		FSndFx_FxHandle_t hSndSpin;
		FSndFx_FxHandle_t hSndSpinup;
		FSndFx_FxHandle_t hSndSpindown;
		FSndFx_FxHandle_t hSndOverheat;

		FDecalDefHandle_t hDecalDef;

	} _UserProps_t;


	typedef struct {
		CFWorldMesh		*m_pWorldMesh;		// This EUK's world mesh
		CFAnimCombiner	*m_pAnimCombiner;	// This EUK's animation combiner
		CFAnimMeshRest	*m_pAnimRest;		// Rest Animation
	} _ResourceData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL	m_bSystemInitialized;
	static u32	m_uWpnClassClientCount;					// Number of guns of this class using the class-wide resources

	static _UserProps_t		m_aUserProps[EUK_COUNT_CHAINGUN];
	static CWeaponChaingun *m_pCallbackThis;
	

	static const FGameData_TableEntry_t m_aUserPropVocab[];
	static const FGameDataMap_t			m_aUserPropMapTable[];

	_ResourceData_t  m_aResourceData[EUK_COUNT_CHAINGUN];
	_ResourceData_t *m_pResourceData;
	_UserProps_t	*m_pUserProps;

	static CDamageProfile*	m_apDamageProfile[EUK_COUNT_CHAINGUN];

	FForceHandle_t m_hForce;						// Force feedback handle so we can kill it when we need to
	CFAudioEmitter *m_pSndEmitter;					// pointer to our looping spin sound

	
	// our bone ids
	u32 m_auPriFireBone[EUK_COUNT_CHAINGUN];
	u32	m_auSpinBarrelBone[EUK_COUNT_CHAINGUN];
	u32 m_auSpinCartridgeBones[EUK_COUNT_CHAINGUN];


	_ChaingunState_e m_eState;						// our current state

	BOOL	m_bTriggerDown;
	f32		m_fBarrelSpeed;							// our current barrel rot speed, when it hits max, we start firing
	f32		m_fFireTimer;							// countdown timer for firing
	f32		m_fSoundFireTimer;						// countdown timer for when to play next fire sound
	f32		m_fDebrisTimer;							// countdown for creating debris
	CFVec3A	m_FireUnitDir;							// direction we're firing
	f32		m_fUnitShotSpread;						// 0.0f - 1.0f, how much our shot is spreading
	f32		m_fBarrelRotation;						// our current barrel rotation in rads
	f32		m_fCartridgeRotation;					// our current cartridge rotation in rads
	f32		m_fSpinDirectionModifier;				// makes the stuff spin backwards if necessary
	f32		m_fUnitHeat;							// heat.  stop firing at 1.0f

	f32		m_fHeatupPerSecond;
	f32		m_fCooldownPerSecond;
	f32		m_fCooldownOverheatPenalty;



	FMeshTexLayerHandle_t		m_hHeatTexLayerBarrel;		// this is for the side of the barrel
	FMeshTexLayerHandle_t		m_hHeatTexLayerFront;		// this is for the front of the barrel

	CEParticle				   *m_pSmokeParticle;
	FParticle_DefHandle_t		m_hSmokeParticleDef;		// handle to smoke particle def

//	static DebrisInitSettings_t		m_DebrisInitSettings;				// mostly the same for all the debris
	static FMesh_t				   *m_apShellMesh[EUK_COUNT_CHAINGUN];	// the meshes

	const CFVec3A	*m_pSoundMuzzlePosOverride;
	const CFVec3A	*m_pLightMuzzlePosOverride;
	BOOL			 m_bPlaySounds;
	BOOL			 m_bDrawMuzzleLights;
	BOOL			 m_bOverrideSoundPos;
	BOOL			 m_bOverrideMuzzleLightPos;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	CWeaponChaingun();
	virtual ~CWeaponChaingun();


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
//	virtual void SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo=FALSE );

	// checkpoint save/restore
//	virtual void StateSaveSelect( s32 nCheckpoint );

	//chaingun only fns
	void SetRotation( BOOL clockwise ) { m_fSpinDirectionModifier = clockwise ? 1.0f : -1.0f; };

	FINLINE f32 GetSpinupTime( void )				{ return m_pUserProps->fSpinupTime; };
	FINLINE f32 GetSpindownTime( void )				{ return m_pUserProps->fSpindownTime; };
	FINLINE f32 GetHeatupPerSecond( void )			{ return m_fHeatupPerSecond; };
	FINLINE f32 GetCooldownPerSecond( void )		{ return m_fCooldownPerSecond; };
	FINLINE f32 GetCooldownOverheatPenalty( void )	{ return m_fCooldownOverheatPenalty; };

	FINLINE void SetHeatupPerSecond( f32 fVal )			{ m_fHeatupPerSecond = fVal; };
	FINLINE void SetCooldownPerSecond( f32 fVal )		{ m_fCooldownPerSecond = fVal; };
	FINLINE void SetCooldownOverheatPenalty( f32 fVal )	{ m_fCooldownOverheatPenalty = fVal; };

	FINLINE f32	 GetUnitHeat( void )					{ return m_fUnitHeat; };


	// These two functions allow for optimizations for the titan, who has two guns.  Instead of each gun playing
	// sounds and creating muzzle lights, one gun will handle these effects, and they will happen at the center of the two guns

	void OverrideSounds( BOOL bPlaySound, BOOL bOverridePos=FALSE, const CFVec3A *pSoundPosition=NULL );
	void OverrideLights( BOOL bShowLight, BOOL bOverridePos=FALSE, const CFVec3A *pPosition=NULL );

	void RestoreDefaultValues( void );

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); if( m_pResourceData ) return m_pResourceData->m_pWorldMesh; else return NULL; }


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
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );

	virtual void ClassHierarchyResetToState( void );
	
	
	virtual void BeginReload( void ) {};											// no reloading
	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );
	virtual void NotifyAmmoMightHaveChanged( void )	{};								// no changing ammo


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	void _TryToFire( void );									// fire a round if we can
	void _PlaySoundForNewState( _ChaingunState_e eNewState );
	void _EjectShell( void );
	static void _SpinBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );


	FCLASS_STACKMEM_ALIGN( CWeaponChaingun );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponSpewBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponChaingunBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponChaingunBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponChaingunBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
