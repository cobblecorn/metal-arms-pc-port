//////////////////////////////////////////////////////////////////////////////////////
// smoketrail.cpp - Smoke trail system.
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
// 08/20/01 Ranck       Created.
// 06/18/02	Ranck		Fang2-ized.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "smoketrail.h"
#include "fpsprite.h"
#include "fworld.h"
#include "floop.h"
#include "frenderer.h"


#define _MAX_PUFFS_PER_FRAME	20


typedef struct {
	f32 fScaleSpeed;
	f32 fScaleAccel;

	f32 fYSpeed;
	f32 fYAccel;

	f32 fOpaqueSpeed;
	f32 fOpaqueAccel;

	CFColorPackedRGB PackedStartColor;
	CFColorPackedRGB PackedEndColor;

	u8 nPackedColorUnitSlider;
	f32 fColorUnitSliderSpeed;
	f32 fColorUnitSliderAccel;
} _PuffState_t;


static BOOL _bSystemInitialized = FALSE;
static FLinkRoot_t _FreeRoot;
static FLinkRoot_t _UsedRoot;

static SmokeTrailAttrib_t _DefaultAttrib;

class _CSmokeTrail : public CFWorldPSGroup {
public:
	FINLINE _CSmokeTrail() {}
	FINLINE ~_CSmokeTrail() {}

	_PuffState_t *m_pStateBuf;		// Pointer to puff state buffer (one entry per puff)

	BOOL8 m_bReturnWhenNoPuffs;		// TRUE=return this smoke trail to the free pool when there are no more puffs
	BOOL8 m_bReset;					// TRUE=next added puff will be the first
	BOOL8 m_bPostDrawThisFrame;		// TRUE=post draw this smoke trail this frame
	BOOL8 m_bNoPuffsPerFrameCap;	// TRUE=don't cap the puffs per frame
	SmokeTrailAttrib_t m_Attrib;	// Attributes for this smoke trail
	CFVec3A m_PrevPuffPos_WS;		// Previous puff position

	FLink_t m_Link;					// Link to other smoke trails
};



static void _ResDestroyedCallback( void *pResMem );
static _CSmokeTrail *_CreateSmokeTrail( u32 nMaxPuffCount );
static void _BuildRandomState( _PuffState_t *pDestPuffState, const SmokeTrailAttrib_t *pAttrib );
static void _UpdateTrackerSphere( _CSmokeTrail *pSmokeTrail );
static void _WorkPuff( FPSprite_t *pPS, _PuffState_t *pState );
static void _Work( _CSmokeTrail *pSmokeTrail );
static BOOL _PSPreDrawCallback( CFWorldPSGroup *pPSGroup, f32 fDistToCameraSq );



BOOL smoketrail_InitSystem( void ) {
	FASSERT( !_bSystemInitialized );

	flinklist_InitRoot( &_FreeRoot, (s32)FANG_OFFSETOF( _CSmokeTrail, m_Link ) );
	flinklist_InitRoot( &_UsedRoot, (s32)FANG_OFFSETOF( _CSmokeTrail, m_Link ) );

	_DefaultAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	_DefaultAttrib.pTexDef = NULL;

	_DefaultAttrib.fScaleMin_WS = 1.0f;
	_DefaultAttrib.fScaleMax_WS = 1.2f;
	_DefaultAttrib.fScaleSpeedMin_WS = 0.8f;
	_DefaultAttrib.fScaleSpeedMax_WS = 1.2f;
	_DefaultAttrib.fScaleAccelMin_WS = 0.0f;
	_DefaultAttrib.fScaleAccelMax_WS = 0.0f;

	_DefaultAttrib.fXRandSpread_WS = 0.2f;
	_DefaultAttrib.fYRandSpread_WS = 0.2f;
	_DefaultAttrib.fDistBetweenPuffs_WS = 0.4f;
	_DefaultAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	_DefaultAttrib.fYSpeedMin_WS = 3.0f;
	_DefaultAttrib.fYSpeedMax_WS = 3.5f;
	_DefaultAttrib.fYAccelMin_WS = 0.0f;
	_DefaultAttrib.fYAccelMax_WS = 0.0f;

	_DefaultAttrib.fUnitOpaqueMin_WS = 0.5f;
	_DefaultAttrib.fUnitOpaqueMax_WS = 0.7f;
	_DefaultAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	_DefaultAttrib.fUnitOpaqueSpeedMax_WS = -0.4f;
	_DefaultAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	_DefaultAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	_DefaultAttrib.StartColorRGB.White();
	_DefaultAttrib.EndColorRGB.Set( 0.5f );
	_DefaultAttrib.fStartColorUnitIntensityMin = 0.8f;
	_DefaultAttrib.fStartColorUnitIntensityMax = 1.0f;
	_DefaultAttrib.fEndColorUnitIntensityMin = 0.8f;
	_DefaultAttrib.fEndColorUnitIntensityMax = 1.0f;

	_DefaultAttrib.fColorUnitSliderSpeedMin = 0.0f;
	_DefaultAttrib.fColorUnitSliderSpeedMax = 0.0f;
	_DefaultAttrib.fColorUnitSliderAccelMin = 0.0f;
	_DefaultAttrib.fColorUnitSliderAccelMax = 0.0f;

	_bSystemInitialized = TRUE;

	return TRUE;
}


void smoketrail_UninitSystem( void ) {
	if( _bSystemInitialized ) {
		_bSystemInitialized = FALSE;
	}
}


BOOL smoketrail_IsSystemInitialized( void ) {
	return _bSystemInitialized;
}


BOOL smoketrail_Allocate( u32 nSmokeTrailCount, u32 nMaxPuffCount ) {
	_CSmokeTrail *pSmokeTrail = NULL;
	u32 i;

	FASSERT( _bSystemInitialized );

	// Start a memory frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Create an fres resource...
	FResHandle_t hRes = fres_CreateWithCallback( NULL, NULL, _ResDestroyedCallback );
	if( hRes == FRES_NULLHANDLE ) {
		DEVPRINTF( "smoketrail_Allocate(): Could not create smoke trail resource object.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<nSmokeTrailCount; ++i ) {
		pSmokeTrail = _CreateSmokeTrail( nMaxPuffCount );
		if( pSmokeTrail == NULL ) {
			goto _ExitWithError;
		}

		flinklist_AddTail( &_FreeRoot, pSmokeTrail );
	}

	// SmokeTrail created successfully...

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


static void _ResDestroyedCallback( void *pResMem ) {
	_CSmokeTrail *pSmokeTrail;

	while( pSmokeTrail = (_CSmokeTrail *)flinklist_GetTail( &_UsedRoot ) ) {
		flinklist_Remove( &_UsedRoot, pSmokeTrail );
		fdelete( pSmokeTrail );
	}

	while( pSmokeTrail = (_CSmokeTrail *)flinklist_GetTail( &_FreeRoot ) ) {
		flinklist_Remove( &_FreeRoot, pSmokeTrail );
		fdelete( pSmokeTrail );
	}
}


static _CSmokeTrail *_CreateSmokeTrail( u32 nMaxPuffCount ) {
	_CSmokeTrail *pSmokeTrail = NULL;

	FResFrame_t ResFrame = fres_GetFrame();

	// Create a new smoke trail object...
	pSmokeTrail = fnew _CSmokeTrail;
	if( pSmokeTrail == NULL ) {
		DEVPRINTF( "smoketrail_Allocate(): Could not allocate memory for smoke trail object.\n" );
		goto _ExitWithError;
	}

	// Create the puff buffer...
	pSmokeTrail->m_pBase = (FPSprite_t *)fres_AlignedAlloc( nMaxPuffCount * sizeof(FPSprite_t), FPSPRITE_BYTE_ALIGNMENT );
	if( pSmokeTrail->m_pBase == NULL ) {
		DEVPRINTF( "smoketrail_Allocate(): Could not allocate memory for vertex buffer.\n" );
		goto _ExitWithError;
	}

	// Create the puff state buffer...
	pSmokeTrail->m_pStateBuf = (_PuffState_t *)fres_Alloc( nMaxPuffCount * sizeof(_PuffState_t) );
	if( pSmokeTrail->m_pStateBuf == NULL ) {
		DEVPRINTF( "smoketrail_Allocate(): Could not allocate memory for puff state buffer.\n" );
		goto _ExitWithError;
	}

	// Init smoke trail object...
	pSmokeTrail->m_nMaxCount = nMaxPuffCount;
	pSmokeTrail->m_nRenderCount = 0;
	pSmokeTrail->m_nRenderStartIndex = 0;
	pSmokeTrail->m_bReset = TRUE;
	pSmokeTrail->m_bReturnWhenNoPuffs = FALSE;
	pSmokeTrail->m_bPostDrawThisFrame = FALSE;
	pSmokeTrail->m_bNoPuffsPerFrameCap = FALSE;

	pSmokeTrail->m_Xfm.Identity();
	pSmokeTrail->m_BoundSphere_MS.Zero();
	pSmokeTrail->m_fCullDist = FMATH_MAX_FLOAT;
	pSmokeTrail->m_nPSFlags = FPSPRITE_FLAG_NONE;
	pSmokeTrail->m_nBlend = FPSPRITE_BLEND_MODULATE;

//	pSmokeTrail->m_pTexInst->SetMipmapBias( 0.0f );
//	pSmokeTrail->m_pTexInst->SetFlags( 0 );
//	pSmokeTrail->m_pTexInst->SetTexDef( NULL );

	fang_MemCopy( &pSmokeTrail->m_Attrib, &_DefaultAttrib, sizeof(SmokeTrailAttrib_t) );

	// Success...

	return pSmokeTrail;

	// Error:
_ExitWithError:
	if( pSmokeTrail ) {
		fdelete( pSmokeTrail );
	}

	fres_ReleaseFrame( ResFrame );

	return NULL;
}


SmokeTrailHandle_t smoketrail_GetFromFreePool( BOOL bPostDraw ) {
	_CSmokeTrail *pSmokeTrail;

	FASSERT( _bSystemInitialized );

	pSmokeTrail = (_CSmokeTrail *)flinklist_RemoveTail( &_FreeRoot );

	if( pSmokeTrail ) {
		flinklist_AddTail( &_UsedRoot, pSmokeTrail );
		pSmokeTrail->m_bReset = TRUE;
		pSmokeTrail->m_bReturnWhenNoPuffs = FALSE;
		pSmokeTrail->m_bPostDrawThisFrame = FALSE;
		pSmokeTrail->m_bNoPuffsPerFrameCap = FALSE;
		pSmokeTrail->SetPreDrawCallback( NULL );

		if( bPostDraw ) {
			pSmokeTrail->SetPreDrawCallback( _PSPreDrawCallback );
		}
	}

	return (SmokeTrailHandle_t)pSmokeTrail;
}


SmokeTrailHandle_t smoketrail_GetFromFreePoolAndSetAttributes( SmokeTrailAttrib_t *pAttrib, BOOL bPostDraw ) {
	_CSmokeTrail *pSmokeTrail;

	FASSERT( _bSystemInitialized );

	pSmokeTrail = (_CSmokeTrail *)smoketrail_GetFromFreePool();

	if( pSmokeTrail ) {
		smoketrail_SetAttributes( (SmokeTrailHandle_t)pSmokeTrail, pAttrib );
		pSmokeTrail->m_bReset = TRUE;
		pSmokeTrail->m_bReturnWhenNoPuffs = FALSE;
		pSmokeTrail->m_bPostDrawThisFrame = FALSE;
		pSmokeTrail->m_bNoPuffsPerFrameCap = FALSE;
		pSmokeTrail->SetPreDrawCallback( NULL );

		if( bPostDraw ) {
			pSmokeTrail->SetPreDrawCallback( _PSPreDrawCallback );
		}
	}

	return (SmokeTrailHandle_t)pSmokeTrail;
}


void smoketrail_ReturnToFreePool( SmokeTrailHandle_t hSmokeTrail, BOOL bWaitForNoMorePuffs ) {
	_CSmokeTrail *pSmokeTrail;

	FASSERT( _bSystemInitialized );

	pSmokeTrail = (_CSmokeTrail *)hSmokeTrail;

#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	_CSmokeTrail *pSearchSmokeTrail;

	// Make sure smoke trail isn't in the free list...
	for( pSearchSmokeTrail=(_CSmokeTrail *)flinklist_GetHead(&_FreeRoot); pSearchSmokeTrail; pSearchSmokeTrail=(_CSmokeTrail *)flinklist_GetNext(&_FreeRoot, pSearchSmokeTrail) ) {
		FASSERT_MSG( pSearchSmokeTrail != pSmokeTrail, "smoketrail_ReturnToFreePool(): Attempt to return a smoketrail that has already been returned." );
	}

	// Make sure smoke trail is in the used list...
	for( pSearchSmokeTrail=(_CSmokeTrail *)flinklist_GetHead(&_UsedRoot); pSearchSmokeTrail; pSearchSmokeTrail=(_CSmokeTrail *)flinklist_GetNext(&_UsedRoot, pSearchSmokeTrail) ) {
		if( pSearchSmokeTrail == pSmokeTrail ) {
			// Found it!
			break;
		}
	}
	if( pSearchSmokeTrail == NULL ) {
		FASSERT_NOW_MSG( "smoketrail_ReturnToFreePool(): Attempt to return a smoketrail that has already been returned." );
	}
#endif

	if( pSmokeTrail ) {
		if( !bWaitForNoMorePuffs || (pSmokeTrail->m_nRenderCount==0) ) {
			flinklist_Remove( &_UsedRoot, pSmokeTrail );
			flinklist_AddTail( &_FreeRoot, pSmokeTrail );
			pSmokeTrail->RemoveFromWorld();
		} else {
			pSmokeTrail->m_bReturnWhenNoPuffs = TRUE;
		}
	}
}


void smoketrail_ReturnAllToFreePoolNow( void ) {
	_CSmokeTrail *pSmokeTrail;

	FASSERT( _bSystemInitialized );

	while( pSmokeTrail=(_CSmokeTrail *)flinklist_RemoveTail( &_UsedRoot ) ) {
		pSmokeTrail->RemoveFromWorld();
		flinklist_AddTail( &_FreeRoot, pSmokeTrail );
	}
}


void smoketrail_SetAttributes( SmokeTrailHandle_t hSmokeTrail, SmokeTrailAttrib_t *pAttrib ) {
	_CSmokeTrail *pSmokeTrail;

	FASSERT( _bSystemInitialized );

	if( hSmokeTrail == SMOKETRAIL_NULLHANDLE ) {
		return;
	}

	pSmokeTrail = (_CSmokeTrail *)hSmokeTrail;

	fang_MemCopy( &pSmokeTrail->m_Attrib, pAttrib, sizeof(SmokeTrailAttrib_t) );

	pSmokeTrail->m_TexInst.SetTexDef( pAttrib->pTexDef );
	pSmokeTrail->m_nPSFlags &= ~(FPSPRITE_FLAG_NO_FOG | FPSPRITE_FLAG_ENABLE_DEPTH_WRITES);

	if( pAttrib->nFlags & SMOKETRAIL_FLAG_NO_FOG ) {
		pSmokeTrail->m_nPSFlags |= FPSPRITE_FLAG_NO_FOG;
	}

	if( pAttrib->nFlags & SMOKETRAIL_FLAG_ENABLE_DEPTH_WRITES ) {
		pSmokeTrail->m_nPSFlags |= FPSPRITE_FLAG_ENABLE_DEPTH_WRITES;
	}

	if( !(pAttrib->nFlags & SMOKETRAIL_FLAG_ADDITIVE_BLEND_MODE) ) {
		pSmokeTrail->m_nBlend = FPSPRITE_BLEND_MODULATE;
	} else {
		pSmokeTrail->m_nBlend = FPSPRITE_BLEND_ADD;
	}
}


void smoketrail_Puff( SmokeTrailHandle_t hSmokeTrail, const CFVec3A *pPos_WS ) {
	FASSERT( _bSystemInitialized );

	if( hSmokeTrail == SMOKETRAIL_NULLHANDLE ) {
		return;
	}

	_CSmokeTrail *pSmokeTrail;
	FPSprite_t *pNewPS;
	_PuffState_t *pNewState;
	f32 fTravelDist_WS, fInvTravelDist_WS, fTravelDist2_WS, fDist_WS;
	u32 nPuffCount;
	CFVec3A PrevToNew_WS, PrevToNewUnitDir_WS, TempVec1, TempVec2;
	CFMtx43A Mtx43;

	pSmokeTrail = (_CSmokeTrail *)hSmokeTrail;

	if( pSmokeTrail->m_bReset ) {
		pSmokeTrail->DecRenderStartIndex();
		pSmokeTrail->IncRenderCount();

		pNewPS = pSmokeTrail->m_pBase + pSmokeTrail->m_nRenderStartIndex;
		pNewState = pSmokeTrail->m_pStateBuf + pSmokeTrail->m_nRenderStartIndex;

		_BuildRandomState( pNewState, &pSmokeTrail->m_Attrib );

		pNewPS->ColorRGBA.ColorRGB.Set( pNewState->PackedStartColor );
		pNewPS->ColorRGBA.fAlpha = fmath_RandomFloatRange( pSmokeTrail->m_Attrib.fUnitOpaqueMin_WS, pSmokeTrail->m_Attrib.fUnitOpaqueMax_WS );
		pNewPS->fDim_MS = fmath_RandomFloatRange( pSmokeTrail->m_Attrib.fScaleMin_WS, pSmokeTrail->m_Attrib.fScaleMax_WS );
		pNewPS->Point_MS = pPos_WS->v3;

		pSmokeTrail->m_PrevPuffPos_WS = *pPos_WS;
		pSmokeTrail->m_bReset = FALSE;

		pSmokeTrail->m_BoundSphere_MS.Set( pNewPS->Point_MS, (0.5f * 1.42f) * pNewPS->fDim_MS );
		pSmokeTrail->UpdateTracker();

		return;
	}

	PrevToNew_WS.Sub( *pPos_WS, pSmokeTrail->m_PrevPuffPos_WS );

	fTravelDist2_WS = PrevToNew_WS.MagSq();
	if( fTravelDist2_WS < (pSmokeTrail->m_Attrib.fDistBetweenPuffs_WS * pSmokeTrail->m_Attrib.fDistBetweenPuffs_WS) ) {
		return;
	}

	fInvTravelDist_WS = fmath_InvSqrt( fTravelDist2_WS );
	PrevToNewUnitDir_WS.Mul( PrevToNew_WS, fInvTravelDist_WS );
	Mtx43.UnitMtxFromUnitVec( &PrevToNewUnitDir_WS );

	fTravelDist_WS = fmath_Inv( fInvTravelDist_WS );

	nPuffCount = 0;
	pNewPS = NULL;
	fDist_WS = pSmokeTrail->m_Attrib.fDistBetweenPuffs_WS;
	while( (fTravelDist_WS - fDist_WS) > pSmokeTrail->m_Attrib.fDistBetweenPuffs_WS ) {
		pSmokeTrail->DecRenderStartIndex();
		pSmokeTrail->IncRenderCount();

		pNewPS = pSmokeTrail->m_pBase + pSmokeTrail->m_nRenderStartIndex;
		pNewState = pSmokeTrail->m_pStateBuf + pSmokeTrail->m_nRenderStartIndex;

		_BuildRandomState( pNewState, &pSmokeTrail->m_Attrib );

		pNewPS->ColorRGBA.ColorRGB.Set( pNewState->PackedStartColor );
		pNewPS->ColorRGBA.fAlpha = fmath_RandomFloatRange( pSmokeTrail->m_Attrib.fUnitOpaqueMin_WS, pSmokeTrail->m_Attrib.fUnitOpaqueMax_WS );
		pNewPS->fDim_MS = fmath_RandomFloatRange( pSmokeTrail->m_Attrib.fScaleMin_WS, pSmokeTrail->m_Attrib.fScaleMax_WS );

		TempVec1.Mul( Mtx43.m_vFront, fDist_WS ).Add( pSmokeTrail->m_PrevPuffPos_WS );

		TempVec2.Mul( Mtx43.m_vRight, pSmokeTrail->m_Attrib.fXRandSpread_WS * fmath_RandomBipolarUnitFloat() );
		TempVec1.Add( TempVec2 );

		TempVec2.Mul( Mtx43.m_vFront, fDist_WS + pSmokeTrail->m_Attrib.fDistBetweenPuffsRandSpread_WS * fmath_RandomBipolarUnitFloat() );
		TempVec1.Add( TempVec2 );

		TempVec2.Mul( Mtx43.m_vUp, pSmokeTrail->m_Attrib.fYRandSpread_WS * fmath_RandomBipolarUnitFloat() );
		TempVec1.Add( TempVec2 );

		pNewPS->Point_MS = TempVec1.v3;

		fDist_WS += pSmokeTrail->m_Attrib.fDistBetweenPuffs_WS;

		++nPuffCount;

		if( !pSmokeTrail->m_bNoPuffsPerFrameCap && (nPuffCount >= _MAX_PUFFS_PER_FRAME) ) {
			break;
		}
	}

	if( pNewPS ) {
		pSmokeTrail->m_PrevPuffPos_WS = TempVec1;
	}

	_UpdateTrackerSphere( pSmokeTrail );
}


static void _BuildRandomState( _PuffState_t *pDestPuffState, const SmokeTrailAttrib_t *pAttrib ) {
	pDestPuffState->fScaleSpeed = fmath_RandomFloatRange( pAttrib->fScaleSpeedMin_WS, pAttrib->fScaleSpeedMax_WS );
	pDestPuffState->fScaleAccel = fmath_RandomFloatRange( pAttrib->fScaleAccelMin_WS, pAttrib->fScaleAccelMax_WS );
	pDestPuffState->fYSpeed = fmath_RandomFloatRange( pAttrib->fYSpeedMin_WS, pAttrib->fYSpeedMax_WS );
	pDestPuffState->fYAccel = fmath_RandomFloatRange( pAttrib->fYAccelMin_WS, pAttrib->fYAccelMax_WS );
	pDestPuffState->fOpaqueSpeed = fmath_RandomFloatRange( pAttrib->fUnitOpaqueSpeedMin_WS, pAttrib->fUnitOpaqueSpeedMax_WS );
	pDestPuffState->fOpaqueAccel = fmath_RandomFloatRange( pAttrib->fUnitOpaqueAccelMin_WS, pAttrib->fUnitOpaqueAccelMax_WS );
	pDestPuffState->PackedStartColor.Set( pAttrib->StartColorRGB * fmath_RandomFloatRange( pAttrib->fStartColorUnitIntensityMin, pAttrib->fStartColorUnitIntensityMax ) );
	pDestPuffState->PackedEndColor.Set( pAttrib->EndColorRGB * fmath_RandomFloatRange( pAttrib->fEndColorUnitIntensityMin, pAttrib->fEndColorUnitIntensityMax ) );
	pDestPuffState->nPackedColorUnitSlider = 0;
	pDestPuffState->fColorUnitSliderSpeed = fmath_RandomFloatRange( pAttrib->fColorUnitSliderSpeedMin, pAttrib->fColorUnitSliderSpeedMax );
	pDestPuffState->fColorUnitSliderAccel = fmath_RandomFloatRange( pAttrib->fColorUnitSliderAccelMin, pAttrib->fColorUnitSliderAccelMax );
}


static void _UpdateTrackerSphere( _CSmokeTrail *pSmokeTrail ) {
	const CFVec3 *pFirstPuffPos_WS, *pLastPuffPos_WS;

	if( pSmokeTrail->m_nRenderCount == 0 ) {
		return;
	}

	if( pSmokeTrail->m_nRenderCount == 1 ) {
		pSmokeTrail->m_BoundSphere_MS.Set( pSmokeTrail->GetFirstPS()->Point_MS, (0.5f * 1.42f) * pSmokeTrail->GetFirstPS()->fDim_MS );
		pSmokeTrail->UpdateTracker();
		return;
	}

	pFirstPuffPos_WS = &pSmokeTrail->GetFirstPS()->Point_MS;
	pLastPuffPos_WS = &pSmokeTrail->GetLastPS()->Point_MS;

	pSmokeTrail->m_BoundSphere_MS.m_Pos = (*pLastPuffPos_WS + *pFirstPuffPos_WS) * 0.5f;
	pSmokeTrail->m_BoundSphere_MS.m_fRadius = (*pLastPuffPos_WS - *pFirstPuffPos_WS).Mag() * 0.5f;

	pSmokeTrail->UpdateTracker();
}


static void _WorkPuff( FPSprite_t *pPS, _PuffState_t *pState, const SmokeTrailAttrib_t *pAttrib ) {
	CFColorRGB StartColorRGB, EndColorRGB;

	// Update color...
	if( pState->fColorUnitSliderSpeed > 0.0f ) {
		pState->fColorUnitSliderSpeed += pState->fColorUnitSliderAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( pState->fColorUnitSliderSpeed, 0.0f );
	} else if( pState->fColorUnitSliderSpeed < 0.0f ) {
		pState->fColorUnitSliderSpeed += pState->fColorUnitSliderAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( pState->fColorUnitSliderSpeed, 0.0f );
	}

	f32 fColorUnitSlider = (f32)pState->nPackedColorUnitSlider * (1.0f / 255.0f);
	fColorUnitSlider += pState->fColorUnitSliderSpeed * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP( fColorUnitSlider, 0.0f, 1.0f );
	pState->nPackedColorUnitSlider = fmath_FloatToU32( 255.0f*fColorUnitSlider );

	StartColorRGB.Set( pState->PackedStartColor );
	EndColorRGB.Set( pState->PackedEndColor );
	pPS->ColorRGBA.ColorRGB.v3.ReceiveLerpOf( fColorUnitSlider, StartColorRGB.v3, EndColorRGB.v3 );

	// Update opaqueness...
	if( pState->fOpaqueSpeed > 0.0f ) {
		pState->fOpaqueSpeed += pState->fOpaqueAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( pState->fOpaqueSpeed, 0.0f );
	} else if( pState->fOpaqueSpeed < 0.0f ) {
		pState->fOpaqueSpeed += pState->fOpaqueAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( pState->fOpaqueSpeed, 0.0f );
	}
	pPS->ColorRGBA.fAlpha += pState->fOpaqueSpeed * FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( pPS->ColorRGBA.fAlpha, 0.0f );

	// Update scale...
	if( pState->fScaleSpeed > 0.0f ) {
		pState->fScaleSpeed += pState->fScaleAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( pState->fScaleSpeed, 0.0f );
	} else if( pState->fScaleSpeed < 0.0f ) {
		pState->fScaleSpeed += pState->fScaleAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( pState->fScaleSpeed, 0.0f );
	}
	pPS->fDim_MS += pState->fScaleSpeed * FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( pPS->fDim_MS, 0.0f );

	// Update position...
	if( pState->fYSpeed > 0.0f ) {
		pState->fYSpeed += pState->fYAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( pState->fYSpeed, 0.0f );
	} else if( pState->fYSpeed < 0.0f ) {
		pState->fYSpeed += pState->fYAccel * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( pState->fYSpeed, 0.0f );
	}
	pPS->Point_MS.y += pState->fYSpeed * FLoop_fPreviousLoopSecs;
}


void smoketrail_WorkAll( void ) {
	FASSERT( _bSystemInitialized );

	if( _UsedRoot.nCount == 0 ) {
		return;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();

	_CSmokeTrail **ppSmokeTrailsToWork = (_CSmokeTrail **)fmem_Alloc( sizeof(_CSmokeTrail *) * _UsedRoot.nCount );
	if( ppSmokeTrailsToWork == NULL ) {
		DEVPRINTF( "smoketrail_WorkAll(): Not enough fmem memory to allocate work buffer.\n" );
		goto _ExitWorkAll;
	}

	_CSmokeTrail *pSmokeTrail;
	u32 i;

	i = 0;

	// Add all smoketrails in the active list...
	for( pSmokeTrail = (_CSmokeTrail *)flinklist_GetHead( &_UsedRoot ); pSmokeTrail; pSmokeTrail = (_CSmokeTrail *)flinklist_GetNext( &_UsedRoot, pSmokeTrail ) ) {
		FASSERT( i < _UsedRoot.nCount );

		ppSmokeTrailsToWork[i] = pSmokeTrail;

		++i;
	}

	for( i=0; i<_UsedRoot.nCount; i++ ) {
		_Work( ppSmokeTrailsToWork[i] );
	}

_ExitWorkAll:
	fmem_ReleaseFrame( MemFrame );
}



static void _Work( _CSmokeTrail *pSmokeTrail ) {
	u32 i, nStartPuffCount, nEndPuffCount;
	_PuffState_t *pState;
	FPSprite_t *pPS;

	if( pSmokeTrail->m_nRenderCount == 0 ) {
		if( pSmokeTrail->m_bReturnWhenNoPuffs ) {
			pSmokeTrail->RemoveFromWorld();
			smoketrail_ReturnToFreePool( (SmokeTrailHandle_t)pSmokeTrail, FALSE );
		}

		return;
	}

	pSmokeTrail->GetRenderCounts( &nStartPuffCount, &nEndPuffCount );

	pState = pSmokeTrail->m_pStateBuf + pSmokeTrail->GetFirstPSIndex();
	pPS = pSmokeTrail->GetFirstPS();
	for( i=0; i<nStartPuffCount; i++, pState++, pPS++ ) {
		_WorkPuff( pPS, pState, &pSmokeTrail->m_Attrib );
	}

	pState = pSmokeTrail->m_pStateBuf;
	pPS = pSmokeTrail->m_pBase;
	for( i=0; i<nEndPuffCount; i++, pState++, pPS++ ) {
		_WorkPuff( pPS, pState, &pSmokeTrail->m_Attrib );
	}

	pPS = pSmokeTrail->GetLastPS();
	while( TRUE ) {
		if( pPS->ColorRGBA.fAlpha > 0.0f ) {
			break;
		}

		if( --pSmokeTrail->m_nRenderCount == 0 ) {
			break;
		}

		if( pPS == pSmokeTrail->m_pBase ) {
			pPS = pSmokeTrail->m_pBase + pSmokeTrail->m_nMaxCount;
		}

		pPS--;
	}

	_UpdateTrackerSphere( pSmokeTrail );
}


// Returns TRUE if the muzzle puff was actually made.
BOOL smoketrail_SpawnMuzzlePuff( SmokeTrailAttrib_t *pSmokeTrailAttrib, const CFVec3A *pStartPos_WS, const CFVec3A *pUnitDir_WS, f32 fLength ) {
	SmokeTrailHandle_t hSmokeTrail;
	CFVec3A SmokeEnd;

	hSmokeTrail = smoketrail_GetFromFreePoolAndSetAttributes( pSmokeTrailAttrib );
	if( hSmokeTrail == SMOKETRAIL_NULLHANDLE ) {
		return FALSE;
	}

	((_CSmokeTrail *)hSmokeTrail)->m_bNoPuffsPerFrameCap = TRUE;

	smoketrail_Puff( hSmokeTrail, pStartPos_WS );

	SmokeEnd.Mul( *pUnitDir_WS, fLength ).Add( *pStartPos_WS );
	smoketrail_Puff( hSmokeTrail, &SmokeEnd );
	smoketrail_ReturnToFreePool( hSmokeTrail, TRUE );

	return TRUE;
}


void smoketrail_PostDrawAll( void ) {
	_CSmokeTrail *pSmokeTrail;
	BOOL bStateSet = FALSE;

	FASSERT( _bSystemInitialized );

	for( pSmokeTrail=(_CSmokeTrail *)flinklist_GetHead(&_UsedRoot); pSmokeTrail; pSmokeTrail=(_CSmokeTrail *)flinklist_GetNext(&_UsedRoot, pSmokeTrail) ) {
		if( pSmokeTrail->m_bPostDrawThisFrame ) {
			if( !bStateSet ) {
				bStateSet = TRUE;
				frenderer_Push( FRENDERER_PSPRITE, NULL );
			}

			pSmokeTrail->Render( FALSE );
			pSmokeTrail->m_bPostDrawThisFrame = FALSE;
		}
	}

	if( bStateSet ) {
		frenderer_Pop();
	}
}


static BOOL _PSPreDrawCallback( CFWorldPSGroup *pPSGroup, f32 fDistToCameraSq ) {
	_CSmokeTrail *pSmokeTrail = (_CSmokeTrail *)pPSGroup;

	// Flag for rendering later...
	pSmokeTrail->m_bPostDrawThisFrame = TRUE;

	// Don't render it now...
	return FALSE;
}


