//////////////////////////////////////////////////////////////////////////////////////
// MaiAddTool3D.h - Tool for adding pathfinding verts
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
#include "fcoll.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fvid.h"
#include "dx\fdx8vid.h"
#include "MaiAddTool3D.h"
#include "MaiEditGraph.h"
#include "MaiUndo.h"
#include "MaiSelectTool.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


AddTool3D::AddTool3D()
:	SelectTool(),
	m_bJoinMode(TRUE),
	m_fLastAddDist(0.0f)
{
	m_uMoveStyle =	MOVE_STYLE_3D;
	m_bNoNewVertOnUp = FALSE;
}


void AddTool3D::OnFocus(void)
{
/*	GraphVert* pLastV = NULL;

	m_nLastVertId = 0;

	if (maimain_pEditGraph && maimain_pEditGraph->GetNumSelected() > 1)
	{	//started link Add with more than one vert selected... not sure what to do, so clear all selected.
		maimain_pEditGraph->ClearSelected();
	}
	else if (maimain_pEditGraph->GetNumSelected() == 1)
	{
		m_nLastVertId = maimain_pEditGraph->GetNthSelected(0);
		if (m_nLastVertId > 0  &&
			maimain_pEditGraph->IsVertValid(m_nLastVertId))
		{
			pLastV = maimain_pEditGraph->GetVert((u16)m_nLastVertId);
			CFVec3A CamPos;
			if (pLastV && 
				pLastV->Is3D() &&
				maimain_GetCamPos(&CamPos))
			{
					m_fLastAddDist = pLastV->GetLocation().Dist(CamPos);
			}
			else
			{
				m_nLastVertId = 0;
			}

		}
		else
		{
			m_nLastVertId = 0
		}
	}
*/
	m_fLastAddDist = 20.0f;
	m_bNoNewVertOnUp = FALSE;

	SelectTool::OnFocus();
}



void AddTool3D::UserInput(const CMaiUserInput& rMaiUI)
{
	GraphVert* pLastV = NULL;
	CMaiUserInput FilterUI = rMaiUI;

	FASSERT(maimain_pEditGraph);


	if (rMaiUI.bLastLeftOnDown)
	{
		GraphVert* pVA = maimain_pEditGraph->FindClosestIntersectingVert3D(maimain_MouseRayOrigin, maimain_MouseRayImpactPt, maimain_pEditGraph->m_fVertVisualRadius);
		if (!pVA)
		{	//prevent base class from clearing the selection
			FilterUI.bLastLeftOnDown = FALSE;
			m_bBeginMove = 1;
		}
		else
		{
			if (maimain_pEditGraph->GetNumSelected()==0)
			{
				m_bNoNewVertOnUp = TRUE;
			}
			else if (maimain_pEditGraph->GetNumSelected()==1)
			{
				u16 uLastVertId = maimain_pEditGraph->GetNthSelected(0);
				GraphVert* pLastV = maimain_pEditGraph->GetVert(uLastVertId);
				f32 fAddDist = m_fLastAddDist;
				if (pLastV && 
					uLastVertId > 0 && 
					pLastV->Is3D() &&
					pLastV != pVA)
				{   //link


					if (m_bJoinMode && pLastV)
					{
						if (maimain_pEditGraph->ConnectVerts(maimain_pEditGraph->GetVertId(pLastV), maimain_pEditGraph->GetVertId(pVA)))
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
		m_bBeginMove == 1)	 //m_bBeginMove ==1 means that no point was moved and no dragrect is going!
	{
		m_bNoNewVertOnUp = FALSE;
		if (maimain_pEditGraph->GetNumSelected() <= 1)
		{	//Add a new vert
			u16 uLastVertId = maimain_pEditGraph->GetNthSelected(0);
			GraphVert* pLastV = maimain_pEditGraph->GetVert(uLastVertId);
			CFVec3A CamPos;
			f32 fAddDist = m_fLastAddDist;
			if (!(pLastV && 
				uLastVertId > 0 &&
				pLastV->Is3D() &&
				maimain_GetCamPos(&CamPos)))
			{
				 uLastVertId = 0;
			}
			pLastV = NULL; //re-acquire pLastV after new vert is added.

			f32 fNewRayLen = maimain_MouseRayImpactPt.Dist(maimain_MouseRayOrigin) - 2.0f;
			if (fAddDist > fNewRayLen)
			{
				fAddDist = fNewRayLen;
			}

			CFVec3A AddPos;
			AddPos.Sub(maimain_MouseRayEnd, maimain_MouseRayOrigin);
			if (AddPos.SafeUnitAndMag(AddPos) > 0.0f)
			{

				AddPos.Mul(fAddDist);
				AddPos.Add(maimain_MouseRayOrigin);

				maiundo_Store();
				GraphVert* pV = maimain_pEditGraph->AddVert(AddPos);
				pLastV = maimain_pEditGraph->GetVert(uLastVertId);
				if (pV)
				{
					m_fLastAddDist = fAddDist;
					u16 uVId = maimain_pEditGraph->GetVertId(pV);

					pV->m_nProps |= (VERTPROP_CUSTOM_VOL | VERTPROP_3D);
					pV->m_fSafeRad = 10.0f;
					pV->m_fHeightClearance = 10.0f;

					if (uLastVertId)
					{
						FASSERT(pLastV->Is3D());
						pV->m_fSafeRad = pLastV->m_fSafeRad;
						pV->m_fHeightClearance = pLastV->m_fHeightClearance;
					}
					FASSERT(pV->Is3D());

					if (m_bJoinMode && uLastVertId)
					{
						maimain_pEditGraph->ConnectVerts(uLastVertId, uVId, FALSE);	//False means don't call pEdge->AutoSetDimesion();
					}

					//switch current selection
					maimain_pEditGraph->ClearSelected();
					maimain_pEditGraph->SelectVert(pV);
					m_bBeginMove = 0;  //stops base class from clearing.
				}
			}
		}
	}

	SelectTool::UserInput(FilterUI);
}

