#include "stdafx.h"
#include "fang.h"
#include "flinklist.h"
#include "fdraw.h"
#include "frenderer.h"
#include "frendersort.h"
#include "fviewport.h"
#include "fxfm.h"
#include "fvid.h"
#include "ftext.h"
#include "fworld.h"
#include "fresload.h"
#include "fworld_coll.h"
#include "AI/AIGraph.h"
#include "AI/AIUtil.h"
#include "AI/AIGraphSearcher.h"
#include "AI/ftl.h"
#include "AI/apenew.h"
#include "MaiMain.h"
#include "MaiEditGraph.h"
#include "MaiToolBar.h"
#include "MaiUndo.h"
#include "MaiQueryDetails.h"
#include "MaiPoiTool.h"
#include "Settings.h"
#include "resource.h"
#if FANG_PLATFORM_WIN
	#include "dx/fdx8vid.h"
#endif


// Make and Keep up to eight backup copies
#define MAX_LEN_FILENAME 256
#define NUM_COPIES_TO_KEEP 8
#define FILE_EXISTS 0
#define FILE_CANWRITE 02
#define YEPITWORKED 0
#include <io.h>


extern CFVec3 _WorldCamPos;
extern f32 _fCamPitch;
#define NIPTR_NODE_POOL_SIZE 3000
FLinkRoot_t		 maimain_NodePool;
void*			 _pNiPtrNodePooMem = NULL;
FLinkRoot_t		 *maimain_pNodePool = NULL;


#define PQ_NODE_POOL_SIZE 200
FLinkRoot_t		 maimain_PQNodePool;
void			*maimain_pPQNodePoolMem  = NULL;
FLinkRoot_t		*maimain_pPQNodePool = NULL;

#define MAX_NUM_SPLINES 50
CFWorldShapeSpline* maimain_apSpline[MAX_NUM_SPLINES];
u32 maimain_uNumSplines  = 0;

// globals
CEditGraph		*maimain_pEditGraph = NULL;
CGraphSearcher	*maimain_pDebugGraphSearcher = NULL;
CMainToolsWnd	*maimain_pMainToolBarWindow = NULL;
f32 maimain_fMouseRayLength = 200.0f;
CFVec3A maimain_MouseRayOrigin;
CFVec3A maimain_MouseRayEnd;
CFVec3A maimain_MouseRayImpactPt;
CFVec3A maimain_MouseRayImpactPtPrevious;
BOOL maimain_bMouseRayImpactThisFrame = FALSE;
CMainToolsWnd *m_toolBarWindow;
u32 maimain_bLargeMode = FALSE;

CNiList<CQueryDetails*> *aimain_pQueryDetailsList = NULL;
CQueryDetails _DefaultQueryDetails;

const char* _kpszDefaultConfigName = "DEFAULT";


const char *maimain_pszGraphFilePath = NULL;
const char *maimain_pszGraphFileName = NULL;
char maimain_szGraphFileFullPath[256];

#define MAX_NUM_DEBUGRAYS 50
enum
{
	DEBUGRAYRGB_MAGENTA = 0,
	DEBUGRAYRGB_GREEN,
	DEBUGRAYRGB_RED,
	DEBUGRAYRGB_WHITE,
	NUM_DEBUGRAYRGBS,
};
#define MAX_NUM_DEBUGCOLORS 4

CFVec3A _aDrawDebugRayOrigin[MAX_NUM_DEBUGRAYS];
CFVec3A _aDrawDebugRayEnd[MAX_NUM_DEBUGRAYS];
s32 _aDrawDebugRayRGBIndex[MAX_NUM_DEBUGRAYS];
s32 _nDrawDebugRays = 0;

CFColorRGBA _aDrawDebugRayRGB[MAX_NUM_DEBUGCOLORS] = 
{
	CFColorRGBA(1.0f, 0.0f, 1.0f, 1.0f),
	CFColorRGBA(0.0f, 1.0f, 0.0f, 1.0f),
	CFColorRGBA(1.0f, .0f, 0.0f, 1.0f),
	CFColorRGBA(1.0f, .0f, 0.0f, 1.0f)
};

#define MAIMAIN_COLOROFF		(0)
#define MAIMAIN_COLORRED		(1)
#define MAIMAIN_COLORGREEN		(2)
#define MAIMAIN_COLORBLUE		(3)
#define MAIMAIN_COLORYELLOW	(4)
#define MAIMAIN_COLORWHITE		(5)
#define MAIMAIN_COLORMAGENTA	(6)

static CFColorRGBA _aSplineRgb[7];


void _ResetDebugRays(void)
{
	_nDrawDebugRays = 0;
}

BOOL _AddDebugRay(const CFVec3A& Origin, const CFVec3A& End, s32 nRGBIndex)
{
	if (_nDrawDebugRays	< MAX_NUM_DEBUGRAYS)
	{
	   _aDrawDebugRayOrigin[_nDrawDebugRays] = Origin;
	   _aDrawDebugRayEnd[_nDrawDebugRays] = End;
	   _aDrawDebugRayRGBIndex[_nDrawDebugRays] = nRGBIndex;
	   return TRUE;
	}
	return FALSE;
}

void _RenderDebugRays(void)
{
	for (int i = 0; i < _nDrawDebugRays; i++)
	{
		fdraw_SolidLine(	&(_aDrawDebugRayOrigin[i].v3),
							&(_aDrawDebugRayEnd[i].v3),
							&(_aDrawDebugRayRGB[_aDrawDebugRayRGBIndex[i]]));
	}
}

BOOL maimain_InitSystem(CWnd* pMainToolBarParentWnd, const char* pszGraphFilePath, const char* pszGraphFileName)
{
	maimain_pszGraphFilePath = pszGraphFilePath;
	maimain_pszGraphFileName = pszGraphFileName;

	strcpy(maimain_szGraphFileFullPath, maimain_pszGraphFilePath);
	strcat(maimain_szGraphFileFullPath, "\\");
	strcat(maimain_szGraphFileFullPath, maimain_pszGraphFileName);

	maiundo_Init();

	//InitNodePools
	int nNiPtrNodeSize = sizeof(CNiNode<int>);
	_pNiPtrNodePooMem = fres_Alloc(NIPTR_NODE_POOL_SIZE * nNiPtrNodeSize );
	flinklist_InitRoot(&maimain_NodePool, 0);
	flinklist_InitPool(&maimain_NodePool, _pNiPtrNodePooMem , nNiPtrNodeSize, NIPTR_NODE_POOL_SIZE);
	maimain_pNodePool = &maimain_NodePool;

	// initialize the PriorityQ Node Pool 
	s32 nPQNodeSize = sizeof(CNiKeyNode_Float<void*>);
	maimain_pPQNodePoolMem = fres_Alloc(NIPTR_NODE_POOL_SIZE * nPQNodeSize );
	flinklist_InitRoot(&maimain_PQNodePool, 0);
	flinklist_InitPool(&maimain_PQNodePool, maimain_pPQNodePoolMem, nPQNodeSize, NIPTR_NODE_POOL_SIZE);
	maimain_pPQNodePool = &maimain_PQNodePool;


	// code to load ascii version of graph files
	CAIPath::InitSystem(maimain_pNodePool);
	maimain_pEditGraph = APE_NEW CEditGraph();
	FASSERT(maimain_pEditGraph);

	BOOL bGraphExists = 0==_access(maimain_szGraphFileFullPath, FILE_EXISTS);
	BOOL bCanOverwrite = 0==_access(maimain_szGraphFileFullPath, FILE_CANWRITE);
	BOOL bReadOnlyMode = FALSE;
	int nMessageBoxReturnVal;
	char szTmp[256];
	if (bGraphExists && !bCanOverwrite)  
	{
		wsprintf(szTmp, "\n\n         Error Opening File: %s       \n\nThe AIGraph file exits, but can't be overwritten!!!\nProbably it isn't checked out.\n", maimain_szGraphFileFullPath);
		nMessageBoxReturnVal = MessageBox(NULL, szTmp, "File Access Error", MB_ABORTRETRYIGNORE);
		while (bGraphExists && !bCanOverwrite && nMessageBoxReturnVal == IDRETRY)
		{
			bGraphExists = 0==_access(maimain_szGraphFileFullPath, FILE_EXISTS);
			bCanOverwrite = 0==_access(maimain_szGraphFileFullPath, FILE_CANWRITE);
			nMessageBoxReturnVal = MessageBox(NULL, szTmp, "File Access Error", MB_ABORTRETRYIGNORE);
		}
		if (nMessageBoxReturnVal == IDIGNORE)
		{
			bReadOnlyMode = TRUE;
		}
	}

	if (bGraphExists && !bCanOverwrite && !bReadOnlyMode)
	{
		return FALSE; //couldn't initialize MAI.exe error loading file
	}

	if (!maimain_pEditGraph->LoadFromAsciiFile(maimain_szGraphFileFullPath))
	{
		wsprintf(szTmp, "Starting a new AI Graphy. File: %s\n", maimain_szGraphFileFullPath);
		MessageBox(NULL, szTmp , "Info", MB_OK);
		maimain_pEditGraph->StartNewGraph();
	}
	else
	{
//		maimain_pEditGraph->TestEdgesWalkability();
	}

	
	//GUI stuff 
	// - toolbars, dialogs, etc
	CPoiTool::InitSystem();

	maimain_pMainToolBarWindow = new CMainToolsWnd();

	maimain_pMainToolBarWindow->Init(IDR_MAITOOLBAR, "MAI Editor Modes", pMainToolBarParentWnd, CRect(20/*+FVid_Mode.nPixelsAcross*/,20+FVid_Mode.nPixelsDown,30,45+FVid_Mode.nPixelsDown));
	
	if (maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() && maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->IsViewVolsButtonDown())
	{
		maimain_pEditGraph->m_bDrawVertVolumes = 1;
	}
	else
	{
		maimain_pEditGraph->m_bDrawVertVolumes = 0;
	}

	if (maimain_pMainToolBarWindow->Get2DGraphEditToolWnd() && maimain_pMainToolBarWindow->Get2DGraphEditToolWnd()->IsViewEdgeVolsButtonDown())
	{
		maimain_pEditGraph->m_bDrawEdgeVolumes = 1;
	}
	else
	{
		maimain_pEditGraph->m_bDrawEdgeVolumes = 0;
	}
	

	aimain_pQueryDetailsList = APE_NEW CNiList<CQueryDetails*>();
	_DefaultQueryDetails.Init(_kpszDefaultConfigName, 2.0f, 3.0f, CQueryDetails::CAN_JUMP, CQueryDetails::IS_SURFACE_MOVER, !CQueryDetails::IS_3D_MOVER);
	aimain_pQueryDetailsList->PushHead(&_DefaultQueryDetails);

	CSettings& Settings = CSettings::GetCurrent();
	CSettings::SetApplicationName( "Mai" );
	s32 nCount = 0;
	const char* pszSectionName = "QUERYDETAILS";
	nCount = Settings.ReadCustomInt(pszSectionName, "COUNT", 0);
		
	for (int i = 0; i < nCount; i++)
	{
		CQueryDetails* pDetails = new CQueryDetails();

		char szTmp[64];
		sprintf(szTmp, "NAME%02d", i);
		char szDetailName[128];
		Settings.ReadCustomString(pszSectionName, szTmp, "EHH", szDetailName, 128);
		sprintf(szTmp, "WIDTH%02d", i);
		pDetails->CopyName(szDetailName);
		pDetails->m_fWidth = Settings.ReadCustomFloat(pszSectionName, szTmp, 0.0f);
		sprintf(szTmp, "HEIGHT%02d", i);
		pDetails->m_fHeight = Settings.ReadCustomFloat(pszSectionName, szTmp, 0.0f);
		sprintf(szTmp, "CANJUMP%02d", i);
		pDetails->m_bCanJump = Settings.ReadCustomInt(pszSectionName, szTmp, 0);
		sprintf(szTmp, "SURFACEMOVER%02d", i);
		pDetails->m_bSurfaceMover = Settings.ReadCustomInt(pszSectionName, szTmp, 0);
		sprintf(szTmp, "3DMOVER%02d", i);
		pDetails->m_b3DMover = Settings.ReadCustomInt(pszSectionName, szTmp, 0);
		aimain_pQueryDetailsList->PushTail(pDetails);
	}
	_aSplineRgb[0] = FColor_MotifWhite;
	_aSplineRgb[1] = FColor_MotifRed,
	_aSplineRgb[2] = FColor_MotifGreen,
	_aSplineRgb[3] = FColor_MotifBlue,
	_aSplineRgb[4] = FColor_MotifBlue,
	_aSplineRgb[5] = FColor_MotifWhite,
	_aSplineRgb[6] = FColor_MotifWhite,
	_aSplineRgb[MAIMAIN_COLORYELLOW].SetColor(1.0f, 1.0f, 0.0f);
	_aSplineRgb[MAIMAIN_COLORMAGENTA].SetColor(1.0f, 0.0f, 1.0f);



/*	//test code to test load/save routines in aigraph

//	maimain_pEditGraph->SaveToAsciiFile("c:\\graph2.gt");
											 
	maimain_pEditGraph->SaveToBinaryFile("c:\\graph2.gb");

	CAIGraph *maimain_pDebugGraphQuickLoad = NULL;

	FILE* fp = fopen("c:\\graph2.gb", "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		void* pFileData = fres_AlignedAlloc(fileLen, 32);
		fread(pFileData, fileLen, 1, fp);
		fclose(fp);
		maimain_pDebugGraphQuickLoad = (CAIGraph*) pFileData;
		maimain_pDebugGraphQuickLoad->PointerizeBinaryData();
	}


  */

	return TRUE;
}


BOOL maimain_UninitSystem(void)
{
	APE_DELETE(maimain_pDebugGraphSearcher); maimain_pDebugGraphSearcher = NULL;
	APE_DELETE(maimain_pEditGraph); maimain_pEditGraph = NULL;

	maimain_uNumSplines = 0;

	if (maimain_pMainToolBarWindow)
	{
		maimain_pMainToolBarWindow->DestroyWindow();
	}
	maimain_pMainToolBarWindow = NULL;

	if (aimain_pQueryDetailsList)
	{
		CSettings& Settings = CSettings::GetCurrent();
		CSettings::SetApplicationName( "Mai" );
		if (aimain_pQueryDetailsList->Size() > 1)
		{
			const char* pszSectionName = "QUERYDETAILS";
			Settings.WriteCustomInt(pszSectionName, "COUNT", aimain_pQueryDetailsList->Size()-1);
			
			CNiIterator<CQueryDetails*> it = aimain_pQueryDetailsList->Begin();
			it.Next();
			s32 nCount = 0;
			while (it.IsValid())
			{
				char szTmp[64];
				CQueryDetails* pDetails = it.Get();
				sprintf(szTmp, "NAME%02d", nCount);
				Settings.WriteCustomString(pszSectionName, szTmp, pDetails->GetName());
				sprintf(szTmp, "WIDTH%02d", nCount);
				Settings.WriteCustomFloat(pszSectionName, szTmp, pDetails->m_fWidth);
				sprintf(szTmp, "HEIGHT%02d", nCount);
				Settings.WriteCustomFloat(pszSectionName, szTmp, pDetails->m_fHeight);
				sprintf(szTmp, "CANJUMP%02d", nCount);
				Settings.WriteCustomInt(pszSectionName, szTmp, pDetails->m_bCanJump);
				sprintf(szTmp, "SURFACEMOVER%02d", nCount);
				Settings.WriteCustomInt(pszSectionName, szTmp, pDetails->m_bSurfaceMover);
				sprintf(szTmp, "3DMOVER%02d", nCount);
				Settings.WriteCustomInt(pszSectionName, szTmp, pDetails->m_b3DMover);

				delete pDetails;

				it.Next();
				nCount++;
			}
		}
		else
		{
			Settings.WriteCustomInt("QUERYDETAILS", "count", 0);
		}

		aimain_pQueryDetailsList->RemoveAll();
		APE_DELETE(aimain_pQueryDetailsList);
		aimain_pQueryDetailsList = NULL;
	}
	
	CPoiTool::UninitSystem();

	maiundo_Uninit();

	CAIPath::UninitSystem();

	if (_pNiPtrNodePooMem)
	{
//		fres_Free(_pNiPtrNodePooMem);
	}
	_pNiPtrNodePooMem = NULL;

	if (maimain_pPQNodePoolMem)
	{
//		fres_free(maimain_pPQNodePoolMem);
	}
	maimain_pPQNodePool = NULL;
	return TRUE;
}


s32 _searcherCycles = 0;


void maimain_Work(const CMaiUserInput& rMaiUI)
{
	if (maimain_GetKeyState( 'L' ) & 1)  //on down
	{			//Large Mode Toggle
		maimain_bLargeMode = 1-maimain_bLargeMode;

		if (maimain_bLargeMode)
		{
			aigraph_RenderLODDist01 = 700.0f;
			aigraph_RenderLODDist02 = 900.0f;
			aigraph_fVertVisualRadius = 4.0f;
			maimain_fMouseRayLength = 500.0f;
			maimain_pEditGraph->m_fVertVisualRadius = aigraph_fVertVisualRadius;
		}
		else
		{
			aigraph_RenderLODDist01 = 100.0f;
			aigraph_RenderLODDist02 = 250.0f;
			aigraph_fVertVisualRadius = 2.0f;
			maimain_fMouseRayLength = 200.0f;
			maimain_pEditGraph->m_fVertVisualRadius = aigraph_fVertVisualRadius;
		}
	}
	
	if (maimain_GetKeyState( 'M' ) & 1)  //on down
	{ //Mesh Render Mode Toggle
		FRS_bRenderFlags ^= (FRS_RENDER_OBJECT_GEO);
	}
	
	if (maimain_GetKeyState( 'N' ) & 1)  //on down
	{ //Mesh Render Mode Toggle
		FRS_bRenderFlags ^= (FRS_RENDER_PASS_SURFACE|FRS_RENDER_SHADOWS|FRS_RENDER_TRANSLUCENCIES|FRS_RENDER_PASS_SPECULAR);
	}

	_searcherCycles = 0;
	if (maimain_pDebugGraphSearcher)
	{
		maimain_pDebugGraphSearcher->Work(&_searcherCycles);
	}

}


u32 _uCurProblemVert = 0;
void maimain_ProblemMenuWork(const CMaiUserInput& rMaiUI)
{
	f32 fLineHeight = 0.03f;
	f32 fMenuT = 0.2f;
	f32 fMenuL = 0.2f;
	f32 fMenuCurY = fMenuT;
	cchar* apszColor[2] = 
	{
		"~c99000099",
		"~c99787899"
	};
	
	if (maimain_pEditGraph->m_uNumProblemVerts)
	{
		ftext_DebugPrintf( fMenuL, fMenuCurY, "~w1~s2.00~c99000099Problem Verts %d, ('P') to cycle", maimain_pEditGraph->m_uNumProblemVerts);
		fMenuCurY += fLineHeight;

		for (u32 i = 0; i < maimain_pEditGraph->m_uNumProblemVerts; i++)
		{
			if (maimain_pEditGraph->m_auProblemVerts[i].m_uProblemType != CAIGraph::VERTPROBLEMTYPE_NONE)
			{
				ftext_DebugPrintf( fMenuL, fMenuCurY, "~w1~s2.00%s%d,%s", apszColor[i==_uCurProblemVert], maimain_pEditGraph->m_auProblemVerts[i].m_uVertId, CAIGraph::s_pszProblemTypeLabels[maimain_pEditGraph->m_auProblemVerts[i].m_uProblemType]);
				fMenuCurY += fLineHeight;
			}
		}

		if (maimain_GetKeyState( 'P' ) & 1)  //on down
		{
			_uCurProblemVert++;
			if (_uCurProblemVert >= maimain_pEditGraph->m_uNumProblemVerts)
			{
			   _uCurProblemVert = 0;
			}
			_WorldCamPos = maimain_pEditGraph->GetVert(maimain_pEditGraph->m_auProblemVerts[_uCurProblemVert].m_uVertId)->m_Location.v3;
			_WorldCamPos.y+=50.0f;
			_fCamPitch = FMATH_HALF_PI;
		}
	}

}

u32 _nDisplayLastMouseXOnDown;
u32 _nDisplayLastMouseYOnDown;
void maimain_Draw(const CMaiUserInput& rMaiUI)
{
	//RENDER
	frenderer_Push( FRENDERER_DRAW, NULL );

	
	//UI
	CFVec3A Vec_WS;
	CFVec2 Vec_SS;
	CFVec3A Vec_VS;

	//
	// Special code to convert screen space into world space ray
	//
	//
	FViewport_t * pView = fviewport_GetActive( );
	if (pView)
	{
		Vec_SS.x = rMaiUI.nLastMouseX * pView->OOHalfRes.x - 1.0f;
		Vec_SS.y = (pView->Res.y - rMaiUI.nLastMouseY) * pView->OOHalfRes.y - 1.0f;

		Vec_VS.x = pView->fTanHalfFOVX*Vec_SS.x*0.9991f;
		Vec_VS.y = pView->fTanHalfFOVY*Vec_SS.y*0.9991f;
		Vec_VS.z = 0.9991f;
//		Vec_VS.x = pView->fTanHalfFOVX*Vec_SS.x*0.001f;
//		Vec_VS.y = pView->fTanHalfFOVY*Vec_SS.y*0.001f;
//		Vec_VS.z = 0.001f;

		FXfm_pView->m_MtxR.MulPoint(Vec_WS,Vec_VS);
		int r= 2;

		if (rMaiUI.bLastLeftOnDown)
		{
			_nDisplayLastMouseXOnDown = rMaiUI.nLastMouseX;
			_nDisplayLastMouseYOnDown = rMaiUI.nLastMouseY;
		}

	}

	CFVec3 rayDir = Vec_WS.v3 - FXfm_pView->m_MtxR.m_vPos.v3;
	rayDir.Unitize();
	maimain_MouseRayEnd.Set(rayDir*maimain_fMouseRayLength + FXfm_pView->m_MtxR.m_vPos.v3);
	maimain_MouseRayOrigin.Set(FXfm_pView->m_MtxR.m_vPos.v3);

	FCollImpact_t oCollInfo;

	if (maimain_bMouseRayImpactThisFrame)
	{
		maimain_MouseRayImpactPtPrevious = maimain_MouseRayImpactPt;
	}

	if (fworld_FindClosestImpactPointToRayStart(&oCollInfo, &maimain_MouseRayOrigin, &maimain_MouseRayEnd))
	{
		maimain_MouseRayImpactPt.Set(oCollInfo.UnitFaceNormal);
		maimain_MouseRayImpactPt.Mul(aigraph_kfSurfaceOffset);
		maimain_MouseRayImpactPt.Add( oCollInfo.ImpactPoint );
		maimain_bMouseRayImpactThisFrame = TRUE;
	}
	else
	{
		maimain_MouseRayImpactPt = maimain_MouseRayEnd;		
		maimain_bMouseRayImpactThisFrame = FALSE;
	}

//	fdraw_SolidLine(&(maimain_MouseRayOrigin.v3), &(maimain_MouseRayEnd.v3));
	fdraw_FacetedWireSphere( &(maimain_MouseRayEnd.v3), 2.0f, 2, 2, &(_aDrawDebugRayRGB[DEBUGRAYRGB_MAGENTA]));
	fdraw_FacetedWireSphere( &(maimain_MouseRayImpactPt.v3), 0.25f, 1, 1, &(_aDrawDebugRayRGB[DEBUGRAYRGB_WHITE]));



	maimain_ProblemMenuWork(rMaiUI);

	if (maimain_pMainToolBarWindow)
	{
		maimain_pMainToolBarWindow->Work(rMaiUI);
	}


	maimain_pEditGraph->Render();

	maimain_RenderSplines();

	if (maimain_bLargeMode)
	{
		ftext_DebugPrintf( 0.9f, 0.150f, "~w1Large Mode");
	}

	u32 uEdgeSize = sizeof(GraphEdge);
	ftext_DebugPrintf( 0.5f, 0.01f, "~w1Num Verts = %d", maimain_pEditGraph->GetNumVerts());
	ftext_DebugPrintf( 0.5f, 0.02f, "~w1Memory = %d", maimain_pEditGraph->GetNumVerts()*sizeof(GraphVert));

	u32 uHoles = 0;
	for (u32 i = 1; i < maimain_pEditGraph->GetNumVerts(); i++)
	{
		if (!maimain_pEditGraph->IsVertValid(i))
		{
			uHoles++;
		}
	}
	ftext_DebugPrintf( 0.5f, 0.03f, "~w1Holes = %d", uHoles);

	/*

	ftext_DebugPrintf( 0.9f, 0.150f, "~arX: ~w1%6u", rMaiUI.nLastMouseX);
	ftext_DebugPrintf( 0.9f, 0.170f, "~arY: ~w1%6u", rMaiUI.nLastMouseY);

//	ftext_DebugPrintf( 0.9f, 0.450f, "~arDX: ~w1%02d", rMaiUI.nDeltaMouseX);
//	ftext_DebugPrintf( 0.9f, 0.470f, "~arDY: ~w1%02d", rMaiUI.nDeltaMouseY);

	ftext_DebugPrintf( 0.9f, 0.250f, "~arSS.x: ~w1%f", Vec_SS.x);
	ftext_DebugPrintf( 0.9f, 0.270f, "~arSS.y: ~w1%f", Vec_SS.y);

	ftext_DebugPrintf( 0.9f, 0.350f, "~arVS_X: ~w1%f", Vec_VS.x);
	ftext_DebugPrintf( 0.9f, 0.370f, "~arVS_Y: ~w1%f", Vec_VS.y);
	ftext_DebugPrintf( 0.9f, 0.390f, "~arVS_Z: ~w1%f", Vec_VS.z);

	ftext_DebugPrintf( 0.9f, 0.450f, "~arWS_X: ~w1%f", Vec_WS.x);
	ftext_DebugPrintf( 0.9f, 0.470f, "~arWS_Y: ~w1%f", Vec_WS.y);
	ftext_DebugPrintf( 0.9f, 0.490f, "~arWS_Z: ~w1%f", Vec_WS.z);
*/
	_RenderDebugRays();

	
	frenderer_Pop();


}



s32 maimain_MakeBackupCopy(const char* pszSrcFileFullPath, const char* pszBackupDir)
{
	char szSrcFileFullPath[MAX_LEN_FILENAME];
	char szBackupFilePathSrc[MAX_LEN_FILENAME];
	char szBackupFilePathDst[MAX_LEN_FILENAME];

	FASSERT(pszSrcFileFullPath && strlen(pszSrcFileFullPath) < MAX_LEN_FILENAME-strlen("BACK"));

	strcpy(szSrcFileFullPath, pszSrcFileFullPath);
	char *pEx = strrchr(szSrcFileFullPath, '.');
	if (pEx)
	{
		char *pszSrcFile = strrchr(szSrcFileFullPath, '\\');
		if (pszSrcFile && ++pszSrcFile)
		{
			*pEx = '\0';
			pEx++;
			for (int i = (NUM_COPIES_TO_KEEP-1); i > 0; i--)
			{
				sprintf(szBackupFilePathSrc, "%s\\%s.BACK%02d.%s", pszBackupDir, pszSrcFile, i, pEx);

				sprintf(szBackupFilePathDst, "%s\\%s.BACK%02d.%s", pszBackupDir, pszSrcFile, i+1, pEx);
				
				if (_access(szBackupFilePathSrc, FILE_EXISTS) == YEPITWORKED)  //0 param means to check for existence only, 0 results mean yes!
				{
					CopyFile(szBackupFilePathSrc, szBackupFilePathDst, FALSE);
				}
			}
			//now backup source
			sprintf(szBackupFilePathDst, "%s\\%s.BACK%02d.%s", pszBackupDir, pszSrcFile, 1, pEx);
			
			if (_access(pszSrcFileFullPath, FILE_EXISTS) == YEPITWORKED)   //is there anything to backup?
			{
				return CopyFile(pszSrcFileFullPath, szBackupFilePathDst, FALSE) !=0 ;
			}
		}
	}
	return 0;
}


BOOL maimain_GetCamPos(CFVec3A* pCamPos)
{
	FViewport_t * pView = fviewport_GetActive( );
	if (pView)
	{
		if (pCamPos)
		{
			(*pCamPos).Set(FXfm_pView->m_MtxR.m_vPos);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL maimain_GetCamLookAt(CFVec3A* pCamLookAt)
{
	FViewport_t * pView = fviewport_GetActive( );
	if (pView)
	{
		if (pCamLookAt)
		{
			CFVec3A tmpAxisZ = CFVec3A::m_UnitAxisZ;
			(*pCamLookAt) = FXfm_pView->m_MtxR.MulDir(tmpAxisZ);
			(*pCamLookAt).Unitize();
			return TRUE;
		}
	}
	return FALSE;
}


BOOL _CreateWorldShape( cchar *pszWorldResName, CFWorldShapeInit *pShapeInit, const void *pFixupOffsetBase )
{
	const CFWorldShapeMesh *pShapeMesh = pShapeInit->m_pMesh;
	BOOL bBuilt = FALSE;

	FResFrame_t ResFrame = fres_GetFrame();

	if (pShapeInit->m_nShapeType == FWORLD_SHAPETYPE_SPLINE)
	{
			// CESpline object...

			bBuilt = TRUE;
			if (maimain_uNumSplines < MAX_NUM_SPLINES)
			{
				maimain_apSpline[maimain_uNumSplines] = APE_NEW CFWorldShapeSpline();
				*(maimain_apSpline[maimain_uNumSplines]) = *pShapeInit->m_pSpline;
				if (maimain_apSpline[maimain_uNumSplines])
				{
					maimain_apSpline[maimain_uNumSplines]->m_pPtArray = APE_NEW CFVec3[maimain_apSpline[maimain_uNumSplines]->m_nPointCount];
					if (maimain_apSpline[maimain_uNumSplines]->m_pPtArray)
					{
						fang_MemCopy(maimain_apSpline[maimain_uNumSplines]->m_pPtArray, pShapeInit->m_pSpline->m_pPtArray, sizeof(CFVec3)*pShapeInit->m_pSpline->m_nPointCount);
					}
				}
				maimain_uNumSplines++;
			}
//			pEntity = fnew CESpline;
//			_SET_ENTITY_TYPE( CESpline, "CESpline" );
			if (!bBuilt)
			{
				goto _ExitWithError;
			}
	}

	if (bBuilt)
	{

		// flag as stored...
		pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_COUNT;
	}

	return bBuilt;

	// Error:
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return NULL;
}



BOOL maimain_WorldShapeCreateCallback( cchar *pszWorldResName, const void *pFixupOffsetBase, const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount )
{
	const CFWorldShapeInit *pShapeInit;
	u32 i;
	FMemFrame_t MemFrame = fmem_GetFrame();
	CFWorldMesh *pWorldMesh;

	for( i=0, pShapeInit=pShapeInitArray; i<nShapeInitCount; i++, pShapeInit++ )
	{

		if( (pShapeInit->m_pGameData==NULL && pShapeInit->m_nParentShapeIndex<0) || !_CreateWorldShape(pszWorldResName, (CFWorldShapeInit *)pShapeInit, pFixupOffsetBase ))
		{
			if( pShapeInit->m_nShapeType==FWORLD_SHAPETYPE_MESH && pShapeInit->m_pMesh->m_pszMeshName ) {
				const CFWorldShapeMesh *pShapeMesh = pShapeInit->m_pMesh;

				FMeshInit_t MeshInit;

				MeshInit.Mtx.Set( pShapeInit->m_Mtx43 );
				MeshInit.fCullDist = fmath_Sqrt( pShapeMesh->m_fCullDist2 );
				MeshInit.nFlags = pShapeMesh->m_nMeshInstFlags;

				MeshInit.pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pShapeMesh->m_pszMeshName );
				if( MeshInit.pMesh == NULL ) {
					// Mesh resource not found...
					DEVPRINTF( "CEntity::_WorldShapeCreateCallback(): Could not load mesh '%s'.\n", pShapeMesh->m_pszMeshName );
					continue;
				}

				pWorldMesh = fnew CFWorldMesh;
				if( pWorldMesh == NULL ) {
					DEVPRINTF( "CEntity::_WorldShapeCreateCallback(): Could not allocate memory for all CFWorldMesh objects.\n" );
					break;
				}

				pWorldMesh->Init( &MeshInit );
				pWorldMesh->SetCollisionFlag( !(pWorldMesh->m_nFlags & FMESHINST_FLAG_NOCOLLIDE) );
				if ( pShapeMesh->m_papszLightMapName[0] )
				{
					// This object has instance lightmaps
					pWorldMesh->SetLightmaps( (cchar **)pShapeMesh->m_papszLightMapName, pShapeMesh->m_anLightMapMotif, FWORLD_MAX_MESH_SHAPE_LIGHTMAPS );
				}
			}

		}
	}
	return TRUE;
}


u32 maimain_GetNumSplines(void)
{
	return maimain_uNumSplines;
}

void maimain_RenderSplineKnot(CFVec3* pLoc, u32 uColor)
{
	s32 nRes = 0;
	fdraw_FacetedWireSphere( pLoc, 2.0f, 1+nRes, 1+nRes, _aSplineRgb+uColor);
}

void maimain_RenderSpline(u32 uId)
{
	FASSERT(maimain_apSpline[uId]);

	CFWorldShapeSpline* pSpline = maimain_apSpline[uId];

	maimain_RenderSplineKnot( pSpline->m_pPtArray+0, MAIMAIN_COLORRED);
	for (u32 i = 1; i < pSpline->m_nPointCount; i++)
	{
		fdraw_SolidLine(pSpline->m_pPtArray+(i-1), pSpline->m_pPtArray+i, _aSplineRgb+MAIMAIN_COLORRED);
		maimain_RenderSplineKnot( pSpline->m_pPtArray+i, MAIMAIN_COLORRED);
	}

	if (pSpline->m_bClosedSpline && i > 2)
	{
		fdraw_SolidLine(pSpline->m_pPtArray+(i-1), pSpline->m_pPtArray, _aSplineRgb+MAIMAIN_COLORRED);
	}


}

void maimain_RenderSplines(void)
{
	for (u32 i = 0; i < maimain_uNumSplines; i++)
	{
		maimain_RenderSpline(i);
	}
}


short maimain_GetKeyState(int vKey)
{
	return GetAsyncKeyState(vKey);
}
