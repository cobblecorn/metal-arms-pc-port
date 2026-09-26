//////////////////////////////////////////////////////////////////////////////////////
// fshadow.h - 
//
// Author: Steve Ranck     
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
// 07/10/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FSHADOW_H_
#define _FSHADOW_H_ 1

#include "fang.h"
#include "fmath.h"


#define FSHADOW_Y_DISTANCE_WEIGHT	0.5f


class CFWorldLight;
class CFLight;
class CFMeshInst;
struct FMesh_RenderLight_t;

extern f32 FShadow_ShadowDist_MP;

extern f32 FShadow_fShadowStartFade_MP2;
extern f32 FShadow_fOOShadowFadeDist_MP2;

extern f32 FShadow_Max_Detail_Dist;
extern f32 FShadow_Max_Circle_Dist;
extern f32 FShader_ShadowRecieveDist;
extern f32 FShader_ShadowLightDist;

extern BOOL FShadow_bMultiplayer;
extern BOOL FShadow_bActive;

//extern global variables.
extern f32 FShadow_Max_Detail_Dist;
extern f32 FShader_ShadowRecieveDist;
extern BOOL FShadow_bMultiplayer;

// Forward declarations
extern BOOL fshadow_ModuleStartup( void );
extern void fshadow_ModuleShutdown( void );

extern void fshadow_ClearLightList();
extern void fshadow_ClearVolumeList();
extern void fshadow_SubmitLight(CFLight *pLight, CFMeshInst *pMesh);
extern void fshadow_EndSubmit();

extern void fshadow_DrawTest();
extern BOOL fshadow_LightCastShadow( CFLight *pLight );

extern f32 fshadow_GetShadowMtx(CFLight *pLight, CFVec3 *pPos, CFVec3 *pDir, CFVec3 *pUp, CFMeshInst *pMesh=NULL);
extern u32 fshadow_GetNumShadowPasses(FMesh_RenderLight_t *pRenderLights, u8 nRenderLightCount, CFMeshInst *pMesh, BOOL bVolumeGeo=TRUE);
extern BOOL fshadow_SetupShadowPass(u32 nPass, BOOL bWorldGeo);
extern void fshadow_BeginShadowRender();
extern void fshadow_EndShadowRender();

extern f32 fshadow_CheckObjDist( CFMeshInst *pMesh, f32 &fFade2 );

extern void fshadow_SaveVolumeLightList(CFWorldLight *apVolumeLights[], u32 nVolumeLightCount, CFMeshInst *pWorldGeo);
FINLINE void fshadow_ActivateShadows(BOOL bActivate=TRUE) { FShadow_bActive = bActivate; }

FINLINE void fshadow_SetMultiplayer(BOOL bMultiplayer) { FShadow_bMultiplayer = (!!bMultiplayer); }

#endif

