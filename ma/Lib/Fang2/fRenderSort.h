//////////////////////////////////////////////////////////////////////////////////////
// fRenderSort.h - Fang module for managing the ordering of render
//
// Author: John Lafleur
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
// 10/14/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FRENDERSORT_H_
#define _FRENDERSORT_H_ 1

#include "fang.h"
#include "fviewport.h"

class CFMeshInst;
class CFWorldPSGroup;
struct FMeshMaterial_t;

#define FRS_RENDER_NONE				0x00000000
#define FRS_RENDER_PASS_LIGHTING	0x00000001
#define FRS_RENDER_PASS_SURFACE		0x00000002
#define FRS_RENDER_PASS_SPECULAR	0x00000004
#define FRS_RENDER_VOLUME_GEO		0x00010000
#define FRS_RENDER_OBJECT_GEO		0x00020000
#define FRS_RENDER_SPRITES			0x00040000
#define FRS_RENDER_SHADOWS			0x00080000
#define FRS_RENDER_TRANSLUCENCIES	0x01000000

extern BOOL frs_ModuleStartup( void );
extern void frs_ModuleShutdown( void );
extern u32  frs_GenerateMaterialDLHash( FMeshMaterial_t *pMaterial );
extern BOOL frs_AddMeshInst( CFMeshInst *pMeshInst, FViewportPlanesMask_t nPlanesMask, f32 fDistToCamSq, f32 fVPtoScreenPixelRatio );
extern BOOL frs_AddVolumeMesh( CFMeshInst *pMeshInst, FViewportPlanesMask_t nPlanesMask, f32 fDistToCamSq );
extern BOOL frs_AddVolumeMeshMaterial( CFMeshInst *pMeshInst, FMeshMaterial_t *pMat, FViewportPlanesMask_t nPlanesMask, f32 fDistToCam );
extern BOOL frs_AddPSGroup( CFWorldPSGroup *pPSGroup, FViewportPlanesMask_t nPlanesMask, f32 fDistToCamSq );
extern void frs_FlushRenderLists( void );

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//	Data and functions for registering render callbacks:
//

typedef enum
{
	FRS_PREPOINTSPRITE_CALLBACK = 0,
	FRS_PRETRANSLUCENCY_CALLBACK,
	FRS_MAX_CALLBACK_TYPES,

} FRS_RenderCallbackType_e;

// Callback 
typedef void FRenderSortCallback_t( CFMtx43A *pRenderCamMtx, FViewport_t *pRenderViewport );

// Function to register a render callback.  Returns the prior callback for this type
// or NULL if no prior callback existed for this type.
//
// INSIDE ANY RENDERSORT CALLBACK, THERE MUST BE NO MODIFICATIONS TO THE CAMERA, VIEW OR PROJECTION MATRICES
//
extern FRenderSortCallback_t* frs_RegisterRenderCallback( FRS_RenderCallbackType_e nType, FRenderSortCallback_t *pCallback );

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


extern BOOL FRS_bRenderFlags;  // See FRS_RENDER_*, above


#endif // _FRENDERSORT_H_