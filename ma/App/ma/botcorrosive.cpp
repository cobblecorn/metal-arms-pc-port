//////////////////////////////////////////////////////////////////////////////////////
// botcorrosive.cpp - General Corrosive
//
// Author: Russ Foushee     
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
// 03/13/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botcorrosive.h"
#include "botanim.h"
#include "fmesh.h"
#include "fresload.h"
#include "meshtypes.h"
#include "protrack.h"
#include "meshentity.h"
#include "reticle.h"
#include "player.h"
#include "muzzleflash.h"
#include "weapon.h"
#include "potmark.h"
#include "Ai\AIEnviro.h"
//#include "fpad.h"
#include "botpart.h"
#include "gamecam.h"
#include "LightPool.h"
#include "MultiplayerMgr.h"
#include "AlarmSys.h"
#include "ESphere.h"
#include "gstring.h"

#include "sas_user.h"

#define	_BOTINFO_FILENAME				"b_corrosive"
#define _BOTPART_FILENAME				"bp_corr"
#define _SOUND_BANK_NAME				"Corrosive"
#define _VSPOT_TABLE_NAME				"Vspots"
#define _NUM_FIELDS_PER_VSPOT			6
#define _DAMAGE_MILESTONES_TABLE_NAME	"DamageMilestones"

static cchar* _apszCorrosiveMeshFilenames[] = { "grmzcorro00", "grmzcorrB00" };   

#define _MESH_NAME			( "grmzcorro00" )

#define _RETICLE_Y			( 0.2f )

#define _NEAR_STATIONARY_VELOCITY_SQUARED		0.25f

#define _BEHAVIOR_BLEND_INOUT_TIME				0.25f
#define _OO_BEHAVIOR_BLEND_INOUT_TIME			1.0f / _BEHAVIOR_BLEND_INOUT_TIME
#define _REORIENT_LEGS_TIME						0.5f
#define _ZERO_VELOCITY_TIME						0.5f 
#define _OO_ZERO_VELOCITY_TIME					1.0f / _ZERO_VELOCITY_TIME

#define _FOOT_COLLISION_TOE_PROJECTION_DIST		7.0f
#define _FOOT_COLLISION_HEEL_PROJECTION_DIST	-3.3f
#define _FOOT_COLLISION_CAPSULE_RADIUS			4.0f

#define _FIST_SMASH_CRASH_SOUND_TIME			1.0f
#define _FOOT_STOMP_CRASH_EFFECT_TIME			1.4f
#define _FORWARD_FOOT_STOMP_CRASH_EFFECT_TIME	0.75f
#define _NON_VSPOT_DAMAGE_MULTIPLIER			0.05f

#define _BOT_TOSS_ALARM_ON_TIME					0.8f
#define _BOT_TOSS_RELEASE_TIME					2.35f
#define _BOT_TOSS_VELOCITY_IMPULSE_MAGNITUDE	80.0f
#define _BOT_TOSS_PARTICLE_EMIT_START_TIME		0.25f
#define _BOT_TOSS_PARTICLE_EMIT_STOP_TIME		2.75f

#define _DEATH_MAX_DESTRUCTION_TIME				15.0f
#define _DEATH_EXPLOSION_TABLE_NAME				"DeathExplosions"
#define _DEATH_EXPLOSION_MIN_TIME				1.0f
#define _DEATH_EXPLOSION_MAX_TIME				7.5f

#define _FINGER_FLICK_LAUGH_TIME				3.5f

// DEBUG DEFINES
#define _DRAW_FOOTCOLLISION_DEBUG_INFO	0
#define _DRAW_VSPOT_SPHERES				0

static CBotCorrosiveBuilder _BotCorrosiveBuilder;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotCorrosive
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


BOOL									CBotCorrosive::m_bSystemInitialized = FALSE;
u32										CBotCorrosive::m_nBotClassClientCount = 0;

CBotCorrosive::BotInfo_Gen_t			CBotCorrosive::m_BotInfo_Gen;
CBotCorrosive::BotInfo_MountAim_t		CBotCorrosive::m_BotInfo_MountAim;
CBotCorrosive::BotInfo_Walk_t			CBotCorrosive::m_BotInfo_Walk;
CBotCorrosive::BotInfo_Jump_t			CBotCorrosive::m_BotInfo_Jump;
CBotCorrosive::BotInfo_Weapon_t			CBotCorrosive::m_BotInfo_Weapon;
CBotCorrosive::BotInfo_Corrosive_t		CBotCorrosive::m_BotInfo_Corrosive;

CBotPartPool*							CBotCorrosive::m_pPartPool = NULL;
CBotCorrosive*							CBotCorrosive::m_pCBCorrosive = NULL;
CFVec3A									CBotCorrosive::m_GroinVecY_WS;
CFCollInfo								CBotCorrosive::m_CollInfo;

SmokeTrailAttrib_t						CBotCorrosive::m_SmokeTrailAttrib;
CBotAnimStackDef						CBotCorrosive::m_AnimStackDef;
CFTexInst								CBotCorrosive::m_CoronaTexInst;
FParticle_DefHandle_t					CBotCorrosive::m_hBotTossFXParticle;
FParticle_DefHandle_t					CBotCorrosive::m_hVSpotHitFXParticle;

DamageInflictedCallback_t CBotCorrosive::m_pDamageCallback=NULL;

BOOL CBotCorrosive::m_bForce2D_RoarRocket=FALSE;

//////////////////////
//STATIC FNs

BOOL CBotCorrosive::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = NULL;

	m_SmokeTrailAttrib.fScaleMin_WS = 0.5f;
	m_SmokeTrailAttrib.fScaleMax_WS = 0.6f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.4f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.7f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_SmokeTrailAttrib.EndColorRGB.White();
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.2f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.6f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	m_pDamageCallback = NULL;

	m_hBotTossFXParticle = FPARTICLE_INVALID_HANDLE;
	m_hVSpotHitFXParticle = FPARTICLE_INVALID_HANDLE;

	return TRUE;
}


void CBotCorrosive::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	FASSERT( m_nBotClassClientCount == 0 );

	m_bSystemInitialized = FALSE;
}


CBotCorrosive::CBotCorrosive() : CBot() {
	m_pInventory	= NULL;
	m_pWorldMesh	= NULL;

	m_fSwatDamageRadiusMul = 1.0f;
}


CBotCorrosive::~CBotCorrosive() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotCorrosive::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotCorrosiveBuilder *pBuilder = (CBotCorrosiveBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotCorrosiveBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTCORROSIVE );
	}

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotCorrosive::_ClearDataMembers( void ) {
	m_fCollCylinderHeight_WS	= 60.0f;
	m_fCollCylinderRadius_WS	= 20.0f;

	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_MountAim	= &m_BotInfo_MountAim;
	m_pBotInfo_Walk		= &m_BotInfo_Walk;
	m_pBotInfo_Jump		= &m_BotInfo_Jump;
	m_pBotInfo_Weapon	= &m_BotInfo_Weapon;

	m_fGravity					= m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS	= m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;
	m_fMountPitchMax_WS			= m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS			= m_pBotInfo_MountAim->fMountPitchUpLimit;
	m_pMoveIdentifier			= &m_nBoneIdxGroin;

	m_anAnimStackIndex[ASI_STAND]				= ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT]			= -1;
	m_anAnimStackIndex[ASI_WALK]				= ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT]			= -1;
	m_anAnimStackIndex[ASI_RUN]					= -1;
	m_anAnimStackIndex[ASI_RUN_PANIC]			= -1;
	m_anAnimStackIndex[ASI_FALL]				= ANIMTAP_JUMP_FLY;
	m_anAnimStackIndex[ASI_RC_TETHERED]			= -1; //ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]		= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]			= ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]				= -1; //ANIMTAP_STOOP_SUMMER;
	m_anAnimStackIndex[ASI_STAND_LIMP_LEFT]		= ANIMTAP_STAND_LIMP_LEFT;
	m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT]	= -1; //ANIMTAP_STAND_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_LIMP_LEFT]			= ANIMTAP_LIMP_LEFT;
	m_anAnimStackIndex[ASI_LIMP_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_HOP_LEFT]			= -1;
	m_anAnimStackIndex[ASI_HOP_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_STARTLE]				= -1;
	m_anAnimStackIndex[ASI_ROLL_LEFT]			= -1;
	m_anAnimStackIndex[ASI_ROLL_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_DOZE_LOOP]			= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_NAPJERK]				= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_WAKE]				= -1; //ANIMTAP_WAKE;

	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock	= m_anEnableBoneNameIndexTableForSummer_Normal;

	fforce_NullHandle( &m_hForce );

	m_nCorrosiveFlags = CORROSIVEFLAG_NONE;
	m_eRequestedBehaviorState = BEHAVIOR_STATE_NONE;
	m_eLastBehaviorState = BEHAVIOR_STATE_NONE;
	m_eBehaviorState = BEHAVIOR_STATE_NONE;

	m_pTargetedEntity = NULL;

	m_nHitEntityCount = 0;
	m_nLeftFootDamagedEntityCount = 0;
	m_nRightFootDamagedEntityCount = 0;

	m_paLimbTrackers = NULL;

	m_fSavedNormHealth = 1.0f;
	m_nVSpotCount = 0;
	m_paVSpots = NULL;
	m_pafDamageMilestones = NULL;
	m_nDamageMilestones = 0;

	m_pBotTossAlarmNet = NULL;		
	m_pBotTossAlarmSpawnSpline = NULL;
	m_hBotTossFXEmitter = FPARTICLE_INVALID_HANDLE;

	m_paDeathExplosions = NULL;
	m_nDeathExplosions = 0;
}


/////////////////////
//CLASS HIERARCHY FNs

BOOL CBotCorrosive::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	m_nBotClassClientCount++;

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
	FResFrame_t frame = fres_GetFrame();
    
	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_NAME ) ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_NAME );
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyLoadSharedResources():  Error parsing %s\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	if( !_BuildAnimStackDef() ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyLoadSharedResources():  Error creating bot anim stack def\n" );
		goto _ExitWithError;
	}

	// Load texture for rocket smoke
	m_SmokeTrailAttrib.pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_BotInfo_Corrosive.pszRocketSmokeTex );
	if( m_SmokeTrailAttrib.pTexDef == NULL ) {
		DEVPRINTF( "CBotTitan::ClassHierarchyLoadSharedResources(): Could not load smoke trail texture.\n" );
		return FALSE;
	}

	// Load corona texture...
	m_CoronaTexInst.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_BotInfo_Corrosive.pszCoronaTexName ) );


	// Lets load the bot toss particle data here...
	m_hBotTossFXParticle = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Corrosive.pszBotTossParticleFXResName );
	FMATH_CLAMP_UNIT_FLOAT( m_BotInfo_Corrosive.fBotTossFXIntensity );

	// Lets load the VSpot Hit particle data here...
	m_hVSpotHitFXParticle = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Corrosive.pszVSpotHitParticleFXResName );

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( frame );

	return FALSE;
}


void CBotCorrosive::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	m_nBotClassClientCount--;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}

	m_AnimStackDef.Destroy();

	m_SmokeTrailAttrib.pTexDef = NULL;

	m_hBotTossFXParticle = FPARTICLE_INVALID_HANDLE;
	m_hVSpotHitFXParticle = FPARTICLE_INVALID_HANDLE;

	CBot::ClassHierarchyUnloadSharedResources();
}


BOOL CBotCorrosive::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	FMesh_t *pMesh;
	FMeshInit_t meshinit;
	FResFrame_t frame;
	cchar * pszMeshFilename = NULL;

	s32 nBoneIdx;

	frame = fres_GetFrame();

	// Get pointer to leaf class's builder object...
	CBotCorrosiveBuilder *pBuilder = (CBotCorrosiveBuilder*)GetLeafClassBuilder();

	// Get input params for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent...
	if( !CBot::ClassHierarchyBuild() ) {
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	//
	// Init from builder...
	//

	// Check to see if we have both an alarm and spawn point name
	if( pBuilder->m_pszBotTossAlarmName && pBuilder->m_pszBotTossSpawnPtName ) {
		m_pBotTossAlarmNet = ( CAlarmNet* ) gstring_Main.AddString( pBuilder->m_pszBotTossAlarmName );
		m_pBotTossAlarmSpawnSpline = ( CEntity* ) gstring_Main.AddString( pBuilder->m_pszBotTossSpawnPtName );
	}

	// Load mesh resource...
	pszMeshFilename = _apszCorrosiveMeshFilenames[0];
	if ( pBuilder->m_uMeshVersionOverride > 0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszCorrosiveMeshFilenames)/sizeof(char*) ) {
		pszMeshFilename = _apszCorrosiveMeshFilenames[pBuilder->m_uMeshVersionOverride];
		// Set the damaged version override right now...
		m_nCorrosiveFlags |= CORROSIVEFLAG_DAMAGEDVERSION;
	}

	// Load mesh...
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create worldmesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to allocate CFWorldMesh\n" );
		goto _ExitWithError;
	}

	// Init worldmesh...
	meshinit.pMesh = pMesh;
	meshinit.fCullDist = FMATH_MAX_FLOAT;
	meshinit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	meshinit.nFlags = 0;
	m_pWorldMesh->Init( &meshinit );

	m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser = this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();


	// Override the walk animation if we are the damaged version of corrosive.
	if( IsDamagedVersion() ) {
		m_anAnimStackIndex[ASI_WALK] = ANIMTAP_DAMAGED_WALK;
	}

	// Build animation stack...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to create animation stack\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	// Set up bone callbacks...
	m_nBoneIdxGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_GROIN ] );
	if( m_nBoneIdxGroin < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_GROIN] );
		goto _ExitWithError;
	}

	m_nBoneIdxTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_TORSO ] );
	if( m_nBoneIdxTorso < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_TORSO] );
		goto _ExitWithError;
	}

	m_nBoneIdxHead = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( m_nBoneIdxHead < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_HEAD] );
		goto _ExitWithError;
	}

	m_nBoneIdxPrimaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_PRIMARY_FIRE ] ); 
	if( m_nBoneIdxPrimaryFire < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_PRIMARY_FIRE] );
		goto _ExitWithError;
	}

	m_nBoneIdxFistDamageSphere = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_HAND_DUMMY ] ); 
	if( m_nBoneIdxFistDamageSphere < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_L_HAND_DUMMY ] );
		goto _ExitWithError;
	}

	m_nBoneIdxSwatDamageSphere = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_MIDDLEFINGERB ] ); 
	if( m_nBoneIdxSwatDamageSphere < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_L_MIDDLEFINGERB ] );
		goto _ExitWithError;
	}

	// Get the foot bones, for damage checks...
	m_nBoneIdxLFoot = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_FOOT ] ); 
	if( m_nBoneIdxLFoot < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_L_FOOT ] );
		goto _ExitWithError;
	}
	m_nBoneIdxLToe = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_TOE ] ); 
	if( m_nBoneIdxLToe < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_L_TOE ] );
		goto _ExitWithError;
	}
	m_nBoneIdxLHeel = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_HEEL ] ); 
	if( m_nBoneIdxLHeel < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_L_HEEL ] );
		goto _ExitWithError;
	}
	m_nBoneIdxRFoot = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_FOOT ] ); 
	if( m_nBoneIdxRFoot < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_R_FOOT ] );
		goto _ExitWithError;
	}
	m_nBoneIdxRToe = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_TOE ] ); 
	if( m_nBoneIdxRToe < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_R_TOE ] );
		goto _ExitWithError;
	}
	m_nBoneIdxRHeel = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_HEEL ] ); 
	if( m_nBoneIdxRHeel < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[ BONE_R_HEEL ] );
		goto _ExitWithError;
	}



	// Set up bone callbacks
	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();
	m_Anim.m_pAnimCombiner->SetBoneCallback( _AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) )
	{
		DEVPRINTF( "CBotEliteGuard::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAUNCH );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_FLY );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAND_UPPER );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAND_LOWER );

	SetMaxHealth();

	m_nBotFlags |= BOTDEATHFLAG_PERSISTAFTERDEATH | BOTDEATHFLAG_PLAYDEATHANIM;

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, _RETICLE_Y );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetType( CReticle::TYPE_DROID_STANDARD );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.EnableDraw( TRUE );
	}

	// Initialize the bot part manager if this is the damaged version of corrosive...
	if( IsDamagedVersion() ) {
		if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
			goto _ExitWithError;
		}

		// Make invincible limbs...
		//MakeLimbInvincible( LIMB_CODE_HEAD, LIMB_TYPE_HEAD, pBuilder );
		MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_LEFT_ARM_UPPER, pBuilder );
		MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_LEFT_ARM_LOWER, pBuilder );
		MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_RIGHT_ARM_UPPER, pBuilder );
		MakeLimbInvincible( LIMB_CODE_LEGS, LIMB_TYPE_RIGHT_LEG_UPPER, pBuilder );
		MakeLimbInvincible( LIMB_CODE_LEGS, LIMB_TYPE_RIGHT_LEG_LOWER, pBuilder );
	}

	// Load the VSpots and DamageMilestones if this is the damaged version
	if( IsDamagedVersion() ) {
		if( !_LoadVSpots() ) {
			goto _ExitWithError;
		}
		if( !_LoadDamageMilestones() ) {
			goto _ExitWithError;
		}
		if( !_LoadDeathExplosions() ) {
			goto _ExitWithError;
		}
	}

	// Find approx eye point...
	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIdx < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vPos;
	}

	// Find gaze direction...
	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIdx < 0 ) {
		DEVPRINTF( "CBotCorrosive::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	// Initialize mtx palette...
	AtRestMatrixPalette();

	// Create tag points
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	// Set up the limb tracker structures...
	if( !_BuildLimbTrackerArrays() ) {
		goto _ExitWithError;
	}

	// Deny the ability for this bot to use the JumpPads
	// Also set the simple sphere test flag, so we can do more complicated damage checks...
	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


BOOL CBotCorrosive::_BuildLimbTrackerArrays( void ) {

	FResFrame_t frame;

	// This function will build the limb tracker arrays
	// This only happens if the 
	if( !IsDamagedVersion() ) {
		// We don't use the bot part system for the undamaged version of corrosive
		return TRUE;
	}

	frame = fres_GetFrame();

	//ARG - GOTO problem
	u32 nBoneDefCount;
	const CBotPartPool::CBoneDef *paBoneDef;

	// Allocate the memory for the tracker structures
	m_paLimbTrackers = ( LimbTracker_t* ) fres_AllocAndZero( LIMB_TYPE_COUNT * sizeof( LimbTracker_t ) );
	if( !m_paLimbTrackers ) {
		DEVPRINTF( "CBotCorrosive::_BuildLimbTrackerArrays : Could not not allocate memory to store the tracker arrays!\n" );
		goto _ExitWithError;
	}
	
	// Now, we want to get the BoneToPartArray from the BotPartSystem and build our tracker lists...
	nBoneDefCount = 0;
	u32 nBoneDefIdx; 
	paBoneDef = m_pPartMgr->GetBotPartPool()->GetBoneInfoArray( &nBoneDefCount );

	// Loop through the bone index list, and tally the number of bones per limb
	for ( nBoneDefIdx = 0; nBoneDefIdx < nBoneDefCount; nBoneDefIdx++ )	{
		FASSERT( paBoneDef[nBoneDefIdx].m_nBoneIndex < m_pWorldMesh->m_pMesh->nBoneCount );

		u8 nBoneIdx = paBoneDef[nBoneDefIdx].m_nBoneIndex;
		s32 nLimbType = m_pPartMgr->GetLimbTypeFromBoneIndex( nBoneIdx );
		FASSERT ( nLimbType < LIMB_TYPE_COUNT );

		if( nLimbType != -1 ) {
			m_paLimbTrackers[ nLimbType ].nLimbElements++;
		}
	}

	// Now, the total amount of bones has been computed for each limb... create the array for each bone...
	u32 nLimbIdx;
	for ( nLimbIdx = 0; nLimbIdx < LIMB_TYPE_COUNT; nLimbIdx++ ) {
		m_paLimbTrackers[ nLimbIdx ].paElement = ( LimbElement_t* ) fres_AlignedAllocAndZero( m_paLimbTrackers[ nLimbIdx ].nLimbElements * sizeof( LimbElement_t ), 16 );
		if( !m_paLimbTrackers[ nLimbIdx ].paElement ) {
			DEVPRINTF( "CBotCorrosive::_BuildLimbTrackerArrays : Could not not allocate memory to store the tracker element arrays!\n" );
			goto _ExitWithError;
		}

		// Now, loop through the bone index list (again), and fill in the tracker array this time...
		u32 nElementIdxCounter = 0;
		for ( nBoneDefIdx = 0; nBoneDefIdx < nBoneDefCount; nBoneDefIdx++ )	{
			FASSERT( paBoneDef[nBoneDefIdx].m_nBoneIndex < m_pWorldMesh->m_pMesh->nBoneCount );

			u8 nBoneIdx = paBoneDef[nBoneDefIdx].m_nBoneIndex;
			s32 nLimbType = m_pPartMgr->GetLimbTypeFromBoneIndex( nBoneIdx );
			FASSERT ( nLimbType < LIMB_TYPE_COUNT );

			if( nLimbType == nLimbIdx ) {
				m_paLimbTrackers[ nLimbIdx ].paElement[ nElementIdxCounter ].nBoneIndex = nBoneIdx;
				nElementIdxCounter++;
			}
		}
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( frame );
	return FALSE;
}


BOOL CBotCorrosive::_LoadVSpots( void ) {


	// Load the bot CSV file and find the VSpot table name...
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	
	// grab an fres and fmem frame
	hMemFrame = fmem_GetFrame();

	//ARG - GOTO problem
	u32 nNumEntriesInTable;
	u32 nBoneDefCount;
	const CBotPartPool::CBoneDef *paBoneDef;

	//////////////////////////////////////////////////
	// load the csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _BOTINFO_FILENAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBotCorrosive::_LoadVSpots() : Could not load the csv file '%s'.\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	// Load the vspots table
	hTable = fgamedata_GetFirstTableHandle( hFile, _VSPOT_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBotCorrosive::_LoadVSpots() : Could not find the group table named '%s'.\n", _VSPOT_TABLE_NAME );
		goto _ExitWithError;
	}
	// Get the number of fields in the table
	nNumEntriesInTable = fgamedata_GetNumFields( hTable );
	if( nNumEntriesInTable % 6 ) {
		DEVPRINTF( "CBotCorrosive::_LoadVSpots() : This table expects three entries per group!\n", _VSPOT_TABLE_NAME );
		goto _ExitWithError;
	}

	m_nVSpotCount = nNumEntriesInTable / _NUM_FIELDS_PER_VSPOT;

	if( m_nVSpotCount == 0 ) {
		// No VSpots -- Bail out
		return TRUE;
	}

	// Allocate the memory for the vspot arrays here...
	m_paVSpots = ( VSpot_t* )fres_AllocAndZero( m_nVSpotCount * sizeof( VSpot_t ) );
	if( !m_paVSpots ) {
		DEVPRINTF( "CBotCorrosive::_LoadVSpots() : Could not allocate Vspot array!\n" );
		goto _ExitWithError;
	}

	// Now, we want to get the BoneToPartArray from the BotPartSystem and build our tracker lists...
	nBoneDefCount = 0;
	paBoneDef = m_pPartMgr->GetBotPartPool()->GetBoneInfoArray( &nBoneDefCount );


	u32 i;
	for( i = 0; i < m_nVSpotCount; i++ ) {

		// First, read in the Vspot Bone name...
		cchar* pszVSpotBoneName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * _NUM_FIELDS_PER_VSPOT + 0, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CBotCorrosive::_LoadVSpots() : Field %d in table %s is not a STRING format!\n", i * _NUM_FIELDS_PER_VSPOT + 0, _VSPOT_TABLE_NAME );
			goto _ExitWithError;
		}

		// Find the bone from the world mesh
		m_paVSpots[ i ].nBoneIndex = m_pWorldMesh->FindBone( pszVSpotBoneName );
		if( m_paVSpots[ i ].nBoneIndex < 0 ) {
			DEVPRINTF( "CBotCorrosive::_LoadVSpots() : Unable to locate VSPOT bone %s\n", pszVSpotBoneName );
			goto _ExitWithError;
		}

		// Now load the radius.
		m_paVSpots[ i ].fRadius = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_FIELDS_PER_VSPOT + 1, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CBotCorrosive::_LoadVSpots() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_FIELDS_PER_VSPOT + 1, _VSPOT_TABLE_NAME );
			goto _ExitWithError;
		}

		// Save off the limb this VSpot is attached to, if any... ( -1 means not attached to a limb )
		m_paVSpots[ i ].nLimbTypeID = m_pPartMgr->GetLimbTypeFromBoneIndex( m_paVSpots[ i ].nBoneIndex );

		// Load whether or not this VSpot is a critical damage vspot or not
		m_paVSpots[ i ].bCriticalDamageOnlySpot = (u32) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_FIELDS_PER_VSPOT + 2, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CBotCorrosive::_LoadVSpots() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_FIELDS_PER_VSPOT + 2, _VSPOT_TABLE_NAME );
			goto _ExitWithError;
		}

		// Load whether or not this VSpot accepts splash damage
		m_paVSpots[ i ].bAcceptSplashDamage = (u32) *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_FIELDS_PER_VSPOT + 3, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CBotCorrosive::_LoadVSpots() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_FIELDS_PER_VSPOT + 3, _VSPOT_TABLE_NAME );
			goto _ExitWithError;
		}

		// Load the VSpot hit emit time
		m_paVSpots[ i ].fMaxEmitTime = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_FIELDS_PER_VSPOT + 4, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CBotCorrosive::_LoadVSpots() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_FIELDS_PER_VSPOT + 4, _VSPOT_TABLE_NAME );
			goto _ExitWithError;
		}

		// Load the VSpot hit emit intenisty
		m_paVSpots[ i ].fEmitIntensity = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * _NUM_FIELDS_PER_VSPOT + 5, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CBotCorrosive::_LoadVSpots() : Field %d in table %s is not a FLOAT format!\n", i * _NUM_FIELDS_PER_VSPOT + 5, _VSPOT_TABLE_NAME );
			goto _ExitWithError;
		}
	}

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( hMemFrame );
	return TRUE;

_ExitWithError:

	// Failure
	fmem_ReleaseFrame( hMemFrame );
	return FALSE;

}


BOOL CBotCorrosive::_LoadDamageMilestones( void ) {
	// Load the bot CSV file and find the DamageMilestones table name...
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	
	// grab an fres and fmem frame
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _BOTINFO_FILENAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBotCorrosive::_LoadDamageMilestones() : Could not load the csv file '%s'.\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	// Load the damamge milestones table
	hTable = fgamedata_GetFirstTableHandle( hFile, _DAMAGE_MILESTONES_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBotCorrosive::_LoadDamageMilestones() : Could not find the group table named '%s'.\n", _DAMAGE_MILESTONES_TABLE_NAME );
		goto _ExitWithError;
	}
	// Get the number of fields in the table
	m_nDamageMilestones = fgamedata_GetNumFields( hTable );

	if( m_nDamageMilestones == 0 ) {
		DEVPRINTF( "CBotCorrosive::_LoadDamageMilestones() : Need to define at least ONE damage milestone (for critical damage!).\n" );
		goto _ExitWithError;
	}

	// Allocate the memory for the DamageMilestone array here...
	m_pafDamageMilestones = ( f32* )fres_AllocAndZero( m_nDamageMilestones * sizeof( f32 ) );
	if( !m_pafDamageMilestones ) {
		DEVPRINTF( "CBotCorrosive::_LoadDamageMilestones : Could not allocate DamageMilestone array!\n" );
		goto _ExitWithError;
	}

	// Now load each damage milestone value
	u32 i;
	for( i = 0; i < m_nDamageMilestones; i++ ) {

		m_pafDamageMilestones[ i ] = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CBotCorrosive::_LoadDamageMilestones() : Field %d in table %s is not a FLOAT format!\n", i, _DAMAGE_MILESTONES_TABLE_NAME );
			goto _ExitWithError;
		}
	}

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( hMemFrame );
	return TRUE;

_ExitWithError:

	// Failure
	fmem_ReleaseFrame( hMemFrame );
	return FALSE;

}


BOOL CBotCorrosive::_LoadDeathExplosions( void ) {

	// Load the bot CSV file and find the VSpot table name...
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	
	// grab an fres and fmem frame
	hMemFrame = fmem_GetFrame();

	//ARG - GOTO problem
	u32 nNumEntriesInTable;

	//////////////////////////////////////////////////
	// load the csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _BOTINFO_FILENAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBotCorrosive::_LoadDeathExplosions() : Could not load the csv file '%s'.\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	// Load the vspots table
	hTable = fgamedata_GetFirstTableHandle( hFile, _DEATH_EXPLOSION_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBotCorrosive::_LoadDeathExplosions() : Could not find the group table named '%s'  There will be no damage explosions.\n", _DEATH_EXPLOSION_TABLE_NAME );
		fmem_ReleaseFrame(hMemFrame);
		return TRUE;
	}
	// Get the number of fields in the table
	nNumEntriesInTable = fgamedata_GetNumFields( hTable );
	if( nNumEntriesInTable % 2 ) {
		DEVPRINTF( "CBotCorrosive::_LoadDeathExplosions() : This table expects two entries per group!\n", _DEATH_EXPLOSION_TABLE_NAME );
		goto _ExitWithError;
	}

	m_nDeathExplosions = nNumEntriesInTable >> 1;

	// Allocate the memory for the DamageExplosion array here...
	m_paDeathExplosions = ( DeathExplosion_t* )fres_AllocAndZero( m_nDeathExplosions * sizeof( DeathExplosion_t ) );
	if( !m_paDeathExplosions ) {
		DEVPRINTF( "CBotCorrosive::_LoadDeathExplosions() : Could not allocate Damage Explosion array!\n" );
		goto _ExitWithError;
	}

	u32 i;
	for( i = 0; i < m_nDeathExplosions; i++ ) {

		// First, read in the Vspot Bone name...
		cchar* pszExplosionBoneName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * 2 + 0, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CBotCorrosive::_LoadDeathExplosions() : Field %d in table %s is not a STRING format!\n", i * 2 + 0, _DEATH_EXPLOSION_TABLE_NAME );
			goto _ExitWithError;
		}

		// Find the bone from the world mesh
		m_paDeathExplosions[ i ].nBoneIndex = m_pWorldMesh->FindBone( pszExplosionBoneName );
		if( m_paDeathExplosions[ i ].nBoneIndex < 0 ) {
			DEVPRINTF( "CBotCorrosive::_LoadDeathExplosions() : Unable to locate Damage Explosion bone %s\n", pszExplosionBoneName );
			goto _ExitWithError;
		}

		// Now load the time to trigger this explosion, -1 means we need a random time.
		m_paDeathExplosions[ i ].fTriggerTime = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * 2 + 1, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CBotCorrosive::_LoadDeathExplosions() : Field %d in table %s is not a FLOAT format!\n", i * 3 + 1, _DEATH_EXPLOSION_TABLE_NAME );
			goto _ExitWithError;
		}
		
		// Check to see if this is a random time, and if it is, then create a random deallywhopper
		// It's getting late (1:00 am), and deallywhopper sounds like a pretty legitimate term right now.
		if( m_paDeathExplosions[ i ].fTriggerTime == -1.0f ) {
			// Do the deallywhopper
			m_paDeathExplosions[ i ].fTriggerTime = fmath_RandomFloatRange( _DEATH_EXPLOSION_MIN_TIME, _DEATH_EXPLOSION_MAX_TIME );
		}
	}

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( hMemFrame );
	return TRUE;

_ExitWithError:

	// Failure
	fmem_ReleaseFrame( hMemFrame );
	return FALSE;
}


void CBotCorrosive::ClassHierarchyResolveEntityPointerFixups( void ) {
	CEntity::ClassHierarchyResolveEntityPointerFixups();

	if( m_pBotTossAlarmNet ) {
		cchar* pszBotTossAlarmNet = (cchar*) m_pBotTossAlarmNet;
		m_pBotTossAlarmNet = CAlarmNet::FindAlarmNet( pszBotTossAlarmNet );
		if( !m_pBotTossAlarmNet ) {
			DEVPRINTF( "CBotCorrosive::ClassHierarchyResolveEntityPointerFixups() : WARNING -- Didn't find Alarm Net %s\n", pszBotTossAlarmNet );
		}
	}

	if( m_pBotTossAlarmSpawnSpline ) {
		cchar* pszBotTossAlarmSpawnSpline = (cchar*) m_pBotTossAlarmSpawnSpline;
		m_pBotTossAlarmSpawnSpline = CEntity::Find( pszBotTossAlarmSpawnSpline );
		if( !m_pBotTossAlarmSpawnSpline ) {
			DEVPRINTF( "CBotCorrosive::ClassHierarchyResolveEntityPointerFixups() : WARNING -- Didn't find Alarm spawn spline %s\n", pszBotTossAlarmSpawnSpline );
		}
	}

	//Now, determine if we have both a bot toss alarm pointer and an alarm spawn spline pointer 
	if( m_pBotTossAlarmNet && m_pBotTossAlarmSpawnSpline ) {
		FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_CANTOSSBOTS );

		// Attach the spawn point to the Corrosives primary fire bone (for the time being -- may decide to use his hand later)
		m_pBotTossAlarmSpawnSpline->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[ BONE_PRIMARY_FIRE ], &CFMtx43A::m_IdentityMtx, FALSE );
		m_pBotTossAlarmNet->m_uNumAutoBotsSoFar = 0; // Set it to zero deployed auto bots
	}
}



void CBotCorrosive::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	if( m_pWorldMesh ) {
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	_ClearDataMembers();

	CBot::ClassHierarchyDestroy();
}

void CBotCorrosive::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	m_pWorldMesh->UpdateTracker();
}

void CBotCorrosive::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	fforce_Kill( &m_hForce );

	m_pWorldMesh->RemoveFromWorld();


	CBot::ClassHierarchyRemoveFromWorld();
}


BOOL CBotCorrosive::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t frame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

//	IgnoreBotVBotCollision();
	SetBotFlag_MeshCollideOnly();
	SetJumpPadDenialFlag();
	if( IsDamagedVersion() ) {
		UseSphereTestForBlastDamage();
	}

	SetTargetable( TRUE );

	SetProjectileReaction( PROJECTILE_REACTION_DEFAULT );

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


CEntityBuilder* CBotCorrosive::GetLeafClassBuilder( void ) {
	return &_BotCorrosiveBuilder;
}


void CBotCorrosive::ClassHierarchyWork( void ) {
	const CFVec3A *pSpherePos_WS;
	CFVec3A PrevSpherePos_WS, vTmp, vTmp2, vPrevLFootPos_WS, vPrevRFootPos_WS;

	FASSERT( IsSystemInitialized() );

	PROTRACK_BEGINBLOCK( "Corrosive" );


	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	////////////////////////////////////////
	//DBG STUFF
/*
	m_BotInfo_Walk.fMaxXlatVelocity				= 30.0f;
	m_BotInfo_Walk.fNormVelocityStepSize		= 1.0f;

	m_BotInfo_Walk.fMinWalkVelocity				= 4.0f;
	m_BotInfo_Walk.fDeltaFeetAnimPerFootWalk	= 0.023f;
	m_BotInfo_Walk.fDeltaFeetAnimPerFootRun		= 0.04f;
	m_BotInfo_Walk.fMinRunBlendVelocity			= 33.0f;
	m_BotInfo_Walk.fMaxRunBlendVelocity			= 35.0f;


	m_BotInfo_Walk.fInvMaxXlatVelocity	= 1.0f/m_BotInfo_Walk.fMaxXlatVelocity;
	m_pBotInfo_Walk->fMinWalkNormVelocity = m_pBotInfo_Walk->fMinWalkVelocity / m_BotInfo_Walk.fMaxXlatVelocity;
	m_BotInfo_Walk.fMinRunBlendNormVelocity = m_pBotInfo_Walk->fMinRunBlendVelocity / m_BotInfo_Walk.fMaxXlatVelocity;
	m_BotInfo_Walk.fMaxRunBlendNormVelocity = m_pBotInfo_Walk->fMaxRunBlendVelocity / m_BotInfo_Walk.fMaxXlatVelocity;


	ftext_DebugPrintf( 0.5f, 0.2f, "~w1speed: %0.2f", m_fSpeed_WS );
	ftext_DebugPrintf( 0.5f, 0.23f, "~w1walk: %0.2f", GetControlValue( ANIMCONTROL_WALK ) );
//	ftext_DebugPrintf( 0.5f, 0.26f, "~w1run: %0.2f", GetControlValue( ANIMCONTROL_RUN ) );
*/
	//DBG
	////////////////////////////////////////
#endif

	Power_Work();
	DataPort_Work();

	ParseControls();

	// Save a copy of previous frame info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// Add in any velocity impulse...
	BOOL bImpulseApplied = HandleVelocityImpulses();

	// Update position...
	vTmp.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( vTmp );
	m_fMaxDeltaJumpVelY_WS = m_pBotInfo_Jump->fVerticalVelocityJump1 + m_pBotInfo_Jump->fVerticalVelocityJump2;

	// Pitch & Yaw... (check to see if we are allowed to yaw right now...)
	if( m_nCorrosiveFlags & CORROSIVEFLAG_ZEROROTATION ) {
		m_fControls_RotateCW = 0.0f; //zero out any rotational inputs!
	}

	// Check to see if we should slow the bot down when the AI is in possession
	if( m_nCorrosiveFlags & CORROSIVEFLAG_NOAIXLAT ) {
		m_ControlsBot_XlatNormSpeedXZ_WS.Zero();
	}

	// Handle pitch and yaw...
	HandlePitchMovement();
	HandleYawMovement();

	HandleHeadLook();

	// Rotate model space velocity to account for yaw change...
	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// If an external velocity impulse was applied above, recompute our velocities
	// and speeds...
	if( bImpulseApplied ) {
		VelocityHasChanged();
	}

	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();


	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
		HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	switch( m_nState ) {
		case STATE_GROUND:
			PROTRACK_BEGINBLOCK("GroundXlat");
				HandleGroundTranslation();
				_HandleJumping();
			PROTRACK_ENDBLOCK();//"GroundXlat");

			PROTRACK_BEGINBLOCK("GroundAnim");
				HandleGroundAnimations();
			PROTRACK_ENDBLOCK();//"GroundAnim");
			break;


		case STATE_AIR:
			if( IsSleeping() ) {
				break;
			}
			PROTRACK_BEGINBLOCK("AirXlat");
				HandleAirTranslation();
			PROTRACK_ENDBLOCK();//"AirXlat");

			PROTRACK_BEGINBLOCK("AirAnim");
				if( HandleAirAnimations() ) {
					_EnterFlyMode();
				}
			PROTRACK_ENDBLOCK();//"AirAnim");
			break;
	}

	PROTRACK_BEGINBLOCK("CommonAnim");
		_HandleJumpAnimations();
		HandleHipsAnimation();
		_HandleBehaviorAnimations();
	PROTRACK_ENDBLOCK();//"CommonAnim");

	PROTRACK_BEGINBLOCK("Weapon");
		HandleTargeting();
		_HandleAimAnimations();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	// Check to see if we should be performing fist damage/collision checks
	if( m_nCorrosiveFlags & CORROSIVEFLAG_INFLICTHANDATTACKDAMAGE ) {
		if( m_eBehaviorState == BEHAVIOR_STATE_FIST_SMASH ) {
			pSpherePos_WS = &m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxFistDamageSphere ]->m_vPos;
		}
		else if ( m_eBehaviorState == BEHAVIOR_STATE_SWAT ) {
			pSpherePos_WS = &m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxSwatDamageSphere ]->m_vPos;
		}
		else {
			FASSERT_NOW;
		}
		PrevSpherePos_WS = *pSpherePos_WS;
	}

	// Grab the positions of the feet before we move them
	vPrevLFootPos_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLFoot ]->m_vPos;
	vPrevRFootPos_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRFoot ]->m_vPos;

	// Do Damage work here
	_DamageWork();

	// Do the limb tracker pre _UpdateMatricies() work
	_LimbTrackerWork_PreUpdateMatricies();

	_UpdateMatrices();

	// Do the limb tracker post _UpdateMatricies() work
	_LimbTrackerWork_PostUpdateMatricies();

	// Handle weapon firing here
	if( m_nCorrosiveFlags & CORROSIVEFLAG_RELEASEROCKET ) {
		_FireRocket();
		FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ROCKETSRELEASED );
		FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_RELEASEROCKET );
	}

	// Handle Damage results here....
	if( m_nCorrosiveFlags & CORROSIVEFLAG_INFLICTHANDATTACKDAMAGE ) {
		_PerformHandAttackDamageCheck( &PrevSpherePos_WS, pSpherePos_WS );
		//fdraw_DevSphere( &pSpherePos_WS->v3, m_CollInfo.ProjSphere.m_fRadius );
		//fdraw_DevSphere( &PrevSpherePos_WS.v3, m_CollInfo.ProjSphere.m_fRadius, &FColor_MotifRed );
	}
	// Now, handle foot collisions based on new position of feet
	_CheckForFootCollisions( &vPrevLFootPos_WS, &vPrevRFootPos_WS );

//#if SAS_ACTIVE_USER == SAS_USER_RUSS
//	fdraw_DevSphere( &m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPrimaryFire]->m_vPos.v3, 2.0f );
//#endif

	_BotTossWork();

	if( IsDeadOrDying() ) {
		DeathWork();
	}

#if _DRAW_VSPOT_SPHERES == 1
	u32 i;
	for( i = 0; i < m_nVSpotCount; i++ ) {
		if( ( m_paVSpots[ i ].nLimbTypeID == -1 ) || ( m_pPartMgr->GetLimbState( m_paVSpots[ i ].nLimbTypeID ) == CBotPartMgr::LIMB_STATE_INTACT ) ) {
			fdraw_DevSphere( &m_pWorldMesh->GetBoneMtxPalette()[ m_paVSpots[ i ].nBoneIndex ]->m_vPos.v3, m_paVSpots[ i ].fRadius );
		}
	}
#endif

	//ftext_DebugPrintf( 0.4f, 0.33f, "~w1Health:  %0.2f", NormHealth() );

	PROTRACK_ENDBLOCK(); //"Corrosive"

	// Only do these things if the player is in control of the bot.
	if ( m_nPossessionPlayerIndex >= 0 )	{

		// MELEE -- FIST SMASH
		if( m_bControls_Melee && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
			RequestBehavior( BEHAVIOR_STATE_FIST_SMASH );
		}

		// FOOT STOMP
		if( ( m_fControls_Fire2 > 0.01f ) && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
			RequestBehavior( BEHAVIOR_STATE_FORWARD_STOMP );
		}

		////TEST STUFF FOR CHEST ATTACKS
		if( ( m_fControls_Fire1 > 0.01f ) && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
			RequestBehavior( BEHAVIOR_STATE_CHEST_ROCKETLAUNCH );
		}


	#if SAS_ACTIVE_USER == SAS_USER_RUSS
	//	////TEST STUFF FOR SWATTING
	//	if( ( m_bControls_Melee > 0.01f ) && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
	//		m_eRequestedBehaviorState = BEHAVIOR_STATE_SWAT;
	//	}
	#endif

	#if SAS_ACTIVE_USER == SAS_USER_RUSS
	//	////TEST STUFF FOR BOT TOSSING
	//	if( ( m_fControls_Fire2 > 0.01f ) && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
	//		m_eRequestedBehaviorState = BEHAVIOR_STATE_BOT_TOSS;
	//	}
	#endif


	//#if SAS_ACTIVE_USER == SAS_USER_RUSS
	//	////TEST STUFF FOR FINGER FLICKING
	//	if( ( m_fControls_Fire1 > 0.01f ) && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
	//		m_eRequestedBehaviorState = BEHAVIOR_STATE_FINGER_FLICK;
	//	}
	//#endif

	//#if SAS_ACTIVE_USER == SAS_USER_RUSS
	//	////TEST STUFF FOR CHEST BEATING
	//	if( ( m_fControls_Fire1 > 0.01f ) && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
	//		m_eRequestedBehaviorState = BEHAVIOR_STATE_BEAT_CHEST;
	//	}
	//#endif

	//#if SAS_ACTIVE_USER == SAS_USER_RUSS
		////TEST STUFF FOR PEERING UNDER
	//	if( ( m_fControls_Fire2 > 0.01f ) && ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) ) {
	//		m_eRequestedBehaviorState = BEHAVIOR_STATE_PEER_UNDER;
	//	}
	//#endif
	}
}


// Used by the bottalk system
void CBotCorrosive::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}


const CFVec3A* CBotCorrosive::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}

void CBotCorrosive::ComputeExactMuzzlePoint_WS( CFVec3A *pExactMuzzlePoint_WS ) {
	*pExactMuzzlePoint_WS = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPrimaryFire]->m_vPos;
}


	
void CBotCorrosive::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
	static const u8 *__apnUserAnimBoneMaskIndex[UABONEMASK_COUNT] = {
		m_aBoneEnableIndices_UserAnim_UpperBody,
		m_aBoneEnableIndices_UserAnim_LowerBody,
		m_aBoneEnableIndices_UserAnim_UpperTorso,
		m_aBoneEnableIndices_UserAnim_LowerTorso,
		m_aBoneEnableIndices_UserAnim_LeftArm,
		m_aBoneEnableIndices_UserAnim_RightArm,
		m_aBoneEnableIndices_UserAnim_Head

	};

	FASSERT( IsCreated() );
	FASSERT( nBoneMaskGroup>=0 && nBoneMaskGroup<UABONEMASK_COUNT );

	CBot::UserAnim_BatchUpdateTapBoneMask( __apnUserAnimBoneMaskIndex[nBoneMaskGroup], m_apszBoneNameTable );
}



void CBotCorrosive::_LimbTrackerWork_PreUpdateMatricies( void ) {
	// First, run through our limb trackers and see if any have gone from
	// an intact state to a dangling state...

	if( !IsDamagedVersion() ) {
		// We only do the bot part stuff if this is the damaged version
		return;
	}

	u32 nTrackerIdx, nElementIdx;
	CFSphere spBone;

	const CFSphere *pBoneSphere_BS;
	CFMtx43A *pBoneMtx;
	CFVec3A BoneSphereCenter_WS;

	// Check a couple of special cases here...
	if( ( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM_UPPER ) == CBotPartMgr::LIMB_STATE_DANGLING ) &&
		( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM_LOWER ) == CBotPartMgr::LIMB_STATE_INTACT ) ) {
		// we need those bones in the lower arm to track explosions and such when they pass through 
		//the ground as well
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_LEFT_ARM_LOWER, FALSE, m_BotInfo_Gen.fGravity );
	}
	if( ( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_LEG_UPPER ) == CBotPartMgr::LIMB_STATE_DANGLING ) &&
		( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_LEG_LOWER ) == CBotPartMgr::LIMB_STATE_INTACT ) ) {
		// we need those bones in the lower arm to track explosions and such when they pass through 
		//the ground as well
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_RIGHT_LEG_LOWER, FALSE, m_BotInfo_Gen.fGravity );
	}

	// Now, run through all the limbs and manage them.
	FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_LOSTLIMBTHISFRAME );
	for( nTrackerIdx = 0; nTrackerIdx < LIMB_TYPE_COUNT; nTrackerIdx++ ) {
		if( ( m_pPartMgr->GetLimbState( nTrackerIdx ) == CBotPartMgr::LIMB_STATE_DANGLING ) && 
			( m_paLimbTrackers[ nTrackerIdx ].nPreviousLimbState != CBotPartMgr::LIMB_STATE_DANGLING ) ) {
			// This limb just started to dangle, lets clear the array
			for( nElementIdx = 0; nElementIdx < m_paLimbTrackers[ nTrackerIdx ].nLimbElements; nElementIdx++ ) {
				LimbElement_t *pLimbElement = &m_paLimbTrackers[ nTrackerIdx ].paElement[ nElementIdx ];
				pLimbElement->bAlreadyImpacted = FALSE;

				//Compute the initial position of the bounding sphere
				pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ pLimbElement->nBoneIndex ];
				pBoneSphere_BS = &m_pWorldMesh->m_pMesh->pBoneArray[ pLimbElement->nBoneIndex ].SegmentedBoundSphere_BS;
				BoneSphereCenter_WS.Set( pBoneSphere_BS->m_Pos );
				pBoneMtx->MulPoint( pLimbElement->vPos, BoneSphereCenter_WS );
			}
			FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_LOSTLIMBTHISFRAME );
		}

		// Check to see if we should remove a limb that has already been set to dangle.
		if( m_pPartMgr->GetLimbState( nTrackerIdx ) == CBotPartMgr::LIMB_STATE_DANGLING ) {
			BOOL bAllImpacted = TRUE;
			for( nElementIdx = 0; nElementIdx < m_paLimbTrackers[ nTrackerIdx ].nLimbElements; nElementIdx++ ) {
				LimbElement_t *pLimbElement = &m_paLimbTrackers[ nTrackerIdx ].paElement[ nElementIdx ];
				if( !pLimbElement->bAlreadyImpacted ) {
					bAllImpacted = FALSE;
					break;
				}
			}
			if( bAllImpacted ) {
				// We should remove this limb now that all of it's bones have impacted with the ground...
				m_pPartMgr->SetState_Removed( nTrackerIdx, TRUE, FALSE );
			}
		}
		// Save off the previous state of the limb...
		m_paLimbTrackers[ nTrackerIdx ].nPreviousLimbState = m_pPartMgr->GetLimbState( nTrackerIdx );
	}
}


void CBotCorrosive::_LimbTrackerWork_PostUpdateMatricies( void ) {
	// In this routine, we want to run through the limb trackers, determine if any are in a dangling state,
	// and if so, do a collision check to see if we should spawn debris or anything like that...

	if( !IsDamagedVersion() ) {
		// We only do the bot part stuff if this is the damaged version
		return;
	}

	u32 nTrackerIdx, nElementIdx;
	CFSphere spBone;
	CFVec3A vTmp; 
	FCollImpact_t CollImpact;

	const CFSphere *pBoneSphere_BS;
	CFMtx43A *pBoneMtx;
	CFVec3A BoneSphereCenter_WS;

	for( nTrackerIdx = 0; nTrackerIdx < LIMB_TYPE_COUNT; nTrackerIdx++ ) {
		// Now, for any limb types that are danging, get the current bounding sphere positions of the bones
		if( m_paLimbTrackers[ nTrackerIdx ].nPreviousLimbState == CBotPartMgr::LIMB_STATE_DANGLING ) {
			for( nElementIdx = 0; nElementIdx < m_paLimbTrackers[ nTrackerIdx ].nLimbElements; nElementIdx++ ) {
				LimbElement_t *pLimbElement = &m_paLimbTrackers[ nTrackerIdx ].paElement[ nElementIdx ];
				if( !pLimbElement->bAlreadyImpacted ) {

					// Calculate the new world space bounding sphere position of the bone
					pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ pLimbElement->nBoneIndex ];
					pBoneSphere_BS = &m_pWorldMesh->m_pMesh->pBoneArray[ pLimbElement->nBoneIndex ].SegmentedBoundSphere_BS;
					BoneSphereCenter_WS.Set( pBoneSphere_BS->m_Pos );
					pBoneMtx->MulPoint( vTmp, BoneSphereCenter_WS );

					if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &pLimbElement->vPos, &vTmp, 0, NULL, TRUE, m_pWorldMesh, ~ENTITY_BITS_ALLBOTBITS, FCOLL_MASK_COLLIDE_WITH_DEBRIS, FCOLL_LOD_HIGHEST ) ) {
						// We hit something!
						
						// Spawn some explosion and debris love here!!!
						_LimbTrackerWork_SpawnExplosion( m_BotInfo_Corrosive.hLimbExplosion, vTmp, pBoneSphere_BS->m_fRadius/*spBone.m_fRadius*/ );

						// Lastly, set this impact to TRUE, so we don't check this guy any more...
						pLimbElement->bAlreadyImpacted = TRUE;
					} else {
						// Save off the newly computed position so we can use it
						// next frame for the same test
						pLimbElement->vPos.Set( vTmp );
					}
				}
			}
		}
	}
}


void CBotCorrosive::_LimbTrackerWork_SpawnExplosion( FExplosion_GroupHandle_t hExplosion, CFVec3A &vPos, f32 fRadius ) {
	if( hExplosion != FEXPLOSION_INVALID_HANDLE ) {
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();

			SpawnParams.Pos_WS.Set( fRadius, 0.0f, 0.0f );
			SpawnParams.Pos_WS.RotateY( fmath_RandomFloatRange( 0.0f, FMATH_2PI ) );
			SpawnParams.Pos_WS.Add( vPos );

			SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;

			CExplosion2::SpawnExplosion( hSpawner, hExplosion, &SpawnParams );
		}
	}
}




void CBotCorrosive::_UpdateMatrices( void ) {
	if( m_pDrivingVehicle ) {
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
		ComputeMtxPalette( TRUE );
		return;
	}

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");
	RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated

	if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() /*|| (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) */ ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	} else {
		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	}

	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );
}


void CBotCorrosive::_AnimBoneCallback( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCBCorrosive );
	FASSERT( m_pCBCorrosive->TypeBits() & ENTITY_BIT_BOTCORROSIVE );

	if( !m_pCBCorrosive ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	// give part mgr first shot
	if( m_pCBCorrosive->m_pPartMgr->AnimBoneCallbackFunctionHandler( uBoneIdx, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	// Groin
	} else if( uBoneIdx == m_pCBCorrosive->m_nBoneIdxGroin ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;

	// Torso
	} else if( uBoneIdx == m_pCBCorrosive->m_nBoneIdxTorso ) {
        CFMtx43A	mtxWorld;
		CFQuatA		q;

		q.BuildQuat( m_GroinVecY_WS, -m_pCBCorrosive->m_fLegsYaw_MS );
		mtxWorld.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = mtxWorld.m_vPos;
		q.MulPoint( rNewMtx.m_vRight, mtxWorld.m_vRight );
		q.MulPoint( rNewMtx.m_vUp, mtxWorld.m_vUp );
		q.MulPoint( rNewMtx.m_vFront, mtxWorld.m_vFront );

	// Head
	} else if( uBoneIdx == m_pCBCorrosive->m_nBoneIdxHead ) {
		m_pCBCorrosive->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );

	// Unknown bone, not good...
	} else {
		FASSERT_NOW;
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
}


void CBotCorrosive::Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier ) {
	CBot::Possess( nPlayerIndex, fPossessionArmorModifier );

	// Set up our HUD
	// We want to disable the ability to exit Corrosive... Thats not allowed!
	CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIndex);
//	pHud->ClearDrawFlags( CHud2::DRAW_SELFDESTRUCT ); // Dont draw the self destruct messages.
}


void CBotCorrosive::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


BOOL CBotCorrosive::CheckpointSave( void ) {
	// Save parent class data...
	CBot::CheckpointSave();

	// Save bot class data. Order must match load order below!

	return TRUE;
}


void CBotCorrosive::CheckpointRestore( void ) {
	CBot::CheckpointRestore();
	u32 i;

	m_eBehaviorState = m_eRequestedBehaviorState = m_eLastBehaviorState = BEHAVIOR_STATE_NONE;
	// Reload / Reset specific variables here...
	m_fSavedNormHealth = NormHealth();

	// Clear all the flags except for DamagedVersion and CanTossBots
	m_nCorrosiveFlags &= ( CORROSIVEFLAG_DAMAGEDVERSION | CORROSIVEFLAG_CANTOSSBOTS );
	if( m_pafDamageMilestones && ( m_fSavedNormHealth <= m_pafDamageMilestones[ m_nDamageMilestones - 1 ] ) ) {
		FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_CRITICALDAMAGEREACHED );
	}
	SetJumpPadDenialFlag(); // Reset the denial flag, as it gets cleared on a restore.

	// Zero out all our specific action animations here
	SetControlValue( ANIMCONTROL_STOMP, 0.0f );
	SetControlValue( ANIMCONTROL_FORWARD_STOMP, 0.0f ); 
	SetControlValue( ANIMCONTROL_FIST_SMASH_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_FIST_SMASH_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_SWAT, 0.0f );
	SetControlValue( ANIMCONTROL_BOT_TOSS, 0.0f );
	SetControlValue( ANIMCONTROL_BEAT_CHEST, 0.0f );
	SetControlValue( ANIMCONTROL_FINGER_FLICK, 0.0f );
	SetControlValue( ANIMCONTROL_PEER_UNDER, 0.0f );
	SetControlValue( ANIMCONTROL_OPEN_CHEST_DOORS, 0.0f );
	SetControlValue( ANIMCONTROL_DEATH, 0.0f );

	// Kill any bot toss effects we may have in progress
	if( m_hBotTossFXEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hBotTossFXEmitter );
		m_hBotTossFXEmitter = FPARTICLE_INVALID_HANDLE;
	}

	// Kill any vspot emitters we may have in progress
	for( i = 0; i < m_nVSpotCount; i++ ) {
		if( m_paVSpots[ i ].hVSpotHitFXEmitter != FPARTICLE_INVALID_HANDLE ) {
			fparticle_StopEmitter( m_paVSpots[ i ].hVSpotHitFXEmitter );
			m_paVSpots[ i ].hVSpotHitFXEmitter = FPARTICLE_INVALID_HANDLE;
		}
	}
}


BOOL CBotCorrosive::RequestBehavior( BehaviorState_e eRequestedBehavior ) {

	if( IsDeadOrDying() ) {
		return FALSE;
	}

	if( ( m_eBehaviorState != BEHAVIOR_STATE_NONE ) || ( m_eRequestedBehaviorState != BEHAVIOR_STATE_NONE ) ) {
		return FALSE;
	}

	// If we are in the air, then we can't do anything special...
	if( IsInAir() || IsJumping() ) {
		return FALSE;
	}

	// If we are requesting a stomp, make sure we are not limping...
	if( ( ( eRequestedBehavior == BEHAVIOR_STATE_STOMP ) || ( eRequestedBehavior == BEHAVIOR_STATE_FORWARD_STOMP ) ) && IsLimping() ) {
		return FALSE;
	}

	// If a bot toss has been requested, make sure that the alarm system and such is set up.
	if( ( eRequestedBehavior == BEHAVIOR_STATE_BOT_TOSS ) && !CanTossABot() ) {
		return FALSE;
	}

	// We currently don't have a requested behavior state, and we have no funky conditions, so...
	m_eRequestedBehaviorState = eRequestedBehavior;

	return TRUE;
}

BOOL CBotCorrosive::IsFingerFlicked() {

	if ( m_eBehaviorState != BEHAVIOR_STATE_FINGER_FLICK ) {
		return FALSE;
	}

	f32 fTime = GetTime( ANIMTAP_FINGER_FLICK );

	if (fTime > 2.3f) return TRUE;

	return FALSE;
}

BOOL CBotCorrosive::IsInMiddleOfSwat() {
	if ( m_eBehaviorState != BEHAVIOR_STATE_SWAT ) {
		return FALSE;
	}
	
	f32 fTime = GetTime( ANIMTAP_SWAT );

	//Hit time ~ 0.98
	//if (fTime < 1.50f)	{
	//	return TRUE;
	//}

	return TRUE;
}

BOOL CBotCorrosive::IsEntityHit( CEntity *pEntity ) {
	u32 i;

	if (m_nHitEntityCount < 1) { return FALSE; }
	for (i=0; i<m_nHitEntityCount; i++) {
		if (m_apHitEntity[i] == pEntity) {
			return TRUE;
		}
	}
	return FALSE;
}

void CBotCorrosive::ClearEntityHitList() {
	m_nHitEntityCount = 0;
}

void CBotCorrosive::_HandleBehaviorAnimations( void ) {

	// We are not in an attack state, and don't have one requested, then don't do anything!
	if( ( m_eBehaviorState == BEHAVIOR_STATE_NONE ) && ( m_eRequestedBehaviorState == BEHAVIOR_STATE_NONE ) ) {
		return;
	}

	if( m_eBehaviorState == BEHAVIOR_STATE_NONE ) {
		// We currently don't have an attack state in progress, so
		// Lets evaluate what our requested attack state is...

		switch( m_eRequestedBehaviorState ) {
			case BEHAVIOR_STATE_STOMP:
				ZeroTime( ANIMTAP_STOMP );
				_InitStartFootStompData();
				break;

			case BEHAVIOR_STATE_FORWARD_STOMP:
				ZeroTime( ANIMTAP_FORWARD_STOMP );
				_InitStartFootStompData();
				break;

			case BEHAVIOR_STATE_FIST_SMASH:
				ZeroTime( ANIMTAP_FIST_SMASH_UPPER );
				ZeroTime( ANIMTAP_FIST_SMASH_LOWER );
				_InitStartHandAttackData();
				break;

			case BEHAVIOR_STATE_SWAT:
				ZeroTime( ANIMTAP_SWAT );
				break;

			case BEHAVIOR_STATE_BOT_TOSS:
				ZeroTime( ANIMTAP_BOT_TOSS );
				m_fBotTossParticleTime = 0.0f;
				FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ALARMBOTSPAWNED | CORROSIVEFLAG_ALARMBOTRELEASED );
				FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_DOBOTTOSSWORK );
				break;

			case BEHAVIOR_STATE_CHEST_ROCKETLAUNCH:
				ZeroTime( ANIMTAP_OPEN_CHEST_DOORS );
				FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ROCKETSRELEASED | CORROSIVEFLAG_RELEASEROCKET );
				break;

			case BEHAVIOR_STATE_FINGER_FLICK:
				ZeroTime( ANIMTAP_FINGER_FLICK );
				break;

			case BEHAVIOR_STATE_BEAT_CHEST_AND_ROAR:
				ZeroTime( ANIMTAP_BEAT_CHEST );
				if (m_bForce2D_RoarRocket)
				{
					PlaySound( m_BotInfo_Corrosive.pSoundGroupAngryRoar, 1.0f, 1.0f, -1.0f, NULL, TRUE ); // Start the angry roar sound as well
				}
				else
				{
					PlaySound( m_BotInfo_Corrosive.pSoundGroupAngryRoar ); // Start the angry roar sound as well
				}
				break;

			case BEHAVIOR_STATE_BEAT_CHEST_AND_LAUGH:
				ZeroTime( ANIMTAP_BEAT_CHEST );
				if (m_bForce2D_RoarRocket)
				{
					PlaySound( m_BotInfo_Corrosive.pSoundGroupLaugh, 1.0f, 1.0f, -1.0f, NULL, TRUE ); // Start the angry roar sound as well
				}
				else
				{
					PlaySound( m_BotInfo_Corrosive.pSoundGroupLaugh ); // Start the laughing sound as well
				}
				break;

			case BEHAVIOR_STATE_PEER_UNDER:
				ZeroTime( ANIMTAP_PEER_UNDER );
				break;

			default:
				FASSERT_NOW; // Should not be here!
		}
		m_eLastBehaviorState = m_eBehaviorState;
		m_eBehaviorState = m_eRequestedBehaviorState;
		m_eRequestedBehaviorState = BEHAVIOR_STATE_NONE;
	}

	// If we are here, then we should check to see if we still have a valid attack state
	if( m_eBehaviorState != BEHAVIOR_STATE_NONE ) {
		f32 fTime, fPrevTime;
		f32 fControlVal;
		switch( m_eBehaviorState ) {
			case BEHAVIOR_STATE_STOMP:
				_HandleFootStompAnimation( ANIMTAP_STOMP );
				break;

			case BEHAVIOR_STATE_FORWARD_STOMP:
				_HandleFootStompAnimation( ANIMTAP_FORWARD_STOMP );
				break;

			case BEHAVIOR_STATE_FIST_SMASH:
				_HandleFistSmashAnimation();
				break;

			case BEHAVIOR_STATE_SWAT:
				_HandleSwatAnimation();
				break;

			case BEHAVIOR_STATE_BOT_TOSS:
				_HandleBotTossAnimation();
				break;

			case BEHAVIOR_STATE_CHEST_ROCKETLAUNCH:
				if( !DeltaTime( ANIMTAP_OPEN_CHEST_DOORS, FLoop_fPreviousLoopSecs, TRUE ) ) {
					// We are still opening
					fControlVal = 1.0f; //default to full chest blend
					fTime = GetTime( ANIMTAP_OPEN_CHEST_DOORS );
					if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
						// Blend in the launch...
						fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					} else if( fTime > ( GetTotalTime( ANIMTAP_OPEN_CHEST_DOORS ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
						// Check to see if we should be blending out...
						fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( ANIMTAP_OPEN_CHEST_DOORS ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					}

					// The AI Version of corrosive must stop to fire from his chest...
					if( m_nPossessionPlayerIndex == -1 ) {
						FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_NOAIXLAT );
					}

					// Check to see if it's time to release the rockets
					if( !( m_nCorrosiveFlags & CORROSIVEFLAG_ROCKETSRELEASED ) && ( fTime >= m_BotInfo_Corrosive.fRocketReleaseAnimTime ) ) {
						FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_RELEASEROCKET );
					}
//					ftext_DebugPrintf( 0.4f, 0.33f, "~w1time:  %0.2f", fTime );
				}
				else {
					// We are done firing our rocket
					fControlVal = 0.0f;
					m_eLastBehaviorState = m_eBehaviorState;
					m_eBehaviorState = BEHAVIOR_STATE_NONE;
					
					// Re-enable AI translation
					FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_NOAIXLAT );
				}
				SetControlValue( ANIMCONTROL_OPEN_CHEST_DOORS, fControlVal);
				break;

			case BEHAVIOR_STATE_FINGER_FLICK:
				fPrevTime = GetTime( ANIMTAP_FINGER_FLICK );
				if( !DeltaTime( ANIMTAP_FINGER_FLICK, FLoop_fPreviousLoopSecs, TRUE ) ) {
					// We are still opening
					fControlVal = 1.0f; // Default to full fingerflick blend
					fTime = GetTime( ANIMTAP_FINGER_FLICK );
					if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
						// Blend in the stomp...
						fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					} else if( fTime > ( GetTotalTime( ANIMTAP_FINGER_FLICK ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
						// Check to see if we should be blending out...
						fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( ANIMTAP_FINGER_FLICK ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					}

					// Don't allow ourself to rotate!
					FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );

					// Reorient our groin...
					f32 fReorientTime = fTime;
					FMATH_CLAMPMAX( fReorientTime, _REORIENT_LEGS_TIME );
					FMATH_BIPOLAR_CLAMPMAX( m_fLegsYaw_MS, ( _REORIENT_LEGS_TIME - fReorientTime ) * m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold );

					// Gradually slow down our velocity over _ZERO_VELOCITY_TIME
					f32 fVelocityTime = fTime;
					FMATH_CLAMPMAX( fVelocityTime, _ZERO_VELOCITY_TIME );
					m_Velocity_WS.Mul( ( _ZERO_VELOCITY_TIME - fVelocityTime ) * _OO_ZERO_VELOCITY_TIME );
					VelocityHasChanged();
					// If the AI is in possession of corrosive, have him laugh
					if( m_nPossessionPlayerIndex == -1 ) {
						if( ( fPrevTime < _FINGER_FLICK_LAUGH_TIME ) && ( fTime >= _FINGER_FLICK_LAUGH_TIME ) ) {
							PlaySound( m_BotInfo_Corrosive.pSoundGroupLaugh );
						}
					}
					//ftext_DebugPrintf( 0.4f, 0.33f, "~w1time:  %0.2f", fTime );
				}
				else {
					// We are done finger flicking
					fControlVal = 0.0f;
					m_eLastBehaviorState = m_eBehaviorState;
					m_eBehaviorState = BEHAVIOR_STATE_NONE;
					
					// Re-enable rotation
					FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );
				}
				SetControlValue( ANIMCONTROL_FINGER_FLICK, fControlVal);
				break;

			case BEHAVIOR_STATE_BEAT_CHEST_AND_ROAR:
			case BEHAVIOR_STATE_BEAT_CHEST_AND_LAUGH:
				if( !DeltaTime( ANIMTAP_BEAT_CHEST, FLoop_fPreviousLoopSecs, TRUE ) ) {
					// We are still beating -- okay, lets keep it clean
					fControlVal = 1.0f; // Default to full chest-beat blend
					fTime = GetTime( ANIMTAP_BEAT_CHEST );
					if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
						// Blend in the chest beat...
						fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					} else if( fTime > ( GetTotalTime( ANIMTAP_BEAT_CHEST ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
						// Check to see if we should be blending out...
						fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( ANIMTAP_BEAT_CHEST ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					}

					// The AI Version of corrosive must stop to beat his chest...
					if( m_nPossessionPlayerIndex == -1 ) {
						FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_NOAIXLAT );
					}
				}
				else {
					// We are done beating our chest
					fControlVal = 0.0f;
					m_eLastBehaviorState = m_eBehaviorState;
					m_eBehaviorState = BEHAVIOR_STATE_NONE;
					
					// Re-enable AI translation
					FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_NOAIXLAT );
				}
				SetControlValue( ANIMCONTROL_BEAT_CHEST, fControlVal);
				break;

			case BEHAVIOR_STATE_PEER_UNDER:
				if( !DeltaTime( ANIMTAP_PEER_UNDER, FLoop_fPreviousLoopSecs, TRUE ) ) {
					// We are still peering
					fControlVal = 1.0f; // Default to full chest-beat blend
					fTime = GetTime( ANIMTAP_PEER_UNDER );
					if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
						// Blend in the chest beat...
						fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					} else if( fTime > ( GetTotalTime( ANIMTAP_PEER_UNDER ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
						// Check to see if we should be blending out...
						fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( ANIMTAP_PEER_UNDER ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
					}

					// Don't allow ourself to rotate!
					FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );

					// Reorient our groin...
					f32 fReorientTime = fTime;
					FMATH_CLAMPMAX( fReorientTime, _REORIENT_LEGS_TIME );
					FMATH_BIPOLAR_CLAMPMAX( m_fLegsYaw_MS, ( _REORIENT_LEGS_TIME - fReorientTime ) * m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold );

					// Gradually slow down our velocity over _ZERO_VELOCITY_TIME
					f32 fVelocityTime = fTime;
					FMATH_CLAMPMAX( fVelocityTime, _ZERO_VELOCITY_TIME );
					m_Velocity_WS.Mul( ( _ZERO_VELOCITY_TIME - fVelocityTime ) * _OO_ZERO_VELOCITY_TIME );
					VelocityHasChanged();
				}
				else {
					// We are done peering
					fControlVal = 0.0f;
					m_eLastBehaviorState = m_eBehaviorState;
					m_eBehaviorState = BEHAVIOR_STATE_NONE;
					
					// Re-enable rotation
					FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );
				}
				SetControlValue( ANIMCONTROL_PEER_UNDER, fControlVal);
				break;

			default:
				FASSERT_NOW; // Should not be here... bad news
		}
	}
}



void CBotCorrosive::_HandleAimAnimations( void )
{

	f32 fUnitAimPitch;
	CFAnimFrame TorsoQuat, HeadQuat;

	// Compute the amount we're aiming up/down (up=1, down=0, middle=0.5)...
	fUnitAimPitch = fmath_Div( m_fMountPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	// Update the torso-twist blend value...
	if( m_fUnitTorsoTwistBlend > 0.0f )
	{
		m_fUnitTorsoTwistBlend -= 2.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
	}

	// Compute the head and torso quaternions...
	TorsoQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-30.0f), FMATH_DEG2RAD(30.0f) ), CFVec3A::m_Null );

	if( m_fUnitTorsoTwistBlend == 0.0f )
	{
		// No torso twist...
		// Torso quaternion has already been computed.
		// Compute head quaternion...

		HeadQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-20.0f), FMATH_DEG2RAD(20.0f) ), CFVec3A::m_Null );

	} 
	else
	{
		// Partial torso twist...

		// Compute the torso twist quaternion...
		CFAnimFrame TorsoTwistQuat;
		TorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, fmath_UnitLinearToSCurve(m_fUnitTorsoTwistBlend) * FMATH_DEG2RAD(55.0f), CFVec3A::m_Null );
		TorsoQuat.Mul( TorsoTwistQuat );

		// Compute the head quaternion...
		TorsoTwistQuat.Negate();
		CFAnimFrame TempQuat;
		TempQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(20.0f), FMATH_DEG2RAD(-20.0f) ), CFVec3A::m_Null );
		HeadQuat.Mul( TempQuat, TorsoTwistQuat );

	}

	// Apply head and torso quaternions...
	m_AnimManFrameAim.UpdateFrame( BONE_HEAD, HeadQuat );
	m_AnimManFrameAim.UpdateFrame( BONE_TORSO, TorsoQuat );
	// Compute the head quaternion...
}



/////////////////////////
//JUMPING

void CBotCorrosive::_HandleJumping( void ) {

	// Make sure that:
	//	1) We are not limping
	//	2) We are not ingaged in a previous behavior
	//	3) The player is not in control
	if( IsLimping() || ( m_eBehaviorState != BEHAVIOR_STATE_NONE ) || ( m_nPossessionPlayerIndex >= 0 ) ) {
		return;
	}

	if( !m_bControlsBot_JumpVec ) {
		if( m_bControlsBot_Jump2 ) {
			_StartSingleJump();
			_StartDoubleJump();
		} else if( m_bControls_Jump ) {
			_StartSingleJump();
		}
	} else {
		// Velocity jump specified...
		_StartVelocityJump( &m_ControlsBot_JumpVelocity_WS );
	}

	if( m_nPrevState == STATE_AIR ) {
		if( m_nJumpState != BOTJUMPSTATE_NONE ) {
			_JumpLanded();
		}
	}
}


void CBotCorrosive::_EnterFlyMode( void ) {
	m_bPlayLandAnim			= FALSE;
	m_bPlayLandAnimLower	= FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );

	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


#define _BIG_LANDING_THRESHOLD (-75.0f)

void CBotCorrosive::_JumpLanded( void ) {
	MakeFootImpactDust( TRUE, TRUE, m_pBotInfo_Gen->fUnitDustKickup, &m_FeetToGroundCollImpact.UnitFaceNormal );

	m_nJumpState				= BOTJUMPSTATE_NONE;
	ClearJumping();
	m_bPlayLandAnim 			= TRUE;

	ZeroTime( ANIMTAP_JUMP_LAND_UPPER );

	m_fMaxLandUnitBlend = GetControlValue( ANIMCONTROL_JUMP_FLY );
	if( m_fMaxLandUnitBlend == 0.0f ) {
		m_fMaxLandUnitBlend = 1.0f;
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	//f32 fVVel = (m_MountPrevPos_WS.y - m_MountPos_WS.y) * FLoop_fPreviousLoopOOSecs;
//	DEVPRINTF( "vvel = %0.2f\n", m_fVerticalVelocity );

	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fMaxLandUnitBlend );

// RAFNOTE -- I am deciding to play the lower body landing animation in all scenarios right now...
//	if( m_fSpeedXZ_WS == 0.0f ) {
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
		m_bPlayLandAnimLower = TRUE;
//	} else {
//		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
//		m_bPlayLandAnimLower = FALSE;
//	}

	// Play sound...
	if ( m_nPossessionPlayerIndex >= 0 )	{
		// Don't play scuff sounds when landing from a jump
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) {
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}
		// Tell the AI that this sound happened
		AIEnviro_BoostPlayerSoundTo(m_nPossessionPlayerIndex, 35.0f);
	} else {
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) {
			// Don't play scuff sounds when landing from a jump
			fsndfx_Play3D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
				&MtxToWorld()->m_vPos,
				m_pBotInfo_Walk->fJumpLandedSoundRadius_3D,
				1.0f,
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_3D
			);
		}
	}

	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
		CFVec3A ImpulseVec_WS;

		ImpulseVec_WS.Set( 0.0f, -m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
		m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
	}
}


void CBotCorrosive::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS ) {
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 ) {
		_StartDoubleJump( FALSE );
	}
}


void CBotCorrosive::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
	if( pJumpVelocity_WS == NULL ) {
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		MS2WS( m_Velocity_WS, m_Velocity_MS );
	} else {
		m_Velocity_WS.x = pJumpVelocity_WS->x;
		m_Velocity_WS.y = pJumpVelocity_WS->y;
		m_Velocity_WS.z = pJumpVelocity_WS->z;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
	}
		
	VelocityHasChanged();

	m_bPlayLandAnim		 = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );	

	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();
}


void CBotCorrosive::_StartDoubleJump( BOOL bAddToVelocityY ) {
	return;
}


void CBotCorrosive::_HandleJumpAnimations( void ) {
	f32 fTemp, fUnitTime;
	BOOL bAnimFinished;

	if( m_nState == STATE_GROUND ) {
		// We're on the ground...
		if( m_fUnitFlyBlend > 0.0f ) {
			m_fUnitFlyBlend = 0.0f;
			_JumpLanded();
		}

		// Handle landing animation...
		if( m_bPlayLandAnim ) {
			if( m_fSpeedXZ_WS > 0.0f ) {
				fTemp = m_pBotInfo_Jump->fMovingLandAnimSpeedMult;
			} else {
				fTemp = 1.0f;
			}
			
			bAnimFinished = DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs * fTemp );

			if( !bAnimFinished ) {
				// Still playing landing animation...

				f32 fUnitTime = 1.01f - GetUnitTime(ANIMTAP_JUMP_LAND_UPPER);
				fUnitTime = m_fMaxLandUnitBlend * fmath_UnitLinearToSCurve( fUnitTime );

				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, fUnitTime );

				if( m_bPlayLandAnimLower ) {
					SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, fUnitTime );
				}
			} else {
				// Done playing landing animation...

				m_bPlayLandAnim = FALSE;
				m_bPlayLandAnimLower = FALSE;

				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
			}
		} else {
			if( m_nJumpState == BOTJUMPSTATE_NONE ) {
				SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
			}
		}
	} else {
		// We're in the air...

		if( m_nJumpState != BOTJUMPSTATE_NONE ) {

			switch( m_nJumpState ) {
				case BOTJUMPSTATE_LAUNCH:
					if( !DeltaTime( ANIMTAP_JUMP_LAUNCH ) ) {
						fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_JUMP_LAUNCH ) );
						FMATH_CLAMPMAX( fUnitTime, 1.0f );
						SetControlValue( ANIMCONTROL_JUMP_LAUNCH, fUnitTime );
					} else {
						SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
						SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
						ZeroTime( ANIMTAP_JUMP_FLY );
						m_nJumpState = BOTJUMPSTATE_AIR;
						SetJumping();
					}

					break;

				case BOTJUMPSTATE_AIR:
					DeltaTime( ANIMTAP_JUMP_FLY );
					break;

				default:
					FASSERT_NOW;
					break;
			}
		}
	}
}


////////////////////////////
// BOT TOSS SUPPORT ROUTINES

// Returns TRUE if the following condtions are true:
//	1) CORROSIVEFLAG_CANTOSSBOTS is set (meaning an alarm system has been set up for this level)
//  2) Corrosives Left arm (which does the toss) is intact
//	3) No other Behavior is currently in progress
//	4) The alarm net spawning criteria is met so that a free bot that can be tossed
BOOL CBotCorrosive::CanTossABot( void ) {

	// First check to see if an alarm net is even set up
	if( !( m_nCorrosiveFlags & CORROSIVEFLAG_CANTOSSBOTS ) ) {
		return FALSE;
	}

	// Next, see if our left arm parts are intact
	if( m_pPartMgr && ( ( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM_UPPER ) != CBotPartMgr::LIMB_STATE_INTACT ) ||
		( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM_LOWER ) != CBotPartMgr::LIMB_STATE_INTACT ) ) ) {
		return FALSE;
	}

	// Make sure we don't have a current behavor in progress or a requested behavior pending
	if( ( m_eBehaviorState != BEHAVIOR_STATE_NONE ) && ( m_eRequestedBehaviorState != BEHAVIOR_STATE_NONE ) ) {
		return FALSE;
	}

	// Now, return whether or not the Alarm system needs a bot
	return m_pBotTossAlarmNet->NetNeedsABot();
}



void CBotCorrosive::_HandleBotTossAnimation( void ) {
	f32 fTime = 0.0f, fPrevTime, fControlVal;

	//floop_SetTimeScale( 0.1f );
	fPrevTime = GetUnitTime( ANIMTAP_BOT_TOSS );
	if( !DeltaTime( ANIMTAP_BOT_TOSS, FLoop_fPreviousLoopSecs, TRUE ) ) {
		// We are still tossing
		fControlVal = 1.0f; //default to full toss
		fTime = GetTime( ANIMTAP_BOT_TOSS );
		if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
			// Blend in the toss...
			fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		} else if( fTime > ( GetTotalTime( ANIMTAP_BOT_TOSS ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
			// Check to see if we should be blending out...
			fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( ANIMTAP_BOT_TOSS ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		}

		if( !( m_nCorrosiveFlags & CORROSIVEFLAG_ALARMBOTSPAWNED ) && ( fTime >= _BOT_TOSS_ALARM_ON_TIME ) ) {

			// no bots have been spawned yet, so check to see if one can be spawned.
			BOOL bBotSpawned = FALSE;
			s32 nAutoBotID = m_pBotTossAlarmNet->FindAvailableAutoBot();
			if( nAutoBotID != -1 ) {
				// We got a valid autobot ID.  Now, try and deploy the auto bot

				// First, inform the alarm system of the currently targeted entity, and where it is.
				if( m_pTargetedEntity ) {
					m_pBotTossAlarmNet->m_pIntruder = m_pTargetedEntity;
					m_pBotTossAlarmNet->m_LastIntruderPos_WS = m_pTargetedEntity->MtxToWorld()->m_vPos;
				}

				// Now, deploy the auto bot
				if( m_pBotTossAlarmNet->DeployAutoBot( nAutoBotID ) ) {
					// The deployment was successful.  Cache the bot spawned and it's GUID
					m_pAlarmNetSpawnedBot = CAlarmNet::m_papAutoBotPool[ nAutoBotID ];
					FASSERT( m_pAlarmNetSpawnedBot );
					m_nAlarmNetSpawnedBotGUID = m_pAlarmNetSpawnedBot->Guid();

					bBotSpawned = TRUE;
				}
			}

			if( bBotSpawned ) {
				// A bot has been spawned, relocate it's position and attach it to Corrosives hand
				m_pAlarmNetSpawnedBot->Relocate_RotXlatFromUnitMtx_WS( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxFistDamageSphere ] );
				m_pAlarmNetSpawnedBot->Attach_ToParent_WithGlue_WS( this, m_apszBoneNameTable[ BONE_L_HAND_DUMMY ] );
				m_pAlarmNetSpawnedBot->ImmobilizeBot(); // Don't let it move yet...

				// Set the spawned flag
				FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ALARMBOTSPAWNED );

				// Make a (big) flash of light at the spawned location! 
				// Use the primary fire bone, as that's theoretically where the bot came from...
				CFVec3A vFlashPos = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxPrimaryFire ]->m_vFront;
				vFlashPos.Mul( 2.0f );
				vFlashPos.Add( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxPrimaryFire ]->m_vPos );

				CMuzzleFlash::AddFlash_CardPosterXY(
							CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
							vFlashPos,
							25.0f,
							0.75f
							);
			}
		} else if( (m_nCorrosiveFlags & CORROSIVEFLAG_ALARMBOTSPAWNED ) && !( m_nCorrosiveFlags & CORROSIVEFLAG_ALARMBOTRELEASED ) && 
				   ( fTime >= _BOT_TOSS_RELEASE_TIME ) ) {
			// Time to release the bot and apply a small impulse to it
			if( m_pAlarmNetSpawnedBot && ( m_pAlarmNetSpawnedBot->Guid() == m_nAlarmNetSpawnedBotGUID ) ) {
				m_pAlarmNetSpawnedBot->DetachFromParent();
				m_pAlarmNetSpawnedBot->MobilizeBot(); //Free it up to move now...

				//Compute an impulse vector that points in the torso direction.
				CFVec3A vImpulse_WS;
				vImpulse_WS.Set( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxTorso ]->m_vFront );
				//Zero out the Y component in the impulse vector, and reunitize
				vImpulse_WS.UnitizeXZ();
				vImpulse_WS.Mul( _BOT_TOSS_VELOCITY_IMPULSE_MAGNITUDE );
				m_pAlarmNetSpawnedBot->ApplyVelocityImpulse_WS( vImpulse_WS );
			}
			// Set the released flag
			FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ALARMBOTRELEASED );
		}

		// Don't allow ourself to rotate!
		FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );

		// Reorient our groin...
		f32 fReorientTime = fTime;
		FMATH_CLAMPMAX( fReorientTime, _REORIENT_LEGS_TIME );
		FMATH_BIPOLAR_CLAMPMAX( m_fLegsYaw_MS, ( _REORIENT_LEGS_TIME - fReorientTime ) * m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold );

		// Gradually slow down our velocity over _ZERO_VELOCITY_TIME
		f32 fVelocityTime = fTime;
		FMATH_CLAMPMAX( fVelocityTime, _ZERO_VELOCITY_TIME );
		m_Velocity_WS.Mul( ( _ZERO_VELOCITY_TIME - fVelocityTime ) * _OO_ZERO_VELOCITY_TIME );
		VelocityHasChanged();
	} else {
		// We are done with our toss
		fControlVal = 0.0f;
		m_eLastBehaviorState = m_eBehaviorState;
		m_eBehaviorState = BEHAVIOR_STATE_NONE;

		// Clear the no rotation flag
		FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );
		//floop_SetTimeScale( 1.0f );
	}
	SetControlValue( ANIMCONTROL_BOT_TOSS, fControlVal);
//	ftext_DebugPrintf( 0.4f, 0.33f, "~w1time:  %0.2f", fTime );
}



void CBotCorrosive::_BotTossWork( void ) {

	f32 fPrevTime;

	// We only want to do work if the particle system is active...
	if( !( m_nCorrosiveFlags & CORROSIVEFLAG_DOBOTTOSSWORK ) ) {
		return;
	}

	// Update the position vector of where the emitter should be...
	// Use the primary fire position (which is where the bot is supposed to come from)
	// until the bot is actually spawned, then use the tag position of the spawned bot...
	if( m_nCorrosiveFlags & CORROSIVEFLAG_ALARMBOTSPAWNED ) {
		// Use the position of the bot
		m_vBotTossParticleEpicenter_WS.Set ( m_pAlarmNetSpawnedBot->GetTagPoint( 0 )->v3 );
	} else {
		m_vBotTossParticleEpicenter_WS.Set( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxPrimaryFire ]->m_vFront );
		m_vBotTossParticleEpicenter_WS.Mul( 2.0f );
		m_vBotTossParticleEpicenter_WS.Add( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxPrimaryFire ]->m_vPos );
	}

	// Increment our timers
	fPrevTime = m_fBotTossParticleTime;
	m_fBotTossParticleTime += FLoop_fPreviousLoopSecs;

	// Check to see if we should spawn the emitter
	if( ( fPrevTime < _BOT_TOSS_PARTICLE_EMIT_START_TIME ) && ( m_fBotTossParticleTime >= _BOT_TOSS_PARTICLE_EMIT_START_TIME ) ) {  
		// Spawn the particle emitters and attach it to the spawned bot
		if( m_hBotTossFXParticle != FPARTICLE_INVALID_HANDLE ) {
			m_hBotTossFXEmitter = fparticle_SpawnEmitter( m_hBotTossFXParticle, &m_vBotTossParticleEpicenter_WS.v3, 
														NULL, NULL, m_BotInfo_Corrosive.fBotTossFXIntensity );				
		}
	}

	// Check to see if we should kill the emitter
	if( m_fBotTossParticleTime >= _BOT_TOSS_PARTICLE_EMIT_STOP_TIME ) {  
		if( m_hBotTossFXEmitter != FPARTICLE_INVALID_HANDLE ) {
			fparticle_StopEmitter( m_hBotTossFXEmitter );
			m_hBotTossFXEmitter = FPARTICLE_INVALID_HANDLE;
		}

		// Now that the emitters are gone, no longer any need to run this work routine
		FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_DOBOTTOSSWORK );
	}
}


//////////////////////////////
// EXTERNAL DAMAGE ROUTINES //
//////////////////////////////

// This routine tracks damage loss, and at certain damage milestones
// blows off a limb (if a minimum number of limbs have not already been removed).
void CBotCorrosive::_DamageWork( void ) {

	u32 i, j, nCriticalDamageVSpotBoneIdx;

	if( NormHealth() < m_fSavedNormHealth ) {
		// We have taken damage within the last frame...
		// Run through the damage milestones and see if we crossed a milestone...

		for( i = 0; i < m_nDamageMilestones; i++ ) {
			if( ( m_fSavedNormHealth > m_pafDamageMilestones[ i ] ) && ( NormHealth() <= m_pafDamageMilestones[ i ] ) ) {
				// We crossed a milestone boundary...  Get a limb to blow off
				LimbType_e eLimbToBlowOff = _DamageWork_FindLimbToBlowOff( i );
				if( eLimbToBlowOff == LIMB_TYPE_HEAD_CHUNKS ) {
					// This is the final milestone!  We have reached critical damage...
					// Just blow the Head chunks clean off, and set the critical damage flag!
					m_pPartMgr->SetState_BlowUp( LIMB_TYPE_HEAD_CHUNKS, FALSE );
					
					// Spawn an explosion...
					if( m_BotInfo_Corrosive.hRocketExplosion != FEXPLOSION_INVALID_HANDLE ) {
						FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

						if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {

							// Find a critical damage vspot to spawn an explosion from...
							nCriticalDamageVSpotBoneIdx = -1;
							for( j = 0; j < m_nVSpotCount; j++ ) {
								if( m_paVSpots[ j ].bCriticalDamageOnlySpot ) {
									nCriticalDamageVSpotBoneIdx = j;
									break;
								}
							}
							
							if( nCriticalDamageVSpotBoneIdx != -1 ) {
								FExplosionSpawnParams_t SpawnParams;
								SpawnParams.InitToDefaults();
								SpawnParams.Pos_WS.Set( m_pWorldMesh->GetBoneMtxPalette()[ nCriticalDamageVSpotBoneIdx ]->m_vPos );
								SpawnParams.UnitDir = m_pWorldMesh->GetBoneMtxPalette()[ nCriticalDamageVSpotBoneIdx ]->m_vFront;
								CExplosion2::SpawnExplosion( hSpawner, m_BotInfo_Corrosive.hRocketExplosion, &SpawnParams );
							}
						}
					}

					FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_CRITICALDAMAGEREACHED );
					DEVPRINTF( "Corrosive CRITICAL DAMAGE LEVEL!\n" );
				} else if( eLimbToBlowOff != LIMB_TYPE_COUNT ) {
					// Set this part to dangle (which will cause it to just fall off)
					m_pPartMgr->SetState_Dangle( eLimbToBlowOff, FALSE, m_BotInfo_Gen.fGravity );
				}
			}
		}
	}

	// Check our Vspots and perform our particle system maintenance
	for( i = 0; i < m_nVSpotCount; i++ ) {
		// Only check blasts for spheres that are NOT attached to a bot part, or if they are, that the bot part is still intact!
		if( m_paVSpots[ i ].hVSpotHitFXEmitter != FPARTICLE_INVALID_HANDLE ) {
			m_paVSpots[ i ].fEmitTime += FLoop_fPreviousLoopSecs;

			if( m_paVSpots[ i ].fEmitTime >= m_paVSpots[ i ].fMaxEmitTime ) {
				// Time to shut this bad boy off!
				fparticle_StopEmitter( m_paVSpots[ i ].hVSpotHitFXEmitter );
				m_paVSpots[ i ].hVSpotHitFXEmitter = FPARTICLE_INVALID_HANDLE;
			}
		}
	}

	// Save off our previous normalized heath for next frames comparison
	m_fSavedNormHealth = NormHealth();
}



// This routine will determine if a limb needs to be blown off.
// If a limb does neeed to be blown off, it will pick the least important
// limb left to remove.
CBotCorrosive::LimbType_e CBotCorrosive::_DamageWork_FindLimbToBlowOff( u32 nMilestoneID ) {
	// Determine if we even need to blow a limb off..
	u32 nNonIntactLimbCount = 0;

	// Check to see if this is the final milestone... if so, return the head
	if( nMilestoneID == ( m_nDamageMilestones - 1 ) ) {
		// This is the critical damage milestone... return the head chunks
		return LIMB_TYPE_HEAD_CHUNKS;
	}

	// Otherwise, lets return 
	for( u32 i = 0; i < LIMB_TYPE_COUNT; i++ ) {
		if( m_pPartMgr->GetLimbState( i ) != CBotPartMgr::LIMB_STATE_INTACT ) {
			nNonIntactLimbCount++;
		}
	}

	if( nNonIntactLimbCount > nMilestoneID ) {
		// We already have lost limbs... no need to pile on too early, you know?
		return LIMB_TYPE_COUNT;
	}

	// If we are here, we need to lose a limb...  Look at each one and determine what we should blow off...
	// I am not too happy with the fact that this decision process is currently hardcoded, but decisions
	// need to be made based on whether a limb is partially remaining or not, so it's hardcoded for the time being...
	LimbType_e eRetLimb = LIMB_TYPE_COUNT;
	if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM_UPPER ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		eRetLimb = LIMB_TYPE_RIGHT_ARM_UPPER;
	} else if ( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM_LOWER ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		// Check to see if the Right leg lower is intact...
		if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_LEG_LOWER ) == CBotPartMgr::LIMB_STATE_INTACT ) {
			// The right leg lower is intact, so pick one of the two
			if( fmath_RandomChoice( 2 ) ) {
				eRetLimb = LIMB_TYPE_LEFT_ARM_LOWER;
			} else {
				eRetLimb = LIMB_TYPE_RIGHT_LEG_LOWER;
			}
		} else {
			// The right leg lower is NOT intact... so see if the upper is intact...
			if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_LEG_UPPER ) == CBotPartMgr::LIMB_STATE_INTACT ) {
				// Blow off the Right leg upper...
				eRetLimb = LIMB_TYPE_RIGHT_LEG_UPPER;
			} else {
				// The entire right leg is gone, just blow off the lower left arm
				eRetLimb = LIMB_TYPE_LEFT_ARM_LOWER;
			}
		}
	} else if ( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM_UPPER ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		// The lower limb is not intact, but the upper is, so blow this guy off!
		eRetLimb = LIMB_TYPE_LEFT_ARM_UPPER;
	} else if ( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_LEG_LOWER ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		eRetLimb = LIMB_TYPE_RIGHT_LEG_LOWER;
	} else if ( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_LEG_UPPER ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		eRetLimb = LIMB_TYPE_RIGHT_LEG_UPPER;
	}

	return eRetLimb;
}



void CBotCorrosive::SetNormHealth( f32 fNormHealth ) {
	
	if( ( fNormHealth <= 0.0f ) && !( m_nCorrosiveFlags & CORROSIVEFLAG_CRITICALDAMAGEREACHED ) ) {
		// We are setting a normlaized health of dead, but we blew right through the critical damage
		// Milestone, so set the heath to the critical damage limit, and when _DamageWork next gets called
		// it will do the critical damage work...
		fNormHealth = m_pafDamageMilestones[ m_nDamageMilestones - 1 ];
	}

	CBot::SetNormHealth( fNormHealth );
}


void CBotCorrosive::InflictDamage( CDamageData *pDamageData ) {
	CBot* pWhoHitMe = pDamageData->m_Damager.pBot; // when hit by a possessed bot,
	u32 nVSpotHitIdx = -1;

	if( !pWhoHitMe ) {
		return;
	}

	// Only take damage from glitch if we are in a single player environment
	if( MultiplayerMgr.IsSinglePlayer() && !( pWhoHitMe->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
		return;
	}

	if( m_pDamageCallback ) {
		m_pDamageCallback();
	}

	// Furthermore, corrosive is only damageable in vulnerable zones...
	// First calculations are for impact damage:
	
	if( pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_IMPACT ) {
		nVSpotHitIdx = _ComputeImpactDamage( pDamageData );
	} else if ( pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST ) {
		nVSpotHitIdx = _ComputeVSpotBlastDamage( pDamageData );
	} else {
		// AMBIENT Damage has zero effect on corrosive
		return;
	}

	if( nVSpotHitIdx != -1 ) {
		// A VSpot was hit and damage was dealt, spawn the emitter( if needed ) and zero out the time...
		if( m_paVSpots[ nVSpotHitIdx ].hVSpotHitFXEmitter == FPARTICLE_INVALID_HANDLE ) {
			CFVec3* pEmitterLoc = &m_pWorldMesh->GetBoneMtxPalette()[ m_paVSpots[ nVSpotHitIdx ].nBoneIndex ]->m_vPos.v3;
			CFVec3* pEmitterUnitDir = &m_pWorldMesh->GetBoneMtxPalette()[ m_paVSpots[ nVSpotHitIdx ].nBoneIndex ]->m_vFront.v3;
			m_paVSpots[ nVSpotHitIdx ].hVSpotHitFXEmitter = fparticle_SpawnEmitter( m_hVSpotHitFXParticle, pEmitterLoc, 
														pEmitterUnitDir, NULL, m_paVSpots[ nVSpotHitIdx ].fEmitIntensity );				
			fparticle_SetDirection( m_paVSpots[ nVSpotHitIdx ].hVSpotHitFXEmitter, pEmitterUnitDir );
		}
		m_paVSpots[ nVSpotHitIdx ].fEmitTime = 0.0f;
	}
	CBot::InflictDamage( pDamageData );
}



s32 CBotCorrosive::_ComputeImpactDamage( CDamageData *pDamageData ) {
	CFVec3A vFromImpactPtToVSpot;
	u32 i;
	s32 nHitVSpotIdx = -1;

	// This is an impact damage.  Check to see if the triangle hit was a vulnerable triangle
	BOOL bHitCriticalVSpot = FALSE;
	if( IsDamagedVersion() ) { //only do damage to the damaged version of corrosive.
		if( pDamageData->m_pTriData->eSurfaceType != GCOLL_SURF_TYPE_VULNERABLE ) {
			// Check to see if we are critically damaged
			if( !( m_nCorrosiveFlags & CORROSIVEFLAG_CRITICALDAMAGEREACHED ) ) {
				// A VSpot was not hit, but we are not critically damage, so reduce the amount
				// of damage done down to 10 percent (or whatever _NON_VSPOT_DAMAGE_MULTIPLIER is)
				pDamageData->m_afDeltaHitpoints[ DAMAGE_HITPOINT_TYPE_PROJECTILE ] *= _NON_VSPOT_DAMAGE_MULTIPLIER;
				pDamageData->m_afDeltaHitpoints[ DAMAGE_HITPOINT_TYPE_BLAST ] *= _NON_VSPOT_DAMAGE_MULTIPLIER;
				pDamageData->m_afDeltaHitpoints[ DAMAGE_HITPOINT_TYPE_FLAME ] *= _NON_VSPOT_DAMAGE_MULTIPLIER;
			}
		} else {
			// A VSpot was hit. We need to figure out what vspot was hit, and also
			// make sure that if we are critically damaged, that
			// that only damage done within a critical damage VSPOT gets applied.  All other body areas
			// (Vspot or not) are not valid for damage infliction.
			for( i = 0; i < m_nVSpotCount; i++ ) {
				if( !!( m_nCorrosiveFlags & CORROSIVEFLAG_CRITICALDAMAGEREACHED ) == !!m_paVSpots[ i ].bCriticalDamageOnlySpot ) {
					// This is a critical damage VSpot and we are critically damaged, or this is a non critical damage vspot, and we are not
					// critically damaged.  Calculate the distance from the impact point to the VSpot
					vFromImpactPtToVSpot.Sub( m_pWorldMesh->GetBoneMtxPalette()[ m_paVSpots[ i ].nBoneIndex ]->m_vPos, pDamageData->m_ImpactPoint_WS );
					f32 fTempDist2 = vFromImpactPtToVSpot.MagSq();
					if( fTempDist2 <= ( m_paVSpots[ i ].fRadius * m_paVSpots[ i ].fRadius ) ) {
						// Our impact point is inside of this VSpot Region
						nHitVSpotIdx = i;
						if( m_nCorrosiveFlags & CORROSIVEFLAG_CRITICALDAMAGEREACHED ) {
							bHitCriticalVSpot = TRUE;
						}
						break;
					}
				}
			}
		}
	}

	if( !IsDamagedVersion() || ( !bHitCriticalVSpot && ( m_nCorrosiveFlags & CORROSIVEFLAG_CRITICALDAMAGEREACHED ) ) ) {
		// Zero out any damage done. 
		// We are either:
		//  a) the non damaged version of corrosive, or
		//  b) critically damaged but took no damage in a critical Damage VSpot
		pDamageData->m_afDeltaHitpoints[ DAMAGE_HITPOINT_TYPE_PROJECTILE ] = 0.0f;
		pDamageData->m_afDeltaHitpoints[ DAMAGE_HITPOINT_TYPE_BLAST ] = 0.0f;
		pDamageData->m_afDeltaHitpoints[ DAMAGE_HITPOINT_TYPE_FLAME ] = 0.0f;
	}

	return nHitVSpotIdx;
}


// This checks to see if an impact point is in a VSpot
BOOL CBotCorrosive::ImpactPointInVSpot( CFVec3A *pImpactPt_WS ) {
	u32 i;
	CFVec3A vFromImpactPtToVSpot;
	for( i = 0; i < m_nVSpotCount; i++ ) {
		// Only check blasts for spheres that are NOT attached to a bot part, or if they are, that the bot part is still intact!
		if( ( m_paVSpots[ i ].nLimbTypeID == -1 ) || ( m_pPartMgr && ( m_pPartMgr->GetLimbState( m_paVSpots[ i ].nLimbTypeID ) == CBotPartMgr::LIMB_STATE_INTACT ) ) ) {
			// Furthermore, if we are critically damamged, only check critical damage vspots
			if( ( m_nCorrosiveFlags & CORROSIVEFLAG_CRITICALDAMAGEREACHED ) && !m_paVSpots[ i ].bCriticalDamageOnlySpot ) {
				// We have reached critical damage, but this VSpot is NOT a critical damage VSpot, so continue iterating through other VSpots
				continue;
			}

			// If we are here, we have a VSpot which is valid to take damage
			// Calculate the distance to the Vspot
			vFromImpactPtToVSpot.Sub( m_pWorldMesh->GetBoneMtxPalette()[ m_paVSpots[ i ].nBoneIndex ]->m_vPos, *pImpactPt_WS );

			f32 fTempDist2 = vFromImpactPtToVSpot.MagSq();

			// Check to see if we are within the radius of the blast...
			f32 fDeltaDist2 = fTempDist2 - ( m_paVSpots[ i ].fRadius * m_paVSpots[ i ].fRadius );
			if( fDeltaDist2 <= ( m_paVSpots[ i ].fRadius * m_paVSpots[ i ].fRadius ) ) {
				//This blast point is within this VSpot... 
				return TRUE;
			}
		}
	}

	return FALSE;
}


s32 CBotCorrosive::_ComputeVSpotBlastDamage( CDamageData *pDamageData ) {
	// Run through the VSpots and see if any of these vspots are within the blast radius.
	// If any VSpots are within the blast radius, recompute the damage done based on the
	// Vspot location.
	s32 nClosestIndex = -1;
	u32 i;
	f32 fClosestIndexDist2;
	CFVec3A vFromEpicenterToVSpot;

	BOOL bBuiltTrackerSkipList = FALSE;

	for( i = 0; i < m_nVSpotCount; i++ ) {
		// Only check blasts for spheres that are NOT attached to a bot part, or if they are, that the bot part is still intact!
		if( ( m_paVSpots[ i ].nLimbTypeID == -1 ) || ( m_pPartMgr->GetLimbState( m_paVSpots[ i ].nLimbTypeID ) == CBotPartMgr::LIMB_STATE_INTACT ) ) {

			// Furthermore, check to make sure we are checking the proper critical damage vspots
			if( !!( m_nCorrosiveFlags & CORROSIVEFLAG_CRITICALDAMAGEREACHED ) != !!m_paVSpots[ i ].bCriticalDamageOnlySpot ) {
				// We are critically damaged and this is a non critical damage vspot, or
				// we are not critically damaged and this is a critical damage vspot.  Either way, skip this one.
				continue;
			}
			// If we are here, we have a VSpot which is valid to take damage
			// Calculate the distance to the Vspot
			vFromEpicenterToVSpot.Sub( m_pWorldMesh->GetBoneMtxPalette()[ m_paVSpots[ i ].nBoneIndex ]->m_vPos, *pDamageData->m_pEpicenter_WS );

			f32 fTempDist2 = vFromEpicenterToVSpot.MagSq();

			// Check to see if we are within the radius of the blast...
			f32 fDeltaDist2 = fTempDist2 - ( m_paVSpots[ i ].fRadius * m_paVSpots[ i ].fRadius );
			if( fDeltaDist2 <= 0.0f ) {
				//This blast point is within this VSpot... Break out and do damage!
				nClosestIndex = i;
				fClosestIndexDist2 = 0.0f; // Set to point within blast radius
				break;
			}
			else if( m_paVSpots[ i ].bAcceptSplashDamage && ( fDeltaDist2 < ( pDamageData->m_fLargestOuterRadius * pDamageData->m_fLargestOuterRadius ) ) ) {
				// This point is not in this VSpot radius, but it IS within blast range.
				// compare how close it is to a previously saved epicenter distance...
				BOOL bCheckBlastLOS = FALSE;
				if( nClosestIndex != -1 ) {
					// We have a previously saved distance... Compare the distance squared
					if( fDeltaDist2 < fClosestIndexDist2 ) {
						// We have found a closer point, but we have to see if it's obstructed
						bCheckBlastLOS = TRUE;
					}
				} else {
					// We don't have a previous distance saved...  Check obstruction
					bCheckBlastLOS = TRUE;
				}

				if( bCheckBlastLOS ) {
					// We want to check this point out and determine if it has an unobstructed blast LOS to the VSpot

					if( !bBuiltTrackerSkipList ) {
						// Build the tracker skip list...

						FWorld_nTrackerSkipListCount = 0;
						AppendTrackerSkipList();

						if( pDamageData->m_Damager.pEntity ) {
							pDamageData->m_Damager.pEntity->AppendTrackerSkipList();
						}
						if( pDamageData->m_Damager.pBot && (pDamageData->m_Damager.pBot != pDamageData->m_Damager.pEntity) ) {
							pDamageData->m_Damager.pBot->AppendTrackerSkipList();
						}
						if( pDamageData->m_Damager.pWeapon && (pDamageData->m_Damager.pWeapon != pDamageData->m_Damager.pEntity) && 
							( (CEntity *) pDamageData->m_Damager.pWeapon != ( CEntity * )pDamageData->m_Damager.pBot ) ) {
							pDamageData->m_Damager.pWeapon->AppendTrackerSkipList();
						}

						bBuiltTrackerSkipList = TRUE;
					}

					BOOL bLOSObstructed = fworld_IsLineOfSightObstructed(
												pDamageData->m_pEpicenter_WS,
												&m_pWorldMesh->GetBoneMtxPalette()[ m_paVSpots[ i ].nBoneIndex ]->m_vPos,
												FWorld_nTrackerSkipListCount,
												FWorld_apTrackerSkipList,
												NULL,
												-1,
												FCOLL_MASK_OBSTRUCT_SPLASH_DAMAGE,
												FCOLL_LOD_HIGHEST
											);
					if( !bLOSObstructed ) {
						// Record off this VSpot index as the current index to use for damage calcuations
						nClosestIndex = i;
						fClosestIndexDist2 = fDeltaDist2;
					}
				} // if this blast ray is not obscured by any blast stopping geo
			} // if the blast epicenter is outside the Vspot radius, but large enough to damage the VSpot, and the VSpot accepts splash damage
		} // if this VSpot is a valid vspot to check for damage
	} // For each VSpot we have...

	// Now, check to see if we have a VSpot that was splashed!
	if( nClosestIndex != -1 ) {
		f32 fDistance = 0.0f;
		if( fClosestIndexDist2 > 0.00001f ) {
			fDistance = fmath_Sqrt( fClosestIndexDist2 );
		}

		// Now, recompute the amount of damage that should be applied and override
		// the amount saved in the damage form...
		f32 fIntensity = CDamage::ComputeRadialIntensity_Hitpoint( fDistance, pDamageData->m_pDamageProfile, pDamageData->m_fDamageFormOrigNormIntensity );
		for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
			pDamageData->m_afDeltaHitpoints[i] = fIntensity * pDamageData->m_pDamageProfile->m_afUnitHitpoints[i];
		}
	} else {
		// This blast did NOT hit a VSpot.  Zero out all damage done!
		for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
			pDamageData->m_afDeltaHitpoints[i] = 0.0f;
		}
	}

	return nClosestIndex;
}


void CBotCorrosive::NotifyFootDown( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitMag ) {
	CBot::NotifyFootDown( bLeftFoot, bRightFoot, fUnitMag );

	// We want to get a damage profile and apply rumble here...
	if( fUnitMag > 0.0f ) {
		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile	= m_BotInfo_Corrosive.pDamageProfileFootStepRumble;
			if( bLeftFoot ) {
				pDamageForm->m_Epicenter_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLToe ]->m_vPos;
			} else if ( bRightFoot ) {
				pDamageForm->m_Epicenter_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRToe ]->m_vPos;
			} else {
				// We should never get here, but I'll put this here to just be safe!
				pDamageForm->m_Epicenter_WS		= m_MountPos_WS;
			}
			pDamageForm->m_Damager.pBot		= this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_fNormIntensity	= 1.0f;//fmath_Sqrt( fUnitMag );

			// submitting footdown damage form for titan mag = 
			CDamage::SubmitDamageForm( pDamageForm );
		}
	}
}



////////////////////////////
// FEET INFLICTED DAMAGE

// This function will compute the capsule position of the feet prior to the feet moving.
// Later, we will take the new position of the feet, and issue a collsion check.
void CBotCorrosive::_PrepareFootCollisionCapsules( CFCapsule *pCollCapsuleL, CFCapsule *pCollCapsuleR ){

	CFVec3A vTmp;

	//
	// LEFT FOOT
	//
	// Calculate the toe capsule endpoint (projected forward)
	vTmp.Mul( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLToe ]->m_vFront, _FOOT_COLLISION_TOE_PROJECTION_DIST );
	vTmp.Add( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLToe ]->m_vPos );
	pCollCapsuleL->m_vPoint1 = vTmp;
	// Calculate the heel capsule endpoint (projected backward)
	vTmp.Mul( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLHeel ]->m_vFront, _FOOT_COLLISION_HEEL_PROJECTION_DIST );
	vTmp.Add( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLHeel ]->m_vPos );
	pCollCapsuleL->m_vPoint2 = vTmp;
	pCollCapsuleL->m_fRadius = _FOOT_COLLISION_CAPSULE_RADIUS;
	//fdraw_DevCapsule( pCollCapsuleL );

	//
	// RIGHT FOOT
	//
	// Calculate the toe capsule endpoint (projected forward)
	vTmp.Mul( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRToe ]->m_vFront, _FOOT_COLLISION_TOE_PROJECTION_DIST );
	vTmp.Add( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRToe ]->m_vPos );
	pCollCapsuleR->m_vPoint1 = vTmp;
	// Calculate the heel capsule endpoint (projected backward)
	vTmp.Mul( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRHeel ]->m_vFront, _FOOT_COLLISION_HEEL_PROJECTION_DIST );
	vTmp.Add( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRHeel ]->m_vPos );
	pCollCapsuleR->m_vPoint2 = vTmp;
	pCollCapsuleR->m_fRadius = _FOOT_COLLISION_CAPSULE_RADIUS;
	//fdraw_DevCapsule( pCollCapsuleR );
}


#if _DRAW_FOOTCOLLISION_DEBUG_INFO == 1
static u32 _nNextCollisionSlot = 0;
static u32 _nNumValidCollisions = 0;
static CFCapsule _aCollCapsule[5];
static CFVec3A _aImpactPt[5];
static CFVec3A _aImpactCenterLinePt[5];
#endif

// This function will perform the actual collision checks.  The feet have already been moved,
// and the Capsules are loaded with the previous position of the feet.
// Here, we will calculate how far the capsules have moved, and then issue the collision checks.
void CBotCorrosive::_CheckForFootCollisions( CFVec3A *pPrevLFootPos_WS, CFVec3A *pPrevRFootPos_WS ){

	CFCapsule CollCapsuleL, CollCapsuleR;
	CFVec3A vDeltaDist, vFootDownVec;

	_PrepareFootCollisionCapsules( &CollCapsuleL, &CollCapsuleR );

	// Now, because we are going to project this movement forward, the
	// calculated capsule position points represent the final position, lets
	// back-project the capsule position points based on the difference between
	// the new and previous foot positions.

	//
	// LEFT FOOT
	//
	vDeltaDist = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLFoot ]->m_vPos;
	vDeltaDist.Sub( *pPrevLFootPos_WS );
	// Only calculate damage collisions of the foot is moving down
	if( vDeltaDist.y < -1.0f ) {  // only if the foot has moved down more than 1 foot this frame
		CollCapsuleL.m_vPoint1.Sub( vDeltaDist );
		CollCapsuleL.m_vPoint2.Sub( vDeltaDist );
		vFootDownVec.Set( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLFoot ]->m_vUp );
		vFootDownVec.Negate();

		// Additionally, we need to determine if the foot stomp is in action... if it is, 
		// we also want to determine the sweet spot for using the increased foot stomp
		// damage profile.
		_CheckFootCollision( &CollCapsuleL, &vDeltaDist, &vFootDownVec, 
							( ( m_nCorrosiveFlags & CORROSIVEFLAG_USEFOOTSTOMPDAMAGEPROFILE ) ? TRUE : FALSE ),
							&m_nLeftFootDamagedEntityCount, m_apLeftFootDamagedEntity );
	}

	//
	// RIGHT FOOT
	//
	vDeltaDist = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRFoot ]->m_vPos;
	vDeltaDist.Sub( *pPrevRFootPos_WS );
	// Only calculate damage collisions of the foot is moving down
	if( vDeltaDist.y < -1.0f ) {  // only if the foot has moved down more than 1 foot this frame
		CollCapsuleR.m_vPoint1.Sub( vDeltaDist );
		CollCapsuleR.m_vPoint2.Sub( vDeltaDist );
		vFootDownVec.Set( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxRFoot ]->m_vUp );
		vFootDownVec.Negate();
		_CheckFootCollision( &CollCapsuleR, &vDeltaDist, &vFootDownVec, FALSE, &m_nRightFootDamagedEntityCount, m_apRightFootDamagedEntity );
	}

#if _DRAW_FOOTCOLLISION_DEBUG_INFO == 1
	// Now draw the debug shmegma
	for( u32 i=0; i<_nNumValidCollisions; i++ ) {
		fdraw_DevCapsule( &_aCollCapsule[ i ] );
		fdraw_DevSphere( &_aImpactPt[ i ].v3, 2.0f,  &FColor_MotifRed );
		fdraw_DevSphere( &_aImpactCenterLinePt[ i ].v3, 2.0f, &FColor_MotifGreen );
	}
#endif
}

void CBotCorrosive::_CheckFootCollision( CFCapsule *pCollCapsulePrev, CFVec3A *pvDeltaDist, CFVec3A *pvFootDownVec, BOOL bFootStomping,
										 u8 *pnEntitiesInCapsuleArray, CEntity** papEntityInCapsuleArray ) {

	u32 uIndex, i;
	FCollImpact_t *pImpact;
	CFVec3A FireUnitDir_WS, vProjectedPoint1_WS, vProjectedPoint2_WS, vTravelDist;
	f32 fMag2;

	CEntity* apCollidedEntities[ HIT_ENTITY_MAX_COUNT ];
	s32 nCollidedEntities = 0;

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	// Fill out the CollData
	CFCollData CollData;
	CollData.pMovement = pvDeltaDist;
	CollData.nCollMask = (m_nPossessionPlayerIndex < 0) ? FCOLL_MASK_COLLIDE_WITH_NPCS : FCOLL_MASK_COLLIDE_WITH_PLAYER;
	CollData.nFlags = ( FCOLL_DATA_IGNORE_BACKSIDE | FCOLL_DATA_IGNORE_WORLD );
	if( !bFootStomping ) {
		CollData.nFlags |= FCOLL_DATA_IGNORE_STATIC_OBJECTS; 
	}

	CollData.nStopOnFirstOfCollMask = 0; // We are interested in ALL collisions
	CollData.pCallback = NULL;//_FootCollisionCallback;
	CollData.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	CollData.ppTrackerSkipList = FWorld_apTrackerSkipList;
	CollData.nTrackerUserTypeBitsMask = ENTITY_BIT_BOT;//~nUnwantedBits;
	if( bFootStomping ) {
		CollData.nTrackerUserTypeBitsMask |= ENTITY_BIT_BOOMER;
	}
	CollData.pLocationHint = m_pWorldMesh;

	// Now, issue a capsule collision call
	FColl_nImpactCount = 0;
	fcoll_Clear();
	fcoll_Check( &CollData, pCollCapsulePrev );

	// Check to see if we had any collsions
	if( FColl_nImpactCount > 0 ) {
		if( bFootStomping ) {
			DEVPRINTF( "FOOT STOMPING!\n");
		}

		// We have collisions -- ensure that they are what we want them to be...

		// First, calculate our unit impact direction becuase we may want to use
		// it more than once, and it would be most beneficial to only calculate
		// it once.
		FireUnitDir_WS = *pvDeltaDist;

		fMag2 = FireUnitDir_WS.MagSq();
		if( fMag2 >= 0.000001f ) {
			FireUnitDir_WS.Mul( fmath_InvSqrt( fMag2 ) );
		} else {
			//FireUnitDir_WS = -CFVec3A::m_UnitAxisY;
			FireUnitDir_WS.Set( 0.0f, -1.0f, 0.0f );
		}

		// Pre-calculate some variables here so they aren't re-calculated
		// for every single collision.
		CFVec3A vCenterLine, vUnitCenterLine;
		vCenterLine.Sub( pCollCapsulePrev->m_vPoint2, pCollCapsulePrev->m_vPoint1 );
		f32 fCapsuleMagnitude = vUnitCenterLine.UnitAndMag( vCenterLine ); 

		// Step through all impacts, looking for bots (or static objects)
		for( uIndex = 0; uIndex < FColl_nImpactCount; uIndex++ ) {

			pImpact = &FColl_aImpactBuf[uIndex];
			CFWorldTracker *pHitTracker = (CFWorldTracker *)pImpact->pTag;
			// Bail out if we have a null tracker
			if( !pHitTracker ) {
				continue;
			}

			// NOTE : I don't belive these checks are really necessary because
			// of the BitMasks I set above.  However, I'll leave them here
			// for the short time until I convince myself that I really am
			// culling out all the bot appropriately through the bitmasks
			// above.

			// Ensure we have an entity
			if( pHitTracker->m_nUser != MESHTYPES_ENTITY ) {
				continue;
			}
			CEntity *pEntity = (CEntity *)pHitTracker->m_pUser;

			// Check to see if for some crazy reason, this is ourself
			if( pEntity == this ) {
				continue;
			}

			// Add this guy to the collided entity list count...
			if( nCollidedEntities >= HIT_ENTITY_MAX_COUNT ) {
				// Bail out... we couldn't add any more entities even if we wanted to!
				break;
			}
			//apCollidedEntities[ nCollidedEntities ] = pBot;
			apCollidedEntities[ nCollidedEntities ] = pEntity;
			nCollidedEntities++;

			// Apply damage to this bot if he is not already in the previous collidedlist...
			BOOL bAlreadyInList = FALSE;

			// RAFHACK -- COMMENT OUT BECAUSE WE WANT QUICK DEATH!
			/*
			for( i = 0; i < *pnEntitiesInCapsuleArray; i++ ) {
				if( papEntityInCapsuleArray[ i ] == pEntity ) {
					//this guy is already in the list...
					bAlreadyInList = TRUE;
					break;
				}
			}
			*/

			if( bAlreadyInList ) {
				// This entity is already in the damage capsule... Don't apply any more damage...
				// What we could do right here is apply an impulse instead!
				continue;
			}

			// Now, we want to see where the impact occured, and if it's in the general
			// downward foot location zone...
			// First, calculate the position of the capsule 
			vProjectedPoint1_WS.Set( pCollCapsulePrev->m_vPoint1 );
			vProjectedPoint2_WS.Set( pCollCapsulePrev->m_vPoint2 );
			if( pImpact->fUnitImpactTime > 0.0f ) {
				vTravelDist.Set( *pvDeltaDist );
				vTravelDist.Mul( pImpact->fUnitImpactTime );
				vProjectedPoint1_WS.Add( vTravelDist );
				vProjectedPoint2_WS.Add( vTravelDist );
			}

			// Now, determine the impact point along the centerline of the capsule
			// Take the impact position and dot it with the unit centerline vector.
			// This will yield the distance along the vector from projected point1
			// to projected point 2 that this impact took place...
			CFVec3A vPoint1ToImpact;
			vPoint1ToImpact.Sub( pImpact->ImpactPoint, vProjectedPoint1_WS );
			f32 fDistFromProjectedPoint1 = vUnitCenterLine.Dot( vPoint1ToImpact );
			// Clamp it to a value between point 1 and point2.
			FMATH_CLAMP( fDistFromProjectedPoint1, 0.0f, fCapsuleMagnitude );
			CFVec3A vCenterLineImpactPoint;
			vCenterLineImpactPoint.Set( vUnitCenterLine );
			vCenterLineImpactPoint.Mul( fDistFromProjectedPoint1 );
			vCenterLineImpactPoint.Add( vProjectedPoint1_WS );

			CFVec3A vCenterLineImpactToImpactPt;
			vCenterLineImpactToImpactPt.Sub( pImpact->ImpactPoint, vCenterLineImpactPoint );
			vCenterLineImpactToImpactPt.Unitize();

#if _DRAW_FOOTCOLLISION_DEBUG_INFO == 1
			_aCollCapsule[ _nNextCollisionSlot ].m_vPoint1 = vProjectedPoint1_WS;
			_aCollCapsule[ _nNextCollisionSlot ].m_vPoint2 = vProjectedPoint2_WS;
			_aCollCapsule[ _nNextCollisionSlot ].m_fRadius = pCollCapsulePrev->m_fRadius;
			_aImpactPt[ _nNextCollisionSlot ] = pImpact->ImpactPoint;
			_aImpactCenterLinePt[ _nNextCollisionSlot ] = vCenterLineImpactPoint;
			_nNextCollisionSlot++;
			_nNextCollisionSlot = _nNextCollisionSlot % 5;
			_nNumValidCollisions++;
			if( _nNumValidCollisions > 5 ) {
				_nNumValidCollisions = 5;
			}
#endif			

			// RAFNOTE -- The following is a hack to reliably destroy boomer objects until all the 
			// bugs in the capsule collision system get worked out:
			BOOL bSkipDotProductCheck = FALSE;
			if( pEntity->TypeBits() & ENTITY_BIT_BOOMER ) {
				// This is a destructable object... Hammer it!
				bSkipDotProductCheck = TRUE;
			}

			// Now we have a centerline impact point.  Lets test that against
			// the foot down vec to see if we are within the confines of the
			// bottom of corrosives foot
			if( !bSkipDotProductCheck ) {
				f32 fDot = pvFootDownVec->Dot( vCenterLineImpactToImpactPt );
				if( fDot < 0.707f ) { // += 45 degrees
					// We are outside of the downward cone, so don't do any damage...
					continue;
				}
			}

			// If you get stepped on, you DIE.  Period.
			// NOTE: Check the health first, because if this entity has a goodie bag, it might spawn multiple times
			if( pEntity->NormHealth() > 0.0f ) {
				pEntity->Die();
				
				if( ( m_nPossessionPlayerIndex >= 0 ) && ( pEntity->TypeBits() & ENTITY_BIT_BOT ) ) {
					// The player is in control of Corrosive, and the entity killed was a bot.
					// Credit the player for the kill
					Player_aPlayer[ m_nPossessionPlayerIndex ].CreditKill( ( ( CBot* )pEntity )->m_nPossessionPlayerIndex, pEntity );
				}
			}
		}
	}

	// Now, update the Damage Array with the bots we collided with this frame
	// Next frame, we will use the array again to compare if bots are still
	// in the damage capsule, or if new ones have entered.

	*pnEntitiesInCapsuleArray = nCollidedEntities;
	for( i = 0; i < *pnEntitiesInCapsuleArray; i++ ) {
		papEntityInCapsuleArray[ i ] = apCollidedEntities[ i ];
	}
}

/*
BOOL CBotCorrosive::_FootCollisionCallback( CFWorldTracker *pTracker ) {
// RAFHACK -- This routine is not really necessary I think because I have
// culled out the stuff I don't want in my bit flags above...  I'll leave it
// in until I am sure I am 'doing the right thing'....
	for( u32 i=0; i < FWorld_nTrackerSkipListCount; i++ ) {
		if( FWorld_apTrackerSkipList[i] == pTracker ) {
			return FALSE;
		}
	}

	//DEVPRINTF( "Got a collision!\n" );
	return TRUE;
}
*/
///////////////////////////
// WEAPONS - ROCKET LAUNCH

void CBotCorrosive::_BuildRocketSkipList( CEProj *pProj ) {
	CBotCorrosive *pBot = (CBotCorrosive*)pProj->GetDamagerBot();
	pBot->AppendTrackerSkipList();
}



void CBotCorrosive::_FireRocket( void ) {
	// Get a free projectile...

	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( CWeapon::m_ahProjPool[CWeapon::PROJ_POOL_TYPE_SWARMER_ROCKET] );
	if( pProj == NULL ) {
		// No more projectiles...
		return;
	}

	// Figure out where we are targeting
	CFVec3A vMuzzlePt;
	CFVec3A vFireUnitDir;

	vMuzzlePt = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPrimaryFire]->m_vPos;

	// We are going to use differrent targeting methods based on who is in control
	// of Corrosive.
	if( m_nPossessionPlayerIndex >= 0 ) {
		// The player is in control of corrosive.  Take a vector from the muzzlepoint
		// to where we are targeted.
		vFireUnitDir.Sub( m_TargetedPoint_WS, vMuzzlePt );
		f32 fMag2 = vFireUnitDir.MagSq();
		if( fMag2 < 0.001f ) {
			vFireUnitDir = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPrimaryFire]->m_vFront;
		} else {
			vFireUnitDir.Mul( fmath_InvSqrt( fMag2 ) );
		}
	}
	else {
		// The AI is in control of Corrosive.  Always have the rockets come straight out
		// of his chest, and they will home in on the player...
		vFireUnitDir = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPrimaryFire]->m_vFront;
	}

	_InitProjectile( pProj, &vMuzzlePt, &vFireUnitDir, FALSE );

	if( m_nPossessionPlayerIndex >= 0 ) {
		
		// If a player is in control of corrosive, only have corrosive target predators and snarks
		m_pTargetedEntity = NULL;
		if( m_pTargetedMesh ) {
			CEntity *pTargetedEntity = (CEntity *)m_pTargetedMesh->m_pUser;
			if( pTargetedEntity && ( ( pTargetedEntity->TypeBits() & ENTITY_BIT_BOTPRED ) || ( pTargetedEntity->TypeBits() & ENTITY_BIT_BOTSNARQ ) ||
				( pTargetedEntity->TypeBits() & ENTITY_BIT_BOTJUMPER ) ) ) {
				m_pTargetedEntity = pTargetedEntity;
			}
		}
	}

	pProj->SetTargetedEntity( m_pTargetedEntity );
	pProj->Launch();

	// Flash of light
	CFVec3A vFlashPos = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxPrimaryFire ]->m_vFront;
	vFlashPos.Mul( 2.0f );
	vFlashPos.Add( m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxPrimaryFire ]->m_vPos );

	CMuzzleFlash::AddFlash_CardPosterXY(
				CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
				vFlashPos,
				2.0f,
				0.75f
				);

	if (m_bForce2D_RoarRocket)
	{
		PlaySound( m_BotInfo_Corrosive.pSoundGroupRocketLauncher, 1.0f, 1.0f, -1.0f, NULL, TRUE ); // We need to hear the rocket launch!
	}
	else
	{
		PlaySound( m_BotInfo_Corrosive.pSoundGroupRocketLauncher );
	}
	if( m_nPossessionPlayerIndex >= 0 ) {
		fforce_Kill( &m_hForce );
		fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );
	}
}



void CBotCorrosive::_InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight ) {

	CFMtx43A ProjMtx;

	ProjMtx.UnitMtxFromUnitVec( pUnitDir_WS );
	ProjMtx.m_vPos = *pPos_WS;

	pProj->Init();
	pProj->SetDamager( NULL, this );
	if( m_nPossessionPlayerIndex >= 0 ) {
		pProj->SetDamageProfile( m_BotInfo_Corrosive.pDamageProfileRocketPlayer );
	} else {
		pProj->SetDamageProfile( m_BotInfo_Corrosive.pDamageProfileRocketAI );
	}
	pProj->SetSkipListCallback( _BuildRocketSkipList );
	pProj->SetMaxDistCanTravel( m_BotInfo_Corrosive.fRocketRange );
	pProj->SetLinSpeed( m_BotInfo_Corrosive.fRocketSpeed );
	pProj->SetLinUnitDir_WS( pUnitDir_WS );
	pProj->SmokeTrailOn( &m_SmokeTrailAttrib );
	pProj->SetExplosionGroup( m_BotInfo_Corrosive.hRocketExplosion );

	BOOL bMultiFire = TRUE;
	if( bMultiFire ) {
		CEProjExt::CEProj_Swarmer_Params_t SwarmerParams;
		SwarmerParams.pMervStaticParams = &m_BotInfo_Corrosive.MervStaticParams;
		SwarmerParams.fSwarmAmplitude = 0.0f;
		SwarmerParams.fSwarmingBlendInInvDist = 0.0f;
		SwarmerParams.fSwarmingSpiralSpeed = 0.0f;
		pProj->SetSwarmerParams( &SwarmerParams );
	}

	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, m_BotInfo_Corrosive.fProjectileMeshScale );

	CFWorldLightItem *pWorldLightItem = CLightPool::GetFromFreePool();
	if( pWorldLightItem ) {
		pWorldLightItem->m_Light.InitOmniLight( &ProjMtx.m_vPos.v3, m_BotInfo_Corrosive.fLightRadius );

		FMATH_SETBITMASK( pWorldLightItem->m_Light.m_nFlags, FLIGHT_FLAG_PER_PIXEL | FLIGHT_FLAG_CORONA | FLIGHT_FLAG_CORONA_WORLDSPACE );

		if( bNoDynamicLight ) {
			FMATH_SETBITMASK( pWorldLightItem->m_Light.m_nFlags, FLIGHT_FLAG_CORONA_ONLY );
		}

		pWorldLightItem->m_Light.m_pCoronaTex = &m_CoronaTexInst;
		pWorldLightItem->m_Light.m_fCoronaScale = m_BotInfo_Corrosive.fCoronaScale;
		pWorldLightItem->m_Light.m_fDeltaScale = m_BotInfo_Corrosive.fCoronaUnitScreenspaceScale;

		pWorldLightItem->m_Light.SetColor( m_BotInfo_Corrosive.fCoronaColorRed, m_BotInfo_Corrosive.fCoronaColorGreen, m_BotInfo_Corrosive.fCoronaColorBlue );
		pWorldLightItem->m_Light.SetMotif( fcolor_GetRandomMotif(FCOLORMOTIF_ROCKET0) );
		pProj->SetAttachedLight( pWorldLightItem );
	}
}

///////////////////////////////
// WEAPONS - FIST SMASH / SWAT

void CBotCorrosive::_HandleFistSmashAnimation( void ) {
	f32 fTime = 0.0f, fPrevTime, fControlVal;

	fPrevTime = GetUnitTime( ANIMTAP_FIST_SMASH_UPPER );
	if( !DeltaTime( ANIMTAP_FIST_SMASH_UPPER, FLoop_fPreviousLoopSecs, TRUE ) ) {
		// We are still smashing
		fControlVal = 1.0f; // Default to full smash blend
		fTime = GetTime( ANIMTAP_FIST_SMASH_UPPER );
		if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { // Quarter of a second blend in time...
			// Blend in the fist smash...
			fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		} else if( fTime > ( GetTotalTime( ANIMTAP_FIST_SMASH_UPPER ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
			// Check to see if we should be blending out...
			fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( ANIMTAP_FIST_SMASH_UPPER ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		}

		// Don't allow ourself to rotate!
		//FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );

		if( m_nPossessionPlayerIndex == -1 ) {
			// If the ai is in control, have it re-orient it's groin for maximum
			// fist accuracy.  If the player is in control, don't do it.
			// Reorient our groin...
			f32 fReorientTime = fTime;
			FMATH_CLAMPMAX( fReorientTime, _REORIENT_LEGS_TIME );
			FMATH_BIPOLAR_CLAMPMAX( m_fLegsYaw_MS, ( _REORIENT_LEGS_TIME - fReorientTime ) * m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold );
		}

		// The AI Version of corrosive must stop to issue his fist attack...
		if( m_nPossessionPlayerIndex == -1 ) {
			FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_NOAIXLAT );
		}

		_HandleHandAttackInflictDamageSweetRange( fPrevTime, fTime, 0.75f, 1.25f );

		// See if we need to play the crashing sounds...
		if( ( m_nCorrosiveFlags & CORROSIVEFLAG_SMASHHITTERRAIN ) && !( m_nCorrosiveFlags & CORROSIVEFLAG_CRASHEFFECTSSPAWNED ) ) {
			_HandleCrashEffects( fTime, fTime, &m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxFistDamageSphere ]->m_vPos ); // Play it immediately
		}
	}
	else {
		// We are done with our fist smash
		fControlVal = 0.0f;
		m_eLastBehaviorState = m_eBehaviorState;
		m_eBehaviorState = BEHAVIOR_STATE_NONE;

		FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_NOAIXLAT );
	}
	SetControlValue( ANIMCONTROL_FIST_SMASH_UPPER, fControlVal);
	SetControlValue( ANIMCONTROL_FIST_SMASH_LOWER, fControlVal);
	//ftext_DebugPrintf( 0.4f, 0.33f, "~w1time:  %0.2f", fTime );
}


void CBotCorrosive::_HandleSwatAnimation( void ) {
	f32 fTime = 0.0f, fPrevTime, fControlVal;

	fPrevTime = GetUnitTime( ANIMTAP_SWAT );
	if( !DeltaTime( ANIMTAP_SWAT, FLoop_fPreviousLoopSecs, TRUE ) ) {
		// We are still swatting
		fControlVal = 1.0f; //default to full swat
		fTime = GetTime( ANIMTAP_SWAT );
		if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
			// Blend in the swat...
			fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		} else if( fTime > ( GetTotalTime( ANIMTAP_SWAT ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
			// Check to see if we should be blending out...
			fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( ANIMTAP_SWAT ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		}

		// Don't allow ourself to rotate!
		//FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );

		// Keep ourself stationary
		
		// Gradually slow down our velocity over _ZERO_VELOCITY_TIME
		f32 fVelocityTime = fTime;
		FMATH_CLAMPMAX( fVelocityTime, _ZERO_VELOCITY_TIME );
		m_Velocity_WS.Mul( ( _ZERO_VELOCITY_TIME - fVelocityTime ) * _OO_ZERO_VELOCITY_TIME );
		VelocityHasChanged();

		_HandleHandAttackInflictDamageSweetRange( fPrevTime, fTime, 1.0f, 1.5f );
	}
	else {
		// We are done with our swat
		fControlVal = 0.0f;
		m_eBehaviorState = BEHAVIOR_STATE_NONE;

		// Re-enable rotation
		//FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );
	}
	SetControlValue( ANIMCONTROL_SWAT, fControlVal);
	//ftext_DebugPrintf( 0.4f, 0.33f, "~w1time:  %0.2f", fTime );
}



void CBotCorrosive::_InitStartHandAttackData( void ) {
	m_nHitEntityCount = 0;
	FMATH_CLEARBITMASK( m_nCorrosiveFlags, ( CORROSIVEFLAG_INFLICTHANDATTACKDAMAGE | CORROSIVEFLAG_SMASHHITTERRAIN | CORROSIVEFLAG_CRASHEFFECTSSPAWNED ) );
	PlaySound( m_BotInfo_Corrosive.pSoundGroupFistSmashGrunt );
}


void CBotCorrosive::_StartHandAttackInflictDamageMode( void ) {
	FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_INFLICTHANDATTACKDAMAGE );
	//floop_SetTimeScale( 0.1f );
}


void CBotCorrosive::_StopHandAttackInflictDamageMode( void ) {
	FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_INFLICTHANDATTACKDAMAGE );
	//floop_SetTimeScale( 1.0f );
}


void CBotCorrosive::_HandleHandAttackInflictDamageSweetRange( f32 fPrevTime, f32 fCurrentTime, f32 fTimeSweetStart, f32 fTimeSweetEnd ) {
	if( fPrevTime<fTimeSweetStart && fCurrentTime>=fTimeSweetStart ) {
		_StartHandAttackInflictDamageMode();
	}

	if( fPrevTime<fTimeSweetEnd && fCurrentTime>=fTimeSweetEnd ) {
		_StopHandAttackInflictDamageMode();
	}
}



void CBotCorrosive::_PerformHandAttackDamageCheck( const CFVec3A *pPrevSpherePos_WS, const CFVec3A *pCurrentSpherePos_WS ) {
	CFTrackerCollideProjSphereInfo TrackerCollideInfo;

	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = -1;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	m_CollInfo.bFindClosestImpactOnly = FALSE;
	m_CollInfo.bCullBacksideCollisions = FALSE;
	m_CollInfo.bCalculateImpactData = TRUE;
	if( m_eBehaviorState == BEHAVIOR_STATE_FIST_SMASH ) {
		m_CollInfo.ProjSphere.Init( pPrevSpherePos_WS, pCurrentSpherePos_WS, m_BotInfo_Corrosive.fFistSmashDamageSphereRadius * m_fScaleToWorld );
	} else if ( m_eBehaviorState == BEHAVIOR_STATE_SWAT ) {
		m_CollInfo.ProjSphere.Init( pPrevSpherePos_WS, pCurrentSpherePos_WS, m_BotInfo_Corrosive.fSwatDamageSphereRadius * m_fScaleToWorld * m_fSwatDamageRadiusMul );
	} else {
		FASSERT_NOW;
	}

	TrackerCollideInfo.pProjSphere = &m_CollInfo.ProjSphere;
	TrackerCollideInfo.pCallback = _HandAttackHitTrackerCallback;
	TrackerCollideInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	TrackerCollideInfo.nTrackerUserTypeBitsMask = -1;
	TrackerCollideInfo.bIgnoreCollisionFlag = FALSE;
	TrackerCollideInfo.nTrackerSkipCount = 0;
	TrackerCollideInfo.ppTrackerSkipList = NULL;

	m_pCollBot = this;
//	fworld_CollideWithTrackers( &TrackerCollideInfo );

//	fcoll_Clear();

	if( m_eBehaviorState != BEHAVIOR_STATE_SWAT ) {
		// Don't collide with world geo for the swat
		if( fworld_CollideWithWorldTris( &m_CollInfo ) ) {
			_SpawnHandAttackImpactEffects( &FColl_aImpactBuf[0] );

			if( !(m_nCorrosiveFlags & CORROSIVEFLAG_SMASHHITTERRAIN) ) {
				// Haven't hit any terrain yet...
				_InflictHandAttackDamage( NULL, NULL, &FColl_aImpactBuf[0] );
				FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_SMASHHITTERRAIN );
			}
		}
	}

	// Now handle collision with non-world geometry
	fcoll_Clear();
	fworld_CollideWithTrackers( &TrackerCollideInfo );
}


BOOL CBotCorrosive::_HandAttackHitTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	CBotCorrosive *pBotCorrosive = (CBotCorrosive *)m_pCollBot;
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Tracker is a non-entity...

		// This is a collision with non-entity geometry
		fcoll_Clear();

		if( pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
			// We hit a terrain triangle...

			pBotCorrosive->_SpawnHandAttackImpactEffects( &FColl_aImpactBuf[0] );
			if( !(pBotCorrosive->m_nCorrosiveFlags & CORROSIVEFLAG_SMASHHITTERRAIN) ) {
				// Haven't hit any terrain yet...
				pBotCorrosive->_InflictHandAttackDamage( NULL, NULL, &FColl_aImpactBuf[0] );
				FMATH_SETBITMASK( pBotCorrosive->m_nCorrosiveFlags, CORROSIVEFLAG_SMASHHITTERRAIN );
			}
		}
		return TRUE;
	}

	// This tracker is an entity...

	CEntity *pEntity = (CEntity *)pTracker->m_pUser;

	if( pEntity == (CEntity *)pBotCorrosive ) {
		// We hit ourselves... How, I don't know...
		return TRUE;
	}

	fcoll_Clear();

	if( !pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
		// We did not hit any triangles on the entity...
		return TRUE;
	}

	// We hit a triangle on the entity...

	pBotCorrosive->_SpawnHandAttackImpactEffects( &FColl_aImpactBuf[0] );

	if( pBotCorrosive->m_nHitEntityCount >= HIT_ENTITY_MAX_COUNT ) {
		// Cannot damage any more entities with this attack...
		return TRUE;
	}

	u32 i;

	for( i=0; i<pBotCorrosive->m_nHitEntityCount; ++i ) {
		if( pBotCorrosive->m_apHitEntity[i] == pEntity ) {
			// Already issued damage to this entity during this attack...
			return TRUE;
		}
	}

	//DEVPRINTF( "Damaged Entity!\n " );
	pBotCorrosive->_InflictHandAttackDamage( pEntity, pWorldMesh, &FColl_aImpactBuf[0] );

	// Now, add this entity to our hit-entity buffer to remember that we've already damaged it...
	if( pEntity->TypeBits() & ENTITY_BIT_WEAPON ) {
		// Entity is a CWeapon...

		CWeapon *pWeapon = (CWeapon *)pEntity;

		if( pWeapon->GetOwner() ) {
			// Weapon has an owner...

			if( pWeapon->GetBotDamageBoneIndex() >= 0 ) {
				// Weapon will pass damage onto its owner...

				pEntity = pWeapon->GetOwner();
			}
		}
	}

	pBotCorrosive->m_apHitEntity[ pBotCorrosive->m_nHitEntityCount++ ] = pEntity;

	return TRUE;
}


void CBotCorrosive::_InflictHandAttackDamage( CEntity *pHitEntity, CFWorldMesh *pHitWorldMesh, const FCollImpact_t *pCollImpact ) {
	if( pHitEntity ) {
		// Inflict damage to the entity...

		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...

			CFVec3A FireUnitDir_WS;
			f32 fMag2;

			FireUnitDir_WS.Sub( m_CollInfo.ProjSphere.m_vCenterEnd_WS, m_CollInfo.ProjSphere.m_vCenterStart_WS );

			fMag2 = FireUnitDir_WS.MagSq();
			if( fMag2 >= 0.000001f ) {
				FireUnitDir_WS.Mul( fmath_InvSqrt( fMag2 ) );
			} else {
				FireUnitDir_WS = CFVec3A::m_UnitAxisY;
			}

			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile = m_BotInfo_Corrosive.pDamageProfileFistSmash;
			pDamageForm->m_Damager.pWeapon = NULL;
			pDamageForm->m_Damager.pBot = this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_pDamageeEntity = pHitEntity;
			pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pCollImpact, &FireUnitDir_WS );

			CDamage::SubmitDamageForm( pDamageForm );
		}

		if( ( m_eBehaviorState == BEHAVIOR_STATE_SWAT ) && ( pHitEntity->TypeBits() & ENTITY_BIT_BOT ) ) {
			// We want to apply an impulse to the bot...	
			CBot *pHitBot = (CBot *)pHitEntity;

			CFVec3A ImpulseVec_WS;
			f32 fMag2;

			ImpulseVec_WS.Sub( m_CollInfo.ProjSphere.m_vCenterEnd_WS, m_CollInfo.ProjSphere.m_vCenterStart_WS );
			ImpulseVec_WS.y = 0.0f; // zero out any y value
			fMag2 = ImpulseVec_WS.MagSq();

			if( fMag2 > 0.00001f ) {
				ImpulseVec_WS.Mul( fmath_InvSqrt(fMag2) * 300.0f );

				DEVPRINTF( "Applying an impulse!\n" );

				pHitBot->ApplyVelocityImpulse_WS( ImpulseVec_WS );
			}
		}
	}
}



void CBotCorrosive::_SpawnHandAttackImpactEffects( const FCollImpact_t *pCollImpact ) {
	const CGCollMaterial *pMaterial = CGColl::GetMaterial( pCollImpact );

	if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST ) ) {
		pMaterial->DrawParticle(
			CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST,
			&pCollImpact->ImpactPoint,
			&pCollImpact->UnitFaceNormal,
			1.0f
		);
	}

	if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_BITS ) ) {
		pMaterial->DrawParticle(
			CGCollMaterial::PARTICLE_TYPE_BITS,
			&pCollImpact->ImpactPoint,
			&pCollImpact->UnitFaceNormal,
			1.0f
		);
	}

	if( pMaterial->HasLooseDust() ) {
		if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST ) ) {
			pMaterial->DrawParticle(
				CGCollMaterial::PARTICLE_TYPE_DUST,
				&pCollImpact->ImpactPoint,
				&pCollImpact->UnitFaceNormal,
				1.0f
			);
		}
	}

	if( pMaterial->CanDrawDebris( CGCollMaterial::DEBRIS_GROUP_MEDIUM ) ) {
		CFDebrisSpawner DebrisSpawner;
		DebrisSpawner.InitToDefaults();

		DebrisSpawner.m_Mtx.m_vPos = pCollImpact->ImpactPoint;
		DebrisSpawner.m_Mtx.m_vZ = pCollImpact->UnitFaceNormal;
		DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
		DebrisSpawner.m_pDebrisGroup = pMaterial->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_MEDIUM ];
		DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
		DebrisSpawner.m_fMinSpeed = 10.0f;
		DebrisSpawner.m_fMaxSpeed = 20.0f;
		DebrisSpawner.m_fUnitDirSpread = 0.2f;
		DebrisSpawner.m_fScaleMul = 1.0f;
		DebrisSpawner.m_fGravityMul = 1.0f;
		DebrisSpawner.m_fRotSpeedMul = 1.0f;
		DebrisSpawner.m_nMinDebrisCount = 1;
		DebrisSpawner.m_nMaxDebrisCount = 3;

		CGColl::SpawnDebris( &DebrisSpawner );
	}
}

///////////////////////////
// WEAPONS - FOOT STOMP

void CBotCorrosive::_HandleFootStompAnimation( AnimTap_e eStompTap ) {
	FASSERT( ( eStompTap == ANIMTAP_STOMP ) || (eStompTap == ANIMTAP_FORWARD_STOMP ) );

	f32 fTime = 0.0f, fPrevTime, fControlVal;

	fPrevTime = GetUnitTime( eStompTap );
	if( !DeltaTime( eStompTap, FLoop_fPreviousLoopSecs, TRUE ) ) {
		// We are still stomping
		fControlVal = 1.0f; //default to full stomp blend
		fTime = GetTime( eStompTap );
		if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
			// Blend in the stomp...
			fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		} else if( fTime > ( GetTotalTime( eStompTap ) - _BEHAVIOR_BLEND_INOUT_TIME ) ) {
			// Check to see if we should be blending out...
			fControlVal = fmath_UnitLinearToSCurve( ( GetTotalTime( eStompTap ) - fTime ) * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		}

		// Don't allow ourself to rotate!
		FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );

		// Reorient our groin...
		f32 fReorientTime = fTime;
		FMATH_CLAMPMAX( fReorientTime, _REORIENT_LEGS_TIME );
		FMATH_BIPOLAR_CLAMPMAX( m_fLegsYaw_MS, ( _REORIENT_LEGS_TIME - fReorientTime ) * m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold );

		// Gradually slow down our velocity over _ZERO_VELOCITY_TIME
		f32 fVelocityTime = fTime;
		FMATH_CLAMPMAX( fVelocityTime, _ZERO_VELOCITY_TIME );
		m_Velocity_WS.Mul( ( _ZERO_VELOCITY_TIME - fVelocityTime ) * _OO_ZERO_VELOCITY_TIME );
		VelocityHasChanged();

		f32 fCrashEffectTime;
		if( eStompTap == ANIMTAP_STOMP ) {
			_HandleFootStompInflictDamageSweetRange( fPrevTime, fTime, 1.0f, 1.5f );
			fCrashEffectTime = _FOOT_STOMP_CRASH_EFFECT_TIME;
		}
		else {
			_HandleFootStompInflictDamageSweetRange( fPrevTime, fTime, 0.5f, 1.25f );
			fCrashEffectTime = _FORWARD_FOOT_STOMP_CRASH_EFFECT_TIME;
		}
		// See if we need to play the foot crashing to the ground sound...
		_HandleCrashEffects( fTime, fCrashEffectTime, &m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxLToe ]->m_vPos );
		//ftext_DebugPrintf( 0.4f, 0.33f, "~w1time:  %0.2f", fTime );
	}
	else {
		// We are done with our stomp
		fControlVal = 0.0f;
		m_eLastBehaviorState = m_eBehaviorState;
		m_eBehaviorState = BEHAVIOR_STATE_NONE;

		// Re-enable rotation
		FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );
		//floop_SetTimeScale( 1.0f );
	}
	SetControlValue( eStompTap, fControlVal);
}


void CBotCorrosive::_InitStartFootStompData( void ) {
	FMATH_CLEARBITMASK( m_nCorrosiveFlags, ( CORROSIVEFLAG_USEFOOTSTOMPDAMAGEPROFILE | CORROSIVEFLAG_CRASHEFFECTSSPAWNED ) );
	FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_ZEROROTATION );
	PlaySound( m_BotInfo_Corrosive.pSoundGroupFistSmashGrunt );
	//floop_SetTimeScale( 0.25f );
}


void CBotCorrosive::_StartFootStompInflictDamageMode( void ) {
	FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_USEFOOTSTOMPDAMAGEPROFILE );
	//floop_SetTimeScale( 0.25f );
}


void CBotCorrosive::_StopFootStompInflictDamageMode( void ) {
	FMATH_CLEARBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_USEFOOTSTOMPDAMAGEPROFILE );
	//floop_SetTimeScale( 1.0f );
}


void CBotCorrosive::_HandleFootStompInflictDamageSweetRange( f32 fPrevTime, f32 fCurrentTime, f32 fTimeSweetStart, f32 fTimeSweetEnd ) {
	if( fPrevTime<fTimeSweetStart && fCurrentTime>=fTimeSweetStart ) {
		_StartFootStompInflictDamageMode();
	}

	if( fPrevTime<fTimeSweetEnd && fCurrentTime>=fTimeSweetEnd ) {
		_StopFootStompInflictDamageMode();
	}
}


void CBotCorrosive::_HandleCrashEffects( f32 fTime, f32 fTimeToSpawnEffects, const CFVec3A* pCrashEpicenter ) {
	if( ( fTime >= fTimeToSpawnEffects ) && !( m_nCorrosiveFlags & CORROSIVEFLAG_CRASHEFFECTSSPAWNED ) ) {
		PlaySound( m_BotInfo_Corrosive.pSoundGroupSmashCrash );

		// Submit a damage form for rumble
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if( pDamageForm ) {
			// Fill out the form...
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile	= m_BotInfo_Corrosive.pDamageProfileFootStompRumble;
			pDamageForm->m_Epicenter_WS		= *pCrashEpicenter;
			pDamageForm->m_Damager.pBot		= this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_fNormIntensity	= 1.0f;//fmath_Sqrt( fUnitMag );

			// submitting footdown damage form for titan mag = 
			CDamage::SubmitDamageForm( pDamageForm );
		}

		FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_CRASHEFFECTSSPAWNED );
	}
}

/////////////////////
// DEATH FUNCTIONS //
/////////////////////

void CBotCorrosive::Die( BOOL bSpawnDeathEffects, BOOL bSpawnGoodies ) {
	u32 i;
	CBot::Die( FALSE );

	//FMATH_SETBITMASK( m_uBotDeathFlags, BOTDEATHFLAG_WALKING_DEAD );
	FMATH_SETBITMASK( m_nCorrosiveFlags, CORROSIVEFLAG_NOAIXLAT );

	// Calculate the times to fire off random rockets
	for( i = 0; i < NUM_DEATH_RANDOM_ROCKETS; i++ ) {
		m_afDeathRandomRocketFireTimes[ i ] = fmath_RandomFloatRange( 1.0f, 7.5f );
	}

	m_fDeathMaxDestructionCountdownSecs = _DEATH_MAX_DESTRUCTION_TIME;
	ZeroTime( ANIMTAP_DEATH );
}


void CBotCorrosive::DeathWork( void ) {

	f32 fPrevTime, fTime = 0.0f, fControlVal;
	fControlVal = 1.0f; //default to full death blend
	
	fPrevTime = GetTime( ANIMTAP_DEATH );
	if( !DeltaTime( ANIMTAP_DEATH, FLoop_fPreviousLoopSecs, TRUE ) ) {

		fTime = GetTime( ANIMTAP_DEATH );
		if( fTime < _BEHAVIOR_BLEND_INOUT_TIME ) { //quarter of a second blend in time...
			// Blend in the death anim...
			fControlVal = fmath_UnitLinearToSCurve( fTime * _OO_BEHAVIOR_BLEND_INOUT_TIME );
		}

		// Reorient our groin...
		f32 fReorientTime = fTime;
		FMATH_CLAMPMAX( fReorientTime, _REORIENT_LEGS_TIME );
		FMATH_BIPOLAR_CLAMPMAX( m_fLegsYaw_MS, ( _REORIENT_LEGS_TIME - fReorientTime ) * m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold );

	}
	SetControlValue( ANIMCONTROL_DEATH, fControlVal);
	
	_DeathWork_PlaySounds( fPrevTime, fTime );
	_DeathWork_HandleEffects( fPrevTime, fTime );

	m_fDeathMaxDestructionCountdownSecs -= FLoop_fPreviousLoopSecs;
	if( m_fDeathMaxDestructionCountdownSecs <= 0.0f ) {
		m_uLifeCycleState = BOTLIFECYCLE_DEAD;
	}

	//ftext_DebugPrintf( 0.4f, 0.33f, "~w1time:  %0.2f", fTime );
}

#define _DEATH_SOUND1_START_TIME	0.5f
#define _DEATH_SOUND2_START_TIME	4.50f
#define _DEATH_CRASH_IMPACT_TIME	7.75f

void CBotCorrosive::_DeathWork_HandleEffects( f32 fPrevTime, f32 fCurrentTime ) {

	u32 i;

	// Check if it's time to submit the crash rumble form
	if( ( fPrevTime < _DEATH_CRASH_IMPACT_TIME ) && ( fCurrentTime >= _DEATH_CRASH_IMPACT_TIME ) ) {
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if( pDamageForm ) {
			// Fill out the form...
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile	= m_BotInfo_Corrosive.pDamageProfileDeathCrashRumble;
			pDamageForm->m_Epicenter_WS = m_pWorldMesh->GetBoneMtxPalette()[ m_nBoneIdxTorso ]->m_vPos;
			pDamageForm->m_Damager.pBot		= this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_fNormIntensity	= 1.0f;//fmath_Sqrt( fUnitMag );

			// submitting footdown damage form for titan mag = 
			CDamage::SubmitDamageForm( pDamageForm );
		}
	}

	// Check to see if we should fire off a random rocket!
	for( i = 0; i < NUM_DEATH_RANDOM_ROCKETS; i++ ) {
		if( ( fPrevTime < m_afDeathRandomRocketFireTimes[ i ] ) && ( fCurrentTime >= m_afDeathRandomRocketFireTimes[ i ] ) ) {
			// Fire off a rocket!
			SetTargetedEntity( NULL ); // We don't want the rockets to home in on glitch!
			_FireRocket();
		}
	}

	// Check to see if we should trigger a death explosion!
	for( i = 0; i < NUM_DEATH_RANDOM_ROCKETS; i++ ) {
		if( ( fPrevTime < m_paDeathExplosions[ i ].fTriggerTime ) && ( fCurrentTime >= m_paDeathExplosions[ i ].fTriggerTime ) ) {
			// Trigger an explosion
			if( m_BotInfo_Corrosive.hDeathExplosion != FEXPLOSION_INVALID_HANDLE ) {
				FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

				if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
					FExplosionSpawnParams_t SpawnParams;
					SpawnParams.InitToDefaults();
					SpawnParams.Pos_WS.Set( m_pWorldMesh->GetBoneMtxPalette()[ m_paDeathExplosions[ i ].nBoneIndex ]->m_vPos );
					SpawnParams.UnitDir = m_pWorldMesh->GetBoneMtxPalette()[ m_paDeathExplosions[ i ].nBoneIndex ]->m_vFront;
					CExplosion2::SpawnExplosion( hSpawner, m_BotInfo_Corrosive.hDeathExplosion, &SpawnParams );
				}
			}
		}
	}
}



void CBotCorrosive::_DeathWork_PlaySounds( f32 fPrevTime, f32 fCurrentTime ) {

	// Check for the first sound...
	if( ( fPrevTime < _DEATH_SOUND1_START_TIME ) && ( fCurrentTime >= _DEATH_SOUND1_START_TIME ) ) {
		// Play the first death sound...
//		PlaySound( m_BotInfo_Corrosive.pSoundGroupDeath1 ); // Start the painful roar
		CFSoundGroup::PlaySound( m_BotInfo_Corrosive.pSoundGroupDeath1 ); // Play the sounds in 2d, as a whole bunch of stuff will be going on anyway
	}

	// Check for the Second death sound...
	if( ( fPrevTime < _DEATH_SOUND2_START_TIME ) && ( fCurrentTime >= _DEATH_SOUND2_START_TIME ) ) {
		// Play the first death sound...
//		PlaySound( m_BotInfo_Corrosive.pSoundGroupDeath2 ); // Start the final roar
		CFSoundGroup::PlaySound( m_BotInfo_Corrosive.pSoundGroupDeath2 ); // Play the sounds in 2d, as a whole bunch of stuff will be going on anyway
	}

	// Check to play a crash sound...
	if( ( fPrevTime < _DEATH_CRASH_IMPACT_TIME ) && ( fCurrentTime >= _DEATH_CRASH_IMPACT_TIME ) ) {
		// Play the first death sound...
//		PlaySound( m_BotInfo_Corrosive.pSoundGroupDeathCrash ); // Play the crashing sound
		CFSoundGroup::PlaySound( m_BotInfo_Corrosive.pSoundGroupDeathCrash ); // Play the sounds in 2d, as a whole bunch of stuff will be going on anyway
	}
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotCorrosiveBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CBotCorrosiveBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTCORROSIVE, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotCorrosive::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotCorrosive::m_BotInfo_Gen.pszArmorProfile;

	m_pszBotTossSpawnPtName = NULL;
	m_pszBotTossAlarmName = NULL;
}


BOOL CBotCorrosiveBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotCorrosiveBuilder::InterpretTable( void ) {

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "BotTossSpawnPtName" ) ) {
		CEntityParser::Interpret_String(&m_pszBotTossSpawnPtName);
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "BotTossAlarmName" ) ) {
		CEntityParser::Interpret_String(&m_pszBotTossAlarmName);
		return TRUE;

	}

	return CBotBuilder::InterpretTable();
}
