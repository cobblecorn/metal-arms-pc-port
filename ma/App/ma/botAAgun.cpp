//////////////////////////////////////////////////////////////////////////////////////
// botAAgun.cpp -
//
// Author: Michael Starich
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 3/14/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "botAAgun.h"
#include "fanim.h"
#include "fresload.h"
#include "floop.h"
#include "gamepad.h"
#include "ftimer.h"
#include "ftext.h"
#include "ItemInst.h"
#include "ItemRepository.h"
#include "fforce.h"
#include "meshtypes.h"
#include "weapons.h"
#include "reticle.h"
#include "player.h"
#include "level.h"
#include "ProTrack.h"
#include "fsndfx.h"
#include "meshentity.h"
#include "Ai\AIEnviro.h"
#include "Ai\aigameutils.h"
#include "Hud2.h"
#include "eparticlepool.h"
#include "gcoll.h"
#include "gamecam.h"
#include "potmark.h"
#include "eproj_mortar.h"
#include "fxshockwave.h"

static cchar *_pszBotInfoFilename = "b_AAGun";
static cchar *_apszAAGunMeshFilenames[] = {
	"GOM_AGun00"
};
static cchar *_pszSoundEffectBank = "AAGun";

static cchar *_pszProjectileMeshName = "GRMMshell01";

// 1 / secs
#define _USERPROPS_CAMRATE_IN			( 1.0f )
#define _USERPROPS_CAMRATE_OUT			( 1.0f )

// a way to speed up debug builds
#if FANG_DEBUG_BUILD
	#define _EFFECTS_ON				TRUE//FALSE
#else
	#define _EFFECTS_ON				TRUE
#endif

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotAAGunBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
static CBotAAGunBuilder *_pBotAAGunBuilder;


void CBotAAGunBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTAAGUN, pszEntityType );

	// Override defaults set by our parent classes...
	m_bFull360 = FALSE;
	m_fMaxRightHeading = 0.0f;// zero means use the csv data
	m_fMaxDownPitch = 0.0f;// zero means use the csv data
	m_bMorterAttachment = FALSE;
	m_fHeadingPerSec = 0.0f;// zero means use the csv data
	m_fPitchPerSec = 0.0f;// zero means use the csv data
}

BOOL CBotAAGunBuilder::PostInterpretFixup( void ) {

	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}

BOOL CBotAAGunBuilder::InterpretTable( void ) {
	f32 fValue;

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "AAGun_Full360" ) ) {
		CEntityParser::Interpret_BOOL( &m_bFull360 ); 

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AAGun_MaxHeading" ) ) {
		CEntityParser::Interpret_F32( &fValue, 0.0f, 180.0f, TRUE );
		m_fMaxRightHeading = FMATH_DEG2RAD( fValue );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AAGun_MaxPitch" ) ) {
		CEntityParser::Interpret_F32( &fValue, 0.0f, FMATH_PI, TRUE );
		m_fMaxDownPitch = FMATH_DEG2RAD( fValue );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AAGun_Morter" ) ) {
		CEntityParser::Interpret_BOOL( &m_bMorterAttachment ); 

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AAGun_PitchPerSec" ) ) {
		CEntityParser::Interpret_F32( &fValue );
		m_fPitchPerSec = FMATH_DEG2RAD( fValue );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AAGun_HeadingPerSec" ) ) {
		CEntityParser::Interpret_F32( &fValue );
		m_fHeadingPerSec = FMATH_DEG2RAD( fValue );

		return TRUE;
	}

	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotAAGun
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

// static AA Gun Vars:
BOOL CBotAAGun::m_bSystemInitialized = FALSE;
u32 CBotAAGun::m_nBotClassClientCount = 0;
CBotAAGun::BotInfo_Gen_t CBotAAGun::m_BotInfo_Gen;
CBotAAGun::BotInfo_MountAim_t CBotAAGun::m_BotInfo_MountAim;
CBotAAGun::BotInfo_Weapon_t CBotAAGun::m_BotInfo_Weapon;
CBotAnimStackDef CBotAAGun::m_AnimStackDef;
f32 CBotAAGun::m_fSecsBackward;
f32 CBotAAGun::m_fOOSecsBackward;
f32 CBotAAGun::m_fSecsForward;
f32 CBotAAGun::m_fOOSecsForward;
f32 CBotAAGun::m_fSecsBetweenShots;
CBotAAGun::BotInfo_AAGun_t CBotAAGun::m_BotInfo_AAGun;
CBotAAGun::MortarInfo_t CBotAAGun::m_MortarInfo;
CEProjPool::PoolHandle_t CBotAAGun::m_hMortarProjPool;

BOOL CBotAAGun::InitSystem( void ) {
	
	FASSERT( !m_bSystemInitialized );

	m_nBotClassClientCount = 0;
	
	_pBotAAGunBuilder = fnew CBotAAGunBuilder;
	if( !_pBotAAGunBuilder ) {
		return FALSE;
	}

	m_bSystemInitialized = TRUE;

	return TRUE;
}

void CBotAAGun::UninitSystem( void ) {

	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
		FASSERT( _pBotAAGunBuilder );
		fdelete( _pBotAAGunBuilder );
		_pBotAAGunBuilder = NULL;
	}
}

BOOL CBotAAGun::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	++m_nBotClassClientCount;

	if( !CBot::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...
		return FALSE;
	}

	if( m_nBotClassClientCount > 1 ) {
		// Resources already loaded...
		return TRUE;
	}

	// Resources not yet loaded...
	FResFrame_t ResFrame = fres_GetFrame();

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _pszSoundEffectBank) ) {
		DEVPRINTF( "CBotAAGun::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _pszSoundEffectBank);
	}
	
	//default for the CSV defaults
	
	if( !ReadBotInfoFile( m_aGameDataMap, _pszBotInfoFilename ) ) {
		goto _ExitInitSystemWithError;
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitInitSystemWithError;
	}

	// compute some values from our csv data
	m_fSecsBackward = (m_BotInfo_AAGun.fSecsBetweenBarrelFires * m_BotInfo_AAGun.fPercentRecoilBackwards);
	m_fOOSecsBackward = (1.0f/m_fSecsBackward);
	m_fSecsForward = (m_BotInfo_AAGun.fSecsBetweenBarrelFires - m_fSecsBackward);
	m_fOOSecsForward = (1.0f/m_fSecsForward);
	m_fSecsBetweenShots = (m_BotInfo_AAGun.fSecsBetweenBarrelFires/CBotAAGun::NUM_BARRELS);


	if( !CEProj_Mortar::LoadSharedResources() ) {
		DEVPRINTF( "CBotAAGun::ClassHierarchyLoadSharedResources(): could not init eproj system failed.\n" );
		goto _ExitInitSystemWithError;
	}

	// Create a pool of projectiles 
	m_hMortarProjPool = CEProjPool::Create( CEProj::PROJTYPE_MORTAR,
		_pszProjectileMeshName,
		m_MortarInfo.uPoolCannonsBalls,
		FMESHINST_FLAG_NOSHADOWLOD |FMESHINST_FLAG_CAST_SHADOWS );
	if( m_hMortarProjPool == EPROJPOOL_NULL_HANDLE ) {
		DEVPRINTF( "CBotAAGun::Init(): Could not create projectile pool.\n" );
		goto _ExitInitSystemWithError;
	}

	return TRUE;

	// Error:
_ExitInitSystemWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}

void CBotAAGun::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) 
	{
		return;
	}
	
	m_AnimStackDef.Destroy();
	CEProj_Mortar::UnloadSharedResources();
	m_hMortarProjPool = EPROJPOOL_NULL_HANDLE; 

	CBot::ClassHierarchyUnloadSharedResources();
}

CBotAAGun::CBotAAGun() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
	m_pRotSoundEmitter = NULL;
}

CBotAAGun::~CBotAAGun() {

	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

BOOL CBotAAGun::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName,
					   const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotAAGunBuilder *pBuilder = (CBotAAGunBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == _pBotAAGunBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTAAGUN );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}

void CBotAAGun::ClassHierarchyDestroy( void ) { 

	fforce_Kill( &m_hForce );
	
	// Delete the items that we had instantiated for us...
	m_Anim.Destroy();
	m_MeshRestAnimSource.Destroy();
	m_AnimManMtxAim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	CBot::ClassHierarchyDestroy();
}

BOOL CBotAAGun::ClassHierarchyBuild( void ) {
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	u32 i;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotAAGunBuilder *pBuilder = (CBotAAGunBuilder *)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	m_bLimitHeading = !pBuilder->m_bFull360;
	if( pBuilder->m_fMaxRightHeading == 0.0f ) {
		// use csv values
		m_fMaxHeading = m_BotInfo_AAGun.fMaxHeading;
		m_fMinHeading = m_BotInfo_AAGun.fMinHeading;
	} else {
		// use the builder values
		m_fMaxHeading = pBuilder->m_fMaxRightHeading;
		m_fMinHeading = -pBuilder->m_fMaxRightHeading;
	}
	if( pBuilder->m_fMaxDownPitch == 0.0f ) {
		// use the csv values
		m_fMinPitch = m_pBotInfo_MountAim->fMountPitchUpLimit;
		m_fMaxPitch = m_pBotInfo_MountAim->fMountPitchDownLimit;
	} else {
		m_fMinPitch = -pBuilder->m_fMaxDownPitch;
		m_fMaxPitch = pBuilder->m_fMaxDownPitch;
	}
	m_fHeadingPerSec = (pBuilder->m_fHeadingPerSec == 0.0f) ? m_BotInfo_AAGun.fHeadingPerSec : pBuilder->m_fHeadingPerSec;
	m_fPitchPerSec = (pBuilder->m_fPitchPerSec == 0.0f) ? m_BotInfo_AAGun.fPitchPerSec : pBuilder->m_fPitchPerSec;
	m_bMorterAttachment = pBuilder->m_bMorterAttachment;

	// Load mesh resource...
	pszMeshFilename = _apszAAGunMeshFilenames[0];
	if( pBuilder->m_uMeshVersionOverride > 0 &&
		pBuilder->m_uMeshVersionOverride < sizeof(_apszAAGunMeshFilenames)/sizeof(char*) ) {
		pszMeshFilename = _apszAAGunMeshFilenames[pBuilder->m_uMeshVersionOverride];
	}
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotAAGun::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotAAGun::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the world mesh...
	MeshInit.pMesh = pMesh;
	MeshInit.nFlags = 0;
	MeshInit.fCullDist = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	m_pWorldMesh->Init( &MeshInit );

	m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser = this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pWorldMesh->SetCollisionFlag( TRUE );
	m_pWorldMesh->SetLineOfSightFlag( FALSE );

	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotAAGun::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	// create the animation stack

	// first the "rest" animation
	if( !m_MeshRestAnimSource.Create( m_pWorldMesh->m_pMesh ) ) {
		DEVPRINTF( "CBotAAGun::ClassHierarchyCreate(): Could not create m_MeshRestAnimSource.\n" );
		goto _ExitWithError;
	}
    m_Anim.BaseAnim_Attach( ANIMTAP_REST, &m_MeshRestAnimSource );
	m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_REST, 1.0f );

	// next the "summer" animation
	if( !m_AnimManMtxAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotAAGun::ClassHierarchyCreate(): Could not create m_AnimManMtxAim.\n" );
		goto _ExitWithError;
	}
	m_Anim.BaseAnim_Attach( ANIMTAP_AIM_SUMMER, &m_AnimManMtxAim );
	m_AnimManMtxAim.Reset();
	m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_AIM_SUMMER, 1.0f );
	
	SetMaxHealth();
	SetActionable( TRUE );

	// find all of our bone indices
	for( i=0; i < BONE_COUNT; i++ ) {
        m_anBoneIndices[i] = m_pWorldMesh->FindBone( m_apszBoneNameTable[i] );
		if( m_anBoneIndices[i] < 0 ) {
			DEVPRINTF( "CBotAAGun::ClassHierarchyCreate(): Could not find bone '%s' in the mesh.\n", m_apszBoneNameTable[i] );
			goto _ExitWithError;
		}
	}

	// create our mortar secondary gun
	if( m_bMorterAttachment ) {

	}

	m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_VERTICAL] ]->m_vPos;
	m_GazeUnitVec_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_VERTICAL] ]->m_vFront;

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	fforce_NullHandle( &m_hForce );

	EnableCameraCollision( TRUE );
	SetBotFlag_MeshCollideOnly();

	_ResetToNeutral();	

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotAAGun::_ResetToNeutral( void ) {
	u32 i;

	m_fGunHeading = 0.0f;
	m_fGunPitch = 0.0f;

	m_fGunUnitShotSpread = 0.0f;
	m_fGunAISoundTimer = 0.0f;
	m_fGunTracerTimer = 0.0f;

	m_fSecsTillNextFire = m_fSecsBetweenShots;
	m_nNextFireBarrelIndex = 0;
	for( i=0; i < NUM_BARRELS; i++ ) {
        m_afBarrelRecoilZ[i] = 0.0f;
		m_afOuterBarrelRecoilZ[i] = 0.0f;
        m_afBarrelTimer[i] = m_BotInfo_AAGun.fSecsBetweenBarrelFires;		
	}

	m_pDriverBot = NULL;
	m_eStationStatus = CVehicle::STATION_STATUS_EMPTY;	
	m_fEntryJumpTimer = 0.0f;

	m_fUnitCameraTransition = 0.0f;
	m_eCameraState = CAMERA_STATE_INACTIVE;
    m_pCameraBot = NULL;

	for( i=0; i < NUM_BARRELS; i++ ) {
		m_ahEmitters[i] = FPARTICLE_INVALID_HANDLE;
	}

	m_fSecsTillNextMortarFire = m_MortarInfo.fOORoundsPerSec;
	m_fSecsTillNextFlakFx = 0.0f;
	m_fSecsTillNextMortarClick = 0.0f;

	m_pRotSoundEmitter = NULL;
	m_fRotVolume = 0.0f;
	m_bAllowDriverMovement = TRUE;
	m_bAllowDriverExit = TRUE;
	m_bDisplayEnterMsg = TRUE;
}

BOOL CBotAAGun::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	m_pWorldMesh->m_nFlags &= (~FMESHINST_FLAG_CAST_SHADOWS);

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

CEntityBuilder *CBotAAGun::GetLeafClassBuilder( void ) {
	return _pBotAAGunBuilder;
}

void CBotAAGun::_ClearDataMembers( void ) {
	// Init data members...
	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Weapon = &m_BotInfo_Weapon;

	m_pMoveIdentifier = &m_anBoneIndices[0];
	
	m_fGravity = m_pBotInfo_Gen->fGravity;
	
	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_pCameraBot = NULL;
	m_pDriverBot = NULL;
	m_fGunHeading = 0.0f;
	m_fGunPitch = 0.0f;
	m_bMorterAttachment = FALSE;

	m_pRotSoundEmitter = NULL;
	m_fRotVolume = 0.0f;
}

void CBotAAGun::ClassHierarchyAddToWorld( void ) {
	
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	//_UpdateMatrices();
	m_pWorldMesh->UpdateTracker();

	m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM | BOTDEATHFLAG_COMEAPART | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;	
}

void CBotAAGun::ClassHierarchyRemoveFromWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	SwiftKickOutDriver();

	fforce_Kill( &m_hForce );
	
	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			m_apWeapon[i]->RemoveFromWorld();
		}
	}

	if( m_pRotSoundEmitter ) {
		m_pRotSoundEmitter->Destroy();
		m_pRotSoundEmitter = NULL;
		m_fRotVolume = 0.0f;
	}

	m_pWorldMesh->RemoveFromWorld();
	CBot::ClassHierarchyRemoveFromWorld();
}

void CBotAAGun::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

void CBotAAGun::ClassHierarchyWork() {
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( gamecam_IsInDebugMode() && m_pDriverBot && !m_pDriverBot->IsWorkEnabled() ) {
		// we are in debug cam mode while the bot is driving us, don't do work
		return;
	}

	ParseControls();

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS = m_MountPos_WS;
	m_nPrevState = m_nState;

	// get a copy of our controls	
    f32 fControllerX, fControllerY;
	BOOL bFire1, bFire2;

	if( m_nPossessionPlayerIndex > -1 && 
		m_eCameraState == CAMERA_STATE_IN_VEHICLE && 
		m_bAllowDriverMovement ) {
		
		fControllerX = m_fControls_RotateCW;
		fControllerY = m_fControls_AimDown;
		bFire1 = (m_fControls_Fire1 > 0.1f);
		bFire2 = (m_fControls_Fire2 > 0.1f);
	} else {
		fControllerX = 0.0f;
		fControllerY = 0.0f;
		bFire1 = FALSE;
		bFire2 = FALSE;
	}	

	// move the barrels based on the x y controllers
	_YawPitchWork( fControllerX, fControllerY );

	_BarrelWork();

	_UpdateMatrices();	

	u32 i;
	CFMtx43A Barrel;
	for( i=0; i < NUM_BARRELS; i++ ) {
		if( m_ahEmitters[i] != FPARTICLE_INVALID_HANDLE ) {
			// update the location of the emitter
			Barrel = *m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_PRIMARY_FIRE1 + i] ];
			TempVec3A.Mul( Barrel.m_vFront, m_afBarrelRecoilZ[i] * 0.95f );
			Barrel.m_vPos.Add( TempVec3A );
			fparticle_TransformParticleIntoNewSpace( m_ahEmitters[i], Barrel );
		}
	}

	HandleTargeting();

	_PrimaryWeaponWork( bFire1 );
	_SecondaryWeaponWork( bFire2 );
	
	_HandleDriverState();
	
	_CameraTransitionWork();

	if( m_pDriverBot &&
		HasHumanOrBotControls() &&
		m_bControls_Action &&
		m_bAllowDriverExit ) {
		// time to exit the gun
		ActionNearby( m_pDriverBot );
	}

	_UpdateMountVars();
}

void CBotAAGun::_UpdateMountVars( void ) {
	CFMtx43A *pMount = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_VERTICAL] ];

	m_GazeUnitVec_WS = pMount->m_vFront;
	f32 fYaw_WS = fmath_Atan( m_GazeUnitVec_WS.x, m_GazeUnitVec_WS.z );
	ChangeMountYaw( fYaw_WS );
	//m_MountPos_WS = pMount->m_vPos;
	m_MountPos_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_BASE] ]->m_vPos;

	f32 fCos = m_GazeUnitVec_WS.y;
	f32 fSin = fmath_Sqrt( 1.0f - fCos*fCos );
	m_fMountPitch_WS = fmath_Atan( fSin, fCos ) - FMATH_HALF_PI;
}

void CBotAAGun::_ParticleEmitterCallback( FParticle_EmitterHandle_t hHandle, void *pUserData ) {
	FParticle_EmitterHandle_t *phHandle = (FParticle_EmitterHandle_t *)pUserData;

	*phHandle = FPARTICLE_INVALID_HANDLE;
}

void CBotAAGun::_YawPitchWork( f32 fControllerX, f32 fControllerY ) {
	f32 fDeltaAngle, fOldAngle;
	BOOL bAngleChanged = FALSE;

    // based off the controls, compute the heading
	fOldAngle = m_fGunHeading;
	fDeltaAngle = fControllerX * (m_fHeadingPerSec * FLoop_fPreviousLoopSecs);
	m_fGunHeading += fDeltaAngle;
	if( m_bLimitHeading ) {
		if( m_fGunHeading > m_fMaxHeading ) {
			m_fGunHeading = m_fMaxHeading;
		} else if( m_fGunHeading < m_fMinHeading ) {
			m_fGunHeading = m_fMinHeading;
		}
	}
	if( fOldAngle != m_fGunHeading ) {
		bAngleChanged = TRUE;
	}
	
	// based off the controls, compute the pitch
	fOldAngle = m_fGunPitch;
	fDeltaAngle = fControllerY * (m_fPitchPerSec * FLoop_fPreviousLoopSecs);
	m_fGunPitch += fDeltaAngle;
	if( m_fGunPitch > m_fMaxPitch ) {
		m_fGunPitch = m_fMaxPitch;
	} else if( m_fGunPitch < m_fMinPitch ) {
		m_fGunPitch = m_fMinPitch;
	}
	if( fOldAngle != m_fGunPitch ) {
		bAngleChanged = TRUE;		
	}

	if( bAngleChanged ) {
		// since we moved don't wait so long before spawning another flak burst
		m_fSecsTillNextFlakFx *= 0.9f;

		if( m_pRotSoundEmitter == NULL ) {
			m_pRotSoundEmitter = AllocAndPlaySound( m_BotInfo_AAGun.pRotateTuret );
		}

		if( m_pRotSoundEmitter != NULL ) {
			f32 fMaxUnitChange;
			fMaxUnitChange = FMATH_MAX( FMATH_FABS( fControllerY ), FMATH_FABS( fControllerX ) );

			m_fRotVolume += ((fMaxUnitChange - m_fRotVolume) * FLoop_fPreviousLoopSecs);
			FMATH_CLAMP( m_fRotVolume, 0.20f, 0.55f );
			m_pRotSoundEmitter->SetVolume( m_fRotVolume );
		}
	} else {
		if( m_pRotSoundEmitter ) {
			// quickly animate the volume to near 0 and then turn off
			m_fRotVolume -= (1.0f/0.5f) * FLoop_fPreviousLoopSecs;
			if( m_fRotVolume <= 0.03f ) {
				m_fRotVolume = 0.0f;
				m_pRotSoundEmitter->Destroy();
				m_pRotSoundEmitter = NULL;
			}
		}
	}
}

void CBotAAGun::_BarrelWork( void ) {
	f32 fUnitTime;
	u32 i;

	for( i=0; i < NUM_BARRELS; i++ ) {
		if( m_afBarrelTimer[i] < m_BotInfo_AAGun.fSecsBetweenBarrelFires ) {
			m_afBarrelTimer[i] += FLoop_fPreviousLoopSecs;

			// animate the barrel
			if( m_afBarrelTimer[i] <= m_fSecsBackward ) {
				// swiftly animate backwards
				fUnitTime = m_afBarrelTimer[i] * m_fOOSecsBackward;
				m_afBarrelRecoilZ[i] = fUnitTime * m_BotInfo_AAGun.fRecoilBack;
				if( fUnitTime >= 0.25f ) {
					m_afOuterBarrelRecoilZ[i] = ((fUnitTime - 0.25f) * (1.0f/0.75f)) * (m_BotInfo_AAGun.fRecoilBack * 0.35f);
				}

			} else if( m_afBarrelTimer[i] < m_BotInfo_AAGun.fSecsBetweenBarrelFires ) {
				// slowly animate back into position
				fUnitTime = (m_afBarrelTimer[i] - m_fSecsBackward) * m_fOOSecsForward;

				if( fUnitTime < 0.35f ) {
					m_afOuterBarrelRecoilZ[i] = (1.0f - (fUnitTime * (1.0f/0.35f))) * (m_BotInfo_AAGun.fRecoilBack * 0.35f);
				} else {
					m_afOuterBarrelRecoilZ[i] = 0.0f;
				}

				fUnitTime *= 0.5f;
				fUnitTime += 0.5f;
				m_afBarrelRecoilZ[i] = fmath_Inv( fUnitTime );
				m_afBarrelRecoilZ[i] -= 1.0f;
				m_afBarrelRecoilZ[i] *= m_BotInfo_AAGun.fRecoilBack;

			} else {
				// done animating the barrel
				m_afBarrelRecoilZ[i] = 0.0f;
				m_afOuterBarrelRecoilZ[i] = 0.0f;
				m_afBarrelTimer[i] = m_BotInfo_AAGun.fSecsBetweenBarrelFires;
			}
		}
	}
}

void CBotAAGun::_UpdateMatrices( void ) {
	CFMtx43A EntityMtxToWorld;

	CFMtx43A Mtx;
	Mtx = CFMtx43A::m_IdentityMtx;

	Mtx.SetRotationY( m_fGunHeading );
	m_AnimManMtxAim.UpdateFrame( BONE_HORIZONTAL, Mtx );

	Mtx.Identity33();
    Mtx.SetRotationX( m_fGunPitch );
    m_AnimManMtxAim.UpdateFrame( BONE_VERTICAL, Mtx );

	Mtx.Identity33();
	u32 i;
	for( i=0; i < NUM_BARRELS; i++ ) {
        Mtx.m_vPos.z = -m_afBarrelRecoilZ[i];
		m_AnimManMtxAim.UpdateFrame( i, Mtx );	
	
		Mtx.m_vPos.z = -m_afOuterBarrelRecoilZ[i];
		m_AnimManMtxAim.UpdateFrame( i+BARREL_TO_SHEATH_BONE_OFFSET, Mtx );	
	}

	m_pWorldMesh->UpdateTracker();

	m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated

//	_UpdateMountVars();

	EntityMtxToWorld.Identity();
	EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	EntityMtxToWorld.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}

void CBotAAGun::InflictDamage( CDamageData *pDamageData ) {
	CBot* pWhoHitMe = pDamageData->m_Damager.pBot; // when hit by a possessed bot, 

	if( pWhoHitMe==this ) { // don't take damage from self
		return;
	}

	CBot::InflictDamage( pDamageData );
}

BOOL CBotAAGun::ActionNearby( CEntity *pEntity )
{
	if( IsDeadOrDying() ) {
		return FALSE;
	}

	CBot::ActionNearby( pEntity );

	if( pEntity != NULL && 
		!(pEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		// It's not a bot.
		return FALSE;
	}

	CBot* pBot = (CBot*)pEntity;

	if( m_eStationStatus == CVehicle::STATION_STATUS_EMPTY ) {
		if( CanOccupyStation( pBot, CVehicle::STATION_GUNNER ) == CVehicle::STATION_STATUS_EMPTY ) {
			EnterStation( pBot, CVehicle::STATION_GUNNER );
		}
	} else if( m_eStationStatus == CVehicle::STATION_STATUS_OCCUPIED ) {
		if( pBot == m_pDriverBot ) {
			ExitStation( pBot, CVehicle::STATION_GUNNER );
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}

	return TRUE;
}

void CBotAAGun::CheckpointSaveSelect( s32 nCheckpoint ) {

	// save bot's weapons first
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->CheckpointSaveSelect( nCheckpoint );
	}

	if( m_apWeapon[1] ) {
		m_apWeapon[1]->CheckpointSaveSelect( nCheckpoint );
	}

	// then save self
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CBotAAGun::CheckpointSave( void ) {

	CBot::CheckpointSave();

	CFCheckPoint::SaveData( (u32 &)m_pDriverBot );
	CFCheckPoint::SaveData( m_fGunHeading );
	CFCheckPoint::SaveData( m_fGunPitch );

	return TRUE;
}

void CBotAAGun::SwiftKickOutDriver( void ) {
	
	if( m_pDriverBot ) {
		// we currently have a driver bot, kick him out and restore the camera if needed
		CBot *pBot = m_pDriverBot;
		m_pDriverBot->IsInWorld();
		
        DriverExit( pBot );

		if( pBot->m_nOwnerPlayerIndex >= 0 ) {
			gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)pBot->m_nOwnerPlayerIndex, pBot );
		}
		m_eCameraState = CAMERA_STATE_INACTIVE;
		m_pCameraBot = NULL;
	}
}

void CBotAAGun::CheckpointRestore( void ) {

	CBot::CheckpointRestore();

	_ResetToNeutral();

	fforce_Kill( &m_hForce );

	CFCheckPoint::LoadData( (u32 &)m_pDriverBot );
	CFCheckPoint::LoadData( m_fGunHeading );
	CFCheckPoint::LoadData( m_fGunPitch );	

	_UpdateMatrices();	
	_UpdateMountVars();
}

void CBotAAGun::CheckpointRestorePostamble( void ) {
	
	CBot::CheckpointRestorePostamble();
	
	if( m_pDriverBot ) {
		CBot* pBot = m_pDriverBot;
		m_pDriverBot = NULL;
		DriverEnter( pBot, m_apszBoneNameTable[BONE_ATTACHPOINT_BOT] );
		m_eStationStatus = CVehicle::STATION_STATUS_OCCUPIED;	// Necessary if not calling enter station
		m_eCameraState = CAMERA_STATE_IN_VEHICLE;  // Necessary if not calling enter station
		if( m_pDriverBot->IsPlayerBot() ) {
			gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pCameraBot->m_nPossessionPlayerIndex, this );			
		}
	}
}

void CBotAAGun::DriverEnter( CBot* pDriverBot, cchar *pszAttachPointBoneName/*=NULL*/ ) {
	_DriverEnter( pDriverBot, pszAttachPointBoneName );
	pDriverBot->Relocate_RotXlatFromUnitMtx_PS( &CFMtx43A::m_IdentityMtx );

	m_eCameraState = CAMERA_STATE_START_ENTER_VEHICLE;
	m_eStationStatus = CVehicle::STATION_STATUS_ENTERING;
	m_pDriverBot = pDriverBot;
	m_pCameraBot = pDriverBot;	
}

void CBotAAGun::DriverExit( CBot* pDriverBot ) {

	if( !pDriverBot ) {
		return;
	}

	if( pDriverBot != m_pDriverBot ) {
		return;
	}

	// if this was a player, restore reticle
	if( pDriverBot->m_nPossessionPlayerIndex >= 0 ) {
		CReticle *pReticle = &Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle;
		// When this is called during a checkpoint restore, sometimes the
		// reticle might not have been created (if the bot was in a position such
		// that a reticle was never needed).
		if( pReticle->IsCreated() ) {
			pReticle->SetNormOrigin( m_fReticlePrevX, m_fReticlePrevY );
			pReticle->SetType( m_ReticlePrevType );
			pReticle->EnableDraw( m_bReticleDrawPrev );
		}
	}

	_DriverExit( pDriverBot );

	m_eStationStatus = CVehicle::STATION_STATUS_EXITING;

	if( m_pDriverBot &&
		m_pDriverBot->IsInWorld() &&
		!m_pDriverBot->IsMarkedForWorldRemove() && 
		!CVehicle::IsDriverUseless( m_pDriverBot ) &&
		IsInWorld() &&
		!IsMarkedForWorldRemove() ) {
        m_eCameraState = CAMERA_STATE_START_EXIT_VEHICLE;
		m_pCameraBot = m_pDriverBot;
	} else {
		if( m_pDriverBot->m_nOwnerPlayerIndex >= 0 ) {
			gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pDriverBot->m_nOwnerPlayerIndex, m_pDriverBot );
		}

		m_eCameraState = CAMERA_STATE_INACTIVE;
		m_pCameraBot = NULL;
	}

	m_pDriverBot->DetachFromParent();
	m_pDriverBot = NULL;	
}

CVehicle::StationStatus_e CBotAAGun::CanOccupyStation( CBot *pBot, CVehicle::Station_e eStation ) {

	if( !pBot ) {
		return CVehicle::STATION_STATUS_WRONG_BOT;
	}

	if( !( pBot->TypeBits() & ( ENTITY_BIT_BOTGLITCH | ENTITY_BIT_BOTGRUNT ) ) ) {
		return CVehicle::STATION_STATUS_WRONG_BOT;
	}

	if( eStation != CVehicle::STATION_GUNNER ) {
		return CVehicle::STATION_STATUS_NO_STATION;
	}

	if( m_pDriverBot ) {
		return CVehicle::STATION_STATUS_OCCUPIED;
	}

	if( pBot->m_nPossessionPlayerIndex < 0 ) {
		// bot is not a player and gun can only take players currently
		return CVehicle::STATION_STATUS_UNAVAILABLE;
	}

	CFMtx43A *pMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_ATTACHPOINT_BOT] ];
	CFVec3A vBotToSeat;
	vBotToSeat.Sub( pBot->MtxToWorld()->m_vPos, pMtx->m_vPos );
	f32 fDot = vBotToSeat.Dot( pMtx->m_vUp );
	if( fDot < -m_BotInfo_AAGun.fEnterHalfHeight || fDot > m_BotInfo_AAGun.fEnterHalfHeight ) {
		return CVehicle::STATION_STATUS_OUT_OF_RANGE;
	}
	if( (vBotToSeat.MagSq() - (fDot * fDot)) <= (m_BotInfo_AAGun.fEnterRadius * m_BotInfo_AAGun.fEnterRadius) ) {
		return CVehicle::STATION_STATUS_EMPTY;
	}
	
	return CVehicle::STATION_STATUS_OUT_OF_RANGE;
}

BOOL CBotAAGun::GetStationEntryPoint( CVehicle::Station_e eStation, CFVec3A *pPoint ) {

	if( eStation != CVehicle::STATION_GUNNER ) {
		return FALSE;
	}

	*pPoint = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_ATTACHPOINT_BOT] ]->m_vPos;
	return TRUE;
}

// attempts to place bot into vehicle at specified station.  if STATION_STATUS_ENTERING is
// returned, then operation succeeded.  Other return values indicate why bot could not 
// enter the station.
// If bProximityCheck is TRUE, bot will only enter vehicle if he's standing in the station 
// entry area.  If bProximityCheck is FALSE, then bot will enter regardless of his location
// relative to the vehicle.
CVehicle::StationStatus_e CBotAAGun::EnterStation( CBot *pBot, CVehicle::Station_e eStation, BOOL bProximityCheck ) {

	if( m_eStationStatus != CVehicle::STATION_STATUS_EMPTY ) {
		return m_eStationStatus;
	}

	CVehicle::StationStatus_e Status = CanOccupyStation( pBot, CVehicle::STATION_GUNNER );

	if( bProximityCheck ) {
		if( Status != CVehicle::STATION_STATUS_EMPTY ) {
			return Status;
		}
	} else {
		if( Status != CVehicle::STATION_STATUS_EMPTY && Status != CVehicle::STATION_STATUS_OUT_OF_RANGE ) {
			return Status;
		}
	}

	m_pDriverBot = pBot;

	if( m_pDriverBot->m_nPossessionPlayerIndex >= 0 ) {
		CFVec3A vJumpVel;
		m_fEntryJumpTimer = CVehicle::ComputeEntryTrajectory( m_pDriverBot->MtxToWorld()->m_vPos,								// start pos
												m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_ATTACHPOINT_BOT] ]->m_vPos,				// end pos
												3.0f,																			// minimum jump height, 
												m_pDriverBot->m_pBotInfo_Gen->fGravity * m_pDriverBot->m_fGravityMultiplier,	// gravity
												vJumpVel );																		// output velocity

		m_pDriverBot->StartVelocityJump( &vJumpVel );
		m_pDriverBot->m_pWorldMesh->SetCollisionFlag( FALSE );
		m_pDriverBot->SetNoCollideStateAir( TRUE );
		m_eStationStatus = CVehicle::STATION_STATUS_PRE_ENTERING;

		return CVehicle::STATION_STATUS_ENTERING;
	} else {
		DriverEnter( m_pDriverBot, m_apszBoneNameTable[BONE_ATTACHPOINT_BOT] );
		return CVehicle::STATION_STATUS_ENTERING;
	}
}

CVehicle::StationStatus_e CBotAAGun::ExitStation( CBot *pBot, CVehicle::Station_e eStation ) {

	if( m_eStationStatus != CVehicle::STATION_STATUS_OCCUPIED ) {
		return m_eStationStatus;
	}

	if( pBot != m_pDriverBot ) {
		return CVehicle::STATION_STATUS_WRONG_BOT;
	}

	FASSERT( m_pDriverBot != NULL );
	DriverExit( m_pDriverBot );

	return CVehicle::STATION_STATUS_EXITING;
}

void CBotAAGun::_HandleDriverState( void ) {

	switch( m_eStationStatus ) {

	case CVehicle::STATION_STATUS_PRE_ENTERING:
		m_fEntryJumpTimer -= FLoop_fPreviousLoopSecs;
		if( m_pDriverBot->IsDeadOrDying() ) {
			// cancel jumping entry if bot dies
			m_pDriverBot->m_pWorldMesh->SetCollisionFlag( TRUE );
			m_pDriverBot->SetNoCollideStateAir( FALSE );
			m_eStationStatus = CVehicle::STATION_STATUS_EXITING;
			m_pDriverBot = NULL;
		}
		else if( m_fEntryJumpTimer < 0.0f ) {
			m_pDriverBot->m_pWorldMesh->SetCollisionFlag( TRUE );
			m_pDriverBot->SetNoCollideStateAir( FALSE );
			DriverEnter( m_pDriverBot, m_apszBoneNameTable[BONE_ATTACHPOINT_BOT] );
			m_eStationStatus = CVehicle::STATION_STATUS_ENTERING;
		}
		break;

	case CVehicle::STATION_STATUS_ENTERING:
		if( !m_pDriverBot->SwitchingWeapons() ) {
			// if we're done switching weapons, go ahead and start the engines and switch the reticle

			// now deal with the reticle... save off previous state and switch to loader reticle
			if( m_nPossessionPlayerIndex > -1 ) {
				// if player is driving, set up new reticle and save info about previous reticle
				CReticle *pReticle = &Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle;
				m_fReticlePrevX = pReticle->GetNormOriginX();
				m_fReticlePrevY = pReticle->GetNormOriginY();
				m_bReticleDrawPrev = pReticle->IsDrawEnabled();
				m_ReticlePrevType = pReticle->GetType();

				pReticle->SetNormOrigin( m_BotInfo_AAGun.fReticleX, m_BotInfo_AAGun.fReticleY );
				pReticle->SetType( CReticle::TYPE_QUADLASER );
				pReticle->EnableDraw( TRUE );
			}
			m_eStationStatus = CVehicle::STATION_STATUS_OCCUPIED;
		}
		break;

	case CVehicle::STATION_STATUS_EXITING:
		//if( m_eCameraState == CAMERA_STATE_INACTIVE ) {
			m_eStationStatus = CVehicle::STATION_STATUS_EMPTY;
		//}
		break;
	}
}

void CBotAAGun::_CameraTransitionWork( void ) {
	CFCamera *pCamera;

	if( (m_eCameraState == CAMERA_STATE_IN_VEHICLE) || (m_eCameraState == CAMERA_STATE_INACTIVE) ) {
		return;
	}
	if( m_pCameraBot == NULL || !m_pCameraBot->IsPlayerBot()) {
		m_eCameraState = CAMERA_STATE_INACTIVE;
		return;
	}

	m_CamInfo.m_pmtxMtx	= &m_ManCamMtx;
	gamecam_CamBotTransInfo_t cbinfo;

	switch( m_eCameraState ) {
	
	case CAMERA_STATE_START_ENTER_VEHICLE:
		m_fUnitCameraTransition = 0.0f;
		m_eCameraState			= CAMERA_STATE_ENTERING_VEHICLE;
		pCamera					= gamecam_GetActiveCamera();
		m_ManCamMtx				= pCamera->GetFinalXfm()->m_MtxR;
		m_TransCamMtx			= pCamera->GetFinalXfm()->m_MtxR;
		pCamera->GetFOV( &m_CamInfo.m_fHalfFOV );
		return;

	case CAMERA_STATE_ENTERING_VEHICLE:
		m_fUnitCameraTransition += FLoop_fPreviousLoopSecs * _USERPROPS_CAMRATE_IN;
		if( m_fUnitCameraTransition >= 1.0f ) {
			gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pCameraBot->m_nPossessionPlayerIndex, this );
			m_eCameraState = CAMERA_STATE_IN_VEHICLE;
			return;
		}
		cbinfo.pBot1	= m_pCameraBot;
		cbinfo.pBot2	= this;
		cbinfo.uPlayer	= m_pCameraBot->m_nPossessionPlayerIndex;
		break;
		
	case CAMERA_STATE_START_EXIT_VEHICLE:
		m_fUnitCameraTransition = 0.0f;
		m_eCameraState			= CAMERA_STATE_EXITING_VEHICLE;
		pCamera					= gamecam_GetActiveCamera();
		m_ManCamMtx				= pCamera->GetFinalXfm()->m_MtxR;
		m_TransCamMtx			= pCamera->GetFinalXfm()->m_MtxR;
		pCamera->GetFOV( &m_CamInfo.m_fHalfFOV );
		return;		

	case CAMERA_STATE_EXITING_VEHICLE:
		m_fUnitCameraTransition += FLoop_fPreviousLoopSecs * _USERPROPS_CAMRATE_OUT;
		if( m_fUnitCameraTransition > 1.0f ) {
			gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pCameraBot->m_nPossessionPlayerIndex, m_pCameraBot );
			m_eCameraState = CAMERA_STATE_INACTIVE;
			m_pCameraBot = NULL;
			return;
		}
		cbinfo.pBot1 = this;
		cbinfo.pBot2 = m_pCameraBot;			
		cbinfo.uPlayer = m_pCameraBot->m_nPossessionPlayerIndex;
		break;			
	}

	// if we're here, we're doing the camera transition thing
	gamecam_CamBotTransResult_t	result, result1, result2;
    
	cbinfo.fUnitInterp	= fmath_UnitLinearToSCurve( m_fUnitCameraTransition );

	gamecam_DoCamBotTransition( &cbinfo, &m_CamInfo, &result, &result1, &result2 );
}

// put driver bot into vehicle
void CBotAAGun::_DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName /*=NULL*/ ) {
	
	if( !pDriverBot ) {
		return;
	}

	// attach bot to vehicle
	pDriverBot->Attach_ToParent_WithGlue_WS( this, pszAttachPointBoneName );

	if( pDriverBot->m_nPossessionPlayerIndex >= 0 ) {
		// transfer driving player bot's controls to vehicle
		pDriverBot->Controls()->Zero();
		SetControls( pDriverBot->Controls() );
		pDriverBot->SetControls( NULL );	

		// set the hud to our gun's settings
		CHud2 *pHud = CHud2::GetHudForPlayer( pDriverBot->m_nPossessionPlayerIndex );
		CBot::SetHudMode( pHud, m_pBotDef );
	}
	
	pDriverBot->EnteringMechWork( this );

	// set vehicle's index to that of driver bot
	m_nPossessionPlayerIndex = pDriverBot->m_nPossessionPlayerIndex;	
#if 0
	if( m_nPossessionPlayerIndex > -1 ) {
		//when players take "possession" of bots with brains, call this
		//because the brain will be repurposed during that time.
		//see exit vehicle for the undoing of this.
		aibrainman_ConfigurePlayerBotBrain( AIBrain(), m_nPossessionPlayerIndex );	 //turns this bot's brain into a player brain
	} else {
		aibrainman_ConfigureNPCVehicleBrain( AIBrain() , pDriverBot->AIBrain() );
	}
#endif
	pDriverBot->ZeroVelocity();
	pDriverBot->ImmobilizeBot();

	// remember a few things about the bot so that we can restore them later
	m_bPrevCastingShadows = ( pDriverBot->m_pWorldMesh->m_nFlags & FMESHINST_FLAG_CAST_SHADOWS );
	m_bPrevInvincible = pDriverBot->IsInvincible();
	m_bPrevSpotlightOn = pDriverBot->IsSpotLightOn();

	// turn off/disable a few things while we are driving the gun
	pDriverBot->m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;
	pDriverBot->SetInvincible( TRUE );	
	pDriverBot->SetSpotLightOff( TRUE );
}

// remove driver bot from vehicle
void CBotAAGun::_DriverExit( CBot *pDriverBot ) {
	CFVec3A vOffset;
	
	if( !pDriverBot ) {
		return;
	}

	if( !pDriverBot->IsImmobilizePending() ) {
		// don't mobilize if an immobilize is pending (EMP grenade for instance)
		pDriverBot->MobilizeBot();
	}

	// if driver's spotlight was on when entering vehicle, turn it back on now.
	if( m_bPrevSpotlightOn ) {
		pDriverBot->SetSpotLightOn();
	}

	// if driver was casting a shadow before entering vehicle, turn it back on now.
	if( m_bPrevCastingShadows && pDriverBot->m_pWorldMesh ) {
		pDriverBot->m_pWorldMesh->m_nFlags |= FMESHINST_FLAG_CAST_SHADOWS;
	}

	// give driver bot class a chance to cleanup anims and stuff
	pDriverBot->ExitingMechWork();
	
	// if the driver has an AIBrain, tell him he has been ejected from this vehicle
	if( pDriverBot->AIBrain() ) {
//		aibrainman_NotifyMechEject( pDriverBot->AIBrain() );
	}

	if( m_nPossessionPlayerIndex >= 0 ) {
		// restore the HUD settings
		CHud2 *pHud = CHud2::GetHudForPlayer( m_nPossessionPlayerIndex );
		CBot::SetHudMode( pHud, pDriverBot->m_pBotDef );

		// give controls back to driving player bot and NULL vehicle controls
		pDriverBot->SetControls( Controls() );
		pDriverBot->Controls()->Zero();
		SetControls( NULL );

		// put the vehicles brain back to normal, now that "player possession" is done.
		//aibrainman_ConfigurePostNPCVehicleBrain(AIBrain());
	} else {
		//put the vehicles brain back to normal, now that "bot possession" is done.
		//aibrainman_ConfigurePostNPCVehicleBrain(AIBrain());	//Normal for a vehicle is set by this function
	}

	m_nPossessionPlayerIndex = -1;

	pDriverBot->SetInvincible( m_bPrevInvincible );
}

void CBotAAGun::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS ) {
	*pApproxMuzzlePoint_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[m_nNextFireBarrelIndex+BARREL_TO_TIP_BONE_OFFSET] ]->m_vPos;
}

void CBotAAGun::HandleTargeting( void ) {
	// don't call base class, it doesn't work since botAAgun doesn't have a weapon

	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );

	//do all targeting lock bit checks here.
	if( m_nPossessionPlayerIndex < 0 ) {
		if(	!m_bControls_Human &&
			(m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT) ) {
			
			m_TargetedPoint_WS = m_ControlBot_TargetPoint_WS;
			
			// Announce that we're locked... and ready to fire
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
		} else {
			CFMtx43A *pMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[m_nNextFireBarrelIndex+BARREL_TO_TIP_BONE_OFFSET] ];

			m_TargetedPoint_WS.Mul( pMtx->m_vFront, m_BotInfo_AAGun.fMaxRange ).Add( pMtx->m_vPos );
		}

		// copy the targeted WS point
		m_vUnFocusedTargetPoint_WS = m_TargetedPoint_WS;
	} else {
		_UpdateReticle();
	}

	ComputeApproxMuzzlePoint_WS( &m_TargetLockUnitVec_WS );
	m_TargetLockUnitVec_WS.Sub( m_TargetedPoint_WS );
	m_TargetLockUnitVec_WS.Unitize().Negate();
}

void CBotAAGun::_UpdateReticle( void ) {

//	m_pTargetedEntity = NULL;
	if( m_nPossessionPlayerIndex < 0 ) {
		return;
	}

	if( m_pDriverBot == NULL ) {
		return;
	}

	ComputeHumanTargetPoint_WS( &m_TargetedPoint_WS, NULL, -1.f, &m_vUnFocusedTargetPoint_WS );
	if( m_pTargetedMesh ) {
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
	}
}

void CBotAAGun::_SecondaryWeaponWork( BOOL bSecondaryFireDown ) {

	if( m_bMorterAttachment ) {
		m_fSecsTillNextMortarFire -= FLoop_fPreviousLoopSecs;

		if( bSecondaryFireDown && (m_fSecsTillNextMortarFire <= 0.0f) ) {
			// fire the mortar
			CFVec3A *pFirePos = &m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_SECONDARY] ]->m_vPos;
			CFVec3A EndRay;
			EndRay.Sub( m_vUnFocusedTargetPoint_WS, *pFirePos );
			EndRay.Unitize();
			EndRay.Mul( m_BotInfo_AAGun.fMaxRange );
			EndRay.Add( *pFirePos );
			
			// find out where to 
			FCollImpact_t CollImpact;

			FWorld_nTrackerSkipListCount = 0;
			AppendTrackerSkipList();
			m_pDriverBot->AppendTrackerSkipList();

			//fdraw_DevLine( &pFirePos->v3, &EndRay.v3 );
	
			if( fworld_FindClosestImpactPointToRayStart( &CollImpact, pFirePos, &EndRay, 
														FWorld_nTrackerSkipListCount, 
														FWorld_apTrackerSkipList, TRUE, m_pWorldMesh, -1,
														FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
				// we hit something...
				FExplosionSpawnParams_t SpawnParams;
			    CFMtx43A ProjMtx;
				CFVec3A vVelocity, vUnitVelocity;


				CEProj *pProj = CEProjPool::GetProjectileFromFreePool( m_hMortarProjPool );
				if( pProj == NULL ) {
					// No more projectiles...
					return;
				}

				f32 fDistXZ = CollImpact.ImpactPoint.DistXZ( *pFirePos );
				f32 fUnitMult = fmath_Div( (fDistXZ - m_MortarInfo.fMinRangeXZ), (m_MortarInfo.fMaxRangeXZ - m_MortarInfo.fMinRangeXZ) );
				FMATH_CLAMPMIN( fUnitMult, 0.0f );
				f32 fStraightLineVel = FMATH_FPOT( fUnitMult, m_MortarInfo.fMinCannonballVelocityXZ, m_MortarInfo.fMaxCannonballVelocityXZ );
				f32 fTimeToTarget = fmath_Div( fDistXZ, fStraightLineVel );
				
				
				ProjMtx.Set( *m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[BONE_SECONDARY] ] );
				
				pProj->Init();
				pProj->SetAmbientPropertiesFromSoundGroup( m_MortarInfo.pCannonBallInAir );
				pProj->SetDamager( NULL, this );
				
				f32 fTimeInAir = aiutils_ComputeTrajectoryFromXZVel( vVelocity, *pFirePos, CollImpact.ImpactPoint, fStraightLineVel, 32.0f );
				
				f32 fLinearVelocity = vUnitVelocity.SafeUnitAndMag(vVelocity);

				pProj->SetLinSpeed( fLinearVelocity );
				pProj->SetLinUnitDir_WS( &vUnitVelocity );

				pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, 1.0f );
				pProj->SetMaxLifeSecs( 20.0f );
				pProj->SetMaxDistCanTravel( m_BotInfo_AAGun.fMaxRange );
				pProj->SetDetonateCallback( _ProjDetonateCallback );
				pProj->Launch();
				PlaySound( m_MortarInfo.pMortarFire );

				SpawnParams.InitToDefaults();
				SpawnParams.pDamager= pProj->GetDamager();
				SpawnParams.Pos_WS.Set( ProjMtx.m_vPos );
				SpawnParams.UnitDir.Set( vUnitVelocity );
				SpawnParams.uFlags |= FEXPLOSION_SPAWN_EXPLOSION_FOLLOW_UNIT_DIR;

				// we'll probably need this handle later, save it off?	
				FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
				if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
					CExplosion2::SpawnExplosion( hSpawner, m_MortarInfo.hExplLaunchGroup, &SpawnParams );
				}

				// add some camera shake
				CFCamera *pCamera = fcamera_GetCameraByIndex( m_nPossessionPlayerIndex );
				pCamera->ShakeCamera( 0.040f, 0.105f );

				if( m_nPossessionPlayerIndex >= 0 ) {
					fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );
				}
				
				// reset our timer so that we don't fire too often
				m_fSecsTillNextMortarFire = m_MortarInfo.fOORoundsPerSec;
			} else {
				m_fSecsTillNextMortarClick -= FLoop_fPreviousLoopSecs;
				if( m_fSecsTillNextMortarClick <= 0.0f ) {
					PlaySound( m_MortarInfo.pMortarNoFire );
					m_fSecsTillNextMortarClick = 1.0f;
				}
			}
		} else {
			m_fSecsTillNextMortarClick = 0.0f;
		}
	}
}

BOOL CBotAAGun::_ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact ) {

	if( bMakeEffect ) {

		if( nEvent == CEProj::EVENT_HIT_GEO ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();
			SpawnParams.pDamager= pProj->GetDamager();
			SpawnParams.Pos_WS  = pImpact->ImpactPoint;
			SpawnParams.UnitDir = pImpact->UnitFaceNormal;
			
			// we'll probably need this handle later, save it off?	
			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				CExplosion2::SpawnExplosion( hSpawner, m_MortarInfo.hExplDetonateGroup, &SpawnParams );
			}
			CFXShockwave::AddShockwave( pImpact->ImpactPoint,
				3.0f,
				4.0f,
				ENTITY_BIT_BOT,
				m_MortarInfo.fMaxShockwaveImpulse,
				m_MortarInfo.fMinShockwaveImpulse );

			CFSoundGroup::PlaySound( m_MortarInfo.pMortarExplode, FALSE, &pImpact->ImpactPoint );			
		}
	}
	return FALSE;
}

void CBotAAGun::_PrimaryWeaponWork( BOOL bPrimaryFireDown ) {

	m_fSecsTillNextFire -= FLoop_fPreviousLoopSecs;
	m_fGunAISoundTimer -= FLoop_fPreviousLoopSecs;
	m_fSecsTillNextFlakFx -= FLoop_fPreviousLoopSecs;

	if( !bPrimaryFireDown ) {
		if( m_fGunUnitShotSpread > 0.0f ) {
			m_fGunUnitShotSpread -= FLoop_fPreviousLoopSecs * m_BotInfo_AAGun.fShotSpreadRecRate;
		}
		return;
	}

	if( m_fSecsTillNextFire <= 0.0f ) {
		// fire another barrel
		m_afBarrelRecoilZ[m_nNextFireBarrelIndex] = 0.0f;
		m_afOuterBarrelRecoilZ[m_nNextFireBarrelIndex] = 0.0f;
		m_afBarrelTimer[m_nNextFireBarrelIndex] = 0.0f;

		f32 fSpreadFactor;
		CFVec3A	vSpread, vMuzzlePoint, vUnitFireDir, vSmokePos;;

		// fire the gun
		JustFired();
		m_fGunUnitShotSpread += FLoop_fPreviousLoopSecs * m_BotInfo_AAGun.fShotSpreadRate;
		FMATH_CLAMP_MAX1( m_fGunUnitShotSpread );

		fSpreadFactor = FMATH_FPOT( m_fGunUnitShotSpread, m_BotInfo_AAGun.fMinShotSpread, m_BotInfo_AAGun.fMaxShotSpread );
		vSpread.Set( fmath_RandomFloatRange( -fSpreadFactor, fSpreadFactor ),
					 fmath_RandomFloatRange( -fSpreadFactor, fSpreadFactor ),
					 fmath_RandomFloatRange( -fSpreadFactor, fSpreadFactor ) );

		ComputeApproxMuzzlePoint_WS( &vMuzzlePoint );

		if( m_nPossessionPlayerIndex >= 0 ) {
			FocusHumanTargetPoint_WS( &m_TargetedPoint_WS, NULL );
			m_TargetLockUnitVec_WS.Set( vMuzzlePoint );
			m_TargetLockUnitVec_WS.Sub( m_TargetedPoint_WS );
			m_TargetLockUnitVec_WS.Unitize().Negate();

			// do some force feedback
			//fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_LOW, &m_hForce );

			// add some camera shake
			CFCamera *pCamera = fcamera_GetCameraByIndex( m_nPossessionPlayerIndex );
			pCamera->ShakeCamera( 0.015f, 0.015f );
		}

		vUnitFireDir.Add( m_TargetLockUnitVec_WS, vSpread );
		vUnitFireDir.Unitize();		
#if _EFFECTS_ON
		// emit particles muzzleflash effects and tail pipe smoke...
		if( m_ahEmitters[m_nNextFireBarrelIndex] != FPARTICLE_INVALID_HANDLE ) {
			// the prevous fx isn't finished yet, kill it's callback function since we are going to reuse this fx
			fparticle_SetKillCallback( m_ahEmitters[m_nNextFireBarrelIndex], NULL, NULL );
		}
		CFMtx43A *pMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[m_nNextFireBarrelIndex+BARREL_TO_TIP_BONE_OFFSET] ];;

		m_ahEmitters[m_nNextFireBarrelIndex] = fparticle_SpawnEmitter( m_BotInfo_AAGun.hFlashPartDef, vMuzzlePoint.v3, &pMtx->m_vFront.v3, 1.0f );
		fparticle_SetKillCallback( m_ahEmitters[m_nNextFireBarrelIndex], CBotAAGun::_ParticleEmitterCallback, &m_ahEmitters[m_nNextFireBarrelIndex] );

		pMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndices[m_nNextFireBarrelIndex] ];
		CFVec3A vNegFireDir;
		vNegFireDir.Mul( pMtx->m_vFront, -1.0f );
		vSmokePos.Mul( vNegFireDir, m_BotInfo_AAGun.fRecoilBack * 0.5f );
		vSmokePos.Add( pMtx->m_vPos );
		fparticle_SpawnEmitter( m_BotInfo_AAGun.hSmokePartDef, vSmokePos.v3, &vNegFireDir.v3, 1.0f );

		// emit some muzzle flash
		CMuzzleFlash::MuzzleLight( &vMuzzlePoint, m_BotInfo_AAGun.fMuzzleLightRadius );
#endif

		// see whether we hit anything...
		CFVec3A			vRayEnd;
		FCollImpact_t	CollImpact;

		vRayEnd.Mul( vUnitFireDir, m_BotInfo_AAGun.fMaxRange ).Add( vMuzzlePoint );
		
		FWorld_nTrackerSkipListCount = 0;
		AppendTrackerSkipList();
		m_pDriverBot->AppendTrackerSkipList();
	
		if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &vMuzzlePoint, &vRayEnd, 
													 FWorld_nTrackerSkipListCount, 
													 FWorld_apTrackerSkipList, TRUE, m_pWorldMesh, -1,
													 FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
		

			vRayEnd = CollImpact.ImpactPoint;
			CFVec3A vFlashPos;
			const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( &CollImpact );

#if _EFFECTS_ON
			// draw impact effects
            pCollMaterial->DrawAllDrawableParticles( &CollImpact.ImpactPoint, 
													 &CollImpact.UnitFaceNormal,
													 (m_nNextFireBarrelIndex & 0x1),
													 fmath_RandomFloatRange( 0.25f, 0.6f ),
													 fmath_RandomFloatRange( 0.5f, 1.0f ),
													 fmath_RandomFloatRange( 0.5f, 1.0f ) );

			// Spawn some chunks (every other barrel and only if close to us)...
			if( (m_nNextFireBarrelIndex & 0x1) && CollImpact.fImpactDistInfo < 0.5f ) {
				CFDebrisSpawner DebrisSpawner;
				DebrisSpawner.InitToDefaults();

				DebrisSpawner.m_Mtx.m_vPos = CollImpact.ImpactPoint;
				DebrisSpawner.m_Mtx.m_vZ = CollImpact.UnitFaceNormal;
				DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
				DebrisSpawner.m_pDebrisGroup = pCollMaterial->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_MEDIUM ];
				DebrisSpawner.m_fMinSpeed = 10.0f;
				DebrisSpawner.m_fMaxSpeed = 20.0f;
				DebrisSpawner.m_fUnitDirSpread = 0.2f;
				DebrisSpawner.m_nMinDebrisCount = 1;
				DebrisSpawner.m_nMaxDebrisCount = 3;

				CGColl::SpawnDebris( &DebrisSpawner );
			}

			if( pCollMaterial->IsImpactFlashEnabled() ) {
				CFVec3A vImpactFlashPos;
				vImpactFlashPos.Mul( CollImpact.UnitFaceNormal, m_BotInfo_AAGun.fMuzzleImpactOffset ).Add( CollImpact.ImpactPoint );
				CMuzzleFlash::AddFlash_CardPosterXY( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
													 vImpactFlashPos,
													 m_BotInfo_AAGun.fMuzzleImpactOffset,
													 m_BotInfo_AAGun.fMuzzleImpactAlpha );
			}
#endif

			if( CollImpact.pTag == NULL ) {
				// hit world geo...
#if _EFFECTS_ON
				//if( pCollMaterial->IsPockMarkEnabled( &CollImpact ) ) {
				//	potmark_NewPotmark( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 
				//						m_BotInfo_AAGun.fPotmarkScale, POTMARKTYPE_BULLET );
				//}
				CFDecal::Create( m_BotInfo_AAGun.hBulletDecalDef, &CollImpact );
#endif
			} else {
				// hit something interesting...
				CFWorldMesh *pWorldMesh = (CFWorldMesh*)CollImpact.pTag;
                
				FASSERT( pWorldMesh->GetType() == FWORLD_TRACKERTYPE_MESH );
				if( pWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
					CEntity *pDamagee = (CEntity*)pWorldMesh->m_pUser;

					// it's an entity...
					CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
					if( pDamageForm ) {
						pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
						pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
						pDamageForm->m_pDamageProfile = m_BotInfo_AAGun.pPrimaryWeaponDamageProfile;
						pDamageForm->m_Damager.pWeapon = NULL;
						pDamageForm->m_Damager.pBot = this;
						pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
						pDamageForm->m_pDamageeEntity = pDamagee;
						pDamageForm->InitTriDataFromCollImpact( pWorldMesh, &CollImpact, &vUnitFireDir );

						CDamage::SubmitDamageForm( pDamageForm );
					}

					// Make an AI impact sound in case any AI are listening...
					if( (m_nPossessionPlayerIndex > -1) && (m_fGunAISoundTimer <= 0.0f) ) {
						AIEnviro_AddSound( CollImpact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, this );
						m_fGunAISoundTimer = 0.5f;
					}
				}
			}
		} else {
			// didn't hit anything, put some flak up in the air
			if( m_fSecsTillNextFlakFx <= 0.0f ) {
				vSmokePos.Mul( vUnitFireDir, m_BotInfo_AAGun.fMaxRange * fmath_RandomFloatRange( 0.75f, 1.0f ) ).Add( vMuzzlePoint );
				vSpread.Mul( m_BotInfo_AAGun.fMaxRange * 0.8f );
				vSmokePos.Add( vSpread );

				fparticle_SpawnEmitter( m_BotInfo_AAGun.hFlakPartDef, vSmokePos.v3, NULL, fmath_RandomFloatRange( 0.5f, 1.0f ) );
				m_fSecsTillNextFlakFx = (1.0f/2.0f);
			}
		}

 		// play a sound
		PlaySound( m_BotInfo_AAGun.pGunFire );

		m_fGunTracerTimer -= FLoop_fPreviousLoopSecs;
		if( m_fGunTracerTimer <= 0.0f ) {
			f32 fTracerLen = vMuzzlePoint.Dist( vRayEnd );
			CMuzzleFlash::AddFlash_CardPosterZ( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1],
												vMuzzlePoint, vUnitFireDir,
												m_BotInfo_AAGun.fTracerWidth,
												fTracerLen,
												m_BotInfo_AAGun.fTracerAlpha );
			m_fGunTracerTimer += m_BotInfo_AAGun.fTracerTime * fmath_RandomFloatRange( 0.9f, 1.1f );
		}

		// update which barrel we will fire from next time we fire
		m_nNextFireBarrelIndex++;
		if( m_nNextFireBarrelIndex >= NUM_BARRELS ) {
			m_nNextFireBarrelIndex = 0;
		}
		m_fSecsTillNextFire = m_fSecsBetweenShots;
	}
}

void CBotAAGun::EnableDriverMovement( BOOL bEnable ) {
	m_bAllowDriverMovement = bEnable;
}

void CBotAAGun::EnableDriverExit( BOOL bEnable ) {
	m_bAllowDriverExit = bEnable;
}

void CBotAAGun::EnableEnterMsgDisplay( BOOL bEnable ) {
	m_bDisplayEnterMsg = bEnable;
}

BOOL CBotAAGun::ShouldEnterTextBeDisplayed( void ) {
	return m_bDisplayEnterMsg; 
}


