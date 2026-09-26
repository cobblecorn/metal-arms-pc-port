#include "fang.h"
#include "flinklist.h"
#include "fScriptSystem.h"
#include "AIThoughtsGeneric.h"
#include "AIMain.h"
#include "AIBrainman.h"
#include "AIBrain.h"
#include "AiBrainMems.h"
#include "AIMover.h"
#include "AIBotMover.h"
#include "AIGraph.h"
#include "AIFormations.h"
#include "AINodePools.h"
#include "AIRooms.h"
#include "ApeNew.h"
#include "AiApi.h"
#include "AiBrainUtils.h"
#include "AIBtatable.h"
#include "AIGameUtils.h"
#include "AIPatrolPath.h"
#include "AIEnviro.h"
#include "AIBTAtable.h"
#include "AIGroup.h"
#include "AIFSM.h"
#include "AIEdgeLock.h"
#include "AIThoughtsGround.h"
#include "../Bot.h"
#include "../botminer.h"
#include "../BotTalkInst.h"
#include "../BotTalkData.h"
#include "../Entity.h"
#include "../ESpline.h"
#include "../GString.h"
#include "../MultiPlayerMgr.h"
#include "../Player.h"
#include "../Site_BotWeapon.h"
#include "../Weapon.h"


extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );

static const f32 kfOOHun = 1.0f/100.0f;



//
//
//  Generic Wait
//
//
//
//static
CFVec3A CGenericWait::s_aVisRay[CGenericWait::NUM_VISRAYS] = 
{
	CFVec3A(0.0f, 0.0f, 1.0f),
	CFVec3A(1.0f, 0.0f, 1.0f),
	CFVec3A(1.0f, 0.0f, 0.0f),
	CFVec3A(1.0f, 0.0f, -1.0f),
	CFVec3A(0.0f, 0.0f, -1.0f),
	CFVec3A(-1.0f, 0.0f, -1.0f),
	CFVec3A(-1.0f, 0.0f, 0.0f),
	CFVec3A(-1.0f, 0.0f, 1.0f),
};
BOOL CGenericWait::s_bVisRaysInitialized = FALSE;
const f32 _WAIT_SCANHIGH_TAN_THETA = 2.0f;
const f32 _WAIT_SCANLOW_TAN_THETA = 1.0f;
const f32 _WAIT_TORSO_SCANRADMAX = FMATH_HALF_PI*0.9f;
const f32 _WAIT_VISRAYSCAN_LOS_DIST = 2.0f;	   //radxz is added to this
const f32 _WAIT_TORSO_SCAN_SPEED_RPS = 0.3f;

void CGenericWait::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /* = NULL*/)
{
	BOOL bSpecifyLookAt = FALSE;	// TRUE/FALSE  (TRUE means use pLookAtObj or LookAtLoc) FALSE means nothing to look at
	u32 uLookAtObjGUID = 0;			// The GUID of an entity to keep looking at (0 means ignore)

	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);

	FASSERT(!pParamPack || pParamPack->m_uMemoryId == MEMORY_LARGE_PARAMPACK);

	m_uWaitFlags = WAIT_NONE;
	m_pLookAtObj = NULL;
	m_uLookAroundChance = 0;
	m_uLookAroundChanceFreqMin = 25;
	m_uLookAroundChanceFreqMax = 35;
	m_uNextLookAround = 0;
	m_fPausedVisRayScanTimeOut = 0.0f;
	m_uPausedVisRayScanMin = 2;
	m_uPausedVisRayScanMax = 4;
	m_fTorsoScanTimer = 0.0f;
	m_fPausedTorsoScanTimeOut = 0.0f;
	m_uPausedTorsoScanMin = 1;			//min length of a TorsoScan pause
	m_uPausedTorsoScanMax = 2;			//max length of a TorsoScan pause
	m_uPausedTorsoScanOdds = 60; 
	m_uNextLOSFrameTimeOut = 0;
	m_uLOSFrameDelay = 3;

	if (pParamPack)
	{
		ExtractWaitParams((CAIMemoryLarge*) pParamPack,
						&bSpecifyLookAt,				// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						&(m_LookAtLoc),					// A position to look at (Ignored if LookAtObjGUID > 0)	
						&uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means ignore)
						&m_uLookAroundChance);			// (0-100) Chance that obj will look around every now and again.
	}

	if (bSpecifyLookAt)
	{
		m_uWaitFlags |= WAIT_LOOKAT_WAS_SPECIFIED;
	}

	if (uLookAtObjGUID)
	{
		m_pLookAtObj = CEntity::Find(uLookAtObjGUID);
		if (!m_pLookAtObj)
		{	//couldn't find the entity I was told to look at while waiting
			m_uWaitFlags &= ~WAIT_LOOKAT_WAS_SPECIFIED;
		}
	}
	
	ResetVisRayScan();
	
	SetNextLookAround();
	m_CurLookAroundLoc = m_pBrain->GetAIMover()->GetEyeLookAt();

}


void CGenericWait::Cleanup(void)
{
	CAIThought::Cleanup();
}


void CGenericWait::SetNextLookAround(void)
{
	m_uNextLookAround = aiutils_GetCurTimeSecsU16() + m_uLookAroundChanceFreqMin + (u16) fmath_RandomChoice(m_uLookAroundChanceFreqMax-m_uLookAroundChanceFreqMin);		  //8+15=23 seconds
}


BOOL CGenericWait::TakeLookAroundChance(void)
{
	BOOL bBeginLookAround = FALSE;
	if (m_uLookAroundChance && m_uNextLookAround < aiutils_GetCurTimeSecsU16())
	{	
		if (fmath_RandomChoice(100) < m_uLookAroundChance)
		{
			bBeginLookAround = TRUE;
		}
		SetNextLookAround();
	}

	return bBeginLookAround;
}


void CGenericWait::ForceTakeLookAroundChance(void)
{
	m_uNextLookAround = 0;
}


void CGenericWait::BeginLookAround(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	m_CurLookAroundLoc = CFVec3A::m_UnitAxisZ;
	m_CurLookAroundLoc.Mul(pMover->GetRadiusXZ()+_WAIT_VISRAYSCAN_LOS_DIST);
	m_CurLookAroundLoc.RotateY(FMATH_2PI*fmath_RandomFloat());
	m_CurLookAroundLoc.Add(pMover->GetEyePos());

	if (m_uWaitFlags & WAIT_USE_MIN_LOS_DIST)
	{
		BOOL bLOSObstructed = aiutils_IsLOSObstructed_IgnoreBots(pMover->GetEyePos(), m_CurLookAroundLoc, pMover->GetEntity());
		aiutils_DebugTrackRay(pMover->GetEyePos(), m_CurLookAroundLoc, !bLOSObstructed);
		if (bLOSObstructed)
		{
		   m_uNextLookAround = 0;
		   return;
		}
	}
	m_uWaitFlags |= WAIT_LOOKING_AROUND;
}




void CGenericWait::InitializeVisRays()
{
	s_bVisRaysInitialized = TRUE;
	for (u32 i = 0; i < NUM_VISRAYS;i++)
	{
		s_aVisRay[i].Unitize();
	}
}


u8 CGenericWait::GetClosestVisRay_Pt(const CFVec3A& Pt, const CFVec3A& Origin)
{
	CFVec3A Vec;
	Vec.Sub(Pt, Origin);

	f32 fHeading = fmath_Atan( Vec.x, Vec.z );
	return ((u8)	(NUM_VISRAYS * fHeading/FMATH_2PI))&0x7;
}


u8 CGenericWait::GetClosestVisRay_Vec(const CFVec3A& Vec)
{
	f32 fHeading = fmath_Atan( Vec.x, Vec.z );
	return ((u8)	(NUM_VISRAYS * fHeading/FMATH_2PI))&0x7;
}


const CFVec3A& CGenericWait::VisRayToLookAtLoc(u8 uVisRay, const CFVec3A& Origin,  f32 fScanDist)
{
	static CFVec3A _RayEnd;
	_RayEnd = s_aVisRay[uVisRay];
	_RayEnd.Mul(fScanDist);
	_RayEnd.Add(Origin);
	return _RayEnd;
}


void CGenericWait::ResetVisRayScan(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	if (!s_bVisRaysInitialized)
	{
		InitializeVisRays();
	}

	fang_MemZero(m_auScannerStatus, NUM_VISRAYS*sizeof(u8));
   	CFVec3A InitPt;
	InitPt.Set(pMover->GetEyeLookAt());
	m_uCurScanner = GetClosestVisRay_Vec(InitPt);

	m_uCurScan = m_uCurScanner;
	m_uLastScan = 0xff;
	m_nCurScanDir = 1;
	m_uDirChange = 0;
	m_uScansSinceDirChange=0;
	m_uDirChangeCount = 0;
	m_uAutoVisRayUpdateTimeOut = 0;
	m_uNextLOSFrameTimeOut = 0;
}


void CGenericWait::UpdateVisRays(const CFVec3A& Origin, f32 fScanDist)
{
	if (m_uWaitFlags & WAIT_SMART_UPDATE_VISRAYS &&
		(m_uAutoVisRayUpdateTimeOut < aiutils_GetCurTimeSecsU16()) &&
		m_pBrain->HasMovedOneFootSince(3.0f)) 
	{
		if (m_pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT &&
			((CBot*) m_pBrain->GetAIMover()->GetEntity())->m_fSpeedXZ_WS > 0.1f)
		{
			ResetVisRayScan();
		}
		else
		{
			ResetVisRayScan();
		}

		m_uAutoVisRayUpdateTimeOut = aiutils_GetCurTimeSecsU16() + 3;
	}


	//
	//  if there are any visrays that are UNSET
	//  or WAIT_TIMER_UPDATE_VISRAYS is on and it is time	for a test,
	//
	if (m_auScannerStatus[m_uCurScanner] == VISRAY_UNSET ||
		((m_uWaitFlags & WAIT_TIMER_UPDATE_VISRAYS) && (m_uNextLOSFrameTimeOut < FVid_nFrameCounter)))
	{
		CFVec3A RayEnd;
		RayEnd = s_aVisRay[m_uCurScanner];
		RayEnd.Mul(fScanDist);
		RayEnd.Add(Origin);
		BOOL bLOSObstructed = aiutils_IsLOSObstructed_IgnoreBots(Origin, RayEnd, NULL);	//findfix:GetBrain()->GetAIMover()->GetEntity()//FilterOut Bots
		aiutils_DebugTrackRay(Origin, RayEnd, !bLOSObstructed);

		m_uNextLOSFrameTimeOut = FVid_nFrameCounter + m_uLOSFrameDelay;

		m_auScannerStatus[m_uCurScanner] = VISRAY_PASSED - bLOSObstructed;
		
		m_uCurScanner++;
		m_uCurScanner &= 0x7;
	}
}


void CGenericWait::ForceLookAtWhileScanning(const CFVec3A& Pt)
{
	m_uLastScan = m_uCurScan;
	m_uCurScan = GetClosestVisRay_Pt(Pt, m_pBrain->GetAIMover()->GetLoc());
	m_fPausedVisRayScanTimeOut = 0.0f;						//
	m_uScansSinceDirChange = NUM_VISRAYS_IN_MAX_RANGE;		// together should cause a go-to, then pause
	m_uDirChangeCount = 0;									// 
}															


BOOL CGenericWait::IsVisRayScanPaused(void)
{
	return m_fPausedVisRayScanTimeOut > aiutils_FTotalLoopSecs();
}

void CGenericWait::DoTorsoScanWork(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	f32 fAlignment;

	//
	// Logic for scanning the torso (and naturaly head) from left to right
	//

	if (m_fPausedTorsoScanTimeOut < aiutils_GetCurTimeSecs())
	{

		f32 fScanRadDir = 1.0f-(2.0f*(!!(m_uWaitFlags & WAIT_TORSO_SCAN_DIR_POS)));
		m_fTorsoScanTimer += fScanRadDir*FLoop_fPreviousLoopSecs*FMATH_2PI*_WAIT_TORSO_SCAN_SPEED_RPS;
	}
	else
	{	//paused for a bit, don't change the torsoScan
		int rr = 44;

	}

	//scan torso
	f32 fTorsoScanYawMax = _WAIT_TORSO_SCANRADMAX;
	if (m_pBrain->GetAIMover()->GetEntity() &&
		(m_pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) &&
		((CBot*) m_pBrain->GetAIMover()->GetEntity())->GetCurMech() &&
		(((CBot*) m_pBrain->GetAIMover()->GetEntity())->GetCurMech()->TypeBits() & ENTITY_BIT_SITEWEAPON))
	{
		CBotSiteWeapon* pSite = (CBotSiteWeapon*) ((CBot*) m_pBrain->GetAIMover()->GetEntity())->GetCurMech();
		f32 tmp[4];
		pSite->GetTurretRanges(tmp, tmp+1, tmp+2, tmp+3);

		fTorsoScanYawMax = FMATH_MIN(tmp[0], tmp[1]);
	}

	if (m_fTorsoScanTimer >= _WAIT_TORSO_SCANRADMAX || m_fTorsoScanTimer <= -_WAIT_TORSO_SCANRADMAX)
	{
		m_uWaitFlags ^= WAIT_TORSO_SCAN_DIR_POS;
		FMATH_CLAMP(m_fTorsoScanTimer, -_WAIT_TORSO_SCANRADMAX, _WAIT_TORSO_SCANRADMAX);

		f32 fScanRadDir = 1.0f-(2.0f*(!!(m_uWaitFlags & WAIT_TORSO_SCAN_DIR_POS)));
		m_fTorsoScanTimer += fScanRadDir*FLoop_fPreviousLoopSecs*1.5f;

		//should we pause?
		if (fmath_RandomChoice(100) < m_uPausedTorsoScanOdds)
		{	//yes
			if (m_uPausedTorsoScanMax > m_uPausedTorsoScanMin)
			{
				m_fPausedTorsoScanTimeOut = (f32)(aiutils_GetCurTimeSecsU16() + m_uPausedTorsoScanMin + fmath_RandomChoice(m_uPausedTorsoScanMax - m_uPausedTorsoScanMin));
			}
		}

	}
	CFVec3A AtXZ;
	BOOL bPausedTorsoScan = FALSE; 
	if (m_fPausedTorsoScanTimeOut > aiutils_GetCurTimeSecs())
	{
		bPausedTorsoScan = TRUE;
	}

	AtXZ = m_pBrain->GetAIMover()->GetTorsoLookAtXZ();

	if ((m_pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT) &&
		 ((CBot*) m_pBrain->GetAIMover()->GetEntity())->m_fSpeedXZ_WS < 0.01f)
	{
		if (m_uWaitFlags & WAIT_AUTOSCAN_TORSO_WHILE_STANDING)
		{
			if (((CBot*) m_pBrain->GetAIMover()->GetEntity())->GetCurMech() && 
				((CBot*) m_pBrain->GetAIMover()->GetEntity())->GetCurMech()->TypeBits() & ENTITY_BIT_SITEWEAPON)
			{
				AtXZ = ((CBotSiteWeapon*) ((CBot*) m_pBrain->GetAIMover()->GetEntity())->GetCurMech())->BaseMtxToWorld()->m_vFront;
				AtXZ.RotateY(m_fTorsoScanTimer);  //m_fTorsoScanBaseRadWhileStopped
			}
			else
			{
				AtXZ = m_CurLookAroundLoc;
				AtXZ.RotateY(m_fTorsoScanTimer);  //m_fTorsoScanBaseRadWhileStopped
			}
		}
	}
	else
	{
		if (m_uWaitFlags & WAIT_AUTOSCAN_TORSO_WHILE_WALKING)
		{
			AtXZ = ((CBot*) m_pBrain->GetAIMover()->GetEntity())->m_UnitVelocityXZ_WS;

			if (((CBot*) m_pBrain->GetAIMover()->GetEntity())->m_UnitVelocityXZ_WS.Dot(m_pBrain->GetAIMover()->GetTorsoLookAtXZ()) < 0.0f)
			{	//walking backwards
				int i = 2;
				m_fPausedTorsoScanTimeOut = 0.0f; //become un-paused
			}
			else
			{	//walking forward
				AtXZ.RotateY(m_fTorsoScanTimer);	 //Turn right
			}
			m_CurLookAroundLoc = AtXZ;
		}
	}


	AtXZ.Mul(8.0f);
	AtXZ.Add(m_pBrain->GetAIMover()->GetEyePos());

	m_pBrain->GetAIMover()->FaceToward(AtXZ, &fAlignment);
}


//Scan the VisRays 
//	- ping-pong between the min and max ranges within the Visible Arc 
//	- Occasionally, do a full scan of ALL the vis-rays that have been checked and passed the los test
//  - Pause at Min/Max of Visible Range or 360 degrees if no rays blocked
void CGenericWait::DoVisRayScan(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	f32 fAlignment;

	//
	// look in the direcion of the current scan ray
	//
	pMover->m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_USE_HEADLOOK;

	CFVec3A LookAtPt_WS = VisRayToLookAtLoc(m_uCurScan, *(pMover->GetEntity()->GetTagPoint(0)), 5.0f);
	pMover->FaceToward(LookAtPt_WS, &fAlignment);
	if (pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
	{	//tell head to look there as well
		LookAtPt_WS.y += 1.4f;		  //eh?
		pMover->m_Controls.GetEntityControl()->m_HeadLookPt_WS = LookAtPt_WS;
		pMover->m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_USE_HEADLOOK;
		if (m_uLastScan != m_uCurScan)
		{
			pMover->m_Controls.GetEntityControl()->m_nFlags |= CBotControl::FLAG_NEW_HEADLOOK_LOCATION;
		}
	}
	m_uLastScan = m_uCurScan;

	//
	// Advance current scan ray
	//
	if (fAlignment > 0.9f )
	{
		//close enough, change scan dir
		u8 uNextScan = (m_uCurScan+m_nCurScanDir)&0x7;
		if ((m_auScannerStatus[m_uCurScan] == VISRAY_PASSED && m_auScannerStatus[uNextScan] == VISRAY_FAILED ) ||
			m_uScansSinceDirChange > NUM_VISRAYS_IN_MAX_RANGE)

		{
			s8 nNewScanDir = -m_nCurScanDir;  //change direction
			if (m_uDirChangeCount > 2+fmath_RandomChoice(2))
			{

				if (m_auScannerStatus[uNextScan] != VISRAY_FAILED )		 
				{	// fake out a scandir direction change
					// so that none actually occurs

					FASSERT(m_uScansSinceDirChange > NUM_VISRAYS_IN_MAX_RANGE);
					nNewScanDir = m_nCurScanDir;   //NO, Don't change direction
					m_uDirChangeCount = 0;	//extend the range
					m_uScansSinceDirChange = 0;
				}

			}
			if (nNewScanDir != m_nCurScanDir)
			{
				u8 uRange = 0;
				if (m_uPausedVisRayScanMax > m_uPausedVisRayScanMin)
				{
					uRange = m_uPausedVisRayScanMax - m_uPausedVisRayScanMin;
				}
				m_fPausedVisRayScanTimeOut = aiutils_FTotalLoopSecs() + (f32)m_uPausedVisRayScanMin + (f32)fmath_RandomChoice(uRange);
				m_uDirChangeCount++;
				m_uScansSinceDirChange=0;
			}
			else
			{
			//	m_fPausedVisRayScanTimeOut = 0.0f;
			}
			m_nCurScanDir = nNewScanDir;

			uNextScan = (m_uCurScan+m_nCurScanDir)&0x7;
		}

		if (m_auScannerStatus[uNextScan] == VISRAY_PASSED || m_auScannerStatus[m_uCurScan] == VISRAY_FAILED)
		{
			m_uLastScan = m_uCurScan;
			m_uCurScan = uNextScan;
			m_uScansSinceDirChange++;
		}
	}

}


void CGenericWait::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();

	if (m_uWaitFlags & WAIT_VISRAYS_ENABLED)
	{
		//Maintian visibility info for eight compass directions in the XZ plane at the height of tagpt0
		UpdateVisRays(*(pMover->GetEntity()->GetTagPoint(0)), pMover->GetRadiusXZ()+_WAIT_VISRAYSCAN_LOS_DIST);

		if (!IsVisRayScanPaused()) 
		{	
			DoVisRayScan();
		}
		else if (m_uWaitFlags & WAIT_AUTOSCAN_TORSO)
		{
			DoTorsoScanWork();
		}
	}
	else if (m_uWaitFlags & WAIT_AUTOSCAN_TORSO)
	{
		DoTorsoScanWork();
	}
	else
	{
		if (TakeLookAroundChance())
		{
			BeginLookAround();
		}

		if (m_uWaitFlags & WAIT_LOOKING_AROUND)
		{
			if (pMover->FaceToward(m_CurLookAroundLoc))
			{
				//did it, turn off the look around 
				m_uWaitFlags &= ~WAIT_LOOKING_AROUND;
			}
		}
		else if ((m_uWaitFlags & WAIT_LOOKAT_WAS_SPECIFIED))
		{
			if (m_pLookAtObj)
			{
				pMover->FaceToward(aiutils_GetEntityLoc(m_pLookAtObj));
			}
			else
			{
				pMover->FaceToward(m_LookAtLoc);
			}
		}
	}


	if (GetBrain()->GetMechLock() && pMover->GetCurMech())
	{
		//init pill head look behavior	 (will only be used when normal torso and head controls determin target has been lost
		m_uWaitFlags |= (CGenericWait::WAIT_AUTOSCAN_TORSO | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_STANDING | CGenericWait::WAIT_AUTOSCAN_TORSO_WHILE_WALKING);
		m_uWaitFlags &= ~CGenericWait::WAIT_VISRAYS_ENABLED;
	}
	else if (GetBrain()->GetCurThoughtPtr() == this &&
		GetBrain()->GetMechUseageCtrl() == CAIBrain::MECH_USEAGE_CTRL_INIT &&
		!GetBrain()->GetFlag_MechUseageCtrlInitFailed() &&
		!(m_uWaitFlags & WAIT_ATTEMPTED_INITVEHICLEACCESS))
	{
		if (!GetBrain()->GetMechLock())
		{
			CBot* pNearbyVehicle = NULL;
			if (GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_PILL)
			{
				pNearbyVehicle = aigroup_FindOpenPillBox(GetBrain(), (f32) GetBrain()->GetMechUseageRadius());
			}
			if (!pNearbyVehicle && GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_LOADER)
			{
				pNearbyVehicle = aigroup_FindOpenLoader(GetBrain(), (f32) GetBrain()->GetMechUseageRadius());
			}
			if (!pNearbyVehicle && GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_SENTINEL)
			{
				pNearbyVehicle = aigroup_FindOpenSentinel(GetBrain(), (f32) GetBrain()->GetMechUseageRadius());
			}
			if (!pNearbyVehicle && GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_RAT)
			{
				pNearbyVehicle = aigroup_FindOpenRat(GetBrain(), (f32) GetBrain()->GetMechUseageRadius());
			}
			if (!pNearbyVehicle && GetBrain()->GetMechUseageTypeFlag() & CAIBrain::MECH_USEAGE_TYPE_FLAG_RATGUN)
			{
				pNearbyVehicle = aigroup_FindOpenRatGun(GetBrain(), (f32) GetBrain()->GetMechUseageRadius());
			}
			if (pNearbyVehicle && ResLock_RequestMechSeatLock(RESLOCK_LOCK_ONLY, pNearbyVehicle, 0, GetBrain()->GetGUID()))
			{
				GetBrain()->SetMechLock(pNearbyVehicle, 0);
			}
		}

		if (GetBrain()->GetMechLock())
		{
			//try to click-in
			GetBrain()->GetAIMover()->RequestMechEntry(GetBrain()->GetMechLock(), CAIMover::MECH_ENTRY_NOPROXIMITYCHECK);
			
			//did it work?
			if (!((CBot*) GetBrain()->GetAIMover()->GetEntity())->GetCurMech())
			{  //didn't gain access, release lock and try using a "blind" attack order to get into the vehicle
				ResLock_FreeMechSeatLock(GetBrain()->GetMechLock(), 0, GetBrain()->GetGUID());
				FASSERT(!ResLock_HasMechSeatLocked((u32) GetBrain()->GetGUID()));
				GetBrain()->ClearMechLock();
				m_uWaitFlags |= WAIT_ATTEMPTED_INITVEHICLEACCESS;
				ai_AssignGoal_Attack(m_pBrain, 0, 0);	//giving a brief attack should cause bots to get in their vehicles.
			}
		}
		else
		{
			FASSERT(!ResLock_HasMechSeatLocked((u32) GetBrain()->GetGUID()));
			//if you can't even get a lock in the first frame, then the Init failed.  Should only try to gain access to vehicles as if AUTO was specified
			GetBrain()->SetFlag_MechUseageCtrlInitFailed();
		}
	}

}


//
//
//  Ground Wander			  
//
//
u32 kNumDifferentSpeeds = 4;
f32 kTestSpeedScale = 0.2f;

void CGenericWander::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /* = NULL*/)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	CAIMover* pMover = m_pBrain->GetAIMover();

	//default values
	m_WanderOrigin.Zero();
	m_WanderGoal.Zero();
	m_fWanderRadius = 0.0f;
	m_uSpeedPctMin = 0;
	m_uSpeedPctMax = 0;
	m_uWanderFlags = 0;
	m_SearchQuery.Clear();
	m_SearchResults[0].Clear();
	m_SearchResults[1].Clear();
	m_uTakeBreakChance = 0;
	m_uWanderPt = 0;
	m_pWanderPts = NULL;
	m_uWanderPtCounter = 0;
	m_uTakeBreakDecisionTimeOut = aiutils_GetCurTimeSecsU16() + 20 +(u16) fmath_RandomChoice(5);	//make next break decision at this time
	m_uSociable = 0;
	m_uLastGoalVertId = 0;
	m_uLastLastGoalVertId  = 0;

	if (aiutils_CanEntityFly(pMover->GetEntity()))
	{
		m_uWanderFlags |= WANDER_3DBOT;
	}

	if (pParamPack)
	{
		BOOL bStayInRoom;
		cchar* pszPointList = NULL;
		ExtractWanderParams((CAIMemoryLarge*) pParamPack, &m_fWanderRadius, &bStayInRoom, &m_uSpeedPctMin, &m_uSpeedPctMax, &m_uSociable, &m_uCuriosity, &m_uTakeBreakChance, &pszPointList);

		if (m_uSpeedPctMax < m_uSpeedPctMin)
		{
			m_uSpeedPctMax = m_uSpeedPctMin;
		}
		
		if (m_uSpeedPctMin || m_uSpeedPctMax)
		{
			m_pBrain->PushBaseSpeedSetting(m_uSpeedPctMin + (u8) fmath_RandomChoice(m_uSpeedPctMax - m_uSpeedPctMin));
		}

		if (bStayInRoom)
		{
			m_uWanderFlags |= WANDER_STAYINROOM;
		}

		if (pszPointList)
		{
			CEntity* pE = CEntity::Find(pszPointList);
			if (pE && pE->TypeBits() & ENTITY_BIT_SPLINE)
			{
				m_pWanderPts = (CESpline*) pE;
				f32 fRadXZ = pMover->GetRadiusXZ();
				f32 fHeight = pMover->GetHeight();
				CPatrolPath::Register(m_pWanderPts, (u8) fRadXZ, (u8) fHeight, !!(m_uWanderFlags & WANDER_3DBOT));
			}
			else
			{
				DEVPRINTF("AI: PatrolPath can't find spline entity named %s\n", pszPointList);
			}
		}	
	}

	if (m_fWanderRadius==0.0f)
	{	
		m_uWanderFlags |= WANDER_IGNORERADIUSLIMIT;
	}
	m_WanderOrigin.Set(m_pBrain->GetLoc());
	m_uStage = WANDERSTAGE_NEEDS_PATH;
	m_uFailedEdgeCount = 0;
}


void CGenericWander::Cleanup(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	CAIThought::Cleanup(); //remove the parampack from memory
	
	//Don't leave mover following a path that this thought gave it
	if (m_pBrain &&
		m_pBrain->GetAIMover() &&
		m_pBrain->GetAIMover()->m_pPath &&
		(m_pBrain->GetAIMover()->m_pPath == m_SearchResults[0].GetAIPath() || 
		m_pBrain->GetAIMover()->m_pPath == m_SearchResults[1].GetAIPath()))
	{
		m_pBrain->GetAIMover()->AssignPath(NULL);
	}
	aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
	m_SearchQuery.Clear();
	m_SearchResults[0].Clear();
	m_SearchResults[1].Clear();

	if (m_uSpeedPctMin || m_uSpeedPctMax)
	{
		m_pBrain->PopBaseSpeedSetting();
	}
}


BOOL CGenericWander::FindWanderPtFromPtArray(CFVec3A * pWanderLoc)
{
	const CFVec3A* paPts;
	if (pWanderLoc && m_pWanderPts  && (paPts = m_pWanderPts->PointArray()))
	{
		*pWanderLoc = paPts[m_uWanderPt];
		if (m_uWanderPtCounter >=69)
		{
			m_uWanderPt = (m_uWanderPt+1)%(u16)m_pWanderPts->PointCount();
			m_uWanderPtCounter++;
			if (m_uWanderPtCounter > 69+(u16)m_pWanderPts->PointCount())
			{
				m_uWanderPtCounter = 0;
			}
		}
		else
		{
			m_uWanderPt = (u8) fmath_RandomChoice((u16)m_pWanderPts->PointCount());
			m_uWanderPtCounter++;
			if (m_uWanderPtCounter > 6+fmath_RandomChoice((u16) 4 ))
			{
				m_uWanderPtCounter = 69;
			}
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CGenericWander::FindXZGraphWanderPt(CFVec3A * pWanderLoc)
{
	CAIGraph* pGraph = aimain_pGraphSearcher->GetGraph();
	CAIMover* pMover = m_pBrain->GetAIMover();
	CFVec3A tmp;
	GraphVert* pV = NULL;

	u16 nThisRoomOnly = 0;
	GraphVert* pCurVert = NULL;

	if (m_uWanderFlags & WANDER_STAYINROOM)
	{
		nThisRoomOnly = (u16) pMover->m_nRoomId;
	}

	//count verts options first
	s32 nOptions = 0;
	CFVec3A KneeLoc;
	KneeLoc = pMover->GetLoc();
	KneeLoc.y+=1.7f;

	if (m_uLastGoalVertId)
	{
		pCurVert = pGraph->GetVert(m_uLastGoalVertId);
	}
	else
	{
		pCurVert = pGraph->FindClosestLOSVert2D(KneeLoc);
	}

	if (pCurVert)
	{
		for (u8 e = 0; e < pCurVert->GetNumEdges(); e++)
		{
			pV = pCurVert->GetOppVert(e, pGraph);

			if (!(m_uWanderFlags & WANDER_IGNORERADIUSLIMIT) && 
				pV->GetLocation().DistSq(m_WanderOrigin) > (pV->GetRad()+m_fWanderRadius)*(pV->GetRad()+m_fWanderRadius))
			{
				continue;  //only find verts who's center is withing wander Radius
			}
			aiutils_anGraphVertSortBuff[nOptions] = pCurVert->GetEdge(e)->m_nNextVertId;
			nOptions++;
		}
	}

	//m_uFailedEdgeCount get's incremented if no path could be found to the last point 
	//returned by this function.  So, if it isn't zero, then
	//just iterate through all the options sequentially
	//if they all get exhausted, the bot has no where to wander to I guess.
	if (nOptions > 0 && (m_uFailedEdgeCount <= nOptions))
	{
		//now sort my options and pick the
		//"best" one based on the amount I would be 
		//required to turn if I chose it.
		s16 uBestii = -1;
		if (m_uFailedEdgeCount==0)
		{
			f32 fBestDot = 0.0f;
			for (u16 ii = 0; ii < nOptions; ii++)
			{
				pV = pGraph->GetVert((u16) aiutils_anGraphVertSortBuff[ii]);

				CFVec3A VecToOption;
				VecToOption = pV->GetLocation();
				VecToOption.Sub(pMover->GetLoc());
				VecToOption.y = 0.0f;
				if (VecToOption.SafeUnitAndMag(VecToOption) > 0.0f)
				{
					CFVec3A FacingVec;
					if (m_uLastGoalVertId && m_uLastLastGoalVertId && (m_uLastGoalVertId != m_uLastLastGoalVertId))
					{
						FacingVec.Sub(pGraph->GetVert(m_uLastGoalVertId)->GetLocation(), pGraph->GetVert(m_uLastLastGoalVertId)->GetLocation());
						FacingVec.y = 0.0f;
						FacingVec.SafeUnitAndMag(FacingVec);
					}
					else
					{
						FacingVec = pMover->GetTorsoLookAtXZ();
					}
					
					f32 fThisDot = VecToOption.Dot(FacingVec);
					if (uBestii == -1 || (fBestDot < fThisDot) || (fThisDot > 0.0f && fmath_RandomChoice(100)<15)) 
					{
						fBestDot = fThisDot;
						uBestii = ii;
					}
				}
			}

			if (fBestDot < 0.0f)
			{
				int i = 2;
			}
		}
		else
		{
			uBestii = m_uFailedEdgeCount-1;
		}
		FASSERT(uBestii >=0 && uBestii < nOptions);

		//within the volume of the vert that I selected, 
		//randomize my actual goal location a little bit!
		pV = pGraph->GetVert((u16) aiutils_anGraphVertSortBuff[uBestii]);
		CFVec3A Loc;
		f32 fBoxMaxDim = pV->GetRad()*0.7f;
		fBoxMaxDim -=pMover->GetRadiusXZ();
		FMATH_CLAMP_MIN0(fBoxMaxDim);
		Loc.x = fmath_RandomBipolarUnitFloat()*fBoxMaxDim;  //cos(45deg)
		Loc.y = 1.0f;
		Loc.z = fmath_RandomBipolarUnitFloat()*fBoxMaxDim;  //cos(45deg)
		Loc.Add(pV->GetLocation());
		if (!(m_uWanderFlags & WANDER_IGNORERADIUSLIMIT) &&
			Loc.DistSqXZ(m_WanderOrigin) > m_fWanderRadius*m_fWanderRadius)
		{
			//clamp to the wander radius, within this volume
			tmp.Sub(m_WanderOrigin, pV->GetLocation());
			if (tmp.x == 0.0f && tmp.z == 0.0f)
			{
				tmp.x +=0.1f;
			}
			tmp.y = 1.0f;
			tmp.Unitize();
			tmp.Mul(fmath_RandomFloatRange(0.001f, pV->GetRad()));
			Loc.Add(tmp, pV->GetLocation());
		}
		//must find the real floor location since we're randomizing within
		//the volume
/*  commented out for now (this routine is a little funky, causing points way up in the sky
		Loc.y = aiutils_FloorHeightAt(Loc.x,
									 Loc.y,
									 Loc.z,
									 pV->m_fHeightClearance*0.95f,
									 aigraph_kfSurfaceOffset);
*/
		if (pWanderLoc)
		{
			*pWanderLoc = Loc;
		}
		return TRUE;
	}

	if (pWanderLoc)
	{
 		*pWanderLoc = pMover->GetLoc();
	}

	return FALSE;
}


BOOL CGenericWander::Find3DGraphWanderPt(CFVec3A * pWanderLoc)
{
	CAIGraph* pGraph = aimain_pGraphSearcher->GetGraph();
	CAIMover* pMover = m_pBrain->GetAIMover();
	CFVec3A tmp;
	GraphVert* pV = NULL;

	u16 nThisRoomOnly = 0;
	GraphVert* pCurVert = NULL;

	if (m_uWanderFlags & WANDER_STAYINROOM)
	{
		nThisRoomOnly = (u16) pMover->m_nRoomId;
	}

	//count verts options first
	s32 nOptions = 0;
	pCurVert = pGraph->FindClosestVert3D(pMover->GetLoc());	//Don't check LOS, but don't require us to be within the vert. Assumption is that we're pretty our closest one is actually attainable.
	if (pCurVert)
	{
		if (pCurVert->GetNumEdges())
		{
			for (u8 e = 0; e < pCurVert->GetNumEdges(); e++)
			{
				pV = pCurVert->GetOppVert(e, pGraph);

				if (!(m_uWanderFlags & WANDER_IGNORERADIUSLIMIT) && 
					pV->GetLocation().DistSq(m_WanderOrigin) > (pV->GetRad()+m_fWanderRadius)*(pV->GetRad()+m_fWanderRadius))
				{
					continue;  //only find verts who's volume intersects wander Radius Sphere
				}
				aiutils_anGraphVertSortBuff[nOptions] = pCurVert->GetEdge(e)->m_nNextVertId;
				nOptions++;
			}
		}
		else if (pCurVert->GetRad() > 5.0f*pMover->GetRadiusXZ())
		{
			aiutils_anGraphVertSortBuff[nOptions] = aimain_pAIGraph->GetVertId(pCurVert);
			nOptions++;
		}

	}

	//m_uFailedEdgeCount get's incremented if no path could be found to the last point 
	//returned by this function.  So, if it isn't zero, then
	//just iterate through all the options sequentially
	//if they all get exhausted, the bot has no where to wander to I guess.
	if (nOptions > 0 && (m_uFailedEdgeCount <= nOptions))
	{
		//now sort my options and pick the
		//"best" one based on the amount I would be 
		//required to turn if I chose it.
		s16 uBestii = -1;
		if (m_uFailedEdgeCount==0)
		{
			f32 fBestDot = 0.0f;
			for (u16 ii = 0; ii < nOptions; ii++)
			{
				pV = pGraph->GetVert((u16) aiutils_anGraphVertSortBuff[ii]);

				CFVec3A VecToOption;
				VecToOption = pV->GetLocation();
				VecToOption.Sub(pMover->GetLoc());
				if (VecToOption.SafeUnitAndMag(VecToOption) > 0.0f)
				{
					CFVec3A FacingVec;
					FacingVec = pMover->GetEntity()->MtxToWorld()->m_vFront;
					
					f32 fThisDot = VecToOption.Dot(FacingVec);
					if (uBestii == -1 || (fBestDot < fThisDot) || (fThisDot > 0.0f && fmath_RandomChoice(100)<15)) 
					{
						fBestDot = fThisDot;
						uBestii = ii;
					}
				}
			}
		}
		else
		{
			uBestii = m_uFailedEdgeCount-1;
		}
		FASSERT(uBestii >=0 && uBestii < nOptions);

		//within the volume of the vert that I selected, 
		//randomize my actual goal location a little bit!
		pV = pGraph->GetVert((u16) aiutils_anGraphVertSortBuff[uBestii]);
		CFVec3A Loc;
		f32 fBoxMaxDimXZ = pV->GetRad()*0.7f;
		fBoxMaxDimXZ -= pMover->GetRadiusXZ();
		FMATH_CLAMP_MIN0(fBoxMaxDimXZ);
		Loc.x = fmath_RandomBipolarUnitFloat()*fBoxMaxDimXZ;  //cos(45deg)
		Loc.z = fmath_RandomBipolarUnitFloat()*fBoxMaxDimXZ;  //cos(45deg)

		f32 fHeight = pV->GetHeight();
		fHeight-=pMover->GetRadiusXZ();
		FMATH_CLAMP_MIN0(fHeight);
		Loc.y = fmath_RandomBipolarUnitFloat()*fHeight;  //cos(45deg)

		Loc.Add(pV->GetLocation());
		if (!(m_uWanderFlags & WANDER_IGNORERADIUSLIMIT) &&
			Loc.DistSq(m_WanderOrigin) > m_fWanderRadius*m_fWanderRadius)
		{
			//clamp to the wander radius, within this volume
			tmp.Sub(m_WanderOrigin, pV->GetLocation());
			if (tmp.MagSq() < FMATH_POS_EPSILON)
			{
				tmp.x +=0.1f;
			}
			tmp.Unitize();
			tmp.Mul(fmath_RandomFloatRange(0.001f, pV->GetRad()));
			Loc.Add(tmp, pV->GetLocation());
		}
		if (pWanderLoc)
		{
			*pWanderLoc = Loc;
		}
		return TRUE;
	}

	if (pWanderLoc)
	{
 		*pWanderLoc = pMover->GetLoc();
	}
	return FALSE;
}


BOOL CGenericWander::FindGraphWanderPt(CFVec3A * pWanderLoc)
{
	if (m_pWanderPts)
	{
		return FindWanderPtFromPtArray(pWanderLoc);

	}
	else if (m_uWanderFlags & WANDER_3DBOT)
	{
		return Find3DGraphWanderPt(pWanderLoc);
	}
	else
	{
		return FindXZGraphWanderPt(pWanderLoc);
	}
	return FALSE;
}


void CGenericWander::FindLookAtPtWhileTakingABreak(CFVec3A* pLookAt)
{
	CAIMemoryLarge* pSeenMem = NULL;
	FASSERT(pLookAt);
	CAIMover* pMover = m_pBrain->GetAIMover();
	u8 uPerceptorsCtrl = m_pBrain->GetPerceptionCtrl();

	if ((m_pBrain->GetInfreqCycleRandom() < 0.25f) &&
		(((uPerceptorsCtrl & CAIBrain::PCFLAG_EYES_INUSE) && m_pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_SEEN, (CAIMemorySmall**) &pSeenMem)) ||
		((uPerceptorsCtrl & CAIBrain::PCFLAG_EARS_INUSE) && m_pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_HEARD, (CAIMemorySmall**) &pSeenMem)) ||
		((uPerceptorsCtrl & CAIBrain::PCFLAG_TOUCH_INUSE) && m_pBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY, (CAIMemorySmall**) &pSeenMem))))
	{
		FASSERT(pSeenMem);
		(*pLookAt).Set(pSeenMem->m_EntityLoc);
	}
	else  //random point to look at
	{
		//findfix: should randomly select from one of the visrays that has passed periodic visiblity test

		(*pLookAt).x = fmath_RandomBipolarUnitFloat();
		(*pLookAt).z = fmath_RandomBipolarUnitFloat();
		(*pLookAt).y = fmath_RandomFloat();
		(*pLookAt).Mul(10.0f);			 //max ten feet radius
		(*pLookAt).Add(pMover->GetLoc());
	}

	
}


void CGenericWander::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	CFVec3A GotoLoc;
	u16 uNowTime = aiutils_GetCurTimeSecsU16();


	if (pMover->IsSleeping())
	{
		return;
	}

	m_uWanderFlags &=~ WANDER_TURNEDAROUND_THISFRAME;
	switch (m_uStage)
	{
		case WANDERSTAGE_NEEDS_PATH:
			//request a path
			{

				if (pMover->CanMove() && FindGraphWanderPt(&m_WanderGoal))
				{
					u16 uSearchParams = CSearchQuery::SEARCHPARAM_NEED_AIPATH;
					if (m_uWanderFlags & WANDER_3DBOT)
					{
						uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
					}
					else
					{
						uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;
					}

					if (!pMover->CanJump())
					{
						uSearchParams &=~ CSearchQuery::SEARCHPARAM_NO_JUMPS;
					}

					aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
					if (m_SearchQuery.GetResults())
					{
					   m_SearchQuery.GetResults()->Clear();
					}
					m_SearchQuery.Clear();
					m_SearchQuery.SetDefaultSearchParams(m_pBrain->GetLoc(),
														 m_WanderGoal,
														pMover->GetRadiusXZ(), //w
														pMover->GetMinHeight(), //h
														 &(m_SearchResults[m_uWanderFlags & WANDER_SEARCHRESULTTOGGLE]),
														 uSearchParams,
														 (u32) m_pBrain);
					m_SearchQuery.SetMaxJumpDist(GetBrain()->GetMaxJumpDist());
					aimain_pGraphSearcher->SubmitQuery(&m_SearchQuery);
					m_uStage = WANDERSTAGE_WAITINFOR_PATH;
					pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
				}
				else
				{
					//can't find somewhere to wander to... consider taking a break.
					m_uFailedEdgeCount++;
				}
			}
				break;

		case WANDERSTAGE_WAITINFOR_PATH:
			//wait for path to be found
			{
				if (m_SearchQuery.IsDone())
				{
					m_SearchQuery.SetHasBeenDone();
					if (m_SearchQuery.GetResults()->GetAIPath())
					{
						pMover->AssignPath(m_SearchQuery.GetResults()->GetAIPath());
					}
					m_uStage = WANDERSTAGE_EN_ROUTE;
					m_uFailedEdgeCount = 0;
				}
				else if (m_SearchQuery.HasFailed())
				{
					m_uStage = WANDERSTAGE_NEEDS_PATH;  //go try to get another wander point
					m_uFailedEdgeCount++;
				}
				pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
			}
			break;
		case WANDERSTAGE_EN_ROUTE: 
			//head toward the next waypoint, checking for proximity to WanderGoal
			{
				if (pMover->FollowPath())
				{
					m_uLastLastGoalVertId = m_uLastGoalVertId;
					m_uLastGoalVertId = 0;
					if (m_pBrain->GetAIMover()->m_pPath)
					{
						m_uLastGoalVertId = m_pBrain->GetAIMover()->m_PathWalker.GetCurVertId();
					}
					//stop following the old path
					m_pBrain->GetAIMover()->AssignPath(NULL);
					m_uStage = WANDERSTAGE_NEEDS_PATH;  //go try to get another wander point
				}

				if (pMover->IsStuck(3.0f))
				{
					//stop following the old path
					m_pBrain->GetAIMover()->AssignPath(NULL);
					m_uStage = WANDERSTAGE_NEEDS_PATH;  //go try to get another wander point
				}

				if (pMover->IsSafeToStop() &&	 //make sure we're not in the middle of a jump, or some crazy move
					m_uTakeBreakDecisionTimeOut < uNowTime)
				{  // takebreak decision 
					if (fmath_RandomChoice(100) < m_uTakeBreakChance)
					{
						m_uStage = WANDERSTAGE_START_TAKING_A_BREAK;  //go try to get another wander point
					}
					else
					{
						m_uTakeBreakDecisionTimeOut = uNowTime + 20 +(u16) fmath_RandomChoice(5);	//make next break decision at this time
					}

					if (m_uSpeedPctMin || m_uSpeedPctMax )
					{
						m_pBrain->PopBaseSpeedSetting();  //findfix: need some way to guarantee that this I'm this owner is the top of the stack!
						m_pBrain->PushBaseSpeedSetting(m_uSpeedPctMin + (u8) fmath_RandomChoice(m_uSpeedPctMax-m_uSpeedPctMin));  //wander speed
					}
				
				}
			}
			break;
		case WANDERSTAGE_START_TAKING_A_BREAK:
			{
				m_uReturnToWorkTime = uNowTime + 5+(u16) fmath_RandomChoice(m_uCuriosity);
				m_uStage = WANDERSTAGE_TAKING_A_BREAK;
				GotoLoc = pMover->GetLoc();	//while taking a break, stay where you are.

				FindLookAtPtWhileTakingABreak(&m_WanderGoal);
				pMover->MoveToward(GotoLoc);	
				pMover->FaceToward(m_WanderGoal);
				m_uTakeBreakDecisionTimeOut = uNowTime + 2 +(u16) fmath_RandomChoice(2);		//make next wanderlookat decision at this time
			}
			break;
		case WANDERSTAGE_TAKING_A_BREAK:
			{
				if ( uNowTime > m_uReturnToWorkTime)
				{
					//break is over.
					m_uStage = WANDERSTAGE_NEEDS_PATH;  //go try to get another wander point
					m_uTakeBreakDecisionTimeOut = uNowTime + 20 +(u16) fmath_RandomChoice(5);	//make next break decision at this time
				}
				pMover->MoveToward(pMover->GetLoc()); //while taking a break, stay where you are.
				f32 fAlignment;
				pMover->FaceToward(m_WanderGoal, &fAlignment);
				if (fAlignment > 0.99f && m_uTakeBreakDecisionTimeOut < uNowTime)
				{
					FindLookAtPtWhileTakingABreak(&m_WanderGoal);
					m_uTakeBreakDecisionTimeOut = uNowTime + 2 +(u16) fmath_RandomChoice(2);	//make next wanderlookat decision at this time
				}


				if (pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOTJUMPER)
				{
					if (GetBrain()->GetInfreqCycleRandom() < 0.8f)
					{
						CAIHoverBotMover* pJMover = (CAIHoverBotMover*) pMover;
						if (pJMover->GetLastGraphVertId()) 
						{
							GraphVert* pV = aimain_pAIGraph->GetVert(pJMover->GetLastGraphVertId());
							pJMover->HoverTowardY(pV->m_Location.y+pV->m_fHeightClearance- pJMover->GetHeight());
						}
					}
				}

			}
			break;
	}
}

//
//  GenericPatrol
//
///////////////////////
//
// ParamPack Useage  : Large
//
//						cchar *pszPathName,				// String table Name of espline entity that contains the path points
//						u8 uPatrolSpeed,				// (0-100)	Pct of speed patrolling unit should travel at
//						u8 uDirection,					// (0,1)	0= Clockwise, 1 = CounterClockWise
//						u8 uNumIgnoreZones,				// how many elements in the next parameter
//						cchar** paszIgonoreZones)		// array of names of patrol zones to ignore
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//
// cchar *pszPathName,			// String table Name of espline entity that contains the path points
// u8 uPatrolSpeed,				// (0-100)	Pct of speed patrolling unit should travel at
// u8 uDirection,				// (0,1)	0= Clockwise, 1 = CounterClockWise
// cchar *pszZoneMarkerAName,	// String table Name of the entity whose origin marks the beginning of the custom behavior "ZONE"
// cchar *pszZoneMarkerBName,	// String table Name of the entity whose origin marks the end of the custom behavior "ZONE"
// u8 uZoneBehavior,			// NORMAL = 0, SPEED = 1, WAIT =2, ANIM = 4, ANIM_STOP_A = 8, LOOK=16, LOOK_AT_A=32, TRIGGER_AT_A=64, TRIGGER_AT_B=128
// u8 uZoneData,				// Behavior dependent	SPEED: (0-100) pct  ANIM: (0-255) Anim_BrainId
// u8 uTriggerParam,			// (0-255) A number that will be passed to the script event if behavior = TRIGGER_AT_A or TRIGGER_AT_B
// u8 uTimeData,				// how long to do some of the zone behaviors (wait, or stop, or look at)
// u32	uLookAtObjGUID);		// the GUID of something to look at while in the zone
//	---
// 22 bytes
///////////////////////
void CGenericPatrol::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	CAIMover* pMover = m_pBrain->GetAIMover();

	m_pPatrolPts = NULL;
	m_uPatrolSpeed = 0;
	m_uPatrolFlags = 0;
	m_uPatrolStage = PATROLSTAGE_OFF_PATH_NEEDS_PATH;
	m_SearchQuery.Clear();
	m_SearchResults.Clear();
	m_pPatrolPath = NULL;
	FASSERT(m_IgnoreZoneList.Size() == 0);
	m_IgnoreZoneList.SetNodePool(aimain_pNodePool);
	m_pCurSpeedZone = NULL;
	m_pCurWaitZone = NULL;
	m_pCurLookZone = NULL;
	m_pCurAnimZone = NULL;
	m_pLookAtObj = NULL;
	m_uZoneSpeed = 0;

	if (aiutils_CanEntityFly(pMover->GetEntity()))
	{
		m_uPatrolFlags |= PATROLFLAG_3DBOT;
	}

	//default values
	if (pParamPack)
	{
		cchar* pszPathName;
		u8 uDir;
		u8 uNumZoneSpecs;
		cchar** paszIgnoreZoneNames;
		ExtractPatrolParams((CAIMemoryLarge*) pParamPack,
							&pszPathName,
							&m_uPatrolSpeed,
							&uDir,
							&uNumZoneSpecs,
							&paszIgnoreZoneNames);
		
		for (u32 i = 0; i < uNumZoneSpecs; i++)
		{
			m_IgnoreZoneList.PushHead(paszIgnoreZoneNames[i]);
		}

		if (uDir)
		{
			m_uPatrolFlags |= PATROLFLAG_REVERSE;
		}

		if (m_uPatrolSpeed)
		{
			m_pBrain->PushBaseSpeedSetting(m_uPatrolSpeed);
		}

		if (pszPathName && gstring_Main.FindString(pszPathName))
		{
			CEntity* pE = CEntity::Find(pszPathName);
			if (pE && pE->TypeBits() & ENTITY_BIT_SPLINE)
			{
				m_pPatrolPts = (CESpline*) pE;
				f32 fAdd2 = 2.0f;
				fAdd2 += 2.0f;





				f32 fRadXZ = pMover->GetRadiusXZ();
				f32 fHeight = pMover->GetHeight();
				CPatrolPath::Register(m_pPatrolPts, (u8) fRadXZ, (u8) fHeight, !!(m_uPatrolFlags & PATROLFLAG_3DBOT));

				if (!m_pPatrolPts->IsClosed())
				{
					m_uPatrolFlags |= PATROLFLAG_PINGPONG;
				}
			}
			else
			{
				DEVPRINTF("AI: Can't find patrol path %s\n", pszPathName);
			}
		}
	}
}


void CGenericPatrol::Cleanup(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	CAIThought::Cleanup(); 
	//Don't leave mover following a path that this thought gave it
	if (m_pBrain &&
		m_pBrain->GetAIMover() &&
		m_pBrain->GetAIMover()->m_pPath &&
		(m_pBrain->GetAIMover()->m_pPath == m_SearchResults.GetAIPath() ||
		(m_pPatrolPath && m_pBrain->GetAIMover()->m_pPath == m_pPatrolPath->GetAIPath())))
	{
		m_pBrain->GetAIMover()->AssignPath(NULL);
	}
	aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
	m_SearchQuery.Clear();
	m_SearchResults.Clear();

	m_IgnoreZoneList.RemoveAll();

	ExitSpeedZone(NULL);
	ExitWaitZone(NULL);
	ExitLookZone(NULL);
	ExitAnimZone(NULL);

	if (m_uPatrolSpeed)
	{
		m_pBrain->PopBaseSpeedSetting();
	}
}


void CGenericPatrol::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	CFVec3A tmp;

	switch (m_uPatrolStage)
	{
		case PATROLSTAGE_OFF_PATH_NEEDS_PATH:
			{
				//find the closest patrol point to where I am now and get a path to there
				if (pMover->CanMove() && m_pPatrolPts)
				{
					s32 nNumPoints = m_pPatrolPts->PointCount();
					f32 fClosestDist2 = 0.0f;
					s32 nClosest = -1;
					for (s32 i = 0; i < nNumPoints; i++)
					{
						f32 fDist2 = m_pPatrolPts->PointArray()[i].DistSq(pMover->GetLoc());

						if (nClosest ==-1 ||   fDist2 < fClosestDist2 )
						{
							nClosest = i;
							fClosestDist2 = fDist2;
						}
					}

					if (nClosest > -1)
					{
						m_uStartPatrolPt = (u8) nClosest;

						aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
						
						if (m_SearchQuery.GetResults())
						{
						   m_SearchQuery.GetResults()->Clear();
						}
						m_SearchQuery.Clear();

						u16 uSearchParams = CSearchQuery::SEARCHPARAM_NEED_AIPATH;
						if (m_uPatrolFlags & PATROLFLAG_3DBOT)
						{
							uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
						}
						else
						{
							uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;

							// code to test some robot that cannot jumps
							//		if (m_pBrain->GetGUID()==4)
							//		{
							//			uSearchParams |= CSearchQuery::SEARCHPARAM_NO_JUMPS; //number 2 never jumps 
							//		}
						}

						if (!pMover->CanJump())
						{
							uSearchParams &=~ CSearchQuery::SEARCHPARAM_NO_JUMPS;
						}


						m_SearchQuery.SetDefaultSearchParams(pMover->GetLoc(),
															 m_pPatrolPts->PointArray()[m_uStartPatrolPt],
															 pMover->GetRadiusXZ(), //w
															 pMover->GetMinHeight(), //h
															 &m_SearchResults,
															 uSearchParams,
															 (u32) m_pBrain);
						m_SearchQuery.SetFudgeDestDist(2.0f);
						aimain_pGraphSearcher->SubmitQuery(&m_SearchQuery);
						m_uPatrolStage = PATROLSTAGE_OFF_PATH_WAITINFOR_PATH;
					}
				}
				pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
			}
			break;
		case PATROLSTAGE_OFF_PATH_WAITINFOR_PATH:
			{
				if (m_SearchQuery.IsDone())
				{
					m_SearchQuery.SetHasBeenDone();
					pMover->AssignPath(m_SearchQuery.GetResults()->GetAIPath());
					m_uPatrolStage = PATROLSTAGE_OFF_PATH_EN_ROUTE;
				}
				else if (m_SearchQuery.HasFailed())
				{
					m_uPatrolStage = PATROLSTAGE_OFF_PATH_NEEDS_PATH;  //go try to get another patrol point
				}
				pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
			}
			break;
		case PATROLSTAGE_OFF_PATH_EN_ROUTE:
			{
				if (pMover->FollowPath())
				{	//path is complete.  Start patrolling
					m_uPatrolStage = PATROLSTAGE_PATROLPATH_INIT;

					//stop following the old path
					m_pBrain->GetAIMover()->AssignPath(NULL);

					aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
					m_SearchQuery.Clear();
					m_SearchResults.Clear();
				}
			}
			break;
		case PATROLSTAGE_PATROLPATH_INIT:
			if (m_pPatrolPath = CPatrolPath::GetRegistered(m_pPatrolPts->Name()))
			{
				if (m_pPatrolPath->Failed())
				{
					//bad patrol path?
				}
				else
				{
					m_uPatrolStage = PATROLSTAGE_ON_PATH_EN_ROUTE;
					pMover->AssignPath(m_pPatrolPath->GetAIPath());
					BOOL bReverse = FALSE;
					
					//findfix: determine 
					if (m_uPatrolFlags & PATROLFLAG_REVERSE)
					{
						bReverse = TRUE;
						pMover->m_PathWalker.ChangeDirection(m_pPatrolPath->GetAIPath());
					}

					s32 nClosest = m_pPatrolPath->GetAIPath()->FindClosestWaypointNum(m_pPatrolPts->PointArray()[m_uStartPatrolPt], bReverse);
					//findfix: might need to make this closest that has los!
					if (nClosest > -1)
					{
						pMover->m_PathWalker.AdvanceTo(m_pPatrolPath->GetAIPath(), (u16) nClosest );
					}

					if (!(m_uPatrolFlags & PATROLFLAG_PINGPONG))
					{
						pMover->m_PathWalker.SetLoop(TRUE);
					}
					
//					pMover->m_PathWalker.SetReverse(1 - ((CAIBotMover*) pMover)->m_PathWalker.GetReverse());
				}
			}

			pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
			break;
		case PATROLSTAGE_ON_PATH_EN_ROUTE:
			{
				if (pMover->FollowPath())
				{	//path is complete.  Start patrolling
					pMover->m_PathWalker.ChangeDirection(m_pPatrolPath->GetAIPath());
				}
				
				if (pMover->IsStuck(3.0f))
				{
					m_uPatrolStage = PATROLSTAGE_OFF_PATH_NEEDS_PATH;
				}
			}
			break;
	}

	//
	// Zone behaviors
	//
	if (m_pCurLookZone && m_pLookAtObj)
	{
		CFVec3A LookAtLoc;
		LookAtLoc = aiutils_GetEntityLoc(m_pLookAtObj);
		//findfix: might improve this, since entity loc is objct origin, which is not necessily the best point to look at)
		pMover->FaceToward(LookAtLoc);
	}

	if (m_pCurWaitZone)
	{
		pMover->MoveToward(pMover->GetLoc());  //don't move this frame
		m_WaitZoneData.Work();				   //use this structure to pick face dir while waiting
		if (m_uWaitZoneTimeout < aiutils_GetCurTimeSecsU16())
		{
		   m_pCurWaitZone = NULL;
		}
	}
}


void CGenericPatrol::EnterSpeedZone(CPatrolZone* pZone)
{
	m_pCurSpeedZone = pZone;
	if (pZone)
	{
		m_pBrain->PushBaseSpeedSetting(pZone->GetSpeed());
	}
}

void CGenericPatrol::ExitSpeedZone(CPatrolZone* pZone)
{
	if (m_pCurSpeedZone)
	{
		m_pBrain->PopBaseSpeedSetting();
	}
	m_pCurSpeedZone = NULL;
}

void CGenericPatrol::EnterWaitZone(CPatrolZone* pZone)
{
	m_pCurWaitZone = pZone;
	m_WaitZoneData.Init(m_pBrain, THOUGHTFLAG_NONE, CAIBrain::TT_INVALID);
	m_WaitZoneData.m_uLookAroundChance = pZone->GetWaitLookAroundChance();
	m_WaitZoneData.ForceTakeLookAroundChance();
	m_uWaitZoneTimeout = aiutils_GetCurTimeSecsU16() + pZone->GetWaitTime();
}

void CGenericPatrol::ExitWaitZone(CPatrolZone* pZone)
{
	m_pCurWaitZone = NULL;
}


void CGenericPatrol::EnterLookZone(CPatrolZone* pZone)
{
	m_pCurLookZone = pZone;
	m_pLookAtObj = pZone->GetLookAtObj();
	if (!m_pLookAtObj)
	{
		m_pCurLookZone = NULL;
		DEVPRINTF("Look PAtrolZone entered, with invalid entity to look at");
	}
}

void CGenericPatrol::ExitLookZone(CPatrolZone* pZone)
{
	m_pCurLookZone = NULL;
	m_pLookAtObj = NULL;
}


void CGenericPatrol::EnterAnimZone(CPatrolZone* pZone)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	m_pCurAnimZone = pZone;
	if (!m_pBrain->IsTalking() && pZone)
	{
		BOOL bStopFirst = FALSE;
		if (pZone->IsWaitZone())
		{
			bStopFirst = TRUE;
		}

		ai_TalkModeBegin(	m_pBrain,
							AIBrain_TalkModeCB,
							(void*) m_pCurAnimZone->GetAnimZoneAnimName(),
							pMover->GetEntity(),
							1,
							bStopFirst,
							FALSE,
							0);	 //nothing particular to look at
	}
}

void CGenericPatrol::ExitAnimZone(CPatrolZone* pZone)
{
	m_pCurAnimZone = NULL;
}


void CGenericPatrol::EnterZone(CPatrolZone* pZone)
{
	if (!pZone)
		return;

	if (pZone->IsSpeedZone())
	{
		EnterSpeedZone(pZone);
	}

	if (pZone->IsAnimZone() && pZone->IsWaitZone())
	{
		EnterAnimZone(pZone);		  //anim and wait means no look and no wait
	}
	else
	{
		if (pZone->IsLookZone())
		{
			EnterLookZone(pZone);
		}
		
		if (pZone->IsWaitZone())
		{
			EnterWaitZone(pZone);
		}
		
		if (pZone->IsAnimZone())
		{
			EnterAnimZone(pZone);
		}
	}
}

void CGenericPatrol::ExitZone(CPatrolZone* pZone)
{
	if (pZone->IsSpeedZone())
	{
		ExitSpeedZone(pZone);
	}

	if (pZone->IsWaitZone())
	{
		ExitWaitZone(pZone);
	}

	if (pZone->IsAnimZone())
	{
		ExitAnimZone(pZone);
	}

	if (pZone->IsLookZone())
	{
		ExitLookZone(pZone);
	}
}

//
//
//  Ground Goto
//
//
void CGenericGoto::_ClearData(void)
{
	m_GoalLoc.Zero();
	m_OriginLoc.Zero();
	m_LookAtLoc.Zero();
	m_pLookAtObj = NULL;
	m_fPctToLookAt = 0.0f;
	m_uGotoFlags  = 0;
	m_SearchQuery.Clear();
	m_SearchResults.Clear();
	m_uStage = GOTOSTAGE_NEEDS_PATH;
	m_uFudgeDest = 0;
	m_pLookAtObj = NULL;
}

//virtual
void CGenericGoto::Init(CAIBrain* pBrain,
						u8 uThoughtFlags,
						u8 uThoughtType,
						CAIMemorySmall* pParamPack /* = NULL*/)
{
	CFVec3A GotoLoc;
	CFVec3A LookAtLoc;		// A pt in world space to aim torso and head at. (only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
	u32	uLookAtObjGUID;		// The GUID of an entity to keep looking at (0 means none).
	u8 uFudgeDest;			// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
	u8 uSpeedPct;			// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
	u16 uGotoFlags;			// from enum GOTOFLAG_
	u8 uPctToLookat;

	if (pParamPack)
	{
		ExtractGotoParams(	(CAIMemoryLarge*) pParamPack,
							&(GotoLoc),
							&(LookAtLoc),
							&(uLookAtObjGUID),
							&(uPctToLookat),		//	u8 uPctToLookat,			// (0-100) Percent to Goal that look at order will turn on  
							&(uFudgeDest),			//  u8 uFudgeDest				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
							&(uSpeedPct),			//	u8 uSpeedPct				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
							&(uGotoFlags));			//  u16 uGotoFlags				// GOTOFLAG_*
	}

	//for a gotothought and there is a desired object or pt_ws to look at.
	InitWithLookAt(	pBrain,
					uThoughtFlags,				// see AITHOUGHT::THOUGHFLAG_*
					uThoughtType,				
					GotoLoc,
					LookAtLoc,					// A pt in world space to aim torso and head at. (only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
					uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means none).
					uPctToLookat,				// pct of the way there that look at will begin
					uFudgeDest,					// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
					uSpeedPct,					// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
					uGotoFlags);				// from enum GOTOFLAG_
	if (!pParamPack)
	{
		m_uStage = GOTOSTAGE_FAILED_PATH;   //bad, just fail asap
	}

	//goto command automatically wakes up sleepers
	if (m_pBrain->GetAIMover()->IsSleeping())
	{
		m_pBrain->GetAIMover()->WakeUp();
	}

	//keep base class happy
	this->m_pParamPack = pParamPack;
}


void CGenericGoto::Init(	CAIBrain* pBrain,
							u8 uThoughtFlags,
							u8 uThoughtType,
							const CFVec3A& GotoLoc,
							u8 uFudgeDest /* = 0*/,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
							u8 uSpeedPct /* = 0*/,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
							u16 uGotoFlags /* = 0*/)			// from enum GOTOFLAG_
{
	_ClearData();

	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, NULL);
	
	m_OriginLoc = m_pBrain->GetLoc();
	
	m_GoalLoc = GotoLoc;

	m_uFudgeDest = uFudgeDest;

	if (uSpeedPct)
	{
		m_pBrain->PushBaseSpeedSetting(uSpeedPct);
		m_uGotoFlags |= GOTOFLAG_DID_SET_SPEED; //remember this sothat we can popbasespeedsetting when goto is complete
	}

	if (aiutils_CanEntityFly(pBrain->GetAIMover()->GetEntity()))
	{
		m_uGotoFlags |= GOTOFLAG_3DBOT;
	}
	
	//or in all exteranlly configureable flags
	m_uGotoFlags |= (uGotoFlags & GOTOFLAGS_ALLEXTERNAL);

	if (uGotoFlags & GOTOFLAG_USE_JOB_REACT_RULES)
	{
		m_uThoughtFlags |= THOUGHTFLAG_USE_JOB_REACT_RULES;
	}
}


void CGenericGoto::InitWithLookAt(	CAIBrain* pBrain,
									u8 uThoughtFlags,				// see AITHOUGHT::THOUGHFLAG_*
									u8 uThoughtType,
									const CFVec3A& GotoLoc,
									const CFVec3A& LookAtLoc,		// A pt in world space to aim torso and head at. (only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
									u32	uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means none).
									u8 uLookAtPct/* = 0*/,				// pct of the way there that look at will begin
									u8 uFudgeDest/* = 0*/,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
									u8 uSpeedPct/* = 0*/,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
									u16 uGotoFlags/* = 0*/)				// from enum GOTOFLAG_
{
	Init(pBrain, uThoughtFlags, uThoughtType, GotoLoc, uFudgeDest, uSpeedPct, uGotoFlags);

	if (uLookAtObjGUID)
	{
		m_uGotoFlags |= GOTOFLAG_LOOKAT;

		m_pLookAtObj = CEntity::Find(uLookAtObjGUID);
		if (!m_pLookAtObj)
		{	//couldn't find the entity I was told to look at while waiting
			m_uGotoFlags &= ~GOTOFLAG_LOOKAT;
		}
	}

	m_fPctToLookAt = uLookAtPct*kfOOHun;
}


void CGenericGoto::Cleanup()
{
	CAIThought::Cleanup();

	if (m_pBrain && m_pBrain->GetAIMover())
	{
		m_pBrain->GetAIMover()->AssignPath(NULL);
	}
	aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
	m_SearchQuery.Clear();
	m_SearchResults.Clear();

	if (m_uGotoFlags & GOTOFLAG_DID_SET_SPEED)
	{
		m_pBrain->PopBaseSpeedSetting();
	}
	
	if (m_uGotoFlags & GOTOFLAG_POWERDOWN_AT_END) 
	{
		if (GetBrain()->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
		{
			CBot* pBot = (CBot*) GetBrain()->GetAIMover()->GetEntity();
			if (pBot->IsInWorld() && !pBot->IsMarkedForWorldRemove() && !pBot->IsDeadOrDying())
			{
				pBot->Power(FALSE);
			}
		}
	}

	if (this->m_uGotoFlags & GOTOFLAG_RETURN_FIRE_OK &&
		GetBrain()->GetCurThought() &&
		GetBrain()->GetCurThought() == CAIBrain::TT_COMBAT &&
		GetBrain()->GetCurThoughtPtr() &&
		GetBrain()->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT))
	{
		//this goto must be the kind that is a reaction and 
		CGroundCombat* pT = (CGroundCombat*) GetBrain()->GetCurThoughtPtr();
		pT->m_uAttackFlags &= ~CGroundCombat::FLAG_COMBAT_CANT_MOVE_BOT;
		GetBrain()->GetCurThoughtPtr()->Work();
	}


	_ClearData(); //what the heck.
}


void CGenericGoto::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	switch (m_uStage)
	{
		case GOTOSTAGE_NEEDS_PATH:
			//request a path
			{
				if (pMover->CanMove())
				{
					u16 uSearchParams = CSearchQuery::SEARCHPARAM_NEED_AIPATH;

					if (m_uGotoFlags & GOTOFLAG_3DBOT)
					{
						uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
					}
					else
					{
						uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;
					}

					if (!pMover->CanJump())
					{
						uSearchParams &=~ CSearchQuery::SEARCHPARAM_NO_JUMPS;
					}

					aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
					if (m_SearchQuery.GetResults())
					{
						m_SearchQuery.GetResults()->Clear();
					}
					m_SearchQuery.Clear();
					m_SearchQuery.SetDefaultSearchParams(	pMover->GetLoc(),  
															m_GoalLoc,  
															pMover->GetRadiusXZ(), //w
															pMover->GetMinHeight(), //h
															&m_SearchResults,
															uSearchParams,
															(u32) m_pBrain);

					if (m_uFudgeDest)
					{
						m_SearchQuery.SetFudgeDestDist(m_uFudgeDest);
					}
					m_SearchQuery.SetMaxJumpDist(GetBrain()->GetMaxJumpDist());

					aimain_pGraphSearcher->SubmitQuery(&m_SearchQuery);
					m_uStage = GOTOSTAGE_WAITINFOR_PATH;
				}
				else
				{
					m_uStage = GOTOSTAGE_FAILED_PATH;
				}
			}
			break;

		case GOTOSTAGE_WAITINFOR_PATH:
			//wait for path to be found
			{
				if (m_SearchQuery.IsDone())
				{
					m_SearchQuery.SetHasBeenDone();

					pMover->AssignPath(m_SearchQuery.GetResults()->GetAIPath(), CAIMover::ASSIGNPATHFLAG_CANROLL*(!!(m_uGotoFlags & GOTOFLAG_FORCE_ROLL)));
					m_uStage = GOTOSTAGE_EN_ROUTE;
				}
				else if (m_SearchQuery.HasFailed())
				{
					m_uStage = GOTOSTAGE_FAILED_PATH;
				}
				pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
			}
			break;
		case GOTOSTAGE_EN_ROUTE: //head toward the next waypoint, checking for proximity to WanderGoal
			if (pMover->FollowPath())
			{
				//stop following the old path
				m_pBrain->GetAIMover()->AssignPath(NULL);
				m_uStage = GOTOSTAGE_CLOSEENOUGH;  //go try to get another patrol point
			}
			else if (	pMover->IsStuck(2.0f) &&
						pMover->m_nClientFeedback & (CAIMover::FEEDBACK_PATH_OBSTRUCTED | CAIMover::FEEDBACK_PATH_SLOWPROGRESS))
			{
				pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
				m_uStage = GOTOSTAGE_NEEDS_PATH;
			}
			break;
		case GOTOSTAGE_FAILED_PATH:
			{
				if (m_uGotoFlags & GOTOFLAG_BLIND_OK)
				{
					m_uStage = GOTOSTAGE_BLIND_GOTO;
					pMover->MoveToward(m_GoalLoc);	//while waiting, stay where you are.
				}
				else
				{
					m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;	//This thought goal can be considered complete!!!!
					if (m_uGotoFlags & GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END)
					{
						CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("ai"), AI_EVENTDATA1_GOAL_COMPLETE, (u32)(this->m_pBrain->GetAIMover()->GetEntity()), FALSE);
					}
					pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.
				}
			}
			break;
		case GOTOSTAGE_CLOSEENOUGH:
			{
				pMover->MoveToward(pMover->GetLoc());	//while waiting, stay where you are.

				if (!(m_uGotoFlags & GOTOFLAG_LOOKAT) || (m_uGotoFlags & GOTOFLAG_DID_LOOKAT))
				{

					if (m_uGotoFlags & GOTOFLAG_VICT_BTA_AT_END)
					{
						if (!GetBrain()->IsTalking())
						{
 							cchar* pszBTAName = AIBTA_EventToBTAName("P*E_VICT", GetBrain());
							if (pszBTAName)
							{
								ai_TalkModeBegin(GetBrain(),
												AIBrain_TalkModeCB,
												(void*) pszBTAName,
												GetBrain()->GetAIMover()->GetEntity(),
												1,
												FALSE, //DON"T STOP
												FALSE, //DON"T LOOK
												0);
							}
						}

					}

					m_uThoughtFlags |= THOUGHTFLAG_GOAL_COMPLETE;	//This thought goal can be considered complete!!!!
					
					if (m_uGotoFlags & GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END)
					{
						CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("ai"), AI_EVENTDATA1_GOAL_COMPLETE, (u32)(this->m_pBrain->GetAIMover()->GetEntity()), TRUE);
					}
				}
			}
			break;
		case GOTOSTAGE_BLIND_GOTO:
			{
				pMover->MoveToward(m_GoalLoc);
				if (pMover->GetLoc().DistSq(m_GoalLoc) < 3.0f)
				{
					m_uStage = GOTOSTAGE_CLOSEENOUGH;  //go try to get another patrol point
				}
				
				if (pMover->IsStuck(2.0f) &&
					pMover->m_nClientFeedback & (CAIMover::FEEDBACK_PATH_OBSTRUCTED))
				{
					m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;	//This thought goal can be considered complete!!!!
					if (m_uGotoFlags & GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END)
					{
						CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("ai"), AI_EVENTDATA1_GOAL_COMPLETE, (u32)(this->m_pBrain->GetAIMover()->GetEntity()), FALSE);
					}
				}
			}

			break;
	}



	if (this->m_uGotoFlags & GOTOFLAG_RETURN_FIRE_OK &&
		GetBrain()->GetCurThought() &&
		GetBrain()->GetCurThought() == CAIBrain::TT_COMBAT &&
		GetBrain()->GetCurThoughtPtr() &&
		GetBrain()->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT))
	{
		//this goto must be the kind that is a reaction and 
		CGroundCombat* pT = (CGroundCombat*) GetBrain()->GetCurThoughtPtr();
		pT->m_uAttackFlags |= CGroundCombat::FLAG_COMBAT_CANT_MOVE_BOT;

		GetBrain()->GetCurThoughtPtr()->Work();
		m_uGotoFlags |= GOTOFLAG_DID_LOOKAT;
	}
	else
	{
		//control heading
		if (m_uGotoFlags & GOTOFLAG_LOOKAT)
		{
			if (m_uGotoFlags & GOTOFLAG_DID_LOOKAT)
			{  //keep on looking.
				f32 fAlignment = 1.0f;
				if (m_pLookAtObj)
				{
					if (pMover->FaceToward(aiutils_GetEntityLoc(m_pLookAtObj), &fAlignment) || fAlignment > 0.95f)
					{
						m_uGotoFlags |= GOTOFLAG_DID_LOOKAT;
					}
				}
				else
				{  //must have specified some specific location to look at
					if (pMover->FaceToward(m_LookAtLoc, &fAlignment)  ||  fAlignment > 0.95f)
					{
						m_uGotoFlags |= GOTOFLAG_DID_LOOKAT;
					}
				}
			}
			else
			{   //Is it time to look yet?
				f32 fFullDist = m_OriginLoc.Dist(m_GoalLoc);
				if (fFullDist > 0.0f)
				{
					f32 fDistSoFar = m_OriginLoc.Dist(pMover->GetLoc());
					if ((m_uStage == GOTOSTAGE_CLOSEENOUGH) ||
						fDistSoFar/fFullDist > (f32) m_fPctToLookAt)
					{
						f32 fAlignment = 1.0f;
						if (m_pLookAtObj)
						{
							if (pMover->FaceToward(aiutils_GetEntityLoc(m_pLookAtObj), &fAlignment) || fAlignment > 0.95f)
							{
								m_uGotoFlags |= GOTOFLAG_DID_LOOKAT;
							}
						}
						else
						{  //must have specified some specific location to look at
							if (pMover->FaceToward(m_LookAtLoc, &fAlignment)  ||  fAlignment > 0.95f)
							{
								m_uGotoFlags |= GOTOFLAG_DID_LOOKAT;
							}
						}
					}
				}
				else if (m_uStage == GOTOSTAGE_CLOSEENOUGH)
				{
					m_uGotoFlags &= ~GOTOFLAG_DID_LOOKAT;   //illegal look at pct
				}
			}
		}
	}
}


//
//
//  Generic FaceIt
//
//
//
void CGenericFaceIt::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /* = NULL*/)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	m_pLookAtObj = NULL;
	m_uTimeOut = 0;
	m_uFaceItFlags = 0;
	FASSERT(pParamPack->m_uMemoryId == MEMORY_LARGE_PARAMPACK);

	u32   uLookAtObjGUID;			// The GUID of an entity to keep looking at (0 means ignore)
	u8   uTimer;
	ExtractFaceItParams((CAIMemoryLarge*) pParamPack,
						&(m_LookAtLoc),	// A position to look at (Ignored if LookAtObjGUID > 0)	
						&uLookAtObjGUID,	// The GUID of an entity to keep looking at (0 means ignore)
						&m_uTrack,
						&uTimer);
	if (uTimer)
	{
		m_uFaceItFlags |= FACEIT_HAS_TIMEOUT;
		m_uTimeOut = uTimer;  //don't add aiutils_GetCurTimeSecsU16()+ It gets added when alighnent is first reached
	}

	if (uLookAtObjGUID)
	{
		m_pLookAtObj = CEntity::Find(uLookAtObjGUID);
		if (!m_pLookAtObj)
		{
			m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;	 //couldn't find that guy!
		}
	}
}


void CGenericFaceIt::Cleanup(void)
{
//	CAIThought::Cleanup();

}


void CGenericFaceIt::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	CFVec3A tmp;
	f32 fTorsoAlign = 1.0f;

	if (m_uTimeOut > 0 &&
		(m_uFaceItFlags & FACEIT_DID_ALIGN) &&
		m_uTimeOut < aiutils_GetCurTimeSecsU16())
	{
		m_uThoughtFlags |= THOUGHTFLAG_GOAL_COMPLETE;
	}

	if (m_pLookAtObj)
	{
		if (!m_pLookAtObj->IsInWorld())
		{
			if (m_uFaceItFlags & FACEIT_DID_ALIGN)
			{
				m_uThoughtFlags |= THOUGHTFLAG_GOAL_COMPLETE;
			}
			else
			{
				m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;
			}
		}
		else
		{
//			pMover->m_Controls.SetFlag_Slam_RotateCW_Average();
			if (pMover->FaceToward(*(m_pLookAtObj->GetTagPoint(0)), &fTorsoAlign) || fTorsoAlign > 0.985)
			{
				if (m_uFaceItFlags & FACEIT_DID_ALIGN)
				{	//check if we weren't supposed to track that much
					if (fTorsoAlign < 0.90f)
					{
						f32 fCosTheta = m_LookAtLoc.Dot(pMover->GetEyeLookAt());
						f32 fMaxTheta = (f32)(100-m_uTrack)*kfOOHun;
						if (fCosTheta < fMaxTheta)
						{
							m_uThoughtFlags |= THOUGHTFLAG_GOAL_COMPLETE;
						}
					}
				}
				else
				{
					m_LookAtLoc = *(m_pLookAtObj->GetTagPoint(0));	 //record my lookat the first time I cought up with him
					m_uFaceItFlags |= FACEIT_DID_ALIGN;
					if (m_uTimeOut)
					{
						m_uTimeOut += aiutils_GetCurTimeSecsU16();	//start the timer
					}
				}
			}

		}
	}
	else
	{
		if (pMover->FaceToward(m_LookAtLoc, &fTorsoAlign) || fTorsoAlign > 0.98)
		{
			if (!(m_uFaceItFlags & FACEIT_DID_ALIGN))
			{
				m_uFaceItFlags |= FACEIT_DID_ALIGN;
				if (m_uTimeOut)
				{
					m_uTimeOut += aiutils_GetCurTimeSecsU16();	//start the timer
				}
			}
		}
	}
}


//
//
//  Generic TalkTo
//
//
//
void CGenericTalkTo::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /* = NULL*/)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	m_pLookAtObj = NULL;
	m_uTimeOut = 0;
	m_uTalkToFlags = 0;
	FASSERT(pParamPack->m_uMemoryId == MEMORY_LARGE_PARAMPACK);

	u32 uLookAtObjGUID;			// The GUID of an entity to keep looking at (0 means ignore)
	u8 uTimer;
	BOOL bStopFirst;
	ExtractTalkToParams((CAIMemoryLarge*) pParamPack,
						&uLookAtObjGUID,	// The GUID of an entity to face before and while talking (0 means no face)
						&uTimer,			// time out after this many secs
						&bStopFirst,		// stop before talking
						&m_pTalkModeFunc,
						&m_pvData1,
						&m_pvData2);
	if (uTimer)
	{
		m_uTalkToFlags |= TALKTO_HAS_TIMEOUT;
		m_uTimeOut = aiutils_GetCurTimeSecsU16()+uTimer;
	}

	if (bStopFirst)
	{
		m_uTalkToFlags |= TALKTO_MUST_STOP;
	}

	if (uLookAtObjGUID)
	{
		m_uTalkToFlags |= TALKTO_MUST_FACE;
		m_pLookAtObj = CEntity::Find(uLookAtObjGUID);
		if (!m_pLookAtObj)
		{
			m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;	 //couldn't find that guy!
		}
	}
}


void CGenericTalkTo::Cleanup(void)
{
//	CAIThought::Cleanup();

	if (m_pTalkModeFunc)
	{
		((TalkModeCallback*) m_pTalkModeFunc)(TMCB_DONE, m_pvData1, m_pvData2);
	}
	if ((m_pBrain->GetAIMover()->IsTalkingWithAnim() ||m_pBrain->GetAIMover()->IsTalkingWithOnlyAudio()) &&
		m_pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
	{
		CBot* pBot = (CBot* ) m_pBrain->GetAIMover()->GetEntity();
		if (pBot->m_pActiveTalkInst)
		{
			CTalkSystem2::TerminateActiveTalk(pBot->m_pActiveTalkInst);
			pBot->m_pActiveTalkInst = NULL;
		}
	}
}


void CGenericTalkTo::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	CFVec3A tmp;

	if (m_uTimeOut > 0 &&
		!(m_uTalkToFlags & TALKTO_DID_TALK) &&
		m_uTimeOut < aiutils_GetCurTimeSecsU16())
	{
		m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;
		//ignored
	}

	if (pMover->CanTalkNow(m_uTalkToFlags & TALKTO_MUST_STOP))
	{
		if (m_pLookAtObj)
		{
			if (!CEntity::Find(m_pLookAtObj->Guid()))	 
			{	//can't find the thing I'm supposed to face
				m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;
			}
			else
			{	//try to face a thing
				f32 fTorsoAlign = 1.0f;
				pMover->FaceToward(aiutils_GetEntityLoc(m_pLookAtObj), &fTorsoAlign);
				if (fTorsoAlign > 0.95f)
				{
					m_uTalkToFlags |= TALKTO_DID_FACE;
				}
			}
		}

		if (m_uTalkToFlags & TALKTO_MUST_STOP)
		{
			pMover->MoveToward(pMover->GetLoc());
		}

		if (pMover->m_nClientFeedback & CAIMover::FEEDBACK_STOPPED)
		{
			m_uTalkToFlags |= TALKTO_DID_STOP;
		}

		if (!(m_uTalkToFlags & TALKTO_DID_TALK) &&
			(!(m_uTalkToFlags & TALKTO_MUST_STOP) || (m_uTalkToFlags & TALKTO_DID_STOP)) &&
			(!(m_uTalkToFlags & TALKTO_MUST_FACE) || (m_uTalkToFlags & TALKTO_DID_FACE)))
		{
			if (m_pTalkModeFunc)
			{
				if (! ((TalkModeCallback*) m_pTalkModeFunc)(TMCB_READY, m_pvData1, m_pvData2) )
				{  //if the talkmodecallback returns false to a TMCB_Ready, it means failure
					m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;
				}
				else
				{	//GO
				}
			}
			m_uTalkToFlags |=TALKTO_DID_TALK;
		}
	}
	else
	{
		if ((m_uTalkToFlags & TALKTO_DID_TALK) && (m_uTalkToFlags & TALKTO_MUST_STOP)) 
		{
	  		pMover->MoveToward(pMover->GetLoc());
		}
	}
}




//
// Follow
//
///////////////////////
//
// ParamPack
//
//	  - Large
///////////////////////
void CGenericFollow::Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack)
{
	CAIThought::Init(pBrain, uThoughtFlags, uThoughtType, pParamPack);
	m_uFollowFlags = FOLLOWFLAG_NONE;
	m_LastGoodPt_WS.Zero();

	m_uLastLookAtLeaderTime = 0;
	m_uLookAtLeaderTimeOut = 0;
	m_uLastLookAheadTime = 0;
	m_uLookAheadTimeOut = 0;
	m_uNextLookAheadTimeOut = 0;
	m_uNextLookAtLeaderTimeOut = 0;
	m_LastMoveToLoc = pBrain->GetAIMover()->GetLoc();
	m_uStuckStage = STUCKSTAGE_NOTSTUCK;
	m_uStuckPathRetryTimeOut = 0;
	m_uPathFailureCount = 0;
	m_uFormationPostPathFailureCount = 0;
	m_pLookAtMgr = NULL;
	m_uLeaderKills = 0;
}


void CGenericFollow::Cleanup(void)
{
	CAIThought::Cleanup();

	if (m_pBrain &&
		m_pBrain->GetAIMover() &&
		m_pBrain->GetAIMover()->m_pPath &&
		(m_pBrain->GetAIMover()->m_pPath == m_aSearchResults[0].GetAIPath() || 
		m_pBrain->GetAIMover()->m_pPath == m_aSearchResults[1].GetAIPath()))
	{
		m_pBrain->GetAIMover()->AssignPath(NULL);
	}


	if (m_pLookAtMgr)
	{
		CGenericWait::BankAccess(m_pLookAtMgr);
		m_pLookAtMgr = NULL;
	}

	aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
	m_SearchQuery.Clear();
	m_aSearchResults[0].Clear();
	m_aSearchResults[1].Clear();
}


void CGenericFollow::_DoSearchWork(void)
{
}


BOOL CGenericFollow::_IsMoverOnFollowPath(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	return (pMover->IsFollowingPath() &&
			(pMover->m_pPath == m_aSearchResults[0].GetAIPath() ||
			pMover->m_pPath == m_aSearchResults[1].GetAIPath()));

}

static f32 _fMotiTimer = 0.0f;
static u32  _uMotiTimerFVid_nFrameCounter = 0;

void CGenericFollow::Work(void)
{
	CAIMover* pMover = m_pBrain->GetAIMover();
	CBot* pBot = NULL;
	CAIBrain* pLeaderBrain = m_pBrain->GetLeader();
	CAIMover* pLeaderMover = NULL;
	CBot* pLeaderBot = NULL;
	CGroundCombat* pT =NULL;
	cchar* pszBTAName = NULL;

	u32 uLastLeaderKills = 0;


	if (_uMotiTimerFVid_nFrameCounter != FVid_nFrameCounter &&
		_fMotiTimer > 0.0f)
	{
		if (FVid_nFrameCounter < 1)
		{
			_fMotiTimer -= FLoop_fPreviousLoopSecs;
		}
		else
		{
			_fMotiTimer -= FLoop_fPreviousLoopSecs;
			_uMotiTimerFVid_nFrameCounter = FVid_nFrameCounter;
		}
	}


	if (pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
	{
		pBot = (CBot*) pMover->GetEntity(); 
	}

	if (pLeaderBrain)
	{
		pLeaderMover = pLeaderBrain->GetAIMover();
		if (pLeaderMover && pLeaderMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT)
		{
			pLeaderBot = (CBot*) pLeaderMover->GetEntity(); 


			if (pLeaderBot->IsPlayerBot())
			{
				uLastLeaderKills = m_uLeaderKills;
				if (MultiplayerMgr.IsSinglePlayer())
				{
					u32 uTmp;
					Player_aPlayer[pLeaderBot->m_nPossessionPlayerIndex].GetSPStats(m_uLeaderKills, uTmp);
				}
				else
				{
					s8 nKills = 0;
					u8 uDeats = 0;
					Player_aPlayer[pLeaderBot->m_nPossessionPlayerIndex].GetMPScore(nKills, uDeats);
					if (nKills >=0)
					{
						m_uLeaderKills = nKills;
					}
				}
			}
		}

	}

	if (!pLeaderBrain || !pLeaderBot || pLeaderBot->IsDeadOrDying() || !pLeaderBot->IsInWorld())
	{
		m_uThoughtFlags |= THOUGHTFLAG_GOAL_FAILED;	 //couldn't find that guy!
	}

	m_pBrain->ClearFlag_Follower_Doesnt_Need_Post();

	//	 !!!!!
	//	 if current thought is a goal, it overrides follow commands.
	//

	if ((m_uFollowFlags & FOLLOWFLAG_HADGOAL) &&
		m_pBrain->GetCurThoughtPtr() &&
		CAIBrain::IsGoal(m_pBrain->GetCurThought()))
	{
		//even though leader has followers, they aren't trying to follow right now.
		m_pBrain->SetFlag_Follower_Doesnt_Need_Post();
		m_pBrain->SetBaseSpeed(m_pBrain->GetAttrib(CAIBrain::ATTRIB_SPEED));

		if (pBot->TypeBits() & ENTITY_BIT_BOTMINER &&
			!GetBrain()->GetFlag_Buddy_Ctrl_NoWeaponStow())
		{
			CBotMiner* pMiner = (CBotMiner*) pBot;

			if (!pMiner->IsPrimaryWeaponInitial() && !pMiner->SwitchingWeapons())
			{
				pMiner->SwitchPrimaryWeapon(TRUE);
			}
		}

		//
		//	if 				
		//      on attack &&
		//		can't remember being damaged &&
		//		leader can't remember damaging anybody
		//
		//	then
		//		attack in place
		//	else
		//		attack has full control of bot's positioning and everything.
		//
		if (m_pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGROUNDCOMBAT))
		{
			if (pBot && pBot->GetDisguisedBot() != pLeaderBot)
			{
				pT = (CGroundCombat*) m_pBrain->GetCurThoughtPtr();

				if (!GetBrain()->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGED_BY) &&
					!pLeaderBrain->GetKnowledge().CanRememberAny(MEMORY_LARGE_DAMAGE_DEALT))
				{

					if (!(pT->m_uAttackFlags & CGroundCombat::FLAG_COMBAT_CANT_MOVE_BOT))
					{   
						
						for (u32 i = 0; i < CGroundCombat::NUM_PATH_REQUEST_SLOTS; i++)
						{
							pT->FreeRequestSlot(i);
						}
						pT->m_uAttackFlags |= CGroundCombat::FLAG_COMBAT_CANT_MOVE_BOT;
					}
				}
				else
				{
					pT->m_uAttackFlags &= ~CGroundCombat::FLAG_COMBAT_CANT_MOVE_BOT;

				}
			}
		}

		//Note! curthought might be a goto, or an attack
		m_pBrain->GetCurThoughtPtr()->Work();


		// if 
		//		leader does something good and 
		//		curthought is an attack and that attack has free reign,
		// then
		//		follow command has nothing else to do.
		if (pLeaderBot &&
			!(pT && (pT->m_uAttackFlags & CGroundCombat::FLAG_COMBAT_CANT_MOVE_BOT)))
		{
			if (uLastLeaderKills != m_uLeaderKills)
			{
				if (pszBTAName = AIBTA_EventToBTAName("P*F_BOSS", GetBrain()))
				{
					if (ai_TalkModeBegin(GetBrain(),
										AIBrain_TalkModeCB,
										(void*) pszBTAName,
										pMover->GetEntity(),
										1,
										FALSE,  //Stop first if I have no enemy
										(!pT || !pT->m_pEnemy),  //LOOK AT  look at leader if I have no enemy
										pLeaderBot->Guid()))
					{
					}
				}
			}
			return;	
		}
	}

	//
	//   some book keeping
	//
	if (pBot->m_fSpeed_WS > 0.01f)
	{
		m_fTimeSinceMoved -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN(m_fTimeSinceMoved, 0.0f);
	}
	else
	{
		m_fTimeSinceMoved += FLoop_fPreviousLoopSecs;
	}

	BOOL bMoveToLocIsFormationPost = FALSE;
	f32 fNoNewPathRange = 0.0f;					//if within this range of the move to loc, don't request a new path
	f32 fCloseEnoughOverride = 0.0f;			//if this close to the leader, then stop trying to get closer
	CFVec3A MoveToLoc = pMover->GetLoc();		//where the follower wants to be this frame
	BOOL bSuspicious = FALSE;
	if (pBot && pBot->GetDisguisedBot() == pLeaderBot)
	{
		bSuspicious = TRUE;
	}

	if (bSuspicious)
	{
        // Following this bot because I'm suspcious of it.  So, try to get in his face.
		// Don't use a his formation

/*		f32 fLeaderDistSq = pLeader->GetLoc().DistSq(pMover->GetLoc());
		f32 fMinDistSq = 10.0f + pLeader->GetAIMover()->GetRadiusXZ()+pMover->GetRadiusXZ();
		fMinDistSq*=fMinDistSq;
		if (fLeaderDistSq > fMinDistSq)
*/			{
			MoveToLoc = pLeaderBot->MtxToWorld()->m_vFront;
			MoveToLoc.y = 0.0f;
			if (MoveToLoc.SafeUnitAndMag(MoveToLoc) > 0.0f)
			{
				MoveToLoc.Mul(pMover->GetRadiusXZ() + 5.0f + pLeaderMover->GetRadiusXZ());
				MoveToLoc.Add(pLeaderMover->GetLoc());
			}
			else
			{
				MoveToLoc = pLeaderMover->GetLoc();
			}
		}
	}
	else
	{
		if (!GetBrain()->CanFollow(GetBrain()->GetLeader()))
		{
			GetBrain()->StopFollowing();
		}
		//
		// Normal follow.  Every follower of every brain gets a post.  
		// Check the flags on that post for specifics about how to follow leader.
		//
		CAIFormation* pFormation = pLeaderBrain->GetFormation();
		CAIPost* pMyPost = m_pBrain->GetFollowerPost();
		BOOL bUseFormation = FALSE;
		
		if ((aiutils_GetCurTimeSecs() - pLeaderBrain->GetLastXYZChangeTime() > 3.0f+5.0f*pLeaderBrain->GetInfreqCycleRandom()))
		{
			u32 uHotFollower = (u32) (pLeaderBrain->GetNumFollowers()*pLeaderBrain->GetInfreqCycleRandom());
			if (uHotFollower == m_pBrain->GetFollowerRank())
			{
				bUseFormation = TRUE;
			}
		}
		

		//pgm removed this whole formation thing since it really wasn't too cool and nobody really cares about buddies walking in formation with you.
		if (bUseFormation &&  
			pFormation &&
			pMyPost &&
			pMyPost->m_uPostFlags & CAIPost::POSTFLAG_FORMATIONSLOT &&
			m_uPathFailureCount < 5)
		{
			//we have a post in the formation, try to get there.
			MoveToLoc = pMyPost->m_Pos_WS;
			bMoveToLocIsFormationPost = TRUE;

			if (m_uFormationPostPathFailureCount > 3)
			{	//override the close enough dist whenever we have been failing to get  path to our post
				fCloseEnoughOverride = m_pBrain->GetFollowerRank()*pLeaderMover->GetRadiusXZ() + 20.0f + pLeaderMover->GetRadiusXZ() + pMover->GetRadiusXZ()*2.0f;
			}
		}
		else
		{
			//No post, so request a path directly to the leader, 
			//and stop when within stop fCloseEnoughOverride and have los of him
			MoveToLoc = pLeaderBot->MtxToWorld()->m_vPos;

			fCloseEnoughOverride = m_pBrain->GetFollowerRank()*pLeaderMover->GetRadiusXZ() + 20.0f + pLeaderMover->GetRadiusXZ() + pMover->GetRadiusXZ()*2.0f;

			//if
			//		leader isn't moving very fast,
			//then
			//		don't ask for path to a new position if within this distance of current moveto location
			//		and not already moving
			if (pBot->m_fSpeed_WS < 1.0f)
			{
				fNoNewPathRange = pLeaderMover->GetRadiusXZ()+pMover->GetRadiusXZ()+25.0f;
				if (pLeaderBot->GetCurMech())
				{
					fNoNewPathRange += pLeaderBot->GetCurMech()->m_fCollCylinderRadius_WS;
				}
			}


		}



		//speed control
		f32 fIdealNormSpeed = 1.0f;
		if (pBot->m_fMaxFlatSurfaceSpeed_WS > 0.0f)
		{
			fIdealNormSpeed = fmath_Div(pLeaderBot->m_fSpeedXZ_WS, pBot->m_fMaxFlatSurfaceSpeed_WS);
		}
		
		f32 fDistToMoveToLoc = MoveToLoc.Dist(pMover->GetLoc());
		f32 fDistToLeader = pLeaderMover->GetLoc().Dist(pMover->GetLoc());
		if ( fDistToMoveToLoc < pMover->GetRadiusXZ()*2.0f)
		{   //try to move no faster than my leader if I'm doing a decent job of keeping up
			pMover->SetForwardGoalSpeed(pLeaderBot->m_fSpeedXZ_WS);
		}
		else
		{
			pMover->SetForwardGoalSpeedPctCapable(1.0f);
		}
		m_LastMoveToLoc = MoveToLoc;

		//
		// special code for miners to make them stow their weapon
		//
		if (pBot->TypeBits() & ENTITY_BIT_BOTMINER && 
			!GetBrain()->GetFlag_Buddy_Ctrl_NoWeaponStow() &&
			(m_pBrain->GetGUID() &1)== (aiutils_GetCurTimeSecsU16()&1) &&
			!pLeaderBot->IsThrowing())
		{
			CBotMiner* pMiner = (CBotMiner*) pBot;

			BOOL bLeaderIsArmed = aiutils_IsArmed(pLeaderBot);
			if (bLeaderIsArmed && !pMiner->IsPrimaryWeaponInitial() && !pMiner->SwitchingWeapons())
			{
				pMiner->SwitchPrimaryWeapon(TRUE);
			}
			else if (!bLeaderIsArmed && !pMiner->SwitchingWeapons() && pMiner->IsPrimaryWeaponInitial())
			{
				pMiner->SwitchPrimaryWeapon(FALSE);
			}
		}
	}

	//
	// Pump the follow paths
	//
	if (m_SearchQuery.IsDone() && !m_SearchQuery.HasBeenDone())	 //Done means successfull path complete!
	{
		m_SearchQuery.SetHasBeenDone();
	}

	if (m_SearchQuery.HasFailed())
	{
		if (m_uFollowFlags & FOLLOWFLAG_FORMATION_POST_PATH)
		{
			m_uFormationPostPathFailureCount++;
		}

		m_uPathFailureCount++;
		if( m_SearchQuery.GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_END )
		{
			
		}
		if( m_SearchQuery.GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_START )
		{
			//stuck!!!
			m_uStuckStage = STUCKSTAGE_STUCK_WAITING;
		}

		m_SearchQuery.Clear();
	}

	// if a path is found that takes me closer to the goal
	// than I am now, or than my current path will take me, 
	// then switch to that path.
	if (m_SearchQuery.IsDone())
	{
		f32 fDistanceOfCurrentGoalSq = -1.0f;
		CFVec3A EndOPath;

		if (m_uFollowFlags & FOLLOWFLAG_FORMATION_POST_PATH)
		{
			m_uFormationPostPathFailureCount = 0;
		}
		m_uFollowFlags &= ~FOLLOWFLAG_FORMATION_POST_PATH;
		m_uPathFailureCount = 0;;

		if (_IsMoverOnFollowPath() &&
			pMover->m_pPath &&
			pMover->m_pPath->GetEndOfPath(&EndOPath))
		{
			fDistanceOfCurrentGoalSq = EndOPath.DistSq(MoveToLoc);
		}
		else 
		{
			if (m_LastGoodPt_WS == CFVec3A::m_Null)
			{
				fDistanceOfCurrentGoalSq = FMATH_MAX_FLOAT;
			}
			else
			{
				fDistanceOfCurrentGoalSq = m_LastGoodPt_WS.DistSq(MoveToLoc);
			}
		}

		if (m_SearchQuery.GetGoalLoc().DistSq(MoveToLoc) < fDistanceOfCurrentGoalSq)
		{
			pMover->AssignPath(m_SearchQuery.GetResults()->GetAIPath());
		}
		else
		{
		   //throw the path away.
		}
		m_SearchQuery.Clear();
	}

	//
	// every so often, if the goal has moved, then start a new path
	//
	f32 fDistToMoveToLoc = pMover->GetLoc().Dist(MoveToLoc);
	if (m_SearchQuery.GetStatus() == CSearchQuery::SEARCHSTATUS_CLEAR)
	{
		if (pMover->CanMove())
		{
			if (fDistToMoveToLoc > fNoNewPathRange &&
				(pLeaderBot->MtxToWorld()->m_vPos.DistSq(pBot->MtxToWorld()->m_vPos) > fCloseEnoughOverride*fCloseEnoughOverride))
			{
				CSearchResults* pFreeResults = &(m_aSearchResults[0]);
				if (_IsMoverOnFollowPath() && pMover->m_pPath == pFreeResults->GetAIPath())
				{
					pFreeResults = &(m_aSearchResults[1]);
				}

				u16 uSearchParams = CSearchQuery::SEARCHPARAM_NEED_AIPATH | CSearchQuery::SEARCHPARAM_FUDGE_DEST;

				if (aiutils_CanEntityFly(pMover->GetEntity()))
				{
					uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
				}
				else
				{
					uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;
				}

				if (!pMover->CanJump())
				{
					uSearchParams &=~ CSearchQuery::SEARCHPARAM_NO_JUMPS;
				}
				aimain_pGraphSearcher->CancelSearch(&m_SearchQuery); //safe even if not in progress
				pFreeResults->Clear();

				BOOL bSkipPathRequest = FALSE;
				if (IsStuck())
				{
					bSkipPathRequest = TRUE;
					if (m_uStuckStage == STUCKSTAGE_STUCK_WAITING)
					{
						if (m_uStuckPathRetryTimeOut < aiutils_GetCurTimeSecsU16())
						{
							m_uStuckPathRetryTimeOut = aiutils_GetTimeOutRandMinMax((u8) 1, (u8) 3);
							bSkipPathRequest = FALSE;
						}
					}
				}

				if (!bSkipPathRequest)
				{
					m_SearchQuery.SetDefaultSearchParams(m_pBrain->GetLoc(),	//const CFVec3A& StartLoc_WS,
													MoveToLoc,	//const CFVec3A& GoalLoc_WS,
													pMover->GetRadiusXZ(),	//f32 fQueryWidth,
													pMover->GetMinHeight(),	//f32 fQueryHeight,
													pFreeResults,//CSearchResults* pResults,
													uSearchParams,	//u16 uSearchParams,
													(u32) m_pBrain);//u32 uClientGuid
					f32 fFudgeDist = 5.0f;

					if (1)//aiutils_CanEntityFly(pMover->GetEntity()))
					{
						fFudgeDist += 10.0f*m_uPathFailureCount;
						FMATH_CLAMPMAX(fFudgeDist, 80.0f);
					}
					m_SearchQuery.SetFudgeDestDist(fFudgeDist);
					m_SearchQuery.SetMaxJumpDist(GetBrain()->GetMaxJumpDist());
					aimain_pGraphSearcher->SubmitQuery(&m_SearchQuery);

					if (bMoveToLocIsFormationPost)
					{
						m_uFollowFlags |= FOLLOWFLAG_FORMATION_POST_PATH;
					}
				}
			}
		}
	}

	if (_IsMoverOnFollowPath())
	{	
		if (pMover->FollowPath())
		{
			//path is complete.
			if (!pMover->m_pPath->GetEndOfPath(&(m_LastGoodPt_WS)))
			{
				m_LastGoodPt_WS.Zero();
			}
			pMover->AssignPath(NULL);
			
		}
		else if (pMover->IsStuck(2.0f))
		{
			pMover->AssignPath(NULL);
		}

		//path complete when within some dist of leader if I don't have a formation post
		if (fCloseEnoughOverride > 0.0f)
		{
			if (pLeaderBot->MtxToWorld()->m_vPos.DistSq(pBot->MtxToWorld()->m_vPos) < fCloseEnoughOverride*fCloseEnoughOverride &&
				m_pBrain->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, pLeaderBot, NULL))
			{   
				m_LastGoodPt_WS = pBot->MtxToWorld()->m_vPos;
				pMover->AssignPath(NULL);
			}
		}
	}
	else 
	{	//stay put
		pMover->MoveToward(pMover->GetLoc());
	}


	//
	// Torso/Head Direction
	//

	if (!pT && !_IsMoverOnFollowPath())
	{
#define ALL_LOOK_AT_FLAGS (FOLLOWFLAG_LOOK_AT_LEADER|  FOLLOWFLAG_LOOK_WITH_LEADER | FOLLOWFLAG_LOOK_AWAYFROM_LEADER)
	
		u16 uNowTime = aiutils_GetCurTimeSecsU16();
 
		if (m_fTimeSinceMoved > 0.1f && m_fTimeSinceMoved < 0.5f && GetBrain()->GetInfreqCycleRandom() > 0.8f)
		{
			m_uNextLookAtLeaderTimeOut  = 0;	//force look at leader
		}

		if (!(m_uFollowFlags & FOLLOWFLAG_LOOK_AT_LEADER) &&
			m_uNextLookAtLeaderTimeOut < uNowTime)
		{
			m_uFollowFlags |= FOLLOWFLAG_LOOK_AT_LEADER;
			m_uLookAtLeaderTimeOut = aiutils_GetTimeOutRandMinMax((u8)1, (u8)3);   //how long to look at our leader
		}
		m_uFollowFlags &=~(FOLLOWFLAG_LOOK_WITH_LEADER|FOLLOWFLAG_LOOK_AWAYFROM_LEADER);

		if (pBot &&
			pLeaderBot && 
			pBot->GetDisguisedBot() != pLeaderBot &&
			pLeaderBot->GetLastFireTime() > 0.01f &&
			(!pLeaderMover || pLeaderMover->GetLoc().DistSq(pMover->GetLoc()) < 50.0f*50.0f))
		{
			if (aiutils_GetCurTimeSecs() - pLeaderBot->GetLastFireTime() < 3.0f)
			{
				if (m_pBrain->GetInfreqCycleRandom() < 0.5f)
				{
					m_uFollowFlags |= FOLLOWFLAG_LOOK_WITH_LEADER;
				}
				else 
				{
					m_uFollowFlags |= FOLLOWFLAG_LOOK_AWAYFROM_LEADER;
				}

				m_uFollowFlags &= ~FOLLOWFLAG_LOOK_AT_LEADER;
				m_uLookAtLeaderTimeOut = aiutils_GetTimeOutRandMinMax((u8)2, (u8)3);
			}
		}

		if (m_uFollowFlags & FOLLOWFLAG_LOOK_WITH_LEADER)
		{
			CFVec3A tmp;
			tmp = pLeaderMover->GetTorsoLookAtXZ();
			tmp.Mul(pLeaderMover->GetRadiusXZ()+ 20.0f);
			tmp.Add(pMover->GetEyePos());
			pMover->FaceToward(tmp);

		}
		else if (m_uFollowFlags & FOLLOWFLAG_LOOK_AWAYFROM_LEADER)
		{
			CFVec3A tmp;
			tmp.Sub(*(pLeaderBot->GetTagPoint(0)), *(pBot->GetTagPoint(0)));
			tmp.Mul(-1.0f);
			tmp.Add(pMover->GetEyePos());
			pMover->FaceToward(tmp);
		}
		else if (m_uFollowFlags & FOLLOWFLAG_LOOK_AT_LEADER)
		{
			CFVec3A tmp;
			tmp.Set(pLeaderMover->GetEyePos()); //look into his eyes
			pMover->FaceToward(tmp);

			if (m_uLookAtLeaderTimeOut < uNowTime)
			{
				m_uFollowFlags &= ~FOLLOWFLAG_LOOK_AT_LEADER;
				m_uNextLookAtLeaderTimeOut = aiutils_GetTimeOutRandMinMax((u8)25, (u8)60);	   //how long before next time to look at leader
			}
		}
		else
		{

			if (!m_pLookAtMgr)
			{
				m_pLookAtMgr = (CGenericWait*) CGenericWait::BankAccess(NULL);
				if (m_pLookAtMgr)
				{
					m_pLookAtMgr->Init(GetBrain(), THOUGHTFLAG_NONE, CAIBrain::TT_INVALID);
					m_pLookAtMgr->m_uWaitFlags |= CGenericWait::WAIT_VISRAYS_ENABLED;
					m_pLookAtMgr->m_uLookAroundChance = 0;  //disable random look around 
					m_pLookAtMgr->m_uPausedVisRayScanMin = 4;	//min length of a VisRayScan pause
					m_pLookAtMgr->m_uPausedVisRayScanMax = 18;	//max length of a VisRayScan pause
				}
			}

			//look like you are on guard by scanning the area.
			if (m_pLookAtMgr)
			{
				GetBrain()->GetAIMover()->m_Controls.Alert();
				m_pLookAtMgr->Work();
			}

		}


//pgm made sight mark reaction logic not react to leader since it is assumed follow will do that in a smarter way
//		m_pBrain->ClearSightMark();	 //since reaction logic is enabled, we need to make sure that the sightmark doesn't cause a facetoward
//		m_pBrain->ClearSoundMark();	 //same for sound

	}



	if (m_pBrain->GetCurThoughtPtr() && CAIBrain::IsGoal(m_pBrain->GetCurThought()))
	{
		m_uFollowFlags |= FOLLOWFLAG_HADGOAL;
	}

	//bottalks
	if (!bSuspicious && pLeaderBot && pLeaderBot->IsPlayerBot())
	{
		if (!GetBrain()->IsTalking())
		{
			BOOL bDidOne = FALSE;
			if (!(m_uFollowFlags & FOLLOWFLAG_GREETED_LEADER))
			{
				if (pszBTAName = AIBTA_EventToBTAName("P*F_JOIN", GetBrain()))
				{
					if (ai_TalkModeBegin(GetBrain(),
										AIBrain_TalkModeCB,
										(void*) pszBTAName,
										pMover->GetEntity(),
										1,
										TRUE, //Stop first if greetlevel == 0
										TRUE,  //LOOK AT
										pLeaderBot->Guid()))
					{
						m_uFollowFlags |= FOLLOWFLAG_GREETED_LEADER;
						bDidOne = TRUE;
					}
				}
			}


			if (!bDidOne)
			{
				if (uLastLeaderKills != m_uLeaderKills)
				{
					if (pszBTAName = AIBTA_EventToBTAName("P*F_BOSS", GetBrain()))
					{
						if (ai_TalkModeBegin(GetBrain(),
											AIBrain_TalkModeCB,
											(void*) pszBTAName,
											pMover->GetEntity(),
											1,
											TRUE, //Stop first if greetlevel == 0
											TRUE,  //LOOK AT
											pLeaderBot->Guid()))
						{
							bDidOne = TRUE;
						}
					}
				}
			}

			if (!bDidOne && _fMotiTimer <= 0.0f)
			{
				if ((aiutils_GetCurTimeSecs() - GetBrain()->GetLastXYZChangeTime() > 5.0f))
				{
					if (aiutils_GetLastTimeSecsU16() != aiutils_GetCurTimeSecsU16() &&
						((aiutils_GetCurTimeSecsU16() &0x3)==3))
					{
						if (GetBrain()->GetInfreqCycleRandom() > 0.3f && GetBrain()->GetInfreqCycleRandom() < 0.6f)
						{
							if (pszBTAName = AIBTA_EventToBTAName("P*F_MOTI", GetBrain()))
							{
								if (ai_TalkModeBegin(GetBrain(),
													AIBrain_TalkModeCB,
													(void*) pszBTAName,
													pMover->GetEntity(),
													1,
													TRUE, //Stop first if greetlevel == 0
													TRUE,  //LOOK AT
													pLeaderBot->Guid()))
								{
									bDidOne = TRUE;
									_fMotiTimer = 6.0f + fmath_RandomFloat()*40.0f;
								}
							}
						}
					}
				}
			}

			if (!bDidOne)
			{
				if ((aiutils_GetCurTimeSecs() - GetBrain()->GetLastXYZChangeTime() > 1.0f))
				{

					if (aiutils_GetLastTimeSecsU16() != aiutils_GetCurTimeSecsU16() &&
						((aiutils_GetCurTimeSecsU16() &0x3)==3))
					{
						if (GetBrain()->GetInfreqCycleRandom() > 0.2f && GetBrain()->GetInfreqCycleRandom() < 0.8f)
						{
							if (pszBTAName = AIBTA_EventToBTAName("P*F_IDLE", GetBrain()))
							{
								if (ai_TalkModeBegin(GetBrain(),
													AIBrain_TalkModeCB,
													(void*) pszBTAName,
													pMover->GetEntity(),
													1,
													TRUE, //Stop first if greetlevel == 0
													FALSE,  //LOOK AT
													pLeaderBot->Guid()))
								{
									bDidOne = TRUE;
								}
							}
						}
					}
				}
			}

		}
	}
}						  




//
//  system functions that every thought class must implement to support allocation pools. 
//	 findfix: make it go away somehow
//
//
CNiBank<CGenericWait>* CGenericWait::s_pAllocBank = NULL;
CNiBank<CGenericWander>* CGenericWander::s_pAllocBank = NULL;
CNiBank<CGenericPatrol>* CGenericPatrol::s_pAllocBank = NULL;
CNiBank<CGenericGoto>* CGenericGoto::s_pAllocBank = NULL;
CNiBank<CGenericFaceIt>* CGenericFaceIt::s_pAllocBank = NULL;
CNiBank<CGenericTalkTo>* CGenericTalkTo::s_pAllocBank = NULL;
CNiBank<CGenericFollow>* CGenericFollow::s_pAllocBank = NULL;

BOOL CGenericWait::InitBank(s32 nHowMany)
{
	if (!CGenericWait::s_pAllocBank)
	{
		CGenericWait::s_pAllocBank = APE_NEW CNiBank<CGenericWait>(aimain_pNodePool, nHowMany, APE_NEW CGenericWait[nHowMany]);	 //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}

	return CGenericWait::s_pAllocBank != NULL;
}


void CGenericWait::CleanupBank(void)
{
	APE_DELETE( CGenericWait::s_pAllocBank); CGenericWait::s_pAllocBank = NULL;
}


CAIThought* CGenericWait::BankAccess(CAIThought* pRecycle)
{
	FASSERT(CGenericWait::s_pAllocBank);   //whoops?

	if (pRecycle)
	{
		FASSERT(pRecycle->GetClassInterface() == CLASS_CGENERICWAIT);
		CGenericWait::s_pAllocBank->Recycle((CGenericWait *) pRecycle);
	}
	else
	{
		return CGenericWait::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CGenericWait::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGENERICWAIT) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGenericWait::GetClassInterface(void)
{
	return CLASS_CGENERICWAIT;
}



BOOL CGenericWander::InitBank(s32 nHowMany)
{
	if (!CGenericWander::s_pAllocBank)
	{
		CGenericWander::s_pAllocBank = APE_NEW CNiBank<CGenericWander>(aimain_pNodePool, nHowMany, APE_NEW CGenericWander[nHowMany]);	 //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CGenericWander::s_pAllocBank != NULL;
}


void CGenericWander::CleanupBank(void)
{
	APE_DELETE( CGenericWander::s_pAllocBank); CGenericWander::s_pAllocBank = NULL;
}


CAIThought* CGenericWander::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		FASSERT(pRecycle->GetClassInterface() == CLASS_CGENERICWANDER);
		CGenericWander::s_pAllocBank->Recycle((CGenericWander *) pRecycle);
	}
	else
	{
		return CGenericWander::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CGenericWander::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGENERICWANDER) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGenericWander::GetClassInterface(void)
{
	return CLASS_CGENERICWANDER;
}


BOOL CGenericPatrol::InitBank(s32 nHowMany)
{
	if (!CGenericPatrol::s_pAllocBank)
	{
		CGenericPatrol::s_pAllocBank = APE_NEW CNiBank<CGenericPatrol>(aimain_pNodePool, nHowMany, APE_NEW CGenericPatrol[nHowMany]);  //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}

	return CGenericPatrol::s_pAllocBank != NULL;
}


void CGenericPatrol::CleanupBank(void)
{
	APE_DELETE( CGenericPatrol::s_pAllocBank); CGenericPatrol::s_pAllocBank = NULL;
}


CAIThought* CGenericPatrol::BankAccess(CAIThought* pRecycle)
{
	FASSERT(CGenericPatrol::s_pAllocBank);   //whoops?
	if (pRecycle)
	{
		FASSERT(pRecycle->GetClassInterface() == CLASS_CGENERICPATROL);
		CGenericPatrol::s_pAllocBank->Recycle((CGenericPatrol *) pRecycle);
	}
	else
	{
		return CGenericPatrol::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CGenericPatrol::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGENERICPATROL) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGenericPatrol::GetClassInterface(void)
{
	return CLASS_CGENERICPATROL;
}


BOOL CGenericGoto::InitBank(s32 nHowMany)
{
	if (!CGenericGoto::s_pAllocBank)
	{
		CGenericGoto::s_pAllocBank = APE_NEW CNiBank<CGenericGoto>(aimain_pNodePool, nHowMany, APE_NEW CGenericGoto[nHowMany]);		 //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}

	return CGenericGoto::s_pAllocBank != NULL;
}


void CGenericGoto::CleanupBank(void)
{
	APE_DELETE( CGenericGoto::s_pAllocBank); CGenericGoto::s_pAllocBank = NULL;
}


CAIThought* CGenericGoto::BankAccess(CAIThought* pRecycle)
{
	FASSERT(CGenericGoto::s_pAllocBank);   //whoops?
	if (pRecycle)
	{
		FASSERT(pRecycle->GetClassInterface() == CLASS_CGENERICGOTO);
		CGenericGoto::s_pAllocBank->Recycle((CGenericGoto *) pRecycle);
	}
	else
	{
		return CGenericGoto::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CGenericGoto::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGENERICGOTO) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGenericGoto::GetClassInterface(void)
{
	return CLASS_CGENERICGOTO;
}


BOOL CGenericFaceIt::InitBank(s32 nHowMany)
{
	if (!CGenericFaceIt::s_pAllocBank)
	{
		CGenericFaceIt::s_pAllocBank = APE_NEW CNiBank<CGenericFaceIt>(aimain_pNodePool, nHowMany, APE_NEW CGenericFaceIt[nHowMany]);	   //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CGenericFaceIt::s_pAllocBank != NULL;
}


void CGenericFaceIt::CleanupBank(void)
{
	APE_DELETE( CGenericFaceIt::s_pAllocBank); CGenericFaceIt::s_pAllocBank = NULL;
}


CAIThought* CGenericFaceIt::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CGenericFaceIt::s_pAllocBank->Recycle((CGenericFaceIt *) pRecycle);
	}
	else
	{
		return CGenericFaceIt::s_pAllocBank->Get();
	}
	return NULL;
}


BOOL CGenericFaceIt::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGENERICFACEIT) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGenericFaceIt::GetClassInterface(void)
{
	return CLASS_CGENERICFACEIT;
}


BOOL CGenericTalkTo::InitBank(s32 nHowMany)
{
	if (!CGenericTalkTo::s_pAllocBank)
	{
		CGenericTalkTo::s_pAllocBank = APE_NEW CNiBank<CGenericTalkTo>(aimain_pNodePool, nHowMany, APE_NEW CGenericTalkTo[nHowMany]);		//note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CGenericTalkTo::s_pAllocBank != NULL;
}


void CGenericTalkTo::CleanupBank(void)
{
	APE_DELETE( CGenericTalkTo::s_pAllocBank); CGenericTalkTo::s_pAllocBank = NULL;
}


CAIThought* CGenericTalkTo::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CGenericTalkTo::s_pAllocBank->Recycle((CGenericTalkTo *) pRecycle);
	}
	else
	{
		return CGenericTalkTo::s_pAllocBank->Get();
	}
	return NULL;
}

BOOL CGenericTalkTo::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGENERICTALKTO) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGenericTalkTo::GetClassInterface(void)
{
	return CLASS_CGENERICTALKTO;
}


BOOL CGenericFollow::InitBank(s32 nHowMany)
{
	if (!CGenericFollow::s_pAllocBank)
	{
		CGenericFollow::s_pAllocBank = APE_NEW CNiBank<CGenericFollow>(aimain_pNodePool, nHowMany, APE_NEW CGenericFollow[nHowMany]);	  //note, this APE_NEW used to and still should be inside the template, but it wouldn't compile on GC platoform. boo
	}
	return CGenericFollow::s_pAllocBank != NULL;
}


void CGenericFollow::CleanupBank(void)
{
	APE_DELETE( CGenericFollow::s_pAllocBank); CGenericFollow::s_pAllocBank = NULL;
}


CAIThought* CGenericFollow::BankAccess(CAIThought* pRecycle)
{
	if (pRecycle)
	{
		CGenericFollow::s_pAllocBank->Recycle((CGenericFollow *) pRecycle);
	}
	else
	{
		return CGenericFollow::s_pAllocBank->Get();
	}
	return NULL;
}

BOOL CGenericFollow::SupportsClassInterface(const s32 nInterfaceId)
{
	return (nInterfaceId == CLASS_CGENERICFOLLOW) || CAIThought::SupportsClassInterface(nInterfaceId);
}


s32 CGenericFollow::GetClassInterface(void)
{
	return CLASS_CGENERICFOLLOW;
}

