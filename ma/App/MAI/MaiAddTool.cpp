//////////////////////////////////////////////////////////////////////////////////////
// MaiAddTool.h - Tool for adding pathfinding verts
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
#include "stdafx.h"
#include "fang.h"
#include "MaiAddTool.h"
#include "fcoll.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "MaiEditGraph.h"
#include "MaiUndo.h"
#include "MaiSelectTool.h"
#include "MaiToolBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


AddTool::AddTool()
: SelectTool(), m_bJoinMode(FALSE), m_bNoNewVertOnUp(FALSE)
{
}


void AddTool::OnFocus(void)
{
	m_bNoNewVertOnUp = FALSE;

	SelectTool::OnFocus();
}



void AddTool::UserInput(const CMaiUserInput& rMaiUI)
{
	GraphVert* pLastV = NULL;
	CMaiUserInput FilterUI = rMaiUI;

	FASSERT(maimain_pEditGraph);

	BOOL bDoAutoDimension = (maimain_pMainToolBarWindow && 
								maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() &&
								!maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_bManModeOn);

	if (rMaiUI.bLastLeftOnDown)
	{
		GraphVert* pVA = maimain_pEditGraph->FindClosestIntersectingVert2D(maimain_MouseRayOrigin, maimain_MouseRayImpactPt, maimain_pEditGraph->m_fVertVisualRadius);
		if (!pVA)
		{	//prevent base class from clearing the selection
			FilterUI.bLastLeftOnDown = FALSE;
			m_bBeginMove = 1;
			m_bNoNewVertOnUp = FALSE;
		}
		else
		{
			m_bNoNewVertOnUp = TRUE;
			if (maimain_pEditGraph->GetNumSelected() == 0)
			{
			}
			else if (maimain_pEditGraph->GetNumSelected() == 1)
			{
				u16 uLastVertId = maimain_pEditGraph->GetNthSelected(0);
				GraphVert* pLastV = maimain_pEditGraph->GetVert(uLastVertId);
				if (pLastV && 
					uLastVertId > 0 && 
					pLastV->Is2D() &&
					pLastV != pVA
					)
				{   //link

					if (m_bJoinMode && pLastV)
					{
						if (maimain_pEditGraph->ConnectVerts(maimain_pEditGraph->GetVertId(pLastV), maimain_pEditGraph->GetVertId(pVA), bDoAutoDimension))
						{
							//don't allow drag if we just linked
							FilterUI.bLastLeftOnDown = FALSE;
							m_bBeginMove = 0;

						}
					}
				}
				maimain_pEditGraph->ClearSelected();
				maimain_pEditGraph->SelectVert(pVA);
			}
		}

	}


	if (rMaiUI.bLastLeftOnUp && 
		!m_bNoNewVertOnUp &&
		maimain_bMouseRayImpactThisFrame &&
		m_bBeginMove == 1)	 //m_bBeginMove == 1 means that no point was moved and no dragrect is going!
	{
		if (maimain_pEditGraph->GetNumSelected() <= 1 )
		{	//Add a new vert
			u16 uLastVertId = maimain_pEditGraph->GetNthSelected(0);
			GraphVert* pLastV = NULL;
			if (uLastVertId >0)
			{
				pLastV = maimain_pEditGraph->GetVert(uLastVertId);
			}

			if (!(pLastV && 
				uLastVertId > 0 &&
				pLastV->Is2D()))
			{
				 uLastVertId = 0;
			}
			else
			{
			}
			pLastV = NULL; //re-acquire pLastV after new vert is added.

			CFVec3A AddPos;
			AddPos = maimain_MouseRayImpactPt;//maimain_MouseRayImpactPt is pre-bumped by aigraph_kfSurfaceOffset along surface normal.  little fudgy here.  This adjust to surface point insn't quite perfect. But, since

			maiundo_Store();
			GraphVert* pV = maimain_pEditGraph->AddVert(AddPos);
			if (pV)
			{
				u16 uVId = maimain_pEditGraph->GetVertId(pV);
				pLastV = maimain_pEditGraph->GetVert(uLastVertId);

				if (bDoAutoDimension)
				{
					pV->AutoSetDimension2D();
				}
				else
				{
					if (pLastV)
					{
						pV->m_fSafeRad = pLastV->m_fSafeRad;
						pV->m_fHeightClearance = pLastV->m_fHeightClearance;
					}
					else
					{
						pV->m_fSafeRad = 4.0;
						pV->m_fHeightClearance = 4.0f;
					}
				}

				if (m_bJoinMode && uLastVertId > 0)
				{
					maimain_pEditGraph->ConnectVerts( uLastVertId, uVId, bDoAutoDimension);	//bDoAutoDimension ==  TRUE means call pEdge->AutoSetDimesion();
				}

				//switch current selection
				maimain_pEditGraph->ClearSelected();
				maimain_pEditGraph->SelectVert(pV);
				m_bBeginMove = 0;  //stops base class from clearing.
			}
		}
	}

	SelectTool::UserInput(FilterUI);
}

/*
	FCollImpact_t oCollInfo;
	GraphVert* pLastV = NULL;

	FASSERT(maimain_pEditGraph);

	if (rMaiUI.bLastLeftOnDown)
	{															  
		if (fworld_FindClosestImpactPointToRayStart(&oCollInfo, &maimain_MouseRayOrigin, &maimain_MouseRayEnd))
		{
			GraphVert* pVA = maimain_pEditGraph->FindClosestIntersectingVert2D(maimain_MouseRayOrigin, maimain_MouseRayEnd, maimain_pEditGraph->m_fVertVisualRadius);
			 
			if (pVA && (pVA->GetLocation().v3 - maimain_MouseRayOrigin.v3).Mag2() < (oCollInfo.ImpactPoint.v3 - maimain_MouseRayOrigin.v3).Mag2())
			{	//Change currently selected vert

				if (m_nLastVertId >= 0 && pVA == maimain_pEditGraph->GetVert((u16) m_nLastVertId))
				{
					//toggle Vert selection status
					if (!maimain_pEditGraph->ToggleSelectVert(pVA))
					{ //just un-selected
						m_nLastVertId = -1;
					}
				}
				else
				{
					//switch current selection
					maimain_pEditGraph->ClearSelected();
					maimain_pEditGraph->SelectVert(pVA);
					m_nLastVertId = maimain_pEditGraph->GetVertId(pVA);
				}
			}
			else
			{	//Add a new vert
				CFVec3A BumpLoc;
				BumpLoc.v3 = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset);	 //little fudgy here.  This adjust to surface point insn't quite perfect. But, since
				GraphVert* pV = maimain_pEditGraph->AddVert(BumpLoc);
				if (pV)
				{
					u16 nVId = maimain_pEditGraph->GetVertId(pV);

					maiundo_Store();

					if (maimain_pMainToolBarWindow && 
						maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() &&
						!maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_bManModeOn)
					{
						pV->AutoSetDimension2D();
					}
					else
					{

						if (m_nLastVertId >0)
						{
							pLastV = maimain_pEditGraph->GetVert((u16) m_nLastVertId);
						}

						if (pLastV)
						{
							pV->m_fSafeRad = pLastV->m_fSafeRad;
							pV->m_fHeightClearance = pLastV->m_fHeightClearance;
						}
						else
						{
							pV->m_fSafeRad = 4.0;
							pV->m_fHeightClearance = 4.0f;
						}
					}

					if (m_bJoinMode && m_nLastVertId >0)
					{
						maimain_pEditGraph->ConnectVerts((u16)m_nLastVertId, (u16)nVId, !maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_bManModeOn);	//calls pEdge->AutoSetDimesion();
					}

					//switch current selection
					maimain_pEditGraph->ClearSelected();
					m_nLastVertId = -1;
					maimain_pEditGraph->SelectVert(pV);
					m_nLastVertId = nVId;
				}
			}
		}
	}

*/