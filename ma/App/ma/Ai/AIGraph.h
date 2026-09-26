//////////////////////////////////////////////////////////////////////////////////////
// AIGraph.h - A directed graph in 3D Space used for pathfinding
//             Implemented as two arrays (edges and verts).  No lists or adjacency matrix.
//             This is more geard toward pre-allocated graphs, Vert and Edge removal and Insertion is 
//             supported, but is slow.
//             - use CGraphSearcher to do best-path searches on this graph
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
// 02/06/02 patm		Created.
// 06/14/02 patm		Better Distinction between 2D and 3D verts
// 08/15/02 patm		Poi
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _AIGRAPH_H_
#define _AIGRAPH_H_ 1

#include "fmath.h"


#ifndef AIGRAPH_EDITOR_ENABLED
	#define AIGRAPH_EDITOR_ENABLED 0
#endif

#if AIGRAPH_EDITOR_ENABLED==2		  //ASCII to Binary Converter uses level 2
	#define XVIRTUAL 
#elif AIGRAPH_EDITOR_ENABLED==1		  //MAI.exe Graph Editor uses 1
	#define XVIRTUAL virtual
#elif AIGRAPH_EDITOR_ENABLED==0		  //Run-time game code uses 0
	#define XVIRTUAL 
#endif

#define AIGRAPH_DEBUG 1

#if AIGRAPH_EDITOR_ENABLED
	#include <stdio.h>	 //FILE
#endif

typedef BOOL (*AIGraph_LOSTestFunc)(const CFVec3A &rRayStart, const CFVec3A &rRayEnd);
void AIGraph_SetLOSTestFunc(AIGraph_LOSTestFunc pFunc);


#if FANG_WINGC
	#define AIGRAPH_NUM_EDGES_PER_VERT 5			//The retail GameCube graph stores five edge slots per vert
#else
	#define AIGRAPH_NUM_EDGES_PER_VERT 6			//How many edges a vert on the graph can have
#endif
#define AIGRAPH_FUTURE_NUM_EDGES_PER_VERT 5		 //In the future, How many edges a vert on the graph can have
#define AIGRAPH_INVALID_VERTID 0


//
//
// class GraphEdge
//
//
class GraphVert;
class CAIGraph;
class CAIGraphDataAccess;

#define EDGEPROP_NONE						0x0000  
#define EDGEPROP_FAILEDEDGETEST				0x0001  //This is automatically set property.
#define EDGEPROP_CUSTOM_VOL					0x0002  //This is only used if AIGRAPH_EDITOR_ENABLED
#define EDGEPROP_3D							0x0004	//This edge is part of a 3D graph and is for Flying bots
#define EDGEPROP_JUMP						0x0038  //There could be 7 types of jumps  currently there are 3 (see JUMPTYPE_*)
#define EDGEPROP_DOWN_ONLY					0x0040	//edge is one way in the down direction
#define EDGEPROP_UP_ONLY					0x0080	//edge is one way in the up direction
#define EDGEPROP_HAZARD_DOOR				0x0100
#define EDGEPROP_HAZARD_LIFT				0x0200
#define EDGEPROP_HAZARD_HAS_SWITCH			0x0400
#define EDGEPROP_HAZARD_DESTRUCTABLE		0x0800	// a destructable entity is blocking the way
#define EDGEPROP_HAZARD_BRIDGE				0x1000	// a entity is required to use this edge
#define EDGEPROP_HAZARD_USE_SWITCH1			0x0002  //NOTE, same bit as EDGEPROP_CUSTOM_VOL, since game code doesn't need that one anyway, just tool
#define EDGEPROP_HAZARD_DOOR_FRONT			0x0002  //NOTE, same bit as EDGEPROP_CUSTOM_VOL, since game code doesn't need that one anyway, just tool

#define JUMPTYPE_OBSTACLE		0x0008  //path follower should jump when an obstacle is encountered
#define JUMPTYPE_LEDGE			0x0010  //path follower should jump when a ledge is encountered
#define JUMPTYPE_VERT_2_VERT	0x0018  //path follower should jump from vert to vert
#define JUMPTYPE_HOVER			0x0020  //path follower should hover from vert to vert

#define EDGEPROP_ALL_CUSTOM					(EDGEPROP_JUMP | EDGEPROP_DOWN_ONLY | EDGEPROP_UP_ONLY | EDGEPROP_HAZARD_DOOR | EDGEPROP_HAZARD_LIFT | EDGEPROP_HAZARD_HAS_SWITCH | EDGEPROP_HAZARD_DESTRUCTABLE | EDGEPROP_HAZARD_BRIDGE)
#define EDGEPROP_ALL_CANT_SHORTCUT			(EDGEPROP_JUMP | EDGEPROP_HAZARD_DOOR | EDGEPROP_HAZARD_LIFT)

FCLASS_NOALIGN_PREFIX class GraphEdge
{
public:
	void				Init(u16 sToVertId, f32 fLength, f32 fMaxSafeWidth, f32 fMaxSafeHeight, f32 fMaxPosHeightDelta);
	GraphEdge			*GetReverseEdge(CAIGraph* pGraph) const;
	FINLINE f32			GetLength2(void)const						{ return m_fLength*m_fLength;}
	FINLINE f32			GetLength(void)	const						{ return m_fLength;}
	FINLINE f32			GetHeight(void)	const						{ return m_fMaxSafeHeight;};
	FINLINE BOOL		Is3D(void)		const						{ return (m_nProps & EDGEPROP_3D);}
	FINLINE BOOL		Is2D(void)		const						{ return !(m_nProps & EDGEPROP_3D);}

#if AIGRAPH_EDITOR_ENABLED
	const char*			ParseAscii(const char* pData, const char* pDataEnd, CAIGraph* pGraph);
	char*				EmitAscii(char* pData, char* pDataEnd);
	void				AutoSetDimension2D(GraphVert* pVA, GraphVert* pVB, u16 uFromVertId, const CFVec3A& LookUpUnit, f32 fInitialMaxWidth);
	void				EndianFlip(void);
	void				ChangeDimension(f32 fLength, f32 fWidth, f32 fHeight, f32 fPosHeightDelta);
#endif

	u16 m_nNextVertId;							//  2			 //Index of the vert that this edge goes to
	u16 m_nProps;								//  2			 //see EDGEPROP_*
	f32 m_fLength;								//  4			 //Length of this edge
	f32 m_fMaxSafeHeight;						//  4			 //Height of this edge
	f32 m_fHalfWidth;							//  4			 //Half width of this edge
	u8 m_nHazardId;								//  1			 //if this edge has a hazard, used to lookup the hazard in the CAIGraph::HazardRegistry
	u8 m_nEdgeGUID;								//  1			 //User specified GUID, can be used to refer to an edge during runtime
	u8 m_uMaxPosHeightDelta;					//  2			 //Kinda like slope
	u8 m_uWaste;								//  1
	static const s32 knAsciiBuffSize;
	static const f32 kfMAX_EDGE_CLEARANCE;
	FCLASS_STACKMEM_NOALIGN(GraphEdge);
} FCLASS_NOALIGN_SUFFIX;						//----
												// 20 

//
//
// class GraphVert
//
//
#define VERTPROP_CUSTOM_VOL		0x01
#define VERTPROP_3D				0x02

FCLASS_ALIGN_PREFIX class GraphVert
{
public:
	void				Init(void);
	inline u8			GetNumEdges(void) const							{ return m_nNumEdges;}
	inline GraphEdge	*GetEdge(u8 nEdgeSlotIndex)						{ return m_aEdgeSlots+nEdgeSlotIndex;}
	inline const GraphEdge	*GetEdge(u8 nEdgeSlotIndex) const { return m_aEdgeSlots+nEdgeSlotIndex;}
	inline u8			GetEdgeSlot(GraphEdge * pEdge)					{ FASSERT(pEdge - m_aEdgeSlots < m_nNumEdges); return (u8) (pEdge - m_aEdgeSlots); }
	inline const CFVec3A& GetLocation(void) const						{ return m_Location;}
	inline f32			GetRad(void) const								{ return m_fSafeRad;}
	inline f32			GetRad2(void) const								{ return m_fSafeRad*m_fSafeRad;}
	inline f32			GetHeight(void) const							{ return m_fHeightClearance;}
	inline BOOL			Is3D(void)										{ return ((m_nProps & VERTPROP_3D) == VERTPROP_3D);}
	inline BOOL			Is2D(void)										{ return ((m_nProps & VERTPROP_3D) != VERTPROP_3D);}

	GraphVert			*GetOppVert(u8 uLinkedByEdge, CAIGraph* pGraph);
	const GraphEdge		*GetEdgeTo(u16 sToVertId) const;
	GraphEdge			*GetEdgeTo(u16 sToVertId);

#if AIGRAPH_EDITOR_ENABLED
	GraphEdge			*AllocEdgeSlot(void);
	void				FreeEdgeSlot(u8 slotId);
	cchar				*ParseAscii(const char* pData, const char* pDataEnd, CAIGraph* pGraph);
	char				*EmitAscii(char* pData, char* pDataEnd, u16 nVertId);
	void				EndianFlip(void);
	void				AutoSetDimension2D(void);
	BOOL				MoveToNearestSurface(const CFVec3& rDirectionToMoveUnit);
	void				CheckAndClampClearance(const CFVec3A& rRayOrigin, const CFVec3A& rLookUpUnit);
#endif	

	CFVec3A		m_Location;									//  16
	f32			m_fSafeRad;									//   4
	f32			m_fHeightClearance;							//   4		//heightclearence in 3d verts mean half-height of cylinder (since origin of cyln is at center, not base)
	union												
	{
		u8		m_nNumEdges;
		u8		m_nVertSlotStatus;
	};														//   1
	u8			m_nProps;									//   1
	u8			m_nSubGraph;								//   1
	u8			m_nHazardId;								//   1
	GraphEdge	m_aEdgeSlots[AIGRAPH_NUM_EDGES_PER_VERT];	// 100		20*5  AIGRAPH_NUM_EDGES_PER_VERT

	static const s32 knAsciiBuffSize;					
	FCLASS_STACKMEM_ALIGN(GraphVert);
} FCLASS_ALIGN_SUFFIX;										//----
															// 128		  
															//		   NOTE!:	128 on Xbox      16 byte alignment
															//					128 on GC	      8 byte alignment
															//



//
//  CGraphPOI
//	   A Point of Interest, contained within a vert or an edge in the graph 
//

FCLASS_ALIGN_PREFIX class CGraphPoi
{
public:

	FINLINE BOOL		IsVertPoi(void)											{ return !(m_uPackedVertEdgeId & 0x8000);}
	FINLINE BOOL		IsEdgePoi(void)											{ return (m_uPackedVertEdgeId & 0x8000);}
	FINLINE u16			UnpackVertId(void)										{ return (u16)(m_uPackedVertEdgeId & 0x0fff);}
	FINLINE u8			UnpackEdgeId(void)										{ return (m_uPackedVertEdgeId & 0x7000)>>12;}
	FINLINE void		PackVertId(u16 uVertId)									{ m_uPackedVertEdgeId &=0xf000; m_uPackedVertEdgeId |= (0x0fff & uVertId);}
	FINLINE void		PackEdgeId(u16 uEdgeId)									{ m_uPackedVertEdgeId &=0x0fff; m_uPackedVertEdgeId |= (0x8000 | ((0x7 & uEdgeId)<<12));}
	void				PackVisRating(u8 uArc,u8 uRange);
	FINLINE BOOL		IsVisible(u8 uArc,u8 uRange)							{ FASSERT(uArc < 8); return (m_uVisInfo & (0x3<<((uArc*2)+uRange))) !=0;}	//is this range visible in this arc
	BOOL				IsLocVisible(CAIGraph* pGraph, const CFVec3A& Loc_WS);		//From this Loc, get a range and an arc, is it visible?
	FINLINE u8			UnpackVisRating(u8 uArc)								{ return (u8)((m_uVisInfo & (0x3<<(uArc*2)))>>(uArc*2));}
	f32					GetMin360Range(void);										//evaluate every arc and find the min vis rating
	f32					GetMax360Range(void);

	void				Init(void);
	void				Init(GraphVert* pVert, CAIGraph* pGraph, const CFVec3A& Pos_WS);
	void				Init(GraphEdge* pEdge, CAIGraph* pGraph, const CFVec3A& Pos_WS);
	void				UpdateVisInfo(CAIGraph* pGraph);
	void				SetLocation(CAIGraph* pGraph, const CFVec3A& Loc_WS);
	void				GetLocation(CAIGraph* pGraph, CFVec3A* pLoc_WS);

	BOOL				IsVisible(const CFVec3A& Pos_WS);
#if AIGRAPH_EDITOR_ENABLED
	void				EndianFlip(void);
#endif

	static FINLINE u8	DistToVisRange(f32 fDist)						{ if (fDist <= kafVisRange[0]) return 0; else if (fDist <= kafVisRange[1]) return 1; else return 2;}
	static FINLINE u8	DistSqToVisRange(f32 fDistSq)					{ if (fDistSq < kafVisRangeSq[0]) return 0; else if (fDistSq < kafVisRangeSq[1]) return 1; else return 2;}
	enum
	{
		VIS_RANGE_SHORT,
		VIS_RANGE_MEDIUM,
		VIS_RANGE_LONG,
		NUM_VIS_RANGES
	};
	static const f32 kafVisRange[NUM_VIS_RANGES];
	static const f32 kafVisRangeSq[NUM_VIS_RANGES];
	static const s32 knAsciiBuffSize;

	enum
	{
		NUM_VIS_ARCS = 8,
	};
	enum
	{
		NUM_VIS_SENSORS_PER_ARC = 2,
	};


	static CFVec3A s_aVisSensors[NUM_VIS_ARCS][NUM_VIS_SENSORS_PER_ARC];
	static CFVec3A s_aVisRays[NUM_VIS_ARCS];
	static BOOL s_bVisSensorsInitialized;
	static void InitVisSensors(void);
	static f32 CalcVisArcNormalizedRating(const CFVec3A& Origin_WS, u32 uArcNum, u32 uVisRange);


	enum
	{
		POIFLAG_OFFENSIVE_HINT	= 0x01,
		POIFLAG_CUSTOM_LOS		= 0x02,
		POIFLAG_SITEWEAPON		= 0x04,
		POIFLAG_PERCH			= 0x08,
		POIFLAG_CONSOLE			= 0x10,
		POIFLAG_GUARDTHIS		= 0x20,
	};

	
	
	u16 m_uPackedVertEdgeId;		//which vert and edge this poi is within
	u16 m_uVisInfo;					//2 bits of Data per Eight Directions, (No Range= 0x00, Short Range=0x01, Med Range = 0x10, Long Range=0x11)
	s8 m_nXOffset;					//x position of combat pt. +-(0-128) stored as a pct of owning graphvert/edge dimension
	s8 m_nYOffset;
	s8 m_nZOffset;
	union
	{
		u8 m_uFlags;
		u8 m_nPoiSlotStatus;
	};

	FCLASS_STACKMEM_NOALIGN(CGraphPoi);
} FCLASS_ALIGN_SUFFIX;


//
//
// class CAIGraph
//	  - collection of vertices and connecting edges
//	  - provides a high-level represetation of the a traversable
//      areas in a level that bots can uses when doing AI

#define AIGRAPH_DEBUG_RENDER_VERTS			0x0001
#define AIGRAPH_DEBUG_RENDER_EDGES			0x0002
#define AIGRAPH_DEBUG_RENDER_VERTVOLS		0x0004
#define AIGRAPH_DEBUG_RENDER_EDGEVOLS		0x0008
#define AIGRAPH_DEBUG_RENDER_2DEDGEVOLS		0x0010
#define AIGRAPH_DEBUG_RENDER_2DVERTVOLS		0x0020
#define AIGRAPH_DEBUG_RENDER_CULLBEHIND		0x0040
#define AIGRAPH_DEBUG_RENDER_3DMODE			0x0080
#define AIGRAPH_DEBUG_RENDER_2DMODE			0x0100
#define AIGRAPH_DEBUG_RENDER_SHOWPOI		0x0200
#define AIGRAPH_DEBUG_RENDER_ALLMASK	   (0x01ff)


#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLOROFF		(0)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLORRED		(1)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLORGREEN		(2)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLORBLUE		(3)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLORYELLOW	(4)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLORWHITE		(5)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLORMAGENTA	(6)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_COLORMASK		(0x0000001f)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVERT	(0x00000020)
#define AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVOL	(0x00000040)

#define VERT_SLOT_FREE 0xfd
#define POI_SLOT_FREE 0x80

FCLASS_ALIGN_PREFIX class CAIGraph
{
public:
						CAIGraph(void);
						CAIGraph(u16 nPreAllocVerts, void* pMemory);
	XVIRTUAL			~CAIGraph(void);

	FINLINE u16			GetNumVerts(void)								{ return m_nNumVerts;}
	FINLINE GraphVert	*GetVert(u16 nVertId)							{ FASSERT(nVertId < m_nNumVerts); return m_paVerts + nVertId;  }
	FINLINE BOOL		IsVertValid(u16 nVertid)						{ return m_paVerts[nVertid].m_nVertSlotStatus != VERT_SLOT_FREE;}
	FINLINE u16			GetNumEdges(void) const							{ return (u16)(m_nNumVerts*AIGRAPH_NUM_EDGES_PER_VERT);}
	FINLINE u16			GetNumEdgesWithUniqueVol(void) const			{ return (u16)(m_nNumVerts*(AIGRAPH_NUM_EDGES_PER_VERT/2));}
	FINLINE u16			GetVertId(const GraphVert *pV)					{ return (u16)(pV - m_paVerts); }
	u16					GetEdgeId(const GraphEdge *pE);
	const GraphEdge		*GetEdge(u16 nFromVertId, u16 nToVertId);
	FINLINE const GraphEdge		*GetEdge(u16 nGlobalEdgeId)						{ return m_paVerts[nGlobalEdgeId/AIGRAPH_NUM_EDGES_PER_VERT].GetEdge(nGlobalEdgeId%AIGRAPH_NUM_EDGES_PER_VERT);}
	s32					IsConnected( u16 nVertIdA, u16 nVertIdB); //returns 1 if A goes to B, 2 if B goes to A and 3 if both
	FINLINE GraphVert	*GetEdgeSrcVert(GraphEdge* pEdge)				{ u16 uVertId = (u16) (((GraphVert*)pEdge) - m_paVerts); FASSERT(uVertId > 0 && uVertId < GetNumVerts()); return m_paVerts + uVertId;}
	FINLINE GraphVert	*GetEdgeDestVert(GraphEdge* pEdge)				{ return m_paVerts + pEdge->m_nNextVertId;}

	BOOL				PointerizeBinaryData(void);
#if FANG_WINGC
	BOOL				ConvertGameCubeBinaryData(u32 nFileBytes);
#endif
	u32					IdentifySubGraphs(void);
	void				BindWithAccessData(CAIGraphDataAccess* pAIGraphDataAccess) { m_pAIGraphDataAccess = pAIGraphDataAccess;}

	GraphVert			*FindFarthestVert(const CFVec3A& loc);
	GraphVert			*FindClosestVert2D(const CFVec3A& loc);
	GraphVert			*FindClosestLOSVert2D(const CFVec3A& rLoc, BOOL bWithinGraphVol = FALSE);
	GraphVert			*FindClosestLOSVert2D_AdjustWithinRange(CFVec3A* pLoc, f32 fAdjustRange);
	GraphVert			*FindClosestLOSVert2DInDonut_AdjustWithinRange(const CFVec3A& rBotLoc, f32 fAdjustWithinRange, const CFVec3A& DonutOrigin, f32 fDonutInnerRad, f32 fDonutOuterRad);
	GraphVert			*FindClosestIntersectingVert2D(const CFVec3A& rRayStart, const CFVec3A& rRayEnd, f32 fUseAsVertRadius = 1.0f);
	BOOL				IsPtIn2DEdgeVol(const GraphVert* pVA, const GraphEdge* pE, const CFVec3A& rLoc, BOOL bBump = TRUE);
	BOOL				IsPtIn2DVertVol(const GraphVert* pV, const CFVec3A& rLoc, BOOL bBump = TRUE);
	
	//returns TRUE if cyln is safely in the volume of the edge
	BOOL				IsCylnSafelyInside2DEdgeVol(	const GraphVert* pVA,					//origin of the edge
														const GraphEdge* pE,					//the edge
														const CFSphereA& rCylnSphere,			//radius and location of base of y-axis cylinder
														f32 fCylnHeight,						//height of of y-axis cylinder
														BOOL bBump,								//bump rCylnSphere loc a little
														CFVec3A* pNewLoc,						//if possible to move to fit, the adjust location of hte cylinder
														BOOL *pbDidAdjust);						//did move to fit	

	BOOL				IsSphereSafelyInside3DEdgeVol(	const GraphVert* pVA,
														const GraphEdge* pE,
														const CFSphereA& rSphere,
														CFVec3A* pNewLoc,
														BOOL *pbDidAdjust);

	GraphVert			*FindClosestVert3D(const CFVec3A& loc);
	GraphVert			*FindClosestLOSVert3D(const CFVec3A& rLoc, BOOL bWithinGraphVol = FALSE);
	GraphVert			*FindClosestLOSVert3D_AdjustWithinRange(CFVec3A* pLoc, f32 fAdjustRange);
	GraphVert			*FindClosestIntersectingVert3D(const CFVec3A& rRayStart, const CFVec3A& rRayEnd, f32 fUseAsVertRadius = 1.0f);
	BOOL				IsPtIn3DEdgeVol(const GraphVert* pVA, const GraphEdge* pE, const CFVec3A& rLoc);
	BOOL				IsPtIn3DVertVol(const GraphVert* pV, const CFVec3A& rLoc);
	BOOL				CalcCeilingAlongEdge(GraphVert* pVA, GraphVert* pVB, const CFVec3A& Pt_WS, f32* pY_WS);

	
	CGraphPoi			*FindClosestPoi(const CFVec3A& loc,
										f32 fWithinRad,
										u16 uWithinVol,
										u8 uMatchPoiFlags = 0x00);
	CGraphPoi			*FindClosestIntersectingPoi(const CFVec3A& rRayStart, const CFVec3A& rRayEnd, f32 fUseAsPoiRadius = 1.0f);
	FINLINE u16			GetNumPoi(void)									{ return m_nNumPoi;}
	FINLINE CGraphPoi	*GetPoi(u16 uPoiId)								{ FASSERT(uPoiId < m_nNumPoi); return m_paPoi + uPoiId;}
	FINLINE u16			GetPoiId(CGraphPoi* pPoi)						{ return (u16)(pPoi - m_paPoi);}

	FINLINE u16 GetNumHazards(void)										{ return m_nNumHazardNames;}
	FINLINE cchar* GetHazardName(u8 uId)								{ return m_papszHazardNames[uId];}
	u8 GetHazardId(cchar* pszName);
	void AddHazard(cchar* pszName);

#if AIGRAPH_EDITOR_ENABLED
	void				StartNewGraph(void);
	BOOL				LoadFromAsciiFile(const char* szFileName);
	BOOL				LoadFromBinaryFile(const char* szFileName);
	void				AutoSetDimension2D(s32 nOneVert = -1);  //-1 means do all verts
	void				MoveVertsToNearestSurface(void);

	BOOL				PackVerts(void);
	XVIRTUAL u32		GetSizeOfBinaryData(void);
	BOOL				SaveToBinaryFile(const char* szFileName, BOOL bEndianFlip = FALSE);
	BOOL				SaveToBinaryFile(FILE *pFileStream, BOOL bEndianFlip = FALSE);
	BOOL				SaveToAsciiFile(const char* szFileName);
	s32					CalcAsciiBuffSize(void); 
	void				ParseAscii(const char* pData, const char* pDataEnd);
	char				*EmitAscii(char* pData, char* pDataEnd);
	char				*EmitAsciiPoi(char* pData, char* pDataEnd);
	cchar				*ParseAsciiPoi(cchar* pData, cchar* pDataEnd);
	void				EndianFlip(void);

	XVIRTUAL GraphVert	*AddVert(const GraphVert& rVert);
	XVIRTUAL GraphVert	*AddVert(const CFVec3A& rLocation);
	XVIRTUAL void		RemoveVert(u16 nVertId);
	void				MoveVert(u16 nVertId, const CFVec3A& rDeltaPos, BOOL bAutoDimension2D = FALSE);
	BOOL				ConnectVerts( u16 nVertIdA, u16 nVertIdB, BOOL bAutoDimension2D = TRUE);
	void				DisconnectVerts(u16 nVertIdA, u16 nVertIdB);
	XVIRTUAL GraphVert	*AllocVertSlot(void);
	XVIRTUAL void		FreeVertSlot(u16 uSlotId);

	XVIRTUAL CGraphPoi	*AddPoi(const CFVec3A& rLocation);
	XVIRTUAL void		MovePoi(u16 uPoiId, const CFVec3A& rDelta, BOOL bUpdateVisInfo = TRUE);
	XVIRTUAL void		RemovePoi(u16 uPoiId);
	XVIRTUAL void		RemoveAllPoiWithinVert(s16 nVertId);
	XVIRTUAL void		RemoveAllPoiWithinEdge(s16 nVertId, s16 nEdgeId);
	XVIRTUAL void		UpdateAllPoiWithinVert(s16 nVertId);
	XVIRTUAL void		UpdateAllPoiWithinEdge(s16 nVertId, s16 nEdgeId);
	XVIRTUAL CGraphPoi	*AllocPoiSlot(void);
	XVIRTUAL void		FreePoiSlot(u16 uSlotId);

#endif //AIGRAPH_EDITOR_ENABLED

#ifdef AIGRAPH_DEBUG
	XVIRTUAL void		Render(	const CFVec3A& CamPos,
								const CFVec3A& CamLookAt,
								f32 fHalfCosThetaRadsVisCone,
								u32 nDrawFlags = AIGRAPH_DEBUG_RENDER_EDGES,
								u32 *pauVertRenderBits = NULL,
								u32 *pauEdgeRenderBits = NULL);

#endif //AIGRAPH_DEBUG

protected:
	FINLINE u16			GetMaxVerts(void)								{ return m_nMaxVerts;};
	FINLINE u16			GetMaxPoi(void)									{ return m_nMaxPoi;};
	GraphVert			*_FindFreeVertSlot(void);
	CGraphPoi			*_FindFreePoiSlot(void);

   	u16			m_nNumVerts;
	u16			m_nNumPoi;
	u16			m_nMaxVerts;
	u16			m_nMaxPoi;
	u16			m_nNumHazardNames;
	GraphVert	*m_paVerts;
	CGraphPoi	*m_paPoi;
	union
	{
		BOOL		m_bMemoryOwner;			//not used if editor isn't enabled, but class structure is unchanged
		CAIGraphDataAccess* m_pAIGraphDataAccess;
	};
	void*		m_pVertAllocBuff;		//dynamic array space for the vert array
	void*		m_pPoiAllocBuff;		//dynamic array space for the poi array
#define MAX_HAZARD_NAME_LEN 64

	char**		m_papszHazardNames;

#if AIGRAPH_EDITOR_ENABLED==1
public:
	#define MAX_PROBLEM_VERTS 1000
	enum
	{
		VERTPROBLEMTYPE_NONE = 0,
		VERTPROBLEMTYPE_TOO_MANY_EDGES,
		VERTPROBLEMTYPE_TOO_SMALL,
		NUM_VERTPROBLEMTYPES
	};

	class ProblemVert
	{
	public:
		u16 m_uVertId;
		u8 m_uProblemType;
	};
	void InitProblemVertRecording(void);
	void RemoveAllProblemVerts(void);
	BOOL RecordProblemVert(u16 uVertId, u8 uVertProblemType);
	BOOL RemoveProblemVert(u16 uVertId, u8 uVertProblemType);


	ProblemVert m_auProblemVerts[MAX_PROBLEM_VERTS];
	u16 m_uNumProblemVerts;
	static const char* s_pszProblemTypeLabels[];
#endif
	FCLASS_STACKMEM_ALIGN(CAIGraph);
} FCLASS_ALIGN_SUFFIX;


FCLASS_ALIGN_PREFIX class CAIGraphDataAccess
{
public:
	CAIGraphDataAccess(void);
	~CAIGraphDataAccess(void);

	BOOL Init(CAIGraph* pGraph);
	BOOL GetArrayOfPoiInEdge(u16 uGlobalEdgeId, u16** ppaPoiArray, u16* uNumPoiInEdge);
	BOOL GetArrayOfPoiInVol(u16 uVolId, u16** ppaPoiArray, u16* uNumPoiInVol);
	BOOL IsPoiLocked(u16 uPoiId);
	void LockPoi(u16 uPoiId);
	void UnLockPoi(u16 uPoiId);

	BOOL IsVertLocked(u16 uVertId);
	void LockVert(u16 uVertId);
	void UnLockVert(u16 uVertId);
protected:

	BOOL InitEdgeToPoiLookup(void);
	BOOL InitVolToPoiLookup(void);

	CAIGraph* m_pGraph;
	u16* m_pauEdgeToPoi;
	u16* m_pauPoiByEdge;

	u16* m_pauVertToPoi;
	u16* m_pauPoiByVert;

	u16* m_pauVolToVert;
	u16* m_pauVertByVol;

	u16* m_pauVolToEdge;
	u16* m_pauEdgeByVol;

	u16* m_pauVolToPoi;
	u16* m_pauPoiByVol;

	u32* m_pauPoiLockBitArray;
	u32* m_pauVertLockBitArray;

	FCLASS_STACKMEM_ALIGN(CAIGraphDataAccess);
} FCLASS_ALIGN_SUFFIX;


extern BOOL aigraph_IsSurfaceWalkValid(const CFVec3& rRayTestOrigin,	// Where to start walking from.  Will be slammed to surface before walking.
								   const CFVec3& rRayTestDir,		// Which direction to walk along the surface in
								   f32 fMaxDist2,					// Maximum Distance to walk (squared)
								   f32 fStep,						// How frequently to evaluate the slope  (been using 1.0f)
								   const CFVec3& LookUpUnit,		// Up vector, by which slope is measured.
								   f32 *pfActualDist2 = NULL,		// Distance the walk was valid for
								   CFVec3* pEndPos = NULL,			// The final position at the end of the walk
								   f32 fImpossibleSlopeRads = FMATH_QUARTER_PI );		//What is a bad slope?

extern const f32 aigraph_kfSurfaceOffset;					//configure this number. Any varation of this height will be completely disregarded.
extern const f32 aigraph_kfMinUsefullPathHalfWidth;
extern f32 aigraph_RenderLODDist01;
extern f32 aigraph_RenderLODDist02;
extern f32 aigraph_fVertVisualRadius;
#ifdef XVIRTUAL
#undef XVIRTUAL

#endif

#endif //_AIGRAPH_H_
