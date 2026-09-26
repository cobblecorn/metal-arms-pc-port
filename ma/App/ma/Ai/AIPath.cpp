#include "fang.h"
#include "fres.h"
#include "AIPath.h"
#include "AIMath.h"
#include "AIGraphSearcher.h"
#include "fdraw.h"

//static member initialization
CNiBank<CAIPathWaypoint>* CAIPath::s_pWayPointBank = NULL;
CAIPathWaypoint* CNiIterator<CAIPathWaypoint*>::s_ReturnError;
FLinkRoot_t* CAIPath::s_pPtrNodePool = NULL;



//
//  CAIPathWaypoint
//


//
//  CAIPath
//
BOOL CAIPath::InitSystem(FLinkRoot_t* pGlobalPtrNodePool, s32 nMaxNumGlobalPathNodes /*= AIPATH_DEFAULT_GLOBAL_WAYPOINT_POOL_COUNT*/)
{
	FASSERT(s_pWayPointBank == NULL);
	s_pWayPointBank = APE_NEW CNiBank<CAIPathWaypoint>(pGlobalPtrNodePool, nMaxNumGlobalPathNodes, APE_NEW CAIPathWaypoint[nMaxNumGlobalPathNodes]);
	s_pPtrNodePool = pGlobalPtrNodePool;
	return s_pWayPointBank != NULL;
}


BOOL CAIPath::InitSystem(FLinkRoot_t* pGlobalPtrNodePool, void* pWaypointNodeMemory, s32 nMemorySizeBytes)
{
	FASSERT(s_pWayPointBank == NULL);

	s32 nWaypointSizeBytes = sizeof(CAIPathWaypoint);
	s32 nBankCount = nMemorySizeBytes/nWaypointSizeBytes;
	s_pWayPointBank = APE_NEW CNiBank<CAIPathWaypoint>(pGlobalPtrNodePool, nBankCount, APE_NEW CAIPathWaypoint[nBankCount]);
	return s_pWayPointBank != NULL;
}


void CAIPath::UninitSystem(void)
{
	//findfix: could verify that all waypoints have been returned
	APE_DELETE(s_pWayPointBank);  s_pWayPointBank = NULL; 
	s_pPtrNodePool = NULL;
}


void CAIPath::UseThisPtrNodePool(FLinkRoot_t* pNodePool)
{
	s_pPtrNodePool = pNodePool;
}


void CAIPath::RecycleWayPt(CAIPathWaypoint* pWay)
{
	s_pWayPointBank->Recycle(pWay);
}



CAIPath::CAIPath(void) 
{
	m_pGraph = NULL;
	m_fPathLength = 0.0f;
	if (s_pPtrNodePool)
	{
		m_WaypointList.SetNodePool(s_pPtrNodePool);
		m_ReverseWaypointList.SetNodePool(s_pPtrNodePool);
	}
}


CAIPath::~CAIPath(void)
{
	FASSERT(m_WaypointList.Size()==0);
	FASSERT(m_ReverseWaypointList.Size()==0);
	Reset();
}


void CAIPath::Reset(void)
{
	CNiList<CAIPathWaypoint*>* aWayPointList[2] = { &m_WaypointList, &m_ReverseWaypointList};

	for (s32 i = 0; i < 2; i++)
	{
		CNiIterator<CAIPathWaypoint*> it = aWayPointList[i]->Begin();
		while (it.IsValid())
		{
			FASSERT(s_pWayPointBank);
			s_pWayPointBank->Recycle(it.Get());
			it.Next();
		}
		aWayPointList[i]->RemoveAll();
	}
	m_fPathLength = 0.0f;
	m_pGraph = NULL;
}


BOOL CAIPath::GetEndOfPath(CFVec3A* pEndPt_WS )
{
	CNiIterator<CAIPathWaypoint*> it = m_WaypointList.End();
	if (it.IsValid())
	{
		if (pEndPt_WS)
		{
			*pEndPt_WS = it.Get()->m_Location;
		}
		return TRUE;
	}
	return FALSE;
}


//
//  CAIPathWalker
//
CAIPathWalker::CAIPathWalker(void)
{
}


CAIPathWalker::~CAIPathWalker(void)
{
}


void CAIPathWalker::Init(CAIPath* pPath)
{
	m_uCurWayPointNum = 0;
	m_uReverse = FALSE;
	m_uLoop = FALSE;
	m_pPath = pPath;
	if (m_pPath)
	{
		m_PathIt = pPath->BeginPathWalk();
	}
	else
	{
		m_PathIt.Reset();
	}
}


void CAIPathWalker::ChangeDirection(CAIPath* pPath)
{
	CFVec3A Loc;
	GetCurLoc(&Loc);
	m_uReverse = 1-m_uReverse;
	if (pPath)
	{
		s32 nWayNum = pPath->FindClosestWaypointNum(Loc, m_uReverse);
		FASSERT(nWayNum>=0);
		m_PathIt = pPath->BeginPathWalk(m_uReverse != 0);
		s32 nCount = 0;
		m_uCurWayPointNum = 0;
		while (nCount != nWayNum && m_PathIt.HasNext())
		{
			m_PathIt.Next();
			nCount++;
			m_uCurWayPointNum++;
		}
	}
	else
	{
		m_PathIt.Reset();
	}
}


void CAIPathWalker::AdvanceTo(CAIPath* pPath, u16 nWayptNum)
{
	m_PathIt = pPath->BeginPathWalk(m_uReverse);
	m_uCurWayPointNum = 0;
	while (m_uCurWayPointNum != nWayptNum && Advance());
}


u16 CAIPathWalker::GetCurVertId(void)
{
	if (m_pPath && m_PathIt.IsValid() && m_pPath->GetGraph() && m_PathIt.Get()->m_pGraphVert)
	{
		return m_pPath->GetGraph()->GetVertId(m_PathIt.Get()->m_pGraphVert);
	}
	return INVALID_VERTID;
}


u16 CAIPathWalker::GetLastVertId(void)
{
	CAIPathWaypoint* pPrevWay = GetPreviousWaypoint();

	if (pPrevWay)
	{
		if (m_pPath && pPrevWay && m_pPath->GetGraph() && pPrevWay->m_pGraphVert)
		{
			return m_pPath->GetGraph()->GetVertId(pPrevWay->m_pGraphVert);
		}
	}
	return INVALID_VERTID;
}

	
BOOL CAIPathWalker::GetCurLoc(CFVec3A *pLoc)
{
	if (m_PathIt.IsValid() && pLoc)
	{
		*pLoc = m_PathIt.Get()->m_Location;
		return TRUE;
	}
	return FALSE;
}


CAIPathWaypoint* CAIPathWalker::GetCurWaypoint(void)
{
	if (m_PathIt.IsValid())
	{
		return m_PathIt.Get();
	}
	return NULL;
}


CAIPathWaypoint* CAIPathWalker::GetPreviousWaypoint(void)
{
	if (m_PathIt.HasBack())
	{
		CNiIterator<CAIPathWaypoint*> It = m_PathIt;
		It.Back();
		FASSERT(It.IsValid());
		return It.Get();
	}
	return NULL;
}

CAIPathWaypoint* CAIPathWalker::GetNextWaypoint(void)
{
	if (m_PathIt.HasNext())
	{
		CNiIterator<CAIPathWaypoint*> It = m_PathIt;
		It.Next();
		FASSERT(It.IsValid());
		return It.Get();
	}
	return NULL;
}

CAIPathWaypoint* CAIPathWalker::GetNextNextWaypoint(void)
{
	if (m_PathIt.HasNextNext())
	{
		CNiIterator<CAIPathWaypoint*> It = m_PathIt;
		It.Next();
		It.Next();
		FASSERT(It.IsValid());
		return It.Get();
	}
	return NULL;
}


const f32 kAbsoluteCloseEnough = 1.0f;

BOOL CAIPathWalker::CloseEnoughXZ(const CFSphereA& Sphere)
{
	if (m_PathIt.IsValid())
	{
		f32 fCloseEnoughDist = 0.0f;   //how close one must be to STOP_AT points and points closer than clients SphereRadius
		CAIPathWaypoint* pWay = GetCurWaypoint();
		FASSERT(pWay);
		if ((pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_STOP_AT))
		{
			//findfix: close enough for ends of paths is not current configureable.. 
			fCloseEnoughDist += kAbsoluteCloseEnough + Sphere.m_fRadius;
			if (fCloseEnoughDist > 10.0f)
			{
				fCloseEnoughDist = 10.0f;		 
			}
		}
		else if (pWay->m_fCloseEnoughDist - Sphere.m_fRadius > 1.5f)  //1.5 is closest for paths along the way
		{
			fCloseEnoughDist = pWay->m_fCloseEnoughDist - Sphere.m_fRadius;
		}
		else
		{
			fCloseEnoughDist = 1.5f;

		}

		f32 fDistSqXZ = Sphere.m_Pos.DistSqXZ(pWay->m_Location);
		if (fDistSqXZ < fCloseEnoughDist*fCloseEnoughDist)
		{
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CAIPathWalker::CloseEnough(const CFSphereA& Sphere)
{
	if (m_PathIt.IsValid())
	{
		f32 fCloseEnoughDist = kAbsoluteCloseEnough;   //how close one must be to STOP_AT points and points smaller that clients SphereRadius
		CAIPathWaypoint* pWay = GetCurWaypoint();
		FASSERT(pWay);

		if ((pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_STOP_AT))
		{
			fCloseEnoughDist = Sphere.m_fRadius;			
			f32 fDist2 = Sphere.m_Pos.DistSq(pWay->m_Location);
			if (fDist2 < (fCloseEnoughDist*fCloseEnoughDist))
			{
				return TRUE;
			}
		}
		else
		{
			f32 fVertHeight = 0.0f;
			if (pWay->m_pGraphVert)
			{
			   fVertHeight = pWay->m_pGraphVert->m_fHeightClearance;
			}
			else
			{
				fVertHeight = pWay->m_fCloseEnoughDist;

			}
			CFCylnA tmp;
			tmp.m_Origin = pWay->m_Location;
			tmp.m_Origin.y -=fVertHeight;
			tmp.m_Axis = CFVec3A::m_UnitAxisY;
			tmp.m_fWidth = pWay->m_fCloseEnoughDist;
			tmp.m_fHeight = fVertHeight*2.0f;
			if (tmp.IntersectSphereApprox(Sphere))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CAIPathWalker::Advance(void)
{
	if (m_PathIt.HasNext())
	{
		m_uCurWayPointNum++;
		m_PathIt.Next();
		return 1;
	}
	else if (m_uLoop)
	{
		BOOL bAdvanced = FALSE;
		while (m_PathIt.HasBack())
		{
			m_uCurWayPointNum--;
			m_PathIt.Back();
			bAdvanced = TRUE;
		}
		return bAdvanced;
	}
	return 0;
}


s32 CAIPath::FindClosestWaypointNum(const CFVec3A& loc, BOOL bReverse)
{
	f32 fDist2;
	f32 fClosestDist2 = 0.0f;
	s32 nClosest = -1;
	s32 nCount = 0;
	CNiIterator<CAIPathWaypoint*> it = BeginPathWalk(bReverse);
	while (it.IsValid())
	{
		CAIPathWaypoint* pTmp = it.Get();
		it.Next();

		fDist2 = pTmp->m_Location.DistSq(loc);
		if (nClosest==-1 || fDist2 < fClosestDist2)
		{
			fClosestDist2 = fDist2;
			nClosest = nCount;
		}
		nCount++;
	}

	return nClosest;
}



void CAIPath::JoinPaths(CAIPath& rOtherPath, BOOL bReverse)
{
	CNiList<CAIPathWaypoint*>* aWayPointList[2] = { &m_WaypointList, &m_ReverseWaypointList};

	CNiIterator<CAIPathWaypoint*> it = rOtherPath.BeginPathWalk();
	CAIPathWaypoint* pTmp;
	while (rOtherPath.GetNumWaypoints())
	{
		pTmp = rOtherPath.m_WaypointList.PopHead();
		aWayPointList[bReverse!=0]->PushTail(pTmp);
	}
}



static 	f32 kfDebugRenderHeightSpread = 4.0f;

void CAIPath::DebugRender(BOOL bReverse)
{
	CFColorRGBA _rgb = FColor_MotifBlue;

	BOOL bLinkLast = 0;
	CFVec3 LastLoc;
	f32 fHeight = 0.0f;

	CNiIterator<CAIPathWaypoint*> it = BeginPathWalk(bReverse);
	while (it.IsValid())
	{
		CAIPathWaypoint* pTmp = it.Get();
		it.Next();

		CFVec3 drawLoc = pTmp->m_Location.v3;
		if (!bLinkLast)
		{
			fHeight = drawLoc.y;
		}
//		drawLoc.y =fHeight;
		fdraw_FacetedWireSphere( &(drawLoc), 2.0f, 1, 1 , &_rgb);
		if (bLinkLast)
		{
			fdraw_SolidLine(&drawLoc, &(LastLoc), &_rgb);
		}
		LastLoc = drawLoc;
		bLinkLast = TRUE;
		fHeight+= kfDebugRenderHeightSpread;
	}
}


void CAIPath::DontStopAtEnd(BOOL bReverse)
{
	CNiList<CAIPathWaypoint*>* aWayPointList[2] = { &m_WaypointList, &m_ReverseWaypointList};

	CNiIterator<CAIPathWaypoint*> it = aWayPointList[(s32) bReverse]->End();
	it.Get()->m_uWayPointFlags &= ~CAIPathWaypoint::WAYPOINTFLAG_STOP_AT;
}

#if !FANG_ENABLE_INLINE_CODE
#include "AIPath.inl"
#endif

