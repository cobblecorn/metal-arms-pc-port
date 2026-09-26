// MaiToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "k9.h"
#include "MaiToolBar.h"
#include "MaiSelectTool.h"
#include "MaiAddTool.h"
#include "MaiJoinTool.h"
#include "MaiPoiTool.h"
#include "maimain.h"
#include "MaiEditGraph.h"
#include "MaiAddTool3D.h"
#include "MaiUndo.h"
#include "QueryDetailsDlg.h"
#include "ftext.h"


/////////////////////////////////////////////////////////////////////////////
//
//
//	 CMaiToolBarWnd
//
//
//

CMaiToolBarWnd::CMaiToolBarWnd()
: m_nToolBarResourceId(-1),
  m_pToolBar(NULL),
  m_paOnDownInputButtons(NULL),
  m_paInputButtonFuncs(NULL)
{
}


CMaiToolBarWnd::~CMaiToolBarWnd()
{
	delete m_pToolBar; m_pToolBar = NULL;
	delete [] m_paOnDownInputButtons; m_paOnDownInputButtons = NULL;
	delete [] m_paInputButtonFuncs;   m_paInputButtonFuncs = NULL;
}


BEGIN_MESSAGE_MAP(CMaiToolBarWnd, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CMainToolsWnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

				  
BOOL CMaiToolBarWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return CMiniFrameWnd::OnCommand(wParam, lParam);
}


BOOL CMaiToolBarWnd::Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect) 
{
	m_nToolBarResourceId = nToolBarResourceId;
	Create(NULL, pszTitle, WS_BORDER |  WS_VISIBLE /* |    WS_CHILD | WS_CAPTION | WS_SYSMENU */, initRect, NULL, 0);
	ShowWindow(SW_SHOW);

	m_pToolBar = new CToolBar();
	m_pToolBar->Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP, m_nToolBarResourceId);
	m_pToolBar->LoadToolBar(m_nToolBarResourceId);

	//how much space for main toolbar needed
    CSize rSize = m_pToolBar->CalcFixedLayout(FALSE, TRUE);
    CRect rcClientNow;
    RepositionBars(m_nToolBarResourceId,
                   m_nToolBarResourceId, 0,
                   reposQuery, rcClientNow);

    CRect rcWindow;
    GetWindowRect(rcWindow);
    rcWindow.right += rSize.cx;
    rcWindow.bottom += rSize.cy;
   
	MoveWindow( rcWindow, TRUE);
	SetWindowPos(&wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW   );
    RepositionBars(m_nToolBarResourceId, m_nToolBarResourceId, 0);

	//	m_pToolBarWindow->EnableDocking(CBRS_ALIGN_ANY);	
	//	m_pToolBar->EnableDocking(CBRS_FLOAT_MULTI );
	return TRUE;
}


void CMaiToolBarWnd::Work(const CMaiUserInput& rMaiUI)
{
	//
	// Do any actions that result from button presses in the control bar
	//
	for (int i = 0; i < GetNumButtons(); i++)
	{
		if (m_paOnDownInputButtons[i] && m_paInputButtonFuncs[i])
		{
			(this->*m_paInputButtonFuncs[i])();
		}
		m_paOnDownInputButtons[i] = 0;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//
//	 CMainToolBarWnd
//
//
//
CMainToolsWnd::CMainToolsWnd(void)
: CMaiToolBarWnd(),
  m_pCurToolWnd(NULL),
  m_nMode(TOOLMODE_INVALID)
{
}


CMainToolsWnd::~CMainToolsWnd(void)
{
	if (m_pCurToolWnd)
	{
		m_pCurToolWnd->DestroyWindow();
	}
	m_pCurToolWnd = NULL;
}


BEGIN_MESSAGE_MAP(CMainToolsWnd, CMaiToolBarWnd)
	//{{AFX_MSG_MAP(CMainToolsWnd)
	ON_COMMAND(ID_GRAPHTOOLBUTTON, OnGraphTool)
	ON_COMMAND(ID_3DGRAPHTOOLBUTTON, On3DGraphTool)
	ON_COMMAND(ID_GRAPHTESTBUTTON, OnGraphTestTool)
//	ON_UPDATE_COMMAND_UI(ID_GRAPHTOOLBUTTON, OnGraphTool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CMainToolsWnd::GetNumButtons(void)
{
	return NUM_MAINTOOLS_BUTTONS;
}


BOOL CMainToolsWnd::Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect)
{
	CMaiToolBarWnd::Init(nToolBarResourceId, pszTitle, pParendWnd, initRect);

	m_pToolBar->SetButtonStyle( 0, TBBS_CHECKGROUP);
	m_pToolBar->GetToolBarCtrl().SetState(ID_GRAPHTOOLBUTTON, TBSTATE_CHECKED | TBSTATE_ENABLED );
	m_pToolBar->SetButtonStyle( 1, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 2, TBBS_CHECKGROUP);

	//
	// Initialize the button handling stuff now
	//
	int numButts = GetNumButtons();
	if (numButts)
	{
		m_paOnDownInputButtons = new u8[numButts];
		for (int i = 0; i < numButts; i++)
		{
			m_paOnDownInputButtons[i] = 0;
		}

		m_paInputButtonFuncs = (PMaiToolBarButtonFunc) new s32[numButts];
		m_paInputButtonFuncs[0] = (MaiToolBarButtonFunc) DoOnGraphTool;
		m_paInputButtonFuncs[1] = (MaiToolBarButtonFunc) DoOn3DGraphTool;
		m_paInputButtonFuncs[2] = (MaiToolBarButtonFunc) DoOnGraphTestTool;
	}
	DoOnGraphTool();  //start in 2D mode
	
	return FALSE;
}


void CMainToolsWnd::Work(const CMaiUserInput& rMaiUI)
{
	CMaiToolBarWnd::Work(rMaiUI);

	if (m_pCurToolWnd)
	{
		m_pCurToolWnd->Work(rMaiUI);
	}
}


void CMainToolsWnd::OnGraphTool()
{
	m_paOnDownInputButtons[0] = 1;
}


void CMainToolsWnd::On3DGraphTool()
{
	m_paOnDownInputButtons[1] = 1;
}


void CMainToolsWnd::OnGraphTestTool()
{
	m_paOnDownInputButtons[2] = 1;
}

void CMainToolsWnd::DoOnGraphTool()
{
	if (m_pCurToolWnd)
	{
		m_pCurToolWnd->DestroyWindow();
		m_pCurToolWnd = NULL;
	}
	m_pCurToolWnd = new CGraphToolsWnd();
	m_pCurToolWnd->Init(IDR_GRAPHTOOLBAR, "2D Graph Editor Tools", this, CRect(0,0,20,40));
	m_nMode = TOOLMODE_EDITGRAPH2D;
	maimain_pEditGraph->m_b3DMode = FALSE;
	maimain_pEditGraph->m_b2DMode = TRUE;

	APE_DELETE(maimain_pDebugGraphSearcher); maimain_pDebugGraphSearcher = NULL;
}


void CMainToolsWnd::DoOn3DGraphTool()
{
	if (m_pCurToolWnd)
	{
		m_pCurToolWnd->DestroyWindow();
		m_pCurToolWnd = NULL;
	}
	m_pCurToolWnd = new C3DGraphToolsWnd();
	m_pCurToolWnd->Init(IDR_GRAPHTOOLBAR3D, "3D Graph Editor Tools", this, CRect(0,0,20,40));
	m_nMode = TOOLMODE_EDITGRAPH3D;
	maimain_pEditGraph->m_b3DMode = TRUE;
	maimain_pEditGraph->m_b2DMode = FALSE;

	APE_DELETE(maimain_pDebugGraphSearcher); maimain_pDebugGraphSearcher = NULL;
}


void CMainToolsWnd::DoOnGraphTestTool()
{
	if (m_pCurToolWnd)
	{
		m_pCurToolWnd->DestroyWindow();
		m_pCurToolWnd = NULL;
	}

	m_nMode = TOOLMODE_TESTGRAPH;
	m_pCurToolWnd = new CTestGraphToolsWnd();
	m_pCurToolWnd->Init(IDR_GRAPHTESTTOOLBAR, "Graph Tester Tools", this, CRect(0,0,20,40));

	APE_DELETE(maimain_pDebugGraphSearcher);
	maimain_pDebugGraphSearcher = APE_NEW CGraphSearcher();
	maimain_pDebugGraphSearcher->BindToGraph(maimain_pEditGraph);
	maimain_pDebugGraphSearcher->UsePNodePool(maimain_pNodePool);
}






/////////////////////////////////////////////////////////////////////////////
//
//
// class CGraphToolsWnd
//
//
//
CGraphToolsWnd::CGraphToolsWnd(void)
: CMaiToolBarWnd(), m_pCurTool(NULL)
{
	m_aGraphTools[GRAPHEDITTOOL2D_MOVE2DVERTTOOL]  = new SelectTool();
	((SelectTool*) m_aGraphTools[GRAPHEDITTOOL2D_MOVE2DVERTTOOL])->m_uMoveStyle = SelectTool::MOVE_STYLE_2D;
	FASSERT(m_aGraphTools[GRAPHEDITTOOL2D_MOVE2DVERTTOOL]);

	m_aGraphTools[GRAPHEDITTOOL2D_MOVE3DVERTTOOL]  = new SelectTool();
	((SelectTool*) m_aGraphTools[GRAPHEDITTOOL2D_MOVE3DVERTTOOL])->m_uMoveStyle = SelectTool::MOVE_STYLE_3D;
	FASSERT(m_aGraphTools[GRAPHEDITTOOL2D_MOVE2DVERTTOOL]);

	m_aGraphTools[GRAPHEDITTOOL2D_ADDTOOL]  = new AddTool();
	FASSERT(m_aGraphTools[GRAPHEDITTOOL2D_ADDTOOL]);
	
	m_aGraphTools[GRAPHEDITTOOL2D_ADDJOINTOOL]  = new AddTool();
	FASSERT(m_aGraphTools[GRAPHEDITTOOL2D_ADDJOINTOOL]);
	((AddTool*) m_aGraphTools[GRAPHEDITTOOL2D_ADDJOINTOOL])->SetJoinMode(TRUE);

	m_aGraphTools[GRAPHEDITTOOL2D_JOINTOOL]  = new JoinTool();
	FASSERT(m_aGraphTools[GRAPHEDITTOOL2D_JOINTOOL]);

	m_aGraphTools[GRAPHEDITTOOL2D_UNJOINTOOL]  = new JoinTool();
	FASSERT(m_aGraphTools[GRAPHEDITTOOL2D_UNJOINTOOL]);
	((JoinTool*) m_aGraphTools[GRAPHEDITTOOL2D_UNJOINTOOL])->m_bUnLinkMode = TRUE;
	
	m_aGraphTools[GRAPHEDITTOOL2D_POITOOL]  = new CPoiTool();
	FASSERT(m_aGraphTools[GRAPHEDITTOOL2D_POITOOL]);
	
	m_pCurTool = NULL;
	m_nCurToolId = GRAPHEDITTOOL2D_INVALIDTOOL;

	m_bCalcingVols = FALSE;

	m_bManModeOn = FALSE;
}


CGraphToolsWnd::~CGraphToolsWnd(void)
{
	//undo any cruel things that this too may have done to the graph.
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->ClearSelected();
	}

	for (int i = 0; i < GRAPHEDITTOOL2D_NUM_GRAPH_TOOLS; i++)
	{
		delete m_aGraphTools[i]; m_aGraphTools[i] = NULL;
	}
	m_pCurTool = NULL;
	m_nCurToolId = GRAPHEDITTOOL2D_INVALIDTOOL;

}


BEGIN_MESSAGE_MAP(CGraphToolsWnd, CMaiToolBarWnd)
	//{{AFX_MSG_MAP(CMainToolsWnd)
	ON_COMMAND(ID_SELECTTOOL, OnSelectTool)
	ON_COMMAND(ID_MOVE3DTOOL, OnMove3DTool)
	ON_COMMAND(ID_ADDONLY, OnAddTool)
	ON_COMMAND(ID_ADDJOIN, OnAddJoinTool)
	ON_COMMAND(ID_JOIN, OnJoinTool)
	ON_COMMAND(ID_UNJOIN, OnUnJoinTool)
	ON_COMMAND(ID_POI, OnPoiTool)
	ON_COMMAND(ID_SAVEBUTTON, OnSave)
	ON_COMMAND(ID_VIEWVOLUMES, OnViewVols)
	ON_COMMAND(ID_CALCVOLS,	OnCalcVols)
	ON_COMMAND(ID_AUTOJOIN, OnAutoJoin)
	ON_COMMAND(ID_VIEWEDGEVOLS, OnViewEdgeVols)
	ON_COMMAND(ID_UNDOBUTTON, OnUndo)
	ON_COMMAND(ID_VIEWALLVOLS, OnViewAllVols)
	ON_COMMAND(ID_VIEWNEARVOLS, OnViewNearVols)
	ON_COMMAND(ID_VIEWCUSTOMVOLS, OnViewCustomVols)
	ON_COMMAND(ID_MANMODE, OnManMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CGraphToolsWnd::Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect)
{
    CRect rcWindow;
    pParendWnd->GetWindowRect(rcWindow);
	int height = rcWindow.Height();
    rcWindow.top = rcWindow.bottom+1;
    rcWindow.bottom = rcWindow.top+22;
    rcWindow.left = rcWindow.left;
    rcWindow.right = rcWindow.left+20;


	CMaiToolBarWnd::Init(nToolBarResourceId, pszTitle, pParendWnd, rcWindow);


	m_pToolBar->SetButtonStyle( 0, TBBS_CHECKGROUP);
	m_pToolBar->GetToolBarCtrl().SetState(ID_SELECTTOOL, TBSTATE_CHECKED | TBSTATE_ENABLED );
	m_pToolBar->SetButtonStyle( 1, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 2, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 3, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 4, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 5, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 6, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 7, TBBS_SEPARATOR);
	m_pToolBar->SetButtonStyle( 8, TBBS_BUTTON); //save
	m_pToolBar->SetButtonStyle( 9, TBBS_SEPARATOR);
	m_pToolBar->SetButtonStyle( 10, TBBS_BUTTON);
	m_pToolBar->SetButtonStyle( 11, TBBS_BUTTON);
	m_pToolBar->SetButtonStyle( 12, TBBS_BUTTON);
	m_pToolBar->SetButtonStyle( 13, TBBS_SEPARATOR);
	m_pToolBar->SetButtonStyle( 14, TBBS_CHECKBOX);  
	m_pToolBar->SetButtonStyle( 15, TBBS_CHECKBOX);		//ID_VIEWEDGEVOLS
	m_pToolBar->SetButtonStyle( 16, TBBS_SEPARATOR);  
	m_pToolBar->SetButtonStyle( 17, TBBS_CHECKGROUP);	//ID_VIEWALLVOLS
	m_pToolBar->SetButtonStyle( 18, TBBS_CHECKGROUP);	//ID_VIEWNEARVOLS
	m_pToolBar->SetButtonStyle( 19, TBBS_CHECKGROUP);	//ID_VIEWCUSTOMVOLS
	m_pToolBar->SetButtonStyle( 20, TBBS_SEPARATOR);
	m_pToolBar->SetButtonStyle( 21, TBBS_CHECKBOX);
	m_pToolBar->GetToolBarCtrl().SetState(ID_VIEWALLVOLS, TBSTATE_CHECKED | TBSTATE_ENABLED );
	m_pToolBar->GetToolBarCtrl().SetState(ID_VIEWVOLUMES, TBSTATE_ENABLED | (maimain_pEditGraph->m_bDrawVertVolumes*TBSTATE_CHECKED));
	m_pToolBar->GetToolBarCtrl().SetState(ID_VIEWEDGEVOLS, TBSTATE_ENABLED | (maimain_pEditGraph->m_bDrawEdgeVolumes*TBSTATE_CHECKED));

	
	m_nCurToolId = GRAPHEDITTOOL2D_MOVE2DVERTTOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];


	//
	// Initialize the button handling stuff now
	//
	s32 numButts = GetNumButtons();
	if (numButts)
	{
		m_paOnDownInputButtons = new u8[numButts];
		for (int i = 0; i < numButts; i++)
		{
			m_paOnDownInputButtons[i] = 0;
		}
		m_paInputButtonFuncs = (PMaiToolBarButtonFunc) new s32[numButts];
		m_paInputButtonFuncs[0] = (MaiToolBarButtonFunc) DoOnSelectTool;
		m_paInputButtonFuncs[1] = (MaiToolBarButtonFunc) DoOnAddTool;
		m_paInputButtonFuncs[2] = (MaiToolBarButtonFunc) DoOnAddJoinTool;
		m_paInputButtonFuncs[3] = (MaiToolBarButtonFunc) DoOnJoinTool;
		m_paInputButtonFuncs[4] = (MaiToolBarButtonFunc) DoOnUnJoinTool;
		m_paInputButtonFuncs[5] = (MaiToolBarButtonFunc) DoOnSave;
		m_paInputButtonFuncs[6] = (MaiToolBarButtonFunc) DoOnCalcVols;
		m_paInputButtonFuncs[7] = (MaiToolBarButtonFunc) DoOnAutoJoin;
		m_paInputButtonFuncs[8] = (MaiToolBarButtonFunc) DoOnViewVols;
		m_paInputButtonFuncs[9] = (MaiToolBarButtonFunc) DoOnViewEdgeVols;
		m_paInputButtonFuncs[10] = (MaiToolBarButtonFunc) DoOnUndo;
		m_paInputButtonFuncs[11] = (MaiToolBarButtonFunc) DoOnMove3DTool;
		m_paInputButtonFuncs[12] = (MaiToolBarButtonFunc) DoOnViewAllVols;
		m_paInputButtonFuncs[13] = (MaiToolBarButtonFunc) DoOnViewNearVols;
		m_paInputButtonFuncs[14] = (MaiToolBarButtonFunc) DoOnViewCustomVols;
		m_paInputButtonFuncs[15] = (MaiToolBarButtonFunc) DoOnManMode;
		m_paInputButtonFuncs[16] = (MaiToolBarButtonFunc) DoOnPoiTool;
	}
	return TRUE;
}


int CGraphToolsWnd::GetNumButtons(void)   //override this and make it match the number of buttons
{
	return NUM_GRAPH_TOOLS_BUTTONS;
}


void CGraphToolsWnd::OnSelectTool()
{
	m_paOnDownInputButtons[0] = 1;
}


void CGraphToolsWnd::OnAddTool()
{
	m_paOnDownInputButtons[1] = 1;
}


void CGraphToolsWnd::OnAddJoinTool()
{
	m_paOnDownInputButtons[2] = 1;
}


void CGraphToolsWnd::OnJoinTool()
{
	m_paOnDownInputButtons[3] = 1;
}


void CGraphToolsWnd::OnUnJoinTool()
{
	m_paOnDownInputButtons[4] = 1;
}


void CGraphToolsWnd::OnSave()
{
	m_paOnDownInputButtons[5] = 1;
}


void CGraphToolsWnd::OnCalcVols()
{
	m_paOnDownInputButtons[6] = 1;
}


void CGraphToolsWnd::OnAutoJoin()
{
	m_paOnDownInputButtons[7] = 1;
}


void CGraphToolsWnd::OnViewVols()
{
	m_paOnDownInputButtons[8] = 1;

}


void CGraphToolsWnd::OnViewEdgeVols()
{
	m_paOnDownInputButtons[9] = 1;
}


void CGraphToolsWnd::OnUndo()
{
	m_paOnDownInputButtons[10] = 1;
}


void CGraphToolsWnd::OnMove3DTool()
{
	m_paOnDownInputButtons[11] = 1;
}


void CGraphToolsWnd::OnViewAllVols()
{
	m_paOnDownInputButtons[12] = 1;
}


void CGraphToolsWnd::OnViewNearVols()
{
	m_paOnDownInputButtons[13] = 1;
}


void CGraphToolsWnd::OnViewCustomVols()
{
	m_paOnDownInputButtons[14] = 1;
}

void CGraphToolsWnd::OnManMode()
{
	m_paOnDownInputButtons[15] = 1;
}

void CGraphToolsWnd::OnPoiTool()
{
	m_paOnDownInputButtons[16] = 1;
}


void CGraphToolsWnd::DoOnSelectTool()
{
	m_nCurToolId = GRAPHEDITTOOL2D_MOVE2DVERTTOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];
	m_pCurTool->OnFocus();
}


void CGraphToolsWnd::DoOnAddTool()
{
	m_nCurToolId = GRAPHEDITTOOL2D_ADDTOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];
	m_pCurTool->OnFocus();
}


void CGraphToolsWnd::DoOnAddJoinTool()
{
	m_nCurToolId = GRAPHEDITTOOL2D_ADDJOINTOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];
	m_pCurTool->OnFocus();
}


void CGraphToolsWnd::DoOnJoinTool()
{
	m_nCurToolId = GRAPHEDITTOOL2D_JOINTOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];
	m_pCurTool->OnFocus();
}


void CGraphToolsWnd::DoOnUnJoinTool()
{
	m_nCurToolId = GRAPHEDITTOOL2D_UNJOINTOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];
	m_pCurTool->OnFocus();
}


void CGraphToolsWnd::DoOnPoiTool()
{
	m_nCurToolId = GRAPHEDITTOOL2D_POITOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];
	m_pCurTool->OnFocus();
}


void CGraphToolsWnd::DoOnMove3DTool()
{
	m_nCurToolId = GRAPHEDITTOOL2D_MOVE3DVERTTOOL;
	m_pCurTool = m_aGraphTools[m_nCurToolId];
	m_pCurTool->OnFocus();
}


void CGraphToolsWnd::DoOnSave()
{
	if (maimain_pEditGraph)
	{
		maimain_MakeBackupCopy(maimain_szGraphFileFullPath, K9_GetBackupDirPath());
		BOOL bDidSave = maimain_pEditGraph->SaveToAsciiFile(maimain_szGraphFileFullPath);
		while (!bDidSave && MessageBox("    Save Failed ", "File Error", MB_RETRYCANCEL) == IDRETRY)
		{
				bDidSave = maimain_pEditGraph->SaveToAsciiFile(maimain_szGraphFileFullPath);
		}
	}
}


void CGraphToolsWnd::DoOnViewVols()
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_bDrawVertVolumes ^= 1;
	}
}


void CGraphToolsWnd::DoOnViewEdgeVols()
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_bDrawEdgeVolumes ^= 1;
	}
}


void CGraphToolsWnd::DoOnCalcVols()
{
	if (maimain_pEditGraph)
	{
		maiundo_Store();

		m_bCalcingVols = TRUE;
		m_bCalcingVolsCurrent = 1;
		m_bCalcingVolsMax = maimain_pEditGraph->GetNumVerts()-1;
	}
}


void CGraphToolsWnd::DoOnAutoJoin()
{
	if (maimain_pEditGraph)
	{
		maiundo_Store();
		maimain_pEditGraph->AutoLinkSelected();
	}
}


void CGraphToolsWnd::DoOnUndo()
{
	if (maimain_pEditGraph)
	{
		maiundo_Restore();
	}
}

void CGraphToolsWnd::DoOnViewAllVols(void)
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_nDrawVolMode = CEditGraph::DRAWVOLMODE_ALL;
	}
}


void CGraphToolsWnd::DoOnViewNearVols(void)
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_nDrawVolMode = CEditGraph::DRAWVOLMODE_NEAR;
	}
}


void CGraphToolsWnd::DoOnViewCustomVols(void)
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_nDrawVolMode = CEditGraph::DRAWVOLMODE_CUSTOM;
	}
}


void CGraphToolsWnd::DoOnManMode(void)
{
	m_bManModeOn ^= 1;
}



BOOL CGraphToolsWnd::IsViewVolsButtonDown(void)
{
	return m_pToolBar->GetToolBarCtrl().GetState(ID_VIEWVOLUMES) & TBSTATE_CHECKED;
}

BOOL CGraphToolsWnd::IsViewEdgeVolsButtonDown(void)
{
	return m_pToolBar->GetToolBarCtrl().GetState(ID_VIEWEDGEVOLS) & TBSTATE_CHECKED;
}

BOOL CGraphToolsWnd::IsManModeButtonDown(void)
{
	return m_pToolBar->GetToolBarCtrl().GetState(ID_MANMODE) & TBSTATE_CHECKED;
}


void CGraphToolsWnd::Work(const CMaiUserInput& rMaiUI)
{
	CMaiToolBarWnd::Work(rMaiUI);

	if (m_pCurTool)
	{
		m_pCurTool->UserInput(rMaiUI);
		m_pCurTool->Draw();
	}

	if (m_bCalcingVols)
	{
		maimain_pEditGraph->AutoSetDimension2D(m_bCalcingVolsCurrent++);
		if (m_bCalcingVolsCurrent > m_bCalcingVolsMax)
		{
			m_bCalcingVols = FALSE;
		}

		ftext_DebugPrintf( 0.45f, 0.450f, "~arCALC_VOLUMES: ~w1%02d of ~w1%02d", m_bCalcingVolsCurrent, m_bCalcingVolsMax);
	}

}



//
//
//
// class CTestGraphToolsWnd
//
//
//

CTestGraphToolsWnd::CTestGraphToolsWnd(void)
: CMaiToolBarWnd()
{
}


CTestGraphToolsWnd ::~CTestGraphToolsWnd()
{
}


BOOL CTestGraphToolsWnd::Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect)
{
    CRect rcWindow;
    pParendWnd->GetWindowRect(rcWindow);
	int height = rcWindow.Height();
    rcWindow.top = rcWindow.bottom+1;
    rcWindow.bottom = rcWindow.top+22;
    rcWindow.left = rcWindow.left;
    rcWindow.right = rcWindow.left+20;

	CMaiToolBarWnd::Init(nToolBarResourceId, pszTitle, pParendWnd, rcWindow);

	
	m_pToolBar->SetButtonStyle( 0, TBBS_CHECKBOX);
	m_pToolBar->SetButtonStyle( 1, TBBS_CHECKBOX);

	m_TestGraphTool.SetTestPointTracking(TestGraphTool::ORIGIN_TESTPOINT, FALSE);
	m_TestGraphTool.SetTestPointTracking(TestGraphTool::GOAL_TESTPOINT, FALSE);
	FASSERT(aimain_pQueryDetailsList->Size()>0);

	m_TestGraphTool.SetQueryDetail(aimain_pQueryDetailsList->Begin().Get());


	
	//
	// Initialize the button handling stuff now
	//
	s32 numButts = GetNumButtons();
	if (numButts)
	{
		m_paOnDownInputButtons = new u8[numButts];
		for (int i = 0; i < numButts; i++)
		{
			m_paOnDownInputButtons[i] = 0;
		}
		m_paInputButtonFuncs = (PMaiToolBarButtonFunc) new s32[numButts];
		m_paInputButtonFuncs[0] = (MaiToolBarButtonFunc) DoOnOriginButton;
		m_paInputButtonFuncs[1] = (MaiToolBarButtonFunc) DoOnGoalButton;
		m_paInputButtonFuncs[2] = (MaiToolBarButtonFunc) DoOnConfigButton;
		m_paInputButtonFuncs[3] = (MaiToolBarButtonFunc) DoOnFindButton;
	}
	
	return TRUE;
}


BEGIN_MESSAGE_MAP(CTestGraphToolsWnd, CMaiToolBarWnd)
	//{{AFX_MSG_MAP(CMainToolsWnd)
	ON_COMMAND(ID_ORIGINBUTTON, OnOriginButton)
	ON_COMMAND(ID_GOALBUTTON, OnGoalButton)
	ON_COMMAND(ID_RANDOMBUTTON, OnConfigButton)
	ON_COMMAND(ID_FINDPATHBUTTON, OnFindButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CTestGraphToolsWnd::GetNumButtons(void)    //override this and make it match the number of buttons
{
	return NUM_TESTGRAPH_BUTTONS;
}


void CTestGraphToolsWnd::Work(const CMaiUserInput& rMaiUI)
{
	CMaiToolBarWnd::Work(rMaiUI);
	m_TestGraphTool.UserInput(rMaiUI);
	m_TestGraphTool.Draw();
}


void CTestGraphToolsWnd::OnOriginButton()
{
	m_paOnDownInputButtons[0] = 1;
}


void CTestGraphToolsWnd::OnGoalButton()
{
	m_paOnDownInputButtons[1] = 1;
}


void CTestGraphToolsWnd::OnConfigButton()
{
	m_paOnDownInputButtons[2] = 1;
}


void CTestGraphToolsWnd::OnFindButton()
{
	m_paOnDownInputButtons[3] = 1;
}



void CTestGraphToolsWnd::DoOnOriginButton()
{
	m_TestGraphTool.ToggleTestPointTracking(TestGraphTool::ORIGIN_TESTPOINT);
}


void CTestGraphToolsWnd::DoOnGoalButton()
{
	m_TestGraphTool.ToggleTestPointTracking(TestGraphTool::GOAL_TESTPOINT);
}


void CTestGraphToolsWnd::DoOnConfigButton()
{
	CQueryDetailsDlg dlg(aimain_pQueryDetailsList, this);

//			dlg.SetWindowPos(&CWnd::wndTopMost, rMaiUI.nLastMouseX, rMaiUI.nLastMouseY,0,0, SWP_NOSIZE | SWP_NOZORDER );
	if (dlg.DoModal())
	{	//copy stuff back into the vert
		m_TestGraphTool.SetQueryDetail(dlg.GetLastSelectedDetails());
	}

}


void CTestGraphToolsWnd::DoOnFindButton()
{
	m_TestGraphTool.FindPath();
}




/////////////////////////////////////////////////////////////////////////////
//
//
// class C3DGraphToolsWnd
//
//
//
C3DGraphToolsWnd::C3DGraphToolsWnd(void)
: CMaiToolBarWnd(), m_pCurTool(NULL)
{
	m_aGraphTools[GRAPHTOOL_3DMOVETOOL] = new SelectTool();
	((SelectTool*) m_aGraphTools[GRAPHTOOL_3DMOVETOOL])->m_uMoveStyle = SelectTool::MOVE_STYLE_3D;
	FASSERT(m_aGraphTools[GRAPHTOOL_3DMOVETOOL]);

	m_aGraphTools[GRAPHTOOL_3DADDJOINTOOL] = new AddTool3D();
	FASSERT(m_aGraphTools[GRAPHTOOL_3DADDJOINTOOL]);
	((AddTool*) m_aGraphTools[GRAPHTOOL_3DADDJOINTOOL])->SetJoinMode(TRUE);

	m_aGraphTools[GRAPHTOOL_3DJOINTOOL] = new JoinTool();
	FASSERT(m_aGraphTools[GRAPHTOOL_3DJOINTOOL]);
	((JoinTool*) m_aGraphTools[GRAPHTOOL_3DJOINTOOL])->m_uJoinStyle = JoinTool::JOIN_STYLE_3D;

	m_aGraphTools[GRAPHTOOL_3DUNJOINTOOL]  = new JoinTool();
	FASSERT(m_aGraphTools[GRAPHTOOL_3DUNJOINTOOL]);
	((JoinTool*) m_aGraphTools[GRAPHTOOL_3DUNJOINTOOL])->m_bUnLinkMode = TRUE;
	((JoinTool*) m_aGraphTools[GRAPHTOOL_3DUNJOINTOOL])->m_uJoinStyle = JoinTool::JOIN_STYLE_3D;
	
	m_pCurTool = NULL;
}


C3DGraphToolsWnd::~C3DGraphToolsWnd(void)
{
	//undo any cruel things that this too may have done to the graph.
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->ClearSelected();
	}

	for (s32 i = 0; i < NUM_GRAPHTOOLS; i++)
	{
		delete m_aGraphTools[i]; m_aGraphTools[i] = NULL;
	}
}


BEGIN_MESSAGE_MAP(C3DGraphToolsWnd, CMaiToolBarWnd)
	//{{AFX_MSG_MAP(C3DGraphToolsWnd)
	ON_COMMAND(ID_3DMOVE3DTOOL,		On3DMove3DTool)
	ON_COMMAND(ID_3DADDJOIN,		On3DAddJoinTool)
	ON_COMMAND(ID_3DJOIN,			On3DJoinTool)
	ON_COMMAND(ID_3DUNJOIN,			On3DUnJoinTool)
	ON_COMMAND(ID_3DSAVEBUTTON,		On3DSave)
	ON_COMMAND(ID_3DVIEWVOLUMES,	On3DViewVols)
	ON_COMMAND(ID_3DVIEWEDGEVOLS,	On3DViewEdgeVols)
	ON_COMMAND(ID_3DUNDOBUTTON,		On3DUndo)
	ON_COMMAND(ID_3DVIEWALLVOLS,	On3DViewAllVols)
	ON_COMMAND(ID_3DVIEWNEARVOLS,	On3DViewNearVols)
	ON_COMMAND(ID_3DVIEWCUSTOMVOLS, On3DViewCustomVols)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL C3DGraphToolsWnd::Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect)
{
    CRect rcWindow;
    pParendWnd->GetWindowRect(rcWindow);
	int height = rcWindow.Height();
    rcWindow.top = rcWindow.bottom+1;
    rcWindow.bottom = rcWindow.top+22;
    rcWindow.left = rcWindow.left;
    rcWindow.right = rcWindow.left+20;

	m_pCurTool = NULL;
	CMaiToolBarWnd::Init(nToolBarResourceId, pszTitle, pParendWnd, rcWindow);


	m_pToolBar->SetButtonStyle( 0, TBBS_CHECKGROUP);
	m_pToolBar->GetToolBarCtrl().SetState(ID_3DMOVE3DTOOL, TBSTATE_CHECKED | TBSTATE_ENABLED );
	m_pToolBar->SetButtonStyle( 1, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 2, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 3, TBBS_CHECKGROUP);
	m_pToolBar->SetButtonStyle( 4, TBBS_SEPARATOR);
	m_pToolBar->SetButtonStyle( 5, TBBS_BUTTON); //save
	m_pToolBar->SetButtonStyle( 6, TBBS_SEPARATOR);
	m_pToolBar->SetButtonStyle( 7, TBBS_BUTTON);  //undo
	m_pToolBar->SetButtonStyle( 8, TBBS_SEPARATOR);
	m_pToolBar->SetButtonStyle( 9, TBBS_CHECKBOX);  //ID_VIEWWAyptVOLS
	m_pToolBar->SetButtonStyle( 10, TBBS_CHECKBOX);	 //ID_VIEWEDGEVOLS
	m_pToolBar->SetButtonStyle( 11, TBBS_SEPARATOR);  
	m_pToolBar->SetButtonStyle( 12, TBBS_CHECKGROUP);	 //ID_VIEWALLVOLS
	m_pToolBar->SetButtonStyle( 13, TBBS_CHECKGROUP);	 //ID_VIEWNEARVOLS

	m_pToolBar->SetButtonStyle( 14, TBBS_CHECKGROUP);	 //ID_VIEWCUSTOMVOLS
	
	m_pToolBar->GetToolBarCtrl().SetState(ID_3DVIEWALLVOLS, TBSTATE_CHECKED | TBSTATE_ENABLED );
	m_pToolBar->GetToolBarCtrl().SetState(ID_3DVIEWVOLUMES, TBSTATE_ENABLED | (maimain_pEditGraph->m_bDrawVertVolumes*TBSTATE_CHECKED));
	m_pToolBar->GetToolBarCtrl().SetState(ID_3DVIEWEDGEVOLS, TBSTATE_ENABLED | (maimain_pEditGraph->m_bDrawEdgeVolumes*TBSTATE_CHECKED));


	
	m_pCurTool = m_aGraphTools[GRAPHTOOL_3DMOVETOOL];

	//
	// Initialize the button handling stuff now
	//
	s32 numButts = GetNumButtons();
	if (numButts)
	{
		m_paOnDownInputButtons = new u8[numButts];
		for (int i = 0; i < numButts; i++)
		{
			m_paOnDownInputButtons[i] = 0;
		}
		m_paInputButtonFuncs = (PMaiToolBarButtonFunc) new s32[numButts];
		m_paInputButtonFuncs[0] = (MaiToolBarButtonFunc) DoOn3DAddJoinTool;
		m_paInputButtonFuncs[1] = (MaiToolBarButtonFunc) DoOn3DJoinTool;
		m_paInputButtonFuncs[2] = (MaiToolBarButtonFunc) DoOn3DUnJoinTool;
		m_paInputButtonFuncs[3] = (MaiToolBarButtonFunc) DoOn3DSave;
		m_paInputButtonFuncs[4] = (MaiToolBarButtonFunc) DoOn3DViewVols;
		m_paInputButtonFuncs[5] = (MaiToolBarButtonFunc) DoOn3DViewEdgeVols;
		m_paInputButtonFuncs[6] = (MaiToolBarButtonFunc) DoOn3DUndo;
		m_paInputButtonFuncs[7] = (MaiToolBarButtonFunc) DoOn3DMove3DTool;
		m_paInputButtonFuncs[8] = (MaiToolBarButtonFunc) DoOn3DViewAllVols;
		m_paInputButtonFuncs[9] = (MaiToolBarButtonFunc) DoOn3DViewNearVols;
		m_paInputButtonFuncs[10] = (MaiToolBarButtonFunc) DoOn3DViewCustomVols;
		ASSERT(10 == numButts-1);
	}
	return TRUE;
}


int C3DGraphToolsWnd::GetNumButtons(void)   //override this and make it match the number of buttons
{
	return NUM_3DGRAPH_TOOLS_BUTTONS;
}



void C3DGraphToolsWnd::On3DAddJoinTool()
{
	m_paOnDownInputButtons[0] = 1;
}


void C3DGraphToolsWnd::On3DJoinTool()
{
	m_paOnDownInputButtons[1] = 1;
}


void C3DGraphToolsWnd::On3DUnJoinTool()
{
	m_paOnDownInputButtons[2] = 1;
}


void C3DGraphToolsWnd::On3DSave()
{
	m_paOnDownInputButtons[3] = 1;
}


void C3DGraphToolsWnd::On3DViewVols()
{
	m_paOnDownInputButtons[4] = 1;
}


void C3DGraphToolsWnd::On3DViewEdgeVols()
{
	m_paOnDownInputButtons[5] = 1;
}


void C3DGraphToolsWnd::On3DUndo()
{
	m_paOnDownInputButtons[6] = 1;
}


void C3DGraphToolsWnd::On3DMove3DTool()
{
	m_paOnDownInputButtons[7] = 1;
}


void C3DGraphToolsWnd::On3DViewAllVols()
{
	m_paOnDownInputButtons[8] = 1;
}


void C3DGraphToolsWnd::On3DViewNearVols()
{
	m_paOnDownInputButtons[9] = 1;
}


void C3DGraphToolsWnd::On3DViewCustomVols()
{
	m_paOnDownInputButtons[10] = 1;
}

void C3DGraphToolsWnd::DoOn3DMove3DTool()
{
	m_pCurTool = m_aGraphTools[GRAPHTOOL_3DMOVETOOL];
	m_pCurTool->OnFocus();
}

void C3DGraphToolsWnd::DoOn3DAddJoinTool()
{
	m_pCurTool = m_aGraphTools[GRAPHTOOL_3DADDJOINTOOL];
	m_pCurTool->OnFocus();
}


void C3DGraphToolsWnd::DoOn3DJoinTool()
{
	m_pCurTool = m_aGraphTools[GRAPHTOOL_3DJOINTOOL];
	m_pCurTool->OnFocus();
}


void C3DGraphToolsWnd::DoOn3DUnJoinTool()
{
	m_pCurTool = m_aGraphTools[GRAPHTOOL_3DUNJOINTOOL];
	m_pCurTool->OnFocus();
}


void C3DGraphToolsWnd::DoOn3DSave()
{
	if (maimain_pEditGraph)
	{
		maimain_MakeBackupCopy(maimain_szGraphFileFullPath, K9_GetBackupDirPath());
		maimain_pEditGraph->SaveToAsciiFile(maimain_szGraphFileFullPath);
	}
}


void C3DGraphToolsWnd::DoOn3DViewVols()
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_bDrawVertVolumes ^= 1;
	}
}


void C3DGraphToolsWnd::DoOn3DViewAllVols(void)
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_nDrawVolMode = CEditGraph::DRAWVOLMODE_ALL;
	}
}


void C3DGraphToolsWnd::DoOn3DViewEdgeVols()
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_bDrawEdgeVolumes ^= 1;
	}
}


void C3DGraphToolsWnd::DoOn3DUndo()
{
	if (maimain_pEditGraph)
	{
		maiundo_Restore();
	}
}


void C3DGraphToolsWnd::DoOn3DViewNearVols(void)
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_nDrawVolMode = CEditGraph::DRAWVOLMODE_NEAR;
	}
}


void C3DGraphToolsWnd::DoOn3DViewCustomVols(void)
{
	if (maimain_pEditGraph)
	{
		maimain_pEditGraph->m_nDrawVolMode = CEditGraph::DRAWVOLMODE_CUSTOM;
	}
}


BOOL C3DGraphToolsWnd::Is3DViewVolsButtonDown(void)
{
	return m_pToolBar->GetToolBarCtrl().GetState(ID_VIEWVOLUMES) & TBSTATE_CHECKED;
}


BOOL C3DGraphToolsWnd::Is3DViewEdgeVolsButtonDown(void)
{
	return m_pToolBar->GetToolBarCtrl().GetState(ID_VIEWEDGEVOLS) & TBSTATE_CHECKED;
}


void C3DGraphToolsWnd::Work(const CMaiUserInput& rMaiUI)
{
	CMaiToolBarWnd::Work(rMaiUI);

	if (m_pCurTool)
	{
		m_pCurTool->UserInput(rMaiUI);
		m_pCurTool->Draw();
	}
}




