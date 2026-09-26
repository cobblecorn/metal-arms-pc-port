
//////////////////////////////////////////////////////////////////////////////////////
// AIGraph.h - A directed graph in 3D Space used for pathfinding
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
// 02/06/02 patm       Created.
// 02/08/02 patm		implemented debug render, ascii load/save, binary load/save and pointerize
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "AIGraph.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fworld_coll.h"
#include "fmath.h"
#include "fres.h"
#include "fvis.h"
#include "Apenew.h"
#if AIGRAPH_EDITOR_ENABLED
#include "AIUtil.h"
#include <stdio.h>
#include <windows.h>  //getkeystate
#endif
#if !AIGRAPH_EDITOR_ENABLED
#include "AIGameutils.h"
#endif

const s32 GraphEdge::knAsciiBuffSize = 64*10;
const s32 GraphVert::knAsciiBuffSize = 64*14;
const s32 CGraphPoi::knAsciiBuffSize = 256;
AIGraph_LOSTestFunc _pLOSTestFunc = NULL;
#if AIGRAPH_EDITOR_ENABLED==1
const cchar* CAIGraph::s_pszProblemTypeLabels[] = 
{
	"NONE",
	"TOO MANY EDGES",
	"TOO SMALL"
};
#endif

#if AIGRAPH_EDITOR_ENABLED
f32 aigraph_GetClearanceAt(const CFVec3& rRayOrigin, const CFVec3& rLookUpUnit, f32 fMaxClearance);
#endif

const f32 aigraph_kfSurfaceOffset = 1.325f;						//configure this number. Any varation of this height will be completely disregarded.
const f32 aigraph_kfMinUsefulVolHeight = 6.0f;
const f32 aigraph_kfMinUsefullPathHalfWidth = 1.0f;
const f32 aigraph_kfMinUsefullPathHeight = 1.0f;
f32 aigraph_fVertVisualRadius = 2.0f;
f32 aigraph_RenderLODDist01 = 100.0f;
f32 aigraph_RenderLODDist02 = 250.0f;

//#define DEBUG_AIGRAPH_EDITOR_ENABLED
#ifdef DEBUG_AIGRAPH_EDITOR_ENABLED

extern CFVec3 _DrawDebugRayOrigin[];
extern CFVec3 _DrawDebugRayEnd[];
extern s32 _nDrawDebugRays;

#endif

BOOL aigraph_RayCollideWorld(const CFVec3A &rRayStart, const CFVec3A &rRayEnd);


//
//
// GraphEdge implememntation
//
//
void GraphEdge::Init(u16 sToVertId, f32 fLength, f32 fMaxSafeWidth, f32 fMaxSafeHeight, f32 fMaxPosHeightDelta)
{
	m_nProps = 0;
	m_nHazardId = 0;
	m_nNextVertId = sToVertId;
	m_fLength = fLength;
	m_fMaxSafeHeight = fMaxSafeHeight;
	m_fHalfWidth = fMaxSafeWidth;

	FMATH_CLAMP(fMaxPosHeightDelta, 0.0f, 255.0f);
	m_uMaxPosHeightDelta = (u8)fMaxPosHeightDelta;
}


GraphEdge *GraphEdge::GetReverseEdge(CAIGraph* pGraph) const
{
	GraphVert *pVB = pGraph->GetVert(m_nNextVertId);

	for (u8 i = 0; i < pVB->GetNumEdges(); i++)
	{
		GraphVert *pVA = pVB->GetOppVert(i, pGraph);

		for (u8 e = 0; e < pVA->GetNumEdges(); e++)
		{
			if (pVA->GetEdge(e) == this)
				return pVB->GetEdge(i);
		}
	}

	return NULL;
}


#if AIGRAPH_EDITOR_ENABLED
void GraphEdge::EndianFlip(void)
{
//	s16 m_nNextVertId;
	m_nNextVertId = fang_ConvertEndian(m_nNextVertId);
//	u16  m_nProps;
	m_nProps = fang_ConvertEndian(m_nProps);
//	u8	m_nHazardId;
	m_nHazardId = fang_ConvertEndian(m_nHazardId);
//	u8	m_nHazardId;
	m_nEdgeGUID = fang_ConvertEndian(m_nEdgeGUID);
//	f32 m_fLength;
	m_fLength = fang_ConvertEndian(m_fLength);
//	f32 m_fMaxSafeHeight;
	m_fMaxSafeHeight = fang_ConvertEndian(m_fMaxSafeHeight);
//	f32 m_fHalfWidth;
	m_fHalfWidth = fang_ConvertEndian(m_fHalfWidth);
//	u8 m_uMaxPosHeightDelta
	m_uMaxPosHeightDelta = fang_ConvertEndian(m_uMaxPosHeightDelta);
}


const char* GraphEdge::ParseAscii(const char* pData, const char* pDataEnd, CAIGraph* pGraph)
{
	const char* pTok = pData;

	FASSERT(m_nProps==0); //was this vert initialized? It should have been
	FASSERT(m_nHazardId == 0); //was this vert initialized? It should have been

	do 
	{
		if (aiutil_MatchAsciiToken("NEXT_VERT_ID", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nNextVertId = (s16) (aiutil_AsciiIntToken(pTok, pDataEnd));
		}
		else if (aiutil_MatchAsciiToken("LENGTH", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_fLength = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("HEIGHTCLEARANCE", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_fMaxSafeHeight = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("MAXSAFEWIDTH", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_fHalfWidth = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("MAXPOSHEIGHTDELTA", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			//old prop, just here for backward compat
			f32 fMaxPosHeightDelta = aiutil_AsciiFloatToken(pTok, pDataEnd);
			FASSERT(fMaxPosHeightDelta >= 0.0f);
			if (fMaxPosHeightDelta > 255.0f)
			{
				fMaxPosHeightDelta = 255.0f;
			}

			m_uMaxPosHeightDelta = (u8) fMaxPosHeightDelta;
		}
		else if (aiutil_MatchAsciiToken("UMAXPOSHEIGHTDELTA", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_uMaxPosHeightDelta = (u8) aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("HAZARD_ID", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nHazardId	= (u8) (aiutil_AsciiIntToken(pTok, pDataEnd));
		}
		else if (aiutil_MatchAsciiToken("EDGE_GUID", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nEdgeGUID	= (u8) (aiutil_AsciiIntToken(pTok, pDataEnd));
		}
		else if (aiutil_MatchAsciiToken("FLAGS", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			//OLD token  just parse for backward compatability
			u8 uOldFlags = (u8) (u32) (aiutil_AsciiIntToken(pTok, pDataEnd));
			if ((uOldFlags & 0xf0)==0xf0)
			{
				m_nProps |= EDGEPROP_3D;
				if (uOldFlags & 0x2)
				{
					m_nProps |= EDGEPROP_CUSTOM_VOL;
				}
			}
			else
			{
				if (uOldFlags & 0x20)
				{	//crevace
					m_nProps &= ~EDGEPROP_JUMP;
					m_nProps |= JUMPTYPE_LEDGE;
				}
			
				if (uOldFlags & 0x80)
				{
					m_nProps |= EDGEPROP_HAZARD_HAS_SWITCH;
				}
			}
		}
		else if (aiutil_MatchAsciiToken("HAZARD_DROP", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			//OLD token  just parse for backward compatability
			BOOL bSet = (aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
			m_nProps |= EDGEPROP_DOWN_ONLY*bSet;
			if (bSet)
			{
				m_nProps &= ~EDGEPROP_JUMP;
				m_nProps |= JUMPTYPE_LEDGE;
			}
		}
		else if (aiutil_MatchAsciiToken("HAZARD_JUMP", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			//OLD token  just parse for backward compatability
			BOOL bSet = (aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
			if (bSet)
			{
				m_nProps &= ~EDGEPROP_JUMP;
				m_nProps |= JUMPTYPE_OBSTACLE*bSet;
			}
		}
		else if (aiutil_MatchAsciiToken("FAILEDEDGETEST", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_FAILEDEDGETEST*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("CUSTOM_VOL", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_CUSTOM_VOL*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("IS_3D", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_3D*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("JUMPTYPE_OBSTACLE", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			BOOL bSet = (aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
			if (bSet)
			{
				m_nProps &= ~EDGEPROP_JUMP;
				m_nProps |= JUMPTYPE_OBSTACLE*bSet;
			}
		}
		else if (aiutil_MatchAsciiToken("JUMPTYPE_LEDGE", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			BOOL bSet = (aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
			if (bSet)
			{
				m_nProps &= ~EDGEPROP_JUMP;
				m_nProps |= JUMPTYPE_LEDGE*bSet;
			}
		}
		else if (aiutil_MatchAsciiToken("JUMPTYPE_V2V", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			BOOL bSet = (aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
			if (bSet)
			{
				m_nProps &= ~EDGEPROP_JUMP;
				m_nProps |= JUMPTYPE_VERT_2_VERT*bSet;
			}
		}
		else if (aiutil_MatchAsciiToken("JUMPTYPE_HOVER", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			BOOL bSet = (aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
			if (bSet)
			{
				m_nProps &= ~EDGEPROP_JUMP;
				m_nProps |= JUMPTYPE_HOVER*bSet;
			}
		}
		else if (aiutil_MatchAsciiToken("DOWN_ONLY", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_DOWN_ONLY*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("UP_ONLY", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_UP_ONLY*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("HAZARD_DOOR", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_HAZARD_DOOR*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("HAZARD_LIFT", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_HAZARD_LIFT*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("HAZARD_SWITCH", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_HAZARD_HAS_SWITCH*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("HAZARD_DESTRUCT", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_HAZARD_DESTRUCTABLE*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("HAZARD_BRIDGE", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= EDGEPROP_HAZARD_BRIDGE*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("END_EDGE", pTok, pDataEnd))
		{
			break;
		}

	} while (pTok = aiutil_NextAsciiToken(pTok, pDataEnd));

	return pTok;
}


char* GraphEdge::EmitAscii(char* pData, char* pDataEnd)
{
	char* pTok = pData;


	if (Is3D())
	{
		//3d edges shouldn't have these props
		m_nProps &=~ (EDGEPROP_JUMP| EDGEPROP_HAZARD_DOOR			
						| EDGEPROP_HAZARD_LIFT			
						| EDGEPROP_HAZARD_HAS_SWITCH		
						| EDGEPROP_HAZARD_DESTRUCTABLE	
						| EDGEPROP_HAZARD_BRIDGE			
						| EDGEPROP_DOWN_ONLY
						| EDGEPROP_UP_ONLY	 );
	}

	pTok = aiutil_EmitAsciiLabel("BEGIN_EDGE", pTok, pDataEnd);
	aiutil_IncAsciiIndent();
	pTok = aiutil_EmitAsciiInt("NEXT_VERT_ID",				m_nNextVertId ,									pTok, pDataEnd);
	pTok = aiutil_EmitAsciiFloat("LENGTH",					m_fLength,										pTok, pDataEnd);
	pTok = aiutil_EmitAsciiFloat("HEIGHTCLEARANCE",			m_fMaxSafeHeight,								pTok, pDataEnd);
	pTok = aiutil_EmitAsciiFloat("MAXSAFEWIDTH",			m_fHalfWidth ,									pTok, pDataEnd);
	pTok = aiutil_EmitAsciiInt("UMAXPOSHEIGHTDELTA",		m_uMaxPosHeightDelta,							pTok, pDataEnd);
	pTok = aiutil_EmitAsciiInt("HAZARD_ID",					m_nHazardId,									pTok, pDataEnd);
	pTok = aiutil_EmitAsciiInt("EDGE_GUID",					m_nEdgeGUID,									pTok, pDataEnd);
	
	if (m_nProps & EDGEPROP_FAILEDEDGETEST)			pTok = aiutil_EmitAsciiInt("FAILEDEDGETEST",			1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_CUSTOM_VOL)				pTok = aiutil_EmitAsciiInt("CUSTOM_VOL",				1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_3D)						pTok = aiutil_EmitAsciiInt("IS_3D",						1,		pTok, pDataEnd);



	if ((m_nProps & EDGEPROP_JUMP) == JUMPTYPE_OBSTACLE) pTok = aiutil_EmitAsciiInt("JUMPTYPE_OBSTACLE",			1,		pTok, pDataEnd);
	else if ((m_nProps & EDGEPROP_JUMP) == JUMPTYPE_LEDGE) pTok = aiutil_EmitAsciiInt("JUMPTYPE_LEDGE",				1,		pTok, pDataEnd);
	else if ((m_nProps & EDGEPROP_JUMP) ==	JUMPTYPE_VERT_2_VERT) pTok = aiutil_EmitAsciiInt("JUMPTYPE_V2V",		1,		pTok, pDataEnd);
	else if ((m_nProps & EDGEPROP_JUMP) ==	JUMPTYPE_HOVER) pTok = aiutil_EmitAsciiInt("JUMPTYPE_HOVER",			1,		pTok, pDataEnd);


	if (m_nProps & EDGEPROP_DOWN_ONLY)				pTok = aiutil_EmitAsciiInt("DOWN_ONLY",					1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_UP_ONLY)				pTok = aiutil_EmitAsciiInt("UP_ONLY",					1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_HAZARD_DOOR)			pTok = aiutil_EmitAsciiInt("HAZARD_DOOR",				1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_HAZARD_LIFT)			pTok = aiutil_EmitAsciiInt("HAZARD_LIFT",				1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_HAZARD_HAS_SWITCH)		pTok = aiutil_EmitAsciiInt("HAZARD_SWITCH",				1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_HAZARD_DESTRUCTABLE)	pTok = aiutil_EmitAsciiInt("HAZARD_DESTRUCT",			1,		pTok, pDataEnd);
	if (m_nProps & EDGEPROP_HAZARD_BRIDGE)			pTok = aiutil_EmitAsciiInt("HAZARD_BRIDGE",				1,		pTok, pDataEnd);
	aiutil_DecAsciiIndent();								
	pTok = aiutil_EmitAsciiLabel("END_EDGE", pTok, pDataEnd);
	
	return pTok;
}



const f32 kEdgeWalkStepDist = 1.0f;
const f32 GraphEdge::kfMAX_EDGE_CLEARANCE = 20.0f;
void GraphEdge::AutoSetDimension2D(GraphVert* pVA, GraphVert* pVB, u16 sFromVertId, const CFVec3A& LookUpUnit, f32 fInitialMaxWidth)
{
	
	FASSERT(m_nNextVertId	!= 0); //unitialized vert probably
	//customized edges don't auto dimension
	if (m_nProps & EDGEPROP_CUSTOM_VOL)
		return;

	FCollImpact_t oCollInfo;
	CFVec3A EdgeVec;
	EdgeVec.Sub(pVB->GetLocation(), pVA->GetLocation());
	//distance2
	m_fLength = fmath_AcuSqrt(EdgeVec.MagSq());	//pgm doing this since we want to have minimal error for getting fLength2*fLength2 later and ,it is tool time anyway!  
	m_fMaxSafeHeight = kfMAX_EDGE_CLEARANCE;
	m_fHalfWidth = fInitialMaxWidth;

	EdgeVec.Unitize();
	f32 fStepSoFar = 0.0f;
	CFVec3A rayOrigin = pVA->GetLocation();
	f32 fMaxY = rayOrigin.GetY();

	m_nProps &= ~EDGEPROP_FAILEDEDGETEST;  //clear this to start, will be retested
	
	CFVec3A RightVec;
	RightVec.Cross(LookUpUnit, EdgeVec);

	while (fStepSoFar < m_fLength)
	{
		f32 fLen2;
		aigraph_IsSurfaceWalkValid(rayOrigin.v3, RightVec.v3, m_fHalfWidth*m_fHalfWidth, 1.0f, LookUpUnit.v3, &fLen2);
		FASSERT(fLen2 >= 0.0f);
		m_fHalfWidth = fmath_AcuSqrt(fLen2);

		aigraph_IsSurfaceWalkValid(rayOrigin.v3, -RightVec.v3, m_fHalfWidth*m_fHalfWidth, 1.0f, LookUpUnit.v3, &fLen2);
		FASSERT(fLen2 >= 0.0f);
		m_fHalfWidth = fmath_AcuSqrt(fLen2);
		
		m_fMaxSafeHeight = aigraph_GetClearanceAt(rayOrigin.v3, LookUpUnit.v3, m_fMaxSafeHeight);
		m_fMaxSafeHeight = aigraph_GetClearanceAt(rayOrigin.v3+(RightVec.v3*m_fHalfWidth), LookUpUnit.v3, m_fMaxSafeHeight);
		m_fMaxSafeHeight = aigraph_GetClearanceAt(rayOrigin.v3-(RightVec.v3*m_fHalfWidth), LookUpUnit.v3, m_fMaxSafeHeight);
		

		CFVec3 SurfaceLoc;
		if (aigraph_IsSurfaceWalkValid(rayOrigin.v3, EdgeVec.v3, kEdgeWalkStepDist*kEdgeWalkStepDist, 1.0f, LookUpUnit.v3, &fLen2, &SurfaceLoc))
		{
			fStepSoFar += fmath_AcuSqrt((SurfaceLoc-rayOrigin.v3).Mag2());
			rayOrigin.Set(SurfaceLoc);
			if (SurfaceLoc.y > fMaxY)
			{
				fMaxY = SurfaceLoc.y;
			}
		}
		else
		{
			m_fHalfWidth = aigraph_kfMinUsefullPathHalfWidth;
			m_nProps |= EDGEPROP_FAILEDEDGETEST;
			if (fMaxY < pVB->GetLocation().y)
			{
				fMaxY = pVB->GetLocation().y;
			}
			break;
		}
	}

	if (m_fHalfWidth < aigraph_kfMinUsefullPathHalfWidth)
	{
		m_fHalfWidth = aigraph_kfMinUsefullPathHalfWidth;
		m_nProps |= EDGEPROP_FAILEDEDGETEST;
	}

	if (m_fMaxSafeHeight < aigraph_kfMinUsefullPathHeight)
	{
		m_fMaxSafeHeight = aigraph_kfMinUsefullPathHeight;
		m_nProps |= EDGEPROP_FAILEDEDGETEST;
	}

	f32 fMaxPosHeightDelta = fMaxY - pVA->GetLocation().y;
	FMATH_CLAMP(fMaxPosHeightDelta, 0.0f,  255.0f);
	m_uMaxPosHeightDelta = (u8) fMaxPosHeightDelta;
}


void GraphEdge::ChangeDimension(f32 fLength, f32 fWidth, f32 fHeight, f32 fPosHeightDelta)
{
	m_fLength = fLength;
	if (!(m_nProps & VERTPROP_CUSTOM_VOL))
	{
		m_fHalfWidth = fWidth;
		m_fMaxSafeHeight = fHeight;
	}
	FMATH_CLAMP(fPosHeightDelta, 0.0f,  255.0f);
	m_uMaxPosHeightDelta = (u8) fPosHeightDelta;
}


#endif //AIGRAPH_EDITOR_ENABLED



//
//
// GraphVert implementation 
//
//
void GraphVert::Init(void)
{
	m_Location.Zero();
	m_fSafeRad = 0.0f;
	m_nNumEdges = 0;
	m_nProps = 0;
	m_nSubGraph = 0;
	m_fHeightClearance = 0.0f;
	m_nHazardId = 0;
}

GraphVert *GraphVert::GetOppVert(u8 uLinkedByEdge, CAIGraph* pGraph)
{
	FASSERT(uLinkedByEdge < GetNumEdges());

	return pGraph->GetVert(m_aEdgeSlots[uLinkedByEdge].m_nNextVertId);
}


const GraphEdge *GraphVert::GetEdgeTo(u16 sVertId) const
{
	for (s32 i = 0; i < m_nNumEdges; i++)
	{
		if (m_aEdgeSlots[i].m_nNextVertId == sVertId)
		{
			return &(m_aEdgeSlots[i]);
		}
	}
	return NULL;
}


GraphEdge * GraphVert::GetEdgeTo(u16 sVertId)
{
	for (s32 i = 0; i < m_nNumEdges; i++)
	{
		if (m_aEdgeSlots[i].m_nNextVertId == sVertId)
		{
			return &(m_aEdgeSlots[i]);
		}
	}
	return NULL;
}


#if AIGRAPH_EDITOR_ENABLED==1


void CAIGraph::RemoveAllProblemVerts(void)
{
	fang_MemSet(m_auProblemVerts, VERTPROBLEMTYPE_NONE, sizeof(m_auProblemVerts));
	m_uNumProblemVerts = 0;
}


void CAIGraph::InitProblemVertRecording(void)
{
	RemoveAllProblemVerts();
}


BOOL CAIGraph::RecordProblemVert(u16 uVertId, u8 uVertProblemType)
{
	BOOL bDidIt = FALSE;
	for (u32 i = 0; i < MAX_PROBLEM_VERTS; i++)
	{
		if (m_auProblemVerts[i].m_uProblemType == VERTPROBLEMTYPE_NONE)
		{
			m_auProblemVerts[i].m_uVertId = uVertId;
			m_auProblemVerts[i].m_uProblemType = uVertProblemType;
			m_uNumProblemVerts++;
			bDidIt = TRUE;
			break;
		}
	}
	return bDidIt;
}


BOOL CAIGraph::RemoveProblemVert(u16 uVertId, u8 uVertProblemType)
{
	u32 uChecked = 0;
	BOOL bDidIt = FALSE;
	for (u32 i = 0; i < MAX_PROBLEM_VERTS && uChecked < m_uNumProblemVerts; i++)
	{
		if (m_auProblemVerts[i].m_uProblemType != VERTPROBLEMTYPE_NONE)
		{
			uChecked++;
			if (m_auProblemVerts[i].m_uVertId == uVertId && m_auProblemVerts[i].m_uProblemType == uVertProblemType)
			{
				m_auProblemVerts[i].m_uVertId = 0;
				m_auProblemVerts[i].m_uProblemType = VERTPROBLEMTYPE_NONE;
				FASSERT(m_uNumProblemVerts > 0);
				m_uNumProblemVerts--;
				bDidIt = TRUE;
				break;
			}
		}
	}
	return bDidIt;
}
#endif

#if AIGRAPH_EDITOR_ENABLED

GraphEdge *GraphVert::AllocEdgeSlot(void)
{
	if (m_nNumEdges < AIGRAPH_NUM_EDGES_PER_VERT)
	{
		m_nNumEdges++;

		GraphEdge* pEdge = &(m_aEdgeSlots[m_nNumEdges-1]);
		pEdge->Init(0, 0.0f, 0.0f, 0.0f, 0.0f);
		return pEdge;
	}
	return NULL; //no free slots in this vert
}


void GraphVert::FreeEdgeSlot(u8 slotId)
{
	FASSERT(slotId >= 0 && slotId < m_nNumEdges);
	FASSERT(m_nNumEdges >0);

	m_aEdgeSlots[slotId].m_nNextVertId = 0;

	//tidy up the edge slot array
	if (slotId < m_nNumEdges - 1)
	{
		m_aEdgeSlots[slotId] = m_aEdgeSlots[m_nNumEdges-1];
	}
	m_nNumEdges--;
}


void GraphVert::EndianFlip(void)
{
	m_fHeightClearance = fang_ConvertEndian(m_fHeightClearance);
	for (int i =0; i < AIGRAPH_NUM_EDGES_PER_VERT; i++)
	{
		m_aEdgeSlots[i].EndianFlip();
	}
	m_Location.x = fang_ConvertEndian(m_Location.x);
	m_Location.y = fang_ConvertEndian(m_Location.y);
	m_Location.z = fang_ConvertEndian(m_Location.z);
	m_fSafeRad = fang_ConvertEndian(m_fSafeRad);
	m_nNumEdges = fang_ConvertEndian(m_nNumEdges);
	m_nHazardId = fang_ConvertEndian(m_nHazardId);
	m_nProps = fang_ConvertEndian(m_nProps);
}


const char* GraphVert::ParseAscii(const char* pData, const char* pDataEnd, CAIGraph* pGraph)
{
	const char* pTok = pData;

	FASSERT(m_nNumEdges == 0);  //was this vert initialized? Should have been.

	do 
	{
		if (aiutil_MatchAsciiToken("LOC_X", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_Location.x = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("LOC_Y", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_Location.y = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("LOC_Z", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_Location.z = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("SAFE_RADIUS", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_fSafeRad = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("FLAGS", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			u8 uOldFlags = (u8) (u32) (aiutil_AsciiIntToken(pTok, pDataEnd));
			if (uOldFlags & 0x01)
			{
				m_nProps |= VERTPROP_CUSTOM_VOL;
			}
			if (uOldFlags & 0x02)
			{
				m_nProps |= VERTPROP_3D;
			}
		}
		else if (aiutil_MatchAsciiToken("HEIGHTCLEARANCE", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_fHeightClearance = aiutil_AsciiFloatToken(pTok, pDataEnd);
		}
		else if (aiutil_MatchAsciiToken("CUSTOM_VOL", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= VERTPROP_CUSTOM_VOL*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("IS_3D", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nProps |= VERTPROP_3D*(aiutil_AsciiIntToken(pTok, pDataEnd) != 0);
		}
		else if (aiutil_MatchAsciiToken("HAZARD_ID", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			m_nHazardId	= (u8) (aiutil_AsciiIntToken(pTok, pDataEnd));
		}
		else if (aiutil_MatchAsciiToken("BEGIN_EDGE", pTok, pDataEnd))
		{
			GraphEdge* pEdge = AllocEdgeSlot();
			if (pEdge)
			{
				pTok = pEdge->ParseAscii(pTok, pDataEnd, pGraph);
			}
		}
		else if (aiutil_MatchAsciiToken("END_VERT", pTok, pDataEnd))
		{
			break;
		}

	} while (pTok = aiutil_NextAsciiToken(pTok, pDataEnd));

	return pTok;
}


char* GraphVert::EmitAscii(char* pData, char* pDataEnd, u16 nVertId)
{
	char* pTok = pData;
	pTok = aiutil_EmitAsciiInt("BEGIN_VERT", nVertId, pTok, pDataEnd);
	aiutil_IncAsciiIndent();
	pTok = aiutil_EmitAsciiLabel("",pTok, pDataEnd); //blank line
	pTok = aiutil_EmitAsciiFloat("LOC_X", m_Location.x, pTok, pDataEnd);
	pTok = aiutil_EmitAsciiFloat("LOC_Y", m_Location.y, pTok, pDataEnd);
	pTok = aiutil_EmitAsciiFloat("LOC_Z", m_Location.z, pTok, pDataEnd);
	pTok = aiutil_EmitAsciiFloat("SAFE_RADIUS", m_fSafeRad, pTok, pDataEnd);
	pTok = aiutil_EmitAsciiFloat("HEIGHTCLEARANCE", m_fHeightClearance, pTok, pDataEnd);
	pTok = aiutil_EmitAsciiInt("CUSTOM_VOL", (m_nProps & VERTPROP_CUSTOM_VOL)!=0 , pTok, pDataEnd);
	pTok = aiutil_EmitAsciiInt("IS_3D", (m_nProps & VERTPROP_3D)!=0 , pTok, pDataEnd);
	pTok = aiutil_EmitAsciiInt("HAZARD_ID", m_nHazardId , pTok, pDataEnd);
	pTok = aiutil_EmitAsciiLabel("",pTok, pDataEnd); //blank line

	FASSERT(m_nNumEdges <= AIGRAPH_NUM_EDGES_PER_VERT && m_nNumEdges >= 0);
	for (s32 i = 0; i < m_nNumEdges; i++)
	{
		pTok = m_aEdgeSlots[i].EmitAscii(pTok, pDataEnd);
		pTok = aiutil_EmitAsciiLabel("",pTok, pDataEnd);
	}
	aiutil_DecAsciiIndent();
	pTok = aiutil_EmitAsciiLabel("END_VERT", pTok, pDataEnd);
	return pTok;
}


BOOL GraphVert::MoveToNearestSurface(const CFVec3& rDirectionToMoveUnit)
{
	FCollImpact_t oCollInfo;
	CFVec3 localrayOrigin = GetLocation().v3;
	localrayOrigin.y+=1.0f;
	CFVec3 localrayDir = rDirectionToMoveUnit*(100.0f);	 //max that it will move
	CFVec3 localrayEnd = localrayDir + localrayOrigin;

	CFVec3A vStart, vEnd;
	vStart.Set(localrayOrigin);
	vEnd.Set(localrayEnd);
	BOOL bCollideWithSurf = fworld_FindClosestImpactPointToRayStart(&oCollInfo, &vStart, &vEnd );
	if (bCollideWithSurf)
	{
		m_Location.Set(oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset));	 //little fudgy here.  This adjust to surface point insn't quite perfect. But, since
		                                                                             //aigraph_kfSurfaceOffset is assumed to be prety darn close, then we're happy!
		return TRUE;
	}
	return FALSE; //found no surface in that dir
}


void GraphVert::CheckAndClampClearance(const CFVec3A& rRayOrigin, const CFVec3A& rLookUpUnit)
{
	FCollImpact_t CollInfo;
	CFVec3A RayEnd;
	CFVec3A RayToImpact;
	RayEnd = rLookUpUnit;
	RayEnd.Mul(m_fHeightClearance);
	RayEnd.Add(rRayOrigin);

	if (fworld_FindClosestImpactPointToRayStart(&CollInfo, &rRayOrigin, &RayEnd))
	{
		RayToImpact.Set(CollInfo.ImpactPoint);
		RayToImpact.Sub(rRayOrigin);
		f32 heightOfCollision = RayToImpact.Dot(rLookUpUnit);
		if (heightOfCollision < m_fHeightClearance)
		{
			m_fHeightClearance = heightOfCollision;
		}
	}
}


const s32 _knNumSpokes2D = 8;
BOOL _bSpokeRaysInitialized = 0;
CFVec3 _aSpokeDirs[_knNumSpokes2D];
void _InitSpokeRays(const CFVec3& rLookUpUnit)
{
	f32 fRadsPerArc = FMATH_2PI/(f32)_knNumSpokes2D;

	CFVec3 InitSpokeDir = rLookUpUnit;		//make any unit vector that is perp to the up
	InitSpokeDir.RotateX(FMATH_HALF_PI);
	InitSpokeDir.RotateY(FMATH_HALF_PI);	
	InitSpokeDir.Unitize();

	CFQuat rotQuat;
	CFMtx43 Mtx43;
	for (s32 i = 0; i < _knNumSpokes2D; i++)
	{
		rotQuat.BuildQuat(rLookUpUnit, (f32)i * fRadsPerArc);
		rotQuat.BuildMtx( Mtx43.m33 );

		_aSpokeDirs[i] = Mtx43.MultDir( InitSpokeDir );
		_aSpokeDirs[i].Unitize();
	}

	_bSpokeRaysInitialized  = 1;
}


void GraphVert::AutoSetDimension2D(void)
{
	//customized edges don't auto dimension
	if (m_nProps & VERTPROP_CUSTOM_VOL)
		return;

	//ceiling at cylinder center.
	CFVec3A LookUpUnit(0.0f, 1.0f, 0.0f);
	f32 fStep = 1.0f;								//configure this (ground units prefer to have an up vector of 1.0f

	//
	//  Set safe radius of this vertex
	//
	if (!_bSpokeRaysInitialized )
	{
		_InitSpokeRays(LookUpUnit.v3);
	}

	m_fSafeRad = 25.0f;  //largest possible size
	f32 fRadsPerArc = FMATH_2PI/(f32)_knNumSpokes2D;
	
	for (s32 i = 0; i < _knNumSpokes2D; i++)
	{

		f32 fOkDistance2;
		aigraph_IsSurfaceWalkValid(GetLocation().v3, _aSpokeDirs[i], GetRad2(), fStep, LookUpUnit.v3, &fOkDistance2);
		if (fOkDistance2 < GetRad2())
		{
			m_fSafeRad = fmath_AcuSqrt(fOkDistance2);
		}
		else if (fOkDistance2 <= 0.0f)
		{
			m_fSafeRad = 0.0f;
			break;
		}
	}

/*
// Super Anal Vert Vol walkable surface determination
// commented out because it is really really slow on complicated levels due
// to the insane number of raytests

	BOOL bRingIsGood = 0;
	f32 fGoodRadius = m_fSafeRad;
	f32 fRadius = m_fSafeRad;
	u32 nSpokeGoRoundCounter = 0;
	f32 fSinHalfTheta = fmath_Sin(fRadsPerArc*0.5f);
	f32 fSpokeTipDist = 2.0f*fSinHalfTheta*fRadius;  //fSpokeTipDist is the distance between end points on the spokes
	while (fSpokeTipDist > 6.0f)
	{
		CFVec3 LastSpokeTip;

		for (s32 i = 0; i < _knNumSpokes2D+1; i++)
		{
			CFVec3 SpokeTip = _aSpokeDirs[i%_knNumSpokes2D]*fRadius;
			f32 fLen;
			CFVec3 SurfaceLoc;
			BOOL bFoundTip = aigraph_IsSurfaceWalkValid(GetLocation().v3, _aSpokeDirs[i%_knNumSpokes2D], fRadius*fRadius, fStep, LookUpUnit.v3, &fLen, &SurfaceLoc);
			FASSERT(bFoundTip);

			SpokeTip = SurfaceLoc;

			if (i > 0)
			{

				CFVec3 RayTestDir;
				RayTestDir = SpokeTip - LastSpokeTip;
				f32 fSpokeToSpokeDist2 = RayTestDir.Mag2();
				RayTestDir.Unitize();

				if (!aigraph_IsSurfaceWalkValid(GetLocation().v3, RayTestDir, fSpokeToSpokeDist2, fStep, LookUpUnit.v3))
				{
					break;  //stop going around, this ring is no good.  kick out to outter loop and try a smaller ring
				}
			}
			
			LastSpokeTip = SpokeTip;
		}
		//did we make it all the way around.. connecting all spokes?
		fRadius -= 1.0f;  //shrink the radius, and get ready for another go-round
		fSpokeTipDist = 2.0f*fSinHalfTheta*fRadius;  //fSpokeTipDist is the distance between end points on the spokes
		if (i <  _knNumSpokes2D+1)
		{
			fGoodRadius = fRadius;
		}
		nSpokeGoRoundCounter++;
	}

	m_fSafeRad = fGoodRadius;
*/

	//
	//   Set Clearance of this vertex
	//
	if (m_fSafeRad > 0.0f)
	{
		CFVec3A rayOrigin = GetLocation();
		m_fHeightClearance =  GraphEdge::kfMAX_EDGE_CLEARANCE;  //default maximum clearance at verterx
		CheckAndClampClearance(GetLocation(), LookUpUnit); //check clearance at vert location  (might be reduced by later as we snoop around to find out our maxRadius

		for (i = 0; i < _knNumSpokes2D; i++)
		{
			rayOrigin.Set(_aSpokeDirs[i]);
			rayOrigin.Mul(m_fSafeRad);
			rayOrigin.Add(GetLocation());
			CheckAndClampClearance(rayOrigin, LookUpUnit); //check clearance at spoke tip

			if (m_fSafeRad > 10.0f)
			{
				rayOrigin.Set(_aSpokeDirs[i]);
				rayOrigin.Mul(m_fSafeRad*0.5f);
				rayOrigin.Add(GetLocation());
				CheckAndClampClearance(rayOrigin, LookUpUnit); //check clearance at halfway down the spoke if spoke is long
			}
		}
	}
}
#endif


const f32 CGraphPoi::kafVisRange[] = {4.0f, 25.0f, 50.0f};
const f32 CGraphPoi::kafVisRangeSq[] = {4.0f*4.0f, 25.0f*25.0f, 50.0f*50.0f};
CFVec3A	CGraphPoi::s_aVisSensors[NUM_VIS_ARCS][NUM_VIS_SENSORS_PER_ARC];
CFVec3A CGraphPoi::s_aVisRays[NUM_VIS_ARCS];
BOOL CGraphPoi::s_bVisSensorsInitialized = FALSE;

void CGraphPoi::InitVisSensors(void)
{
	if (!s_bVisSensorsInitialized)
	{
		f32 fRadsPerArc = FMATH_2PI /(f32) NUM_VIS_ARCS;
		for (u32 i = 0; i < NUM_VIS_ARCS; i++)
		{
			CFVec3A Axis(0.0f, 0.0f, 1.0f);
			for (u32 j = 0; j < NUM_VIS_SENSORS_PER_ARC; j++)
			{
				Axis.Set(CFVec3A::m_UnitAxisZ);
				Axis.RotateY(fRadsPerArc*i + (j+1)*(fRadsPerArc/(NUM_VIS_SENSORS_PER_ARC+1)));  //plus one is so that range boundries are not actually tested
				Axis.Unitize();
				s_aVisSensors[i][j] = Axis;
			}
			Axis.Set(CFVec3A::m_UnitAxisZ);
			Axis.RotateY(fRadsPerArc*i);
			Axis.Unitize();
			s_aVisRays[i] = Axis;
		}
		s_bVisSensorsInitialized = TRUE;
	}
}


void CGraphPoi::GetLocation(CAIGraph* pGraph, CFVec3A* pLoc_WS)
{
	CFVec3A DeltaPos;
	GraphVert* pV = pGraph->GetVert(UnpackVertId());
	pLoc_WS->Set((f32) m_nXOffset, (f32) m_nYOffset, (f32) m_nZOffset);
	pLoc_WS->Add(pV->GetLocation());
}

const f32 kfRadsPerVisArc = FMATH_2PI /(f32) CGraphPoi::NUM_VIS_ARCS;
const f32 kOOfRadsPerVisArc = 1.0f/(FMATH_2PI /(f32) CGraphPoi::NUM_VIS_ARCS);

BOOL CGraphPoi::IsLocVisible(CAIGraph* pGraph, const CFVec3A& Loc_WS)
{
	CFVec3A PoiLoc;
	GetLocation(pGraph, &PoiLoc);

	f32 fRad = fmath_Atan(Loc_WS.x - PoiLoc.x, Loc_WS.z - PoiLoc.z);
	if (fRad < 0.0f)
	{
		fRad = FMATH_2PI  + fRad;
	}
	f32 fDistSq = Loc_WS.DistSq(PoiLoc);
	return IsVisible((u8) (fRad*kOOfRadsPerVisArc), DistSqToVisRange(fDistSq));
}

//evaluate every arc and find the min vis rating
f32 CGraphPoi::GetMin360Range(void)
{
	u8 uMinRating = VIS_RANGE_LONG;
	u8 uRating;

	for (u8 i = 0; i < NUM_VIS_ARCS; i++)
	{
		uRating = UnpackVisRating(i);
		if (uRating < uMinRating)
		{
			uMinRating = uRating;
		}
	}
	if (uMinRating)
	{
		return kafVisRange[uMinRating-1];
	}
	return 0.0f;
}

//evaluate every arc and find the min vis rating
f32 CGraphPoi::GetMax360Range(void)
{
	u8 uMaxRating = VIS_RANGE_SHORT;
	u8 uRating;

	for (u8 i = 0; i < NUM_VIS_ARCS; i++)
	{
		uRating = UnpackVisRating(i);
		if (uRating > uMaxRating)
		{
			uMaxRating = uRating;
		}
	}
	if (uMaxRating)
	{
		return kafVisRange[uMaxRating-1];
	}
	return 0.0f;
}


#if AIGRAPH_EDITOR_ENABLED

f32 CGraphPoi::CalcVisArcNormalizedRating(const CFVec3A& Origin_WS, u32 uArcNum, u32 uVisRange)
{
	InitVisSensors();
	CFVec3A End;
	u32 uLosCounter = 0;
	for (u32 i = 0; i < NUM_VIS_SENSORS_PER_ARC; i++)
	{
		End = s_aVisSensors[uArcNum][i];
		End.Mul(kafVisRange[uVisRange]);
		End.Add(Origin_WS);

		uLosCounter += !aigraph_RayCollideWorld(Origin_WS, End);
	}
	return (f32)uLosCounter/NUM_VIS_SENSORS_PER_ARC;
}



void CGraphPoi::EndianFlip(void)
{
	m_uPackedVertEdgeId = fang_ConvertEndian(m_uPackedVertEdgeId);
	m_uVisInfo = fang_ConvertEndian(m_uVisInfo);
}


void CGraphPoi::Init(void)
{
	m_uPackedVertEdgeId = 0;
	m_uVisInfo = 0;			
	m_nXOffset = 0;			
	m_nYOffset = 0;
	m_nZOffset = 0;
	m_uFlags = 0;
}


void CGraphPoi::Init(GraphVert* pVert, CAIGraph* pGraph, const CFVec3A& Loc_WS)
{
	Init();
	PackVertId(pGraph->GetVertId(pVert));
	SetLocation(pGraph, Loc_WS);
	UpdateVisInfo(pGraph);
}


void CGraphPoi::Init(GraphEdge* pEdge, CAIGraph* pGraph, const CFVec3A& Loc_WS)
{
	Init();
	GraphVert* pSrcVert = pGraph->GetEdgeSrcVert(pEdge);
	PackVertId(pGraph->GetVertId(pSrcVert));
	PackEdgeId(pSrcVert->GetEdgeSlot(pEdge));
	SetLocation(pGraph, Loc_WS);
	UpdateVisInfo(pGraph);
}


void CGraphPoi::SetLocation(CAIGraph* pGraph, const CFVec3A& Loc_WS)
{
	CFVec3A DeltaPos;
	GraphVert* pV = pGraph->GetVert(UnpackVertId());
	DeltaPos.Sub(Loc_WS, pV->GetLocation());

   FMATH_CLAMP(DeltaPos.x, -128.0f,  128.0f);
   FMATH_CLAMP(DeltaPos.y, -128.0f,  128.0f);
   FMATH_CLAMP(DeltaPos.z, -128.0f,  128.0f);
   m_nXOffset = (s8) DeltaPos.x;
   m_nYOffset = (s8) DeltaPos.y;
   m_nZOffset = (s8) DeltaPos.z;
}


void CGraphPoi::PackVisRating(u8 uArc,u8 uRange)
{
	m_uVisInfo &= ~(uRange<<(uArc*2));
	m_uVisInfo |= uRange<<(uArc*2);
}


void CGraphPoi::UpdateVisInfo(CAIGraph* pGraph)
{
	GraphVert* pV = pGraph->GetVert(UnpackVertId());
	CFVec3A Loc;

	m_uVisInfo = 0;	 //nothing is visible
	GetLocation(pGraph,	&Loc);
	for (u8 i = 0; i < NUM_VIS_ARCS; i++)
	{
		for (u8 j = 0; j < NUM_VIS_RANGES; j++)
		{
			f32 fRating = CalcVisArcNormalizedRating(Loc, i, j);  //how well can we see into the specified arc at range

			if (fRating < 1.0f)
			{
			   break;
			}
		}
		if (j)
		{
			PackVisRating(i, j);
		}
	}
}

#endif  //AIGRAPH_EDITOR_ENABLED

//
//
// CAIGraph implementation 
//
//
CAIGraph::CAIGraph(void)
: m_nNumVerts(0),
  m_nMaxVerts(0),
  m_paVerts(NULL),
  m_nNumPoi(0),
  m_nMaxPoi(0),
  m_paPoi(NULL),
  m_bMemoryOwner(FALSE),
  m_pVertAllocBuff(NULL),
  m_pPoiAllocBuff(NULL),
  m_nNumHazardNames(0),
  m_papszHazardNames(NULL)
{
#if AIGRAPH_EDITOR_ENABLED==1
	InitProblemVertRecording();
#endif
#if AIGRAPH_EDITOR_ENABLED

	m_papszHazardNames =  new char*[255];
	char* pMem = new char[255*MAX_HAZARD_NAME_LEN];
	for (u32 i = 0; i < 255; i++)
	{
		m_papszHazardNames[i] = pMem+i*MAX_HAZARD_NAME_LEN;
	}

#endif
}


CAIGraph::CAIGraph(u16 nPreAllocVerts, void* pMemory)
: m_nNumVerts(0),
  m_nMaxVerts(nPreAllocVerts),
  m_paVerts((GraphVert*)pMemory),
  m_nNumPoi(0),
  m_nMaxPoi(0),
  m_paPoi(NULL),
  m_bMemoryOwner(FALSE),
  m_pVertAllocBuff(NULL),
  m_pPoiAllocBuff(NULL),
  m_nNumHazardNames(0),
  m_papszHazardNames(NULL)
{
	for (int i = 0; i < nPreAllocVerts; i++)
	{
		m_paVerts[i].m_nVertSlotStatus = VERT_SLOT_FREE;
	}

#if AIGRAPH_EDITOR_ENABLED==1
	InitProblemVertRecording();
#endif

#if AIGRAPH_EDITOR_ENABLED
	m_papszHazardNames =  new char*[255];
	char* pMem = new char[255*MAX_HAZARD_NAME_LEN];
	for (u32 i = 0; i < 255; i++)
	{
		m_papszHazardNames[i] = pMem+i*MAX_HAZARD_NAME_LEN;
	}
#endif
}


CAIGraph::~CAIGraph(void)
{
	m_nNumVerts = 0;

#if AIGRAPH_EDITOR_ENABLED
	if( m_bMemoryOwner && m_pVertAllocBuff )
	{
		fang_Free( m_pVertAllocBuff );
		m_pVertAllocBuff  = NULL;
	}

	if (m_pPoiAllocBuff)
	{
		fang_Free( m_pPoiAllocBuff );
		m_pPoiAllocBuff = NULL;
	}
	m_nNumPoi = 0;
	m_nMaxPoi = 0;
	m_paPoi= NULL;

	delete [] m_papszHazardNames[0];
	delete [] m_papszHazardNames;
	m_papszHazardNames = NULL;

#endif
	m_paVerts = NULL;
	m_nMaxVerts = 0;
}


u16	CAIGraph::GetEdgeId(const GraphEdge *pE)  //Creepy, but it works
{
	u16 nVertNum = ((GraphVert*) pE) - m_paVerts;
	GraphVert* pV = m_paVerts+nVertNum;
	u16 nLocalEdgeNum = pE - pV->m_aEdgeSlots;
	return nVertNum*AIGRAPH_NUM_EDGES_PER_VERT+nLocalEdgeNum;
}  


BOOL CAIGraph::PointerizeBinaryData(void)
{
	m_paVerts = (GraphVert*) (((u8*)this) + sizeof(CAIGraph));
	m_paPoi = (CGraphPoi*) ((u8*)m_paVerts + sizeof(GraphVert)*m_nNumVerts);
	m_papszHazardNames = (char**) (((u8*)m_paPoi) + sizeof(CGraphPoi)*m_nNumPoi);

	char* pSrc = (char*) (((u8*)m_papszHazardNames) + sizeof(cchar*)*m_nNumHazardNames);
	for (s32 i = 0; i < m_nNumHazardNames; i++)
	{
		m_papszHazardNames[i] = pSrc;

		if (i < m_nNumHazardNames-1)
		{
			while (*pSrc != '\0') pSrc++;
		}
		pSrc++;
	}

	if (m_nNumVerts == 0)
	{
	   m_paVerts = NULL;
	}

	if (m_nNumPoi == 0)
	{
	   m_paPoi = NULL;
	}

	if (m_nNumHazardNames == 0)
	{
	   m_papszHazardNames = NULL;
	}

	m_bMemoryOwner = FALSE;
	return TRUE;
}

#if FANG_WINGC
static u16 _ReadGraphBE16( const u8 *pData )
{
	return (u16)(((u16)pData[0] << 8) | pData[1]);
}

static BOOL _GraphRangeValid( u32 nOffset, u32 nLength, u32 nTotalBytes )
{
	return nOffset <= nTotalBytes && nLength <= nTotalBytes - nOffset;
}

BOOL CAIGraph::ConvertGameCubeBinaryData( u32 nFileBytes )
{
	static_assert( sizeof(CAIGraph) == 40, "GameCube graph header layout changed" );
	static_assert( sizeof(GraphVert) == 128, "GameCube graph vertices must use five edge slots" );
	static_assert( sizeof(CGraphPoi) == 8, "GameCube graph POI layout changed" );

	u8 *pBytes = (u8 *)this;
	if( nFileBytes < sizeof(CAIGraph) )
	{
		DEVPRINTF( "AIGraph: GameCube graph is smaller than its header (%u bytes).\n", nFileBytes );
		return FALSE;
	}

	const u16 nNumVerts = _ReadGraphBE16( pBytes + 0 );
	const u16 nNumPoi = _ReadGraphBE16( pBytes + 2 );
	const u16 nMaxVerts = _ReadGraphBE16( pBytes + 4 );
	const u16 nMaxPoi = _ReadGraphBE16( pBytes + 6 );
	const u16 nNumHazardNames = _ReadGraphBE16( pBytes + 8 );
	u32 nOffset = sizeof(CAIGraph);
	if( nNumVerts > nMaxVerts || nNumPoi > nMaxPoi || nNumHazardNames > 255 ||
		!_GraphRangeValid( nOffset, (u32)nNumVerts * sizeof(GraphVert), nFileBytes ) )
	{
		DEVPRINTF( "AIGraph: invalid GameCube graph counts (verts=%u/%u poi=%u/%u hazards=%u).\n",
			nNumVerts, nMaxVerts, nNumPoi, nMaxPoi, nNumHazardNames );
		return FALSE;
	}

	const u32 nVertsOffset = nOffset;
	nOffset += (u32)nNumVerts * sizeof(GraphVert);
	if( !_GraphRangeValid( nOffset, (u32)nNumPoi * sizeof(CGraphPoi), nFileBytes ) )
	{
		DEVPRINTF( "AIGraph: invalid GameCube graph POI array.\n" );
		return FALSE;
	}
	const u32 nPoiOffset = nOffset;
	nOffset += (u32)nNumPoi * sizeof(CGraphPoi);
	if( !_GraphRangeValid( nOffset, (u32)nNumHazardNames * sizeof(cchar *), nFileBytes ) )
	{
		DEVPRINTF( "AIGraph: invalid GameCube graph hazard pointer array.\n" );
		return FALSE;
	}
	nOffset += (u32)nNumHazardNames * sizeof(cchar *);
	for( u32 i = 0; i < nNumHazardNames; i++ )
	{
		const u32 nNameStart = nOffset;
		while( nOffset < nFileBytes && pBytes[nOffset] != 0 ) nOffset++;
		if( nOffset == nNameStart || nOffset >= nFileBytes )
		{
			DEVPRINTF( "AIGraph: invalid GameCube graph hazard name %u.\n", i );
			return FALSE;
		}
		nOffset++;
	}
	if( nOffset != nFileBytes )
	{
		DEVPRINTF( "AIGraph: unexpected trailing data in GameCube graph (%u bytes).\n",
			nFileBytes - nOffset );
		return FALSE;
	}

	GraphVert *pVerts = (GraphVert *)(pBytes + nVertsOffset);
	for( u32 i = 0; i < nNumVerts; i++ )
	{
		const u8 nEdgeCount = pVerts[i].m_nNumEdges;
		if( nEdgeCount == VERT_SLOT_FREE ) continue;
		if( nEdgeCount > AIGRAPH_NUM_EDGES_PER_VERT )
		{
			DEVPRINTF( "AIGraph: invalid GameCube graph edge count %u at vert %u.\n", nEdgeCount, i );
			return FALSE;
		}
		for( u32 j = 0; j < nEdgeCount; j++ )
		{
			const u8 *pEdge = pBytes + nVertsOffset + i * sizeof(GraphVert) +
				offsetof(GraphVert, m_aEdgeSlots) + j * sizeof(GraphEdge);
			if( _ReadGraphBE16( pEdge + offsetof(GraphEdge, m_nNextVertId) ) >= nNumVerts )
			{
				DEVPRINTF( "AIGraph: invalid GameCube graph edge target at vert %u edge %u.\n", i, j );
				return FALSE;
			}
		}
	}

	m_nNumVerts = nNumVerts;
	m_nNumPoi = nNumPoi;
	m_nMaxVerts = nMaxVerts;
	m_nMaxPoi = nMaxPoi;
	m_nNumHazardNames = nNumHazardNames;
	for( u32 i = 0; i < nNumVerts; i++ )
	{
		GraphVert *pVert = &pVerts[i];
		pVert->m_Location.x = fang_ConvertEndian( pVert->m_Location.x );
		pVert->m_Location.y = fang_ConvertEndian( pVert->m_Location.y );
		pVert->m_Location.z = fang_ConvertEndian( pVert->m_Location.z );
		pVert->m_fSafeRad = fang_ConvertEndian( pVert->m_fSafeRad );
		pVert->m_fHeightClearance = fang_ConvertEndian( pVert->m_fHeightClearance );
		for( u32 j = 0; j < AIGRAPH_NUM_EDGES_PER_VERT; j++ )
		{
			GraphEdge *pEdge = &pVert->m_aEdgeSlots[j];
			pEdge->m_nNextVertId = fang_ConvertEndian( pEdge->m_nNextVertId );
			pEdge->m_nProps = fang_ConvertEndian( pEdge->m_nProps );
			pEdge->m_fLength = fang_ConvertEndian( pEdge->m_fLength );
			pEdge->m_fMaxSafeHeight = fang_ConvertEndian( pEdge->m_fMaxSafeHeight );
			pEdge->m_fHalfWidth = fang_ConvertEndian( pEdge->m_fHalfWidth );
		}
	}
	CGraphPoi *pPoi = (CGraphPoi *)(pBytes + nPoiOffset);
	for( u32 i = 0; i < nNumPoi; i++ )
	{
		pPoi[i].m_uPackedVertEdgeId = fang_ConvertEndian( pPoi[i].m_uPackedVertEdgeId );
		pPoi[i].m_uVisInfo = fang_ConvertEndian( pPoi[i].m_uVisInfo );
	}

	DEVPRINTF( "AIGraph: converted GameCube graph (%u verts, %u POIs, %u hazards).\n",
		nNumVerts, nNumPoi, nNumHazardNames );
	return TRUE;
}
#endif

#if AIGRAPH_EDITOR_ENABLED

BOOL CAIGraph::LoadFromBinaryFile(const char* szFileName)
{
	FASSERT(m_paVerts == NULL);


	FILE* fp = fopen(szFileName, "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		s32 fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		m_pVertAllocBuff = fang_Malloc(fileLen, 16);
		FASSERT(m_pVertAllocBuff);
		fread(m_pVertAllocBuff, fileLen, 1, fp);
		*this = *((CAIGraph*) m_pVertAllocBuff);
		this->m_paVerts = (GraphVert*) (((u8*)m_pVertAllocBuff)+sizeof(CAIGraph));

		m_bMemoryOwner = TRUE;
		fclose(fp);
		return TRUE;
	}

	return FALSE;
}


void CAIGraph::StartNewGraph(void)
{
	if (m_bMemoryOwner)
	{
		if (m_pVertAllocBuff)
		{
			fang_Free(m_pVertAllocBuff);
		}
		m_pVertAllocBuff = NULL;
		m_paVerts = NULL;
		if (m_pPoiAllocBuff)
		{
			fang_Free(m_pPoiAllocBuff);
		}
		m_pPoiAllocBuff =NULL;
		m_bMemoryOwner = FALSE;
	}
	m_nNumVerts = 0;
	m_nMaxVerts = 0;
	m_nNumPoi = 0;
	m_nMaxPoi = 0;

	m_bMemoryOwner = TRUE;
}


BOOL CAIGraph::LoadFromAsciiFile(const char* szFileName)
{
	FILE* fp = fopen(szFileName, "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		s32 fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		FMemFrame_t MemFrame = fmem_GetFrame();
		void* pFileData = fmem_Alloc(fileLen);
		fread(pFileData, fileLen, 1, fp);
		fclose(fp);
		ParseAscii((char*)pFileData, ((char*)pFileData)+fileLen);
		fmem_ReleaseFrame(MemFrame);
		return TRUE;
	}
	return FALSE;
}


void CAIGraph::ParseAscii(const char* pData, const char* pDataEnd)
{
	FMemFrame_t MemFrame = fmem_GetFrame();
	s32 i;

	FASSERT(m_nNumVerts == 0 && m_paVerts== NULL);

	//count verts
	const char* pTok = pData;
	s32 maxVertId = -1;
	do 
	{
		if (aiutil_MatchAsciiToken("BEGIN_VERT", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			s32 vertId = aiutil_AsciiIntToken(pTok, pDataEnd);
			if (vertId > maxVertId)
			{
				maxVertId  = vertId;
			}
		}
	} while (pTok = aiutil_NextAsciiToken(pTok, pDataEnd));

	if (maxVertId >=0)
	{
		s32 newNumVerts = maxVertId+1;
		s32* aUsed =  (s32*) fmem_Alloc(sizeof(s32)*newNumVerts);	//scratch memory
		for (i = 1; i < newNumVerts; i++)	 //start at one, because we don't need a slot 0, ever.
		{
			aUsed[i] = 0;									
			GraphVert* pV = AllocVertSlot();
		}

		// read in graph... one vert at a time
		pTok = pData;  
		do 
		{
			if (aiutil_MatchAsciiToken("BEGIN_VERT", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
			{
				s32 nVertId = aiutil_AsciiIntToken(pTok, pDataEnd);
				FASSERT(nVertId > 0 && nVertId < newNumVerts);
				aUsed[nVertId] = 1;
				pTok = m_paVerts[nVertId].ParseAscii(pTok, pDataEnd, this);

#if AIGRAPH_EDITOR_ENABLED==1
				if (m_paVerts[nVertId].GetNumEdges() > AIGRAPH_FUTURE_NUM_EDGES_PER_VERT)
				{
					RecordProblemVert(nVertId, CAIGraph::VERTPROBLEMTYPE_TOO_MANY_EDGES);
				}
#endif
			}
		} while (pTok = aiutil_NextAsciiToken(pTok, pDataEnd));

		m_paVerts[0].m_nVertSlotStatus = VERT_SLOT_FREE;
		for (i = 1; i < newNumVerts; i++)
		{
			if (!aUsed[i])
			{
				FreeVertSlot((u16)i);
			}
		}
	}

	//
	// Go through all existing edges and make sure that there are matching reverse edges (mainly this is for tool and UI reasons) 
	//
#if AIGRAPH_EDITOR_ENABLED==1
	for (i = 1; i < m_nNumVerts; i++)
	{
		if (IsVertValid(i))
		{
			GraphVert* pV = GetVert(i);
			BOOL bBadEdge = TRUE;
			while (bBadEdge)
			{
				bBadEdge = FALSE;
				for (u32 j = 0; j < pV->GetNumEdges(); j++)
				{
					GraphEdge* pE = pV->GetEdge(j);
					if (!pE->GetReverseEdge(this))
					{
						//problem, bad edge must have been due to..
						RecordProblemVert(i, VERTPROBLEMTYPE_TOO_MANY_EDGES);
						bBadEdge = TRUE;
						pV->FreeEdgeSlot(j);
						break;
					}
				}
			}

			if (pV->GetRad() < 1.0f && pV->GetHeight() < 1.0f &&
				pV->GetLocation().DistSq(CFVec3A::m_Null) < 2.0f*2.0f)
			{
				pV->m_nVertSlotStatus = VERT_SLOT_FREE;
			}
			else if (pV->GetRad() < 1.0f || pV->GetHeight() < 1.0f)
			{
				RecordProblemVert(i, CAIGraph::VERTPROBLEMTYPE_TOO_SMALL);
			}
		}
	}
#endif

	//
	// Now, get the poi
	//
	pTok = pData;  
	do 
	{
		if (pTok && aiutil_MatchAsciiToken("BEGIN_POI", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{

			pTok = ParseAsciiPoi(pTok, pDataEnd);
			break;
		}
	}  while (pTok = aiutil_NextAsciiToken(pTok, pDataEnd));


	//
	// Now, get hazards
	//
	pTok = pData;  
	do 
	{
		if (aiutil_MatchAsciiToken("HAZNAME", pTok, pDataEnd) && (pTok = aiutil_NextAsciiToken(pTok, pDataEnd)))
		{
			char szHazNameTmp[MAX_HAZARD_NAME_LEN];

			char* psDest = szHazNameTmp;
			cchar* psData = pTok;
			while ( psData &&
					psData < pDataEnd && 
					psDest - szHazNameTmp < (MAX_HAZARD_NAME_LEN-1) && 
					!(*psData==' ' || *psData==13 || *psData=='\n' || *psData=='\t' || *psData==',' || *psData== '=' ))
			{
				*psDest = *psData;
				psDest++;
				psData++;
			}
			*psDest = '\0';
			AddHazard(szHazNameTmp);

		}
	} while (pTok = aiutil_NextAsciiToken(pTok, pDataEnd));


#if AIGRAPH_EDITOR_ENABLED==1
	//
	//  Now pack any holes.
	//
	//
	PackVerts();
#endif

	fmem_ReleaseFrame(MemFrame);
}


u32 CAIGraph::GetSizeOfBinaryData(void)
{
	u32 nBytes = sizeof( CAIGraph );
	u32 nVertSize = sizeof( GraphVert );
	u32 nPoiSize = sizeof( CGraphPoi );
	nBytes += (nVertSize * m_nNumVerts);
	nBytes += (nPoiSize * m_nNumPoi);
	nBytes += (sizeof(cchar*) * m_nNumHazardNames);
	
	for (u32 i = 0; i < m_nNumHazardNames; i++)
	{
		nBytes += fclib_strlen(m_papszHazardNames[i])+1;
	}
	return nBytes;
}


BOOL CAIGraph::SaveToBinaryFile(const char* szFileName, BOOL bEndianFlip /*= FALSE*/)
{
	FILE* fp = fopen(szFileName, "wb");
	if (fp)
	{
		SaveToBinaryFile(fp, bEndianFlip);
		fclose(fp);
		return TRUE;
	}

	return FALSE;
}


BOOL CAIGraph::SaveToBinaryFile(FILE *pFileStream, BOOL bEndianFlip /*= FALSE*/)
{
	//store local copy of any data that might be need while graph is endian-flipped
	u32 uGraphSize = sizeof(CAIGraph);
	s32 nNumVerts = GetNumVerts();
	s32 nNumPoi = GetNumPoi();
	s32 nNumHazardNames = m_nNumHazardNames;
	s32 i;

	if (bEndianFlip)
	{
		EndianFlip();

		for (i = 0; i < nNumVerts; i++)
		{ //not using this if on purpose		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
			m_paVerts[i].EndianFlip();
		}
		for (i = 0; i < nNumPoi; i++)
		{ //not using this if on purpose		if (m_paPoi[i].m_nPoiSlotStatus != POI_SLOT_FREE)
			m_paPoi[i].EndianFlip();
		}
	}
	//dump
	fwrite( this, sizeof(CAIGraph), 1, pFileStream );
	if (nNumVerts)
	{
		fwrite( m_paVerts, sizeof(GraphVert) * nNumVerts, 1, pFileStream );
	}
	if (nNumPoi)
	{
		fwrite( m_paPoi, sizeof(CGraphPoi) * nNumPoi, 1, pFileStream );
	}
	if (nNumHazardNames)
	{
		fwrite( m_papszHazardNames, sizeof(cchar*) * nNumHazardNames, 1, pFileStream );	//write out the array of pointers to be re-pointerized at load
		for (i = 0; i < nNumHazardNames; i++)
		{
			fwrite(m_papszHazardNames[i], fclib_strlen(m_papszHazardNames[i])+1, 1, pFileStream);
		}
	}

	//un-endian the graph, verts and pois
	if (bEndianFlip)
	{
		EndianFlip();
		for (i = 0; i < nNumVerts; i++)
		{ //not using this if on purpose		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
			m_paVerts[i].EndianFlip();
		}
		for (i = 0; i < nNumPoi; i++)
		{ //not using this if on purpose		if (m_paPoi[i].m_nPoiSlotStatus != POI_SLOT_FREE)
			m_paPoi[i].EndianFlip();
		}
	}

	return TRUE;
}


s32	CAIGraph::CalcAsciiBuffSize(void)
{
	s32 size = GraphVert::knAsciiBuffSize * m_nNumVerts;


	for (s32 i =0; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			size += GraphEdge::knAsciiBuffSize * m_paVerts[i].m_nNumEdges;
		}
	}

	size+= m_nNumPoi*CGraphPoi::knAsciiBuffSize;

	size += 10+m_nNumHazardNames;
	return size;
}


char *CAIGraph::EmitAsciiPoi(char* pData, char* pDataEnd)
{
	char* pTok = pData;
	aiutil_SetAsciiIndent(0);
	aiutil_SetTokenDelimiter('\n');
	pTok = aiutil_EmitAsciiLabel("BEGIN_POI", pData, pDataEnd);

	aiutil_SetTokenDelimiter(' ');
	for (u32 i = 1; i < m_nNumPoi; i++)	  //don't write out 0
	{
	   if (m_paPoi[i].m_nPoiSlotStatus != POI_SLOT_FREE)
	   {
			pTok = aiutil_EmitAsciiUnsignedInt(NULL, m_paPoi[i].m_uPackedVertEdgeId , pTok, pDataEnd);
			pTok = aiutil_EmitAsciiUnsignedInt(NULL, m_paPoi[i].m_uVisInfo, pTok, pDataEnd);
			pTok = aiutil_EmitAsciiInt(NULL, m_paPoi[i].m_nXOffset, pTok, pDataEnd);
			pTok = aiutil_EmitAsciiInt(NULL, m_paPoi[i].m_nYOffset, pTok, pDataEnd);
			pTok = aiutil_EmitAsciiInt(NULL, m_paPoi[i].m_nZOffset, pTok, pDataEnd);
			pTok = aiutil_EmitAsciiUnsignedInt(NULL, m_paPoi[i].m_uFlags, pTok, pDataEnd);
			pTok = aiutil_EmitAsciiLabel("\n", pTok, pDataEnd);
	   }
	}

	aiutil_SetTokenDelimiter('\n');
	pTok = aiutil_EmitAsciiLabel("END_POI", pTok, pDataEnd);
	return pTok;
}


cchar *CAIGraph::ParseAsciiPoi(cchar* pData, cchar* pDataEnd)
{
	cchar* pTok = pData;
	u32 uItemCount = 0;
	u32 uItemPerPoi = 6;
	do 
	{
		if (!pTok || aiutil_MatchAsciiToken("END_POI", pTok, pDataEnd))
		{
			break;
		}
		else
		{
			CGraphPoi* pPoi = AllocPoiSlot();
			if (!pPoi)
			{
				break;
			}
			pPoi->m_uPackedVertEdgeId = (u16) aiutil_AsciiUnsignedIntToken(pTok, pDataEnd);
			pTok = aiutil_NextAsciiToken(pTok, pDataEnd);
			pPoi->m_uVisInfo = (u16) aiutil_AsciiUnsignedIntToken(pTok, pDataEnd);
			pTok = aiutil_NextAsciiToken(pTok, pDataEnd);
			pPoi->m_nXOffset = (u8) aiutil_AsciiIntToken(pTok, pDataEnd);
			pTok = aiutil_NextAsciiToken(pTok, pDataEnd);
			pPoi->m_nYOffset = (u8) aiutil_AsciiIntToken(pTok, pDataEnd);
			pTok = aiutil_NextAsciiToken(pTok, pDataEnd);
			pPoi->m_nZOffset = (u8) aiutil_AsciiIntToken(pTok, pDataEnd);
			pTok = aiutil_NextAsciiToken(pTok, pDataEnd);
			pPoi->m_uFlags = (u8) aiutil_AsciiUnsignedIntToken(pTok, pDataEnd);
		}

	} while (pTok = aiutil_NextAsciiToken(pTok, pDataEnd));

	return pTok;
}


BOOL CAIGraph::SaveToAsciiFile(const char* szFileName)
{
	FMemFrame_t MemFrame = fmem_GetFrame();

	s32 fileLen = 1+CalcAsciiBuffSize();
	void* pFileData = fmem_Alloc(fileLen);
	FASSERT(pFileData);
	
	//emit the verts and edges
	char* pLastChar = EmitAscii((char*)pFileData, ((char*)pFileData)+fileLen);

	//emit the poi
	if (m_nNumPoi)
	{
		pLastChar = EmitAsciiPoi(pLastChar, ((char*)pFileData)+fileLen);
	}

	//emit the hazards
	for (u32 i = 0; i < m_nNumHazardNames; i++)
	{
		pLastChar = aiutil_EmitAsciiString("HAZNAME", m_papszHazardNames[i], pLastChar, ((char*)pFileData)+fileLen);
	}

	
	fileLen = pLastChar - (char*)pFileData;

	FILE* fp = fopen(szFileName, "wt");
	if (fp)
	{
		fwrite(pFileData, fileLen, 1, fp);
		fclose(fp);
		return TRUE;
	}



	fmem_ReleaseFrame(MemFrame);

	return FALSE;
}


void CAIGraph::AutoSetDimension2D(s32 nOneVert/* -1*/ )
{
	u16 nBegin = 0;
	u16 nEnd = m_nNumVerts;

	if (nOneVert != -1)
	{
		FASSERT(nOneVert >0 && nOneVert < m_nNumVerts);
		nBegin = (u16) nOneVert;
		nEnd = nBegin+1;
	}
	for (u16 i = nBegin; i < nEnd; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			m_paVerts[i].AutoSetDimension2D();
			for (u8 e = 0; e < m_paVerts[i].m_nNumEdges; e++)
			{
				GraphVert* pVA = m_paVerts+i;
				GraphEdge* pEAB = pVA->GetEdge(e);
				GraphVert* pVB = GetVert(pEAB->m_nNextVertId);

				//the max initial edge with is the radius of the smalled connecting vert
				f32 fMaxInitialEdgeWidth = pVA->m_fSafeRad;
				if (fMaxInitialEdgeWidth > pVB->m_fSafeRad)
				{
					fMaxInitialEdgeWidth  = pVB->m_fSafeRad;
				}

				pEAB->AutoSetDimension2D(	pVA,
											pVB,
											i,
											CFVec3A::m_UnitAxisY,
											fMaxInitialEdgeWidth);
				if (GetVertId(pVB) >= i && GetVertId(pVB) >= nEnd)
				{	//better fixup the reverse edge	if it won't get by outter loop
					GraphEdge *pEBA = pEAB->GetReverseEdge(this);
					pEBA->AutoSetDimension2D(	pVB,
												pVA,
												GetVertId(pVB),
												CFVec3A::m_UnitAxisY,
												pEAB->m_fHalfWidth);	 //use width of ab edge as max for ba edge
				}
			}
		}
	}
}


void CAIGraph::MoveVertsToNearestSurface(void)
{
	for (s32 i =0; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			if (m_paVerts[i].Is2D())
			{
				m_paVerts[i].MoveToNearestSurface(CFVec3::m_UnitAxisY * -1.0f);
			}
		}
	}
}


char* CAIGraph::EmitAscii(char* pData, char* pDataEnd)
{
	FASSERT(pData);
	char* pTok = pData;

	s32 count = 0;

	// write out graph... one vert at a time
	for (u16 i = 0; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			pTok = m_paVerts[i].EmitAscii(pTok, pDataEnd, i);
			pTok = aiutil_EmitAsciiLabel("\n",pTok, pDataEnd);
			count++;
		}
	}

	return (pTok);
}


GraphVert *CAIGraph::_FindFreeVertSlot(void)
{
	GraphVert* pV = NULL;
	
	//find the first free slot
	for (s32 i = 1; i < m_nMaxVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus == VERT_SLOT_FREE)//poop, I'm pretty sure && ((i < (m_nNumVerts-1) || m_nNumVerts < m_nMaxVerts)))
		{
			pV = &(m_paVerts[i]);
			break;
		}
	}
	return pV;
}


GraphVert *CAIGraph::AllocVertSlot(void)
{
	GraphVert* pV = NULL;

	pV = _FindFreeVertSlot();
	if (pV)
	{
		//found one, give it out
		pV->m_nNumEdges = 0;

		if (GetVertId(pV) >= m_nNumVerts)
		{
			m_nNumVerts = GetVertId(pV)+1;
		}
	}
	else
	{	//no slots free, so realloc the whole array
		u16 newMax = m_nMaxVerts+10;//vertex array growth rate
		void* pAllocBuff = fang_MallocAndZero(sizeof(GraphVert)*newMax, 16);
		m_bMemoryOwner = TRUE;
		GraphVert* paVerts = (GraphVert*) pAllocBuff;  //tool only!!!!!  needs to act like a heap here
		if (paVerts)
		{
			//move over the old array
			if (m_paVerts)
			{
				fang_MemCopy(paVerts, m_paVerts, sizeof(GraphVert)*	m_nMaxVerts);
			}

			for (u32 i = m_nMaxVerts; i < newMax; i++)
			{
				paVerts[i].m_nVertSlotStatus = VERT_SLOT_FREE;
			}
			if (m_paVerts)  
			{
				FASSERT(m_bMemoryOwner && m_pVertAllocBuff);
				fang_MemSet(m_paVerts,0xfe, m_nMaxVerts* sizeof(GraphVert));  //blitz the old array, not really necessary, but might help find bugs
				fang_Free(m_pVertAllocBuff);	//delete old memory
			}
			m_pVertAllocBuff = pAllocBuff;
			m_nMaxVerts = newMax;
			m_paVerts = paVerts;

			//now, give a new vert out
			if (m_nNumVerts==0)
			{
				m_nNumVerts = 1;
			}
			pV =  m_paVerts + m_nNumVerts;
			m_nNumVerts++;
			pV->m_nNumEdges = 0;
		}
	}

	if (pV)
	{
		pV->Init();
	}

	return pV; //Null means no free slots in this graph and no more memory to allocate
}


void CAIGraph::FreeVertSlot(u16 nSlotId)
{
	FASSERT(nSlotId >= 0 && nSlotId < m_nNumVerts && m_paVerts[nSlotId].m_nNumEdges >=0 && m_paVerts[nSlotId].m_nNumEdges <= AIGRAPH_NUM_EDGES_PER_VERT);
	FASSERT(m_nNumVerts > 0);

	m_paVerts[nSlotId].m_nVertSlotStatus = VERT_SLOT_FREE;

	if (nSlotId == m_nNumVerts-1)
	{
		m_nNumVerts--;
	}
}


GraphVert* CAIGraph::AddVert( const GraphVert& rVert)
{
	GraphVert* pV = AllocVertSlot();
	if (pV)
	{
		*pV = rVert;
	}

	return pV;
}


GraphVert* CAIGraph::AddVert( const CFVec3A& rLocation)
{
	GraphVert* pV = AllocVertSlot();
	if (pV)
	{
		pV->m_Location = rLocation;
	}

	return pV;
}


void CAIGraph::MoveVert(u16 nVertId, const CFVec3A& rDeltaPos, BOOL bAutoDimension2D)
{
	GraphVert* pV = GetVert(nVertId);
	CFVec3A tmp;
	tmp.Set(rDeltaPos);
	pV->m_Location.Add(tmp);

	for (u8 i = 0; i < pV->GetNumEdges(); i++)
	{
		GraphEdge* pEdge = pV->GetEdge(i);
		GraphVert* pOtherVert = GetVert(pEdge->m_nNextVertId);

		pEdge->m_fLength = fmath_AcuSqrt(pV->m_Location.DistSq(pOtherVert->GetLocation()));
		GraphEdge* pReverseEdge = pEdge->GetReverseEdge(this);
		FASSERT(pReverseEdge);
		if (pReverseEdge)
		{
			pReverseEdge->m_fLength = pEdge->m_fLength;
		}
	}

	if (bAutoDimension2D && pV->Is2D())
	{
		pV->AutoSetDimension2D();
	}

}


void CAIGraph::RemoveVert( u16 nVertId)
{
	//remove edges to this vert.
	for (s32 i = m_paVerts[nVertId].m_nNumEdges-1; i >=0 ; i--)
	{
		DisconnectVerts(nVertId, m_paVerts[nVertId].m_aEdgeSlots[i].m_nNextVertId);
	}
	//remove poi that belong to this vert
	RemoveAllPoiWithinVert(nVertId);

	FreeVertSlot(nVertId);
}


BOOL CAIGraph::ConnectVerts( u16 nVertIdA, u16 nVertIdB, BOOL bAutoDimensionMode2D)
{
	FASSERT(nVertIdA >0 && nVertIdA< m_nNumVerts && m_paVerts[nVertIdA].m_nNumEdges <= AIGRAPH_NUM_EDGES_PER_VERT);  //validate a
	FASSERT(nVertIdB >0 && nVertIdB< m_nNumVerts && m_paVerts[nVertIdB].m_nNumEdges <= AIGRAPH_NUM_EDGES_PER_VERT);  //validate b

	GraphEdge* pEdgeAB;
	GraphEdge* pEdgeBA;
	
	BOOL bConnected = IsConnected(nVertIdA, nVertIdB);
	if (!bConnected)
	{
		GraphVert* pVA = m_paVerts+nVertIdA;
		GraphVert* pVB = m_paVerts+nVertIdB;

		if (pVA->GetNumEdges() < AIGRAPH_FUTURE_NUM_EDGES_PER_VERT && 
			pVB->GetNumEdges() < AIGRAPH_FUTURE_NUM_EDGES_PER_VERT)
		{
			pEdgeAB = pVA->AllocEdgeSlot();
			BOOL b3DEdge = FALSE;
			if ((pVA->m_nProps & VERTPROP_3D) ||
				(pVB->m_nProps & VERTPROP_3D))

			{
				b3DEdge = TRUE;
			}

			if (pEdgeAB)
			{	//initialize edge from vertA to vertB
				pEdgeBA = pVB->AllocEdgeSlot();
				if (pEdgeBA)
				{	//initialize edge from vertB to vertA and back!
					f32 fMaxInitialEdgeWidth = GetVert(nVertIdA)->m_fSafeRad;
					if (fMaxInitialEdgeWidth > GetVert(nVertIdB)->m_fSafeRad)
					{
						fMaxInitialEdgeWidth  = GetVert(nVertIdB)->m_fSafeRad;
					}
					f32 fMaxInitialEdgeHeight = GetVert(nVertIdA)->m_fHeightClearance;
					if (fMaxInitialEdgeHeight > GetVert(nVertIdB)->m_fHeightClearance)
					{
						fMaxInitialEdgeHeight  = GetVert(nVertIdB)->m_fHeightClearance;
					}
					
					f32 fMinRad = 0.0f;
					f32 fMinHeight = 0.0f;
					f32 fEdgeRad = 0.0f;
					f32 fEdgeLen = fmath_AcuSqrt(pVA->GetLocation().DistSq(pVB->GetLocation()));
					if (b3DEdge)
					{
						fMinRad = FMATH_MIN(pVA->GetRad(), pVB->GetRad());
						fMinHeight = FMATH_MIN(pVA->m_fHeightClearance, pVB->m_fHeightClearance);
						fEdgeRad = FMATH_MIN(fMinRad, fMinHeight);
						pEdgeAB->Init(nVertIdB,		//nToVert
									fEdgeLen,		//Edge Length
									fEdgeRad,		//Radius
									0.0f,			//height is nonsense for 3D Edges
									0.0f);		//slope can't mean anything for 3D
						pEdgeAB->m_nProps |= EDGEPROP_3D;
						pEdgeAB->m_nProps |= EDGEPROP_CUSTOM_VOL;
					}
					else
					{
						if (bAutoDimensionMode2D)
						{
							pEdgeAB->Init(nVertIdB, 0.0f, 0.0f, 0.0f, 0.0f);
							pEdgeAB->AutoSetDimension2D(GetVert(nVertIdA), GetVert(nVertIdB), nVertIdA, CFVec3A::m_UnitAxisY, fMaxInitialEdgeWidth);
						}
						else
						{
							f32 fLengthSq = pVA->GetLocation().DistSq(pVB->GetLocation());
							f32 fLength = fmath_AcuSqrt(fLengthSq);
							f32 fDY = pVB->GetLocation().y - pVA->GetLocation().y;
							f32 fMaxPosHeightDelta = FMATH_FABS(fDY);
							pEdgeAB->Init(nVertIdB, fLength, fMaxInitialEdgeWidth, fMaxInitialEdgeHeight, fMaxPosHeightDelta);
						}
					}
					// fixup backward link
					if (b3DEdge)
					{
						pEdgeBA->Init(nVertIdA,		 //nToVert
									fEdgeLen,		//Edge Length
									fEdgeRad,		//Radius
									0.0f,			//height is nonsense for 3D Edges
									0.0f);		//slope can't mean anything for 3D
						pEdgeBA->m_nProps |= EDGEPROP_3D;
						pEdgeBA->m_nProps |= EDGEPROP_CUSTOM_VOL;
					}
					else
					{
						if (bAutoDimensionMode2D)
						{
							pEdgeBA->Init(nVertIdA, 0.0f, 0.0f, 0.0f, 0.0f);
							pEdgeBA->AutoSetDimension2D(GetVert(nVertIdB), GetVert(nVertIdA), nVertIdB, CFVec3A::m_UnitAxisY, pEdgeAB->m_fHalfWidth);
						}
						else
						{
							f32 fLengthSq = pVA->GetLocation().DistSq(pVB->GetLocation());
							f32 fLength = fmath_AcuSqrt(fLengthSq);
							f32 fDY = pVA->GetLocation().y - pVB->GetLocation().y;
							f32 fMaxPosHeightDelta = FMATH_FABS(fDY);
							pEdgeBA->Init(nVertIdA, fLength, fMaxInitialEdgeWidth, fMaxInitialEdgeHeight, fMaxPosHeightDelta);
						}
					}
					return TRUE;
				}
				else
				{
					pVA->FreeEdgeSlot(pVA->m_nNumEdges-1);  //free the edge just added
				}
			}
		}
	}

	return FALSE;
}


void CAIGraph::DisconnectVerts(u16 nVertIdA, u16 nVertIdB)
{
	GraphVert *pV; 
	GraphEdge *pE;
	
	FASSERT(nVertIdA >0 && nVertIdA< m_nNumVerts && m_paVerts[nVertIdA].m_nNumEdges <= AIGRAPH_NUM_EDGES_PER_VERT);  //validate a
	FASSERT(nVertIdB >0 && nVertIdB< m_nNumVerts && m_paVerts[nVertIdB].m_nNumEdges <= AIGRAPH_NUM_EDGES_PER_VERT);  //validate b

	//disconnect A to B
	pV = &(m_paVerts[nVertIdA]);
	FASSERT(pV);
	pE = pV->GetEdgeTo(nVertIdB);
	FASSERT(pE);
	if (pE)
	{
		pV->FreeEdgeSlot(pV->GetEdgeSlot(pE));
	}

	//disconnect B to A
	pV = &(m_paVerts[nVertIdB]);
	FASSERT(pV);
	pE = pV->GetEdgeTo(nVertIdA);
	FASSERT(pE);
	if (pE)
	{
		pV->FreeEdgeSlot(pV->GetEdgeSlot(pE));
	}

}

void CAIGraph::EndianFlip()
{
   	m_nNumVerts = fang_ConvertEndian(m_nNumVerts);
	m_nMaxVerts = fang_ConvertEndian(m_nMaxVerts);
   	m_nNumPoi = fang_ConvertEndian(m_nNumPoi);
	m_nMaxPoi = fang_ConvertEndian(m_nMaxPoi);
	m_nNumHazardNames = fang_ConvertEndian(m_nNumHazardNames);
	m_bMemoryOwner = fang_ConvertEndian((u32)m_bMemoryOwner);
}


u8 CAIGraph::GetHazardId(cchar* pszName)
{
	for (u32 i = 0; i < m_nNumHazardNames; i++)
	{
		if (!fclib_stricmp(pszName, m_papszHazardNames[i]))
		{
		   return (u8) i;
		}
	}
	return 0;
}

void CAIGraph::AddHazard(cchar* pszName)
{
	if (m_nNumHazardNames==0)
	{
		fclib_strcpy(m_papszHazardNames[0], "NONE");
		m_nNumHazardNames++;
	}
	if (fclib_stricmp(pszName, "NONE") && !GetHazardId(pszName))
	{
		if (m_nNumHazardNames < 255)
		{
			fclib_strncpy(m_papszHazardNames[m_nNumHazardNames], pszName, MAX_HAZARD_NAME_LEN);
			m_nNumHazardNames++;
		}
	}
}


#endif //AIGRAPH_EDITOR_ENABLED


s32 CAIGraph::IsConnected( u16 nVertIdA, u16 nVertIdB)
{
	s32 i;
	s32 rval = 0;
	for (i = 0; i < m_paVerts[nVertIdA].m_nNumEdges; i++)
	{
		if (m_paVerts[nVertIdA].m_aEdgeSlots[i].m_nNextVertId == nVertIdB)
			rval = 1;
	}

	for (i = 0; i < m_paVerts[nVertIdB].m_nNumEdges; i++)
	{
		if (m_paVerts[nVertIdB].m_aEdgeSlots[i].m_nNextVertId == nVertIdA)
			rval |= 2;
	}

	return rval;
}


GraphVert* CAIGraph::FindClosestVert2D(const CFVec3A& rLoc)
{
	GraphVert* pClosestVert = NULL;
	f32 fMinDist2 = 0.0f;
	BOOL bInit = 1;
	//blich, this is order n
	for (s32 i = 0; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is2D()) 
		{
			f32 fDist2 = m_paVerts[i].m_Location.DistSq(rLoc);
			if (bInit || (fDist2 < fMinDist2))
			{
				fMinDist2 = fDist2;
				pClosestVert = &(m_paVerts[i]);
				bInit = 0;
			}
		}
	}
	return pClosestVert;
}


GraphVert* CAIGraph::FindClosestVert3D(const CFVec3A& rLoc)
{
	GraphVert* pClosestVert = NULL;
	f32 fMinDist2 = 0.0f;
	BOOL bInit = 1;
	//blich, this is order n
	for (s32 i = 1; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is3D())
		{
			f32 fDist2 = m_paVerts[i].m_Location.DistSq(rLoc);
			if (bInit || (fDist2 < fMinDist2))
			{
				fMinDist2 = fDist2;
				pClosestVert = &(m_paVerts[i]);
				bInit = 0;
			}
		}
	}
	return pClosestVert;
}


void AIGraph_SetLOSTestFunc(AIGraph_LOSTestFunc pFunc)
{
	_pLOSTestFunc = pFunc;
}


BOOL aigraph_RayCollideWorld(const CFVec3A &rRayStart, const CFVec3A &rRayEnd)
{
	if (_pLOSTestFunc)
	{
		return _pLOSTestFunc(rRayStart, rRayEnd);
	}


	//This code is only for TOOL. game shouldhave a _pLOSTESTFunc specified!
	CFCollInfo rayColl;
	rayColl.nCollTestType =	FMESH_COLLTESTTYPE_RAY;
	rayColl.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	rayColl.bFindClosestImpactOnly = FALSE;
	rayColl.nCollMask = FCOLL_MASK_CHECK_ALL;
	rayColl.nResultsLOD = FCOLL_LOD_HIGHEST;
	rayColl.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	rayColl.bCullBacksideCollisions = TRUE;
	rayColl.Ray.Init( &rRayStart, &rRayEnd );
	if( fworld_CollideWithWorldTris(&rayColl) && FColl_nImpactCount > 0 )
	{	  
		return TRUE;
	}
	return FALSE;  
}


BOOL CAIGraph::IsPtIn2DEdgeVol(const GraphVert* pVA, const GraphEdge* pE, const CFVec3A& rLoc, BOOL bBump)
{
	GraphVert* pVB = GetVert(pE->m_nNextVertId);
	CFVec3A BumpLoc = rLoc;
	if (bBump)
	{
		BumpLoc.y += aigraph_kfSurfaceOffset+1.0f;
	}
	CFVec3A EdgeNorm;
	CFVec3A TestRay;
	EdgeNorm.Sub(pVB->GetLocation(), pVA->GetLocation());
	EdgeNorm.y = 0.0f;
	if (EdgeNorm.SafeUnitAndMag(EdgeNorm) > 0.0f)
	{
		TestRay.Sub(BumpLoc, pVA->m_Location);
		TestRay.y = 0.0f;
		f32 fXZDist = EdgeNorm.Dot(TestRay);
		if (fXZDist >=0.0f && fXZDist < pE->GetLength())
		{
			EdgeNorm.Mul(fXZDist);
			EdgeNorm.Add(pVA->m_Location); //re-use edgenorm

			if (EdgeNorm.DistSqXZ(BumpLoc) < pE->m_fHalfWidth*pE->m_fHalfWidth)
			{	//xz intersection
				//cheap-o test for y, but a real test isn't truely correct either since
				//walkable ground can be inside the edge vol
				if ((BumpLoc.y >= pVA->m_Location.y && BumpLoc.y < (pVA->m_Location.y+pVA->m_fHeightClearance)) ||
					(BumpLoc.y >= pVB->m_Location.y && BumpLoc.y < (pVB->m_Location.y+pVB->m_fHeightClearance)))
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL CAIGraph::IsSphereSafelyInside3DEdgeVol(const GraphVert* pVA, const GraphEdge* pE, const CFSphereA& rSphere, CFVec3A* pNewLoc,  BOOL *pbDidAdjust)
{
	GraphVert* pVB = GetVert(pE->m_nNextVertId);
	CFVec3A rLoc = rSphere.m_Pos;
	CFVec3A EdgeNorm;
	CFVec3A TestRay;
	CFVec3A Norm;
	if (rSphere.m_fRadius < pE->m_fHalfWidth)
	{
		EdgeNorm.Sub(pVB->GetLocation(), pVA->GetLocation());
		if (EdgeNorm.SafeUnitAndMag(EdgeNorm) > 0.0f)
		{
			TestRay.Sub(rLoc, pVA->m_Location);
			f32 fDist = EdgeNorm.Dot(TestRay);
			if (fDist >= 0.0f && fDist < pE->GetLength())
			{
				EdgeNorm.Mul(fDist);
				EdgeNorm.Add(pVA->m_Location); //re-use edgenorm

				f32 fInnerRad = pE->m_fHalfWidth-rSphere.m_fRadius;
				if (EdgeNorm.DistSq(rLoc) < fInnerRad*fInnerRad)
				{	//no need to move
					return TRUE;
				}
				else 
				{
					if (pbDidAdjust)
					{
						*pbDidAdjust = TRUE;
						Norm.Sub(rLoc, EdgeNorm);
						if (Norm.SafeUnitAndMag(Norm) > 0.0f)
						{
							Norm.Mul(fInnerRad);
							Norm.Add(EdgeNorm);
							if (pNewLoc)
							{
								*pNewLoc = Norm;
							}
						}
					}
					return FALSE;
				}
			}
		}
	}
	return FALSE;
}



BOOL CAIGraph::IsCylnSafelyInside2DEdgeVol(const GraphVert* pVA, const GraphEdge* pE, const CFSphereA& rSphere, f32 fCylnHeight, BOOL bBump, CFVec3A* pNewLoc,  BOOL *pbDidAdjust)
{
	GraphVert* pVB = GetVert(pE->m_nNextVertId);
	CFVec3A rLoc = rSphere.m_Pos;
	if (bBump)
	{
		rLoc.y+= aigraph_kfSurfaceOffset+1.0f;
	}
	CFVec3A EdgeNorm;
	CFVec3A TestRay;
	CFVec3A Norm;
	if (rSphere.m_fRadius < pE->m_fHalfWidth && fCylnHeight < pE->GetHeight())
	{
	//xz intersection
		//cheap-o test for y, but a real test isn't truely correct either since
		//walkable ground can be inside the edge vol
		if ((rLoc.y >= pVA->m_Location.y && rLoc.y < (pVA->m_Location.y+pVA->m_fHeightClearance)) ||
			(rLoc.y >= pVB->m_Location.y && rLoc.y < (pVB->m_Location.y+pVB->m_fHeightClearance)))
		{
			EdgeNorm.Sub(pVB->GetLocation(), pVA->GetLocation());
			EdgeNorm.y = 0.0f;
			if (EdgeNorm.SafeUnitAndMagXZ(EdgeNorm) > 0.0f)
			{
				TestRay.Sub(rLoc, pVA->m_Location);
				TestRay.y = 0.0f;
				f32 fXZDist = EdgeNorm.Dot(TestRay);
				if (fXZDist >=0.0f && fXZDist < pE->GetLength())
				{
					EdgeNorm.Mul(fXZDist);
					EdgeNorm.Add(pVA->m_Location); //re-use edgenorm

					f32 fInnerRad = pE->m_fHalfWidth-rSphere.m_fRadius;
					if (EdgeNorm.DistSqXZ(rLoc) < fInnerRad*fInnerRad)
					{	//no need to move
						return TRUE;
					}
					else 
					{
						if (pbDidAdjust)
						{
							*pbDidAdjust = TRUE;
							Norm.Sub(rLoc, EdgeNorm);
							if (Norm.SafeUnitAndMag(Norm) > 0.0f)
							{
								Norm.Mul(fInnerRad);
								Norm.Add(EdgeNorm);
								if (pNewLoc)
								{
									*pNewLoc = Norm;
								}
							}
						}
						return FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}


BOOL CAIGraph::CalcCeilingAlongEdge(GraphVert* pVA, GraphVert* pVB, const CFVec3A& Pt_WS, f32* pY_WS)
{
	GraphEdge* pE = pVA->GetEdgeTo(GetVertId(pVB));

	CFVec3A Edge;
	Edge.Sub(pVB->GetLocation(), pVA->GetLocation());
	if (Edge.SafeUnitAndMag(Edge) > 0.0f)
	{
		CFVec3A tmp;
		tmp.Sub(Pt_WS, pVA->GetLocation());
		if (tmp.SafeUnitAndMag(tmp) > 0.0f)
		{
			f32 fLerp = tmp.Dot(Edge);
			if (fLerp > 0.0f)
			{
				f32 fY = FMATH_FPOT(fLerp, pVA->GetLocation().y, pVB->GetLocation().y);
				fY+=pE->GetHeight();
				if (pY_WS)
				{
					*pY_WS = fY;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


BOOL CAIGraph::IsPtIn2DVertVol(const GraphVert* pV, const CFVec3A& rLoc, BOOL bBump)
{
	CFVec3A rLocBumped;
	rLocBumped = rLoc;
	if (bBump)
	{
		rLocBumped.y += aigraph_kfSurfaceOffset+1.0f;
	}
	f32 fDistXZ2 = pV->m_Location.DistSqXZ(rLocBumped);
	return (fDistXZ2 < pV->GetRad2() &&
			rLocBumped.y >= pV->GetLocation().y &&
			rLocBumped.y < pV->GetLocation().y + pV->m_fHeightClearance);
}


#define _NUM_CLOSEST_TO_TRACK 3
const f32 _kfMaxInitialDistanceFromGraph = 75.0f;
const f32 _kfMaxInitialDistanceFromGraphSq = 75.0f*75.0f;
const f32 _kfMaxInitialDistanceFrom3dGraphSq = 100.0f*100.0f;
static s32 _anClosestVertsXZ[_NUM_CLOSEST_TO_TRACK];
GraphVert* CAIGraph::FindClosestLOSVert2D(const CFVec3A& rLoc, BOOL bOnlyWithinGraphVol)
{
	GraphVert* pClosestVert = NULL;
	f32 fMinDist2 = 0.0f;
	BOOL bNoVertFound = 1;
	FCollImpact_t oCollInfo;
	CFVec3A rLocBumped;
	u8 e;
	s32 i;
	rLocBumped = rLoc;
	rLocBumped.y += aigraph_kfSurfaceOffset+1.0f;

	_anClosestVertsXZ[0] = -1;
	_anClosestVertsXZ[1] = -1; 
	_anClosestVertsXZ[2] = -1; 

	//blich, this is order n
	for (i = 1; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is2D())
		{
			f32 fDistXZ2 = m_paVerts[i].m_Location.DistSqXZ(rLocBumped);
			if (bNoVertFound || fDistXZ2 < fMinDist2)
			{
				if (fDistXZ2 < m_paVerts[i].GetRad2() &&
					rLocBumped.y >= m_paVerts[i].GetLocation().y &&
					rLocBumped.y < m_paVerts[i].GetLocation().y + m_paVerts[i].m_fHeightClearance)
				{
					fMinDist2 = fDistXZ2;
					pClosestVert =  &(m_paVerts[i]);
					bNoVertFound = 0;
				}
			}

			//until a TRUE LOS vert is found, we better keep track of the 3 closest verts,
			//so that if by the end, none is found, it will be quicker to do los
			//tests on just the closest.
			if (bNoVertFound && fDistXZ2 < (_kfMaxInitialDistanceFromGraph+m_paVerts[i].GetRad())*(_kfMaxInitialDistanceFromGraph+m_paVerts[i].GetRad()) )	//findfix: this should be a param
			{
				for (s32 o = 0; o < _NUM_CLOSEST_TO_TRACK; o++)
				{

					BOOL bDoIt = _anClosestVertsXZ[o] == -1;
					if (!bDoIt)
					{
						f32 fL1 = m_paVerts[i].m_Location.DistSq(rLocBumped);
						f32 fL2 = m_paVerts[_anClosestVertsXZ[o]].m_Location.DistSq(rLocBumped);
						bDoIt = fL1 < fL2;
					}
					if (bDoIt)
					{
						for (s32 b = _NUM_CLOSEST_TO_TRACK-1; b > o ; b--)
						{
							_anClosestVertsXZ[b] = _anClosestVertsXZ[b-1];
						}
						_anClosestVertsXZ[o] = i;
						break;  //kick out, since we found a slot
					}

				}
			}

		}
	}

	if (bNoVertFound)
	{	//didn't find any verts that rLocBumped is within the bounds of
		//so, find of the 3 closest verts I did find, Find one that rLocBumped has los to.
		for (i = 0; i < _NUM_CLOSEST_TO_TRACK; i++)
		{
			if (_anClosestVertsXZ[i] != -1)
			{
				if (bOnlyWithinGraphVol)
				{
					GraphVert* pVA = GetVert((u16)_anClosestVertsXZ[i]);
					for (e = 0; e < pVA->GetNumEdges(); e++)
					{
						if (IsPtIn2DEdgeVol(pVA, pVA->GetEdge(e), rLocBumped, FALSE))
						{
							pClosestVert = pVA;
							break;
						}
					}
					if (pClosestVert)
					{
						break;	//	found one
					}
				}
				else
				{
					if (!aigraph_RayCollideWorld(m_paVerts[_anClosestVertsXZ[i]].m_Location, rLocBumped))
					{
						pClosestVert = GetVert((u16)_anClosestVertsXZ[i]);
	#if !AIGRAPH_EDITOR_ENABLED
						aiutils_DebugTrackRay(rLocBumped,m_paVerts[_anClosestVertsXZ[i]].m_Location, 1); 
	#endif
						break;
					}
					else
					{
	#if !AIGRAPH_EDITOR_ENABLED
						aiutils_DebugTrackRay(rLocBumped,m_paVerts[_anClosestVertsXZ[i]].m_Location, 0); 
	#endif
					}
				}
			}
			_anClosestVertsXZ[i] = -1;

		}

	}
	return pClosestVert;
}


GraphVert* CAIGraph::FindClosestLOSVert2D_AdjustWithinRange(CFVec3A* pLoc, f32 fAdjustWithinRange)
{
	FASSERT(pLoc);
	GraphVert* pClosestVert = NULL;
	f32 fMinDist2 = 0.0f;
	BOOL bNoVertFound = 1;
	FCollImpact_t oCollInfo;
	CFVec3A rLocBumped;
	rLocBumped = *pLoc;
	rLocBumped.y += aigraph_kfSurfaceOffset+1.0f;
	s32 i;
	_anClosestVertsXZ[0] = -1;
	_anClosestVertsXZ[1] = -1;
	_anClosestVertsXZ[2] = -1;

	//blich, this is order n
	for (i = 1; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is2D())
		{
			f32 fDistXZ2 = m_paVerts[i].m_Location.DistSqXZ(rLocBumped);
			if (bNoVertFound || fDistXZ2 < fMinDist2)
			{
				if (fDistXZ2 < m_paVerts[i].GetRad2() &&
					rLocBumped.y >= m_paVerts[i].GetLocation().y &&
					rLocBumped.y < (m_paVerts[i].GetLocation().y + m_paVerts[i].m_fHeightClearance))
				{
					fMinDist2 = fDistXZ2;
					pClosestVert =  &(m_paVerts[i]);
					bNoVertFound = 0;
				}
			}

			//until a TRUE LOS vert is found, we better keep track of the 3 closest verts,
			//so that if by the end, none is found, it will be necessary to do los
			//tests on only those
			if (bNoVertFound && fDistXZ2 < _kfMaxInitialDistanceFromGraphSq)	//findfix: this should be a param
			{
				for (s32 o = 0; o < _NUM_CLOSEST_TO_TRACK; o++)
				{
					BOOL bDoIt = _anClosestVertsXZ[o] == -1;
					if (!bDoIt)
					{
						f32 fL1 = m_paVerts[i].m_Location.DistSq(rLocBumped);
						f32 fL2 = m_paVerts[_anClosestVertsXZ[o]].m_Location.DistSq(rLocBumped);
						bDoIt = fL1 < fL2;
					}
					if (bDoIt)
					{
						for (s32 b = _NUM_CLOSEST_TO_TRACK-1; b > o ; b--)
						{
							_anClosestVertsXZ[b] = _anClosestVertsXZ[b-1];
						}
						_anClosestVertsXZ[o] = i;
						break;  //kick out, since we found a slot
					}
				}
			}

		}
	}

	if (bNoVertFound)
	{	//didn't find any verts that rLocBumped is within the bounds of
		//so, find of the 3 closest verts I did find, Find one that rLocBumped has los to.
		for (i = 0; i < _NUM_CLOSEST_TO_TRACK; i++)
		{
			if (_anClosestVertsXZ[i] != -1)
			{
				GraphVert* pVA = GetVert((u16)_anClosestVertsXZ[i]);
				for (u8 e = 0; e < pVA->GetNumEdges(); e++)
				{
					if (IsPtIn2DEdgeVol(pVA, pVA->GetEdge(e), rLocBumped, FALSE))
					{
						pClosestVert = pVA;
						break;
					}
				}
				if (pClosestVert)
				{
					break;
				}
				else
				{	//must not be in an edge of this vert
					//check los to the vert
					f32 fRadSum = fAdjustWithinRange+m_paVerts[_anClosestVertsXZ[i]].GetRad();
					fRadSum*=fRadSum;
					if (m_paVerts[_anClosestVertsXZ[i]].m_Location.DistSqXZ(rLocBumped) < fRadSum)
					{
						if (!aigraph_RayCollideWorld(m_paVerts[_anClosestVertsXZ[i]].m_Location, rLocBumped))
						{
							pLoc->Sub(rLocBumped, m_paVerts[_anClosestVertsXZ[i]].m_Location);
							if (pLoc->SafeUnitAndMagXZ(*pLoc) >0.0f)
							{
								pLoc->y = 0.0f;
								pLoc->Mul(m_paVerts[_anClosestVertsXZ[i]].GetRad() - 2.0f);
								pLoc->Add(m_paVerts[_anClosestVertsXZ[i]].m_Location);
								
								pClosestVert = GetVert((u16)_anClosestVertsXZ[i]);
								break;	//found a vert and adjusted the location

							}
	#if !AIGRAPH_EDITOR_ENABLED
						aiutils_DebugTrackRay(rLocBumped,m_paVerts[_anClosestVertsXZ[i]].m_Location, 1); 
	#endif
						}
						else
						{
		#if !AIGRAPH_EDITOR_ENABLED
							aiutils_DebugTrackRay(rLocBumped,m_paVerts[_anClosestVertsXZ[i]].m_Location, 0); 
		#endif
						}
					}
				}
			}
		}

	}
	return pClosestVert;
}


GraphVert* CAIGraph::FindClosestLOSVert2DInDonut_AdjustWithinRange(const CFVec3A& rBotLoc, f32 fAdjustWithinRange,const CFVec3A& DonutOrigin, f32 fDonutInnerRad, f32 fDonutOuterRad)
{
	GraphVert* pClosestVert = NULL;
	f32 fMinDist2 = 0.0f;
	BOOL bNoVertFound = 1;
	FCollImpact_t oCollInfo;
	CFVec3A rLocBumped;
	rLocBumped = rBotLoc;
	rLocBumped.y += aigraph_kfSurfaceOffset+1.0f;
	s32 i;
	_anClosestVertsXZ[0] = -1;
	_anClosestVertsXZ[1] = -1;
	_anClosestVertsXZ[2] = -1;

	//blich, this is order n
	for (i = 1; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is2D())
		{
			f32 fDonutDistXZSq = m_paVerts[i].m_Location.DistSqXZ(DonutOrigin);
			if (fDonutDistXZSq > fDonutInnerRad*fDonutInnerRad && fDonutDistXZSq < fDonutOuterRad*fDonutOuterRad)
			{
				f32 fDistXZ2 = m_paVerts[i].m_Location.DistSqXZ(rLocBumped);
				//until a TRUE LOS vert is found, we better keep track of the 3 closest verts,
				//so that if by the end, none is found, it will be necessary to do los
				//tests on only those
				for (s32 o = 0; o < _NUM_CLOSEST_TO_TRACK; o++)
				{
					BOOL bDoIt = _anClosestVertsXZ[o] == -1;
					if (!bDoIt)
					{
						f32 fL1 = m_paVerts[i].m_Location.DistSq(rLocBumped);
						f32 fL2 = m_paVerts[_anClosestVertsXZ[o]].m_Location.DistSq(rLocBumped);
						bDoIt = fL1 < fL2;
					}
					if (bDoIt)
					{
						for (s32 b = _NUM_CLOSEST_TO_TRACK-1; b > o ; b--)
						{
							_anClosestVertsXZ[b] = _anClosestVertsXZ[b-1];
						}
						_anClosestVertsXZ[o] = i;
						break;  //kick out, since we found a slot
					}
				}
			}
		}
	}

	if (bNoVertFound)
	{
		if (_anClosestVertsXZ[0] != -1)
		{
			pClosestVert = GetVert((u16)_anClosestVertsXZ[0]);
		}
	}
	return pClosestVert;
}


GraphVert* CAIGraph::FindClosestIntersectingVert2D(const CFVec3A& rRayStart, const CFVec3A& rRayEnd, f32 fUseAsVertRadius /*= 1.0f*/) 
{
	f32 fMaxDistToCheck = 300.0f;
	GraphVert* pClosestVert  = NULL;

	for (s32 i = 0; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is2D())
		{
			CFSphereA VertSphere(m_paVerts[i].m_Location, fUseAsVertRadius);
			CFVec3A HitLoc;
			f32 fDistFromRayStart;
			if (VertSphere.IsIntersecting( rRayStart, rRayEnd, &HitLoc, &fDistFromRayStart))
			{
				if (fDistFromRayStart < fMaxDistToCheck)
				{
					pClosestVert = &(m_paVerts[i]);
					fMaxDistToCheck = fDistFromRayStart; 
				}
			}
		}
	}

	return pClosestVert;
}


BOOL CAIGraph::IsPtIn3DEdgeVol(const GraphVert* pVA, const GraphEdge* pE, const CFVec3A& rPt)
{
	GraphVert* pVB = GetVert(pE->m_nNextVertId);
	CFVec3A EdgeNorm;
	CFVec3A TestRay;
	f32 fDot;
	EdgeNorm.Sub(pVB->GetLocation(), pVA->GetLocation());
	if (pE->GetLength() > 0.0f)
	{					   
		FASSERT(EdgeNorm.SafeUnitAndMag(EdgeNorm)>0.0f);	 //this should be guaranteed by pE->GetLength`
		EdgeNorm.Unitize();
		TestRay.Sub(rPt, pVA->m_Location);
		fDot = EdgeNorm.Dot(TestRay);
		if (fDot >= 0.0f && fDot < pE->GetLength())
		{
			//re-use EdgeNorm
			EdgeNorm.Mul(fDot);
			EdgeNorm.Add(pVA->m_Location); 

			if (EdgeNorm.DistSq(rPt) < pE->m_fHalfWidth*pE->m_fHalfWidth)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


BOOL CAIGraph::IsPtIn3DVertVol(const GraphVert* pV, const CFVec3A& rPt)
{
	f32 fCylnBottom = pV->GetLocation().y-pV->GetHeight();
	f32 fDistXZ2 = pV->m_Location.DistSqXZ(rPt);
	return (fDistXZ2 < pV->GetRad2() &&
			rPt.y >= fCylnBottom &&
			rPt.y < fCylnBottom + 2.0f*pV->GetHeight());
}


static s32 _anClosestVerts[_NUM_CLOSEST_TO_TRACK];
GraphVert* CAIGraph::FindClosestLOSVert3D(const CFVec3A& rLoc, BOOL bOnlyWithinGraphVol)
{
	GraphVert* pClosestVert = NULL;
	f32 fMinDist2 = 0.0f;
	BOOL bNoVertFound = 1;
	FCollImpact_t oCollInfo;

	_anClosestVerts[0] = -1;
	_anClosestVerts[1] = -1; 
	_anClosestVerts[2] = -1; 

	//blich, this is order n
	for (s32 i = 1; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is3D())
		{
			f32 fDist2 = m_paVerts[i].m_Location.DistSq(rLoc);
			if (bNoVertFound || fDist2 < fMinDist2)
			{
				fMinDist2 = fDist2;
			}
			if (IsPtIn3DVertVol(m_paVerts+i, rLoc))
			{
				pClosestVert =  &(m_paVerts[i]);
				bNoVertFound = 0;
			}

			//until a TRUE LOS vert is found, keep track of the 3 closest verts,
			//so that if by the end, none is found, it will be quicker to do los
			//tests on just the closest.
			if (bNoVertFound && fDist2 < _kfMaxInitialDistanceFrom3dGraphSq)	//findfix: this should be a param
			{
				for (s32 o = 0; o < _NUM_CLOSEST_TO_TRACK; o++)
				{
					if (_anClosestVerts[o] == -1 || fDist2 < m_paVerts[_anClosestVerts[o]].m_Location.DistSq(rLoc))
					{
						for (s32 b = _NUM_CLOSEST_TO_TRACK-1; b > o ; b--)
						{
							_anClosestVerts[b] = _anClosestVerts[b-1];
						}
						_anClosestVerts[o] = i;
						break;  //kick out, since we found a slot
					}
				}
			}
		}
	}

	if (bNoVertFound)
	{	//didn't find any verts that rLocBumped is within the bounds of
		//so, find of the 3 closest verts I did find, Find one that rLocBumped has los to.
		for (s32 i = 0; i < _NUM_CLOSEST_TO_TRACK; i++)
		{
			if (_anClosestVerts[i] != -1)
			{
				if (bOnlyWithinGraphVol)
				{
					GraphVert* pVA = GetVert((u16)_anClosestVerts[i]);
					if (IsPtIn3DVertVol(pVA, rLoc))
					{
						pClosestVert = pVA;
						break;
					}
					else
					{
						for (u8 e = 0; e < pVA->GetNumEdges(); e++)
						{
							if (IsPtIn3DEdgeVol(pVA, pVA->GetEdge(e), rLoc))
							{
								pClosestVert = pVA;
								break;
							}
						}
					}
				}
				else
				{
					CFVec3A BumpedRayTestOrigin;
					BumpedRayTestOrigin = rLoc;
					BumpedRayTestOrigin.y+=1.0f;
					if (!aigraph_RayCollideWorld(m_paVerts[_anClosestVerts[i]].m_Location, BumpedRayTestOrigin))
					{
						pClosestVert = GetVert((u16)_anClosestVerts[i]);
	#if !AIGRAPH_EDITOR_ENABLED
						aiutils_DebugTrackRay(BumpedRayTestOrigin, m_paVerts[_anClosestVerts[i]].m_Location, 1); 
	#endif
						break;
					}
					else
					{
	#if !AIGRAPH_EDITOR_ENABLED
						aiutils_DebugTrackRay(BumpedRayTestOrigin, m_paVerts[_anClosestVerts[i]].m_Location, 0); 
	#endif
					}
				}
			}
		}

	}
	return pClosestVert;
}


GraphVert* CAIGraph::FindClosestLOSVert3D_AdjustWithinRange(CFVec3A* pLoc, f32 fAdjustWithinRange)
{
	FASSERT(pLoc);
	GraphVert* pClosestVert = NULL;
	f32 fMinDist2 = 0.0f;
	FCollImpact_t oCollInfo;
	CFVec3A rLoc;
	rLoc = *pLoc;
	s32 i;
	_anClosestVerts[0] = -1;
	_anClosestVerts[1] = -1;
	_anClosestVerts[2] = -1;
	GraphVert* pVA;

	//blich, this is order n
	for (i = 1; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is3D())
		{
			if (IsPtIn3DVertVol(m_paVerts+i, rLoc))
			{
				pClosestVert = m_paVerts+i;
				break;
			}
			pVA = m_paVerts+i;

			//until a TRUE LOS vert is found, keep track of the 3 closest verts,
			//so that if by the end, none is found, it will be quicker to do los
			//tests on just the closest.
			f32 fRadSum = fAdjustWithinRange+pVA->GetRad();
			fRadSum*=fRadSum;
			f32 fDist2 = pVA->m_Location.DistSqXZ(rLoc);
			if (fDist2 < fRadSum &&
				(fmath_Abs(rLoc.y -pVA->m_Location.y) < pVA->GetHeight()+fAdjustWithinRange))
			{
				for (s32 o = 0; o < _NUM_CLOSEST_TO_TRACK; o++)
				{
					if (_anClosestVerts[o] == -1 || fDist2 < m_paVerts[_anClosestVerts[o]].m_Location.DistSqXZ(rLoc))
					{
						for (s32 b = _NUM_CLOSEST_TO_TRACK-1; b > o ; b--)
						{
							_anClosestVerts[b] = _anClosestVerts[b-1];
						}
						_anClosestVerts[o] = i;
						break;  //kick out, since we found a slot
					}
				}
			}
		}
	}

	if (!pClosestVert)
	{	//didn't find any verts that rLoc is within the bounds of
		//so, of the 3 closest verts I did find, Find one that rLoc has los to.
		for (i = 0; i < _NUM_CLOSEST_TO_TRACK && _anClosestVerts[i] !=-1; i++)
		{
			pClosestVert = NULL;
			pVA = GetVert((u16)_anClosestVerts[i]);
			//check edges too
			for (u8 e = 0; e < pVA->GetNumEdges(); e++)
			{
				if (IsPtIn3DEdgeVol(pVA, pVA->GetEdge(e), rLoc))
				{
					pClosestVert = pVA;
					break;
				}
			}
			if (pClosestVert)
			{
				break;
			}
			//must not be in an edge or vol of this vert
			//check los to the vert
			f32 fRadSum = fAdjustWithinRange+pVA->GetRad();
			fRadSum*=fRadSum;
			if (pVA->m_Location.DistSqXZ(rLoc) < fRadSum &&
				(fmath_Abs(rLoc.y -pVA->m_Location.y) < pVA->GetHeight()+fAdjustWithinRange))
			{
				f32 fDistXZ = pVA->m_Location.DistXZ(rLoc);
				if (fDistXZ > pVA->GetRad())
				{
					fDistXZ = pVA->GetRad();
				}

				f32 fDeltaY = rLoc.y - pVA->m_Location.y;
				if (fmath_Abs(fDeltaY) > pVA->GetHeight())
				{
					fDeltaY =  pVA->GetHeight() * FMATH_FSIGN(fDeltaY);
				}
				pLoc->Sub(rLoc, pVA->m_Location);
				pLoc->y = fDeltaY;
				if (pLoc->SafeUnitAndMagXZ(*pLoc) < 0.0f)
				{
					(*pLoc).x = 0.0f;
					(*pLoc).z = 0.0f;
				}
				else
				{
					(*pLoc).x *= fDistXZ;
					(*pLoc).z *= fDistXZ;
				}
				pLoc->Add(pVA->m_Location);
				pClosestVert = pVA;
				break;	//found a vert and adjusted the location
			}
		}
	}
	return pClosestVert;
}


GraphVert* CAIGraph::FindClosestIntersectingVert3D(const CFVec3A& rRayStart, const CFVec3A& rRayEnd, f32 fUseAsVertRadius /*= 1.0f*/) 
{
	f32 fMaxDistToCheck = 300.0f;
	GraphVert* pClosestVert  = NULL;

	for (s32 i = 0; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paVerts[i].Is3D())
		{
			CFSphereA VertSphere(m_paVerts[i].m_Location, fUseAsVertRadius);
			CFVec3A HitLoc;
			f32 fDistFromRayStart;
			if (VertSphere.IsIntersecting( rRayStart, rRayEnd, &HitLoc, &fDistFromRayStart))
			{
				if (fDistFromRayStart < fMaxDistToCheck)
				{
					pClosestVert = &(m_paVerts[i]);
					fMaxDistToCheck = fDistFromRayStart; 
				}
			}
		}
	}

	return pClosestVert;
}


CGraphPoi* CAIGraph::FindClosestIntersectingPoi(const CFVec3A& rRayStart, const CFVec3A& rRayEnd, f32 fUseAsPoiRadius /*= 1.0f*/) 
{
	f32 fMaxDistToCheck = 300.0f;
	CGraphPoi* pClosestPoi  = NULL;

	for (s32 i = 0; i < m_nNumPoi; i++)
	{
		if (m_paPoi[i].m_nPoiSlotStatus != POI_SLOT_FREE)
		{
			CFSphereA PoiSphere;
			m_paPoi[i].GetLocation(this, &(PoiSphere.m_Pos));
			PoiSphere.m_fRadius = fUseAsPoiRadius;
			
			CFVec3A HitLoc;
			f32 fDistFromRayStart;
			if (PoiSphere.IsIntersecting( rRayStart, rRayEnd, &HitLoc, &fDistFromRayStart))
			{
				if (fDistFromRayStart < fMaxDistToCheck)
				{
					pClosestPoi = &(m_paPoi[i]);
					fMaxDistToCheck = fDistFromRayStart; 
				}
			}
		}
	}

	return pClosestPoi;
}


#if AIGRAPH_EDITOR_ENABLED

CGraphPoi *CAIGraph::_FindFreePoiSlot(void)
{
	CGraphPoi* pV = NULL;
	
	//find the first free slot
	for (s32 i = 1; i < m_nMaxPoi; i++)
	{
		if (m_paPoi[i].m_nPoiSlotStatus == POI_SLOT_FREE)
		{
			pV = &(m_paPoi[i]);
			break;
		}
	}
	return pV;
}


CGraphPoi *CAIGraph::AllocPoiSlot(void)
{
	CGraphPoi* pPoi = NULL;

	pPoi = _FindFreePoiSlot();
	if (pPoi)
	{
		//found one, give it out
		pPoi->m_nPoiSlotStatus = 0;

		if (GetPoiId(pPoi) >= m_nNumPoi)
		{
			m_nNumPoi = GetPoiId(pPoi)+1;
		}
	}
	else
	{	//no slots free, so realloc the whole array
		u16 newMax = m_nMaxPoi+10;//vertex array growth rate
		void* pAllocBuff = fang_MallocAndZero(sizeof(CGraphPoi)*newMax, 16);
		CGraphPoi* paPoi = (CGraphPoi*) pAllocBuff;  //tool only!!!!!  needs to act like a heap here
		if (paPoi)
		{
			//move over the old array
			if (m_paPoi)
			{
				fang_MemCopy(paPoi, m_paPoi, sizeof(CGraphPoi)*	m_nMaxPoi);
			}

			for (u32 i = m_nMaxPoi; i < newMax; i++)
			{
				paPoi[i].m_nPoiSlotStatus = POI_SLOT_FREE;
			}
			if (m_paPoi)  
			{
				FASSERT(m_pPoiAllocBuff);
				fang_MemSet(m_paPoi,0xfe, m_nMaxPoi* sizeof(CGraphPoi));  //blitz the old array, not really necessary, but might help find bugs
				fang_Free(m_pPoiAllocBuff);	//delete old memory
			}
			m_pPoiAllocBuff = pAllocBuff;
			m_nMaxPoi = newMax;
			m_paPoi = paPoi;

			//now, give a new poi out
			if (m_nNumPoi==0)
			{
				m_nNumPoi = 1;
			}
			pPoi =  m_paPoi + m_nNumPoi;
			m_nNumPoi++;
			pPoi->m_nPoiSlotStatus = 0;
		}
	}

	if (pPoi)
	{
		pPoi->Init();
	}

	return pPoi; //Null means no free slots in this graph and no more memory to allocate
}


void CAIGraph::FreePoiSlot(u16 nSlotId)
{
	FASSERT(nSlotId >= 0 && nSlotId < m_nNumPoi && m_paPoi[nSlotId].m_nPoiSlotStatus != POI_SLOT_FREE);
	FASSERT(m_nNumPoi > 0);

	m_paPoi[nSlotId].m_nPoiSlotStatus = POI_SLOT_FREE;

	if (nSlotId == m_nNumPoi-1)
	{
		m_nNumPoi--;
	}
}


CGraphPoi* CAIGraph::AddPoi( const CFVec3A& rLoc )
{
	GraphVert* pV = FindClosestLOSVert2D(rLoc, TRUE);
	GraphEdge* pE = NULL;
	CGraphPoi* pPoi = NULL;
	if (!IsPtIn2DVertVol(pV, rLoc))
	{
		for (u8 i = 0; i < pV->GetNumEdges();i++)
		{
			if (IsPtIn2DEdgeVol(pV, pV->GetEdge(i), rLoc))
			{
				pE = pV->GetEdge(i);
				break;
			}
		}
	}

	if (pV && (pPoi = AllocPoiSlot()))
	{
		if (pE)
		{
			pPoi->Init(pE, this, rLoc);
		}
		else
		{
			pPoi->Init(pV, this, rLoc);
		}
	}
	return pPoi;
}


void CAIGraph::MovePoi( u16 uPoiId, const CFVec3A& DeltaPos, BOOL bUpdateVisInfo /* TRUE*/)
{
	CFVec3A OldLoc;
	CGraphPoi* pPoi = GetPoi(uPoiId);
	pPoi->GetLocation(this, &OldLoc);
	OldLoc.Add(DeltaPos);
	pPoi->SetLocation(this, OldLoc);

	if (bUpdateVisInfo)
	{
		pPoi->UpdateVisInfo(this);
	}
}


void CAIGraph::RemovePoi(u16 uPoiId)
{
	FreePoiSlot(uPoiId);
}


void CAIGraph::RemoveAllPoiWithinVert(s16 nVertId)
{
	GraphVert* pV = GetVert(nVertId);
	for (u8 i =0; i < m_nNumPoi; i++)
	{
		if (m_paPoi[i].m_nPoiSlotStatus != POI_SLOT_FREE && m_paPoi[i].UnpackVertId()==nVertId)
		{
			RemovePoi(i);
		}
	}
}


void CAIGraph::RemoveAllPoiWithinEdge(s16 nVertId, s16 nEdgeId)
{
}


void CAIGraph::UpdateAllPoiWithinVert(s16 nVertId)
{
}


void CAIGraph::UpdateAllPoiWithinEdge(s16 nVertId, s16 nEdgeId)
{
}
#endif//	AIGRAPH_EDITOR_ENABLED
#if AIGRAPH_EDITOR_ENABLED==1

BOOL CAIGraph::PackVerts(void)
{
	s32 e,i,j,k,p;

	for (i = 1; i < m_nNumVerts-1; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus == VERT_SLOT_FREE)
		{
			u32 uMoveTo = i;
			u32 uMoveFrom = 0;

			for (j = i+1; j < m_nNumVerts; j++)
			{
				if (m_paVerts[j].m_nVertSlotStatus != VERT_SLOT_FREE)
				{
					uMoveFrom = j;
					break;
				}
			}

			if (uMoveFrom != 0)
			{
				//make a move
				m_paVerts[uMoveTo] = m_paVerts[uMoveFrom];

				for (k = 1; k < m_nNumVerts; k++)
				{
					if (k != uMoveTo &&
						k != uMoveFrom &&
						m_paVerts[k].m_nVertSlotStatus != VERT_SLOT_FREE)
					{
						GraphVert* pV = m_paVerts+k;
						for (e = 0; e < pV->GetNumEdges(); e++)
						{
							GraphEdge* pE = pV->GetEdge(e);
							if (pE->m_nNextVertId == uMoveFrom)
							{
								pE->m_nNextVertId = uMoveTo;
							}
						}
					}
				}


				for (p = 0; p < m_nNumPoi; p++)
				{
					CGraphPoi* pPoi = GetPoi(p);
					if (pPoi->m_nPoiSlotStatus != POI_SLOT_FREE)
					{
						if (pPoi->UnpackVertId() == uMoveFrom)
						{
							pPoi->PackVertId(uMoveTo);
						}
					}
				}


				for (u32 x = 0; x < MAX_PROBLEM_VERTS; x++)
				{
					if (m_auProblemVerts[x].m_uProblemType == VERTPROBLEMTYPE_NONE)
					{
						if (m_auProblemVerts[x].m_uVertId == uMoveFrom)
						{
							m_auProblemVerts[x].m_uVertId = uMoveTo;
						}
					}
				}


				m_paVerts[uMoveFrom].m_nVertSlotStatus = VERT_SLOT_FREE;
			}
		}
	}

	return TRUE;
}
#endif


CGraphPoi* CAIGraph::FindClosestPoi(const CFVec3A& loc,
									f32 fWithinRad,
									u16 uWithinVol,
									u8 uMatchPoiFlags /*= 0x00*/)
{
	f32 fMaxDistToCheck = 300.0f;
	CGraphPoi* pClosestPoi  = NULL;

	BOOL bFirstHit = FALSE;
	if (fWithinRad!=0.0f)
	{
		bFirstHit = TRUE;
	}
	f32 fMinRadSq = fWithinRad*fWithinRad;
	u16* pauPois;
	u16 uNumPois;
	u16 i;
	if (m_pAIGraphDataAccess &&
		uWithinVol &&
		m_pAIGraphDataAccess->GetArrayOfPoiInVol(uWithinVol, &pauPois, &uNumPois))
	{
		for (i = 0; i < uNumPois; i++)
		{
			if (m_paPoi[pauPois[i]].m_nPoiSlotStatus != POI_SLOT_FREE &&
				((m_paPoi[pauPois[i]].m_uFlags & uMatchPoiFlags) == uMatchPoiFlags) &&
				(!m_pAIGraphDataAccess || !m_pAIGraphDataAccess->IsPoiLocked(pauPois[i])))	   //locked poi will never be given out
			{
				CFVec3A PoiLoc;
				m_paPoi[i].GetLocation(this, &(PoiLoc));
				f32 fThisDistSq = PoiLoc.DistSq(loc);
				if (!bFirstHit || fThisDistSq < fMinRadSq)
				{
					pClosestPoi = m_paPoi+i;
					fMinRadSq = fThisDistSq	;
					bFirstHit = TRUE;
				}
			}
		}

	}
	else
	{
		for (i = 0; i < m_nNumPoi; i++)
		{
			if (m_paPoi[i].m_nPoiSlotStatus != POI_SLOT_FREE &&
				((m_paPoi[i].m_uFlags & uMatchPoiFlags) == uMatchPoiFlags) &&
				(!m_pAIGraphDataAccess || !m_pAIGraphDataAccess->IsPoiLocked(i)))			 //locked poi will never be given out
			{
				CFVec3A PoiLoc;
				m_paPoi[i].GetLocation(this, &(PoiLoc));
				f32 fThisDistSq = PoiLoc.DistSq(loc);
				if (!bFirstHit || fThisDistSq < fMinRadSq)
				{
					pClosestPoi = m_paPoi+i;
					fMinRadSq = fThisDistSq	;
					bFirstHit = TRUE;
				}
			}
		}
	}
	return pClosestPoi;
}

#if FANG_PRODUCTION_BUILD	
void CAIGraph::Render(	const CFVec3A& CamPos,
						const CFVec3A& CamLookAt,
						f32 fHalfCosThetaRadsVisCone,
						u32 uDrawFlags, /* AIGRAPH_DEBUG_RENDER_VERTS */
						u32 *pauVertRenderBits, /* NULL*/
						u32 *pauEdgeRenderBits /* NULL*/)
{
}

#else //FANG_PRODUCTION_BUILD	


void CAIGraph::Render(	const CFVec3A& CamPos,
						const CFVec3A& CamLookAt,
						f32 fHalfCosThetaRadsVisCone,
						u32 uDrawFlags, /* AIGRAPH_DEBUG_RENDER_VERTS */
						u32 *pauVertRenderBits, /* NULL*/
						u32 *pauEdgeRenderBits /* NULL*/)
{
	BOOL bSkipVertDraw;
	BOOL bSkipVertBDraw;
	CFVec3A CamToV;
	CFVec3A DrawFrom;
	CFVec3A DrawTo;
	s32 nColor = 0;
	CFColorRGBA _aRgb[7] = 
	{
		FColor_MotifWhite,
		FColor_MotifRed,
		FColor_MotifGreen,
		FColor_MotifBlue,
		FColor_MotifBlue,
		FColor_MotifWhite,
		FColor_MotifWhite,
	};

	_aRgb[AIGRAPH_DEBUGRENDER_VERTFLAG_COLORYELLOW].SetColor(1.0f, 1.0f, 0.0f);
	_aRgb[AIGRAPH_DEBUGRENDER_VERTFLAG_COLORMAGENTA].SetColor(1.0f, 0.0f, 1.0f);

	for (s32 i = 1; i < m_nNumVerts; i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			GraphVert *pV = m_paVerts+i;
			nColor = 0;
			bSkipVertDraw = FALSE;
			f32 fCamDist2 = pV->GetLocation().DistSq(CamPos);
			s32 nLod = 0;
			if (fCamDist2 < aigraph_RenderLODDist01*aigraph_RenderLODDist01)
			{
				nLod = 2;
			}
			else if (fCamDist2 < aigraph_RenderLODDist02*aigraph_RenderLODDist02)
			{
				nLod = 1;
			}

			if (pauVertRenderBits && ((pauVertRenderBits[i] & AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVERT) || !(pauVertRenderBits[i] & AIGRAPH_DEBUGRENDER_VERTFLAG_COLORMASK) ))
			{
				bSkipVertDraw = TRUE;
			}
			else if ((pV->Is2D() && !(uDrawFlags & AIGRAPH_DEBUG_RENDER_2DMODE)) ||
					 (pV->Is3D() && !(uDrawFlags & AIGRAPH_DEBUG_RENDER_3DMODE)))
			{
				continue;
			}
			else if (pauVertRenderBits)
			{
				nColor = pauVertRenderBits[i] & AIGRAPH_DEBUGRENDER_VERTFLAG_COLORMASK;
				FASSERT(nColor < 6);
			}


			if (uDrawFlags & AIGRAPH_DEBUG_RENDER_CULLBEHIND)
			{
				CamToV.Sub(pV->GetLocation(), CamPos);
				if (CamToV.Dot(CamLookAt) < fHalfCosThetaRadsVisCone)
				{
					bSkipVertDraw = TRUE;
				}
			}

			if ((uDrawFlags & AIGRAPH_DEBUG_RENDER_VERTS) && !bSkipVertDraw)
			{
				s32 nRes;
				nRes = nLod-1;
				if (nRes < 0)
				{
					nRes = 0;
				}
				
				fdraw_FacetedWireSphere( &(pV->m_Location.v3), aigraph_fVertVisualRadius, 1+nRes, 1+nRes, _aRgb+nColor);
			}

			if (nLod == 0)
			{
				continue;
			}

			if ((uDrawFlags & AIGRAPH_DEBUG_RENDER_VERTVOLS) && !bSkipVertDraw  && (!pauVertRenderBits || !(pauVertRenderBits[i] & AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVOL)))
			{
				CFVec3A CylinderBottom;
				CylinderBottom = pV->m_Location;
				f32 fHeight = m_paVerts[i].m_fHeightClearance;
				if (pV->Is3D())
				{
					CylinderBottom.y -=	m_paVerts[i].m_fHeightClearance;	  //heightclearence in 3d verts mean half-height of cylinder
					fHeight*=2.0f;
				}
				fdraw_FacetedCylinder( &(CylinderBottom.v3), &(CFVec3A::m_UnitAxisY.v3), m_paVerts[i].m_fSafeRad, fHeight, _aRgb+nColor, 2 );
			}

			if ((uDrawFlags & AIGRAPH_DEBUG_RENDER_2DVERTVOLS) && !bSkipVertDraw)
			{
				fdraw_FacetedCircle( &(CFVec3A::m_UnitAxisY.v3), &(pV->m_Location.v3), m_paVerts[i].m_fSafeRad, _aRgb+nColor, 2 );
			}

			for (s32 j = 0; j < m_paVerts[i].m_nNumEdges; j++)
			{
				CFVec3A RightVec;
				CFVec3A UpVec;
				CFVec3A EdgeVec;
				s32 nextVertId = pV->m_aEdgeSlots[j].m_nNextVertId;
				GraphEdge *pE = &(pV->m_aEdgeSlots[j]);
				GraphVert *pVb = &m_paVerts[nextVertId];
				bSkipVertBDraw = FALSE;
				s32 nEdgeId = GetEdgeId(pE);
				if (nextVertId < i) //this if makes it so that edges are only drawn in one direction.
				{
					continue;
				}

				CamToV.Sub(pVb->GetLocation(), CamPos);
				if (CamToV.Dot(CamLookAt) < fHalfCosThetaRadsVisCone)
				{
					bSkipVertBDraw = TRUE;
				}

				if (bSkipVertBDraw && bSkipVertDraw)   //don't draw this edge if neither vert can be seen? Not really correct, unless both verts are actually behind camera
				{
					continue;
				}


				nColor = 0;
				if (pauEdgeRenderBits)
				{
					nColor = pauEdgeRenderBits[nEdgeId] & AIGRAPH_DEBUGRENDER_VERTFLAG_COLORMASK;
					FASSERT(nColor < 6);
				}


				if (uDrawFlags & AIGRAPH_DEBUG_RENDER_EDGES)
				{
					fdraw_SolidLine(&(pV->m_Location.v3), &(pVb->m_Location.v3), _aRgb+nColor);
				}

				if (pauEdgeRenderBits && pauEdgeRenderBits[nEdgeId] & AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVOL)
				{
				}
				else if (pV->Is3D())
				{
					EdgeVec.Sub(pVb->GetLocation(), pV->GetLocation());
					if (EdgeVec.SafeUnitAndMag(EdgeVec) > 0.0f)
					{
						if ((uDrawFlags & AIGRAPH_DEBUG_RENDER_2DEDGEVOLS) || (uDrawFlags & AIGRAPH_DEBUG_RENDER_EDGEVOLS))
						{
							CFVec3 UpVec;
							fdraw_FacetedCylinder(	&(pV->m_Location.v3),
													&(EdgeVec.v3),
													pE->m_fHalfWidth,
													pE->m_fLength,
													_aRgb+nColor,
													2 );
						}
					}
				}
				else
				{
					if (uDrawFlags & AIGRAPH_DEBUG_RENDER_2DEDGEVOLS)
					{
						EdgeVec.Sub(pVb->GetLocation(), pV->GetLocation());
						EdgeVec.Unitize();

						RightVec.Cross(CFVec3A::m_UnitAxisY, EdgeVec);
						RightVec.Mul(pE->m_fHalfWidth);
						UpVec = CFVec3A::m_UnitAxisY;
						UpVec.Mul(pE->m_fMaxSafeHeight);

						//draw along the length of the one edge
						DrawFrom.Add(pV->GetLocation(), RightVec);
						DrawTo.Add(pVb->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

						//draw along the length of the one edge
						DrawFrom.Sub(pV->GetLocation(), RightVec);
						DrawTo.Sub(pVb->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);
					}

					if (uDrawFlags & AIGRAPH_DEBUG_RENDER_EDGEVOLS)
					{
						EdgeVec.Sub(pVb->GetLocation(), pV->GetLocation());
						EdgeVec.Unitize();

						RightVec.Cross(CFVec3A::m_UnitAxisY, EdgeVec);
						RightVec.Mul(pE->m_fHalfWidth);
						UpVec = CFVec3A::m_UnitAxisY;
						UpVec.Mul(pE->m_fMaxSafeHeight);

						//draw along the length of the edge
//						fdraw_SolidLine(&(pV->GetLocation().v3+RightVec), &(pVb->GetLocation().v3+RightVec), _aRgb+nColor);
						DrawFrom.Add(pV->GetLocation(), RightVec);
						DrawTo.Add(pVb->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pV->GetLocation().v3-RightVec), &(pVb->GetLocation().v3-RightVec), _aRgb+nColor);
						DrawFrom.Sub(pV->GetLocation(), RightVec);
						DrawTo.Sub(pVb->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pV->GetLocation().v3+RightVec+UpVec), &(pVb->GetLocation().v3+RightVec+UpVec), _aRgb+nColor);
						DrawFrom.Add(pV->GetLocation(), RightVec);
						DrawFrom.Add(UpVec);
						DrawTo.Add(pVb->GetLocation(), RightVec);
						DrawTo.Add(UpVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pV->GetLocation().v3-RightVec+UpVec), &(pVb->GetLocation().v3-RightVec+UpVec), _aRgb+nColor);
						DrawFrom.Sub(pV->GetLocation(), RightVec);
						DrawFrom.Add(UpVec);
						DrawTo.Sub(pVb->GetLocation(), RightVec);
						DrawTo.Add(UpVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

						//draw caps
						//a
//						fdraw_SolidLine(&(pV->GetLocation().v3+RightVec),		&(pV->GetLocation().v3-RightVec), _aRgb+nColor);
						DrawFrom.Add(pV->GetLocation(), RightVec);
						DrawTo.Sub(pV->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pV->GetLocation().v3-RightVec),		&(pV->GetLocation().v3-RightVec+UpVec), _aRgb+nColor);
						DrawFrom.Sub(pV->GetLocation(), RightVec);
						DrawTo.Sub(pV->GetLocation(), RightVec);
						DrawTo.Add(UpVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pV->GetLocation().v3-RightVec+UpVec),	&(pV->GetLocation().v3+RightVec+UpVec), _aRgb+nColor);
						DrawFrom.Sub(pV->GetLocation(), RightVec);
						DrawFrom.Add(UpVec);
						DrawTo.Add(pV->GetLocation(), RightVec);
						DrawTo.Add(UpVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pV->GetLocation().v3+RightVec+UpVec),	&(pV->GetLocation().v3+RightVec), _aRgb+nColor);
						DrawFrom.Add(pV->GetLocation(), RightVec);
						DrawFrom.Add(UpVec);
						DrawTo.Add(pV->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

						
						//b
//						fdraw_SolidLine(&(pVb->GetLocation().v3+RightVec),		&(pVb->GetLocation().v3-RightVec), _aRgb+nColor);
						DrawFrom.Add(pVb->GetLocation(), RightVec);
						DrawTo.Sub(pVb->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pVb->GetLocation().v3-RightVec),		&(pVb->GetLocation().v3-RightVec+UpVbec), _aRgb+nColor);
						DrawFrom.Sub(pVb->GetLocation(), RightVec);
						DrawTo.Sub(pVb->GetLocation(), RightVec);
						DrawTo.Add(UpVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pVb->GetLocation().v3-RightVec+UpVbec),	&(pVb->GetLocation().v3+RightVec+UpVbec), _aRgb+nColor);
						DrawFrom.Sub(pVb->GetLocation(), RightVec);
						DrawFrom.Add(UpVec);
						DrawTo.Add(pVb->GetLocation(), RightVec);
						DrawTo.Add(UpVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);

//						fdraw_SolidLine(&(pVb->GetLocation().v3+RightVec+UpVbec),	&(pVb->GetLocation().v3+RightVec), _aRgb+nColor);
						DrawFrom.Add(pVb->GetLocation(), RightVec);
						DrawFrom.Add(UpVec);
						DrawTo.Add(pVb->GetLocation(), RightVec);
						fdraw_SolidLine(&(DrawFrom.v3), &(DrawTo.v3), _aRgb+nColor);
					}
				}
			}
		}
	}

	if (uDrawFlags & AIGRAPH_DEBUG_RENDER_SHOWPOI)
	{
		u16 j;
		for (j = 1; j < m_nNumPoi; j++)
		{
			CGraphPoi* pPoi = GetPoi(j);
			if (pPoi->m_nPoiSlotStatus != POI_SLOT_FREE)
			{
				CFVec3A loc;
				pPoi->GetLocation(this, &loc);

				if (pPoi->m_uFlags & CGraphPoi::POIFLAG_OFFENSIVE_HINT)
				{
					fdraw_FacetedWireSphere( &(loc.v3), 1.0f, 1, 1, _aRgb+AIGRAPH_DEBUGRENDER_VERTFLAG_COLORYELLOW);
				}
				else
				{
					fdraw_FacetedWireSphere( &(loc.v3), 1.0f, 1, 1, _aRgb+AIGRAPH_DEBUGRENDER_VERTFLAG_COLORMAGENTA);
				}
			}
		}
	}
}
#endif//FANG_PRODUCTION_BUILD




#if AIGRAPH_EDITOR_ENABLED

//
// Utils'
//
f32 aigraph_GetClearanceAt(const CFVec3& rRayOrigin, const CFVec3& rLookUpUnit, f32 fMaxClearance)
{
	FCollImpact_t CollInfo;

	CFVec3 RayEnd;
	RayEnd = rLookUpUnit * fMaxClearance;
	RayEnd += rRayOrigin;

	CFVec3A vStart, vEnd;
	vStart.Set(rRayOrigin);
	vEnd.Set(RayEnd);
	if (fworld_FindClosestImpactPointToRayStart(&CollInfo, &vStart, &vEnd))
	{
		f32 heightOfCollision = (CollInfo.ImpactPoint.v3 - rRayOrigin).Dot(rLookUpUnit);
		if (heightOfCollision < fMaxClearance)
		{
			return heightOfCollision;
		}
	}
	return fMaxClearance;
}

BOOL aigraph_IsSurfaceWalkValid(const CFVec3& rRayTestOrigin, const CFVec3& rRayTestDir,
								f32 fMaxDist2, f32 fStep, const CFVec3& LookUpUnit,
								f32 *pfActualDist2 /* NULL*/,
								CFVec3* pEndPos /*= NULL*/,
								f32 fImpossibleSlopeRads /*= FMATH_QUARTER_PI */)
{
	FCollImpact_t oCollInfo;
	CFVec3 rayEnd;
	CFVec3 rayTestLookUp = LookUpUnit;
	CFVec3 rayTestOrigin = rRayTestOrigin;
	CFVec3 rayTestDir = rRayTestDir;
	f32 fCosImpossibleSlopeRads =  fmath_Cos(fImpossibleSlopeRads);

	CFVec3A vStart, vEnd;
	BOOL bBadSlope = FALSE;
	f32 fPerpDist2 = 0.0f;
	while (!bBadSlope && (fPerpDist2 < (fMaxDist2-0.1f)))
	{
		rayEnd = rayTestDir + rayTestOrigin;
		vStart.Set(rayTestOrigin);
		vEnd.Set(rayEnd);
		if (fworld_FindClosestImpactPointToRayStart(&oCollInfo, &vStart, &vEnd))
		{	//bump in surface or blockade?
			f32 fDot = oCollInfo.UnitFaceNormal.v3.Dot(LookUpUnit);
			if (fDot < fCosImpossibleSlopeRads && fDot >= 0.0f)
			{  // call it quits, because new slope too steep
			   bBadSlope = TRUE;

#ifdef DEBUG_AIGRAPH_EDITOR_ENABLED
			   if (_nDrawDebugRays == 0 && fPerpDist2 < 1.0f)
			   {
					_DrawDebugRayOrigin[0] = rayTestOrigin;
					_DrawDebugRayEnd[0] = rayEnd;

					_DrawDebugRayOrigin[1] = oCollInfo.ImpactPoint;
					_DrawDebugRayEnd[1] = oCollInfo.ImpactPoint + oCollInfo.UnitFaceNormal;
					_nDrawDebugRays=2;
			   }
#endif
			}
			else
			{  //or adjust rayTestOrigin to be just above surface
				rayTestOrigin = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset);

				//bend raydir to be parallel to surface
				CFVec3 r = rayTestLookUp.Cross(rayTestDir);
				rayTestDir = r.Cross(oCollInfo.UnitFaceNormal.v3);
				rayTestLookUp = oCollInfo.UnitFaceNormal.v3;
			}
		}
		else
		{  //nothing hit
			CFVec3 localrayOrigin = rayEnd;
			CFVec3 localrayDir = LookUpUnit*(-fStep*5.0f);	//try to find floor by shooting a ray straight down.
			CFVec3 localrayEnd = localrayDir + localrayOrigin;

			vStart.Set(localrayOrigin);
			vEnd.Set(localrayEnd);
			BOOL bCollideWithFloor = fworld_FindClosestImpactPointToRayStart(&oCollInfo, &vStart, &vEnd);
			if (bCollideWithFloor)
			{	//bump in surface or blockade?

				//check to see if surface dropped too much
				f32 dot;
				CFVec3 stepVec = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset) - rayTestOrigin;
				if (stepVec.Mag2() <= 0.0001f)
				{
					bBadSlope = TRUE;
				}
				else
				{
					stepVec.Unitize();
					dot = stepVec.Dot((LookUpUnit*-1.0f));
					
					if (dot > 0.0f && dot > fCosImpossibleSlopeRads )
					{
						bBadSlope = TRUE;
					}
					else if (oCollInfo.UnitFaceNormal.v3.Dot(LookUpUnit) < fCosImpossibleSlopeRads )
					{ //or, if the new surface itself is just too sloped
						bBadSlope = TRUE;
	#ifdef DEBUG_AIGRAPH_EDITOR_ENABLED
					   if (_nDrawDebugRays == 0 && fPerpDist2 < 1.0f)
					   {
							_DrawDebugRayOrigin[0] = localrayOrigin;
							_DrawDebugRayEnd[0] = localrayEnd;

							_DrawDebugRayOrigin[1] = oCollInfo.ImpactPoint;
							_DrawDebugRayEnd[1] = oCollInfo.ImpactPoint + oCollInfo.UnitFaceNormal;
							_nDrawDebugRays=2;
					   }
	#endif
					}
					else
					{	//ok. New step is cool. So, adjust rayTestOrigin to be just above surface and proceed with check
						rayTestOrigin = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset);

						//bend raydir to be parallel to surface
						CFVec3 r = rayTestLookUp.Cross(rayTestDir);
						rayTestDir = r.Cross(oCollInfo.UnitFaceNormal.v3);
						rayTestLookUp = oCollInfo.UnitFaceNormal.v3;
					}
				}
			}
			else
			{  //something seriously wrong.  Where did the floor(surface, I was riding on) go?
				bBadSlope = TRUE;
			}
		}
	
		if (!bBadSlope)
		{
			if (aigraph_kfMinUsefulVolHeight > aigraph_GetClearanceAt(rayTestOrigin,  LookUpUnit, aigraph_kfMinUsefulVolHeight))
			{
				bBadSlope = TRUE;
			}
		}
		
		if (!bBadSlope)
		{

			CFVec3 deltaVec;
			deltaVec = rayTestOrigin - rRayTestOrigin;
			f32 fNewPerpDist2 = (deltaVec - (LookUpUnit * deltaVec.Dot(LookUpUnit))).Mag2();
//			FASSERT(fNewPerpDist2 > fPerpDist2);
			if (fNewPerpDist2 > fPerpDist2)
			{
				fPerpDist2 = fNewPerpDist2;  //never shrink, it means you wen't backwards or something?
			}
			else
			{
				bBadSlope = TRUE;
			}


		}

	}

	FASSERT(fPerpDist2 >= 0.0f);
	if (fPerpDist2 > fMaxDist2)
	{
		fPerpDist2 = fMaxDist2;
	}

	if (pfActualDist2)
	{
		*pfActualDist2 = fPerpDist2;
	}
	
	if (pEndPos)
	{
		*pEndPos = rayTestOrigin;
	}

	return !bBadSlope;
}

f32 aigraph_IsArcWalkable(const CFVec3& rRayTestOrigin, const CFVec3& rRayTestDir, const CFVec3& rLastRayTestDir,  f32 fMaxDist2, f32 fStep, const CFVec3& LookUpUnit, f32 fImpossibleSlopeRads /*= FMATH_QUARTER_PI */)
{
	FCollImpact_t oCollInfo;
	CFVec3 rayEnd;
	CFVec3 rayTestLookUp = LookUpUnit;
	CFVec3 rayTestOrigin = rRayTestOrigin;
	CFVec3 rayTestDir = rRayTestDir;

	bool bBadSlope = FALSE;
	f32 fPerpDist2 = 0.0f;

	f32 fCosImpossibleSlopeRads =  fmath_Cos(fImpossibleSlopeRads);

	CFVec3A vStart, vEnd;
	while (!bBadSlope && (fPerpDist2 < fMaxDist2))
	{
		rayEnd = rayTestDir + rayTestOrigin;
		vStart.Set(rayTestOrigin);
		vEnd.Set(rayEnd);
		if (fworld_FindClosestImpactPointToRayStart(&oCollInfo, &vStart, &vEnd))
		{	//bump in surface or blockade?
			f32 fDot = oCollInfo.UnitFaceNormal.v3.Dot(LookUpUnit);
			if (fDot < fCosImpossibleSlopeRads && fDot >= 0.0f)
			{  // call it quits, because new slope too steep
			   bBadSlope = TRUE;
			}
			else
			{  //or adjust rayTestOrigin to be just above surface
				rayTestOrigin = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset);

				//bend raydir to be parallel to surface
				CFVec3 r = rayTestLookUp.Cross(rayTestDir);
				rayTestDir = r.Cross(oCollInfo.UnitFaceNormal.v3);
				rayTestLookUp = oCollInfo.UnitFaceNormal.v3;
			}
		}
		else
		{  //nothing hit
			CFVec3 localrayOrigin = rayEnd;
			CFVec3 localrayDir = LookUpUnit*(-100.0f);
			CFVec3 localrayEnd = localrayDir + localrayOrigin;

			vStart.Set(localrayOrigin);
			vEnd.Set(localrayEnd);
			BOOL bCollideWithFloor = fworld_FindClosestImpactPointToRayStart(&oCollInfo, &vStart, &vEnd);
			if (bCollideWithFloor)
			{	//bump in surface or blockade?

				//check to see if surface dropped too much
				CFVec3 stepVec = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset) - rayTestOrigin;
				FASSERT(stepVec.Mag2() >= 0.0f);
				stepVec.Unitize();
				f32 dot = stepVec.Dot((LookUpUnit*-1.0f));
				
				if (dot > 0.0f && dot > fCosImpossibleSlopeRads )
				{
					bBadSlope = TRUE;
				}
				else if (oCollInfo.UnitFaceNormal.v3.Dot(LookUpUnit) < fCosImpossibleSlopeRads )
				{ //or, if the new surface itself is just too sloped
					bBadSlope = TRUE;
				}
				else
				{	//ok. New step is cool. So, adjust rayTestOrigin to be just above surface and proceed with check
					rayTestOrigin = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset);

					//bend raydir to be parallel to surface
					CFVec3 r = rayTestLookUp.Cross(rayTestDir);
					rayTestDir = r.Cross(oCollInfo.UnitFaceNormal.v3);
					rayTestLookUp = oCollInfo.UnitFaceNormal.v3;
				}
			}
			else
			{  //something seriously wrong.  Where did the floor(surface, I was riding on) go?
				bBadSlope = TRUE;
			}
		}
	
		if (!bBadSlope)
		{
			if (aigraph_kfMinUsefulVolHeight > aigraph_GetClearanceAt(rayTestOrigin,  LookUpUnit, aigraph_kfMinUsefulVolHeight))
			{
				bBadSlope = TRUE;
			}
		}
		
		if (!bBadSlope)
		{

			CFVec3 deltaVec;
			deltaVec = rayTestOrigin - rRayTestOrigin;
			f32 fNewPerpDist2 = (deltaVec - (LookUpUnit * deltaVec.Dot(LookUpUnit))).Mag2();
//			FASSERT(fNewPerpDist2 > fPerpDist2);
			if (fNewPerpDist2 > fPerpDist2)
			{
				fPerpDist2 = fNewPerpDist2;  //never shrink, it means you wen't backwards or something?
			}
			else
			{
				bBadSlope = TRUE;
			}


		}

	}

	FASSERT(fPerpDist2 >= 0.0f);
	if (fPerpDist2 > fMaxDist2)
	{
		fPerpDist2 = fMaxDist2;
	}

	return fPerpDist2;
}

#endif// AIGRAPH_EDITOR_ENABLED


CAIGraphDataAccess::CAIGraphDataAccess(void)
:	m_pGraph(NULL),
	m_pauEdgeToPoi(NULL),
	m_pauPoiByEdge(NULL),
	m_pauVertToPoi(NULL),
	m_pauPoiByVert(NULL),
	m_pauVolToVert(NULL),
	m_pauVertByVol(NULL),
	m_pauVolToEdge(NULL),
	m_pauEdgeByVol(NULL),
	m_pauVolToPoi(NULL),
	m_pauPoiByVol(NULL),
	m_pauPoiLockBitArray(NULL)
{

}


CAIGraphDataAccess::~CAIGraphDataAccess(void)
{
	m_pauEdgeToPoi = NULL;
	m_pauPoiByEdge = NULL;
	m_pauVertToPoi = NULL;
	m_pauPoiByVert = NULL;
	m_pauVolToVert = NULL;
	m_pauVertByVol = NULL;
	m_pauVolToEdge = NULL;
	m_pauEdgeByVol = NULL;
	m_pauVolToPoi = NULL;
	m_pauPoiByVol = NULL;
	m_pauPoiLockBitArray = NULL;
	m_pauVertLockBitArray = NULL;
}


BOOL CAIGraphDataAccess::IsPoiLocked(u16 uPoiId)
{
	return (m_pauPoiLockBitArray[uPoiId>>5] & (1<<(uPoiId & 31)));
}


void CAIGraphDataAccess::LockPoi(u16 uPoiId)
{
	if (uPoiId > 0 && uPoiId < m_pGraph->GetNumPoi())
	{
		m_pauPoiLockBitArray[uPoiId>>5] |= (1<<(uPoiId & 31));
	}
}


void CAIGraphDataAccess::UnLockPoi(u16 uPoiId)
{
	if (uPoiId > 0 && uPoiId < m_pGraph->GetNumPoi())
	{
		m_pauPoiLockBitArray[uPoiId>>5] &= ~(1<<(uPoiId & 31));
	}
}


BOOL CAIGraphDataAccess::IsVertLocked(u16 uVertId)
{
	return (m_pauVertLockBitArray[uVertId>>5] & (1<<(uVertId & 31)));
}


void CAIGraphDataAccess::LockVert(u16 uVertId)
{
	if (uVertId > 0 && uVertId < m_pGraph->GetNumVerts())
	{
		m_pauVertLockBitArray[uVertId>>5] |= (1<<(uVertId & 31));
	}
}


void CAIGraphDataAccess::UnLockVert(u16 uVertId)
{
	if (uVertId > 0 && uVertId < m_pGraph->GetNumVerts())
	{
		m_pauVertLockBitArray[uVertId>>5] &= ~(1<<(uVertId & 31));
	}
}


BOOL CAIGraphDataAccess::InitEdgeToPoiLookup(void)
{
	u16 i, j, k;
	u16 uPoiInEdgeCount = 0;
	u16 uEdgeCount = 0;
	u16 uNumGlobalEdges = 0;
	FMemFrame_t ScratchMemFrame = fmem_GetFrame();
	FResFrame_t ResFrame = fres_GetFrame();
#define MAX_POI_IN_EDGE_COUNT 8000
	u16* pauScratch = (u16*) fmem_AllocAndZero(MAX_POI_IN_EDGE_COUNT*sizeof(u16), 16);
	if (!pauScratch)
	{
		goto _CAIGraphDataAccessInit_WithError;
	}
	uNumGlobalEdges = m_pGraph->GetNumEdges();

	m_pauEdgeToPoi = (u16*) fres_AlignedAllocAndZero((uNumGlobalEdges+1)*sizeof(u16), 16);
	if (!m_pauEdgeToPoi)
	{
		goto _CAIGraphDataAccessInit_WithError;
	}

	//count the total number of poi that are within edges
	for (i = 0; i < m_pGraph->GetNumVerts();i++)
	{
		GraphVert* pV = m_pGraph->GetVert(i);
		if (pV->m_nVertSlotStatus == VERT_SLOT_FREE)
		{
			uEdgeCount+=6; //skip unused edge slots
			continue;
		}

		for (j = 0; j < pV->GetNumEdges();j++)
		{
			GraphEdge* pE = pV->GetEdge((u8) j);
			m_pauEdgeToPoi[uEdgeCount] = uPoiInEdgeCount;
			if (pE->m_nNextVertId < i) //this if makes it so that edges are only tested in one direction.
			{  //remember, this means that when accessing the PoiByEdge
				m_pauEdgeToPoi[uEdgeCount] = 0xffef;  //cookie here, because we have a reverse edge, come clean it up later and make it point to the same poi's as the reverse
			}
			else
			{

				for (k = 0; k < m_pGraph->GetNumPoi();k++)
				{	//
					CGraphPoi* pPoi = m_pGraph->GetPoi(k);
					if (pPoi->m_nPoiSlotStatus != POI_SLOT_FREE)
					{
						CFVec3A PoiLoc;
						pPoi->GetLocation(m_pGraph, &PoiLoc);
						if (m_pGraph->IsPtIn2DEdgeVol(pV, pE, PoiLoc, FALSE))
						{
							if (uPoiInEdgeCount < MAX_POI_IN_EDGE_COUNT)
							{
							   pauScratch[uPoiInEdgeCount] = k;
							   uPoiInEdgeCount++;
							}
						}
					}
				}
			}
			uEdgeCount++;
		}
		uEdgeCount+=(AIGRAPH_NUM_EDGES_PER_VERT-j);  //skip unused edge slots
	}

	m_pauPoiByEdge = (u16*) fres_AlignedAlloc(uPoiInEdgeCount*sizeof(u16), 16);
	if (!m_pauPoiByEdge)
	{
		goto _CAIGraphDataAccessInit_WithError;
	}
	fang_MemCopy(m_pauPoiByEdge, pauScratch, uPoiInEdgeCount*sizeof(u16));

	for (j = 0; j < m_pGraph->GetNumEdges(); j++)
	{
		if (m_pauEdgeToPoi[j] == 0xffef)  //reverse edge that was skipped, make it match it's reverse
		{
			const GraphEdge* pReverseE = m_pGraph->GetEdge(j);
			const GraphEdge* pE = pReverseE->GetReverseEdge(m_pGraph);
			m_pauEdgeToPoi[j] = m_pauEdgeToPoi[m_pGraph->GetEdgeId(pE)];
		}
	}

	fmem_ReleaseFrame(ScratchMemFrame);

	return TRUE;
_CAIGraphDataAccessInit_WithError:
	fmem_ReleaseFrame(ScratchMemFrame);

	DEVPRINTF( "CAIGraphDataAccess::Init: Out o memory.\n" );
	fres_ReleaseFrame(ResFrame);

	m_pauEdgeToPoi = NULL;
	return FALSE;
}

BOOL CAIGraphDataAccess::GetArrayOfPoiInEdge(u16 uGlobalEdgeId, u16** ppaPoiArray, u16* uNumPoiInEdge)
{
	if (m_pauEdgeToPoi && uGlobalEdgeId > 0 && uGlobalEdgeId < m_pGraph->GetNumEdges())
	{
		*uNumPoiInEdge = m_pauEdgeToPoi[uGlobalEdgeId+1] - m_pauEdgeToPoi[uGlobalEdgeId];
		if (*uNumPoiInEdge)
		{
			*ppaPoiArray = m_pauPoiByEdge + m_pauEdgeToPoi[uGlobalEdgeId];
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CAIGraphDataAccess::InitVolToPoiLookup(void)
{
	FMemFrame_t ScratchMemFrame = fmem_GetFrame();
	FResFrame_t ResFrame = fres_GetFrame();
	u16 k, i, j;
	u16 uScratchVolIdtoPoiIdBegin = 0;
	u16 uVolIdtoPoiIdCount = 0;
	u32 uNumVols = 0;
	u16 uInVolCount = 0;


#define MAX_POI_IN_VOL_COUNT 2000
	u16* pauScratch = (u16*) fmem_AllocAndZero(MAX_POI_IN_EDGE_COUNT*sizeof(u16), 16);
	if (!pauScratch)
	{
		goto _CAIGraphDataAccessInitVolToPoi_WithError;
	}

	uNumVols = FVis_pVisData->nVolumeCount;
	uInVolCount = 0;

	m_pauVolToPoi = (u16*) fres_AlignedAllocAndZero((uNumVols+1)*sizeof(u16), 16);
	if (!m_pauVolToPoi)
	{
		goto _CAIGraphDataAccessInitVolToPoi_WithError;
	}

	//store in the scratch, poiId to volId table
	for (k = 0; k < m_pGraph->GetNumPoi();k++)
	{	
		CGraphPoi* pPoi = m_pGraph->GetPoi(k);
		if (pPoi->m_nPoiSlotStatus != POI_SLOT_FREE)
		{
			CFVec3A PoiLoc;
			pPoi->GetLocation(m_pGraph, &PoiLoc);
			FVisVolume_t* pVol = NULL;
			if (fworld_GetVolumeContainingPoint(&PoiLoc, &pVol) > 0.0f && pVol)
			{
			   pauScratch[k] = pVol - FVis_pVisData->paVolumes;
			}
			else
			{
				pauScratch[k] = 0xffef; //cookie meaning this is not in a volume
			}
		}
		else
		{
			pauScratch[k] = 0xffef; //cookie meaning this is not in a volume
		}
	}

	//store next in scratch a volId to PoiId table
	uScratchVolIdtoPoiIdBegin = k+5;
	uVolIdtoPoiIdCount = 0;
	for (i = 0; i < uNumVols; i++)
	{	
		m_pauVolToPoi[i] = uVolIdtoPoiIdCount;
		for (j = 0; j < m_pGraph->GetNumPoi(); j++)
		{
			if (pauScratch[j] == i)
			{
				pauScratch[uScratchVolIdtoPoiIdBegin + uVolIdtoPoiIdCount] = j;
				uVolIdtoPoiIdCount++;
			}
		}
	}
	m_pauVolToPoi[uNumVols] = uVolIdtoPoiIdCount;

	m_pauPoiByVol = (u16*) fres_AlignedAlloc(uVolIdtoPoiIdCount*sizeof(u16), 16);
	if (!m_pauPoiByVol)
	{
		goto _CAIGraphDataAccessInitVolToPoi_WithError;
	}
	fang_MemCopy(m_pauPoiByVol, pauScratch+uScratchVolIdtoPoiIdBegin, uVolIdtoPoiIdCount*sizeof(u16));

	m_pauPoiLockBitArray = (u32*) fres_AlignedAllocAndZero(4*((m_pGraph->GetNumPoi()>>5)+1), 16);
	if (!m_pauPoiLockBitArray)
	{
		goto _CAIGraphDataAccessInitVolToPoi_WithError;
	}

	m_pauVertLockBitArray = (u32*) fres_AlignedAllocAndZero(4*((m_pGraph->GetNumVerts()>>5)+1), 16);
	if (!m_pauVertLockBitArray)
	{
		goto _CAIGraphDataAccessInitVolToPoi_WithError;
	}


	fmem_ReleaseFrame(ScratchMemFrame);
	return TRUE;
_CAIGraphDataAccessInitVolToPoi_WithError:
	DEVPRINTF( "CAIGraphDataAccess::InitVolToPoi: Out o memory.\n" );
	fmem_ReleaseFrame(ScratchMemFrame);
	fres_ReleaseFrame(ResFrame);

	m_pauPoiByVol = NULL;
	return FALSE;
}


BOOL CAIGraphDataAccess::GetArrayOfPoiInVol(u16 uVolId, u16** ppaPoiArray, u16* uNumPoiInVol)
{
	if (m_pauVolToPoi && uVolId > 0 && uVolId < FVis_pVisData->nVolumeCount)
	{
		*uNumPoiInVol = m_pauVolToPoi[uVolId+1] - m_pauVolToPoi[uVolId];
		if (*uNumPoiInVol)
		{
			*ppaPoiArray = m_pauPoiByVol + m_pauVolToPoi[uVolId];
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CAIGraphDataAccess::Init(CAIGraph* pGraph)
{
	m_pGraph = pGraph;

	BOOL bInitOK = FALSE;
	
	if (InitEdgeToPoiLookup() &&
		InitVolToPoiLookup())
	{
		return TRUE;
	}

	DEVPRINTF( "CAIGraphDataAccess::Init: Out o memory.\n" );

	m_pauEdgeToPoi = NULL;
	m_pauPoiByEdge = NULL;
	m_pauVertToPoi = NULL;
	m_pauPoiByVert = NULL;
	m_pauVolToVert = NULL;
	m_pauVertByVol = NULL;
	m_pauVolToEdge = NULL;
	m_pauEdgeByVol = NULL;
	m_pauVolToPoi = NULL;
	m_pauPoiByVol = NULL;
	return FALSE;
}


