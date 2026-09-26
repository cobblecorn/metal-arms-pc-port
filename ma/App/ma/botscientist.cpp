//////////////////////////////////////////////////////////////////////////////////////
// botscientist.cpp - mil scientist bot data
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
// 04/19/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botscientist.h"
#include "botanim.h"
#include "fmesh.h"
#include "fresload.h"
#include "fparticle.h"
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


#define _BOTINFO_FILENAME		( "b_scientist" )
#define _BOTPART_FILENAME		( "bp_sci" )
#define _MESH_NAME				( "GRMIscien00" )
#define _FIRE_THRESHOLD			( 0.25f )
#define _VALID_TGT_COS			( 0.45 )
#define _SFX_BANK				( "Scientist" )

static CBotScientistBuilder _BotScientistBuilder;

#define _DEATHPARTICLENAME ("Cryspart1")
FParticle_DefHandle_t	_DeathParticleDef;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotScientist
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL 									CBotScientist::m_bSystemInitialized		= FALSE;
u32										CBotScientist::m_nBotClassClientCount	= 0;


CBotScientist::BotInfo_Gen_t			CBotScientist::m_BotInfo_Gen;
CBotScientist::BotInfo_MountAim_t		CBotScientist::m_BotInfo_MountAim;
CBotScientist::BotInfo_Walk_t			CBotScientist::m_BotInfo_Walk;
CBotScientist::BotInfo_Jump_t			CBotScientist::m_BotInfo_Jump;
CBotScientist::BotInfo_Weapon_t			CBotScientist::m_BotInfo_Weapon;

CBotAnimStackDef						CBotScientist::m_AnimStackDef;
CBotScientist*							CBotScientist::m_pCBScientist			= NULL;
CFVec3A									CBotScientist::m_GroinVecY_WS;
CFBolt::CFBoltData						CBotScientist::m_BoltData;
FLinkRoot_t 							CBotScientist::m_ScientistList;
CBotPartPool*							CBotScientist::m_pPartPool				= NULL;

CBotScientist::BotInfo_Scientist_t		CBotScientist::m_BotInfo_Scientist;
//= {
//
//	1.0f,						//f32 fAttackTime;
//	2.0f,						//f32 fAttackRechargeTime;
//	100.0f,						// f32 fAttackMaxRange;
//	50.0f,						// f32 fHealMaxRange;
//	NULL,						// pBoltTex
//	NULL,						//CDamageProfile* 		pJoltDamageProfile;
//	NULL,						//CDamageProfile*			pHealDamageProfile;
//	FPARTICLE_INVALID_HANDLE,	//FParticle_DefHandle_t	hJoltParticleDef;
//	FPARTICLE_INVALID_HANDLE,	//FParticle_DefHandle_t	hHealParticleDef;
//	NULL,						//pSoundShock;
//	NULL,						//pSoundHeal;
//	NULL,						//pSoundHoverLoop;
//};


BOOL CBotScientist::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;

	flinklist_InitRoot( &m_ScientistList, FANG_OFFSETOF( CBotScientist, m_Link ) );
	
	return TRUE;
}


void CBotScientist::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	m_bSystemInitialized = FALSE;
}


BOOL CBotScientist::ClassHierarchyLoadSharedResources( void ) {
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

	// load sound bank
	if( !fresload_Load( FSNDFX_RESTYPE, _SFX_BANK ) ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _SFX_BANK );
	}


	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyLoadSharedResources():  Error parsing %s\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	// set up bolt data
	m_BoltData.m_TexInst.SetTexDef( m_BotInfo_Scientist.pBoltTex );
	m_BoltData.m_Color.Zero();												// will set this as needed
	m_BoltData.m_nMaxJointCount = 100;
	m_BoltData.m_nBoltCount = 10;
	m_BoltData.m_fDistBetweenJoints = 1.0f;
	m_BoltData.m_fTrumpetRadius = 3.0f;
	m_BoltData.m_fMinBoltThickness = 0.8f;
	m_BoltData.m_fMaxBoltThickness = 0.8f;
	m_BoltData.m_fMinBoltDisplacement = 0.2f;
	m_BoltData.m_fMaxBoltDisplacement = 1.0f;
	m_BoltData.m_fUnitEmitterPinch = 0.1f;
	m_BoltData.m_fUnitDistPinch = 0.25f;


	if( !_BuildAnimStackDef() ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyLoadSharedResources():  Error creating bot anim stack def\n" );
		goto _ExitWithError;
	}

	_DeathParticleDef  = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _DEATHPARTICLENAME );

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( frame );

	return FALSE;
}


void CBotScientist::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}

	m_AnimStackDef.Destroy();

	CBot::ClassHierarchyUnloadSharedResources();
}


CBotScientist::CBotScientist() : CBot() {
	m_pInventory	= NULL;
	m_pWorldMesh	= NULL;
}


CBotScientist::~CBotScientist() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotScientist::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotScientistBuilder *pBuilder = (CBotScientistBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotScientistBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTSCIENTIST );
	}

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


void CBotScientist::_ClearDataMembers( void ) {
	m_fCollCylinderHeight_WS	= 8.0f;
	m_fCollCylinderRadius_WS	= 2.25f;

	m_pBotInfo_Gen				= &m_BotInfo_Gen;
	m_pBotInfo_MountAim			= &m_BotInfo_MountAim;
	m_pBotInfo_Walk				= &m_BotInfo_Walk;
	m_pBotInfo_Jump				= &m_BotInfo_Jump;
	m_pBotInfo_Weapon			= &m_BotInfo_Weapon;
	
	m_fMaxFlatSurfaceSpeed_WS	= m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;
	m_fMountPitchMax_WS			= m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS			= m_pBotInfo_MountAim->fMountPitchUpLimit;
	m_pMoveIdentifier			= &m_fAimPitch_WS;

	m_anAnimStackIndex[ASI_STAND]						= ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_RC_TETHERED]					= ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]				= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]					= ANIMTAP_RC_POWER_UP;

	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_anEnableBoneNameIndexTableForSummer_Normal;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock	= m_anEnableBoneNameIndexTableForSummer_Normal;

	
	m_bUsingConsole			= FALSE;
	m_pTargetBot			= NULL;
	m_uNumDamagedBots		= 0;
	m_bHealing				= FALSE;
	m_pHoverAudioEmitter	= NULL;
}


BOOL CBotScientist::ClassHierarchyBuild( void ) {
	FMesh_t		*pMesh;
 	FMeshInit_t	meshInit;
	FResFrame_t	frame;
	s32 nBoneIndex;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	frame = fres_GetFrame();

	// get pointer to leaf class's builder object...
	CBotScientistBuilder *pBuilder = (CBotScientistBuilder*)GetLeafClassBuilder();

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
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Could not find mesh '%s'\n", _MESH_NAME );
		goto _ExitWithError;
	}

	// create worldmesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Unable to allocate CFWorldMesh\n" );
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
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Unable to create animation stack\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	//m_nBoneIdxGroin = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_WAIST] );
	//if( m_nBoneIdxBase < 0 ) {
	//	DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Unable to find groin bone %s in mesh\n", m_apszBoneNameTable[BONE_WAIST] );
	//	goto _ExitWithError;
	//}

	m_nBoneIdxTorso = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_TORSO] );
	if( m_nBoneIdxTorso < 0 ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Unable to find torso bone %s in mesh\n", m_apszBoneNameTable[BONE_TORSO] );
		goto _ExitWithError;
	}

	m_nBoneIdxBase = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_BASE] );
	if( m_nBoneIdxBase < 0 ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Unable to find base bone %s in mesh\n", m_apszBoneNameTable[BONE_BASE] );
		goto _ExitWithError;
	}

	m_nBoneIdxLHand = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_HAND_L] );
	if( m_nBoneIdxLHand < 0 ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Unable to find bone %s in mesh\n", m_apszBoneNameTable[BONE_HAND_L] );
		goto _ExitWithError;
	}

	m_nBoneIdxRHand = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_HAND_R] );
	if( m_nBoneIdxRHand < 0 ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Unable to find base bone %s in mesh\n", m_apszBoneNameTable[BONE_HAND_R] );
		goto _ExitWithError;
	}


	// set up bone callbacks...
	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();
	m_Anim.m_pAnimCombiner->SetBoneCallback( _AnimBoneCallback );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_nBoneIdxBase );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_nBoneIdxTorso );

	SetMaxHealth();

	//if( m_nPossessionPlayerIndex >= 0 ) {
	//	Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, _RETICLE_Y );
	//	Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetType( CReticle::TYPE_DROID_STANDARD );
	//	Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.EnableDraw( TRUE );

	//}

	// initialize part pool
	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		FASSERT_NOW;
		goto _ExitWithError;
	}
	m_pPartMgr->MakeLimbInvincible( LIMB_TYPE_HEAD );

	// Make invincible limbs...

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_TORSO ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotScientist::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_TORSO ] );
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

	m_fUnitFloatTimer = fmath_RandomFloat();
	m_fUnitFloatTimer2 = fmath_RandomFloat();

	//// Set up data port stuff...

	DataPort_SetupTetherShockInfo();

	flinklist_AddTail( &m_ScientistList, this );

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;

}


void CBotScientist::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	if( m_pWorldMesh ) {
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	CBot::ClassHierarchyDestroy();

	flinklist_Remove( &m_ScientistList, this );
}


void CBotScientist::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
	m_pWorldMesh->UpdateTracker();

	m_AttackState = ATTACKSTATE_NONE;
	m_fAttackTimer = 0.0f;
}


void CBotScientist::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();

	if( m_pHoverAudioEmitter ) {
		m_pHoverAudioEmitter->Destroy();
		m_pHoverAudioEmitter = NULL;
	}

	CBot::ClassHierarchyRemoveFromWorld();

}


BOOL CBotScientist::ClassHierarchyBuilt( void ) {
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


CEntityBuilder* CBotScientist::GetLeafClassBuilder( void ) {
	return &_BotScientistBuilder;
}


void CBotScientist::ClassHierarchyWork( void ) {
	FASSERT( IsSystemInitialized() );

	CFVec3A vTmp;

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	PROTRACK_BEGINBLOCK("Scientist");

	Power_Work();
	DataPort_Work();

	// take care of this here, because the bot doesn't handle ground anims
	if( IsImmobilizePending() ) {
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
	}


	ParseControls();

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

	//HandleHeadLook();

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
	
	// Move and animate our bot...
	switch( m_nState ) {
		case STATE_GROUND:
			PROTRACK_BEGINBLOCK("GroundXlat");
				HandleGroundTranslation();
				//_HandleJumping();
				//TEMP _HandleHopping();
			PROTRACK_ENDBLOCK();//"GroundXlat");

			//PROTRACK_BEGINBLOCK("GroundAnim");
			//	HandleGroundAnimations();
			//PROTRACK_ENDBLOCK();//"GroundAnim");
			break;


		case STATE_AIR:
			PROTRACK_BEGINBLOCK("AirXlat");
				HandleAirTranslation();
			PROTRACK_ENDBLOCK();//"AirXlat");

			PROTRACK_BEGINBLOCK("AirAnim");
				//if( HandleAirAnimations() ) {
				//	//_EnterFlyMode();
				//}
			PROTRACK_ENDBLOCK();//"AirAnim");
			break;
	}

	if( m_fSpeed_WS > 1.0f ) {
		m_bUsingConsole = FALSE;
	}
	_HandleConsoleAnimations();
	DeltaTime( ANIMTAP_STAND );


	m_TargetedPoint_WS = m_ControlBot_TargetPoint_WS;
	if( m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT ) {

		CFVec3A vToTgt;
		vToTgt.Sub( m_TargetedPoint_WS, m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vPos );
		
		f32 fMag2 = vToTgt.MagSq();
		if( fMag2 > 0.01f ) {
			vToTgt.Mul( fmath_InvSqrt( fMag2 ) );
			
			// see if we're within range
			if( fMag2 < m_BotInfo_Scientist.fAttackMaxRange * m_BotInfo_Scientist.fAttackMaxRange ) {

				if( vToTgt.Dot( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxTorso]->m_vFront ) < _VALID_TGT_COS ) {
					m_nBotFlags |= BOTFLAG_TARGET_LOCKED;
				}
			}
		}
	}

	_UpdateMatrices();

	_AttackWork();

	if( IsDeadOrDying() ) {
		DeathWork();
		if( m_pHoverAudioEmitter ) {
			m_pHoverAudioEmitter->Destroy();
			m_pHoverAudioEmitter = NULL;
		}
	} else {
		if( !m_pHoverAudioEmitter ) {
			m_pHoverAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Scientist.pSoundHoverLoop, FALSE, &m_MountPos_WS );
		} else {
			m_pHoverAudioEmitter->SetPosition( &m_MountPos_WS );
		}
	}

	PROTRACK_ENDBLOCK(); //"Scientist"
}


void CBotScientist::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}


const CFVec3A* CBotScientist::GetApproxEyePoint( void ) const {
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}


void CBotScientist::_UpdateMatrices( void ) {
	CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS );
	CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;

	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	//if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
		m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	//} else {
	//	m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	//}

	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );
}


void CBotScientist::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


void CBotScientist::CheckpointRestore( void ) {
	CBot::CheckpointRestore();
}


BOOL CBotScientist::CheckpointSave( void ) {
	return CBot::CheckpointSave();
}


#define _CONSOLE_ANIM_BLEND_ON	( 1.0f )
#define _CONSOLE_ANIM_BLEND_OFF	( 1.0f )

void CBotScientist::_HandleConsoleAnimations( void ) {
	f32 fVal;

	if( m_bUsingConsole ) {
		DeltaTime( ANIMTAP_CONSOLE );

		fVal = GetControlValue( ANIMCONTROL_CONSOLE );
		if( fVal < 1.0f ) {
			fVal += FLoop_fPreviousLoopSecs * _CONSOLE_ANIM_BLEND_ON;
			FMATH_CLAMP_MAX1( fVal );
			SetControlValue( ANIMCONTROL_CONSOLE, fVal );
		}

	} else {
		fVal = GetControlValue( ANIMCONTROL_CONSOLE );
		if( fVal > 0.0f ) {
			DeltaTime( ANIMTAP_CONSOLE );

			fVal -= FLoop_fPreviousLoopSecs * _CONSOLE_ANIM_BLEND_OFF;
			FMATH_CLAMP_MIN0( fVal );
			SetControlValue( ANIMCONTROL_CONSOLE, fVal );
		}
	}
}


void CBotScientist::_AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	FASSERT( m_pCBScientist && (m_pCBScientist->TypeBits() & ENTITY_BIT_BOTSCIENTIST) );
	if( !m_pCBScientist || !(m_pCBScientist->TypeBits() & ENTITY_BIT_BOTSCIENTIST) ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}

	if( m_pCBScientist->m_pPartMgr && m_pCBScientist->m_pPartMgr->IsCreated() ) {
		if( m_pCBScientist->m_pPartMgr->AnimBoneCallbackFunctionHandler( uBoneidx, rNewMtx, rParentMtx, rBoneMtx ) ) {
			return;
		}
	}

	//if( uBoneidx == (u32)m_pCBScientist->m_nBoneIdxGroin ) {
	//	// Groin
	//	rNewMtx.Mul( rParentMtx, rBoneMtx );
	//	m_GroinVecY_WS = rNewMtx.m_vUp;
	//} else
	
	if( uBoneidx == (u32)m_pCBScientist->m_nBoneIdxTorso ) {
		rNewMtx.Mul( rParentMtx, rBoneMtx );
//		// Torso
//		CFMtx43A WorldMtx;
//		CFQuatA Quat;
//
//		Quat.BuildQuat( m_GroinVecY_WS, -m_pCBScientist->m_fLegsYaw_MS );
//		WorldMtx.Mul( rParentMtx, rBoneMtx );
//
//		rNewMtx.m_vPos = WorldMtx.m_vPos;
//		Quat.MulPoint( rNewMtx.m_vRight, WorldMtx.m_vRight );
//		Quat.MulPoint( rNewMtx.m_vUp, WorldMtx.m_vUp );
//		Quat.MulPoint( rNewMtx.m_vFront, WorldMtx.m_vFront );
//
////		m_TorsoVecY_WS = rNewMtx.m_vUp;
//
	} else if( uBoneidx == (u32)m_pCBScientist->m_nBoneIdxBase ) {
		m_pCBScientist->_CalcBasePosition( rNewMtx, rParentMtx, rBoneMtx );
		m_GroinVecY_WS = rNewMtx.m_vUp;
	}
}



#define _MAX_FLOAT_HEIGHT		( 0.75f )
#define _MIN_FLOAT_HEIGHT		( 0.5f )
#define _FLOAT_RANGE			( _MAX_FLOAT_HEIGHT - _MIN_FLOAT_HEIGHT )
#define _MID_FLOAT_HEIGHT		(_MIN_FLOAT_HEIGHT + (0.5f*_FLOAT_RANGE))
#define _FLOAT_PERIOD			( 3.0f )
#define _OO_FLOAT_PERIOD		( 1.0f/_FLOAT_PERIOD )
#define _RANDOM_TIMER_ADJ		( 0.1f )

#define _MAX_FLOAT_HEIGHT2		( 0.4f )
#define _MIN_FLOAT_HEIGHT2		( 0.2f )
#define _FLOAT_RANGE2			( _MAX_FLOAT_HEIGHT2 - _MIN_FLOAT_HEIGHT2 )
#define _MID_FLOAT_HEIGHT2		(_MIN_FLOAT_HEIGHT2 + (0.5f*_FLOAT_RANGE2))
#define _FLOAT_PERIOD2			( 1.0f )
#define _OO_FLOAT_PERIOD2		( 1.0f/_FLOAT_PERIOD )
#define _RANDOM_TIMER_ADJ2		( 0.1f )


void CBotScientist::_CalcBasePosition( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	f32 fHeight;
	f32 fMaxHeight;

	m_fUnitFloatTimer += FLoop_fPreviousLoopSecs * _OO_FLOAT_PERIOD;
	m_fUnitFloatTimer2 += FLoop_fPreviousLoopSecs * _OO_FLOAT_PERIOD2;

	if( m_fUnitFloatTimer > 1.0f ) {
		m_fUnitFloatTimer -= 1.0f;
//		m_fUnitFloatTimer += fmath_RandomFloat() * _RANDOM_TIMER_ADJ;
	}

	if( m_fUnitFloatTimer2 > 1.0f ) {
		m_fUnitFloatTimer2 -= 1.0f;
//		m_fUnitFloatTimer2 += fmath_RandomFloat() * _RANDOM_TIMER_ADJ;
	}

	fHeight = _MID_FLOAT_HEIGHT + (_FLOAT_RANGE * fmath_Cos( FMATH_2PI * m_fUnitFloatTimer )) + (_MID_FLOAT_HEIGHT2 + (_FLOAT_RANGE2 * fmath_Cos( FMATH_2PI * m_fUnitFloatTimer2 )));

	// handle powered down states:
	if( Power_IsPoweredDown() ) {
		fHeight = _MIN_FLOAT_HEIGHT + _MIN_FLOAT_HEIGHT2;
	} else if( Power_IsPoweringDown() ) {
		fMaxHeight = _MIN_FLOAT_HEIGHT + _MIN_FLOAT_HEIGHT2 + ((_FLOAT_RANGE + _FLOAT_RANGE2) * (1.0f - GetControlValue( ANIMCONTROL_RC_POWER_DOWN )));
		FMATH_CLAMPMAX( fHeight, fMaxHeight );

	} else if( Power_IsPoweringUp() ) {
		fMaxHeight = _MIN_FLOAT_HEIGHT + _MIN_FLOAT_HEIGHT2 + ((_FLOAT_RANGE + _FLOAT_RANGE2) * (1.0f - GetControlValue( ANIMCONTROL_RC_POWER_UP )));
		FMATH_CLAMPMAX( fHeight, fMaxHeight );
	}

	if( !Power_IsPoweredUp() ) {
		f32 fMaxHeight;
		fMaxHeight = _MIN_FLOAT_HEIGHT + (_FLOAT_RANGE + _FLOAT_RANGE2) * (1.0f - FMATH_MAX( GetControlValue( ANIMCONTROL_RC_POWER_UP ), GetControlValue( ANIMCONTROL_RC_POWER_DOWN )));
        FMATH_CLAMPMAX( fHeight, fMaxHeight );
	}

	rNewMtx.Mul( rParentMtx, rBoneMtx );
	rNewMtx.m_vPos.y += fHeight;
}


#define _ATTACK_ANIM_ON_RATE		( 2.0f )
#define _ATTACK_ANIM_OFF_RATE		( 2.0f )


void CBotScientist::_AttackWork( void ) {
	f32 fVal;


#if 0 && SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	if( IsPlayerBot() ) {
		m_TargetedPoint_WS.Mul( m_MtxToWorld.m_vFront, 25.0f );
		m_TargetedPoint_WS.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxRHand]->m_vPos );
	}
	fdraw_DevLine( &(m_TargetedPoint_WS.v3), &(m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxRHand]->m_vPos.v3), &FColor_MotifGreen, &FColor_MotifGreen );
	fdraw_DevSphere( &(m_TargetedPoint_WS.v3), 2.0f, &FColor_MotifGreen );
#endif

	switch( m_AttackState ) {
	case ATTACKSTATE_NONE:
		if( m_fControls_Fire1 > _FIRE_THRESHOLD ) {
			m_bHealing = FALSE;
			m_AttackState = ATTACKSTATE_STARTING;
		}

		if( m_fControls_Fire2 > _FIRE_THRESHOLD ) {
			m_bHealing = TRUE;
			m_AttackState = ATTACKSTATE_STARTING;
		}
		break;

	case ATTACKSTATE_STARTING:
        DeltaTime( ANIMTAP_ATTACK );
		fVal = GetControlValue( ANIMCONTROL_ATTACK );
		fVal += FLoop_fPreviousLoopSecs * _ATTACK_ANIM_ON_RATE;
		if( fVal < 1.0f ) {
			SetControlValue( ANIMCONTROL_ATTACK, fVal );

		} else {
			SetControlValue( ANIMCONTROL_ATTACK, 1.0f );
			if( _BeginAttack() ) {
				m_AttackState = ATTACKSTATE_ATTACKING;
			} else if( m_bHealing && m_fControls_Fire2 < _FIRE_THRESHOLD ) {
				m_AttackState = ATTACKSTATE_STOPPING;
			} else if( !m_bHealing && m_fControls_Fire1 < _FIRE_THRESHOLD ) {
				m_AttackState = ATTACKSTATE_STOPPING;
			}
		}
		break;

	case ATTACKSTATE_ATTACKING:
		DeltaTime( ANIMTAP_ATTACK );
		m_fAttackTimer -= FLoop_fPreviousLoopSecs;
		if( (m_fAttackTimer <= 0.0f) || !_FindAttackTarget() ) {
            _EndAttack();
			m_AttackState = ATTACKSTATE_STOPPING;
		}

		break;

	case ATTACKSTATE_STOPPING:
		DeltaTime( ANIMTAP_ATTACK );
		fVal = GetControlValue( ANIMCONTROL_ATTACK );
		fVal -= FLoop_fPreviousLoopSecs * _ATTACK_ANIM_ON_RATE;
		if( fVal > 0.0f ) {
			SetControlValue( ANIMCONTROL_ATTACK, fVal );
		} else {
			SetControlValue( ANIMCONTROL_ATTACK, 0.0f );
			m_AttackState = ATTACKSTATE_RECHARGING;
		}
		break;

	case ATTACKSTATE_RECHARGING:
		m_fAttackTimer -= FLoop_fPreviousLoopSecs;
		if( m_fAttackTimer <= 0.0f ) {
			m_AttackState = ATTACKSTATE_NONE;
		}
		break;
	}
}


BOOL CBotScientist::_BeginAttack( void ) {
	m_uNumDamagedBots = 0;

	//if( !(m_nBotFlags & BOTFLAG_TARGET_LOCKED) ) {
	//	return FALSE;
	//}

	m_pTargetBot = NULL;

	_FindAttackTarget();

	if( !m_pTargetBot ) {
		return FALSE;
	}

	if( m_bHealing ) {
		PlaySound( m_BotInfo_Scientist.pSoundHeal );
	} else {
		PlaySound( m_BotInfo_Scientist.pSoundShock );
	}

	m_fAttackTimer = m_BotInfo_Scientist.fAttackTime;
	return TRUE;
}


void CBotScientist::SpawnDeathEffects(void)
{	//no death anim, so just kick of particles or something magical like a teleport effect
//	fparticle_SpawnEmitter( _DeathParticleDef, m_MtxToWorld.m_vPos.v3, NULL); 
}

void CBotScientist::_EndAttack( void ) {
	m_fAttackTimer = m_BotInfo_Scientist.fAttackRechargeTime;
	m_pTargetBot = NULL;
}

#define _ATTACK_SPHERE_RADIUS	( 0.5f )
#define _ATTACK_MAX_COS			( 0.2f )

BOOL CBotScientist::_FindAttackTarget( void ) {
	f32 fMag2;
	CFVec3A vToTgt;
	CEntity *pDamagedEntity;
	f32 fMaxRangeSq = m_bHealing ? m_BotInfo_Scientist.fHealMaxRange : m_BotInfo_Scientist.fAttackMaxRange;
	fMaxRangeSq *= fMaxRangeSq;

	if( (m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_PRIMARY ) != CBotPartMgr::COMPONENT_STATUS_SOME_FULLY_OPERATIONAL) &&
		(m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_PRIMARY ) != CBotPartMgr::COMPONENT_STATUS_ALL_FULLY_OPERATIONAL) ) {
		return FALSE;
	}

	CFVec3A vAttackOrigin;
	vAttackOrigin.Mul( m_MtxToWorld.m_vUp, 8.0f );
	vAttackOrigin.Add( m_MtxToWorld.m_vPos );
    
	if( m_pTargetBot ) {
		vToTgt.Sub( m_pTargetBot->MtxToWorld()->m_vPos, vAttackOrigin );
	} else {
		vToTgt.Sub( m_TargetedPoint_WS, vAttackOrigin );
	}

	
	fMag2 = vToTgt.MagSq();
	if( fMag2 > fMaxRangeSq ) {
		m_pTargetBot = NULL;
		return FALSE;
	}

	if( vToTgt.Dot( m_MountUnitFrontXZ_WS ) * fmath_InvSqrt( fMag2 ) < _ATTACK_MAX_COS ) {
		return FALSE;
	}

    CFCollData colldata;
	CFSphere sphere;

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	colldata.nCollMask					= FCOLL_MASK_COLLIDE_WITH_NPCS | FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	colldata.nFlags						= FCOLL_DATA_FLAGS_NONE;
	colldata.nStopOnFirstOfCollMask		= FCOLL_MASK_NONE; //FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	colldata.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	colldata.pCallback					= _TrackerCollCB;
	colldata.pLocationHint				= m_pWorldMesh;
	colldata.pMovement					= &vToTgt;
	colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
	colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;
	
	sphere.Set( vAttackOrigin.v3, _ATTACK_SPHERE_RADIUS );

	fcoll_Clear();

	m_pCBScientist = this;
	if( fcoll_Check( &colldata, &sphere ) ) {
		fcoll_SortDynamic();
		pDamagedEntity = CGColl::ExtractEntity( FColl_apSortedImpactBuf[0] );
		if( pDamagedEntity && pDamagedEntity->TypeBits() & ENTITY_BIT_BOT ) {
			_DamageBot( (CBot*)pDamagedEntity );
		}
	}
	m_pCBScientist = NULL;

	return TRUE;
}


// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void CBotScientist::DrawAllFX( void ) {
	CBotScientist *pScientist;

	for( pScientist=(CBotScientist*)flinklist_GetHead( &m_ScientistList ); pScientist; pScientist = (CBotScientist*)flinklist_GetNext( &m_ScientistList, pScientist ) ) {
		if( pScientist->m_AttackState == ATTACKSTATE_ATTACKING ) {
			if (pScientist->m_pTargetBot ) {
				if( pScientist->m_bHealing ) {
					m_BoltData.m_Color.Set( 0.188f, 1.0f, 0.796f, 1.0f );
				} else {
					m_BoltData.m_Color.Set( 0.188f, 0.592f, 1.0f, 1.0f );
				}

				m_BoltData.m_EndPos_WS = *pScientist->m_pTargetBot->GetTagPoint( 0 );

				if( pScientist->m_pPartMgr->GetLimbState( LIMB_TYPE_LEFT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT ) {
					m_BoltData.m_StartPos_WS = pScientist->m_pWorldMesh->GetBoneMtxPalette()[pScientist->m_nBoneIdxLHand]->m_vPos;
					CFBolt::Draw( &m_BoltData );
				}

				if( pScientist->m_pPartMgr->GetLimbState( LIMB_TYPE_RIGHT_ARM ) == CBotPartMgr::LIMB_STATE_INTACT ) {
					m_BoltData.m_StartPos_WS = pScientist->m_pWorldMesh->GetBoneMtxPalette()[pScientist->m_nBoneIdxRHand]->m_vPos;
					CFBolt::Draw( &m_BoltData );
				}
			}
		}
	}
}


u32 CBotScientist::_TrackerCollCB( CFWorldTracker *pTracker ) {
	return FCOLL_CHECK_CB_FIRST_IMPACT_ONLY;	// check this object
}


void CBotScientist::_DamageBot( CBot *pBot ) {
	u32 i;
	m_pTargetBot = pBot;

	if( m_uNumDamagedBots == MAX_DAMAGE_BOTS ) {
		// damaged all we can
		return;
	}

	for( i=0; i<m_uNumDamagedBots; i++ ) {
		if( m_apDamagedBots[i] == m_pTargetBot ) {
			// already damaged this guy this attack, bail out
			return;
		}
	}
	
	// damaging a new bot, turn off tethershock on all the other bots
	for( i=0; i<m_uNumDamagedBots; i++ ) {
		if( !(m_apDamagedBots[i]->IsDeadOrDying() ) ) {
			m_apDamagedBots[i]->DataPort_Shock( FALSE );
		}
	}

	m_apDamagedBots[m_uNumDamagedBots++] = m_pTargetBot;

	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
	if( pDamageForm ) {
		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_Damager.pWeapon = NULL;
		pDamageForm->m_Damager.pBot = this;
		pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
		pDamageForm->m_pDamageeEntity = pBot;
		if( m_bHealing ) {
			pDamageForm->m_pDamageProfile = m_BotInfo_Scientist.pHealDamageProfile;
			fparticle_SpawnEmitter( m_BotInfo_Scientist.hHealParticleDef, &(m_pTargetBot->GetApproxEyePoint()->v3), &(CFVec3A::m_UnitAxisY.v3), &(m_Velocity_WS.v3), 1.0f );
			m_pTargetBot->RepairAllLimbs();
		} else {
			pDamageForm->m_pDamageProfile = m_BotInfo_Scientist.pJoltDamageProfile;
			fparticle_SpawnEmitter( m_BotInfo_Scientist.hJoltParticleDef, &(m_pTargetBot->GetApproxEyePoint()->v3), &(CFVec3A::m_UnitAxisY.v3), &(m_Velocity_WS.v3), 1.0f );
			if( !pBot->IsDeadOrDying() ) {
				pBot->DataPort_Shock( TRUE, m_BotInfo_Scientist.fAttackTime );
			}
		}

		CDamage::SubmitDamageForm( pDamageForm );
	}

}





















//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotScientistBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CBotScientistBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTSCIENTIST, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotScientist::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotScientist::m_BotInfo_Gen.pszArmorProfile;

	 // This class of bots can be recruited...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED | BOT_BUILDER_DPORT_NOT_NEEDED_FOR_RECRUITMENT );

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotScientistBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotScientistBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}
