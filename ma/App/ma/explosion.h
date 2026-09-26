//////////////////////////////////////////////////////////////////////////////////////
// explosion.h - 
//
// Author: Michael Starich   
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
// 08/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _EXPLOSION_H_
#define _EXPLOSION_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fviewport.h"
#include "fmesh.h"
#include "fexplosion.h"



// Nate's new explosion system:

class CExplosion2 {
public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }

	static FINLINE FExplosion_GroupHandle_t GetExplosionGroup( cchar *pszGroupName ) { return fexplosion_GetExplosionGroup( pszGroupName ); }
	static FINLINE FExplosion_SpawnerHandle_t GetExplosionSpawner( void ) { return fexplosion_GetExplosionSpawner(); }

	static FINLINE void SpawnExplosion( FExplosion_SpawnerHandle_t hSpawner, 
								FExplosion_GroupHandle_t hGroup, 
								FExplosionSpawnParams_t *pSpawnParams,
								const void *pDamageImpactTriData = NULL ) { fexplosion_SpawnExplosion( hSpawner, hGroup, pSpawnParams, pDamageImpactTriData ); }
private:
	FINLINE CExplosion2() {}

	static void _ExplosionDebrisCallback( CFDebrisSpawner *pSpawner, FExplosionSurfceDebrisSize_e eDebrisSize, u16 uSurfaceType );
	static const CFTexInst *_ExplosionGetDustTexInstCallback( u16 uSurfaceType );
	static void _ExplosionDamageCallback( const void *pDamage, const void *pDamager, const CFVec3A *pPos, const void *pDamageImpactTriData = NULL );

private:
	static BOOL m_bSystemInitialized;
};


#endif

