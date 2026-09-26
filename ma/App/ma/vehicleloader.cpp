//////////////////////////////////////////////////////////////////////////////////////
// vehicleloader.cpp -
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
// 01/14/02 Elliott 	Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "vehicleloader.h"
#include "meshtypes.h"
#include "fforce.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "protrack.h"
#include "gamecam.h"
#include "meshentity.h"
#include "muzzleflash.h"
#include "weapon.h"
#include "potmark.h"
#include "ai\aienviro.h"
#include "player.h"
#include "cambot.h"
#include "botpart.h"
#include "MultiplayerMgr.h"
#include "sas_user.h"
//#include "ftext.h"
//#include "gamepad.h"


#define _ENABLE_BOT_CRUSHING					0

#define _IMPACT_IMPULSE_MULTIPLIER				0.25f

static cchar _pszMeshFile[] =					"grdlloadr00";

#define _BOTINFO_FILENAME						( "b_vloader" )
#define _SFXBANK_LOADER							( "Loader" )
//#define	_SFXBANK_HEAVY							( "Bots_Heavy" )
#define _SFXBANK_WEAPON							( "Weapons" )

#define _TUMBLE_DAMPING							( 250.0f )
#define _IMPACT_ENGINE_CUTOFF_THRESHOLD			( 1000.0f )


#define _VEHICLE_COLLISION_MASK					((~ENTITY_BITS_ALLBOTBITS) | ENTITY_BIT_VEHICLE)

//////////////////////////////
//PHYSICS DATA
#define _FRICTION_CONTACT_TIME					( 0.1f )	// time within which to apply friction after contact with ground is broken
#define _KINETIC_FORCE_MUL						( 0.2f )	// multiplier for applied forces when under kinetic friction
#define _STATIC_TRANS_HYST						( 0.7f )	// fraction of force which must be reduced to before static friction is restored

#define _HOVER_MAX_HEIGHT						40.0f
#define _HOVER_HEIGHT							6.0f
#define _HOVER_MIN_HEIGHT						4.5f
#define _HOVER_TOP_ZONE							(_HOVER_MAX_HEIGHT-_HOVER_HEIGHT)
#define _HOVER_BOTTOM_ZONE						(_HOVER_HEIGHT-_HOVER_MIN_HEIGHT)
#define _ALTCHECK_COLLSPHERE_RADIUS				( 0.5f )							// radius of altitude check collsphere

// ENGINE
#define _ENGINE_X_ANGLE							( FMATH_DEG2RAD(30.0f) )			// max X axis rotation
#define _ENGINE_Z_ANGLE							( FMATH_DEG2RAD(15.0f) )			// max Z axis rotation
#define _ENGINE_HOVER_Z_ANGLE					( FMATH_DEG2RAD(20.0f) )			// max Z axis rotation for hover height
#define	_ENGINE_UPDATE_RATE						( 7.5f )							// rate the engine updates

// CLAW
#define _MIN_CLAW_ROTATE						(FMATH_DEG2RAD(55.0f))				// claw's rotation at 0 - open
#define _MAX_CLAW_ROTATE						(FMATH_DEG2RAD(-12.0f))				// claw's rotation at 1 - closed
#define _MAX_CLAW_DELTA							(10.0f)
#define _CLAW_ARM_RAD							(-FMATH_DEG2RAD(11.0f))
#define _PICKUP_FORCE							( -10.0f )
#define _CLAW_RADIUS							( 7.0f )
#define _CLAW_CENTER_THROW						( 8.0f )
#define _CLAW_ARM_THROW							( 4.0f ) 
#define _CLAWABLE_ENTITY_BITS					(ENTITY_BIT_BOTGRUNT | ENTITY_BIT_MESHENTITY)
#define _MAX_SPAN								( 10.0f )
#define _CLAW_MIN_UNIT_ANGLE					( 0.1f )
#define _BOTCRUSH_PRESSURE						( 2.0f )			// amount of trigger pressure needed to crush a bot


#define _RETICLE_NORM_X							( 0.0f )
#define _RETICLE_NORM_Y							( 0.3f )

#define _CLAWED_COLSPHERE_RAD_MULT				( 0.75f )

#define _GUN_SLERP_SPEED						( 20.0f )

#define _USERPROPS_MAX_ENGINE_SOUND_DELTA_UP	( 2.0f )
#define _USERPROPS_MAX_ENGINE_SOUND_DELTA_DOWN	( 2.0f )

#define _ENGINE_VOLUME_UP_RATE					( 2.0f )
#define _ENGINE_VOLUME_DOWN_RATE				( 0.5f )

#define _RIGHT_FORCE_MULTIPLIER					( 0.2f )

static CVehicleLoaderBuilder	_VehicleLoaderBuilder;



//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	f32 _fClawPressureTracker;
//#endif


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleLoader
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

//static VehicleLoader data
BOOL					CVehicleLoader::m_bSystemInitialized = FALSE;	// TRUE: InitSystem() has been called
u32						CVehicleLoader::m_nBotClassClientCount = 0;
f32						CVehicleLoader::m_afCollisionPtsRadius[CVehicleLoader::COLPOINT_COUNT];
CFVec3A					CVehicleLoader::m_avCollisionPts[CVehicleLoader::COLPOINT_COUNT];
CFVec3A 				CVehicleLoader::m_vForceAppPos;

CFVerlet*				CVehicleLoader::m_apVerletPool[_VERLET_COUNT];
BOOL					CVehicleLoader::m_abVerletPoolActive[_VERLET_COUNT];
CFVec3A					CVehicleLoader::m_avVerletObjectAveSpeed[_VERLET_COUNT];
CMeshEntity*			CVehicleLoader::m_apVerletPoolME[_VERLET_COUNT];
BOOL					CVehicleLoader::m_abVerletMarkedForDeactivate[_VERLET_COUNT];

CVehicleLoader*			CVehicleLoader::m_pCBLoader					= NULL;
CFSphere				CVehicleLoader::m_CBSphere;
CEntity*				CVehicleLoader::m_pCBEntity					= NULL;
FParticle_DefHandle_t	CVehicleLoader::m_hExhaustParticleDef		= FPARTICLE_INVALID_HANDLE;
FParticle_DefHandle_t	CVehicleLoader::m_hEngineStopParticleDef	= FPARTICLE_INVALID_HANDLE;

f32						CVehicleLoader::m_fExhaustParticleHeight	= 0.1f;

CFVec2					CVehicleLoader::m_avEngineVib[4];

f32						CVehicleLoader::m_fOOMaxForce;			// precalculated value
f32						CVehicleLoader::m_fOOMaxTorque;			// precalculated value
f32						CVehicleLoader::m_fRightingForceMag;	// precalculated value


CVehicleLoader::BotInfo_Gen_t				CVehicleLoader::m_BotInfo_Gen;
CVehicleLoader::BotInfo_MountAim_t			CVehicleLoader::m_BotInfo_MountAim;
CVehicleLoader::BotInfo_VehiclePhysics_t	CVehicleLoader::m_BotInfo_VehiclePhysics;
CVehicleLoader::BotInfo_LoaderSounds_t		CVehicleLoader::m_BotInfo_LoaderSounds;
CVehicleLoader::BotInfo_Loader_t			CVehicleLoader::m_BotInfo_Loader;
CVehicle::BotInfo_Vehicle_t					CVehicleLoader::m_BotInfo_Vehicle;				// Vehicle bot info
CVehicleCamera::BotInfo_VehicleCamera_t		CVehicleLoader::m_BotInfo_DriverCamera;
CVehicleCamera::BotInfo_VehicleCamera_t		CVehicleLoader::m_BotInfo_MPDriverCamera;

s32 CVehicleLoader::m_nBoneIdxDriverAttach;
s32 CVehicleLoader::m_nBoneIdxGun;
s32 CVehicleLoader::m_nBoneIdxPriFire;
s32 CVehicleLoader::m_nBoneIdxVictimAttach;
s32 CVehicleLoader::m_nBoneIdxClawArm;
s32 CVehicleLoader::m_nBoneIdxClawRight;
s32 CVehicleLoader::m_nBoneIdxClawLeft;
s32 CVehicleLoader::m_nBoneIdxEngineRight;
s32 CVehicleLoader::m_nBoneIdxEngineLeft; 


BOOL CVehicleLoader::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;

	// set some precalculated values
	m_avEngineVib[0].Set( FMATH_DEG2RAD(1.0f),	0.0f );
	m_avEngineVib[2].Set( 0.0f,					FMATH_DEG2RAD(1.0f) );
	m_avEngineVib[1].Set( FMATH_DEG2RAD(-1.0f),	0.0f );
	m_avEngineVib[3].Set( 0.0f,					FMATH_DEG2RAD(-1.0f) );

	m_vForceAppPos.Set( 0.0f, 0.75f, 0.0f );

	return TRUE;
}


BOOL CVehicleLoader::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	m_nBotClassClientCount++;

	if( !CVehicle::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...

		return FALSE;
	}

	if( m_nBotClassClientCount > 1 ) {
		// Resources already loaded...

		return TRUE;
	}

	CFVec3A vDim;
	FResFrame_t frame;
	
	frame = fres_GetFrame();

	// SOUNDS

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _SFXBANK_LOADER ) ) {
		DEVPRINTF( "CVehicleLoader::_InitSounds(): Could not load sound effect bank '%s'\n", _SFXBANK_LOADER );
	}

	//load weapon sounds too for the gun
	if( !fresload_Load( FSNDFX_RESTYPE, _SFXBANK_WEAPON ) ) {
		DEVPRINTF( "CVehicleLoader::_InitSounds(): Could not load sound effect bank '%s'\n", _SFXBANK_WEAPON );
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitWithError;
	}

	if( !fgamedata_ReadFileUsingMap( m_aSoundDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitWithError;
	}

	m_fRightingForceMag	= m_BotInfo_VehiclePhysics.fMass * m_BotInfo_VehiclePhysics.fRightingForceMag;
	m_fOOMaxForce		= fmath_Inv( m_BotInfo_VehiclePhysics.fMass * m_BotInfo_VehiclePhysics.fMaxAccel );
	m_fOOMaxTorque		= fmath_Inv( m_fRightingForceMag * 0.5f );		

	// set up verlet pool
	vDim.Set( 1.0f, 1.0f, 1.0f );

	// create verlet object pool...
	for( u32 i=0; i<_VERLET_COUNT; i++ ) {
		m_abVerletMarkedForDeactivate[i]			= FALSE;
		m_abVerletPoolActive[i]						= FALSE;
		m_apVerletPoolME[i]							= NULL;

		m_apVerletPool[i] = fnew CFVerlet();
		if( m_apVerletPool[i] == NULL ) {
			DEVPRINTF( "CVehicleLoader::InitSystem():  Error allocating verlet pool\n" );
			goto _ExitWithError;
		}

		m_apVerletPool[i]->Create( 100.0f, &vDim, (u32)0 );
		m_apVerletPool[i]->SetMovedCallback( _VerletMovedCB );
		m_apVerletPool[i]->SetDeactivateCallback( _VerletDeactivatedCB );
		m_apVerletPool[i]->SetUnanchoredCollType( CFVerlet::UNANCHORED_COLL_TYPE_KDOP );
		m_apVerletPool[i]->m_nUser = i;
		m_apVerletPool[i]->EnableDynamicObjColl( TRUE );
		m_apVerletPool[i]->SetCollisionCallback( _VerletTrackerCB );
	}

	m_hExhaustParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Loader.pszExhaustParticle );
	if( m_hExhaustParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CVehicleLoader::InitSystem():  Error loading engine exhaust particle %s\n", m_BotInfo_Loader.pszExhaustParticle );
		goto _ExitWithError;
	}

	m_hEngineStopParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Loader.pszEngineStopParticle );
	if( m_hEngineStopParticleDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CVehicleLoader::InitSystem():  Error loading engine stop particle %s\n", m_BotInfo_Loader.pszEngineStopParticle );
		goto _ExitWithError;
	}


	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( frame );

	return FALSE;
}


void CVehicleLoader::ClassHierarchyUnloadSharedResources( void ) {
    FASSERT( m_nBotClassClientCount > 0 );

	m_nBotClassClientCount--;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}


	// clean up verlet objects...
	for( u32 i=0; i<_VERLET_COUNT; i++ ) {
		if( m_apVerletPool[i] != NULL ) {
			fdelete( m_apVerletPool[i] );
			m_apVerletPool[i] = NULL;
		}
	}

	CVehicle::ClassHierarchyUnloadSharedResources();
}


void CVehicleLoader::UninitSystem( void ) {

	m_bSystemInitialized = FALSE;
}


//void CVehicleLoader::UninitLevel( void ) {
//	fang_MemZero( &m_BotInfo_LoaderSounds, sizeof( m_BotInfo_LoaderSounds ) );
//}


//BOOL CVehicleLoader::_InitSounds( void ) {
//	FASSERT( m_bSystemInitialized );
//
//	if( m_bSoundsInitialized ) {
//		return TRUE;
//	}
//
//	// Load the sound effects bank for this bot...
//	if( !fresload_Load( FSNDFX_RESTYPE, _SFXBANK_LOADER ) ) {
//		DEVPRINTF( "CVehicleLoader::_InitSounds(): Could not load sound effect bank '%s'\n", _SFXBANK_LOADER );
//	}
//
//	// load the heavy bots too, for power up/down sounds
//	if( !fresload_Load( FSNDFX_RESTYPE, _SFXBANK_HEAVY ) ) {
//		DEVPRINTF( "CVehicleLoader::_InitSounds(): Could not load sound effect bank '%s'\n", _SFXBANK_HEAVY );
//	}
//
//	//load weapon sounds too for the gun
//	if( !fresload_Load( FSNDFX_RESTYPE, _SFXBANK_HEAVY ) ) {
//		DEVPRINTF( "CVehicleLoader::_InitSounds(): Could not load sound effect bank '%s'\n", _SFXBANK_WEAPON );
//	}
//
//	if( !fgamedata_ReadFileUsingMap( m_aSoundDataMap, _BOTINFO_FILENAME ) ) {
//		return FALSE;
//	}
//
//	m_bSoundsInitialized = TRUE;
//	return TRUE;
//}



///////////////////////////////////////
//CVehicleLoader

CVehicleLoader::CVehicleLoader() {
	_ClearDataMembers();
}


CVehicleLoader::~CVehicleLoader() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CVehicleLoader::Create( s32 nPlayerIndex/*=-1*/, BOOL bInstallDataPort/*=FALSE*/, cchar *pszEntityName/*=NULL*/, const CFMtx43A *pMtx/*=NULL*/, cchar *pszAIBuilderName/*=NULL*/ ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		FASSERT_NOW;
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CVehicleLoaderBuilder *pBuilder = (CVehicleLoaderBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_VehicleLoaderBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_VEHICLELOADER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CVehicle::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CVehicleLoader::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	FMeshInit_t meshInit;
	u32 i;
	s32 nBoneIndex;
	FMesh_t* pMesh = NULL;		//CPS 4.7.03
	
	FResFrame_t frame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CVehicleLoaderBuilder *pBuilder = (CVehicleLoaderBuilder*)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CVehicle::ClassHierarchyBuild() ){
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();
	SetMaxUnitSpeed( 1.0f, FALSE /*bInternalOnly*/ );

	// We'll handle our own impulse application...
	FMATH_SETBITMASK( m_uVehicleFlags, VEHICLE_FLAG_NO_DMG_IMPULSE );

	// Initialize from builder object...
	m_bGunEnabled = pBuilder->m_bEnableGun;

	// Load the mesh...
	m_pWorldMesh = fnew CFWorldMesh();
	if( m_pWorldMesh == NULL) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild():  Out of memory creating worldmesh\n" );
		goto _ExitWithError;
	}

//CPS 4.7.03	FMesh_t* pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _pszMeshFile );
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _pszMeshFile );
	if( pMesh == NULL) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild():  Could not load mesh %s\n", _pszMeshFile );
		goto _ExitWithError;
	}

	meshInit.pMesh		= pMesh;
	meshInit.nFlags		= 0;
	meshInit.fCullDist	= FMATH_MAX_FLOAT;
	meshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );

	//// ME:  insane, but here goes...
	//meshInit.Mtx.m_vRight.Cross( meshInit.Mtx.m_vUp, meshInit.Mtx.m_vFront );
	//m_MtxToWorld = meshInit.Mtx;

	m_pWorldMesh->Init( &meshInit );

	m_pWorldMesh->m_nUser	= MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser	= this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();

	// create and initialize our animations...
	m_pAnimCombiner	= fnew CFAnimCombiner();
	if( m_pAnimCombiner == NULL ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild():   Out of memory creating CFAnimCombiner\n" );
		goto _ExitWithError;
	}

	//ok, this seems a little shaky, but vehicle needs this
	m_Anim.m_pAnimCombiner = m_pAnimCombiner;

	m_pAnimManFrame = fnew CFAnimManFrame();
	if( m_pAnimManFrame == NULL ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild():   Out of memory creating CFAnimManFrame\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimManFrame->Create( ANIMBONE_COUNT, m_apszAnimBoneNames ) ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild():   Could not create CFAnimManFrame\n" );
		goto _ExitWithError;
	}

	m_pAnimManFrame->Reset();
	if( !m_pAnimCombiner->CreateSimple( m_pAnimManFrame, m_pWorldMesh ) ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild():   Unable to init anim combiner\n" );
		goto _ExitWithError;
	}

	// find tag points
	m_nTagPointCount = 0;
	for( i=0; i<TAG_POINT_COUNT; i++ ) {
		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );
		if( nBoneIndex < 0 ) {
			DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Could not locate TagPoint bone '%s'.\n", m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );
		} else {
			m_apTagPoint_WS[m_nTagPointCount++] = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		}
	}

	if( m_nTagPointCount == 0 ) {
		// couldn't find one, just use origin
		m_nTagPointCount = 1;
		m_apTagPoint_WS[0] = &m_MtxToWorld.m_vPos;
	}

	// find our approx eye pt...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PRIMARY_FIRE] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_PRIMARY_FIRE ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	//no torso, use mtxt to world
	m_pAISteerMtx = &m_MtxToWorld;

	// init our mtx palette...
	for( i=0; i<m_pWorldMesh->m_pMesh->nBoneCount; i++ ) {
		if( m_pWorldMesh->m_pMesh->pBoneArray[i].nFlags & FMESH_BONEFLAG_SKINNEDBONE ) { 
			*m_pWorldMesh->GetBoneMtxPalette()[i] = m_pWorldMesh->m_Xfm.m_MtxF;
		} else {
			m_pWorldMesh->GetBoneMtxPalette()[i]->Mul( m_pWorldMesh->m_Xfm.m_MtxF, m_pWorldMesh->m_pMesh->pBoneArray[i].AtRestBoneToModelMtx );
		}
	}

	// get some bones...
	m_nBoneIdxEngineLeft = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ENGINE_L] );
	if( m_nBoneIdxEngineLeft < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_ENGINE_L ] );
		goto _ExitWithError;
	}

	m_nBoneIdxEngineRight = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ENGINE_R] );
	if( m_nBoneIdxEngineLeft < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_ENGINE_R ] );
		goto _ExitWithError;
	}

	m_nBoneIdxClawLeft = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_CLAW_L] );
	if( m_nBoneIdxEngineLeft < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_CLAW_L ] );
		goto _ExitWithError;
	}

	m_nBoneIdxClawRight = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_CLAW_R] );
	if( m_nBoneIdxEngineLeft < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_CLAW_R ] );
		goto _ExitWithError;
	}

	m_nBoneIdxClawArm = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_CLAW_ARM ] );
	if( m_nBoneIdxEngineLeft < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_CLAW_ARM ] );
		goto _ExitWithError;
	}

	m_nBoneIdxDriverAttach = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_DRIVER_ATTACH] );
	if( m_nBoneIdxDriverAttach < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_DRIVER_ATTACH ] );
		goto _ExitWithError;
	}

	m_nBoneIdxVictimAttach = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_VICTIM_ATTACH] );
	if( m_nBoneIdxVictimAttach < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_VICTIM_ATTACH ] );
		goto _ExitWithError;
	}

	m_nBoneIdxPriFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PRIMARY_FIRE] );
	if( m_nBoneIdxPriFire < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_PRIMARY_FIRE ] );
		goto _ExitWithError;
	}

	m_nBoneIdxGun = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GUN] );
	if( m_nBoneIdxGun < 0 ) {
		DEVPRINTF( "CVehicleLoader::ClassHierarchyBuild(): Critical error finding bone '%s'.\n", m_apszBoneNameTable[ BONE_GUN ] );
		goto _ExitWithError;
	}

	m_pAnimCombiner->DisableAllBoneCallbacks();
	if( !m_bGunEnabled ) {
		m_pAnimCombiner->EnableBoneCallback( m_nBoneIdxGun );
		m_pAnimCombiner->SetBoneCallback( _AnimBoneCallback );
		
	}



	// initialize engine positions
	m_AnimFrameEngineLeft.BuildQuat( m_pWorldMesh->m_pMesh->pBoneArray[m_nBoneIdxEngineLeft].AtRestBoneToParentMtx );
	m_AnimFrameEngineRight.BuildQuat( m_pWorldMesh->m_pMesh->pBoneArray[m_nBoneIdxEngineRight].AtRestBoneToParentMtx );

	// initialize our particles
	fparticle_SetCfgToDefaults( &m_ExhaustPartCfgLeft );
	m_ExhaustPartCfgLeft.bEmitShapeWillMove		= TRUE;

	m_ExhaustPartCfgLeft.pEmitterVelocityPerSec	= &(m_Velocity_WS.v3);
	m_ExhaustPartCfgLeft.pfUnitIntensity		= &m_fExhaustPartIntensityLeft;
	m_ExhaustPartCfgLeft.pEmitShape				= &m_ExhaustPartShapeLeft;
	m_ExhaustPartShapeLeft.nShapeType			= FPARTICLE_SHAPETYPE_CYLINDER;
	m_ExhaustPartShapeLeft.Cylinder.pCenterBase = &(m_vExhaustPartPosLeft);
	m_ExhaustPartShapeLeft.Cylinder.pfHeight	= &(m_fExhaustParticleHeight);
	m_ExhaustPartShapeLeft.Cylinder.pfRadius	= &(m_fExhaustPartRadiusLeft);
	m_ExhaustPartShapeLeft.Cylinder.pNorm		= &(m_vExhaustPartDirLeft);

	fparticle_SetCfgToDefaults( &m_ExhaustPartCfgRight );
	m_ExhaustPartCfgRight.bEmitShapeWillMove = TRUE;

	m_ExhaustPartCfgRight.pEmitterVelocityPerSec	= &(m_Velocity_WS.v3);
	m_ExhaustPartCfgRight.pfUnitIntensity			= &m_fExhaustPartIntensityRight;
	m_ExhaustPartCfgRight.pEmitShape				= &m_ExhaustPartShapeRight;
	m_ExhaustPartShapeRight.nShapeType				= FPARTICLE_SHAPETYPE_CYLINDER;
	m_ExhaustPartShapeRight.Cylinder.pCenterBase	= &(m_vExhaustPartPosRight);
	m_ExhaustPartShapeRight.Cylinder.pfHeight		= &(m_fExhaustParticleHeight);
	m_ExhaustPartShapeRight.Cylinder.pfRadius		= &(m_fExhaustPartRadiusRight);
	m_ExhaustPartShapeRight.Cylinder.pNorm			= &(m_vExhaustPartDirRight);


	_InitPhysicsObject();

	//useful for AI
	m_fMaxFlatSurfaceSpeed_WS	= m_BotInfo_VehiclePhysics.fMaxSpeed;
	m_fMaxVerticalSpeed_WS		= 0.0f;


	// player uses action button to enter & exit vehicle
	SetActionable( TRUE );

	fforce_NullHandle( &m_hForce );

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;

}


void CVehicleLoader::ClassHierarchyDestroy( void ) {
	fforce_Kill( &m_hForce );

	if( m_pWorldMesh != NULL ) {
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	if( m_pAnimManFrame != NULL ) {
		fdelete( m_pAnimManFrame );
		m_pAnimManFrame = NULL;
	}

	if( m_pAnimCombiner != NULL ) {
		fdelete( m_pAnimCombiner );
		m_pAnimCombiner = NULL;
	}

	CVehicle::ClassHierarchyDestroy();
}


void CVehicleLoader::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	m_uBotDeathFlags &= (~BOTDEATHFLAG_PLAYDEATHANIM);
	m_uBotDeathFlags |= BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;

	CVehicle::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	f32 fYaw_WS = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
	ChangeMountYaw( fYaw_WS );
	
	m_fEngineDisruptTimer = 0.0f;
}


void CVehicleLoader::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();

	CVehicle::ClassHierarchyRemoveFromWorld();

	ExitStation( m_pDriverBot, TRUE /*bImmediately*/ );

	fforce_Kill( &m_hForce );

	if( m_hExhaustPartEmitterLeft != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hExhaustPartEmitterLeft );
		m_hExhaustPartEmitterLeft = FPARTICLE_INVALID_HANDLE;
	}

	if( m_hExhaustPartEmitterRight != NULL ) {
		fparticle_KillEmitter( FPARTICLE_INVALID_HANDLE );
		m_hExhaustPartEmitterRight = FPARTICLE_INVALID_HANDLE;
	}

	if( m_pAudioEmitterEngHigh ) {
		m_pAudioEmitterEngHigh->Destroy();
		m_pAudioEmitterEngHigh = NULL;
	}

	if( m_pAudioEmitterEngLow ) {
		m_pAudioEmitterEngLow->Destroy();
		m_pAudioEmitterEngLow = NULL;
	}
}


CEntityBuilder *CVehicleLoader::GetLeafClassBuilder( void ) {
	return &_VehicleLoaderBuilder;
}


BOOL CVehicleLoader::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t frame = fres_GetFrame();

	if( !CVehicle::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CVehicleLoader::ClassHierarchyWork( void ) {
    FASSERT( m_bSystemInitialized );
	PROTRACK_BEGINBLOCK( "Loader" );

	CVehicle::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		PROTRACK_ENDBLOCK(); //"Loader"
		return;
	}

	if( gamecam_IsInDebugMode() && m_pDriverBot && !m_pDriverBot->IsWorkEnabled() ) {
		// we are in debug cam mode while the bot is driving us, don't do work
		PROTRACK_ENDBLOCK(); //"Loader"
		return;
	}

	ParseControls();

	ComputeXlatStickInfo();

	PROTRACK_BEGINBLOCK( "ComputeNormal" );
	ComputeGroundNormal();
	PROTRACK_ENDBLOCK(); //CGN

	if (m_pDriverBot && IsDriverUseless(m_pDriverBot))	{
		ExitStation(m_pDriverBot, FALSE /*bImmediately*/);
	}

	if( m_fEngineDisruptTimer > 0.0f ) {
		m_PhysObj.SetAngularDamping( _TUMBLE_DAMPING );

		m_fEngineDisruptTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_MIN0( m_fEngineDisruptTimer );
	} else {
		m_PhysObj.SetAngularDamping( m_BotInfo_VehiclePhysics.fAngularDampingLow );	
	}

	if( (m_fEngineDisruptTimer == 0.0f) && (m_eEngineState == ENGINE_STATE_RUNNING) ) {
		PROTRACK_BEGINBLOCK( "Yaw & Pitch" );
			HandleYawMovement();
			HandlePitchMovement();
		PROTRACK_ENDBLOCK();

		PROTRACK_BEGINBLOCK( "Control Force" );
			_ApplyControlForces();
		PROTRACK_ENDBLOCK();

		PROTRACK_BEGINBLOCK( "Hover force" );
			_ApplyHoverForce();
		PROTRACK_ENDBLOCK();

		PROTRACK_BEGINBLOCK( "ApplyTorque" );
			_ApplyStabilizingTorque();
		PROTRACK_ENDBLOCK();
	}

	PROTRACK_BEGINBLOCK( "UpdateMatrices" );
	UpdateMatrices();
	PROTRACK_ENDBLOCK();

	_ClawWork();
	_EngineWork();
	
	_HandleDriverState();

	_CameraTransitionWork();

	PROTRACK_BEGINBLOCK( "PhysicsSim" );
		m_pCBLoader = this;
		f32 fClampedDT = FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( fClampedDT, 1.0f / 30.0f );
		m_PhysObj.Simulate( fClampedDT );
		m_pCBLoader = NULL;
	PROTRACK_ENDBLOCK();

	// update velocity member variables
	m_Velocity_WS.Set( *(m_PhysObj.GetVelocity()) ); //world
	//now set model space as well
	CFMtx43A mInvOr;
	mInvOr.Set( *(m_PhysObj.GetOrientation()) );
	mInvOr.AffineInvert( FALSE );
	mInvOr.MulPoint( m_Velocity_MS, m_Velocity_WS );
	VelocityHasChanged();


	if( m_pDriverBot &&
		HasHumanOrBotControls() &&
		m_bControls_Action ) {
		ActionNearby( m_pDriverBot );
	}

	HandleTargeting();

	_WeaponWork();

	_SoundWork();


	// send an AI Sound radius to AI so brains can "hear" vehicle coming and try to dodge it
	if (m_pDriverBot && m_pDriverBot->m_nPossessionPlayerIndex > -1)
	{
		f32 fSpeed = m_fSpeed_WS;
		f32 fFakeMaxSpeed = 75.0f;
		FMATH_CLAMPMAX(fSpeed, fFakeMaxSpeed);	//some arbitrary max speed just used to approximate get a rough normalized speed
		AIEnviro_BoostPlayerSoundTo(m_pDriverBot->m_nPossessionPlayerIndex, 50.0f+ AIEnviro_fVehicleBotSoundRadius*fmath_Div(fSpeed, fFakeMaxSpeed));
	}

	if( m_pDriverBot && IsUpsideDown() ) {
		ExitStation( m_pDriverBot, FALSE /*bImmediately*/ );
	}



	if( m_pCameraBot ) {
		EnableCameraCollision( FALSE );
	} else {
		EnableCameraCollision( TRUE );
	}


	if( m_fFlipVehicleTimer > 0.0f ) {
		m_fFlipVehicleTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_MIN0( m_fFlipVehicleTimer );

		CFVec3A vPt, vForce;
		vPt.Sub( m_vFlipPosition, *(m_PhysObj.GetPosition()) );
		f32 fMag2 = vPt.MagSq();
		if( fMag2 < 0.01f ) {
			vPt = m_MtxToWorld.m_vFront;
		} else {
			vPt.Mul( fmath_InvSqrt( fMag2 ) );
		}
		vPt.Mul( 10.0f );

		vForce.Set( 0.0f, 75000.0f, 0.0f );

		m_PhysObj.ApplyForce( &vForce, &vPt );
	}




#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	//temp...
	CFVec3A vForce;

	if( m_bControls_Jump ) {
		vForce.Set( 0.0f, 1000000.0f, 0.0f );
		m_PhysObj.ApplyForce( &vForce, &m_vCMOffset );
	}

	m_Velocity_WS = *(m_PhysObj.GetVelocity());
#endif

	for( u32 i=0; i<_VERLET_COUNT; i++ ) {
		if( m_abVerletMarkedForDeactivate[i] ) {
			_VerletDeactivate( i );
		}
	}

	if( m_eStationStatus == STATION_STATUS_EMPTY && m_PhysObj.IsContacting() )
	{
		m_PhysObj.SetMomentumClamping( TRUE, 1200.0f, 3300.0f );
		m_PhysObj.SetAngularMomentumClamping( TRUE, 3600.0f, 10000.0f );
	}
	else
	{
		m_PhysObj.SetMomentumClamping( FALSE );
		m_PhysObj.SetAngularMomentumClamping( FALSE );
	}

	PROTRACK_ENDBLOCK(); //"Loader"
	return;
}


void CVehicleLoader::CheckpointRestore( void ) {
	CVehicle::CheckpointRestore();

	CFCheckPoint::LoadData( (u32&) m_pDriverBot );

	// load the saved state of the pool of verlet objects...
	for( u32 i=0; i<_VERLET_COUNT; i++ ) {
		//NOTE : Probably need to restore the state of the
		// companion verlet arrays as well...
		if( m_apVerletPool[i] ) {
			m_apVerletPool[i]->CheckPointLoad();
		}
	}

	fforce_Kill( &m_hForce );

	_InitPhysicsObject();
	
	m_pClawedEntity = NULL;
	m_ClawState = CLAWSTATE_OPEN;
}


void CVehicleLoader::CheckpointRestorePostamble( void )
{
	CVehicle::CheckpointRestorePostamble();

	if( m_pDriverBot ) {
		CBot *pBot;
		pBot = m_pDriverBot;
		m_pDriverBot = NULL;
		EnterStation( pBot, STATION_DRIVER, FALSE /*bProximityCheck*/, TRUE /*bImmediately*/ );
	}
}


BOOL CVehicleLoader::CheckpointSave( void ) {
	CVehicle::CheckpointSave();

	CFCheckPoint::SaveData( (u32&) m_pDriverBot );

	//save the state of the pool of verlet objects.
	for( u32 i=0; i<_VERLET_COUNT; i++ ) {
		//NOTE : Probably need to save the state of the
		// companion verlet arrays as well...
		if( m_apVerletPool[i] ) {
			m_apVerletPool[i]->CheckPointSave();
		}
	}

	return TRUE;
}


void CVehicleLoader::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


void CVehicleLoader::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );

	FASSERT( (nTrackerSkipListCount + 1) < FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
	
	if( m_pDriverBot ) {
		m_pDriverBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pCameraBot && (m_pCameraBot != m_pDriverBot) ) {

		m_pCameraBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( ClawIsClosed() && m_pClawedEntity ) {
		m_pClawedEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}


const CFVec3A* CVehicleLoader::GetTagPoint( u32 nTagPointIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nTagPointIndex < m_nTagPointCount );

	return m_apTagPoint_WS[nTagPointIndex];
}


const CFVec3A* CVehicleLoader::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CVehicleLoader::ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS ) {
	// apply a force to get this effect
	CFVec3A vForce;
	CFVec3A vForceAppPos;

	vForceAppPos.Mul( CFVec3A::m_UnitAxisY, 3.0f );
	f32 fdt = FLoop_fPreviousLoopOOSecs;
	FMATH_CLAMPMIN( fdt, 1.0f/60.0f);

	vForce.Mul( rVelocityImpulseVec_WS, 2.0f * m_PhysObj.GetMass() * fdt );
	m_PhysObj.ApplyForce( &vForce, &vForceAppPos );
}

//-----------------------------------------------------------------------------
// Call when m_Velocity_WS and m_Velocity_MS have changed.
void CVehicleLoader::VelocityHasChanged( void )
{
	// Update m_VelocityXZ_WS...
	m_VelocityXZ_WS = m_Velocity_WS;
	m_VelocityXZ_WS.y = 0.0f;

	// Update m_NormVelocityXZ_WS...
	m_NormVelocityXZ_WS = m_Velocity_WS;
	m_NormVelocityXZ_WS.y = 0.0f;

	// Compute speeds...
	m_fSpeed_WS = m_Velocity_WS.Mag();
	m_fUnitSpeed_WS = fmath_Div( m_fSpeed_WS, m_BotInfo_VehiclePhysics.fMaxSpeed );
	m_fSpeedXZ_WS = m_Velocity_WS.MagXZ();
	
	m_fNormSpeedXZ_WS = fmath_Div(  m_fSpeedXZ_WS, m_BotInfo_VehiclePhysics.fMaxSpeed );
	m_fClampedNormSpeedXZ_WS = m_fNormSpeedXZ_WS;
	FMATH_CLAMPMAX( m_fClampedNormSpeedXZ_WS, 1.0f );

	// Update m_UnitVelocity_WS...
	if( m_fSpeed_WS > 0.0f )
	{
		m_UnitVelocity_WS.Div( m_Velocity_WS, m_fSpeed_WS );
	}
	else
	{
		m_UnitVelocity_WS.Zero();
	}

	// Update m_UnitVelocityXZ_WS...
	if( m_fSpeedXZ_WS > 0.0f )
	{
		m_UnitVelocityXZ_WS.Div( m_VelocityXZ_WS, m_fSpeedXZ_WS );
	}
	else
	{
		m_UnitVelocityXZ_WS.Zero();
	}
}


void CVehicleLoader::_ClearDataMembers( void ) {
	m_pWorldMesh					= NULL;
	m_pMoveIdentifier				= &m_apTagPoint_WS[0];

	m_fCollCylinderHeight_WS		= 8.0f;
	m_fCollCylinderRadius_WS		= 8.0f;

	m_pBotInfo_Gen					= &m_BotInfo_Gen;
	m_pBotInfo_MountAim				= &m_BotInfo_MountAim;
	m_pBotInfo_Vehicle				= &m_BotInfo_Vehicle;
	m_pBotInfo_VehiclePhysics		= &m_BotInfo_VehiclePhysics;
	m_pBotInfo_Engine				= NULL;	// not needed unless calling UpdateEngineSounds()
	m_pBotInfo_Walk					= NULL;
	if( MultiplayerMgr.IsMultiplayer() )
	{
		m_pBotInfo_DriverCamera		= &m_BotInfo_MPDriverCamera;
	}
	else
	{
		m_pBotInfo_DriverCamera		= &m_BotInfo_DriverCamera;
	}

	m_pAnimCombiner					= NULL;
//	m_pAnimManMtx					= NULL;
	m_pAnimManFrame					= NULL;
	m_pDriverBot					= NULL;

	m_fMountPitchMax_WS 			= m_BotInfo_MountAim.fMountPitchDownLimit;
	m_fMountPitchMin_WS 			= m_BotInfo_MountAim.fMountPitchUpLimit;

	m_ClawState						= CLAWSTATE_OPEN;
	m_fClawedEntitySpan				= 0.0f;

	m_fMaxUnitClawAngle				= 1.0f;
	m_fClawLockUnitTimer			= 0.0f;
	m_fUnitClawAngle				= 0.0f;
	m_fLastUnitClawAngle			= 0.0f;
	m_fClawLockUnitTimer			= 0.0f;
	m_fClawPressure					= 0.0f;
	m_pClawedEntity					= NULL;

	m_hExhaustPartEmitterLeft		= FPARTICLE_INVALID_HANDLE;
	m_hExhaustPartEmitterRight		= FPARTICLE_INVALID_HANDLE;

	m_vExhaustPartPosLeft.Zero();
	m_vExhaustPartDirLeft			= CFVec3A::m_UnitAxisY;
	m_fExhaustPartIntensityLeft		= 0.0f;
	m_fExhaustPartRadiusLeft		= 2.0f;

	m_vExhaustPartPosRight.Zero();
	m_vExhaustPartDirRight			= CFVec3A::m_UnitAxisY;
	m_fExhaustPartIntensityRight	= 0.0f;
	m_fExhaustPartRadiusRight		= 1.0f;

	m_fEngineDisruptTimer			= 0.0f;

	m_vForceApplied.Zero();
	m_fTorqueApplied = 0.0f;


	m_AnimFrameEngineLeft.Identity();
	m_AnimFrameEngineRight.Identity();


	// weapon...
	m_fGunFireTimer					= 0.0f;
	m_fGunUnitShotSpread			= 0.0f;
	m_fGunAISoundTimer				= 0.0f;
	m_fGunTracerTimer				= 0.0f;
	m_TargetedPoint_WS.Zero();
	m_TargetLockUnitVec_WS			= CFVec3A::m_UnitAxisY;
	m_bGunReady						= TRUE;
	m_qLastGun.Identity();

	m_eCameraState					= CAMERA_STATE_INACTIVE;
	m_eStationStatus				= STATION_STATUS_EMPTY;
	m_eEngineState					= ENGINE_STATE_STOPPED;

	m_fEngineVibTimerLeft			= 0.0f;
	m_fEngineVibTimerRight			= 0.0f;
	m_fEngineVibMagLeft				= 0.0f;
	m_fEngineVibMagRight			= 0.0f;
	m_uEngineVibCtrLeft				= 0;
	m_uEngineVibCtrRight			= 0;

	m_pAudioEmitterEngHigh			= NULL;
	m_pAudioEmitterEngLow			= NULL;
	m_b3DSounds						= FALSE;
	m_fEngineLoad					= 0.0f;		
	m_fLastEngineLoad				= 0.0f;
	m_fEngineLoadAccum				= 0.0f;
	m_fFlipVehicleTimer				= 0.0f;

	m_pCameraBot = NULL;
}


BOOL CVehicleLoader::_AltitudeTrackerCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	for( u32 i=0; i < FWorld_nTrackerSkipListCount; i++ ) {
		if( FWorld_apTrackerSkipList[i] == pTracker ) {
			return TRUE;
		}
	}

	// m_CollInfo should still be set from _CheckAltitude

	((CFWorldMesh*)pTracker)->CollideWithMeshTris( &m_CollInfo );
	return TRUE;
}


//static CFSphere _dbgspheres[9];
//static u32		_dbgspherectr = 0;

f32	 CVehicleLoader::_CheckAltitude( const CFVec3A &vPt, const CFVec3A &vDir, f32 fMaxLen, CFVec3A *pvGroundPt, CFVec3A *pvGroundNormal ) {

	CFCollData collData;
	CFVec3A vMovement;
	CFVec3A vColPtFar;
	CFVec3A vImpactPt	= CFVec3A::m_Null;
	CFVec3A vImpactNorm	= CFVec3A::m_Null;


	vMovement.Mul( vDir, fMaxLen );
	CFSphere sphere;
	sphere.Set( vPt.v3, _ALTCHECK_COLLSPHERE_RADIUS );

	//DBG
	//if( _dbgspherectr == 9 ) {
	//	_dbgspherectr = 0;
	//}
	//_dbgspheres[_dbgspherectr++] = sphere;
	//_dbgspheres[_dbgspherectr] = sphere;
	//_dbgspheres[_dbgspherectr++].m_Pos.Add( vMovement.v3 );
	//_dbgspheres[_dbgspherectr] = sphere;
	//END DBG

	FWorld_nTrackerSkipListCount = 0;
	fcoll_Clear();
	AppendTrackerSkipList();

	collData.nCollMask = FCOLL_MASK_CHECK_ALL;
	collData.nFlags = FCOLL_DATA_IGNORE_BACKSIDE;
	collData.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	collData.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	collData.pCallback = NULL;//_IntersectingTrackerCallback;
	collData.pLocationHint = m_pWorldMesh;
	collData.pMovement = &vMovement;
	collData.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	collData.ppTrackerSkipList = FWorld_apTrackerSkipList;

	fcoll_Check( &collData, &sphere );

	if( FColl_nImpactCount > 0 ) {
		fcoll_SortDynamic( TRUE );

		if( pvGroundPt ) {
            *pvGroundPt = FColl_apSortedImpactBuf[0]->ImpactPoint;
		}

		if( pvGroundNormal ) {
			*pvGroundNormal = FColl_apSortedImpactBuf[0]->UnitFaceNormal;
		}

		////DBG
		//vMovement.Mul( vDir, FColl_apSortedImpactBuf[0]->fImpactDistInfo /** fMaxLen*/ );
		//_dbgspheres[_dbgspherectr++].m_Pos.Add( vMovement.v3 );

		return vPt.y - FColl_apSortedImpactBuf[0]->ImpactPoint.y;
	}

	////DBG
	//vMovement.Mul( vDir, fMaxLen );
	//_dbgspheres[_dbgspherectr++].m_Pos.Add( vMovement.v3 );

	return fMaxLen + 0.01f;
}


#if 0
	CFVec3A vColPtFar;
	CFVec3A vImpactPt	= CFVec3A::m_Null;
	CFVec3A vImpactNorm	= CFVec3A::m_Null;

	fMaxLen += 0.1f;

	vColPtFar.Mul( vDir, fMaxLen );
	vColPtFar.Add( vPt );

	// first, collide with world geo...
	fcoll_Clear();

	fang_MemZero( &m_CollInfo, sizeof( m_CollInfo ) );
	m_CollInfo.nCollTestType			= FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.bFindFirstImpactOnly		= FALSE;
	m_CollInfo.bFindClosestImpactOnly	= FALSE;
	m_CollInfo.nCollMask				= FCOLL_MASK_CHECK_ALL;
	m_CollInfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	m_CollInfo.bCullBacksideCollisions	= FALSE;
	m_CollInfo.nTrackerUserTypeBitsMask	= ~ENTITY_BIT_VEHICLELOADER;
	m_CollInfo.pTag						= NULL;
	m_CollInfo.bCalculateImpactData		= TRUE;
	m_CollInfo.ProjSphere.Init( &vPt, &vColPtFar, _ALTCHECK_COLLSPHERE_RADIUS );
	m_CollInfo.pSphere_WS				= NULL;


	fcoll_Clear();

	if( fworld_CollideWithWorldTris( &m_CollInfo, m_pWorldMesh ) ) { 
		vImpactPt	= FColl_aImpactBuf[0].ImpactPoint;
		vImpactNorm = FColl_aImpactBuf[0].UnitFaceNormal;
		fMaxLen		= vPt.y - vImpactPt.y;
	}

	vColPtFar.Mul( vDir, fMaxLen ).Add( vPt );
	m_CollInfo.ProjSphere.Init( &vPt, &vColPtFar, _ALTCHECK_COLLSPHERE_RADIUS );

	// now collide with trackers...
	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();
	fcoll_Clear();

	CFTrackerCollideProjSphereInfo trackerCollInfo;
	trackerCollInfo.bIgnoreCollisionFlag		= FALSE;
	trackerCollInfo.nTrackerTypeBits			= FWORLD_TRACKERTYPEBIT_MESH;
	trackerCollInfo.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	trackerCollInfo.pCallback					= _AltitudeTrackerCB;
	trackerCollInfo.pProjSphere					= &m_CollInfo.ProjSphere;
	fworld_CollideWithTrackers( &trackerCollInfo );
	
	if( FColl_nImpactCount > 0 ) {
		fcoll_Sort( TRUE );
        
		// if we're here it's shorter?
		if( FColl_apSortedImpactBuf[0]->fImpactDistInfo < fMaxLen ) {
			vImpactPt	= FColl_apSortedImpactBuf[0]->ImpactPoint;
			vImpactNorm	= FColl_apSortedImpactBuf[0]->UnitFaceNormal;
			fMaxLen		= vPt.y - vImpactPt.y;
		}
	}

	if( pvGroundPt ) {
		*pvGroundPt = vImpactPt;
	}

	if( pvGroundNormal ) {
		*pvGroundNormal = vImpactNorm;
	}

	return fMaxLen;
}
#endif

void CVehicleLoader::StartEngine( void ) {

	FASSERT( m_hExhaustPartEmitterLeft == FPARTICLE_INVALID_HANDLE );
	FASSERT( m_hExhaustPartEmitterRight == FPARTICLE_INVALID_HANDLE );

	m_hExhaustPartEmitterLeft	= fparticle_SpawnEmitter( m_hExhaustParticleDef, &m_ExhaustPartCfgLeft );
	m_hExhaustPartEmitterRight	= fparticle_SpawnEmitter( m_hExhaustParticleDef, &m_ExhaustPartCfgRight );

	if( m_nPossessionPlayerIndex < 0 ) {
		fsndfx_Play3D( m_BotInfo_LoaderSounds.hPowerUp, &m_MountPos_WS, m_BotInfo_Loader.fSoundRadius3D, 1.0f, m_BotInfo_Loader.fSoundVolume3D );
	} else {
		fsndfx_Play2D( m_BotInfo_LoaderSounds.hPowerUp, m_BotInfo_Loader.fSoundVolume2D );
	}

	// create persistent engine sounds...
	if( m_nPossessionPlayerIndex < 0 ) {
		m_b3DSounds				= TRUE;
		m_pAudioEmitterEngLow	= FSNDFX_ALLOCNPLAY3D( m_BotInfo_LoaderSounds.hHoverLow, &m_MtxToWorld.m_vPos, m_BotInfo_Loader.fSoundRadius3D, m_BotInfo_Loader.fSoundVolume3D, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
		m_pAudioEmitterEngHigh	= FSNDFX_ALLOCNPLAY3D( m_BotInfo_LoaderSounds.hHoverHigh, &m_MtxToWorld.m_vPos, m_BotInfo_Loader.fSoundRadius3D, m_BotInfo_Loader.fSoundVolume3D, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
	} else {
		m_b3DSounds				= FALSE;
		m_pAudioEmitterEngLow	= FSNDFX_ALLOCNPLAY2D( m_BotInfo_LoaderSounds.hHoverLow, m_BotInfo_Loader.fSoundVolume2D, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
		m_pAudioEmitterEngHigh	= FSNDFX_ALLOCNPLAY2D( m_BotInfo_LoaderSounds.hHoverHigh, m_BotInfo_Loader.fSoundVolume2D, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );   
	}

	m_eEngineState = ENGINE_STATE_RUNNING;
}


void CVehicleLoader::StopEngine( BOOL bImmediately /*= FALSE*/ ) {
	FASSERT( IsCreated() );

	if( m_hExhaustPartEmitterLeft != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hExhaustPartEmitterLeft );
		m_hExhaustPartEmitterLeft = FPARTICLE_INVALID_HANDLE;
	}

	if( m_hExhaustPartEmitterRight != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hExhaustPartEmitterRight );
		m_hExhaustPartEmitterRight = FPARTICLE_INVALID_HANDLE;
	}

	if( m_nPossessionPlayerIndex < 0 ) {
		fsndfx_Play3D( m_BotInfo_LoaderSounds.hPowerDown, &m_MountPos_WS, m_BotInfo_Loader.fSoundRadius3D, 1.0f, m_BotInfo_Loader.fSoundVolume3D );
	} else {
		fsndfx_Play2D( m_BotInfo_LoaderSounds.hPowerDown, m_BotInfo_Loader.fSoundVolume2D );
	}

	// kill our engine sounds...
	if( m_pAudioEmitterEngHigh != NULL ) {
		m_pAudioEmitterEngHigh->Destroy();
		m_pAudioEmitterEngHigh = NULL;
	}
	if( m_pAudioEmitterEngLow != NULL ) {
		m_pAudioEmitterEngLow->Destroy();
		m_pAudioEmitterEngLow = NULL;
	}

	// create a little puff of black smoke (the mils should take better care of their equipment)
	if (IsInWorld() && !IsMarkedForWorldRemove())
	{
		fparticle_SpawnEmitter( m_hEngineStopParticleDef, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineLeft]->m_vPos.v3), &(CFVec3A::m_UnitAxisY.v3), &(CFVec3A::m_UnitAxisY.v3), 1.0f );
		fparticle_SpawnEmitter( m_hEngineStopParticleDef, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineRight]->m_vPos.v3), &(CFVec3A::m_UnitAxisY.v3), &(CFVec3A::m_UnitAxisY.v3), 1.0f );
	}

	m_eEngineState = ENGINE_STATE_STOPPED;
}


void CVehicleLoader::_InitPhysicsObject( void ) {
	m_PhysObj.Init();
	m_PhysObj.SetPosition( &m_MtxToWorld.m_vPos );
	m_PhysObj.SetOrientation( &m_MtxToWorld );
	m_PhysObj.SetCollisionDetectionCallback( _LoaderCollisionDetect );
	m_PhysObj.SetElasticity( 0.0f );
	m_PhysObj.SetOtherObjElasticity( 0.0f );
	m_PhysObj.SetMass( m_BotInfo_VehiclePhysics.fMass );
	m_PhysObj.SetInertialSize( 6.0f, 5.0f, 15.0f );
	m_PhysObj.SetGravity( m_BotInfo_Gen.fGravity );
	m_PhysObj.SetLinearDamping( m_BotInfo_VehiclePhysics.fLinearDampingLow );
	m_PhysObj.SetAngularDamping( m_BotInfo_VehiclePhysics.fAngularDampingLow );
	m_PhysObj.RegisterCollisionPoints( m_aColPoints, COLPOINT_COUNT );
	m_PhysObj.IgnoreFrictionPosition( TRUE );	// position vector in collision point pmFrictOr will be ignored


	m_avCollisionPts[COLPOINT_ENGINE_LEFT].Set( -6.0f, 4.0f, 0.0f );
	m_avCollisionPts[COLPOINT_ENGINE_RIGHT].Set( 6.0f, 4.0f, 0.0f );
	m_avCollisionPts[COLPOINT_BODY].Set( 0.0f, 5.0f, -0.5f );
	m_avCollisionPts[COLPOINT_FRONT].Set( 0.0f, 3.0f, 5.5f );

	m_avCollisionPts[COLPOINT_CLAW_LEFT].Set( -3.25f, 2.0f, 7.5f );
	m_avCollisionPts[COLPOINT_CLAW_RIGHT].Set( 3.25f, 2.0f, 7.5f );
	m_avCollisionPts[COLPOINT_CLAW_CENTER].Set( 0.0f, 2.0f, 6.0f );
	
	m_afCollisionPtsRadius[COLPOINT_ENGINE_LEFT]	= 3.5f;
	m_afCollisionPtsRadius[COLPOINT_ENGINE_RIGHT]	= 3.5f;
	m_afCollisionPtsRadius[COLPOINT_BODY]			= 4.0f;
	m_afCollisionPtsRadius[COLPOINT_FRONT]			= 3.0f;

	m_afCollisionPtsRadius[COLPOINT_CLAW_LEFT]		= 2.0f;
	m_afCollisionPtsRadius[COLPOINT_CLAW_RIGHT]		= 2.0f;
	m_afCollisionPtsRadius[COLPOINT_CLAW_CENTER]	= 2.5f;

	// init our collision points
	for( u32 i=0; i<COLPOINT_COUNT; i++ ) {
		CFPhysicsObject::InitCollisionPoint( &m_aColPoints[i] );
		m_aColPoints[i].uResultFlags			= 0;
		m_aColPoints[i].fTimeSinceCollision		= _FRICTION_CONTACT_TIME + 1.0f;	// set so friction isn't applied until after first collision
		m_aColPoints[i].fStaticHysteresis		= _STATIC_TRANS_HYST;
		m_aColPoints[i].fFrictionSmoothTime		= _FRICTION_CONTACT_TIME;
		m_aColPoints[i].pmFrictOr				= NULL;
		m_aColPoints[i].fAppliedForceKineticMul	= _KINETIC_FORCE_MUL;
		m_aColPoints[i].fKineticTransition		= 0.0f;
//		m_aColPoints[i].vAppliedForce.Set( m_BotInfo_VehiclePhysics.fBodyFriction, 
//										   m_BotInfo_VehiclePhysics.fBodyFriction, 
//										   m_BotInfo_VehiclePhysics.fBodyFriction );
		m_aColPoints[i].vAppliedForce.Zero();
		m_aColPoints[i].vStaticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, 
											 m_BotInfo_VehiclePhysics.fBodyFriction, 
											 m_BotInfo_VehiclePhysics.fBodyFriction );
		m_aColPoints[i].vKineticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, 
											  m_BotInfo_VehiclePhysics.fBodyFriction, 
											  m_BotInfo_VehiclePhysics.fBodyFriction );
	}
}


void CVehicleLoader::_GetColSphere( u32 uID, CFSphere *pSphere, const CFMtx43A &mtx ) {
	FASSERT( uID < COLPOINT_COUNT );
	pSphere->m_Pos = m_avCollisionPts[uID].v3;

	if( uID == COLPOINT_CLAW_LEFT ) {
		CFVec3A vTmp;
		f32 fDist = ClawIsClosed() ? 1.0f : m_fUnitClawAngle;
		vTmp.Mul(  CFVec3A::m_UnitAxisZ, _CLAW_ARM_THROW * fDist ); //= m_vClawTipPosLeft_MS;
		vTmp.Add( m_avCollisionPts[uID] );
		mtx.MulPoint( pSphere->m_Pos, vTmp.v3 );
		pSphere->m_fRadius = m_afCollisionPtsRadius[uID];
	} else if( uID == COLPOINT_CLAW_RIGHT ) {
		CFVec3A vTmp;
		f32 fDist = ClawIsClosed() ? 1.0f : m_fUnitClawAngle;
		vTmp.Mul(  CFVec3A::m_UnitAxisZ, _CLAW_ARM_THROW * fDist ); //= m_vClawTipPosLeft_MS;
		vTmp.Add( m_avCollisionPts[uID] );
		mtx.MulPoint( pSphere->m_Pos, vTmp.v3 );
		pSphere->m_fRadius = m_afCollisionPtsRadius[uID];
	} else if( uID == COLPOINT_CLAW_CENTER ) {
		CFVec3A vTmp;
		f32 fDist = ClawIsClosed() ? 1.0f : m_fUnitClawAngle;
		vTmp.Mul( CFVec3A::m_UnitAxisZ, _CLAW_CENTER_THROW * fmath_Sqrt( fDist )/* * fDist*/ );
		vTmp.Add( m_avCollisionPts[COLPOINT_CLAW_CENTER] );
        mtx.MulPoint( pSphere->m_Pos, vTmp.v3 );
		pSphere->m_fRadius = m_afCollisionPtsRadius[uID];
	} else if( uID == COLPOINT_OBJECT ) {
		if( m_pClawedEntity && (m_pClawedEntity->TypeBits() & ENTITY_BIT_MESHENTITY) ) {
			*pSphere = ((CMeshEntity*)m_pClawedEntity)->GetMeshInst()->m_pMesh->BoundSphere_MS;
			pSphere->m_fRadius *= _CLAWED_COLSPHERE_RAD_MULT;
			//pSphere->m_Pos.Add( m_pClawedEntity->MtxToWorld()->m_vPos.v3 );
			pSphere->m_Pos.Add( m_vClawedEntityDesiredPos.v3 );
		} else if( m_pClawedEntity && (m_pClawedEntity->TypeBits() & ENTITY_BIT_BOT) ) {
			pSphere->m_Pos		= m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos.v3;
			pSphere->m_fRadius	= ((CBot*)m_pClawedEntity)->m_pBotInfo_Gen->fCollSphere1Radius_MS * 2.0f;
		} else {
			pSphere->m_Pos		= mtx.m_vPos.v3;
			pSphere->m_fRadius	= 0.01f;
		}
	} else {
		mtx.MulPoint( pSphere->m_Pos, m_avCollisionPts[uID].v3 );
		pSphere->m_fRadius = m_afCollisionPtsRadius[uID];
	}

	
}


void CVehicleLoader::_ApplyControlForces( void ) {
	f32 fUnitForceAvail = 1.0f - fmath_Div( m_fAltitude, _HOVER_MAX_HEIGHT );
	FMATH_CLAMP_UNIT_FLOAT( fUnitForceAvail );
	if( fUnitForceAvail == 0.0f ) {
		return;
	}


	CFVec3A vVel_MS;
	CFVec3A vDesVel_MS;
	CFVec3A vAccel_MS;

	WS2MS( vVel_MS, *(m_PhysObj.GetVelocity()) );
	vVel_MS.y = 0.0f;

	vDesVel_MS.Mul( m_XlatStickNormVecXZ_MS, m_BotInfo_VehiclePhysics.fMaxSpeed );

	vAccel_MS.Sub( vDesVel_MS, vVel_MS );

	FMATH_CLASS_DEBUG_FCHECK( *(m_PhysObj.GetVelocity()) );
	FMATH_CLASS_DEBUG_FCHECK( vVel_MS );
	FMATH_CLASS_DEBUG_FCHECK( vDesVel_MS );
	FMATH_CLASS_DEBUG_FCHECK( vAccel_MS );

	f32 fAccelMag = vAccel_MS.SafeUnitAndMag( vAccel_MS );
	if( fAccelMag > 0.01f ) {
		FMATH_CLAMPMAX( fAccelMag, m_BotInfo_VehiclePhysics.fMaxAccel );

		if( !FMATH_FSAMESIGN( vVel_MS.z, vAccel_MS.z ) ) {
			vAccel_MS.z *= m_BotInfo_Loader.fDecelBonusZ;
		}

		if( !FMATH_FSAMESIGN( vVel_MS.x, vAccel_MS.x ) ) {
			vAccel_MS.x *= m_BotInfo_Loader.fDecelBonusX;
		}

		vAccel_MS.Mul( fAccelMag * m_BotInfo_VehiclePhysics.fMass * fUnitForceAvail );
		_ApplyControlForce( MS2WS( vAccel_MS ) );
	}
}



void CVehicleLoader::_ApplyHoverForce( void ) {
	CFVec3A vTmp;
	CFVec3A vHoverForce;
	CFVec3A vHoverForcePos;

	vTmp = m_MtxToWorld.m_vUp;
	vTmp.Negate();

	PROTRACK_BEGINBLOCK( "Check Altitude" );
		m_fAltitudeLeft		= _CheckAltitude( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineLeft]->m_vPos,	vTmp, _HOVER_MAX_HEIGHT, &m_vExhaustPartPosLeft, &m_vExhaustPartDirLeft );
		m_fAltitudeRight	= _CheckAltitude( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineRight]->m_vPos, vTmp, _HOVER_MAX_HEIGHT, &m_vExhaustPartPosRight, &m_vExhaustPartDirRight );
		m_fAltitudeFront	= _CheckAltitude( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxClawArm]->m_vPos,		vTmp, _HOVER_MAX_HEIGHT, NULL, NULL );
	PROTRACK_ENDBLOCK();

	m_vExhaustPartPosLeft.Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineLeft]->m_vUp, -1.0f * m_fAltitudeLeft );
	m_vExhaustPartPosLeft.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineLeft]->m_vPos );

	m_vExhaustPartPosRight.Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineRight]->m_vUp, -1.0f * m_fAltitudeRight );
	m_vExhaustPartPosRight.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineRight]->m_vPos );

	if( m_hExhaustPartEmitterLeft != FPARTICLE_INVALID_HANDLE ) {
		fparticle_EnableEmission( m_hExhaustPartEmitterLeft, m_fAltitudeLeft < _HOVER_MAX_HEIGHT );
	} else {
		m_hExhaustPartEmitterLeft = fparticle_SpawnEmitter( m_hExhaustParticleDef, &m_ExhaustPartCfgLeft );
	}

	if( m_hExhaustPartEmitterRight != FPARTICLE_INVALID_HANDLE ) {
		fparticle_EnableEmission( m_hExhaustPartEmitterRight, m_fAltitudeRight < _HOVER_MAX_HEIGHT );
	} else {
		m_hExhaustPartEmitterRight = fparticle_SpawnEmitter( m_hExhaustParticleDef, &m_ExhaustPartCfgRight );
	}

	m_fAltitude = (m_fAltitudeLeft + m_fAltitudeRight /*+ m_fAltitudeFront + m_fAltitudeFront*/) * 0.5f;

	//fGA = gravity effect, fVA = velocity effect, fSA = surface effect.  Work together to determine how much up force to apply
	f32		fGA, fVA, fSA, fHoverForceMag;
	//fVA shared for everyone:
	fVA = fmath_Div( -100.0f - m_PhysObj.GetVelocity()->Dot( CFVec3A::m_UnitAxisY ), -100.0f );
	FMATH_CLAMP( fVA, 0.05f, 5.0f );
	fVA = fmath_Inv( fVA );

	// left...
	if( m_fAltitudeLeft > _HOVER_HEIGHT ) {
		fGA = fmath_Div( _HOVER_MAX_HEIGHT - m_fAltitudeLeft, _HOVER_TOP_ZONE );
	} else {
		m_fAltitudeLeft -= _HOVER_MIN_HEIGHT;

		FMATH_CLAMPMIN( m_fAltitudeLeft, 0.1f );
		fGA = fmath_Div( _HOVER_BOTTOM_ZONE, m_fAltitudeLeft );
	}

	FMATH_CLAMP( fGA, 0.0f, 2.0f );

	if( m_vExhaustPartDirLeft.y < 0.5f ) {
		fSA = fmath_Div( m_vExhaustPartDirLeft.y - 0.5f, 0.5f ); //fmath_Sqrt( m_vExhaustPartDirLeft.y );
		fSA *= fSA;
	} else {
		fSA = 1.0f;
	}
	FMATH_CLAMP_MIN0( fSA );

	fHoverForceMag = fGA * fVA * fSA;
	fHoverForceMag *= fmath_RandomFloatRange( 0.95f, 1.05f );

	vHoverForce.Mul( m_vGroundNormal, -fHoverForceMag * m_BotInfo_VehiclePhysics.fMass * m_BotInfo_Gen.fGravity * 0.4f );
	vHoverForcePos.Sub( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineLeft]->m_vPos, m_MtxToWorld.m_vPos );
	m_PhysObj.ApplyForce( &vHoverForce, &vHoverForcePos );

	// right...
	if( m_fAltitudeRight > _HOVER_HEIGHT ) {
		fGA = fmath_Div( _HOVER_MAX_HEIGHT - m_fAltitudeRight, _HOVER_TOP_ZONE );
	} else {
		m_fAltitudeRight -= _HOVER_MIN_HEIGHT;

		FMATH_CLAMPMIN( m_fAltitudeRight, 0.1f );
		fGA = fmath_Div( _HOVER_BOTTOM_ZONE, m_fAltitudeRight );
	}

	FMATH_CLAMP( fGA, 0.0f, 2.0f );

	if( m_vExhaustPartDirRight.y < 0.5f ) {
		fSA = fmath_Div( m_vExhaustPartDirLeft.y - 0.5f, 0.5f ); //fmath_Sqrt( m_vExhaustPartDirLeft.y );
		fSA *= fSA;
	} else {
		fSA = 1.0f;
	}
	FMATH_CLAMP_MIN0( fSA );

	fHoverForceMag = fGA * fVA * fSA;
 	fHoverForceMag *= fmath_RandomFloatRange( 0.95f, 1.05f );

	vHoverForce.Mul( m_vGroundNormal, -fHoverForceMag *  m_BotInfo_VehiclePhysics.fMass * m_BotInfo_Gen.fGravity * 0.4f );
	vHoverForcePos.Sub( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxEngineRight]->m_vPos, m_MtxToWorld.m_vPos );
	m_PhysObj.ApplyForce( &vHoverForce, &vHoverForcePos );

	// front...
	if( m_fAltitudeFront > _HOVER_HEIGHT ) {
		fGA = fmath_Div( _HOVER_MAX_HEIGHT - m_fAltitudeFront, _HOVER_TOP_ZONE );
	} else {
		m_fAltitudeFront -= _HOVER_MIN_HEIGHT;

		FMATH_CLAMPMIN( m_fAltitudeFront, 0.1f );
		fGA = fmath_Div( _HOVER_BOTTOM_ZONE, m_fAltitudeFront );
	}

	FMATH_CLAMP( fGA, 0.0f, 2.0f );

	if( m_SurfaceUnitNorm_WS.y < 0.5f ) {
		fSA = fmath_Div( m_vExhaustPartDirLeft.y - 0.5f, 0.5f ); //fmath_Sqrt( m_vExhaustPartDirLeft.y );
		fSA *= fSA;
	} else {
		fSA = 1.0f;
	}
	FMATH_CLAMP_MIN0( fSA );

	fHoverForceMag = fGA * fVA * fSA;

	vHoverForce.Mul( m_vGroundNormal, -fHoverForceMag *  m_BotInfo_VehiclePhysics.fMass * m_BotInfo_Gen.fGravity * 0.2f );
	vHoverForcePos.Sub( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxClawArm]->m_vPos, m_MtxToWorld.m_vPos );
	m_PhysObj.ApplyForce( &vHoverForce, &vHoverForcePos );
}


void CVehicleLoader::_ApplyStabilizingTorque( void ) {
	CFVec3A vForce, vArm, vTorque;

	f32 fAvailForce = fmath_Div( _HOVER_MAX_HEIGHT - m_fAltitude, _HOVER_MAX_HEIGHT );
	FMATH_CLAMP( fAvailForce, 0.25f, 1.0f );

	vForce.Mul( m_MtxToWorld.m_vUp, m_fRightingForceMag * fAvailForce * _RIGHT_FORCE_MULTIPLIER );
	vArm = m_vGroundNormal;
	vTorque.Cross( vForce, vArm );
	
	_ApplyControlTorque( vTorque );

	vForce.Mul( m_MountUnitFrontXZ_WS, m_fRightingForceMag );
	vArm.ReceiveUnitXZ( m_MtxToWorld.m_vFront );
	vTorque.Cross( vArm, vForce );
	_ApplyControlTorque( vTorque );

}


void CVehicleLoader::_ClawWork( void ) {
	_ClawTriggerWork();
	_ClawAnimWork();

	

//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	if( m_fClawPressure > _fClawPressureTracker ) {
//		_fClawPressureTracker = m_fClawPressure;
//	}
//
//	ftext_DebugPrintf( 0.25f, 0.4f, "~w1Claw pressure:  %0.2f, max: %0.2f", m_fClawPressure, _fClawPressureTracker );
//#endif

	switch( m_ClawState ) {
		case CLAWSTATE_OPEN:
			m_fClawedEntitySpan	= 0.0f;
			m_pClawedEntity		= NULL;

//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//			_fClawPressureTracker = 0.0f;
//#endif

			break;

		case CLAWSTATE_OPENING:
			if( m_fUnitClawAngle <= _CLAW_MIN_UNIT_ANGLE ) {
				//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
				//	_fClawPressureTracker = 0.0f;
				//#endif

				m_ClawState		= CLAWSTATE_OPEN;
				m_pClawedEntity = NULL;
			}
			break;

		case CLAWSTATE_CLOSING:

			if( m_pClawedEntity ) {
				_ClearEntityVerletObject( m_pClawedEntity );
				_ClawOrientObject();
			} else {
				m_fClawedEntitySpan = 0.0f;
			}

			if( m_fClawSpan < m_fClawedEntitySpan ) {
				if( m_pClawedEntity && (m_pClawedEntity->TypeBits() & ENTITY_BIT_BOT) ) {
					((CBot*)m_pClawedEntity)->StartForcedPanic();
					((CBot*)m_pClawedEntity)->Attach_ToParent_WithGlue_WS( this, m_apszBoneNameTable[BONE_VICTIM_ATTACH] );

					m_ClawState = CLAWSTATE_CLOSED_ON_BOT;

					// play the sound for closing on nothing...
					if( m_nPossessionPlayerIndex < 0 ) {
						fsndfx_Play3D( m_BotInfo_LoaderSounds.hClawClose, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos), m_BotInfo_Loader.fSoundRadius3D, 1.0f, m_BotInfo_Loader.fSoundVolume3D );
					} else {
						fsndfx_Play2D( m_BotInfo_LoaderSounds.hClawClose, m_BotInfo_Loader.fSoundVolume2D );
					}

				} else if( m_pClawedEntity ) {
					m_pClawedEntity->Attach_ToParent_WS( this, m_apszBoneNameTable[BONE_VICTIM_ATTACH] );
					m_ClawState = CLAWSTATE_CLOSED_ON_OBJECT;
					m_PhysObj.SetLinearDamping( m_BotInfo_VehiclePhysics.fLinearDampingHigh );
					m_PhysObj.SetAngularDamping( m_BotInfo_VehiclePhysics.fAngularDampingHigh );

					// play the close on object sound...
					if( m_nPossessionPlayerIndex < 0 ) {
						fsndfx_Play3D( m_BotInfo_LoaderSounds.hClawCloseObject, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos), m_BotInfo_Loader.fSoundRadius3D, 1.0f, m_BotInfo_Loader.fSoundVolume3D );
					} else {
						fsndfx_Play2D( m_BotInfo_LoaderSounds.hClawCloseObject, m_BotInfo_Loader.fSoundVolume2D );
					}

				} else {
					m_ClawState = CLAWSTATE_CLOSED;
					// play the sound for closing on nothing...
					if( m_nPossessionPlayerIndex < 0 ) {
						fsndfx_Play3D( m_BotInfo_LoaderSounds.hClawClose, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos), m_BotInfo_Loader.fSoundRadius3D, 1.0f, m_BotInfo_Loader.fSoundVolume3D );
					} else {
						fsndfx_Play2D( m_BotInfo_LoaderSounds.hClawClose, m_BotInfo_Loader.fSoundVolume2D );
					}
				}
			}

			// do this last
			_CheckForClawEntity();
			break;
		
		case CLAWSTATE_CLOSED:
			m_fClawPressure = 1.0f;
			if( m_fUnitClawAngle < 0.9f ) {
				m_ClawState = CLAWSTATE_OPENING;
			}
			break;

		case CLAWSTATE_CLOSED_ON_OBJECT:
			FASSERT( m_pClawedEntity );
			if( !m_pClawedEntity ) {
				m_ClawState = CLAWSTATE_CLOSED;
				break;
			}

			m_vClawedEntityDesiredPos = m_pClawedEntity->MtxToWorld()->m_vPos;

			if( m_fClawLockUnitTimer < 1.0f ) {
				m_fClawLockUnitTimer += FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MAX1( m_fClawLockUnitTimer );
			}

			if( m_fClawSpan > m_fClawedEntitySpan ) {
				m_PhysObj.SetLinearDamping( m_BotInfo_VehiclePhysics.fLinearDampingLow );
				m_PhysObj.SetAngularDamping( m_BotInfo_VehiclePhysics.fAngularDampingLow );
				//m_PhysObj.SetAngularDamping( 500.0f );
				m_ClawState = CLAWSTATE_OPENING;	
				_DropHeldObject();
			} else {
                CFVec3A vForce = CFVec3A::m_UnitAxisY;
				CFVec3A vForcePos;
				vForce.Mul( _PICKUP_FORCE * m_BotInfo_VehiclePhysics.fMass );
				m_PhysObj.ApplyForce( &vForce, &m_vCMOffset );

				vForce.Mul( 2.0f );

				vForcePos.Sub( m_vClawedEntityDesiredPos, m_MtxToWorld.m_vPos );
				m_PhysObj.ApplyForce( &vForce, &vForcePos );

			}
			break;

		case CLAWSTATE_CLOSED_ON_BOT:
			FASSERT( m_pClawedEntity && (m_pClawedEntity->TypeBits() & ENTITY_BIT_BOT) );
			if( !m_pClawedEntity || !(m_pClawedEntity->TypeBits() & ENTITY_BIT_BOT) ){
				m_ClawState = CLAWSTATE_CLOSED;
				break;
			}

			m_vClawedEntityDesiredPos = m_pClawedEntity->MtxToWorld()->m_vPos;
			m_ClawEntityOrientMtx = *m_pClawedEntity->MtxToWorld();
			
			if( m_fClawLockUnitTimer < 1.0f ) {
				m_fClawLockUnitTimer += FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MAX1( m_fClawLockUnitTimer );
			}

			if( m_fClawSpan > m_fClawedEntitySpan ) {
				m_ClawState = CLAWSTATE_OPENING;

				((CBot*)m_pClawedEntity)->EndForcedPanic();
				((CBot*)m_pClawedEntity)->DetachFromParent();

				// give the bot a little velocity...
				CFVec3A vBotVelocity;
				CFVec3A vBotVelLoc;
				vBotVelLoc.Sub( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos, m_MtxToWorld.m_vPos );
				m_PhysObj.GetVelocity( &vBotVelLoc, &vBotVelocity );
				vBotVelocity.Mul( 1.2f );
				((CBot*)m_pClawedEntity)->ApplyVelocityImpulse_WS( vBotVelocity );

			} else if( (MultiplayerMgr.IsMultiplayer() || _ENABLE_BOT_CRUSHING) && (m_fClawPressure > _BOTCRUSH_PRESSURE) ) {
				if( m_nPossessionPlayerIndex >= 0 ) {
					fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE, &m_hForce );
				}

				_CauseCrushDamage( (CBot*)m_pClawedEntity );
			}
			break;
	}

	if( !ClawIsClosed() ) {
		if( m_fClawLockUnitTimer > 0.0f ) {
			m_fClawLockUnitTimer -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0( m_fClawLockUnitTimer );
		}
	}
}


//Calculates:  m_fClawPressure, m_fUnitClawAngle, m_fClawAngle, m_fClawSpan, m_bClawLocked, m_fClawLockUnitTimer
void CVehicleLoader::_ClawTriggerWork( void ) {
	f32 fSpeed;
	f32 fDesiredUnitClawAngle;

	FMATH_DEBUG_FCHECK( m_fClawPressure );

	fDesiredUnitClawAngle = m_fControls_Fire1;

	fSpeed = (fDesiredUnitClawAngle - m_fUnitClawAngle) * FLoop_fPreviousLoopOOSecs;
	FMATH_BIPOLAR_CLAMPMAX( fSpeed, _MAX_CLAW_DELTA );

	if( !ClawIsClosed() ) {
		if( fDesiredUnitClawAngle > m_fUnitClawAngle ) {
			m_ClawState = CLAWSTATE_CLOSING;
		} else if( fDesiredUnitClawAngle < m_fUnitClawAngle ) {
			m_ClawState = CLAWSTATE_OPENING;
		}
	}

	f32 _fk = 5.0f * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_UNIT_FLOAT( _fk );
	m_fClawPressure = (m_fClawPressure * (1.0f-_fk)) + (fSpeed * _fk);
	FMATH_CLAMP_MIN0( m_fClawPressure );

	m_fUnitClawAngle = m_fUnitClawAngle + fSpeed * FLoop_fPreviousLoopSecs;
	FASSERT_UNIT_FLOAT( m_fMaxUnitClawAngle );
	FMATH_CLAMP( m_fUnitClawAngle, _CLAW_MIN_UNIT_ANGLE, m_fMaxUnitClawAngle );

	f32 fsin = fmath_Div( m_fClawedEntitySpan*0.5f, _CLAW_RADIUS );
	FMATH_CLAMP_UNIT_FLOAT( fsin );
	f32 fMaxAngle = fmath_Atan( fsin, fmath_Sqrt( 1.0f - (fsin*fsin) ) );
	fMaxAngle = 1.0f - fmath_Div( fMaxAngle, (_MIN_CLAW_ROTATE-_MAX_CLAW_ROTATE) );
	FMATH_CLAMPMAX( m_fUnitClawAngle, fMaxAngle );


	m_fClawAngle	= FMATH_FPOT( m_fUnitClawAngle, _MIN_CLAW_ROTATE, _MAX_CLAW_ROTATE );
	m_fClawSpan		= (2.0f * _CLAW_RADIUS * fmath_Sin( m_fClawAngle )) + 2.0f;
}


void CVehicleLoader::_ClawAnimWork( void ) {
	// Set up the claw animation...
	CFAnimFrame animframe;

	animframe.BuildQuat( CFVec3A::m_UnitAxisY, -m_fClawAngle,  m_pWorldMesh->m_pMesh->pBoneArray[m_nBoneIdxClawLeft].AtRestBoneToParentMtx.m_vPos );
	m_pAnimManFrame->UpdateFrame( ANIMBONE_CLAW_LEFT, animframe );

	animframe.BuildQuat( CFVec3A::m_UnitAxisY, m_fClawAngle,  m_pWorldMesh->m_pMesh->pBoneArray[m_nBoneIdxClawRight].AtRestBoneToParentMtx.m_vPos );
	m_pAnimManFrame->UpdateFrame( ANIMBONE_CLAW_RIGHT, animframe );

	animframe.BuildQuat( CFVec3A::m_UnitAxisX, fmath_UnitLinearToSCurve( m_fClawLockUnitTimer ) * _CLAW_ARM_RAD,
						 m_pWorldMesh->m_pMesh->pBoneArray[m_nBoneIdxClawArm].AtRestBoneToParentMtx.m_vPos );
	m_pAnimManFrame->UpdateFrame( ANIMBONE_CLAW_ARM, animframe );

}


BOOL CVehicleLoader::_TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
    FASSERT( m_pCollBot && (m_pCollBot->TypeBits() & ENTITY_BIT_VEHICLELOADER) );
	FASSERT( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH );
	FASSERT( pTracker->m_nUser == MESHTYPES_ENTITY );
	FASSERT( ((CEntity*)pTracker->m_pUser)->TypeBits() & (ENTITY_BIT_BOT | ENTITY_BIT_MESHENTITY) );
	
	if( ((CEntity*)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_MESHENTITY ) {
 		if( ((CMeshEntity*)pTracker->m_pUser)->IsLoaderPickupEnabled() ) {
			((CVehicleLoader*)m_pCollBot)->m_pClawedEntity = (CEntity*)pTracker->m_pUser;
			return FALSE;
		} else {
			return TRUE;
		}
	}

	// it must be a bot
	((CVehicleLoader*)m_pCollBot)->m_pClawedEntity = (CEntity*)pTracker->m_pUser;
	return FALSE;

	return TRUE;
}


void CVehicleLoader::_CheckForClawEntity( void ) {
	CFSphere sphere;
	sphere.m_Pos.Mul( m_MtxToWorld.m_vFront.v3, 13.0f );
	sphere.m_Pos.Add( m_MtxToWorld.m_vPos.v3 );
	sphere.m_fRadius = 3.0f;

	CEntity *pPrevClawEntity = m_pClawedEntity;
    
	// clear it here, it will be set by the callback
	m_pClawedEntity = NULL;
	m_pCollBot = this;
	u64 uBits = (MultiplayerMgr.IsSinglePlayer()) ? _CLAWABLE_ENTITY_BITS : _CLAWABLE_ENTITY_BITS | ENTITY_BIT_BOTGLITCH;
	fworld_FindTrackersIntersectingSphere( &sphere, FWORLD_TRACKERTYPE_MESH, _TrackerCollisionCallback, 0, NULL, NULL, MESHTYPES_ENTITY, uBits ); 
	m_pCollBot = NULL;

	if( m_pClawedEntity /*&& (m_pClawedEntity->TypeBits() & ENTITY_BIT_MESHENTITY)*/ ) {
		if( !_ValidateClawableObject( (CMeshEntity*)m_pClawedEntity ) ) {
			m_pClawedEntity = NULL;
		}
	}

	// if for whatever reason, we've changed or cleared the clawed entity while locked on, get rid of it
	if( ((m_ClawState == CLAWSTATE_CLOSED_ON_BOT) || (m_ClawState == CLAWSTATE_CLOSED_ON_OBJECT)) && pPrevClawEntity && (pPrevClawEntity != m_pClawedEntity) ) {
		pPrevClawEntity->DetachFromParent();
		if( pPrevClawEntity->TypeBits() & ENTITY_BIT_BOT ) {
			((CBot*)pPrevClawEntity)->EndForcedPanic();
		}
	}
}


void CVehicleLoader::_ClawOrientObject( void ) {
	FASSERT( m_pClawedEntity );

	// if claws are open, return
	if( m_fUnitClawAngle < 0.01f ) {
		return;
	}

	// if it's already attached, return
	if( m_pClawedEntity->GetParent() == this ) {
		return;
	}

	m_pClawedEntity->DetachFromParent();

	f32 fPickupSlack = m_fClawSpan - m_fClawedEntitySpan;
	FMATH_CLAMP_MIN0( fPickupSlack );

	CFVec3A		vSide;
	CFVec3A		vAdj;
	CFVec3A		vErr;

	vErr.Sub( m_pClawedEntity->MtxToWorld()->m_vPos, m_ClawEntityOrientMtx.m_vPos );
	f32 fDist = vAdj.SafeUnitAndMag( vErr );
	if( fDist > 0.01f ) {
		FMATH_CLAMPMAX( fDist, fPickupSlack * 0.5f );
		vAdj.Mul( fDist );
		vAdj.Add( m_ClawEntityOrientMtx.m_vPos );
	} else {
		vAdj = m_ClawEntityOrientMtx.m_vPos;
	}

	CFMtx43A mTmp;
	CFQuatA	 qO, qA, qF;
	f32		 fSlerp;

	qO.BuildQuat( m_ClawEntityOrientMtx );
	qA.BuildQuat( *(m_pClawedEntity->MtxToWorld()) );

	fSlerp = 1.0f - fmath_Div( fPickupSlack, _MAX_SPAN );
	qF.ReceiveSlerpOf( fSlerp, qA, qO );

	qF.BuildMtx( mTmp );
	mTmp.m_vPos = vAdj;

	m_vClawedEntityDesiredPos = m_ClawEntityOrientMtx.m_vPos;
	m_pClawedEntity->Relocate_RotXlatFromUnitMtx_WS( &mTmp );
}


void CVehicleLoader::_EngineWork( void ) {
	m_fExhaustPartIntensityLeft		= 1.0f; // - fmath_Div( m_fAltitudeLeft, _HOVER_MAX_HEIGHT );
	m_fExhaustPartIntensityRight	= 1.0f; // - fmath_Div( m_fAltitudeRight, _HOVER_MAX_HEIGHT );

	FMATH_CLAMP_UNIT_FLOAT( m_fExhaustPartIntensityLeft );
	FMATH_CLAMP_UNIT_FLOAT( m_fExhaustPartIntensityRight );

	WS2MS( m_vForceApplied );
	m_vForceApplied.Mul( m_fOOMaxForce );
	m_fTorqueApplied *= m_fOOMaxTorque;

	CFVec3A vEngineLeft;
	CFVec3A vEngineRight;

	if( m_eEngineState == ENGINE_STATE_RUNNING ) {
		if( FMATH_FABS(m_XlatStickNormVecXZ_MS.z) > FMATH_FABS( m_vForceApplied.z) ) {
			m_vForceApplied.z = m_XlatStickNormVecXZ_MS.z;
		}

		if( FMATH_FABS(m_XlatStickNormVecXZ_MS.x) > FMATH_FABS( m_vForceApplied.x) ) {
			m_vForceApplied.x = m_XlatStickNormVecXZ_MS.x;
		}

		vEngineLeft.x	= -m_vForceApplied.x;
		vEngineRight.x	= -m_vForceApplied.x;

		vEngineLeft.z	= m_vForceApplied.z;
		vEngineRight.z	= m_vForceApplied.z;

		vEngineLeft.z  += m_fTorqueApplied;
		vEngineRight.z -= m_fTorqueApplied;

		FMATH_BIPOLAR_CLAMPMAX( vEngineLeft.x, 1.0f );
		FMATH_BIPOLAR_CLAMPMAX( vEngineRight.x, 1.0f );
		FMATH_BIPOLAR_CLAMPMAX( vEngineLeft.z, 1.0f );
		FMATH_BIPOLAR_CLAMPMAX( vEngineRight.z, 1.0f );
	} else {
		vEngineLeft.Zero();
		vEngineRight.Zero();
	}

	// calculate input based on altitude
	f32 fHoverInput = 1.0f - fmath_Div( m_fAltitude, _HOVER_MAX_HEIGHT );
	FMATH_CLAMP_UNIT_FLOAT( fHoverInput );
    

	//Calculate engine vibration angles...
	if( m_eEngineState == ENGINE_STATE_RUNNING ) {
		m_fEngineVibTimerLeft	-= FLoop_fPreviousLoopSecs;
		if( m_fEngineVibTimerLeft <= 0.0f ) {
			// advance counter
			if( ++m_uEngineVibCtrLeft > 3 ) {
				m_uEngineVibCtrLeft = 0;
			}
			m_fEngineVibMagLeft		=  fmath_RandomFloatRange( 1.0f, 1.1f ) * 2.0f; //(2.0f - 1.0f);//m_XlatStickNormVecXZ_MS.MagSq());
			m_fEngineVibTimerLeft	+= (0.02f + fmath_RandomFloatRange( 0.01f, 0.05f )) * (2.0f - m_XlatStickNormVecXZ_MS.MagSq() );
		}

		m_fEngineVibTimerRight	-= FLoop_fPreviousLoopSecs;
		if( m_fEngineVibTimerRight <= 0.0f ) {
			// advance counter
			if( ++m_uEngineVibCtrRight > 3 ) {
				m_uEngineVibCtrRight = 0;
			}
			m_fEngineVibMagRight	=  fmath_RandomFloatRange( 1.0f, 1.1f ) * 2.0f; //(2.0f - 1.0f);//m_XlatStickNormVecXZ_MS.MagSq());
			m_fEngineVibTimerRight	+= (0.02f + fmath_RandomFloatRange( 0.01f, 0.05f )) * (2.0f - m_XlatStickNormVecXZ_MS.MagSq() );
		}
	} else {
		m_fEngineVibMagRight	= 0.0f;
		m_fEngineVibMagLeft		= 0.0f;
	}

	m_fEngineLoad = 5.0f * (1.0f - fmath_Div( m_fAltitude, _HOVER_HEIGHT ));

	if( FMATH_FABS( m_vForceApplied.x ) > m_fEngineLoad ) {
		m_fEngineLoad = FMATH_FABS( m_vForceApplied.x );
	}
	if( FMATH_FABS( m_vForceApplied.z ) > m_fEngineLoad ) {
		m_fEngineLoad = FMATH_FABS( m_vForceApplied.z );
	}
	//if( FMATH_FABS( m_fTorqueApplied ) > m_fEngineLoad ) {
	//	m_fEngineLoad = FMATH_FABS( m_fTorqueApplied );
	//}



	CFQuatA qT, qN, qR;
	f32 fSlerp = _ENGINE_UPDATE_RATE * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_MAX1( fSlerp );

	// left...
	qT.Identity();
	qR.BuildQuatRotX( _ENGINE_X_ANGLE * vEngineLeft.z + m_avEngineVib[m_uEngineVibCtrLeft].y * m_fEngineVibMagLeft );
	qT.Mul( qR );
	qR.BuildQuatRotZ( _ENGINE_Z_ANGLE * vEngineLeft.x - (_ENGINE_HOVER_Z_ANGLE * fHoverInput * fHoverInput)  + m_avEngineVib[m_uEngineVibCtrLeft].x * m_fEngineVibMagLeft );
	qT.Mul( qR );
	qN.ReceiveSlerpOf( fSlerp, m_AnimFrameEngineLeft, qT );
	m_AnimFrameEngineLeft.v = qN.v;
	m_pAnimManFrame->UpdateFrame( ANIMBONE_ENGINE_LEFT, m_AnimFrameEngineLeft );

	// right...
	qT.Identity();
	qR.BuildQuatRotX( _ENGINE_X_ANGLE * vEngineRight.z + m_avEngineVib[m_uEngineVibCtrRight].y * m_fEngineVibMagRight );
	qT.Mul( qR );
	qR.BuildQuatRotZ( _ENGINE_Z_ANGLE * vEngineRight.x + (_ENGINE_HOVER_Z_ANGLE * fHoverInput * fHoverInput)  + m_avEngineVib[m_uEngineVibCtrRight].x * m_fEngineVibMagRight );
	qT.Mul( qR );
	qN.ReceiveSlerpOf( fSlerp, m_AnimFrameEngineRight, qT );
	m_AnimFrameEngineRight.v = qN.v;
	m_pAnimManFrame->UpdateFrame( ANIMBONE_ENGINE_RIGHT, m_AnimFrameEngineRight );

	// clear accumulators...
	m_vForceApplied.Zero();
	m_fTorqueApplied = 0.0f;
}


f32 CVehicleLoader::ComputeEstimatedControlledStopTimeXZ( void ) {
	f32 fDecelBonus = FMATH_MIN( m_BotInfo_Loader.fDecelBonusX, m_BotInfo_Loader.fDecelBonusX );
	return   fmath_Div(m_fSpeed_WS, m_BotInfo_VehiclePhysics.fMaxAccel*fDecelBonus);
}


BOOL CVehicleLoader::ActionNearby( CEntity *pEntity ) {
	CVehicle::ActionNearby( pEntity );
	
	if( !pEntity || !(pEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		return FALSE;
	}

	CBot* pBot = (CBot*)pEntity;

	if( m_eStationStatus == STATION_STATUS_EMPTY ) {
		if( IsUpsideDown() ) {
			FlipRightsideUp( pEntity );
		} else if( CanOccupyStation( pBot, STATION_DRIVER ) == STATION_STATUS_EMPTY ) {
			EnterStation( pBot, STATION_DRIVER, TRUE /*bProximityCheck*/, FALSE /*bImmediately*/ );
		}

	} else if( m_eStationStatus == STATION_STATUS_OCCUPIED ) {
		if( pBot == m_pDriverBot ) {
			ExitStation( pBot, FALSE /*bImmediately*/ );
		}else {
		return FALSE;
		}
	} else {
		return FALSE;
	}

	return TRUE;
}


void CVehicleLoader::DriverEnter( CBot* pDriverBot, cchar *pszAttachPointBoneName/* = NULL*/, BOOL bImmediately /* = FALSE */ ) {
	CVehicle::DriverEnter( pDriverBot, pszAttachPointBoneName );
	pDriverBot->Relocate_RotXlatFromUnitMtx_PS( &CFMtx43A::m_IdentityMtx );

	m_eCameraState = CAMERA_STATE_START_ENTER_VEHICLE;
	m_eStationStatus = STATION_STATUS_ENTERING;
	m_pDriverBot = pDriverBot;
	m_pCameraBot = pDriverBot;

	m_fEngineDisruptTimer = 0.0f;

	StartEngine();
}

void CVehicleLoader::DriverExit( CBot* pDriverBot ) {
	if( pDriverBot == NULL )
	{
		return;
	}

	if( pDriverBot != m_pDriverBot )
	{
		return;
	}

	//// if this was a player, restore reticle
	//if( pDriverBot->m_nPossessionPlayerIndex >= 0 ) {
	//	CReticle *pReticle = &(Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle);
	//	pReticle->SetNormOrigin( m_fReticlePrevX, m_fReticlePrevY );
	//	pReticle->SetType( m_ReticlePrevType );
	//	pReticle->EnableDraw( m_bReticleDrawPrev );
	//}

	CVehicle::DriverExit( pDriverBot );

	m_eStationStatus = STATION_STATUS_EXITING;

	if( m_pDriverBot &&
		m_pDriverBot->IsInWorld() &&
		!m_pDriverBot->IsMarkedForWorldRemove() && 
		!IsDriverUseless(m_pDriverBot) &&
		IsInWorld() && !IsMarkedForWorldRemove() ) {
        m_eCameraState = CAMERA_STATE_START_EXIT_VEHICLE;
		m_pCameraBot = m_pDriverBot;
	} else {
		if( m_pDriverBot->m_nOwnerPlayerIndex >= 0 ) {
			gamecam_SwitchPlayerTo3rdPersonCamera((GameCamPlayer_e) m_pDriverBot->m_nOwnerPlayerIndex, m_pDriverBot );
		}

		m_eCameraState = CAMERA_STATE_INACTIVE;
		m_pCameraBot = NULL;
	}

	m_pDriverBot->DetachFromParent();
	m_pDriverBot = NULL;

	m_fEngineDisruptTimer = 0.0f;
	
	StopEngine( TRUE );
}


CBot *CVehicleLoader::IsStationObstructed( Station_e eStation )
{
	FASSERT( eStation == STATION_DRIVER );
	return CVehicle::StationObstructed( this, m_nBoneIdxDriverAttach, m_pBotInfo_Vehicle->fDriverStationRadius );
}


#define _USERPROPS_ENTER_DISTANCE_SQ	( 2.0f * 2.0f )

CVehicle::StationStatus_e CVehicleLoader::CanOccupyStation( CBot *pBot, Station_e eStation ) {
	if( !pBot ) {
		return STATION_STATUS_WRONG_BOT;
	}

	if( !( pBot->TypeBits() & ( ENTITY_BIT_BOTGLITCH | ENTITY_BIT_BOTGRUNT ) ) ) {
		return STATION_STATUS_WRONG_BOT;
	}

	if( eStation != STATION_DRIVER ) {
		return STATION_STATUS_NO_STATION;
	}

	if( m_pDriverBot ) {
		return STATION_STATUS_OCCUPIED;
	}

	if( pBot->m_nPossessionPlayerIndex >= 0 && (m_uVehicleFlags & VEHICLE_FLAG_NO_PLAYER_DRIVE) ) {
		// bot is a player and vehicle is flagged no-player-drive
		return STATION_STATUS_UNAVAILABLE;
	}

	if( pBot->m_nPossessionPlayerIndex >= 0 )
	{
		if( CHud2::GetHudForPlayer(pBot->m_nPossessionPlayerIndex)->GetUnitSelfDestruct() > 0.0f )
		{
			// don't allow entry while player is holding "X" to exit possessed bot.
			return STATION_STATUS_UNAVAILABLE;
		}
	}

	if( pBot->m_nPossessionPlayerIndex >= 0 ) {
		CFVec3A vTestPoint;
		CFVec3A vTestDir;
		CFVec3A vDelta;
		CFVec3A vTemp;
		f32 fDist;

		// see if driver is in valid entry area behind vehicle
		vTestDir.Set( m_MtxToWorld.m_vFront );
		vTestDir.Negate();
		vTestPoint.Set( vTestDir );
		vTestPoint.Mul( 2.5f );
		vTemp.Set( m_MtxToWorld.m_vUp );
		vTemp.Mul( 3.0f );
		vTestPoint.Add( vTemp );
		vTestPoint.Add( m_MtxToWorld.m_vPos );
		vDelta.Set( pBot->MtxToWorld()->m_vPos );
		vDelta.Sub( vTestPoint );
		fDist = vTestDir.Dot( vDelta );
		if( fDist > 0.0f && fDist < 6.0f ) {
			return STATION_STATUS_EMPTY;
		}
	} else {
		// AI bots only want to know if they're near the driving position, not the entry area.
		if( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach]->m_vPos.DistSq( pBot->MtxToWorld()->m_vPos ) < _USERPROPS_ENTER_DISTANCE_SQ ) {
			return STATION_STATUS_EMPTY;
		}
	}

	return STATION_STATUS_OUT_OF_RANGE;
}


BOOL CVehicleLoader::GetStationEntryPoint( Station_e eStation, CFVec3A *pPoint )
{
	*pPoint = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach]->m_vPos;
	return TRUE;
}


//-----------------------------------------------------------------------------
// attempts to place bot into vehicle at specified station.  if STATION_STATUS_ENTERING is
// returned, then operation succeeded.  Other return values indicate why bot could not 
// enter the station.
// If bProximityCheck is TRUE, bot will only enter vehicle if he's standing in the station 
// entry area.  If bProximityCheck is FALSE, then bot will enter regardless of his location
// relative to the vehicle.
CVehicle::StationStatus_e CVehicleLoader::EnterStation( CBot *pBot, Station_e eStation, BOOL bProximityCheck, BOOL bImmediately ) {
	if( m_eStationStatus != STATION_STATUS_EMPTY ) {
		return m_eStationStatus;
	}

	StationStatus_e status = CanOccupyStation( pBot, STATION_DRIVER );

	if( bProximityCheck ) {
		if( status != STATION_STATUS_EMPTY ) {
			return status;
		}
	} else {
		if( status != STATION_STATUS_EMPTY && status != STATION_STATUS_OUT_OF_RANGE ) {
			return status;
		}
	}

	m_pDriverBot = pBot;

	if( bImmediately ) {
		DriverEnter( m_pDriverBot, m_apszBoneNameTable[BONE_DRIVER_ATTACH] );

		m_eStationStatus = STATION_STATUS_OCCUPIED;	//necessary if not calling enter station
		m_eCameraState = CAMERA_STATE_IN_VEHICLE;  //Necessary if not calling enter station
		if (m_pDriverBot->IsPlayerBot())
		{
			m_pCameraBot = m_pDriverBot;
			gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pCameraBot->m_nPossessionPlayerIndex, this );
		}

		return STATION_STATUS_ENTERING;
	} else {
		CFVec3A vJumpVel;
		m_fEntryJumpTimer = ComputeEntryTrajectory( m_pDriverBot->MtxToWorld()->m_vPos,											// start pos
												m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach]->m_vPos,				// end pos
												3.0f,																			// minimum jump height, 
												m_pDriverBot->m_pBotInfo_Gen->fGravity * m_pDriverBot->m_fGravityMultiplier,	// gravity
												vJumpVel );																		// output velocity

		m_pDriverBot->StartVelocityJump( &vJumpVel );
		m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
		m_pDriverBot->SetNoCollideStateAir( TRUE );
		m_eStationStatus = STATION_STATUS_PRE_ENTERING;

		return STATION_STATUS_ENTERING;
	}
}


CVehicle::StationStatus_e CVehicleLoader::ExitStation( CBot *pBot, BOOL bImmediately ) {
	if( m_eStationStatus != STATION_STATUS_OCCUPIED ) {
		return m_eStationStatus;
	}

	if( pBot == NULL ) {
		return STATION_STATUS_WRONG_BOT;
	}

	if( pBot != m_pDriverBot ) {
		return STATION_STATUS_WRONG_BOT;
	}

	if( bImmediately ) {
		if (m_pDriverBot && m_pDriverBot->IsCreated() ) {
			if (m_pDriverBot && m_pDriverBot->IsPlayerBot()) {
				gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pDriverBot->m_nPossessionPlayerIndex, m_pDriverBot );
			}
			DriverExit(m_pDriverBot);
		}
		m_pDriverBot = NULL;
		m_eStationStatus = STATION_STATUS_EMPTY;	//necessary if not calling exitstation
		m_eCameraState = CAMERA_STATE_INACTIVE;  //Necessary if not calling exit station
		m_pCameraBot = NULL;

	} else {
		FASSERT( m_pDriverBot != NULL );
		DriverExit( m_pDriverBot );
	}

	return STATION_STATUS_EXITING;
}


void CVehicleLoader::_HandleDriverState( void ) {
	switch( m_eStationStatus ) {
		case STATION_STATUS_PRE_ENTERING:
			m_fEntryJumpTimer -= FLoop_fPreviousLoopSecs;
			if( m_pDriverBot->IsDeadOrDying() || IsUpsideDown() )
			{
				// cancel jumping entry if bot dies
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
				m_pDriverBot->SetNoCollideStateAir( FALSE );
				m_eStationStatus = STATION_STATUS_EXITING;
				m_pDriverBot = NULL;
			}
			else if( m_fEntryJumpTimer < 0.0f )
			{
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
				m_pDriverBot->SetNoCollideStateAir( FALSE );
				DriverEnter( m_pDriverBot, m_apszBoneNameTable[BONE_DRIVER_ATTACH] );
				m_eStationStatus = STATION_STATUS_ENTERING;
			}
			break;

		case STATION_STATUS_ENTERING:	  //
			m_eStationStatus = STATION_STATUS_OCCUPIED;
			//if( !m_pDriverBot->SwitchingWeapons() ) {
			//	// if we're done switching weapons, go ahead and start the engines and switch the reticle

			//	// now deal with the reticle... save off previous state and switch to loader reticle
			//	if( m_nPossessionPlayerIndex > -1 ) {
			//		if( m_pReticle && !m_pReticle->IsDrawEnabled() ) {
			//			m_pReticle->SetNormOrigin( _RETICLE_NORM_X, _RETICLE_NORM_Y );
			//			m_pReticle->SetType( CReticle::TYPE_TANK_CANNON );
			//			m_pReticle->EnableDraw( TRUE );

			//			// if player is driving, set up new reticle and save info about previous reticle
			//			CReticle *pReticle	= &(Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle);
			//			m_fReticlePrevX		= pReticle->GetNormOriginX();
			//			m_fReticlePrevY		= pReticle->GetNormOriginY();
			//			m_bReticleDrawPrev	= pReticle->IsDrawEnabled();
			//			m_ReticlePrevType	= pReticle->GetType();

			//			pReticle->SetNormOrigin( _RETICLE_NORM_X, _RETICLE_NORM_Y );
			//			pReticle->SetType( CReticle::TYPE_DROID_STANDARD );
			//			pReticle->EnableDraw( TRUE );
			//		}
			//	}
			//	m_eStationStatus = STATION_STATUS_OCCUPIED;
			//}
			break;

		case STATION_STATUS_EXITING:
			if( m_eCameraState == CAMERA_STATE_INACTIVE ) {
				m_eStationStatus = STATION_STATUS_EMPTY;
			}
			break;

		case STATION_STATUS_OCCUPIED:
			SetVehicleReticle( m_pDriverBot, CReticle::TYPE_DROID_STANDARD, _RETICLE_NORM_X, _RETICLE_NORM_Y ); 
			break;
	}
}



#define _USERPROPS_CAMRATE_IN ( 1.0f )
#define _USERPROPS_CAMRATE_OUT ( 1.0f )


void CVehicleLoader::_CameraTransitionWork( void ) {
	CFCamera *pCamera;

	//	CBot	 *pTransToBot, *pTransFromBot;	
	////DEBUG
	//pCamera = gamecam_GetActiveCamera();
	//CFVec3A vCPos = pCamera->GetFinalXfm()->m_MtxR.m_vPos;

	if( (m_eCameraState == CAMERA_STATE_IN_VEHICLE) || (m_eCameraState == CAMERA_STATE_INACTIVE) ) {
		return;
	}
	if( m_pCameraBot == NULL || !m_pCameraBot->IsPlayerBot()) {
		m_eCameraState = CAMERA_STATE_INACTIVE;
		return;
	}

	m_CamInfo.m_pmtxMtx	= &m_ManCamMtx;
	gamecam_CamBotTransInfo_t	cbinfo;

	switch( m_eCameraState ) {
		case CAMERA_STATE_START_ENTER_VEHICLE:
			m_fUnitCameraTransition = 0.0f;
			m_eCameraState			= CAMERA_STATE_ENTERING_VEHICLE;
			pCamera					= gamecam_GetActiveCamera();
			m_ManCamMtx				= pCamera->GetFinalXfm()->m_MtxR;
			m_TransCamMtx			= pCamera->GetFinalXfm()->m_MtxR;
			pCamera->GetFOV( &m_CamInfo.m_fHalfFOV );
			return;

		case CAMERA_STATE_START_EXIT_VEHICLE:
			m_fUnitCameraTransition = 0.0f;
			m_eCameraState			= CAMERA_STATE_EXITING_VEHICLE;
			pCamera					= gamecam_GetActiveCamera();
			m_ManCamMtx				= pCamera->GetFinalXfm()->m_MtxR;
			m_TransCamMtx			= pCamera->GetFinalXfm()->m_MtxR;
			pCamera->GetFOV( &m_CamInfo.m_fHalfFOV );
			return;

		case CAMERA_STATE_ENTERING_VEHICLE:
			m_fUnitCameraTransition += FLoop_fPreviousLoopSecs * _USERPROPS_CAMRATE_IN;
			if( m_fUnitCameraTransition > 1.0f ) {
				gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pCameraBot->m_nPossessionPlayerIndex, this );
				m_eCameraState		= CAMERA_STATE_IN_VEHICLE;
				return;
			}
			cbinfo.pBot1	= m_pCameraBot;
			cbinfo.pBot2	= this;
			cbinfo.uPlayer	= m_pCameraBot->m_nPossessionPlayerIndex;
			break;

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

/*
//-----------------------------------------------------------------------------
// callback for fcoll collision.  Returns TRUE if collision should be performed
// on pTracker, FALSE if not.
u32 CVehicleLoader::_IntersectingTrackerCallback( CFWorldTracker *pTracker ) {
	for( u32 i=0; i < FWorld_nTrackerSkipListCount; i++ )
	{
		if( FWorld_apTrackerSkipList[i] == pTracker )
		{
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		}
	}
	return FCOLL_CHECK_CB_ALL_IMPACTS;
}
*/

void CVehicleLoader::_LoaderCollisionDetect( const CFPhysicsObject::PhysicsState_t *pState ) {
	FASSERT( m_pCBLoader && (m_pCBLoader->TypeBits() & ENTITY_BIT_VEHICLELOADER) );
	
	m_pCBLoader->m_fMaxUnitClawAngle = 1.0f;		// set this each frame
	FCollImpact_t *pImpact;

	CFPhysicsObject::CollisionPoint_t *paPoints = m_pCBLoader->m_aColPoints;

	FWorld_nTrackerSkipListCount = 0;
	m_pCBLoader->AppendTrackerSkipList();

	CFCollData	colldata;
	CFSphere	startSphere, endSphere;
	BOOL		bIgnoreCollision;
	u32			uColIdx;
	CFVec3A		vMovement = CFVec3A::m_Null;
	u32 i;


	colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE;
	colldata.nCollMask					= FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_VEHICLES;
	colldata.nTrackerUserTypeBitsMask	= _VEHICLE_COLLISION_MASK;
	colldata.pCallback					= NULL;//_IntersectingTrackerCallback;
	colldata.pLocationHint				= m_pCBLoader->m_pWorldMesh;
	colldata.pMovement					= &vMovement;
	colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
	colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;


	for( i=0; i<COLPOINT_COUNT; i++ ) {
		fcoll_Clear();
		m_pCBLoader->_GetColSphere( i, &startSphere, m_pCBLoader->m_MtxToWorld );
		CFMtx43A::m_Temp.Set( pState->mOrientation.m_vRight, pState->mOrientation.m_vUp, pState->mOrientation.m_vFront, pState->vPosition );
		m_pCBLoader->_GetColSphere( i, &endSphere, CFMtx43A::m_Temp );
        vMovement.Sub( endSphere.m_Pos, startSphere.m_Pos );

		//fdraw_DevSphere( &startSphere.m_Pos, startSphere.m_fRadius, &FColor_MotifRed, 4, 4, 4 );
		//fdraw_DevSphere( &endSphere.m_Pos, endSphere.m_fRadius, &FColor_MotifBlue, 4, 4, 4 );

		// this is kind of pointless
		//if( i == COLPOINT_CLAW_CENTER ) {
		//	colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE; // | FCOLL_DATA_FIRST_IMPACT_ONLY;
		//	colldata.nTrackerUserTypeBitsMask	= _VEHICLE_COLLISION_MASK; ////FCOLL_USER_TYPE_BITS_ALL;
		//} else {
		//	colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE;
		//	colldata.nTrackerUserTypeBitsMask	= _VEHICLE_COLLISION_MASK;
		//}

		fcoll_Check( &colldata, &startSphere );
		bIgnoreCollision = TRUE;
		uColIdx = 0;
		fcoll_Sort( FALSE );

		while( bIgnoreCollision && uColIdx < FColl_nImpactCount ) {
			pImpact = FColl_apSortedImpactBuf[uColIdx];
			bIgnoreCollision = FALSE;

			paPoints[i].pOtherObj	= NULL;

			// see whether we've hit a bot
			CFWorldTracker *pHitTracker = (CFWorldTracker*)pImpact->pTag;
			if( ((i==COLPOINT_CLAW_LEFT) || (i==COLPOINT_CLAW_RIGHT) || (i==COLPOINT_CLAW_CENTER)) && pHitTracker ) {
				if( pHitTracker->GetType() == FWORLD_TRACKERTYPE_MESH ) {
					if( pHitTracker->m_nUser == MESHTYPES_ENTITY ) {
						CEntity *pEntity = (CEntity*)(pHitTracker->m_pUser);
						if( pEntity == m_pCBLoader->m_pClawedEntity ) {
							bIgnoreCollision = TRUE;
						}
					}
				}
			}
			uColIdx++;
		}

		if( !bIgnoreCollision ) {
			paPoints[i].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_COLLIDED;
			if( m_pCBLoader->m_eEngineState != ENGINE_STATE_RUNNING ) {
				paPoints[i].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_APPLY_FRICTION;
			}
			paPoints[i].fTimeSinceCollision = 0.0f;
			paPoints[i].vNormal		= pImpact->UnitFaceNormal;
			paPoints[i].vPoint		= pImpact->ImpactPoint;
			paPoints[i].vPoint.Sub( pState->vPosition );
			paPoints[i].fDepth		= pImpact->fImpactDistInfo;
			paPoints[i].vPush		= pImpact->PushUnitVec;
			paPoints[i].pmFrictOr	= &(m_pCBLoader->m_MtxToWorld);

			CEntity *pEntity = CGColl::ExtractEntity( pImpact );
			if( pEntity && pEntity->TypeBits() & ENTITY_BIT_VEHICLE ) {
				paPoints[i].pOtherObj = &(((CVehicle*)pEntity)->m_PhysObj);
			} else {
				paPoints[i].pOtherObj = NULL;
			}

			if( i == COLPOINT_OBJECT ) {
				if( m_pCBLoader->m_pClawedEntity && m_pCBLoader->m_pClawedEntity->TypeBits() & ENTITY_BIT_BOT ) {

					// if we have a bot grabbed and we hit something, do some damage
					f32 fImpactSpeed = pImpact->PushUnitVec.Dot( m_pCBLoader->m_Velocity_WS );

					// make sure we're moving into the wall fast, and only do this once per frame
					if( (fImpactSpeed < -50.0f) ) {
						if( ((CBot*)m_pCBLoader->m_pClawedEntity)->GetPartMgr() != NULL ) {
							// DEVPRINTF( "Clawed bot hit something, damaging limb...\n" );
							m_pCBLoader->JustFired();		// let the AI know we're doing something suspicious

							// If we are carrying Glitch (most likely multiplayer) damage him but
							// don't dangle any limbs
							if (m_pCBLoader->m_pClawedEntity->TypeBits() & ENTITY_BIT_BOTGLITCH)
								m_pCBLoader->_CauseCrushDamage((CBot*)m_pCBLoader->m_pClawedEntity);
							else {
								((CBot*)m_pCBLoader->m_pClawedEntity)->GetPartMgr()->SetRandomLimb_Dangle();

								if( fmath_RandomChoice(2) ) {
									((CBot*)m_pCBLoader->m_pClawedEntity)->GetPartMgr()->SetRandomLimb_Dangle();
								}
							}
						}
					}
				}
			} else if( (i== COLPOINT_CLAW_CENTER) ) {
				// paPoints[i*_COLL_PER_POINT+j].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_ADD_COLLIDE_VELOCITY;
				m_pCBLoader->m_fMaxUnitClawAngle = m_pCBLoader->m_fUnitClawAngle;
				f32 fVel = (m_pCBLoader->m_fUnitClawAngle - m_pCBLoader->m_fLastUnitClawAngle) * FLoop_fPreviousLoopOOSecs * _CLAW_CENTER_THROW;
				FMATH_CLAMP( fVel, 0.0f, 5.0f );
				paPoints[i].vCollideVel.Mul( m_pCBLoader->m_MtxToWorld.m_vFront, fVel );
			}
		} else {
			paPoints[i].uClientFlags = 0;
			paPoints[i].fTimeSinceCollision += FLoop_fPreviousLoopSecs;
			paPoints[i].pOtherObj = NULL;
		}
	}


#if 0
	m_pCBLoader->m_fMaxUnitClawAngle = 1.0f;		// set this each frame

	FCollImpact_t *pImpact;

	CFPhysicsObject::CollisionPoint_t *paPoints = m_pCBLoader->m_aColPoints;

	FWorld_nTrackerSkipListCount = 0;
	m_pCBLoader->AppendTrackerSkipList();

	CFCollData	colldata;
	CFSphere	collsphere;
	CFVec3A		vMovement = CFVec3A::m_Null;
	BOOL		bIgnoreCollision;
	u32			uColIdx;


	colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE;		// set this later
	colldata.nCollMask					= FCOLL_MASK_CHECK_ALL;
//	colldata.nTrackerUserTypeBitsMask	= _VEHICLE_COLLISION_MASK;			// set this later
	colldata.pCallback					= NULL;//_IntersectingTrackerCallback;
	colldata.pLocationHint				= NULL;
	colldata.pMovement					= &vMovement;
	colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
	colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;
    
	for( u32 i=0; i<COLPOINT_COUNT; i++ ) {
		fcoll_Clear();
		m_pCBLoader->_GetColSphere( i, &collsphere );

		if( i == COLPOINT_CLAW_CENTER ) {
			colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE; // | FCOLL_DATA_FIRST_IMPACT_ONLY;
			colldata.nTrackerUserTypeBitsMask	= _VEHICLE_COLLISION_MASK; ////FCOLL_USER_TYPE_BITS_ALL;
		} else {
			colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE;
			colldata.nTrackerUserTypeBitsMask	= _VEHICLE_COLLISION_MASK;
		}

        fcoll_Check( &colldata, &collsphere );

		bIgnoreCollision = TRUE;
		uColIdx = 0;
		fcoll_Sort( FALSE );

		for( u32 j=0; j<_COLL_PER_POINT; j++ ) {
			while( bIgnoreCollision && uColIdx < FColl_nImpactCount ) {
	        
				pImpact = FColl_apSortedImpactBuf[uColIdx];
				bIgnoreCollision = FALSE;

				paPoints[i*_COLL_PER_POINT+j].pOtherObj	= NULL;

				// see whether we've hit a bot
				CFWorldTracker *pHitTracker = (CFWorldTracker*)pImpact->pTag;
				if( ((i==COLPOINT_CLAW_LEFT) || (i==COLPOINT_CLAW_RIGHT) || (i==COLPOINT_CLAW_CENTER)) && pHitTracker ) {
					if( pHitTracker->GetType() == FWORLD_TRACKERTYPE_MESH ) {
						if( pHitTracker->m_nUser == MESHTYPES_ENTITY ) {
							CEntity *pEntity = (CEntity*)(pHitTracker->m_pUser);
							if( pEntity == m_pCBLoader->m_pClawedEntity ) {
								bIgnoreCollision = TRUE;
							}

							if( pEntity->TypeBits() & ENTITY_BIT_VEHICLE ) {
								paPoints[i].pOtherObj = &(((CVehicle*)pEntity)->m_PhysObj);
							}
						}
					}
				}
				uColIdx++;
			}

			if( !bIgnoreCollision ) {
				paPoints[i*_COLL_PER_POINT+j].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_COLLIDED;
				if( m_pCBLoader->m_eEngineState != ENGINE_STATE_RUNNING ) {
					paPoints[i*_COLL_PER_POINT+j].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_APPLY_FRICTION;
				}
				paPoints[i*_COLL_PER_POINT+j].fTimeSinceCollision = 0.0f;
				paPoints[i*_COLL_PER_POINT+j].vNormal		= pImpact->UnitFaceNormal;
				paPoints[i*_COLL_PER_POINT+j].vPoint		= pImpact->ImpactPoint;
				paPoints[i*_COLL_PER_POINT+j].vPoint.Sub( pState->vPosition );
				paPoints[i*_COLL_PER_POINT+j].fDepth		= pImpact->fImpactDistInfo;
				paPoints[i*_COLL_PER_POINT+j].vPush		= pImpact->PushUnitVec;
				paPoints[i*_COLL_PER_POINT+j].pmFrictOr	= &(m_pCBLoader->m_MtxToWorld);
				
				if( i == COLPOINT_OBJECT ) {
					if( m_pCBLoader->m_pClawedEntity && m_pCBLoader->m_pClawedEntity->TypeBits() & ENTITY_BIT_BOT ) {
						// if we have a bot grabbed and we hit something, do some damage
						f32 fImpactSpeed = pImpact->PushUnitVec.Dot( m_pCBLoader->m_Velocity_WS );
//						DEVPRINTF( "fImpactSpeed = %0.2f\n", fImpactSpeed );
						// make sure we're moving into the wall fast, and only do this once per frame
						if( (fImpactSpeed < -50.0f) && (j == 0) ) {
							if( ((CBot*)m_pCBLoader->m_pClawedEntity)->GetPartMgr() != NULL ) {
//								DEVPRINTF( "Clawed bot hit something, damaging limb...\n" );
								m_pCBLoader->JustFired();		// let the AI know we're doing something suspicious


								// If we are carrying Glitch (most likely multiplayer) damage him but
								// don't dangle any limbs
								if (m_pCBLoader->m_pClawedEntity->TypeBits() & ENTITY_BIT_BOTGLITCH)
									m_pCBLoader->_CauseCrushDamage((CBot*)m_pCBLoader->m_pClawedEntity);
								else {
									((CBot*)m_pCBLoader->m_pClawedEntity)->GetPartMgr()->SetRandomLimb_Dangle();

									if( fmath_RandomChoice(2) ) {
										((CBot*)m_pCBLoader->m_pClawedEntity)->GetPartMgr()->SetRandomLimb_Dangle();
									}
								}
							}
						}
					}
				} else if( (i== COLPOINT_CLAW_CENTER) ) {
//					paPoints[i*_COLL_PER_POINT+j].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_ADD_COLLIDE_VELOCITY;
					m_pCBLoader->m_fMaxUnitClawAngle = m_pCBLoader->m_fUnitClawAngle;
					f32 fVel = (m_pCBLoader->m_fUnitClawAngle - m_pCBLoader->m_fLastUnitClawAngle) * FLoop_fPreviousLoopOOSecs * _CLAW_CENTER_THROW;
					FMATH_CLAMP( fVel, 0.0f, 5.0f );
					paPoints[i*_COLL_PER_POINT+j].vCollideVel.Mul( m_pCBLoader->m_MtxToWorld.m_vFront, fVel );
				}
			} else {
				paPoints[i*_COLL_PER_POINT+j].uClientFlags = 0;
				paPoints[i*_COLL_PER_POINT+j].fTimeSinceCollision += FLoop_fPreviousLoopSecs;
				paPoints[i*_COLL_PER_POINT+j].pOtherObj = NULL;
			}
		}
	}

#endif
}

void CVehicleLoader::_CauseCrushDamage( CBot* pBot ) {
	// Get an empty damage form...
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	if( pDamageForm ) {
		// Fill out the form...

		pDamageForm->m_pDamageeEntity	= pBot;
		pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_AMBIENT;
		pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_pDamageProfile	= m_pBotInfo_Vehicle->pSecondaryWeaponDamageProfile;
		pDamageForm->m_Damager.pBot		= this;
		pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;

		CDamage::SubmitDamageForm( pDamageForm );
	}
}


void CVehicleLoader::MoveVehicle( void ) {
	const CFMtx43A* pmtxPhysOr;
	const CFVec3A*	pvPhysVel;

	pmtxPhysOr	= m_PhysObj.GetOrientation();
	pvPhysVel	= m_PhysObj.GetVelocity();

}

//static CFVec3A	_CMEX;
//static CFVec3A	_CMEZ;
//static CFVec3A	_CMEPos;
//
//static CFVec3A  _vCP0;
//static CFVec3A  _vCP1;


BOOL CVehicleLoader::_ValidateClawableObject( CMeshEntity *pME ) {
	if( ClawIsClosed() ) {
		return (pME == m_pClawedEntity);
		m_ClawEntityOrientMtx = *pME->MtxToWorld();
	}

	// if it's a bot, it's easy, just update here and get out...
	if( pME->TypeBits() & ENTITY_BIT_BOT ) {
		if( !((CBot*)pME)->m_pBotInfo_Gen->uLoaderPickupEnabled ) {
			return FALSE;
		}

		// there is a bot and pickup is enabled...
		m_ClawEntityOrientMtx = *pME->MtxToWorld();
		m_ClawEntityOrientMtx.m_vPos.Mul( pME->MtxToWorld()->m_vUp, -((CBot*)pME)->m_pBotInfo_Gen->fCollSphere1Y_MS );
		m_ClawEntityOrientMtx.m_vPos.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos );
		m_fClawedEntitySpan = 0.0f; //((CBot*)pME)->m_pBotInfo_Gen->fCollSphere1Radius_MS * 2.0f;
		return TRUE;
	}

	f32		fDotZ, fDotY, fDotX;
	f32		fD1, fD2;
	

	fDotZ = pME->MtxToWorld()->m_vFront.Dot( m_MtxToWorld.m_vRight );
	fDotY = pME->MtxToWorld()->m_vUp.Dot( m_MtxToWorld.m_vRight );
	fDotX = pME->MtxToWorld()->m_vRight.Dot( m_MtxToWorld.m_vRight );

	if( (FMATH_FABS( fDotZ ) > FMATH_FABS( fDotY )) && FMATH_FABS( fDotZ ) > FMATH_FABS( fDotX ) ) {
		m_fClawedEntitySpan = pME->GetMeshInst()->m_pMesh->vBoundBoxMax_MS.z - pME->GetMeshInst()->m_pMesh->vBoundBoxMin_MS.z;
		//Z
		m_ClawEntityOrientMtx.m_vFront = m_MtxToWorld.m_vRight;

		if( fDotZ < 0.0f ) {
			m_ClawEntityOrientMtx.m_vFront.Negate();
		}

		fD1 = pME->MtxToWorld()->m_vUp.Dot( m_MtxToWorld.m_vUp );
		fD2 = pME->MtxToWorld()->m_vRight.Dot( m_MtxToWorld.m_vUp );

		//Y & X
		if( FMATH_FABS( fD1 ) > FMATH_FABS( fD2 ) ) {
			m_ClawEntityOrientMtx.m_vUp = m_MtxToWorld.m_vUp;
			if( fD1 < 0.0f ) {
				m_ClawEntityOrientMtx.m_vUp.Negate();
			}
			m_ClawEntityOrientMtx.m_vRight.Cross( m_ClawEntityOrientMtx.m_vUp, m_ClawEntityOrientMtx.m_vFront );
		} else {
			m_ClawEntityOrientMtx.m_vRight = m_MtxToWorld.m_vUp;
			if( fD2 < 0.0f ) {
				m_ClawEntityOrientMtx.m_vRight.Negate();
			}
			m_ClawEntityOrientMtx.m_vUp.Cross( m_ClawEntityOrientMtx.m_vFront, m_ClawEntityOrientMtx.m_vRight );
		}
	} else if( FMATH_FABS( fDotY ) > FMATH_FABS( fDotX ) ) {
		//Y
		m_fClawedEntitySpan = pME->GetMeshInst()->m_pMesh->vBoundBoxMax_MS.y - pME->GetMeshInst()->m_pMesh->vBoundBoxMin_MS.y;

		m_ClawEntityOrientMtx.m_vUp = m_MtxToWorld.m_vRight;
		if( fDotY < 0.0f ) {
			m_ClawEntityOrientMtx.m_vUp.Negate();
		}

		//Z & X
		fD1 = pME->MtxToWorld()->m_vFront.Dot( m_MtxToWorld.m_vUp );
		fD2 = pME->MtxToWorld()->m_vRight.Dot( m_MtxToWorld.m_vUp );
		if( FMATH_FABS( fD1 ) > FMATH_FABS( fD2 ) ) {
			m_ClawEntityOrientMtx.m_vFront = m_MtxToWorld.m_vUp;
			if( fD1 < 0.0 ) {
				m_ClawEntityOrientMtx.m_vFront.Negate();
			}
			m_ClawEntityOrientMtx.m_vRight.Cross( m_ClawEntityOrientMtx.m_vUp, m_ClawEntityOrientMtx.m_vFront );
		} else {
			m_ClawEntityOrientMtx.m_vRight = m_MtxToWorld.m_vUp;
			if( fD2 < 0.0f ) {
				m_ClawEntityOrientMtx.m_vRight.Negate();
			}
			m_ClawEntityOrientMtx.m_vFront.Cross( m_ClawEntityOrientMtx.m_vRight, m_ClawEntityOrientMtx.m_vUp );
		}
	} else {
		//X
		m_fClawedEntitySpan = pME->GetMeshInst()->m_pMesh->vBoundBoxMax_MS.x - pME->GetMeshInst()->m_pMesh->vBoundBoxMin_MS.x;

		m_ClawEntityOrientMtx.m_vRight = m_MtxToWorld.m_vRight;
		if( fDotX < 0.0f ) {
			m_ClawEntityOrientMtx.m_vRight.Negate();
		}

		fD1 = pME->MtxToWorld()->m_vUp.Dot( m_MtxToWorld.m_vUp );
		fD2 = pME->MtxToWorld()->m_vFront.Dot( m_MtxToWorld.m_vUp );

		//Y
		if( FMATH_FABS( fD1 ) > FMATH_FABS( fD2 ) ) {
			m_ClawEntityOrientMtx.m_vUp = m_MtxToWorld.m_vUp;
			if( fD1 < 0.0f ) {
				m_ClawEntityOrientMtx.m_vUp.Negate();
			}
			m_ClawEntityOrientMtx.m_vFront.Cross( m_ClawEntityOrientMtx.m_vRight, m_ClawEntityOrientMtx.m_vUp );
		} else {
			m_ClawEntityOrientMtx.m_vFront = m_MtxToWorld.m_vUp;
			if( fD2 < 0.0f ) {
				m_ClawEntityOrientMtx.m_vFront.Negate();
			}
			m_ClawEntityOrientMtx.m_vUp.Cross( m_ClawEntityOrientMtx.m_vFront, m_ClawEntityOrientMtx.m_vRight );
		}
	}

//	if( m_fClawedEntitySpan > _CLAW_RADIUS * 2.0f ) {
	if( m_fClawedEntitySpan > m_fClawSpan ) {
		return FALSE;
	}

	m_ClawEntityOrientMtx.m_vPos = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos;

	if( m_ClawEntityOrientMtx.m_vPos.DistSq( pME->MtxToWorld()->m_vPos ) < (m_fClawedEntitySpan * m_fClawedEntitySpan * 0.16f) ) {
		return _CheckForStackedObject();
	} else {
		return FALSE;
	}
}

#if 0
BOOL CVehicleLoader::_StackedObjectTrackerCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	for( u32 i=0; i < FWorld_nTrackerSkipListCount; i++ ) {
		if( FWorld_apTrackerSkipList[i] == pTracker ) {
			return FALSE;
		}
	}

	if( pTracker == ((CMeshEntity*)m_pCBLoader->m_pClawedEntity)->GetMeshInst() ) {
		return TRUE;
	}

	CFCollInfo collinfo;
#if 0
	CFVec3A	vCylTop;
	vCylTop.Mul( m_pCBLoader->m_MtxToWorld.m_vUp, m_CBSphere.m_fRadius * 1.5f );
	vCylTop.Add( m_CBSphere.m_Pos );
	
	collinfo.bCalculateImpactData		= TRUE;
	collinfo.bCullBacksideCollisions	= TRUE;
	collinfo.bFindClosestImpactOnly		= TRUE;
	collinfo.nStopOnFirstOfCollMask		= FCOLL_MASK_NONE;
	collinfo.nCollMask					= FCOLL_MASK_CHECK_ALL;
	collinfo.nCollTestType				= FMESH_COLLTESTTYPE_CYLINDER;
	collinfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	collinfo.pTag						= NULL;
	collinfo.Cylinder.Init( &m_CBSphere.m_Pos, &vCylTop.v3, m_CBSphere.m_fRadius * 0.25f );
#else
	collinfo.pSphere_WS					= &m_CBSphere;
	collinfo.bCalculateImpactData		= TRUE;
	collinfo.bCullBacksideCollisions	= TRUE;
	collinfo.bFindClosestImpactOnly		= TRUE;
	collinfo.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	collinfo.nStopOnFirstOfCollMask		= FCOLL_MASK_NONE;
	collinfo.nCollMask					= FCOLL_MASK_CHECK_ALL;
	collinfo.nCollTestType				= FMESH_COLLTESTTYPE_SPHERE;
	collinfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	collinfo.pTag						= NULL;
#endif
	u32 uLastCollCount = FColl_nImpactCount;
	CFVec3A vCollPt;
	((CFWorldMesh*)pTracker)->CollideWithMeshTris( &collinfo );
	if( FColl_nImpactCount > uLastCollCount ) {
		vCollPt = FColl_aImpactBuf[uLastCollCount].ImpactPoint;
	} else {
		return TRUE;
	}

	m_pCBLoader->m_pClawedEntity = NULL;

	return FALSE;
}
#endif


BOOL CVehicleLoader::_CheckForStackedObject( void ) {
	FASSERT( m_pClawedEntity && (m_pClawedEntity->TypeBits() & ENTITY_BIT_MESHENTITY) );

	// Remove once capsule collision
//	return TRUE;

	CMeshEntity *pME = (CMeshEntity*)m_pClawedEntity;

	CFCollData colldata;
//	CFVec3A v;
//	v.Zero();

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();
	FASSERT( FWorld_nTrackerSkipListCount + 1 < FWORLD_MAX_SKIPLIST_ENTRIES );
	FWorld_apTrackerSkipList[FWorld_nTrackerSkipListCount++] = pME->GetMeshInst();

	colldata.nCollMask					= FCOLL_MASK_CHECK_ALL;
	colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE; // | FCOLL_DATA_DONT_CALCULATE_IMPACT;
	colldata.nStopOnFirstOfCollMask		= FCOLL_MASK_CHECK_ALL;
	colldata.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	colldata.pCallback					= NULL;//_IntersectingTrackerCallback;
	colldata.pLocationHint				= m_pWorldMesh;
	colldata.pMovement					= NULL;
	colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
	colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;

	CFCapsule capsule;
	capsule.m_fRadius = pME->GetMeshInst()->m_pMesh->BoundSphere_MS.m_fRadius;
	capsule.m_vPoint1.Set( pME->GetMeshInst()->m_pMesh->BoundSphere_MS.m_Pos );
	capsule.m_vPoint1.Add( m_pClawedEntity->MtxToWorld()->m_vPos.v3 );
	capsule.m_vPoint2.Mul( CFVec3A::m_UnitAxisY, capsule.m_fRadius ).Add( capsule.m_vPoint1 );
	capsule.m_vPoint2.x += 0.25f;
	capsule.m_vPoint2.z += 0.25f;
	capsule.m_fRadius *= 0.25f;

#if SAS_ACTIVE_USER == SAS_USER_JOHN
//	fdraw_DevCapsule( &capsule, &FColor_MotifBlue, 4, 4, 4 );
#endif

	fcoll_Clear();
	if( !fcoll_Check( &colldata, &capsule ) ) {
		return TRUE;
	} else {
		m_pClawedEntity = NULL;
		return FALSE;	
	}
#if 0
	FASSERT( m_pClawedEntity && (m_pClawedEntity->TypeBits() & ENTITY_BIT_MESHENTITY) );

	m_CBSphere = ((CMeshEntity*)m_pClawedEntity)->GetMeshInst()->m_pMesh->BoundSphere_MS;
	m_CBSphere.m_fRadius *= 0.5f;
	m_CBSphere.m_Pos.Add( m_pClawedEntity->MtxToWorld()->m_vPos.v3 );
	m_CBSphere.m_Pos.y += m_CBSphere.m_fRadius;

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();
	fcoll_Clear();
	m_pCBLoader = this;
	fworld_FindTrackersIntersectingSphere( &m_CBSphere, FWORLD_TRACKERTYPE_MESH, _StackedObjectTrackerCB, 
						FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, NULL, MESHTYPES_ENTITY, FCOLL_USER_TYPE_BITS_ALL, FALSE );
	m_pCBLoader = NULL;

	return !!m_pClawedEntity;
#endif
}


void CVehicleLoader::DebugDraw( CVehicleLoader *pVoid ) {
}


u32 CVehicleLoader::_GetFreeVerletFromPool( void ) {
	u32 i;
	u32 uVerIdx = -1;

	for( i=0; i<_VERLET_COUNT; i++ ) {
		if( !m_abVerletPoolActive[i] ) {
			uVerIdx = i;
			break;
		}
	}

	if( uVerIdx > _VERLET_COUNT ) {
		// we didn't find a free one.  Find the one that's moving slowest
		f32 fSlowest = m_avVerletObjectAveSpeed[0].MagSq();
		u32 uSlowest = 0;
		f32 fTmp;

		for( i=1; i<_VERLET_COUNT; i++ ) {
			fTmp = m_avVerletObjectAveSpeed[i].MagSq();
			if( fTmp < fSlowest ) {
				fSlowest = fTmp;
				uSlowest = i;
			}
		}

		uVerIdx = uSlowest;

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "No free verlet objects, returning idx %d, speed was %0.2f\n", uVerIdx, fSlowest );
#endif
	}

	FASSERT( uVerIdx < _VERLET_COUNT );
    

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "CVehicleLoader::_GetFreeVerletFromPool():  Returning verlet object %d\n", uVerIdx );
#endif
	return uVerIdx;
}

void CVehicleLoader::_InitAndStartVerlet( CMeshEntity *pME ) {   
	CFVec3A vDim;
	u32 idx = _GetFreeVerletFromPool();
	FASSERT( idx < _VERLET_COUNT );

	vDim.Sub( pME->GetMeshInst()->m_pMesh->vBoundBoxMax_MS, pME->GetMeshInst()->m_pMesh->vBoundBoxMin_MS );
	m_abVerletPoolActive[idx] = TRUE;
	m_avVerletObjectAveSpeed[idx].Set( 50.0f, 50.0f, 50.0f );
	m_apVerletPoolME[idx] = pME;

	m_apVerletPool[idx]->InitUnitMtx( pME->MtxToWorld() );
	m_apVerletPool[idx]->SetMass( 100.0f );
	m_apVerletPool[idx]->SetObjectDim( vDim );
	m_apVerletPool[idx]->m_pUser = pME;
	m_apVerletPool[idx]->SetWorldMesh( pME->GetMeshInst() );
	m_apVerletPool[idx]->SetUnanchoredCollType( CFVerlet::UNANCHORED_COLL_TYPE_KDOP );
	m_apVerletPool[idx]->Reset();
	m_apVerletPool[idx]->ClearForces();
	m_apVerletPool[idx]->ClearImpulses();
	m_apVerletPool[idx]->EnableWork( TRUE );
	m_apVerletPool[idx]->Net_Kickstart();
	m_apVerletPool[idx]->Net_MoveToActiveList();

	// give the crate some velocity...
	CFVec3A vImpulse;
	CFVec3A vImpulseLoc;

	vImpulseLoc.Sub( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxVictimAttach]->m_vPos, m_MtxToWorld.m_vPos );
	m_PhysObj.GetVelocity( &vImpulseLoc, &vImpulse );
	vImpulse.Mul( 100.0f * 1.2f );	// multiply by the mass, plus a little extra
	m_apVerletPool[idx]->ApplyImpulseToCOM( &vImpulse );


#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "CVehicleLoader::_InitAndStartVerlet():  Starting Verlet object %d\n", idx );
#endif

}



void CVehicleLoader::_DropHeldObject( void ) {
	FASSERT( (m_pClawedEntity != NULL) && (m_pClawedEntity->TypeBits() & ENTITY_BIT_MESHENTITY) );

	CMeshEntity *pME = (CMeshEntity*)m_pClawedEntity;

	m_pClawedEntity->DetachFromParent();
	m_pClawedEntity = NULL;

	_InitAndStartVerlet( pME );
}

void CVehicleLoader::_ClearEntityVerletObject( CEntity *pME ) {
	for( u32 i=0; i<_VERLET_COUNT; i++ ) {
		if( pME == m_apVerletPoolME[i] ) {
			_VerletDeactivate( i, FALSE );
			return;
		}
	}
}

void CVehicleLoader::_VerletMovedCB( CFVerlet *pVerlet, f32 fDeltaSecs ) {
	if( !pVerlet->m_pUser ) {
		return;
	}
	FASSERT( pVerlet->m_pUser );
	FASSERT( (pVerlet->m_nUser >= 0) && (pVerlet->m_nUser < _VERLET_COUNT) );

	CEntity *pE = ((CEntity*)pVerlet->m_pUser);
	pE->Relocate_RotXlatFromUnitMtx_WS( pVerlet->GetUnitMtx() );
	f32 fK = 0.5f * fDeltaSecs;
	FMATH_CLAMP_UNIT_FLOAT( fK );
	m_avVerletObjectAveSpeed[pVerlet->m_nUser].Mul( 1.0f - fK );
	CFVec3A vOSpd;
	pVerlet->ComputeOriginVelocity_WS( &vOSpd, fmath_Inv( fDeltaSecs ) );
	vOSpd.Mul( fK );
	m_avVerletObjectAveSpeed[pVerlet->m_nUser].Add( vOSpd );

	if( (m_avVerletObjectAveSpeed[pVerlet->m_nUser].MagSq() < 3.0f) ) {
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
		DEVPRINTF( "Verlet %d moving too slow, deactivating\n", pVerlet->m_nUser );
#endif
		_VerletDeactivatedCB( pVerlet );
	} else if( m_avVerletObjectAveSpeed[pVerlet->m_nUser].MagSq() > (105.0f * 105.0f) ) {		// verlet seems to max out at about 105
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
		DEVPRINTF( "Verlet %d moving too fast, deactivating\n", pVerlet->m_nUser );
#endif
		_VerletDeactivatedCB( pVerlet );
	}
}


void CVehicleLoader::_VerletDeactivate( u32 uIdx, BOOL bCheckForAttach/* = TRUE*/ ) {
	FASSERT( uIdx < _VERLET_COUNT );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "CVehicleLoader::_VerletDeactivate():  Verlet object %d deactivated.\n", uIdx );
#endif

	if( bCheckForAttach && m_apVerletPoolME[uIdx] ) {
		m_CBSphere  = m_apVerletPoolME[uIdx]->GetMeshInst()->m_pMesh->BoundSphere_MS;
		m_CBSphere.m_fRadius *= 0.5f;
		m_CBSphere.m_Pos = m_apVerletPoolME[uIdx]->MtxToWorld()->m_vPos.v3;
		m_CBSphere.m_Pos.y -= m_CBSphere.m_fRadius;

		
		CFTrackerCollideSpheresInfo collinfo;
		collinfo.bIgnoreCollisionFlag		= FALSE;
		collinfo.nTrackerTypeBits			= FWORLD_TRACKERTYPEBIT_MESH;
		collinfo.nTrackerUserTypeBitsMask	= ENTITY_BIT_MESHENTITY;
		collinfo.pMasterSphere_WS			= &m_CBSphere;
		collinfo.pCallback					= _VerletStoppedTrackerCB;
		
		m_pCBEntity = m_apVerletPoolME[uIdx];

		FWorld_nTrackerSkipListCount = 0;
		m_apVerletPoolME[uIdx]->AppendTrackerSkipList();;
		collinfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
		collinfo.ppTrackerSkipList = FWorld_apTrackerSkipList;

		fworld_CollideWithTrackers( &collinfo, NULL );
		m_pCBEntity = NULL;
	}

	m_apVerletPool[uIdx]->EnableWork( FALSE );
	m_apVerletPool[uIdx]->SetWorldMesh( NULL );

	m_abVerletPoolActive[uIdx]			= FALSE;
	m_apVerletPool[uIdx]->m_pUser		= NULL;
	m_apVerletPoolME[uIdx]				= NULL;
	m_abVerletMarkedForDeactivate[uIdx]	= FALSE;
}


BOOL CVehicleLoader::_VerletStoppedTrackerCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex ) {
	CFCollInfo collinfo;

	for( u32 i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker ==  FWorld_apTrackerSkipList[0] ) {
			return TRUE;
		}
	}

	collinfo.bCalculateImpactData		= FALSE;
	collinfo.bFindClosestImpactOnly		= TRUE;
	collinfo.nStopOnFirstOfCollMask		= FCOLL_MASK_CHECK_ALL;
	collinfo.bCullBacksideCollisions	= TRUE;
	collinfo.nCollMask					= FCOLL_MASK_COLLIDE_WITH_OBJECTS;
	collinfo.nCollTestType				= FMESH_COLLTESTTYPE_SPHERE;
	collinfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	collinfo.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	collinfo.pSphere_WS					= &m_CBSphere;

	if( ((CFWorldMesh*)pTracker)->CollideWithMeshTris( &collinfo ) ) {
		// we hit an entity, keep track of it in case we want to attach...
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
		DEVPRINTF( "CVehicleLoader::_VerletStoppedTrackerCB, attaching ME to ME:  %x\n", pTracker->m_pUser );
#endif
		m_pCBEntity->Attach_ToParent_WS( ((CEntity*)pTracker->m_pUser) );
		return FALSE;
	}
	return TRUE;
}


void CVehicleLoader::_VerletDeactivatedCB( CFVerlet *pVerlet ) {
    FASSERT( pVerlet != NULL );
	FASSERT( (pVerlet->m_nUser >= 0) && (pVerlet->m_nUser < _VERLET_COUNT) );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	DEVPRINTF( "CVehicleLoader::_VerletDeactivatedCB():  Verlet object %d marked for deactivation.\n", pVerlet->m_nUser );
#endif	

	m_abVerletMarkedForDeactivate[pVerlet->m_nUser] = TRUE;
}


BOOL CVehicleLoader::_VerletTrackerCB( CFVerlet *pVerlet, CFWorldTracker *pTracker ) {
	if( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH ) {
		if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
			if( ((CEntity*)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT ) {
				return FALSE;
			}

			for( u32 i=0; i<_VERLET_COUNT; i++ ) {
				if( ((CEntity*)pTracker->m_pUser) == m_apVerletPoolME[i] ) {
					CFVec3A vVSpd, vTSpd;
					pVerlet->ComputeOriginVelocity_WS( &vVSpd, 1.0f );
					m_apVerletPool[i]->ComputeOriginVelocity_WS( &vTSpd, 1.0f );
					if( vVSpd.MagSq() > vTSpd.MagSq() ) {
						_ClearEntityVerletObject( (CEntity*)pTracker->m_pUser );
					}

					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


void CVehicleLoader::_OrientWeapon( void ) {
	CFQuatA		qNewGun;
	CFQuatA		qRotY;
	CFQuatA		qFinal;
	CFAnimFrame animframe;
	f32			fSlerp;

	f32 fX = m_TargetLockUnitVec_WS.Dot( m_MtxToWorld.m_vRight );
	f32 fZ = m_TargetLockUnitVec_WS.Dot( m_MtxToWorld.m_vFront );
	f32 fA = fmath_Atan( fX, fZ );

	if( FMATH_FABS( fA ) > FMATH_QUARTER_PI ) {
		FMATH_BIPOLAR_CLAMPMAX( fA, FMATH_QUARTER_PI );
		m_bGunReady = FALSE;
	} else {
		m_bGunReady = TRUE;
	}

	qNewGun.BuildQuat( CFVec3A::m_UnitAxisX, m_fMountPitch_WS ); //, m_pWorldMesh->m_pMesh->pBoneArray[m_nBoneIdxGun].AtRestBoneToParentMtx.m_vPos );
	qRotY.BuildQuatRotY( fA );
	
	qNewGun.Mul( qRotY );

	fSlerp = _GUN_SLERP_SPEED * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_UNIT_FLOAT( fSlerp );

	qFinal.ReceiveSlerpOf( fSlerp, m_qLastGun, qNewGun );
	animframe.v			= qFinal.v;
	animframe.m_Pos		= m_pWorldMesh->m_pMesh->pBoneArray[m_nBoneIdxGun].AtRestBoneToParentMtx.m_vPos.v3;
	animframe.m_fScale	= 1.0f;

	m_qLastGun = qFinal;

	m_pAnimManFrame->UpdateFrame( ANIMBONE_GUN, animframe );
}


#define _GUN_REACTION_FORCE		( -2.0f )

void CVehicleLoader::_WeaponWork( void ) {
	if( !m_bGunEnabled ) {
		return;
	}


	_OrientWeapon();

	if( (m_fControls_Fire2 < 0.25f) || (!m_bGunReady) ) {
		if( m_fGunFireTimer > 0.0f ) {
			m_fGunFireTimer -= FLoop_fPreviousLoopSecs;
		}

		if( m_fGunUnitShotSpread > 0.0f ) {
			m_fGunUnitShotSpread -= FLoop_fPreviousLoopSecs * m_BotInfo_Loader.fShotSpreadRecRate;
		}

		if( m_fGunAISoundTimer > 0.0f ) {
			m_fGunAISoundTimer -= FLoop_fPreviousLoopSecs;
		}
		return;
	}

	m_fGunFireTimer		-= FLoop_fPreviousLoopSecs;
	m_fGunAISoundTimer	-= FLoop_fPreviousLoopSecs;

	if( m_fGunFireTimer <= 0.0f ) {
		f32		fSpreadFactor, fRandomScale;
		CFVec3A	vSpread, vMuzzlePoint, vUnitFireDir;
		// fire the gun
		JustFired();
		m_fGunUnitShotSpread += FLoop_fPreviousLoopSecs * m_BotInfo_Loader.fShotSpreadRate;
		FMATH_CLAMP_MAX1( m_fGunUnitShotSpread );

		fSpreadFactor = FMATH_FPOT( m_fGunUnitShotSpread, m_BotInfo_Loader.fMinShotSpread, m_BotInfo_Loader.fMaxShotSpread );
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
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce );
		}

		vUnitFireDir.Add( m_TargetLockUnitVec_WS, vSpread );
		vUnitFireDir.Unitize();

		fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );

		// give the loader a little push back
		CFVec3A vGunForce, vGunForcePos;
		vGunForce.Mul( vUnitFireDir, _GUN_REACTION_FORCE * m_BotInfo_VehiclePhysics.fMass );
		vGunForcePos.Sub( vMuzzlePoint, *m_PhysObj.GetPosition() );
		m_PhysObj.ApplyForce( &vGunForce, &vGunForcePos );

        // Draw all muzzleflash effects...
		CMuzzleFlash::AddFlash_CardPosterZ( CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
											vMuzzlePoint,
											vUnitFireDir,
											m_BotInfo_Loader.fMuzzleFlashZWidth * fRandomScale,
											m_BotInfo_Loader.fMuzzleFlashZHeight * fRandomScale,
											m_BotInfo_Loader.fMuzzleFlashZAlpha );
	
		CMuzzleFlash::AddFlash_CardPosterXY( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
											 vMuzzlePoint,
											 m_BotInfo_Loader.fMuzzleFlashXYScale * fRandomScale,
											 m_BotInfo_Loader.fMuzzleFlashXYAlpha );

		CMuzzleFlash::AddFlash_Mesh3D( CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_3D_STAR_1 + fmath_RandomRange( 0, 1 ) ],
									   vMuzzlePoint, vUnitFireDir,
                                       m_BotInfo_Loader.fMuzzleFlashMeshScale * fRandomScale,
									   fmath_RandomFloat() * FMATH_2PI );	// rotation

		CMuzzleFlash::MuzzleLight( &vMuzzlePoint, m_BotInfo_Loader.fMuzzleLightRadius );

        // handle force feedback...

		// handle sound...

		// see whether we hit anything...
		CFVec3A			vRayEnd;
		FCollImpact_t	impact;

		vRayEnd.Mul( vUnitFireDir, m_BotInfo_Loader.fMaxRange ).Add( vMuzzlePoint );
		
		//SPECIAL CASE:  we can't use the normal skiplist, because it includes the entity that's currently grabbed
		FWorld_nTrackerSkipListCount	= 1;
		FWorld_apTrackerSkipList[0]		= m_pWorldMesh;
	
		if( fworld_FindClosestImpactPointToRayStart( &impact, &vMuzzlePoint, &vRayEnd, 
													 FWorld_nTrackerSkipListCount, 
													 FWorld_apTrackerSkipList, TRUE, NULL, -1,
													 FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
		

			vRayEnd = impact.ImpactPoint;
			CFVec3A vFlashPos;
			const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( &impact );

			// draw impact effects
			pCollMaterial->DrawAllDrawableParticles( &impact.ImpactPoint, &impact.UnitFaceNormal,
													 FALSE,
													 fmath_RandomFloatRange( 0.2f, 1.0f ),
													 fmath_RandomFloatRange( 0.2f, 0.5f ),
													 fmath_RandomFloatRange( 0.1f, 0.3f ) );

			if( pCollMaterial->IsImpactFlashEnabled() ) {
				CFVec3A vImpactFlashPos;
				vImpactFlashPos.Mul( impact.UnitFaceNormal, m_BotInfo_Loader.fMuzzleImpactOffset ).Add( impact.ImpactPoint );
				CMuzzleFlash::AddFlash_CardPosterXY( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
													 vImpactFlashPos,
													 m_BotInfo_Loader.fMuzzleImpactOffset,
													 m_BotInfo_Loader.fMuzzleImpactAlpha );
			}


			CFDecal::Create( m_BotInfo_Loader.hBulletDecal, &impact, &vUnitFireDir );
			
			if( impact.pTag == NULL ) {
				// hit world geo...
				//if( pCollMaterial->IsPockMarkEnabled( &impact ) ) {
				//	//potmark_NewPotmark( &impact.ImpactPoint, &impact.UnitFaceNormal, 
				//	//					m_BotInfo_Loader.fPotmarkScale, POTMARKTYPE_BULLET );
				//}
			} else {
				// hit something interesting...
				CFWorldMesh *pWorldMesh = (CFWorldMesh*)impact.pTag;
                
				FASSERT( pWorldMesh->GetType() == FWORLD_TRACKERTYPE_MESH );
				if( pWorldMesh->m_nUser == MESHTYPES_ENTITY ) {
					CEntity *pDamagee = (CEntity*)pWorldMesh->m_pUser;

					// it's an entity...
					CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
					if( pDamageForm ) {
						pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_IMPACT;
						pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
						pDamageForm->m_pDamageProfile	= m_pBotInfo_Vehicle->pPrimaryWeaponDamageProfile;
						pDamageForm->m_Damager.pWeapon	= NULL;
						pDamageForm->m_Damager.pBot		= this;
						pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
						pDamageForm->m_pDamageeEntity	= pDamagee;
						pDamageForm->InitTriDataFromCollImpact( pWorldMesh, &impact, &vUnitFireDir );

						CDamage::SubmitDamageForm( pDamageForm );

					}

					// Make an AI impact sound in case any AI are listening...
					if( (m_nPossessionPlayerIndex > -1) && (m_fGunAISoundTimer <= 0.0f) ) {
						AIEnviro_AddSound( impact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, this );
						m_fGunAISoundTimer = 0.5f;
					}
				}
			}
		}

 		// play a sound
		if( m_nPossessionPlayerIndex < 0 ) {
			fsndfx_Play3D( m_BotInfo_LoaderSounds.hGun, &vMuzzlePoint, m_BotInfo_Loader.fSoundRadius3D, 1.0f, m_BotInfo_Loader.fSoundVolume3D );
		} else {
			fsndfx_Play2D( m_BotInfo_LoaderSounds.hGun, m_BotInfo_Loader.fSoundVolume2D );
		}

		m_fGunTracerTimer -= FLoop_fPreviousLoopSecs;
		if( m_fGunTracerTimer <= 0.0f ) {
			f32 fTracerLen = vMuzzlePoint.Dist( vRayEnd );
			CMuzzleFlash::AddFlash_CardPosterZ( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1],
												vMuzzlePoint, vUnitFireDir,
												m_BotInfo_Loader.fTracerWidth,
												fTracerLen,
												m_BotInfo_Loader.fTracerAlpha );
			m_fGunTracerTimer += m_BotInfo_Loader.fTracerTime * fmath_RandomFloatRange( 0.9f, 1.1f );
		}

		m_fGunFireTimer += m_BotInfo_Loader.fOOGunRoundsPerSec;
	}
}


void CVehicleLoader::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS ) {
	*pApproxMuzzlePoint_WS = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPriFire]->m_vPos;
}


void CVehicleLoader::_UpdateReticle( void ) {
//	m_pTargetedEntity = NULL;
	if( m_nPossessionPlayerIndex < 0 ) {
		return;
	}

	if( m_pDriverBot == NULL ) {
		return;
	}

	ComputeHumanTargetPoint_WS( &m_TargetedPoint_WS, NULL );
	if ( m_pTargetedMesh )
	{
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
	}
}

void CVehicleLoader::HandleTargeting( void ) {
	//don't call base class, it doesn't work since
	//vehicle loader doesn't have a weapon

	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );

	//do all targeting lock bit checks here.
	if( m_nPossessionPlayerIndex < 0 ) {
		if(	!m_bControls_Human &&
			(m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT) ) {
			
			if( m_bGunReady ) {
				m_TargetedPoint_WS = m_ControlBot_TargetPoint_WS;
				// Announce that we're locked... and ready to fire
				FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
			}
		} else {
			m_TargetedPoint_WS.Mul( m_MtxToWorld.m_vFront, 1000.0f ).Add( m_MtxToWorld.m_vPos );
		}
	} else {
		_UpdateReticle();
	}

	ComputeApproxMuzzlePoint_WS( &m_TargetLockUnitVec_WS );
	m_TargetLockUnitVec_WS.Sub( m_TargetedPoint_WS );
	m_TargetLockUnitVec_WS.Unitize().Negate();
}


void CVehicleLoader::_SoundWork( void ) {
	FMATH_CLAMP_UNIT_FLOAT( m_fEngineLoad );
	f32 fMaxDeltaThisFrame;

	if( m_fClawLockUnitTimer > 0.0f ) {
		m_fEngineLoad = FMATH_MAX( m_fEngineLoad, 0.66f );
	}

	if( m_fEngineLoad > m_fLastEngineLoad + (10.0f * FLoop_fPreviousLoopSecs) ) {
		if( m_fUnitEngineVolume < m_fEngineLoad ) {
			m_fUnitEngineVolume += _USERPROPS_MAX_ENGINE_SOUND_DELTA_UP * FLoop_fPreviousLoopSecs;
		}

		fMaxDeltaThisFrame = _USERPROPS_MAX_ENGINE_SOUND_DELTA_UP * FLoop_fPreviousLoopSecs;
		if( m_fEngineLoad - m_fLastEngineLoad > fMaxDeltaThisFrame ) {
			m_fEngineLoad = m_fLastEngineLoad + fMaxDeltaThisFrame;
		}
	} else {
		m_fUnitEngineVolume -= 0.05f * FLoop_fPreviousLoopSecs;

		fMaxDeltaThisFrame = _USERPROPS_MAX_ENGINE_SOUND_DELTA_DOWN * FLoop_fPreviousLoopSecs;
		if( m_fLastEngineLoad - m_fEngineLoad > fMaxDeltaThisFrame ) {
			m_fEngineLoad = m_fLastEngineLoad - fMaxDeltaThisFrame;
		}
	}

	FMATH_CLAMP_UNIT_FLOAT( m_fEngineLoad );
	FMATH_CLAMP( m_fUnitEngineVolume, 0.3f, 0.7f );


	f32 fHighVol = m_BotInfo_Loader.fSoundVolume3D  * fmath_Sqrt( m_fEngineLoad ); //fmath_Div( m_fEngineLoad - .33f, 0.66f );
	f32 fLowFreq = FMATH_FPOT( m_fEngineLoad, 0.8f, 1.3f );

	FMATH_CLAMP_UNIT_FLOAT( fHighVol );

	if( m_pAudioEmitterEngHigh ) {
		m_pAudioEmitterEngHigh->SetVolume( fHighVol * m_fUnitEngineVolume );
	}
	if( m_pAudioEmitterEngLow ) {
		m_pAudioEmitterEngLow->SetFrequencyFactor( fLowFreq );
		m_pAudioEmitterEngLow->SetVolume( m_fUnitEngineVolume );
	}

	m_fLastEngineLoad = m_fEngineLoad;
}


void CVehicleLoader::ClassHierarchyRelocated( void *pIdentifier ) {
	if( pIdentifier != m_pMoveIdentifier ) {
		_InitPhysicsObject();
		m_PhysObj.SetPosition( &m_MtxToWorld.m_vPos );
		m_PhysObj.SetOrientation( &m_MtxToWorld );
		m_fEngineDisruptTimer = 0.0f;
		f32 fYaw_WS = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
		ChangeMountYaw( fYaw_WS );
	}
}


void CVehicleLoader::InflictDamageResult( const CDamageResult *pDamageResult ) {
	CVehicle::InflictDamageResult( pDamageResult );

	BOOL bPlayerIsDriving = FALSE;
	f32 fImpulseMultiplier = 0.0f;

	bPlayerIsDriving = m_pDriverBot && (m_pDriverBot->m_nPossessionPlayerIndex >= 0);

	switch( pDamageResult->m_pDamageData->m_nDamageLocale ) {
	case CDamageForm::DAMAGE_LOCALE_IMPACT:
		if( bPlayerIsDriving || MultiplayerMgr.IsMultiplayer() ) {
			fImpulseMultiplier = 0.5f;
		} else {
			fImpulseMultiplier = 1.0f;
		}

		break;

	case CDamageForm::DAMAGE_LOCALE_BLAST:
		if( bPlayerIsDriving && MultiplayerMgr.IsSinglePlayer() ) {
			fImpulseMultiplier = 1.5f;
		} else {
			fImpulseMultiplier = 3.0f;
		}

		break;
	};

	if( fImpulseMultiplier > 0.0f ) {
		CFVec3A vMomentum, vPoint;

		vPoint.Set( pDamageResult->m_pDamageData->m_ImpactPoint_WS );
		vPoint.Sub( *m_PhysObj.GetPosition() );
		f32 fMag2 = vPoint.MagSq();
		if( fMag2 > 0.01f ) {
			vPoint.Mul( fmath_InvSqrt( fMag2 ) * 10.0f );
		}

		vMomentum.Set( 0.0f, pDamageResult->m_fImpulseMag * fImpulseMultiplier, 0.0f );

		m_PhysObj.ApplyMomentum( &vMomentum, &vPoint );

#if 0
		if( pDamageResult->m_fImpulseMag > _IMPACT_ENGINE_CUTOFF_THRESHOLD ) {
			m_fEngineDisruptTimer = 2.0f;
		}
#endif
	}
}


BOOL CVehicleLoader::IsUpsideDown( void ) const {
	return (m_fFlipVehicleTimer == 0.0f) && (m_MtxToWorld.m_vUp.y < 0.5f);
}


void CVehicleLoader::FlipRightsideUp( CEntity *pEntity ) {
	m_fFlipVehicleTimer = 1.0f;
	if( pEntity == NULL ) {
		m_vFlipPosition = *m_PhysObj.GetPosition();
	} else {
		m_vFlipPosition = pEntity->MtxToWorld()->m_vPos;
	}
}


void CVehicleLoader::_AnimBoneCallback( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCBLoader && (m_pCBLoader->TypeBits() & ENTITY_BIT_VEHICLELOADER) );
	if( !m_pCBLoader ||
		!(m_pCBLoader->TypeBits() & ENTITY_BIT_VEHICLELOADER) ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	// the only valid reason for calling this fn is to place the gun outside the world
	FASSERT( uBoneIdx == m_pCBLoader->m_nBoneIdxGun );
	rNewMtx = rBoneMtx;
	rNewMtx.m_vPos.Set( 2000000.0f, 2000000.0f, 2000000.0f ); 	
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleLoaderBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


void CVehicleLoaderBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) 
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CVehicleBuilder, ENTITY_BIT_VEHICLELOADER, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount	= CVehicleLoader::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile		= CVehicleLoader::m_BotInfo_Gen.pszArmorProfile;
	m_pszEC_DamageProfile		= CVehicleLoader::m_BotInfo_Vehicle.pszDamageProfile;
	m_bEnableGun				= TRUE;
}


BOOL CVehicleLoaderBuilder::PostInterpretFixup( void ) 
{
	if( !CVehicleBuilder::PostInterpretFixup() ) 
	{
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	return FALSE;
}


BOOL CVehicleLoaderBuilder::InterpretTable( void ) 
{
	cchar *pszString;

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "Gun" ) ) {
		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "On" ) ) {
				m_bEnableGun = TRUE;
			} else if( !fclib_stricmp( pszString, "Off" ) ) {
				m_bEnableGun = FALSE;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}
		return TRUE;
	}

	return CVehicleBuilder::InterpretTable();
}

