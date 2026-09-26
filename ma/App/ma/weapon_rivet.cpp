//////////////////////////////////////////////////////////////////////////////////////
// weapon_rivet.cpp - Mining rivet implementation.
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
// 03/28/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_rivet.h"
#include "weapon.h"
#include "fworld.h"
#include "fresload.h"
#include "ftex.h"
#include "fviewport.h"
#include "tracer.h"
#include "gamecam.h"
#include "floop.h"
#include "fforce.h"
#include "reticle.h"
#include "fcolor.h"
#include "player.h"
#include "reticle.h"
#include "floop.h"
#include "FCheckPoint.h"
#include "explosion.h"
#include "meshtypes.h"
#include "potmark.h"
#include "iteminst.h"
#include "Ai/AIEnviro.h"
#include "weapon_scope.h"
#include "damage.h"
#include "bot.h"
#include "meshentity.h"
#include "fsound.h"
#include "eproj_arrow.h"
#include "eparticlepool.h"


#define _USER_PROP_FILENAME		"w_rivet.csv"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRivetBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponRivetBuilder _WeaponRivetBuilder;


void CWeaponRivetBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONRIVET, pszEntityType );
}


BOOL CWeaponRivetBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponRivetBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRivet
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CWeaponRivet::m_bSystemInitialized;
CWeaponRivet::_UserProps_t CWeaponRivet::m_aUserProps[EUK_COUNT_RIVET];
CEProjPool::PoolHandle_t CWeaponRivet::m_hArrowProjPool;
SmokeTrailAttrib_t CWeaponRivet::m_SmokeTrailAttrib;
TracerGroupHandle_t CWeaponRivet::m_hTracerGroup;
CFTexInst CWeaponRivet::m_TracerTexInst;
TracerDef_t CWeaponRivet::m_TracerDef;
CWeaponRivet *CWeaponRivet::m_pCallbackThis;
CDamageForm *CWeaponRivet::m_apDamageFormArray[DAMAGE_FORM_COUNT];
u32 CWeaponRivet::m_nDamageFormCount;
CEntity *CWeaponRivet::m_pHitEntity;
CFCollInfo CWeaponRivet::m_CollInfoArrowRayTest;
CFTrackerCollideRayInfo CWeaponRivet::m_TrackerCollideRayInfo;
const CFVec3A *CWeaponRivet::m_pAttackUnitDir_WS;


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponRivet::m_aUserPropVocab[] = {
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

	// apszMeshName[_MESH_ARROW]:
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

	// pszTracerTexName:
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
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
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

	// fShotSpreadFactor:
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

	// nProjInPoolCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_1000,

	// fMaxLiveRange:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fUnitRecoil
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fProjCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fProjSpeed:
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

	FGAMEDATA_VOCAB_PARTICLE,		// hParticleImpact
	FGAMEDATA_VOCAB_DAMAGE,			// pDamageProfileImpact

	FGAMEDATA_VOCAB_MESH,			// pMeshEjectClip

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEmpty
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEjectClip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAttachClip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupSlapInClip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupRicochet

	EPROJ_ARROW_STATICPARAMS_GAMEDATA_VOCAB,	// ArrowProjStaticParams


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponRivet::m_aUserPropMapTable[] = {
	"RivetL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"RivetL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"RivetL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"RivetMozer",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],
	
	"RivetMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[4],
	
	"RivetMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[5],
	
	NULL
};



CWeaponRivet::CWeaponRivet( void ) {
	_ClearDataMembers();
}


CWeaponRivet::~CWeaponRivet( void ) {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponRivet::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	FTexDef_t *pTexDef;
	Info_t *pInfo;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	m_hTracerGroup = TRACER_NULLGROUPHANDLE;

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponRivet::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		return FALSE;
	}

	// Create a pool of projectiles for the arrows...
	m_hArrowProjPool = CEProjPool::Create( CEProj::PROJTYPE_ARROW, m_aUserProps[0].apszMeshName[_MESH_ARROW], m_aUserProps[0].nProjInPoolCount );

	if( m_hArrowProjPool == EPROJPOOL_NULL_HANDLE ) {
		DEVPRINTF( "CWeaponRivet::InitSystem(): Could not create projectile pool.\n" );
		goto _ExitWithError;
	}

	// Fill out our global info data for each EUK level...
	for( i=0; i<EUK_COUNT_RIVET; i++ ) {
		pInfo = &m_aaInfo[WEAPON_TYPE_RIVET_GUN][i];

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

	// Load our tracer texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_aUserProps[0].pszTracerTexName );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRivet::InitSystem(): Could not find texture '%s' for tracer.\n", m_aUserProps[0].pszTracerTexName );
		goto _ExitWithError;
	}
	m_TracerTexInst.SetTexDef( pTexDef );

	// Create our tracer group...
	m_hTracerGroup = tracer_CreateGroup( &m_TracerTexInst, MAX_TRACER_COUNT );
	if( m_hTracerGroup == TRACER_NULLGROUPHANDLE ) {
		DEVPRINTF( "CWeaponRivet::InitSystem(): Could not create tracer group.\n" );
		goto _ExitWithError;
	}

	m_TracerDef.uFlags = TRACERFLAG_NONE;
	m_TracerDef.ColorRGBA.fRed = 0.8f;
	m_TracerDef.ColorRGBA.fGreen = 0.8f;
	m_TracerDef.ColorRGBA.fBlue = 0.8f;
	m_TracerDef.ColorRGBA.fAlpha = 0.75f;
	m_TracerDef.fWidth_WS = 0.2f;
	m_TracerDef.fLength_WS = 100.0f;
	m_TracerDef.fBeginDeathUnitFade_WS = 0.25f;
	m_TracerDef.pFcnKillCallback = _TracerKilledCallback;
	m_TracerDef.pFcnBuildSkipList = _BuildTracerSkipListCallback;

	m_CollInfoArrowRayTest.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	m_CollInfoArrowRayTest.nCollMask = FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES;
	m_CollInfoArrowRayTest.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfoArrowRayTest.bCalculateImpactData = TRUE;
	m_CollInfoArrowRayTest.nTrackerUserTypeBitsMask = -1;
	m_CollInfoArrowRayTest.nStopOnFirstOfCollMask = 0;
	m_CollInfoArrowRayTest.bFindClosestImpactOnly = TRUE;
	m_CollInfoArrowRayTest.bCullBacksideCollisions = TRUE;
	m_CollInfoArrowRayTest.pTag = NULL;

	m_TrackerCollideRayInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	m_TrackerCollideRayInfo.nTrackerUserTypeBitsMask = -1;
	m_TrackerCollideRayInfo.bIgnoreCollisionFlag = FALSE;
	m_TrackerCollideRayInfo.bComputeIntersection = FALSE;
	m_TrackerCollideRayInfo.bSort = FALSE;
	m_TrackerCollideRayInfo.pCallback = _FindTrackersIntersectingArrowCallback;
	m_TrackerCollideRayInfo.nTrackerSkipCount = 0;
	m_TrackerCollideRayInfo.ppTrackerSkipList = NULL;

	_SetSmokeTrailAttributes();

	m_bSystemInitialized = TRUE;

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	tracer_DestroyGroup( m_hTracerGroup );
	m_hTracerGroup = TRACER_NULLGROUPHANDLE;

	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponRivet::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		// Destroy tracer group...
		tracer_DestroyGroup( m_hTracerGroup );
		m_hTracerGroup = TRACER_NULLGROUPHANDLE;

		m_bSystemInitialized = FALSE;
	}
}


void CWeaponRivet::_SetSmokeTrailAttributes( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1steam01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRivet::SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 0.3f;
	m_SmokeTrailAttrib.fScaleMax_WS = 0.4f;
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

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.25f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.35f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 0.8f, 0.8f, 0.8f );
	m_SmokeTrailAttrib.EndColorRGB.White();
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.2f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.6f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
}


BOOL CWeaponRivet::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );

	// Get pointer to the leaf class's builder object...
	CWeaponRivetBuilder *pBuilder = (CWeaponRivetBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponRivetBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONRIVET );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_RIVET_GUN, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponRivet::ClassHierarchyBuild( void ) {
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	_ResourceData_t *pResourceData;
	s32 nBoneIndex;
	u32 i;
	u32 uMeshEUK;
	u32 uResEUK;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponRivetBuilder *pBuilder = (CWeaponRivetBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_RIVET; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_RIVET );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponRivet::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );

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

			pResourceData->m_pAnimCombiner->AtRestMtxPalette();
		}
		
		nBoneIndex = pResourceData->m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE] );
		if( nBoneIndex < 0 ) {
			DEVPRINTF( "CWeaponRivet::ClassHierarchyBuild(): Bone '%s' doesn't exist in object '%s'.\n", m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE], Name() );
			goto _ExitWithError;
		}
		pResourceData->m_pFireMtx_WS = pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}

	// Build our clip mesh entity...
	m_pClipMeshEntity = fnew CMeshEntity;
	if( m_pClipMeshEntity == NULL ) {
		DEVPRINTF( "CWeaponRivet::Create(): Not enough memory to create the clip CMeshEntity.\n" );
		goto _ExitWithError;
	}

	if( !m_pClipMeshEntity->Create( m_aUserProps[0].apszMeshName[_MESH_CLIP] ) ) {
		DEVPRINTF( "CWeaponRivet::Create(): Could not create the clip CMeshEntity.\n" );
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


BOOL CWeaponRivet::ClassHierarchyBuilt( void ) {
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


void CWeaponRivet::ClassHierarchyDestroy( void ) {
	u32 i;

	fforce_Kill( &m_hForce );

	for( i=0; i<EUK_COUNT_RIVET; ++i ) {
		DestroySharedEUKData( i );
	}

	fdelete( m_pClipMeshEntity );

	_ClearDataMembers();

	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponRivet::GetLeafClassBuilder( void ) {
	return &_WeaponRivetBuilder;
}


void CWeaponRivet::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_RIVET; ++i ) {
		m_aResourceData[i].m_pAnimCombiner = NULL;
		m_aResourceData[i].m_pAnimRest = NULL;
		m_aResourceData[i].m_pWorldMesh = NULL;
		m_aResourceData[i].m_pFireMtx_WS = NULL;
	}

	m_pResourceData = NULL;
	m_pUserProps = NULL;

	m_pClipMeshEntity = NULL;

	m_nNextTracerID = 1;
	m_nRivetFlags = RIVET_FLAG_NONE;
	m_bFireThisFrame = FALSE;
	m_TargetPos_WS.Zero();
	m_pBuddyFirePos_WS = NULL;

	m_fSecondsCountdownTimer = 0.0f;

	m_nChargingTracerID = 0;
	m_pChargingArrowProj1 = NULL;
	m_pChargingArrowProj2 = NULL;

	fforce_NullHandle( &m_hForce );
}


void CWeaponRivet::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponRivet::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo );

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NUMERIC );
	}
}


void CWeaponRivet::_AddClipToWorld( void ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_pUserProps->apszBoneName[_BONE_CLIP_ATTACH], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();
}


void CWeaponRivet::_RemoveClipFromWorld( void ) {
	m_pClipMeshEntity->DetachFromParent();
	m_pClipMeshEntity->RemoveFromWorld();
}


void CWeaponRivet::ClassHierarchyAddToWorld( void ) {
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


void CWeaponRivet::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Detonate the arrow we're charging...
	_DetonateChargingArrow();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();

	// Remove clip from the world...
	if( m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED ) {
		_RemoveClipFromWorld();
	}
}


void CWeaponRivet::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	u32 i;

	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this weapon...

		for( i=0; i<EUK_COUNT_RIVET; ++i ) {
			FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );
	} else {
		// Disable drawing of this weapon...
		for( i=0; i<EUK_COUNT_RIVET; ++i ) {
			FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );
	}
}


void CWeaponRivet::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		// Handle animations and attached objects...
		m_pCallbackThis = this;
		m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );
		m_pCallbackThis = NULL;
		RelocateAllChildren();

	}
}


CFMtx43A *CWeaponRivet::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponRivet::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponRivet::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( nTrackerSkipListCount < FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;

	// Add our clip to the list...
	m_pClipMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}


void CWeaponRivet::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_RIVET_GUN][nPreviousUpgradeLevel];

	// Detonate the arrow we're charging...
	_DetonateChargingArrow();

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


void CWeaponRivet::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	// New compute method using the Primary_Fire bone...
	*pMuzzlePoint_WS = m_pResourceData->m_pFireMtx_WS->m_vPos;
}


u32 CWeaponRivet::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS ) {
	FASSERT( IsCreated() );

	m_bFireThisFrame = FALSE;

	if( m_nWeaponFlags & WEAPONFLAG_WAIT_FOR_TRIGGER_UP ) {
		// We're waiting for the trigger to be released...

		if( fUnitTriggerVal1 > 0.0f ) {
			// Trigger is still down...
			return 0;
		}

		// Trigger has been released...

		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_WAIT_FOR_TRIGGER_UP );

		// Detonate the arrow we're charging...
		m_nChargingTracerID = 0;
		_DetonateChargingArrow();
	}

	if( fUnitTriggerVal1 == 0.0f ) {
		// Trigger not down...

		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

		goto _ExitWithoutTriggerPressed;
	}

	// Trigger is down...

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

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		goto _ExitWithoutTriggerPressed;
	}

	m_TargetPos_WS.Set( *pTargetPos_WS );
	if( IsOwnedByPlayer() ) {
		m_pOwnerBot->FocusHumanTargetPoint_WS( &m_TargetPos_WS, this );
	}

	m_bFireThisFrame = TRUE;
	m_pBuddyFirePos_WS = pBuddyFirePos_WS;

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND | WEAPONFLAG_WAIT_FOR_TRIGGER_UP );

	return 1;

_ExitWithoutTriggerPressed:

	return 0;
}


void CWeaponRivet::ClassHierarchyWork( void ) {
	CWeapon::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( !m_bFireThisFrame ) {
		return;
	}

	CFVec3A FireUnitDir_WS;
	f32 fJitter;

	m_TracerDef.fMaxTailDist_WS = m_pUserProps->fMaxLiveRange;
	m_TracerDef.fSpeed_WS = m_pUserProps->fProjSpeed;
	m_TracerDef.pUser = this;

	if( m_nUpgradeLevel == 0 ) {
		m_TracerDef.nUser = 0;
		m_nChargingTracerID = 0;
	} else {
		m_TracerDef.nUser = m_nNextTracerID;
		m_nChargingTracerID = m_nNextTracerID;

		// Advance our tracer ID...
		if( ++m_nNextTracerID == 0 ) {
			// Skip ID 0...
			++m_nNextTracerID;
		}
	}

	fJitter = m_aUserProps[m_nUpgradeLevel].fShotSpreadFactor;
	if( m_pAttachedScope && m_pAttachedScope->IsZoomEnabled() ) {
		fJitter *= m_aUserProps[m_nUpgradeLevel].afScopeSpreadMult[ m_pAttachedScope->GetUpgradeLevel() ];
	}

	if( m_nUpgradeLevel != 3 ) {
		ComputeFireUnitDir( &FireUnitDir_WS, &m_pResourceData->m_pFireMtx_WS->m_vPos, &m_pResourceData->m_pFireMtx_WS->m_vZ, &m_TargetPos_WS, TRUE );
	} else {
		ComputeFireUnitDir( &FireUnitDir_WS, &m_pResourceData->m_pFireMtx_WS->m_vPos, &m_pResourceData->m_pFireMtx_WS->m_vZ, &m_TargetPos_WS, FALSE );
	}

	_FireRound( &m_pResourceData->m_pFireMtx_WS->m_vPos, &FireUnitDir_WS, fJitter, FALSE );

	if( m_pBuddyFirePos_WS ) {
		_FireRound( m_pBuddyFirePos_WS, &FireUnitDir_WS, fJitter, TRUE );
	}

	RemoveFromClip( 1 );
	
	m_bFireThisFrame = FALSE;
}


void CWeaponRivet::_FireRound( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fJitterFactor, BOOL bBuddy ) {
	CFVec3A FireUnitDir_WS;

	s32 nPlayerIndex = GetSafeOwnerPlayerIndex();

	fmath_ScatterUnitVec( &FireUnitDir_WS, pUnitDir_WS, fJitterFactor );

	if( !bBuddy ) {
		// Play firing sound...
		PlaySound( m_pUserProps->pSoundGroupFire );

		// Generate rumble...
		if( nPlayerIndex >= 0 ) {
			fforce_Kill( &m_hForce );

			switch( m_nUpgradeLevel ) {
			case 0:
				fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ARROW_THUNK_LIGHT, &m_hForce );
				break;

			case 1:
				fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ARROW_THUNK_MED, &m_hForce );
				break;

			default:
				fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ARROW_THUNK_HEAVY, &m_hForce );
				break;
			}
		}
	}

	// Spawn a tracer to represent the projectile...
	m_TracerDef.TailPos_WS = *pPos_WS;
	m_TracerDef.UnitDir_WS = FireUnitDir_WS;

	ConstructDamagerData();
	tracer_NewTracer( m_hTracerGroup, &m_TracerDef, 1.0f, TRUE, FALSE, &CDamageForm::m_TempDamager );

	// Spawn a smoke trail for the muzzle effect...
	smoketrail_SpawnMuzzlePuff( &m_SmokeTrailAttrib, pPos_WS, &FireUnitDir_WS, 3.0f );
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponRivet::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_bFireThisFrame = FALSE;
	m_fSecondsCountdownTimer = 0.0f;

	// Detonate the arrow we're charging...
	_DetonateChargingArrow();
}


void CWeaponRivet::BeginReload( void ) {
	TransferFromReserveToClip( (u16)m_pUserProps->fClipAmmoMax );
	ReloadComplete();
}


void CWeaponRivet::NotifyAmmoMightHaveChanged( void ) {
}


void CWeaponRivet::Clip_AttachToOwnerBotBone( cchar *pszBoneName ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();

	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );
}


void CWeaponRivet::Clip_AttachToWeapon( void ) {
	if( IsInWorld() ) {
		_AddClipToWorld();

		PlaySound( m_pUserProps->pSoundGroupAttachClip );
	}

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	Reload();
}


void CWeaponRivet::Clip_DiscardAttachedToOwnerBotBone( void ) {
	if( !(m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED) ) {
		m_pClipMeshEntity->RemoveFromWorld();
	}
}


void CWeaponRivet::Clip_Eject( void ) {
	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	_RemoveClipFromWorld();

	PlaySound( m_pUserProps->pSoundGroupEjectClip );

	EjectClip( &m_pClipMeshEntity->MtxToWorld()->m_vPos, m_pUserProps->pMeshEjectClip,m_pClipMeshEntity->ScaleToWorld() );
}


void CWeaponRivet::Clip_SlapIn( void ) {
	PlaySound( m_pUserProps->pSoundGroupSlapInClip );
}


void CWeaponRivet::CheckpointSaveSelect( s32 nCheckpoint ) {
	// Save weapon's clip...
	if( m_pClipMeshEntity ) {
		m_pClipMeshEntity->CheckpointSaveSelect( nCheckpoint );
	}

	// Save self...
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


void CWeaponRivet::_BuildTracerSkipListCallback( void *pUser ) {
	CWeaponRivet *pWeaponRivet = (CWeaponRivet *)pUser;

	if( pWeaponRivet->m_pOwnerBot ) {
		// Add our owner (this should also add us)...
		pWeaponRivet->m_pOwnerBot->AppendTrackerSkipList();
	} else {
		// Add ourselves...
		pWeaponRivet->AppendTrackerSkipList();
	}
}


void CWeaponRivet::_TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pCollImpact ) {
	CWeaponRivet *pWeaponRivet = (CWeaponRivet *)pTracerDef->pUser;

	if( nKillReason == TRACER_KILLREASON_HIT_GEO ) {
		pWeaponRivet->_StickArrowProjectile( pTracerDef, pCollImpact );
	}
}


void CWeaponRivet::_StickArrowProjectile( TracerDef_t *pTracerDef, const FCollImpact_t *pCollImpact ) {
	FASSERT( pCollImpact );

	CFVec3A ReflectionUnitVec_WS;

	ReflectionUnitVec_WS.ReceiveReflection( pTracerDef->UnitDir_WS, pCollImpact->UnitFaceNormal );

	m_pHitEntity = CGColl::ExtractEntity( pCollImpact );

	const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pCollImpact );
	u8 nProjReact = CGColl::ComputeProjectileReaction( pCollImpact );

	// Spawn a puff...
	pCollMaterial->DrawParticle(
		CGCollMaterial::PARTICLE_TYPE_DUST,
		&pCollImpact->ImpactPoint,
		&ReflectionUnitVec_WS,
		pCollMaterial->HasLooseDust() ? 0.5f : 0.25f
	);

	if( (nProjReact == CEntity::PROJECTILE_REACTION_RICOCHET) || (nProjReact == CEntity::PROJECTILE_REACTION_SHIELD) ) {
		// Ricochet the arrow...

		TracerDef_t RicochetTracerDef;

		fang_MemCopy( &RicochetTracerDef, pTracerDef, sizeof(TracerDef_t) );

		RicochetTracerDef.UnitDir_WS = ReflectionUnitVec_WS;
		RicochetTracerDef.TailPos_WS = pCollImpact->ImpactPoint;

		ConstructDamagerData();
		tracer_NewTracer( m_hTracerGroup, &RicochetTracerDef, 1.0f, TRUE, FALSE, &CDamageForm::m_TempDamager );

		// Inflict damage to the entity that the arrow hit...
		_InflictDamageToEntityArrowStruck( pTracerDef, pCollImpact );

		// Play ricochet sound...
		CFSoundGroup::PlaySound( m_pUserProps->pSoundGroupRicochet, FALSE, &pCollImpact->ImpactPoint );

		// Spawn some sparks...
		pCollMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST, &pCollImpact->ImpactPoint, &ReflectionUnitVec_WS, 0.5f );

		return;
	}

	if( CGColl::ExtractWire( pCollImpact ) ) {
		// We hit a wire. Ignore...

		TracerDef_t PassThroughTracerDef;

		fang_MemCopy( &PassThroughTracerDef, pTracerDef, sizeof(TracerDef_t) );
		PassThroughTracerDef.TailPos_WS = pCollImpact->ImpactPoint;

		ConstructDamagerData();
		tracer_NewTracer( m_hTracerGroup, &PassThroughTracerDef, 1.0f, TRUE, FALSE, &CDamageForm::m_TempDamager );

		return;
	}

	// Stick the arrow (the arrow projectile will handle detonation surfaces):

	// Spawn some bits...
	pCollMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_BITS, &pCollImpact->ImpactPoint, &ReflectionUnitVec_WS, 1.0f );

	// Draw the impact effect...
	if( m_pUserProps->hParticleImpact ) {
		CFVec3A ParticleImpactPos_WS, ParticleUnitDir_WS;

		ParticleUnitDir_WS.ReceiveNegative( pTracerDef->UnitDir_WS );
		ParticleImpactPos_WS.Mul( ParticleUnitDir_WS, 1.0f ).Add( pCollImpact->ImpactPoint );

		fparticle_SpawnEmitter( m_pUserProps->hParticleImpact, ParticleImpactPos_WS.v3, &ParticleUnitDir_WS.v3, 1.0f );
	}

	// Get a free projectile...
	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( m_hArrowProjPool );
	if( pProj == NULL ) {
		// No more projectiles...
		return;
	}

	if( m_nChargingTracerID && (pTracerDef->nUser == m_nChargingTracerID) ) {
		// Trigger is still down...

		_DetonateChargingArrow();

		m_pChargingArrowProj1 = pProj;
	}

	CFMtx43A ProjMtx;
	CFVec3A StickUnitDir_WS;
	CEProjExt::CEProj_Arrow_Params_t ArrowProjParams;
	f32 fProjRadius;

	fProjRadius = pProj->GetWorldMesh()->m_pMesh->BoundSphere_MS.m_fRadius;

	// Set the orientation matrix (apply a little randomness)...
	fmath_ScatterUnitVec( &StickUnitDir_WS, &pTracerDef->UnitDir_WS, 0.2f );
	ProjMtx.UnitMtxFromUnitVec( &StickUnitDir_WS );
	ProjMtx.m_vPos = pCollImpact->ImpactPoint;

	// Setup the projectile...
	pProj->Init();
	pProj->SetDamager( this, m_pOwnerBot );
	pProj->SetDetonateCallback( _ArrowDetonatedCallback );
	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, 1.0f );

	// Launch it!
	ArrowProjParams.pArrowIsStuckToEntity = m_pHitEntity;
	ArrowProjParams.pCollImpact = pCollImpact;
	ArrowProjParams.pArrowStaticParams = &m_pUserProps->ArrowProjStaticParams;
	pProj->SetArrowParams( &ArrowProjParams );
	pProj->Launch();

	// Inflict damage to the entity that the arrow hit...
	_InflictDamageToEntityArrowStruck( pTracerDef, pCollImpact );

	// Submit damage to any other entities that the arrow hit...
	_DamageNearbyEntities( &ProjMtx, fProjRadius, &pTracerDef->Damager );
}


// m_pHitEntity must be set up prior to this call.
void CWeaponRivet::_InflictDamageToEntityArrowStruck( TracerDef_t *pTracerDef, const FCollImpact_t *pCollImpact ) {
	if( m_pHitEntity ) {
		// The arrow hit an entity...

		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...

			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile = m_pUserProps->pDamageProfileImpact;
			pDamageForm->m_Damager = pTracerDef->Damager;
			pDamageForm->m_pDamageeEntity = m_pHitEntity;
			pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *)pCollImpact->pTag, pCollImpact, &pTracerDef->UnitDir_WS );

			CDamage::SubmitDamageForm( pDamageForm );
		}
	}
}


// m_pHitEntity must be set up prior to this call. This entity will be skipped.
void CWeaponRivet::_DamageNearbyEntities( const CFMtx43A *pArrowUnitMtx_WS, f32 fArrowRadius, const CDamageForm::Damager_t *pDamagerData ) {
	CFVec3A RayEnd_WS;

	// Ray test is from arrow center to arrow head...
	RayEnd_WS.Mul( pArrowUnitMtx_WS->m_vZ, fArrowRadius ).Add( pArrowUnitMtx_WS->m_vPos );
	m_CollInfoArrowRayTest.Ray.Init( &pArrowUnitMtx_WS->m_vPos, &RayEnd_WS );

	m_TrackerCollideRayInfo.StartPoint_WS = pArrowUnitMtx_WS->m_vPos;
	m_TrackerCollideRayInfo.EndPoint_WS = RayEnd_WS;

	m_nDamageFormCount = 0;
	m_pAttackUnitDir_WS = &pArrowUnitMtx_WS->m_vZ;

	fworld_CollideWithTrackers( &m_TrackerCollideRayInfo );

	if( m_nDamageFormCount ) {
		u32 i;

		for( i=0; i<m_nDamageFormCount; ++i ) {
			m_apDamageFormArray[i]->m_Damager = *pDamagerData;
			m_apDamageFormArray[i]->m_pDamageProfile = m_pUserProps->pDamageProfileImpact;

			CDamage::SubmitDamageForm( m_apDamageFormArray[i] );
		}
	}
}


BOOL CWeaponRivet::_FindTrackersIntersectingArrowCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection ) {
	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not an entity...
		return TRUE;
	}

	CEntity *pEntity = (CEntity *)pTracker->m_pUser;

	if( pEntity == m_pHitEntity ) {
		// Skip this entity since we've already dealt damage to it...
		return TRUE;
	}

	// Determine if arrow hits any triangles on this entity...

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	fcoll_Clear();
	m_CollInfoArrowRayTest.pTag = pTracker;
	pWorldMesh->CollideWithMeshTris( &m_CollInfoArrowRayTest );

	if( FColl_nImpactCount == 0 ) {
		// Our arrow doesn't collide with this entity...
		return TRUE;
	}

	// Our arrow has hit this entity...

	CDamageForm *pDamageForm;
	CFVec3A AttackUnitDir_WS;

	pDamageForm = CDamage::GetEmptyDamageFormFromPool();
	if( pDamageForm ) {
		// Fill out the form...

		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_pDamageeEntity = pEntity;
		pDamageForm->InitTriDataFromCollImpact( pWorldMesh, FColl_aImpactBuf, m_pAttackUnitDir_WS );

		FASSERT( m_nDamageFormCount < DAMAGE_FORM_COUNT );
		m_apDamageFormArray[ m_nDamageFormCount ] = pDamageForm;

		++m_nDamageFormCount;
	}

	if( m_nDamageFormCount >= DAMAGE_FORM_COUNT ) {
		return FALSE;
	}

	return TRUE;
}


void CWeaponRivet::_DetonateChargingArrow( void ) {
	if( m_pChargingArrowProj1 == NULL ) {
		return;
	}

	CEProj_Arrow *pProjArrow = (CEProj_Arrow *)m_pChargingArrowProj1->GetExtensionObject( CEProj::PROJTYPE_ARROW );

	if( pProjArrow->IsInTerminateMode() ) {
		m_pChargingArrowProj1->Detonate();
	}

	m_pChargingArrowProj1 = NULL;
}


BOOL CWeaponRivet::_ArrowDetonatedCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact ) {
	CWeaponRivet *pWeaponRivet = (CWeaponRivet *)pProj->GetDamagerWeapon();

	if( pWeaponRivet->m_pChargingArrowProj1 == pProj ) {
		pWeaponRivet->m_pChargingArrowProj1 = NULL;
	}

	return TRUE;
}


BOOL CWeaponRivet::IsArrowSuspendedByTriggerDown( CEProj *pProj ) {
	CWeaponRivet *pWeaponRivet = (CWeaponRivet *)pProj->GetDamagerWeapon();

	if( pWeaponRivet->m_pChargingArrowProj1 == pProj ) {
		return TRUE;
	}

	return FALSE;
}



