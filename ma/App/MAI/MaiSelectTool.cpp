//////////////////////////////////////////////////////////////////////////////////////
// MAISelectTool.h - Tool for selecting verts (part of the graph editor)
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
#include "MaiSelectTool.h"
#include "MaiEditGraph.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "MaiUndo.h"
#include "MaiToolbar.h"
#include "fdraw.h"
#include "floop.h"
#include "resource.h"
#include "EdgePropsDlg.h"
#include "VertPropsDlg.h"
#include "dx\fdx8vid.h"
#include "fxfm.h"
#include "fvis.h"
#include "ftext.h"
#include "maimain.h"
#include "AI\AIGraph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CFVec3 _oldImpactPoint;
static u32 _nMarkMouseX;
static u32 _nMarkMouseY;
static CFXfm _Camera;

//vert positions can be zoomed in or in 3d movement mode
SHORT _nVertZoomInKeyState_Old;
SHORT _nVertZoomOutKeyState_Old;
CFVec3A _VertZoomVec;
GraphVert *_pZoomBasisVert= NULL;


SelectTool::SelectTool(void)
: BaseTool(),
  m_bMakingDragRect(FALSE),
  m_uMovingVerts(0),
  m_bBeginMove(0),
  m_uMoveStyle(MOVE_STYLE_2D),
  m_uLastVertClicked(0)
{
	_nVertZoomInKeyState_Old = 0;
	_nVertZoomOutKeyState_Old = 0;
}


bool StoreImpactPoint(CFVec3 * pStore)
{
	FCollImpact_t oCollInfo;

	if (fworld_FindClosestImpactPointToRayStart(&oCollInfo, &maimain_MouseRayOrigin, &maimain_MouseRayEnd))
	{
		CFVec3 BumpLoc = oCollInfo.ImpactPoint.v3+(oCollInfo.UnitFaceNormal.v3*aigraph_kfSurfaceOffset);
		*pStore = BumpLoc;
		return true;
	}
	return false;
}

static int x=0;
static s32 m_nClearOnUp = -1;

void _BeginVertZoom(GraphVert* pV)
{
 	maiundo_Store();
	maimain_pEditGraph->RecordSelectedOffsets(pV);

	_VertZoomVec.Sub(pV->GetLocation(), FXfm_pView->m_MtxR.m_vPos);
	if (_VertZoomVec.SafeUnitAndMag(_VertZoomVec) > 0.0f)
	{
		_pZoomBasisVert = pV;
	}
	else
	{
		_pZoomBasisVert = NULL;
	}
}


void SelectTool::OnFocus(void)
{
	m_uMovingVerts = 0;
	m_bMakingDragRect = 0;
	m_bBeginMove = 0;
	m_nClearOnUp = -1;
	_pZoomBasisVert = NULL;
}


void SelectTool::UserInput(const CMaiUserInput& rMaiUI)
{
	BOOL b3DMode = (maimain_pMainToolBarWindow &&
					maimain_pMainToolBarWindow->GetMode() == CMainToolsWnd::TOOLMODE_EDITGRAPH3D);

	SHORT nVertZoomInKeyState = GetAsyncKeyState( '1' );
	SHORT nVertZoomOutKeyState = GetAsyncKeyState( '2' );
	GraphVert* pV;
	CFVec3A Delta;
	CFVec3A Origin;
	
	if (rMaiUI.bLastLeftOnDown)
	{
		m_nClearOnUp = -1;
		m_bBeginMove = 1;
		m_uMovingVerts = 0;

		if (!b3DMode)
		{
			pV = maimain_pEditGraph->FindClosestIntersectingVert2D(maimain_MouseRayOrigin, maimain_MouseRayEnd, maimain_pEditGraph->m_fVertVisualRadius);
		}
		else
		{
			pV = maimain_pEditGraph->FindClosestIntersectingVert3D(maimain_MouseRayOrigin, maimain_MouseRayEnd, maimain_pEditGraph->m_fVertVisualRadius);
		}
		
		if (pV)
		{
			m_uLastVertClicked = maimain_pEditGraph->GetVertId(pV);

			//toggle Vert selection status
			if (maimain_pEditGraph->IsSelected(pV))
			{
				m_nClearOnUp = maimain_pEditGraph->GetVertId(pV);
			}
			else
			{
				if (!( GetAsyncKeyState( VK_CONTROL ) < 0  || GetAsyncKeyState( VK_SHIFT ) < 0))
				{
					maimain_pEditGraph->ClearSelected();
				}
				maimain_pEditGraph->SelectVert(pV);
			}
			FASSERT(m_uMovingVerts == 0);
			m_uMovingVerts = maimain_pEditGraph->GetVertId(pV);
			StoreImpactPoint(&_oldImpactPoint);	 //initialize this now
		}
		else
		{
			//begin drag rectangle
			m_DragRectOrigin.x = (f32) rMaiUI.nLastMouseX;
			m_DragRectOrigin.y = (f32) rMaiUI.nLastMouseY;
			m_bMakingDragRect = 1;
		}
	}

	
	if (rMaiUI.bLastLeftOnUp)
	{
		if (m_uMovingVerts)
		{  // mouse originally went down on a vert

			pV = maimain_pEditGraph->GetVert((u16)m_uMovingVerts);
			FASSERT(pV);

			if (m_bBeginMove == 0)
			{ //m_bBeginMove == 0 means that the mouse moved while it was down.

				// autodimension 2d is slow
				// so, wait til the up-click to recalculate
				BOOL bAutoSetDimensions2D = FALSE;
				if (maimain_pMainToolBarWindow && 
					maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() &&
					!maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_bManModeOn)
				{
					bAutoSetDimensions2D = TRUE;
				}


				//
				// Vert Vol
				//
				if (bAutoSetDimensions2D)
				{
					pV->AutoSetDimension2D();
				}
				
				//
				// Edge Vols
				//	 Note! even if bAutoSetDimension is FALSE
				//         still go through the edges and adjust length and slope params!

				for (int e = 0; e < pV->GetNumEdges(); e++)
				{
					GraphVert* pVA = pV;
					GraphVert* pVB = pVA->GetOppVert(e, maimain_pEditGraph);
					GraphEdge *pEAB = pVA->GetEdge(e);
					GraphEdge *pEBA = pEAB->GetReverseEdge(maimain_pEditGraph);
					//the max initial edge with is the radius of the smallest connecting vert
					f32 fMaxInitialEdgeWidth = pVA->GetRad();
					if (fMaxInitialEdgeWidth > pVB->GetRad())
					{
						fMaxInitialEdgeWidth  = pVB->GetRad();
					}
					//the max initial edge with is the radius of the smalled connecting vert
					f32 fMaxInitialEdgeHeight = pVA->m_fHeightClearance;
					if (fMaxInitialEdgeHeight > pVB->m_fHeightClearance)
					{
						fMaxInitialEdgeHeight  = pVB->m_fHeightClearance;
					}

					if (bAutoSetDimensions2D)
					{
						pEAB->AutoSetDimension2D( pVA,
												pVB,
												(u16) m_uMovingVerts,
												CFVec3A::m_UnitAxisY,
												fMaxInitialEdgeWidth);
						//fixup reverse edge
						pEBA->AutoSetDimension2D( pVB, pVA, maimain_pEditGraph->GetVertId(pVB), CFVec3A::m_UnitAxisY, pEAB->m_fHalfWidth);//use width of ab edge as max for ba edge
					}
					else
					{
						f32 fLengthSq = pVA->GetLocation().DistSq(pVB->GetLocation());
						f32 fLength = fmath_AcuSqrt(fLengthSq);
						f32 fDY = pVB->GetLocation().y - pVA->GetLocation().y;
						f32 fMaxPosHeightDelta = FMATH_FABS(fDY);
						pEAB->ChangeDimension(fLength, pEAB->m_fHalfWidth, pEAB->m_fMaxSafeHeight, fMaxPosHeightDelta);
						fDY = pVA->GetLocation().y - pVB->GetLocation().y;
						fMaxPosHeightDelta = FMATH_FABS(fDY);
						pEBA->ChangeDimension(fLength, pEBA->m_fHalfWidth, pEBA->m_fMaxSafeHeight, fMaxPosHeightDelta);
					}
				}
			}
			else
			{
				BOOL bCtrlDown = (GetAsyncKeyState( VK_CONTROL ) < 0 || GetAsyncKeyState( VK_SHIFT ) < 0);
				if (bCtrlDown &&
					(maimain_pEditGraph->m_SelectedVerts.Size() > 0 ) &&
					(m_nClearOnUp == maimain_pEditGraph->GetVertId(pV)))
				{
					maimain_pEditGraph->UnSelectVert(pV);
				}
			}
		}
		else
		{  	//mouse down was not on a vert
		
			if (m_bBeginMove == 1)
			{   //mouse did not move while it was down

				//clear current selection
				if (!( GetAsyncKeyState( VK_CONTROL ) < 0 || (GetAsyncKeyState( VK_SHIFT ) < 0)))
				{
					maimain_pEditGraph->ClearSelected();
					m_uMovingVerts = 0;
				}
			}
		}
		
		m_uMovingVerts = 0;
		m_bMakingDragRect = 0;
		m_bBeginMove = 0;
	}


	if ((rMaiUI.nDeltaMouseX || rMaiUI.nDeltaMouseY))
	{

		//
		//   UI when there are one or more verts selected
		//
		//move current vert based on mouse movement
		if (m_uMovingVerts && !_pZoomBasisVert)
		{
			GraphVert* pV = maimain_pEditGraph->GetVert(m_uMovingVerts);

			if (m_bBeginMove)
			{
				 maiundo_Store();
				 maimain_pEditGraph->RecordSelectedOffsets(pV);
				 m_bBeginMove = 0;
			}

			if (m_uMoveStyle == MOVE_STYLE_2D)
			{
				CFVec3 newImpactPoint; 
				if (StoreImpactPoint(&newImpactPoint))
				{
					CFVec3 vertDeltaPos = newImpactPoint - _oldImpactPoint;

					_oldImpactPoint = newImpactPoint;

					Delta.Set(newImpactPoint);
					Delta.Sub(pV->GetLocation());

					maimain_pEditGraph->MoveVert(m_uMovingVerts, Delta);
		
					// in real time, 
					// update all the auto-dimension and walkabiliy flags in the moved vert
					// if rendering of vols is on
					// otherwise, only recalc on mouseup
					if (maimain_pEditGraph->m_bDrawVertVolumes &&
						maimain_pMainToolBarWindow && 
						maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() &&
						!maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->m_bManModeOn)
					{
						pV->AutoSetDimension2D();
					}

					//NOTE! Edges will be re-dimensioned when the mouse come up! not while dragging verts
				}
			}
			else if (m_uMoveStyle == MOVE_STYLE_3D)
			{

				CFVec3 CamToVert = pV->GetLocation().v3 - FXfm_pView->m_MtxR.m_vPos.v3;
				f32 fDist2Plane = CamToVert.Dot(FXfm_pView->m_MtxR.m_vFront.v3);

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
					CFVec3 Vec_WS = FXfm_pView->m_MtxR.MulPoint(tmp).v3;

					Delta.Set(Vec_WS);
					Delta.Sub(pV->GetLocation());
					maimain_pEditGraph->MoveVert(m_uMovingVerts, Delta);
				}
	
				maimain_pEditGraph->ApplySelectedOffsets(pV);   //think this moves all selected verts by the amount pV just moved.
			}
		}
		else if (m_bMakingDragRect)
		{
			m_DragRectCur.x = (f32) rMaiUI.nLastMouseX;
			m_DragRectCur.y = (f32) rMaiUI.nLastMouseY;
			m_bBeginMove = 0;

			FViewport_t * pView = fviewport_GetActive();
			f32 fUx = m_DragRectOrigin.x/(pView->HalfRes.x*2.0f);
			f32 fUy = m_DragRectOrigin.y/(pView->HalfRes.y*2.0f);
			f32 fBx = m_DragRectCur.x/(pView->HalfRes.x*2.0f);
			f32 fBy = m_DragRectCur.y/(pView->HalfRes.y*2.0f);

//			ftext_DebugPrintf( 0.9f, 0.470f, "~arDRO: ~w1%f,~w1%f", fUx, fUy);
//			ftext_DebugPrintf( 0.9f, 0.500f, "~arDRC: ~w1%f,~w1%f", fBx, fBy);

			for (u32 i = 0; i < maimain_pEditGraph->GetNumVerts();i++)
			{
				GraphVert* pV = maimain_pEditGraph->GetVert(i);
				if (!maimain_pEditGraph->IsVertValid(i))
				{
					continue;
				}
				if (!( (b3DMode && pV->Is3D()) || (!b3DMode && pV->Is2D()) ))
				{
					continue;
				}

				if (pV && !maimain_pEditGraph->IsSelected(pV))
				{
					CFVec3A ScreenPos;
					ScreenPos.z = -1.0f;
					f32 fLineHeight = 0.02f;

					CFVec3A Point_VS;
					Point_VS = pV->GetLocation();
					FXfm_pView->m_MtxF.MulPoint(Point_VS);
					BOOL bInView = FALSE;
					if (Point_VS.z > 0.0f)
					{
						CFVec3A loc = pV->GetLocation();

						fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &(FXfm_pView->m_MtxF), &loc, &ScreenPos);
						if (ScreenPos.z >0.0f && ScreenPos.x < 1.0f && ScreenPos.x > -1.0f && ScreenPos.y < 1.0f && ScreenPos.y > -1.0f)
						{
							ScreenPos.x = 0.5f+0.5f*ScreenPos.x;
							ScreenPos.y = (0.375f -ScreenPos.y*0.375f);
							bInView = TRUE;
						}
					}

					if (bInView)
					{
//						ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~arDRC:~w1%d", i);
						if (ScreenPos.x > fUx && ScreenPos.x < fBx && ScreenPos.y > fUy && ScreenPos.y < fBy)
						{

							maimain_pEditGraph->SelectVert(pV);
						}
					}
				}
			}
		}
	}


	// move selected verts in/out mouse ray
	if (maimain_pEditGraph->GetNumSelected())
	{
		if (m_uMovingVerts)
		{
			if (m_uMoveStyle == MOVE_STYLE_3D)	 //(MOVE_STYLE_3D can be b3DMode or not)
			{
				pV = maimain_pEditGraph->GetVert(m_uMovingVerts);
				FASSERT(pV);

				///Move verts In and Out along  "the vertzoom axis"
				if (nVertZoomInKeyState < 0)
				{
					if (_nVertZoomInKeyState_Old >= 0)
					{	//on keydown
						_BeginVertZoom(pV);
					}
					if (_pZoomBasisVert)
					{
						Delta.Sub(_pZoomBasisVert->GetLocation().v3,FXfm_pView->m_MtxR.m_vPos.v3);
						f32 fZoomVertDist2Cam = Delta.Dot(FXfm_pView->m_MtxR.m_vFront);
						Delta = _VertZoomVec;
						Delta.Mul(fZoomVertDist2Cam*0.1f);
						
						maimain_pEditGraph->MoveVert(maimain_pEditGraph->GetVertId(pV), Delta);
						//findfix: should autodimension now
						maimain_pEditGraph->ApplySelectedOffsets(pV);		 //think this moves all selected verts by the amount pV just moved.
					}
				}
				else if (nVertZoomOutKeyState < 0)
				{
					if (_nVertZoomOutKeyState_Old >=0)
					{
						_BeginVertZoom(pV);
					}

					if (_pZoomBasisVert)
					{
						Delta.Sub(_pZoomBasisVert->GetLocation().v3,FXfm_pView->m_MtxR.m_vPos.v3);
						f32 ftmp = Delta.Dot(FXfm_pView->m_MtxR.m_vFront);
						Delta = _VertZoomVec;
						Delta.Mul(-ftmp*0.1f);


						maimain_pEditGraph->MoveVert(maimain_pEditGraph->GetVertId(pV), Delta);
						//findfix: should autodimension now
						maimain_pEditGraph->ApplySelectedOffsets(pV);		 //think this moves all selected verts by the amount pV just moved.
					}
				}
				else
				{
					if (_pZoomBasisVert)
					{
						_pZoomBasisVert = NULL;
						m_bBeginMove = 1;   //incase a drag is performed
					}
				}
			}
		}

		//rotate all selected points about the current point that the mouse is down on
		if (GetAsyncKeyState( '3' ) < 0 || GetAsyncKeyState( '4' ) < 0)
		{

			s32 nOriginVert = 0;
			if (m_uMovingVerts)
			{
				nOriginVert = m_uMovingVerts;
			}
			else if (m_uLastVertClicked &&
					 maimain_pEditGraph->IsVertValid(m_uLastVertClicked) &&
					 ( (b3DMode && maimain_pEditGraph->GetVert(m_uLastVertClicked)->Is3D()) || (!b3DMode && maimain_pEditGraph->GetVert(m_uLastVertClicked)->Is2D()) ) &&
					 maimain_pEditGraph->IsSelected(maimain_pEditGraph->GetVert(m_uLastVertClicked)))
			{
				nOriginVert = m_uLastVertClicked;
			}

			if (nOriginVert)
			{
				Origin = maimain_pEditGraph->GetVert(nOriginVert)->GetLocation();
				f32 fDir = 1.0f;
				f32 fRevsPerSec = 0.5f;
				if (GetAsyncKeyState( '3' ) < 0)
				{
					fDir = -1.0f;
				}

				for (u32 i = 0; i < maimain_pEditGraph->GetNumVerts();i++)
				{
					GraphVert* pV = maimain_pEditGraph->GetVert(i);
					if (!maimain_pEditGraph->IsVertValid(i))
					{
						continue;
					}
					if (pV && maimain_pEditGraph->IsSelected(pV))
					{
						Delta.Sub(pV->GetLocation(), Origin);
						Delta.RotateY(fDir*fRevsPerSec*FLoop_fPreviousLoopSecs);
						Delta.Add(Origin);
						pV->m_Location = Delta;
					}
				}
			}
		}


		if (m_uMovingVerts && GetAsyncKeyState('6') < 0)
		{	//move selected 2D verts to the nearest surface
			u16 uSelected = m_uMovingVerts;
			GraphVert* pV = maimain_pEditGraph->GetVert(uSelected);

			if (pV && ( (!b3DMode && pV->Is2D()) ))
			{
				if (pV && maimain_pEditGraph->IsSelected(pV))
				{
					maiundo_Store();
					maimain_pEditGraph->RecordSelectedOffsets(pV);
					pV->MoveToNearestSurface(CFVec3::m_UnitAxisY * -1.0f);
				}
				maimain_pEditGraph->ApplySelectedOffsets(pV);
			}
		}


		if (GetAsyncKeyState('7') < 0)
		{	//move selected 2D verts to the nearest surface
			BOOL bDidOne = FALSE;
			for (u32 i = 0; i < maimain_pEditGraph->GetNumVerts();i++)
			{
				pV = maimain_pEditGraph->GetVert(i);
				if (!maimain_pEditGraph->IsVertValid(i))
				{
					continue;
				}
				if (!( (!b3DMode && pV->Is2D()) ))
				{
					continue;
				}
				if (pV && maimain_pEditGraph->IsSelected(pV))
				{
					if (!bDidOne)
					{
						maiundo_Store();

						bDidOne = TRUE;
					}
					pV->MoveToNearestSurface(CFVec3::m_UnitAxisY * -1.0f);
				}
			}
	
		}

		//Move all selected verts to the world origin
		if (GetAsyncKeyState( '8' ) < 0)
		{
			s32 nOriginVert = 0;
			if (m_uMovingVerts)
			{
				nOriginVert = m_uMovingVerts;
			}
			else if (m_uLastVertClicked &&
					 maimain_pEditGraph->IsVertValid(m_uLastVertClicked) &&
					 ( (b3DMode && maimain_pEditGraph->GetVert(m_uLastVertClicked)->Is3D()) || (!b3DMode && maimain_pEditGraph->GetVert(m_uLastVertClicked)->Is2D()) ) &&
					 maimain_pEditGraph->IsSelected(maimain_pEditGraph->GetVert(m_uLastVertClicked)))
			{
				nOriginVert = m_uLastVertClicked;
			}

			if (nOriginVert)
			{

				CFVec3A Origin;
				Origin = maimain_pEditGraph->GetVert(nOriginVert)->GetLocation();
 				maiundo_Store();

				for (u32 i = 0; i < maimain_pEditGraph->GetNumVerts();i++)
				{
					pV = maimain_pEditGraph->GetVert(i);
					if (!maimain_pEditGraph->IsVertValid(i))
					{
						continue;
					}
					if (!( (b3DMode && pV->Is3D()) || (!b3DMode && pV->Is2D()) ))
					{
						continue;
					}
					if (pV && maimain_pEditGraph->IsSelected(pV))
					{
						Delta.Sub(Origin, pV->GetLocation());
						pV->m_Location = Delta;
					}
				}
			}
		}
		
		//Move all selected verts to the world origin
		s32 nHeightAdjust = (GetAsyncKeyState( 'Y' ) < 0);
		u32 uSlamHeightsToVert = 0;
		if (!nHeightAdjust)
		{
			nHeightAdjust = -1*(GetAsyncKeyState( 'H' ) < 0);
		}
		
		if (!nHeightAdjust)
		{
			if (GetAsyncKeyState( '9' ) < 0)
			{
				if (m_uMovingVerts)
				{
					uSlamHeightsToVert = m_uMovingVerts;
				}
				else if (m_uLastVertClicked &&
						maimain_pEditGraph->IsVertValid(m_uLastVertClicked) &&
						( (b3DMode && maimain_pEditGraph->GetVert(m_uLastVertClicked)->Is3D()) || (!b3DMode && maimain_pEditGraph->GetVert(m_uLastVertClicked)->Is2D()) ) &&
						maimain_pEditGraph->IsSelected(maimain_pEditGraph->GetVert(m_uLastVertClicked)))
				{
					uSlamHeightsToVert = m_uLastVertClicked;
				}
			}
		}

		if (nHeightAdjust || uSlamHeightsToVert )
		{
			BOOL bDidOne = FALSE;
			for (u32 i = 0; i < maimain_pEditGraph->GetNumVerts();i++)
			{
				pV = maimain_pEditGraph->GetVert(i);
				if (!maimain_pEditGraph->IsVertValid(i))
				{
					continue;
				}
				if (!( (!b3DMode && pV->Is2D()) ))
				{
					continue;
				}
				if (pV && maimain_pEditGraph->IsSelected(pV))
				{
					if (!bDidOne)
					{
						maiundo_Store();

						bDidOne = TRUE;
					}
					pV->m_nProps |= VERTPROP_CUSTOM_VOL;
					if (uSlamHeightsToVert)
					{
						pV->m_fHeightClearance = maimain_pEditGraph->GetVert(uSlamHeightsToVert)->GetHeight();
					}
					else
					{
						pV->m_fHeightClearance += 2.0f*FLoop_fPreviousLoopSecs*((f32)nHeightAdjust);
					}

					for (u32 uu = 0; uu < pV->GetNumEdges(); uu++)
					{
						GraphVert* pVB = pV->GetOppVert(uu, maimain_pEditGraph);
						if (maimain_pEditGraph->IsSelected(pVB))
						{
							pV->GetEdge(uu)->m_nProps |= EDGEPROP_CUSTOM_VOL;
							if (uSlamHeightsToVert)
							{
								pV->GetEdge(uu)->m_fMaxSafeHeight = maimain_pEditGraph->GetVert(uSlamHeightsToVert)->GetHeight();
							}
							else
							{
								pV->GetEdge(uu)->m_fMaxSafeHeight += 2.0f*FLoop_fPreviousLoopSecs*((f32)nHeightAdjust);
							}
						}
					}
				}
			}
		}

		if (GetAsyncKeyState( VK_DELETE ) < 0)
		{	//delete key went down. remove points in current selection list
			maiundo_Store();
			maimain_pEditGraph->DeleteSelected();
			m_uMovingVerts = 0;
			m_bMakingDragRect = 0;
			m_uLastVertClicked = 0;
		}
	}

	//select all verts
	if (GetAsyncKeyState( '5' ) < 0)
	{
		for (u32 i = 0; i < maimain_pEditGraph->GetNumVerts();i++)
		{
			pV = maimain_pEditGraph->GetVert(i);
			if (!maimain_pEditGraph->IsVertValid(i))
			{
				continue;
			}
			if (!( (b3DMode && pV->Is3D()) || (!b3DMode && pV->Is2D()) ))
			{
				continue;
			}
			maimain_pEditGraph->SelectVert(pV);
		}
	}


	//
	// right clicks customize edges and verts
	//
	maiselectTool_DoRightClickCustomizeUI(rMaiUI, (m_uMoveStyle & MOVE_STYLE_3D)!=0);

	_nVertZoomInKeyState_Old = nVertZoomInKeyState;
	_nVertZoomOutKeyState_Old = nVertZoomOutKeyState;
}

static FDrawVtx_t _aoDrawGuidesVertsBuffer[ 2 ];

void SelectTool::Draw(void)
{
#if 0
/*	CFColorRGBA _rgb = FColor_MotifWhite;
	if (bHitEdge)
	{
		_rgb.SetColor(1.0f, 1.0f, 0.0f);
	}

	CFVec3 P1;
	CFVec3 P2;
	P1.Set(EdgeSeg.Origin().x, EdgeSeg.Origin().y, EdgeSeg.Origin().z);
	P2.Set(EdgeSeg.Direction().x, EdgeSeg.Direction().y, EdgeSeg.Direction().z);
	fdraw_SolidLine(&(P1), &(P2), &_rgb);
*/


	if (m_bMakingDragRect)
	{
	//	ftext_DebugPrintf( 0.9f, 0.370f, "~arDRO: ~w1%f,~w1%f", m_DragRectOrigin.x, m_DragRectOrigin.y);
	//	ftext_DebugPrintf( 0.9f, 0.400f, "~arDRC: ~w1%f,~w1%f", m_DragRectCur.x, m_DragRectCur.y);
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
	
//		_Camera.Identity();
//		_Camera.InitStackWithView();

		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );

		//draw line in X,Z

		CFVec3 CornerA;
		CFVec3 CornerB;
		CornerA.x = m_DragRectOrigin.x/(f32) nWidth;
		CornerA.y = 1.0f - m_DragRectOrigin.y/(f32) nHeight;
		CornerA.z = 1.0f;
		CornerB.x = m_DragRectCur.x/(f32) nWidth;
		CornerB.y = 1.0f - m_DragRectCur.y/(f32) nHeight;
		CornerB.z = 1.0f;
		fdraw_SolidLine( &(CornerA), &(CornerB), &FColor_MotifGreen);

		_aoDrawGuidesVertsBuffer[ 0 ].ColorRGBA = FColor_MotifWhite;//_rgb[_aRayDestDebugData[i]];
		_aoDrawGuidesVertsBuffer[ 0 ].Pos_MS = CornerA;
		_aoDrawGuidesVertsBuffer[ 1 ].ColorRGBA = FColor_MotifWhite;
		_aoDrawGuidesVertsBuffer[ 1 ].Pos_MS = CornerB;
		fdraw_Line(_aoDrawGuidesVertsBuffer, _aoDrawGuidesVertsBuffer+1);

		frenderer_Pop();
		fviewport_SetActive( pPreviousVP );
	}
#endif
}


void maiselectTool_DoRightClickCustomizeUI(const CMaiUserInput& rMaiUI, BOOL b3DMode)
{
	if (rMaiUI.bLastRightOnDown)
	{
		_nMarkMouseX = rMaiUI.nLastMouseX;
		_nMarkMouseY = rMaiUI.nLastMouseY;
	}

	if (rMaiUI.bLastRightOnUp && _nMarkMouseX == rMaiUI.nLastMouseX && _nMarkMouseY == rMaiUI.nLastMouseY)
	{
		GraphVert* pV;
		if (!b3DMode)
		{
			pV = maimain_pEditGraph->FindClosestIntersectingVert2D(maimain_MouseRayOrigin, maimain_MouseRayEnd, maimain_pEditGraph->m_fVertVisualRadius);
		}
		else
		{
			pV = maimain_pEditGraph->FindClosestIntersectingVert3D(maimain_MouseRayOrigin, maimain_MouseRayEnd, maimain_pEditGraph->m_fVertVisualRadius);
		}

		if (pV && ((b3DMode && pV->Is3D()) || (!b3DMode && pV->Is2D()) ))
		{
			CWnd wnd;
			wnd.Attach(fdx8vid_GetWindowHandle());
			CVertPropsDlg dlg(pV, &wnd);
//			dlg.SetWindowPos(&CWnd::wndTopMost, rMaiUI.nLastMouseX, rMaiUI.nLastMouseY,0,0, SWP_NOSIZE | SWP_NOZORDER );
			if (dlg.DoModal())
			{	//copy stuff back into the vert
				pV->m_nProps = (pV->m_nProps & ~VERTPROP_CUSTOM_VOL) | (VERTPROP_CUSTOM_VOL*( dlg.m_bCustom != 0));
			}

			if (pV->m_nProps & VERTPROP_CUSTOM_VOL)
			{
				pV->m_fHeightClearance = dlg.m_fCustomHeight;
				pV->m_fSafeRad = dlg.m_fCustomRadius;
				//vert and edges will be updated
				maimain_pEditGraph->AutoSetDimension2D(maimain_pEditGraph->GetVertId(pV));
			}
			else if (dlg.NeedToRecalcVol())
			{
				//vert and edges will be updated
				maimain_pEditGraph->AutoSetDimension2D(maimain_pEditGraph->GetVertId(pV));
			}

			wnd.Detach();
		}
		else
		{
			GraphEdge* pEdge;
			int nVertId = maimain_pEditGraph->FindClosestEdge(maimain_MouseRayOrigin.v3, maimain_MouseRayEnd.v3-maimain_MouseRayOrigin.v3, 1.0f, &pEdge);
			if (nVertId)
			{
				if (pEdge && ((b3DMode && pEdge->Is3D()) || (!b3DMode && pEdge->Is2D()) ))
				{
					CWnd wnd;
					wnd.Attach(fdx8vid_GetWindowHandle());
					CEdgePropsDlg dlg(nVertId, pEdge, &wnd);
//					dlg.SetWindowPos(&CWnd::wndTopMost, rMaiUI.nLastMouseX, rMaiUI.nLastMouseY,0,0, SWP_NOSIZE | SWP_NOZORDER );
					if (dlg.DoModal())
					{	//copy edgestuff from the dialog back into the edge
						pEdge->m_nProps = (pEdge->m_nProps & ~EDGEPROP_HAZARD_DOOR) | (EDGEPROP_HAZARD_DOOR*( dlg.m_bHazardDoor != 0));
						pEdge->m_nProps = (pEdge->m_nProps & ~EDGEPROP_HAZARD_LIFT) | (EDGEPROP_HAZARD_LIFT*( dlg.m_bHazardLift != 0));
						pEdge->m_nProps = (pEdge->m_nProps & ~EDGEPROP_CUSTOM_VOL) | (EDGEPROP_CUSTOM_VOL*( dlg.m_bCustom != 0));
						pEdge->m_nProps = (pEdge->m_nProps & ~EDGEPROP_DOWN_ONLY) | (EDGEPROP_DOWN_ONLY*( dlg.m_bDownOnly != 0));
						pEdge->m_nProps = (pEdge->m_nProps & ~EDGEPROP_UP_ONLY) | (EDGEPROP_UP_ONLY*( dlg.m_bUpOnly != 0));
						pEdge->m_nProps = (pEdge->m_nProps & ~EDGEPROP_HAZARD_DESTRUCTABLE) | (EDGEPROP_HAZARD_DESTRUCTABLE*( dlg.m_bHazardDestructable != 0));
						pEdge->m_nProps = (pEdge->m_nProps & ~EDGEPROP_HAZARD_BRIDGE) | (EDGEPROP_HAZARD_BRIDGE*( dlg.m_bHazardBridge != 0));

						pEdge->m_nProps &= ~EDGEPROP_JUMP;
						pEdge->m_nProps |= dlg.m_uJumpType;

						pEdge->m_nHazardId = dlg.m_nHazardId;

						if (pEdge->m_nProps & EDGEPROP_CUSTOM_VOL)
						{	//override width and height
							pEdge->m_fMaxSafeHeight = dlg.m_fCustomHeight;
							pEdge->m_fHalfWidth = dlg.m_fCustomWidth;
						}
						else if (dlg.NeedToRecalcVol())
						{
							//autocalc
							GraphVert* pVA = maimain_pEditGraph->GetVert(nVertId);
							GraphVert* pVB = maimain_pEditGraph->GetVert(pEdge->m_nNextVertId);
							f32 fMaxInitialEdgeWidth = pVA->GetRad();
							if (fMaxInitialEdgeWidth > pVB->GetRad())
							{
								fMaxInitialEdgeWidth  = pVB->GetRad();
							}
							fMaxInitialEdgeWidth = fMaxInitialEdgeWidth;
							pEdge->AutoSetDimension2D(pVA, pVB, nVertId, CFVec3A::m_UnitAxisY, fMaxInitialEdgeWidth);
						}

						//fixup the reverse edge  (o.k not to autosetdimensioni on reverse, because we know the verts didn't move!
						GraphEdge* pReverseEdge = pEdge->GetReverseEdge(maimain_pEditGraph);
						if (pReverseEdge)
						{
							pReverseEdge->m_nProps = pEdge->m_nProps;
							pReverseEdge->m_nHazardId = pEdge->m_nHazardId;
							pReverseEdge->m_fMaxSafeHeight = pEdge->m_fMaxSafeHeight;
							pReverseEdge->m_fHalfWidth = pEdge->m_fHalfWidth;
						}
					
					}
					wnd.Detach();
				}
			}
		}
	}
}