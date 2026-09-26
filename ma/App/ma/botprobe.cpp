//////////////////////////////////////////////////////////////////////////////////////
// botprobe.cpp - 
//
// Author: Nathan Miller
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
// 12/30/02 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdebris.h"
#include "fworld_coll.h"
#include "fresload.h"

#include "Damage.h"
#include "botprobe.h"
#include "MeshTypes.h"
#include "ProTrack.h"
#include "AI/AIBrainMan.h"
#include "botglitch.h"
#include "collectable.h"
#include "botpart.h"

// !!Nate
//#include "player.h"

#define _SOUND_BANK_PROBE				"Leech"
#define _BOTINFO_FILENAME				"b_probe"
#define _BOTPART_FILENAME				"bp_probe"
#define _BOTINFO_MESHNAME				"GRMlleech00"
#define _RING_MESH						"gfml_ring"

#define _JET_INTENSITY_IDLE				( 0.0f )

#define _LEECH_TURN_TIME_MAX			( 0.25f )
#define _OO_LEECH_TURN_TIME_MAX			( 4.0f )

#define _MAX_VELOCITY_XZ				( 30.0f )
#define _MAX_ACCEL_XZ					( 90.0f )
#define _DECEL_MULT_XZ					( 1.25f )

#define _MAX_VERT_VEL					( 30.0f )
#define _VERT_ACCEL_PER_SEC 			( 90.0f )
#define _DECEL_MULTIPLIER				( 1.5f )

#define _NEXT_SPIN_HIT_TIME				( 0.1f )

#define _SHAKE_UNIT_INTENSITY			( 0.25f )
#define _SHAKE_DURATION					( 0.25f )

#define _IDLE_VOLUME_MIN				( 0.35f )
#define _IDLE_VOLUME_MAX				( 1.0f )

#define _SOUND_RADIUS_IDLE				( 50.0f )
#define _SOUND_RADIUS_SPIN				( 50.0f )
#define _SOUND_RADIUS_MISS				( 50.0f )
#define _SOUND_RADIUS_GROUND_HIT		( 50.0f )
#define _SOUND_RADIUS_FLING				( 50.0f )
#define _SOUND_IMPACT_RADIUS			( 50.0f )

#define _EMITTER_VOLUME_TIMESCALE		( 1.0f )
#define _OO_EMITTER_VOLUME_TIMESCALE	( 1.0f )

#define _HIT_SOUND_TIME					( 0.15f )

#define _LAG_POINT_MOVE_PERCENT			( 2.0f )
#define _LAG_POINT_MOVE_PERCENT_ATTACK  ( 4.0f )
#define _LAG_POINT_MAX_DISTANCE			( 5.0f )
#define _LAG_POINT_MAX_DISTANCE_SQ		( ( _LAG_POINT_MAX_DISTANCE * _LAG_POINT_MAX_DISTANCE ) )
#define _LAG_POINT_Y_POS				( -15.0f )

#define _BOT_PART_NUM_MIN				( 3 )
#define _BOT_PART_NUM_MAX				( 6 )

#define _GROUND_SPHERE_DROP_DISTANCE	( -5.5f )

#define _CORONA_TEXTURE_ID				( 1 )

#define _NUM_MAGNET_RINGS				( 20 )
#define _RING_TIME						( 0.15f )
#define _SHAKE_TIME						( 0.5f )
#define _RING_TIME_SCALE				( 1.0f )
#define _RING_SHAKE_DURATION			( 0.1f )

struct _MagnetRing_t {
	f32	fUnitTime;
	CFWorldMesh *pMesh;
	FLink_t link;
};

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotProbeBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotProbeBuilder _BotProbeBuilder;


void CBotProbeBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTPROBE, pszEntityType );

	m_pszEC_ArmorProfile = CBotProbe::m_BotInfo_Gen.pszArmorProfile;

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotProbeBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotProbeBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotProbe
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotProbe::m_bSystemInitialized = FALSE;
u32 CBotProbe::m_nBotClassClientCount = 0;
BOOL CBotProbe::m_bRingsSetup = FALSE;
CBotAnimStackDef CBotProbe::m_AnimStackDef;
CBotProbe::BotInfo_Gen_t		CBotProbe::m_BotInfo_Gen;
CBotProbe::BotInfo_MountAim_t	CBotProbe::m_BotInfo_MountAim;
CBotProbe::BotInfo_Walk_t		CBotProbe::m_BotInfo_Walk;
CBotProbe::BotInfo_Weapon_t		CBotProbe::m_BotInfo_Weapon;
CBotProbe::BotInfo_Probe_t		CBotProbe::m_BotInfo_Probe;
//CFCollInfo CBotProbe::m_CollInfo;										// Used for collision detection
//CFSphere CBotProbe::m_CollSphere;										// Used for collision detection
CBotProbe *CBotProbe::m_pThisBot;										// Used for collision detection
FParticle_DefHandle_t CBotProbe::m_hJetParticleDef = FPARTICLE_INVALID_HANDLE;
FParticle_DefHandle_t CBotProbe::m_hSparkParticleDef = FPARTICLE_INVALID_HANDLE;
FParticle_DefHandle_t CBotProbe::m_hPuffParticleDef =  FPARTICLE_INVALID_HANDLE;

CDamageProfile *CBotProbe::m_apDamageProfile[_DAMAGE_COUNT];
FSndFx_FxHandle_t CBotProbe::m_hSounds[_SOUND_COUNT];
CFDebrisGroup *CBotProbe::m_pDebrisGroup = NULL;
_MagnetRing_t *CBotProbe::m_aRings = NULL;
FLinkRoot_t CBotProbe::m_FreeRingList;
CBotPartPool *CBotProbe::m_pPartPool = NULL;

/*CBotProbe::BotInfo_Gen_t CBotProbe::m_BotInfo_Gen = {
	0.0f,							//	f32 fGravity;

	0.0f, 							//	f32 fCollSphere1X_MS;
	2.9f, 							//	f32 fCollSphere1Y_MS;
	0.0f, 							//	f32 fCollSphere1Z_MS;
	3.0f, 							//	f32 fCollSphere1Radius_MS;

	NULL,							// cchar *pszDataPortBoneName;					// NULL=Bot cannot have a data port
	NULL, NULL,						// cchar *apszDataPortMeshName[2];				// [0]=closed, [1]=opened   (NULL=none)

	1.0f,							// f32 fPowerDownSpeedMult;						// Anim speed multiplier applied to the power-down animation
	1.0f,							// f32 fPowerUpSpeedMult;						// Anim speed multiplier applied to the power-up animation
	30.0f,							// f32 fDisguiseTime;							// Amount of time a possessed bot will be disguised
	150.0f,							// f32 fPossessionDist;							// Maximum distance a possessed bot can travel
	"light",						// cchar *pszWeightClass;						// Weight class of the bot: 'light', 'medium' and 'heavy'

	1,								// u32 nBatteryCount;							// Number of batteries (health containers) for this bot
	"Probe",						// cchar *pszArmorProfile;						// The default armor profile to use with this bot
};


CBotProbe::BotInfo_MountAim_t CBotProbe::m_BotInfo_MountAim = {
	0.191f,						//# fMountPitchUpDownRevsPerSec
	FMATH_DEG2RAD( 50.0f ),		//# fMountPitchDownLimit
	FMATH_DEG2RAD( -30.0f ),	//# fMountPitchUpLimit
	0.25f,						//# fMountYawBaseRevsPerSec
	0.2f,						//# fMountYawOverdriveRevsPerSec
	2.0f,						//# fYawDeltaUnitIntoOverdrivePerSec
	10.0f,						//# fYawDeltaUnitOutofOverdrivePerSec
	0.93f,						//# fYawUnitIntoControlOverdriveThresh
	0.89f,						//# fYawUnitOutofControlOverdriveThresh
	2.0f						//# fMountYawAIQuickTurnRevsPerSec
};

CBotProbe::BotInfo_Probe_t CBotProbe::m_BotInfo_Probe = {
	0.0f, 5.0f, 0.0f,	// Grab collision sphere
	3.5f,

	1.0f, 3.2f, 13.5f,	// Dead sphere
	3.0f,

	// Magnet stuff
	35.0f,
	15.0f,

	"PRMLjet_001",
	"probe_spark",
	"probe_puff",

	"ProbePinch",		// cchar *pszDamageProfilePinch
	"ProbeGroundHit",	// cchar *pszDamageProfileShake
	"ProbeSpin",		// cchar *pszDamageProfileSpin
};*/

BOOL CBotProbe::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_nBotClassClientCount = 0;
	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CBotProbe::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_AnimStackDef.Destroy();
		m_bSystemInitialized = FALSE;
	}
}

CBotProbe::CBotProbe() : CBot() {

}

CBotProbe::~CBotProbe() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

BOOL CBotProbe::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	CBotProbeBuilder *pBuilder = (CBotProbeBuilder *) GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotProbeBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTPROBE );
	}

	// set up any builder params...

	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}

// Begin the magnet work on the passed bot.  
// !!EndMagnet must be called when we don't want to do magnet work on the bot any longer
void CBotProbe::BeginMagnet( CBot *pBot ) {
	FASSERT( pBot );

	// NKM - Don't let the magnet turn on when powered down
	if( Power_IsPoweredDown() ) {
		return;
	}

	m_MagnetDir_MS.Zero();

	m_pMagnetBot = pBot;
	m_pMagnetBot->SetStickModifier_MS( &m_MagnetDir_MS );

	m_pMagnetEmitter = FSNDFX_ALLOCNPLAY3D( m_hSounds[_SOUND_ATTACK_MAGNET_LOOP], m_pTorsoBonePos, m_BotInfo_Probe.fMagnetOuterRadius + 6.0f, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );

	if( m_pMagnetEmitter ) {
		m_pMagnetEmitter->SetVolume( 1.0f );
	}
}

void CBotProbe::EndMagnet( void ) {
	_DisableMagnet();
}

void CBotProbe::AbortAttack( void ) {
	m_fUnitBlendDir = -1.0f;
	m_pBotLeeching = NULL;
	m_BotState = BOTSTATE_IDLE;
}

void CBotProbe::GroundHitDamageBot( CBot *pBot ) {
	FASSERT( pBot );

	CFVec3A SpawnPos;

	SpawnPos = CFVec3A::m_UnitAxisY;
	SpawnPos.Mul( 2.0f );
	SpawnPos.Sub( CFVec3A::m_UnitAxisZ );
	SpawnPos.Add( pBot->MtxToWorld()->m_vPos );

	fparticle_SpawnEmitter( m_hPuffParticleDef, SpawnPos.v3, &CFVec3A::m_UnitAxisY.v3, fmath_RandomFloat() );
	fparticle_SpawnEmitter( m_hPuffParticleDef, SpawnPos.v3, &CFVec3A::m_UnitAxisY.v3, fmath_RandomFloat() );
	fparticle_SpawnEmitter( m_hPuffParticleDef, SpawnPos.v3, &CFVec3A::m_UnitAxisY.v3, fmath_RandomFloat() );
	fparticle_SpawnEmitter( m_hPuffParticleDef, SpawnPos.v3, &CFVec3A::m_UnitAxisY.v3, fmath_RandomFloat() );
	
	// Some sparks
	fparticle_SpawnEmitter( m_hSparkParticleDef, &SpawnPos.v3, &CFVec3A::m_UnitAxisY.v3 );
	fparticle_SpawnEmitter( m_hSparkParticleDef, &SpawnPos.v3, &CFVec3A::m_UnitAxisY.v3 );

	// Damage the bot
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	if( pDamageForm ) {
		// Fill out the form...
		pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_AMBIENT;
		pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_pDamageProfile	= m_apDamageProfile[_DAMAGE_HIT_GROUND];
		pDamageForm->m_Damager.pWeapon	= NULL;
		pDamageForm->m_Damager.pBot		= this;
		pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
		pDamageForm->m_pDamageeEntity	= pBot;
		
		CDamage::SubmitDamageForm( pDamageForm );
	}

	// Play a sound
	if( pBot->m_nPossessionPlayerIndex < 0 ) {
		fsndfx_Play3D( m_hSounds[_SOUND_ATTACK_GROUND_HIT], &pBot->MtxToWorld()->m_vPos, _SOUND_RADIUS_GROUND_HIT );
	} else {
		fsndfx_Play2D( m_hSounds[_SOUND_ATTACK_GROUND_HIT] );
	}

	_SpawnBotParts( &pBot->MtxToWorld()->m_vPos, _BOT_PART_NUM_MIN, _BOT_PART_NUM_MAX );
}

void CBotProbe::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CBotProbe::CheckpointSave( void ) { 
	CBot::CheckpointSave();

	return TRUE;
}

void CBotProbe::CheckpointRestore( void ) { 
	m_pCollBot = this;

	CBot::CheckpointRestore();

	m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );

	m_BotState = BOTSTATE_IDLE;
	m_BotAnimControl = ANIMCONTROL_STAND;
	m_BotAnimTap = ANIMTAP_STAND;

	SetControlValue( m_BotAnimControl, 1.0f );
	UpdateUnitTime( m_BotAnimTap, fmath_RandomFloat() );

	SetControlValue( ANIMTAP_ATTACK_LEECH, 0.0f );
	SetControlValue( ANIMTAP_ATTACK_SPIN, 0.0f );
	SetControlValue( ANIMTAP_ATTACK_FLING, 0.0f );
	SetControlValue( ANIMTAP_POWER_DOWN, 0.0f );
	SetControlValue( ANIMTAP_POWER_UP, 0.0f );

	m_bAttackTested = FALSE;
	m_fUnitBlendAttack = 0.0f;
	m_fUnitBlendDir = 0.0f;

	m_fLeechTurnTime = 0.0f;
	m_StartPosition.Zero();
	m_pBotLeeching = NULL;

	AbortAttack();
	_ReleaseRings();
}

void CBotProbe::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies /*= TRUE*/ ) {
	_DisableMagnet();
	_ReleaseRings();

	if( m_hJetEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hJetEmitter );
	}

	CBot::Die( bSpawnDeathEffects, bSpawnGoodies );
}

void CBotProbe::Power( BOOL bPowerUp, f32 fPowerOffTime/*=0.0f*/, f32 fPowerOffOnSpeedMult/*=1.0f*/ ) {
	if( bPowerUp ) {
		//SetSpotLightOn();
	} else {
		//SetSpotLightOff();
		_DisableMagnet();
	}

	CBot::Power( bPowerUp );
}


f32 CBotProbe::ComputeEstimatedControlledStopTimeXZ( void ) {
	FASSERT( IsCreated() );
	// our accel is constant, and always at the increased value, because we're coming to a stop.
	return fmath_Div( m_fSpeedXZ_WS, _MAX_ACCEL_XZ * _DECEL_MULT_XZ );
}


f32 CBotProbe::ComputeEstimatedControlledStopTimeY( void ) {
	FASSERT( IsCreated() );
	return FMATH_FABS( fmath_Div( m_Velocity_WS.y, _VERT_ACCEL_PER_SEC * _DECEL_MULTIPLIER ) );
}

BOOL CBotProbe::ClassHierarchyLoadSharedResources( void ) {
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
		// resources already loaded
		return TRUE;
	}

	FResFrame_t ResFrame;

	ResFrame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	fang_MemZero( &m_BotInfo_Walk, sizeof( m_BotInfo_Walk ) );
	fang_MemZero( &m_BotInfo_Weapon, sizeof( m_BotInfo_Weapon ) );

	m_aRings = (_MagnetRing_t *) fres_AlignedAlloc( _NUM_MAGNET_RINGS * sizeof( _MagnetRing_t ), 16 );

	if( m_aRings == NULL ) {
		DEVPRINTF( "CBotProbe::ClassHierarchyLoadSharedResources(): Unable to allocate array of rings\n" );
		goto _ExitInitSystemWithError;
	}

	flinklist_InitRoot( &m_FreeRingList, FANG_OFFSETOF( _MagnetRing_t, link ) );

	for( u32 i = 0; i < _NUM_MAGNET_RINGS; ++i ) {
		m_aRings[i].fUnitTime = 0.0f;
		m_aRings[i].pMesh = NULL;
		flinklist_AddTail( &m_FreeRingList, &m_aRings[i] );
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitInitSystemWithError;
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitInitSystemWithError;
	}

	// Load the particles
	if( m_BotInfo_Probe.pszJetParticleName != NULL ) {
		m_hJetParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Probe.pszJetParticleName );
		
		if( m_hJetParticleDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CBotProbe::ClassHierarchyLoadSharedResources(): Could not find particle definition '%s'.\n", m_BotInfo_Probe.pszJetParticleName );
		}
	}

	if( m_BotInfo_Probe.pszSparkParticleName != NULL ) {
		m_hSparkParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Probe.pszSparkParticleName );
		
		if( m_hSparkParticleDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CBotProbe::ClassHierarchyLoadSharedResources(): Could not find particle definition '%s'.\n", m_BotInfo_Probe.pszSparkParticleName );
		}
	}

	if( m_BotInfo_Probe.pszPuffParticleName != NULL ) {
		m_hPuffParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Probe.pszPuffParticleName );
		
		if( m_hPuffParticleDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CBotProbe::ClassHierarchyLoadSharedResources(): Could not find particle definition '%s'.\n", m_BotInfo_Probe.pszPuffParticleName );
		}
	}

	m_apDamageProfile[_DAMAGE_PINCH]		= CDamage::FindDamageProfile( m_BotInfo_Probe.pszDamageProfilePinch );
	m_apDamageProfile[_DAMAGE_HIT_GROUND]	= CDamage::FindDamageProfile( m_BotInfo_Probe.pszDamageProfileShake );
	m_apDamageProfile[_DAMAGE_SPIN]			= CDamage::FindDamageProfile( m_BotInfo_Probe.pszDamageProfileSpin );

	// Get our debris group
	m_pDebrisGroup = CFDebrisGroup::Find( "MechMed" );

	if( !m_pDebrisGroup ) {
		DEVPRINTF( "CBotProbe::ClassHierarchyLoadSharedResources(): Could not find debris group '%s'.\n", "MechSml" );
	}

	// Sounds
	_InitSoundHandles();

	if( !m_bRingsSetup ) {
		if( !_SetupRings() ) {
			goto _ExitInitSystemWithError;
		}
	}

	return TRUE;

	// Error:
_ExitInitSystemWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotProbe::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	m_nBotClassClientCount--;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}

	m_AnimStackDef.Destroy();
	
	_DestroyRings();

	for( u32 i = 0; i < _SOUND_COUNT; ++i ) {
		m_hSounds[i] = FSNDFX_INVALID_FX_HANDLE;
	}

	m_hJetParticleDef = FPARTICLE_INVALID_HANDLE;
	m_hSparkParticleDef = FPARTICLE_INVALID_HANDLE;
	m_hPuffParticleDef = FPARTICLE_INVALID_HANDLE;

	m_apDamageProfile[_DAMAGE_PINCH]		= NULL;
	m_apDamageProfile[_DAMAGE_HIT_GROUND]	= NULL;
	m_apDamageProfile[_DAMAGE_SPIN]			= NULL;

	m_pDebrisGroup = NULL;

	CBot::ClassHierarchyUnloadSharedResources();
}

void CBotProbe::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

//	for( u32 i = 0; i < _SOUND_COUNT; ++i ) {
//		m_hSounds[i] = FSNDFX_INVALID_FX_HANDLE;
//	}
//
//	_DestroyRings();

	CBot::ClassHierarchyDestroy();
}

BOOL CBotProbe::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	FMesh_t		*pMesh;
	FMeshInit_t  MeshInit;
	FResFrame_t  frame = fres_GetFrame();
	u32			 uBoneIdx;
	s32			 sBone;			//CPS 4.7.03
	
	// Get pointer to the leaf class's builder object...
	CBotProbeBuilder *pBuilder = (CBotProbeBuilder*) GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Create the mesh...
	pMesh = (FMesh_t*) fresload_Load( FMESH_RESTYPE, _BOTINFO_MESHNAME );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotProbe::ClassHierarchyBuild(): Could not find mesh '%s'\n", _BOTINFO_MESHNAME );
		goto _ExitWithError;
	}

	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotProbe::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	MeshInit.pMesh		= pMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist	= FMATH_MAX_FLOAT;
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



	// Find approx eye point...
	uBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( uBoneIdx < 0 ) {
		DEVPRINTF( "CBotProbe::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[uBoneIdx]->m_vPos;
	}

	// Find gaze direction...
	uBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( uBoneIdx < 0 ) {
		DEVPRINTF( "CBotProbe::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[uBoneIdx]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotProbe::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Get our bones of interest
//CPS 4.7.03	s32 sBone = m_pWorldMesh->FindBone( "Torso" );
	sBone = m_pWorldMesh->FindBone( "Torso" );
	FASSERT( sBone >= 0 );
	m_pTorsoBonePos = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;

	sBone = m_pWorldMesh->FindBone( "Leg_Root_Dummy" );
	FASSERT( sBone >= 0 );
	m_pJetAttachBonePos = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;

	sBone = m_pWorldMesh->FindBone( "Leg1_Dummy" );
	FASSERT( sBone >= 0 );
	m_apLegTestBones[0] = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;

	sBone = m_pWorldMesh->FindBone( "Leg2_Dummy" );
	FASSERT( sBone >= 0 );
	m_apLegTestBones[1] = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;

	sBone = m_pWorldMesh->FindBone( "Leg3_Dummy" );
	FASSERT( sBone >= 0 );
	m_apLegTestBones[2] = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;

	sBone = m_pWorldMesh->FindBone( "Leg1_E" );
	FASSERT( sBone >= 0 );
	m_apLegTestBones[3] = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;

	sBone = m_pWorldMesh->FindBone( "Leg2_E" );
	FASSERT( sBone >= 0 );
	m_apLegTestBones[4] = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;

	sBone = m_pWorldMesh->FindBone( "Leg3_E" );
	FASSERT( sBone >= 0 );
	m_apLegTestBones[5] = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;


	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();

	// Setup the animations
	m_BotAnimControl = ANIMCONTROL_STAND;
	m_BotAnimTap = ANIMTAP_STAND;

	SetControlValue( m_BotAnimControl, 1.0f );
	UpdateUnitTime( m_BotAnimTap, fmath_RandomFloat() );

	EnableControlSmoothing( ANIMTAP_ATTACK_LEECH );
	EnableControlSmoothing( ANIMTAP_ATTACK_SPIN );
	EnableControlSmoothing( ANIMTAP_ATTACK_FLING );
	EnableControlSmoothing( ANIMTAP_POWER_DOWN );
	EnableControlSmoothing( ANIMTAP_POWER_UP );

	//m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _CORONA_TEXTURE_ID );
	//SetSpotLightOff();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	// Setup the ring list
	flinklist_InitRoot( &m_MagnetRings, FANG_OFFSETOF( _MagnetRing_t, link ) );


	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		goto _ExitWithError;
	}

	// Misc stuff...
	SetMaxHealth();
	SetBotFlag_Enemy();

//	// Sounds
//	_InitSoundHandles();
//
//	if( !m_bRingsSetup ) {
//		if( !_SetupRings() ) {
//			goto _ExitWithError;
//		}
//	}

	// Let the collectable system know we need washers, they are tossed in botglitch
	CCollectable::NotifyCollectableUsedInWorld( "washer" );

	return TRUE;

// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}

BOOL CBotProbe::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	// All bots, by default, cast shadows.  THis one doesn't
	m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;

	EnableOurWorkBit();

	// !!Nate
//	SetControls( &Player_aPlayer[0].m_HumanControl );
//	m_bControls_Human = TRUE;
//	m_nPossessionPlayerIndex = 0;

	// Hack, just for testing
//	BeginMagnet( (CBot *) Player_aPlayer[0].m_pEntityOrig );

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

CEntityBuilder *CBotProbe::GetLeafClassBuilder( void ) {
	return &_BotProbeBuilder;
}

void CBotProbe::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	//m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM;
	//m_uBotDeathFlags |= BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;

	// Idle sound
	if( !m_pIdleEmitter ) {
		m_pIdleEmitter = FSNDFX_ALLOCNPLAY3D( m_hSounds[_SOUND_HOVER_LOOP], m_pTorsoBonePos, _SOUND_RADIUS_IDLE, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
	}

	m_LagPoint = MtxToWorld()->m_vPos;
	
	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	// Start going straight down
	m_JetEmitterDir.Set( CFVec3A::m_UnitAxisY );
	m_JetEmitterDir.Mul( -1.0f );

	// Spawn our particle emitter
	m_fJetIntensity = _JET_INTENSITY_IDLE;
	m_hJetEmitter = fparticle_SpawnEmitter( m_hJetParticleDef, &(*m_pJetAttachBonePos).v3, &m_JetEmitterDir.v3, &m_Velocity_WS.v3, m_fJetIntensity );

	if( m_hJetEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_SetIntensity( m_hJetEmitter, &m_fJetIntensity );
		fparticle_SetDirection( m_hJetEmitter, &m_JetEmitterDir.v3 );
	} else {
		DEVPRINTF( "CBotProbe::ClassHierarchyBuild: Failed to create particle emitter!\n" );
	}

	m_fNextRingTime = _RING_TIME;
	m_fNextShakeTime = _SHAKE_TIME;
	m_pMagnetBot = NULL;
}


void CBotProbe::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	if( m_pIdleEmitter ) {
		m_pIdleEmitter->Destroy();
		m_pIdleEmitter = NULL;
	}

	if( m_pSpinEmitter ) {
		m_pSpinEmitter->Destroy();
		m_pSpinEmitter = NULL;
	}

	if( m_pMagnetEmitter ) {
		m_pMagnetEmitter->Destroy();
		m_pMagnetEmitter = NULL;
	}

	m_pWorldMesh->RemoveFromWorld();

	// Destroy the particle emitter
	fparticle_StopEmitter( m_hJetEmitter );

	//SetSpotLightOff();

	_ReleaseRings();

	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotProbe::ClassHierarchyWork( void ) {
	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	Power_Work();

	ParseControls();
	ComputeXlatStickInfo();

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// Apply velocity impulses that were accumulated last frame...
	BOOL bImpulseApplied = HandleVelocityImpulses();

	// Update position...
	CFVec3A TempVec3A;
	CFVec3A UnitXZ;

	TempVec3A.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( TempVec3A );
	
	TempVec3A.Sub( m_MountPos_WS, m_LagPoint );
	TempVec3A.y = 0.0f;

	if( !IsAttackSpinEnabled() ) {
		TempVec3A.Mul( ( _LAG_POINT_MOVE_PERCENT * FLoop_fPreviousLoopSecs ) );
	} else { 
		// If we are attacking, get back to where we should be a whole lot faster
		TempVec3A.Mul( ( _LAG_POINT_MOVE_PERCENT_ATTACK * FLoop_fPreviousLoopSecs ) );
	}

	m_LagPoint.Add( TempVec3A );

	if( TempVec3A.Sub( m_LagPoint, m_MountPos_WS ).MagSqXZ() > _LAG_POINT_MAX_DISTANCE_SQ ) {
		TempVec3A.UnitizeXZ();
		m_LagPoint = TempVec3A;
		m_LagPoint.y = 0.0f;
		m_LagPoint.Mul( _LAG_POINT_MAX_DISTANCE );
		m_LagPoint.Add( m_MountPos_WS );
	}
	
	// let CBot handle the mount yaw and pitch so it's consistent w/ other bots
	// So we don't turn while animating
	if( m_BotState != BOTSTATE_LEECHING && m_BotState != BOTSTATE_LEECH_TURN ) {
		HandleYawMovement();
		HandlePitchMovement();
	}

	// Rotate model space velocity to account for yaw change...
	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// If an external velocity impulse was applied above, recompute our velocities
	// and speeds...
	if( bImpulseApplied ) {
		VelocityHasChanged();
	}

	PROTRACK_BEGINBLOCK("Coll");
		_HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	PROTRACK_BEGINBLOCK("Move");
		_HandleHover();
		_HandleTranslation();
	PROTRACK_ENDBLOCK();//"Move"

	PROTRACK_BEGINBLOCK("Attack");
		_HandleAttacks();
		_HandleAttackAnimation();
	PROTRACK_ENDBLOCK();//"Attack"

	// Always play the stand animation
	DeltaTime( ANIMTAP_STAND );

	_UpdateMatrices();
	_UpdateMagnet();
	_UpdateActiveRings();

	if( IsDeadOrDying() ) {
		if( m_pIdleEmitter ) {
			m_pIdleEmitter->Destroy();
			m_pIdleEmitter = NULL;
		}

		if( m_pSpinEmitter ) {
			m_pSpinEmitter->Destroy();
			m_pSpinEmitter = NULL;
		}

		if( m_pMagnetEmitter ) {
			m_pMagnetEmitter->Destroy();
			m_pMagnetEmitter = NULL;
		}

		DeathWork();
	}

	m_bSkipAnimAdvance = FALSE;
}

void CBotProbe::_ClearDataMembers( void ) {
	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_MountAim	= &m_BotInfo_MountAim;
	m_pBotInfo_Walk		= &m_BotInfo_Walk;
	m_pBotInfo_Jump		= NULL;
	m_pBotInfo_Weapon	= &m_BotInfo_Weapon;

	m_BotState = BOTSTATE_IDLE;
	m_BotAnimControl = ANIMCONTROL_STAND;
	m_BotAnimTap = ANIMTAP_STAND;

	m_bAttackTested = FALSE;
	m_fUnitBlendAttack = 0.0f;
	m_fUnitBlendDir = 0.0f;

	m_fLeechTurnTime = 0.0f;
	m_StartPosition.Zero();
	m_pBotLeeching = NULL;

	m_pTorsoBonePos = NULL;
	m_pJetAttachBonePos = NULL;
	
	for( u32 i = 0; i < _NUM_COLLISION_BONES; ++i ) {
		m_apLegTestBones[i] = NULL;
		m_aLegTestBonesPrev[i].Zero();
	}

	m_fJetIntensity = 0.0f;
	m_JetEmitterDir.Zero();
	m_hJetEmitter = FPARTICLE_INVALID_HANDLE;

	m_fCollCylinderRadius_WS = 5.0f;
	m_fCollCylinderHeight_WS = 15.0f;

	m_fMaxFlatSurfaceSpeed_WS = _MAX_VELOCITY_XZ;
	m_fMaxVerticalSpeed_WS = _MAX_VERT_VEL;

	m_SpinDeathZonePos_WS.Zero();

	m_pIdleEmitter = NULL;
	m_pSpinEmitter = NULL;
	m_pMagnetEmitter = NULL;
	m_fEmitterVolume = 0.0f;
	m_fEmitterVolumeDir = 0.0f;

	m_LagPoint.Zero();

	m_fNextRingTime = 0.0f;
	m_fNextShakeTime = 0.0f;
	m_pMagnetBot = NULL;
	m_MagnetDir_MS.Zero();
	m_LastBotPosition.Zero();

	m_anAnimStackIndex[ASI_RC_POWER_DOWN] = ANIMTAP_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP] = ANIMTAP_POWER_UP;

	m_bSkipAnimAdvance = FALSE;
}

BOOL CBotProbe::_SetupRings( void ) {
	if( m_bRingsSetup ) {
		return TRUE;
	}

	FResFrame_t Frame = fres_GetFrame();
	FMeshInit_t MeshInit;
	FMesh_t *pMesh;

	pMesh = (FMesh_t *) fresload_Load( FMESH_RESTYPE, _RING_MESH );

	if( pMesh == NULL ) {
		DEVPRINTF( "CBotProbe::InitSystem():  Unable to load mesh %s\n", _RING_MESH );
		goto _ExitInitEffectsWithError;
	}

	MeshInit.fCullDist = 200.0f;
	MeshInit.nFlags = 0;
	MeshInit.pMesh = pMesh;
	MeshInit.Mtx.Identity();

	for( u32 i = 0; i < _NUM_MAGNET_RINGS; ++i ) {
		m_aRings[i].fUnitTime = 0.0f;
		m_aRings[i].pMesh = NULL;
		m_aRings[i].pMesh = fnew CFWorldMesh;

		if( !m_aRings[i].pMesh ) {
			goto _ExitInitEffectsWithError;
		}

		m_aRings[i].pMesh->Init( &MeshInit );
		m_aRings[i].pMesh->SetCollisionFlag( FALSE );
		m_aRings[i].pMesh->m_nUser = NULL;
		m_aRings[i].pMesh->m_pUser = NULL;
		m_aRings[i].pMesh->SetUserTypeBits( 0 );
		m_aRings[i].pMesh->UpdateTracker();
		m_aRings[i].pMesh->SetCullDirection( FMESH_CULLDIR_NONE );
		m_aRings[i].pMesh->RemoveFromWorld();
	}

	m_bRingsSetup = TRUE;

	return TRUE;

	// Error:
_ExitInitEffectsWithError:
	fres_ReleaseFrame( Frame );
	return FALSE;
}

void CBotProbe::_DestroyRings( void ) {
	if( !m_bRingsSetup ) {
		return;
	}

	for( u32 i = 0; i < _NUM_MAGNET_RINGS; ++i ) {
		fdelete( m_aRings[i].pMesh );
		m_aRings[i].pMesh = NULL;
	}

	m_bRingsSetup = FALSE;
}

void CBotProbe::_InitSoundHandles( void ) {
	// So we only load stuff once
	if( m_hSounds[0] != FSNDFX_INVALID_FX_HANDLE ) {
		return;
	}

	if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_PROBE ) ) 
	{
		DEVPRINTF( "CBotProbe::_InitSoundHandles(): Could not load sound effect bank '%s'\n", _SOUND_BANK_PROBE );
		return; 
	}

	cchar *pszSoundTable[_SOUND_COUNT] =  {
		"SRMLattaimp",
		"SRMLattmiss",
		"SRMLdrain",
		"SRMLdrop",
		"SRMLhover",
		"SRMLspinatt",
		"SRMLsucks"
	};
	
	for( u32 i = 0; i < _SOUND_COUNT; ++i ) {
		m_hSounds[i] = fsndfx_GetFxHandle( pszSoundTable[i] );

		if( m_hSounds[i] == FSNDFX_INVALID_FX_HANDLE ) {
			DEVPRINTF( "CBotProbe::_InitSoundHandles(): Could not load sound '%s'\n", pszSoundTable[i] );
		}
	}
}


void CBotProbe::_UpdateMatrices( void ) {
	CFMtx43A EntityMtxToWorld;

	if( !IsDeadOrDying() ) {
		CFMtx43A RotMtx, AboutMtx;
		CFVec3A TorsoMS;

		// Save the previous bone positions
		for( u32 i = 0; i < _NUM_COLLISION_BONES; ++i ) {
			m_aLegTestBonesPrev[i] = *m_apLegTestBones[i];
		}

		TorsoMS.Sub( *m_pTorsoBonePos, m_MountPos_WS );

		// Bot's yaw
		CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
		CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

		// Build rotation matrix from our lag point
		m_LagPoint.y = (*m_pTorsoBonePos).y + _LAG_POINT_Y_POS;

		RotMtx.m_vUp.Sub( *m_pTorsoBonePos, m_LagPoint ).Unitize();
		RotMtx.m_vRight.Cross( RotMtx.m_vUp, CFVec3A::m_UnitAxisZ );
		
		if( RotMtx.m_vRight.MagSq() < 0.001f ) {
			RotMtx.Identity();
		} else {
			RotMtx.m_vFront.Cross( RotMtx.m_vRight, RotMtx.m_vUp ).Unitize();
		}

		RotMtx.m_vPos.Set( TorsoMS );
		CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( RotMtx.m_vPos );

		AboutMtx.Mul( RotMtx, CFMtx43A::m_Xlat );

		// Rotate the offset matrix by our yaw
		CFMtx43A::m_XlatRotY.m_vPos.Zero();
		AboutMtx.Mul( CFMtx43A::m_XlatRotY );

		CFMtx43A::m_Xlat.m_vPos = m_MountPos_WS;
		EntityMtxToWorld.Mul( CFMtx43A::m_Xlat, AboutMtx );

	} else {
		EntityMtxToWorld.Identity();
		EntityMtxToWorld.m_vPos = m_MountPos_WS;
		//CFQuatA qRot;
		//qRot.BuildQuat( m_vDeathRotAxis, _DEATH_ROTATION_RATE * FLoop_fPreviousLoopSecs );
		//qRot.MulPoint( EntityMtxToWorld.m_vFront,	m_MtxToWorld.m_vFront );
		//qRot.MulPoint( EntityMtxToWorld.m_vUp,		m_MtxToWorld.m_vUp );
		//qRot.MulPoint( EntityMtxToWorld.m_vRight,	m_MtxToWorld.m_vRight );
		//EntityMtxToWorld.m_vPos = m_MountPos_WS;
	}

	m_pWorldMesh->m_Xfm.BuildFromMtx( EntityMtxToWorld );
	m_pWorldMesh->UpdateTracker();

	// Update the direction of the jet emitter
	m_JetEmitterDir = EntityMtxToWorld.m_vUp;
	m_JetEmitterDir.Mul( -1.0f );

	PROTRACK_BEGINBLOCK( "ComputeMtxPal" );
		// Update bone matrix palette...
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

	if( !IsDeadOrDying() ) {
		EntityMtxToWorld.Identity();
		EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
		EntityMtxToWorld.m_vPos = m_MountPos_WS;
	}

	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
}

void CBotProbe::_SpawnBotParts( const CFVec3A *pPos, const u8 &uMin, const u8 &uMax ) {
	if( !m_pDebrisGroup ) {
		return;
	}

	// Throw out some bot parts
	CFDebrisSpawner DebrisSpawner;
	DebrisSpawner.InitToDefaults();

	DebrisSpawner.m_Mtx.m_vPos = *pPos;
	DebrisSpawner.m_Mtx.m_vZ = CFVec3A::m_UnitAxisY;
	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
	DebrisSpawner.m_pDebrisGroup = m_pDebrisGroup;
	DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
	DebrisSpawner.m_fMinSpeed = 15.0f;
	DebrisSpawner.m_fMaxSpeed = 25.0f;
	DebrisSpawner.m_fUnitDirSpread = 0.75f;
	DebrisSpawner.m_fScaleMul = 1.0f;
	DebrisSpawner.m_fGravityMul = 1.0f;
	DebrisSpawner.m_fRotSpeedMul = 1.0f;
	DebrisSpawner.m_nMinDebrisCount = uMin;
	DebrisSpawner.m_nMaxDebrisCount = uMax;
	DebrisSpawner.m_pFcnCallback = NULL;

	CGColl::SpawnDebris( &DebrisSpawner );
}

void CBotProbe::_HandleHover( void ) {
	BOOL bAccelApplied = FALSE;

	// Don't move when leeching successfully, screws the up the animation
	if( ( m_BotState == BOTSTATE_LEECH_TURN || m_BotState == BOTSTATE_LEECHING )) {
		return;
	}

	if( m_fControls_FlyUp < 0.0f ) {
		m_Velocity_WS.y += ( m_fControls_FlyUp * _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs );
		bAccelApplied = TRUE;

		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	if( m_fControls_FlyUp > 0.0f ) {
		m_Velocity_WS.y += ( m_fControls_FlyUp * _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs );
		bAccelApplied = TRUE;

		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	if( !bAccelApplied ) {
		if( m_Velocity_WS.y > 0.0f ) {
			m_Velocity_WS.y -= _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs * 1.5f;
			FMATH_CLAMP_MIN0( m_Velocity_WS.y );
		} else if( m_Velocity_WS.y < 0.0f ) {
			m_Velocity_WS.y += _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs * 1.5f;
			FMATH_CLAMPMAX( m_Velocity_WS.y, 0.0f );
		}
	}
}

void CBotProbe::_HandleTranslation( void ) {
	// Don't move when leeching, screws the up the animation
	if( IsAttackLeechEnabled() ) {
		if( m_BotState == BOTSTATE_LEECH_TURN || m_BotState == BOTSTATE_LEECHING ) {
			return;
		}
	}

	CFVec3A vDesVel_WS;
	CFVec3A vDeltaVel_WS;
	f32		fMaxAccelThisFrame = _MAX_ACCEL_XZ * FLoop_fPreviousLoopSecs;

	vDesVel_WS.Mul( m_XlatStickNormVecXZ_WS, _MAX_VELOCITY_XZ );
	vDeltaVel_WS.Sub( vDesVel_WS, m_Velocity_WS );
	vDeltaVel_WS.y = 0.0f;

	if( vDeltaVel_WS.Dot( m_Velocity_WS ) < 0.0f ) {
		fMaxAccelThisFrame *= _DECEL_MULT_XZ;
	}

	if( vDeltaVel_WS.MagSq() < ( fMaxAccelThisFrame * fMaxAccelThisFrame ) ) {
		m_Velocity_WS.x = vDesVel_WS.x;
		m_Velocity_WS.z = vDesVel_WS.z;
	} else  {
		vDeltaVel_WS.Unitize().Mul( fMaxAccelThisFrame );
        m_Velocity_WS.Add( vDeltaVel_WS );     
	}

	WS2MS( m_Velocity_MS, m_Velocity_WS );
	VelocityHasChanged();

	m_fJetIntensity = m_Velocity_MS.MagXZ() * fmath_Inv( _MAX_VELOCITY_XZ );
	FMATH_CLAMP( m_fJetIntensity, 0.0f, 1.0f );

	// Update our sound emitters
	if( m_pIdleEmitter ) {
		m_pIdleEmitter->SetPosition( m_pTorsoBonePos );

		if( m_fJetIntensity < _IDLE_VOLUME_MIN ) {
			m_pIdleEmitter->SetVolume( _IDLE_VOLUME_MIN );
		} else {
			m_pIdleEmitter->SetVolume( m_fJetIntensity );
		}
	}

	if( m_pSpinEmitter ) {
		m_pSpinEmitter->SetPosition( m_pTorsoBonePos );
	}

	if( m_pMagnetEmitter ) {
		m_pMagnetEmitter->SetPosition( m_pTorsoBonePos );
	}
}

void CBotProbe::_HandleCollision( void ) {
	m_CollSphere.m_Pos = (*m_pTorsoBonePos).v3;
	m_CollSphere.m_fRadius	= m_pBotInfo_Gen->fCollSphere1Radius_MS;
	m_uCollisionStatus = COLLSTATE_NO_COLLISION;

	fcoll_Clear();

	m_CollInfo.nCollTestType			= FMESH_COLLTESTTYPE_SPHERE;
	m_CollInfo.nStopOnFirstOfCollMask   = FCOLL_MASK_NONE;
	m_CollInfo.bFindClosestImpactOnly	= FALSE;
	m_CollInfo.nCollMask				= (m_nPossessionPlayerIndex < 0) ? FCOLL_MASK_COLLIDE_WITH_NPCS : FCOLL_MASK_COLLIDE_WITH_PLAYER;
	m_CollInfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_CollInfo.pSphere_WS				= &m_CollSphere;
	m_CollInfo.pTag						= NULL;

	// first collide with tracker type things
	m_pCollBot = this;
	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();
	
	CFWorldUser UserTracker;
	UserTracker.MoveTracker( m_CollSphere );
	UserTracker.FindIntersectingTrackers( _TrackerCollisionCallback, FWORLD_TRACKERTYPE_MESH );

	fworld_CollideWithWorldTris( &m_CollInfo, m_pWorldMesh );

	if( FColl_nImpactCount ) {
		fcoll_Sort( FALSE );
	
		_HandleCollisionResponse();
	} else {
		CFVec3A Down = CFVec3A::m_UnitAxisY;

		Down.Mul( _GROUND_SPHERE_DROP_DISTANCE );

		m_CollSphere.m_Pos = (*m_pTorsoBonePos).v3;
		m_CollSphere.m_Pos.Add( Down.v3 );

		FWorld_nTrackerSkipListCount = 0;
		AppendTrackerSkipList();
	
		UserTracker.MoveTracker( m_CollSphere );
		UserTracker.FindIntersectingTrackers( _TrackerCollisionCallback, FWORLD_TRACKERTYPE_MESH );

		fworld_CollideWithWorldTris( &m_CollInfo, m_pWorldMesh );

		if( FColl_nImpactCount ) {
			fcoll_Sort( FALSE );
			m_uCollisionStatus = COLLSTATE_FLOOR;
	
			_HandleCollisionResponse();
		}
	}

	if( m_nBotFlags & BOTFLAG_IMMOBILIZE_PENDING ) {
		if( m_fSpeed_WS == 0.0f ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
		}
	}
}

void CBotProbe::_HandleCollisionResponse( void ) {
	CFVec3A ImpactToPos_WS, PushAmount_WS;
	FCollImpact_t *pImpact;
	f32 fDot;

	for( u32 i = 0; i < FColl_nImpactCount; ++i ) {
		pImpact = FColl_apSortedImpactBuf[i];

		ImpactToPos_WS.Sub( m_CollSphere.m_Pos, pImpact->aTriVtx[0] );
		fDot = ImpactToPos_WS.Dot( pImpact->UnitFaceNormal );
		if( fDot <= 0.0f ) {
			continue;
		}

		fDot = m_Velocity_WS.Dot( pImpact->UnitFaceNormal );
		if( fDot > 0.0f ) {
			continue;
		}

		fDot = m_Velocity_WS.Dot( pImpact->PushUnitVec );
		if( fDot > 0.0f ) {
			continue;
		}

		PushAmount_WS.x = pImpact->PushUnitVec.x * pImpact->fImpactDistInfo;
		PushAmount_WS.y = pImpact->PushUnitVec.y * pImpact->fImpactDistInfo;
		PushAmount_WS.z = pImpact->PushUnitVec.z * pImpact->fImpactDistInfo;

		m_MountPos_WS.Add( PushAmount_WS );

		CFVec3A Right, Front, PushUnitVec;
		PushUnitVec.Set( pImpact->PushUnitVec );
		Right.Cross( PushUnitVec, m_Velocity_WS );
		Front.Cross( Right, PushUnitVec );
		m_Velocity_WS = Front;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}
}

void CBotProbe::_HandleAttacks( void ) {
	// Can only attack if we don't have anything else going on
	if( m_BotAnimControl == ANIMCONTROL_STAND ) {
		if( m_fControls_Fire2 != 0.0f ) {
			m_fUnitBlendAttack = 0.0f;
			m_fUnitBlendDir = 1.0f;

			m_bAttackTested = FALSE;

			m_BotState = BOTSTATE_LEECH_ATTACK;
			m_BotAnimControl = ANIMCONTROL_ATTACK_LEECH;
			m_BotAnimTap = ANIMTAP_ATTACK_LEECH;

			ZeroTime( m_BotAnimTap );
			SetControlValue( m_BotAnimControl, 0.0f );
		}

		if( m_BotState == BOTSTATE_IDLE && m_fControls_Fire1 != 0.0f ) {
			m_fUnitBlendAttack = 0.0f;
			m_fUnitBlendDir = 1.0f;

			m_BotState = BOTSTATE_SPIN_ATTACK;
			m_BotAnimControl = ANIMCONTROL_ATTACK_SPIN;
			m_BotAnimTap = ANIMTAP_ATTACK_SPIN;

			ZeroTime( m_BotAnimTap );
			SetControlValue( m_BotAnimControl, 0.0f );

			m_fEmitterVolume = 0.0f;
			m_fEmitterVolumeDir = 1.0f;
			m_pSpinEmitter = FSNDFX_ALLOCNPLAY3D( m_hSounds[_SOUND_ATTACK_SPIN_LOOP], m_pTorsoBonePos, _SOUND_RADIUS_SPIN, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );

			if( m_pSpinEmitter ) {
				m_pSpinEmitter->SetVolume( 0.0f );
			}
		}
	} 

	// Update the spin death zone hit position
	m_SpinDeathZonePos_WS.x = m_BotInfo_Probe.fSpinDeathZoneX_MS;
	m_SpinDeathZonePos_WS.y = m_BotInfo_Probe.fSpinDeathZoneY_MS;
	m_SpinDeathZonePos_WS.z = m_BotInfo_Probe.fSpinDeathZoneZ_MS;
	MS2WS( m_SpinDeathZonePos_WS );
	m_SpinDeathZonePos_WS.Add( m_MountPos_WS );
}

void CBotProbe::_HandleAttackAnimation( void ) {
	if( m_BotState == BOTSTATE_LEECH_TURN ) {
		if( _HandleLeechTurn() ) {
			return;
		}
	}

	f32 fUnitTime;

	// We our on an animation other than standing
	if( m_BotAnimControl != ANIMCONTROL_STAND ) {
		m_fUnitBlendAttack += ( m_fUnitBlendDir * FLoop_fPreviousLoopSecs );
		FMATH_CLAMP_UNIT_FLOAT( m_fUnitBlendAttack );

		// Time to blend away from the animation
		if( m_fUnitBlendAttack == 1.0f && GetUnitTime( m_BotAnimTap ) >= 1.0f ) {
			m_fUnitBlendDir = -1.0f;

			// Handle things that need to be done at the end of the animation
			switch( m_BotAnimControl ) {
				case ANIMCONTROL_ATTACK_SPIN:
					if( m_pSpinEmitter ) {
						m_pSpinEmitter->Destroy();
						m_pSpinEmitter = NULL;
					}
				break;
			}
		}

		fUnitTime = GetUnitTime( m_BotAnimTap );

		// Handle things that need to be done every frame for animations
		switch( m_BotAnimControl ) {
			case ANIMCONTROL_ATTACK_LEECH:
				if( m_pBotLeeching ) {
					m_fUnitBlendDir = 1.0f;
				}

				// Only try to grab if we don't have anything yet and are animating towards the grab
				/*if( fUnitTime >= 0.80f && !m_pBotLeeching && m_fUnitBlendDir > 0.0f ) {
					_LeechGrabTest();
				}*/
				if( !m_bAttackTested && fUnitTime >= 1.0f ) {
					_LeechGrabTest();

					if( !m_pBotLeeching ) {
						fsndfx_Play3D( m_hSounds[_SOUND_ATTACK_MISS], &m_MountPos_WS, _SOUND_RADIUS_MISS );
						fparticle_SpawnEmitter( m_hSparkParticleDef, (*m_apLegTestBones[0]).v3, &CFVec3A::m_UnitAxisY.v3, 1.0f );
					}
				}
			break;

			case ANIMCONTROL_ATTACK_SPIN:
				m_fEmitterVolume += ( FLoop_fPreviousLoopSecs * m_fEmitterVolumeDir * _EMITTER_VOLUME_TIMESCALE );

				// !!Nate Use volume min and max from above?
				FMATH_CLAMP( m_fEmitterVolume, 0.0f, 1.0f );

				// If we are blending down, don't try to collide, this can happen if we abort the attrack
				if(  fUnitTime > 0.2f && fUnitTime < 0.9f && m_fUnitBlendDir != -1.0f ) {
					_SpinHitTest();
				}

				if( ( GetTotalTime( m_BotAnimTap ) - GetTime( m_BotAnimTap ) ) <= _OO_EMITTER_VOLUME_TIMESCALE ) {
					m_fEmitterVolumeDir = -1.0f;
				}

				if( m_pSpinEmitter ) {
					m_pSpinEmitter->SetVolume( m_fEmitterVolume );
				}
			break;
		}

		SetControlValue( m_BotAnimControl, m_fUnitBlendAttack );

		if( !m_bSkipAnimAdvance ) {
			DeltaTime( m_BotAnimTap, FLoop_fPreviousLoopSecs, TRUE );
		}

		if( m_fUnitBlendAttack == 0.0f && m_fUnitBlendDir == -1.0f && m_BotState != BOTSTATE_IDLE ) {
			SetControlValue( m_BotAnimControl, 0.0f );
			
			m_BotState = BOTSTATE_IDLE;
			m_BotAnimControl = ANIMCONTROL_STAND;
			m_BotAnimTap = ANIMTAP_STAND;
		}
	}
}

BOOL CBotProbe::_HandleLeechTurn( void ) { 
	f32 fNewYaw;
	f32 fUnitTime = ( m_fLeechTurnTime * _OO_LEECH_TURN_TIME_MAX );
	CFVec3A Dir;
	CFVec3A Front = m_pBotLeeching->MtxToWorld()->m_vFront;

	fNewYaw = FMATH_FPOT( fUnitTime, m_fMountYaw_WS, ( m_pBotLeeching->m_fMountYaw_WS /*+ m_pBotLeeching->m_fLegsYaw_MS*/ ) );
	ChangeMountYaw( fNewYaw );

	Dir.Sub( m_pBotLeeching->MtxToWorld()->m_vPos, m_StartPosition );
	Dir.Mul( fUnitTime );

	m_MountPos_WS.Add( m_StartPosition, Dir );

	m_fLeechTurnTime += FLoop_fPreviousLoopSecs;

	FMATH_CLAMPMAX( m_fLeechTurnTime, _LEECH_TURN_TIME_MAX );

	if( m_fLeechTurnTime == _LEECH_TURN_TIME_MAX ) {
		if( !m_pBotLeeching->IsDeadOrDying() ) {
			m_MountPos_WS = m_pBotLeeching->MtxToWorld()->m_vPos;
			m_MountPos_WS.y -= 0.5f;

			ZeroTime( m_BotAnimTap );
			SetControlValue( m_BotAnimControl, 0.0f );

			m_BotState = BOTSTATE_LEECHING;
			m_BotAnimControl = ANIMCONTROL_ATTACK_FLING;
			m_BotAnimTap = ANIMTAP_ATTACK_FLING;

			m_fUnitBlendDir = 1.0f;
			ZeroTime( m_BotAnimTap );
			SetControlValue( m_BotAnimControl, 1.0f );

			m_pBotLeeching->EnableProbeSwing( this );

			if( m_pBotLeeching->m_nPossessionPlayerIndex < 0 ) {
				fsndfx_Play3D( m_hSounds[_SOUND_ATTACK_FLING], &m_pBotLeeching->MtxToWorld()->m_vPos, _SOUND_RADIUS_FLING );
			} else {
				fsndfx_Play2D( m_hSounds[_SOUND_ATTACK_FLING] );
			}

			m_pBotLeeching = NULL;
			
			m_bSkipAnimAdvance = TRUE;

			return TRUE;
		} else {
			// If we get here, bot is dead.  Don't go onto the swing, just bail out
			AbortAttack();
		}
	}

	return FALSE;
}

void CBotProbe::_LeechGrabTest( void ) {
	CFSphere Sphere;

	Sphere.m_Pos = m_MountPos_WS.v3;
	Sphere.m_Pos.x += m_BotInfo_Probe.fGrabCollisionSphereBumpX;
	Sphere.m_Pos.y += m_BotInfo_Probe.fGrabCollisionSphereBumpY;
	Sphere.m_Pos.z += m_BotInfo_Probe.fGrabCollisionSphereBumpZ;
	Sphere.m_fRadius = m_BotInfo_Probe.fGrabCollisionSphereRadius;

	if( m_pMagnetBot ) {
		CFVec3A NewPoint;
		CFMtx43A About;

		m_LagPoint.y = (*m_pTorsoBonePos).y + _LAG_POINT_Y_POS;

		CFMtx43A::m_Temp.m_vPos.Zero();
		CFMtx43A::m_Temp.m_vUp.Sub( *m_pTorsoBonePos, m_LagPoint ).Unitize();
		CFMtx43A::m_Temp.m_vRight.Cross( CFMtx43A::m_Temp.m_vUp, CFVec3A::m_UnitAxisZ );

		if( CFMtx43A::m_Temp.m_vRight.MagSq() < 0.001f ) {
			CFMtx43A::m_Temp.Identity();
		} else {
			CFMtx43A::m_Temp.m_vFront.Cross( CFMtx43A::m_Temp.m_vRight, CFMtx43A::m_Temp.m_vUp );
		}

		CFMtx43A::m_Temp.m_vPos.Set( *m_pTorsoBonePos );
		CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( CFMtx43A::m_Temp.m_vPos );
		About.Mul( CFMtx43A::m_Temp, CFMtx43A::m_Xlat );

		About.MulPoint( NewPoint.v3, Sphere.m_Pos );
		Sphere.m_Pos = NewPoint.v3;
	}

	m_CollInfo.nCollTestType			= FMESH_COLLTESTTYPE_SPHERE;
	m_CollInfo.nStopOnFirstOfCollMask   = FCOLL_MASK_CHECK_ALL;
	m_CollInfo.bFindClosestImpactOnly	= FALSE;
	m_CollInfo.nCollMask				= FCOLL_MASK_CHECK_ALL;
	m_CollInfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_CollInfo.pSphere_WS				= &Sphere;
	m_CollInfo.pTag						= NULL;

	m_pBotLeeching = NULL;
	m_pThisBot = this;
	m_bAttackTested = TRUE;

	// We got him
	if( !fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_MESH, _GrabCollisionCallback ) ) {
		FASSERT( m_pBotLeeching ); // This better be non-NULL

		_DisableMagnet();

		// Don't move Probe anymore
		ZeroVelocity();
		m_LagPoint = m_MountPos_WS;
		m_fLeechTurnTime = 0.0f;
		m_StartPosition = m_MountPos_WS;

		m_BotState = BOTSTATE_LEECH_TURN;

		// I don't actually tell the bot he is grabbed yet since we still need to animate to our final position
		// Once we are done animating, the bot is told that he was grabbed
		m_pBotLeeching->PreEnableProbeSwing();
		m_pBotLeeching->ImmobilizeBot();

		CFVec3A Shoot = *m_pBotLeeching->m_pApproxEyePoint_WS;

		// Spawn some sparks
		fparticle_SpawnEmitter( m_hSparkParticleDef, Shoot.v3, &CFVec3A::m_UnitAxisX.v3, 1.0f );
		fparticle_SpawnEmitter( m_hSparkParticleDef, Shoot.v3, &CFVec3A::m_UnitAxisY.v3, 1.0f );
		fparticle_SpawnEmitter( m_hSparkParticleDef, Shoot.v3, &CFVec3A::m_UnitAxisZ.v3, 1.0f );

		// Damage the bot
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_AMBIENT;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile	= m_apDamageProfile[_DAMAGE_PINCH];
			pDamageForm->m_Damager.pWeapon	= NULL;
			pDamageForm->m_Damager.pBot		= this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_pDamageeEntity	= m_pBotLeeching;

			CDamage::SubmitDamageForm( pDamageForm );
		}

		// Play a sound 
		if( m_pBotLeeching->m_nPossessionPlayerIndex < 0 ) {
			fsndfx_Play3D( m_hSounds[_SOUND_ATTACK_IMPACT], &m_pBotLeeching->MtxToWorld()->m_vPos, _SOUND_IMPACT_RADIUS );
		} else {
			fsndfx_Play2D( m_hSounds[_SOUND_ATTACK_IMPACT] );
		}

		// Toss out some parts
		_SpawnBotParts( m_pBotLeeching->m_pApproxEyePoint_WS, _BOT_PART_NUM_MIN, _BOT_PART_NUM_MAX );
	}
}

void CBotProbe::_SpinHitTest( void ) {
	CFTrackerCollideProjSphereInfo CollProjSphereInfo;

	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.bFindClosestImpactOnly = FALSE;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	m_CollInfo.bCullBacksideCollisions = TRUE;
	m_CollInfo.bCalculateImpactData = TRUE;
	m_CollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	m_CollInfo.pTag = NULL;

	CollProjSphereInfo.pProjSphere = &m_CollInfo.ProjSphere;
	CollProjSphereInfo.bIgnoreCollisionFlag = FALSE;
	CollProjSphereInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	CollProjSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	CollProjSphereInfo.pCallback = _TrackersCallbackProjSphere;
	CollProjSphereInfo.nTrackerSkipCount = 0;
	CollProjSphereInfo.ppTrackerSkipList = NULL;


	m_pThisBot = this;

	fcoll_Clear();

	for( u32 i = 0; i < _NUM_COLLISION_BONES; ++i ) {
		m_CollInfo.ProjSphere.Init( &m_aLegTestBonesPrev[i], m_apLegTestBones[i], m_BotInfo_Probe.fSpinCollisionSphereRadius );
		fworld_CollideWithTrackers( &CollProjSphereInfo, m_pThisBot->m_pWorldMesh );
	}
	m_pThisBot = NULL;
}

void CBotProbe::_ReleaseRings( void ) {
	// Give back any rings we may have
	while( flinklist_GetHead( &m_MagnetRings ) ) {
		_FreeRing( (_MagnetRing_t *) flinklist_RemoveHead( &m_MagnetRings ) );
	}
}

void CBotProbe::_DisableMagnet( void ) {
	if( m_pMagnetBot ) {
		m_pMagnetBot->SetStickModifier_MS( NULL );
		m_pMagnetBot = NULL;
	}

	if( m_pMagnetEmitter ) {
		m_pMagnetEmitter->Destroy();
		m_pMagnetEmitter = NULL;
	}
}

void CBotProbe::_UpdateMagnet( void ) { 
	m_MagnetDir_MS.Zero();

	if( !m_pMagnetBot || m_pMagnetBot->IsImmobileOrPending() ) {
		return;
	}

	if( (m_BotState == BOTSTATE_LEECH_TURN || m_BotState == BOTSTATE_LEECHING )) {	 //got ya
		return;
	}


	// Don't move dead ones, they are no good
	if( m_pMagnetBot->IsDeadOrDying() ) {
		_DisableMagnet();
		return;
	}

	if( FMATH_FABS( m_MountPos_WS.y - m_pMagnetBot->MtxToWorld()->m_vPos.y ) > m_BotInfo_Probe.fMagnetYRange ) {
		return;
	}

	CFVec3A ProbeToBot_XZ;
	f32 fDis, fUnitDis;

	ProbeToBot_XZ.Sub( m_pMagnetBot->MtxToWorld()->m_vPos, m_MountPos_WS );
	ProbeToBot_XZ.y = 0.0f;

	if( ProbeToBot_XZ.MagSqXZ() <= 0.001f ) {
		return;
	}

	m_MagnetDir_MS.Zero();

	fDis = ProbeToBot_XZ.UnitAndMagXZ( ProbeToBot_XZ );

	// Are we close enough to bother the bot
	if( fDis < m_BotInfo_Probe.fMagnetOuterRadius ) {

		if( fDis < 2.0f ) {
			return;
		}

		m_fNextRingTime -= FLoop_fPreviousLoopSecs;

		fUnitDis = fDis * m_BotInfo_Probe.fOOMagnetOuterRadius;
		fUnitDis = 1.0f - fUnitDis;

		if( m_fNextRingTime < 0.0f ) {
			_MagnetRing_t *pRing = NULL;

			pRing = _GetRing();
			
			if( pRing ) {
				m_fNextRingTime = _RING_TIME;
				pRing->fUnitTime = 0.0f;
				pRing->pMesh->AddToWorld();
				flinklist_AddTail( &m_MagnetRings, pRing );
			}
		}

		ProbeToBot_XZ.UnitizeXZ();
		ProbeToBot_XZ.Mul( fUnitDis );
		
		// Swing towards the bot
		m_LagPoint.Add( ProbeToBot_XZ );

		// Make it go in the other direction
		ProbeToBot_XZ.Mul( -1.0f );

		m_pMagnetBot->WS2MS( m_MagnetDir_MS, ProbeToBot_XZ );


		m_fNextShakeTime -= FLoop_fPreviousLoopSecs;

		if( m_fNextShakeTime < 0.0f ) {
			m_pMagnetBot->ShakeCamera( ( 1.0f - fUnitDis ) * 0.5f, _RING_SHAKE_DURATION );
			m_fNextShakeTime = _SHAKE_TIME;
		}
	}
}

void CBotProbe::_UpdateActiveRings( void ) {
	_MagnetRing_t *pCurrent = (_MagnetRing_t *) flinklist_GetHead( &m_MagnetRings );
	_MagnetRing_t *pNext = NULL;

	if( !pCurrent ) {
		return;
	}

	CFVec3A BotPt_WS, Dir_WS;
	
	if( m_pMagnetBot ) {
		BotPt_WS = CFVec3A::m_UnitAxisY;
		BotPt_WS.Mul( m_pMagnetBot->m_fCollCylinderHeight_WS * 0.5f );
		BotPt_WS.Add( m_pMagnetBot->MtxToWorld()->m_vPos );
		m_LastBotPosition = BotPt_WS;

		Dir_WS.Sub( BotPt_WS, *m_pJetAttachBonePos );
		Dir_WS.Mul( -1.0f );

		if( Dir_WS.MagSq() > 0.0001f ) {
			Dir_WS.Unitize();
		} else {
			Dir_WS = CFVec3A::m_UnitAxisY;
		}
	} else {
		Dir_WS.Sub( m_LastBotPosition, *m_pJetAttachBonePos );
		Dir_WS.Mul( -1.0f );

		if( Dir_WS.MagSq() > 0.0001f ) {
			Dir_WS.Unitize();
		} else {
			Dir_WS = CFVec3A::m_UnitAxisY;
		}

		BotPt_WS = m_LastBotPosition;
	}

	while( pCurrent ) {
		pNext = (_MagnetRing_t *) flinklist_GetNext( &m_MagnetRings, pCurrent );

		pCurrent->fUnitTime += ( FLoop_fPreviousLoopSecs * _RING_TIME_SCALE );
		FMATH_CLAMPMAX( pCurrent->fUnitTime, 1.0f );

		if( pCurrent->fUnitTime == 1.0f ) {
			flinklist_Remove( &m_MagnetRings, pCurrent );

			_FreeRing( pCurrent );
			pCurrent = pNext;

			continue;
		}

		CFMtx43A::m_Temp.m_vFront = Dir_WS;
		CFMtx43A::m_Temp.m_vRight.CrossYWithVec( CFMtx43A::m_Temp.m_vFront );

		if( CFMtx43A::m_Temp.m_vRight.MagSq() > 0.0001f ) {
			CFMtx43A::m_Temp.m_vRight.Unitize();
		} else {
			CFMtx43A::m_Temp.m_vRight = CFVec3A::m_UnitAxisX;
		}

		CFMtx43A::m_Temp.m_vUp.Cross( CFMtx43A::m_Temp.m_vFront, CFMtx43A::m_Temp.m_vRight );
		CFMtx43A::m_Temp.m_vPos.Lerp( pCurrent->fUnitTime, BotPt_WS, *m_pJetAttachBonePos );

		pCurrent->pMesh->SetMeshAlpha( FMATH_FPOT( 1.0f - pCurrent->fUnitTime, 0.0f, 0.2f ) );
		pCurrent->pMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, FMATH_MAX( 1.0f - pCurrent->fUnitTime, 0.001f ) );
		pCurrent->pMesh->UpdateTracker();

		pCurrent = pNext;
	}
}

_MagnetRing_t *CBotProbe::_GetRing( void ) {
	return (_MagnetRing_t * ) flinklist_RemoveHead( &m_FreeRingList );
}

void CBotProbe::_FreeRing( _MagnetRing_t *pRing ) {
	if( pRing->pMesh ) {
		pRing->pMesh->RemoveFromWorld();
	}

	flinklist_AddTail( &m_FreeRingList, pRing );
}

BOOL CBotProbe::_TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode ) {
	CBot *pBot = (CBot *) pTracker->m_pUser;

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT  &&
			((CEntity *)pTracker->m_pUser) != m_pThisBot) {
			
			if( !( pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
				return TRUE;
			}
//			if( ( pBot->TypeBits() & ENTITY_BIT_BOTPROBE ) ) {
//				return TRUE;
//			}
			if( pBot->IsDeadOrDying() ) {
				return TRUE;
			}

			m_CollInfo.pTag = pTracker;

			if( ((CFWorldMesh *)pTracker)->CollideWithMeshTris( &m_CollInfo ) ) {
				// Camera shake
				pBot->ShakeCamera( _SHAKE_UNIT_INTENSITY, _SHAKE_DURATION );

				// Damage the bot
				CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

				if( pDamageForm ) {
					// Fill out the form...
					pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_AMBIENT;
					pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
					pDamageForm->m_pDamageProfile	= m_apDamageProfile[_DAMAGE_SPIN];
					pDamageForm->m_Damager.pWeapon	= NULL;
					pDamageForm->m_Damager.pBot		= m_pThisBot;
					pDamageForm->m_Damager.nDamagerPlayerIndex = m_pThisBot ? m_pThisBot->m_nPossessionPlayerIndex : -1;
					pDamageForm->m_pDamageeEntity	= pBot;

					CDamage::SubmitDamageForm( pDamageForm );
				}

				// Play a hit sound if we can
				if( ((CBotGlitch *) pBot )->GetProbeHitSoundTime() == 0.0f ) {		  //botglitch check is above
					((CBotGlitch *) pBot )->SetProbeHitSoundTime( _HIT_SOUND_TIME );

					if( pBot->m_nPossessionPlayerIndex < 0 ) {
						fsndfx_Play3D( m_hSounds[_SOUND_ATTACK_IMPACT], &pBot->MtxToWorld()->m_vPos, _SOUND_IMPACT_RADIUS );
					} else {
						fsndfx_Play2D( m_hSounds[_SOUND_ATTACK_IMPACT] );
					}
				}

				// Bump the bot a bit backwards
				CFVec3A Impulse = pBot->MtxToWorld()->m_vFront;

				Impulse.Mul( -3.0f );
				pBot->ApplyVelocityImpulse_WS( Impulse );

				// Some sparks
				fparticle_SpawnEmitter( m_hSparkParticleDef, FColl_aImpactBuf[0].ImpactPoint.v3, &CFVec3A::m_UnitAxisY.v3, fmath_RandomFloat() );

				fcoll_Clear();
			}
		}
	}

	return TRUE;
}


BOOL CBotProbe::_TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	return CBot::TrackerCollisionCallback( pTracker, pVolume );
}


#if 0
// use CBot::NewTrackerCollisionCallback
BOOL CBotProbe::_TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	u32 i;
	for( i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}


	CFSphere HisCollSphere_WS;
	CBot *pBot;
	CFVec3A HisToMySphereCenter_WS;
	f32 fDistFromHisSphereToMine_WS, fIntersectDist;

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

//	if( pWorldMesh->m_nFlags & FMESHINST_FLAG_DONT_DRAW ) {
		// Not being drawn...
//		return TRUE;
//	}

	if( !pWorldMesh->IsCollisionFlagSet() ) {
		// Not collidable...
		return TRUE;
	}

	for( i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
		if( (((CEntity*)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_WEAPON) && (((CWeapon*)pTracker->m_pUser)->GetOwner()) ) {

			// never collide with weapons attached to bots
			return TRUE;
		}
	}

	pBot = NULL;

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT ) {
			pBot = (CBot *)pTracker->m_pUser;
		}
	}
	
	if (pBot && pBot->m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_PILLBOX)
	{
		if (m_pThisBot->m_nBotFlags & BOTFLAG_GLUED_TO_PARENT)// in this case, we probably don't want to collide
		{
			return TRUE;
		}
		else
		{
			m_CollInfo.pTag = pTracker;	  //any impacts that are generated will be marked with this pTag
			pWorldMesh->CollideWithMeshTris( &m_CollInfo );
			return TRUE;
		}
	}

	if ( (pBot)	&&									     // if valid pointer
		(!(pBot->TypeBits() & ENTITY_BIT_SITEWEAPON)) && // and it's not a site weapon
		(pBot->IsDeadOrDying() || m_pCollBot->IsDeadOrDying()) ) // and a collision participant is dead/dying
	{  //no special bot-v-bot collision with dead or dying things
		return TRUE;
	}

	if( pBot && (pBot->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		// single sphere collision doesn't work well with vehicle shapes
		m_CollInfo.pTag = pTracker;
		pWorldMesh->CollideWithMeshTris( &m_CollInfo );
	}
	else if( pBot && pBot->m_pBotInfo_Gen ) 
	{
		// Bot...
		// Using MtxToWorld() since I don't have access to m_MountPos_WS
		HisCollSphere_WS.m_Pos.x = pBot->m_pBotInfo_Gen->fCollSphere1X_MS + pBot->MtxToWorld()->m_vPos.x;//pBot->m_MountPos_WS.x;
		HisCollSphere_WS.m_Pos.y = pBot->m_pBotInfo_Gen->fCollSphere1Y_MS + pBot->MtxToWorld()->m_vPos.x;//m_MountPos_WS.y;
		HisCollSphere_WS.m_Pos.z = pBot->m_pBotInfo_Gen->fCollSphere1Z_MS + pBot->MtxToWorld()->m_vPos.x;//m_MountPos_WS.z;
		HisCollSphere_WS.m_fRadius = pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;

		if( !m_CollSphere.IsIntersecting( HisCollSphere_WS ) ) {
			// No collision...
			return TRUE;
		}

		HisToMySphereCenter_WS.v3 = m_CollSphere.m_Pos - HisCollSphere_WS.m_Pos;
		fDistFromHisSphereToMine_WS = HisToMySphereCenter_WS.Mag();
		if( fDistFromHisSphereToMine_WS == 0.0f ) {
			return TRUE;
		}

		fIntersectDist = (HisCollSphere_WS.m_fRadius + m_CollSphere.m_fRadius) - fDistFromHisSphereToMine_WS;

		if( fIntersectDist < 0.0f ) {
			return TRUE;
		}

		// NKM - Don't need this, pushes Glitch through the ground
		/*HisToMySphereCenter_WS.Unitize();
		HisToMySphereCenter_WS.Mul( fIntersectDist );

		if( m_pCollBot->NotifyBotCollision( pBot ) ) {
			m_pCollBot->AdjustForCollision( &HisToMySphereCenter_WS );
		}*/

		if (pBot->AIBrain() && m_pCollBot->m_nPossessionPlayerIndex > -1)
		{
			aibrainman_RammedByNotify(pBot->AIBrain(), m_pCollBot);
		}

	} else {
		// Other type of object...
		m_CollInfo.pTag = pTracker;	  //any impacts that are generated will be marked with this pTag
		pWorldMesh->CollideWithMeshTris( &m_CollInfo );
//		FASSERT(FColl_nImpactCount > 0);
	}

	return TRUE;
}
#endif


BOOL CBotProbe::_GrabCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not a CEntity...
		return TRUE;
	}

	if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
		CBotGlitch *pGlitch = (CBotGlitch *) pTracker->m_pUser;

		// Don't try to leech if we are already in the process of being leeched
		if( pGlitch->IsLeeched() ) {
			return TRUE;
		}

		// If they are immobile already, leave them alone
		if( pGlitch->IsImmobile() ) {
			return TRUE;
		}

		// Don't attack him if invincible
		if( pGlitch->IsInvincible() ) {
			return TRUE;
		}

		// Don't bother when in vehicle
		if( pGlitch->GetCurMech() ) {
			CBot *pBot = pGlitch->GetCurMech();

			if( ( pBot->TypeBits() & ENTITY_BIT_VEHICLE ) ) {
				return TRUE;
			}
		}

		if( ((CFWorldMesh *) pTracker)->CollideWithMeshTris( &m_CollInfo ) ) {
			m_pThisBot->m_pBotLeeching = pGlitch;

			return FALSE;
		}
	}

	return TRUE;
}


void CBotProbe::AppendTrackerSkipList( void ) {
	FASSERT( IsCreated() );

	CBot::AppendTrackerSkipList();

	FASSERT( (FWorld_nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	FWorld_apTrackerSkipList[FWorld_nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_pBotLeeching ) {
		m_pBotLeeching->AppendTrackerSkipList();
	}
}


void CBotProbe::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCollBot && m_pCollBot->TypeBits() & ENTITY_BIT_BOTPROBE );
	FASSERT( ((CBotProbe*)m_pCollBot)->m_pPartMgr && ((CBotProbe*)m_pCollBot)->m_pPartMgr->IsCreated() );

	if( !((CBotProbe*)m_pCollBot)->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
}












