#include "fang.h"
#include "fdraw.h"  //debug path rendering
#include "fworld_coll.h"
#include "apenew.h"
#include "AIPath.h"
#include "AIGraphSearcher.h"
#ifdef USE_HAZARDS
#include "AIHazard.h"
#include "../Door.h"
#include "../eSwitch.h"
#include "AIGameutils.h"
#include "../meshtypes.h"
#endif

const GraphVert* CNiIterator<const GraphVert*>::s_ReturnError;
const f32 CGraphSearcher::s_kfDefaultSlopeCost = 0.0f;

#define GETNODE(uNodeId) (m_paSearchNodes + (uNodeId))
#define GETNODEID(pNode) ((u16) ((pNode) - m_paSearchNodes))


//
//
//  CSearchNode
//     - Nodes used while CGraphSearcher is doing A* on the graph.
//
//
#define SEARCHNODE_BIT_OPEN					0x0000
#define SEARCHNODE_BIT_CLOSED				0x0001
#define SEARCHNODE_BIT_VISITED				0x0002
#define SEARCHNODE_BIT_NEXTMASK				0xfffc
FCLASS_NOALIGN_PREFIX class CSearchNode
{
public:
	f32 m_h;
	f32 m_g;
	u16 m_uPrevNodeId;
	u16 m_bits;				//used to store status and an index into the scratch array of searchnodes. Index is used to link pending nodes in a priority queue
	FINLINE u16 GetNext(void)				{ return (m_bits & SEARCHNODE_BIT_NEXTMASK)>>2;}
	FINLINE void SetNext(u16 uNext)			{ m_bits = (uNext<<2) | (m_bits & 3);}
	FCLASS_STACKMEM_NOALIGN(CSearchNode);
} FCLASS_NOALIGN_SUFFIX;


FCLASS_NOALIGN_PREFIX class CPQNode
{
public:
	u8 m_uNext;
	u8 m_uPrev;
	u16 m_uData;
	f32 m_fPriority;
	FCLASS_STACKMEM_NOALIGN(CPQNode);
} FCLASS_NOALIGN_SUFFIX;


FCLASS_NOALIGN_PREFIX class CPQ
{
public:
	enum
	{
		NUM_CPQNODES = 100,
	};

	CPQ(void) : m_uHigh(0), m_uLow(0), m_uFreeHead(1)	
	{
		for (u32 i = 1; i < NUM_CPQNODES; i++)
		{
			m_apNodes[i].m_uNext = i+1;
		}
	}


	void RemoveAll(void)
	{
		u8 uTmp = m_uLow;
		u8 uClr;
		while (uTmp)
		{
			uClr = uTmp;
			uTmp = m_apNodes[uTmp].m_uNext;
			m_apNodes[uClr].m_uNext = m_uFreeHead;
			m_uFreeHead	= uClr;
		}

		m_apNodes[0].m_uNext = 0;
		m_apNodes[NUM_CPQNODES-1].m_uNext = 0;
		m_uLow = m_uHigh = 0;
	}


	void Insert(u16 uData, f32 fPriority)
	{
		//get a pq node from the free list
		u8 uNew = m_uFreeHead;
		FASSERT(uNew != 0);
		m_uFreeHead	= m_apNodes[m_uFreeHead].m_uNext;

		//fill with data and priority
		m_apNodes[uNew].m_uData = uData;
		m_apNodes[uNew].m_fPriority = fPriority;
		m_apNodes[uNew].m_uNext = 0;
		m_apNodes[uNew].m_uPrev = 0;

		//hook this into the dbl link list in order
		//ie dbl link list sorted insert
		if (m_uLow == 0)
		{	//empty list, 
			FASSERT(m_uHigh ==0);
			m_uLow = m_uHigh = uNew;
		}
		else
		{
			u8 uCur = m_uLow;
			while (	uCur != 0 &&
					fPriority > m_apNodes[uCur].m_fPriority)
			{
				uCur = m_apNodes[uCur].m_uNext;
			}

			if (uCur == 0)
			{   //add tail
				FASSERT(m_uHigh);
				m_apNodes[m_uHigh].m_uNext = uNew;
				m_apNodes[uNew].m_uPrev = m_uHigh;
				m_uHigh = uNew;
			}
			else if (m_apNodes[uCur].m_uPrev)
			{	//insert before
				m_apNodes[uNew].m_uPrev = m_apNodes[uCur].m_uPrev;
				m_apNodes[uNew].m_uNext = uCur;
				m_apNodes[m_apNodes[uCur].m_uPrev].m_uNext = uNew;
				m_apNodes[uCur].m_uPrev = uNew;
			}
			else
			{	//add head
				FASSERT(m_uLow);
				m_apNodes[uNew].m_uNext = m_uLow;
				m_apNodes[m_uLow].m_uPrev = uNew;
				m_uLow = uNew;
			}
		}
		FASSERT(m_apNodes[m_uLow].m_uPrev == 0 && m_apNodes[m_uHigh].m_uNext == 0);
	}


	u16 PopLow(void)
	{
		if (!m_uLow)
		{
			FASSERT(!m_uHigh);
			return 0;
		}

		u8 uTmp = m_apNodes[m_uLow].m_uNext;
		m_apNodes[m_uLow].m_uNext = m_uFreeHead;
		m_uFreeHead	= m_uLow;
		m_uLow = uTmp;
		m_apNodes[m_uLow].m_uPrev = 0;
		if (m_uHigh == m_uFreeHead)
		{
			m_uHigh = 0;
		}
		FASSERT(m_apNodes[m_uLow].m_uPrev == 0 && m_apNodes[m_uHigh].m_uNext == 0);
		return m_apNodes[m_uFreeHead].m_uData;
	}

	BOOL Remove(u16 uData)
	{
		u8 uCur = m_uLow;
		while (	uCur != 0 &&
				uData != m_apNodes[uCur].m_uData)
		{
			uCur = m_apNodes[uCur].m_uNext;
		}

		if (uCur ==0)
		{
			FASSERT(0);
			return FALSE;
		}

		if (m_apNodes[uCur].m_uPrev)
		{
			m_apNodes[m_apNodes[uCur].m_uPrev].m_uNext = m_apNodes[uCur].m_uNext;
		}
		else
		{
			FASSERT(m_uLow == uCur);
			m_uLow = m_apNodes[uCur].m_uNext;
			m_apNodes[m_uLow].m_uPrev = 0;
		}

		if (m_apNodes[uCur].m_uNext)
		{
			m_apNodes[m_apNodes[uCur].m_uNext].m_uPrev = m_apNodes[uCur].m_uPrev;
		}
		else
		{
			FASSERT(m_uHigh == uCur);
			m_uHigh = m_apNodes[uCur].m_uPrev;
			m_apNodes[m_uHigh].m_uNext = 0;
		}
		FASSERT(m_apNodes[m_uLow].m_uPrev == 0 && m_apNodes[m_uHigh].m_uNext == 0);

		m_apNodes[uCur].m_uNext = m_uFreeHead;
		m_uFreeHead	= uCur;
		return TRUE;
	}

	BOOL IsEmpty(void)
	{
		return !m_uLow;
	}

	CPQNode m_apNodes[NUM_CPQNODES];
	u8 m_uLow;
	u8 m_uHigh;
	u8 m_uFreeHead;
	
	FCLASS_STACKMEM_NOALIGN(CPQ);
} FCLASS_NOALIGN_SUFFIX;

//
//
// CSearchResults
//
//

CSearchResults::CSearchResults(void)
{
	Clear();
}

#if AIGRAPH_EDITOR_ENABLED

CSearchResults::CSearchResults(FLinkRoot_t* pVertPathNodePool)
: m_VertPath(pVertPathNodePool)
{
	Clear();
}
#endif

CSearchResults::~CSearchResults(void)
{
}


void CSearchResults::Clear(void)
{
	m_uResultsFlags = RESULTSFLAG_NONE;
	m_Path.Reset();

#if AIGRAPH_EDITOR_ENABLED
	m_VertPath.RemoveAll();
	m_fPathLength = 0.0f;
	m_EntryShortcut_WS.Zero();
	m_ExitShortcut_WS.Zero();
#endif

#if AIGRAPH_DEBUG
	m_fCookTime = 0;
	m_fFiniTime = 0;
	m_nCookLoops = 0;
#endif
}


#if AIGRAPH_EDITOR_ENABLED

void CSearchResults::Render(void)
{
	CNiIterator<const GraphVert*> it = m_VertPath.Begin();

	CFVec3 _lastVertLoc;
	s32 count = 0;
	while (it.IsValid())
	{
		const GraphVert *pV = it.Get();
		it.Next();
		CFColorRGBA _rgb = FColor_MotifRed;
		
/*
		if (pV == _CurGoalVert)
		{
			_rgb = FColor_MotifGreen;
		}
		else if (pV == _CurStartVert)
		{
			_rgb = FColor_MotifBlue;
		}
*/

		fdraw_FacetedWireSphere( &(pV->GetLocation().v3), 2.0f, 2, 2 , &_rgb);

		if (count++>0)
			fdraw_SolidLine(&(pV->GetLocation().v3), &(_lastVertLoc), &_rgb);
		_lastVertLoc = pV->GetLocation().v3;
	}
}

#endif //#if AIGRAPH_EDITOR_ENABLED


//
//
// CSearchQuery
//
//

CSearchQuery::CSearchQuery(void)
{
	Clear();
}


CSearchQuery::~CSearchQuery(void)
{
}


//called by CGraphSearcher whenever a new path is requested using this CSearchQuery Object
void CSearchQuery::Clear(void)
{
	m_fQueryWidth = 0.0f;
	m_fQueryHeight = 0.0f;
	m_fSlopeCost = 0.0f;
	m_fAvoidCost = 0.0f;
	m_fMaxDestinationFudge = 0.0f;
	m_uStatus = 0;
	m_uParams = 0;
	m_uKnownStartVertId = 0;
	m_uKnownGoalVertId = 0;
	m_StartLoc_WS.Zero();
	m_GoalLoc_WS.Zero();
	m_AvoidPt_WS.Zero();
	m_pResults = NULL;
	m_pAvoidVert = NULL;
	m_fMaxJumpDist = 0.0f;
}



//
//
//	 class GraphSearcher 
// 		- weighted, directed graph based pathfinder using A* algorithm
//		- graph structure is defined by CAIGraph class
//
//

CGraphSearcher::CGraphSearcher(void)
 :	m_nMaxLoopsPerSession(200),
	m_pCurSearch(NULL),
	m_pStartNode(NULL),
	m_pGoalNode(NULL),
	m_pGraph(NULL),
	m_paSearchNodes(NULL),
	m_bHazardsResolved(FALSE),
	m_nNumSearchNodes(0),
	m_pCPQ(NULL)
{
}


void CGraphSearcher::UsePNodePool(FLinkRoot_t* pNodePool)
{
	m_RequestQueue.SetNodePool(pNodePool);
}



CGraphSearcher::~CGraphSearcher(void)
{
	m_RequestQueue.RemoveAll();
	APE_ARRAYDELETE(m_paSearchNodes); m_paSearchNodes = NULL;
	APE_DELETE(m_pCPQ); m_pCPQ = NULL;
}


BOOL CGraphSearcher::BindToGraph(CAIGraph *pGraph)
{
	m_pGraph = pGraph;

	APE_ARRAYDELETE(m_paSearchNodes); m_paSearchNodes = NULL;
	APE_DELETE(m_pCPQ); m_pCPQ = NULL;

	if (m_pGraph)
	{
		m_nNumSearchNodes = m_pGraph->GetNumVerts();
		if (m_nNumSearchNodes > 0)
		{
			m_paSearchNodes = APE_NEW(CSearchNode[m_nNumSearchNodes]);
		}
	}

	m_pCPQ = APE_NEW CPQ();

	return m_pCPQ && (!m_nNumSearchNodes || m_paSearchNodes);
}


void CGraphSearcher::ResolveHazardPtrs(void)
{
#ifdef USE_HAZARDS
	if (!m_pGraph || m_bHazardsResolved)
		return;

	m_bHazardsResolved = TRUE;
	for (u16 i = 0; i < m_pGraph->GetNumVerts(); i++)
	{
		GraphVert* pV = m_pGraph->GetVert(i);

		if (!m_pGraph->IsVertValid(i) || pV->Is3D())	//3D edges can't have hazard info
			continue;


		for (u8 e = 0; e < pV->GetNumEdges(); e++)
		{
			FASSERT(e < AIGRAPH_NUM_EDGES_PER_VERT);
			GraphEdge* pE = pV->GetEdge(e);

			//RESOLVE first: ie. Can we find a door or lift at all?
			if (pE->m_nProps & EDGEPROP_HAZARD_DOOR)
			{
				CFVec3A EdgeMid;
				EdgeMid.Add(pV->GetLocation(), pV->GetOppVert(e, m_pGraph)->GetLocation());
				EdgeMid.Mul(0.5f);

				CDoorEntity *pDoor = CDoorEntity::AI_FindWithNearestPad(EdgeMid, CDoorEntity::USERTYPE_DOOR);
				if (pDoor)
				{
					pE->m_nHazardId = m_DoorHazardReg.Register((void*) pDoor);
				}
				else
				{
					DEVPRINTF("Can't find door anywhere near door edge at (%d, %d, %d)\n", (s32)EdgeMid.x, (s32)EdgeMid.y, (s32)EdgeMid.z);
				}
			}
			else if (pE->m_nProps & EDGEPROP_HAZARD_LIFT)
			{
				CDoorEntity *pLift = CDoorEntity::AI_FindWithNearestPad(pV->GetLocation(), CDoorEntity::USERTYPE_LIFT);
				//note! this assumes that the nearest pad lift goes to pVB.  Might want to assert that someday.
				if (pLift)
				{
					pE->m_nHazardId = m_DoorHazardReg.Register((void*) pLift);
				}
				else
				{
					DEVPRINTF("Can't find a lift anywhere near graph vert at (%d, %d, %d)\n", (s32)pV->GetLocation().x, (s32)pV->GetLocation().y, (s32)pV->GetLocation().z);
				}
			}

			//check lift for a switch to open it from this direction if it is shut
			CDoorEntity *pSwitchDoor;
			if (((pE->m_nProps & EDGEPROP_HAZARD_DOOR) || (pE->m_nProps & EDGEPROP_HAZARD_LIFT)) &&
				 (pSwitchDoor = (CDoorEntity *) m_DoorHazardReg.GetRegistered(pE->m_nHazardId)))
			{
				BOOL bCanSeeSwitch = FALSE;
				if (pSwitchDoor)
				{
					BOOL bClosest = 0;
					for (s32 ii = 0; ii < 2; ii++)
					{	//doors can have two switches, check for both for one that will work in the direction we are traveling!

						CESwitch* pSwitch =	 (CESwitch*) pSwitchDoor->AI_GetSwitch(ii);
						if (pSwitch && pSwitch->TypeBits() & ENTITY_BIT_SWITCH)
						{  //check LOS between the switch and the src vert.

							CFVec3A LookAtLoc;
							LookAtLoc = pV->GetLocation();
							LookAtLoc.v3 -= pSwitch->GetBoundingSphere_WS().m_Pos;
							LookAtLoc.Unitize();
							LookAtLoc.Mul(pSwitch->GetBoundingSphere_WS().m_fRadius*0.75f);
							LookAtLoc.v3 += pSwitch->GetBoundingSphere_WS().m_Pos;

							//Some Trackers to ignore in the LOS test.
							FWorld_nTrackerSkipListCount = 0;
							pSwitch->AppendTrackerSkipList();
							if (!fworld_IsLineOfSightObstructed( &(pV->GetLocation()), &LookAtLoc, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList))
							{
								//we have a hit
								if (bCanSeeSwitch)
								{
									FASSERT(ii = 1);
									//we can see both switches.  IS this one closer?
									LookAtLoc.Set(((CESwitch*) pSwitchDoor->AI_GetSwitch(0))->GetBoundingSphere_WS().m_Pos);
									f32 fDistToSwitch0Sq = LookAtLoc.DistSq(pV->GetLocation());

									LookAtLoc.Set(pSwitch->GetBoundingSphere_WS().m_Pos);
									if (LookAtLoc.DistSq(pV->GetLocation()) > fDistToSwitch0Sq)
									{
										continue;
									}
								}
								bCanSeeSwitch = TRUE;
								if (ii == 1)
								{
								   pE->m_nProps |= EDGEPROP_HAZARD_USE_SWITCH1;		//flip this bit if the edge in this direction uses switch1, otherwise, it is to be assumed to use switch0
								}
								else
								{
								   pE->m_nProps &= ~EDGEPROP_HAZARD_USE_SWITCH1;		//flip this bit if the edge in this direction uses switch1, otherwise, it is to be assumed to use switch0
								}
								pE->m_nProps |= EDGEPROP_HAZARD_HAS_SWITCH;	 //A switch exists to open the door from this side
							}
							aiutils_DebugTrackRay(pV->GetLocation(), LookAtLoc, bCanSeeSwitch);
						}
					}
				}
			}

			//Now decide wether this link can be traverse based on being able to find the door, and a switch in this direction?
			if (pE->m_nProps & EDGEPROP_HAZARD_DOOR && pE->m_nHazardId)
			{
				CDoorEntity *pDoor = (CDoorEntity *) m_DoorHazardReg.GetRegistered(pE->m_nHazardId);
				if (pDoor)
				{
					CFVec3A vecNodeMinusDoor = pV->GetLocation();
					vecNodeMinusDoor.Sub(pDoor->MtxToWorld()->m_vPos);
					vecNodeMinusDoor.Unitize();

					f32 fDot = vecNodeMinusDoor.Dot(pDoor->MtxToWorld()->m_vZ);

					if(fDot < 0.0f)
					{
						// We're in front.
						// Check to see if this door won't open from this direction.
						if ((pDoor->GetBehavior() != CDoorEntity::DOOR_BEHAVIOR_OTHER) &&
							(pDoor->GetBehavior() != CDoorEntity::DOOR_BEHAVIOR_DOOR) &&			 //pgm: I think DOOR_BEHAVIOR_DOOR this means it opens automaticaly from both sides
							!(pDoor->m_uBehaviorCtrlFlags & CDoorEntity::DOOR_BEHAVIOR_CTRL_OPENFRONT) &&
							!(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
						{
							pE->m_nHazardId = 0;
						}

 						if (!(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
						{
							if (pDoor->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_OTHER &&
								(pDoor->m_uBehaviorCtrlFlags & CDoorEntity::DOOR_BEHAVIOR_CTRL_OPENFRONT))
							{
								pE->m_nProps |= EDGEPROP_HAZARD_DOOR_FRONT;
							}
							else
							{
								pE->m_nProps &= ~EDGEPROP_HAZARD_DOOR_FRONT;
							}
						}
					}
					else
					{
						// We're in back.
						// Check to see if this door won't open from this direction.
						if ((pDoor->GetBehavior() != CDoorEntity::DOOR_BEHAVIOR_OTHER) &&
							(pDoor->GetBehavior() != CDoorEntity::DOOR_BEHAVIOR_DOOR) &&
							!(pDoor->m_uBehaviorCtrlFlags & CDoorEntity::DOOR_BEHAVIOR_CTRL_OPENBACK)	&&
							!(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
						{
							pE->m_nHazardId = 0;
						}

 						if (!(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
						{
							pE->m_nProps &= ~EDGEPROP_HAZARD_DOOR_FRONT;
						}
					}
				}
			}
			else if (pE->m_nProps & EDGEPROP_HAZARD_LIFT)
			{
				CDoorEntity *pLift = (CDoorEntity *) m_DoorHazardReg.GetRegistered(pE->m_nHazardId);
				//note! this assumes that the nearest pad lift goes to pVB.  Might want to assert that someday.
				if (pLift)
				{
					if (pLift->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_OSCILLATE)
					{ //oscilating platform, always link it up, rely on pathfollower to use it properly
					}
					else if (pLift->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_DOOR  &&
							 !(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
					{ //one way
						CFVec3A MovePos;
						MovePos = pLift->AI_GetInitPos().m_vPos;
						MovePos.Add(pLift->AI_GetMoveVec());
						f32 fDistSqMovePos = MovePos.DistSq(pV->GetLocation());
						f32 fDistSqInitPos = pLift->AI_GetInitPos().m_vPos.DistSq(pV->GetLocation());
					//	if ( MovePos.DistSq(pV->GetLocation()) < pLift->AI_GetInitPos().m_vPos.DistSq(pV->GetLocation()))	//findfix: this doesn't work if inlines are enabled
						if ( fDistSqMovePos < fDistSqInitPos )
						{	//break the link, this is one way, and the closest position is the MovePos
							pE->m_nHazardId = 0;   	 //make a hazard edge with a hazard ID = 0
						}
						else
						{  //all good from this direction
						}
					}
					else if (pLift->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_ELEVATOR)
					{  //two way  (If we find the lift, assume that it is the right one!
						if((pLift->AI_GetUnitTargetPos(pV->m_Location) == 0.0f) && (pLift->m_uBehaviorCtrlFlags & CDoorEntity::DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL1) && !(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
						{
							pE->m_nHazardId = 0;   	 //make a hazard edge with a hazard ID = 0
						}
						if((pLift->AI_GetUnitTargetPos(pV->m_Location) == 1.0f) && (pLift->m_uBehaviorCtrlFlags & CDoorEntity::DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL0) && !(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
						{
							pE->m_nHazardId = 0;   	 //make a hazard edge with a hazard ID = 0
						}
					}
					else if (pLift->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_OTHER)
					{  //two way  (If we find the lift, assume that it is the right one!
						if (!(pE->m_nProps & EDGEPROP_HAZARD_HAS_SWITCH))
						{
							pE->m_nHazardId = 0;   	 //make a hazard edge with a hazard ID = 0
						}
					}
				}
			}


		}
	}
#endif
}


void CGraphSearcher::SetMaxLoopsPerSession(s32 nMaxLoops)
{
	m_nMaxLoopsPerSession = nMaxLoops;
}


void CGraphSearcher::SubmitQuery(CSearchQuery* pNewSearch)
{
	if (!(m_pGraph && pNewSearch))	// must have a graph and a query, or there is no reason to call this function
	{
		return;
	}

	FASSERT(!m_RequestQueue.Find(pNewSearch));

	//  Add to request queue
	m_RequestQueue.PushTail(pNewSearch);
	pNewSearch->m_uStatus = CSearchQuery::SEARCHSTATUS_PENDING;
}


const GraphVert* CGraphSearcher::_FindStartVert(CAIGraph* pGraph, CSearchQuery* pSearch)
{
	const GraphVert* pStartGraphVert = NULL;

	if (pSearch->m_uParams & CSearchQuery::SEARCHPARAM_3D)
	{
		pStartGraphVert = pGraph->FindClosestLOSVert3D(pSearch->m_StartLoc_WS);
	}
	else
	{
		pStartGraphVert = pGraph->FindClosestLOSVert2D(pSearch->m_StartLoc_WS);
	}
	return pStartGraphVert;
}


const GraphVert* CGraphSearcher::_FindGoalVert(CAIGraph* pGraph, CSearchQuery* pSearch, const GraphVert* pStartGraphVert)
{
	const GraphVert* pGoalGraphVert = NULL;
	CFVec3A FudgeGoal;
	BOOL bDidFudgeGoal = FALSE;
	CFSphereA tmpSphere;

	if (pSearch->m_uParams & CSearchQuery::SEARCHPARAM_3D)
	{
		if (pSearch->GetParams() & CSearchQuery::SEARCHPARAM_FUDGE_DEST)
		{
			pGoalGraphVert = pGraph->FindClosestLOSVert3D_AdjustWithinRange(&(pSearch->m_GoalLoc_WS), pSearch->m_fMaxDestinationFudge);	//true means goal must be "in the graph"
		}
		else
		{
			pGoalGraphVert = pGraph->FindClosestLOSVert3D(pSearch->m_GoalLoc_WS, TRUE);	//true means goal must be "in the graph"
		}


//Do volume checking on goal only
		if (pGoalGraphVert)
		{
			if (pGraph->IsPtIn3DVertVol(pGoalGraphVert, pSearch->m_GoalLoc_WS))
			{
				if (pSearch->m_fQueryWidth < pGoalGraphVert->m_fSafeRad)
				{
					f32 fDistXZ; 
					CFVec3A Norm;
					Norm.Sub(pSearch->m_GoalLoc_WS,	pGoalGraphVert->m_Location);
					Norm.y = 0.0f;
					if ((fDistXZ = Norm.SafeUnitAndMagXZ(Norm)) > 0.0f)
					{
						if (pGoalGraphVert->m_fSafeRad - fDistXZ < (pSearch->m_fQueryWidth-1.0f))  //1 foott off graph is allowed.
						{
							BOOL bSafelyInEdge = FALSE;

							for (u32 j = 0; j < pGoalGraphVert->GetNumEdges(); j++)
							{
								tmpSphere.Set(pSearch->m_GoalLoc_WS,pSearch->m_fQueryWidth);
								if (pGraph->IsSphereSafelyInside3DEdgeVol(pGoalGraphVert,
																	pGoalGraphVert->GetEdge(j),
																	tmpSphere,
																	NULL,
																	NULL))
								{

									bSafelyInEdge = TRUE;
									break;
								}
							}
  

							if (!bSafelyInEdge)
							{
								//prob
								if ( pSearch->GetParams() & CSearchQuery::SEARCHPARAM_FUDGE_DEST)
								{	//can fix, so do
									Norm.Mul(pGoalGraphVert->m_fSafeRad - pSearch->m_fQueryWidth);
									Norm.Add(pGoalGraphVert->m_Location);
									pSearch->m_GoalLoc_WS = Norm;
								}
								else
								{
									//screwed
									pGoalGraphVert = NULL;
								}
							}
						}
						else
						{
							//cool.
						}
					}
				}
				else
				{
					//screwed
					pGoalGraphVert = NULL;
				}
			}
			else
			{	 //not in a vert, must be in an edge
				BOOL bSafelyInEdge = FALSE;

				for (u32 j = 0; j < pGoalGraphVert->GetNumEdges(); j++)
				{
					if (pGraph->IsPtIn3DEdgeVol(pGoalGraphVert,
												pGoalGraphVert->GetEdge(j),
												pSearch->m_GoalLoc_WS))
					{
						bDidFudgeGoal = FALSE;
						tmpSphere.Set(pSearch->m_GoalLoc_WS, pSearch->m_fQueryWidth);
						if (pGraph->IsSphereSafelyInside3DEdgeVol(pGoalGraphVert,
														pGoalGraphVert->GetEdge(j),
														tmpSphere,
														&FudgeGoal,
														&bDidFudgeGoal))
						{
							//cool.
							bSafelyInEdge = TRUE;
							break;
						}
						else
						{
						   if ((pSearch->GetParams() & CSearchQuery::SEARCHPARAM_FUDGE_DEST) && bDidFudgeGoal)
						   {
								pSearch->m_GoalLoc_WS = FudgeGoal;
								bSafelyInEdge = TRUE;
								break;
						   }
						}
					}
				}
				if (!bSafelyInEdge)
				{
					//screwed
					pGoalGraphVert = NULL;
				}
			}
		}


	}
	else
	{
		if (pSearch->GetParams() & CSearchQuery::SEARCHPARAM_FUDGE_DEST)
		{
			pGoalGraphVert = pGraph->FindClosestLOSVert2D_AdjustWithinRange(&pSearch->m_GoalLoc_WS, pSearch->m_fMaxDestinationFudge);
		}
		else
		{
			pGoalGraphVert = pGraph->FindClosestLOSVert2D(pSearch->m_GoalLoc_WS, TRUE);	//true means goal must be "in the graph"
		}


		//Do volume checking on goal only
		if (pGoalGraphVert)
		{
			if (pGraph->IsPtIn2DVertVol(pGoalGraphVert, pSearch->m_GoalLoc_WS, TRUE))
			{
				if (pSearch->m_fQueryWidth < pGoalGraphVert->m_fSafeRad)
				{
					f32 fDistXZ; 
					CFVec3A Norm;
					Norm.Sub(pSearch->m_GoalLoc_WS,	pGoalGraphVert->m_Location);
					Norm.y = 0.0f;
					if ((fDistXZ = Norm.SafeUnitAndMagXZ(Norm)) > 0.0f)
					{
						if (pGoalGraphVert->m_fSafeRad - fDistXZ < (pSearch->m_fQueryWidth-1.0f))  //1 foott off graph is allowed.
						{
							BOOL bSafelyInEdge = FALSE;

							for (u32 j = 0; j < pGoalGraphVert->GetNumEdges(); j++)
							{
								tmpSphere.Set(pSearch->m_GoalLoc_WS,pSearch->m_fQueryWidth);
								if (pGraph->IsCylnSafelyInside2DEdgeVol(pGoalGraphVert,
																	pGoalGraphVert->GetEdge(j),
																	tmpSphere,
																	pSearch->m_fQueryHeight, 
																	TRUE,
																	NULL,
																	NULL))
								{

									bSafelyInEdge = TRUE;
									break;
								}
							}


							if (!bSafelyInEdge)
							{
								//prob
								if ( pSearch->GetParams() & CSearchQuery::SEARCHPARAM_FUDGE_DEST)
								{	//can fix, so do
									Norm.Mul(pGoalGraphVert->m_fSafeRad - pSearch->m_fQueryWidth);
									Norm.Add(pGoalGraphVert->m_Location);
									pSearch->m_GoalLoc_WS = Norm;
								}
								else
								{
									//screwed
									pGoalGraphVert = NULL;
								}
							}
						}
						else
						{
							//cool.
						}
					}
				}
				else
				{
					//screwed
					pGoalGraphVert = NULL;
				}
			}
			else
			{	 //not in a vert, must be in an edge
				BOOL bSafelyInEdge = FALSE;

				for (u32 j = 0; j < pGoalGraphVert->GetNumEdges(); j++)
				{
					if (pGraph->IsPtIn2DEdgeVol(pGoalGraphVert,
												pGoalGraphVert->GetEdge(j),
												pSearch->m_GoalLoc_WS,
												TRUE))
					{
						bDidFudgeGoal = FALSE;
						tmpSphere.Set(pSearch->m_GoalLoc_WS,pSearch->m_fQueryWidth);
						if (pGraph->IsCylnSafelyInside2DEdgeVol(pGoalGraphVert,
														pGoalGraphVert->GetEdge(j),
														tmpSphere,
														pSearch->m_fQueryHeight, 
														TRUE,
														&FudgeGoal,
														&bDidFudgeGoal))
						{
							//cool.
							bSafelyInEdge = TRUE;
							break;
						}
						else
						{
						   if ((pSearch->GetParams() & CSearchQuery::SEARCHPARAM_FUDGE_DEST) && bDidFudgeGoal)
						   {
								pSearch->m_GoalLoc_WS = FudgeGoal;
								bSafelyInEdge = TRUE;
								break;
						   }
						}
					}
				}
				if (!bSafelyInEdge)
				{
					//screwed
					pGoalGraphVert = NULL;
				}
			}
		}
	}

	return pGoalGraphVert;
}


int uCount =0;
BOOL CGraphSearcher::BeginSearch(void)
{
	const GraphVert* pStartGraphVert = NULL;
	FASSERT(m_pCurSearch == NULL);
	
	if (m_RequestQueue.Size())
	{
		m_pCurSearch = m_RequestQueue.PopHead();
		uCount ++;

		pStartGraphVert = _FindStartVert(m_pGraph, m_pCurSearch);
		if (pStartGraphVert)
		{	//no need to look for a goal, if there isn't a start
			m_pGoalVert = _FindGoalVert(m_pGraph, m_pCurSearch, pStartGraphVert);
		}
		
		if (pStartGraphVert && m_pGoalVert)	//true means goal must be "in the graph"
		{
			if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_AVOIDPT_PATH)
			{
				if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_3D)
				{
					m_pCurSearch->m_pAvoidVert = m_pGraph->FindClosestLOSVert3D(m_pCurSearch->m_AvoidPt_WS, TRUE);	//true means goal must be "in the graph"
				}
				else
				{
					m_pCurSearch->m_pAvoidVert = m_pGraph->FindClosestLOSVert2D(m_pCurSearch->m_AvoidPt_WS, TRUE);	//true means goal must be "in the graph"
				}
			}

			// Start working on a new search
			FASSERT(m_pCurSearch->m_uStatus & CSearchQuery::SEARCHSTATUS_PENDING);
			m_pCurSearch->m_uStatus &=~CSearchQuery::SEARCHSTATUS_PENDING;
			m_pCurSearch->m_uStatus |=CSearchQuery::SEARCHSTATUS_ROUTING;

			m_pCPQ->RemoveAll();

			//start
			m_pStartNode = &(m_paSearchNodes[m_pGraph->GetVertId(pStartGraphVert)]);

			//goal
			m_pGoalNode = &(m_paSearchNodes[m_pGraph->GetVertId(m_pGoalVert)]);

			if (m_pStartNode == m_pGoalNode)
			{   //clear only one node, since we know search will complete after one iteration.
				m_pStartNode->m_uPrevNodeId = 0;
				m_pStartNode->m_bits = 0;
				m_pStartNode->m_g = 0.0f;
				m_pStartNode->m_h = 0.0f;
			}
			else
			{   //clear the whole scratch 
				fang_MemZero(m_paSearchNodes, sizeof(CSearchNode)*m_pGraph->GetNumVerts());
			}

			m_fCurSearchPathLength = 0.0f;

			FASSERT(m_pStartNode->m_g == 0.0f && m_pStartNode->m_bits==0); //actual cost to search to this point
			m_pStartNode->m_h = GetEstimatedCostToGoal(pStartGraphVert);
			m_pCPQ->Insert(m_pStartNode - m_paSearchNodes, m_pStartNode->m_g + m_pStartNode->m_h);
		}
		else
		{
			FASSERT(m_pCurSearch->m_uStatus & CSearchQuery::SEARCHSTATUS_PENDING);
			m_pCurSearch->m_uStatus &=~CSearchQuery::SEARCHSTATUS_PENDING;
			m_pCurSearch->m_uStatus |= CSearchQuery::SEARCHSTATUS_FAILED;
			m_pCurSearch->m_uStatus |= (!pStartGraphVert) * CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_START;
			m_pCurSearch->m_uStatus |= (!m_pGoalVert) * CSearchQuery::SEARCHSTATUS_FAILED_REASON_BAD_END;
			m_pCurSearch = NULL;
		}
	}

	return m_pCurSearch != NULL;
}


void CGraphSearcher::Work(s32* pnLoopCounter)
{
	//blich.  this really shouldn't go here, but until later it will do
	if (!m_bHazardsResolved)
	{
		ResolveHazardPtrs();
	}

	GraphVert* pV;
	FASSERT(m_pGraph); // must have a graph for searcher to work atall.
	FASSERT(m_pGraph->GetNumVerts() <= m_nNumSearchNodes);

	if (!m_pCurSearch && !(BeginSearch()))
	{
		return;	//no work to do this time
	}
	FASSERT(m_pCurSearch);

	m_CurSearchTimer.Reset();

	CSearchNode* pBest = NULL; 
	u16 uBest = 0;
	while ( (*pnLoopCounter)++ < m_nMaxLoopsPerSession &&
			((pBest = (m_paSearchNodes+m_pCPQ->PopLow())) != m_paSearchNodes))
	{
		FASSERT(pBest);
		if (pBest == m_pGoalNode)
		{
			break;
		}

		pBest->m_bits |= SEARCHNODE_BIT_CLOSED;
		pV = m_pGraph->GetVert(GETNODEID(pBest));
		u8 nNumEdges = pV->GetNumEdges();
		for (u8 e = 0; e < nNumEdges; e++)
		{
			GraphEdge* pEdge = pV->GetEdge(e);
			u16 uNextId = pEdge->m_nNextVertId;
			CSearchNode* pNextSearchNode = GETNODE(uNextId);
			const GraphVert* pNextSearchVert = m_pGraph->GetVert(uNextId);

			if (pNextSearchNode->m_bits & SEARCHNODE_BIT_CLOSED)
			{
				continue;
			}

			if (pEdge->m_fHalfWidth < m_pCurSearch->m_fQueryWidth) 
			{
				continue; //too wide for path. ignore it
			}

			if (pEdge->Is3D())
			{  //3D passability conditions
				if (!(m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_3D))
				{
					//block.
					continue;
				}
			}
			else 
			{	//2D passability conditions
				if (pEdge->m_fMaxSafeHeight < m_pCurSearch->m_fQueryHeight)
				{
					continue; //too tall for a path. ignore it
				}

				if ((pEdge->m_nProps & EDGEPROP_JUMP) && 
					((m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_NO_JUMPS) ||
					 (((pEdge->m_nProps & EDGEPROP_JUMP) == JUMPTYPE_VERT_2_VERT) &&
					  (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_LIMIT_JUMP_DIST) &&
					  pEdge->GetLength() >= m_pCurSearch->m_fMaxJumpDist)))
				{
					continue; //can't use that jump edge, ignore it
				}
				if (pEdge->m_nProps & EDGEPROP_DOWN_ONLY)
				{ 
					if (pNextSearchVert->GetLocation().y > pV->GetLocation().y)
					{
						continue;  //can't use that edge
					}
				}
				if (pEdge->m_nProps & EDGEPROP_UP_ONLY)
				{ 
					if (pNextSearchVert->GetLocation().y < pV->GetLocation().y)
					{
						continue;  //can't use that edge
					}
				}
				if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_AVOIDPT_PATH && pNextSearchVert == m_pCurSearch->m_pAvoidVert)
				{
					continue;  //can't use that drop edge, ignore it
				}
#ifdef USE_HAZARDS
				if (((pEdge->m_nProps & EDGEPROP_HAZARD_DOOR) || (pEdge->m_nProps & EDGEPROP_HAZARD_LIFT)))
				{
					CDoorEntity* pDoor = (CDoorEntity*) m_DoorHazardReg.GetRegistered(pEdge->m_nHazardId);
					u32 uSwitchNum = 0;
					uSwitchNum = (pEdge->m_nProps & EDGEPROP_HAZARD_USE_SWITCH1)!=0;
					if (pDoor &&
						(pDoor->AI_IsOpen(pV->GetLocation()) || 	//let's hope it stays open
						 (!pDoor->IsLocked() &&
						  ((pDoor->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_DOOR) ||
						  ((pDoor->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_OTHER) && (pEdge->m_nProps & EDGEPROP_HAZARD_DOOR_FRONT) && (pDoor->m_uBehaviorCtrlFlags & CDoorEntity::DOOR_BEHAVIOR_CTRL_OPENFRONT)) ||
						  ((pDoor->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_OTHER) && !(pEdge->m_nProps & EDGEPROP_HAZARD_DOOR_FRONT) && (pDoor->m_uBehaviorCtrlFlags & CDoorEntity::DOOR_BEHAVIOR_CTRL_OPENBACK)))) || //this door will open automatically from this direction, And, it isn't locked.
						 (pDoor->GetBehavior() == CDoorEntity::DOOR_BEHAVIOR_ELEVATOR && !pDoor->IsLocked()) ||
						 ((pEdge->m_nProps &  EDGEPROP_HAZARD_HAS_SWITCH) &&   //this door has a nearby switch that will open it
						  (CESwitch*) pDoor->AI_GetSwitch(uSwitchNum) && 					  //note to self.  I guess I could put these switch validity checks up in resolve hazards!
						  ((CESwitch*) pDoor->AI_GetSwitch(uSwitchNum))->CanOpenDoor(pDoor))))  
					{
					   //the door will work.
					}
					else
					{
						continue;  //can't pass through door
					}
				}
#endif		 
			}


			f32 fCostFactor = GetCostOfEdge(pV, pEdge);
			FASSERT(fCostFactor > 0.0f);
			if (!(pNextSearchNode->m_bits & SEARCHNODE_BIT_VISITED))
			{
				pNextSearchNode->m_bits |= SEARCHNODE_BIT_VISITED;
				//set these fields as we need to, instead of all at once
				pNextSearchNode->m_h = GetEstimatedCostToGoal(pNextSearchVert);
			}
			else //(pNextSearchNode->m_bits & SEARCHNODE_BIT_OPEN)
			{
				if (pNextSearchNode->m_g > pBest->m_g+fCostFactor)
				{
					//remove it since we have improved data for it.  
					//re-add to pq momentarily
					m_pCPQ->Remove(uNextId);//pNextSearchNode); //findfix: NonIntrusive list slows this removal down.....
				}
				else
				{
					pNextSearchNode = NULL;
				}
			}
			if (pNextSearchNode)
			{
				pNextSearchNode->m_uPrevNodeId = GETNODEID(pBest);
				pNextSearchNode->m_g = pBest->m_g+fCostFactor;
				m_pCPQ->Insert(uNextId, pNextSearchNode->m_g + pNextSearchNode->m_h);
			}
		}
	}

	FASSERT(m_pCurSearch);
	BOOL bSearchComplete =  FALSE;
	if (pBest == m_pGoalNode)
	{	///////  Path Found  ///////
		bSearchComplete = TRUE;
#if AIGRAPH_DEBUG
		m_pCurSearch->m_pResults->m_fFiniTime = m_CurSearchTimer.SampleSeconds(FALSE)*1000.0f;	  //time this function
#endif
		FinishSearch();

#if AIGRAPH_DEBUG
		m_pCurSearch->m_pResults->m_fFiniTime = m_CurSearchTimer.SampleSeconds(FALSE)*1000.0f-m_pCurSearch->m_pResults->m_fFiniTime;
#endif
	}
	else if (m_pCPQ->IsEmpty())
	{	//no path found
		m_pCurSearch->m_pResults->m_fCookTime += m_CurSearchTimer.SampleSeconds(FALSE)*1000.0f;	 //ms
		bSearchComplete = TRUE;
		FASSERT(m_pCurSearch->m_uStatus & CSearchQuery::SEARCHSTATUS_ROUTING);
		m_pCurSearch->m_uStatus &= ~CSearchQuery::SEARCHSTATUS_ROUTING;
		m_pCurSearch->m_uStatus |= CSearchQuery::SEARCHSTATUS_FAILED;
	}
	else 
	{	//keep working next time
	}

	FASSERT(m_pCurSearch);
#if AIGRAPH_DEBUG
	m_pCurSearch->m_pResults->m_fCookTime += m_CurSearchTimer.SampleSeconds(FALSE)*1000.0f;
	m_pCurSearch->m_pResults->m_nCookLoops += *pnLoopCounter;
#endif
	if (bSearchComplete)
	{
		m_pCurSearch = NULL;
	}
}


BOOL CGraphSearcher::_CalcAndStoreEntryShortcut(const GraphVert* pFirstVert, const GraphVert* pAfterFirst)
{
	FASSERT(m_pCurSearch && m_pCurSearch->m_pResults);
	FASSERT(pFirstVert);

	CFVec3A EntryVec, EdgeVec;
	CSearchResults* pResults = m_pCurSearch->m_pResults;
	//can't use shortcuts on edges with certain properties!
	if (!pAfterFirst || !(pFirstVert->GetEdgeTo(m_pGraph->GetVertId(pAfterFirst))->m_nProps & EDGEPROP_ALL_CANT_SHORTCUT))
	{
		//would it be backtracking to go to first, then to the one just after first?
		if (pAfterFirst)
		{
			EdgeVec.Sub(pAfterFirst->GetLocation(), pFirstVert->GetLocation());
		}
		else
		{
			//assume goal loc is on graph
			//if start loc is in vert, then don't go by any waypts.
			//go straight to goal
			if (((m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_3D) && m_pGraph->IsPtIn3DVertVol(pFirstVert, m_pCurSearch->m_StartLoc_WS)) ||
				(!(m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_3D) && m_pGraph->IsPtIn2DVertVol(pFirstVert, m_pCurSearch->m_StartLoc_WS, TRUE)))
			{
				//this is a fake entryshortcut
#if AIGRAPH_EDITOR_ENABLED
				pResults->m_EntryShortcut_WS.Set(m_pCurSearch->m_GoalLoc_WS);
#endif
				m_CurSearchEntryShortcut_WS.Set(m_pCurSearch->m_GoalLoc_WS);
				pResults->m_uResultsFlags |= CSearchResults::RESULTSFLAG_VALID_ENTRY_SHORTCUT;
				return TRUE;
			}
			else
			{
				EdgeVec.Sub(m_pCurSearch->m_GoalLoc_WS, pFirstVert->GetLocation());
			}
		}
		EntryVec.Sub(pFirstVert->GetLocation(), m_pCurSearch->m_StartLoc_WS);
		if (EdgeVec.Dot(EntryVec) < 0.0f)
		{	//more than 90 degrees so yes! find entry point 
			EdgeVec.Unitize();
			EntryVec.Mul(-1.0f);
			EdgeVec.Mul(EdgeVec.Dot(EntryVec));
#if AIGRAPH_EDITOR_ENABLED
			pResults->m_EntryShortcut_WS.Add(EdgeVec, pFirstVert->GetLocation());
#endif
			m_CurSearchEntryShortcut_WS.Add(EdgeVec, pFirstVert->GetLocation());
			pResults->m_uResultsFlags |= CSearchResults::RESULTSFLAG_VALID_ENTRY_SHORTCUT;
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CGraphSearcher::_CalcAndStoreExitShortcut(const GraphVert* pLastVert, const GraphVert* pBeforeLast)
{
	CFVec3A ExitVec, EdgeVec;
	CSearchResults* pResults = m_pCurSearch->m_pResults;
	//can't use shortcuts on edges with certain properties!
	if ((!pBeforeLast && !(pResults->m_uResultsFlags & CSearchResults::RESULTSFLAG_VALID_ENTRY_SHORTCUT)) ||
		(pBeforeLast && !(pBeforeLast->GetEdgeTo(m_pGraph->GetVertId(m_pGoalVert))->m_nProps & EDGEPROP_ALL_CANT_SHORTCUT)))
	{
		//would it be backtracking to go to first, then after first?
		if (pBeforeLast)
		{
			EdgeVec.Sub(m_pGoalVert->GetLocation(), pBeforeLast->GetLocation());
		}
		else
		{
			EdgeVec.Sub(m_pGoalVert->GetLocation(), m_pCurSearch->m_StartLoc_WS);
		}
		ExitVec.Sub(m_pCurSearch->m_GoalLoc_WS, m_pGoalVert->GetLocation());
		if (EdgeVec.Dot(ExitVec) < 0)
		{	//more than 90 degrees so yes! find exit point
			EdgeVec.Unitize();
			EdgeVec.Mul(EdgeVec.Dot(ExitVec));
#if AIGRAPH_EDITOR_ENABLED
			pResults->m_ExitShortcut_WS.Add(EdgeVec, m_pGoalVert->GetLocation());
#endif
			m_CurSearchExitShortcut_WS.Add(EdgeVec, m_pGoalVert->GetLocation());
			pResults->m_uResultsFlags |= CSearchResults::RESULTSFLAG_VALID_EXIT_SHORTCUT;
			return TRUE;
		}
	}
	return FALSE;
}

void CGraphSearcher::FinishSearch(void)
{
	CFVec3A PathVec;
	CFVec3A tmp;
	u16	uTrailingPathNodeId;

	FASSERT(m_pCurSearch);

	FASSERT(m_pCurSearch->m_uStatus & CSearchQuery::SEARCHSTATUS_ROUTING);
	m_pCurSearch->m_uStatus &= ~CSearchQuery::SEARCHSTATUS_ROUTING;
	m_pCurSearch->m_uStatus |= CSearchQuery::SEARCHSTATUS_ALLDONE;

	// find pBeforeLast and pAfterFirst nodes to help look for entry/exit shortcuts
 	CSearchNode* pPathNode = m_pGoalNode;
	u16	uPathNodeId = GETNODEID(pPathNode);
	GraphVert* pBeforeLast = NULL;
	const GraphVert* pAfterFirst = NULL;
	GraphVert* pFirstVert = m_pGraph->GetVert(GETNODEID(m_pStartNode));
	CSearchResults* pResults = m_pCurSearch->m_pResults;
	FASSERT(pFirstVert);

#if AIGRAPH_EDITOR_ENABLED
	FASSERT(pResults->m_VertPath.Size() == 0);
#endif
	if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_NEED_PATH_LENGTH)
	{	pResults->m_uResultsFlags |= CSearchResults::RESULTSFLAG_VALID_PATH_LENGTH;
		m_fCurSearchPathLength = m_pGoalNode->m_g;
	}

	if (pPathNode->m_uPrevNodeId)  //nodeId= 0 is invalid node
	{
		pBeforeLast = m_pGraph->GetVert(pPathNode->m_uPrevNodeId);
	}

	//check for entry shortcut
#if AIGRAPH_EDITOR_ENABLED
	if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_NEED_VERTPATH)
	{
		pResults->m_uResultsFlags |= CSearchResults::RESULTSFLAG_VALID_VERTPATH;
		while (uPathNodeId)
		{
			pResults->m_VertPath.PushHead(m_pGraph->GetVert(uPathNodeId));
			uPathNodeId = GETNODE(uPathNodeId)->m_uPrevNodeId;
		}

		CNiIterator<const GraphVert*> it = pResults->m_VertPath.Begin();
		if (it.IsValid() )
		{
			it.Next();
			if (it.IsValid())
			{
				pAfterFirst = it.Get();	//may be NULL for one-vert paths
			}
			_CalcAndStoreEntryShortcut(pFirstVert, pAfterFirst);
		}
	}
	else
#endif
	{
		uTrailingPathNodeId = 0;
		while (uPathNodeId && GETNODE(uPathNodeId)->m_uPrevNodeId)
		{
			uTrailingPathNodeId = uPathNodeId;
			uPathNodeId = GETNODE(uPathNodeId)->m_uPrevNodeId;
		}

		//uPathNode should be FirstNode and
		//uTrailingPathNodeId AfterFirst
		if (uPathNodeId)
		{
			FASSERT(pFirstVert && m_pGraph->GetVertId(pFirstVert) == uPathNodeId);
			if (uTrailingPathNodeId)
			{
				pAfterFirst = m_pGraph->GetVert(uTrailingPathNodeId);	  //may be null
			}
			else
			{
				FASSERT(pAfterFirst == NULL);
			}
			_CalcAndStoreEntryShortcut(pFirstVert, pAfterFirst);
		}
	}

	//check for exit shortcut
	if (pBeforeLast)
	{
		FASSERT(pBeforeLast->GetEdgeTo(m_pGraph->GetVertId(m_pGoalVert)));
	}
	FASSERT(m_pGoalVert);
	_CalcAndStoreExitShortcut(m_pGoalVert, pBeforeLast);

	//create an AIPath for them if desired
	if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_NEED_AIPATH)
	{
		CreateCookedAIPath(pResults, pAfterFirst, pBeforeLast);
	}
}


//static
void CGraphSearcher::CalcAproachEdgeData(CAIGraph* pGraph,
										 GraphVert* pFromV,	 //from this vert
										 GraphVert* pToV,	 //to this vert
										 u16 *wayPtFlags, f32 *pApproachWidth, CFVec3A* pApproachVector)
{
	FASSERT(pGraph && pFromV && pToV && wayPtFlags && pApproachWidth && pApproachVector);
	GraphEdge* pEntryEdge = pFromV->GetEdgeTo(pGraph->GetVertId(pToV));
	FASSERT(pEntryEdge);
	pApproachVector->Sub(pFromV->m_Location, pToV->m_Location);
	if (pApproachVector->SafeUnitAndMag(*pApproachVector) > 0.0f)
	{
		*wayPtFlags |= CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH;
		*pApproachWidth = pEntryEdge->m_fHalfWidth;
		
		if (!pEntryEdge->Is3D())
		{
			if (pEntryEdge->m_nProps & EDGEPROP_FAILEDEDGETEST && !(pEntryEdge->m_nProps & EDGEPROP_CUSTOM_VOL))
			{
				*pApproachWidth = pFromV->GetRad();
				if (pToV->GetRad() < *pApproachWidth)
				{
					*pApproachWidth = pToV->GetRad();
				}
			}

			//pgm took this out, since followr code now enforces it at a more appropriate time.
			//if (pEntryEdge->m_nProps & EDGEPROP_JUMP)
			//{
				//*pApproachWidth = 1.0f;	//hardcode a narrow edge width on this jump-link in an attempt to force better alignment of followers
			//}
		}
	}
}


void CGraphSearcher::CreateCookedAIPath(CSearchResults* pResults, const GraphVert* pAfterFirst, const GraphVert* pBeforeLast)
{
	u16 uPathNodeId = GETNODEID(m_pGoalNode);
	GraphVert* pLastVert = NULL;
	CFVec3A ApproachDirUnit;
	GraphVert* pStartGraphVert = m_pGraph->GetVert(GETNODEID(m_pStartNode));

	FASSERT(pResults->m_Path.GetNumWaypoints()==0);

	pResults->m_Path.SetGraph(m_pGraph);
	
	pResults->m_Path.AddWayPointHead(	CAIPathWaypoint(CAIPathWaypoint::WAYPOINTFLAG_STOP_AT,			// u16 uWayPointFlags, 
														m_pCurSearch->m_GoalLoc_WS,						// const CFVec3A& Location,
														m_pCurSearch->GetQueryWidth(),					// f32 fCloseEnoughDist,
														NULL,											// GraphVert* pGraphVert,
														EDGEPROP_NONE,									// u16 uExitEdgeProps,
														0.0f,											// f32 fApproachEdgeWidth,
														CFVec3A::m_Null));								// const CFVec3A& EntryVecUnit
	
	
	if (pResults->m_uResultsFlags & CSearchResults::RESULTSFLAG_VALID_EXIT_SHORTCUT)
	{
		const GraphEdge* pStartEdge = pBeforeLast->GetEdgeTo(GETNODEID(m_pGoalNode));
		FASSERT(pStartEdge);
		f32 fEdgeWidth = pStartEdge->m_fHalfWidth;
		if (pStartEdge->m_nProps & EDGEPROP_FAILEDEDGETEST)
		{
			if (fEdgeWidth <= 1.0f)
			{
				DEVPRINTF("AIGraph:: Bad edge volume at vert %d\n", GETNODEID(m_pGoalNode));
			}
			fEdgeWidth = pBeforeLast->GetRad();  
		}
		pResults->m_Path.AddWayPointHead(	CAIPathWaypoint(CAIPathWaypoint::WAYPOINTFLAG_NONE,		// u16 uWayPointFlags, 
															m_CurSearchExitShortcut_WS,				// const CFVec3A& Location,
															fEdgeWidth,								// f32 fCloseEnoughDist,
															NULL,									// GraphVert* pGraphVert,
															EDGEPROP_NONE,							// u16 uExitEdgeProps,
															0.0f,									// f32 fApproachEdgeWidth,
															CFVec3A::m_Null));						// const CFVec3A& EntryVecUnit
		//skip exit node, since there is a exit shortcut
		uPathNodeId = GETNODE(uPathNodeId)->m_uPrevNodeId;
	}

	while (uPathNodeId)
	{
		GraphVert* pV = m_pGraph->GetVert(uPathNodeId);
		GraphVert* pPrev = NULL;
		CSearchNode* pPathNode = GETNODE(uPathNodeId);
		f32 fApproachEdgeWidth = 0.0f;
		if (pResults->m_uResultsFlags & CSearchResults::RESULTSFLAG_VALID_ENTRY_SHORTCUT &&
			uPathNodeId && GETNODE(uPathNodeId)->m_uPrevNodeId == 0)
		{	 //don't add first, because there is an entry shortcut coming
			uPathNodeId = GETNODE(uPathNodeId)->m_uPrevNodeId;
			continue;
		}

		u16 wayPtFlags = CAIPathWaypoint::WAYPOINTFLAG_NONE;
		u16 uExitEdgeProps = EDGEPROP_NONE;

		//find out the approach edge width and approach edge unit direction vector
		f32 fNodeRad = pV->GetRad();
		if (pPathNode->m_uPrevNodeId)
		{
			//NOTE!  pPrev is previous in terms of the path, but will be the next this list traversal
			wayPtFlags |= CAIPathWaypoint::WAYPOINTFLAG_CONSTRAIN_APPROACH;
			pPrev = m_pGraph->GetVert(pPathNode->m_uPrevNodeId);
			CalcAproachEdgeData(m_pGraph, pPrev, pV, &wayPtFlags, &fApproachEdgeWidth, &ApproachDirUnit);
		}
		else
		{	//if there is a waypoint next, it isn't a graph vert, so don't set the WAYPOINTFLAG_CONSTRAIN_APPROACH bit, or specify any usefull entryvec, or width
			fApproachEdgeWidth = 0.0f;
			ApproachDirUnit = CFVec3A::m_Null;
		}
		
		//set any applicable FROM type properties of this vert
		if (pLastVert)	//NOTE!  pLastVert is the last in this list travers, but next in terms of the path!
		{
			GraphEdge* pEx = pV->GetEdgeTo(m_pGraph->GetVertId(pLastVert));
			if (pEx)
			{
				uExitEdgeProps = pEx->m_nProps;
			}
		}
		pResults->m_Path.AddWayPointHead(	CAIPathWaypoint(wayPtFlags,			    // u16 uWayPointFlags, 
															pV->m_Location,		    // const CFVec3A& Location,
															fNodeRad,				// f32 fCloseEnoughDist,									  ,
															pV,						// GraphVert* pGraphVert,
															uExitEdgeProps,		    // u16 uExitEdgeProps,
															fApproachEdgeWidth,	    // f32 fApproachEdgeWidth,
															ApproachDirUnit));	    // const CFVec3A& EntryVecUnit
		pLastVert = pV;
		uPathNodeId = GETNODE(uPathNodeId)->m_uPrevNodeId;
	}

	if (pResults->m_uResultsFlags & CSearchResults::RESULTSFLAG_VALID_ENTRY_SHORTCUT)
	{
		f32 fEdgeWidth = 0.0f;
		if (pAfterFirst)
		{
			GraphEdge* pStartEdge = pStartGraphVert->GetEdgeTo(m_pGraph->GetVertId(pAfterFirst));
			FASSERT(pStartEdge);
			fEdgeWidth = pStartEdge->m_fHalfWidth;
		}
		else
		{
			if (m_pCurSearch->m_fQueryWidth < pStartGraphVert->GetRad())
			{
				fEdgeWidth = m_pCurSearch->m_fQueryWidth;
			}
			else
			{
				fEdgeWidth = pStartGraphVert->GetRad();
			}
		}
		pResults->m_Path.AddWayPointHead(	CAIPathWaypoint(CAIPathWaypoint::WAYPOINTFLAG_NONE,	 // u16 uWayPointFlags, 
															m_CurSearchEntryShortcut_WS,		 // const CFVec3A& Location,
															fEdgeWidth,							 // f32 fCloseEnoughDist,		
															NULL,								 // GraphVert* pGraphVert,
															EDGEPROP_NONE,						 // u16 uExitEdgeProps,
															0.0f,								 // f32 fApproachEdgeWidth,
															CFVec3A::m_Null));					 // const CFVec3A& EntryVecUnit
	}

	pResults->m_uResultsFlags |= CSearchResults::RESULTSFLAG_VALID_AIPATH;
}



void CGraphSearcher::CancelSearch(CSearchQuery* pSearch)
{
	if (pSearch)
	{
		if (m_RequestQueue.Remove(pSearch))
		{
			pSearch->m_uStatus = CSearchQuery::SEARCHSTATUS_CANCELED;
		}

		if (pSearch == m_pCurSearch)
		{
			pSearch->m_uStatus = CSearchQuery::SEARCHSTATUS_CANCELED;
			m_pCurSearch = NULL;
		}
	}
	
}


f32 CGraphSearcher::GetEdgeLosCost(GraphVert* pSrcVert, GraphEdge* pEdge)
{
	//find all poi within edge (average the los rating)
	//cost is length of edge * this average
//	m_pCurSearch->m_fAvoidCost*0.5f*(pSrcVert->GetLocation().Dist(m_pCurSearch->m_AvoidPt_WS) + m_pGraph->GetVert(pEdge->m_nNextVertId)->GetLocation().Dist(m_pCurSearch->m_AvoidPt_WS));	 //ich, average the distance to the avoid point and both ends of the edge}
	return 0.0f;
}

f32 CGraphSearcher::GetCostOfEdge(GraphVert* pSrcVert, GraphEdge* pEdge)
{
	f32 fExtraCost = 0.0f;
	fExtraCost += m_pCurSearch->m_fSlopeCost*(f32)pEdge->m_uMaxPosHeightDelta;
	if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_AVOIDPT_PATH)
	{  //m_fAvoidCost is per foot
		f32 fDist1 = pSrcVert->GetLocation().Dist(m_pCurSearch->m_AvoidPt_WS);	 //findfix: only one dist per line if inlines are enabled... yikes
		fExtraCost += m_pCurSearch->m_fAvoidCost*0.5f*(fDist1 + m_pGraph->GetVert(pEdge->m_nNextVertId)->GetLocation().Dist(m_pCurSearch->m_AvoidPt_WS));	 //ich, average the distance to the avoid point and both ends of the edge
	}
	else if (m_pCurSearch->m_uParams & CSearchQuery::SEARCHPARAM_AVOIDLOS_PATH)
	{
	 	fExtraCost += GetEdgeLosCost(pSrcVert, pEdge);
	}
	
	return pEdge->GetLength() + fExtraCost;
}


f32 CGraphSearcher::GetEstimatedCostToGoal(const GraphVert* pVert)
{
	FASSERT(m_pCurSearch);

	return pVert->GetLocation().Dist(m_pGoalVert->GetLocation());  //putting a faster/rougher estimate here is tempting, but dangerous 
}


