//////////////////////////////////////////////////////////////////////////////////////
// MaiJoinTool.cpp - Tool for joining verts in the pathfinding graph
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
// 02/20/02 patm       Created
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "MaiJoinTool.h"
#include "fcoll.h"
#include "fworld.h"
#include "MaiEditGraph.h"
#include "MaiUndo.h"
#include "MaiSelectTool.h"

JoinTool::JoinTool()
: BaseTool(), m_nVertAId(-1), m_bUnLinkMode(0), m_uJoinStyle(JOIN_STYLE_2D)
{
}


void JoinTool::OnFocus()
{
	m_nVertAId = -1;
	if (maimain_pEditGraph && maimain_pEditGraph->GetNumSelected() > 1)
	{
		//findfix: could make this mean Auto-Link!
		maimain_pEditGraph->ClearSelected();
	}
	else if (maimain_pEditGraph && maimain_pEditGraph->GetNumSelected())
	{
		m_nVertAId = maimain_pEditGraph->GetNthSelected(0);
	}
}


void JoinTool::UserInput(const CMaiUserInput& rMaiUI)
{
	if (rMaiUI.bLastLeftOnDown)
	{
		GraphVert* pV;
		if (m_uJoinStyle== JOIN_STYLE_2D)
		{
			pV = maimain_pEditGraph->FindClosestIntersectingVert2D(maimain_MouseRayOrigin, maimain_MouseRayEnd, maimain_pEditGraph->m_fVertVisualRadius);
		}
		else
		{
			pV = maimain_pEditGraph->FindClosestIntersectingVert3D(maimain_MouseRayOrigin, maimain_MouseRayEnd, maimain_pEditGraph->m_fVertVisualRadius);
		}

		if (pV)
		{
			s32 thisVertId = maimain_pEditGraph->GetVertId(pV);
			//toggle Vert selection status
			if (m_nVertAId == thisVertId)
			{
				if (!maimain_pEditGraph->ToggleSelectVert(pV))
				{ //just un-selected
					m_nVertAId = -1;
				}
			}
			else if (m_nVertAId != -1)
			{
				maiundo_Store();

				if (m_bUnLinkMode)
				{
					if (maimain_pEditGraph->IsConnected((u16)m_nVertAId, (u16)thisVertId))
					{
						maimain_pEditGraph->DisconnectVerts((u16)m_nVertAId, (u16)thisVertId);
					}
				}
				else
				{
					maimain_pEditGraph->ConnectVerts((u16)m_nVertAId, (u16)thisVertId);	 //calls autosetdimension on the new Edge
				}
				maimain_pEditGraph->ClearSelected();
				maimain_pEditGraph->SelectVert(pV);
				m_nVertAId = thisVertId;
			}
			else
			{
				maimain_pEditGraph->ClearSelected();
				maimain_pEditGraph->SelectVert(pV);
				m_nVertAId = thisVertId;
			}

		}
		else
		{

			//begin drag rectangle
		//	FASSERT(!m_bDragging);

		}
	}

	if (rMaiUI.bLastLeftOnUp)
	{
	}


	if (GetAsyncKeyState( VK_DELETE ) < 0)
	{	//delete key went down. remove points in current selection list
		maiundo_Store();
		maimain_pEditGraph->DeleteSelected();
		m_nVertAId = -1;
	}

	//
	// right clicks customize edges and verts
	//
	maiselectTool_DoRightClickCustomizeUI(rMaiUI);

}
