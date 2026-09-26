#include "fang.h"
#include "floop.h"
#include "AI3DBotMover.h"
#include "AIBrain.h"
#include "AINodePools.h"
#include "AIBrainMan.h"
#include "AIRooms.h"
#include "AIEdgeLock.h"
#include "AIGameUtils.h"
#include "AIMain.h"
#include "AIHazard.h"
#include "AIGraphSearcher.h"
#include "../player.h"
#include "../Entity.h"
#include "../bot.h"

extern f32 kfEssentiallyStoppedSpeed;
extern const f32 kfEssentiallyStoppedSpeed2;
extern BOOL bBotAvoidanceOn;
static const f32 kfSearchAheadDistSpeedScale = 0.5f;
const f32 kfAvoidanceConeCos = 0.2f;
const f32 kfSteeringForceMagicScale = 4.0f;
const f32 kfBotMaxSpeed = 1.0f;
const f32 kfAbsolutelyCloseEnoughDistXZ = 0.75f;
const f32 kfAbsolutelyCloseSpeedRamDistXZ = 4.0f;
const f32 kfAbsolutelyCloseEnoughDistY = 0.75f;

static CFCylnA s_testCyln;

CAI3DBotMover::CAI3DBotMover(void) 
:	CAIMover(),
	m_pBot(NULL),
	m_pBrain(NULL),
	m_nNumYawForces(0),
	m_nNumPitchForces(0),
	m_nNumDXForces(0),
	m_nNumDYForces(0),
	m_nNumDZForces(0),
	m_u3DBotMoverFlags(0)
{
	s32 i;
	for (i = 0; i < NUM_FORCETYPES;i++)
	{
		m_afYawForce[i] = 0.0f;
		m_afPitchForce[i] = 0.0f;
		m_afDXForce[i] = 0.0f;
		m_afDYForce[i] = 0.0f;
		m_afDZForce[i] = 0.0f;
	}

	m_TorsoLookAtXZ = CFVec3A::m_UnitAxisX;
	m_HeadingUnit = CFVec3A::m_UnitAxisX;
	m_uMoverFlags |= MOVERFLAG_CAN_FLY;
}


CAI3DBotMover::~CAI3DBotMover(void)
{
}


void CAI3DBotMover::AddToWorld()
{
	s32 i;

	CEntity* pEntity = GetEntity();
	FASSERT(pEntity);
	//better have given me something that derives from bot!
	FASSERT(pEntity->TypeBits() & ENTITY_BIT_BOT);
	//Base class needs this info too
	CAIMover::AddToWorld();

	m_pBot = (CBot*) GetEntity();

	FASSERT(m_pBot);
	m_pBrain = GetEntity()->AIBrain();
	m_pPath = NULL;
	m_nNumYawForces = 0;
	m_nNumPitchForces = 0;
	m_nNumDXForces = 0;
	m_nNumDYForces = 0;
	m_nNumDZForces = 0;
	m_fStuckForSecs = 0.0f;
	m_u3DBotMoverFlags = 0;
	m_u3DBotMoverFlags |= MODEL_ORIGIN_IS_MORE_NEAR_MIDDLE_OF_CYLN;

	for (i = 0; i < NUM_FORCETYPES; i++)
	{
		m_afYawForce[i] = 0.0f;
		m_afDXForce[i] = 0.0f;
		m_afDYForce[i] = 0.0f;
		m_afDZForce[i] = 0.0f;
	}

	SetForwardGoalSpeedPctCapable(m_pBrain->GetAttrib(CAIBrain::ATTRIB_SPEED)/100.0f);
	m_uEdgeType = EF_EDGE_NORMAL;
}


void CAI3DBotMover::Create(CEntity* pEntity, u16 uGUID)
{
	CAIMover::Create(pEntity, uGUID);

	if (pEntity->TypeBits() & ENTITY_BIT_BOTPROBE)
	{
		m_u3DBotMoverFlags &=!MODEL_ORIGIN_FLAGS_MASK;
		m_u3DBotMoverFlags |= MODEL_ORIGIN_IS_MORE_NEAR_BOTTOM_OF_CYLN;
	}

	m_Controls.SetFlag_Slam_AverageWhenZero();
}


void CAI3DBotMover::RemoveFromWorld(void)
{
	FASSERT(m_pPath == NULL);
	if (m_pBrain)
	{
		ResLock_FreeAllEdgeLocks(m_pBrain->GetGUID());
	}
	CAIMover::RemoveFromWorld();
}


BOOL CAI3DBotMover::MoveToward(const CFVec3A& GotoPoint_WS, f32* pfVelAlgnmnt)
{
	//
	// State Info
	//
	m_u3DBotMoverFlags |= RECEIVED_EXTERNAL_MOVE_GOAL;

	BOOL bXZCloseEnough = MoveTowardXZ(GotoPoint_WS, pfVelAlgnmnt);

	if (m_pBot->TypeBits() & ENTITY_BIT_BOTPRED && m_pBot->IsSpotLightOn())
	{
		CFVec3A LookAt_WS;
		LookAt_WS.Sub(GotoPoint_WS, GetLoc());
		LookAt_WS.y = 0.0f;
		if (LookAt_WS.SafeUnitAndMagXZ(LookAt_WS) > 0.0f)
		{
			LookAt_WS.y = -3.0f*m_pBrain->GetInfreqCycleRandom();
			LookAt_WS.Mul(GetRadiusXZ()*4.0f);
			LookAt_WS.Add(GetLoc());
			SetHeadLookGoal_WS(LookAt_WS);
		}
	}

	//
	//  Now do FlyUp/Down
	//
	f32 fBotY = GetLoc().y;
	f32 fGoalY = GotoPoint_WS.y;
	f32 fAbsDeltaY = FMATH_FABS(fGoalY-fBotY);
	if (fAbsDeltaY < kfAbsolutelyCloseEnoughDistY) //Close enough, just don't move
	{
		m_afDYForce[FORCETYPE_GOAL] = 0.0f;
		if (m_nNumDYForces <= FORCETYPE_GOAL)
		{
			m_nNumDYForces = FORCETYPE_GOAL;
		}
		if (pfVelAlgnmnt)
		{
			*pfVelAlgnmnt = 1.0f;
		}
		return bXZCloseEnough;
	}
	else
	{	//adjust height!

		//
		// Speed control
		//
		f32 fSpeedY = 1.0f;

		if (m_pBot->m_fMaxVerticalSpeed_WS > 0.0f)
		{
			FMATH_CLAMPMAX(fSpeedY, m_fForwardGoalSpeed / m_pBot->m_fMaxVerticalSpeed_WS);
		}

		if (fAbsDeltaY < GetHeight())
		{
	//		fSpeedY = fAbsDeltaY/GetHeight();
		}

		f32 fBreaksY = 0.0f;
		//when I'm going to fast to stop at my goal, start turning on the breaks
		//findfix: probably also need to slow down if turnrad is greater than (distance_to_point-close_enough_distance)
		f32 fStoppingDist = m_pBot->ComputeEstimatedControlledStopTimeY()*FMATH_FABS(m_pBot->m_Velocity_WS.y);
		if (fStoppingDist > fAbsDeltaY)
		{
			fBreaksY = 1.0f;
		}


	//  spike the breaks if bot is going faster than mover wants it to for some reason.
	//  cool idea, but caused an oscillation that was icky.  Even though I average the AI input inside of AIControlGaols, oscillation was obvious.
	//	if (m_pBot->m_fSpeedYY_WS > m_fForwardGoalSpeed)
	//	{
	//		fBreaksY = 1.0f;
	//	}

		fSpeedY -= fSpeedY*fBreaksY; 
			
		m_afDYForce[FORCETYPE_GOAL] = fSpeedY*FMATH_FSIGN(fGoalY-fBotY);
		if (m_nNumDYForces <= FORCETYPE_GOAL)
		{
			m_nNumDYForces = FORCETYPE_GOAL;
		}

		FaceTowardPitch(GotoPoint_WS, NULL);
	}

	return FALSE;
}


//
//  Move Toward the GotoPoint
//	  -but stay within the specified distance from the "entry vector"
//
BOOL CAI3DBotMover::MoveTowardUseApproachConstraint(const CFVec3A& GotoPoint_WS, const CFVec3A& EntryVecUnit, f32 fMaxEntryVecDist, f32* pfVelAlgnmnt)
{
	BOOL bIgnoreConstraint = FALSE;
	CFVec3A BotUnitFrontXZ_WS;
	CFVec3A BotUnitRightXZ_WS;
	CFVec3A GoalToBot;

	GoalToBot.Sub(GetLoc(), GotoPoint_WS);

	if (m_pBot->m_fSpeed_WS == 0.0f)
	{	//don't goto an intermediate point if not moving at all
		bIgnoreConstraint = TRUE;
	}
	else if (m_pBot->m_UnitVelocity_WS.Dot(GoalToBot) > 0.0f)
	{	//don't goto an intermediate point if heading in complete wrong direction
		bIgnoreConstraint = TRUE;
	}
	else
	{  //goto an intermediate point ONLY if bot is too far from the EntryRay
		CFVec3A NearestEdgePos;
		NearestEdgePos = EntryVecUnit;
		NearestEdgePos.Mul(EntryVecUnit.Dot(GoalToBot));
		NearestEdgePos.Add(GotoPoint_WS);
		
		f32 fDistToEntryEdge2 = NearestEdgePos.DistSq(GetLoc());
		if (fDistToEntryEdge2 > fMaxEntryVecDist*fMaxEntryVecDist)
		{  
			MoveToward(NearestEdgePos, NULL);

			//clients want to know alignemt to the goal they sent, 
			//not the intermediate goal
			if (pfVelAlgnmnt)
			{
				GoalToBot.Mul(-1.0f);
				CFVec3A BotToGoalUnit;
				f32 fDistToDest = BotToGoalUnit.SafeUnitAndMag(GoalToBot);
				if (fDistToDest > 0.8f)	  //Not close enough dist
				{
					*pfVelAlgnmnt = m_pBot->m_UnitVelocity_WS.Dot(GoalToBot);
				}
				else
				{
					*pfVelAlgnmnt = 1.0f;	//close enough is considered good alignemnt
				}
			}

			return FALSE; //as far as caller are concerned, robot is not heading in correct direction..yet
		}
	}

	return MoveToward(GotoPoint_WS, pfVelAlgnmnt);
}


BOOL CAI3DBotMover::MoveTowardXZ(const CFVec3A& GotoPoint_WS, f32* pfVelAlgnmnt)
{

	//
	//	Movement Control
	//
	CFVec3A BotPosXZ_WS = GetLoc();	BotPosXZ_WS.y = 0;
	CFVec3A GotoPointXZ_WS = GotoPoint_WS;	GotoPointXZ_WS.y = 0.0f;
	CFVec3A UnitDeltaToDestXZ;  
	CFVec3A tmp;
	tmp.Sub(GotoPointXZ_WS, BotPosXZ_WS);
	f32 fDistToDest = UnitDeltaToDestXZ.SafeUnitAndMag(tmp);

	if (fDistToDest < kfAbsolutelyCloseEnoughDistXZ) //Close enough, just don't move
	{
		m_afDXForce[FORCETYPE_GOAL] = 0.0f;
		if (m_nNumDXForces <= FORCETYPE_GOAL)
		{
			m_nNumDXForces = FORCETYPE_GOAL;
		}

		m_afDZForce[FORCETYPE_GOAL] = 0.0f;
		if (m_nNumDZForces <= FORCETYPE_GOAL)
		{
			m_nNumDZForces = FORCETYPE_GOAL;
		}
		if (pfVelAlgnmnt)
		{
			*pfVelAlgnmnt = 1.0f;
		}
		return TRUE;
	}

	
	f32 fAlignment = 1.0f;
	// first factor the percent we are moving in the direction of the goal
	if (m_pBot->m_fSpeedXZ_WS > 0.0f)
	{
		fAlignment = m_pBot->m_UnitVelocityXZ_WS.Dot( UnitDeltaToDestXZ );
	}

	//
	// Speed control
	//
	f32 fSpeedXZ = 1.0f;
	if (fAlignment > 0.0f)
	{
		fSpeedXZ = fAlignment;
	}
	else
	{
		int i =2;
	}

	if (m_pBot->m_fMaxFlatSurfaceSpeed_WS > 0.0f)
	{
		FMATH_CLAMPMAX(fSpeedXZ, m_fForwardGoalSpeed / m_pBot->m_fMaxFlatSurfaceSpeed_WS);
	}

	if (fDistToDest < kfAbsolutelyCloseSpeedRamDistXZ)
	{
		fSpeedXZ *= fDistToDest* (1.0f/kfAbsolutelyCloseSpeedRamDistXZ);  //intentionally no 0.5f
	}

	f32 fBreaksXZ = 0.0f;
	//when I'm going to fast to stop at my goal, start turning on the breaks
	//findfix: probably also need to slow down if turnrad is greater than (distance_to_point-close_enough_distance)
	f32 fStoppingDist = m_pBot->ComputeEstimatedControlledStopTimeXZ()*m_pBot->m_fSpeedXZ_WS;
	if (fStoppingDist > fDistToDest)
	{
		fBreaksXZ = 1.0f;
	}

//  spike the breaks if bot is going faster than mover wants it to for some reason.
//  cool idea, but caused an oscillation that was icky.  Even though I average the AI input inside of AIControlGaols, oscillation was obvious.
//	if (m_pBot->m_fSpeedXZXZ_WS > m_fForwardGoalSpeed)
//	{
//		fBreaksXZ = 1.0f;
//	}

	fSpeedXZ -= fSpeedXZ*fBreaksXZ; 

	//
	// Default Torso control 
	//    - Face Torso in the direction of movement
	//	  - This can be overwritten this frame by FaceToward 
	//
	f32 fTorsoAlgn = 1.0f;
	BOOL bDontClearBit = 0;
	if (m_u3DBotMoverFlags & RECEIVED_EXTERNAL_TORSO_GOAL)
	{
		bDontClearBit = TRUE;
	}
	FaceTowardXZ(GotoPoint_WS, &fTorsoAlgn);
	if (!bDontClearBit)
	{
		m_u3DBotMoverFlags &=~ RECEIVED_EXTERNAL_TORSO_GOAL;
	}
	
	if (!((m_uMoverFlags & MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE  ) && (m_u3DBotMoverFlags & HAS_RECENT_TORSO_GOAL))
		&& fSpeedXZ > fTorsoAlgn)
	{
		if (fTorsoAlgn < 0.0f)
		{
			fTorsoAlgn = 0.0f;
		}
		fSpeedXZ += (fTorsoAlgn-fSpeedXZ)*m_fTorsoAlignmentSpeedConstraint;			//some movers' speed may be constrained differently by the fact that their torso is not aligned
	}

	CFVec3A PushVec;
	f32 fVelConstraintOverride =  m_fTorsoAlignmentVelConstraint;
	if ((m_uMoverFlags & MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE ) && (m_u3DBotMoverFlags & HAS_RECENT_TORSO_GOAL))
	{
		fVelConstraintOverride = 0.0f;
	}

	PushVec.Lerp(fVelConstraintOverride, UnitDeltaToDestXZ, GetTorsoLookAtXZ());
	PushVec.Unitize();
	PushVec.Mul(fSpeedXZ);

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

BOOL CAI3DBotMover::FaceToward(const CFVec3A& Location, f32* pfTorsoAlgnmnt)
{
	//Yaw
	BOOL bAlignedYaw = FaceTowardXZ(Location, pfTorsoAlgnmnt);

	f32 fPitchAlignment = 0.0f;
	BOOL bAlignedPitch = FaceTowardPitch(Location, &fPitchAlignment);

	if (pfTorsoAlgnmnt)
	{
		if (FMATH_FABS(*pfTorsoAlgnmnt) < FMATH_FABS(fPitchAlignment))
		{
			*pfTorsoAlgnmnt = fPitchAlignment;
		}

	}
	SetHeadLookGoal_WS(Location);

	return (bAlignedPitch && bAlignedYaw);
}

f32 kfMaxPitch = FMATH_HALF_PI-0.1f;
BOOL CAI3DBotMover::FaceTowardPitch(const CFVec3A& Location, f32* pfPitchAlgnmnt)	//1.0f pfPitchAlgnmnt is perfect aligned (0.0f is not)
{
	//Pitch
	f32 fDY = Location.y - GetLoc().y;

	if (FMATH_FABS(fDY) < GetHeight())
	{
		fDY = 0.0f;
	}
	f32 fIdealThetaRads = 0.0f;
	CFVec3A DeltaToDestXZUnit;
	CFVec3A Tmp;
	Tmp.Sub(GetLoc(), Location);
	Tmp.y = 0.0f;
	f32 fDistXZ = DeltaToDestXZUnit.SafeUnitAndMag(Tmp);
	if (fDistXZ < 0.8f)
	{
		fIdealThetaRads = kfMaxPitch*FMATH_FSIGN(fDY);
	}
	else
	{
		fIdealThetaRads = fmath_Atan(FMATH_FABS(fDY), fDistXZ);
		FMATH_BIPOLAR_CLAMPMAX(fIdealThetaRads, kfMaxPitch);
		fIdealThetaRads*=-FMATH_FSIGN(fDY);
	}
	f32 fPitchDist = (fIdealThetaRads-m_pBot->m_fMountPitch_WS);
	if (FMATH_FABS(fPitchDist) > FMATH_QUARTER_PI)
	{
		fPitchDist = FMATH_FSIGN(fPitchDist)*FMATH_QUARTER_PI;
	}
	fPitchDist/=FMATH_QUARTER_PI;


	m_afPitchForce[FORCETYPE_GOAL] = fPitchDist;
	if (m_nNumPitchForces <= FORCETYPE_GOAL)
	{
		m_nNumPitchForces = FORCETYPE_GOAL;
	}

	if (pfPitchAlgnmnt)
	{
		*pfPitchAlgnmnt = 1.0f - (fPitchDist);
	}
	if (FMATH_FABS(fPitchDist) < 0.1f)
	{
		return TRUE; //Aligned
	}
	return FALSE;
}


// 2D Torso control
BOOL CAI3DBotMover::FaceTowardXZ(const CFVec3A& Location, f32* pfTorsoAlgnmnt)
{
	CFVec3A  tmp;
	CFVec3A  BotPosXZ_WS = GetLoc();
	BotPosXZ_WS.y = 0;

	CFVec3A GotoPointXZ_WS;
	GotoPointXZ_WS = Location;
	GotoPointXZ_WS.y = 0.0f;
	CFVec3A BotUnitFrontXZ_WS, BotUnitRightXZ_WS;

	BotUnitFrontXZ_WS = GetTorsoLookAtXZ();
	BotUnitRightXZ_WS.x = BotUnitFrontXZ_WS.z;
	BotUnitRightXZ_WS.y = 0.0f;
	BotUnitRightXZ_WS.z = -BotUnitFrontXZ_WS.x;

	CFVec3A UnitDeltaToDestXZ;  
	tmp.Sub(GotoPointXZ_WS, BotPosXZ_WS);
	f32 fDistToDest = UnitDeltaToDestXZ.SafeUnitAndMag(tmp);
	f32 fFrontDot;
	BOOL bCloseEnough = FALSE;

	if (fDistToDest > 0.0f)
	{
		f32 fRightDot;
		f32 fForwardControl = 0.0f;
		fFrontDot = BotUnitFrontXZ_WS.Dot( UnitDeltaToDestXZ );
		fRightDot = BotUnitRightXZ_WS.Dot( UnitDeltaToDestXZ );
		if( fFrontDot < 0.0f )
		{
			if( fRightDot >= 0.0f )
			{
				fRightDot = 1.0f;
			}
			else
			{
				fRightDot = -1.0f;
			}
		}

		f32 fAbsRight = FMATH_FABS(fRightDot);
		if (fAbsRight < 0.001f)//kfEssentiallyStoppedSpeed)
		{
			fRightDot = 0.0f;
			bCloseEnough = TRUE;
		}
		else
		{
		//	fAbsRight = fmath_Sqrt(fAbsRight);
		//	fAbsRight = fmath_Sqrt(fAbsRight);
			fRightDot = fAbsRight*FMATH_FSIGN(fRightDot);
		}
		m_afYawForce[FORCETYPE_GOAL] = fRightDot;
		if (m_nNumYawForces <= FORCETYPE_GOAL)
		{
			m_nNumYawForces = FORCETYPE_GOAL;
		}
	}
	else
	{
	   fFrontDot  = 1.0f;
	   bCloseEnough = TRUE;
	}

	if (pfTorsoAlgnmnt)
	{
		*pfTorsoAlgnmnt = fFrontDot;
	}

	//
	// State Info
	//
	m_u3DBotMoverFlags |= RECEIVED_EXTERNAL_TORSO_GOAL;
	
	return bCloseEnough;
}


void CAI3DBotMover::AssignPath(CAIPath* pPath, u32 uAssignPathFlags)
{
	ResLock_FreeAllEdgeLocks(m_pBrain->GetGUID());
	m_pPath = pPath;
	if (m_pPath)
	{
		m_PathWalker.Init(m_pPath);	 //if following a path, track progress using this class

		//state information
		m_uEdgeType	= EF_EDGE_NORMAL;
		
		//reset the path related feedback flags
		m_fLinkStartTime = aiutils_FTotalLoopSecs();
		f32 fMaxSpeed = m_pBot->GetMaxFlatSurfaceSpeed();
		if( fMaxSpeed > 0.0f )
		{
			m_fLinkEstimateTime = fmath_Div(DistToNextWay(), fMaxSpeed);	 //average feet per second of robot?
		}
		else
		{
			m_fLinkEstimateTime = fmath_Div(DistToNextWay(), 10.0f);
		}
		m_uAssignPathFlags = uAssignPathFlags;


		if (m_PathWalker.GetCurWaypoint() && m_PathWalker.GetCurWaypoint()->m_pGraphVert)
		{
			if (m_pPath->GetGraph()->IsPtIn3DVertVol(m_PathWalker.GetCurWaypoint()->m_pGraphVert, GetLoc()))
			{
				SetLastGraphVertId(m_pPath->GetGraph()->GetVertId(m_PathWalker.GetCurWaypoint()->m_pGraphVert));
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


CAIPathWaypoint* CAI3DBotMover::FollowPath_WayPtProgress(void)							   
{
	BOOL bBumpedCloseEnough = FALSE;
	CAIPathWaypoint *pWay = m_PathWalker.GetCurWaypoint();
	FASSERT(pWay);
	CAIPathWaypoint *pLastWay = NULL;

	//if I am following a path, and bump into someone who is standing on my waypoint
	//then proceeed to next waypoint, or whatever
	if (m_pAvoidThis &&
		m_PathWalker.CloseEnough(CFSphereA(aiutils_GetEntityLoc(m_pAvoidThis), aiutils_GetEntityRadXZ(m_pAvoidThis)))
		)
	{
		bBumpedCloseEnough = TRUE;
	}

	f32 fCloseEnoughDist = GetRadiusXZ();

	if (m_uAssignPathFlags & ASSIGNPATHFLAG_PRECISION_GOAL && m_PathWalker.GetCurWaypointNum() == (m_pPath->GetNumWaypoints() -1))
	{
		fCloseEnoughDist = fmath_Sqrt(kfAbsolutelyCloseEnoughDistXZ*kfAbsolutelyCloseEnoughDistXZ+kfAbsolutelyCloseEnoughDistY *kfAbsolutelyCloseEnoughDistY );
	}
	if (bBumpedCloseEnough ||
		m_PathWalker.CloseEnough(CFSphereA(GetLoc(), fCloseEnoughDist))) //findfix: 0.0f for now, but really should be heading
	{
		//base class would like to keep track of verts as they are reached.
		if (pWay && pWay->m_pGraphVert)
		{
			SetLastGraphVertId(m_pPath->GetGraph()->GetVertId(pWay->m_pGraphVert));
		}

		if (!m_PathWalker.Advance())  //when advance returns FALSE, the last waypoint inthe path has been reached
		{
			pWay = NULL;
		}
		else
		{
			pLastWay = pWay;
			pWay = m_PathWalker.GetCurWaypoint();
			FASSERT(pWay);

			if (pLastWay && pLastWay->m_pGraphVert && pWay->m_pGraphVert)
			{
				GraphEdge* pE = pLastWay->m_pGraphVert->GetEdgeTo(m_pPath->GetGraph()->GetVertId(pWay->m_pGraphVert));
				//base class would like to keep track of edges as they are reached.
				SetLastGraphEdgeId(m_pPath->GetGraph()->GetEdgeId(pE));
			}


			m_fLinkStartTime = aiutils_FTotalLoopSecs();
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
	}
	return pWay;
}


BOOL CAI3DBotMover::FollowPath_NormalEdge(void)
{
	CAIPathWaypoint* pWay = NULL;
	CAIPathWaypoint* pLastWay = NULL;
	BOOL bIsDone = TRUE;

	if (m_pPath &&
		(pWay = FollowPath_WayPtProgress())) //Get the waypoint we are approaching?  Advance it when necessary.
	{
		bIsDone = FALSE;

		BOOL bInCur = TRUE;
		BOOL bInLast = TRUE;
		if (pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH)
		{
			bInCur = (pWay->m_pGraphVert &&
							pWay->m_Location.DistSq(GetLoc()) < pWay->m_fCloseEnoughDist*pWay->m_fCloseEnoughDist);
			pLastWay = m_PathWalker.GetPreviousWaypoint();
			bInLast = (pLastWay &&
							pLastWay->m_pGraphVert &&
							pLastWay->m_Location.DistSq(GetLoc()) < pLastWay->m_fCloseEnoughDist*pLastWay->m_fCloseEnoughDist);
		}
		
		if (!bInCur &&
			!bInLast)
		{
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


BOOL CAI3DBotMover::FollowPath(void)
{
	BOOL bDone = FALSE;

	if (!m_pPath || !m_PathWalker.IsValid())
	{
		bDone = TRUE;
	}
	else
	{
		bDone = FollowPath_NormalEdge();
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


BOOL CAI3DBotMover::_ActiveBrain_CB(CAIBrain* pOtherBrain, void* pData)
{
	CAI3DBotMover* pThisMover = (CAI3DBotMover*) pData;

	if (!pOtherBrain->IsLODActive() ||
		(pOtherBrain->GetAIMover() == pThisMover) ||
		(pOtherBrain->GetAIMover()->GetCurRoomId() != pThisMover->GetCurRoomId()))
	{
		return AIBRAINMAN_CONTINUE_ITERATION;
	}

	if (s_testCyln.IntersectSphereApprox(CFSphereA(pOtherBrain->GetAIMover()->GetLoc(), pOtherBrain->GetAIMover()->GetRadiusXZ())))
	{
		FASSERT(pOtherBrain->GetAIMover());
		pThisMover->m_AvoidEntityList.PushHead(pOtherBrain->GetAIMover()->GetEntity());
	}
	return AIBRAINMAN_CONTINUE_ITERATION; //TRUE would mean stop iterating
}

BOOL CAI3DBotMover::CalcAvoidanceTestCyln(CFCylnA* pCyln)
{
	if (m_pBot->m_fSpeedXZ_WS == 0.0f || 
		(m_nNumDXForces==0 && m_nNumDZForces==0) ||
		(m_afDXForce[m_nNumDXForces]==0.0f && m_afDZForce[m_nNumDZForces]==0.0f))
	{
		//no need to avoid if not trying to go anywhere
		return FALSE;
	}

	CFVec3A BotUnitFrontXZ_WS;
	BotUnitFrontXZ_WS.Set(m_afDXForce[m_nNumDXForces], 0.0f, m_afDZForce[m_nNumDZForces]);
	f32 fForceMag = BotUnitFrontXZ_WS.SafeUnitAndMag(BotUnitFrontXZ_WS);
	if (fForceMag > 0.0f)
	{
		f32 fDXDZSpeed = BotUnitFrontXZ_WS.Dot(m_pBot->m_VelocityXZ_WS);	 //how much speed to I have in the direction that my goal wants me to go?
		f32 fSearchAheadDist = 5.0f+fDXDZSpeed*kfSearchAheadDistSpeedScale;//pgm findfix: I should factor in how hard I need to steer this bot to avoid something! (momementum maybe?)

		pCyln->m_Axis = BotUnitFrontXZ_WS;
		pCyln->m_Origin = GetLoc();
		if (m_u3DBotMoverFlags & MODEL_ORIGIN_IS_MORE_NEAR_BOTTOM_OF_CYLN)
		{   //bump the Y to half way up the height, for more accurate testcyln
			pCyln->m_Origin.y += GetHeight()*0.5f;
		}
		else if (m_u3DBotMoverFlags & MODEL_ORIGIN_IS_MORE_NEAR_MIDDLE_OF_CYLN)
		{
			pCyln->m_Origin.y += GetHeight()*0.5f;
		}
		//for testcyln radius, use larger of half collcyln height or radiusxz
		pCyln->m_fWidth = GetRadiusXZ() > GetHeight()*0.5f ? GetRadiusXZ() : GetHeight()*0.5f;
		pCyln->m_fHeight = fSearchAheadDist;
		return TRUE;
	}
	return FALSE;
}


void CAI3DBotMover::FindObstacles(void)
{
	m_pAvoidThis = NULL;
	FASSERT(m_AvoidEntityList.Size()==0);

	if (!CalcAvoidanceTestCyln(&s_testCyln))
	{
		return;
	}

	//find all active bots that intersect my test cylinder, put in a list
	aibrainman_IterateActiveBrains(CAI3DBotMover::_ActiveBrain_CB, this);

	//Add Player Bots into the AvoidList as well

	if (!(m_uMoverFlags & MOVERFLAG_DONT_AVOID_PLAYER_BOTS))
	{
		CEntity* pPlayerEntity = NULL;
		for (s32 i = 0; i < CPlayer::m_nPlayerCount; i++)
		{
			if ((pPlayerEntity = Player_aPlayer[i].m_pEntityCurrent) &&	(pPlayerEntity->TypeBits() & ENTITY_BIT_BOT))
			{
				if (s_testCyln.IntersectSphereApprox(CFSphereA(pPlayerEntity->MtxToWorld()->m_vPos, ((CBot*) pPlayerEntity)->m_fCollCylinderRadius_WS)))
				{
					m_AvoidEntityList.PushHead(pPlayerEntity);
				}
			}
		}
	}

	//find the closest one to me, that's who I'll avoid
	CNiIterator<CEntity*> itE = m_AvoidEntityList.Begin();
	while (itE.IsValid())
	{
		CEntity* pOtherEntity = itE.Get();
		if (m_pAvoidThis == NULL || (aiutils_GetEntityDist2(pOtherEntity, GetEntity()) < aiutils_GetEntityDist2(m_pAvoidThis, GetEntity()) ))
		{
			m_pAvoidThis = pOtherEntity;
		}
		itE.Next();
	}
	m_AvoidEntityList.RemoveAll();
}


void CAI3DBotMover::AvoidObstacles(void)
{
	if (!bBotAvoidanceOn)
	{
		return;
	}
	if (!m_pAvoidThis)
	{
/*		if (m_afYawForce[1] > 60.0f*FLoop_fPreviousLoopSecs)
		{
			m_nNumYawForces++;
			m_afYawForce[1] -= 60.0f*FLoop_fPreviousLoopSecs;
		}
		else
		{
			m_afYawForce[1] = 0.0f;
		}
*/
		return;
	}
	if (m_pBot->m_fSpeedXZ_WS == 0.0f ||
		(m_afDXForce[m_nNumDXForces]==0.0f && m_afDZForce[m_nNumDXForces]==0.0f))
	{
		//no need to avoid if not trying to go anywhere
		FASSERT(0);  //SHould not have obstacles to avoid!
	}

	CFVec3A BotUnitFrontXZ_WS;
	BotUnitFrontXZ_WS.x = m_afDXForce[m_nNumDXForces];
	BotUnitFrontXZ_WS.y = 0.0f;
	BotUnitFrontXZ_WS.z = m_afDZForce[m_nNumDZForces];
	BotUnitFrontXZ_WS.Unitize();

	CFVec3A BotUnitRightXZ_WS;
	BotUnitRightXZ_WS.x = BotUnitFrontXZ_WS.z;
	BotUnitRightXZ_WS.y = 0.0f;
	BotUnitRightXZ_WS.z = -BotUnitFrontXZ_WS.x;

//	CFSphereA OtherSphere(m_pAvoidThis->GetLoc(), m_pAvoidThis->GetRadiusXZ());
// findfix: switch this to above line when CEntity gets finished.
	CFSphereA OtherSphere(m_pAvoidThis->MtxToWorld()->m_vPos, ((CBot*) m_pAvoidThis)->m_fCollCylinderRadius_WS);
// test code	CFSphereA OtherSphere(GotoPoint_WS, 2.0f);  //test a specific spot
	CFSphereA MySphere(GetLoc(), GetRadiusXZ());  

	CFVec3A UnitDeltaToDestXZ;  
	CFVec3A tmp;
	tmp.Sub(OtherSphere.m_Pos, GetLoc());
	f32 fDeltaVecMag = UnitDeltaToDestXZ.SafeUnitAndMag(tmp);

	if (fDeltaVecMag > 0.0f)
	{
		fDeltaVecMag -= OtherSphere.m_fRadius;
		fDeltaVecMag -= MySphere.m_fRadius;
		if (fDeltaVecMag < 0.0f)
			fDeltaVecMag = 0.0f;

		f32 fForwardDot = UnitDeltaToDestXZ.Dot(BotUnitFrontXZ_WS);
		if (fForwardDot > kfAvoidanceConeCos)	 //field of view apporx 78 degrees
		{
			f32 fPerpDot = BotUnitRightXZ_WS.Dot(UnitDeltaToDestXZ);
			f32 fSteerFactor;

			if (m_pAvoidThis &&
				m_pAvoidThis->AIBrain() &&
				m_pAvoidThis->AIBrain()->GetAIMover() &&
				m_pAvoidThis->AIBrain()->GetAIMover()->GetAvoidThis() == GetEntity() && 
				FMATH_FSIGN(m_pAvoidThis->AIBrain()->GetAIMover()->GetAvoidSteeringInfluence()) != FMATH_FSIGN(GetAvoidSteeringInfluence()) &&
				m_pAvoidThis->AIBrain()->GetAIMover()->GetAvoidThis() > GetEntity())	   //kinda goofy random test, maybe might look better to decide which one should turn other way, but....
			{   // if we're avoiding each other and
			    // turning toward the same direction
			    // not sure why, but don't let it happen. 
				// one of us turns the other way!
//				m_pBot->m_Controls.fForward *= 0.5f;
				fPerpDot *=-1.0f;  //one of us goes the other way
			}

			if (fPerpDot > 0.0f)
			{
				//obstacle is to the right, turn left
				fSteerFactor = -1.0f+fPerpDot;
			}
			else
			{	//obstacle is to the left, turn right
				fSteerFactor = 1.0f+fPerpDot;
			}

			f32 fFactor = 1.0f;
		
			//factor in how far away this obstacle is
			f32 fSearchAheadDist = s_testCyln.m_fHeight;  //SearchAheadDist
			fFactor =((fSearchAheadDist-fDeltaVecMag)/fSearchAheadDist);
			FMATH_CLAMP(fFactor, 0.0f, 1.0f);
			fSteerFactor *= fFactor;

			//factor in how directly in front of me this obstacles center is.
			fFactor = 0.7f+(fForwardDot-kfAvoidanceConeCos)/(1.0f-kfAvoidanceConeCos);
			FMATH_CLAMP(fFactor, 0.0f, 1.0f);
			fSteerFactor *= fFactor;

			//just scale the steering force as well
//			fFactor = kfSteeringForceMagicScale;
//			FMATH_CLAMP(fFactor, 0.0f, 1.0f);
//			fSteerFactor*=fFactor;

			if (fDeltaVecMag < 0.1f && FMATH_FABS(fSteerFactor) < 0.1f)
			{
				fSteerFactor = FMATH_FSIGN(fSteerFactor)*0.1f;
			}

			f32 fGoalSpeed = m_afDXForce[m_nNumDXForces]*m_afDXForce[m_nNumDXForces] + m_afDZForce[m_nNumDZForces]*m_afDZForce[m_nNumDZForces];
			if (fGoalSpeed > 0.0f)
			{
				fGoalSpeed = fmath_Sqrt(fGoalSpeed);
			}
			BotUnitRightXZ_WS.Mul(fSteerFactor);
			CFVec3A AvoidForce;
			AvoidForce.x = m_afDXForce[m_nNumDXForces];
			AvoidForce.y = 0.0f;
			AvoidForce.z = m_afDZForce[m_nNumDZForces];
			AvoidForce.Add(BotUnitRightXZ_WS);
		//	AvoidForce.Mul(0.5f);
			AvoidForce.Unitize();
			AvoidForce.Mul(fGoalSpeed);

			m_afDXForce[FORCETYPE_AVOID] = AvoidForce.x;
			if (m_nNumDXForces <= FORCETYPE_AVOID)
			{
				m_nNumDXForces = FORCETYPE_AVOID;
			}
			m_afDZForce[FORCETYPE_AVOID] = AvoidForce.z;
			if (m_nNumDZForces <= FORCETYPE_AVOID)
			{
				m_nNumDZForces = FORCETYPE_AVOID;
			}
		}
	}
			
/*			if ((aiutils_FTotalLoopSecs() - m_fLastAvoidSteeringChange) > 4.0f ||
				(FMATH_FABS(m_afYawForce[AVOIDANCE_STEERING_INFLUENCE]) < 0.01f) ||
				(FMATH_FSIGN(m_afYawForce[AVOIDANCE_STEERING_INFLUENCE]) == FMATH_FSIGN(fSteerFactor)))
			{
				if (((FMATH_FABS(m_afYawForce[AVOIDANCE_STEERING_INFLUENCE]) < 0.01f) ||
					(FMATH_FSIGN(m_afYawForce[AVOIDANCE_STEERING_INFLUENCE]) != FMATH_FSIGN(fSteerFactor))))
				{
					m_fLastAvoidSteeringChange  = aiutils_FTotalLoopSecs();
				}
				m_afYawForce[AVOIDANCE_STEERING_INFLUENCE] = fSteerFactor;
				if (m_nNumYawForces < AVOIDANCE_STEERING_INFLUENCE+1)
				{
					m_nNumYawForces = 2;
				}
			}
		}
	}
*/
}


void CAI3DBotMover::BeginFrame(void)
{
	CAIMover::BeginFrame();

	m_nNumYawForces = 0;
	m_nNumDXForces = 0;
	m_nNumDZForces = 0;
	m_Controls.ResetButtons();	  //reset buttons at top of every frame
	m_u3DBotMoverFlags &= ~RECEIVED_EXTERNAL_TORSO_GOAL;
	m_u3DBotMoverFlags &= ~RECEIVED_EXTERNAL_MOVE_GOAL;

	
	m_TorsoLookAtXZ.Set(m_pBot->MtxToWorld()->m_vFront.x, 0.0f, m_pBot->MtxToWorld()->m_vFront.z);
	if (m_TorsoLookAtXZ.MagXZ() > 0.0f)
	{
		m_TorsoLookAtXZ.UnitizeXZ();
	}
	else
	{
		m_TorsoLookAtXZ = CFVec3A::m_UnitAxisX;
	}

	m_HeadingUnit.Set(m_pBot->MtxToWorld()->m_vFront.x, m_pBot->MtxToWorld()->m_vFront.y, m_pBot->MtxToWorld()->m_vFront.z);
	if (m_HeadingUnit.Mag() > 0.0f)
	{
		m_HeadingUnit.Unitize();
	}
	else
	{
		m_HeadingUnit = CFVec3A::m_UnitAxisX;
	}

	//Did the bot move last frame?
	if (m_pBot->m_MountPrevPos_WS.DistSq(GetLoc()) < kfEssentiallyStoppedSpeed)
	{
		m_nClientFeedback |= FEEDBACK_STOPPED;
	}
	else
	{
		m_nClientFeedback &= ~FEEDBACK_STOPPED;
	}

}


void CAI3DBotMover::DoStuckWork(void)
{
	CFVec3A Force;
	CFVec3A Tmp;
	Tmp.Set(m_afDXForce[m_nNumDXForces], m_afDYForce[m_nNumDYForces], m_afDZForce[m_nNumDZForces]);
	CFVec3A Vel;
	Vel.Sub(GetLoc(), m_pBot->m_MountPrevPos_WS);
	f32 fDist = Force.SafeUnitAndMag(Tmp);

	f32 fDot = 0.0f;
	if (fDist > 0.0f)
	{
		fDot = Vel.Dot(Force);
	}

	if (m_pBot->m_MountPrevPos_WS.DistSq(GetLoc()) < kfEssentiallyStoppedSpeed)
	{
		m_nClientFeedback |= FEEDBACK_STOPPED;
	}
	else
	{
		m_nClientFeedback &= ~FEEDBACK_STOPPED;
	}

	if ((fDist > 0.0f && (m_nClientFeedback & FEEDBACK_STOPPED)) ||
		(fDist > 0.0f && fDot < 0.05f))
	{

		m_fStuckForSecs +=FLoop_fPreviousLoopSecs;

		//
		// if normal controls indicate there is somewhere to go,
		// but we are stuck, then try to get unstuck
		//
		if (m_nNumDXForces || m_nNumDZForces)	   //got somewhere to go, but am stuck
		{

			//
			//  Which way to try to get unstuck
			//	 -- also, make sure direction doesn't change that often.
			//
			if (this->m_pAvoidThis &&	  //I was avoiding something before getting stuck
				!(m_uMoverFlags & MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME))  //first time
			{
				f32 fUnstuckSteerFactor = 0.5f*FMATH_FSIGN(m_afYawForce[FORCETYPE_PREVIOUS]);

				if ((aiutils_FTotalLoopSecs() - m_fLastOverrideSteeringChange) > 4.0f)
				{
					fUnstuckSteerFactor*=-1.0f;
					m_fLastOverrideSteeringChange = aiutils_FTotalLoopSecs();
				}

				m_nNumYawForces = FORCETYPE_UNSTUCK;
				m_afYawForce[FORCETYPE_UNSTUCK] = fUnstuckSteerFactor;
				m_uMoverFlags |= MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
				m_fLastAvoidSteeringChange = aiutils_FTotalLoopSecs();
			}
			else if (m_uMoverFlags & MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME)
			{
				f32 fUnstuckSteerFactor = FMATH_FSIGN(m_afYawForce[FORCETYPE_PREVIOUS]);
				m_nNumYawForces = FORCETYPE_UNSTUCK;
				m_afYawForce[FORCETYPE_UNSTUCK] = fUnstuckSteerFactor;
				m_uMoverFlags |= MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
				m_fLastAvoidSteeringChange = aiutils_FTotalLoopSecs();
			}
			else
			{
				int i = 2;
			}
		}
	}
	else
	{
		m_uMoverFlags &= ~MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
		m_fStuckForSecs = 0.0f;
	}
}


void CAI3DBotMover::EndFrame(void)
{
	if (m_pBot->m_Velocity_WS.MagSq() >= kfEssentiallyStoppedSpeed)
	{  //moving
		m_nRoomId = airooms_FindRoomID(GetLoc());
	}

	DoStuckWork();
	FindObstacles();
	AvoidObstacles();

	m_Controls.SetAimDownGoal(m_afPitchForce[m_nNumPitchForces]);
	m_Controls.SetRotateCWGoal(m_afYawForce[m_nNumYawForces]);
	m_Controls.SetDXGoal(m_afDXForce[m_nNumDXForces]);
	m_Controls.SetDYGoal(m_afDYForce[m_nNumDYForces]);
	m_Controls.SetDZGoal(m_afDZForce[m_nNumDZForces]);

	//keep track of the controls given to the bot last time

	//yaw
	if (m_nNumYawForces > 0)
	{
		m_afYawForce[FORCETYPE_PREVIOUS] = m_Controls.GetRotateCWGoal();
	}
	else if (FMATH_FABS(m_afYawForce[FORCETYPE_PREVIOUS]) > 0.1f)
	{
		m_afYawForce[FORCETYPE_PREVIOUS] *= 0.8f;
	}
	else
	{
		m_afYawForce[FORCETYPE_PREVIOUS] = 0.0f;
	}

	//pitch
	if (m_nNumPitchForces > 0)
	{
		m_afPitchForce[FORCETYPE_PREVIOUS] = m_Controls.GetAimDownGoal();
	}
	else if (FMATH_FABS(m_afPitchForce[FORCETYPE_PREVIOUS]) > 0.1f)
	{
		m_afPitchForce[FORCETYPE_PREVIOUS] *= 0.8f;
	}
	else
	{
		m_afPitchForce[FORCETYPE_PREVIOUS] = 0.0f;
	}

	//dx
	if (m_nNumDXForces > 0)
	{
		m_afDXForce[FORCETYPE_PREVIOUS] = m_Controls.GetDXGoal();
	}
	else if (FMATH_FABS(m_afDXForce[FORCETYPE_PREVIOUS]) > 0.1f)
	{
		m_afDXForce[FORCETYPE_PREVIOUS] *= 0.8f;
	}
	else
	{
		m_afDXForce[FORCETYPE_PREVIOUS] = 0.0f;
	}

	//DY
	if (m_nNumDYForces > 0)
	{
		m_afDYForce[FORCETYPE_PREVIOUS] = m_Controls.GetDYGoal();
	}
	else if (FMATH_FABS(m_afDYForce[FORCETYPE_PREVIOUS]) > 0.1f)
	{
		m_afDYForce[FORCETYPE_PREVIOUS] *= 0.8f;
	}
	else
	{
		m_afDYForce[FORCETYPE_PREVIOUS] = 0.0f;
	}

	//DZ
	if (m_nNumDZForces > 0)
	{
		m_afDZForce[FORCETYPE_PREVIOUS] = m_Controls.GetDZGoal();
	}
	else if (FMATH_FABS(m_afDZForce[FORCETYPE_PREVIOUS]) > 0.1f)
	{
		m_afDZForce[FORCETYPE_PREVIOUS] *= 0.8f;
	}
	else
	{
		m_afDZForce[FORCETYPE_PREVIOUS] = 0.0f;
	}

	if (m_uMoverFlags & MOVERFLAG_RECEIVED_EXTERNAL_HEADLOOK_GOAL)
	{
		m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_USE_HEADLOOK;
		if (GetEntity()->TypeBits() & ENTITY_BIT_BOT)
		{ //tell head to look there as well
			
			m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_USE_HEADLOOK;
			if (m_Controls.GetEntityControl()->m_HeadLookPt_WS.DistSq(GetHeadLookGoal_WS()) > 1.0f)
			{
				m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_NEW_HEADLOOK_LOCATION;
				m_Controls.GetEntityControl()->m_HeadLookPt_WS = GetHeadLookGoal_WS();
			}
		}
	}

	//tell entity what to do!
	m_Controls.ApplyGoals();

	if (m_u3DBotMoverFlags & RECEIVED_EXTERNAL_TORSO_GOAL)
	{
		m_u3DBotMoverFlags |= HAS_RECENT_TORSO_GOAL;
	}
	else
	{
		m_u3DBotMoverFlags &=~ HAS_RECENT_TORSO_GOAL;
	}
}


f32 CAI3DBotMover::DistToNextWay(void)
{
	CFVec3A tmp; 
	if (!m_PathWalker.GetCurLoc(&tmp))
	{
		FASSERT(0);
	}

	f32 fDistToNextWay = tmp.Dist(GetLoc());
	return fDistToNextWay;
}


BOOL CAI3DBotMover::CanTalkNow(BOOL bRequireLowerBody /*= FALSE*/)
{
	return !GetWeaponCtrlPtr(0) || !(GetWeaponCtrlPtr(0)->IsBursting());
	return TRUE;

	//BOOL bCan = 1; //IsSafeToStop();
	//// check for bot reloading weapon
	//if (!bCan)
	//{
	//	if (m_uEdgeType & EF_EDGE_DOOR)
	//	{
	//		bCan = (m_uDoorStage == DOOR_STAGE_WAITING) || (m_uDoorStage == DOOR_STAGE_FINDSWITCH);
	//	}
	//	else if (m_uEdgeType & EF_EDGE_LIFT)
	//	{
	//		bCan = (m_uLiftStage == LIFT_STAGE_WAITINGFOR) || (m_uLiftStage == LIFT_STAGE_FINDSWITCH);
	//	}

	//	if( m_pBot->IsReloading() )
	//	{
	//		bCan = FALSE;
	//	}
	//}

	//return bCan;

}
