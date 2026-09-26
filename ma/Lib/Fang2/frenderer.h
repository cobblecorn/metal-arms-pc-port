//////////////////////////////////////////////////////////////////////////////////////
// frenderer.h - Master Fang render control.
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
// 04/05/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FRENDERER_H_
#define _FRENDERER_H_ 1

#include "fang.h"
#include "fcolor.h"



const f32 FRENDERER_DEFAULT_FOG_STARTZ	= 0.0f;		// Default fog start Z
const f32 FRENDERER_DEFAULT_FOG_ENDZ	= 1000.0f;	// Default fog end Z


typedef enum {
	FRENDERER_NONE = 0,				// Don't use any system
	FRENDERER_MESH,					// Prepare to use fmesh system
	FRENDERER_DRAW,					// Prepare to use fdraw system
	FRENDERER_COLL,					// Prepare to use collision system
	FRENDERER_PSPRITE,				// Prepare to use psprite system
	FRENDERER_SHADOW,				// Prepare to use shadow system

	FRENDERER_COUNT
} FRenderer_e;


typedef enum {
	FRENDERER_FILLMODE_NORMAL,		// Normal triangles
	FRENDERER_FILLMODE_WIREFRAME,	// Triangle edges
	FRENDERER_FILLMODE_POINT,		// Triangle vertices

	FRENDERER_FILLMODE_COUNT
} FRendererFillMode_e;


typedef struct {
	BOOL bEnable;					// TRUE if fog is enabled
	f32 fStartZ;					// Distance at which fogging begins
	f32 fEndZ;						// Distance at which fogging is at maximum value
	CFColorMotif Motif;				// Fog motif
} FRendererFogState_t;




// These are globally available for read only.
// Don't modify them directly:
extern FRendererFillMode_e FRenderer_nFillMode;	// Specifies the current polygon fill mode

extern BOOL FRenderer_bDrawBoundInfo;			// TRUE to draw bounding info
extern BOOL FRenderer_bDrawBoneInfo;			// TRUE to draw bone info

extern BOOL FRenderer_bFogEnabled;				// TRUE if fog is enabled
extern f32 FRenderer_fFogStartZ;				// Z distance at which fog begins
extern f32 FRenderer_fFogEndZ;					// Z distance at which fog is at maximum
extern f32 FRenderer_fOOFogEndMinusFogStartZ;	// 1.0f / (FRenderer_fFogEndZ - FRenderer_fFogStartZ)
extern CFColorMotif FRenderer_FogMotif;			// Fog motif
extern CFColorRGB FRenderer_FogColorRGB;		// Computed fog color (set by calling frenderer_Fog_ComputeColor)
extern u32 FRenderer_nFogChangedKey;			// Incremented each time start, end, or motif is changed

#if FANG_PLATFORM_DX
	extern u32 FRenderer_FogD3DColor;
	extern u32 FRenderer_nD3DFillMode;
	extern const u32 FRenderer_anD3DFillMode[FRENDERER_FILLMODE_COUNT];
#endif





extern BOOL frenderer_ModuleStartup( void );
extern void frenderer_ModuleShutdown( void );

extern FRenderer_e frenderer_GetActive( void );
extern u32 frenderer_GetStateSize( FRenderer_e nRenderer = frenderer_GetActive() );

extern FRenderer_e frenderer_Push( FRenderer_e nNewRenderer );
extern FRenderer_e frenderer_Push( FRenderer_e nNewRenderer, void *pNewState );
extern FRenderer_e frenderer_Pop( void );
extern void frenderer_PopAll( void );

extern void frenderer_GetState( void *pDestState );
extern void frenderer_SetState( const void *pState );
extern void frenderer_SetDefaultState( void );

extern FRendererFillMode_e frenderer_FillMode_Set( FRendererFillMode_e nFillMode );
extern FRendererFillMode_e frenderer_FillMode_Get( void );

extern BOOL frenderer_DrawBound_Enable( BOOL bEnable );
extern BOOL frenderer_DrawBound_IsEnabled( void );

extern BOOL frenderer_DrawBones_Enable( BOOL bEnable );
extern BOOL frenderer_DrawBones_IsEnabled( void );

extern void frenderer_Fog_Enable( BOOL bEnable=TRUE );
extern void frenderer_Fog_SetRange( f32 fStartZ=FRENDERER_DEFAULT_FOG_STARTZ, f32 fEndZ=FRENDERER_DEFAULT_FOG_ENDZ );
extern void frenderer_Fog_SetMotif( const CFColorMotif *pMotif=&FColor_MotifWhite );
extern void frenderer_Fog_SetState( const FRendererFogState_t *pFogState );
extern BOOL frenderer_Fog_IsEnabled( void );
extern void frenderer_Fog_GetRange( f32 *pfStartZ, f32 *pfEndZ );
extern void frenderer_Fog_GetMotif( CFColorMotif *pMotif );
extern void frenderer_Fog_GetState( FRendererFogState_t *pDestFogState );
extern void frenderer_Fog_ComputeColor( void );




#endif

