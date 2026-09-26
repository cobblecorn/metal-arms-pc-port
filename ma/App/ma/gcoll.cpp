//////////////////////////////////////////////////////////////////////////////////////
// GColl.cpp - Game collision module.
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
// 12/03/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "gcoll.h"
#include "fparticle.h"
#include "fgamedata.h"
#include "gstring.h"
#include "fworld.h"
#include "fclib.h"
#include "meshtypes.h"
#include "damage.h"
#include "fdebris.h"
#include "entity.h"


#define _MATERIALS_CSV_FILENAME			"materials.csv"
#define _DEBRIS_DAMAGE_PROFILE_NAME		"Debris"

#if 0
	#define _POCKMARK_ENTITY_BITS		FCOLL_USER_TYPE_BITS_ALL
	#define _BLAST_ENTITY_BITS			FCOLL_USER_TYPE_BITS_ALL
#else
	#define _POCKMARK_ENTITY_BITS		~(ENTITY_BIT_SHIELD | ENTITY_BIT_LIQUIDMESH | ENTITY_BIT_LIQUIDVOLUME | ENTITY_BIT_BOTSWARMER )
	#define _BLAST_ENTITY_BITS			( ENTITY_BIT_DOOR | ENTITY_BIT_BOTTITAN | ENTITY_BIT_VEHICLE | ENTITY_BIT_BOTMORTAR | ENTITY_BIT_BOTMOZER | ENTITY_BIT_JUMPPAD | ENTITY_BIT_SITEWEAPON | ENTITY_BIT_MESHENTITY )
#endif


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CGColl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CGCollMaterial CGColl::m_aMaterialTable[MATERIAL_TYPE_MAX_COUNT];

BOOL CGColl::m_bSystemInitialized;
CDamageProfile *CGColl::m_pDebrisDamageProfile;


typedef struct {
	BOOL bEnablePockMark;
	BOOL bEnableBlastDecal;
	BOOL bEnableImpactFlash;
	BOOL bCanCatchFire;
	BOOL bLooseDust;

	cchar *pszProjReact;

	CFDebrisGroup *apDebrisGroup[CGCollMaterial::DEBRIS_GROUP_COUNT];

	FParticle_DefHandle_t ahParticle[CGCollMaterial::PARTICLE_TYPE_COUNT];
	
	FTexDef_t *pExplosionDustTexDef;
} _MaterialUserData_t;


const FGameData_TableEntry_t CGColl::m_aMaterialVocab[] = {
	// bEnablePockMark:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// bEnableBlastDecal:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// bEnableImpactFlash:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// bCanCatchFire:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// bLooseDust:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// pszProjReact:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// apDebrisGroup[DEBRIS_GROUP_SMALL]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP,
	sizeof( CFDebrisGroup * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apDebrisGroup[DEBRIS_GROUP_MEDIUM]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP,
	sizeof( CFDebrisGroup * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apDebrisGroup[DEBRIS_GROUP_LARGE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP,
	sizeof( CFDebrisGroup * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// ahParticle[PARTICLE_TYPE_SPARKS_BURST]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// ahParticle[PARTICLE_TYPE_SPARKS_SHOWER]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// ahParticle[PARTICLE_TYPE_BITS]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// ahParticle[PARTICLE_TYPE_DUST]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pExplosionDustTexDef
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_TEXDEF | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( FTexDef_t * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};



BOOL CGColl::InitSystem( void ) {
	FASSERT( !IsSystemInitialized() );

	m_bSystemInitialized = TRUE;

	ClearMaterialTable();

	m_pDebrisDamageProfile = CDamage::FindDamageProfile( _DEBRIS_DAMAGE_PROFILE_NAME );

	// hook callback up the the decal system
	if( CFDecal::IsModuleStartedUp() ) {
		CFDecal::RegisterMaterialCallback( MaterialAllowsDecal );
		CFDecal::SetCollisionMasks( _POCKMARK_ENTITY_BITS, _BLAST_ENTITY_BITS );

	} else {
		FASSERT_NOW;
		DEVPRINTF( "CGColl::InitSystem():  Warning, decal system is not started, unable to register callback\n" );
	}

	return TRUE;
}


void CGColl::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;
	}
}


// Returns NULL if pszMtlName is not a valid material name.
CGCollMaterial *CGColl::FindMaterial( cchar *pszMtlName ) {
	FASSERT( IsSystemInitialized() );

	u32 i;

	for( i=0; i<MATERIAL_TYPE_MAX_COUNT; ++i ) {
		if( !fclib_stricmp( pszMtlName, m_aMaterialTable[i].m_pszMtlName ) ) {
			// Found the name...
			return &m_aMaterialTable[i];
		}
	}

	// Name not found...
	return NULL;
}


void CGColl::ClearMaterialTable( void ) {
	FASSERT( IsSystemInitialized() );

	u32 i;

	for( i=0; i<MATERIAL_TYPE_MAX_COUNT; ++i ) {
		ClearMaterial( &m_aMaterialTable[i], i );
	}
}


void CGColl::ClearMaterial( CGCollMaterial *pMaterial, u32 nMtlID ) {
	FASSERT( nMtlID < MATERIAL_TYPE_MAX_COUNT );

	u32 i;

	pMaterial->m_nMtlID = nMtlID;
	pMaterial->m_pszMtlName = "<unnamed>";
	pMaterial->m_nFlags = CGCollMaterial::FLAG_NONE;
	pMaterial->m_nProjectileReaction = CEntity::PROJECTILE_REACTION_DEFAULT;

	for( i=0; i<CGCollMaterial::DEBRIS_GROUP_COUNT; ++i ) {
		pMaterial->m_apDebrisGroup[i] = NULL;
	}

	for( i=0; i<CGCollMaterial::PARTICLE_TYPE_COUNT; ++i ) {
		pMaterial->m_ahParticle[i] = FPARTICLE_INVALID_HANDLE;
	}

	pMaterial->m_ExplosionDustTexInst.SetToDefaults();
}


BOOL CGColl::LoadMaterial( FGameDataTableHandle_t hTable, u32 nMtlID ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( nMtlID < MATERIAL_TYPE_MAX_COUNT );

	_MaterialUserData_t UserData;
	CGCollMaterial *pMaterial;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	pMaterial = &m_aMaterialTable[nMtlID];

	ClearMaterial( pMaterial, nMtlID );

	pMaterial->m_pszMtlName = gstring_Main.AddString( fgamedata_GetTableName( hTable ) );
	if( pMaterial->m_pszMtlName == NULL ) {
		DEVPRINTF( "CGColl::LoadMaterialTable(): Not enough memory to store material name '%s' found in '%s'.\n", pMaterial->m_pszMtlName, fgamedata_GetFileNameFromTableHandle(hTable) );
		goto _ExitWithError;
	}

	if( !fgamedata_GetTableData( hTable, m_aMaterialVocab, &UserData, sizeof(UserData) ) ) {
		// Error...
		DEVPRINTF( "CGColl::LoadMaterialTable(): Error in definition of material name '%s' found in '%s'.\n", pMaterial->m_pszMtlName, fgamedata_GetFileNameFromTableHandle(hTable) );
		goto _ExitWithError;
	}

	if( UserData.bEnablePockMark ) {
		FMATH_SETBITMASK( pMaterial->m_nFlags, CGCollMaterial::FLAG_ENABLE_POCKMARK );
	}

	if( UserData.bEnableBlastDecal ) {
		FMATH_SETBITMASK( pMaterial->m_nFlags, CGCollMaterial::FLAG_ENABLE_BLAST_DECAL );
	}

	if( UserData.bEnableImpactFlash ) {
		FMATH_SETBITMASK( pMaterial->m_nFlags, CGCollMaterial::FLAG_ENABLE_IMPACT_FLASH );
	}

	if( UserData.bCanCatchFire ) {
		FMATH_SETBITMASK( pMaterial->m_nFlags, CGCollMaterial::FLAG_CAN_CATCH_FIRE );
	}

	if( !fclib_stricmp( UserData.pszProjReact, "Default" ) ) {
		pMaterial->m_nProjectileReaction = CEntity::PROJECTILE_REACTION_DEFAULT;
	} else if( !fclib_stricmp( UserData.pszProjReact, "Ricochet" ) ) {
		pMaterial->m_nProjectileReaction = CEntity::PROJECTILE_REACTION_RICOCHET;
	} else if( !fclib_stricmp( UserData.pszProjReact, "DeathToAll" ) ) {
		pMaterial->m_nProjectileReaction = CEntity::PROJECTILE_REACTION_DEATH_TO_ALL;
	} else if( !fclib_stricmp( UserData.pszProjReact, "Sticky" ) ) {
		pMaterial->m_nProjectileReaction = CEntity::PROJECTILE_REACTION_STICKY;
	} else if( !fclib_stricmp( UserData.pszProjReact, "HurtMe" ) ) {
		pMaterial->m_nProjectileReaction = CEntity::PROJECTILE_REACTION_HURT_ME;
	} else if( !fclib_stricmp( UserData.pszProjReact, "Shield" ) ) {
		pMaterial->m_nProjectileReaction = CEntity::PROJECTILE_REACTION_SHIELD;
	} else {
		DEVPRINTF( "CGColl::LoadMaterialTable(): Error in definition of material name '%s' found in '%s'. Projectile Reaction '%s' unrecognized.\n", pMaterial->m_pszMtlName, fgamedata_GetFileNameFromTableHandle(hTable), UserData.pszProjReact );
		goto _ExitWithError;
	}

	if( UserData.bLooseDust ) {
		FMATH_SETBITMASK( pMaterial->m_nFlags, CGCollMaterial::FLAG_LOOSE_DUST );
	}

	for( i=0; i<CGCollMaterial::DEBRIS_GROUP_COUNT; ++i ) {
		pMaterial->m_apDebrisGroup[i] = UserData.apDebrisGroup[i];
	}

	for( i=0; i<CGCollMaterial::PARTICLE_TYPE_COUNT; ++i ) {
		pMaterial->m_ahParticle[i] = UserData.ahParticle[i];
	}

	if( UserData.pExplosionDustTexDef ) {
		pMaterial->m_ExplosionDustTexInst.SetTexDef( UserData.pExplosionDustTexDef );
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CGColl::LoadMaterialTable( cchar *pszFileName ) {
	FASSERT( IsSystemInitialized() );

	FGameDataFileHandle_t hMtlMapDataFile, hMtlDefDataFile;
	FGameDataTableHandle_t hMtlMapTable, hMtlDefTable;
	FGameDataWalker_t hMtlMapWalker, hMtlDefWalker;
	FGameData_VarType_e nFieldType;
	cchar *pszMtlName;
	u32 nMtlID;
	const void *pData;

	ClearMaterialTable();

	FResFrame_t ResFrame = fres_GetFrame();
	FMemFrame_t MemFrame = fmem_GetFrame();

	// Load map CSV file...
	hMtlMapDataFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hMtlMapDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CGColl::LoadMaterialTable(): Could not open '%s'.\n", pszFileName );
		goto _ExitWithError;
	}

	// Load material definition file...
	hMtlDefDataFile = fgamedata_LoadFileToFMem( _MATERIALS_CSV_FILENAME );
	if( hMtlDefDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CGColl::LoadMaterialTable(): Could not open '%s'.\n", _MATERIALS_CSV_FILENAME );
		goto _ExitWithError;
	}

	hMtlMapTable = fgamedata_GetFirstTable( hMtlMapDataFile, hMtlMapWalker );
	for( ; hMtlMapTable != FGAMEDATA_INVALID_TABLE_HANDLE; hMtlMapTable = fgamedata_GetNextTable( hMtlMapWalker ) ) {
		// Get the material name...
		pszMtlName = fgamedata_GetTableName( hMtlMapTable );

		// See if this material name already exists...
		if( FindMaterial( pszMtlName ) ) {
			DEVPRINTF( "CGColl::LoadMaterialTable(): Material name '%s' exists more than once in '%s.csv'.\n", pszMtlName, pszFileName );
			continue;
		}

		// Check validity of table field...
		if( fgamedata_GetNumFields( hMtlMapTable ) != 1 ) {
			DEVPRINTF( "CGColl::LoadMaterialTable(): Material name '%s' in '%s.csv' must have exactly one field.\n", pszMtlName, pszFileName );
			continue;
		}

		pData = fgamedata_GetPtrToFieldData( hMtlMapTable, 0, nFieldType );
		if( nFieldType != FGAMEDATA_VAR_TYPE_FLOAT ) {
			DEVPRINTF( "CGColl::LoadMaterialTable(): Material name '%s' in '%s.csv' must specify a numeric value.\n", pszMtlName, pszFileName );
			continue;
		}

		// Get material ID...
		nMtlID = (u32)*((f32 *)pData);

		if( nMtlID >= MATERIAL_TYPE_MAX_COUNT ) {
			DEVPRINTF( "CGColl::LoadMaterialTable(): Material name '%s' in '%s.csv' specifies a material ID greater than %u.\n", pszMtlName, pszFileName, MATERIAL_TYPE_MAX_COUNT-1 );
			continue;
		}

		if( fclib_stricmp( m_aMaterialTable[nMtlID].m_pszMtlName, "<unnamed>" ) ) {
			DEVPRINTF( "CGColl::LoadMaterialTable(): Material name '%s' in '%s.csv' specifies a material ID (%u) that has already been defined.\n", pszMtlName, pszFileName, nMtlID );
			continue;
		}

		// Find this material in the material definition file...
		hMtlDefTable = fgamedata_GetFirstTable( hMtlDefDataFile, hMtlDefWalker );
		for( ; hMtlDefTable != FGAMEDATA_INVALID_TABLE_HANDLE; hMtlDefTable = fgamedata_GetNextTable( hMtlDefWalker ) ) {
			if( !fclib_stricmp( fgamedata_GetTableName( hMtlDefTable ), pszMtlName ) ) {
				// Found it!
				LoadMaterial( hMtlDefTable, nMtlID );
				break;
			}
		}
		if( hMtlDefTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			// Material not found in the material definition file...
			DEVPRINTF( "CGColl::LoadMaterialTable(): Material name '%s' in '%s.csv' was not found in '%s.csv'.\n", pszMtlName, pszFileName, _MATERIALS_CSV_FILENAME );
			continue;
		}
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


// Note that this function overwrites the callback field of the provided spawner.
void CGColl::SpawnDebris( CFDebrisSpawner *pDebrisSpawner, BOOL bSetSoundBit, BOOL bEnableSounds ) {
	if( pDebrisSpawner->m_pDebrisGroup == NULL ) {
		pDebrisSpawner->Kill();
		return;
	}

	if( pDebrisSpawner->m_pDebrisGroup->m_fUnitDustKickUp==0.0f && pDebrisSpawner->m_pDebrisGroup->m_fUnitHitpointDamage==0.0f ) {
		pDebrisSpawner->m_pFcnCallback = NULL;
	} else {
		pDebrisSpawner->m_pFcnCallback = _DebrisCallback;
	}

	if( bSetSoundBit ) {
		FMATH_WRITEBIT( bEnableSounds, pDebrisSpawner->m_nFlags, CFDebrisSpawner::FLAG_PLAY_IMPACT_SOUND | CFDebrisSpawner::FLAG_PLAY_FLAMING_SOUND );
	}

	pDebrisSpawner->Spawn();
}


void CGColl::_DebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact ) {
	if( nReason != CFDebrisDef::CALLBACK_REASON_COLLISION ) {
		return;
	}

	const CGCollMaterial *pCollMaterial = GetMaterial( pCollImpact );

	if( pCollMaterial->HasLooseDust() ) {
		// The surface the debris hit has loose dust on it...

		if( pDebris->GetDebrisDef()->m_fUnitDustKickUp > 0.0f ) {
			// This debris particle can kick up dust...

			if( pCollMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST ) ) {
				// We're allowed to draw dust from this surface...

				_KickUpDustFromDebrisImpact( pDebris, pCollMaterial, pCollImpact );
			}
		}
	}

	if( pCollImpact->pTag ) {
		// We hit a mesh...

		if( pDebris->GetDebrisDef()->m_fUnitHitpointDamage > 0.0f ) {
			// This debris partical can deal damage...

			CFWorldMesh *pHitWorldMesh = (CFWorldMesh *)pCollImpact->pTag;

			if( pHitWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
				// We hit an entity...

				CEntity *pHitEntity = (CEntity *)pHitWorldMesh->m_pUser;

				_DamageEntityFromDebrisImpact( pDebris, pHitEntity, pHitWorldMesh, pCollImpact );
			}
		}
	}
}


#define _SPAWN_LOOSE_DUST_MIN_SPEED		15.0f
#define _SPAWN_LOOSE_DUST_MAX_SPEED		30.0f


void CGColl::_KickUpDustFromDebrisImpact( CFDebris *pDebris, const CGCollMaterial *pCollMaterial, const FCollImpact_t *pCollImpact ) {
	const CFDebrisDef *pDebrisDef = pDebris->GetDebrisDef();

	f32 fSpeed2 = pDebrisDef->m_LinVel_WS.MagSq();
	if( fSpeed2 < (_SPAWN_LOOSE_DUST_MIN_SPEED * _SPAWN_LOOSE_DUST_MIN_SPEED) ) {
		return;
	}

	// We hit this surface going pretty fast...
	f32 fUnitIntensity = fmath_Div( fmath_Sqrt( fSpeed2 ) - _SPAWN_LOOSE_DUST_MIN_SPEED, _SPAWN_LOOSE_DUST_MAX_SPEED - _SPAWN_LOOSE_DUST_MIN_SPEED );
	FMATH_CLAMP_UNIT_FLOAT( fUnitIntensity );
	fUnitIntensity *= pDebris->GetDebrisDef()->m_fUnitDustKickUp;

	pCollMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST, &pCollImpact->ImpactPoint, &pCollImpact->UnitFaceNormal, fUnitIntensity );
}


#define _DEAL_DAMAGE_MIN_SPEED		15.0f
#define _DEAL_DAMAGE_MAX_SPEED		30.0f


void CGColl::_DamageEntityFromDebrisImpact( CFDebris *pDebris, CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact ) {
	const CFDebrisDef *pDebrisDef = pDebris->GetDebrisDef();

	f32 fSpeed2 = pDebrisDef->m_LinVel_WS.MagSq();
	if( fSpeed2 < (_DEAL_DAMAGE_MIN_SPEED * _DEAL_DAMAGE_MIN_SPEED) ) {
		return;
	}

	// We hit the entity going pretty fast...
	f32 fUnitIntensity = fmath_Div( fmath_Sqrt( fSpeed2 ) - _DEAL_DAMAGE_MIN_SPEED, _DEAL_DAMAGE_MAX_SPEED - _DEAL_DAMAGE_MIN_SPEED );
	FMATH_CLAMP_UNIT_FLOAT( fUnitIntensity );
	fUnitIntensity *= pDebris->GetDebrisDef()->m_fUnitHitpointDamage;

	// Get an empty damage form...
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	if( pDamageForm ) {
		// Fill out the form...

		CFVec3A DebrisUnitDir_WS;
		f32 fMag2;

		fMag2 = pDebris->GetDebrisDef()->m_LinVel_WS.MagSq();
		if( fMag2 > 0.00001f ) {
			DebrisUnitDir_WS.Mul( pDebris->GetDebrisDef()->m_LinVel_WS, fmath_InvSqrt( fMag2 ) );
		} else {
			DebrisUnitDir_WS = CFVec3A::m_UnitAxisX;
		}

		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_pDamageProfile = m_pDebrisDamageProfile;
		pDamageForm->m_pDamageeEntity = pHitEntity;
		pDamageForm->m_fNormIntensity = fUnitIntensity;
		pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pCollImpact, &DebrisUnitDir_WS );

		CDamage::SubmitDamageForm( pDamageForm );
	}
}


// Given the provided collision impact info, returns the projectile reaction code.
u8 CGColl::ComputeProjectileReaction( const FCollImpact_t *pCollImpact ) {
	if( pCollImpact == NULL ) {
		return CEntity::PROJECTILE_REACTION_DEFAULT;
	}

	const CGCollMaterial *pMaterial = GetMaterial( pCollImpact );

	if( pCollImpact->pTag ) {
		CFWorldTracker *pWorldTracker = (CFWorldTracker *)pCollImpact->pTag;

		if( pWorldTracker->m_nUser == MESHTYPES_ENTITY ) {
			// This is an entity...

			return pMaterial->GetProjectileReaction( (CEntity *)pWorldTracker->m_pUser );
		}
	}

	// Not an entity...

	return pMaterial->GetProjectileReaction();
}


// Extracts the entity pointer from a collision impact.
// Returns NULL if the collision impact tag is not an entity.
CEntity *CGColl::ExtractEntity( const FCollImpact_t *pCollImpact ) {
	if( pCollImpact->pTag ) {
		// Tag is a tracker...

		CFWorldTracker *pWorldTracker = (CFWorldTracker *)pCollImpact->pTag;

		if( pWorldTracker->m_nUser == MESHTYPES_ENTITY ) {
			// This is an entity...

			return (CEntity *)pWorldTracker->m_pUser;
		}
	}

	return NULL;
}


// Extracts the wire pointer from a collision impact.
// Returns NULL if the collision impact does not indicate a wire.
CFWire *CGColl::ExtractWire( const FCollImpact_t *pCollImpact ) {
	if( pCollImpact->pTag ) {
		// Tag is a tracker...

		CFWorldTracker *pWorldTracker = (CFWorldTracker *)pCollImpact->pTag;

		if( pWorldTracker->m_nUser == FWORLD_USERTYPE_WIRE ) {
			// This is a wire...

			return (CFWire *)pWorldTracker->m_pUser;
		}
	}

	return NULL;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CGCollMaterial
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


BOOL CGCollMaterial::IsPockMarkEnabled( const FCollImpact_t *pCollImpact ) const {
	if( !IsPockMarkEnabled() ) {
		return FALSE;
	}

	// Pockmarks are enabled...

	if( pCollImpact == NULL ) {
		// No more info...
		return TRUE;
	}

	if( pCollImpact->pTag == NULL ) {
		// Always draw on world geo...
		return TRUE;
	}

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pCollImpact->pTag;

	if( pWorldMesh->m_nUser == FWORLD_USERTYPE_WIRE ) {
		// Don't draw pockmarks on wires...
		return FALSE;
	}

	if( pWorldMesh->m_nUser != MESHTYPES_ENTITY ) {
		// Ok to draw pockmarks on non-entities...
		return TRUE;
	}

	// The artists aren't flagging their objects as dynamic so let's just not draw any pockmarks...

	return FALSE;

#if 0
	if( pWorldMesh->m_nFlags & FMESHINST_FLAG_STATIC ) {
		// Ok to draw pockmarks on static meshes...
		return TRUE;
	}

	// Dynamic meshes won't receive pockmarks...

	return FALSE;
#endif
}


// If a NULL entity is specified, this simply returns the material's projectile reaction.
// If an entity is passed in, this will return the entity's projectile reaction, unless
// it is set to Default, in which case the material's projectile reaction is returned.
u8 CGCollMaterial::GetProjectileReaction( const CEntity *pEntity ) const {
	if( pEntity == NULL ) {
		return GetProjectileReaction();
	}

	u8 nEntityProjReact = pEntity->GetProjectileReaction();

	if( nEntityProjReact == CEntity::PROJECTILE_REACTION_DEFAULT ) {
		return GetProjectileReaction();
	}

	return nEntityProjReact;
}


void CGCollMaterial::DrawParticle( ParticleType_e nParticleType, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fUnitIntensity ) const {
	FASSERT( nParticleType>=0 && nParticleType<PARTICLE_TYPE_COUNT );

	if( !CanDrawParticle( nParticleType ) ) {
		return;
	}

	fparticle_SpawnEmitter( m_ahParticle[nParticleType], pPos_WS->v3, &pUnitDir_WS->v3, fUnitIntensity );
}


void CGCollMaterial::DrawAllDrawableParticles( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bShowerOfSparks, f32 fUnitIntensitySparks, f32 fUnitIntensityBits, f32 fUnitIntensityDust ) const {
	if( bShowerOfSparks ) {
		if( CanDrawParticle( PARTICLE_TYPE_SPARKS_SHOWER ) ) {
			fparticle_SpawnEmitter( m_ahParticle[PARTICLE_TYPE_SPARKS_SHOWER], pPos_WS->v3, &pUnitDir_WS->v3, fUnitIntensitySparks );
		}
	} else {
		if( CanDrawParticle( PARTICLE_TYPE_SPARKS_BURST ) ) {
			fparticle_SpawnEmitter( m_ahParticle[PARTICLE_TYPE_SPARKS_BURST], pPos_WS->v3, &pUnitDir_WS->v3, fUnitIntensitySparks );
		}
	}

	if( CanDrawParticle( PARTICLE_TYPE_BITS ) ) {
		fparticle_SpawnEmitter( m_ahParticle[PARTICLE_TYPE_BITS], pPos_WS->v3, &pUnitDir_WS->v3, fUnitIntensityBits );
	}

	if( CanDrawParticle( PARTICLE_TYPE_DUST ) ) {
		fparticle_SpawnEmitter( m_ahParticle[PARTICLE_TYPE_DUST], pPos_WS->v3, &pUnitDir_WS->v3, fUnitIntensityDust );
	}
}


BOOL CGColl::MaterialAllowsDecal( const FCollImpact_t *pImpact, CFDecal::Flags_e decalType ) {
	const CGCollMaterial *pMaterial = GetMaterial( pImpact );

	return ((decalType == CFDecal::TYPE_BLAST) && pMaterial->IsBlastDecalEnabled()) ||
			((decalType == CFDecal::TYPE_POCKMARK) && pMaterial->IsPockMarkEnabled());
}

