//////////////////////////////////////////////////////////////////////////////////////
// frenderer.cpp - Master Fang render control.
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

#include "fang.h"
#include "frenderer.h"
#include "fres.h"
#include "fmath.h"

#if FANG_PLATFORM_DX
	#include "dx\\fdx8.h"
#endif


FRendererFillMode_e FRenderer_nFillMode;

BOOL FRenderer_bDrawBoundInfo;
BOOL FRenderer_bDrawBoneInfo;

BOOL FRenderer_bFogEnabled;
f32 FRenderer_fFogStartZ;
f32 FRenderer_fFogEndZ;
f32 FRenderer_fOOFogEndMinusFogStartZ;
CFColorMotif FRenderer_FogMotif;
CFColorRGB FRenderer_FogColorRGB;
u32 FRenderer_nFogChangedKey;

#if FANG_PLATFORM_DX
	u32 FRenderer_FogD3DColor;
	u32 FRenderer_nD3DFillMode;

	const u32 FRenderer_anD3DFillMode[FRENDERER_FILLMODE_COUNT] = {
		D3DFILL_SOLID,
		D3DFILL_WIREFRAME,
		D3DFILL_POINT
	};
#endif






typedef void _FcnOpen( void );
static void _None_Renderer_Open( void );
extern void fmesh_Renderer_Open( void );
extern void fdraw_Renderer_Open( void );
extern void fcoll_Renderer_Open( void );
extern void fpsprite_Renderer_Open( void );
//extern void fshadow_Renderer_Open( void );
static _FcnOpen *_aFcnOpen[FRENDERER_COUNT] = {
	_None_Renderer_Open,
	fmesh_Renderer_Open,
	fdraw_Renderer_Open,
	fcoll_Renderer_Open,
	fpsprite_Renderer_Open,
//	fshadow_Renderer_Open,
};

typedef void _FcnClose( void );
static void _None_Renderer_Close( void );
extern void fmesh_Renderer_Close( void );
extern void fdraw_Renderer_Close( void );
extern void fcoll_Renderer_Close( void );
extern void fpsprite_Renderer_Close( void );
//extern void fshadow_Renderer_Close( void );
static _FcnClose *_aFcnClose[FRENDERER_COUNT] = {
	_None_Renderer_Close,
	fmesh_Renderer_Close,
	fdraw_Renderer_Close,
	fcoll_Renderer_Close,
	fpsprite_Renderer_Close,
//	fshadow_Renderer_Close,
};

typedef u32 _FcnGetStateSize( void );
static u32 _None_Renderer_GetStateSize( void );
extern u32 fmesh_Renderer_GetStateSize( void );
extern u32 fdraw_Renderer_GetStateSize( void );
extern u32 fcoll_Renderer_GetStateSize( void );
extern u32 fpsprite_Renderer_GetStateSize( void );
//extern u32 fshadow_Renderer_GetStateSize( void );
static _FcnGetStateSize *_aFcnGetStateSize[FRENDERER_COUNT] = {
	_None_Renderer_GetStateSize,
	fmesh_Renderer_GetStateSize,
	fdraw_Renderer_GetStateSize,
	fcoll_Renderer_GetStateSize,
	fpsprite_Renderer_GetStateSize,
//	fshadow_Renderer_GetStateSize,
};

typedef void _FcnGetState( void *pDestState );
static void _None_Renderer_GetState( void *pDestState );
extern void fmesh_Renderer_GetState( void *pDestState );
extern void fdraw_Renderer_GetState( void *pDestState );
extern void fcoll_Renderer_GetState( void *pDestState );
extern void fpsprite_Renderer_GetState( void *pDestState );
//extern void fshadow_Renderer_GetState( void *pDestState );
static _FcnGetState *_aFcnGetState[FRENDERER_COUNT] = {
	_None_Renderer_GetState,
	fmesh_Renderer_GetState,
	fdraw_Renderer_GetState,
	fcoll_Renderer_GetState,
	fpsprite_Renderer_GetState,
	//fshadow_Renderer_GetState,
};

typedef void _FcnSetState( const void *pState );
static void _None_Renderer_SetState( const void *pState );
extern void fmesh_Renderer_SetState( const void *pState );
extern void fdraw_Renderer_SetState( const void *pState );
extern void fcoll_Renderer_SetState( const void *pState );
extern void fpsprite_Renderer_SetState( const void *pState );
//extern void fshadow_Renderer_SetState( const void *pState );
static _FcnSetState *_aFcnSetState[FRENDERER_COUNT] = {
	_None_Renderer_SetState,
	fmesh_Renderer_SetState,
	fdraw_Renderer_SetState,
	fcoll_Renderer_SetState,
	fpsprite_Renderer_SetState,
	//fshadow_Renderer_SetState,
};

typedef void _FcnSetDefaultState( void );
static void _None_Renderer_SetDefaultState( void );
extern void fmesh_Renderer_SetDefaultState( void );
extern void fdraw_Renderer_SetDefaultState( void );
extern void fcoll_Renderer_SetDefaultState( void );
extern void fpsprite_Renderer_SetDefaultState( void );
//extern void fshadow_Renderer_SetDefaultState( void );
static _FcnSetDefaultState *_aFcnSetDefaultState[FRENDERER_COUNT] = {
	_None_Renderer_SetDefaultState,
	fmesh_Renderer_SetDefaultState,
	fdraw_Renderer_SetDefaultState,
	fcoll_Renderer_SetDefaultState,
	fpsprite_Renderer_SetDefaultState,
	//fshadow_Renderer_SetDefaultState,
};


struct _StateStackEntry_s;
typedef struct _StateStackEntry_s _StateStackEntry_t;

struct _StateStackEntry_s {
	FRenderer_e nRenderer;			// The renderer this state belongs to
	u32 nStateBytes;				// The size in bytes of the state information that immediately follows this structure
	_StateStackEntry_t *pPrevState;	// Pointer to the previous state
};





static BOOL _bModuleInitialized;
static FRenderer_e _nRenderer;

static u32 _nStateStackBytes;
static _StateStackEntry_t *_pStateStack;
static _StateStackEntry_t *_pStateStackBufferLimit;
static _StateStackEntry_t *_pCurrentStateEntry;



static void _SwitchRenderer( FRenderer_e nNewRenderer );
static void _PushState( void );
static void _PopState( void );



BOOL frenderer_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	_nRenderer = FRENDERER_NONE;

	_nStateStackBytes = FMATH_BYTE_ALIGN_UP( Fang_ConfigDefs.nRenderer_StackBytes, sizeof(u32) );
	if( _nStateStackBytes < sizeof(_StateStackEntry_t) ) {
		DEVPRINTF( "frenderer_ModuleStartup(): A minimum of %u bytes are required for the state stack. Specified bytes=%u.\n", Fang_ConfigDefs.nRenderer_StackBytes );
		return FALSE;
	}

	_pStateStack = (_StateStackEntry_t *)fres_Alloc( _nStateStackBytes );
	if( _pStateStack == NULL ) {
		DEVPRINTF( "frenderer_ModuleStartup(): Could not allocate %u bytes for state stack.\n", _nStateStackBytes );
		return FALSE;
	}

	_pStateStackBufferLimit = (_StateStackEntry_t *)( (u32)_pStateStack + _nStateStackBytes );

	_pCurrentStateEntry = _pStateStack;
	_pCurrentStateEntry->nRenderer = FRENDERER_NONE;
	_pCurrentStateEntry->nStateBytes = 0;
	_pCurrentStateEntry->pPrevState = NULL;

	FRenderer_nFillMode = FRENDERER_FILLMODE_NORMAL;

	FRenderer_bDrawBoundInfo = FALSE;
	FRenderer_bDrawBoneInfo = FALSE;

	FRenderer_bFogEnabled = FALSE;
	FRenderer_fFogStartZ = FRENDERER_DEFAULT_FOG_STARTZ;
	FRenderer_fFogEndZ = FRENDERER_DEFAULT_FOG_ENDZ;
	FRenderer_fOOFogEndMinusFogStartZ = 1.0f / (FRenderer_fFogEndZ - FRenderer_fFogStartZ);
	FRenderer_FogMotif.OpaqueWhite();
	FRenderer_FogColorRGB = FRenderer_FogMotif.ColorRGB;
	FRenderer_nFogChangedKey = 1;

	#if FANG_PLATFORM_DX
		FRenderer_FogD3DColor = 0x00ffffff;
		FRenderer_nD3DFillMode = FRenderer_anD3DFillMode[FRenderer_nFillMode];
	#endif

	_bModuleInitialized = TRUE;

	return TRUE;
}

void frenderer_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}

FRenderer_e frenderer_GetActive( void ) {
	FASSERT( _bModuleInitialized );
	return _nRenderer;
}

FRenderer_e frenderer_Push( FRenderer_e nNewRenderer ) {
	FRenderer_e nPrevRenderer;

	FASSERT( _bModuleInitialized );

	nPrevRenderer = _nRenderer;

	// Save state of current renderer...
	_PushState();

	if( nNewRenderer != _nRenderer ) {
		// Switch renderers...
		_SwitchRenderer( nNewRenderer );
	}

	return nPrevRenderer;
}

FRenderer_e frenderer_Push( FRenderer_e nNewRenderer, void *pNewState ) {
	FRenderer_e nPrevRenderer;

	FASSERT( _bModuleInitialized );

	nPrevRenderer = _nRenderer;

	// Save state of current renderer...
	_PushState();

	if( nNewRenderer != _nRenderer ) {
		// Switch renderers...
		_SwitchRenderer( nNewRenderer );
	}

	if( pNewState ) {
		// New state specified...
		frenderer_SetState( pNewState );
	} else {
		// Default state requested...
		frenderer_SetDefaultState();
	}

	return nPrevRenderer;
}

FRenderer_e frenderer_Pop( void ) {
	FASSERT( _bModuleInitialized );

	if( _pCurrentStateEntry->nRenderer != _nRenderer ) {
		// Switch renderers...
		_SwitchRenderer( _pCurrentStateEntry->nRenderer );
	}

	// Restore state of new renderer...
	_PopState();

	return _nRenderer;
}

void frenderer_PopAll( void ) {
	if( _nRenderer != FRENDERER_NONE ) {
		_SwitchRenderer( FRENDERER_NONE );
	}

	_pCurrentStateEntry = _pStateStack;
}

u32 frenderer_GetStateSize( FRenderer_e nRenderer ) {
	FASSERT( _bModuleInitialized );

	if( nRenderer == FRENDERER_NONE ) {
		nRenderer = _nRenderer;
	}

	FASSERT( nRenderer>=0 && nRenderer<FRENDERER_COUNT );
	return FMATH_BYTE_ALIGN_UP( _aFcnGetStateSize[nRenderer](), sizeof(u32) );
}

void frenderer_GetState( void *pDestState ) {
	FASSERT( _bModuleInitialized );
	FASSERT( _nRenderer>=0 && _nRenderer<FRENDERER_COUNT );
	_aFcnGetState[_nRenderer]( pDestState );
}

void frenderer_SetState( const void *pState ) {
	FASSERT( _bModuleInitialized );
	FASSERT( _nRenderer>=0 && _nRenderer<FRENDERER_COUNT );
	_aFcnSetState[_nRenderer]( pState );
}

void frenderer_SetDefaultState( void ) {
	FASSERT( _bModuleInitialized );
	FASSERT( _nRenderer>=0 && _nRenderer<FRENDERER_COUNT );
	_aFcnSetDefaultState[_nRenderer]();
}

FRendererFillMode_e frenderer_FillMode_Set( FRendererFillMode_e nFillMode ) {
	FRendererFillMode_e nPrevFillMode;

	FASSERT( _bModuleInitialized );
	FASSERT( nFillMode>=0 && nFillMode<FRENDERER_FILLMODE_COUNT );

	nPrevFillMode = FRenderer_nFillMode;
	FRenderer_nFillMode = nFillMode;

	#if FANG_PLATFORM_DX
		FRenderer_nD3DFillMode = FRenderer_anD3DFillMode[FRenderer_nFillMode];
	#endif

	return nPrevFillMode;
}

FRendererFillMode_e frenderer_FillMode_Get( void ) {
	FASSERT( _bModuleInitialized );
	return FRenderer_nFillMode;
}

BOOL frenderer_DrawBound_Enable( BOOL bEnable ) {
	BOOL bPrevEnable;

	FASSERT( _bModuleInitialized );

	bPrevEnable = FRenderer_bDrawBoundInfo;
	FRenderer_bDrawBoundInfo = bEnable;

	return bPrevEnable;
}

BOOL frenderer_DrawBound_IsEnabled( void ) {
	FASSERT( _bModuleInitialized );
	return FRenderer_bDrawBoundInfo;
}

BOOL frenderer_DrawBones_Enable( BOOL bEnable ) {
	BOOL bPrevEnable;

	FASSERT( _bModuleInitialized );

	bPrevEnable = FRenderer_bDrawBoneInfo;
	FRenderer_bDrawBoneInfo = bEnable;

	return bPrevEnable;
}

BOOL frenderer_DrawBones_IsEnabled( void ) {
	FASSERT( _bModuleInitialized );
	return FRenderer_bDrawBoneInfo;
}

void frenderer_Fog_Enable( BOOL bEnable ) {
	FASSERT( _bModuleInitialized );
	FRenderer_bFogEnabled = bEnable;
}

void frenderer_Fog_SetRange( f32 fStartZ, f32 fEndZ ) {
	FASSERT( _bModuleInitialized );

	if( fStartZ!=FRenderer_fFogStartZ || fEndZ!=FRenderer_fFogEndZ ) {
		// Values changed...
		FRenderer_nFogChangedKey++;

		FRenderer_fFogStartZ = fStartZ;
		FRenderer_fFogEndZ = fEndZ;

		if( fEndZ != fStartZ ) {
			FRenderer_fOOFogEndMinusFogStartZ = 1.0f / (fEndZ - fStartZ);
		} else {
			FRenderer_fOOFogEndMinusFogStartZ = 1.0f;
		}
	}
}

void frenderer_Fog_SetMotif( const CFColorMotif *pMotif ) {
	FASSERT( _bModuleInitialized );

	if( *pMotif != FRenderer_FogMotif ) {
		// Value changed...
		FRenderer_nFogChangedKey++;
		FRenderer_FogMotif = *pMotif;
	}
}

void frenderer_Fog_SetState( const FRendererFogState_t *pFogState ) {
	FASSERT( _bModuleInitialized );
	frenderer_Fog_Enable( pFogState->bEnable );
	frenderer_Fog_SetRange( pFogState->fStartZ, pFogState->fEndZ );
	frenderer_Fog_SetMotif( &pFogState->Motif );
}

BOOL frenderer_Fog_IsEnabled( void ) {
	FASSERT( _bModuleInitialized );
	return FRenderer_bFogEnabled;
}

void frenderer_Fog_GetRange( f32 *pfStartZ, f32 *pfEndZ ) {
	FASSERT( _bModuleInitialized );

	if( pfStartZ ) {
		*pfStartZ = FRenderer_fFogStartZ;
	}
	if( pfEndZ ) {
		*pfEndZ = FRenderer_fFogEndZ;
	}
}

void frenderer_Fog_GetMotif( CFColorMotif *pMotif ) {
	FASSERT( _bModuleInitialized );

	if( pMotif ) {
		*pMotif = FRenderer_FogMotif;
	}
}

void frenderer_Fog_GetState( FRendererFogState_t *pDestFogState ) {
	FASSERT( _bModuleInitialized );
	pDestFogState->bEnable = frenderer_Fog_IsEnabled();
	frenderer_Fog_GetRange( &pDestFogState->fStartZ, &pDestFogState->fEndZ );
	frenderer_Fog_GetMotif( &pDestFogState->Motif );
}

// Computes FRenderer_FogColorRGB
void frenderer_Fog_ComputeColor( void ) {
	#if FANG_PLATFORM_DX
		CFColorRGB FogColorRGB;

		FRenderer_FogMotif.ComputeColor( &FogColorRGB );

		if( FogColorRGB != FRenderer_FogColorRGB ) {
			u32 nR, nG, nB;

			// We need to compute the D3D fog color...

			FRenderer_FogColorRGB = FogColorRGB;

			nR = fmath_FloatToU32( FogColorRGB.fRed * 255.0f);
			nG = fmath_FloatToU32( FogColorRGB.fGreen * 255.0f);
			nB = fmath_FloatToU32( FogColorRGB.fBlue * 255.0f);

			FASSERT( nR < 256 );
			FASSERT( nG < 256 );
			FASSERT( nB < 256 );

			FRenderer_FogD3DColor = (nR<<16) | (nG<<8) | nB;
		}
	#else
		FRenderer_FogMotif.ComputeColor( &FRenderer_FogColorRGB );
	#endif
}



static void _None_Renderer_Open( void ) {
	// Body intentionally left void...
}

static void _None_Renderer_Close( void ) {
	// Body intentionally left void...
}

static u32 _None_Renderer_GetStateSize( void ) {
	return 0;
}

static void _None_Renderer_GetState( void *pDestState ) {
	// Body intentionally left void...
	pDestState;
}

static void _None_Renderer_SetState( const void *pState ) {
	// Body intentionally left void...
	pState;
}

static void _None_Renderer_SetDefaultState( void ) {
	// Body intentionally left void...
}

static void _SwitchRenderer( FRenderer_e nNewRenderer ) {
	// Close current renderer...
	FASSERT( _nRenderer>=0 && _nRenderer<FRENDERER_COUNT );
	_aFcnClose[_nRenderer]();

	// Switch to new renderer...
	_nRenderer = nNewRenderer;

	// Open new renderer...
	FASSERT( _nRenderer>=0 && _nRenderer<FRENDERER_COUNT );
	_aFcnOpen[_nRenderer]();
}

static void _PushState( void ) {
	u32 nStateBytes, nTotalStateBytes;
	_StateStackEntry_t *pNextStateEntry;

	nStateBytes = frenderer_GetStateSize( _nRenderer );
	nTotalStateBytes = sizeof(_StateStackEntry_t) + nStateBytes;
	pNextStateEntry = (_StateStackEntry_t *)( (u32)_pCurrentStateEntry + sizeof(_StateStackEntry_t) + _pCurrentStateEntry->nStateBytes );

	if( ((u32)pNextStateEntry + nTotalStateBytes) > (u32)_pStateStackBufferLimit ) {
		DEVPRINTF( "frenderer::_PushState(): Out of room in state stack buffer. Try increasing Fang_ConfigDefs.nRenderer_StackBytes.\n" );
		return;
	}

	pNextStateEntry->nRenderer = _nRenderer;
	pNextStateEntry->nStateBytes = nStateBytes;
	pNextStateEntry->pPrevState = _pCurrentStateEntry;

	// make sure that we align our state info pointer so that we can use the new math classes without crashing
	void *pAlignedPtr = pNextStateEntry + 1;
	pAlignedPtr = (void *)FMATH_BYTE_ALIGN_UP( (u32)pAlignedPtr, FCLASS_BYTE_ALIGN );
	frenderer_GetState( pAlignedPtr );

	_pCurrentStateEntry = pNextStateEntry;
}

static void _PopState( void ) {
	FASSERT( _pCurrentStateEntry->nRenderer == _nRenderer );

	if( _pCurrentStateEntry->pPrevState ) {

		// make sure that we align our state info pointer so that we can use the new math classes without crashing
		void *pAlignedPtr = _pCurrentStateEntry + 1;
		pAlignedPtr = (void *)FMATH_BYTE_ALIGN_UP( (u32)pAlignedPtr, FCLASS_BYTE_ALIGN );
		frenderer_GetState( pAlignedPtr );

		_pCurrentStateEntry = _pCurrentStateEntry->pPrevState;
	}
}

