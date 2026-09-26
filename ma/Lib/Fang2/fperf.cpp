//////////////////////////////////////////////////////////////////////////////////////
// fperf.cpp - Fang performance info module.
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

#include "fang.h"
#include "fperf.h"
#include "fvid.h"
#include "fxfm.h"
#include "ffile.h"
#include "fclib.h"
#include "ftext.h"
#include "fdraw.h"
#include "floop.h"
#include "fviewport.h"
#include "frenderer.h"
#include "fDataStreaming.h"

#include <string.h>
#include <stdio.h>


//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _MAX_FRAME_TIME_MEASURE					30

#define _PERF_TRACKING_DISPLAY_ENABLED			(TRUE & (!FANG_PRODUCTION_BUILD))// Setting to false will remove ALL performance info from screen
#define _MILLISECOND_TRACKING_BAR_ENABLED		TRUE

#define _MAX_PERF_TRACKERS						48
#define _PERF_TRACKING_BAR_SPACING				0.038f
#define _PERF_TRACKING_BAR_MAX_VALUE			1.5f

#if FANG_PLATFORM_WIN
	#define _PERF_TRACKING_BAR_X_START			0.07f
#else
	#define _PERF_TRACKING_BAR_X_START			0.135f
#endif
#define _PERF_TRACKING_BAR_Y_START				0.12f

// Target framerate is 30 fps
#define _TARGET_SECONDS_A_FRAME			(0.0166666666667f * 2.f)


//////////////////////////////////////////////////////////////////////////////////////
// Local structures:
//////////////////////////////////////////////////////////////////////////////////////


#if _PERF_TRACKING_DISPLAY_ENABLED
	enum
	{
		_PERF_TRACK_TYPE_BAR,
		_PERF_TRACK_TYPE_INT,
		_PERF_TRACK_TYPE_FLOAT,
		_PERF_TRACK_TYPE_STRING,
	};
	struct PerfTracker
	{
		char  szLabel[16];
		f32   fLife;
		u16   nType;
		u16   nPerfType;
		char  szText[16];
		union
		{
			f32   fValue;
			s32   nValue;
		};
	};
		
	static PerfTracker _PerfTrackers[_MAX_PERF_TRACKERS];
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

u32 FPerf_nUpdateKey;						// Increments with each call to fperf_Reset()
CFTimer FPerf_Timer;						// General purpose timer

u32 FPerf_nTotalAnimSysBones;		// Number of bones that passed through the animation system
u32 FPerf_nTotalBonesStatic;		// Number of bones that were not animated
u32 FPerf_nTotalExecBuffersBuilt;	// Number of bone animation execute buffers that were built.
u32 FPerf_nTotalKeysInterpolated;	// Total number of animation keys interpolated
u32 FPerf_nTotalKeysNotInterpolated;	// Total number of animation keys not interpolated

u32 FPerf_nVolumeMeshDrawnCount;	// Number of volume meshes drawn
u32 FPerf_nMeshInstDrawnCount;		// Number of CFMeshInst objects drawn
u32 FPerf_nMeshMtlDrawnCount;		// Number of materials drawn
u32 FPerf_nVolMeshMtlFrstmTested;	// Number of volume mesh materials that were tested against frustum
u32 FPerf_nVolMeshMtlCullCount;	// Number of volume materials culled based on the viewport
u32 FPerf_nVolMeshMtlUnclipCount;	// Number of volume materials with clip flag removed

u32 FPerf_nPSGroupDrawnCount;		// Number of CFPSpriteGroup drawn
u32 FPerf_nPSpritesDrawnCount;		// Number of point sprites drawn

u32 FPerf_nMeshTris;				// Number of mesh triangles in rendered meshes
u32 FPerf_nMeshShadowTris;			// Number of mesh shadow triangles sent to the GPU
u32 FPerf_nTotalTrisLighting;		// Number of triangles in lighting pass
u32 FPerf_nTotalTrisShadow;		// Number of triangles in shadow pass
u32 FPerf_nTotalTrisSurface;		// Number of triangles in surface pass
u32 FPerf_nTotalTrisSpecular;		// Number of triangles in specular pass
u32 FPerf_nTotalTrisTranslucent;	// Number of triangles in translucent pass
u32 FPerf_nRawStripTriCount;		// Number of triangles submitted to the graphics hardware
u32 FPerf_nRawListTriCount;		// Number of triangles submitted to the graphics hardware
u32 FPerf_nRawTriListCount;		// Number of triangle lists submitted to the graphics hardware
u32 FPerf_nRawTriStripCount;		// Number of triangle strips submitted to the graphics hardware
u32 FPerf_nRawVertexCount;			// Number of vertices submitted to the graphics hardware

u32 FPerf_nRawTexSwitchCount;		// Number of times textures were switched during geometry rendering
u32 FPerf_nRawVBSwitchCount;		// Number of times vertex buffers were switched during geometry rendering
u32 FPerf_nRawShSwitchCount;		// Number of times shaders were switched during geometry rendering
u32 FPerf_nFastSurfaceShaderCount;	// Number of fast surface shader calls during geometry rendering
u32 FPerf_nFullSurfaceShaderCount;	// Number of full surface shader calls during geometry rendering

u32 FPerf_nUniqueTexturesUsed;		// Number of unique textures selected during geometry rendering

u32 FPref_nCollNodes;				// Number of nodes involved with triangle collision testing (total, not unique)
u32 FPerf_nCollTris;				// Number of triangles tested for collision

f32 FPerf_fCollGenDataSecs;		// Number of seconds spent in fworld_Coll_GenerateData()
f32 FPerf_fCollGenSubset;			// Number of seconds spent in fworld_Coll_GenerateSubset()
f32 FPerf_fCollIntersectSubset;	// Number of seconds spent in fworld_Coll_IntersectSubsetTris()
f32 FPerf_fCollWithWorldTris;		// Number of seconds spent in fworld_CollideWithWorldTris()

FPerf_Display_Type_e FPerf_nDisplayPerfType;
BOOL FPerf_bDisplayPerfAlways;

f32  FPerf_AvgSPF;
f32  FPerf_fAvgSimSPF;
f32  FPerf_fAvgDrawSPF;
f32  FPerf_fAvgSwapSPF;
f32  FPerf_fAvgToTargetSPFRatio;
f32  FPerf_fCPUvGPURatio;
BOOL FPerf_bGPUBound;

PerfTimerInfo_t	FPerf_Timers[FPERF_NUM_TIMERS];
u32 FPerf_uStartTime;


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;
static u8   _FPerf_pszWriteElement[128];


static f32  _afFrameTime[FPERF_TIMER_COUNT][_MAX_FRAME_TIME_MEASURE];
static u32  _anAccFrames[FPERF_TIMER_COUNT];
static f32  _afAccFrameSec[FPERF_TIMER_COUNT];
static f32  _afLastFrameSec[FPERF_TIMER_COUNT];
static f32  _fLastFPSReport;
static BOOL _bFilledFrametimeBuffer;
static CFTimer _aTimers[FPERF_TIMER_COUNT];


//////////////////////////////////////////////////////////////////////////////////////
// Static functions:
//////////////////////////////////////////////////////////////////////////////////////

static void _Render_Reset( void );
static void _Render( void );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fperf_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );
	u32 i;

	fang_MemZero( _afFrameTime, sizeof( f32 ) * _MAX_FRAME_TIME_MEASURE * FPERF_TIMER_COUNT );
	fang_MemZero( _anAccFrames, sizeof( u32 ) * FPERF_TIMER_COUNT );
	fang_MemZero( _afAccFrameSec, sizeof( f32 ) * FPERF_TIMER_COUNT );
	fang_MemZero( _afLastFrameSec, sizeof( f32 ) * FPERF_TIMER_COUNT );
	_fLastFPSReport = 0;
	_bFilledFrametimeBuffer = FALSE;

	FPerf_AvgSPF = _TARGET_SECONDS_A_FRAME * 1000.f;
	FPerf_fAvgSimSPF = 0.f;
	FPerf_fAvgDrawSPF = 0.f;
	FPerf_fAvgSwapSPF = 0.f;
	FPerf_fAvgToTargetSPFRatio = 1.f;
	FPerf_fCPUvGPURatio = 1.f;
	FPerf_bGPUBound = FALSE;
	
	FPerf_nDisplayPerfType = FPERF_TYPE_NONE;
	FPerf_bDisplayPerfAlways = FALSE;
	
	_bModuleInitialized = TRUE;

	FPerf_nUpdateKey = 0;
	fperf_Reset();
	
	FPerf_uStartTime = 0; //not in reset...
	FPerf_ResetTimers();
	
#if _PERF_TRACKING_DISPLAY_ENABLED
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		_PerfTrackers[i].fLife = -FMATH_MAX_FLOAT;
	}
#endif	

	for ( i = 0; i < FPERF_TIMER_COUNT; i++ )
	{
		_aTimers[i].Reset();
	}

	return TRUE;
}


//
//
//
void fperf_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}


//
//
//
void fperf_ModuleUpdate( void ) 
{
	// Calculate Average Framerate
	if ( _bFilledFrametimeBuffer )
	{
		FPerf_AvgSPF = _afAccFrameSec[FPERF_FRAME] / _MAX_FRAME_TIME_MEASURE;
		FPerf_fAvgSimSPF =  _afAccFrameSec[FPERF_SIM] / _MAX_FRAME_TIME_MEASURE;
		FPerf_fAvgDrawSPF = _afAccFrameSec[FPERF_DRAW] / _MAX_FRAME_TIME_MEASURE;
		FPerf_fAvgSwapSPF = _afAccFrameSec[FPERF_SWAP] / _MAX_FRAME_TIME_MEASURE;

		if ( FPerf_fAvgSwapSPF > 0.001f )
		{	
			FPerf_bGPUBound = TRUE;
		}

		FPerf_fCPUvGPURatio = FPerf_fAvgSimSPF / (FPerf_fAvgDrawSPF + FPerf_fAvgSwapSPF);
		FPerf_fAvgToTargetSPFRatio = FPerf_AvgSPF / _TARGET_SECONDS_A_FRAME;

#if FPERF_ENABLE
		fperf_Render_AddPerfInt( FPERF_TYPE_FRAMERATE, "FPS", (s32)fmath_Inv( FPerf_AvgSPF ), 0.f );
		fperf_Render_AddPerfFloat( FPERF_TYPE_FRAMERATE, "Wrk", FPerf_fAvgSimSPF * 1000.0f, 0.f );
		fperf_Render_AddPerfFloat( FPERF_TYPE_FRAMERATE, "Drw", FPerf_fAvgDrawSPF * 1000.0f, 0.f );
		fperf_Render_AddPerfFloat( FPERF_TYPE_FRAMERATE, "Swp", FPerf_fAvgSwapSPF * 1000.0f, 0.f );

		fperf_Render_AddPerfInt( FPERF_TYPE_ANIM, "Total bones:", FPerf_nTotalAnimSysBones, 0.f );
		if ( FPerf_nTotalAnimSysBones )
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_ANIM, "Pcnt static:", (f32)FPerf_nTotalBonesStatic/(f32)FPerf_nTotalAnimSysBones, 0.f );
		}
		fperf_Render_AddPerfInt( FPERF_TYPE_ANIM, "ExcBf Built:", FPerf_nTotalExecBuffersBuilt, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_ANIM, "Total Frmes:", FPerf_nTotalKeysInterpolated + FPerf_nTotalKeysNotInterpolated, 0.f );
		if ( FPerf_nTotalKeysInterpolated + FPerf_nTotalKeysNotInterpolated )
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_ANIM, "Pcnt Interp:", (f32)FPerf_nTotalKeysInterpolated/(f32)(FPerf_nTotalKeysInterpolated + FPerf_nTotalKeysNotInterpolated), 0.f );
		}

		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Materials :", FPerf_nMeshMtlDrawnCount, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "VMat Tests:", FPerf_nVolMeshMtlFrstmTested, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "VMat Cull :", FPerf_nVolMeshMtlCullCount, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "VMat Unclp:", FPerf_nVolMeshMtlUnclipCount, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Volumes   :", FPerf_nVolumeMeshDrawnCount, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Objects   :", FPerf_nMeshInstDrawnCount, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Mesh Tris :", FPerf_nMeshTris, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Shdw Tris :", FPerf_nMeshShadowTris, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Lighting  :", FPerf_nTotalTrisLighting, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Shadows   :", FPerf_nTotalTrisShadow , 0.f);
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Surface   :", FPerf_nTotalTrisSurface, 0.f );
//		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Specular  :", FPerf_nTotalTrisSpecular, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Trnslucnt :", FPerf_nTotalTrisTranslucent, 0.f );
		if ( FPerf_nRawStripTriCount + FPerf_nRawListTriCount )
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Pcnt strip:", 100.f * ((f32)FPerf_nRawStripTriCount / (FPerf_nRawStripTriCount + FPerf_nRawListTriCount)), 0.f );
		}
		else
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Pcnt strip:", 0.f, 0.f );
		}
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "GPU Tris  :", (s32)FPerf_nRawStripTriCount + FPerf_nRawListTriCount, 0.f );
		if ( FPerf_nRawTriStripCount )
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Avg Strip :", fmath_Div((f32)FPerf_nRawStripTriCount, (f32)FPerf_nRawTriStripCount), 0.f );
		}
		else
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Avg Strip :", 0.f, 0.f );
		}
//		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "List Tris:", FPerf_nRawListTriCount );
		if ( FPerf_nRawTriListCount )
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Avg List  :", fmath_Div((f32)FPerf_nRawListTriCount, (f32)FPerf_nRawTriListCount), 0.f );
		}
		else
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Avg List  :", 0.f, 0.f );
		}
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Tex switch:", FPerf_nRawTexSwitchCount, 0.f );
		fperf_Render_AddPerfInt( FPERF_TYPE_RENDER, "Shd switch:", FPerf_nRawShSwitchCount, 0.f );
		if ( FPerf_nFullSurfaceShaderCount )
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Prcnt Fast:", 100.f * ((f32)FPerf_nFastSurfaceShaderCount / (f32)(FPerf_nFastSurfaceShaderCount + FPerf_nFullSurfaceShaderCount)), 0.f );
		}
		else
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Prcnt Fast:", 0.f, 0.f );
		}
		if ( FPerf_nMeshTris )
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Avg PassPP:", (f32)(FPerf_nRawStripTriCount + FPerf_nRawListTriCount ) / (f32)FPerf_nMeshTris, 0.f );
		}
		else
		{
			fperf_Render_AddPerfFloat( FPERF_TYPE_RENDER, "Avg PassPP:", 0.f, 0.f );
		}
#endif
	}
}


//
//
//
void fperf_TimerReport( FPerf_TimerType_e nTimerType, FPerf_TimerFunction_e nFunction )
{
	FASSERT( nTimerType >= 0 && nTimerType < FPERF_TIMER_COUNT );
	FASSERT( nFunction >= 0 && nFunction < FPERF_FUNCTION_COUNT );

	if ( nFunction == FPERF_TIMER_START )
	{
		_aTimers[nTimerType].Reset();
	}
	else
	{
		f32 fSeconds = _aTimers[nTimerType].SampleSeconds();

		_afAccFrameSec[nTimerType] -= _afFrameTime[nTimerType][ _anAccFrames[nTimerType] ];

		_afFrameTime[nTimerType][ _anAccFrames[nTimerType] ] = fSeconds;
		_afAccFrameSec[nTimerType] += _afFrameTime[nTimerType][ _anAccFrames[nTimerType] ];
		_afLastFrameSec[nTimerType] = fSeconds;


#if !FANG_PRODUCTION_BUILD	// fperf_Render_AddPerfBar not defined in production build?
#if _MILLISECOND_TRACKING_BAR_ENABLED
		if ( nTimerType == FPERF_FRAME )
		{
			f32 fRatio = _afFrameTime[nTimerType][ _anAccFrames[nTimerType] ] * (1.f / _TARGET_SECONDS_A_FRAME );
			fperf_Render_AddPerfBar( FPERF_TYPE_FRAMERATE, "MS", fRatio, 0.f );
		}
#endif
#endif
		_anAccFrames[nTimerType]++;
		if ( _anAccFrames[nTimerType] == _MAX_FRAME_TIME_MEASURE )
		{
			_anAccFrames[nTimerType] = 0;
			_bFilledFrametimeBuffer = TRUE;
		}
	}
}


//
//
//
void fperf_Reset( void ) 
{
	FASSERT( _bModuleInitialized );

	FPerf_nUpdateKey++;

	FPerf_nVolumeMeshDrawnCount = 0;
	FPerf_nMeshInstDrawnCount = 0;
	FPerf_nMeshMtlDrawnCount = 0;
	FPerf_nVolMeshMtlFrstmTested = 0;
	FPerf_nVolMeshMtlCullCount = 0;
	FPerf_nVolMeshMtlUnclipCount = 0;

	FPerf_nTotalAnimSysBones = 0;
	FPerf_nTotalBonesStatic = 0;
	FPerf_nTotalExecBuffersBuilt = 0;
	FPerf_nTotalKeysInterpolated = 0;
	FPerf_nTotalKeysNotInterpolated = 0;

	FPerf_nPSGroupDrawnCount = 0;
	FPerf_nPSpritesDrawnCount = 0;

	FPerf_nMeshTris = 0;
	FPerf_nMeshShadowTris = 0;
	FPerf_nTotalTrisLighting = 0;
	FPerf_nTotalTrisShadow = 0;
	FPerf_nTotalTrisSurface = 0;
	FPerf_nTotalTrisSpecular = 0;
	FPerf_nTotalTrisTranslucent = 0;
	FPerf_nRawStripTriCount = 0;
	FPerf_nRawListTriCount = 0;
	FPerf_nRawTriListCount = 0;
	FPerf_nRawTriStripCount = 0;
	FPerf_nRawVertexCount = 0;

	FPerf_nRawTexSwitchCount = 0;
	FPerf_nRawVBSwitchCount = 0;
	FPerf_nRawShSwitchCount = 0;
	FPerf_nFastSurfaceShaderCount = 0;
	FPerf_nFullSurfaceShaderCount = 0;

	FPerf_nUniqueTexturesUsed = 0;

	FPref_nCollNodes = 0;
	FPerf_nCollTris = 0;

	FPerf_fCollGenDataSecs = 0.0f;
	FPerf_fCollGenSubset = 0.0f;
	FPerf_fCollIntersectSubset = 0.0f;
	FPerf_fCollWithWorldTris = 0.0f;
}


//
//
//
void FPerf_InitTimer( FPerfTimerName_e nTimerEnum, u8 *pszTimerName ) 
{
	FASSERT((u32)nTimerEnum < FPERF_NUM_TIMERS);

	FPerf_Timers[nTimerEnum].pszTimerName = pszTimerName;
}


//
//
//
void FPerf_ResetTimers( void ) 
{
	u32 uCount;

	FPerf_uStartTime = FVid_nSwapCounter;

	for( uCount = 0;uCount < FPERF_NUM_TIMERS;uCount++ ) {
		fang_MemSet( &FPerf_Timers[uCount].uSampleTicks[0], 0x00, sizeof(FPerf_Timers[uCount].uSampleTicks) );
	}
}


//
//
//
void FPerf_WriteTimers( u8 *pszFName ) 
{
	u32 uCount, uCount2;
	FFileHandle hWriteFile;

	hWriteFile = ffile_Open((char *)pszFName, FFILE_OPEN_TRUNC_OR_CREATE_WONLY);
	if( hWriteFile >= 0 ) {
		for( uCount = 0;uCount < FPERF_NUM_TIMERS;uCount++ ) {
			if( FPerf_Timers[uCount].pszTimerName ) {
				ffile_Write( hWriteFile, fclib_strlen((char *)FPerf_Timers[uCount].pszTimerName), FPerf_Timers[uCount].pszTimerName );

				for( uCount2 = 0;uCount2 < FPERF_NUM_SAMPLE_FRAMES;uCount2++ ) {
					sprintf( (char *)_FPerf_pszWriteElement, ",%d", FPerf_Timers[uCount].uSampleTicks[uCount2] );
					ffile_Write( hWriteFile, fclib_strlen((char *)_FPerf_pszWriteElement), _FPerf_pszWriteElement );
				}

				ffile_Write( hWriteFile, 1, (char *)"\r\n" );
			}
		}

		ffile_Close( hWriteFile );
	}
}


//
//
//
PerfTimerInfo_t *FPerf_GetTimerInfo( FPerfTimerName_e nTimerEnum ) 
{
	FASSERT( (u32)nTimerEnum < FPERF_NUM_TIMERS );
	FASSERT( FPerf_Timers[nTimerEnum].pszTimerName );

	return &FPerf_Timers[nTimerEnum];
}


#if !_PERF_TRACKING_DISPLAY_ENABLED
	void fperf_Render_AddPerfBar( FPerf_Display_Type_e nType, char *pszLabel, f32 fValue, BOOL bAlwaysVisible )
	{
	}
	void fperf_Render_AddPerfNumber( FPerf_Display_Type_e nType, char *pszLabel, f32 fValue, BOOL bAlwaysVisible )
	{
	}
	void fperf_Render_AddPerfFloat( FPerf_Display_Type_e nType, char *pszLabel, f32 fValue, f32 fLifespan/*=7.f*/ )
	{
	}
	void fperf_Render_AddPerfInt( FPerf_Display_Type_e nType, char *pszLabel, s32 nValue, f32 fLifespan/*=7.f*/ )
	{
	}
#else
//
//
//
static void _AddPerfTracker( FPerf_Display_Type_e nPerfType, char *pszLabel, f32 fValue, s32 nValue, char *pszText, f32 fLifespan, u32 nType )
{
	u32 i;
	
	// Make sure we have room for a new one	
	// If this one already exists, then update it
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		if ( _PerfTrackers[i].fLife == -FMATH_MAX_FLOAT )
		{
			break;
		}
	}
	
	if ( i == _MAX_PERF_TRACKERS )
	{
		return;
	}

	fclib_strncpy( _PerfTrackers[i].szLabel, pszLabel, 15 );
	_PerfTrackers[i].szLabel[15] = 0;
	_PerfTrackers[i].fLife = fLifespan;
	if ( nType == _PERF_TRACK_TYPE_INT )
	{
		_PerfTrackers[i].nValue = nValue;
	}
	else if ( nType == _PERF_TRACK_TYPE_STRING )
	{
		fclib_strncpy( _PerfTrackers[i].szText, pszText, 15 );
		_PerfTrackers[i].szText[15] = 0;
	}
	else
	{
		_PerfTrackers[i].fValue = fValue;
	}
	_PerfTrackers[i].nType = nType;
	_PerfTrackers[i].nPerfType = nPerfType;
}


//
//
//
void fperf_Render_AddPerfBar( FPerf_Display_Type_e nPerfType, char *pszLabel, f32 fValue, f32 fLifespan/*=7.f*/ )
{
	FASSERT( pszLabel && fLifespan >= 0.f || fLifespan == -1.f );

	u32 i;

	FMATH_CLAMP( fValue, 0.f, _PERF_TRACKING_BAR_MAX_VALUE );

	// If this one already exists, then update it
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		if (   _PerfTrackers[i].fLife != -FMATH_MAX_FLOAT 
			&& _PerfTrackers[i].nType == _PERF_TRACK_TYPE_BAR
			&& fclib_stricmp( _PerfTrackers[i].szLabel, pszLabel ) == 0  )
		{
			_PerfTrackers[i].fValue = fValue;
			_PerfTrackers[i].fLife = fLifespan;
			return;
		}
	}
	
	_AddPerfTracker( nPerfType, pszLabel, fValue, 0, NULL, fLifespan, _PERF_TRACK_TYPE_BAR );
}


//
//
//
void fperf_Render_AddPerfInt( FPerf_Display_Type_e nPerfType, char *pszLabel, s32 nValue, f32 fLifespan/*=7.f*/ )
{
	FASSERT( pszLabel && fLifespan >= 0.f || fLifespan == -1.f );
	
	u32 i;
	
	// If this one already exists, then update it
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		if (   _PerfTrackers[i].fLife != -FMATH_MAX_FLOAT 
			&& _PerfTrackers[i].nType == _PERF_TRACK_TYPE_INT
			&& fclib_stricmp( _PerfTrackers[i].szLabel, pszLabel ) == 0  )
		{
			_PerfTrackers[i].nValue = nValue;
			_PerfTrackers[i].fLife = fLifespan;
			return;
		}
	}

	_AddPerfTracker( nPerfType, pszLabel, 0.f, nValue, NULL, fLifespan, _PERF_TRACK_TYPE_INT );
}


//
//
//
void fperf_Render_AddPerfFloat( FPerf_Display_Type_e nPerfType, char *pszLabel, f32 fValue, f32 fLifespan/*=7.f*/ )
{
	FASSERT( pszLabel && fLifespan >= 0.f || fLifespan == -1.f );
	
	u32 i;
	
	// If this one already exists, then update it
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		if (   _PerfTrackers[i].fLife != -FMATH_MAX_FLOAT 
			&& _PerfTrackers[i].nType == _PERF_TRACK_TYPE_FLOAT
			&& fclib_stricmp( _PerfTrackers[i].szLabel, pszLabel ) == 0  )
		{
			_PerfTrackers[i].fValue = fValue;
			_PerfTrackers[i].fLife = fLifespan;
			return;
		}
	}

	_AddPerfTracker( nPerfType, pszLabel, fValue, 0, NULL, fLifespan, _PERF_TRACK_TYPE_FLOAT );
}


//
//
//
void fperf_Render_AddPerfString( FPerf_Display_Type_e nPerfType, char *pszLabel, char *pszText, f32 fLifespan/*=7.f*/ )
{
	FASSERT( pszLabel && pszText && fLifespan >= 0.f || fLifespan == -1.f );
	
	u32 i;
	
	// If this one already exists, then update it
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		if (   _PerfTrackers[i].fLife != -FMATH_MAX_FLOAT 
			&& _PerfTrackers[i].nType == _PERF_TRACK_TYPE_STRING
			&& fclib_stricmp( _PerfTrackers[i].szLabel, pszLabel ) == 0  )
		{
			fclib_strncpy( _PerfTrackers[i].szText, pszText, 15 );
			_PerfTrackers[i].szText[15] = 0;
			_PerfTrackers[i].fLife = fLifespan;
			return;
		}
	}

	_AddPerfTracker( nPerfType, pszLabel, 0.f, 0, pszText, fLifespan, _PERF_TRACK_TYPE_STRING );
}


//
// Draw registered performance metrics
//
static void _Render( void )
{
	FDrawVtx_t Verts[24];
	fang_MemZero( Verts, sizeof( FDrawVtx_t ) * 24 );

	u32 i, ii;
	f32 fX, fY;
	char szText[64];
	CFColorRGBA Color;
	FDrawVtx_t *pVert = &Verts[0];
	
	// Get current viewport
	FViewport_t *pPreviousVP = fviewport_GetActive();
	u32 nWidth = FViewport_pDefaultOrtho->nWidth;
	u32 nHeight = FViewport_pDefaultOrtho->nHeight;
	
	// Decrement the lifespan
	f32 fLowestLife = 0.f;
	u32 nActiveEntries = 0;
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		if ( _PerfTrackers[i].nPerfType != FPerf_nDisplayPerfType && _PerfTrackers[i].nPerfType != FPERF_TYPE_ALL )
		{
			continue;
		}
		
		if ( _PerfTrackers[i].fLife == -FMATH_MAX_FLOAT )
		{
			continue;
		}
		
		nActiveEntries++;
	}
	
	if ( nActiveEntries == 0 )
	{
		return;
	}
	
	// Set the performance monitoring viewport and fdraw renderer
	fviewport_SetActive( FViewport_pDefaultOrtho );
	CFXfm m_Xfm;
	m_Xfm.Identity();
	m_Xfm.InitStackWithView();
	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );

	f32 fBackgroundWidth = 0.355f;
	if ( FPerf_nDisplayPerfType == FPERF_TYPE_FRAMERATE )
	{
		fBackgroundWidth = 0.28f;
	}
	else if ( FPerf_nDisplayPerfType == FPERF_TYPE_MEMORY )
	{
		fBackgroundWidth = 0.34f;
	}
	else if ( FPerf_nDisplayPerfType == FPERF_TYPE_RENDER )
	{
		fBackgroundWidth = 0.38f;
	}
	else if ( FPerf_nDisplayPerfType == FPERF_TYPE_ANIM )
	{
		fBackgroundWidth = 0.38f;
	}
	//
	// Draw Background
	fX = (nWidth * _PERF_TRACKING_BAR_X_START) - 60;
	fY = (nHeight * _PERF_TRACKING_BAR_Y_START) - 5;
	Color.Set( 0.f, 0.f, 0.f, 0.6f );
	pVert->Pos_MS.Set( fX, fY, 1.f );
	pVert->ColorRGBA = Color;
	pVert++;
	pVert->Pos_MS.Set( fX + (fBackgroundWidth * nWidth), fY, 1.f );
	pVert->ColorRGBA = Color;
	pVert++;
	pVert->Pos_MS.Set( fX, fY + (nActiveEntries * _PERF_TRACKING_BAR_SPACING * nHeight), 1.f );
	pVert->ColorRGBA = Color;
	pVert++;
	(*pVert) = *(pVert - 1);
	pVert++;
	(*pVert) = *(pVert - 3);
	pVert++;
	pVert->Pos_MS.Set( fX + (fBackgroundWidth * nWidth), fY + (nActiveEntries * _PERF_TRACKING_BAR_SPACING * nHeight), 1.f );
	pVert->ColorRGBA = Color;
	pVert++;

	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, Verts, 6 );
	pVert = &Verts[0];
	
	f32 fAlphaAdjust;

	// Draw the performance info
	nActiveEntries = 0;
	f32 fTextYPos = _PERF_TRACKING_BAR_Y_START - (14.f / (f32)nHeight);
	f32 fTextYSpacing = 0.028f;
	f32 fTextXPos = _PERF_TRACKING_BAR_X_START - (53.f / (f32)nWidth);
	for ( ii = 0; ii < 3; ii++ )
	{
		for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
		{
			if ( _PerfTrackers[i].nPerfType != FPerf_nDisplayPerfType && _PerfTrackers[i].nPerfType != FPERF_TYPE_ALL )
			{
				continue;
			}
			
			if ( _PerfTrackers[i].fLife == -FMATH_MAX_FLOAT )
			{
				continue;
			}

			if ( ii == 2 )
			{
				if ( _PerfTrackers[i].nPerfType != FPERF_TYPE_ALL )
				{
					continue;
				}
			}
			else
			{
				if ( ii == 0 && (_PerfTrackers[i].nType != _PERF_TRACK_TYPE_BAR || _PerfTrackers[i].nPerfType == FPERF_TYPE_ALL) )
				{
					// First thing listed are the bars that are of this type
                    continue;
				}

				if ( ii == 1 && (_PerfTrackers[i].nType == _PERF_TRACK_TYPE_BAR || _PerfTrackers[i].nPerfType == FPERF_TYPE_ALL) )
				{
					// Second thing listed are the numerical values that are of this type
					continue;
				}
			}

			fAlphaAdjust = 1.f;

			f32 fRedWidth = fBackgroundWidth * 0.215f;
			f32 fGreenWidth = fRedWidth * 2.f;
			f32 fRedAndGreenWidth = fRedWidth + fGreenWidth;

			if ( _PerfTrackers[i].nType == _PERF_TRACK_TYPE_BAR )
			{
				fX = (_PERF_TRACKING_BAR_X_START * nWidth);
				fY = (_PERF_TRACKING_BAR_Y_START * nHeight) + (_PERF_TRACKING_BAR_SPACING * nActiveEntries * nHeight);
				
				Color.Set( 0.8f, 0.f, 0.f, 0.8f * fAlphaAdjust );
				pVert->Pos_MS.Set( fX + (fGreenWidth * nWidth), fY, 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				pVert->Pos_MS.Set( fX + (fRedAndGreenWidth * nWidth), fY, 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				pVert->Pos_MS.Set( fX + (fGreenWidth * nWidth), fY + (0.006f * nHeight), 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				(*pVert) = *(pVert - 1);
				pVert++;
				(*pVert) = *(pVert - 3);
				pVert++;
				pVert->Pos_MS.Set( fX + (fRedAndGreenWidth * nWidth), fY + (0.006f * nHeight), 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				
				Color.Set( 0.f, 0.8f, 0.f, 0.8f * fAlphaAdjust );
				pVert->Pos_MS.Set( fX, fY, 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				pVert->Pos_MS.Set( fX + (fGreenWidth * nWidth), fY, 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				pVert->Pos_MS.Set( fX, fY + (0.006f * nHeight), 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				(*pVert) = *(pVert - 1);
				pVert++;
				(*pVert) = *(pVert - 3);
				pVert++;
				pVert->Pos_MS.Set( fX + (fGreenWidth * nWidth), fY + (0.006f * nHeight), 0.9f );
				pVert->ColorRGBA = Color;
				pVert++;
				
				if ( _PerfTrackers[i].fValue == _PERF_TRACKING_BAR_MAX_VALUE )
				{
					Color.Set( 0.9f, 0.1f, 0.1f, 0.9f * fAlphaAdjust );
				}
				else if ( _PerfTrackers[i].fValue >= 1.f )
				{
					Color.Set( 0.6f, 0.f, 0.f, 0.8f * fAlphaAdjust );
				}
				else if ( _PerfTrackers[i].fValue > 0.9 )
				{
					Color.Set( 0.8f, 0.7f, 0.f, 0.8f * fAlphaAdjust );
				}
				else
				{
					Color.Set( 0.f, 0.7f, 0.f, 0.8f * fAlphaAdjust );
				}
				
				fX = (_PERF_TRACKING_BAR_X_START * nWidth);
				fY = (_PERF_TRACKING_BAR_Y_START * nHeight) + (0.006f * nHeight) + (nActiveEntries * _PERF_TRACKING_BAR_SPACING * nHeight);
				pVert->Pos_MS.Set( fX, fY, 1.f );
				pVert->ColorRGBA = Color;
				pVert++;
				pVert->Pos_MS.Set( fX + (fGreenWidth * nWidth * _PerfTrackers[i].fValue), fY, 1.f );
				pVert->ColorRGBA = Color;
				pVert++;
				pVert->Pos_MS.Set( fX, fY + (0.0146f * nHeight), 1.f );
				pVert->ColorRGBA = Color;
				pVert++;
				(*pVert) = *(pVert - 1);
				pVert++;
				(*pVert) = *(pVert - 3);
				pVert++;
				pVert->Pos_MS.Set( fX + (fGreenWidth * nWidth * _PerfTrackers[i].fValue), fY + (0.0146f * nHeight), 1.f );
				pVert->ColorRGBA = Color;
				pVert++;

				fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, Verts, 18 );
				pVert = &Verts[0];
				
				// Draw the label:
				sprintf( szText, "~w0~ar~C%02d%02d%02d%02d%s", 99, 99, 99, (u32)(99.f * fAlphaAdjust), _PerfTrackers[i].szLabel );
				ftext_DebugPrintf( fTextXPos + 0.07f, fTextYPos + (fTextYSpacing * nActiveEntries), szText );
				
			}
			else if ( _PerfTrackers[i].nType == _PERF_TRACK_TYPE_INT )
			{
				// Draw the numbers
				sprintf( szText, "~w1~al~C%02d%02d%02d%02d%s %d", 99, 99, 99, (u32)(99.f * fAlphaAdjust), _PerfTrackers[i].szLabel, _PerfTrackers[i].nValue );
				ftext_DebugPrintf( fTextXPos + 0.02f, fTextYPos + (fTextYSpacing * nActiveEntries), szText );
			}
			else if ( _PerfTrackers[i].nType == _PERF_TRACK_TYPE_STRING )
			{
				// Draw the text
				sprintf( szText, "~w1~al~C%02d%02d%02d%02d%s %s", 99, 99, 99, (u32)(99.f * fAlphaAdjust), _PerfTrackers[i].szLabel, _PerfTrackers[i].szText );
				ftext_DebugPrintf( fTextXPos + 0.02f, fTextYPos + (fTextYSpacing * nActiveEntries), szText );
			}
			else if ( _PerfTrackers[i].nType == _PERF_TRACK_TYPE_FLOAT )
			{
				// Draw the numbers
				sprintf( szText, "~w1~al~C%02d%02d%02d%02d%s %.02f", 99, 99, 99, (u32)(99.f * fAlphaAdjust), _PerfTrackers[i].szLabel, _PerfTrackers[i].fValue );
				ftext_DebugPrintf( fTextXPos + 0.02f, fTextYPos + (fTextYSpacing * nActiveEntries), szText );
			}

			nActiveEntries++;
		}
	}
	
	for ( i = 0; i < _MAX_PERF_TRACKERS; i++ )
	{
		if ( _PerfTrackers[i].fLife >= 0.f )
		{
			_PerfTrackers[i].fLife -= FLoop_fRealPreviousLoopSecs;
			if ( _PerfTrackers[i].fLife < 0.f )
			{
				_PerfTrackers[i].fLife = -FMATH_MAX_FLOAT;
			}
		}
	}

	// Restore original state
	frenderer_Pop();
	fviewport_SetActive( pPreviousVP );
}
		
#endif

// 
//
//
void fperf_RenderPerf( void )
{
#if !_PERF_TRACKING_DISPLAY_ENABLED
	return;
#else

	static f32 __fAverageFrameTime = 16;
	static f32 __fLowAverageFrameTime = FMATH_MAX_FLOAT;
	static f32 __fHighAverageFrameTime = -FMATH_MAX_FLOAT;

	FASSERT( _bModuleInitialized );
	
	if ( !FViewport_pDefaultOrtho || !FVid_bOnline || (FPerf_nDisplayPerfType == FPERF_TYPE_NONE) )
	{
		return;
	}
	
#if FANG_PLATFORM_GC | FANG_PLATFORM_XB
	FDS_StreamMgr.DrawStatus();
#endif
	
/*
	fperf_Render_AddPerfNumber( "Tex Switches:", FPerf_nRawTexSwitchCount, TRUE );
	fperf_Render_AddPerfNumber( "ShSwitches:", FPerf_nRawShSwitchCount, TRUE );
	fperf_Render_AddPerfNumber( "MatSwitches:", FPerf_nMeshMtlDrawnCount, TRUE );
*/

	_Render();
#endif
}

