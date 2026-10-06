//////////////////////////////////////////////////////////////////////////////////////
// weapon_mortar.cpp - Small Projectile Emitter Weapon.
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
#include "weapon_mortar.h"
#include "fgamedata.h"
#include "fworld.h"
#include "fmesh.h"
#include "fanim.h"
#include "meshtypes.h"
#include "fresload.h"
#include "floop.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "explosion.h"
#include "potmark.h"
#include "fcamera.h"
#include "ItemInst.h"
#include "meshentity.h"




#define _USER_PROP_FILENAME		"w_mortar.csv"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponMortarBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponMortarBuilder _WeaponMortarBuilder;


void CWeaponMortarBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONMORTAR, pszEntityType );
}


BOOL CWeaponMortarBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponMortarBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponMortar
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponMortar::_UserProps_t CWeaponMortar::m_aUserProps[EUK_COUNT_MORTAR];
FTexDef_t *CWeaponMortar::m_apTexDef[EUK_COUNT_MORTAR];

// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponMortar::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_FORK]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszMeshName[_MESH_SLING]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszAnimName[_SLING_ANIM_TAKE_UP_SLACK]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszAnimName[_SLING_ANIM_AIM]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszAnimName[_SLING_ANIM_FIRE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fWeaponCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fDistFromWeaponOrigToMuzzle:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupPullBack


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponMortar::m_aUserPropMapTable[] = {
	"MortarL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	NULL
};



BOOL CWeaponMortar::InitSystem( void ) {
	Info_t *pInfo;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponMortar::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Fill out our global info data...
	pInfo = &m_aaInfo[WEAPON_TYPE_MORTAR][0];

	// Do any special limitations on the values we got from the CSV file.

	pInfo->nGrip = GRIP_RIGHT_ARM;
	pInfo->nReloadType = RELOAD_TYPE_MORTAR;
	pInfo->nStanceType = STANCE_TYPE_MORTAR;
	pInfo->fMinTargetAssistDist = 10.0f;
	pInfo->fMaxTargetAssistDist = 1000.0f;
	pInfo->fMaxLiveRange = 1000.0f;
	pInfo->nClipAmmoMax = INFINITE_AMMO;
	pInfo->nReserveAmmoMax = INFINITE_AMMO;
	pInfo->nInfoFlags = INFOFLAG_LEFT_HAND_RELOADS | INFOFLAG_NO_AMMO;
	pInfo->nReticleType = CReticle::TYPE_NONE;

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponMortar::UninitSystem( void ) {
}


CWeaponMortar::CWeaponMortar() {
	_ClearDataMembers();
}


CWeaponMortar::~CWeaponMortar() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponMortar::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponMortarBuilder *pBuilder = (CWeaponMortarBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponMortarBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONMORTAR );
	}

	m_pSoundEmitter = NULL;

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_MORTAR, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponMortar::ClassHierarchyBuild( void ) {
	u32 i;
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	Info_t *pInfo;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponMortarBuilder *pBuilder = (CWeaponMortarBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	pInfo = &m_aaInfo[WEAPON_TYPE_MORTAR][0];
	m_pResourceData = &m_aResourceData[0];

	// Build our fork anim combiner config...
	m_pAnimCombinerConfig = fnew CFAnimCombinerConfig;
	if( m_pAnimCombinerConfig == NULL ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Not enough memory to create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->BeginCreation( "AimFire", CFAnimMixer::TYPE_BLENDER ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddMixer( "Reload", CFAnimMixer::TYPE_BLENDER, "AimFire", 0 ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create m_pAnimCombinerConfig mixer 'Reload'.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "Rest", "Reload", 0 ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create 'Reload' Tap 0 for m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "Reload", "Reload", 1 ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create 'Reload' Tap 1 for m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "AimFire", "AimFire", 1 ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create 'AimFire' Tap 1 for m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->EndCreation() ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Trouble creating m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	// Create the fork world mesh...
	m_pResourceData->m_pForkWorldMesh = fnew CFWorldMesh;
	if( m_pResourceData->m_pForkWorldMesh == NULL ) {
		DEVPRINTF( "CWeaponMortar::Create(): Not enough memory to create m_pForkWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Load the mesh resource...
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[0].apszMeshName[_MESH_FORK] );
	if( pMesh == NULL ) {
		DEVPRINTF( "CWeaponMortar::Create(): Could not find mesh '%s'.\n",m_aUserProps[0].apszMeshName[_MESH_FORK] );

		pMesh = m_pErrorMesh;
		if( pMesh == NULL ) {
			goto _ExitWithError;
		}
	}

	// Init the world mesh...
	MeshInit.pMesh = pMesh;
	MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
	MeshInit.fCullDist = m_aUserProps[0].fWeaponCullDist;
	MeshInit.Mtx = *MtxToWorld();
	m_pResourceData->m_pForkWorldMesh->Init( &MeshInit );
	m_pResourceData->m_pForkWorldMesh->RemoveFromWorld();
	m_pResourceData->m_pForkWorldMesh->SetCollisionFlag( TRUE );
	m_pResourceData->m_pForkWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pResourceData->m_pForkWorldMesh->m_pUser = this;
	m_pResourceData->m_pForkWorldMesh->SetUserTypeBits( TypeBits() );

	// Build our anim combiner...
	m_pResourceData->m_pForkAnimCombiner = fnew CFAnimCombiner;
	if( m_pResourceData->m_pForkAnimCombiner == NULL ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Not enough memory to create m_pForkAnimCombiner.\n" );
		goto _ExitWithError;
	}

	if( !m_pResourceData->m_pForkAnimCombiner->Create( m_pAnimCombinerConfig, m_pResourceData->m_pForkWorldMesh ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create m_pForkAnimCombiner.\n" );
		goto _ExitWithError;
	}

	// Get tap IDs...
	m_pResourceData->m_anForkTapID[_ANIM_TAP_REST] = m_pResourceData->m_pForkAnimCombiner->GetTapID( "Rest" );
	m_pResourceData->m_anForkTapID[_ANIM_TAP_RELOAD] = m_pResourceData->m_pForkAnimCombiner->GetTapID( "Reload" );
	m_pResourceData->m_anForkTapID[_ANIM_TAP_AIMFIRE] = m_pResourceData->m_pForkAnimCombiner->GetTapID( "AimFire" );
	for( i=0; i<_ANIM_TAP_COUNT; ++i ) {
		if( m_pResourceData->m_anForkTapID[i] < 0 ) {
			DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not find Tap %u in m_pForkAnimCombiner.\n", i );
			goto _ExitWithError;
		}
	}

	// Get control IDs...
	m_pResourceData->m_anForkControlID[_ANIM_CONTROL_RELOAD] = m_pResourceData->m_pForkAnimCombiner->GetControlID( "Reload" );
	m_pResourceData->m_anForkControlID[_ANIM_CONTROL_AIMFIRE] = m_pResourceData->m_pForkAnimCombiner->GetControlID( "AimFire" );
	for( i=0; i<_ANIM_CONTROL_COUNT; ++i ) {
		if( m_pResourceData->m_anForkControlID[i] < 0 ) {
			DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not find Control %u in m_pForkAnimCombiner.\n", i );
			goto _ExitWithError;
		}
	}

	// Create our rest anim...
	m_pResourceData->m_pForkAnimMeshRest = fnew CFAnimMeshRest;
	if( m_pResourceData->m_pForkAnimMeshRest == NULL ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Not enough memory to create m_pForkAnimMeshRest.\n" );
		goto _ExitWithError;
	}

	if( !m_pResourceData->m_pForkAnimMeshRest->Create( pMesh ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create m_pForkAnimMeshRest.\n" );
		goto _ExitWithError;
	}

	// Attach our rest anim to the "Rest" tap...
	m_pResourceData->m_pForkAnimCombiner->AttachToTap( m_pResourceData->m_anForkTapID[_ANIM_TAP_REST], m_pResourceData->m_pForkAnimMeshRest );

	// Create a CMeshEntity for the sling...
	m_pResourceData->m_pSlingMeshEntity = fnew CMeshEntity;
	if( m_pResourceData->m_pSlingMeshEntity == NULL ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Not enough memory to create m_pSlingMeshEntity.\n" );
		goto _ExitWithError;
	}

	if( !m_pResourceData->m_pSlingMeshEntity->Create( m_aUserProps[0].apszMeshName[_MESH_SLING], _SLING_ANIM_COUNT, m_aUserProps[0].apszAnimName ) ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyBuild(): Could not create m_pSlingMeshEntity.\n" );
		goto _ExitWithError;
	}

	m_pResourceData->m_pSlingMeshEntity->DriveMeshWithAnim( TRUE );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( 0 );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_SetClampMode( TRUE );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( TRUE );
	m_pResourceData->m_pSlingMeshEntity->EnableAutoWork( FALSE );
	m_pResourceData->m_pSlingMeshEntity->RemoveFromWorld();

	fforce_NullHandle( &m_hForce );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponMortar::ClassHierarchyBuilt( void ) {
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


void CWeaponMortar::ClassHierarchyDestroy( void ) {
	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	fdelete( m_aResourceData[0].m_pForkWorldMesh );
	fdelete( m_aResourceData[0].m_pForkAnimCombiner );
	fdelete( m_aResourceData[0].m_pForkAnimMeshRest );
	fdelete( m_aResourceData[0].m_pSlingMeshEntity );
	fdelete( m_pAnimCombinerConfig );

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponMortar::GetLeafClassBuilder( void ) {
	return &_WeaponMortarBuilder;
}


void CWeaponMortar::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<_ANIM_TAP_COUNT; ++i ) {
		m_aResourceData[0].m_anForkTapID[i] = -1;
	}

	for( i=0; i<_ANIM_CONTROL_COUNT; ++i ) {
		m_aResourceData[0].m_anForkControlID[i] = -1;
	}

	m_aResourceData[0].m_pForkWorldMesh = NULL;
	m_aResourceData[0].m_pForkAnimCombiner = NULL;
	m_aResourceData[0].m_pSlingMeshEntity = NULL;
	m_aResourceData[0].m_pForkAnimMeshRest = NULL;

	m_pResourceData = NULL;
	m_bAbortForkAnims = FALSE;
	m_nSlingMode = SLING_MODE_IDLE;
	m_fUnitStretch = 0.0f;
	m_fSecondsCountdownTimer = 0.0f;
	m_pAnimCombinerConfig = NULL;

	m_nNextFreeTriggerSampleIndex = 0;
	m_nTriggerSampleCount = 0;

	for( i=0; i<_MAX_SAMPLE_COUNT; ++i ) {
		m_afTriggerValue[i] = 0.0f;
		m_afTriggerDeltaTime[i] = 0.0f;
	}
}


void CWeaponMortar::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponMortar::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst(pItemInst,bUpdateItemInstAmmoFromWeaponAmmo);
	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NONE, CItemInst::AMMODISPLAY_NONE );
	}
}


void CWeaponMortar::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pForkWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pForkWorldMesh->UpdateTracker();

	m_pResourceData->m_pSlingMeshEntity->Attach_UnitMtxToParent_PS( this, "SlingShot", &CFMtx43A::m_IdentityMtx, TRUE );
	m_pResourceData->m_pSlingMeshEntity->AddToWorld();

	m_nSlingMode = SLING_MODE_IDLE;
	m_fUnitStretch = 0.0f;
	m_fUnitLaunchSpeed = 0.0f;

	Mortar_Fork_Idle();
	Mortar_Sling_Idle();
}


void CWeaponMortar::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pForkWorldMesh->RemoveFromWorld();
	m_pResourceData->m_pSlingMeshEntity->RemoveFromWorld();
	m_pResourceData->m_pSlingMeshEntity->DetachFromParent();

	m_nSlingMode = SLING_MODE_IDLE;
	m_fUnitStretch = 0.0f;
	m_fUnitLaunchSpeed = 0.0f;

	Mortar_Fork_Idle();
	Mortar_Sling_Idle();
}

void CWeaponMortar::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	u32 i;
	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this weapon...

		for( i=0; i<EUK_COUNT_MORTAR; ++i ) {
			FMATH_CLEARBITMASK( m_aResourceData[i].m_pForkWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			m_aResourceData[i].m_pSlingMeshEntity->DrawEnable(bDrawingHasBeenEnabled);
		}
	} else {
		// Disable drawing of this weapon...

		for( i=0; i<EUK_COUNT_MORTAR; ++i ) {
			FMATH_SETBITMASK( m_aResourceData[i].m_pForkWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			m_aResourceData[i].m_pSlingMeshEntity->DrawEnable(bDrawingHasBeenEnabled);
		}
	}
}


void CWeaponMortar::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pForkWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pForkWorldMesh->UpdateTracker();
	}
}


void CWeaponMortar::Mortar_Fork_Idle( void ) {
	FASSERT( IsCreated() );
	Mortar_Fork_AttachReloadAnim( NULL );
	Mortar_Fork_AttachAimFireAnim( NULL );
	Mortar_Fork_SetReloadAnimControl( 0.0f );
	Mortar_Fork_SetAimFireAnimControl( 0.0f );
}


void CWeaponMortar::Mortar_Fork_Abort( void ) {
	FASSERT( IsCreated() );
	m_bAbortForkAnims = TRUE;
}


void CWeaponMortar::Mortar_Fork_AttachReloadAnim( CFAnimSource *pAnimSource ) {
	FASSERT( IsCreated() );
	m_aResourceData[0].m_pForkAnimCombiner->AttachToTap( m_aResourceData[0].m_anForkTapID[_ANIM_TAP_RELOAD], pAnimSource );
	m_bAbortForkAnims = FALSE;
}


void CWeaponMortar::Mortar_Fork_AttachAimFireAnim( CFAnimSource *pAnimSource ) {
	FASSERT( IsCreated() );
	m_aResourceData[0].m_pForkAnimCombiner->AttachToTap( m_aResourceData[0].m_anForkTapID[_ANIM_TAP_AIMFIRE], pAnimSource );
	m_bAbortForkAnims = FALSE;
}


void CWeaponMortar::Mortar_Fork_SetReloadAnimControl( f32 fUnitControl ) {
	FASSERT( IsCreated() );
	m_aResourceData[0].m_pForkAnimCombiner->SetControlValue( m_aResourceData[0].m_anForkControlID[_ANIM_CONTROL_RELOAD], fUnitControl );
	m_bAbortForkAnims = FALSE;
}


void CWeaponMortar::Mortar_Fork_SetAimFireAnimControl( f32 fUnitControl ) {
	FASSERT( IsCreated() );
	m_aResourceData[0].m_pForkAnimCombiner->SetControlValue( m_aResourceData[0].m_anForkControlID[_ANIM_CONTROL_AIMFIRE], fUnitControl );
	m_bAbortForkAnims = FALSE;
}


void CWeaponMortar::Mortar_Sling_Idle( void ) {
	FASSERT( IsCreated() );

	m_nSlingMode = SLING_MODE_IDLE;

	m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( _SLING_ANIM_TAKE_UP_SLACK );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateTime( 0.0f );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( TRUE );
}


void CWeaponMortar::Mortar_Sling_TakeUpSlack( void ) {
	FASSERT( IsCreated() );

	m_nSlingMode = SLING_MODE_TAKING_UP_SLACK;

	m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( _SLING_ANIM_TAKE_UP_SLACK );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateTime( 0.0f );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( FALSE );

	PlaySound( m_aUserProps[m_nUpgradeLevel].pSoundGroupPullBack );
}


void CWeaponMortar::Mortar_Sling_Release( BOOL bPlayFireSound ) {
	f32 fUnit;
	BOOL bOkToPlaySound = FALSE;

	FASSERT( IsCreated() );

	switch( m_nSlingMode ) {
	case SLING_MODE_IDLE:
		// Nothing to do...
		break;

	case SLING_MODE_TAKING_UP_SLACK:
		m_nSlingMode = SLING_MODE_FIRING;

		m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( _SLING_ANIM_FIRE );
		m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateTime( 0.9f );
		m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( FALSE );

		bOkToPlaySound = TRUE;

		break;

	case SLING_MODE_AIMING:
		m_nSlingMode = SLING_MODE_FIRING;

		fUnit = m_aResourceData[0].m_pForkAnimCombiner->GetControlValue( m_aResourceData[0].m_anForkControlID[_ANIM_CONTROL_AIMFIRE] );

		m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( _SLING_ANIM_FIRE );
		m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateTime( 0.8f - (0.8f * m_fUnitStretch * fUnit) );
		m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( FALSE );

		bOkToPlaySound = TRUE;

		break;

	case SLING_MODE_FIRING:
		// Already firing...
		break;
	}

	if( bOkToPlaySound && bPlayFireSound ) {
		PlaySound( m_aUserProps[m_nUpgradeLevel].pSoundGroupFire );
	}
}


CFMtx43A *CWeaponMortar::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pForkWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponMortar::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pForkWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponMortar::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pForkWorldMesh;
}


// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponMortar::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_MORTAR][nPreviousUpgradeLevel];

	// Remove old mesh from world...
	pOldResourceData->m_pForkWorldMesh->RemoveFromWorld();

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	if( IsInWorld() ) {
		m_pResourceData->m_pForkWorldMesh->m_Xfm = pOldResourceData->m_pForkWorldMesh->m_Xfm;
		m_pResourceData->m_pForkWorldMesh->UpdateTracker();
	}

	m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( _SLING_ANIM_TAKE_UP_SLACK );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateTime( 0.0f );
	m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( TRUE );
}


void CWeaponMortar::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	pMuzzlePoint_WS->Mul( m_MtxToWorld.m_vFront, m_aUserProps[m_nUpgradeLevel].fDistFromWeaponOrigToMuzzle ).Add( m_MtxToWorld.m_vPos );
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
u32 CWeaponMortar::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pProjUnitDir_WS, const CFVec3A *pBuddyFirePos_WS ) {
	FASSERT( IsCreated() );

//	if( m_nSlingMode == SLING_MODE_AIMING ) {
//		fUnitTriggerVal1 = m_aResourceData[0].m_pForkAnimCombiner->GetControlValue( m_aResourceData[0].m_anForkControlID[_ANIM_CONTROL_AIMFIRE] );
//	}

	m_fUnitStretch = fUnitTriggerVal1;
	m_fUnitLaunchSpeed = 0.0f;

	if( m_nSlingMode != SLING_MODE_AIMING ) {
		// We have to be in aim mode to do any sort of trigger work...
		goto _NoTriggerWork;
	}

	if( CurrentState() != STATE_DEPLOYED ) {
		// Must be in deployed state to do trigger work...
		goto _NoTriggerWork;
	}

	if( (fUnitTriggerVal1 == 0.0f) && (m_nTriggerSampleCount == 0) ) {
		// Don't record samples unless the trigger has already been pressed...
		goto _NoTriggerWork;
	}

	m_fUnitStretch = fUnitTriggerVal1;

	if( FLoop_fPreviousLoopSecs > 0.000001f ) {
		m_afTriggerValue[m_nNextFreeTriggerSampleIndex] = fUnitTriggerVal1;
		m_afTriggerDeltaTime[m_nNextFreeTriggerSampleIndex] = FLoop_fPreviousLoopSecs;

		if( ++m_nNextFreeTriggerSampleIndex >= _MAX_SAMPLE_COUNT ) {
			m_nNextFreeTriggerSampleIndex = 0;
		}

		if( m_nTriggerSampleCount < _MAX_SAMPLE_COUNT ) {
			++m_nTriggerSampleCount;
		}
	}

	if( fUnitTriggerVal1 > 0.0f ) {
		return 0;
	}

	// Find the most recent trigger value peak...

	f32 fMaxTriggerValue, fElapsedTime = 0.0f, fReleaseRate;
	s32 nIndex, nIndexOfMaxValue;
	u32 i;

	fMaxTriggerValue = 0.0f;
	nIndex = m_nNextFreeTriggerSampleIndex;
	nIndexOfMaxValue = -1;

	for( i=0; i<m_nTriggerSampleCount; ++i ) {
		// Step to previous sample...
		if( nIndex-- == 0 ) {
			nIndex = (_MAX_SAMPLE_COUNT - 1);
		}

		if( m_afTriggerValue[nIndex] > fMaxTriggerValue ) {
			fMaxTriggerValue = m_afTriggerValue[nIndex];
			nIndexOfMaxValue = nIndex;
		}
	}

	if( nIndexOfMaxValue >= 0 ) {
		// At least one entry was greater than 0.0f.
		// Compute the elapsed time from the peak sample to now...

		fElapsedTime = 0.0f;
		nIndex = m_nNextFreeTriggerSampleIndex;

		while( TRUE ) {
			// Step to previous sample...
			if( nIndex-- == 0 ) {
				nIndex = (_MAX_SAMPLE_COUNT - 1);
			}

			if( nIndex == nIndexOfMaxValue ) {
				break;
			}

			fElapsedTime += m_afTriggerDeltaTime[nIndex];
		}
	}

	m_nNextFreeTriggerSampleIndex = 0;
	m_nTriggerSampleCount = 0;

#if defined(MA_PC_INPUT)
	// Digital release and a gradual analog release both fire the loaded round.
	// Frame stalls must not turn a held shot into an abort.
	if( fMaxTriggerValue <= 0.0f ) return 0;
#else
	if( fElapsedTime <= 0.000001f ) {
		return 0;
	}

	fReleaseRate = fmath_Div( fMaxTriggerValue, fElapsedTime );

	if( fReleaseRate < 10.0f ) {
		return 0;
	}
#endif

#if 0
		// Find the average of our trigger velocities...

		u32 i;
		f32 fAvgTriggerVelocity, fMaxTriggerValue;

		fAvgTriggerVelocity = 0.0f;
		fMaxTriggerValue = 0.0f;

		for( i=0; i<m_nTriggerSampleCount; ++i ) {
			fAvgTriggerVelocity += m_afTriggerVelocity[i];

			if( m_afTriggerValue[i] > fMaxTriggerValue ) {
				fMaxTriggerValue = m_afTriggerValue[i];
			}
		}

		fAvgTriggerVelocity = fMaxTriggerValue * fmath_Div( fAvgTriggerVelocity, (f32)m_nTriggerSampleCount );

		m_nNextFreeTriggerSampleIndex = 0;
		m_nTriggerSampleCount = 0;
#endif

	// Let's fire off a round...
	m_UnitFireDir_WS = *pProjUnitDir_WS;
	m_fUnitStretch = fUnitTriggerVal1;
	m_fUnitLaunchSpeed = fMaxTriggerValue;

	return 1;

	// No trigger work...
_NoTriggerWork:
	m_nNextFreeTriggerSampleIndex = 0;
	m_nTriggerSampleCount = 0;

	return 0;
}


BOOL CWeaponMortar::Mortar_IsReadyToFire( void ) {
	FASSERT( IsCreated() );

	if( m_pOwnerBot->m_apWeapon[1] == NULL ) {
		// Owner bot doesn't have secondary weapon in hand...
		return FALSE;
	}

	if( m_pOwnerBot->m_apWeapon[1]->GetClipAmmo() == 0 ) {
		// No secondary ammo...
		return FALSE;
	}

	return TRUE;
}


f32 CWeaponMortar::GetProjectileSpeed( void ) const {
	return 0.0f;
}


// Called once per frame to update animations, timers, etc.
void CWeaponMortar::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( m_bAbortForkAnims ) {
		f32 fUnitReload, fUnitAimFire;

		fUnitReload = m_pResourceData[0].m_pForkAnimCombiner->GetControlValue( m_pResourceData[0].m_anForkControlID[_ANIM_CONTROL_RELOAD] );
		if( fUnitReload > 0.0f ) {
			fUnitReload -= 3.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fUnitReload, 0.0f );

			m_pResourceData[0].m_pForkAnimCombiner->SetControlValue( m_pResourceData[0].m_anForkControlID[_ANIM_CONTROL_RELOAD], fUnitReload );
		}

		fUnitAimFire = m_pResourceData[0].m_pForkAnimCombiner->GetControlValue( m_pResourceData[0].m_anForkControlID[_ANIM_CONTROL_AIMFIRE] );
		if( fUnitAimFire > 0.0f ) {
			fUnitAimFire -= 3.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fUnitAimFire, 0.0f );

			m_pResourceData[0].m_pForkAnimCombiner->SetControlValue( m_pResourceData[0].m_anForkControlID[_ANIM_CONTROL_AIMFIRE], fUnitAimFire );
		}

		if( fUnitReload==0.0f && fUnitAimFire==0.0f ) {
			m_bAbortForkAnims = FALSE;
		}
	}

	switch( m_nSlingMode ) {
	case SLING_MODE_IDLE:
		break;

	case SLING_MODE_TAKING_UP_SLACK:
		{
			CFAnimInst *pAnimInst = m_pResourceData->m_pSlingMeshEntity->UserAnim_GetCurrentInst();
			f32 fUnitAnimProgress = pAnimInst->GetUnitTime();

//			if( fUnitAnimProgress >= 1.0f ) {
			if( fUnitAnimProgress >= 0.999f ) {
				// Anim is done playing...

				m_nSlingMode = SLING_MODE_AIMING;
				m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( _SLING_ANIM_AIM );
				m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateTime( 0.0f );
				m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( TRUE );
			}
		}

		break;

	case SLING_MODE_AIMING:
		{
			f32 fUnit;

			fUnit = m_aResourceData[0].m_pForkAnimCombiner->GetControlValue( m_aResourceData[0].m_anForkControlID[_ANIM_CONTROL_AIMFIRE] );
			m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateUnitTime( m_fUnitStretch * fUnit );
		}

		break;

	case SLING_MODE_FIRING:
		{
			CFAnimInst *pAnimInst = m_pResourceData->m_pSlingMeshEntity->UserAnim_GetCurrentInst();
			f32 fUnitAnimProgress = pAnimInst->GetUnitTime();

			if( fUnitAnimProgress >= 1.0f ) {
				// Anim is done playing...

				m_nSlingMode = SLING_MODE_IDLE;
				m_pResourceData->m_pSlingMeshEntity->UserAnim_Select( _SLING_ANIM_TAKE_UP_SLACK );
				m_pResourceData->m_pSlingMeshEntity->UserAnim_UpdateTime( 0.0f );
				m_pResourceData->m_pSlingMeshEntity->UserAnim_Pause( TRUE );
			}
		}

		break;
	}

	m_aResourceData[0].m_pForkAnimCombiner->ComputeMtxPalette( FALSE );

	RelocateAllChildren();

	m_pResourceData->m_pSlingMeshEntity->Work();
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponMortar::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_fSecondsCountdownTimer = 0.0f;
	m_fUnitStretch = 0.0f;

	Mortar_Fork_Idle();
	Mortar_Sling_Idle();
}


void CWeaponMortar::NotifyAmmoMightHaveChanged( void ) {
}



