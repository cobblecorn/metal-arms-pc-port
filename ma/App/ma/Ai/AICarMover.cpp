#include "fang.h"
#include "floop.h"
#include "AICarMover.h"
#include "AIBrain.h"
#include "AINodePools.h"
#include "AIBrainMan.h"
#include "AIRooms.h"
#include "AIEdgeLock.h"
#include "AIGameUtils.h"
#include "AIMain.h"
#include "AIHazard.h"
#include "AIGraphSearcher.h"
#include "AIGroup.h"
#include "../player.h"
#include "../Entity.h"
#include "../bot.h"
#include "../meshtypes.h"
#include "../Vehiclerat.h"

#define _TACTIC_TIME		( 3.0f )

CAICarMover::CAICarMover(void)
: CAIBotMover(),
	m_uAICarMoverFlags(AICARMOVERFLAG_NONE)
{
	m_uMoverFlags |= MOVERFLAG_DISABLE_OBJECT_AVOIDANCE;	  //
}


CAICarMover::~CAICarMover(void)
{
}



void CAICarMover::AddToWorld(void)
{
	CAIBotMover::AddToWorld();

	m_fAtDestTime = 0.0f;
	m_fDesiredEdgeOffset = 0.0f;
	m_vEdgeLocation.Zero();
	m_ePlayerLocation = VEHICLE_LOCATION_AHEAD;
	m_eTactic = TACTIC_STAY_AHEAD;
	m_fTacticTimer = 0.0f;
	m_uAICarMoverFlags = 0;
}

void CAICarMover::RemoveFromWorld(void)
{
	CAIBotMover::RemoveFromWorld();
}


void CAICarMover::BeginFrame(void)	  //call this at the top of every game frame. It does important stuff
{
	CAIBotMover::BeginFrame();
}

void CAICarMover::_UpdateEstimatedLinkTraverseTime( void )
{
	CVehicle* pVehicle = (CVehicle*) m_pEntity;

	if( pVehicle )
	{
		f32 fCurrentSpeed = pVehicle->m_fSpeedXZ_WS;
		FMATH_CLAMPMIN( fCurrentSpeed, pVehicle->ComputeMinimumManeuveringSpeed() ); 
		FASSERT(fCurrentSpeed >= 0.0f);
		m_fLinkEstimateTime = DistToNextWay() / fCurrentSpeed;
	}
	else
	{
		f32 fMaxSpeed = m_pBot->GetMaxFlatSurfaceSpeed();
		if( fMaxSpeed > 0.0f )
		{
			m_fLinkEstimateTime = fmath_Div(DistToNextWay(), fMaxSpeed);	 //average feet per second of robot?
		}
		else
		{
			m_fLinkEstimateTime = fmath_Div(DistToNextWay(), 10.0f);
		}
	}

	m_fLinkStartTime = aiutils_FTotalLoopSecs();
}

void CAICarMover::AssignPath(CAIPath* pPath, u32 uAssignPathFlags /* =0 */)
{
	ResLock_FreeAllEdgeLocks(m_pBrain->GetGUID());
	m_pPath = pPath;
	if (m_pPath)
	{
		m_PathWalker.Init(m_pPath);	 //if following a path, track progress using this class

		//state information
		m_uJumpStage = JUMP_STAGE_NOPE;
		m_uEdgeType	= EF_EDGE_NORMAL;

		_UpdateEstimatedLinkTraverseTime();

		//reset the path related feedback flags
		m_uAssignPathFlags = uAssignPathFlags;

		if (!(HasCanHopPathFlags() && _StartHopPath()))	//whoever is assigning us to this path says we may hop it if possible
		{
			if (HasCanRollPathFlags())
			{
				_StartRollPath();
			}
		}

	}
	else
	{
		m_PathWalker.Init(NULL);
		m_uAssignPathFlags = 0;
	}
	m_nClientFeedback &= ~FEEDBACK_PATH_COMPLETE;
	m_nClientFeedback &= ~FEEDBACK_PATH_OBSTRUCTED;
	m_nClientFeedback &= ~FEEDBACK_PATH_SLOWPROGRESS;

}


#define _UNSTUCK_TIME	( 0.2f )
void CAICarMover::_DoStuckWork(void)
{
	CFVec3A vDeltaPos, vUnitDeltaPos;
	f32 fDeltaPosMag;
	f32 fDot;
	f32 fActualVelocity;
	CVehicle *pVehicle;

	pVehicle = (CVehicle*) m_pEntity;
	if( !m_pEntity || pVehicle->GetDriverBot() == NULL )
	{
		//clear the stuck timer
		m_uMoverFlags &= ~MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
		m_fStuckForSecs = 0.0f;
		return;
	}

	vDeltaPos.Sub(GetLoc(), m_pBot->m_MountPrevPos_WS);
	fDeltaPosMag = vUnitDeltaPos.SafeUnitAndMag( vDeltaPos );

	fActualVelocity = fDeltaPosMag * FLoop_fPreviousLoopOOSecs;

	fDot = 1.0f; 
		
	if( fDeltaPosMag >= 0.0f )
	{
		fDot = m_pEntity->MtxToWorld()->m_vFront.Dot( vUnitDeltaPos );
	}

	if( m_fSpeedXZ > 0.0f && !m_Controls.IsSpecial2() )	 //Hi Chris, you used to have IsDrivingInReverse() here, but I need to use special2 for other things sometimes.
	{
		if( (m_nClientFeedback & FEEDBACK_STOPPED) || fDot < 0.05f || fDeltaPosMag < 0.0f ||
			(fActualVelocity < pVehicle->GetMaxFlatSurfaceSpeed() * m_fSpeedXZ * 0.1f) )
		{
			//inc the stuck timer
			m_fStuckForSecs += FLoop_fPreviousLoopSecs;
			m_fUnStuckForSecs = 0.0f;
		}
		else
		{
			m_fUnStuckForSecs += FLoop_fPreviousLoopSecs;

			if( m_fUnStuckForSecs > _UNSTUCK_TIME )
			{
				//clear the stuck timer
				m_uMoverFlags &= ~MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
				m_fStuckForSecs = 0.0f;
			}
		}
	}
	else
	{
		m_fUnStuckForSecs += FLoop_fPreviousLoopSecs;

		if( m_fUnStuckForSecs > _UNSTUCK_TIME )
		{
			//clear the stuck timer
			m_uMoverFlags &= ~MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
			m_fStuckForSecs = 0.0f;
		}
	}

//	ftext_Printf( 0.15f, 0.56f, "~f1~C92929299~w1~al~s0.69Stuck %.2f", m_fStuckForSecs );
}

// Determine if rvPoint is inside turn radius fRadius of vehicle located at rvPos, with right orientation rvRight.
static BOOL _IsPointInsideTurnRadius( const CFVec3A &rvPos, const CFVec3A &rvPoint, const CFVec3A &rvRight, f32 fRadius )
{
	CFVec3A vTurnOrigin_MS, vTurnOrigin_WS, vDistance;

	// find model-space center of turn radius circle on the right side of vehicle
	vTurnOrigin_MS.Set( rvRight );
	vTurnOrigin_MS.Mul( fRadius );

	// construct world-space turn radius circle origin point on right side
	vTurnOrigin_WS.Set( vTurnOrigin_MS );
	vTurnOrigin_WS.Add( rvPos );

	// find distance from right side turn origin to target point
	vDistance.Set( rvPoint );
	vDistance.Sub( vTurnOrigin_WS );

	// determine if target point is inside turn radius on right side
	if( vDistance.Mag() < fRadius )
	{
		return TRUE;
	}
	else
	{
		// construct world-space turn radius circle origin point on left side
		vTurnOrigin_WS.Set( vTurnOrigin_MS );
		vTurnOrigin_WS.Negate();
		vTurnOrigin_WS.Add( rvPos );

		// find distance from left side turn origin to target point
		vDistance.Set( rvPoint );
		vDistance.Sub( vTurnOrigin_WS );

		// determine if target point is inside turn radius on left side
		if( vDistance.Mag() < fRadius )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// array indices for _ComputeWaypointVectors() 
enum
{
	_PREV_WAYPOINT = 0,
	_CURR_WAYPOINT,
	_NEXT_WAYPOINT,
	_WAYPOINT_COUNT,
};

#define _PSEUDO_WAYPOINT_LENGTH		( 75.0f )
// Fills paWayLocations with position of previous, current, and next waypoints.
// Fills paWayDirections with unit vectors pointing to next waypoint (i.e. previous-to-current, current-to-next, next-to-one-after-next)
// Fills paWayLengths with distance from one waypoint to next (i.e. previous-to-current, current-to-next, next-to-one-after-next)
// Returns FALSE if pWalker is invalid, or the function otherwise fails.
// If the previous waypoint doesn't exist, a pseudo waypoint is created behind the current
// waypoint, along the line defined by the current and next waypoints.
// A similar pseudo waypoint is created if the next waypoint doesn't exist.
// A pseudo direction is created for the _NEXT_WAYPOINT location, as the next-after-next location isn't computed.
static BOOL _ComputeWaypointVectors( CAIPathWalker *pWalker, CFVec3A *paWayLocations, CFVec3A *paWayDirections, f32 *paWayLengths, f32 *paWayWidths )
{
	CAIPathWaypoint *pWay;
	BOOL bPrevExists = TRUE;
	BOOL bNextExists = TRUE;

	if( !pWalker->IsValid() )
	{
		return FALSE;
	}

	// fill in current waypoint location
	pWay = pWalker->GetCurWaypoint();
	if( !pWay )
	{
		return FALSE;
	}
	paWayLocations[_CURR_WAYPOINT].Set( pWay->m_Location );

	paWayWidths[_PREV_WAYPOINT] = 0.0f;
	paWayWidths[_CURR_WAYPOINT] = 0.0f;
	paWayWidths[_NEXT_WAYPOINT] = 0.0f;

	if( pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH )
	{
		paWayWidths[_CURR_WAYPOINT] = pWay->m_fApproachWidth;
	}

	// fill in previous waypoint location, if it exists
	pWay = pWalker->GetPreviousWaypoint();
	if( pWay )
	{
		if( pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH )
		{
			paWayWidths[_PREV_WAYPOINT] = pWay->m_fApproachWidth;
		}

		paWayLocations[_PREV_WAYPOINT].Set( pWay->m_Location );
		if( paWayLocations[_PREV_WAYPOINT] == paWayLocations[_CURR_WAYPOINT] )
		{
			// not valid to have previous and current points be the same
			bPrevExists = FALSE;
		}
	}
	else
	{
		bPrevExists = FALSE;
	}

	// fill in next waypoint location, if it exists
	pWay = pWalker->GetNextWaypoint();
	if( pWay )
	{
		paWayLocations[_NEXT_WAYPOINT].Set( pWay->m_Location );

		if( pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH )
		{
			paWayWidths[_NEXT_WAYPOINT] = pWay->m_fApproachWidth;
		}

		if( paWayLocations[_NEXT_WAYPOINT] == paWayLocations[_CURR_WAYPOINT] )
		{
			// not valid to have current and next points be the same
			bNextExists = FALSE;
		}
	}
	else
	{
		bNextExists = FALSE;
	}

	if( !bPrevExists && !bNextExists )
	{
		// must have at least two valid waypoints
		return FALSE;
	}

	if( bPrevExists && bNextExists )
	{
		// construct direction unit vector for last waypoint
		paWayDirections[_PREV_WAYPOINT].Set( paWayLocations[_CURR_WAYPOINT] );
		paWayDirections[_PREV_WAYPOINT].Sub( paWayLocations[_PREV_WAYPOINT] );
		paWayLengths[_PREV_WAYPOINT] = paWayDirections[_PREV_WAYPOINT].Mag();
		paWayDirections[_PREV_WAYPOINT].y = 0.0f;
		paWayDirections[_PREV_WAYPOINT].Unitize();

		// construct direction unit vector for current waypoint
		paWayDirections[_CURR_WAYPOINT].Set( paWayLocations[_NEXT_WAYPOINT] );
		paWayDirections[_CURR_WAYPOINT].Sub( paWayLocations[_CURR_WAYPOINT] );
		paWayLengths[_CURR_WAYPOINT] = paWayDirections[_CURR_WAYPOINT].Mag();
		paWayDirections[_CURR_WAYPOINT].y = 0.0f;
		paWayDirections[_CURR_WAYPOINT].Unitize();

		// construct pseudo direction vector for next waypoint
		paWayDirections[_NEXT_WAYPOINT].Set( paWayDirections[_CURR_WAYPOINT] );

	}
	else if( bNextExists ) // previous doesn't exist
	{
		// construct direction unit vector for current waypoint
		paWayDirections[_CURR_WAYPOINT].Set( paWayLocations[_NEXT_WAYPOINT] );
		paWayDirections[_CURR_WAYPOINT].Sub( paWayLocations[_CURR_WAYPOINT] );
		paWayLengths[_CURR_WAYPOINT] = paWayDirections[_CURR_WAYPOINT].Mag();
		paWayDirections[_CURR_WAYPOINT].y = 0.0f;
		paWayDirections[_CURR_WAYPOINT].Unitize();

		// construct pseudo waypoint location for missing previous waypoint
		paWayLocations[_PREV_WAYPOINT].Set( paWayDirections[_CURR_WAYPOINT] );
		paWayLocations[_PREV_WAYPOINT].Negate();
		paWayLocations[_PREV_WAYPOINT].Mul( _PSEUDO_WAYPOINT_LENGTH );
		paWayLocations[_PREV_WAYPOINT].Add( paWayLocations[_CURR_WAYPOINT] );

		paWayLengths[_PREV_WAYPOINT] = _PSEUDO_WAYPOINT_LENGTH;

		// construct pseudo waypoint direction for missing previous waypoint
		paWayDirections[_PREV_WAYPOINT].Set( paWayDirections[_CURR_WAYPOINT] );

		// construct pseudo waypoint direction for next waypoint (would point toward next-after-next, which isn't computed)
		paWayDirections[_NEXT_WAYPOINT].Set( paWayDirections[_CURR_WAYPOINT] );
	}
	else // bPrevExists, next doesn't exist
	{
		// construct direction unit vector for last waypoint
		paWayDirections[_PREV_WAYPOINT].Set( paWayLocations[_CURR_WAYPOINT] );
		paWayDirections[_PREV_WAYPOINT].Sub( paWayLocations[_PREV_WAYPOINT] );
		paWayLengths[_PREV_WAYPOINT] = paWayDirections[_PREV_WAYPOINT].Mag();
		paWayDirections[_PREV_WAYPOINT].y = 0.0f;
		paWayDirections[_PREV_WAYPOINT].Unitize();

		// construct pseudo waypoint direction for current waypoint
		paWayDirections[_CURR_WAYPOINT].Set( paWayDirections[_PREV_WAYPOINT] );

		// construct pseudo waypoint direction for next waypoint
		paWayDirections[_NEXT_WAYPOINT].Set( paWayDirections[_PREV_WAYPOINT] );

		// construct pseudo waypoint location for missing next waypoint
		paWayLocations[_NEXT_WAYPOINT].Set( paWayDirections[_CURR_WAYPOINT] );
		paWayLocations[_NEXT_WAYPOINT].Mul( _PSEUDO_WAYPOINT_LENGTH );
		paWayLocations[_NEXT_WAYPOINT].Add( paWayLocations[_CURR_WAYPOINT] );

		paWayLengths[_CURR_WAYPOINT] = _PSEUDO_WAYPOINT_LENGTH;
	}

	paWayLengths[_NEXT_WAYPOINT] = _PSEUDO_WAYPOINT_LENGTH;
	return TRUE;
}

#if 0
// determine if pOtherVehicle is ahead of, along side, or behind pVehicle.
// fill in pfEdgeOffset with edge offset of pOtherVehicle.
static CAICarMover::VehicleLocation_t _VehicleRelativeLocation( CVehicle *pVehicle, CVehicle *pOtherVehicle, const CFVec3A &rvEdgeDirection )
{
	CFVec3A vTestPos, vDelta;
	CAICarMover::VehicleLocation_t eLocation;

	// compute a test position ahead of this vehicle along waypoint direction
	vTestPos.Set( rvEdgeDirection );
	vTestPos.Mul( pVehicle->m_fCollCylinderRadius_WS * 3.0f );
	vTestPos.Add( pVehicle->MtxToWorld()->m_vPos );

	vDelta.Set( pOtherVehicle->MtxToWorld()->m_vPos );
	vDelta.Sub( vTestPos );

	if( rvEdgeDirection.Dot( vDelta ) > 0.0f )
	{
		// other vehicle is ahead of this vehicle
		eLocation = CAICarMover::VEHICLE_LOCATION_AHEAD;
	}
	else
	{
		// compute a test position behind this vehicle along waypoint direction
		vTestPos.Set( rvEdgeDirection );
		vTestPos.Mul( pVehicle->m_fCollCylinderRadius_WS * -3.0f );
		vTestPos.Add( pVehicle->MtxToWorld()->m_vPos );

		vDelta.Set( pOtherVehicle->MtxToWorld()->m_vPos );
		vDelta.Sub( vTestPos );

		if( rvEdgeDirection.Dot( vDelta ) < 0.0f )
		{
			// other vehicle is behind this vehicle
			eLocation = CAICarMover::VEHICLE_LOCATION_BEHIND;
		}
		else
		{
			// other vehicle is next to this vehicle
			eLocation = CAICarMover::VEHICLE_LOCATION_BESIDE;
		}
	}

	return eLocation;
}
#else
static CAICarMover::VehicleLocation_t _VehicleRelativeLocation( CVehicle *pVehicle, CVehicle *pOtherVehicle, const CFVec3A &rvEdgeDirection )
{
	CFVec3A vTestPos, vDelta;
	CAICarMover::VehicleLocation_t eLocation;

	eLocation = CAICarMover::VEHICLE_LOCATION_BESIDE;

	// compute a test position behind this vehicle along waypoint direction
	vTestPos.Set( rvEdgeDirection );
	vTestPos.Mul( pVehicle->m_fCollCylinderRadius_WS * -3.0f );
	vTestPos.Add( pVehicle->MtxToWorld()->m_vPos );
	vDelta.Set( pOtherVehicle->MtxToWorld()->m_vPos );
	vDelta.Sub( vTestPos );
	if( rvEdgeDirection.Dot( vDelta ) < 0.0f )
	{
		// other vehicle is behind this vehicle
		eLocation = CAICarMover::VEHICLE_LOCATION_BEHIND;
	}
	else
	{
		vDelta.Set( pOtherVehicle->MtxToWorld()->m_vPos );
		vDelta.Sub( pVehicle->MtxToWorld()->m_vPos );
		if( vDelta.SafeUnitAndMag( vDelta ) > 0.0f )
		{
			if( rvEdgeDirection.Dot( vDelta ) > 0.707f )
			{
				eLocation = CAICarMover::VEHICLE_LOCATION_AHEAD;
			}
		}
	}

	return eLocation;
}
#endif

#define _VEHICLE_MIN_MANEUVERING_UNIT_SPEED		( 0.4f )	// minimum unit speed at which vehicle travels (NOTE: somehow convert to use ComputeMinimumManeuveringSpeed()!)
#define _VEHICLE_MAX_MANEUVERING_UNIT_SPEED		( 0.65f )	// below this unit speed, vehicle is considered to be "maneuvering" to get on path instead of "cruising" on path
#define _MIN_WAYPOINT_LENGTH					( 30.0f )	// a waypoint that is less than this distance from prior waypoint will be ignored
#define _MIN_AT_DESTINATION_TIME				( 0.35f )	// amount of time that must pass while vehicle stopped at goto point before considered "at destination"
#define _EDGE_OFFSET_STEERING_STRENGTH			( 0.05f )	// how strongly vehicle will steer towards desired offset from graph edge.  0.0 to 1.0
#define _FOLLOW_AHEAD_DISTANCE					( 100.0f )	// distance AI vehicle should try to stay ahead of player vehicle in TACTIC_STAY_AHEAD
#define _FOLLOW_BESIDE_DISTANCE					( 20.0f )	// distance AI vehicle should try to stay ahead of player vehicle in TACTIC_STAY_BESIDE
#define _FOLLOW_BEHIND_DISTANCE					( -40.0f )	// distance AI vehicle should try to stay behind player vehicle in TACTIC_STAY_BEHIND
#define _THROTTLE_ADJUST_DISTANCE				( 100.0f )	// delta from target distance at which 1.0 will be added/subtracted from throttle
#define _MAX_CATCH_UP_UNIT_SPEED				( 2.0f )	// max speed that AI vehicle will attain in order to catch up to player
// returns TRUE when Mover is moving directly at vGotoPoint
BOOL CAICarMover::MoveToward(const CFVec3A& vGoto, f32* pfVelAlgnmnt/* = NULL*/)			
{
	CFVec3A vUnitDeltaToDestXZ;  
	CFVec3A vTemp;
	CFVec3A vGotoPoint;
	CFVec3A vGotoPointXZ;
	CFVec3A vVehiclePos;
	CFVec3A MountPos;
	CFVec3A lookAtPt;
	CFVec3A vWaypointLocation[_WAYPOINT_COUNT];
	CFVec3A vWaypointDirection[_WAYPOINT_COUNT];
	CFVec3A vColor, vStart, vEnd;
	f32 afWaypointLength[_WAYPOINT_COUNT];
	f32 afWaypointWidth[_WAYPOINT_COUNT];
	f32 fDistToDest;
	f32 fDistToEdge;
	BOOL bGoodWaypoints;
	f32 fMaxOffset = 0.0f;
	f32 fDesiredOffset = m_fDesiredEdgeOffset;
	f32 fEdgeWidth = 0.0f;
	CVehicleRat *pThisRAT = (CVehicleRat*) m_pEntity;
	FASSERT( pThisRAT );

	// flip vehicle rightside up if necessary and appropriate
	if( pThisRAT->GetDriverBot() && level_IsRacingLevel() && pThisRAT->IsUpsideDown() )
	{
		pThisRAT->FlipRightsideUp();
	}

	// make a copy of the goto point, it might get modified by the desired edge offset.
	vGotoPoint.Set( vGoto );

	m_uBotMoverFlags |= BOTMOVERFLAG_RECEIVED_EXTERNAL_MOVE_GOAL;

	// attempt to generate a set of data for previous, current, and next waypoints.
	// Function will try to fill in missing values with reasonable substitutes.
	bGoodWaypoints = _ComputeWaypointVectors( &m_PathWalker, vWaypointLocation, vWaypointDirection, afWaypointLength, afWaypointWidth );

	// compute edge points and distances
	if( bGoodWaypoints )
	{
		// compute WS point on edge that is nearest to vehicle
		CFVec3A vWayToVehicle;
		vWayToVehicle.Set( pThisRAT->MtxToWorld()->m_vPos );
		vWayToVehicle.Sub( vWaypointLocation[_CURR_WAYPOINT] );
		m_vEdgeLocation = vWaypointDirection[_PREV_WAYPOINT];
		m_vEdgeLocation.Mul( vWaypointDirection[_PREV_WAYPOINT].Dot(vWayToVehicle) );
		m_vEdgeLocation.Add( vWaypointLocation[_CURR_WAYPOINT] );
#if 0
vColor.Set( 1.0f, 0.0f, 0.0f );
vStart.Set( m_vEdgeLocation );
vStart.y += 10.0f;
pThisRAT->m_PhysObj.DrawSphere( &vStart, 3.0f, &vColor );
#endif

		// find vehicle's offset from current waypoint edge
		vWayToVehicle.Set( pThisRAT->MtxToWorld()->m_vPos );
		vWayToVehicle.Sub( vWaypointLocation[_PREV_WAYPOINT] );
		vTemp.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
		vTemp.Unitize();	// vWaypointDirection array always contains valid vectors
		fDistToEdge = vTemp.Dot( vWayToVehicle );
#if 0
vStart.Set( vTemp );
vStart.Mul( fDistToEdge );
vStart.Add( m_vEdgeLocation );
vStart.y += 10.0f;
vColor.Set( 0.0f, 0.0f, 1.0f );
pThisRAT->m_PhysObj.DrawSphere( &vStart, 3.0f, &vColor );
#endif

		// as a default, set edge width to vehicle's current edge offset.
		fEdgeWidth = FMATH_FABS( fDistToEdge );	
		// if an actual edge width is available from waypoint data, use that instead
		if( afWaypointWidth[_CURR_WAYPOINT] != 0.0f )
		{
			fEdgeWidth = afWaypointWidth[_CURR_WAYPOINT];
		}
		else if( afWaypointWidth[_PREV_WAYPOINT] != 0.0f )
		{
			fEdgeWidth = afWaypointWidth[_PREV_WAYPOINT];
		}

		// compute maximum possible offset for this vehicle on this edge. fMaxOffset is initialized at top of function.
		fMaxOffset = fEdgeWidth - ( 1.5f * pThisRAT->m_fCollCylinderRadius_WS );

		// compute new desired offset, respecting maximum possible offset. fDesiredOffset is initialized at top of function.
		FMATH_BIPOLAR_CLAMPMAX( fDesiredOffset, fMaxOffset );

#if 0
		// debug draw colored spheres at each waypoint
		vColor.Set( 1.0f, 0.0f, 0.0f );
		pThisRAT->m_PhysObj.DrawSphere( &vWaypointLocation[_PREV_WAYPOINT], 2.0f, &vColor );

		vColor.Set( 0.0f, 1.0f, 0.0f );
		pThisRAT->m_PhysObj.DrawSphere( (CFVec3A*) &vGotoPoint, 2.0f, &vColor );

		vColor.Set( 0.0f, 0.0f, 1.0f );
		pThisRAT->m_PhysObj.DrawSphere( &vWaypointLocation[_NEXT_WAYPOINT], 2.0f, &vColor );
#endif
	}

	// start our (upcoming) throttle calculations with a default full-speed value
	m_fSpeedXZ = 1.0f;

	// Compute WS unit vector representing direction in which vehicle should drive,
	// (First find current and goto XZ positions.)
	vVehiclePos = GetLoc();
	vVehiclePos.y = 0;
	vGotoPointXZ = vGotoPoint;
	vGotoPointXZ.y = 0.0f;
	if( bGoodWaypoints && vVehiclePos != vGotoPointXZ )
	{
		//If available, factor desired edge offset into direction vector.
		CFVec3A vOffset;
		vOffset.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
		vOffset.Mul( fDesiredOffset );
		vGotoPointXZ.Add( vOffset );
		vGotoPoint.Add( vOffset );
	}
	// now calculate WS unit direction vector and distance remaining to goto point
	vTemp.Sub( vGotoPointXZ, vVehiclePos );
	fDistToDest = vUnitDeltaToDestXZ.SafeUnitAndMag( vTemp );

	if( fDistToDest > 0.0f )
	{
		// vehicle position is not exactly equal to goto position

		// reset the at-destination counter
		m_fAtDestTime = 0.0f;

		// steer vehicle towards its desired left/right offset from the graph edge
		if( bGoodWaypoints )
		{
			f32 fPlayerEdgeOffset;
			CVehicleRat *pPlayerRat = (CVehicleRat*) aigroup_FindPlayerVehicle( ENTITY_BIT_VEHICLERAT );

			if( pPlayerRat && m_pBrain->GetAttrib( CAIBrain::ATTRIB_AGGRESSION ) < 90 )
			{
				// found a player vehicle and aggression is not set to ignore player
				CFVec3A vDelta;

				m_fTacticTimer -= FLoop_fPreviousLoopSecs;
				if( m_fTacticTimer <= 0.0f )
				{
					if( pThisRAT->AIShouldStayBehind() )
					{
						m_eTactic = TACTIC_STAY_BEHIND;
					}
					else if( pThisRAT->AIShouldStayBeside() )
					{
						m_eTactic = TACTIC_STAY_BESIDE;
					}
					else
					{
						m_eTactic = TACTIC_STAY_AHEAD;
					}

					m_fTacticTimer = _TACTIC_TIME;
				}
#if 0
if( m_eTactic == TACTIC_STAY_AHEAD )
ftext_Printf( 0.15f, 0.47f, "~f1~C92929299~w1~al~s0.80STAY AHEAD" );
else if( m_eTactic == TACTIC_STAY_BESIDE )
ftext_Printf( 0.15f, 0.47f, "~f1~C92929299~w1~al~s0.80STAY BESIDE" );
else
ftext_Printf( 0.15f, 0.47f, "~f1~C92929299~w1~al~s0.80STAY BEHIND" );
#endif

				m_ePlayerLocation = _VehicleRelativeLocation(	pThisRAT, pPlayerRat, vWaypointDirection[_PREV_WAYPOINT] );

				// find other vehicle's offset from current waypoint edge
				vDelta.Set( pPlayerRat->MtxToWorld()->m_vPos );
				vDelta.Sub( vWaypointLocation[_PREV_WAYPOINT] );
				vTemp.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
				vTemp.Unitize();	// vWaypointDirection array always contains valid vectors
				fPlayerEdgeOffset = vTemp.Dot( vDelta );
//				ftext_Printf( 0.15f, 0.55f, "~f1~C92929299~w1~al~s0.80Player Edge Offset %.2f", fPlayerEdgeOffset );

				// find AI vehicle distance ahead/behind a 45 degree field-of-view cone
				CFVec3A vPerpEdgeDir;
				f32 fXDist, fZDist;
				f32 fDistance;
				vDelta.Set( pThisRAT->MtxToWorld()->m_vPos );
				vDelta.Sub( pPlayerRat->MtxToWorld()->m_vPos );
				vPerpEdgeDir.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
				fXDist = vPerpEdgeDir.Dot( vDelta );
				fZDist = vWaypointDirection[_PREV_WAYPOINT].Dot( vDelta );
				fDistance = fZDist - FMATH_FABS( fXDist );

				// compute speed of player along waypoint edge
				f32 fPlayerSpeed = vWaypointDirection[_PREV_WAYPOINT].Dot( pPlayerRat->m_Velocity_WS );

				if( m_ePlayerLocation == VEHICLE_LOCATION_AHEAD && m_eTactic != TACTIC_STAY_BEHIND )
				{
					// vehicle needs to catch up to player
					m_fSpeedXZ = 1.0f;
				}
				else
				{
					// compute throttle value needed to match speed of player
					if (m_pBot->m_fMaxFlatSurfaceSpeed_WS > 0.0f)
					{
						m_fSpeedXZ = fmath_Div( fPlayerSpeed, pThisRAT->m_fMaxFlatSurfaceSpeed_WS );
					}
					else
					{
						m_fSpeedXZ = 0.0f;
					}

					FMATH_CLAMP_UNIT_FLOAT( m_fSpeedXZ );
				}

				f32 fFollowDistance = 0.0f;
				switch( m_eTactic )
				{
					case TACTIC_STAY_AHEAD:
						fFollowDistance = _FOLLOW_AHEAD_DISTANCE;
						break;

					case TACTIC_STAY_BESIDE:
						fFollowDistance = _FOLLOW_BESIDE_DISTANCE;
						break;

					case TACTIC_STAY_BEHIND:
						fFollowDistance = _FOLLOW_BEHIND_DISTANCE;
						break;

					default:
						FASSERT_MSG( FALSE, "AI RAT has invalid tactic" );
						break;
				}

				// modify throttle so vehicle will try to attain desired position relative to player RAT
				m_fSpeedXZ *= 1.0f + fmath_Div( fFollowDistance - fDistance, _THROTTLE_ADJUST_DISTANCE );

				if( m_ePlayerLocation == VEHICLE_LOCATION_AHEAD )
				{
					// let vehicle catch up if player is ahead
					FMATH_CLAMP( m_fSpeedXZ, 0.0f, _MAX_CATCH_UP_UNIT_SPEED );
				}
				else
				{
					FMATH_CLAMP( m_fSpeedXZ, 0.0f, 1.2f );
				}

//ftext_Printf( 0.15f, 0.58f, "~f1~C92929299~w1~al~s0.80m_fSpeedXZ %.2f", m_fSpeedXZ );

				// see if vehicle has a clear "lane" (to pass player or to avoid being rammed by player).
				if( FMATH_FABS( fPlayerEdgeOffset - fDistToEdge ) < 2.0f * pThisRAT->m_fCollCylinderRadius_WS )
				{
					f32 fNewOffset;
					BOOL bFoundGood = FALSE;

					if( fDistToEdge >= fPlayerEdgeOffset )
					{
						fNewOffset = fPlayerEdgeOffset + (3.0f * pThisRAT->m_fCollCylinderRadius_WS);
						if( fNewOffset <= fMaxOffset )
						{
							bFoundGood = TRUE;
						}
					}
					else
					{
						fNewOffset = fPlayerEdgeOffset - (3.0f * pThisRAT->m_fCollCylinderRadius_WS);
						if( fNewOffset >= -fMaxOffset )
						{
							bFoundGood = TRUE;
						}
					}

					if( bFoundGood )
					{
						m_fDesiredEdgeOffset = fNewOffset;
					}
					else
					{
						if( m_ePlayerLocation == VEHICLE_LOCATION_BESIDE )
						{
							// We are beside player and there isn't room for us.
							// Slow down to avoid getting bumped off graph.
							m_fSpeedXZ *= 0.5f;
						}
						else
						{
							// Try to move to a left/right position on graph edge
							// that will give vehicle a clear lane.
							if( fPlayerEdgeOffset >= 0.0f )
							{
								m_fDesiredEdgeOffset = fPlayerEdgeOffset - (2.5f * pThisRAT->m_fCollCylinderRadius_WS);
							}
							else
							{
								m_fDesiredEdgeOffset = fPlayerEdgeOffset + (2.5f * pThisRAT->m_fCollCylinderRadius_WS);
							}
							FMATH_BIPOLAR_CLAMPMAX( m_fDesiredEdgeOffset, fMaxOffset );
						}
					}
				}
#if 0
CFVec3A vStart;
vStart.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
vStart.Mul( m_fDesiredEdgeOffset );
vStart.Add( m_vEdgeLocation );
vStart.y += 4.0f;
pThisRAT->m_PhysObj.DrawSphere( &vStart, 3.0f );
#endif
			}
			else
			{
				// move to the center of the graph
				fDesiredOffset = 0.0f;
				m_fDesiredEdgeOffset = fDesiredOffset;
			}

			// adjust WS steering vector to steer vehicle towards desired edge offset
			// by adding a vector perpendicular to the base steering vector which is
			// scaled in proportion to the difference between the current and desired
			// edge offsets.
			f32 fScale = fmath_Div( m_fDesiredEdgeOffset - fDistToEdge, 2.0f * fEdgeWidth );
			fScale *= _EDGE_OFFSET_STEERING_STRENGTH;
//			FMATH_BIPOLAR_CLAMPMAX( fScale, _EDGE_OFFSET_STEERING_STRENGTH );
			vTemp.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
			vTemp.Mul( fScale );
			vTemp.Add( vUnitDeltaToDestXZ );
			vUnitDeltaToDestXZ.SafeUnitAndMag( vTemp );
#if 0
			// draw some debug lines
			vStart.Set( pThisRAT->MtxToWorld()->m_vPos );
			vStart.y += 15.0f;
			vEnd.Set( vTemp );
			vEnd.Mul( 100.0f );
			vEnd.Add( vStart );
			pThisRAT->m_PhysObj.DrawLine( &vStart, &vEnd );

			vColor.Set( 1.0f, 1.0f, 1.0f );
			pThisRAT->m_PhysObj.DrawSphere( &m_vEdgeLocation, 2.0f, &vColor );

			vColor.Set( 0.5f, 0.5f, 0.5f );
			vTemp.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
			vTemp.Mul( fDesiredOffset );
			vTemp.Add( m_vEdgeLocation );
			pThisRAT->m_PhysObj.DrawSphere( &vTemp, 3.0f, &vColor );

			vColor.Set( 0.25f, 0.25f, 0.25f );
			vTemp.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
			vTemp.Mul( fDistToEdge );
			vTemp.Add( m_vEdgeLocation );
			pThisRAT->m_PhysObj.DrawSphere( &vTemp, 3.0f, &vColor );

			vColor.Set( 0.1f, 0.1f, 0.1f );
			vTemp.Set( vWaypointDirection[_PREV_WAYPOINT].z, 0.0f, -vWaypointDirection[_PREV_WAYPOINT].x );
			vTemp.Mul( fMaxOffset );
			vTemp.Add( m_vEdgeLocation );
			pThisRAT->m_PhysObj.DrawSphere( &vTemp, 3.0f, &vColor );

			vStart.Set( pThisRAT->MtxToWorld()->m_vPos );
			vStart.y += 15.0f;
			vEnd.Set( vUnitDeltaToDestXZ );
			vEnd.Mul( 15.0f );
			vEnd.Add( vStart );
			pThisRAT->m_PhysObj.DrawLine( &vStart, &vEnd );
#endif
		}
		else
		{
			// no waypoints
			// just keep goin' straight
			vUnitDeltaToDestXZ.Set( pThisRAT->MtxToWorld()->m_vFront );
		}
	}
	else
	{
		// goto point is same as vehicle position
		fDistToDest = 0.0f;
		// just keep goin' straight
		vUnitDeltaToDestXZ.Set( pThisRAT->MtxToWorld()->m_vFront );

		// increment the at-destination time
		m_fAtDestTime += FLoop_fPreviousLoopSecs;
	}

	f32 fAlignment = 1.0f;
	if( m_pBot->m_fSpeedXZ_WS > 0.0f && fDistToDest > 0.0f )
	{
		fAlignment = pThisRAT->m_UnitVelocityXZ_WS.Dot( vUnitDeltaToDestXZ );
	}

	//
	// Speed control
	//

	if( bGoodWaypoints )
	{
		f32 fCosWaypointVel = 1.0f;
		f32 fDeltaSpeed, fNewSpeed;
		BOOL bCheckForTurn = TRUE;
		BOOL bNextNext = TRUE;

		// adjust vehicle speed based on how closely vehicle is aligned with the current waypoint path segment
		if( m_pBot->m_fSpeed_WS > 0.0f )
		{
			fCosWaypointVel = vWaypointDirection[_PREV_WAYPOINT].Dot( m_pBot->m_UnitVelocity_WS );
		}
		else
		{
			bCheckForTurn = FALSE;
		}

		if( m_PathWalker.GetNextNextWaypoint() == NULL )
		{
			// if next-next is null, then next is end of path, and therefore
			// next is spline knot.  We don't actually travel to spline knot,
			// so no need to check for turn.
			bCheckForTurn = FALSE;
			bNextNext = FALSE;
		}

		// adjust speed based on how well aligned the vehicle is with the path,
		// and see if we need to slow down for an upcoming turn.
		if( bCheckForTurn && fCosWaypointVel > _VEHICLE_MIN_MANEUVERING_UNIT_SPEED )
		{
			f32 fDot;

			// make basic speed proportional to vehicle's angle with current graph edge
			m_fSpeedXZ *= fCosWaypointVel;

			// Find angle between current waypoint and next.
			// This tells us if we have a turn coming up soon.
			// fDot will be used to scale speed down for turn.
			fDot = vWaypointDirection[_CURR_WAYPOINT].Dot( vWaypointDirection[_PREV_WAYPOINT] );
			if( fDot > 0.0f )
			{
				// scale speeds down faster
				fDot = fDot * fDot * fDot * fDot;
			}
			else
			{
				// turn is greater than 90 degrees, maximum slow down
				fDot = 0.0f;
			}

			// construct delta target speed for turn, positive number is how much to slow down
			fDeltaSpeed = m_fSpeedXZ - fDot;

//			ftext_Printf( 0.25f, 0.52f, "~f1~C92929299~w1~al~s0.69Target Turn Speed %.2f", m_fSpeedXZ - fDeltaSpeed );

			if( fDeltaSpeed > 0.0f )
			{
				// scale slowdown amount by distance to upcoming turn
				fDeltaSpeed = fDeltaSpeed - ( fDeltaSpeed * fmath_Div( fDistToDest, afWaypointLength[_PREV_WAYPOINT] ) );

				// construct new target speed for turn (as scaled by distance to turn)
				fNewSpeed = m_fSpeedXZ - fDeltaSpeed;

				// if turn speed required is lower than previously set speed, make turn speed the new speed
				if( fNewSpeed < m_fSpeedXZ )
				{
					m_fSpeedXZ = fNewSpeed;
				}
			}

			// if next waypoint will be within turning radius of vehicle at currently determined
			// speed for upcoming turn, then slow down a lot, to minimum speed
			CFVec3A vRight;
			vRight.x =  vWaypointDirection[_CURR_WAYPOINT].z;
			vRight.z = -vWaypointDirection[_CURR_WAYPOINT].x;
			vRight.y = 0;
			vRight.Unitize();
			if( m_fSpeedXZ < _VEHICLE_MAX_MANEUVERING_UNIT_SPEED && _IsPointInsideTurnRadius( vGotoPoint, 
											vWaypointLocation[_NEXT_WAYPOINT], 
											vRight, 
											pThisRAT->ComputeEstimatedMinimumTurnRadius( m_fSpeedXZ * pThisRAT->GetMaxFlatSurfaceSpeed() ) ) )
			{
				m_fSpeedXZ = _VEHICLE_MIN_MANEUVERING_UNIT_SPEED;
			}

		}
		else	// not checking for upcoming turn
		{
			// If reason we skipped turn check is that the next waypoint is
			// the upcoming spline knot, then just make speed proportional to fDot.
			if( !bNextNext )
			{
				m_fSpeedXZ = fCosWaypointVel;
				FMATH_CLAMPMIN( m_fSpeedXZ, _VEHICLE_MIN_MANEUVERING_UNIT_SPEED );
			}
			else
			{
				// vehicle speed is zero
				m_fSpeedXZ = _VEHICLE_MIN_MANEUVERING_UNIT_SPEED;
			}
		}
	}
	else	
	{
		// bGoodWaypoints is FALSE so vehicle is 
		//probably off-graph or otherwise needs to maneuver slowly
		m_fSpeedXZ = _VEHICLE_MIN_MANEUVERING_UNIT_SPEED;
	}

//ftext_Printf( 0.15f, 0.36f, "~f1~C92929299~w1~al~s0.69m_fSpeedXZ %.2f", m_fSpeedXZ );

	// If stuck, try to get unstuck by backing up and doing a "j turn" for a 
	// fixed amount of time.  This algorithm could probably use some improvement.
	if( m_uAICarMoverFlags & AICARMOVERFLAG_STUCK_EXTRACTION )
	{
		m_Controls.Special2();
		m_fSpeedXZ = _VEHICLE_MIN_MANEUVERING_UNIT_SPEED;

		// turn wheels to right or left depending on location of waypoint
		vUnitDeltaToDestXZ.Set( pThisRAT->MtxToWorld()->m_vRight );
		if( pThisRAT->MtxToWorld()->m_vRight.Dot( vGotoPointXZ ) < 0.0f )
		{
			vUnitDeltaToDestXZ.Negate();
		}

		// decrement stuck counter and exit if done
		m_fStuckCount -= FLoop_fPreviousLoopSecs;
		if( m_fStuckCount < 0.0f )
		{
			m_uAICarMoverFlags &= ~AICARMOVERFLAG_STUCK_EXTRACTION;
		}
	}
	else
	{
		// activate "stuck" mode if stuck for too long.
		if( IsStuck(2.5f) )
		{
			m_uAICarMoverFlags |= AICARMOVERFLAG_STUCK_EXTRACTION;
			m_fStuckCount = 2.0f;
		}
	}

	// Handle case where target point is inside vehicle turn radius.
	// Don't check if target point is same as vehicle position, or if
	// travelling above max maneuvering speed.
	if( fDistToDest > 0.0f &&  m_fSpeedXZ < _VEHICLE_MAX_MANEUVERING_UNIT_SPEED && 
		_IsPointInsideTurnRadius( pThisRAT->MtxToWorld()->m_vPos, 
								vGotoPoint, 
								pThisRAT->MtxToWorld()->m_vRight, 
								pThisRAT->ComputeEstimatedMinimumTurnRadius( pThisRAT->m_fSpeed_WS ) ) )
	{
		m_Controls.Special2();
		m_fSpeedXZ = _VEHICLE_MIN_MANEUVERING_UNIT_SPEED;
//ftext_Printf( 0.15f, 0.56f, "~f1~C92929299~w1~al~s0.69Inside Turn Radius" );
	}

	if( m_fAtDestTime > _MIN_AT_DESTINATION_TIME )
	{
		// come to a stop if vehicle position = goto position for more
		// than some minimum time.
		m_fSpeedXZ = 0.0f;
//ftext_Printf( 0.15f, 0.52f, "~f1~C92929299~w1~al~s0.69AtDestination" );
	}

	// prevent brief uncontrolled swerves at high speed
#if 0
	if( m_fSpeedXZ > _VEHICLE_MAX_MANEUVERING_UNIT_SPEED )
	{
		if( vUnitDeltaToDestXZ.Dot( pThisRAT->MtxToWorld()->m_vFront ) < 0.707f )
		{
			m_fSpeedXZ = _VEHICLE_MIN_MANEUVERING_UNIT_SPEED;
			vUnitDeltaToDestXZ.Set( pThisRAT->MtxToWorld()->m_vFront );
		}
	}
#endif

	// set controls based on m_fSpeedXZ and vUnitDeltaToDestXZ
	CFVec3A PushVec;
	PushVec.SafeUnitAndMag( vUnitDeltaToDestXZ );
	PushVec.Mul(m_fSpeedXZ);

	m_afDXForce[FORCETYPE_GOAL] = PushVec.x;
	if (m_nNumDXForces <= FORCETYPE_GOAL)
	{
		m_nNumDXForces = FORCETYPE_GOAL;
	}
	m_afDZForce[FORCETYPE_GOAL] = PushVec.z;
	if (m_nNumDZForces <= FORCETYPE_GOAL)
	{
		m_nNumDZForces = FORCETYPE_GOAL;
	}

	if (pfVelAlgnmnt)
	{
		*pfVelAlgnmnt = fAlignment;
	}

	return FALSE;
}


BOOL CAICarMover::FaceToward(const CFVec3A& Location, f32* pfTorsoAlgnmnt/* = NULL*/)
{
	return TRUE;  //cars have no way to face toward right now, since they cant turn in place, special code would be needed to actually make them back up a little or something.
} 


CAIPathWaypoint* CAICarMover::CarPath_DoWayPtProgress(void)
{
	BOOL bBumpedCloseEnough = FALSE;
	BOOL bPassedWaypoint = FALSE;
	CAIPathWaypoint *pWay = m_PathWalker.GetCurWaypoint();
	FASSERT(pWay);

	// if I am following a path, and bump into someone who is standing on the waypoint I'm trying to get to,
	// then consider myself to be close enough
	if( m_pAvoidThis &&
		m_PathWalker.CloseEnoughXZ(CFSphereA(aiutils_GetEntityLoc(m_pAvoidThis), aiutils_GetEntityRadXZ(m_pAvoidThis))))
	{
		bBumpedCloseEnough = TRUE;
	}

	if( pWay )
	{
		CFVec3A vCurrWaypoint;
		vCurrWaypoint.Set( pWay->m_Location );

		CAIPathWaypoint *pPrevWay = m_PathWalker.GetPreviousWaypoint();
		if( pPrevWay )
		{
			CFVec3A vTemp, vWaypointDir;
			f32 fMag;

			// compute unit vector pointing from previous to current waypoint
			vTemp.Set( vCurrWaypoint );
			vTemp.Sub( pPrevWay->m_Location );
			fMag = vWaypointDir.SafeUnitAndMag( vTemp );

			if( fMag > 0.0f )
			{
				{
					// find delta vector from current waypoint to vehicle location
					vTemp.Set( GetLoc() );
					vTemp.Sub( vCurrWaypoint );
					if( vWaypointDir.Dot( vTemp ) > -0.2f )	//100 degrees
					{
						// vehicle has gone past the current waypoint.
						bPassedWaypoint = TRUE;
					}
				}
			}
		}
	}

	if (bBumpedCloseEnough || bPassedWaypoint ||
//		m_PathWalker.CloseEnoughXZ(CFSphereA(GetLoc(), GetRadiusXZ())))
		m_PathWalker.CloseEnoughXZ(CFSphereA(m_vEdgeLocation, GetRadiusXZ())))
	{
		//
		//
		//	We're close enough to the waypoint, that normally, we would advance to the next one, 
        //	but first, let's check some special conditions that might superceed
		//	being close to the waypoint.  
		//
		//
		
		//base class would like to keep track of verts as they are reached.
		if (pWay && pWay->m_pGraphVert)
		{
			SetLastGraphVertId(m_pPath->GetGraph()->GetVertId(pWay->m_pGraphVert));
		}

		//  no conditions yet


		//
		//	Made it this far eh, then I guess we really are going to
		//  start heading toward the next waypoint.
		//
		//	Now is the chance to cleanup any state type vbls related to the edge
		//  we were just following

		//  no special states yet.

		if (!m_PathWalker.Advance())  //when m_PathWalker.Advance returns FALSE, the last waypoint in the path has been reached
		{
			pWay = NULL;
		}
		else
		{
			if( m_PathWalker.GetNextWaypoint() && m_PathWalker.GetNextNextWaypoint() )
			{
				CAIPathWaypoint* pLastWay = pWay;
				pWay = m_PathWalker.GetCurWaypoint();
				FASSERT(pWay);

				if (pLastWay && pLastWay->m_pGraphVert && pWay->m_pGraphVert)
				{
					GraphEdge* pE = pLastWay->m_pGraphVert->GetEdgeTo(m_pPath->GetGraph()->GetVertId(pWay->m_pGraphVert));
					//base class would like to keep track of edges as they are reached.
					SetLastGraphEdgeId(m_pPath->GetGraph()->GetEdgeId(pE));
				}

				_UpdateEstimatedLinkTraverseTime();
//				CAIPathWaypoint *pLastWay = m_PathWalker.GetPreviousWaypoint();
//				//Is there anything special (like, is it a jump or anything?) about the edge we're about to start following
//				if (pLastWay)
//				{
//					GraphEdge* pE = NULL;
//					if (pLastWay->m_pGraphVert && pWay->m_pGraphVert)
//					{
//						pE = pLastWay->m_pGraphVert->GetEdgeTo(aimain_pAIGraph->GetVertId(pWay->m_pGraphVert));
//					}
//				}
			}
			else
			{
				// This is the last waypoint for this path.  In the vehicle case, the last 
				// waypoint is the spline knot location itself.  We don't actually want to 
				// drive to the spline knot position, so we're done with this path.
				pWay = NULL;
			}
		}
	}

	return pWay;
}


BOOL CAICarMover::CarPath_FollowNormalEdge(void)
{
	CAIPathWaypoint* pWay = NULL;
	CAIPathWaypoint* pLastWay = NULL;
	BOOL bIsDone = TRUE;

	if (m_pPath &&
		(pWay = CarPath_DoWayPtProgress())) //Get the waypoint we are approaching?  Advance it when necessary.
	{
		bIsDone = FALSE;

		BOOL bInCur = TRUE;
		BOOL bInLast = TRUE;
		if (pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH)
		{
			bInCur = (pWay->m_pGraphVert &&
							pWay->m_Location.DistSqXZ(GetLoc()) < pWay->m_fCloseEnoughDist* pWay->m_fCloseEnoughDist);
			pLastWay = m_PathWalker.GetPreviousWaypoint();
			bInLast = (pLastWay &&
							pLastWay->m_pGraphVert &&
							pLastWay->m_Location.DistSqXZ(GetLoc()) < pLastWay->m_fCloseEnoughDist*pLastWay->m_fCloseEnoughDist);
		}
		
		//only do the edge contraint if we're not within the radius of either the last or the next waypoint
		if (!bInCur &&
			!bInLast)
		{
			//this tweaks the goto pt if bot is currently outside the edge,
			//so that it will get back within the edge asap
			MoveTowardUseApproachConstraint(pWay->m_Location, pWay->m_ApproachDirUnit, pWay->m_fApproachWidth);
		}
		else
		{
			MoveToward(pWay->m_Location);
		}
	}
	else
	{
		MoveToward(GetLoc());
	}

	return bIsDone;
}


BOOL CAICarMover::FollowPath(void)
{
	BOOL bDone = FALSE;
	if (!m_pPath || !m_PathWalker.IsValid())
	{
		bDone = TRUE;
	}
	else
	{
		bDone = CarPath_FollowNormalEdge();
	}

	if (bDone)
	{
		m_nClientFeedback |= FEEDBACK_PATH_COMPLETE;
	}

	if (aiutils_FTotalLoopSecs() - m_fLinkStartTime > m_fLinkEstimateTime*2.0f)
	{	//probably got a problem here.  It has been twice as long as the original estimate for me to reach the goal
		m_nClientFeedback |= FEEDBACK_PATH_SLOWPROGRESS;
	}
	else
	{
		m_nClientFeedback &= ~FEEDBACK_PATH_SLOWPROGRESS;
	}
	
	return bDone != 0;
}


void CAICarMover::EndFrame(void)
{
	CAIBotMover::EndFrame();
}
