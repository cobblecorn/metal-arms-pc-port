//////////////////////////////////////////////////////////////////////////////////////
// fperf.h - Fang performance info module.
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
// 03/01/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FPERF_H_
#define _FPERF_H_ 1

#include "fang.h"
#include "ftimer.h"
#include "fvid.h"


#define FPERF_ENABLE				(TRUE & (!FANG_PRODUCTION_BUILD))		// Disable performance code for production builds

#define FPERF_NUM_SAMPLE_FRAMES		60

extern u32	FPerf_uStartTime;


// Add all the enums you want here - Init each one before use with a name, and then fill in the sample data:
typedef enum 
{
	FPERF_MAIN_LOOP_WITH_VSYNC = 0,
	FPERF_MAIN_LOOP_WITHOUT_VSYNC,
	FPERF_MAIN_FWORLD_DRAW,
	FPERF_FPS2TEXUL_NUM_UPLOAD_TEX_BYTES,
	FPERF_FPS2TEXUL_WAIT_TEX0,
	FPERF_FPS2TEXUL_WAIT_TEX1,
	FPERF_FPS2TEXUL_CHAIN_MANAGEMENT,
	
	FPERF_FPS2MESH_MAT_DB_WAIT,
	FPERF_FPS2MESH_MAT_DB_WAIT_BLOCK,
	
	FPERF_FPS2MESH_BATCH_MINUS_DRAW,
	
	FPERF_FPS2MESH_DRAWTB_TEX_REG_SETUP,
	FPERF_FPS2MESH_DRAWTB,
	FPERF_FPS2MESH_DRAWTB_COUNT,
	
	FPERF_FPS2MESH_SUB_MAT_DRAW_TEX_REG_SETUP,
	FPERF_FPS2MESH_SUB_MAT_DRAW,
	FPERF_FPS2MESH_SUB_MAT_COUNT,
	
	FPERF_FPS2MESH_COLLISION,
	FPERF_FPS2MESH_NUM_VERTS,
	//FPERF_FPS2MESH_NUM_TRIS, //hard to know this...
	FPERF_FPS2MESH_NUM_UPLOAD_VERT_BYTES,
	
	// Don't add after here!!!
	FPERF_NUM_TIMERS
} FPerfTimerName_e;


typedef struct {
	// ADD A TYPE FIELD HERE SO WE CAN STORE F32 AND OTHER NON-U32 DATA (pointers to structs even)!!!
	u8 *pszTimerName;

	u32 uSampleTicks[FPERF_NUM_SAMPLE_FRAMES];

	CFTimer Timer;
} PerfTimerInfo_t;

//#if (FPERF_ENABLE)
#if (0)
#define FPERF_ADD_TIME( x ) { if((FVid_nSwapCounter - FPerf_uStartTime) < FPERF_NUM_SAMPLE_FRAMES) { (x)->uSampleTicks[FVid_nSwapCounter - FPerf_uStartTime] += (x)->Timer.SampleTicks(0); } }
#define FPERF_SET_TIME( x ) { if((FVid_nSwapCounter - FPerf_uStartTime) < FPERF_NUM_SAMPLE_FRAMES) { (x)->uSampleTicks[FVid_nSwapCounter - FPerf_uStartTime] = (x)->Timer.SampleTicks(0); } }

#define FPERF_ADD_OTHER( x, y ) { if((FVid_nSwapCounter - FPerf_uStartTime) < FPERF_NUM_SAMPLE_FRAMES) { (x)->uSampleTicks[FVid_nSwapCounter - FPerf_uStartTime] += (u32)(y); } }
#define FPERF_SET_OTHER( x, y ) { if((FVid_nSwapCounter - FPerf_uStartTime) < FPERF_NUM_SAMPLE_FRAMES) { (x)->uSampleTicks[FVid_nSwapCounter - FPerf_uStartTime] = (u32)(y); } }
#else
#define FPERF_ADD_TIME( x ) {}
#define FPERF_SET_TIME( x ) {}

#define FPERF_ADD_OTHER( x, y ) {}
#define FPERF_SET_OTHER( x, y ) {}
#endif



// pszTimerName is NULL if you don't care about writing this timer anymore...
extern void FPerf_InitTimer( FPerfTimerName_e nTimerEnum, u8 *pszTimerName );
extern void FPerf_ResetTimers();
extern void FPerf_WriteTimers( u8 *pszFName );
extern PerfTimerInfo_t *FPerf_GetTimerInfo( FPerfTimerName_e nTimerEnum );

extern u32 FPerf_nUpdateKey;						// Increments with each call to fperf_Reset()
extern CFTimer FPerf_Timer;							// General purpose timer

extern u32 FPerf_nTotalAnimSysBones;		// Number of bones that passed through the animation system
extern u32 FPerf_nTotalBonesStatic;			// Number of bones that were not animated
extern u32 FPerf_nTotalExecBuffersBuilt;	// Number of bone animation execute buffers that were built.
extern u32 FPerf_nTotalKeysInterpolated;	// Total number of animation keys interpolated
extern u32 FPerf_nTotalKeysNotInterpolated;	// Total number of animation keys not interpolated

extern u32 FPerf_nVolumeMeshDrawnCount;	// Number of volume meshes drawn
extern u32 FPerf_nMeshInstDrawnCount;		// Number of CFMeshInst objects drawn
extern u32 FPerf_nMeshMtlDrawnCount;		// Number of materials drawn
extern u32 FPerf_nVolMeshMtlFrstmTested;	// Number of volume mesh materials that were tested against frustum
extern u32 FPerf_nVolMeshMtlCullCount;		// Number of volume materials culled based on the viewport
extern u32 FPerf_nVolMeshMtlUnclipCount;	// Number of volume materials with clip flag removed

extern u32 FPerf_nPSGroupDrawnCount;		// Number of CFPSpriteGroup drawn
extern u32 FPerf_nPSpritesDrawnCount;		// Number of point sprites drawn

extern u32 FPerf_nMeshTris;				// Number of mesh triangles in rendered meshes
extern u32 FPerf_nMeshShadowTris;			// Number of mesh shadow triangles sent to the GPU
extern u32 FPerf_nTotalTrisLighting;		// Number of triangles in lighting pass
extern u32 FPerf_nTotalTrisShadow;			// Number of triangles in shadow pass
extern u32 FPerf_nTotalTrisSurface;		// Number of triangles in surface pass
extern u32 FPerf_nTotalTrisSpecular;		// Number of triangles in specular pass
extern u32 FPerf_nTotalTrisTranslucent;	// Number of triangles in translucent pass
extern u32 FPerf_nRawStripTriCount;		// Number of triangles submitted to the graphics hardware (reflects multiple passes)
extern u32 FPerf_nRawListTriCount;			// Number of triangles submitted to the graphics hardware (reflects multiple passes)
extern u32 FPerf_nRawTriListCount;			// Number of triangle lists submitted to the graphics hardware
extern u32 FPerf_nRawTriStripCount;		// Number of triangle strips submitted to the graphics hardware
extern u32 FPerf_nRawVertexCount;			// Number of vertices submitted to the graphics hardware

extern u32 FPerf_nRawTexSwitchCount;		// Number of times textures were switched during geometry rendering
extern u32 FPerf_nRawVBSwitchCount;		// Number of times vertex buffers were switched during geometry rendering
extern u32 FPerf_nRawShSwitchCount;		// Number of times shaders were switched during geometry rendering
extern u32 FPerf_nFastSurfaceShaderCount;	// Number of fast surface shader calls during geometry rendering
extern u32 FPerf_nFullSurfaceShaderCount;	// Number of full surface shader calls during geometry rendering

extern u32 FPerf_nUniqueTexturesUsed;		// Number of unique textures selected during geometry rendering

extern u32 FPref_nCollNodes;				// Number of nodes involved with triangle collision testing (total, not unique)
extern u32 FPerf_nCollTris;				// Number of triangles tested for collision

extern f32 FPerf_fCollGenDataSecs;			// Number of seconds spent in fworld_Coll_GenerateData()
extern f32 FPerf_fCollGenSubset;			// Number of seconds spent in fworld_Coll_GenerateSubset()
extern f32 FPerf_fCollIntersectSubset;		// Number of seconds spent in fworld_Coll_IntersectSubsetTris()
extern f32 FPerf_fCollWithWorldTris;		// Number of seconds spent in fworld_CollideWithWorldTris()

extern BOOL fperf_ModuleStartup( void );
extern void fperf_ModuleShutdown( void );

typedef enum
{
	FPERF_TIMER_START = 0,
	FPERF_TIMER_END,
	FPERF_FUNCTION_COUNT,

} FPerf_TimerFunction_e;

typedef enum
{
	FPERF_FRAME = 0,
	FPERF_SIM,
	FPERF_DRAW,
	FPERF_SWAP,
	FPERF_TIMER_COUNT,

} FPerf_TimerType_e;

extern void fperf_ModuleUpdate( void );
extern void fperf_TimerReport( FPerf_TimerType_e nTimerType, FPerf_TimerFunction_e nFunction );

extern void fperf_Reset( void );


///////////////////////////////////////////////////
///////////////////////////////////////////////////

extern f32  FPerf_AvgSPF;			// Average number of seconds per frame
extern f32  FPerf_fAvgSimSPF;		// Average number of seconds per frame spent in simulation
extern f32  FPerf_fAvgDrawSPF;		// Average number of seconds per frame spent drawing
extern f32  FPerf_fAvgSwapSPF;		// Average number of seconds per frame spent waiting for swap (waiting on the GPU)
extern f32  FPerf_fAvgToTargetSPFRatio;	// Ration of current average and target seconds per frame (1.f == on target, < 1.f == above target, > 1.f below target)
extern f32  FPerf_fCPUvGPURatio;	// Value which represents the approximate ration of CPU time to GPU time (sim time / (draw time + swap time));
extern BOOL FPerf_bGPUBound;		// If the value is TRUE, then the gmae is graphics draw bound



typedef enum 
{
	FPERF_TYPE_NONE = 0,
	FPERF_TYPE_FRAMERATE,
	FPERF_TYPE_MEMORY,
	FPERF_TYPE_LIGHTING,
	FPERF_TYPE_RENDER,
	FPERF_TYPE_ANIM,
	FPERF_TYPE_ALL,				// This type will be displayed in all modes

	FPERF_MAX_TYPES,

} FPerf_Display_Type_e;


///////////////////////////////////////////////////
///////////////////////////////////////////////////
// Performance bar data and calls.  Do not use in
// production builds:
//
extern FPerf_Display_Type_e FPerf_nDisplayPerfType;
extern BOOL FPerf_bDisplayPerfAlways;			// TRUE | FALSE - If FALSE, performance will fade out after a few seconds of being triggered

// 
// Causes performance to be drawn on screen using fdraw.
//
extern void fperf_RenderPerf( void );

// To cause a performance tracker to be drawn forever, enter -1.f as the lifespan
// Multiple calls will update the tracker previously submitted.
// Multiple trackers cannot have the same label (this is used for updating).
extern void fperf_Render_AddPerfBar( FPerf_Display_Type_e nType, char *pszLabel, f32 fValue, f32 fLifespan = 7.f );
extern void fperf_Render_AddPerfInt( FPerf_Display_Type_e nType, char *pszLabel, s32 nValue, f32 fLifespan = 7.f );
extern void fperf_Render_AddPerfFloat( FPerf_Display_Type_e nType, char *pszLabel, f32 fValue, f32 fLifespan = 7.f );
extern void fperf_Render_AddPerfString( FPerf_Display_Type_e nType, char *pszLabel, char *pszText, f32 fLifespan = 7.f );

#endif

