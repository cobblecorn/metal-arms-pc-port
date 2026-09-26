//////////////////////////////////////////////////////////////////////////////////////
// vehiclerat.cpp -
//
// Author: Chris MacDonald
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
// 11/07/02 MacDonald     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "vehiclerat.h"
#include "FCheckPoint.h"
#include "fsound.h"
#include "meshtypes.h"
#include "gamepad.h"
#include "fresload.h"
#include "player.h"
#include "botpart.h"
#include "fscriptsystem.h"
#include "eparticlepool.h"
#include "AI/AIAPI.h"
#include "AI/AIEnviro.h"
#include "AI/AIBrain.h"
#include "AI/AIBrainman.h"
#include "AI/AIWeaponctrl.h"
#include "AI/AImover.h"
#include "weapon.h"
#include "MultiplayerMgr.h"
#include "meshentity.h"
#include "site_ratgun.h"
#include "eboomer.h"

#include "protrack.h"

extern CFPhysicsObject *_pDrivenPhysObj;

static cchar _apszRatMeshFilename[] = "GRMRRata_00";
static cchar _apszDroidRatMeshFilename[] = "GRDRRata_00";
static cchar _apszDeadRatMeshFilename[] = "GRMRratAD01";
static cchar _apszDeadDroidRatMeshFilename[] = "GRDRratAD01";

#define _DRIVER_EXIT_ON_FLIP			( 0 )

#define _BOTINFO_FILENAME				( "b_vrat" )
#define _DIRT_PARTICLE_DEF_NAME			( "dirtyjr01" )
#define _CLOUD_PARTICLE_DEF_NAME		( "rat_dust01" )
#define _WATER_PARTICLE_DEF_NAME		( "e_ratsplsh" )
#define _TOTAL_NUM_WEAPONS				( 2 )
#define _BOTPART_FILENAME				( "bp_rat" )
#define _EXPLOSION_GROUP_NAME			( "Scout" )

#define _HEADLIGHT_TEXTURE_ID			( 1 )
#define _BRAKE_LIGHT_TEXTURE_ID			( 1 )
#define _REVERSE_LIGHT_TEXTURE_ID		( 2 )
#define _TIRE_TEXTURE_ID				( 3 )

#define _ZOBBY_MESH_NAME				( "GRDZzobby01" )
#define _ZOBBY_ANIM_NAME				( "ARDZgunner1" )

#define _ENTITY_BITS_TO_SKIP_COLLISION (ENTITY_BIT_WEAPON)


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleRatBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CVehicleRatBuilder _VehicleRatBuilder;

void CVehicleRatBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) 
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CVehicleBuilder, ENTITY_BIT_VEHICLERAT, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CVehicleRat::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CVehicleRat::m_BotInfo_Gen.pszArmorProfile;
	m_pszEC_DamageProfile = CVehicleRat::m_BotInfo_Vehicle.pszDamageProfile;

	m_uFlags = 0;
}


BOOL CVehicleRatBuilder::PostInterpretFixup( void ) 
{
	if( !CBotBuilder::PostInterpretFixup() ) 
	{
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}


BOOL CVehicleRatBuilder::InterpretTable( void ) 
{
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "DroidRatMesh" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, RAT_BUILDER_FLAG_DROID_MESH );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "RatDeathFlip" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, RAT_BUILDER_FLAG_DEATH_FLIP );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AIStayBehind" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, RAT_BUILDER_FLAG_AI_STAY_BEHIND );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AIStayBeside" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, RAT_BUILDER_FLAG_AI_STAY_BESIDE );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ZobbyDriving" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, RAT_BUILDER_FLAG_ZOBBY_DRIVER );

		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ZobbyGunning" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, RAT_BUILDER_FLAG_ZOBBY_GUNNER );

		return TRUE;
	}

	return CVehicleBuilder::InterpretTable();
}


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleRat
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
#define _MIN_TIME_BETWEEN_COLLIDE_FLAG_SET	( 0.5f )
#define _MIN_CLOUD_SPEED				( 20.0f )	// minimum speed at which dust cloud particle emitter activates

// weapons
#define _GUN_CAGE_TIME					( 0.5f )	// how long it takes to open/close gun cage
#define _GUN_TRANSITION_TIME			( _GUN_CAGE_TIME + 0.1f )	// state machine wait time
#define _GUNNERCAM_ELEVATION_ANGLE		( FMATH_DEG2RAD( 0.0f ) )

// physics & collision
#define _DRAW_SPHERES					( 0 )		// set to 1 to draw wireframe collision spheres
#define _PLAYER_MASS_MUL				( 1.3f )	// mass multiplier for player-driven vehicles
#define _WHEEL_RADIUS					( 4.00f )	// radius of collision spheres around RAT wheels (spheres now are larger than wheels)
#define _OO_WHEEL_RADIUS				( 1.0f/ _WHEEL_RADIUS )
#define _SMOOTHED_CONTACT_TIME			( 0.3f )	// minimum time that vehicle must not be touching ground before SmoothContact() is FALSE
#define _FRICTION_CONTACT_TIME			( 0.1f )	// time within which to apply friction after contact with ground is broken
#define _OO_NUM_WHEELS					( 1.0f / (f32) CVehicleRat::NUM_WHEELS )
#define _TIRE_ROLLING_FRICTION			( 0.05f )
#define _LOW_TRACTION_KT				( 0.005f )	// kinetic transition for low-friction surface
#define _HIGH_TRACTION_KT				( 0.01f )	// kinetic transition for high-friction surface
#define _KINETIC_TRANS_BALANCE			( 0.00f )	// variance of transition from front wheels to back wheels
#define _STATIC_TRANS_HYST				( 0.90f )	// fraction of force which must be reduced to before static friction is restored
#define _KINETIC_FORCE_MUL				( 0.65f )	// multiplier for applied forces when under kinetic friction
#define _SUSPENSION_OFFSET				( 0.2f )	// offset to spring length to lift suspension up a bit for visual reasons (looks less extended)
#define _SPRING_STRETCHED_LENGTH		( 2.5f )	// maximum length of vehicle shock absorbers
#define _SPRING_COMPRESSED_LENGTH		( 0.0f )	// minimum length of vehicle shock absorbers
#define _SPRING_REST_LENGTH				( 3.0f )	// rest length of vehicle shock absorbers
#define _SPRING_APPLIED_FORCE_RATIO		( 4.0f )	// amount of force present at collision point that will be applied to the spring
#define _SPRING_COEFFICIENT				( 200.0f )	// stiffness of shock absorbers
#define _SPRING_COMPRESSION_RATIO		( 0.5f )	// ratio of compression damping to stretch damping
#define _SPRING_DAMPING_RATIO			(0.40f)		// damping of shock absorbers as a fraction of stiffness
#define _SPRING_BALANCE					( 0.0f )	// balances spring coefficients front-to-back. range -1.0 to 1.0. Positive numbers make front springs stiffer.
#define _GROUND_GRAVITY					( -180.0f )	// physics object gravity constant when vehicle is on the ground
#define _AIR_GRAVITY					( -130.0f )	// gravity constant while airborne (to enhance jumps)
#define _GRAVITY_TRANSITION_HEIGHT		( 30.0f )	// height above ground over which gravity transitions from ground gravity to air gravity
#define _OO_GRAVITY_TRANSITION_HEIGHT	( 1.0f / _GRAVITY_TRANSITION_HEIGHT )

#define _MIN_GROUND_LINEAR_DAMPING		( 0.3f )	// minimum dynamic damping
#define _MIN_GROUND_ANGULAR_DAMPING		( 150.0f )	
#define _GROUND_LINEAR_DAMPING			( 70.0f )	// dyamic linear damping of physics object while on the ground
#define _AIR_LINEAR_DAMPING				( 0.1f )	// linear damping of physics object while airborne
#define _GROUND_ANGULAR_DAMPING			( 3000.0f )	// dynamic angular damping of physics object while on the ground
#define _AIR_ANGULAR_DAMPING			( 50.0f )	// angular damping of physics object while airborne
#define _DYNAMIC_DAMPING_VELOCITY		( 5.0f )
#define _DYNAMIC_DAMPING_ANGULAR_VEL	( 5.0f )
#define _DYNAMIC_DAMPING_FORCE			( 20000.0f )

#define _CM_OFFSET_Y					( 3.0f )	// offset from dummy bone to center of mass
#define _CM_OFFSET_Z					( 0.0f )
#define _WHEEL_SINK						( 0.3f + 1.25f )	// sink wheels into ground so tires look flattened + offset to make large wheel spheres contact ground correctly
#define _ELASTICITY						( 0.0f )
#define _OTHER_OBJ_ELASTICITY			( 0.0f )
#define _PLAYER_OTHER_OBJ_ELASTICITY	( 0.8f )
#define _MIN_SKID_VELOCITY				( 8.0f )
#define _MAX_SKID_VELOCITY				( 25.0f )
#define _OO_MAX_SKID_VELOCITY			( 1.0f / _MAX_SKID_VELOCITY )

// camera
#define _CAMERA_TRANSITION_TIME			( 1.0f )
#define _CAM_HEIGHT_DEAD_ZONE			( 0.3f )	// unitized dead zone inside which driver camera height won't change

// movement
#define _MAX_WHEEL_BLUR_VEL_MUL			( _OO_WHEEL_RADIUS  * 0.38f )
#define _MAX_REVERSE_THROTTLE			( 0.3f )
#define _MIN_RAMMING_VELOCITY			( 15.0f )
#define _FORWARD_FORCE					(235000.0f )	// maximum forward force that can be applied to a vehicle by throttle
#define _PLAYER_FORWARD_FORCE_MUL		( 1.25f )		// forward force multiplier for player-driven vehicles
#define _ADDED_FORWARD_STEERING_FORCE	( 100000.0f );
#define _BRAKING_FORCE					( 1000.0f )
#define _SIDE_FORCE						( 2000.0f )
#define _ROT_FORCE						( 30000.0f )
#define _ROT_POINT						( 10.0f )
#define _JUMP_FORCE						( 500000.0f )
#define _ROT_SPEED						( FMATH_DEG2RAD( 90.0f ) )
#define _FLIP_TIME						( 1.7f )
#define _MAX_FLIP_VEL_SQ				( 256.0f )
#define _FLIP_UPRIGHT_MOMENTUM			( 150000.0f )
#define _RIGHTING_FORCE					( 0.0f )	// force to assist vehicle in staying upright
#define _RIGHTING_ARM					( 5.0f )			// arm length at which righting force is applied
#define _BRAKE_BALANCE					( -1.0f )	// 0.0 = neutral, 1.0 = max rear, -1.0 = max front
#define _MAX_STEERING_RATE				( 6.0f )	// speed at which wheels turn
#define _HIGH_SPEED_MAX_STEERING		( 1.0f )

#define _BASE_FRONT_AXLE_TURN_ANGLE		( FMATH_DEG2RAD( 10.0f ) )
#define _VAR_FRONT_AXLE_TURN_ANGLE		( FMATH_DEG2RAD( 15.0f ) )
#define _BASE_REAR_AXLE_TURN_ANGLE		( FMATH_DEG2RAD( 4.0f ) )
#define _VAR_REAR_AXLE_TURN_ANGLE		( FMATH_DEG2RAD( 18.0f ) )

// engine sfx
#define _RAT_SOUND_BANK					( "Rat" )
#define _ENGINE_SFX_RADIUS				( 100.0f )
#define _ENGINE_SFX_VOLUME				( 0.3f )
#define _ENGINE_START_TIME				( 0.7f )
#define _ENGINE_STOP_TIME				( 1.0f )

#define _CLOUD_INWARD_OFFSET			( 4.0f )

// wheel animations
#define _DIRT_KINETIC_INTENSITY_MUL		( 2.0f )
#define _MIN_DIRT_KINETIC_INTENSITY		( 0.4f )
#define _MIN_DIRT_INTENSITY				( 0.4f )
#define _MIN_DIRT_VEL					( 20.0f )
#define _MIN_DIRT_VEL_SQ				( _MIN_DIRT_VEL * _MIN_DIRT_VEL )
#define _MAX_DIRT_VEL					( 75.0f )
#define _MAX_DIRT_VEL_SQ				( _MAX_DIRT_VEL * _MAX_DIRT_VEL )
#define _MAX_ROOSTER_VEL				( 25.0f )
#define _DIRT_EMISSION_ANGLE			( 30.0f )
#define _MIN_TIRE_BLUR_BLEND			( 0.1f )

#define _DRIVING_STATION_COLLSPHERE_RADIUS	( 2.0f )	// radius of occupation-determining collision sphere around driving station
#define _GUNNING_STATION_COLLSPHERE_RADIUS	( 4.0f )	// radius of occupation-determining collision sphere around gunning station

// vehicle destruction
#define _MIN_DEATH_EVENT_TIME			( 0.07f )
#define _MAX_DEATH_EVENT_TIME			( 0.3f )
#define _MIN_DEBRIS_WHEEL_VEL_ADD		( 40.0f )
#define _MAX_DEBRIS_WHEEL_VEL_ADD		( 80.0f )
#define _NUM_DEATH_EVENTS				( 17 )
#define _SWITCH_MESH_DEATH_EVENT		( 9 )
#define _MIN_EXPLOSION_FORCE			( 100.0f )
#define _MAX_EXPLOSION_FORCE			( 100.0f )
#define _START_SMOKE_HEALTH				( 0.3f )

#define _FRONT_STATIC_FRICTION			( 4.0f )
#define _REAR_STATIC_FRICTION			( 4.0f )
#define _FRONT_KINETIC_FRICTION			( 2.0f )
#define _REAR_KINETIC_FRICTION			( 2.0f )

#if ENABLE_BOOST_FEATURE
#define _BOOST_TIME						( 7.0f )
#define _BOOST_RECOVERY_TIME			( 10.0f )
#define _MAX_BOOST_VEL_MUL				( 1.35f )
#define _BOOST_LIFT_TIME				( 1.0f )
#define _BOOST_LIFT_FORCE				( 0 ) //( 90000.0f )
#define _BOOST_LIFT_ARM					( 15.0f )
#define _BOOST_FOV_TIME					( 1.0f )
#define _BOOST_DELTA_FOV				( FMATH_DEG2RAD( 6.0f ) ) //( FMATH_DEG2RAD( 12.0f ) )
#define _BOOST_GRAVITY_MULTIPLIER		( 1.0f )
#endif

#define _VEHICLE_DAMAGE_DELAY			( 0.2f )
#define _BOOMER_DAMAGE_DELAY			( 0.2f )
#define _BOT_DAMAGE_DELAY				( 0.1f )
BOOL CVehicleRat::m_bSystemInitialized = FALSE;
CBotAnimStackDef CVehicleRat::m_AnimStackDef;
CBot *CVehicleRat::m_pCurrentCollider;
CFSphere CVehicleRat::m_RatColSphere;
FParticle_DefHandle_t CVehicleRat::m_hDirtParticleDef;
FParticle_DefHandle_t CVehicleRat::m_hCloudParticleDef;
FParticle_DefHandle_t CVehicleRat::m_hWaterParticleDef;
CBot::BotInfo_Gen_t CVehicleRat::m_BotInfo_Gen;
CVehicleRat::BotInfo_Rat_t CVehicleRat::m_BotInfo_Rat;
CVehicleRat::BotInfo_VehiclePhysics_t CVehicleRat::m_BotInfo_VehiclePhysics;
CVehicleCamera::BotInfo_VehicleCamera_t CVehicleRat::m_BotInfo_DriverCamera;
CVehicleCamera::BotInfo_VehicleCamera_t CVehicleRat::m_BotInfo_MPDriverCamera;
CVehicleCamera::BotInfo_VehicleCamera_t CVehicleRat::m_BotInfo_GunnerCamera;
CVehicleCamera::BotInfo_VehicleCamera_t CVehicleRat::m_BotInfo_MGGunnerCamera;
CVehicle::BotInfo_Vehicle_t CVehicleRat::m_BotInfo_Vehicle;
CVehicle::BotInfo_Engine_t CVehicleRat::m_BotInfo_Engine;
CFCollData CVehicleRat::m_RatCollData;
CBotPartPool *CVehicleRat::m_pPartPool;
u32 CVehicleRat::m_nBotClassClientCount = 0;
FMesh_t	*CVehicleRat::m_pRatMesh;
FMesh_t	*CVehicleRat::m_pDeadRatMesh;
FMesh_t	*CVehicleRat::m_pDroidRatMesh;
FMesh_t	*CVehicleRat::m_pDeadDroidRatMesh;
u32 CVehicleRat::m_anBoneIndexWheel[RAT_TYPE_COUNT][ NUM_WHEELS ];	// bone indices of wheels
u32 CVehicleRat::m_anBoneIndexAxle[RAT_TYPE_COUNT][ NUM_WHEELS ];	// bone indices of axles
u32 CVehicleRat::m_anBoneIndexHubcap[RAT_TYPE_COUNT][ NUM_WHEELS ];	// bone indices of "hubcaps" (wheel vertical support arm)
u32 CVehicleRat::m_nBoneIdxDriverAttach[RAT_TYPE_COUNT];			// bone index of bot driving position
u32 CVehicleRat::m_nBoneIdxGunnerAttach[RAT_TYPE_COUNT];			// bone index of gunning position
u32 CVehicleRat::m_nBoneIndexRightHatch[RAT_TYPE_COUNT];			// bone index of driver's hatch door
u32 CVehicleRat::m_nBoneIndexTurretAttach[RAT_TYPE_COUNT];			// rat gun attach point
f32 CVehicleRat::m_fAxleLength;

FExplosion_GroupHandle_t CVehicleRat::m_hExplosionGroup = FEXPLOSION_INVALID_HANDLE;
FExplosion_GroupHandle_t CVehicleRat::m_hBigFinalExplosion = FEXPLOSION_INVALID_HANDLE;
SmokeTrailAttrib_t CVehicleRat::m_SmokeTrailAttrib;
BOOL CVehicleRat::m_bLoadingDroidRat;	// flag to tell rat gun to load droid mesh

// statics for realtime adjustment with slider system

#if FANG_PRODUCTION_BUILD
#define _ADJUST_SPHERES	0
#else
#define _ADJUST_SPHERES	0
#endif

#if _ADJUST_SPHERES
#define _ADJUST_SPHERE	0
static f32 _fAdjust;
static CFVec3A _vAdjust;

#define _NUM_GROUPS 5
static s32 _nGroupStart[_NUM_GROUPS] = { 0, 2, 3, 5, 6 };
static s32 _nGroupSize[_NUM_GROUPS]  = { 2, 1, 2, 1, 1 };
static s32 _nGroup = 0;
static BOOL _bPressed = FALSE;

static BOOL _bDrewSpheres = FALSE;
#endif


static void _RatCollisionDetect( const CFPhysicsObject::PhysicsState_t *pState );

static f32 _afBodySpherePos[CVehicleRat::MAX_COL_POINTS - CVehicleRat::NUM_WHEELS][3] = 
{
	{ -3.4f, -0.13f, 9.35f },	// left front bumper
	{  3.4f, -0.13f, 9.35f },	// right front bumper

	{  0.0f, 4.0f, -7.4f },	// turret

	{ -3.6f, -0.2f, -10.8f },	// left rear bumper
	{  3.6f, -0.2f, -10.8f },	// right rear bumper

	{ 0.0f, 0.18f, 11.22f },	// front center bumper

	{ 0.0f, -0.2f, -11.5f },	// rear center bumper
};

static f32 _afBodySphereRadius[CVehicleRat::MAX_COL_POINTS - CVehicleRat::NUM_WHEELS] = 
{
	3.2f,	// left front bumper
	3.2f,	// right front bumper

	3.4f,	// Turret

	3.1f,	// left rear bumper
	3.1f,	// right rear bumper

	3.0f,	// front center bumper

	3.1f,	// rear center bumper
};

//-----------------------------------------------------------------------------
// private functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CVehicleRat::_InitPhysicsObject( void )
{
	m_PhysObj.Init();
	m_MtxToWorld.m_vPos.y += 3.0f;
	m_PhysObj.SetPosition( &m_MtxToWorld.m_vPos );
	m_PhysObj.SetOrientation( &m_MtxToWorld );
	m_PhysObj.SetCollisionDetectionCallback( _RatCollisionDetect );
	m_PhysObj.SetElasticity( _ELASTICITY );
	m_PhysObj.SetOtherObjElasticity( _OTHER_OBJ_ELASTICITY );
	m_PhysObj.SetMass( m_BotInfo_VehiclePhysics.fMass );
	m_PhysObj.SetInertialSize( 9.0f, 8.0f, 14.4f );
	m_PhysObj.SetGravity( _GROUND_GRAVITY );
	m_PhysObj.RegisterCollisionPoints( m_aColPoints, MAX_COL_POINTS );

	m_PhysObj.SetMomentumClamping( FALSE );
	m_PhysObj.SetAngularMomentumClamping( FALSE );

	m_PhysObj.SetLinearDamping( _AIR_LINEAR_DAMPING );
	m_PhysObj.SetAngularDamping( _AIR_ANGULAR_DAMPING );
	m_PhysObj.SetDynamicDampingParams( _DYNAMIC_DAMPING_VELOCITY, _DYNAMIC_DAMPING_ANGULAR_VEL, _DYNAMIC_DAMPING_FORCE );

#if _ADJUST_SPHERES
	if( _nGroupSize[_nGroup] > 1 )
	{
		_vAdjust.x = _afBodySpherePos[_nGroupStart[_nGroup]+1][0];
		_vAdjust.y = _afBodySpherePos[_nGroupStart[_nGroup]+1][1];
		_vAdjust.z = _afBodySpherePos[_nGroupStart[_nGroup]+1][2];
		_fAdjust = _afBodySphereRadius[_nGroupStart[_nGroup]];
	}
	else
	{
		_vAdjust.x = _afBodySpherePos[_nGroupStart[_nGroup]][0];
		_vAdjust.y = _afBodySpherePos[_nGroupStart[_nGroup]][1];
		_vAdjust.z = _afBodySpherePos[_nGroupStart[_nGroup]][2];
		_fAdjust = _afBodySphereRadius[_nGroupStart[_nGroup]];
	}
#endif
}

//-----------------------------------------------------------------------------
// collide a sphere and report results to physics object.
static BOOL _CollideSphere( CVehicleRat *pRat, const CFPhysicsObject::PhysicsState_t *pState, s32 nColIndex )
{
	CFWorldUser UserTracker;
	CFVec3A vMovement, vPointMS;
	CFVec3A vForce;
	FCollImpact_t *pImpact;
	CFVec3A *pvCurrentPos;
	CFVec3A *pvProposedPos;
	f32 fTrans;
	BOOL bReturnVal = FALSE;
	u32 uIndex;
	BOOL bSkipCol = FALSE;
	u32 uFirstColIndex = 0;
	BOOL bNonWaterCollision = FALSE;

	FColl_nImpactCount = 0;

	if( pRat->m_aRatColPoints[nColIndex].uFlags & CVehicleRat::RATCOLPOINT_FLAG_ENABLE_COLLISION )
	{
		pvCurrentPos = &pRat->m_aRatColPoints[nColIndex].vCurrentPos;
		pvProposedPos = &pRat->m_aRatColPoints[nColIndex].vProposedPos;

		vMovement.Set( *pvProposedPos );
		vMovement.Sub( *pvCurrentPos );

		pRat->m_RatCollData.pMovement = &vMovement;

		pRat->m_RatColSphere.m_fRadius = pRat->m_aRatColPoints[nColIndex].fRadius;
		pRat->m_RatColSphere.m_Pos.Set( pvCurrentPos->x, pvCurrentPos->y, pvCurrentPos->z );

#if _ADJUST_SPHERES
		if( !_bDrewSpheres )
#endif
#if _DRAW_SPHERES
		{
			CFVec3A vStartColor, vEndColor;
			vStartColor.Set( 0.7f, 0.7f, 0.7f );
			vEndColor.Set( 1.0f, 1.0f, 1.0f );
			//CFCapsule capsule;
			//capsule.m_fRadius = pRat->m_aRatColPoints[nColIndex].fRadius;
			//capsule.m_vPoint1.Set( *pvCurrentPos );
			//capsule.m_vPoint2.Set( *pvProposedPos );
			//fdraw_DevCapsule( &capsule );

#if _ADJUST_SPHERES
			if( nColIndex >= CVehicleRat::NUM_WHEELS + _nGroupStart[_nGroup] && nColIndex < CVehicleRat::NUM_WHEELS + _nGroupStart[_nGroup] + _nGroupSize[_nGroup] )
			{
				vStartColor.Set( 0.7f, 0.7f, 0.0f );

			}
			else
			{
				vStartColor.Set( 0.6f, 0.6f, 0.6f );
			}
#endif
			pRat->m_PhysObj.DrawSphere( pvCurrentPos, pRat->m_aRatColPoints[nColIndex].fRadius, &vStartColor );
			//		pRat->m_PhysObj.DrawSphere( pvProposedPos, pRat->m_aRatColPoints[nColIndex].fRadius, &vEndColor );
			//	//	pRat->m_PhysObj.DrawLine( pvCurrentPos, pvProposedPos, &vEndColor );
		}
#endif
		pRat->m_aColPoints[nColIndex].pOtherObj = NULL;

		// perform collision detection
		fcoll_Clear();
PROTRACK_BEGINBLOCK( "Rat Collision" );
		fcoll_Check( &pRat->m_RatCollData, &pRat->m_RatColSphere );
PROTRACK_ENDBLOCK();
	}

	pRat->m_aRatColPoints[nColIndex].uFlags &= ~CVehicleRat::RATCOLPOINT_FLAG_TOUCHING_WATER;

	// fill in collision data for physics object
	if( FColl_nImpactCount > 0 )
	{
		fcoll_SortDynamic( TRUE );

		CFVerlet *pLastVerlet = NULL;

		if( !pRat->IsDeadOrDying() )
		{
			// step through all impacts
			for( uIndex = 0; uIndex < FColl_nImpactCount; uIndex++ )
			{
				pImpact = FColl_apSortedImpactBuf[uIndex];

				// did we hit water?
				const CGCollMaterial *pMaterial = CGColl::GetMaterial( pImpact );
				if( pMaterial && pMaterial->m_nMtlID == 10 )
				{
					// mark wheel as having touched water and record y position of water
					if( !(pRat->m_aRatColPoints[nColIndex].uFlags & CVehicleRat::RATCOLPOINT_FLAG_TOUCHING_WATER) )
					{
						pRat->m_aRatColPoints[nColIndex].uFlags |= CVehicleRat::RATCOLPOINT_FLAG_TOUCHING_WATER;
						pRat->m_fWaterY = pImpact->ImpactPoint.y;
					}

					if( !bNonWaterCollision )
					{
						// keep track of first non-water collision
						uFirstColIndex = uIndex + 1;
					}
					continue;
				}

				bNonWaterCollision = TRUE;

				// see if we hit a tracker
				CFWorldTracker *pHitTracker = (CFWorldTracker *)pImpact->pTag;
				if( pHitTracker && pHitTracker->m_nUser == MESHTYPES_ENTITY ) 
				{
					CEntity *pEntity = (CEntity *)pHitTracker->m_pUser;

					CFVerlet *pVerlet;
					pVerlet = pEntity->GetVerlet();
					if( pVerlet && pVerlet != pLastVerlet )
					{
						if( pImpact->UnitFaceNormal.Dot( CFVec3A::m_UnitAxisY ) > 0.707f )
						{
							// consider this type of collision as resting on verlet object
							vForce.Set( 0.0f, -pRat->m_PhysObj.GetMass(), 0.0f );
						}
						else
						{
							// colliding with object
							vForce.Set( *pRat->m_PhysObj.GetVelocity() );
							vForce.Mul( pRat->m_PhysObj.GetMass() );
							if( uIndex == uFirstColIndex )
							{
								bSkipCol = TRUE;
							}
						}
						pVerlet->ApplyForce( &pImpact->ImpactPoint, &vForce );
						pLastVerlet = pVerlet;
					}

					// pass damage to destruct entities, other vehicles, and bots
					if( uIndex == uFirstColIndex && pRat->m_fSpeed_WS > 0.0f && !pRat->IsDeadOrDying() )
					{
						BOOL bDamage = FALSE;
						f32 fUnitIntensity = 1.0f;

						if( pEntity->TypeBits() & ENTITY_BIT_BOOMER )
						{
							CEBoomer *pBoomer;
							pBoomer = (CEBoomer*) pEntity;

							if( !pBoomer->IsDetPackOnly() && !pBoomer->IsVehicleCollide() )
							{
								bSkipCol = TRUE;

								if( pBoomer->NormHealth() > 0.0f )
								{
									bDamage = TRUE;
									fUnitIntensity = 100.0f;	// kill boomer fast

									if( pRat->m_fBoomerDamageTimer <= 0.0f )
									{
										pRat->m_fBoomerDamageTimer = _BOOMER_DAMAGE_DELAY;

										pRat->m_fMomentumReduction = pBoomer->GetVehicleSlowdown();
									}
								}
							}
						}
						else if( pEntity->TypeBits() & ENTITY_BIT_VEHICLE )
						{
							CVehicle *pVehicle = (CVehicle*) pEntity;

							if( pRat->m_fInterVehicleDamageTimer <= 0.0f && !pVehicle->IsDeadOrDying() )
							{
								if( pVehicle->GetDriverBot() )
								{
									if( level_IsRacingLevel() && !pVehicle->GetDriverBot()->IsPlayerBot() )
									{
										if( !pVehicle->GetGunnerBot() || !pVehicle->GetGunnerBot()->IsPlayerBot() )
										{
											bDamage = TRUE;
										}
									}
								}
								else if( pVehicle->GetGunnerBot() )
								{
									if( level_IsRacingLevel() && !pVehicle->GetGunnerBot()->IsPlayerBot() )
									{
										bDamage = TRUE;
									}
								}
							}

							if( bDamage )
							{
								pRat->m_fInterVehicleDamageTimer = _VEHICLE_DAMAGE_DELAY;

								// compute total impact velocity
								CFVec3A vPoint, vImpactVel, vOtherVel;
								vPoint.Set( pImpact->ImpactPoint );
								vPoint.Sub( pState->vPosition );
								pRat->m_PhysObj.GetVelocity( &vPoint, &vImpactVel );

								vPoint.Set( pImpact->ImpactPoint );
								vPoint.Sub( *pVehicle->m_PhysObj.GetPosition() );
								pVehicle->m_PhysObj.GetVelocity( &vPoint, &vOtherVel );

								vImpactVel.Sub( vOtherVel );

								// compute a unit impact intensity
								fUnitIntensity = vImpactVel.Mag() * pRat->GetOOMaxSpeed(); // m_BotInfo_VehiclePhysics.fMaxSpeed;
								FMATH_CLAMP( fUnitIntensity, 0.0f, 1.0f );
								fUnitIntensity = fmath_Sqrt( fUnitIntensity );
							}
						}
						else if( (pEntity->TypeBits() & ENTITY_BIT_BOT) && pEntity->GetParent() != pRat && pEntity->GetParent() != pRat->RatGun() )
						{
							CBot *pBot;
							pBot = (CBot*) pEntity;

							if( pBot->m_fCollCylinderHeight_WS <= 8.0f )
							{
								// let RAT run through "short" bots.  They will be continually damaged and eventually blow up.
								bSkipCol = TRUE;
							}

							if( pRat->m_fInterVehicleDamageTimer <= 0.0f && !pBot->IsDeadOrDying() && !pRat->IsDeadOrDying() )
							{
								// compute total impact velocity
								CFVec3A vPoint, vImpactVel;
								vPoint.Set( pImpact->ImpactPoint );
								vPoint.Sub( pState->vPosition );
								pRat->m_PhysObj.GetVelocity( &vPoint, &vImpactVel );

								if( vImpactVel.Mag() > _MIN_RAMMING_VELOCITY )
								{
									bDamage = TRUE;
									pRat->m_fInterVehicleDamageTimer = _BOT_DAMAGE_DELAY;

									vPoint.Set( pImpact->ImpactPoint );
									vPoint.Sub( pBot->MtxToWorld()->m_vPos );

									vImpactVel.Sub( pBot->m_Velocity_WS );

									// compute a unit impact intensity
									fUnitIntensity = vImpactVel.Mag() * pRat->GetOOMaxSpeed();
									FMATH_CLAMP( fUnitIntensity, 0.0f, 1.0f );
									fUnitIntensity = fmath_Sqrt( fUnitIntensity );
								}
							}
						}

						if( bDamage )
						{
							CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
							if( pDamageForm )
							{
								pDamageForm->m_fNormIntensity = fUnitIntensity;
								pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
								pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
								pDamageForm->m_pDamageProfile = pRat->GetDamageProfile();
								pDamageForm->m_Damager.pEntity = (CEntity*) pRat;
								pDamageForm->m_Damager.pWeapon = NULL;
								pDamageForm->m_Damager.pBot = pRat;
								pDamageForm->m_Damager.nDamagerPlayerIndex = pRat ? pRat->m_nPossessionPlayerIndex : -1;
								pDamageForm->m_pDamageeEntity = pEntity;
								pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *) pImpact->pTag, pImpact, &pRat->m_UnitVelocity_WS );
								CDamage::SubmitDamageForm( pDamageForm );
							}
						}
					}

					// pass collision info to other vehicles
					if( pEntity->TypeBits() & ENTITY_BIT_VEHICLE )
					{
						CVehicle *pVehicle = (CVehicle *) pEntity;

						if( uIndex == uFirstColIndex )
						{
							if( !pVehicle->IsDeadOrDying() && !pRat->IsDeadOrDying() )
							{
								// record pointer to other vehicle's physics object
								pRat->m_aColPoints[nColIndex].pOtherObj = &pVehicle->m_PhysObj;
							}
							else
							{
								// don't send this collision to physics system
								bSkipCol = TRUE;
							}
						}
					}

					// if this is an entity we want to skip, skip
					if( (uIndex == uFirstColIndex) && (pEntity->TypeBits() & _ENTITY_BITS_TO_SKIP_COLLISION) ) 
					{
						bSkipCol = TRUE;
					}

					if( (uIndex == uFirstColIndex) && (pEntity->TypeBits() & ENTITY_BIT_BOT) && !(pEntity->TypeBits() & ENTITY_BIT_VEHICLE) ) 
					{
						bSkipCol = TRUE;
					}

					if( uIndex == uFirstColIndex )
					{
						CEntity *pParent;

						pParent = pEntity->GetParent();

						while( pParent )
						{
							if( pParent == pRat || pParent == pRat->RatGun() )
							{
								// if entity or any of its ancestors is standing on the rat, don't collide
								bSkipCol = TRUE;
								break;
							}

							pParent = pParent->GetParent();
						}
					}
				}
			}
		}

		if ( !bSkipCol && uFirstColIndex < FColl_nImpactCount )
		{
			CFVec3A vSphereColUnitDir, vSphereCenter;

			pImpact = FColl_apSortedImpactBuf[uFirstColIndex];

			// find unit direction from sphere center to impact point on sphere.

			// first find position of collided sphere center along the projection
			if( pImpact->fUnitImpactTime >= 0.0f )
			{
				vSphereCenter.Set( vMovement );
				vSphereCenter.Mul( pImpact->fUnitImpactTime );
				vSphereCenter.Add( *pvCurrentPos );
			}
			else
			{
				// sphere started out in collision, so just use end position
				vSphereCenter.Set( *pvCurrentPos );
			}

			// now find unit vector from collided sphere center to collision point
			vSphereColUnitDir.Set( pImpact->ImpactPoint );
			vSphereColUnitDir.Sub( vSphereCenter );
			vSphereColUnitDir.Unitize();
#if 0
CFVec3A vStart, vEnd, vTemp;
fdraw_DevSphere( &vSphereCenter.v3, 0.5f, &FColor_MotifBlue );
fdraw_DevSphere( &pImpact->ImpactPoint.v3, 0.5f, &FColor_MotifGreen );
vTemp.Set( pState->mOrientation.m_vRight );
vTemp.Mul( 3.0f );
if( !(nColIndex & 1) ) vTemp.Negate();
vStart.Set( vTemp );
vStart.Add( vSphereCenter );
vEnd.Set( vSphereColUnitDir );
vEnd.Mul( 3.0f );
vEnd.Add( vStart );
fdraw_DevLine( (const CFVec3*)&vStart, (const CFVec3*) &vEnd );
#endif
			pRat->m_aRatColPoints[nColIndex].pMaterial = CGColl::GetMaterial( pImpact );

			pRat->m_aColPoints[nColIndex].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_COLLIDED;
			pRat->m_aColPoints[nColIndex].fTimeSinceCollision = 0.0f;

//			if( pRat->SmoothContact() && nColIndex < CVehicleRat::NUM_WHEELS && 
//				vSphereColUnitDir.Dot( pImpact->UnitFaceNormal ) > -0.9f )
////				vSphereColUnitDir.Dot( pState->mOrientation.m_vUp ) > -0.9f )
//			{
//				// if sphere collides with tri such that the tri normal is significantly different than the
//				// direction from the sphere to the collision point on the sphere, then use the negated sphere direction
//				// as the collision normal instead of the tri normal.  The goal is to try to allow the tires to
//				// ride up over vertical curbs.
//				pRat->m_aColPoints[nColIndex].vNormal = vSphereColUnitDir;
//				pRat->m_aColPoints[nColIndex].vNormal.Negate();
//			}
//			else
			{
				pRat->m_aColPoints[nColIndex].vNormal = pImpact->UnitFaceNormal;
			}

			pRat->m_aColPoints[nColIndex].vPoint = pImpact->ImpactPoint;
			pRat->m_aColPoints[nColIndex].vPoint.Sub( pState->vPosition );

//fdraw_DevSphere((const CFVec3*)&pImpact->ImpactPoint, 1.0f );

			pRat->m_aColPoints[nColIndex].fDepth = pImpact->fImpactDistInfo;
			pRat->m_aColPoints[nColIndex].vPush = pImpact->PushUnitVec;

			// allow friction in vertical direction when collision point on sphere is in direction of
			// the x vector of the friction matrix. (i.e. vertical friction only when that part of sphere
			// is touching the ground.)
			pRat->m_aColPoints[nColIndex].vStaticFriction.y = pRat->m_BotInfo_VehiclePhysics.fBodyFriction * FMATH_FABS( vSphereColUnitDir.Dot( pRat->m_aRatColPoints[nColIndex].mFriction.m_vRight ) );
		}

		bReturnVal = TRUE;
	}
	else
	{
		pRat->m_aRatColPoints[nColIndex].pMaterial = NULL;

		pRat->m_aColPoints[nColIndex].uClientFlags &= ~CFPhysicsObject::CLIENT_FLAG_COLLIDED;
		pRat->m_aColPoints[nColIndex].fTimeSinceCollision += FLoop_fPreviousLoopSecs;
	}

	fTrans = _HIGH_TRACTION_KT;
	if( pRat->m_aRatColPoints[nColIndex].pMaterial )
	{
		if( pRat->m_aRatColPoints[nColIndex].pMaterial->HasLooseDust() )
		{
			fTrans = _LOW_TRACTION_KT;
		}
	}
	if( nColIndex < 2 )
	{
		fTrans += _KINETIC_TRANS_BALANCE * fTrans;
	}
	else if( nColIndex > 3 )
	{
		fTrans -= _KINETIC_TRANS_BALANCE * fTrans;
	}
	pRat->m_aColPoints[nColIndex].fKineticTransition = fTrans;


	if( nColIndex < CVehicleRat::NUM_WHEELS )
	{
		pRat->m_aColPoints[nColIndex].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_SPRING_ACTIVE;
	}

	pRat->m_aColPoints[nColIndex].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_APPLY_FRICTION;

	return bReturnVal;
}

//-----------------------------------------------------------------------------
// this function is called by the vehicle's physics object.  It collides the vehicle with
// the world terrain and reports the resulting collisions to the physics object via the array m_aColPoints[].
static void _RatCollisionDetect( const CFPhysicsObject::PhysicsState_t *pState )
{
	CBot *pBot = CVehicleRat::m_pCurrentCollider;
	CVehicleRat *pRat = ( CVehicleRat* ) CVehicleRat::m_pCurrentCollider;
	const CFVec3A *pvPos;
	const CFMtx43A *pOr = &pState->mOrientation;
	CFVec3A vOffset;
	CFVec3A vWheelSink;
	s32 nIndex;

	if( pBot == NULL )
	{
		return;
	}

	pRat->m_fInterVehicleDamageTimer -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( pRat->m_fInterVehicleDamageTimer, 0.0f );

	pRat->m_fBoomerDamageTimer -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( pRat->m_fBoomerDamageTimer, 0.0f );

	// set up the collision info struct for all collsions below
	pRat->m_RatCollData.nFlags = FCOLL_DATA_IGNORE_BACKSIDE;
	pRat->m_RatCollData.nCollMask = FCOLL_MASK_COLLIDE_WITH_VEHICLES;
	pRat->m_RatCollData.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	pRat->m_RatCollData.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	pRat->m_RatCollData.pCallback = NULL;//_IntersectingTrackerCallback;
	pRat->m_RatCollData.pLocationHint = pRat->m_pWorldMesh;

	vWheelSink.Set( pRat->MtxToWorld()->m_vUp );
	vWheelSink.Mul( _WHEEL_SINK );	// make tires look flattened a little, plus offset the large wheel spheres so bottom matches up with bottom of tires

	const CFVec3A *pvWheel;
	// set the Rat collision point info for the wheels
	for( nIndex = 0; nIndex < CVehicleRat::NUM_WHEELS; nIndex++ )
	{
		pvWheel = pRat->GetTagPoint( CVehicleRat::TAG_POINT_INDEX_L_WHEEL_FRONT + nIndex );
		FASSERT( pvWheel );
		pRat->m_aRatColPoints[nIndex].vCurrentPos.Set( *pvWheel );
		pRat->m_aRatColPoints[nIndex].vCurrentPos.Add( vWheelSink );
		// (wheel friction matrix updated in animate bone callback)
	}

	// get a pointer to the world space position of the chassis bone of the vehicle.
	pvPos = pRat->GetTagPoint( CVehicleRat::TAG_POINT_INDEX_CHASSIS );
	FASSERT( pvPos );

	// set Rat collision point info for non-wheel collision spheres.
	for( nIndex = 0; nIndex < CVehicleRat::MAX_COL_POINTS - CVehicleRat::NUM_WHEELS; nIndex++ )
	{
		vOffset.Set( _afBodySpherePos[nIndex][0], _afBodySpherePos[nIndex][1], _afBodySpherePos[nIndex][2] );
		pOr->MulPoint( vOffset );
		vOffset.Add( *pvPos );
		pRat->m_aRatColPoints[CVehicleRat::NUM_WHEELS + nIndex].vCurrentPos.Set( vOffset );
		pRat->m_aRatColPoints[CVehicleRat::NUM_WHEELS + nIndex].mFriction.Set( *pRat->MtxToWorld() );
		pRat->m_aRatColPoints[CVehicleRat::NUM_WHEELS + nIndex].mFriction.m_vPos.Zero();
	}

	// do a manual UpdateMatrices() with pState position and orientation
	{
		CFMtx43A mRot = pState->mOrientation;
		CFVec3A vCMOffset;

		// copy physics object's position and orientation to vehicle's world mesh.
		mRot.MulPoint( vCMOffset, pRat->m_vCMOffset );
		mRot.m_vPos = pState->vPosition;
		mRot.m_vPos.Sub( vCMOffset );
		pRat->m_pWorldMesh->m_Xfm.BuildFromMtx( mRot );
		pRat->m_pWorldMesh->UpdateTracker();

		// Update bone matrix palette
		pRat->ComputeMtxPalette( FALSE );
	}

	// set proposed Rat collision point info for wheel collision spheres.
	for( nIndex = 0; nIndex < CVehicleRat::NUM_WHEELS; nIndex++ )
	{
		pRat->m_aRatColPoints[nIndex].vProposedPos.Set( *(pRat->GetTagPoint( CVehicleRat::TAG_POINT_INDEX_L_WHEEL_FRONT + nIndex )) );
		pRat->m_aRatColPoints[nIndex].vProposedPos.Add( vWheelSink );
	}
	// get a pointer to the world space position of the chassis bone of the vehicle.
	pvPos = pRat->GetTagPoint( CVehicleRat::TAG_POINT_INDEX_CHASSIS );
	FASSERT( pvPos );
	// set Rat collision point info for non-wheel collision spheres.
	for( nIndex = 0; nIndex < CVehicleRat::MAX_COL_POINTS - CVehicleRat::NUM_WHEELS; nIndex++ )
	{
		vOffset.Set( _afBodySpherePos[nIndex][0], _afBodySpherePos[nIndex][1], _afBodySpherePos[nIndex][2] );
		pOr->MulPoint( vOffset );
		vOffset.Add( *pvPos );
		pRat->m_aRatColPoints[CVehicleRat::NUM_WHEELS + nIndex].vProposedPos.Set( vOffset );
	}

	// collide all points
	for( nIndex = 0; nIndex < CVehicleRat::MAX_COL_POINTS; nIndex++ )
	{
		FWorld_nTrackerSkipListCount = 0;
		pRat->AppendTrackerSkipList();
		pRat->m_RatCollData.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
		pRat->m_RatCollData.ppTrackerSkipList = FWorld_apTrackerSkipList;
		_CollideSphere( pRat, pState, nIndex );
	}

#if _ADJUST_SPHERES
	_bDrewSpheres = TRUE;
#endif
}

//-----------------------------------------------------------------------------
// set spring values to defaults
void CVehicleRat::_ResetSprings( void )
{
	s32 nIndex;

	for( nIndex = 0; nIndex < NUM_WHEELS; nIndex++ )
	{
		m_aColPoints[nIndex].Spring.fCurrentLength = _SPRING_STRETCHED_LENGTH;
		m_aColPoints[nIndex].Spring.fRestLength = _SPRING_REST_LENGTH;
		m_aColPoints[nIndex].Spring.fCompressedLength = _SPRING_COMPRESSED_LENGTH;
		m_aColPoints[nIndex].Spring.fStretchedLength = _SPRING_STRETCHED_LENGTH;
		m_aColPoints[nIndex].Spring.fStiffness = _SPRING_COEFFICIENT;
		if( nIndex < 2 )
		{
			m_aColPoints[nIndex].Spring.fStiffness += _SPRING_BALANCE * _SPRING_COEFFICIENT;
		}
		else if( nIndex > 3 )
		{
			m_aColPoints[nIndex].Spring.fStiffness -= _SPRING_BALANCE * _SPRING_COEFFICIENT;
		}

		m_aColPoints[nIndex].Spring.fDamping = _SPRING_COEFFICIENT * _SPRING_DAMPING_RATIO;
		m_aColPoints[nIndex].Spring.fDampingRatio = _SPRING_COMPRESSION_RATIO;
		m_aColPoints[nIndex].Spring.fVelocity = 0.0f;
		m_aColPoints[nIndex].Spring.fForce = 0.0f;
		m_aColPoints[nIndex].Spring.fAppliedForceRatio = _SPRING_APPLIED_FORCE_RATIO;
	}

	for( nIndex = NUM_WHEELS; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		m_aColPoints[nIndex].Spring.fStiffness = 0.0f;
	}
}


//-----------------------------------------------------------------------------
void CVehicleRat::_ClearDataMembers( void )
{
	s32 nIndex;

	m_pMoveIdentifier = &m_apTagPoint_WS[0];

	m_pBotInfo_Gen = &m_BotInfo_Gen;
	m_pBotInfo_Vehicle = &m_BotInfo_Vehicle;
	m_pBotInfo_VehiclePhysics = &m_BotInfo_VehiclePhysics;
	m_pBotInfo_Engine = &m_BotInfo_Engine;
	if( MultiplayerMgr.IsMultiplayer() )
	{
		m_pBotInfo_DriverCamera = &m_BotInfo_MPDriverCamera;
	}
	else
	{
		m_pBotInfo_DriverCamera = &m_BotInfo_DriverCamera;
	}

	m_eRatState = VEHICLERAT_STATE_DRIVER_UNOCCUPIED;
	m_eGunState = VEHICLERAT_GUN_STATE_UNOCCUPIED;
	m_pGunnerBot = NULL;
	m_fSteeringPosition = 0.0f;
	m_fRawSteeringPosition = 0.0f;
	m_qGroundNormal.Identity();
	m_fFlipOverTime = 0.0f;
	m_fCollCylinderRadius_WS = 10.0f;   //adjust these if the size of the model changes
	m_fCollCylinderHeight_WS = 16.0f;
	for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
	{
		m_hCloudParticle[nIndex] = FPARTICLE_INVALID_HANDLE;
	}
	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		m_hDirtParticle[nIndex] = FPARTICLE_INVALID_HANDLE;
	}
	m_fGunStateTimer = 0.0f;
	m_uRatFlags = 0;
	m_pEParticleSmoke = NULL;
	m_fRightHatchUnitPos = 0.0f;
	m_uRatType = 0;
	m_pZobbyME = NULL;
#if ENABLE_BOOST_FEATURE
	m_fBoostTimer = 0.0f;
	m_fUnitBoost = 1.0f;
#endif
	m_DriverCameraTrans.SetTransitionLength( _CAMERA_TRANSITION_TIME );
	m_GunnerCameraTrans.SetTransitionLength( _CAMERA_TRANSITION_TIME );
	m_fInterVehicleDamageTimer = 0.0f;
	m_fBoomerDamageTimer = 0.0f;
	m_fMomentumReduction = 0.0f;
	m_pPreviousGunnerArmorProfile = NULL;
	m_fWaterY = 0.0f;
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateAISound( void )
{
	// send a sound radius to AI so brains can "hear" vehicle coming and try to dodge it
	if( m_pDriverBot && m_pDriverBot->IsPlayerBot() )
	{
		f32 fSpeed = m_PhysObj.GetVelocity()->Mag();
		f32 fFakeMaxSpeed = 75.0f;
		FMATH_CLAMPMAX( fSpeed, fFakeMaxSpeed );	//some arbitrary max speed just used to approximate get a rough normalized speed
		AIEnviro_BoostPlayerSoundTo( m_pDriverBot->m_nPossessionPlayerIndex, 25.0f + AIEnviro_fVehicleBotSoundRadius * fmath_Div( fSpeed, fFakeMaxSpeed ) );
	}
}


//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateTireTextures( void )
{
	// compute rotational velocity of tire in radians per second
	f32 fRollingVel = m_PhysObj.GetVelocity()->Dot( m_PhysObj.GetOrientation()->m_vFront );
	fRollingVel *= _OO_WHEEL_RADIUS;
	if( (m_aColPoints[COLPOINT_L_REAR_WHEEL_INDEX].uClientFlags & CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION) &&
		(m_aColPoints[COLPOINT_R_REAR_WHEEL_INDEX].uClientFlags & CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION) )
	{
		if( m_fBrake > 0.1f && m_fThrottle < 0.1f )
		{
			// when sliding and brakes are applied, tires don't spin
			fRollingVel = 0.0f;
		}
		else
		{
			// when sliding, tires spin at rate determined by throttle
			fRollingVel = m_fMaxSpeed * m_fThrottle;
			fRollingVel *= _OO_WHEEL_RADIUS;
		}	
	}

	// mix in the motion-blurred tire texture based on wheel rotation speed
	if( m_hTireLayer != FMESH_TEXLAYERHANDLE_INVALID )
	{
		f32 fBlend;
		f32 fBlurVel;
		fBlurVel = _MAX_WHEEL_BLUR_VEL_MUL * m_BotInfo_VehiclePhysics.fMaxSpeed;
		// blend value of 0.0 = full blur, 1.0 = no blur
		fBlend = fmath_Div( fBlurVel - fmath_Abs( fRollingVel ), fBlurVel );
		FMATH_CLAMPMIN( fBlend, _MIN_TIRE_BLUR_BLEND );
		FMATH_CLAMPMAX( fBlend, 1.0f );
		m_pWorldMesh->LayerAlpha_Set( m_hTireLayer, fBlend );
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateCollisionFlags( void )
{
	s32 nIndex;

	m_fTimeSinceLastHitGround += FLoop_fPreviousLoopSecs;
	m_fTimeSinceLastCollide += FLoop_fPreviousLoopSecs;

	// update collision & contacting flags & times
	m_uVehicleFlags &= ~(VEHICLE_FLAG_HIT_GROUND | VEHICLE_FLAG_BECAME_AIRBORNE);
	if( m_PhysObj.IsColliding() || m_PhysObj.IsContacting() )
	{
		m_fTimeSinceLastContact = 0.0f;
		if( !SmoothContact() )
		{
			m_uVehicleFlags |= VEHICLE_FLAG_HIT_GROUND;
			m_fTimeSinceLastHitGround = 0.0f;
			m_fTimeSinceLastCollide = 0.0f;
		}
		m_uVehicleFlags |= VEHICLE_FLAG_SMOOTH_CONTACT;
	}
	else
	{
		m_fTimeSinceLastContact += FLoop_fPreviousLoopSecs;

		if( m_fTimeSinceLastContact > _SMOOTHED_CONTACT_TIME )
		{
			if( SmoothContact() )
			{
				m_uVehicleFlags |= VEHICLE_FLAG_BECAME_AIRBORNE;
			}

			m_uVehicleFlags &= ~VEHICLE_FLAG_SMOOTH_CONTACT;
		}
	}

	// set collided/jostled flags and record collision magnitude
	m_uVehicleFlags &= ~(VEHICLE_FLAG_COLLIDED | VEHICLE_FLAG_JOSTLED);
	m_fRecentCollisionMagnitude = 0.0f;
	m_pRecentCollisionMaterial = NULL;
	for( nIndex = 0; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		if( m_aColPoints[nIndex].uResultFlags & CFPhysicsObject::COLLISION_RESULT_COLLIDE )
		{
			f32 fMag = m_aColPoints[nIndex].Result.vMomentum.Mag();
			if( fMag > m_fRecentCollisionMagnitude )
			{
				m_fRecentCollisionMagnitude = fMag;
				m_pRecentCollisionMaterial = m_aRatColPoints[nIndex].pMaterial;
			}
		}
	}

	if( m_fTimeSinceLastCollide > _MIN_TIME_BETWEEN_COLLIDE_FLAG_SET || HitGround() )
	{
		if( HitGround() )
		{
			if( m_fRecentCollisionMagnitude > m_pBotInfo_Vehicle->fHardHitGroundMom )
			{
				m_fTimeSinceLastCollide = 0.0f;
				m_uVehicleFlags |= VEHICLE_FLAG_COLLIDED;
			}
			else if( m_fRecentCollisionMagnitude > m_pBotInfo_Vehicle->fSoftHitGroundMom )
			{
				m_fTimeSinceLastCollide = 0.0f;
				m_uVehicleFlags |= VEHICLE_FLAG_JOSTLED;
			}
		}
		else
		{
			if( m_fRecentCollisionMagnitude > m_pBotInfo_Vehicle->fHardCollMom )
			{
				m_fTimeSinceLastCollide = 0.0f;
				m_uVehicleFlags |= VEHICLE_FLAG_COLLIDED;
			}
			else if( m_fRecentCollisionMagnitude > m_pBotInfo_Vehicle->fSoftCollMom )
			{
				m_fTimeSinceLastCollide = 0.0f;
				m_uVehicleFlags |= VEHICLE_FLAG_JOSTLED;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateDustCloud( void )
{
	s32 nIndex;

	for( nIndex = 0; nIndex < NUM_WHEELS; nIndex++ )
	{
		m_fDirtParticleIntensity[nIndex] = 0.0f;
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[nIndex], FALSE );
		}
	}

	FASSERT( MAX_CLOUD_PARTICLES == 2 );
	if( m_hCloudParticle[0] != FPARTICLE_INVALID_HANDLE && m_hCloudParticle[1] != FPARTICLE_INVALID_HANDLE )
	{
		if( SmoothContact() && m_fSpeed_WS > _MIN_CLOUD_SPEED &&
			((m_aRatColPoints[COLPOINT_L_FRONT_WHEEL_INDEX].pMaterial && 
			m_aRatColPoints[COLPOINT_L_FRONT_WHEEL_INDEX].pMaterial->HasLooseDust()) ||
			(m_aRatColPoints[COLPOINT_R_FRONT_WHEEL_INDEX].pMaterial && 
			m_aRatColPoints[COLPOINT_R_FRONT_WHEEL_INDEX].pMaterial->HasLooseDust())) )
		{
			for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
			{
				fparticle_EnableEmission( m_hCloudParticle[nIndex], TRUE );
				m_fCloudParticleIntensity[nIndex] = m_fClampedNormSpeedXZ_WS;
				if( !IsPlayerBot() )
				{
					// ai Rats kick up less dust so player can see them when racing
					m_fCloudParticleIntensity[nIndex] *= 0.4f;
				}

				m_vCloudParticleDirection[nIndex].Set( m_PhysObj.GetOrientation()->m_vFront );
				m_vCloudParticleVelocity[nIndex].Set( *m_PhysObj.GetVelocity() );
			}

			m_vCloudParticlePosition[0].Set( m_PhysObj.GetOrientation()->m_vRight );
			m_vCloudParticlePosition[0].Mul( _CLOUD_INWARD_OFFSET );
			m_vCloudParticlePosition[1].Set( m_PhysObj.GetOrientation()->m_vRight );
			m_vCloudParticlePosition[1].Mul( -_CLOUD_INWARD_OFFSET );

			if( m_MtxToWorld.m_vFront.Dot( *m_PhysObj.GetVelocity() ) >= 0.0f )
			{
				m_vCloudParticlePosition[0].Add( *GetTagPoint( CVehicleRat::TAG_POINT_INDEX_L_WHEEL_REAR ) );
				m_vCloudParticlePosition[1].Add( *GetTagPoint( CVehicleRat::TAG_POINT_INDEX_R_WHEEL_REAR ) );
				m_vCloudParticleDirection[0].Negate();
				m_vCloudParticleDirection[1].Negate();
			}
			else
			{
				m_vCloudParticlePosition[0].Add( *GetTagPoint( CVehicleRat::TAG_POINT_INDEX_L_WHEEL_FRONT ) );
				m_vCloudParticlePosition[1].Add( *GetTagPoint( CVehicleRat::TAG_POINT_INDEX_R_WHEEL_FRONT ) );
			}
		}
		else
		{
			for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
			{
				m_fCloudParticleIntensity[nIndex] = 0.0f;
				fparticle_EnableEmission( m_hCloudParticle[nIndex], FALSE );
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateWaterEffects( void )
{
	s32 nIndex;

	// spawn water particles
	if( m_hWaterParticleDef != FPARTICLE_INVALID_HANDLE && m_fNormSpeedXZ_WS > 0.0f )
	{
		f32 fSpawnChance = fmath_Sqrt( m_fNormSpeedXZ_WS ) * 0.7f;
		f32 fIntensity = m_fNormSpeedXZ_WS * m_fNormSpeedXZ_WS;
		FMATH_CLAMP( fSpawnChance, 0.0f, 1.0f );
		FMATH_CLAMP( fIntensity, 0.0f, 1.0f );
		for( nIndex = 0; nIndex < MAX_COL_POINTS; nIndex++ )
		{
			if( nIndex == COLPOINT_TURRET_INDEX )
			{
				if( m_aRatColPoints[nIndex].uFlags & CVehicleRat::RATCOLPOINT_FLAG_TOUCHING_WATER )
				{
					// make up to three splashes for turret

					if( fmath_RandomChance( fSpawnChance ) )
					{
						fparticle_SpawnEmitter( m_hWaterParticleDef, &m_aRatColPoints[nIndex].vCurrentPos.v3, &CFVec3A::m_UnitAxisY.v3, &m_PhysObj.GetVelocity()->v3, fIntensity );
					}

					if( fmath_RandomChance( fSpawnChance ) )
					{
						m_vLeftTurretWaterPos.Set( m_MtxToWorld.m_vRight );
						m_vLeftTurretWaterPos.Mul( -3.0f );
						m_vLeftTurretWaterPos.Add( m_aRatColPoints[nIndex].vCurrentPos );
						fparticle_SpawnEmitter( m_hWaterParticleDef, &m_vLeftTurretWaterPos.v3, &CFVec3A::m_UnitAxisY.v3, &m_PhysObj.GetVelocity()->v3, fIntensity );
					}

					if( fmath_RandomChance( fSpawnChance ) )
					{
						m_vRightTurretWaterPos.Set( m_MtxToWorld.m_vRight );
						m_vRightTurretWaterPos.Mul( 3.0f );
						m_vRightTurretWaterPos.Add( m_aRatColPoints[nIndex].vCurrentPos );
						fparticle_SpawnEmitter( m_hWaterParticleDef, &m_vRightTurretWaterPos.v3, &CFVec3A::m_UnitAxisY.v3, &m_PhysObj.GetVelocity()->v3, fIntensity );
					}
				}
			}
			else
			{
				if( ((m_aRatColPoints[nIndex].uFlags & CVehicleRat::RATCOLPOINT_FLAG_TOUCHING_WATER) || (m_aRatColPoints[COLPOINT_TURRET_INDEX].uFlags & CVehicleRat::RATCOLPOINT_FLAG_TOUCHING_WATER)) &&
					fmath_RandomChance( fSpawnChance ) )
				{
					fparticle_SpawnEmitter( m_hWaterParticleDef, &m_aRatColPoints[nIndex].vCurrentPos.v3, &CFVec3A::m_UnitAxisY.v3, &m_PhysObj.GetVelocity()->v3, fIntensity );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateRatState( void )
{
	switch( m_eRatState )
	{
		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT:
			m_pDriverBot->AbortScopeMode( TRUE );
			StartTransitionToDriverVehicleCamera( m_pDriverBot->m_nPossessionPlayerIndex );

			{
				CFVec3A vJumpVel;
				m_fEntryJumpTimer = ComputeEntryTrajectory( m_pDriverBot->MtxToWorld()->m_vPos,					// start pos
										m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach[m_uRatType]]->m_vPos,	// end pos
										3.0f,																	// minimum jump height, 
										m_pDriverBot->m_pBotInfo_Gen->fGravity * m_pDriverBot->m_fGravityMultiplier,	// gravity
										vJumpVel );																// output velocity

				m_pDriverBot->StartVelocityJump( &vJumpVel );
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
				m_pDriverBot->SetNoCollideStateAir( TRUE );
				m_eRatState = VEHICLERAT_STATE_MOVE_DRIVER_TO_ENTRY_POINT;
				SetHatchOpen( TRUE /*bOpen*/, FALSE /*bImmediately*/ );
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_MOVE_DRIVER_TO_ENTRY_POINT:
			m_fEntryJumpTimer -= FLoop_fPreviousLoopSecs;

			if( IsDriverUseless( m_pDriverBot ) )
			{
				// abort entry if driver becomes unable to drive
				_AbortDriverEntry();
			}
			else if( m_fEntryJumpTimer < 0.0f )
			{
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
				m_pDriverBot->SetNoCollideStateAir( FALSE );
				m_eRatState = VEHICLERAT_STATE_START_DRIVER_ENTER;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_START_DRIVER_ENTER:
			_StartDriverEnterWork( FALSE /*bImmediately*/ );
			CrankEngine();
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_ENGINE_STARTING:
			LocateDriverAtBone( m_nBoneIdxDriverAttach[m_uRatType] );

			if( EngineState() == ENGINE_STATE_RUNNING )
			{
//				SetSpotLightOn();
				m_eRatState = VEHICLERAT_STATE_DRIVER_ENTERING;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_DRIVER_ENTERING:
			LocateDriverAtBone( m_nBoneIdxDriverAttach[m_uRatType] );

			// Wait for weapon switch to complete before driving.
			m_eRatState = VEHICLERAT_STATE_DRIVING;
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_DRIVING:
			LocateDriverAtBone( m_nBoneIdxDriverAttach[m_uRatType] );
			if( m_pDriverBot && IsDriverUseless(m_pDriverBot) )
			{
				m_eRatState = VEHICLERAT_STATE_START_DRIVER_EXIT;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_START_DRIVER_EXIT:
			{
				CFMtx43A mSeat;
				CFVec3A vTemp, vJumpVel, vPostExitPoint;

				if( IsPlayerBot() )
				{
					StartTransitionToDriverBotCamera();
				}
				StopEngine();
//				SetSpotLightOff();


				// set up exit jump
				if( m_MtxToWorld.m_vUp.Dot( CFVec3A::m_UnitAxisY ) < 0.2f )
				{
					// RAT is steeply tilted or upside down, jump straight up to get clear of vehicle
					vPostExitPoint.Set( CFVec3A::m_UnitAxisY );
					vPostExitPoint.Mul( 10.0f );
					vPostExitPoint.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach[m_uRatType]]->m_vPos );
				}
				else
				{
					vPostExitPoint.Set( m_MtxToWorld.m_vRight );
					vPostExitPoint.Mul( -4.0f );
					vTemp.Set( m_MtxToWorld.m_vUp );
					vTemp.Mul( 4.0f );
					vPostExitPoint.Add( vTemp );
					vPostExitPoint.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach[m_uRatType]]->m_vPos );
				}

 				m_fEntryJumpTimer = ComputeEntryTrajectory( m_pDriverBot->MtxToWorld()->m_vPos,	// start pos
															vPostExitPoint,						// end pos
															3.0f,								// minimum jump height, 
															m_pDriverBot->m_pBotInfo_Gen->fGravity * m_pDriverBot->m_fGravityMultiplier, // gravity
															vJumpVel );							// output velocity

				m_pDriverBot->StartVelocityJump( &vJumpVel );
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
				m_pDriverBot->SetNoCollideStateAir( TRUE );
				m_pDriverBot->DetachFromParent();
			}

			SetHatchOpen( FALSE /*bOpen*/, FALSE /*bImmediately*/ );

			m_eRatState = VEHICLERAT_STATE_DRIVER_EXITING;
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_DRIVER_EXITING:
			m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
			m_pDriverBot->SetNoCollideStateAir( TRUE );
			m_fEntryJumpTimer -= FLoop_fPreviousLoopSecs;
			if( m_fEntryJumpTimer < 0.0f )
			{
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
				m_pDriverBot->SetNoCollideStateAir( FALSE );
				m_eRatState = VEHICLERAT_STATE_DRIVER_EXIT_CAMERA;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_DRIVER_EXIT_CAMERA:
			if( m_DriverCameraTrans.IsCameraInactive() )
			{
				// wait for camera to finish before exiting
				DriverExit( m_pDriverBot );
				m_eRatState = VEHICLERAT_STATE_ENGINE_STOPPING;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_ENGINE_STOPPING:
			if( EngineState() == ENGINE_STATE_STOPPED )
			{
				m_eRatState = VEHICLERAT_STATE_DRIVER_UNOCCUPIED;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_START_UPSIDE_DOWN:
			// move driver bot to a safer exit point above vehicle
			m_eRatState = VEHICLERAT_STATE_UPSIDE_DOWN;
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_UPSIDE_DOWN:
			if( SmoothContact() &&														// vehicle touching ground
				m_PhysObj.GetAngularVelocity()->Mag() < 4.0f &&							// not rotating rapidly
				m_PhysObj.GetOrientation()->m_vUp.Dot( CFVec3A::m_UnitAxisY ) > 0.0f )	// is rightside Up
			{
				// vehicle was somehow righted by external forces...
				if( m_pDriverBot )
				{
					m_eRatState = VEHICLERAT_STATE_DRIVING;
				}
				else
				{
					m_eRatState = VEHICLERAT_STATE_DRIVER_UNOCCUPIED;
				}
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_START_FLIPPING_UPRIGHT:
			{
				const CFMtx43A *pmPhysObjOr = m_PhysObj.GetOrientation();
				CFVec3A vCross;

				m_fFlipOverTime = _FLIP_TIME;
			}

			m_eRatState = VEHICLERAT_STATE_FLIPPING_UPRIGHT;
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_FLIPPING_UPRIGHT:
			if( m_fFlipOverTime <= 0.0f )
			{
				if( m_pDriverBot )
				{
					m_eRatState = VEHICLERAT_STATE_DRIVING;
				}
				else
				{
					m_eRatState = VEHICLERAT_STATE_DRIVER_UNOCCUPIED;
				}
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_STATE_DRIVER_UNOCCUPIED:
			if( SmoothContact() &&														// vehicle touching ground
				m_PhysObj.GetAngularVelocity()->Mag() < 4.0f &&							// not rotating rapidly
				m_PhysObj.GetOrientation()->m_vUp.Dot( CFVec3A::m_UnitAxisY ) < 0.0f )	// is rightside Up
			{
				// vehicle was somehow flipped upside down by external forces...
				m_eRatState = VEHICLERAT_STATE_UPSIDE_DOWN;
			}

		default:
			break;
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateRatGunState( void )
{
	switch( m_eGunState )
	{
		//---------------------------------------------------------------------
		case VEHICLERAT_GUN_STATE_UNOCCUPIED:
			if( IsStationObstructed( STATION_GUNNER ) )
			{
				if( !m_RatGun.GetOpenOrClosed() )
				{
					m_RatGun.SetOpenOrClosed( TRUE /*bOpen*/, _GUN_CAGE_TIME );
				}
			}
			else
			{
				if( m_RatGun.GetOpenOrClosed() )
				{
					m_RatGun.SetOpenOrClosed( FALSE /*bOpen*/, _GUN_CAGE_TIME );
				}
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_GUN_STATE_START_GUNNER_TO_ENTRY_POINT:
			m_RatGun.SetOpenOrClosed( TRUE /*bOpen*/, _GUN_CAGE_TIME );
			StartTransitionToGunnerVehicleCamera( m_pGunnerBot->m_nPossessionPlayerIndex );
			{
				CFVec3A vJumpVel;
				m_fGunEntryJumpTimer = ComputeEntryTrajectory( m_pGunnerBot->MtxToWorld()->m_vPos,				// start pos
										m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxGunnerAttach[m_uRatType]]->m_vPos,	// end pos
										3.0f,																	// minimum jump height, 
										m_pGunnerBot->m_pBotInfo_Gen->fGravity * m_pGunnerBot->m_fGravityMultiplier,	// gravity
										vJumpVel );																// output velocity

				m_pGunnerBot->StartVelocityJump( &vJumpVel );
				m_pGunnerBot->m_pWorldMesh->SetCollisionFlag(FALSE);
				m_pGunnerBot->SetNoCollideStateAir( TRUE );
				m_eGunState = VEHICLERAT_GUN_STATE_MOVE_GUNNER_TO_ENTRY_POINT;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_GUN_STATE_MOVE_GUNNER_TO_ENTRY_POINT:
			{
				m_fGunEntryJumpTimer -= FLoop_fPreviousLoopSecs;
				if( m_fGunEntryJumpTimer < 0.0f )
				{
					m_pGunnerBot->m_pWorldMesh->SetCollisionFlag(TRUE);
					m_pGunnerBot->SetNoCollideStateAir( FALSE );
					m_pGunnerBot->ImmobilizeBot();
					m_eGunState = VEHICLERAT_GUN_STATE_ENTERING;
					if( m_pDriverBot == NULL && IsDriveable() )
					{
						CrankEngine();
					}
				}
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_GUN_STATE_ENTERING:
			_StartGunnerEnterWork( FALSE /*bImmediately*/ );
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_GUN_STATE_POST_ENTER_WAIT:
			m_fGunStateTimer -= FLoop_fPreviousLoopSecs;
			if( m_fGunStateTimer <= 0.0f )
			{
				m_fGunStateTimer = 0.0f;
				m_eGunState = VEHICLERAT_GUN_STATE_OCCUPIED;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_GUN_STATE_OCCUPIED:
			if( m_pGunnerBot && IsDriverUseless(m_pGunnerBot) )
			{
				CBot* pOldGunner = m_pGunnerBot;
				ExitStation( m_pGunnerBot, FALSE /*bImmediately*/ );
				FASSERT(!m_RatGun.GetDriverBot());
				if( pOldGunner->AIBrain() )
				{
					aibrainman_NotifyMechEject( pOldGunner->AIBrain() );	//this brain got ejected from its mech
				}
				StartTransitionToGunnerBotCamera();
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLERAT_GUN_STATE_EXITING:
			m_fGunStateTimer -= FLoop_fPreviousLoopSecs;
			if( m_fGunStateTimer <= 0.0f )
			{
				m_fGunStateTimer = 0.0f;
				m_eGunState = VEHICLERAT_GUN_STATE_UNOCCUPIED;
			}
			break;
	}

}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateRatGun( void )
{
	m_RatGun.Work();

	// manually call work function for AI ratgun gunner.
	// This ensures that gunner work is called after ratgun work,
	// which in turn is called after RAT work.  This keeps all the
	// entities glued together positionally without any frame lag.
	if( m_RatGun.GetDriverBot() )
	{
		CBot *pGunner;
		pGunner = m_RatGun.GetDriverBot();

		if( !pGunner->IsPlayerBot() )
		{
			FASSERT( pGunner->AIBrain() );
			FASSERT( pGunner->AIBrain()->GetFlag_DisableEntityWork() );

			pGunner->Work();
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateDamageSmoke( void )
{
	if( m_pBotInfo_Vehicle->hSmokeParticle != FPARTICLE_INVALID_HANDLE )
	{
		f32 fUnitHealth = ComputeUnitHealth();

		if( fUnitHealth < _START_SMOKE_HEALTH )
		{
			f32 fIntensity = _START_SMOKE_HEALTH + ( 1.0f - fUnitHealth) * ( 1.0f - _START_SMOKE_HEALTH );

			if( !m_pEParticleSmoke )
			{
				m_pEParticleSmoke = eparticlepool_GetEmitter();
				if( m_pEParticleSmoke )
				{
					CFMtx43A mOr;

					if( SmoothContact() )
					{
						// smoke always emits straight up while on ground
						mOr.m_vFront.Set( CFVec3A::m_UnitAxisY );
						mOr.m_vRight.Set( CFVec3A::m_UnitAxisX );
						mOr.m_vUp.Set( CFVec3A::m_UnitAxisZ );
						mOr.m_vUp.Negate();
						mOr.m_vPos.Set( *m_apTagPoint_WS[ TAG_POINT_INDEX_CHASSIS ] );
					}
					else
					{
						// let smoke emit out top of vehicle while airborne
						mOr.m_vFront.Set( m_MtxToWorld.m_vUp );
						mOr.m_vUp.Set( m_MtxToWorld.m_vFront );
						mOr.m_vUp.Negate();
						mOr.m_vRight.Set( m_MtxToWorld.m_vRight );
						mOr.m_vPos.Set( *m_apTagPoint_WS[ TAG_POINT_INDEX_CHASSIS ] );
					}

					m_pEParticleSmoke->StartEmission( m_BotInfo_Vehicle.hSmokeParticle, fIntensity );

					m_pEParticleSmoke->Relocate_RotXlatFromUnitMtx_WS( &mOr );
					m_pEParticleSmoke->Attach_ToParent_WS( this, m_apszBoneNameTable[BONE_CHASSIS], FALSE );
				}
			}
			else
			{
				m_pEParticleSmoke->SetUnitIntensity( fIntensity );
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateDriverControls( void )
{
	if( m_pDriverBot != NULL )
	{
		// allow flipping right-side-up with button push while inside
		if( m_eRatState == VEHICLERAT_STATE_DRIVING || m_eRatState == VEHICLERAT_STATE_UPSIDE_DOWN )
		{
			// check for driver request to exit vehicle (must occur before ParseControls())
			if( m_bControls_Human && Controls() &&
				((CHumanControl *) Controls())->m_nPadFlagsAction & GAMEPAD_BUTTON_1ST_PRESS_MASK )
			{
				ActionNearby(m_pDriverBot);
			}		
		}

		// update the control inputs from the driver
		ParseControls();
	}
}

#if ENABLE_BOOST_FEATURE
//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateBoost( void )
{
	if( m_uRatType != RAT_TYPE_DROID )
	{
		// only droid RATs can boost
		return;
	}

	if( m_fBoostTimer > 0.0f )
	{
		// currently boosting.

		if( m_fBoostTimer > _BOOST_TIME - _BOOST_LIFT_TIME && SmoothContact() )
		{
			// do slight front-end lifting effect at start of boost
			CFVec3A vForce, vArm;
			f32 fForce;
			fForce = m_MtxToWorld.m_vUp.Dot( m_vGroundNormal );
			fForce *= fForce * fForce * fForce * fForce;
			FMATH_CLAMP( fForce, 0.0f, 1.0f );
			fForce *= _BOOST_LIFT_FORCE;
			vForce.Set( m_MtxToWorld.m_vUp );
			vForce.Mul( fForce );
			vArm.Set( m_MtxToWorld.m_vFront );
			vArm.Mul( _BOOST_LIFT_ARM );
			m_PhysObj.ApplyForce( &vForce, &vArm );
		}

		if( m_fBoostTimer > _BOOST_TIME - _BOOST_FOV_TIME )
		{
			// change to boost FOV over time as boost starts up
			f32 fScale;
			fScale = m_fBoostTimer - ( _BOOST_TIME - _BOOST_FOV_TIME );
			fScale = fmath_Div( fScale, _BOOST_FOV_TIME );
			FMATH_CLAMP( fScale, 0.0f, 1.0f );
			fScale = 1.0f - fScale;

			m_DriverCamera.SetFOV( FMATH_DEG2RAD( m_pBotInfo_DriverCamera->fFOV ) + fScale * _BOOST_DELTA_FOV );
		}
		else if( m_fBoostTimer < _BOOST_FOV_TIME )
		{
			// change back to normal FOV as boost is ending
			f32 fScale;
			fScale = fmath_Div( m_fBoostTimer, _BOOST_FOV_TIME );
			FMATH_CLAMP( fScale, 0.0f, 1.0f );
			m_DriverCamera.SetFOV( FMATH_DEG2RAD( m_pBotInfo_DriverCamera->fFOV ) + fScale * _BOOST_DELTA_FOV );
		}

		// update unit boost value for boost meter
		m_fUnitBoost = fmath_Div( m_fBoostTimer, _BOOST_TIME );

		m_fBoostTimer -= FLoop_fPreviousLoopSecs;

		if( m_fBoostTimer <= 0.0f )
		{
			// finished with boost, set recovery time and reduce speed
			m_fBoostTimer = -_BOOST_RECOVERY_TIME;
			m_fMaxSpeed = m_BotInfo_VehiclePhysics.fMaxSpeed;
			m_fOOMaxSpeed = 1.0f / m_BotInfo_VehiclePhysics.fMaxSpeed;

			// set FOV back to normal
			m_DriverCamera.SetFOV( FMATH_DEG2RAD( m_pBotInfo_DriverCamera->fFOV ) );

			// set boost meter to recovering mode
			_SetBoostMeter( TRUE /*bRecovering*/ );
		}
	}
	else if( m_fBoostTimer < 0.0f )
	{
		// update unit boost value for boost meter
		m_fUnitBoost = fmath_Div( _BOOST_RECOVERY_TIME + m_fBoostTimer, _BOOST_RECOVERY_TIME );

		// currently recovering from boost
		m_fBoostTimer += FLoop_fPreviousLoopSecs;

		if( m_fBoostTimer >= 0.0f )
		{
			// ready for next boost
			m_fBoostTimer = 0.0f;

			// set boost meter to boost time mode
			_SetBoostMeter( FALSE /*bRecovering*/ );
		}
	}
	else if( m_fThrottle > 0.9f && SmoothContact() && m_pDriverBot )
	{
		// okay to boost, check for boost trigger from user
		if( m_pDriverBot->m_nPossessionPlayerIndex >= 0 )
		{
			u32 nControlIndex = Player_aPlayer[m_pDriverBot->m_nPossessionPlayerIndex].m_nControllerIndex;
			if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_FIRE_PRIMARY]->fCurrentState > 0.5f )
			{
				m_fBoostTimer = _BOOST_TIME;
				m_fMaxSpeed = _MAX_BOOST_VEL_MUL * m_BotInfo_VehiclePhysics.fMaxSpeed;
				m_fOOMaxSpeed = 1.0f / m_fMaxSpeed;
				PlaySound( m_BotInfo_Rat.pSoundGroupBoost );
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_SetBoostMeter( BOOL bRecovering )
{
	CHud2* pHud;

	FASSERT( m_nPossessionPlayerIndex >= 0 );
	if( m_nPossessionPlayerIndex < 0 )
	{
		return;
	}

	pHud = CHud2::GetHudForPlayer( m_nPossessionPlayerIndex );

	CFColorRGBA startColor, endColor;

	if( bRecovering )
	{
		startColor.OpaqueRed();
		endColor.OpaqueRed();

		pHud->OverrideAmmoData( CHud2::RIGHT, CHud2::OVERRIDE_METER_MIL, &m_fUnitBoost, TRUE, &startColor, &endColor );
	}
	else
	{
		startColor.OpaqueGreen();
		endColor.OpaqueGreen();

		pHud->OverrideAmmoData( CHud2::RIGHT, CHud2::OVERRIDE_METER_MIL, &m_fUnitBoost, TRUE, &startColor, &endColor );
	}
}
#endif //ENABLE_BOOST_FEATURE

//-----------------------------------------------------------------------------
static void _WheelDebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact )
{
	const CFDebrisDef *pDebrisDef = pDebris->GetDebrisDef();
//	CBotPartMgr *pPartMgr = (CBotPartMgr *)pDebrisDef->m_pUser;

	if( nReason == CFDebrisDef::CALLBACK_REASON_BUILD_TRACKER_SKIP_LIST )
	{
		return;
	}

	if( nReason == CFDebrisDef::CALLBACK_REASON_COLLISION )
	{
	} else if( (nReason == CFDebrisDef::CALLBACK_REASON_DEAD) || (nReason == CFDebrisDef::CALLBACK_REASON_START_FADE) )
	{
	}

//	pDebris->Kill();
}

//-----------------------------------------------------------------------------
void CVehicleRat::_RandomExplosionOnBody( void )
{
	if( m_hExplosionGroup == FEXPLOSION_INVALID_HANDLE )
	{
		return;
	}

	CFMtx43A *pBoneMtx;
	s32 nBoneIndex = fmath_RandomChoice( BONE_COUNT );
	s32 nBone = m_pWorldMesh->FindBone( m_apszBoneNameTable[ nBoneIndex ] );

	// This assert may go off until bones in droid rat match those in mil rat.
	// Okay to ignore.
	FASSERT( nBone >= 0 );
	if( nBone < 0 )
	{
		return;
	}

	pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ nBone ];

	FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
	if( hSpawner == FEXPLOSION_INVALID_HANDLE )
	{
		return;
	}

	FExplosionSpawnParams_t SpawnParams;

	SpawnParams.InitToDefaults();

	SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
	SpawnParams.Pos_WS = pBoneMtx->m_vPos;
	SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
	SpawnParams.uSurfaceType = 0;
	SpawnParams.pDamageProfile = GetDamageProfile();
	SpawnParams.pDamager = NULL;

	CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );

//	// push vehicle around
//	CFVec3A vMomentum;
//	vMomentum.Set( m_MtxToWorld.m_vUp );
//	vMomentum.Mul( fmath_RandomFloatRange( _MIN_EXPLOSION_FORCE, _MAX_EXPLOSION_FORCE ) );
//	m_PhysObj.ApplyMomentum( &vMomentum, &pBoneMtx->m_vPos );
}

//-----------------------------------------------------------------------------
void CVehicleRat::_BigExplosion( void )
{
	if( m_hExplosionGroup == FEXPLOSION_INVALID_HANDLE ||
		m_hBigFinalExplosion == FEXPLOSION_INVALID_HANDLE )
	{
		return;
	}

	FExplosionSpawnParams_t SpawnParams;
	CFMtx43A *pBoneMtx;
	FExplosion_SpawnerHandle_t hSpawner;

	pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndexAxle[m_uRatType][AXLE_BONE_ARRAY_L_REAR_INDEX] ];
	SpawnParams.InitToDefaults();
	SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
	SpawnParams.Pos_WS = pBoneMtx->m_vPos;
	SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
	SpawnParams.uSurfaceType = 0;
	SpawnParams.pDamageProfile = GetDamageProfile();
	SpawnParams.pDamager = NULL;

	// spawn the big explosion
	hSpawner = CExplosion2::GetExplosionSpawner();
	if( hSpawner != FEXPLOSION_INVALID_HANDLE )
	{
		CExplosion2::SpawnExplosion( hSpawner, m_hBigFinalExplosion, &SpawnParams );
	}

	hSpawner = CExplosion2::GetExplosionSpawner();
	if( hSpawner != FEXPLOSION_INVALID_HANDLE )
	{
		pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndexAxle[m_uRatType][AXLE_BONE_ARRAY_R_REAR_INDEX] ];
		SpawnParams.Pos_WS = pBoneMtx->m_vPos;
		CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );
	}

	hSpawner = CExplosion2::GetExplosionSpawner();
	if( hSpawner != FEXPLOSION_INVALID_HANDLE )
	{
		pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndexAxle[m_uRatType][AXLE_BONE_ARRAY_L_FRONT_INDEX] ];
		SpawnParams.Pos_WS = pBoneMtx->m_vPos;
		CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );
	}

	hSpawner = CExplosion2::GetExplosionSpawner();
	if( hSpawner != FEXPLOSION_INVALID_HANDLE )
	{
		pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ m_anBoneIndexAxle[m_uRatType][AXLE_BONE_ARRAY_R_FRONT_INDEX] ];
		SpawnParams.Pos_WS = pBoneMtx->m_vPos;
		CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_BlowOffRandomWheel( void )
{
	s32 nWheel;

	for( nWheel = 0; nWheel <= COLPOINT_R_REAR_WHEEL_INDEX; nWheel++ )
	{
		if( !(m_aRatColPoints[nWheel].uFlags & RATCOLPOINT_FLAG_ENABLE_COLLISION) )
		{
			continue;
		}

		m_aRatColPoints[nWheel].uFlags &= ~RATCOLPOINT_FLAG_ENABLE_COLLISION;

		CFDebrisSpawner DebrisSpawner;
		const CFDebrisGroup *pBoneGeoDebrisGroup = m_pBotInfo_Gen->apBoneGeoDebrisGroup[ 0 ];
		const CFSphere *pBoneSphere_BS;
		CFMtx43A *pBoneMtx;
		CFVec3A BoneSphereCenter_WS;
		CFVec3A vSpawnDir;
		s32 nBoneIndex = m_anBoneIndexWheel[m_uRatType][nWheel];

		DebrisSpawner.InitToDefaults();

		vSpawnDir.Set( m_MtxToWorld.m_vRight );
		if( !(nWheel & 0x00000001) )
		{
			vSpawnDir.Negate();
		}
		vSpawnDir.Add(CFVec3A::m_UnitAxisY);
		vSpawnDir.Add( m_NormVelocityXZ_WS );
		if( vSpawnDir.SafeUnitAndMag( vSpawnDir ) <= 0.0f )
		{
			vSpawnDir.Set( CFVec3A::m_UnitAxisY );
		}

		pBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];
		pBoneSphere_BS = &m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].SegmentedBoundSphere_BS;
		BoneSphereCenter_WS.Set( pBoneSphere_BS->m_Pos );
		pBoneMtx->MulPoint( DebrisSpawner.m_Mtx.m_vPos, BoneSphereCenter_WS );

		DebrisSpawner.m_pDebrisGroup = pBoneGeoDebrisGroup;
		DebrisSpawner.m_Mtx.m_vZ = vSpawnDir;
		DebrisSpawner.m_fMinSpeed = m_fSpeed_WS + _MIN_DEBRIS_WHEEL_VEL_ADD;
		DebrisSpawner.m_fMaxSpeed = m_fSpeed_WS + _MAX_DEBRIS_WHEEL_VEL_ADD;
		DebrisSpawner.m_fUnitDirSpread = 0.3f;
		DebrisSpawner.m_fScaleMul = 1.0f;
		DebrisSpawner.m_fGravityMul = 4.0f;
		DebrisSpawner.m_nMinDebrisCount = 1;
		DebrisSpawner.m_nMaxDebrisCount = 1;
		DebrisSpawner.m_pFcnCallback = _WheelDebrisCallback;
		DebrisSpawner.m_pUser = this;
		DebrisSpawner.m_nFlags = CFDebrisSpawner::FLAG_BUILD_TRACKER_SKIP_LIST;
		DebrisSpawner.m_pSmokeTrailAttrib = &m_SmokeTrailAttrib;

		if( DebrisSpawner.Spawn( m_pWorldMesh, nBoneIndex ) )
		{
			// add an explosion where the wheel came off
			if( m_hExplosionGroup != FEXPLOSION_INVALID_HANDLE )
			{
				FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
				if( hSpawner != FEXPLOSION_INVALID_HANDLE )
				{
					FExplosionSpawnParams_t SpawnParams;

					SpawnParams.InitToDefaults();

					SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
					SpawnParams.Pos_WS = DebrisSpawner.m_Mtx.m_vPos;
					SpawnParams.UnitDir = m_MtxToWorld.m_vUp;
					SpawnParams.uSurfaceType = 0;
					SpawnParams.pDamageProfile = GetDamageProfile();
					SpawnParams.pDamager = NULL;

					CExplosion2::SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );
				}
			}
		}
		else
		{
			DEVPRINTF( "Unable to spawn wheel debris" );
		}

		break;
	}

}

//-----------------------------------------------------------------------------
// Work done by the start-driver-enter state.
// Also called directly from EnterStation() when immediate entry is requested.
// This is done so that there is not a one frame delay before AI can check 
// to see if it is in the vehicle.
void CVehicleRat::_StartDriverEnterWork( BOOL bImmediately )
{
	DriverEnter( m_pDriverBot, NULL, bImmediately );
	LocateDriverAtBone( m_nBoneIdxDriverAttach[m_uRatType] );

	m_PhysObj.SetGravity( _GROUND_GRAVITY );

	m_eRatState = VEHICLERAT_STATE_ENGINE_STARTING;
}

//-----------------------------------------------------------------------------
// Work done by the start-gunner-enter state.
// Also called directly from EnterStation() when immediate entry is requested.
// This is done so that there is not a one frame delay before AI can check 
// to see if it is in the vehicle.
void CVehicleRat::_StartGunnerEnterWork( BOOL bImmediately )
{
	s32 nStartingIndex = m_nPossessionPlayerIndex;

		// start cage closing animation
	m_RatGun.SetOpenOrClosed( FALSE /*bOpen*/, _GUN_CAGE_TIME );
	m_fGunStateTimer -= FLoop_fPreviousLoopSecs;
	if( m_fGunStateTimer <= 0.0f )
	{
		// undo the "freeze" from EnterStation()
		m_pGunnerBot->MobilizeBot();
		m_pGunnerBot->m_pWorldMesh->SetCollisionFlag(TRUE);

		if( m_RatGun.SetSiteWeaponDriver( m_pGunnerBot ) )
		{
			m_fGunStateTimer = 0.25f;
			m_eGunState = VEHICLERAT_GUN_STATE_POST_ENTER_WAIT;
			if( m_pDriverBot == NULL || m_pDriverBot->m_nPossessionPlayerIndex < 0 )
			{
				m_nPossessionPlayerIndex = m_pGunnerBot->m_nPossessionPlayerIndex;
			}

			if( nStartingIndex < 0 && m_nPossessionPlayerIndex >= 0 )
			{
				// if a player gets into vehicle which previously had 3D emitters,
				// re-allocate as 2D
				_AllocateEngineEmitters();
			}

			// tell the bot what vehicle he is driving
			m_pGunnerBot->m_pDrivingVehicle = this;

			if( m_pGunnerBot->IsPlayerBot() && level_IsRacingLevel() )
			{
				SetHealthContainerCount( m_pGunnerBot->HealthContainerCount() );
				SetNormHealth( m_pGunnerBot->NormHealth() );
			}

			// give player gunner the armor profile of a vehicle occupant
			if( m_pGunnerBot->IsPlayerBot() )
			{
				FASSERT( m_pPreviousGunnerArmorProfile == NULL );
				m_pPreviousGunnerArmorProfile = m_pGunnerBot->GetArmorProfile();
				if( level_IsRacingLevel() )
				{
					m_pGunnerBot->SetArmorProfile( m_pBotInfo_Vehicle->pOccupantRacingArmorProfile );
				}
				else
				{
					m_pGunnerBot->SetArmorProfile( m_pBotInfo_Vehicle->pOccupantArmorProfile );
				}
			}
		}
		else
		{
			FASSERT( FALSE );
			// something went wrong, exit station
			ExitStation( m_pGunnerBot, FALSE /*bImmediately*/ );
		}
	}
}

//-----------------------------------------------------------------------------
// abort driver entry into vehicle
void CVehicleRat::_AbortDriverEntry( void )
{
	m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
	m_pDriverBot->SetNoCollideStateAir( FALSE );
	m_pDriverBot = NULL;
	m_DriverCameraTrans.SetCameraStateAbort();

	m_eRatState = VEHICLERAT_STATE_DRIVER_UNOCCUPIED;
}

//-----------------------------------------------------------------------------
void CVehicleRat::_AllocateEngineEmitters( void )
{
	if( m_pEngineIdleEmitter )
	{
		m_pEngineIdleEmitter->Destroy();
	}
	m_pEngineIdleEmitter = AllocAndPlaySound( m_BotInfo_Engine.pSoundGroupEngineIdle );
	if( m_pEngineIdleEmitter == NULL )
	{
		DEVPRINTF( "Unable to alloc 'n play Vehicle EngineIdle SFX.\n" );
	}
	else
	{
		m_pEngineIdleEmitter->SetVolume( 0.0f );
	}


	if( m_pEngineLowEmitter )
	{
		m_pEngineLowEmitter->Destroy();
	}
	m_pEngineLowEmitter = AllocAndPlaySound( m_BotInfo_Engine.pSoundGroupEngineLow );
	if( m_pEngineLowEmitter == NULL )
	{
		DEVPRINTF( "Unable to alloc 'n play Vehicle EngineLow SFX.\n" );
	}
	else
	{
		m_pEngineLowEmitter->SetVolume( 0.0f );
	}

	if( m_pEngineHighEmitter )
	{
		m_pEngineHighEmitter->Destroy();
	}
	m_pEngineHighEmitter = AllocAndPlaySound( m_BotInfo_Engine.pSoundGroupEngineHigh );
	if( m_pEngineHighEmitter == NULL )
	{
		DEVPRINTF( "Unable to alloc 'n play Vehicle EngineHigh SFX.\n" );
	}
	else
	{
		m_pEngineHighEmitter->SetVolume( 0.0f );
	}

	if( m_pSkidEmitter )
	{
		m_pSkidEmitter->Destroy();
	}
	m_pSkidEmitter = AllocAndPlaySound( m_BotInfo_Rat.pSoundGroupSkid );
	if( m_pSkidEmitter == NULL )
	{
		DEVPRINTF( "Unable to alloc 'n play Vehicle Skid SFX.\n" );
	}
	else
	{
		m_pSkidEmitter->SetVolume( 0.0f );
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::_UpdateSkidSound( void )
{
	s32 nIndex;
	s32 nSkidWheels = 0;
	f32 fUnitSkidVolume;
	f32 fUnitSkidPitch;
	f32 fMinPitch, fMaxPitch, fMaxVolume;

	CFSoundInfo *pSoundInfo;
	CFSoundInfo::DecompressedSoundInfo_t InfoSkid;

	pSoundInfo = CFSoundGroup::GetSoundInfo( m_BotInfo_Rat.pSoundGroupSkid, 0 );
	if( pSoundInfo )
	{
		pSoundInfo->Decompress( &InfoSkid );
		fMinPitch = InfoSkid.m_fMinPitchMult;
		fMaxPitch = InfoSkid.m_fMaxPitchMult;
		if( IsPlayerBot() )
		{
			fMaxVolume = InfoSkid.m_fUnitVol2D;
		}
		else
		{
			fMaxVolume = InfoSkid.m_fUnitVol3D;
		}
	}
	else
	{
		fMinPitch = 1.0f;
		fMaxPitch = 1.0f;
		fMaxVolume = 1.0f;
		FASSERT_NOW;
	}

//	if( IsPlayerBot() )
//	{
//		for( nIndex = NUM_WHEELS; nIndex < MAX_COL_POINTS; nIndex++ )
//		{
//			if( m_aColPoints[nIndex].uResultFlags & CFPhysicsObject::COLLISION_RESULT_COLLIDE )
//			{
//				SCRIPT_MESSAGE( "Body Sphere %d collided", nIndex );
//			}
//			if( m_aColPoints[nIndex].uResultFlags & CFPhysicsObject::COLLISION_RESULT_CONTACT )
//			{
//				SCRIPT_MESSAGE( "Body Sphere %d contacted", nIndex );
//			}
//
//		}
//	}

	for( nIndex = 0; nIndex < CVehicleRat::NUM_WHEELS; nIndex++ )
	{
		if( (m_aColPoints[nIndex].uResultFlags & CFPhysicsObject::COLLISION_RESULT_CONTACT) ||
			(m_aColPoints[nIndex].uResultFlags & CFPhysicsObject::COLLISION_RESULT_COLLIDE) )
		{
			++nSkidWheels;
		}
	}

	if( nSkidWheels )
	{
		fUnitSkidVolume = m_PhysObj.GetVelocity()->Dot( m_MtxToWorld.m_vRight );
		fUnitSkidVolume = FMATH_FABS( fUnitSkidVolume );
		fUnitSkidVolume -= _MIN_SKID_VELOCITY;
		fUnitSkidVolume *= _OO_MAX_SKID_VELOCITY;
		FMATH_CLAMP( fUnitSkidVolume, 0.0f, 1.0f );
	}
	else
	{
		fUnitSkidVolume = 0.0f;
	}

	if( fUnitSkidVolume > 0.0f )
	{
		fUnitSkidPitch = fMinPitch + fUnitSkidVolume * ( fMaxPitch - fMinPitch );
	}
	else
	{
		fUnitSkidPitch = 1.0f;
	}

	// scale by max volume from csv file
	fUnitSkidVolume *= fMaxVolume;

	if( m_pSkidEmitter )
	{
		m_pSkidEmitter->SetVolume( fUnitSkidVolume );
		m_pSkidEmitter->SetFrequencyFactor( fUnitSkidPitch );
	}
}

//-----------------------------------------------------------------------------
// protected functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CVehicleRat::SetHatchOpen( BOOL bOpen, BOOL bImmediately )
{
	if( bImmediately )
	{
		if( bOpen )
		{
			m_eRightHatchState = HATCH_STATE_OPEN;
			m_fRightHatchUnitPos = 0.0f;
		}
		else
		{
			m_eRightHatchState = HATCH_STATE_CLOSED;
			m_fRightHatchUnitPos = 1.0f;
		}
	}
	else
	{
		if( bOpen )
		{
			m_eRightHatchState = HATCH_STATE_OPENING;
		}
		else
		{
			m_eRightHatchState = HATCH_STATE_CLOSING;
		}
	}

}

//-----------------------------------------------------------------------------
void CVehicleRat::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies )
{
	m_RatGun.Die(FALSE, FALSE);
	m_fDeathWorkTimer = fmath_RandomFloatRange( _MIN_DEATH_EVENT_TIME, _MAX_DEATH_EVENT_TIME );
	m_nDeathEvents = 0;

	if( m_uRatFlags & RAT_FLAG_DISABLE_COLLSION_ON_DEAD_RATS ) {
		if( m_pRatWorldMesh ) {
			m_pRatWorldMesh->SetCollisionFlag( FALSE );
		}
		if( m_pDeadRatWorldMesh ) {
			m_pDeadRatWorldMesh->SetCollisionFlag( FALSE );
		}
		m_RatGun.m_pWorldMesh->SetCollisionFlag( FALSE );
	}

	if( !(m_uRatFlags & RAT_FLAG_DONT_KILL_DRIVER_ON_DIE) ) {
		if( m_pDriverBot && !m_pDriverBot->IsPlayerBot() )
		{
			m_pDriverBot->Die();
		}
	}
	if( m_pGunnerBot && !m_pGunnerBot->IsPlayerBot() )
	{
		m_pGunnerBot->Die();
	}

	StopEngine();
	CBot::Die( FALSE, FALSE );
}

//-----------------------------------------------------------------------------
void CVehicleRat::UnDie( void )
{
	s32 nIndex;

	for( nIndex = 0; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		m_aRatColPoints[nIndex].uFlags |= RATCOLPOINT_FLAG_ENABLE_COLLISION;
	}
	FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
	FMATH_SETBITMASK( m_pDeadRatWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );

	CBot::UnDie();
	
	m_RatGun.UnDie();
}

//-----------------------------------------------------------------------------
enum
{
	_EXPLOSION_EVENT_BLAST = 0,
	_EXPLOSION_EVENT_BLAST2,
	_EXPLOSION_EVENT_WHEEL,
	_MAX_EXPLOSION_EVENTS,
};

void CVehicleRat::DeathWork( void )
{
	if( m_nDeathEvents < _NUM_DEATH_EVENTS )
	{
		m_fDeathWorkTimer -= FLoop_fPreviousLoopSecs;
		if( m_fDeathWorkTimer <= 0.0f )
		{
			m_fDeathWorkTimer = fmath_RandomFloatRange( _MIN_DEATH_EVENT_TIME, _MAX_DEATH_EVENT_TIME );

			if( m_nDeathEvents == _SWITCH_MESH_DEATH_EVENT )
			{
				_BigExplosion();
				m_RatGun.RemoveFromWorld();
			}
			else
			{
				switch( fmath_RandomChoice( _MAX_EXPLOSION_EVENTS ) )
				{
					case _EXPLOSION_EVENT_BLAST:
					case _EXPLOSION_EVENT_BLAST2:
						_RandomExplosionOnBody();
						break;

					case _EXPLOSION_EVENT_WHEEL:
						//_BlowOffRandomWheel();
						break;
				}
			}

			if( m_nDeathEvents == _NUM_DEATH_EVENTS - 1 )
			{
				m_uLifeCycleState = BOTLIFECYCLE_BURIED;	// to trigger "press A to continue" text
			}

			++m_nDeathEvents;
		}
	}

	if( m_nDeathEvents > _SWITCH_MESH_DEATH_EVENT )
	{
		m_pDeadRatWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
		m_pDeadRatWorldMesh->UpdateTracker();
		FMATH_CLEARBITMASK( m_pDeadRatWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		FMATH_SETBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		if( !(m_uRatFlags & RAT_FLAG_DISABLE_COLLSION_ON_DEAD_RATS) ) {
			m_pDeadRatWorldMesh->SetCollisionFlag( TRUE );
		}
	}

	CBot::DeathWork();
}

#define _MIN_AI_MANEUVERING_THROTTLE	( 0.2f )
#define _CAMERA_Y_ROT_RATE				( FMATH_DEG2RAD( 130.0f ) )
//-----------------------------------------------------------------------------
// move the vehicle in response to driver control inputs
void CVehicleRat::MoveVehicle( void )
{
	CFMtx43A mInvOr;
	CFVec3A vArm, vForce, vPoint, vVelForward;
	const CFMtx43A *pmPhysObjOr;
	const CFVec3A *pvVel;
	s32 nWheelsContacting = 0;
	s32 nWheel;
	s32 nIndex;
	f32 fVelForward;
	f32 fUnitVelForward;
	f32 fSteeringPos, fDeltaSteering, fMaxDeltaSteering;
	BOOL bGunnerIsDriving = FALSE;

// exception handler debug test
//#if 0
//#if 1
//volatile f32 a, b;
//a = 1.0f;
//b = 0.0f;
//a /= b;
//#else
//EXCEPTION_RECORD r;
//_EXCEPTION_POINTERS p;
//p.ExceptionRecord = &r;
//r.ExceptionCode = EXCEPTION_FLT_DIVIDE_BY_ZERO;
//r.ExceptionAddress = (PVOID) 0xbadc0de;
//extern long _fexception_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo );
//_fexception_Handler( &p );
//#endif
//#endif

	// force on wheels defaults to zero
	for( nWheel = 0; nWheel < NUM_WHEELS; nWheel++ )
	{
		m_aColPoints[nWheel].vAppliedForce.Zero();
	}

	// set body friction low unless vehicle is flipped upside down
	for( nIndex = NUM_WHEELS; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		if( m_MtxToWorld.m_vUp.Dot( CFVec3A::m_UnitAxisY ) > 0.0f )
		{
			// vehicle right-side-up, set body sphere friction low
			m_aColPoints[nIndex].vStaticFriction.Set( 0.3f, 0.3f, 0.3f );
			m_aColPoints[nIndex].vKineticFriction.Set( 0.3f, 0.3f, 0.3f );
		}
		else
		{
			// vehicle upside-down, set body sphere friction high
			m_aColPoints[nIndex].vStaticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );
			m_aColPoints[nIndex].vKineticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );
		}
	}

	if( IsPlayerBot() )
	{
		// player vehicle heavier to improve recovery from collisions with AI vehicles
		if( Level_aInfo[ Level_nLoadedIndex ].nLevel == LEVEL_WASTELAND_CHASE )
		{
			m_PhysObj.SetMass( 2000.0f );	// hack to make player rat harder to knock off course in Wasteland Chase.
		}
		else
		{
			m_PhysObj.SetMass( m_BotInfo_VehiclePhysics.fMass * _PLAYER_MASS_MUL ); 
		}
	}
	else
	{
		m_PhysObj.SetMass( m_BotInfo_VehiclePhysics.fMass ); 
	}

	if( SmoothContact() )
	{
		m_PhysObj.SetGravity( _GROUND_GRAVITY );
	}
	else
	{
		f32 fScale, fGravity;
		fScale = m_fHeightAboveGround * _OO_GRAVITY_TRANSITION_HEIGHT;
		FMATH_CLAMP_UNIT_FLOAT( fScale );

		fGravity = _GROUND_GRAVITY + fScale * ( _AIR_GRAVITY - _GROUND_GRAVITY );
		m_PhysObj.SetGravity( fGravity );
	}

#if ENABLE_BOOST_FEATURE
	if( m_fBoostTimer > 0.0f )
	{
		m_PhysObj.SetGravity( m_PhysObj.GetGravity() * _BOOST_GRAVITY_MULTIPLIER );
	}
#endif

	pmPhysObjOr = m_PhysObj.GetOrientation();

	// reset forced kinetic friction flags for rear wheels
	m_aColPoints[WHEEL_BONE_ARRAY_L_MIDDLE_INDEX].uClientFlags &= ~CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION;
	m_aColPoints[WHEEL_BONE_ARRAY_R_MIDDLE_INDEX].uClientFlags &= ~CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION;
	m_aColPoints[WHEEL_BONE_ARRAY_L_REAR_INDEX].uClientFlags &= ~CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION;
	m_aColPoints[WHEEL_BONE_ARRAY_R_REAR_INDEX].uClientFlags &= ~CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION;

	// front wheels always have static friction
	m_aColPoints[WHEEL_BONE_ARRAY_L_FRONT_INDEX].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_STATIC_FRICTION;
	m_aColPoints[WHEEL_BONE_ARRAY_R_FRONT_INDEX].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_STATIC_FRICTION;

	// get vehicle velocity
	pvVel = m_PhysObj.GetVelocity();
	// find component of velocity in direction that vehicle is pointing
	fVelForward = m_MtxToWorld.m_vFront.Dot( *pvVel );
	vVelForward.Mul( m_MtxToWorld.m_vFront, fVelForward );
	fUnitVelForward = fVelForward * m_fOOMaxSpeed;

	//----------------------------------------------------------------------------
	// read controls to get throttle and brake values
	if( m_pDriverBot != NULL && m_eRatState == VEHICLERAT_STATE_DRIVING && !IsDeadOrDying() )
	{
		if ( m_bControls_Human )
		{
			// get brake value 0.0 to 1.0 from trigger (no negative values possible with trigger)
			m_fBrake = 0.0f;
			m_fThrottle = m_fControlsHuman_Forward;

			if( m_fThrottle < 0.1f && m_fThrottle > -0.1f )
			{
				// prevent vehicle creep or drift due to throttle stick being slightly off 0.0f
				m_fThrottle = 0.0f;
			}

			if( fUnitVelForward < 0.0f && m_fThrottle < 0.0f )
			{
				// limit throttle when reversing
				m_fThrottle *= _MAX_REVERSE_THROTTLE;
			}
		}
		else // ai controls
		{
			m_fThrottle = m_ControlsBot_XlatNormSpeedXZ_WS.Mag();
			if( m_nControlsBot_Buttons & CBotControl::BUTTONFLAG_SPECIAL2 )
			{
				// AI uses BUTTONFLAG_SPECIAL2 to engage reverse
				m_fThrottle = -m_fThrottle;
			}

			m_fBrake = 0.0f;
		}
	}
	else
	{
		// nobody driving vehicle, or vehicle un-drivable
		m_fThrottle = 0.0f;
		m_fBrake = 0.0f;
	}

	//-------------------------------------------------------------------------
	// read controls and compute steering angle.

	// compute maximum change in turning angle allowable this frame
	fMaxDeltaSteering = _MAX_STEERING_RATE * FLoop_fPreviousLoopSecs;

	if( m_pDriverBot && m_bControls_Human )
	{
		// compute steering change requested by driver
		fSteeringPos = m_fControls_RotateCW;
#if defined(MA_PC_INPUT)
		// Desktop players steer with A/D (the left stick's X axis, unused by the driver on the
		// GameCube) as well as the right stick; the stronger input wins.
		if( FMATH_FABS( m_fControlsHuman_StrafeRight ) > FMATH_FABS( fSteeringPos ) )
		{
			fSteeringPos = m_fControlsHuman_StrafeRight;
		}
#endif
		fDeltaSteering = fSteeringPos - m_fRawSteeringPosition;
	}
	else
	{
		// ai controls
		if( m_fThrottle != 0.0f )
		{
			CFVec3A vFrontXZ = m_MtxToWorld.m_vFront;
			vFrontXZ.y = 0.0f;
			if( vFrontXZ.SafeUnitAndMag( vFrontXZ ) > 0.0f )
			{
				CFVec3A vRightXZ;
				vRightXZ.x = vFrontXZ.z;
				vRightXZ.z = -vFrontXZ.x;
				vRightXZ.y = 0.0f;

				CFVec3A vUnitControlsBot_XlatNormSpeedXZ_WS;

				if( vUnitControlsBot_XlatNormSpeedXZ_WS.SafeUnitAndMagXZ( m_ControlsBot_XlatNormSpeedXZ_WS ) > 0.0f )
				{
					f32 fDot = vUnitControlsBot_XlatNormSpeedXZ_WS.Dot( vFrontXZ );
					f32 fDotRight = vUnitControlsBot_XlatNormSpeedXZ_WS.Dot( vRightXZ );
					if( fDot < 0.0f )
					{
						// Target point is behind the vehicle.
						{
							// not the gunner driving
							if( fDotRight > 0.0f )
							{
								// target point is to the right, steer right
								fSteeringPos = 1.0f;
							}
							else
							{
								// target point is to the left, steer left
								fSteeringPos = -1.0f;
							}
						}
					}
					else
					{
						// Desired target point is in front of vehicle, turn towards target point.
						fSteeringPos = fmath_Sqrt( FMATH_FABS(fDotRight) );
						if( fDotRight < 0.0f )
						{
							fSteeringPos = -fSteeringPos;
						}
					}

					if( m_fThrottle < 0.0f && fUnitVelForward < 0.0f )
					{
						// trying to move backwards, so switch the steering direction
						fSteeringPos = -fSteeringPos;
					}

					fDeltaSteering = fSteeringPos - m_fRawSteeringPosition;
				}
				else
				{
					// m_ControlsBot_XlatNormSpeedXZ_WS was zero length
					fSteeringPos = 0.0f;
					fDeltaSteering = 0.0f;
				}
			}
			else
			{
				// vehicle is pointing straight up
				fSteeringPos = 0.0f;
				fDeltaSteering = 0.0f;
			}
		}
		else // ai throttle is zero
		{
			fSteeringPos = 0.0f;
			fDeltaSteering = 0.0f;
		}
	}

	// clamp requested steering change to maximum rate, if necessary.
	if( fDeltaSteering  > fMaxDeltaSteering )
	{
		m_fRawSteeringPosition += fMaxDeltaSteering;
	}
	else if( fDeltaSteering  < -fMaxDeltaSteering )
	{
		m_fRawSteeringPosition -= fMaxDeltaSteering;
	}
	else
	{
		m_fRawSteeringPosition = fSteeringPos;
	}

	// count how many wheels are currently sliding (middle and rear wheels only)
	s32 nSliding = 0;
	for( nWheel = COLPOINT_L_MIDDLE_WHEEL_INDEX; nWheel < COLPOINT_L_FRONT_BUMPER_INDEX; nWheel++ )
	{
		if( m_aColPoints[nWheel].uResultFlags & CFPhysicsObject::COLLISION_RESULT_KINETIC )
		{
			++nSliding;
		}
	}

	if( nSliding >= 3 )
	{
		// allow more steering control when sliding
		m_fSteeringPosition = m_fRawSteeringPosition;
	}
	else
	{
		m_fSteeringPosition = m_fRawSteeringPosition * ( 1.0f - (1.0f - _HIGH_SPEED_MAX_STEERING) * m_fClampedNormSpeedXZ_WS );
	}

//if( IsPlayerBot() )
//ftext_Printf( 0.15f, 0.32f, "~f1~C92929299~w1~al~s0.69clamping %.2f", m_PhysObj.GetClampingCoefficient() );


	//-------------------------------------------------------------------------
	// apply forward/reverse driving force to vehicle
	if( SmoothContact() )
	{
		f32 fForce;
		if( IsPlayerBot() )
		{
			// Player gets more engine power to offset extra mass of player vehicles
			fForce = _FORWARD_FORCE * _PLAYER_FORWARD_FORCE_MUL + FMATH_FABS( m_fThrottle ) * FMATH_FABS( m_fSteeringPosition ) * _ADDED_FORWARD_STEERING_FORCE;
		}
		else
		{
			fForce = _FORWARD_FORCE + FMATH_FABS( m_fThrottle ) * FMATH_FABS( m_fSteeringPosition ) * _ADDED_FORWARD_STEERING_FORCE;
		}

		// calculate remaining unit difference between throttle setting and actual speed.
		f32 fUnitDeltaSpeed;
		if( m_fThrottle >= 0.0f )
		{
			fUnitDeltaSpeed = m_fThrottle - fUnitVelForward;
		}
		else
		{
			fUnitDeltaSpeed = fUnitVelForward - m_fThrottle;
		}
		FMATH_CLAMP( fUnitDeltaSpeed, -1.0f, 1.0f );

		if( fUnitDeltaSpeed < -0.1f )
		{
			// if speed is substantially exceeding the throttle setting, apply opposite force to slow down
			fForce *= 0.6f * fmath_Sqrt( -fUnitDeltaSpeed );
			fForce = - fForce;
		}
		else if( fUnitDeltaSpeed < 0.0f )
		{
			// speed exceeds throttle setting, but not by much, so zero out force
			fForce = 0.0f;
		}
		else if( fUnitDeltaSpeed < 0.1f )
		{
			// as actual speed closely approaches throttle setting, reduce applied force to zero
			fForce = ( fUnitDeltaSpeed * ( 1.0f / 0.1f ) * fForce );
		}

		if( m_fThrottle < 0.0f )
		{
			// negate the applied force for negative throttle settings
			fForce = -fForce;
		}
#if 1
		// reduce driving force as slope steepens
		f32 fSlopeMul = CFVec3A::m_UnitAxisY.Dot( m_PhysObj.GetOrientation()->m_vFront );
		if( m_fThrottle >= 0.0f )
		{
			if( fSlopeMul > 0.0f )
			{
				fSlopeMul = 1.0f - fSlopeMul;
				FMATH_CLAMP_UNIT_FLOAT( fSlopeMul );
			}
			else
			{
				fSlopeMul = 1.0f;
			}
		}
		else
		{
			if( fSlopeMul < 0.0f )
			{

				fSlopeMul = 1.0f + fSlopeMul;
				FMATH_CLAMP_UNIT_FLOAT( fSlopeMul );
			}
			else
			{
				fSlopeMul = 1.0f;
			}
		}
		fSlopeMul = fmath_Sqrt( fSlopeMul );
#else
		f32 fSlopeMul = 1.0f;
#endif
		// If current speed is significantly different than throttle 
		// setting, apply throttle force to center-of-mass.
		// Otherwise apply no force, which allows physics object momentum 
		// clamping to take effect when vehicle speed is very low or stopped.
		// (Clamping is automatically disabled when an external force is
		// applied to the physics object.)
		// GetMaxUnitSpeed() check prevents vehicle from moving when max speed
		// is set to zero.
		if( FMATH_FABS( fUnitDeltaSpeed ) > 0.01f && GetMaxUnitSpeed() > 0.0f )
		{
			vForce.Set( m_MtxToWorld.m_vFront );
			vForce.Mul( fForce );
			vForce.Mul( fSlopeMul );
			m_PhysObj.ApplyForce( &vForce );
		}

//		if( IsPlayerBot() )
//		{
//			ftext_Printf( 0.15f, 0.28f, "~f1~C92929299~w1~al~s0.69fUnitDeltaSpeed %.4f", fUnitDeltaSpeed );
//			ftext_Printf( 0.15f, 0.30f, "~f1~C92929299~w1~al~s0.69m_fSpeed_WS %.2f", m_fSpeed_WS );
//			ftext_Printf( 0.15f, 0.32f, "~f1~C92929299~w1~al~s0.69m_fThrottle %.2f", m_fThrottle );
//			ftext_Printf( 0.15f, 0.34f, "~f1~C92929299~w1~al~s0.69fForce %.1f", fForce );
//		}
	}

	//-------------------------------------------------------------------------
	// detect and handle flipped-over vehicle
	if( m_fFlipOverTime > 0.0f )
	{
		CFVec3A vAxis;
		f32 fMom;

		// currently righting a vehicle that is flipped-over
		m_fFlipOverTime -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fFlipOverTime, 0.0f );

		if( m_MtxToWorld.m_vUp.Dot( CFVec3A::m_UnitAxisY ) > 0.9f )
		{
			// vehicle is close enough to upright that the flipover maneuver
			// can be terminated
			m_fFlipOverTime = 0.0f;
		}

		// find rotation axis by crossing vehicle up vector with world y axis
		vAxis.Cross( m_MtxToWorld.m_vUp, CFVec3A::m_UnitAxisY );
		if( vAxis.SafeUnitAndMag( vAxis ) > 0.0f )
		{
			// dot the unitized rotation axis with the vehicle's current angular
			// momentum to determine how much momentum must be added around the
			// desired axis to return the vehicle upright
			fMom = vAxis.Dot( *m_PhysObj.GetAngularMomentum() );		

			// compute and apply the delta momentum needed for flipping over
			fMom = _FLIP_UPRIGHT_MOMENTUM - fMom;
			vAxis.Mul( fMom );
			m_PhysObj.ApplyAngularMomentum( &vAxis );
		}
	}
	else if( SmoothContact() && // vehicle touching ground
		pmPhysObjOr->m_vUp.Dot( CFVec3A::m_UnitAxisY ) < 0.0f ) // is upside down
	{
		// vehicle is upside down, change to flipped-over state
		if( m_pDriverBot != NULL && m_eRatState == VEHICLERAT_STATE_DRIVING )
		{
			m_eRatState = VEHICLERAT_STATE_START_UPSIDE_DOWN;
		}
	}
#if !FANG_PRODUCTION_BUILD
	else
	{
#if 0
		// "sink" test
		if( m_bControls_Jump )
		{
			const CFVec3A *pPos = m_PhysObj.GetPosition();
			m_PhysObj.SetPosition( pPos->x, pPos->y -3.5f, pPos->z );
		}
#else
		// allow driver to "jump" the vehicle
		vPoint.Set( 0.0f, 0.0f, 0.0f );
//		vPoint.Set( m_MtxToWorld.m_vRight );
//		vPoint.Mul( 4.0f );
		vForce.Set( 0.0f, m_bControls_Jump * _JUMP_FORCE, 0.0f );
		m_PhysObj.ApplyForce( &vForce, &vPoint );
#endif
	}
#endif

	// Scale physics object damping based on velocity and applied force.
	// Reduces "drifting".  Momentum clamping is used to remove remaining drift
	if( SmoothContact() )
	{
		m_PhysObj.SetDynamicLinearDamping( TRUE, _MIN_GROUND_LINEAR_DAMPING, _GROUND_LINEAR_DAMPING );
		m_PhysObj.SetDynamicAngularDamping( TRUE, _MIN_GROUND_ANGULAR_DAMPING, _GROUND_ANGULAR_DAMPING );
	}
	else
	{
		m_PhysObj.SetDynamicLinearDamping( FALSE );
		m_PhysObj.SetDynamicAngularDamping( FALSE );
	}

	// turn on momentum clamping if on the ground, turn off if airborne.
	if( SmoothContact() )
	{
		m_PhysObj.SetMomentumClamping( TRUE, 3000.0f, 8000.0f );
		m_PhysObj.SetAngularMomentumClamping( TRUE, 7000.0f, 20000.0f );
	}
	else
	{
		m_PhysObj.SetMomentumClamping( FALSE );
		m_PhysObj.SetAngularMomentumClamping( FALSE );
	}


	_UpdateTireTextures();
PROTRACK_BEGINBLOCK( "Rat Physics" );
	m_pCurrentCollider = this;

	// Physics object simulate calls RAT collision function which updates bone positions,
	// so tell anim bone callback that this is not the final bone update for this frame.
	SetFinalBoneUpdate( FALSE );
	m_PhysObj.Simulate( FLoop_fPreviousLoopSecs );
	SetFinalBoneUpdate( TRUE );
PROTRACK_ENDBLOCK();

	UpdateMatrices();

	if( m_fMomentumReduction > 0.0f )
	{
		// slow vehicle down
		CFVec3A vMom;
		vMom.Set( *m_PhysObj.GetMomentum() );
		vMom.Negate();
		vMom.Mul( m_fMomentumReduction );
		m_PhysObj.ApplyMomentum( &vMom );
		m_fMomentumReduction = 0.0f;
	}

	// update velocity member variables
	m_Velocity_WS.Set( *(m_PhysObj.GetVelocity()) );
	mInvOr.Set( *(m_PhysObj.GetOrientation()) );
	mInvOr.AffineInvert( FALSE );
	mInvOr.MulPoint( m_Velocity_MS, m_Velocity_WS );
	VelocityHasChanged();
}


//-----------------------------------------------------------------------------
// put driver bot into vehicle
void CVehicleRat::DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName /* = NULL */, BOOL bImmediately /* = FALSE */ )
{
	s32 nIndex;
	s32 nStartingIndex = m_nPossessionPlayerIndex;

	if( pDriverBot == NULL )
	{
		return;
	}

	CVehicle::DriverEnter( pDriverBot, "R_Hatch" );

	if( pDriverBot->IsPlayerBot() && m_uRatType == RAT_TYPE_DROID )
	{
		CHud2* pHud;
		pHud = CHud2::GetHudForPlayer( pDriverBot->m_nPossessionPlayerIndex );
#if ENABLE_BOOST_FEATURE
		pHud->SetDrawFlags( CHud2::DRAW_ENABLEOVERRIDE | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR | CHud2::DRAW_AMMOBOXES );
		_SetBoostMeter( FALSE /*bRecovering*/ );
#else
		pHud->SetDrawFlags( CHud2::DRAW_ENABLEOVERRIDE | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR );
#endif
	}

	if( pDriverBot->IsPlayerBot() && level_IsRacingLevel() )
	{
		SetHealthContainerCount( pDriverBot->HealthContainerCount() );
		SetNormHealth( pDriverBot->NormHealth() );
	}

	// enable dirt emitters 
	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[nIndex], TRUE );
		}
	}

	m_pDriverBot = pDriverBot;

	if( m_pDriverBot->IsPlayerBot() )
	{
		m_PhysObj.SetOtherObjElasticity( _PLAYER_OTHER_OBJ_ELASTICITY );
	}
	else
	{
		m_PhysObj.SetOtherObjElasticity( _OTHER_OBJ_ELASTICITY );
	}

	if( nStartingIndex < 0 && m_nPossessionPlayerIndex >= 0 )
	{
		// if a player gets into vehicle which previously had 3D emitters,
		// re-allocate as 2D
		_AllocateEngineEmitters();
	}
}

//-----------------------------------------------------------------------------
// remove driver from vehicle
void CVehicleRat::DriverExit( CBot *pDriverBot )
{
	CFVec3A vOffset;
	s32 nIndex;

	if( pDriverBot == NULL )
	{
		return;
	}

	if( pDriverBot != m_pDriverBot )
	{
		return;
	}

	if( m_eRatState == VEHICLERAT_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT || m_eRatState == VEHICLERAT_STATE_MOVE_DRIVER_TO_ENTRY_POINT )
	{
		// DriverEnter() has not yet been called.
		_AbortDriverEntry();
		return;
	}

	CVehicle::DriverExit( pDriverBot );

	pDriverBot->DetachFromParent();

	// turn off dirt emitters
	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[nIndex], FALSE );
		}
	}

	m_pDriverBot = NULL;

	if( m_pGunnerBot )
	{
		m_nPossessionPlayerIndex = m_pGunnerBot->m_nPossessionPlayerIndex;
	}

	m_PhysObj.SetOtherObjElasticity( 0.0f );
}

//-----------------------------------------------------------------------------
// update the location of the 3rd person driver camera
void CVehicleRat::UpdateDriverCamera( void )
{
	if( m_DriverCameraTrans.IsCameraInactive() )
	{
		return;
	}

	if( m_DriverCameraTrans.GetPlayerIndex() < 0 )
	{
		return;
	}

	if( SmoothContact() )
	{
		m_DriverCamera.SetAirTime( 0.0f );
		m_GunnerCamera.SetAirTime( 0.0f );
	}
	else
	{
		// m_fTimeSinceLastContact doesn't have hysteresis of SmoothContact()
		m_DriverCamera.SetAirTime( m_fTimeSinceLastContact );
		m_GunnerCamera.SetAirTime( m_fTimeSinceLastContact );
	}

	f32 fAim = m_fControls_AimDown;
	if( FMATH_FABS( fAim ) < _CAM_HEIGHT_DEAD_ZONE )
	{
		// create a dead zone where camera height won't change to help prevent
		// accidental changes in height
		fAim = 0.0f;
	}
	else
	{
		fAim -= _CAM_HEIGHT_DEAD_ZONE;
		fAim *= 1.0f / (1.0f - _CAM_HEIGHT_DEAD_ZONE);
	}
	m_DriverCamera.SetHeight( fAim );

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();
	m_DriverCamera.Update( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexTurretAttach[m_uRatType]], *m_PhysObj.GetVelocity(), m_pWorldMesh, m_vGroundNormal );

	m_DriverCameraTrans.UpdateTransition();
}

//-----------------------------------------------------------------------------
// update the location of the 3rd person gunner camera
void CVehicleRat::UpdateGunnerCamera( void )
{
	CRatGun *pRatGun;
	s32 nBoneIndex;

	if( m_GunnerCameraTrans.IsCameraInactive() ||  m_GunnerCameraTrans.GetPlayerIndex() < 0 )
	{
		return;
	}

	if( SmoothContact() )
	{
		m_GunnerCamera.SetAirTime( 0.0f );
	}
	else
	{
		// m_fTimeSinceLastContact doesn't have hysteresis of SmoothContact()
		m_GunnerCamera.SetAirTime( m_fTimeSinceLastContact );
	}

	u32 nControlIndex = Player_aPlayer[m_GunnerCameraTrans.GetPlayerIndex()].m_nControllerIndex;
	if( IsDriveable() )
	{
		m_GunnerCamera.SetHeightFromXZPlane( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState );
	}
	else
	{
		m_GunnerCamera.SetElevationAngle( m_RatGun.GetGunPitch() + _GUNNERCAM_ELEVATION_ANGLE );
	}

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	pRatGun = (CRatGun*) m_RatGun.GetMyGun();
	nBoneIndex = pRatGun->m_pAliveMesh->FindBone( pRatGun->m_apszBoneNameList[ CRatGun::BONE_ATTACHPOINT_GUNNER ] );
	if( nBoneIndex >= 0 )
	{
		m_GunnerCamera.Update( pRatGun->m_pAliveMesh->GetBoneMtxPalette()[nBoneIndex], *m_PhysObj.GetVelocity(), m_pWorldMesh, m_vGroundNormal );
	}
	m_GunnerCameraTrans.UpdateTransition();
}

//-----------------------------------------------------------------------------
void CVehicleRat::StartTransitionToGunnerVehicleCamera( s32 nPlayerIndex )
{
	m_GunnerCameraTrans.InitTransitionToVehicleCamera( nPlayerIndex );

	// snap the camera to a position behind the vehicle (but will still transition smoothly from bot camera)
	m_GunnerCamera.Snap();
}

//-----------------------------------------------------------------------------
void CVehicleRat::StartTransitionToGunnerBotCamera( void )
{
	m_GunnerCameraTrans.InitTransitionToBotCamera();
}

//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyBecameActive( void )
{
	if( m_pPartMgr->IsCreated() )
	{
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_CRATE_RIGHT_REAR );
	}

	CVehicle::ClassHierarchyBecameActive();

	_AllocateEngineEmitters();
}

//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyBecameInactive( void )
{
	CVehicle::ClassHierarchyBecameInactive();

	if( m_pEngineIdleEmitter )
	{
		m_pEngineIdleEmitter->Destroy();
		m_pEngineIdleEmitter = FALSE;
	}

	if( m_pEngineLowEmitter )
	{
		m_pEngineLowEmitter->Destroy();
		m_pEngineLowEmitter = FALSE;
	}

	if( m_pEngineHighEmitter )
	{
		m_pEngineHighEmitter->Destroy();
		m_pEngineHighEmitter = FALSE;
	}

	if( m_pSkidEmitter )
	{
		m_pSkidEmitter->Destroy();
		m_pSkidEmitter = FALSE;
	}
}


//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CVehicleRat::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;

	return TRUE;
}


//-----------------------------------------------------------------------------
void CVehicleRat::UninitSystem( void )
{
	if( m_bSystemInitialized )
	{
		m_AnimStackDef.Destroy();
		m_bSystemInitialized = FALSE;
	}
}



//-----------------------------------------------------------------------------
CVehicleRat::CVehicleRat() : CVehicle()
{
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


//-----------------------------------------------------------------------------
CVehicleRat::~CVehicleRat()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld(TRUE);
		ClassHierarchyDestroy();
	}
}



//-----------------------------------------------------------------------------
BOOL CVehicleRat::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CVehicleRatBuilder *pBuilder = (CVehicleRatBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_VehicleRatBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_VEHICLERAT );
	}

	// Set our builder parameters...

	// Create an entity...
	return CVehicle::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyDestroy( void )
{
	// Delete the items that we had instantiated for us...

	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	if( m_pDeadRatWorldMesh )
	{
		fdelete( m_pDeadRatWorldMesh );
	}

	DestroyDustAndCloudParticles();
	
	if( m_pZobbyME )
	{
		fdelete( m_pZobbyME );
		m_pZobbyME = NULL;
	}

	CVehicle::ClassHierarchyDestroy();
	m_RatGun.Destroy();
}

//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled )
{
	CVehicle::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	if( m_pZobbyME )
	{
		m_pZobbyME->DrawEnable( bDrawingHasBeenEnabled );
	}
}

//-----------------------------------------------------------------------------
BOOL CVehicleRat::ClassHierarchyLoadSharedResources( void )
{
	FTexDef_t *pTexDef = NULL;	

	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	++m_nBotClassClientCount;

	if( !CVehicle::ClassHierarchyLoadSharedResources() )
	{
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...

		return FALSE;
	}

	if( m_nBotClassClientCount > 1 )
	{
		// Resources already loaded...

		return TRUE;
	}

	// Resources not yet loaded...

	FResFrame_t ResFrame = fres_GetFrame();

	if( !_BuildAnimStackDef() )
	{
		goto _ExitWithError;
	}

	m_hDirtParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _DIRT_PARTICLE_DEF_NAME );
	if( m_hDirtParticleDef == FPARTICLE_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CVehicleRat::InitSystem(): Could not find particle definition '%s'.\n", _DIRT_PARTICLE_DEF_NAME );
	}

	m_hCloudParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _CLOUD_PARTICLE_DEF_NAME );
	if( m_hCloudParticleDef == FPARTICLE_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CVehicleRat::InitSystem(): Could not find particle definition '%s'.\n", _CLOUD_PARTICLE_DEF_NAME );
	}

	m_hWaterParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _WATER_PARTICLE_DEF_NAME );
	if( m_hWaterParticleDef == FPARTICLE_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CVehicleRat::InitSystem(): Could not find particle definition '%s'.\n", _WATER_PARTICLE_DEF_NAME );
	}

	m_hExplosionGroup = CExplosion2::GetExplosionGroup( _EXPLOSION_GROUP_NAME );
	if( m_hExplosionGroup == FEXPLOSION_INVALID_HANDLE )
	{
		DEVPRINTF( "CVehicleRat::InitSystem(): Could not find explosion group '%s'.\n", _EXPLOSION_GROUP_NAME );
	}
	// by default the big explosion is the same as the regular explosion
	m_hBigFinalExplosion = m_hExplosionGroup;

	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
	if( pTexDef == NULL )
	{
		DEVPRINTF( "CVehicleRat::InitSystem(): Could not load smoke trail texture.\n" );
	}
	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 1.0f;
	m_SmokeTrailAttrib.fScaleMax_WS = 1.3f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.3f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.6f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.7f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_SmokeTrailAttrib.EndColorRGB.Set( 0.7f, 0.7f, 0.7f );
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.0f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.2f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	// initialize sfx handles

	if( !fresload_Load( FSNDFX_RESTYPE, _RAT_SOUND_BANK ) ) 
	{
		DEVPRINTF( "CVehicleRat Load Shared Resources: Could not load sound effect bank '%s'\n", _RAT_SOUND_BANK );
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		goto _ExitWithError;
	}

	// Load mesh resources
	if( Level_aInfo[ Level_nLoadedIndex ].nLevel == LEVEL_RAT_RACE || Level_aInfo[ Level_nLoadedIndex ].nLevel == LEVEL_WASTELAND_CHASE )
	{
		// load droid rat meshes on rat race level only
		m_pDroidRatMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszDroidRatMeshFilename);
		if( m_pDroidRatMesh == NULL )
		{
			DEVPRINTF( "CVehicleRat Load Shared Resources: Could not find mesh '%s'\n", _apszDroidRatMeshFilename );
			goto _ExitWithError;
		}

//		m_pDeadDroidRatMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszDeadDroidRatMeshFilename);
//		if( m_pDeadDroidRatMesh == NULL )
//		{
//			DEVPRINTF( "CVehicleRat Load Shared Resources: Could not find mesh '%s'\n", _apszDeadDroidRatMeshFilename );
//			goto _ExitWithError;
//		}
		m_pDeadDroidRatMesh = NULL;
	}
	else
	{
		m_pDroidRatMesh = NULL;
		m_pDeadDroidRatMesh = NULL;
	}

	m_pRatMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszRatMeshFilename);

	if( m_pRatMesh == NULL )
	{
		DEVPRINTF( "CVehicleRat Load Shared Resources: Could not find mesh '%s'\n", _apszRatMeshFilename );
		goto _ExitWithError;
	}

	m_pDeadRatMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszDeadRatMeshFilename);
	if( m_pDeadRatMesh == NULL )
	{
		DEVPRINTF( "CVehicleRat Load Shared Resources: Could not find mesh '%s'\n", _apszRatMeshFilename );
		goto _ExitWithError;
	}
	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyUnloadSharedResources( void )
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	CVehicle::ClassHierarchyUnloadSharedResources();
}


//-----------------------------------------------------------------------------
BOOL CVehicleRat::ClassHierarchyBuild( void )
{
	FMeshInit_t MeshInit;
	u32 i;
	s32 nBoneIndex;
	s32 nIndex;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CVehicleRatBuilder *pBuilder = (CVehicleRatBuilder *)GetLeafClassBuilder();

	// Set input parameters for CBot creation...
	pBuilder->m_pBotDef = &m_BotDef;

	// Build parent class...
	if( !CVehicle::ClassHierarchyBuild() )
	{
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();
	SetMaxUnitSpeed( 1.0f, FALSE /*bInternalOnly*/ );


	// Create dead world mesh
	m_pDeadRatWorldMesh = fnew CFWorldMesh;
	if( m_pDeadRatWorldMesh == NULL )
	{
		DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Not enough memory to create destructed RAT CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the dead RAT world mesh...
//	if( Level_aInfo[ Level_nLoadedIndex ].nLevel == LEVEL_RAT_RACE && AIBrain() && AIBrain()->GetRace() == AIRACE_DROID )
//	{
//		MeshInit.pMesh = m_pDeadDroidRatMesh;
//	}
//	else
	{
		MeshInit.pMesh = m_pDeadRatMesh;
	}
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	m_pDeadRatWorldMesh->Init( &MeshInit );

	m_pDeadRatWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pDeadRatWorldMesh->m_pUser = this;
	m_pDeadRatWorldMesh->SetUserTypeBits( TypeBits() );
	m_pDeadRatWorldMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pDeadRatWorldMesh->SetCollisionFlag( FALSE );
	m_pDeadRatWorldMesh->SetLineOfSightFlag( FALSE );
	m_pDeadRatWorldMesh->UpdateTracker();
	m_pDeadRatWorldMesh->RemoveFromWorld();

	// Create world mesh
	m_pRatWorldMesh = fnew CFWorldMesh;
	if( m_pRatWorldMesh == NULL )
	{
		DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Not enough memory to create RAT CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the RAT world mesh...
	if( (pBuilder->m_uFlags & RAT_BUILDER_FLAG_DROID_MESH) )
	{
		if( m_pDroidRatMesh )
		{
			 MeshInit.pMesh = m_pDroidRatMesh;
			m_uRatType = RAT_TYPE_DROID;
		}
		else
		{
			SCRIPT_ERROR( "Can't load Yellow RAT mesh on this level." );
			MeshInit.pMesh = m_pRatMesh;
			m_uRatType = RAT_TYPE_MIL;
		}
	}
	else
	{
		MeshInit.pMesh = m_pRatMesh;
		m_uRatType = RAT_TYPE_MIL;
	}

	MeshInit.nFlags = 0;
	MeshInit.fCullDist = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( pBuilder->m_EC_Mtx_WS );
	m_pRatWorldMesh->Init( &MeshInit );

	m_pRatWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pRatWorldMesh->m_pUser = this;
	m_pRatWorldMesh->SetUserTypeBits( TypeBits() );
	m_pRatWorldMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pRatWorldMesh->SetCollisionFlag( TRUE );
	m_pRatWorldMesh->SetLineOfSightFlag( FALSE );
	m_pRatWorldMesh->UpdateTracker();

	m_pWorldMesh = m_pRatWorldMesh;
	m_pWorldMesh->RemoveFromWorld();

	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) )
	{
		DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_REST, 1.0f );

	m_anBoneIndexAxle[m_uRatType][ AXLE_BONE_ARRAY_L_FRONT_INDEX  ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_AXLE_FRONT ] );
	m_anBoneIndexAxle[m_uRatType][ AXLE_BONE_ARRAY_R_FRONT_INDEX  ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_AXLE_FRONT ] );
	m_anBoneIndexAxle[m_uRatType][ AXLE_BONE_ARRAY_L_MIDDLE_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_AXLE_MIDDLE ] );
	m_anBoneIndexAxle[m_uRatType][ AXLE_BONE_ARRAY_R_MIDDLE_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_AXLE_MIDDLE ] );
	m_anBoneIndexAxle[m_uRatType][ AXLE_BONE_ARRAY_L_REAR_INDEX   ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_AXLE_REAR ] );
	m_anBoneIndexAxle[m_uRatType][ AXLE_BONE_ARRAY_R_REAR_INDEX   ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_AXLE_REAR ] );

	m_anBoneIndexHubcap[m_uRatType][ HUBCAP_BONE_ARRAY_L_FRONT_INDEX  ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_HUBCAP_FRONT ] );
	m_anBoneIndexHubcap[m_uRatType][ HUBCAP_BONE_ARRAY_R_FRONT_INDEX  ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_HUBCAP_FRONT ] );
	m_anBoneIndexHubcap[m_uRatType][ HUBCAP_BONE_ARRAY_L_MIDDLE_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_HUBCAP_MIDDLE ] );
	m_anBoneIndexHubcap[m_uRatType][ HUBCAP_BONE_ARRAY_R_MIDDLE_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_HUBCAP_MIDDLE ] );
	m_anBoneIndexHubcap[m_uRatType][ HUBCAP_BONE_ARRAY_L_REAR_INDEX   ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_HUBCAP_REAR ] );
	m_anBoneIndexHubcap[m_uRatType][ HUBCAP_BONE_ARRAY_R_REAR_INDEX   ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_HUBCAP_REAR ] );

	m_anBoneIndexWheel[m_uRatType][ WHEEL_BONE_ARRAY_L_FRONT_INDEX  ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_FRONT ] );
	m_anBoneIndexWheel[m_uRatType][ WHEEL_BONE_ARRAY_R_FRONT_INDEX  ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_FRONT ] );
	m_anBoneIndexWheel[m_uRatType][ WHEEL_BONE_ARRAY_L_MIDDLE_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_MIDDLE ] );
	m_anBoneIndexWheel[m_uRatType][ WHEEL_BONE_ARRAY_R_MIDDLE_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_MIDDLE ] );
	m_anBoneIndexWheel[m_uRatType][ WHEEL_BONE_ARRAY_L_REAR_INDEX   ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_REAR ] );
	m_anBoneIndexWheel[m_uRatType][ WHEEL_BONE_ARRAY_R_REAR_INDEX   ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_REAR ] );

	m_nBoneIdxDriverAttach[m_uRatType] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_DRIVER ] );
	m_nBoneIdxGunnerAttach[m_uRatType] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_GUNNER ] );
	m_nBoneIndexRightHatch[m_uRatType] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_HATCH ] );
	m_nBoneIndexTurretAttach[m_uRatType] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_TURRET ] );

	FASSERT( m_nBoneIdxDriverAttach[m_uRatType] != -1 );
	FASSERT( m_nBoneIdxGunnerAttach[m_uRatType] != -1 );
	FASSERT( m_nBoneIndexRightHatch[m_uRatType] != -1 );
	FASSERT( m_nBoneIndexTurretAttach[m_uRatType] != -1 );

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_FRONT ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_FRONT ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_MIDDLE ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_MIDDLE ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_REAR ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_REAR ] );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_AXLE_FRONT ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_AXLE_FRONT ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_AXLE_MIDDLE ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_AXLE_MIDDLE ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_AXLE_REAR ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_AXLE_REAR ] );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_HUBCAP_FRONT ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_HUBCAP_FRONT ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_HUBCAP_MIDDLE ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_HUBCAP_MIDDLE ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_HUBCAP_REAR ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_HUBCAP_REAR ] );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_HATCH ] );

	for( i = 0; i < NUM_WHEELS; i++ )
	{
		m_afWheelAngle[ i ] = 0.0f;
	}

	// Find tag points...
	for( i=0, m_nTagPointCount=0; i<TAG_POINT_COUNT; i++ )
	{
		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );

		if( nBoneIndex < 0 )
		{
			DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Could not locate TagPoint bone '%s'.\n", m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );
		} 
		else
		{
			m_apTagPoint_WS[m_nTagPointCount++] = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		}
	}

	if( m_nTagPointCount == 0 )
	{
		// No tag points. Use entity origin...
		m_nTagPointCount = 1;
		m_apTagPoint_WS[0] = &m_MtxToWorld.m_vPos;
	}

	// Find approx eye point...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} 
	else
	{
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_CHASSIS ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_CHASSIS ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} 
	else
	{
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	// Initialize matrix palette (this is important for attached entities)...
	for( i=0; i<m_pWorldMesh->m_pMesh->nBoneCount; i++ )
	{
		if( m_pWorldMesh->m_pMesh->pBoneArray[i].nFlags & FMESH_BONEFLAG_SKINNEDBONE )
		{
			*m_pWorldMesh->GetBoneMtxPalette()[i] = m_pWorldMesh->m_Xfm.m_MtxF;
		} 
		else
		{
			m_pWorldMesh->GetBoneMtxPalette()[i]->Mul( m_pWorldMesh->m_Xfm.m_MtxF, m_pWorldMesh->m_pMesh->pBoneArray[i].AtRestBoneToModelMtx );
		}
	}

	// set offset from mesh origin to physics object center-of-mass
	m_vCMOffset.Set( 0.0f, _CM_OFFSET_Y, _CM_OFFSET_Z );
	
	if (pBuilder->CVehicleBuilder::m_uFlags & VEHICLE_BUILDER_FLAG_NO_ENTITY_DRIVE)
		m_vCMOffset.Zero();

	// player uses action button to enter & exit vehicle
	SetActionable( TRUE );

	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		m_vDirtParticlePosition[nIndex].Set( m_MtxToWorld.m_vPos );		// set to a default value
		m_vDirtParticleVelocity[nIndex].Zero();							// set to a default value
		m_vDirtParticleDirection[nIndex].Set( CFVec3A::m_UnitAxisX );	// set to a default value
		m_hDirtParticle[nIndex] = fparticle_SpawnEmitter( m_hDirtParticleDef, (const CFVec3 *) &m_vDirtParticlePosition[nIndex],
																			  (const CFVec3 *) &m_vDirtParticleDirection[nIndex],
																			  (const CFVec3 *) &m_vDirtParticleVelocity[nIndex]); 
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			m_fDirtParticleIntensity[nIndex] = 0.0f;
			fparticle_SetDirection( m_hDirtParticle[nIndex], (const CFVec3 *) &m_vDirtParticleDirection[nIndex] );
			fparticle_SetIntensity( m_hDirtParticle[nIndex], (const f32 *) &m_fDirtParticleIntensity[nIndex] );
			fparticle_EnableEmission( m_hDirtParticle[nIndex], FALSE );
		}
	}

	if( m_hCloudParticleDef != FPARTICLE_INVALID_HANDLE )
	{
		for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
		{
			m_vCloudParticlePosition[nIndex].Set( m_MtxToWorld.m_vPos );		// set to a default value
			m_vCloudParticleVelocity[nIndex].Zero();							// set to a default value
			m_vCloudParticleDirection[nIndex].Set( CFVec3A::m_UnitAxisZ );	// set to a default value
			m_hCloudParticle[nIndex] = fparticle_SpawnEmitter( m_hCloudParticleDef, (const CFVec3 *) &m_vCloudParticlePosition[nIndex],
																		(const CFVec3 *) &m_vCloudParticleDirection[nIndex],
																		(const CFVec3 *) &m_vCloudParticleVelocity[nIndex]); 
			if( m_hCloudParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
			{
				m_fCloudParticleIntensity[nIndex] = 0.0f;
				fparticle_SetDirection( m_hCloudParticle[nIndex], (const CFVec3 *) &m_vCloudParticleDirection[nIndex] );
				fparticle_SetIntensity( m_hCloudParticle[nIndex], (const f32 *) &m_fCloudParticleIntensity[nIndex] );
				fparticle_EnableEmission( m_hCloudParticle[nIndex], FALSE );
			}
		}
	}

	// find length of axle for suspension animation code
	{
		CFVec3A vAxle;
		s32 nAxle, nHubcap;
		nAxle = m_pWorldMesh->FindBone( "L_Axle_Front" );
		nHubcap = m_pWorldMesh->FindBone( "L_Hubcap_Front" );

		vAxle.Sub( m_pWorldMesh->GetBoneMtxPalette()[nAxle]->m_vPos, m_pWorldMesh->GetBoneMtxPalette()[nHubcap]->m_vPos );
		m_fAxleLength = vAxle.Mag();
	}

	// initialize collision point data

	for( nIndex = 0; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		CFPhysicsObject::InitCollisionPoint( &m_aColPoints[nIndex] );
		m_aColPoints[nIndex].uResultFlags = 0;
		m_aColPoints[nIndex].uClientFlags = 0;
		m_aColPoints[nIndex].fTimeSinceCollision = _FRICTION_CONTACT_TIME + 1.0f;	// set so friction isn't applied until after first collision
		m_aColPoints[nIndex].fStaticHysteresis = _STATIC_TRANS_HYST;
		m_aColPoints[nIndex].fFrictionSmoothTime = _FRICTION_CONTACT_TIME;
		m_aColPoints[nIndex].pmFrictOr = &m_aRatColPoints[nIndex].mFriction;
		m_aColPoints[nIndex].fAppliedForceKineticMul = _KINETIC_FORCE_MUL;

		m_aRatColPoints[nIndex].vCurrentPos.Zero();
		m_aRatColPoints[nIndex].vProposedPos.Zero();
		m_aRatColPoints[nIndex].mFriction.Identity();
		m_aRatColPoints[nIndex].pMaterial = NULL;
		m_aRatColPoints[nIndex].uFlags |= RATCOLPOINT_FLAG_ENABLE_COLLISION;
	}

	_ResetSprings();

	for( nIndex = 0; nIndex < COLPOINT_L_REAR_WHEEL_INDEX; nIndex++ )
	{
		m_aColPoints[nIndex].vStaticFriction.Set( _FRONT_STATIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aColPoints[nIndex].vKineticFriction.Set( _FRONT_KINETIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aRatColPoints[nIndex].fRadius = _WHEEL_RADIUS;
	}
	for( nIndex = COLPOINT_L_REAR_WHEEL_INDEX; nIndex < NUM_WHEELS; nIndex++ )
	{
		m_aColPoints[nIndex].vStaticFriction.Set( _REAR_STATIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aColPoints[nIndex].vKineticFriction.Set( _REAR_KINETIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aRatColPoints[nIndex].fRadius = _WHEEL_RADIUS;
	}
	for( nIndex = NUM_WHEELS; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		m_aColPoints[nIndex].vAppliedForce.Zero();
		m_aColPoints[nIndex].vStaticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );
		m_aColPoints[nIndex].vKineticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );
		m_aRatColPoints[nIndex].fRadius = _afBodySphereRadius[nIndex-NUM_WHEELS];
	}

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _HEADLIGHT_TEXTURE_ID );
	SetSpotLightOff();

	m_hBrakeLightLayer = m_pWorldMesh->GetTexLayerHandle( _BRAKE_LIGHT_TEXTURE_ID );
	m_hReverseLightLayer = m_pWorldMesh->GetTexLayerHandle( _REVERSE_LIGHT_TEXTURE_ID );
	m_hTireLayer = m_pWorldMesh->GetTexLayerHandle( _TIRE_TEXTURE_ID );

	// Find turret pt
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_TURRET ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Could not locate turret bone '%s'.\n", m_apszBoneNameTable[ BONE_ATTACHPOINT_TURRET ] );
	} 
	else
	{
		if( (pBuilder->m_uFlags & RAT_BUILDER_FLAG_DROID_MESH) && m_pDroidRatMesh )
		{
			m_bLoadingDroidRat = TRUE;
		}
		else
		{
			m_bLoadingDroidRat = FALSE;
		}
		m_RatGun.Create(BOTSUBCLASS_SITEWEAPON_RATGUN,-1,FALSE,NULL,m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex],"Default");
		m_RatGun.SetOpenOrClosed( FALSE /*bOpen*/, 0.05f );
		m_bLoadingDroidRat = FALSE;

		CAIWeaponCtrl *pWpnCtrl = m_RatGun.AIBrain()->GetAIMover()->GetWeaponCtrlPtr(0);
		pWpnCtrl->m_fAccuracy = 1.0f;
		pWpnCtrl->m_fBurstDelay = 1.0f;		// time between bursts
		pWpnCtrl->m_fBurstDelayBonus = 1.0f;// random bonus delay 
		pWpnCtrl->m_fFireDelay = 2.0f;		// base length of burst (holding trigger down)
		pWpnCtrl->m_fFireDelayBonus = 2.0f;	// bonus length if decided to do so
		pWpnCtrl->m_uCtrlFlags |= CAIWeaponCtrl::CTRLFLAG_FIRE_DELAY_MEANS_TIMED_BURSTS;
	}

//	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) )
//	{
//		DEVPRINTF( "CVehicleRat::ClassHierarchyBuild() Failed to init Bot Part Manager.\n" );
//	}

	if( m_pPartMgr->IsCreated() )
	{
		m_pPartMgr->RepairWhenStolen( LIMB_TYPE_CRATE_RIGHT_REAR );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_CRATE_RIGHT_REAR );
	}

	m_fMountYaw_WS = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );

	SetHatchOpen( FALSE /*bOpen*/, TRUE /*bImmediately*/ );

	if( (pBuilder->m_uFlags & RAT_BUILDER_FLAG_ZOBBY_DRIVER) || (pBuilder->m_uFlags & RAT_BUILDER_FLAG_ZOBBY_GUNNER) )
	{
		FASSERT( m_pZobbyME == NULL );

		if( !m_pZobbyME )
		{
			m_pZobbyME = fnew CMeshEntity;
			if( m_pZobbyME == NULL )
			{
				DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Not enough memory to allocate the Zobby mesh entity.\n" );
				goto _ExitWithError;
			}

			if( !m_pZobbyME->Create( _ZOBBY_MESH_NAME, _ZOBBY_ANIM_NAME ) )
			{
				DEVPRINTF( "CVehicleRat::ClassHierarchyBuild(): Could not create the Zobby mesh entity.\n" );
				goto _ExitWithError;
			}

			m_pZobbyME->SetCollisionFlag( FALSE );
			m_pZobbyME->SetLineOfSightFlag( FALSE );
			m_pZobbyME->SetCullDist( 1000.0f );
			m_pZobbyME->MeshFlip_SetFlipsPerSec( 0.0f );
			m_pZobbyME->RemoveFromWorld();
		}

		if( pBuilder->m_uFlags & RAT_BUILDER_FLAG_ZOBBY_DRIVER ) 
		{
			m_uRatFlags |= RAT_FLAG_ZOBBY_DRIVING;
		}
		else if( pBuilder->m_uFlags & RAT_BUILDER_FLAG_ZOBBY_GUNNER )
		{
			m_uRatFlags |= RAT_FLAG_ZOBBY_GUNNING;
		}
	}

	if( pBuilder->m_uFlags & RAT_BUILDER_FLAG_DEATH_FLIP )
	{
		SetRatFlipOnDeath( TRUE );
	}

	if( pBuilder->m_uFlags & RAT_BUILDER_FLAG_AI_STAY_BEHIND )
	{
		m_uRatFlags |= RAT_FLAG_AI_STAY_BEHIND;
	}

	if( pBuilder->m_uFlags & RAT_BUILDER_FLAG_AI_STAY_BESIDE )
	{
		m_uRatFlags |= RAT_FLAG_AI_STAY_BESIDE;
	}

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL CVehicleRat::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	_InitPhysicsObject();

	if( !CVehicle::ClassHierarchyBuilt() )
	{
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	if( IsDriveable() )
	{
		m_GunnerCamera.Init( m_RatGun.GetCameraInfo(), &m_BotInfo_MGGunnerCamera );
	}
	else
	{
		m_GunnerCamera.Init( m_RatGun.GetCameraInfo(), &m_BotInfo_GunnerCamera );
	}
	m_GunnerCameraTrans.Init( m_RatGun.GetCameraInfo(), 1.0f /* transition length */ );

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

//-----------------------------------------------------------------------------
// play one-shot engine crank sound, after which, start the engine
void CVehicleRat::CrankEngine( void )
{
	m_eEngineState = ENGINE_STATE_CRANKING;
	m_fEngineTime = _ENGINE_START_TIME;

	PlaySound( m_BotInfo_Engine.pSoundGroupEngineStart );
}

//-----------------------------------------------------------------------------
// start the looping engine sound (no engine crank sound)
void CVehicleRat::StartEngine( void )
{
	m_fEngineTime = 0.0f;
	m_eEngineState = ENGINE_STATE_RUNNING;
}

//-----------------------------------------------------------------------------
// stop the engine sounds.  If bImmediately is TRUE, don't play engine die sound.
void CVehicleRat::StopEngine( BOOL bImmediately )
{
	if( bImmediately )
	{
		m_fEngineTime = 0.0f;
		m_eEngineState = ENGINE_STATE_STOPPED;
	}
	else
	{
		m_fEngineTime = _ENGINE_STOP_TIME;
		m_eEngineState = ENGINE_STATE_STOPPING;

		PlaySound( m_BotInfo_Engine.pSoundGroupEngineStop );
	}

	// stop engine sounds
	if( m_pEngineIdleEmitter )
	{
		m_pEngineIdleEmitter->SetVolume(0.0f);
	}

	if( m_pEngineLowEmitter )
	{
		m_pEngineLowEmitter->SetVolume(0.0f);
	}

	if( m_pEngineHighEmitter )
	{
		m_pEngineHighEmitter->SetVolume(0.0f);
	}

	if( m_pSkidEmitter )
	{
		m_pSkidEmitter->SetVolume(0.0f);
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::RunEngine( void )
{
	switch( m_eEngineState )
	{
		case ENGINE_STATE_STOPPED:
			break;

		case ENGINE_STATE_CRANKING:
			m_fEngineTime -= FLoop_fPreviousLoopSecs;
			if( m_fEngineTime <= 0.0f )
			{
				StartEngine();
			}
			break;

		case ENGINE_STATE_RUNNING:
			UpdateEngineSounds();
			break;

		case ENGINE_STATE_STOPPING:
			m_fEngineTime -= FLoop_fPreviousLoopSecs;
			if( m_fEngineTime <= 0.0f )
			{
				m_fEngineTime = 0.0f;
				m_eEngineState = ENGINE_STATE_STOPPED;
			}
			break;
	}
}


//-----------------------------------------------------------------------------
void CVehicleRat::FlipRightsideUp( void )
{
	if( m_eRatState == VEHICLERAT_STATE_UPSIDE_DOWN )
	{
		m_eRatState = VEHICLERAT_STATE_START_FLIPPING_UPRIGHT;
	}
}

//-----------------------------------------------------------------------------
BOOL CVehicleRat::ActionNearby( CEntity *pEntity )
{
	CVehicle::ActionNearby(pEntity);

	if( pEntity == NULL )
	{
		return FALSE;
	}

	if( !( pEntity->TypeBits() & ENTITY_BIT_BOT ) )
	{
		return FALSE;
	}

	CBot *pBot = (CBot*) pEntity;

	if( pBot == &m_RatGun )
	{
		if( m_eGunState ==  VEHICLERAT_GUN_STATE_OCCUPIED )
		{
			// gunner wants to exit or flip right-side-up
			FASSERT( m_pGunnerBot != NULL );
			if( m_pGunnerBot == NULL )
			{
				return FALSE;
			}

			if( m_pGunnerBot->IsPlayerBot() )
			{
				// only player gunners can exit or flip
				if( IsUpsideDown() )
				{
					FlipRightsideUp();
					return TRUE;
				}
				else if( m_eGunState == VEHICLERAT_GUN_STATE_OCCUPIED && (!level_IsRacingLevel() || IsGunnerExitable()) )
				{
					if( ExitStation( m_pGunnerBot, FALSE /*bImmediately*/ ) == STATION_STATUS_EXITING )
					{
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
			}
		}
	}
	else if( pBot == m_pDriverBot )
	{
		// Driving bot wants to exit or flip right-side-up
		if( IsUpsideDown() )
		{
			FlipRightsideUp();
			return TRUE;
		}
		else if( m_eRatState == VEHICLERAT_STATE_DRIVING && (!level_IsRacingLevel() || IsDriverExitable()) )
		{
			if( ExitStation( pBot, FALSE /*bImmediately*/ ) == STATION_STATUS_EXITING )
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	else 
	{
		// Bot pressing action button is not the driver or the gunner.
		// Now see if bot can enter either driver station or gunner station.

		if( pBot->IsPlayerBot() && IsUpsideDown() )
		{
			FlipRightsideUp();
			return TRUE;
		}

		if( m_pDriverBot == NULL )
		{
			if( m_eRatState == VEHICLERAT_STATE_DRIVER_UNOCCUPIED )
			{
				// nobody driving vehicle, see if bot can enter
				if( CanOccupyStation( pBot, STATION_DRIVER ) == STATION_STATUS_EMPTY )
				{
					if( EnterStation( pBot, STATION_DRIVER, TRUE /*bProximityCheck*/, FALSE /*bImmediately*/ ) == STATION_STATUS_ENTERING )
					{
						return TRUE;
					}
				}
			}
		}
		
		if( m_pGunnerBot == NULL )
		{
			if( m_eGunState == VEHICLERAT_GUN_STATE_UNOCCUPIED )
			{
				// nobody is gunning, see if bot can enter
				if( CanOccupyStation( pBot, STATION_GUNNER ) == STATION_STATUS_EMPTY )
				{
					if( EnterStation( pBot, STATION_GUNNER, TRUE /*bProximityCheck*/, FALSE /*bImmediately*/ ) == STATION_STATUS_ENTERING )
					{
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

CBot *CVehicleRat::IsStationObstructed( Station_e eStation )
{
	if( eStation == STATION_DRIVER )
	{
		return CVehicle::StationObstructed( this, m_nBoneIdxDriverAttach[m_uRatType], m_pBotInfo_Vehicle->fDriverStationRadius );
	}
	else if( eStation == STATION_GUNNER )
	{
		return CVehicle::StationObstructed( this, m_nBoneIdxGunnerAttach[m_uRatType], m_pBotInfo_Vehicle->fGunnerStationRadius );
	}
	else
	{
		FASSERT_NOW;
		return NULL;
	}
}


//-----------------------------------------------------------------------------
// returns TRUE if pBot is located in a position where it can enter eStation.
BOOL CVehicleRat::IsInStationEntryArea( CBot *pBot, Station_e eStation )
{
	FASSERT( pBot );
	if( pBot == NULL )
	{
		return FALSE;
	}

	if( eStation == STATION_DRIVER )
	{
		// see if the player-controlled bot is standing in the entry area for the station

		if( pBot->IsPlayerBot() )
		{
			CFVec3A vTestPoint;
			CFVec3A vTestDir;
			CFVec3A vDelta;
			CFVec3A vTemp;
			f32 fDist;

			// no driver entry if at rear half of vehicle
			vDelta.Set( pBot->MtxToWorld()->m_vPos );
			vDelta.Sub( m_MtxToWorld.m_vPos );
			if( m_MtxToWorld.m_vFront.Dot( vDelta ) < 0.0f )
			{
				return FALSE;
			}

			// see if driver is in valid entry area on right side of vehicle
			vTestDir.Set( m_MtxToWorld.m_vRight );
			vTestPoint.Set( vTestDir );
			vTestPoint.Mul( 9.0f );
			vTestPoint.Add( m_MtxToWorld.m_vPos );
			vDelta.Set( pBot->MtxToWorld()->m_vPos );
			vDelta.Sub( vTestPoint );
			fDist = vTestDir.Dot( vDelta );
			if( fDist > 0.0f && fDist < 5.0f )
			{
	//			m_PhysObj.DrawLine( &vTestPoint, &pBot->MtxToWorld()->m_vPos );
				return TRUE;
			}

			// see if driver is in valid entry area on left side of vehicle
			vTestDir.Negate();
			vTestPoint.Set( vTestDir );
			vTestPoint.Mul( 9.0f );
			vTestPoint.Add( m_MtxToWorld.m_vPos );
			vDelta.Set( pBot->MtxToWorld()->m_vPos );
			vDelta.Sub( vTestPoint );
			fDist = vTestDir.Dot( vDelta );
			if( fDist > 0.0f && fDist < 5.0f )
			{
	//			m_PhysObj.DrawLine( &vTestPoint, &pBot->MtxToWorld()->m_vPos );
				return TRUE;
			}

			// see if driver is in valid entry area on top deck of vehicle
			vTestDir.Set( m_MtxToWorld.m_vUp );
			vTestPoint.Set( vTestDir );
			vTestPoint.Mul( 5.0f );
			vTemp.Set( m_MtxToWorld.m_vFront );
			vTemp.Mul( 8.0f );
			vTestPoint.Add( vTemp );
			vTestPoint.Add( m_MtxToWorld.m_vPos );
			vDelta.Set( pBot->MtxToWorld()->m_vPos );
			vDelta.Sub( vTestPoint );
			fDist = vTestDir.Dot( vDelta );
			if( fDist > 0.0f && fDist < 5.0f )
			{
				return TRUE;
			}
		}
		else
		{
			// ai bots just want to know if they are near the actual driving position, not the entry area

			CFSphere BotCollSphere, StationCollSphere;
			CFMtx43A *pmDriverSeat;

			pmDriverSeat = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach[m_uRatType]];

			StationCollSphere.m_Pos.x = pmDriverSeat->m_vPos.x;
			StationCollSphere.m_Pos.y = pmDriverSeat->m_vPos.y;
			StationCollSphere.m_Pos.z = pmDriverSeat->m_vPos.z;
			StationCollSphere.m_fRadius = _DRIVING_STATION_COLLSPHERE_RADIUS;

			BotCollSphere.m_Pos = pBot->MtxToWorld()->m_vPos.v3;
			BotCollSphere.m_fRadius = 0.5f;//pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;

			//m_PhysObj.DrawSphere( (CFVec3A*) &StationCollSphere.m_Pos, StationCollSphere.m_fRadius );
			//m_PhysObj.DrawSphere( (CFVec3A*) &BotCollSphere.m_Pos, BotCollSphere.m_fRadius );

			if( StationCollSphere.IsIntersecting( BotCollSphere ) )
			{
				return TRUE;
			}
		}
	}
	else if( eStation == STATION_GUNNER )
	{
		// gunning station
		CFVec3A vDelta;
		vDelta.Set( pBot->MtxToWorld()->m_vPos );
		vDelta.Sub( m_MtxToWorld.m_vPos );
		if( vDelta.SafeUnitAndMag( vDelta ) > 0.0f )
		{
			if( m_MtxToWorld.m_vFront.Dot( vDelta ) < -0.6f )
			{
				return TRUE;
			}
		}

		if( IsStationObstructed( STATION_GUNNER ) == pBot )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// inquiry function to determine if it is possible for a bot to enter a vehicle
// at a particular station (driving, gunning, etc).
// if STATION_STATUS_EMPTY is returned, then bot may enter.  Other return values
// indicate why bot cannot occupy the station.
CVehicle::StationStatus_e CVehicleRat::CanOccupyStation( CBot *pBot, Station_e eStation )
{
	FASSERT( pBot );
	if( pBot == NULL )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( !( pBot->TypeBits() & ( ENTITY_BIT_BOTGLITCH | ENTITY_BIT_BOTGRUNT ) ) )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( IsDeadOrDying() )
	{
		return STATION_STATUS_UNAVAILABLE;
	}

	if( pBot->SwitchingWeapons() )
	{
		return STATION_STATUS_WEAPON_SWITCH;
	}

	if( pBot->IsPlayerBot() && (m_uVehicleFlags & VEHICLE_FLAG_NO_PLAYER_DRIVE) )
	{
		// bot is a player and vehicle is flagged no-player-drive
		return STATION_STATUS_UNAVAILABLE;
	}
	
	if( (eStation == STATION_DRIVER) && (m_uVehicleFlags & VEHICLE_FLAG_NO_ENTITY_DRIVE) )
	{
		// bot is asking to drive and vehicle is flagged no-entity-drive
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

	if( eStation == STATION_DRIVER )
	{
		if( m_pDriverBot != NULL )
		{
			return STATION_STATUS_OCCUPIED;
		}

		if( m_eRatState != VEHICLERAT_STATE_DRIVER_UNOCCUPIED )
		{
			return STATION_STATUS_UNAVAILABLE;
		}

		if( IsInStationEntryArea( pBot, eStation ) )
		{
			return STATION_STATUS_EMPTY;
		}
	}
	else if( eStation == STATION_GUNNER )
	{
		if( m_pGunnerBot != NULL )
		{
			return STATION_STATUS_OCCUPIED;
		}

		if( m_eGunState != VEHICLERAT_GUN_STATE_UNOCCUPIED )
		{
			return STATION_STATUS_UNAVAILABLE;
		}

		if( IsInStationEntryArea( pBot, eStation ) )
		{
			return STATION_STATUS_EMPTY;
		}
	}

	return STATION_STATUS_OUT_OF_RANGE;
}

//-----------------------------------------------------------------------------
// attempts to place bot into vehicle at specified station.  if STATION_STATUS_ENTERING is
// returned, then operation succeeded.  Other return values indicate why bot could
// not enter the station.
// If bProximityCheck is TRUE, bot will only enter vehicle if he's standing in the station 
// entry area.  If bProximityCheck is FALSE, then bot will enter regardless of his location
// relative to the vehicle.
CVehicle::StationStatus_e CVehicleRat::EnterStation( CBot *pBot, Station_e eStation, BOOL bProximityCheck, BOOL bImmediately )
{
	StationStatus_e eResult;

	FASSERT( pBot );
	if( pBot == NULL )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( eStation != STATION_DRIVER && eStation != STATION_GUNNER )
	{
		return STATION_STATUS_NO_STATION;
	}

	eResult = CanOccupyStation( pBot, eStation );
	if( bProximityCheck )
	{
		if( eResult != STATION_STATUS_EMPTY )
		{
			return eResult;
		}
	}
	else
	{
		if( eResult != STATION_STATUS_EMPTY && eResult != STATION_STATUS_OUT_OF_RANGE && eResult != STATION_STATUS_WEAPON_SWITCH )
		{
			return eResult;
		}
	}

	if( eStation == STATION_DRIVER )
	{
		m_pDriverBot = pBot;

		if( bImmediately )
		{
			// put bot into driver seat immediately.
			_StartDriverEnterWork( bImmediately );
//			StartEngine();
			CrankEngine();
			SetHatchOpen( TRUE /*bOpen*/, TRUE /*bImmediately*/ );
			StartTransitionToDriverVehicleCamera( pBot->m_nPossessionPlayerIndex );
			m_DriverCameraTrans.SetTransitionSnap();
		}
		else
		{
			// let state machine put driver in.
			m_eRatState = VEHICLERAT_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT;
		}

		if( m_pZobbyME && !pBot->IsPlayerBot() && (m_uRatFlags & RAT_FLAG_ZOBBY_DRIVING) )
		{
			// hide gunner mesh and display zobby instead
			pBot->DrawEnable( FALSE /*bEnable*/, TRUE /*bForce*/ );
			m_pZobbyME->DrawEnable( TRUE /*bEnable*/, TRUE /*bForce*/ );
		}

		return STATION_STATUS_ENTERING;
	}
	else if( eStation == STATION_GUNNER )
	{
		m_pGunnerBot = pBot;
		if( bImmediately )
		{
			// put bot into gunner seat immediately
			m_fGunStateTimer = 0.0f;
			pBot->ImmobilizeBot();
			pBot->m_pWorldMesh->SetCollisionFlag(FALSE);

			_StartGunnerEnterWork( bImmediately );
			StartTransitionToGunnerVehicleCamera( m_pGunnerBot->m_nPossessionPlayerIndex );
			m_GunnerCameraTrans.SetTransitionSnap();
			if( m_pDriverBot == NULL )
			{
				StartEngine();
			}
		}
		else
		{
			m_fGunStateTimer = _GUN_TRANSITION_TIME;
			m_eGunState = VEHICLERAT_GUN_STATE_START_GUNNER_TO_ENTRY_POINT;
		}

		if( m_pZobbyME && !pBot->IsPlayerBot() && (m_uRatFlags & RAT_FLAG_ZOBBY_GUNNING) )
		{
			// hide gunner mesh and display zobby instead
			pBot->DrawEnable( FALSE /*bEnable*/, TRUE /*bForce*/ );
			m_pZobbyME->DrawEnable( TRUE /*bEnable*/, TRUE /*bForce*/ );
		}

		return STATION_STATUS_ENTERING;
	}

	return STATION_STATUS_NO_STATION;
}

//-----------------------------------------------------------------------------
// note: "exit station immediately" on a gunner currently only works in the context of a checkpoint restore.
CVehicle::StationStatus_e CVehicleRat::ExitStation( CBot *pBot, BOOL bImmediately )
{
	s32 nStartingIndex = m_nPossessionPlayerIndex;
	StationStatus_e eReturnStatus = STATION_STATUS_WRONG_BOT;

	if( pBot == NULL )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( !pBot->IsCreated() )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( pBot == m_pDriverBot )
	{
		if( bImmediately )
		{
			// kick driver out of station immediately
			if( m_pDriverBot && m_pDriverBot->IsCreated() )
			{
				if( m_pDriverBot->IsPlayerBot() )
				{
					gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pDriverBot->m_nPossessionPlayerIndex, m_pDriverBot );
				}

				DriverExit( m_pDriverBot );
			}

			m_eRatState = VEHICLERAT_STATE_DRIVER_UNOCCUPIED;
			m_DriverCameraTrans.SetCameraStateInactive();  //Necessary if not calling exit station
		}
		else
		{
			// let state machine kick driver out
			m_eRatState = VEHICLERAT_STATE_START_DRIVER_EXIT;
		}

		if( m_pZobbyME && !pBot->IsPlayerBot() && (m_uRatFlags & RAT_FLAG_ZOBBY_DRIVING) )
		{
			// turn Driver mesh back on and hide zobby mesh
			pBot->DrawEnable( TRUE /*bEnable*/, TRUE /*bForce*/ );
			m_pZobbyME->DrawEnable( FALSE /*bEnable*/, TRUE /*bForce*/ );
		}

		eReturnStatus =  STATION_STATUS_EXITING;
	}
	else if( pBot == m_pGunnerBot )
	{
		if( bImmediately )
		{
			m_pGunnerBot->m_pDrivingVehicle = NULL;

			// Can't call SetSiteWeaponDriver( NULL ) myself because ratgun calls it in its 
			// checkpoint restore and then restores its m_pData->m_pDriverBot (if a driver 
			// was inside).  So, the functionality here (gunner bot exiting immediately) only 
			// works correctly if called while doing a checkpoint restore...
//			m_RatGun.SetSiteWeaponDriver( NULL );

			if( m_pGunnerBot && m_pGunnerBot->IsCreated() )
			{
				if( m_pGunnerBot->IsPlayerBot() )
				{
					gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pGunnerBot->m_nPossessionPlayerIndex, m_pGunnerBot );
				}
			}

			m_eGunState = VEHICLERAT_GUN_STATE_UNOCCUPIED;
			m_GunnerCameraTrans.SetCameraStateInactive();
			m_RatGun.SetOpenOrClosed( FALSE /*bOpen*/, 0.05f );
			m_pGunnerBot = NULL;

			if( m_pDriverBot )
			{
				m_nPossessionPlayerIndex = m_pDriverBot->m_nPossessionPlayerIndex;
			}
			else
			{
				m_nPossessionPlayerIndex = -1;
				StopEngine( bImmediately );
			}

			eReturnStatus = STATION_STATUS_EXITING;
		}
		else
		{
			if( m_eGunState != VEHICLERAT_GUN_STATE_OCCUPIED )
			{
				return STATION_STATUS_UNAVAILABLE;
			}

			if( m_RatGun.SetSiteWeaponDriver( NULL ) )
			{
				StartTransitionToGunnerBotCamera();

				m_eGunState = VEHICLERAT_GUN_STATE_EXITING;
				m_fGunStateTimer = _GUN_TRANSITION_TIME;
				m_pGunnerBot->m_pDrivingVehicle = NULL;

				m_pGunnerBot = NULL;

				// start cage opening animation
				m_RatGun.SetOpenOrClosed( TRUE /*bOpen*/, _GUN_CAGE_TIME );

				if( m_pDriverBot )
				{
					m_nPossessionPlayerIndex = m_pDriverBot->m_nPossessionPlayerIndex;
				}
				else
				{
					m_nPossessionPlayerIndex = -1;
					StopEngine( bImmediately );
				}

				eReturnStatus =  STATION_STATUS_EXITING;
			}
			else
			{
				return STATION_STATUS_UNAVAILABLE;
			}
		}

		if( m_pZobbyME && !pBot->IsPlayerBot() && (m_uRatFlags & RAT_FLAG_ZOBBY_GUNNING) )
		{
			// turn gunner mesh back on and hide zobby mesh
			pBot->DrawEnable( TRUE /*bEnable*/, TRUE /*bForce*/ );
			m_pZobbyME->DrawEnable( FALSE /*bEnable*/, TRUE /*bForce*/ );
		}

		// restore gunner's armor profile
		if( pBot->IsPlayerBot() )
		{
			if( m_pPreviousGunnerArmorProfile )
			{
				pBot->SetArmorProfile( m_pPreviousGunnerArmorProfile );
				m_pPreviousGunnerArmorProfile = NULL;
			}
		}
	}

	if( nStartingIndex >= 0 && m_nPossessionPlayerIndex < 0 )
	{
		// if a player exits vehicle which has no other players inside,
		// re-allocate emitters as 3D
		_AllocateEngineEmitters();
	}

	return eReturnStatus;
}

//-----------------------------------------------------------------------------
BOOL CVehicleRat::GetStationEntryPoint( Station_e eStation, CFVec3A *pPoint )
{
	if( eStation < 0 || eStation >= NUM_STATIONS )
	{
		return FALSE;
	}

	if( eStation ==STATION_DRIVER )
	{
		*pPoint = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach[m_uRatType]]->m_vPos;
	}
	else if( eStation == STATION_GUNNER )
	{
		*pPoint = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxGunnerAttach[m_uRatType]]->m_vPos;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
void CVehicleRat::InflictDamage( CDamageData *pDamageData )
{
	if( m_pGunnerBot && !level_IsRacingLevel() && pDamageData->m_Damager.pBot != &m_RatGun )
	{
		// Gunners take damage only on non-racing levels
		// (special driver armor profile reduces damage while in vehicle)

		// temporarily switch locale to ambient before applying to gunner, as
		// bone information would be from vehicle and invalid for gunner bot.
		CDamageForm::DamageLocale_e eSaveLocale = pDamageData->m_nDamageLocale;
		pDamageData->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;

		m_pGunnerBot->InflictDamage( pDamageData );

		// restore the locale and continue on.
		pDamageData->m_nDamageLocale = eSaveLocale;
	}

	CVehicle::InflictDamage( pDamageData );	
}

//-----------------------------------------------------------------------------
void CVehicleRat::InflictDamageResult( const CDamageResult *pDamageResult )
{
	if( pDamageResult->m_pDamageData->m_Damager.pBot == &m_RatGun )
	{
		// ratgun mustn't damage rat
		return;
	}

	if( m_pGunnerBot && level_IsRacingLevel() && !m_pDriverBot )
	{
		// vehicles take damage on racing levels
		CBot::InflictDamageResult( pDamageResult );
	}

	f32 fHealthBeforeDamage = NormHealth();

	CVehicle::InflictDamageResult( pDamageResult );

	// must be called after calling the base class, this way we know if we've been killed or not
	if( m_uRatFlags & RAT_FLAG_FLIP_RAT_ON_DEATH )
	{
		CEntity *pEntity = pDamageResult->m_pDamageData->m_Damager.pEntity;
		if( fHealthBeforeDamage > 0.0f &&
			IsDeadOrDying() )
		{
			if( (m_uRatFlags & RAT_FLAG_FLIP_RAT_PROJECTILE_ONLY) &&
				( !pEntity || !(pEntity->TypeBits() & ENTITY_BIT_PROJECTILE) ) )
			{
				return;
			}

			// remember that we should flip the rat.
			// (applying momentum here was problematic if occurring during physics collision callback)
			m_uRatFlags |= RAT_FLAG_FLIP;
			m_vFlipDir.Set( pDamageResult->m_pDamageData->m_AttackUnitDir_WS );
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	m_uBotDeathFlags &= (~BOTDEATHFLAG_PLAYDEATHANIM);
	m_uBotDeathFlags |= BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;

	CVehicle::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	DisableAllMeshCollisionsIfDeadOrDying();

	// attach Zobby to RAT or RatGun
	if( m_pZobbyME )
	{
		const CFMtx43A *pMtx;
		s32 nBoneIndex;

		if( m_uRatFlags & RAT_FLAG_ZOBBY_DRIVING )
		{
			nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_DRIVER ] );
			if( nBoneIndex >= 0 )
			{
				pMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
				m_pZobbyME->Relocate_RotXlatFromUnitMtx_WS( pMtx );
				m_pZobbyME->Attach_ToParent_WS( this, m_apszBoneNameTable[ BONE_ATTACHPOINT_DRIVER ] );
				m_pZobbyME->AddToWorld();
				m_pZobbyME->DrawEnable( FALSE /*bEnable*/, TRUE /*bForce*/ );
			}
			else
			{
				DEVPRINTF( "CVehicleRat::ClassHierarchyAddToWorld(): Could not locate rat bone '%s'.\n", m_apszBoneNameTable[ BONE_ATTACHPOINT_DRIVER ] );
			}
		}
		else if( m_uRatFlags & RAT_FLAG_ZOBBY_GUNNING )
		{
			CRatGun *pRatGun;

			pRatGun = (CRatGun*) m_RatGun.GetMyGun();

			nBoneIndex = pRatGun->m_pAliveMesh->FindBone( pRatGun->m_apszBoneNameList[ CRatGun::BONE_ATTACHPOINT_GUNNER ] );
			if( nBoneIndex >= 0 )
			{
				pMtx = pRatGun->m_pAliveMesh->GetBoneMtxPalette()[nBoneIndex];
				m_pZobbyME->Relocate_RotXlatFromUnitMtx_WS( pMtx );
				m_pZobbyME->Attach_ToParent_WS( &m_RatGun, pRatGun->m_apszBoneNameList[ CRatGun::BONE_ATTACHPOINT_GUNNER ] );
				m_pZobbyME->AddToWorld();
				m_pZobbyME->DrawEnable( FALSE /*bEnable*/, TRUE /*bForce*/ );
			}
			else
			{
				DEVPRINTF( "CVehicleRat::ClassHierarchyAddToWorld(): Could not locate rat gun bone '%s'.\n", pRatGun->m_apszBoneNameList[ CRatGun::BONE_ATTACHPOINT_GUNNER ] );
			}
		}

		// we will call Zobby's work ourselves to ensure he doesn't lag behind 
		m_pZobbyME->EnableAutoWork( FALSE );
	}

	// Attach RatGun to RAT
	s32 nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_TURRET ] );
	const CFMtx43A* pmtxTurret = &m_MtxToWorld;
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CVehicleRat::ClassHierarchyAddToWorld(): Could not locate turret bone '%s'.\n", m_apszBoneNameTable[ BONE_ATTACHPOINT_TURRET ] );
	} 
	else
	{
		pmtxTurret = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}
	m_RatGun.Relocate_RotXlatFromUnitMtx_WS( pmtxTurret );
	m_RatGun.Attach_ToParent_WithGlue_WS(this,m_apszBoneNameTable[ BONE_ATTACHPOINT_TURRET ]);
	m_RatGun.AddToWorld();
}


//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyRemoveFromWorld( void )
{
	s32 nIndex;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	StopEngine(TRUE);	// kill engine sounds immediately

	FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );// just in case we were dead, then next time we will be ok
	m_pWorldMesh->RemoveFromWorld();
	CVehicle::ClassHierarchyRemoveFromWorld();

	if( m_pDeadRatWorldMesh )
	{
		FMATH_SETBITMASK( m_pDeadRatWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		m_pDeadRatWorldMesh->RemoveFromWorld();
		m_pDeadRatWorldMesh->SetCollisionFlag( FALSE );
	}

	ExitStation( m_pDriverBot, TRUE /*bImmediately*/ );
	ExitStation( m_pGunnerBot, TRUE /*bImmediately*/ );

	if( m_pEParticleSmoke && m_pEParticleSmoke->IsCreated() )	
	{
		m_pEParticleSmoke->StopEmission();
		m_pEParticleSmoke = NULL;
	}

	for( nIndex = 0; nIndex < NUM_WHEELS; nIndex++ )
	{
		m_fDirtParticleIntensity[nIndex] = 0.0f;
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[nIndex], FALSE );
		}
	}

	for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
	{
		m_fCloudParticleIntensity[nIndex] = 0.0f;
		if( m_hCloudParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hCloudParticle[nIndex], FALSE );
		}
	}

	if( m_pZobbyME )
	{
		m_pZobbyME->EnableAutoWork( TRUE );
		m_pZobbyME->RemoveFromWorld();
	}

	// remove gunner
	if( m_pGunnerBot )
	{
		m_RatGun.SetSiteWeaponDriver( NULL );
		m_pGunnerBot = NULL;
	}
	m_RatGun.RemoveFromWorld();
	m_eGunState = VEHICLERAT_GUN_STATE_UNOCCUPIED;
	m_fGunStateTimer = 0.0f;

}



//-----------------------------------------------------------------------------
CEntityBuilder *CVehicleRat::GetLeafClassBuilder( void )
{
	return &_VehicleRatBuilder;
}

//-----------------------------------------------------------------------------
void CVehicleRat::ClassHierarchyWork()
{
	if( IsPlayerBot() )
	{
		_pDrivenPhysObj = &m_PhysObj;
	}
	else
	{
		_pDrivenPhysObj = NULL;
	}

	FASSERT( m_bSystemInitialized );
	CVehicle::ClassHierarchyWork();
	
	UpdateSpotLight();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;


	ComputeGroundNormal();

	_UpdateDriverControls();

	if( IsRealPhysicsRunning() ) 
	{
		MoveVehicle();
		_UpdateCollisionFlags();
		_UpdateRatState();
		_UpdateRatGunState();
		_UpdateRatGun();
		UpdateDriverCamera();
		UpdateGunnerCamera();
		RunEngine();
		PlayCollisionSounds();
  		if( IsDeadOrDying() )
		{
			DeathWork();
		}
		_UpdateDamageSmoke();
		_UpdateAISound();
		_UpdateSkidSound();
#if ENABLE_BOOST_FEATURE
		_UpdateBoost();
#endif
	}
	else
	{ 
		// non-driveable rats don't need all of the work above to be done
		UpdateMatrices();
		_UpdateRatGunState();
		_UpdateRatGun();
		UpdateGunnerCamera();
  		if( IsDeadOrDying() )
		{
			DeathWork();
		}
	}

	_UpdateDustCloud();
	_UpdateWaterEffects();

	// handle application of rat flip momentum
	if( m_uRatFlags & RAT_FLAG_FLIP )
	{
		m_uRatFlags &= ~RAT_FLAG_FLIP;

		CFVec3A Momentum, ApplyPt_MS;
		f32 fRandom = fmath_RandomFloat();
		Momentum.Mul( CFVec3A::m_UnitAxisY, 175000.0f + (fRandom * 100000.0f) );

		MtxToWorld()->MulDir( ApplyPt_MS, m_vFlipDir ); 
		ApplyPt_MS.y = 0.0f;
		ApplyPt_MS.Mul( (1.0f - (fRandom*0.85f)) * -15.0f );	// as we apply higher amounts of momentum, we need to apply it more toward the center of the vehicle		

		m_PhysObj.ApplyMomentum( &Momentum, &ApplyPt_MS );			
	}

	// call Zobby's work function ourselves to ensure he doesn't lag behind rat
	if( m_pZobbyME )
	{
		if( !m_pZobbyME->IsAutoWorkEnabled() )
		{
			m_pZobbyME->Work();
		}
	}
#if _ADJUST_SPHERES
	_bDrewSpheres = FALSE;
	gamepad_AxisSlider( _vAdjust.x, _vAdjust.x, 0.0f, 20000.0f, 0.0001f, 0.25f, GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X, 1 );
	gamepad_AxisSlider( _vAdjust.y, _vAdjust.y, -10.0f, 10.0f, 0.0001f, 0.25f, GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_Y, 1 );
	gamepad_AxisSlider( _vAdjust.z, _vAdjust.z, -30.0f, 30.0f, 0.0001f, 0.25f, GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_Y, 1 );
	gamepad_AxisSlider( _fAdjust, _fAdjust,     0.0f, 10.0f, 0.0001f, 0.25f, GAMEPAD_SLIDER_AXIS_RIGHT_ANALOG_X, 1 );

	if( _nGroupSize[_nGroup] > 1 )
	{
		_afBodySpherePos[_nGroupStart[_nGroup]][0] = -_vAdjust.x;
		_afBodySpherePos[_nGroupStart[_nGroup]][1] = _vAdjust.y;
		_afBodySpherePos[_nGroupStart[_nGroup]][2] = _vAdjust.z;
		_afBodySpherePos[_nGroupStart[_nGroup]+1][0] = _vAdjust.x;
		_afBodySpherePos[_nGroupStart[_nGroup]+1][1] = _vAdjust.y;
		_afBodySpherePos[_nGroupStart[_nGroup]+1][2] = _vAdjust.z;
		_afBodySphereRadius[_nGroupStart[_nGroup]] = _fAdjust;
		_afBodySphereRadius[_nGroupStart[_nGroup]+1] = _fAdjust;
		m_aRatColPoints[CVehicleRat::NUM_WHEELS+_nGroupStart[_nGroup]].fRadius = _fAdjust;
		m_aRatColPoints[CVehicleRat::NUM_WHEELS+_nGroupStart[_nGroup]+1].fRadius = _fAdjust;
	}
	else
	{
		_afBodySpherePos[_nGroupStart[_nGroup]][0] = _vAdjust.x;
		_afBodySpherePos[_nGroupStart[_nGroup]][1] = _vAdjust.y;
		_afBodySpherePos[_nGroupStart[_nGroup]][2] = _vAdjust.z;
		_afBodySphereRadius[_nGroupStart[_nGroup]] = _fAdjust;
		m_aRatColPoints[CVehicleRat::NUM_WHEELS+_nGroupStart[_nGroup]].fRadius = _fAdjust;
	}
	ftext_Printf( 0.2f, 0.60f, "~f1~C92929299~w1~al~s0.69X       Left X:%.3f", _vAdjust.x );
	ftext_Printf( 0.2f, 0.62f, "~f1~C92929299~w1~al~s0.69Y       Left Y:%.3f", _vAdjust.y );
	ftext_Printf( 0.2f, 0.64f, "~f1~C92929299~w1~al~s0.69Z       Right Y:%.3f", _vAdjust.z );
	ftext_Printf( 0.2f, 0.66f, "~f1~C92929299~w1~al~s0.69Radius  Right X:%.3f", _fAdjust );
	ftext_Printf( 0.2f, 0.68f, "~f1~C92929299~w1~al~s0.69Group: %d", _nGroup );

	if( Gamepad_aapSample[1][GAMEPAD_MAIN_ACTION]->fCurrentState > 0.1f )
	{
		if( !_bPressed )
		{
			_bPressed = TRUE;
			_nGroup++;
			if( _nGroup >= _NUM_GROUPS )
			{
				_nGroup = 0;
			}

			if( _nGroupSize[_nGroup] > 1 )
			{
				_vAdjust.x = _afBodySpherePos[_nGroupStart[_nGroup]+1][0];
				_vAdjust.y = _afBodySpherePos[_nGroupStart[_nGroup]+1][1];
				_vAdjust.z = _afBodySpherePos[_nGroupStart[_nGroup]+1][2];
				_fAdjust = _afBodySphereRadius[_nGroupStart[_nGroup]];
			}
			else
			{
				_vAdjust.x = _afBodySpherePos[_nGroupStart[_nGroup]][0];
				_vAdjust.y = _afBodySpherePos[_nGroupStart[_nGroup]][1];
				_vAdjust.z = _afBodySpherePos[_nGroupStart[_nGroup]][2];
				_fAdjust = _afBodySphereRadius[_nGroupStart[_nGroup]];
			}
		}
	}
	else
	{
		_bPressed = FALSE;
	}

#endif	// _ADJUST_SPHERES
}

//-----------------------------------------------------------------------------
void CVehicleRat::AnimateAxles( u32 uAxleIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFMtx43A mRot;
	CFMtx43A mSpring;
	CFVec3A vArm;
	const CFVec3A *pvVel;
	f32 fVel, fRot;
	f32 fManualVel;
	f32 fVelForward;

	// get vehicle velocity
	pvVel = m_PhysObj.GetVelocity();
	// find component of velocity in direction that vehicle is pointing
	fVelForward = m_MtxToWorld.m_vFront.Dot( *pvVel );

	fVel = m_PhysObj.GetVelocity()->Mag();

	// compute matrix for axle, except for steering angle
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	mRot.Identity();
	switch( uAxleIndex )
	{
		case AXLE_BONE_ARRAY_R_FRONT_INDEX:
		case AXLE_BONE_ARRAY_L_FRONT_INDEX:
		case AXLE_BONE_ARRAY_R_REAR_INDEX:
		case AXLE_BONE_ARRAY_L_REAR_INDEX:
			// compute rotation matrix for rear axle steering.
			// (turning angle scales inversely with vehicle velocity.)

			// get velocity of vehicle and clamp to maximum amount over which steering will vary.
			if ( m_bControls_Human )
			{
				if( fVelForward <= 0.0f ) {
					// allow full rear-wheel steer when going in reverse
					fVel = 0.0f;
				}

				if( m_pDriverBot->m_nPossessionPlayerIndex >= 0 )
				{
					u32 nControlIndex = Player_aPlayer[m_pDriverBot->m_nPossessionPlayerIndex].m_nControllerIndex;
					fManualVel = (1.0f - Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_FIRE_SECONDARY]->fCurrentState) * m_fMaxSpeed;
				}
				else
				{
					fManualVel = fVel;
				}


				if( fManualVel < fVel )
				{
					fVel = fManualVel;
				}
			}
			else
			{
				fVel = m_PhysObj.GetVelocity()->Mag();
			}

			FMATH_CLAMPMIN( fVel, 0.0f );
			FMATH_CLAMPMAX( fVel, m_fMaxSpeed );

			if( uAxleIndex == AXLE_BONE_ARRAY_R_FRONT_INDEX || uAxleIndex == AXLE_BONE_ARRAY_L_FRONT_INDEX )
			{
				// compute steering angle that scales inversely with vehicle velocity
				fRot = m_fSteeringPosition * _VAR_FRONT_AXLE_TURN_ANGLE;
				fRot = ( (m_fMaxSpeed - fVel) * fRot ) * m_fOOMaxSpeed;

				// add in base amount of steering angle which does not vary with velocity
				fRot += m_fSteeringPosition * _BASE_FRONT_AXLE_TURN_ANGLE;
			}
			else
			{
				// compute steering angle that scales inversely with vehicle velocity
				fRot = -m_fSteeringPosition * _VAR_REAR_AXLE_TURN_ANGLE;
				fRot = ( (m_fMaxSpeed - fVel) * fRot ) * m_fOOMaxSpeed;

				// add in base amount of steering angle which does not vary with velocity
				fRot += -m_fSteeringPosition * _BASE_REAR_AXLE_TURN_ANGLE;
			}

			// set up the rotation matrix for this axle's steering
			mRot.SetRotationY( fRot );
			break;
	}

	// rotate axle to steering angle
	rNewMtx.Mul( mRot );

	// animate axle for suspension
	vArm.x = m_fAxleLength;
	vArm.y = -m_aColPoints[uAxleIndex].Spring.fCurrentLength;
	vArm.y += _SUSPENSION_OFFSET;	// move suspension up a bit so it doesn't look so extended while vehicle is on the ground
	vArm.z = 0.0f;

	vArm.Unitize();

	mSpring.Identity();
	mSpring.m_vRight.Set( vArm );
	mSpring.m_vFront.Cross( vArm, CFVec3A::m_UnitAxisY );
	mSpring.m_vFront.Unitize();
	mSpring.m_vUp.Cross( mSpring.m_vFront, vArm );

	rNewMtx.Mul( mSpring );

	FASSERT( rNewMtx.m_vPos.x > -1000000.0f && rNewMtx.m_vPos.x < 1000000.0f );
	FASSERT( rNewMtx.m_vPos.y > -1000000.0f && rNewMtx.m_vPos.y < 1000000.0f );
	FASSERT( rNewMtx.m_vPos.z > -1000000.0f && rNewMtx.m_vPos.z < 1000000.0f );
}

//-----------------------------------------------------------------------------
void CVehicleRat::AnimateHubcaps( u32 uWheelIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFMtx43A mSpring;
	CFVec3A vArm;

	rNewMtx.Mul( rParentMtx, rBoneMtx );

	// animate hubcap for suspension
	vArm.x = m_fAxleLength;
	vArm.y = m_aColPoints[uWheelIndex].Spring.fCurrentLength;
	vArm.y -= _SUSPENSION_OFFSET;	// move suspension up a bit so it doesn't look so extended while vehicle is on the ground
	if( ( uWheelIndex & 1 ) == 0 )
	{
		vArm.y = -vArm.y;
	}
	vArm.z = 0.0f;

	vArm.Unitize();

	mSpring.Identity();
	mSpring.m_vRight.Set( vArm );
	mSpring.m_vFront.Cross( vArm, CFVec3A::m_UnitAxisY );
	mSpring.m_vFront.Unitize();
	mSpring.m_vUp.Cross( mSpring.m_vFront, vArm );

	rNewMtx.Mul( mSpring );

	m_aRatColPoints[uWheelIndex].mFriction.Set( rNewMtx );

	//	m_aRatColPoints[uWheelIndex].mFriction.m_vPos.Zero();
	// move friction application point up towards center of mass to allow higher
	// lateral friction without flipping vehicle over
	m_aRatColPoints[uWheelIndex].mFriction.m_vPos.Set( rNewMtx.m_vUp );
	m_aRatColPoints[uWheelIndex].mFriction.m_vPos.Mul( 2.0f );

	FASSERT( rNewMtx.m_vPos.x > -1000000.0f && rNewMtx.m_vPos.x < 1000000.0f );
	FASSERT( rNewMtx.m_vPos.y > -1000000.0f && rNewMtx.m_vPos.y < 1000000.0f );
	FASSERT( rNewMtx.m_vPos.z > -1000000.0f && rNewMtx.m_vPos.z < 1000000.0f );
}

//-----------------------------------------------------------------------------
void CVehicleRat::AnimateWheels( u32 uWheelIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFMtx43A mMat, mDirt;
	CFVec3A vVelAtTire;
	CFVec3A vLateralVel;
	CFVec3A vPoint;
	const CFVec3A *pPos;
	f32 fRollingVel;
	f32 fLateralVel;

	// compute the matrix of the tire, before rolling is applied.
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( !IsFinalBoneUpdate() )
	{
		// don't do rest of wheel animation work
		return;
	}

	mMat.Identity();
	mMat.SetRotationX( FMATH_DEG2RAD( _DIRT_EMISSION_ANGLE ) );

	mDirt.Set( rNewMtx );
	mDirt.Mul( mMat );

	// set position of dirt particle emitter for this wheel
	vPoint.Set( rNewMtx.m_vUp );
	vPoint.Mul( -_WHEEL_RADIUS );
	vPoint.Add( rNewMtx.m_vPos );
	m_vDirtParticlePosition[uWheelIndex].Set( vPoint );

	// set dirt particle emitter velocity to velocity at tire
	pPos = m_PhysObj.GetPosition();
	vPoint.Sub( *pPos );
	m_PhysObj.GetVelocity( &vPoint, &vVelAtTire );
	m_vDirtParticleVelocity[uWheelIndex].Set( vVelAtTire );

	// set direction of dirt particle emitter for this wheel to point opposite velocity direction
	if( vVelAtTire.MagSq() > 1.0f )
	{
		m_vDirtParticleDirection[uWheelIndex].Set( vVelAtTire );
		m_vDirtParticleDirection[uWheelIndex].Negate();
		m_vDirtParticleDirection[uWheelIndex].Unitize();
	}
	else
	{
		m_vDirtParticleDirection[uWheelIndex].Set( mDirt.m_vFront );
		m_vDirtParticleDirection[uWheelIndex].Negate();
	}

	// set intensity of dirt particle emitter to be proportional to lateral velocity at tire
	vLateralVel.Mul( mDirt.m_vRight, vVelAtTire.Dot( mDirt.m_vRight ) );
	fLateralVel = vLateralVel.Mag();
	if( fLateralVel - _MIN_DIRT_VEL > 0.0f )
	{
		m_fDirtParticleIntensity[uWheelIndex] = fmath_Div( fLateralVel, _MAX_DIRT_VEL );
		FMATH_CLAMPMIN( m_fDirtParticleIntensity[uWheelIndex], _MIN_DIRT_INTENSITY );
	}
	else
	{
		m_fDirtParticleIntensity[uWheelIndex] = 0.0f;
	}
	// modify intensity if sliding
	if( m_aColPoints[uWheelIndex].uClientFlags & CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION )
	{
		if( m_fBrake > 0.1f )
		{
			// boost intensity if sliding while braking
			m_fDirtParticleIntensity[uWheelIndex] *= _DIRT_KINETIC_INTENSITY_MUL;
		}
		else
		{
			// sliding but not braking so kick up dirt in proportion to throttle
			m_fDirtParticleIntensity[uWheelIndex] = m_fThrottle;

			// set direction to correspond to throttle setting
			m_vDirtParticleDirection[uWheelIndex].Set( mDirt.m_vFront );
			if( m_fThrottle > 0.0f )
			{
				// emission direction is opposite of throttle direction
				m_vDirtParticleDirection[uWheelIndex].Negate();

				// set velocity proportional to throttle
				m_vDirtParticleVelocity[uWheelIndex].Set( mDirt.m_vFront );
				m_vDirtParticleVelocity[uWheelIndex].Negate();
				m_vDirtParticleVelocity[uWheelIndex].Mul( m_fThrottle * _MAX_ROOSTER_VEL );
			}
		}
		FMATH_CLAMPMIN( m_fDirtParticleIntensity[uWheelIndex], _MIN_DIRT_KINETIC_INTENSITY );
	}
	FMATH_CLAMPMAX( m_fDirtParticleIntensity[uWheelIndex], 1.0f );

	// turn on emitter if velocity exceeds minimum setting
	if( m_fDirtParticleIntensity[uWheelIndex] > 0.0f && SmoothContact() &&
		m_aRatColPoints[uWheelIndex].pMaterial && m_aRatColPoints[uWheelIndex].pMaterial->HasLooseDust() &&
		( (m_aColPoints[uWheelIndex].uClientFlags & CFPhysicsObject::CLIENT_FLAG_COLLIDED) || m_aColPoints[uWheelIndex].fTimeSinceCollision < m_aColPoints[uWheelIndex].fFrictionSmoothTime ) )
	{
		if( m_hDirtParticle[uWheelIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[uWheelIndex], TRUE );
		}
	}

	// compute rotational velocity of tire in radians per second
	fRollingVel = vVelAtTire.Dot( rNewMtx.m_vFront );
	fRollingVel *= _OO_WHEEL_RADIUS;
	if( m_aColPoints[uWheelIndex].uClientFlags & CFPhysicsObject::CLIENT_FLAG_KINETIC_FRICTION )
	{
		if( m_fBrake > 0.1f && m_fThrottle < 0.1f )
		{
			// when sliding and brakes are applied, tires don't spin
			fRollingVel = 0.0f;
		}
		else
		{
			// when sliding, tires spin at rate determined by throttle
			fRollingVel = m_fMaxSpeed * m_fThrottle;
			fRollingVel *= _OO_WHEEL_RADIUS;
		}	
	}

	// compute current rolling angle of tire
	m_afWheelAngle[ uWheelIndex ] += fRollingVel * FLoop_fPreviousLoopSecs;
	while( m_afWheelAngle[ uWheelIndex ] > FMATH_DEG2RAD( 360.0f ) )
	{
		m_afWheelAngle[ uWheelIndex ] -= FMATH_DEG2RAD( 360.0f );
	}

	while( m_afWheelAngle[ uWheelIndex ] < 0.0f )
	{
		m_afWheelAngle[ uWheelIndex ] += FMATH_DEG2RAD( 360.0f );
	}

	// transform tire to its new rolling rotation
	CFMtx43A mRot;
	mRot.Identity();
	mRot.SetRotationX( m_afWheelAngle[ uWheelIndex ] );
	rNewMtx.Mul( mRot );

	FASSERT( rNewMtx.m_vPos.x > -1000000.0f && rNewMtx.m_vPos.x < 1000000.0f );
	FASSERT( rNewMtx.m_vPos.y > -1000000.0f && rNewMtx.m_vPos.y < 1000000.0f );
	FASSERT( rNewMtx.m_vPos.z > -1000000.0f && rNewMtx.m_vPos.z < 1000000.0f );
}

#define _CLOSED_HATCH_ROT	( FMATH_DEG2RAD( 175.0f ) )
#define _HATCH_ROTATE_SPEED	( 2.0f )	// units per second
//-----------------------------------------------------------------------------
void CVehicleRat::AnimateHatch( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFMtx43A mSpring;
	CFVec3A vArm;
	f32 fRightHatchRot;

	FASSERT( uBoneIndex == m_nBoneIndexRightHatch[m_uRatType] );

	rNewMtx.Mul( rParentMtx, rBoneMtx );

	switch( m_eRightHatchState )
	{
		case HATCH_STATE_OPEN:
			m_fRightHatchUnitPos = 0.0f;
			break;

		case HATCH_STATE_CLOSED:
			m_fRightHatchUnitPos = 1.0f;
			break;

		case HATCH_STATE_OPENING:
			m_fRightHatchUnitPos -= FLoop_fPreviousLoopSecs * _HATCH_ROTATE_SPEED;
			if( m_fRightHatchUnitPos <= 0.0f )
			{
				m_fRightHatchUnitPos = 0.0f;
				m_eRightHatchState = HATCH_STATE_OPEN;
				PlaySound( m_BotInfo_Rat.pSoundGroupHatchClank );
			}
			break;

		case HATCH_STATE_CLOSING:
			m_fRightHatchUnitPos += FLoop_fPreviousLoopSecs * _HATCH_ROTATE_SPEED;
			if( m_fRightHatchUnitPos >= 1.0f )
			{
				m_fRightHatchUnitPos = 1.0f;
				m_eRightHatchState = HATCH_STATE_CLOSED;
				PlaySound( m_BotInfo_Rat.pSoundGroupHatchClank );
			}
			break;
	}

	fRightHatchRot = m_fRightHatchUnitPos * _CLOSED_HATCH_ROT;

	CFMtx43A mRot;
	mRot.Identity();
	mRot.SetRotationX( fRightHatchRot );
	rNewMtx.Mul( mRot );
}


//-----------------------------------------------------------------------------
void CVehicleRat::_AnimBoneCallback( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) 
{
	CVehicleRat *pRat;

	// find which bone this callback was called for, and invoke the corresponding animation function
	pRat = (CVehicleRat *) m_pCollBot;

	if( pRat->m_pPartMgr->IsCreated() && pRat->IsFinalBoneUpdate() )
	{
		if( pRat->m_pPartMgr->AnimBoneCallbackFunctionHandler( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) )
		{
			return;
		}
	}

	if( uBoneIndex == pRat->m_nBoneIndexRightHatch[pRat->m_uRatType] )
	{
		pRat->AnimateHatch( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
		return;
	}

	for( u32 uWheelIndex = 0; uWheelIndex < CVehicleRat::NUM_WHEELS; uWheelIndex++ )
	{
		if( uBoneIndex == pRat->m_anBoneIndexWheel[pRat->m_uRatType][uWheelIndex] )
		{
			pRat->AnimateWheels( uWheelIndex, uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
		}
		else if( uBoneIndex == pRat->m_anBoneIndexAxle[pRat->m_uRatType][uWheelIndex] )
		{
			pRat->AnimateAxles( uWheelIndex, uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
		}
		else if( uBoneIndex == pRat->m_anBoneIndexHubcap[pRat->m_uRatType][uWheelIndex] )
		{
			pRat->AnimateHubcaps( uWheelIndex, uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
		}
	}
}



//-----------------------------------------------------------------------------
void CVehicleRat::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;

	if( m_pDeadRatWorldMesh )
	{
		apTrackerSkipList[nTrackerSkipListCount++] = m_pDeadRatWorldMesh;
	}

	// rat gun adds gunner bot...
	m_RatGun.AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);

	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_RatGun.m_pWorldMesh;

	if( m_pDriverBot )
	{
		m_pDriverBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pZobbyME )
	{
		m_pZobbyME->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}

	if( m_pGunnerBot )
	{
		m_pGunnerBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}


//-----------------------------------------------------------------------------
const CFVec3A *CVehicleRat::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT( IsCreated() );
	FASSERT( nTagPointIndex < m_nTagPointCount );

	return m_apTagPoint_WS[nTagPointIndex];
}

//-----------------------------------------------------------------------------
const CFVec3A *CVehicleRat::GetApproxEyePoint( void ) const
{
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}

//-----------------------------------------------------------------------------
// Call when m_Velocity_WS and m_Velocity_MS have changed.
void CVehicleRat::VelocityHasChanged( void )
{
	// Update m_VelocityXZ_WS...
	m_VelocityXZ_WS = m_Velocity_WS;
	m_VelocityXZ_WS.y = 0.0f;

	// Update m_NormVelocityXZ_WS...
	m_NormVelocityXZ_WS = m_Velocity_WS;
	m_NormVelocityXZ_WS.y = 0.0f;
	m_NormVelocityXZ_WS.Mul( m_fOOMaxSpeed );

	// Compute speeds...
	m_fSpeed_WS = m_Velocity_WS.Mag();
	m_fUnitSpeed_WS = m_fSpeed_WS * m_fOOMaxSpeed;
	m_fSpeedXZ_WS = m_Velocity_WS.MagXZ();
	
	m_fNormSpeedXZ_WS = m_fSpeedXZ_WS * m_fOOMaxSpeed;
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

//-----------------------------------------------------------------------------
void CVehicleRat::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	pApproxMuzzlePoint_WS->Set( *m_apTagPoint_WS[TAG_POINT_INDEX_CHASSIS] );
}

//-----------------------------------------------------------------------------
f32 CVehicleRat::ComputeEstimatedControlledStopTimeXZ( void )
{
	return m_fUnitSpeed_WS * 3.5f;
}

//-----------------------------------------------------------------------------
f32 CVehicleRat::ComputeEstimatedMinimumTurnRadius( f32 fVelocity )
{ 
	// formula for current turn radius is current linear velocity divided by maximum possible angular 
	// velocity.  Max angular velocity is around 1.0 radians/sec., so just return linear velocity.

	if(  m_fMaxSpeed > 0.0001f && fmath_Div( fVelocity, m_fMaxSpeed ) > _MIN_AI_MANEUVERING_THROTTLE )
	{
		return fVelocity;
	}
	else
	{
		// If stopped or below minimum AI speed, return radius reflecting minimum maneuvering speed.
		return m_fMaxSpeed * _MIN_AI_MANEUVERING_THROTTLE;
	}
}


//-----------------------------------------------------------------------------
f32 CVehicleRat::ComputeMinimumManeuveringSpeed( void )
{
	return _MIN_AI_MANEUVERING_THROTTLE * m_fMaxFlatSurfaceSpeed_WS;
}

//-----------------------------------------------------------------------------
void CVehicleRat::CheckpointSaveSelect( s32 nCheckpoint )
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

//-----------------------------------------------------------------------------
// saves state for checkpoint
BOOL CVehicleRat::CheckpointSave( void )
{
	// save parent class data
	CVehicle::CheckpointSave();

	CFCheckPoint::SaveData( (u32&) m_pDriverBot );
	CFCheckPoint::SaveData( (u32&) m_pGunnerBot );
	CFCheckPoint::SaveData( (u32&) m_pSplinePath );
	CFCheckPoint::SaveData( m_uRatFlags );

	return TRUE;
}

//-----------------------------------------------------------------------------
// loads state for checkpoint
void CVehicleRat::CheckpointRestore( void )
{
	// since bot state restore updates the anim matrices, which calls
	// AnimateAxles(), etc.  Must reset springs before bot state restore.
	_ResetSprings();

	// since AnimateAxles() is called by CBot::CheckpointRestore(), must set
	// m_pCollBot pointer.
	m_pCollBot = this;

	// load parent class data
	CVehicle::CheckpointRestore();

	CFCheckPoint::LoadData( (u32&) m_pDriverBot );
	CFCheckPoint::LoadData( (u32&) m_pGunnerBot );
	CFCheckPoint::LoadData( (u32&) m_pSplinePath );
	CFCheckPoint::LoadData( m_uRatFlags );

	_InitPhysicsObject();

	// must update matrices or first frame of rat collision will have bad vPoint's
	UpdateMatrices();

	if( m_pPartMgr->IsCreated() )
	{
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_CRATE_RIGHT_REAR );
	}
}

void CVehicleRat::CheckpointRestorePostamble( void )
{
	CVehicle::CheckpointRestorePostamble();

	if( m_pDriverBot )
	{
		CBot *pBot;
		pBot = m_pDriverBot;
		m_pDriverBot = NULL;
		EnterStation( pBot, STATION_DRIVER, FALSE /*bProximityCheck*/, TRUE /*bImmediately*/ );
	}
	else
	{
		SetHatchOpen( FALSE /*bOpen*/, TRUE /*bImmediately*/ );
	}

	if( m_pGunnerBot )
	{
		CBot *pBot;
		pBot = m_pGunnerBot;
		m_pGunnerBot = NULL;
		EnterStation( pBot, STATION_GUNNER, FALSE /*bProximityCheck*/, TRUE /*bImmediately*/ );
	}

	// lets non-driveable vehicles drop to ground on restore
	m_uVehicleFlags &= ~VEHICLE_FLAG_SMOOTH_CONTACT;
}

void CVehicleRat::RecycleVehicle( CFMtx43A &rNewLocation,
								  CESpline *pSpline,
								  u32 nEnemyGUID,
								  f32 fUnitMaxSpeed )
{
	// reset the speed parameters
	SetMaxUnitSpeed( fUnitMaxSpeed, FALSE );

	// setup the collision flags properly
	if( m_pRatWorldMesh && !m_pRatWorldMesh->IsCollisionFlagSet() )
	{
        m_pRatWorldMesh->SetCollisionFlag( TRUE );
	}
	if( m_pDeadRatWorldMesh && m_pDeadRatWorldMesh->IsCollisionFlagSet() )
	{
		m_pDeadRatWorldMesh->SetCollisionFlag( FALSE );
	}
	m_RatGun.m_pWorldMesh->SetCollisionFlag( TRUE );
	
	// gun always defaults to empty state
	m_pGunnerBot = NULL;
	m_eGunState = VEHICLERAT_GUN_STATE_UNOCCUPIED;
	m_RatGun.SetOpenOrClosed( FALSE /*bOpen*/, 0.05f );

	u32 i;
	for( i=0; i < MAX_COL_POINTS; i++ )
	{
		CFPhysicsObject::InitCollisionPoint( &m_aColPoints[i] );
		m_aColPoints[i].uResultFlags = 0;
		m_aColPoints[i].uClientFlags = 0;
		m_aColPoints[i].fTimeSinceCollision = _FRICTION_CONTACT_TIME + 1.0f;	// set so friction isn't applied until after first collision
		m_aColPoints[i].fStaticHysteresis = _STATIC_TRANS_HYST;
		m_aColPoints[i].fFrictionSmoothTime = _FRICTION_CONTACT_TIME;
		m_aColPoints[i].pmFrictOr = &m_aRatColPoints[i].mFriction;
		m_aColPoints[i].fAppliedForceKineticMul = _KINETIC_FORCE_MUL;

		m_aRatColPoints[i].vCurrentPos.Zero();
		m_aRatColPoints[i].vProposedPos.Zero();
		m_aRatColPoints[i].mFriction.Identity();
		m_aRatColPoints[i].pMaterial = NULL;
		m_aRatColPoints[i].uFlags |= RATCOLPOINT_FLAG_ENABLE_COLLISION;
	}

	// since bot state restore updates the anim matrices, which calls
	// AnimateAxles(), etc.  Must reset springs before bot state restore.
	_ResetSprings();

	for( i = 0; i < COLPOINT_L_REAR_WHEEL_INDEX; i++ )
	{
		m_aColPoints[i].vStaticFriction.Set( _FRONT_STATIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aColPoints[i].vKineticFriction.Set( _FRONT_KINETIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aRatColPoints[i].fRadius = _WHEEL_RADIUS;
	}
	for( i = COLPOINT_L_REAR_WHEEL_INDEX; i < NUM_WHEELS; i++  )
	{
		m_aColPoints[i].vStaticFriction.Set( _REAR_STATIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aColPoints[i].vKineticFriction.Set( _REAR_KINETIC_FRICTION, m_BotInfo_VehiclePhysics.fBodyFriction, _TIRE_ROLLING_FRICTION );
		m_aRatColPoints[i].fRadius = _WHEEL_RADIUS;
	}
	for( i = NUM_WHEELS; i < MAX_COL_POINTS; i++ )
	{
		m_aColPoints[i].vAppliedForce.Zero();
		m_aColPoints[i].vStaticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );
		m_aColPoints[i].vKineticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );
		m_aRatColPoints[i].fRadius = _afBodySphereRadius[i-NUM_WHEELS];
	}

	// since AnimateAxles() is called by CBot::CheckpointRestore(), must set
	// m_pCollBot pointer.
	m_pCollBot = this;

	// move the vehicle to its new world location
	Relocate_RotXlatFromUnitMtx_WS( &rNewLocation, FALSE );	
	AddToWorld();
	SetUnitHealth( 1.0f );	
	DrawEnable( TRUE, TRUE );

	// assign the new spline and tell the AI to attack
	SetSplinePath( pSpline );
	ai_AssignGoal_Attack( AIBrain(), nEnemyGUID, 0 );

	for( i = 0; i < NUM_WHEELS; i++ )
	{
		m_afWheelAngle[ i ] = 0.0f;
	}
	m_fSteeringPosition = 0.0f;
	m_fRawSteeringPosition = 0.0f;
	m_qGroundNormal.Identity();
	m_fFlipOverTime = 0.0f;
	m_fTimeSinceLastContact = 0.0f;
	m_fTimeSinceLastCollide = 0.0f;
	m_fTimeSinceLastHitGround = 0.0f;

	_InitPhysicsObject();

	// must update matrices or first frame of rat collision will have bad vPoint's
	AtRestMatrixPalette();
	UpdateMatrices();

	if( m_pPartMgr->IsCreated() )
	{
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_CRATE_RIGHT_REAR );
	}

	m_pDriverBot = NULL;
	m_PhysObj.SetGravity( _GROUND_GRAVITY );
	SnapCameraTransition();
	StartEngine();
	
	m_eRatState = VEHICLERAT_STATE_DRIVER_UNOCCUPIED;

	// lets non-driveable vehicles drop to ground on restore
	m_uVehicleFlags &= ~VEHICLE_FLAG_SMOOTH_CONTACT;
}

BOOL CVehicleRat::IsRealPhysicsRunning( void )
{
	BOOL bAllWork = FALSE;

	if( IsDriveable() == TRUE ||
		!SmoothContact() ||
		m_PhysObj.GetClampingCoefficient() > 0.0f ||
		( IsDeadOrDying() && m_nDeathEvents < _NUM_DEATH_EVENTS ) ) 
	{
		bAllWork = TRUE;
	}
	if( IsDeadOrDying() && m_nDeathEvents >= _NUM_DEATH_EVENTS )
	{
		bAllWork = FALSE;
	}

	return bAllWork;
}

void CVehicleRat::DisableAllMeshCollisionsIfDeadOrDying( void )
{
	m_uRatFlags |= RAT_FLAG_DISABLE_COLLSION_ON_DEAD_RATS;
}

void CVehicleRat::DisableDriverKillingOnDeath( void )
{
	m_uRatFlags |= RAT_FLAG_DONT_KILL_DRIVER_ON_DIE;
}

void CVehicleRat::SetRatFlipOnDeath( BOOL bEnable, BOOL bProjectileOnly )
{
	if( bEnable )
	{
		m_uRatFlags |= RAT_FLAG_FLIP_RAT_ON_DEATH;
	}
	else
	{
		m_uRatFlags &= (~RAT_FLAG_FLIP_RAT_ON_DEATH);
	}

	if( bProjectileOnly )
	{
		m_uRatFlags |= RAT_FLAG_FLIP_RAT_PROJECTILE_ONLY;
	}
	else
	{
		m_uRatFlags &= ~RAT_FLAG_FLIP_RAT_PROJECTILE_ONLY;
	}
}

void CVehicleRat::DestroyDustAndCloudParticles( void ) {
	s32 nIndex;

	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_KillEmitter( m_hDirtParticle[nIndex] );
			m_hDirtParticle[nIndex] = FPARTICLE_INVALID_HANDLE;
		}
	}

	for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
	{
		if( m_hCloudParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_KillEmitter( m_hCloudParticle[nIndex] );
			m_hCloudParticle[nIndex] = FPARTICLE_INVALID_HANDLE;
		}
	}
}

void CVehicleRat::OverWriteDeathSmoke( FParticle_DefHandle_t hNewSmokeHandle )
{
	m_BotInfo_Vehicle.hSmokeParticle = hNewSmokeHandle;
}

void CVehicleRat::SetRatTargetable( BOOL bEnable ) {
	SetTargetable( bEnable );
	m_RatGun.SetTargetable( bEnable );
}
									
