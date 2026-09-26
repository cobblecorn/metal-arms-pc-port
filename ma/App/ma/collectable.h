//////////////////////////////////////////////////////////////////////////////////////
// collectable.h -
//
// Author: Nathan Miller
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
// 02/24/03 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _COLLECTABLE_H_
#define _COLLECTABLE_H_ 1

#include "fang.h"
#include "fanim.h"
#include "fsound.h"
#include "fdebris.h"
#include "fmeshpool.h"
#include "flinklist.h"
#include "fparticle.h"
#include "fcheckpoint.h"

#include "entity.h"

class CCollectableBuilder;
class CCollectable;
class CPlayer;
class CWeapon;
class CBot;
class CBotGlitch;
class CHud2;
class CInventory;
class CItemInst;
struct FVisVolume_t;

typedef enum
{
	COLLECTABLE_UNKNOWN = 0,	// Unknown collectables are ones that are not well defined in code
	
	COLLECTABLE_WASHER,
	COLLECTABLE_CHIP,
	COLLECTABLE_PUP_HIJUMP,
	COLLECTABLE_PUP_SPEED,
	COLLECTABLE_PUP_ARMOR,
	COLLECTABLE_PUP_WEAPON,
	COLLECTABLE_PUP_ENERGY,

	COLLECTABLE_DET_PACK,
	COLLECTABLE_BATTERY,
	COLLECTABLE_MEGA_HEALTH,
	COLLECTABLE_SECRET_CHIP,

	COLLECTABLE_ARM_SERVO_L1,
	COLLECTABLE_ARM_SERVO_L2,
	COLLECTABLE_ARM_SERVO_L3,

	// All weapons should be placed between COLLECTABLE_WEAPON_START and COLLECTABLE_WEAPON_END
	// Don't mess around with the order of the EUKs, ClassifyPlayerWeapon() depends on the order
	COLLECTABLE_WEAPON_START = COLLECTABLE_ARM_SERVO_L3, // Don't use this one

	COLLECTABLE_WEAPON_LASER_L1,
	COLLECTABLE_WEAPON_LASER_L2,
	COLLECTABLE_WEAPON_LASER_L3,
	COLLECTABLE_WEAPON_RIPPER_L1,
	COLLECTABLE_WEAPON_RIPPER_L2,
	COLLECTABLE_WEAPON_RIPPER_L3,
	COLLECTABLE_WEAPON_RIVET_GUN_L1,
	COLLECTABLE_WEAPON_RIVET_GUN_L2,
	COLLECTABLE_WEAPON_RIVET_GUN_L3,
	COLLECTABLE_WEAPON_ROCKET_L1,
	COLLECTABLE_WEAPON_ROCKET_L2,
	COLLECTABLE_WEAPON_ROCKET_L3,
	COLLECTABLE_WEAPON_BLASTER_L1,
	COLLECTABLE_WEAPON_BLASTER_L2,
	COLLECTABLE_WEAPON_BLASTER_L3,
	COLLECTABLE_WEAPON_TETHER_L1,
	COLLECTABLE_WEAPON_TETHER_L2,
	COLLECTABLE_WEAPON_TETHER_L3,
	COLLECTABLE_WEAPON_SPEW_L1,
	COLLECTABLE_WEAPON_SPEW_L2,
	COLLECTABLE_WEAPON_SPEW_L3,
	COLLECTABLE_WEAPON_SCOPE_L1,
	COLLECTABLE_WEAPON_SCOPE_L2,
	COLLECTABLE_WEAPON_MORTAR_L1,
	COLLECTABLE_WEAPON_FLAMER_L1,
	COLLECTABLE_WEAPON_CORING_CHARGE,
	COLLECTABLE_WEAPON_EMP,
	COLLECTABLE_WEAPON_MAGMA_BOMB,
	COLLECTABLE_WEAPON_CLEANER,
	COLLECTABLE_WEAPON_WRENCH,
	COLLECTABLE_WEAPON_RECRUITER,

	COLLECTABLE_WEAPON_END, // Don't use this one

	COLLECTABLE_COUNT = COLLECTABLE_WEAPON_END,
} CollectableType_e;


typedef void CollectableCallback_t( CollectableType_e eType, CBot *pPickupBot );


FCLASS_ALIGN_PREFIX class CCollectableType {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	BOOL Setup( void );
	void Reset( void );

	CFWorldMesh *GetWorldMesh( BOOL *pbIsEUKMesh = NULL );
	void ReturnWorldMesh( CFWorldMesh *pMesh );
	void ForceReturnAllMeshes( void );

	static void RemapTCs( CFWorldMesh *pWorldMesh, CollectableType_e eType );

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static const FGameData_TableEntry_t m_aGameDataVocab[];
	static FMeshTexLayerHandle_t m_hEUKTexLayer;

	BOOL8 m_bSetup;
	BOOL8 m_bNeedSetup;
	BOOL8 m_bSpecialEUK;
	BOOL8 m_bShouldPoster;
	
	cchar *m_pszName;
	cchar *m_pszMeshName;

	cchar *m_pszParticleName;
	cchar *m_pszParticleRespawnName;
	cchar *m_pszEUKMeshName;
	cchar *m_pszMeshAnimName;

	f32 m_fHUDScale;
	f32 m_fHUDScaleSpecialEUK;

	s32 m_nAmmoCount; 
	CollectableType_e m_eType;
	u32 m_uItemRepositoryIndex;

	u32 m_uMeshPoolSize;
	CFMeshPool *m_pMeshPool;
	FMesh_t *m_pMesh;

	// This pool will be m_uMeshPoolSize in size.
	CFMeshPool *m_pEUKMeshPool;
	FMesh_t *m_pEUKMesh;

	CFSoundGroup *m_pPickupSound;
	CFSoundGroup *m_pPickupWeaponSound;
	CFSoundGroup *m_pRespawnSound;

	FParticle_DefHandle_t m_hParticleDef;
	FParticle_DefHandle_t m_hParticleRespawnDef;

	CFAnimInst *m_pAnimInst;
	CFAnimCombiner **m_paAnimCombiners;

	FLink_t m_Link;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	friend class CCollectable;

	void _ClearDataMembers( void );
	BOOL _SetupMeshPool( u32 uPoolSize, CFMeshPool **pMeshPool, FMesh_t **pMesh, cchar *pszMeshName, BOOL bUseAnimation = FALSE, BOOL bIsSpecialEUK = FALSE );
	BOOL _SetupAnimation( void );

	FCLASS_STACKMEM_ALIGN( CCollectableType );
} FCLASS_ALIGN_SUFFIX;


FCLASS_ALIGN_PREFIX class CCollectableSpawnPoint { 

public:
	f32 m_fScale;						// Scale to use
	f32 m_fSpawnTime;					// How much left until we spawn
	f32 m_fSpawnTimeTotal;				// How often to spawn
	CFVec3A m_SpawnPoint;				// Point to spawn at
	s32 m_nCurAmmoCount;				// Ammo to give out with it
	CollectableType_e m_eSpawnType;		// Type to spawn

	FLink_t m_Link;
	FCLASS_STACKMEM_ALIGN( CCollectableSpawnPoint );
} FCLASS_ALIGN_SUFFIX;




FCLASS_ALIGN_PREFIX class CCollectable : public CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

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
		STATE_STOPPED,
		STATE_FLOAT_UP,
		STATE_PLACED,
		STATE_BOUNCE,
	} CollectableState_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bSystemInitialized;
	static const FGameData_TableEntry_t m_aGameDataVocabOverride[];

	static FLinkRoot_t m_CollectablePoolDead;
	static FLinkRoot_t m_CollectablePoolAlive;
	static FLinkRoot_t m_CollectableTypeList;
	static FLinkRoot_t m_CollectableSpawnPoolAlive;
	static FLinkRoot_t m_CollectableSpawnPoolDead;

	static f32 m_fRotY;
	static f32 m_fTransY; // Is a value in radians

	static CFCollInfo m_CollInfo;
	static CFTrackerCollideProjSphereInfo CollProjSphereInfo;

	// Only valid when we pickup something
	// Static since they are only valid for the frame we pickup something, don't need one set per class
	static CPlayer *m_pPlayer;
	static CBot *m_pCollectBot;
	static CHud2 *m_pCollectHud;

	static CollectableCallback_t *m_pCollectionCallback;
	
	static FParticle_DefHandle_t m_hSpecialPickup;
	static CFSoundGroup *m_pSpecialGroup;

	static CFCheckPoint::ObjectDataHandle_t m_hSaveData[ FCHECKPOINT_MAX_CHECKPOINTS ];

	static CFDebrisGroup *m_pGlassGroup;

	// If m_bUsedInScript is TRUE we will never recycle the collectable and will assume it to always be in use
	// by the scripting system.  This means that in GetCollectable()
	// The collectable will be put into the dead list and treated like any other collectable.
	BOOL m_bUsedInScript;

	BOOL8 m_bInActiveList;
	BOOL8 m_bOverPoolAlloc;
	u8 m_uRotOffset;
	u8 m_uBobOffset;

	// Ammo count in this specific instance
	// If we point to a battery, this value holds the battery number if there is one
	s32 m_nCurAmmoCount;		

	f32 m_fSpawnTime;
	CollectableState_e m_eState;

	f32 m_fScale;
	CFWorldMesh *m_pWorldMesh;

	CFVec3A m_LastPos;
	CFVec3A m_PushPoint;
	CFVec3A m_VelocityWS;
	FVisVolume_t *m_pVolume;

	CCollectableType *m_pCollectableType;

	FParticle_EmitterHandle_t m_hParticleEmitter;

	CEntity *m_pSpawnIgnoreEntity;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	FLink_t m_Link;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL IsSystemInitialized() { return m_bSystemInitialized; }

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	
	static BOOL InitLevel( void );
	static void UnInitLevel( void );

	static BOOL LoadOverrideCSV( cchar *pszCSVName );

	static void CheckpointSaveGlobal( s32 nCheckpointID );
	static void CheckpointRestoreGlobal( s32 nCheckpointID );

	// bAllocateNewIfEmpty will allocate a new collectable if the pool is empty otherwise it will remove one from the pool
	// This should only be TRUE during world load.  It should NEVER be true after the world has been loaded and all entities have
	// been created.
	static CCollectable *GetCollectable( BOOL bAllocateNewIfEmpty = FALSE );
	static void ReturnCollectable( CCollectable *pCollectable );

	static CCollectableType *GetCollectableTypeObjectFromString( cchar *pszName );
	static CollectableType_e GetCollectableTypeFromString( cchar *pszName );
	static CCollectableType* GetCollectableTypeObjectFromType(CollectableType_e eType);

	// Definition of 'need'
	// Yes for health, if the player has less than full health
	//  No for health, if the player is maxed
	// Yes for weapon ammo, if the player has base instance of weapon, and room for the ammo
	//  No for weapon ammo, if the player has maximum ammo already
	// Yes for weapons the player doesn't have
	//  No for weapons the player does have
	// Yes for others until such time determination is made
	static BOOL PlayerNeeds(CBotGlitch* pPlayer, CollectableType_e eType, BOOL bRepresentAmmo);

	static CollectableType_e ClassifyBotWeapon( CWeapon *pWeap, u32 uEUKLevel = 0 );
	static CollectableType_e ClassifyPlayerWeapon( CWeapon *pWeap );

	// Call this to let the collectable system know that a collectable is used in the world
	// If bFromBot is TRUE and the passed type is a primary weapon, all EUKs for that weapon type will be loaded.
	// This is done since when a bot drops a weapon, it will drop the EUK Glitch has.
	static void NotifyCollectableUsedInWorld( cchar *pszName, BOOL bFromBot = FALSE );
	static void NotifyCollectableUsedInWorld( CollectableType_e eType, BOOL bFromBot = FALSE );

	static BOOL PostPlayerCreationSetup( void );
	static BOOL GiveToPlayer( CBot *pBot, CollectableType_e eType, f32 fScale );
	static BOOL GiveToPlayer( CBot *pBot, cchar *pszCollectableName, f32 fScale );
	static BOOL GiveWeaponToPlayer( CBot *pBot, cchar *pszWeaponName, s32 nAmmoCount = -1 );

	static void Work( void );
	static BOOL PlaceIntoWorld( CollectableType_e eType, const CFMtx43A *pMtx, const CFVec3A *pVelWS = NULL, f32 fScale = 1.0f, s32 nAmmoCount = -1, CEntity *pSpawnIgnoreEntity = NULL );
	static BOOL PlaceIntoWorld( cchar *pszCollectableName, const CFMtx43A *pMtx, const CFVec3A *pVelWS = NULL, f32 fScale = 1.0f, s32 nAmmoCount = -1, CEntity *pSpawnIgnoreEntity = NULL );

	// The collection callback will be set to NULL at level unload!
	static FINLINE BOOL IsCollectionCallbackRegistered( void )						{ return m_pCollectionCallback != NULL; }
	static FINLINE void SetCollectionCallback( CollectableCallback_t *pCallback )	{ m_pCollectionCallback = pCallback; }

	static FINLINE BOOL IsWeapon( CollectableType_e eType ) { return ( eType > COLLECTABLE_WEAPON_START ) && ( eType < COLLECTABLE_WEAPON_END ); }

	virtual ~CCollectable();

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	BOOL Create( void );

	CCollectableType *GetCollectableType() { return m_pCollectableType; }
	static BOOL TypeToHUDString(wchar* pDest, u32 uDestWcharCount,CCollectableType *pCollectType, BOOL bAddConcatText=FALSE, BOOL bSkipLevelText=FALSE);
	static BOOL IsPrimaryWeaponType( CCollectableType *pType );

	static void FixupInventoryWeapons( CInventory *pInv );

	void SetUsedInScript( BOOL bScript );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	FINLINE CCollectable() { } // NO ONE SHOULD CREATE A CCollectable OBJECT - THEY ALL COME FROM POOLS

	virtual void ClassHierarchyDestroy( void );
	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual void ClassHierarchyWork( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _LoadCollectableCSV( void );
	static void _LoadFromGameData( FGameDataTableHandle_t hTable );
	static void _LoadFromOverrideGameData( FGameDataTableHandle_t hTable );
	static CCollectableType *_FindCollectableType( cchar *pszName );
	static CCollectableType *_FindCollectableType( CollectableType_e eType );
	static CollectableType_e _ClassifyCollectableType( cchar *pszTypeName );
	
	static void _GiveWeaponToPlayer( CBot *pBot, CInventory *pInv, cchar *pszWeaponName, s32 uAmmoCount );
	static BOOL _GiveWeaponAmmoToPlayer( CBot *pBot, CItemInst *pItemInst, CInventory *pInv, cchar *pszWeaponName, s32 uAmmoCount );

	static BOOL _PlaceIntoWorld( CCollectableType *pType, const CFMtx43A *pMtx, const CFVec3A *pVelWS = NULL, f32 fScale = 1.0f, f32 fSpawnTime = 0.0f, s32 nAmmoCount = -1, CEntity *pSpawnIgnoreEntity = NULL );

	static void _PickupSpecialItem( CCollectableType *pCollectType );
	static void _ForceSpecialMeshesToNull( CCollectableType *pCollectType );

	static BOOL _IsWeapon( CCollectableType *pType );
	static u8 _GetEUKForWeapon( CCollectableType *pType );
	static u8 _GetEUKForWeapon( CollectableType_e eType );

	static void _ForceNotifyHigherEUK( CCollectableType *pType );
	static void _EUKMeshSwap( CCollectableType *pType, u32 uEUKCurrent );

	static BOOL _TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode );

	void _ClearDataMembers( void );
	BOOL _Setup( const CFMtx43A *pMtx, f32 fScale, f32 fSpawnTime = 0.0f, s32 nAmmoCount = -1 );
	void _ReturnWorldMesh( void );

	void _HandleCollision( void );
	void _CollisionResponse( FCollImpact_t *pNearImpact );

	BOOL _AddSpawnPoint( void );
	static void _RespawnItem( CCollectableSpawnPoint *pPoint );

	s32 _CanPickup( void );
	BOOL _IsPickupAllowed( void );
	BOOL _PlayerPickupCollectable( void );
	

	// _Pickup* methods below for all know pickup game types
	// _Pickup* methods should not remove from world or play collectable type specific sounds
	void _PickupWasher( void );
	BOOL _PickupEnergy( void );
	void _PickupHighJump( void );
	void _PickupSpeed( void );
	void _PickupArmor( void );
	void _PickupWeaponPower( void );
	BOOL _PickupBattery( void );
	BOOL _PickupMegaHealth( void );
	BOOL _PickupArmServo( CollectableType_e eType );
	void _AddToInventory( void );

	BOOL _CheckRemapItem( CCollectableBuilder *pBuilder );
	CCollectableType *_RemapItem( void );
	
	void _ActiveListGetResources( void );
	void _NonActiveListReleaseResources( void );

	void _SpawnGlass( void );
#if 0
	static BOOL _RaceToTheRocketSetup( void );
#endif

	friend class CCollectableBuilder;

	FCLASS_STACKMEM_ALIGN( CCollectable );
} FCLASS_ALIGN_SUFFIX;







FCLASS_ALIGN_PREFIX class CCollectableBuilder : public CEntityBuilder
{
public:
	FINLINE CCollectableBuilder() {}

	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );

public:
	enum {
		GAMETYPE_FLAG_LIMITED_WEAPONS = 0x01,
	};

protected:
	virtual BOOL InterpretTable();

public:
	f32 m_fScale;
	f32 m_fSpawnTime;
	s32 m_nAmmo;
	u32 m_uGameFlags;
	u32 m_uBatteryNumber;

	CCollectableType *m_pCollectableType;

	FCLASS_STACKMEM_ALIGN( CCollectableBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif
