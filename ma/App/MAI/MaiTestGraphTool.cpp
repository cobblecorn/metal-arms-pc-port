#include "stdafx.h"
#include "fang.h"
#include "MaiTestGraphTool.h"
#include "maimain.h"
#include "maiEditGraph.h"
#include "resource.h"
#include "MaiToolbar.h"
#include "ftext.h"
#include "fxfm.h"
#include "floop.h"
#include "fviewport.h"

extern bool StoreImpactPoint(CFVec3 * pStore);

const float kfUseAsTestPointRadius = 2.0f;
static CFVec3 _oldImpactPoint;

static CFVec3 _oldTestPoints[TestGraphTool::NUM_TESTPOINTS];

TestGraphTool::TestGraphTool() : BaseTool()
{
	m_pQueryDetail = NULL;
	m_pQuery = APE_NEW CSearchQuery();
	m_pQueryResults = APE_NEW CSearchResults();


	for (int i = 0; i < NUM_TESTPOINTS; i++)
	{
		m_bTracking[i] = 0;
		m_bSelected[i] = 0;
		m_fCuRad[i] =  fmath_RandomUnitFloatLessThan1();
		m_rgb[i] = FColor_MotifWhite;
		m_TestPoint[i] = _oldTestPoints[i];
	}

	m_rgb[ORIGIN_TESTPOINT] = FColor_MotifBlue;
	m_rgb[GOAL_TESTPOINT].SetColor(1.0f, 1.0f, 0.0f);

}


TestGraphTool::~TestGraphTool()
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->ClearPropFromAllVerts(CEditVert::EV_ONHOTPATH);
	}
	if (maimain_pDebugGraphSearcher)
	{
		maimain_pDebugGraphSearcher->CancelSearch(m_pQuery);
	}
	if (m_pQueryResults)
	{
		m_pQueryResults->Clear();
	}
	if (m_pQuery)
	{
		m_pQuery->Clear();
	}

	for (int i = 0; i < NUM_TESTPOINTS; i++)
	{
		_oldTestPoints[i] = m_TestPoint[i];
	}


	
	APE_DELETE(m_pQuery); m_pQuery = NULL;
	APE_DELETE(m_pQueryResults); m_pQueryResults = NULL;


	m_pQueryDetail = NULL;
}


s32 TestGraphTool::FindClosestIntersectingTestPoint(const CFVec3A& rRayStart, const CFVec3A& rRayEnd)
{
	f32 fMaxDistToCheck = 300.0f;
	s32 nClosestPoint = -1;

	for (int i = 0; i < NUM_TESTPOINTS; i++)
	{
		CFSphere VertSphere(m_TestPoint[i], kfUseAsTestPointRadius);
		CFVec3 HitLoc;
		float fDistFromRayStart;
		if (VertSphere.IsIntersecting( rRayStart.v3, rRayEnd.v3, &HitLoc, &fDistFromRayStart))
		{
			if (fDistFromRayStart < fMaxDistToCheck)
			{
				nClosestPoint = i;
				fMaxDistToCheck = fDistFromRayStart; 
			}
		}
	}

	return nClosestPoint;
}


void TestGraphTool::UserInput(const CMaiUserInput& rMaiUI)
{
	if (rMaiUI.bLastLeftOnDown)
	{
		s32 nHitPoint = FindClosestIntersectingTestPoint(maimain_MouseRayOrigin, maimain_MouseRayEnd);
		if (nHitPoint > -1)
		{
			m_bSelected[nHitPoint] = 1;
			StoreImpactPoint(&_oldImpactPoint);
		}
		else
		{
			for (int i = 0; i < NUM_TESTPOINTS; i++)
			{
				if (m_bTracking[i])
				{
					if (StoreImpactPoint(&m_TestPoint[i]))
					{
						
						m_bTracking[i] = 0;
						int toolIds[2] = {ID_ORIGINBUTTON,  ID_GOALBUTTON};
						
						if (maimain_pMainToolBarWindow && maimain_pMainToolBarWindow->GetCurToolWnd() && maimain_pMainToolBarWindow->GetCurToolWnd()->m_pToolBar)
						{
							int curState = maimain_pMainToolBarWindow->GetCurToolWnd()->m_pToolBar->GetToolBarCtrl().GetState(toolIds[i]);
							curState &= ~ TBSTATE_CHECKED;
							maimain_pMainToolBarWindow->GetCurToolWnd()->m_pToolBar->GetToolBarCtrl().SetState(toolIds[i], curState );
						}
					}
				}
			}
		}
	}

	if (rMaiUI.bLastLeftOnUp)
	{
		for (int i = 0; i < NUM_TESTPOINTS; i++)
		{
			m_bSelected[i] = 0;
		}
	}

	if ((rMaiUI.nDeltaMouseX || rMaiUI.nDeltaMouseY))
	{
		if (!m_pQueryDetail->m_b3DMover)
		{
			CFVec3 newImpactPoint;
			if (StoreImpactPoint(&newImpactPoint))
			{
				for (int i = 0; i < NUM_TESTPOINTS; i++)
				{
					if (m_bSelected[i])
					{
						m_TestPoint[i] += (newImpactPoint - _oldImpactPoint);
					}
				}
				_oldImpactPoint = newImpactPoint;
			}
		}
		else
		{
			for (int i = 0; i < NUM_TESTPOINTS; i++)
			{
				if (m_bSelected[i])
				{

					CFVec3 CamToVert = m_TestPoint[i] - FXfm_pView->m_MtxR.m_vPos.v3;
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

						m_TestPoint[i] = Vec_WS;
					}
				}
			}

		}
	}


	CFVec3 mouseRayDir = maimain_MouseRayEnd.v3 - maimain_MouseRayOrigin.v3;
	mouseRayDir.Unitize();
	if (GetAsyncKeyState( '1' ) < 0)
	{
		for (int i = 0; i < NUM_TESTPOINTS; i++)
		{
			if (m_bSelected[i])
			{
				CFVec3 CamToVert = m_TestPoint[i] - FXfm_pView->m_MtxR.m_vPos.v3;
				f32 fDistCam2Vert = CamToVert.Dot(FXfm_pView->m_MtxR.m_vFront.v3);
				f32 fZoomScale = fDistCam2Vert*0.1f;

				CFVec3A Delta;
				Delta.Set(mouseRayDir);
				Delta.Mul(fZoomScale);
				m_TestPoint[i] +=Delta.v3;
			}
		}
	}
	else if (GetAsyncKeyState( '2' ) < 0)
	{
		for (int i = 0; i < NUM_TESTPOINTS; i++)
		{
			if (m_bSelected[i])
			{
				CFVec3 CamToVert = m_TestPoint[i] - FXfm_pView->m_MtxR.m_vPos.v3;
				f32 fDistCam2Vert = CamToVert.Dot(FXfm_pView->m_MtxR.m_vFront.v3);
				f32 fZoomScale = -fDistCam2Vert*0.1f;

				CFVec3A Delta;
				Delta.Set(mouseRayDir);
				Delta.Mul(fZoomScale);
				m_TestPoint[i] +=Delta.v3;
			}
		}
	}
	
	
	if (m_pQuery->IsDone() && !m_pQuery->HasBeenDone())
	{
		m_pQuery->SetHasBeenDone();

		if (m_pQueryResults->GetVertPath())
		{
			maimain_pEditGraph->ClearPropFromAllVerts(CEditVert::EV_ONHOTPATH);
			
			CFVec3 _lastVertLoc;
			int count = 0;
			CNiIterator<const GraphVert*> it = m_pQueryResults->GetVertPath()->Begin();
			while (it.IsValid())
			{
				const GraphVert *pV = it.Get();
				it.Next();
				maimain_pEditGraph->SetVertProperty(maimain_pEditGraph->GetVertId(pV), CEditVert::EV_ONHOTPATH);
			}
		}
	}

}


void TestGraphTool::OnFocus(void)
{
}


void TestGraphTool::FindPath(void)
{
	if (maimain_pEditGraph && !maimain_pDebugGraphSearcher->IsBusy())
	{
		maimain_pEditGraph->ClearPropFromAllVerts(CEditVert::EV_ONHOTPATH);

		CFVec3A GoalLoc;
		GoalLoc.Set(m_TestPoint[GOAL_TESTPOINT]);
		GoalLoc.y+=0.1f; //pgm. find and fix this please, it is a cheese fix for when test points are slightly in the ground

		CFVec3A StartLoc;
		StartLoc.Set(m_TestPoint[ORIGIN_TESTPOINT]); 
		StartLoc.y+=0.1f;
		float fQueryWidth = 2.0f;
		float fQueryHeight = 3.0f;
		float fQuerySlopeCost = 0.0f;
		u16 uSearchParams = CSearchQuery::SEARCHPARAM_FLOOR | CSearchQuery::SEARCHPARAM_NEED_AIPATH;// | CSearchQuery::SEARCHPARAM_NEED_VERTPATH;

		if (m_pQueryDetail)
		{
			fQueryWidth = m_pQueryDetail->m_fWidth;
			fQueryHeight = m_pQueryDetail->m_fHeight;
			fQuerySlopeCost = m_pQueryDetail->m_fSlopeCost;
			uSearchParams &=~ CSearchQuery::SEARCHPARAM_NO_JUMPS;
			if (!m_pQueryDetail->m_bCanJump)
			{
				uSearchParams |= CSearchQuery::SEARCHPARAM_NO_JUMPS;
			}
			uSearchParams &=~ CSearchQuery::SEARCHPARAM_FLOOR;
			if (m_pQueryDetail->m_bSurfaceMover)
			{
				uSearchParams |= CSearchQuery::SEARCHPARAM_FLOOR;
			}
			else if (m_pQueryDetail->m_b3DMover)
			{
				uSearchParams |= CSearchQuery::SEARCHPARAM_3D;
			}

		}
		
		maimain_pDebugGraphSearcher->CancelSearch(m_pQuery);
		m_pQueryResults->Clear();
		m_pQuery->Clear();
		m_pQuery->SetDefaultSearchParams(	StartLoc,
											GoalLoc,
											fQueryWidth,
											fQueryHeight,
											m_pQueryResults,
											uSearchParams,
											0);
		m_pQuery->SetFudgeDestDist(10.0f);
		m_pQuery->SetSlopeCost(fQuerySlopeCost);
		maimain_pDebugGraphSearcher->SubmitQuery(m_pQuery);
	}
}


void TestGraphTool::Draw(void)
{
	float fQueryWidth = 2.0f;
	float fQueryHeight = 3.0f;
	BOOL b3DTester = FALSE;

	if (m_pQueryDetail)
	{
		fQueryWidth = m_pQueryDetail->m_fWidth;
		fQueryHeight = m_pQueryDetail->m_fHeight;
		b3DTester = m_pQueryDetail->m_b3DMover;
	}


	for (int i = 0; i < NUM_TESTPOINTS; i++)
	{
		if (b3DTester)
		{
			fdraw_FacetedWireSphere( &(m_TestPoint[i]), fQueryWidth, 2, 2 , &m_rgb[i]);
		}
		else
		{
			float fTmp = 1.0f;//(fmath_Sin(m_fCuRad[i])*0.4f+0.6f);
			m_fCuRad[i]+=0.05f;
			fdraw_FacetedCylinder( &(m_TestPoint[i]), &(CFVec3::m_UnitAxisY), fQueryWidth*fTmp, fQueryHeight, &m_rgb[i], 2 );
		}
	}

	if (m_pQuery->HasBeenDone())
	{
		if (m_pQueryResults->GetAIPath())
		{
			CFVec3A LineFrom;
			CFVec3A LineTo;
			CAIPathWalker PathWalker;
			CFColorRGBA _rgb = FColor_MotifWhite;
			_rgb.SetColor(0.0f, 1.0f, 0.0f);

			PathWalker.Init(m_pQueryResults->GetAIPath());
			LineFrom = m_pQuery->GetStartLoc();
			if (PathWalker.GetCurLoc(&LineTo))
			{
				fdraw_SolidLine(&(LineFrom.v3), &(LineTo.v3), &_rgb);
				LineFrom = LineTo;
			}

			if (PathWalker.GetCurLoc(&LineFrom))
			{
				while (PathWalker.Advance())
				{
					if (PathWalker.GetCurLoc(&LineTo))
					{
						static f32 fUnitTime = 0.0f;
						fUnitTime += FLoop_fPreviousLoopSecs;
						if (fUnitTime > 1.0f)
						{
							fUnitTime = 0.0f;
						}
						LineTo.v3.x+=-0.1f+0.2f*fmath_UnitLinearToSCurve( fUnitTime );
						LineTo.v3.y+=-0.1f+0.2f*fmath_UnitLinearToSCurve( fUnitTime );
						LineTo.v3.z+=-0.1f+0.2f*fmath_UnitLinearToSCurve( fUnitTime );
						fdraw_SolidLine(&(LineFrom.v3), &(LineTo.v3), &_rgb);
						LineFrom = LineTo;
					}
				}
			}
		}
		else if (m_pQueryResults->GetVertPath())
		{
			if (m_pQueryResults->GetFlags() & CSearchResults::RESULTSFLAG_VALID_ENTRY_SHORTCUT)
			{
				CFColorRGBA _rgb = FColor_MotifWhite;
				_rgb.SetColor(1.0f, 1.0f, 0.0f);
				fdraw_SolidLine(&(m_pQueryResults->GetEntryShortCut().v3), &(m_pQuery->GetStartLoc().v3), &_rgb);
			}
			else
			{
				if (m_pQueryResults->GetVertPath()->Size())
				{
					const GraphVert* pVert = m_pQueryResults->GetVertPath()->PopHead();
					if (pVert)
					{
						CFColorRGBA _rgb = FColor_MotifWhite;
						_rgb.SetColor(1.0f, 1.0f, 0.0f);
						fdraw_SolidLine(&(pVert->GetLocation().v3), &(m_pQuery->GetStartLoc().v3), &_rgb);
						m_pQueryResults->GetVertPath()->PushHead(pVert);
					}
				}
			}

		
			if (m_pQueryResults->GetFlags() & CSearchResults::RESULTSFLAG_VALID_EXIT_SHORTCUT)
			{
				CFColorRGBA _rgb = FColor_MotifWhite;
				_rgb.SetColor(1.0f, 1.0f, 0.0f);
				fdraw_SolidLine(&(m_pQueryResults->GetExitShortCut().v3), &(m_pQuery->GetGoalLoc().v3), &_rgb);
			}
			else
			{
				const GraphVert* pVert = m_pQueryResults->GetVertPath()->PopTail();
				if (pVert)
				{
					CFColorRGBA _rgb = FColor_MotifWhite;
					_rgb.SetColor(1.0f, 1.0f, 0.0f);
					fdraw_SolidLine(&(pVert->GetLocation().v3), &(m_pQuery->GetGoalLoc().v3), &_rgb);
					m_pQueryResults->GetVertPath()->PushTail(pVert);
				}
			}
		}
	}

	if (m_pQuery->HasBeenDone())
	{
/*
		ftext_DebugPrintf( 0.99f, 0.235f, "~arTICKS: ~w1%f", m_pQueryResults->m_fCookTime);
		ftext_DebugPrintf( 0.99f, 0.250f, "~arFINIT: ~w1%f", m_pQueryResults->m_fFiniTime);
		ftext_DebugPrintf( 0.99f, 0.265f, "~arLOOPS: ~w1%6u", m_pQueryResults->m_nCookLoops);
*/
	}

}


