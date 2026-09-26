#include "stdafx.h"
#include "fang.h"
#include "fdraw.h"
#include "fxfm.h"
#include "fviewport.h"
#include "MaiEditGraph.h"
#include "MgcSegment3.h"
#include "MgcRay3.h"
#include "MgcDistLin3Lin3.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

s32 CNiIterator<s32>::s_ReturnError;
CFVec3 CNiIterator<CFVec3>::s_ReturnError;



CEditVert::CEditVert(void)
: m_uEVProps(0)
{
}


CEditGraph::CEditGraph(void)
:	CAIGraph(), 
	m_bSelectionLocked(FALSE),
	m_paEditVerts(NULL),
	m_panRenderPropTmp(NULL),
	m_panEdgeRenderPropTmp(NULL),
	m_nMaxEditVerts(0),
	m_bDrawVertVolumes(FALSE),
	m_bDrawEdgeVolumes(FALSE),
	m_fVertVisualRadius(2.0f),
	m_bDrawVerts(TRUE),
	m_nDrawVolMode(DRAWVOLMODE_ALL),
	m_b2DMode(TRUE),
	m_b3DMode(TRUE)
{ 
}


CEditGraph::CEditGraph(s32 nPreAllocVerts, void* pMemory)
:	CAIGraph(nPreAllocVerts, pMemory),
	m_bSelectionLocked(FALSE),
	m_paEditVerts(NULL),
	m_panRenderPropTmp(NULL),
	m_panEdgeRenderPropTmp(NULL),
	m_nMaxEditVerts(0),
	m_bDrawVertVolumes(FALSE),
	m_bDrawEdgeVolumes(FALSE),
	m_fVertVisualRadius(2.0f),
	m_bDrawVerts(TRUE),
	m_nDrawVolMode(DRAWVOLMODE_ALL),
	m_b2DMode(TRUE),
	m_b3DMode(TRUE)
{
}


CEditGraph::~CEditGraph(void)
{
	ClearSelected();
	delete m_paEditVerts; m_paEditVerts = NULL;
	delete m_panRenderPropTmp; m_panRenderPropTmp = NULL;
	delete m_panEdgeRenderPropTmp; m_panEdgeRenderPropTmp = NULL;
}


u32 CEditGraph::GetSizeOfBinaryData(void)
{
	u32 nBytes = CAIGraph::GetSizeOfBinaryData();
	nBytes -=sizeof(CAIGraph);
	nBytes +=sizeof(CEditGraph);
	nBytes +=sizeof(CEditVert)*GetNumVerts();
	return nBytes;
}


void CEditGraph::SaveToMemory(void* pMem)
{
	fang_MemCopy(pMem, this, sizeof(CEditGraph));
	u32 sEGSize = sizeof(CEditGraph);
	pMem = (void*) ((u32)pMem + sEGSize);
	memcpy(pMem, m_paVerts, sizeof(GraphVert)*GetNumVerts());

	pMem = (void*) ((u32)pMem + sizeof(GraphVert)*GetNumVerts());
	memcpy(pMem, m_paPoi, sizeof(CGraphPoi)*GetNumPoi());

	pMem = (void*) ((u32)pMem + sizeof(CGraphPoi)*GetNumPoi());
	
	memcpy(pMem, m_paEditVerts, sizeof(CEditVert)*GetNumVerts());
}


void CEditGraph::ResetFromMemory(void* pMem)
{
	CEditGraph* pBufferedGraph = (CEditGraph*) pMem;  //yikes!
	GraphVert* paBufferedGraphVerts = (GraphVert*) ((u32)pMem+sizeof(CEditGraph));  //yikes!
	CGraphPoi* paBufferedPoi = (CGraphPoi*) ( ((u32)paBufferedGraphVerts) +  sizeof(GraphVert)*pBufferedGraph->m_nNumVerts );  //yikes!
	CEditVert* paBufferedEditVerts = (CEditVert*) ( ((u32)paBufferedPoi) + sizeof(CGraphPoi)*pBufferedGraph->m_nNumPoi);  //yikes!

	FASSERT(m_nMaxVerts >= pBufferedGraph->m_nMaxVerts);
	if (m_nMaxVerts >= pBufferedGraph->m_nMaxVerts)
	{
		ClearSelected();
		GraphVert* paVerts = m_paVerts;
		CEditVert* paEditVerts = m_paEditVerts;
		s32 nMaxVerts = m_nMaxVerts;
		void* pVertAllocBuff = m_pVertAllocBuff;
		CGraphPoi* paPoi = m_paPoi;        
		u16	nMaxPoi = m_nMaxPoi;      
		void* pPoiAllocBuff = m_pPoiAllocBuff;

		//restore the graph (proper)!!!!!!!!
		*this = *pBufferedGraph;

		//keep un-restored ptrs
		m_paVerts = paVerts;
		m_paEditVerts = paEditVerts;
		m_nMaxVerts = nMaxVerts;
		m_pVertAllocBuff = pVertAllocBuff;
		m_paPoi = paPoi; 
		m_nMaxPoi = nMaxPoi;
		m_pPoiAllocBuff = pPoiAllocBuff;
		
		CNiList<s32> t;
		m_SelectedVerts = t;

		CNiList<CFVec3> tV3;
		m_SelectedOffsets = tV3;

		for (s32 i = 0; i < GetNumVerts(); i++)
		{
			m_paVerts[i] = paBufferedGraphVerts[i];
			m_paEditVerts[i] = paBufferedEditVerts[i];
			m_paEditVerts[i].m_uEVProps &= ~CEditVert::EV_SELECTED;
		}

		for (; i < this->GetMaxVerts(); i++)
		{
			m_paVerts[i].m_nVertSlotStatus = VERT_SLOT_FREE;
		}

		for (u16 poi = 0; poi < GetNumPoi(); poi++)
		{
			m_paPoi[poi] = paBufferedPoi[poi];
		}

		for (poi; poi < this->GetMaxPoi(); poi++)
		{
			m_paPoi[poi].m_nPoiSlotStatus = POI_SLOT_FREE;
		}
	
	
	}


}


void CEditGraph::TestEdgesWalkability(GraphVert* pV/* = NULL*/)
{
	s32 nBegin = 0;
	s32 nEnd = GetNumVerts();

	if (pV)
	{
		nBegin = GetVertId(pV);
		nEnd = nBegin+1;
	}

	for (s32 i = nBegin; i < nEnd; i++)
	{
		GraphVert* pV = m_paVerts+i;
		if (pV->m_nVertSlotStatus != VERT_SLOT_FREE && pV->Is2D())
		{
			for (s32 l = 0; l < pV->GetNumEdges(); l++)
			{
				//forward link
				//clear it
				s32 nSrcId = i;
				s32 nDestId = pV->m_aEdgeSlots[l].m_nNextVertId;
				GraphVert* pDestVert = GetVert(nDestId);
				GraphVert* pSrcVert = &(m_paVerts[i]);

				pV->m_aEdgeSlots[l].m_nProps &= ~EDGEPROP_FAILEDEDGETEST;


				CFVec3 deltaVec = GetVert(nDestId)->GetLocation().v3 - GetVert(nSrcId)->GetLocation().v3;
				f32 fDeltaMag2 = deltaVec.Mag2();
				deltaVec.Unitize();
				if (!aigraph_IsSurfaceWalkValid(GetVert(nSrcId)->GetLocation().v3, deltaVec, fDeltaMag2, 1.0f, CFVec3::m_UnitAxisY))
				{
					pV->m_aEdgeSlots[l].m_nProps |= EDGEPROP_FAILEDEDGETEST;
				}
				else
				{
					//check ray that runs down the right side  of the edge vol
					CFVec3 RightVec;
					RightVec  = CFVec3::m_UnitAxisY.Cross(deltaVec);

					CFVec3 RightVecStartPos;
					if (aigraph_IsSurfaceWalkValid(GetVert(nSrcId)->GetLocation().v3+RightVec, RightVec, aigraph_kfMinUsefullPathHalfWidth, 1.0f, CFVec3::m_UnitAxisY, NULL, &RightVecStartPos))
					{
						if (!aigraph_IsSurfaceWalkValid(RightVecStartPos, deltaVec, fDeltaMag2, 1.0f, CFVec3::m_UnitAxisY))
						{
							pV->m_aEdgeSlots[l].m_nProps |= EDGEPROP_FAILEDEDGETEST;
						}
						else
						{  	//check ray that runs down the left side  of the edge vol
							RightVec *= -1.0f;
							CFVec3 LeftVecStartPos;
							if (aigraph_IsSurfaceWalkValid(GetVert(nSrcId)->GetLocation().v3+RightVec, RightVec, aigraph_kfMinUsefullPathHalfWidth, 1.0f, CFVec3::m_UnitAxisY, NULL, &LeftVecStartPos))
							{
								if (!aigraph_IsSurfaceWalkValid(LeftVecStartPos, deltaVec, fDeltaMag2, 1.0f, CFVec3::m_UnitAxisY))
								{
									pV->m_aEdgeSlots[l].m_nProps |= EDGEPROP_FAILEDEDGETEST;
								}
							}
							else
							{
								//bad slope. not even wide enough for aigraph_kfMinUsefullPathHalfWidth of width to the left of the origin vert
								int i = 2;
							}

						}
					}
					else
					{
						//bad slope. not even wide enough for aigraph_kfMinUsefullPathHalfWidth of width to the right of the origin vert
						int i = 3;
					}
				}

				//fixup backward link
				FASSERT(pDestVert);
				GraphEdge *pE = pDestVert->GetEdgeTo(i);
				if (pE)
				{
					pE->m_nProps &= ~EDGEPROP_FAILEDEDGETEST;
					pE->m_nProps |= (pV->m_aEdgeSlots[l].m_nProps & EDGEPROP_FAILEDEDGETEST);
				}
			}
		}
	}
}


void CEditGraph::AutoLinkSelected(void)
{

	CNiIterator<s32> it = m_SelectedVerts.Begin();
	while (it.IsValid())
	{
		s32 nSrcId = it.Get();
		it.Next();

		CNiIterator<s32> it2 = m_SelectedVerts.Begin();
		while (it2.IsValid())
		{
			s32 nDestId = it2.Get();
			it2.Next();
			if (nSrcId == nDestId)
			{
				continue;
			}

			if (!IsConnected(nSrcId, nDestId))
			{	//can we connect them, should we connect them?

				CFVec3 deltaVec = GetVert(nDestId)->GetLocation().v3 - GetVert(nSrcId)->GetLocation().v3;
				float fDeltaMag2 = deltaVec.Mag2();
				deltaVec.Unitize();

				//findfix: one idea is to not allow any edges that have too small of an angle between them.
				//If there is a conflict like such, only connect the vert to the closer of the two
				if (aigraph_IsSurfaceWalkValid(GetVert(nSrcId)->GetLocation().v3, deltaVec, fDeltaMag2, 1.0f, CFVec3(0.0f, 1.0f, 0.0f)))
				{
					ConnectVerts(nSrcId, nDestId);	//calls pEdge->AutoSetDimesion();
				}
			}
			
		}
	}

}


void CEditGraph::UnLinkSelected(void)
{

	CNiIterator<s32> it = m_SelectedVerts.Begin();
	while (it.IsValid())
	{
		s32 nSrcId = it.Get();
		it.Next();

		CNiIterator<s32> it2 = m_SelectedVerts.Begin();
		while (it2.IsValid())
		{
			s32 nDestId = it2.Get();
			it2.Next();
			if (nSrcId == nDestId)
			{
				continue;
			}

			if (IsConnected(nSrcId, nDestId))
			{	//can we connect them, should we connect them?
				DisconnectVerts(nSrcId, nDestId);
			}
		}
	}

}


u16 CEditGraph::GetNthSelected(s32 nN)
{
	CNiIterator<s32> it = m_SelectedVerts.Begin();
	s32 i = 0;
	while (it.IsValid() && i != nN)
	{
		it.Next();
		i++;
	}

	if (it.IsValid() && i==nN)
	{
		s32 sVertId = it.Get();
		return (u16) sVertId;
	}
	return 0;
}




void CEditGraph::DeleteSelected(void)
{
	for (s32 i = 0; i < GetNumVerts(); i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE && m_paEditVerts[i].m_uEVProps & CEditVert::EV_SELECTED)
		{
			RemoveVert(i);
		}
	}
	m_SelectedVerts.RemoveAll();
}


void CEditGraph::ClearSelected(void)
{
	m_SelectedVerts.RemoveAll();
	m_SelectedOffsets.RemoveAll();
	for (s32 i = 0; i < GetNumVerts(); i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			m_paEditVerts[i].m_uEVProps &= ~CEditVert::EV_SELECTED;
		}
	}
}


void CEditGraph::ClearPropFromAllVerts(u32 nProps)  //bits in nProps will be cleared from all vert props
{
	for (s32 i = 0; i < GetNumVerts(); i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			m_paEditVerts[i].m_uEVProps &= ~nProps;
		}
	}
}


void CEditGraph::SetVertProperty(s32 nVertId, u32 nProps)  //bits in nProps will be or'd into this verts m_uEVProps field
{
	m_paEditVerts[nVertId].m_uEVProps |= nProps;
}



BOOL CEditGraph::IsSelected(GraphVert* pVert)
{
	return (m_paEditVerts[GetVertId(pVert)].m_uEVProps & CEditVert::EV_SELECTED);
}


BOOL CEditGraph::ToggleSelectVert(GraphVert* pVert)
{
	if 	(m_paEditVerts[GetVertId(pVert)].m_uEVProps & CEditVert::EV_SELECTED)
	{
		UnSelectVert(pVert);
		return 0;
	} 
	else
	{
		SelectVert(pVert);
		return 1;
	}

}


void CEditGraph::SelectVert(GraphVert* pVert)
{
	m_SelectedVerts.PushHead(GetVertId(pVert));
	m_paEditVerts[GetVertId(pVert)].m_uEVProps |= CEditVert::EV_SELECTED;
}


void CEditGraph::UnSelectVert(GraphVert* pVert)
{
	FASSERT(m_paEditVerts[GetVertId(pVert)].m_uEVProps & CEditVert::EV_SELECTED);
	BOOL didIt = m_SelectedVerts.Remove(GetVertId(pVert));
	FASSERT(didIt);

	m_paEditVerts[GetVertId(pVert)].m_uEVProps &= ~CEditVert::EV_SELECTED;
}


GraphVert *CEditGraph::AllocVertSlot(void)
{
	GraphVert * pV = CAIGraph::AllocVertSlot();
	FASSERT(pV);

	if (GetMaxVerts() > m_nMaxEditVerts)
	{  //just added alloced new vert
		s32 newMax = GetMaxVerts();
		CEditVert* paEditVerts = new CEditVert[newMax];
		for (s32 i = 0; i < m_nMaxEditVerts; i++)
		{
			paEditVerts[i] = m_paEditVerts[i];
		}

		for (i = m_nMaxEditVerts; i < newMax; i++)
		{
			paEditVerts[i].m_uEVProps = 0;
		}
		m_nMaxEditVerts = newMax;
		delete [] m_paEditVerts;	//delete old memory
		m_paEditVerts = paEditVerts;
		delete [] m_panRenderPropTmp;
		m_panRenderPropTmp = new u32[newMax];
		delete [] m_panEdgeRenderPropTmp;
		m_panEdgeRenderPropTmp = new u32[newMax*AIGRAPH_NUM_EDGES_PER_VERT];
	}
	m_paEditVerts[GetVertId(pV)].m_uEVProps = CEditVert::EV_ALLOCATED;
	return pV;
}


void CEditGraph::FreeVertSlot(s32 nSlotId)
{
	CAIGraph::FreeVertSlot(nSlotId);
}


s32 CEditGraph::FindClosestEdge(const CFVec3 &RayOrigin, const CFVec3 &RayDir, f32 fMinDistance, GraphEdge** ppEdge)
{
	MgcSegment3 mgcEdgeSeg;
	MgcRay3		mgcRay;
	BOOL bHitEdge = 0;
  

	*ppEdge = NULL;

	for (s32 i = 0; i < GetNumVerts(); i++)
	{
		if (m_paVerts[i].m_nVertSlotStatus != VERT_SLOT_FREE)
		{
			for (s32 e = 0; e < m_paVerts[i].GetNumEdges(); e++)
			{
				if (i < m_paVerts[i].GetEdge(e)->m_nNextVertId)  //this test prevents double links from getting tested twice
				{
					mgcEdgeSeg.Origin().x = m_paVerts[i].GetLocation().x;
					mgcEdgeSeg.Origin().y = m_paVerts[i].GetLocation().y;
					mgcEdgeSeg.Origin().z = m_paVerts[i].GetLocation().z;

					mgcEdgeSeg.Direction().x = GetVert(m_paVerts[i].GetEdge(e)->m_nNextVertId)->GetLocation().x - mgcEdgeSeg.Origin().x;
					mgcEdgeSeg.Direction().y = GetVert(m_paVerts[i].GetEdge(e)->m_nNextVertId)->GetLocation().y - mgcEdgeSeg.Origin().y;
					mgcEdgeSeg.Direction().z = GetVert(m_paVerts[i].GetEdge(e)->m_nNextVertId)->GetLocation().z - mgcEdgeSeg.Origin().z;

					mgcRay.Origin().x = RayOrigin.x;
					mgcRay.Origin().y = RayOrigin.y;
					mgcRay.Origin().z = RayOrigin.z;

					CFVec3 RayDirUnit = RayDir;
					RayDirUnit.Unitize();
					

					mgcRay.Direction().x = RayDirUnit.x;
					mgcRay.Direction().y = RayDirUnit.y;
					mgcRay.Direction().z = RayDirUnit.z;

					f32 fDist2 = MgcSqrDistance(mgcRay, mgcEdgeSeg);
					if 	(fDist2 < fMinDistance*fMinDistance)
					{
						*ppEdge = m_paVerts[i].GetEdge(e);
						return i;
					}
				}
			}
		}
	}
	return 0;
}


BOOL CEditGraph::IsNear(GraphVert* pV)
{
	return GetEditVert(GetVertId(pV))->m_uEVProps & CEditVert::EV_SELECTED;
	return TRUE;
}


BOOL CEditGraph::IsNear(GraphEdge* pEdge)
{
	u16 nVertAId = GetVertId((GraphVert*)pEdge);
	u16 nVertBId = pEdge->m_nNextVertId;
	return (GetEditVert(nVertAId)->m_uEVProps & CEditVert::EV_SELECTED || GetEditVert(nVertBId)->m_uEVProps & CEditVert::EV_SELECTED);
}

BOOL _bBlinkOn = FALSE;
u16 _uBlinkCounter = 0;

void CEditGraph::Render(void)
{
	_uBlinkCounter++;
	_uBlinkCounter &=0xf;
	if (_uBlinkCounter == 0)
	{
		_bBlinkOn = 1-_bBlinkOn;
	}

	for (s32 i = 1; i < GetNumVerts(); i++)
	{
		if (!IsVertValid(i))
		{
			continue;
		}
		GraphVert *pV = &m_paVerts[i];
		CEditVert *pEV = &m_paEditVerts[i];

		m_panRenderPropTmp[i] = 0;
		if (pEV->m_uEVProps & CEditVert::EV_SELECTED)
		{
			m_panRenderPropTmp[i] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORGREEN;
		}
		else if (pEV->m_uEVProps & CEditVert::EV_ONHOTPATH)
		{
			m_panRenderPropTmp[i] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORYELLOW;
		}
		else
		{
			m_panRenderPropTmp[i] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORWHITE;
		}
	
		if (m_nDrawVolMode == DRAWVOLMODE_CUSTOM && !(pV->m_nProps & VERTPROP_CUSTOM_VOL))
		{
			m_panRenderPropTmp[i] |= AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVOL;
		}
		else if (m_nDrawVolMode == DRAWVOLMODE_NEAR && !IsNear(pV))
		{
			m_panRenderPropTmp[i] |= AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVOL;
		}

		for (s32 j = 0; j < m_paVerts[i].GetNumEdges(); j++)
		{
			s32 nextVertId = pV->m_aEdgeSlots[j].m_nNextVertId;
			GraphEdge *pE = &(pV->m_aEdgeSlots[j]);
			GraphVert *pVb = &m_paVerts[nextVertId];
			s32 nEdgeId = GetEdgeId(pE);

			if (pE->Is3D())
			{
				if ((pEV->m_uEVProps & CEditVert::EV_ONHOTPATH) && (m_paEditVerts[nextVertId].m_uEVProps & CEditVert::EV_ONHOTPATH))
				{
					m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORYELLOW;
				}
				else
				{
					m_panEdgeRenderPropTmp[nEdgeId] = AIGRAPH_DEBUGRENDER_VERTFLAG_COLORWHITE;
				}
			}
			else
			{
				m_panEdgeRenderPropTmp[nEdgeId] = 0;
				if ((pEV->m_uEVProps & CEditVert::EV_ONHOTPATH) && (m_paEditVerts[nextVertId].m_uEVProps & CEditVert::EV_ONHOTPATH))
				{
					m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORYELLOW;
				}
				else if (m_paVerts[i].m_aEdgeSlots[j].m_nProps & EDGEPROP_FAILEDEDGETEST && !(m_paVerts[i].m_aEdgeSlots[j].m_nProps & EDGEPROP_CUSTOM_VOL))
				{
					m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORRED;
				}
				else if ((m_paVerts[i].m_aEdgeSlots[j].m_nProps & EDGEPROP_ALL_CUSTOM) &&
							((!(m_paVerts[i].m_aEdgeSlots[j].m_nProps & EDGEPROP_CUSTOM_VOL))  || ((m_paVerts[i].m_aEdgeSlots[j].m_nProps & EDGEPROP_CUSTOM_VOL) && _bBlinkOn)))
				{
					m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORYELLOW;
				}
				else if (m_paVerts[i].m_aEdgeSlots[j].m_nProps & EDGEPROP_CUSTOM_VOL)
				{
					m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORBLUE;
				}
				else
				{
					m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_COLORWHITE;
				}
			}
			if (m_nDrawVolMode == DRAWVOLMODE_CUSTOM && !(pE->m_nProps & EDGEPROP_CUSTOM_VOL))
			{
				m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVOL;
			}
			else if	(m_nDrawVolMode == DRAWVOLMODE_NEAR && !IsNear(pE))
			{
				m_panEdgeRenderPropTmp[nEdgeId] |= AIGRAPH_DEBUGRENDER_VERTFLAG_DONTDRAWVOL;
			}
		}
	}

	FViewport_t * pView = fviewport_GetActive( );
	if (pView)
	{
		CFVec3A CamLookAt;
		CFVec3A tmpAxisZ = CFVec3A::m_UnitAxisZ;
		CamLookAt = FXfm_pView->m_MtxR.MulDir(tmpAxisZ);
		CamLookAt.Unitize();

		CFVec3A CamPos;
		CamPos.Set(FXfm_pView->m_MtxR.m_vPos);

		u32 uRenderFlags;
		uRenderFlags = AIGRAPH_DEBUG_RENDER_CULLBEHIND;

		if (m_bDrawVerts)
		{
			uRenderFlags |= (AIGRAPH_DEBUG_RENDER_VERTS | AIGRAPH_DEBUG_RENDER_EDGES);
		}
		if (m_bDrawVertVolumes)
		{
			uRenderFlags |= (AIGRAPH_DEBUG_RENDER_VERTVOLS);
		}
		if (m_bDrawEdgeVolumes)
		{
			uRenderFlags |= (AIGRAPH_DEBUG_RENDER_EDGEVOLS);
		}
		if (m_b3DMode)
		{
			uRenderFlags |= (AIGRAPH_DEBUG_RENDER_3DMODE);
		}
		if (m_b2DMode)
		{
			uRenderFlags |= (AIGRAPH_DEBUG_RENDER_2DMODE);
		}

		CAIGraph::Render(	CamPos,
							CamLookAt,
							0.0f,  //cos 90 degrees
							uRenderFlags,
							m_panRenderPropTmp,
							m_panEdgeRenderPropTmp);
	}
}


void CEditGraph::RecordSelectedOffsets(GraphVert* pRoot)
{
	//clear the selectedOffsets list, we're about to make a new one to match the currently selected verts
	m_SelectedOffsets.RemoveAll();

	CNiIterator<s32> it = m_SelectedVerts.Begin();
	while (it.IsValid())
	{
		s32 nSrcId = it.Get();
		GraphVert* pV = GetVert(nSrcId);
		it.Next();

		m_SelectedOffsets.PushTail(pV->GetLocation().v3 - pRoot->GetLocation().v3);
	}

	FASSERT(m_SelectedOffsets.Size() == m_SelectedVerts.Size());
}


void CEditGraph::ApplySelectedOffsets(GraphVert* pRoot)
{
	FASSERT(m_SelectedOffsets.Size() == m_SelectedVerts.Size());

	CNiIterator<s32> itVertId = m_SelectedVerts.Begin();
	CNiIterator<CFVec3> itOffsetVec = m_SelectedOffsets.Begin();
	while (itVertId.IsValid())
	{
		s32 nSrcId = itVertId.Get();
		GraphVert* pV = GetVert(nSrcId);
		CFVec3 VertOffset = itOffsetVec.Get();

		itVertId.Next();
		itOffsetVec.Next();

		pV->m_Location.Set(pRoot->GetLocation().v3+VertOffset);
	}
}

