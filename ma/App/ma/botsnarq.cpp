//////////////////////////////////////////////////////////////////////////////////////
// botsnarq.cpp - mil snarq data
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
// 05/01/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botsnarq.h"
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
#include "botpart.h"
#include "gamecam.h"
#include "fsound.h"


#define _BOTINFO_FILENAME		( "b_snarq" )
#define _MESH_NAME				( "grmqsnarq00" )

static CBotSnarqBuilder _BotSnarqBuilder;
#define _GUN_TRACER_TEX			( "TFMQshot_01" )
#define _STREAMER_TEX			( 
#define _FIRE_THRESHOLD			( 0.25f )


#define	_BOT_SOUNDBANK			( "Rasp" )		
#define _SCATTERFACTOR			( 0.02f )
#define _POWEREDDOWN_FALL_RATE	( -2.0f )

#define _HEADLIGHT_TEXTURE_ID	( 1 )

#define _MAX_CONTROLLED_ROLL	(FMATH_DEG2RAD( -50.0f ) )
#define _MAX_CONTROLLED_PITCH	(FMATH_DEG2RAD( 30.0f ) )
#define _MAX_EFFECT_ROLL		(FMATH_DEG2RAD( 2.5f ) )

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSnarq
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL 								CBotSnarq::m_bSystemInitialized		= FALSE;
u32									CBotSnarq::m_nBotClassClientCount	= 0;
CFTexInst							CBotSnarq::m_StreamerTex;


CBotSnarq::BotInfo_Gen_t			CBotSnarq::m_BotInfo_Gen;
CBotSnarq::BotInfo_MountAim_t		CBotSnarq::m_BotInfo_MountAim;
CBotSnarq::BotInfo_Walk_t			CBotSnarq::m_BotInfo_Walk;
CBotSnarq::BotInfo_Jump_t			CBotSnarq::m_BotInfo_Jump;
CBotSnarq::BotInfo_Weapon_t			CBotSnarq::m_BotInfo_Weapon;

CBotSnarq::BotInfo_Snarq_t			CBotSnarq::m_BotInfo_Snarq; // = {
//	80.0f,				//f32 fMaxXZVel;
//	250.0f,				//f32 fMaxXZAccel;
//	30.0f,				//f32 fMaxYVel;
//	50.0f,				//f32 fMaxYAccel;
//
//	20,					//u32 uMaxTracers
//	0.45f,				//f32 fTracerWidth;		
//	15.0f,				//f32 fTracerLength;	
//	350.0f,				//f32 fTracerSpeed;	
//	1000.0f,			//f32 fTracerTailDist;
//
//	15,					//uStreamerDepth;
//	0.5f,				//fStreamerAlpha; 
//	2.0f,				//fStreamerSize;
//	15.0f,				//fStreamerSamples;
//	NULL,				// pStreamerTex;
//
//	1.0f/5.0f,			//f32 fOOFireRate
//	1.0f/5.0f,			//f32 fOOFireSoundRate
//
//	0.25f,				// fEngineResponseRate
//	NULL,				//CDamageProfile *pGunDamageProfile;
//	NULL,				// 					FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupRespawn;
//	NULL,				//	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupGSlap;
//	FPARTICLE_INVALID_HANDLE,	// hExhaustParticle;
//	FEXPLOSION_INVALID_HANDLE,	// FExplosion_GroupHandle_t hDeathExplosion2;
//	FEXPLOSION_INVALID_HANDLE,							// FExplosion_GroupHandle_t hDeathExplosion2;
//
//};

CBotSnarq*							CBotSnarq::m_pCBSnarq				= NULL;
TracerGroupHandle_t					CBotSnarq::m_hTracerGroup			= TRACER_NULLGROUPHANDLE;
TracerDef_t							CBotSnarq::m_TracerDef;	
CFTexInst							CBotSnarq::m_GunTracerTex;


BOOL CBotSnarq::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;
	
	return TRUE;
}


void CBotSnarq::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	m_bSystemInitialized = FALSE;
}


BOOL CBotSnarq::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	FTexDef_t *pTexDef;

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
	FResFrame_t frame = fres_GetFrame();

	// Load the sound effects bank for this bot...
	if( !fresload_Load( FSNDFX_RESTYPE, _BOT_SOUNDBANK ) ) {
		DEVPRINTF( "CBotSnarq::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _BOT_SOUNDBANK );
	}

 	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		DEVPRINTF( "CBotSnarq::ClassHierarchyLoadSharedResources():  Error parsing %s\n", _BOTINFO_FILENAME );
		
	}


	////TEMP TEMP TEMP
	//m_BotInfo_Snarq.pGunDamageProfile	= CDamage::FindDamageProfile( "SnarqLaser" );
	//m_BotInfo_Snarq.pHoverSound			= CFSoundGroup::RegisterGroup( "SnarqMove" );
	//m_BotInfo_Snarq.pGunSound			= CFSoundGroup::RegisterGroup( "SnarqFire" );
	//m_BotInfo_Snarq.pStreamerTex		= (FTexDef_t*)fresload_Load(FTEX_RESNAME, "tfdmgrenstr" );
	//m_BotInfo_Snarq.hExhaustParticle	= (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, "PRMQjetbk01" );
	//m_BotInfo_Snarq.hDeathExplosion1	= CExplosion2::GetExplosionGroup( "SnarqDeath1" );
	//m_BotInfo_Snarq.hDeathExplosion2	= CExplosion2::GetExplosionGroup( "SnarqDeath2" );
	////!TEMP


	// load the tracers
	m_TracerDef.pUser					= NULL;
	m_TracerDef.pFcnKillCallback		= _TracerKilledCallback;
	m_TracerDef.pFcnBuildSkipList		= _TracerBuildTrackerSkipList;
	m_TracerDef.fWidth_WS				= m_BotInfo_Snarq.fTracerWidth;		
	m_TracerDef.fLength_WS				= m_BotInfo_Snarq.fTracerLength;	
	m_TracerDef.fSpeed_WS				= m_BotInfo_Snarq.fTracerSpeed;	
	m_TracerDef.fMaxTailDist_WS			= m_BotInfo_Snarq.fTracerTailDist;	
	m_TracerDef.fBeginDeathUnitFade_WS	= 0.75;
	m_TracerDef.uFlags					= TRACERFLAG_DRAW_ADDITIVE;
	m_TracerDef.ColorRGBA.OpaqueWhite();

	pTexDef = (FTexDef_t*)fresload_Load( FTEX_RESNAME, _GUN_TRACER_TEX );
	if( !pTexDef ) {
		DEVPRINTF( "CBotSnarq::ClassHierarchyLoadSharedResources():  Couldn't load tracer tex %s\n", _GUN_TRACER_TEX );
		goto _ExitWithError;
	}

	m_GunTracerTex.SetTexDef( pTexDef );

	m_hTracerGroup = tracer_CreateGroup( &m_GunTracerTex, m_BotInfo_Snarq.uMaxTracers );
	if( m_hTracerGroup == TRACER_NULLGROUPHANDLE ) {
		DEVPRINTF( "CBotSnarq::ClassHierarchyLoadSharedResources():  Couldn't create tracer pool\n" );
		goto _ExitWithError;
	}

	m_StreamerTex.SetTexDef( m_BotInfo_Snarq.pStreamerTex );


	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( frame );

	return FALSE;
}


void CBotSnarq::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}

	tracer_DestroyGroup( m_hTracerGroup );

	CBot::ClassHierarchyUnloadSharedResources();
}


CBotSnarq::CBotSnarq() : CBot() {
	m_pInventory	= NULL;
	m_pWorldMesh	= NULL;
}


CBotSnarq::~CBotSnarq() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotSnarq::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotSnarqBuilder *pBuilder = (CBotSnarqBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotSnarqBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTSCIENTIST );
	}

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotSnarq::_ClearDataMembers( void ) {
	m_fCollCylinderHeight_WS	= 5.0f;
	m_fCollCylinderRadius_WS	= 5.0f;

	m_pBotInfo_Gen				= &m_BotInfo_Gen;
	m_pBotInfo_MountAim			= &m_BotInfo_MountAim;
	m_pBotInfo_Walk				= &m_BotInfo_Walk;
	m_pBotInfo_Jump				= &m_BotInfo_Jump;
	m_pBotInfo_Weapon			= &m_BotInfo_Weapon;
	
	m_fMountPitchMax_WS			= m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS			= m_pBotInfo_MountAim->fMountPitchUpLimit;
	m_pMoveIdentifier			= &m_fAimPitch_WS;

	m_fMaxFlatSurfaceSpeed_WS	= m_BotInfo_Snarq.fMaxXZVel;
	m_fMaxVerticalSpeed_WS		= m_BotInfo_Snarq.fMaxYVel;

	m_Anim.m_pAnimCombiner		= NULL;
	m_hStreamer					= FXSTREAMER_INVALID_HANDLE;
	m_pHoverAudioEmitter		= NULL;
	m_hJetPartTop				= FPARTICLE_INVALID_HANDLE;
	m_hJetPartBottom			= FPARTICLE_INVALID_HANDLE;
	m_fEngineLevel				= 0.0f;

	m_pauArmorBones				= NULL;

}


BOOL CBotSnarq::ClassHierarchyBuild( void ) {
	FMesh_t		*pMesh;
 	FMeshInit_t	meshInit;
	FResFrame_t	frame;
	s32 nBoneIndex;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	frame = fres_GetFrame();

	// get pointer to leaf class's builder object...
	CBotSnarqBuilder *pBuilder = (CBotSnarqBuilder*)GetLeafClassBuilder();

	// set input params for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// build parent...
	if( !CBot::ClassHierarchyBuild() ) {
		goto _ExitWithError;
	}

	// set defaults...
	_ClearDataMembers();

	// init from builder...
	// nothing yet

	// load mesh...
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _MESH_NAME );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotSnarq::ClassHierarchyBuild(): Could not find mesh '%s'\n", _MESH_NAME );
		goto _ExitWithError;
	}

	// create worldmesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotSnarq::ClassHierarchyBuild(): Unable to allocate CFWorldMesh\n" );
		goto _ExitWithError;
	}

	// init worldmesh...
	meshInit.pMesh		= pMesh;
	meshInit.fCullDist	= FMATH_MAX_FLOAT;
	meshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	meshInit.nFlags		= 0;
	m_pWorldMesh->Init( &meshInit );
	m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_NOBONES;

	m_pWorldMesh->m_nUser	= MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser	= this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();


	//// set up our animation
	//m_pAnimCombiner = fnew CFAnimCombiner;
 //   m_pAnimMeshRest = fnew CFAnimMeshRest;
	//m_pAnimMeshRest->Create( m_pWorldMesh->m_pMesh );
	//m_pAnimCombiner->CreateSimple( m_pAnimMeshRest, m_pWorldMesh );

	// grab some bones
	m_nBoneIdxPriFireLeft = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PRIFIRE_L] );
	if( m_nBoneIdxPriFireLeft < 0 ) {
		DEVPRINTF( "CBotSnarq::CLassHierarchyBuild():  Can't find bone %s in model %s\n", m_apszBoneNameTable[BONE_PRIFIRE_L], _MESH_NAME );
		goto _ExitWithError;
	}

	m_nBoneIdxPriFireRight = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_PRIFIRE_R] );
	if( m_nBoneIdxPriFireRight < 0 ) {
		DEVPRINTF( "CBotSnarq::CLassHierarchyBuild():  Can't find bone %s in model %s\n", m_apszBoneNameTable[BONE_PRIFIRE_R], _MESH_NAME );
		goto _ExitWithError;
	}

	m_nBoneIdxJetTop = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_JETBACK1] );
	if( m_nBoneIdxPriFireRight < 0 ) {
		DEVPRINTF( "CBotSnarq::CLassHierarchyBuild():  Can't find bone %s in model %s\n", m_apszBoneNameTable[BONE_JETBACK1], _MESH_NAME );
		goto _ExitWithError;
	}

	m_nBoneIdxJetBottom = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_JETBACK2] );
	if( m_nBoneIdxPriFireRight < 0 ) {
		DEVPRINTF( "CBotSnarq::CLassHierarchyBuild():  Can't find bone %s in model %s\n", m_apszBoneNameTable[BONE_JETBACK2], _MESH_NAME );
		goto _ExitWithError;
	}

	SetMaxHealth();

	m_nBotFlags |= BOTDEATHFLAG_PERSISTAFTERDEATH;
	// Make invincible limbs...

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotSnarq::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	if( !_InitArmorBoneArray() ) {
		return FALSE;
	}
	// Find gaze direction...
//	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_DUMMY ] );
//	if( nBoneIndex < 0 ) {
//		DEVPRINTF( "CBotSnarq::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_DUMMY ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
//	} else {
//		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
//	}

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _HEADLIGHT_TEXTURE_ID );

	m_pAISteerMtx = &m_MtxToWorld;

	// initialize mtx palette...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;

}


void CBotSnarq::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	if( m_pWorldMesh ) {
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	//if( m_pauArmorBones ) {
	//	fdelete( m_pauArmorBones );
	//	m_pauArmorBones = NULL;
	//}

	CBot::ClassHierarchyDestroy();
}


void CBotSnarq::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
	m_pWorldMesh->UpdateTracker();

	// create streamers...
	m_hStreamer = CFXStreamerEmitter::SpawnFromMeshBones( m_BotInfo_Snarq.uStreamerDepth, 
														  m_pWorldMesh, 
														  m_apszStreamerBones, 
														  m_BotInfo_Snarq.fStreamerAlpha, 
														  &m_StreamerTex,
														  m_BotInfo_Snarq.fStreamerSize,
														  m_BotInfo_Snarq.fStreamerSamples, 
														  CFXStreamerEmitter::USE_FRONT_AXIS );


	FASSERT( !m_pHoverAudioEmitter );
	m_pHoverAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Snarq.pHoverSound, FALSE, &m_MountPos_WS );
   
	m_fGunTimer = 0.0f;
	m_fGunSoundTimer = 0.0f;
	m_fEngineLevel = 0.0f;
	m_vUnitPYR.Zero();
	m_fUnitYawTimer = 0.0f;
	m_fUnitFlipTimer = 1.0f;

	// reset flip
	m_FlipMode = FLIP_NONE;
}


void CBotSnarq::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();

	_StopAllEffects();


	CBot::ClassHierarchyRemoveFromWorld();
}


void CBotSnarq::_StopAllEffects( void ) {
	// disable streamers...
	if( m_hStreamer != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamer, FALSE );
		CFXStreamerEmitter::ReturnEmitterHandle( m_hStreamer );
		m_hStreamer = FXSTREAMER_INVALID_HANDLE;
	}

	if( m_pHoverAudioEmitter ) {
		m_pHoverAudioEmitter->Destroy();
		m_pHoverAudioEmitter = NULL;
	}

	if( m_hJetPartTop != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hJetPartTop );
		m_hJetPartTop = FPARTICLE_INVALID_HANDLE;
	}

	if( m_hJetPartBottom != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hJetPartBottom );
		m_hJetPartBottom = FPARTICLE_INVALID_HANDLE;
	}
}


BOOL CBotSnarq::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t frame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


#define _YAW_RATE (0.5f)

CEntityBuilder* CBotSnarq::GetLeafClassBuilder( void ) {
	return &_BotSnarqBuilder;
}


void CBotSnarq::ClassHierarchyWork( void ) {
	FASSERT( IsSystemInitialized() );

	CFVec3A vTmp;

	m_fCollCylinderRadius_WS = 5.0f;

	CFVec3 vTmp3;
	vTmp3.Set( m_BotInfo_Gen.fCollSphere1X_MS, m_BotInfo_Gen.fCollSphere1Y_MS, m_BotInfo_Gen.fCollSphere1Z_MS );
	vTmp3.Add( m_MountPos_WS.v3 );
	//fdraw_DevSphere( &vTmp3, m_BotInfo_Gen.fCollSphere1Radius_MS, &FColor_MotifGreen );

	//@!TEMP

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	PROTRACK_BEGINBLOCK("Snarq");

	_Power_Work();

	// take care of this here, because the bot is always in the air
	if( IsImmobilizePending() ) {
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
	}

	if( m_FlipMode != FLIP_QUICKTURN ) {
		ParseControls();
	} else {
		ZeroControls();
	}

	//ALL DEBUG STUFF
	if( IsPlayerBot() ) {
		m_fControls_FlyUp = m_fControls_Fire1;
		m_fControls_FlyUp -= m_fControls_Fire2;
		m_fControls_Fire1 = ((CHumanControl*)Controls())->m_nPadFlagsJump ? 1.0f : 0.0f;

		if( m_bControls_Action ) Die();

		//if( m_bControls_Action ) {
		//	StartQuickturn( 0.0f );
		//}

		//if( m_bControls_Action ) {
		//	StartFlip( FLIP_ROLL, 1 );
		//}
	}
	//END DEBUG

	// update the engine level...
	f32 fDesiredEngineLevel = m_XlatStickUnitVecXZ_WS.Mag();
	if( m_fEngineLevel < fDesiredEngineLevel ) {
		m_fEngineLevel += m_BotInfo_Snarq.fEngineResponseRate * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_fEngineLevel, fDesiredEngineLevel );
	} else if( m_fEngineLevel > fDesiredEngineLevel ) {
		m_fEngineLevel -= m_BotInfo_Snarq.fEngineResponseRate * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fEngineLevel, fDesiredEngineLevel );
	}
    
	// save a copy of previous frame info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// add in any velocity impulse...
	BOOL bImpulseApplied = HandleVelocityImpulses();
	
	// update position...
	vTmp.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( vTmp );

	// pitch & yaw...
	HandlePitchMovement();
	HandleYawMovement();

	WS2MS( m_Velocity_MS, m_Velocity_WS );

	if( bImpulseApplied ) {
		VelocityHasChanged();
	}

	// Update translation analog stick vectors from raw controller data...
	ComputeXlatStickInfo();

	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
		HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	// hanlde all translation
	_HandleTranslation();

	PROTRACK_BEGINBLOCK("Guns");
		HandleTargeting();
		_GunWork();
	PROTRACK_ENDBLOCK();//"Guns"

	_CalcPitchYawRoll();
	_FlipWork();
	_UpdateMatrices();

	_SoundWork();
	_ParticleWork();

	UpdateSpotLight();

	if( IsDeadOrDying() ) {
		DeathWork();
	}

	PROTRACK_ENDBLOCK(); //"Scientist"
}


void CBotSnarq::_HandleTranslation( void ) {

	if( IsDeadOrDying() ) {
		m_Velocity_WS.y += m_BotInfo_Gen.fGravity * FLoop_fPreviousLoopSecs;
		VelocityHasChanged();
		return;
	}

	// XZ direction
	f32 fVal, fAccelMag;
	CFVec3A vDesVel;
	CFVec3A vDeltaV;
	
	vDesVel.Mul( m_XlatStickNormVecXZ_WS, m_BotInfo_Snarq.fMaxXZVel );
	vDeltaV.Sub( vDesVel, m_Velocity_WS );
	vDeltaV.y = 0;
    
	fAccelMag = vDeltaV.Mag();

	if( fAccelMag > 0.01f ) {
		fVal = fmath_Inv( fAccelMag );
		vDeltaV.x *= fVal;
		vDeltaV.z *= fVal;
		vDeltaV.y = 0.0f;

		FMATH_CLAMPMAX( fAccelMag, m_BotInfo_Snarq.fMaxXZAccel * FLoop_fPreviousLoopSecs );
		vDeltaV.Mul( fAccelMag );
		m_Velocity_WS.Add( vDeltaV );
	}


	// Y direction
	f32 fDesYVel;
	f32 fDeltaYVel;
	fDesYVel = m_fControls_FlyUp * m_BotInfo_Snarq.fMaxYVel;
	fDeltaYVel = fDesYVel - m_Velocity_WS.y;
	FMATH_BIPOLAR_CLAMPMAX( fDeltaYVel, m_BotInfo_Snarq.fMaxYAccel * FLoop_fPreviousLoopSecs );
	m_Velocity_WS.y += fDeltaYVel;


	if( Power_IsPoweredDown() ) {
		m_Velocity_WS.y = _POWEREDDOWN_FALL_RATE;
	}

	VelocityHasChanged();
}


void CBotSnarq::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}


const CFVec3A* CBotSnarq::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}

void CBotSnarq::_UpdateMatrices( void ) {
	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.SetRotationYXZ(	m_fMountYaw_WS, 
										m_fMountPitch_WS + (m_vUnitPYR.x * _MAX_CONTROLLED_PITCH), 
										m_vUnitPYR.z * _MAX_CONTROLLED_ROLL );

	if( m_FlipMode != FLIP_NONE ) {
		_BuildFlipMtx( &CFMtx43A::m_Temp );
	}

	CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;

	if( DataPort_IsBeingShocked() ) {
		fmath_ScatterUnitVec( &CFMtx43A::m_Temp.m_vFront, 0.1f );
		fmath_ScatterUnitVec( &CFMtx43A::m_Temp.m_vUp, 0.1f );
		fmath_ScatterUnitVec( &CFMtx43A::m_Temp.m_vRight, 0.1f );
	}

	Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );
	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp );
	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		m_pWorldMesh->AtRestMtxPalette();
		_MoveDestroyedBones();
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	//if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	//} else {
	//	m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	//}
}


void CBotSnarq::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


void CBotSnarq::CheckpointRestore( void ) {
	CBot::CheckpointRestore();

	// reset all armor bones to not be damaged.
	for( u32 i=0; i<m_pWorldMesh->m_pMesh->nBoneCount; i++ ) {
		m_pauArmorBones[i] &= ~BONEFLAG_DESTROYED;
	}
}


BOOL CBotSnarq::CheckpointSave( void ) {
	return CBot::CheckpointSave();
}


void CBotSnarq::_AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT_NOW;
}



/////////////////////////////
// WEAPONS

void CBotSnarq::_GunWork( void ) {
//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	fdraw_DevSphere( &m_TargetedPoint_WS.v3, 3.0f, &FColor_MotifBlue );
//#endif

	if( m_fGunTimer > 0.0f ) {
		m_fGunTimer -= FLoop_fPreviousLoopSecs;
		return;
	}

	if( m_fControls_Fire1 > _FIRE_THRESHOLD ) {
		_FireRound();
		m_fGunTimer += m_BotInfo_Snarq.fOOFireRate;
	}
}


void CBotSnarq::_FireRound( void ) {
	const CFMtx43A *pFireMtx;

	m_TracerDef.pUser = this;

	if( m_bFireLeftSide ) {
		pFireMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPriFireLeft];
	} else {
		pFireMtx = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxPriFireRight];
	}

	m_TracerDef.UnitDir_WS.Sub( m_TargetedPoint_WS, pFireMtx->m_vPos );
	m_TracerDef.TailPos_WS = pFireMtx->m_vPos;
	f32 fMag2 = m_TracerDef.UnitDir_WS.MagSq();

	if( fMag2 > 0.001f ) {
		m_TracerDef.UnitDir_WS.Mul( fmath_InvSqrt( fMag2 ) );
	} else {
		return;
	}

	fmath_ScatterUnitVec( &m_TracerDef.UnitDir_WS, _SCATTERFACTOR );

	ConstructDamagerData();
	if( tracer_NewTracer( m_hTracerGroup, &m_TracerDef, 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager ) ) {
		JustFired();
		m_bFireLeftSide = !m_bFireLeftSide;
		PlaySound( m_BotInfo_Snarq.pGunSound );
	}
}


void CBotSnarq::_TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
	if( nKillReason != TRACER_KILLREASON_HIT_GEO ) {
		return;
	}

	FASSERT( pTracerDef->pUser );
	//would fassert here that we were the right entity if we had a bit

	const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );

	pCollMaterial->DrawAllDrawableParticles( &pImpact->ImpactPoint, &pImpact->UnitFaceNormal, TRUE,
											 fmath_RandomFloatRange( 0.2f, 0.4f ),		// sparks
											 fmath_RandomFloatRange( 0.25f, 0.75f ),	// bits
											 0.0f );									// dust

   	CEntity *pEntity = CGColl::ExtractEntity( pImpact );

	if( pEntity ) {
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if( pDamageForm ) {
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile	= m_BotInfo_Snarq.pGunDamageProfile;
			pDamageForm->m_Damager.pWeapon	= NULL;
			pDamageForm->m_Damager.pBot		= (CBotSnarq*)pTracerDef->pUser;
			pDamageForm->m_Damager.nDamagerPlayerIndex = ((CBotSnarq*)pTracerDef->pUser)->m_nPossessionPlayerIndex;
			pDamageForm->m_pDamageeEntity	= pEntity; 
			pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh*)pImpact->pTag, pImpact, &pTracerDef->UnitDir_WS );

			CDamage::SubmitDamageForm( pDamageForm );

		}
	}
}


void CBotSnarq::_TracerBuildTrackerSkipList( void *pUser ) {
	FASSERT( pUser );

	FWorld_nTrackerSkipListCount = 0;
	((CBotSnarq*)pUser)->AppendTrackerSkipList();
}


void CBotSnarq::_SoundWork( void ) {
	if( m_pHoverAudioEmitter ) {
		m_pHoverAudioEmitter->SetPosition( &m_MountPos_WS );
	}
}


void CBotSnarq::_ParticleWork( void ) {
	if( m_hJetPartTop == FPARTICLE_INVALID_HANDLE ) {
		m_hJetPartTop = fparticle_SpawnEmitter( m_BotInfo_Snarq.hExhaustParticle, 
												&(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxJetTop]->m_vPos.v3), 
												&(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxJetTop]->m_vFront.v3),
												&(m_Velocity_WS.v3), 1.0f );
		fparticle_SetIntensity( m_hJetPartTop, &m_fEngineLevel );
	}

	if( m_hJetPartBottom == FPARTICLE_INVALID_HANDLE ) {
		m_hJetPartBottom = fparticle_SpawnEmitter( m_BotInfo_Snarq.hExhaustParticle, 
												   &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxJetBottom]->m_vPos.v3), 
												   &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxJetBottom]->m_vFront.v3), 
												   &(m_Velocity_WS.v3), 1.0f );
		fparticle_SetIntensity( m_hJetPartBottom, &m_fEngineLevel );
	}
}


BOOL CBotSnarq::_InitArmorBoneArray( void ) {
	u32 i;
	u32 uBoneCt = m_pWorldMesh->m_pMesh->nBoneCount;

	FASSERT( uBoneCt < 100 );
	if( uBoneCt >= 100 ) {
		DEVPRINTF( "CBotSnarq::_InitArmorBoneArray():  Mesh has a ridiculous number of bones\n" );
		return FALSE;
	}

	m_pauArmorBones = (u8*)fres_AllocAndZero( sizeof( u8 ) * uBoneCt );
	if( !m_pauArmorBones ) {
		DEVPRINTF( "CBotSnarq::_InitArmorBoneArray():  Couldn't allocate array of bone states\n" );
		return FALSE;
	}

	for( i=0; i<uBoneCt; i++ ) {
		if( !(fclib_strnicmp( "Armor", m_pWorldMesh->m_pMesh->pBoneArray[i].szName, fclib_strlen( "Armor" ))) ||
			!(fclib_strnicmp( "L_Armor", m_pWorldMesh->m_pMesh->pBoneArray[i].szName, fclib_strlen( "L_Armor" ))) ||
			!(fclib_strnicmp( "R_Armor", m_pWorldMesh->m_pMesh->pBoneArray[i].szName, fclib_strlen( "R_Armor" ))) ) {
		
			m_pauArmorBones[i] = BONEFLAG_ARMOR;
		}
	}

	return TRUE;
}


void CBotSnarq::InflictDamageResult( const CDamageResult* pDamageResult ) {
	CBot::InflictDamageResult( pDamageResult );

	if( pDamageResult->m_pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_AMBIENT ) {
		_CheckForArmorBoneDestroy( pDamageResult );
	}

	return;
}


void CBotSnarq::_CheckForArmorBoneDestroy( const CDamageResult* pDamageResult ) {
	u32	uBoneToBreak = -1;
	u32 i;
	CFSphere bonesphere;
	f32 fVal;
	f32 fDistSq;
	CFVec3A vDebrisDir;

	if( pDamageResult->m_pDamageData->m_nDamageLocale ==  CDamageForm::DAMAGE_LOCALE_IMPACT ) {
		uBoneToBreak = pDamageResult->m_pDamageData->m_pTriData->nDamageeBoneIndex;
		if( uBoneToBreak < m_pWorldMesh->m_pMesh->nBoneCount ) {
			if( (m_pauArmorBones[uBoneToBreak] & BONEFLAG_ARMOR) ) {
				m_pauArmorBones[uBoneToBreak] |= BONEFLAG_DESTROYED;
				vDebrisDir = pDamageResult->m_pDamageData->m_pTriData->ImpactUnitNormal_WS;
			} else {
				uBoneToBreak = -1;
			}
		}

	} else if( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST ) {
		// find closest bone that can blow up
		fDistSq = FMATH_MAX_FLOAT;
		for( i=0; i<m_pWorldMesh->m_pMesh->nBoneCount; i++ ) {
			if( m_pauArmorBones[i] & BONEFLAG_ARMOR && !(m_pauArmorBones[i] & BONEFLAG_DESTROYED) ) {
				m_pWorldMesh->GetBoneSphere_WS( i, &bonesphere );
				fVal = pDamageResult->m_pDamageData->m_pEpicenter_WS->DistSq( bonesphere.m_Pos );
				if( fVal < fDistSq ) {
					fDistSq = fVal;
					uBoneToBreak = i;
					vDebrisDir = bonesphere.m_Pos.Sub( pDamageResult->m_pDamageData->m_pEpicenter_WS->v3 );
					fVal = vDebrisDir.MagSq();
					if( fVal > 0.001f ) {
						vDebrisDir.Mul( fmath_InvSqrt( fVal ) );
					} else {
						vDebrisDir = CFVec3A::m_UnitAxisY;
					}
				}
			}
		}

	}


	if( uBoneToBreak < m_pWorldMesh->m_pMesh->nBoneCount ) {
		CFDebrisSpawner DebrisSpawner;
		DebrisSpawner.InitToDefaults();

		DebrisSpawner.m_Mtx.m_vPos = m_pWorldMesh->GetBoneMtxPalette()[uBoneToBreak]->m_vPos;
		DebrisSpawner.m_Mtx.m_vPos.FCheck();

		DebrisSpawner.m_Mtx.m_vZ = vDebrisDir;
		DebrisSpawner.m_Mtx.m_vZ.FCheck();

		DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
		DebrisSpawner.m_pDebrisGroup = m_pBotInfo_Gen->pDebrisGroup_Chunks;
		DebrisSpawner.m_fMinSpeed = 10.0f;
		DebrisSpawner.m_fMaxSpeed = 20.0f;
		DebrisSpawner.m_fUnitDirSpread = 0.2f;
		DebrisSpawner.m_nMinDebrisCount = 1;
		DebrisSpawner.m_nMaxDebrisCount = 3;


		CGColl::SpawnDebris( &DebrisSpawner );
	}
}


void CBotSnarq::_MoveDestroyedBones( void ) {
	for( u32 i=0; i<m_pWorldMesh->m_pMesh->nBoneCount; i++ ) {
		if( m_pauArmorBones[i] & BONEFLAG_DESTROYED ) {
			m_pWorldMesh->GetBoneMtxPalette()[i]->m_vPos.Set( 2000000.0f, 2000000.0f, 2000000.0f );
		}
	}
}

void CBotSnarq::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies/*=TRUE*/ ) {
	CBot::Die( bSpawnDeathEffects, bSpawnGoodies );

	if( bSpawnDeathEffects ) {
		FExplosionSpawnParams_t SpawnParams;
		FExplosion_SpawnerHandle_t hSpawner;

		SpawnParams.InitToDefaults();
		SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
		SpawnParams.Pos_WS = m_MountPos_WS;
		SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
		SpawnParams.uSurfaceType = 0;
		SpawnParams.pDamageProfile = NULL;
		SpawnParams.pDamager = NULL;

		hSpawner = CExplosion2::GetExplosionSpawner();
		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			CExplosion2::SpawnExplosion( hSpawner, m_BotInfo_Snarq.hDeathExplosion1, &SpawnParams );
		}

		ClearSpin();
		StartRoll( 20.0f );
	}

	// swap out mesh here
}


void CBotSnarq::DeathWork( void ) {
	if( GetCollisionState() ) {
		RemoveFromWorld();
		m_uLifeCycleState = BOTLIFECYCLE_DEAD;

		FExplosionSpawnParams_t SpawnParams;
		FExplosion_SpawnerHandle_t hSpawner;

		SpawnParams.InitToDefaults();
		SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
		SpawnParams.Pos_WS = m_MountPos_WS;
		SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
		SpawnParams.uSurfaceType = 0;
		SpawnParams.pDamageProfile = NULL;
		SpawnParams.pDamager = NULL;

		hSpawner = CExplosion2::GetExplosionSpawner();
		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			CExplosion2::SpawnExplosion( hSpawner, m_BotInfo_Snarq.hDeathExplosion2, &SpawnParams );
		}
	}
}

#define _SWAY_RATE				( 0.1f )
#define _MIN_ROLL_RATE			( 0.1f )
#define _MAX_ROLL_RATE			( 1.0f )
#define _ROLL_ADD_FOR_YAW		(0.33f )

void CBotSnarq::_CalcPitchYawRoll( void ) {
	CFVec3A vNormVel_MS;

	vNormVel_MS.Div( m_Velocity_MS, m_BotInfo_Snarq.fMaxXZVel );
	f32 fXF, fXS, fDF;

	m_fUnitYawTimer += FLoop_fPreviousLoopSecs * _SWAY_RATE;
	if( m_fUnitYawTimer > 1.0f ) {
		m_fUnitYawTimer -= 1.0f;
	}


	// ROLL
	fXS = vNormVel_MS.x;
	fXF = m_XlatStickNormVecXZ_MS.x;

	fDF = m_XlatStickNormVecXZ_MS.x - vNormVel_MS.x;

	fDF += m_fControls_RotateCW * _ROLL_ADD_FOR_YAW;

	if( FMATH_FABS( fXF ) > FMATH_FABS( fDF ) ) {
		fDF = fXF;
	}

	f32 fDiffMag = FMATH_FABS(fDF - m_vUnitPYR.z);
	f32 fRollRate = FMATH_FPOT( fDiffMag, _MIN_ROLL_RATE, _MAX_ROLL_RATE );

	fDF += fmath_Sin( m_fUnitYawTimer * FMATH_2PI ) * _MAX_EFFECT_ROLL;

	if( m_vUnitPYR.z < fDF ) {
		m_vUnitPYR.z += fRollRate * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_vUnitPYR.z, fDF );
	} else if( m_vUnitPYR.z > fDF ) {
		m_vUnitPYR.z -= fRollRate * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_vUnitPYR.z, fDF );
	}

	//PITCH
	fXS = vNormVel_MS.z;
	fXF = m_XlatStickNormVecXZ_MS.z;

	fDF = m_XlatStickNormVecXZ_MS.z - vNormVel_MS.z;

	if( FMATH_FABS( fXF ) > FMATH_FABS( fDF ) ) {
		fDF = fXF;
	}

	if( m_vUnitPYR.x < fDF ) {
		m_vUnitPYR.x += _YAW_RATE * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_vUnitPYR.x, fDF );
	} else if( m_vUnitPYR.x > fDF ) {
		m_vUnitPYR.x -= _YAW_RATE * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_vUnitPYR.x, fDF );
	}


//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	ftext_DebugPrintf( 0.2f, 0.4f, "~w1X: %0.2f (%0.2f)", m_vUnitPYR.z, fDF );
//#endif
}

#define _FLIP_RADS_PER_SEC ( FMATH_2PI * 1.0f )


void CBotSnarq::_FlipWork( void ) {
	if( m_fUnitFlipTimer >= 1.0f ) {
		m_fUnitFlipTimer = 1.0f;
		m_FlipMode = FLIP_NONE;
		return;
	}
	m_fUnitFlipTimer += FLoop_fPreviousLoopSecs * m_fOOFlipTime;
	


	switch( m_FlipMode ) {
	case FLIP_ROLL:
		//m_fFlipRads += _FLIP_RADS_PER_SEC * FLoop_fPreviousLoopSecs;
		m_fFlipRads = m_fFlipRevolutions * FMATH_2PI * fmath_UnitLinearToSCurve( m_fUnitFlipTimer );
		if( m_fFlipRads > FMATH_2PI ) {
			m_fFlipRads -= FMATH_2PI;
		}
		break;

	case FLIP_QUICKTURN:
		break;

	default:
		break;

	}
}


void CBotSnarq::StartFlip( FlipMode_e flipType, f32 fNumRevs/*=1.0f*/ ) {
	FASSERT( IsCreated() );

	if( m_FlipMode != FLIP_NONE ) {
		return;
	}
    
	m_FlipMode = flipType;
	m_fUnitFlipTimer = 0.0f;
	m_fFlipRevolutions = fNumRevs;
	m_fOOFlipTime = fmath_Div( _FLIP_RADS_PER_SEC, fNumRevs * FMATH_2PI );
	
	switch( m_FlipMode ) {
	case FLIP_ROLL:
		m_vFlipAxis_MS = CFVec3A::m_UnitAxisZ;
        m_fFlipRads = 0.0f;
		break;

	default:
		break;
	}
}

#define _OO_QUICKTURN_TIME		( 5.0f )

void CBotSnarq::StartQuickturn( f32 fNewMountYaw_WS ) {
	FASSERT( IsCreated() );
	CFMtx43A mFinal, mHalfway;
	f32 fSin, fCos;

	// calculate final matrix
	fmath_SinCos( fNewMountYaw_WS, &fSin, &fCos );
	mFinal.m_vFront.Set( fSin, 0.0f, fCos );
	mFinal.m_vUp = CFVec3A::m_UnitAxisY;
	mFinal.m_vRight.Set( fCos, 0.0f, -fSin );

	mHalfway.m_vFront = CFVec3A::m_UnitAxisY;
	if( m_fMountPitch_WS < 0.0f ) {
		mHalfway.m_vFront.Negate();
	}

	mHalfway.m_vUp = m_MountUnitFrontXZ_WS;  //m_MtxToWorld.m_vRight;
	//if( m_MtxToWorld.m_vRight.Dot( mFinal.m_vFront ) < 0.0f ) {
	//	mHalfway.m_vUp.Negate();
	//}
		
	mHalfway.m_vRight.Cross( mHalfway.m_vUp, mHalfway.m_vFront );
	FASSERT( mHalfway.m_vRight.MagSq() > 0.01f );
	mHalfway.m_vRight.Unitize();	// impossible that the right axis could be straight up or down

	m_qQTStart.BuildQuat( m_MtxToWorld );
	m_qQTFinal.BuildQuat( mFinal );
	m_qQTHalfway.BuildQuat( mHalfway );

	m_FlipMode = FLIP_QUICKTURN;
	m_fUnitFlipTimer = 0.0f;
	m_fOOFlipTime = _OO_QUICKTURN_TIME;

	ChangeMountYaw( fNewMountYaw_WS );
	m_fMountPitch_WS = 0.0f;

}


void CBotSnarq::_BuildFlipMtx( CFMtx43A *pmtx ) {
	switch( m_FlipMode ) {
	case FLIP_ROLL: {
		CFVec3A vFlipAxis_WS;
		CFQuatA qRot;
		CFQuatA qFrom, qTo;

		CFMtx43A::m_Temp.MulDir( vFlipAxis_WS, m_vFlipAxis_MS );

		qRot.BuildQuat( vFlipAxis_WS, m_fFlipRads );
		qRot.MulPoint( pmtx->m_vFront );
		qRot.MulPoint( pmtx->m_vRight );
		qRot.MulPoint( pmtx->m_vUp );
		break;
	}

	case FLIP_QUICKTURN: {
		CFQuatA qRot;
//		f32 fSlerp;
		qRot.ReceiveSlerpOf( m_fUnitFlipTimer, m_qQTStart, m_qQTFinal );
		//if( m_fUnitFlipTimer < 0.5f ) {
		//	fSlerp = m_fUnitFlipTimer * 2.0f;
		//	qRot.ReceiveSlerpOf( fSlerp, m_qQTStart, m_qQTHalfway  );
		//} else {
		//	fSlerp = (m_fUnitFlipTimer - 0.5f) * 2.0f;
		//	qRot.ReceiveSlerpOf( fSlerp, m_qQTHalfway, m_qQTFinal );
		//}
		qRot.BuildMtx33( *pmtx );
		break;
	}
	default:
		break;
	}
}


void CBotSnarq::ClearSpin( void ) {
	m_FlipMode = FLIP_NONE;
}


void CBotSnarq::StartRoll( f32 fNumRevs/*=1.0f*/, u32 uDirection/*=0*/ ) {
	FASSERT( IsCreated() );

	if( m_FlipMode != FLIP_NONE ) {
		return;
	}
    
	m_FlipMode = FLIP_ROLL;
	m_fUnitFlipTimer = 0.0f;
	m_fFlipRevolutions = fNumRevs;
	m_fOOFlipTime = fmath_Div( _FLIP_RADS_PER_SEC, fNumRevs * FMATH_2PI );
	
	m_vFlipAxis_MS = CFVec3A::m_UnitAxisZ;
	if( (uDirection == -1) || ((uDirection == 0) && fmath_RandomChance( 0.5f )) ) {
		m_vFlipAxis_MS.Negate();
	}

	m_fFlipRads = 0.0f;
}


void CBotSnarq::_Power_Work( void ) {
	switch( m_nPowerState ) {
	case POWERSTATE_POWERED_UP:
		break;

	case POWERSTATE_POWERED_DOWN:
		// If we were turned off on a timer...
		if( m_fPowerOffOnTime > 0.0f ) {
			m_fPowerOffOnTime -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fPowerOffOnTime, 0.0f );

			// Time is done, turn the bot back on
			if( m_fPowerOffOnTime == 0.0f && !IsDead() ) {
				Power( TRUE );
			}
		}
		break;

	case POWERSTATE_WAITING_FOR_IMMOBILIZED:
	if( IsImmobile() ) {
		// Bot is finally immobilized...
		m_nPowerState = POWERSTATE_POWERING_DOWN;
		ZeroTime( ASI_RC_POWER_DOWN );
		m_fPowerAnimUnitBlend = 0.0f;
	} else {
		// keep trying to immobilize him.  fixes bug where bot is 
		// re-mobilized in same frame after being EMPed.
		ImmobilizeBot();
	}

	case POWERSTATE_POWERING_DOWN:
		m_nPowerState = POWERSTATE_POWERED_DOWN;
		if( m_bOppositePowerStatePending ) {
			// Start power-up...
			Power( TRUE, 0.0f, m_fPendingPowerOffOnAnimSpeedMult );
		}
		break;

	case POWERSTATE_POWERING_UP:
		// Done powering up...
		m_nPowerState = POWERSTATE_FINISHING_UP;
		break;

	case POWERSTATE_FINISHING_UP:
		m_nPowerState = POWERSTATE_POWERED_UP;
		if( m_bOppositePowerStatePending ) {
			// Start powering down...
			Power( FALSE, m_fPendingOffOnTime, m_fPendingPowerOffOnAnimSpeedMult );
		} else {
			MobilizeBot();
		}

		break;

	default:
		FASSERT_NOW;
	}

	if( (m_nPowerState == POWERSTATE_POWERING_DOWN) || (m_nPowerState == POWERSTATE_POWERED_DOWN) ) {
		if( m_fMountPitch_WS < m_BotInfo_MountAim.fMountPitchDownLimit ) {
			m_fMountPitch_WS += FLoop_fPreviousLoopSecs * 0.5f;
			FMATH_CLAMPMAX( m_fMountPitch_WS, m_BotInfo_MountAim.fMountPitchDownLimit );
		}
	}

}

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSnarqBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CBotSnarqBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTSNARQ, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotSnarq::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotSnarq::m_BotInfo_Gen.pszArmorProfile;

	 // This class of bots can be recruited...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED | BOT_BUILDER_DPORT_NOT_NEEDED_FOR_RECRUITMENT );

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotSnarqBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotSnarqBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}
