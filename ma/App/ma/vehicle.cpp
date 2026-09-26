//////////////////////////////////////////////////////////////////////////////////////
// vehicle.cpp -
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
// 11/07/02 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fresload.h"
#include "fscriptsystem.h"
#include "fworld_coll.h"
#include "fsound.h"
#include "vehicle.h"
#include "splat.h"
#include "hud2.h"
#include "gamecam.h"
#include "botpart.h"
#include "AI\AIBrainman.h"
#include "AI\AIGroup.h"
#include "meshtypes.h"
#include "SplitScreen.h"
#include "player.h"
#include "MultiplayerMgr.h"

#if FANG_PLATFORM_XB
extern BOOL fxbforce_DecayCoarse( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
extern BOOL fxbforce_DecayFine( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
#endif

// collision sfx
#define _HARD_COLLISION_SFX_VOLUME		( 1.0f )
#define _SOFT_COLLISION_SFX_VOLUME		( 1.0f )
#define _COLLISION_SFX_RADIUS			( 100.0f )

#define _MIN_SOFT_COLLISION_MOMENTUM	( 1000.0f * 10.0f )
#define _MIN_HARD_COLLISION_MOMENTUM	( 1000.0f * 100.0f )
#define _MIN_HARD_HIT_GROUND_MOMENTUM	( 1000.0f * 6.0f )

#define _MAX_RUMBLE_HEIGHT				( 50.0f )
#define _MID_RUMBLE_HEIGHT				( 30.0f )
#define _MIN_RUMBLE_HEIGHT				( 15.0f )

static void _SetFOV( s32 nIndex, f32 fFOV );

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicleBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CVehicleBuilder _VehicleBuilder;


void CVehicleBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) 
{
	m_uFlags = 0;

	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_VEHICLE, pszEntityType );
}


BOOL CVehicleBuilder::PostInterpretFixup( void ) 
{
	if( !CBotBuilder::PostInterpretFixup() ) 
	{
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:
	FASSERT_NOW;
	return FALSE;
}


BOOL CVehicleBuilder::InterpretTable( void ) 
{
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "No_Player_Drive" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, VEHICLE_BUILDER_FLAG_NO_PLAYER_DRIVE );
		return TRUE;
	}
	else if ( !fclib_stricmp( CEntityParser::m_pszTableName, "No_Entity_Drive" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, VEHICLE_BUILDER_FLAG_NO_ENTITY_DRIVE );
		return TRUE;
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DetPackOnly" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, VEHICLE_BUILDER_FLAG_DET_PACK_DESTROY );
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DriverExit" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, VEHICLE_BUILDER_FLAG_DRIVER_EXIT );
		m_uFlags |= VEHICLE_BUILDER_FLAG_DRIVER_EXIT_SELECT;	// remember that this user prop was specified
	}
	else if( !fclib_stricmp( CEntityParser::m_pszTableName, "GunnerExit" ) )
	{
		CEntityParser::Interpret_Flag( &m_uFlags, VEHICLE_BUILDER_FLAG_GUNNER_EXIT );
		m_uFlags |= VEHICLE_BUILDER_FLAG_GUNNER_EXIT_SELECT;	// remember that this user prop was specified
	}


	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CVehicle
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

#define _DRAW_BOUNDING_SPHERE			( 0 )	// draw the vehicle's bounding sphere (fCollSphere1)

#define _CAM_FOV						( 42.5f )

//-----------------------------------------------------------------------------
// static member variables
//-----------------------------------------------------------------------------

BOOL CVehicle::m_bSystemInitialized = FALSE;
u32 CVehicle::m_nBotClassClientCount = 0;

//-----------------------------------------------------------------------------
// system functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CVehicle::InitSystem( void )
{
//	FResFrame_t ResFrame;

	FASSERT( !m_bSystemInitialized );

//	ResFrame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	return TRUE;

	// Error:
//_ExitInitSystemWithError:
//	UninitSystem();
//	fres_ReleaseFrame( ResFrame );
//	return FALSE;
}


//-----------------------------------------------------------------------------
void CVehicle::UninitSystem( void )
{
	if( m_bSystemInitialized )
	{
		m_bSystemInitialized = FALSE;
	}
}


//-----------------------------------------------------------------------------
BOOL CVehicle::ClassHierarchyLoadSharedResources( void )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	++m_nBotClassClientCount;

	if( !CBot::ClassHierarchyLoadSharedResources() )
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

//	FResFrame_t ResFrame = fres_GetFrame();

	return TRUE;


//_ExitWithError:
//	ClassHierarchyUnloadSharedResources();
//	fres_ReleaseFrame( ResFrame );
//	return FALSE;
}


//-----------------------------------------------------------------------------
void CVehicle::ClassHierarchyUnloadSharedResources( void )
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	CBot::ClassHierarchyUnloadSharedResources();
}



//-----------------------------------------------------------------------------
// private member functions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void CVehicle::_ClearDataMembers( void )
{
	m_uPreviousHudFlags	= 0;
	m_CamInfo.m_fHalfFOV = FMATH_DEG2RAD( _CAM_FOV );
	m_CamInfo.m_pmtxMtx = &m_mCamera;
	m_DriverCameraTrans.Init( &m_CamInfo, 1.0f /* transition length */ );
	m_pEngineIdleEmitter = NULL;
	m_pEngineLowEmitter = NULL;
	m_pEngineHighEmitter = NULL;
	m_pSkidEmitter = NULL;
	m_fEngineTime = 0.0f;
	m_fThrottle = 0.0f;
	m_fBrake = 0.0f;
	m_fEngineSpeed = 0.0f;
	m_fEngineVariance = 1.0f;
	m_fUnitSpeed_WS = 0.0f;
	m_uVehicleFlags = 0;
	m_vGroundNormal.Set( CFVec3A::m_UnitAxisY );
	m_qGroundNormal.Identity();
	m_vCMOffset.Zero();
	m_pGazeDir_WS = NULL;
	m_pDriverBot = NULL;
	m_pSplinePath = NULL;
	m_pRecentCollisionMaterial = NULL;
	m_fRecentCollisionMagnitude = 0.0f;
	m_fHeightAboveGround = 0.0f;
	m_pPreviousDriverArmorProfile = NULL;
	fforce_NullHandle( &m_hForce );
	m_fSoundFxRadiusMultiplier = 1.0f;
	SaveDriverReticle( NULL );	// set reticle defaults
	m_fTimeSinceLastContact = 0.0f;
	m_fTimeSinceLastCollide = 0.0f;
	m_fTimeSinceLastHitGround = 0.0f;
	m_fEngineMasterVolume = 1.0f;
}

//-----------------------------------------------------------------------------
// protected member functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// put driver bot into vehicle
void CVehicle::DriverEnter( CBot *pDriverBot, cchar *pszAttachPointBoneName /* = NULL */, BOOL bImmediately /* = FALSE */ )
{
	if( pDriverBot == NULL )
	{
		return;
	}

	// attach bot to vehicle
//	pDriverBot->Attach_ToParent_WS( this, pszAttachPointBoneName );
	pDriverBot->Attach_ToParent_WithGlue_WS( this, pszAttachPointBoneName );

	SaveDriverReticle( pDriverBot );

	if( pDriverBot->m_nPossessionPlayerIndex >= 0 )
	{
		// transfer driving player bot's controls to vehicle
		pDriverBot->Controls()->Zero();
		SetControls( pDriverBot->Controls() );
		pDriverBot->SetControls( NULL );

		// set up the HUD
		CHud2* pHud = CHud2::GetHudForPlayer(pDriverBot->m_nPossessionPlayerIndex);
		m_uPreviousHudFlags = pHud->SetDrawFlags( CHud2::DRAW_ENABLEOVERRIDE | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR );
	}

	
	pDriverBot->EnteringMechWork(this);


	// set vehicle's index to that of driver bot
	if( !GetGunnerBot() || !GetGunnerBot()->IsPlayerBot() )
	{
		m_nPossessionPlayerIndex = pDriverBot->m_nPossessionPlayerIndex;
	}

	if( pDriverBot->m_nPossessionPlayerIndex >= 0 )
	{
		//when players take "possession" of bots with brains, call this
		//because the brain will be repurposed during that time.
		//see exit vehicle for the undoing of this.
		aibrainman_ConfigurePlayerBotBrain( AIBrain(), m_nPossessionPlayerIndex );	 //turns this bot's brain into a player brain
	}
	else
	{
		aibrainman_ConfigureNPCVehicleBrain( AIBrain() , pDriverBot->AIBrain());
	}

	pDriverBot->ZeroVelocity();
	pDriverBot->ImmobilizeBot();

	// remember if driver's light was on, then turn it off while in vehicle
	if( pDriverBot->IsSpotLightOn() )
	{
		m_uVehicleFlags |= VEHICLE_FLAG_DRIVER_LIGHT_ON;
		pDriverBot->SetSpotLightOff(TRUE);
	}
	else
	{
		m_uVehicleFlags &= ~VEHICLE_FLAG_DRIVER_LIGHT_ON;
	}

	// remember if driver was casting a shadow, then turn it off while in vehicle
	if( pDriverBot->m_pWorldMesh->m_nFlags & FMESHINST_FLAG_CAST_SHADOWS )
	{
		m_uVehicleFlags |= VEHICLE_FLAG_DRIVER_HAS_SHADOW;
		pDriverBot->m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;
	}
	else
	{
		m_uVehicleFlags &= ~VEHICLE_FLAG_DRIVER_HAS_SHADOW;
	}

	// tell the bot what vehicle he is driving
	pDriverBot->m_pDrivingVehicle = this;

	if( pDriverBot->m_nPossessionPlayerIndex >= 0 )
	{
		// trigger a "player entered vehicle" script event
		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("vehicle"), 
									 pDriverBot->m_nPossessionPlayerIndex, (u32) this, TRUE);
	}

	FASSERT( m_pPreviousDriverArmorProfile == NULL );
	m_pPreviousDriverArmorProfile = pDriverBot->GetArmorProfile();

	if( level_IsRacingLevel() )
	{
		pDriverBot->SetArmorProfile( m_pBotInfo_Vehicle->pOccupantRacingArmorProfile );
	}
	else
	{
		pDriverBot->SetArmorProfile( m_pBotInfo_Vehicle->pOccupantArmorProfile );
	}
}

//-----------------------------------------------------------------------------
// remove driver bot from vehicle
void CVehicle::DriverExit( CBot *pDriverBot )
{
	CFVec3A vOffset;
	
	if( pDriverBot == NULL)
	{
		return;
	}

	UnSetVehicleReticle( pDriverBot );

	if( !pDriverBot->IsImmobilizePending() )
	{
		// don't mobilize if an immobilize is pending (EMP grenade for instance)
		pDriverBot->MobilizeBot();
	}

	// if driver's spotlight was on when entering vehicle, turn it back on now.
	if( m_uVehicleFlags & VEHICLE_FLAG_DRIVER_LIGHT_ON )
	{
		pDriverBot->SetSpotLightOn();
	}

	// if driver was casting a shadow before entering vehicle, turn it back on now.
	if( m_uVehicleFlags & VEHICLE_FLAG_DRIVER_HAS_SHADOW )
	{
		pDriverBot->m_pWorldMesh->m_nFlags |= FMESHINST_FLAG_CAST_SHADOWS;
	}

	//give driver bot class a chance to cleanup anims and stuff
	pDriverBot->ExitingMechWork();
	
	//if the driver has an AIBrain, tell him he has been ejected from this vehicle
	if (pDriverBot->AIBrain())
	{
		aibrainman_NotifyMechEject(pDriverBot->AIBrain());
	}

	if( pDriverBot->m_nPossessionPlayerIndex >= 0)
	{
		// restore the HUD settings
//		CHud2* pHud = CHud2::GetHudForPlayer(m_nPossessionPlayerIndex);
//		pHud->SetDrawFlags( m_uPreviousHudFlags );

		CHud2* pHud = CHud2::GetHudForPlayer( pDriverBot->m_nPossessionPlayerIndex );
		CBot::SetHudMode( pHud, pDriverBot->m_pBotDef );

		m_uPreviousHudFlags = 0;

		// give controls back to driving player bot and NULL vehicle controls
		pDriverBot->SetControls( Controls() );
		pDriverBot->Controls()->Zero();
		SetControls( NULL );

		// trigger a "player exited vehicle" script event
		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("vehicle"), 
									 pDriverBot->m_nPossessionPlayerIndex, (u32) this, FALSE);

		//put the vehicles brain back to normal, now that "player possession" is done.
		aibrainman_ConfigurePostNPCVehicleBrain(AIBrain());
	}
	else
	{
		//put the vehicles brain back to normal, now that "bot possession" is done.
		aibrainman_ConfigurePostNPCVehicleBrain(AIBrain());	//Normal for a vehicle is set by this function
	}


	if( GetGunnerBot() )
	{
		m_nPossessionPlayerIndex = GetGunnerBot()->m_nPossessionPlayerIndex;
	}
	else
	{
		m_nPossessionPlayerIndex = -1;
	}

	pDriverBot->m_pDrivingVehicle = NULL;

	//FASSERT( m_pPreviousDriverArmorProfile != NULL );
	if( m_pPreviousDriverArmorProfile ) {
		pDriverBot->SetArmorProfile( m_pPreviousDriverArmorProfile );
		m_pPreviousDriverArmorProfile = NULL;
	}
}

//-----------------------------------------------------------------------------
CVehicle::Station_e CVehicle::WhichStationOccupied( CBot *pBot )
{
	if( pBot == GetDriverBot() )
	{
		return STATION_DRIVER;
	}
	else if( pBot == GetGunnerBot() )
	{
		return STATION_GUNNER;
	}
	else
	{
		return STATION_NONE;
	}
}

//-----------------------------------------------------------------------------
// save driver's original reticle information
BOOL CVehicle::SaveDriverReticle( CBot *pDrivingBot )
{
	m_fReticlePrevX		= 0.0f;
	m_fReticlePrevY		= 0.0f;
	m_bReticleDrawPrev	= FALSE;
	m_ReticlePrevType	= CReticle::TYPE_DROID_STANDARD;

	if( !pDrivingBot || !pDrivingBot->IsPlayerBot() )
	{
		return FALSE;
	}

	CReticle *pReticle = &(Player_aPlayer[ pDrivingBot->m_nPossessionPlayerIndex ].m_Reticle);

	if( !pReticle )
	{
		return FALSE;
	}

	m_fReticlePrevX		= pReticle->GetNormOriginX();
	m_fReticlePrevY		= pReticle->GetNormOriginY();
	m_bReticleDrawPrev	= pReticle->IsDrawEnabled();
	m_ReticlePrevType	= pReticle->GetType();
	return TRUE;
}

//-----------------------------------------------------------------------------
// When appropriate, sets pDrivingBot's reticle type to eType at fXPos, fYPos.
// Okay to call periodically (i.e. while waiting for weapon switch to complete).
BOOL CVehicle::SetVehicleReticle( CBot *pDrivingBot, CReticle::Type_e eType, f32 fXPos, f32 fYPos )
{
	if( !pDrivingBot || !pDrivingBot->IsPlayerBot() )
	{
		return FALSE;
	}

	CReticle *pReticle = &(Player_aPlayer[ pDrivingBot->m_nPossessionPlayerIndex ].m_Reticle);
	
	if( !m_pDriverBot ||						// don't set reticle if no driver
		m_pDriverBot->SwitchingWeapons() ||		// don't set if switching weapons
		!IsPlayerBot() ||						// don't set if vehicle is AI controlled
		!pReticle ||							// don't set if no pointer to reticle
		(pReticle->IsDrawEnabled() && pReticle->GetType() == eType ) )	// don't set if already set and visible
	{
		return FALSE;
	}

	// note that this code may not get executed if driver's reticle matches vehicle reticle (eType).
	pReticle->SetNormOrigin( fXPos, fYPos );
	pReticle->SetType( eType );
	pReticle->EnableDraw( TRUE );

	return TRUE;
}

//-----------------------------------------------------------------------------
// restores the driver's original reticle if appropriate. (Glitch will get his
// reticle back after he switches weapons.)
void CVehicle::UnSetVehicleReticle( CBot *pDrivingBot )
{
	if( !pDrivingBot || !pDrivingBot->IsPlayerBot() )
	{
		return;
	}

	if( pDrivingBot->TypeBits() & ENTITY_BIT_BOTGLITCH )
	{
		// Glitch will get his reticle back after he switches weapons.
		return;
	}

	CReticle *pReticle = &(Player_aPlayer[ pDrivingBot->m_nPossessionPlayerIndex ].m_Reticle);

	if( !pReticle )
	{
		return;
	}

	pReticle->SetNormOrigin( m_fReticlePrevX, m_fReticlePrevY );
	pReticle->SetType( m_ReticlePrevType );
	pReticle->EnableDraw( m_bReticleDrawPrev );
}

//-----------------------------------------------------------------------------
#define _FIND_GROUND_RAY_LENGTH		( 50.0f )
#define _GROUND_NORMAL_SLERP_RATE	( 3.0f )
// find the normal of the surface underneath the vehicle,
// smooth it and save it to member vector and quaternion.
void CVehicle::ComputeGroundNormal( void )
{
	CFQuatA qNormal;
	CFVec3A vStart, vEnd, vGroundNormal;
	FCollImpact_t Impact;
	f32 fSlerp;

	// set start and end point of a downward ray
	vStart.Set( *m_PhysObj.GetPosition() );
	vEnd.Set( vStart );
	vEnd.y -= _FIND_GROUND_RAY_LENGTH;

	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	// find intersection beneath vehicle
	fcoll_Clear();
	fworld_FindClosestImpactPointToRayStart( &Impact, &vStart, &vEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList  );

	m_fHeightAboveGround = _FIND_GROUND_RAY_LENGTH;

	// set the target unit vector
	if( FColl_nImpactCount > 0 )
	{
		if( Impact.UnitFaceNormal.Dot( CFVec3A::m_UnitAxisY ) > 0.0f )
		{
			vGroundNormal.Set( Impact.UnitFaceNormal );

			CFVec3A vDelta;
			vDelta.Set( Impact.ImpactPoint );
			vDelta.Sub( *m_PhysObj.GetPosition() );
			m_fHeightAboveGround = vDelta.Mag();
		}
	}
	else
	{
		vGroundNormal.Set( CFVec3A::m_UnitAxisY );
	}

	if( BecameAirborne() )
	{
		m_fMaxHeightAboveGround = 0.0f;
	}

	if( !SmoothContact() )
	{
		if( m_fHeightAboveGround > m_fMaxHeightAboveGround )
		{
			m_fMaxHeightAboveGround = m_fHeightAboveGround;
		}
	}

	// build a quaternion representing smoothed rotation from y axis to current ground normal
	qNormal.BuildQuat( CFVec3A::m_UnitAxisY, vGroundNormal );

	fSlerp = _GROUND_NORMAL_SLERP_RATE * FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMAX( fSlerp, 1.0f );	// clamp in case we have a very long frame

	qNormal.ReceiveSlerpOf( fSlerp, m_qGroundNormal, qNormal );
	m_qGroundNormal.Set( qNormal );

	// make a smoothed ground normal vector from quaternion
	m_qGroundNormal.MulPoint( m_vGroundNormal, CFVec3A::m_UnitAxisY );
}

//-----------------------------------------------------------------------------
void CVehicle::UpdateMatrices( void )
{
	CFMtx43A EntityMtxToWorld;
	CFVec3A vCMOffset;

	const CFMtx43A *pRot = m_PhysObj.GetOrientation();

	m_MountPrevPos_WS = m_MountPos_WS;

	// copy physics object's position and orientation to vehicle's world mesh.
	pRot->MulPoint( vCMOffset, m_vCMOffset );
	EntityMtxToWorld.m_vX = pRot->m_vX;
	EntityMtxToWorld.m_vY = pRot->m_vY;
	EntityMtxToWorld.m_vZ = pRot->m_vZ;
	EntityMtxToWorld.m_vPos = *m_PhysObj.GetPosition();
	EntityMtxToWorld.m_vPos.Sub( vCMOffset );
	m_pWorldMesh->m_Xfm.BuildFromMtx( EntityMtxToWorld );
	m_pWorldMesh->UpdateTracker();

	// Update bone matrix palette...
	ComputeMtxPalette( TRUE );

	m_GazeUnitVec_WS.ReceiveUnit( *m_pGazeDir_WS );

	// copy physics object's position and orientation to that of vehicle's
	Relocate_RotXlatFromUnitMtx_WS( &EntityMtxToWorld, TRUE, m_pMoveIdentifier );
	m_MountPos_WS = m_MtxToWorld.m_vPos;

	//update the mount yaw
//	m_MountUnitFrontXZ_WS.ReceiveUnitXZ( m_MtxToWorld.m_vFront );
//	m_fMountYaw_WS			= fmath_Atan( m_MountUnitFrontXZ_WS.x, m_MountUnitFrontXZ_WS.z );
//	m_fMountYawSin_WS		= m_MountUnitFrontXZ_WS.x;
//	m_fMountYawCos_WS		= m_MountUnitFrontXZ_WS.z;
}

//-----------------------------------------------------------------------------
void CVehicle::LocateDriverAtBone( s32 nBoneIndex )
{
	if( m_pDriverBot == NULL ) 
	{
		return;
	}

	m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS( m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex] );
}


//-----------------------------------------------------------------------------
// public member functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CVehicle::CVehicle() : CBot()
{
}


//-----------------------------------------------------------------------------
CVehicle::~CVehicle()
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
BOOL CVehicle::Create( const CBotDef *pBotDef, s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get pointer to the leaf class's builder object...
	CVehicleBuilder *pBuilder = (CVehicleBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_VehicleBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_VEHICLE );
	}

	// Set our builder parameters...

	// Create an entity...
	return CBot::Create( pBotDef, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}

//-----------------------------------------------------------------------------
void CVehicle::ClassHierarchyDestroy( void )
{
	CBot::ClassHierarchyDestroy();
	fforce_Kill( &m_hForce );

}

//-----------------------------------------------------------------------------
BOOL CVehicle::ClassHierarchyBuild( void )
{
	FMeshInit_t MeshInit;
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CVehicleBuilder *pBuilder = (CVehicleBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CBot::ClassHierarchyBuild() )
	{
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// set default "exitability" of vehicle stations based on if current
	// level is a "racing" level or not.
	if( level_IsRacingLevel() )
	{
		SetDriverExitable( FALSE );
		SetGunnerExitable( FALSE );
	}
	else
	{
		SetDriverExitable( TRUE );
		SetGunnerExitable( TRUE );
	}

	// allow exitability to be customized by user property
	if( pBuilder->m_uFlags & VEHICLE_BUILDER_FLAG_DRIVER_EXIT_SELECT )
	{
		SetDriverExitable( !!(pBuilder->m_uFlags & VEHICLE_BUILDER_FLAG_DRIVER_EXIT) );
	}
	if( pBuilder->m_uFlags & VEHICLE_BUILDER_FLAG_GUNNER_EXIT_SELECT )
	{
		SetGunnerExitable( !!(pBuilder->m_uFlags & VEHICLE_BUILDER_FLAG_GUNNER_EXIT) );
	}

	if( pBuilder->m_uFlags & VEHICLE_BUILDER_FLAG_NO_PLAYER_DRIVE )
	{
		m_uVehicleFlags |= VEHICLE_FLAG_NO_PLAYER_DRIVE;
	}
	if( pBuilder->m_uFlags & VEHICLE_BUILDER_FLAG_NO_ENTITY_DRIVE )
	{
		m_uVehicleFlags |= VEHICLE_FLAG_NO_ENTITY_DRIVE;
	}
	if( pBuilder->m_uFlags & VEHICLE_BUILDER_FLAG_DET_PACK_DESTROY )
	{
		m_uVehicleFlags |= VEHICLE_FLAG_DET_PACK_DESTROY;
	}

	fforce_NullHandle( &m_hForce );

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

//-----------------------------------------------------------------------------
void CVehicle::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CBot::ClassHierarchyAddToWorld();
	m_pWorldMesh->UpdateTracker();

	aigroup_RegisterAsMech(this);

	if (AIBrain())
	{
		aibrainman_ConfigurePostNPCVehicleBrain(AIBrain());		  //Normal for a vehicle is set by this function
	}

	m_fEngineSpeed = 0.0f;
}

//-----------------------------------------------------------------------------
void CVehicle::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CBot::ClassHierarchyRemoveFromWorld();

	fforce_Kill( &m_hForce );

	aigroup_UnregisterAsMech(this);
}

//-----------------------------------------------------------------------------
BOOL CVehicle::SquishBot( CBot *pBot )
{
	if( pBot->IsSquishy() && 
		pBot->m_fCollCylinderHeight_WS < m_fCollCylinderHeight_WS)		//and he is shorter than me.
	{
		pBot->Squish( this );
		//if( !pBot->IsPlayerOutOfBody() ) {
		//	if( pBot->m_pBotDef->m_nClass == BOTCLASS_MINER ) {
		//		CSplat::AddSplat( CSplat::SPLATTYPE_SQUISHED_GLITCH, 6.0f, pBot->MtxToWorld()->m_vPos, m_MountUnitFrontXZ_WS );
		//		return TRUE;
		//	} else if( pBot->m_pBotDef->m_nClass == BOTCLASS_GRUNT ) {
		//		CSplat::AddSplat( CSplat::SPLATTYPE_SQUISHED_GRUNT, 6.0f, pBot->MtxToWorld()->m_vPos, m_MountUnitFrontXZ_WS );
		//		return TRUE;
		//	}
		//}
	}
	return FALSE;
}
	
//-----------------------------------------------------------------------------
CEntityBuilder *CVehicle::GetLeafClassBuilder( void )
{
	return &_VehicleBuilder;
}

//-----------------------------------------------------------------------------
BOOL CVehicle::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() )
	{
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	m_DriverCamera.Init( &m_CamInfo, m_pBotInfo_DriverCamera );

	SetMaxUnitSpeed( 1.0f, FALSE /*bInternalOnly*/ );

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

//-----------------------------------------------------------------------------
void CVehicle::ClassHierarchyWork()
{
	CFVec3A TempVec3A;

	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();

	if( !IsOurWorkBitSet() )
	{
		return;
	}

	// Apply velocity impulses that were accululated last frame...
	if( m_ImpulseVelocity_WS.MagSq() > 0.01f )
	{
		CFVec3A vMomentum;
		vMomentum.Set( m_ImpulseVelocity_WS );
		vMomentum.Mul( m_PhysObj.GetMass() );
		m_PhysObj.ApplyMomentum( &vMomentum );
		m_ImpulseVelocity_WS.Zero();
	}

	UpdateFForceRumble();
#if _DRAW_BOUNDING_SPHERE
	CFVec3A vTemp;
	vTemp.x = m_pBotInfo_Gen->fCollSphere1X_MS + m_MountPos_WS.x;
	vTemp.y = m_pBotInfo_Gen->fCollSphere1Y_MS + m_MountPos_WS.y - 1.0f;
	vTemp.z = m_pBotInfo_Gen->fCollSphere1Z_MS + m_MountPos_WS.z;
	m_PhysObj.DrawSphere( &vTemp, m_pBotInfo_Gen->fCollSphere1Radius_MS );
#endif
}

//-----------------------------------------------------------------------------
void CVehicle::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT_NOW;
//	FASSERT( IsCreated() );
//	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
//	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}

//-----------------------------------------------------------------------------
void CVehicle::InflictDamage( CDamageData *pDamageData )
{
    
	// ambient damage will still affect the driver, so don't pass it along
	if( m_pDriverBot && !level_IsRacingLevel() && (pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_AMBIENT) )
	{
		// drivers take damage only on non-racing levels
		// (special driver armor profile reduces damage while in vehicle)

		// temporarily switch locale to ambient before applying to driver, as
		// bone information would be from vehicle and invalid for driver bot.
		CDamageForm::DamageLocale_e eSaveLocale = pDamageData->m_nDamageLocale;
		pDamageData->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;

		if( Level_aInfo[Level_nLoadedIndex].nLevel != LEVEL_HOLD_YOUR_GROUND ) {
            m_pDriverBot->InflictDamage( pDamageData );
		}

		// restore the locale and continue on.
		pDamageData->m_nDamageLocale = eSaveLocale;
	}

	CEntity::InflictDamage( pDamageData );
}

//-----------------------------------------------------------------------------
void CVehicle::InflictDamageResult( const CDamageResult *pDamageResult )
{
	FASSERT( pDamageResult );
	FASSERT( m_pBotInfo_Vehicle );

	if( m_uVehicleFlags & VEHICLE_FLAG_DET_PACK_DESTROY &&
		( pDamageResult->m_pDamageData->m_Damager.pEntity ) && 
		( pDamageResult->m_pDamageData->m_Damager.pEntity->TypeBits() & ENTITY_BIT_DETPACKDROP ) ) 
	{
		// destroy vehicle with detpack
		SetNormHealth( 0.01f );
		CBot::InflictDamageResult( pDamageResult );
	}
	else if( ( m_pDriverBot || GetGunnerBot() ) && level_IsRacingLevel() )
	{
		// vehicles take damage on racing levels
		CBot::InflictDamageResult( pDamageResult );
	}
	else
	{
		CEntity::ShakeCamera( pDamageResult );
	}

	if( !pDamageResult || !pDamageResult->m_pDamageData )
	{
		return;
	}

	if( pDamageResult && pDamageResult->m_pDamageData->m_Damager.pBot && 
		pDamageResult->m_pDamageData->m_Damager.pBot->TypeBits() & ENTITY_BIT_VEHICLE )
	{
		// movement caused by vehicle-to-vehicle collisions is
		// handled separately (by the vehicle physics code).
		return;
	}

	if( GetGunnerBot() )
	{
		// inform gunner that his vehicle was damaged
		GetGunnerBot()->InformAIOfDamageResult( pDamageResult );
	}

	if( !(m_uVehicleFlags & VEHICLE_FLAG_NO_DMG_IMPULSE) )
	{
		if( IsDriveable() && pDamageResult->m_fImpulseMag > 0.0f &&
			!(Level_aInfo[ Level_nLoadedIndex ].nLevel == LEVEL_WASTELAND_CHASE && GetGunnerBot() && GetGunnerBot()->IsPlayerBot()) )
		{
			// push vehicle around with this damage result
			CFVec3A vMomentum, vPoint;
			vPoint.Set( pDamageResult->m_pDamageData->m_ImpactPoint_WS );
			vPoint.Sub( *m_PhysObj.GetPosition() );

			if( vPoint.MagSq() > m_PhysObj.GetInertialSize()->MagSq() ) {
				vPoint.Unitize().Mul( m_PhysObj.GetInertialSize()->Mag() );
			}

			vMomentum.Set( pDamageResult->m_Impulse_WS );
			vMomentum.Mul( m_pBotInfo_Vehicle->fImpulseFraction );
			m_PhysObj.ApplyMomentum( &vMomentum, &vPoint );
		}
	}
}

//-----------------------------------------------------------------------------
// computes a trajectory for jumping entry into vehicle
// rvStartPos = starting position of jump
// rvEndPos = ending position of jump
// fMinJumpHeight = minimum height of jump
// fGravity = gravitational acceleration applied to jumping object. Negative values only.
// rvOutputVel = resulting jump velocity required to make the jump (xz and y included)
// Function return value is the time it will take to make the jump
f32 CVehicle::ComputeEntryTrajectory( const CFVec3A &rvStartPos, const CFVec3A &rvEndPos, f32 fMinJumpHeight, f32 fGravity, CFVec3A &rvOutputVel )
{
	CFVec3A vDeltaXZ;
	f32 fUpDeltaY, fDownDeltaY;
	f32 fInitialJumpSpeedY, fTerminalJumpSpeedY;
	f32 fJumpSpeedXZ;
	f32 fJumpTime;
	f32 fDistanceXZ;
	f32 fDistTravelledY;

	FASSERT( fMinJumpHeight > 0.0f );
	FASSERT( fGravity < 0.0f );

	vDeltaXZ.Set( rvEndPos );
	vDeltaXZ.Sub( rvStartPos );
	vDeltaXZ.y = 0.0f;

	if( rvEndPos.y > rvStartPos.y )
	{
		// end point is above start 
		fUpDeltaY = rvEndPos.y - rvStartPos.y + fMinJumpHeight;
		fDownDeltaY = fMinJumpHeight;
	}
	else
	{
		fUpDeltaY = fMinJumpHeight;
		fDownDeltaY = rvStartPos.y - rvEndPos.y + fMinJumpHeight;
	}

	fDistTravelledY = fUpDeltaY + fDownDeltaY;

	fInitialJumpSpeedY = fmath_Sqrt( -2.0f * fGravity * fUpDeltaY );
	fTerminalJumpSpeedY = fmath_Sqrt( -2.0f * fGravity * fDownDeltaY );

	fJumpTime = fInitialJumpSpeedY / -fGravity + fTerminalJumpSpeedY / -fGravity;
	FASSERT( fJumpTime > 0.0f );

	fDistanceXZ = vDeltaXZ.Mag();

	fJumpSpeedXZ = fmath_Div( fDistanceXZ, fJumpTime );

	if( rvOutputVel.SafeUnitAndMag( vDeltaXZ ) > 0.0f )
	{
		rvOutputVel.Mul( fJumpSpeedXZ );
	}
	else
	{
		rvOutputVel.Zero();
	}

	rvOutputVel.y = fInitialJumpSpeedY;

	return fJumpTime;
}

/*
//-----------------------------------------------------------------------------
// callback for station obstruction fcoll collision check.  Returns TRUE if 
// collision should be performed on pTracker, FALSE if not.
static u32 _StationObstructedCallback( CFWorldTracker *pTracker )
{
	u32 nIndex;

	for( nIndex = 0; nIndex < FWorld_nTrackerSkipListCount; nIndex++ )
	{
		if( FWorld_apTrackerSkipList[nIndex] == pTracker )
		{
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		}
	}

	return FCOLL_CHECK_CB_ALL_IMPACTS;
}
*/

//-----------------------------------------------------------------------------
// returns a pointer to the first bot found in the immediate area of the station, or NULL if none.
CBot *CVehicle::StationObstructed( CBot* pThis, u32 uBoneIndex, f32 fRadius )
{
	CFSphere StationCollSphere;
	CFCollData StationCollData;
	CFVec3A vMovement;
	CFMtx43A *pmStation;
	FCollImpact_t *pImpact;
	u32 uIndex;

	pmStation = pThis->m_pWorldMesh->GetBoneMtxPalette()[uBoneIndex];
	StationCollSphere.m_fRadius = fRadius;

	StationCollSphere.m_Pos.x = pmStation->m_vPos.x;
	StationCollSphere.m_Pos.y = pmStation->m_vPos.y;
	StationCollSphere.m_Pos.z = pmStation->m_vPos.z;

	vMovement.Zero();
	StationCollData.nFlags = FCOLL_DATA_IGNORE_BACKSIDE;
	StationCollData.nCollMask = FCOLL_MASK_CHECK_ALL;
	StationCollData.nTrackerUserTypeBitsMask = ENTITY_BIT_BOT;//FCOLL_USER_TYPE_BITS_ALL;
	StationCollData.pCallback = NULL; //_StationObstructedCallback;
	StationCollData.pLocationHint = (CFWorldTracker*) pThis->m_pWorldMesh;
	StationCollData.pMovement = &vMovement;

	FWorld_nTrackerSkipListCount = 0;
	pThis->AppendTrackerSkipList();
	StationCollData.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	StationCollData.ppTrackerSkipList = FWorld_apTrackerSkipList;

	fcoll_Clear();
	fcoll_Check( &StationCollData, &StationCollSphere );

	if( FColl_nImpactCount > 0 )
	{
		// step through all impacts
		for( uIndex = 0; uIndex < FColl_nImpactCount; uIndex++ )
		{
			// see if we hit a tracker
			pImpact = &FColl_aImpactBuf[uIndex];
			CFWorldTracker *pHitTracker = (CFWorldTracker *)pImpact->pTag;
			if( pHitTracker ) 
			{
				// find out if tracker is a bot
				if( pHitTracker->m_nUser == MESHTYPES_ENTITY ) 
				{
					CEntity *pEntity = (CEntity *)pHitTracker->m_pUser;
					if( pEntity->TypeBits() & ENTITY_BIT_BOT )
					{
//						m_PhysObj.DrawSphere( (CFVec3A*) &StationCollSphere.m_Pos, StationCollSphere.m_fRadius );
						CBot *pBot = (CBot *) pEntity;
						return pBot;
					}
				}
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// set speed of vehicle as a fraction of its CSV-set design speed.
// (unit speeds less than 0.01 are considered to be zero.)
void CVehicle::SetMaxUnitSpeed( f32 fUnitSpeed, BOOL bInternalOnly /*=FALSE*/)
{
	FASSERT( fUnitSpeed >= 0.0f );
	FASSERT( fUnitSpeed <= 1.0f );
	FMATH_CLAMP( fUnitSpeed, 0.0f, 1.0f );

	m_fMaxSpeed = m_pBotInfo_VehiclePhysics->fMaxSpeed * fUnitSpeed;

	if( fUnitSpeed > 0.01f )
	{
		m_fOOMaxSpeed = fmath_Inv( m_fMaxSpeed );
	}
	else
	{
		m_fMaxSpeed = 0.0f;
//		m_fOOMaxSpeed = FMATH_MAX_FLOAT;
		m_fOOMaxSpeed = 100.0f;
	}

	if( !bInternalOnly )
	{
		m_fMaxFlatSurfaceSpeed_WS = m_fMaxSpeed;
	}
}

//-----------------------------------------------------------------------------
f32 CVehicle::GetMaxUnitSpeed( void ) const {
	FASSERT( IsCreated() );

	f32 fUnitMaxSpeed;

	fUnitMaxSpeed = fmath_Div( m_fMaxSpeed, m_pBotInfo_VehiclePhysics->fMaxSpeed );
	FMATH_CLAMP_UNIT_FLOAT( fUnitMaxSpeed );
	
	return fUnitMaxSpeed;
}

//-----------------------------------------------------------------------------
// Note:  It appears that CBotSiteWeapon is now using this function as well.
BOOL CVehicle::IsDriverUseless(CBot* pDriver)
{
	return (!pDriver || pDriver->IsUseless() || 
		pDriver->Power_IsPoweringDown() ||
		pDriver->Power_IsPoweredDown() );
}


//-----------------------------------------------------------------------------
// protected member functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CVehicle::UpdateFForceRumble( void )
{
	if( m_nPossessionPlayerIndex < 0 )
	{
		fforce_Kill( &m_hForce );
		return;
	}

#if FANG_PLATFORM_XB
	f32 fUnitIntensity;
	BOOL bApply = FALSE;
	if( HitGround() )
	{
		fUnitIntensity = fmath_Div( m_fMaxHeightAboveGround, _MAX_RUMBLE_HEIGHT );
		FMATH_CLAMPMAX( fUnitIntensity, 1.0f );
		fUnitIntensity = fmath_Sqrt( fUnitIntensity );
		bApply = TRUE;
	}
	else if( Collided() || Jostled() )
	{
		fUnitIntensity = fmath_Div( m_fRecentCollisionMagnitude - m_pBotInfo_Vehicle->fSoftCollMom,
			m_pBotInfo_Vehicle->fHardCollMom - m_pBotInfo_Vehicle->fSoftCollMom );
		bApply = TRUE;
	}
	if( bApply )
	{
		fUnitIntensity *= 0.5f;
		fUnitIntensity += 0.5f;
		FMATH_CLAMP( fUnitIntensity, 0.0f, 1.0f );
		fxbforce_DecayCoarse( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, (fUnitIntensity * 0.7f + 0.4f), fUnitIntensity );
		fxbforce_DecayFine( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, (fUnitIntensity * 0.7f + 0.4f), fUnitIntensity );
	}
#else
	s32 nMagnitude = 0;
	if( HitGround() )
	{
		if( m_fMaxHeightAboveGround > _MAX_RUMBLE_HEIGHT )
		{
			nMagnitude = 3;
		}
		else if( m_fMaxHeightAboveGround > _MID_RUMBLE_HEIGHT )
		{
			nMagnitude = 2;
		}
		else if( m_fMaxHeightAboveGround > _MIN_RUMBLE_HEIGHT )
		{
			nMagnitude = 1;
		}
	}
	else if( Collided() )
	{
		nMagnitude = 3;
	}
	else if( Jostled() )
	{
		nMagnitude = 1;
	}
	switch( nMagnitude )
	{
		case 1:
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_HIGH, &m_hForce );
			break;

		case 2:
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );
			break;

		case 3:
			fforce_Play( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );
			break;

		default:
			break;
	}
#endif
}

#define _MIN_COLLISION_VOLUME	( 0.4f )
//-----------------------------------------------------------------------------
// check for collisions and play appropriate sounds
void CVehicle::PlayCollisionSounds( void )
{
	s32 nType = COLLISION_SOUND_TYPE_DIRT;
	cchar *pszType = NULL;

	if( m_pBotInfo_Vehicle == NULL )
	{
		return;
	}

//	if( m_pRecentCollisionMaterial )
//	{
//		// temporary hard-coded material ID's for audio test
//		switch( m_pRecentCollisionMaterial->m_nMtlID )
//		{
//			case 0: //"Metal", 
//			case 3: //"Metal Grate", 
//			case 6: //"Glass",
//				nType = COLLISION_SOUND_TYPE_METAL;
//				pszType = "METAL";
//				break;
//
//			case 4: //"Dirt", 
//			case 2: //"none", 
//				nType = COLLISION_SOUND_TYPE_DIRT;
//				pszType = "DIRT";
//				break;
//
//			case 5: //"Rock", 
//			case 1: //"Concrete", 
//				nType = COLLISION_SOUND_TYPE_ROCK;
//				pszType = "ROCK";
//				break;
//
//			case 9: //"Junk", 
//			case 7: //"Composite", 
//			case 8: //"Electronics", 
//			case 10: //"Water", 
//			case 11: //"Goop", 
//			case 12: //"Acid", 
//			case 13: //"Force
//			default:
//				pszType = "JUNK";
//				nType = COLLISION_SOUND_TYPE_JUNK;
//				break;
//		}
//	}

	// experimental - try randomly picking sound type
	nType = fmath_RandomRange( 0, COLLISION_SOUND_NUM_TYPES - 1 );

	if( HitGround() )
	{
		if( m_fRecentCollisionMagnitude > m_pBotInfo_Vehicle->fHardHitGroundMom )
		{
			PlaySound( m_pBotInfo_Vehicle->pSoundGroupHardHitGround );
//			SCRIPT_MESSAGE( "Played Long %s Hit Ground", pszType );
		}
		else if( m_fRecentCollisionMagnitude > m_pBotInfo_Vehicle->fSoftHitGroundMom )
		{
			PlaySound( m_pBotInfo_Vehicle->pSoundGroupSoftHitGround );
//			SCRIPT_MESSAGE( "Played Short %s Hit Ground", pszType );
		}
	}
	else
	{
		f32 fUnitVol;
		
		fUnitVol = fmath_Div( m_fRecentCollisionMagnitude - m_pBotInfo_Vehicle->fSoftCollMom, m_pBotInfo_Vehicle->fHardCollMom - m_pBotInfo_Vehicle->fSoftCollMom );
		fUnitVol *= 0.67f;
		FMATH_CLAMP( fUnitVol, 0.0f, 1.0f );
		if( fUnitVol < _MIN_COLLISION_VOLUME )
		{
			fUnitVol = _MIN_COLLISION_VOLUME;
		}

		if( Collided() )
		{
			PlaySound( m_pBotInfo_Vehicle->apSoundGroupCollision[ nType ][ COLLISION_SOUND_LENGTH_LONG ], fUnitVol );
//			SCRIPT_MESSAGE( "Played Long %s Collision", pszType );
		}
		else if( Jostled() )
		{
			PlaySound( m_pBotInfo_Vehicle->apSoundGroupCollision[ nType ][ COLLISION_SOUND_LENGTH_SHORT ], fUnitVol );
//			SCRIPT_MESSAGE( "Played Short %s Collision", pszType );
		}
	}
}

//-----------------------------------------------------------------------------
// This function fades among three engine sounds, based on a unitized engine speed value 
// which is computed from parameters such as throttle setting and vehicle speed.
//
// The engine sounds and crossfade values are specified in the vehicle's CSV file.
// The volume and pitch values of the engine sounds are set in the snd_veh.csv file.
//
// The idle engine sound plays at the lowest engine speed, while rising in pitch and fading out
// as the speed increases.
//
// The low engine sound fades in as the idle sound fades out, rises in pitch over
// the crossfade range specified in the vehicle CSV file, and fades out as the engine
// speed reaches the top of the crossfade range.
//
// The high engine sound fades in over the crossfade range, and rises in pitch over the
// crossfade range.
//
void CVehicle::UpdateEngineSounds( void )
{
	const CFVec3A *pvVel;
	f32 fSpeed;
	f32 fPitchIdle;
	f32 fPitchLow;
	f32 fPitchHigh;
	f32 fVel;

	pvVel = m_PhysObj.GetVelocity();

	if( m_pEngineLowEmitter && m_pEngineHighEmitter && m_pEngineIdleEmitter )
	{
		// variables ending in Low are for low engine sound,
		// those ending in High are for high engine sound,
		CFSoundInfo *pSoundInfo;
		CFSoundInfo::DecompressedSoundInfo_t InfoIdle;
		CFSoundInfo::DecompressedSoundInfo_t InfoLow;
		CFSoundInfo::DecompressedSoundInfo_t InfoHigh;

		//---------------------------------------------------------------------
		// get info about sound groups.  Must decompress every frame?
		pSoundInfo = CFSoundGroup::GetSoundInfo( m_pBotInfo_Engine->pSoundGroupEngineIdle, 0 );
		if( pSoundInfo )
		{
			pSoundInfo->Decompress( &InfoIdle );
		}
		else
		{
			FASSERT_NOW;
		}

		pSoundInfo = CFSoundGroup::GetSoundInfo( m_pBotInfo_Engine->pSoundGroupEngineLow, 0 );
		if( pSoundInfo )
		{
			pSoundInfo->Decompress( &InfoLow );
		}
		else
		{
			FASSERT_NOW;
		}

		pSoundInfo = CFSoundGroup::GetSoundInfo( m_pBotInfo_Engine->pSoundGroupEngineHigh, 0 );
		if( pSoundInfo )
		{
			pSoundInfo->Decompress( &InfoHigh );
		}
		else
		{
			FASSERT_NOW;
		}

		//---------------------------------------------------------------------
		// compute new unit engine speed
		//		if( SmoothContact() )
		if( m_PhysObj.IsColliding() || m_PhysObj.IsContacting() )
		{
			// on ground

			if( m_fMaxSpeed > 0.0f )
			{
				fSpeed = m_fUnitSpeed_WS;
			}
			else
			{
				fSpeed = 0.0f;
			}

			// average in throttle setting if throttling in same direction as vehicle movement
			fVel = m_MtxToWorld.m_vFront.Dot( *pvVel );
			if( fVel >= 0.0f && m_fThrottle >= 0.0f ||
				fVel < 0.0f && m_fThrottle < 0.0f )
			{
				if( !IsPlayerBot() && (TypeBits() & ENTITY_BIT_VEHICLERAT) )
				{
					// make AI RAT engines sound faster
					f32 fThrottle = m_fThrottle;
					fThrottle *= 1.3f;
					FMATH_CLAMPMAX( fThrottle, 1.0f );
					fSpeed += FMATH_FABS(fThrottle);
				}
				else
				{
					fSpeed += FMATH_FABS(m_fThrottle);
				}

				fSpeed += FMATH_FABS(m_fThrottle);
				fSpeed *= 0.5f;
			}

			if( fmath_RandomChance( m_pBotInfo_Engine->fVarianceChance /*0.04f*/ ) )
			{
				m_fEngineVariance = fmath_RandomFloatRange( m_pBotInfo_Engine->fVarianceMinMul /*0.85f*/, m_pBotInfo_Engine->fVarianceMaxMul /*1.0f*/ );
			}

			fSpeed *= m_fEngineVariance;
			fSpeed *= m_pBotInfo_Engine->fOOAirSpeedMul;	// helps keeps unit engine speed from reaching 1.0 (which is reserved for in-air speed).

			if( m_fTimeSinceLastHitGround < 0.5f )
			{
				// move speed quickly back down to normal after being in the air
				m_fEngineSpeed += ( fSpeed - m_fEngineSpeed ) * m_pBotInfo_Engine->fRevRate * 3.0f * FLoop_fPreviousLoopSecs;
			}
			else
			{
				// move towards new speed setting
				m_fEngineSpeed += ( fSpeed - m_fEngineSpeed ) * m_pBotInfo_Engine->fRevRate * FLoop_fPreviousLoopSecs;
			}
		}
		else
		{
			// in air

			if( !IsPlayerBot() && (TypeBits() & ENTITY_BIT_VEHICLERAT) )
			{
				// make AI RAT engines sound faster
				f32 fThrottle = m_fThrottle;
				fThrottle *= 1.3f;
				FMATH_CLAMPMAX( fThrottle, 1.0f );
				fSpeed = FMATH_FABS(fThrottle) * m_pBotInfo_Engine->fAirSpeedMul;
			}
			else
			{
				fSpeed = FMATH_FABS(m_fThrottle) * m_pBotInfo_Engine->fAirSpeedMul;
			}

			// move towards new speed setting
			m_fEngineSpeed += ( fSpeed - m_fEngineSpeed ) * m_pBotInfo_Engine->fAirRevRate * FLoop_fPreviousLoopSecs;
		}


		//---------------------------------------------------------------------
		// crossfade between low and high engine sounds based on engine speed
		f32 fCrossfade, fInvCrossfade;
		f32 fLogCrossfade, fInvLogCrossfade;
		f32 fVolumeLow, fVolumeHigh, fVolumeIdle;
		f32 fMinFadePoint = m_pBotInfo_Engine->fCrossfadeUnitCenter - m_pBotInfo_Engine->fCrossfadeUnitWidth * 0.5f;
		f32 fMaxFadePoint = m_pBotInfo_Engine->fCrossfadeUnitCenter + m_pBotInfo_Engine->fCrossfadeUnitWidth * 0.5f;
		FMATH_CLAMPMIN( fMinFadePoint, 0.0f );
		FMATH_CLAMPMAX( fMaxFadePoint, 1.0f );

		// compute idle sound unit value as a function of it's CSV specified range
		f32 fUnitIdle;
		fUnitIdle = fmath_Div( m_fEngineSpeed, m_pBotInfo_Engine->fIdleUnitWidth );
		FMATH_CLAMP( fUnitIdle, 0.0f, 1.0f );

		if( m_fEngineSpeed < fMinFadePoint )
		{
			fCrossfade = 0.0f;
		}
		else if( m_fEngineSpeed > fMaxFadePoint )
		{
			fCrossfade = 1.0f;
		}
		else
		{
			fCrossfade = fmath_Div( m_fEngineSpeed - fMinFadePoint, m_pBotInfo_Engine->fCrossfadeUnitWidth );
		}
		FMATH_CLAMP( fCrossfade, 0.0f, 1.0f );

		fInvCrossfade = 1.0f - fCrossfade;

		// give crossfades a log curve
		fInvLogCrossfade = fmath_Sqrt( fInvCrossfade );
		fLogCrossfade = fmath_Sqrt( fCrossfade );

		if( IsPlayerBot() )
		{
			//			fVolumeIdle = fmath_Sqrt(1.0f - fUnitIdle) * InfoIdle.m_fUnitVol2D;
			fVolumeIdle = (1.0f - fUnitIdle) * InfoIdle.m_fUnitVol2D;	// non-log fade sounds better here
			fVolumeLow = fInvLogCrossfade * InfoLow.m_fUnitVol2D;
			fVolumeHigh = fLogCrossfade * InfoHigh.m_fUnitVol2D;
		}
		else
		{
			//			fVolumeIdle = fmath_Sqrt(1.0f - fUnitIdle) * InfoIdle.m_fUnitVol3D;
			fVolumeIdle = (1.0f - fUnitIdle) * InfoIdle.m_fUnitVol3D;	// non-log fade sounds better here
			fVolumeLow = fInvLogCrossfade * InfoLow.m_fUnitVol3D;
			fVolumeHigh = fLogCrossfade * InfoHigh.m_fUnitVol3D;
		}
		FMATH_CLAMP( fVolumeLow, 0.0f, 1.0f );
		FMATH_CLAMP( fVolumeHigh, 0.0f, 1.0f );

		// fade in low engine sound if within idle engine sound range
		if( m_fEngineSpeed < m_pBotInfo_Engine->fIdleUnitWidth )
		{
			f32 fLow;
			// if specd, allow a little bit of low engine sound to sound even at idle
			fLow = m_pBotInfo_Engine->fLowMinVolume /*0.4f*/ + (1.0f - m_pBotInfo_Engine->fLowMinVolume ) /*0.6f*/ * fUnitIdle;
			fVolumeLow *= fLow;
		}

		m_pEngineIdleEmitter->SetVolume( fVolumeIdle * m_fEngineMasterVolume );
		m_pEngineLowEmitter->SetVolume( fVolumeLow * m_fEngineMasterVolume );
		m_pEngineHighEmitter->SetVolume( fVolumeHigh * m_fEngineMasterVolume );

		// compute new pitch values based on engine speed in relation to crossfade widths
		f32 fUnitPitch;
		fUnitPitch = m_fEngineSpeed * fCrossfade;

		fPitchIdle = InfoIdle.m_fMinPitchMult + fUnitIdle * (InfoIdle.m_fMaxPitchMult - InfoIdle.m_fMinPitchMult);
		fPitchLow = InfoLow.m_fMinPitchMult + fUnitPitch * (InfoLow.m_fMaxPitchMult - InfoLow.m_fMinPitchMult);
		fPitchHigh = InfoHigh.m_fMinPitchMult + fUnitPitch * (InfoHigh.m_fMaxPitchMult - InfoHigh.m_fMinPitchMult);
		FMATH_CLAMPMIN( fPitchIdle, 0.0f );
		FMATH_CLAMPMAX( fPitchIdle, InfoIdle.m_fMinPitchMult + (InfoIdle.m_fMaxPitchMult - InfoIdle.m_fMinPitchMult) * m_pBotInfo_Engine->fAirSpeedMul );
		FMATH_CLAMPMIN( fPitchLow, 0.0f );
		FMATH_CLAMPMAX( fPitchLow, InfoLow.m_fMinPitchMult + (InfoLow.m_fMaxPitchMult - InfoLow.m_fMinPitchMult) * m_pBotInfo_Engine->fAirSpeedMul );
		FMATH_CLAMPMIN( fPitchHigh, 0.0f );
		FMATH_CLAMPMAX( fPitchHigh, InfoHigh.m_fMinPitchMult + (InfoHigh.m_fMaxPitchMult - InfoHigh.m_fMinPitchMult) * m_pBotInfo_Engine->fAirSpeedMul );
		// update frequency of engine sound
		m_pEngineIdleEmitter->SetFrequencyFactor( fPitchIdle );
		m_pEngineLowEmitter->SetFrequencyFactor( fPitchLow );
		m_pEngineHighEmitter->SetFrequencyFactor( fPitchHigh );

		//---------------------------------------------------------------------
		// update position of engine sound
		if( !IsPlayerBot() && (!GetGunnerBot() || !GetGunnerBot()->IsPlayerBot() ) )
		{
			m_pEngineIdleEmitter->SetPosition( m_PhysObj.GetPosition() );
			m_pEngineLowEmitter->SetPosition( m_PhysObj.GetPosition() );
			m_pEngineHighEmitter->SetPosition( m_PhysObj.GetPosition() );
		}
	}
}


//-----------------------------------------------------------------------------
static void _SetFOV( s32 nIndex, f32 fFOV )
{
	CFCamera* pCam = fcamera_GetCameraByIndex( nIndex );
	pCam->SetFOV( splitscreen_RemapFOV( nIndex, fFOV ) );
}

//-----------------------------------------------------------------------------
// saves state for checkpoint
BOOL CVehicle::CheckpointSave( void )
{
	// save parent class data
	CBot::CheckpointSave();

	CFCheckPoint::SaveData( m_fMaxSpeed );
	CFCheckPoint::SaveData( m_fOOMaxSpeed );
	CFCheckPoint::SaveData( m_uVehicleFlags );
	CFCheckPoint::SaveData( m_fReticlePrevX );
	CFCheckPoint::SaveData( m_fReticlePrevY );
	CFCheckPoint::SaveData( m_bReticleDrawPrev );
	CFCheckPoint::SaveData( (u32&) m_ReticlePrevType );

	return TRUE;
}

//-----------------------------------------------------------------------------
// loads state for checkpoint
void CVehicle::CheckpointRestore( void )
{
	// load parent class data
	CBot::CheckpointRestore();

	CFCheckPoint::LoadData( m_fMaxSpeed );
	CFCheckPoint::LoadData( m_fOOMaxSpeed );
	CFCheckPoint::LoadData( m_uVehicleFlags );
	CFCheckPoint::LoadData( m_fReticlePrevX );
	CFCheckPoint::LoadData( m_fReticlePrevY );
	CFCheckPoint::LoadData( m_bReticleDrawPrev );
	CFCheckPoint::LoadData( (u32&) m_ReticlePrevType );

	m_fTimeSinceLastCollide = 0.0f;
	m_fTimeSinceLastContact = 0.0f;
	m_fTimeSinceLastHitGround = 0.0f;
}

//-----------------------------------------------------------------------------
void CVehicle::StartTransitionToDriverVehicleCamera( s32 nPlayerIndex )
{
	m_DriverCameraTrans.InitTransitionToVehicleCamera( nPlayerIndex );

	// snap the camera to a position behind the vehicle (but will still transition smoothly from bot camera)
	SnapCameraPosition();
}

//-----------------------------------------------------------------------------
void CVehicle::StartTransitionToDriverBotCamera( void )
{
	m_DriverCameraTrans.InitTransitionToBotCamera();
}

void CVehicle::SetSoundFxRadiusMultiplier( f32 fNewRadiusMultipler ) {
	FASSERT( IsCreated() );
	FASSERT( fNewRadiusMultipler > 0.0f );
	
	m_fSoundFxRadiusMultiplier = fNewRadiusMultipler;
}


//-----------------------------------------------------------------------------
// CCameraTrans functions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CCameraTrans::Init( CCamManualInfo *pCamInfo, f32 fTransitionLength )
{
	FASSERT( pCamInfo );
	FASSERT( fTransitionLength > 0.0f );

	m_pCamInfo = pCamInfo;
	m_eCameraState = CAMERA_STATE_INACTIVE;
	m_fTransitionLength = fTransitionLength;
	m_fOOTransitionLength = 1.0f / fTransitionLength;
	m_fRemainingTime = 0.0f;
	m_qPreviousCam.Identity();
	m_vPreviousCam.Zero();
	m_fPreviousCamFOV = 0.0f;
	m_nCameraPlayerIndex = -1;
	m_uFlags = 0;
}

//-----------------------------------------------------------------------------
void CCameraTrans::InitTransitionToBotCamera( void )
{
	if( m_nCameraPlayerIndex >= 0 )
	{
		m_eCameraState = CAMERA_STATE_START_EXIT_VEHICLE;
	}
}

//-----------------------------------------------------------------------------
void CCameraTrans::InitTransitionToVehicleCamera( s32 nPlayerIndex )
{
	// remember player number (or -1) that will be used by camera system for transitions
	m_nCameraPlayerIndex = nPlayerIndex;

	if( m_nCameraPlayerIndex >= 0 )
	{
		// remember player bot's camera
		CFCamera *pPreviousCam = fcamera_GetCameraByIndex( m_nCameraPlayerIndex );
		m_qPreviousCam.BuildQuat( pPreviousCam->GetFinalXfm()->m_MtxR );
		m_vPreviousCam.Set( pPreviousCam->GetFinalXfm()->m_MtxR.m_vPos );
		pPreviousCam->GetFOV( &m_fPreviousCamFOV );
		m_fPreviousCamFOV = splitscreen_UnmapFOV( m_nCameraPlayerIndex, m_fPreviousCamFOV );

  		m_eCameraState = CAMERA_STATE_START_ENTER_VEHICLE;
	}
	else
	{
		m_uFlags &= ~FLAG_SNAP_CAM_TRANS;
		m_qPreviousCam.Identity();
		m_vPreviousCam.Zero();
	}
}

void CCameraTrans::InitTransitionToCamera( CCamManualInfo *pCamInfo, f32 fTransitionLength )
{
	m_vPreviousCam = m_pCamInfo->m_pmtxMtx->m_vPos;
	m_qPreviousCam.BuildQuat( *m_pCamInfo->m_pmtxMtx );
	m_fPreviousCamFOV = m_pCamInfo->m_fHalfFOV;
	m_fTransitionLength = fTransitionLength;
	m_pCamInfo = pCamInfo;
	m_eCameraState = CAMERA_STATE_START_ENTER_VEHICLE;
}

//-----------------------------------------------------------------------------
// Call every frame after setting manual camera to desired position/orientation.
// Transitions between bot camera and manual vehicle camera based on camera state.
void CCameraTrans::UpdateTransition( void )
{
	
	if( m_eCameraState == CAMERA_STATE_ABORT_TO_INACTIVE )
	{
		m_fRemainingTime = 0.0f;
		if( m_nCameraPlayerIndex >= 0 )
		{
			gamecam_SwitchPlayerTo3rdPersonCamera((GameCamPlayer_e) m_nCameraPlayerIndex, (CBot*) Player_aPlayer[ m_nCameraPlayerIndex ].m_pEntityCurrent );
		}
		m_eCameraState = CAMERA_STATE_INACTIVE;
	}

	if( m_eCameraState == CAMERA_STATE_START_ENTER_VEHICLE )
	{
		m_eCameraState = CAMERA_STATE_ENTERING_VEHICLE;
		if( m_uFlags & FLAG_SNAP_CAM_TRANS )
		{
			m_uFlags &= ~FLAG_SNAP_CAM_TRANS;
			m_fRemainingTime = 0.0f;
		}
		else
		{
			m_fRemainingTime = m_fTransitionLength;
		}

		gamecam_SwitchPlayerToManualCamera((GameCamPlayer_e) m_nCameraPlayerIndex, m_pCamInfo );
	}

	if( m_eCameraState == CAMERA_STATE_START_EXIT_VEHICLE || m_eCameraState == CAMERA_STATE_EXITING_VEHICLE )
	{
		// temporarily switch to bot 3rd person camera
		gamecam_SwitchPlayerTo3rdPersonCamera((GameCamPlayer_e) m_nCameraPlayerIndex, (CBot*) Player_aPlayer[ m_nCameraPlayerIndex ].m_pEntityCurrent );
		CFCamera *pBotCam = fcamera_GetCameraByIndex( m_nCameraPlayerIndex );
		fcamera_Work();	// hackish way to get bot camera to update

		// copy bot camera orientation and position
		m_qPreviousCam.BuildQuat( pBotCam->GetFinalXfm()->m_MtxR );
		m_vPreviousCam.Set( pBotCam->GetFinalXfm()->m_MtxR.m_vPos );
		// return to vehicle camera
		gamecam_SwitchPlayerToManualCamera((GameCamPlayer_e) m_nCameraPlayerIndex, m_pCamInfo );

		if( m_eCameraState == CAMERA_STATE_START_EXIT_VEHICLE )
		{
			m_eCameraState = CAMERA_STATE_EXITING_VEHICLE;
			m_fRemainingTime = m_fTransitionLength;
		}
	}
	
	if( m_eCameraState == CAMERA_STATE_ENTERING_VEHICLE || m_eCameraState == CAMERA_STATE_EXITING_VEHICLE )
	{
		CFQuatA qVehicleCam, qSlerp;
		f32 fUnitTransition;
		CFCamera *pCamera;
		pCamera = fcamera_GetCameraByIndex( m_nCameraPlayerIndex );
		f32 fFOV;

		qVehicleCam.BuildQuat( *m_pCamInfo->m_pmtxMtx );

		m_fRemainingTime -= FLoop_fPreviousLoopSecs;

		if( m_eCameraState == CAMERA_STATE_ENTERING_VEHICLE )
		{
			if( m_fRemainingTime < 0.0f )
			{
				// transition to in-vehicle camera complete
				m_fRemainingTime = 0.0f;
				m_eCameraState = CAMERA_STATE_IN_VEHICLE;
			}
			else // more time left in transition
			{
				fUnitTransition = m_fTransitionLength - m_fRemainingTime;
				fUnitTransition *= m_fOOTransitionLength;
				fUnitTransition = fmath_UnitLinearToSCurve( fUnitTransition );

				// interpolate from bot's last camera orientation to vehicle cam orientation
				qSlerp.ReceiveSlerpOf( fUnitTransition, m_qPreviousCam, qVehicleCam );
				qSlerp.BuildMtx33( *m_pCamInfo->m_pmtxMtx );

				// interpolate from bot's last camera position to vehicle cam position
				m_pCamInfo->m_pmtxMtx->m_vPos.Sub( m_vPreviousCam );
				m_pCamInfo->m_pmtxMtx->m_vPos.Mul( fUnitTransition );
				m_pCamInfo->m_pmtxMtx->m_vPos.Add( m_vPreviousCam );

				// interpolate from bot's last camera FOV to vehicle cam FOV
				fFOV = m_fPreviousCamFOV + ( ( m_pCamInfo->m_fHalfFOV - m_fPreviousCamFOV ) * fUnitTransition );
				m_pCamInfo->m_fHalfFOV = fFOV;
			}
		}
		else // CAMERA_STATE_EXITING_VEHICLE
		{
			if( m_fRemainingTime < 0.0f )
			{
				// transition to previous bot camera complete
				m_fRemainingTime = 0.0f;
				gamecam_SwitchPlayerTo3rdPersonCamera((GameCamPlayer_e) m_nCameraPlayerIndex, (CBot*) Player_aPlayer[ m_nCameraPlayerIndex ].m_pEntityCurrent );
				m_eCameraState = CAMERA_STATE_INACTIVE;
			}
			else // more time left in transition
			{
				fUnitTransition = m_fTransitionLength - m_fRemainingTime;
				fUnitTransition *= m_fOOTransitionLength;
				fUnitTransition = fmath_UnitLinearToSCurve( fUnitTransition );

				// interpolate from vehicle cam orientation to bot's last camera orientation
				qSlerp.ReceiveSlerpOf( fUnitTransition, qVehicleCam, m_qPreviousCam );
				qSlerp.BuildMtx33( *m_pCamInfo->m_pmtxMtx );

				// interpolate from vehicle cam position to bot's last camera position
				CFVec3A vCamPos;
				vCamPos.Set( m_pCamInfo->m_pmtxMtx->m_vPos );
				m_pCamInfo->m_pmtxMtx->m_vPos.Set( m_vPreviousCam );
				m_pCamInfo->m_pmtxMtx->m_vPos.Sub( vCamPos );
				m_pCamInfo->m_pmtxMtx->m_vPos.Mul( fUnitTransition );
				m_pCamInfo->m_pmtxMtx->m_vPos.Add( vCamPos );

				// interpolate from bot's last camera FOV to vehicle cam FOV
				fFOV = m_pCamInfo->m_fHalfFOV + ( ( m_fPreviousCamFOV - m_pCamInfo->m_fHalfFOV ) * fUnitTransition );
				m_pCamInfo->m_fHalfFOV = fFOV;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// CVehicleCamera functions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CVehicleCamera::Init( CCamManualInfo *pCamInfo, BotInfo_VehicleCamera_t *pParams )
{
	FASSERT( pCamInfo );
	FASSERT( pParams );

	m_qCamLookBack.Identity();
	m_pCamInfo = pCamInfo;
	m_pParams = pParams;
	m_fCamDistance = m_pParams->fCamDistance;
	m_fCamHeight = m_pParams->fCamDefaultHeight;
	m_fCamBipolarUnitHeight = 0.0f;
	m_fCamSlerp = m_pParams->fCamGroundRotRate;
	m_fAirTime = 0.0f;
	m_fFOV = FMATH_DEG2RAD( m_pParams->fFOV );
	m_uFlags = 0;
}

//-----------------------------------------------------------------------------
// sets unit height of camera.  Camera height will be relative to plane defined
// by the ground normal.
void CVehicleCamera::SetHeight( f32 fBipolarUnitHeight )
{
	FMATH_CLAMP( fBipolarUnitHeight, -1.0f, 1.0f ); 
	m_fCamBipolarUnitHeight = fBipolarUnitHeight; 
	m_uFlags &= ~(_FLAG_DIRECT_ELEVATION_ANGLE | _FLAG_HEIGHT_FROM_XZ_PLANE); 
};

//-----------------------------------------------------------------------------
// sets unit height of camera.  Camera height will be relative to XZ plane.
void CVehicleCamera::SetHeightFromXZPlane( f32 fBipolarUnitHeight )
{
	FMATH_CLAMP( fBipolarUnitHeight, -1.0f, 1.0f ); 
	m_fCamBipolarUnitHeight = fBipolarUnitHeight; 
	m_uFlags |= _FLAG_HEIGHT_FROM_XZ_PLANE; 
	m_uFlags &= ~_FLAG_DIRECT_ELEVATION_ANGLE; 
}

//-----------------------------------------------------------------------------
// directly sets elevation angle of camera relative to pmTargetObject parameter
// of Update() function.
void CVehicleCamera::SetElevationAngle( f32 fAngle )
{ 
	m_fElevationAngle = fAngle; 
	m_uFlags |= _FLAG_DIRECT_ELEVATION_ANGLE;
	m_uFlags &= ~_FLAG_HEIGHT_FROM_XZ_PLANE; 
};

//-----------------------------------------------------------------------------
// Note: must set tracker skip list before calling this function!
void CVehicleCamera::Update( CFMtx43A *pmTargetObject, const CFVec3A &vObjectVel, CFWorldTracker *pWorldMesh, const CFVec3A &vGroundNormal )
{
	CFMtx43A mLook, mRot;
	CFVec3A vCamPos, vLook, vLookBack, vLookPoint, vRight, vUp, vTemp;
	CFQuatA qLook, qLastLook, qSlerp;
	CFVec3A vMovement;
	CFCollData CollData;
	CFSphere CollSphere;
	f32 fSlerp;
	f32 fTargetDistance;

	// compute point out in front of vehicle at which camera should look. 
	vLookPoint.Set( pmTargetObject->m_vFront );
	vLookPoint.PlanarProjection( vGroundNormal );
	if( vLookPoint.SafeUnitAndMag( vLookPoint ) <= 0.0f )
	{
		// vehicle pointing straight up or down, use last frame camera front vector
		vLookPoint.Set( m_pCamInfo->m_pmtxMtx->m_vFront );
	}
	vLookPoint.Mul( m_pParams->fLookForward );
	vLookPoint.Add( pmTargetObject->m_vPos );
	vLookPoint.y += m_pParams->fLookHeight;

	if( m_uFlags & _FLAG_HEIGHT_FROM_XZ_PLANE )	// client used SetHeightFromXZPlane()
	{
		vLookBack.Set( pmTargetObject->m_vFront );
		vLookBack.Negate();
		vLookBack.y = 0.0f;
		if( vLookBack.SafeUnitAndMag( vLookBack ) > 0.0f )
		{
			m_fCamHeight += m_fCamBipolarUnitHeight * m_pParams->fCamHeightVel * FLoop_fPreviousLoopSecs;
			FMATH_CLAMP( m_fCamHeight, m_pParams->fCamMinHeight, m_pParams->fCamMaxHeight );

			vTemp.Mul( m_fCamHeight );
			vLookBack.Mul( m_pParams->fCamDistance );
			vLookBack.y += m_fCamHeight;
			vLookBack.Unitize();
		}
		else
		{
			vLookBack.Set( pmTargetObject->m_vFront );
			vLookBack.Negate();
		}
	}
	else if( m_uFlags & _FLAG_DIRECT_ELEVATION_ANGLE )	// client used SetElevationAngle()
	{
		vLookBack.Set( CFVec3A::m_UnitAxisZ );
		vLookBack.Negate();
		mRot.Identity();
		mRot.SetRotationX( m_fElevationAngle );
		mRot.MulDir( vLookBack );
		pmTargetObject->MulDir( vLookBack );	
	}
	else	// client used SetHeight()
	{
		// use orientation of vehicle for vector looking back from vLookPoint to camera position.
		vLookBack.Set( pmTargetObject->m_vFront );
		vLookBack.Negate();

		// mix vehicle orientation with vehicle velocity to get camera orientation
		vTemp.Set( vObjectVel );
		f32 fMagSq = vTemp.MagSq();
		if( fMagSq > 16.0f && vTemp.Dot( vLookBack ) < 0.0f )
		{
			// construct a scaled velocity vector 
			fMagSq -= 16.0f;
			fMagSq *= (1.0f / 400.0f);
			FMATH_CLAMPMAX( fMagSq, 1.0f );
			vTemp.Negate();
			vTemp.Unitize();
			vTemp.Mul( fMagSq );

			// mix in scaled velocity vector and re-unitize
			vLookBack.Add( vTemp );
			vLookBack.Unitize();
		}

		// project lookback onto ground plane
		vLookBack.PlanarProjection( vGroundNormal );

		m_fCamHeight += m_fCamBipolarUnitHeight * m_pParams->fCamHeightVel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMP( m_fCamHeight, m_pParams->fCamMinHeight, m_pParams->fCamMaxHeight );

		vTemp.Set( vGroundNormal );
		vTemp.Mul( m_fCamHeight );
		vLookBack.Mul( m_pParams->fCamDistance );
		vLookBack.Add( vTemp );
		vLookBack.Unitize();
	}

	// build a quaternion representing rotation from Z axis to lookback vector
	mLook.m_vFront.Set( vLookBack );
	mLook.m_vRight.Cross( CFVec3A::m_UnitAxisY, vLookBack );
	if( mLook.m_vRight.SafeUnitAndMag( mLook.m_vRight ) > 0.0f )
	{
		mLook.m_vUp.Cross( vLookBack, mLook.m_vRight );
		if( mLook.m_vUp.SafeUnitAndMag( mLook.m_vUp ) )
		{
			qLook.BuildQuat( mLook );
		}
		else
		{
			qLook.Set( m_qCamLookBack );
		}
	}
	else
	{
		qLook.Set( m_qCamLookBack );
	}

	// adjust camera movement speed (quaternion slerp amount) based on vehicle
	// being airborne or not.  Helps to reduce violent camera movement during
	// vehicle crashes.
	if( m_fAirTime == 0.0f )
	{
		// slowly ramp camera movement up to normal speed when vehicle is
		// contacting the ground after a period of being airborne.
		m_fCamSlerp += FLoop_fPreviousLoopSecs * 1.0f;
		FMATH_CLAMPMAX( m_fCamSlerp, m_pParams->fCamGroundRotRate );
	}
	else
	{
		// move camera very slowly while vehicle is in the air.
		// helps to reduce violent camera moves while vehicle is tumbling.
		m_fCamSlerp = m_pParams->fCamAirRotRate;
	}

	if( m_uFlags & _FLAG_SNAP )
	{
		fSlerp = 1.0f;
		m_uFlags &= ~_FLAG_SNAP;
	}
	else
	{
		fSlerp = m_fCamSlerp * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( fSlerp, 1.0f );	// clamp in case we have a very long frame
	}

	// move camera from current lookback towards new lookback via quaternion slerp.  
	// This smooths the orientation changes of the camera.
	qSlerp.ReceiveSlerpOf( fSlerp, m_qCamLookBack, qLook );

	// remember new lookback
	m_qCamLookBack.Set( qSlerp );

	// construct a unit vector in direction of lookback
	m_qCamLookBack.MulPoint( vLookBack, CFVec3A::m_UnitAxisZ );

	// compute unit lookat vector
	vLook.Set( vLookBack );
	vLook.Negate();

	// push camera back while in the air
	fTargetDistance = m_fAirTime * m_pParams->fCamDistanceAirVel;

	// add in base distance
	fTargetDistance += m_pParams->fCamDistance + m_pParams->fLookForward;

	if( m_uFlags & _FLAG_DEAD )
	{
		fTargetDistance = m_pParams->fCamDeadDistance;
	}

	m_fCamDistance += ( fTargetDistance - m_fCamDistance ) * FLoop_fPreviousLoopSecs * m_pParams->fCamDistanceVel;

	FMATH_CLAMPMAX( m_fCamDistance, m_pParams->fCamMaxDistance + m_pParams->fLookForward );

	// project along lookback unit vector to get delta distance from vehicle
	vCamPos.Mul( vLookBack, m_fCamDistance );

	// add in the lookat point to get a worldspace position for camera
	vCamPos.Add( vLookPoint );

	// check for camera collision
	// (check for collisions from camera to center of object, not out to look point in front)
	vMovement.Set( vCamPos );
	vMovement.Sub( pmTargetObject->m_vPos );
	CollSphere.m_Pos.Set( pmTargetObject->m_vPos.x, pmTargetObject->m_vPos.y, pmTargetObject->m_vPos.z );
	CollSphere.m_fRadius = m_pParams->fCamColSphereRadius;
	CollData.nFlags = FCOLL_DATA_FLAGS_NONE;
	CollData.nCollMask = FCOLL_MASK_COLLIDE_WITH_CAMERA;
	CollData.nTrackerUserTypeBitsMask = ~(ENTITY_BITS_ALLBOTBITS);
	CollData.pCallback = NULL;//_TrackerCallback;
	CollData.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	CollData.ppTrackerSkipList = FWorld_apTrackerSkipList;
	CollData.pLocationHint = pWorldMesh;
	CollData.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	CollData.pMovement = &vMovement;
	fcoll_Clear();
	fcoll_Check( &CollData, &CollSphere );
	if( FColl_nImpactCount > 0 )
	{
		fcoll_SortDynamic( TRUE );

		vMovement.Negate();

		if( FColl_apSortedImpactBuf[0]->fUnitImpactTime > 0.0f )
		{
			vMovement.Mul( 1.0f - FColl_apSortedImpactBuf[0]->fUnitImpactTime );
		}
		else
		{
			if( vMovement.SafeUnitAndMag( vMovement ) )
			{
				vMovement.Mul( FColl_apSortedImpactBuf[0]->fImpactDistInfo );
			}
			else
			{
				vMovement.Zero();
			}
		}

		vCamPos.Add( vMovement );
	}

	// compute new camera matrix
	m_pCamInfo->m_pmtxMtx->UnitMtxFromUnitVec(&vLook);
	m_pCamInfo->m_pmtxMtx->m_vPos.Set( vCamPos );
	m_pCamInfo->m_fHalfFOV = m_fFOV;

//	CFCapsule capsule;
//	capsule.m_vPoint1.Set( vCamPos );
//	capsule.m_vPoint2.Set( pmTargetObject->m_vPos );
//	capsule.m_fRadius =	m_pParams->fCamColSphereRadius;
//	fdraw_DevCapsule( &capsule );
}

/*
//-----------------------------------------------------------------------------
// callback for fcoll collision.  Returns TRUE if collision should be performed
// on pTracker, FALSE if not.
BOOL CVehicleCamera::_TrackerCallback( CFWorldTracker *pTracker )
{
	u32 nIndex;

	for( nIndex = 0; nIndex < FWorld_nTrackerSkipListCount; nIndex++ )
	{
		if( FWorld_apTrackerSkipList[nIndex] == pTracker )
		{
			return FALSE;
		}
	}

	return TRUE;
}
*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
const FGameData_TableEntry_t CVehicle::m_aBotInfoVocab_VehiclePhysics[] = {
	//fMass;					
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	//fMaxSpeed;				
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000,

	//fMaxAccel;				
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000,

	//fLinearDampingHigh;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000,

	//fLinearDampingLow;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000,

	//fAngularDampingHigh;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000,

	//fAngularDampingLow;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000,

	//fBodyFriction;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//fBodyStickiness;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	//fRightingForceMag;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


//-----------------------------------------------------------------------------
const FGameData_TableEntry_t CVehicle::m_aBotInfoVocab_Vehicle[] = {
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupSoftHitGround
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupHardHitGround

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision dirt short
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision dirt long

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision rock short
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision rock long

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision junk short
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision junk long

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision metal short
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCollision metal long


	//fSoftCollMom					
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000000,

	//fHardCollMom				
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000000,

	//fSoftHitGroundMom
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000000,

	//fHardHitGroundMom
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000000,

	// hSmokeParticle:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//fImpulseFraction
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_20,

	//fDriverStationRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_20,

	//fGunnerStationRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_20,

	FGAMEDATA_VOCAB_ARMOR,	// pOccupantArmorProfile
	FGAMEDATA_VOCAB_ARMOR,	// pOccupantRacingArmorProfile

	// pszDamageProfile
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_DAMAGE,	// pRammingDamageProfile
	FGAMEDATA_VOCAB_DAMAGE,	// pPrimaryWeaponDamageProfile
	FGAMEDATA_VOCAB_DAMAGE,	// pSecondaryWeaponDamageProfile

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


//-----------------------------------------------------------------------------
const FGameData_TableEntry_t CVehicleCamera::m_aBotInfoVocab_VehicleCamera[] = {
	//fCamDistance
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamDistanceVel
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamDistanceAirVel
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamMinDistance
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamMaxDistance
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamDeadDistance
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamGroundRotRate
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamAirRotRate
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamDefaultHeight
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	//fCamMinHeight
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	//fCamMaxHeight
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	//fCamHeightVel
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fCamColSphereRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fLookForward
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fLookHeight
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	//fFOV
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CVehicle::m_aBotInfoVocab_Engine[] =
{
		FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEngineStart
		FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEngineStop
		FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEngineIdle
		FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEngineLow
		FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEngineHigh

		// fEngineIdleUnitWidth
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fEngineCrossfadeUnitCenter
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fEngineCrossfadeUnitWidth
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//fAirSpeedMul, fOOAirSpeedMul 
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
		sizeof( f32 ) * 2,
		F32_DATATABLE_0,
		F32_DATATABLE_10,
		
		//fRevRate
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100,

		//fAirRevRate
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100,

		//fVarianceChance
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//fVarianceMinMul
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		//fVarianceMaxMul
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10,

		//fLowMinVolume
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// End of table:
		FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};
