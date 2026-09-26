//////////////////////////////////////////////////////////////////////////////////////
// GColl.h - Game collision module.
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
// 11/06/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _GCOLL_H_
#define _GCOLL_H_ 1

#include "fang.h"
#include "ftex.h"
#include "fcoll.h"
#include "fparticle.h"
#include "fdebris.h"
#include "fdecal.h"


class CEntity;
class CDamageProfile;
class CFWire;


enum GCollSurfType_e {
	GCOLL_SURF_TYPE_NORMAL,			// Normal surface
	GCOLL_SURF_TYPE_SLIPPERY,		// Slippery surface
	GCOLL_SURF_TYPE_LOW_HURT,		// Surface that deals a small amount of damage
	GCOLL_SURF_TYPE_MEDIUM_HURT,	// Surface that deals a medium amount of damage
	GCOLL_SURF_TYPE_INSTANT_KILL,	// Surface that deals an instant kill
	GCOLL_SURF_TYPE_VULNERABLE,		// Surface that represents a vulnerable area for an object/entity 

	GCOLL_SURF_TYPE_COUNT
};




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CGCollMaterial
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CGCollMaterial {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		FLAG_ENABLE_POCKMARK				= 0x00000001,		// Enable the drawing of pockmarks on this surface
		FLAG_ENABLE_BLAST_DECAL				= 0x00000002,		// Enable the drawing of blast decals on this surface
		FLAG_ENABLE_IMPACT_FLASH			= 0x00000004,		// Enable the drawing of impact flashes on this surface
		FLAG_CAN_CATCH_FIRE					= 0x00000008,		// This surface can catch fire from the flamer
		FLAG_LOOSE_DUST						= 0x00000010,		// This surface is formed from loose dust that is easily kicked up

		FLAG_NONE							= 0
	};


	enum DebrisGroup_e {
		DEBRIS_GROUP_SMALL,
		DEBRIS_GROUP_MEDIUM,
		DEBRIS_GROUP_LARGE,

		DEBRIS_GROUP_COUNT
	};


	enum ParticleType_e {
		PARTICLE_TYPE_SPARKS_BURST,
		PARTICLE_TYPE_SPARKS_SHOWER,
		PARTICLE_TYPE_BITS,
		PARTICLE_TYPE_DUST,

		PARTICLE_TYPE_COUNT
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	cchar *m_pszMtlName;										// Material name
	u32 m_nMtlID;												// Material ID used in art assets

	u16 m_nFlags;												// See FLAG_* for info
	u8 m_nProjectileReaction;									// See CEntity::ProjectileReaction_e for info

	CFDebrisGroup *m_apDebrisGroup[DEBRIS_GROUP_COUNT];			// The debris groups
	FParticle_DefHandle_t m_ahParticle[PARTICLE_TYPE_COUNT];	// The particle handles
	CFTexInst m_ExplosionDustTexInst;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE BOOL IsPockMarkEnabled( void ) const { return m_nFlags & FLAG_ENABLE_POCKMARK; }
	BOOL IsPockMarkEnabled( const FCollImpact_t *pCollImpact ) const;
	FINLINE BOOL IsBlastDecalEnabled( void ) const { return m_nFlags & FLAG_ENABLE_BLAST_DECAL; }
	FINLINE BOOL IsImpactFlashEnabled( void ) const { return m_nFlags & FLAG_ENABLE_IMPACT_FLASH; }
	FINLINE BOOL CanCatchFire( void ) const { return m_nFlags & FLAG_CAN_CATCH_FIRE; }
	FINLINE BOOL HasLooseDust( void ) const { return m_nFlags & FLAG_LOOSE_DUST; }

	FINLINE u8 GetProjectileReaction( void ) const { return m_nProjectileReaction; }
	u8 GetProjectileReaction( const CEntity *pEntity ) const;

	FINLINE BOOL CanDrawDebris( DebrisGroup_e nDebrisGroup ) const { FASSERT( nDebrisGroup>=0 && nDebrisGroup<DEBRIS_GROUP_COUNT ); return (m_apDebrisGroup[nDebrisGroup] != NULL); }

	FINLINE BOOL CanDrawParticle( ParticleType_e nParticleType ) const { FASSERT( nParticleType>=0 && nParticleType<PARTICLE_TYPE_COUNT ); return (m_ahParticle[nParticleType] != FPARTICLE_INVALID_HANDLE); }
	void DrawParticle( ParticleType_e nParticleType, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fUnitIntensity ) const;
	void DrawAllDrawableParticles( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bShowerOfSparks, f32 fUnitIntensitySparks, f32 fUnitIntensityBits, f32 fUnitIntensityDust ) const;


	FCLASS_STACKMEM_NOALIGN( CGCollMaterial );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CGColl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CGColl {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		SURFACE_TYPE_START_BIT				= 0,
		SURFACE_TYPE_BIT_COUNT				= 3,
		SURFACE_TYPE_MAX_COUNT				= (1 << SURFACE_TYPE_BIT_COUNT),
		SURFACE_TYPE_BIT_MASK				= (SURFACE_TYPE_MAX_COUNT - 1),

		MATERIAL_TYPE_START_BIT				= 3,
		MATERIAL_TYPE_BIT_COUNT				= 5,
		MATERIAL_TYPE_MAX_COUNT				= (1 << MATERIAL_TYPE_BIT_COUNT),
		MATERIAL_TYPE_BIT_MASK				= (MATERIAL_TYPE_MAX_COUNT - 1),
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static CGCollMaterial m_aMaterialTable[MATERIAL_TYPE_MAX_COUNT];




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;
	static FGameDataMap_t m_GameDataMap;
	static const FGameData_TableEntry_t m_aMaterialVocab[];
	static CDamageProfile *m_pDebrisDamageProfile;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }

	static void ClearMaterialTable( void );
	static void ClearMaterial( CGCollMaterial *pMaterial, u32 nMtlID );

	static BOOL LoadMaterial( FGameDataTableHandle_t hTable, u32 nMtlID );
	static BOOL LoadMaterialTable( cchar *pszFileName );

	static CGCollMaterial *FindMaterial( cchar *pszMtlName );

	static FINLINE GCollSurfType_e GetSurfaceType( u16 nCollUserType ) { return (GCollSurfType_e)((nCollUserType >> SURFACE_TYPE_START_BIT) & SURFACE_TYPE_BIT_MASK); }
	static FINLINE GCollSurfType_e GetSurfaceType( const FCollImpact_t *pCollImpact ) { return (GCollSurfType_e)((pCollImpact->nUserType >> SURFACE_TYPE_START_BIT) & SURFACE_TYPE_BIT_MASK); }

	static FINLINE u32 GetMtlID( u16 nCollUserType ) { return (nCollUserType >> MATERIAL_TYPE_START_BIT) & MATERIAL_TYPE_BIT_MASK; }

	static FINLINE const CGCollMaterial *GetMaterial( u16 nCollUserType ) { return &m_aMaterialTable[ GetMtlID(nCollUserType) ]; }
	static FINLINE const CGCollMaterial *GetMaterial( const FCollImpact_t *pCollImpact ) { return &m_aMaterialTable[ GetMtlID(pCollImpact->nUserType) ]; }

	static void SpawnDebris( CFDebrisSpawner *pDebrisSpawner, BOOL bSetSoundBit=TRUE, BOOL bEnableSounds=TRUE );

	static u8 ComputeProjectileReaction( const FCollImpact_t *pCollImpact );

	static CEntity *ExtractEntity( const FCollImpact_t *pCollImpact );
	static CFWire *ExtractWire( const FCollImpact_t *pCollImpact );
	
	static BOOL MaterialAllowsDecal( const FCollImpact_t *pImpact, CFDecal::Flags_e decalType );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _DebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact );
	static void _KickUpDustFromDebrisImpact( CFDebris *pDebris, const CGCollMaterial *pCollMaterial, const FCollImpact_t *pCollImpact );
	static void _DamageEntityFromDebrisImpact( CFDebris *pDebris, CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact );


	FCLASS_STACKMEM_NOALIGN( CGColl );
} FCLASS_NOALIGN_SUFFIX;




#endif

