//////////////////////////////////////////////////////////////////////////////////////
// weapon_ripper.cpp - Small Projectile Emitter Weapon.
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
#include "weapon_ripper.h"
#include "fgamedata.h"
#include "fworld.h"
#include "fmesh.h"
#include "fanim.h"
#include "meshtypes.h"
#include "floop.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "potmark.h"
#include "fcamera.h"
#include "fresload.h"
#include "ItemInst.h"
#include "fsndfx.h"
#include "meshentity.h"
#include "AI/AIEnviro.h"
#include "weapon_scope.h"
#include "damage.h"
#include "fsound.h"


#define _USER_PROP_FILENAME		"w_ripper.csv"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRipperBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponRipperBuilder _WeaponRipperBuilder;

void CWeaponRipperBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONRIPPER, pszEntityType );
}


BOOL CWeaponRipperBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponRipperBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRipper
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponRipper::_UserProps_t CWeaponRipper::m_aUserProps[EUK_COUNT_RIPPER];
CDamageProfile *CWeaponRipper::m_apImpactDamageProfile[EUK_COUNT_RIPPER];
CDamageProfile *CWeaponRipper::m_apCarveDamageProfile[EUK_COUNT_RIPPER];
CEProjPool::PoolHandle_t CWeaponRipper::m_ahProjPool[EUK_COUNT_RIPPER];
CFTexInst CWeaponRipper::_aStreamerTexInst[EUK_COUNT_RIPPER];
CEProjExt::CEProj_Saw_Params_t CWeaponRipper::m_aSawParms[EUK_COUNT_RIPPER];
CWeaponRipper *CWeaponRipper::m_pCallbackThis;
f32 CWeaponRipper::m_afSecsBetweenEmptySounds[EUK_COUNT_RIPPER];


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponRipper::m_aUserPropVocab[] = {
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

	// apszMeshName[_MESH_SAW]:
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

	// fProjectilesInPoolCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( f32 ),
	F32_DATATABLE_0,
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

	// fSawCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fSawScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fSawSpeed:
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

	// pszImpactDamageProfile:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszCarveDamageProfile:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSawCarveDamageSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_30,

	// fSawCarveInitialSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// nMinRicochetCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_20,

	// nMaxRicochetCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_20,

	// pszStreamerTexName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fStreamerAlpha:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fStreamerWidth:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// nStreamerVtxCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( f32 ),
	F32_DATATABLE_2,
	F32_DATATABLE_100,

	// fStreamerSamplesPerSec:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_60,

	// fDamageSphereRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleScale_Impact:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
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

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroupShatter

	FGAMEDATA_VOCAB_MESH,			// pMeshEjectClip

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEmpty
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEjectClip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAttachClip
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupSlapInClip

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupRicochet
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupStick
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCutStart
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCutStop
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCutLoop

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponRipper::m_aUserPropMapTable[] = {
	"RipperL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"RipperL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"RipperL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	NULL
};



BOOL CWeaponRipper::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponRipper::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_RIPPER; i++ ) {
		// Create a pool of projectiles...
		m_ahProjPool[i] = CEProjPool::Create( CEProj::PROJTYPE_SAW, m_aUserProps[i].apszMeshName[_MESH_SAW], m_aUserProps[i].nProjectilesInPoolCount );

		if( m_ahProjPool[i] == EPROJPOOL_NULL_HANDLE ) {
			DEVPRINTF( "CWeaponRipper::InitSystem(): Could not create projectile pool.\n" );
			goto _ExitWithError;
		}

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_RIPPER][i];

		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_CLIP;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->fUnitRecoil = m_aUserProps[i].fUnitRecoil;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_LEFT_HAND_RELOADS | INFOFLAG_SCOPE_ENABLED | INFOFLAG_THICK_TARGETING | INFOFLAG_DISPLAY_AMMO_TEXT;
		pInfo->nReticleType = CReticle::TYPE_DROID_STANDARD;

		// Load streamer texture...
		_aStreamerTexInst[i].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_aUserProps[i].pszStreamerTexName ) );
		_aStreamerTexInst[i].SetFlags( CFTexInst::FLAG_WRAP_T );

		// Load damage profiles...
		m_apImpactDamageProfile[i] = CDamage::FindDamageProfile( m_aUserProps[i].pszImpactDamageProfile );
		m_apCarveDamageProfile[i] = &CDamage::m_DamageProfileNone;

		if( i == 2 ) {
			if( m_aUserProps[i].pszCarveDamageProfile && m_aUserProps[i].pszCarveDamageProfile[0] ) {
				m_apCarveDamageProfile[i] = CDamage::FindDamageProfile( m_aUserProps[i].pszCarveDamageProfile );
			}
		}

		// Set up projectile parameters...
		m_aSawParms[i].nMinRicochetCount = m_aUserProps[i].nMinRicochetCount;
		m_aSawParms[i].nMaxRicochetCount = m_aUserProps[i].nMaxRicochetCount;
		m_aSawParms[i].bCarvingAllowed = (i == 2);
		m_aSawParms[i].pCarveDamageProfile = m_apCarveDamageProfile[i];
		m_aSawParms[i].fCarveDamageSecs = m_aUserProps[i].fSawCarveDamageSecs;
		m_aSawParms[i].fCarveInitialSpeed = m_aUserProps[i].fSawCarveInitialSpeed;
		m_aSawParms[i].fDamageSphereRadius = m_aUserProps[i].fDamageSphereRadius;
		m_aSawParms[i].fMuzzleScale_Impact = m_aUserProps[i].fMuzzleScale_Impact;
		m_aSawParms[i].fMuzzleAlpha_Impact = m_aUserProps[i].fMuzzleAlpha_Impact;
		m_aSawParms[i].fMuzzleOffset_Impact = m_aUserProps[i].fMuzzleOffset_Impact;

		m_aSawParms[i].pSoundGroupRicochet = m_aUserProps[i].pSoundGroupRicochet;
		m_aSawParms[i].pSoundGroupStick = m_aUserProps[i].pSoundGroupStick;
		m_aSawParms[i].pSoundGroupCutStart = m_aUserProps[i].pSoundGroupCutStart;
		m_aSawParms[i].pSoundGroupCutStop = m_aUserProps[i].pSoundGroupCutStop;
		m_aSawParms[i].pSoundGroupCutLoop = m_aUserProps[i].pSoundGroupCutLoop;

		m_afSecsBetweenEmptySounds[i] = fsndfx_GetDuration( CFSoundGroup::GetSoundHandle( m_aUserProps[i].pSoundGroupEmpty, 0 ) );
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponRipper::UninitSystem( void ) {
}



CWeaponRipper::CWeaponRipper() {
	_ClearDataMembers();
}



CWeaponRipper::~CWeaponRipper() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}



BOOL CWeaponRipper::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponRipperBuilder *pBuilder = (CWeaponRipperBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponRipperBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONRIPPER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_RIPPER, pszEntityName, pMtx, pszAIBuilderName );
}



BOOL CWeaponRipper::ClassHierarchyBuild( void ) {
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
	CWeaponRipperBuilder *pBuilder = (CWeaponRipperBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_RIPPER; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_RIPPER );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}


		pInfo = &m_aaInfo[WEAPON_TYPE_RIPPER][i];

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponRipper::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );

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
				DEVPRINTF( "CWeaponTether::Create(): Could not create animation combiner.\n" );
				goto _ExitWithError;
			}

#if 0
			if( i == 2 ) {
				pResourceData->m_pAnimCombiner->SetBoneCallback( _CartridgeBoneCallback );
				pResourceData->m_pAnimCombiner->DisableAllBoneCallbacks();
				pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[i].apszBoneName[_BONE_L3_SPIN_CARTRIDGE] );
			}
#endif
		}
	}

	// Build our clip mesh entity...
	m_pClipMeshEntity = fnew CMeshEntity;
	if( m_pClipMeshEntity == NULL ) {
		DEVPRINTF( "CWeaponRipper::Create(): Not enough memory to create the clip CMeshEntity.\n" );
		goto _ExitWithError;
	}

	if( !m_pClipMeshEntity->Create( m_aUserProps[0].apszMeshName[_MESH_CLIP] ) ) {
		DEVPRINTF( "CWeaponRipper::Create(): Could not create the clip CMeshEntity.\n" );
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

	fforce_NullHandle( &m_hForce );
	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponRipper::ClassHierarchyBuilt( void ) {
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


void CWeaponRipper::ClassHierarchyDestroy( void ) {
	u32 i;

	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	for( i=0; i<EUK_COUNT_RIPPER; ++i ) {
		DestroySharedEUKData( i );
	}

	fdelete( m_pClipMeshEntity );

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponRipper::GetLeafClassBuilder( void ) {
	return &_WeaponRipperBuilder;
}


void CWeaponRipper::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_RIPPER; ++i ) {
		m_aResourceData[i].m_pAnimCombiner = NULL;
		m_aResourceData[i].m_pAnimRest = NULL;
		m_aResourceData[i].m_pWorldMesh = NULL;
	}

	m_pResourceData = NULL;
	m_pUserProps = NULL;

	m_pClipMeshEntity = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_fPlayEmptySoundTimer = 0.0f;
	m_bFireThisFrame = FALSE;
	m_TargetPos_WS.Zero();
	m_pBuddyFirePos_WS = NULL;
}


void CWeaponRipper::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponRipper::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst(pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);
	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NUMERIC );
	}
}


void CWeaponRipper::_AddClipToWorld( void ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_pUserProps->apszBoneName[_BONE_CLIP_ATTACH], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();
}


void CWeaponRipper::_RemoveClipFromWorld( void ) {
	m_pClipMeshEntity->DetachFromParent();
	m_pClipMeshEntity->RemoveFromWorld();
}


void CWeaponRipper::ClassHierarchyAddToWorld( void ) {
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


void CWeaponRipper::ClassHierarchyRemoveFromWorld( void ) {
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


void CWeaponRipper::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

//	if( IsInWorld() ) 
	{
		u32 i;

		if( bDrawingHasBeenEnabled ) {
			// Enable drawing of this weapon...

			for( i=0; i<EUK_COUNT_RIPPER; ++i ) {
				FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}

			m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );
		} else {
			// Disable drawing of this weapon...
			for( i=0; i<EUK_COUNT_RIPPER; ++i ) {
				FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}

			m_pClipMeshEntity->DrawEnable( bDrawingHasBeenEnabled );
		}
	}
}


void CWeaponRipper::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		m_pCallbackThis = this;
		m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );
		RelocateAllChildren();
		m_pCallbackThis = NULL;
	}
}


CFMtx43A *CWeaponRipper::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponRipper::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponRipper::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;

	// Add our clip to the list...
	m_pClipMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}


void CWeaponRipper::_BuildProjSkipList( CEProj *pProj ) {
	CWeaponRipper *pWeaponRipper = (CWeaponRipper *)pProj->GetDamagerWeapon();

	if( pWeaponRipper->m_pOwnerBot ) {
		pWeaponRipper->m_pOwnerBot->AppendTrackerSkipList();
	}
}


// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponRipper::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_RIPPER][nPreviousUpgradeLevel];

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


void CWeaponRipper::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	pMuzzlePoint_WS->Mul( m_MtxToWorld.m_vFront, m_pUserProps->fDistFromWeaponOrigToMuzzle ).Add( m_MtxToWorld.m_vPos );
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
u32 CWeaponRipper::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPoint_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );

	m_bFireThisFrame = FALSE;

	if( fUnitTriggerVal1 < 0.25f ) {
		// Trigger not down...

		if( m_fPlayEmptySoundTimer <= 0.0f ) {
			FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND | WEAPONFLAG_WAIT_FOR_TRIGGER_UP );
		}

		return 0;
	}

	// Trigger is down...

	if( (m_nUpgradeLevel == 0) && (m_nWeaponFlags & WEAPONFLAG_WAIT_FOR_TRIGGER_UP) ) {
		// We need to wait until the player releases the trigger for the L1 version of this weapon...
		return 0;
	}

	if( m_nClipAmmo == 0 ) {
		// No rounds in clip...

		if( m_nReserveAmmo == 0 ) {
			// Completely out of ammo. Make empty click sound...

			if( !(m_nWeaponFlags & WEAPONFLAG_MADE_EMPTY_CLICK_SOUND) ) {
				if( m_fPlayEmptySoundTimer <= 0.0f ) {
					FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

					PlaySound( m_pUserProps->pSoundGroupEmpty );
					m_fPlayEmptySoundTimer = m_afSecsBetweenEmptySounds[ m_nUpgradeLevel ];
				}
			}
		}

		return 0;
	}

	m_fPlayEmptySoundTimer = 0.0f;

	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		return 0;
	}

	if( !(m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED) ) {
		// The clip is not installed...
		return 0;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return 0;
	}

	if( m_nUpgradeLevel == 0 ) {
		FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_WAIT_FOR_TRIGGER_UP );
	}

	m_TargetPos_WS.Set( *pvTargetPoint_WS );
	if ( m_pOwnerBot && m_pOwnerBot->m_nPossessionPlayerIndex >= 0 ) {
		AIEnviro_BoostPlayerSoundTo(m_pOwnerBot->m_nPossessionPlayerIndex, 30.0f);
		m_pOwnerBot->FocusHumanTargetPoint_WS( &m_TargetPos_WS, this );
	}

	m_bFireThisFrame = TRUE;
	m_pBuddyFirePos_WS = pBuddyFirePos_WS;

	return 1;
}


// Called once per frame to update animations, timers, etc.
void CWeaponRipper::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( m_fPlayEmptySoundTimer > 0.0f ) {
		m_fPlayEmptySoundTimer -= FLoop_fPreviousLoopSecs;
	}

	if( m_bFireThisFrame ) {
		CFVec3A FireUnitDir_WS;
		f32 fJitterFactor;

		fJitterFactor = m_pUserProps->fShotSpreadFactor;
		if( m_pAttachedScope && m_pAttachedScope->IsZoomEnabled() ) {
			fJitterFactor *= m_pUserProps->afScopeSpreadMult[ m_pAttachedScope->GetUpgradeLevel() ];
		}

		CFVec3A MuzzlePoint;
		ComputeMuzzlePoint_WS( &MuzzlePoint );
		ComputeFireUnitDir( &FireUnitDir_WS, &MuzzlePoint, &m_MtxToWorld.m_vFront, &m_TargetPos_WS, FALSE );

		_FireRound( &MuzzlePoint, &FireUnitDir_WS, fJitterFactor, FALSE );

		if( m_pBuddyFirePos_WS ) {
			_FireRound( m_pBuddyFirePos_WS, &FireUnitDir_WS, fJitterFactor, TRUE );
		}

		m_fSecondsCountdownTimer += m_pUserProps->fOORoundsPerSec;
		RemoveFromClip(1);
		FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}
}


void CWeaponRipper::_FireRound( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, f32 fJitterFactor, BOOL bBuddy ) {
	CFVec3A FireUnitDir_WS;

	fmath_ScatterUnitVec( &FireUnitDir_WS, pUnitDir_WS, fJitterFactor );

	if( !bBuddy ) {
		PlaySound( m_pUserProps->pSoundGroupFire );

		if( IsOwnedByPlayer() ) {
			s32 nPlayerIndex = GetOwner()->m_nPossessionPlayerIndex;
			FASSERT( nPlayerIndex >= 0 );

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

	// Get a free projectile...
	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( m_ahProjPool[m_nUpgradeLevel] );
	if( pProj == NULL ) {
		// No more projectiles...
		return;
	}

	// Let's fire a round...

	CFMtx43A ProjMtx;

	ProjMtx.UnitMtxFromUnitVec( &FireUnitDir_WS );
	ProjMtx.m_vPos = *pPos_WS;

	pProj->Init();
	pProj->SetDamager( this, m_pOwnerBot );
	pProj->SetDamageProfile( m_apImpactDamageProfile[ m_nUpgradeLevel ] );
	pProj->SetSkipListCallback( _BuildProjSkipList );
	pProj->SetMaxDistCanTravel( m_pUserProps->fMaxLiveRange );
	pProj->SetLinSpeed( m_pUserProps->fSawSpeed );
	pProj->SetLinUnitDir_WS( &FireUnitDir_WS );
	pProj->SetExplosionGroup( m_pUserProps->hExplosionGroupShatter );
	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, m_pUserProps->fSawScale );

	pProj->SetSawParams( &m_aSawParms[m_nUpgradeLevel] );

	if( _aStreamerTexInst[m_nUpgradeLevel].GetTexDef() ) {
		pProj->StreamerOn(
			&_aStreamerTexInst[m_nUpgradeLevel],
			m_pUserProps->fStreamerAlpha,
			CFXStreamerEmitter::USE_RIGHT_AXIS,
			m_pUserProps->fStreamerWidth,
			m_pUserProps->nStreamerVtxCount,
			m_pUserProps->fStreamerSamplesPerSec
		);
	}

	pProj->Launch();
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponRipper::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_bFireThisFrame = FALSE;
	m_fSecondsCountdownTimer = 0.0f;
	m_fPlayEmptySoundTimer = 0.0f;
}


void CWeaponRipper::BeginReload( void ) {
	TransferFromReserveToClip( (u16)m_pUserProps->fClipAmmoMax );
	ReloadComplete();
}


void CWeaponRipper::NotifyAmmoMightHaveChanged( void ) {
}


void CWeaponRipper::Clip_AttachToOwnerBotBone( cchar *pszBoneName ) {
	m_pClipMeshEntity->Attach_UnitMtxToParent_PS_NewScale_PS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_pClipMeshEntity->AddToWorld();

	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );
}


void CWeaponRipper::Clip_AttachToWeapon( void ) {
	if( IsInWorld() ) {
		_AddClipToWorld();

		PlaySound( m_pUserProps->pSoundGroupAttachClip );
	}

	FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	Reload();
}


void CWeaponRipper::Clip_DiscardAttachedToOwnerBotBone( void ) {
	if( !(m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED) ) {
		m_pClipMeshEntity->RemoveFromWorld();
	}
}


void CWeaponRipper::Clip_Eject( void ) {
	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_CLIP_INSTALLED );

	_RemoveClipFromWorld();

	PlaySound( m_pUserProps->pSoundGroupEjectClip );

	EjectClip( &m_pClipMeshEntity->MtxToWorld()->m_vPos, m_pUserProps->pMeshEjectClip );
}


void CWeaponRipper::Clip_SlapIn( void ) {
	PlaySound( m_pUserProps->pSoundGroupSlapInClip );
}


#if 0
void CWeaponRipper::_CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	CFMtx43A::m_RotZ.SetRotationZ( m_pCallbackThis->m_fCartridgeSpinAngle );
	rNewMtx.Mul( rParentMtx, rBoneMtx );
	rNewMtx.Mul( CFMtx43A::m_RotZ );
}
#endif

