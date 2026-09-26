//////////////////////////////////////////////////////////////////////////////////////
// AIPatrolPath.cpp - 
//
// Author: Pat MacKellar 
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
// 05/21/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIPatrolPath.h"
#include "Apenew.h"
#include "AIPath.h"
#include "AINodepools.h"
#include "AIMain.h"
#include "AIBrain.h"
#include "AIMover.h"
#include "AiApi.h"
#include "AIThoughtsGeneric.h"
#include "AIGameUtils.h"
#include "../Entity.h"
#include "../gstring.h"
#include "../espline.h"
#include "../talksystem2.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions

//==================
// private functions



CNiList<CPatrolPath*> CPatrolPath::s_CookingList;
CNiList<CPatrolPath*> CPatrolPath::s_RegisterList;
CNiBank<CPatrolPath>* CPatrolPath::s_pAllocBank = NULL;
CPatrolPath *CNiIterator<class CPatrolPath *>::s_ReturnError;
CSearchQuery *CPatrolPath::s_pSearchQuery = NULL;
CSearchResults *CPatrolPath::s_pSearchResults = NULL;
u16 CPatrolPath::s_uNumPatrolLinksFound = 0;
const s32 knPatrolPathBankSize = 50;

BOOL CPatrolPath::InitSystem(void)
{
	if (!s_pAllocBank)
	{
		s_pAllocBank = APE_NEW CNiBank<CPatrolPath>(aimain_pNodePool, knPatrolPathBankSize, APE_NEW CPatrolPath[knPatrolPathBankSize]);
		if (!s_pAllocBank)
		{
			return FALSE;
		}
		FASSERT(s_RegisterList.Size()==0);
		s_RegisterList.SetNodePool(aimain_pNodePool);
		s_CookingList.SetNodePool(aimain_pNodePool);

		s_pSearchQuery = APE_NEW CSearchQuery();
		s_pSearchResults = APE_NEW CSearchResults();
	}
	return TRUE;
}


void CPatrolPath::UninitSystem(void)
{
	//cancel any current search the patrol path system might have in the oven
	aimain_pGraphSearcher->CancelSearch(s_pSearchQuery); //safe even if not in progress

	APE_DELETE(s_pSearchQuery); s_pSearchQuery = NULL;
	APE_DELETE(s_pSearchResults); s_pSearchResults = NULL;

	//reset all cooked paths
	CNiIterator<CPatrolPath*> it = s_RegisterList.Begin();
	while (it.IsValid())
	{
		CPatrolPath* pPatrolPath = it.Get();
		it.Next();
		if (pPatrolPath)
		{
			pPatrolPath->GetAIPath()->Reset();
		}
	}
	s_RegisterList.RemoveAll();

	it = s_CookingList.Begin();
	while (it.IsValid())
	{
		CPatrolPath* pPatrolPath = it.Get();
		it.Next();
		if (pPatrolPath)
		{
			pPatrolPath->GetAIPath()->Reset();
		}
	}
	s_CookingList.RemoveAll();

	APE_DELETE( s_pAllocBank); s_pAllocBank = NULL;

}


void CPatrolPath::Work(void)
{
	if (s_CookingList.Size())
	{
		CPatrolPath* pWorking = s_CookingList.Begin().Get();
		CESpline* pPtArray = pWorking->GetPtArray();
		FASSERT(pPtArray);
		FASSERT(pWorking);
		switch (pWorking->m_uStage)
		{
		case FORWARD_INIT:
			s_uNumPatrolLinksFound = 0;
			pWorking->m_uStage = FORWARD_EDGE_INIT;
			break;
		case REVERSE_INIT:
			s_uNumPatrolLinksFound = 0;
			pWorking->m_uStage = REVERSE_EDGE_INIT;
			break;
		case FORWARD_EDGE_INIT:
			{
				u32 uNumNeeded = pPtArray->PointCount();
				if (!pPtArray->IsClosed())  //pingpong
				{
					uNumNeeded--;
				}

				if (s_uNumPatrolLinksFound < uNumNeeded)
				{
					u16 uSearchParams = CSearchQuery::SEARCHPARAM_NEED_AIPATH;

					if (pWorking->m_b3DPath)
					{
						uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
					}
					else
					{
					   uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;
					}
					//findfix: need someway to make 3d patrol paths
					u16 uThisLink = s_uNumPatrolLinksFound%(u16)pPtArray->PointCount();
					u16 uNextLink = (s_uNumPatrolLinksFound+1)%(u16)pPtArray->PointCount();
					aimain_pGraphSearcher->CancelSearch(s_pSearchQuery); //safe even if not in progress
					s_pSearchResults->Clear();
					s_pSearchQuery->Clear();

					CFVec3A start =pPtArray->PointArray()[uThisLink];
					f32 fNewHeight = 0.0f;
					if (!(pWorking->m_b3DPath) && aiutils_FloorElevationAt(start, &fNewHeight, 10.0f) && fNewHeight > start.y)
					{
						start.y = fNewHeight;
					}

					CFVec3A end = pPtArray->PointArray()[uNextLink];
					if (!(pWorking->m_b3DPath) && aiutils_FloorElevationAt(end, &fNewHeight, 10.0f) && fNewHeight > end.y)
					{
						end.y = fNewHeight;
					}
					s_pSearchQuery->SetDefaultSearchParams(start,
															end,
															pWorking->GetWidth(),
															pWorking->GetHeight(),
															s_pSearchResults,
															uSearchParams,
															0xfeed);
					s_pSearchQuery->SetFudgeDestDist(10.0f);
					aimain_pGraphSearcher->SubmitQuery(s_pSearchQuery);
														
					pWorking->m_uStage = FORWARD_EDGE_WAITING;
				}
				else
				{	//done finding link-paths
					pWorking->m_uStage = FORWARD_COOKED;
				}
			}
			break;
		case REVERSE_EDGE_INIT:
			{
				u32 uNumNeeded = pPtArray->PointCount();
				u32 uFirstLink = 0;
				u16 uThisLink;
				u16 uNextLink;

				if (pPtArray->IsClosed())
				{	//reverse
					uThisLink = (u16) ((pPtArray->PointCount()*4-s_uNumPatrolLinksFound)% pPtArray->PointCount());
					uNextLink = (u16) ((pPtArray->PointCount()*4-1-s_uNumPatrolLinksFound)% pPtArray->PointCount());
				}
				else
				{	//pingpong
					uNumNeeded--;
					uThisLink = (u16) ((pPtArray->PointCount()*4-1-s_uNumPatrolLinksFound) % pPtArray->PointCount());
					uNextLink = (u16) ((pPtArray->PointCount()*4-2-s_uNumPatrolLinksFound) % pPtArray->PointCount());
				}
				if (s_uNumPatrolLinksFound < uNumNeeded)
				{
					u16 uSearchParams = CSearchQuery::SEARCHPARAM_NEED_AIPATH;

					if (pWorking->m_b3DPath)
					{
						uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
					}
					else
					{
						uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;
					}

					aimain_pGraphSearcher->CancelSearch(s_pSearchQuery); //safe even if not in progress
					s_pSearchResults->Clear();
					s_pSearchQuery->Clear();

					CFVec3A start =pPtArray->PointArray()[uThisLink];
					f32 fNewHeight = 0.0f;
					if (aiutils_FloorElevationAt(start, &fNewHeight, 10.0f) && fNewHeight > start.y)
					{
						start.y = fNewHeight;
					}

					CFVec3A end = pPtArray->PointArray()[uNextLink];
					if (aiutils_FloorElevationAt(end, &fNewHeight, 10.0f) && fNewHeight > end.y)
					{
						end.y = fNewHeight;
					}
					s_pSearchQuery->SetDefaultSearchParams(start,
															end,
															pWorking->GetWidth(),
															pWorking->GetHeight(),
															s_pSearchResults,
															uSearchParams,
															0xfeed);
					s_pSearchQuery->SetFudgeDestDist(10.0f);
					aimain_pGraphSearcher->SubmitQuery(s_pSearchQuery);
					pWorking->m_uStage = REVERSE_EDGE_WAITING;
				}
				else
				{	//done finding link-paths
					pWorking->m_uStage = REVERSE_COOKED;
				}
			}
			break;
		case FORWARD_EDGE_WAITING:
			if (s_pSearchQuery->IsDone())
			{
				if (s_pSearchResults->GetAIPath())
				{
					pWorking->m_AIPath.SetGraph(aimain_pAIGraph);
					pWorking->m_AIPath.JoinPaths(*(s_pSearchResults->GetAIPath()));
				}
				s_uNumPatrolLinksFound++;
				pWorking->m_uStage = FORWARD_EDGE_INIT;
			}
			else if (s_pSearchQuery->HasFailed())
			{
				if (s_pSearchQuery->GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_START)
				{
					DEVPRINTF("AI: Can't connect all knots in patrol path '%s'. Failed Knot %d of %d\n",
								pWorking->GetPtArray()->Name() ? pWorking->GetPtArray()->Name() : "NONAME",
								s_uNumPatrolLinksFound, pPtArray->PointCount());
				}
				else if (s_pSearchQuery->GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_END)
				{
					DEVPRINTF("AI: Can't connect all knots in patrol path '%s'. Failed on Knot After %d of %d\n",
								pWorking->GetPtArray()->Name() ? pWorking->GetPtArray()->Name() : "NONAME",
								s_uNumPatrolLinksFound, pPtArray->PointCount());
				}
				else
				{
					DEVPRINTF("AI: Can't connect all knots in patrol path '%s'. Failed on Knot %d of %d\n",
								pWorking->GetPtArray()->Name() ? pWorking->GetPtArray()->Name() : "NONAME",
								s_uNumPatrolLinksFound, pPtArray->PointCount());
				}
		

				pWorking->m_uStage = FAILED;
				s_CookingList.PopHead();
				s_RegisterList.PushHead();
			}
			break;
		case REVERSE_EDGE_WAITING:
			if (s_pSearchQuery->IsDone())
			{
				if (s_pSearchResults->GetAIPath())
				{
					pWorking->m_AIPath.SetGraph(aimain_pAIGraph);
					pWorking->m_AIPath.JoinPaths(*(s_pSearchResults->GetAIPath()), 1);  //1 means reverse path
				}
				s_uNumPatrolLinksFound++;
				pWorking->m_uStage = REVERSE_EDGE_INIT;
			}
			else if (s_pSearchQuery->HasFailed())
			{
				if (s_pSearchQuery->GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_START)
				{
					DEVPRINTF("AI: Can't connect all knots in patrol path '%s'. Failed Knot %d of %d\n",
								pWorking->GetPtArray()->Name() ? pWorking->GetPtArray()->Name() : "NONAME",
								s_uNumPatrolLinksFound, pPtArray->PointCount());
				}
				else if (s_pSearchQuery->GetStatus() & CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_END)
				{
					DEVPRINTF("AI: Can't connect all knots in patrol path '%s'. Failed on Knot After %d of %d\n",
								pWorking->GetPtArray()->Name() ? pWorking->GetPtArray()->Name() : "NONAME",
								s_uNumPatrolLinksFound, pPtArray->PointCount());
				}
				else
				{
					DEVPRINTF("AI: Can't connect all knots in patrol path '%s'. Failed on Knot %d of %d\n",
								pWorking->GetPtArray()->Name() ? pWorking->GetPtArray()->Name() : "NONAME",
								s_uNumPatrolLinksFound, pPtArray->PointCount());
				}
				pWorking->m_uStage = FAILED;
				s_CookingList.PopHead();
				s_RegisterList.PushHead();
			}
			break;
		case FORWARD_COOKED:
			{
				CNiIterator<CAIPathWaypoint*> it = pWorking->m_AIPath.BeginPathWalk();
				CAIPathWaypoint* pLast = NULL;

				if (it.IsValid())
				{
					pLast = it.Get();
					it.Next(); //skip first!
					while (it.IsValid())
					{
						CAIPathWaypoint* pWay = it.Get();

			
						pWay->m_uWayPointFlags &=~CAIPathWaypoint::WAYPOINTFLAG_STOP_AT;
						pWay->m_uWayPointFlags &=~CAIPathWaypoint::WAYPOINTFLAG_END_OF_PATH;

						if (pLast && pWay->m_Location.DistSq(pLast->m_Location) < 2.0f)
						{
					//		if (pLast->m_pGraphVert && (pLast->m_pGraphVert == pWay->m_pGraphVert))
					//		{
					//			pLast->m_uExitEdgeProps = pWay->m_uExitEdgeProps;  //copy exit edge info before discarding the dup vert
					//		}
							if (pWay->m_pGraphVert)
							{
								*pLast = *pWay;

								CNiIterator<CAIPathWaypoint*> itback = it;
								if (itback.HasBack() && itback.HasBack())
								{
									itback.Back();
									itback.Back();

									CAIPathWaypoint* pLastLast = itback.Get();
									if (pLastLast && pLastLast->m_pGraphVert && pLast && pLast->m_pGraphVert && !(pLast->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH))
									{
										GraphEdge* pE = pLastLast->m_pGraphVert->GetEdgeTo(pWorking->m_AIPath.GetGraph()->GetVertId(pLast->m_pGraphVert));
										if (pE)
										{ 
											pLast->m_ApproachDirUnit.Sub(pLastLast->m_Location, pLast->m_Location);
											if (pLast->m_ApproachDirUnit.SafeUnitAndMag(pLast->m_ApproachDirUnit) > 0.0f)
											{
												pLast->m_uWayPointFlags |= CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH;
												pLast->m_fApproachWidth = pE->m_fHalfWidth;
											}
										}


									}
								}

							}

							it.RemoveThenNext();
							CAIPath::RecycleWayPt(pWay);
						}
						else if (!it.HasNext())
						{  //skip last
							pLast = pWay;
							it.Next();
							continue;
						}
						else
						{
							if (pLast && pLast->m_pGraphVert && pWay && pWay->m_pGraphVert && !(pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH))
							{
								GraphEdge* pE = pLast->m_pGraphVert->GetEdgeTo(pWorking->m_AIPath.GetGraph()->GetVertId(pWay->m_pGraphVert));
								if (pE)
								{ 
									pWay->m_ApproachDirUnit.Sub(pLast->m_Location, pWay->m_Location);
									if (pWay->m_ApproachDirUnit.SafeUnitAndMag(pWay->m_ApproachDirUnit) > 0.0f)
									{
										pWay->m_uWayPointFlags |= CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH;
										pWay->m_fApproachWidth = pE->m_fHalfWidth;
									}
								}


							}

							pLast = pWay;
							it.Next();
						}

					}
					if (pLast)
					{
						pLast->m_uWayPointFlags |= CAIPathWaypoint::WAYPOINTFLAG_END_OF_PATH;
					}
				}
			}
			pWorking->m_uStage = REVERSE_INIT;
			break;
		case REVERSE_COOKED:
			{
				CNiIterator<CAIPathWaypoint*> it = pWorking->m_AIPath.BeginPathWalk(1);
				CAIPathWaypoint* pLast = NULL;
				if (it.IsValid())
				{
					pLast = it.Get();
					it.Next(); //skip first!
					while (it.IsValid())
					{
						CAIPathWaypoint* pWay = it.Get();
						
	
						pWay->m_uWayPointFlags &=~CAIPathWaypoint::WAYPOINTFLAG_STOP_AT;
						pWay->m_uWayPointFlags &=~CAIPathWaypoint::WAYPOINTFLAG_END_OF_PATH;

						if (pLast && pWay->m_Location.DistSq(pLast->m_Location) < 2.0f)
						{
					//		if (pLast->m_pGraphVert && (pLast->m_pGraphVert == pWay->m_pGraphVert))
					//		{
					//			pLast->m_uExitEdgeProps = pWay->m_uExitEdgeProps;  //copy exit edge info before discarding the dup vert
					//		}
							if (pWay->m_pGraphVert)
							{
								*pLast = *pWay;

								CNiIterator<CAIPathWaypoint*> itback = it;
								if (itback.HasBack() && itback.HasBack())
								{
									itback.Back();
									itback.Back();

									CAIPathWaypoint* pLastLast = itback.Get();
									if (pLastLast && pLastLast->m_pGraphVert && pLast && pLast->m_pGraphVert && !(pLast->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH))
									{
										GraphEdge* pE = pLastLast->m_pGraphVert->GetEdgeTo(pWorking->m_AIPath.GetGraph()->GetVertId(pLast->m_pGraphVert));
										if (pE)
										{ 
											pLast->m_ApproachDirUnit.Sub(pLastLast->m_Location, pLast->m_Location);
											if (pLast->m_ApproachDirUnit.SafeUnitAndMag(pLast->m_ApproachDirUnit) > 0.0f)
											{
												pLast->m_uWayPointFlags |= CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH;
												pLast->m_fApproachWidth = pE->m_fHalfWidth;
											}
										}


									}
								}


							}
							it.RemoveThenNext();
							CAIPath::RecycleWayPt(pWay);
						}
						else if (!it.HasNext())
						{  //skip last
							pLast = pWay;
							it.Next();
							continue;
						}
						else
						{
							if (pLast && pLast->m_pGraphVert && pWay && pWay->m_pGraphVert && !(pWay->m_uWayPointFlags & CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH))
							{
								GraphEdge* pE = pLast->m_pGraphVert->GetEdgeTo(pWorking->m_AIPath.GetGraph()->GetVertId(pWay->m_pGraphVert));
								if (pE)
								{ 
									pWay->m_ApproachDirUnit.Sub(pLast->m_Location, pWay->m_Location);
									if (pWay->m_ApproachDirUnit.SafeUnitAndMag(pWay->m_ApproachDirUnit) > 0.0f)
									{
										pWay->m_uWayPointFlags |= CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH;
										pWay->m_fApproachWidth = pE->m_fHalfWidth;
									}
								}
							}
							pLast = pWay;
							it.Next();
						}

					}

					if (pLast)
					{
						pLast->m_uWayPointFlags |= CAIPathWaypoint::WAYPOINTFLAG_END_OF_PATH;
					}
				}

				
				s_CookingList.PopHead();
				s_RegisterList.PushHead();
			}
			break;
		case FAILED:
			FASSERT(0); //why are you in the cookinglist?
			break;
		}
	}
}


BOOL CPatrolPath::Register(CESpline* pPtArrayEntity,
							u8 uMinXZRadius, 
							u8 uMinHeight, 
							BOOL b3DPatrolPath)
{
	if (!IsCooking(pPtArrayEntity->Name()) &&
		!GetRegistered(pPtArrayEntity->Name()))
	{
		CPatrolPath* pPatrolPath = s_pAllocBank->Get();
		if (pPatrolPath)
		{
			pPatrolPath->Init(pPtArrayEntity, uMinXZRadius, uMinHeight, b3DPatrolPath);
			s_CookingList.PushTail(pPatrolPath);
		}
		else
		{	
			DEVPRINTF("AI: Blew the PatrolPathBank\n");
			return FALSE;
		}
	}
	return TRUE;
}



CPatrolPath* CPatrolPath::GetRegistered(cchar* pszName)
{
	CPatrolPath* pFoundPath = NULL;
//	cchar* pszTableName = gstring_Main.FindString(pszName);// Hey, Thought these names would be in the string table!

	if (pszName)
	{
		CNiIterator<CPatrolPath*> it = s_RegisterList.Begin();
		while (!pFoundPath && it.IsValid())
		{
			CPatrolPath* pPatrolPath = it.Get();
			it.Next();
			if (pPatrolPath->GetPtArray()->Name() == pszName)
			{
				pFoundPath = pPatrolPath; 
			}
		}
	}
	return pFoundPath;
}


BOOL CPatrolPath::IsCooking(cchar* pszName)
{
//	cchar* pszTableName = gstring_Main.FindString(pszName);// Hey, Thought these names would be in the string table!

	if (pszName)
	{
		CNiIterator<CPatrolPath*> it = s_CookingList.Begin();
		while (it.IsValid())
		{
			CPatrolPath* pPatrolPath = it.Get();
			it.Next();
			if (pPatrolPath->GetPtArray()->Name() == pszName)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}



void CPatrolPath::DebugRender(void)
{
	CNiIterator<CPatrolPath*> it = s_RegisterList.Begin();
	while (it.IsValid())
	{
		CPatrolPath* pPatrolPath = it.Get();
		it.Next();
		if (pPatrolPath)
		{
			pPatrolPath->GetAIPath()->DebugRender();
		}
	}
}


//
// CPatrolZone
// 	 --used to control very precisely the behavior of a patrolling unit
//
void CPatrolZone::InitFromTripwireBuilder(void)
{
	m_pszLookAtObjName = CTripwireBuilder::m_pszLookAtObjName;
	m_uZoneBehavior =  CTripwireBuilder::m_uZoneBehavior;
	m_uZoneTimeParam = CTripwireBuilder::m_uZoneTimeParam;
	for (u32 i = 0; i < NUM_ZONE_BEHAVIORS; i++)
	{
		if (CTripwireBuilder::m_auZoneToBehaviorSlotMap[i] > 0)
		{
			m_auZoneRandBehaviorParamMin[i] =  CTripwireBuilder::m_auZoneRandBehaviorParamMin[CTripwireBuilder::m_auZoneToBehaviorSlotMap[i]-1];
			m_auZoneRandBehaviorParamMax[i] =  CTripwireBuilder::m_auZoneRandBehaviorParamMax[CTripwireBuilder::m_auZoneToBehaviorSlotMap[i]-1];
			m_auZoneNonRandBehaviorParam[i] =  CTripwireBuilder::m_auZoneNonRandBehaviorParam[CTripwireBuilder::m_auZoneToBehaviorSlotMap[i]-1];
		}
		else
		{
			m_auZoneRandBehaviorParamMin[i] = 0;
			m_auZoneRandBehaviorParamMax[i] = 0;
			m_auZoneNonRandBehaviorParam[i] = 0;
		}
	}
}


void CPatrolZone::OnEnter(CEntity* pEntity)
{
	CAIBrain* pBrain;
	if (!pEntity || !(pBrain = pEntity->AIBrain()))
	{
		return;
	}
	
	if (pBrain->GetCurThought() != CAIBrain::TT_PATROL)
	{
		return;
	}

	if (pBrain->GetCurThought() == CAIBrain::TT_PATROL && 
		pBrain->GetCurThoughtPtr() &&
		pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGENERICPATROL))
	{
		((CGenericPatrol*) pBrain->GetCurThoughtPtr())->EnterZone(this);
	}
}



void CPatrolZone::OnExit(CEntity* pEntity)
{
	CAIBrain* pBrain;
	if (!pEntity || !(pBrain = pEntity->AIBrain()))
	{
		return;
	}
	
	if (pBrain->GetCurThought() != CAIBrain::TT_PATROL)
	{
		return;
	}

	if (pBrain->GetCurThought() == CAIBrain::TT_PATROL && 
		pBrain->GetCurThoughtPtr() &&
		pBrain->GetCurThoughtPtr()->SupportsClassInterface(CLASS_CGENERICPATROL))
	{
		((CGenericPatrol*) pBrain->GetCurThoughtPtr())->ExitZone(this);
	}
}


u8 CPatrolZone::GetSpeed(void)
{
	u8 uVal = m_auZoneRandBehaviorParamMin[ZONESPEC_SPEED_LOG2] + (u8) fmath_RandomChoice(m_auZoneRandBehaviorParamMax[ZONESPEC_SPEED_LOG2] - m_auZoneRandBehaviorParamMin[ZONESPEC_SPEED_LOG2]);
	return uVal;
}


u8 CPatrolZone::GetWaitTime(void)
{
	u8 uVal = m_auZoneRandBehaviorParamMin[ZONESPEC_WAIT_LOG2] + (u8) fmath_RandomChoice(m_auZoneRandBehaviorParamMax[ZONESPEC_WAIT_LOG2] - m_auZoneRandBehaviorParamMin[ZONESPEC_WAIT_LOG2]);
	return uVal;
}

u8 CPatrolZone::GetWaitLookAroundChance(void)
{
	return m_auZoneNonRandBehaviorParam[ZONESPEC_WAIT_LOG2];
}

u8 CPatrolZone::GetLookZoneBehaviorValue(void)
{
	u8 uVal = m_auZoneRandBehaviorParamMin[ZONESPEC_LOOK_LOG2] + (u8) fmath_RandomChoice(m_auZoneRandBehaviorParamMax[ZONESPEC_LOOK_LOG2] - m_auZoneRandBehaviorParamMin[ZONESPEC_LOOK_LOG2]);
	return uVal;
}


u8 CPatrolZone::GetAnimZoneBehaviorValue(void)
{
	u8 uVal = m_auZoneRandBehaviorParamMin[ZONESPEC_ANIM_LOG2] + (u8) fmath_RandomChoice(m_auZoneRandBehaviorParamMax[ZONESPEC_ANIM_LOG2] - m_auZoneRandBehaviorParamMin[ZONESPEC_ANIM_LOG2]);
	return uVal;
}

CEntity* CPatrolZone::GetLookAtObj(void)
{
	if (m_pszLookAtObjName)
	{
		return CEntity::Find(m_pszLookAtObjName);
	}
	return NULL;
}

u8 CPatrolZone::GetTimeParam(void)
{
	return m_uZoneTimeParam;
}
