//////////////////////////////////////////////////////////////////////////////////////
// vehiclesentinel.cpp -
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
#include "fworld_coll.h"
#include "FCheckPoint.h"
#include "vehiclesentinel.h"
#include "meshtypes.h"
#include "gamepad.h"
#include "gamecam.h"
#include "AI/AIEnviro.h"
#include "fresload.h"
#include "player.h"
#include "explosion.h"
#include "potmark.h"
#include "botpart.h"
#include "muzzleflash.h"
#include "weapon.h"
#include "MultiplayerMgr.h"
#include "eboomer.h"

static cchar _apszVehicleMeshFilename[] = "GRMCTANK_00";


#define _BOTINFO_FILENAME			( "b_vsentinel" )
#define _DIRT_PARTICLE_DEF_NAME		( "dirtyjr01" )
#define _CLOUD_PARTICLE_DEF_NAME	( "rat_dust01" )
#define _SMOKE_PARTICLE_DEF_NAME	( "PWMPMuzzle1" )
#define _BOTPART_FILENAME			( "bp_sentinel" )
#define _BULLET_DECAL				( "GenericBullet" )

// engine sfx
#define _SENTINEL_SOUND_BANK			( "Sentinel" )
#define _RAT_SOUND_BANK					( "Rat" )
#define	_ENGINE_START_SFX_TAG_NAME		( "RatStart" )
#define	_ENGINE_STOP_SFX_TAG_NAME		( "RatOff" )
#define	_ENGINE_LOW_SFX_TAG_NAME		( "RatIdleLow" )
#define	_ENGINE_HIGH_SFX_TAG_NAME		( "RatIdleHigh" )
#define	_SKID_SFX_TAG_NAME				( "RatSkid" )
#define _ENGINE_SFX_RADIUS				( 100.0f )
#define _ENGINE_SFX_VOLUME				( 0.6f )
#define _ENGINE_START_TIME				( 0.7f )
#define _ENGINE_STOP_TIME				( 1.0f )

// run engine 
#define _ENGINE_MIN_PITCH			( 0.7f )
#define _ENGINE_PITCH_RANGE			( 1.6f )
#define _ENGINE_AIR_SPEED_MUL		( 1.3f )
#define _ENGINE_MAX_PITCH			( _ENGINE_MIN_PITCH + (_ENGINE_PITCH_RANGE * 1.2f ) )
#define _ENGINE_REV_RATE			( 3.5f )
#define _BRAKE_SKID_SFX_START		( 0.5f )
#define _BRAKE_SKID_SFX_RANGE		( 1.0f - _BRAKE_SKID_SFX_START )
#define _OO_BRAKE_SKID_SFX_RANGE	( 1.0f / _BRAKE_SKID_SFX_START )

#define _HEADLIGHT_ID				( 1 )
#define _RIGHT_TREAD_TEXTURE_ID		( 11 )
#define _LEFT_TREAD_TEXTURE_ID		( 12 )

#define _ANTENNA_SEGMENTS			( 5 )
#define _ANTENNA_FREQUENCY			( 60.0f )
#define _ANTENNA_DAMPING			( 3.0f )
#define _ANTENNA_AMPLITUDE			( 20.0f )

#define _ENTITY_BITS_TO_SKIP_COLLISION (ENTITY_BIT_WEAPON)


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleSentinelBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CVehicleSentinelBuilder _VehicleSentinelBuilder;

void CVehicleSentinelBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) 
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CVehicleBuilder, ENTITY_BIT_VEHICLESENTINEL, pszEntityType );

	// Override defaults set by our parent classes...
	m_nEC_HealthContainerCount = CVehicleSentinel::m_BotInfo_Gen.nBatteryCount;
	m_pszEC_ArmorProfile = CVehicleSentinel::m_BotInfo_Gen.pszArmorProfile;
	m_pszEC_DamageProfile = CVehicleSentinel::m_BotInfo_Vehicle.pszDamageProfile;
}


BOOL CVehicleSentinelBuilder::PostInterpretFixup( void ) 
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


BOOL CVehicleSentinelBuilder::InterpretTable( void ) 
{
	return CVehicleBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleSentinel
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


#define _MIN_TIME_BETWEEN_COLLIDE_FLAG_SET	( 0.5f )
#define _MIN_CLOUD_SPEED			( 20.0f )	// minimum speed at which dust cloud particle emitter activates

// weapons
#define _CANNON_RELOAD_TIME			( 2.0f )
#define _CANNON_UP_FORCE			( 50000.0f )
#define _CANNON_BACK_FORCE			( 100000.0f )
#define _CANNON_FORCE_TIME			( 0.2f )
#define _CANNON_FORCE_ARM			( 8.0f )
#define _CANNON_MUZZLE_LIGHT_TIME	( 0.18f )
#define _CANNON_MUZZLE_FLASH_TIME	( 0.1f )
#define _CANNON_MUZZLE_LIGHT_RADIUS	( 50.0f )
#define _CANNON_MUZZLE_SMOKE_TIME	( 0.5f )
#define _CANNON_MAX_RANGE			( 3000.0f )
#define _CANNON_ROUND_SPEED			( 5000.0f )

//#define _CANNON_SFX_TAG_NAME		( "SRMPfire1" )Fire Scatt Base
#define _CANNON_SFX_TAG_NAME		( "Fire Scatt Base" )
#define _CANNON_SFX_RADIUS			( _CANNON_MAX_RANGE * 0.5f )
#define _CANNON_SFX_VOLUME			( 1.0f )
#define _MACHINE_GUN_RELOAD_TIME	( 1.0f / 30.0f )
#define _MACHINE_GUN_JITTER_FACTOR	( 0.5f )
#define _MACHINE_GUN_MAX_RANGE		( 1000.0f )
#define	_MACHINE_GUN_SFX_TAG_NAME	( "SPEW Fire" )
#define _MACHINE_GUN_SFX_RADIUS		( _MACHINE_GUN_MAX_RANGE / 0.5f )
#define _MACHINE_GUN_SFX_VOLUME		( 0.60f )


// physics & collision
#define _DRAW_SPHERES				( 0 )		// set to 1 to draw wireframe collision spheres
#define _GRAVITY					( -128.0f )

#define _REAR_WHEEL_COLL_SPHERE_RADIUS	( 1.4f )
#define _TREAD_THICKNESS			( 1.1f )
#define _SMOOTHED_CONTACT_TIME		( 0.3f )	// minimum time that vehicle must not be touching ground before SmoothContact() is FALSE
#define _FRICTION_CONTACT_TIME		( 0.1f )	// time within which to apply friction after contact with ground is broken
#define _OO_NUM_WHEELS				( 1.0f / (f32) CVehicleSentinel::NUM_WHEELS )
#define _TREAD_ROLLING_FRICTION		( 0.2f )
#define _MIN_KINETIC_TRANS			( 0.3f )	// minimum force at which transition from static to kinetic friction (sliding) occurs
#define _VAR_KINETIC_TRANS			( 0.2f )	// varying amount of force added in proportion to speed (gets stickier at higher speeds)
#define _KINETIC_TRANS_BALANCE		( 0.0f )	// variance of transition from front wheels to back wheels
#define _STATIC_TRANS_HYST			( 0.7f )	// fraction of force which must be reduced to before static friction is restored
#define _KINETIC_FORCE_MUL			( 0.2f )	// multiplier for applied forces when under kinetic friction
#define _EXIT_VELOCITY_SQ			( 2500.0f )	// maximum squared velocity at which driver can exit vehicle
#define _MIN_SQUISH_VEL_SQ			( 50.0f )	// minimum vehicle velocity before a bot can be squished by vehicle

// camera
#define _CAM_ELEVATION_OFFSET_ANGLE	( FMATH_DEG2RAD( 26.0f ) )
#define _CAMERA_TRANSITION_TIME		( 0.5f )

// movement
#define _MAX_REVERSE_VEL_MUL		( 1.0f )
#define _FORWARD_FORCE				( 150000.0f )
#define _BRAKING_FORCE				( 110000.0f )
#define _SIDE_FORCE					( 2000.0f )
#define _ROT_FORCE					( 30000.0f )
#define _ROT_POINT					( 10.0f )
#define _JUMP_FORCE					( 500000.0f )
#define _ROT_SPEED					( FMATH_DEG2RAD( 90.0f ) )
#define _FLIP_FORCE					( 50000.0f )
#define _FLIP_VERTICAL_FORCE		( 350000.0f )
#define _FLIP_MOMENT_ARM			( 10.0f )
#define _FLIP_TIME					( 1.7f )
#define _MAX_FLIP_VEL_SQ			( 256.0f )
#define _RIGHTING_FORCE				( 80000.0f )
#define _RIGHTING_ARM				( 5.0f )
#define _BRAKE_BALANCE				( -1.0f )	// 0.0 = neutral, 1.0 = max rear, -1.0 = max front
#define _TURRET_ROTATION_RATE		( FMATH_DEG2RAD( 75.0f ) )	// rate of turret turning, in radians per second.
#define _CANNON_ELEVATION_RATE		( FMATH_DEG2RAD( 20.0f ) )	// rate of cannon elevating, in radians per second.
#define _CANNON_MAX_UP_ELEVATION	( FMATH_DEG2RAD( 37.0f ) )
#define _CANNON_MAX_DOWN_ELEVATION	( FMATH_DEG2RAD( 12.0f ) )
#define _CANNON_MAX_VIS_DOWN_ELEV	( FMATH_DEG2RAD( 7.0f ) )
#define _STEERING_ARM				( 15.0f )
#define _STEERING_FORCE				( 120000.0f )
#define _TURRET_RETURN_RATE			( FMATH_DEG2RAD( 90.0f ) )	// rate at which turret should return to face forward after driver exits. (in radians-per-second)

#define _RETICLE_NORM_X				( 0.0f )
#define _RETICLE_NORM_Y				( 0.3f )

#define _CLOUD_INWARD_OFFSET		( 4.0f )

#define _STATION_COLLSPHERE_RADIUS	( 2.5f )	// radius of occupation-determining collision sphere around driving station

#define _MIN_GROUND_LINEAR_DAMPING		( 0.3f )	// minimum dynamic damping
#define _MIN_GROUND_ANGULAR_DAMPING		( 150.0f )	
#define _GROUND_LINEAR_DAMPING			( 70.0f )	// dyamic linear damping of physics object while on the ground
#define _AIR_LINEAR_DAMPING				( 0.1f )	// linear damping of physics object while airborne
#define _GROUND_ANGULAR_DAMPING			( 3000.0f )	// dynamic angular damping of physics object while on the ground
#define _AIR_ANGULAR_DAMPING			( 50.0f )	// angular damping of physics object while airborne
#define _DYNAMIC_DAMPING_VELOCITY		( 5.0f )
#define _DYNAMIC_DAMPING_ANGULAR_VEL	( 5.0f )
#define _DYNAMIC_DAMPING_FORCE			( 10000.0f )

static f32 _fStaticFriction = 3.0f;
static f32 _fKineticFriction = 1.0f;

BOOL CVehicleSentinel::m_bSystemInitialized = FALSE;
CBotAnimStackDef CVehicleSentinel::m_AnimStackDef;
CBot *CVehicleSentinel::m_pCurrentCollider;
CFSphere CVehicleSentinel::m_SentinelColSphere;
FParticle_DefHandle_t CVehicleSentinel::m_hDirtParticleDef;
FParticle_DefHandle_t CVehicleSentinel::m_hCloudParticleDef;
FParticle_DefHandle_t CVehicleSentinel::m_hSmokeParticleDef;
CBot::BotInfo_Gen_t CVehicleSentinel::m_BotInfo_Gen;
CVehicle::BotInfo_Vehicle_t CVehicleSentinel::m_BotInfo_Vehicle;
CVehicle::BotInfo_Engine_t CVehicleSentinel::m_BotInfo_Engine;
CVehicleSentinel::BotInfo_VehiclePhysics_t CVehicleSentinel::m_BotInfo_VehiclePhysics;
CVehicleCamera::BotInfo_VehicleCamera_t CVehicleSentinel::m_BotInfo_DriverCamera;
CVehicleCamera::BotInfo_VehicleCamera_t CVehicleSentinel::m_BotInfo_MPDriverCamera;
CFCollData CVehicleSentinel::m_SentinelColData;
u32 CVehicleSentinel::m_anBoneIndexWheel[ CVehicleSentinel::NUM_ALL_WHEELS ]; // bone indices of wheels
u32 CVehicleSentinel::m_nBoneIdxDriverAttach;
u32 CVehicleSentinel::m_nBoneIndexTurret;									// bone index of turret
u32 CVehicleSentinel::m_nBoneIndexCannonBase;								// bone index of cannon base
u32 CVehicleSentinel::m_nBoneIndexCannon;									// bone index of cannon barrel
u32 CVehicleSentinel::m_nBoneIndexSpotLight;								// bone index of spotlight
u32 CVehicleSentinel::m_nBoneIndexPrimaryFire;								// bone index of cannon fire point
u32 CVehicleSentinel::m_nBoneIndexSecondaryFire;							// bone index of machine gun fire point
_TracerDef_s CVehicleSentinel::m_CannonTracerDef;
CFTexInst CVehicleSentinel::m_CannonTracerTexInst;
FDecalDefHandle_t CVehicleSentinel::m_hBulletDecal = FDECALDEF_INVALID_HANDLE;
FSndFx_FxHandle_t CVehicleSentinel::m_hCannonSFX;
FSndFx_FxHandle_t CVehicleSentinel::m_hMachineGunSFX;
CBotPartPool *CVehicleSentinel::m_pPartPool;
FExplosion_GroupHandle_t CVehicleSentinel::m_hShellExplosion;
FMesh_t	*CVehicleSentinel::m_pSentinelMesh;
u32 CVehicleSentinel::m_nBotClassClientCount = 0;
f32 CVehicleSentinel::m_afStationEntryPoints[NUM_STATIONS][3] =
{	// array of local space vectors containing entry points to vehicle stations
	{12.0f, 0.0f, 10.0f},	// STATION_DRIVER
	{0.0f, 0.0f, 0.0f},		// STATION_GUNNER (not used)
};
FSndFx_FxHandle_t CVehicleSentinel::m_hEngineStartSFX;
FSndFx_FxHandle_t CVehicleSentinel::m_hEngineStopSFX;
FSndFx_FxHandle_t CVehicleSentinel::m_hSkidSFX;


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

// debugging use only
extern CFPhysicsObject *_pDrivenPhysObj;

// used by collision callback
#define _MAX_BOTS_TO_SQUISH		( 3 )
static s32 _nBotsToSquish;
static CBot *_apBotsToSquish[_MAX_BOTS_TO_SQUISH];

static void _SentinelCollisionDetect( const CFPhysicsObject::PhysicsState_t *pState );

static f32 _afBodySpherePos[CVehicleSentinel::MAX_COL_POINTS - CVehicleSentinel::NUM_WHEELS][3] = 
{
	{ -3.68f, 6.67f, 6.63f },	// left front bumper
	{  3.68f, 6.67f, 6.63f },	// right front bumper

	{  0.0f, 10.9f, -4.0f },	// turret

	{ -4.11f, 5.55f, -5.1f },	// left rear bumper
	{  4.11f, 5.55f, -5.1f },	// right rear bumper

	{ 0.0f, 5.93f, 9.46f },	// front center bumper

	{ 0.0f, 5.2f, -8.43f },	// rear center bumper
};

static f32 _afBodySphereRadius[CVehicleSentinel::MAX_COL_POINTS - CVehicleSentinel::NUM_WHEELS] = 
{
	4.0f,	// left front bumper
	4.0f,	// right front bumper

	4.8f,	// Turret

	3.6f,	// left rear bumper
	3.6f,	// right rear bumper

	4.8f,	// front center bumper

	4.0f,	// rear center bumper
};

static f32 _afWheelRadius[ CVehicleSentinel::NUM_ALL_WHEELS ] =
{
//	1.73f, 1.73f,	// A
	1.90f, 1.90f,	// A
	2.52f, 2.52f,	// C
	0.53f, 0.53f,	// E
	3.2f, 3.2f,		// B
	2.03f, 2.03f,	// D
};

static f32 _afOOWheelRadius[ CVehicleSentinel::NUM_ALL_WHEELS ] =
{
//	0.578f, 0.578f,	// A
	0.526f, 0.526f,	// A
	0.397f, 0.397f,	// C
	 1.89f,  1.89f,	// E
	0.313f, 0.313f,	// B
	0.493f, 0.493f,	// D
};


//-----------------------------------------------------------------------------
// private functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CVehicleSentinel::_InitPhysicsObject( void )
{
	m_PhysObj.Init();
	m_MtxToWorld.m_vPos.y += 3.0f;
	m_PhysObj.SetPosition( &m_MtxToWorld.m_vPos );
	m_PhysObj.SetOrientation( &m_MtxToWorld );
	m_PhysObj.SetCollisionDetectionCallback( _SentinelCollisionDetect );
	m_PhysObj.SetElasticity( 0.0f );
	m_PhysObj.SetOtherObjElasticity( 0.0f );
	m_PhysObj.SetMass( m_BotInfo_VehiclePhysics.fMass );
	m_PhysObj.SetInertialSize( 8.0f, 8.0f, 14.4f );
	m_PhysObj.SetGravity( _GRAVITY );
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
// callback for fcoll collision.
static u32 _IntersectingTrackerCallback( CFWorldTracker *pTracker )
{
	// don't collide with self
	if( (void*) CVehicleSentinel::m_pCurrentCollider == (void*) pTracker->m_pUser )
	{
		return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
	}

	return FCOLL_CHECK_CB_ALL_IMPACTS;
}

//-----------------------------------------------------------------------------
// collide a sphere and report results to physics object.
static BOOL _CollideSphere( CVehicleSentinel *pSentinel, const CFPhysicsObject::PhysicsState_t *pState, s32 nColIndex )
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
	s32 nBots;
	BOOL bSkipCol = FALSE;

	FColl_nImpactCount = 0;

//	if( pSentinel->m_aSentinelColPoints[nColIndex].bEnableCollision )
	{
		pvCurrentPos = &pSentinel->m_aSentinelColPoints[nColIndex].vCurrentPos;
		pvProposedPos = &pSentinel->m_aSentinelColPoints[nColIndex].vProposedPos;

		vMovement.Set( *pvProposedPos );
		vMovement.Sub( *pvCurrentPos );

		pSentinel->m_SentinelColData.pMovement = &vMovement;

		pSentinel->m_SentinelColSphere.m_fRadius = pSentinel->m_aSentinelColPoints[nColIndex].fRadius;
		pSentinel->m_SentinelColSphere.m_Pos.Set( pvCurrentPos->x, pvCurrentPos->y, pvCurrentPos->z );

#if _ADJUST_SPHERES
		if( !_bDrewSpheres )
#endif
#if _DRAW_SPHERES
		{
			CFVec3A vStartColor, vEndColor;
			vStartColor.Set( 0.7f, 0.7f, 0.7f );
			vEndColor.Set( 1.0f, 1.0f, 1.0f );
			//CFCapsule capsule;
			//capsule.m_fRadius = pSentinel->m_aSentinelColPoints[nColIndex].fRadius;
			//capsule.m_vPoint1.Set( *pvCurrentPos );
			//capsule.m_vPoint2.Set( *pvProposedPos );
			//fdraw_DevCapsule( &capsule );

#if _ADJUST_SPHERES
			if( nColIndex >= CVehicleSentinel::NUM_WHEELS + _nGroupStart[_nGroup] && nColIndex < CVehicleSentinel::NUM_WHEELS + _nGroupStart[_nGroup] + _nGroupSize[_nGroup] )
			{
				vStartColor.Set( 0.7f, 0.7f, 0.0f );

			}
			else
			{
				vStartColor.Set( 0.6f, 0.6f, 0.6f );
			}
#endif
			pSentinel->m_PhysObj.DrawSphere( pvCurrentPos, pSentinel->m_aSentinelColPoints[nColIndex].fRadius, &vStartColor );
			//		pSentinel->m_PhysObj.DrawSphere( pvProposedPos, pSentinel->m_aSentinelColPoints[nColIndex].fRadius, &vEndColor );
			//	//	pSentinel->m_PhysObj.DrawLine( pvCurrentPos, pvProposedPos, &vEndColor );
		}
#endif


		pSentinel->m_aColPoints[nColIndex].pOtherObj = NULL;

		// perform collision detection
		fcoll_Clear();
//PROTRACK_BEGINBLOCK( "Sentinel Collision" );
		fcoll_Check( &pSentinel->m_SentinelColData, &pSentinel->m_SentinelColSphere );
//PROTRACK_ENDBLOCK();
	}


	// fill in collision data for physics object
	if( FColl_nImpactCount > 0 )
	{
		fcoll_SortDynamic( TRUE );

		CFVerlet *pLastVerlet = NULL;

		// step through all impacts, looking for bots
		for( uIndex = 0; uIndex < FColl_nImpactCount; uIndex++ )
		{
			// see if we hit a tracker
			//pImpact = &7FColl_aImpactBuf[uIndex];
			pImpact = FColl_apSortedImpactBuf[uIndex];
			CFWorldTracker *pHitTracker = (CFWorldTracker *)pImpact->pTag;
			if( pHitTracker ) 
			{
				// find out if tracker is a bot
				if( pHitTracker->m_nUser == MESHTYPES_ENTITY ) 
				{
					CEntity *pEntity = (CEntity *)pHitTracker->m_pUser;
					CFVerlet *pVerlet;
					pVerlet = pEntity->GetVerlet();
					if( pVerlet && pVerlet != pLastVerlet )
					{
						if( pImpact->UnitFaceNormal.Dot( CFVec3A::m_UnitAxisY ) > 0.707f )
						{
							// consider this type of collision as resting on verlet object
							vForce.Set( 0.0f, -pSentinel->m_PhysObj.GetMass(), 0.0f );
						}
						else
						{
							// colliding with object
							vForce.Set( *pSentinel->m_PhysObj.GetVelocity() );
							vForce.Mul( pSentinel->m_PhysObj.GetMass() );
							bSkipCol = TRUE;
						}
						pVerlet->ApplyForce( &pImpact->ImpactPoint, &vForce );
						pLastVerlet = pVerlet;
					}

					// pass damage to boomer entities
					if( pEntity->TypeBits() & ENTITY_BIT_BOOMER && pSentinel->m_fSpeed_WS > 0.0f )
					{
						CEBoomer *pBoomer;
						pBoomer = (CEBoomer*) pEntity;

						if( !pBoomer->IsDetPackOnly() && !pBoomer->IsVehicleCollide() )
						{
							if( uIndex == 0 )
							{
								bSkipCol = TRUE;
							}

							CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
							if( pDamageForm )
							{
								pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
								pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
								pDamageForm->m_pDamageProfile = pSentinel->GetDamageProfile();
								pDamageForm->m_Damager.pEntity = (CEntity*) pSentinel;
								pDamageForm->m_Damager.pWeapon = NULL;
								pDamageForm->m_Damager.pBot = pSentinel;
								pDamageForm->m_Damager.nDamagerPlayerIndex = pSentinel ? pSentinel->m_nPossessionPlayerIndex : -1;
								pDamageForm->m_pDamageeEntity = pEntity;
								pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *) pImpact->pTag, pImpact, &pSentinel->m_UnitVelocity_WS );
								CDamage::SubmitDamageForm( pDamageForm );
							}
						}
					}

					// pass collision info to other vehicles, and look for bots to squish
					if( pEntity->TypeBits() & ENTITY_BIT_BOT && !( pEntity->TypeBits() & ENTITY_BIT_SITEWEAPON ) )
					{
						CBot *pBot = (CBot *) pEntity;

						if( uIndex == 0 )
						{
							if( pBot->TypeBits() & ENTITY_BIT_VEHICLE )
							{
								CVehicle *pVehicle = (CVehicle *) pBot;
								// record pointer to other vehicle's physics object
								pSentinel->m_aColPoints[nColIndex].pOtherObj = &pVehicle->m_PhysObj;
							}
							else
							{
								// don't send this collision to physics system
								bSkipCol = TRUE;
							}
						}

						// attempt to squish bot
						if( pSentinel->m_PhysObj.GetVelocity()->MagSq() > _MIN_SQUISH_VEL_SQ &&		// no squish if moving very slowly
							!pSentinel->IsFlippingUpright() &&										// no squish if flipping upright
							pBot->GetParent() != pSentinel &&										// no squish if bot is standing on vehicle
							!pBot->IsInAir() &&														// no squish if bot is in the air
							pBot != pSentinel->GetDriverBot() )										// don't squish if bot is driving vehicle
						{
							if( _nBotsToSquish < _MAX_BOTS_TO_SQUISH )
							{
								BOOL bQueued = FALSE;

								for( nBots = 0; nBots < _nBotsToSquish; nBots++ )
								{
									if( _apBotsToSquish[ nBots ] == pBot )
									{
										// this bot already queued for squishing
										bQueued = TRUE;
										break;
									}
								}

								if( !bQueued )
								{
									_apBotsToSquish[ _nBotsToSquish ] = pBot;
									++_nBotsToSquish;
								}
							}
						}
					}

					// if this is an entity we want to skip, skip
					if( (uIndex == 0) && (pEntity->TypeBits() & _ENTITY_BITS_TO_SKIP_COLLISION) ) 
					{
						bSkipCol = TRUE;
					}
				}
			}
		}

		if( !bSkipCol )
		{
			CFVec3A vSphereColUnitDir, vSphereCenter;

			pImpact = *FColl_apSortedImpactBuf;

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

  			if( pSentinel->SmoothContact() && nColIndex < CVehicleSentinel::COLPOINT_L_C_WHEEL_INDEX && 
				vSphereColUnitDir.Dot( pImpact->UnitFaceNormal ) > -0.9f )
			{
				// if sphere collides with tri such that the tri normal is significantly different than the
				// direction from the sphere to the collision point on the sphere, then use the negated sphere direction
				// as the collision normal instead of the tri normal.  The goal is to try to allow the front treads to
				// ride up over vertical curbs & rocks.
				pSentinel->m_aColPoints[nColIndex].vNormal = vSphereColUnitDir;
				pSentinel->m_aColPoints[nColIndex].vNormal.Negate();
			}
			else
			{
				pSentinel->m_aColPoints[nColIndex].vNormal = pImpact->UnitFaceNormal;
			}

			pSentinel->m_aSentinelColPoints[nColIndex].pMaterial = CGColl::GetMaterial( pImpact );

			pSentinel->m_aColPoints[nColIndex].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_COLLIDED;
			pSentinel->m_aColPoints[nColIndex].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_APPLY_FRICTION;
			pSentinel->m_aColPoints[nColIndex].fTimeSinceCollision = 0.0f;
//			pSentinel->m_aColPoints[nColIndex].vNormal = pImpact->UnitFaceNormal;
			pSentinel->m_aColPoints[nColIndex].vPoint = pImpact->ImpactPoint;
			pSentinel->m_aColPoints[nColIndex].vPoint.Sub( pState->vPosition );
			pSentinel->m_aColPoints[nColIndex].fDepth = pImpact->fImpactDistInfo;
			pSentinel->m_aColPoints[nColIndex].vPush = pImpact->PushUnitVec;

			// allow friction in vertical direction when collision point on sphere is in direction of
			// the x vector of the friction matrix. (i.e. vertical friction only when that part of sphere
			// is touching the ground.)
			CFVec3A vSpherePoint;
			vSpherePoint.Set( pImpact->ImpactPoint );
			vSpherePoint.Sub( *pvCurrentPos );
			vSpherePoint.Unitize();
			pSentinel->m_aColPoints[nColIndex].vStaticFriction.y = pSentinel->m_BotInfo_VehiclePhysics.fBodyFriction * 
							FMATH_FABS( vSpherePoint.Dot( pSentinel->m_aColPoints[nColIndex].pmFrictOr->m_vRight ) );
		}

		bReturnVal = TRUE;
	}
	else
	{
		pSentinel->m_aColPoints[nColIndex].uClientFlags = 0;
		pSentinel->m_aColPoints[nColIndex].uClientFlags |= CFPhysicsObject::CLIENT_FLAG_APPLY_FRICTION;
		pSentinel->m_aColPoints[nColIndex].fTimeSinceCollision += FLoop_fPreviousLoopSecs;
	}

		// compute kinetic transition value
	fTrans = fmath_Div( pState->vVelocity.MagSq(), pSentinel->m_BotInfo_VehiclePhysics.fMaxSpeed * pSentinel->m_BotInfo_VehiclePhysics.fMaxSpeed );
	FMATH_CLAMPMAX( fTrans, 1.0f );
	fTrans *= _VAR_KINETIC_TRANS;
	fTrans += _MIN_KINETIC_TRANS;
	if( nColIndex < 2 )
	{
				fTrans += _KINETIC_TRANS_BALANCE * fTrans;
	}
	else if( nColIndex > 3 )
	{
		fTrans -= _KINETIC_TRANS_BALANCE * fTrans;
	}
	pSentinel->m_aColPoints[nColIndex].fKineticTransition = fTrans;



	return bReturnVal;
}

//-----------------------------------------------------------------------------
// this function is called by the vehicle's physics object.  It collides the vehicle with
// the world terrain and reports the resulting collisions to the physics object via the array m_aColPoints[].
static void _SentinelCollisionDetect( const CFPhysicsObject::PhysicsState_t *pState )
{
	CBot *pBot = CVehicleSentinel::m_pCurrentCollider;
	CVehicleSentinel *pSentinel = ( CVehicleSentinel* ) CVehicleSentinel::m_pCurrentCollider;
	const CFVec3A *pvPos;
	const CFMtx43A *pOr = &pState->mOrientation;
	CFVec3A vOffset;
	const CFVec3A *pvWheel;
	s32 nIndex;

	if( pBot == NULL )
	{
		return;
	}

	// set up the collision info struct for all collsions below
	pSentinel->m_SentinelColData.nFlags = FCOLL_DATA_IGNORE_BACKSIDE;
	pSentinel->m_SentinelColData.nCollMask = FCOLL_MASK_COLLIDE_WITH_VEHICLES;
	pSentinel->m_SentinelColData.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	pSentinel->m_SentinelColData.pCallback = _IntersectingTrackerCallback;
	pSentinel->m_SentinelColData.pLocationHint = pSentinel->m_pWorldMesh;

	// set the sentinel collision point info for the wheels
	for( nIndex = 0; nIndex < CVehicleSentinel::NUM_WHEELS; nIndex++ )
	{
		if( nIndex < 4 )
		{
			pvWheel = pSentinel->GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_L_WHEEL_A + nIndex );
		}
		else
		{
			// rear wheel collision points are custom bones in Sentinel, not rear wheels
			pvWheel = pSentinel->GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_COLLBONE_R1 + nIndex - 4 );
		}
		FASSERT( pvWheel );
		pSentinel->m_aSentinelColPoints[nIndex].vCurrentPos.Set( *pvWheel );
		pSentinel->m_aSentinelColPoints[nIndex].mFriction.Set( *pSentinel->MtxToWorld() );
		pSentinel->m_aSentinelColPoints[nIndex].mFriction.m_vPos.Zero();
	}

	// get a pointer to the world space position of the chassis bone of the vehicle.
	pvPos = pSentinel->GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_TORSO );
	FASSERT( pvPos );

	// set sentinel collision point info for non-wheel collision spheres.
	for( nIndex = 0; nIndex < CVehicleSentinel::MAX_COL_POINTS - CVehicleSentinel::NUM_WHEELS; nIndex++ )
	{
		vOffset.Set( _afBodySpherePos[nIndex][0], _afBodySpherePos[nIndex][1], _afBodySpherePos[nIndex][2] );
		pOr->MulPoint( vOffset );
		vOffset.Add( *pvPos );
		pSentinel->m_aSentinelColPoints[CVehicleSentinel::NUM_WHEELS + nIndex].vCurrentPos.Set( vOffset );
		pSentinel->m_aSentinelColPoints[CVehicleSentinel::NUM_WHEELS + nIndex].mFriction.Set( *pSentinel->MtxToWorld() );
		pSentinel->m_aSentinelColPoints[CVehicleSentinel::NUM_WHEELS + nIndex].mFriction.m_vPos.Zero();
	}

	// do a manual UpdateMatrices() with pState position and orientation
	{
		CFMtx43A mRot = pState->mOrientation;
		CFVec3A vCMOffset;

		// copy physics object's position and orientation to vehicle's world mesh.
		mRot.MulPoint( vCMOffset, pSentinel->m_vCMOffset );
		mRot.m_vPos = pState->vPosition;
		mRot.m_vPos.Sub( vCMOffset );
		pSentinel->m_pWorldMesh->m_Xfm.BuildFromMtx( mRot );
		pSentinel->m_pWorldMesh->UpdateTracker();

		// Update bone matrix palette
		pSentinel->ComputeMtxPalette( FALSE );
	}

	for( nIndex = 0; nIndex < CVehicleSentinel::NUM_WHEELS; nIndex++ )
	{
		if( nIndex < 4 )
		{
			pSentinel->m_aSentinelColPoints[nIndex].vProposedPos.Set( *(pSentinel->GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_L_WHEEL_A + nIndex )) );
		}
		else
		{
			// rear wheel collision points are custom bones in Sentinel, not rear wheels
			pSentinel->m_aSentinelColPoints[nIndex].vProposedPos.Set( *(pSentinel->GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_COLLBONE_R1 + nIndex - 4 )) );
		}
	}

	// get a pointer to the world space position of the chassis bone of the vehicle.
	pvPos = pSentinel->GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_TORSO );
	FASSERT( pvPos );

	// set sentinel collision point info for non-wheel collision spheres.
	for( nIndex = 0; nIndex < CVehicleSentinel::MAX_COL_POINTS - CVehicleSentinel::NUM_WHEELS; nIndex++ )
	{
		vOffset.Set( _afBodySpherePos[nIndex][0], _afBodySpherePos[nIndex][1], _afBodySpherePos[nIndex][2] );
		pOr->MulPoint( vOffset );
		vOffset.Add( *pvPos );
		pSentinel->m_aSentinelColPoints[CVehicleSentinel::NUM_WHEELS + nIndex].vProposedPos.Set( vOffset );
	}


	// collide all points
	for( nIndex = 0; nIndex < CVehicleSentinel::MAX_COL_POINTS; nIndex++ )
	{
		FWorld_nTrackerSkipListCount = 0;
		pSentinel->AppendTrackerSkipList();
		pSentinel->m_SentinelColData.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
		pSentinel->m_SentinelColData.ppTrackerSkipList = FWorld_apTrackerSkipList;
		_CollideSphere( pSentinel, pState, nIndex );
	}

#if _ADJUST_SPHERES
	_bDrewSpheres = TRUE;
#endif
}


//-----------------------------------------------------------------------------
void CVehicleSentinel::_UpdateCannon( void )
{
	f32 fReloadTime;
	CFMtx43A *pmPrimaryFire = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire];
	m_fCannonReloadTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fCannonReloadTime, 0.0f );

	// construct time that increases (m_fCannonReloadTime decreases to 0)
	fReloadTime = _CANNON_RELOAD_TIME - m_fCannonReloadTime;

	// fire cannon
	if( m_eSentinelState == VEHICLESENTINEL_STATE_DRIVING &&
		m_fControls_Fire1 > 0.01f && m_fCannonReloadTime < 0.001f )
	{
		m_fCannonReloadTime = _CANNON_RELOAD_TIME;
//#if 1
//#if 0
//f32 a = 1.0f;
//f32 b = 0.0f;
//f32 c = a/b;
//#else
//EXCEPTION_RECORD r;
//_EXCEPTION_POINTERS p;
//p.ExceptionRecord = &r;
//r.ExceptionCode = EXCEPTION_FLT_DIVIDE_BY_ZERO;
//r.ExceptionAddress = (PVOID) 0xbadc0de;
//extern long _fexception_Production_Handler( struct _EXCEPTION_POINTERS *pExceptionInfo );
//_fexception_Production_Handler( &p );
//#endif
//#endif

		CBot::JustFired();

		CFMtx43A mProj;
		mProj.Set( *pmPrimaryFire );

		m_CannonTracerDef.pUser = this;
		m_CannonTracerDef.UnitDir_WS.Set( m_vTargetPoint );
		m_CannonTracerDef.UnitDir_WS.Sub( mProj.m_vPos );
		m_CannonTracerDef.UnitDir_WS.Unitize();
		m_CannonTracerDef.TailPos_WS = mProj.m_vPos;
		m_CannonTracerDef.TailPos_WS.Add( mProj.m_vFront );

		ConstructDamagerData();
		if( tracer_NewTracer( m_hCannonTracerGroup, &m_CannonTracerDef, 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager ) )
		{
			// controller rumble
			if( m_nPossessionPlayerIndex >= 0 )
			{
				fforce_Kill( &m_hForce );
				fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );

				ShakeCamera( 0.05f, 0.25f );
			}
		}

		// play cannon firing sfx
		if( m_nPossessionPlayerIndex >= 0 )
		{
			if( m_hCannonSFX != FSNDFX_INVALID_FX_HANDLE )
			{
				fsndfx_Play2D( m_hCannonSFX, _CANNON_SFX_VOLUME, fmath_RandomFloatRange( 0.45f, 0.52f ) );
			}
		} 
		else
		{
			if( m_hCannonSFX != FSNDFX_INVALID_FX_HANDLE )
			{
				fsndfx_Play3D( m_hCannonSFX, &pmPrimaryFire->m_vPos, _MACHINE_GUN_SFX_RADIUS, 1.0f, _MACHINE_GUN_SFX_VOLUME, fmath_RandomFloatRange( 0.45f, 0.52f ) );
			}
		}
	}

	// muzzle smoke
	if( fReloadTime < _CANNON_MUZZLE_SMOKE_TIME )
	{
		f32 fUnitTime = fmath_Div( fReloadTime, _CANNON_MUZZLE_SMOKE_TIME );
		FMATH_CLAMP_UNIT_FLOAT( fUnitTime );
		if( m_hSmokeParticle != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hSmokeParticle, TRUE );
			fparticle_SetIntensity( m_hSmokeParticle, 1.0f - fUnitTime );
			fparticle_SetDirection( m_hSmokeParticle, (const CFVec3 *) &pmPrimaryFire->m_vFront );
		}
	}
	else
	{
		if( m_hSmokeParticle != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_SetIntensity( m_hSmokeParticle, 0.0f );
			fparticle_EnableEmission( m_hSmokeParticle, FALSE );
		}
	}

	// muzzle flash
	if( fReloadTime < _CANNON_MUZZLE_FLASH_TIME )
	{
		f32 fUnitTime = fmath_Div( fReloadTime, _CANNON_MUZZLE_FLASH_TIME );
		FMATH_CLAMP_UNIT_FLOAT( fUnitTime );

		CMuzzleFlash::AddFlash_CardPosterXY(
			CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
			pmPrimaryFire->m_vPos,	// position
			3.0f + fUnitTime * 2.0f,// scale
			1.0f - fUnitTime );		// alpha

		CMuzzleFlash::AddFlash_CardPosterZ(
			CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 ],
			pmPrimaryFire->m_vPos,		// position
			pmPrimaryFire->m_vFront,	// direction
			3.0f + fUnitTime * 2.0f,	// width
			50.0f + fUnitTime * 10.0f,	// length
			1.0f - fUnitTime );			// alpha
#if 1
		CMuzzleFlash::AddFlash_Mesh3D(
//			CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_3D_STAR_1 ],
			CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_3D_ENERGY_ORANGE ],
			pmPrimaryFire->m_vPos,		// position
			pmPrimaryFire->m_vFront,	// direction
//			2.0f + fUnitTime,			// scale
			0.5f + 0.5f *fUnitTime,		// scale
			0.0f );						// rotation
#endif
	}

	// muzzle light
	if( fReloadTime < _CANNON_MUZZLE_LIGHT_TIME * 2.0f )
	{
		f32 fRadius;
		if( fReloadTime < _CANNON_MUZZLE_LIGHT_TIME )
		{
			fRadius = fmath_Div( fReloadTime, _CANNON_MUZZLE_LIGHT_TIME ) * _CANNON_MUZZLE_LIGHT_RADIUS;
		}
		else
		{
			fRadius = (1.0f - fmath_Div( fReloadTime - _CANNON_MUZZLE_LIGHT_TIME, _CANNON_MUZZLE_LIGHT_TIME  )) * _CANNON_MUZZLE_LIGHT_RADIUS;
		}

		FMATH_CLAMPMIN( fRadius, 1.0f );
		CMuzzleFlash::MuzzleLight( &pmPrimaryFire->m_vPos, fRadius );
	}

	// push tank up and back
	if( fReloadTime < _CANNON_FORCE_TIME )
	{
		CFMtx43A *pmTurret = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexTurret];
		const CFMtx43A *pmBody = m_PhysObj.GetOrientation();
		CFVec3A vPoint, vForce;
		f32 fDot;

		// construct up force vector and application point
		vPoint.Set( pmTurret->m_vFront );
		vPoint.Mul( _CANNON_FORCE_ARM );
		vForce.Set( pmBody->m_vUp );
		vForce.Mul( _CANNON_UP_FORCE );

		// add more up force if cannon is pointing forwards or backwards along tank body
		fDot = pmTurret->m_vFront.Dot( pmBody->m_vFront );
		fDot = 1.3f * FMATH_FABS( fDot ) + 1.2f;
		vForce.Mul( fDot );
		m_PhysObj.ApplyForce( &vForce, &vPoint );

		vForce.Set( pmTurret->m_vFront );
		vForce.Mul( -_CANNON_BACK_FORCE );
		m_PhysObj.ApplyForce( &vForce );
	}
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact )
{
	if( nKillReason == TRACER_KILLREASON_HIT_GEO )
	{
		FASSERT( pTracerDef->pUser != NULL );
		CVehicleSentinel *pSentinel = (CVehicleSentinel*)(pTracerDef->pUser);

#if 1
		if( pSentinel->m_hShellExplosion != FEXPLOSION_INVALID_HANDLE ) {
			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				FExplosionSpawnParams_t ExplosionSpawnParams;

				ExplosionSpawnParams.InitToDefaults();

				ExplosionSpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
				ExplosionSpawnParams.Pos_WS = pImpact->ImpactPoint;
				ExplosionSpawnParams.UnitDir = pImpact->UnitFaceNormal;
				ExplosionSpawnParams.uSurfaceType = CGColl::GetSurfaceType( pImpact );
				ExplosionSpawnParams.pDamageProfile = NULL;
				ExplosionSpawnParams.pDamager = &pTracerDef->Damager;

				CExplosion2::SpawnExplosion( hSpawner, pSentinel->m_hShellExplosion, &ExplosionSpawnParams );
			}
		}
#else
		const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );

		pCollMaterial->DrawAllDrawableParticles(
			&pImpact->ImpactPoint,
			&pImpact->UnitFaceNormal,
			TRUE,
			fmath_RandomFloatRange( 0.0f, 1.0f ),
			fmath_RandomFloatRange( 0.0f, 1.0f ),
			fmath_RandomFloatRange( 0.0f, 1.0f )
		);

		explosion_Spawn( EXPLOSION_TYPE_ROCKET,		// type
						 pImpact->ImpactPoint,		// position
						 1.0f,						// intensity
						 &pImpact->UnitFaceNormal,	// direction
						 TRUE,						// spawn debris
						 1000.f );					// audio radius

		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm )
		{
			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
			pDamageForm->m_pDamageProfile = m_pBotInfo_Vehicle->pPrimaryWeaponDamageProfile;
			pDamageForm->m_Epicenter_WS = pImpact->ImpactPoint;
			pDamageForm->m_Damager.nDamagerPlayerIndex = pTracerDef->nOwnerPlayerIndex;
			pDamageForm->m_Damager.pEntity = pSentinel;
			CDamage::SubmitDamageForm( pDamageForm );
		}
#endif

		// make an AI impact sound in case any AI are listening
		if( pSentinel->m_nPossessionPlayerIndex > -1 )
		{
			AIEnviro_AddSound( pImpact->ImpactPoint,				// position
							   AIEnviro_fLaserImpactAudibleRange,	// audible radius
							   0.3f,								// duration
							   AISOUNDTYPE_WEAPON,					// sound type
							   0,									// control flags
							   pSentinel );							// entity making sound
		}
	}
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_UpdateMachineGun( void )
{
	m_fMachineGunReloadTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fMachineGunReloadTime, 0.0f );

	m_fNextAIMachineGunSound -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fNextAIMachineGunSound, 0.0f );

	m_fMachineGunDebrisTimer -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fMachineGunDebrisTimer, 0.0f );

	m_fNextMachineGunTracer -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fNextMachineGunTracer, 0.0f );

	if( m_eSentinelState == VEHICLESENTINEL_STATE_DRIVING &&
		m_fControls_Fire2 > 0.01f && m_fMachineGunReloadTime <= 0.0f )
	{
		CFVec3A vFireUnitDir, vPerturbVec;
		CFMtx43A *pmSecondaryFire = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexSecondaryFire];

		m_fMachineGunReloadTime = _MACHINE_GUN_RELOAD_TIME;

		CBot::JustFired();

		vPerturbVec.Set( fmath_RandomFloatRange( -_MACHINE_GUN_JITTER_FACTOR, _MACHINE_GUN_JITTER_FACTOR ),
						fmath_RandomFloatRange( -_MACHINE_GUN_JITTER_FACTOR, _MACHINE_GUN_JITTER_FACTOR ),
						fmath_RandomFloatRange( -_MACHINE_GUN_JITTER_FACTOR, _MACHINE_GUN_JITTER_FACTOR ) );

		vFireUnitDir.Set( m_vTargetPoint );
		vFireUnitDir.Add( vPerturbVec );
		vFireUnitDir.Sub( pmSecondaryFire->m_vPos );
		vFireUnitDir.Unitize();

		f32 fRandomScale = fmath_RandomFloatRange( 0.5f, 1.0f );

		if( IsDrawEnabled() )
		{
			CMuzzleFlash::AddFlash_CardPosterZ( CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
				pmSecondaryFire->m_vPos,
				vFireUnitDir,
				3.0f * fRandomScale,	// width
				3.0f * fRandomScale,	// height
				1.0f );					// alpha

			CMuzzleFlash::AddFlash_CardPosterXY( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
				pmSecondaryFire->m_vPos,
				3.0f * fRandomScale,			// scale
				1.0f );							// alpha

			CMuzzleFlash::AddFlash_Mesh3D( CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_3D_STAR_1 + fmath_RandomRange( 0, 1 ) ],
				pmSecondaryFire->m_vPos,
				vFireUnitDir,
				3.0f * fRandomScale,				// scale
				0.0f );								// rotation
		}

		CMuzzleFlash::MuzzleLight( &pmSecondaryFire->m_vPos, 10.0f );

		if( m_nPossessionPlayerIndex >= 0 )
		{
			fforce_Kill( &m_hForce );

			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce );

			if( m_hMachineGunSFX != FSNDFX_INVALID_FX_HANDLE )
			{
				fsndfx_Play2D( m_hMachineGunSFX, 1.0f, fmath_RandomFloatRange( 0.8f, 1.2f ) );
			}
		} 
		else
		{
			if( m_hMachineGunSFX != FSNDFX_INVALID_FX_HANDLE )
			{
				fsndfx_Play3D( m_hMachineGunSFX, &pmSecondaryFire->m_vPos, _MACHINE_GUN_SFX_RADIUS, 1.0f, 1.0f, fmath_RandomFloatRange( 0.8f, 1.2f ) );
			}
		}

		CFVec3A RayEnd;
		FCollImpact_t CollImpact;

		RayEnd.Mul( vFireUnitDir, _MACHINE_GUN_MAX_RANGE ).Add( pmSecondaryFire->m_vPos );

		FWorld_nTrackerSkipListCount = 0;
		AppendTrackerSkipList();
		if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &pmSecondaryFire->m_vPos, &RayEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) )
		{
			// Hit something...

			RayEnd = CollImpact.ImpactPoint;

			const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( &CollImpact );

			pCollMaterial->DrawAllDrawableParticles( &CollImpact.ImpactPoint,
													&CollImpact.UnitFaceNormal,
													FALSE,
													fmath_RandomFloatRange( 0.2f, 1.0f ),
													fmath_RandomFloatRange( 0.2f, 0.5f ),
													fmath_RandomFloatRange( 0.1f, 0.3f ) );

			if( m_fMachineGunDebrisTimer <= 0.0f )
			{
				CFDebrisSpawner DebrisSpawner;
				DebrisSpawner.InitToDefaults();

				DebrisSpawner.m_Mtx.m_vPos = CollImpact.ImpactPoint;
				DebrisSpawner.m_Mtx.m_vZ = CollImpact.UnitFaceNormal;
				DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
				DebrisSpawner.m_pDebrisGroup = pCollMaterial->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_LARGE ];
				DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
				DebrisSpawner.m_fMinSpeed = 10.0f;
				DebrisSpawner.m_fMaxSpeed = 20.0f;
				DebrisSpawner.m_fUnitDirSpread = 0.2f;
				DebrisSpawner.m_fScaleMul = 1.0f;
				DebrisSpawner.m_fGravityMul = 1.0f;
				DebrisSpawner.m_fRotSpeedMul = 1.0f;
				DebrisSpawner.m_nMinDebrisCount = 3;
				DebrisSpawner.m_nMaxDebrisCount = 6;

				CGColl::SpawnDebris( &DebrisSpawner );

				m_fMachineGunDebrisTimer = fmath_RandomFloatRange( 1.0f, 3.0f );
			}


			if( pCollMaterial->IsImpactFlashEnabled() )
			{
				CFVec3A ImpactFlashPos;
				ImpactFlashPos.Mul( CollImpact.UnitFaceNormal, 1.0f ).Add( CollImpact.ImpactPoint );
				CMuzzleFlash::AddFlash_CardPosterXY( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
					ImpactFlashPos,
					1.0f,		// scale
					1.0f );		// alpha
			}

			CFDecal::Create( m_hBulletDecal, &CollImpact, &vFireUnitDir );

			if( CollImpact.pTag == NULL )
			{
				// Hit world geo...
				//if( pCollMaterial->IsPockMarkEnabled( &CollImpact ) )
				//{
				//	potmark_NewPotmark( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.2f, POTMARKTYPE_BULLET );
				//}
			}
			else
			{
				// Hit an object...
				CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;

				if( pWorldMesh->m_nUser == MESHTYPES_ENTITY )
				{
					CEntity *pDamagee = (CEntity *)pWorldMesh->m_pUser;

					// Get an empty damage form...
					CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

					if( pDamageForm )
					{
						pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
						pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
						pDamageForm->m_pDamageProfile = m_pBotInfo_Vehicle->pSecondaryWeaponDamageProfile;
						pDamageForm->m_Damager.pWeapon = NULL;
						pDamageForm->m_Damager.pBot = this;
						pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
						pDamageForm->m_pDamageeEntity = pDamagee;
						pDamageForm->InitTriDataFromCollImpact( pWorldMesh, &CollImpact, &vFireUnitDir );

						CDamage::SubmitDamageForm( pDamageForm );
					}
				}
			}

			// Make an AI impact sound in case any AI are listening...
			if( m_nPossessionPlayerIndex > -1 && m_fNextAIMachineGunSound <= 0.0f )
			{
				AIEnviro_AddSound( CollImpact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, this );
				m_fNextAIMachineGunSound = 0.5f;
			}

		}

		BOOL bMakeTracer = FALSE;
		f32 fTracerLen = pmSecondaryFire->m_vPos.Dist( RayEnd );

		if( m_nPossessionPlayerIndex < 0 )
		{
			// AI...
			bMakeTracer = fmath_RandomChance( 0.4f );
		}
		else
		{
			// Human-controlled...
			if( m_fNextMachineGunTracer <= 0.0f )
			{
				m_fNextMachineGunTracer = fmath_RandomFloatRange( 0.05f, 0.125f );
				bMakeTracer = TRUE;
			}
		}

		if( bMakeTracer )
		{
			// Make a tracer...

			CMuzzleFlash::AddFlash_CardPosterZ( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1],	// group handle
												pmSecondaryFire->m_vPos,	// starting position
												vFireUnitDir,				// unit direction of travel
												0.3f,						// width
												fTracerLen,					// z length
												0.5f );						// alpha
		}
	}
}



//-----------------------------------------------------------------------------
// animate tread textures
void CVehicleSentinel::_UpdateTreads( void )
{
	if( m_hTreadTexLayerR != FMESH_TEXLAYERHANDLE_INVALID && m_hTreadTexLayerL != FMESH_TEXLAYERHANDLE_INVALID )
	{
		CFVec3A vOffset, vVel;
		CFVec2 vVec;
		const CFMtx43A *pmOr;
		f32 fVelR, fVelL;
		vVec.x = 0.0f;

		pmOr = m_PhysObj.GetOrientation();

		vOffset.Set( 6.0f, 0.0f, 0.0f );
		pmOr->MulDir( vOffset );
		m_PhysObj.GetVelocity( &vOffset, &vVel );
		fVelR = vVel.Dot( pmOr->m_vFront );

		vOffset.Negate();
		m_PhysObj.GetVelocity( &vOffset, &vVel );
		fVelL = vVel.Dot( pmOr->m_vFront );

		m_fTreadUnitAnimPosR += FLoop_fPreviousLoopSecs * fVelR * 0.5f;
		if( m_fTreadUnitAnimPosR > 1.0f )
		{
			m_fTreadUnitAnimPosR -=1.0f;
		}
		m_fTreadUnitAnimPosL += FLoop_fPreviousLoopSecs * fVelL * 0.5f;
		if( m_fTreadUnitAnimPosL > 1.0f )
		{
			m_fTreadUnitAnimPosL -=1.0f;
		}

		vVec.y = -m_fTreadUnitAnimPosR;
		m_pWorldMesh->AnimTC_SetScrollST( m_hTreadTexLayerR, vVec );
		vVec.y = -m_fTreadUnitAnimPosL;
		m_pWorldMesh->AnimTC_SetScrollST( m_hTreadTexLayerL, vVec );
	}
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_UpdateReticle( void )
{
	if( m_nPossessionPlayerIndex < 0 )
	{
		return;
	}

	if( m_pDriverBot == NULL )
	{
		return;
	}

#if 1
	ComputeHumanTargetPoint_WS( &m_vTargetPoint, NULL, _CANNON_MAX_RANGE );
#else
	CFCamera *pCamera;
	const FViewport_t *pViewport;
	f32 fReticleX, fReticleY, fMaxLiveRange, fEndPointReticleAdjust;
	CFVec3A RayStartPoint, RayEndPoint, ReticleAdjustX, ReticleAdjustY;
	FCollImpact_t CollImpact;
	CFTrackerCollideRayInfo CollRayInfo;
	CFWorldTracker *pHitTracker;
	CEntity *pHitEntity;
	CReticle* pReticle;

	m_pTargetedEntity = NULL;


	fReticleX = pReticle->GetNormOriginX();
	fReticleY = pReticle->GetNormOriginY();

	pReticle->ColorScheme_SelectInactive();

	// Get camera and viewport info...
	pCamera = gamecam_GetActiveCamera();
	pViewport = pCamera->GetViewport();

	// Ray start point is the camera position...
	RayStartPoint.Set( m_mCamera.m_vPos );

	// Compute the ray end point which lies in the center of the reticle...
	fMaxLiveRange = _CANNON_MAX_RANGE;

	fEndPointReticleAdjust = (fMaxLiveRange * fReticleX) * pViewport->fTanHalfFOVX;
	ReticleAdjustX.Mul( m_mCamera.m_vRight, fEndPointReticleAdjust );

	fEndPointReticleAdjust = (fMaxLiveRange * fReticleY) * pViewport->fTanHalfFOVY;
	ReticleAdjustY.Mul( m_mCamera.m_vUp, fEndPointReticleAdjust );

	RayEndPoint.Mul( m_mCamera.m_vFront, fMaxLiveRange ).Add( RayStartPoint ).Add( ReticleAdjustX ).Add( ReticleAdjustY );
	
	// Build tracker skip list...
	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	// Find the closest impact to the camera...
	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &RayStartPoint, &RayEndPoint, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList ) ) 
	{
		m_vTargetPoint.Set( CollImpact.ImpactPoint );

		pHitTracker = (CFWorldTracker *)CollImpact.pTag;
		if( pHitTracker ) 
		{
			// We hit a tracker...
			if( pHitTracker->m_nUser == MESHTYPES_ENTITY ) 
			{
				// We hit an entity...

				pHitEntity = (CEntity *)pHitTracker->m_pUser;

				if( pHitEntity->IsTargetable() ) 
				{
					// It's a targetable entity...
					pReticle->ColorScheme_SelectActive();
					m_pTargetedEntity = pHitEntity;
					return;
				}
			}
		}
	}
	else
	{
		m_vTargetPoint.Set( RayEndPoint );
	}
#endif
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_UpdateCollisionFlags( void )
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
				m_pRecentCollisionMaterial = m_aSentinelColPoints[nIndex].pMaterial;
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
void CVehicleSentinel::_UpdateDustCloud( void )
{
	s32 nIndex;

	// update dust cloud 
	FASSERT( MAX_CLOUD_PARTICLES == 2 );
	if( m_hCloudParticle[0] != FPARTICLE_INVALID_HANDLE &&
		m_hCloudParticle[1] != FPARTICLE_INVALID_HANDLE )
	{
		if( SmoothContact() && m_fSpeed_WS > _MIN_CLOUD_SPEED &&
			((m_aSentinelColPoints[COLPOINT_L_A_WHEEL_INDEX].pMaterial && 
			m_aSentinelColPoints[COLPOINT_L_A_WHEEL_INDEX].pMaterial->HasLooseDust()) ||
			(m_aSentinelColPoints[COLPOINT_R_A_WHEEL_INDEX].pMaterial && 
			m_aSentinelColPoints[COLPOINT_R_A_WHEEL_INDEX].pMaterial->HasLooseDust())) )
		{
			for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
			{
				fparticle_EnableEmission( m_hCloudParticle[nIndex], TRUE );
				m_fCloudParticleIntensity[nIndex] = m_fClampedNormSpeedXZ_WS;
				m_vCloudParticleDirection[nIndex].Set( m_PhysObj.GetOrientation()->m_vFront );
				m_vCloudParticleVelocity[nIndex].Set( *m_PhysObj.GetVelocity() );
			}

			m_vCloudParticlePosition[0].Set( m_PhysObj.GetOrientation()->m_vRight );
			m_vCloudParticlePosition[0].Mul( _CLOUD_INWARD_OFFSET );
			m_vCloudParticlePosition[1].Set( m_PhysObj.GetOrientation()->m_vRight );
			m_vCloudParticlePosition[1].Mul( -_CLOUD_INWARD_OFFSET );

			if( m_MtxToWorld.m_vFront.Dot( *m_PhysObj.GetVelocity() ) >= 0.0f )
			{
				m_vCloudParticlePosition[0].Add( *GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_L_WHEEL_E ) );
				m_vCloudParticlePosition[1].Add( *GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_R_WHEEL_E ) );
				m_vCloudParticleDirection[0].Negate();
				m_vCloudParticleDirection[1].Negate();
			}
			else
			{
				m_vCloudParticlePosition[0].Add( *GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_L_WHEEL_A ) );
				m_vCloudParticlePosition[1].Add( *GetTagPoint( CVehicleSentinel::TAG_POINT_INDEX_R_WHEEL_A ) );
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
void CVehicleSentinel::_StartDriverEnterWork( void )
{
	DriverEnter( m_pDriverBot );

	m_eSentinelState = VEHICLESENTINEL_STATE_ENGINE_STARTING;
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_AbortEntry( void )
{
	m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
	m_pDriverBot->SetNoCollideStateAir( FALSE );
	m_pDriverBot = NULL;
	m_DriverCameraTrans.SetCameraStateAbort();

	m_eSentinelState = VEHICLESENTINEL_STATE_UNOCCUPIED;
}


//-----------------------------------------------------------------------------
void CVehicleSentinel::_ClearDataMembers( void )
{
	s32 nIndex;

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

	m_eSentinelState = VEHICLESENTINEL_STATE_UNOCCUPIED;
	m_fSteeringPosition = 0.0f;
	m_fTurretAngleWS = 0.0f;
	m_fTurretUnitRot = 0.0f;
	m_fCannonElevation = 0.0f;
	m_vTurretXZ.Set( CFVec3A::m_UnitAxisZ );

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

	m_fTreadUnitAnimPosR = 0.0f;
	m_fTreadUnitAnimPosL = 0.0f;
	m_hTreadTexLayerR = FMESH_TEXLAYERHANDLE_INVALID;
	m_hTreadTexLayerL = FMESH_TEXLAYERHANDLE_INVALID;

	m_fCannonReloadTime = 0.0f;
	m_vTurretRecoil.Zero();

	m_hCannonTracerGroup = TRACER_NULLGROUPHANDLE;
	m_pTargetedEntity = NULL;
	m_vTargetPoint.Zero();

	m_fMachineGunReloadTime = 0.0f;
	m_fNextAIMachineGunSound = 0.0f;
	m_fMachineGunDebrisTimer = 0.0f;
	m_fNextMachineGunTracer = 0.0f;

	m_uSentinelFlags = 0;
	m_DriverCameraTrans.SetTransitionLength( _CAMERA_TRANSITION_TIME );
}


//-----------------------------------------------------------------------------
// protected functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// put driver bot into vehicle
void CVehicleSentinel::DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName /* = NULL */, BOOL bImmediately /* = FALSE */ )
{
	s32 nIndex;

	if( pDriverBot == NULL )
	{
		return;
	}

	CVehicle::DriverEnter( pDriverBot, "AttachPoint_Driver" );

	// enable dirt emitters 
	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[nIndex], TRUE );
		}
	}
	
	m_pDriverBot = pDriverBot;
}

//-----------------------------------------------------------------------------
// remove driver from vehicle
void CVehicleSentinel::DriverExit( CBot *pDriverBot )
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

	if( m_eSentinelState == VEHICLESENTINEL_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT || m_eSentinelState == VEHICLESENTINEL_STATE_MOVE_DRIVER_TO_ENTRY_POINT )
	{
		// DriverEnter() has not yet been called.
		_AbortEntry();
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
}

//-----------------------------------------------------------------------------
// move the vehicle in response to driver control inputs
void CVehicleSentinel::MoveVehicle( void )
{
	CFVec3A vVelForward;
	const CFMtx43A *pmPhysObjOr;
	const CFVec3A *pvVel;
	s32 nWheelsContacting = 0;
	f32 fVelForward;
	f32 fForce;
	f32 fSteerForceScaling = 0.0f;	// no steering force unless vehicle is occupied

	pmPhysObjOr = m_PhysObj.GetOrientation();

	// get vehicle velocity
	pvVel = m_PhysObj.GetVelocity();
	// find component of velocity in direction that vehicle is pointing
	fVelForward = m_MtxToWorld.m_vFront.Dot( *pvVel );
	vVelForward.Mul( m_MtxToWorld.m_vFront, fVelForward );

	// over time, return turret to face front if driver's seat is empty
	if( m_pDriverBot == NULL )
	{
		f32 fNewAngle = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
		f32 fDeltaAngle = fNewAngle - m_fTurretAngleWS;
		if( fDeltaAngle >= 0.0f )
		{
			if( fDeltaAngle > FMATH_DEG2RAD( 180.0f ) )
			{
				fDeltaAngle = FMATH_DEG2RAD( 360.0f ) - fDeltaAngle;
			}
		}
		else
		{
			if( fDeltaAngle < FMATH_DEG2RAD( -180.0f ) )
			{
				fDeltaAngle = FMATH_DEG2RAD( 360.0f ) + fDeltaAngle;
			}
		}

		f32 fMaxRate = _TURRET_RETURN_RATE * FLoop_fPreviousLoopSecs;
		if( fDeltaAngle > fMaxRate )
		{
			fDeltaAngle = fMaxRate;
		}
		else if( fDeltaAngle < -fMaxRate )
		{
			fDeltaAngle = -fMaxRate;
		}

		m_fTurretAngleWS += fDeltaAngle;
	}

	f32 fAIThrottle = 0.0f;
	if( m_pDriverBot != NULL && m_eSentinelState == VEHICLESENTINEL_STATE_DRIVING )
	{
		m_fTurretAngleWS += m_fControls_RotateCW * FLoop_fPreviousLoopSecs * _TURRET_ROTATION_RATE;
#if defined(MA_PC_INPUT)
		// Mouse look aims the turret and cannon directly (a raw angle, not a rate).
		m_fTurretAngleWS += TakeMouseLookDelta( FALSE );
		m_fCannonElevation += TakeMouseLookDelta( TRUE );
#endif
		if( m_fTurretAngleWS > FMATH_DEG2RAD( 360.0f ) )
		{
			m_fTurretAngleWS -= FMATH_DEG2RAD( 360.0f );
		}
		else if( m_fTurretAngleWS < 0.0f )
		{
			m_fTurretAngleWS += FMATH_DEG2RAD( 360.0f );
		}

		// allow controls to affect elevation angle of turret cannon
		m_fCannonElevation += m_fControls_AimDown * FLoop_fPreviousLoopSecs * _CANNON_ELEVATION_RATE;
		if( m_fCannonElevation > _CANNON_MAX_DOWN_ELEVATION )
		{
			m_fCannonElevation = _CANNON_MAX_DOWN_ELEVATION;
		}
		else if( m_fCannonElevation < -_CANNON_MAX_UP_ELEVATION )
		{
			m_fCannonElevation = -_CANNON_MAX_UP_ELEVATION;
		}

		if( m_bControls_Human )
		{
			m_ControlsBot_XlatNormSpeedXZ_WS.x = m_fControlsHuman_StrafeRight;
			m_ControlsBot_XlatNormSpeedXZ_WS.z = m_fControlsHuman_Forward;
			m_ControlsBot_XlatNormSpeedXZ_WS.y = 0.0f;
			m_ControlsBot_XlatNormSpeedXZ_WS.RotateY(m_fTurretAngleWS);
		}

		m_fThrottle = m_ControlsBot_XlatNormSpeedXZ_WS.Mag();
		fAIThrottle = m_fThrottle;
		if (m_fThrottle !=0.0f)
		{
			CFVec3A vFrontXZ = m_MtxToWorld.m_vFront;
			vFrontXZ.y = 0.0f;
			if (vFrontXZ.SafeUnitAndMag(vFrontXZ))
			{
				CFVec3A vRightXZ;
				vRightXZ.x = vFrontXZ.z;
				vRightXZ.z = -vFrontXZ.x;
				vRightXZ.y = 0.0f;

				CFVec3A UnitControlsBot_XlatNormSpeedXZ_WS;
				UnitControlsBot_XlatNormSpeedXZ_WS.ReceiveUnitXZ(m_ControlsBot_XlatNormSpeedXZ_WS);

				f32 fDot = UnitControlsBot_XlatNormSpeedXZ_WS.Dot( vFrontXZ );
				f32 fDotRight = UnitControlsBot_XlatNormSpeedXZ_WS.Dot( vRightXZ );
				if( fDot < 0.0f )
				{
					// new desired direction is more than 90 degrees from current facing direction

					if( m_bControls_Human && UnitControlsBot_XlatNormSpeedXZ_WS.Dot( m_mCamera.m_vFront ) < 0.5f )
					{
						// Let player tank move backwards rather than always spinning body around to face forward.
						// But, only allow this if the tank body isn't facing too close to the screen Z
						// direction.  This prevents the tank from facing backwards while driving forward
						// into the screen.
						fDotRight = -fDotRight;
						m_fThrottle = -m_fThrottle;
					}
					else
					{
						// rotate in place until less than 90 degrees from desired direction
						m_fThrottle = 0.0f;
						if( fDotRight > 0.0f )
						{
							fDotRight = 1.0f;
						}
						else
						{
							fDotRight = -1.0f;
						}
					}
				}
				else
				{
					m_fThrottle *= fDot;
				}
				m_fTurretUnitRot = fDotRight;
			}
		}
		else
		{
			m_fTurretUnitRot = 0.0f;
		}

		// ai driver can always steer tank, even if not moving
		fSteerForceScaling = 1.0f;
	}
	else
	{
		// nobody driving vehicle, or vehicle un-drivable, zero throttle
		m_fThrottle = 0.0f;
	}

	if( FMATH_FABS( m_fThrottle ) > 0.02f || fAIThrottle > 0.0f )
	{
		// driver is applying throttle
		f32 fVelMag = vVelForward.Mag();
		BOOL bApply = TRUE;

		// only apply force if vehicle is under maximum speed
		if( fVelForward > 0.0f && m_fThrottle > 0.0f && fVelMag > m_fMaxSpeed  )
		{
			bApply = FALSE;
		}
		else if( fVelForward < 0.0f && m_fThrottle < 0.0f && fVelMag > m_fMaxSpeed * _MAX_REVERSE_VEL_MUL )
		{
			bApply = FALSE;
		}

		if( bApply )
		{
			if( SmoothContact() )
			{
				// reduce driving force as slope steepens
				f32 fSlopeMul = CFVec3A::m_UnitAxisY.Dot(m_PhysObj.GetOrientation()->m_vFront );
				if( m_fThrottle >= 0.0f )
				{
					if( fSlopeMul > 0.0f )
					{
						fSlopeMul = 1.0f - fSlopeMul;
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
					}
					else
					{
						fSlopeMul = 1.0f;
					}
				}

				// apply forward force to vehicle center-of-mass
				CFVec3A vForce;
				vForce.Set( m_PhysObj.GetOrientation()->m_vFront );
				vForce.Mul( m_fThrottle );
				vForce.Mul( _FORWARD_FORCE );
				vForce.Mul( fSlopeMul );
				m_PhysObj.ApplyForce( &vForce );
			}
		}
	}

	// steer tank with forces proportional to angle between turret and tank body
	if( FMATH_FABS( fVelForward ) > 1.0f || fAIThrottle > 0.0f )
	{
		// don't apply force unless moving a little bit.
		// this prevents calling ApplyForce() unecessarily,
		// which in turn disables the momentum clamping
		CFVec3A vSteerArm, vSteerForce;
		f32 fSteerForce;

		fSteerForce = m_fTurretUnitRot * fSteerForceScaling;

		fSteerForce *= _STEERING_FORCE;

		vSteerArm.Set( m_PhysObj.GetOrientation()->m_vFront );
		vSteerArm.Mul( _STEERING_ARM );

		vSteerForce.Set( m_PhysObj.GetOrientation()->m_vRight );
		vSteerForce.Mul( fSteerForce );

		m_PhysObj.ApplyForce( &vSteerForce, &vSteerArm );

		vSteerArm.Negate();
		vSteerForce.Negate();
		m_PhysObj.ApplyForce( &vSteerForce, &vSteerArm );
	}

	// do flipover work
	if( m_fFlipOverTime > 0.0f )
	{
		CFVec3A vForce, vPoint;
		m_fFlipOverTime -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fFlipOverTime, 0.0f );
		vPoint.Set( pmPhysObjOr->m_vRight );
		vPoint.Mul( m_fFlipArm );
		vForce.Set( pmPhysObjOr->m_vUp );
		vForce.Mul( -_FLIP_FORCE );
		m_PhysObj.ApplyForce( &vForce, &vPoint );
	}
	else if( SmoothContact() && // vehicle touching ground
		pmPhysObjOr->m_vUp.Dot( CFVec3A::m_UnitAxisY ) < 0.0f ) // is upside down
	{
		if( m_pDriverBot != NULL && m_eSentinelState == VEHICLESENTINEL_STATE_DRIVING )
		{
			m_eSentinelState = VEHICLESENTINEL_STATE_START_UPSIDE_DOWN;
		}
	}
#if !FANG_PRODUCTION_BUILD
	else
	{
		// allow driver to "jump" the vehicle
		CFVec3A vForce, vPoint;
		vPoint.Set( 0.0f, 0.0f, 0.0f );
//		vPoint.Set( m_MtxToWorld.m_vRight );
//		vPoint.Mul( 4.0f );

		vForce.Set( 0.0f, m_bControls_Jump * _JUMP_FORCE, 0.0f );
		m_PhysObj.ApplyForce( &vForce, &vPoint );
	}
#endif

	if( !SmoothContact() )
	{
		// vehicle is in the air, lets try to help it stay upright

		// compute unitized "righting" force to apply: 0.0 = rightside up, 1.0 = upside down
		fForce = m_PhysObj.GetOrientation()->m_vUp.Dot( m_vGroundNormal );
		fForce -= 1.0f;
		fForce *= -0.5f;

		if( fForce > 0.01f && fForce < 0.4f )
		{
			CFVec3A vForce, vArm;
			fForce *= _RIGHTING_FORCE;
			vForce.Set( m_vGroundNormal );
			vForce.Mul( fForce );

			// find arm at which to apply force
			vArm.Cross( m_vGroundNormal, pmPhysObjOr->m_vUp );
			vArm.Unitize();
			vArm.Cross( CFVec3A::m_UnitAxisY );
			vArm.Unitize();
			vArm.Mul( _RIGHTING_ARM );

			m_PhysObj.ApplyForce( &vForce, &vArm );

			vForce.Negate();
			vArm.Negate();

			m_PhysObj.ApplyForce( &vForce, &vArm );
		}
	}
}

//-----------------------------------------------------------------------------
// update the location of the 3rd person vehicle camera
void CVehicleSentinel::UpdateCamera( void )
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
	}
	else
	{
		// m_fTimeSinceLastContact doesn't have hysteresis of SmoothContact()
		m_DriverCamera.SetAirTime( m_fTimeSinceLastContact );
	}

	m_DriverCamera.SetElevationAngle( m_fCannonElevation + _CAM_ELEVATION_OFFSET_ANGLE );
	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();
	m_DriverCamera.Update( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexTurret], *m_PhysObj.GetVelocity(), m_pWorldMesh, m_vGroundNormal );

	m_DriverCameraTrans.UpdateTransition();
}

void CVehicleSentinel::ClassHierarchyBecameActive( void )
{
	if( m_pPartMgr->IsCreated() )
	{
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_A_LEFT_FRONT );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_B_LEFT_REAR );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_C_RIGHT_FRONT );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_D_RIGHT_REAR );
	}

	CVehicle::ClassHierarchyBecameActive();
}


//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CVehicleSentinel::InitSystem( void )
{
//	FResFrame_t ResFrame;
	
	FASSERT( !m_bSystemInitialized );

//	ResFrame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	m_nBotClassClientCount = 0;

	return TRUE;

	// Error:
//_ExitInitSystemWithError:
//	FASSERT(0);
//	UninitSystem();
//	fres_ReleaseFrame( ResFrame );
//	return FALSE;
}


//-----------------------------------------------------------------------------
void CVehicleSentinel::UninitSystem( void )
{
	if( m_bSystemInitialized )
	{
		m_AnimStackDef.Destroy();
		m_bSystemInitialized = FALSE;
	}
}



//-----------------------------------------------------------------------------
CVehicleSentinel::CVehicleSentinel() : CVehicle()
{
	m_pInventory = NULL;
	m_pWorldMesh = NULL;
}


//-----------------------------------------------------------------------------
CVehicleSentinel::~CVehicleSentinel()
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
BOOL CVehicleSentinel::ClassHierarchyLoadSharedResources( void )
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
		DEVPRINTF( "CVehicleSentinel::InitSystem(): Could not find particle definition '%s'.\n", _DIRT_PARTICLE_DEF_NAME );
	}

	m_hCloudParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _CLOUD_PARTICLE_DEF_NAME );
	if( m_hCloudParticleDef == FPARTICLE_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CVehicleSentinel::InitSystem(): Could not find particle definition '%s'.\n", _CLOUD_PARTICLE_DEF_NAME );
	}

	m_hSmokeParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _SMOKE_PARTICLE_DEF_NAME );
	if( m_hSmokeParticleDef == FPARTICLE_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CVehicleSentinel::InitSystem(): Could not find particle definition '%s'.\n", _SMOKE_PARTICLE_DEF_NAME );
	}

	m_hShellExplosion = CExplosion2::GetExplosionGroup( "Sentinel" );

	pTexDef = (FTexDef_t *)( fresload_Load(FTEX_RESNAME, "tfmpshot_01") );
	if(pTexDef == NULL)
	{
		DEVPRINTF("CVehicleSentinel::InitSystem() : Could not load tracer texture '%s'.\n", "tfmpshot_01" );
	} 
	else
	{
		m_CannonTracerTexInst.SetTexDef( pTexDef );
	}

	// initialize sfx handles

	if( !fresload_Load( FSNDFX_RESTYPE, _RAT_SOUND_BANK ) ) 
	{
		// Sentinel currently uses Rat SFX
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Could not load sound effect bank '%s'\n", _RAT_SOUND_BANK );
	}

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		goto _ExitWithError;
	}

	m_hEngineStartSFX = fsndfx_GetFxHandle( _ENGINE_START_SFX_TAG_NAME );
	if( m_hEngineStartSFX == FSNDFX_INVALID_FX_HANDLE )
	{
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Unable to get SFX handle for %s.\n", _ENGINE_START_SFX_TAG_NAME );
	}
	m_hEngineStopSFX = fsndfx_GetFxHandle( _ENGINE_STOP_SFX_TAG_NAME );
	if( m_hEngineStopSFX == FSNDFX_INVALID_FX_HANDLE )
	{
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Unable to get SFX handle for %s.\n", _ENGINE_STOP_SFX_TAG_NAME );
	}
	m_hSkidSFX = fsndfx_GetFxHandle( _SKID_SFX_TAG_NAME );
	if( m_hSkidSFX == FSNDFX_INVALID_FX_HANDLE )
	{
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Unable to get SFX handle for %s.\n", _SKID_SFX_TAG_NAME );
	}
	m_hCannonSFX = fsndfx_GetFxHandle( _CANNON_SFX_TAG_NAME );
	if( m_hCannonSFX == FSNDFX_INVALID_FX_HANDLE )
	{
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Unable to get SFX handle for %s.\n", _CANNON_SFX_TAG_NAME );
	}
	m_hMachineGunSFX = fsndfx_GetFxHandle( _MACHINE_GUN_SFX_TAG_NAME );
	if( m_hMachineGunSFX == FSNDFX_INVALID_FX_HANDLE )
	{
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Unable to get SFX handle for %s.\n", _MACHINE_GUN_SFX_TAG_NAME );
	}

	// Load mesh resource...
	m_pSentinelMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszVehicleMeshFilename );
	if( m_pSentinelMesh == NULL )
	{
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Could not find mesh '%s'\n", _apszVehicleMeshFilename );
		goto _ExitWithError;
	}

	m_hBulletDecal = CFDecal::GetDefByName( _BULLET_DECAL );
	if( m_hBulletDecal == FDECALDEF_INVALID_HANDLE )
	{
		DEVPRINTF( "CVehicleSentinel Load Shared Resources: Unable to get decal handle for %s.\n", _BULLET_DECAL );

	}

	return TRUE;


_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//-----------------------------------------------------------------------------
void CVehicleSentinel::ClassHierarchyUnloadSharedResources( void )
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	CVehicle::ClassHierarchyUnloadSharedResources();
}




//-----------------------------------------------------------------------------
BOOL CVehicleSentinel::Create( s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );


	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CVehicleSentinelBuilder *pBuilder = (CVehicleSentinelBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_VehicleSentinelBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_VEHICLESENTINEL );
	}

	// Set our builder parameters...

	// Create an entity...
	return CVehicle::Create( &m_BotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}


//-----------------------------------------------------------------------------
void CVehicleSentinel::ClassHierarchyDestroy( void )
{
	s32 nIndex;
	// Delete the items that we had instantiated for us...

	tracer_DestroyGroup(m_hCannonTracerGroup);
	m_hCannonTracerGroup = TRACER_NULLGROUPHANDLE;

	fforce_Kill( &m_hForce );

	m_Anim.Destroy();

	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_KillEmitter( m_hDirtParticle[nIndex] );
		}
	}

	for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
	{
		if( m_hCloudParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_KillEmitter( m_hCloudParticle[nIndex] );
		}
	}

	CVehicle::ClassHierarchyDestroy();
}

//-----------------------------------------------------------------------------
#define _MAX_TRACERS_PER_SENTINEL	( 4 )
#define _CM_OFFSET_Y				( 4.0f )
BOOL CVehicleSentinel::ClassHierarchyBuild( void )
{
	FMeshInit_t MeshInit;
	u32 i;
	s32 nBoneIndex;
	s32 nIndex;
	CFMtx43A *pmPrimaryFire = NULL;
	
	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CVehicleSentinelBuilder *pBuilder = (CVehicleSentinelBuilder *)GetLeafClassBuilder();

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


	// Initialize from builder object...

	// Create world mesh...
	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the world mesh...
	MeshInit.pMesh		= m_pSentinelMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
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

	if( !m_Anim.Create( &m_AnimStackDef, m_pWorldMesh ) )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild(): Trouble creating m_Anim.\n" );
		goto _ExitWithError;
	}

	SetControlValue( ANIMCONTROL_REST, 1.0f );

	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_L_A_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_A ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_R_A_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_A ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_L_C_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_C ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_R_C_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_C ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_L_E_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_E ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_R_E_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_E ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_L_B_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_B ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_R_B_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_B ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_L_D_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_L_WHEEL_D ] );
	m_anBoneIndexWheel[ WHEEL_BONE_ARRAY_R_D_INDEX ] = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_R_WHEEL_D ] );
	m_nBoneIndexTurret = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_TURRET ] );
	m_nBoneIndexCannonBase = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_CANNONBASE ] );
	m_nBoneIndexCannon = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_CANNON ] );
	m_nBoneIndexSpotLight = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_SPOTLIGHT ] );
	m_nBoneIndexPrimaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_PRIMARY_FIRE ] );
	m_nBoneIndexSecondaryFire = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_SECONDARY_FIRE ] );
	m_nBoneIdxDriverAttach = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_ATTACHPOINT_DRIVER ] );

	m_Anim.m_pAnimCombiner->SetBoneCallback( &_AnimBoneCallback );

	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_A ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_A ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_C ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_C ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_E ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_E ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_B ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_B ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_L_WHEEL_D ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_R_WHEEL_D ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_TURRET ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_CANNONBASE ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_SPOTLIGHT ] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[ BONE_CANNON ] );

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
		for( uIndex = 0; uIndex < _ANTENNA_SEGMENTS; uIndex++ )
		{
			if( anBones[uIndex] < 0 )
			{
				DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild: Antenna Bone %d doesn't exist in object '%s'.\n", uIndex, Name() );
			}
		}

		m_Antenna.RegisterBones( anBones );
	}
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA1] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA2] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA3] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA4] );
	m_Anim.m_pAnimCombiner->EnableBoneCallback( m_apszBoneNameTable[BONE_ANTENNA5] );

	for( i = 0; i < NUM_ALL_WHEELS; i++ )
	{
		m_afWheelAngle[ i ] = 0.0f;
	}

	// Find tag points...
	for( i=0, m_nTagPointCount=0; i<TAG_POINT_COUNT; i++ )
	{
		nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );

		if( nBoneIndex < 0 )
		{
			DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild(): Could not locate TagPoint bone '%s'.\n", m_apszBoneNameTable[ m_anTagPointBoneNameIndexArray[i] ] );
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
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild(): Could not locate approx eye point bone '%s'.\n", m_apszBoneNameTable[ m_nApproxEyePointBoneNameIndex ] );
		m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	} 
	else
	{
		m_pApproxEyePoint_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vPos;
	}

	// Find gaze direction...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_PRIMARY_FIRE ] );
	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild(): Could not locate gaze bone '%s'.\n", m_apszBoneNameTable[ BONE_PRIMARY_FIRE ] );
		m_pGazeDir_WS = &m_MtxToWorld.m_vFront;
	} 
	else
	{
		m_pGazeDir_WS = &m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex]->m_vFront;
	}

	// Find torso mtx...
	nBoneIndex = m_pWorldMesh->FindBone( m_apszBoneNameTable[ BONE_TURRET ] );
	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBotBlink::ClassHierarchyBuild(): Could not locate torso bone '%s'.\n", m_apszBoneNameTable[ nBoneIndex ] );
		m_pAISteerMtx = &m_MtxToWorld;
	} else {
		m_pAISteerMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
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
	m_vCMOffset.Set( 0.0f, _CM_OFFSET_Y, 0.0f );

	// player uses action button to enter & exit vehicle
	SetActionable( TRUE );

	for( nIndex = 0; nIndex < MAX_DIRT_PARTICLES; nIndex++ )
	{
		m_vDirtParticlePosition[nIndex].Set( m_MtxToWorld.m_vPos );		// set to a default value
		m_vDirtParticleVelocity[nIndex].Zero();							// set to a default value
		m_vDirtParticleDirection[nIndex].Set( CFVec3A::m_UnitAxisX );	// set to a default value
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
			if( m_hCloudParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
			{
				m_fCloudParticleIntensity[nIndex] = 0.0f;
				fparticle_SetDirection( m_hCloudParticle[nIndex], (const CFVec3 *) &m_vCloudParticleDirection[nIndex] );
				fparticle_SetIntensity( m_hCloudParticle[nIndex], (const f32 *) &m_fCloudParticleIntensity[nIndex] );
				fparticle_EnableEmission( m_hCloudParticle[nIndex], FALSE );
			}
		}
	}

	pmPrimaryFire = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIndexPrimaryFire];
	m_hSmokeParticle = fparticle_SpawnEmitter( m_hSmokeParticleDef, (const CFVec3 *) &pmPrimaryFire->m_vPos );
	if( m_hSmokeParticle != FPARTICLE_INVALID_HANDLE )
	{
		fparticle_SetDirection( m_hSmokeParticle, (const CFVec3 *) &CFVec3::m_UnitAxisY );
		fparticle_SetIntensity( m_hSmokeParticle, 0.0f );
		fparticle_EnableEmission( m_hSmokeParticle, FALSE );
	}

	// initialize collision point data
	for( nIndex = 0; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		CFPhysicsObject::InitCollisionPoint( &m_aColPoints[nIndex] );

		m_aColPoints[nIndex].uResultFlags = 0;
		m_aColPoints[nIndex].fTimeSinceCollision = _FRICTION_CONTACT_TIME + 1.0f;	// set so friction isn't applied until after first collision
		m_aColPoints[nIndex].fStaticHysteresis = _STATIC_TRANS_HYST;
		m_aColPoints[nIndex].fFrictionSmoothTime = _FRICTION_CONTACT_TIME;
		m_aColPoints[nIndex].pmFrictOr = &m_aSentinelColPoints[nIndex].mFriction;
		m_aColPoints[nIndex].fAppliedForceKineticMul = _KINETIC_FORCE_MUL;

		m_aSentinelColPoints[nIndex].vCurrentPos.Zero();
		m_aSentinelColPoints[nIndex].vProposedPos.Zero();
		m_aSentinelColPoints[nIndex].mFriction.Identity();
		m_aSentinelColPoints[nIndex].pMaterial = NULL;
	}

	for( nIndex = 0; nIndex < NUM_WHEELS; nIndex++ )
	{
		m_aColPoints[nIndex].vStaticFriction.Set( _fStaticFriction, m_BotInfo_VehiclePhysics.fBodyFriction, _TREAD_ROLLING_FRICTION );
		m_aColPoints[nIndex].vKineticFriction.Set( _fKineticFriction, m_BotInfo_VehiclePhysics.fBodyFriction, _TREAD_ROLLING_FRICTION );

		m_aSentinelColPoints[nIndex].fRadius = _afWheelRadius[nIndex] + _TREAD_THICKNESS;
		if( nIndex >= COLPOINT_L_E_WHEEL_INDEX )
		{
			m_aSentinelColPoints[nIndex].fRadius = _REAR_WHEEL_COLL_SPHERE_RADIUS + _TREAD_THICKNESS;
		}
	}
	for( nIndex = NUM_WHEELS; nIndex < MAX_COL_POINTS; nIndex++ )
	{
		m_aColPoints[nIndex].vAppliedForce.Zero();
		m_aColPoints[nIndex].vStaticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );
		m_aColPoints[nIndex].vKineticFriction.Set( m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction, m_BotInfo_VehiclePhysics.fBodyFriction );

		m_aSentinelColPoints[nIndex].fRadius = _afBodySphereRadius[nIndex-NUM_WHEELS];
	}

	m_pSpotLight = m_pWorldMesh->GetAttachedLightByID( _HEADLIGHT_ID );
	SetSpotLightOff();

	m_hTreadTexLayerR = m_pWorldMesh->GetTexLayerHandle( _RIGHT_TREAD_TEXTURE_ID );
	if( m_hTreadTexLayerR == FMESH_TEXLAYERHANDLE_INVALID )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild():  Unable to get tex layer handle for right Tread\n" );
	}
	else
	{
		m_pWorldMesh->AnimTC_AnimateScroll( m_hTreadTexLayerR, FALSE );
		m_pWorldMesh->LayerAlpha_Set( m_hTreadTexLayerR, 1.0f );
	}

	m_hTreadTexLayerL = m_pWorldMesh->GetTexLayerHandle( _LEFT_TREAD_TEXTURE_ID );
	if( m_hTreadTexLayerL == FMESH_TEXLAYERHANDLE_INVALID )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild():  Unable to get tex layer handle for left Tread\n" );
	}
	else
	{
		m_pWorldMesh->AnimTC_AnimateScroll( m_hTreadTexLayerL, FALSE );
		m_pWorldMesh->LayerAlpha_Set( m_hTreadTexLayerL, 1.0f );
	}

	m_CannonTracerDef.pUser						= NULL;
	m_CannonTracerDef.pFcnKillCallback			= _TracerKilledCallback;
	m_CannonTracerDef.pFcnBuildSkipList			= NULL;
	m_CannonTracerDef.fWidth_WS					= 4.0f;
	m_CannonTracerDef.fLength_WS				= 500.0f;
	m_CannonTracerDef.fSpeed_WS					= _CANNON_ROUND_SPEED;
	m_CannonTracerDef.fMaxTailDist_WS			= _CANNON_MAX_RANGE;
	m_CannonTracerDef.fBeginDeathUnitFade_WS	= 0.25f;
	m_CannonTracerDef.uFlags					= TRACERFLAG_DRAW_ADDITIVE | TRACERFLAG_THICK_PROJECTILE;
	m_CannonTracerDef.ColorRGBA.OpaqueWhite();

	m_hCannonTracerGroup = tracer_CreateGroup( &m_CannonTracerTexInst,  _MAX_TRACERS_PER_SENTINEL );
	if( m_hCannonTracerGroup == TRACER_NULLGROUPHANDLE )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild(): Could not create tracer group.\n" );
		goto _ExitWithError;
	}

	fforce_NullHandle( &m_hForce );

	if( !m_pPartMgr->Create( this, &m_pPartPool, _BOTPART_FILENAME, PART_INSTANCE_COUNT_PER_TYPE, LIMB_TYPE_COUNT ) )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyBuild() Failed to init Bot Part Manager.\n" );
	}

	if( m_pPartMgr->IsCreated() )
	{
		m_pPartMgr->RepairWhenStolen( LIMB_TYPE_BOX_A_LEFT_FRONT  );
		m_pPartMgr->RepairWhenStolen( LIMB_TYPE_BOX_B_LEFT_REAR );
		m_pPartMgr->RepairWhenStolen( LIMB_TYPE_BOX_C_RIGHT_FRONT );
		m_pPartMgr->RepairWhenStolen( LIMB_TYPE_BOX_D_RIGHT_REAR );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_A_LEFT_FRONT );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_B_LEFT_REAR );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_C_RIGHT_FRONT );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_D_RIGHT_REAR );
	}

	// set initial turret angle to face front
	m_fTurretAngleWS = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL CVehicleSentinel::ClassHierarchyBuilt( void )
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

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


//-----------------------------------------------------------------------------
void CVehicleSentinel::FlipRightsideUp( void )
{
	if( m_eSentinelState == VEHICLESENTINEL_STATE_UPSIDE_DOWN )
	{
		m_eSentinelState = VEHICLESENTINEL_STATE_START_FLIPPING_UPRIGHT;
	}
}

//-----------------------------------------------------------------------------
// play one-shot engine crank sound, after which, start the engine
void CVehicleSentinel::CrankEngine( void )
{
	m_eEngineState = ENGINE_STATE_CRANKING;
	m_fEngineTime = _ENGINE_START_TIME;

	if( m_hEngineStartSFX != FSNDFX_INVALID_FX_HANDLE )
	{
		if( m_nPossessionPlayerIndex >= 0 )
		{
			fsndfx_Play2D( m_hEngineStartSFX, _ENGINE_SFX_VOLUME );
		}
		else
		{
			fsndfx_Play3D( m_hEngineStartSFX, &m_MtxToWorld.m_vPos, _ENGINE_SFX_RADIUS, -1.0f, _ENGINE_SFX_VOLUME );
		}
	}
}

//-----------------------------------------------------------------------------
// start the looping engine sound (no engine crank sound)
void CVehicleSentinel::StartEngine( void )
{
	m_fEngineTime = 0.0f;
	m_eEngineState = ENGINE_STATE_RUNNING;

	if( m_pEngineIdleEmitter == NULL )
	{
		m_pEngineIdleEmitter = AllocAndPlaySound( m_BotInfo_Engine.pSoundGroupEngineIdle );

		if( m_pEngineIdleEmitter == NULL )
		{
			DEVPRINTF( "Unable to alloc 'n play Vehicle EngineIdle SFX.\n" );
		}
		else
		{
			m_pEngineIdleEmitter->SetVolume( 0.0f );

		}
	}

	if( m_pEngineLowEmitter == NULL )
	{
		m_pEngineLowEmitter = AllocAndPlaySound( m_BotInfo_Engine.pSoundGroupEngineLow );

		if( m_pEngineLowEmitter == NULL )
		{
			DEVPRINTF( "Unable to alloc 'n play Vehicle EngineLow SFX.\n" );
		}
		else
		{
			m_pEngineLowEmitter->SetVolume( 0.0f );

		}
	}

	if( m_pEngineHighEmitter == NULL )
	{
		m_pEngineHighEmitter = AllocAndPlaySound( m_BotInfo_Engine.pSoundGroupEngineHigh );

		if( m_pEngineHighEmitter == NULL )
		{
			DEVPRINTF( "Unable to alloc 'n play Vehicle EngineHigh SFX.\n" );
		}
		else
		{
			m_pEngineHighEmitter->SetVolume( 0.0f );

		}
	}

	if( m_pSkidEmitter == NULL )
	{
		if( m_nPossessionPlayerIndex >= 0 )
		{
			m_pSkidEmitter = FSNDFX_ALLOCNPLAY2D( m_hSkidSFX, _ENGINE_SFX_VOLUME, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
		}
		else
		{
			m_pSkidEmitter = FSNDFX_ALLOCNPLAY3D( m_hSkidSFX, &m_MtxToWorld.m_vPos, _ENGINE_SFX_RADIUS, _ENGINE_SFX_VOLUME, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
		}

		if( m_pSkidEmitter == NULL )
		{
			DEVPRINTF( "Unable to alloc 'n play Vehicle Skid SFX.\n" );
		}
		else
		{
			m_pSkidEmitter->SetVolume( 0.0f );
		}
	}
}

//-----------------------------------------------------------------------------
// stop the engine sounds.  If bImmediately is TRUE, don't play engine die sound.
void CVehicleSentinel::StopEngine( BOOL bImmediately )
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

		if( m_hEngineStopSFX != FSNDFX_INVALID_FX_HANDLE )
		{
			if( m_nPossessionPlayerIndex >= 0 )
			{
				fsndfx_Play2D( m_hEngineStopSFX, _ENGINE_SFX_VOLUME );
			}
			else
			{
				fsndfx_Play3D( m_hEngineStopSFX, &m_MtxToWorld.m_vPos, _ENGINE_SFX_RADIUS, -1.0f, _ENGINE_SFX_VOLUME );
			}
		}
	}

	// stop engine sounds
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
void CVehicleSentinel::RunEngine( void )
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
void CVehicleSentinel::HandleTargeting( void )
{
	//don't call base class, it doesn't work since
	//vehicle sentinel doesn't aim the mount.

	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );

	//do all targeting lock bit checks here.
	if (!m_bControls_Human &&
		(m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT) &&
		(m_fCannonReloadTime < 0.001f || m_fMachineGunReloadTime <= 0.0f))
	{
		//findfix: should check that m_ControlBot_TargetPoint_WS is currently aimed at within reason.
		m_vTargetPoint = m_ControlBot_TargetPoint_WS;
		// Announce that we're locked... and ready to fire
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
	}
}

//-----------------------------------------------------------------------------
BOOL CVehicleSentinel::ActionNearby( CEntity *pEntity )
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

	if( m_pDriverBot == NULL )
	{
		if( IsUpsideDown() )
		{
			FlipRightsideUp();
			return TRUE;
		}

		// nobody driving vehicle, allow bot to enter
		if( CanOccupyStation( pBot, STATION_DRIVER ) == STATION_STATUS_EMPTY )
		{
			if( EnterStation( pBot, STATION_DRIVER, TRUE /*bProximityCheck*/, FALSE /*bImmediately*/ ) == STATION_STATUS_ENTERING )
			{
				return TRUE;
			}
		}
	}
	else
	{
		if( m_pDriverBot == pBot )
		{
			// Driving bot has requested to exit vehicle

			if( m_eSentinelState == VEHICLESENTINEL_STATE_DRIVING )
			{
				if( ExitStation( pBot, FALSE /*bImmediately*/ ) == STATION_STATUS_EXITING )
				{
					return TRUE;
				}
			}
		}
	
		// otherwise it's another bot trying to enter vehicle while being driven
	}

	return FALSE;
}

CBot *CVehicleSentinel::IsStationObstructed( Station_e eStation )
{
	FASSERT( eStation == STATION_DRIVER );
	return CVehicle::StationObstructed( this, m_nBoneIdxDriverAttach, m_pBotInfo_Vehicle->fDriverStationRadius );
}

//-----------------------------------------------------------------------------
// inquiry function to determine if it is possible for a bot to enter a vehicle
// at a particular station.
// if STATION_STATUS_EMPTY is returned, then bot may enter.  Other return values
// indicate why bot cannot occupy the station.
CVehicle::StationStatus_e CVehicleSentinel::CanOccupyStation( CBot *pBot, Station_e eStation )
{
	CFSphere BotCollSphere, StationCollSphere;
	CFVec3A vTestPoint;
	CFVec3A vTestDir;
	CFVec3A vDelta;
	CFVec3A vTemp;
	CFMtx43A *pmDriverSeat;
	f32 fDist;

	FASSERT( pBot );
	if( pBot == NULL )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( !( pBot->TypeBits() & ( ENTITY_BIT_BOTGLITCH | ENTITY_BIT_BOTGRUNT ) ) )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( eStation != STATION_DRIVER )
	{
		return STATION_STATUS_NO_STATION;
	}

	if( m_pDriverBot != NULL )
	{
		return STATION_STATUS_OCCUPIED;
	}

	if( pBot->SwitchingWeapons() )
	{
		return STATION_STATUS_WEAPON_SWITCH;
	}

	if( pBot->m_nPossessionPlayerIndex >= 0 && (m_uVehicleFlags & VEHICLE_FLAG_NO_PLAYER_DRIVE) )
	{
		// bot is a player and vehicle is flagged no-player-drive
		return STATION_STATUS_UNAVAILABLE;
	}

	if( m_eSentinelState != VEHICLESENTINEL_STATE_UNOCCUPIED )
	{
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

	if( pBot->m_nPossessionPlayerIndex >= 0 )
	{
		// see if player-controlled driver is in valid entry area at rear of vehicle.
		// (AI bots only want to know if they're at the driving position, not the entry area.)
		vTestDir.Set( m_MtxToWorld.m_vFront );
		vTestDir.Negate();
		vTestPoint.Set( vTestDir );
		vTestPoint.Mul( 10.0f );
		vTestPoint.Add( m_MtxToWorld.m_vPos );
		vDelta.Set( pBot->MtxToWorld()->m_vPos );
		vDelta.Sub( vTestPoint );
		fDist = vTestDir.Dot( vDelta );
		if( fDist > 0.0f && fDist < 6.0f )
		{
	//		m_PhysObj.DrawLine( &vTestPoint, &pBot->MtxToWorld()->m_vPos );
			return STATION_STATUS_EMPTY;
		}
	}

	pmDriverSeat = m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach];
	StationCollSphere.m_Pos.x = pmDriverSeat->m_vPos.x;
	StationCollSphere.m_Pos.y = pmDriverSeat->m_vPos.y;
	StationCollSphere.m_Pos.z = pmDriverSeat->m_vPos.z;
	StationCollSphere.m_fRadius = _STATION_COLLSPHERE_RADIUS;
	BotCollSphere.m_Pos.x = pBot->m_pBotInfo_Gen->fCollSphere1X_MS + pBot->MtxToWorld()->m_vPos.x;
	BotCollSphere.m_Pos.y = pBot->m_pBotInfo_Gen->fCollSphere1Y_MS + pBot->MtxToWorld()->m_vPos.y;
	BotCollSphere.m_Pos.z = pBot->m_pBotInfo_Gen->fCollSphere1Z_MS + pBot->MtxToWorld()->m_vPos.z;
	BotCollSphere.m_fRadius = pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;
	if( StationCollSphere.IsIntersecting( BotCollSphere ) )
	{
		if( pBot->m_nPossessionPlayerIndex == -1 || pmDriverSeat->m_vFront.Dot( pBot->MtxToWorld()->m_vFront ) > 0.0f )
		{
//			m_PhysObj.DrawSphere( (CFVec3A*) &StationCollSphere.m_Pos, StationCollSphere.m_fRadius );
//			m_PhysObj.DrawSphere( (CFVec3A*) &BotCollSphere.m_Pos, BotCollSphere.m_fRadius );
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
// if bImmediately is TRUE, bot is put into station with no delay, otherwise state machine
// does it over time (with jumping animation, etc.).
CVehicle::StationStatus_e CVehicleSentinel::EnterStation( CBot *pBot, Station_e eStation, BOOL bProximityCheck, BOOL bImmediately )
{
	StationStatus_e eResult;

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

	// nobody driving vehicle, allow bot to enter
	m_pDriverBot = pBot;
	if( bImmediately )
	{
		_StartDriverEnterWork();
		StartEngine();

		StartTransitionToDriverVehicleCamera( pBot->m_nPossessionPlayerIndex );
		m_DriverCameraTrans.SetTransitionSnap();
	}
	else
	{
		m_eSentinelState = VEHICLESENTINEL_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT;
	}
	return STATION_STATUS_ENTERING;
}

//-----------------------------------------------------------------------------
CVehicle::StationStatus_e CVehicleSentinel::ExitStation( CBot *pBot, BOOL bImmediately )
{
	if( m_pDriverBot == NULL )
	{
		return STATION_STATUS_EMPTY;
	}

	if( m_pDriverBot != pBot )
	{
		return STATION_STATUS_WRONG_BOT;
	}

	if( bImmediately )
	{
		if( m_pDriverBot && m_pDriverBot->IsCreated() )
		{
			if( m_pDriverBot->IsPlayerBot() )
			{
				gamecam_SwitchPlayerTo3rdPersonCamera( (GameCamPlayer_e)m_pDriverBot->m_nPossessionPlayerIndex, m_pDriverBot );
			}

			DriverExit( m_pDriverBot );
		}

		m_eSentinelState = VEHICLESENTINEL_STATE_UNOCCUPIED;
		m_DriverCameraTrans.SetCameraStateInactive();
	}
	else
	{
		m_eSentinelState = VEHICLESENTINEL_STATE_START_DRIVER_EXIT;
	}
	return STATION_STATUS_EXITING;
}

//-----------------------------------------------------------------------------
BOOL CVehicleSentinel::GetStationEntryPoint( Station_e eStation, CFVec3A *pPoint )
{
	if( eStation != STATION_DRIVER )
	{
		return FALSE;
	}

	pPoint->Set( m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach]->m_vPos );

	return TRUE;
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	m_uBotDeathFlags &= (~BOTDEATHFLAG_PLAYDEATHANIM);
	m_uBotDeathFlags |= BOTDEATHFLAG_AUTOPERSISTAFTERDEATH;

	CVehicle::ClassHierarchyAddToWorld();

	// snap antenna to new position
	m_Antenna.SetPos();
	AtRestMatrixPalette();
}



//-----------------------------------------------------------------------------
void CVehicleSentinel::ClassHierarchyRemoveFromWorld( void )
{
	s32 nIndex;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	StopEngine(TRUE);	// kill engine sounds immediately

	m_pWorldMesh->RemoveFromWorld();
	CVehicle::ClassHierarchyRemoveFromWorld();

	if( m_hSmokeParticle != FPARTICLE_INVALID_HANDLE )
	{
		fparticle_SetIntensity( m_hSmokeParticle, 0.0f );
		fparticle_EnableEmission( m_hSmokeParticle, FALSE );
	}

	for( nIndex = 0; nIndex < MAX_CLOUD_PARTICLES; nIndex++ )
	{
		m_fCloudParticleIntensity[nIndex] = 0.0f;
		if( m_hCloudParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hCloudParticle[nIndex], FALSE );
		}
	}

	for( nIndex = 0; nIndex < NUM_WHEELS; nIndex++ )
	{
		m_fDirtParticleIntensity[nIndex] = 0.0f;
		if( m_hDirtParticle[nIndex] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[nIndex], FALSE );
		}
	}


	ExitStation( m_pDriverBot, TRUE /*bImmediately*/ );
}


//-----------------------------------------------------------------------------
CFMtx43A *CVehicleSentinel::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName )
{
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL )
	{
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 )
	{
		DEVPRINTF( "CVehicleSentinel::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}



//-----------------------------------------------------------------------------
CEntityBuilder *CVehicleSentinel::GetLeafClassBuilder( void )
{
	return &_VehicleSentinelBuilder;
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::ClassHierarchyWork()
{
	CFVec3A TempVec3A;
	s32 nWheel;
	s32 nIndex;

	FASSERT( m_bSystemInitialized );

	CVehicle::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	// Save a copy of the previous frame's info...
	m_MountPrevPos_WS	= m_MountPos_WS;
	m_nPrevState		= m_nState;

	// force on wheels defaults to zero
	for( nWheel = 0; nWheel < NUM_WHEELS; nWheel++ )
	{
		m_aColPoints[nWheel].vAppliedForce.Zero();
		m_fDirtParticleIntensity[nWheel] = 0.0f;
		if( m_hDirtParticle[nWheel] != FPARTICLE_INVALID_HANDLE )
		{
			fparticle_EnableEmission( m_hDirtParticle[nWheel], FALSE );
		}
	}

	_UpdateDustCloud();

	ComputeGroundNormal();

	if( m_pDriverBot != NULL )
	{
		// There is a driver controlling the vehicle.

		// set debug pointer to physics object currently controlled by player
		_pDrivenPhysObj = &m_PhysObj;

		// check for driver request to exit vehicle (must occur before ParseControls())
		if( m_eSentinelState == VEHICLESENTINEL_STATE_DRIVING )
		{
			if( m_bControls_Human && Controls() &&
				((CHumanControl *) Controls())->m_nPadFlagsAction & GAMEPAD_BUTTON_1ST_PRESS_MASK )
			{
				ActionNearby(m_pDriverBot);
			}
		}

		// update the control inputs from the driver
		ParseControls();

		// send a sound radius to AI so brains can "hear" vehicle coming and try to dodge it
		if (m_pDriverBot->m_nPossessionPlayerIndex > -1)
		{
			f32 fSpeed = m_PhysObj.GetVelocity()->Mag();
			f32 fFakeMaxSpeed = 75.0f;
			FMATH_CLAMPMAX(fSpeed, fFakeMaxSpeed);	//some arbitrary max speed just used to approximate get a rough normalized speed
			AIEnviro_BoostPlayerSoundTo(m_pDriverBot->m_nPossessionPlayerIndex, 25.0f+ AIEnviro_fVehicleBotSoundRadius*fmath_Div(fSpeed, fFakeMaxSpeed));
		}
	}
	else
	{
		_pDrivenPhysObj = NULL;
	}

	UpdateSpotLight();
	MoveVehicle();
	HandleTargeting();
	_UpdateReticle();
	_UpdateCannon();
	_UpdateMachineGun();
	_UpdateTreads();
	_HandleWeaponWork();
	RunEngine();
	PlayCollisionSounds();


	m_pCurrentCollider = this;
	m_uSentinelFlags |= SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER;
	m_PhysObj.Simulate( FLoop_fPreviousLoopSecs );
	m_uSentinelFlags &= ~SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER;

	UpdateMatrices();

	// update velocity member variables
	m_Velocity_WS.Set( *(m_PhysObj.GetVelocity()) );
	CFMtx43A mInvOr;
	mInvOr.Set( *(m_PhysObj.GetOrientation()) );
	mInvOr.AffineInvert( FALSE );
	mInvOr.MulPoint( m_Velocity_MS, m_Velocity_WS );
	VelocityHasChanged();

	// handle bot squishing
	for( nIndex = 0; nIndex < _nBotsToSquish; nIndex++ )
	{
		SquishBot( _apBotsToSquish[nIndex] );
	}
	_nBotsToSquish = 0;

	_UpdateCollisionFlags();

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

	// turn on momentum clamping if on the ground,
	// turn off if airborne.
	if( SmoothContact() )
	{
		m_PhysObj.SetMomentumClamping( TRUE, 1200.0f, 3300.0f );
		m_PhysObj.SetAngularMomentumClamping( TRUE, 3600.0f, 10000.0f );
	}
	else
	{
		m_PhysObj.SetMomentumClamping( FALSE );
		m_PhysObj.SetAngularMomentumClamping( FALSE );
	}

	switch( m_eSentinelState )
	{
		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT:
			m_pDriverBot->AbortScopeMode( TRUE );
			StartTransitionToDriverVehicleCamera( m_pDriverBot->m_nPossessionPlayerIndex );

			{
				CFVec3A vJumpVel;
				m_fEntryJumpTimer = ComputeEntryTrajectory( m_pDriverBot->MtxToWorld()->m_vPos,					// start pos
										m_pWorldMesh->GetBoneMtxPalette()[m_nBoneIdxDriverAttach]->m_vPos,	// end pos
										3.0f,																	// minimum jump height, 
										m_pDriverBot->m_pBotInfo_Gen->fGravity * m_pDriverBot->m_fGravityMultiplier,	// gravity
										vJumpVel );																// output velocity

				m_pDriverBot->StartVelocityJump( &vJumpVel );
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
				m_pDriverBot->SetNoCollideStateAir( TRUE );
				m_eSentinelState = VEHICLESENTINEL_STATE_MOVE_DRIVER_TO_ENTRY_POINT;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_MOVE_DRIVER_TO_ENTRY_POINT:
			m_fEntryJumpTimer -= FLoop_fPreviousLoopSecs;

			if( IsDriverUseless( m_pDriverBot ) )
			{
				// abort entry if driver becomes unable to drive
				_AbortEntry();
			}
			else if( m_fEntryJumpTimer < 0.0f )
			{
				m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
				m_pDriverBot->SetNoCollideStateAir( FALSE );
				m_eSentinelState = VEHICLESENTINEL_STATE_START_DRIVER_ENTER;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_START_DRIVER_ENTER:
			_StartDriverEnterWork();
			CrankEngine();
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_ENGINE_STARTING:
			LocateDriverAtBone( m_nBoneIdxDriverAttach );

			if( EngineState() == ENGINE_STATE_RUNNING )
			{
//				SetSpotLightOn();
				m_eSentinelState = VEHICLESENTINEL_STATE_DRIVER_ENTERING;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_DRIVER_ENTERING:
			m_eSentinelState = VEHICLESENTINEL_STATE_DRIVING;
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_DRIVING:
			SetVehicleReticle( m_pDriverBot, CReticle::TYPE_TANK_CANNON, _RETICLE_NORM_X, _RETICLE_NORM_Y );

			LocateDriverAtBone( m_nBoneIdxDriverAttach );
			if( m_pDriverBot && IsDriverUseless(m_pDriverBot) )
			{
				m_eSentinelState = VEHICLESENTINEL_STATE_START_DRIVER_EXIT;
			}

			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_START_DRIVER_EXIT:
			LocateDriverAtBone( m_nBoneIdxDriverAttach );
			if( m_nPossessionPlayerIndex >= 0 )
			{
				StartTransitionToDriverBotCamera();
			}
			StopEngine();
//			SetSpotLightOff();
			m_eSentinelState = VEHICLESENTINEL_STATE_DRIVER_EXITING;
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_DRIVER_EXITING:
			LocateDriverAtBone( m_nBoneIdxDriverAttach );
			if( m_DriverCameraTrans.IsCameraInactive() )
			{
				// wait for camera to finish before exiting
				DriverExit( m_pDriverBot );
				m_eSentinelState = VEHICLESENTINEL_STATE_ENGINE_STOPPING;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_ENGINE_STOPPING:
			if( EngineState() == ENGINE_STATE_STOPPED )
			{
				m_eSentinelState = VEHICLESENTINEL_STATE_UNOCCUPIED;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_START_UPSIDE_DOWN:
			// move driver bot to a safer exit point above vehicle
			if( m_pDriverBot )	// should always be true
			{
				CFMtx43A mSafeLoc;
				mSafeLoc.Set( *m_pDriverBot->MtxToWorld() );
				mSafeLoc.m_vPos.y += 15.0f;
				m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS( &mSafeLoc );

				// note: this check must occur before DriverExit(), which clears m_nPossessPlayerIndex
				if( m_nPossessionPlayerIndex >= 0 )
				{
					StartTransitionToDriverBotCamera();
				}

				DriverExit( m_pDriverBot );
			}

			StopEngine();
//			SetSpotLightOff();
			m_eSentinelState = VEHICLESENTINEL_STATE_UPSIDE_DOWN;
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_UPSIDE_DOWN:
			if( SmoothContact() &&														// vehicle touching ground
				m_PhysObj.GetAngularVelocity()->Mag() < 4.0f &&							// not rotating rapidly
				m_PhysObj.GetOrientation()->m_vUp.Dot( CFVec3A::m_UnitAxisY ) > 0.0f )	// is rightside Up
			{
				// vehicle was somehow righted by external forces...
				m_eSentinelState = VEHICLESENTINEL_STATE_UNOCCUPIED;
			}

			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_START_FLIPPING_UPRIGHT:
			{
				const CFMtx43A *pmPhysObjOr = m_PhysObj.GetOrientation();
				CFVec3A vCross;

				m_fFlipOverTime = _FLIP_TIME;

				vCross.Cross( pmPhysObjOr->m_vUp, CFVec3A::m_UnitAxisY );
				if( vCross.Dot( pmPhysObjOr->m_vFront ) < 0.0f )
				{
					m_fFlipArm = _FLIP_MOMENT_ARM;
				}
				else
				{
					m_fFlipArm = -_FLIP_MOMENT_ARM;
				}
			}

			m_eSentinelState = VEHICLESENTINEL_STATE_FLIPPING_UPRIGHT;
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_FLIPPING_UPRIGHT:
			if( m_fFlipOverTime <= 0.0f )
			{
				m_eSentinelState = VEHICLESENTINEL_STATE_UNOCCUPIED;
			}
			break;

		//---------------------------------------------------------------------
		case VEHICLESENTINEL_STATE_UNOCCUPIED:
			if( SmoothContact() &&														// vehicle touching ground
				m_PhysObj.GetAngularVelocity()->Mag() < 4.0f &&							// not rotating rapidly
				m_PhysObj.GetOrientation()->m_vUp.Dot( CFVec3A::m_UnitAxisY ) < 0.0f )	// is rightside Up
			{
				// vehicle was somehow flipped upside down by external forces...
				m_eSentinelState = VEHICLESENTINEL_STATE_UPSIDE_DOWN;
			}

		default:
			break;
	}

	UpdateCamera();

	m_Antenna.Work();

#if _ADJUST_SPHERES
	_bDrewSpheres = FALSE;
	gamepad_AxisSlider( _vAdjust.x, _vAdjust.x, 0.0f, 20000.0f, 0.0001f, 0.25f, GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_X, 1 );
	gamepad_AxisSlider( _vAdjust.y, _vAdjust.y, -10.0f, 20.0f, 0.0001f, 0.25f, GAMEPAD_SLIDER_AXIS_LEFT_ANALOG_Y, 1 );
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
		m_aSentinelColPoints[CVehicleSentinel::NUM_WHEELS+_nGroupStart[_nGroup]].fRadius = _fAdjust;
		m_aSentinelColPoints[CVehicleSentinel::NUM_WHEELS+_nGroupStart[_nGroup]+1].fRadius = _fAdjust;
	}
	else
	{
		_afBodySpherePos[_nGroupStart[_nGroup]][0] = _vAdjust.x;
		_afBodySpherePos[_nGroupStart[_nGroup]][1] = _vAdjust.y;
		_afBodySpherePos[_nGroupStart[_nGroup]][2] = _vAdjust.z;
		_afBodySphereRadius[_nGroupStart[_nGroup]] = _fAdjust;
		m_aSentinelColPoints[CVehicleSentinel::NUM_WHEELS+_nGroupStart[_nGroup]].fRadius = _fAdjust;
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

#endif	// !FANG_PRODUCTION_BUILD
}

//-----------------------------------------------------------------------------
#define _WHEEL_KINETIC_VEL_MUL			( 2.5f )
#define _DIRT_KINETIC_INTENSITY_MUL		( 2.0f )
#define _MIN_DIRT_KINETIC_INTENSITY		( 0.4f )
#define _MIN_DIRT_INTENSITY				( 0.4f )
#define _MIN_DIRT_VEL					( 20.0f )
#define _MIN_DIRT_VEL_SQ				( _MIN_DIRT_VEL * _MIN_DIRT_VEL )
#define _MAX_DIRT_VEL					( 75.0f )
#define _MAX_DIRT_VEL_SQ				( _MAX_DIRT_VEL * _MAX_DIRT_VEL )
#define _MAX_ROOSTER_VEL				( 25.0f )
#define _DIRT_EMISSION_ANGLE			( 30.0f )
void CVehicleSentinel::AnimateWheels( u32 uWheelIndex, u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFVec3A vVelAtTire;
	CFVec3A vPoint;
	f32 fRollingVel;

	// compute the matrix of the tire, before rolling is applied.
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( m_uSentinelFlags & SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER )
	{
		return;
	}

	// find velocity at wheel
	vPoint.Set( rNewMtx.m_vPos );
	vPoint.Sub( *m_PhysObj.GetPosition() );
	m_PhysObj.GetVelocity( &vPoint, &vVelAtTire );

	// compute rotational velocity of tire in radians per second
	fRollingVel = vVelAtTire.Dot( rNewMtx.m_vFront );

	fRollingVel *= _afOOWheelRadius[uWheelIndex];

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
}

#define _TURRET_RECOIL_TIME		( 0.1f )
#define _TURRET_RECOVER_TIME	( 0.4f )
//-----------------------------------------------------------------------------
void CVehicleSentinel::AnimateTurret( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFMtx43A mRot, mTemp;

	mTemp.Mul( rParentMtx, rBoneMtx );

	// find unit vector representing WS XZ rotation of turret
	mRot.Identity();
	mRot.SetRotationY( m_fTurretAngleWS );

	// init Turret matrix and set translation component
	rNewMtx.Identity();
	rNewMtx.m_vPos.Set( mTemp.m_vPos );

	// construct Turret rotation matrix based on WS XZ direction of turret.
	rNewMtx.m_vFront.ReceivePlanarProjection( mRot.m_vFront, rParentMtx.m_vUp );
	rNewMtx.m_vFront.Unitize();
	rNewMtx.m_vRight.Cross( rParentMtx.m_vUp, rNewMtx.m_vFront );
	rNewMtx.m_vRight.Unitize();
	rNewMtx.m_vUp.Set( rParentMtx.m_vUp );

	// recoil turret when cannon has fired
	f32 fReloadTime = _CANNON_RELOAD_TIME - m_fCannonReloadTime;
	if( fReloadTime < _TURRET_RECOIL_TIME )
	{
		m_vTurretRecoil.Set( rNewMtx.m_vFront );
		m_vTurretRecoil.Mul( -( fReloadTime * 16.0f ) );
		rNewMtx.m_vPos.Add( m_vTurretRecoil );
	}
	else if( (fReloadTime - _TURRET_RECOIL_TIME) < _TURRET_RECOVER_TIME )
	{
		m_vTurretRecoil.Set( rNewMtx.m_vFront );
		m_vTurretRecoil.Mul( -( ( _TURRET_RECOVER_TIME - (fReloadTime - _TURRET_RECOIL_TIME) ) * 4.0f ) );
		rNewMtx.m_vPos.Add( m_vTurretRecoil );
	}
	else
	{
		m_vTurretRecoil.Zero();
	}
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::AnimateCannon( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFVec3A vTemp;

	// compute the basic matrix of the Cannon base
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	// recoil cannon barrel when fired
	f32 fReloadTime = _CANNON_RELOAD_TIME - m_fCannonReloadTime;

	if( fReloadTime < _TURRET_RECOIL_TIME )
	{
		vTemp.Set( rNewMtx.m_vFront );
		vTemp.Mul( -( fReloadTime * 8.0f ) );
		rNewMtx.m_vPos.Add( vTemp );
	}
	else if( (fReloadTime - _TURRET_RECOIL_TIME) < _TURRET_RECOVER_TIME )
	{
		vTemp.Set( rNewMtx.m_vFront );
		vTemp.Mul( -( ( _TURRET_RECOVER_TIME - (fReloadTime - _TURRET_RECOIL_TIME) ) * 2.0f ) );
		rNewMtx.m_vPos.Add( vTemp );
	}
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::AnimateCannonBase( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFMtx43A mRot;
	f32 fElev;

	// compute the basic matrix of the Cannon base
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	fElev = m_fCannonElevation;
	if( fElev > _CANNON_MAX_VIS_DOWN_ELEV )
	{
		fElev = _CANNON_MAX_VIS_DOWN_ELEV;
	}

	// elevate cannon by current angle
	mRot.Identity();
	mRot.SetRotationX( fElev );
	rNewMtx.Mul( mRot );
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::AnimateSpotLight( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	CFMtx43A mRot;

	// compute the basic matrix of the Spot light
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	// elevate spot light by current angle
	mRot.Identity();
	mRot.SetRotationX( m_fCannonElevation );
	rNewMtx.Mul( mRot );
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_AnimBoneCallback( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) 
{
	CVehicleSentinel *pSentinel;
	pSentinel = (CVehicleSentinel *) m_pCollBot;

	if( pSentinel->m_pPartMgr->IsCreated() && !( pSentinel->m_uSentinelFlags & SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER ) )
	{
		if( pSentinel->m_pPartMgr->AnimBoneCallbackFunctionHandler( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) )
		{
			return;
		}
	}

	if( !( pSentinel->m_uSentinelFlags & SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER ) )
	{
		if( pSentinel->m_Antenna.AnimateBone( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) )
		{
			return;
		}
	}


#if 1
	// find which bone this callback was called for, and invoke the corresponding animation function
	for( u32 uWheelIndex = 0; uWheelIndex < CVehicleSentinel::NUM_ALL_WHEELS; uWheelIndex++ )
	{
		if( uBoneIndex == pSentinel->m_anBoneIndexWheel[uWheelIndex] )
		{
			pSentinel->AnimateWheels( uWheelIndex, uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
			return;
		}
	}
#endif

	if( uBoneIndex == pSentinel->m_nBoneIndexTurret )
	{
		pSentinel->AnimateTurret( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
	}
	else if( uBoneIndex == pSentinel->m_nBoneIndexCannonBase )
	{
		pSentinel->AnimateCannonBase( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
	}
	else if( uBoneIndex == pSentinel->m_nBoneIndexCannon )
	{
		pSentinel->AnimateCannon( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
	}
	else if( uBoneIndex == pSentinel->m_nBoneIndexSpotLight )
	{
		pSentinel->AnimateSpotLight( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx );
	}
}



//-----------------------------------------------------------------------------
void CVehicleSentinel::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
	if( m_pDriverBot )
	{
		m_pDriverBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}


//-----------------------------------------------------------------------------
const CFVec3A *CVehicleSentinel::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT( IsCreated() );
	FASSERT( nTagPointIndex < m_nTagPointCount );

	return m_apTagPoint_WS[nTagPointIndex];
}

//-----------------------------------------------------------------------------
const CFVec3A *CVehicleSentinel::GetApproxEyePoint( void ) const
{
	FASSERT( IsCreated() );

	return m_pApproxEyePoint_WS;
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_HandleWeaponFiring( void )
{
}

//-----------------------------------------------------------------------------
void CVehicleSentinel::_HandleWeaponWork( void )
{
}

//-----------------------------------------------------------------------------
// Call when m_Velocity_WS and m_Velocity_MS have changed.
void CVehicleSentinel::VelocityHasChanged( void )
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
void CVehicleSentinel::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	pApproxMuzzlePoint_WS->Set( *m_apTagPoint_WS[TAG_POINT_INDEX_PRIMARY_FIRE] );
}


//-----------------------------------------------------------------------------
f32 CVehicleSentinel::ComputeEstimatedControlledStopTimeXZ( void )
{
	return m_fUnitSpeed_WS * 2.0f;
}


//-----------------------------------------------------------------------------
void CVehicleSentinel::CheckpointSaveSelect( s32 nCheckpoint )
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

//-----------------------------------------------------------------------------
// saves state for checkpoint
BOOL CVehicleSentinel::CheckpointSave( void )
{
	// save parent class data
	CVehicle::CheckpointSave();

	CFCheckPoint::SaveData( (u32&) m_pDriverBot );
	CFCheckPoint::SaveData( m_fTurretAngleWS );
	CFCheckPoint::SaveData( m_fTurretUnitRot );
	CFCheckPoint::SaveData( m_fCannonElevation );

	return TRUE;
}

//-----------------------------------------------------------------------------
// loads state for checkpoint
void CVehicleSentinel::CheckpointRestore( void )
{
	// since AnimateAxles() is called by CBot::CheckpointRestore(), must set
	// m_pCollBot pointer.
	m_pCollBot = this;

	// load parent class data
	CVehicle::CheckpointRestore();

	CFCheckPoint::LoadData( (u32&) m_pDriverBot );
	CFCheckPoint::LoadData( m_fTurretAngleWS );
	CFCheckPoint::LoadData( m_fTurretUnitRot );
	CFCheckPoint::LoadData( m_fCannonElevation );

	_InitPhysicsObject();

	if( m_pPartMgr->IsCreated() )
	{
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_A_LEFT_FRONT );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_B_LEFT_REAR );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_C_RIGHT_FRONT );
		m_pPartMgr->SetState_Dangle( LIMB_TYPE_BOX_D_RIGHT_REAR );
	}
}

void CVehicleSentinel::CheckpointRestorePostamble( void )
{
	CVehicle::CheckpointRestorePostamble();

	if( m_pDriverBot )
	{
		CBot *pBot;
		pBot = m_pDriverBot;
		m_pDriverBot = NULL;
		EnterStation( pBot, STATION_DRIVER, FALSE /*bProximityCheck*/, TRUE /*bImmediately*/ );
	}
}

