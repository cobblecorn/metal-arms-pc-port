//////////////////////////////////////////////////////////////////////////////////////
// MaiPoiTool.cpp - 
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
// 08/16/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "MaiSelectTool.h"
#include "MaiEditGraph.h"
#include "MaiPoiTool.h"
#include "frenderer.h"
#include "fworld.h"
#include "MaiUndo.h"
#include "MaiToolbar.h"
#include "fdraw.h"
#include "resource.h"
#include "PoiPropsDlg.h"
#include "dx\fdx8vid.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static FDrawVtx_t _aoDrawGuidesVertsBuffer[2];

CNiList<CGraphPoi*> CPoiTool::m_SelectedPoi;
enum
{
  POICOLOR_OFF = 0,  //off
  POICOLOR_RED,
  POICOLOR_GREEN,
  POICOLOR_BLUE,
  POICOLOR_YELLOW,
  POICOLOR_CYAN,
  POICOLOR_MAGENTA,
  POICOLOR_WHITE,  //off
  NUM_POICOLORS
};		   

static CFColorRGBA _aPoiColors[NUM_POICOLORS];
CGraphPoi* CNiIterator<CGraphPoi*>::s_ReturnError;

void CPoiTool::InitSystem(void)
{
	_aPoiColors[POICOLOR_WHITE].SetColor(FColor_MotifWhite);
	_aPoiColors[POICOLOR_RED].SetColor(FColor_MotifRed);
	_aPoiColors[POICOLOR_GREEN].SetColor(FColor_MotifGreen);
	_aPoiColors[POICOLOR_BLUE].SetColor(FColor_MotifBlue);
	_aPoiColors[POICOLOR_YELLOW].SetColor(1.0f, 1.0f, 0.0f);
	_aPoiColors[POICOLOR_MAGENTA].SetColor(1.0f, 0.0f, 1.0f);
	_aPoiColors[POICOLOR_CYAN].SetColor(0.0f, 1.0f, 1.0f);
}


void CPoiTool::UninitSystem(void)
{
	m_SelectedPoi.RemoveAll();
}


void CPoiTool::SelectPoi(CGraphPoi* pPoi)
{
	if (!m_SelectedPoi.Find(pPoi))
	{
		m_SelectedPoi.PushTail(pPoi);
	}
}


void CPoiTool::ClearSelectedPoi(void)
{
	m_SelectedPoi.RemoveAll();
}


void CPoiTool::DeleteSelectedPoi(void)
{
	CNiIterator<CGraphPoi*> it= m_SelectedPoi.Begin();
	while (it.IsValid())
	{
		maimain_pEditGraph->RemovePoi(maimain_pEditGraph->GetPoiId(it.Get()));
		it.Next();
	}
	m_SelectedPoi.RemoveAll();
}


void CPoiTool::UnselectPoi(CGraphPoi* pPoi)
{
	m_SelectedPoi.Remove(pPoi);
}


BOOL CPoiTool::IsPoiSelected(CGraphPoi* pPoi)
{
	return m_SelectedPoi.Find(pPoi);
}

s32 CPoiTool::m_bMakingDragRect = FALSE;
CFVec2 CPoiTool::m_DragRectOrigin;
CFVec2 CPoiTool::m_DragRectCur;

CPoiTool::CPoiTool(void)
: BaseTool(),
  m_nMovingPoi(0),
  m_bBeginMove(0),
  m_nUnselectPoiIdOnUp(-1),
  m_uMoveStyle(MOVE_STYLE_2D)
{
	m_DragRectOrigin.Zero();
	m_DragRectCur.Zero();
	m_bOffensivePoiMode = FALSE;

	//make sure this is done atleast once
	CGraphPoi::InitVisSensors();
}


void CPoiTool::UserInput(const CMaiUserInput& rMaiUI)
{
	if (rMaiUI.bLastLeftOnDown)
	{
		CGraphPoi* pPoi = NULL;
		m_nUnselectPoiIdOnUp = 0;

		if (maimain_bMouseRayImpactThisFrame)
		{
			pPoi = maimain_pEditGraph->FindClosestIntersectingPoi(maimain_MouseRayOrigin, maimain_MouseRayEnd);
			if (pPoi)
			{
				if (IsPoiSelected(pPoi))
				{
					m_nUnselectPoiIdOnUp = maimain_pEditGraph->GetPoiId(pPoi);
				}
				else
				{
					if (!(GetAsyncKeyState( VK_CONTROL ) < 0))
					{
						ClearSelectedPoi();
					}
					SelectPoi(pPoi);
				}

//pgm: disabled moving poi
//     there are quite a few issues with this since poi are stored relative to a graphvert
//     and also, store position as an int, not float
//     hopefully poi will be simple enough to create that deleting them and recreating them won't be too much of a hassle
				//				m_nMovingPoi = maimain_pEditGraph->GetPoiId(pPoi);
				m_bBeginMove = 1;
			}
			else
			{
				if (GetAsyncKeyState( VK_CONTROL ))
				{
					  // ctr + left click won't clear selection, or add a new poi
				}
				else if (GetAsyncKeyState( VK_SHIFT ))
				{
					  // shift + left click clear selection, no new poi
					ClearSelectedPoi();
				}
				else
				{
					ClearSelectedPoi();

					//going to add one now!!!!!!!!!
					//poi are associated with a vert, so find a vert first
					GraphVert* pV = maimain_pEditGraph->FindClosestLOSVert2D(maimain_MouseRayImpactPt, TRUE);
					if (pV)
					{
						maiundo_Store();
						if (pPoi = maimain_pEditGraph->AddPoi(maimain_MouseRayImpactPt))
						{
							SelectPoi(pPoi);

							//set some flags on the new pPoi
							if (m_bOffensivePoiMode)
							{
								pPoi->m_uFlags |= CGraphPoi::POIFLAG_OFFENSIVE_HINT;
							}
						}
					}
				}
			}
		}

		if (m_bMakingDragRect)
		{
			m_bMakingDragRect = FALSE;
		}

		if (!pPoi && !m_nMovingPoi)
		{
			m_DragRectOrigin.x = (f32) rMaiUI.nLastMouseX;
			m_DragRectOrigin.y = (f32) rMaiUI.nLastMouseY;
			m_bMakingDragRect = TRUE;	
		}

	}


	if (rMaiUI.bLastLeftOnUp)
	{
		if (m_bMakingDragRect)
		{
			//select those poi whose 2D projection is within the rect
			m_bMakingDragRect = FALSE;
		}


		if (m_nMovingPoi)
		{
			CGraphPoi* pPoi = maimain_pEditGraph->GetPoi(m_nMovingPoi);
			FASSERT(pPoi);

			if (m_bBeginMove == 0)	 
			{
				maimain_pEditGraph->MovePoi(m_nMovingPoi, CFVec3A::m_Null, TRUE);
			}
			else
			{
				BOOL bCtrlDown = GetAsyncKeyState( VK_CONTROL ) < 0;
				if (//bCtrlDown &&
					(IsPoiSelected(pPoi)) &&
					(m_nUnselectPoiIdOnUp == m_nMovingPoi))
				{
					UnselectPoi(pPoi);
				}
			}
		}
		m_nMovingPoi = 0;
	}


	if ((rMaiUI.nDeltaMouseX || rMaiUI.nDeltaMouseY))
	{
		if (m_bMakingDragRect)
		{
			m_DragRectCur.x = (f32) rMaiUI.nLastMouseX;
			m_DragRectCur.y = (f32) rMaiUI.nLastMouseY;
		}

		if (m_nMovingPoi)
		{
			CGraphPoi* pPoi = maimain_pEditGraph->GetPoi(m_nMovingPoi);

			if (m_bBeginMove)
			{
				 maiundo_Store();	  //starting to move a poi, so save a copy of graph in mem
				 m_bBeginMove = 0;
			}

			if (m_uMoveStyle == MOVE_STYLE_2D)
			{
				if (maimain_bMouseRayImpactThisFrame)
				{
					CFVec3A Delta;
					Delta.Sub(maimain_MouseRayImpactPt, maimain_MouseRayImpactPtPrevious);


					BOOL bUpdateVisInfo = FALSE;

					// if rendering of vols is on
					// recalc the visiblity info
					// in real time. Otherwise, only recalc on mouseup
					if (maimain_pEditGraph->m_bDrawVertVolumes &&
						maimain_pMainToolBarWindow && 
						maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() &&
						!maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_bManModeOn)
					{
						bUpdateVisInfo = TRUE;
					}

					maimain_pEditGraph->MovePoi(m_nMovingPoi, Delta, bUpdateVisInfo);
		
				}

			}
			else if (m_uMoveStyle == MOVE_STYLE_3D)
			{

				CFVec3A PoiLoc;
				pPoi->GetLocation(maimain_pEditGraph, &PoiLoc);
				CFVec3 CamToPoi = PoiLoc.v3 - FXfm_pView->m_MtxR.m_vPos.v3;
				f32 fDist2Plane = CamToPoi.Dot(FXfm_pView->m_MtxR.m_vFront.v3);

				FViewport_t * pView = fviewport_GetActive( );
				if (pView)
				{
					CFVec2 Vec_SS;
					Vec_SS.x = rMaiUI.nLastMouseX * pView->OOHalfRes.x - 1.0f;
					Vec_SS.y = (pView->Res.y - rMaiUI.nLastMouseY) * pView->OOHalfRes.y - 1.0f;

					CFVec3 Vec_VS;
					Vec_VS.x = pView->fTanHalfFOVX*Vec_SS.x*fDist2Plane;
					Vec_VS.y = pView->fTanHalfFOVY*Vec_SS.y*fDist2Plane;
					Vec_VS.z = fDist2Plane;

					CFVec3A tmp;
					tmp.Set(Vec_VS);
					CFVec3A Vec_WS = FXfm_pView->m_MtxR.MulPoint(tmp);

					CFVec3A delta;
					delta.Sub(Vec_WS, PoiLoc);

					maimain_pEditGraph->MovePoi(m_nMovingPoi, delta);
				}
			}
		}
	}


	if (GetAsyncKeyState( 'Z' ) < 0)
	{
	}
	else if (GetAsyncKeyState( 'X' ) < 0)
	{
	}
	else if (GetAsyncKeyState( VK_DELETE ) < 0)
	{	//delete key went down
		maiundo_Store();
		DeleteSelectedPoi();
	}

	if (rMaiUI.bLastRightOnDown)
	{

	}

	if (rMaiUI.bLastRightOnUp)
	{
		CGraphPoi* pPoi = maimain_pEditGraph->FindClosestIntersectingPoi(maimain_MouseRayOrigin, maimain_MouseRayEnd);
		if (pPoi)
		{
			CWnd wnd;
			wnd.Attach(fdx8vid_GetWindowHandle());
			CPoiPropsDlg dlg(pPoi, &wnd);
//			dlg.SetWindowPos(&CWnd::wndTopMost, rMaiUI.nLastMouseX, rMaiUI.nLastMouseY,0,0, SWP_NOSIZE | SWP_NOZORDER );
			if (dlg.DoModal())
			{	//copy stuff back into the poi
				pPoi->m_uFlags = 0;
				pPoi->m_uFlags |= CGraphPoi::POIFLAG_OFFENSIVE_HINT * dlg.m_bOffensivePoi;
				pPoi->m_uFlags |= CGraphPoi::POIFLAG_CUSTOM_LOS * dlg.m_bCustom;

				if (dlg.m_bCustom)
				{
					pPoi->m_uVisInfo = 0;  //clear it, init from dlg 

					pPoi->PackVisRating(0, dlg.m_bN*(dlg.m_nRange+1));
					pPoi->PackVisRating(1, dlg.m_bNE*(dlg.m_nRange+1));
					pPoi->PackVisRating(2, dlg.m_bE*(dlg.m_nRange+1));
					pPoi->PackVisRating(3, dlg.m_bSE*(dlg.m_nRange+1));
					pPoi->PackVisRating(4, dlg.m_bS*(dlg.m_nRange+1));
					pPoi->PackVisRating(5, dlg.m_bSW*(dlg.m_nRange+1));
					pPoi->PackVisRating(6, dlg.m_bW*(dlg.m_nRange+1));
					pPoi->PackVisRating(7, dlg.m_bNW*(dlg.m_nRange+1));
				}
				else
				{
					pPoi->UpdateVisInfo(maimain_pEditGraph);
				}
				
			}

			//make the last right-clicked poi into a mode to be used when adding poi.
			//select different poi to change modes. yich.
			m_bOffensivePoiMode = pPoi->m_uFlags & CGraphPoi::POIFLAG_OFFENSIVE_HINT;

			wnd.Detach();

		}
		else
		{
			ClearSelectedPoi();
		}
	}



}


void CPoiTool::RenderPoi(void)
{
	u16 j;
	for (j = 1; j < maimain_pEditGraph->GetNumPoi(); j++)
	{
		CGraphPoi* pPoi = maimain_pEditGraph->GetPoi(j);
		if (pPoi->m_nPoiSlotStatus != POI_SLOT_FREE)
		{
			CFVec3A loc;
			pPoi->GetLocation(maimain_pEditGraph, &loc);

			u8 uColor = POICOLOR_MAGENTA;

			if (IsPoiSelected(pPoi))
			{
				//draw the scanner lookat rays
				for (u8 i = 0; i < CGraphPoi::NUM_VIS_ARCS; i++)
				{
					CFVec3A RayEnd = CGraphPoi::s_aVisRays[i];
					u8 uRating = pPoi->UnpackVisRating(i);
					if (uRating)
					{
						RayEnd.Mul(CGraphPoi::kafVisRange[uRating-1]);
						RayEnd.Add(loc);
//						u8 auScannerStatToColorMap[4] = {0,1,2,3};
//						uColor = auScannerStatToColorMap[uRating];
						fdraw_SolidLine( &(loc.v3), &(RayEnd.v3), _aPoiColors+POICOLOR_GREEN);
					}
				}
			//	uColor = POICOLOR_GREEN;
			}
			else if (pPoi->m_uFlags & CGraphPoi::POIFLAG_OFFENSIVE_HINT)
			{
			  uColor = POICOLOR_CYAN;
			}
				 
			fdraw_FacetedWireSphere( &(loc.v3), 1.0f, 1, 1, _aPoiColors+uColor);
		}
	}


	if (m_bMakingDragRect)
	{  //2D Draw mode
		// Get current viewport
		FViewport_t *pPreviousVP = fviewport_GetActive();
		u32 nWidth = FViewport_pDefaultOrtho->nWidth;
		u32 nHeight = FViewport_pDefaultOrtho->nHeight;
		
		// Set the performance monitoring viewport and fdraw renderer
		fviewport_SetActive( FViewport_pDefaultOrtho );
		frenderer_Push( FRENDERER_DRAW, NULL );

		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		fdraw_Color_SetFunc( FDRAW_COLORFUNC_ADD );

		//draw line in X,Z

		CFVec3 CornerA;
		CFVec3 CornerB;
		CornerA.x = m_DragRectOrigin.x/(f32) nWidth;
		CornerA.y = m_DragRectOrigin.y/(f32) nHeight;
		CornerA.z = 1.0f;
		CornerB.x = m_DragRectCur.x/(f32) nWidth;
		CornerB.y = m_DragRectCur.y/(f32) nHeight;
		CornerB.z = 1.0f;
//		fdraw_SolidLine( &(CornerA), &(CornerB), _aPoiColors+POICOLOR_GREEN);

		_aoDrawGuidesVertsBuffer[ 0 ].ColorRGBA = FColor_MotifWhite;//_rgb[_aRayDestDebugData[i]];
		_aoDrawGuidesVertsBuffer[ 0 ].Pos_MS = CornerA;
		_aoDrawGuidesVertsBuffer[ 1 ].ColorRGBA = FColor_MotifWhite;
		_aoDrawGuidesVertsBuffer[ 1 ].Pos_MS = CornerB;
		fdraw_Line(_aoDrawGuidesVertsBuffer, _aoDrawGuidesVertsBuffer+1);
  
		frenderer_Pop();
		fviewport_SetActive( pPreviousVP );
	}


}


void CPoiTool::Draw(void)
{
	RenderPoi();
}


