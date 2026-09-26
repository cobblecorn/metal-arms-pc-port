//////////////////////////////////////////////////////////////////////////////////////
// weapon_tether.cpp - Tether weapon.
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
// 06/05/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_tether.h"
#include "fgamedata.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fmesh.h"
#include "fanim.h"
#include "meshtypes.h"
#include "floop.h"
#include "fresload.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "explosion.h"
#include "iteminst.h"
#include "meshentity.h"
#include "fcamera.h"
#include "tether.h"
#include "AI/AIEnviro.h"
#include "fsound.h"



#define _USER_PROP_FILENAME		"w_tether.csv"
#define _RELOCATE_IDENTIFIER	(&m_fSecondsCountdownTimer)

#define _TARGET_HOLD_TIME		1.0f
#define _TARGET_LOCK_TIME		3.0f

#define _AUTO_PUMP_TIME			1.25f
#define _AUTO_PUMP_OO_TIME		( 1.0f / _AUTO_PUMP_TIME )

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponTetherBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponTetherBuilder _WeaponTetherBuilder;


void CWeaponTetherBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONTETHER, pszEntityType );
}


BOOL CWeaponTetherBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponTetherBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponTether
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponTether::_UserProps_t CWeaponTether::m_aUserProps[EUK_COUNT_TETHER];
CWeaponTether *CWeaponTether::m_pCallbackTether;


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponTether::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_WEAPON]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszMeshName[_MESH_TETHER]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszBoneName[_BONE_BARREL]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszBoneName[_BONE_PRIMARY_FIRE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszPumpAnimName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fReserveAmmoMax:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_65534,

	// fWeaponCullDist:
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

	// fBoltCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fBoltSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt1,
	F32_DATATABLE_100000,

	// fMuzzleRecoilDistance:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	// fBoltRestOffsetZ:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fBoltScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMinTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt1,
	F32_DATATABLE_100000,

	// fMaxTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fTetherLength:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	// fStretchLength:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	// fMaxFlyDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	// fPossessSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_1000,

	// fLockOnSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_1000,

	// nPumpCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEmpty
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupPump
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAttachBolt

	TETHER_GAMEDATA_VOCAB,			// CTether's user props


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponTether::m_aUserPropMapTable[] = {
	"TetherL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"TetherL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"TetherL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"TetherKrunk",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],
	
	NULL
};

BOOL CWeaponTether::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponTether::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		return FALSE;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_TETHER; i++ ) {
		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_TETHER][i];

		if( i == KRUNKS_EUK_INDEX ) {
			// krunk only uses his right arm to shoot the tether
			pInfo->nGrip = GRIP_RIGHT_ARM;
			pInfo->nStanceType = STANCE_TYPE_STANDARD;
		} else {
			pInfo->nGrip = GRIP_BOTH_ARMS;
			pInfo->nStanceType = STANCE_TYPE_TWO_HANDED_TORSO_TWIST;
		}
		pInfo->nReloadType = RELOAD_TYPE_TETHER;		
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->nClipAmmoMax = 1;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_LEFT_HAND_RELOADS | INFOFLAG_THICK_TARGETING;
		pInfo->nReticleType = CReticle::TYPE_TETHER;
	}

	// Success...

	return TRUE;
}


void CWeaponTether::UninitSystem( void ) {
}


CWeaponTether::CWeaponTether() {
	_ClearDataMembers();
}


CWeaponTether::~CWeaponTether() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponTether::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponTetherBuilder *pBuilder = (CWeaponTetherBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponTetherBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONTETHER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_TETHER, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponTether::ClassHierarchyBuild( void ) {
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
	CWeaponTetherBuilder *pBuilder = (CWeaponTetherBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_TETHER; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_TETHER );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		pInfo = &m_aaInfo[WEAPON_TYPE_TETHER][i];

		//// Create the world mesh...
		//pResourceData->m_pWorldMesh = fnew CFWorldMesh;
		//if( pResourceData->m_pWorldMesh == NULL ) {
		//	DEVPRINTF( "CWeaponTether::Create(): Not enough memory to create m_pWorldMesh.\n" );
		//	goto _ExitWithError;
		//}

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponTether::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );

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
		if( !CreateSharedEUKData( &MeshInit, &uResEUK, &(pResourceData->m_pWorldMesh), &(pResourceData->m_pAnimCombiner) ) ) {
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

			// Create our pump anim...
			pResourceData->m_pPumpAnimInst = CFAnimInst::Load( m_aUserProps[uMeshEUK].pszPumpAnimName );
			if( pResourceData->m_pPumpAnimInst == NULL ) {
				DEVPRINTF( "CWeaponTether::Create(): Could not create CFAnimInst.\n" );
				goto _ExitWithError;
			}

			if( !pResourceData->m_pAnimCombiner->CreateSimple( pResourceData->m_pPumpAnimInst, pResourceData->m_pWorldMesh ) ) {
				DEVPRINTF( "CWeaponTether::Create(): Could not create animation combiner.\n" );
				goto _ExitWithError;
			}

			pResourceData->m_pAnimCombiner->SetBoneCallback( _CartridgeBoneCallback );
			pResourceData->m_pAnimCombiner->DisableAllBoneCallbacks();
			pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[i].apszBoneName[_BONE_BARREL] );
		} else {
			pResourceData->m_pPumpAnimInst = m_aResourceData[uResEUK].m_pPumpAnimInst;
		}

		pResourceData->m_nPrimaryFireBoneIndex = pResourceData->m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE] );
		if( pResourceData->m_nPrimaryFireBoneIndex == -1 ) {
			DEVPRINTF( "CWeaponTether::Create(): Could not find bone '%s' in mesh '%s'.\n", m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE], pResourceData->m_pWorldMesh->m_pMesh->szName );
			goto _ExitWithError;
		}
	}

	// Build our bolt mesh entity...
	m_pBoltMeshEntity = fnew CMeshEntity;
	if( m_pBoltMeshEntity == NULL ) {
		DEVPRINTF( "CWeaponTether::Create(): Not enough memory to create the bolt CMeshEntity.\n" );
		goto _ExitWithError;
	}

	if( !m_pBoltMeshEntity->Create( m_aUserProps[0].apszMeshName[_MESH_BOLT] ) ) {
		DEVPRINTF( "CWeaponTether::Create(): Could not create the bolt CMeshEntity.\n" );
		goto _ExitWithError;
	}

	m_pBoltMeshEntity->SetCollisionFlag( FALSE );
	m_pBoltMeshEntity->SetLineOfSightFlag( FALSE );
	m_pBoltMeshEntity->SetCullDist( m_aUserProps[0].fBoltCullDist );
	m_pBoltMeshEntity->MeshFlip_SetFlipsPerSec( 0.0f );
	m_pBoltMeshEntity->RemoveFromWorld();

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	fforce_NullHandle( &m_hForce );

	m_pTether = fnew CTether;
	if( m_pTether == NULL ) {
		DEVPRINTF( "CWeaponTether::Create(): Not enough memory to create CTether.\n" );
		goto _ExitWithError;
	}

	if( !m_pTether->Create( 40 ) ) {
		DEVPRINTF( "CWeaponTether::Create(): Could not create CTether.\n" );
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponTether::ClassHierarchyBuilt( void ) {
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


void CWeaponTether::ClassHierarchyDestroy( void ) {
	u32 i;

	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	for( i=0; i<EUK_COUNT_TETHER; ++i ) {
		if( DestroySharedEUKData( i ) ) {
			fdelete( m_aResourceData[i].m_pPumpAnimInst );
		}
		//fdelete( m_aResourceData[i].m_pAnimCombiner );
		//fdelete( m_aResourceData[i].m_pPumpAnimInst );
		//fdelete( m_aResourceData[i].m_pWorldMesh );
	}

	fdelete( m_pBoltMeshEntity );
	fdelete( m_pTether );

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponTether::GetLeafClassBuilder( void ) {
	return &_WeaponTetherBuilder;
}


void CWeaponTether::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_TETHER; ++i ) {
		m_aResourceData[i].m_pWorldMesh = NULL;
		m_aResourceData[i].m_pAnimCombiner = NULL;
		m_aResourceData[i].m_pPumpAnimInst = NULL;
	}

	m_bFireThisFrame = FALSE;

	m_pResourceData = NULL;
	m_pBoltMeshEntity = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_nPumpsNeededCount = 0;
	m_fPumpAnimTimeScale = 0.0f;
	m_fUnitMuzzleRecoilDist = 0.0f;

	m_pTether = NULL;
}


void CWeaponTether::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponTether::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NONE, CItemInst::AMMODISPLAY_NONE );
	}
}


void CWeaponTether::DestroyTether( BOOL bSupressEffects/*=FALSE*/ ) {
	FASSERT( IsCreated() );
	FASSERT( m_pTether );

	m_pTether->Kill( bSupressEffects );
}


void CWeaponTether::_RemoveBoltFromWorld( void ) {
	m_pBoltMeshEntity->DetachFromParent();
	m_pBoltMeshEntity->RemoveFromWorld();
}


void CWeaponTether::_AddBoltToWorld( void ) {
	CFMtx43A BoltPosMtx;

	BoltPosMtx.Identity();
	BoltPosMtx.m_vPos.z = m_pUserProps->fBoltRestOffsetZ;

	m_pBoltMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_pUserProps->apszBoneName[_BONE_PRIMARY_FIRE], &BoltPosMtx, 1.0f, TRUE );
	m_pBoltMeshEntity->AddToWorld();
}


void CWeaponTether::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pWorldMesh->UpdateTracker();

	// Add bolt to the world...
	if( GetClipAmmo() ) {
		_AddBoltToWorld();
	}

	_SetReticleLockOnSpeed();
}


void CWeaponTether::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();

	// Remove bolt from the world...
	if( GetClipAmmo() ) {
		_RemoveBoltFromWorld();
	}

	// Sever tether...
	m_pTether->Sever();
}


void CWeaponTether::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );
	
	u32 i;
	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this weapon...

		for( i=0; i<EUK_COUNT_TETHER; ++i ) {
			FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
	} else {
		// Disable drawing of this weapon...

		for( i=0; i<EUK_COUNT_TETHER; ++i ) {
			FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
	}
	if (m_pBoltMeshEntity)
		m_pBoltMeshEntity->DrawEnable(bDrawingHasBeenEnabled);
}


void CWeaponTether::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( pIdentifier != _RELOCATE_IDENTIFIER ) {
		// Ah, we were not responsible for this relocation...

		if( IsInWorld() ) {
			m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
			m_pResourceData->m_pWorldMesh->UpdateTracker();
		}
	}
}


CFMtx43A *CWeaponTether::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponTether::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponTether::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;

	// Add our bolt to the list...
	m_pBoltMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}


void CWeaponTether::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	*pMuzzlePoint_WS = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_pResourceData->m_nPrimaryFireBoneIndex ]->m_vPos;
}


void CWeaponTether::Trigger_AttachRoundToOwnerBotBone( cchar *pszBoneName ) {
	m_pBoltMeshEntity->Attach_UnitMtxToParent_PS_NewScale_WS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, 1.0f, FALSE );
	m_pBoltMeshEntity->AddToWorld();
}


void CWeaponTether::Trigger_AttachRoundToWeapon( void ) {
	if( IsInWorld() ) {
		CFMtx43A BoltPosMtx;

		BoltPosMtx.Identity();
		BoltPosMtx.m_vPos.z = m_pUserProps->fBoltRestOffsetZ;

		m_pBoltMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_pUserProps->apszBoneName[_BONE_PRIMARY_FIRE], &BoltPosMtx, 1.0f, TRUE );
		m_pBoltMeshEntity->AddToWorld();

		PlaySound( m_pUserProps->pSoundGroupAttachBolt );
	}

	m_fUnitMuzzleRecoilDist = 0.0f;
}


void CWeaponTether::Trigger_RemoveRoundFromWorld( void ) {
	m_pBoltMeshEntity->RemoveFromWorld();
}


void CWeaponTether::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_pResourceData->m_pPumpAnimInst->UpdateTime( 0.0f );
	m_fPumpAnimTimeScale = 0.0f;
	m_bFireThisFrame = FALSE;

	// Reset the reticle targeting system...
	CReticle *pReticle = GetSafeReticle();
	if( pReticle ) {
		pReticle->TargetSys_Reset();
	}

	_SetReticleLockOnSpeed();
}


// Note that ClassHierarchyResetToState() is always called after this call.
void CWeaponTether::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_SPEW][nPreviousUpgradeLevel];

	BOOL bClipAttached = GetClipAmmo();

	if( bClipAttached ) {
		_RemoveBoltFromWorld();
	}

	m_fUnitMuzzleRecoilDist = 0.0f;
	m_fSecondsCountdownTimer = 0.0f;
	m_nPumpsNeededCount = 0;

	// Remove old mesh from world...
	pOldResourceData->m_pWorldMesh->RemoveFromWorld();

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		if( bClipAttached ) {
			_AddBoltToWorld();
		}
	}

	_SetReticleLockOnSpeed();
}


void CWeaponTether::_SetReticleLockOnSpeed( void ) {
	CReticle *pReticle = GetSafeReticle();
	if( pReticle ) {
		pReticle->TargetSys_SetLockOnSpeed( m_pUserProps->fLockOnSpeed );
	}
}


// Returns TRUE if no more pumps are needed.
BOOL CWeaponTether::Pump( f32 fTimeScale ) {
	FASSERT( IsCreated() );

	if( m_nPumpsNeededCount ) {
		--m_nPumpsNeededCount;

		PlaySound( m_pUserProps->pSoundGroupPump );

		m_pResourceData->m_pPumpAnimInst->UpdateTime( 0.0f );
		m_fPumpAnimTimeScale = fTimeScale;

		return FALSE;
	}

	return TRUE;
}


void CWeaponTether::_LaunchTetherCable( CBot *pTargetBot, const CFVec3A *pTargetPos_WS ) {
	m_pTether->Launch(
		this,
		m_pUserProps->apszBoneName[_BONE_PRIMARY_FIRE],
		pTargetBot,
		pTargetPos_WS,
		m_pUserProps->fTetherLength,
		m_pUserProps->fStretchLength,
		m_pUserProps->fMaxFlyDist,
		m_pUserProps->fPossessSpeed,
		&m_pUserProps->TetherUserProps
	);
}


u32 CWeaponTether::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS ) {
	FASSERT( IsCreated() );

	m_bFireThisFrame = FALSE;

	if( fUnitTriggerVal1 < 0.25f ) {
		// Trigger not down...

		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

		return 0;
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

		return 0;
	}

	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		return 0;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return 0;
	}

	if( GetSafeOwnerPlayerIndex() < 0 ) {
		// We don't support tethers used by non-player bots...
		return 0;
	}

	m_bFireThisFrame = TRUE;

	return 1;
}


void CWeaponTether::_FireTether( void ) {
	CReticle *pReticle;
	CFCamera *pCamera;
	const FViewport_t *pViewport;
	BOOL bNonLockedFire = TRUE;
	const CFXfm *pCamXfm;

	s32 nPlayerIndex = GetSafeOwnerPlayerIndex();
	FASSERT( nPlayerIndex >= 0 );

	m_fUnitMuzzleRecoilDist = 1.0f;
	m_fSecondsCountdownTimer = _AUTO_PUMP_TIME;

	m_nPumpsNeededCount = m_pUserProps->nPumpCount;

	Trigger_RemoveRoundFromWorld();
	RemoveFromClip( 1 );

	// Get the reticle, camera and viewport...
	pReticle = &Player_aPlayer[ nPlayerIndex ].m_Reticle;
	pCamera = fcamera_GetCameraByIndex( nPlayerIndex );
	pViewport = pCamera->GetViewport();
	pCamXfm = pCamera->GetFinalXfm();

	// Play force and sound...
	fforce_Kill( &m_hForce );
	fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE, &m_hForce );

	// Play firing sound...
	PlaySound( m_pUserProps->pSoundGroupFire );

	if( m_pTargetedEntity ) {
		CBot *pTargetedBot;
		f32 fUnitInaccuracy;

		// Get bot pointer...
		FASSERT( m_pTargetedEntity->TypeBits() & ENTITY_BIT_BOT );
		pTargetedBot = (CBot *)m_pTargetedEntity;

		// Compute unit inaccuracy...
		fUnitInaccuracy = pReticle->TargetSys_ComputeUnitInaccuracy();

		if( fUnitInaccuracy == 0.0f ) {
			// Dead on...

			bNonLockedFire = FALSE;
			_LaunchTetherCable( pTargetedBot, pTargetedBot->DataPort_GetPos() );

		} else if( fUnitInaccuracy >= 0.0f ) {
			// Introduce inaccuracy...

			f32 fRandomUnitSpread, fRandomAngle, fRandomRadius, fSin, fCos, fPerpDistFromCameraToDataPort;
			CFVec3A TargetPoint_WS, CamToDataPort_WS;

			CamToDataPort_WS.Sub( *pTargetedBot->DataPort_GetPos(), pCamXfm->m_MtxR.m_vPos );
			fPerpDistFromCameraToDataPort = CamToDataPort_WS.Dot( pCamXfm->m_MtxR.m_vFront );

			fRandomUnitSpread = fmath_RandomFloat();

			if( fUnitInaccuracy <= 0.2f ) {
				// Most accurate...
			} else if( fUnitInaccuracy <= 0.4f ) {
				// Accurate...
				fRandomUnitSpread *= fRandomUnitSpread;
			} else if( fUnitInaccuracy <= 0.6f ) {
				// Middle of the road...
				fRandomUnitSpread *= fRandomUnitSpread * fRandomUnitSpread;
			} else if( fUnitInaccuracy <= 0.8f ) {
				// Inaccurate...
				fRandomUnitSpread *= fRandomUnitSpread * fRandomUnitSpread * fRandomUnitSpread;
			} else {
				// Most inaccurate...
				fRandomUnitSpread *= fRandomUnitSpread * fRandomUnitSpread * fRandomUnitSpread * fRandomUnitSpread;
			}

			fRandomUnitSpread = 1.0f - fRandomUnitSpread;

			fRandomUnitSpread *= FMATH_FPOT( fUnitInaccuracy, 0.25f, 1.0f );

			fRandomAngle = fmath_RandomFloat() * FMATH_2PI;
			fRandomRadius = fRandomUnitSpread * fPerpDistFromCameraToDataPort * pViewport->fTanHalfFOVY * pReticle->GetScreenSpaceRadius();

			fmath_SinCos( fRandomAngle, &fSin, &fCos );
			TargetPoint_WS.Set( fSin*fRandomRadius, fCos*fRandomRadius, 0.0f );
			pCamXfm->m_MtxR.MulDir( TargetPoint_WS );
			TargetPoint_WS.Add( *pTargetedBot->DataPort_GetPos() );

			_LaunchTetherCable( pTargetedBot, &TargetPoint_WS );

			bNonLockedFire = FALSE;
		}
	}

	if( bNonLockedFire ) {
		FCollImpact_t CollImpact;
		CFVec3A RayStartPoint, RayEndPoint, TargetPoint_WS, ReticleAdjustX, ReticleAdjustY, ScatterAimUnitVec_WS;
		f32 fReticleX, fReticleY, fMaxLiveRange, fEndPointReticleAdjust;

		fReticleX = pReticle->GetNormOriginX();
		fReticleY = pReticle->GetNormOriginY();

		// Ray start point is the camera position...
		RayStartPoint.Set( pCamXfm->m_MtxR.m_vPos );

		// Compute the ray end point which lies in the center of the reticle...
		fMaxLiveRange = m_pInfo->fMaxLiveRange;

		fEndPointReticleAdjust = (fMaxLiveRange * fReticleX) * pViewport->fTanHalfFOVX;
		ReticleAdjustX.Mul( pCamXfm->m_MtxR.m_vRight, fEndPointReticleAdjust );

		fEndPointReticleAdjust = (fMaxLiveRange * fReticleY) * pViewport->fTanHalfFOVY;
		ReticleAdjustY.Mul( pCamXfm->m_MtxR.m_vUp, fEndPointReticleAdjust );

		fmath_ScatterUnitVec( &ScatterAimUnitVec_WS, &pCamXfm->m_MtxR.m_vFront, 0.1f );

		RayEndPoint.Mul( ScatterAimUnitVec_WS, fMaxLiveRange ).Add( RayStartPoint ).Add( ReticleAdjustX ).Add( ReticleAdjustY );

		BOOL bFoundImpact = fworld_FindClosestImpactPointToRayStart(
								&CollImpact,
								&RayStartPoint,
								&RayEndPoint,
								0,
								NULL,
								TRUE,
								NULL,
								-1,
								FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES
							);

		if( !bFoundImpact ) {
			TargetPoint_WS = RayEndPoint;
		} else {
			TargetPoint_WS.Set( CollImpact.ImpactPoint );
		}

		_LaunchTetherCable( NULL, &TargetPoint_WS );
	}

	pReticle->TargetSys_UpdateTargetedIntensity( 0.0f );
	m_pTargetedEntity = NULL;
}


// Called once per frame to update animations, timers, etc.
void CWeaponTether::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( m_bFireThisFrame ) {
		FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );
		_FireTether();
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fSecondsCountdownTimer, 0.0f );
	}

	if( m_fPumpAnimTimeScale > 0.0f ) {
		if( !m_pResourceData->m_pPumpAnimInst->DeltaTime( FLoop_fPreviousLoopSecs * m_fPumpAnimTimeScale, TRUE ) ) {
			f32 fUnitPumpTime = m_pResourceData->m_pPumpAnimInst->GetUnitTime();
			f32 fPumpSin = fmath_Sin( FMATH_PI * fUnitPumpTime );

			m_fUnitMuzzleRecoilDist = fmath_Div( (f32)m_nPumpsNeededCount + 1.0f - fUnitPumpTime, (f32)m_pUserProps->nPumpCount );
			m_fUnitMuzzleRecoilDist -= 0.2f * fPumpSin * fPumpSin * fPumpSin * fPumpSin;
		} else {
			m_fUnitMuzzleRecoilDist = fmath_Div( (f32)m_nPumpsNeededCount, (f32)m_pUserProps->nPumpCount );
			m_pResourceData->m_pPumpAnimInst->UpdateTime( 0.0f );
			m_fPumpAnimTimeScale = 0.0f;
		}
	} else if( (m_fUnitMuzzleRecoilDist != 0.0f) && !m_nPumpsNeededCount ) {
		// The current weapon is not being pumped. We still need to get the
		// barrel back into shooting position. Simulate an "auto pump action" weapon...

		m_fUnitMuzzleRecoilDist = fmath_UnitLinearToSCurve( m_fSecondsCountdownTimer * _AUTO_PUMP_OO_TIME );
		m_fUnitMuzzleRecoilDist *= m_fUnitMuzzleRecoilDist;

		if( m_fUnitMuzzleRecoilDist < 0.15f ) {
			// Snap back into place...

			m_fUnitMuzzleRecoilDist = 0.0f;
			m_fSecondsCountdownTimer = 0.0f;
		}
	}

	m_pCallbackTether = this;
	m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );
	RelocateAllChildren();
	m_pCallbackTether = NULL;

	s32 nPlayerIndex = GetSafeOwnerPlayerIndex();

	// See if the entity we're targeting is still targetable...
	if( m_pTargetedEntity && (nPlayerIndex >= 0) ) {
		CReticle *pReticle;
		DataPortTest_e nDataPortTest;

		pReticle = &Player_aPlayer[ nPlayerIndex ].m_Reticle;

		FASSERT( m_pTargetedEntity->TypeBits() & ENTITY_BIT_BOT );

		const FViewport_t *pViewport;
		CBot *pBot = (CBot *)m_pTargetedEntity;

		if( pBot->DataPort_IsOpen() ) {
			// Data port is still open...

			pViewport = fcamera_GetCameraByIndex( nPlayerIndex )->GetViewport();
			nDataPortTest = _TestDataPortTargetable( pBot, pViewport, pReticle );
		} else {
			// Data port has closed...

			nDataPortTest = DATAPORTTEST_CANCEL;
		}

		if( nDataPortTest == DATAPORTTEST_CANCEL ) {
			// Cancel targeting this entity...

			m_pTargetedEntity = NULL;
			m_fTargetedEntityHoldTimer = 0.0f;

			pReticle->TargetSys_UpdateTargetedIntensity( 0.0f );
		} else {
			pReticle->TargetSys_UpdateTargetedPoint( pViewport, pBot->DataPort_GetPos() );

			if( nDataPortTest == DATAPORTTEST_HOLDABLE ) {
				// Continue to target this entity even though it's momentarily untargetable...

				if( m_fTargetedEntityHoldTimer == _TARGET_HOLD_TIME ) {
					// Data port was previously targetable, but now is holdable...

					m_fTargetedEntityHoldTimer = (_TARGET_HOLD_TIME * 0.999f);
				} else {
					// Data port was previously holdable and still is...

					if( m_fTargetedEntityHoldTimer > 0.0f ) {
						m_fTargetedEntityHoldTimer -= FLoop_fPreviousLoopSecs;
						if( m_fTargetedEntityHoldTimer <= 0.0f ) {
							// Don't hold any longer...
							m_pTargetedEntity = NULL;
							m_fTargetedEntityHoldTimer = 0.0f;
							pReticle->TargetSys_UpdateTargetedIntensity( 0.0f );
						}
					}
				}

				pReticle->TargetSys_UpdateTargetedIntensity( m_fTargetedEntityHoldTimer * (1.0f / _TARGET_HOLD_TIME) );
			} else {
				// Entity is targetable...

				m_fTargetedEntityHoldTimer = _TARGET_HOLD_TIME;
				pReticle->TargetSys_UpdateTargetedIntensity( 1.0f );
			}
		}
	}

	Reticle_EnableDraw( GetClipAmmo() );
}


void CWeaponTether::BeginReload( void ) {
	// Sever tether...
	m_pTether->Sever();

	Trigger_AttachRoundToWeapon();
	TransferFromReserveToClip();
	ReloadComplete();
}


void CWeaponTether::_CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( m_pCallbackTether->m_fUnitMuzzleRecoilDist != 0.0f ) {
		CFMtx43A::m_Xlat.m_vPos.x = 0.0f;
		CFMtx43A::m_Xlat.m_vPos.y = 0.0f;
		CFMtx43A::m_Xlat.m_vPos.z = m_aUserProps[m_pCallbackTether->m_nUpgradeLevel].fMuzzleRecoilDistance * m_pCallbackTether->m_fUnitMuzzleRecoilDist;
		rNewMtx.Mul( CFMtx43A::m_Xlat );
	}
}


void CWeaponTether::NotifyAmmoMightHaveChanged( void ) {
}


void CWeaponTether::ReticleTypeSet( void ) {
	_SetReticleLockOnSpeed();
}


void CWeaponTether::HumanTargeting_FindDesiredTargetPoint_WS( CFVec3A *pTargetPoint_WS ) {
	s32 nPlayerIndex;
	CFCamera *pCamera;

	nPlayerIndex = GetSafeOwnerPlayerIndex();
	FASSERT( nPlayerIndex >= 0 );
	m_pCallback_Reticle = &Player_aPlayer[ nPlayerIndex ].m_Reticle;
	pCamera = fcamera_GetCameraByIndex( nPlayerIndex );
	m_pCallback_Viewport = pCamera->GetViewport();

	if( (CurrentState() == STATE_DEPLOYED) && !IsClipEmpty() ) {
		if( m_pTargetedEntity ) {
			// We have a currently-targeted entity...

			if( m_pCallback_Reticle->TargetSys_GetUnitLock() > 0.0f ) {
				// We're either locked or in the process of locking...

				FASSERT( m_pTargetedEntity->TypeBits() & ENTITY_BIT_BOT );

				CBot *pBot = (CBot *)m_pTargetedEntity;
				*pTargetPoint_WS = *pBot->DataPort_GetPos();
			} else {
				// We're not locked...
				_FindUnlockedTargetPoint( pCamera, pTargetPoint_WS );
			}

			return;
		}

		// Find a new target...

		CFSphere Sphere;
		CFWorldUser UserTracker;

		// Build tracker skip list...
		FWorld_nTrackerSkipListCount = 0;
		if( m_pOwnerBot ) {
			m_pOwnerBot->AppendTrackerSkipList();
		}

		// Build user tracker...
		Sphere.m_Pos = m_MtxToWorld.m_vPos.v3;
		Sphere.m_fRadius = m_pInfo->fMaxTargetAssistDist;
		UserTracker.MoveTracker( Sphere );

		// Find all trackers within range...
		m_pCallback_Weapon = this;
		m_pCallback_ClosestEntity = NULL;
		UserTracker.FindIntersectingTrackers( _FindTrackersInRangeCallback, FWORLD_TRACKERTYPE_MESH );

		if( m_pCallback_ClosestEntity ) {
			// Found a targetable data port...
			m_pTargetedEntity = m_pCallback_ClosestEntity;
		}

		// Cleanup...
		UserTracker.RemoveFromWorld();
	} else {
		m_pTargetedEntity = NULL;
	}

	if( m_pTargetedEntity == NULL ) {
		_FindUnlockedTargetPoint( pCamera, pTargetPoint_WS );
	}
}


void CWeaponTether::_FindUnlockedTargetPoint( CFCamera *pCamera, CFVec3A *pTargetPoint_WS ) {
	f32 fMaxLiveRange, fEndPointReticleAdjust;
	CFVec3A RayStartPoint, ReticleAdjustX, ReticleAdjustY;
	const CFXfm *pCamXfm;
	FCollImpact_t CollImpact;

	pCamXfm = pCamera->GetFinalXfm();

	// Ray start point is the camera position...
	RayStartPoint.Set( pCamXfm->m_MtxR.m_vPos );

	// Compute the ray end point which lies in the center of the reticle...
	fMaxLiveRange = m_pInfo->fMaxLiveRange;

	fEndPointReticleAdjust = (fMaxLiveRange * m_pCallback_Reticle->GetNormOriginX()) * m_pCallback_Viewport->fTanHalfFOVX;
	ReticleAdjustX.Mul( pCamXfm->m_MtxR.m_vRight, fEndPointReticleAdjust );

	fEndPointReticleAdjust = (fMaxLiveRange * m_pCallback_Reticle->GetNormOriginY()) * m_pCallback_Viewport->fTanHalfFOVY;
	ReticleAdjustY.Mul( pCamXfm->m_MtxR.m_vUp, fEndPointReticleAdjust );

	pTargetPoint_WS->Mul( pCamXfm->m_MtxR.m_vFront, fMaxLiveRange ).Add( RayStartPoint ).Add( ReticleAdjustX ).Add( ReticleAdjustY );

	// Find the closest impact to the camera...
	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &RayStartPoint, pTargetPoint_WS, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES ) ) {
		// Our ray hit something...

		pTargetPoint_WS->Set( CollImpact.ImpactPoint );
	}
}


// Note: Does NOT check for whether the data port is opened.
CWeaponTether::DataPortTest_e CWeaponTether::_TestDataPortTargetable( CBot *pBot, const FViewport_t *pViewport, CReticle *pReticle, f32 *pfDistFromMuzzleToDataPort ) {
	CFVec3A MuzzlePoint_WS, PortToMuzzle, *pMuzzleDir_WS, DataPortUnitNormal_WS;
	const CFVec3A *pDataPortPos_WS;
	f32 fDist, fDot;

	pDataPortPos_WS = pBot->DataPort_GetPos();
	pBot->DataPort_ComputeUnitNormal( &DataPortUnitNormal_WS );

	ComputeMuzzlePoint_WS( &MuzzlePoint_WS );
	PortToMuzzle.Sub( MuzzlePoint_WS, *pDataPortPos_WS );

	fDot = PortToMuzzle.Dot( DataPortUnitNormal_WS );
	if( fDot <= 0.0f ) {
		// Data port is behind muzzle...
		return DATAPORTTEST_HOLDABLE;
	}

	fDist = PortToMuzzle.Mag();

	if( fDist > m_pInfo->fMaxTargetAssistDist ) {
		// Data port is too far away...
		return DATAPORTTEST_HOLDABLE;
	}

	if( fDist < m_pInfo->fMinTargetAssistDist ) {
		// Data port is too close...
		return DATAPORTTEST_HOLDABLE;
	}

	fDot = fmath_Div( fDot, fDist );

	if( fDot < 0.3f ) {
		// Data port is at too much of an angle...
		return DATAPORTTEST_HOLDABLE;
	}

	if( !pReticle->IsPointWithinReticle( pViewport, pDataPortPos_WS ) ) {
		// Data port is not within reticle...
		return DATAPORTTEST_HOLDABLE;
	}

	pMuzzleDir_WS = &m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_pResourceData->m_nPrimaryFireBoneIndex ]->m_vFront;
	if( pMuzzleDir_WS->Dot( DataPortUnitNormal_WS ) >= 0.0f ) {
		return DATAPORTTEST_HOLDABLE;
	}

	// Data port is within reticle...

	FWorld_nTrackerSkipListCount = 0;
	pBot->AppendTrackerSkipList();
	m_pOwnerBot->AppendTrackerSkipList();

	if( fworld_IsLineOfSightObstructed( &FXfm_pView->m_MtxR.m_vPos, pDataPortPos_WS, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList ) ) {
		// Data port is obstructed...

		return DATAPORTTEST_HOLDABLE;
	}

	// Data port is targetable...

	if( pfDistFromMuzzleToDataPort ) {
		*pfDistFromMuzzleToDataPort = fDist;
	}

	return DATAPORTTEST_TARGETABLE;
}


BOOL CWeaponTether::_FindTrackersInRangeCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	if( !pTracker->IsCollisionFlagSet() ) {
		// No collision possible with this tracker...
		return TRUE;
	}

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not an entity...
		return TRUE;
	}

	// Tracker is a world mesh...
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if( pWorldMesh->m_nFlags & FMESHINST_FLAG_NOCOLLIDE ) {
		// Mesh is not drawn, so skip...
		return TRUE;
	}

	// Tracker is a CEntity...
	CEntity *pEntity = (CEntity *)pWorldMesh->m_pUser;

	if( !(pEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		// Not a bot...
		return TRUE;
	}

	// Entity is a bot...
	CBot *pBot = (CBot *)pEntity;

	if( !pBot->DataPort_IsOpen() ) {
		// Data port is not open (or is not installed)...
		return TRUE;
	}

	u32 i;

	// Skip if in our skip list...
	for( i=0; i<FWorld_nTrackerSkipListCount; ++i ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	// This is a bot with an open data port...

	f32 fDistFromMuzzleToDataPort;
	DataPortTest_e nDataPortTest = ((CWeaponTether *)m_pCallback_Weapon)->_TestDataPortTargetable( pBot, m_pCallback_Viewport, m_pCallback_Reticle, &fDistFromMuzzleToDataPort );
	if( nDataPortTest != DATAPORTTEST_TARGETABLE ) {
		// Data port is not targetable...
		return TRUE;
	}

	// Found a targetable data port...

	if( (m_pCallback_ClosestEntity == NULL) || (fDistFromMuzzleToDataPort < m_fCallback_ShortestDistance2) ) {
		// Found a closer targetable data port...

		m_pCallback_ClosestEntity = pEntity;
		m_fCallback_ShortestDistance2 = fDistFromMuzzleToDataPort;
	}

	return TRUE;
}


