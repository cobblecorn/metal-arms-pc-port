//////////////////////////////////////////////////////////////////////////////////////
// FDebris.h - Debris system.
//
// Author: Steve Ranck   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 12/04/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FDEBRIS_H_
#define _FDEBRIS_H_ 1

#include "fang.h"
#include "fmath.h" 
#include "smoketrail.h"
#include "fparticle.h"



class CFDebris;
class CFMeshPool;
class CFWorldLight;
class CFWorldMesh;
class CFSoundGroup;
struct FMesh_t;
struct FCollImpact_t;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisDef
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFDebrisDef {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		FLAG_FLAT_OBJECT				= 0x01,		// Object is wafer-thin and lies on its XZ plane with origin at y=0
		FLAG_ROTATIONAL_MOTION			= 0x02,		// Implement rotational motion for the object
		FLAG_USE_PROVIDED_WORLDMESH		= 0x04,		// 0=use provided FMesh_t *,  1=use provided CFWorldMesh *
		FLAG_RANDOM_ORIENTATION			= 0x08,		// Start with a random orientation
		FLAG_BUILD_TRACKER_SKIP_LIST	= 0x10,		// If set, the callback function will be called for the user to build a tracker skip list
		FLAG_PLAY_IMPACT_SOUND			= 0x20,		// If the debris has an impact sound associated with it, this flag allows it to be played
		FLAG_PLAY_FLAMING_SOUND			= 0x40,		// If the debris has a flaming sound associated with it, this flag allows it to be played
		FLAG_OVERRIDE_MESH_TINT			= 0x80,		// Override the mesh tint that's specified in the mesh set

		FLAG_NONE						= 0
	};


	enum CollType_e {
		COLL_TYPE_NONE,								// No collision will be performed
		COLL_TYPE_RAY,								// Cheap ray collision will be performed

		COLL_TYPE_COUNT
	};


	enum FadeType_e {
		FADE_TYPE_ALPHA,							// Fade out the opacity of the object
		FADE_TYPE_SHRINK,							// Shrink down the object
		FADE_TYPE_POP_OFF,							// Just pop off the object (m_fFadeSecs will not be used)

		FADE_TYPE_COUNT
	};


	typedef enum {
		LIGHTING_TYPE_NONE,							// Perform no lighting on this object
		LIGHTING_TYPE_AMBIENT_ONLY,					// Only ambient lighting will be applied to the object
		LIGHTING_TYPE_FULL_DYNAMIC,					// Full dynamic lighting will be applied to the object

		LIGHTING_TYPE_COUNT
	} LightingType_e;


	enum CallbackReason_e {
		CALLBACK_REASON_COLLISION,					// The debris particle has collided
		CALLBACK_REASON_START_FADE,					// The debris particle has begun to fade (never used if FADE_TYPE_POP_OFF is set)
		CALLBACK_REASON_DEAD,						// The debris particle is dead
		CALLBACK_REASON_BUILD_TRACKER_SKIP_LIST,	// Set FWorld_nTrackerSkipListCount and FWorld_apTrackerSkipList appropriately (FWorld_nTrackerSkipListCount defaults to 0)


		CALLBACK_REASON_COUNT
	};


	typedef void Callback_t( CFDebris *pDebris, CallbackReason_e nReason, const FCollImpact_t *pCollImpact );




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static CFDebrisDef m_DefaultDebrisDef;


	Callback_t *m_pFcnCallback;			// Callback function
	void *m_pUser;						// User field
	u16 m_nUser;						// User field
	u8 m_nOwnerID;						// Used to uniquely identify the owner of the debris so that a selective kill may be done (0=not identified)

	u8 m_nPriority;						// 0=lowest, 2=highest
	s8 m_nBoneIndex;					// If >=0, this is the bone to drive in m_pWorldMesh (FLAG_USE_PROVIDED_WORLDMESH must also be set)

	u8 m_nCollType;						// Type of collision to perform
	u8 m_nFadeType;						// Type of fade effect to use
	u8 m_nLightingType;					// Type of lighting to use

	CFVec3A m_Pos_WS;					// Position in world space
	CFVec3A m_LinVel_WS;				// Linear velocity in world space
	CFVec3A m_UnitRotAxis_WS;			// Unit axis of rotation
	f32 m_fRotSpeed;					// Rotational speed

	u8 m_nFlags;						// See FLAG_* for info
	u8 m_nTintRed;						// Red tinting
	u8 m_nTintGreen;					// Green tinting
	u8 m_nTintBlue;						// Blue tinting

	FMesh_t *m_pMesh;					// The mesh to use when FLAG_USE_PROVIDED_WORLDMESH is 0
	CFWorldMesh *m_pWorldMesh;			// The world mesh to use when FLAG_USE_PROVIDED_WORLDMESH is 1

	f32 m_fMeshScale;					// The scale to apply to the mesh
	f32 m_fMeshUnitOpacity;				// The unit opacity to apply to the mesh (1=fully opaque)
	f32 m_fGravity;						// The gravity for the particle

	f32 m_fMinSpeedToStartFade2;		// When the linear-speed-squared falls below this value (and is colliding), the fade effect will be started
	f32 m_fSecsBeforeMinSpeedValid;		// Seconds the particle must be alive before m_fMinSpeedToStartFade2 is examined
	f32 m_fSecsUnderMinSpeedToStartFade;// Linear speed must be under m_fMinSpeedToStartFade for this many seconds

	f32 m_fAliveSecs;					// Maximum amount of time until the fade effect begins
	f32 m_fFadeSecs;					// Amount of time the fade effect will last
	f32 m_fCullDist;					// Cull distance

	f32 m_fUnitDustKickUp;				// How much dust may be kicked up by an impact of this debris particle (0=none)
	f32 m_fUnitHitpointDamage;			// How much damage might be dealt by a collision with this debris particle (0=none)

	CFSoundGroup *m_pSoundGroupImpact;	// Sound group used for when debris impacts geo
	CFSoundGroup *m_pSoundGroupFlaming;	// Sound group used for flaming debris

	SmokeTrailAttrib_t *m_pSmokeTrailAttrib;	// Smoke trail attribute (NULL if a smoke trail is not wanted)
	FParticle_DefHandle_t m_hParticleDef1;		// Particle definition handle 1 (FPARTICLE_INVALID_HANDLE if a particle effect is not wanted)
	FParticle_DefHandle_t m_hParticleDef2;		// Particle definition handle 2 (FPARTICLE_INVALID_HANDLE if a particle effect is not wanted)




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bModuleStartedUp;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleStartedUp( void ) { return m_bModuleStartedUp; }

	FINLINE void InitToDefaults( void ) { fang_MemCopy( this, &m_DefaultDebrisDef, sizeof(CFDebrisDef) ); }



	FCLASS_STACKMEM_ALIGN( CFDebrisDef );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisMeshUncompressed
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CFDebrisMeshUncompressed {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FMesh_t *m_pMesh;					// The mesh (NULL will terminate the list)

	f32 m_fScale;						// Global scale to apply to the mesh
	BOOL m_bFlatObject;					// TRUE=This mesh is a flat object
	BOOL m_bCanKickUpDust;				// TRUE=This mesh can kick up dust
	BOOL m_bCanDamage;					// TRUE=This mesh can do some damage
	BOOL m_bOverrideableTint;			// TRUE=This mesh's tint can be overridden
	f32 m_fTintUnitRed;					// Red unit tinting
	f32 m_fTintUnitGreen;				// Green unit tinting
	f32 m_fTintUnitBlue;				// Blue unit tinting


	FCLASS_STACKMEM_NOALIGN( CFDebrisMeshUncompressed );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisMesh
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CFDebrisMesh {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		FLAG_FLAT_OBJECT			= 0x01,		// TRUE=This mesh is a flat object
		FLAG_CAN_KICKUP_DUST		= 0x02,		// TRUE=This mesh can kick up dust
		FLAG_CAN_DAMAGE				= 0x04,		// TRUE=This mesh can do some damage
		FLAG_OVERRIDEABLE_TINT		= 0x08,		// TRUE=This mesh's tint can be overridden
		FLAG_TINT_IS_WHITE			= 0x10,		// TRUE=This mesh's tint color is white

		FLAG_NONE					= 0
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FMesh_t *m_pMesh;					// The mesh (NULL will terminate the list)
	f32 m_fScale;						// Global scale to apply to the mesh

	u8 m_nFlags;						// See FLAG_* for info
	u8 m_nTintRed;						// Tint: red component
	u8 m_nTintGreen;					// Tint: green component
	u8 m_nTintBlue;						// Tint: blue component


	FCLASS_STACKMEM_NOALIGN( CFDebrisMesh );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisMeshSet
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFDebrisMeshSet {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	cchar *m_pszMeshSetName;								// The name of this mesh set
	const CFDebrisMesh *m_pDebrisMeshArray;					// Array of compressed debris meshes
	u32 m_nDebrisMeshCount;									// Number of elements in m_pDebrisMeshArray




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bModuleStartedUp;							// TRUE once ModuleStartup() has been called
	static FLinkRoot_t m_LinkRoot;							// Linklist of all loaded mesh sets
	static const FGameData_TableEntry_t m_aGameDataVocab[];	// Vocabulary for parsing CSV data

	FLink_t m_Link;											// Link to other mesh sets




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleStartedUp( void ) { return m_bModuleStartedUp; }

	static CFDebrisMeshSet *Find( cchar *pszMeshSetName );
	static u32 CountDebrisMesh( const CFDebrisMesh *pDebrisMeshArray );

	static CFDebrisMeshSet *Build( cchar *pszMeshSetName, const CFDebrisMeshUncompressed *pUncompressedDebrisMeshArray, u32 nDebrisMeshCount );
	static CFDebrisMeshSet *LoadFromGameData( FGameDataTableHandle_t hTable );
	static CFDebrisMeshSet *LoadFromGameData( cchar *pszFileName, cchar *pszMeshSetName );
	static BOOL LoadAllFromGameData( cchar *pszFileName );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _ResDestroyedCallback( void *pResMem );


	FCLASS_STACKMEM_ALIGN( CFDebrisMeshSet );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisGroup
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFDebrisGroup {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef struct {
		cchar *pszMeshSet;
		CFSoundGroup *pSoundGroupImpact;	// Sound group used for when debris impacts geo
		CFSoundGroup *pSoundGroupFlaming;	// Sound group used for flaming debris

		u32 nPriority;						// From 0 to 2 (2 is the highest priority)

		BOOL bIgnoreFlatObjectFlag;
		BOOL bRotationalMotion;
#if !FANG_WINGC
		BOOL bRandomOrientation;			// Not in the retail GameCube data
#endif

		f32 fUnitDustKickUp;
		f32 fUnitHitpointDamage;

		cchar *pszCollType;					// "None" or "Ray"
		cchar *pszFadeType;					// "Shrink", "Alpha", or "Pop"
		cchar *pszLightingType;				// "None", "Full", or "Ambient"

		f32 fMinSpeedToStartFade2;
		f32 fSecsBeforeMinSpeedValid;
		f32 fSecsUnderMinSpeedToStartFade;

		f32 fMinAliveSecs;
		f32 fMaxAliveSecs;
		f32 fFadeSecs;
		f32 fCullDist;

		f32 fMinScale;
		f32 fMaxScale;

		f32 fMinGravity;
		f32 fMaxGravity;

		f32 fMinRotSpeed;
		f32 fMaxRotSpeed;
	} _UserProps_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static CFDebrisDef m_DebrisDef;					// Built from the non-static members below


	cchar *m_pszDebrisGroupName;					// Name of this debris group

	const CFDebrisMesh *m_pDebrisMeshArray;			// Array of debris meshes to use with this group
	u32 m_nDebrisMeshCount;							// Number of elements in m_pDebrisMeshArray

	CFSoundGroup *m_pSoundGroupImpact;				// Sound group used for when debris impacts geo
	CFSoundGroup *m_pSoundGroupFlaming;				// Sound group used for flaming debris

	u8 m_nPriority;									// 0=lowest, 2=highest
	BOOL8 m_bIgnoreFlatObjectFlag;					// TRUE=ignore the flat-object flag in the mesh set
	BOOL8 m_bRotationalMotion;						// Compute rotational motion
	BOOL8 m_bRandomOrientation;						// Pick a random initial orientation

	f32 m_fUnitDustKickUp;							// See CFDebrisDef::m_fUnitDustKickUp for info
	f32 m_fUnitHitpointDamage;						// See CFDebrisDef::m_fUnitHitpointDamage for info

	CFDebrisDef::CollType_e m_nCollType;			// Collision type
	CFDebrisDef::FadeType_e m_nFadeType;			// Fade type
	CFDebrisDef::LightingType_e m_nLightingType;	// Lighting type

	f32 m_fMinSpeedToStartFade2;					// See CFDebrisDef::m_fMinSpeedToStartFade2 for info
	f32 m_fSecsBeforeMinSpeedValid;					// See CFDebrisDef::m_fSecsBeforeMinSpeedValid for info
	f32 m_fSecsUnderMinSpeedToStartFade;			// See CFDebrisDef::m_fSecsUnderMinSpeedToStartFade for info

	f32 m_fMinAliveSecs;							// Minimum alive seconds (See CFDebrisDef::m_fAliveSecs for info)
	f32 m_fMaxAliveSecs;							// Maximum alive seconds (See CFDebrisDef::m_fAliveSecs for info)
	f32 m_fFadeSecs;								// See CFDebrisDef::m_fFadeSecs for info
	f32 m_fCullDist;								// See CFDebrisDef::m_fCullDist for info

	f32 m_fMinScale;								// Minimum scale to apply
	f32 m_fMaxScale;								// Maximum scale to apply

	f32 m_fMinGravity;								// Minimum gravity to apply
	f32 m_fMaxGravity;								// Maximum gravity to apply

	f32 m_fMinRotSpeed;								// Minimum rotational speed (used only if m_bRotationalMotion is TRUE)
	f32 m_fMaxRotSpeed;								// Maximum rotational speed (used only if m_bRotationalMotion is TRUE)




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bModuleStartedUp;							// TRUE once ModuleStartup() has been called
	static FLinkRoot_t m_LinkRoot;							// Linklist of all loaded debris groups
	static const FGameData_TableEntry_t m_aGameDataVocab[];	// Vocabulary for parsing CSV data into _UserProps_t

	FLink_t m_Link;											// Link to other debris groups




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleStartedUp( void ) { return m_bModuleStartedUp; }

	static CFDebrisGroup *Find( cchar *pszDebrisGroupName );

	static CFDebrisGroup *AllocEmpty( cchar *pszDebrisGroupName );
	static CFDebrisGroup *LoadFromGameData( FGameDataTableHandle_t hTable, cchar *pszMeshSetFileName );
	static CFDebrisGroup *LoadFromGameData( cchar *pszFileName, cchar *pszDebrisGroupName, cchar *pszMeshSetFileName );
	static BOOL LoadAllFromGameData( cchar *pszFileName, cchar *pszMeshSetFileName );

	u32 GetDebrisMeshCount( void ) const { return m_nDebrisMeshCount; }
	const CFDebrisMesh *GetDebrisMesh( u32 nDebrisMeshIndex ) const { FASSERT( nDebrisMeshIndex < m_nDebrisMeshCount ); return &m_pDebrisMeshArray[nDebrisMeshIndex]; }





//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _ResDestroyedCallback( void *pResMem );


	FCLASS_STACKMEM_ALIGN( CFDebrisGroup );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisSpawner
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFDebrisSpawner {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		FLAG_BUILD_TRACKER_SKIP_LIST	= 0x01,	// If set, the callback function will be called for the user to build a tracker skip list
		FLAG_PLAY_IMPACT_SOUND			= 0x02,	// If the debris has an impact sound associated with it, this flag allows it to be played
		FLAG_PLAY_FLAMING_SOUND			= 0x04,	// If the debris has a flaming sound associated with it, this flag allows it to be played
		FLAG_OVERRIDE_MESH_TINT			= 0x08,	// Override the mesh tint that's specified in the mesh set

		FLAG_NONE						= 0x00
	};


	enum EmitterType_e {
		EMITTER_TYPE_POINT,					// Emitter is a point
		EMITTER_TYPE_BOUNDED_PLANE,			// Emitter is a bounded plane

		EMITTER_TYPE_COUNT
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum ListMember_e {
		LIST_MEMBER_FREE,					// Spawner is part of the free list
		LIST_MEMBER_ACTIVE,					// Spawner is part of the active list
		LIST_MEMBER_NONE,					// Spawner is not part of our lists

		LIST_MEMBER_COUNT
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CFMtx43A m_Mtx;			// For point emitters, place the position in m_vPos and the unit direction in m_vZ (m_vX and m_vY are ignored)
							// For plane emitters, place the origin in m_vPos and fill out m_vX, m_vY, and m_vZ (emissions will be in the +Z axis)

	f32 m_fPlaneDimX;							// Total X dimension of the bounded plane (used only for bounded plane emitters)
	f32 m_fPlaneDimY;							// Total Y dimension of the bounded plane (used only for bounded plane emitters)

	EmitterType_e m_nEmitterType;				// Type of emitter
	const CFDebrisGroup *m_pDebrisGroup;		// The debris group to use with this spawner

	CFDebrisDef::Callback_t *m_pFcnCallback;	// Callback function for each particle
	void *m_pUser;								// User field
	u16 m_nUser;								// User field
	u8 m_nOwnerID;								// Used to uniquely identify the owner of the debris so that a selective kill may be done (0=not identified)

	u8 m_nFlags;								// See FLAG_* for info
	u8 m_nMinDebrisCount;						// Minimum number of debris particles to spawn
	u8 m_nMaxDebrisCount;						// Maximum number of debris particles to spawn

	u8 m_nTintRed;								// Red tinting
	u8 m_nTintGreen;							// Green tinting
	u8 m_nTintBlue;								// Blue tinting

	const CFDebrisMesh *m_pDebrisMesh;			// Debris mesh to use (NULL=obtain mesh from the debris group)

	f32 m_fSpawnerAliveSecs;					// Seconds this spawner will be in full-affect before starting to fade (0=spawn for one frame only)
	f32 m_fSecsBetweenSpawns;					// Seconds between spawns
	f32 m_fSpawnerFadeSecs;						// How long the fading will last before finally killing off this spawner (0=instant kill)

	f32 m_fMinSpeed;							// Minimum initial speed
	f32 m_fMaxSpeed;							// Maximum initial speed

	f32 m_fUnitDirSpread;						// 0=spawn along unit direction, 1=max spread angle from unit direction

	f32 m_fScaleMul;							// Scale multiplier
	f32 m_fGravityMul;							// Gravity multiplier
	f32 m_fRotSpeedMul;							// Rotational speed multiplier

	SmokeTrailAttrib_t *m_pSmokeTrailAttrib;	// Smoke trail attribute (NULL if a smoke trail is not wanted)
	FParticle_DefHandle_t m_hParticleDef1;		// Particle definition handle 1 (FPARTICLE_INVALID_HANDLE if a particle effect is not wanted)
	FParticle_DefHandle_t m_hParticleDef2;		// Particle definition handle 2 (FPARTICLE_INVALID_HANDLE if a particle effect is not wanted)

	static u32 m_nLowestDebrisWeight;			// Current lowest debris weight (or 0xffffffff if not calculated, yet)



//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;				// TRUE once InitSpawnerSystem() has been called
	static FLinkRoot_t m_RootFree;					// Linklist of free spawners
	static FLinkRoot_t m_RootActive;				// Linklist of active spawners
	static CFDebrisSpawner *m_pSpawnerPool;			// Pool of spawner objects
	static CFDebrisSpawner **m_ppSpawnerWorkList;	// Holds pointers to the spawners we're currently working on
	static CFDebrisSpawner m_DefaultSpawner;		// Default spawner object

	BOOL8 m_bFading;						// We're in our fading phase
	BOOL8 m_bCameFromFreePool;				// We originally came from our free pool
	ListMember_e m_nListMember;				// Which list we're in

	f32 m_fCountdownTimer;					// Times how long the spawner has been alive or is fading
	f32 m_fSecsBeforeNextSpawn;				// Countdown seconds until we spawn again

	FLink_t m_Link;							// Link to other spawners




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitSpawnerSystem( u32 nMaxSpawnerCount );
	static void UninitSpawnerSystem( void );
	static FINLINE BOOL IsSpawnerSystemInitialized( void ) { return m_bSystemInitialized; }

#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	FINLINE CFDebrisSpawner() { m_bCameFromFreePool=FALSE; m_nListMember=LIST_MEMBER_NONE; m_pSmokeTrailAttrib=(SmokeTrailAttrib_t *)0xffffffff; }
#else
	FINLINE CFDebrisSpawner() { m_bCameFromFreePool=FALSE; m_nListMember=LIST_MEMBER_NONE; }
#endif

	FINLINE void InitToDefaults( void );

	static CFDebrisSpawner *GetFreeSpawner( void );
	static void KillAll( void );
	void Kill( void );
	void Spawn( void );
	BOOL Spawn( CFWorldMesh *pWorldMesh, u32 nBoneIndex );

	BOOL IsSpawning( void ) const;
	f32 GetUnitIntensity( void ) const;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	// These are called by CFDebris:
	static void Work( void );

	BOOL _Spawn( CFWorldMesh *pWorldMesh=NULL, s32 nBoneIndex=-1 );


	friend class CFDebris;


	FCLASS_STACKMEM_ALIGN( CFDebrisSpawner );
} FCLASS_ALIGN_SUFFIX;


FINLINE void CFDebrisSpawner::InitToDefaults( void ) {
	FASSERT( IsSpawnerSystemInitialized() );
	FASSERT( !m_bCameFromFreePool );
	FASSERT( m_nListMember == LIST_MEMBER_NONE );

	fang_MemCopy( this, &m_DefaultSpawner, sizeof(CFDebrisSpawner) );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebris
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFDebris {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef BOOL KillSelectiveCallback( const CFDebris *pDebris );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum ListMember_e {
		LIST_MEMBER_FREE_POOL,					// This object is currently in the free pool
		LIST_MEMBER_ACTIVE_POOL,				// This object is currently in the active pool

		LIST_MEMBER_COUNT
	};


	enum LifeState_e {
		LIFE_STATE_NORMAL,						// The object is alive
		LIFE_STATE_FADING_OUT,					// The object is fading out

		LIFE_STATE_COUNT
	};


	enum {
		FLAG_FLATTEN_MODE			= 0x01,		// TRUE when we're in flatten mode
		FLAG_IN_CALLBACK			= 0x02,		// TRUE when we're in the callback
		FLAG_CALLBACK_KILLED_US		= 0x04,		// TRUE if the callback called Kill()
		FLAG_FINAL_REST_POS			= 0x08,		// TRUE if the debris piece has come to rest on world geo
		FLAG_PLAYED_IMPACT_SOUND	= 0x10,		// TRUE if we've already played the impact sound

		FLAG_NONE					= 0x00
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;	// TRUE once InitDebrisSystem has been called
	static FLinkRoot_t m_RootFree;		// Linklist of free debris objects
	static FLinkRoot_t m_RootActive;	// Linklist of active debris objects
	static u32 m_nMaxPoolElementCount;	// Number of elements in m_pDebrisPool
	static CFDebris *m_pDebrisPool;		// Pool of debris objects
	static CFDebris **m_ppWorkList;		// Array of debris pointers we're currently working on
	static CFMeshPool *m_pMeshPool;		// Pool of meshes
	static u32 m_nSampleLightInstance;	// Used to spread out ambient light sampling over several frames
	static f32 m_fImpactSoundTimer;		// Counts down to govern the number of impact sounds we play

	FLink_t m_Link;						// Link to other debris objects in pool

	u8 m_nListMember;					// Which linklist this object is a member of
	u8 m_nLifeState;					// Whether we're alive or fading out

	u8 m_nSampleLightCountdown;			// When this reaches 0, it's time to sample the ambient light
	u8 m_nFlags;						// See FLAG_* for info

	CFDebrisDef m_DebrisDef;			// Debris definition parameters

	CFAudioEmitter *m_pAudioEmitterFlaming;	// Audio emitter for the flaming sound (NULL=none)

	SmokeTrailHandle_t m_hSmokeTrail;		// Handle to our smoke trail (SMOKETRAIL_NULLHANDLE if none)
	FParticle_EmitterHandle_t m_hParticle1;	// Particle emitter handle 1 (FPARTICLE_INVALID_HANDLE if none)
	FParticle_EmitterHandle_t m_hParticle2;	// Particle emitter handle 2 (FPARTICLE_INVALID_HANDLE if none)

	CFQuatA m_Quat;						// Rotational quaternion

	CFVec3A m_FlattenModeUnitRotAxis;	// When in flatten mode, this is the unit axis we're rotating about
	CFVec3A m_FlattenModeUnitFaceNorm;	// When in flatten mode, this is the unit face normal we're shooting for

	f32 m_fMovingSlowlyCountdownSecs;	// Used to determine how long the particle has been moving slowly
	f32 m_fCountdownSecs;				// Used for timing life, fade-out, etc.
	f32 m_fFadeFactor;					// (m_DebrisDef.m_fMeshScale / m_DebrisDef.m_fFadeSecs)  ...or...  (m_DebrisDef.m_fMeshUnitOpacity / m_DebrisDef.m_fFadeSecs)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitDebrisSystem( u32 nMaxDebrisCount );
	static void UninitDebrisSystem( void );
	static FINLINE BOOL IsDebrisSystemInitialized( void ) { return m_bSystemInitialized; }

	static CFDebris *Spawn( const CFDebrisDef *pDebrisDef, const CFMtx43A *pInitialMtx=NULL );
	static void KillAll( BOOL bCallTheCallbacks=FALSE );
	static void KillAllSelective( u8 nOwnerID, KillSelectiveCallback *pFcnFilterCallback, BOOL bCallTheCallbacks );

	static void Work( void );

	FINLINE BOOL IsCreated( void ) const { return (m_nListMember == LIST_MEMBER_ACTIVE_POOL); }

	void Kill( BOOL bCallTheCallback=FALSE );
	BOOL StartFading( BOOL bCallTheCallback=FALSE );

	FINLINE const CFDebrisDef *GetDebrisDef( void ) const { FASSERT( IsCreated() ); return &m_DebrisDef; }

	FINLINE void SetCallback( CFDebrisDef::Callback_t *pFcnCallback ) { FASSERT( IsCreated() ); m_DebrisDef.m_pFcnCallback = pFcnCallback; }
	FINLINE CFDebrisDef::Callback_t *GetCallback( void ) const { FASSERT( IsCreated() ); return m_DebrisDef.m_pFcnCallback; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FINLINE CFDebris() {}

	void _SpawnBoned( void );
	static CFDebris *_GetFreeDebris( const CFDebrisDef *pDebrisDef );
	void _Work( void );
	BOOL _CallCallback( CFDebrisDef::CallbackReason_e nReason, FCollImpact_t *pCollImpact );
	void _FinishKill( void );

	BOOL _MoveLinear_NoColl( f32 fTimeStepSecs );
	BOOL _MoveLinear_RayColl( f32 fTimeStepSecs );


	FCLASS_STACKMEM_ALIGN( CFDebris );
} FCLASS_ALIGN_SUFFIX;



#endif

