//////////////////////////////////////////////////////////////////////////////////////
// fexplosion.cpp - 
//
// Author: Nathan Miller   
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
// 02/03/03 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fres.h"
#include "fclib.h"
#include "fdraw.h"
#include "ffile.h"
#include "floop.h"
#include "fvtxpool.h"
#include "fresload.h"
#include "frenderer.h"
#include "fexplosion.h"
#include "fstringtable.h"
#include "smoketrail.h"
#include "fsound.h"
#include "fliquid.h"
#include "fdecal.h"

static ExplosionDebrisCallback_t *_pSurfaceDebrisCallback			= NULL;
static ExplosionDustTextureCallback_t *_pSurfaceDustTextureCallback = NULL;
static ExplosionDamageCallback_t *_pDamageCallback = NULL;

#define _SPAWNER_SIGNATURE	( 0x12345678 )
#define _DUST_GRAVITY		( -32.0f )

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFExplosionDef
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

// Explosion definition from CSV
FCLASS_ALIGN_PREFIX class CFExplosionDef {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct {
		f32 fExplosionTexSize;							// Size of the explosion texture
		f32 fExplosionTexLife;							// Life of the explosion texture
		f32 fExplosionTexLoops;							// Number of loops
		u32 uExplosionMultiple;							// TRUE if we want multiple explosion textures, false otherwise

		f32 fDustSizeEnd;								// Size of the dust to end
		f32 fDustLife;									// Life of the dust
		f32 fDustUpVel;									// How fast to float up

		f32 fSmokeUnitIntensity;						// Unit intensity of the smoke particle system. (0.0f = min, 1.0f = max)
		f32 fSmokeParticleDelay;						// Delay until the smoke particles are put into the world

		cchar *pszTextureName;							// Base texture name to use for the explosion
		f32 fTexPushAmntGround;							// When on the ground, how much of the texture size * 0.5 to push off the ground
		f32 fTexPushAmntWall;							// When on a wall, how much of the texture size * 0.5 to push off the wall
		f32 fTexPushOnAxis;								// How much of the texture size * 0.5 to use when spacing explosions

		FParticle_DefHandle_t hSmokeParticleDef;		// Particle system to spawn for smoke
		FParticle_DefHandle_t hExplosionParticleDef;	// Particle system to spawn when the explosion is created
		CFSoundGroup *pSoundGroup;						// Explosion sound group


		// Dynamic light
		f32 fLightLifetime;								// Life of the light
		f32 fLightRadius;								// Radius of the light
		f32 fLightRed;									// R component of the light (0 - 1)
		f32 fLightGreen;								// G component of the light (0 - 1)
		f32 fLightBlue;									// B component of the light (0 - 1)
		u32 nLightMotif;								// Light motif

		cchar *pszSurfaceDebrisSize1;					// Size of surface debris
		cchar *pszSurfaceDebrisSize2;					// Size of surface debris

		CFDebrisGroup *pDebrisGroup;					// Debris group to use for the explosion debris
		FParticle_DefHandle_t hSmokeDebrisParticleDef;	// Handle for smoke
		FParticle_DefHandle_t hFireDebrisParticleDef;	// Handle for fire

		u32 uMinDebrisNormal;							// Min amount of normal explosion debris
		u32 uMaxDebrisNormal;							// Max amount of normal explosion debris
		u32 uMinDebrisFire;								// Min amount of fire explosion debris
		u32 uMaxDebrisFire;								// Max amount of fire explosion debris

		f32 fMinSpeed;									// Minimum initial speed
		f32 fMaxSpeed;									// Maximum initial speed
		f32 fUnitDirSpread;								// 0=spawn along unit direction, 1=max spread angle from unit direction
		f32 fScaleMul;									// Scale multiplier
		f32 fGravityMul;								// Gravity multiplier
		f32 fRotSpeedMul;								// Rotational speed multiplier

		void *pDamageProfile;							// The damage profile for this explosion
		FDecalDefHandle_t hDecalDef;					// The decal definition for this explosion

	} _ExplosionDef_t;


//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static const FGameData_TableEntry_t m_aGameDataVocab[];

	cchar *m_pszExplosionName;

	f32 m_fExplosionTexSize;							// Size of the explosion texture
	f32 m_fExplosionTexLife;							// Life of the explosion texture
	f32 m_fExplosionTexLoops;							// Number of loops
	BOOL m_bExplosionMultiple;							// TRUE if we want multiple explosion textures

	f32 m_fDustSizeEnd;									// Size of the dust to end
	f32 m_fDustLife;									// Life of the dust
	f32 m_fDustUpVel;									// How fast to float up

	f32 m_fSmokeUnitIntensity;							// Unit intensity of the smoke particle system. (0.0f = min, 1.0f = max)
	f32 m_fSmokeParticleDelay;							// Delay until particle smoke

	u32 m_uNumTextures;									// Number of textures in our animation
	f32 m_fAnimRate;									// Rate that we should animate at
	CFTexInst **m_apTextures;							// Pointer to our textures
	f32 m_fTexPushAmntGround;							// When on the ground, how much of the texture size * 0.5 to push off the ground
	f32 m_fTexPushAmntWall;								// When on a wall, how much of the texture size * 0.5 to push off the wall
	f32 m_fTexPushOnAxis;								// How much of the texture size * 0.5 to use when spacing explosions

	FParticle_DefHandle_t m_hSmokeParticleDef;			// Particle system to spawn for smoke
	FParticle_DefHandle_t m_hExplosionParticleDef;		// Particle system to spawn when the explosion is created
	CFSoundGroup *m_pSoundGroup;						// Explosion sound group

	// Dynamic light
	f32 m_fLightLifetime;								// Life of the light
	f32 m_fLightRadius;									// Radius of the light
	f32 m_fLightRed;									// R component of the light (0 - 1)
	f32 m_fLightGreen;									// G component of the light (0 - 1)
	f32 m_fLightBlue;									// B component of the light (0 - 1)
	u32 m_nLightMotif;									// Light motif index

	FExplosionSurfceDebrisSize_e m_eDebrisSize1;		// Size of surface debris
	FExplosionSurfceDebrisSize_e m_eDebrisSize2;		// Size of surface debris

	CFDebrisGroup *m_pDebrisGroup;						// Debris for explosion
	FParticle_DefHandle_t m_hSmokeDebrisParticleDef;	// Smoke
	FParticle_DefHandle_t m_hFireDebrisParticleDef;		// Fire

	u8 m_uMinDebrisNormal;								// Min amount of normal explosion debris
	u8 m_uMaxDebrisNormal;								// Max amount of normal explosion debris
	u8 m_uMinDebrisFire;								// Min amount of fire explosion debris
	u8 m_uMaxDebrisFire;								// Max amount of fire explosion debris

	f32 m_fMinSpeed;									// Minimum initial speed
	f32 m_fMaxSpeed;									// Maximum initial speed
	f32 m_fUnitDirSpread;								// 0=spawn along unit direction, 1=max spread angle from unit direction
	f32 m_fScaleMul;									// Scale multiplier
	f32 m_fGravityMul;									// Gravity multiplier
	f32 m_fRotSpeedMul;									// Rotational speed multiplier

	void *m_pDamageProfile;								// Damage profile for the explosion
	FDecalDefHandle_t m_hDecalDef;						// Decal def for the explosion

	FLink_t m_Link;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	FINLINE CFExplosionDef() {}

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FCLASS_STACKMEM_ALIGN( CFExplosionDef );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFExplosionSet
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

// A set of explosions loaded from the explosion CSV
FCLASS_ALIGN_PREFIX class CFExplosionSet {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bModuleStartedUp;
	static FLinkRoot_t m_RootExplosions;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleStartedUp( void ) { return m_bModuleStartedUp; }

	static CFExplosionDef *Find( cchar *pszName );
	static BOOL Load( cchar *pszName );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static void _LoadFromGameData( FGameDataTableHandle_t hTable );
	static BOOL _BuildTextureList( cchar *pszBaseName, CFExplosionDef *pDef );
	static void _ResDestroyedCallback( void *pResMem );

	FINLINE CFExplosionSet() {}

	FCLASS_STACKMEM_ALIGN( CFExplosionSet );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFExplosionGroup
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

// An explosion group loaded from a CSV
FCLASS_ALIGN_PREFIX class CFExplosionGroup {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	typedef struct {
		cchar *pszEplosionName;
		u32 uNumExplosions;
		f32 fExplosionDelay;
		f32 fOffsetRange;
	} _ExplosionGroupDef_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static FLinkRoot_t m_LinkRoot;							// Linklist of all loaded debris groups

	u32 m_uIndexInList;										// Used to verify that we have a valid handle

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bModuleStartedUp;
	static const FGameData_TableEntry_t m_aGameDataVocab[];	// Vocabulary for parsing CSV data

	cchar *m_pszGroupName;									// Name of this explosion group

	u32 m_uNumExplosions;									// Number of explosions to spawn
	f32 m_fExplosionDelay;									// Delay between explosions
	f32 m_fOffsetRange;										// How much to offset each explosion from initial spawn point

	CFExplosionDef *m_pExplosionDef;						// Explosion definition to use

	FLink_t m_Link;											// Link to other debris groups

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleStartedUp( void ) { return m_bModuleStartedUp; }

	static CFExplosionGroup *Find( cchar *pszName );
	static BOOL LoadAll( cchar *pszSet, cchar *pszGroups );

	FINLINE const u32 &GetNumExplosions( void )		{ return m_uNumExplosions; }
	FINLINE const f32 &GetExplosionDelay( void )	{ return m_fExplosionDelay; }
	FINLINE const f32 &GetOffsetRange( void )		{ return m_fOffsetRange; }
	FINLINE CFExplosionDef *GetExplosionDef( void ) { return m_pExplosionDef; }

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FINLINE CFExplosionGroup() {}

	static BOOL _LoadGroups( cchar *pszName );
	static void _LoadFromGameData( FGameDataTableHandle_t hTable );
	static void _ResDestroyedCallback( void *pResMem );

	FCLASS_STACKMEM_ALIGN( CFExplosionGroup );
} FCLASS_ALIGN_SUFFIX;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFExplosionSpawner
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFExplosionSpawner {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	
//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	u32 m_uSignature;								// Used to verify that we have a valid handle.  Should always be _SPAWNER_SIGNATURE

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;				// TRUE once InitSpawnerSystem() has been called
	static FLinkRoot_t m_RootFree;					// Linklist of free spawners
	static FLinkRoot_t m_RootActive;				// Linklist of active spawners
	static u32 m_uSpawnerPoolSize;					// How big our pool is
	static CFExplosionSpawner *m_pSpawnerPool;		// Pool of spawner objects
	static u32 m_uDamagerSize;						// Size of the damager structure.  DO NOT MODIFY AFTER InitSpawnerSystem() HAS BEEN CALLED
	static void *m_pDamagerPool;					// Damager pool used to copy needed damage data

	void *m_pDamagerData;							// Pointer to damager data that is filled in from data in FExplosionSpawnParams_t
	FExplosionSpawnParams_t SpawnParams;

	u32 m_uNumExplosionsLeft;						// Number of explosions we have left to spawn
	f32 m_fTimeTillNextExplosion;					// Time till we spawn another explosion

	CFWorldLight *m_pLight;							// Shared light
	CFExplosionGroup *m_pExplosionGroup;			// Explosion group we are using

	FLinkRoot_t m_RootExplosions;					// Explosions we have spawned

	FLink_t m_Link;									// Link to other spawners

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL InitSpawnerSystem( u32 nMaxSpawnerCount, const u32 &uDamagerSize );
	static void UninitSpawnerSystem( void );
	static void Work( void );
	//static void KillAll( void );
	static FINLINE BOOL IsSpawnerSystemInitialized( void )	{ return m_bSystemInitialized; }
	static CFExplosionSpawner *GetFreeSpawner( void );
	static void ReturnSpawner( CFExplosionSpawner *pSpawn ); 

	FINLINE CFExplosionSpawner()							{ m_uSignature = _SPAWNER_SIGNATURE; } 
	FINLINE BOOL IsDead( void )								{ return m_RootExplosions.nCount == 0 && m_uNumExplosionsLeft == 0; }

	void InitToDefaults( void );
	void Spawn( CFExplosionGroup *pExplosionGroup, FExplosionSpawnParams_t *pSpawnParams, const void *pDamageImpactTriData = NULL );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _Work( void );
	void _SpawnExplosion( const void *pDamageImpactTriData = NULL );

	FCLASS_STACKMEM_ALIGN( CFExplosionSpawner );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFExplosion
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFExplosion {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum {
		_NUM_DUST_PUFFS = 3,
	};

	enum {
		_FLAG_NONE					= 0x00,
		_FLAG_ACTIVE_TEXTURE		= 0x01,
		_FLAG_ACTIVE_SMOKE			= 0x02,
		_FLAG_ACTIVE_LIGHT			= 0x04,
		_FLAG_ACTIVE_DUST			= 0x08,
		_FLAG_SHARED_LIGHT			= 0x10,
		_FLAG_SINGLE_EXPLOSION_TEX	= 0x20,
		_FLAG_FOLLOW_UNIT_DIR		= 0x40,
		_FLAG_SINGLE_SMOKE			= 0x80, // This flag is for when we have no texture animation, but we still want a single puff of smoke
	};

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	typedef struct {
		f32 fTimeTillSmoke;							// How much longer until this texture fireball makes smoke
		u32 uExplosionAnimFrame;					// Current frame in texture animation
		f32 fExplosionAnimTimeOn;					// How long has the animation lived
		CFVec3A Pos_WS;								// Position in WS
	} _FireballTextureAnimInfo_t;

	typedef struct {
		f32 fDustLife;								// How long the dust will live
		f32 fDustSize;								// How big is it
		f32 fDustVelY;								// How much to move up
		CFVec3A Pos_WS;								// Position in WS
	} _DustPuffInfo_t;

	static BOOL m_bSystemInitialized;
	static FLinkRoot_t m_RootFree;					// Linklist of free explosion objects
	static FLinkRoot_t m_RootActive;				// Linklist of active explosion objects
	static CFExplosion *m_pExplosionPool;			// Pool of explosion objects
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;	// Smoke trail

	u32 m_uExplosionFlags;							// Flags that give information about the explosion's state, see _FLAG_* above

	f32 m_fLife;									// How long we have been alive

	CFVec3A m_Pos_WS;								// Epicenter in WS
	CFVec3A m_UnitDir;								// Unit direction of surface we hit
	CFExplosionDef *m_pExpDef;						// Explosion definition we are using

	_FireballTextureAnimInfo_t m_ExplosionTexInfo[3];	// Texture animations for fireballs
	_DustPuffInfo_t m_DustPuffs[_NUM_DUST_PUFFS];		// Dust puffs

	f32 m_fLightLife;								// Life of the light
	CFWorldLight *m_pLight;							// Light we are using, may or may not own
	
	const CFTexInst *m_pDustTexture;				// Dust Texture we are using

	FLink_t m_ExplosionLink;						// Used in CFExplosion only to link explosions.  DO NOT USE ELSEWHERE

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	FLink_t m_Link;									// Used for general linking of explosions.

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitExplosionSystem( u32 uMaxExplosions );
	static void UninitExplosionSystem( void );
	static FINLINE BOOL IsExplosionSystemInitialized( void ) { return m_bSystemInitialized; }

	static CFExplosion *GetExplosionFromPool( void );
	static void ReturnExplosionToPool( CFExplosion *pExp );

	static void Draw( CFCamera *pCam );

	// Returns true if we can return the explosion back to the pool
	FINLINE BOOL IsDead( void )				{ return ( m_uExplosionFlags & ~( _FLAG_SHARED_LIGHT | _FLAG_FOLLOW_UNIT_DIR ) ) == _FLAG_NONE; }
	FINLINE CFWorldLight *GetLight( void )	{ return m_pLight; }

	void Destroy( void );
	void Spawn( FExplosionSpawnParams_t *pSpawnParams, void *pDamagerData, const CFVec3A &Pos_WS, CFExplosionDef *pExpDef, CFWorldLight *pLight = NULL, const void *pDamageImpactTriData = NULL );
	void Work( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	FINLINE CFExplosion() {}

	static void _InitSmokeTrail( void );

	void _Draw( CFCamera *pCam );
	void _DestroyLight( void );
	void _SetupFireBalls( void );
	void _SpawnSurfaceDebris( FExplosionSpawnParams_t *pSpawnParams, const CFVec3A *pPos_WS, FExplosionSurfceDebrisSize_e eSize );
	void _SpawnExplosionDebris( const CFVec3A *pPos_WS, const CFVec3A *pUnitUp );
	void _SpawnSurfaceDust( FExplosionSpawnParams_t *pSpawnParams );

	FINLINE void _EnableTexture( void )				{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_TEXTURE ); }
	FINLINE void _EnableSmoke( void )				{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_SMOKE ); }
	FINLINE void _EnableLight( void )				{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_LIGHT ); }
	FINLINE void _EnableDust( void )				{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_DUST ); }
	FINLINE void _EnableSharedLight( void )			{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_SHARED_LIGHT ); }
	FINLINE void _EnableSingleExplosionTex( void )	{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_SINGLE_EXPLOSION_TEX ); }
	FINLINE void _EnableFollowUnitDir( void )		{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_FOLLOW_UNIT_DIR ); }
	FINLINE void _EnableSingleSmoke( void )			{ FMATH_SETBITMASK( m_uExplosionFlags, _FLAG_SINGLE_SMOKE ); }

	FINLINE void _DisableTexture( void )			{ FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_TEXTURE ); }
	FINLINE void _DisableSmoke( void )				{ FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_SMOKE ); }
	FINLINE void _DisableLight( void )				{ FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_LIGHT ); }
	FINLINE void _DisableDust( void )				{ FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_ACTIVE_DUST ); }
	FINLINE void _DisableSharedLight( void )		{ FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_SHARED_LIGHT ); }
	FINLINE void _DisableSingleExplosionTex( void ) { FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_SINGLE_EXPLOSION_TEX ); }
	FINLINE void _DisableFollowUnitDir( void )		{ FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_FOLLOW_UNIT_DIR ); }
	FINLINE void _DisableSingleSmoke( void )		{ FMATH_CLEARBITMASK( m_uExplosionFlags, _FLAG_SINGLE_SMOKE ); }

	FINLINE BOOL _IsActiveTexture( void )			{ return ( m_uExplosionFlags & _FLAG_ACTIVE_TEXTURE ); }
	FINLINE BOOL _IsActiveSmoke( void )				{ return ( m_uExplosionFlags & _FLAG_ACTIVE_SMOKE ); }
	FINLINE BOOL _IsActiveLight( void )				{ return ( m_uExplosionFlags & _FLAG_ACTIVE_LIGHT ); }
	FINLINE BOOL _IsActiveDust( void )				{ return ( m_uExplosionFlags & _FLAG_ACTIVE_DUST ); }
	FINLINE BOOL _IsSharedLight( void )				{ return ( m_uExplosionFlags & _FLAG_SHARED_LIGHT ); }
	FINLINE BOOL _IsSingleExplosionTex( void )		{ return ( m_uExplosionFlags & _FLAG_SINGLE_EXPLOSION_TEX ); }
	FINLINE BOOL _IsFollowUnitDir( void )			{ return ( m_uExplosionFlags & _FLAG_FOLLOW_UNIT_DIR ); }
	FINLINE BOOL _IsSingleSmoke( void )				{ return ( m_uExplosionFlags & _FLAG_SINGLE_SMOKE ); }

	FCLASS_STACKMEM_ALIGN( CFExplosion );
} FCLASS_ALIGN_SUFFIX;




const FGameData_TableEntry_t CFExplosionDef::m_aGameDataVocab[] = {
	// fExplosionTexRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fExplosionTexLife
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fExplosionTexLoops
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// uExplosionMultiple
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fDustSizeEnd
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fDustLife
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_10000,

	// fDustUpVel
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fSmokeUnitIntensity
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fSmokeParticleDelay
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

    // pszTextureName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fTexPushAmntGround
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fTexPushAmntWall
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fTexPushOnAxis
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	FGAMEDATA_VOCAB_PARTICLE,		// hSmokeParticleDef
	FGAMEDATA_VOCAB_PARTICLE,		// hExplosionParticleDef
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroup

	// fLightLifetime
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fLightRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_10000,

	// fLightRed
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fLightGreen
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fLightBlue
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	FGAMEDATA_VOCAB_MOTIF,			// nLightMotif

	// pszSurfaceDebrisSize1
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszSurfaceDebrisSize2
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_DEBRIS_GROUP,	// pszExplosionDebris
	FGAMEDATA_VOCAB_PARTICLE,		// hSmokeDebrisParticleDef
	FGAMEDATA_VOCAB_PARTICLE,		// hFireDebrisParticleDef

	// uMinDebrisNormal
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_255,

	// uMaxDebrisNormal
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_255,

	// uMinDebrisFire
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_255,

	// uMinDebrisFire
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_255,

	// fMinSpeed
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fMaxSpeed
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fUnitDirSpread
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fScaleMul
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fGravityMul
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fRotSpeedMul
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	FGAMEDATA_VOCAB_DAMAGE,	// pDamageProfile

	// hDecal
	FGAMEDATA_VOCAB_DECAL_DEF,


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


BOOL CFExplosionSet::m_bModuleStartedUp = FALSE;
FLinkRoot_t CFExplosionSet::m_RootExplosions;

BOOL CFExplosionSet::ModuleStartup( void ) {
	m_bModuleStartedUp = TRUE;

	flinklist_InitRoot( &m_RootExplosions, FANG_OFFSETOF( CFExplosionDef, m_Link ) );

	return TRUE;
}

void CFExplosionSet::ModuleShutdown( void ) {
	m_bModuleStartedUp = FALSE;
	flinklist_InitRoot( &m_RootExplosions, FANG_OFFSETOF( CFExplosionDef, m_Link ) );
}

CFExplosionDef *CFExplosionSet::Find( cchar *pszName ) {
	FASSERT( IsModuleStartedUp() );
	
	CFExplosionDef *pCurr = (CFExplosionDef *) flinklist_GetHead( &m_RootExplosions );

	for( ; pCurr; pCurr = (CFExplosionDef *) flinklist_GetNext( &m_RootExplosions, pCurr ) ) {
		if( !fclib_stricmp( pszName, pCurr->m_pszExplosionName ) ) {
			return pCurr;
		}
	}

	return NULL;
}

BOOL CFExplosionSet::Load( cchar *pszName ) {
	FASSERT( IsModuleStartedUp() );
	FASSERT( m_RootExplosions.nCount == 0 );

	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	
	FMemFrame_t MemFrame = fmem_GetFrame();

	hFile = fgamedata_LoadFileToFMem( pszName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFExplosionSet::Load: Could not load explosion set file '%s.csv'.\n", pszName );
		goto _ExitWithError;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		_LoadFromGameData( hTable );
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );

	return FALSE;
}

void CFExplosionSet::_LoadFromGameData( FGameDataTableHandle_t hTable ) {
	FASSERT( IsModuleStartedUp() );

	cchar *pszTableName;
	CFExplosionDef::_ExplosionDef_t ExpDef;
	CFExplosionDef *pDef = NULL;	

	pszTableName = fgamedata_GetTableName( hTable );

	if( Find( pszTableName ) ) {
		return;
	}

	FResFrame_t ResFrame = fres_GetFrame();

//ARG - >>>>>
//ARG - initialization crosses 'goto' scope
	FResHandle_t hRes;
//ARG - <<<<<

	if( !fgamedata_GetTableData( hTable, CFExplosionDef::m_aGameDataVocab, &ExpDef, sizeof( CFExplosionDef::_ExplosionDef_t ) ) ) {
		DEVPRINTF( "CFExplosionSet::_LoadFromGameData(): Trouble parsing explosion data for '%s'.\n", pszTableName );
		goto _ExitWithError;
	}

	hRes = fres_CreateWithCallback( NULL, NULL, _ResDestroyedCallback );
	
	if( hRes == FRES_NULLHANDLE ) {
		DEVPRINTF( "CFExplosionSet::_LoadFromGameData(): Could not create explosion resource object.\n" );
		goto _ExitWithError;
	}

	pDef = fnew CFExplosionDef;
	
	if( pDef == NULL ) {
		DEVPRINTF( "CFExplosionSet::_LoadFromGameData(): Not enough memory for new CFExplosionDef.\n" );
		goto _ExitWithError;
	}

	fres_SetBase( hRes, pDef );

	pDef->m_pszExplosionName = CFStringTable::AddString( NULL, pszTableName );
	
	if( pDef->m_pszExplosionName == NULL ) {
		DEVPRINTF( "CFExplosionSet::_LoadFromGameData(): Not enough memory to store name for explosion '%s'.\n", pszTableName );
		goto _ExitWithError;
	}

	pDef->m_fExplosionTexSize			= ExpDef.fExplosionTexSize;
	pDef->m_fExplosionTexLife			= ExpDef.fExplosionTexLife;
	pDef->m_fExplosionTexLoops			= ExpDef.fExplosionTexLoops;
	pDef->m_bExplosionMultiple			= ExpDef.uExplosionMultiple;
	pDef->m_fDustSizeEnd				= ExpDef.fDustSizeEnd;
	pDef->m_fDustLife					= ExpDef.fDustLife;
	pDef->m_fDustUpVel					= ExpDef.fDustUpVel;
	pDef->m_fSmokeUnitIntensity			= ExpDef.fSmokeUnitIntensity;
	pDef->m_fSmokeParticleDelay			= ExpDef.fSmokeParticleDelay;
	pDef->m_hSmokeParticleDef			= ExpDef.hSmokeParticleDef;
	pDef->m_hExplosionParticleDef		= ExpDef.hExplosionParticleDef;
	pDef->m_pSoundGroup					= ExpDef.pSoundGroup;
	pDef->m_fLightLifetime				= ExpDef.fLightLifetime;
	pDef->m_fLightRadius				= ExpDef.fLightRadius;
	pDef->m_fLightRed					= ExpDef.fLightRed;
	pDef->m_fLightGreen					= ExpDef.fLightGreen;
	pDef->m_fLightBlue					= ExpDef.fLightBlue;
	pDef->m_nLightMotif					= ExpDef.nLightMotif;
	pDef->m_fTexPushAmntGround			= ExpDef.fTexPushAmntGround;
	pDef->m_fTexPushAmntWall			= ExpDef.fTexPushAmntWall;
	pDef->m_fTexPushOnAxis				= ExpDef.fTexPushOnAxis;
	pDef->m_pDamageProfile				= ExpDef.pDamageProfile;

	pDef->m_eDebrisSize1 = FEXPLOSION_SIZE_NONE;

	if( ExpDef.pszSurfaceDebrisSize1 != NULL ) {
		if( !fclib_stricmp( "small", ExpDef.pszSurfaceDebrisSize1 ) ) {
			pDef->m_eDebrisSize1 = FEXPLOSION_SIZE_SMALL;
		} else if( !fclib_stricmp( "medium", ExpDef.pszSurfaceDebrisSize1 ) ) {
			pDef->m_eDebrisSize1 = FEXPLOSION_SIZE_MEDIUM;
		} else if( !fclib_stricmp( "large", ExpDef.pszSurfaceDebrisSize1 ) ) {
			pDef->m_eDebrisSize1 = FEXPLOSION_SIZE_LARGE;
		}
	}

	pDef->m_eDebrisSize2 = FEXPLOSION_SIZE_NONE;

	if( ExpDef.pszSurfaceDebrisSize2 != NULL ) {
		if( !fclib_stricmp( "small", ExpDef.pszSurfaceDebrisSize2 ) ) {
			pDef->m_eDebrisSize2 = FEXPLOSION_SIZE_SMALL;
		} else if( !fclib_stricmp( "medium", ExpDef.pszSurfaceDebrisSize2 ) ) {
			pDef->m_eDebrisSize2 = FEXPLOSION_SIZE_MEDIUM;
		} else if( !fclib_stricmp( "large", ExpDef.pszSurfaceDebrisSize2 ) ) {
			pDef->m_eDebrisSize2 = FEXPLOSION_SIZE_LARGE;
		}
	}

	pDef->m_pDebrisGroup = ExpDef.pDebrisGroup;
	pDef->m_hSmokeDebrisParticleDef = ExpDef.hSmokeDebrisParticleDef;
	pDef->m_hFireDebrisParticleDef = ExpDef.hFireDebrisParticleDef;

	pDef->m_uMinDebrisNormal = ExpDef.uMinDebrisNormal;
	pDef->m_uMaxDebrisNormal = ExpDef.uMaxDebrisNormal;
	pDef->m_uMinDebrisFire = ExpDef.uMinDebrisFire;
	pDef->m_uMaxDebrisFire = ExpDef.uMaxDebrisFire;

	pDef->m_fMinSpeed = ExpDef.fMinSpeed;
	pDef->m_fMaxSpeed = ExpDef.fMaxSpeed;
	pDef->m_fUnitDirSpread = ExpDef.fUnitDirSpread;
	pDef->m_fScaleMul = ExpDef.fScaleMul;
	pDef->m_fGravityMul = ExpDef.fGravityMul;
	pDef->m_fRotSpeedMul = ExpDef.fRotSpeedMul;
	pDef->m_hDecalDef = ExpDef.hDecalDef;

	if( !_BuildTextureList( ExpDef.pszTextureName, pDef ) ) {
		goto _ExitWithError;
	}

	flinklist_AddTail( &m_RootExplosions, pDef );

	return;

_ExitWithError:
	fdelete( pDef );
	fres_ReleaseFrame( ResFrame );
}

BOOL CFExplosionSet::_BuildTextureList( cchar *pszBaseName, CFExplosionDef *pDef ) {
	pDef->m_uNumTextures = 0;
	pDef->m_apTextures = NULL;

	if( !pszBaseName ) {
		return TRUE;
	}

	if( pDef->m_fExplosionTexLife <= 0.0f ) {
		return TRUE;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();
	FResFrame_t ResFrame = fres_GetFrame();
	char *pszTempName = NULL;
	s32 nLen = fclib_strlen( pszBaseName );
	u32 i, j;

	pszTempName = (char *) fmem_Alloc( nLen + 3 );
	fclib_strcpy( pszTempName, pszBaseName );

	// First, find out how many textures we have
	for( i = 0; i < 100; ++i ) {
		pszTempName[ nLen ] = (char)( ( ( i / 10 ) % 10) + '0' );
		pszTempName[ nLen + 1 ] = (char)( ( i % 10 ) + '0' );
		pszTempName[ nLen + 2 ] = 0;

		if( !fresload_ExistsOnDisk( FTEX_RESNAME, pszTempName ) ) {
			break;
		}
	}

	// We have at least one of them
	if( i > 0 ) {
		pDef->m_uNumTextures = i;
		pDef->m_apTextures = (CFTexInst **) fres_Alloc( sizeof( CFTexInst * ) * pDef->m_uNumTextures );

		if( pDef->m_apTextures == NULL ) {
			DEVPRINTF( "CFExplosionSet::_BuildTextureList(): Failed to allocate memory for CFTexInst **.\n" );
			goto _ExitWithError;
		}

		for( j = 0; j < pDef->m_uNumTextures; ++j ) {
			pDef->m_apTextures[j] = NULL;
		}

		for( j = 0; j < i; ++j ) {
			pszTempName[ nLen ] = (char)( ( ( j / 10 ) % 10) + '0' );
			pszTempName[ nLen + 1 ] = (char)( ( j % 10 ) + '0' );
			pszTempName[ nLen + 2 ] = 0;

			pDef->m_apTextures[j] = fnew CFTexInst;

			if( pDef->m_apTextures[j] == NULL ) {
				DEVPRINTF( "CFExplosionSet::_BuildTextureList(): Failed to allocate memory for CFTexInst.\n" );
				goto _ExitWithError;
			}

			pDef->m_apTextures[j]->SetTexDef( (FTexDef_t *) fresload_Load( FTEX_RESNAME, pszTempName ) );
	
			if( pDef->m_apTextures[j]->GetTexDef() == NULL ) {
				DEVPRINTF( "CFExplosionSet::_BuildTextureList(): Failed to load '%s'.\n", pszTempName );
			}
		}

		pDef->m_fAnimRate = (f32) pDef->m_uNumTextures * fmath_Inv( pDef->m_fExplosionTexLife );

		if( pDef->m_fExplosionTexLoops > 0.0f ) {
			pDef->m_fAnimRate *= fmath_Inv( pDef->m_fExplosionTexLoops );
		}
	}

	fmem_ReleaseFrame( MemFrame );
	
	return TRUE;

_ExitWithError:

	if( pDef->m_apTextures ) {
		for( j = 0; j < pDef->m_uNumTextures; ++j ) {
			fdelete( pDef->m_apTextures[i] );
		}
	}

	fdelete_array( pDef->m_apTextures );

	pDef->m_apTextures = NULL;
	pDef->m_uNumTextures = 0;

	fres_ReleaseFrame( ResFrame );

	return FALSE;
}

void CFExplosionSet::_ResDestroyedCallback( void *pResMem ) {
	CFExplosionDef *pDef = (CFExplosionDef *) pResMem;

	FASSERT( pDef == (CFExplosionDef *) flinklist_GetStructurePointer( &m_RootExplosions, m_RootExplosions.pTailLink ) );

	flinklist_Remove( &m_RootExplosions, pDef );
}

BOOL CFExplosionGroup::m_bModuleStartedUp = FALSE;
FLinkRoot_t CFExplosionGroup::m_LinkRoot;

const FGameData_TableEntry_t CFExplosionGroup::m_aGameDataVocab[] = {
	// pszEplosionName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// uNumExplosions
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fExplosionDelay
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// fOffsetRange
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg10000,
	F32_DATATABLE_10000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

BOOL CFExplosionGroup::ModuleStartup( void ) {
	FASSERT( !IsModuleStartedUp() );

	flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CFExplosionGroup, m_Link ) );

	m_bModuleStartedUp = TRUE;

	return TRUE;
}


void CFExplosionGroup::ModuleShutdown( void ) {
	if( IsModuleStartedUp() ) {
		m_bModuleStartedUp = FALSE;
		//ReleaseAll();
		flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CFExplosionGroup, m_Link ) );
	}
}

CFExplosionGroup *CFExplosionGroup::Find( cchar *pszName ) {
	FASSERT( IsModuleStartedUp() );

	if( pszName == NULL ) {
		return NULL;
	}
	
	CFExplosionGroup *pCurr = (CFExplosionGroup *) flinklist_GetHead( &m_LinkRoot );

	for( ; pCurr; pCurr = (CFExplosionGroup *) flinklist_GetNext( &m_LinkRoot, pCurr ) ) {
		if( !fclib_stricmp( pszName, pCurr->m_pszGroupName ) ) {
			return pCurr;
		}
	}

	return NULL;
}

BOOL CFExplosionGroup::LoadAll( cchar *pszSet, cchar *pszGroups ) {
	FASSERT( IsModuleStartedUp() );

	if( !CFExplosionSet::Load( pszSet ) ) {
		return FALSE;
	}

	if( !_LoadGroups( pszGroups ) ) {
		return FALSE;
	}

	return TRUE;
}

BOOL CFExplosionGroup::_LoadGroups( cchar *pszName ) {
	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	
	FMemFrame_t MemFrame = fmem_GetFrame();

	hFile = fgamedata_LoadFileToFMem( pszName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFExplosionGroup::_LoadGroups(): Could not load explosion group file '%s.csv'.\n", pszName );
		return FALSE;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		_LoadFromGameData( hTable );
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;
}

void CFExplosionGroup::_LoadFromGameData( FGameDataTableHandle_t hTable ) {
	FASSERT( IsModuleStartedUp() );

	cchar *pszTableName;
	_ExplosionGroupDef_t GrpDef;
	CFExplosionGroup *pGroup = NULL;	

	pszTableName = fgamedata_GetTableName( hTable );

	if( Find( pszTableName ) ) {
		return;
	}

	FResFrame_t ResFrame = fres_GetFrame();

//ARG - >>>>>
//ARG - initialization crosses 'goto' scope
	FResHandle_t hRes;
//ARG - <<<<<

	if( !fgamedata_GetTableData( hTable, m_aGameDataVocab, &GrpDef, sizeof( _ExplosionGroupDef_t ) ) ) {
		DEVPRINTF( "CFExplosionGroup::_LoadFromGameData(): Trouble parsing explosion group data for '%s'.\n", pszTableName );
		goto _ExitWithError;
	}

	pGroup = fnew CFExplosionGroup;

	if( pGroup == NULL ) {
		DEVPRINTF( "CFExplosionGroup::_LoadFromGameData(): Not enough memory for new CFExplosionGroup.\n" );
		goto _ExitWithError;
	}

	pGroup->m_pExplosionDef = CFExplosionSet::Find( GrpDef.pszEplosionName );

	if( pGroup->m_pExplosionDef == NULL ) {
		DEVPRINTF( "CFExplosionGroup::_LoadFromGameData(): Could not find explosion '%s'.\n", GrpDef.pszEplosionName );
		goto _ExitWithError;		
	}

	pGroup->m_pszGroupName = CFStringTable::AddString( NULL, pszTableName );
	
	if( pGroup->m_pszGroupName == NULL ) {
		DEVPRINTF( "CFExplosionGroup::_LoadFromGameData(): Not enough memory to store name for explosion group '%s'.\n", pszTableName );
		goto _ExitWithError;
	}

	hRes = fres_CreateWithCallback( NULL, NULL, _ResDestroyedCallback );
	
	if( hRes == FRES_NULLHANDLE ) {
		DEVPRINTF( "CFExplosionGroup::_LoadFromGameData(): Could not create explosion resource object.\n" );
		goto _ExitWithError;
	}

	fres_SetBase( hRes, pGroup );

	pGroup->m_fExplosionDelay	= GrpDef.fExplosionDelay;
	pGroup->m_fOffsetRange		= GrpDef.fOffsetRange;
	pGroup->m_uNumExplosions	= GrpDef.uNumExplosions;
	pGroup->m_uIndexInList		= m_LinkRoot.nCount;

	flinklist_AddTail( &m_LinkRoot, pGroup );

	return;

_ExitWithError:
	fdelete( pGroup );
	fres_ReleaseFrame( ResFrame );
}

void CFExplosionGroup::_ResDestroyedCallback( void *pResMem ) {
	CFExplosionGroup *pGrp = (CFExplosionGroup *) pResMem;

	FASSERT( pGrp == (CFExplosionGroup *) flinklist_GetStructurePointer( &m_LinkRoot, m_LinkRoot.pTailLink ) );

	flinklist_Remove( &m_LinkRoot, pGrp );
}

BOOL CFExplosionSpawner::m_bSystemInitialized = FALSE;
FLinkRoot_t CFExplosionSpawner::m_RootFree;
FLinkRoot_t CFExplosionSpawner::m_RootActive;
u32 CFExplosionSpawner::m_uSpawnerPoolSize = 0;
CFExplosionSpawner *CFExplosionSpawner::m_pSpawnerPool = NULL;
u32 CFExplosionSpawner::m_uDamagerSize = 0;
void *CFExplosionSpawner::m_pDamagerPool = NULL;

BOOL CFExplosionSpawner::InitSpawnerSystem( u32 nMaxSpawnerCount, const u32 &uDamagerSize ) {
	FASSERT( !IsSpawnerSystemInitialized() );
	FASSERT( nMaxSpawnerCount > 0 );
	FASSERT( uDamagerSize > 0 );

	flinklist_InitRoot( &m_RootFree, FANG_OFFSETOF( CFExplosionSpawner, m_Link ) );
	flinklist_InitRoot( &m_RootActive, FANG_OFFSETOF( CFExplosionSpawner, m_Link ) );

	FResFrame_t ResFrame = fres_GetFrame();

	m_pSpawnerPool = fnew CFExplosionSpawner[nMaxSpawnerCount];

	if( m_pSpawnerPool == NULL ) {
		DEVPRINTF( "CFExplosionSpawner::InitSpawnerSystem(): Not enough memory to allocate explosion spawner pool.\n" );
		goto _ExitWithError;
	}

	m_pDamagerPool = fres_Alloc( uDamagerSize * nMaxSpawnerCount );

	if( m_pDamagerPool == NULL ) {
		DEVPRINTF( "CFExplosionSpawner::InitSpawnerSystem(): Not enough memory to allocate explosion damager pool.\n" );
		goto _ExitWithError;
	}

	m_uSpawnerPoolSize = nMaxSpawnerCount;
	m_uDamagerSize = uDamagerSize;

	// Set the pointers for the damager data
	for( u32 i = 0; i < nMaxSpawnerCount; ++i ) {
		m_pSpawnerPool[i].m_pDamagerData = ( (u8 *) m_pDamagerPool ) + ( i * uDamagerSize );
	}

	flinklist_InitPool( &m_RootFree, m_pSpawnerPool, sizeof( CFExplosionSpawner ), nMaxSpawnerCount );

	m_bSystemInitialized = TRUE;

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	m_bSystemInitialized = FALSE;
	return FALSE;
}

void CFExplosionSpawner::UninitSpawnerSystem( void ) {
	fdelete_array( m_pSpawnerPool );

	m_pSpawnerPool = NULL;
	m_pDamagerPool = NULL;
	m_bSystemInitialized = FALSE;
	m_uDamagerSize = 0;
	m_uSpawnerPoolSize = 0;

	flinklist_InitRoot( &m_RootFree, FANG_OFFSETOF( CFExplosionSpawner, m_Link ) );
	flinklist_InitRoot( &m_RootActive, FANG_OFFSETOF( CFExplosionSpawner, m_Link ) );
}

void CFExplosionSpawner::Work( void ) {
	FASSERT( IsSpawnerSystemInitialized() );

	CFExplosionSpawner *pSpawner = (CFExplosionSpawner *) flinklist_GetHead( &m_RootActive );
	CFExplosionSpawner *pSpawnerNext = NULL;

	while( pSpawner ) {
		FASSERT_MSG( ( m_RootActive.nCount + m_RootFree.nCount ) == m_uSpawnerPoolSize, "CFExplosionSpawner::Work(): Active list plus free list does not equal total!" );

		pSpawnerNext = (CFExplosionSpawner *) flinklist_GetNext( &m_RootActive, pSpawner );

		pSpawner->_Work();

		if( pSpawner->IsDead() ) {
			flinklist_Remove( &m_RootActive, pSpawner );
			flinklist_AddHead( &m_RootFree, pSpawner );
		}

		pSpawner = pSpawnerNext;
	}
}

/*void CFExplosionSpawner::KillAll( void ) {
	CFExplosionSpawner *pSpawner = (CFExplosionSpawner *) flinklist_GetHead( &m_RootActive );
	CFExplosionSpawner *pSpawnerNext = NULL;
	CFExplosion *pExp = NULL;
	CFExplosion *pExpNext = NULL;

	// Go through all spawners
	while( pSpawner ) {
		pSpawnerNext = (CFExplosionSpawner *) flinklist_GetNext( &m_RootActive, pSpawner );

		flinklist_Remove( &m_RootActive, pSpawner );
		flinklist_AddHead( &m_RootFree, pSpawner );

		pExp = (CFExplosion *) flinklist_GetHead( &pSpawner->m_RootExplosions );
		pExpNext = NULL;

		// Remove all spawned explosions
		while( pExp ) {
			pExpNext = (CFExplosion *) flinklist_GetNext( &pSpawner->m_RootExplosions, pExp );

			pExp->Destroy();

			flinklist_Remove( &pSpawner->m_RootExplosions, pExp );
			CFExplosion::ReturnExplosionToPool( pExp );

			pExp = pExpNext;
		}

		pSpawner = pSpawnerNext;
	}
}*/

CFExplosionSpawner *CFExplosionSpawner::GetFreeSpawner( void ) {
	FASSERT( IsSpawnerSystemInitialized() );

	CFExplosionSpawner *pSpawner;

	pSpawner = (CFExplosionSpawner *) flinklist_RemoveTail( &m_RootFree );
	
	if( !pSpawner ) {
		return 0;
	}

	return pSpawner;
}

// Don't call from within CFExplosionSpawner
// This is used for when we try to spawn an explosion and already have a handle to a spawner, but the spawner isn't in the active pool
void CFExplosionSpawner::ReturnSpawner( CFExplosionSpawner *pSpawn ) {
	CFExplosionSpawner *pCurr = (CFExplosionSpawner *) flinklist_GetHead( &m_RootActive );

	while( pCurr ) {
		if( pCurr == pSpawn ) {
			// This should not happen
			DEVPRINTF( "CFExplosionSpawner::ReturnSpawner(): Someone is trying to return a spawner to the free pool when it is already in the active pool!\n" );
			DEVPRINTF( "CFExplosionSpawner::ReturnSpawner(): Something has gone terribly wrong!\n" );
			FASSERT( 0 );
		}

		pCurr = (CFExplosionSpawner *) flinklist_GetNext( &m_RootActive, pCurr );
	}

	flinklist_AddTail( &m_RootFree, pSpawn );
}

void CFExplosionSpawner::InitToDefaults( void ) {
	// Private Data
	m_uNumExplosionsLeft = 0;
	m_fTimeTillNextExplosion = 0.0f;
	m_pExplosionGroup = NULL;
	m_pLight = NULL;

	flinklist_InitRoot( &m_RootExplosions, FANG_OFFSETOF( CFExplosion, m_Link ) );
}

void CFExplosionSpawner::Spawn( CFExplosionGroup *pExplosionGroup, FExplosionSpawnParams_t *pSpawnParams, const void *pDamageImpactTriData/* = NULL*/ ) {
	FASSERT( IsSpawnerSystemInitialized() );
	// Assert on these now, we should never get in here with bad data
	FASSERT( pSpawnParams );
	FASSERT( pExplosionGroup );

	fang_MemCopy( &SpawnParams, pSpawnParams, sizeof( FExplosionSpawnParams_t ) );
	
	// Copy over the damager data if we have any
	FASSERT( m_pDamagerData );

	if( pSpawnParams->pDamager != NULL ) {
		fang_MemCopy( m_pDamagerData, pSpawnParams->pDamager, m_uDamagerSize );
	} else {
		fang_MemZero( m_pDamagerData, m_uDamagerSize );
	}

	m_pExplosionGroup = pExplosionGroup;
	m_uNumExplosionsLeft = m_pExplosionGroup->GetNumExplosions();
	m_fTimeTillNextExplosion = m_pExplosionGroup->GetExplosionDelay();

	--m_uNumExplosionsLeft;
	_SpawnExplosion( pDamageImpactTriData );

	// Add ourself to the active list
	flinklist_AddHead( &m_RootActive, this );
}

void CFExplosionSpawner::_Work( void ) {
	// NKM changed this around to not count when no explosions to spawn
	if( m_uNumExplosionsLeft > 0 ) {
		m_fTimeTillNextExplosion -= FLoop_fPreviousLoopSecs;

		if( m_fTimeTillNextExplosion < 0.0f ) {
			m_fTimeTillNextExplosion = m_pExplosionGroup->GetExplosionDelay();
			--m_uNumExplosionsLeft;
			_SpawnExplosion();
		}
	}

	CFExplosion *pExp = (CFExplosion *) flinklist_GetHead( &m_RootExplosions );
	CFExplosion *pExpNext = NULL;

	while( pExp ) {
		pExpNext = (CFExplosion *) flinklist_GetNext( &m_RootExplosions, pExp );

		pExp->Work();

		if( pExp->IsDead() ) {
			pExp->Destroy();

			flinklist_Remove( &m_RootExplosions, pExp );
			CFExplosion::ReturnExplosionToPool( pExp );
		}

		pExp = pExpNext;
	}
}

// Check uFlag for uCheck.  If it is there, clear it and set uSet
FINLINE void _BitFlagCheckClearAndSet( u32 &uFlag, const u32 &uCheck, const u32 &uSet ) {
	if( ( uFlag & uCheck ) ) {
		FMATH_SETBITMASK( uFlag, uSet );
		FMATH_CLEARBITMASK( uFlag, uCheck );
	}
}

void CFExplosionSpawner::_SpawnExplosion( const void *pDamageImpactTriData/* = NULL*/ ) {
	CFExplosion *pExp = CFExplosion::GetExplosionFromPool();

	if( !pExp ) {
		return;
	}

	CFVec3A NewPos = SpawnParams.Pos_WS;

	if( SpawnParams.UnitDir.MagSq() > 0.0001f ) {
		// If the offset range is >= 0.0f, use the normal mode, if < 0.0f we use the trace radius mode
		if( m_pExplosionGroup->GetOffsetRange() >= 0.0f ) {
			// Directional explosion
			CFVec3A Right, Up, Temp;
			f32 fDot;

			Right = CFVec3A::m_UnitAxisX;
			fDot = Right.Dot( SpawnParams.UnitDir );

			if( fDot > 0.75f || fDot < -0.75f ) {
				Right = CFVec3A::m_UnitAxisZ;
			}

			Up.Cross( SpawnParams.UnitDir, Right ).Unitize();
			Right.Cross( SpawnParams.UnitDir, Up ).Unitize();	

	//		if( !( SpawnParams.uFlags & FEXPLOSION_SPAWN_STAY_ON_PLANE ) ) {
	//			Temp = SpawnParams.UnitDir;
	//			Temp.Mul( fmath_RandomFloat() );
	//		} else {
				Temp.Zero();
	//		}

			Right.Mul( fmath_RandomBipolarUnitFloat() );
			Up.Mul( fmath_RandomBipolarUnitFloat() );
				
			Temp.Add( Right );
			Temp.Add( Up );
			Temp.Unitize();
			Temp.Mul( fmath_RandomFloatRange( 0.0f, m_pExplosionGroup->GetOffsetRange() ) );

			NewPos.Add( Temp );
		} else {
			if( m_pExplosionGroup->GetNumExplosions() > 1 ) {
				f32 fUnitVal;
				CFVec3A StartPt = SpawnParams.UnitDir;
				CFVec3A EndPt = SpawnParams.UnitDir;

				fUnitVal = 1.0f - ( (f32) m_uNumExplosionsLeft * fmath_Inv( (f32) ( m_pExplosionGroup->GetNumExplosions() - 1 ) ) );
				FMATH_CLAMP( fUnitVal, 0.0f, 1.0f );

				// Start at -radius and end at +radius
				StartPt.Mul( m_pExplosionGroup->GetOffsetRange() ).Add( SpawnParams.Pos_WS );
				EndPt.Mul( -m_pExplosionGroup->GetOffsetRange() ).Add( SpawnParams.Pos_WS );

				NewPos.Lerp( fUnitVal, StartPt, EndPt );
			} else {
				NewPos.Set( SpawnParams.UnitDir );
				NewPos.Mul( m_pExplosionGroup->GetOffsetRange() );
				NewPos.Add( SpawnParams.Pos_WS );
			}
		}
	} else {
		// "In space" explosion
		CFVec3A RandomDir;

		RandomDir.x = fmath_RandomBipolarUnitFloat();
		RandomDir.y = fmath_RandomBipolarUnitFloat();
		RandomDir.z = fmath_RandomBipolarUnitFloat();

		RandomDir.Unitize();
		RandomDir.Mul( fmath_RandomFloatRange( 0.0f, m_pExplosionGroup->GetOffsetRange() ) );

		NewPos.Add( RandomDir );
	}

	// Turn off things that we don't have valid data to use
	CFExplosionDef *pExpDef = m_pExplosionGroup->GetExplosionDef();
	
	if( pExpDef->m_uNumTextures <= 0 ) {
		FMATH_SETBITMASK( SpawnParams.uFlags, FEXPLOSION_SPAWN_NO_TEXTURE );
	}

	// Spawn it
	pExp->Spawn( &SpawnParams, m_pDamagerData, NewPos, m_pExplosionGroup->GetExplosionDef(), m_pLight, pDamageImpactTriData );

	if( ( SpawnParams.uFlags & FEXPLOSION_SPAWN_SHARELIGHT ) ) {
		m_pLight = pExp->GetLight();

		if( m_pLight ) {
			// If we got a light, keep it and re-use
			FMATH_CLEARBITMASK( SpawnParams.uFlags, FEXPLOSION_SPAWN_SHARELIGHT );
		}
	}

	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_TEXTURE,				FEXPLOSION_SPAWN_NO_TEXTURE );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_DLIGHT,					FEXPLOSION_SPAWN_NO_DLIGHT );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_EXPLOSION_DEBRIS,		FEXPLOSION_SPAWN_NO_EXPLOSION_DEBRIS );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_SOUND,					FEXPLOSION_SPAWN_NO_SOUND );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_SMOKE,					FEXPLOSION_SPAWN_NO_SMOKE );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_EXPLOSION_PARTICLE,		FEXPLOSION_SPAWN_NO_EXPLOSION_PARTICLE );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_SURFACE_DEBRIS,			FEXPLOSION_SPAWN_NO_SURFACE_DEBRIS );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_SURFACE_DUST,			FEXPLOSION_SPAWN_NO_SURFACE_DUST );
	_BitFlagCheckClearAndSet( SpawnParams.uFlags, FEXPLOSION_SPAWN_ONCE_DAMAGE,					FEXPLOSION_SPAWN_NO_DAMAGE );

	flinklist_AddHead( &m_RootExplosions, pExp );
}

BOOL CFExplosion::m_bSystemInitialized = FALSE;
FLinkRoot_t CFExplosion::m_RootFree;
FLinkRoot_t CFExplosion::m_RootActive;
CFExplosion *CFExplosion::m_pExplosionPool = NULL;
SmokeTrailAttrib_t CFExplosion::m_SmokeTrailAttrib;

BOOL CFExplosion::InitExplosionSystem( u32 uMaxExplosions ) {
	FASSERT( !IsExplosionSystemInitialized() );

	FResFrame_t ResFrame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	flinklist_InitRoot( &m_RootFree, FANG_OFFSETOF( CFExplosion, m_ExplosionLink ) );
	flinklist_InitRoot( &m_RootActive, FANG_OFFSETOF( CFExplosion, m_ExplosionLink ) );

	if( uMaxExplosions <= 0 ) {
		goto _ExitSuccessfully;
	}

	m_pExplosionPool = fnew CFExplosion[ uMaxExplosions ];
	if( m_pExplosionPool == NULL ) {
		DEVPRINTF( "CFExplosion::InitExplosionSystem(): Not enough memory to allocate explosion pool.\n" );
		goto _ExitWithError;
	}

	flinklist_InitPool( &m_RootFree, m_pExplosionPool, sizeof( CFExplosion ), uMaxExplosions );

	_InitSmokeTrail();

_ExitSuccessfully:
	// Success...
	return TRUE;

_ExitWithError:
	UninitExplosionSystem();
	fres_ReleaseFrame( ResFrame );
	m_bSystemInitialized = FALSE;

	return FALSE;
}


void CFExplosion::UninitExplosionSystem( void ) {
	if( IsExplosionSystemInitialized() ) {
		m_bSystemInitialized = FALSE;

		fdelete_array( m_pExplosionPool );
		m_pExplosionPool = NULL;

		flinklist_InitRoot( &m_RootFree, FANG_OFFSETOF( CFExplosion, m_ExplosionLink ) );
		flinklist_InitRoot( &m_RootActive, FANG_OFFSETOF( CFExplosion, m_ExplosionLink ) );

		//CFExplosionGroup::ModuleShutdown();
		//CFExplosionSpawner::UninitSpawnerSystem();
	}
}

CFExplosion *CFExplosion::GetExplosionFromPool( void ) {
	FASSERT( IsExplosionSystemInitialized() );

	CFExplosion *pExp = (CFExplosion *) flinklist_RemoveHead( &m_RootFree );

	if( !pExp ) {
		// Remove the oldest active?
		//pExp = (CFExplosion *) flinklist_RemoveTail( &m_RootActive );	
		return NULL;
	}

	if( pExp ) {
		flinklist_AddHead( &m_RootActive, pExp );
	}

	return pExp;
}

void CFExplosion::ReturnExplosionToPool( CFExplosion *pExp ) {
	FASSERT( pExp );
	FASSERT( IsExplosionSystemInitialized() );

	flinklist_Remove( &m_RootActive, pExp );
	flinklist_AddHead( &m_RootFree, pExp );
}

void CFExplosion::Draw( CFCamera *pCam ) {
	CFExplosion *pExp = (CFExplosion *) flinklist_GetHead( &m_RootActive );

	frenderer_Push( FRENDERER_DRAW, NULL );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_Depth_SetBiasLevel( 1 );

	while( pExp ) {
		if( pExp->_IsActiveTexture() || pExp->_IsActiveDust() ) {
			pExp->_Draw( pCam );
		}

		pExp = (CFExplosion *) flinklist_GetNext( &m_RootActive, pExp );
	}

	frenderer_Pop();
}

void CFExplosion::Destroy( void ) {
	_DestroyLight();
}

void CFExplosion::Spawn( FExplosionSpawnParams_t *pSpawnParams, void *pDamagerData, const CFVec3A &Pos_WS, CFExplosionDef *pExpDef, CFWorldLight *pLight/* = NULL*/, const void *pDamageImpactTriData /*= NULL*/ ) {
	FASSERT( IsExplosionSystemInitialized() );
	FASSERT( pExpDef );
	FASSERT( pSpawnParams );

	u32 uFlags = pSpawnParams->uFlags;

	// MAKE SURE THESE ARE SET UP HERE, EVERYTHING ELSE DEPENDS ON THEM
	m_Pos_WS = Pos_WS;
	m_pExpDef = pExpDef;
	m_uExplosionFlags = _FLAG_NONE;

	m_fLife = 0.0f;
	m_fLightLife = m_pExpDef->m_fLightLifetime;
	m_pLight = NULL;
	m_UnitDir = pSpawnParams->UnitDir;

	// Do we want to follow the unit dir for the explosion fireballs
	if( uFlags & FEXPLOSION_SPAWN_EXPLOSION_FOLLOW_UNIT_DIR ) {
		_EnableFollowUnitDir();
	}

	// Setup the texture
	if( !( uFlags & FEXPLOSION_SPAWN_NO_TEXTURE ) && pExpDef->m_uNumTextures > 0 ) {
		_SetupFireBalls();
	}

	// Spawn the d-light if needed
	if( !( uFlags & FEXPLOSION_SPAWN_NO_DLIGHT ) ) {
		if( m_pExpDef->m_fLightLifetime > 0.0f ) {
			m_pLight = pLight;
			_EnableLight();

			if( m_pLight == NULL ) {
				m_pLight = FLightPool_pPool->GetFromFreePool();
			} else {
				_EnableSharedLight();
			}

			if( m_pLight ) {
				FLightInit_t LightInit;
				CFVec3A PushPos = Pos_WS;

				PushPos.Add( pSpawnParams->UnitDir );

				LightInit.szName[0] = 0;
				LightInit.szPerPixelTexName[0] = 0;
				LightInit.szCoronaTexName[0] = 0;
				LightInit.nLightID = 0xffff;
				LightInit.nFlags = FLIGHT_FLAG_ENABLE | FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_PER_PIXEL;
				LightInit.nType = FLIGHT_TYPE_OMNI;
				LightInit.nParentBoneIdx = -1;
				LightInit.fIntensity = 1.0f;
				LightInit.Motif.Set( m_pExpDef->m_fLightRed, m_pExpDef->m_fLightGreen, m_pExpDef->m_fLightBlue, 1.0f );
				LightInit.Motif.SetIndex( fcolor_GetRandomMotif( m_pExpDef->m_nLightMotif ) );
				LightInit.spInfluence.m_Pos = PushPos.v3;
				LightInit.spInfluence.m_fRadius = m_pExpDef->m_fLightRadius;
				LightInit.mtxOrientation.Identity();
				LightInit.mtxOrientation.m_vPos = PushPos.v3;
				LightInit.fSpotInnerRadians = 0.0f;
				LightInit.fSpotOuterRadians = 0.0f;
				LightInit.fCoronaScale = 0.0f;

				m_pLight->Init( &LightInit );
			} else {
				_DisableLight();
			}
		} else {
			_DisableLight();
		}
	}

	// Spawn the debris for the explosion (NOT SURFACE DEBRIS)
	if( !( uFlags & FEXPLOSION_SPAWN_NO_EXPLOSION_DEBRIS ) ) {
		_SpawnExplosionDebris( &Pos_WS, &pSpawnParams->UnitDir );		
	}

	// Spawn the sound
	if( !( uFlags & FEXPLOSION_SPAWN_NO_SOUND ) ) {
		CFSoundGroup::PlaySound( m_pExpDef->m_pSoundGroup, FALSE, &Pos_WS, pSpawnParams->nNoisyPlayerIndex, TRUE );
	}

	// Enable the smoke if we have it
	if( !( uFlags & FEXPLOSION_SPAWN_NO_SMOKE ) ) {
		if( _IsActiveTexture() ) {
			_EnableSmoke();
		} else {
			_EnableSingleSmoke();
		}
	}

	// Spawn the explosion particles
	if( !( uFlags & FEXPLOSION_SPAWN_NO_EXPLOSION_PARTICLE ) ) {
		if( m_pExpDef->m_hExplosionParticleDef != FPARTICLE_INVALID_HANDLE ) {
			if( _IsFollowUnitDir() ) {
				fparticle_SpawnEmitter( m_pExpDef->m_hExplosionParticleDef, Pos_WS.v3, &pSpawnParams->UnitDir.v3, 1.0f );
			} else {
				fparticle_SpawnEmitter( m_pExpDef->m_hExplosionParticleDef, Pos_WS.v3, &CFVec3A::m_UnitAxisY.v3, 1.0f );
			}
		}
	}

	// Create decal if we have one
	if( m_pExpDef->m_hDecalDef != FDECALDEF_INVALID_HANDLE ) {
		if( _IsFollowUnitDir() ) {
			CFDecal::Create( m_pExpDef->m_hDecalDef, Pos_WS, &pSpawnParams->UnitDir );
		} else {
			CFDecal::Create( m_pExpDef->m_hDecalDef, Pos_WS, &CFVec3A::m_UnitAxisY );
		}
	}

	// !!Nate - TODO
	// Need to add a size to bots so that when we spawn an explosion on a bot we can override
	// the size that is given in the CSV.
#if 0
	// Spawn surface debris
	if( !( uFlags & FEXPLOSION_SPAWN_NO_SURFACE_DEBRIS ) && _pSurfaceDebrisCallback ) {
		_SpawnSurfaceDebris( pSpawnParams, &Pos_WS, pExpDef->m_eDebrisSize1 );
		_SpawnSurfaceDebris( pSpawnParams, &Pos_WS, pExpDef->m_eDebrisSize2 );
	}
#endif

#if 0
	// Spawn surface dust
	if( !( uFlags & FEXPLOSION_SPAWN_NO_SURFACE_DUST ) ) {
		if( _pSurfaceDustTextureCallback ) {
			_SpawnSurfaceDust( pSpawnParams );
		} else {
			_DisableDust();
		}
	}
#endif

	// Submit some damage
	if( _pDamageCallback && !( uFlags & FEXPLOSION_SPAWN_NO_DAMAGE )) {
		if( pSpawnParams->pDamageProfile == NULL ) {
			_pDamageCallback( pExpDef->m_pDamageProfile, pSpawnParams->pDamager ? pDamagerData : NULL, &m_Pos_WS, pDamageImpactTriData );
		} else {
			// NKM - Changed this from using pSpawnParams->pDamager since this can go out of scope
			//		 Use pDamagerData since it won't go out of scope.
			_pDamageCallback( pSpawnParams->pDamageProfile, pSpawnParams->pDamager ? pDamagerData : NULL, &m_Pos_WS, pDamageImpactTriData );
		}
	}
}

#define _LIGHT_START_FADE_UNIT_TIME		( 0.75f )	// (0.0 - 1.0f) ( exclusive )
#define _LIGHT_START_FADE_OO_RANGE_SIZE ( 4.0f )	// 1.0f * fmath_Inv( 1.0f - _LIGHT_START_FADE_UNIT_TIME )

void CFExplosion::Work( void ) {
	// Keep track of how long we have lived
	m_fLife += FLoop_fPreviousLoopSecs;

	// NKM - Added the _IsActiveSmoke() so that we can still get smoke after texture has died
	//		 What happens to the texture doesn't matter since it is disabled.
	if( _IsActiveTexture() || _IsActiveSmoke() ) {
		u32 uMaxNum = 3;
		u32 uNewFrame;
		u32 uNumDead = 0;
		u32 uNumSmoked = 0;
		f32 fCnt = 0.0f;
		CFVec3A AvgPos;

		AvgPos.Zero();

		if( _IsSingleExplosionTex() ) {
			uMaxNum = 1;
		}

		for( u32 i = 0; i < uMaxNum; ++i ) {
			if( m_ExplosionTexInfo[i].fExplosionAnimTimeOn >= 0.0f ) {
				// Animate the texture
				uNewFrame = (u32) ( m_pExpDef->m_fAnimRate * m_ExplosionTexInfo[i].fExplosionAnimTimeOn );
				uNewFrame %= m_pExpDef->m_uNumTextures;

				m_ExplosionTexInfo[i].uExplosionAnimFrame = uNewFrame;
				m_ExplosionTexInfo[i].fExplosionAnimTimeOn += FLoop_fPreviousLoopSecs;

				if( m_ExplosionTexInfo[i].fExplosionAnimTimeOn >= m_pExpDef->m_fExplosionTexLife ) {
					++uNumDead;
				}

				// If we want smoke
				if( _IsActiveSmoke() ) {
					if( m_ExplosionTexInfo[i].fTimeTillSmoke != -1.0f ) {
						m_ExplosionTexInfo[i].fTimeTillSmoke -= FLoop_fPreviousLoopSecs;
						FMATH_CLAMPMIN( m_ExplosionTexInfo[i].fTimeTillSmoke, 0.0f );
					}

					if( m_ExplosionTexInfo[i].fTimeTillSmoke == 0.0f ) {
						if( m_pExpDef->m_hSmokeParticleDef != FPARTICLE_INVALID_HANDLE ) {
							fparticle_SpawnEmitter( m_pExpDef->m_hSmokeParticleDef, 
													m_ExplosionTexInfo[i].Pos_WS.v3, 
													&CFVec3A::m_UnitAxisY.v3, 
													m_pExpDef->m_fSmokeUnitIntensity );
						}

						m_ExplosionTexInfo[i].fTimeTillSmoke = -1.0f;
					}

					if( m_ExplosionTexInfo[i].fTimeTillSmoke == -1.0f ) {
						++uNumSmoked;
					}
				}

				fCnt += 1.0f;
				AvgPos.Add( m_ExplosionTexInfo[i].Pos_WS );
			} else {
				m_ExplosionTexInfo[i].fExplosionAnimTimeOn += FLoop_fPreviousLoopSecs;
			}

		}

		if( uNumDead == uMaxNum ) {
			_DisableTexture();
			_DisableSingleExplosionTex();
		}

		if( uNumSmoked == uMaxNum ) {
			_DisableSmoke();
		}

		// Move the light to the average position of the fireballs if we have a light
		if( _IsActiveLight() && fCnt > 0.0f ) {
			AvgPos.Mul( fmath_Inv( fCnt ) );
			m_pLight->m_Light.m_mtxOrientation_WS.m_vPos = AvgPos.v3;
			m_pLight->UpdateTracker();
		}
	}

	if( _IsActiveLight() ) {
		FASSERT( m_pLight ); // If we don't have a light pointer, something has gone wrong
        
		f32 fUnitLife = 1.0f - ( m_fLightLife * fmath_Inv( m_pExpDef->m_fLightLifetime ) );

		// Fade out the light quickly
		if( fUnitLife >= _LIGHT_START_FADE_UNIT_TIME ) {
			fUnitLife -= _LIGHT_START_FADE_UNIT_TIME;
			FMATH_CLAMPMIN( fUnitLife, 0.0f );
			fUnitLife *= _LIGHT_START_FADE_OO_RANGE_SIZE;

			m_pLight->m_Light.SetIntensity( FMATH_FPOT( fUnitLife, 1.0f, 0.0f ) );
		}

		m_fLightLife -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fLightLife, 0.0f );

		if( m_fLightLife == 0.0f ) {
			_DestroyLight();
		}
	}

	// NKM - Added so that when we don't have a texture, but we want smoke we can still get smoke.
	//		 The smoke will come from the epicenter of the explosion.
	if( _IsSingleSmoke() ) {
		if( m_fLife > m_pExpDef->m_fSmokeParticleDelay ) {
			_DisableSingleSmoke();

			if( m_pExpDef->m_hSmokeParticleDef != FPARTICLE_INVALID_HANDLE ) {
				fparticle_SpawnEmitter( m_pExpDef->m_hSmokeParticleDef, 
										m_Pos_WS.v3, 
										&CFVec3A::m_UnitAxisY.v3, 
										m_pExpDef->m_fSmokeUnitIntensity );
			}
		}
	}
}

void CFExplosion::_InitSmokeTrail( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );

	if( pTexDef == NULL ) {
		DEVPRINTF( "CFExplosion::_InitSmokeTrail(): Could not load smoke trail texture.\n" );
	}

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 1.0f;
	m_SmokeTrailAttrib.fScaleMax_WS = 1.3f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.3f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.6f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.7f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_SmokeTrailAttrib.EndColorRGB.Set( 0.7f, 0.7f, 0.7f );
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.0f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.2f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
}

static FINLINE void _BuildTriStripFace( FDrawVtx_t *pPts, const CFVec3A &Right, const CFVec3A &Up ) {
	pPts[0].Pos_MS.Zero();
	pPts[0].Pos_MS.Sub( Right.v3 );
	pPts[0].Pos_MS.Sub( Up.v3 );

	pPts[1].Pos_MS.Zero();
	pPts[1].Pos_MS.Sub( Right.v3 );
	pPts[1].Pos_MS.Add( Up.v3 );

	pPts[2].Pos_MS.Zero();
	pPts[2].Pos_MS.Add( Right.v3 );
	pPts[2].Pos_MS.Sub( Up.v3 );

	pPts[3].Pos_MS.Zero();
	pPts[3].Pos_MS.Add( Right.v3 );
	pPts[3].Pos_MS.Add( Up.v3 );
}

#define _FIRE_START_FADE_UNIT_TIME		( 0.6f )	// (0.0 - 1.0f) ( exclusive )
#define _FIRE_START_FADE_OO_RANGE_SIZE	( 2.5f )	// 1.0f * fmath_Inv( 1.0f - _FIRE_START_FADE_UNIT_TIME )
#define _FIRE_SIZE_RAMP_UP_SCALE		( 2.0f )

void CFExplosion::_Draw( CFCamera *pCam ) {
	CFVec3A Right, Up;
	CFVec3A Pos, Dir;
	f32 fSize;
	f32 fAlpha;
	f32 fUnitLife;
	CFXfm Xfm;
	FDrawVtx_t *pDrawVerts = fvtxpool_GetArray( 4 );

	if( pDrawVerts == NULL || pCam == NULL) {
		return;
	}

	pDrawVerts[0].ST.Set( 0.0f, 1.0f );
	pDrawVerts[1].ST.Set( 0.0f, 0.0f );
	pDrawVerts[2].ST.Set( 1.0f, 1.0f );
	pDrawVerts[3].ST.Set( 1.0f, 0.0f );

	pDrawVerts[0].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	pDrawVerts[1].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	pDrawVerts[2].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	pDrawVerts[3].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f );

	if( _IsActiveTexture() ) {
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		// Draw the explosion fireballs
		for( u32 i = 0; i < 3; ++i ) {
			if( m_ExplosionTexInfo[i].fExplosionAnimTimeOn >= 0.0f ) {
				fUnitLife = m_ExplosionTexInfo[i].fExplosionAnimTimeOn * fmath_Inv( m_pExpDef->m_fExplosionTexLife );
				fAlpha = fUnitLife;

				// Fade out the explosion quickly
				if( fAlpha >= _FIRE_START_FADE_UNIT_TIME ) {
					fAlpha -= _FIRE_START_FADE_UNIT_TIME;
					FMATH_CLAMPMIN( fAlpha, 0.0f );
					fAlpha *= _FIRE_START_FADE_OO_RANGE_SIZE;
					fAlpha = FMATH_FPOT( fAlpha, 1.0f, 0.0f );
					FMATH_CLAMP( fAlpha, 0.0f, 1.0f );
				} else {
					fAlpha = 1.0f;
				}

				Right = pCam->GetFinalXfm()->m_MtxR.m_vRight;
				Up = pCam->GetFinalXfm()->m_MtxR.m_vUp;

				fSize = FMATH_FPOT( FMATH_MIN( fUnitLife * _FIRE_SIZE_RAMP_UP_SCALE, 1.0f ), 0.0f, m_pExpDef->m_fExplosionTexSize );

				Right.Mul( fSize * 0.5f );
				Up.Mul( fSize * 0.5f );

				_BuildTriStripFace( pDrawVerts, Right, Up );

				pDrawVerts[0].ColorRGBA.fAlpha = fAlpha;
				pDrawVerts[1].ColorRGBA.fAlpha = fAlpha;
				pDrawVerts[2].ColorRGBA.fAlpha = fAlpha;
				pDrawVerts[3].ColorRGBA.fAlpha = fAlpha;

				Xfm.BuildTranslation( m_ExplosionTexInfo[i].Pos_WS );
				Xfm.PushModel();
				fdraw_SetTexture( m_pExpDef->m_apTextures[m_ExplosionTexInfo[i].uExplosionAnimFrame] );
				fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pDrawVerts, 4 );	
				Xfm.PopModel();
			}
		}
	}

	if( _IsActiveDust() ) {
		// Draw the dust puffs
		u32 uNumDead;
		f32 fHalfSize;
		f32 fUnitLife;
		f32 fGravity;
		
 		uNumDead = 0;
		
		fGravity = ( _DUST_GRAVITY * FLoop_fPreviousLoopSecs );

 		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		// !!Nate - HACK until we have a real dust texture
		pDrawVerts[0].ColorRGBA.Set( 0.5f, 0.25f, 0.0f, 1.0f );
		pDrawVerts[1].ColorRGBA.Set( 0.5f, 0.25f, 0.0f, 1.0f );
		pDrawVerts[2].ColorRGBA.Set( 0.5f, 0.25f, 0.0f, 1.0f );
		pDrawVerts[3].ColorRGBA.Set( 0.5f, 0.25f, 0.0f, 1.0f );

		for( u32 i = 0; i < _NUM_DUST_PUFFS; ++i ) {
			if( m_DustPuffs[i].fDustLife >= m_pExpDef->m_fDustLife ) {
				++uNumDead;
				continue;
			}

			// Get directions
			Right = pCam->GetFinalXfm()->m_MtxR.m_vRight;
			Up = pCam->GetFinalXfm()->m_MtxR.m_vUp;

  			fUnitLife = m_DustPuffs[i].fDustLife * fmath_Inv( m_pExpDef->m_fDustLife );
 			fAlpha = FMATH_MIN( fmath_Sin( fUnitLife * FMATH_PI ), 0.2f );

			m_DustPuffs[i].fDustSize = FMATH_FPOT( fUnitLife, 0.0f, m_pExpDef->m_fDustSizeEnd );

			fHalfSize = m_DustPuffs[i].fDustSize * 0.5f;
			Right.Mul( fHalfSize );
			Up.Mul( fHalfSize );

			_BuildTriStripFace( pDrawVerts, Right, Up );

			m_DustPuffs[i].Pos_WS.y += ( m_DustPuffs[i].fDustVelY * FLoop_fPreviousLoopSecs );
			m_DustPuffs[i].fDustVelY += fGravity;

			pDrawVerts[0].ColorRGBA.fAlpha = fAlpha;
			pDrawVerts[1].ColorRGBA.fAlpha = fAlpha;
			pDrawVerts[2].ColorRGBA.fAlpha = fAlpha;
			pDrawVerts[3].ColorRGBA.fAlpha = fAlpha;

			Xfm.BuildTranslation( m_DustPuffs[i].Pos_WS );
			Xfm.PushModel();
			fdraw_SetTexture( (CFTexInst *) m_pDustTexture );
			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pDrawVerts, 4 );	
			Xfm.PopModel();

			m_DustPuffs[i].fDustLife += FLoop_fPreviousLoopSecs;

			FMATH_CLAMPMAX( m_DustPuffs[i].fDustLife, m_pExpDef->m_fDustLife );
		}

		if( uNumDead == _NUM_DUST_PUFFS ) {
			_DisableDust();
		}
	}

	fvtxpool_ReturnArray( pDrawVerts );
}

void CFExplosion::_DestroyLight( void ) {
	if( m_pLight ) {
		m_pLight->RemoveFromWorld();

		if( !_IsSharedLight() ) {
			FLightPool_pPool->ReturnToFreePool( (CFWorldLightItem *)m_pLight );
		}

		m_pLight = NULL;
		_DisableLight();
		_DisableSharedLight();
	}
}

// _DELAY_TIME1 and 2 are for stacked explosions
// _DELAY_TIME3 is for explosions that are not stacked
#define _DELAY_TIME1				( -0.06f )
#define _DELAY_TIME2				( -0.2f )
#define _DELAY_TIME3				( -0.1f )

void CFExplosion::_SetupFireBalls( void ) {
	f32 fMax;
	CFVec3A Push;

	_EnableTexture();

	// See if they only want a single explosion texture
	if( !m_pExpDef->m_bExplosionMultiple ) {
		_EnableSingleExplosionTex();
	}

	m_ExplosionTexInfo[0].fTimeTillSmoke = m_pExpDef->m_fSmokeParticleDelay;
	m_ExplosionTexInfo[1].fTimeTillSmoke = m_pExpDef->m_fSmokeParticleDelay;
	m_ExplosionTexInfo[2].fTimeTillSmoke = m_pExpDef->m_fSmokeParticleDelay;

	fMax = FMATH_MAX( FMATH_FABS( m_UnitDir.x ), FMATH_MAX( FMATH_FABS( m_UnitDir.y ), FMATH_FABS( m_UnitDir.z ) ) );

	m_ExplosionTexInfo[0].Pos_WS = m_Pos_WS;

	// NKM
	// If we want to follow the unit direction, we assume the caller has placed the explosion correctly
	if( fMax == FMATH_FABS( m_UnitDir.y ) || _IsFollowUnitDir() ) {
		Push = m_UnitDir;
		Push.Mul( m_pExpDef->m_fExplosionTexSize );
		Push.Mul( ( 0.5f * m_pExpDef->m_fTexPushAmntGround ) );
		
		m_ExplosionTexInfo[0].Pos_WS.Add( Push );

		Push = m_UnitDir;
		Push.Mul( m_pExpDef->m_fExplosionTexSize );
		Push.Mul( ( 0.5f * m_pExpDef->m_fTexPushOnAxis ) );

		m_ExplosionTexInfo[1].Pos_WS = m_ExplosionTexInfo[0].Pos_WS;
		m_ExplosionTexInfo[1].Pos_WS.Add( Push );

		m_ExplosionTexInfo[2].Pos_WS = m_ExplosionTexInfo[1].Pos_WS;
		m_ExplosionTexInfo[2].Pos_WS.Add( Push );
		
		m_ExplosionTexInfo[1].fExplosionAnimTimeOn = _DELAY_TIME1;
		m_ExplosionTexInfo[2].fExplosionAnimTimeOn = _DELAY_TIME2;
	} else {
		Push = m_UnitDir;
		Push.Mul( m_pExpDef->m_fExplosionTexSize );
		Push.Mul( ( 0.5f * m_pExpDef->m_fTexPushAmntWall ) );
		
		m_ExplosionTexInfo[0].Pos_WS.Add( Push );

		Push = CFVec3A::m_UnitAxisY;
		Push.Mul( m_pExpDef->m_fExplosionTexSize );
		Push.Mul( ( 0.5f * m_pExpDef->m_fTexPushOnAxis ) );

		m_ExplosionTexInfo[1].Pos_WS = m_ExplosionTexInfo[0].Pos_WS;
		m_ExplosionTexInfo[1].Pos_WS.Add( Push );

		m_ExplosionTexInfo[2].Pos_WS = m_ExplosionTexInfo[0].Pos_WS;
		m_ExplosionTexInfo[2].Pos_WS.Sub( Push );

		m_ExplosionTexInfo[1].fExplosionAnimTimeOn = _DELAY_TIME3;
		m_ExplosionTexInfo[2].fExplosionAnimTimeOn = _DELAY_TIME3;
	}

	m_ExplosionTexInfo[0].uExplosionAnimFrame = 0;
	m_ExplosionTexInfo[0].fExplosionAnimTimeOn = 0.0f;

	// fAnimTimeOn is set above
	m_ExplosionTexInfo[1].uExplosionAnimFrame = 0;
	m_ExplosionTexInfo[2].uExplosionAnimFrame = 0;
}

#define _PART_NUM_MIN				( 4 )
#define _PART_NUM_MAX				( 6 )

void CFExplosion::_SpawnSurfaceDebris( FExplosionSpawnParams_t *pSpawnParams, const CFVec3A *pPos_WS, FExplosionSurfceDebrisSize_e eSize ) {
	FASSERT( pPos_WS );
	FASSERT( pSpawnParams );
	FASSERT( _pSurfaceDebrisCallback );

	if( eSize == FEXPLOSION_SIZE_NONE ) {
		return;
	}

	CFDebrisSpawner DebrisSpawner;

	DebrisSpawner.InitToDefaults();

	DebrisSpawner.m_Mtx.m_vPos = *pPos_WS;
	DebrisSpawner.m_Mtx.m_vZ = pSpawnParams->UnitDir;
	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
	//DebrisSpawner.m_pDebrisGroup !! This will be set by the callback!! 
	DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
	DebrisSpawner.m_fMinSpeed = 25.0f;
	DebrisSpawner.m_fMaxSpeed = 35.0f;
	DebrisSpawner.m_fUnitDirSpread = 1.0f;

	DebrisSpawner.m_fScaleMul = 1.0f;
	DebrisSpawner.m_fGravityMul = 1.0f;
	DebrisSpawner.m_fRotSpeedMul = 1.0f;
	DebrisSpawner.m_nMinDebrisCount = _PART_NUM_MIN;
	DebrisSpawner.m_nMaxDebrisCount = _PART_NUM_MAX;
	DebrisSpawner.m_pFcnCallback = NULL;

	_pSurfaceDebrisCallback( &DebrisSpawner, eSize, pSpawnParams->uSurfaceType );
}

#define _CEILING_VEL_SCALE ( 0.5f )

void CFExplosion::_SpawnExplosionDebris( const CFVec3A *pPos_WS, const CFVec3A *pUnitUp ) {
	FASSERT( pPos_WS );
	FASSERT( pUnitUp );

	if( m_pExpDef->m_pDebrisGroup == NULL ) {
		return;
	}

	CFDebrisSpawner DebrisSpawner;
	f32 fAbsY, fMax;

	DebrisSpawner.InitToDefaults();

	DebrisSpawner.m_Mtx.m_vPos = *pPos_WS;
	DebrisSpawner.m_Mtx.m_vZ = *pUnitUp;
	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
	DebrisSpawner.m_pDebrisGroup = m_pExpDef->m_pDebrisGroup;
	DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
	DebrisSpawner.m_fMinSpeed = m_pExpDef->m_fMinSpeed;
	DebrisSpawner.m_fMaxSpeed = m_pExpDef->m_fMaxSpeed;
	DebrisSpawner.m_pFcnCallback = NULL;

	if( pUnitUp->MagSq() > 0.001f ) {
		DebrisSpawner.m_fUnitDirSpread = m_pExpDef->m_fUnitDirSpread;
	} else {
		// Give full spread since we are in space and not on a surface
		DebrisSpawner.m_fUnitDirSpread = 1.0f;
	}

	fAbsY = FMATH_FABS( pUnitUp->y );
	fMax = FMATH_MAX( FMATH_FABS( pUnitUp->x ), fAbsY );
	fMax = FMATH_MAX( fMax, FMATH_FABS( pUnitUp->z ) );

	// We have a surface that is major Y, scale down our velocities a bit so it looks good
	if( ( fAbsY == fMax ) && pUnitUp->y < 0.0f ) {
		DebrisSpawner.m_fMinSpeed = m_pExpDef->m_fMinSpeed * _CEILING_VEL_SCALE;
		DebrisSpawner.m_fMaxSpeed = m_pExpDef->m_fMaxSpeed * _CEILING_VEL_SCALE;
	}

	if( m_pExpDef->m_uMinDebrisNormal != 0 || m_pExpDef->m_uMaxDebrisNormal != 0 ) {
		DebrisSpawner.m_fScaleMul = m_pExpDef->m_fScaleMul;
		DebrisSpawner.m_fGravityMul = m_pExpDef->m_fGravityMul;
		DebrisSpawner.m_fRotSpeedMul = m_pExpDef->m_fRotSpeedMul;
		DebrisSpawner.m_nMinDebrisCount = m_pExpDef->m_uMinDebrisNormal;
		DebrisSpawner.m_nMaxDebrisCount = m_pExpDef->m_uMaxDebrisNormal;
		DebrisSpawner.Spawn();
	}

	if( m_pExpDef->m_uMinDebrisFire != 0 || m_pExpDef->m_uMaxDebrisFire != 0 ) {
		DebrisSpawner.m_nMinDebrisCount = m_pExpDef->m_uMinDebrisFire;
		DebrisSpawner.m_nMaxDebrisCount = m_pExpDef->m_uMaxDebrisFire;
		DebrisSpawner.m_pSmokeTrailAttrib = &m_SmokeTrailAttrib;
		DebrisSpawner.m_hParticleDef1 = m_pExpDef->m_hSmokeDebrisParticleDef;
		DebrisSpawner.m_hParticleDef2 = m_pExpDef->m_hFireDebrisParticleDef;

		FMATH_SETBITMASK( DebrisSpawner.m_nFlags, CFDebrisSpawner::FLAG_PLAY_FLAMING_SOUND | CFDebrisSpawner::FLAG_PLAY_IMPACT_SOUND );

		DebrisSpawner.Spawn();
	}
}

void CFExplosion::_SpawnSurfaceDust( FExplosionSpawnParams_t *pSpawnParams ) {
	FASSERT( _pSurfaceDustTextureCallback );

	// Don't spawn dust on surfaces where Y isn't the major axis
	CFVec3A Dir = pSpawnParams->UnitDir;
	
	Dir.x = FMATH_FABS( Dir.x );
	Dir.y = FMATH_FABS( Dir.y );
	Dir.z = FMATH_FABS( Dir.z );

	if( FMATH_MAX( Dir.x, Dir.z ) > Dir.y ) {
		_DisableDust();
		return;
	}

	m_pDustTexture = _pSurfaceDustTextureCallback( pSpawnParams->uSurfaceType );

	if( !m_pDustTexture || !m_pDustTexture->GetTexDef() ) {
		_DisableDust();
		return;
	}

	if( m_pExpDef->m_fDustLife <= 0.0f ) {
		_DisableDust();
		return;
	}

	CFVec3A JitterAmnt;

	for( u32 i = 0; i < _NUM_DUST_PUFFS; ++i ) {
		JitterAmnt.x = fmath_RandomBipolarUnitFloat() * fmath_RandomFloatRange( 1.0f, 5.0f );
		JitterAmnt.y = fmath_RandomBipolarUnitFloat() * fmath_RandomFloatRange( 1.0f, 5.0f );
		JitterAmnt.z = fmath_RandomBipolarUnitFloat() * fmath_RandomFloatRange( 1.0f, 5.0f );

		// So we can have some dust that is ahead of the others
		m_DustPuffs[i].fDustLife = fmath_RandomFloatRange( 0.0f, m_pExpDef->m_fDustLife * 0.2f );
		m_DustPuffs[i].fDustSize = 0.0f;
		m_DustPuffs[i].fDustVelY = m_pExpDef->m_fDustUpVel;
		m_DustPuffs[i].Pos_WS = m_Pos_WS;
		m_DustPuffs[i].Pos_WS.Add( JitterAmnt );
	}

	_EnableDust();
}


FINLINE CFExplosionGroup *_GetGroupDefPtr( FExplosion_GroupHandle_t hHandle ) {
	CFExplosionGroup *pDef = (CFExplosionGroup *) hHandle;

	// If you hit this assert, you passed an invalid handle to the explosion system
	FASSERT( pDef->m_uIndexInList < CFExplosionGroup::m_LinkRoot.nCount );

	return pDef;
}

FINLINE CFExplosionSpawner *_GetSpawnerDefPtr( FExplosion_SpawnerHandle_t hHandle ) {
	CFExplosionSpawner *pSpawn = (CFExplosionSpawner *) hHandle;

	if( !pSpawn || pSpawn->m_uSignature != _SPAWNER_SIGNATURE ) {
		return NULL;
	}

	return pSpawn;
}

BOOL fexplosion_ModuleStartup( void ) {
	if( !CFExplosionSet::ModuleStartup() ) {
		DEVPRINTF( "fexplosion_ModuleStartup(): Failed to startup CFExplosionSet.\n" );
		return FALSE;
	}

	if( !CFExplosionGroup::ModuleStartup() ) {
		DEVPRINTF( "fexplosion_ModuleStartup(): Failed to startup CFExplosionGroup.\n" );
		return FALSE;
	}

	return TRUE;
}

void fexplosion_ModuleShutdown( void ) {
	CFExplosionSet::ModuleShutdown();
	CFExplosionGroup::ModuleShutdown();

	CFExplosionSpawner::UninitSpawnerSystem();
	CFExplosion::UninitExplosionSystem();
}

BOOL fexplosion_LoadExplosionData( cchar *pszExplosionSet, cchar *pszExplosionGroups ) {
	if( !CFExplosionGroup::LoadAll( pszExplosionSet, pszExplosionGroups ) ) {
		return FALSE;
	}

	return TRUE;
}

BOOL fexplosion_InitExplosionSystem( const u32 &uMaxExplosions, const u32 &uMaxSpawners, const u32 &uDamagerSize ) {
	if( !CFExplosionSpawner::InitSpawnerSystem( uMaxSpawners, uDamagerSize ) ) {
		goto _ExitWithError;
	}

	if( !CFExplosion::InitExplosionSystem( uMaxExplosions ) ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}

void fexplosion_UninitExplosionSystem( void ) {
	CFExplosionSpawner::UninitSpawnerSystem();
	CFExplosion::UninitExplosionSystem();
}

void fexplosion_SetSurfaceDebrisCallback( ExplosionDebrisCallback_t *pCallback ) {
	_pSurfaceDebrisCallback = pCallback;
}

void fexplosion_SetSurfaceDustTextureCallback( ExplosionDustTextureCallback_t *pCallback ) {
	_pSurfaceDustTextureCallback = pCallback;
}

void fexplosion_SetDamageCallback( ExplosionDamageCallback_t *pCallback ) {
	_pDamageCallback = pCallback;
}

void fexplosion_Work( void ) {
	CFExplosionSpawner::Work();
}

void fexplosion_Draw( CFCamera *pCam ) {
	CFExplosion::Draw( pCam );
}

FExplosion_GroupHandle_t fexplosion_GetExplosionGroup( cchar *pszGroupName ) { 
	return CFExplosionGroup::Find( pszGroupName );
}

FExplosion_SpawnerHandle_t fexplosion_GetExplosionSpawner( void ) {
	CFExplosionSpawner *pSpawn = CFExplosionSpawner::GetFreeSpawner();
	
	if( pSpawn ) {
		pSpawn->InitToDefaults();
	}

	return pSpawn;
}

FExplosionSurfceDebrisSize_e fexplosion_GetSurfaceDebrisSize1( FExplosion_GroupHandle_t hGroup ) {
	CFExplosionGroup *pGroup = _GetGroupDefPtr( hGroup );

	if( !pGroup ) {
		return FEXPLOSION_SIZE_NONE;
	}

	if( pGroup->GetExplosionDef() ) {
		return pGroup->GetExplosionDef()->m_eDebrisSize1;
	}
	
	return FEXPLOSION_SIZE_NONE;
}

FExplosionSurfceDebrisSize_e fexplosion_GetSurfaceDebrisSize2( FExplosion_GroupHandle_t hGroup ) {
	CFExplosionGroup *pGroup = _GetGroupDefPtr( hGroup );

	if( !pGroup ) {
		return FEXPLOSION_SIZE_NONE;
	}

	if( pGroup->GetExplosionDef() ) {
		return pGroup->GetExplosionDef()->m_eDebrisSize2;
	}
	
	return FEXPLOSION_SIZE_NONE;
}

void fexplosion_SpawnExplosion( FExplosion_SpawnerHandle_t hSpawner, 
								FExplosion_GroupHandle_t hGroup, 
								FExplosionSpawnParams_t *pSpawnParams,
								const void *pDamageImpactTriData/* = NULL*/ ) {
	if( hSpawner == FEXPLOSION_INVALID_HANDLE ) {
		DEVPRINTF( "fexplosion_SpawnExplosion(): Called with invalid explosion spawner handle.\n" );
		return;
	}

	CFExplosionSpawner *pSpawner = _GetSpawnerDefPtr( hSpawner );

	if( hGroup == FEXPLOSION_INVALID_HANDLE ) {
		DEVPRINTF( "fexplosion_SpawnExplosion(): Called with invalid group handle.\n" );

		// If we happen to get called with a bad group handle, return the spawner
		CFExplosionSpawner::ReturnSpawner( pSpawner );
		return;
	}

	pSpawner->Spawn( _GetGroupDefPtr( hGroup ), pSpawnParams, pDamageImpactTriData );

	LiquidSystem.SpawnExplosion( pSpawnParams->Pos_WS, 1.0f );
}

/*void fexplosion_KillAll( void ) {
	CFExplosionSpawner::KillAll();
}*/