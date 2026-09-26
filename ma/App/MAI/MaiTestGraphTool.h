//////////////////////////////////////////////////////////////////////////////////////
// MaiTestGraphTool.h - Tool for adding pathfinding verts
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
// 02/19/02 patm       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAITESTGRAPHTOOL_H_
#define _MAITESTGRAPHTOOL_H_ 1

#include "MaiBaseTool.h"
#include "MaiQueryDetails.h"
#include "AI/AIGraph.h"
#include "AI/AIGraphSearcher.h"
#include "fdraw.h"


class TestGraphTool: public BaseTool
{
public:
	TestGraphTool(void);
	virtual ~TestGraphTool(void);
	virtual void UserInput(const CMaiUserInput& rMaiUI);
	virtual void OnFocus(void);
	virtual void Draw(void);
	void SetQueryDetail(CQueryDetails* pDetail)					{m_pQueryDetail = pDetail;};
	CQueryDetails* GetQueryDetails(void)						{return m_pQueryDetail;};

	enum 
	{
		ORIGIN_TESTPOINT = 0,
		GOAL_TESTPOINT,
		NUM_TESTPOINTS
	};


	void ToggleTestPointTracking(s32 nTestPointId)				{	m_bTracking[nTestPointId] ^= 1;};
	void SetTestPointTracking(s32 nTestPointId, BOOL bOnOff)	{	m_bTracking[nTestPointId]  = bOnOff;};
	void FindPath(void);
	s32  FindClosestIntersectingTestPoint(const CFVec3A& rRayStart, const CFVec3A& rRayEnd);
	
	CFVec3 m_TestPoint[NUM_TESTPOINTS];
	BOOL   m_bTracking[NUM_TESTPOINTS];
	BOOL   m_bSelected[NUM_TESTPOINTS];
	f32	   m_fCuRad[NUM_TESTPOINTS];
	CFColorRGBA m_rgb[NUM_TESTPOINTS];

	CSearchQuery *m_pQuery;
	CSearchResults *m_pQueryResults;
	CQueryDetails	*m_pQueryDetail;  //details about the type of query this test tool is going to perform if invoked
};


#endif //_MAITESTGRAPHTOOL_H_