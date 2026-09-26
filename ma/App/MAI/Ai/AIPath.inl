#include "Ftl.h"


FINLINE void CAIPath::AddWayPointHead(const CAIPathWaypoint& rWaypoint, BOOL bReverse /*= FALSE*/)
{
	CNiList<CAIPathWaypoint*>* aWayPointList[2] = { &m_WaypointList, &m_ReverseWaypointList};

	CAIPathWaypoint* pWay = s_pWayPointBank->Get();
	FASSERT(pWay);
	if (pWay)
	{
		*pWay = rWaypoint;
		aWayPointList[(s32) bReverse]->PushHead(pWay);
	}
}


FINLINE void CAIPath::AddWayPointTail(const CAIPathWaypoint& rWaypoint, BOOL bReverse /*= FALSE*/)
{
	CNiList<CAIPathWaypoint*>* aWayPointList[2] = { &m_WaypointList, &m_ReverseWaypointList};
	CAIPathWaypoint* pWay = s_pWayPointBank->Get();
	FASSERT(pWay);
	if (pWay)
	{
		*pWay = rWaypoint;
		aWayPointList[(s32) bReverse]->PushTail(pWay);
	}
}


FINLINE CAIPathWaypoint* CAIPath::ReverseAddWayPointHead(const CAIPathWaypoint& rWaypoint)
{
	CAIPathWaypoint* pWay = s_pWayPointBank->Get();
	FASSERT(pWay);
	if (pWay)
	{
		*pWay = rWaypoint;
		m_ReverseWaypointList.PushHead(pWay);
	}
	return pWay;
}


FINLINE CAIPathWaypoint* CAIPath::ReverseAddWayPointTail(const CAIPathWaypoint& rWaypoint)
{
	CAIPathWaypoint* pWay = s_pWayPointBank->Get();
	FASSERT(pWay);
	if (pWay)
	{
		*pWay = rWaypoint;
		m_ReverseWaypointList.PushTail(pWay);
	}

	return pWay;
}


FINLINE CNiIterator<CAIPathWaypoint*> CAIPath::BeginPathWalk(BOOL bReverse /* = FALSE*/)
{
	CNiList<CAIPathWaypoint*>* aWayPointList[2] = { &m_WaypointList, &m_ReverseWaypointList};

	return aWayPointList[(s32) bReverse]->Begin();
}


FINLINE u16 CAIPath::GetNumWaypoints(void)
{
	return m_WaypointList.Size();
}


