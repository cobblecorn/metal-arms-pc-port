//////////////////////////////////////////////////////////////////////////////////////
// fexplosion.h - 
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
#ifndef _FEXPLOSION_H_
#define _FEXPLOSION_H_ 1

#include "fang.h"
#include "fsndfx.h"
#include "fcamera.h"
#include "fdebris.h"
#include "flinklist.h"
#include "fparticle.h"
#include "flightpool.h"

enum FExplosionSurfceDebrisSize_e {
	FEXPLOSION_SIZE_NONE = 0,
	FEXPLOSION_SIZE_SMALL,
	FEXPLOSION_SIZE_MEDIUM,
	FEXPLOSION_SIZE_LARGE,
};

typedef void * FExplosion_GroupHandle_t;
typedef void * FExplosion_SpawnerHandle_t;

typedef void ExplosionDebrisCallback_t( CFDebrisSpawner *pSpawner, FExplosionSurfceDebrisSize_e eDebrisSize, u16 uSurfaceType );
typedef const CFTexInst *ExplosionDustTextureCallback_t( u16 uSurfaceType );
typedef void ExplosionDamageCallback_t( const void *pDamage, const void *pDamager, const CFVec3A *pPos, const void *pDamageImpactTriData );

#define FEXPLOSION_INVALID_HANDLE					( NULL )

#define FEXPLOSION_SPAWN_NONE						( 0x00000000 )
#define FEXPLOSION_SPAWN_SHARELIGHT					( 0x00000001 )	// With explosion groups, share a single light among all explosions
#define FEXPLOSION_SPAWN_STAY_ON_PLANE				( 0x00000002 )	// !!Nate - This is not used, take out
#define FEXPLOSION_SPAWN_NO_TEXTURE					( 0x00000004 )	// No explosion texture
#define FEXPLOSION_SPAWN_NO_DLIGHT					( 0x00000008 )	// No d-light
#define FEXPLOSION_SPAWN_NO_EXPLOSION_DEBRIS		( 0x00000010 )	// No explosion debris
#define FEXPLOSION_SPAWN_NO_SOUND					( 0x00000020 )	// No sound
#define FEXPLOSION_SPAWN_NO_SMOKE					( 0x00000040 )	// No smoke particles
#define FEXPLOSION_SPAWN_NO_EXPLOSION_PARTICLE		( 0x00000080 )	// No explosion particles
#define FEXPLOSION_SPAWN_NO_SURFACE_DEBRIS			( 0x00000100 )	// No surface debris
#define FEXPLOSION_SPAWN_NO_SURFACE_DUST			( 0x00000200 )	// No surface dust
#define FEXPLOSION_SPAWN_NO_DAMAGE					( 0x00000400 )	// No damage given out by the explosion
#define FEXPLOSION_SPAWN_ONCE_TEXTURE				( 0x00000800 )	// With groups, only spawn texture once
#define FEXPLOSION_SPAWN_ONCE_DLIGHT				( 0x00001000 )	// With groups, only spawn d-light once
#define FEXPLOSION_SPAWN_ONCE_EXPLOSION_DEBRIS		( 0x00002000 )	// With groups, only spawn explosion debris once
#define FEXPLOSION_SPAWN_ONCE_SOUND					( 0x00004000 )	// With groups, only spawn sound once
#define FEXPLOSION_SPAWN_ONCE_SMOKE					( 0x00008000 )	// With groups, only spawn explosion smoke once
#define FEXPLOSION_SPAWN_ONCE_EXPLOSION_PARTICLE	( 0x00010000 )	// With groups, only spawn explosion particles once
#define FEXPLOSION_SPAWN_ONCE_SURFACE_DEBRIS		( 0x00020000 )	// With groups, only spawn surface debris once
#define FEXPLOSION_SPAWN_ONCE_SURFACE_DUST			( 0x00040000 )	// With groups, only spawn dust once
#define FEXPLOSION_SPAWN_ONCE_DAMAGE				( 0x00080000 )	// With groups, only spawn damage once
#define FEXPLOSION_SPAWN_EXPLOSION_FOLLOW_UNIT_DIR	( 0x00100000 )	// When the explosion is spawned, follow the unit direction when placing fireballs

// !!!!!
// YOU MUST CALL InitToDefaults() BEFORE PASSING THIS TO THE EXPLOSION SYSTEM
// !!!!!
FCLASS_ALIGN_PREFIX struct FExplosionSpawnParams_t {
	u32 uFlags;					// Flags for the explosion.  See FEXPLOSION_SPAWN_*
	u16 uSurfaceType;			// Surface type this explosion happened on, 0 for none
	s8 nNoisyPlayerIndex;		// Player responsible for making the sound associated with this explosion (-1=don't inform AI of sound)

	CFVec3A Pos_WS;				// Epicenter of the explosion
	CFVec3A UnitDir;			// Up direction of the explosion.  If non-zero, we have a surface explosion.  Zero is in space explosion.

	// Damage options
	const void *pDamageProfile;	// Damage profile for the explosion, overrides the damage profile in the explosion definition
	const void *pDamager;		// Damager for this explosion (CDamageForm::Damager_t)

	FINLINE void InitToDefaults( void ) {
		uFlags = FEXPLOSION_SPAWN_NONE;
		uSurfaceType = 0;
		nNoisyPlayerIndex = -1;
		Pos_WS.Zero();
		UnitDir.Zero();

		pDamageProfile = NULL;
		pDamager = NULL;
	}

	FCLASS_STACKMEM_ALIGN( FExplosionSpawnParams_t );
} FCLASS_ALIGN_SUFFIX;

BOOL fexplosion_ModuleStartup( void );
void fexplosion_ModuleShutdown( void );
BOOL fexplosion_LoadExplosionData( cchar *pszExplosionSet, cchar *pszExplosionGroups );
BOOL fexplosion_InitExplosionSystem( const u32 &uMaxExplosions, const u32 &uMaxSpawners, const u32 &uDamagerSize );
void fexplosion_UninitExplosionSystem( void );

void fexplosion_SetSurfaceDebrisCallback( ExplosionDebrisCallback_t *pCallback );
void fexplosion_SetSurfaceDustTextureCallback( ExplosionDustTextureCallback_t *pCallback );
void fexplosion_SetDamageCallback( ExplosionDamageCallback_t *pCallback );

void fexplosion_Work( void );
void fexplosion_Draw( CFCamera *pCam );

FExplosion_GroupHandle_t fexplosion_GetExplosionGroup( cchar *pszGroupName );
FExplosion_SpawnerHandle_t fexplosion_GetExplosionSpawner( void );

// pDamageImpactTriData points to a CDamageForm::TriData_t
void fexplosion_SpawnExplosion( FExplosion_SpawnerHandle_t hSpawner, 
								FExplosion_GroupHandle_t hGroup, 
								FExplosionSpawnParams_t *pSpawnParams,
								const void *pDamageImpactTriData = NULL );

#endif

