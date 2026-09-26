//////////////////////////////////////////////////////////////////////////////////////
// weapon_flamer.cpp - Small Projectile Emitter Weapon.
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
#include "weapon_flamer.h"
#include "fgamedata.h"
#include "fworld.h"
#include "fmesh.h"
#include "meshtypes.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "ItemInst.h"
#include "fsndfx.h"
#include "fresload.h"
#include "meshentity.h"
#include "flamer.h"
#include "damage.h"
#include "AI/AIEnviro.h"




#define _USER_PROP_FILENAME		"w_flamer.csv"
#define _MAX_CLIP_AMMO			100




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponFlamerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponFlamerBuilder _WeaponFlamerBuilder;

void CWeaponFlamerBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONFLAMER, pszEntityType );
}


BOOL CWeaponFlamerBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponFlamerBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponFlamer
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponFlamer::_UserProps_t CWeaponFlamer::m_aUserProps[EUK_COUNT_FLAMER];
CDamageProfile *CWeaponFlamer::m_apDamageProfileHit[EUK_COUNT_FLAMER];
CDamageProfile *CWeaponFlamer::m_apDamageProfileBurn[EUK_COUNT_FLAMER];


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponFlamer::m_aUserPropVocab[] = {
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

	// pszDamageProfileHit:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszDamageProfileBurn:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_MESH,			// pMeshEjectClip

	// fSoundRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000000,

	// hFiringSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hFadeOutSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hAttachClipSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hSlapInSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hEjectSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hEmptyClickSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponFlamer::m_aUserPropMapTable[] = {
	"FlamerL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],
	
	"FlamerMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"FlamerMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"FlamerSlosh",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],

	NULL
};


BOOL CWeaponFlamer::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponFlamer::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_FLAMER; i++ ) {
		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_FLAMER][i];

		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_CLIP;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->fUnitRecoil = m_aUserProps[i].fUnitRecoil;
		pInfo->nClipAmmoMax = _MAX_CLIP_AMMO;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_NO_AUTO_TARGETING | INFOFLAG_LEFT_HAND_RELOADS;
		pInfo->nReticleType = CReticle::TYPE_FLAMER;

		m_apDamageProfileHit[i] = CDamage::FindDamageProfile( m_aUserProps[i].pszDamageProfileHit );
		m_apDamageProfileBurn[i] = CDamage::FindDamageProfile( m_aUserProps[i].pszDamageProfileBurn );
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponFlamer::UninitSystem( void ) {
}



CWeaponFlamer::CWeaponFlamer() {
	_ClearDataMembers();
}



CWeaponFlamer::~CWeaponFlamer() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}



BOOL CWeaponFlamer::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponFlamerBuilder *pBuilder = (CWeaponFlamerBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponFlamerBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONFLAMER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_FLAMER, pszEntityName, pMtx, pszAIBuilderName );
}



BOOL CWeaponFlamer::ClassHierarchyBuild( void ) {
	u32 i;
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	_ResourceData_t *pResourceData;
	Info_t *pInfo;
	u32 uMeshEUK;
	u32 uResEUK;


	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponFlamerBuilder *pBuilder = (CWeaponFlamerBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_FLAMER; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_FLAMER );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		pInfo = &m_aaInfo[WEAPON_TYPE_FLAMER][i];

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponFlamer::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );

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
		//pResourceData->m_pWorldMesh->Init( &MeshInit );

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
				DEVPRINTF( "CWeaponTether::Create(): Could not create animation combiner.\n" );
				goto _ExitWithError;
			}
		}

		s32 nBoneIndex = pResourceData->m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE] );
		if( nBoneIndex < 0 ) {
			DEVPRINTF( "CWeaponFlamer::ClassHierarchyBuild(): Bone '%s' doesn't exist in object '%s'.\n", m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE], Name() );
			goto _ExitWithError;
		}
		m_apFirePos_WS[i] = &pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Build our clip mesh entity...
	m_pClipMeshEntity = fnew CMeshEntity;
	if( m_pClipMeshEntity == NULL ) {
		DEVPRINTF( "CWeaponFlamer::Create(): Not enough memory to create the clip CMeshEntity.\n" );
		goto _ExitWithError;
	}

	if( !m_pClipMeshEntity->Create( m_aUserProps[0].apszMeshName[_MESH_CLIP] ) ) {
		DEVPRINTF( "CWeaponFlamer::Create(): Could not create the clip CMeshEntity.\n" );
		goto _ExitWithError;
	}

	m_pClipMeshEntity->SetCollisionFlag( FALSE );
	m_pClipMeshEntity->SetLineOfSightFlag( FALSE );
	m_pClipMeshEntity->SetCullDist( m_aUserProps[0].fClipCullDist );
	m_pClipMeshEntity->MeshFlip_SetFlipsPerSec( 0.0f );
	m_pClipMeshEntity->RemoveFromWorld();

	// Create our flamer...
	m_pFlamer = fnew CFlamer;
	if( m_pFlamer == NULL ) {
		DEVPRINTF( "CWeaponFlamer::Create(): Not enough memory to create the clip m_pFlamer.\n" );
		goto _ExitWithError;
	}

	if( !m_pFlamer->Create( this, m_apDamageProfileHit[0], m_apDamageProfileBurn[0], CFlamer::FLAMER_DEFAULT_STREAM_NODE_COUNT, m_aUserProps[0].hFiringSound, m_aUserProps[0].hFadeOutSound ) ) {
		DEVPRINTF( "CWeaponFlamer::Create(): Could not create m_pFlamer.\n" );
		goto _ExitWithError;
	}

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	fforce_NullHandle( &m_hForce );

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponFlamer::ClassHierarchyBuilt( void ) {
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


void CWeaponFlamer::ClassHierarchyDestroy( void ) {
	u32 i;

	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	for( i=0; i<EUK_COUNT_FLAMER; ++i ) {
		DestroySharedEUKData( i );
	}

	fdelete( m_pClipMeshEntity );
	fdelete( m_pFlamer );

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponFlamer::GetLeafClassBuilder( void ) {
	return &_WeaponFlamerBuilder;
}


void CWeaponFlamer::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_FLAMER; ++i ) {
		m_aResourceData[i].m_pAnimCombiner = NULL;
		m_aResourceData[i].m_pAnimRest = NULL;
		m_aResourceData[i].m_pWorldMesh = NULL;
	}

	m_pResourceData = NULL;
	m_pUserProps = NULL;
	fforce_NullHandle( &m_hForce );

	m_pClipMeshEntity = NULL;
	m_pFlamer = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_fTrigThisFrame = 0.0f;
	m_TargetPos_WS.Zero();
	m_pBuddyFirePos_WS = NULL;
}


void CWeaponFlamer::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponFlamer::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst(pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_METER, CItemInst::AMMODISPLAY_NUMERIC );
	}
}

void CWeaponFlamer::CheckpointRestore( void ) {
	CWeapon::CheckpointRestore();

	if( m_pFlamer ) {
		m_pFlamer->StopFlamer();
	}
}

void CWeaponFlamer::_AddClipToWorld( void ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_pUserProps->apszBoneName[_BONE_CLIP_ATTACH], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();
}


void CWeaponFlamer::_RemoveClipFromWorld( void ) {
	m_pClipMeshEntity->DetachFromParent();
	m_pClipMeshEntity->RemoveFromWorld();
}


void CWeaponFlamer::ClassHierarchyAddToWorld( void ) {
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


void CWeaponFlamer::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();

	// Remove clip from the world...
	if( m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED ) {
		_RemoveClipFromWorld();
	}
}


void CWeaponFlamer::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	u32 i;

	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this weapon...

		for( i=0; i<EUK_COUNT_FLAMER; ++i ) {
			FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );
	} else {
		// Disable drawing of this weapon...
		for( i=0; i<EUK_COUNT_FLAMER; ++i ) {
			FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );
	}
}


void CWeaponFlamer::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );
		RelocateAllChildren();
	}
}


CFMtx43A *CWeaponFlamer::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponFlamer::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponFlamer::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;

	// Add our clip to the list...
	m_pClipMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}


// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponFlamer::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_FLAMER][nPreviousUpgradeLevel];

	if( m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED ) {
		_RemoveClipFromWorld();
	}

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


void CWeaponFlamer::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	*pMuzzlePoint_WS = *m_apFirePos_WS[m_nUpgradeLevel];
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
u32 CWeaponFlamer::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );

	m_fTrigThisFrame = 0.0f;

	if( fUnitTriggerVal1 == 0.0f ) {
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

				if( IsOwnedByPlayer() ) {
					fsndfx_Play2D( m_pUserProps->hEmptyClickSound );
				} else {
					fsndfx_Play3D( m_pUserProps->hEmptyClickSound, &m_MtxToWorld.m_vPos, m_pUserProps->fSoundRadius );
				}
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

	m_TargetPos_WS.Set( *pvTargetPos_WS );

	m_fTrigThisFrame = fUnitTriggerVal1;
	m_pBuddyFirePos_WS = pBuddyFirePos_WS;

	return 1;

_ExitWithoutTriggerPressed:
	return 0;
}


// Called once per frame to update animations, timers, etc.
void CWeaponFlamer::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}
	
	CFVec3A MuzzlePoint;
	CFVec3A vUnitFireDir;

	ComputeMuzzlePoint_WS( &MuzzlePoint );
	ComputeFireUnitDir( &vUnitFireDir, &MuzzlePoint, &m_MtxToWorld.m_vFront, &m_TargetPos_WS, FALSE );

	if( m_pFlamer->TriggerWork( m_fTrigThisFrame, &vUnitFireDir, &MuzzlePoint, &m_pOwnerBot->m_Velocity_WS ) ) {
		// Fired...
		if( GetOwner() && (GetOwner()->m_nPossessionPlayerIndex >= 0) ) {
			AIEnviro_BoostPlayerSoundTo( GetOwner()->m_nPossessionPlayerIndex, 15.0f );
			GetOwner()->JustFired();
		}

		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		while( m_fSecondsCountdownTimer <= 0.0f ) {
			m_fSecondsCountdownTimer += 0.05f;
			RemoveFromClip( 1 );
		}
	}
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponFlamer::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_fTrigThisFrame = 0.0f;
	m_fSecondsCountdownTimer = 0.0f;
}


void CWeaponFlamer::BeginReload( void ) {
	TransferFromReserveToClip( _MAX_CLIP_AMMO );
	ReloadComplete();
}


void CWeaponFlamer::NotifyAmmoMightHaveChanged( void ) {
}


void CWeaponFlamer::Clip_AttachToOwnerBotBone( cchar *pszBoneName ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();

	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );
}


void CWeaponFlamer::Clip_AttachToWeapon( void ) {
	if( IsInWorld() ) {
		_AddClipToWorld();

		if( IsOwnedByPlayer() ) {
			fsndfx_Play2D( m_pUserProps->hAttachClipSound );
		} else {
			fsndfx_Play3D( m_pUserProps->hAttachClipSound, &m_MtxToWorld.m_vPos, m_pUserProps->fSoundRadius );
		}
	}

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	Reload();
}


void CWeaponFlamer::Clip_DiscardAttachedToOwnerBotBone( void ) {
	if( !(m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED) ) {
		m_pClipMeshEntity->RemoveFromWorld();
	}
}


void CWeaponFlamer::Clip_Eject( void ) {
	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	_RemoveClipFromWorld();

	if( IsOwnedByPlayer() ) {
		fsndfx_Play2D( m_pUserProps->hEjectSound );
	} else {
		fsndfx_Play3D( m_pUserProps->hEjectSound, &m_MtxToWorld.m_vPos, m_pUserProps->fSoundRadius );
	}

	EjectClip( &m_pClipMeshEntity->MtxToWorld()->m_vPos, m_pUserProps->pMeshEjectClip,m_pClipMeshEntity->ScaleToWorld() );
}


void CWeaponFlamer::Clip_SlapIn( void ) {
	if( IsOwnedByPlayer() ) {
		fsndfx_Play2D( m_pUserProps->hSlapInSound );
	} else {
		fsndfx_Play3D( m_pUserProps->hSlapInSound, &m_MtxToWorld.m_vPos, m_pUserProps->fSoundRadius );
	}
}



