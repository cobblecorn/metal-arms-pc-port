#include "fang.h"
#include "floop.h"
#include "AIBotMover.h"
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
#include "../Door.h"
#include "../ESwitch.h"
#include "../meshtypes.h"
#include "../Vehicle.h"
#include "../Site_BoTweapon.h"
#include "../botjumper.h"


CEntity *  CNiIterator<class CEntity *>::s_ReturnError;

f32 kfEssentiallyStoppedSpeed = 0.0001f;
const f32 kfEssentiallyStoppedSpeed2 = kfEssentiallyStoppedSpeed*kfEssentiallyStoppedSpeed;
const f32 kfAvoidanceConeCos = 0.2f;
const f32 kfSteeringForceMagicScale = 4.0f;
const f32 kfBotMaxSpeed = 1.0f;
const f32 kfSwitchStandOffDist = 2.0f;
BOOL bBotAvoidanceOn = 1;
const f32 kMinHopSpeed = 0.25f;	 //must be pretty close to stopped for the bot to jump
static CFCylnA s_testCyln;


f32 _CalcJumpVelFor45DegreeTakeoffTrajectory(f32 dy, f32 dxz, f32 a)
{
	if (dxz < dy+0.001f)
	{
		dxz = dy + 2.0f;  //somebody is trying to take off at a little more than a 45deg angle, adjust it
	}
	f32 y0 = -0.5f* FMATH_SQRT2 * fmath_Sqrt((dy - dxz)*a)*dxz/(dy-dxz);
	return  y0;
}


CAIBotMover::CAIBotMover(void) 
:	CAIMover(),
	m_pBot(NULL),
	m_pBrain(NULL),
	m_nNumRotForces(0),
	m_nNumDXForces(0),
	m_nNumDZForces(0),
	m_uBotMoverFlags(0),
	m_uJumpStage(0),
	m_fAvoidForceRotation(0.0f),
	m_uHopMin(5),	
	m_uHopMax(7),	
	m_uHopTimeOut(0),
	m_uRollMin(0),	
	m_uRollMax(0),	
	m_uRollTimeOut(0)
{
	s32 i;
	for (i = 0; i < NUM_FORCETYPES;i++)
	{
		m_afRotForce[i] = 0.0f;
		m_afDXForce[i] = 0.0f;
		m_afDZForce[i] = 0.0f;
	}

	m_TorsoLookAtXZ = CFVec3A::m_UnitAxisX;

}


CAIBotMover::~CAIBotMover(void)
{
}


void CAIBotMover::AddToWorld()
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
	FASSERT(m_pBrain);

	if (m_pBot && m_pBrain && m_pBot->IsRatGun())
	{
		m_pBrain->SetFlag_DisableEntityWork();
	}

	m_pPath = NULL;
	m_nNumRotForces = 0;
	m_nNumDXForces = 0;
	m_nNumDZForces = 0;
	m_fStuckForSecs = 0.0f;
	m_uBotMoverFlags = 0;

	for (i = 0; i < NUM_FORCETYPES; i++)
	{
		m_afRotForce[i] = 0.0f;
		m_afDXForce[i] = 0.0f;
		m_afDZForce[i] = 0.0f;
	}

	m_uJumpStage = JUMP_STAGE_NOPE;
	SetForwardGoalSpeedPctCapable(m_pBrain->GetAttrib(CAIBrain::ATTRIB_SPEED)/100.0f);
	m_uEdgeType = EF_EDGE_NORMAL;
	m_uHopTimeOut = 0;
	m_uRollTimeOut = 0;
	m_uMechEntryStage = MECH_ENTRY_STAGE_NOPE;
}


void CAIBotMover::Create(CEntity* pEntity, u16 uGUID)
{
	CAIMover::Create(pEntity, uGUID);
}


void CAIBotMover::RemoveFromWorld(void)
{
	FASSERT(m_pPath == NULL);
	if (m_pBrain)
	{
		ResLock_FreeAllEdgeLocks(m_pBrain->GetGUID());
	}
	CAIMover::RemoveFromWorld();
}

void CAIBotMover::SimpleMoveToward(const CFVec3A& GotoPoint_WS)
{
	CFVec3A  BotPosXZ_WS = GetLoc();
	BotPosXZ_WS.y = 0;

	CFVec3A GotoPointXZ_WS;
	GotoPointXZ_WS = GotoPoint_WS;
	GotoPointXZ_WS.y = 0.0f;

	CFVec3A UnitDeltaToDestXZ;  
	CFVec3A tmp;
	tmp.Sub(GotoPointXZ_WS, BotPosXZ_WS);
	f32 fDistToDest = UnitDeltaToDestXZ.SafeUnitAndMag(tmp);

	if (fDistToDest<0.1f) //error case, just don't move
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
	}
	else
	{
		//
		// Speed control
		//
		f32 fMaxSpeedXZ = 1.0f;
		f32 fMinSpeedXZ = .40f;
		f32 fSpeedXZ = fMaxSpeedXZ;
		
		f32 fStartSlowingDownAt = 5.0f;
		f32 fOOStartSlowingDownAt = 1.0f/5.0f;
		if (fDistToDest < fStartSlowingDownAt)
		{
			fSpeedXZ = FMATH_FPOT(fDistToDest*fOOStartSlowingDownAt, fMaxSpeedXZ, fMinSpeedXZ);
		}
		f32 fStoppingDist = m_pBot->ComputeEstimatedControlledStopTimeXZ()*m_pBot->m_fSpeedXZ_WS;
		if (fStoppingDist > fDistToDest)
		{
			fSpeedXZ = 0.f;
		}
		CFVec3A PushVec; 
		PushVec = UnitDeltaToDestXZ;
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
	}
	m_Controls.SetFlag_Slam_DX_Average();
	m_Controls.SetFlag_Slam_DZ_Average();
}

BOOL CAIBotMover::MoveToward(const CFVec3A& GotoPoint_WS, f32* pfVelAlgnmnt)
{
	CFVec3A MountPos;
	CFVec3A lookAtPt;

	//
	// State Info
	//
	m_uBotMoverFlags |= BOTMOVERFLAG_RECEIVED_EXTERNAL_MOVE_GOAL;

	//
	//	Movement Control
	//
	CFVec3A  BotPosXZ_WS = GetLoc();
	BotPosXZ_WS.y = 0;

	CFVec3A GotoPointXZ_WS;
	GotoPointXZ_WS = GotoPoint_WS;
	GotoPointXZ_WS.y = 0.0f;

	CFVec3A UnitDeltaToDestXZ;  
	CFVec3A tmp;
	tmp.Sub(GotoPointXZ_WS, BotPosXZ_WS);
	f32 fDistToDest = UnitDeltaToDestXZ.SafeUnitAndMag(tmp);

	if (fDistToDest <0.8f) //error case, just don't move
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
		fSpeedXZ = 0.5f;
	}

	f32 fAbsolutelyCloseEnoughSlowDownOutterRad = GetRadiusXZ()*0.5f;
	FMATH_CLAMPMAX(fAbsolutelyCloseEnoughSlowDownOutterRad, 4.0f);
	if (fDistToDest < fAbsolutelyCloseEnoughSlowDownOutterRad)
	{
		fSpeedXZ *= fmath_Div(fDistToDest, GetRadiusXZ());  //intentionally no 0.5f
	}

	if (m_pBot->m_fMaxFlatSurfaceSpeed_WS > 0.0f)
	{
		FMATH_CLAMPMAX(fSpeedXZ, m_fForwardGoalSpeed / m_pBot->m_fMaxFlatSurfaceSpeed_WS);
	}


	f32 fBrakes = 0.0f;
	//when I'm close, start turning on the brakes
	//findfix: probably also need to slow down if turnrad is greater than (distance_to_point-close_enough_distance)
	f32 fStoppingDist = m_pBot->ComputeEstimatedControlledStopTimeXZ()*m_pBot->m_fSpeedXZ_WS;
	
//	if (fStoppingDist < GetRadiusXZ())
//	{
//		fStoppingDist = GetRadiusXZ();
//	}

	if (fStoppingDist > fDistToDest)
	{
		fBrakes = 1.0f;// - fDistToDest/GetRadiusXZ();
	}

//  spike the brakes if bot is going faster than mover wants it to for some reason.
//  cool idea, but caused an oscillation that was icky.  Even though I average the AI input inside of AIControlGaols, oscillation was obvious.
//	if (m_pBot->m_fSpeedXZ_WS > m_fForwardGoalSpeed)
//	{
//		fBrakes = 1.0f;
//	}

	fSpeedXZ -= fSpeedXZ*fBrakes; 
	
	//slam controls when going for a jump. Regardless of max speed.
	if (m_uJumpStage == JUMP_STAGE_RUNNINGSTART)
	{
		fSpeedXZ = 1.0f;	 //assumption is that alignmnent is pretty good by the time mover gets to "runningstart"
	}


	//
	// Default Torso control
	//    - Face Torso in the direction of movement
	//	  - This can be overwritten this frame by FaceToward 
	//
	f32 fTorsoAlgn = 1.0f;

	if (!(m_uBotMoverFlags & BOTMOVERFLAG_RECEIVED_EXTERNAL_TORSO_GOAL) &&
		fDistToDest > GetRadiusXZ())
	{
		_FaceTowardXZ(GotoPoint_WS, &fTorsoAlgn);
	}
	else
	{
		f32 fTmp;
		CalcFrontRightAlgnDotsXZ(GotoPoint_WS, &fTorsoAlgn, &fTmp);
	}

	// Try to tell the unit to have a head pitch of 0 degrees
	//pgm: findfix this code seems extremely strange,
	//and I'm not sure it is working, expecially for 
	//grunts.  
	lookAtPt = m_pBot->MtxToWorld()->m_vFront;
	lookAtPt.y = 0.0f;
	lookAtPt.Mul(5.0f);
	m_pBot->ComputeApproxMuzzlePoint_WS(&MountPos);
	lookAtPt.Add(MountPos);
 
	_FaceTowardPitch(lookAtPt);

	if (!((m_uMoverFlags & MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE  ) && (m_uBotMoverFlags & BOTMOVERFLAG_HAS_RECENT_TORSO_GOAL))
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
	if ((m_uMoverFlags & MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE ) && (m_uBotMoverFlags & BOTMOVERFLAG_HAS_RECENT_TORSO_GOAL))
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


//
//  Move Toward the GotoPoint
//	  -but stay within the specified distance from the "entry vector"
//
BOOL CAIBotMover::MoveTowardUseApproachConstraint(const CFVec3A& GotoPoint_WS, const CFVec3A& EntryVecUnit, f32 fMaxEntryVecDist, f32* pfVelAlgnmnt)
{
	BOOL bIgnoreConstraint = FALSE;
	CFVec3A BotUnitFrontXZ_WS;
	CFVec3A BotUnitRightXZ_WS;
	CFVec3A GoalToBotXZ;

	CFVec3A  BotPosXZ_WS = GetLoc();
	BotPosXZ_WS.y = 0;

	CFVec3A GotoPointXZ_WS = GotoPoint_WS;
	GotoPointXZ_WS.y = 0.0f;
	
	GoalToBotXZ.Sub(BotPosXZ_WS, GotoPointXZ_WS);

	if (m_pBot->m_fSpeedXZ_WS == 0.0f)
	{	//don't goto an intermediate point if not moving at all
		bIgnoreConstraint = TRUE;
	}
	else if (m_pBot->m_UnitVelocityXZ_WS.Dot(GoalToBotXZ) > 0.0f)
	{	//don't goto an intermediate point if heading in complete wrong direction
		bIgnoreConstraint = TRUE;
	}
	else
	{  //goto an intermediate point ONLY if bot is too far from the EntryRay
		CFVec3A EntryVecUnitXZ;
		EntryVecUnitXZ = EntryVecUnit;
		EntryVecUnitXZ.y = 0.0f;
		EntryVecUnitXZ.UnitizeXZ();

		CFVec3A NearestEdgePosXZ;
		NearestEdgePosXZ = EntryVecUnitXZ;
		NearestEdgePosXZ.Mul(EntryVecUnitXZ.Dot(GoalToBotXZ));
		NearestEdgePosXZ.Add(GotoPointXZ_WS);
		
		f32 fDistToEntryEdge2 = NearestEdgePosXZ.DistSq(BotPosXZ_WS);
		if (fDistToEntryEdge2 > fMaxEntryVecDist*fMaxEntryVecDist)
		{  
			MoveToward(NearestEdgePosXZ, NULL);

			if (!(m_uBotMoverFlags & BOTMOVERFLAG_RECEIVED_EXTERNAL_TORSO_GOAL))
			{
				_FaceTowardXZ(GotoPoint_WS);
			}

			//clients want to know alignemt to the goal they sent, 
			//not the intermediate goal
			if (pfVelAlgnmnt)
			{
				CFVec3A BotToGoalUnitXZ;
				GoalToBotXZ.Mul(-1.0f);
				f32 fDistToDest = BotToGoalUnitXZ.SafeUnitAndMag(GoalToBotXZ);
				if (fDistToDest > 0.8f)	  //Not close enough dist
				{
					*pfVelAlgnmnt = m_pBot->m_UnitVelocityXZ_WS.Dot(BotToGoalUnitXZ);
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


// 3D Torso control
f32 _kfMaxPitch = FMATH_HALF_PI-0.1f;
BOOL CAIBotMover::FaceToward(const CFVec3A& Location, f32* pfTorsoAlgnmnt)
{
	BOOL bCloseEnough;
	bCloseEnough = _FaceTowardXZ(Location, pfTorsoAlgnmnt);

	_FaceTowardPitch(Location); 

	SetHeadLookGoal_WS(Location);

	// Remember that this function was called so that any other internal calls to _FaceTowardXZ or _FaceTowardPitch in the same frame, don't override the Location specified here.
	m_uBotMoverFlags |= BOTMOVERFLAG_RECEIVED_EXTERNAL_TORSO_GOAL;
	return bCloseEnough;
}

BOOL CAIBotMover::CalcFrontRightAlgnDotsXZ(const CFVec3A& Location, f32* pfFrontDot, f32* pfRightDot)
{
	CFVec3A  tmp;
	CFVec3A  BotPosXZ_WS;
	
	if (m_pBot->GetCurMech() && m_pBot->GetCurMech()->TypeBits() & ENTITY_BIT_BOT)
	{
		BotPosXZ_WS = m_pBot->GetCurMech()->m_pAISteerMtx->m_vPos;
	}
	else
	{
		BotPosXZ_WS = m_pBot->m_pAISteerMtx->m_vPos;
	}
	BotPosXZ_WS.y = 0;

	CFVec3A GotoPointXZ_WS;
	GotoPointXZ_WS = Location;
	GotoPointXZ_WS.y = 0.0f;
	CFVec3A BotUnitFrontXZ_WS, BotUnitRightXZ_WS;

	BotUnitFrontXZ_WS = GetTorsoLookAtXZ();		//returns unitfacingxz of mech if this bot is operating one
	BotUnitRightXZ_WS.x = BotUnitFrontXZ_WS.z;
	BotUnitRightXZ_WS.y = 0.0f;
	BotUnitRightXZ_WS.z = -BotUnitFrontXZ_WS.x;

	CFVec3A UnitDeltaToDestXZ;  
	tmp.Sub(GotoPointXZ_WS, BotPosXZ_WS);
	f32 fDistToDest = UnitDeltaToDestXZ.SafeUnitAndMag(tmp);

	FASSERT(pfFrontDot && pfRightDot);
	if (fDistToDest < 0.0f)
	{
		*pfFrontDot = 1.0f;
		*pfRightDot = 0.0f;
		return FALSE;
	}
	else
	{
		*pfFrontDot = BotUnitFrontXZ_WS.Dot( UnitDeltaToDestXZ );
		*pfRightDot = BotUnitRightXZ_WS.Dot( UnitDeltaToDestXZ );
		return TRUE;
	}
}


// returns TRUE when Mover's Torso is facing Loc
BOOL CAIBotMover::_FaceTowardXZ(	const CFVec3A& Location, f32* pfTorsoAlgnmnt)
{
	BOOL bCloseEnough = FALSE;
	f32 fFrontDot;
	f32 fRightDot;
	f32 fForwardDotAimPrecision = 0.98f;
	CalcFrontRightAlgnDotsXZ(Location, &fFrontDot, &fRightDot);

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
	if (fFrontDot > fForwardDotAimPrecision)
	{
		fRightDot = 0.0f;
		bCloseEnough = TRUE;
	}
	else
	{
		FASSERT(fRightDot != 0.0f);
		//sqrt makes it so that the controller force approaches zero a little less softly
		fRightDot = FMATH_FSIGN(fRightDot)* fmath_Sqrt(FMATH_FABS(fRightDot));
		FMATH_CLAMP(fRightDot, -m_fMaxXZTurnSpeedPct, m_fMaxXZTurnSpeedPct);
	}

	m_afRotForce[FORCETYPE_GOAL] = fRightDot;

	if (m_nNumRotForces <= FORCETYPE_GOAL)
	{
		m_nNumRotForces = FORCETYPE_GOAL;
	}

	if (pfTorsoAlgnmnt)
	{
		*pfTorsoAlgnmnt = fFrontDot;
	}
	return bCloseEnough;
}


BOOL CAIBotMover::_FaceTowardPitch(	const CFVec3A& Location)
{
	BOOL bCloseEnough = FALSE;
	//Pitch
	CFVec3A MountPos;
	f32 fCurPitch = 0.0f;
	f32 fGoalPitch = 0.0f;
//	if (m_pBot->GetCurMech())
//	{
//		m_pBot->GetCurMech()->ComputeApproxMuzzlePoint_WS(&MountPos);
//		fCurPitch = aiutils_GetBotAimPitch(m_pBot->GetCurMech());
//	}
//	else
	{
		m_pBot->ComputeApproxMuzzlePoint_WS(&MountPos);
		fCurPitch = aiutils_GetBotAimPitch(m_pBot);
	}

	//way of calculating the Goal Pitch
	CFVec3A DeltaToDestUnit;
	DeltaToDestUnit.Sub(Location, MountPos);
	f32 fDist = DeltaToDestUnit.SafeUnitAndMag(DeltaToDestUnit);
	if (fDist > FMATH_POS_EPSILON)
	{
		FMATH_CLAMP(DeltaToDestUnit.y, -1.0f, 1.0f);  //because safeunitandmag
		fGoalPitch = fmath_Atan( fmath_Sqrt( 1.0f - DeltaToDestUnit.y*DeltaToDestUnit.y ), DeltaToDestUnit.y ) - FMATH_HALF_PI;
	}
	else
	{
		fGoalPitch = 0.0f;
	}

	f32 fPitchDelta = fGoalPitch - fCurPitch;
	FMATH_CLAMP(fPitchDelta, -FMATH_QUARTER_PI, FMATH_QUARTER_PI);
	fPitchDelta*=1.0f/FMATH_QUARTER_PI;		//Normalize in the range
	if (fPitchDelta > 0.0f)
	{
		fPitchDelta = fmath_Sqrt(fPitchDelta);
	}
	else
	{
		fPitchDelta = -fmath_Sqrt(-fPitchDelta);
	}

	m_Controls.SetAimDownGoal(fPitchDelta);
	if (FMATH_FABS(fPitchDelta) < 0.001f)
	{
		bCloseEnough = TRUE;
	}

	return bCloseEnough;
}

f32 _kfLateralDodgeCosAngMin = 0.0f;  //0.9f;

BOOL CAIBotMover::_StartHopPath(void)
{
	BOOL bHopStarted = FALSE;
	if (HasCanHopPathFlags())	//whoever is assigning us to this path says we may hop it if possible
	{
		if (m_uHopTimeOut < aiutils_GetCurTimeSecsU16())
		{
			m_uHopTimeOut = aiutils_GetCurTimeSecsU16() + m_uHopMin;
			if (m_uHopMax > m_uHopMin)
			{
				m_uHopTimeOut += fmath_RandomChoice(m_uHopMax - m_uHopMin);
			}

			
			CFVec3A PathEnd;
			if (m_pBot->IsCapableOfHopping() &&
				m_pPath->GetEndOfPath(&PathEnd) &&
			//	m_pBot->m_fSpeed_WS < kMinHopSpeed &&
				m_pPath->GetNumWaypoints() <=4 )
			{
				f32 fHopDistSq = m_pBot->GetApproxHopDist();
				fHopDistSq*=fHopDistSq;
				if (PathEnd.DistSqXZ(GetLoc()) >= fHopDistSq)
				{
					f32 fRightHopDot;
					f32 fFrontHopDot;
					if (CalcFrontRightAlgnDotsXZ(PathEnd, &fFrontHopDot, &fRightHopDot))
					{
						if (fRightHopDot > _kfLateralDodgeCosAngMin)
						{
							m_Controls.HopRight();
							bHopStarted = TRUE;
						}
						else if (fRightHopDot < -_kfLateralDodgeCosAngMin)
						{
							m_Controls.HopLeft();
							bHopStarted = TRUE;
						}

						if (bHopStarted)
						{
							//if no hop count was specified, then assume they meant one
							if ((m_uAssignPathFlags >> ASSIGNPATHFLAG_HOPCOUNTSHIFT)==0)
							{
								m_uAssignPathFlags |= (1<<ASSIGNPATHFLAG_HOPCOUNTSHIFT);
							}
							m_uAssignPathFlags |= ASSIGNPATHFLAG_ISHOPPING;
						}
					}
				}
			}
		}
	}

	return bHopStarted;
}


BOOL CAIBotMover::_HandleHoppingPath(void)
{
	BOOL bDonePath = FALSE;
	if (HasHoppingPathFlags())
	{
		m_Controls.Alert(); //always alert while hopping.	   //findfix: should this only be for grunts?
		if (!m_Controls.IsHopFlagSet() && !m_pBot->IsHopping())
		{
			//just finished a hop, decrement the hop counter
			u8 uHopCount = (u8) ( m_uAssignPathFlags>>ASSIGNPATHFLAG_HOPCOUNTSHIFT);
			if (uHopCount > 0)
			{
				uHopCount--;
			}
			m_uAssignPathFlags &=~ASSIGNPATHFLAG_HOPCOUNTMASK;
			m_uAssignPathFlags |= uHopCount<<ASSIGNPATHFLAG_HOPCOUNTSHIFT;

			if (uHopCount > 0 ||
				m_uAssignPathFlags & ASSIGNPATHFLAG_CANHOPALONG)
			{
				CFVec3A PathEnd;
				if (m_pPath->GetEndOfPath(&PathEnd))
				{
					f32 fHopDistSq = m_pBot->GetApproxHopDist();
					fHopDistSq*=fHopDistSq;
					if (PathEnd.DistSqXZ(GetLoc()) >= fHopDistSq)
					{
						f32 fRightHopDot;
						f32 fFrontHopDot;
						if (CalcFrontRightAlgnDotsXZ(PathEnd, &fFrontHopDot, &fRightHopDot))
						{
							if (fRightHopDot > _kfLateralDodgeCosAngMin)
							{
								m_Controls.HopRight();
							}
							else if (fRightHopDot < -_kfLateralDodgeCosAngMin)
							{
								m_Controls.HopLeft();
							}
							else
							{
								m_uAssignPathFlags &= ~ASSIGNPATHFLAG_ISHOPPING;
							}
						}
						else
						{
							m_uAssignPathFlags &= ~ASSIGNPATHFLAG_ISHOPPING;
						}
					}
				}
			}
		}
		else if (m_pBot->IsHopping())
		{
			CFVec3A PathEnd;
			if (m_pPath->GetEndOfPath(&PathEnd))
			{
				PathEnd.Sub(GetLoc());
				f32 tmp;
				tmp = PathEnd.x;
				PathEnd.x = PathEnd.z;
				PathEnd.z = -tmp;
				PathEnd.Mul(10.0f);
				_FaceTowardXZ(PathEnd);
			}
		}

		if (!(m_uAssignPathFlags & ASSIGNPATHFLAG_CANHOPALONG) &&
			(m_uAssignPathFlags>>ASSIGNPATHFLAG_HOPCOUNTSHIFT) == 0)
		{
			bDonePath = TRUE;  //just finished a hop
		}
	}
	return bDonePath;
}


BOOL CAIBotMover::_StartRollPath(void)
{
	BOOL bRollStarted = FALSE;
	if (HasCanRollPathFlags())	//whoever is assigning us to this path says we may hop it if possible
	{
		if (m_uRollTimeOut < aiutils_GetCurTimeSecsU16())
		{
			m_uRollTimeOut = aiutils_GetCurTimeSecsU16() + m_uRollMin;
			if (m_uRollMax > m_uRollMin)
			{
				m_uRollTimeOut += fmath_RandomChoice(m_uRollMax - m_uRollMin);
			}
			
			CFVec3A PathEnd;
			if (m_pBot->IsCapableOfRolling() &&
				m_pPath->GetEndOfPath(&PathEnd) &&
			//	m_pBot->m_fSpeed_WS < kMinRollSpeed &&
				m_pPath->GetNumWaypoints() <=4 )
			{
				f32 fRollDistSq = m_pBot->GetApproxRollDist();
				fRollDistSq*=fRollDistSq;
				if (PathEnd.DistSqXZ(GetLoc()) >= fRollDistSq)
				{
					f32 fRightRollDot;
					f32 fFrontRollDot;
					if (CalcFrontRightAlgnDotsXZ(PathEnd, &fFrontRollDot, &fRightRollDot))
					{
						if (fRightRollDot > _kfLateralDodgeCosAngMin)
						{
							m_Controls.RollRight();
							bRollStarted = TRUE;
						}
						else if (fRightRollDot < -_kfLateralDodgeCosAngMin)
						{
							m_Controls.RollLeft();
							bRollStarted = TRUE;
						}

						if (bRollStarted)
						{
							//if no hop count was specified, then assume they meant one
							if ((m_uAssignPathFlags >> ASSIGNPATHFLAG_ROLLCOUNTSHIFT)==0)
							{
								m_uAssignPathFlags |= (1<<ASSIGNPATHFLAG_ROLLCOUNTSHIFT);
							}
							m_uAssignPathFlags |= ASSIGNPATHFLAG_ISROLLING;
						}
					}
				}
			}
		}

	}
	return bRollStarted;
}


BOOL CAIBotMover::_HandleRollingPath(void)
{
	BOOL bDonePath = FALSE;
	if (HasRollingPathFlags())
	{
		m_Controls.Alert(); //always alert while hopping.	 //should this only be for grunts?
		if (!m_Controls.IsRollFlagSet() && !m_pBot->IsRolling())
		{
			//just finished a hop, decrement the hop counter
			u8 uRollCount = (u8) ( m_uAssignPathFlags>>ASSIGNPATHFLAG_ROLLCOUNTSHIFT);
			if (uRollCount > 0)
			{
				uRollCount--;
			}
			m_uAssignPathFlags &=~ASSIGNPATHFLAG_ROLLCOUNTMASK;
			m_uAssignPathFlags |= uRollCount<<ASSIGNPATHFLAG_ROLLCOUNTSHIFT;

			if (uRollCount > 0 ||
				m_uAssignPathFlags & ASSIGNPATHFLAG_CANROLLALONG)
			{
				CFVec3A PathEnd;
				if (m_pPath->GetEndOfPath(&PathEnd))
				{
					f32 fRollDistSq = m_pBot->GetApproxRollDist();
					fRollDistSq*=fRollDistSq;
					if (PathEnd.DistSqXZ(GetLoc()) >= fRollDistSq)
					{
						f32 fRightRollDot;
						f32 fFrontRollDot;
						if (CalcFrontRightAlgnDotsXZ(PathEnd, &fFrontRollDot, &fRightRollDot))
						{
							if (fRightRollDot > _kfLateralDodgeCosAngMin)
							{
								m_Controls.RollRight();
							}
							else if (fRightRollDot < -_kfLateralDodgeCosAngMin)
							{
								m_Controls.RollLeft();
							}
							else
							{
								m_uAssignPathFlags &= ~ASSIGNPATHFLAG_ISROLLING;
							}
						}
						else
						{
							m_uAssignPathFlags &= ~ASSIGNPATHFLAG_ISROLLING;
						}
					}
				}
			}

		}
		else if (m_pBot->IsRolling())
		{
			CFVec3A PathEnd;
			if (m_pPath->GetEndOfPath(&PathEnd))
			{
				PathEnd.Sub(GetLoc());
				f32 tmp;
				tmp = PathEnd.x;
				PathEnd.x = PathEnd.z;
				PathEnd.z = -tmp;
				PathEnd.Mul(10.0f);
				_FaceTowardXZ(PathEnd);
			}
		}

		if (!(m_uAssignPathFlags & ASSIGNPATHFLAG_CANROLLALONG) &&
			(m_uAssignPathFlags>>ASSIGNPATHFLAG_ROLLCOUNTSHIFT) == 0)
		{
			bDonePath = TRUE;  //just finished a hop
		}
	}
	return bDonePath;
}


void CAIBotMover::AssignPath(CAIPath* pPath, u32 uAssignPathFlags /* =0 */)
{
	ResLock_FreeAllEdgeLocks(m_pBrain->GetGUID());
	m_pPath = pPath;
	if (m_pPath)
	{
		m_PathWalker.Init(m_pPath);	 //if following a path, track progress using this class

		//state information
		m_uJumpStage = JUMP_STAGE_NOPE;
		m_uEdgeType	= EF_EDGE_NORMAL;
		m_uMechEntryStage = MECH_ENTRY_STAGE_NOPE;
		
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

		if (!(HasCanHopPathFlags() && _StartHopPath()))	//whoever is assigning us to this path says we may hop it if possible
		{
			if (HasCanRollPathFlags())
			{
				_StartRollPath();
			}
		}

		if (m_PathWalker.GetCurWaypoint() && m_PathWalker.GetCurWaypoint()->m_pGraphVert)
		{
			if (m_pPath->GetGraph()->IsPtIn2DVertVol(m_PathWalker.GetCurWaypoint()->m_pGraphVert, GetLoc(), TRUE))
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


void CAIBotMover::_PrepareRunningJump(void)
{
	CAIPathWaypoint *pLastWay = m_PathWalker.GetPreviousWaypoint();
	FASSERT(pLastWay->m_pGraphVert);

	//skip the jump-stage where we are waiting for edge lock if we are granted the lock right now.
	//by checking for edge lock right now
	if (ResLock_RequestEdgeLock(RESLOCK_LOCK_OR_WAIT, m_PathWalker.GetCurVertId(), m_PathWalker.GetLastVertId(), m_pBrain->GetGUID()) == RESLOCKRETVAL_LOCK_ACQUIRED)   
	{ //I've got the lock! proceed to ALIGN
		m_uJumpStage = JUMP_STAGE_ALIGN;
	}
	else
	{	//Rejected. Keep trying
		m_uJumpStage = JUMP_STAGE_LOCKEDGE;
	}
}



CAIPathWaypoint* CAIBotMover::FollowPath_WayPtProgress(void)							   
{
	BOOL bBumpedCloseEnough = FALSE;
	CAIPathWaypoint *pWay = m_PathWalker.GetCurWaypoint();
	FASSERT(pWay);

	//if I am following a path, and bump into someone who is standing on my next waypoint
	//then consider it to have been tagged
	if (!(m_uEdgeType & EF_EDGE_JUMP) &&
		m_pAvoidThis &&
		m_PathWalker.CloseEnoughXZ(CFSphereA(aiutils_GetEntityLoc(m_pAvoidThis), aiutils_GetEntityRadXZ(m_pAvoidThis)))
		)
	{
		bBumpedCloseEnough = TRUE;
	}


	if (bBumpedCloseEnough ||
		m_PathWalker.CloseEnoughXZ(CFSphereA(GetLoc(), GetRadiusXZ()))) //findfix: 0.0f for now, but really should be heading
	{
		//base class would like to keep track of verts as they are reached.
		if (pWay->m_pGraphVert)
		{
			SetLastGraphVertId(m_pPath->GetGraph()->GetVertId(pWay->m_pGraphVert));
		}

		//
		//
		//	We're close enough to the waypoint, that normally, we would advance to the next one, 
        //	but first, let's check some special conditions that might superceed
		//	being close to the waypoint.  
		//
		//
		
		if (m_uEdgeType & EF_EDGE_JUMP &&
			pWay->m_pGraphVert	&& //this graphvert assurance in case there are chump waypts in the middle of a real jump link
			(m_pBot->IsJumping() && !(m_uEdgeType & EF_EDGE_HOVER)))		//ignore the fact that we are close enough if bot is still in the air from a previous jump or something
		{	//kill the air speed!
			CFVec3A Vec2Vert;
			Vec2Vert.Sub(pWay->m_pGraphVert->GetLocation(), m_pBot->MtxToWorld()->m_vPos);
			Vec2Vert.y = 0.0f;
			if (m_pBot->m_VelocityXZ_WS.Dot(Vec2Vert) < 0.0f)
			{
				f32 fYVel = m_pBot->m_Velocity_WS.y;
				m_pBot->ZeroVelocity();
				CFVec3A VertVel;
				VertVel.Zero();
				VertVel.y = fYVel;
				m_pBot->ApplyVelocityImpulse_WS(VertVel);
			}
			return pWay;	  //No, not finished up a link

		}

		if (m_uAssignPathFlags & ASSIGNPATHFLAG_ENTERMECH_PATH &&
			m_pBot->AIBrain()->GetMechLock())
		{
			if (!m_PathWalker.HasNext())
			{
				m_uMechEntryStage = MECH_ENTRY_STAGE_ATBASE;
				return pWay;   //don't advance the way pt if it is the last one and we're on a mech entry path
			}
		}


		//
		//	Made it this far eh, then I guess we really are going to
		//  start heading toward the next waypoint.
		//
		//	Now is the chance to cleanup any state type vbls related to the edge
		//  we were just following

		//finished a jump link!
		if (m_uEdgeType & EF_EDGE_JUMP
			&& pWay->m_pGraphVert)	//this graphvert assurance in case there are chump waypts in the middle of a real jump link
		{
			FASSERT(m_uJumpStage != JUMP_STAGE_NOPE); //why wouldn't we have a jump stage if our edge type is jump?
			BOOL bDidIt = ResLock_FreeEdgeLock(m_PathWalker.GetCurVertId(), m_PathWalker.GetLastVertId(), m_pBrain->GetGUID());
			FASSERT(bDidIt && !ResLock_HasEdgeLocked(m_pBrain->GetGUID())); //it definetely isn't locked by me
			
			//if (m_uJumpStage == JUMP_STAGE_ARRIVALPENDING)		//old if that required bot to be in this jump stage before finishing
			m_uJumpStage = JUMP_STAGE_NOPE;  
			m_uEdgeType &= ~EF_EDGE_JUMP;
			m_uEdgeType &= ~EF_EDGE_JUMPCREVACE;
			m_uEdgeType &= ~EF_EDGE_HOVER;
		}

		// finished going through a door
		if (m_uEdgeType & EF_EDGE_DOOR
			&& pWay->m_pGraphVert)	//this graphvert assurance in case there are chump waypts in the middle of a real door
		{
			m_uEdgeType &= ~EF_EDGE_DOOR;
			m_uDoorStage = DOOR_STAGE_NOPE;
		}

		// finished riding a lift
		if (m_uEdgeType & EF_EDGE_LIFT
			&& pWay->m_pGraphVert)	//this graphvert assurance in case there are chump waypts in the middle of a real door
		{
			m_uEdgeType &= ~EF_EDGE_LIFT;
			m_uLiftStage = LIFT_STAGE_NOPE;

			BOOL bDidIt = ResLock_FreeEdgeLock(m_PathWalker.GetCurVertId(), m_PathWalker.GetLastVertId(), m_pBrain->GetGUID());
			FASSERT(bDidIt && !ResLock_HasEdgeLocked(m_pBrain->GetGUID())); //it definetely isn't locked by me
		}

		//finished an edge with a switch
		if (m_uEdgeType & EF_EDGE_HAS_SWITCH)
		{
			m_uEdgeType &= ~EF_EDGE_HAS_SWITCH;
			m_uEdgeType &= ~EF_EDGE_USE_SWITCH1;
		}


		if (!m_PathWalker.Advance())  //m_PathWalker.Advance when advance returns FALSE, the last waypoint inthe path has been reached
		{
			pWay = NULL;

			if (m_pBot->IsCapableOfStooping())
			{
				CAIPathWaypoint *pLastWay = m_PathWalker.GetPreviousWaypoint();

				if (pLastWay && pLastWay->m_pGraphVert)
				{	
					f32 fTallHeight = GetTallHeight();
					if (pLastWay->m_pGraphVert->GetHeight() < fTallHeight)
					{
						m_pBot->Stoop(TRUE);
					}
					else
					{
						m_pBot->Stoop(FALSE);
					}
				}
			}


		}
		else
		{
			CAIPathWaypoint *pLastWay = pWay;

			pWay = m_PathWalker.GetCurWaypoint();
			FASSERT(pWay);

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

			//
			//  We're about to start following a new edge
			//  maybe there are some special states that mover needs to have
			//  set based on the type of edge.
			//

			if (pLastWay)
			{
				GraphEdge* pE = NULL;


				if (m_pBot->IsCapableOfStooping())
				{
					f32 fTallHeight = GetTallHeight();
					if (pLastWay->m_pGraphVert && pWay->m_pGraphVert)
					{
						pE = pLastWay->m_pGraphVert->GetEdgeTo(aimain_pAIGraph->GetVertId(pWay->m_pGraphVert));
						if (pE)
						{
							if (pE->GetHeight() < fTallHeight)
							{
								m_pBot->Stoop(TRUE);
							}
							else
							{
								m_pBot->Stoop(FALSE);
							}
						}
					}
				}

				if (pE)
				{
					//base class would like to keep track of verts as they are reached.
					SetLastGraphEdgeId(m_pPath->GetGraph()->GetEdgeId(pE));
				}

				if (pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)
				{
					m_uEdgeType |= EF_EDGE_JUMP;

					if ((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_LEDGE)
					{
						m_uEdgeType |= EF_EDGE_JUMPCREVACE;
					}
					else if ((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_HOVER)
					{
						m_uEdgeType |= EF_EDGE_HOVER;
					}

					_PrepareRunningJump();
				}
				else if (pLastWay->m_uExitEdgeProps & EDGEPROP_HAZARD_DOOR)
				{
					GraphEdge* pE = pLastWay->m_pGraphVert->GetEdgeTo(aimain_pAIGraph->GetVertId(pWay->m_pGraphVert));
					if (pE)
					{
						m_uHazardId = pE->m_nHazardId;
						m_uEdgeType |= EF_EDGE_DOOR;
						m_uDoorStage = DOOR_STAGE_ENROUTE;
					}
				}
				else if (pLastWay->m_uExitEdgeProps & EDGEPROP_HAZARD_LIFT)
				{
					if (!pE)
					{
						pE = pLastWay->m_pGraphVert->GetEdgeTo(aimain_pAIGraph->GetVertId(pWay->m_pGraphVert));
					}
					if (pE)
					{
						m_uHazardId = pE->m_nHazardId;
						m_uEdgeType |= EF_EDGE_LIFT;
						m_uLiftStage = LIFT_STAGE_WAITINGFOR;

						ResLock_RequestEdgeLock(RESLOCK_LOCK_OR_WAIT, m_PathWalker.GetCurVertId(), m_PathWalker.GetLastVertId(), m_pBrain->GetGUID());
					}
				}
				//intentionally not an else (doors or lifts can have switches (p
				if (pLastWay->m_uWayPointFlags & EDGEPROP_HAZARD_HAS_SWITCH)
				{
					m_uEdgeType |= EF_EDGE_HAS_SWITCH;
					if (pLastWay->m_uWayPointFlags & EDGEPROP_HAZARD_USE_SWITCH1)
					{
						m_uEdgeType |= EF_EDGE_USE_SWITCH1;
					}
					else
					{
						m_uEdgeType &= ~EF_EDGE_USE_SWITCH1;
					}
				}
			}
			
		}
	}
	else if (m_uAssignPathFlags & ASSIGNPATHFLAG_ENTERMECH_PATH &&
			m_uMechEntryStage == MECH_ENTRY_STAGE_NOPE &&
			m_pBot->AIBrain()->GetMechLock())
	{
		f32 fDistSq = GetRadiusXZ();
		if (m_pBot->AIBrain()->GetMechLock()->AIBrain())
		{
			fDistSq += 5.0f + m_pBot->AIBrain()->GetMechLock()->AIBrain()->GetAIMover()->GetRadiusXZ();
		}
		fDistSq*=fDistSq;

		// bot is on path to enter a mech and is within range.
		if (m_pBot->AIBrain()->GetMechLock()->MtxToWorld()->m_vPos.DistSqXZ(GetLoc()) < fDistSq)
		{
			m_uMechEntryStage = MECH_ENTRY_STAGE_ATBASE;
		}
	}

	return pWay;
}


BOOL CAIBotMover::FollowPath_NormalEdge(void)
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
			bInCur =	(pWay->m_pGraphVert &&
						pWay->m_Location.DistSqXZ(GetLoc()) < pWay->m_fCloseEnoughDist* pWay->m_fCloseEnoughDist);
			pLastWay = m_PathWalker.GetPreviousWaypoint();
			bInLast =	(pLastWay &&
						pLastWay->m_pGraphVert &&
						pLastWay->m_Location.DistSqXZ(GetLoc()) < pLastWay->m_fCloseEnoughDist*pLastWay->m_fCloseEnoughDist);
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


BOOL CAIBotMover::FollowPath_LiftEdge(void)
{
	CAIPathWaypoint* pWay = NULL;
	CAIPathWaypoint* pLastWay = NULL;
	BOOL bIsDone = TRUE;
	CFVec3A KneeLoc;
	CFVec3A KneeLocAtGoal;
	u32 nSwitchId = 0;
	CESwitch* pSwitch = NULL;
	CFVec3A LiftPos;

	CDoorEntity* pLift = (CDoorEntity*) aimain_pGraphSearcher->m_DoorHazardReg.GetRegistered(m_uHazardId);
	if (pLift &&
		m_pPath &&
		(pWay = FollowPath_WayPtProgress()) && //Get the waypoint we are approaching?  Advance it when necessary.
		(pLastWay = m_PathWalker.GetPreviousWaypoint())	)

	{
		bIsDone = FALSE;
		CFVec3A GotoLoc;
		GotoLoc = GetLoc();
		f32 fDistSq;
		
		LiftPos.Set(pLift->GetBoundingSphere_WS().m_Pos);

		
		if (pLastWay->m_uExitEdgeProps & EF_EDGE_HAS_SWITCH)
		{
			if (m_uEdgeType & EF_EDGE_USE_SWITCH1)
			{
				nSwitchId = 1;
			}
			pSwitch = (CESwitch*) pLift->AI_GetSwitch(nSwitchId);
		}

		switch (m_uLiftStage)
		{
			case LIFT_STAGE_WAITINGFOR:
				{

					if (ResLock_RequestEdgeLock(RESLOCK_LOCK_OR_WAIT, m_PathWalker.GetCurVertId(), m_PathWalker.GetLastVertId(), m_pBrain->GetGUID()) == RESLOCKRETVAL_LOCK_ACQUIRED)   
					{

						if (pLift->AI_IsOpen(pLastWay->m_Location))
						{
							m_uLiftStage = LIFT_STAGE_ENTERING;
							pLift->AI_HoldAtEndLiftFor(2.0f);

							GotoLoc = LiftPos;
						}
						else
						{
							if (pSwitch)
							{
								m_uLiftStage = LIFT_STAGE_FINDSWITCH;
							}
							else
							{
								//nudge to the front of the take off cylinder volume while continuing to wait
								GotoLoc.Sub(pWay->m_Location, pLastWay->m_Location);
								GotoLoc.Unitize();
								GotoLoc.Mul(pLastWay->m_fCloseEnoughDist*0.8f);
								GotoLoc.Add(pLastWay->m_Location);
							}
						}
					}
				}
				break;
			case LIFT_STAGE_ENTERING:
				GotoLoc = LiftPos;
				if (GotoLoc.DistXZ(GetLoc()) < pLift->AI_GetOnRadiusXZ())//GetRadiusXZ())
				{
					m_uLiftStage = LIFT_STAGE_ON;
				}
				//pgm. Hope that the elevator doesn't pull away before I get a chance to get on it
				break;
			case LIFT_STAGE_ON:
				GotoLoc = LiftPos;

				fDistSq = (2.0f + pWay->m_fCloseEnoughDist+pLift->GetBoundingSphere_WS().m_fRadius);
				fDistSq*=fDistSq;
				if (LiftPos.DistSq(pWay->m_Location) < fDistSq)
				{	//Pretty close to the goal waypoint now, do a ray test to find out if I can see the top of it

					KneeLoc = GetLoc();
					KneeLoc.y+=2.0f;

					KneeLocAtGoal = pWay->m_Location;
	
					if (!fworld_IsLineOfSightObstructed( &KneeLoc, &KneeLocAtGoal, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList))
					{
						m_uLiftStage = LIFT_STAGE_EXITING;
						GotoLoc = KneeLocAtGoal; 
					}
					aiutils_DebugTrackRay(KneeLoc, KneeLocAtGoal, m_uLiftStage == LIFT_STAGE_EXITING);
				}
				break;
			case LIFT_STAGE_EXITING:
				GotoLoc = pWay->m_Location;
				break;
			case LIFT_STAGE_FINDSWITCH:
				{
					if (pLift && pLift->AI_IsOpen(pLastWay->m_Location))
					{
						m_uLiftStage = LIFT_STAGE_ENTERING;
						pLift->AI_HoldAtEndLiftFor(3.0f);
					}
					else
					{
						FASSERT(pSwitch);
						if (pSwitch)
						{
							CFVec3A SwitchLoc;
							SwitchLoc.v3 = pSwitch->GetBoundingSphere_WS().m_Pos;
							f32 fDist = GetRadiusXZ()+kfSwitchStandOffDist;
							if (SwitchLoc.DistSqXZ(GetLoc()) < fDist*fDist)
							{
								pSwitch->ActionNearby(GetEntity());	  //Just says that someday, he might thow on a talk order to the bot mover as a result of this actionnearby
							}
							else
							{
								//goto the spot right in front of the switch?
								GotoLoc = pSwitch->MtxToWorld()->m_vZ;
								GotoLoc.Mul(1.8f); // findfix: this is a good distance for blink only
								GotoLoc.Add(pSwitch->MtxToWorld()->m_vPos);
							}
						}
					}
				}
				break;

		}
		MoveToward(GotoLoc);
	}
	else
	{
		MoveToward(GetLoc());
	}

	return bIsDone;
}


BOOL CAIBotMover::FollowPath_MechEntry(void)
{
	BOOL bDone = FALSE;
	BOOL bFoundStation = FALSE;
	CFVec3A StationLoc;
	CFVec3A Vec2Station;

	if (m_pBot->AIBrain()->GetMechLock())
	{
		if (m_pBot->AIBrain()->GetMechLock()->TypeBits() & ENTITY_BIT_VEHICLE)
		{
			if (((CVehicle*) m_pBot->AIBrain()->GetMechLock())->GetStationEntryPoint(CVehicle::STATION_DRIVER, &StationLoc))
			{
 
				bFoundStation = TRUE;
			}
		}
		else if (m_pBot->AIBrain()->GetMechLock()->TypeBits() & ENTITY_BIT_SITEWEAPON)
		{
			if (((CBotSiteWeapon*) m_pBot->AIBrain()->GetMechLock())->GetStationEntryPoint(&StationLoc))
			{
				bFoundStation = TRUE;
			}
		}
	}

	if (!bFoundStation)
	{
		m_uMechEntryStage = MECH_ENTRY_STAGE_FAILED;
	}

	switch (m_uMechEntryStage)
	{
		case MECH_ENTRY_STAGE_NOPE:
			break;
		case MECH_ENTRY_STAGE_FAILED:
			bDone = TRUE;
			break;
		case MECH_ENTRY_STAGE_ATBASE:
			{
				if (1)//(FVid_nFrameCounter & 1))
				{
    				RequestMechEntry(m_pBot->AIBrain()->GetMechLock());
				}

				if (m_pBot->GetCurMech())
				{
					bDone = TRUE;
				}
				else
				{
					//Jump toward the station.
					//while entering the mech, disable bot-v-bot collision
					//so that we don't get hung-up on details of the mech geometry.  
					m_pBot->IgnoreBotVBotCollision();

					f32 fIdealJumpDist = GetLoc().DistXZ(StationLoc);
					f32 fIdealJumpSpeed = _CalcJumpVelFor45DegreeTakeoffTrajectory(StationLoc.y - GetLoc().y, fIdealJumpDist, m_pBot->m_fGravity);
					CFVec3A IdealVec;
					IdealVec.Sub(StationLoc, GetLoc());
					IdealVec.y = 0.0f;
					if (IdealVec.SafeUnitAndMag(IdealVec) > 0.0f)
					{
						IdealVec.y = 1.0f;
						IdealVec.Mul(fIdealJumpSpeed);  
						m_Controls.JumpVec(IdealVec);
					}
					else
					{
						IdealVec = CFVec3A::m_UnitAxisY;
						IdealVec.Mul(10.0f);
						m_Controls.JumpVec(IdealVec);
					}

					m_uMechEntryStage = MECH_ENTRY_STAGE_INAIR;
				}
			}
			break;
		case MECH_ENTRY_STAGE_INAIR:
			{
				if (1)//(FVid_nFrameCounter & 1))
				{
    				RequestMechEntry(m_pBot->AIBrain()->GetMechLock());
				}

				if (m_pBot->GetCurMech())
				{
					bDone = TRUE;
				}
				else
				{	//haven't gained access yet.
					Vec2Station.Sub(StationLoc, m_pBot->MtxToWorld()->m_vPos);
					if (m_pBot->m_fSpeed_WS < 0.0001f || m_pBot->m_UnitVelocity_WS.Dot(Vec2Station) < 0.0f)
					{  //begin fudging it baby
						m_uMechEntryStage = MECH_ENTRY_STAGE_SUCKTOWARDSEAT;
					}
				}
			}
			break;
		case MECH_ENTRY_STAGE_SUCKTOWARDSEAT:
			{
				BOOL bRequestEntry = TRUE;
				if (m_pBot->AIBrain()->GetMechLock() &&
					m_pBot->AIBrain()->GetMechLock()->TypeBits() & ENTITY_BIT_VEHICLE)
				{
					CVehicle *pVehicle;
					pVehicle = (CVehicle*) m_pBot->AIBrain()->GetMechLock();
					if( pVehicle )
					{
						if( pVehicle->IsUpsideDown() )
						{
							pVehicle->FlipRightsideUp();
							bRequestEntry = FALSE;
						}
					}
				}
				if (bRequestEntry && !m_pBot->GetCurMech())
				{
					if (1)//(FVid_nFrameCounter & 1))
					{
    					RequestMechEntry(m_pBot->AIBrain()->GetMechLock());
					}
				}


				if (m_pBot->GetCurMech())
				{
					bDone = TRUE;
				}
				else
				{
					Vec2Station.Sub(StationLoc, m_pBot->MtxToWorld()->m_vPos);

					//fudge it baby
					m_uMechEntryStage = MECH_ENTRY_STAGE_SUCKTOWARDSEAT;
					CFVec3A NewVel;
					
					if (NewVel.SafeUnitAndMag(Vec2Station) > -1.0f)
					{
						m_pBot->ZeroVelocity();
						f32 fSpeed = 10.0f*StationLoc.Dist(m_pBot->MtxToWorld()->m_vPos);
						FMATH_CLAMP(fSpeed, 1.0f, 20.0f);
						NewVel.Mul(fSpeed);
						m_pBot->ApplyVelocityImpulse_WS(NewVel);
					}
				}
			}
			break;
	}

	if (!CanDriveMech())
	{
		bDone = TRUE;
	}

	if (bDone)
	{
		m_pBot->DontIgnoreBotVBotCollision();
	}
	return bDone;
}


BOOL CAIBotMover::FollowPath_JumpEdge(void)
{
	CFVec3A TempLoc;
	CFVec3A TempOffset;
	CFVec3A loc;
	CFVec3A Center;
	CFVec3A KneeLoc;
	CFVec3A KneeLocAtGoal;
	s32 isDone = 0;
	f32 fVelAlgnmnt;
	u16 nPlaceInLine;
	CAIPathWaypoint *pWay;
	CAIPathWaypoint *pLastWay;
	GraphEdge* pE = NULL;


	if (m_pPath &&
		(pWay = FollowPath_WayPtProgress()) &&
		(pLastWay = m_PathWalker.GetPreviousWaypoint())	)
	{

		if (pLastWay->m_pGraphVert && pWay->m_pGraphVert)
		{
			pE = pLastWay->m_pGraphVert->GetEdgeTo(aimain_pAIGraph->GetVertId(pWay->m_pGraphVert));
		}

		fVelAlgnmnt = 1.0f;
		if (m_uJumpStage != JUMP_STAGE_LOCKEDGE && m_uJumpStage != JUMP_STAGE_INAIR)
		{  //bots waiting for edge lock don't move along the path anymore!

			BOOL bInCur = (pWay->m_pGraphVert &&
							pWay->m_Location.DistSqXZ(GetLoc()) < pWay->m_fCloseEnoughDist* pWay->m_fCloseEnoughDist);
			CAIPathWaypoint* pLastWay = m_PathWalker.GetPreviousWaypoint();
			BOOL bInLast = (pLastWay &&
							pLastWay->m_pGraphVert &&
							pLastWay->m_Location.DistSqXZ(GetLoc()) < pLastWay->m_fCloseEnoughDist*pLastWay->m_fCloseEnoughDist);
			if (m_PathWalker.GetCurWaypointNum() > 0 &&
				pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH &&
				!bInCur &&
				!bInLast)
			{
				f32 fUltraConstrain = pWay->m_fApproachWidth;
				if (m_uJumpStage==JUMP_STAGE_ALIGN || m_uJumpStage==JUMP_STAGE_RUNNINGSTART)
				{
					fUltraConstrain = 1.0f;
				}

				MoveTowardUseApproachConstraint(pWay->m_Location, pWay->m_ApproachDirUnit, fUltraConstrain, &fVelAlgnmnt);
			}
			else
			{
				MoveToward(pWay->m_Location, &fVelAlgnmnt);
			}
		}

		switch (m_uJumpStage)
		{
		case JUMP_STAGE_LOCKEDGE: 
			{
				if (ResLock_RequestEdgeLock(RESLOCK_LOCK_OR_WAIT, m_PathWalker.GetCurVertId(), m_PathWalker.GetLastVertId(), m_pBrain->GetGUID()) == RESLOCKRETVAL_LOCK_ACQUIRED)  //it is locked and by me
				{
					m_uJumpStage = JUMP_STAGE_ALIGN;
				}
				else if (ResLock_HasEdgeLocked(m_pBrain->GetGUID(), &nPlaceInLine))
				{	//edge is locked and otherGUID tells by who
					TempLoc.Sub(pLastWay->m_Location, pWay->m_Location);
					TempLoc.Unitize();
					
					//rotate ray so that it is perp to jump edge
					TempOffset.x = TempLoc.z;
					TempOffset.y = TempLoc.y;
					TempOffset.z = -TempLoc.x;
					TempOffset.Mul(4.0f* (f32)nPlaceInLine);

					TempLoc.Mul(pLastWay->m_fCloseEnoughDist);
					TempLoc.Add(pLastWay->m_Location);
					TempLoc.Add(TempOffset);
					MoveToward(TempLoc, &fVelAlgnmnt);
				}
				else
				{
					FASSERT(0);
				}
			}
			break;
		case JUMP_STAGE_ALIGN:  //alignment
			if (fVelAlgnmnt < 0.94f)  // strict requirement for alignment
			{
				if (fVelAlgnmnt > 0.0f)
				{
					m_afDXForce[FORCETYPE_GOAL] = 0.0f; //stop moving until alignment is good
					m_afDZForce[FORCETYPE_GOAL] = 0.0f; //stop moving until alignment is good
				}
				break;	//break intentionally moved here!
			}
			else
			{
				m_uJumpStage = JUMP_STAGE_RUNNINGSTART;
			}
			//intentionally no break here!
		case JUMP_STAGE_RUNNINGSTART:  
			{
				if (!m_pBot->IsInAir() || CanHover() )
				{
					f32 fBotGravity = m_pBot->m_fGravity;
					if (fBotGravity == 0.0f)
					{
						fBotGravity = -64.0f;   //why would this happen?
					}

					if ((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_LEDGE)
					{
						KneeLoc = m_pBot->m_UnitVelocityXZ_WS; //using2D here, because sometimes the bot is coming down from a jump. Maybe this should be aligned with ground under the bot?
						KneeLoc.Mul(1.0f+GetRadiusXZ()+m_pBot->m_fSpeed_WS*FLoop_fPreviousLoopSecs);	  //estimate how much I will move next frame in secs
						KneeLoc.Add(GetLoc());
						KneeLoc.y+=1.0f;

						KneeLocAtGoal = KneeLoc;
						KneeLocAtGoal.y -= 3.0f;

						if (!aiutils_IsLOSObstructed_IgnoreBots(KneeLoc, KneeLocAtGoal, m_pBot))		  //checking to see when my "sensor" detects no floor.
						{
							f32 fMinusDist = pWay->m_fCloseEnoughDist - GetRadiusXZ();
							if (fMinusDist < 0.0f)
							{  //shouldn't ever happen, since path finder wouldn't have given us this path ef we don't fit on it.
								fMinusDist = 0.0f;
							}

							f32 fIdealJumpDist = GetLoc().DistXZ(pWay->m_Location) - fMinusDist;
							f32 fIdealJumpSpeed = _CalcJumpVelFor45DegreeTakeoffTrajectory(pWay->m_Location.y - GetLoc().y, fIdealJumpDist, fBotGravity);
							CFVec3A IdealVec;
							IdealVec.Sub(pWay->m_Location, GetLoc());
							IdealVec.y = 0.0f;
							IdealVec.Unitize();
							IdealVec.y = 1.0f;
							IdealVec.Mul(fIdealJumpSpeed);  

							m_Controls.JumpVec(IdealVec);
							m_uJumpStage = JUMP_STAGE_INAIR;

							//pgm: add some boost in the direction of the landing pad.
							//how much requires math
						}
						aiutils_DebugTrackRay(KneeLoc, KneeLocAtGoal, m_uJumpStage==JUMP_STAGE_INAIR);
					}
					else if ((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_OBSTACLE)//(!(m_pBot->TypeBits() & ENTITY_BIT_BOTJUMPER))
					{	//jumping an obstacle
						f32 fRayXZLength;

						for (s32 i = 0; i < 2; i++)
						{
							switch (i)
							{
								case 0:
									KneeLoc = m_pBot->m_UnitVelocityXZ_WS; //using 2D here, because sometimes the bot is coming down from a jump. Maybe this should be aligned with ground under the bot?
									KneeLoc.Mul(2.0f+GetRadiusXZ()+m_pBot->m_fSpeed_WS*FLoop_fPreviousLoopSecs);
									KneeLoc.Add(GetLoc());
									KneeLoc.y += 1.4f;

									fRayXZLength = (m_pBot->m_pApproxEyePoint_WS->y-KneeLoc.y)*1.7f;//
									KneeLocAtGoal = m_pBot->m_UnitVelocityXZ_WS; //trying 2D here, because sometimes the bot is coming down from a jump. Maybe this should be aligned with ground under the bot?
									KneeLocAtGoal.Mul(2.0f+GetRadiusXZ()+m_pBot->m_fSpeed_WS*FLoop_fPreviousLoopSecs+fRayXZLength);
									KneeLocAtGoal.Add(*(m_pBot->m_pApproxEyePoint_WS));
								break;
								case 1:
									KneeLocAtGoal = KneeLoc;
									KneeLoc = *(m_pBot->m_pApproxEyePoint_WS);
									break;

							}

							if (aiutils_IsLOSObstructed_IgnoreBots(KneeLoc, KneeLocAtGoal, m_pBot))
							{
								f32 fMinusDist = pWay->m_fCloseEnoughDist - GetRadiusXZ();
								if (fMinusDist < 0.0f)
								{  //shouldn't ever happen, since path finder wouldn't have given us this path ef we don't fit on it.
									fMinusDist = 0.0f;
								}

								f32 fIdealJumpDist = GetLoc().DistXZ(pWay->m_Location) - fMinusDist;
								f32 fIdealJumpSpeed = _CalcJumpVelFor45DegreeTakeoffTrajectory(pWay->m_Location.y - GetLoc().y, fIdealJumpDist, fBotGravity);

								CFVec3A IdealVec;
								IdealVec.Sub(pWay->m_Location, GetLoc());
								IdealVec.y = 0.0f;
								IdealVec.Unitize();
								IdealVec.y = 1.0f;
								IdealVec.Mul(fIdealJumpSpeed);  //cos 45degrees

								m_Controls.JumpVec(IdealVec);

								m_uJumpStage = JUMP_STAGE_INAIR;
								break;
							}
							aiutils_DebugTrackRay(KneeLoc, KneeLocAtGoal, m_uJumpStage!=JUMP_STAGE_INAIR);
						}
					}
					else if (((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_VERT_2_VERT))
					{	//jump right away
						CFVec3A IdealVec;

						f32 fXZVel = m_pBot->m_fMaxFlatSurfaceSpeed_WS;
						if (fXZVel <=5.0f)
						{
							fXZVel = 25.0f;
						}
						aiutils_ComputeTrajectoryFromXZVel(IdealVec, GetLoc(), pWay->m_Location, fXZVel, FMATH_FABS(fBotGravity));
						m_Controls.JumpVec(IdealVec);
						m_uJumpStage = JUMP_STAGE_INAIR;
					}
					else if ((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_HOVER)
					{
						if (pE)
						{
							f32 fY;
							if (!m_pPath->GetGraph()->CalcCeilingAlongEdge(pLastWay->m_pGraphVert, pWay->m_pGraphVert, GetLoc(), &fY))
							{
								fY = GetLoc().y;
							}
							fY-=GetHeight();

							HoverTowardY(fY);
							m_uJumpStage = JUMP_STAGE_ARRIVALPENDING;
						}
					}
				}


				if ((IsStuck(2.0f) /*|| (m_nClientFeedback & FEEDBACK_PATH_SLOWPROGRESS)*/)	&&
					!m_pAvoidThis)	//findfix: maybe there should be a better test here?  Need some better way to determine if The jump was a failure.
				{
					m_uJumpStage = JUMP_STAGE_RETRY_1;
				}


			}
			break;
		case JUMP_STAGE_INAIR:
			{
				if (m_pBot->IsJumping())
				{
					//don't screw with the precalculated jump velocity by working
					//the controller
					m_afDXForce[FORCETYPE_GOAL] = 0.0f; //stop moving while in air since initial trajectory didn't account for air-control
					m_afDZForce[FORCETYPE_GOAL] = 0.0f; 
				}
				else
				{
					m_uJumpStage = JUMP_STAGE_ARRIVALPENDING;
				}
			}
			break;
		case JUMP_STAGE_ARRIVALPENDING:
			{
				//must have landed?

				if ((IsStuck(2.0f) /*|| (m_nClientFeedback & FEEDBACK_PATH_SLOWPROGRESS)*/)	&&
					!m_pAvoidThis)	//findfix: maybe there should be a better test here?  Need some better way to determine if The jump was a failure.
				{
					if (m_uEdgeType & EF_EDGE_JUMPCREVACE)
					{
						m_uJumpStage = JUMP_STAGE_RETRY_1;
					}
					else if (((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_VERT_2_VERT) || ((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_HOVER))
					{
						m_uJumpStage = JUMP_STAGE_RETRY_3;
					}
					else
					{
						KneeLoc = GetLoc();
						KneeLoc.y += 1.0f;
						KneeLocAtGoal = pWay->m_Location;
						KneeLocAtGoal.y += 1.0f;
						if (aiutils_IsLOSObstructed_IgnoreBots(KneeLoc, KneeLocAtGoal, m_pBot))
						{
							m_uJumpStage = JUMP_STAGE_RETRY_1;
						}
						else
						{
							int i = 4;
						}
						aiutils_DebugTrackRay(KneeLoc, KneeLocAtGoal, m_uJumpStage ==JUMP_STAGE_RETRY_1);

					}
				}
				else
				{
					if ((pLastWay->m_uExitEdgeProps & EDGEPROP_JUMP)==JUMPTYPE_HOVER || (m_uEdgeType & EF_EDGE_HOVER))
					{
						if (pE)
						{
							f32 fY;
							if (!m_pPath->GetGraph()->CalcCeilingAlongEdge(pLastWay->m_pGraphVert, pWay->m_pGraphVert, GetLoc(), &fY))
							{
								fY = GetLoc().y;
							}
							fY-=GetHeight();

							HoverTowardY(fY);
						}
					}

				}
			}
			break;
		case JUMP_STAGE_RETRY_1:
			{
				m_uJumpStage = JUMP_STAGE_RETRY_2;
			}
			break;
		case JUMP_STAGE_RETRY_2:
			{
				TempLoc.Sub(pLastWay->m_Location, pWay->m_Location);
				TempLoc.Unitize();
				TempLoc.Mul(pLastWay->m_fCloseEnoughDist);
				TempLoc.Add(pLastWay->m_Location);
				MoveToward(TempLoc);
				if (TempLoc.DistSqXZ(GetLoc()) <= (GetRadiusXZ()*0.5f)*(GetRadiusXZ()*0.5f))
				{
					m_uJumpStage = JUMP_STAGE_RETRY_3;
				}
			}
			break;
		case JUMP_STAGE_RETRY_3:
			{
				m_uJumpStage = JUMP_STAGE_ALIGN;
			}
			break;
		}
	}

	return isDone!=0;
}



BOOL CAIBotMover::FollowPath_DoorEdge(void)
{
	CAIPathWaypoint* pWay = m_PathWalker.GetCurWaypoint();
	CAIPathWaypoint* pLastWay = NULL;
	BOOL bIsDone = FALSE;
	CFVec3A GotoLoc;
	CFVec3A TempLoc;
	CDoorEntity* pDoor = (CDoorEntity*) aimain_pGraphSearcher->m_DoorHazardReg.GetRegistered(m_uHazardId);
	FASSERT(pDoor);
	u32 nSwitchId = 0;
	CESwitch* pSwitch = NULL;
	
	if (m_uEdgeType & EF_EDGE_HAS_SWITCH)
	{
		if (m_uEdgeType & EF_EDGE_USE_SWITCH1)
		{
			nSwitchId = 1;
		}
		pSwitch = (CESwitch*) pDoor->AI_GetSwitch(nSwitchId);
	}
	
	GotoLoc = GetLoc();
								 
	FASSERT(pDoor);
	switch (m_uDoorStage)
	{
		case DOOR_STAGE_ENROUTE:
			{
				if (m_pPath &&
					(pWay = FollowPath_WayPtProgress())) //Get the waypoint we are approaching?  Advance it when necessary.
				{
					bIsDone = FALSE;

					if (!pDoor->AI_IsOpen(GetLoc()))
					{  
						if (pSwitch)
						{
							m_uDoorStage = DOOR_STAGE_FINDSWITCH;
						}
						else
						{
							CFVec3A DeltaToDoor;
							DeltaToDoor.Sub(pDoor->MtxToWorld()->m_vPos, GetLoc());
							CFVec3A DeltaToGoal;
							DeltaToDoor.Sub(GotoLoc, GetLoc());
							if (DeltaToDoor.Dot(GotoLoc) > 0.0f)
							{
								//stand in place, waiting for door to open or something
								m_uDoorStage = DOOR_STAGE_WAITING;
							}
						}
					}
					else
					{
						//door is open, go on through!
						GotoLoc = pWay->m_Location;
					}
				}
				else
				{
					bIsDone = TRUE;
				}
			}
			break;
		case DOOR_STAGE_WAITING:
			{
				if (pDoor->AI_IsOpen(GetLoc()))
				{
					m_uDoorStage = DOOR_STAGE_ENROUTE;
				}
				else if (!pDoor->AI_IsOpening(GetLoc()) && pSwitch)
				{
					m_uDoorStage = DOOR_STAGE_FINDSWITCH;
				}
				else
				{
					pLastWay = m_PathWalker.GetPreviousWaypoint();

					if (pWay && pLastWay)
					{
						GotoLoc.Sub(pWay->m_Location, pLastWay->m_Location);
						GotoLoc.Unitize();
						GotoLoc.Mul(pLastWay->m_fCloseEnoughDist*0.65f);
						GotoLoc.Add(pLastWay->m_Location);
					}

					//do nothing, maybe face away from the door or look pissed or something?
					//stand aside or something.

				}

			}
			break;
		case DOOR_STAGE_FINDSWITCH:
			{
				if (pDoor && pDoor->AI_IsOpen(GetLoc()))
				{
					m_uDoorStage = DOOR_STAGE_ENROUTE;
				}
				else if (!pDoor || (pDoor && !pDoor->AI_IsOpening(GetLoc())))
				{
					FASSERT(pSwitch);
					if (pSwitch)
					{
						CFVec3A SwitchLoc;
						SwitchLoc.v3 = pSwitch->GetBoundingSphere_WS().m_Pos;
						f32 fDist = GetRadiusXZ()+kfSwitchStandOffDist;
						if (SwitchLoc.DistSqXZ(GetLoc()) < fDist*fDist)
						{
							pSwitch->ActionNearby(GetEntity());	  //As a result of this call to ActionNearby(), a talk order might be pushed by the switch
							m_uDoorStage = DOOR_STAGE_WAITING;
						}
						else
						{
							//where is the spot right in front of the switch?
							
							GotoLoc = pSwitch->MtxToWorld()->m_vZ;
							GotoLoc.Mul(1.8f); // findfix: this is a good distance for blink only
							GotoLoc.Add(pSwitch->MtxToWorld()->m_vPos);
						}
					}
				}
			}
			break;
	}

	MoveToward(GotoLoc);
	return bIsDone;
}


BOOL CAIBotMover::FollowPath(void)
{
	BOOL bDone = FALSE;
	if (!m_pPath || !m_PathWalker.IsValid())
	{
		bDone = TRUE;
	}
	else if (m_uMechEntryStage != MECH_ENTRY_STAGE_NOPE)
	{
		bDone = FollowPath_MechEntry();
	}
	else if (m_uEdgeType & EF_EDGE_DOOR)
	{
		bDone = FollowPath_DoorEdge();
	}
	else if (m_uEdgeType & EF_EDGE_JUMP)
	{
		bDone = FollowPath_JumpEdge();
	}
	else if (m_uEdgeType & EF_EDGE_LIFT)
	{
		bDone = FollowPath_LiftEdge();
	}
	else
	{
		bDone = FollowPath_NormalEdge();
	}
	if (!bDone)
	{
		if (HasHoppingPathFlags())
		{
			bDone =	_HandleHoppingPath();
		}
		else if (HasRollingPathFlags())
		{
			bDone =	_HandleRollingPath();
		}
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


static const f32 kfSearchAheadDistSpeedScale = 0.5f;
BOOL CAIBotMover::CalcAvoidanceTestCyln(CFCylnA* pCyln)
{

	if (m_pBot->m_fSpeedXZ_WS == 0.0f ||
		(m_nNumDXForces==0 && m_nNumDZForces==0) ||
		(m_afDXForce[m_nNumDXForces]==0.0f && m_afDZForce[m_nNumDZForces]==0.0f))
	{
		//no need to avoid if not trying to go anywhere
		return FALSE;
	}


	CFVec3A IdealPushUnitXZ;
	IdealPushUnitXZ.x = m_afDXForce[m_nNumDXForces];
	IdealPushUnitXZ.y = 0.0f;
	IdealPushUnitXZ.z = m_afDZForce[m_nNumDZForces];
	IdealPushUnitXZ.UnitizeXZ();

	f32 fDXDZSpeed = IdealPushUnitXZ.Dot(m_pBot->m_VelocityXZ_WS);		//how much speed to I have in the direction that my goal wants me to go?
	f32 fSearchAheadDist = 5.0f+fDXDZSpeed*kfSearchAheadDistSpeedScale;	//pgm findfix: I should factor in how hard I need to steer this bot to avoid something! (momementum maybe?)

	//setup the m_testCyln once before starting to loop over all collision candidates
	pCyln->m_Origin = GetLoc();
	pCyln->m_Axis = IdealPushUnitXZ;
	pCyln->m_fWidth = GetRadiusXZ();
	pCyln->m_fHeight = fSearchAheadDist;

	return TRUE;
}

BOOL CAIBotMover::_ActiveBrain_CB(CAIBrain* pOtherBrain, void* pData)
{
	CAIBotMover* pThisMover = (CAIBotMover*) pData;

	if ((pOtherBrain->GetAIMover() == pThisMover) ||		//don't avoid self
		!pOtherBrain->IsLODActive() ||						//don't avoid brains that are inactive
		(pOtherBrain->GetAIMover()->GetEntity() == pThisMover->GetEntity()->AIBrain()->GetMechLock()) ||	   //don't avoid your own mech
		(pOtherBrain->GetMechLock() == pThisMover->GetEntity()) ||						// don't avoid mech's driver
		(pOtherBrain->GetAIMover()->GetCurRoomId() != pThisMover->GetCurRoomId()))		  //don't avoid things in other rooms.
	{
		return AIBRAINMAN_CONTINUE_ITERATION;
	}
	CFSphereA tmp;
	tmp.m_Pos = pOtherBrain->GetAIMover()->GetLoc();
	tmp.m_fRadius = pOtherBrain->GetAIMover()->GetRadiusXZ();

	if (s_testCyln.IntersectSphereApprox(tmp))
	{
		FASSERT(pOtherBrain->GetAIMover());
		pThisMover->m_AvoidEntityList.PushHead(pOtherBrain->GetAIMover()->GetEntity());
	}
	return AIBRAINMAN_CONTINUE_ITERATION; //TRUE would mean stop iterating
}


BOOL CAIBotMover::IterateEntityObstacleCBFunc(CEntity* pEntity, void* pData)
{
	CAIBotMover* pThisMover = (CAIBotMover*) pData;
	FASSERT(pEntity);

	CFSphereA tmp;
	tmp.m_Pos = pEntity->MtxToWorld()->m_vPos;
	tmp.m_fRadius = aiutils_GetEntityRadXZ(pEntity);
	if (s_testCyln.IntersectSphereApprox(tmp))
	{
		pThisMover->m_AvoidEntityList.PushHead(pEntity);
	}

	return AIGROUP_CONTINUE_ENTITY_ITERATION;
}

void CAIBotMover::_FindObstacles(void)
{
	m_pAvoidThis = NULL;

	FASSERT(m_AvoidEntityList.Size()==0);

	if (m_uMoverFlags & MOVERFLAG_DISABLE_OBJECT_AVOIDANCE)
	{
		return;
	}
	if (!((m_uJumpStage == JUMP_STAGE_NOPE) || (m_uJumpStage == JUMP_STAGE_LOCKEDGE)))
	{
		return;	  //pgm:  I think this condition was a late night hack!
	}

	if (!CalcAvoidanceTestCyln(&s_testCyln))
	{
		return;
	}

	//find all active bots that intersect my test cylinder, put in a list
	aibrainman_IterateActiveBrains(CAIBotMover::_ActiveBrain_CB, this);

	//Add Player Bots into the AvoidList as well
	for (s32 i = 0; i < CPlayer::m_nPlayerCount; i++)
	{
		CEntity* pPlayerEntity = NULL;
		if ((pPlayerEntity = Player_aPlayer[i].m_pEntityCurrent) &&	(pPlayerEntity->TypeBits() & ENTITY_BIT_BOT))
		{
			if (s_testCyln.IntersectSphereApprox(CFSphereA(pPlayerEntity->MtxToWorld()->m_vPos, ((CBot*) pPlayerEntity)->m_fCollCylinderRadius_WS)))
			{
				m_AvoidEntityList.PushHead(pPlayerEntity);
			}
		}
	}

	//add any of these darn special case site weapons that are usually in the middle of the path-finding graph, but must
	//be avoided by anyone not planning on operating them
	CBot* pSite = aigroup_FindNearestSite(s_testCyln.m_Origin, s_testCyln.m_fHeight + 10.0f);
	if (pSite && (m_pBrain->GetMechLock() != pSite))
	{
		m_AvoidEntityList.PushHead(pSite);
	}

	//
	//avoid any special entities that the AI system has been
	//notified of by the scripts
	//
	aigroup_IterateAvoidanceEntityList(IterateEntityObstacleCBFunc, this);

	//
	// Of all, find the closest obstacle to me, that's who I'll avoid
	//
	f32 fClosestSq = FMATH_MAX_FLOAT;
	CNiIterator<CEntity*> itE = m_AvoidEntityList.Begin();
	while (itE.IsValid())
	{
		CEntity* pOtherEntity = itE.Get();

		f32 fDistSq = aiutils_GetEntityDist2(pOtherEntity, GetEntity());
		if (fDistSq < fClosestSq)
		{
			fClosestSq = fDistSq;
			m_pAvoidThis = pOtherEntity;
		}
		itE.Next();
	}
	m_AvoidEntityList.RemoveAll();
}


void CAIBotMover::_AvoidObstacles(void)
{
	if (!bBotAvoidanceOn)
	{
		return;
	}
	if (!m_pAvoidThis)
	{
		return;
	}
	if (m_pBot->m_fSpeedXZ_WS == 0.0f ||
		(m_nNumDXForces==0 && m_nNumDZForces==0) ||
		(m_afDXForce[m_nNumDXForces]==0.0f && m_afDZForce[m_nNumDZForces]==0.0f))
	{
		//no need to avoid if not trying to go anywhere
		FASSERT(0);  //Should not have obstacles to avoid!
	}

	FASSERT(m_nNumDXForces == FORCETYPE_GOAL && m_nNumDZForces==FORCETYPE_GOAL);
	f32 fNowTimeSecs = aiutils_FTotalLoopSecs();
	
	CFVec3A IdealPushUnitXZ;
	IdealPushUnitXZ.x = m_afDXForce[m_nNumDXForces];
	IdealPushUnitXZ.y = 0.0f;
	IdealPushUnitXZ.z = m_afDZForce[m_nNumDZForces];
	IdealPushUnitXZ.UnitizeXZ();

	CFVec3A IdealRightUnitXZ;
	IdealRightUnitXZ.x = IdealPushUnitXZ.z;
	IdealRightUnitXZ.y = 0.0f;
	IdealRightUnitXZ.z = -IdealPushUnitXZ.x;

	CFSphereA OtherSphere(m_pAvoidThis->MtxToWorld()->m_vPos, aiutils_GetEntityRadXZ(m_pAvoidThis));
	CFSphereA MySphere(GetLoc(), GetRadiusXZ());  

	CFVec3A DeltaToOtherUnitXZ;  
	DeltaToOtherUnitXZ.Sub(OtherSphere.m_Pos, GetLoc());
	DeltaToOtherUnitXZ.y = 0.0f;

	f32 fDeltaVecMag = DeltaToOtherUnitXZ.SafeUnitAndMag(DeltaToOtherUnitXZ);
	if (fDeltaVecMag > 0.0f)
	{
		fDeltaVecMag -= OtherSphere.m_fRadius;
		fDeltaVecMag -= MySphere.m_fRadius;
		if (fDeltaVecMag < 0.0f)
		{
			fDeltaVecMag = 0.0f; //already penetrating each other's spheres
		}

		f32 fForwardDot = DeltaToOtherUnitXZ.Dot(IdealPushUnitXZ);
		if (fForwardDot > kfAvoidanceConeCos)	 //field of view apporx 78 degrees
		{
			f32 fPerpDot = IdealRightUnitXZ.Dot(DeltaToOtherUnitXZ);
			f32 fSteerFactor;

/*
			if (m_pAvoidThis &&
				m_pAvoidThis->AIBrain() &&
				m_pAvoidThis->AIBrain()->GetAIMover() &&
				m_pAvoidThis->AIBrain()->GetAIMover()->GetAvoidThis() == GetEntity() && 
		//		FMATH_FSIGN(m_pAvoidThis->AIBrain()->GetAIMover()->GetAvoidSteeringInfluence()) != FMATH_FSIGN(GetAvoidSteeringInfluence()) &&
				m_pAvoidThis->AIBrain()->GetAIMover()->GetAvoidThis() > GetEntity())	   //kinda goofy random test, maybe might look better to decide which one should turn other way, but....
			{   // if we're avoiding each other and
			    // turning toward the same direction
			    // not sure why, but don't let it happen. 
				// one of us turns the other way!
//				m_pBot->m_Controls.fForward *= 0.5f;
				fPerpDot *=-1.0f;  //one of us goes the other way
			}
*/

			if (fPerpDot > 0.0f)
			{
				//obstacle is to the right, turn left
				fSteerFactor = -1.0f+fPerpDot;
			}
			else
			{	//obstacle is to the left, turn right
				fSteerFactor = 1.0f+fPerpDot;
			}

			f32 fSteerFactorbrakes = 1.0f;
		
			//factor in how far away this obstacle is
			f32 fSearchAheadDist = s_testCyln.m_fHeight;  //SearchAheadDist
			fSteerFactorbrakes = fmath_Div((fSearchAheadDist-fDeltaVecMag), fSearchAheadDist);
			FMATH_CLAMP(fSteerFactorbrakes, 0.0f, 1.0f);
			fSteerFactor *= fSteerFactorbrakes;

			//factor in how directly in front of me this obstacles center is.
			fSteerFactorbrakes = fmath_Div(0.7f+(fForwardDot-kfAvoidanceConeCos), (1.0f-kfAvoidanceConeCos));
			FMATH_CLAMP(fSteerFactorbrakes, 0.0f, 1.0f);
			fSteerFactor *= fSteerFactorbrakes;

			//just scale the steering force as well
//			fSteerFactorbrakes = kfSteeringForceMagicScale;
//			FMATH_CLAMP(fSteerFactorbrakes, 0.0f, 1.0f);
//			fSteerFactor*=fSteerFactorbrakes;

			if (fDeltaVecMag < 0.1f && FMATH_FABS(fSteerFactor) < 0.1f)
			{
				fSteerFactor = FMATH_FSIGN(fSteerFactor)*0.1f;	   //what the heck?
			}

			f32 fGoalForceMag = m_afDXForce[m_nNumDXForces]*m_afDXForce[m_nNumDXForces] + m_afDZForce[m_nNumDZForces]*m_afDZForce[m_nNumDZForces];
			if (fGoalForceMag > 0.01f)
			{
				fGoalForceMag = fmath_Sqrt(fGoalForceMag);
			}

			CFVec3A AvoidForce;
			AvoidForce = IdealRightUnitXZ;
			AvoidForce.Mul(fSteerFactor);
			AvoidForce.x += m_afDXForce[m_nNumDXForces];
			AvoidForce.y = 0.0f;
			AvoidForce.z += m_afDZForce[m_nNumDZForces];
		//	AvoidForce.Mul(0.5f);
			if (AvoidForce.SafeUnitAndMag(AvoidForce) > 0.0f)
			{
				AvoidForce.Mul(fGoalForceMag);
			}
			else
			{
				AvoidForce = IdealRightUnitXZ;
			}

			m_afDXForce[FORCETYPE_AVOID] = AvoidForce.x;
			if (m_nNumDXForces < FORCETYPE_AVOID)
			{
				m_nNumDXForces = FORCETYPE_AVOID;
			}
			m_afDZForce[FORCETYPE_AVOID] = AvoidForce.z;
			if (m_nNumDZForces < FORCETYPE_AVOID)
			{
				m_nNumDZForces = FORCETYPE_AVOID;
			}

			if (m_fLastAvoidSteeringChange > fNowTimeSecs)
			{
				//use previous unstuck force>!!! Note
				if (m_nNumDXForces < FORCETYPE_UNSTUCK)
				{
					m_nNumDXForces = FORCETYPE_UNSTUCK;
				}
				if (m_nNumDZForces < FORCETYPE_UNSTUCK)
				{
					m_nNumDZForces = FORCETYPE_UNSTUCK;
				}
			}

			if (IsStuck(0.5f) &&
				m_pAvoidThis &&
				((m_nNumDXForces > FORCETYPE_PREVIOUS) || (m_nNumDZForces>FORCETYPE_PREVIOUS)) &&
				((m_uJumpStage == JUMP_STAGE_NOPE || m_uJumpStage == JUMP_STAGE_LOCKEDGE))) 	  //skip if we're jumping!
			{
				if (m_fLastAvoidSteeringChange < fNowTimeSecs)
				{
					m_fLastAvoidSteeringChange = fNowTimeSecs + 4.0f;

					CFVec3A AvoidForce;
					AvoidForce.x = m_afDXForce[FORCETYPE_AVOID];
					AvoidForce.y = 0.0f;
					AvoidForce.z = m_afDZForce[FORCETYPE_AVOID];

					CFVec3A IdealPushUnitXZ;
					IdealPushUnitXZ.x = m_afDXForce[FORCETYPE_GOAL];
					IdealPushUnitXZ.y = 0.0f;
					IdealPushUnitXZ.z = m_afDZForce[FORCETYPE_GOAL];
					IdealPushUnitXZ.UnitizeXZ();

					AvoidForce.Negate();
					CFVec3A UnstuckForce;
					UnstuckForce.ReceiveReflection(AvoidForce, IdealPushUnitXZ);

					m_afDXForce[FORCETYPE_UNSTUCK] = UnstuckForce.x;
					if (m_nNumDXForces < FORCETYPE_UNSTUCK)
					{
						m_nNumDXForces = FORCETYPE_UNSTUCK;
					}
					m_afDZForce[FORCETYPE_UNSTUCK] = UnstuckForce.z;
					if (m_nNumDZForces < FORCETYPE_UNSTUCK)
					{
						m_nNumDZForces = FORCETYPE_UNSTUCK;
					}
				}
			}
		// Got somewhere to go, but am currently stuck
	/*	if ((m_nNumDXForces > FORCETYPE_PREVIOUS) || (m_nNumDZForces>FORCETYPE_PREVIOUS))
		{
			//
			//  Which way to try to get unstuck
			//	 -- also, make sure direction doesn't change that often.
			//
			if ((m_uJumpStage == JUMP_STAGE_NOPE || m_uJumpStage == JUMP_STAGE_LOCKEDGE) &&	//jumpers don't come in here!
				 m_pAvoidThis &&	  //I was avoiding something before getting stuck
				!(m_uMoverFlags & MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME)  //first time
			{
				f32 fUnstuckSteerFactor = 0.5f*FMATH_FSIGN(m_afRotForce[FORCETYPE_PREVIOUS]);  //try to get continuing to rotate in the direction that bot was previously rotating in.

				if ((fNowTimeSecs - m_fLastOverrideSteeringChange) > 4.0f)
				{
					fUnstuckSteerFactor *= -1.0f;						  //unless, it has been four seconds since the last time It decided to change direction of rotation
					m_fLastOverrideSteeringChange = fNowTimeSecs;
				}

				m_nNumRotForces = FORCETYPE_UNSTUCK;
				m_afRotForce[FORCETYPE_UNSTUCK] = fUnstuckSteerFactor;
				m_uMoverFlags |= MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
				m_fLastAvoidSteeringChange = fNowTimeSecs;
			}
			else if (m_uMoverFlags & MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME)

			{
				f32 fUnstuckSteerFactor = 0.5f*FMATH_FSIGN(m_afRotForce[FORCETYPE_PREVIOUS]);
				m_nNumRotForces = FORCETYPE_UNSTUCK;
				m_afRotForce[FORCETYPE_UNSTUCK] = fUnstuckSteerFactor;
				m_fLastAvoidSteeringChange = fNowTimeSecs;
			}
			else
			{
				int i = 2;
			}
		}
		*/

		}
	}
			
/*

			if ((aiutils_FTotalLoopSecs() - m_fLastAvoidSteeringChange) > 4.0f ||
				(FMATH_FABS(m_afRotForce[FORCETYPE_AVOID]) < 0.01f) ||
				(FMATH_FSIGN(m_afRotForce[FORCETYPE_AVOID]) == FMATH_FSIGN(fSteerFactor)))
			{
				if (((FMATH_FABS(m_afRotForce[FORCETYPE_AVOID]) < 0.01f) ||
					(FMATH_FSIGN(m_afRotForce[FORCETYPE_AVOID]) != FMATH_FSIGN(fSteerFactor))))
				{
					m_fLastAvoidSteeringChange  = aiutils_FTotalLoopSecs();
				}
				m_afRotForce[FORCETYPE_AVOID] = fSteerFactor;
				if (m_nNumRotForces < FORCETYPE_AVOID+1)
				{
					m_nNumRotForces = FORCETYPE_AVOID;
				}
			}
		}
	}
*/
}


void CAIBotMover::BeginFrame(void)
{
	FASSERT(m_pBot);

	CAIMover::BeginFrame();
	m_nNumRotForces = 0;
	m_nNumDXForces = 0;
	m_nNumDZForces = 0;
	m_Controls.ResetButtons();	  //reset buttons at top of every frame
	m_uBotMoverFlags &= ~BOTMOVERFLAG_RECEIVED_EXTERNAL_TORSO_GOAL;
	m_uBotMoverFlags &= ~BOTMOVERFLAG_RECEIVED_EXTERNAL_MOVE_GOAL;
	m_Controls.SetAimDownGoal(0.0f);

	//MtxToWorld is the mount matrix!
	CBot* pBot = m_pBot;
	if (pBot->GetCurMech())
	{
		pBot = pBot->GetCurMech();
	}
	m_TorsoLookAtXZ.Set(GetTorsoLookAt().x, 0.0f, GetTorsoLookAt().z);
	f32 fMag = m_TorsoLookAtXZ.MagSq();
	if (fMag > 0.0f)
	{
		fMag = fmath_AcuSqrt(fMag);
		m_TorsoLookAtXZ.Mul(1.0f/fMag);
	}
	else
	{
		m_TorsoLookAtXZ = CFVec3A::m_UnitAxisX;
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

	//this makes it so that shorty's feet point forward whenever he is talking and standing still
	if (IsTalkingWithAnim())
	{
		m_pBot->OverrideHipSlackThisFrame();
	}
}

void CAIBotMover::_DoStuckWork(void)
{
//	f32 fNowTimeSecs = aiutils_FTotalLoopSecs();
	CFVec3A DeltaPos;
	DeltaPos.Sub(GetLoc(), m_pBot->m_MountPrevPos_WS);
	CFVec3A PushUnit;
	f32 fPushMag = PushUnit.SafeUnitAndMag(CFVec3A(m_Controls.GetEntityControl()->m_XlatNormSpeedXZ_WS.x, 0.0f, m_Controls.GetEntityControl()->m_XlatNormSpeedXZ_WS.z));
	f32 fDot = 0.0f;
	if (fPushMag > 0.0f)
	{
		fDot = DeltaPos.Dot(PushUnit);	   //fDot should have range of 0.0f - fPushMag
	}

	//
	// if normal controls indicate there is somewhere to go,
	// this this mover is stuck
	//
	if ((fPushMag > 0.0f && (m_nClientFeedback & FEEDBACK_STOPPED)) ||
		(fPushMag > 0.0f && fDot < 0.05f*fPushMag))	   //Stick is being pushed, but actual movement in that direction is less than .05 feet
	{
		//inc the stuck timer
		m_fStuckForSecs += FLoop_fPreviousLoopSecs;
	}
	else
	{
		//clear the stuck timer
		m_uMoverFlags &= ~MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME;
		m_fStuckForSecs = 0.0f;
	}
}

void CAIBotMover::EndFrame(void)
{
	if (m_pBot->m_Velocity_WS.MagSq() >= kfEssentiallyStoppedSpeed)
	{  //moving
		m_nRoomId = airooms_FindRoomID(GetLoc());
	}


	if (m_uJumpStage == JUMP_STAGE_INAIR)	 //when in this stage, our trajectory has been pre-computed,so no steering forces, no matter what.
	{
		m_afDXForce[m_nNumDXForces] = 0.0f;
		m_afDZForce[m_nNumDZForces]	= 0.0f;
		m_Controls.SetFlag_Slam_DX_Average();
		m_Controls.SetFlag_Slam_DZ_Average();
	}
	_FindObstacles();
	_AvoidObstacles();
	_DoStuckWork();

/*
	m_afRotForce[FORCETYPE_PREVIOUS] = 0.0f;
	m_afDXForce[FORCETYPE_PREVIOUS] = 0.0f;
	m_afDZForce[FORCETYPE_PREVIOUS] = 0.0f;
	m_nNumDXForces = 0;
	m_nNumDZForces = 0;
	m_nNumRotForces = 0;
*/
	m_Controls.SetRotateCWGoal(m_afRotForce[m_nNumRotForces]);
	m_Controls.SetDXGoal(m_afDXForce[m_nNumDXForces]);
	m_Controls.SetDZGoal(m_afDZForce[m_nNumDZForces]);

	//keep track of the controls given to the bot last time
	if (m_nNumRotForces > 0)
	{
		m_afRotForce[FORCETYPE_PREVIOUS] = m_Controls.GetRotateCWGoal();
	}
	else if (FMATH_FABS(m_afRotForce[FORCETYPE_PREVIOUS]) > 0.1f)
	{
		m_afRotForce[FORCETYPE_PREVIOUS] *= 0.8f;
	}
	else
	{
		m_afRotForce[FORCETYPE_PREVIOUS] = 0.0f;
	}

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

	if (m_uBotMoverFlags & BOTMOVERFLAG_RECEIVED_EXTERNAL_TORSO_GOAL)	  
	{
		m_uBotMoverFlags |= BOTMOVERFLAG_HAS_RECENT_TORSO_GOAL;
	}
	else
	{
		m_uBotMoverFlags &= ~BOTMOVERFLAG_HAS_RECENT_TORSO_GOAL;
	}

}


f32 CAIBotMover::DistToNextWay(void)
{
	CFVec3A tmp; 
	if (!m_PathWalker.GetCurLoc(&tmp))
	{
		FASSERT(0);
	}

	f32 fDistToNextWay = tmp.Dist(GetLoc());
	return fDistToNextWay;
}

BOOL CAIBotMover::IsSafeToStop(void)
{
	BOOL bCan = TRUE;
	if (m_uEdgeType & EF_EDGE_JUMP)
	{
		bCan = (m_uJumpStage == JUMP_STAGE_LOCKEDGE);
	}
	return bCan;
}


BOOL CAIBotMover::CanTalkNow(BOOL bRequireLowerBody /* FALSE*/)
{
	BOOL bCan = IsSafeToStop();
	// check for bot reloading weapon
	if (!bCan)
	{
		if (m_uEdgeType & EF_EDGE_DOOR)
		{
			bCan = (m_uDoorStage == DOOR_STAGE_WAITING) || (m_uDoorStage == DOOR_STAGE_FINDSWITCH);
		}
		else if (m_uEdgeType & EF_EDGE_LIFT)
		{
			bCan = (m_uLiftStage == LIFT_STAGE_WAITINGFOR) || (m_uLiftStage == LIFT_STAGE_FINDSWITCH);
		}
	
		if( m_pBot->IsReloading() )
		{
			bCan = FALSE;
		}
	}

	return bCan;

}






CAIHoverBotMover::CAIHoverBotMover(void)
: CAIBotMover()
{
	m_uMoverFlags |= MOVERFLAG_CAN_HOVER;
	for (u32 i = 0; i < NUM_FORCETYPES; i++)
	{
		m_afDYForce[i] = 0.0f;
	}
	m_nNumDYForces = 0;

	m_fMinAltitude = 0.0f;

}


CAIHoverBotMover::~CAIHoverBotMover(void)
{
}



void CAIHoverBotMover::BeginFrame(void)
{
	CAIBotMover::BeginFrame();
	m_nNumDYForces = 0;
	m_uBotMoverFlags &=~BOTMOVERFLAG_RECEIVED_HOVER_GOAL;
}


void CAIHoverBotMover::EndFrame(void)
{

	if (!(m_uBotMoverFlags & BOTMOVERFLAG_RECEIVED_HOVER_GOAL) &&
		m_fMinAltitude > 0.0f)
	{
		if (GetLastGraphVertId()) 
		{
			GraphVert* pV = aimain_pAIGraph->GetVert(GetLastGraphVertId());
			HoverTowardY(pV->m_Location.y + m_fMinAltitude + m_pBrain->GetInfreqCycleRandom()*(pV->m_fHeightClearance- m_fMinAltitude - GetHeight()));
		}
		else
		{

		}
	}



	if (m_pBot->TypeBits() & ENTITY_BIT_BOTJUMPER && ((CBotJumper*) m_pBot)->IsDiving())
	{
		m_afRotForce[m_nNumRotForces] = 0.0f;
		m_afDXForce[m_nNumDXForces] = 0.0f;
		m_afDYForce[m_nNumDYForces] = 0.0f;
		m_afDZForce[m_nNumDZForces]	= 0.0f;
		m_Controls.SetFlag_Slam_DX_Average();
		m_Controls.SetFlag_Slam_DY_Average();
		m_Controls.SetFlag_Slam_DZ_Average();

	}

	if (m_uJumpStage == JUMP_STAGE_INAIR)	 //when in this stage, our trajectory has been pre-computed,so no steering forces, no matter what.
	{
//		m_afRotForce[m_nNumRotForces] = 0.0f;
		m_afDYForce[m_nNumDYForces] = 0.0f;
		m_Controls.SetFlag_Slam_DY_Average();
		m_Controls.GetEntityControl()->m_nButtons &= ~CBotControl::BUTTONFLAG_JUMP2; //for jumper bots, this tells the bot class that it should behave based on the flyup/flydown
	}
	m_Controls.SetDYGoal(m_afDYForce[m_nNumDYForces]);

	//keep track of the controls given to the bot last time
	if (m_nNumDYForces > 0)
	{
		m_afDYForce[FORCETYPE_PREVIOUS] = m_Controls.GetDYGoal();
	}
	else if (FMATH_FABS(m_afDYForce[FORCETYPE_PREVIOUS]) > 0.1f)
	{
		m_afDYForce[FORCETYPE_PREVIOUS] -= 0.15f;
		FMATH_CLAMPMIN(m_afDYForce[FORCETYPE_PREVIOUS], -1.0f);
	}
	else
	{
		m_afDYForce[FORCETYPE_PREVIOUS] = -1.0f;
	}

	CAIBotMover::EndFrame();
}

const f32 kfAbsolutelyCloseEnoughDistY = 0.75f;


BOOL CAIHoverBotMover::HoverTowardY(f32 fY_WS)
{
	BOOL bYCloseEnough = FALSE;

	m_Controls.JumpHigh(); //for jumper bots, this tells the bot class that it should behave based on the flyup/flydown

	//
	//  FlyUp / Down
	//
	f32 fBotY = GetLoc().y;
	f32 fGoalY = fY_WS;
	f32 fAbsDeltaY = FMATH_FABS(fGoalY-fBotY);
	if (fAbsDeltaY < kfAbsolutelyCloseEnoughDistY) //Close enough, just don't move
	{
		m_afDYForce[FORCETYPE_GOAL] = 0.0f;
		if (m_nNumDYForces <= FORCETYPE_GOAL)
		{
			m_nNumDYForces = FORCETYPE_GOAL;
			m_Controls.SetFlag_Slam_DY_Average();
		}
		return bYCloseEnough;
	}
	else
	{	//adjust height!

		//
		// Speed control
		//
		f32 fSpeedY = 1.0f;

/*
	if (m_pBot->m_fMaxVerticalSpeed_WS > 0.0f)
		{
			FMATH_CLAMPMAX(fSpeedY, m_fForwardGoalSpeed / m_pBot->m_fMaxVerticalSpeed_WS);
		}
*/
		if (fAbsDeltaY < GetRadiusXZ())
		{
			fSpeedY = fAbsDeltaY/GetHeight();
			fSpeedY*=fSpeedY;
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
	}


	return bYCloseEnough;
}

