//////////////////////////////////////////////////////////////////////////////////////
// GraphSearcher.h - 
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
// 04/04/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GRAPHSEARCHER_H_
#define _GRAPHSEARCHER_H_ 1

#include "AIGraph.h"
#include "AIPath.h"
#include "ftl.h"

#if AIGRAPH_DEBUG
#include "ftimer.h"
#endif

#ifndef USE_HAZARDS
#if !AIGRAPH_EDITOR_ENABLED
#define USE_HAZARDS	1					 //use_hazaards will match aigraph_editor enabled if defined in aigraph.h
#endif
#endif

#ifdef USE_HAZARDS
#include "AIHazard.h"
#endif

class CGraphSearcher;
class CSearchNode;
class CPQ;

#if !AIGRAPH_EDITOR_ENABLED

//
//
// CSearchResults
//	   - Clients pass one of these with each search request
//	   - it is where the results of the search go.
//
FCLASS_NOALIGN_PREFIX class CSearchResults
{
public:
	CSearchResults(void);
	~CSearchResults(void);

	enum
	{
		RESULTSFLAG_NONE					= 0x0000,
		RESULTSFLAG_VALID_PATH_LENGTH		= 0x0001,
		RESULTSFLAG_VALID_AIPATH			= 0x0002,
		RESULTSFLAG_VALID_ENTRY_SHORTCUT	= 0x0008,
		RESULTSFLAG_VALID_EXIT_SHORTCUT		= 0x0010,
		RESULTSFLAG_VALID_FUDGE_DEST		= 0x0020,
	};

	void Clear(void);
	FINLINE u16 GetFlags(void)								{ return m_uResultsFlags;}
	FINLINE CAIPath* GetAIPath(void)						{ return (m_uResultsFlags & RESULTSFLAG_VALID_AIPATH) ? &m_Path : NULL;}

protected:
	friend class CGraphSearcher;
	CAIPath m_Path;

	//some stats regarding the search

#if AIGRAPH_DEBUG
	u32 m_nCookLoops;		// how many A* loop iterations...
	f32 m_fFiniTime;		// how long it took to finish the path
	f32 m_fCookTime;		// how many ticks passed while processing the path
	f32 m_fRequestTime;		// tick at request time.. could be used to get delta time includeing time in pending queue
#endif

	u16 m_uResultsFlags;

	FCLASS_STACKMEM_NOALIGN(CSearchResults);
} FCLASS_NOALIGN_SUFFIX;

#elif AIGRAPH_EDITOR_ENABLED

//
//
// CSearchResults
//	   - Clients pass one of these with each search request
//	   - it is where the results of the search go.
//

FCLASS_ALIGN_PREFIX class CSearchResults
{
public:
	CSearchResults(void);
	~CSearchResults(void);

	enum
	{
		RESULTSFLAG_NONE					= 0x0000,
		RESULTSFLAG_VALID_PATH_LENGTH		= 0x0001,
		RESULTSFLAG_VALID_AIPATH			= 0x0002,
		RESULTSFLAG_VALID_VERTPATH			= 0x0004,
		RESULTSFLAG_VALID_ENTRY_SHORTCUT	= 0x0008,
		RESULTSFLAG_VALID_EXIT_SHORTCUT		= 0x0010,
		RESULTSFLAG_VALID_FUDGE_DEST		= 0x0020,
	};

	void Clear(void);
	FINLINE u16 GetFlags(void)								{ return m_uResultsFlags;}
	FINLINE CAIPath* GetAIPath(void)						{ return (m_uResultsFlags & RESULTSFLAG_VALID_AIPATH) ? &m_Path : NULL;}

	CSearchResults(FLinkRoot_t* pVertPathNodePool);
	FINLINE const CFVec3A& GetEntryShortCut(void)			{ return m_EntryShortcut_WS;}
	FINLINE const CFVec3A& GetExitShortCut(void)			{ return m_ExitShortcut_WS;}
	FINLINE CNiList<const GraphVert*>* GetVertPath(void)	{ return (m_uResultsFlags & RESULTSFLAG_VALID_VERTPATH) ? &m_VertPath : NULL;}
	FINLINE f32 GetPathLength(void)							{ return m_fPathLength;}

	void Render(void);

protected:
	friend class CGraphSearcher;

	CFVec3A m_EntryShortcut_WS;
	CFVec3A m_ExitShortcut_WS;
	
	// Depending on what was specified as being needed, results are stored in m_VertPath and/or m_Path
	CNiList<const GraphVert*> m_VertPath;
	f32 m_fPathLength;

	CAIPath m_Path;

	//some stats regarding the search

	u32 m_nCookLoops;		// how many A* loop iterations...
	f32 m_fFiniTime;		// how long it took to finish the path
	f32 m_fCookTime;		// how many ticks passed while processing the path
	f32 m_fRequestTime;		// tick at request time.. could be used to get delta time includeing time in pending queue

	u16 m_uResultsFlags;

	FCLASS_STACKMEM_ALIGN(CSearchResults);
} FCLASS_ALIGN_SUFFIX;

#endif   //EDITOR version of searchresults


//
//
//  CSearchQuery
//	  - used to submit and receive resulting paths from graph searches
//	  - These could be pooled, and handed out as necessary, or
//    	clients can keep local static copies for specif needs.
//
FCLASS_ALIGN_PREFIX class CSearchQuery
{
public:

	CSearchQuery(void);
	~CSearchQuery(void);

	void Clear(void); //blank the structure
	

	// description of the type of query.
	// 1) fill out the CSearchQuery
	// 2) then submit it to CGraphSearcher
	// 3) then sit back and wait for a path, checking the status flags in the CSearchQuery obj
	enum
	{
		SEARCHPARAM_FLOOR				= 0x0000,
		SEARCHPARAM_NO_JUMPS			= 0x0001,
		SEARCHPARAM_3D					= 0x0002,
		SEARCHPARAM_NO_SWITCHES			= 0x0004,
		SEARCHPARAM_NO_LIFTS			= 0x0008,
		SEARCHPARAM_AVOIDLOS_PATH		= 0x0010,
		SEARCHPARAM_FUDGE_DEST			= 0x0020,
		SEARCHPARAM_KNOWN_START_VERT	= 0x0040,
		SEARCHPARAM_KNOWN_GOAL_VERT		= 0x0080,
		SEARCHPARAM_NEED_PATH_LENGTH	= 0x0100,
		SEARCHPARAM_NEED_AIPATH			= 0x0200,
		SEARCHPARAM_NEED_VERTPATH		= 0x0400,
		SEARCHPARAM_AVOIDPT_PATH		= 0x0800,
		SEARCHPARAM_LIMIT_JUMP_DIST		= 0x1000,
	};
	
	FINLINE void SetDefaultSearchParams(	const CFVec3A& StartLoc_WS,
										const CFVec3A& GoalLoc_WS,
										f32 fQueryWidth,
										f32 fQueryHeight,
										CSearchResults* pResults,
										u16 uSearchParams,
										u32 uClientGuid)
													{	m_StartLoc_WS = StartLoc_WS;
														m_GoalLoc_WS = GoalLoc_WS;
														m_fQueryWidth = fQueryWidth;
														m_fQueryHeight = fQueryHeight,
														m_pResults = pResults;
														m_uParams = uSearchParams;
														m_uClientGuid = uClientGuid;
													};

	// Status of the search
	enum
	{
		SEARCHSTATUS_CLEAR							= 0x0000,
		SEARCHSTATUS_PENDING						= 0x0001,
		SEARCHSTATUS_ROUTING						= 0x0002,
		SEARCHSTATUS_ALLDONE						= 0x0004,
		SEARCHSTATUS_BEENDONE						= 0x0008,
		SEARCHSTATUS_CANCELED						= 0x0010,
		SEARCHSTATUS_FAILED							= 0x0020,
		SEARCHSTATUS_FAILED_REASON_BAD_START		= 0x0040,
		SEARCHSTATUS_FAILED_REASON_BAD_END			= 0x0080,
	};

	
	FINLINE BOOL HasFailed(void)					{ return m_uStatus & SEARCHSTATUS_FAILED;}
	FINLINE BOOL IsDone(void)						{ return m_uStatus & SEARCHSTATUS_ALLDONE;}
	FINLINE BOOL HasBeenDone(void)					{ return m_uStatus & SEARCHSTATUS_BEENDONE;}
	FINLINE BOOL WasCanceled(void)					{ return m_uStatus & SEARCHSTATUS_CANCELED;}
	FINLINE void SetHasBeenDone(void)				{ m_uStatus |= SEARCHSTATUS_BEENDONE;}
	FINLINE void SetGoal(const CFVec3A& Pt_WS)		{ m_GoalLoc_WS = Pt_WS;}
	FINLINE void SetStart(const CFVec3A& Pt_WS)		{ m_StartLoc_WS = Pt_WS;}
	FINLINE void SetSlopeCost(f32 fSlopeCost)		{ m_fSlopeCost = fSlopeCost;}
	FINLINE void SetLOSPt(const CFVec3A& Pt_WS)		{ m_uParams |= SEARCHPARAM_AVOIDLOS_PATH; m_AvoidPt_WS = Pt_WS;}
	FINLINE void SetAvoidPt(const CFVec3A& Pt_WS, f32 fAvoidCostPerDist)	{ m_uParams |= SEARCHPARAM_AVOIDPT_PATH; m_AvoidPt_WS = Pt_WS; m_fAvoidCost = fAvoidCostPerDist;}
	FINLINE void SetFudgeDestDist(f32 fDist)		{ m_uParams |= SEARCHPARAM_FUDGE_DEST; m_fMaxDestinationFudge = fDist;} 
	FINLINE void SetKnownStartVertId(u16 uVertId)	{ m_uParams |= SEARCHPARAM_KNOWN_START_VERT; m_uKnownStartVertId = uVertId;}
	FINLINE void SetKnownGoalVertId(u16 uVertId)	{ m_uParams |= SEARCHPARAM_KNOWN_GOAL_VERT; m_uKnownGoalVertId = uVertId;}
	FINLINE void SetMaxJumpDist(f32 fMaxJumpDist)	{ m_uParams |= SEARCHPARAM_LIMIT_JUMP_DIST; m_fMaxJumpDist = fMaxJumpDist;}
	
	FINLINE CSearchResults* GetResults(void)		{ return m_pResults;};
	FINLINE const CFVec3A& GetStartLoc(void) const	{ return m_StartLoc_WS;};
	FINLINE const CFVec3A& GetGoalLoc(void) const	{ return m_GoalLoc_WS;};
	FINLINE f32 GetQueryWidth(void) const			{ return m_fQueryWidth;};
	FINLINE f32 GetQueryHeight(void) const			{ return m_fQueryHeight;};
	FINLINE u16 GetStatus(void)						{ return m_uStatus;}
	FINLINE u16 GetParams(void)						{ return m_uParams;}
	FINLINE u16 SetParams(u16 uNewParams)			{ return m_uParams = uNewParams;}

protected:
	friend class CGraphSearcher;
	CFVec3A m_StartLoc_WS;
	CFVec3A m_GoalLoc_WS;
	CFVec3A m_AvoidPt_WS;
	GraphVert* m_pAvoidVert;
	f32 m_fQueryWidth;
	f32 m_fQueryHeight;
	f32 m_fSlopeCost;
	f32 m_fAvoidCost;
	f32 m_fMaxDestinationFudge;
	f32 m_fMaxJumpDist;
	u32 m_uClientGuid;					//a Guid that the client can specify.  GraphSearcher can use this to block repetitive searches by stuck clients
	CSearchResults* m_pResults;			//where the results of this search will be stored
	u16 m_uStatus;	  					// Check this while graph searcher is working for various things defined by SEARCHSTATUS_* above
	u16 m_uParams;						// Control the path finder
	u16 m_uKnownStartVertId;
	u16 m_uKnownGoalVertId;
	FCLASS_STACKMEM_ALIGN(CSearchQuery);
} FCLASS_ALIGN_SUFFIX;


//
//
//  CGraphSearcer
//	  - Searches graph network for the best path between start and goal locations
//
//
FCLASS_ALIGN_PREFIX class CGraphSearcher
{
protected:
	static const f32 s_kfDefaultSlopeCost;	//code worrier likes this at the top

public:
	CGraphSearcher(void);
	virtual ~CGraphSearcher(void);

	BOOL BindToGraph(CAIGraph *pGraph);				//ALERT! this causes memory allocation, and is meant to be part of initialization

	void UsePNodePool(FLinkRoot_t* pNodePool);		//Use this node pool for internal lists of pointers
	void SetMaxLoopsPerSession(s32 nMaxLoops);
	void Work(s32* pnWorkLoopCounter);

	//Clients call this to request that a search be performed.
	void SubmitQuery( CSearchQuery* pNewSearch);				 

	void CancelSearch(CSearchQuery* pNewSearch);		 //Clients call this to cancel a search that is pending 'or' processing
	BOOL IsBusy(void) const {return m_RequestQueue.Size() > 0;};
	BOOL IsOperational(void) const {return m_pGraph != NULL;};

	FINLINE CAIGraph* GetGraph(void) {return m_pGraph;};

#ifdef USE_HAZARDS
	CAIHazardReg m_DoorHazardReg;
#endif

	static void CalcAproachEdgeData(CAIGraph* pGraph, GraphVert* pPrev, GraphVert* pV,  u16 *wayPtFlags, f32 *pApproachWidth, CFVec3A* pApproachVector);

	const GraphVert* CGraphSearcher::_FindStartVert(CAIGraph* pGraph, CSearchQuery* pSearch);
	const GraphVert* CGraphSearcher::_FindGoalVert(CAIGraph* pGraph, CSearchQuery* pSearch, const GraphVert* pStartGraphVert);

protected:

	BOOL BeginSearch(void);
	BOOL _CalcAndStoreEntryShortcut(const GraphVert* pFirstVert, const GraphVert* pAfterVert);
	BOOL _CalcAndStoreExitShortcut(const GraphVert* pLastVert, const GraphVert* pBeforeLast);
	void FinishSearch(void);
	void CreateCookedAIPath(CSearchResults* pResults, const GraphVert* pAfterFirst, const GraphVert* pBeforeLast);
	void RequestReverseSearch(CSearchQuery* pSearch);


	FINLINE u16 GetNumNodes(void) { FASSERT(m_pGraph); return m_pGraph->GetNumVerts(); };
	f32 GetEstimatedCostToGoal(const GraphVert* pVert);	//estimate the cost to go from this vert to the current search's goal vert
	f32 GetEdgeLosCost(GraphVert* pSrcVert, GraphEdge* pEdge);
	f32 GetCostOfEdge(GraphVert* pSrcVert, GraphEdge* pEdge);	//the cost to go from this vert to the current search's goal vert
	void ResolveHazardPtrs(void);

	CAIGraph *m_pGraph;				//the graph which the searcher is bound to.
	CSearchNode *m_paSearchNodes;	//markup space, one for each vert in m_pGraph
	CPQ* m_pCPQ;					   // the priority queue that the searcher sues
	s32 m_nNumSearchNodes;			//the number of search nodes at time of graph binding

	//server vbls
	CSearchQuery *m_pCurSearch;		//Search currently in progress, NULL when none
#if AIGRAPH_DEBUG
	CFTimer m_CurSearchTimer;
#endif
	s32 m_nMaxLoopsPerSession;
	CNiList<CSearchQuery*> m_RequestQueue;
	CFVec3A m_CurSearchEntryShortcut_WS;
	CFVec3A m_CurSearchExitShortcut_WS;
	f32 m_fCurSearchPathLength;
	
	// working vbls for the current search
	CSearchNode *m_pStartNode;
	CSearchNode *m_pGoalNode;
	const GraphVert *m_pGoalVert;
	BOOL m_bHazardsResolved;

	FCLASS_STACKMEM_ALIGN(CGraphSearcher);
} FCLASS_ALIGN_SUFFIX;

#endif