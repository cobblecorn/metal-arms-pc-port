//////////////////////////////////////////////////////////////////////////////////////
// fdraw.h - Fang drawing utility module.
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
// 09/14/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDRAW_H_
#define _FDRAW_H_ 1

#include "fang.h"
#include "fcolor.h"



//--------------------------------------------------------------------------------------------------------------------
// General Definitions:

#define FDRAW_MAX_CIRCLE_FACETS						(4*16)
#define FDRAW_DEFAULT_CIRCLE_FACETS_PER_QUARTER		8

class CFLight;
class CFTexInst;



//--------------------------------------------------------------------------------------------------------------------
// Vertex Definition:

struct FDrawVtx_t {				// Vertex:
	// Constructors:
	FDrawVtx_t() { }
	FDrawVtx_t(f32 fX, f32 fY, f32 fZ, f32 fR, f32 fG, f32 fB, f32 fA, f32 fS, f32 fT) : Pos_MS( fX, fY, fZ ), ColorRGBA( fR, fG, fB, fA), ST( fS, fT) {}
	FDrawVtx_t(CFVec3 v1, CFColorRGBA v2, CFVec2 v3) : Pos_MS( v1 ), ColorRGBA( v2 ), ST( v3 ) { }

	// Data:
	CFVec3 Pos_MS;				// Position in modelspace
	CFColorRGBA ColorRGBA;		// RGBA color (each component ranges from 0.0f to 1.0f)
	CFVec2 ST;					// Texture coordinates (0.0f to 1.0f for no wrapping)
};



//--------------------------------------------------------------------------------------------------------------------
// Draw Primitives:

typedef enum {
	FDRAW_PRIMTYPE_POINTS = 0,	// Each vertex renders one point
	FDRAW_PRIMTYPE_LINELIST,	// Every two vertices renders a separate line
	FDRAW_PRIMTYPE_LINESTRIP,	// Every N vertices renders N-1 connected lines
	FDRAW_PRIMTYPE_TRILIST,		// Every three vertices renders a separate triangle
	FDRAW_PRIMTYPE_TRISTRIP,	// Every N vertices renders N-2 connected triangles in strip form
//	FDRAW_PRIMTYPE_TRIFAN,		// Every N vertices renders N-2 connected triangles in fan form
	FDRAW_PRIMTYPE_QUADLIST,	// Every four vertices renders a separate quad

	FDRAW_PRIMTYPE_COUNT
} FDrawPrimType_e;




//--------------------------------------------------------------------------------------------------------------------
// Cull Direction:

typedef enum {					// Cull Mode:
	FDRAW_CULLDIR_CW = 0,		// Cull triangles with a clockwise screenspace vertex specification
	FDRAW_CULLDIR_CCW,			// Cull triangles with a counter-clockwise screenspace vertex specification
	FDRAW_CULLDIR_NONE,			// Don't cull any triangles

	FDRAW_CULLDIR_COUNT
} FDrawCullDir_e;




//--------------------------------------------------------------------------------------------------------------------
// Color Combiner Function:

typedef enum {								// (Note: t=texture, i=iterated from vtx)
	FDRAW_COLORFUNC_DECAL_AI,				// C = Ci,					A = Ai
	FDRAW_COLORFUNC_DECALTEX_AI,			// C = Ct,					A = Ai
	FDRAW_COLORFUNC_DECALTEX_AT,			// C = Ct,					A = At
	FDRAW_COLORFUNC_DIFFUSETEX_AI,			// C = Ci*Ct,				A = Ai
	FDRAW_COLORFUNC_DIFFUSETEX_AT,			// C = Ci*Ct,				A = At
	FDRAW_COLORFUNC_DIFFUSETEX_AIAT,		// C = Ci*Ct,				A = Ai*At
	FDRAW_COLORFUNC_SPECULARTEX_AT,			// C = Ci*Ct + Ai,			A = At
	FDRAW_COLORFUNC_ADD,					// C = Ci + Ct,				A = Ai + At
	FDRAW_COLORFUNC_BLEND_AIPLUSAT,			// C = LERP( Ai, Ct, Ci ),	A = Ai + At
	FDRAW_COLORFUNC_CT_PLUS_CIAT_AI,		// C = Ct + At*Ci,			A = Ai

	//New func
	FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY,	// C = Ct (r=g=b),			A = Ai
	//

	FDRAW_COLORFUNC_COUNT
} FDrawColorFunc_e;

enum _FDrawCapColorFunc_e {									// Note: Maintain (1<<FDrawColorFunc_e)==_FDrawCapColorFunc_e
	FDRAW_CAP_COLORFUNC_DECAL_AI			= 0x00000001,	// This bit is always set
	FDRAW_CAP_COLORFUNC_DECALTEX_AI			= 0x00000002,	// Device supports FDRAW_COLORFUNC_DECALTEX_AI
	FDRAW_CAP_COLORFUNC_DECALTEX_AT			= 0x00000004,	// Device supports FDRAW_COLORFUNC_DECALTEX_AT
	FDRAW_CAP_COLORFUNC_DIFFUSETEX_AI		= 0x00000008,	// Device supports FDRAW_COLORFUNC_DIFFUSETEX_AI
	FDRAW_CAP_COLORFUNC_DIFFUSETEX_AT		= 0x00000010,	// Device supports FDRAW_COLORFUNC_DIFFUSETEX_AT
	FDRAW_CAP_COLORFUNC_DIFFUSETEX_AIAT		= 0x00000020,	// Device supports FDRAW_COLORFUNC_DIFFUSE_AIAT
	FDRAW_CAP_COLORFUNC_SPECULARTEX_AT		= 0x00000040,	// Device supports FDRAW_COLORFUNC_DIFFUSETEX_AIAT
	FDRAW_CAP_COLORFUNC_ADD					= 0x00000080,	// Device supports FDRAW_COLORFUNC_ADD
	FDRAW_CAP_COLORFUNC_BLEND_AIPLUSAT		= 0x00000100,	// Device supports FDRAW_COLORFUNC_BLEND_AIPLUSAT
	FDRAW_CAP_COLORFUNC_CT_PLUS_CIAT_AI		= 0x00000200,	// Device supports FDRAW_COLORFUNC_CT_PLUS_CIAT_AI
	FDRAW_CAP_COLORFUNC_DECALTEX_AI_INTENSITY = 0x00000400,	// Device supports FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY

	FDRAW_CAP_COLORFUNC_ALL					= FDRAW_CAP_COLORFUNC_DECAL_AI
											| FDRAW_CAP_COLORFUNC_DECALTEX_AI
											| FDRAW_CAP_COLORFUNC_DECALTEX_AT
											| FDRAW_CAP_COLORFUNC_DIFFUSETEX_AI
											| FDRAW_CAP_COLORFUNC_DIFFUSETEX_AT
											| FDRAW_CAP_COLORFUNC_DIFFUSETEX_AIAT
											| FDRAW_CAP_COLORFUNC_SPECULARTEX_AT
											| FDRAW_CAP_COLORFUNC_ADD
											| FDRAW_CAP_COLORFUNC_BLEND_AIPLUSAT
											| FDRAW_COLORFUNC_CT_PLUS_CIAT_AI
											| FDRAW_COLORFUNC_DECALTEX_AI_INTENSITY
};
typedef u32 FDrawCapColorFunc_e;			// Represents the flag-based enum for _FDrawCapColorFunc_e


//--------------------------------------------------------------------------------------------------------------------
// Fdraw color function to shader mapping:

struct FDrawShaderMap_t {
	u32 nFdrawColorFunc;
	u32 nShader;
};

extern FDrawShaderMap_t FDraw_ColorFuncToShaderMap[FDRAW_COLORFUNC_COUNT];



//--------------------------------------------------------------------------------------------------------------------
// Alpha Test:

typedef enum {
	FDRAW_ALPHATEST_ALWAYS,					// Always pass
	FDRAW_ALPHATEST_NEVER,					// Never pass
	FDRAW_ALPHATEST_LESS,					// Pass if the fragment value is less than the reference value
	FDRAW_ALPHATEST_LEQUAL,					// Pass if the fragment value is less than or equal to the reference value
	FDRAW_ALPHATEST_EQUAL,					// Pass if the fragment value is equal to the reference value
	FDRAW_ALPHATEST_NOTEQUAL,				// Pass if the fragment value is unequal to the reference value
	FDRAW_ALPHATEST_GEQUAL,					// Pass if the fragment value is greater than or equal the reference value
	FDRAW_ALPHATEST_GREATER,				// Pass if the fragment value is greater than the reference value

	FDRAW_ALPHATEST_COUNT
} FDrawAlphaTest_e;

enum _FDrawCapAlphaTest_e {									// Note: Maintain (1<<FDrawAlphaTest_e)==_FDrawCapAlphaTest_e
	FDRAW_CAP_ALPHATEST_ALWAYS				= 0x00000001,	// This bit is always set
	FDRAW_CAP_ALPHATEST_NEVER				= 0x00000002,	// Device supports FDRAW_ALPHATEST_NEVER
	FDRAW_CAP_ALPHATEST_LESS				= 0x00000004,	// Device supports FDRAW_ALPHATEST_LESS
	FDRAW_CAP_ALPHATEST_LEQUAL				= 0x00000008,	// Device supports FDRAW_ALPHATEST_LEQUAL
	FDRAW_CAP_ALPHATEST_EQUAL				= 0x00000010,	// Device supports FDRAW_ALPHATEST_EQUAL
	FDRAW_CAP_ALPHATEST_NOTEQUAL			= 0x00000020,	// Device supports FDRAW_ALPHATEST_NOTEQUAL
	FDRAW_CAP_ALPHATEST_GEQUAL				= 0x00000040,	// Device supports FDRAW_ALPHATEST_GEQUAL
	FDRAW_CAP_ALPHATEST_GREATER				= 0x00000080,	// Device supports FDRAW_ALPHATEST_GREATER

	FDRAW_CAP_ALPHATEST_ALL					= FDRAW_CAP_ALPHATEST_ALWAYS
											| FDRAW_CAP_ALPHATEST_NEVER
											| FDRAW_CAP_ALPHATEST_LESS
											| FDRAW_CAP_ALPHATEST_LEQUAL
											| FDRAW_CAP_ALPHATEST_EQUAL
											| FDRAW_CAP_ALPHATEST_NOTEQUAL
											| FDRAW_CAP_ALPHATEST_GEQUAL
											| FDRAW_CAP_ALPHATEST_GREATER
};
typedef u32 FDrawCapAlphaTest_e;			// Represents the flag-based enum for _FDrawCapAlphaTest_e




//--------------------------------------------------------------------------------------------------------------------
// Alpha Blending:

typedef enum {									// Notes: C = resulting color
												//        Cs = fragment color
												//        Cd = color already in frame buffer
												//        As = fragment alpha
												//        (all above values range from 0 to 1)

	FDRAW_BLENDOP_SRC,							// C = Cs
	FDRAW_BLENDOP_DST,							// C = Cd
	FDRAW_BLENDOP_ZERO,							// C = 0
	FDRAW_BLENDOP_SRC_PLUS_DST,					// C = Cs + Cd
	FDRAW_BLENDOP_ALPHA_TIMES_SRC,				// C = As*Cs
	FDRAW_BLENDOP_ALPHA_TIMES_DST,				// C = As*Cd
	FDRAW_BLENDOP_INVALPHA_TIMES_SRC,			// C = (1-As)*Cs
	FDRAW_BLENDOP_INVALPHA_TIMES_DST,			// C = (1-As)*Cd
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// C = As*Cs + Cd
	FDRAW_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC,		// C = As*Cd + Cs
	FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE,		// C = As*Cs + (1-As)*Cd
	FDRAW_BLENDOP_LERP_WITH_ALPHA_XPARENT,		// C = As*Cd + (1-As)*Cs
	
	FDRAW_BLENDOP_COUNT
} FDrawBlendOp_e;

enum _FDrawCapBlendOp_e {						// Note: Maintain (1<<FDrawBlendOp_e)==_FDrawCapBlendOp_e
	FDRAW_CAP_BLENDOP_SRC						= 0x00000001,	// This bit is always set
	FDRAW_CAP_BLENDOP_DST						= 0x00000002,	// Device supports FDRAW_BLENDOP_DST
	FDRAW_CAP_BLENDOP_ZERO						= 0x00000004,	// Device supports FDRAW_BLENDOP_ZERO
	FDRAW_CAP_BLENDOP_SRC_PLUS_DST				= 0x00000008,	// Device supports FDRAW_BLENDOP_SRC_PLUS_DST
	FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC			= 0x00000010,	// Device supports FDRAW_BLENDOP_ALPHA_TIMES_SRC
	FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST			= 0x00000020,	// Device supports FDRAW_BLENDOP_ALPHA_TIMES_DST
	FDRAW_CAP_BLENDOP_INVALPHA_TIMES_SRC		= 0x00000040,	// Device supports FDRAW_BLENDOP_INVALPHA_TIMES_SRC
	FDRAW_CAP_BLENDOP_INVALPHA_TIMES_DST		= 0x00000080,	// Device supports FDRAW_BLENDOP_INVALPHA_TIMES_DST
	FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST	= 0x00000100,	// Device supports FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST
	FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC	= 0x00000200,	// Device supports FDRAW_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC
	FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_OPAQUE	= 0x00000400,	// Device supports FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE
	FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_XPARENT	= 0x00000800,	// Device supports FDRAW_BLENDOP_LERP_WITH_ALPHA_XPARENT

	FDRAW_CAP_BLENDOP_ALL						= FDRAW_CAP_BLENDOP_SRC
												| FDRAW_CAP_BLENDOP_DST
												| FDRAW_CAP_BLENDOP_ZERO
												| FDRAW_CAP_BLENDOP_SRC_PLUS_DST
												| FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC
												| FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST
												| FDRAW_CAP_BLENDOP_INVALPHA_TIMES_SRC
												| FDRAW_CAP_BLENDOP_INVALPHA_TIMES_DST
												| FDRAW_CAP_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST
												| FDRAW_CAP_BLENDOP_ALPHA_TIMES_DST_PLUS_SRC
												| FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_OPAQUE
												| FDRAW_CAP_BLENDOP_LERP_WITH_ALPHA_XPARENT
};
typedef u32 FDrawCapBlendOp_e;					// Represents the flag-based enum for _FDrawCapBlendOp_e




//--------------------------------------------------------------------------------------------------------------------
// Depth Buffer Test:

typedef enum {
	FDRAW_DEPTHTEST_ALWAYS,						// Test always passes
	FDRAW_DEPTHTEST_NEVER,						// Test never passes
	FDRAW_DEPTHTEST_CLOSER,						// Test passes if the pixel is closer to the viewer
	FDRAW_DEPTHTEST_CLOSER_OR_EQUAL,			// Test passes if the pixel is closer or the same distance to the viewer
	FDRAW_DEPTHTEST_EQUAL,						// Test passes if the pixel's depth is equal to that of the existing pixel
	FDRAW_DEPTHTEST_NOTEQUAL,					// Test passes if the pixel's depth is not equal to that of the existing pixel
	FDRAW_DEPTHTEST_FARTHER,					// Test passes if the pixel is farther from the viewer
	FDRAW_DEPTHTEST_FARTHER_OR_EQUAL,			// Test passes if the pixel is farther or the same distance to the viewer

	FDRAW_DEPTHTEST_COUNT
} FDrawDepthTest_e;

enum _FDrawCapDepthTest_e {									// Note: Maintain (1<<FDrawDepthTest_e)==_FDrawCapDepthTest_e
	FDRAW_CAP_DEPTHTEST_ALWAYS				= 0x00000001,	// This bit is always set
	FDRAW_CAP_DEPTHTEST_NEVER				= 0x00000002,	// Device supports FDRAW_DEPTHTEST_NEVER
	FDRAW_CAP_DEPTHTEST_CLOSER				= 0x00000004,	// Device supports FDRAW_DEPTHTEST_CLOSER
	FDRAW_CAP_DEPTHTEST_CLOSER_OR_EQUAL		= 0x00000008,	// Device supports FDRAW_DEPTHTEST_CLOSER_OR_EQUAL
	FDRAW_CAP_DEPTHTEST_EQUAL				= 0x00000010,	// Device supports FDRAW_DEPTHTEST_EQUAL
	FDRAW_CAP_DEPTHTEST_NOTEQUAL			= 0x00000020,	// Device supports FDRAW_DEPTHTEST_NOTEQUAL
	FDRAW_CAP_DEPTHTEST_FARTHER				= 0x00000040,	// Device supports FDRAW_DEPTHTEST_FARTHER
	FDRAW_CAP_DEPTHTEST_FARTHER_OR_EQUAL	= 0x00000080,	// Device supports FDRAW_DEPTHTEST_FARTHER_OR_EQUAL

	FDRAW_CAP_DEPTHTEST_ALL					= FDRAW_CAP_DEPTHTEST_ALWAYS
											| FDRAW_CAP_DEPTHTEST_NEVER
											| FDRAW_CAP_DEPTHTEST_CLOSER
											| FDRAW_CAP_DEPTHTEST_CLOSER_OR_EQUAL
											| FDRAW_CAP_DEPTHTEST_EQUAL
											| FDRAW_CAP_DEPTHTEST_NOTEQUAL
											| FDRAW_CAP_DEPTHTEST_FARTHER
											| FDRAW_CAP_DEPTHTEST_FARTHER_OR_EQUAL
};
typedef u32 FDrawCapDepthTest_e;			// Represents the flag-based enum for _FDrawCapDepthTest_e




//--------------------------------------------------------------------------------------------------------------------
// Stencil Operation:

typedef enum {
	FDRAW_STENCILTEST_ALWAYS,				// Always pass
	FDRAW_STENCILTEST_NEVER,				// Never pass
	FDRAW_STENCILTEST_LESS,					// Pass if the fragment value is less than the reference value
	FDRAW_STENCILTEST_LEQUAL,				// Pass if the fragment value is less than or equal to the reference value
	FDRAW_STENCILTEST_EQUAL,				// Pass if the fragment value is equal to the reference value
	FDRAW_STENCILTEST_NOTEQUAL,				// Pass if the fragment value is unequal to the reference value
	FDRAW_STENCILTEST_GEQUAL,				// Pass if the fragment value is greater than or equal the reference value
	FDRAW_STENCILTEST_GREATER,				// Pass if the fragment value is greater than the reference value

	FDRAW_STENCILTEST_COUNT
} FDrawStencilTest_e;

typedef enum {
	FDRAW_STENCILOP_KEEP,					// Don't modify the stencil buffer value
	FDRAW_STENCILOP_ZERO,					// Zero the stencil buffer value
	FDRAW_STENCILOP_REPLACE,				// Replace the stencil buffer value
	FDRAW_STENCILOP_INC_CLAMP,				// Increment the stencil buffer value, clamping to the maximum allowed value
	FDRAW_STENCILOP_DEC_CLAMP,				// Decrement the stencil buffer value, clamping to zero
	FDRAW_STENCILOP_INVERT,					// Bitwise-invert the stencil buffer value

	FDRAW_STENCILOP_COUNT
} FDrawStencilOp_e;




//--------------------------------------------------------------------------------------------------------------------
// Texture:

typedef enum {
	FDRAW_TEXCOORDXFMMODE_NONE,						// Use unmodified FDrawVtx_t texture coordinates
	FDRAW_TEXCOORDXFMMODE_MTXONLY,					// Apply TC transformation matrix to FDrawVtx_t texture coordinates
	FDRAW_TEXCOORDXFMMODE_CAMSPACE_NORMAL,			// Use the camera space normal vector as the texture coordinates (Mtx is also applied)
	FDRAW_TEXCOORDXFMMODE_CAMSPACE_POS,				// Use the camera space position as the texture coordinates (Mtx is also applied)
	FDRAW_TEXCOORDXFMMODE_CAMSPACE_REFLECTION,		// Use the camera space reflection vector as the texture coordinates (Mtx is also applied)

	FDRAW_TEXCOORDXFMMODE_COUNT
} FDrawTexCoordXfmMode_e;




//--------------------------------------------------------------------------------------------------------------------
// Default States:

#define FDRAW_DEFAULT_CULLDIR						FDRAW_CULLDIR_NONE
#define FDRAW_DEFAULT_CLIPENABLE					TRUE
#define FDRAW_DEFAULT_COLORFUNC						FDRAW_COLORFUNC_DECAL_AI
#define FDRAW_DEFAULT_TEXMIPBIAS					0.0f
#define FDRAW_DEFAULT_COLORMASK_RED					FALSE
#define FDRAW_DEFAULT_COLORMASK_GREEN				FALSE
#define FDRAW_DEFAULT_COLORMASK_BLUE				FALSE
#define FDRAW_DEFAULT_DITHERENABLE					FALSE
#define FDRAW_DEFAULT_ALPHATEST						FDRAW_ALPHATEST_ALWAYS
#define FDRAW_DEFAULT_ALPHAREF						0
#define FDRAW_DEFAULT_BLENDOP						FDRAW_BLENDOP_SRC
#define FDRAW_DEFAULT_DEPTHWRITEENABLE				TRUE
#define FDRAW_DEFAULT_DEPTHTEST						FDRAW_DEPTHTEST_CLOSER_OR_EQUAL
#define FDRAW_DEFAULT_DEPTHBIAS						0
#define FDRAW_DEFAULT_TEXCOORDXFMMODE				FDRAW_TEXCOORDXFMMODE_NONE
#define FDRAW_DEFAULT_STENCIL_TEST					FDRAW_STENCILTEST_ALWAYS
#define FDRAW_DEFAULT_STENCIL_REF					0
#define FDRAW_DEFAULT_STENCIL_TESTMASK				0
#define FDRAW_DEFAULT_STENCIL_WRITEMASK				0
#define FDRAW_DEFAULT_STENCIL_STENCILFAILOP			FDRAW_STENCILOP_KEEP
#define FDRAW_DEFAULT_STENCIL_DEPTHFAILOP			FDRAW_STENCILOP_KEEP
#define FDRAW_DEFAULT_STENCIL_DEPTHPASSOP			FDRAW_STENCILOP_KEEP




//--------------------------------------------------------------------------------------------------------------------
// Global Information:

extern FDrawCapColorFunc_e	FDraw_nCapColorFunc;	// Bits indicating which color combine functions are supported
extern FDrawCapAlphaTest_e	FDraw_nCapAlphaTest;	// Bits indicating which alpha tests are supported
extern FDrawCapBlendOp_e	FDraw_nCapBlendOp;		// Bits indicating which blend operations are supported
extern FDrawCapDepthTest_e	FDraw_nCapDepthTest;	// Bits indicating which depth tests are supported

extern BOOL FDraw_nDepthBitDepth;					// Number of depth bits (0=depth buffer not supported)
extern u32 FDraw_nStencilBitDepth;					// Number of stencil bits (0=stencil buffer not supported)
extern u32 FDraw_nStencilBitMask;					// Mask with the valid stencil bits set

extern BOOL FDraw_bCanDisableDepthWrites;			// TRUE=writing to depth buffer can be disabled
extern u32 FDraw_nMaxDepthBiasValue;				// The maximum depth bias value supported (0=depth biasing not supported)




//---------------------------------------------------------------------------------------------------------------------
// fdraw_ModuleStartup()
// fdraw_ModuleShutdown()
//
// Starts up and shuts down this module, respectively.
// Called by the Fang initialization system.
// Returns TRUE if successful, or FALSE if the module could not successfully initialized.
extern BOOL fdraw_ModuleStartup( void );
extern void fdraw_ModuleShutdown( void );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Point()
// fdraw_Line()
// fdraw_Triangle()
// fdraw_PrimList()
//
// Draws the specified primitive using the current rendering state and viewport.
// The supplied vertices are in modelspace and are transformed by the current camera
// and model transformation matrices (see fxfm module).
extern void fdraw_Point( const FDrawVtx_t *pVtx1 );
extern void fdraw_Line( const FDrawVtx_t *pVtx1, const FDrawVtx_t *pVtx2 );
extern void fdraw_Triangle( const FDrawVtx_t *pVtx1, const FDrawVtx_t *pVtx2, const FDrawVtx_t *pVtx3 );
extern void fdraw_PrimList( FDrawPrimType_e nPrimType, const FDrawVtx_t *paVertices, u32 nVtxCount );
extern void fdraw_IndexedPrimList( FDrawPrimType_e nPrimType, const FDrawVtx_t *paVertices, const u16 *paIndices, u32 nIndexCount );

extern void fdraw_FacetedCircle( const CFVec3 *pUnitNormal_MS, const CFVec3 *pCenter_MS, f32 fRadius_MS, CFColorRGBA *pVtxColorRGBA=&FColor_MotifWhite, u32 nVtxCountPerQuarter=FDRAW_DEFAULT_CIRCLE_FACETS_PER_QUARTER );
extern void fdraw_FacetedWireSphere( const CFVec3 *pCenter_MS, f32 fRadius_MS, u32 nSliceCountPerHalf, u32 nRingCountPerQuarter=1, CFColorRGBA *pVtxColorRGBA=&FColor_MotifWhite, u32 nVtxCountPerQuarter=FDRAW_DEFAULT_CIRCLE_FACETS_PER_QUARTER );
extern void fdraw_FacetedWireSphere( const CFVec3 *pCenter_MS, f32 fRadius_MS, CFColorRGBA *pVtxColorRGBA=&FColor_MotifWhite, u32 nVtxCountPerQuarter=FDRAW_DEFAULT_CIRCLE_FACETS_PER_QUARTER );
extern void fdraw_FacetedCylinder( const CFVec3 *pBottomCenter_MS, const CFVec3 *pUnitNormal_MS, f32 fRadius_MS, f32 fHeight_MS, CFColorRGBA *pVtxColorRGBA=&FColor_MotifWhite, u32 nVtxCountPerQuarter=FDRAW_DEFAULT_CIRCLE_FACETS_PER_QUARTER );

extern void fdraw_ModelSpaceAxis( f32 fScale = 1.0f );
extern void fdraw_SolidPoint( const CFVec3 *pPos_MS, const CFColorRGBA *pColor = &CFColorRGBA( 1.0f, 1.0f, 1.0f, 1.0f ) );
extern void fdraw_SolidLine( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFColorRGBA *pColor = &CFColorRGBA( 1.0f, 1.0f, 1.0f, 1.0f ) );
extern void fdraw_SolidLine( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFColorRGBA *pColor1, const CFColorRGBA *pColor2 );
extern void fdraw_SolidTriangle( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFVec3 *pPos3_MS, const CFColorRGBA *pColor = &CFColorRGBA( 1.0f, 1.0f, 1.0f, 1.0f ) );
extern void fdraw_SolidQuad( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFVec3 *pPos3_MS, const CFVec3 *pPos4_MS, const CFColorRGBA *pColor = &CFColorRGBA( 1.0f, 1.0f, 1.0f, 1.0f ) );
extern void fdraw_TexQuad( const CFVec3 *pPos1_MS, const CFVec3 *pPos2_MS, const CFVec3 *pPos3_MS, const CFVec3 *pPos4_MS, const CFColorRGBA *pColor = &CFColorRGBA( 1.0f, 1.0f, 1.0f, 1.0f ) );

extern void fdraw_ClearDepth_Cache();

#if !FANG_PRODUCTION_BUILD
	extern void fdraw_DevCapsule( const CFCapsule *pCapsule_WS, CFColorRGBA *pColor=&FColor_MotifWhite, u32 nSliceCountPerHalf=3, u32 nRingCountPerQuarter=3, u32 nVtxCountPerQuarter=3 );
	extern void fdraw_DevSphere( const CFVec3 *pCenter_WS, f32 fRadius_WS, CFColorRGBA *pColor=&FColor_MotifWhite, u32 nSliceCountPerHalf=3, u32 nRingCountPerQuarter=3, u32 nVtxCountPerQuarter=3 );
	extern void fdraw_DevTriangle( const CFVec3 *pPoint0_WS, const CFVec3 *pPoint1_WS, const CFVec3 *pPoint2_WS, CFColorRGBA *pColor=&FColor_MotifWhite );
	extern void fdraw_DevLine( const CFVec3 *pPoint0_WS, const CFVec3 *pPoint1_WS, CFColorRGBA *pColor1=&FColor_MotifWhite, CFColorRGBA *pColor2=&FColor_MotifWhite );
	extern void fdraw_RenderDevShapes( void );
	extern void fdraw_ClearDevShapes( void );
#endif


//---------------------------------------------------------------------------------------------------------------------
// fdraw_IsColorMaskSupported()
//
// Pass in the desired color components to mask out, and this function will return TRUE
// if that mask is supported by the hardware.
extern BOOL fdraw_IsColorMaskSupported( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_IsStencilModeSupported()
//
// Pass in the desired stencil operating mode and this function will return TRUE if
// the hardware supports it.
extern BOOL fdraw_IsStencilModeSupported( FDrawStencilTest_e nStencilTest, FDrawStencilOp_e nStencilFailOp,
										  FDrawStencilOp_e nDepthFailOp, FDrawStencilOp_e nDepthPassOp );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_SetCullDir()
// fdraw_GetCullDir()
//
// Sets and gets the cull state for triangle rendering. Ignored for points and lines.
// The Set function returns the prior state.
extern void fdraw_SetCullDir( FDrawCullDir_e nCullDir );
extern FDrawCullDir_e fdraw_GetCullDir( void );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_EnableClipping()
// fdraw_IsClippingEnabled()
//
// Indicates whether or not clipping is required on primitives. Set to FALSE for optimal
// performance when you know the primitive is entirely within the frustum.
// The EnableClipping function returns the prior state.
extern void fdraw_EnableClipping( BOOL bEnable );
extern BOOL fdraw_IsClippingEnabled( void );


extern void fdraw_Color_SetFunc( FDrawColorFunc_e nColorFunc );
extern FDrawColorFunc_e fdraw_Color_GetFunc( void );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_SetTexture()
// fdraw_GetTexture()
//
// Sets and gets the current texture. If CFTexInst is NULL, primitives are rendered
// with a solid color and opaqueness specified by the vertex color. Otherwise, the vertex
// color is modulated by the texture color. The SetTexture function returns the prior state.
extern void fdraw_SetTexture( CFTexInst *pTexInst );
extern void fdraw_GetTexture( CFTexInst *pDestTexInst );



extern void fdraw_SetTexCoordXfmMode( FDrawTexCoordXfmMode_e nTexCoordXfmMode );
extern FDrawTexCoordXfmMode_e fdraw_GetTexCoordXfmMode( void );
extern void fdraw_SetTexCoordXfmMtx( const CFMtx43A *pMtx43A );
extern void fdraw_GetTexCoordXfmMtx( CFMtx43A *pMtx43A );



//---------------------------------------------------------------------------------------------------------------------
// fdraw_Color_SetMask()
// fdraw_Color_GetMask()
//
// Sets and gets the current RGB mask state. Set the parameters to TRUE to disable writing
// to the associated color component of the color buffer, and FALSE to enable writing.
// To determine whether the hardware supports color masking, use fdraw_IsColorMaskSupported().
extern void fdraw_Color_SetMask( BOOL bMaskOffRed, BOOL bMaskOffGreen, BOOL bMaskOffBlue );
extern void fdraw_Color_GetMask( BOOL *pbMaskOffRed, BOOL *pbMaskOffGreen, BOOL *pbMaskOffBlue );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Color_EnableDither()
// fdraw_Color_IsDitherEnabled()
//
// Sets and gets the dither state. Ignored if the hardware doesn't support the requested
// value. The EnableDither function returns the prior state.
extern void fdraw_Color_EnableDither( BOOL bEnable );
extern BOOL fdraw_Color_IsDitherEnabled( void );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Alpha_SetTest
// fdraw_Alpha_GetTest
//
// Sets and gets the current alpha test state. A test condition and reference value are provided.
// Ignored if the hardware doesn't support the requested alpha test condition.
// Use FDraw_nCapAlphaTest to determine which alpha test conditions are supported by the hardware.
extern void fdraw_Alpha_SetTest( FDrawAlphaTest_e nAlphaTest, float fReference );
extern FDrawAlphaTest_e fdraw_Alpha_GetTest( float *pfReference=NULL );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Alpha_SetBlendOp()
// fdraw_Alpha_GetBlendOp()
//
// Sets and gets the current alpha blending operation.
// Ignored if the hardware doesn't support the requested alpha blend function.
// Use FDraw_nCapBlendOp to determine which blending operations are supported by the hardware.
// The Set function returns the prior state.
extern void fdraw_Alpha_SetBlendOp( FDrawBlendOp_e nBlendOp );
extern FDrawBlendOp_e fdraw_Alpha_GetBlendOp( void );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Depth_EnableWriting
// fdraw_Depth_IsWritingEnabled
//
// Sets and gets the depth buffer writing mask state. Ignored if the hardware doesn't permit
// disabling writes to the depth buffer. Use FDraw_bCanDisableDepthWrites to determine
// whether disabling writes to the depth buffer is permitted.
// The EnableWriting function returns the prior state.
extern void fdraw_Depth_EnableWriting( BOOL bEnable );
extern BOOL fdraw_Depth_IsWritingEnabled( void );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Depth_SetTest()
// fdraw_Depth_GetTest()
//
// Sets and gets the current depth buffer test state. Ignored if the hardware doesn't support
// the requested test. Use FDraw_nCapDepthTest to determine which depth test conditions
// are supported by the hardware. The Set function returns the prior state.
extern void fdraw_Depth_SetTest( FDrawDepthTest_e nDepthTest );
extern FDrawDepthTest_e fdraw_Depth_GetTest( void );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Depth_SetBiasLevel()
// fdraw_Depth_GetBiasLevel()
// fdraw_Depth_DeltaBiasLevel()
//
// These functions provide a way to add an offset to the Z value that's compared and written
// to the depth buffer. A bias level of 0 is normal. The more positive the bias value, the more
// the triangles will appear in front of coplanar triangles. Triangles with a depth bias of 1
// appear in front of triangles with a depth bias of 0. Triangles with a depth bias of 2 appear
// in front of triangles with a depth bias of 1, etc. The maximum allowed value is hardware-
// specific, and is clamped by Fang. Negative bias values are not allowed.
//
// Use FDraw_nMaxDepthBiasValue to determine the maximum supported value (0 indicates that depth
// biasing is not supported).
//
// Use the Set and Get functions to set and get the current bias level. The Set function returns
// the prior value.
//
// Use the Delta function to add to the current depth level. The prior value is returned.
extern void fdraw_Depth_SetBiasLevel( u32 nNewLevel );
extern u32 fdraw_Depth_GetBiasLevel( void );
extern void fdraw_Depth_DeltaBiasLevel( s32 nDeltaLevel );




//---------------------------------------------------------------------------------------------------------------------
// fdraw_Stencil_SetMode()
// fdraw_Stencil_GetMode()
//
// Sets and gets the current stencil state. Ignored if the hardware doesn't support the
// requested state. Use fdraw_IsStencilModeSupported() to determine whether a stencil
// mode is supported by the hardware.
extern void fdraw_Stencil_SetMode( FDrawStencilTest_e nStencilTest, u32 nReference, u32 nTestMask, u32 nWriteMask,
								   FDrawStencilOp_e nStencilFailOp, FDrawStencilOp_e nDepthFailOp,
								   FDrawStencilOp_e nDepthPassOp );
extern void fdraw_Stencil_GetMode( FDrawStencilTest_e *pnStencilTest, u32 *pnReference, u32 *pnTestMask,
								   u32 *pnWriteMask, FDrawStencilOp_e *pnStencilFailOp,
								   FDrawStencilOp_e *pnDepthFailOp, FDrawStencilOp_e *pnDepthPassOp );


extern BOOL fdraw_ComputeBoundingSphere( const FDrawVtx_t *paVtx, u32 nVtxCount, CFSphere &rSphere );
extern BOOL fdraw_ComputeMinMaxBox( const FDrawVtx_t *paVtx, u32 nVtxCount, CFVec3 &rMin, CFVec3 &rMax );


extern void fdraw_Renderer_Open( void );
extern void fdraw_Renderer_Close( void );
extern u32 fdraw_Renderer_GetStateSize( void );
extern void fdraw_Renderer_GetState( void *pDestState );
extern void fdraw_Renderer_SetState( const void *pState );
extern void fdraw_Renderer_SetDefaultState( void );

extern void fdraw_RenderCorona(const CFMtx43A *pCamera, CFLight *pLight, float fCamDistSq, u32 nFlags, u32 nPhase);
extern void fdraw_RenderScrQuad(const CFMtx43A *pCamera, CFVec3 *pLoc, CFTexInst *pTex, float fScale);

extern void fdraw_SetLineWidth(f32 fWidth);

FINLINE void fdraw_TransformVerts( FDrawVtx_t* paDestVerts,const FDrawVtx_t* paSourceVerts, const CFMtx43A& rModelMtx, u32 uCount)
{
	FASSERT(paDestVerts);
	FASSERT(paSourceVerts);
	FASSERT(uCount < ((u16)-1)); // Sanity check

	while(uCount--)
	{
		rModelMtx.MulPoint(paDestVerts[uCount].Pos_MS,paSourceVerts[uCount].Pos_MS);
	}
}

// Array of temporary vert indices to be used for drawing indexed primitives
extern u16 *FDraw_paTempVertIndices;
// Maximum amount of indices that can be placed in the FDraw_paTempVertIndices
extern u16 FDraw_nTempVertIndicesArraySize;

#endif

