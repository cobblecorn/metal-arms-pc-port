//////////////////////////////////////////////////////////////////////////////////////
// weapon_spew.cpp - Small Projectile Emitter Weapon.
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

#include "fang.h"
#include "weapon_spew.h"
#include "fgamedata.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "fmesh.h"
#include "fanim.h"
#include "meshtypes.h"
#include "floop.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "potmark.h"
#include "fcamera.h"
#include "ItemInst.h"
#include "meshentity.h"
#include "AI/AIEnviro.h"
#include "weapon_scope.h"
#include "damage.h"
#include "gcoll.h"
#include "fsound.h"
#include "eparticlepool.h"
#include "fdecal.h"



#define _USER_PROP_FILENAME		"w_spew.csv"
#define _SMOKE_UNIT_THRESHOLD	0.25f



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponSpewBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponSpewBuilder _WeaponSpewBuilder;

void CWeaponSpewBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONSPEW, pszEntityType );
}


BOOL CWeaponSpewBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponSpewBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponSpew
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CWeaponSpew::m_bSystemInitialized;
CWeaponSpew::_UserProps_t CWeaponSpew::m_aUserProps[EUK_COUNT_SPEW];
CWeaponSpew *CWeaponSpew::m_pCallbackThis;


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponSpew::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_WEAPON]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszMeshName[_MESH_CLIP]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszMeshName[_BONE_PRIMARY_FIRE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszMeshName[_BONE_CLIP_ATTACH]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszBoneName[_BONE_L3_SPIN_CARTRIDGE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszBoneName[_BONE_L3_SPIN_BARREL]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fClipAmmoMax:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_65534,

	// fReserveAmmoMax:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_65534,

	// fRoundsPerSec:
	// fOORoundsPerSec:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_Pt001,
	F32_DATATABLE_1000,

	// fWeaponCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fClipCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMaxLiveRange:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fUnitRecoil:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fDistFromWeaponOrigToMuzzle:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fMinTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMaxTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fUnitCamVibration:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fBestShotSpreadFactor:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fWorstShotSpreadFactor:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// afScopeSpreadMult[0]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// afScopeSpreadMult[1]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fShotSpreadSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fShotSpreadRecoverySpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleWidth_Z:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleHeight_Z:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleAlpha_Z:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fMuzzleScale_XY:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleAlpha_XY:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fMuzzleScale_Mesh:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleRot_Mesh:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMuzzleScale_Impact:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleAlpha_Impact:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fMuzzleOffset_Impact:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fL3_CartridgeMaxRevsPerSec:
	// fL3_CartridgeOOMaxRevsPerSec:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fL3_CartridgeRotAccel:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fL3_CartridgeRotDecel:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	FGAMEDATA_VOCAB_PARTICLE,		// hParticleSmoke

	// fHeatUpSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_100000,

	// fCoolDownSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_100000,

	// fMaxSmokeUnitIntensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	FGAMEDATA_VOCAB_DAMAGE,			// pDamageProfile

	FGAMEDATA_VOCAB_MESH,			// pMeshEjectClip

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEmpty
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEjectClip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAttachClip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupSlapInClip

	FGAMEDATA_VOCAB_DECAL_DEF,		// hDecalDef

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponSpew::m_aUserPropMapTable[] = {
	"SpewL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"SpewL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"SpewL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"SpewMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],

	"SpewMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[4],

	NULL
};



BOOL CWeaponSpew::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;

	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponSpew::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_SPEW; i++ ) {
		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_SPEW][i];

		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_CLIP;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->fUnitRecoil = m_aUserProps[i].fUnitRecoil;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_LEFT_HAND_RELOADS | INFOFLAG_SCOPE_ENABLED | INFOFLAG_DISPLAY_AMMO_TEXT;
		pInfo->nReticleType = CReticle::TYPE_DROID_STANDARD;
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponSpew::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}



CWeaponSpew::CWeaponSpew() {
	_ClearDataMembers();
}



CWeaponSpew::~CWeaponSpew() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}



BOOL CWeaponSpew::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );

	// Get pointer to the leaf class's builder object...
	CWeaponSpewBuilder *pBuilder = (CWeaponSpewBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponSpewBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONSPEW );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_SPEW, pszEntityName, pMtx, pszAIBuilderName );
}



BOOL CWeaponSpew::ClassHierarchyBuild( void ) {
	u32 i;
	s32 nBoneIndex;
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	_ResourceData_t *pResourceData;
	u32 uMeshEUK;
	u32 uResEUK;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponSpewBuilder *pBuilder = (CWeaponSpewBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_SPEW; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_SPEW );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponSpew::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );

			pMesh = m_pErrorMesh;
			if( pMesh == NULL ) {
				goto _ExitWithError;
			}
		}

		// Init the world mesh...
		MeshInit.pMesh = pMesh;
		MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist = m_aUserProps[i].fWeaponCullDist;
		MeshInit.Mtx = *MtxToWorld();

		uResEUK = i;
		if( !CreateSharedEUKData( &MeshInit, &uResEUK, &(pResourceData->m_pWorldMesh), &(pResourceData->m_pAnimCombiner), &(pResourceData->m_pAnimRest) ) ) {
			DEVPRINTF( "CWeaponFlamer::ClassHierarchyBuild():  Error creating EUK shared data\n" );
			goto _ExitWithError;
		}

		if( uResEUK == i ) {
			pResourceData->m_pWorldMesh->RemoveFromWorld();
			pResourceData->m_pWorldMesh->SetCollisionFlag( TRUE );
			pResourceData->m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
			pResourceData->m_pWorldMesh->m_pUser = this;
			pResourceData->m_pWorldMesh->SetUserTypeBits( TypeBits() );
			pResourceData->m_pWorldMesh->SetLineOfSightFlag(FALSE); //pgm added this so that weapons never block los tests

			if( !pResourceData->m_pAnimCombiner->CreateSimple( pResourceData->m_pAnimRest, pResourceData->m_pWorldMesh ) ) {
				DEVPRINTF( "CWeaponSpew::Create(): Could not create animation combiner.\n" );
				goto _ExitWithError;
			}

			if( uMeshEUK >= 2 ) {
				pResourceData->m_pAnimCombiner->SetBoneCallback( _CartridgeBoneCallback );
				pResourceData->m_pAnimCombiner->DisableAllBoneCallbacks();
				pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[uMeshEUK].apszBoneName[_BONE_L3_SPIN_CARTRIDGE] );
				pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[uMeshEUK].apszBoneName[_BONE_L3_SPIN_BARREL] );
			}
		}

		nBoneIndex = pResourceData->m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE] );
		if( nBoneIndex < 0 ) {
			DEVPRINTF( "CWeaponSpew::ClassHierarchyBuild(): Bone '%s' doesn't exist in object '%s'.\n", m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE], Name() );
			goto _ExitWithError;
		}
		pResourceData->m_pFireMtx_WS = pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}

	// Build our clip mesh entity...
	m_pClipMeshEntity = fnew CMeshEntity;
	if( m_pClipMeshEntity == NULL ) {
		DEVPRINTF( "CWeaponSpew::Create(): Not enough memory to create the clip CMeshEntity.\n" );
		goto _ExitWithError;
	}

	if( !m_pClipMeshEntity->Create( m_aUserProps[0].apszMeshName[_MESH_CLIP] ) ) {
		DEVPRINTF( "CWeaponSpew::Create(): Could not create the clip CMeshEntity.\n" );
		goto _ExitWithError;
	}

	m_pClipMeshEntity->SetCollisionFlag( FALSE );
	m_pClipMeshEntity->SetLineOfSightFlag( FALSE );
	m_pClipMeshEntity->SetCullDist( m_aUserProps[0].fClipCullDist );
	m_pClipMeshEntity->MeshFlip_SetFlipsPerSec( 0.0f );
	m_pClipMeshEntity->RemoveFromWorld();

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponSpew::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CWeapon::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponSpew::ClassHierarchyDestroy( void ) {
	u32 i;

	_KillAudioEmitters();

	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	for( i=0; i<EUK_COUNT_SPEW; ++i ) {
		DestroySharedEUKData( i );
	}

	fdelete( m_pClipMeshEntity );

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponSpew::GetLeafClassBuilder( void ) {
	return &_WeaponSpewBuilder;
}


void CWeaponSpew::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_SPEW; ++i ) {
		m_aResourceData[i].m_pAnimCombiner = NULL;
		m_aResourceData[i].m_pAnimRest = NULL;
		m_aResourceData[i].m_pWorldMesh = NULL;
		m_aResourceData[i].m_pFireMtx_WS = NULL;
	}

	m_pResourceData = NULL;
	m_pUserProps = NULL;

	m_pClipMeshEntity = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_fUnitShotSpread = 0.0f;
	m_fSecondsUntilNextTracer = 0.0f;
	m_bFireThisFrame = FALSE;
	m_TargetPos_WS.Zero();
	m_pBuddyFirePos_WS = NULL;
	m_pParticleSmoke = NULL;
	m_fUnitHeatup = 0.0f;
	m_fCartridgeSpinAngle = 0.0f;
	m_fCartridgeRotSpeed = 0.0f;

	fforce_NullHandle( &m_hForce );

	for( i=0; i<MAX_SIMULTANEOUS_FIRE_SOUNDS; ++i ) {
		m_apAudioEmitterFire[i] = NULL;
	}

	m_nAudioEmitterIndex = 0;

	_PickRandomDebrisChunkTime();
}


void CWeaponSpew::_KillAudioEmitters( void ) {
	u32 i;

	for( i=0; i<MAX_SIMULTANEOUS_FIRE_SOUNDS; ++i ) {
		if( m_apAudioEmitterFire[i] ) {
			m_apAudioEmitterFire[i]->Destroy();
			m_apAudioEmitterFire[i] = NULL;
		}
	}
}


void CWeaponSpew::_PickRandomDebrisChunkTime( void ) {
	m_fDebrisSecondsCountdownTimer = fmath_RandomFloatRange( 0.3f, 0.6f );
}


void CWeaponSpew::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponSpew::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst(pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);
	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NUMERIC );
	}
}


void CWeaponSpew::_AddClipToWorld( void ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_pUserProps->apszBoneName[_BONE_CLIP_ATTACH], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();
}


void CWeaponSpew::_RemoveClipFromWorld( void ) {
	m_pClipMeshEntity->DetachFromParent();
	m_pClipMeshEntity->RemoveFromWorld();
}


void CWeaponSpew::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pWorldMesh->UpdateTracker();

	// Add clip to the world...
	if( m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED ) {
		_AddClipToWorld();
	}
}


void CWeaponSpew::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	// Remove smoke from world...
	_KillSmoke();

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();

	// Remove clip from the world...
	if( m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED ) {
		_RemoveClipFromWorld();
	}	
}


void CWeaponSpew::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	u32 i;

	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this weapon...

		for( i=0; i<EUK_COUNT_SPEW; ++i ) {
			FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );
	} else {
		// Disable drawing of this weapon...
		for( i=0; i<EUK_COUNT_SPEW; ++i ) {
			FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );

		_KillSmoke();
	}
}


void CWeaponSpew::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		// Handle animations and attached objects...
		m_pCallbackThis = this;
		m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );
		RelocateAllChildren();
		m_pCallbackThis = NULL;		
	}
}


CFMtx43A *CWeaponSpew::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponSpew::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponSpew::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;

	// Add our clip to the list...
	m_pClipMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}


// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponSpew::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_SPEW][nPreviousUpgradeLevel];

	if( m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED ) {
		_RemoveClipFromWorld();
	}

	_KillSmoke();

	// Remove old mesh from world...
	pOldResourceData->m_pWorldMesh->RemoveFromWorld();

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		if( m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED ) {
			_AddClipToWorld();
		}
	}

	TransferFromReserveToClip( INFINITE_AMMO, FALSE );
}


void CWeaponSpew::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	// New compute method using the Primary_Fire bone...
	*pMuzzlePoint_WS = m_pResourceData->m_pFireMtx_WS->m_vPos;
}


// This function is called to let the weapon know what the trigger values are.
// It should be called whether or not the triggers are pressed because this function
// is responsible for firing and reloading of the weapon.
//
// pProjUnitDir_WS must point in the direction that the weapon will fire.
//
// Return value:
//   Bit 0: Trigger #1 fired
//   Bit 1: Trigger #2 fired
u32 CWeaponSpew::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );

	m_bFireThisFrame = FALSE;

	if( fUnitTriggerVal1 < 0.25f ) {
		// Trigger not down...

		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

		goto _ExitWithoutTriggerPressed;
	}

	if( m_nClipAmmo == 0 ) {
		// No rounds in clip...

		if( m_nReserveAmmo == 0 ) {
			// Completely out of ammo. Make empty click sound...

			if( !(m_nWeaponFlags & WEAPONFLAG_MADE_EMPTY_CLICK_SOUND) ) {
				FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

				PlaySound( m_pUserProps->pSoundGroupEmpty );
			}
		}

		goto _ExitWithoutTriggerPressed;
	}

	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		goto _ExitWithoutTriggerPressed;
	}

	if( !(m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED) ) {
		// The clip is not installed...
		goto _ExitWithoutTriggerPressed;
	}

	if( m_fDebrisSecondsCountdownTimer > 0.0f ) {
		m_fDebrisSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}

	if( m_fUnitShotSpread < 1.0f ) {
		m_fUnitShotSpread += FLoop_fPreviousLoopSecs * m_pUserProps->fShotSpreadSpeed;
		FMATH_CLAMPMAX( m_fUnitShotSpread, 1.0f );
	}

	if( m_fUnitHeatup < 1.0f ) {
		m_fUnitHeatup += m_pUserProps->fHeatUpSpeed * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_fUnitHeatup, 1.0f );
	}
	_UpdateSmoke();

	if( m_nUpgradeLevel == 2 ) {
		if( m_fCartridgeRotSpeed < m_pUserProps->fL3_CartridgeMaxRevsPerSec ) {
			m_fCartridgeRotSpeed += FLoop_fPreviousLoopSecs * m_pUserProps->fL3_CartridgeRotAccel;
			FMATH_CLAMPMAX( m_fCartridgeRotSpeed, m_pUserProps->fL3_CartridgeMaxRevsPerSec );
		}
	}

	if( IsOwnedByPlayer() ) {
		CFCamera *pCamera = fcamera_GetCameraByIndex( GetOwner()->m_nPossessionPlayerIndex );
		pCamera->SetCamVibrationForOneFrame( m_pUserProps->fUnitCamVibration );
	}

	if( m_fSecondsUntilNextTracer > 0.0f ) {
		m_fSecondsUntilNextTracer -= FLoop_fPreviousLoopSecs;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return 0;
	}

	m_TargetPos_WS.Set( *pTargetPos_WS );
	if( IsOwnedByPlayer() ) {
		m_pOwnerBot->FocusHumanTargetPoint_WS( &m_TargetPos_WS, this );
	}

	m_bFireThisFrame = TRUE;
	m_pBuddyFirePos_WS = pBuddyFirePos_WS;

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

	return 1;

_ExitWithoutTriggerPressed:
	if( m_fUnitShotSpread > 0.0f ) {
		m_fUnitShotSpread -= FLoop_fPreviousLoopSecs * m_pUserProps->fShotSpreadRecoverySpeed;
		FMATH_CLAMPMIN( m_fUnitShotSpread, 0.0f );
	}

	if( m_fUnitHeatup > 0.0f ) {
		m_fUnitHeatup -= m_pUserProps->fCoolDownSpeed * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fUnitHeatup, 0.0f );
	}
	_UpdateSmoke();

	if( m_nUpgradeLevel == 2 ) {
		if( m_fCartridgeRotSpeed > 0.0f ) {
			m_fCartridgeRotSpeed -= FLoop_fPreviousLoopSecs * m_pUserProps->fL3_CartridgeRotDecel;
			FMATH_CLAMPMIN( m_fCartridgeRotSpeed, 0.0f );
		}
	}

	return 0;
}


// Called once per frame to update animations, timers, etc.
void CWeaponSpew::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	// For L3 SPEW, rotate the cartridge...
	if( m_nUpgradeLevel == 2 ) {
		m_fCartridgeSpinAngle += FLoop_fPreviousLoopSecs * m_fCartridgeRotSpeed;

		while( m_fCartridgeSpinAngle > FMATH_2PI ) {
			m_fCartridgeSpinAngle -= FMATH_2PI;
		}
	}

	// Fire off a round...
	if( m_bFireThisFrame ) {
		// Compute fire direction...

		f32 fJitterFactor, fBestSpread, fWorstSpread;

		fBestSpread = m_pUserProps->fBestShotSpreadFactor;
		fWorstSpread = m_pUserProps->fWorstShotSpreadFactor;

		fJitterFactor = FMATH_FPOT( m_fUnitShotSpread, fBestSpread, fWorstSpread );
		if( m_pAttachedScope && m_pAttachedScope->IsZoomEnabled() ) {
			fJitterFactor *= m_pUserProps->afScopeSpreadMult[ m_pAttachedScope->GetUpgradeLevel() ];
		}

		CFVec3A MuzzlePoint;
		CFVec3A FireUnitDir_WS;

		ComputeMuzzlePoint_WS( &MuzzlePoint );
		ComputeFireUnitDir( &FireUnitDir_WS, &MuzzlePoint, &m_MtxToWorld.m_vFront, &m_TargetPos_WS, FALSE );

		_FireRound( &MuzzlePoint, &FireUnitDir_WS, fJitterFactor, FALSE );

		if( m_pBuddyFirePos_WS ) {
			_FireRound( m_pBuddyFirePos_WS, &FireUnitDir_WS, fJitterFactor, TRUE );
		}

		m_fSecondsCountdownTimer += m_pUserProps->fOORoundsPerSec;
		RemoveFromClip(1);
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}
}


void CWeaponSpew::_FireRound( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fJitterFactor, BOOL bBuddy ) {
	CFVec3A FireUnitDir_WS;

	s32 nPlayerIndex = GetSafeOwnerPlayerIndex();

	fmath_ScatterUnitVec( &FireUnitDir_WS, pUnitDir_WS, fJitterFactor );

	// Draw muzzle flash...
	if( IsDrawEnabled() ) {
		f32 fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );

		CMuzzleFlash::AddFlash_CardPosterZ(
			m_ahMuzzleFlashGroup[ MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
			*pPos_WS,
			FireUnitDir_WS,
			m_pUserProps->fMuzzleWidth_Z * fRandomScale,
			m_pUserProps->fMuzzleHeight_Z * fRandomScale,
			m_pUserProps->fMuzzleAlpha_Z
		);

		CMuzzleFlash::AddFlash_CardPosterXY(
			m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
			*pPos_WS,
			m_pUserProps->fMuzzleScale_XY * fRandomScale,
			m_pUserProps->fMuzzleAlpha_XY
		);

		CMuzzleFlash::AddFlash_Mesh3D(
			m_ahMuzzleFlashGroup[ MUZZLEFLASH_TYPE_3D_STAR_1 + fmath_RandomRange( 0, 1 ) ],
			*pPos_WS,
			FireUnitDir_WS,
			m_pUserProps->fMuzzleScale_Mesh * fRandomScale,
			m_pUserProps->fMuzzleRot_Mesh + fmath_RandomFloatRange( FMATH_DEG2RAD( -15.0f ), FMATH_DEG2RAD( 15.0f ) )
		);
	}

	if( !bBuddy ) {
		// Play firing sound...
		if( m_apAudioEmitterFire[m_nAudioEmitterIndex] ) {
			m_apAudioEmitterFire[m_nAudioEmitterIndex]->Destroy();
			m_apAudioEmitterFire[m_nAudioEmitterIndex] = NULL;
		}

		PlaySound( m_pUserProps->pSoundGroupFire, 1.0f, 1.0f, -1.0f, NULL, &m_apAudioEmitterFire[m_nAudioEmitterIndex] );

		if( ++m_nAudioEmitterIndex >= MAX_SIMULTANEOUS_FIRE_SOUNDS ) {
			m_nAudioEmitterIndex = 0;
		}

		// Assume AI for muzzle light radius...
		f32 fMuzzleLightRadius = 10.0f;

		// Generate rumble...
		if( nPlayerIndex >= 0 ) {
			fMuzzleLightRadius = 15.0f;

			fforce_Kill( &m_hForce );

			switch( m_nUpgradeLevel ) {
			case 0:
				fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce );
				break;

			case 1:
				fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_HIGH, &m_hForce );
				break;

			default:
				fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MAX, &m_hForce );
				break;
			}
		}

		// Spawn muzzle light...
		CMuzzleFlash::MuzzleLight( pPos_WS, fMuzzleLightRadius, fmath_RandomFloatRange( 0.5f, 1.0f ) );
	}

	CFVec3A RayEnd;
	FCollImpact_t CollImpact;

	RayEnd.Mul( FireUnitDir_WS, m_pInfo->fMaxLiveRange ).Add( *pPos_WS );

	FWorld_nTrackerSkipListCount = 0;
	m_pOwnerBot->AppendTrackerSkipList();
	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, pPos_WS, &RayEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, m_pResourceData->m_pWorldMesh, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
		// Hit something...

		RayEnd = CollImpact.ImpactPoint;

		// Extract the material we hit...
		const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( &CollImpact );

		// Draw impact particles...
		pCollMaterial->DrawAllDrawableParticles(
			&CollImpact.ImpactPoint,
			&CollImpact.UnitFaceNormal,
			FALSE,
			fmath_RandomFloatRange( 0.2f, 1.0f ),
			fmath_RandomFloatRange( 0.2f, 0.5f ),
			fmath_RandomFloatRange( 0.1f, 0.3f )
		);

		// Spawn some chunks...
		_SpawnImpactChunks( pCollMaterial, &CollImpact );

		// Draw an impact flash...
		if( pCollMaterial->IsImpactFlashEnabled() ) {
			CFVec3A ImpactFlashPos;
			ImpactFlashPos.Mul( CollImpact.UnitFaceNormal, m_pUserProps->fMuzzleOffset_Impact ).Add( CollImpact.ImpactPoint );

			CMuzzleFlash::AddFlash_CardPosterXY(
				m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
				ImpactFlashPos,
				m_pUserProps->fMuzzleScale_Impact,
				m_pUserProps->fMuzzleAlpha_Impact
			);
		}

		CFDecal::Create( m_pUserProps->hDecalDef, &CollImpact, &FireUnitDir_WS );

		// Damage the entity we hit...
		CEntity *pEntity = CGColl::ExtractEntity( &CollImpact );
		if( pEntity ) {
			// Hit an entity...

			CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

			if( pDamageForm ) {
				// Fill out the form...

				pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
				pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
				pDamageForm->m_pDamageProfile = m_pUserProps->pDamageProfile;
				pDamageForm->m_Damager.pWeapon = this;
				pDamageForm->m_Damager.pBot = GetOwner();
				pDamageForm->m_Damager.nDamagerPlayerIndex = GetOwner() ? GetOwner()->m_nPossessionPlayerIndex : -1;
				pDamageForm->m_pDamageeEntity = pEntity;
				pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *)CollImpact.pTag, &CollImpact, &FireUnitDir_WS );

				CDamage::SubmitDamageForm( pDamageForm );
			}
		}

		// Make an AI impact sound in case any AI are listening...
		if( GetOwner() && (GetOwner()->m_nPossessionPlayerIndex > -1) && (m_fSecsUntilNextSound <= 0.0f) ) {
			AIEnviro_AddSound( CollImpact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, this );
			m_fSecsUntilNextSound = 0.5f;
		}
	}

	// Draw a tracer...
	BOOL bMakeTracer = FALSE;

	if( nPlayerIndex < 0 ) {
		// AI...

		bMakeTracer = fmath_RandomChance( 0.4f );
	} else {
		// Human-controlled...

		if( m_fSecondsUntilNextTracer <= 0.0f ) {
			m_fSecondsUntilNextTracer = fmath_RandomFloatRange( 0.1f, 0.25f );
			bMakeTracer = TRUE;
		}
	}

	if( bMakeTracer ) {
		f32 fTracerLen = pPos_WS->Dist( RayEnd );

		CMuzzleFlash::AddFlash_CardPosterZ(
			m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1],
			*pPos_WS,
			FireUnitDir_WS,
			0.15f,
			fTracerLen,
			0.2f
		);
	}
}


void CWeaponSpew::_SpawnImpactChunks( const CGCollMaterial *pCollMaterial, const FCollImpact_t *pCollImpact ) {
	if( m_fDebrisSecondsCountdownTimer > 0.0f ) {
		return;
	}

	if( m_nUpgradeLevel == 0 ) {
		return;
	}

	u32 nMinDebrisCount, nMaxDebrisCount;

	if( m_nUpgradeLevel == 1 ) {
		nMinDebrisCount = 1;
		nMaxDebrisCount = 1;
	} else {
		nMinDebrisCount = 1;
		nMaxDebrisCount = 2;
	}

	CFDebrisSpawner DebrisSpawner;
	DebrisSpawner.InitToDefaults();

	DebrisSpawner.m_Mtx.m_vPos = pCollImpact->ImpactPoint;
	DebrisSpawner.m_Mtx.m_vZ = pCollImpact->UnitFaceNormal;
	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
	DebrisSpawner.m_pDebrisGroup = pCollMaterial->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_MEDIUM ];
	DebrisSpawner.m_fMinSpeed = 10.0f;
	DebrisSpawner.m_fMaxSpeed = 20.0f;
	DebrisSpawner.m_fUnitDirSpread = 0.2f;
	DebrisSpawner.m_nMinDebrisCount = nMinDebrisCount;
	DebrisSpawner.m_nMaxDebrisCount = nMaxDebrisCount;

	CGColl::SpawnDebris( &DebrisSpawner );

	_PickRandomDebrisChunkTime();
}


void CWeaponSpew::_UpdateSmoke( void ) {
	if( !IsDrawEnabled() || (m_fUnitHeatup <= _SMOKE_UNIT_THRESHOLD) ) {
		// Smoke should be off...

		_KillSmoke();
	} else {
		if( m_pUserProps->hParticleSmoke ) {
			// Smoke should be on...

			f32 fUnitIntensity = (m_fUnitHeatup - _SMOKE_UNIT_THRESHOLD) * (1.0f / (1.0f-_SMOKE_UNIT_THRESHOLD)) * m_pUserProps->fMaxSmokeUnitIntensity;

			if( m_pParticleSmoke ) {
				// Smoke is already on. Update intensity...
				m_pParticleSmoke->SetUnitIntensity( fUnitIntensity );
			} else {
				// Smoke is not on yet. Turn it on...
				m_pParticleSmoke = eparticlepool_GetEmitter();

				if( m_pParticleSmoke ) {
					const CFMtx43A *pBoneMtx_WS;
					CFMtx43A Mtx;

					pBoneMtx_WS = m_pResourceData->m_pFireMtx_WS;

					Mtx.m_vX = pBoneMtx_WS->m_vX;
					Mtx.m_vY.ReceiveNegative( pBoneMtx_WS->m_vY );
					Mtx.m_vZ.ReceiveNegative( pBoneMtx_WS->m_vZ );
					Mtx.m_vPos = pBoneMtx_WS->m_vPos;

					m_pParticleSmoke->StartEmission( m_pUserProps->hParticleSmoke, fUnitIntensity );

					m_pParticleSmoke->Relocate_RotXlatFromScaledMtx_WS_NewScale_WS( &Mtx, Mtx.m_vX.Mag(), 1.0f, FALSE );
					m_pParticleSmoke->Attach_ToParent_WS( this, m_pUserProps->apszBoneName[_BONE_PRIMARY_FIRE], FALSE );
				}
			}
		}
	}
}


void CWeaponSpew::_KillSmoke( void ) {
	if( m_pParticleSmoke && m_pParticleSmoke->IsCreated() ) {
		m_pParticleSmoke->DetachFromParent();
		m_pParticleSmoke->StopEmission();
		m_pParticleSmoke = NULL;
	}
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponSpew::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_bFireThisFrame = FALSE;
	m_fUnitHeatup = 0.0f;
	m_fSecondsCountdownTimer = 0.0f;
	m_fCartridgeRotSpeed = 0.0f;
}


void CWeaponSpew::BeginReload( void ) {
	TransferFromReserveToClip( (u16)m_pUserProps->fClipAmmoMax );
	ReloadComplete();
}


void CWeaponSpew::NotifyAmmoMightHaveChanged( void ) {
}


void CWeaponSpew::Clip_AttachToOwnerBotBone( cchar *pszBoneName ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();

	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );
}


void CWeaponSpew::Clip_AttachToWeapon( void ) {
	if( IsInWorld() ) {
		_AddClipToWorld();

		PlaySound( m_pUserProps->pSoundGroupAttachClip );
	}

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	Reload();
}


void CWeaponSpew::Clip_DiscardAttachedToOwnerBotBone( void ) {
	if( !(m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED) ) {
		m_pClipMeshEntity->RemoveFromWorld();
	}
}


void CWeaponSpew::Clip_Eject( void ) {
	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	_RemoveClipFromWorld();

	PlaySound( m_pUserProps->pSoundGroupEjectClip );

	EjectClip( &m_pClipMeshEntity->MtxToWorld()->m_vPos, m_pUserProps->pMeshEjectClip );
}


void CWeaponSpew::Clip_SlapIn( void ) {
	PlaySound( m_pUserProps->pSoundGroupSlapInClip );
}


void CWeaponSpew::_CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	// This callback gets called for the cartridge and the barrel for the level 3 weapons.
	// Currently, they spin at the same speed.

	CFMtx43A::m_RotZ.SetRotationZ( m_pCallbackThis->m_fCartridgeSpinAngle );
	rNewMtx.Mul( rParentMtx, rBoneMtx );
	rNewMtx.Mul( CFMtx43A::m_RotZ );
}


void CWeaponSpew::CheckpointSaveSelect( s32 nCheckpoint ) {
	// Save weapon's clip...
	if( m_pClipMeshEntity ) {
		m_pClipMeshEntity->CheckpointSaveSelect( nCheckpoint );
	}

	// Save self...
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


void CWeaponSpew::CheckpointRestore( void ) {
	_KillAudioEmitters();
	CWeapon::CheckpointRestore();
}


