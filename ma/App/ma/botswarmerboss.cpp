//////////////////////////////////////////////////////////////////////////////////////
// botswarmerboss.cpp - Swarmer boss bot data
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
// 03.31.03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////


#if 0

#include "fang.h"
#include "fanim.h"
#include "botswarmerboss.h"
#include "botanim.h"
#include "fmesh.h"
#include "fresload.h"
#include "meshtypes.h"
#include "protrack.h"

//METMP
#include "fdraw.h"
#include "picklevel.h"
#include "sas_user.h"

#define _BOTINFO_FILENAME			( "b_SBoss" )
#define _MESH_NAME					( "grmaswbos00" )


static CBotSwarmerBossBuilder _BotSwarmerBossBuilder;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotJumper
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL 									CBotSwarmerBoss::m_bSystemInitialized	= FALSE;
u32										CBotSwarmerBoss::m_nBotClassClientCount	= 0;

CBotSwarmerBoss::BotInfo_Gen_t			CBotSwarmerBoss::m_BotInfo_Gen;
CBotSwarmerBoss::BotInfo_MountAim_t		CBotSwarmerBoss::m_BotInfo_MountAim;
CBotSwarmerBoss::BotInfo_Walk_t			CBotSwarmerBoss::m_BotInfo_Walk;
CBotSwarmerBoss::BotInfo_Jump_t			CBotSwarmerBoss::m_BotInfo_Jump;
CBotSwarmerBoss::BotInfo_Weapon_t		CBotSwarmerBoss::m_BotInfo_Weapon;
CBotSwarmerBoss::BotInfo_SwarmerBoss_t	CBotSwarmerBoss::m_BotInfo_SwarmerBoss;

CBotPartPool*							CBotSwarmerBoss::m_pPartPool			= NULL;
CBotSwarmerBoss*						CBotSwarmerBoss::m_pCBSBoss				= NULL;

CBotAnimStackDef						CBotSwarmerBoss::m_AnimStackDef;


////////////////////////////
//STATIC FNs

BOOL CBotSwarmerBoss::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;
	
	return TRUE;
}


void CBotSwarmerBoss::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	FASSERT( m_nBotClassClientCount == 0 );

	m_bSystemInitialized = FALSE;
}



/////////////////////////////
//CLASS HIERARCHY FNs


BOOL CBotSwarmerBoss::ClassHierarchyLoadSharedResources( void ) {
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
	// FTexDef_t *pTexDef;

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources():  Error parsing %s\n", _BOTINFO_FILENAME );
		goto _ExitWithError;
	}

	if( !_BuildAnimStackDef() ) {
		DEVPRINTF( "CBotJumper::ClassHierarchyLoadSharedResources():  Error creating bot anim stack def\n" );
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


void CBotSwarmerBoss::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) {
		return;
	}


	m_AnimStackDef.Destroy();

	CBot::ClassHierarchyUnloadSharedResources();
}


BOOL CBotSwarmerBoss::ClassHierarchyBuild( void ) {
	FMesh_t		*pMesh;
	FMeshInit_t	meshInit;
	FResFrame_t	frame;
//	s32 nBoneIndex;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	frame = fres_GetFrame();

	// get pointer to leaf class's builder object...
	CBotSwarmerBossBuilder *pBuilder = (CBotSwarmerBossBuilder*)GetLeafClassBuilder();

	// set input params for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// build parent...
	if( !CBot::ClassHierarchyBuild() ) {
		goto _ExitWithError;
	}

	// precalc this
	m_fOOGravity = fmath_Inv( m_BotInfo_Gen.fGravity );
	m_nBotFlags |= BOTFLAG_HIPSLACK_OVERRIDE;

	// set defaults...
	_ClearDataMembers();

	// load mesh...
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _MESH_NAME );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBotSwarmerBoss::ClassHierarchyBuild(): Could not find mesh '%s'\n", _MESH_NAME );
		goto _ExitWithError;
	}

	// create worldmesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotSwarmerBoss::ClassHierarchyBuild(): Unable to allocate CFWorldMesh\n" );
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
		DEVPRINTF( "CBotSwarmerBoss::ClassHierarchyBuild(): Unable to create animation stack\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_STAND, 1.0f );
	UpdateUnitTime( ANIMTAP_STAND, fmath_RandomFloat() );

	// set up bone callbacks...

	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();

	SetMaxHealth();

	//// initialize part pool
	//if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
	//	FASSERT_NOW;
	//	goto _ExitWithError;
	//}

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


void CBotSwarmerBoss::ClassHierarchyDestroy( void ) {
	m_Anim.Destroy();

	if( m_pWorldMesh ) {
		fdelete( m_pWorldMesh );
		m_pWorldMesh = NULL;
	}

	CBot::ClassHierarchyDestroy();
}


void CBotSwarmerBoss::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();

	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
	m_pWorldMesh->UpdateTracker();
	m_MountPos_WS = m_MtxToWorld.m_vPos;
	m_vLastWallMountPos = m_MtxToWorld.m_vPos;
}



void CBotSwarmerBoss::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();

	CBot::ClassHierarchyRemoveFromWorld();
}


BOOL CBotSwarmerBoss::ClassHierarchyBuilt( void ) {
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


void CBotSwarmerBoss::_NormalMoveWork( void ) {
	CFVec3A vTmp;
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

// head?
//	HandleHeadLook();

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

	if( m_uCollisionStatus & (COLLSTATE_FLOOR | COLLSTATE_WALL) ) {
		m_nState = STATE_GROUND;
	}



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
		//HandleHipsAnimation();
		//_HandleAimAnimations();
	PROTRACK_ENDBLOCK();//"CommonAnim");


}


void CBotSwarmerBoss::_WallWork( void ) {
	f32 fDX;
	fDX = m_vLastWallMountPos.Dist( m_MountPos_WS );

	m_Velocity_WS.Sub( m_MountPos_WS, m_vLastWallMountPos );
	m_Velocity_WS.Mul( FLoop_fPreviousLoopOOSecs );

	m_vLastWallMountPos	= m_MountPos_WS;

	VelocityHasChanged();
	m_fNormSpeedAlongSurface_WS = m_Velocity_WS.Mag();
	m_fSpeedAlongSurface_WS = m_Velocity_WS.Mag();
	HandleGroundAnimations();
}


void CBotSwarmerBoss::ClassHierarchyWork( void ) {
	FASSERT( IsSystemInitialized() );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	PROTRACK_BEGINBLOCK("SwarmerBoss");

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	m_BotInfo_Gen.fCollSphere1Radius_MS = 4.f;
	m_pBotInfo_Walk->fSteepSlopeUnitNormY = -1.0f;
#endif

//	Power_Work();
//	DataPort_Work();

	
	switch( m_eMoveState ) {
		case MOVESTATE_WALL:
			_WallWork();
			break;

		case MOVESTATE_NORMAL:
			_NormalMoveWork();
			break;

		case MOVESTATE_JUMPING_TO_FLOOR:
			_JumpToFloorWork();
			break;

		case MOVESTATE_JUMPING_TO_WALL:
			break;
	}


	PROTRACK_BEGINBLOCK("Weapon");
		//HandleTargeting();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

	_UpdateMatrices();

	if( IsDeadOrDying() ) {
		DeathWork();
	}

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	_DebugFn();
#endif
	

	PROTRACK_ENDBLOCK(); //"SwarmerBoss"
}


////////////////////
//PUBLICS

CBotSwarmerBoss::CBotSwarmerBoss() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


CBotSwarmerBoss::~CBotSwarmerBoss() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotSwarmerBoss::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotSwarmerBossBuilder *pBuilder = (CBotSwarmerBossBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotSwarmerBossBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTSWARMERBOSS );
	}

	// Create an entity...
	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


CEntityBuilder* CBotSwarmerBoss::GetLeafClassBuilder( void ) {
	return &_BotSwarmerBossBuilder;
}


void CBotSwarmerBoss::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}

// Used by the bottalk system
void CBotSwarmerBoss::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
	//static const u8 *__apnUserAnimBoneMaskIndex[UABONEMASK_COUNT] = {
	//	m_aBoneEnableIndices_UserAnim_UpperBody,
	//	m_aBoneEnableIndices_UserAnim_LowerBody,
	//	m_aBoneEnableIndices_UserAnim_UpperTorso,
	//	m_aBoneEnableIndices_UserAnim_LowerTorso,
	//	m_aBoneEnableIndices_UserAnim_LeftArm,
	//	m_aBoneEnableIndices_UserAnim_RightArm,
	//	m_aBoneEnableIndices_UserAnim_Head
	//};

	//FASSERT( IsCreated() );
	//FASSERT( nBoneMaskGroup>=0 && nBoneMaskGroup<UABONEMASK_COUNT );

	//CBot::UserAnim_BatchUpdateTapBoneMask( __apnUserAnimBoneMaskIndex[nBoneMaskGroup], m_apszBoneNameTable );
}



///////////////////
//PRIVATES

void CBotSwarmerBoss::_ClearDataMembers( void ) {
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
	m_pMoveIdentifier			= &m_pWorldMesh;

	m_anAnimStackIndex[ASI_STAND]				= ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT]			= -1;
	m_anAnimStackIndex[ASI_WALK]				= ANIMTAP_WALK;
	m_anAnimStackIndex[ASI_WALK_ALERT]			= -1;
	m_anAnimStackIndex[ASI_RUN]					= -1;
	m_anAnimStackIndex[ASI_RUN_PANIC]			= -1;
	m_anAnimStackIndex[ASI_FALL]				= ANIMTAP_JUMP_FLY;
	m_anAnimStackIndex[ASI_RC_TETHERED]			= -1;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]		= -1;
	m_anAnimStackIndex[ASI_RC_POWER_UP]			= -1;
	m_anAnimStackIndex[ASI_STOOP]				= -1; //ANIMTAP_STOOP_SUMMER;
	m_anAnimStackIndex[ASI_STAND_LIMP_LEFT]		= -1; //ANIMTAP_STAND_LIMP_LEFT;
	m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT]	= -1; //ANIMTAP_STAND_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_LIMP_LEFT]			= -1; //ANIMTAP_LIMP_LEFT;
	m_anAnimStackIndex[ASI_LIMP_RIGHT]			= -1; //ANIMTAP_LIMP_RIGHT;
	m_anAnimStackIndex[ASI_HOP_LEFT]			= -1;
	m_anAnimStackIndex[ASI_HOP_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_STARTLE]				= -1;
	m_anAnimStackIndex[ASI_ROLL_LEFT]			= -1;
	m_anAnimStackIndex[ASI_ROLL_RIGHT]			= -1;
	m_anAnimStackIndex[ASI_DOZE_LOOP]			= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_NAPJERK]				= -1; //ANIMTAP_SLEEP;
	m_anAnimStackIndex[ASI_WAKE]				= -1; //ANIMTAP_WAKE;
	
	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_anEnableBoneNameIndexTableForSummer_Normal;
}


void CBotSwarmerBoss::_UpdateMatrices( void ) {

	if( m_eMoveState == MOVESTATE_NORMAL ) {
		CFMtx43A::m_XlatRotY.SetRotationY( m_fMountYaw_WS + m_fLegsYaw_MS );
		CFMtx43A::m_XlatRotY.m_vPos = m_MountPos_WS;
		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
		m_pWorldMesh->UpdateTracker();
	} else {
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
		m_pWorldMesh->UpdateTracker();
	}

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	//if( (m_pPartMgr == NULL) || !m_pPartMgr->IsCreated() || (m_pPartMgr->GetLimbState( LIMB_TYPE_HEAD ) == CBotPartMgr::LIMB_STATE_INTACT) ) {
	//	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );
	//} else {
		m_GazeUnitVec_WS = m_MtxToWorld.m_vFront;
	//}

	if( m_eMoveState == MOVESTATE_NORMAL ) {
		CFMtx43A::m_Temp.Identity();
		CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
		CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
		Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );
	} else {
		//Relocate_RotXlatFromUnitMtx_WS( &(CFMtx43A::m_Temp), TRUE, m_pMoveIdentifier );
	}

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	CFVec3A vTmp;
	vTmp.Mul( m_MtxToWorld.m_vRight, 1000.0f ).Add( m_MtxToWorld.m_vPos );
//	fdraw_DevLine( &m_MtxToWorld.m_vPos.v3, &vTmp.v3, &FColor_MotifRed, &FColor_MotifRed );
#endif
	
}


void CBotSwarmerBoss::_StartSingleJump( const CFVec3A *pvJumpVec /*=NULL*/ ) {
	if( !pvJumpVec ) {
		m_Velocity_MS.y += m_pBotInfo_Jump->fVerticalVelocityJump1 * m_fJumpMultiplier;
		MS2WS( m_Velocity_WS, m_Velocity_MS );
	} else {
		m_Velocity_WS.Set( *pvJumpVec );	
		MS2WS( m_Velocity_MS, m_Velocity_WS );
	}
	
	VelocityHasChanged();

	m_bPlayLandAnim = FALSE;

    SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

	ZeroTime( ANIMTAP_JUMP_LAUNCH );

	m_nJumpState = BOTJUMPSTATE_LAUNCH;
	SetJumping();
}


void CBotSwarmerBoss::_EnterFlyMode( void ) {
	m_bPlayLandAnim = TRUE;

	SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
	SetControlValue( ANIMCONTROL_JUMP_FLY, 1.0f );
	SetControlValue( ANIMCONTROL_JUMP_LAND, 0.0f );
	
	m_nJumpState = BOTJUMPSTATE_AIR;
	SetJumping();
}


void CBotSwarmerBoss::_HandleJumping( void ) {
	if( m_bControlsBot_JumpVec ) {
		_StartSingleJump( &m_ControlsBot_JumpVelocity_WS );
	} else if( m_bControls_Jump ) {
		_StartSingleJump();
	}
}


void CBotSwarmerBoss::_HandleJumpAnimations( void ) {
	f32 fUnitTime;

	if( m_nState == STATE_GROUND ) {
		if( m_fUnitFlyBlend > 0.0f ) {
			m_fUnitFlyBlend = 0.0f;			//ME:  wtf is this?
			_JumpLanded();
		}
	
		if( m_bPlayLandAnim ) {
			if( !DeltaTime( ANIMTAP_JUMP_LAND, FLoop_fPreviousLoopSecs ) ) {
			// Still playing landing animation...
				f32 fUnitTime = 1.01f - GetUnitTime(ANIMTAP_JUMP_LAND);
				fUnitTime = m_fMaxLandUnitBlend * fmath_UnitLinearToSCurve( fUnitTime );
				SetControlValue( ANIMCONTROL_JUMP_LAND, fUnitTime );
				SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );

			} else {
				// done playing animation
   				m_bPlayLandAnim = FALSE;
				m_bPlayLandAnimLower = FALSE;

				SetControlValue( ANIMCONTROL_JUMP_LAND, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_LAUNCH, 0.0f );
				SetControlValue( ANIMCONTROL_JUMP_FLY, 0.0f );
			}

		} else {
			if( m_nJumpState == BOTJUMPSTATE_NONE ) {
				fUnitTime = GetControlValue( ANIMCONTROL_JUMP_LAND );
				if( fUnitTime > 0.0f ) {
					fUnitTime -= FLoop_fPreviousLoopSecs * 3.0f;
					FMATH_CLAMP_MIN0( fUnitTime );
				}
			}
		}
	} else {
		// still in the air
		if( m_nJumpState != BOTJUMPSTATE_NONE ) {

			switch( m_nJumpState ) {
				case BOTJUMPSTATE_LAUNCH:
					if( !DeltaTime( ANIMTAP_JUMP_LAUNCH ) ) {
						fUnitTime = 3.0f * fmath_UnitLinearToSCurve( GetUnitTime( ANIMTAP_JUMP_LAUNCH ) );
						FMATH_CLAMPMAX( fUnitTime, 1.0f );
						SetControlValue( ANIMCONTROL_JUMP_LAUNCH, fUnitTime );
						m_fUnitFlyBlend = fUnitTime;
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


void CBotSwarmerBoss::_JumpLanded( void ) {
	m_nJumpState = BOTJUMPSTATE_NONE;
	ClearJumping();
	m_bPlayLandAnim = TRUE;

	ZeroTime( ANIMTAP_JUMP_LAND );

	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
		CFVec3A ImpulseVec_WS;

		ImpulseVec_WS.Set( 0.0f, -m_pBotInfo_Gen->fPhysForce_Land, 0.0f );
		m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
	}
}


void CBotSwarmerBoss::EnterMoveStateWall( void ) {
	m_eMoveState = MOVESTATE_WALL;
}


void CBotSwarmerBoss::EnterMoveStateNormal( void ) {
	m_eMoveState = MOVESTATE_NORMAL;
}


void CBotSwarmerBoss::JumpFromWallToPoint( const CFVec3A &vPt ) {
	f32 fdx, fdy;
	CFVec3A vJump;

	vJump.Sub( vPt, m_MtxToWorld.m_vPos );

	fdx = vJump.MagXZ();
	fdy = vPt.y - m_MtxToWorld.m_vPos.y;
	FMATH_CLAMPMAX( fdy, 0.0f );

	m_qTakeoff.BuildQuat( m_MtxToWorld );

	// calculate time in air
	m_fJumpTimer = 0.0f;
	//m_fOOJumpTime = fmath_Div( 2.0f * fdy, m_BotInfo_Gen.fGravity );
	m_fOOJumpTime = 2.0f * fdy * m_fOOGravity;
	m_fOOJumpTime = fmath_InvSqrt( m_fOOJumpTime );

	// calculate final orientation
	CFVec3A vToTgt;
	vToTgt.Sub( *m_pvTgtPos, vPt );
	CFMtx43A::m_Temp.UnitMtxFromNonUnitVec( &vToTgt );
	
	FASSERT( CFMtx43A::m_Temp.m_vUp.y > 0.9f );
	
	m_qLanding.BuildQuat( CFMtx43A::m_Temp );

	CFVec3A vJumpVel;
	vJumpVel.Mul( vJump, fmath_Inv( fdx ) );
	vJumpVel.x *= fdx * m_fOOJumpTime;
	vJumpVel.z *= fdx * m_fOOJumpTime;
	vJumpVel.y = 0.0f;

	m_eMoveState = MOVESTATE_JUMPING_TO_FLOOR;
	_StartSingleJump( &vJumpVel );
}

	
void CBotSwarmerBoss::_JumpToFloorWork( void ) {
	f32 fSlerp;
	CFQuatA qTmp;
	CFVec3A vTmp;

	m_fJumpTimer += FLoop_fPreviousLoopSecs;

	// handle orientation
	fSlerp = m_fJumpTimer * m_fOOJumpTime;
	if( fSlerp >= 1.0f ) {
		fSlerp = 1.0f;
		m_eMoveState = MOVESTATE_NORMAL;
		f32 fYaw_WS = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
		ChangeMountYaw( fYaw_WS );
		ZeroVelocity();
	}

	qTmp.ReceiveSlerpOf( fmath_UnitLinearToSCurve( fSlerp ), m_qTakeoff, m_qLanding );

	qTmp.BuildMtx33( m_MtxToWorld );

	// save a copy of previous frame info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// add in any velocity impulse...
	BOOL bImpulseApplied = HandleVelocityImpulses();
	
	// update position...
	vTmp.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( vTmp );

	m_MtxToWorld.m_vPos = m_MountPos_WS;

	WS2MS( m_Velocity_MS, m_Velocity_WS );

	if( bImpulseApplied ) {
		VelocityHasChanged();
	}

	// Collide with the world below our feet...
	PROTRACK_BEGINBLOCK("Coll");
		HandleCollision();
	PROTRACK_ENDBLOCK();//"Coll"

	if( m_uCollisionStatus & (COLLSTATE_FLOOR | COLLSTATE_WALL) ) {
		m_nState = STATE_GROUND;
		m_eMoveState = MOVESTATE_NORMAL;
		f32 fYaw_WS = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
		ChangeMountYaw( fYaw_WS );
		ZeroVelocity();
	}

	PROTRACK_BEGINBLOCK("AirXlat");
		HandleAirTranslation();
	PROTRACK_ENDBLOCK();//"AirXlat");

	PROTRACK_BEGINBLOCK("AirAnim");
		if( HandleAirAnimations() ) {
			_EnterFlyMode();
		}
	PROTRACK_ENDBLOCK();//"AirAnim");

	PROTRACK_BEGINBLOCK("CommonAnim");
		_HandleJumpAnimations();
		//HandleHipsAnimation();
		//_HandleAimAnimations();
	PROTRACK_ENDBLOCK();//"CommonAnim");
}
	
	
//METMP
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT

static f32 _fDbgTime = 1.0f;

void CBotSwarmerBoss::_DebugFn( void ) {
	CFVec3 vcollsphere;
	vcollsphere.Set( m_pBotInfo_Gen->fCollSphere1X_MS, m_pBotInfo_Gen->fCollSphere1Y_MS, m_pBotInfo_Gen->fCollSphere1Z_MS );
	vcollsphere.Add( m_MountPos_WS.v3 );

	fdraw_DevSphere( &vcollsphere, m_pBotInfo_Gen->fCollSphere1Radius_MS, &FColor_MotifBlue, 4, 4 );

	u32 uCtr = 0;

	//if( m_bControls_Action ) {
	//	if( _fDbgTime == 1.0f ) {
	//		_fDbgTime = 0.1f;
	//	} else {
	//		_fDbgTime = 1.0f;
	//	}
	//}

	if( m_bControls_Melee ) {
		picklevel_Start();
	}
	if( m_nState == STATE_GROUND ) {
		ftext_DebugPrintf( 0.1f, 0.5f, "~w1ground" );
	} else {
		ftext_DebugPrintf( 0.1f, 0.5f, "~w1air" );
	}

	floop_SetTimeScale( _fDbgTime );

	for( u32 i=0; i<ANIMCONTROL_BASE_COUNT; i++ ) {
		if( GetControlValue( i ) != 0.0f ) {
			ftext_DebugPrintf( 0.4f, 0.33f + (0.03f * uCtr++), "~w1ctl:  %s, %0.2f", m_apszBaseControlNameTable[i], GetControlValue( i ) );
		}
	}

}
#endif





//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSwarmerBossBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CBotSwarmerBossBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTSWARMERBOSS, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CBotSwarmerBoss::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CBotSwarmerBoss::m_BotInfo_Gen.pszArmorProfile;
}


BOOL CBotSwarmerBossBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotSwarmerBossBuilder::InterpretTable( void ) {
	return CBotBuilder::InterpretTable();
}






#endif //0