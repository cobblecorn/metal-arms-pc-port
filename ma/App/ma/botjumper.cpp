//////////////////////////////////////////////////////////////////////////////////////
// botjumper.cpp - Jump Trooper bot data
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
// 02/09/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botjumper.h"
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
//#include "user_elliott.h"

#define _DEBUG_START_SLEEPING 0


#define _BOTINFO_FILENAME					( "b_jumper" )
#define _BOTPART_FILENAME					( "bp_jumper" )
#define _SOUND_BANK							( "Jumper" )
#define _MESH_NAME							( "grmjjumpt00" )

#define _MELEE_BONE_LEFT					( "off_L_Arm_Dummy" )
#define _MELEE_BONE_RIGHT					( "off_R_Arm_Dummy" )

#define	_JET_BONE_LEFT						( "AttachPoint_L_Jet" )
#define	_JET_BONE_RIGHT						( "AttachPoint_R_Jet" )

#define _SMOKE_BONE_LEFT					( "AttachPoint_L_Smoke" )
#define _SMOKE_BONE_RIGHT					( "AttachPoint_R_Smoke" )

#define _JETS_ON_RATE						( 10.0f )
#define _JETS_OFF_RATE						( 2.0f )
#define _JUMPJETS_ANIM_ON_RATE				( 5.0f )
#define _JUMPJETS_DIVE_TIME_START			( 0.5f )

#define _MIN_TORSO_TWIST					( FMATH_DEG2RAD( -35.0f ) )
#define _MAX_TORSO_TWIST					( FMATH_DEG2RAD( 35.0f ) )
#define _MIN_HEAD_TWIST						( FMATH_DEG2RAD( -20.0f ) )
#define _MAX_HEAD_TWIST						( FMATH_DEG2RAD( 20.0f ) )
#define _ARM_SLERP_RATE						( 30.0f )

// melee
#define _ATTACK_BLEND_IN_RATE				( 15.0f )
#define _ATTACK_BLEND_OUT_RATE				( 5.0f )

// tracers
#define _GUN_TRACER_TEX						( "TFMJshot001" )
#define _TRACER_STREAMER_TEX				( "TFMJshot002" )  

#define _GUNSTREAMER_DEPTH					( 15 )
#define _GUNSTREAMER_SAMPLERATE				( 60.0f )
#define _GUNSTREAMER_ALPHA					( 0.5f )
#define _GUNSTREAMER_WIDTH					( 0.75f )
#define _GUNSTREAMER_ROTATE_RATE			( 2.5f )
#define _TRACER_LEN							( 30.0f )

#define _STREAMER_TEX_BLADE					( "TFMJstrslv1" )
#define _STREAMER_TEX_GREEN					( "TFMJstrgrn1" )
#define _STREAMER_TEX_RED					( "TFMJstrred1" )

#define _WING_STREAMER_SAMPLERATE			( 15.0f )
#define _WING_STREAMER_DEPTH				( 15 )
#define _WING_STREAMER_WIDTH				( 0.25f )

#define _STREAMER_BLADE_DEPTH				( 10 )
#define _STREAMER_BLADE_SAMPLERATE			( 60.0f )
#define _STREAMER_BLADE_ALPHA				( 0.5f )
#define _STREAMER_BLADE_WIDTH				( 2.0f )

#define _INVERTED_POSSESS_PITCH				( 0.75f )

#define _JUMPJET_TORSO_MINANGLE				( FMATH_DEG2RAD(-5.0f) )
#define _JUMPJET_TORSO_MAXANGLE				( FMATH_DEG2RAD(5.0f) )
#define _ATTACK_ROT_Z						( FMATH_DEG2RAD( 30.0f ) )
#define _ATTACK_BLEND_RATE					( 4.0f )
#define _MELEE_SPHERE_RADIUS				( 2.0f )

#define _FIRE_BLEND_OUT_RATE				( 10.0f )
#define _FIRE_BLEND_IN_RATE					( 10.0f )
#define _ANIM_FIRE_POINT0					( 0.01f )
#define _ANIM_FIRE_POINT1					( 0.22f )
#define _ANIM_FIRE_POINT2					( 0.06f )
#define _ANIM_FIRE_POINT3					( 0.28f )

#define _DIVE_VELOCITY						( 200.0f )
#define _DIVE_ACCEL							( 250.0f )
#define _JUMPJETS_DIVE_STARTVELOCITY		( 5.0f )
#define _DIVE_DECEL_DIST					( 10.0f )
#define _DIVE_DECEL							( (_DIVE_VELOCITY * _DIVE_VELOCITY) / (_DIVE_DECEL_DIST * 2.0f) )
#define _DIVE_MAX_DISTANCE					( 200.0f )
#define _DIVEHIT_CAMSHAKE_INTENSITY			( 0.25f )
#define _DIVEHIT_CAMSHAKE_DURATION			( 0.75f )

#define _JUMPJET_SOUNDON_RATE				( 5.0f )
#define _JUMPJET_SOUNDOFF_RATE				( 5.0f )
#define _JUMPJET_NORMAL_SOUND				( 0.75f )
#define _RETICLE_Y							( 0.33f )

#define _AIHOVER_VERT_ACCEL_PER_SEC			(50.0f)			//pgm 4/11: when the AI  is making jumptrooper hover, this is the verticle acceleration (up, or down)
static CBotJumperBuilder _BotJumperBuilder;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotJumper
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL 								CBotJumper::m_bSystemInitialized	= FALSE;
u32									CBotJumper::m_nBotClassClientCount	= 0;


CBotJumper::BotInfo_Gen_t			CBotJumper::m_BotInfo_Gen;
CBotJumper::BotInfo_MountAim_t		CBotJumper::m_BotInfo_MountAim;
CBotJumper::BotInfo_Walk_t			CBotJumper::m_BotInfo_Walk;
CBotJumper::BotInfo_Jump_t			CBotJumper::m_BotInfo_Jump;
CBotJumper::BotInfo_Weapon_t		CBotJumper::m_BotInfo_Weapon;
CBotJumper::BotInfo_JumperSounds_t	CBotJumper::m_BotInfo_JumperSounds;
CBotJumper::BotInfo_Jumper_t		CBotJumper::m_BotInfo_Jumper;

CBotPartPool*						CBotJumper::m_pPartPool				= NULL;
CBotJumper*							CBotJumper::m_pCBJumper				= NULL;

CFVec3A								CBotJumper::m_GroinVecY_WS;

CBotAnimStackDef					CBotJumper::m_AnimStackDef;

TracerGroupHandle_t					CBotJumper::m_hTracerGroup			= TRACER_NULLGROUPHANDLE;	
TracerDef_t							CBotJumper::m_TracerDef;	
CFTexInst							CBotJumper::m_TracerTexInst;

CFTexInst							CBotJumper::m_GunStreamerTexInst;
FXStreamerHandle_t*					CBotJumper::m_aGunStreamerPool		= NULL;
CFMtx43A*							CBotJumper::m_aGunStreamerMtxPool	= NULL;
f32*								CBotJumper::m_afGunStreamerTimer	= NULL;
u32									CBotJumper::m_uNoActiveStreamers	= 0;

CFTexInst							CBotJumper::m_BladeStreamerTex;
CFTexInst							CBotJumper::m_StreamerTexGreen;
CFTexInst							CBotJumper::m_StreamerTexRed;


// Static FNs

BOOL CBotJumper::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;
	
	return TRUE;
}


void CBotJumper::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	m_bSystemInitialized = FALSE;
}


BOOL CBotJumper::_InitTracerPool( void ) {
	FASSERT( IsSystemInitialized() );

	// see if we already have a tracer group.  If so, just quit
	if( m_hTracerGroup != TRACER_NULLGROUPHANDLE ) {
		return TRUE;
	}

	m_TracerDef.pUser					= NULL;
	m_TracerDef.pFcnKillCallback		= _TracerKilledCallback;
	m_TracerDef.pFcnBuildSkipList		= _TracerBuildTrackerSkipList;
	m_TracerDef.fWidth_WS				= 0.85f;		
	m_TracerDef.fLength_WS				= _TRACER_LEN;	
	m_TracerDef.fSpeed_WS				= m_BotInfo_Jumper.fTracerSpeed;	
	m_TracerDef.fMaxTailDist_WS			= 1000.0f;	
	m_TracerDef.fBeginDeathUnitFade_WS	= 0.25f;
	m_TracerDef.uFlags					= 0;
	m_TracerDef.ColorRGBA.OpaqueWhite();

	FTexDef_t *pTexDef = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, _GUN_TRACER_TEX ));
	if(pTexDef == NULL) {
		DEVPRINTF( "CBotJumper::_InitTracerPool() : Could not load tracer texture '%s'.\n", _GUN_TRACER_TEX );
		return FALSE;
	} else {
		m_TracerTexInst.SetTexDef( pTexDef );
	}

	m_hTracerGroup = tracer_CreateGroup( &m_TracerTexInst, m_BotInfo_Jumper.uMaxTracers );
	if( m_hTracerGroup == TRACER_NULLGROUPHANDLE ) {
		return FALSE;
	}

	return TRUE;
}

BOOL CBotJumper::ClassHierarchyLoadSharedResources( void ) {
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
	FResFrame_t frame = fres_GetFrame();
	FTexDef_t *pTexDef;

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources():  Error parsing %s\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	// little bit of fixup...
	m_BotInfo_Jumper.fDiveTurnDegPerSecSinSq *= m_BotInfo_Jumper.fDiveTurnDegPerSecSinSq;

	if( !_BuildAnimStackDef() ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources():  Error creating bot anim stack def\n" );
		goto _ExitWithError;
	}

	FASSERT( m_aGunStreamerPool == NULL );

	m_aGunStreamerMtxPool = fnew CFMtx43A[m_BotInfo_Jumper.uMaxTracers];
	if( !m_aGunStreamerMtxPool ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources():  Error allocating gun streamer pool\n" );
		goto _ExitWithError;
	}

	m_afGunStreamerTimer = (f32*)fres_Alloc( sizeof( f32 ) * m_BotInfo_Jumper.uMaxTracers );
	if( !m_afGunStreamerTimer ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources():  Error allocating gun streamer pool\n" );
		goto _ExitWithError;
	}

	m_aGunStreamerPool = (FXStreamerHandle_t*)fres_Alloc( sizeof( FXStreamerHandle_t ) * m_BotInfo_Jumper.uMaxTracers );
	if( !m_aGunStreamerPool ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources():  Error allocating gun streamer pool\n" );
		goto _ExitWithError;
	}


	for( u32 i=0; i<m_BotInfo_Jumper.uMaxTracers; i++ ) {
		m_aGunStreamerMtxPool[i].Identity();
		m_afGunStreamerTimer[i] = -1.0f;
		m_aGunStreamerPool[i]	= FXSTREAMER_INVALID_HANDLE;
	}

	pTexDef = (FTexDef_t *)(fresload_Load( FTEX_RESNAME, _TRACER_STREAMER_TEX ));
	if(pTexDef == NULL) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources() : Could not load gun streamer texture '%s'.\n", _TRACER_STREAMER_TEX );
		goto _ExitWithError;
	} else {
		m_GunStreamerTexInst.SetTexDef( pTexDef );
	}


	// load other streamer textures
	pTexDef = (FTexDef_t *)(fresload_Load( FTEX_RESNAME, _STREAMER_TEX_BLADE ));
	if(pTexDef == NULL) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources() : Could not load streamer texture '%s'.\n", _STREAMER_TEX_BLADE );
		goto _ExitWithError;
	} else {
		m_BladeStreamerTex.SetTexDef( pTexDef );
	}

	pTexDef = (FTexDef_t *)(fresload_Load( FTEX_RESNAME, _STREAMER_TEX_GREEN ));
	if(pTexDef == NULL) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources() : Could not load streamer texture '%s'.\n", _STREAMER_TEX_GREEN );
		goto _ExitWithError;
	} else {
		m_StreamerTexRed.SetTexDef( pTexDef );
	}

	pTexDef = (FTexDef_t *)(fresload_Load( FTEX_RESNAME, _STREAMER_TEX_RED ));
	if(pTexDef == NULL) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources() : Could not load streamer texture '%s'.\n", _STREAMER_TEX_RED );
		goto _ExitWithError;
	} else {
		m_StreamerTexGreen.SetTexDef( pTexDef );
	}

	// initialize tracer pool
	if( !_InitTracerPool() ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources(): Error creating tracer pool\n" );
		goto _ExitWithError;
	}

	// load sound banks...
	if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK) ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK);
	}

	// now load sounds
	if( !fgamedata_ReadFileUsingMap( m_aSoundDataMap, _BOTINFO_FILENAME ) ) {
		goto _ExitWithError;
	}


	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( frame );

	return FALSE;
}


void CBotJumper::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}


	m_AnimStackDef.Destroy();

	// destroy tracers
	if( m_hTracerGroup != TRACER_NULLGROUPHANDLE ) {
		tracer_DestroyGroup( m_hTracerGroup );
		m_hTracerGroup = TRACER_NULLGROUPHANDLE;
	}

	// clear sound data
	fang_MemZero( &m_BotInfo_JumperSounds, sizeof( m_BotInfo_JumperSounds ) );

	// clear gun streamer pool
	if( m_aGunStreamerMtxPool ) {
		fdelete_array( m_aGunStreamerMtxPool );
		m_aGunStreamerMtxPool = NULL;
	}
	m_afGunStreamerTimer	= NULL;
	m_aGunStreamerPool		= NULL;


	
	CBot::ClassHierarchyUnloadSharedResources();
}


CBotJumper::CBotJumper() : CBot() {
	m_pInventory	= NULL;
	m_pWorldMesh	= NULL;
}


CBotJumper::~CBotJumper() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotJumper::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotJumperBuilder *pBuilder = (CBotJumperBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotJumperBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTJUMPER );
	}

#if _DEBUG_START_SLEEPING
	// Set our builder parameters...
	pBuilder->m_bSleepAtStart = TRUE;
#endif

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotJumper::_ClearDataMembers( void ) {
	m_fCollCylinderHeight_WS	= 8.0f;
	m_fCollCylinderRadius_WS	= 4.0f;

	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_MountAim	= &m_BotInfo_MountAim;
	m_pBotInfo_Walk		= &m_BotInfo_Walk;
	m_pBotInfo_Jump		= &m_BotInfo_Jump;
	m_pBotInfo_Weapon	= &m_BotInfo_Weapon;

	m_fMaxFlatSurfaceSpeed_WS	= m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;
	m_fMountPitchMax_WS			= m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS			= m_pBotInfo_MountAim->fMountPitchUpLimit;
	m_pMoveIdentifier			= &m_fJetsParticleIntensity;

	m_fGravity = m_pBotInfo_Gen->fGravity;

	m_anAnimStackIndex[ASI_STAND]				= ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT]			= -1;
	m_anAnimStackIndex[ASI_WALK]				= ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT]			= -1;
	m_anAnimStackIndex[ASI_RUN]					= ANIMTAP_RUN;
	m_anAnimStackIndex[ASI_RUN_PANIC]			= -1;
	m_anAnimStackIndex[ASI_FALL]				= ANIMTAP_JUMP_FLY;
	m_anAnimStackIndex[ASI_RC_TETHERED]			= ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]		= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]			= ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]				= -1; //ANIMTAP_STOOP_SUMMER;
	m_anAnimStackIndex[ASI_STAND_LIMP_LEFT]		= -1; //ANIMTAP_STAND_LIMP_LEFT;
	m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT]	= -1; //ANIMTAP_STAND_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_LIMP_LEFT]			= -1; //ANIMTAP_LIMP_LEFT;
	m_anAnimStackIndex[ASI_LIMP_RIGHT]			= -1; //ANIMTAP_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_HOP_LEFT]			= ANIMTAP_HOP_LEFT;
	m_anAnimStackIndex[ASI_HOP_RIGHT]			= ANIMTAP_HOP_RIGHT;
	m_anAnimStackIndex[ASI_STARTLE]				= -1;
	m_anAnimStackIndex[ASI_ROLL_LEFT]			= -1;
	m_anAnimStackIndex[ASI_ROLL_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_DOZE_LOOP]			= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_NAPJERK]				= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_WAKE]				= -1; //ANIMTAP_WAKE;

	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock	= m_anEnableBoneNameIndexTableForSummer_Normal;

	m_MeleeState				= MELEE_STATE_NONE;
	m_fAttackAngleCurrentL		= 0.0f;
	m_fAttackAngleCurrentR		= 0.0f;
	m_fAttackAngleDesiredL		= 0.0f;
	m_fAttackAngleDesiredR		= 0.0f;

	m_fFireTimer				= 0.0f;

	m_JumpJetState				= JUMPJETS_OFF;
	m_fJumpJetDesiredVelocity	= m_BotInfo_Jumper.fTerminalVelocity;
	m_fJetsParticleIntensity	= 0.0f;
	m_fJumpJetTimer				= 0.0f;
	m_pJumpJetAudioEmitter		= NULL;
	m_pJumpDiveAudioEmitter		= NULL;
	m_fJumpJetSoundVolume		= 0.0f;

	m_TorsoAnimFrame.BuildQuat( CFVec3A::m_UnitAxisX, 0.0f, CFVec3A::m_Null );

	m_hStreamerGreen		= FXSTREAMER_INVALID_HANDLE;
	m_hStreamerRed			= FXSTREAMER_INVALID_HANDLE;
	m_hStreamerBladeLeft	= FXSTREAMER_INVALID_HANDLE;
	m_hStreamerBladeRight	= FXSTREAMER_INVALID_HANDLE;


	m_hJetFireParticleLeft		= FPARTICLE_INVALID_HANDLE;
	m_hJetSmokeParticleLeft		= FPARTICLE_INVALID_HANDLE;
	m_hJetLaunchParticleLeft	= FPARTICLE_INVALID_HANDLE;
	m_hJetFireParticleRight		= FPARTICLE_INVALID_HANDLE;
	m_hJetSmokeParticleRight	= FPARTICLE_INVALID_HANDLE;
	m_hJetLaunchParticleRight	= FPARTICLE_INVALID_HANDLE;

	m_bAllowJumpTranslation		= FALSE;

	//m_vLastMeleePositionLeft.Zero();
	//m_vLastMeleePositionRight.Zero();


	fforce_NullHandle( &m_hForce );
}


BOOL CBotJumper::ClassHierarchyBuild( void ) {
	FMesh_t		*pMesh;
	FMeshInit_t	meshInit;
	FResFrame_t	frame;
	s32 nBoneIndex;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	frame = fres_GetFrame();

	// get pointer to leaf class's builder object...
	CBotJumperBuilder *pBuilder = (CBotJumperBuilder*)GetLeafClassBuilder();

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
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Could not find mesh '%s'\n", _MESH_NAME );
		goto _ExitWithError;
	}

	// create worldmesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to allocate CFWorldMesh\n" );
		goto _ExitWithError;
	}

	// init worldmesh...
	meshInit.pMesh		= pMesh;
	meshInit.fCullDist	= FMATH_MAX_FLOAT;
	meshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	meshInit.nFlags		= 0;
	m_pWorldMesh->Init( &meshInit );

	m_pWorldMesh->m_nUser	= MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser	= this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();


	// build animation stack...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to create animation stack\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	// set up bone callbacks...
	m_nBoneIdxGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GROIN] );
	if( m_nBoneIdxGroin < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_GROIN] );
		goto _ExitWithError;
	}

	m_nBoneIdxTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	if( m_nBoneIdxTorso < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_TORSO] );
		goto _ExitWithError;
	}

	m_nBoneIdxHead = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_HEAD] );
	if( m_nBoneIdxHead < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_HEAD] );
		goto _ExitWithError;
	}

	m_nBoneIdxMeleeLeft = m_pWorldMesh->FindBone( _MELEE_BONE_LEFT );
	if( m_nBoneIdxMeleeLeft < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n", _MELEE_BONE_LEFT );
		goto _ExitWithError;
	}

	m_nBoneIdxMeleeRight = m_pWorldMesh->FindBone( _MELEE_BONE_RIGHT );
	if( m_nBoneIdxMeleeRight < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n", _MELEE_BONE_RIGHT );
		goto _ExitWithError;
	}

	// pri fire bones...
	m_anBoneIdxPriFire[0] = m_pWorldMesh->FindBone(  m_apszBoneNameTable[BONE_GUN_1_L] );
	if( m_anBoneIdxPriFire[0] < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n",  m_apszBoneNameTable[BONE_GUN_1_L] );
		goto _ExitWithError;
	}

	m_anBoneIdxPriFire[1] = m_pWorldMesh->FindBone(  m_apszBoneNameTable[BONE_GUN_1_R] );
	if( m_anBoneIdxPriFire[1] < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n",  m_apszBoneNameTable[BONE_GUN_1_R] );
		goto _ExitWithError;
	}

	m_anBoneIdxPriFire[2] = m_pWorldMesh->FindBone(  m_apszBoneNameTable[BONE_GUN_2_L] );
	if( m_anBoneIdxPriFire[2] < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n",  m_apszBoneNameTable[BONE_GUN_2_L] );
		goto _ExitWithError;
	}

	m_anBoneIdxPriFire[3] = m_pWorldMesh->FindBone(  m_apszBoneNameTable[BONE_GUN_2_R] );
	if( m_anBoneIdxPriFire[3] < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n",  m_apszBoneNameTable[BONE_GUN_2_R] );
		goto _ExitWithError;
	}

	// get bone indices for arm aiming...
	m_nBoneIdxArmLower_L = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_LOWER_L] );
	if( m_nBoneIdxArmLower_L < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_ARM_LOWER_L] );
		goto _ExitWithError;
	}

	m_nBoneIdxArmLower_R = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_LOWER_R] );
	if( m_nBoneIdxArmLower_R < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Unable to locate bone %s\n", m_apszBoneNameTable[BONE_ARM_LOWER_R] );
		goto _ExitWithError;
	}

	if( !m_AnimManFrameAim.Create( m_AnimStackDef.m_nBoneCount, m_AnimStackDef.m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyCreate(): Could not create aim animation summer.\n" );
		goto _ExitWithError;
	}
	AttachAnim( ANIMTAP_AIM_SUMMER, &m_AnimManFrameAim );
	m_AnimManFrameAim.Reset();
	UpdateBoneMask( ANIMTAP_AIM_SUMMER, m_aBoneEnableIndices_AimSummer, TRUE );


	// set up animation summer...
	m_pAnimManFrameAttack = fnew CFAnimManFrame;
	if( !m_pAnimManFrameAttack ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Out of memory creating CFAnimManFrame\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimManFrameAttack->Create( SUMMERBONE_COUNT, m_apszSummerBoneNames ) ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Error creating CFAnimManFrame\n" );
		goto _ExitWithError;
	}

	m_pAnimManFrameAttack->Reset();

	AttachAnim( ANIMTAP_ATTACK_SUMMER, m_pAnimManFrameAttack );

	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();
	m_Anim.m_pAnimCombiner->SetBoneCallback( _AnimBoneCallback );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GROIN] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_TORSO] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_HEAD] );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_LOWER_L] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_LOWER_R] );

	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAUNCH );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_FLY_JETS );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_FLY );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAND_UPPER );
	EnableControlSmoothing(	ANIMCONTROL_JUMP_LAND_LOWER );

	SetMaxHealth();

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, _RETICLE_Y );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetType( CReticle::TYPE_DROID_STANDARD );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.EnableDraw( TRUE );

	}

	// initialize part pool
	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		FASSERT_NOW;
		goto _ExitWithError;
	}

	// Make invincible limbs...
	MakeLimbInvincible( LIMB_CODE_HEAD, LIMB_TYPE_HEAD, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_LEFT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_RIGHT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_WINGS, LIMB_TYPE_LEFT_WING, pBuilder );
	MakeLimbInvincible( LIMB_CODE_WINGS, LIMB_TYPE_RIGHT_WING, pBuilder );
	MakeLimbInvincible( LIMB_CODE_MISC, LIMB_TYPE_CHEST_TOP, pBuilder );
	MakeLimbInvincible( LIMB_CODE_MISC, LIMB_TYPE_CHEST_MIDDLE, pBuilder );
	MakeLimbInvincible( LIMB_CODE_MISC, LIMB_TYPE_CHEST_BOTTOM, pBuilder );

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_HEAD ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_HEAD ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	// initialize mtx palette...
	AtRestMatrixPalette();

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	// Set up data port stuff...
	DataPort_SetupTetherShockInfo();


	// grab bones for particle emitters
	m_nBoneIdxJetFireLeft	= m_pWorldMesh->FindBone( _JET_BONE_LEFT );
	m_nBoneIdxJetFireRight	= m_pWorldMesh->FindBone( _JET_BONE_RIGHT );
	m_nBoneIdxJetSmokeLeft	= m_pWorldMesh->FindBone( _SMOKE_BONE_LEFT );
	m_nBoneIdxJetSmokeRight	= m_pWorldMesh->FindBone( _SMOKE_BONE_RIGHT );

	if( (m_nBoneIdxJetSmokeRight < 0) || (m_nBoneIdxJetSmokeLeft < 0) || (m_nBoneIdxJetFireRight < 0) || (m_nBoneIdxJetFireLeft < 0) ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyBuild(): Could not locate particle bones in mesh.\n" );
		goto _ExitWithError;
	}

	// init melee

	MeleeInit( &m_MeleeDataLeft, _MELEE_SPHERE_RADIUS, m_BotInfo_Jumper.apDamageProfileMelee[0], &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeLeft]->m_vPos) );
	MeleeInit( &m_MeleeDataRight, _MELEE_SPHERE_RADIUS, m_BotInfo_Jumper.apDamageProfileMelee[0], &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeRight]->m_vPos) );
	
	m_fMaxVerticalSpeed_WS = FMATH_FABS( m_BotInfo_Jumper.fTerminalVelocity );
	m_fMaxFlatSurfaceSpeed_WS = m_BotInfo_Walk.fMaxXlatVelocity;
	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;

}


void CBotJumper::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	if( m_pWorldMesh ) {
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	CBot::ClassHierarchyDestroy();
}



void CBotJumper::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	//fparticle_EnableEmission( m_hJetFireParticleLeft,		TRUE );
	//fparticle_EnableEmission( m_hJetSmokeParticleLeft,		TRUE );
	//fparticle_EnableEmission( m_hJetLaunchParticleLeft, 	FALSE );
	//fparticle_EnableEmission( m_hJetFireParticleRight,		TRUE );
	//fparticle_EnableEmission( m_hJetSmokeParticleRight,		TRUE );
	//fparticle_EnableEmission( m_hJetLaunchParticleRight, 	FALSE );

	//_StartAllEmitters();

	m_JumpJetState = JUMPJETS_OFF;
	m_fJumpJetTimer = 0.0f;			//pgm 4/11: better reset this as to not have lingering jumpjets after a restore, or bot-reuse or something.

	m_pWorldMesh->UpdateTracker();

}


void CBotJumper::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	//fparticle_EnableEmission( m_hJetFireParticleLeft,		FALSE );
	//fparticle_EnableEmission( m_hJetSmokeParticleLeft,		FALSE );
	//fparticle_EnableEmission( m_hJetLaunchParticleLeft, 	FALSE );
	//fparticle_EnableEmission( m_hJetFireParticleRight,		FALSE );
	//fparticle_EnableEmission( m_hJetSmokeParticleRight,		FALSE );
	//fparticle_EnableEmission( m_hJetLaunchParticleRight, 	FALSE );

	_KillAllEmitters();

	if( m_hStreamerBladeLeft != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerBladeLeft, FALSE );
		m_hStreamerBladeLeft = FXSTREAMER_INVALID_HANDLE;
	}

	if( m_hStreamerBladeRight != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerBladeRight, FALSE );
		m_hStreamerBladeRight = FXSTREAMER_INVALID_HANDLE;
	}


	if( m_hStreamerGreen != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerGreen, FALSE );
		m_hStreamerGreen = FXSTREAMER_INVALID_HANDLE;
	}

	if( m_hStreamerRed != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerRed, FALSE );
		m_hStreamerRed = FXSTREAMER_INVALID_HANDLE;
	}

	if( m_pJumpJetAudioEmitter ) {
		m_pJumpJetAudioEmitter->Destroy();
		m_pJumpJetAudioEmitter = NULL;
	}

	if( m_pJumpDiveAudioEmitter ) {
		m_pJumpDiveAudioEmitter->Destroy();
		m_pJumpDiveAudioEmitter = NULL;
	}


	fforce_Kill( &m_hForce );

	m_pWorldMesh->RemoveFromWorld();

	CBot::ClassHierarchyRemoveFromWorld();
}


BOOL CBotJumper::ClassHierarchyBuilt( void ) {
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


CEntityBuilder* CBotJumper::GetLeafClassBuilder( void ) {
	return &_BotJumperBuilder;
}


// Used by the bottalk system
void CBotJumper::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
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


void CBotJumper::ClassHierarchyWork( void ) {
	FASSERT( IsSystemInitialized() );

	CFVec3A vTmp;

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	PROTRACK_BEGINBLOCK("Jumper");

	Power_Work();
	DataPort_Work();

	ParseControls();

	// do some special parsing because there's so much to do, and so few buttons...
	if( m_bControls_Human ) {
		// human player...
		m_fControls_FlyUp = m_fControls_Fire2;
		//m_bControls_Melee = !!((CHumanControl*)Controls())->m_nPadFlagsSelect1; 
	} else {
		m_bControls_Melee = m_fControls_Fire2 > 0.1f;
	}

	if (IsPlayerBot())
	{
		m_pWorldMesh->m_AmbientRGB.fRed	=  0.0f;
		m_pWorldMesh->m_AmbientRGB.fGreen=  0.0f;
		m_pWorldMesh->m_AmbientRGB.fBlue=  0.0f;
	}
	else
	{
		m_pWorldMesh->m_AmbientRGB.fRed	=  0.5f;
		m_pWorldMesh->m_AmbientRGB.fGreen=  0.5f;
		m_pWorldMesh->m_AmbientRGB.fBlue=  0.5f;
	}

	// save a copy of previous frame info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// add in any velocity impulse...
	BOOL bImpulseApplied = HandleVelocityImpulses();
	
	// update position...
	vTmp.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( vTmp );

	if( JumpJetsAreOn() ) {
		// need to set this every frame
		m_nBotFlags |= BOTFLAG_HIPSLACK_OVERRIDE;
	}

	// pitch & yaw...
	HandlePitchMovement();
	HandleYawMovement();

	HandleHeadLook();

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
	
	_JumpJetWork();
	_SoundWork();

	if( IsSleeping() ) {
		if( m_nBotFlags & BOTFLAG_IMMOBILIZE_PENDING ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
		}
	}

	if( IsDiving() ) {
		_HandleDiveTranslation();
		if( m_JumpJetState == JUMPJETS_DIVE_ENDING ) {
			HandleGroundAnimations();
		}
	} else {
		// Move and animate our bot...
		switch( m_nState ) {
			case STATE_GROUND:
				PROTRACK_BEGINBLOCK("GroundXlat");
					HandleGroundTranslation();
					_HandleJumping();
					HandleHopRollStartleGeneric();
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
					_HandleAirTranslation();
				PROTRACK_ENDBLOCK();//"AirXlat");

				PROTRACK_BEGINBLOCK("AirAnim");
					if( HandleAirAnimations() ) {
						_EnterFlyMode();
					}
				PROTRACK_ENDBLOCK();//"AirAnim");

				if( m_bControls_Jump && 
					(m_nJumpState == BOTJUMPSTATE_LAUNCH) ) {
					_StartDoubleJump();
				}
				break;
		}
	}


	PROTRACK_BEGINBLOCK("CommonAnim");
		_HandleJumpAnimations();
		HandleHipsAnimation();
		_HandleAimAnimations();
	PROTRACK_ENDBLOCK();//"CommonAnim");

	PROTRACK_BEGINBLOCK("Weapon");
		HandleTargeting();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_UpdateMatrices();

	_HandleSleeping();
	_MeleeWork();
	_GunWork();

	if( IsDeadOrDying() ) {
		DeathWork();
		if( IsDiving() ) {
			_EndDive( FALSE );
		}
	}

	PROTRACK_ENDBLOCK(); //"Jumper"
}


void CBotJumper::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

const CFVec3A* CBotJumper::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}



///////////////////
// Jump trooper FNs


void CBotJumper::_HandleJumping( void ) {
	if( !m_bControlsBot_JumpVec ) {
		if( m_bControlsBot_Jump2 ) {
			_StartSingleJump();
			_StartDoubleJump();
		} else if( m_bControls_Jump ) {
			_StartSingleJump();
		}
	} else {
		// Velocity jump specified...
		_StartSingleJump( &m_ControlsBot_JumpVelocity_WS );
	}

	if( m_nPrevState == STATE_AIR ) {
		if( (m_nJumpState != BOTJUMPSTATE_NONE)/* && 
			(m_JumpJetState != JUMPJETS_DIVING) &&
			(m_JumpJetState != JUMPJETS_RECOVERING)*/ ) {
			_JumpLanded();
		}
	}
}


void CBotJumper::_EnterFlyMode( void ) {
	m_bPlayLandAnim			= FALSE;
	m_bPlayLandAnimLower	= FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );

	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


void CBotJumper::_JumpLanded( void ) {
	MakeFootImpactDust( TRUE, TRUE, m_pBotInfo_Gen->fUnitDustKickup, &m_FeetToGroundCollImpact.UnitFaceNormal );

	//f32 fJumpCtlValue = GetControlValue( ANIMCONTROL_JUMP_LAND_UPPER );
	//if( fJumpCtlValue < 1.0f && fJumpCtlValue > 0.0f ) {
	//	DEVPRINTF( "jcv = %0.2f\n", fJumpCtlValue );
	//}

	m_nJumpState			= BOTJUMPSTATE_NONE;
	ClearJumping();
	m_fFlipPitch			= 0.0f;
	m_bPlayLandAnim 		= TRUE;
	m_fGravityMultiplier	= 1.0f;
	KillJumpJets();

	ZeroTime( ANIMTAP_JUMP_LAND_UPPER );

	m_fMaxLandUnitBlend = GetControlValue( ANIMCONTROL_JUMP_FLY );
	if( m_fMaxLandUnitBlend == 0.0f ) {
		m_fMaxLandUnitBlend = 1.0f;
	}

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, m_fMaxLandUnitBlend );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	if ( m_nPossessionPlayerIndex >= 0 )	{
		// Don't play scuff sounds when landing from a jump
		if( m_pBotInfo_Sound && m_nSurfaceTypeOn != SURFACE_TYPE_NONE) {
			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[m_nSurfaceTypeOn][ fmath_RandomRange( 0, ( BOTSOUND_SCUFF_SOUND_START - 1 ) ) ],
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
}


void CBotJumper::_HandleJumpAnimations( void ) {
	f32 fTemp, fUnitTime;

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
					if( !DeltaTime( ANIMTAP_JUMP_LAUNCH, FLoop_fPreviousLoopSecs * 2.0f, TRUE ) ) {
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


void CBotJumper::_StartSingleJump( const CFVec3A *pJumpVelocity_WS ) {
	if( pJumpVelocity_WS == NULL ) {
		m_fJumpJetDesiredVelocity = 0.0f;				  //pgm put this here so that it doesn't interfere with jumpvel
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		m_Velocity_MS.z += 0.0f; //m_BotInfo_Jumper.fHorizVelocityJump1 * m_fJumpMultiplier;		//ME:  a little confused about what just happened here
		MS2WS( m_Velocity_WS, m_Velocity_MS );
	} else {
		m_Velocity_WS.x = pJumpVelocity_WS->x;
		m_Velocity_WS.y = pJumpVelocity_WS->y;
		m_Velocity_WS.z = pJumpVelocity_WS->z;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
	}
		
	VelocityHasChanged();

	m_bAllowJumpTranslation = FALSE;
	m_bPlayLandAnim			= FALSE;
	m_bPlayLandAnimLower	= FALSE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();
	SetDoubleJumpTimer( m_pBotInfo_Gen->fDoubleJumpTime );
}


void CBotJumper::_StartDoubleJump( void ) {
	if( !HaveTimeToDoubleJump() ) {
		return;
	}

	m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump2;
	MS2WS( m_Velocity_WS, m_Velocity_MS );
	VelocityHasChanged();
}


void CBotJumper::_HandleAirTranslation( void ) {

	// If we're moving in the air, m_MovingSurfaceUnitVec_WS is simply our unit velocity vector.
	// Otherwise, we'll just set it to m_MountUnitFrontXZ_WS...
	m_MovingSurfaceUnitVec_WS = (m_fSpeed_WS != 0.0f) ? m_UnitVelocity_WS : m_MountUnitFrontXZ_WS;

	// No surface normal...
	m_SurfaceUnitNorm_WS.Zero();

	// Since we're in the air, reset walk mode...
	m_nTractionMode = TRACTIONMODE_NORMAL;

	m_fSecsSinceLastFootCollision += FLoop_fPreviousLoopSecs;
	
	//pgm 4/11: FYI: m_fjumpJetDesireVelocity is not set when jumper is under AI control.
	if (m_nPossessionPlayerIndex == -1)
	{
		if (m_nControlsBot_Buttons & CBotControl::BUTTONFLAG_JUMP2)
		{
			//fly mode, jumper should behave based on the m_fControls_FlyUp.. Note! there is no gravity
			if (m_fControls_FlyUp > 0.001f)
			{
				m_Velocity_MS.y += m_fControls_FlyUp * m_BotInfo_Jumper.fJumpLaunchVel * FLoop_fPreviousLoopSecs;
			}
			else if (m_fControls_FlyUp < -0.001f)
			{
				m_Velocity_MS.y += m_fControls_FlyUp * m_BotInfo_Jumper.fJumpLaunchVel * FLoop_fPreviousLoopSecs;
			}
			else
			{
				f32 fVelDiff = 0.0f - m_Velocity_MS.y;
				if (fVelDiff > 0.0f)
				{
					FMATH_CLAMPMAX( fVelDiff, m_BotInfo_Jumper.fJumpLaunchAccel * FLoop_fPreviousLoopSecs );
					m_Velocity_MS.y += fVelDiff;
					FMATH_CLAMPMAX( m_Velocity_MS.y, 0.0f );
				}
				else
				{
					FMATH_CLAMPMIN( fVelDiff, -m_BotInfo_Jumper.fJumpLaunchAccel * FLoop_fPreviousLoopSecs );
					m_Velocity_MS.y += fVelDiff;
					FMATH_CLAMPMIN( m_Velocity_MS.y, 0.0f );
				}
			}
		}
		else
		{
			//must be in the air due to having fallen, or jumped, so do gravity.
			m_Velocity_MS.y += m_pBotInfo_Gen->fGravity * m_fGravityMultiplier * FLoop_fPreviousLoopSecs;
		}
	}
	else
	{
		m_Velocity_MS.y += m_pBotInfo_Gen->fGravity * m_fGravityMultiplier * FLoop_fPreviousLoopSecs;

		if( m_fJumpJetDesiredVelocity > m_Velocity_MS.y ) {
			f32 fVelDiff = m_fJumpJetDesiredVelocity - m_Velocity_MS.y;
			FMATH_CLAMPMAX( fVelDiff, m_BotInfo_Jumper.fJumpLaunchAccel * FLoop_fPreviousLoopSecs );
			m_Velocity_MS.y += fVelDiff;
			FMATH_CLAMPMAX( m_Velocity_MS.y, m_fJumpJetDesiredVelocity );
		}
	}
	m_Velocity_WS.y = m_Velocity_MS.y;

	
	//
	// pgm 4/11: Other influences on Vertical velocity
	//
	//if (m_nPossessionPlayerIndex >= 0)
	if( m_bAllowJumpTranslation )
	{
		CFVec3A vDesVel_MS;
		CFVec3A vVelXZ_MS;
		CFVec3A vAccel_MS;
	    
		vVelXZ_MS	= m_Velocity_MS;
		vVelXZ_MS.y = 0.0f;

		vDesVel_MS.Mul( m_XlatStickNormVecXZ_MS, m_pBotInfo_Walk->fAirControlNudgeSpeed + m_pBotInfo_Walk->fMaxXlatVelocity );
		vAccel_MS.Sub( vDesVel_MS, vVelXZ_MS );

		f32 fAccelMag = vAccel_MS.SafeUnitAndMag( vAccel_MS );
		if( fAccelMag > 0.01f ) {
			FMATH_CLAMPMAX( fAccelMag, m_BotInfo_Jumper.fMaxAirAccel );
			if( vVelXZ_MS.Dot( vAccel_MS ) < 0.0f ) {
				fAccelMag *= m_BotInfo_Jumper.fDecelMult;
			}

			vAccel_MS.Mul( fAccelMag * FLoop_fPreviousLoopSecs );
			m_Velocity_MS.Add( vAccel_MS );
		}
	}
	else
	{
		if( m_XlatStickNormVecXZ_MS.MagSq() > 0.1f || 
			(m_nPossessionPlayerIndex == -1 && (m_nControlsBot_Buttons & CBotControl::BUTTONFLAG_JUMP2)))
		{
			m_bAllowJumpTranslation = TRUE;
		}
	}

	MS2WS( m_Velocity_WS, m_Velocity_MS );

	if( _CheckForCeilingCollision() ) {
		m_Velocity_WS.y = m_Velocity_MS.y = 0.0f;
	}

	VelocityHasChanged();
}   


void CBotJumper::KillJumpJets( void ) {
	if( m_JumpJetState == JUMPJETS_DIVE_ENDING ) {
		m_fJumpJetTimer = m_BotInfo_Jumper.fJumpDiveInterval;
		m_JumpJetState = JUMPJETS_OFF;
	} else if( m_JumpJetState != JUMPJETS_OFF ) {
		m_fJumpJetTimer = m_BotInfo_Jumper.fJumpMinInterval;
		m_JumpJetState = JUMPJETS_OFF;
	}

	if( m_hStreamerGreen != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerGreen, FALSE );
		m_hStreamerGreen = FXSTREAMER_INVALID_HANDLE;
	}

	if( m_hStreamerRed != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerRed, FALSE );
		m_hStreamerRed = FXSTREAMER_INVALID_HANDLE;
	}
}


void CBotJumper::_JumpJetWork( void ) {
	f32 fDesiredPartIntensity;
	f32 fPartIntensity = 0.0f;
	f32 fVal;
	EnableControlSmoothing( ANIMCONTROL_DIVE );

	if( m_uBotDeathFlags & BOTDEATHFLAG_WALKING_DEAD ) {
		_KillAllEmitters();
		m_JumpJetState = JUMPJETS_OFF;
	}

	// check the emitters, start if necessary
	_CheckAndStartEmitter( &m_hJetSmokeParticleLeft, &m_BotInfo_Jumper.hParticleDefJumpJetSmoke, m_nBoneIdxJetSmokeLeft );
	_CheckAndStartEmitter( &m_hJetSmokeParticleRight, &m_BotInfo_Jumper.hParticleDefJumpJetSmoke, m_nBoneIdxJetSmokeRight );
	
	switch( m_JumpJetState ) {
		case JUMPJETS_OFF:
			fDesiredPartIntensity		= 0.0f;
			m_fJumpJetDesiredVelocity	= m_BotInfo_Jumper.fTerminalVelocity;
			m_fMaxFlatSurfaceSpeed_WS	= m_BotInfo_Walk.fMaxXlatVelocity;

			// blend out of fly jets animation (wings up)
			fVal = GetControlValue( ANIMCONTROL_JUMP_FLY_JETS );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs * _JUMPJETS_ANIM_ON_RATE;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_JUMP_FLY_JETS, fVal );
				DeltaTime( ANIMTAP_JUMP_FLY_JETS );
			}

			// blend out of dive animations
			fVal = GetControlValue( ANIMCONTROL_DIVE );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_DIVE, fVal );
			}
			fVal = GetControlValue( ANIMCONTROL_DIVE_RECOVER_UPPER );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_DIVE_RECOVER_UPPER, fVal );
			}
			fVal = GetControlValue( ANIMCONTROL_DIVE_RECOVER_LOWER );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_DIVE_RECOVER_LOWER, fVal );
			}
			
			// count down the timer
			if( m_fJumpJetTimer > 0.0f ) {
				m_fJumpJetTimer -= FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MIN0( m_fJumpJetTimer );
			}

			if( m_BotInfo_Jumper.fJumpDiveInterval > 0.0f ) {
				m_fUnitJumpJetRecharge = fmath_Div( m_fJumpJetTimer - 0.5f, m_BotInfo_Jumper.fJumpDiveInterval );
			}
			FMATH_CLAMP_UNIT_FLOAT( m_fUnitJumpJetRecharge );

			// shut down sounds if they're still playing
			if( m_pJumpJetAudioEmitter || m_pJumpDiveAudioEmitter ) {
				m_fJumpJetSoundVolume -= FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDOFF_RATE;
				FMATH_CLAMP_MIN0( m_fJumpJetSoundVolume );
				if( m_pJumpJetAudioEmitter ) {
					m_pJumpJetAudioEmitter->SetVolume( m_fJumpJetSoundVolume );
					if( m_fJumpJetSoundVolume == 0.0f ) {
						m_pJumpJetAudioEmitter->Destroy();
						m_pJumpJetAudioEmitter = NULL;
					}
				}
				if( m_pJumpDiveAudioEmitter ) {
					m_pJumpDiveAudioEmitter->SetVolume( m_fJumpJetSoundVolume );
					if( m_fJumpJetSoundVolume == 0.0f ) {
						m_pJumpDiveAudioEmitter->Destroy();
						m_pJumpDiveAudioEmitter = NULL;
					}
				}
			}

			// check for launch
			if( !IsBackBroken() && (m_fControls_FlyUp > 0.75f) ) {
				// see whether the timer has elapsed.  If so go ahead and fire.  Otherwise, sputter a little
				if( m_fJumpJetTimer == 0.0f ) {	
					m_fJumpJetTimer = m_BotInfo_Jumper.fJumpLaunchTime;

					//pgm 4/11: Jump jets for AI controlled jumpers is purely graphical.  Don't set vbls that control physics behavior
					if( IsPlayerBot() ) {
						m_fJumpJetDesiredVelocity	= m_BotInfo_Jumper.fJumpLaunchVel;
						m_JumpJetState = JUMPJETS_LAUNCH;
					} else {
						m_JumpJetState = JUMPJETS_AIFLY;
					}
					m_nState = STATE_AIR;

					f32 fStreamerAlpha = m_nPossessionPlayerIndex < 0 ? m_BotInfo_Jumper.fWingStreamerAlphaAI : m_BotInfo_Jumper.fWingStreamerAlphaPlayer;

					if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_WING ) != CBotPartMgr::LIMB_STATE_REMOVED ) {
						m_hStreamerGreen = CFXStreamerEmitter::SpawnFromMeshBones( _WING_STREAMER_DEPTH, m_pWorldMesh, m_apszStreamerBonesGreen, fStreamerAlpha, 
																					&m_StreamerTexGreen, _WING_STREAMER_WIDTH, _WING_STREAMER_SAMPLERATE, CFXStreamerEmitter::USE_UP_AXIS );
					}

					if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_WING ) != CBotPartMgr::LIMB_STATE_REMOVED ) {
						m_hStreamerRed = CFXStreamerEmitter::SpawnFromMeshBones( _WING_STREAMER_DEPTH, m_pWorldMesh, m_apszStreamerBonesRed, fStreamerAlpha, 
																					&m_StreamerTexRed, _WING_STREAMER_WIDTH, _WING_STREAMER_SAMPLERATE, CFXStreamerEmitter::USE_UP_AXIS );
					}

					m_fJumpJetSoundVolume = 0.0f;
					// play the launch sound
					if( m_nPossessionPlayerIndex < 0 ) {
						fsndfx_Play3D( m_BotInfo_JumperSounds.hJetsLaunch, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vPos), m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D );
						if( m_pJumpJetAudioEmitter ) {
							m_pJumpJetAudioEmitter->SetVolume( 0.0f );
						} else {
							m_pJumpJetAudioEmitter = FSNDFX_ALLOCNPLAY3D( m_BotInfo_JumperSounds.hJetsLoop, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vPos), m_BotInfo_Jumper.fSoundRadius3D, 1.0f, 0.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
						}
					} else {
						fsndfx_Play2D( m_BotInfo_JumperSounds.hJetsLaunch, m_BotInfo_Jumper.fSoundVolume3D );
						if( m_pJumpJetAudioEmitter ) {
							m_pJumpJetAudioEmitter->SetVolume( 0.0f );
						} else {
							m_pJumpJetAudioEmitter = FSNDFX_ALLOCNPLAY2D( m_BotInfo_JumperSounds.hJetsLoop, 0.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE  );
						}
					}
				} else {
					_JumpJetsSputter();
				}
			}
			break;

		case JUMPJETS_AIFLY:

			if( (m_fControls_FlyUp < 0.0f) && (m_nState == STATE_GROUND) ) {
				KillJumpJets();
			}

			m_fMaxFlatSurfaceSpeed_WS = m_BotInfo_Walk.fMaxXlatVelocity + m_BotInfo_Walk.fAirControlNudgeSpeed;

			m_nState = STATE_AIR;
			fVal = GetControlValue( ANIMCONTROL_JUMP_FLY_JETS );
			if( fVal < 1.0f ) {
				fVal += FLoop_fPreviousLoopSecs * _JUMPJETS_ANIM_ON_RATE;
				FMATH_CLAMP_MAX1( fVal );
				SetControlValue( ANIMCONTROL_JUMP_FLY_JETS, fVal );
			}
			DeltaTime( ANIMTAP_JUMP_FLY_JETS );

			if( m_fControls_FlyUp >= 0.0f ) {
				m_fJumpJetDesiredVelocity = FMATH_FPOT( m_fControls_FlyUp, 0.0f, m_BotInfo_Jumper.fJumpLaunchVel );
			} else {
				m_fJumpJetDesiredVelocity = FMATH_FPOT( -m_fControls_FlyUp, 0.0f, m_BotInfo_Jumper.fTerminalVelocity );
			}

			fDesiredPartIntensity = FMATH_FPOT( (m_fControls_FlyUp + 1.0f) * 0.5f, 0.0f, 0.9f );
			FMATH_IS_UNIT_FLOAT( fDesiredPartIntensity );
			DeltaTime( ANIMTAP_JUMP_FLY_JETS );

			if( m_pJumpJetAudioEmitter ) {
				if( m_fJumpJetSoundVolume > fDesiredPartIntensity ) {
					m_fJumpJetSoundVolume -= FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDOFF_RATE;
					FMATH_CLAMPMIN( m_fJumpJetSoundVolume, fDesiredPartIntensity );
				} else if( m_fJumpJetSoundVolume < fDesiredPartIntensity ) {
					m_fJumpJetSoundVolume += FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDON_RATE;
					FMATH_CLAMPMAX( m_fJumpJetSoundVolume, fDesiredPartIntensity );
				}
				m_pJumpJetAudioEmitter->SetVolume( m_fJumpJetSoundVolume * _JUMPJET_NORMAL_SOUND );
			}

			// if somehow you become a player, drop into descend mode
			if( IsPlayerBot() ) {
				FMATH_CLAMP_UNIT_FLOAT( m_fControls_FlyUp );		// have to do this because this ranges from 0 - 1 for a human controlled jumper
				m_JumpJetState = JUMPJETS_DESCEND;
			}

			break;

		case JUMPJETS_LAUNCH:
			fDesiredPartIntensity		= 1.0f;
			m_nState					= STATE_AIR;
			
			// advance the jet animation...
			fVal = GetControlValue( ANIMCONTROL_JUMP_FLY_JETS );
			if( fVal < 1.0f ) {
				fVal += FLoop_fPreviousLoopSecs * _JUMPJETS_ANIM_ON_RATE;
				FMATH_CLAMP_MAX1( fVal );
				SetControlValue( ANIMCONTROL_JUMP_FLY_JETS, fVal );
			}
			DeltaTime( ANIMTAP_JUMP_FLY_JETS );

			m_fJumpJetTimer -= FLoop_fPreviousLoopSecs;
			if( m_fJumpJetTimer <= 0.0f ) {
				m_JumpJetState				= JUMPJETS_DESCEND;
				m_fJumpJetTimer				= 0.0f; 
			}

			// bring jets sound up
			if( m_pJumpJetAudioEmitter && (m_fJumpJetSoundVolume < 1.0f) ) {
				m_fJumpJetSoundVolume += FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDON_RATE;
				FMATH_CLAMP_MAX1( m_fJumpJetSoundVolume );
				m_pJumpJetAudioEmitter->SetVolume( m_fJumpJetSoundVolume * _JUMPJET_NORMAL_SOUND );
			}
			break;

		case JUMPJETS_HOVER:
			FASSERT_NOW;
			break;

		case JUMPJETS_DESCEND:
			if( m_nPossessionPlayerIndex < 0 ) {
				//pgm: 4/11 if it's an AI jumper, hte jumpjets are purely graphics. no physics
//				m_fJumpJetDesiredVelocity = FMATH_FPOT( (1.0f-m_fControls_FlyUp), 0.0f, m_BotInfo_Jumper.fJumpLaunchVel );
			} else {
				fVal = 1.0f - m_fJumpJetTimer;
				FMATH_CLAMP_UNIT_FLOAT( fVal );
				// get max velocity based on time
				fVal = FMATH_FPOT( fVal, m_BotInfo_Jumper.fJumpDescendVel, m_BotInfo_Jumper.fJumpLaunchVel );
				// max vertical velocity is related to horiz translation
				fVal = FMATH_FPOT( m_XlatStickNormVecXZ_MS.MagSq(), fVal, -15.0f );

				m_fJumpJetDesiredVelocity = FMATH_FPOT( m_fControls_FlyUp, m_BotInfo_Jumper.fTerminalVelocity, fVal );
			}

			fDesiredPartIntensity = FMATH_FPOT( m_fControls_FlyUp, 0.0f, 0.9f );
			DeltaTime( ANIMTAP_JUMP_FLY_JETS );

			if( m_pJumpJetAudioEmitter ) {
				if( m_fJumpJetSoundVolume > fDesiredPartIntensity ) {
					m_fJumpJetSoundVolume -= FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDOFF_RATE;
					FMATH_CLAMPMIN( m_fJumpJetSoundVolume, fDesiredPartIntensity );
				} else if( m_fJumpJetSoundVolume < fDesiredPartIntensity ) {
					m_fJumpJetSoundVolume += FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDON_RATE;
					FMATH_CLAMPMAX( m_fJumpJetSoundVolume, fDesiredPartIntensity );
				}
				m_pJumpJetAudioEmitter->SetVolume( m_fJumpJetSoundVolume * _JUMPJET_NORMAL_SOUND );
			}

			m_fJumpJetTimer += FLoop_fPreviousLoopSecs;
			if( /*(m_fJumpJetTimer < 0.0f ) ||*/
				(m_nState == STATE_GROUND) ) {
				KillJumpJets();
			}
			break;

		case JUMPJETS_DIVE_STARTING:
			m_fUnitJumpJetRecharge = 1.0f;
			fDesiredPartIntensity = 1.0f;
			m_fDiveTimer += FLoop_fPreviousLoopSecs;

			fVal = GetControlValue( ANIMCONTROL_DIVE );
			if( fVal < 1.0f ) {
				fVal += FLoop_fPreviousLoopSecs * 2.0f;
				FMATH_CLAMP_MAX1( fVal );
				SetControlValue( ANIMCONTROL_DIVE, fVal );
			}

			if( m_fDiveTimer > _JUMPJETS_DIVE_TIME_START ) {
				m_JumpJetState = JUMPJETS_DIVING;
			}

			if( CollidedThisFrame() ) {
				_EndDive( TRUE );
			}

			break;

		case JUMPJETS_DIVING:
			fDesiredPartIntensity = 1.0f;
	
			m_fDiveTimer += FLoop_fPreviousLoopSecs;
			DeltaTime( ANIMTAP_DIVE );

			_MeleeSwing();

			//_HandleMeleeAttack();

			fVal = GetControlValue( ANIMCONTROL_DIVE );
			if( fVal < 1.0f ) {
				fVal += FLoop_fPreviousLoopSecs;
				FMATH_CLAMP_MAX1( fVal );
				SetControlValue( ANIMCONTROL_DIVE, fVal );
			}

			// bring jets sound up
			if( m_pJumpJetAudioEmitter && (m_fJumpJetSoundVolume < 1.0f) ) {
				m_fJumpJetSoundVolume += FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDON_RATE;
				FMATH_CLAMP_MAX1( m_fJumpJetSoundVolume );
				m_pJumpJetAudioEmitter->SetVolume( m_fJumpJetSoundVolume );
			}

			if( !Power_IsPoweredUp() ) {
				_EndDive( FALSE );
			}

			if( CollidedThisFrame() ) {
				_EndDive( TRUE );
			}

			break;

		case JUMPJETS_DIVE_ENDING:
			fDesiredPartIntensity = 0.0f;
			//_HandleMeleeAttack();
			_MeleeSwing();

			m_fDiveTimer += FLoop_fPreviousLoopSecs;

			fVal = GetControlValue( ANIMCONTROL_DIVE_RECOVER_LOWER );
			if( fVal > 0.0f ) {
				fVal -= FLoop_fPreviousLoopSecs * 2.0f;
				FMATH_CLAMP_MIN0( fVal );
				SetControlValue( ANIMCONTROL_DIVE_RECOVER_LOWER, fVal );
			}

			// shut down sounds if they're going
			if( m_pJumpJetAudioEmitter ) {
				m_fJumpJetSoundVolume -= FLoop_fPreviousLoopSecs * _JUMPJET_SOUNDOFF_RATE;
				FMATH_CLAMP_MIN0( m_fJumpJetSoundVolume );
				m_pJumpJetAudioEmitter->SetVolume( m_fJumpJetSoundVolume );
				if( m_fJumpJetSoundVolume == 0.0f ) {
					m_pJumpJetAudioEmitter->Destroy();
					m_pJumpJetAudioEmitter = NULL;
				}
			}

			if( DeltaTime( ANIMTAP_DIVE_RECOVER_UPPER, FLoop_fPreviousLoopSecs * 1.5f, TRUE ) ) {
				// done playing attack animation
				KillJumpJets();

				// stop the blade streamers
				if( m_hStreamerBladeLeft != FXSTREAMER_INVALID_HANDLE ) {
					CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerBladeLeft, FALSE );
					m_hStreamerBladeLeft = FXSTREAMER_INVALID_HANDLE;
				}

				if( m_hStreamerBladeRight != FXSTREAMER_INVALID_HANDLE ) {
					CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerBladeRight, FALSE );
					m_hStreamerBladeRight = FXSTREAMER_INVALID_HANDLE;
				}

			}
			break;
	}


	//extra special code that makes the jet effect turn
	//on when ai bots are doing velocity jumps.
	if (m_nPossessionPlayerIndex ==-1 &&
		fDesiredPartIntensity <=0.01f &&
		IsJumping())
	{
		f32 fAbsYVel = FMATH_FABS(m_Velocity_WS.y );
		if (fAbsYVel < 10.0f)
		{
			fDesiredPartIntensity = fmath_Div(fAbsYVel, 10.0f);
			FMATH_CLAMPMIN(fDesiredPartIntensity, 0.35f);
		}
		else
		{
			fDesiredPartIntensity = 1.0f;
		}
	}

	if( fDesiredPartIntensity > m_fJetsParticleIntensity ) {
		m_fJetsParticleIntensity += FLoop_fPreviousLoopSecs * _JETS_ON_RATE;
		FMATH_CLAMPMAX( m_fJetsParticleIntensity, fDesiredPartIntensity );
	} else if( fDesiredPartIntensity < m_fJetsParticleIntensity ) {
		m_fJetsParticleIntensity -= FLoop_fPreviousLoopSecs * _JETS_OFF_RATE;
		FMATH_CLAMPMIN( m_fJetsParticleIntensity, fDesiredPartIntensity );
	}

	if( (m_JumpJetState == JUMPJETS_DIVE_STARTING) || (m_JumpJetState == JUMPJETS_DIVING) ) {
		// stop the fire particles...
		_StopEmitter( &m_hJetFireParticleLeft );
		_StopEmitter( &m_hJetFireParticleRight );

		// start the launch particles
		_CheckAndStartEmitter( &m_hJetLaunchParticleLeft, &m_BotInfo_Jumper.hParticleDefJumpLaunch, m_nBoneIdxJetSmokeLeft );
		_CheckAndStartEmitter( &m_hJetLaunchParticleRight, &m_BotInfo_Jumper.hParticleDefJumpLaunch, m_nBoneIdxJetSmokeRight );

	} else {
		if( m_fJetsParticleIntensity > 0.0f ) {
			_CheckAndStartEmitter( &m_hJetFireParticleLeft, &m_BotInfo_Jumper.hParticleDefJumpJetFire, m_nBoneIdxJetFireLeft );
			_CheckAndStartEmitter( &m_hJetFireParticleRight, &m_BotInfo_Jumper.hParticleDefJumpJetFire, m_nBoneIdxJetFireRight );
		} else {
			_StopEmitter( &m_hJetFireParticleLeft );
			_StopEmitter( &m_hJetFireParticleRight );
		}

		_StopEmitter( &m_hJetLaunchParticleLeft );
		_StopEmitter( &m_hJetLaunchParticleRight );
	}
}


void CBotJumper::_JumpJetsSputter( void ) {
	if( m_fJumpJetSputterTime < m_fJumpJetTimer ) {
		return;
	}

	m_fJetsParticleIntensity = fmath_RandomFloatRange( 0.25f, 0.5f );
	m_fJumpJetSoundVolume = m_fJetsParticleIntensity;

	if( m_pJumpJetAudioEmitter == NULL ) {
		if( m_nPossessionPlayerIndex < 0 ) {
			m_pJumpJetAudioEmitter = FSNDFX_ALLOCNPLAY3D( m_BotInfo_JumperSounds.hJetsLoop, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vPos), m_BotInfo_Jumper.fSoundRadius3D, 1.0f, 0.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
		} else {
            m_pJumpJetAudioEmitter = FSNDFX_ALLOCNPLAY2D( m_BotInfo_JumperSounds.hJetsLoop, m_fJumpJetSoundVolume, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
		}
	}
							
	//if( m_hJetFireParticleLeft == FPARTICLE_INVALID_HANDLE ) {
	//	_CheckAndStartEmitter( &m_hJetFireParticleLeft, &m_BotInfo_Jumper.hParticleDefJumpJetFire, m_nBoneIdxJetFireLeft );
	//} else {
	//	_StopEmitter( &m_hJetFireParticleLeft );
	//}

	//if( m_hJetFireParticleRight == FPARTICLE_INVALID_HANDLE ) {
	//	_CheckAndStartEmitter( &m_hJetFireParticleRight, &m_BotInfo_Jumper.hParticleDefJumpJetFire, m_nBoneIdxJetFireRight );
	//} else {
	//	_StopEmitter( &m_hJetFireParticleRight );
	//}
	
	m_fJumpJetSputterTime = m_fJumpJetTimer - fmath_RandomFloatRange( 0.25f, 0.5f );
}


void CBotJumper::_SoundWork( void ) {
	if( m_nPossessionPlayerIndex >= 0 ) {
		return;
	}

	if( m_pJumpDiveAudioEmitter ) {
		m_pJumpDiveAudioEmitter->SetPosition( &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vPos) );
	}

	if( m_pJumpJetAudioEmitter ) {
		m_pJumpJetAudioEmitter->SetPosition( &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vPos) );
	}
}


void CBotJumper::_UpdateMatrices( void ) {
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

	if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	} else {
		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	}

	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );
}


void CBotJumper::_AnimBoneCallback( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCBJumper );
	FASSERT( m_pCBJumper->TypeBits() & ENTITY_BIT_BOTJUMPER );

	if( !m_pCBJumper ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	// give part mgr first shot
	if( m_pCBJumper->m_pPartMgr->AnimBoneCallbackFunctionHandler( uBoneIdx, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;

	// Groin
	} else if( uBoneIdx == m_pCBJumper->m_nBoneIdxGroin ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;

	// Torso
	} else if( uBoneIdx == m_pCBJumper->m_nBoneIdxTorso ) {
        CFMtx43A	mtxWorld;
		CFQuatA		q;

		q.BuildQuat( m_GroinVecY_WS, -m_pCBJumper->m_fLegsYaw_MS );
		mtxWorld.Mul( rParentMtx, rBoneMtx );

		rNewMtx.m_vPos = mtxWorld.m_vPos;
		q.MulPoint( rNewMtx.m_vRight, mtxWorld.m_vRight );
		q.MulPoint( rNewMtx.m_vUp, mtxWorld.m_vUp );
		q.MulPoint( rNewMtx.m_vFront, mtxWorld.m_vFront );

	// Head
	} else if( uBoneIdx == m_pCBJumper->m_nBoneIdxHead ) {
		m_pCBJumper->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );

	// arm aiming
	} else if( (uBoneIdx == m_pCBJumper->m_nBoneIdxArmLower_L) ||
			   (uBoneIdx == m_pCBJumper->m_nBoneIdxArmLower_R) ) {
		m_pCBJumper->_AimArm( uBoneIdx, rNewMtx, rParentMtx, rBoneMtx );
	
	// Unknown bone, not good...
	} else {
		FASSERT_NOW;
		rNewMtx.Mul( rParentMtx, rBoneMtx );
	}
}



// I don't really want to have 20 summers, so the aim summer will also sum...
// torso orientation for jumpjets

void CBotJumper::_HandleAimAnimations( void ) {
	f32	fUnitAimPitch;
	f32 fJumpJetX, fJumpJetZ;
	CFAnimFrame animframe;
	CFAnimFrame rotQuat;

	if( JumpJetsAreOn() && (m_fControls_FlyUp > 0.0f) ) {
		fJumpJetZ = FMATH_FPOT( m_XlatStickNormVecXZ_MS.z + 1.0f * 0.5f, _JUMPJET_TORSO_MINANGLE, _JUMPJET_TORSO_MAXANGLE );
		fJumpJetX = FMATH_FPOT( m_XlatStickNormVecXZ_MS.x + 1.0f * 0.5f, _JUMPJET_TORSO_MAXANGLE, _JUMPJET_TORSO_MINANGLE );
	} else {
		fJumpJetZ = 0.0f;
		fJumpJetX = 0.0f;
	}

	SetControlValue( ANIMCONTROL_AIM_SUMMER, 1.0f );

	fUnitAimPitch = fmath_Div( m_fAimPitch_WS - m_fMountPitchMin_WS, m_fMountPitchMax_WS - m_fMountPitchMin_WS );
	FMATH_CLAMP_UNIT_FLOAT( fUnitAimPitch );

	animframe.BuildQuat( CFVec3A::m_UnitAxisX, fJumpJetZ + FMATH_FPOT( fUnitAimPitch, _MIN_TORSO_TWIST, _MAX_TORSO_TWIST ), CFVec3A::m_Null );
	rotQuat.BuildQuat( CFVec3A::m_UnitAxisZ, fJumpJetX, CFVec3A::m_Null );
	animframe.Mul( rotQuat );

	m_TorsoAnimFrame.ReceiveSlerpOf( FLoop_fPreviousLoopSecs * 3.0f, m_TorsoAnimFrame, animframe );
	m_AnimManFrameAim.UpdateFrame( BONE_TORSO, m_TorsoAnimFrame );

	animframe.BuildQuat( CFVec3A::m_UnitAxisX, FMATH_FPOT( fUnitAimPitch, _MIN_HEAD_TWIST, _MAX_HEAD_TWIST ), CFVec3A::m_Null );
	m_AnimManFrameAim.UpdateFrame( BONE_HEAD, animframe );
}


void CBotJumper::_AimArm( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	f32	fUnitBlend = GetControlValue( ANIMCONTROL_FIRE_1 );
	CFQuatA qFinal, qTo;
	CFQuatA *pArmQuat;
	f32 fSlerp;

	if( uBoneIdx == m_nBoneIdxArmLower_L ) {
		pArmQuat = &m_qArmAim_L;
	} else {
		pArmQuat = &m_qArmAim_R;
	}

	if( fUnitBlend == 0.0f ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		pArmQuat->BuildQuat( rNewMtx );
		return;
	} else if( fUnitBlend < 0.75f ) {
		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );
	} else {
		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );
		CFMtx43A::m_Temp.m_vFront.Sub( m_TargetedPoint_WS, rNewMtx.m_vPos );
		CFMtx43A::m_Temp.m_vFront.Unitize();
		CFMtx43A::m_Temp.m_vRight.Cross( rNewMtx.m_vUp, CFMtx43A::m_Temp.m_vFront );
		CFMtx43A::m_Temp.m_vRight.Unitize();
		CFMtx43A::m_Temp.m_vUp.Cross( CFMtx43A::m_Temp.m_vFront, CFMtx43A::m_Temp.m_vRight );
	}

	fSlerp = _ARM_SLERP_RATE * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP_UNIT_FLOAT( fSlerp );
	
	qTo.BuildQuat( CFMtx43A::m_Temp );
	pArmQuat->ReceiveSlerpOf( fSlerp, *pArmQuat, qTo );
	pArmQuat->BuildMtx( rNewMtx );
	rNewMtx.m_vPos = CFMtx43A::m_Temp.m_vPos;
}


void CBotJumper::_MeleeWork( void ) {
	f32 fCtlValue;

	switch( m_MeleeState ) {
		case MELEE_STATE_NONE:
			if( m_bControls_Melee ) {
				if( (m_nState == STATE_AIR) ) {
					if( !IsBackBroken() && !IsDiving() ) {
						_BeginDive();
					}					
					return;
				}

				if( m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_PRIMARY ) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL ) {
					break;
				}

				m_MeleeState = MELEE_STATE_STARTING;
				ZeroTime( ANIMTAP_ATTACK );
				ZeroTime( ANIMTAP_ATTACK_START );
				ZeroTime( ANIMTAP_ATTACK_END );

				SetControlValue( ANIMCONTROL_ATTACK,		0.0f );
				SetControlValue( ANIMCONTROL_ATTACK_START,	0.0f );
				SetControlValue( ANIMCONTROL_ATTACK_END,	0.0f );
				SetControlValue( ANIMCONTROL_ATTACK_SUMMER,	0.0f );

				//m_vLastMeleePositionLeft = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeLeft]->m_vPos;
				//m_vLastMeleePositionRight = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeRight]->m_vPos;

				MeleeStartAttack( &m_MeleeDataLeft );
				MeleeStartAttack( &m_MeleeDataRight );

				// start the streamers
				if( (m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT) && (m_hStreamerBladeLeft == FXSTREAMER_INVALID_HANDLE) ) {
					m_hStreamerBladeLeft = CFXStreamerEmitter::SpawnFromMeshBones( _STREAMER_BLADE_DEPTH, m_pWorldMesh, m_apszStreamerBonesBladeLeft, _STREAMER_BLADE_ALPHA, 
																					&m_BladeStreamerTex, _STREAMER_BLADE_WIDTH, _STREAMER_BLADE_SAMPLERATE, CFXStreamerEmitter::USE_FRONT_AXIS );
				}

				if( (m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT) && (m_hStreamerBladeRight == FXSTREAMER_INVALID_HANDLE) ) {
					m_hStreamerBladeRight = CFXStreamerEmitter::SpawnFromMeshBones( _STREAMER_BLADE_DEPTH, m_pWorldMesh, m_apszStreamerBonesBladeRight, _STREAMER_BLADE_ALPHA, 
																					&m_BladeStreamerTex, _STREAMER_BLADE_WIDTH, _STREAMER_BLADE_SAMPLERATE, CFXStreamerEmitter::USE_FRONT_AXIS );
				}
			}
			break;

		case MELEE_STATE_STARTING:
			_AttackSummerBlendWork();
			//_HandleMeleeAttack();
			_MeleeSwing();
			
			// blend in our attack animation (very quickly)
			if( (fCtlValue = GetControlValue( ANIMCONTROL_ATTACK_START )) < 1.0f ) {
				fCtlValue += FLoop_fPreviousLoopSecs * _ATTACK_BLEND_IN_RATE;
				FMATH_CLAMP_MAX1( fCtlValue );
				SetControlValue( ANIMCONTROL_ATTACK_START, fCtlValue );
				SetControlValue( ANIMCONTROL_ATTACK_SUMMER, fCtlValue );
			}

            // if done playing attack start, move on to attack
			if( DeltaTime( ANIMTAP_ATTACK_START, FLoop_fPreviousLoopSecs * m_BotInfo_Jumper.fMeleeAttackSpeed, TRUE ) ) {
				SetControlValue( ANIMCONTROL_ATTACK_START, 0.0f );
				SetControlValue( ANIMCONTROL_ATTACK, 1.0f );
				m_MeleeState = MELEE_STATE_SLASHING;
				_NewRandomAttack();
			}

			break;

		case MELEE_STATE_SLASHING:
			_AttackSummerBlendWork();
			//_HandleMeleeAttack();
			_MeleeSwing();

			if( DeltaTime( ANIMTAP_ATTACK, FLoop_fPreviousLoopSecs * m_BotInfo_Jumper.fMeleeAttackSpeed ) ) {
				if( !m_bControls_Melee ) {
					// stop attacking
					SetControlValue( ANIMCONTROL_ATTACK, 0.0f );
					SetControlValue( ANIMCONTROL_ATTACK_END, 1.0f );
					m_MeleeState = MELEE_STATE_FINISHING;
				} else {
					_NewRandomAttack();
				}
			}
			break;

		case MELEE_STATE_FINISHING:
			// blend out the attack if necessary
			if( (fCtlValue = GetControlValue( ANIMCONTROL_ATTACK_END )) > 0.0f ) {
				fCtlValue -= FLoop_fPreviousLoopSecs * _ATTACK_BLEND_OUT_RATE;
				FMATH_CLAMP_MIN0( fCtlValue );
				SetControlValue( ANIMCONTROL_ATTACK_END,	fCtlValue );
				SetControlValue( ANIMCONTROL_ATTACK_SUMMER, fCtlValue );
			}

			if( DeltaTime( ANIMTAP_ATTACK_END, FLoop_fPreviousLoopSecs * m_BotInfo_Jumper.fMeleeAttackSpeed, TRUE ) ) {
				m_MeleeState = MELEE_STATE_NONE;
			}

			// kill the streamers
			if( m_hStreamerBladeLeft != FXSTREAMER_INVALID_HANDLE ) {
				CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerBladeLeft, FALSE );
				m_hStreamerBladeLeft = FXSTREAMER_INVALID_HANDLE;
			}
			if( m_hStreamerBladeRight != FXSTREAMER_INVALID_HANDLE ) {
				CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerBladeRight, FALSE );
				m_hStreamerBladeRight = FXSTREAMER_INVALID_HANDLE;
			}

			break;
	}
}


void CBotJumper::_NewRandomAttack( void ) {
	//m_bHitWithLeftThisAttack = FALSE;
	//m_bHitWithRightThisAttack = FALSE;

	//m_nHitEntityCountLeft	= 0;
	//m_nHitEntityCountRight	= 0;

	m_fAttackAngleDesiredL	= fmath_RandomBipolarUnitFloat() * _ATTACK_ROT_Z;
	m_fAttackAngleDesiredR	= fmath_RandomBipolarUnitFloat() * _ATTACK_ROT_Z;

	if( /*(m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_SECONDARY ) == CBotPartMgr::COMPONENT_STATUS_SOME_FULLY_OPERATIONAL) ||*/	// wtf?
		(m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_SECONDARY ) == CBotPartMgr::COMPONENT_STATUS_ALL_FULLY_OPERATIONAL) ) {
		// play the sound
		if( m_nPossessionPlayerIndex < 0 ) {
			fsndfx_Play3D( m_BotInfo_JumperSounds.hMeleeAttack, &m_MountPos_WS, m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D, fmath_RandomFloatRange( 0.9f, 1.1f ) );
		} else {
			fsndfx_Play2D( m_BotInfo_JumperSounds.hMeleeAttack, m_BotInfo_Jumper.fSoundVolume2D, fmath_RandomFloatRange( 0.9f, 1.1f ) );
		}
	}

}


void CBotJumper::_AttackSummerBlendWork( void ) {
	CFAnimFrame animframe;

	if( m_fAttackAngleDesiredL > m_fAttackAngleCurrentL ) {
		m_fAttackAngleCurrentL += FLoop_fPreviousLoopSecs * _ATTACK_BLEND_RATE;
		FMATH_CLAMPMAX( m_fAttackAngleCurrentL, m_fAttackAngleDesiredL );
	} else if( m_fAttackAngleDesiredL < m_fAttackAngleCurrentL ) {
		m_fAttackAngleCurrentL -= FLoop_fPreviousLoopSecs * _ATTACK_BLEND_RATE;
		FMATH_CLAMPMIN( m_fAttackAngleCurrentL, m_fAttackAngleDesiredL );
	}
	
	animframe.BuildQuat( CFVec3A::m_UnitAxisZ, m_fAttackAngleCurrentL, CFVec3A::m_Null );
	m_pAnimManFrameAttack->UpdateFrame( SUMMERBONE_ARM_UPPER_L, animframe );

	if( m_fAttackAngleDesiredR > m_fAttackAngleCurrentR ) {
		m_fAttackAngleCurrentR += FLoop_fPreviousLoopSecs * _ATTACK_BLEND_RATE;
		FMATH_CLAMPMAX( m_fAttackAngleCurrentR, m_fAttackAngleDesiredR );
	} else if( m_fAttackAngleDesiredR < m_fAttackAngleCurrentR ) {
		m_fAttackAngleCurrentR -= FLoop_fPreviousLoopSecs * _ATTACK_BLEND_RATE;
		FMATH_CLAMPMIN( m_fAttackAngleCurrentR, m_fAttackAngleDesiredR );
	}

	animframe.BuildQuat( CFVec3A::m_UnitAxisZ, m_fAttackAngleCurrentR, CFVec3A::m_Null );
	m_pAnimManFrameAttack->UpdateFrame( SUMMERBONE_ARM_UPPER_R, animframe );
}


//void CBotJumper::_HandleMeleeAttack( void ) {
//	// check for collisions...
//	CFTrackerCollideProjSphereInfo trackerCollInfo;
//	
//	m_CollInfo.nCollTestType					= FMESH_COLLTESTTYPE_PROJSPHERE;
//	m_CollInfo.nCollMask						= FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
//	m_CollInfo.nResultsLOD						= FCOLL_LOD_HIGHEST;
//	m_CollInfo.nTrackerUserTypeBitsMask			= FCOLL_USER_TYPE_BITS_ALL;
//	m_CollInfo.nStopOnFirstOfCollMask           = FCOLL_MASK_NONE;
//	m_CollInfo.bFindClosestImpactOnly			= TRUE;
//	m_CollInfo.bCullBacksideCollisions			= FALSE;
//	m_CollInfo.bCalculateImpactData				= TRUE;
//
//	trackerCollInfo.pProjSphere					= &m_CollInfo.ProjSphere;
//	trackerCollInfo.pCallback					= _MeleeHitTrackerCB;
//	trackerCollInfo.nTrackerTypeBits			= FWORLD_TRACKERTYPEBIT_MESH;
//	trackerCollInfo.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
//	trackerCollInfo.bIgnoreCollisionFlag		= FALSE;
//
//	m_pCBJumper = this;
//
//	// LEFT...
//	if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT ) {
//		m_bAttackingWithLeft = TRUE;
//		m_CollInfo.ProjSphere.Init( &m_vLastMeleePositionLeft, 
//									&(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeLeft]->m_vPos), 
//									_MELEE_SPHERE_RADIUS );
//
//		trackerCollInfo.nTrackerSkipCount = 0;
//		trackerCollInfo.ppTrackerSkipList = NULL;
//
//		fworld_CollideWithTrackers( &trackerCollInfo );
//
//		fcoll_Clear();
//
//		if( fworld_CollideWithWorldTris( &m_CollInfo ) ) {
//			_SpawnMeleeEffects( &FColl_aImpactBuf[0] ); 
//			if( !m_bHitWithLeftThisAttack ) {
//				// play a sound
//				if( m_nPossessionPlayerIndex < 0 ) {
//					fsndfx_Play3D( m_BotInfo_JumperSounds.hMeleeImpact, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeLeft]->m_vPos), 
//									m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D );
//				} else {
//					fsndfx_Play2D( m_BotInfo_JumperSounds.hMeleeImpact, m_BotInfo_Jumper.fSoundVolume3D );
//				}
//
//				m_bHitWithLeftThisAttack = TRUE;
//			}
//		}
//	}
//
//	// RIGHT...
//	if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT ) {
//		m_bAttackingWithLeft = FALSE;
//		m_CollInfo.ProjSphere.Init( &m_vLastMeleePositionRight, 
//									&(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeRight]->m_vPos), 
//									_MELEE_SPHERE_RADIUS );
//
//		trackerCollInfo.nTrackerSkipCount = 0;
//		trackerCollInfo.ppTrackerSkipList = NULL;
//
//		fworld_CollideWithTrackers( &trackerCollInfo );
//
//		fcoll_Clear();
//
//		if( fworld_CollideWithWorldTris( &m_CollInfo ) ) {
//			_SpawnMeleeEffects( &FColl_aImpactBuf[0] );        
//		
//			if( !m_bHitWithRightThisAttack ) {
//				// play a sound
//				if( m_nPossessionPlayerIndex < 0 ) {
//					fsndfx_Play3D( m_BotInfo_JumperSounds.hMeleeImpact, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeLeft]->m_vPos), 
//									m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D );
//				} else {
//					fsndfx_Play2D( m_BotInfo_JumperSounds.hMeleeImpact, m_BotInfo_Jumper.fSoundVolume3D );
//				}
//				m_bHitWithRightThisAttack = TRUE;
//			}
//		}
//	}
//
//	m_vLastMeleePositionLeft  = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeLeft]->m_vPos;
//	m_vLastMeleePositionRight = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeRight]->m_vPos;
//	m_pCBJumper = NULL;
//}


//BOOL CBotJumper::_MeleeHitTrackerCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
//	CFWorldMesh *pWMesh = (CFWorldMesh*)pTracker;
//
//	FASSERT( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH );
//	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
//		return TRUE;
//	}
//
//	// hit an entity...
//	CEntity *pEntity = (CEntity*)pTracker->m_pUser;
//
//	if( pEntity == m_pCBJumper ) {
//		// hit ourself
//		return TRUE;
//	}
//
//	fcoll_Clear();
//	if( !pWMesh->CollideWithMeshTris( &m_CollInfo ) ) {
//		// missed
//		return TRUE;
//	}
//
//	m_pCBJumper->_SpawnMeleeEffects( &FColl_aImpactBuf[0] );
//    m_pCBJumper->_InflictMeleeDamage( pEntity, pWMesh, &FColl_aImpactBuf[0] );
//
//	return TRUE;
//}


void CBotJumper::_SpawnMeleeEffects( const FCollImpact_t *pCollImpact ) {
	const CGCollMaterial *pMaterial = CGColl::GetMaterial( pCollImpact );

	if( m_nPossessionPlayerIndex >= 0 ) {
		fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_LOW, &m_hForce );
	}

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

		DebrisSpawner.m_Mtx.m_vPos			= pCollImpact->ImpactPoint;
		DebrisSpawner.m_Mtx.m_vZ			= pCollImpact->UnitFaceNormal;
		DebrisSpawner.m_nEmitterType		= CFDebrisSpawner::EMITTER_TYPE_POINT;
		DebrisSpawner.m_pDebrisGroup		= pMaterial->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_MEDIUM ];
		DebrisSpawner.m_fSpawnerAliveSecs	= 0.0f;
		DebrisSpawner.m_fMinSpeed			= 10.0f;
		DebrisSpawner.m_fMaxSpeed			= 20.0f;
		DebrisSpawner.m_fUnitDirSpread		= 0.2f;
		DebrisSpawner.m_fScaleMul			= 1.0f;
		DebrisSpawner.m_fGravityMul			= 1.0f;
		DebrisSpawner.m_fRotSpeedMul		= 1.0f;
		DebrisSpawner.m_nMinDebrisCount		= 1;
		DebrisSpawner.m_nMaxDebrisCount		= 3;

		CGColl::SpawnDebris( &DebrisSpawner );
	}
}


//void CBotJumper::_InflictMeleeDamage( CEntity* pEntity, CFWorldMesh *pWMesh, const FCollImpact_t *pCollImpact ) {
//	u32 i;
//
//	if( m_bAttackingWithLeft ) {
//		if( m_nHitEntityCountLeft >= _MAX_HIT_ENTITIES ) {
//			return;
//		}
//		
//		for( i=0; i<m_nHitEntityCountLeft; i++ ) {
//			if( pEntity == m_apHitEntityBufferLeft[i] ) {
//				// already hit this guy once
//				return;
//			}
//		}
//
//		// have a new victim...
//		if( m_nHitEntityCountLeft < _MAX_HIT_ENTITIES ) {
//			m_apHitEntityBufferLeft[m_nHitEntityCountLeft] = pEntity;
//			m_nHitEntityCountLeft++;
//		}
//	} else {
//		if( m_nHitEntityCountRight >= _MAX_HIT_ENTITIES ) {
//			return;
//		}
//		
//		for( i=0; i<m_nHitEntityCountRight; i++ ) {
//			if( pEntity == m_apHitEntityBufferRight[i] ) {
//				// already hit this guy once
//				return;
//			}
//		}
//
//		// have a new victim...
//		if( ++m_nHitEntityCountRight < _MAX_HIT_ENTITIES ) {
//			m_apHitEntityBufferRight[m_nHitEntityCountRight] = pEntity;
//		}
//	}
//
//	// if we're here, it's time to hurt someone...
//    
////	DEVPRINTF( "Melee attack damaging:  %x\n", pEntity );
//	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
//	if( !pDamageForm ) {
//		return;
//	}
//
//	CFVec3A vFireDir;
//	f32		fMag2;
//
//	vFireDir.Sub( m_CollInfo.ProjSphere.m_vCenterEnd_WS, m_CollInfo.ProjSphere.m_vCenterStart_WS );
//	fMag2 = vFireDir.MagSq();
//
//	if( fMag2 > 0.00001f ) {
//		vFireDir.Mul( fmath_InvSqrt( fMag2 ) );
//	} else {
//		vFireDir = CFVec3A::m_UnitAxisY;
//	}
//
//	pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_IMPACT;
//	pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
//	pDamageForm->m_pDamageProfile	= m_BotInfo_Jumper.apDamageProfileMelee[ m_nPossessionPlayerIndex < 0 ? 0 : 1 ];
//	pDamageForm->m_Damager.pWeapon	= NULL;
//	pDamageForm->m_Damager.pBot		= this;
//	pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
//	pDamageForm->m_pDamageeEntity	= pEntity;
//	pDamageForm->InitTriDataFromCollImpact( pWMesh, pCollImpact, &vFireDir );
//
//	CDamage::SubmitDamageForm( pDamageForm );
//}


void CBotJumper::MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstCollision ) {
	CBot::MeleeCollision( pEntity, pImpact, bFirstCollision );
	_SpawnMeleeEffects( pImpact );

	if( bFirstCollision ) {
		if( !IsPlayerBot() ) {
			fsndfx_Play3D( m_BotInfo_JumperSounds.hMeleeImpact, &pImpact->ImpactPoint, 
							m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D );
		} else {
			fsndfx_Play2D( m_BotInfo_JumperSounds.hMeleeImpact, m_BotInfo_Jumper.fSoundVolume2D );
		}
	}
}


/////////////////////
//Guns


void CBotJumper::_GunWork( void ) {
	f32 fCtlValue, fLastUnitAnimTime;
	BOOL bPlaySound;

	if( m_fControls_Fire1 < 0.1f ) {
		fCtlValue = GetControlValue( ANIMCONTROL_FIRE_1 );
		if( fCtlValue > 0.0f ) {
            fCtlValue -= FLoop_fPreviousLoopSecs * _FIRE_BLEND_OUT_RATE;
			FMATH_CLAMP_MIN0( fCtlValue );
			SetControlValue( ANIMCONTROL_FIRE_1, fCtlValue );
			DeltaTime( ANIMTAP_FIRE_1 );
		}
		m_fFireTimer = 0.0f;
		return;
	}

	// guns are firing
	// guns are synced with the animation...
	fCtlValue = GetControlValue( ANIMCONTROL_FIRE_1 );
	if( fCtlValue < 1.0f ) {
		fCtlValue += FLoop_fPreviousLoopSecs * _FIRE_BLEND_IN_RATE;
		FMATH_CLAMP_MAX1( fCtlValue );
		SetControlValue( ANIMCONTROL_FIRE_1, fCtlValue );
	}

	fLastUnitAnimTime = m_fFireTimer;
	m_fFireTimer += FLoop_fPreviousLoopSecs * m_BotInfo_Jumper.fFireRate;
	if( m_fFireTimer > 1.0f ) {
		fLastUnitAnimTime = 0.0f;
		m_fFireTimer -= 1.0f;
	}
	UpdateUnitTime( ANIMTAP_FIRE_1, m_fFireTimer );

	// check to see whether we've passed a point in the anim where we should fire
	s32 nFiringGun = -1;

	if( (fLastUnitAnimTime < _ANIM_FIRE_POINT0) && (m_fFireTimer > _ANIM_FIRE_POINT0) ) {
		nFiringGun = 0;
		bPlaySound = TRUE;
	} if( (fLastUnitAnimTime < _ANIM_FIRE_POINT1) && (m_fFireTimer > _ANIM_FIRE_POINT1) ) {
		nFiringGun = 1;
		bPlaySound = TRUE;
	} else if( (fLastUnitAnimTime < _ANIM_FIRE_POINT2) && (m_fFireTimer > _ANIM_FIRE_POINT2) ) {
		bPlaySound = FALSE;
	} else if( (fLastUnitAnimTime < _ANIM_FIRE_POINT3) && (m_fFireTimer > _ANIM_FIRE_POINT3) ) {
		bPlaySound = FALSE;
	}

	if( nFiringGun < 0 ) {
		return;
	}

	FASSERT( nFiringGun < 4 );

	// check the state of the arm that's firing the gun.  0 & 2 are the left arm
	BOOL bArmDisabled = FALSE;

	if( m_pPartMgr && m_pPartMgr->IsCreated() ) {

		if( nFiringGun == 0 || nFiringGun == 2 ) {
			if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_REMOVED ) {
				return;
			} else if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_DANGLING ) {
				bArmDisabled = TRUE;
			}
		} else {
			if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_REMOVED ) {
				return;
			} else if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_DANGLING ) {
				bArmDisabled = TRUE;
			}
		}
	}
	
	CFVec3A vMuzzlePt	= m_pWorldMesh->GetBoneMtxPalette()[m_anBoneIdxPriFire[nFiringGun]]->m_vPos;
	CFVec3A vTgtPt		= m_TargetedPoint_WS;
	vTgtPt.x += fmath_RandomBipolarUnitFloat();
	vTgtPt.y += fmath_RandomBipolarUnitFloat();
	vTgtPt.z += fmath_RandomBipolarUnitFloat();

	// init the tracer
	m_TracerDef.pUser		= this;
	m_TracerDef.TailPos_WS	= vMuzzlePt;

	// if arm is disabled, just aim down the barrel of the gun
	if( bArmDisabled ) {
		m_TracerDef.UnitDir_WS = m_pWorldMesh->GetBoneMtxPalette()[m_anBoneIdxPriFire[nFiringGun]]->m_vFront;
	} else {
		m_TracerDef.UnitDir_WS.Sub( vTgtPt, vMuzzlePt );

		f32 fMag2 = m_TracerDef.UnitDir_WS.MagSq();
		if( fMag2 < 0.001f ) {
			m_TracerDef.UnitDir_WS = m_pWorldMesh->GetBoneMtxPalette()[m_anBoneIdxPriFire[nFiringGun]]->m_vFront;
		} else {
			m_TracerDef.UnitDir_WS.Mul( fmath_InvSqrt( fMag2 ) );
		}
	}

	ConstructDamagerData();
	if( tracer_NewTracer( m_hTracerGroup, &m_TracerDef, 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager ) ) {
		JustFired();
		// play a sound, do some ff
		if( bPlaySound ) {
			if( m_nPossessionPlayerIndex < 0 ) {
				fsndfx_Play3D( m_BotInfo_JumperSounds.hGunsFire, &vMuzzlePt, m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D );
			} else {
				fsndfx_Play2D( m_BotInfo_JumperSounds.hGunsFire, m_BotInfo_Jumper.fSoundVolume3D );
				fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_LIGHT_PULSE, &m_hForce );
			}
		}

		// create the streamer that trails the tracer
		for( u32 i=0; i<m_BotInfo_Jumper.uMaxTracers; i++ ) {
			if( m_afGunStreamerTimer[i] < 0.0f ) {
				m_afGunStreamerTimer[i] = 0.0f;

				if( i & 0x01 ) {
					m_aGunStreamerPool[i] = CFXStreamerEmitter::SpawnFromMtx43A( _GUNSTREAMER_DEPTH, &(m_aGunStreamerMtxPool[i]), _GUNSTREAMER_ALPHA, &m_GunStreamerTexInst, NULL, _GUNSTREAMER_WIDTH, _GUNSTREAMER_SAMPLERATE, CFXStreamerEmitter::USE_RIGHT_AXIS );
				} else {
					m_aGunStreamerPool[i] = CFXStreamerEmitter::SpawnFromMtx43A( _GUNSTREAMER_DEPTH, &(m_aGunStreamerMtxPool[i]), _GUNSTREAMER_ALPHA, &m_GunStreamerTexInst, NULL, _GUNSTREAMER_WIDTH, _GUNSTREAMER_SAMPLERATE, CFXStreamerEmitter::USE_UP_AXIS );
				}

				CFXStreamerEmitter::EmitPoint( m_aGunStreamerPool[i] );
				tracer_AddExtraParamsToLastTracer( _TracerMovedCB, (void*)i );

				m_aGunStreamerMtxPool[i].m_vFront = m_TracerDef.UnitDir_WS;
				m_aGunStreamerMtxPool[i].m_vRight.CrossYWithVec( m_aGunStreamerMtxPool[i].m_vFront );
				if( m_aGunStreamerMtxPool[i].m_vRight.MagSq() > 0.1f ) {
					m_aGunStreamerMtxPool[i].m_vRight.Unitize();
				} else {
					m_aGunStreamerMtxPool[i].m_vRight = CFVec3A::m_UnitAxisX;
				}

				m_aGunStreamerMtxPool[i].m_vUp.Cross( m_aGunStreamerMtxPool[i].m_vFront, m_aGunStreamerMtxPool[i].m_vRight );
				m_aGunStreamerMtxPool[i].RotateZ( FMATH_PI * fmath_RandomFloat() );
				m_aGunStreamerMtxPool[i].m_vPos = vMuzzlePt;
				break;
			}
		}
	}
}


void CBotJumper::_TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
	if( nKillReason != TRACER_KILLREASON_HIT_GEO ) {
		return;
	}

	FASSERT( pTracerDef->pUser );
	FASSERT( ((CBotJumper*)pTracerDef->pUser)->TypeBits() & ENTITY_BIT_BOTJUMPER );

	CBotJumper *pJumper = (CBotJumper*)pTracerDef->pUser;

	const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );

	pCollMaterial->DrawAllDrawableParticles( &pImpact->ImpactPoint, &pImpact->UnitFaceNormal, TRUE,
											 fmath_RandomFloatRange( 0.2f, 0.4f ),		// sparks
											 fmath_RandomFloatRange( 0.25f, 0.75f ),	// bits
											 0.0f );									// dust
	// hey, we have an explosion now, let it do the work
	//CFVec3A vPush;
	//vPush.Mul( pImpact->UnitFaceNormal, _USERPROPS_IMPACT_OFFSET ).Add( pImpact->ImpactPoint );
	//CMuzzleFlash::AddFlash_CardPosterXY( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1], 
	//										vPush, _USERPROPS_IMPACT_SCALE, _USERPROPS_IMPACT_ALPHA );

	//if( pCollMaterial->IsPockMarkEnabled() ) {
	//	potmark_NewPotmark( &pImpact->ImpactPoint, &pImpact->UnitFaceNormal, _USERPROPS_POTMARK_SIZE, POTMARKTYPE_LASER1 );
	//}

	// spawn an explosion
	FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

	if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
		FExplosionSpawnParams_t SpawnParams;

		SpawnParams.InitToDefaults();

		SpawnParams.uFlags			= FEXPLOSION_SPAWN_NONE;
		SpawnParams.Pos_WS			= pImpact->ImpactPoint;
		SpawnParams.UnitDir			= pImpact->UnitFaceNormal;
		SpawnParams.uSurfaceType	= pImpact->nUserType;
		SpawnParams.pDamageProfile	= pJumper->m_BotInfo_Jumper.apDamageProfileGun[ pJumper->m_nPossessionPlayerIndex < 0 ? 0 : 1 ];
		SpawnParams.pDamager		= &pTracerDef->Damager;

		CExplosion2::SpawnExplosion( hSpawner, pJumper->m_BotInfo_Jumper.hGunExplosion, &SpawnParams );
	}
}


void CBotJumper::_TracerMovedCB( void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied ) {
	u32 uStreamerID = (u32)pUserData;
	f32 fVal;

	FASSERT( uStreamerID < m_BotInfo_Jumper.uMaxTracers );

	if( bDied ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_aGunStreamerPool[uStreamerID], FALSE );
		m_afGunStreamerTimer[uStreamerID] = -1.0f;
		return;
	}

	CFVec3A vOffset;

	m_afGunStreamerTimer[uStreamerID] += FLoop_fPreviousLoopSecs * 5.0f;


	fVal = m_afGunStreamerTimer[uStreamerID];
	FMATH_CLAMP_MAX1( fVal );
	

	m_aGunStreamerMtxPool[uStreamerID].m_vPos.Zero();
	if( uStreamerID & 0x01 ) {
		CFMtx43A::m_RotZ.SetRotationZ( FLoop_fPreviousLoopSecs * 2.5f * FMATH_2PI );
		m_aGunStreamerMtxPool[uStreamerID].Mul( CFMtx43A::m_RotZ );
		vOffset.Mul( m_aGunStreamerMtxPool[uStreamerID].m_vRight, fVal * 3.0f );
	} else {
		CFMtx43A::m_RotZ.SetRotationZ( FLoop_fPreviousLoopSecs * -2.5f * FMATH_2PI );
		m_aGunStreamerMtxPool[uStreamerID].Mul( CFMtx43A::m_RotZ );
		vOffset.Mul( m_aGunStreamerMtxPool[uStreamerID].m_vUp, fVal * 3.0f );
	}
	

	m_aGunStreamerMtxPool[uStreamerID].m_vPos.Add( NewPos_WS, vOffset );
}


void CBotJumper::_TracerBuildTrackerSkipList( void *pUser ) {
	FASSERT( pUser );
	FASSERT( ((CBotJumper*)pUser)->TypeBits() & ENTITY_BIT_BOTJUMPER );

	((CBotJumper*)pUser)->AppendTrackerSkipList();
}


////////////////////
// DIVING


void CBotJumper::_BeginDive( void ) {
	if( (m_JumpJetState == JUMPJETS_OFF) && (m_nPossessionPlayerIndex >-1 && m_fJumpJetTimer > 0.0f) ) {
		return;
	}

	MeleeStartAttack( &m_MeleeDataLeft );
	MeleeStartAttack( &m_MeleeDataRight );


	if (m_nPossessionPlayerIndex ==-1)
	{
		m_vDiveTgt = m_ControlBot_TargetPoint_WS;
	}
	else
	{
		m_vDiveTgt = m_TargetedPoint_WS;
	}
	FMATH_CLAMPMAX( m_vDiveTgt.y, m_MountPos_WS.y );

	m_pDiveTgtEntity	= NULL;
	m_bDiveUseControls	= FALSE;

	if( m_pTargetedMesh && (m_pTargetedMesh->m_nUser == MESHTYPES_ENTITY) ) {
		m_pDiveTgtEntity = (CEntity*)(m_pTargetedMesh->m_pUser);
	} else {
		m_pDiveTgtEntity = NULL;
	}

	m_JumpJetState		= JUMPJETS_DIVE_STARTING;
	m_fDiveTimer		= 0.0f;
	m_fDiveDecelSpeed	= 0.0f;

	
	ZeroTime( ANIMTAP_DIVE );
	SetControlValue( ANIMCONTROL_DIVE, 0.0f );

	// start the streamers
	if( (m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT) && (m_hStreamerBladeLeft == FXSTREAMER_INVALID_HANDLE) ) {
		m_hStreamerBladeLeft = CFXStreamerEmitter::SpawnFromMeshBones( _STREAMER_BLADE_DEPTH, m_pWorldMesh, m_apszStreamerBonesBladeLeft, _STREAMER_BLADE_ALPHA, 
																		&m_BladeStreamerTex, _STREAMER_BLADE_WIDTH, _STREAMER_BLADE_SAMPLERATE, CFXStreamerEmitter::USE_FRONT_AXIS );
	}
	if( (m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT) && (m_hStreamerBladeRight == FXSTREAMER_INVALID_HANDLE) ) {
		m_hStreamerBladeRight = CFXStreamerEmitter::SpawnFromMeshBones( _STREAMER_BLADE_DEPTH, m_pWorldMesh, m_apszStreamerBonesBladeRight, _STREAMER_BLADE_ALPHA, 
																		&m_BladeStreamerTex, _STREAMER_BLADE_WIDTH, _STREAMER_BLADE_SAMPLERATE, CFXStreamerEmitter::USE_FRONT_AXIS );
	}


	// start sound 
	if( m_nPossessionPlayerIndex < 0 ) {
		m_pJumpDiveAudioEmitter = FSNDFX_ALLOCNPLAY3D( m_BotInfo_JumperSounds.hDive, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vPos), m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D, FAudio_EmitterDefaultPriorityLevel, TRUE );
	} else {
		m_pJumpDiveAudioEmitter = FSNDFX_ALLOCNPLAY2D( m_BotInfo_JumperSounds.hDive, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
	}
}


void CBotJumper::_EndDive( BOOL bHitSomething ) {
	if( !bHitSomething ) {
		m_fDiveDecelSpeed = 200.0f;
	}

	// play the sound
	if( m_nPossessionPlayerIndex < 0 ) {
		fsndfx_Play3D( m_BotInfo_JumperSounds.hMeleeAttack, &m_MountPos_WS, m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D, fmath_RandomFloatRange( 1.2f, 1.4f ) );
	} else {
		fsndfx_Play2D( m_BotInfo_JumperSounds.hMeleeAttack, m_BotInfo_Jumper.fSoundVolume2D, fmath_RandomFloatRange( 1.2f, 1.4f ) );
	}

	if( bHitSomething ) {
		// do some sound & FF
		if( m_nPossessionPlayerIndex < 0 ) {
			fsndfx_Play3D( m_BotInfo_JumperSounds.hMeleeImpact, &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxMeleeLeft]->m_vPos), 
							m_BotInfo_Jumper.fSoundRadius3D, 1.0f, m_BotInfo_Jumper.fSoundVolume3D );
		} else {
			fsndfx_Play2D( m_BotInfo_JumperSounds.hMeleeImpact, m_BotInfo_Jumper.fSoundVolume3D );
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MAX, &m_hForce );
			CFCamera *pCamera = fcamera_GetCameraByIndex( PLAYER_CAM( m_nPossessionPlayerIndex ) );
			if( pCamera ) {
				pCamera->ShakeCamera( _DIVEHIT_CAMSHAKE_INTENSITY, _DIVEHIT_CAMSHAKE_DURATION );
			}
		}

	}


	m_nJumpState			= BOTJUMPSTATE_NONE;
	ClearJumping();
	m_fJumpJetTimer			= 10.0f;
	m_fJumpJetSputterTime	= 10.0f;

//	DEVPRINTF( "Dive ending, speed = %0.2f, dist = %0.2f, decel = %0.2f\n", m_fSpeed_WS, _DIVE_DECEL_DIST, m_fDiveDecelSpeed );

	
	m_JumpJetState = JUMPJETS_DIVE_ENDING;
	ZeroTime( ANIMTAP_DIVE_RECOVER_UPPER );
	SetControlValue( ANIMCONTROL_DIVE_RECOVER_UPPER, 1.0f );
	SetControlValue( ANIMCONTROL_DIVE_RECOVER_LOWER, 1.0f );
	SetControlValue( ANIMCONTROL_DIVE, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY_JETS, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_LOWER, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND_UPPER, 0.0f );
}


void CBotJumper::_HandleDiveTranslation( void ) {
	CFVec3A vToTgt;
	f32		fSpd;

	if( IsPlayerBot() && m_pDiveTgtEntity ) {
		m_vDiveTgt = m_pDiveTgtEntity->MtxToWorld()->m_vPos;
	} else if( m_bDiveUseControls ) {
		m_vDiveTgt = m_TargetedPoint_WS;
	} else if(  m_XlatStickNormVecXZ_MS.MagSq() > 0.1f ) {
		m_bDiveUseControls = TRUE;
	}

	switch( m_JumpJetState ) {
		case JUMPJETS_DIVE_STARTING:
			// need to transition to the correct dive position and velocity

			// straighten out legs
			FMATH_BIPOLAR_CLAMPMAX( m_fLegsYaw_MS, _JUMPJETS_DIVE_TIME_START - m_fDiveTimer );
			vToTgt.Sub( m_vDiveTgt, m_MtxToWorld.m_vPos ).Unitize();
			vToTgt.Mul( _JUMPJETS_DIVE_STARTVELOCITY );
			FMATH_CLAMPMAX( vToTgt.y, 0.0f );

			if( m_fDiveTimer <= _JUMPJETS_DIVE_TIME_START ) {
				f32 fUnitTime = fmath_Div( m_fDiveTimer, _JUMPJETS_DIVE_TIME_START );
				fUnitTime = fmath_UnitLinearToSCurve( fUnitTime );
				
				// now interpolate between current velocity and desired velocity
				m_Velocity_WS.x = FMATH_FPOT( fUnitTime, m_Velocity_WS.x, vToTgt.x );
				m_Velocity_WS.y = FMATH_FPOT( fUnitTime, m_Velocity_WS.y, vToTgt.y );
				m_Velocity_WS.z = FMATH_FPOT( fUnitTime, m_Velocity_WS.z, vToTgt.z );
				VelocityHasChanged();
			}

			// update the speed necessary to stop
			m_fDiveDecelSpeed	= fmath_Div( m_fSpeed_WS * m_fSpeed_WS, _DIVE_DECEL_DIST * 2.0f );
			m_fDiveDistance		= 0.0f;

			_CheckForDiveCollision();

			break;

		case JUMPJETS_DIVING:
			// make sure the bot doesn't try to twist legs
			m_fLegsYaw_MS = 0.0f;

			if( m_pDiveTgtEntity ) {
				
				FASSERT( m_UnitVelocity_WS.MagSq() > 0.1f );

				f32 fMag2;
				CFVec3A vRotAxis;
				//CFVec3A vToTgt;

				vToTgt.Sub( m_pDiveTgtEntity->MtxToWorld()->m_vPos, m_MtxToWorld.m_vPos );
				if( vToTgt.SafeUnitAndMag( vToTgt ) < 0.1f ) {
					_EndDive( TRUE );
					return;
				}

				vRotAxis.Cross( m_UnitVelocity_WS, vToTgt );
				fMag2 = vRotAxis.MagSq();
				if( fMag2 < m_BotInfo_Jumper.fDiveTurnDegPerSecSinSq * FLoop_fPreviousLoopSecs * FLoop_fPreviousLoopSecs) {

					// close enough, just set it there
					m_Velocity_WS = vToTgt;
				} else {
					CFQuatA qTmp;
					vRotAxis.Mul( fmath_InvSqrt( fMag2 ) );
					qTmp.BuildQuat( vRotAxis, m_BotInfo_Jumper.fDiveTurnDegPerSec * FLoop_fPreviousLoopSecs );
					qTmp.MulPoint( m_Velocity_WS, m_UnitVelocity_WS );
				}
			} else {
				m_Velocity_WS.Sub( m_vDiveTgt, m_MtxToWorld.m_vPos );
				FMATH_CLAMPMAX( m_Velocity_WS.y, 0.0f );

				// see if at point or if point is behind us
				if( (m_Velocity_WS.MagSq() < 0.1f) || (m_Velocity_WS.Dot( m_MtxToWorld.m_vFront ) < 0.0f) ) {
					// passed through dive
					_EndDive( FALSE );	
					return;
				}
				m_Velocity_WS.Unitize();
			}

			

			// accelerate to dive velocity
			//m_Velocity_WS = vToTgt;
			if( m_fSpeed_WS < _DIVE_VELOCITY ) {
				m_fSpeed_WS += _DIVE_ACCEL * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fSpeed_WS, _DIVE_VELOCITY );
			}
			m_Velocity_WS.Mul( m_fSpeed_WS );

			_CheckForDiveCollision();

			// update the speed necessary to stop
			m_fDiveDecelSpeed = fmath_Div( m_fSpeed_WS * m_fSpeed_WS, _DIVE_DECEL_DIST * 2.0f );

			m_fDiveDistance += m_fSpeed_WS * FLoop_fPreviousLoopSecs;
			if( (m_fDiveDistance > _DIVE_MAX_DISTANCE) || (m_Velocity_WS.y > 0.0f) ) {
				_EndDive( FALSE );
			}
			break;


		case JUMPJETS_DIVE_ENDING:
			fSpd = m_Velocity_WS.SafeUnitAndMag( m_Velocity_WS );
			if( fSpd > 0.0f ) {
				fSpd -= m_fDiveDecelSpeed * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( fSpd, 5.0f );
				m_Velocity_WS.Mul( fSpd );
			}
			break;
	}
}


//static CFSphere  _dbgsphere;

void CBotJumper::_CheckForDiveCollision( void ) {
	CFCollData	colldata;
	CFSphere	collsphere;
	CFVec3A		vMovement = CFVec3A::m_Null;

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	colldata.nFlags						= FCOLL_DATA_IGNORE_BACKSIDE;
	colldata.nCollMask					= FCOLL_MASK_CHECK_ALL;
	colldata.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	colldata.pCallback					= NULL;//_DiveTrackerCB;
	colldata.pLocationHint				= m_pWorldMesh;
	colldata.pMovement					= &vMovement;
	colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
	colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;

	collsphere.m_fRadius = 3.0f;
	collsphere.m_Pos = m_Velocity_WS.v3;
	collsphere.m_Pos.Unitize().Mul( _DIVE_DECEL_DIST ).Add( m_MtxToWorld.m_vPos.v3 );
	
	////TEST ONLY
	//_dbgsphere = collsphere;
	//////

	FColl_nImpactCount	= 0;
	fcoll_Check( &colldata, &collsphere );
	if( FColl_nImpactCount > 0 ) {
		// hit something, decelerate
		_EndDive( TRUE );
	}
}

/*
BOOL CBotJumper::_DiveTrackerCB( CFWorldTracker *pTracker ) {
	for( u32 i=0; i < FWorld_nTrackerSkipListCount; i++ ) {
		if( FWorld_apTrackerSkipList[i] == pTracker ) {
			return FALSE;
		}
	}
	return TRUE;
}
*/

BOOL CBotJumper::NotifyBotCollision( CBot *pBot ) {
	if( IsDiving() && !m_pDiveTgtEntity ) {
		m_pDiveTgtEntity = pBot;

		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if( pDamageForm ) {
			pDamageForm->m_nDamageLocale	= CDamageForm::DAMAGE_LOCALE_AMBIENT;
			pDamageForm->m_nDamageDelivery	= CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile	= m_BotInfo_Jumper.apDamageProfileDive[ m_nPossessionPlayerIndex < 0 ? 0 : 1 ];
			pDamageForm->m_Damager.pWeapon	= NULL;
			pDamageForm->m_Damager.pBot		= this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_pDamageeEntity	= pBot; 

			CDamage::SubmitDamageForm( pDamageForm );

		}

//		DEVPRINTF( "Jumper hit bot:  %x during a dive\n", pBot );
	}
	return TRUE;
}


BOOL CBotJumper::WakeUp( void ) {
	if( !IsSleeping() ) {
		return FALSE;
	}

	if( m_nSleepState != BOTSLEEPSTATE_WAKE ) {
		ZeroTime( ANIMTAP_WAKE );
		m_nSleepState = BOTSLEEPSTATE_WAKE;
	}

	return TRUE;
}


void CBotJumper::_HandleSleeping( void ) {

	switch( m_nSleepState ) {
		case BOTSLEEPSTATE_INIT:
			SetControlValue( ANIMCONTROL_SLEEP, 1.0f );
			m_nSleepState = BOTSLEEPSTATE_DOZE_LOOP;
			_SetInverted( TRUE );
			fparticle_EnableEmission( m_hJetSmokeParticleLeft,	FALSE );
			fparticle_EnableEmission( m_hJetSmokeParticleRight, FALSE );

			ZeroTime( ANIMTAP_DROP );
			ZeroTime( ANIMTAP_WAKE );
			break;

		case BOTSLEEPSTATE_DOZE_LOOP:
			DeltaTime( ANIMTAP_SLEEP );
			break;

		case BOTSLEEPSTATE_WAKE:
			if( !Power_IsPoweredUp() || (m_nBotFlags & BOTFLAG_IS_BEING_SHOCKED) || (m_nBotFlags & BOTFLAG_IS_IMMOBILE) ) {
				if( m_nPowerState == POWERSTATE_POWERING_UP ) {
					
					SetControlValue( ANIMCONTROL_DROP, 1.0f );
				}
				break;
			}

			if( GetControlValue( ANIMTAP_DROP ) < 1.0f ) {
				// need to wake first
				SetControlValue( ANIMCONTROL_AWARE, 0.0f );
				SetControlValue( ANIMCONTROL_SLEEP, 0.0f );
				SetControlValue( ANIMCONTROL_WAKE, 1.0f );
				if( DeltaTime( ANIMTAP_WAKE, FLoop_fPreviousLoopSecs, TRUE ) ) {
					SetControlValue( ANIMCONTROL_WAKE, 0.0f );
					SetControlValue( ANIMCONTROL_DROP, 1.0f );
					ZeroTime( ANIMTAP_DROP );
				}
			} else {
				if( DeltaTime( ANIMTAP_DROP, FLoop_fPreviousLoopSecs/* * 2.0f*/, TRUE ) ) {
					SetControlValue( ANIMCONTROL_AWARE, 0.0f );
					SetControlValue( ANIMCONTROL_SLEEP, 0.0f );
					SetControlValue( ANIMCONTROL_DROP, 0.0f );
					SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
					ZeroTime( ANIMTAP_JUMP_FLY );
					m_nSleepState = BOTSLEEPSTATE_NONE;
					m_fUnitFlyBlend = 1.0f;
					_SetInverted( FALSE );
					fparticle_EnableEmission( m_hJetSmokeParticleLeft,	TRUE );
					fparticle_EnableEmission( m_hJetSmokeParticleRight, TRUE );
				}
				// give a little artificial accel down
				m_Velocity_WS.y -= 25.0f * FLoop_fPreviousLoopSecs;
			}

			break;
	}
}


void CBotJumper::_SetInverted( BOOL bInverted ) {
	FASSERT( GetControlValue( ASI_RC_POWER_UP ) == 0.0f );
	SetControlValue( ASI_RC_POWER_UP, 0.0f );
	FASSERT( GetControlValue( ASI_RC_POWER_DOWN ) == 0.0f );
	SetControlValue( ASI_RC_POWER_DOWN, 0.0f );

	if( bInverted ) {
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_INVERTED );
		m_anAnimStackIndex[ASI_RC_POWER_UP]		= ANIMTAP_RC_POWER_UP_INV;
		m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= ANIMTAP_RC_POWER_DOWN_INV;
	} else {
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_INVERTED );
		m_anAnimStackIndex[ASI_RC_POWER_UP]		= ANIMTAP_RC_POWER_UP;
		m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= ANIMTAP_RC_POWER_DOWN;
	}
}


void CBotJumper::Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier ) {
	CBot::Possess( nPlayerIndex, fPossessionArmorModifier );

	// if bot is inverted, set the mount pitch so that the camera is looking up at the bot
	if( m_nBotFlags & BOTFLAG_IS_INVERTED ) {
		m_fMountPitch_WS = -_INVERTED_POSSESS_PITCH;
	}

	// set up our HUD
	CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIndex);
	pHud->SetDrawFlags( CHud2::DRAW_LEFT_AMMOBOX | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR | CHud2::DRAW_SELFDESTRUCT | CHud2::DRAW_ENABLEOVERRIDE );
	CFColorRGBA startColor, endColor;
	startColor.OpaqueYellow();
	endColor.OpaqueRed();
	pHud->OverrideAmmoData( CHud2::LEFT, CHud2::OVERRIDE_METER_MIL, &m_fUnitJumpJetRecharge, TRUE, &startColor, &endColor );

	// have no weapon, so have to set this up manually
	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, _RETICLE_Y );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetType( CReticle::TYPE_DROID_STANDARD );
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.EnableDraw( TRUE );

		m_MeleeDataLeft.pDamageProfile = m_BotInfo_Jumper.apDamageProfileMelee[1];
		m_MeleeDataRight.pDamageProfile = m_BotInfo_Jumper.apDamageProfileMelee[1];
	} else {
		m_MeleeDataLeft.pDamageProfile = m_BotInfo_Jumper.apDamageProfileMelee[0];
		m_MeleeDataRight.pDamageProfile = m_BotInfo_Jumper.apDamageProfileMelee[0];
	}
}


void CBotJumper::_KillAllEmitters( void ) {
	fparticle_KillEmitter( m_hJetFireParticleLeft );
	m_hJetFireParticleLeft = FPARTICLE_INVALID_HANDLE;

	fparticle_KillEmitter( m_hJetSmokeParticleLeft );
	m_hJetSmokeParticleLeft = FPARTICLE_INVALID_HANDLE;

	fparticle_KillEmitter( m_hJetLaunchParticleLeft );
	m_hJetLaunchParticleLeft = FPARTICLE_INVALID_HANDLE;

	fparticle_KillEmitter( m_hJetFireParticleRight );
	m_hJetFireParticleRight = FPARTICLE_INVALID_HANDLE;

	fparticle_KillEmitter( m_hJetSmokeParticleRight );
	m_hJetSmokeParticleRight = FPARTICLE_INVALID_HANDLE;

	fparticle_KillEmitter( m_hJetLaunchParticleRight );
	m_hJetLaunchParticleRight = FPARTICLE_INVALID_HANDLE;
}


void CBotJumper::_CheckAndStartEmitter( FParticle_EmitterHandle_t *pEmitter, FParticle_DefHandle_t *pDefHandle, s32 nBoneIdx ) {
	FASSERT( *pDefHandle != FPARTICLE_INVALID_HANDLE );
	FASSERT( (nBoneIdx > -1) && (nBoneIdx < m_pWorldMesh->m_pMesh->nBoneCount) );
	
	if( *pEmitter != FPARTICLE_INVALID_HANDLE ) {
		return;
	}

	// start the particle emitter if possible
	*pEmitter = fparticle_SpawnEmitter( *pDefHandle, &(m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vPos.v3), &(m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vFront.v3), &(m_Velocity_WS.v3), 1.0f );
	if( *pEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_SetDirection( *pEmitter, &(m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vFront.v3) );
		fparticle_SetIntensity( *pEmitter, &m_fJetsParticleIntensity );		
	}
}

void CBotJumper::_StopEmitter( FParticle_EmitterHandle_t *pEmitter ) {
	if( *pEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( *pEmitter );
		*pEmitter = FPARTICLE_INVALID_HANDLE;
	}
}

f32 CBotJumper::ComputeEstimatedControlledStopTimeXZ( void ) {
	if( m_nState == STATE_GROUND ) {
		return CBot::ComputeEstimatedControlledStopTimeXZ();
	} else if( m_bAllowJumpTranslation ) {
		return fmath_Div( m_Velocity_MS.MagXZ(), m_BotInfo_Jumper.fMaxAirAccel * m_BotInfo_Jumper.fDecelMult );
	} else {
		return -1.0f;
	}
}


f32 CBotJumper::ComputeEstimatedControlledStopTimeY( void ) {
/*
	if( m_Velocity_MS.y > 0.0f ) {
		return fmath_Div( -m_Velocity_MS.y, m_BotInfo_Gen.fGravity );
	} else if( m_Velocity_MS.y < 0.0f ) {
		return fmath_Div( -m_Velocity_MS.y, m_BotInfo_Jumper.fJumpLaunchAccel );
	} else {
		return -1.0f;
	}
	*/

	return FMATH_FABS(fmath_Div( m_Velocity_MS.y, m_BotInfo_Jumper.fJumpLaunchVel ));
}

BOOL CBotJumper::_CheckForCeilingCollision( void ) {
	if( IsPlayerBot() && (m_Velocity_WS.y > 0.0f) ) {

		CFSphere sphere;
		sphere.m_Pos.Mul( CFVec3A::m_UnitAxisY.v3, 10.0f );
		sphere.m_Pos.Add( m_MtxToWorld.m_vPos.v3 );
		sphere.m_fRadius = 1.0f;

		FWorld_nTrackerSkipListCount = 0;
		AppendTrackerSkipList();

		CFCollData colldata;
		colldata.nFlags						= FCOLL_DATA_FLAGS_NONE;
		colldata.nCollMask					= FCOLL_MASK_COLLIDE_WITH_CAMERA;
		colldata.nStopOnFirstOfCollMask		= FCOLL_MASK_COLLIDE_WITH_CAMERA;
		colldata.pMovement					= NULL;
		colldata.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
		colldata.pCallback					= NULL;
		colldata.pLocationHint				= m_pWorldMesh;
		colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
		colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;
	
		fcoll_Clear();
		if( fcoll_Check( &colldata, &sphere ) ) {
			return TRUE;
		} else {
			return FALSE;
		}
	}

	return FALSE;
}

void CBotJumper::_MeleeSwing( void ) {
	FASSERT( m_pPartMgr && m_pPartMgr->IsCreated() );
	if( m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		MeleeSwing( &m_MeleeDataLeft );
	}

	if( m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT ) {
		MeleeSwing( &m_MeleeDataRight );
	}
}
     

/////////////////////////
//CHECKPOINT SAVE/RESTORE

void CBotJumper::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


void CBotJumper::CheckpointRestore( void ) {
	CBot::CheckpointRestore();
}


BOOL CBotJumper::CheckpointSave( void ) {
	return CBot::CheckpointSave();
}

//////////////////////
// DEBUG
#if 0
void CBotJumper::DebugDraw(  CBotJumper *pJumper  ) {
	return;
}
#endif


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotJumperBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CBotJumperBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTJUMPER, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotJumper::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotJumper::m_BotInfo_Gen.pszArmorProfile;

	// This class of bots can be recruited...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED );

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotJumperBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotJumperBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}
