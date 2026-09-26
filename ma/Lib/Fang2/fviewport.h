//////////////////////////////////////////////////////////////////////////////////////
// fviewport.h - Fang viewport module.
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
// 09/13/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FVIEWPORT_H_
#define _FVIEWPORT_H_ 1

#include "fang.h"
#include "fmath.h"


struct FTexDef_t;


#define FVIEWPORT_DEFAULT_HALFFOVX		FMATH_QUARTER_PI
#define FVIEWPORT_DEFAULT_NEARZ			1.0f
#define FVIEWPORT_DEFAULT_FARZ			1000.0f


#define FVIEWPORT_MIN_HALFFOVX			0.0001f		// Minimum allowed value for FViewport_t::fHalfFOVX
#define FVIEWPORT_MAX_HALFFOVX			(FMATH_HALF_PI-0.0001f)

#define FVIEWPORT_MIN_NEAR_Z			0.0001f		// Minimum allowed value for FViewport_t::fNearZ
#define FVIEWPORT_MIN_NEAR_FAR_DELTA	0.0001f		// Minimum allowed delta from FViewport_t::fFarZ to FViewport_t::fNearZ


enum {
	FVIEWPORT_CLEARFLAG_COLOR		= 0x00000001,	// Set to clear the color buffer
	FVIEWPORT_CLEARFLAG_DEPTH		= 0x00000002,	// Set to clear the depth buffer
	FVIEWPORT_CLEARFLAG_STENCIL		= 0x00000004,	// Set to clear the stencil buffer

	FVIEWPORT_CLEARFLAG_ALL			= (FVIEWPORT_CLEARFLAG_COLOR | FVIEWPORT_CLEARFLAG_DEPTH | FVIEWPORT_CLEARFLAG_STENCIL),
	FVIEWPORT_CLEARFLAG_NONE		= 0x00000000
};

typedef enum {
	FVIEWPORT_TYPE_PERSPECTIVE,		// 3D perspective viewport. Origin is in center of render target
	FVIEWPORT_TYPE_ORTHO3D,			// 2D orthographic viewport. Origin is in cetner of render target
	FVIEWPORT_TYPE_ORTHO2D,			// 2D orthographic viewport. Origin is in upper-left of render target

	FVIEWPORT_TYPE_COUNT
} FViewportType_e;


#if FANG_PLATFORM_DX
	class CFDX8ViewportIS;
#elif FANG_PLATFORM_GC
	class CFGCViewportIS;
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	class CFPS2ViewportIS;
//ARG - <<<<<
#else
	#error <Fang Error: No viewport defined for this platform.>
#endif


typedef enum {
	FVIEWPORT_DIR_IN,			// Inward pointing
	FVIEWPORT_DIR_OUT,			// Outward pointing

	FVIEWPORT_DIR_COUNT
} FViewportDir_e;


typedef enum {
	FVIEWPORT_SPACE_VS,			// View space
	FVIEWPORT_SPACE_WS,			// World space
	FVIEWPORT_SPACE_MS,			// Model space

	FVIEWPORT_SPACE_COUNT
} FViewportSpace_e;


struct FViewport_t{
	FViewportType_e nType;		// Viewport type
	u32 uViewportKey;			// Updated by the IS code to indicate that there's been a change...

	void *pUserData;			// For application's use

	FTexDef_t *pTexDef;			// NULL=color backbuffer, otherwise specifies a render target texture

	f32 fHalfFOVX;				// Perspective projection half horizontal field-of-view in radians (0.0f=orthographic projection)
	f32 fFullFOVX;				// Perspective projection full horizontal field-of-view in radians (0.0f=orthographic projection)
	f32 fHalfFOVY;				// Perspective projection half vertical field-of-view in radians (0.0f=orthographic projection)
	f32 fFullFOVY;				// Perspective projection full vertical field-of-view in radians (0.0f=orthographic projection)

	f32 fNearZ;					// Nearplane Z (must be > 0.0f)
	f32 fFarZ;					// Farplane Z (must be > fNearZ)

	u32 nWidth;					// Width, in pixels, of the viewable viewport region
	u32 nHeight;				// Height, in pixels, of the viewable viewport region
	u32 nScreenLeftX;			// X screen coordinate of the viewable left edge of the viewport region
	u32 nScreenTopY;			// Y screen coordinate of the viewable top edge of the viewport region
	u32 nScreenRightX;			// Same as nScreenLeftX + nWidth
	u32 nScreenBottomY;			// Same as nScreenTopY + nHeight
	CFRect2D ScreenCorners;		// Same as above, but in floating point vector form
	CFVec2 Res;					// Same as nWidth and nHeight, but in floating point vector form ([0]=width, [1]=height)
	CFVec2 HalfRes;				// Res/2
	CFVec2 OORes;				// 1.0f / Res
	CFVec2 OOHalfRes;			// 1.0f / HalfRes
	f32 fAspectWOH;				// Resolution aspect, width/height
	f32 fAspectHOW;				// Resolution aspect, height/width
	f32 fTanHalfFOVX;			// Tan( fHalfFOVX )
	f32 fTanHalfFOVY;			// Tan( fHalfFOVY )
	f32 fVPtoWindowPixelRatio;	// Ratio of viewport pixels to window pixels at fviewport_Init() time

	// Unit normals: [][][0]=right, [][][1]=left, [][][2]=bottom, [][][3]=top, [][][4]=far, [][][5]=near
	CFVec3 aaaUnitNorm[FVIEWPORT_DIR_COUNT][FVIEWPORT_SPACE_COUNT][6];

	// [0][]: A point on the frustum plane in view space
	// [1][]: A point on the frustum plane in world space
	// [2][]: A point on the frustum plane in model space
	CFVec3 aaPointOnPlane[FVIEWPORT_SPACE_COUNT][6];

	u32 anNormCalcViewKey[FVIEWPORT_DIR_COUNT];			// Snapshot value of FXfm_nViewKey of when aaaUnitNorm[][FVIEWPORT_SPACE_WS] was last computed
	u32 anNormCalcModelViewKey[FVIEWPORT_DIR_COUNT];	// Snapshot value of FXfm_nModelViewKey of when aaaUnitNorm[][FVIEWPORT_SPACE_MS] was last computed

	#if FANG_PLATFORM_DX
		CFDX8ViewportIS *pViewportIS;	// Pointer to implementation-specific object data
	#elif FANG_PLATFORM_GC
		CFGCViewportIS *pViewportIS;	// Pointer to implementation-specific object data
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		CFPS2ViewportIS *pViewportIS;	// Pointer to implementation-specific object data
//ARG - <<<<<
	#else
		#error <Fang Error: No viewport defined for this platform.>
	#endif
};


// Bitmask of planes:
//   bit 0: Right frustum plane
//   bit 1: Left frustum plane
//   bit 2: Bottom frustum plane
//   bit 3: Top frustum plane
//   bit 4: Far frustum plane
//   bit 5: Near frustum plane
typedef s32 FViewportPlanesMask_t;		// See FVIEWPORT_PLANESMASK_* for info

enum {
	FVIEWPORT_PLANESMASK_RIGHT	= 0x00000001,
	FVIEWPORT_PLANESMASK_LEFT	= 0x00000002,
	FVIEWPORT_PLANESMASK_BOTTOM	= 0x00000004,
	FVIEWPORT_PLANESMASK_TOP	= 0x00000008,
	FVIEWPORT_PLANESMASK_FAR	= 0x00000010,
	FVIEWPORT_PLANESMASK_NEAR	= 0x00000020,

	FVIEWPORT_PLANESMASK_ALL	= (FVIEWPORT_PLANESMASK_RIGHT|FVIEWPORT_PLANESMASK_LEFT|FVIEWPORT_PLANESMASK_BOTTOM|FVIEWPORT_PLANESMASK_TOP|FVIEWPORT_PLANESMASK_FAR|FVIEWPORT_PLANESMASK_NEAR),
	FVIEWPORT_PLANESMASK_NONE	= 0x00000000,
};

typedef enum {
	FVIEWPORT_PLANE_RIGHT,
	FVIEWPORT_PLANE_LEFT,
	FVIEWPORT_PLANE_BOTTOM,
	FVIEWPORT_PLANE_TOP,
	FVIEWPORT_PLANE_FAR,
	FVIEWPORT_PLANE_NEAR,

	FVIEWPORT_PLANE_COUNT
} FViewportPlane_e;

typedef void (*fViewport_Callback)(FViewport_t *pCurrentView);

extern fViewport_Callback FViewport_CallbackFunc;


extern FViewport_t *FViewport_pDefaultOrtho;


// Note:
//
// A Persp viewport has its origin in the center of the screen with screen X/Y incrementing toward the upper-right
// corner of the screen. A circle rendered in a Persp viewport appears as a uniform circle.
//
// An Ortho3D viewport has its origin in the center of the screen with screen X/Y incrementing toward the upper-right
// corner of the screen. A circle rendered in an Ortho3D viewport appears as a uniform circle.
//
// An Ortho2D viewport has its origin in the upper-left center of the screen with screen X/Y incrementing toward the lower-right
// corner of the screen. A circle rendered in an Ortho2D viewport has a 1:1 pixels-per-unit aspect and so it may or may
// not appear as a uniform circle depending on whether or not the screen pixels are square.


extern BOOL fviewport_ModuleStartup( void );
extern void fviewport_ModuleShutdown( void );

extern FViewport_t *fviewport_Create( void );

extern void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef=NULL );
extern void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef=NULL );
extern void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef=NULL );
extern void fviewport_InitPersp( FViewport_t *pViewport, f32 fHalfFOVX, f32 fHalfFOVY, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef=NULL );

extern void fviewport_InitOrtho3D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef=NULL );
extern void fviewport_InitOrtho3D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef=NULL );
extern void fviewport_InitOrtho2D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, FTexDef_t *pTexDef=NULL );
extern void fviewport_InitOrtho2D( FViewport_t *pViewport, f32 fNearZ, f32 fFarZ, u32 nScreenLeftX, u32 nScreenTopY, u32 nWidth, u32 nHeight, FTexDef_t *pTexDef=NULL );

extern BOOL fviewport_IsInit( FViewport_t *pViewport );

extern FViewport_t *fviewport_SetActive( FViewport_t *pViewport );
extern FViewport_t *fviewport_GetActive( void );
extern FViewport_t *fviewport_GetActive_Absolute( void );

extern void fviewport_Clear( void );
extern void fviewport_Clear( f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue );
extern void fviewport_Clear( u32 nClearFlags, f32 fUnitRed, f32 fUnitGreen, f32 fUnitBlue, f32 fDepthValue, u32 nStencilValue );

extern void fviewport_ComputeNormals( FViewport_t *pViewport, FViewportDir_e nDir, FViewportSpace_e nSpace );

extern FViewportPlanesMask_t fviewport_TestSphere_SS( FViewport_t *pViewport, FViewportSpace_e nSphereSpace, const CFSphere *pBoundSphere, FViewportPlanesMask_t nCrossesPlanesMask );
extern FViewportPlanesMask_t fviewport_TestSphere_MS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_MS, FViewportPlanesMask_t nCrossesPlanesMask );
extern FViewportPlanesMask_t fviewport_TestSphere_WS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_WS, FViewportPlanesMask_t nCrossesPlanesMask );
extern FViewportPlanesMask_t fviewport_TestSphere_VS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_VS, FViewportPlanesMask_t nCrossesPlanesMask );

extern f32 fviewport_ComputeOrtho2DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS );
extern f32 fviewport_ComputeOrtho3DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS );
extern f32 fviewport_ComputeUnitOrtho3DScreenPoint_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFVec3A *pPoint_WS, CFVec3A *pPoint_SS );
extern BOOL fviewport_ComputeScreenPointAndSize_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFSphere *pSphere_WS, CFSphere *pResult_SS );
extern f32 fviewport_ComputeScreenRadius_WS( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, const CFSphere *pSphere_WS );

extern const CFMtx44A *fviewport_GetPlatformSpecificProjectionMatrix( const FViewport_t *pViewport );

//Called whenever the viewport changes.
extern void fviewport_SetCallback(fViewport_Callback pCallback);

extern f32 fviewport_fCullDistOverride; //set to 0 for no override

#endif

