//////////////////////////////////////////////////////////////////////////////////////
// botpred.cpp - 
//
// Author: Steve Ranck     
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
// 02/18/02 Ranck       Created.
// 12/10/02 Elliott		Took ownership
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botpred.h"
#include "meshtypes.h"
#include "Protrack.h"
#include "Player.h"
#include "fcamera.h"
#include "fresload.h"
#include "fclib.h"
#include "meshentity.h"
#include "weapon_quadlaser.h"
#include "gamesave.h"
#include "botpart.h"
#include "fsound.h"
#include "eparticlepool.h"

#include "fworld_coll.h"
#include "eshield.h"

#define _MESH_FNAME			"GRMPpreda00"
#define _PRED_SFX_BANK		"Predator"
#define _BOTINFO_FILENAME	"b_pred"
#define _MESH_FNAME			"GRMPpreda00"

#define _BOTPART_FILENAME	"bp_pred"


static CBotPredBuilder _BotPredBuilder;

#define _DEBUG_DISABLE_SHIELD		1


//#define _XZ_DECEL_MUL				1.25f
//#define _FORWARD_MAX_VEL			80.0f
//#define _LATERAL_MAX_VEL			_FORWARD_MAX_VEL //40.0f
//
//#define _XZ_ACCEL_FORWARD			60.0f
//#define _XZ_ACCEL_LATERAL			_XZ_ACCEL_FORWARD //40.0f

#define _MAX_VELOCITY_XZ			80.0f	// don't use _MAX_VELOCITY_XZ directly, use m_fMaxFlatSurfaceSpeed_WS instead
#define _MAX_ACCEL_XZ				60.0f
#define _DECEL_MULT_XZ				1.25f

#define _VERT_ACCEL_PER_SEC 		50.0f
#define _DECEL_MULTIPLIER			1.5f
#define _MAX_VERT_VEL				30.0f

#define _TRANS_LAG_FACTOR			0.90f		// 1.0 will put the model (basically) where the mountpos is.  0.5 will lag very far behind
#define _MIN_ALTITUDE				1.0f
#define _ALTITUDE_BUFFER			3.0f

#define _LINEAR_Y_OFFSET 			0.25f		// where to apply the linear forces to get the model to tilt
#define _ROT_STAB_FORCE	 			100.0f

#define _ROT_MIN_STAB_VEL			FMATH_DEG2RAD( 2.0f )
#define _ROT_MAX_STAB_VEL			FMATH_DEG2RAD( 80.0f )
#define _ROT_STAB_VEL_RANGE			(_ROT_MAX_STAB_VEL - _ROT_MIN_STAB_VEL)
#define _ROT_MIN_STAB_EFFECT		0.75f	//0.495f
#define _ROT_MAX_STAB_EFFECT		5.0f	//3.0f
#define _ROT_IMPACT_ADJ				-0.05f			// modifies the torque applied on impact, is negative because it's multiplied to our velocity
#define _FIRE_TORQUE_ADJ			-5.1f			// adjustment to torque applied when weapons fire
#define _MOTION_MASS				1000.0f			// mass of our physics object

#define _AIM_OO_ON_TIME				(1.0f/0.5f)
#define _AIM_OO_OFF_TIME			(1.0f/1.0f)

#define _STREAMER_DEPTH				30
#define _STREAMER_ALPHA_MAX			0.15f
#define _STREAMER_ALPHA_MIN			0.05f
#define _STREAMER_SAMPLES			15.0f
#define _STREAMER_SIZE				3.0f

#define _EXHAUST_ANGLE_CHANGE_ON	6.0f
#define _EXHAUST_ANGLE_CHANGE_OFF	3.0f

#define _EXHAUST_MAX_PITCH			FMATH_DEG2RAD( -90.0f )
#define _EXHAUST_MIN_PITCH			FMATH_DEG2RAD( -60.0f )

#define _EXHAUST_MAX_YAW			FMATH_DEG2RAD( -30.0f )
#define _EXHAUST_MIN_YAW			FMATH_DEG2RAD( 30.0f )

#define _SPOTLIGHT_TEXTURE_ID		1

//these are used to control how much the visual model can tilt (not as much as the mount
//#define _MAX_MODEL_TGT_ANGLE		25.0f
#define _MAX_MODEL_TGT_COS			0.90630778703664996324255265675432f
#define _MAX_MODEL_TGT_OO_TAN		(1.0f/0.46630765815499859283000619479956f)




#define _DEATH_LIMB_GRAVITY						-10.0f
#define _DEATH_XZ_DECELERATION					32.0f
#define _DEATH_Y_ACCELERATION					-3.0f
#define _DEATH_ROT_TIME							3.0f
#define _DEATH_ROT_MAX_SPEED					0.15f
#define _DEATH_ROT_MIN_SPEED					0.025f

#define _DEATH_INITIAL_TIME_BETWEEN_EXPLOSIONS	2.0f

#define _DEATH_MIN_TIME_BETWEEN_BIG_EXPLOSIONS	2.0f
#define _DEATH_MAX_TIME_BETWEEN_BIG_EXPLOSIONS	3.0f
#define _DEATH_BIG_EXPLOSION_RADIUS				5.0f

#define _DEATH_MIN_TIME_BETWEEN_TINY_EXPLOSIONS	0.25f
#define _DEATH_MAX_TIME_BETWEEN_TINY_EXPLOSIONS	0.5f
#define _DEATH_TINY_EXPLOSION_RADIUS			8.0f

#define _DEATH_INITIAL_LIMB_BLOW_TIME			2.0f
#define _DEATH_LIMB_BLOW_MIN_TIME				1.0f
#define _DEATH_LIMB_BLOW_MAX_TIME				2.0f

#define _DEATH_MAX_DESTRUCTION_TIME				20.0f



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPred
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
BOOL					CBotPred::m_bSystemInitialized		= FALSE;
u32						CBotPred::m_nBotClassClientCount = 0;
CBotAnimStackDef		CBotPred::m_AnimStackDef;
CFTexInst				CBotPred::m_StreamerTexInst;
FParticle_DefHandle_t	CBotPred::m_hExhaustParticleDefBack;
FParticle_DefHandle_t	CBotPred::m_hExhaustParticleDefDown;
FMesh_t*				CBotPred::m_pMesh = NULL;
FMesh_t*				CBotPred::m_pSpotlightMesh = NULL;


CBotPred::BotInfo_Gen_t			CBotPred::m_BotInfo_Gen;
CBotPred::BotInfo_MountAim_t	CBotPred::m_BotInfo_MountAim;
CBotPred::BotInfo_Pred_t		CBotPred::m_BotInfo_Pred;

CBotPred*						CBotPred::m_pCBPred					= NULL;


CBotPartPool *CBotPred::m_pPartPool;



//////////////////////////////////////////////////////////////////
//STATIC FNS

BOOL CBotPred::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );
	
	FResFrame_t resFrame = fres_GetFrame();
	
	m_nBotClassClientCount = 0;

	m_bSystemInitialized = TRUE;
    
	return TRUE;
}


void CBotPred::UninitSystem() {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}




//////////////////////////////////////////////////////////////////
//CLASS HEIRARCHY FNS

BOOL CBotPred::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	s32			 nBoneIdx;
	FMeshInit_t  meshInit;
	FResFrame_t  frame = fres_GetFrame();
	CFSphere	 sphere;					// used to init the spotlight
	CFVec3A		vBoxMax, vBoxMin;			// used to init the FMotion object
	CFMtx43A	*pMtx;

	// Get pointer to the leaf class's builder object...
	CBotPredBuilder *pBuilder = (CBotPredBuilder*)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// get params from builder...
	m_fMaxAltitude_WS = pBuilder->m_fMaxAltitude;

	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	meshInit.pMesh		= m_pMesh;
	meshInit.nFlags		= 0;
	meshInit.fCullDist	= FMATH_MAX_FLOAT;
	meshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );

	m_pWorldMesh->Init( &meshInit );
	m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser = this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );
	m_pWorldMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pWorldMesh->SetCollisionFlag( TRUE );
	m_pWorldMesh->SetLineOfSightFlag( FALSE );
	m_pWorldMesh->UpdateTracker();
	m_pWorldMesh->RemoveFromWorld();

	// Find approx eye point...
	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIdx < 0 ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} else {
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vPos;
	}

	// Find gaze direction...
	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIdx < 0 ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} else {
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx]->m_vFront;
	}

	m_pAISteerMtx = &m_MtxToWorld;

	// get some bones...
	m_uBoneIndexSpotlight	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_SPOTLITE] );
	m_uBoneIndexExhaust		= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_JET_PORT] );
	m_uBoneIndexArmSeg0TL 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_UPPER_L2] );
	m_uBoneIndexArmSeg0TR 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_UPPER_R2] );
	m_uBoneIndexArmSeg0BL 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_UPPER_L1] );
	m_uBoneIndexArmSeg0BR 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_UPPER_R1] );

	m_uBoneIndexArmSeg1TL 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_LOWER_L2] );
	m_uBoneIndexArmSeg1TR 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_LOWER_R2] );
	m_uBoneIndexArmSeg1BL 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_LOWER_L1] );
	m_uBoneIndexArmSeg1BR 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_ARM_LOWER_R1] );

	m_uBoneIndexArmSeg2TL 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GUN_L2] );
	m_uBoneIndexArmSeg2TR 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GUN_R2] );
	m_uBoneIndexArmSeg2BL 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GUN_L1] );
	m_uBoneIndexArmSeg2BR 	= m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_GUN_R1] );


	if( m_uBoneIndexSpotlight < 0 ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild():  Unable to find bone:  %s in model.\n", m_apszBoneNameTable[BONE_SPOTLITE] );
		goto _ExitWithError;
	}

	// ANIMATION
	// Build our animation stack and load animations...
	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	// Initialize matrix palette (this is important for attached entities)...
	AtRestMatrixPalette();


	// get bones and set up callbacks...
	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );
	m_Anim.m_pAnimCombiner->DisableAllBoneCallbacks();
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_SPOTLITE] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_JET_PORT] );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_UPPER_L2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_UPPER_R2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_UPPER_L1] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_UPPER_R1] );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_LOWER_L2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_LOWER_R2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_LOWER_L1] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ARM_LOWER_R1] );
											  
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GUN_L2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GUN_R2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GUN_L1] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_GUN_R1] );

	SetControlValue( ASI_STAND, 1.0f );
	UpdateUnitTime( ASI_STAND, fmath_RandomFloat() );

	// Initialize FMotion object...
	vBoxMin.Set( -3.0f, -3.0f, -3.0f );
	vBoxMax.Set( 3.0f, 3.0f, 3.0f );

	m_Motion.Simple_InitBody( &m_pWorldMesh->m_Xfm.m_MtxF, 
							  CFMotion::ComputeInvUniformInertiaTensorMag( m_pBotInfo_Gen->fCollSphere1Radius_MS, _MOTION_MASS ), 
							  _MOTION_MASS, 0.0f );

	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) ) {
		goto _ExitWithError;
	}

	// Make invincible limbs...
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_TOP_RIGHT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_BOTTOM_RIGHT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_TOP_LEFT_ARM, pBuilder );
	MakeLimbInvincible( LIMB_CODE_ARMS, LIMB_TYPE_BOTTOM_LEFT_ARM, pBuilder );

	// Misc stuff...
	SetMaxHealth();

	SetBotFlag_Enemy();
	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle.SetNormOrigin( 0.0f, 0.23f );
	}


	////// Create our light mesh
	m_pSpotLightMesh = fnew CFWorldMesh;
	if( m_pSpotLightMesh == NULL ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	meshInit.pMesh		= m_pSpotlightMesh;
	meshInit.nFlags		= 0;
	meshInit.fCullDist	= FMATH_MAX_FLOAT;
	meshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );

	m_pSpotLightMesh->Init( &meshInit );
	m_pSpotLightMesh->m_nUser = MESHTYPES_UNKNOWN;
	m_pSpotLightMesh->m_pUser = this;
	m_pSpotLightMesh->m_nFlags = FMESHINST_FLAG_NOCOLLIDE;
	m_pSpotLightMesh->SetCollisionFlag( FALSE );
	m_pSpotLightMesh->SetLineOfSightFlag( FALSE );
	m_pSpotLightMesh->UpdateTracker();
	m_pSpotLightMesh->RemoveFromWorld();

	m_pSpotLightMeshMtx = m_pWorldMesh->GetBoneMtxPalette()[m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_SPOTLITE_ATTACH] )];

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _SPOTLIGHT_TEXTURE_ID );
	if( m_pSpotLight == NULL ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild():  Warning!  Unable to retrieve spotlight ID\n" );
	}

	FMATH_SETBITMASK( m_pSpotLightMesh->m_nFlags, FMESHINST_FLAG_POSTER_Z );

	SetSpotLightOff( TRUE );

	//Init weapons
	m_apWeapon[0] = fnew CWeaponQuadLaser();
	if( m_apWeapon[0] == NULL ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild(): Could not allocate WeaponQuadLaser\n" );
		goto _ExitWithError;
	} else {
		if( !((CWeaponQuadLaser*)m_apWeapon[0])->Create() ) {
			goto _ExitWithError;
		}

		m_apWeapon[0]->SetUpgradeLevel( 0 );
		m_apWeapon[0]->EnableAutoWork( FALSE );
		m_apWeapon[0]->RemoveFromWorld();
		m_apWeapon[0]->SetOwner(this);
		m_apWeapon[0]->SetDesiredState( CWeapon::STATE_DEPLOYED );

		//primary must be attached
		m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_DUMMY] );

		// For the predator's quad laser, we need to feed in our muzzle point bones
		((CWeaponQuadLaser*)m_apWeapon[0])->SetFireBones( m_pWorldMesh,  m_apszBoneNameTable[BONE_PRIMARY_FIRE_1], m_apszBoneNameTable[BONE_PRIMARY_FIRE_3],
																		 m_apszBoneNameTable[BONE_PRIMARY_FIRE_2], m_apszBoneNameTable[BONE_PRIMARY_FIRE_4] );
	}

	// Set up data port stuff...
	DataPort_SetupTetherShockInfo();

	// Initialize our exhaust particle emitters...
	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_JETBACK1_ATTACH] );
	if( nBoneIdx > -1 ) {
		pMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx];
		m_hExhaustParticleEmitterBack1 = fparticle_SpawnEmitter( m_hExhaustParticleDefBack, &(pMtx->m_vPos.v3), NULL, &(m_Velocity_WS.v3), 0.0f );
		fparticle_SetDirection( m_hExhaustParticleEmitterBack1, &(pMtx->m_vFront.v3) );
		fparticle_SetIntensity( m_hExhaustParticleEmitterBack1, &m_fUnitExhaustBackMag );
	}

	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_JETBACK2_ATTACH] );
	if( nBoneIdx > -1 ) {
		pMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx];
		m_hExhaustParticleEmitterBack2 = fparticle_SpawnEmitter( m_hExhaustParticleDefBack, &(pMtx->m_vPos.v3), NULL, &(m_Velocity_WS.v3), 0.0f );
		fparticle_SetDirection( m_hExhaustParticleEmitterBack2, &(pMtx->m_vFront.v3) );
		fparticle_SetIntensity( m_hExhaustParticleEmitterBack2, &m_fUnitExhaustBackMag );
	}

	nBoneIdx = m_pWorldMesh->FindBone( m_apszBoneNameTable[BONE_JETDOWN_ATTACH] );
	if( nBoneIdx > -1 ) {
		pMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIdx];
		m_hExhaustParticleEmitterDown = fparticle_SpawnEmitter( m_hExhaustParticleDefBack, &(pMtx->m_vPos.v3), NULL, &(m_Velocity_WS.v3), 0.0f );
		fparticle_SetDirection( m_hExhaustParticleEmitterDown, &(pMtx->m_vFront.v3) );
		fparticle_SetIntensity( m_hExhaustParticleEmitterDown, &m_fUnitExhaustDownMag );
	}

#if !_DEBUG_DISABLE_SHIELD
	m_pShield = fnew CEShield();

	if( !m_pShield->Create( "PredShield" ) ) {
		DEVPRINTF( "CBotPred::ClassHierarchyBuild(): Could not create shield\n" );
		goto _ExitWithError;
	} else {
		ShieldInit_t shieldInit;

		shieldInit.uShieldFlags			= CEShield::SHIELD_ENABLE_TINT;
		shieldInit.fShieldRechargeTime	= m_BotInfo_Pred.fShieldRechargeTime;
		shieldInit.fShieldRechargeDelay	= m_BotInfo_Pred.fShieldRechargeDelay;
		shieldInit.fShieldScale			= 2.5f;
		shieldInit.pArmorProfile		= CDamage::FindArmorProfile( m_BotInfo_Pred.pszShieldArmorProfile );
		shieldInit.rgbTint.Set( 0.75f, 0.0f, 0.0f );

		m_pShield->Init( this, &shieldInit );
	}
#endif

	// Create tag points...
	if( !TagPoint_CreateFromBoneArray( m_anTagPointBoneNameIndexArray, m_apszBoneNameTable ) ) {
		goto _ExitWithError;
	}

	//useful for AI, never changes for the predator
	m_fMaxFlatSurfaceSpeed_WS	= pBuilder->m_fMaxSpeed;
	m_fMaxVerticalSpeed_WS		= _MAX_VERT_VEL;

	RemoveFromWorld();

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CBotPred::ClassHierarchyDestroy( void ) {
	// destroy everything we built in ClassHierarchyBuild()

	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	if( m_pSpotLightMesh != NULL ) {
		fdelete( m_pSpotLightMesh );
		m_pSpotLightMesh = NULL;
	}

	//if( m_pShield ) {
	//	fdelete( m_pShield );
	//	m_pShield = NULL;
	//}

	// destroy exhaust smoke particle emitters
	fparticle_KillEmitter( m_hExhaustParticleEmitterBack1 );
	fparticle_KillEmitter( m_hExhaustParticleEmitterBack2 );
	fparticle_KillEmitter( m_hExhaustParticleEmitterDown );

	WeaponsDestroy( 0 );

	CBot::ClassHierarchyDestroy();
}


void CBotPred::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	// init our lag position to be our position
	m_ModelLagPos		= m_MtxToWorld.m_vPos;

	// put our motion object in the right place
	m_Motion.Simple_UpdateOrientation( &m_MtxToWorld );

	m_PrevVelocity_WS.Zero();

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	m_TargetedPoint_WS.Mul( m_MtxToWorld.m_vFront, 250.0f ).Add( m_MtxToWorld.m_vPos );
	m_vLastValidTargetPoint_WS.Zero();

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_DUMMY] );
		((CWeaponQuadLaser*)m_apWeapon[0])->ResetAimPoints( m_TargetedPoint_WS );
	}

	AtRestMatrixPalette();

	// create streamers...
	m_hStreamer = CFXStreamerEmitter::SpawnFromMeshBones( _STREAMER_DEPTH, m_pWorldMesh, m_apszStreamerBones, _STREAMER_ALPHA_MAX, &m_StreamerTexInst, _STREAMER_SIZE, _STREAMER_SAMPLES, CFXStreamerEmitter::USE_FRONT_AXIS );

	// enable particle emitters...
	fparticle_EnableEmission( m_hExhaustParticleEmitterBack1, TRUE );
	fparticle_EnableEmission( m_hExhaustParticleEmitterBack2, TRUE );
	fparticle_EnableEmission( m_hExhaustParticleEmitterDown, TRUE );

	// turn on light...
	m_pSpotLightMesh->AddToWorld();
	SetSpotLightOn();

	//// create the sounds...
	//if( m_nPossessionPlayerIndex < 0 ) {
	//	m_b3DSounds				= TRUE;

	//	m_pWindAudioEmitter  = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupWind, FALSE, &m_MtxToWorld.m_vPos );
	//	m_pHoverAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupHover, FALSE, &m_MtxToWorld.m_vPos );
	//} else {
	//	m_b3DSounds				= FALSE;

	//	m_pWindAudioEmitter  = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupWind, TRUE );
	//	m_pHoverAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupHover, TRUE );
	//}

	m_fUnitAim = 0.0f;


	// add the shield
#if !_DEBUG_DISABLE_SHIELD
	m_pShield->AddToWorld();
	m_pShield->Attach_UnitMtxToParent_PS( this, m_apszBoneNameTable[BONE_DUMMY] );
	m_pShield->EnableShield( TRUE );
#endif

	WeaponsAddToWorld( 0 );
}


void CBotPred::_RemoveAllAliveEffects( void ) {
	// disable streamers...
	if( m_hStreamer != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamer, FALSE );
		CFXStreamerEmitter::ReturnEmitterHandle( m_hStreamer );
		m_hStreamer = FXSTREAMER_INVALID_HANDLE;
	}

	// disable particle emitters...
	fparticle_EnableEmission( m_hExhaustParticleEmitterBack1,	FALSE );
	fparticle_EnableEmission( m_hExhaustParticleEmitterBack2,	FALSE );
	fparticle_EnableEmission( m_hExhaustParticleEmitterDown,	FALSE );

	// turn off light...
	m_pSpotLightMesh->RemoveFromWorld();
	SetSpotLightOff();

	//// get rid of the shield...
	//if( m_pShield ) {
	//	m_pShield->RemoveFromWorld();
	//}

	// get rid of our sounds
	if( m_pWindAudioEmitter != NULL ) {
		m_pWindAudioEmitter->Destroy();	
		m_pWindAudioEmitter = NULL;
	}

	if( m_pHoverAudioEmitter != NULL ) {
		m_pHoverAudioEmitter->Destroy();	
		m_pHoverAudioEmitter = NULL;
	}
	m_fUnitWindLevel = 0.0f;
}


void CBotPred::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();


	WeaponsRemoveFromWorld( 0 );

	_RemoveAllAliveEffects();
	_StopBurnSmoke();

	CBot::ClassHierarchyRemoveFromWorld();
}


BOOL CBotPred::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	// All bots, by default, cast shadows.  THis one doesn't
	m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;

	EnableOurWorkBit();

	//xxxxxxxxxxxxxx
#if 0	// <----- RUSS: ENABLE THIS CODE (don't forget to disable it when you check in, though
#if !FANG_PRODUCTION_BUILD
	SetNormHealth( 0.00001f );
#endif
#endif

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotPred::ClassHierarchyRelocated( void *pIdentifier ) {
	CBot::ClassHierarchyRelocated( pIdentifier );

	// if we were put here by someone else, update the position of our physics object.
	// otherwise, the model will glide over from where it was last frame
	if( pIdentifier != m_pMoveIdentifier ) {
		m_Motion.Simple_UpdateOrientation( &m_MtxToWorld );
		m_ModelLagPos		= m_MtxToWorld.m_vPos;
	}
}


CEntityBuilder *CBotPred::GetLeafClassBuilder( void ) {
	return &_BotPredBuilder;
}



void CBotPred::ClassHierarchyWork( void ) {
	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( IsDeadOrDying() ) {
		DeathWork();
		return;
	}

	Power_Work();
	DataPort_Work();

	if( DataPort_IsBeingShocked() ) {
		_UpdateMatrices();
		WeaponsWork(0);
		return;
	}

	ParseControls();
	ComputeXlatStickInfo();

	// update position...
	CFVec3A vTmp;
	m_MountPrevPos_WS = m_MountPos_WS;
	
	vTmp.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	m_MountPos_WS.Add( vTmp );

	
	PROTRACK_BEGINBLOCK("Coll");
		_HandleCollision();
		_CheckAltitude();
	PROTRACK_ENDBLOCK();//"Coll"

	_MoveModel();
	m_PrevVelocity_WS = m_Velocity_WS;

	_HandleHover();
	_HandleXZTranslation();

	// let CBot handle the mount yaw and pitch so it's consistent w/ other bots
	HandleYawMovement();
	HandlePitchMovement();

	if( m_nPossessionPlayerIndex >= 0 ) {
		m_vHeadLookPoint_WS.Sub( m_TargetedPoint_WS, m_MountPos_WS );
		if( m_vHeadLookPoint_WS.MagSq() > 0.001f ) {
			m_vHeadLookPoint_WS.Unitize().Mul( 1000.0f );
			m_vHeadLookPoint_WS.Add( m_MountPos_WS );
			HeadLook();
		}
	} else  {
		HandleHeadLook();
	}

	DeltaTime( ASI_STAND );

	PROTRACK_BEGINBLOCK("UpdateMatrices");
		_UpdateMatrices();
	PROTRACK_ENDBLOCK();
	
	PROTRACK_BEGINBLOCK("Weapon");
		_HandleTargeting();
		_HandleWeaponWork();
	PROTRACK_ENDBLOCK();//"WeaponAnim");

//	_LightWork();
	_ExhaustWork();
	UpdateSpotLight();
	_SoundWork();

//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	fdraw_DevSphere( &m_TargetedPoint_WS.v3, 3.0f, &FColor_MotifBlue );
//	if( m_bControls_Melee ) {
//		m_pPartMgr->SetState_BlowUp( LIMB_TYPE_TOP_LEFT_ARM );
//		m_pPartMgr->SetState_BlowUp( LIMB_TYPE_TOP_RIGHT_ARM );
//		m_pPartMgr->SetState_BlowUp( LIMB_TYPE_BOTTOM_RIGHT_ARM );
//	}
//#endif

}


//////////////////////////////////////////////////////////////////
//PREDATOR FNS

CBotPred::CBotPred() : CBot() {
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


CBotPred::~CBotPred() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotPred::Create( s32 nPlayerIndex/*=-1*/, BOOL bInstallDataPort/*=FALSE*/, cchar *pszEntityName/*=NULL*/, const CFMtx43A *pMtx/*=NULL*/, cchar *pszAIBuilderName/*=NULL*/ ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Resources have already been cleaned up...
		return FALSE;
	}


	CBotPredBuilder *pBuilder = (CBotPredBuilder*)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotPredBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTPRED );
	}

	// set up any builder params...

	return CBot::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CBotPred::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	if( !CBot::ClassHierarchyLoadSharedResources() ) {
		return FALSE;
	}

	++m_nBotClassClientCount;

	if( m_nBotClassClientCount > 1 ) {
		// Resources already loaded...

		return TRUE;
	}

	// Resources not yet loaded...
	FTexDef_t	*pTex;
	FResFrame_t ResFrame = fres_GetFrame();

	if( !fresload_Load( FSNDFX_RESTYPE, _PRED_SFX_BANK ) ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources(): Could not load sound effect bank '%s'\n", _PRED_SFX_BANK);
	}

	// read bot info file
	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources():  Error reading bot info file\n" );
		goto _ExitWithError;
	}

	if( !_BuildAnimStackDef() ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources():  Error building animation stack\n" );
		goto _ExitWithError;
	}

	// load streamer texture...
	pTex = (FTexDef_t*)fresload_Load( FTEX_RESNAME, m_BotInfo_Pred.pszStreamerTex );
	if( pTex == NULL ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources():  Error loading streamer texture:  %s\n", m_BotInfo_Pred.pszStreamerTex );
		goto _ExitWithError;
	}
	
	m_StreamerTexInst.SetTexDef( pTex );


	// load exhaust particles...
	m_hExhaustParticleDefBack = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Pred.pszParticleJetBack );
	if( m_hExhaustParticleDefBack == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources():  Error loading exhaust particle effect:  %s\n", m_BotInfo_Pred.pszParticleJetBack );
		goto _ExitWithError;
	}

	m_hExhaustParticleDefDown = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, m_BotInfo_Pred.pszParticleJetDown );
	if( m_hExhaustParticleDefDown == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources():  Error loading exhaust particle effect:  %s\n", m_BotInfo_Pred.pszParticleJetDown );
		goto _ExitWithError;
	}

	// load meshes...
	m_pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _MESH_FNAME );
	if( m_pMesh == NULL ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources(): Could not find mesh '%s'\n", _MESH_FNAME );
		goto _ExitWithError;
	}

	m_pSpotlightMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, m_BotInfo_Pred.pszSpotlightMesh );
	if( m_pSpotlightMesh == NULL ) {
		DEVPRINTF( "CBotPred::ClassHierarchyLoadSharedResources(): Could not find mesh '%s'\n", m_BotInfo_Pred.pszSpotlightMesh );
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CBotPred::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount == 0 ) {
		m_AnimStackDef.Destroy();
	}

	m_pMesh = NULL;
	m_pSpotlightMesh = NULL;
	m_hExhaustParticleDefBack = FPARTICLE_INVALID_HANDLE;
	m_hExhaustParticleDefDown = FPARTICLE_INVALID_HANDLE;

	CBot::ClassHierarchyUnloadSharedResources();
}


void CBotPred::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) < FWORLD_MAX_SKIPLIST_ENTRIES );

	if( m_apWeapon[0] ) {
		m_apWeapon[0]->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

#if !_DEBUG_DISABLE_SHIELD
	m_pShield->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
#endif

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}


// Checkpoint save / restore functions
void CBotPred::CheckpointSaveSelect( s32 nCheckpoint ) {

	// save weapon
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->CheckpointSaveSelect( nCheckpoint );
	}

	// then save self
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


BOOL CBotPred::CheckpointSave( void ) {
	if( !CBot::CheckpointSave() ) {
		return FALSE;
	}

	CFCheckPoint::SaveData( m_DeathRotUnitAxis_WS );
	CFCheckPoint::SaveData( m_fDeathMaxDestructionCountdownSecs );
	CFCheckPoint::SaveData( m_fDeathUnitRotVel );
	CFCheckPoint::SaveData( m_fDeathBigExplosionCountdownSecs );
	CFCheckPoint::SaveData( m_fDeathTinyExplosionCountdownSecs );
	CFCheckPoint::SaveData( m_fDeathLimbBlowCountdownSecs );
	CFCheckPoint::SaveData( m_nDeathLimbBlowIndex );

	//f32 fShieldHealth;

	//if( m_pShield ) {
	//	fShieldHealth = m_pShield->NormHealth();
	//} else {
	//	fShieldHealth = 0.0f;
	//}

	//CCheckPoint::SaveData( fShieldHealth );

	return TRUE;
}



void CBotPred::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS ) {
	CFVec3A Right, Front;

	*pApproxMuzzlePoint_WS = m_MountPos_WS;
//	Right.Mul( m_MtxToWorld.m_vRight, m_pBotInfo_Weapon->fOriginToApproxMuzzleX );
//	Front.Mul( m_MountUnitFrontXZ_WS, m_pBotInfo_Weapon->fOriginToApproxMuzzleZ );

//	pApproxMuzzlePoint_WS->Add( Right, Front ).Add( m_MtxToWorld.m_vPos );
//	pApproxMuzzlePoint_WS->y += m_pBotInfo_Weapon->fOriginToApproxMuzzleY;
}


void CBotPred::CheckpointRestore( void ) {
	CBot::CheckpointRestore();

	////f32 fShieldHealth;
	////CCheckPoint::LoadData( fShieldHealth );

	m_fUnitAim					= 0.0f;
	m_fUnitExhaustBackMag		= 0.0f;
	m_fUnitExhaustDownMag		= 0.0f;
	m_fUnitExhaustPitch			= 0.0f;	
	m_fUnitExhaustYaw			= 0.0f;
	m_fAltitude					= 0.0f;
	m_PrevVelocity_WS			= m_Velocity_WS;
	m_vLastValidTargetPoint_WS.Zero();
	m_ModelLagPos				= m_MountPos_WS;
	
	// reinit motion object
	m_Motion.Simple_InitBody( &m_pWorldMesh->m_Xfm.m_MtxF, 
							  CFMotion::ComputeInvUniformInertiaTensorMag( m_pBotInfo_Gen->fCollSphere1Radius_MS, _MOTION_MASS ), 
							  _MOTION_MASS, 0.0f );

	CFCheckPoint::LoadData( m_DeathRotUnitAxis_WS );
	CFCheckPoint::LoadData( m_fDeathMaxDestructionCountdownSecs );
	CFCheckPoint::LoadData( m_fDeathUnitRotVel );
	CFCheckPoint::LoadData( m_fDeathBigExplosionCountdownSecs );
	CFCheckPoint::LoadData( m_fDeathTinyExplosionCountdownSecs );
	CFCheckPoint::LoadData( m_fDeathLimbBlowCountdownSecs );
	CFCheckPoint::LoadData( m_nDeathLimbBlowIndex );

	_StopBurnSmoke();

	if( m_uLifeCycleState == BOTLIFECYCLE_DYING ) {
		// Restoring to a death state...

		m_pPartMgr->SetState_Dangle( LIMB_TYPE_TOP_RIGHT_ARM, TRUE, _DEATH_LIMB_GRAVITY );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOTTOM_RIGHT_ARM, TRUE, _DEATH_LIMB_GRAVITY );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_TOP_LEFT_ARM, TRUE, _DEATH_LIMB_GRAVITY );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOTTOM_LEFT_ARM, TRUE, _DEATH_LIMB_GRAVITY );

		_StartBurnSmoke();
	}
	

	//xxxxxxxxxxxxxx
#if 0	// <----- RUSS: ENABLE THIS CODE (don't forget to disable it when you check in, though
#if !FANG_PRODUCTION_BUILD
	SetNormHealth( 0.00001f );
#endif
#endif
}
 

void CBotPred::_ClearDataMembers( void ) {

	m_fCollCylinderRadius_WS = 9.4f;
	m_fCollCylinderHeight_WS = 9.3f;

	m_pBotInfo_Gen		= &m_BotInfo_Gen;
	m_pBotInfo_MountAim	= &m_BotInfo_MountAim;
	m_pBotInfo_Walk		= NULL;
	m_pBotInfo_Jump		= NULL;
	m_pBotInfo_Weapon	= NULL;

	m_fMountPitchMax_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
	m_fMountPitchMin_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;

	m_anAnimStackIndex[ASI_STAND]			= ANIMTAP_STAND;
	m_anAnimStackIndex[ASI_STAND_ALERT]		= -1;
	m_anAnimStackIndex[ASI_WALK]			= -1;
	m_anAnimStackIndex[ASI_WALK_ALERT]		= -1;
	m_anAnimStackIndex[ASI_RUN]				= -1;
	m_anAnimStackIndex[ASI_RUN_PANIC]		= -1;
	m_anAnimStackIndex[ASI_FALL]			= -1;
	m_anAnimStackIndex[ASI_RC_TETHERED]		= ANIMTAP_RC_TETHERED;
	m_anAnimStackIndex[ASI_RC_POWER_DOWN]	= ANIMTAP_RC_POWER_DOWN;
	m_anAnimStackIndex[ASI_RC_POWER_UP]		= ANIMTAP_RC_POWER_UP;
	m_anAnimStackIndex[ASI_STOOP]			= -1;

	// set up some bone masks used by the bot layer
	m_pnEnableBoneNameIndexTableForSummer_Normal		= m_aBoneEnableIndices_FullBody;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock	= m_aBoneEnableIndices_FullBody;


	m_ModelLagPos.Zero();

	m_pSpotLightMesh = NULL;

	m_fUnitAim = 0.0f;
	m_vLastValidTargetPoint_WS.Zero();

	m_fMaxAltitude_WS = FMATH_MAX_FLOAT;
	m_fAltitude = 0.0f;

	m_hStreamer	= FXSTREAMER_INVALID_HANDLE;

	// exhaust particle stuff...
	m_hExhaustParticleEmitterBack1	= FPARTICLE_INVALID_HANDLE;
	m_hExhaustParticleEmitterBack2	= FPARTICLE_INVALID_HANDLE;
	m_hExhaustParticleEmitterDown	= FPARTICLE_INVALID_HANDLE;
	m_fUnitExhaustBackMag			= 0.0f;
	m_fUnitExhaustDownMag			= 0.0f;
	m_fUnitExhaustPitch				= 0.0f;
	m_fUnitExhaustYaw				= 0.0f;

	m_pMoveIdentifier = &m_fAltitude;

	m_pWindAudioEmitter		= NULL;
	m_pHoverAudioEmitter	= NULL;
	m_fUnitWindLevel		= 0.0f;

	m_pDeathBurnSmokeParticle = NULL;
}


void CBotPred::_MoveModel( void ) {

	CFVec3A vForcePos;		// where the force is applied
	CFVec3A	vForce;			// the force we're applying
	CFVec3A vTorque;

	// First, calculate our delta velocity and apply it as a force to our physics system so the model tilts w/ acceleration
	vForcePos = m_Motion.m_Mtx.m_vPos;
	vForcePos.y += _LINEAR_Y_OFFSET;
	vForce.Sub( m_Velocity_WS, m_PrevVelocity_WS );
	vForce.Mul( m_Motion.m_fMass * FLoop_fPreviousLoopOOSecs );
	m_Motion.ApplyForce_WS( &vForcePos, &vForce );


	//These forces act to orient the predator
	CFVec3A vDesiredUpVec = m_MtxToWorld.m_vUp;
	f32 fDot = m_MtxToWorld.m_vUp.Dot( CFVec3A::m_UnitAxisY );

	if( fDot < _MAX_MODEL_TGT_COS ) {
		vDesiredUpVec.UnitizeXZ();
		vDesiredUpVec.y = _MAX_MODEL_TGT_OO_TAN;
		vDesiredUpVec.Unitize();
	}

	fDot = vDesiredUpVec.Dot( CFVec3A::m_UnitAxisY );

	// Try to orient it so it's facing up
	vForce.Mul( vDesiredUpVec, _ROT_STAB_FORCE * m_Motion.m_fMass );
	vTorque.Cross( m_Motion.m_Mtx.m_vUp, vForce );

	m_Motion.ApplyTorque_WS( &vTorque );

	// Try to orient the nose to where the mount is pointing
	vForce.Mul( m_MountUnitFrontXZ_WS, _ROT_STAB_FORCE * m_Motion.m_fMass * 2.0f );
	vForcePos.ReceiveUnitXZ( m_Motion.m_Mtx.m_vFront );
	vTorque.Cross( vForcePos, vForce );
	m_Motion.ApplyTorque_WS( &vTorque );


	// Now add some dampening effects
	if( m_Motion.m_AngularVelocity_WS.MagSq() > _ROT_MIN_STAB_VEL * _ROT_MIN_STAB_VEL ) {
		f32 fdVel	= m_Motion.m_AngularVelocity_WS.Mag();
		f32 fAccel	= fmath_Div( fdVel - _ROT_MIN_STAB_VEL, _ROT_STAB_VEL_RANGE );
		fAccel = FMATH_FPOT( fAccel, _ROT_MIN_STAB_EFFECT, _ROT_MAX_STAB_EFFECT );

		vTorque.Mul( m_Motion.m_AngularVelocity_WS, -1.0f );
		vTorque.Unitize();
		vTorque.Mul( fAccel );
		vTorque.Div( m_Motion.m_fOOUniformTensorMag );

		m_Motion.ApplyTorque_WS( &vTorque );
	}


	// enough physics, now try to get the model to where the mount is
	CFVec3A vPredictPos, vTmp;
	vPredictPos.Add( m_MountPos_WS, m_Velocity_WS );
	vTmp.Sub( vPredictPos, m_ModelLagPos );
	vTmp.Mul( FLoop_fPreviousLoopSecs * _TRANS_LAG_FACTOR );
	m_ModelLagPos.Add( vTmp );   	
}



void CBotPred::_UpdateMatrices( void ) {
	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

	f32 fdt = FLoop_fPreviousLoopSecs;
	FMATH_CLAMP( fdt, (1.0f/60.0f), 1.0f );
	m_Motion.Simple_Simulate( fdt );

	CFMtx43A::m_Temp = m_Motion.m_Mtx;
	CFMtx43A::m_Temp.m_vPos = m_ModelLagPos;

	m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp );
	m_pWorldMesh->UpdateTracker();

	PROTRACK_BEGINBLOCK("ComputeMtxPal");
		// Update bone matrix palette...
		ComputeMtxPalette( TRUE );
	PROTRACK_ENDBLOCK();//"ComputeMtxPal");

	
	CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp, TRUE, m_pMoveIdentifier );
}


void CBotPred::_HandleHover( void ) {
	BOOL	bAccelApplied		= FALSE;
	f32		fUnitAvailablVel;

	// when we're checking max altitude, we're checking absolute Y position, and only for human controlled preds
	if( m_nPossessionPlayerIndex >= 0 ) {
		if( m_MountPos_WS.y > m_fMaxAltitude_WS - _ALTITUDE_BUFFER ) {
			if( m_Velocity_WS.y > 0.0f ) {
				// still headed up
				FMATH_CLAMPMAX( m_MountPos_WS.y, m_fMaxAltitude_WS );	// don't let the pred get above max altitude
				fUnitAvailablVel = 1.0f - fmath_Div( m_MountPos_WS.y - (m_fMaxAltitude_WS - _ALTITUDE_BUFFER), _ALTITUDE_BUFFER );
				FMATH_CLAMPMAX( m_Velocity_WS.y, fUnitAvailablVel * _MAX_VERT_VEL );
				VelocityHasChanged();
			}

			if( m_Velocity_WS.y > -0.1f * _VERT_ACCEL_PER_SEC ) {
				m_Velocity_WS.y -= _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs;
				VelocityHasChanged();
				bAccelApplied = TRUE;
			}
		}
	}

	// when we're checking min altitude, we're checking m_fAltitude, which is the height above the ground (or an object)
	if( m_fAltitude < _MIN_ALTITUDE + _ALTITUDE_BUFFER ) {
		if( m_Velocity_WS.y < 0.0f ) {
			// still headed down, let's do something about that
			FMATH_CLAMPMIN( m_fAltitude, _MIN_ALTITUDE );
			fUnitAvailablVel = -1.0f * fmath_Div( m_fAltitude - _MIN_ALTITUDE, _ALTITUDE_BUFFER );
			FMATH_CLAMPMIN( m_Velocity_WS.y, fUnitAvailablVel * _MAX_VERT_VEL );
            VelocityHasChanged();			
		}

		if( m_Velocity_WS.y < 0.1f * _VERT_ACCEL_PER_SEC ) {
			m_Velocity_WS.y += _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs;
			VelocityHasChanged();
			bAccelApplied = TRUE;
		}
	}

	if( m_fControls_FlyUp < 0.0f ) {
		m_Velocity_WS.y += m_fControls_FlyUp * _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs;
		bAccelApplied = TRUE;
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	// up?
	if( m_fControls_FlyUp > 0.0f ) {
		m_Velocity_WS.y += m_fControls_FlyUp * _VERT_ACCEL_PER_SEC * FLoop_fPreviousLoopSecs;
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

void CBotPred::_HandleXZTranslation( void ) {
	CFVec3A vDesVel_WS;
	CFVec3A vDeltaVel_WS;
	f32		fMaxAccelThisFrame = _MAX_ACCEL_XZ * FLoop_fPreviousLoopSecs;

	vDesVel_WS.Mul( m_XlatStickNormVecXZ_WS, m_fMaxFlatSurfaceSpeed_WS );
	vDeltaVel_WS.Sub( vDesVel_WS, m_Velocity_WS );
	vDeltaVel_WS.y = 0.0f;

	if( vDeltaVel_WS.Dot( m_Velocity_WS ) < 0.0f ) {
		fMaxAccelThisFrame *= _DECEL_MULT_XZ;
	}

	if( vDeltaVel_WS.MagSq() < (fMaxAccelThisFrame * fMaxAccelThisFrame) ) {
		m_Velocity_WS.x = vDesVel_WS.x;
		m_Velocity_WS.z = vDesVel_WS.z;
	} else  {
		vDeltaVel_WS.Unitize().Mul( fMaxAccelThisFrame );
        m_Velocity_WS.Add( vDeltaVel_WS );     
	}

	CBot::VelocityHasChanged();
}
	
/*
CFVec3A vDesiredVelocity_MS;
	CFVec3A vVelDiff_MS;
	f32		fMaxAccelX, fMaxAccelZ;
	f32		fVal;

	WS2MS( m_Velocity_MS, m_Velocity_WS );

	vDesiredVelocity_MS.x = m_XlatStickNormVecXZ_MS.x * _LATERAL_MAX_VEL;
	vDesiredVelocity_MS.z = m_XlatStickNormVecXZ_MS.z;
	vDesiredVelocity_MS.y = m_Velocity_MS.y;
	if( vDesiredVelocity_MS.z > 0.0f ) {
		vDesiredVelocity_MS.z *= _FORWARD_MAX_VEL;
	} else {
		vDesiredVelocity_MS.z *= _LATERAL_MAX_VEL;
	}

	vVelDiff_MS.Sub( vDesiredVelocity_MS, m_Velocity_MS );
	fMaxAccelX = FMATH_FABS(vVelDiff_MS.x);
	fMaxAccelZ = FMATH_FABS(vVelDiff_MS.z);

	if( vVelDiff_MS.MagSqXZ() < 0.1f ) {
		vVelDiff_MS.x = 0.0f;
		vVelDiff_MS.z = 0.0f;
		m_Velocity_MS.x = vDesiredVelocity_MS.x;
		m_Velocity_MS.z = vDesiredVelocity_MS.z;
	} else {
		vVelDiff_MS.UnitizeXZ();
		vVelDiff_MS.x *= _XZ_ACCEL_LATERAL;
		if( vVelDiff_MS.z > 0.0f ) {
			vVelDiff_MS.z *=_XZ_ACCEL_FORWARD;
		} else {
			vVelDiff_MS.z *=_XZ_ACCEL_LATERAL;
		}

		// if we're switching direction, slow down faster
		if( (vVelDiff_MS.z * m_Velocity_MS.z) < 0.0f ) {
            vVelDiff_MS.z *= _XZ_DECEL_MUL;
		}

		if( (vVelDiff_MS.x * m_Velocity_MS.x) < 0.0f ) {
            vVelDiff_MS.x *= _XZ_DECEL_MUL;
		}
	}
	
	fVal = vVelDiff_MS.x * FLoop_fPreviousLoopSecs;
	FMATH_BIPOLAR_CLAMPMAX( fVal, fMaxAccelX );
	m_Velocity_MS.x += fVal;

	fVal = vVelDiff_MS.z * FLoop_fPreviousLoopSecs;
	FMATH_BIPOLAR_CLAMPMAX( fVal, fMaxAccelZ );
	m_Velocity_MS.z += fVal;

	MS2WS( m_Velocity_WS, m_Velocity_MS );

	VelocityHasChanged();
}

*/


f32 CBotPred::ComputeEstimatedControlledStopTimeXZ( void ) {
	FASSERT( IsCreated() );

	// our accel is constant, and always at the increased value, because we're coming to a stop.
	return fmath_Div( m_fSpeedXZ_WS, _MAX_ACCEL_XZ * _DECEL_MULT_XZ );
}


f32 CBotPred::ComputeEstimatedControlledStopTimeY( void ) {
	return FMATH_FABS( fmath_Div( m_Velocity_WS.y, _VERT_ACCEL_PER_SEC * _DECEL_MULTIPLIER ) );
}



void CBotPred::_HandleCollision( void ) {
	FCollImpact_t *pImpact;
	CFVec3A PrevToCurrentPos_WS, PushAmount_WS, PrevSphereCenter_WS;
	u32 i;
	f32 fDot;

	m_CollSphere.m_Pos.x	= m_pBotInfo_Gen->fCollSphere1X_MS + m_MountPos_WS.x;
	m_CollSphere.m_Pos.y	= m_pBotInfo_Gen->fCollSphere1Y_MS + m_MountPos_WS.y - 1.0f;
	m_CollSphere.m_Pos.z	= m_pBotInfo_Gen->fCollSphere1Z_MS + m_MountPos_WS.z;
	m_CollSphere.m_fRadius	= m_pBotInfo_Gen->fCollSphere1Radius_MS;

	PrevSphereCenter_WS.x	= m_CollSphere.m_Pos.x;
	PrevSphereCenter_WS.y	= m_pBotInfo_Gen->fCollSphere1Y_MS + m_MountPrevPos_WS.y - 1.0f;
	PrevSphereCenter_WS.z	= m_CollSphere.m_Pos.z;

	// check for collisions with terrain
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
	UserTracker.FindIntersectingTrackers( CBot::TrackerCollisionCallback, FWORLD_TRACKERTYPE_MESH );

	fworld_CollideWithWorldTris( &m_CollInfo, m_pWorldMesh );

	if( FColl_nImpactCount ) {
		fcoll_Sort( FALSE );

		for( i=0; i<FColl_nImpactCount; i++ ) {
			pImpact = FColl_apSortedImpactBuf[i];

			PrevToCurrentPos_WS.Sub( PrevSphereCenter_WS, pImpact->aTriVtx[0] );
			fDot = PrevToCurrentPos_WS.Dot( pImpact->UnitFaceNormal );
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

			// add a torque impulse...
			
			CFVec3A vPush;
			vPush.Mul( m_Velocity_WS, m_Motion.m_fMass * _ROT_IMPACT_ADJ );
			CFVec3A vArm;
			vArm.Sub( pImpact->ImpactPoint, m_MountPos_WS );
			CFVec3A vTorque;
			vTorque.Cross( vArm, vPush );
			m_Motion.ApplyTorqueImpulse_WS( &vTorque );
		}
	}

	// Update immobile flag...
	if( m_nBotFlags & BOTFLAG_IMMOBILIZE_PENDING ) {
		if( m_fSpeed_WS == 0.0f ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
		}
	}
	m_pCollBot = NULL;	//hopefully this will start a trend among bot implementers.
}


void CBotPred::_CheckAltitude( void ) {
	CFVec3A vCollLen;

	// check for collisions with terrain
	fcoll_Clear();

	m_CollInfo.nCollTestType			= FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.nStopOnFirstOfCollMask   = FCOLL_MASK_NONE;
	m_CollInfo.bFindClosestImpactOnly	= TRUE;
	m_CollInfo.nCollMask				= (m_nPossessionPlayerIndex < 0) ? FCOLL_MASK_COLLIDE_WITH_NPCS : FCOLL_MASK_COLLIDE_WITH_PLAYER;
	m_CollInfo.nResultsLOD				= FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_CollInfo.pTag						= NULL;
	m_CollInfo.bCalculateImpactData		= FALSE;		// don't need it

	vCollLen.Mul( CFVec3A::m_UnitAxisY, -_ALTITUDE_BUFFER );
	vCollLen.Add( m_MountPos_WS );

	m_CollInfo.ProjSphere.Init( &m_MountPos_WS, &vCollLen, m_pBotInfo_Gen->fCollSphere1Radius_MS * 0.75f);

	if( fworld_CollideWithWorldTris( &m_CollInfo, m_pWorldMesh ) ) {
		m_fAltitude = FColl_aImpactBuf[0].fImpactDistInfo;
	} else {
		m_fAltitude = 20.0f;
	}
}


void CBotPred::_ExhaustWork( void ) {

	m_fUnitExhaustBackMag = m_XlatStickNormVecXZ_MS.z;
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitExhaustBackMag );
	
	m_fUnitExhaustDownMag = m_fControls_FlyUp;
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitExhaustBackMag );

	m_fUnitExhaustDownMag = FMATH_MAX( m_fUnitExhaustBackMag, m_fUnitExhaustDownMag );

	if( m_fControls_FlyUp > 0.0f ) {
		m_fUnitExhaustPitch -= FLoop_fPreviousLoopSecs;
	} else {
		m_fUnitExhaustPitch += FLoop_fPreviousLoopSecs;
	}
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitExhaustPitch );

	if( m_fControls_RotateCW > 0.0f ) {
		m_fUnitExhaustYaw += FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_MAX1( m_fUnitExhaustYaw );
	} else if( m_fControls_RotateCW < 0.0f ) {
		m_fUnitExhaustYaw -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_MIN0( m_fUnitExhaustYaw );
	} else {
		if( m_fUnitExhaustYaw > 0.5f ) {
			m_fUnitExhaustYaw -= FLoop_fPreviousLoopSecs * 0.5f;
			FMATH_CLAMPMIN( m_fUnitExhaustYaw, 0.5f );
		} else if( m_fUnitExhaustYaw < 0.5f ) {
			m_fUnitExhaustYaw += FLoop_fPreviousLoopSecs * 0.5f;
			FMATH_CLAMPMAX( m_fUnitExhaustYaw, 0.5f );
		}
	}
}

#define _MIN_WIND_PITCH		1.0f
#define _MAX_WIND_PITCH		1.2f
#define _MIN_WIND_VOLUME	0.0f
#define _MAX_WIND_VOLUME	1.0f

#define _MIN_HOVER_PITCH	1.0f
#define _MAX_HOVER_PITCH	1.1f
#define _MIN_HOVER_VOLUME	0.5f
#define _MAX_HOVER_VOLUME	1.0f
#define _WIND_RAMP_UP_RATE	5.0f

void CBotPred::_SoundWork( void ) {
	FASSERT( !IsDeadOrDying() );
	if( !m_pHoverAudioEmitter ) {
		m_pHoverAudioEmitter = AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupHover );
#if FANG_WINGC
		if( m_pHoverAudioEmitter ) DEVPRINTF( "Port: jet '%s' hover loop allocated: 3d=%d player=%d.\n",
			Name() ? Name() : "unnamed", m_pHoverAudioEmitter->Is3D(), m_nPossessionPlayerIndex );
#endif
	}

	if( m_pHoverAudioEmitter ) {
		f32 fUnitPowerLevel = m_XlatStickUnitVecXZ_MS.Mag();
		FMATH_CLAMPMIN( fUnitPowerLevel, m_fControls_FlyUp );
		FMATH_CLAMP_UNIT_FLOAT( fUnitPowerLevel );

		m_pHoverAudioEmitter->SetFrequencyFactor( FMATH_FPOT( fUnitPowerLevel, _MIN_HOVER_PITCH, _MAX_HOVER_PITCH ) );
		m_pHoverAudioEmitter->SetVolume( FMATH_FPOT( fUnitPowerLevel, _MIN_HOVER_VOLUME, _MAX_HOVER_VOLUME ) );
		

		if( m_pHoverAudioEmitter->Is3D() ) {
			m_pHoverAudioEmitter->SetPosition( &m_MountPos_WS );
		}
		
	}
	if( !m_pWindAudioEmitter ) {
		m_pWindAudioEmitter = AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupWind );
#if FANG_WINGC
		if( m_pWindAudioEmitter ) DEVPRINTF( "Port: jet '%s' wind loop allocated: 3d=%d player=%d.\n",
			Name() ? Name() : "unnamed", m_pWindAudioEmitter->Is3D(), m_nPossessionPlayerIndex );
#endif
	}

	if( m_pWindAudioEmitter ) {
		f32 fUnitWindLevel = fmath_Div( m_fSpeed_WS, _MAX_VELOCITY_XZ );
		FMATH_CLAMP_UNIT_FLOAT( fUnitWindLevel );
		
		if( fUnitWindLevel > m_fUnitWindLevel ) {
			m_fUnitWindLevel += _WIND_RAMP_UP_RATE * FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MAX1( m_fUnitWindLevel );
		} else {
			m_fUnitWindLevel -= _WIND_RAMP_UP_RATE * FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0( m_fUnitWindLevel );
		}

		m_pWindAudioEmitter->SetFrequencyFactor( FMATH_FPOT( m_fUnitWindLevel, _MIN_WIND_PITCH, _MAX_WIND_PITCH ) );
		m_pWindAudioEmitter->SetVolume( FMATH_FPOT( m_fUnitWindLevel, _MIN_WIND_VOLUME, _MAX_WIND_VOLUME ) );

		if( m_pWindAudioEmitter->Is3D() ) {
			m_pWindAudioEmitter->SetPosition( &m_MountPos_WS );
		}
	}
}

void CBotPred::Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier ) {
	CBot::Possess( nPlayerIndex, fPossessionArmorModifier );

	if (IsInWorld())
	{
		if( m_pHoverAudioEmitter ) {
			m_pHoverAudioEmitter->Destroy();
			m_pHoverAudioEmitter = NULL;
		}

		if( m_pWindAudioEmitter ) {
			m_pWindAudioEmitter->Destroy();
			m_pWindAudioEmitter = NULL;
		}



		//FASSERT( m_pHoverAudioEmitter != NULL );
		//FASSERT( m_pWindAudioEmitter != NULL );

		//if( nPlayerIndex < 0 ) {
		//	m_b3DSounds				= TRUE;

		//	m_pWindAudioEmitter  = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupWind, FALSE, &m_MtxToWorld.m_vPos );
		//	m_pHoverAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupHover, FALSE, &m_MtxToWorld.m_vPos );
		//} else {
		//	m_b3DSounds				= FALSE;

		//	m_pWindAudioEmitter  = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupWind, TRUE );
		//	m_pHoverAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_BotInfo_Pred.pSoundGroupHover, TRUE );
		//}

		// set up some HUD stuff
		CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIndex);
		//pHud->SetDrawFlags( CHud2::DRAW_AMMOBOXES | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR | CHud2::DRAW_SELFDESTRUCT | CHud2::DRAW_WEAPONBOXES | CHud2::DRAW_ENABLEOVERRIDE );
		pHud->SetDrawFlags( CHud2::DRAW_LEFT_AMMOBOX | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR | CHud2::DRAW_SELFDESTRUCT | CHud2::DRAW_LEFT_WEAPONBOX | CHud2::DRAW_ENABLEOVERRIDE );
		CFColorRGBA startColor, endColor;
		//startColor.OpaqueYellow();
		//endColor.OpaqueRed();
		//pHud->OverrideAmmoData( CHud2::RIGHT, CHud2::OVERRIDE_METER_MIL, &m_fChaingunHeat, TRUE, &startColor, &endColor );
		startColor.OpaqueYellow();
		endColor.OpaqueRed();
		pHud->OverrideAmmoData( CHud2::LEFT, CHud2::OVERRIDE_METER_MIL, ((CWeaponQuadLaser*)m_apWeapon[0])->GetUnitSecondaryChargeTimerPtr(), TRUE, &startColor, &endColor );
		pHud->OverrideWeaponBox( TRUE, CHud2::LEFT, "RLauncher L1", TRUE );
	}
}


BOOL CBotPred::_AnimationsActive( void ) {
	// might be a better way to do this...
	// also should probably check bottalks...
	return ( (GetControlValue( ASI_RC_POWER_DOWN ) != 0.0f) ||
			 (GetControlValue( ASI_RC_POWER_UP ) != 0.0f) ||
			 (GetControlValue( ASI_RC_TETHERED ) != 0.0f) );
}



///////////////////////////
//WEAPONS


// Computes the following:
//   m_TargetLockUnitVec_WS
//   m_TargetedPoint_WS
//   m_fAimPitch_WS
//   m_fAimDeltaYaw_MS
//   m_nBotFlags::BOTFLAG_TARGET_LOCKED

void CBotPred::_HandleTargeting( void ) {
	BOOL bAiming;
	
	CBot::HandleTargeting();

	// if human controlled, aim always, otherwise, check to make sure that 1:  AI wants us to aim and 2:  CBot::HandleTargetting() is satisfied 
	// that our target is in a reasonable position.
    if( m_nPossessionPlayerIndex >= 0 ) {
		bAiming = TRUE;

		// keep the target point at least min dist away
		f32 fTgtDist2 = m_MountPos_WS.DistSq( m_TargetedPoint_WS );
		if( fTgtDist2 < m_BotInfo_Pred.fMinTargetDist * m_BotInfo_Pred.fMinTargetDist ) {
			if( fTgtDist2 > 0.001f ) {
				m_TargetedPoint_WS.Sub( m_MountPos_WS );
				m_TargetedPoint_WS.Mul( fmath_InvSqrt( fTgtDist2 ) * m_BotInfo_Pred.fMinTargetDist );
				m_TargetedPoint_WS.Add( m_MountPos_WS );
			} else {
				m_TargetedPoint_WS.Mul( m_MtxToWorld.m_vFront, m_BotInfo_Pred.fMinTargetDist ).Add( m_MountPos_WS );
			}
		}

		// temp
		FocusHumanTargetPoint_WS( & m_TargetedPoint_WS, m_apWeapon[0] );
		//if( m_MountPos_WS.DistSq( m_TargetedPoint_WS ) <  m_BotInfo_Pred.fMinTargetDist * m_BotInfo_Pred.fMinTargetDist ) {
		//	m_TargetedPoint_WS.Sub( m_MountPos_WS );
		//	m_TargetedPoint_WS.Unitize().Mul( m_BotInfo_Pred.fMinTargetDist );
		//	m_TargetedPoint_WS.Add( m_MountPos_WS );
		//}
	} else {
		if( (m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT) && 
			(m_nBotFlags & BOTFLAG_TARGET_LOCKED) && 
			(m_MountPos_WS.DistSq( m_TargetedPoint_WS ) > m_BotInfo_Pred.fMinTargetDist * m_BotInfo_Pred.fMinTargetDist) ) {
			bAiming = TRUE;
		} else {
			bAiming = FALSE;
			if( m_vLastValidTargetPoint_WS.MagSq() == 0.0f ) {
				m_vLastValidTargetPoint_WS.Sub( m_TargetedPoint_WS, m_MountPos_WS );
			}

			if( m_fUnitAim > 0.0f ) {
				m_TargetedPoint_WS.Add( m_MountPos_WS, m_vLastValidTargetPoint_WS );
			}
		}
	}

	if( bAiming && _AnimationsActive() ) {
		bAiming = FALSE;
	}

	// move in/out of aiming state as required
	if( bAiming ) {
		m_fUnitAim += _AIM_OO_ON_TIME * FLoop_fPreviousLoopSecs;
		m_vLastValidTargetPoint_WS.Zero();
		((CWeaponQuadLaser*)m_apWeapon[0])->EnableArmLag( TRUE );
	} else {
		m_fUnitAim -= _AIM_OO_OFF_TIME * FLoop_fPreviousLoopSecs;
		((CWeaponQuadLaser*)m_apWeapon[0])->EnableArmLag( FALSE );
	}
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitAim );

	// if not 100% aimed, we are not ready to fire, clear the flag
	if( m_nBotFlags & BOTFLAG_TARGET_LOCKED ) {
		if( m_fUnitAim < 1.0f ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
		}
	}
}



BOOL CBotPred::GetLimbsStatus(u32* puNumIntact, u32* puNumDangling) {
	u32 uNumIntact = 0;
	u32 uNumDangling = 0;


	//hi mike, I tried my best on these braces, please send me an email at patmagroin@swinginape.com if not.
	if (m_pPartMgr && m_pPartMgr->IsCreated())	{
		for	(u32 i = 0;	i <	CWeaponQuadLaser::ARM_COUNT; i++) {
			switch(m_pPartMgr->GetLimbState( m_auArmLimbTable[i])) {
			case CBotPartMgr::LIMB_STATE_INTACT: 
				uNumIntact++;
				break;
			case CBotPartMgr::LIMB_STATE_DANGLING:
				uNumDangling++;	
				break;
			}
		}

		if (puNumIntact) {
			*puNumIntact = uNumIntact;
		}
		if (puNumDangling)	{
			*puNumDangling = uNumDangling;
		}
	}

	return (uNumIntact || uNumDangling);
}


void CBotPred::_HandleWeaponWork( void ) {
	if( m_fUnitAim == 1.0f ) {
		m_apWeapon[0]->TriggerWork( m_fControls_Fire1, m_fControls_Fire2, &m_TargetedPoint_WS, NULL );
	} else {
		m_apWeapon[0]->TriggerWork( 0.0f, 0.0f, &m_TargetedPoint_WS, NULL );
	}

	WeaponsWork( 0 );

	// if any of the pred's guns fired, add a torque impulse to our motion object...
	if( ((CWeaponQuadLaser*)m_apWeapon[0])->m_nArmFiredThisFrame > -1 ) {
		JustFired();
		s32 sArm = ((CWeaponQuadLaser*)m_apWeapon[0])->m_nArmFiredThisFrame;

		// seems like a good place to test this so we're not doing it for every gun every frame
		
		FASSERT( (sArm >= 0) && (sArm < CWeaponQuadLaser::ARM_COUNT) );
		switch( m_pPartMgr->GetLimbState( m_auArmLimbTable[sArm] ) ) {
			case CBotPartMgr::LIMB_STATE_INTACT: {
				CFVec3A vPush, vArm, vTorque;

				FASSERT( sArm < CWeaponQuadLaser::ARM_COUNT );
				
				((CWeaponQuadLaser*)m_apWeapon[0])->ComputeMuzzlePoint_WS( (u32)sArm, &vArm ); 

				vPush.Mul( m_TargetLockUnitVec_WS, m_Motion.m_fMass * _FIRE_TORQUE_ADJ );
				vArm.Sub( m_MountPos_WS );

				vTorque.Cross( vArm, vPush );
				m_Motion.ApplyTorqueImpulse_WS( &vTorque );
				break;
			}

			case CBotPartMgr::LIMB_STATE_DANGLING:
				break;

			case CBotPartMgr::LIMB_STATE_REMOVED:
				((CWeaponQuadLaser*)m_apWeapon[0])->DisableArm( (u32)sArm );
				break;
		}
	}
}


//////////////////////////
//ANIMATION
void CBotPred::_AnimBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	if (!m_pCBPred)
	{
        rNewMtx.Mul( rParentMtx, rBoneMtx );
		return;
	}
	FASSERT(m_pCBPred->TypeBits() & ENTITY_BIT_BOTPRED);

	CFMtx43A mtxTmp;

	if( m_pCBPred->m_pPartMgr->AnimBoneCallbackFunctionHandler( nBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) ) {
		return;
	} else if( nBoneIndex == m_pCBPred->m_uBoneIndexSpotlight ) {
		// Spotlight (Head)
		m_pCBPred->HeadLookUpdate( rNewMtx, rParentMtx, rBoneMtx, FALSE );
		rNewMtx.m_vFront.Lerp( 0.2f, m_pCBPred->m_Motion.m_Mtx.m_vFront );
		rNewMtx.m_vRight.Lerp( 0.2f, m_pCBPred->m_Motion.m_Mtx.m_vRight );
		rNewMtx.m_vUp.Lerp( 0.2f, m_pCBPred->m_Motion.m_Mtx.m_vUp );
	} else if( nBoneIndex == m_pCBPred->m_uBoneIndexExhaust ) {
		CFMtx43A::m_RotX.SetRotationX( FMATH_FPOT( fmath_UnitLinearToSCurve( m_pCBPred->m_fUnitExhaustPitch ), _EXHAUST_MIN_PITCH, _EXHAUST_MAX_PITCH ) );
		CFMtx43A::m_RotY.SetRotationY( FMATH_FPOT( fmath_UnitLinearToSCurve( m_pCBPred->m_fUnitExhaustYaw ), _EXHAUST_MIN_YAW, _EXHAUST_MAX_YAW ) );
		CFMtx43A::m_Temp.Mul( CFMtx43A::m_RotX, CFMtx43A::m_RotY );
		rNewMtx.Mul( rBoneMtx, CFMtx43A::m_Temp );
		rNewMtx.Mul( rParentMtx, rNewMtx );
	// Arm seg 0	(shoulder)
	} else {
		CWeaponQuadLaser* pWpn = (CWeaponQuadLaser*)(m_pCBPred->m_apWeapon[0]);
		if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg0TL ) {
			pWpn->AimSeg0( CWeaponQuadLaser::ARM_TL, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg0TR ) {
			pWpn->AimSeg0( CWeaponQuadLaser::ARM_TR, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg0BL ) {
			pWpn->AimSeg0( CWeaponQuadLaser::ARM_BL, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg0BR ) {
			pWpn->AimSeg0( CWeaponQuadLaser::ARM_BR, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );

		// Arm Seg 1	(elbow)
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg1TL ) {
			pWpn->AimSeg1( CWeaponQuadLaser::ARM_TL, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg1TR ) {
			pWpn->AimSeg1( CWeaponQuadLaser::ARM_TR, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg1BL ) {
			pWpn->AimSeg1( CWeaponQuadLaser::ARM_BL, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg1BR ) {
			pWpn->AimSeg1( CWeaponQuadLaser::ARM_BR, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );

		// Arm Seg 2	(gun)
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg2TL ) {
			pWpn->AimSeg2( CWeaponQuadLaser::ARM_TL, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg2TR ) {
			pWpn->AimSeg2( CWeaponQuadLaser::ARM_TR, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg2BL ) {
			pWpn->AimSeg2( CWeaponQuadLaser::ARM_BL, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		} else if( nBoneIndex == m_pCBPred->m_uBoneIndexArmSeg2BR ) {
			pWpn->AimSeg2( CWeaponQuadLaser::ARM_BR, m_pCBPred->m_fUnitAim, rNewMtx, rParentMtx, rBoneMtx );
		}
	}
}

void CBotPred::UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) {
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


//////////////////////////
//DEBUG


void CBotPred::DebugDraw( CBotPred *pPred ) {
#if 0
	//fdraw_FacetedWireSphere( &pPred->m_TargetedPoint_WS.v3, 4.0f, 4, 4, &FColor_MotifGreen );
	//fdraw_FacetedWireSphere( &pPred->m_ControlBot_TargetPoint_WS.v3, 4.0f, 4, 4, &FColor_MotifBlue );
	//((CWeaponQuadLaser*)(pPred->m_apWeapon[0]))->DebugDraw();
	return;
	const CFMtx43A *pMtx;
	CFVec3A vStart, vEnd;

	return;


	fdraw_FacetedWireSphere( &pPred->m_TargetedPoint_WS.v3, 4.0f, 4, 4, &FColor_MotifGreen );

	pMtx = pPred->m_pWorldMesh->GetBoneMtxPalette()[pPred->m_uBoneIndexSpotlight];

	vStart = pMtx->m_vPos;
	vEnd.Mul( pMtx->m_vFront, 1000.0f );
	vEnd.Add( pMtx->m_vPos );

	fdraw_SolidLine( &vStart.v3, &vEnd.v3, &FColor_MotifBlue );

	// now the arms

	pMtx = pPred->m_pWorldMesh->GetBoneMtxPalette()[pPred->m_uBoneIndexArmSeg2TL];
	vStart = pMtx->m_vPos;
	vEnd.Mul( pMtx->m_vFront, 1000.0f );
	vEnd.Add( vStart );
	fdraw_SolidLine( &vStart.v3, &vEnd.v3, &FColor_MotifRed );

	pMtx = pPred->m_pWorldMesh->GetBoneMtxPalette()[pPred->m_uBoneIndexArmSeg2TR];
	vStart = pMtx->m_vPos;
	vEnd.Mul( pMtx->m_vFront, 1000.0f );
	vEnd.Add( vStart );
	fdraw_SolidLine( &vStart.v3, &vEnd.v3, &FColor_MotifRed );

	pMtx = pPred->m_pWorldMesh->GetBoneMtxPalette()[pPred->m_uBoneIndexArmSeg2BL];
	vStart = pMtx->m_vPos;
	vEnd.Mul( pMtx->m_vFront, 1000.0f );
	vEnd.Add( vStart );
	fdraw_SolidLine( &vStart.v3, &vEnd.v3, &FColor_MotifRed );

	pMtx = pPred->m_pWorldMesh->GetBoneMtxPalette()[pPred->m_uBoneIndexArmSeg2BR];
	vStart = pMtx->m_vPos;
	vEnd.Mul( pMtx->m_vFront, 1000.0f );
	vEnd.Add( vStart );
	fdraw_SolidLine( &vStart.v3, &vEnd.v3, &FColor_MotifRed );
    
	((CWeaponQuadLaser*)(pPred->m_apWeapon[0]))->DebugDraw();

	return;

	vStart = pPred->m_MountPos_WS.v3;
	vEnd   = pPred->m_MountUnitFrontXZ_WS.v3;
	vEnd *= 100.0f;
	vEnd += vStart;

	

	fdraw_FacetedWireSphere( &pPred->m_MountPos_WS.v3, 2.0f, 4, 4, &FColor_MotifRed );
//	fdraw_FacetedWireSphere( &m_CollSphere.m_Pos, m_CollSphere.m_fRadius, 4, 4, &FColor_MotifBlue );
//	fdraw_FacetedWireSphere( &pPred->m_ModelLagPos.v3, 2.0f, 4, 4, &FColor_MotifBlue );
//	fdraw_FacetedWireSphere( &_TempPredPos.v3, 2.0f, 4, 4, &FColor_MotifBlue );
	fdraw_SolidLine( &pPred->m_ModelLagPos.v3, &pPred->m_MountPos_WS.v3, &FColor_MotifGreen );

	fdraw_SolidLine( &vStart, &vEnd, &FColor_MotifGreen );
	
	vEnd = pPred->m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.v3;
	vEnd *= 100.0f;
	vEnd += vStart;

	fdraw_SolidLine( &vStart, &vEnd, &FColor_MotifRed );

	fdraw_FacetedWireSphere( &pPred->m_MountPos_WS.v3, 5.0f, 4, 4, &FColor_MotifGreen );

	
//	fdraw_FacetedWireSphere( &_tmpvForcePos.v3, 2.0f, 4, 4, &FColor_MotifBlue );	
	CFVec3A vTmp;
	vTmp.Mul( pPred->m_Motion.m_AngularVelocity_WS, 20.0f );
	vTmp.Add( pPred->m_Motion.m_Mtx.m_vPos );
	fdraw_FacetedWireSphere( &vTmp.v3, 2.0f, 4, 4, &FColor_MotifGreen );

//	vTmp.Mul( _tmpvTorqueAxis, 20.0f );
//	vTmp.Add( pPred->m_Motion.m_Mtx.m_vPos );
//	fdraw_FacetedWireSphere( &vTmp.v3, 2.5f, 4, 4, &FColor_MotifRed );
#endif
}


void CBotPred::InflictDamageResult( const CDamageResult *pDamageResult ) {
	CBot::InflictDamageResult( pDamageResult );

#if 0
	// SER: This would be cool if it worked. Couldn't get it to work reliably.
	//      The Predator would just spin out of control.
	if( IsDead() ) {
		return;
	}

	if( pDamageResult->m_fImpulseMag == 0.0f ) {
		return;
	}

	CFVec3A Force_WS;

	Force_WS.Mul( pDamageResult->m_Impulse_WS, 0.25f );

	m_Motion.ApplyImpulse_WS( &pDamageResult->m_pDamageData->m_ImpactPoint_WS, &Force_WS );
#endif
}


void CBotPred::Die( BOOL bSpawnDeathEffects, BOOL bSpawnGoodies ) {
	BOOL bWasAliveUntilVeryRecently = m_uLifeCycleState == BOTLIFECYCLE_LIVING;

	CBot::Die( bSpawnDeathEffects, bSpawnGoodies );

	if( !bWasAliveUntilVeryRecently ) {
		// Why die again if already dead or dying?
		return;
	}


	FMATH_SETBITMASK( m_uBotDeathFlags, BOTDEATHFLAG_WALKING_DEAD );

	IgnoreBotVBotCollision();

	_RemoveAllAliveEffects();

	m_Motion.m_Mtx.m_vPos = m_ModelLagPos;
	m_ModelLagPos.Zero();

	m_MountPos_WS = m_Motion.m_Mtx.m_vPos;

	m_DeathRotUnitAxis_WS.Set( 0.5f, 0.25f, 1.0f ).Unitize();
	m_Motion.m_Mtx.MulDir( m_DeathRotUnitAxis_WS );

	m_fDeathUnitRotVel = 0.0f;

	m_pPartMgr->SetState_Dangle( LIMB_TYPE_TOP_RIGHT_ARM, TRUE, _DEATH_LIMB_GRAVITY );
	m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOTTOM_RIGHT_ARM, TRUE, _DEATH_LIMB_GRAVITY );
	m_pPartMgr->SetState_Dangle( LIMB_TYPE_TOP_LEFT_ARM, TRUE, _DEATH_LIMB_GRAVITY );
	m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOTTOM_LEFT_ARM, TRUE, _DEATH_LIMB_GRAVITY );

	_DeathWork_SpawnExplosion( m_BotInfo_Pred.hExplosionBig, 0.0f );
	m_fDeathBigExplosionCountdownSecs = _DEATH_INITIAL_TIME_BETWEEN_EXPLOSIONS;
	m_fDeathTinyExplosionCountdownSecs = 0.0f;

	m_fDeathLimbBlowCountdownSecs = _DEATH_INITIAL_LIMB_BLOW_TIME;
	m_nDeathLimbBlowIndex = 0;

	m_fDeathMaxDestructionCountdownSecs = _DEATH_MAX_DESTRUCTION_TIME;
}


void CBotPred::DeathWork( void ) {
	DeltaTime( ASI_STAND );

	_DeathWork_UpdatePos();
	_DeathWork_UpdateRot();
	_DeathWork_UpdateExplosion();

	m_pWorldMesh->m_Xfm.BuildFromMtx( m_Motion.m_Mtx );
	m_pWorldMesh->UpdateTracker();

	ComputeMtxPalette( FALSE );

	CFMtx43A::m_Temp.SetRotationYXZ( m_fMountYaw_WS, m_fMountPitch_WS, 0.0f );
	CFMtx43A::m_Temp.m_vPos = m_MountPos_WS;
	Relocate_RotXlatFromUnitMtx_WS( &CFMtx43A::m_Temp, TRUE, m_pMoveIdentifier );

	m_fDeathMaxDestructionCountdownSecs -= FLoop_fPreviousLoopSecs;
	if( m_fDeathMaxDestructionCountdownSecs <= 0.0f ) {
		_DeathWork_SpawnExplosion( m_BotInfo_Pred.hExplosionFinal, 0.0f );
		m_uLifeCycleState = BOTLIFECYCLE_DEAD;
		RemoveFromWorld();
	}
}


void CBotPred::_DeathWork_UpdatePos( void ) {
	CFVec3A DeltaPos_WS, PrevPos_WS;
	FCollImpact_t CollImpact;

	PrevPos_WS = m_MountPos_WS;

	m_Velocity_WS.y += _DEATH_Y_ACCELERATION * FLoop_fPreviousLoopSecs;

	if( m_Velocity_WS.x > 0.0f ) {
		m_Velocity_WS.x -= _DEATH_XZ_DECELERATION * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_Velocity_WS.x, 0.0f );
	} else if( m_Velocity_WS.x < 0.0f ) {
		m_Velocity_WS.x += _DEATH_XZ_DECELERATION * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_Velocity_WS.x, 0.0f );
	}

	if( m_Velocity_WS.z > 0.0f ) {
		m_Velocity_WS.z -= _DEATH_XZ_DECELERATION * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_Velocity_WS.z, 0.0f );
	} else if( m_Velocity_WS.z < 0.0f ) {
		m_Velocity_WS.z += _DEATH_XZ_DECELERATION * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_Velocity_WS.z, 0.0f );
	}

	DeltaPos_WS.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );

	m_Motion.m_Mtx.m_vPos.Add( DeltaPos_WS );
	m_MountPos_WS = m_Motion.m_Mtx.m_vPos;

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &PrevPos_WS, &m_MountPos_WS, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, m_pWorldMesh, -1, FCOLL_MASK_COLLIDE_WITH_NPCS | FCOLL_MASK_COLLIDE_WITH_PLAYER | FCOLL_MASK_COLLIDE_WITH_OBJECTS ) ) {
		_DeathWork_SpawnExplosion( m_BotInfo_Pred.hExplosionFinal, 0.0f );
		m_uLifeCycleState = BOTLIFECYCLE_DEAD;
		RemoveFromWorld();
	}
}


void CBotPred::_DeathWork_UpdateRot( void ) {
	CFQuatA NewModelQuat, OrigModelQuat, RotateQuat;
	f32 fRotSpeed, fHalfAngle, fSin, fCos;

	m_fDeathUnitRotVel += (1.0f / _DEATH_ROT_TIME) * FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMAX( m_fDeathUnitRotVel, 1.0f );

	fRotSpeed = _DEATH_ROT_MAX_SPEED * (0.5f + 0.5f * fmath_Cos( m_fDeathUnitRotVel * (0.75f * FMATH_2PI) - FMATH_QUARTER_PI ));
	FMATH_CLAMPMIN( fRotSpeed, _DEATH_ROT_MIN_SPEED );

	OrigModelQuat.BuildQuat( m_Motion.m_Mtx );

	fHalfAngle = fRotSpeed * FLoop_fPreviousLoopSecs;
	fmath_SinCos( fHalfAngle, &fSin, &fCos );

	RotateQuat.x = m_DeathRotUnitAxis_WS.x * fSin;
	RotateQuat.y = m_DeathRotUnitAxis_WS.y * fSin;
	RotateQuat.z = m_DeathRotUnitAxis_WS.z * fSin;
	RotateQuat.w = fCos;

	NewModelQuat.Mul( RotateQuat, OrigModelQuat );

	NewModelQuat.BuildMtx33( m_Motion.m_Mtx );
}


void CBotPred::_DeathWork_UpdateExplosion( void ) {
	static const u32 __anLimbType[4] = { LIMB_TYPE_BOTTOM_LEFT_ARM, LIMB_TYPE_BOTTOM_RIGHT_ARM, LIMB_TYPE_TOP_LEFT_ARM, LIMB_TYPE_TOP_RIGHT_ARM };

	m_fDeathBigExplosionCountdownSecs -= FLoop_fPreviousLoopSecs;
	if( m_fDeathBigExplosionCountdownSecs <= 0.0f ) {
		m_fDeathBigExplosionCountdownSecs = fmath_RandomFloatRange( _DEATH_MIN_TIME_BETWEEN_BIG_EXPLOSIONS, _DEATH_MAX_TIME_BETWEEN_BIG_EXPLOSIONS );

		_DeathWork_SpawnExplosion( m_BotInfo_Pred.hExplosionSecondary, _DEATH_BIG_EXPLOSION_RADIUS );
	}

	m_fDeathTinyExplosionCountdownSecs -= FLoop_fPreviousLoopSecs;
	if( m_fDeathTinyExplosionCountdownSecs <= 0.0f ) {
		m_fDeathTinyExplosionCountdownSecs = fmath_RandomFloatRange( _DEATH_MIN_TIME_BETWEEN_TINY_EXPLOSIONS, _DEATH_MAX_TIME_BETWEEN_TINY_EXPLOSIONS );

		_DeathWork_SpawnExplosion( m_BotInfo_Pred.hExplosionTiny, _DEATH_TINY_EXPLOSION_RADIUS );

		if( m_pDeathBurnSmokeParticle == NULL ) {
			_StartBurnSmoke();
		}
	}

	if( m_nDeathLimbBlowIndex < 4 ) {
		m_fDeathLimbBlowCountdownSecs -= FLoop_fPreviousLoopSecs;
		if( m_fDeathLimbBlowCountdownSecs <= 0.0f ) {
			m_fDeathLimbBlowCountdownSecs = fmath_RandomFloatRange( _DEATH_LIMB_BLOW_MIN_TIME, _DEATH_LIMB_BLOW_MAX_TIME );

			if( m_pPartMgr->GetLimbState( __anLimbType[m_nDeathLimbBlowIndex] ) != CBotPartMgr::LIMB_STATE_REMOVED ) {
				s32 nBoneIndex;

				switch( m_nDeathLimbBlowIndex ) {
				case 0:
					nBoneIndex = m_uBoneIndexArmSeg2BL;
					break;

				case 1:
					nBoneIndex = m_uBoneIndexArmSeg2BR;
					break;

				case 2:
					nBoneIndex = m_uBoneIndexArmSeg2TL;
					break;

				case 3:
					nBoneIndex = m_uBoneIndexArmSeg2TR;
					break;

				default:
					FASSERT_NOW;
				}

				if( nBoneIndex >= 0 ) {
					if( m_BotInfo_Pred.hExplosionTiny != FEXPLOSION_INVALID_HANDLE ) {
						FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

						if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
							FExplosionSpawnParams_t SpawnParams;

							SpawnParams.InitToDefaults();

							SpawnParams.Pos_WS = m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vPos;

							SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;

							CExplosion2::SpawnExplosion( hSpawner, m_BotInfo_Pred.hExplosionTiny, &SpawnParams );
						}
					}
				}

				m_pPartMgr->SetState_BlowUp( __anLimbType[m_nDeathLimbBlowIndex] );
			}

			++m_nDeathLimbBlowIndex;
		}
	}
}


void CBotPred::_DeathWork_SpawnExplosion( FExplosion_GroupHandle_t hExplosion, f32 fRadius ) {
	if( hExplosion != FEXPLOSION_INVALID_HANDLE ) {
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();

			SpawnParams.Pos_WS.Set( fRadius, 0.0f, 0.0f );
			SpawnParams.Pos_WS.RotateY( fmath_RandomFloatRange( 0.0f, FMATH_2PI ) );
			SpawnParams.Pos_WS.Add( m_MtxToWorld.m_vPos );

			SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;

			CExplosion2::SpawnExplosion( hSpawner, hExplosion, &SpawnParams );
		}
	}
}


void CBotPred::_StartBurnSmoke( void ) {
	if( m_BotInfo_Pred.hParticleBurnSmoke != FPARTICLE_INVALID_HANDLE ) {
		_StopBurnSmoke();

		m_pDeathBurnSmokeParticle = eparticlepool_GetEmitter();
		if( m_pDeathBurnSmokeParticle ) {
			CFMtx43A Mtx;

			Mtx.Identity33();
			Mtx.m_vPos = m_MtxToWorld.m_vPos;

			m_pDeathBurnSmokeParticle->StartEmission( m_BotInfo_Pred.hParticleBurnSmoke );

			m_pDeathBurnSmokeParticle->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &Mtx, 1.0f, FALSE );
			m_pDeathBurnSmokeParticle->Attach_ToParent_WS( this );
		}
	}
}


void CBotPred::_StopBurnSmoke( void ) {
	if( m_pDeathBurnSmokeParticle ) {
		m_pDeathBurnSmokeParticle->DetachFromParent();
		m_pDeathBurnSmokeParticle->StopEmission();
		m_pDeathBurnSmokeParticle = NULL;
	}
}


void CBotPred::SpawnDeathEffects( void ) {
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPredBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


void CBotPredBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_BOTPRED, pszEntityType );

	m_nEC_HealthContainerCount	= CBotPred::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile		= CBotPred::m_BotInfo_Gen.pszArmorProfile;
	m_fMaxAltitude				= FMATH_MAX_FLOAT;
	m_fMaxSpeed					= _MAX_VELOCITY_XZ;

	// This class of bots can be recruited...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED );

	// This class of bots shows up on Glitch's radar...
	FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_SHOWS_UP_ON_RADAR );
}


BOOL CBotPredBuilder::PostInterpretFixup( void ) {
	if( !CBotBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CBotPredBuilder::InterpretTable( void ) {

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "MaxAltitude" ) ) {
		if( !CEntityParser::Interpret_F32( &m_fMaxAltitude ) ) {
			CEntityParser::Error_InvalidParameterValue();
		}

		return TRUE;
	}else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PredSpeed" ) ) {
		if( !CEntityParser::Interpret_F32( &m_fMaxSpeed ) ) {
			CEntityParser::Error_InvalidParameterValue();
		}

		return TRUE;
	}


	return CBotBuilder::InterpretTable();
}
