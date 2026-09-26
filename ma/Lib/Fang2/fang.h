//////////////////////////////////////////////////////////////////////////////////////
// fang.h - Master Fang header file. Must be included by all Fang modules.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 08/29/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FANG_H_
#define _FANG_H_ 1



//------------------------------------------------------------------------------------------------
// Preprocessor flags recognized by the Fang system.
//
//
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//
// INPUT FLAGS:
//
// Define these in the global project settings.
//
// Build Flags (See below for definition of build types.):
//   _DEBUG:						Define for debug builds.
//   _FANGDEF_TEST_BUILD:			Define for test builds.
//   _FANGDEF_RELEASE_BUILD:		Define for release builds.
//   _FANGDEF_PRODUCTION_BUILD:		Define for production builds.
//
// Platform Flags (Only one is permitted to be defined. If none are defined, _FANGDEF_PLATFORM_WIN is assumed.):
//   _FANGDEF_PLATFORM_WIN:			Windows platform (compiler is assumed to be MSVC or Intel)
//   _FANGDEF_PLATFORM_XB:			Xbox platform (compiler is assumed to be MSVC or Intel)
//   _FANGDEF_PLATFORM_PS2:			Sony PlayStation2 platform (compiler is assumed to be CodeWarrior)
//   _FANGDEF_PLATFORM_GC:			Nintendo Gamecube platform (compiler is assumed to be CodeWarrior)
//
// Misc Flags (Any combination is allowed to be defined.):
//   _FANGDEF_NO_INLINE_CODE:		Define to disable the inlining of functions
//   _FANGDEF_SIMD:					Define to compile in SIMD instructions
//   _FANGDEF_NO_DEVPRINTF:			Define to have compiler skip over DEVPRINTF() statements
//   _FANGDEF_NO_FASSERT:			Define to have compiler skip over FASSERT() statements
//   _FANGDEF_NO_MEMLEAK_DETECT:	Define to disable Fang's Malloc memory leak detection
//   _FANGDEF_PAL_VIDEO:			Define to build a version of Fang for PAL TVs
//   _FANGDEF_WINGC:				Can be used only with _FANGDEF_PLATFORM_WIN. Specifies to use GC alignment. Used for tools.
//
//
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//
// BUILD FLAG DESCRIPTION:
//
// Flag							DEVPRINTF			FASSERT/checks		Debug Compile		Purpose
// -------------------------    ------------------  ------------------	------------------	-----------
// _DEBUG						Yes					Yes					Yes					Debugging
// _FANGDEF_TEST_BUILD			Yes					Yes					No					Testing
// _FANGDEF_RELEASE_BUILD		Yes					No					No					Speed
// _FANGDEF_PRODUCTION_BUILD	No					No					No					Production
//
// Note that only one Build Flag is permitted to be defined.
// If no build flags are defined, _FANGDEF_RELEASE_BUILD is assumed.
//
//
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
//
// OUTPUT FLAGS:
//
// fang.h takes the above input flags and derives the following flags from them:
//
// FANG_DEBUG_BUILD			TRUE for a debug build type
// FANG_TEST_BUILD			TRUE for a test build type
// FANG_RELEASE_BUILD		TRUE for a release build type
// FANG_PRODUCTION_BUILD	TRUE for a production build type
//
// FANG_PLATFORM_WIN		TRUE for Windows platform
// FANG_PLATFORM_XB			TRUE for the Microsoft Xbox platform
// FANG_PLATFORM_PS2		TRUE for the Sony Playstation2 platform
// FANG_PLATFORM_GC			TRUE for the Nintendo Gamecube platform
// FANG_PLATFORM_DX			= (FANG_PLATFORM_WIN | FANG_PLATFORM_XB)
// FANG_PLATFORM_CONSOLE	= (FANG_PLATFORM_XB | FANG_PLATFORM_PS2 | FANG_PLATFORM_GC )
//
// FANG_COMPILER_MSVC		TRUE for a Microsoft or Intel compiler (set for WIN and XB platforms)
// FANG_COMPILER_CW			TRUE for the CodeWarrior compiler
//
// FANG_ENABLE_INLINE_CODE	TRUE if function inlining is enabled
// FANG_ENABLE_SIMD			TRUE if SIMD instructions should be used
// FANG_ENABLE_DEVPRINTF	TRUE if the DEVPRINTF macros are enabled
// FANG_ENABLE_FASSERT		TRUE if the FASSERT macros are enabled
// FANG_CODE_OPTIMIZED		TRUE if compiler code optimizations are turned on (non-debug builds only)
// FANG_DETECT_MEMLEAKS		TRUE if memory leaks through fang_Malloc() are to be tracked (always FALSE for production builds)
// FANG_PAL_VIDEO			TRUE if this is a PAL TV build of Fang
// FANG_WINGC				TRUE if _FANGDEF_WINGC is specified on a FANG_PLATFORM_WIN build to indicate the use of GC alignment and no SSE
//
//
// In addition, fang.h defines these macros:
//
// FINLINE
// DEVPRINTF() and DEVPRINTF()
// FASSERT and its derivatives
//
//
//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
// Platform:

#if !defined(_FANGDEF_PLATFORM_WIN) && !defined(_FANGDEF_PLATFORM_XB) && !defined(_FANGDEF_PLATFORM_PS2) && !defined(_FANGDEF_PLATFORM_GC)
	// No platform flags set. Assuming Windows platform by default...
	#define _FANGDEF_PLATFORM_WIN
#endif


#if defined(_FANGDEF_PLATFORM_WIN)
	#define FANG_PLATFORM_WIN 1
#else
	#define FANG_PLATFORM_WIN 0
#endif


#if defined(_FANGDEF_PLATFORM_XB)
	#define FANG_PLATFORM_XB 1
#else
	#define FANG_PLATFORM_XB 0
#endif


#if defined(_FANGDEF_PLATFORM_PS2)
//ARG - >>>>>
	#include "bolt3d.h"

	using namespace MMI;
//ARG - <<<<<
	#define FANG_PLATFORM_PS2 1
	
	//turn on (non-standard, but uber-cool) anonymous structs/unions/classes, and the like...
//ARG	#pragma cpp_extensions on
#else
	#define FANG_PLATFORM_PS2 0
#endif


#if defined(_FANGDEF_PLATFORM_GC)
	#define FANG_PLATFORM_GC 1
	
	//turn on (non-standard, but uber-cool) anonymous structs/unions/classes, and the like...
	#pragma cpp_extensions on
#else
	#define FANG_PLATFORM_GC 0
#endif


#if (FANG_PLATFORM_WIN + FANG_PLATFORM_XB + FANG_PLATFORM_PS2 + FANG_PLATFORM_GC) > 1
	#error <Fang Error: Multiple platform flags defined.>
#endif

#define FANG_PLATFORM_DX		(FANG_PLATFORM_WIN | FANG_PLATFORM_XB)
#define FANG_PLATFORM_CONSOLE	(FANG_PLATFORM_XB | FANG_PLATFORM_PS2 | FANG_PLATFORM_GC)



//------------------------------------------------------------------------------------------------
// Compiler:

#if FANG_PLATFORM_DX
	#define FANG_COMPILER_MSVC 1
	#define FANG_COMPILER_CW 0
	#define FANG_COMPILER_SN 0	//ARG
#elif FANG_PLATFORM_PS2
	#define FANG_COMPILER_MSVC 0
//ARG	#define FANG_COMPILER_CW 1
	#define FANG_COMPILER_CW 0	//ARG
	#define FANG_COMPILER_SN 1	//ARG
#elif FANG_PLATFORM_GC
	#define FANG_COMPILER_MSVC 0
	#define FANG_COMPILER_CW 1
	#define FANG_COMPILER_SN 0	//ARG
#else
	#error <Fang Error: Compiler undefined for this platform.>
#endif




//------------------------------------------------------------------------------------------------
// Build type:

#if !defined(_DEBUG) && !defined(_FANGDEF_TEST_BUILD) &&  !defined(_FANGDEF_RELEASE_BUILD) && !defined(_FANGDEF_PRODUCTION_BUILD)
	// No build flags set. Assuming release build by default...
	#define _FANGDEF_RELEASE_BUILD
#endif


#ifdef _DEBUG
	#define FANG_DEBUG_BUILD 1
#else
	#define FANG_DEBUG_BUILD 0
#endif


#ifdef _FANGDEF_TEST_BUILD
	#define FANG_TEST_BUILD 1
#else
	#define FANG_TEST_BUILD 0
#endif


#ifdef _FANGDEF_RELEASE_BUILD
	#define FANG_RELEASE_BUILD 1
#else
	#define FANG_RELEASE_BUILD 0
#endif


#ifdef _FANGDEF_PRODUCTION_BUILD
	#define FANG_PRODUCTION_BUILD 1
#else
	#define FANG_PRODUCTION_BUILD 0
#endif


#if (FANG_DEBUG_BUILD + FANG_TEST_BUILD + FANG_RELEASE_BUILD + FANG_PRODUCTION_BUILD) == 0
	#error <Fang Error: No build flags defined.>
#endif


#if (FANG_DEBUG_BUILD + FANG_TEST_BUILD + FANG_RELEASE_BUILD + FANG_PRODUCTION_BUILD) > 1
	#error <Fang Error: Multiple build flags defined.>
#endif




//------------------------------------------------------------------------------------------------
// Option flags:

#if !FANG_DEBUG_BUILD
	#define FANG_CODE_OPTIMIZED 1
#else
	#define FANG_CODE_OPTIMIZED 0
#endif


#ifdef _FANGDEF_NO_INLINE_CODE
	#define FANG_ENABLE_INLINE_CODE 0
#else
	#define FANG_ENABLE_INLINE_CODE 1
#endif


#ifdef _FANGDEF_SIMD
	#define FANG_ENABLE_SIMD 1
#else
	#define FANG_ENABLE_SIMD 0
#endif


#if defined(_FANGDEF_NO_DEVPRINTF) || FANG_PRODUCTION_BUILD
	#define FANG_ENABLE_DEVPRINTF 0
#else
	#define FANG_ENABLE_DEVPRINTF 1
#endif

#if defined(_FANGDEF_NO_FASSERT) || FANG_PRODUCTION_BUILD || FANG_RELEASE_BUILD
	#define FANG_ENABLE_FASSERT 0
#else
	#define FANG_ENABLE_FASSERT 1
#endif


#if defined(_FANGDEF_NO_MEMLEAK_DETECT) || FANG_PRODUCTION_BUILD
	#define FANG_DETECT_MEMLEAKS 0
#else
	#define FANG_DETECT_MEMLEAKS 1
#endif


#if defined(_FANGDEF_PAL_VIDEO)
	#define FANG_PAL_VIDEO 1
#else
	#define FANG_PAL_VIDEO 0
#endif


#if defined(_FANGDEF_WINGC)
	#if FANG_PLATFORM_WIN
		#define FANG_WINGC 1
	#else
		#define FANG_WINGC 0
		#error <_FANGDEF_WINGC may be specified only on Windows builds.>
	#endif
#else
	#define FANG_WINGC 0
#endif

// SET THIS TO TRUE TO ENABLE DEVELOPMENT FEATURES THAT SHOULD NOT BE INCLUDED IN THE FINAL RETAIL GAME
// SET TO FALSE WHEN COMPILING A RETAIL BUILD OF THE GAME
#if defined( _FANGDEF_ENABLE_DEV_FEATURES )
	#define FANG_ENABLE_DEV_FEATURES 1
#else
	#define FANG_ENABLE_DEV_FEATURES 0
#endif


//This allows lights with intensity higher then 1.0
#define FANG_LIGHTING_2X 1

#define FSH_DYNAMIC_SREFLECT 1


//------------------------------------------------------------------------------------------------

#include "fangtypes.h"
#include "fangmacros.h"

//------------------------------------------------------------------------------------------------
// Function inlining:

#if FANG_ENABLE_INLINE_CODE
	// Allow inline functions...

	#if FANG_COMPILER_MSVC
		#define FINLINE __forceinline
	#elif (FANG_COMPILER_CW)
		#define FINLINE inline
//ARG - >>>>>
	#elif FANG_COMPILER_SN
		#define FINLINE inline
//ARG - <<<<<
	#else
//ARG		#error <Fang Error: Don't know what the 'inline' specifier is for this compiler.>
		#error <Fang Error: Dont know what the inline specifier is for this compiler.>	//ARG
	#endif
#else
	// Don't allow inline functions...

	#define FINLINE
#endif




//------------------------------------------------------------------------------------------------
// DEVPRINTF definitions:

typedef void FangFcnPrintf_t( cchar *pszFormat, FANG_VA_LIST Args );


#if FANG_ENABLE_DEVPRINTF
	// Enable DEVPRINTF macros...
	#define DEVPRINTF fang_DevPrintf

	#if (FANG_PLATFORM_PS2)
		#include <stdio.h>
		#define fang_DevPrintf printf
	#elif (FANG_PLATFORM_GC)
		#include <dolphin.h>
		#define fang_DevPrintf OSReport
	#endif
#else
	#if (FANG_PLATFORM_PS2)
		#include <stdio.h>
		#define fang_DevPrintf printf
	#elif (FANG_PLATFORM_GC)
		#include <dolphin.h>
		#define fang_DevPrintf OSReport
	#endif

	// Disable DEVPRINTF macros...
	#define DEVPRINTF 1 ? (void)0 : (void)fang_DevPrintf
#endif




//------------------------------------------------------------------------------------------------
// FASSERT definitions:

#if FANG_ENABLE_FASSERT
#if FANG_PLATFORM_WIN
	// PC...

	#if FANG_TEST_BUILD
		#define FASSERT( bExpr ) if( !(bExpr) ) fang_Assert( __FILE__, __LINE__, #bExpr );
		#define FASSERT_MSG( bExpr, pszMsg ) if( !(bExpr) ) fang_Assert( __FILE__, __LINE__, pszMsg );
	#else
		#include <crtdbg.h>

		#define FASSERT( bExpr ) _ASSERTE( bExpr )
		#define FASSERT_MSG( bExpr, pszMsg ) \
			do { \
				if( !(bExpr) && (1 == _CrtDbgReport( _CRT_ASSERT, __FILE__, __LINE__, NULL, pszMsg )) ) { \
					_CrtDbgBreak(); \
				} \
			} while (0)
	#endif

#elif FANG_PLATFORM_XB
	// Xbox:

	#if FANG_TEST_BUILD
		#define FASSERT( bExpr ) if( !(bExpr) ) fang_Assert( __FILE__, __LINE__, #bExpr );
		#define FASSERT_MSG( bExpr, pszMsg ) if( !(bExpr) ) fang_Assert( __FILE__, __LINE__, pszMsg );
	#else
		#include <assert.h>
		#define FASSERT( bExpr ) assert( bExpr )
		#define FASSERT_MSG( bExpr, pszMsg ) assert( (pszMsg, bExpr) )
	#endif

#elif FANG_PLATFORM_PS2

	// PlayStation2...

	#include <assert.h>
	//#define FASSERT assert
	#define FASSERT(e)	if(!(e)) {printf("ASSERT! %s, %d, %s\n", __FILE__, __LINE__, #e);for(;;){}}
	#define FASSERT_MSG(a, b) assert((b, a));

#elif FANG_PLATFORM_GC

	// GameCube...

	#define FASSERT( bExpr ) if( !(bExpr) ) fang_Assert( __FILE__, __LINE__, #bExpr )
	#define FASSERT_MSG( bExpr, pszMsg ) if( !(bExpr) ) fang_Assert( __FILE__, __LINE__, pszMsg );

#else

	#error <Fang Error: FASSERT handling undefined for this platform.>

#endif

#else
	// Disable FASSERTs...
	#define FASSERT( bExpr ) ((void)0)
	#define FASSERT_MSG( bExpr, pszMsg ) ((void)0)

#endif

#define FASSERT_NOW							FASSERT( FALSE )
#define FASSERT_NOW_MSG( pszMsg )			FASSERT_MSG( FALSE, pszMsg )
#define FASSERT_UNIT_FLOAT( fVal )			FASSERT( ((fVal)>=0.0f && (fVal)<=1.0f) )
#define FASSERT_BIPOLAR_UNIT_FLOAT( fVal )	FASSERT( ((fVal)>=-1.0f && (fVal)<=1.0f) )
#define FASSERT_POS_FLOAT( fVal )			FASSERT( ((fVal)>=0.0f) )
#define FASSERT_NEG_FLOAT( fVal )			FASSERT( ((fVal)<0.0f ) )
#define FASSERT_POS_INT( nVal )				FASSERT( ((nVal)>=0 ) )
#define FASSERT_NEG_INT( nVal )				FASSERT( ((nVal)<0 ) )



//------------------------------------------------------------------------------------------------
// Fang configuration definitions:

typedef struct {
	FangFcnPrintf_t *pFang_FcnPrintf;		// Points to DEVPRINTF function handler (NULL=none)

	u32 nRes_HeapBytes;						// Amount of memory given to the FRes module for the Fang heap
	void *pRes_HeapStart;					// NULL to have FRes allocate the memory (default), or set to point to mem allocated by app

	u32 nAudio_MaxSoundBytes;				// Maximum number of sound bytes
	
	u32 nAMem_FastAuxiliaryMemoryBytes;		// Number of bytes of fast memory to allocate for the famem system in auxiliary memory (or main memory if it's not available)
	u32 nAMem_SlowAuxiliaryMemoryBytes;		// Number of bytes of slow memory to allocate for the famem system in auxiliary memory (or main memory if it's not available)

	u32 nViewport_MaxViewports;				// Maximum number of viewports that can exist simultaneously

	u32 nTex_MaxTextureMemBytes;			// Maximum number of bytes to store texture data (ignored for DX8)
	BOOL bTex_D3DNoSysLock;					// For D3D only: TRUE=Specify the D3DLOCK_NOSYSLOCK flag when locking textures

	u32 nMesh_MaxActiveLights;				// The maximum number of active lights that can be added via fmesh_AddLight()
	u32 nMesh_MaxCollSpheres;				// The maximum number of collision spheres that can be given to the mesh collision system in a single call

	u32 nColl_MaxImpacts;					// The maximum number of impacts that can be held in the impact buffer

	u32 nXfm_MaxModelStackXfms;				// Maximum number of model xfm's that the stack can hold

	u32 nWorld_NodeCacheBytes;				// Size in bytes of the node cache
	u32 nWorld_MaxDesiredLightsPerNode;		// World system will try to subdivide terrain down until the number of dynamic lights reaches this value or lower
	u32 nWorld_MaxSortedObjectsPerNode;		// The maximum number of objects produced per node when sorting is enabled by fworld_CollideRayWithTrackers()
	u32 nWorld_MaxLightsPerDraw;			// The maximum number of dynamic lights that can be used during a single call to fworld_Draw()
	u32 nWorld_MaxMeshesPerDraw;			// The maximum number of mesh instances that can be drawn during a single call to fworld_Draw()
	u32 nWorld_CollBufBytes;				// Collision work buffer bytes
	u32 nWorld_MaxIntersects;				// Maximum number of intersect objects

	u32 nRenderer_StackBytes;				// Maximum bytes used to store renderer state information between Push/Pop pairs

	cchar *pszFile_GameRootPathName;		// Pointer to the game's root directory
	cchar *pszFile_MasterFilePathName;		// Pointer to the master file's full pathname (if NULL, a master file is not used)
	cchar *pszMovie_BasePathName;			// Pointer to the full pathname for BINK Movies (if NULL, NO Movie functionality)

	u32 nPSprite_XBVBCount;					// For XB only: Number of vertex buffers to use
	u32 nPSprite_D3DVBVertexCount;			// For D3D only: Number of vertices the vertex buffers are to each hold

	u32 nDraw_XBVBCount;					// For XB only: Number of vertex buffers to use
	u32 nDraw_D3DVBVertexCount;				// For D3D only: Number of vertices the vertex buffers are to each hold

	u32 nShadow_D3DIndexBufferCount;		// For D3D only: Number of elements in the IB used to render shadowed triangles

	u32 nText_MaxCharsPerFrame;				// The maximum number of characters displayed between each text_Draw()
	u32 nText_MaxCharsPerPrintf;			// The character buffer size used when processing ftext_printf()'s

	u32 nPadio_MaxSamplesPerSec;			// The maximum number of samples polled from the input devices per second

	u32 nFile_MaxSimultaneousFilesOpen;		// The maximum number of files that can be open at once.

	u32 nForce_MaxActiveEnvelopes;			// The maximum number of force feedback envelopes that can be active at a time

	u32 nStringTable_DefaultMaxStringCountPerCluster;	// The default max number of strings in a cluster
	BOOL bStringTable_CaseSensativeTableNames;			// Use case-sensative comparisons of table names
	BOOL bStringTable_CaseSensativeStringNames;			// Use case-sensative comparisons of string names

	u32 nMaxParticleEmitters;				// What is the max number of particles that should be allocated
	u32 nMaxParticles;						// What is the max number of particles that should be allocated	
	u32 nMaxParticleEmitterSprites;			// What is the max number of sprites that any one particle emitter will need (this is the number of psprites that will be allocated by the particle system)

	u32 nVerlet_MaxCount;					// The maximum number of CFVerlet objects that can exist
	f32 fVerlet_MaxDeltaSecs;				// The maximum chunk of time to clamp FLoop_fPreviousLoopSecs to

	u32 nVtxPool_VertexCount;				// The number of FDrawVtx_t vertices to create for the vertex pool 

	BOOL bCheckPoint_StartupSystem;			// TRUE to start up the FCHECKPOINT system
	BOOL bExceptionHandler_StartupSystem;	// TRUE to start up the exception handler system

	u32 nLightGroup_nMaxVirtualLights;		// how many virtual lights should the light group mgr allocate

	u32 nLightPool_nNumLights;				// how many lights should be in the global fang light pool

	u32 nMaxDecals;							// maximum number of decals that can be drawn at once
	u32 nMaxDecalVertices;					// maximum number of vertices that can be used at once by the decal system

} FangConfigDef_t;




//------------------------------------------------------------------------------------------------
// General definitions:

typedef struct {
	void *pStart;		// Points to the first byte of the buffer (NULL=none)
	u32 nByteCount;		// Size of the buffer in bytes (0 is valid)
} FangBuf_t;


#define FANG_ZEROSTRUCT( StructName ) fang_MemZero( &(StructName), sizeof(StructName) )
#define FANG_COPYSTRUCT( DstStructName, SrcStructName ) \
					FASSERT( sizeof(DstStructName) == sizeof(SrcStructName) ); \
					fang_MemCopy( &(DstStructName), &(SrcStructName), sizeof(DstStructName) )



//------------------------------------------------------------------------------------------------
// Publicized functions and data:


extern FangConfigDef_t Fang_ConfigDefs;

extern void fang_Init( void );
extern BOOL fang_Startup( void );
extern void fang_Shutdown( void );
extern BOOL fang_IsStartedUp( void );

extern void fang_ResetHardware( void );

extern const char *fang_GetErrorString( void );

extern BOOL fang_SetValidationState( BOOL bEnable );
extern BOOL fang_GetValidationState( void );

extern void *fang_MemCopy( void *pDst, const void *pSrc, u32 nBytes );
extern BOOL fang_IsMemIdentical( const void *pSrc1, const void *pSrc2, u32 nBytes );
extern void *fang_MemZero( void *pDst, u32 nBytes );
extern void *fang_MemSet( void *pDst, u8 nValue, u32 nBytes );


// Functions to change the endian format of common types.
// If you pass in a big endian var, these functions return a little endian value.
// If you pass in a little endian var, these functions return a big endian value.
extern u64 fang_ConvertEndian( u64 num );
extern s64 fang_ConvertEndian( s64 num );
extern f32 fang_ConvertEndian( f32 num );
extern u32 fang_ConvertEndian( u32 num );
extern s32 fang_ConvertEndian( s32 num );
extern u16 fang_ConvertEndian( u16 num );
extern s16 fang_ConvertEndian( s16 num );
extern void* fang_ConvertEndian( void *ptr );
s8 fang_ConvertEndian( s8 num );
u8 fang_ConvertEndian( u8 num );
char fang_ConvertEndian( char num );



#if !FANG_DETECT_MEMLEAKS
	extern void *fang_Malloc( u32 nBytes, u32 nByteAlignment );
	extern void *fang_MallocAndZero( u32 nBytes, u32 nByteAlignment );
#else
	#define fang_Malloc( nBytes, nByteAlignment ) fang_MallocWithLeakDetection( __FILE__, __LINE__, nBytes, nByteAlignment )
	#define fang_MallocAndZero( nBytes, nByteAlignment ) fang_MallocAndZeroWithLeakDetection( __FILE__, __LINE__, nBytes, nByteAlignment )
	extern void *fang_MallocWithLeakDetection( cchar *pszFileName, int nLineNum, u32 nBytes, u32 nByteAlignment );
	extern void *fang_MallocAndZeroWithLeakDetection( cchar *pszFileName, int nLineNum, u32 nBytes, u32 nByteAlignment );
#endif
extern void fang_Free( void *pMemBlock );

#if !(FANG_PLATFORM_PS2) && !(FANG_PLATFORM_GC)
	extern int fang_DevPrintf( const char *pszFormat, ... );
#endif

extern void fang_Assert( cchar *pszFile, int nLine, cchar *pszExpr );


#if FANG_PLATFORM_GC || FANG_WINGC
	#define FANG_USE_PORTAL		TRUE
#else
	#define FANG_USE_PORTAL		TRUE//FALSE	
#endif

typedef enum
{
	FANG_LAUNCH_TYPE_UNKNOWN = 0,
	FANG_LAUNCH_TYPE_DEBUGGER,
	FANG_LAUNCH_TYPE_DEMO,
	FANG_LAUNCH_TYPE_STANDALONE,
} Fang_LaunchType_e;

extern Fang_LaunchType_e Fang_nLaunchType;

//------------------------------------------------------------------------------------------------
// Memory management definitions:
//------------------------------------------------------------------------------------------------

#include "fangalign.h"


#endif

