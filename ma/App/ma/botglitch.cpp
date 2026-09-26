//////////////////////////////////////////////////////////////////////////////////////
// botglitch.cpp - 
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
// 02/26/03 Elliott     Cut & pasted from botblink.cpp
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botglitch.h"
#include "bot.h"
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
#include "RoboBuddy.h"
#include "meshentity.h"
#include "AI/AIEnviro.h"	//for AI sounds
#include "AI/AIGameUtils.h"	// for autils_DebugTrackRay
#include "fmesh_coll.h"
#include "zipline.h"
#include "gamesave.h"
#include "gamecam.h"
#include "botprobe.h"
#include "Hud2.h"
#include "vehicle.h"
#include "site_botWeapon.h"
#include "game.h"
#include "botfx.h"
#include "fcheckpoint.h"
#include "botpart.h"
#include "collectable.h"
#include "SpyVsSpy.h"
#include "ai/aiapi.h"
#include "ai/aibrainman.h"
#include "AI/aibrain.h"
#include "AI/AIBrainMems.h"
#include "MultiplayerMgr.h"
#include "sas_user.h"
#include "botAAGun.h"
#include "econsole.h"
#include "eswitch.h"
#include "edetpackdrop.h"
#include "letterbox.h"
#include "bartersystem.h"

//#include "miscdraw.h"


#define _DEATH_TIME 1.0f
#define _DEATH_DELAY_RESPAWN_TIME 1.5f
#define _DEATH_DELAY_RESPAWN_TIME_MP 2.0f



#define _HEADLIGHT_TEXTURE_ID			( 1 )
#define _BOTINFO_FILENAME				( "b_glitch" )
#define _BOTPART_FILENAME				( "bp_glitch" )
//#define _CHECKPOINT_PARTICLE_DEF_NAME	( "tfp_warp02" )
cchar* BotGlitch_pszSoundEffectBank		= "Glitch";
static cchar* _apszDMMeshFilenames[]	= {"grdggltch00", "Grdmgener00"}; 
static cchar* _pszMultiplayerMeshName	= "GRDGgltchMP";

//BOOL CBotGlitch::m_bColWithPieces=FALSE;

//#define _RECOIL_ROT_RIGHT_SHOULDER		( FMATH_DEG2RAD(-70.0f) )
#define _RECOIL_ROT_RIGHT_SHOULDER		( FMATH_DEG2RAD( 10.0f) )			// y axis now
#define _RECOIL_ROT_TORSO				( FMATH_DEG2RAD( 15.0f ) )
#define _RECOIL_ROT_LEFT_SHOULDER		( FMATH_DEG2RAD( 0.0f ) )
#define _RECOIL_ROT_LEFT_ELBOW			( FMATH_DEG2RAD( 5.0f ) )
#define _RECOIL_ROT_DUMMY				( FMATH_DEG2RAD( -2.0f ) )

#define _FIRE_ANIM_ON_TIME				( 0.1f )
#define _FIRE_ANIM_OO_ON_TIME			( 1.0f / _FIRE_ANIM_ON_TIME )
#define _FIRE_ANIM_HOLD_TIME			( 0.66f )
#define _FIRE_ANIM_READY_TIME			( 5.0f )
#define _FIRE_ANIM_OFF_TIME				( 1.0f )
#define _FIRE_ANIM_OO_OFF_TIME			( 1.0f / _FIRE_ANIM_OFF_TIME )
#define _FIRE_READY_BLEND				( 0.33f )

#define _ARM_SLERP_RATE					( 20.0f )
#define _RECOIL_ON_SPEED				( 15.0f )
#define _RECOIL_OFF_SPEED				( 2.0f )

#define _FIRE_LEAN_ANGLE				( FMATH_DEG2RAD( 15.0f ) )
#define _FIRE_LEAN_ON_RATE				( 5.0f )
#define _FIRE_LEAN_OFF_RATE				( 2.0f )

#define _TWO_HANDED_TORSO_TWIST_YAW		FMATH_DEG2RAD( 55.0f )
#define _MORTAR_TORSO_TWIST_YAW			FMATH_DEG2RAD( -63.5f )

#define _DAMAGE_PARTICLE_TIME_MIN		( 5.0f )
#define _DAMAGE_PARTICLE_TIME_MAX		( 20.0f )
#define _DAMAGE_ACCUM_INTERVAL			( 0.5f )
#define _DAMAGE_EFFECT_THRESHOLD		( 0.2f )			// if damage dealt in a short period of time over his remaining health is more than this value
															// glitch will be visibly damaged.
#define _DAMAGE_EFFECT_MIN				( 1 )
#define _DAMAGE_EFFECT_MAX				( 3 )
#define _EXTRA_DAMAGE_THRESHOLD			( 1.0f )
#define _DEATH_EXPLOSION				( "GlitchDeath" )
#define _DEATH_NUM_EXPLOSIONS			( 3 )
#define _DEATH_EXPOSION_CHANCE			( 1.0f/((f32)LIMB_TYPE_COUNT ) )

#define _ANTENNA_SEGMENTS				( 5 )
#define _ANTENNA_FREQUENCY				( 170.0f )
#define _ANTENNA_DAMPING				( 1.0f )
#define _ANTENNA_AMPLITUDE				( 25.0f )

static s32 _CalculateAmmoToDrop( CWeapon* pWeapon );

#define _RESPAWN_PARTICLE_INTENSITY		( 0.05f )
#define _RESPAWN_MESH_START_SCALE		( 1.0f )
#define _RESPAWN_MESH_FINISH_SCALE		( 3.0f )

static const float _fStunnedFromBeingThrownTime = .75f; // seconds

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotGlitchBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotGlitchBuilder *_pBotGlitchBuilder = NULL;// Starich - made dynamically allocated instead of static


void CBotGlitchBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTGLITCH, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotGlitch::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotGlitch::m_BotInfo_Gen.pszArmorProfile;

	m_anNPCWeaponType[0] = _NPC_WEAPON_LASER;			// NPC Glitchs default to a laser
	m_anNPCWeaponType[1] = _NPC_WEAPON_HAND;			// NPC Glitchs default to a hand in secondary slot

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotGlitchBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotGlitchBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotGlitch
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotGlitch::m_bSystemInitialized = FALSE;

CBotGlitch::BotInfo_Gen_t CBotGlitch::m_BotInfo_Gen;
CBotGlitch::BotInfo_MountAim_t CBotGlitch::m_BotInfo_MountAim;
CBotGlitch::BotInfo_Walk_t CBotGlitch::m_BotInfo_Walk;
CBotGlitch::BotInfo_Jump_t CBotGlitch::m_BotInfo_Jump;
CBotGlitch::BotInfo_Weapon_t CBotGlitch::m_BotInfo_Weapon;
CBotGlitch::BotInfo_Glitch_t CBotGlitch::m_BotInfo_Glitch;
CVehicleCamera::BotInfo_VehicleCamera_t CBotGlitch::m_BotInfo_GrabCamera;
CVehicleCamera::BotInfo_VehicleCamera_t CBotGlitch::m_BotInfo_ThrowCamera;
CBotGlitch::BotInfo_Idle_t* CBotGlitch::m_BotInfo_Idle=NULL;
u32 CBotGlitch::m_nBotInfo_Idles=0;

//FParticle_DefHandle_t CBotGlitch::m_hCheckpointParticleDef;

CBotAnimStackDef CBotGlitch::m_AnimStackDef;

CFAnimFrame CBotGlitch::m_Rocket1TorsoTwistQuat;
CFAnimFrame CBotGlitch::m_Rocket1TorsoUntwistQuat;
CFAnimFrame CBotGlitch::m_MortarTorsoTwistQuat;
CFAnimFrame CBotGlitch::m_MortarTorsoUntwistQuat;

CFVec3A CBotGlitch::m_GroinVecY_WS;
CFVec3A CBotGlitch::m_TorsoVecY_WS;						// Recorded torso matrice's Y vector (used in the animation callback)

CBotPartPool* CBotGlitch::m_pPartPool	= NULL;

FParticle_DefHandle_t	CBotGlitch::m_ahParticleDamage[DAMAGE_PARTICLE_COUNT];



BOOL CBotGlitch::InitSystem( void ) {
	FResFrame_t ResFrame;

	FASSERT( !m_bSystemInitialized );

	ResFrame = fres_GetFrame();

	_pBotGlitchBuilder = fnew CBotGlitchBuilder;
	if( !_pBotGlitchBuilder ) {
		DEVPRINTF( "CBotGlitch::InitSystem():  Unable to allocate a builder object.\n" );
		return FALSE;
	}

	m_bSystemInitialized = TRUE;

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitInitSystemWithError;
	}

	if( !ReadBotIdleFile( &m_BotInfo_Idle,&m_nBotInfo_Idles, _BOTINFO_FILENAME ) ) {
		goto _ExitInitSystemWithError;
	}
	if (m_nBotInfo_Idles) {
		m_apszIdleAnimNameTable = (cchar**) fres_Alloc( sizeof(cchar*) * m_nBotInfo_Idles);
		if (!m_apszIdleAnimNameTable)
			goto _ExitInitSystemWithError;
		
		m_apnEnableBoneNameIndexTableForEachIdleTap = (const u8**) fres_Alloc( sizeof(u8*) * m_nBotInfo_Idles);
		if (!m_apnEnableBoneNameIndexTableForEachIdleTap)
			goto _ExitInitSystemWithError;

		for (u32 uIndex=0;uIndex<m_nBotInfo_Idles;uIndex++)	{
			m_apszIdleAnimNameTable[uIndex] = m_BotInfo_Idle[uIndex].pszIdleName;
			m_apnEnableBoneNameIndexTableForEachIdleTap[uIndex]= m_aBoneEnableIndices_FullBody;
		}
	}

	for( u32 i=0; i<DAMAGE_PARTICLE_COUNT; i++ ) {
		m_ahParticleDamage[i] = (FParticle_DefHandle_t *)fresload_Load( FPARTICLE_RESTYPE, m_apszDamageParticles[i] );
		if( m_ahParticleDamage[i] == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CBotGlitch::InitSystem():  Unable to load particle %s\n", m_apszDamageParticles[i] );
		}
	}

	if( !_BuildAnimStackDef() ) {
		goto _ExitInitSystemWithError;
	}
	
	m_Rocket1TorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, _TWO_HANDED_TORSO_TWIST_YAW, CFVec3A::m_Null );
	m_Rocket1TorsoUntwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, -_TWO_HANDED_TORSO_TWIST_YAW, CFVec3A::m_Null );
	m_MortarTorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, _MORTAR_TORSO_TWIST_YAW, CFVec3A::m_Null );
	m_MortarTorsoUntwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, -_MORTAR_TORSO_TWIST_YAW, CFVec3A::m_Null );

	//m_hCheckpointParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _CHECKPOINT_PARTICLE_DEF_NAME );
	//if( m_hCheckpointParticleDef == FPARTICLE_INVALID_HANDLE ) {
	//	DEVPRINTF( "CBotGlitch::InitSystem(): Could not find particle definition '%s'.\n", _CHECKPOINT_PARTICLE_DEF_NAME );
	//}

	return TRUE;

	// Error:
_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotGlitch::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		fdelete( _pBotGlitchBuilder );
		_pBotGlitchBuilder = NULL;
		m_AnimStackDef.Destroy();
		m_bSystemInitialized = FALSE;
	}
}


CBotGlitch::CBotGlitch() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
	m_pRoboBuddy = NULL;
}


CBotGlitch::~CBotGlitch() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotGlitch::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );
	
	// ONLY PLAYERS CAN BE GLITCHY
	FASSERT( nPlayerIndex >= 0 );

	// Get pointer to the leaf class's builder object...
	CBotGlitchBuilder *pBuilder = (CBotGlitchBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == _pBotGlitchBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTGLITCH );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotGlitch::ClassHierarchyDestroy( void ) {
	u32 i;

	// Delete the items that we had instantiated for us...
	for( i=0; i<ItemInst_uMaxInventoryWeapons; ++i ) {
		fdelete( m_WeaponInv[0].m_apWeapon[i] );
		m_WeaponInv[0].m_apWeapon[i] = NULL;
	}

	for( i=0; i<ItemInst_uMaxInventoryWeapons; ++i ) {
		fdelete( m_WeaponInv[1].m_apWeapon[i] );
		m_WeaponInv[1].m_apWeapon[i] = NULL;
	}

	// this must happen before deleting the robo buddy - Mike
	if (m_pPowerupFx)
	{
		m_pPowerupFx->KillAll();
	}

	if( m_pRoboBuddy != NULL ) {
		m_pRoboBuddy->Destroy();
		fdelete( m_pRoboBuddy );
		m_pRoboBuddy = NULL;
	}

	fdelete(m_pPowerupFx);
	m_pPowerupFx = NULL;

	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	//for( int nIndex = 0; nIndex < MAX_CP_PARTICLES; nIndex++ )
	//{
	//	if( pECheckpointParticle[ nIndex ] )
	//	{
	//		pECheckpointParticle[ nIndex ]->Destroy();
	//		fdelete( pECheckpointParticle[ nIndex ] );
	//	}
	//}

	CBot::ClassHierarchyDestroy();
}


BOOL CBotGlitch::ClassHierarchyBuild( void ) {
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	s32 nBoneIndex, sLeftHandSecondaryFire;
	CFVec3A vRespawnPos_MS;
	cchar* pszMeshFilename = NULL;			//CPS 4.7.03

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CBotGlitchBuilder *pBuilder = (CBotGlitchBuilder *)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	//// NULL checkpoint particle emitter pointers.
	//// must happen before first call to _ExitWithError.
	//for( int nIndex = 0; nIndex < MAX_CP_PARTICLES; nIndex++ ) {
	//	pECheckpointParticle[ nIndex ] = NULL;
	//}

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, BotGlitch_pszSoundEffectBank ) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Could not load sound effect bank '%s'\n", BotGlitch_pszSoundEffectBank );
	}

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// ONLY PLAYERS CAN BE GLITCHY
	FASSERT( m_nOwnerPlayerIndex >= 0 );
	FASSERT( m_nPossessionPlayerIndex >= 0 );

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	// Load mesh resource...
//CPS 4.7.03	cchar* pszMeshFilename;
	if ( MultiplayerMgr.IsSinglePlayer() ) {
		pszMeshFilename = _apszDMMeshFilenames[0];
		if ( pBuilder->m_uMeshVersionOverride >0 && pBuilder->m_uMeshVersionOverride < sizeof(_apszDMMeshFilenames)/sizeof(char*) ) {
			pszMeshFilename = _apszDMMeshFilenames[pBuilder->m_uMeshVersionOverride];
		}

		if( pBuilder->m_pszMeshReplacement ) {
			pszMeshFilename = pBuilder->m_pszMeshReplacement;
		}
	}
	else {
		pszMeshFilename = _pszMultiplayerMeshName;
	}

	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
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

	//Jeremy: Register the player mesh with the engine so I can track lighting
	fmesh_RegisterPlayerMesh(m_pWorldMesh);

	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	for (u32 uIndex=0;uIndex<m_nBotInfo_Idles;uIndex++)	{
		m_Anim.IdleAnim_EnableControlSmoothing(uIndex);
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );
	EnableControlSmoothing( ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED );
	EnableControlSmoothing( ANIMCONTROL_AIM_SHOULDER_MOUNTED );
	EnableControlSmoothing( ANIMCONTROL_FIRE_2_UPPER );
	EnableControlSmoothing( ANIMCONTROL_FIRE_2_LOWER );
	EnableControlSmoothing( ANIMCONTROL_AIM_MORTAR );

	m_nBoneIndexGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	m_nBoneIndexTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	m_nBoneIndexArmLower = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_ARM_LOWER] );
	m_nBoneIndexArmElbow = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_ARM_ELBOW] );
	m_nBoneIndexHead = m_pWorldMesh->FindBone(m_apszBoneNameTable[BONE_HEAD]);

	if( (m_nBoneIndexArmLower < 0) || (m_nBoneIndexArmElbow < 0) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Can't find arm bones in mesh.\n" );
		goto _ExitWithError;
	}

	m_nBoneIndexPriFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );
	if( m_nBoneIndexPriFire < 0 ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Can't find primary bone in mesh.\n" );
		goto _ExitWithError;
	}

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_RIGHT_ARM_LOWER] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_RIGHT_ARM_ELBOW] );

	// set up antenna
	{
		s32 anBones[_ANTENNA_SEGMENTS];
		u32 uIndex;
		m_Antenna.Create( m_pWorldMesh, _ANTENNA_SEGMENTS, _ANTENNA_FREQUENCY, _ANTENNA_DAMPING, _ANTENNA_AMPLITUDE );
		anBones[0] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA1] );
		anBones[1] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA2] );
		anBones[2] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA3] );
		anBones[3] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA4] );
		anBones[4] = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ANTENNA5] );
		for( uIndex = 0; uIndex < _ANTENNA_SEGMENTS; uIndex++ ) {
			if( anBones[uIndex] < 0 ) {
				DEVPRINTF( "CBotGlitch::ClassHierarchyBuild: Antenna Bone %d doesn't exist in object '%s'.\n", uIndex, Name() );
			}
		}

		m_Antenna.RegisterBones( anBones );
	}

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA1] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA3] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA4] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA5] );


	// Set up data port stuff...
	m_pnEnableBoneNameIndexTableForSummer_TetherShock	= m_aBoneEnableIndices_FullBody;
	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_aBoneEnableIndices_FullBody;
	DataPort_SetupTetherShockInfo();

	// NKM - For Spy Vs. Spy
	if( Level_nLoadedIndex >= 0 && Level_aInfo[Level_nLoadedIndex].nLevel == LEVEL_SPY_VS_SPY ) {
		CSpyVsSpy::SetupGlitchPointer( this );
		m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallbackSpyVsSpy );
	}

	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_LEFT_TOE] );
	if( nBoneIndex >= 0 ) {
		m_pLeftFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;

		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_TOE] );
		if( nBoneIndex >= 0 ) {
			m_pRightFootDustPos_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		} else {
			m_pLeftFootDustPos_WS = NULL;
		}
	}

	if( !m_AnimManFrameRecoil.Create( ANIMBONE_RECOIL_COUNT, m_apszRecoilBoneNameTable ) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Could not create recoil animation summer.\n" );
		goto _ExitWithError;
	}

	AttachAnim( ANIMTAP_RECOIL_SUMMER, &m_AnimManFrameRecoil );
	m_AnimManFrameRecoil.Reset();
	UpdateBoneMask( ANIMTAP_RECOIL_SUMMER, m_aBoneEnableIndices_RecoilSummer, TRUE );

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );
	SetControlValue( ANIMCONTROL_RECOIL_SUMMER, 0.0f );

	// create weapon inventory from ItemInst inventory
	if( !_InitWeaponInventory(pBuilder) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Trouble creating Inventory.\n" );
		goto _ExitWithError;
	}

	SetMaxHealth();

	switch( m_apWeapon[0]->m_pInfo->nStanceType ) {
	case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
		SetControlValue( ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED, 1.0f );
		break;

	case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
		SetControlValue( ANIMCONTROL_AIM_SHOULDER_MOUNTED, 1.0f );
		break;
	}

	_WeaponOrUpgradeLevelMayHaveChanged();

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.23f );

		m_pPowerupFx = fnew CBotPowerupFx();	 //only player Glitchs get powerupFx objects
		m_pPowerupFx->SetBot( this );
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ nBoneIndex ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;   
	}

	m_pAISteerMtx = &m_MtxToWorld;
	
	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	//if( m_nPossessionPlayerIndex >= 0 ) {
	//	for( int nIndex = 0; nIndex < MAX_CP_PARTICLES; nIndex++ ) {
	//		pECheckpointParticle[ nIndex ] = fnew CEParticle;
	//		if( pECheckpointParticle[ nIndex ] ) {
	//			pECheckpointParticle[ nIndex ]->Create( "CheckpointParticle" );
	//		}
	//	}
	//}

	sLeftHandSecondaryFire = m_pWorldMesh->FindBone( "Secondary_Fire" );

	if( sLeftHandSecondaryFire < 0 ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild: Bone '%s' doesn't exist in object '%s'.\n", "Secondary_Fire", Name() );
		m_pLeftHandSecondaryFireBonePos = &m_MtxToWorld.m_vPos;
	} else {
		m_pLeftHandSecondaryFireBonePos = &m_pWorldMesh->GetBoneMtxPalette()[sLeftHandSecondaryFire]->m_vPos;
	}

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _HEADLIGHT_TEXTURE_ID );

	// set up part mgr...
	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, CPlayer::m_nPlayerCount, LIMB_TYPE_COUNT ) ) {
		goto _ExitWithError;
	}

	// init respawn effect
	vRespawnPos_MS.Set( 0.0f, 2.0f, 0.0f );
	if( !RespawnEffect_Init( _RESPAWN_PARTICLE_INTENSITY, m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_MINER_DUMMY] ), vRespawnPos_MS, _RESPAWN_MESH_START_SCALE, _RESPAWN_MESH_FINISH_SCALE ) ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Error initializing respawn effects\n" );
		goto _ExitWithError;
	}

	// load the death explosion
	m_hDeathExplosion = CExplosion2::GetExplosionGroup( _DEATH_EXPLOSION );
	if( m_hDeathExplosion == FEXPLOSION_INVALID_HANDLE ) {
        DEVPRINTF( "CBotGlitch::ClassHierarchyBuild(): Error loading death explosion group %s\n", _DEATH_EXPLOSION );
	}

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CBotGlitch::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	SetServoLevel( SERVO_TYPE_ARMS, m_anServoLevel[SERVO_TYPE_ARMS], TRUE );

	// turn glitch headlight on by default so artists can see their bump maps...
	#if( FANG_PLATFORM_WIN )
		if( m_nPossessionPlayerIndex >= 0 )	{
			SetSpotLightOn(TRUE);
		}
	#endif

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


CEntityBuilder *CBotGlitch::GetLeafClassBuilder( void ) {
	return _pBotGlitchBuilder;
}

void CBotGlitch::_ClearDataMembers( void ) {
	u32 i;

	// Clear out our array of weapon instances...
	for( i=0; i<ItemInst_uMaxInventoryWeapons; ++i ) {
		m_WeaponInv[0].m_apWeapon[i] = NULL;
		m_WeaponInv[1].m_apWeapon[i] = NULL;
	}

	// Init data members...

	m_pMoveIdentifier = &m_nBoneIndexGroin;

	m_fCollCylinderRadius_WS = 2.0f;
	m_fCollCylinderHeight_WS = 4.0f;

	for( i=0; i<SERVO_TYPE_COUNT; i++ ) {
		m_anServoLevel[i] = 0;
	}

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_MountAim = &m_BotInfo_MountAim;
	m_pBotInfo_Walk = &m_BotInfo_Walk;
	m_pBotInfo_Jump = &m_BotInfo_Jump;
	m_pBotInfo_Weapon = &m_BotInfo_Weapon;
	m_pBotInfo_Idle = m_BotInfo_Idle;
	m_nCountIdles = m_nBotInfo_Idles;


	m_fGravity = m_pBotInfo_Gen->fGravity;
	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;
	m_bFingerOnTrigger1 = FALSE;
	m_bFingerOnTrigger2 = FALSE;
	m_fFireAnimSpeedMult = 1.0f;
	m_fUnitTetherShockVibration = 0.0f;
	m_nMortarState = MORTAR_STATE_IDLE;
	m_fMortarUnitBlend = 0.0f;
	m_fMortarAbortUnitControl1 = 0.0f;
	m_fMortarAbortUnitControl2 = 0.0f;
	m_nMortarAbortControlIndex1 = (AnimControl_e)0;
	m_nMortarAbortControlIndex2 = (AnimControl_e)0;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_SNEAK] = ANIMTAP_SNEAK;
	m_anAnimStackIndex[ASI_WALK] = ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_RUN] = ANIMTAP_RUN;

	m_anAnimStackIndex[ASI_FALL] = ANIMCONTROL_JUMP_FLY;

	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_GRAB_AMMO] = ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_DROP_IN] = ANIMTAP_RELOAD_ROCKET1_DROP_IN;
	m_anAnimStackIndex[ASI_RELOAD_WITH_LEFT_FINISH] = ANIMTAP_RELOAD_ROCKET1_FINISH;

	m_anAnimStackIndex[ASI_RELOAD_TETHER_PREPARE_TO_PUMP] = ANIMTAP_RELOAD_TETHER_PREPARE_TO_PUMP;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_PUMP] = ANIMTAP_RELOAD_TETHER_PUMP;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_GRAB_AMMO] = ANIMTAP_RELOAD_TETHER_GRAB_AMMO;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_DROP_IN] = ANIMTAP_RELOAD_TETHER_DROP_IN;
	m_anAnimStackIndex[ASI_RELOAD_TETHER_FINISH] = ANIMTAP_RELOAD_TETHER_FINISH;

	m_anAnimStackIndex[ASI_RELOAD_SINGLE_ARM] = ANIMTAP_RELOAD_SINGLE_ARM;
	m_anAnimStackIndex[ASI_RELOAD_SINGLE_BODY] = ANIMTAP_RELOAD_SINGLE_BODY;

	m_anAnimStackIndex[ASI_RELOAD_CLIP_EJECT_OLD] = ANIMCONTROL_RELOAD_CLIP_EJECT_OLD;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_GRAB_NEW] = ANIMCONTROL_RELOAD_CLIP_GRAB_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_INSERT_NEW] = ANIMCONTROL_RELOAD_CLIP_INSERT_NEW;
	m_anAnimStackIndex[ASI_RELOAD_CLIP_SLAPIN_NEW] = ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW;

	m_anAnimStackIndex[ASI_RC_TETHERED]		= ANIMTAP_RC_TETHER;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= -1;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= -1;
	m_anAnimStackIndex[ASI_STOOP]			= -1;
	m_anAnimStackIndex[ASI_RESPAWN]			= ANIMTAP_RESPAWN;

	m_anAnimStackIndex[ASI_AIM_PILLBOX]		= ANIMTAP_AIM_PILLBOX;

	m_anAnimStackIndex[ASI_ASSEMBLE]		= ANIMTAP_ASSEMBLE;
	m_anAnimStackIndex[ASI_DISASSEMBLE]		= ANIMTAP_DISASSEMBLE;

	m_nHeadIdleState = HEAD_IDLE_STATE_SET_IDLE;
	m_fHeadIdleLookChance = 0.0f;
	m_fHeadIdleTime = HEAD_LOOK_IDLE_WAIT_TIME;
	//m_fCheckpointEffectTime = 0.0f;

	m_pnEnableBoneNameIndexTableForSummer_TetherShock = m_aBoneEnableIndices_FullBody;

	// Probe related
	m_bLeeched = FALSE;
	m_bNoAdvanceAnim = FALSE;
	m_bAnimateCamera = FALSE;
	m_fLookUnit = 0.0f;
	m_fLeechUnitBlend = 0.0f;
	m_LookStartQuat.Identity();
	m_LookStartDirection.Zero();
	m_LookEndAnimDirection.Zero();
	m_pLookTarget = NULL;
	m_pMinerDummy = NULL;
	m_pProbe = NULL;
	m_fProbeHitSoundTime = 0.0f;

	m_fProbeCamStartFOV = 0.0f;
	m_fProbeCamEndFOV = 0.0f;
	m_fProbeCamFOVTime = 0.0f;
	m_fProbeCamFOVDir = 0.0f;

	m_fWasherLoseTime = 0.0f;
	m_fWasherLoseNextTime = 0.0f;

	m_bGrabbedInAir = FALSE;

	m_mGrabCamera.Identity();									// matrix for grab camera
	m_mThrowCamera.Identity();									// matrix for grab camera
	m_GrabCameraInfo.m_pmtxMtx=&m_mGrabCamera;					// camera info for grab camera
	m_ThrowCameraInfo.m_pmtxMtx=&m_mThrowCamera;				// camera info for grab camera
	m_GrabCameraTrans.Init(&m_GrabCameraInfo,0.5f);				// camera transition object to/from grab
	m_GrabCamera.Init(&m_GrabCameraInfo,&m_BotInfo_GrabCamera);	// vehicle camera object for glitch
	m_bGrabbedByHead	= FALSE;			// we are grabbed
	m_pGrabberBot		= NULL;				// Who grabbed me;
	m_nGrabberBoneIndex = -1;				// By which bone;
	m_nBoneIndexHead	= -1;				// index of my own head
	m_fThrownYawVelocity= 0.f;				// Yaw Delta
	m_fGrabbedHeadYaw	= 0.f;
	m_fFallDownTime	= 0.f;				// thrown stun time;

	m_fUnitCableBlend = 0.0f;
	
	m_fUnitRecoil = 0.0f;
	m_fUnitRecoilTarget = 0.0f;

	m_fUnitFireCtlUpper = 0.0f;
	m_fUnitFireCtlLower = 0.0f;
	m_fFireTimer = 1000.0f;
	m_fUnitAim = 0.0f;

	m_fDeathTimer = 0.0f;
	m_uDeathExplosionCounter = _DEATH_NUM_EXPLOSIONS;
	m_fNextDeathEvent = 0.0f;

	m_fDamageAccumulator = 0.0f;
	m_uLastTimeDamaged	= 0;	

	m_uNumActiveDamageParticles = 0;

	//m_bInPieces = FALSE;
	//m_bFallDown = FALSE;
	//m_bFallAnimDone = TRUE;
	//m_bOnGround = FALSE;
	//m_fTimeInPieces = 0.0f;
	//m_fUnitPiecesBlend = 0.0f;

	m_eMeleeState = MELEESTATE_NONE;

	m_nStowedWeaponIndex[0] = 0;
	m_nStowedWeaponIndex[1] = 0;

	fforce_NullHandle( &m_hForce );
}	


void CBotGlitch::SetServoLevel( ServoType_e nServoType, u32 nServoLevel, BOOL bForceUpdate ) {
	FASSERT( IsCreated() );
	FASSERT( nServoType>=0 && nServoType<SERVO_TYPE_COUNT );
	FASSERT( nServoLevel < SERVO_COUNT );

	if( bForceUpdate || (m_anServoLevel[nServoType] != nServoLevel) ) {
		switch( nServoType ) {
		case SERVO_TYPE_LEGS:
			m_pBotInfo_Walk->fSneakNormVelocity = m_pBotInfo_Walk->fSneakVelocity / m_BotInfo_Glitch.afMaxXlatSpeed[ nServoLevel ];
			m_pBotInfo_Walk->fMinWalkNormVelocity = m_pBotInfo_Walk->fMinWalkVelocity / m_BotInfo_Glitch.afMaxXlatSpeed[ nServoLevel ];
			m_pBotInfo_Walk->fMinRunBlendNormVelocity = m_pBotInfo_Walk->fMinRunBlendVelocity / m_BotInfo_Glitch.afMaxXlatSpeed[ nServoLevel ];
			m_pBotInfo_Walk->fMaxRunBlendNormVelocity = m_pBotInfo_Walk->fMaxRunBlendVelocity / m_BotInfo_Glitch.afMaxXlatSpeed[ nServoLevel ];
			m_pBotInfo_Walk->fMaxXlatVelocity = m_BotInfo_Glitch.afMaxXlatSpeed[ nServoLevel ];
			m_pBotInfo_Walk->fInvMaxXlatVelocity = 1.0f / m_pBotInfo_Walk->fMaxXlatVelocity;
			m_pBotInfo_Walk->fNormVelocityStepSize = m_BotInfo_Glitch.afNormVelocityStepSize[ nServoLevel ];

			m_pBotInfo_Jump->fVerticalVelocityJump1 = m_BotInfo_Glitch.afVertVelocityJump1[ nServoLevel ];
			m_pBotInfo_Jump->fVerticalVelocityJump2 = m_BotInfo_Glitch.afVertVelocityJump2[ nServoLevel ];

			break;

		case SERVO_TYPE_ARMS:
			m_pBotInfo_Weapon->fSwitchTimeScale = m_BotInfo_Glitch.afWeaponSwitchTimeScale[ nServoLevel ];
			m_pBotInfo_Weapon->fTetherReloadAnimSpeedMult = m_BotInfo_Glitch.afTetherReloadAnimSpeedMult[ nServoLevel ];
			m_pBotInfo_Weapon->fTetherPumpAnimSpeedMult = m_BotInfo_Glitch.afTetherPumpAnimSpeedMult[ nServoLevel ];
			m_pBotInfo_Weapon->fClipReloadAnimSpeedMult = m_BotInfo_Glitch.afClipReloadAnimSpeedMult[ nServoLevel ];

			break;

		default:
			FASSERT_NOW;
		}

		m_anServoLevel[nServoType] = nServoLevel;
	}
}


// Returns TRUE if the servo level has been upgraded.
// Returns FALSE if the servo level was already maxed out (no change in servo level).
BOOL CBotGlitch::UpgradeServoLevel( ServoType_e nServoType ) {
	FASSERT( IsCreated() );

	if( GetServoLevel(nServoType) >= GetMaxServoLevel(nServoType) ) {
		// Servo level already at max. Don't do upgrade...
		return FALSE;
	}

	// Perform servo upgrade...
	SetServoLevel( nServoType, GetServoLevel(nServoType) + 1 );

	return TRUE;
}


// Returns a pointer to the specified weapon type.
// If the specified weapon type is not part of Glitch's inventory, this function returns NULL.
CWeapon *CBotGlitch::FindWeaponByType( CWeapon::WeaponType_e nWeaponType ) {
	FASSERT( IsCreated() );

	u32 i, j;

	for( j=0; j<2; ++j ) {
		for( i=0; i<m_WeaponInv[j].m_nWeaponInvCount; ++i ) {
			if( m_WeaponInv[j].m_apWeapon[i] ) {
				if( m_WeaponInv[j].m_apWeapon[i]->Type() == nWeaponType ) {
					// Found the weapon...
					return m_WeaponInv[j].m_apWeapon[i];
				}
			}
		}
	}

	// Weapon not found...
	return NULL;
}


void CBotGlitch::ClassHierarchyAddToWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	//m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM | BOTDEATHFLAG_COMEAPART;
	//m_uBotDeathFlags |= BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;

	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			m_apWeapon[i]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
			m_apWeapon[i]->AddToWorld();
			m_apWeapon[i]->ResetToState( CWeapon::STATE_DEPLOYED );
		}
	}

	// snap antenna to new position
	m_Antenna.SetPos();
	AtRestMatrixPalette();

#if 0
	if( !IsDrawEnabled() ) {
		FMATH_SETBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );

		m_apWeapon[0]->DrawEnable( FALSE );

		if( m_apWeapon[1] ) {
			m_apWeapon[1]->DrawEnable( FALSE );
		}
	}
#endif
}


void CBotGlitch::ClassHierarchyRemoveFromWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	for( i=0; i<2; i++ ) {
		if( m_apWeapon[i] ) {
			m_apWeapon[i]->RemoveFromWorld();
		}
	}

	m_pWorldMesh->RemoveFromWorld();

	fforce_Kill( &m_hForce );

	_ClearAllDamageEffects();

	CBot::ClassHierarchyRemoveFromWorld();
}

void CBotGlitch::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
	
	if (m_pGrabberBot)
		m_pGrabberBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}


const CFVec3A *CBotGlitch::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotGlitch::ClassHierarchyWork() {
 	CFVec3A TempVec3A;
	BOOL bVelocityImpulseApplied;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	UpdateSpotLight();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	m_fCableGrabTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fCableGrabTime, 0.0f );

	DataPort_Work();

	//if( m_fCheckpointEffectTime > 0.0f )
	//{
	//	f32 fAngle = m_fCheckpointEffectTime * FMATH_2PI * 1.2f;
	//	f32 fSin, fCos;
	//	f32 fCircleSize = 1.3f;

	//	fmath_SinCos( fAngle, &fSin, &fCos );

	//	for( int nIndex = 0; nIndex < MAX_CP_PARTICLES; nIndex++ )
	//	{
	//		pECheckpointParticle[ nIndex ]->MtxToParent()->m_vPos.x = fCircleSize * fSin;
	//		pECheckpointParticle[ nIndex ]->MtxToParent()->m_vPos.z = fCircleSize * fCos;
	//		pECheckpointParticle[ nIndex ]->Relocate_RotXlatFromUnitMtx_PS( pECheckpointParticle[ nIndex ]->MtxToParent() );

	//		// assuming only two emitters here.
	//		// this places them on opposite sides.
	//		fCircleSize = -fCircleSize;
	//	}

	//	m_fCheckpointEffectTime -= FLoop_fPreviousLoopSecs;
	//	FMATH_CLAMPMIN( m_fCheckpointEffectTime, 0.0f );
	//}
	//else
	//{
	//	if( !IsPlayerOutOfBody() ) {
	//		ParseControls();
	//	}
	//}

	if( !IsPlayerOutOfBody() ) {
		if( RespawnEffects_GetRemainingTime() == 0.0f ) {
			ParseControls();
		} else {
			ZeroControls();
			m_Antenna.SetPos();
		}
	}

	if( m_bControls_Melee && (m_eMeleeState == MELEESTATE_NONE) && !SwitchingWeapons() ) {
		_StartGSlap();
	}
			
#if !FANG_PRODUCTION_BUILD
	// Temp stuff...
	if( m_nPossessionPlayerIndex >= 0 ) {
		u32 nPortNum = Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex;

		// No more turbo
		//if( (Gamepad_aapSample[nPortNum][GAMEPAD_MAIN_TURBO]->uLatches & (FPAD_LATCH_CHANGED | FPAD_LATCH_ON)) == (FPAD_LATCH_CHANGED | FPAD_LATCH_ON) ) {
		//	// Apply a turbo boost...
		//	if( !IsImmobileOrPending() ) {
		//		ApplyVelocityImpulse_MS( CFVec3A( 0.0f, 0.0f, 150.0f ) );
		//	}
		//}
		if( (Gamepad_aapSample[nPortNum][GAMEPAD_MAIN_UP_EUK]->uLatches & (FPAD_LATCH_CHANGED | FPAD_LATCH_ON)) == (FPAD_LATCH_CHANGED | FPAD_LATCH_ON) ) {
			// Apply an EUK upgrade...
			u32  nMaxUpgradeLevel;

			if( m_apWeapon[0]->GetItemInst() ) {
				nMaxUpgradeLevel = FMATH_MIN( m_apWeapon[0]->GetMaxUpgradeLevel(), m_apWeapon[0]->GetItemInst()->HighestUpgradeAvailable() );
			} else {
				nMaxUpgradeLevel = m_apWeapon[0]->GetMaxUpgradeLevel();
			}

			if( m_apWeapon[0]->GetUpgradeLevel() >= nMaxUpgradeLevel ) {
				m_apWeapon[0]->SetUpgradeLevel( 0 );
			} else {
				m_apWeapon[0]->UpgradeWeapon();
			}
		}
	}
#endif

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS = m_MountPos_WS;
	m_nPrevState = m_nState;

	// Apply velocity impulses that were accululated last frame...
	bVelocityImpulseApplied = HandleVelocityImpulses();

	// Update position...
	TempVec3A.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( TempVec3A );
	m_fMaxDeltaJumpVelY_WS = m_pBotInfo_Jump->fVerticalVelocityJump1 + m_pBotInfo_Jump->fVerticalVelocityJump2;

	// Handle pitch and yaw...
	if (!m_bGrabbedByHead)
	{
		HandlePitchMovement();
		HandleYawMovement();
	}
	else
	{
		_HandleGrabbedPitchYaw();
	}

	if( m_nPossessionPlayerIndex < 0 )
	{
		HandleHeadLook();
	}

	HandleReverseFacing();

	_HandleIdleHeadMovement();

	// Rotate model space velocity to account for yaw change...
	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// If an external velocity impulse was applied above, recompute our velocities
	// and speeds...
	if( bVelocityImpulseApplied ) {
		VelocityHasChanged();
	}

	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();

	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
		HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	// Move and animate our bot...
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
		PROTRACK_BEGINBLOCK("AirXlat");
			_AbortMortarImmediately( TRUE );
			HandleAirTranslation();
			_HandleCableTranslation();
			_HandleCableAndDoubleJump();
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
		_HandleCableAnimations();
		HandleHipsAnimation();
	PROTRACK_ENDBLOCK();//"CommonAnim");

	PROTRACK_BEGINBLOCK("Weapon");
		HandleTargeting();
		_HandleWeaponFiring();
		_HandleWeaponAnimations();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_UpdateMatrices();

	PROTRACK_BEGINBLOCK("WeaponWork");
		_HandleWeaponWork();
		_HandleMeleeAnimations();
		if( m_eMeleeState == MELEESTATE_ATTACKING ) {
			_HandleMeleeAttack();
		}
	PROTRACK_ENDBLOCK();//"WeaponWork");

	_ProbeSwingWork();
	_GrabbedByHeadWork();

	// Call the work function for our Robo Buddy.
	if( m_pRoboBuddy != NULL ) {
		m_pRoboBuddy->Work( m_TargetedPoint_WS );
	}

	if( m_pCableHook && m_pCableHook->m_bAttached ) {
		m_pCableHook->m_Pos_Draw_WS = *m_pLeftHandSecondaryFireBonePos;
	}

	// call the work function for our botfx system
	if (GetPowerupFx())
	{
		GetPowerupFx()->Work();
	}

	if( IsDeadOrDying() ) {
		DeathWork();
	}

	_DamageEffectWork();

	BotInPiecesWork();
	//_BotInPiecesWork();

	RespawnEffect_Work();

	// Time till we can play the next hit sound
	m_fProbeHitSoundTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fProbeHitSoundTime, 0.0f );

	PROTRACK_BEGINBLOCK("GlitchAntenna");
	m_Antenna.Work();
	PROTRACK_ENDBLOCK();


#if FANG_PLATFORM_XB
	#if !FANG_PRODUCTION_BUILD
		static BOOL __bLightOn = FALSE;
		// debug code to turn on/off glitch headlight with second controller black button
		if( (CPlayer::m_nPlayerCount == 1) && (m_nPossessionPlayerIndex >= 0 ))
		{
			if( Gamepad_aapSample[Gamepad_nDebugPortIndex][GAMEPAD_MAIN_MELEE2]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
			{
				__bLightOn = !__bLightOn;
				if( __bLightOn )
				{
					SetSpotLightOn();
				}
				else
				{
					SetSpotLightOff();
				}
			}
		}
	#endif
#endif


// antenna tuning
//		extern f32 _fSpring;
//		extern f32 _fDamp;
//		extern f32 _fMagnitude;
////							mod var,    last value,		min,		max,	step,	velocity,		axis,							port
//		gamepad_AxisSlider( _fSpring,	_fSpring,		0.0f,	10000.0f,	1.0f,		10.0f,		GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X, 1 );
//		gamepad_AxisSlider( _fDamp,		_fDamp,			0.0f,	10.0f,	    0.001f,		0.25f,		GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_X, 1 );
//		gamepad_AxisSlider( _fMagnitude,_fMagnitude,	0.0f,	50.0f,	    0.01f,		0.5f,		GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_Y, 1 );
//		ftext_Printf( 0.3f, 0.54f, "~f1~C92929299~w1~al~s0.69spring       Left X:%.3f", _fSpring );
//		ftext_Printf( 0.3f, 0.56f, "~f1~C92929299~w1~al~s0.69damp        Right X:%.3f", _fDamp );
//		ftext_Printf( 0.3f, 0.58f, "~f1~C92929299~w1~al~s0.69magnitude   Right Y:%.3f", _fMagnitude );
//		m_Antenna.SetSpringConstant( _fSpring );
//		m_Antenna.SetSpringDamping( _fDamp );
//		m_Antenna.SetEffectMagnitude( _fMagnitude );
}
// antenna tuning
//f32 _fSpring = 70.0f;
//f32 _fDamp = 4.0f;
//f32 _fMagnitude = 5.0f;


BOOL CBotGlitch::GrabCable( CEZipLine *pCable ) {
	if( pCable == m_pCablePrev && m_fCableGrabTime != 0.0f ) {
		return FALSE;
	}

	m_pCableHook = pCable->AttachHook( &m_MountPos_WS, &m_Velocity_WS, this );

	if( m_pCableHook == NULL ) {
		return FALSE;
	}

	// NKM
	//m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1;
	//m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1;

	m_nJumpState = BOTJUMPSTATE_CABLE;
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_CABLE_GRASP, 1.0f );
	UpdateTime( ANIMCONTROL_CABLE_GRASP, 1.0f );
	m_fUnitCableBlend = 1.0f;

	SetJumping();

	// Update the hook hand brake value
	m_pCableHook->m_fInputBrakeAmount = m_fControls_Fire2;

	// NKM
	/*CFVec3A Pos_WS_Diff;

	Pos_WS_Diff.Sub( m_pCableHook->m_Pos_WS, *m_pLeftHandSecondaryFireBonePos );
	m_MountPos_WS.Add( Pos_WS_Diff );*/

	// Don't let the hand spin
	Summer_UpdateBoneMask( m_apszBoneNameTable[BONE_LEFT_HAND], TRUE );
	
	f32 fDot = ( m_MountUnitFrontXZ_WS.x * m_pCableHook->m_pCable->m_UnitVec12XZ.x ) + 
				( m_MountUnitFrontXZ_WS.z * m_pCableHook->m_pCable->m_UnitVec12XZ.y) ;
	f32 fZipYaw = fmath_Atan( m_pCableHook->m_pCable->m_UnitVec12XZ.y, m_pCableHook->m_pCable->m_UnitVec12XZ.x );

	if( fDot >= 0.0f ) {
		m_pCableHook->m_fYawSnapOffset = fZipYaw;
	} else {
		m_pCableHook->m_fYawSnapOffset = fZipYaw - FMATH_PI;
	}

	CFMtx43A::m_RotZ.SetRotationZ( -( m_fMountYaw_WS + m_pCableHook->m_fYawSnapOffset - FMATH_HALF_PI ) );
	*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_RotZ;

	ZipLineVerletSetup();

	return TRUE;
}

void CBotGlitch::ReleaseCable( void ) {
	if( !m_pCableHook ) {
		return;
	}

	SetControlValue( ANIMCONTROL_CABLE_GRASP, 0.0f );

	m_pCableHook->m_pCable->DetachHook( m_pCableHook, this );
	m_pCablePrev = m_pCableHook->m_pCable;
	m_fCableGrabTime = BOT_ZIPLINE_GRAB_TIME;
	m_pCableHook = NULL;

	*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_IdentityMtx;
	Summer_UpdateBoneMask( m_apszBoneNameTable[BONE_LEFT_HAND], FALSE );
}

void CBotGlitch::_HandleCableAndDoubleJump( void ) {
	// If Glitch is in a forced panic, that means he is being carried and thus cannot jump
	if( (m_nJumpState != BOTJUMPSTATE_CABLE) && !(m_nBotFlags & BOTFLAG_FORCED_PANIC) ) {
		if( m_bControls_Jump && ((m_nBotFlags2 & BOTFLAG2_CAN_DOUBLE_JUMP) || (m_Velocity_WS.y >= 0.0f)) && !IsJumpPadBoosted()) {
			if( m_nJumpState==BOTJUMPSTATE_LAUNCH || m_nJumpState==BOTJUMPSTATE_AIR ) {
				if( m_nBotFlags2 & BOTFLAG2_CAN_DOUBLE_JUMP ) {
					// Flip and boost...
					_StartDoubleJump( TRUE );
				} else {
					// Just flip...
					_StartDoubleJump( FALSE );
				}
			}
		}
	}
}

void CBotGlitch::_HandleCableTranslation( void ) {
	if( m_nJumpState == BOTJUMPSTATE_CABLE ) {
		if( m_pCableHook->m_bAttached ) {
			// Update the hook hand brake value
			m_pCableHook->m_fInputBrakeAmount = m_fControls_Fire2;

			m_Velocity_MS = m_pCableHook->m_Vel_WS;

			// NKM
			/*CFVec3A Pos_WS_Diff;

			Pos_WS_Diff.Sub( m_pCableHook->m_Pos_WS, *m_pLeftHandSecondaryFireBonePos );
			m_MountPos_WS.Add( Pos_WS_Diff );*/

			// Don't spin the hand
			CFMtx43A::m_RotZ.SetRotationZ( -( m_fMountYaw_WS + m_pCableHook->m_fYawSnapOffset - FMATH_HALF_PI ) );
			*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( BONE_LEFT_HAND ) = CFMtx43A::m_RotZ;

			m_Velocity_WS = m_pCableHook->m_Vel_WS;

			// Update velocities...
			WS2MS( m_Velocity_MS, m_Velocity_WS );
			VelocityHasChanged();

			if( m_bControlsBot_Jump2 ) {
				_StartSingleJump();
				_StartDoubleJump();

				ReleaseCable();
			} else if( m_bControls_Jump ) {
				_StartSingleJump();

				// Push off the cable in the direction we are holding
				CFVec3A Dir;

				Dir = m_XlatStickUnitVecXZ_WS;
				Dir.Mul(-10.0f);

				ApplyVelocityImpulse_MS(Dir);
				
				ReleaseCable();
			}
		} else {
			_StartSingleJump();
			ReleaseCable();
		}
	}
}


void CBotGlitch::_HandleCableAnimations( void ) {
	if( m_fUnitCableBlend>0.0f && m_nJumpState!=BOTJUMPSTATE_CABLE ) {
		m_fUnitCableBlend -= FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult;	// SER Change this!
		FMATH_CLAMPMIN( m_fUnitCableBlend, 0.0f );
		SetControlValue( ANIMCONTROL_CABLE_GRASP, m_fUnitCableBlend );
	}
}


void CBotGlitch::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
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


void CBotGlitch::DeployBuddy() {
	FASSERT( m_pRoboBuddy != NULL );
	m_pRoboBuddy->StartDeploy();
}



void CBotGlitch::SetBuddyUnitEffectEnd(f32 fUnitEffectEnd) {
	FASSERT( m_pRoboBuddy != NULL );
	m_pRoboBuddy->SetUnitEffectEnd(fUnitEffectEnd);
}


void CBotGlitch::StowBuddy() {
//	FASSERT( m_pRoboBuddy != NULL );
	if( m_pRoboBuddy != NULL ) {
		m_pRoboBuddy->StartStow();
	}
}

static const f32 _fTransitionToFlatTime = .367f;
static const f32 _fOOTransitionToFlatTime = 1.0f/_fTransitionToFlatTime;
static const f32 _fGetupBlendOutTime = .15f;
static const f32 _fOOGetupBlendOutTime = 1.0f/_fGetupBlendOutTime;
void CBotGlitch::_HandleJumping( void ) {
	if (m_nJumpState != BOTJUMPSTATE_AIR2)	{
		if( !m_bControlsBot_JumpVec ) {
			if( m_bControlsBot_Jump2 ) {
				_StartSingleJump();
				_StartDoubleJump();
			} else if( m_bControls_Jump ) {
				_StartSingleJump();

				FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );
			}
		} else {
			// Velocity jump specified...
			_StartVelocityJump( &m_ControlsBot_JumpVelocity_WS );
		}
	}
	
	if( m_nPrevState == STATE_AIR ) {
		if( m_nJumpState != BOTJUMPSTATE_NONE ) {
			_JumpLanded();
		}

		// If we hit ground while on a cable, detach
		if( m_pCableHook ) {
			ReleaseCable();
		}
	}
	if( (m_nState==STATE_GROUND) && (m_nJumpState == BOTJUMPSTATE_AIR2) ){
		if (m_fFallDownTime > 0.0f)
		{
			m_fFallDownTime -= FLoop_fPreviousLoopSecs;
		}
		else if (!DeltaTime( ANIMTAP_GETUP, FLoop_fPreviousLoopSecs, FALSE )) {
			f32 fTotalTimeRemaining = GetTotalTime(ANIMTAP_GETUP) - GetTime(ANIMTAP_GETUP);
			if( fTotalTimeRemaining < _fGetupBlendOutTime) {
				SetControlValue(ANIMCONTROL_GETUP, (fTotalTimeRemaining*_fOOGetupBlendOutTime));
			}
		}
		else {
			SetControlValue(ANIMCONTROL_GETUP, 0.0f);
			m_pGrabberBot = NULL;
			m_nJumpState = BOTJUMPSTATE_NONE;
			ClearJumping();
			m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;
			ReticleEnable(TRUE);
			m_GrabCameraTrans.InitTransitionToBotCamera();
		}
	}
}


void CBotGlitch::_EnterFlyMode( void ) {
	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );

	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


void CBotGlitch::_StartVelocityJump( const CFVec3A *pJumpVelocity_WS ) {
	CFVec3A JumpVelocity_WS = *pJumpVelocity_WS;
	FMATH_CLAMP( JumpVelocity_WS.y, 0.0f, m_fMaxDeltaJumpVelY_WS );

	_StartSingleJump( &JumpVelocity_WS );

	if( pJumpVelocity_WS->y >= m_pBotInfo_Jump->fVerticalVelocityJump1 ) {
		_StartDoubleJump( FALSE );
	}
}


void CBotGlitch::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
#if 0
	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
		CFVec3A VerletVel_WS;
		m_pStickyEntity->GetVerlet()->ComputeSurfaceVelocity_WS( &VerletVel_WS, &m_MtxToWorld.m_vPos, FLoop_fPreviousLoopOOSecs );
		m_Velocity_WS.Add( VerletVel_WS );
		WS2MS( m_Velocity_MS, m_Velocity_WS );
	}
#endif

	if( pJumpVelocity_WS == NULL ) {
		m_Velocity_WS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
	} else {
		m_Velocity_WS.x = pJumpVelocity_WS->x;
		m_Velocity_WS.y = pJumpVelocity_WS->y;
		m_Velocity_WS.z = pJumpVelocity_WS->z;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );

	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_fUnitJump1Jump2Blend = 0.0f;
	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	m_nState = STATE_AIR;
	SetJumping();
	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}


void CBotGlitch::_StartDoubleJump( BOOL bAddToVelocityY ) {
	if( !HaveTimeToDoubleJump() ) {
		return;
	}

	StartDoubleJumpGeneric(bAddToVelocityY,
							ANIMTAP_JUMP_TUCK,
							ANIMTAP_JUMP_UNTUCK,
							ANIMCONTROL_JUMP_LAUNCH,
							ANIMCONTROL_JUMP_FLY,
							ANIMCONTROL_JUMP_TUCK);

	// NKM - Remove all attached Swarmers
	DetachAllSwarmers();
}

void CBotGlitch::FallDown( f32 fHowLongSecs ) 
{
	m_fFallDownTime = fHowLongSecs;
	m_nState=STATE_GROUND;
	m_nJumpState = BOTJUMPSTATE_AIR2;
	SetControlValue(ANIMCONTROL_GETUP, 1.0f);
	UpdateUnitTime(ANIMTAP_GETUP, 0.0f);
}

void CBotGlitch::_JumpLanded( void ) {
	
	if (m_nJumpState != BOTJUMPSTATE_AIR2) {
		m_nJumpState = BOTJUMPSTATE_NONE;
		ClearJumping();	
		MakeFootImpactDust( TRUE, TRUE, m_pBotInfo_Gen->fUnitDustKickup, &m_FeetToGroundCollImpact.UnitFaceNormal );
	}
	else {
		MakeFootImpactDust( TRUE, TRUE, 1.0f, &m_FeetToGroundCollImpact.UnitFaceNormal );
		m_pPartMgr->RepairLimb(LIMB_TYPE_RIGHTARM);
		m_pPartMgr->RepairLimb(LIMB_TYPE_LEFTARM);
		m_pPartMgr->RepairLimb(LIMB_TYPE_LEFTLEG);
		m_pPartMgr->RepairLimb(LIMB_TYPE_RIGHTLEG);
		FallDown(_fStunnedFromBeingThrownTime);
	}
	

	if (IsJumpPadBoosted()) // we took off from a jumppad 
	{
		f32 fOOMaxInt16=fmath_Inv((f32) (u16)-1);
		f32 fLandingDrag = ((f32)m_uLandingVelocityOverride)*fOOMaxInt16;
		m_Velocity_WS.Mul(fLandingDrag);
		WS2MS(m_Velocity_MS,m_Velocity_WS);
		VelocityHasChanged();
		ClearJumpPadBoost();
	}
	m_fFlipPitch = 0.0f;
	m_bPlayLandAnim = TRUE;

	ZeroTime( ANIMTAP_JUMP_LAND_UPPER );

	m_fMaxLandUnitBlend = GetControlValue( ANIMCONTROL_JUMP_FLY );
	if( m_fMaxLandUnitBlend == 0.0f ) {
		m_fMaxLandUnitBlend = 1.0f;
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fMaxLandUnitBlend );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );

	if ( m_nPossessionPlayerIndex >= 0 )	{
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) {
			// Don't play scuff sounds when landing from a jump
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1) ) ],
				m_pBotInfo_Walk->fJumpLandedSoundUnitVolume_2D * m_fStepVolumeScale
			);
		}
		//tell the AI that this sound happened
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

	if( m_fSpeedXZ_WS == 0.0f ) {
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, m_fMaxLandUnitBlend );
		m_bPlayLandAnimLower = TRUE;
	} else {
		SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
		m_bPlayLandAnimLower = FALSE;
	}

	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
		CFVec3A ImpulseVec_WS;

		ImpulseVec_WS.Set( 0.0f, -m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
		m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
	}

	_AddJolt( 1.0f );
}


void CBotGlitch::_HandleJumpAnimations( void ) {
	f32 fTemp, fUnitTime;

	if( m_nState == STATE_GROUND ) {
		// We're on the ground...
		if( m_fUnitFlyBlend > 0.0f ) {
			m_fUnitFlyBlend = 0.0f;
			_JumpLanded();
			FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );
		}

		// Handle landing animation...
		if( m_bPlayLandAnim ) {
			if( m_fSpeedXZ_WS > 0.0f ) {
				fTemp = m_pBotInfo_Jump->fMovingLandAnimSpeedMult;
			} else {
				fTemp = 1.0f;
			}

			if( !DeltaTime( ANIMTAP_JUMP_LAND_UPPER, FLoop_fPreviousLoopSecs * fTemp ) ) {
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
				SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 0.0f );
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
			if( m_nJumpState==BOTJUMPSTATE_TUCK || m_nJumpState==BOTJUMPSTATE_FLIPPING || m_nJumpState==BOTJUMPSTATE_UNTUCK ) {
				m_fFlipPitch += m_fFlipPitchVelocity * FLoop_fPreviousLoopSecs;
			}

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

				m_fUnitJump1Jump2Blend -= m_pBotInfo_Jump->fUntuckToFlyBlendSpeed * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fUnitJump1Jump2Blend, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_UNTUCK, m_fUnitJump1Jump2Blend );

				if( CanAttachToZipLine() ) {
					CFSphere Sphere;

					//Sphere.m_Pos = m_MountPos_WS.v3;
					Sphere.m_Pos = (*m_pLeftHandSecondaryFireBonePos).v3;
					Sphere.m_fRadius = FMATH_MAX(m_fCollCylinderHeight_WS, m_fCollCylinderRadius_WS);

					CEZipLine *pZipHit = CEZipLine::GetZipCollision( Sphere );

					if( pZipHit ) {
						GrabCable( pZipHit );
					}
				}

				break;

			case BOTJUMPSTATE_TUCK:
				if( DeltaTime( ANIMTAP_JUMP_TUCK, FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult, TRUE ) ) {
					// Tuck animation completed...

					if( m_fUntuckStartCountdownSecs > 0.0f ) {
						m_nJumpState = BOTJUMPSTATE_FLIPPING;
						SetJumping();
					} else {
						// Time to start untuck animation...
						SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
						SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 1.0f );
						ZeroTime( ANIMTAP_JUMP_UNTUCK );
						m_nJumpState = BOTJUMPSTATE_UNTUCK;
						SetJumping();
					}
				}

				break;

			case BOTJUMPSTATE_FLIPPING:
				m_fUntuckStartCountdownSecs -= FLoop_fPreviousLoopSecs;

				if( m_fUntuckStartCountdownSecs <= 0.0f ) {
					// Time to start untuck animation...
					SetControlValue( ANIMCONTROL_JUMP_TUCK, 0.0f );
					SetControlValue( ANIMCONTROL_JUMP_UNTUCK, 1.0f );
					ZeroTime( ANIMTAP_JUMP_UNTUCK );
					m_nJumpState = BOTJUMPSTATE_UNTUCK;
					SetJumping();
				}

				break;

			case BOTJUMPSTATE_UNTUCK:
				if( DeltaTime( ANIMTAP_JUMP_UNTUCK, FLoop_fPreviousLoopSecs * m_fTuckUntuckAnimTimeMult, TRUE ) ) {
					// Untuck animation completed...
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_fFlipPitch = 0.0f;
					m_fUnitJump1Jump2Blend = 1.0f;
					m_nJumpState = BOTJUMPSTATE_AIR;
					SetJumping();
				}

				break;

			case BOTJUMPSTATE_CABLE:
				DeltaTime( ANIMTAP_JUMP_FLY );
				DeltaTime( ANIMTAP_CABLE_GRASP, FLoop_fPreviousLoopSecs, TRUE );

				if( m_fUnitCableBlend < 1.0f ) {
					m_fUnitCableBlend = 1.0f;
					SetControlValue( ANIMCONTROL_CABLE_GRASP, 1.0f );
				}

				break;
			case BOTJUMPSTATE_AIR2:
				{
					f32 fUnitControl  = GetControlValue(ANIMCONTROL_GETUP);
					fUnitControl += (FLoop_fPreviousLoopSecs * _fOOTransitionToFlatTime);
					if (fUnitControl < 1.0f)
						SetControlValue(ANIMCONTROL_GETUP, fUnitControl);
					else
					{	
						SetControlValue(ANIMCONTROL_GETUP, 1.0f);
					}
					m_fMountYaw_WS += (m_fThrownYawVelocity*FLoop_fPreviousLoopSecs);
				}
				break;
			default:
				FASSERT_NOW;
			}
		}
	}
}


void CBotGlitch::_AddJolt( f32 fUnitJolt ) {
	if( m_apWeapon[1] && (m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE) ) {
		((CWeaponScope *)m_apWeapon[1])->SetJolt( fUnitJolt );
	}
}


void CBotGlitch::_HandleWeaponWork( void ) {
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->Work();
	}
	if( m_apWeapon[1] ) {
		m_apWeapon[1]->Work();
	}
}


void CBotGlitch::_ChangeWeaponIndex( u32 nHandIndex, u32 nNewIndex ) {
	FASSERT( nHandIndex==0 || nHandIndex==1 );
	FASSERT( nNewIndex < m_WeaponInv[ nHandIndex ].m_nWeaponInvCount );

	m_WeaponInv[ nHandIndex ].m_nWeaponInvIndex = (u8)nNewIndex;

	BOOL bWeaponDrawEnabled = TRUE;
	if( m_apWeapon[nHandIndex] ) {
		bWeaponDrawEnabled = m_apWeapon[nHandIndex]->IsDrawEnabled();
		m_apWeapon[nHandIndex]->RemoveFromWorld();
	}

	m_apWeapon[nHandIndex] = m_WeaponInv[nHandIndex].m_apWeapon[ nNewIndex ];

	if( m_apWeapon[nHandIndex] ) {
		m_apWeapon[nHandIndex]->AddToWorld();
		m_apWeapon[nHandIndex]->DrawEnable(bWeaponDrawEnabled);

		if( nHandIndex == 0 ) {
			m_apWeapon[nHandIndex]->ResetToState( CWeapon::STATE_STOWED );
			m_apWeapon[nHandIndex]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_SECONDARY_FIRE], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
		} else {
			m_apWeapon[nHandIndex]->ResetToState( CWeapon::STATE_DEPLOYED );
		}
	}
}


BOOL CBotGlitch::_IsPrimaryWeaponSwitchQueued( void ) {
	if( m_WeaponInv[0].m_nWeaponSwitchToIndex == 255 ) {
		// No switch request...
		return FALSE;
	}

	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		// Can't switch while firing...
		return FALSE;
	}

	if( m_nThrowState != THROWSTATE_NONE ) {
		// Can't switch while throwing a secondary grenade...
		return FALSE;
	}

	if( m_WeaponInv[0].m_nWeaponSwitchToIndex == m_WeaponInv[0].m_nWeaponInvIndex ) {
		// Switch request is same as current weapon. Absorb it...
		m_WeaponInv[0].m_nWeaponSwitchToIndex = 255;
		return FALSE;
	}

	if( m_nJumpState == BOTJUMPSTATE_CABLE ) {
		// A primary weapon switch request is not allowed...
		return FALSE;
	}

	if( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_TETHER ) {
		CWeaponTether *pTether = (CWeaponTether *)m_apWeapon[0];
		if( pTether->IsTetherAttachedToTarget() ) {
			return FALSE;
		}
	}

	if( !IsReverseFacingForward() ) {
		return FALSE;
	}

	// A primary weapon switch is queued and allowed...

	return TRUE;
}


BOOL CBotGlitch::_IsSecondaryWeaponSwitchQueued( void ) {
	if( m_WeaponInv[1].m_nWeaponSwitchToIndex == 255 ) {
		// No switch request...
		return FALSE;
	}

	if( m_nThrowState != THROWSTATE_NONE ) {
		// Can't switch while throwing a secondary grenade...
		return FALSE;
	}

	if( m_nMortarState != MORTAR_STATE_IDLE ) {
		// Can't switch while mortaring
		return FALSE;
	}

	if( m_WeaponInv[1].m_nWeaponSwitchToIndex == m_WeaponInv[1].m_nWeaponInvIndex ) {
		// Switch request is same as current weapon. Absorb it...
		m_WeaponInv[1].m_nWeaponSwitchToIndex = 255;
		return FALSE;
	}

	if( !IsReverseFacingForward() ) {
		return FALSE;
	}

	// A secondary weapon switch is queued and allowed...

	return TRUE;
}


// Returns TRUE if the primary weapon switch has begun.
void CBotGlitch::_StartPrimaryWeaponSwitch( void ) {
	FASSERT( _IsPrimaryWeaponSwitchQueued() );

	// Cancel any reload requests...
	m_abReloadRequest[0] = FALSE;

	if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST ) {
		m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL;
	} else if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_SHOULDER_MOUNT ) {
		m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED;
	} else {
		m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE;
	}

	SetHeadLookInterrupt();
	m_nWSState = WS_STATE_GRABBING;
	SetControlValue( m_nWSASI, 0.0f );
	UpdateUnitTime( m_nWSASI, 1.0f );

	m_apWeapon[0]->SetDesiredState( CWeapon::STATE_STOWED );
}


void CBotGlitch::_SwitchPrimaryWeaponsInBackpack( void ) {
	FASSERT( m_WeaponInv[0].m_nWeaponSwitchToIndex != 255 );

	_ChangeWeaponIndex( 0, m_WeaponInv[0].m_nWeaponSwitchToIndex );

	m_nWSState = WS_STATE_RETRIEVING;

	m_WeaponInv[0].m_nWeaponSwitchToIndex = 255;

	if ( m_nPossessionPlayerIndex >= 0 ){
		fsndfx_Play2D( m_pBotInfo_Weapon->hWSBackpackSound );
	}

	_WeaponOrUpgradeLevelMayHaveChanged();
}


void CBotGlitch::_HandleWeaponSwitchingAnimations( void ) {
	#define __NUM_WEAP_ANIM_BLENDS 2

	f32 fUnitTime, fControlVal;
	BOOL bUpdateBlend, abBlendIn[__NUM_WEAP_ANIM_BLENDS];
	u32 i;

	static const AnimControl_e __anIndexBlend[__NUM_WEAP_ANIM_BLENDS] = {
		ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED,
		ANIMCONTROL_AIM_SHOULDER_MOUNTED,
	};


	if( _IsSecondaryWeaponSwitchQueued() ) {
		_ChangeWeaponIndex( 1, m_WeaponInv[1].m_nWeaponSwitchToIndex );
		m_WeaponInv[1].m_nWeaponSwitchToIndex = 255;
	}

	// Handle weapon switching animations...
	switch( m_nWSState ) {
	case WS_STATE_NONE:
		if( _IsPrimaryWeaponSwitchQueued() ) {
			AbortReloadImmediately();
			AbortScopeMode();
			_StartPrimaryWeaponSwitch();
			m_fFireTimer = _FIRE_ANIM_READY_TIME + 1.0f;	// tell fire animation we're done aiming
		} else {
			// If the EUK level has changed, we must update which aim anim we use...

			// default to no blending
			bUpdateBlend = FALSE;
			abBlendIn[0] = FALSE;
			abBlendIn[1] = FALSE;

			switch( m_apWeapon[0]->m_pInfo->nStanceType ) {
			case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
				bUpdateBlend = TRUE;	// activate blending code below
				abBlendIn[0] = TRUE;	// set flag to blend in torso twist
				abBlendIn[1] = FALSE;	// set flag to blend out shoulder mount

				break;

			case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
				bUpdateBlend = TRUE;	// activate blending code below
				abBlendIn[0] = FALSE;	// set flag to blend out torso twist
				abBlendIn[1] = TRUE;	// set flag to blend in shoulder mount

				break;
			}

			if( bUpdateBlend ) {
				// there are anims to blend in/out
				for( i=0; i<__NUM_WEAP_ANIM_BLENDS; ++i ) {
					// step through list of anims to blend
					if( abBlendIn[i] ) {
						// over time, increase anim control value to max
						fControlVal = GetControlValue( __anIndexBlend[i] );
						if( fControlVal < 1.0f ) {
							fControlVal += 2.0f * FLoop_fPreviousLoopSecs;
							FMATH_CLAMPMAX( fControlVal, 1.0f );
							SetControlValue( __anIndexBlend[i], fControlVal );
						}
					} else {
						// over time, decrease anim control value to min
						fControlVal = GetControlValue( __anIndexBlend[i] );
						if( fControlVal > 0.0f ) {
							fControlVal -= 2.0f * FLoop_fPreviousLoopSecs;
							FMATH_CLAMPMIN( fControlVal, 0.0f );
							SetControlValue( __anIndexBlend[i], fControlVal );
						}
					}
				}
			}
		}

		break;

	case WS_STATE_GRABBING:
		if( !DeltaTime( m_nWSASI, -FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fSwitchTimeScale, TRUE ) ) {
			// We're in the process of grabbing...

			fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( m_nWSASI ) );

			SetControlValue( m_nWSASI, 1.01f - fUnitTime );

			switch( m_apWeapon[0]->m_pInfo->nStanceType ) {
			case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
				SetControlValue( ANIMCONTROL_AIM_SHOULDER_MOUNTED, fUnitTime );
				break;

			case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
				SetControlValue( ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED, fUnitTime );
				break;
			}
		} else {
			// We're done grabbing...

			if ( m_nPossessionPlayerIndex >= 0 ){
				fsndfx_Play2D( m_pBotInfo_Weapon->hWSDetachSound );
			}

			m_nWSState = WS_STATE_STOWING;
			SetControlValue( ANIMCONTROL_WEAPON_SWITCH, 1.0f );

			ZeroTime( ANIMTAP_WEAPON_SWITCH );
			SetControlValue( m_nWSASI, 0.0f );

			m_apWeapon[0]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_SECONDARY_FIRE], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
		}

		break;

	case WS_STATE_STOWING:
	case WS_STATE_RETRIEVING:
		if( !DeltaTime( ANIMTAP_WEAPON_SWITCH, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fSwitchTimeScale, TRUE ) ) {
			// We're in the process of stowing or retrieving...

			if( m_nWSState==WS_STATE_STOWING && GetUnitTime( ANIMTAP_WEAPON_SWITCH )>=0.5f ) {
				// We're done stowing...
				_SwitchPrimaryWeaponsInBackpack();
			}
		} else {
			// We're done retrieving...

			if ( m_nPossessionPlayerIndex >= 0 ){
				fsndfx_Play2D( m_pBotInfo_Weapon->hWSAttachSound );
			}

			if( m_nWSState == WS_STATE_STOWING ) {
				// If we reached the end of the animation without ever triggering
				// the stow->retrieve switch point, let's switch now...
				_SwitchPrimaryWeaponsInBackpack();
			}

			if( _IsPrimaryWeaponSwitchQueued() ) {
				// Another switch request has been queued...

				m_nWSState = WS_STATE_STOWING;
				ZeroTime( ANIMTAP_WEAPON_SWITCH );
			} else {
				// No additional switch requests are in the queue...

				ClearHeadLookInterrupt();
				m_nWSState = WS_STATE_RELEASING;
				SetControlValue( ANIMCONTROL_WEAPON_SWITCH, 0.0f );

				switch( m_apWeapon[0]->m_pInfo->nStanceType ) {
				case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
					m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED;
					break;

				case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
					m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL;
					break;

				default:
					// Single-handed...
					m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE;
					break;
				}

				ZeroTime( m_nWSASI );
				SetControlValue( m_nWSASI, 1.0f );

				m_apWeapon[0]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
				m_apWeapon[0]->SetDesiredState( CWeapon::STATE_DEPLOYED );
			}
		}

		break;

	case WS_STATE_RELEASING:
		if( !DeltaTime( m_nWSASI, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fSwitchTimeScale * 0.5f, TRUE ) ) {
			// We're in the process of releasing...

			fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( m_nWSASI ) );

			SetControlValue( m_nWSASI, 1.01f - fUnitTime );

			switch( m_apWeapon[0]->m_pInfo->nStanceType ) {
			case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
				SetControlValue( ANIMCONTROL_AIM_SHOULDER_MOUNTED, fUnitTime );
				break;

			case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
				SetControlValue( ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED, fUnitTime );
				break;

			}
		} else {
			// We're done releasing...

			SetControlValue( m_nWSASI, 0.0f );

			if( _IsPrimaryWeaponSwitchQueued() ) {
				// Another primary weapon switch is requested. Start it...
				_StartPrimaryWeaponSwitch();
			} else {
				// No other primary weapon switches. We're done...
				m_nWSState = WS_STATE_NONE;
			}
		}

		break;


	default:
		FASSERT_NOW;
		break;
	}

	#undef __NUM_WEAP_ANIM_BLENDS
}


// Called by the weapon when it's EUK upgrade level has changed...
void CBotGlitch::NotifyWeaponUpgraded( CWeapon *pUpgradedWeapon ) {
	if( pUpgradedWeapon != m_apWeapon[0] ) {
		// Not our current weapon...
		return;
	}

	// The EUK upgrade level of our currently-selected primary weapon
	// has changed...

	_WeaponOrUpgradeLevelMayHaveChanged();
}


void CBotGlitch::AbortReloadImmediately( void ) {
	if( m_apWeapon[0]->Type() != CWeapon::WEAPON_TYPE_MORTAR ) {
		CBot::AbortReloadImmediately();
		return;
	}

	_AbortMortarImmediately();
}


void CBotGlitch::_AbortMortarImmediately( BOOL bKeepRoundInHand ) {
	if( bKeepRoundInHand && (m_nMortarState==MORTAR_STATE_JUMPING) ) {
		return;
	}

	if( m_apWeapon[0]->Type() != CWeapon::WEAPON_TYPE_MORTAR ) {
		return;
	}

	MortarState_e nAbortState = bKeepRoundInHand ? MORTAR_STATE_JUMPING : MORTAR_STATE_ABORTING;
	CWeaponMortar *pWeaponMortar = (CWeaponMortar *)m_apWeapon[0];

	// Handle mortar...
	switch( m_nMortarState ) {
	case MORTAR_STATE_GRABBING_ROUND:
		m_nMortarState = nAbortState;
		m_nMortarAbortControlIndex1 = ANIMCONTROL_RELOAD_MORTAR_GRAB_AMMO;
		m_fMortarAbortUnitControl1 = GetControlValue( ANIMTAP_RELOAD_MORTAR_GRAB_AMMO );
		m_fMortarAbortUnitControl2 = 0.0f;

		pWeaponMortar->Mortar_Fork_Abort();
		pWeaponMortar->Mortar_Sling_Release( FALSE );

		break;

	case MORTAR_STATE_PLACING_ROUND:
		m_nMortarState = nAbortState;
		m_nMortarAbortControlIndex1 = ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING;
		m_fMortarAbortUnitControl1 = GetControlValue( ANIMTAP_RELOAD_MORTAR_PLACE_IN_SLING );
		m_fMortarAbortUnitControl2 = 0.0f;

		pWeaponMortar->Mortar_Fork_Abort();
		pWeaponMortar->Mortar_Sling_Release( FALSE );

		break;

	case MORTAR_STATE_TAKING_UP_SLACK:
		m_nMortarState = nAbortState;
		m_nMortarAbortControlIndex1 = ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK;
		m_fMortarAbortUnitControl1 = GetControlValue( ANIMTAP_RELOAD_MORTAR_TAKE_UP_SLACK );
		m_fMortarAbortUnitControl2 = 0.0f;

		pWeaponMortar->Mortar_Fork_Abort();
		pWeaponMortar->Mortar_Sling_Release( FALSE );

		break;

	case MORTAR_STATE_PLACE_TO_AIM_TRANSITION:
		m_nMortarState = nAbortState;
		m_nMortarAbortControlIndex1 = ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK;
		m_fMortarAbortUnitControl1 = GetControlValue( ANIMTAP_RELOAD_MORTAR_TAKE_UP_SLACK );

		m_nMortarAbortControlIndex2 = ANIMCONTROL_AIM_MORTAR;
		m_fMortarAbortUnitControl2 = GetControlValue( ANIMTAP_AIM_MORTAR );

		pWeaponMortar->Mortar_Fork_Abort();
		pWeaponMortar->Mortar_Sling_Release( FALSE );

		break;

	case MORTAR_STATE_AIMING:
		m_nMortarState = nAbortState;
		m_nMortarAbortControlIndex1 = ANIMCONTROL_AIM_MORTAR;
		m_fMortarAbortUnitControl1 = GetControlValue( ANIMTAP_AIM_MORTAR );
		m_fMortarAbortUnitControl2 = 0.0f;

		pWeaponMortar->Mortar_Fork_Abort();
		pWeaponMortar->Mortar_Sling_Release( FALSE );

		break;

	case MORTAR_STATE_FIRING:
		m_nMortarState = nAbortState;
		m_nMortarAbortControlIndex1 = ANIMCONTROL_FIRE_1_UPPER;
		m_fMortarAbortUnitControl1 = GetControlValue( ANIMTAP_FIRE_1_UPPER );
		m_fMortarAbortUnitControl2 = 0.0f;

		pWeaponMortar->Mortar_Fork_Abort();
		pWeaponMortar->Mortar_Sling_Release( FALSE );

		break;

	case MORTAR_STATE_REPLACE_ROUND_AFTER_JUMP:
		m_nMortarState = nAbortState;
		m_nMortarAbortControlIndex1 = ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING;
		m_fMortarAbortUnitControl1 = GetControlValue( ANIMTAP_RELOAD_MORTAR_PLACE_IN_SLING );
		m_fMortarAbortUnitControl2 = 0.0f;

		pWeaponMortar->Mortar_Fork_Abort();
		pWeaponMortar->Mortar_Sling_Release( FALSE );

		break;

	case MORTAR_STATE_JUMPING:
		m_nMortarState = MORTAR_STATE_ABORTING;

		break;
	}
}


void CBotGlitch::StartedUsingLeftArmToReload( void ) {
	AbortScopeMode();
}


void CBotGlitch::HandleWeaponReloadingAnimations( void ) {
	if( (m_nWRState != WR_STATE_NONE) && 
		(m_nWRState != WR_STATE_FULLYAUTO_RELOAD_OPENING) && 
		(m_nWRState != WR_STATE_FULLYAUTO_RELOADING) ) {
		// blend out of fire animations
		m_fFireTimer = _FIRE_ANIM_READY_TIME + 1.0f;
	}

	if( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_MORTAR ) {
		_HandleMortarReloadingAnimations();
	}

	CBot::HandleWeaponReloadingAnimations();
}


void CBotGlitch::_HandleMortarReloadingAnimations( void ) {
	FASSERT( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_MORTAR );
	FASSERT( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_MORTAR );

	CWeaponMortar *pWeaponMortar = (CWeaponMortar *)m_apWeapon[0];
	f32 fUnit, fUnit2;

	switch( m_nMortarState ) {
	case MORTAR_STATE_IDLE:
		if( (m_fControls_Fire1 > 0.0f) && pWeaponMortar->Mortar_IsReadyToFire() ) {
			if( m_nWSState == WS_STATE_NONE ) {
				if( m_nJumpState == BOTJUMPSTATE_NONE && m_apWeapon[1]->Type() != CWeapon::WEAPON_TYPE_CLEANER ) {
					if( m_apWeapon[1]->Throwable_Prime() ) {
						m_nMortarState = MORTAR_STATE_GRABBING_ROUND;
						UpdateTime( ANIMTAP_RELOAD_MORTAR_GRAB_AMMO, 0.0f );
						SetControlValue( ANIMCONTROL_RELOAD_MORTAR_GRAB_AMMO, 0.0f );

						pWeaponMortar->Mortar_Fork_AttachReloadAnim( &m_Anim.m_pLoadedBaseAnimInst[ ANIM_RELOAD_MORTAR_GRAB_AMMO ] );
						pWeaponMortar->Mortar_Fork_AttachAimFireAnim( NULL );
						pWeaponMortar->Mortar_Fork_SetReloadAnimControl( 0.0f );
						pWeaponMortar->Mortar_Fork_SetAimFireAnimControl( 0.0f );
						pWeaponMortar->Mortar_Sling_Idle();
					}
				}
			}
		}

		break;

	case MORTAR_STATE_GRABBING_ROUND:
		if( !DeltaTime( ANIMTAP_RELOAD_MORTAR_GRAB_AMMO, FLoop_fPreviousLoopSecs, TRUE ) ) {
			// Still grabbing...

			fUnit = GetControlValue( ANIMCONTROL_RELOAD_MORTAR_GRAB_AMMO );
			if( fUnit < 1.0f ) {
				fUnit += 4.0f * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( fUnit, 1.0f );
				SetControlValue( ANIMCONTROL_RELOAD_MORTAR_GRAB_AMMO, fUnit );

				pWeaponMortar->Mortar_Fork_SetReloadAnimControl( fUnit );
			}

			if( m_fControls_Fire1 == 0.0f ) {
				_AbortMortarImmediately();
			}
		} else {
			// Done grabbing...

			m_nMortarState = MORTAR_STATE_PLACING_ROUND;
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_GRAB_AMMO, 0.0f );
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING, 1.0f );
			UpdateTime( ANIMTAP_RELOAD_MORTAR_PLACE_IN_SLING, 0.0f );

			pWeaponMortar->Mortar_Fork_AttachReloadAnim( &m_Anim.m_pLoadedBaseAnimInst[ ANIM_RELOAD_MORTAR_PLACE_IN_SLING ] );

			m_apWeapon[1]->Throwable_AttachGrenadeToOwnerBotBone( "Secondary_Fire" );
		}

		break;

	case MORTAR_STATE_PLACING_ROUND:
		if( !DeltaTime( ANIMTAP_RELOAD_MORTAR_PLACE_IN_SLING, FLoop_fPreviousLoopSecs, TRUE ) ) {
			// Still placing round...

			if( m_fControls_Fire1 == 0.0f ) {
				_AbortMortarImmediately();
			}
		} else {
			// Done placing...

			m_nMortarState = MORTAR_STATE_TAKING_UP_SLACK;
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING, 0.0f );
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK, 1.0f );
			UpdateTime( ANIMTAP_RELOAD_MORTAR_TAKE_UP_SLACK, 0.0f );

			pWeaponMortar->Mortar_Fork_AttachReloadAnim( &m_Anim.m_pLoadedBaseAnimInst[ ANIM_RELOAD_MORTAR_TAKE_UP_SLACK ] );
			pWeaponMortar->Mortar_Sling_TakeUpSlack();
		}

		break;

	case MORTAR_STATE_TAKING_UP_SLACK:
		if( DeltaTime( ANIMTAP_RELOAD_MORTAR_TAKE_UP_SLACK, FLoop_fPreviousLoopSecs, TRUE ) ) {
			// Done taking up the slack...

			m_nMortarState = MORTAR_STATE_PLACE_TO_AIM_TRANSITION;
			SetControlValue( ANIMCONTROL_AIM_MORTAR, 1.0f );
			m_fMortarUnitBlend = 1.0f;

			pWeaponMortar->Mortar_Fork_AttachAimFireAnim( &m_Anim.m_pLoadedBaseAnimInst[ ANIM_AIM_MORTAR ] );
		}

		break;

	case MORTAR_STATE_PLACE_TO_AIM_TRANSITION:
		UpdateUnitTime( ANIMTAP_AIM_MORTAR, pWeaponMortar->Mortar_GetUnitStretch() );

		m_fMortarUnitBlend -= 2.0f * FLoop_fPreviousLoopSecs;

		if( m_fMortarUnitBlend > 0.0f ) {
			fUnit = fmath_UnitLinearToSCurve( m_fMortarUnitBlend );
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK, fUnit );

			pWeaponMortar->Mortar_Fork_SetAimFireAnimControl( 1.0f - fUnit );

			if( m_fControls_Fire1 == 0.0f ) {
				_AbortMortarImmediately();
			}
		} else {
			m_nMortarState = MORTAR_STATE_AIMING;
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK, 0.0f );

			pWeaponMortar->Mortar_Fork_SetAimFireAnimControl( 1.0f );
		}

		break;

	case MORTAR_STATE_AIMING:
		if( m_fControls_Fire1 > 0.0f ) {
			UpdateUnitTime( ANIMTAP_AIM_MORTAR, pWeaponMortar->Mortar_GetUnitStretch() );
		} else {
			_AbortMortarImmediately();
		}

		break;

	case MORTAR_STATE_FIRING:
		fUnit = GetControlValue( ANIMCONTROL_AIM_MORTAR );
		if( fUnit > 0.0f ) {
			fUnit -= 6.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fUnit, 0.0f );
			SetControlValue( ANIMCONTROL_AIM_MORTAR, fUnit );
		}

		fUnit2 = GetControlValue( ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK );
		if( fUnit2 > 0.0f ) {
			fUnit2 -= 6.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fUnit2, 0.0f );
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK, fUnit2 );
		}

		if( (fUnit == 0.0f) && (fUnit2 == 0.0f ) ) {
			m_nMortarState = MORTAR_STATE_IDLE;
		}

		break;

	case MORTAR_STATE_REPLACE_ROUND_AFTER_JUMP:
		if( !DeltaTime( ANIMTAP_RELOAD_MORTAR_PLACE_IN_SLING, FLoop_fPreviousLoopSecs, TRUE ) ) {
			// Still placing round...

			fUnit = GetControlValue( ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING );
			if( fUnit < 1.0f ) {
				fUnit += 3.0f * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( fUnit, 1.0f );
				SetControlValue( ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING, fUnit );

				pWeaponMortar->Mortar_Fork_SetReloadAnimControl( fUnit );
			}

			if( m_fControls_Fire1 == 0.0f ) {
				_AbortMortarImmediately();
			}
		} else {
			// Done placing...

			m_nMortarState = MORTAR_STATE_TAKING_UP_SLACK;
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING, 0.0f );
			SetControlValue( ANIMCONTROL_RELOAD_MORTAR_TAKE_UP_SLACK, 1.0f );
			UpdateTime( ANIMTAP_RELOAD_MORTAR_TAKE_UP_SLACK, 0.0f );

			pWeaponMortar->Mortar_Fork_AttachReloadAnim( &m_Anim.m_pLoadedBaseAnimInst[ ANIM_RELOAD_MORTAR_TAKE_UP_SLACK ] );
			pWeaponMortar->Mortar_Sling_TakeUpSlack();
		}

		break;

	case MORTAR_STATE_JUMPING:
		if( m_fControls_Fire1 == 0.0f ) {
			_AbortMortarImmediately();
			break;
		}

		// NOTE: Fall into MORTAR_STATE_ABORTING...

	case MORTAR_STATE_ABORTING:
		if( m_fMortarAbortUnitControl1 > 0.0f ) {
			m_fMortarAbortUnitControl1 -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fMortarAbortUnitControl1, 0.0f );

			SetControlValue( m_nMortarAbortControlIndex1, m_fMortarAbortUnitControl1 );
		}

		if( m_fMortarAbortUnitControl2 > 0.0f ) {
			m_fMortarAbortUnitControl2 -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fMortarAbortUnitControl2, 0.0f );

			SetControlValue( m_nMortarAbortControlIndex2, m_fMortarAbortUnitControl2 );
		}

		if( m_fMortarAbortUnitControl1==0.0f && m_fMortarAbortUnitControl2==0.0f ) {
			if( m_nMortarState == MORTAR_STATE_ABORTING ) {
				m_nMortarState = MORTAR_STATE_IDLE;
				if (m_apWeapon[1])
					m_apWeapon[1]->Throwable_ReturnGrenade();
				break;
			}

			if( m_nWSState == WS_STATE_NONE ) {
				if( m_nJumpState == BOTJUMPSTATE_NONE ) {
					m_nMortarState = MORTAR_STATE_REPLACE_ROUND_AFTER_JUMP;
					UpdateTime( ANIMTAP_RELOAD_MORTAR_PLACE_IN_SLING, 0.0f );
					SetControlValue( ANIMCONTROL_RELOAD_MORTAR_PLACE_IN_SLING, 0.0f );

					pWeaponMortar->Mortar_Fork_AttachReloadAnim( &m_Anim.m_pLoadedBaseAnimInst[ ANIM_RELOAD_MORTAR_PLACE_IN_SLING ] );
					pWeaponMortar->Mortar_Fork_AttachAimFireAnim( NULL );
					pWeaponMortar->Mortar_Fork_SetReloadAnimControl( 0.0f );
					pWeaponMortar->Mortar_Fork_SetAimFireAnimControl( 0.0f );
					pWeaponMortar->Mortar_Sling_Idle();
				}
			}
		}

		break;

	default:
		FASSERT_NOW;
	}
}

#define _FIRE_ANIM_BLEND_RATE_LOWER		( 10.0f )

void CBotGlitch::_HandleFiringAnimations( void ) {
	f32 fUnitFireControlValue;

	// Primary fire...
	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		// blend in primary fire animation(s)
		if( m_fUnitFireCtlUpper < 1.0f ) {
			m_fUnitFireCtlUpper = 1.0f;
			SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
		}

		if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_STANDARD ) {
			m_fUnitAim = 1.0f;
		}


		if( m_fUnitFireCtlLower < 1.0f ) {
			m_fUnitFireCtlLower += FLoop_fPreviousLoopSecs * _FIRE_ANIM_BLEND_RATE_LOWER;
			FMATH_CLAMP_MAX1( m_fUnitFireCtlLower );
			SetControlValue( ANIMCONTROL_FIRE_1_LOWER, m_fUnitFireCtlLower );
		}

		// advance the animation if not one handed fire pose.  Otherwise, as long as necessary
		if( m_nBotFlags & BOTFLAG_PLAY_TETHER_VIBRATE ) {
			// Playing special tether zap vibration...

			f32 fUnitAnim;

			m_fUnitTetherShockVibration += FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitTetherShockVibration, 1.0f );

			fUnitAnim = 0.05f*fmath_RandomFloat() + 0.25f*m_fUnitTetherShockVibration;
			FMATH_CLAMP( fUnitAnim, 0.0f, 1.0f );

			UpdateUnitTime( ANIMTAP_FIRE_1_UPPER, fUnitAnim );

		} else if( m_apWeapon[0] && m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_STANDARD ) {
			// playing one handed fire animation
			m_fFireTimer += FLoop_fPreviousLoopSecs;

			// hold the pose until time is up, unless clip is empty
			if( (m_fFireTimer > _FIRE_ANIM_HOLD_TIME) || m_apWeapon[0]->IsClipEmpty() ) {
				FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
			}
		} else {
			// custom animation
			if( DeltaTime( ANIMTAP_FIRE_1_UPPER, FLoop_fPreviousLoopSecs * m_fFireAnimSpeedMult, TRUE ) ) {
				FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
				m_fUnitFireCtlUpper = 0.0f;
			}
		}

	} else {
		m_fFireTimer += FLoop_fPreviousLoopSecs;

		if( m_apWeapon[0] && (m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_STANDARD) ) {
			// handle animations for standard one handed weapons
			
			if( (m_fFireTimer < _FIRE_ANIM_READY_TIME) ) {
				if( m_fUnitFireCtlUpper > _FIRE_READY_BLEND ) {
					m_fUnitFireCtlUpper -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_OFF_TIME;
					FMATH_CLAMPMIN( m_fUnitFireCtlUpper, _FIRE_READY_BLEND );
					SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
				}
			} else {

				// blend out primary fire (upper)
				if( m_fUnitFireCtlUpper > 0.0f ) {
					// if control is above _FIRE_READY_BLEND, we are probably going into a different animation.  Blend out quickly
					if( m_fUnitFireCtlUpper > _FIRE_READY_BLEND ) {
                        m_fUnitFireCtlUpper -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_ON_TIME;        
						FMATH_CLAMP_MIN0( m_fUnitFireCtlUpper );
						SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
					} else {
						m_fUnitFireCtlUpper -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_OFF_TIME;
						FMATH_CLAMP_MIN0( m_fUnitFireCtlUpper );
						SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
					}
				}

				// blend out primary fire (lower)
				if( !DeltaTime( ANIMTAP_FIRE_1_UPPER, FLoop_fPreviousLoopSecs, TRUE  ) ) {
				} else {
					if( m_fUnitFireCtlLower > 0.0f ) {
						m_fUnitFireCtlLower -= FLoop_fPreviousLoopSecs * _FIRE_ANIM_OO_OFF_TIME;
						FMATH_CLAMP_MIN0( m_fUnitFireCtlLower );
						SetControlValue( ANIMCONTROL_FIRE_1_LOWER, m_fUnitFireCtlLower );
					}
				}
			}
		} else {
//			fUnitFireControlValue = GetControlValue( ANIMCONTROL_FIRE_1_UPPER );
			m_fUnitFireCtlUpper = GetControlValue( ANIMCONTROL_FIRE_1_UPPER );

			if( m_fUnitFireCtlUpper > 0.0f ) {
				m_fUnitFireCtlUpper -= m_pBotInfo_Weapon->fFire1BlendOutSpeed * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fUnitFireCtlUpper, 0.0f );
				SetControlValue( ANIMCONTROL_FIRE_1_UPPER, m_fUnitFireCtlUpper );
				if( m_bPlayLowerFireAnim ) {
					SetControlValue( ANIMCONTROL_FIRE_1_LOWER, m_fUnitFireCtlUpper );
				}

				if( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_MORTAR ) {
					CWeaponMortar *pWeaponMortar = (CWeaponMortar *)m_apWeapon[0];

					pWeaponMortar->Mortar_Fork_SetAimFireAnimControl( m_fUnitFireCtlUpper );
				}
			}

			m_fUnitFireCtlLower = m_fUnitFireCtlUpper;
		}
	}

	// Secondary fire...
	if( m_nThrowState != THROWSTATE_NONE ) {
		f32 fUnitTime;

		if( DeltaTime( ANIMTAP_FIRE_2_UPPER, 2.0f * FLoop_fPreviousLoopSecs, TRUE ) ) {
			fUnitTime = 1.0f;
			fUnitFireControlValue = 0.0f;
		} else {
			fUnitTime = GetUnitTime( ANIMTAP_FIRE_2_UPPER );

			if( fUnitTime < m_pBotInfo_Weapon->fThrowBlendInUnitTime ) {
				fUnitFireControlValue = fUnitTime * m_pBotInfo_Weapon->fOOThrowBlendInUnitTime;
			} else if( fUnitTime > m_pBotInfo_Weapon->fThrowBlendOutUnitTime ) {
				fUnitFireControlValue = 1.01f - (fUnitTime - m_pBotInfo_Weapon->fThrowBlendOutUnitTime) * m_pBotInfo_Weapon->fOOInvThrowBlendOutUnitTime;
			} else {
				fUnitFireControlValue = 1.0f;
			}
		}

		SetControlValue( ANIMCONTROL_FIRE_2_UPPER, fUnitFireControlValue );
		SetControlValue( ANIMCONTROL_FIRE_2_LOWER, fUnitFireControlValue );

		switch( m_nThrowState ) {
		case THROWSTATE_REACHING_FOR_AMMO:
			if( fUnitTime <= 0.25f ) {
				break;
			}

			// Attach ammo...
			m_apWeapon[1]->Throwable_AttachGrenadeToOwnerBotBone( "Secondary_Fire" );
			m_nThrowState = THROWSTATE_WINDING_UP;

			// Fall through to next state...

		case THROWSTATE_WINDING_UP:
			if( fUnitTime <= 0.63f ) {
				break;
			}

			// Attach ammo...
			m_apWeapon[1]->Throwable_ThrowGrenade_MountAimDirection();


			m_nThrowState = THROWSTATE_RECOVERING;
			// Fall through to next state...

		case THROWSTATE_RECOVERING:
			if( fUnitTime >= 1.0f ) {
				m_nThrowState = THROWSTATE_NONE;
			}
			break;
		};
	}

	if( (m_nScopeState != SCOPESTATE_NONE) && (m_nScopeState != SCOPESTATE_ABORTING) ) {
		if( (m_fFlipPitch != 0.0f) || !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_SCOPE_ENABLED) ) {
			AbortScopeMode();
		}
	}

	if( m_nScopeState != SCOPESTATE_LOOKING_THROUGH_SCOPE ) {
		m_apWeapon[0]->SetAttachedScope( NULL );
	} else {
		FASSERT( m_apWeapon[1] && m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE );
		m_apWeapon[0]->SetAttachedScope( (CWeaponScope *)m_apWeapon[1] );
	}

	switch( m_nScopeState ) {
	case SCOPESTATE_NONE:
		if( (m_fControls_PrevFrameFire2 == 0.0f) && (m_fControls_Fire2 > 0.0f) ) {
			// Player is trying to activate the scope...

			if( m_apWeapon[1] && (m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE) ) {
				// We have a scope...

				if( m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_SCOPE_ENABLED ) {
					// Primary weapon can be used with the scope...

					if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
						// We're not on a cable...

						if( (m_nWRState == WR_STATE_NONE) || !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) ) {
							// Either we're not reloading or the reload doesn't require the left hand...

							if( m_bFingerOnTrigger2 ) {
								// We're allowed to press the fire2 button...

								if( !UserAnim_IsLocked() ) {
									// User anim is not being played...

									if( IsReverseFacingForward() ) {
										// Enter scope mode...

										m_nScopeState = SCOPESTATE_REACHING_FOR_SCOPE;
										m_fScopeUnzoomHitpointAccumulator = 0.0f;
										m_fScopeUnzoomHitpointTimer = 0.0f;
									}
								}
							}
						}
					}
				}

				// This is a scope...

				if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_SCOPE_ENABLED) || (m_nJumpState == BOTJUMPSTATE_CABLE) ) {
					// Play reject sound...

					((CWeaponScope *)m_apWeapon[1])->PlayRejectSound();
				}
			}
		}

		break;

	case SCOPESTATE_REACHING_FOR_SCOPE:
		m_fUnitScope += 6.0f * FLoop_fPreviousLoopSecs;

		if( m_fUnitScope < 1.0f ) {
			// Still reaching for scope...
			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, m_fUnitScope );
		} else {
			// Done reaching for scope...
			m_fUnitScope = 1.0f;
			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, 1.0f );
			m_nScopeState = SCOPESTATE_ATTACHING_SCOPE;
		}

		break;

	case SCOPESTATE_ATTACHING_SCOPE:
		if( DeltaTime( ANIMTAP_SCOPE_ATTACHING, FLoop_fPreviousLoopSecs * 3.0f, TRUE ) ) {
			// Done attaching scope...
			m_nScopeState = SCOPESTATE_LOOKING_THROUGH_SCOPE;

			FASSERT( m_apWeapon[1] && m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE );
			((CWeaponScope *)m_apWeapon[1])->EnterZoomMode();
		}

		break;

	case SCOPESTATE_LOOKING_THROUGH_SCOPE:
		if( m_bFingerOnTrigger2 ) {
			// We're allowed to press the fire2 button...

			if( (m_nWRState == WR_STATE_NONE) || !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) ) {
				// Either we're not reloading or the reload doesn't require the left hand...

				if( (m_fControls_PrevFrameFire2 == 0.0f) && (m_fControls_Fire2 > 0.0f) ) {
					// Toggle zoom...

					FASSERT( m_apWeapon[1] && m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE );
					if( ((CWeaponScope *)m_apWeapon[1])->DigitalZoom_ToggleZoomLevel() ) {
						// Scope has left zoom mode...
						m_nScopeState = SCOPESTATE_RETURNING_TO_IDLE;
					}
				}
			}
		}

		break;

	case SCOPESTATE_RETURNING_TO_IDLE:
		m_fUnitScope -= FLoop_fPreviousLoopSecs;

		if( m_fUnitScope > 0.0f ) {
			// Still returning scope...

			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, m_fUnitScope );
			UpdateUnitTime( ANIMTAP_SCOPE_ATTACHING, m_fUnitScope );

			if( m_bFingerOnTrigger2 ) {
				// We're allowed to press the fire2 button...

				if( (m_nWRState == WR_STATE_NONE) || !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) ) {
					// Either we're not reloading or the reload doesn't require the left hand...

					if( (m_fControls_PrevFrameFire2 == 0.0f) && (m_fControls_Fire2 > 0.0f) ) {
						// Re-enter scope mode...
						m_nScopeState = SCOPESTATE_REATTACHING_SCOPE;
					}
				}
			}
		} else {
			// Done returning scope...

			m_fUnitScope = 0.0f;
			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, 0.0f );
			UpdateUnitTime( ANIMTAP_SCOPE_ATTACHING, 0.0f );
			m_nScopeState = SCOPESTATE_NONE;
		}

		break;

	case SCOPESTATE_REATTACHING_SCOPE:
		m_fUnitScope += 3.0f * FLoop_fPreviousLoopSecs;

		if( m_fUnitScope < 1.0f ) {
			// Still reattaching scope...

			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, m_fUnitScope );
			UpdateUnitTime( ANIMTAP_SCOPE_ATTACHING, m_fUnitScope );
		} else {
			// Done reattaching scope...

			m_fUnitScope = 1.0f;
			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, 1.0f );
			UpdateUnitTime( ANIMTAP_SCOPE_ATTACHING, 1.0f );

			m_nScopeState = SCOPESTATE_LOOKING_THROUGH_SCOPE;

			FASSERT( m_apWeapon[1] && m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE );
			((CWeaponScope *)m_apWeapon[1])->EnterZoomMode();
		}

		break;

	case SCOPESTATE_ABORTING:
		m_fUnitScope -= 4.0f * FLoop_fPreviousLoopSecs;

		if( m_fUnitScope > 0.0f ) {
			// Still aborting...

			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, m_fUnitScope );
		} else {
			// Done returning scope...

			m_fUnitScope = 0.0f;
			SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, 0.0f );
			m_nScopeState = SCOPESTATE_NONE;
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CBotGlitch::AbortScopeMode( BOOL bImmediate ) {
	CBot::AbortScopeMode( bImmediate );

	if( !bImmediate ) {
		if( (m_nScopeState != SCOPESTATE_NONE) && (m_nScopeState != SCOPESTATE_ABORTING) ) {
			if( m_apWeapon[1] && (m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE) ) {
				((CWeaponScope *)m_apWeapon[1])->LeaveZoomMode();
			}

			m_nScopeState = SCOPESTATE_ABORTING;
		}
	} else {
		m_nScopeState = SCOPESTATE_NONE;
		m_fUnitScope = 0.0f;
		SetControlValue( ANIMCONTROL_SCOPE_ATTACHING, 0.0f );
		UpdateTime( ANIMTAP_SCOPE_ATTACHING, 0.0f );

		if( m_apWeapon[1] && (m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE) ) {
			((CWeaponScope *)m_apWeapon[1])->LeaveZoomMode( TRUE );
		}
	}
}


void CBotGlitch::GetScopeViewPos( CFVec3A *pScopeViewPos_WS ) {
	FASSERT( IsCreated() );

	if( m_apWeapon[0]->Type() != CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER ) {
		pScopeViewPos_WS->Mul( m_MtxToWorld.m_vRight, 1.0f ).Add( m_MtxToWorld.m_vPos );
		pScopeViewPos_WS->y += 3.0f;
	} else {
		// Rocket launcher...
		pScopeViewPos_WS->Mul( m_MtxToWorld.m_vRight, 1.0f ).Add( m_MtxToWorld.m_vPos );
		pScopeViewPos_WS->y += 3.5f;
	}
}

void CBotGlitch::EnteringMechWork(CBot* pMech) {
	if (m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_MORTAR)
		_AbortMortarImmediately(FALSE);

	// put weapon away
	FASSERT( LeafTypeBit() & ENTITY_BIT_BOTGLITCH );

	m_nStowedWeaponIndex[0] = m_WeaponInv[0].m_nWeaponInvIndex;
	m_nStowedWeaponIndex[1] = m_WeaponInv[1].m_nWeaponInvIndex;

	m_pInventory->SetCurWeapon( 0, (u32) 0, FALSE, TRUE );
	m_pInventory->SetCurWeapon( 1, (u32) 0, FALSE, TRUE );
	
	// scared to do it, but here goes...
	FASSERT( IsPlayerBot() );
	Player_aPlayer[m_nPossessionPlayerIndex].m_Reticle.EnableDraw( FALSE );

	// disallow weapon switching
	if (m_nPossessionPlayerIndex >= 0)
	{
		CHud2* pHud = CHud2::GetHudForPlayer(m_nPossessionPlayerIndex);
		pHud->SetWSEnable( FALSE );
	}

	CBot::EnteringMechWork(pMech);
}

void CBotGlitch::ExitingMechWork(void) {
	FASSERT( LeafTypeBit() & ENTITY_BIT_BOTGLITCH );

	// deploy weapons
	m_pInventory->SetCurWeapon( 0, m_nStowedWeaponIndex[0], FALSE, TRUE );
	m_pInventory->SetCurWeapon( 1, m_nStowedWeaponIndex[1], FALSE, TRUE );

	m_nStowedWeaponIndex[0] = 0;
	m_nStowedWeaponIndex[1] = 0;

	
	// allow weapon switching
	if (m_nPossessionPlayerIndex >= 0)
	{
		CHud2* pHud = CHud2::GetHudForPlayer(m_nPossessionPlayerIndex);
		pHud->SetWSEnable( TRUE );
	}

	// scared to do it, but here goes...
	FASSERT( IsPlayerBot() );
	Player_aPlayer[m_nPossessionPlayerIndex].m_Reticle.EnableDraw( FALSE );


	CBot::ExitingMechWork();
}

void CBotGlitch::_HandleAimAnimations( void ) {
	f32 fUnitAimPitch, fControlVal;
	CFAnimFrame TorsoQuat, HeadQuat;

	if( m_apWeapon[0]->m_pInfo->nStanceType != CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST ) {
		// Make sure we're not using the torso-twist aim anim...

		fControlVal = GetControlValue( ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED );
		if( fControlVal > 0.0f ) {
			fControlVal -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fControlVal, 0.0f );
			SetControlValue( ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED, fControlVal );
		}
	}

	if( m_apWeapon[0]->m_pInfo->nStanceType != CWeapon::STANCE_TYPE_SHOULDER_MOUNT ) {
		// Make sure we're not using the shoulder-mounted aim anim...

		fControlVal = GetControlValue( ANIMCONTROL_AIM_SHOULDER_MOUNTED );
		if( fControlVal > 0.0f ) {
			fControlVal -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( fControlVal, 0.0f );
			SetControlValue( ANIMCONTROL_AIM_SHOULDER_MOUNTED, fControlVal );
		}
	}

	// Compute the amount we're aiming up/down (up=1, down=0, middle=0.5)...
	fUnitAimPitch = fmath_Div( m_fAimPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	if( !IsReverseFacingForward() ) {
		// smoothly blend from normal (0 to 1) to inverted (1 to 0) as torso turns around
		// for reverse facing mode

		// shift unit float to be zero-centered
		fUnitAimPitch -= 0.5f;

		// scale from normal through inverted via current reverse facing angle
		fUnitAimPitch *= fmath_Div( m_fReverseFacingYaw_MS - FMATH_DEG2RAD( 90.0f ), -FMATH_DEG2RAD( 90.0f ) );

		// restore to 0.5 centering
		fUnitAimPitch += 0.5f;
		FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );
	}

	// Update the torso-twist blend value...
	switch( m_apWeapon[0]->m_pInfo->nStanceType ) {
	case CWeapon::STANCE_TYPE_STANDARD:
		if( m_fUnitTorsoTwistBlend > 0.0f ) {
			m_fUnitTorsoTwistBlend -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
		}

		break;

	case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
		UpdateUnitTime( ANIMTAP_AIM_SHOULDER_MOUNTED, FMATH_FPOT( fUnitAimPitch, 0.1f, 0.9f ) );

		if( m_fUnitTorsoTwistBlend > 0.0f ) {
			m_fUnitTorsoTwistBlend -= 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
		}

		break;

	case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
		UpdateUnitTime( ANIMTAP_AIM_DUAL_WITH_TORSO_TWISTED, FMATH_FPOT( fUnitAimPitch, 0.1f, 0.9f ) );

		if( m_fUnitTorsoTwistBlend < 1.0f ) {
			m_fUnitTorsoTwistBlend += 2.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitTorsoTwistBlend, 1.0f );
		}

		break;

	case CWeapon::STANCE_TYPE_MORTAR:
		{
			FASSERT( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_MORTAR );
			CWeaponMortar *pWeaponMortar = (CWeaponMortar *)m_apWeapon[0];

			if( (pWeaponMortar->Mortar_GetUnitStretch() > 0.0f) && pWeaponMortar->Mortar_IsReadyToFire() && (m_nWSState == WS_STATE_NONE) && (m_nJumpState == BOTJUMPSTATE_NONE) && (m_nMortarState != MORTAR_STATE_ABORTING) ) {
				if( m_fUnitTorsoTwistBlend < 1.0f ) {
					m_fUnitTorsoTwistBlend += 3.0f * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMAX( m_fUnitTorsoTwistBlend, 1.0f );
				}
			} else {
				if( m_fUnitTorsoTwistBlend > 0.0f ) {
					m_fUnitTorsoTwistBlend -= 3.0f * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMIN( m_fUnitTorsoTwistBlend, 0.0f );
				}
			}
		}

		break;

	default:
		FASSERT_NOW;
	}

	TorsoQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-30.0f), FMATH_DEG2RAD(30.0f) ) + m_fUnitFireCtlLower * _FIRE_LEAN_ANGLE, CFVec3A::m_Null );

	if( m_fUnitTorsoTwistBlend == 0.0f ) {
		// No torso twist...
		// Torso quaternion has already been computed.
		// Compute head quaternion...

		HeadQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(-20.0f), FMATH_DEG2RAD(20.0f) ), CFVec3A::m_Null );

	} else {
		// We must introduce some torso twist...

		CFTQuatA *pTwistQuat, *pUntwistQuat;
		f32 fTorsoTwistYaw;

		if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_MORTAR ) {
			pTwistQuat = &m_MortarTorsoTwistQuat;
			pUntwistQuat = &m_MortarTorsoUntwistQuat;
			fTorsoTwistYaw = _MORTAR_TORSO_TWIST_YAW;
		} else {
			pTwistQuat = &m_Rocket1TorsoTwistQuat;
			pUntwistQuat = &m_Rocket1TorsoUntwistQuat;
			fTorsoTwistYaw = _TWO_HANDED_TORSO_TWIST_YAW;
		}

		if( m_fUnitTorsoTwistBlend == 1.0f ) {
			// 100% torso twist...

			// Compute the torso twist quaternion...
			TorsoQuat.Mul( *pTwistQuat );

			// Compute the head quaternion...
			CFAnimFrame TempQuat;
			TempQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(20.0f), FMATH_DEG2RAD(-20.0f) ), CFVec3A::m_Null );
			HeadQuat.Mul( TempQuat, *pUntwistQuat );
		} else {
			// Partial torso twist...

			// Compute the torso twist quaternion...
			CFAnimFrame TorsoTwistQuat;
			TorsoTwistQuat.BuildQuat( CFVec3A::m_UnitAxisY, fmath_UnitLinearToSCurve(m_fUnitTorsoTwistBlend) * fTorsoTwistYaw, CFVec3A::m_Null );
			TorsoQuat.Mul( TorsoTwistQuat );

			// Compute the head quaternion...
			TorsoTwistQuat.Negate();
			CFAnimFrame TempQuat;
			TempQuat.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, FMATH_DEG2RAD(20.0f), FMATH_DEG2RAD(-20.0f) ), CFVec3A::m_Null );
			HeadQuat.Mul( TempQuat, TorsoTwistQuat );
		}
	}

	// Apply head and torso quaternions...
	m_AnimManFrameAim.UpdateFrame( BONE_HEAD, HeadQuat );
	m_AnimManFrameAim.UpdateFrame( BONE_TORSO, TorsoQuat );
}


void CBotGlitch::_HandleWeaponAnimations( void ) {
	_HandleWeaponSwitchingAnimations();
	HandleWeaponReloadingAnimations();
	_HandleFiringAnimations();
	_HandleAimAnimations();
	_HandleRecoil();
}


void CBotGlitch::_WeaponOrUpgradeLevelMayHaveChanged( void ) {
	if( m_nWRState != WR_STATE_NONE ) {
		// Snap out of any weapon reloading animation...

		m_nWRState = WR_STATE_NONE;
		SetControlValue( ANIMCONTROL_RELOAD_ROCKET1_GRAB_AMMO, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_ROCKET1_DROP_IN, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_ROCKET1_FINISH, 0.0f );

		SetControlValue( ANIMCONTROL_RELOAD_TETHER_PREPARE_TO_PUMP, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_TETHER_PUMP, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_TETHER_GRAB_AMMO, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_TETHER_DROP_IN, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_TETHER_FINISH, 0.0f );

		SetControlValue( ANIMCONTROL_RELOAD_SINGLE_ARM, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_SINGLE_BODY, 0.0f );

		SetControlValue( ANIMCONTROL_RELOAD_CLIP_EJECT_OLD, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_CLIP_GRAB_NEW, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_CLIP_INSERT_NEW, 0.0f );
		SetControlValue( ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW, 0.0f );
	}

	m_fFireAnimSpeedMult = 1.0f;

	switch( m_apWeapon[0]->m_pInfo->nStanceType ) {
	case CWeapon::STANCE_TYPE_STANDARD:
		if( GetAttachedAnim( ANIMTAP_FIRE_1_UPPER ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_PRIMARY ] ) {
			AttachAnim( ANIMTAP_FIRE_1_UPPER, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_PRIMARY ] );
			UpdateBoneMask( ANIMTAP_FIRE_1_UPPER, m_aBoneEnableIndices_TapFire1, TRUE );
		}
		m_bPlayLowerFireAnim = TRUE;

		break;

	case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
		if( GetAttachedAnim( ANIMTAP_FIRE_1_UPPER ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET1 ] ) {
			AttachAnim( ANIMTAP_FIRE_1_UPPER, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET1 ] );
			UpdateBoneMask( ANIMTAP_FIRE_1_UPPER, m_aBoneEnableIndices_FireRocket1, TRUE );
		}
		m_fFireAnimSpeedMult = 1.5f;
		m_bPlayLowerFireAnim = FALSE;

		break;

	case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
		if( GetAttachedAnim( ANIMTAP_FIRE_1_UPPER ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET23 ] ) {
			AttachAnim( ANIMTAP_FIRE_1_UPPER, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_ROCKET23 ] );
			UpdateBoneMask( ANIMTAP_FIRE_1_UPPER, m_aBoneEnableIndices_FireRocket23, TRUE );
		}
		m_bPlayLowerFireAnim = FALSE;

		break;

	case CWeapon::STANCE_TYPE_MORTAR:
		m_nMortarState = MORTAR_STATE_IDLE;

		if( GetAttachedAnim( ANIMTAP_FIRE_1_UPPER ) != &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_MORTAR ] ) {
			AttachAnim( ANIMTAP_FIRE_1_UPPER, &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_MORTAR ] );
			UpdateBoneMask( ANIMTAP_FIRE_1_UPPER, m_aBoneEnableIndices_FireMortar, TRUE );
		}
		m_bPlayLowerFireAnim = FALSE;

		FASSERT( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_MORTAR );
		((CWeaponMortar *)m_apWeapon[0])->Mortar_Fork_Idle();
		((CWeaponMortar *)m_apWeapon[0])->Mortar_Sling_Idle();

		break;

	default:
		FASSERT_NOW;
	}

	_ResetRecoilSummer();
}


CFMtx43A *CBotGlitch::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotGlitch::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}

void CBotGlitch::_UpdateMatrices( void ) {
	//if( m_nBotFlags & BOTFLAG_GLUED_TO_PARENT ) {
	//	return;
	//}


	CFMtx43A FlipMtx, MeshMtx, NewMtx, EntityMtxToWorld;

	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS + m_fGrabbedHeadYaw );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	// Update xfm...
	if( m_fFlipPitch ) {
		CFMtx43A::m_XlatRotX.m_vPos.Set( m_pBotInfo_Jump->fFlipOriginX, m_pBotInfo_Jump->fFlipOriginY, m_pBotInfo_Jump->fFlipOriginZ );

		if( m_fFlipPitch >= 0.0f ) {
			CFMtx43A::m_XlatRotX.SetRotationX( fmath_UnitLinearToSCurve( m_fFlipPitch * (1.0f/FMATH_2PI) ) * FMATH_2PI );
		} else {
			CFMtx43A::m_XlatRotX.SetRotationX( fmath_UnitLinearToSCurve( m_fFlipPitch * (-1.0f/FMATH_2PI) ) * -FMATH_2PI );
		}

		CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( CFMtx43A::m_XlatRotX.m_vPos );

		FlipMtx.Mul( CFMtx43A::m_XlatRotX, CFMtx43A::m_Xlat );

		MeshMtx.Mul( CFMtx43A::m_XlatRotY, FlipMtx );
		m_pWorldMesh->m_Xfm.BuildFromMtx( MeshMtx );

		CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS );
	} else if( m_pCableHook ) {
		// We are on a zipline...
		CFMtx43A RotMtx;

		ZipLineVerletWork( m_pLeftHandSecondaryFireBonePos, &RotMtx );

		// Build the matrix to rotate about our hand position
		RotMtx.m_vPos.Set( m_ZipHandPos_MS );
		CFMtx43A::m_Xlat.m_vPos.ReceiveNegative( RotMtx.m_vPos );
		FlipMtx.Mul( RotMtx, CFMtx43A::m_Xlat );

		// Rotate the offset matrix by our yaw
		CFMtx43A::m_XlatRotY.m_vPos.Zero();
		FlipMtx.Mul( CFMtx43A::m_XlatRotY );

		CFMtx43A::m_Xlat.m_vPos = m_MountPos_WS;
		MeshMtx.Mul( CFMtx43A::m_Xlat, FlipMtx );
		m_pWorldMesh->m_Xfm.BuildFromMtx( MeshMtx );
	} else if ( m_pCurMech && m_pCurMech->m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_PILLBOX) {
		// Driving...
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
	} else {
		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	}

	m_pWorldMesh->UpdateTracker();


	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		// Update bone matrix palette...
		ComputeMtxPalette( FALSE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	if( m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	} else {
		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	}

	if (m_bGrabbedByHead)
	{
		CFMtx43A* pHeadShouldBeMtx =  m_pGrabberBot->m_pWorldMesh->GetBoneMtxPalette()[m_nGrabberBoneIndex];
		CFMtx43A* pHeadIsAtMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexHead];
		m_MtxToWorld.m_vPos.Add(pHeadShouldBeMtx->m_vPos).Sub(pHeadIsAtMtx->m_vPos);
		Relocate_RotXlatFromUnitMtx_WS(&m_MtxToWorld);
	}

	if (!(m_nBotFlags & BOTFLAG_GLUED_TO_PARENT)) {
		EntityMtxToWorld.Identity();
		EntityMtxToWorld.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
		EntityMtxToWorld.m_vPos = m_MountPos_WS;
		Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
	} else if ( m_nBotFlags & BOTFLAG_FORCED_PANIC ) {
		// Glitch is being carried by a loader (multiplayer). Add his yaw to 
		// his current transform before relocating children.
		CFVec3A vPos = m_MtxToWorld.m_vPos;
		EntityMtxToWorld.Identity();
		EntityMtxToWorld.SetRotationY( m_fMountYaw_WS, 1.0f );
		EntityMtxToWorld.m_vPos = vPos;
		Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
	} else {
		RelocateAllChildren();
	}
}


// Returns TRUE if Glitch's finger is on his primary weapon's trigger.
// In other words, TRUE is returned if Glitch is in a state where he
// can pull the trigger.
BOOL CBotGlitch::_IsFingerOnTrigger1( void ) const {
	BOOL bFingerIsOnTrigger = FALSE;

	if( m_eMeleeState != MELEESTATE_NONE ) {
		return FALSE;
	}

	if( m_nBotFlags & BOTFLAG_REVERSE_FACING) {
		return FALSE;
	}

	if( m_nWSState==WS_STATE_NONE || m_nWSState==WS_STATE_RELEASING ) {
		// We're not switching or reloading the weapon...

		if( (m_fFlipPitch == 0.0f) || (fmath_Abs(m_fLegsYaw_MS) > (0.8f * FMATH_HALF_PI)) ) {
			//We're not flipping...

			switch( m_apWeapon[0]->m_pInfo->nGrip ) {
			case CWeapon::GRIP_RIGHT_ARM:
				bFingerIsOnTrigger = TRUE;
				break;

			case CWeapon::GRIP_BOTH_ARMS:
				if( m_nJumpState == BOTJUMPSTATE_NONE ) {
					f32 fLandBlend = GetControlValue( ANIMCONTROL_JUMP_LAND_UPPER );

					if( fLandBlend == 0.0f ) {
						if( m_nState == STATE_GROUND ) {
							bFingerIsOnTrigger = TRUE;
						} else {
							if( m_fUnitFlyBlend == 0.0f ) {
								bFingerIsOnTrigger = TRUE;
							}
						}
					}
				}

				// if we're in the process of throwing a grenade, can't shoot a two handed wpn
				if( GetControlValue( ANIMCONTROL_FIRE_2_UPPER ) != 0.0f ) {
					bFingerIsOnTrigger = FALSE;
				}

				break;

			default:
				FASSERT_NOW;
			};
		}	//flipping
	}

	return bFingerIsOnTrigger;
}


// Returns TRUE if Glitch's finger is on his secondary weapon's trigger.
// In other words, TRUE is returned if Glitch is in a state where he
// can pull the trigger.
BOOL CBotGlitch::_IsFingerOnTrigger2( void ) const {
	BOOL bFingerIsOnTrigger = FALSE;

	if( m_nWSState==WS_STATE_NONE || m_nWSState==WS_STATE_RELEASING ) {
		// We're not switching weapons...

		//if( m_apWeapon[0]->m_pInfo->nGrip == CWeapon::GRIP_RIGHT_ARM ) {
			// Primary weapon is not two-handed...

			if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
				// We're not on a cable...

				//if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) || (m_nWRState == WR_STATE_NONE) ) {
				//	// Primary weapon is either auto-reloading, or is not currently reloading...

					if( (m_apWeapon[0]->Type() != CWeapon::WEAPON_TYPE_MORTAR) || (m_nMortarState == MORTAR_STATE_IDLE) ) {
						bFingerIsOnTrigger = TRUE;
					}
				//}
			}
		//}
	}

	return bFingerIsOnTrigger;
}


void CBotGlitch::_HandleWeaponFiring( void ) {
	f32 fTrig1, fTrig2;
	u32 nFireBitsPrimary;

	m_bFingerOnTrigger1 = _IsFingerOnTrigger1();
	m_bFingerOnTrigger2 = _IsFingerOnTrigger2();

	if( m_bFingerOnTrigger1 ) {
		fTrig1 = m_fControls_Fire1;
		EnableAiming( TRUE );
	} else {
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		EnableAiming( FALSE );
		fTrig1 = 0.0f;
		
	}

	fTrig2 = m_bFingerOnTrigger2 ? m_fControls_Fire2 : 0.0f;

	if( m_pRoboBuddy ) {
		nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS, m_pRoboBuddy->GetFirePos() );
	} else {
		nFireBitsPrimary = m_apWeapon[0]->TriggerWork( fTrig1, fTrig2, &m_TargetedPoint_WS );
	}

	if( nFireBitsPrimary ) {
		// We fired the primary weapon...
		JustFired();
		m_fFireTimer = 0.0f;
		FASSERT( m_apWeapon[0] != NULL );
		FASSERT_UNIT_FLOAT( m_apWeapon[0]->m_pInfo->fUnitRecoil );
		_Recoil( m_apWeapon[0]->m_pInfo->fUnitRecoil );

		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_TETHER_VIBRATE );

		if( m_apWeapon[0]->Type() != CWeapon::WEAPON_TYPE_MORTAR ) {
			ZeroTime( ANIMTAP_FIRE_1_UPPER );
		} else {
			// Mortar...
			CWeaponMortar *pWeaponMortar = (CWeaponMortar *)m_apWeapon[0];

			f32 fUnit;

			fUnit = pWeaponMortar->Mortar_GetUnitLaunchSpeed();
			fUnit *= fUnit * fUnit * fUnit;

			m_fFireAnimSpeedMult = fUnit;
			FMATH_CLAMP( m_fFireAnimSpeedMult, 0.1f, 1.0f );

			fUnit = 1.0f - fUnit;

			UpdateUnitTime( ANIMTAP_FIRE_1_UPPER, fUnit );

			m_nMortarState = MORTAR_STATE_FIRING;
			pWeaponMortar->Mortar_Fork_AttachAimFireAnim( &m_Anim.m_pLoadedBaseAnimInst[ ANIM_FIRE_MORTAR ] );
			pWeaponMortar->Mortar_Fork_AttachReloadAnim( NULL );
			pWeaponMortar->Mortar_Fork_SetAimFireAnimControl( 1.0f );
			pWeaponMortar->Mortar_Fork_SetReloadAnimControl( 0.0f );
			pWeaponMortar->Mortar_Sling_Release( TRUE );

			m_apWeapon[1]->Throwable_ThrowGrenade_MountAimDirection( pWeaponMortar->Mortar_GetUnitLaunchSpeed() * 200.0f );
		}

	} else {
		// Primary weapon not fired...

		// Do special tether zapping...
		if( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_TETHER ) {
			CWeaponTether *pTether = (CWeaponTether *)m_apWeapon[0];

			if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM) ) {
				if( pTether->IsTetherFiringZapPulse() ) {
					FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM | BOTFLAG_PLAY_TETHER_VIBRATE );
					ZeroTime( ANIMTAP_FIRE_1_UPPER );
					m_fUnitTetherShockVibration = 0.0f;
				}
			} else {
				if( (m_nBotFlags & BOTFLAG_PLAY_TETHER_VIBRATE) && !pTether->IsTetherFiringZapPulse() ) {
					FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM | BOTFLAG_PLAY_TETHER_VIBRATE );
				}
			}
		}
	}

	if( m_apWeapon[1] ) {
		// Copy targeted entity from primary to secondary weapon
		// (done so that cleaner could use targeted entity info)
		//Commented out because the CBOT::humantargeting logic takes into account
		//if we have a secondary weapon that needs targeting information.

//		m_apWeapon[1]->SetTargetedEntity( m_apWeapon[0]->GetTargetedEntity() );

		if( m_apWeapon[1]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_THROWABLE ) {
			// Secondary is throwable...

			if( m_nThrowState != THROWSTATE_NONE ) {
				fTrig2 = 0.0f;
			}

			// NKM - We don't go into the throw state for the wrench, it just takes us apart.
			if( m_apWeapon[1]->Throwable_TriggerWork( fTrig2 ) && m_apWeapon[1]->Type() != CWeapon::WEAPON_TYPE_WRENCH ) {
			//if( m_apWeapon[1]->Throwable_TriggerWork( fTrig2 ) ) {
				// We can throw the grenade/cleaner...
				AbortReloadImmediately();

				m_nThrowState = THROWSTATE_REACHING_FOR_AMMO;
				ZeroTime( ANIMTAP_FIRE_2_UPPER );
			}
		}
	}
}


void CBotGlitch::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	BOOL bAntennaBone;
	CBotGlitch *pBotGlitch = (CBotGlitch *)m_pCollBot;

	// let the part mgr try first
	FASSERT( pBotGlitch->m_pPartMgr );
	FASSERT( pBotGlitch->m_pPartMgr->IsCreated() );
	if( ((CBotGlitch*)m_pCollBot)->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	}

	if( pBotGlitch->BotInPieces_InPiecesAnimBoneCB( rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	}

	PROTRACK_BEGINBLOCK("GlitchAntenna");
	bAntennaBone = pBotGlitch->m_Antenna.AnimateBone( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
	PROTRACK_ENDBLOCK();

	if( bAntennaBone ) {
		// antenna returns TRUE if it animated the bone.
	}
	else if( nBoneIndex == (u32)pBotGlitch->m_nBoneIndexGroin ) {
		// Groin
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	} else if( nBoneIndex == (u32)pBotGlitch->m_nBoneIndexTorso ) {
		// Torso
		CFMtx43A WorldMtx;
		CFQuatA Quat;

		Quat.BuildQuat( m_GroinVecY_WS,/* pBotGlitch->m_fAimDeltaYaw_MS*/ - pBotGlitch->m_fLegsYaw_MS );
		WorldMtx.Mul( rParentMtx, rBoneMtx );

		if( !m_pCollBot->IsReverseFacingForward() ) {
			// Rotate torso when in reverse facing mode...
			CFQuatA RevQuat;
			RevQuat.BuildQuat( m_GroinVecY_WS, fmath_UnitLinearToSCurve( m_pCollBot->m_fReverseFacingYaw_MS * ( 1.0f / FMATH_DEG2RAD( 180.0f ) ) ) * FMATH_DEG2RAD( 180.0f ) );
			Quat.Mul( RevQuat );
		}

		rNewMtx.m_vPos = WorldMtx.m_vPos;
		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );

		m_TorsoVecY_WS = rNewMtx.m_vUp;

	} else if( nBoneIndex == (u32)((CBotGlitch*)m_pCollBot)->m_nBoneIndexArmLower )  {
		((CBotGlitch*)m_pCollBot)->_AimLowerArm( rNewMtx, rParentMtx, rBoneMtx );

	} else if ( nBoneIndex == (u32)((CBotGlitch*)m_pCollBot)->m_nBoneIndexArmElbow ) {
		((CBotGlitch*)m_pCollBot)->_AimElbow( rNewMtx, rParentMtx, rBoneMtx );

	} else if ( nBoneIndex == (u32)((CBotGlitch*)m_pCollBot)->m_nBoneIndexHead) {
		// Head
		if (!((CBotGlitch*)m_pCollBot)->m_bGrabbedByHead)
		{
			m_pCollBot->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
		}
		else
		{
			// Head
			CFMtx43A HeadMtx;
			HeadMtx.m_vRight = rBoneMtx.m_vRight;
			HeadMtx.m_vUp = rBoneMtx.m_vUp;
			HeadMtx.m_vFront = rBoneMtx.m_vFront;

			HeadMtx.RotateY(-((CBotGlitch*)m_pCollBot)->m_fGrabbedHeadYaw);

			HeadMtx.m_vPos = rBoneMtx.m_vPos;

			rNewMtx.Mul( rParentMtx, HeadMtx);
		}
	}

	// This must be last
	pBotGlitch->BotInPieces_RecoverAnimBoneCB( rNewMtx, rParentMtx, rBoneMtx );

	//// This must be last
	//// Blend between where the animation wants to put our pieces and where the callback
	//// wants to put our pieces
	//if( pBotGlitch->m_bInPieces && pBotGlitch->m_fUnitPiecesBlend <= _START_BLEND_TIME ) {
	//	CFVec3A DestPos, SrcPos;
	//	CFQuatA SrcQuat, DestQuat, Slerp;

	//	DestPos = rNewMtx.m_vPos;
	//	DestQuat.BuildQuat( rNewMtx );

	//	rNewMtx.Mul( rParentMtx, rBoneMtx );
	//	SrcPos = rNewMtx.m_vPos;
	//	SrcQuat.BuildQuat( rNewMtx );

	//	Slerp.ReceiveSlerpOf( pBotGlitch->m_fUnitPiecesBlend * _OO_START_BLEND_TIME, DestQuat, SrcQuat );
	//	Slerp.BuildMtx( rNewMtx );
	//	rNewMtx.m_vPos.Lerp( pBotGlitch->m_fUnitPiecesBlend * _OO_START_BLEND_TIME, DestPos, SrcPos );
	//}
}

#define _START_BLEND_TIME		( 0.33f )
#define _OO_START_BLEND_TIME	( 1.0f / _START_BLEND_TIME )


void CBotGlitch::_AnimBoneCallbackSpyVsSpy( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	// let the part mgr try first
	FASSERT( ((CBotGlitch *)m_pCollBot)->m_pPartMgr );
	FASSERT( ((CBotGlitch *)m_pCollBot)->m_pPartMgr->IsCreated() );

	if( ((CBotGlitch*)m_pCollBot)->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	}

	BOOL bAntennaBone;
	CBotGlitch *pBotGlitch = (CBotGlitch *) m_pCollBot;

	// If we are in pieces
	if( pBotGlitch->m_bInPieces ) {
		// If we are on the ground, falling down or we are in the early stages of building up
		if( pBotGlitch->m_bOnGround ||
			pBotGlitch->m_bFallDown ||
			pBotGlitch->m_fUnitPiecesBlend > _START_BLEND_TIME ) {

			if( !( CSpyVsSpy::m_pCommonData->m_bHaveHead || CSpyVsSpy::m_pCommonData->m_bHaveLegs || CSpyVsSpy::m_pCommonData->m_bHaveTorso ) ) {
				rNewMtx.Mul( rParentMtx, rBoneMtx );
				return;
			}
		}
	}

	PROTRACK_BEGINBLOCK("GlitchAntenna");
	bAntennaBone = pBotGlitch->m_Antenna.AnimateBone( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
	PROTRACK_ENDBLOCK();

	if( bAntennaBone ) {
		// antenna returns TRUE if it animated the bone.
	} else if( nBoneIndex == (u32)((CBotGlitch *)m_pCollBot)->m_nBoneIndexGroin ) {
		// Groin
		if( CSpyVsSpy::m_pCommonData->m_bHaveLegs ) {
			rNewMtx = CSpyVsSpy::m_pCommonData->m_LegsMtx;
		} else {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
			m_GroinVecY_WS = rNewMtx.m_vUp;

			CSpyVsSpy::m_pCommonData->m_LegsStartQuat.BuildQuat( rNewMtx );
			CSpyVsSpy::m_pCommonData->m_LegsStartPos = rNewMtx.m_vPos;
		}
	} else if( nBoneIndex == (u32)((CBotGlitch *)m_pCollBot)->m_nBoneIndexTorso ) {
		if( CSpyVsSpy::m_pCommonData->m_bHaveTorso ) {
			rNewMtx = CSpyVsSpy::m_pCommonData->m_TorsoMtx;
		} else {
			// Torso
			CFMtx43A WorldMtx;
			CFQuatA Quat;
			CBotGlitch *pBotGlitch = (CBotGlitch *)m_pCollBot;

			Quat.BuildQuat( m_GroinVecY_WS,/* pBotGlitch->m_fAimDeltaYaw_MS*/ - pBotGlitch->m_fLegsYaw_MS );
			WorldMtx.Mul( rParentMtx, rBoneMtx );

			if( !m_pCollBot->IsReverseFacingForward() ) {
				// Rotate torso when in reverse facing mode...
				CFQuatA RevQuat;
				RevQuat.BuildQuat( m_GroinVecY_WS, fmath_UnitLinearToSCurve( m_pCollBot->m_fReverseFacingYaw_MS * ( 1.0f / FMATH_DEG2RAD( 180.0f ) ) ) * FMATH_DEG2RAD( 180.0f ) );
				Quat.Mul( RevQuat );
			}

			rNewMtx.m_vPos = WorldMtx.m_vPos;
			Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
			Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
			Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );

			m_TorsoVecY_WS = rNewMtx.m_vUp;

			CSpyVsSpy::m_pCommonData->m_TorsoStartQuat = Quat;
			CSpyVsSpy::m_pCommonData->m_TorsoStartPos = rNewMtx.m_vPos;
		}
	} else if( nBoneIndex == (u32)((CBotGlitch*)m_pCollBot)->m_nBoneIndexArmLower )  {
		((CBotGlitch*)m_pCollBot)->_AimLowerArm( rNewMtx, rParentMtx, rBoneMtx );

	} else if ( nBoneIndex == (u32)((CBotGlitch*)m_pCollBot)->m_nBoneIndexArmElbow ) {
		((CBotGlitch*)m_pCollBot)->_AimElbow( rNewMtx, rParentMtx, rBoneMtx );

	} else {
		// Head
		if( CSpyVsSpy::m_pCommonData->m_bHaveHead ) {
			rNewMtx = CSpyVsSpy::m_pCommonData->m_HeadMtx;
		} else {
			m_pCollBot->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, TRUE );

			CSpyVsSpy::m_pCommonData->m_HeadStartQuat.BuildQuat( rNewMtx );
			CSpyVsSpy::m_pCommonData->m_HeadStartPos = rNewMtx.m_vPos;
		}
	}

	// This must be last
	// Blend between where the animation wants to put our pieces and where the callback
	// wants to put our pieces
	if( pBotGlitch->m_bInPieces && pBotGlitch->m_fUnitPiecesBlend <= _START_BLEND_TIME ) {
		if( !( CSpyVsSpy::m_pCommonData->m_bHaveHead || CSpyVsSpy::m_pCommonData->m_bHaveLegs || CSpyVsSpy::m_pCommonData->m_bHaveTorso ) ) {
			CFVec3A DestPos, SrcPos;
			CFQuatA SrcQuat, DestQuat, Slerp;

			DestPos = rNewMtx.m_vPos;
			DestQuat.BuildQuat( rNewMtx );

			rNewMtx.Mul( rParentMtx, rBoneMtx );
			SrcPos = rNewMtx.m_vPos;
			SrcQuat.BuildQuat( rNewMtx );

			Slerp.ReceiveSlerpOf( pBotGlitch->m_fUnitPiecesBlend * _OO_START_BLEND_TIME, DestQuat, SrcQuat );
			Slerp.BuildMtx( rNewMtx );
			rNewMtx.m_vPos.Lerp( pBotGlitch->m_fUnitPiecesBlend * _OO_START_BLEND_TIME, DestPos, SrcPos );
		}
	}
}

// create weapon inventory from ItemInst inventory
BOOL CBotGlitch::_InitWeaponInventory( CBotBuilder* pBuilder ) {

	u32 i;

	if( m_nOwnerPlayerIndex == -1 ) {
		// They're not a player bot so we don't want to go through the whole inventory deal,
		// but we do need them to have weapons.
		// They also don't need a robo buddy.

		// Set primary and secondary weapon...
		for( i = 0; i < 2; i++ ) {
			m_apWeapon[i] = CBotBuilder::AllocNPCWeaponOfType(pBuilder->m_anNPCWeaponType[i]);
			if( m_apWeapon[i] == NULL ) {
				DEVPRINTF( "CBotGrunt::ClassHierarchyBuild() : Error instantiating CWeapon %d.\n", i );
				goto _ExitWithError;
			}
			if( !CBotBuilder::CreateNPCWeaponOfType(m_apWeapon[i], pBuilder->m_anNPCWeaponType[i])) {
				DEVPRINTF( "CBotGrunt::ClassHierarchyBuild() : Could not create CWeapon %d.\n", i );
				goto _ExitWithError;
			}

			m_apWeapon[i]->SetUpgradeLevel(0);	  // 3 is special level for npc

			if( pBuilder->m_anNPCWeaponType[i] == CBotBuilder::_NPC_WEAPON_SPEW ) {
				m_apWeapon[i]->SetClipAmmo( 30 );
				m_apWeapon[i]->SetReserveAmmo( CWeapon::INFINITE_AMMO );
			} else if( pBuilder->m_anNPCWeaponType[i] == CBotBuilder::_NPC_WEAPON_FLAMER ) {
				m_apWeapon[i]->SetClipAmmo( 200 );
				m_apWeapon[i]->SetReserveAmmo( CWeapon::INFINITE_AMMO );
			}

			m_apWeapon[i]->EnableAutoWork( FALSE );
			m_apWeapon[i]->RemoveFromWorld();
			m_apWeapon[i]->SetOwner(this);
			m_apWeapon[i]->SetDesiredState( CWeapon::STATE_DEPLOYED );

			// Initialize this structure only Glitch has it, must be something pretty Glitch specific (read Justin)
			m_WeaponInv[i].m_apWeapon[0] = m_apWeapon[i];
			m_WeaponInv[i].m_nWeaponInvIndex = 0;
			m_WeaponInv[i].m_nWeaponInvCount = 1;
			m_WeaponInv[i].m_nWeaponSwitchToIndex = 255; // Not switching weapon currently.
		}

		// Primary must be attached
		m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY] );

		// Make primary weapon pass along damage to upper-right arm...
		m_apWeapon[0]->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_SHOULDER] ) );

		m_pRoboBuddy = NULL;

		// Clear pointers to currently selected weapons to make sure the _changeweaponindex does something...
		m_apWeapon[0] = m_apWeapon[1] = NULL;

		if( m_WeaponInv[0].m_nWeaponInvCount ) {
			_ChangeWeaponIndex( 0, 0 );
		}

		if( m_WeaponInv[1].m_nWeaponInvCount ) {
			_ChangeWeaponIndex( 1, 0 );
		}

	} else {

		m_pInventory = CPlayer::GetInventory(m_nOwnerPlayerIndex);
		if( m_pInventory == NULL ) {
			// What is going on here?!  This shouldn't happen!
			DEVPRINTF( "CBotGlitch::_InitInventory() : Could not get inventory from CPlayer.\n" );
			return FALSE;
		}

		// At this point, we have a valid, current, accurate inventory object.
		m_pInventory->m_pUser = this;
		m_pInventory->m_pfcnCallback = _InventoryCallback;

#if 0
		if( m_nOwnerPlayerIndex != -1 ) {
			// JUSTIN: This is temp code to give Glitch an EUK that operates stuff.
			m_pInventory->m_aoItems[INVPOS_EUK].m_nClipAmmo = 4;

			for( i=0; i<4; ++i ) {
				m_pInventory->m_auEUKFlags[i] = (u16)(-1);
			}
		}
#endif

		CWeapon *pWeap;
		s32 uCurWpnIdx;

		for( i=0; i<2; ++i ) {
			// step through primary and secondary weapons

			for( uCurWpnIdx=0; uCurWpnIdx < m_pInventory->m_auNumWeapons[i]; ++uCurWpnIdx ) {
				// step through all weapons in ItemInst inventory

				// create a weapon object from the ItemInst parameters
				pWeap = m_pInventory->m_aoWeapons[i][uCurWpnIdx].MakeWeapon();
				if( pWeap == NULL )
				{
					// The game will eventually not run if this is hit.  Believe
					// me I wasted enough time on it...  JLAFLEUR
					continue;
				}

				// attach weapon to weapon inventory
				m_WeaponInv[i].m_apWeapon[uCurWpnIdx] = pWeap;

				// set weapon states
				pWeap->EnableAutoWork( FALSE );

				//RUSSNOTE - We want to remove this object from the world immediately, because if we wait for the
				// WorldRemoveCallback to remove the item from the world, when it does get removed, it will DISABLE
				// the reticle.  This is a problem because by the time the callback is issued, a weapon will already
				// have been selected and the reticle will have been enabled.  The delayed removal of this weapon will
				// disable the reticle.  Thus, immediate removal means the reticle will be disabled immediately, and then
				// later the default weapon will be selected and re-enable the reticle.
				pWeap->RemoveFromWorld( TRUE );
				pWeap->SetOwner(this);

				if( i == 0 ) {
					// Make primary weapon pass along damage to upper-right arm...
					pWeap->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_SHOULDER] ) );
				}

				// give weapon a pointer back to its associated ItemInst object
				pWeap->SetItemInst( &m_pInventory->m_aoWeapons[i][uCurWpnIdx], TRUE );
			}

			m_WeaponInv[i].m_nWeaponInvCount = (u8)(m_pInventory->m_auNumWeapons[i]);
			m_WeaponInv[i].m_nWeaponSwitchToIndex = 255;
		}

		// NKM - Setup the battery counts
		FASSERT( m_pInventory->m_uNumBatteries > 0 );
		SetHealthContainerCount( (f32) m_pInventory->m_uNumBatteries );

		m_pRoboBuddy = fnew CRoboBuddy;
		if( m_pRoboBuddy == NULL ) {
			DEVPRINTF( "CBotGlitch::_InitInventory() : Out of memory.\n");
			return FALSE;
		}
		if( !m_pRoboBuddy->Create() ) {
			DEVPRINTF( "CBotGlitch::_InitInventory() : Error creating robo buddy.\n" );
			return FALSE;
		}
		m_pRoboBuddy->m_pOwnerBot = this;

		// clear pointers to currently selected weapons
		m_apWeapon[0] = m_apWeapon[1] = NULL;
		if( m_WeaponInv[ 0 ].m_nWeaponInvCount ) {
			if( m_pInventory ) {
				// change to current primary weapon as recorded in inventory
				_ChangeWeaponIndex( 0, m_pInventory->m_auCurWeapon[0] );
			} else {
				_ChangeWeaponIndex( 0, 1 );
			}
#if FANG_WINGC
			if( !m_apWeapon[0] ) {
				for( u32 uIndex=0; uIndex<m_WeaponInv[0].m_nWeaponInvCount; uIndex++ ) {
					if( m_WeaponInv[0].m_apWeapon[uIndex] ) {
						_ChangeWeaponIndex( 0, uIndex );
						break;
					}
				}
			}
			if( !m_apWeapon[0] ) {
				DEVPRINTF( "CBotGlitch::_InitInventory(): No supported primary weapon is available; continuing unarmed.\n" );
				m_WeaponInv[0].m_nWeaponInvCount = 0;
			}
			else
#endif
			m_apWeapon[0]->SetDesiredState( CWeapon::STATE_DEPLOYED );
		}

		if( m_WeaponInv[ 1 ].m_nWeaponInvCount ) {
			if( m_pInventory ) {
				// change to current secondary weapon as recorded in inventory
				_ChangeWeaponIndex( 1, m_pInventory->m_auCurWeapon[1] );
			} else {
				_ChangeWeaponIndex( 1, 0 );
			}
			if( m_apWeapon[1] ) {
				m_apWeapon[1]->SetDesiredState( CWeapon::STATE_DEPLOYED );
			}
#if FANG_WINGC
			else {
				DEVPRINTF( "CBotGlitch::_InitInventory(): No supported secondary weapon is available.\n" );
				m_WeaponInv[1].m_nWeaponInvCount = 0;
			}
#endif
		}
	}


	return TRUE;
_ExitWithError:
	return FALSE;
}


BOOL CBotGlitch::_InventoryCallback( CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 nHandIndex, u32 nWeaponIndex ) {
	CBotGlitch *pBotGlitch = (CBotGlitch *)pInventory->m_pUser;

	switch( eReason ) {
	case IREASON_WEAPONCHANGE:
		pBotGlitch->m_WeaponInv[nHandIndex].m_nWeaponSwitchToIndex = (u8)pInventory->m_auCurWeapon[nHandIndex];

		if( nHandIndex == 1 ) {
			pBotGlitch->AbortScopeMode( TRUE );
		}

		break;

	case IREASON_RELOAD:
		if( nHandIndex == 0 ) {
			pBotGlitch->m_abReloadRequest[0] = TRUE;
		} else if( nHandIndex == 1 ) {
			if( pBotGlitch->m_apWeapon[1] ) {
				pBotGlitch->m_apWeapon[1]->ClearAllWeaponTargets();
			}
		}

		break;

	case IREASON_WEAPONUPGRADED:
		pBotGlitch->m_WeaponInv[nHandIndex].m_apWeapon[nWeaponIndex]->UpgradeWeapon();
		pBotGlitch->AbortScopeMode();

		break;

	}

	return TRUE;
}


void CBotGlitch::_HandleIdleHeadMovement( void )
{
	if( m_nPossessionPlayerIndex < 0 )
	{
		return;
	}

	switch( m_nHeadIdleState )
	{
		case HEAD_IDLE_STATE_SET_IDLE:
			// init the head idle state
			HeadStopLook();
			m_fHeadIdleTime = ( 0.5f * HEAD_LOOK_IDLE_WAIT_TIME ) + ( fmath_RandomFloat() * 0.5f * HEAD_LOOK_IDLE_WAIT_TIME );
			m_nHeadIdleState = HEAD_IDLE_STATE_IDLE;
			break;

		case HEAD_IDLE_STATE_IDLE:
			if( m_fControls_Fire1 || m_fControls_Fire2 || m_bControls_Action || m_bControls_Jump || HeadShouldLook() || IsReverseFacingMode() )
			{
				// reset idle time count if a control button is pressed
				m_fHeadIdleTime = ( 0.5f * HEAD_LOOK_IDLE_WAIT_TIME ) + ( fmath_RandomFloat() * 0.5f * HEAD_LOOK_IDLE_WAIT_TIME );
			}
			else
			{
				// count down time to next head look
				m_fHeadIdleTime -= FLoop_fPreviousLoopSecs;
			}

			if( m_fHeadIdleTime <= 0.0f )
			{
				// idle time has passed, start a head look
				m_fHeadIdleLookChance = 1.0f;
				m_nHeadIdleState = HEAD_IDLE_STATE_SET_LOOK;
			}
			break;

		case HEAD_IDLE_STATE_SET_LOOK:
			if( fmath_RandomChance( m_fHeadIdleLookChance ) )
			{
				// initiate new head look if passed random chance
				f32 fYaw, fPitch;
				CFVec3A vLook;

				// pick a random direction within head look range
				fYaw = fmath_RandomBipolarUnitFloatWithGap( 0.5f ) * FMATH_DEG2RAD( 50.0f );
				fPitch = fmath_RandomBipolarUnitFloatWithGap( 0.5f ) * FMATH_DEG2RAD( 20.0f );

				// construct lookat vector
				vLook.Set( m_MountUnitFrontXZ_WS );
				vLook.RotateY( fYaw );
				vLook.RotateX( fPitch );

				// scale to 50 units length so lookat point is a bit far away
				vLook.Mul( 50.0f );

				// add in current bot position so lookat point will be in world space relative to bot
				vLook.Add( *m_pApproxEyePoint_WS );

				// start head look mode
				SetHeadLookLocation( vLook );
				HeadLook();

				// reduce chance of subsequent sequential looks
				m_fHeadIdleLookChance *= 0.45f;
				if( m_fHeadIdleLookChance < 0.10f )
				{
					m_fHeadIdleLookChance = 0.0f;
				}

				// set random time for head to look
				m_fHeadIdleTime = ( 0.5f * HEAD_LOOK_IDLE_LOOK_TIME ) + ( fmath_RandomFloat() * 0.5f * HEAD_LOOK_IDLE_LOOK_TIME );
				m_nHeadIdleState = HEAD_IDLE_STATE_LOOK;
			}
			else
			{
				// random chance failed, go back to idle state
				m_nHeadIdleState = HEAD_IDLE_STATE_SET_IDLE;
			}
			break;

		case HEAD_IDLE_STATE_LOOK:
			if( m_fControls_Fire1 || m_fControls_Fire2 || m_bControls_Action || m_bControls_Jump || IsReverseFacingMode() )
			{
				// exit head look immediately if a control button is pressed
				m_nHeadIdleState = HEAD_IDLE_STATE_SET_IDLE;
			}
			else
			{
//				aiutils_DebugTrackRay( *m_pApproxEyePoint_WS, m_vHeadLookPoint_WS, 2 );

				// count down time to end of head look
				m_fHeadIdleTime -= FLoop_fPreviousLoopSecs;

				if( m_fHeadIdleTime <= 0.0f )
				{
					// head look time has elapsed, check for another head look
					m_nHeadIdleState = HEAD_IDLE_STATE_SET_LOOK;
				}
			}
			break;

		default:
			break;
	}
}


void CBotGlitch::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies/* = TRUE*/ ) {
	// only players can be glitch
	FASSERT( m_nPossessionPlayerIndex >= 0 );

	if (m_nPossessionPlayerIndex <0)
	{
		BOOL bYouFoundAPoseesionBug = TRUE;
		return;
	}

	if( !m_bAllowPlayerDeath ) {
		// This makes it so that the player bot never dies!!!
		return;
	}

	if( IsDeadOrDying() ) {
		return;
	}

	// Let the player know we died
	Player_aPlayer[m_nPossessionPlayerIndex].CreditDeath();

	// If we are being attacked by the Probe's swing animation, don't play death animation
	if( m_bLeeched ) {
		m_uBotDeathFlags &= ~BOTDEATHFLAG_PLAYDEATHANIM;
	}

	// Get rid of robobuddy if he is out
	StowBuddy();

	m_fDeathTimer = _DEATH_TIME;
	m_fNextDeathEvent = _DEATH_TIME;
	m_uDeathExplosionCounter = _DEATH_NUM_EXPLOSIONS;

	CBot::Die( FALSE );

	CFVec3A vLookAt, vPos;
	vPos.Set( 0.0f, 2.0f, -10.0f );
	SetCameraOverride( TRUE, _DEATH_TIME, TRUE, CFVec3A::m_Null, vPos );
	ClearBotFlag_IgnoreControls();

	FASSERT( m_nPossessionPlayerIndex >= 0 );
	CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetDrawEnabled( FALSE );
	CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWSEnable( FALSE );

	// If this is multiplayer, drop our current weapon
	if (MultiplayerMgr.IsMultiplayer()) {
		s32 nWeaponType;	// really a CollectableType_e
		s32 nAmmo;

		_ChooseWeaponToDrop( &nWeaponType, &nAmmo );
		if ( nWeaponType != COLLECTABLE_UNKNOWN ) {
			CFMtx43A Mtx;
			Mtx.Identity();
			Mtx.m_vPos = m_MountPos_WS;
			Mtx.m_vPos.Add( CFVec3A::m_UnitAxisY );

			CFVec3A Vel(0.0f, 30.0f, 0.0f);

			CCollectable::PlaceIntoWorld( (CollectableType_e)nWeaponType, &Mtx, &Vel, 1.0f, nAmmo );
		}
	}
}

void CBotGlitch::CheckpointSaveSelect( s32 nCheckpoint ) {
	s32 nHand, nWeap;

	// save weapons first
	for( nHand = 0; nHand < 2; nHand++ )
	{
		for( nWeap = 0; nWeap < ItemInst_uMaxInventoryWeapons; nWeap++ )
		{
			if( m_WeaponInv[ nHand ].m_apWeapon[ nWeap ] )
			{
				m_WeaponInv[ nHand ].m_apWeapon[ nWeap ]->CheckpointSaveSelect( nCheckpoint );
			}
		}
	}

	// then save self
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


BOOL CBotGlitch::CheckpointSave( void )
{
	CArmorProfile *pCurProfile;
	u32 uCurWpn0;
	u32 uCurWpn1;

	// if gmm is in a mech, set his saved data to be the pre-mech data
	if( m_pCurMech && (m_pCurMech->TypeBits() & ENTITY_BIT_VEHICLE) ) {
		CVehicle *pVehicle = (CVehicle*)m_pCurMech;

		pCurProfile = GetArmorProfile();
		uCurWpn0 = m_WeaponInv[0].m_nWeaponInvIndex;
		uCurWpn1 = m_WeaponInv[1].m_nWeaponInvIndex;
	
		SetArmorProfile( pVehicle->GetPrevArmorProfile() );
		m_WeaponInv[0].m_nWeaponInvIndex = m_nStowedWeaponIndex[0];
		m_WeaponInv[1].m_nWeaponInvIndex = m_nStowedWeaponIndex[1];
	}

	// save parent class data
	CBot::CheckpointSave();

	if (GetPowerupFx())
	{
		GetPowerupFx()->CheckpointSave();
	}

	// load botGlitch class data.
	// order must match load order below.
	CFCheckPoint::SaveData( (void*) m_WeaponInv, 2 * sizeof( WeaponInv_t ) );

	// save off some glitchy specific damage stuff
	FASSERT( m_pPartMgr && m_pPartMgr->IsCreated() );
	for( u32 i=0; i<NUM_DAMAGEABLE_LIMBS; i++ ) {
		CFCheckPoint::SaveData( (BOOL)(m_pPartMgr->GetLimbState( m_aDamageLimbs[i] ) == CBotPartMgr::LIMB_STATE_DANGLING) );
	}

	// fixup our data...
	if( m_pCurMech && (m_pCurMech->TypeBits() & ENTITY_BIT_VEHICLE) ) {
		SetArmorProfile( pCurProfile );
		m_WeaponInv[0].m_nWeaponInvIndex = uCurWpn0;
		m_WeaponInv[1].m_nWeaponInvIndex = uCurWpn1;
	}

	//CFCheckPoint::SaveData( m_bInPieces );
	//CFCheckPoint::SaveData( m_fTimeInPieces );
	//CFCheckPoint::SaveData( m_bColWithPieces );

	// Save off the arm servo information...
	CFCheckPoint::SaveData( m_anServoLevel[SERVO_TYPE_ARMS] );

	return TRUE;
}

// Restore this bot and its children. Used for restoring a bot in multiplayer,
// not during the normal checkpoint restore process.
void CBotGlitch::CheckpointRestoreThisBot( void )
{
	// First restore all of our weapons.
	// IMPORTANT NOTE: This will only work if we can acquire weapons but not
	// lose them. If our startup weapons are not in our list, they will not
	// be restored properly!
	int i, j;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < m_WeaponInv[j].m_nWeaponInvCount; ++i ) {
			if (m_WeaponInv[j].m_apWeapon[i])
				m_WeaponInv[j].m_apWeapon[i]->CheckpointRestore();
		}
	}

	// Now restore this bot
	CheckpointRestore();
}

void CBotGlitch::CheckpointRestore( void )
{
	// load parent class data
	FASSERT( m_hSaveData[CFCheckPoint::GetCheckPoint()] != CFCheckPoint::NULL_HANDLE );
	CBot::CheckpointRestore();

	if (GetPowerupFx())
	{
		GetPowerupFx()->CheckpointRestore();
	}

	// load botGlitch class data.
	// order must match save order above.
	CFCheckPoint::LoadData( (void*) m_WeaponInv, 2 * sizeof( WeaponInv_t ) );
	CCollectable::FixupInventoryWeapons( m_pInventory );

	// load some glitch specific damage stuff
	FASSERT( m_pPartMgr && m_pPartMgr->IsCreated() );
	BOOL bLimbDangling;

	_ClearAllDamageEffects();
	for( u32 i=0; i<NUM_DAMAGEABLE_LIMBS; i++ ) {
		CFCheckPoint::LoadData( bLimbDangling );
		if( bLimbDangling ) {
			m_pPartMgr->SetState_Dangle( m_aDamageLimbs[i] );
		}
	}
	


	// Nate
	m_Anim.Reset();
	SetControlValue( ANIMCONTROL_STAND, 1.0f );

	EnableControlSmoothing( ANIMCONTROL_AIM_DUAL_WITH_TORSO_TWISTED );
	EnableControlSmoothing( ANIMCONTROL_AIM_SHOULDER_MOUNTED );
	EnableControlSmoothing( ANIMCONTROL_FIRE_2_UPPER );
	EnableControlSmoothing( ANIMCONTROL_FIRE_2_LOWER );
	EnableControlSmoothing( ANIMCONTROL_AIM_MORTAR );

	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );

	AttachAnim( ANIMTAP_RECOIL_SUMMER, &m_AnimManFrameRecoil );
	m_AnimManFrameRecoil.Reset();
	UpdateBoneMask( ANIMTAP_RECOIL_SUMMER, m_aBoneEnableIndices_RecoilSummer, TRUE );
	_ResetRecoilSummer();
	SetControlValue( ANIMCONTROL_RECOIL_SUMMER, 0.0f );
	// Nate end

	// tell hud that weapon changed
	if( m_pInventory )
	{
		// only players have inventory

		// change to restored weapon
		_ChangeWeaponIndex( 0, m_WeaponInv[0].m_nWeaponInvIndex );
		_ChangeWeaponIndex( 1, m_WeaponInv[1].m_nWeaponInvIndex );

		m_WeaponInv[0].m_nWeaponSwitchToIndex = 255;
		_WeaponOrUpgradeLevelMayHaveChanged();

		m_pInventory->SetCurWeapon( 0, m_WeaponInv[0].m_nWeaponInvIndex, FALSE, TRUE );
		m_pInventory->SetCurWeapon( 1, m_WeaponInv[1].m_nWeaponInvIndex, FALSE, TRUE );
	}
	else
	{
		_ChangeWeaponIndex( 0, m_WeaponInv[0].m_nWeaponInvIndex );

		m_WeaponInv[0].m_nWeaponSwitchToIndex = 255;
		_WeaponOrUpgradeLevelMayHaveChanged();
	}

	// set correct stance for restored primary weapon
	m_nWSState = WS_STATE_RELEASING;
	SetControlValue( ANIMCONTROL_WEAPON_SWITCH, 0.0f );
	switch( m_apWeapon[0]->m_pInfo->nStanceType )
	{
		case CWeapon::STANCE_TYPE_SHOULDER_MOUNT:
			m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED;
			break;

		case CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST:
			m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL;
			break;

		default:
			// Single-handed...
			m_nWSASI = ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE;
			break;
	}
	ZeroTime( m_nWSASI );
	SetControlValue( m_nWSASI, 1.0f );

	AbortScopeMode( TRUE );

	// attach weapon to arm
	m_apWeapon[0]->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_apszBoneNameTable[BONE_ATTACHPOINT_PRIMARY], &CFMtx43A::m_IdentityMtx, 1.0f, TRUE );
	m_apWeapon[0]->SetDesiredState( CWeapon::STATE_DEPLOYED );

	if( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_TETHER )
	{
		if( m_apWeapon[0]->IsClipEmptyAndCanItBeReloaded() )
		{
			m_apWeapon[0]->Trigger_AttachRoundToWeapon();
			m_apWeapon[0]->TransferFromReserveToClip();
		}
	}

	if( m_bLeeched ) {
		_DisableProbeSwing();
	}
	
	DisableGrabbedByHead();

	// only players can be glitch
	FASSERT( m_nPossessionPlayerIndex >= 0 );
	CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetDrawEnabled( TRUE );
	CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWSEnable( TRUE );

	//m_bFallDown = FALSE;
	//m_bFallAnimDone = TRUE;
	//m_bOnGround = FALSE;
	//CFCheckPoint::LoadData( m_bInPieces );
	//CFCheckPoint::LoadData( m_fTimeInPieces );
	//CFCheckPoint::LoadData( m_bColWithPieces );

	// Restore arm servo level...
	u32 nArmServoLevel;
	CFCheckPoint::LoadData( nArmServoLevel );
	SetServoLevel( SERVO_TYPE_ARMS, nArmServoLevel, TRUE );

	////if bot was in pieces at save time, set bot in pieces immediately upon restore.
	//if( m_bInPieces ) {
	//	f32 fTime;

	//	// BreakIntoPieces() uses current value of m_fTimeInPieces to decide 
	//	// if bot should be assembled or disassembled. We want the bot to be
	//	// disassembled, so we must make sure m_fTimeInPieces is not set to -1.0 
	//	// before calling BreakIntoPieces().
	//	fTime = m_fTimeInPieces;
	//	m_fTimeInPieces = 0.0f;
	//	BreakIntoPieces( fTime, TRUE /*bImmediately*/ );
	//}

	// cjm added !IsInPieces() below, to prevent respawn effect followed by breaking into pieces effect when
	// restoring to a state where glitch is possessing a bot or has used the wrench.
	if( IsPlayerBot() && m_bRespawnEffectInitialized && !IsInPieces() ) {
		RespawnEffect_Start();
		PlaySound( m_BotInfo_Glitch.pSoundGroupRespawn );
	}

	m_pWorldMesh->SetCollisionFlag( TRUE );
	SetTargetable( TRUE );
	FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_UNDER_CONSTRUCTION );

	ZeroControls();
}

#define _LEECH_LOOK_UP_BUMP			( 3.0f )
#define _ANIMATION_GROUND_HIT_TIME	( 0.56f )
#define _ANIMATION_GROUND_DEAD_TIME	( 0.78f )
#define _ANIMATION_FOV_BACK_TIME	( 0.40f )
#define _WASHER_LOSE_MIN			( 10 )
#define _WASHER_LOSE_MAX			( 20 )
#define _SHAKE_UNIT_INTENSITY		( 0.25f )
#define _SHAKE_DURATION				( 0.25f )

// This is here so that I can take care of things in the bot before his swing animation starts
// and before the bot is actually told to start his swing animation
void CBotGlitch::PreEnableProbeSwing( void ) {
	// No more movement from the bot
	ZeroVelocity();
		
	// No more zipline
	ReleaseCable();

	// Abort scope here so that we have a chance to get back into the bot camera.
	AbortScopeMode( TRUE );

	// We grabbed the bot in the air
	if( IsInAir() ) {
		SetGrabbedInAir( TRUE );
	}

	ShakeCamera( _SHAKE_UNIT_INTENSITY, _SHAKE_DURATION );

	// Weapon stuff
	if( m_apWeapon[0] ) {
		// Turn off the reticle
		ReticleEnable( FALSE );

		// Abort the mortar
		if( ( m_apWeapon[0]->TypeBits() & ENTITY_BIT_WEAPONMORTAR ) ) {
			_AbortMortarImmediately( TRUE );
		}

		// Kill the tether
		if( ( m_apWeapon[0]->TypeBits() & ENTITY_BIT_WEAPONTETHER ) ) {
			((CWeaponTether *)m_apWeapon[0])->DestroyTether();
		}
	}	

	if( m_apWeapon[1] ) {
		// If we have a cleaner in hand, get rid of all targets
		if( ( m_apWeapon[1]->TypeBits() & ENTITY_BIT_WEAPONCLEANER ) ) {
			((CWeaponCleaner * ) m_apWeapon[1])->ClearAllWeaponTargets();
		}
	}

	// If we are in a siteweapon, we need to get out of its view
	if( GetCurMech() ) {
		CBot *pVehicle = GetCurMech();

		if( ( pVehicle->TypeBits() & ENTITY_BIT_SITEWEAPON ) ) {
			CBotSiteWeapon *pWeap = (CBotSiteWeapon *) pVehicle;

			// Get out so the probe can abuse us
			pWeap->SetSiteWeaponDriver( NULL, FALSE );
		}
	}
}

void CBotGlitch::EnableProbeSwing( CBotProbe *pProbe ) {
	FASSERT( !m_bLeeched );
	FASSERT( pProbe );
	
	f32 fFOV;
	s32 sBone;

	ImmobilizeBot();

	sBone = m_pWorldMesh->FindBone( "Head" );
	if( sBone < 0 ) {
		DEVPRINTF( "CBotGlitch::EnableProbeSwing: Bone '%s' doesn't exist in object '%s'.\n", "Head", Name() );
		m_pLookTarget = &m_MtxToWorld.m_vPos;
	} else {
		m_pLookTarget = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;
	}

	sBone = m_pWorldMesh->FindBone( "MinerDummy" );
	if( sBone < 0 ) {
		DEVPRINTF( "CBotGlitch::EnableProbeSwing: Bone '%s' doesn't exist in object '%s'.\n", "MinerDummy", Name() );
		m_pMinerDummy = &m_MtxToWorld.m_vPos;
	} else {
		m_pMinerDummy = &m_pWorldMesh->GetBoneMtxPalette()[sBone]->m_vPos;
	}

	m_pProbe = pProbe;
	m_bLeeched = TRUE;
	m_fLookUnit = 0.0f;

	// Don't let the legs have any yaw once we grab the bot, this will break our animation
	m_fLegsYaw_MS = 0.0f;

	// Animation stuff
	m_fLeechUnitBlend = 1.0f;
	ZeroTime( ANIMTAP_LEECH );
	SetControlValue( ANIMCONTROL_LEECH, m_fLeechUnitBlend );

	// Only animate if we can get the camera now and we are a player controlled bot
	if( m_nPossessionPlayerIndex >= 0 && fcamera_GetCameraByIndex(m_nPossessionPlayerIndex) ) {
		m_bAnimateCamera = TRUE;

		// Get the actual camera
		CFCamera* pCam = fcamera_GetCameraByIndex(m_nPossessionPlayerIndex);

		// !! Make sure we don't have ANY shake in the camera
		m_LookStartQuat.BuildQuat( pCam->GetXfmWithoutShake()->m_MtxR/*m_MtxToWorld*/ );
		m_LookStartDirection = pCam->GetXfmWithoutShake()->m_MtxR.m_vFront;
		pCam->GetFOV( &fFOV );

		// Leave the camera where it started
		m_ProbeCamSimple.m_Pos_WS = pCam->GetXfmWithoutShake()->m_MtxR.m_vPos;
		m_ProbeCamSimple.m_Quat_WS.BuildQuat( pCam->GetXfmWithoutShake()->m_MtxR );

		gamecam_SwitchPlayerToSimpleCamera( PLAYER_CAM(m_nPossessionPlayerIndex), &m_ProbeCamSimple );
		pCam->SetFOV( fFOV );

		m_fProbeCamStartFOV = fFOV;
		m_fProbeCamEndFOV = fFOV + FMATH_DEG2RAD( 15.0f );
		m_fProbeCamFOVTime = 0.0f;
		m_fProbeCamFOVDir = 1.0f;
	}

	// So we don't pick up washers as we lose them
	FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_DONT_PICKUP_ITEMS );

	s32 uNumLose = fmath_RandomRange( _WASHER_LOSE_MIN, _WASHER_LOSE_MAX );
	
	// Sanity check
	FASSERT( uNumLose > 0 );

	// Drop the washers over our total animation time
	m_fWasherLoseTime = GetTotalTime( ANIMTAP_LEECH ) * fmath_Inv( (f32) uNumLose );
	m_fWasherLoseNextTime = m_fWasherLoseTime;
}

void CBotGlitch::_DisableProbeSwing( void ) { 
	m_bLeeched = FALSE;

	MobilizeBot();

	if( m_bAnimateCamera ) {
		// Put back the camera
		gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(m_nPossessionPlayerIndex), this );
	}

	if( m_apWeapon[0] ) {
		if( !IsReticleEnabled() ) {
			ReticleEnable( TRUE );
		}
	}

	m_bNoAdvanceAnim = FALSE;
	m_bAnimateCamera = FALSE;
	m_bGrabbedInAir = FALSE;

	// Dont swinging, we can pickup washers
	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_DONT_PICKUP_ITEMS );

	// Make sure this always gets cleared
	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_NO_GRAVITY );
}

void CBotGlitch::DisableGrabbedByHead( void )
{
	ClearGrabbedByHead();
	m_anAnimStackIndex[ASI_STAND] = ANIMTAP_STAND;
	m_fThrownYawVelocity = 0.0f;
	m_fGrabbedHeadYaw = 0.0f;
	m_fFallDownTime = 0.0f;
	m_pPartMgr->RepairLimb(LIMB_TYPE_LEFTARM);
	m_pPartMgr->RepairLimb(LIMB_TYPE_RIGHTARM);
	m_pPartMgr->RepairLimb(LIMB_TYPE_LEFTLEG);
	m_pPartMgr->RepairLimb(LIMB_TYPE_RIGHTLEG);

	if (! m_GrabCameraTrans.IsCameraInactive() )
	{
		m_GrabCameraTrans.SetCameraStateAbort();
		m_GrabCameraTrans.UpdateTransition();
	}
}

void CBotGlitch::_ProbeSwingWork( void ) {
	if( !m_bLeeched ) {
		return;
	}

	// So we don't lose Glitch as he flies up in the air
	CFSphere CollSphere;
	CollSphere.m_Pos.x = m_pMinerDummy->x;
	CollSphere.m_Pos.y = m_pMinerDummy->y;
	CollSphere.m_Pos.z = m_pMinerDummy->z;
	CollSphere.m_fRadius = 10.0f;

	m_pWorldMesh->MoveTracker( CollSphere );

    _AnimateProbeCamera();

	// Toss a washer?
	if( m_fWasherLoseNextTime == 0.0f ) {
		_TossWasher( m_pMinerDummy );
		m_fWasherLoseNextTime = m_fWasherLoseTime;
	}

	// We hit the ground at ~0.59f, do this only once
	if( GetUnitTime( ANIMTAP_LEECH ) >= _ANIMATION_GROUND_HIT_TIME && m_pProbe ) {
		if( !m_bGrabbedInAir ) {
			m_pProbe->GroundHitDamageBot( this );
			m_pProbe = NULL;
		} else {
			if( m_nBotFlags & BOTFLAG_NO_GRAVITY ) {		
				CFVec3A Vel;

				Vel.Zero();
				Vel.y = -32.0f;
				ApplyVelocityImpulse_WS( Vel );

				gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(m_nPossessionPlayerIndex), this );

				// Want gravity now
				FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_NO_GRAVITY );

				// Don't move our animation
				m_bNoAdvanceAnim = TRUE;
			}

			// We finally hit the ground
			if( !IsInAir() ) {
				m_bGrabbedInAir = FALSE;
				m_bNoAdvanceAnim = FALSE;
				ImmobilizeBot();
			}
		}
	}

	// Update time based stuff
	if( !m_bNoAdvanceAnim ) {
		DeltaTime( ANIMTAP_LEECH, FLoop_fPreviousLoopSecs, TRUE );
	}

	m_fLookUnit += FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMAX( m_fLookUnit, 1.0f );

	m_fProbeCamFOVTime += ( FLoop_fPreviousLoopSecs * m_fProbeCamFOVDir );
	FMATH_CLAMP( m_fProbeCamFOVTime, 0.0f, 1.0f );

	m_fWasherLoseNextTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fWasherLoseNextTime, 0.0f );

	// Animate our FOV
	if( m_bAnimateCamera && fcamera_GetCameraByIndex(m_nPossessionPlayerIndex) ) {
		fcamera_GetCameraByIndex(m_nPossessionPlayerIndex)->SetFOV( FMATH_FPOT( m_fProbeCamFOVTime, m_fProbeCamStartFOV, m_fProbeCamEndFOV ) );
	}
}

void CBotGlitch::_GrabbedByHeadWork( void ) {
	if( m_bGrabbedByHead ) 
	{
		if (m_pGrabberBot->IsDeadOrDying() || this->IsDeadOrDying())
		{
			DisableGrabbedByHead();
			return;
		}
		FWorld_nTrackerSkipListCount = 0;
		AppendTrackerSkipList();
		CFVec3A vToZombie,vTemp;
		BOOL bUpdateCamera = TRUE;
		for(CEntity* pChildEntity=GetFirstChild(); pChildEntity; pChildEntity=GetNextChild(pChildEntity) ) 
		{
			// I'm only computing camera bUpdate this way because a specific fn call
			// would take too long to compile...
			if ( pChildEntity->TypeBits() & ENTITY_BIT_PARTICLE )
			{
				bUpdateCamera = FALSE;
			}
		}
		CFMtx43A mCamera;
		if (bUpdateCamera)
		{
			vToZombie.Sub(*m_pGrabberBot->GetTagPoint(2), m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexHead]->m_vPos);
			vTemp.Mul(m_pGrabberBot->MtxToWorld()->m_vRight, 2.5f);
			vToZombie.Sub(vTemp);

			mCamera.UnitMtxFromNonUnitVec(&vToZombie);
			mCamera.m_vPos = *m_pGrabberBot->GetTagPoint(2);
			m_GrabCamera.Update( &mCamera, CFVec3A::m_Null, m_pWorldMesh, CFVec3A::m_UnitAxisY);
		}
	}
	else if (m_nJumpState == BOTJUMPSTATE_AIR2) // aka thrown state
	{
		if (IsDeadOrDying() == FALSE)
		{
			FWorld_nTrackerSkipListCount = 0;
			AppendTrackerSkipList();

			CFMtx43A mCamera;
			mCamera.UnitMtxFromNonUnitVec(&m_VelocityXZ_WS); // This function handles null vec fine.

			mCamera.m_vPos = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexHead]->m_vPos;
			m_GrabCamera.Update( &mCamera, CFVec3A::m_Null, m_pWorldMesh, CFVec3A::m_UnitAxisY);
		}
		else
		{
			m_GrabCameraTrans.SetCameraStateAbort();
			m_nJumpState = BOTJUMPSTATE_NONE;
		}
	}
	
	if (m_GrabCameraTrans.IsCameraInactive() == FALSE)
	{
		m_GrabCameraTrans.UpdateTransition();
	}
}

void CBotGlitch::_HandleGrabbedPitchYaw( void )
{
	/*
	f32 fMaxRevsPerSec;
	fMaxRevsPerSec = m_pBotInfo_MountAim->fMountPitchUpDownRevsPerSec;
	m_fMountPitchClampedNormDelta = fmath_Abs( m_fControls_AimDown );
	// Update look pitch...
	if( m_fControls_AimDown != 0.0f) {
		f32 fAdjustment = 0;
		if ( m_fControls_AimDown ) {
			fAdjustment = fMaxRevsPerSec * FMATH_2PI * m_fControls_AimDown * fmath_Abs(m_fControls_AimDown) * FLoop_fPreviousLoopSecs;
		}

		m_fMountPitch_WS += fAdjustment;

		if( fAdjustment > 0.0f ) {
			if( m_fMountPitch_WS > m_pBotInfo_MountAim->fMountPitchDownLimit ) {
				m_fMountPitch_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
				m_fMountPitchClampedNormDelta = 0.0f;
			}
		} else {
			if( m_fMountPitch_WS < m_pBotInfo_MountAim->fMountPitchUpLimit ) {
				m_fMountPitch_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;
				m_fMountPitchClampedNormDelta = 0.0f;
			}
		}
	}
	*/
	
	f32 fAbsControlsRotateCW = FMATH_FABS( m_fControls_RotateCW );
	m_fMountYawClampedNormDelta = 0.0f;

	if( fAbsControlsRotateCW > 0.0f) {
		f32 fDelta = 0.f;
		f32 fBaseRevsPerSec = fmath_Div( fAbsControlsRotateCW * fAbsControlsRotateCW * fAbsControlsRotateCW, m_pBotInfo_MountAim->fMountYawUnitIntoControlOverdriveThresh ) * m_pBotInfo_MountAim->fMountYawBaseRevsPerSec;
		f32 fYawRevsPerSec_WS = fBaseRevsPerSec; // fBaseRevsPerSec + m_fYawUnitOverdrive*m_pBotInfo_MountAim->fMountYawOverdriveRevsPerSec;
		// Restore sign to rotation rate
		if( m_fControls_RotateCW < 0.0f ) {
			fYawRevsPerSec_WS = -fYawRevsPerSec_WS;
		}
		// Compute rotation delta for this frame and convert to radians
		fDelta = 1.75f * fYawRevsPerSec_WS * FLoop_fPreviousLoopSecs * FMATH_2PI;
		m_fGrabbedHeadYaw += fDelta;
	}
	
/*
	CFVec3A targetPtWS;
	targetPtWS.ReceiveRotationY(CFVec3A::m_UnitAxisZ,m_fMountYaw_WS + m_fLegsYaw_MS + m_fGrabbedHeadYaw );
	targetPtWS.Mul(3.0f);
	targetPtWS.Add(m_MtxToWorld.m_vPos);
	targetPtWS.y += 5.0f;

	m_ControlBot_TargetPoint_WS = targetPtWS;
*/	
	

/*
	CDebugDraw::Sphere(targetPtWS,1,&FColor_MotifWhite);

	CFVec3A Vx,Vy,Vz;
	Vx.Mul(CFMtx43A::m_XlatRotY.m_vRight, 100.f).Add(m_MtxToWorld.m_vPos);
	Vy.Mul(CFMtx43A::m_XlatRotY.m_vUp, 100.f).Add(m_MtxToWorld.m_vPos);
	Vz.Mul(CFMtx43A::m_XlatRotY.m_vFront, 100.f).Add(m_MtxToWorld.m_vPos);
	CDebugDraw::Line(m_MtxToWorld.m_vPos, Vx, &FColor_MotifRed, &FColor_MotifRed);
	CDebugDraw::Line(m_MtxToWorld.m_vPos, Vy, &FColor_MotifGreen, &FColor_MotifGreen);
	CDebugDraw::Line(m_MtxToWorld.m_vPos, Vz, &FColor_MotifBlue, &FColor_MotifBlue);
*/
}
	

void CBotGlitch::_AnimateProbeCamera( void ) {
	CFVec3A CurrentLook;
	CFMtx43A Mtx;
	CFQuatA LookEndQuat, Slerp, RotQuat;
	CFXfm Xfm;
	f32 fUnitTime, fTimeLeft;

	// If we are dead or well on our way, give up
	if( IsDeadOrDying() ) {
		// We hit the gound and just died, stay there
		if( GetUnitTime( ANIMTAP_LEECH ) >= _ANIMATION_GROUND_DEAD_TIME && m_pLookTarget ) {
			if( m_bAnimateCamera ) {
				// Bump up the look target so that we see more of the Probe
				m_LookEndAnimDirection.Set( CFVec3A::m_UnitAxisY ).Mul( _LEECH_LOOK_UP_BUMP );
				m_LookEndAnimDirection.Add( *m_pLookTarget );
				m_LookEndAnimDirection.Sub( m_ProbeCamSimple.m_Pos_WS ).Unitize();
			}
			
			m_pLookTarget = NULL;
			m_fLookUnit = 0.0f;
			m_bNoAdvanceAnim = TRUE;
		}

		if( m_bNoAdvanceAnim && m_fLookUnit >= 1.0f ) {
			_DisableProbeSwing();
		}
	}
	
	fUnitTime = GetUnitTime( ANIMTAP_LEECH );
	fTimeLeft = ( GetTotalTime( ANIMTAP_LEECH ) - GetTime( ANIMTAP_LEECH ) );

	if( fTimeLeft <= 1.0f ) {
		if( m_pLookTarget ) {
			if( m_bAnimateCamera ) {
				// Bump up the look target so that we see more of the Probe
				m_LookEndAnimDirection.Set( CFVec3A::m_UnitAxisY ).Mul( _LEECH_LOOK_UP_BUMP );
				m_LookEndAnimDirection.Add( *m_pLookTarget );
				m_LookEndAnimDirection.Sub( m_ProbeCamSimple.m_Pos_WS ).Unitize();
			}

			m_pLookTarget = NULL;
			m_fLookUnit = 1.0f - fTimeLeft;
		}


		m_fLeechUnitBlend -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fLeechUnitBlend, 0.0f );

		SetControlValue( ANIMCONTROL_LEECH, m_fLeechUnitBlend );
	}

	// Time to animate the camera FOV down
	//if( fUnitTime >= _ANIMATION_FOV_BACK_TIME && m_fProbeCamFOVDir == 1.0f ) {
	if( fUnitTime >= _ANIMATION_FOV_BACK_TIME ) { 
		if ( m_fProbeCamFOVDir == 1.0f ) {
			m_fProbeCamFOVDir = -1.0f;
		}

		if( m_pLookTarget && m_bGrabbedInAir && fTimeLeft < 1.0f ) {
			// !!Nate - put this into a func along with the one above
			m_LookEndAnimDirection.Set( CFVec3A::m_UnitAxisY ).Mul( _LEECH_LOOK_UP_BUMP );
			m_LookEndAnimDirection.Add( *m_pLookTarget );
			m_LookEndAnimDirection.Sub( m_ProbeCamSimple.m_Pos_WS ).Unitize();

			m_pLookTarget = NULL;
			m_fLookUnit = 0.0f;
		}
	} 

	// We are done
	if( fUnitTime >= 1.0f && m_fLookUnit >= 1.0f ) {
        _DisableProbeSwing();

		ZeroTime( ANIMTAP_LEECH );
		SetControlValue( ANIMCONTROL_LEECH, 0.0f );

		return;
	}

	// The rest of this animates, so if we don't need to animate, leave
	if( !m_bAnimateCamera ) {
		return;
	}

	// Update the camera to look at the attacker
	if( m_pLookTarget ) {
		// Bump up the look target so that we see more of the Probe
		Mtx.m_vFront.Set( CFVec3A::m_UnitAxisY ).Mul( _LEECH_LOOK_UP_BUMP );
		Mtx.m_vFront.Add( *m_pLookTarget );
        Mtx.m_vFront.Sub( m_ProbeCamSimple.m_Pos_WS ).Unitize();
	} else {
		Mtx.m_vFront = m_LookEndAnimDirection;
	}

	Mtx.m_vRight.CrossYWithVec( Mtx.m_vFront );

	if( Mtx.m_vRight.MagSq() < 0.001f ) {
		Mtx.m_vRight.UnitCross( CFVec3A::m_UnitAxisX, Mtx.m_vFront );
	} else {
		Mtx.m_vRight.Unitize();
	}

	Mtx.m_vUp.UnitCross( Mtx.m_vFront, Mtx.m_vRight );

	LookEndQuat.BuildQuat( Mtx );

	if( m_pLookTarget ) {
		Slerp.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fLookUnit ), m_LookStartQuat, LookEndQuat );
	} else {
		Slerp.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fLookUnit ), LookEndQuat, m_LookStartQuat );
	}

	Slerp.MulPoint( CurrentLook, CFVec3A::m_UnitAxisZ );
	Xfm.BuildLookatFromDirVec( m_ProbeCamSimple.m_Pos_WS.v3, CurrentLook.v3, CFVec3A::m_UnitAxisY.v3 );
	m_ProbeCamSimple.m_Quat_WS.BuildQuat( Xfm.m_MtxR );
}

// Tosses a washer from the passed position and takes one away from Glitch
void CBotGlitch::_TossWasher( const CFVec3A *pPos ) { 
	// Only toss them if we have them
	if( m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo > 0 ) {
		CFMtx43A Mtx;
		CFVec3A Vel;

		Mtx.Identity();
		Mtx.m_vPos = *pPos;

		Vel.x = fmath_RandomFloatRange( 0.25f, 1.0f ) * 10.0f; 
		Vel.y = fmath_RandomFloatRange( 0.25f, 1.0f ) * 10.0f; 
		Vel.z = fmath_RandomFloatRange( 0.25f, 1.0f ) * 10.0f; 

		if( fmath_RandomChance( 0.5f ) ) {
			Vel.x = -Vel.x;
		}

		if( fmath_RandomChance( 0.5f ) ) {
			Vel.y = -Vel.y;
		}

		if( fmath_RandomChance( 0.5f ) ) {
			Vel.z = -Vel.z;
		}

		CCollectable::PlaceIntoWorld( "washer", &Mtx, &Vel, 1.0f );

		// One less washer
		--m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;

		if( m_nPossessionPlayerIndex >= 0 ) {
			CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWasherTimed( 0.1f );
		}
	}
}

void CBotGlitch::SetGrabbedByHead(CBot* pWhosGotMe, cchar* szByWhatBone)
{
	m_bGrabbedByHead = TRUE;
	m_pGrabberBot = pWhosGotMe;
	m_nGrabberBoneIndex = m_pGrabberBot->m_pWorldMesh->FindBone(szByWhatBone);
	if ( (m_nGrabberBoneIndex < 0) || (m_nBoneIndexHead < 0) )
	{
		DEVPRINTF("BONE FAILURE\n");
		FASSERT(0);
	}
	Attach_ToParent_WithGlue_WS(pWhosGotMe,szByWhatBone);
	m_GrabCamera.Init(&m_GrabCameraInfo,&m_BotInfo_GrabCamera);	// vehicle camera object for glitch
	m_GrabCameraTrans.Init(&m_GrabCameraInfo,0.5f);				// camera transition object to/from grab
    m_GrabCameraTrans.InitTransitionToVehicleCamera( m_nPossessionPlayerIndex );
	m_GrabCamera.Snap();

	m_anAnimStackIndex[ASI_STAND] = -1;

	ReticleEnable(FALSE);

	FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_NO_FEET_ANIM );	
	
	m_pPartMgr->SetState_Dangle( LIMB_TYPE_LEFTARM,FALSE, -1000.f );
	m_pPartMgr->SetState_Dangle( LIMB_TYPE_RIGHTARM,FALSE, -1000.f );

	m_pPartMgr->SetState_Dangle( LIMB_TYPE_LEFTLEG,FALSE, -1000.f );
	m_pPartMgr->SetState_Dangle( LIMB_TYPE_RIGHTLEG,FALSE, -1000.f );
}

void CBotGlitch::ClearGrabbedByHead(void)
{
	m_bGrabbedByHead = FALSE;
	m_fGrabbedHeadYaw = 0.0f;
	if (m_pGrabberBot)
	{
		if (GetParent() == m_pGrabberBot)
			DetachFromParent();

		m_pGrabberBot = NULL;
	}
	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_NO_FEET_ANIM );	

//	m_GrabCameraTrans.InitTransitionToVehicleCamera( m_nPossessionPlayerIndex );
}

void CBotGlitch::ThrownAway(CFVec3A& rTossUnitDirWS, f32 fTossVelocity)
{
	if (!IsDeadOrDying())
	{
		CFVec3A vImpulseWS = rTossUnitDirWS;
		vImpulseWS.Mul(fTossVelocity);
		ApplyVelocityImpulse_WS( vImpulseWS );
		m_fThrownYawVelocity = (f32) -1 + 2 * fmath_RandomChoice(2);
		m_fThrownYawVelocity *= 20.0f;
		m_nJumpState = BOTJUMPSTATE_AIR2; // rename next time you wanna big recompile
		m_GrabCameraTrans.InitTransitionToCamera(&m_ThrowCameraInfo, .25f);
		m_GrabCamera.Init(&m_ThrowCameraInfo,&m_BotInfo_ThrowCamera);	// vehicle camera object for glitch
	}
}

#define _AIM_BLENDOUT_RATE ( 4.0f )

void CBotGlitch::_AimLowerArm( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	f32 fSlerp;
	
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( m_fUnitAim == 0.0f ) {
		return;
	}

	// aiming disabled, blend out...
	if( (m_nBotFlags2 & BOTFLAG2_DISABLE_AIMING) || (m_fUnitFireCtlUpper < _FIRE_READY_BLEND) ) {
		m_fUnitAim -= FLoop_fPreviousLoopSecs * _AIM_BLENDOUT_RATE;
		FMATH_CLAMP_MIN0( m_fUnitAim );
		CFQuatA qAim;
		qAim.BuildQuat( rNewMtx );
		m_qRightLowerArm.ReceiveSlerpOf( fmath_UnitLinearToSCurve( 1.0f - m_fUnitAim ), m_qRightLowerArm, qAim );
		m_qRightLowerArm.BuildMtx33( rNewMtx );
		return;
	}

    // calculate aim mtx
	CFVec3A vAim;
	fSlerp = _ARM_SLERP_RATE * FLoop_fPreviousLoopSecs;

	vAim.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
	if( vAim.MagSq() > 0.00001f ) {
		rNewMtx.m_vFront = vAim.PlanarProjection( rNewMtx.m_vRight );

		f32 fMag2 = rNewMtx.m_vFront.MagSq();
		if( fMag2 > 0.00001f ) {
			rNewMtx.m_vFront.Mul( fmath_InvSqrt( fMag2 ) );
			rNewMtx.m_vUp.Cross( rNewMtx.m_vFront, rNewMtx.m_vRight );
		} else {
			// Restore original front vector...
			rNewMtx.m_vFront.Cross( rNewMtx.m_vRight, rNewMtx.m_vUp );
		}
	}

	// if fired very recently, go directly there, otherwise move toward correct position
	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		m_qRightLowerArm.BuildQuat( rNewMtx );
	} else {
		CFQuatA qTo;
		FMATH_CLAMP_UNIT_FLOAT( fSlerp );

		qTo.BuildQuat( rNewMtx );
		m_qRightLowerArm.ReceiveSlerpOf( fSlerp, m_qRightLowerArm, qTo );
		m_qRightLowerArm.BuildMtx33( rNewMtx );
	}





	//f32 fSlerp;
	//f32 fCtl;

	//rNewMtx.Mul( rParentMtx, rBoneMtx );
	//
	//fCtl = GetControlValue( ANIMCONTROL_FIRE_1_UPPER );
	//FMATH_CLAMPMAX( fCtl, (1.0f - GetControlValue( ANIMCONTROL_GSLAP_UPPER )) * (1.0f - GetControlValue( ANIMCONTROL_GSLAP_UPPER )) );

	//if( (fCtl == 0.0f) || (m_apWeapon[0] == NULL) || (m_apWeapon[0]->m_pInfo->nStanceType != CWeapon::STANCE_TYPE_STANDARD) ) {
	//	return;
	//}

	//if( m_fUnitFireCtlUpper >= _FIRE_READY_BLEND ) {
	//	CFVec3A vAim;
	//	fSlerp = _ARM_SLERP_RATE * FLoop_fPreviousLoopSecs;

	//	vAim.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
	//	if( vAim.MagSq() > 0.00001f ) {
	//		rNewMtx.m_vFront = vAim.PlanarProjection( rNewMtx.m_vRight );

	//		f32 fMag2 = rNewMtx.m_vFront.MagSq();
	//		if( fMag2 > 0.00001f ) {
	//			rNewMtx.m_vFront.Mul( fmath_InvSqrt( fMag2 ) );
	//			rNewMtx.m_vUp.Cross( rNewMtx.m_vFront, rNewMtx.m_vRight );
	//		} else {
	//			// Restore original front vector...
	//			rNewMtx.m_vFront.Cross( rNewMtx.m_vRight, rNewMtx.m_vUp );
	//		}
	//	}
	//} else {
	//	fSlerp = 1.0f - fmath_Div( fCtl, _FIRE_READY_BLEND );
	//}

	//// if fired very recently, go directly there, otherwise move toward correct position
	//if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
	//	m_qRightLowerArm.BuildQuat( rNewMtx );
	//} else {
	//	CFQuatA qTo;
	//	FMATH_CLAMP_UNIT_FLOAT( fSlerp );

	//	qTo.BuildQuat( rNewMtx );
	//	m_qRightLowerArm.ReceiveSlerpOf( fSlerp, m_qRightLowerArm, qTo );
	//	m_qRightLowerArm.BuildMtx33( rNewMtx );
	//}
}


void CBotGlitch::_AimElbow( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	f32 fSlerp;
	
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( m_fUnitAim == 0.0f ) {
		return;
	}

	// aiming disabled, blend out...
	if( (m_nBotFlags2 & BOTFLAG2_DISABLE_AIMING) || (m_fUnitFireCtlUpper < _FIRE_READY_BLEND) ) {
		m_fUnitAim -= FLoop_fPreviousLoopSecs * _AIM_BLENDOUT_RATE;
		FMATH_CLAMP_MIN0( m_fUnitAim );
		CFQuatA qAim;
		qAim.BuildQuat( rNewMtx );
		m_qRightElbow.ReceiveSlerpOf( fmath_UnitLinearToSCurve( 1.0f - m_fUnitAim ), m_qRightElbow, qAim );
		m_qRightElbow.BuildMtx33( rNewMtx );
		return;
	}

    // calculate aim mtx
	CFVec3A vAim;
	fSlerp = _ARM_SLERP_RATE * FLoop_fPreviousLoopSecs;

	vAim.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
	if( vAim.MagSq() > 0.00001f ) {
		rNewMtx.m_vUp = vAim.PlanarProjection( rNewMtx.m_vFront );

		f32 fMag2 = rNewMtx.m_vUp.MagSq();
		if( fMag2 > 0.00001f ) {
			rNewMtx.m_vUp.Mul( -fmath_InvSqrt( fMag2 ) );
			rNewMtx.m_vRight.Cross( rNewMtx.m_vUp, rNewMtx.m_vFront );
		} else {
			// Restore original up vector...
			rNewMtx.m_vUp.Cross( rNewMtx.m_vFront, rNewMtx.m_vRight );
		}
	}

	// if fired very recently, go directly there, otherwise move toward correct position
	if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
		m_qRightElbow.BuildQuat( rNewMtx );
	} else {
		CFQuatA qTo;
		FMATH_CLAMP_UNIT_FLOAT( fSlerp );

		qTo.BuildQuat( rNewMtx );
		m_qRightElbow.ReceiveSlerpOf( fSlerp, m_qRightElbow, qTo );
		m_qRightElbow.BuildMtx33( rNewMtx );
	}

	//f32 fSlerp;
	//f32 fCtl;

	//rNewMtx.Mul( rParentMtx, rBoneMtx );
	//return;
	//
	//fCtl = GetControlValue( ANIMCONTROL_FIRE_1_UPPER );
	//FMATH_CLAMPMAX( fCtl, (1.0f - GetControlValue( ANIMCONTROL_GSLAP_UPPER )) * (1.0f - GetControlValue( ANIMCONTROL_GSLAP_UPPER ))  );


	//if( (fCtl == 0.0f) || (m_apWeapon[0] == NULL) || (m_apWeapon[0]->m_pInfo->nStanceType != CWeapon::STANCE_TYPE_STANDARD) ) {
	//	return;
	//}

	//if( m_fUnitFireCtlUpper >= _FIRE_READY_BLEND ) {
	//	CFVec3A vAim;
	//	fSlerp = _ARM_SLERP_RATE * FLoop_fPreviousLoopSecs;

	//	vAim.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
	//	if( vAim.MagSq() > 0.00001f ) {
	//		rNewMtx.m_vUp = vAim.PlanarProjection( rNewMtx.m_vFront );

	//		f32 fMag2 = rNewMtx.m_vUp.MagSq();
	//		if( fMag2 > 0.00001f ) {
	//			rNewMtx.m_vUp.Mul( -fmath_InvSqrt( fMag2 ) );
	//			rNewMtx.m_vRight.Cross( rNewMtx.m_vUp, rNewMtx.m_vFront );
	//		} else {
	//			// Restore original up vector...
	//			rNewMtx.m_vUp.Cross( rNewMtx.m_vFront, rNewMtx.m_vRight );
	//		}
	//	}
	//} else {
	//	fSlerp = 1.0f - fmath_Div( fCtl, _FIRE_READY_BLEND );
	//}

	//if( m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM ) {
	//	m_qRightElbow.BuildQuat( rNewMtx );
	//} else {
	//	CFQuatA qTo;
	//	FMATH_CLAMP_UNIT_FLOAT( fSlerp );

	//	qTo.BuildQuat( rNewMtx );
	//	m_qRightElbow.ReceiveSlerpOf( fSlerp, m_qRightElbow, qTo );
	//	m_qRightElbow.BuildMtx33( rNewMtx );
	//}
}


void CBotGlitch::_ResetRecoilSummer( void ) {
	CFAnimFrame qFrame;

	m_AnimManFrameRecoil.Reset();

	if( m_apWeapon[0] && m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_STANDARD ) {
		qFrame.BuildQuat( CFVec3A::m_UnitAxisY, _RECOIL_ROT_RIGHT_SHOULDER, CFVec3A::m_Null );
		m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_RIGHT_SHOULDER, qFrame );

		CFVec3A vTorsoAxis;
		vTorsoAxis.Set( -1.0f, 2.0f, 0.0f );
		vTorsoAxis.Unitize();

		qFrame.BuildQuat( vTorsoAxis, _RECOIL_ROT_TORSO, CFVec3A::m_Null );
		m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_TORSO, qFrame );

		qFrame.BuildQuat( CFVec3A::m_UnitAxisX, _RECOIL_ROT_LEFT_SHOULDER, CFVec3A::m_Null );
		m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_LEFT_SHOULDER, qFrame );

		qFrame.BuildQuat( CFVec3A::m_UnitAxisX, _RECOIL_ROT_LEFT_ELBOW, CFVec3A::m_Null );
		m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_LEFT_LOWER_ARM, qFrame );
	}
}


void CBotGlitch::_Recoil( f32 fRecoilAmount ) {
	FASSERT_UNIT_FLOAT( fRecoilAmount );

	if( m_fUnitRecoil + fRecoilAmount > m_fUnitRecoilTarget ) {
		m_fUnitRecoilTarget = m_fUnitRecoil + fRecoilAmount;
		FMATH_CLAMPMAX( m_fUnitRecoilTarget, FMATH_MIN( 1.0f, 1.5f * fRecoilAmount ) );
	}
}


void CBotGlitch::_HandleRecoil( void ) {
	// calculate our dummy rotation
	f32 fSin, fCos;
	CFVec3A vAxis;
	CFAnimFrame qFrame;

	fmath_SinCos( m_fAimDeltaYaw_MS - m_fLegsYaw_MS, &fSin, &fCos );
	vAxis.Set( fCos, 0.0f, -fSin );

	qFrame.BuildQuat( vAxis, _RECOIL_ROT_DUMMY, CFVec3A::m_Null );
	m_AnimManFrameRecoil.UpdateFrame( ANIMBONE_RECOIL_DUMMY, qFrame );

	// don't recoil unless fire animation is at 100%
	if( m_fUnitFireCtlUpper != 1.0f ) {
		m_fUnitRecoilTarget = 0.0f;
	}

	if( m_fUnitRecoil <= m_fUnitRecoilTarget ) {

        // we're recoiling
		m_fUnitRecoil += FLoop_fPreviousLoopSecs * _RECOIL_ON_SPEED;
		
		if( m_fUnitRecoil >= m_fUnitRecoilTarget ) {		// got there, now let's head back
			m_fUnitRecoil = m_fUnitRecoilTarget;
			m_fUnitRecoilTarget = 0.0f;
		}

		SetControlValue( ANIMCONTROL_RECOIL_SUMMER, m_fUnitRecoil );

	} else if( m_fUnitRecoil > m_fUnitRecoilTarget ) {
		// recovering
		m_fUnitRecoil -= FLoop_fPreviousLoopSecs * _RECOIL_OFF_SPEED;
		FMATH_CLAMP_MIN0( m_fUnitRecoil );
		SetControlValue( ANIMCONTROL_RECOIL_SUMMER, m_fUnitRecoil );
	}
}


void CBotGlitch::InflictDamageResult( const CDamageResult *pDamageResult ) {
	if( FLoop_nTotalLoopTicks - m_uLastTimeDamaged > (u64)(FLoop_nTicksPerSec * _DAMAGE_ACCUM_INTERVAL) ) {
		m_fDamageAccumulator = 0.0f;
	}
    f32 fHealth = NormHealth();

	CBot::InflictDamageResult( pDamageResult );

	m_fDamageAccumulator += fHealth - NormHealth();
	m_uLastTimeDamaged = FLoop_nTotalLoopTicks;

	FMATH_CLAMPMIN( fHealth, 0.0001f );

	f32 fPerc = fmath_Div( m_fDamageAccumulator, fHealth );
	if( fPerc > _DAMAGE_EFFECT_THRESHOLD ) {
		if( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_IMPACT ) {
			_StartDamageEffect( pDamageResult->m_pDamageData->m_pTriData->nDamageeBoneIndex );
		} else {
			_StartDamageEffect( -1 );
		}
		m_fDamageAccumulator = 0.0f;
	}

	GetPowerupFx()->SetDamagePoint( pDamageResult->m_pDamageData->m_ImpactPoint_WS );
}

#define _DEATH_IMPULSE_X 			( 2.0f )
#define _DEATH_IMPULSE_Y 			( 10.0f )
#define _DEATH_IMPULSE_Z 			( 2.0f )
#define _DEATH_CAMSHAKE_INTENSITY	( 0.25f )
#define _DEATH_CAMSHAKE_DURATION	( 0.25f )
#define _DAMAGE_CAMSHAKE_INTENSITY	( 0.125f )
#define _DAMAGE_CAMSHAKE_DURATION	( 0.25f )

void CBotGlitch::DeathWork( void ) {
	if( m_uLifeCycleState == BOTLIFECYCLE_BURIED ) {
		CBot::DeathWork();
		return;
	}

	f32 fLastTimer = m_fDeathTimer;
	m_fDeathTimer -= FLoop_fPreviousLoopSecs;

	// Handle transition from dead to buried separately.
	// NOTE: Don't call CBot::DeathWork until after we transition to buried!!
	if ( m_uLifeCycleState == BOTLIFECYCLE_DEAD ) {
		if ( m_fDeathTimer < 0.0f ) {
			m_uLifeCycleState = BOTLIFECYCLE_BURIED;
		}

		// Must return!
		return;
	}

	if( (fLastTimer >= m_fNextDeathEvent) && (m_fDeathTimer < m_fNextDeathEvent) ) {
		m_pPartMgr->SetRandomLimb_Dangle();

		// give glitchy a little push...
		CFVec3A vImpulse;
		vImpulse.Set( fmath_RandomBipolarUnitFloat() * _DEATH_IMPULSE_X,
					  fmath_RandomFloatRange( 0.0f/*0.5f * _DEATH_IMPULSE_Y*/, _DEATH_IMPULSE_Y ),
					  fmath_RandomBipolarUnitFloat() * _DEATH_IMPULSE_Z );
		ApplyVelocityImpulse_MS( vImpulse );
		
		ShakeCamera( _DAMAGE_CAMSHAKE_INTENSITY, _DAMAGE_CAMSHAKE_DURATION );

		// spawn an explosion...
		if( (m_hDeathExplosion != FEXPLOSION_INVALID_HANDLE) && (m_uDeathExplosionCounter > 0) && fmath_RandomChance( 0.5 ) ) {

			FExplosionSpawnParams_t SpawnParams;
			FExplosion_SpawnerHandle_t hSpawner;
	
			SpawnParams.InitToDefaults();
			SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
			SpawnParams.Pos_WS = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexTorso]->m_vPos;
			SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
			SpawnParams.uSurfaceType = 0;
			SpawnParams.pDamageProfile = NULL;
			SpawnParams.pDamager = NULL;

			hSpawner = CExplosion2::GetExplosionSpawner();
			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				CExplosion2::SpawnExplosion( hSpawner, m_hDeathExplosion, &SpawnParams );
				m_uDeathExplosionCounter--;
			}
		}


		f32 fVal = 0.0f;
		for( u32 i=0; i<LIMB_TYPE_COUNT; i++ ) {
			if( m_pPartMgr->GetLimbState( i ) == CBotPartMgr::LIMB_STATE_INTACT ) {
				fVal += 1.0f;
			}
		}

		if( fVal > 0.0f ) {
			fVal = fmath_Inv( fVal ) * m_fDeathTimer;
            m_fNextDeathEvent = m_fDeathTimer - fmath_RandomFloatRange( fVal * 0.5f, fVal );
		}
	}

	if( m_fDeathTimer < 0.0f ) {
		ShakeCamera( _DEATH_CAMSHAKE_INTENSITY, _DEATH_CAMSHAKE_DURATION );
		m_uLifeCycleState = BOTLIFECYCLE_DEAD;
		m_pPartMgr->DestroyBot( NULL );
		SetBotFlag_IgnoreControls();

		// Reset the death timer to delay a transition to our "buried" state.
		// The buried state basically just prevents respawn or restart for
		// a period of time after death.
		m_fDeathTimer = MultiplayerMgr.IsMultiplayer() ? _DEATH_DELAY_RESPAWN_TIME_MP : _DEATH_DELAY_RESPAWN_TIME;
	}
}


void CBotGlitch::_StartDamageEffect( s32 nBone ) {
	FASSERT( m_pPartMgr && m_pPartMgr->IsCreated() );

	u32 uNumPartEffects = fmath_RandomRange( _DAMAGE_EFFECT_MIN, _DAMAGE_EFFECT_MAX );
	u32 uNumDamEffects = fmath_RandomRange( 0, uNumPartEffects );
	u32 uEffect;

	if( nBone >= 0 ) {
		// smoke the bone that got damaged.
		FASSERT( nBone < m_pWorldMesh->m_pMesh->nBoneCount );

		if( m_uNumActiveDamageParticles < MAX_ACTIVE_DAMAGE_PARTICLES ) {
			uEffect = fmath_RandomChoice( DAMAGE_PARTICLE_ON_BONE_MAX );
          	const CFMtx43A *pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[nBone];
			m_ahActiveDamageParticles[m_uNumActiveDamageParticles] = fparticle_SpawnEmitter( m_ahParticleDamage[uEffect], &(pBoneMtx->m_vPos.v3), &(pBoneMtx->m_vUp.v3) );
			m_afActiveDamageParticleTimers[m_uNumActiveDamageParticles] = fmath_RandomFloatRange( _DAMAGE_PARTICLE_TIME_MIN, _DAMAGE_PARTICLE_TIME_MAX );
			m_uNumActiveDamageParticles++;

			uNumPartEffects--;
		}
	}

	if( NormHealth() > _EXTRA_DAMAGE_THRESHOLD ) {
		return;
	}

	FMATH_CLAMPMIN( uNumPartEffects, 1 );
	FMATH_CLAMPMIN( uNumDamEffects, 1 );

	while( uNumPartEffects > 0 ) {
		if( m_uNumActiveDamageParticles < MAX_ACTIVE_DAMAGE_PARTICLES ) {
			uEffect = fmath_RandomRange( 0, (NUM_DAMAGE_PART_EFFECTS-1) )*2;
			s32 nBone = m_pWorldMesh->FindBone( m_apszBoneNameTable[m_auDamageParticleTable[uEffect]] );
			FASSERT( nBone >= 0 );

			const CFMtx43A *pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[nBone];

			m_ahActiveDamageParticles[m_uNumActiveDamageParticles] = fparticle_SpawnEmitter( m_ahParticleDamage[m_auDamageParticleTable[uEffect+1]], &(pBoneMtx->m_vPos.v3), &(pBoneMtx->m_vFront.v3) );
			m_afActiveDamageParticleTimers[m_uNumActiveDamageParticles] = fmath_RandomFloatRange( _DAMAGE_PARTICLE_TIME_MIN, _DAMAGE_PARTICLE_TIME_MAX );
			m_uNumActiveDamageParticles++;
		}

		uNumPartEffects--;
	}

	if( nBone < 0 ) {
		while( uNumDamEffects > 0 ) {
			uEffect = fmath_RandomChoice( NUM_DAMAGEABLE_LIMBS );
			if( m_pPartMgr->GetLimbState( m_aDamageLimbs[uEffect] ) == CBotPartMgr::LIMB_STATE_INTACT ) {
				m_pPartMgr->SetState_Dangle( m_aDamageLimbs[uEffect] );
			}
			uNumDamEffects--;
		}
	}
}


void CBotGlitch::_ClearAllDamageEffects( void ) {
	FASSERT( m_pPartMgr && m_pPartMgr->IsCreated() );

	// clear up dangling pieces
	m_pPartMgr->ResetAllToIntact();

	// get rid of particle effects...
	for( u32 i=0; i<m_uNumActiveDamageParticles; i++ ) {
		m_afActiveDamageParticleTimers[i] = 0.0f;
		if( m_ahActiveDamageParticles[i] != FPARTICLE_INVALID_HANDLE ) {
			fparticle_KillEmitter( m_ahActiveDamageParticles[i] );
			m_ahActiveDamageParticles[i] = FPARTICLE_INVALID_HANDLE;
		}
	}
    m_uNumActiveDamageParticles = 0;
}


void CBotGlitch::_RepairAllDamageEffects( void ) {
	FASSERT( m_pPartMgr && m_pPartMgr->IsCreated() );

	// clear up dangling pieces
	for( u32 i=0; i<LIMB_TYPE_COUNT; i++ ) {
		if( m_pPartMgr->GetLimbState( i ) == CBotPartMgr::LIMB_STATE_DANGLING ) {
			m_pPartMgr->RepairLimb( i );
		}
	}
    
	// get rid of particle effects...
	for( u32 i=0; i<m_uNumActiveDamageParticles; i++ ) {
		m_afActiveDamageParticleTimers[i] = 0.0f;
		if( m_ahActiveDamageParticles[i] != FPARTICLE_INVALID_HANDLE ) {
			fparticle_StopEmitter( m_ahActiveDamageParticles[i] );
			m_ahActiveDamageParticles[i] = FPARTICLE_INVALID_HANDLE;

		}
	}
    m_uNumActiveDamageParticles = 0;
}


void CBotGlitch::_DamageEffectWork( void ) {

	for( u32 i=0; i<m_uNumActiveDamageParticles; i++ ) {
		m_afActiveDamageParticleTimers[i] -= FLoop_fPreviousLoopSecs;

		if( m_afActiveDamageParticleTimers[i] < 0.0f ) {
			if( m_ahActiveDamageParticles[i] == FPARTICLE_INVALID_HANDLE ) {
			} else {
				fparticle_StopEmitter( m_ahActiveDamageParticles[i] );
				m_ahActiveDamageParticles[i] = FPARTICLE_INVALID_HANDLE;
			}

			// shrink the array
			m_uNumActiveDamageParticles--;
			if( m_uNumActiveDamageParticles > 0 ) {
				m_afActiveDamageParticleTimers[i] = m_afActiveDamageParticleTimers[m_uNumActiveDamageParticles];
				m_ahActiveDamageParticles[i] = m_ahActiveDamageParticles[m_uNumActiveDamageParticles];
				i--;
			}
		}
	}
}

//void CBotGlitch::_BotInPiecesWork( void ) {
//	if( !m_bInPieces ) {
//		return;
//	}
//
//	if( m_fTimeInPieces != 0.0f && m_fTimeInPieces != -1.0f ) {
//		m_fTimeInPieces -= FLoop_fPreviousLoopSecs;
//		FMATH_CLAMPMIN( m_fTimeInPieces, 0.0f );
//
//		if( m_fTimeInPieces == 0.0f ) {
//			m_bFallDown = FALSE;
//			m_bOnGround = FALSE;
//
//			m_fUnitPiecesBlend = 1.0f;
//			ZeroTime( ANIMTAP_ASSEMBLE );
//			SetControlValue( ANIMCONTROL_ASSEMBLE, m_fUnitPiecesBlend );
//			SetControlValue( ANIMCONTROL_DISASSEMBLE, 0.0f );
//		}
//	}
//
//	if( m_bFallDown ) {
//		DeltaTime( ANIMTAP_DISASSEMBLE, FLoop_fPreviousLoopSecs, TRUE );
//		SetControlValue( ANIMCONTROL_DISASSEMBLE, m_fUnitPiecesBlend );
//
//		m_fUnitPiecesBlend += FLoop_fPreviousLoopSecs;
//		FMATH_CLAMPMAX( m_fUnitPiecesBlend, 1.0f );
//
//		if( GetUnitTime( ANIMTAP_DISASSEMBLE ) >= 0.9999f ) {
//			m_bFallAnimDone = TRUE;
//			m_bOnGround = TRUE;
//		} else {
//			aibrainman_SetLODOverrideForOneWork( AIBrain() );
//		}
//	} else {
//		DeltaTime( ANIMTAP_ASSEMBLE, FLoop_fPreviousLoopSecs, TRUE );
//		m_pWorldMesh->SetCollisionFlag( TRUE );
//		SetTargetable( TRUE );
//		FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_UNDER_CONSTRUCTION );
//		_PushAwayVehicles();
//
//		if( GetUnitTime( ANIMTAP_ASSEMBLE ) >= 0.9999f ) {
//			m_fUnitPiecesBlend -= ( 4.0f * FLoop_fPreviousLoopSecs );
//			FMATH_CLAMPMIN( m_fUnitPiecesBlend, 0.0f );
//			SetControlValue( ANIMCONTROL_ASSEMBLE, m_fUnitPiecesBlend );
//
//			if( m_fUnitPiecesBlend == 0.0f ) {
//				m_bInPieces = FALSE;
//				m_fTimeInPieces = 0.0f;
//				
//
//				SetControlValue( ANIMCONTROL_DISASSEMBLE, 0.0f );
//				SetControlValue( ANIMCONTROL_ASSEMBLE, 0.0f );
//
//				// We're done animating, so we can move
//				FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_UNDER_CONSTRUCTION );
//				MobilizeBot();
//				CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWSEnable( TRUE );
//			}
//		}
//	}
//}


void CBotGlitch::SetNormHealth( f32 fNormHealth ) {
	if( fNormHealth > NormHealth() ) {
		_RepairAllDamageEffects();
	}

	CBot::SetNormHealth( fNormHealth );
}

void CBotGlitch::SetupWeaponDamageToArm( CWeapon *pWeap ) {
	FASSERT( pWeap );

	// Make primary weapon pass along damage to upper-right arm...
	pWeap->SetBotDamageBoneIndex( m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_RIGHT_SHOULDER] ) );
}

//// Causes Glitch to break into pieces, or restores him if he's already in pieces.
//// Passing a positive number in fTimeInPieces will cause Glitch to remain in pieces
//// for that amount of time and then re-assemble himself automatically (used with wrench).
//// Otherwise -1.0 should be passed in, which causes Glitch to remain in pieces until
//// another call to the function is made.
//// The parameter bImmediately can be set to TRUE to have Glitch go straight to in-pieces state
//// without animating.  bImmediately does not currently function in the case of re-assembling.
//void CBotGlitch::BreakIntoPieces( const f32 &fTimeInPieces, BOOL bImmediately /*=FALSE*/ ) {
//	if( fTimeInPieces < 0.0f && fTimeInPieces != -1.0f ) {
//		FASSERT_NOW;
//	}
//
//	// We are already in pieces, put ourself back together
//	if( m_fTimeInPieces == -1.0f ) {
//		FASSERT_MSG( !bImmediately, "immediate re-assembly of Glitch not supported" );
//		m_bFallDown = FALSE;
//		m_bOnGround = FALSE;
//
//		m_fTimeInPieces = 0.0f;
//		m_fUnitPiecesBlend = 1.0f;
//
//		ZeroTime( ANIMTAP_ASSEMBLE );
//		SetControlValue( ANIMCONTROL_ASSEMBLE, m_fUnitPiecesBlend );
//		SetControlValue( ANIMCONTROL_DISASSEMBLE, 0.0f );
//
//		return;
//	}
//
//	// we are not in pieces, initialize the break-into-pieces effect
//	if( bImmediately ) {
//		// immediately snap to final in-pieces state
//		m_fUnitPiecesBlend = 1.0f;
//		UpdateUnitTime( ANIMTAP_DISASSEMBLE, 1.0f );
//		ZeroTime( ANIMTAP_ASSEMBLE );
//		SetControlValue( ANIMCONTROL_DISASSEMBLE, m_fUnitPiecesBlend );
//		SetControlValue( ANIMCONTROL_ASSEMBLE, 0.0f );
//		ImmobilizeBot();
//		m_bInPieces = TRUE;
//		m_bFallDown = TRUE;
//		m_bFallAnimDone = TRUE;
//		m_bOnGround = TRUE;
//		m_fTimeInPieces = fTimeInPieces;
//		m_pWorldMesh->SetCollisionFlag( m_bColWithPieces );
//		SetTargetable( FALSE );
//		aibrainman_SetLODOverrideForOneWork( AIBrain() );	// ensure Glitch work runs for one frame to update animation
//		CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWSEnable( FALSE );
//	} else {
//		// init animated effect
//		m_fUnitPiecesBlend = 0.0f;
//		ZeroTime( ANIMTAP_DISASSEMBLE );
//		ZeroTime( ANIMTAP_ASSEMBLE );
//		SetControlValue( ANIMCONTROL_DISASSEMBLE, m_fUnitPiecesBlend );
//		SetControlValue( ANIMCONTROL_ASSEMBLE, 0.0f );
//		ImmobilizeBot();
//		m_bInPieces = TRUE;
//		m_bFallDown = TRUE;
//		m_bFallAnimDone = FALSE;
//		m_fTimeInPieces = fTimeInPieces;
//		m_pWorldMesh->SetCollisionFlag( m_bColWithPieces );
//		SetTargetable( FALSE );
//		CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWSEnable( FALSE );
//	}
//}


BOOL CBotGlitch::SwitchingWeapons( void ) const { 
	FASSERT( IsCreated() ); 
	if( m_nWSState != WS_STATE_NONE ) {
		return TRUE;
	}

	if( (m_WeaponInv[0].m_nWeaponSwitchToIndex != 255) ||
		(m_WeaponInv[1].m_nWeaponSwitchToIndex != 255) ) {
		return TRUE;
	}
	
	return FALSE;
}


///////////////////////
//DEBUG

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	static f32 _fDbgTime = 1.0f;
#endif

void CBotGlitch::_DebugAnimData( void ) {
//#if 0
#if 0 && SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	u32 uCtr = 0;

	//if( m_bControls_Action ) {
	//	BreakIntoPieces( 5.0f );
	//}

	//if( m_bControls_Action ) {
	//	if( _fDbgTime == 1.0f ) {
	//		_fDbgTime = 0.1f;
	//	} else {
	//		_fDbgTime = 1.0f;
	//	}
	//}

	floop_SetTimeScale( _fDbgTime );

	for( u32 i=0; i<ANIMCONTROL_BASE_COUNT; i++ ) {
		if( GetControlValue( i ) != 0.0f ) {
			ftext_DebugPrintf( 0.4f, 0.33f + (0.03f * uCtr++), "~w1ctl:  %s, %0.2f", m_apszBaseControlNameTable[i], GetControlValue( i ) );
		}
	}

	//SetControlValue( ANIMCONTROL_AIM_SUMMER, 1.0f ) ;
	//SetControlValue( ANIMCONTROL_RECOIL_SUMMER, 0.0f );

#endif
}

void CBotGlitch::DrawText( void )
{
	// don't draw text while letterbox is up
	if( letterbox_GetUnitSlideOnAmount() != 0.0f ) {
		return;
	}

	CEConsole::DrawText( this );

	cwchar* wszFormat = (CPlayer::m_nPlayerCount > 1) ? L"~f9~C92929299~w0~aC~o1%ls" : L"~f1~C92929299~w0~aC~s1.00%ls";

	if( IsPlayerBot() && m_pDrivingVehicle && m_pDrivingVehicle->IsUpsideDown() && !m_pDrivingVehicle->IsDeadOrDying() ) {
		ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_FLIP_VEHICLE_OVER ] );
		return;
	}

	// print "enter vehicle" text
	if( m_pActionableEntityNearby &&
		m_nPossessionPlayerIndex >= 0 &&
		!IsDeadOrDying() )	{

		CVehicle* pVehicleNearby = NULL;
		
		if ((m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_VEHICLE) ) {
			pVehicleNearby = (CVehicle*)m_pActionableEntityNearby;
		}

		if( pVehicleNearby && m_pDrivingVehicle == NULL && !pVehicleNearby->IsDeadOrDying() ) {
			if( pVehicleNearby->IsUpsideDown() ) {
				ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_FLIP_VEHICLE_OVER ] );
			} else if( !pVehicleNearby->IsPlayerDriveable() ) {
				ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_VEHICLE_IS_LOCKED ] );
			} else if( pVehicleNearby->CanOccupyStation( this, CVehicle::STATION_DRIVER ) == CVehicle::STATION_STATUS_EMPTY ) {
				ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_DRIVE_VEHICLE ] );
			} else if( pVehicleNearby->CanOccupyStation( this, CVehicle::STATION_GUNNER ) == CVehicle::STATION_STATUS_EMPTY ) {
				ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_OPERATE_GUN ] );
			}
		} else if( (m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_BOT) && ((CBot*)m_pActionableEntityNearby)->IsDeadOrDying() ) {
			//do nothing
		}
		else if( (m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_SITEWEAPON) && ((CBot*)m_pActionableEntityNearby)->m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN ) {
				CBotSiteWeapon* pSiteWeapon = (CBotSiteWeapon*)m_pActionableEntityNearby;
				CBot* pObstructor = pSiteWeapon->IsStationObstructed();
				if ( ((pObstructor == NULL) || (pObstructor == this)) && pSiteWeapon->CanOccupyStation(this) ) {
					ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_OPERATE_GUN ] );
				}
		} else if( m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_BOTAAGUN ) {
			CBotAAGun *pAAGun = (CBotAAGun *)m_pActionableEntityNearby;
			if( pAAGun->CanOccupyStation( this, CVehicle::STATION_GUNNER ) == CVehicle::STATION_STATUS_EMPTY ) {
				// gun is not being used
				if( pAAGun->ShouldEnterTextBeDisplayed() ) {
                    ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_OPERATE_GUN ] );
				}
			}
		} else if ( m_pActionableEntityNearby->IsActionable())	{
			if (m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_BOT)
			{
				CBot* pActionableBot = (CBot*) m_pActionableEntityNearby;
				if (m_pActionableEntityNearby->AIBrain() &&
					!pActionableBot->IsPlayerBot() &&
					!m_pActionableEntityNearby->AIBrain()->GetKnowledge().CanRememberAny(MEMORY_SMALL_PLAYERPROMPTED))
				{
					CFVec3A DeltaToBot;
					DeltaToBot.Sub(pActionableBot->MtxToWorld()->m_vPos, MtxToWorld()->m_vPos);
					if (DeltaToBot.SafeUnitAndMag(DeltaToBot) > 0.0f)
					{
						if (!m_pActionableEntityNearby->AIBrain()->GetLeader() && m_pActionableEntityNearby->AIBrain()->CanFollow(AIBrain()))
						{
							if (MtxToWorld()->m_vFront.Dot(DeltaToBot) > 0.9f)
							{
								ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_RECRUIT_BUDDY ]);   //findfix: put this in gamephrases and then email keith
							}
						}
						else if (m_pActionableEntityNearby->AIBrain()->GetLeader() == this->AIBrain())
						{
							if (MtxToWorld()->m_vFront.Dot(DeltaToBot) > 0.9f)
							{
								ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_DISCHARGE_BUDDY] ); //findfix: put this in gamephrases and then email keith
							}
						}
					}
				}
			}
			else if (m_pActionableEntityNearby->TypeBitsRecurseParents() & ENTITY_BIT_SWITCH)
			{
				CESwitch* pSwitch = (CESwitch*) m_pActionableEntityNearby->GetParentOfType(ENTITY_BIT_SWITCH);   //in case there is a triggerbox that is actually causing the message to appear
				BOOL bCanUse = TRUE;
				if (!pSwitch->HasBeenActivated() && !pSwitch->IsBusy())
				{
					if (pSwitch->IsMilOnly())  //this is botglich code, so we know glitch isn't a mil
					{
						ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_MIL_ONLY] );
						bCanUse = FALSE;
					}
					else if (pSwitch->RequiresChip())
					{
						if ( (m_pInventory == NULL) || (m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo <= 0) )
						{
							ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_CHIP_REQUIRED] );
							bCanUse = FALSE;
						}

					}

					if (bCanUse)
					{
						ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_USE_SWITCH] );  //findfix: put this in gamephrases and then email keith
					}
				}
			}
			else if (m_pActionableEntityNearby->TypeBitsRecurseParents() & ENTITY_BIT_DETPACKDROP)
			{
				if (m_pInventory->m_aoItems[INVPOS_DETPACK].m_nClipAmmo > 0)
					ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_USE_DET_PACK] );	 
				else
					ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_DETPACK_REQUIRED] );	   
			}
			else if (m_pActionableEntityNearby->TypeBits() & ENTITY_BIT_MESHENTITY)
			{
				if (bartersystem_IsBarterBot(m_pActionableEntityNearby))
				{
					ftext_Printf( 0.5f, 0.58f, wszFormat, Game_apwszPhrases[ GAMEPHRASE_PRESS_Y_TO_SHOP ] );	   
				}
			}
		}

	}
}


/////////////////////////
//MELEE
#define		_GSLAP_ANIM_ON_TIME			( 5.0f )
#define		_GSLAP_ANIM_OFF_TIME		( 5.0f )
#define		_GSLAP_ANIM_TIME			( 1.5f )
#define		_GSLAP_ATTACK_FINISHED		( 0.8f )
#define		_MELEE_CENTER_DIST			( 3.2f )		// offset from primary fire bone
#define		_MELEE_SPHERE_RADIUS		( 0.5f )
#define		_MELEE_IMPULSE				( 25.0f )



void CBotGlitch::_HandleMeleeAnimations( void ) {
	f32 fVal;

	switch( m_eMeleeState ) {
		case MELEESTATE_NONE:
			break;

		case MELEESTATE_STARTING:
			// run the animation
			fVal = GetUnitTime( ANIMTAP_GSLAP_UPPER );
			fVal += FLoop_fPreviousLoopSecs * _GSLAP_ANIM_TIME;
			FMATH_CLAMP_MAX1( fVal );
			UpdateUnitTime( ANIMTAP_GSLAP_UPPER, fVal );

			fVal = GetControlValue( ANIMCONTROL_GSLAP_UPPER );
			if( fVal < 1.0f ) {

				// blend the animation in
				fVal += FLoop_fPreviousLoopSecs * _GSLAP_ANIM_ON_TIME;
				FMATH_CLAMP_MAX1( fVal );

				SetControlValue( ANIMCONTROL_GSLAP_UPPER, fVal );
				SetControlValue( ANIMCONTROL_GSLAP_LOWER, fVal );
			} else {
				m_vMeleeLastPos.Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vFront, 0.0f /*_MELEE_CENTER_DIST*/ );
				m_vMeleeLastPos.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vPos );

				m_eMeleeState = MELEESTATE_ATTACKING;
			}
			break;

		case MELEESTATE_ATTACKING:
			fVal = GetUnitTime( ANIMTAP_GSLAP_UPPER );
			fVal += FLoop_fPreviousLoopSecs * _GSLAP_ANIM_TIME;
			FMATH_CLAMP_MAX1( fVal );
			UpdateUnitTime( ANIMTAP_GSLAP_UPPER, fVal );

			if( fVal > _GSLAP_ATTACK_FINISHED ) {
				m_eMeleeState = MELEESTATE_STOPPING;
			}
			break;

		case MELEESTATE_STOPPING:
			DeltaTime( ANIMTAP_GSLAP_UPPER, FLoop_fPreviousLoopSecs * _GSLAP_ANIM_TIME, TRUE );
			fVal = GetControlValue( ANIMCONTROL_GSLAP_UPPER );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs * _GSLAP_ANIM_OFF_TIME;
				FMATH_CLAMP_MIN0( fVal );

				SetControlValue( ANIMCONTROL_GSLAP_UPPER, fVal );
				SetControlValue( ANIMCONTROL_GSLAP_LOWER, fVal );
			} else {
				m_eMeleeState = MELEESTATE_NONE;
				UpdateUnitTime( ANIMTAP_GSLAP_UPPER, 0.0f );

			}

			break;
	}
}


void CBotGlitch::_HandleMeleeAttack( void ) {
	CFVec3A vAttackPos;
	CFVec3A vMove;

	vAttackPos.Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vFront, fmath_Sqrt( GetUnitTime( ANIMTAP_GSLAP_UPPER ) ) * _MELEE_CENTER_DIST );
	vAttackPos.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vPos );

	vMove.Sub( vAttackPos, m_vMeleeLastPos );

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	CFCollData colldata;
	colldata.nCollMask					= FCOLL_MASK_COLLIDE_WITH_PLAYER | FCOLL_MASK_COLLIDE_WITH_OBJECTS | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	colldata.nFlags						= FCOLL_DATA_FLAGS_NONE;
	colldata.nStopOnFirstOfCollMask		= FCOLL_MASK_CHECK_ALL;
	colldata.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	colldata.pCallback					= NULL;//_MeleeHitTrackerCB;
	colldata.pLocationHint				= m_pWorldMesh;
	colldata.pMovement					= &vMove;
	colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
	colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;
	
	CFSphere sphere;
	sphere.Set( m_vMeleeLastPos.v3, _MELEE_SPHERE_RADIUS );
	fcoll_Clear();

	if( fcoll_Check( &colldata, &sphere ) ) {
		if( !m_bMeleeHitWithThisAttack ) {
			_MeleeSpawnEffects( &FColl_aImpactBuf[0] );
			m_bMeleeHitWithThisAttack = TRUE;
		}

		if( FColl_aImpactBuf[0].pTag ) {
            // see if we hit an entity
			CFWorldTracker *pTracker = (CFWorldTracker*)FColl_aImpactBuf[0].pTag;
			if( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH ) {
				if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
					_MeleeInflictDamage( (CEntity*)pTracker->m_pUser, (CFWorldMesh*)pTracker, &FColl_aImpactBuf[0] );
				}
			}
		}
	}
	m_vMeleeLastPos = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vPos;



#if 0
	CFTrackerCollideProjSphereInfo trackerCollInfo;

	m_CollInfo.nCollTestType					= FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.nCollMask						= FCOLL_MASK_COLLIDE_WITH_PLAYER | FCOLL_MASK_COLLIDE_WITH_OBJECTS | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;										   				
	m_CollInfo.nResultsLOD						= FCOLL_LOD_HIGHEST;					
	m_CollInfo.nTrackerUserTypeBitsMask			= FCOLL_USER_TYPE_BITS_ALL;				
	m_CollInfo.nStopOnFirstOfCollMask           = FCOLL_MASK_NONE;						
	m_CollInfo.bFindClosestImpactOnly			= TRUE;									
	m_CollInfo.bCullBacksideCollisions			= FALSE;								
	m_CollInfo.bCalculateImpactData				= TRUE;									

	trackerCollInfo.pProjSphere					= &m_CollInfo.ProjSphere;
	trackerCollInfo.pCallback					= _MeleeHitTrackerCB;
	trackerCollInfo.nTrackerTypeBits			= FWORLD_TRACKERTYPEBIT_MESH;
	trackerCollInfo.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	trackerCollInfo.bIgnoreCollisionFlag		= FALSE;

	m_pCollBot = this;

	fcoll_Clear();

	vAttackPos.Mul( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vFront, fmath_Sqrt( GetUnitTime( ANIMTAP_GSLAP_UPPER ) ) * _MELEE_CENTER_DIST );
	vAttackPos.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vPos );

	m_CollInfo.ProjSphere.Init( &(m_vMeleeLastPos.v3), 
								&(vAttackPos.v3), 
								_MELEE_SPHERE_RADIUS );

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	trackerCollInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	trackerCollInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;

	fworld_CollideWithTrackers( &trackerCollInfo );

//	m_vMeleeLastPos = vAttackPos;
	m_vMeleeLastPos = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPriFire]->m_vPos;

	if( fworld_CollideWithWorldTris( &m_CollInfo ) ) {
		if( !m_bMeleeHitWithThisAttack ) {
			_MeleeSpawnEffects( &FColl_aImpactBuf[0] );
			m_bMeleeHitWithThisAttack = TRUE;
		}
	}
#endif
}


void CBotGlitch::_StartGSlap( void ) {
	AbortReloadImmediately();
	AbortScopeMode( TRUE );

	m_eMeleeState = MELEESTATE_STARTING;
	m_bMeleeHitWithThisAttack = FALSE;
	
	m_uMeleeHitEntityCount = 0;  
}

/*
BOOL CBotGlitch::_MeleeHitTrackerCB( CFWorldTracker *pTracker ) {
	for( u32 i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return FALSE;
		}
	}
	return TRUE;

#if 0
	CFWorldMesh *pWMesh = (CFWorldMesh*)pTracker;

	FASSERT( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH );
	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		return TRUE;
	}

	CEntity *pEntity = (CEntity*)pTracker->m_pUser;

	if( pEntity == m_pCollBot ) {
		return TRUE;
	}

	fcoll_Clear();
	if( !pWMesh->CollideWithMeshTris( &m_CollInfo ) ) {
		// missed
		return TRUE;
	}

	((CBotGlitch*)m_pCollBot)->_MeleeSpawnEffects( &FColl_aImpactBuf[0] );
	((CBotGlitch*)m_pCollBot)->_MeleeInflictDamage( pEntity, pWMesh, &FColl_aImpactBuf[0] );

	return TRUE;
#endif
}
*/

void CBotGlitch::_MeleeInflictDamage( CEntity *pEntity, CFWorldMesh *pWMesh, FCollImpact_t *pImpact ) {
	u32 i;

	if( m_uMeleeHitEntityCount >= _MAX_HIT_ENTITIES ) {
		return;
	}

	for( i=0; i<m_uMeleeHitEntityCount; i++ ) {
		if( pEntity == m_apMeleeHitEntityBuffer[i] ) {
			return;
		}
	}

	// have a new victim
	if( m_uMeleeHitEntityCount < _MAX_HIT_ENTITIES ) {
		m_apMeleeHitEntityBuffer[m_uMeleeHitEntityCount] = pEntity;
		m_uMeleeHitEntityCount++;
	}

	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
	if( !pDamageForm ) {
		return;
	}

	if( aiutils_IsShieldActive( pEntity ) ) {
		return;
	}

	CFVec3A vFireDir;
	f32		fMag2;

	vFireDir.Sub( m_CollInfo.ProjSphere.m_vCenterEnd_WS, m_CollInfo.ProjSphere.m_vCenterStart_WS );
	fMag2 = vFireDir.MagSq();

	if( fMag2 > 0.00001f ) {
		vFireDir.Mul( fmath_InvSqrt( fMag2 ) );
	} else {
		vFireDir = CFVec3A::m_UnitAxisY;
	}

	// submit damage

   	pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_IMPACT;
	pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
	pDamageForm->m_pDamageProfile	= m_BotInfo_Glitch.pDamageProfileGSlap;
	pDamageForm->m_Damager.pWeapon	= NULL;
	pDamageForm->m_Damager.pBot		= this;
	pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
	pDamageForm->m_pDamageeEntity	= pEntity;
	pDamageForm->InitTriDataFromCollImpact( pWMesh, pImpact, &vFireDir );

	CDamage::SubmitDamageForm( pDamageForm );

	if( pEntity->TypeBits() & ENTITY_BIT_BOT ) {
		CBot *pBot = (CBot*)pEntity;
		JustFired();

		if( MultiplayerMgr.IsSinglePlayer() ) {
			if( (ai_GetBrainAlertUnit( pBot->AIBrain() ) == 0.0f) || pBot->IsSleeping() ) {
				if( pBot->m_pBotInfo_Gen->uNumHitsToBreakLimb == 1 ) {
					pBot->BreakAllLimbs();	
				} else {
					pBot->BreakLimb( &m_MtxToWorld.m_vPos );	// pick the bone closest to glitch himself.  This seems to be a good compromise (since the sphere is often on the other side of the tgt bot)
				}
			} else {
				pBot->TryToBreakLimb( &m_MtxToWorld.m_vPos );
			}
		}

		// apply a little impulse
		if( (pBot->GetWeightClass() == BOTWEIGHT_LIGHT) || (pBot->GetWeightClass() == BOTWEIGHT_MEDIUM) ) {
			CFVec3A vToTgt;
			vToTgt.Sub( pBot->MtxToWorld()->m_vPos, MtxToWorld()->m_vPos );
			f32 fm2 = vToTgt.MagSq();
			if( fm2 > 0.0001f ) {
				vToTgt.Mul( fmath_InvSqrt( fm2 ) * _MELEE_IMPULSE );
				pBot->ApplyVelocityImpulse_WS( vToTgt );
			}
		}

		aibrainman_RammedByNotify( pBot->AIBrain(), this );
		
		CDamageData DamageData;
		DamageData.m_Damager.pBot = this;
		DamageData.m_pDamageeEntity = pBot;
		DamageData.m_ImpactPoint_WS = pBot->MtxToWorld()->m_vPos;
		
		//Here's where I tell the ai system that glitch attacked and damaged a bot, this way the bots will 
		//attack Glitch and not just sit there and take it.
		CDamageResult DamageRes;
		DamageRes.m_pDamageData = &DamageData;
		DamageRes.m_fDeltaHitpoints = 0.0f;
		
		aibrainman_DamageDealtNotify( this->AIBrain(), (const CDamageResult*)&DamageRes );
		aibrainman_DamagedNotify( pBot->AIBrain(), (const CDamageResult*)&DamageRes );
	}
}


void CBotGlitch::_MeleeSpawnEffects( const FCollImpact_t *pImpact ) {
	const CGCollMaterial *pMaterial = CGColl::GetMaterial( pImpact );
	
	if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_BITS ) ) {
		pMaterial->DrawParticle(
			CGCollMaterial::PARTICLE_TYPE_BITS,
			&pImpact->ImpactPoint,
			&pImpact->UnitFaceNormal,
			1.0f
		);
	}

	if( pMaterial->HasLooseDust() ) {
		if( pMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST ) ) {
			pMaterial->DrawParticle(
				CGCollMaterial::PARTICLE_TYPE_DUST,
				&pImpact->ImpactPoint,
				&pImpact->UnitFaceNormal,
				1.0f
			);
		}
	}

	PlaySound( m_BotInfo_Glitch.pSoundGroupGSlap );

	if( m_nPossessionPlayerIndex >= 0 ) {
		fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_LOW, &m_hForce );
	}
}

//-----------------------------------------------------------------------------
void CBotGlitch::_ChooseWeaponToDrop( s32* pnWeaponType, s32* pnAmmoCount )
{
	// Pick a random side
	s32 nSide = fmath_RandomRange(0, 1);

	// Starting with the side we picked, check each side to see if it is suitable
	// for dropping. To be suitable, it must be recognized by the collectable
	// system (a hand is a valid weapon, but cannot be dropped or collected), 
	// and it must also have some ammo.
	for (s32 i = 0; i < 2; i++) {
		if (m_apWeapon[nSide]) {
			CollectableType_e eType = CCollectable::ClassifyPlayerWeapon( m_apWeapon[nSide] );
			if (eType != COLLECTABLE_UNKNOWN) {
				s32 nAmmoCount = _CalculateAmmoToDrop( m_apWeapon[nSide] );
				if (nAmmoCount > 0) {
					*pnAmmoCount = nAmmoCount;
					*pnWeaponType = eType;
					return;
				}
			}
		}

		// Try the other side
		nSide = 1 - nSide;
	}

	// Nothing found!
	*pnAmmoCount = -1;
	*pnWeaponType = COLLECTABLE_UNKNOWN;
}

//-----------------------------------------------------------------------------
static s32 _CalculateAmmoToDrop( CWeapon* pWeapon )
{
	if (pWeapon == NULL)
		return -1;

	u16 nClipAmmo = pWeapon->GetClipAmmo();
	if (nClipAmmo == CWeapon::INFINITE_AMMO)
		return -1;

	s32 nTotal = (s32)nClipAmmo;

	u16 nReserve = pWeapon->GetReserveAmmo();
	if (nReserve != CWeapon::INFINITE_AMMO)
		nTotal += (s32)nReserve;

	return nTotal;
}

#if 0 //SAS_ACTIVE_USER == SAS_USER_ELLIOTT
void CBotGlitch::DebugDraw( CBotGlitch *pGlitchy ) {
	if( !(pGlitchy->TypeBits() & ENTITY_BIT_BOTGLITCH) ) {
		return;
	}

	CFVec3A vPt;

	vPt.Mul( pGlitchy->m_pWorldMesh->GetBoneMtxPalette()[pGlitchy->m_nBoneIndexPriFire]->m_vFront, _MELEE_CENTER_DIST );
	vPt.Add( pGlitchy->m_pWorldMesh->GetBoneMtxPalette()[pGlitchy->m_nBoneIndexPriFire]->m_vPos );
	
	fdraw_FacetedWireSphere( &(vPt.v3), _MELEE_SPHERE_RADIUS, 4, 4, &FColor_MotifRed );
	fdraw_FacetedWireSphere( &(pGlitchy->m_vMeleeLastPos.v3), _MELEE_SPHERE_RADIUS, 4, 4, &FColor_MotifBlue );
}
#endif

