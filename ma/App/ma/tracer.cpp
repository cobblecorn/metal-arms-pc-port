//////////////////////////////////////////////////////////////////////////////////////
// tracer.cpp - Tracer object module.
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
// 08/14/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "tracer.h"
#include "fdraw.h"
#include "ftex.h"
#include "fcolor.h"
#include "fmath.h"
#include "floop.h"
#include "flinklist.h"
#include "fres.h"
#include "fvid.h"
#include "fworld_coll.h"
#include "frenderer.h"
#include "meshtypes.h"


#define _VERTS_PER_TRACER	12


typedef struct _Tracer_s _Tracer_t;


typedef struct 
{
	FLink_t Link;					// Linklist to other groups

	CFTexInst *pTexInst;			// Texture to use for this group
	u16 nMaxTracerCount;			// Maximum number of tracers in this group
	u16 nActiveCount;				// Number of active tracers in this group
	u16 nVertexCount;				// Number of active vertices in this group
	_Tracer_t *pTracerBuf;			// Pointer to tracer buffer for this group
	FDrawVtx_t *pVtxBuf;			// Pointer to vertex buffer for this group
} _Group_t;


FCLASS_ALIGN_PREFIX struct _Tracer_s 
{
	TracerDef_t TracerDef;			// Tracer definition

	BOOL bUseFadeTail;
	f32 fTotalDistMoved_WS;			// Total tail distance moved in world space
	f32 fOOMaxDist_WS;				// 1.0f / TracerDef.fMaxTailDist_WS
	f32 fOOLength_WS;				// 1.0f / TracerDef.fLength_WS
	f32 fDeathFadeoutK;				// 1.0f / (1.0f - TracerDef.fBeginDeathUnitFade_WS)
	f32 fTimeOfBirth;				//
	CFVec3A vHeadPos_WS;			// Tracer's head position in world space
	CFVec3A vFadePos_WS;			// Tracer's fade position in world space
	CFVec3A vCollRayStart_WS;		// Ray start point used for collision

	u16 nTracerIndex;				// Index into pGroup->pTracerBuf for this tracer
	u8	uFlags;
	BOOL8 bAbsorbMode;				// FALSE=normal, TRUE=clip head position to plane and kill when tail position passes plane
	u32 nAddedFrame;				// Snapshot of FVid_nFrameCounter of when this tracer was added
	f32 fAlphaFadeStartTime;		// how long until overall alpha will start to fade out
	f32 fAlphaFadeTime;				// how long overall alpha will take to fade out
	CFVec3A AbsorbPoint_WS;			// Used when bAbsorbMode is TRUE to clip tracer against a plane
	GlobalTracerMoveCB* pMoveCB;	// A CB Func to be called when the tracer moves
	void *pMoveCBData;				// some data to be passed to the move CB func


	FCLASS_STACKMEM_ALIGN( _Tracer_s );
} FCLASS_ALIGN_SUFFIX;



static BOOL _bSystemInitialized = FALSE;
static FLinkRoot_t _GroupRoot;
static void *_pRenderState;


static void _KillActiveTracer( _Group_t *pGroup, _Tracer_t *pTracer, TracerKillReason_e nReason, const FCollImpact_t *pImpact );
static void _GroupWork( _Group_t *pGroup );
static void _ComputePosteredVertexPositions( const _Tracer_t *pTracer, const CFVec3A *pCamPos_WS, FDrawVtx_t *pVtx );
static void _ComputePosterMtx( CFMtx43A *pDestMtx43, const _Tracer_t *pTracer, const CFVec3A *pCamPos_WS );
static void _RenderStateMemFreedCallback( void *pResMem );


//
//
//
BOOL tracer_InitSystem( void ) 
{
	FASSERT( !_bSystemInitialized );

	flinklist_InitRoot( &_GroupRoot, (s32)FANG_OFFSETOF( _Group_t, Link ) );

	_pRenderState = NULL;

	_bSystemInitialized = TRUE;

	return TRUE;
}


//
//
//
void tracer_UninitSystem( void ) 
{
	_bSystemInitialized = FALSE;
}


//
//
//
BOOL tracer_IsSystemInitialized( void ) 
{
	return _bSystemInitialized;
}


//
//
//
u32 tracer_GetGroupCount( void ) 
{
	FASSERT( _bSystemInitialized );
	return _GroupRoot.nCount;
}


//
//
//
TracerGroupHandle_t tracer_CreateGroup( CFTexInst *pTexInst, u32 nMaxTracerCount ) 
{
	_Group_t *pGroup;
	FResFrame_t ResFrame;
	FDrawVtx_t *pVtx;
	u32 i;

	FASSERT( _bSystemInitialized );

	// Cap the maximum number of tracers to 65535...
	if ( nMaxTracerCount > 65535 ) 
	{
		DEVPRINTF( "tracer_CreateGroup(): More than 65535 tracers specified. Clamping to 65535.\n" );
		nMaxTracerCount = 65535;
	}

	// Start a memory frame...
	ResFrame = fres_GetFrame();

	// Allocate memory to hold a new group...
	pGroup = (_Group_t *)fres_AllocAndZero( sizeof(_Group_t) );
	if ( pGroup == NULL ) 
	{
		// Not enough memory...
		DEVPRINTF( "tracer_CreateGroup(): Could not allocated memory for new group.\n" );
		goto _ExitCreateGroupWithError;
	}

	// Create group...
	pGroup->pTexInst = pTexInst;
	pGroup->nMaxTracerCount = (u16)nMaxTracerCount;

	// Allocate tracer buffer...
	pGroup->pTracerBuf = (_Tracer_t *)fres_AlignedAllocAndZero( pGroup->nMaxTracerCount * sizeof(_Tracer_t), FCLASS_BYTE_ALIGN );
	if ( pGroup->pTracerBuf == NULL ) 
	{
		// Not enough memory...
		DEVPRINTF( "tracer_CreateGroup(): Could not allocated memory for tracer buffer.\n" );
		goto _ExitCreateGroupWithError;
	}

	// Allocate vertex buffer...
	pGroup->pVtxBuf = (FDrawVtx_t *)fres_AllocAndZero( _VERTS_PER_TRACER * pGroup->nMaxTracerCount * sizeof(FDrawVtx_t) );
	if ( pGroup->pVtxBuf == NULL ) 
	{
		// Not enough memory...
		DEVPRINTF( "tracer_CreateGroup(): Could not allocated memory for vertex buffer.\n" );
		goto _ExitCreateGroupWithError;
	}

	// Init vertex buffer with values that never change...
	for ( i = 0, pVtx = pGroup->pVtxBuf; i < pGroup->nMaxTracerCount; i++, pVtx += _VERTS_PER_TRACER ) 
	{
		pVtx[0].ST.Set( 0.0f, 0.0f );
		pVtx[2].ST.Set( 0.0f, 1.0f );
		pVtx[3].ST = pVtx[2].ST;
	}

	// Add group to our linklist...
	flinklist_AddTail( &_GroupRoot, pGroup );

	// Save our render state...
	if ( _pRenderState == NULL ) 
	{
		frenderer_Push( FRENDERER_DRAW, NULL );

		_pRenderState = fres_AlignedCreateAndAllocAndZero( NULL, NULL, _RenderStateMemFreedCallback, frenderer_GetStateSize(), FCLASS_BYTE_ALIGN, NULL );
		if ( _pRenderState == NULL ) 
		{
			frenderer_Pop();
			goto _ExitCreateGroupWithError;
		}

		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
		fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
		frenderer_GetState( _pRenderState );
		frenderer_Pop();
	}

	// Group created successfully...

	return (TracerGroupHandle_t)pGroup;

_ExitCreateGroupWithError:
	fres_ReleaseFrame( ResFrame );
	return TRACER_NULLGROUPHANDLE;
}


//
//
//
void tracer_DestroyGroup( TracerGroupHandle_t hGroup ) 
{
	_Group_t *pGroup;

	if ( hGroup == TRACER_NULLGROUPHANDLE ) 
	{
		return;
	}

	FASSERT( _bSystemInitialized );

	pGroup = (_Group_t *)hGroup;

	tracer_DestroyAllTracers( hGroup );

	flinklist_Remove( &_GroupRoot, pGroup );
}


void tracer_SetNewTexture( TracerGroupHandle_t hGroup, CFTexInst *pTexInst )
{
	_Group_t *pGroup;

	if ( hGroup == TRACER_NULLGROUPHANDLE ) 
	{
		return;
	}

	FASSERT( _bSystemInitialized );

	pGroup = (_Group_t *)hGroup;

	pGroup->pTexInst = pTexInst;
}



//
//
//
u32 tracer_GetMaxTracerCount( TracerGroupHandle_t hGroup ) 
{
	FASSERT( _bSystemInitialized );

	if ( hGroup != TRACER_NULLGROUPHANDLE ) 
	{
		return ((_Group_t *)hGroup)->nMaxTracerCount;
	} 
	else 
	{
		return 0;
	}
}


//
//
//
u32 tracer_GetActiveTracerCount( TracerGroupHandle_t hGroup ) 
{
	FASSERT( _bSystemInitialized );

	if ( hGroup != TRACER_NULLGROUPHANDLE ) 
	{
		return ((_Group_t *)hGroup)->nActiveCount;
	} 
	else 
	{
		return 0;
	}
}

//
//
//
u32 tracer_GetFreeTracerCount( TracerGroupHandle_t hGroup ) 
{
	FASSERT( _bSystemInitialized );

	if ( hGroup != TRACER_NULLGROUPHANDLE ) 
	{
		return ((_Group_t *)hGroup)->nMaxTracerCount - ((_Group_t *)hGroup)->nActiveCount;
//		return ((_Group_t *)hGroup)->nMaxTracerCount - ((_Group_t *)hGroup)->nActiveCount - ((_Group_t *)hGroup)->nQueuedCount;
	} 
	else 
	{
		return 0;
	}
}


//
// Returns TRUE if a tracer was created, or FALSE if it could not be.
//
// Important: It's possible that the tracer's kill callback will be called by this function if
//            the initial position described by pTracerDef is hitting an object during this call.
//
static _Tracer_t* _pLastTracer = NULL;
BOOL tracer_NewTracer( TracerGroupHandle_t hGroup, const TracerDef_t *pTracerDef, f32 fHeadDistanceOut, 
						BOOL bUseFadeTail, BOOL bImmediateAbsorb, const CDamageForm::Damager_t *pDamager ) 
{
	_Group_t *pGroup;
	_Tracer_t *pTracer;
	FDrawVtx_t *pVtx;

	FASSERT( _bSystemInitialized );
	FASSERT( fHeadDistanceOut >= 0.f );

	_pLastTracer = NULL;
	if ( hGroup == TRACER_NULLGROUPHANDLE ) 
	{
		return FALSE;
	}

	pGroup = (_Group_t *)hGroup;

	// See if there's room in our tracer buffer for a new tracer...
	if ( pGroup->nActiveCount >= pGroup->nMaxTracerCount ) 
	{
		// No more room for another tracer...
		return FALSE;
	}

	pTracer = pGroup->pTracerBuf + pGroup->nActiveCount;
	pVtx = pGroup->pVtxBuf + (_VERTS_PER_TRACER * pGroup->nActiveCount);
	_pLastTracer = pTracer;

	// Initialize tracer parameters...
	fang_MemCopy( &pTracer->TracerDef, pTracerDef, sizeof(TracerDef_t) );
	pTracer->fOOMaxDist_WS = fmath_Inv( pTracer->TracerDef.fMaxTailDist_WS );
	pTracer->fOOLength_WS = fmath_Inv( pTracer->TracerDef.fLength_WS );
	pTracer->fDeathFadeoutK = fmath_Inv( 1.f - pTracer->TracerDef.fBeginDeathUnitFade_WS );
	pTracer->vCollRayStart_WS = pTracer->TracerDef.TailPos_WS;
	pTracer->pMoveCB = NULL;
	pTracer->pMoveCBData = NULL;

	if( pDamager ) {
		pTracer->TracerDef.Damager = *pDamager;
	} else {
		pTracer->TracerDef.Damager.nDamagerPlayerIndex = -1;
		pTracer->TracerDef.Damager.pBot = NULL;
		pTracer->TracerDef.Damager.pEntity = NULL;
		pTracer->TracerDef.Damager.pWeapon = NULL;
	}

	if ( fHeadDistanceOut > 0.f )
	{
		f32 fTravel = fHeadDistanceOut * pTracerDef->fLength_WS;
		pTracer->vHeadPos_WS.Mul( pTracerDef->UnitDir_WS, fTravel );
		pTracer->vHeadPos_WS.Add( pTracerDef->TailPos_WS );
		if ( fHeadDistanceOut > 1.f )
		{
			fTravel = fTravel - pTracerDef->fLength_WS;
			pTracer->TracerDef.TailPos_WS.Mul( pTracerDef->UnitDir_WS, fTravel );
			pTracer->vFadePos_WS.Set( pTracerDef->TailPos_WS );
		}
		else
		{
			pTracer->vFadePos_WS.Set( pTracerDef->TailPos_WS );
		}
		pTracer->fTotalDistMoved_WS = fTravel;
	}
	else
	{
		pTracer->vHeadPos_WS.Set( pTracerDef->TailPos_WS );
		pTracer->vFadePos_WS.Set( pTracerDef->TailPos_WS );
		pTracer->fTotalDistMoved_WS = 0.f;
	}
	pTracer->nTracerIndex = pGroup->nActiveCount;
	pTracer->bAbsorbMode = bImmediateAbsorb;
	pTracer->nAddedFrame = FVid_nFrameCounter;
	pTracer->bUseFadeTail = bUseFadeTail;

	pTracer->AbsorbPoint_WS.Set( pTracer->vHeadPos_WS );

	// Initialize tracer static values...
	pVtx[0].ST.Set( 0.f, 0.f );
	pVtx[0].ColorRGBA = pTracerDef->ColorRGBA;
	pVtx[1].ST.Set( 1.f, 0.f );
	pVtx[1].ColorRGBA = pTracerDef->ColorRGBA;
	pVtx[2].ST.Set( 0.f, 1.f );
	pVtx[2].ColorRGBA = pTracerDef->ColorRGBA;
	pVtx[3] = pVtx[2];
	pVtx[4] = pVtx[1];
	pVtx[5].ST.Set( 1.f, 1.f );
	pVtx[5].ColorRGBA = pTracerDef->ColorRGBA;
	
	pVtx[6].ST.Set( 1.f, 0.f );
	pVtx[6].ColorRGBA = pTracerDef->ColorRGBA;
	pVtx[7].ST.Set( 0.f, 0.f );
	pVtx[7].ColorRGBA = pTracerDef->ColorRGBA;
	pVtx[7].ColorRGBA.fAlpha = 0.f;
	pVtx[8].ST.Set( 1.f, 1.f );
	pVtx[8].ColorRGBA = pTracerDef->ColorRGBA;
	pVtx[9] = pVtx[8];
	pVtx[10] = pVtx[7];
	pVtx[11].ST.Set( 0.f, 1.f );
	pVtx[11].ColorRGBA = pTracerDef->ColorRGBA;
	pVtx[11].ColorRGBA.fAlpha = 0.f;

	pGroup->nActiveCount++;
	
	pTracer->uFlags = pTracerDef->uFlags;
	pTracer->fAlphaFadeStartTime = pTracerDef->fAlphaFadeStartTime;
	pTracer->fAlphaFadeTime = pTracerDef->fAlphaFadeTime;
	pTracer->fTimeOfBirth = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
	
	return TRUE;
}

void tracer_AddExtraParamsToLastTracer(GlobalTracerMoveCB* pMoveCB, void* pMoveCBData)
{
	if (_pLastTracer)
	{
		_pLastTracer->pMoveCB = pMoveCB;
		_pLastTracer->pMoveCBData = pMoveCBData;
	}
}
//
// Destroys all tracers.
//
void tracer_DestroyAllTracers( void ) 
{
	_Group_t *pGroup;

	FASSERT( _bSystemInitialized );

	for ( pGroup = (_Group_t *)flinklist_GetHead( &_GroupRoot ); pGroup; pGroup=(_Group_t *)flinklist_GetNext( &_GroupRoot, pGroup ) ) 
	{
		pGroup->nActiveCount = 0;
//		pGroup->nQueuedCount = 0;
	}
}


//
// Destroys all tracers in the specified group.
//
void tracer_DestroyAllTracers( TracerGroupHandle_t hGroup ) 
{
	_Group_t *pGroup;

	FASSERT( _bSystemInitialized );

	if ( hGroup == TRACER_NULLGROUPHANDLE ) 
	{
		return;
	}

	pGroup = (_Group_t *)hGroup;
	if ( !pGroup )
	{
		return;
	}

	pGroup->nActiveCount = 0;
//	pGroup->nQueuedCount = 0;
}


//
//
//
void tracer_Work( void ) 
{
	_Group_t *pGroup;

	FASSERT( _bSystemInitialized );

	for ( pGroup = (_Group_t *)flinklist_GetHead( &_GroupRoot ); pGroup; pGroup = (_Group_t *)flinklist_GetNext( &_GroupRoot, pGroup ) ) 
	{
		_GroupWork( pGroup );
	}
}


//
// Assumes:
//   - Xfm stack has been set up with proper camera and identity model xfms
//   - FDraw is the active renderer
//   - The viewport has been set up
//
void tracer_Draw( void ) 
{
	_Group_t *pGroup;
	BOOL bStateSet;
	u32 i;

	FASSERT( _bSystemInitialized );
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if ( fviewport_GetActive() == NULL ) 
	{
		return;
	}

	bStateSet = FALSE;

	for ( pGroup = (_Group_t *)flinklist_GetHead( &_GroupRoot ); pGroup; pGroup=(_Group_t *)flinklist_GetNext( &_GroupRoot, pGroup ) ) 
	{
		if ( pGroup->nActiveCount ) 
		{
			if ( !bStateSet ) 
			{
				frenderer_SetState( _pRenderState );
				bStateSet = TRUE;
			}

			for ( i = 0; i < pGroup->nActiveCount; i++ )
			{
				// Initialize tracer vertex coordinates...
				_ComputePosteredVertexPositions( &pGroup->pTracerBuf[i], &FXfm_pView->m_MtxR.m_vPos, pGroup->pVtxBuf + (i * _VERTS_PER_TRACER) );
			}
			
			if( pGroup->pTracerBuf->uFlags & TRACERFLAG_DRAW_ADDITIVE ) {
				fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST);
			} else {
				fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);
			}

			fdraw_SetTexture( pGroup->pTexInst );
			fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pGroup->pVtxBuf, pGroup->nActiveCount * _VERTS_PER_TRACER );
		}
	}
}


//
// Set nReason to TRACER_KILLREASON_COUNT to not call the callback function.
//
static void _KillActiveTracer( _Group_t *pGroup, _Tracer_t *pTracer, TracerKillReason_e nReason, const FCollImpact_t *pImpact ) 
{
	u32 nTracerIndex;

	if ( nReason != TRACER_KILLREASON_COUNT ) 
	{
		if ( pTracer->TracerDef.pFcnKillCallback ) 
		{
			// Inform callback function that the tracer is about to be killed...
			pTracer->TracerDef.pFcnKillCallback( &pTracer->TracerDef, nReason, pImpact );
		}
	}

	nTracerIndex = pTracer->nTracerIndex;

	FASSERT( nTracerIndex < pGroup->nActiveCount );

	// One less active tracer in this group...

	if (pTracer->pMoveCB)
	{
		(*(pTracer->pMoveCB))(pTracer->pMoveCBData, pTracer->vHeadPos_WS, TRACER_DIED);
	}

	pGroup->nActiveCount--;

	if ( nTracerIndex < pGroup->nActiveCount ) 
	{
		// Copy last tracer in group's tracer buffer to our killed tracer's slot...
		fang_MemCopy( pTracer, &pGroup->pTracerBuf[ pGroup->nActiveCount ], sizeof(_Tracer_t) );
		pTracer->nTracerIndex = (u16)nTracerIndex;

		// Copy the tracer's vertices, too...
		fang_MemCopy( &pGroup->pVtxBuf[ _VERTS_PER_TRACER * nTracerIndex ], &pGroup->pVtxBuf[ _VERTS_PER_TRACER * pGroup->nActiveCount ], _VERTS_PER_TRACER * sizeof(FDrawVtx_t) );
	}
}


//
//
//
static void _GroupWork( _Group_t *pGroup ) 
{
	u32 i;
	_Tracer_t *pTracer;
	FCollImpact_t Impact;
	BOOL bClipHeadToAbsorbPlane;
	CFVec3A DeltaPos_WS, DeltaClipVec_WS, TailToAbsorbPoint_WS;
	f32 fDeltaDistToMove_WS, fUnitDistMoved, fUnitOpaqueness, fDistFromTailToAbsorbPoint;

	FDrawVtx_t *pVtx = pGroup->pVtxBuf;
	pTracer = pGroup->pTracerBuf;
	for ( i = 0; i < pGroup->nActiveCount; i++, pTracer++, pVtx += _VERTS_PER_TRACER ) 
	{
//		if ( pTracer->nAddedFrame == FVid_nFrameCounter ) 
//		{
			// This tracer was added this frame. Don't move it...
//			continue;
//		}

		// Compute the distance to move the tracer...
		fDeltaDistToMove_WS = pTracer->TracerDef.fSpeed_WS * FLoop_fPreviousLoopSecs;

		// Compute the total distance we've moved the tracer...
		pTracer->fTotalDistMoved_WS += fDeltaDistToMove_WS;
		if ( pTracer->fTotalDistMoved_WS >= pTracer->TracerDef.fMaxTailDist_WS ) 
		{
			// We've moved as far as we're going to go...

			_KillActiveTracer( pGroup, pTracer, TRACER_KILLREASON_REACHED_MAX_DIST, NULL );

			i--;
			pTracer--;
			pVtx -= _VERTS_PER_TRACER;
			continue;
		}

		// Compute unit distance we've moved...
		fUnitDistMoved = pTracer->fTotalDistMoved_WS * pTracer->fOOMaxDist_WS;
		// fOOMaxDist_WS uses an approximate reciprocal. Near the end of a tracer's
		// lifetime that can round the normalized distance slightly above 1.0 even
		// though the max-distance check above has not fired yet. Keep the fade
		// input in range so it cannot produce a negative vertex alpha.
		if( fUnitDistMoved < 0.0f ) {
			fUnitDistMoved = 0.0f;
		} else if( fUnitDistMoved > 1.0f ) {
			fUnitDistMoved = 1.0f;
		}

		// Compute overall opaqueness...
		if ( fUnitDistMoved > pTracer->TracerDef.fBeginDeathUnitFade_WS ) 
		{
			fUnitOpaqueness = (1.0f - fUnitDistMoved) * pTracer->fDeathFadeoutK * pTracer->TracerDef.ColorRGBA.fAlpha;
			if( fUnitOpaqueness < 0.0f ) {
				fUnitOpaqueness = 0.0f;
			} else if( fUnitOpaqueness > 1.0f ) {
				fUnitOpaqueness = 1.0f;
			}

			pVtx[0].ColorRGBA.fAlpha = fUnitOpaqueness;
			pVtx[1].ColorRGBA.fAlpha = fUnitOpaqueness;
			pVtx[2].ColorRGBA.fAlpha = fUnitOpaqueness;
			pVtx[3].ColorRGBA.fAlpha = fUnitOpaqueness;
			pVtx[4].ColorRGBA.fAlpha = fUnitOpaqueness;
			pVtx[5].ColorRGBA.fAlpha = fUnitOpaqueness;
		}

		f32 fNowTime = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
		if ( pTracer->uFlags & TRACERFLAG_USE_ALPHA_FADES )
		{
			if (pTracer->fTimeOfBirth+pTracer->fAlphaFadeStartTime+pTracer->fAlphaFadeTime > fNowTime)
			{
				if (pTracer->fTimeOfBirth+pTracer->fAlphaFadeStartTime < fNowTime)
				{	//alpha is fading out
					fUnitDistMoved = (fNowTime - (pTracer->fTimeOfBirth+pTracer->fAlphaFadeStartTime))/pTracer->fAlphaFadeTime;
					if( fUnitDistMoved < 0.0f ) {
						fUnitDistMoved = 0.0f;
					} else if( fUnitDistMoved > 1.0f ) {
						fUnitDistMoved = 1.0f;
					}
					fUnitOpaqueness = (1.0f - fUnitDistMoved) * pTracer->TracerDef.ColorRGBA.fAlpha;
					if( fUnitOpaqueness < 0.0f ) {
						fUnitOpaqueness = 0.0f;
					} else if( fUnitOpaqueness > 1.0f ) {
						fUnitOpaqueness = 1.0f;
					}
					pVtx[0].ColorRGBA.fAlpha = fUnitOpaqueness;
					pVtx[1].ColorRGBA.fAlpha = fUnitOpaqueness;
					pVtx[2].ColorRGBA.fAlpha = fUnitOpaqueness;
					pVtx[3].ColorRGBA.fAlpha = fUnitOpaqueness;
					pVtx[4].ColorRGBA.fAlpha = fUnitOpaqueness;
					pVtx[5].ColorRGBA.fAlpha = fUnitOpaqueness;
				}
			}
			else
			{
				_KillActiveTracer( pGroup, pTracer, TRACER_KILLREASON_REACHED_MAX_DIST, NULL );

				i--;
				pTracer--;
				pVtx -= _VERTS_PER_TRACER;
				continue;
			}
		}


		// Compute delta move vector...
		DeltaPos_WS.Mul( pTracer->TracerDef.UnitDir_WS, fDeltaDistToMove_WS );

 		// Assume we won't need to clip head to absorb plane...
		bClipHeadToAbsorbPlane = FALSE;

		// Once the head has grown out to the length of the tracer, we start moving the tail and fade
		f32 fHeadLength = 0;
		if ( pTracer->fTotalDistMoved_WS > pTracer->TracerDef.fLength_WS || pTracer->bAbsorbMode )
		{
			// Update tail position...
			if ( fDeltaDistToMove_WS > pTracer->TracerDef.fLength_WS )
			{
				// If we moved more than the length of the tracer, stretch the head to
				// tail size to the distance traveled.
				pTracer->TracerDef.TailPos_WS.Set( pTracer->vHeadPos_WS );
				fHeadLength = fDeltaDistToMove_WS;
			}
			else
			{
				// Otherwise, we've traveled less than the length of the tracer, so
				// the tracer size should be equal to the length
				pTracer->TracerDef.TailPos_WS.Add( DeltaPos_WS );
				fHeadLength = pTracer->TracerDef.fLength_WS;
			}
		}

		if ( !pTracer->bAbsorbMode ) 
		{
			// Move head position...
			pTracer->vHeadPos_WS.Add( DeltaPos_WS );

			// Do collision...
			FWorld_nTrackerSkipListCount = 0;

			if( pTracer->TracerDef.pFcnBuildSkipList != NULL ) {
				pTracer->TracerDef.pFcnBuildSkipList( pTracer->TracerDef.pUser );
			}

			u16 nCollMask;

			if( !(pTracer->TracerDef.uFlags & TRACERFLAG_THICK_PROJECTILE) )
			{
				nCollMask = FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES;
			}
			else
			{
				nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
			}

			if ( fworld_FindClosestImpactPointToRayStart( &Impact, &pTracer->vCollRayStart_WS, &pTracer->vHeadPos_WS, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, NULL, -1, nCollMask ) ) 
			{
				// Found collision...

				if ( pTracer->TracerDef.pFcnKillCallback ) 
				{
					// Inform callback function that the tracer hit something...
					pTracer->TracerDef.pFcnKillCallback( &pTracer->TracerDef, TRACER_KILLREASON_HIT_GEO, &Impact );
				}

				pTracer->bAbsorbMode = TRUE;
				pTracer->AbsorbPoint_WS.Set( Impact.ImpactPoint );

				// Head could be poking out of absorb plane, so flag to clip it...
				bClipHeadToAbsorbPlane = TRUE;
			}
		}

		if ( pTracer->bAbsorbMode ) 
		{
			// Compute distance from tail to absorb point...
			TailToAbsorbPoint_WS.Sub( pTracer->AbsorbPoint_WS, pTracer->TracerDef.TailPos_WS );
			fDistFromTailToAbsorbPoint = TailToAbsorbPoint_WS.Dot( pTracer->TracerDef.UnitDir_WS );
			
			if ( fDistFromTailToAbsorbPoint <= 0.0f ) 
			{
				// Tail has passed beyond the absorb point...
				_KillActiveTracer( pGroup, pTracer, TRACER_KILLREASON_COUNT, NULL );
				i--;
				pTracer--;
				pVtx -= _VERTS_PER_TRACER;
				continue;
			}
#if 1
			pTracer->vHeadPos_WS.Set( pTracer->AbsorbPoint_WS );
#else
			// Compute unit distance from tail to absorb plane...
			f32 fUnitDistFromTailToAbsorbPlane = fDistFromTailToAbsorbPoint * pTracer->fOOLength_WS;

			if ( bClipHeadToAbsorbPlane ) 
			{
				// Clip head to absorb plane...

				// Compute distance head pokes through absorb plane...
				f32 fPenetrationDist = pTracer->TracerDef.fLength_WS - fDistFromTailToAbsorbPoint;

				// Compute negative adjustment vector...
				DeltaClipVec_WS.Mul( pTracer->TracerDef.UnitDir_WS, fPenetrationDist );

				// Clip head position and vertices...
				pTracer->vHeadPos_WS.Sub( DeltaClipVec_WS );
			}
#endif
		}

		if ( fHeadLength )
		{
			// Update fade position...
			f32 fFadeLength = (fDeltaDistToMove_WS * 2.f);
			if ( fFadeLength + fHeadLength < pTracer->fTotalDistMoved_WS )
			{
				CFVec3A vFadeSize;
				vFadeSize.Mul( DeltaPos_WS, 2.f );
				pTracer->vFadePos_WS.Sub( pTracer->TracerDef.TailPos_WS, DeltaPos_WS );
			}
			else if ( pTracer->fTotalDistMoved_WS > fHeadLength )
			{
				CFVec3A vFadeSize;
				vFadeSize.Mul( pTracer->TracerDef.UnitDir_WS, pTracer->fTotalDistMoved_WS - fHeadLength );
				pTracer->vFadePos_WS.Sub( pTracer->TracerDef.TailPos_WS, vFadeSize );
			}
		}
		
		if (pTracer->pMoveCB)
		{
			(*(pTracer->pMoveCB))(pTracer->pMoveCBData, pTracer->vHeadPos_WS, TRACER_STILL_ALIVE);
		}
	

		pTracer->vCollRayStart_WS.Set( pTracer->vHeadPos_WS );
	}
}


//
//
//
static void _ComputePosteredVertexPositions( const _Tracer_t *pTracer, const CFVec3A *pCamPos_WS, FDrawVtx_t *pVtx ) 
{
	CFVec3A TracerToCam_WS;
	CFMtx43A Mtx43;

	// Compute poster matrix...
	_ComputePosterMtx( &Mtx43, pTracer, pCamPos_WS );

	CFVec3A vDiff, vScaledZ;
	vDiff.Sub( pTracer->TracerDef.TailPos_WS, pTracer->vHeadPos_WS );
	f32 fLength = vDiff.Mag();
	vScaledZ.Mul( Mtx43.m_vFront, fLength );
	
	// Compute world space vertices...
	pVtx[0].Pos_MS.x = -0.5f*Mtx43.aa[0][0] + Mtx43.aa[3][0];
	pVtx[0].Pos_MS.y = -0.5f*Mtx43.aa[0][1] + Mtx43.aa[3][1];
	pVtx[0].Pos_MS.z = -0.5f*Mtx43.aa[0][2] + Mtx43.aa[3][2];
	pVtx[1].Pos_MS.x = -0.5f*Mtx43.aa[0][0] + vScaledZ.x + Mtx43.aa[3][0];
	pVtx[1].Pos_MS.y = -0.5f*Mtx43.aa[0][1] + vScaledZ.y + Mtx43.aa[3][1];
	pVtx[1].Pos_MS.z = -0.5f*Mtx43.aa[0][2] + vScaledZ.z + Mtx43.aa[3][2];
	pVtx[2].Pos_MS.x =  0.5f*Mtx43.aa[0][0] + Mtx43.aa[3][0];
	pVtx[2].Pos_MS.y =  0.5f*Mtx43.aa[0][1] + Mtx43.aa[3][1];
	pVtx[2].Pos_MS.z =  0.5f*Mtx43.aa[0][2] + Mtx43.aa[3][2];
	pVtx[5].Pos_MS.x =  0.5f*Mtx43.aa[0][0] + vScaledZ.x + Mtx43.aa[3][0];
	pVtx[5].Pos_MS.y =  0.5f*Mtx43.aa[0][1] + vScaledZ.y + Mtx43.aa[3][1];
	pVtx[5].Pos_MS.z =  0.5f*Mtx43.aa[0][2] + vScaledZ.z + Mtx43.aa[3][2];
	pVtx[3].Pos_MS = pVtx[2].Pos_MS;
	pVtx[4].Pos_MS = pVtx[1].Pos_MS;

	vDiff.Sub( pTracer->TracerDef.TailPos_WS, pTracer->vFadePos_WS );
	fLength = vDiff.Mag();
	vScaledZ.Mul( Mtx43.m_vFront, -fLength );
	
	if ( !pTracer->bUseFadeTail )
	{
		// Compute world space vertices...
		pVtx[6].ColorRGBA.Set( 0.f,0.f,0.f, 0.f );
		pVtx[7].ColorRGBA.Set( 0.f,0.f,0.f, 0.f );
		pVtx[8].ColorRGBA.Set( 0.f,0.f,0.f, 0.f );
		pVtx[9].ColorRGBA.Set( 0.f,0.f,0.f, 0.f );
		pVtx[10].ColorRGBA.Set( 0.f,0.f,0.f, 0.f );
		pVtx[11].ColorRGBA.Set( 0.f,0.f,0.f, 0.f );
		return;
	}
	// Compute world space vertices...
	pVtx[6].Pos_MS.x = -0.5f*Mtx43.aa[0][0] + Mtx43.aa[3][0];
	pVtx[6].Pos_MS.y = -0.5f*Mtx43.aa[0][1] + Mtx43.aa[3][1];
	pVtx[6].Pos_MS.z = -0.5f*Mtx43.aa[0][2] + Mtx43.aa[3][2];
	pVtx[7].Pos_MS.x = -0.5f*Mtx43.aa[0][0] + vScaledZ.x + Mtx43.aa[3][0];
	pVtx[7].Pos_MS.y = -0.5f*Mtx43.aa[0][1] + vScaledZ.y + Mtx43.aa[3][1];
	pVtx[7].Pos_MS.z = -0.5f*Mtx43.aa[0][2] + vScaledZ.z + Mtx43.aa[3][2];
	pVtx[8].Pos_MS.x =  0.5f*Mtx43.aa[0][0] + Mtx43.aa[3][0];
	pVtx[8].Pos_MS.y =  0.5f*Mtx43.aa[0][1] + Mtx43.aa[3][1];
	pVtx[8].Pos_MS.z =  0.5f*Mtx43.aa[0][2] + Mtx43.aa[3][2];
	pVtx[11].Pos_MS.x =  0.5f*Mtx43.aa[0][0] + vScaledZ.x + Mtx43.aa[3][0];
	pVtx[11].Pos_MS.y =  0.5f*Mtx43.aa[0][1] + vScaledZ.y + Mtx43.aa[3][1];
	pVtx[11].Pos_MS.z =  0.5f*Mtx43.aa[0][2] + vScaledZ.z + Mtx43.aa[3][2];
	pVtx[9].Pos_MS = pVtx[8].Pos_MS;
	pVtx[10].Pos_MS = pVtx[7].Pos_MS;
}


//
// Important: Does not compute pDestMtx43->aa[1] (not needed for card postering).
//
static void _ComputePosterMtx( CFMtx43A *pDestMtx43, const _Tracer_t *pTracer, const CFVec3A *pCamPos_WS ) 
{
	CFVec3A TracerToCam_WS;
	f32 fMagX2;

	// Compute position...
	pDestMtx43->m_vPos = pTracer->TracerDef.TailPos_WS;

	// Compute unit Z axis...
	pDestMtx43->m_vFront = pTracer->TracerDef.UnitDir_WS;

	// Compute unit X axis...
	TracerToCam_WS.Sub( *pCamPos_WS, pTracer->TracerDef.TailPos_WS );
	pDestMtx43->m_vRight = TracerToCam_WS.Cross( pDestMtx43->m_vFront );
	fMagX2 = pDestMtx43->m_vRight.MagSq();
	if ( fMagX2 < 0.0001f ) 
	{
		// Could not compute axis from cross product using tail point. Try head point...
		TracerToCam_WS.Sub( *pCamPos_WS, pTracer->vHeadPos_WS );
		pDestMtx43->m_vRight = TracerToCam_WS.Cross( pDestMtx43->m_vFront );
		fMagX2 = pDestMtx43->m_vRight.MagSq();
		if ( fMagX2 < 0.0001f ) 
		{
			// Could not compute axis from cross product using head point. Try world up vector...
			pDestMtx43->m_vRight.CrossYWithVec( pDestMtx43->m_vFront );
			fMagX2 = pDestMtx43->m_vRight.MagSq();
			if ( fMagX2 < 0.0001f ) 
			{
				// Use world right vector...
				pDestMtx43->m_vRight.Cross( CFVec3A::m_UnitAxisX, pDestMtx43->m_vFront );
				fMagX2 = pDestMtx43->m_vRight.MagSq();
			}
		}
	}
	pDestMtx43->m_vRight.Div( fmath_Sqrt(fMagX2) );

	// Scale the matrix to desired width and length...
	pDestMtx43->m_vRight.Mul( pTracer->TracerDef.fWidth_WS );
}


//
//
//
static void _RenderStateMemFreedCallback( void *pResMem ) 
{
	_pRenderState = NULL;
}

