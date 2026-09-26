#ifndef _AIPATH_H_
#define _AIPATH_H_ 1

//////////////////////////////////////////////////////////////////////////////////////
// AIPath.h 
//  classes:
//      CAIPathWaypoint - a waypoint in a path
//		CAIPath	- maintain a list of waypoints and path description info
//      CAIPathWalker - Progress tracker/iterator used by objects that are following a path.
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
// 03/14/02 Mackellar      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fmath.h"
#include "Ftl.h"

class CAIPath;
class GraphVert;
class GraphEdge;
class CAIGraph;

//
//  CAIPathWaypoint
//		a waypoint in a path.  
//		At a minimum has a location in world space and a close enough distance.
//
FCLASS_ALIGN_PREFIX class CAIPathWaypoint
{
public:
	FINLINE CAIPathWaypoint(void) {}
	FINLINE CAIPathWaypoint(u16 uWayPointFlags, 
							const CFVec3A& Location,
							f32 fCloseEnoughDist,
							GraphVert* pGraphVert,
							u16 uExitEdgeProps,
							f32 fApproachEdgeWidth,
							const CFVec3A& EntryVecUnit) :
								m_Location(Location),
								m_ApproachDirUnit(EntryVecUnit),
								m_fApproachWidth(fApproachEdgeWidth),
								m_fCloseEnoughDist(fCloseEnoughDist),
								m_pGraphVert(pGraphVert),
								m_uExitEdgeProps(uExitEdgeProps),
								m_uWayPointFlags(uWayPointFlags) 	{		}
	enum
	{
	   WAYPOINTFLAG_NONE					= 0x0000,
	   WAYPOINTFLAG_STOP_AT					= 0x0001,
	   WAYPOINTFLAG_END_OF_PATH				= 0x0002,
	   WAYPOINTFLAG_CONSTRAIN_APPROACH		= 0x0004,
	};

	CFVec3A m_Location;				// 16
	CFVec3A m_ApproachDirUnit;		// 16	//the Approach direction from the previous waypoint. NOTE! Only valid if (WAYPOINTFLAG_CONSTRAIN_APPROACH) flag is set.
	f32 m_fApproachWidth;			//  4	//the half width of the path leading up to the waypoint.  NOTE! Only valid if (WAYPOINTFLAG_CONSTRAIN_APPROACH) flag is set.
	f32 m_fCloseEnoughDist;			//  4	//the radius of the waypoint (cylinder rad for Floor graphs, Sphere radius for 3D graphs)
	GraphVert* m_pGraphVert;		//  4	//If not NULL, points to a vert in the pathfinding graph.
	u16 m_uExitEdgeProps;			//  2	//the properties of the edge that leaves this waypoint
	u16 m_uWayPointFlags;			//  2	//see enum above.
									// 48
	FCLASS_STACKMEM_ALIGN(CAIPathWaypoint);
} FCLASS_ALIGN_SUFFIX;
//48 bytes

//
//  CAIPathWalker
//
FCLASS_NOALIGN_PREFIX class CAIPathWalker
{
public:
	CAIPathWalker(void);
	~CAIPathWalker(void);

	void Init(CAIPath* pPath);
	void ChangeDirection(CAIPath* pPath);
	void AdvanceTo(CAIPath* pPath, u16 nWayptNum);
	BOOL Advance(void);  //returns false if there are no more waypoints.

	inline u16 GetCurWaypointNum(void)			{ return m_uCurWayPointNum;}	//out of the total number of waypoints in path we're iterating, number of the current one
	CAIPathWaypoint* GetCurWaypoint(void);			//pointer to the waypont walker is heading toward
	CAIPathWaypoint* GetPreviousWaypoint(void);
	CAIPathWaypoint* GetNextWaypoint(void);
	CAIPathWaypoint* GetNextNextWaypoint(void);
	BOOL GetCurLoc(CFVec3A* pLoc);					//what is the location of the waypoint I'm heading toward?
	BOOL CloseEnoughXZ(const CFSphereA& Sphere);	//is this sphere close enough to the current waypoint in the XZ plane
	BOOL CloseEnough(const CFSphereA& Sphere);		//is this sphere close enough to the current waypoint
	BOOL HasNext(void)							{ return m_PathIt.HasNext();}
	u16 GetCurVertId(void);
	u16 GetLastVertId(void);
	void SetLoop(BOOL bLoop)					{ m_uLoop = (u8) bLoop;}
	BOOL GetReverse(void)						{ return m_uReverse==1;}
	BOOL GetLoop(void)							{ return m_uLoop==1;}
	BOOL IsValid(void)							{ return m_PathIt.IsValid();}

	enum
	{
		INVALID_VERTID = 0
	};
protected:
	CAIPath* m_pPath;
	u16 m_uCurWayPointNum;   //out of the total number of waypoints in path we're iterating, number of the current one
	u8 m_uReverse;
	u8 m_uLoop;
	CNiIterator<CAIPathWaypoint*> m_PathIt;
	FCLASS_STACKMEM_NOALIGN(CAIPathWalker);
} FCLASS_NOALIGN_SUFFIX;



//
//  CAIPath
//

FCLASS_NOALIGN_PREFIX class CAIPath
{
public:
	CAIPath(void);
	~CAIPath(void);

	FINLINE u16 GetNumWaypoints(void);	//how many waypoints are there in this path?
	FINLINE CNiIterator<CAIPathWaypoint*> BeginPathWalk(BOOL bReverse = FALSE);
	BOOL GetEndOfPath(CFVec3A* pEndPt_WS );
	s32	FindClosestWaypointNum(const CFVec3A& loc, BOOL bReverse = FALSE);	//out of the total number of waypoints, find the number of the one that is closest to pt in space
	FINLINE CAIGraph* GetGraph(void)				{ return m_pGraph;}	   //which graph does are this path's verts from?
	FINLINE void SetGraph(CAIGraph* pGraph)			{ m_pGraph = pGraph;}
	//creating a path
	FINLINE void AddWayPointHead(const CAIPathWaypoint& rWayPoint, BOOL bReverse = FALSE);
	FINLINE void AddWayPointTail(const CAIPathWaypoint& rWayPoint, BOOL bReverse = FALSE);
	void JoinPaths(CAIPath& rOtherPath, BOOL bReverse = FALSE);
	void DontStopAtEnd(BOOL bReverse = FALSE);

	void Reset(void);
	void DebugRender(BOOL bReverse = FALSE);

protected:
	FINLINE CAIPathWaypoint* ReverseAddWayPointHead(const CAIPathWaypoint& rWayPoint);
	FINLINE CAIPathWaypoint* ReverseAddWayPointTail(const CAIPathWaypoint& rWayPoint);
	
	CAIGraph* m_pGraph;
	f32 m_fPathLength;
	CNiList<CAIPathWaypoint*> m_WaypointList;
	CNiList<CAIPathWaypoint*> m_ReverseWaypointList;

	//
	//  Statics
	//
public:
	enum
	{
		AIPATH_DEFAULT_GLOBAL_WAYPOINT_POOL_COUNT = 500,
	};
	static BOOL InitSystem(struct FLinkRoot_s* pNodePools, s32 nMaxNumGlobalPathNodes = AIPATH_DEFAULT_GLOBAL_WAYPOINT_POOL_COUNT);
	static BOOL InitSystem(struct FLinkRoot_s* pNodePools, void* pWaypointNodeMemory, s32 nMemorySizeBytes);
	static void UninitSystem(void);
	static BOOL IsSystemInitialized(void) {return s_pWayPointBank != NULL;}
	static void UseThisPtrNodePool(FLinkRoot_t* pNodePool);

public:
	static void RecycleWayPt(CAIPathWaypoint* pWay);
protected:
	
	static CNiBank<CAIPathWaypoint>* s_pWayPointBank;
	static FLinkRoot_t* s_pPtrNodePool;

	FCLASS_STACKMEM_NOALIGN(CAIPath);
} FCLASS_NOALIGN_SUFFIX;


#if FANG_ENABLE_INLINE_CODE
#include "AIPath.inl"
#endif

#endif //_AIPATH_H_