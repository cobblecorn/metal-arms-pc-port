//////////////////////////////////////////////////////////////////////////////////////
// weapon_chaingun.cpp - Titan's chain gun
//
// Author: Mike Elliott
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
// 10/29/02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_chaingun.h"
#include "fworld_coll.h"
#include "fanim.h"
#include "bot.h"
#include "meshtypes.h"
#include "ftext.h"
#include "fresload.h"
#include "potmark.h"
#include "AI/AIEnviro.h"
#include "eshield.h"
#include "player.h"


#define _USER_PROP_FILENAME					"w_chaingun.csv"
#define _BARREL_SMOKE_THRESHOLD				0.25f

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponChaingunBuilder
// 
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponChaingunBuilder _WeaponChaingunBuilder;

void CWeaponChaingunBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_UNSPECIFIED, pszEntityType );
}


BOOL CWeaponChaingunBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponChaingunBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponChaingun
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL	CWeaponChaingun::m_bSystemInitialized;

CWeaponChaingun::_UserProps_t	CWeaponChaingun::m_aUserProps[EUK_COUNT_CHAINGUN];
CDamageProfile*					CWeaponChaingun::m_apDamageProfile[EUK_COUNT_CHAINGUN];

CWeaponChaingun *CWeaponChaingun::m_pCallbackThis;

//DebrisInitSettings_t	CWeaponChaingun::m_DebrisInitSettings;
FMesh_t*				CWeaponChaingun::m_apShellMesh[EUK_COUNT_CHAINGUN];
u32						CWeaponChaingun::m_uWpnClassClientCount = 0;


const FGameData_TableEntry_t CWeaponChaingun::m_aUserPropVocab[] = {
		//cchar *apszMeshName;				// Mesh names
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//cchar *apszBoneName[_BONE_PRIMARY_FIRE];		// Bone names
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//cchar *apszBoneName[_BONE_SPIN_BARRELS];		// Bone names
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//cchar *apszBoneName[_BONE_SPIN_CARTRIDGE];	// Bone names
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,


		// f32 fMaxAmmo;						// Maximum number of clip rounds (<0 means infinite)
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_Neg1,
		F32_DATATABLE_65534,

		//f32 fRoundsPerSec;					// Maximum fire rate
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
		sizeof( f32 ) * 2,
		F32_DATATABLE_0,
		F32_DATATABLE_65534,

		//f32 fWeaponCullDist;				// Don't draw the weapon if it's this far from the camera
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMaxLiveRange;					// The maximum distance the projectile can travel before self-destructing
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_1,
		F32_DATATABLE_100000,
	
		//f32 fDistFromWeaponOrigToMuzzle;	// Distance from the weapon's origin to where the projectiles come out
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_1,
		F32_DATATABLE_100000,

		//f32 fMinTargetAssistDist;			// The minimum targeting assistance distance (closer than this gets clamped)
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMaxTargetAssistDist;			// The maximum targeting assistance distance (farther will not get assisted) (0=no targeting assistance)
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fUnitCamVibration;				// Amount camera vibrates when firing
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//f32 fBestShotSpreadFactor;			// Tightest shot spread factor
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//f32 fWorstShotSpreadFactor;			// Widest shot spread factor
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//f32 fShotSpreadSpeed;				// Speed at which shot spreads out
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fShotSpreadRecoverySpeed;		// Speed at which shot spreads in
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMuzzleWidth_Z;					// Width of the muzzle flash Z
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMuzzleHeight_Z;				// Height of the muzzle flash Z
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMuzzleAlpha_Z;					// Alpha of muzzle flash Z
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//f32 fMuzzleScale_XY;				// Scale of muzzle flash XY
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMuzzleAlpha_XY;				// Alpha of muzzle flash XY
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//f32 fMuzzleScale_Mesh;				// Scale of muzzle flash mesh
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMuzzleRot_Mesh;				// Rotation of muzzle flash mesh
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//f32 fMuzzleScale_Impact;			// Scale of impact flash
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fMuzzleAlpha_Impact;			// Alpha of impact flash
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//f32 fMuzzleOffset_Impact;			// Offset of impact flash
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//f32 fBarrelsMaxRevsPerSec;			// The maximum revolutions per second of the barrels
		//f32 fBarrelsOOMaxRevsPerSec;		// F32_DATATABLE_1 / fBarrelsMaxRevsPerSec
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
		sizeof( f32 ) * 2,
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fSpinupTIme
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
		sizeof( f32 ) * 2,
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 fSpindownTime;				// The barrels rotation deceleration
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
		sizeof( f32 ) * 2,
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		//f32 pszDamageProfile;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//fMuzzleLightRadius;		
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_50,

		//fCartridgeRPS;			
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10,

		//fBarrelIdleRPS
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10,

		//fHeatUpRate;			
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10,

		//fCoolDownRate;
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10,

		//fOverheatPenalty;		
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//fCoolDownThreshold;		
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,


		//pszSmokeParticleName;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//pszShellMeshName;	
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//uBarrelHeatTexID;		
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_CONVERT_TO_U32,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//uBarrelFrontHeatTexID;		
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_CONVERT_TO_U32,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//f32 fSoundRadius;					// Distance sound from this weapon travels
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10000000,

		//FSndFx_FxHandle_t hSndFire;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
		sizeof( u32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//FSndFx_FxHandle_t hSndSpin;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
		sizeof( u32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//FSndFx_FxHandle_t hSndSpinup;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
		sizeof( u32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//FSndFx_FxHandle_t hSndSpindown;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
		sizeof( u32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//FSndFx_FxHandle_t hSndOverheat;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
		sizeof( u32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		FGAMEDATA_VOCAB_DECAL_DEF,	// hDecalDef

		// End of table:
		FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponChaingun::m_aUserPropMapTable[] = {
	"Chaingun_Glitch",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"Chaingun_Titan",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"Chaingun_TitanPossessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	NULL
};


BOOL CWeaponChaingun::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );


	m_uWpnClassClientCount = 0;

#if 0
	// init our debris settings
	m_DebrisInitSettings.bSampleDynamicLights	= TRUE;
	m_DebrisInitSettings.fMinRotVel				= 0.65f;
	m_DebrisInitSettings.fMaxRotVel				= 1.15f;
	m_DebrisInitSettings.fCullDist2				= (85.0f * 85.0f);
	m_DebrisInitSettings.fLinearFrictionPerSec	= 20.0f;
	m_DebrisInitSettings.fRotFrictionPerSec		= 10.0f;
	m_DebrisInitSettings.fGravityPerSec			= 40.0f;
	m_DebrisInitSettings.pStateChangeCBFunc		= NULL;
#endif
	
	m_bSystemInitialized = TRUE;

	return TRUE;
}


BOOL CWeaponChaingun::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( m_uWpnClassClientCount != 0xffffffff );

	m_uWpnClassClientCount++;

	if( !CWeapon::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...
		return FALSE;
	}

	if( m_uWpnClassClientCount > 1 ) {
		// Resources already loaded...
		return TRUE;
	}

    Info_t *pInfo;
	FResFrame_t ResFrame = fres_GetFrame();

	fresload_Load( FSNDFX_RESTYPE, "Titan" );

	fang_MemZero( m_aUserProps, sizeof( m_aUserProps ) );

	//Read all user properties
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponChaingun::ClassHierarchyLoadSharedResources(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// fill out a Info_t structure for each type
	
	// Do this for each EUK level...
	for( u32 i=0; i<EUK_COUNT_CHAINGUN; i++ )
	{
		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_CHAINGUN][i];

		pInfo->nGrip				= GRIP_DROPPED_FOR_BLINK;
		pInfo->nReloadType			= RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType			= STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange		= m_aUserProps[i].fMaxLiveRange;
		pInfo->nInfoFlags			= INFOFLAG_NONE;
		pInfo->nReticleType			= CReticle::TYPE_DROID_STANDARD;

		m_apDamageProfile[i] = CDamage::FindDamageProfile( m_aUserProps[i].pszDamageProfile );

		m_apShellMesh[i] = (FMesh_t*)fresload_Load( FMESH_RESTYPE,  m_aUserProps[i].pszShellMeshName );
		if( m_apShellMesh[i] == NULL ) {
			DEVPRINTF( "CWeaponChaingun::ClassHierarchyLoadSharedResources(): Could not load shell mesh '%s'.\n", m_aUserProps[i].pszShellMeshName );
			goto _ExitWithError;
		}
	}

	return TRUE;

_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;

}


void CWeaponChaingun::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_uWpnClassClientCount > 0 );

	m_uWpnClassClientCount--;

	if( m_uWpnClassClientCount > 0 ) {
		return;
	}
}


void CWeaponChaingun::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


CWeaponChaingun::CWeaponChaingun() {
	_ClearDataMembers();
}


CWeaponChaingun::~CWeaponChaingun() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponChaingun::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}


	// Get pointer to the leaf class's builder object...
	CWeaponChaingunBuilder *pBuilder = (CWeaponChaingunBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponChaingunBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONCHAINGUN );
	}

	

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_CHAINGUN, pszEntityName, pMtx, pszAIBuilderName );
}


// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponChaingun::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_FLAMER][nPreviousUpgradeLevel];

	// Remove old mesh from world...
	pOldResourceData->m_pWorldMesh->RemoveFromWorld();

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}
}


BOOL CWeaponChaingun::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	u32 i;
	FMesh_t			*pMesh;
	FMeshInit_t		 MeshInit;
	_ResourceData_t *pResourceData;
	u32 uResEUK;

	//get frame
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponChaingunBuilder *pBuilder = (CWeaponChaingunBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	//Init from builder object

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_CHAINGUN; ++i, ++pResourceData ) {
		uResEUK = i;

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[i].pszMeshName );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponChaingun::Create(): Could not find mesh '%s'.\n", m_aUserProps[i].pszMeshName );

			pMesh = m_pErrorMesh;
			if( pMesh == NULL ) {
				goto _ExitWithError;
			}
		}

		// Init the world mesh...
		MeshInit.pMesh		= pMesh;
		MeshInit.nFlags		= FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist  = m_aUserProps[i].fWeaponCullDist;
		MeshInit.Mtx		= *MtxToWorld();

		m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
		m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

		//pResourceData->m_pWorldMesh->Init( &MeshInit );
		if( !CreateSharedEUKData( &MeshInit, &uResEUK, &(pResourceData->m_pWorldMesh), &(pResourceData->m_pAnimCombiner), &(pResourceData->m_pAnimRest) ) ) {
			DEVPRINTF( "CWeaponSpew::ClassHierarchyBuild():  Error creating EUK shared data\n" );
			goto _ExitWithError;
		}

		if( uResEUK == i ) {
			pResourceData->m_pWorldMesh->RemoveFromWorld();
			pResourceData->m_pWorldMesh->SetCollisionFlag( TRUE );
			pResourceData->m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
			pResourceData->m_pWorldMesh->m_pUser = this;
			pResourceData->m_pWorldMesh->SetUserTypeBits( TypeBits() );
			pResourceData->m_pWorldMesh->SetLineOfSightFlag(FALSE); //pgm added this so that weapons never block los tests


		//// Create our mesh rest anim...
		//pResourceData->m_pAnimRest = fnew CFAnimMeshRest;
		//if( pResourceData->m_pAnimRest == NULL ) {
		//	DEVPRINTF( "CWeaponChaingun::Create(): Not enough memory to create CFAnimMeshRest.\n" );
		//	goto _ExitWithError;
		//}

		//if( !pResourceData->m_pAnimRest->Create( pResourceData->m_pWorldMesh->m_pMesh ) ) {
		//	DEVPRINTF( "CWeaponChaingun::Create(): Could not create CFAnimMeshRest.\n" );
		//	goto _ExitWithError;
		//}

		// Create our animation combiner...
		//pResourceData->m_pAnimCombiner = fnew CFAnimCombiner;
		//if( pResourceData->m_pAnimCombiner == NULL ) {
		//	DEVPRINTF( "CWeaponChaingun::Create(): Not enough memory to create CFAnimCombiner.\n" );
		//	goto _ExitWithError;
		//}

			if( !pResourceData->m_pAnimCombiner->CreateSimple( pResourceData->m_pAnimRest, pResourceData->m_pWorldMesh ) ) {
				DEVPRINTF( "CWeaponChaingun::Create(): Could not create animation combiner.\n" );
				goto _ExitWithError;
			}

			pResourceData->m_pAnimCombiner->SetBoneCallback( _SpinBoneCallback );
			pResourceData->m_pAnimCombiner->DisableAllBoneCallbacks();
			pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[i].apszBoneName[_BONE_SPIN_BARRELS]	);
			pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[i].apszBoneName[_BONE_SPIN_CARTRIDGE]  );
		}

		//now grab our bone ids
		m_auPriFireBone[i]			= m_pResourceData->m_pWorldMesh->FindBone( m_aUserProps[i].apszBoneName[_BONE_PRIMARY_FIRE] );
		m_auSpinBarrelBone[i]		= m_pResourceData->m_pWorldMesh->FindBone( m_aUserProps[i].apszBoneName[_BONE_SPIN_BARRELS] ); 
		m_auSpinCartridgeBones[i]	= m_pResourceData->m_pWorldMesh->FindBone( m_aUserProps[i].apszBoneName[_BONE_SPIN_CARTRIDGE] );

		if( (m_auPriFireBone[i] < 0) ||
			(m_auSpinBarrelBone[i] < 0) ||
			(m_auSpinCartridgeBones[i] < 0) ) {
			DEVPRINTF( "CWeaponChaingun::Create(): Could not locate bones in mesh.\n" );
			goto _ExitWithError;
		}
	}

	fforce_NullHandle( &m_hForce );

	if( m_nUpgradeLevel == 0 ) {
		m_hHeatTexLayerBarrel = m_pResourceData->m_pWorldMesh->GetTexLayerHandle( m_pUserProps->uBarrelHeatTexID );
		if( m_hHeatTexLayerBarrel < 0 ) {
			DEVPRINTF( "CWeaponChaingun::ClassHierarchyBuild(): Couldn't get tex layer handle for heat effect.\n" );
			goto _ExitWithError;
		}

		m_hHeatTexLayerFront = m_pResourceData->m_pWorldMesh->GetTexLayerHandle( m_pUserProps->uBarrelFrontHeatTexID );
		if( m_hHeatTexLayerFront < 0 ) {
			DEVPRINTF( "CWeaponChaingun::ClassHierarchyBuild(): Couldn't get tex layer handle for heat effect.\n" );
			goto _ExitWithError;
		}
		
		// set the barrels to be cold
		m_pResourceData->m_pWorldMesh->LayerAlpha_Set( m_hHeatTexLayerBarrel, 0.0f );
		m_pResourceData->m_pWorldMesh->LayerAlpha_Set( m_hHeatTexLayerFront, 0.0f  );
	}

	m_pSmokeParticle = fnew CEParticle();
	if( m_pSmokeParticle == NULL ) {
		DEVPRINTF( "CWeaponChaingun::ClassHierarchyBuild(): Not enough memory to create CEParticle.\n" );
		goto _ExitWithError;			
	}

	m_pSmokeParticle->Create();
	m_hSmokeParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_pUserProps->pszSmokeParticleName );
	m_pSmokeParticle->RemoveFromWorld();

	if( m_hSmokeParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CWeaponChaingun::ClassHierarchyBuild(): Could not find particle definition '%s'.\n", m_pUserProps->pszSmokeParticleName );
		goto _ExitWithError;
	}
	
	m_fHeatupPerSecond			= m_pUserProps->fHeatUpRate;
	m_fCooldownPerSecond		= m_pUserProps->fCoolDownRate;			
	m_fCooldownOverheatPenalty	= m_pUserProps->fOverheatPenalty;


	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}




BOOL CWeaponChaingun::ClassHierarchyBuilt( void ) {
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



void CWeaponChaingun::ClassHierarchyDestroy( void ) {
	fforce_Kill( &m_hForce );

	for( u32 i=0; i<EUK_COUNT_CHAINGUN; i++ ) {
		DestroySharedEUKData( i );
	}

	if( m_pSmokeParticle ) {
		fdelete( m_pSmokeParticle );
		m_pSmokeParticle = NULL;
	}

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


void CWeaponChaingun::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pWorldMesh->UpdateTracker();

	m_pSmokeParticle->AddToWorld();
	m_pSmokeParticle->Attach_UnitMtxToParent_PS( this, m_pUserProps->apszBoneName[_BONE_PRIMARY_FIRE] );

	m_fUnitHeat = 0.0f;
}


void CWeaponChaingun::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();
	m_pSmokeParticle->RemoveFromWorld();

	// kill the sound if necessary
	if( m_pSndEmitter ) {
		m_pSndEmitter->Destroy();
		m_pSndEmitter = NULL;
	}
}


void CWeaponChaingun::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

//	if( IsInWorld() ) 
	{
		u32 i;

		if( bDrawingHasBeenEnabled ) {
			// Enable drawing of this weapon...

			for( i=0; i<EUK_COUNT_CHAINGUN; ++i ) {
				FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		} else {
			// Disable drawing of this weapon...
			for( i=0; i<EUK_COUNT_CHAINGUN; ++i ) {
				FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}
	}
}


void CWeaponChaingun::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
        m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();
		
		m_pCallbackThis = this;
		m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );
		RelocateAllChildren();
		m_pCallbackThis = NULL;

		if( m_pSndEmitter ) {
			// don't really like this, but doesn't seem to be a way to determine whether it's a 2d/3d sound, will have to add that
			if( !IsOwnedByPlayer() ) {
				m_pSndEmitter->SetPosition( &m_MtxToWorld.m_vPos );
			}
		}
	}
}


void CWeaponChaingun::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;
}


CEntityBuilder *CWeaponChaingun::GetLeafClassBuilder( void ) {
	return &_WeaponChaingunBuilder;
}



void CWeaponChaingun::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_CHAINGUN; ++i ) {
		m_aResourceData[i].m_pAnimCombiner = NULL;
		m_aResourceData[i].m_pAnimRest = NULL;
		m_aResourceData[i].m_pWorldMesh = NULL;
	}

	m_eState					= _STATE_IDLE;
	m_pResourceData				= NULL;
	m_pUserProps				= NULL;
	m_bTriggerDown				= FALSE;
	m_fBarrelSpeed				= 0.0f;
	m_fFireTimer				= 0.0f;
	m_fUnitShotSpread			= 0.0f;
	m_fDebrisTimer				= 0.0f;
	m_fBarrelRotation			= 0.0f;
	m_fCartridgeRotation		= 0.0f;
	m_fSpinDirectionModifier	= 1.0f;
	m_fUnitHeat					= 0.0f;
	m_fSoundFireTimer			= 0.0f;
	m_pSndEmitter				= NULL;

	m_pSoundMuzzlePosOverride	= NULL;
	m_pLightMuzzlePosOverride	= NULL;
	m_bPlaySounds				= TRUE;
	m_bDrawMuzzleLights			= TRUE;
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponChaingun::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();
}


CFMtx43A *CWeaponChaingun::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponChaingun::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponChaingun::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();
	f32 fVal;

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	switch( m_eState ) {
		case _STATE_OFF:
			m_fBarrelSpeed -=  m_pUserProps->fOOSpindownTime * FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0( m_fBarrelSpeed );

			m_fUnitHeat -= m_fCooldownPerSecond * FLoop_fPreviousLoopSecs;

			if( !(m_nWeaponFlags & WEAPONFLAG_WEAPON_DISABLED) ) {
				m_eState = _STATE_IDLE;
			}
			break;

		case _STATE_IDLE:
			m_fBarrelSpeed	= m_pUserProps->fBarrelIdleRPS;

			m_fUnitHeat	   -= m_fCooldownPerSecond * FLoop_fPreviousLoopSecs;
			if( m_bTriggerDown ) {
				m_eState = _STATE_SPINUP;
				
				_PlaySoundForNewState( _STATE_SPINUP );
			}
			break;

		case _STATE_SPINUP:
			m_fBarrelSpeed += (m_pUserProps->fBarrelsMaxRevsPerSec - m_pUserProps->fBarrelIdleRPS) * m_pUserProps->fOOSpinupTime * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fBarrelSpeed, m_pUserProps->fBarrelsMaxRevsPerSec );

			m_fUnitHeat -= m_fCooldownPerSecond * FLoop_fPreviousLoopSecs;
			if( m_fBarrelSpeed == m_pUserProps->fBarrelsMaxRevsPerSec ) {
				if( m_bTriggerDown ) {
					m_eState = _STATE_FIRING;
					_PlaySoundForNewState( _STATE_FIRING );
				} else {
					m_eState = _STATE_SPINDOWN;
					_PlaySoundForNewState( _STATE_SPINDOWN );
				}
			}
			break;

		case _STATE_SPINDOWN:
			m_fBarrelSpeed -= (m_pUserProps->fBarrelsMaxRevsPerSec - m_pUserProps->fBarrelIdleRPS) * m_pUserProps->fOOSpindownTime * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fBarrelSpeed, m_pUserProps->fBarrelIdleRPS );
			
			m_fUnitHeat -= m_fCooldownPerSecond * FLoop_fPreviousLoopSecs;

			if( m_fBarrelSpeed == m_pUserProps->fBarrelIdleRPS ) {
				m_eState = _STATE_IDLE;
			}
			break;

		case _STATE_FIRING:
			if( m_bTriggerDown ) {
				m_fBarrelSpeed = m_pUserProps->fBarrelsMaxRevsPerSec;
				m_fUnitHeat   += m_fHeatupPerSecond * FLoop_fPreviousLoopSecs;
				_TryToFire();
			} else {
				m_eState = _STATE_SPINDOWN;
				_PlaySoundForNewState( _STATE_SPINDOWN );
			}
			break;

		case _STATE_OVERHEAT_FORCED_COOLDOWN:
			m_fBarrelSpeed	= 0.0f;
			m_fUnitHeat	   -= m_fCooldownPerSecond * FLoop_fPreviousLoopSecs * m_fCooldownOverheatPenalty;

			if( m_fUnitHeat < m_pUserProps->fCoolDownThreshold ) {
				m_eState = _STATE_IDLE;
			}
			break;
	}


	// spin stuff that needs spinning
	m_fBarrelRotation += m_fBarrelSpeed * FMATH_2PI * FLoop_fPreviousLoopSecs * m_fSpinDirectionModifier;

	// if we're firing, rotate the cartridge, it just has a fixed rate
	if( m_eState == _STATE_FIRING ) {
		m_fCartridgeRotation += m_pUserProps->fCartridgeRPS * FMATH_2PI * FLoop_fPreviousLoopSecs * m_fSpinDirectionModifier;
	}

	// make sure we're 0< <2PI
	while( m_fBarrelRotation > FMATH_2PI ) {
		m_fBarrelRotation -= FMATH_2PI;
	}
	while( m_fBarrelRotation < 0.0f ) {
		m_fBarrelRotation += FMATH_2PI;
	}

	while( m_fCartridgeRotation > FMATH_2PI ) {
		m_fCartridgeRotation -= FMATH_2PI;
	}
	
	while( m_fCartridgeRotation < 0.0f ) {
		m_fCartridgeRotation += FMATH_2PI;
	}


	// deal with heat
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitHeat );
	if( m_fUnitHeat == 1.0f ) {
		m_eState = _STATE_OVERHEAT_FORCED_COOLDOWN;
		_PlaySoundForNewState( _STATE_OVERHEAT_FORCED_COOLDOWN );
	}

	// handle barrel heat visual effect
	fVal = m_fUnitHeat * 3.0f;
	FMATH_CLAMP_MAX1( fVal );
	m_pResourceData->m_pWorldMesh->LayerAlpha_Set( m_hHeatTexLayerBarrel, fVal );
	m_pResourceData->m_pWorldMesh->LayerAlpha_Set( m_hHeatTexLayerFront, fVal * 0.5f );

	// handle smoke
	if( m_fUnitHeat > _BARREL_SMOKE_THRESHOLD ) {
		if( !m_pSmokeParticle->IsEmitting() ) {
			m_pSmokeParticle->StartEmission( m_hSmokeParticleDef, 1.0f );
		}
	} else {
		if( m_pSmokeParticle->IsEmitting() ) {
			m_pSmokeParticle->StopEmission();
		}
	}

	//make sure the weapon isn't disabled
	if( (m_nWeaponFlags & WEAPONFLAG_WEAPON_DISABLED) ) {
		m_eState = _STATE_OFF;
	}
}



void CWeaponChaingun::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


u32 CWeaponChaingun::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS ) {
	m_bTriggerDown = (fUnitTriggerVal1 > 0.25f);
    	
	return (m_eState == _STATE_FIRING);
}


void CWeaponChaingun::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	pMuzzlePoint_WS->Mul( m_MtxToWorld.m_vFront, m_pUserProps->fDistFromWeaponOrigToMuzzle).Add( m_MtxToWorld.m_vPos );
}


void CWeaponChaingun::OverrideLights( BOOL bShowLight, BOOL bOverridePos/*=FALSE*/, const CFVec3A *pPosition/*=NULL*/ ) {
	if( (bShowLight != m_bDrawMuzzleLights) || (bOverridePos != m_bOverrideMuzzleLightPos) ) {
		m_bDrawMuzzleLights			= bShowLight;
		m_bOverrideMuzzleLightPos	= bOverridePos;
		if( m_bOverrideMuzzleLightPos ) {
			FASSERT( pPosition != NULL );
			m_pLightMuzzlePosOverride	= pPosition;
		}
	}
}


void CWeaponChaingun::OverrideSounds( BOOL bPlaySounds, BOOL bOverridePos/*=FALSE*/, const CFVec3A *pPosition/*=NULL*/ ) {
	if( (bPlaySounds != m_bPlaySounds) || (bOverridePos != m_bOverrideSoundPos) ) {
		m_bPlaySounds		= bPlaySounds;
		m_bOverrideSoundPos = bOverridePos;
		if( m_bOverrideSoundPos ) {
			FASSERT( pPosition != NULL );
			m_pLightMuzzlePosOverride	= pPosition;
		}
	}
}

void CWeaponChaingun::RestoreDefaultValues( void ) {
	m_fHeatupPerSecond			= m_pUserProps->fHeatUpRate;
	m_fCooldownPerSecond		= m_pUserProps->fCoolDownRate;
	m_fCooldownOverheatPenalty	= m_pUserProps->fOverheatPenalty;
}


void CWeaponChaingun::_TryToFire( void ) {
	f32 fJitter;

	m_fFireTimer -= FLoop_fPreviousLoopSecs;
	
	if( m_fFireTimer >= 0.0f ) {		//not firing this frame
		m_fUnitShotSpread -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_MIN0( m_fUnitShotSpread );
		return;	
	}

	//we're firing
	m_fFireTimer = m_pUserProps->fOORoundsPerSec;

	fJitter = FMATH_FPOT( m_fUnitShotSpread, m_pUserProps->fBestShotSpreadFactor, m_pUserProps->fWorstShotSpreadFactor );

	//now increment spread,
	m_fUnitShotSpread += FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_MAX1( m_fUnitShotSpread );

	CFVec3A MuzzlePoint;
	CFVec3A vecPerturb;

	vecPerturb.Set( fmath_RandomFloatRange( -fJitter, fJitter ),
					fmath_RandomFloatRange( -fJitter, fJitter ),
					fmath_RandomFloatRange( -fJitter, fJitter ) );

	// chainguns fire right down the barrel now...
	m_FireUnitDir.Set( m_MtxToWorld.m_vFront );

	m_FireUnitDir.Add( vecPerturb );
	m_FireUnitDir.Unitize();

	ComputeMuzzlePoint_WS( &MuzzlePoint );

	//do the muzzleflash thing
	if( IsDrawEnabled() ) {
		if( fmath_RandomChance( 0.75f ) ) {
			
			f32 fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );
			CMuzzleFlash::AddFlash_CardPosterZ(
				m_ahMuzzleFlashGroup[ MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
				MuzzlePoint,
				m_FireUnitDir,
				m_pUserProps->fMuzzleWidth_Z  * fRandomScale,
				m_pUserProps->fMuzzleHeight_Z * fRandomScale,
				m_pUserProps->fMuzzleAlpha_Z
			);

			CMuzzleFlash::AddFlash_CardPosterXY(
				m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
				MuzzlePoint,
				m_pUserProps->fMuzzleScale_XY * fRandomScale,
				m_pUserProps->fMuzzleAlpha_XY
			);

			m_FireUnitDir.Unitize();

			CMuzzleFlash::AddFlash_Mesh3D(
				m_ahMuzzleFlashGroup[ MUZZLEFLASH_TYPE_3D_STAR_1 + fmath_RandomRange( 0, 1 ) ],
				MuzzlePoint,
				m_FireUnitDir,
				m_pUserProps->fMuzzleScale_Mesh * fRandomScale,
				m_pUserProps->fMuzzleRot_Mesh + fmath_RandomFloatRange( 0, 2 * FMATH_PI )
			);

			// don't go crazy w/ the shells
			if( fmath_RandomChance( 0.5f ) ) {
				_EjectShell();
			}

			if( m_bDrawMuzzleLights ) {
				
				if( m_bOverrideMuzzleLightPos ) {
					CMuzzleFlash::MuzzleLight( m_pLightMuzzlePosOverride, m_pUserProps->fMuzzleLightRadius );
				} else {
					CMuzzleFlash::MuzzleLight( &MuzzlePoint, m_pUserProps->fMuzzleLightRadius );
				}
			}
			//do some force feedback work
			if( IsOwnedByPlayer()) {
				fforce_Kill( &m_hForce );
				fforce_Play( Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce );
			}
		}
	}


	//handle the actual projectile
	CFVec3A RayEnd;
	FCollImpact_t CollImpact;

	RayEnd.Mul( m_FireUnitDir, m_pUserProps->fMaxLiveRange ).Add( MuzzlePoint );

	FWorld_nTrackerSkipListCount = 0;
	m_pOwnerBot->AppendTrackerSkipList();
	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &MuzzlePoint, &RayEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, m_pResourceData->m_pWorldMesh, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
		
		// Hit something...
		RayEnd = CollImpact.ImpactPoint;

		CFVec3A ImpactFlashPos;
		if( fmath_RandomChance( 0.5 ) ) {
			CGColl::GetMaterial( CollImpact.nUserType )->DrawAllDrawableParticles( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, FALSE,
																				   fmath_RandomFloatRange( 0.5f, 0.8f ),
																				   fmath_RandomFloatRange( 0.5f, 0.8f ),
																				   fmath_RandomFloatRange( 0.5f, 0.8f ) );
			if( m_fDebrisTimer <= 0.0f ) {
//				CDebris::SpawnDefaultDebris( CDebris::DEBRIS_TYPE_CEMENT, &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.9f, 6, 3 );
				m_fDebrisTimer = fmath_RandomFloatRange( 0.5f, 1.0f );
			}
			m_fDebrisTimer -= FLoop_fPreviousLoopSecs;

			ImpactFlashPos.Mul( CollImpact.UnitFaceNormal, m_pUserProps->fMuzzleOffset_Impact ).Add( CollImpact.ImpactPoint );

			CMuzzleFlash::AddFlash_CardPosterXY( m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
												 ImpactFlashPos, m_pUserProps->fMuzzleScale_Impact,
												 m_pUserProps->fMuzzleAlpha_Impact );
		}

		

		//if( CollImpact.pTag == NULL ) {
		//	//Hit world geo...
		//	if( fmath_RandomChance( 0.5f ) ) {
		//		potmark_NewPotmark( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.75f, POTMARKTYPE_BULLET );
		//	}
		//} else {
			// Hit an object...

		if( fmath_RandomChance( 0.25f ) ) {
			CFDecal::Create( m_pUserProps->hDecalDef, &CollImpact, &m_FireUnitDir );
		}

		if( CollImpact.pTag != NULL ) {
			CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;

			// See if the world mesh that we found is an entity...
			FASSERT( pWorldMesh->GetType() == FWORLD_TRACKERTYPE_MESH );		// coll fn only returns mesh tracker types
			if( pWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
					//It's an Entity!

				CEntity *pDamagee = (CEntity *)pWorldMesh->m_pUser;

				//if( !(pDamagee->TypeBits() & ENTITY_BIT_BOT) || !(pDamagee->TypeBits() & ENTITY_BIT_SHIELD) ) {
				//	potmark_NewPotmark( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.2f, POTMARKTYPE_BULLET );
				//}

				// Get an empty damage form...
				CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

				if( pDamageForm )
				{
					// Fill out the form...

					pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_IMPACT;
					pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
					pDamageForm->m_pDamageProfile	= m_apDamageProfile[m_nUpgradeLevel];
					pDamageForm->m_Damager.pWeapon	= this;
					pDamageForm->m_Damager.pBot		= GetOwner();
					pDamageForm->m_Damager.nDamagerPlayerIndex = GetOwner() ? GetOwner()->m_nPossessionPlayerIndex : -1;
					pDamageForm->m_pDamageeEntity	= pDamagee;

					pDamageForm->InitTriDataFromCollImpact( pWorldMesh, &CollImpact, &m_FireUnitDir );

					CDamage::SubmitDamageForm( pDamageForm );
				}
			}
		}

		// Make an AI impact sound in case any AI are listening...
		if( GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1 && m_fSecsUntilNextSound <= 0.0f ) {
			AIEnviro_AddSound( CollImpact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, this );
			m_fSecsUntilNextSound = 0.5f;
		}

		if( fmath_RandomChance( 0.5f ) ) {
			// Make a tracer...

			f32 fTracerLen = MuzzlePoint.Dist( RayEnd );

			CMuzzleFlash::AddFlash_CardPosterZ( m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1], MuzzlePoint, m_FireUnitDir,
												0.25f, fTracerLen, 0.2f	);
		}
	}
}

void CWeaponChaingun::_SpinBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	if( nBoneIndex == m_pCallbackThis->m_auSpinBarrelBone[m_pCallbackThis->m_nUpgradeLevel] ) {
		CFMtx43A::m_RotZ.SetRotationZ( m_pCallbackThis->m_fBarrelRotation );
	} else {
		CFMtx43A::m_RotZ.SetRotationZ( m_pCallbackThis->m_fCartridgeRotation );
	}

	rNewMtx.Mul( rParentMtx, rBoneMtx );
	rNewMtx.Mul( CFMtx43A::m_RotZ );
}


void CWeaponChaingun::_PlaySoundForNewState( _ChaingunState_e eNewState ) {
	BOOL bPlay2D = (IsOwnedByPlayer());
	const CFVec3A *pSndPos;

	if( !m_bPlaySounds ) {
		if( m_pSndEmitter ) {
			m_pSndEmitter->Destroy();
			m_pSndEmitter = NULL;
		}
		return;
	}

	if( m_bOverrideSoundPos ) {
		pSndPos = m_pSoundMuzzlePosOverride;
	} else {
		pSndPos = &m_MtxToWorld.m_vPos;
	}
	
	switch( eNewState ) {
		case _STATE_SPINUP:
			if( bPlay2D ) {
				fsndfx_Play2D( m_pUserProps->hSndSpinup );
			} else {
				fsndfx_Play3D( m_pUserProps->hSndSpinup, pSndPos, m_pUserProps->fSoundRadius );
			}
			break;

		case _STATE_SPINDOWN:
			//kill looping spin sound
			if( m_pSndEmitter ) {
				m_pSndEmitter->Destroy();
				m_pSndEmitter = NULL;
			}

			if( bPlay2D ) {
				fsndfx_Play2D( m_pUserProps->hSndSpindown );
			} else {
				fsndfx_Play3D( m_pUserProps->hSndSpindown, pSndPos, m_pUserProps->fSoundRadius );
			}
			break;

		case _STATE_OVERHEAT_FORCED_COOLDOWN:
			//kill looping spin sound
			if( m_pSndEmitter ) {
				m_pSndEmitter->Destroy();
				m_pSndEmitter = NULL;
			}

			if( bPlay2D ) {
				fsndfx_Play2D( m_pUserProps->hSndOverheat );
			} else {
				fsndfx_Play3D( m_pUserProps->hSndOverheat, pSndPos, m_pUserProps->fSoundRadius );
			}
			break;

		case _STATE_FIRING:
			FASSERT( m_pSndEmitter == NULL );

			if( bPlay2D ) {
				m_pSndEmitter = FSNDFX_ALLOCNPLAY2D( m_pUserProps->hSndSpin, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
			} else {
				m_pSndEmitter = FSNDFX_ALLOCNPLAY3D( m_pUserProps->hSndSpin, pSndPos, m_pUserProps->fSoundRadius, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
			}
			break;
	}
}


void CWeaponChaingun::_EjectShell( void ) {
#if 0
	CFVec3A vTmp;
	CFVec3A vVel;

	m_DebrisInitSettings.fShrinkPerSec = fmath_RandomFloatRange( 1.0f, 10.0f );
	
	vTmp = MtxToWorld()->m_vUp;
	vTmp.Mul( fmath_RandomFloatRange( 3.0f, 5.0f ) );
	vVel = MtxToWorld()->m_vRight;
	vVel.Mul( fmath_RandomFloatRange( 7.0f, 12.0f ) );
	vVel.Mul( m_fSpinDirectionModifier );
	vVel.Add( vTmp );
	vVel.Add( m_pOwnerBot->m_Velocity_WS  );

	vTmp = MtxToWorld()->m_vFront;
	vTmp.Mul( 2.0f );
	vTmp.Add( MtxToWorld()->m_vPos );

	CDebris::CreateDebris( &vTmp, &vVel, &m_apShellMesh[m_nUpgradeLevel], 1, 1.0f, &m_DebrisInitSettings );
#endif
}



































