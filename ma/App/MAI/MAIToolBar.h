#if !defined(AFX_MAITOOLBAR_H__8318AEDD_3573_4C55_BB91_66CD1BAF95C7__INCLUDED_)
#define AFX_MAITOOLBAR_H__8318AEDD_3573_4C55_BB91_66CD1BAF95C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////////////////////
// MaiToolBar.h - 
//
// Author: Pat MacKellar
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/19/02 Mackellar     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "maimain.h"

class CMaiToolBarWnd;


typedef void (CMaiToolBarWnd::*MaiToolBarButtonFunc)(void);
 
typedef void (CMaiToolBarWnd::**PMaiToolBarButtonFunc)(void);

class CGraphToolsWnd;
class C3DGraphToolsWnd;
class CTestGraphToolsWnd;

//
//
//
// class CMaiToolBarWnd 
//
//
//
class CMaiToolBarWnd : public CMiniFrameWnd
{
// Construction
public:
	CMaiToolBarWnd();
	virtual ~CMaiToolBarWnd();

// Attributes
public:

// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaiToolBarWnd)
	//}}AFX_VIRTUAL

	// Implementation
	virtual BOOL Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect);
	virtual void Work(const CMaiUserInput& rMaiUI);
	virtual int GetNumButtons(void) = 0;   //override this and make it match the number of buttons

	CToolBar	*m_pToolBar;
	int			m_nToolBarResourceId;

	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	// Generated message map functions
protected:
	//{{AFX_MSG(CMaiToolBarWnd)
	//}}AFX_MSG

	u8 *m_paOnDownInputButtons;		//array of button status bytes. One for each button.  0 is up, 1 means just went down
	PMaiToolBarButtonFunc m_paInputButtonFuncs;

	DECLARE_MESSAGE_MAP()
};


//
//
//
// class CMainToolsWnd
//	  -- Selecte between the editing modes
//         graph editor or graph tester
//
//

class CMainToolsWnd : public  CMaiToolBarWnd
{
public:
	CMainToolsWnd();
	virtual ~CMainToolsWnd(void);

	virtual BOOL Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect);
	virtual void Work(const CMaiUserInput& rMaiUI);
	virtual int GetNumButtons(void);   //override this and make it match the number of buttons

	enum 
	{
		TOOLMODE_INVALID = 0,
		TOOLMODE_EDITGRAPH2D,
		TOOLMODE_EDITGRAPH3D,
		TOOLMODE_TESTGRAPH,
		NUM_TOOLMODES
	};

	enum
	{
		NUM_MAINTOOLS_BUTTONS = 3,
	};

	int GetMode(void)								{ return m_nMode;}
	CGraphToolsWnd* Get2DGraphEditToolWnd(void)		{ return (m_nMode == TOOLMODE_EDITGRAPH2D) ? (CGraphToolsWnd*) m_pCurToolWnd: NULL;}
	C3DGraphToolsWnd* Get3DGraphEditToolWnd(void)	{ return (m_nMode == TOOLMODE_EDITGRAPH3D) ? (C3DGraphToolsWnd*) m_pCurToolWnd: NULL;}
	CTestGraphToolsWnd* GetGraphTestToolWnd(void)	{ return (m_nMode == TOOLMODE_TESTGRAPH) ? (CTestGraphToolsWnd*) m_pCurToolWnd: NULL;}
	CMaiToolBarWnd* GetCurToolWnd(void)				{ return m_pCurToolWnd;}

	//{{AFX_VIRTUAL(CMaiToolBarWnd)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMaiToolBarWnd)
	afx_msg void OnGraphTool();
	afx_msg void On3DGraphTool();
	afx_msg void OnGraphTestTool();
	//}}AFX_MSG

	void DoOnGraphTool(void);
	void DoOn3DGraphTool(void);
	void DoOnGraphTestTool(void);


	DECLARE_MESSAGE_MAP()

protected:

	CMaiToolBarWnd *m_pCurToolWnd;
	int m_nMode;
};



//
//
//
// class CGraphToolsWnd
//
//
//

class BaseTool;
class CGraphToolsWnd : public  CMaiToolBarWnd
{
public:
	CGraphToolsWnd();

	virtual BOOL Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect);
	virtual void Work(const CMaiUserInput& rMaiUI);
	virtual int GetNumButtons(void);   //override this and make it match the number of buttons

	//{{AFX_VIRTUAL(CGraphToolsWnd)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGraphToolsWnd)
	afx_msg void OnSelectTool();
	afx_msg void OnMove3DTool();
	afx_msg void OnAddTool();
	afx_msg void OnAddJoinTool();
	afx_msg void OnJoinTool();
	afx_msg void OnUnJoinTool();
	afx_msg void OnPoiTool();
	afx_msg void OnSave();
	afx_msg void OnViewVols();
	afx_msg void OnCalcVols();
	afx_msg void OnAutoJoin();
	afx_msg void OnViewEdgeVols();
	afx_msg void OnViewAllVols();
	afx_msg void OnViewNearVols();
	afx_msg void OnViewCustomVols();
	afx_msg void OnUndo();
	afx_msg void OnManMode();

	//}}AFX_MSG

	void DoOnSelectTool(void);
	void DoOnMove3DTool(void);
	void DoOnAddTool(void);
	void DoOnAddJoinTool(void);
	void DoOnJoinTool(void);
	void DoOnUnJoinTool(void);
	void DoOnPoiTool(void);
	void DoOnSave(void);
	void DoOnViewVols(void);
	void DoOnCalcVols(void);
	void DoOnAutoJoin(void);
	void DoOnViewEdgeVols(void);
	void DoOnUndo(void);
	void DoOnViewAllVols(void);
	void DoOnViewNearVols(void);
	void DoOnViewCustomVols(void);
	void DoOnManMode(void);

	DECLARE_MESSAGE_MAP()

public:
	virtual ~CGraphToolsWnd(void);

	enum
	{
		GRAPHEDITTOOL2D_INVALIDTOOL = -1,
		GRAPHEDITTOOL2D_MOVE2DVERTTOOL,
		GRAPHEDITTOOL2D_MOVE3DVERTTOOL,
		GRAPHEDITTOOL2D_ADDTOOL,
		GRAPHEDITTOOL2D_ADDJOINTOOL,
		GRAPHEDITTOOL2D_JOINTOOL,
		GRAPHEDITTOOL2D_UNJOINTOOL,
		GRAPHEDITTOOL2D_POITOOL,
		GRAPHEDITTOOL2D_NUM_GRAPH_TOOLS,
	};

	enum 
	{
		NUM_GRAPH_TOOLS_BUTTONS = 17,
	};
	int GetCurToolId(void)										{ return m_nCurToolId;}
	BOOL IsViewVolsButtonDown(void);
	BOOL IsViewEdgeVolsButtonDown(void);
	BOOL IsManModeButtonDown(void);
	BaseTool* m_aGraphTools[GRAPHEDITTOOL2D_NUM_GRAPH_TOOLS];
	BaseTool* m_pCurTool;
	s32 m_nCurToolId;

	BOOL m_bCalcingVols;
	u16 m_bCalcingVolsCurrent;
	u16 m_bCalcingVolsMax;
	BOOL m_bManModeOn;
};


//
//
//
// class CTestGraphToolsWnd
//
//
//
#include "MaiTestGraphTool.h"

class CTestGraphToolsWnd : public  CMaiToolBarWnd
{
public:
	CTestGraphToolsWnd ();

	virtual BOOL Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect);
	virtual void Work(const CMaiUserInput& rMaiUI);
	virtual int GetNumButtons(void);   //override this and make it match the number of buttons

	//{{AFX_VIRTUAL(CTestGraphToolsWnd )
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTestGraphToolsWnd )
	afx_msg void OnOriginButton();
	afx_msg void OnGoalButton();
	afx_msg void OnConfigButton();
	afx_msg void OnFindButton();
	//}}AFX_MSG

	void DoOnOriginButton();
	void DoOnGoalButton();
	void DoOnConfigButton();
	void DoOnFindButton();

	DECLARE_MESSAGE_MAP()

	enum
	{
		NUM_TESTGRAPH_BUTTONS = 4,
	};

public:
	virtual ~CTestGraphToolsWnd (void);
	TestGraphTool m_TestGraphTool;
};

//
//
//
// class CGraphToolsWnd
//
//
//

class C3DGraphToolsWnd : public  CMaiToolBarWnd
{
public:
	C3DGraphToolsWnd();

	virtual BOOL Init(int nToolBarResourceId, const char * pszTitle, CWnd* pParendWnd, const CRect &initRect);
	virtual void Work(const CMaiUserInput& rMaiUI);
	virtual int GetNumButtons(void);   //override this and make it match the number of buttons

	//{{AFX_VIRTUAL(C3DGraphToolsWnd)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(C3DGraphToolsWnd)
	afx_msg void On3DMove3DTool();
	afx_msg void On3DAddJoinTool();
	afx_msg void On3DJoinTool();
	afx_msg void On3DUnJoinTool();
	afx_msg void On3DSave();
	afx_msg void On3DViewVols();
	afx_msg void On3DViewEdgeVols();
	afx_msg void On3DViewAllVols();
	afx_msg void On3DViewNearVols();
	afx_msg void On3DViewCustomVols();
	afx_msg void On3DUndo();

	//}}AFX_MSG

	void DoOn3DMove3DTool(void);
	void DoOn3DAddJoinTool(void);
	void DoOn3DJoinTool(void);
	void DoOn3DUnJoinTool(void);
	void DoOn3DSave(void);
	void DoOn3DViewVols(void);
	void DoOn3DViewEdgeVols(void);
	void DoOn3DUndo(void);
	void DoOn3DViewAllVols(void);
	void DoOn3DViewNearVols(void);
	void DoOn3DViewCustomVols(void);

	DECLARE_MESSAGE_MAP()

public:
	virtual ~C3DGraphToolsWnd(void);

	enum
	{
		GRAPHTOOL_3DMOVETOOL = 0,
		GRAPHTOOL_3DADDJOINTOOL,
		GRAPHTOOL_3DJOINTOOL,
		GRAPHTOOL_3DUNJOINTOOL,
		NUM_GRAPHTOOLS,
	};


	enum 
	{
		NUM_3DGRAPH_TOOLS_BUTTONS = 11,
	};

	BOOL Is3DViewVolsButtonDown(void);
	BOOL Is3DViewEdgeVolsButtonDown(void);
	BaseTool* m_aGraphTools[NUM_GRAPHTOOLS];
	BaseTool* m_pCurTool;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAITOOLBAR_H__8318AEDD_3573_4C55_BB91_66CD1BAF95C7__INCLUDED_)
