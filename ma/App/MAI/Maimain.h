//////////////////////////////////////////////////////////////////////////////////////
// MAIMain.h - AI Module interface
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
// 02/13/02 patm       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAIMAIN_H_
#define _MAIMAIN_H_ 1

class CMainToolsWnd;

#include "fmath.h"
#include "MaiQueryDetails.h"

extern BOOL maimain_InitSystem(CWnd* pMainToolBarParentWnd, const char* pszGraphFilePath, const char* pszGraphFileName);
extern BOOL maimain_UninitSystem(void);

class CEditGraph;
class CGraphSearcher;
class CMainToolsWnd;
class CMaiUserInput
{
public:
	CMaiUserInput() : bLastLeftOnUp(0),
						bLastRightOnUp(0),
						bLastLeftOnDown(0),
						bLastRightOnDown(0),
						bLastLeftButton(0),
						bLastRightButton(0),
						nLastMouseX(0),
						nLastMouseY(0),
						nDeltaMouseX(0),
						nDeltaMouseY(0) { };
	BOOL bLastLeftOnUp;
	BOOL bLastRightOnUp;
	BOOL bLastLeftOnDown;
	BOOL bLastRightOnDown;
	BOOL bLastLeftButton;
	BOOL bLastRightButton;
	u32  nLastMouseX;
	u32  nLastMouseY;
	s32  nDeltaMouseX;
	s32  nDeltaMouseY;
};

void maimain_Work(const CMaiUserInput& rMaiUI);
void maimain_Draw(const CMaiUserInput& rMaiUI);
BOOL maimain_GetCamPos(CFVec3A* pCamPos);
BOOL maimain_GetCamLookAt(CFVec3A* pCamLookAt);
s32 maimain_MakeBackupCopy(const char* pszSrcFileFullPath, const char* pszBackupDirPath);

class CFWorldShapeInit;
BOOL maimain_WorldShapeCreateCallback( cchar *pszWorldResName, const void *pFixupOffsetBase, const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount );
void maimain_RenderSplines(void);
void maimain_RenderSpline(u32 uSplineId);
u32 maimain_GetNumSplines(void);



extern CFVec3A maimain_MouseRayOrigin;
extern CFVec3A maimain_MouseRayEnd;
extern CFVec3A maimain_MouseRayImpactPt;
extern CFVec3A maimain_MouseRayImpactPtPrevious;
extern BOOL	maimain_bMouseRayImpactThisFrame;
extern CEditGraph *maimain_pEditGraph;
extern const char *maimain_pszGraphFilePath;
extern const char *maimain_pszGraphFileName;
extern char maimain_szGraphFileFullPath[256];
extern CGraphSearcher *maimain_pDebugGraphSearcher;
extern CMainToolsWnd *maimain_pMainToolBarWindow;
extern CNiList<CQueryDetails*> *aimain_pQueryDetailsList;
extern FLinkRoot_t		*maimain_pPQNodePool;
extern FLinkRoot_t		*maimain_pNodePool;

short maimain_GetKeyState(int vKey);

#endif //_MAIMAIN_H_