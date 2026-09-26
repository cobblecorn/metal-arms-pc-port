//////////////////////////////////////////////////////////////////////////////////////
// FXStreamer.cpp - 
//
// Author: Michael Starich   
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
// 03/31/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "FXStreamer.h"
#include "fdatapool.h"
#include "fvtxpool.h"
#include "frenderer.h"
#include "fclib.h"
#include "fdraw.h"
#include "floop.h"

//====================
// private definitions

#define _NUM_PTS_PER_STREAMER				250
#define _NUM_STREAMER_PTS					( _NUM_PTS_PER_STREAMER * FXSTREAMER_NUM_STREAMER_EMITTERS )

struct _StreamerPt_s {
	CFVec3 PosWS;
	f32 fAlpha;
	CFVec3 UnitDir;
	f32 fFadeMultiplier;
};

//=================
// public variables

//==================
// private variables

static BOOL _bSystemInited = FALSE;
static CFDataPool _StreamerPtPool;
static u32 _nNumStreamerEmitters;
static CFXStreamerEmitter *_paStreamerEmitters = NULL;
static u32 _nKey = 0;
static u32 _nMaxEmittersUsed;

//===================
// private prototypes

static CFXStreamerEmitter *_ConvertHandleToPtr( FXStreamerHandle_t hHandle );

//=================
// public functions

///////////////////////////////////////////////////
// START OF THE PUBLIC INTERFACE
///////////////////////////////////////////////////

BOOL CFXStreamerEmitter::InitSystem() {
	FResFrame_t Frame;

	FASSERT( !_bSystemInited );

	Frame = fres_GetFrame();

	// allocate a streamer emitter pool
	_paStreamerEmitters = fnew CFXStreamerEmitter[FXSTREAMER_NUM_STREAMER_EMITTERS];
	if( !_paStreamerEmitters ) {
		goto _ExitSetupWithError;
	}
	_nNumStreamerEmitters = FXSTREAMER_NUM_STREAMER_EMITTERS;

	// allocate a streamer pt pool
	if( !_StreamerPtPool.Create( sizeof( _StreamerPt_t ), _NUM_STREAMER_PTS, 4, 32 ) ) {
		goto _ExitSetupWithError;		
	}

	_nKey = 1;
	_nMaxEmittersUsed = 0;
	_bSystemInited = TRUE;

	return TRUE;

_ExitSetupWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );
	return FALSE;	
}

void CFXStreamerEmitter::UninitSystem() {

	if( _paStreamerEmitters ) {
		fdelete_array( _paStreamerEmitters );
		_paStreamerEmitters = NULL;
	}
	_nNumStreamerEmitters = 0;

	u32 nNumWastedEmitters = FXSTREAMER_NUM_STREAMER_EMITTERS - _nMaxEmittersUsed;
	u32 nNumBytesThatCouldBeSaved = sizeof( CFXStreamerEmitter ) * nNumWastedEmitters;
	nNumBytesThatCouldBeSaved += ( nNumWastedEmitters * _NUM_PTS_PER_STREAMER * sizeof( _StreamerPt_t ) );

	DEVPRINTF( "FXStreamer : %d of %d CFXStreamerEmitter's were used at any one time, you can save %d bytes by lowering the count.\n",
			_nMaxEmittersUsed,
			FXSTREAMER_NUM_STREAMER_EMITTERS,
			nNumBytesThatCouldBeSaved );	
	
	_bSystemInited = FALSE;
}

FXStreamerHandle_t CFXStreamerEmitter::SpawnFromMtx43A( u32 nMaxStreamDepth,
													    const CFMtx43A *pMtx43A,
														f32 fInitialAlpha,
														CFTexInst *pTexture, 
														const CFVec3 *pSpawnPt_MS/*=NULL*/,
														f32 fWidth_WS/*=1.0f*/,
														f32 fSamplesPerSec/*=15.0f*/,
														UseAxis_e nAxis/*=USE_RIGHT_AXIS*/ ) {
	// grab an available handle
	u32 i;
	CFXStreamerEmitter *pStreamerEmitter;
	
	for( i=0; i < _nNumStreamerEmitters; i++ ) {
		pStreamerEmitter = &_paStreamerEmitters[i];
		if( pStreamerEmitter->GetMyHandle() == FXSTREAMER_INVALID_HANDLE ) {
			// call the spawn function
			pStreamerEmitter->CreateMyHandle( i );
			pStreamerEmitter->SpawnStreams( nMaxStreamDepth,
											pMtx43A,
											fInitialAlpha,
											pTexture, 
											pSpawnPt_MS,
											fWidth_WS,
											fSamplesPerSec,
											nAxis );
			return pStreamerEmitter->GetMyHandle();			
		}
	}
	return FXSTREAMER_INVALID_HANDLE;
}

FXStreamerHandle_t CFXStreamerEmitter::SpawnFromMeshBones( u32 nMaxStreamDepth,
														   const CFWorldMesh *pWorldMesh,
														   cchar **papszBoneNames, 
														   f32 fInitialAlpha,
														   CFTexInst *pTexture,
														   f32 fWidth_WS/*=1.0f*/,
														   f32 fSamplesPerSec/*=15.0f*/,
														   UseAxis_e nAxis/*=USE_RIGHT_AXIS*/ ) {
	// grab an available handle
	u32 i;
	CFXStreamerEmitter *pStreamerEmitter;


	// make sure that there are at least 1 but no more than _MAX_STREAMS bone names
	i = 0;
	while( papszBoneNames[i] != NULL ) {
		i++;
		if( i > _MAX_STREAMS ) {
			DEVPRINTF( "CFXStreamerEmitter::SpawnFromMeshBones(): the bone name list contains too many bone names, each streamer can support upto %d bones.\n", _MAX_STREAMS );
            return FXSTREAMER_INVALID_HANDLE;
		}
	};
	if( i == 0 ) {
		DEVPRINTF( "CFXStreamerEmitter::SpawnFromMeshBones(): the bone name list doesn't contain any name, at least 1 is required.\n", _MAX_STREAMS );
        return FXSTREAMER_INVALID_HANDLE;
	}
	
	for( i=0; i < _nNumStreamerEmitters; i++ ) {
		pStreamerEmitter = &_paStreamerEmitters[i];
		if( pStreamerEmitter->GetMyHandle() == FXSTREAMER_INVALID_HANDLE ) {
			// call the spawn function
			pStreamerEmitter->CreateMyHandle( i );
			pStreamerEmitter->SpawnStreams( nMaxStreamDepth,
											pWorldMesh,
											papszBoneNames, 
											fInitialAlpha,
											pTexture,
											fWidth_WS,
											fSamplesPerSec,
											nAxis );
			return pStreamerEmitter->GetMyHandle();			
		}
	}
	return FXSTREAMER_INVALID_HANDLE;
}

void CFXStreamerEmitter::EnableStreamerEmission( FXStreamerHandle_t hHandle, BOOL bEnable ) {

	CFXStreamerEmitter *pStreamerEmitter = _ConvertHandleToPtr( hHandle );
	if( pStreamerEmitter ) {
		pStreamerEmitter->m_bEmitStreamer = bEnable;
		if( bEnable ) {
			pStreamerEmitter->m_bNearingEnd = FALSE;
			pStreamerEmitter->m_fPercentTillEnd = 0.0f;
		}
	}
}

void CFXStreamerEmitter::KillAllStreamers( void ) {
	u32 i;
	CFXStreamerEmitter *pStreamerEmitter;
		
	for( i=0; i < _nNumStreamerEmitters; i++ ) {
		pStreamerEmitter = &_paStreamerEmitters[i];
		if( pStreamerEmitter->GetMyHandle() != FXSTREAMER_INVALID_HANDLE ) {
			pStreamerEmitter->KillAll();
		}
	}
}

BOOL CFXStreamerEmitter::IsValidHandle( FXStreamerHandle_t hHandle ) {

	CFXStreamerEmitter *pStreamerEmitter = _ConvertHandleToPtr( hHandle );
	if( pStreamerEmitter ) {
		return TRUE;
	}
	return FALSE;
}

void CFXStreamerEmitter::ReturnEmitterHandle( FXStreamerHandle_t hHandle ) {

	CFXStreamerEmitter *pStreamerEmitter = _ConvertHandleToPtr( hHandle );
	if( pStreamerEmitter ) {
		pStreamerEmitter->KillAll();
	}
}

void CFXStreamerEmitter::Work() {
	u32 i, nInUseCount = 0;
	CFXStreamerEmitter *pStreamerEmitter;
		
	for( i=0; i < _nNumStreamerEmitters; i++ ) {
		pStreamerEmitter = &_paStreamerEmitters[i];
		if( pStreamerEmitter->GetMyHandle() != FXSTREAMER_INVALID_HANDLE ) {
			pStreamerEmitter->WorkEmitter();
			nInUseCount++;
		}
	}
	// keep track of the max emitters used at any 1 time
	_nMaxEmittersUsed = FMATH_MAX( _nMaxEmittersUsed, nInUseCount );
}

void CFXStreamerEmitter::Draw() {
	u32 i;
	CFXStreamerEmitter *pStreamerEmitter;
	BOOL bStateSet = FALSE;
	
	for( i=0; i < _nNumStreamerEmitters; i++ ) {
		pStreamerEmitter = &_paStreamerEmitters[i];
		if( pStreamerEmitter->GetMyHandle() != FXSTREAMER_INVALID_HANDLE ) {
			
			if( !bStateSet ) {
				// setup the rendering states
				frenderer_Push( FRENDERER_DRAW, NULL );
//fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
				fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
				fdraw_Depth_EnableWriting( FALSE );
				fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );	
				fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
				fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
				bStateSet = TRUE;
			}
			pStreamerEmitter->DrawEmitter();
		}
	}

	if( bStateSet ) {
		// restore the rendering states
		frenderer_Pop();
	}
}

void CFXStreamerEmitter::NearingEnd( FXStreamerHandle_t hHandle, f32 fUnitPercentTillEnd ) {

	CFXStreamerEmitter *pStreamerEmitter = _ConvertHandleToPtr( hHandle );
	if( pStreamerEmitter ) {
		if( pStreamerEmitter->m_bEmitStreamer ) {
			pStreamerEmitter->m_bNearingEnd = TRUE;
			pStreamerEmitter->m_fPercentTillEnd = fUnitPercentTillEnd;
		}
	}
}

void CFXStreamerEmitter::EmitPoint( FXStreamerHandle_t hHandle ) {
	CFXStreamerEmitter *pStreamerEmitter = _ConvertHandleToPtr( hHandle );
	if( pStreamerEmitter ) {
		// schedule a point to be emitted
		pStreamerEmitter->m_fSecsTillNextSample = 0.0f;
	}
}

///////////////////////////////////////////////////
// END OF THE PUBLIC INTERFACE
///////////////////////////////////////////////////

CFXStreamerEmitter::CFXStreamerEmitter() {
	m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
	
	m_nNumPts = 0;
	m_paPts = NULL;
	m_nNumStreams = 0;
	m_nPtsPerStream = 0;
	m_fInitialAlpha = 1.0f;
	m_pTexInst = NULL;
	m_fHalfWidth_WS = 0.5f;
	m_pWorldMesh = NULL;
	m_pOrientation = NULL;
	m_fOOSamplesPerSec = 0.0f;
	m_fFadePerSec = 0.0f;
	m_bEmitStreamer = FALSE;
	m_nOldestIndex = 0;
	m_nUsedCount = 0;
	m_fSecsTillNextSample = 0.0f;
	m_bNearingEnd = FALSE;
	m_fPercentTillEnd = 0.0f;
	m_bEmitFromBones = FALSE;
	m_Offset_MS.Zero();
	m_nAxis = USE_RIGHT_AXIS;
	m_nFlags = FLAGS_NONE;
}

BOOL CFXStreamerEmitter::SpawnStreams( u32 nMaxStreamDepth,
									  const CFMtx43A *pMtx43A,
									  f32 fInitialAlpha,
									  CFTexInst *pTexture, 
									  const CFVec3 *pSpawnPt_MS,
									  f32 fWidth_WS,
									  f32 fSamplesPerSec,
									  UseAxis_e nAxis ) {
	if( !InitCommonFields( nMaxStreamDepth, NULL, fInitialAlpha, pTexture, fWidth_WS, fSamplesPerSec, nAxis ) ) {
		// already spawning streamers or invalid parameters, fail
		m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
		return FALSE;
	}
	if( !pMtx43A ) {
		// must have an mtx43A pointer
		m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
		return FALSE;
	}
	m_pOrientation = pMtx43A;
	
	m_nNumStreams = 1;
	m_bEmitFromBones = FALSE;
	if( pSpawnPt_MS ) {
		m_Offset_MS = *pSpawnPt_MS;
	}

	m_nNumPts = m_nPtsPerStream * m_nNumStreams;
	m_paPts = (_StreamerPt_t *)_StreamerPtPool.GetArray( m_nNumPts );
	if( !m_paPts ) {
		m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
		return FALSE;
	}
		
	m_bEmitStreamer = TRUE;
		
	return TRUE;
}

BOOL CFXStreamerEmitter::SpawnStreams( u32 nMaxStreamDepth,
									 const CFWorldMesh *pWorldMesh,
									 cchar **papszBoneNames, 
									 f32 fInitialAlpha,
									 CFTexInst *pTexture,
									 f32 fWidth_WS,
									 f32 fSamplesPerSec,
									 UseAxis_e nAxis ) {
	u32 i;

	if( !InitCommonFields( nMaxStreamDepth, pWorldMesh, fInitialAlpha, pTexture, fWidth_WS, fSamplesPerSec, nAxis ) ) {
		// already spawning streamers or invalid parameters, fail
		m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
		return FALSE;
	}

	if( !papszBoneNames ) {
		// must have a bone names array
		m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
		return FALSE;
	}
	// find out how many streamers we will be emitting
	u32 nNameIndex = 0, nNumBones = pWorldMesh->m_pMesh->nBoneCount;
	FMeshBone_t *pCurrent;

	m_nNumStreams = 0;
	while( m_nNumStreams < _MAX_STREAMS && papszBoneNames[nNameIndex] != NULL ) {
		for( i=0; i < nNumBones; i++ ) {
			pCurrent = &pWorldMesh->m_pMesh->pBoneArray[i];
			
			if( fclib_stricmp( papszBoneNames[nNameIndex], pCurrent->szName ) == 0 ) {
				m_anBoneIndices[m_nNumStreams] = (u8)i;
				m_nNumStreams++;
				break;
			}
		}
		nNameIndex++;
	}
	if( !m_nNumStreams ) {
		// no streamer bones found, switch to the mesh's origin
		DEVPRINTF( "CFXStreamerEmitter::SpawnFromMeshBones() - Could not find any bones to emit from, switching to mesh emission.\n" );
		m_nNumStreams = 1;
	} else {
		m_bEmitFromBones = TRUE;
	}

	// calculate how many pts we need
	m_nNumPts = m_nPtsPerStream * m_nNumStreams;
	m_paPts = (_StreamerPt_t *)_StreamerPtPool.GetArray( m_nNumPts );
	if( !m_paPts ) {
		m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
		return FALSE;
	}
		
	m_bEmitStreamer = TRUE;
		
	return TRUE;
}

void CFXStreamerEmitter::KillAll() {

	if( m_hMyHandle != FXSTREAMER_INVALID_HANDLE ) {
		m_hMyHandle = FXSTREAMER_INVALID_HANDLE;
		_StreamerPtPool.ReturnToPool( m_paPts, m_nNumPts );
		m_paPts = NULL;
		m_nNumPts = 0;
		m_nUsedCount = 0;
		
		m_pWorldMesh = NULL;
		m_pOrientation = NULL;
		m_bEmitStreamer = FALSE;
		m_bNearingEnd = FALSE;
		m_fPercentTillEnd = 0.0f;
	}
}

void CFXStreamerEmitter::WorkEmitter() {
	u32 i, j, nPtOffset, nIndex, nRemovedIndex;
	_StreamerPt_t *pPt;
	f32 fDeltaAlpha;

	if( m_hMyHandle != FXSTREAMER_INVALID_HANDLE ) {
		// work
		fDeltaAlpha = m_fFadePerSec * FLoop_fPreviousLoopSecs;		
		for( i=0; i < m_nNumStreams; i++ ) {
			nPtOffset = i * m_nPtsPerStream;
			nIndex = m_nOldestIndex;

			pPt = &m_paPts[ nPtOffset + nIndex];
			for( j=0; j < m_nUsedCount; j++ ) {
				pPt->fAlpha -= fDeltaAlpha * pPt->fFadeMultiplier;
												
				nIndex++;
				if( nIndex >= m_nPtsPerStream ) {
					// wrap around
					nIndex = 0;
					pPt = &m_paPts[ nPtOffset + nIndex];
				} else {
					pPt++;
				}
			}
		}
		// see if we can throw out a pt
		if( m_nUsedCount ) {
			if( (m_paPts[m_nOldestIndex].fAlpha <= _MIN_STREAMER_ALPHA) ||
				(m_bEmitStreamer && (m_nUsedCount >= m_nPtsPerStream) ) ) {
				do {
					m_bDrawLastRemoved = TRUE;
					nRemovedIndex = m_nOldestIndex;
					// since the first pt would be the 1st to die, if we are killing a point the 1st must be dead
					m_nFlags &= (~FLAGS_1ST_POINT_STILL_AROUND);

					m_nUsedCount--;
					m_nOldestIndex++;
					if( m_nOldestIndex >= m_nPtsPerStream ) {
						// wrap around
						m_nOldestIndex = 0;
					}
				} while( m_nUsedCount && (m_paPts[m_nOldestIndex].fAlpha <= _MIN_STREAMER_ALPHA) );

				// copy each streams last removed pt
				for( i=0; i < m_nNumStreams; i++ ) {
					pPt = &m_paPts[ (i * m_nPtsPerStream) + nRemovedIndex ];
					
					m_aLastRemovedPosWS[i] = pPt->PosWS;
					m_afLastRemovedAlpha[i] = pPt->fAlpha;
					FMATH_CLAMPMIN( m_afLastRemovedAlpha[i], (_MIN_STREAMER_ALPHA*0.5) );
					m_aLastRemovedUnitDir[i] = pPt->UnitDir;
				}

			} else {
				m_bDrawLastRemoved = FALSE;
			}
		} else {
			m_bDrawLastRemoved = FALSE;
		}

		// emit
		if( m_bEmitStreamer ) {
			// grab the bots mtx array

			// figure out if we should emit a pt
			m_fSecsTillNextSample -= FLoop_fPreviousLoopSecs;
			if( m_fSecsTillNextSample <= 0.0f ) {
				m_fSecsTillNextSample += m_fOOSamplesPerSec;

				_EmitPoint();

			} else if( m_nUsedCount > 1 ) {
				// don't emit a point, but update the last emitted point so that there aren't gaps

				CFMtx43A **ppMtxPalette = NULL;
				if( m_bEmitFromBones ) {
					ppMtxPalette = m_pWorldMesh->GetBoneMtxPalette();
				}

				nIndex = m_nOldestIndex + m_nUsedCount - 1;
				if( nIndex >= m_nPtsPerStream ) {
					nIndex -= m_nPtsPerStream;
				}

				pPt = &m_paPts[nIndex];
				for( i=0; i < m_nNumStreams; i++ ) {
					if( m_bEmitFromBones ) {
						pPt->PosWS = ppMtxPalette[ m_anBoneIndices[i] ]->m_vP.v3;
						switch( m_nAxis ) {
						default:
						case USE_RIGHT_AXIS:
							pPt->UnitDir = ppMtxPalette[ m_anBoneIndices[i] ]->m_vRight.v3;
							break;
						case USE_UP_AXIS:
							pPt->UnitDir = ppMtxPalette[ m_anBoneIndices[i] ]->m_vUp.v3;
							break;
						case USE_FRONT_AXIS:
							pPt->UnitDir = ppMtxPalette[ m_anBoneIndices[i] ]->m_vFront.v3;
							break;
						}						
					} else {
						pPt->PosWS = m_pOrientation->m44.MultPoint( m_Offset_MS );
						switch( m_nAxis ) {
						default:
						case USE_RIGHT_AXIS:
							pPt->UnitDir = m_pOrientation->m_vRight.v3;
							break;
						case USE_UP_AXIS:
							pPt->UnitDir = m_pOrientation->m_vUp.v3;
							break;
						case USE_FRONT_AXIS:
							pPt->UnitDir = m_pOrientation->m_vFront.v3;
							break;
						}
					}

					pPt += m_nPtsPerStream;
				}
			}
		} else if( m_nUsedCount == 0 ) {
			// there are no more pts so we can't draw anything
			KillAll();
			return;
		}
	}
}

void CFXStreamerEmitter::_EmitPoint( void ) {
	u32 i, nIndex;
	_StreamerPt_t *pPt;
	CFMtx43A **ppMtxPalette = NULL;

	if( m_bEmitFromBones ) {
		ppMtxPalette = m_pWorldMesh->GetBoneMtxPalette();
	}

	// determine where to add a pt
	nIndex = m_nOldestIndex + m_nUsedCount;
	if( nIndex >= m_nPtsPerStream ) {
		nIndex -= m_nPtsPerStream;
	}

	pPt = &m_paPts[nIndex];
	for( i=0; i < m_nNumStreams; i++ ) {
		if( m_bEmitFromBones ) {
			pPt->PosWS = ppMtxPalette[ m_anBoneIndices[i] ]->m_vPos.v3;
			switch( m_nAxis ) {
			default:
			case USE_RIGHT_AXIS:
				pPt->UnitDir = ppMtxPalette[ m_anBoneIndices[i] ]->m_vRight.v3;
				break;
			case USE_UP_AXIS:
				pPt->UnitDir = ppMtxPalette[ m_anBoneIndices[i] ]->m_vUp.v3;
				break;
			case USE_FRONT_AXIS:
				pPt->UnitDir = ppMtxPalette[ m_anBoneIndices[i] ]->m_vFront.v3;
				break;
			}
		} else {
			pPt->PosWS = m_pOrientation->m44.MultPoint( m_Offset_MS );
			switch( m_nAxis ) {
			default:
			case USE_RIGHT_AXIS:
				pPt->UnitDir = m_pOrientation->m_vRight.v3;
				break;
			case USE_UP_AXIS:
				pPt->UnitDir = m_pOrientation->m_vUp.v3;
				break;
			case USE_FRONT_AXIS:
				pPt->UnitDir = m_pOrientation->m_vFront.v3;
				break;
			}						
		}

		if( !m_bNearingEnd ) {
			pPt->fAlpha = m_fInitialAlpha;
			pPt->fFadeMultiplier = 1.0f; 
		} else {
			pPt->fAlpha = FMATH_FPOT( m_fPercentTillEnd, m_fInitialAlpha, 0.0f );
			pPt->fFadeMultiplier = FMATH_FPOT( m_fPercentTillEnd, 1.0f, 0.0f );
		}
		pPt += m_nPtsPerStream;
	}
	m_nUsedCount++;
	FASSERT( m_nUsedCount <= m_nPtsPerStream );
}

void CFXStreamerEmitter::DrawEmitter() {
	u32 i, j, nPtOffset, nIndex, nNumVerts;
	_StreamerPt_t *pPt;
	FDrawVtx_t *pVtx, *pNextVtx;

	if( m_hMyHandle != FXSTREAMER_INVALID_HANDLE ) {
		// draw (there must be at least 2 points to draw anything)
		nNumVerts = (!m_bDrawLastRemoved) ? m_nUsedCount : (m_nUsedCount + 1);
		if( nNumVerts > 1 ) {
			// grab enough verts as if they were all on the same page
			nNumVerts <<= 1;
			pVtx = fvtxpool_GetArray( nNumVerts );
			if( pVtx ) {
				// set the texture 
				fdraw_SetTexture( m_pTexInst );

				f32 fT = 0.0f;

				for( i=0; i < m_nNumStreams; i++ ) {
					nPtOffset = i * m_nPtsPerStream;
					nIndex = m_nOldestIndex;
					pNextVtx = pVtx;

					if( m_bDrawLastRemoved ) {
						if( i == 0 ) {
							pNextVtx->ColorRGBA.Set( 1.0f, m_afLastRemovedAlpha[i] );
							pNextVtx->ST.Set( 1.0f, fT );

							pNextVtx[1].ColorRGBA = pNextVtx[0].ColorRGBA;
							pNextVtx[1].ST.Set( 0.0f, fT );

							fT += 1.0f;
						}
						pNextVtx->Pos_MS = m_aLastRemovedPosWS[i];
						pNextVtx->Pos_MS += m_aLastRemovedUnitDir[i] * m_fHalfWidth_WS;
						
						pNextVtx[1].Pos_MS = pNextVtx[0].Pos_MS;
						pNextVtx[1].Pos_MS -= m_aLastRemovedUnitDir[i] * (m_fHalfWidth_WS * 2.0f);

						pNextVtx += 2;
					}


					pPt = &m_paPts[ nPtOffset + nIndex];
					for( j=0; j < m_nUsedCount; j++ ) {
						if( i == 0 ) {
							// only setup the color & st for the 1st streamer
							if( j == 0 && (m_nFlags & FLAGS_1ST_POINT_STILL_AROUND) ) {
								pNextVtx->ColorRGBA.Set( 1.0f, 0.0f );
							} else {
                                pNextVtx->ColorRGBA.Set( 1.0f, pPt->fAlpha );
							}
							pNextVtx->ST.Set( 1.0f, fT );

							pNextVtx[1].ColorRGBA = pNextVtx[0].ColorRGBA;
							pNextVtx[1].ST.Set( 0.0f, fT );

							fT += 1.0f;
						}
						pNextVtx->Pos_MS = pPt->PosWS;
						pNextVtx->Pos_MS += pPt->UnitDir * m_fHalfWidth_WS;
						
						pNextVtx[1].Pos_MS = pNextVtx[0].Pos_MS;
						pNextVtx[1].Pos_MS -= pPt->UnitDir * (m_fHalfWidth_WS * 2.0f);
										
						nIndex++;
						if( nIndex >= m_nPtsPerStream ) {
							// wrap around
							nIndex = 0;
							pPt = &m_paPts[ nPtOffset + nIndex];
						} else {
							pPt++;
						}
						pNextVtx += 2;
					}
					fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, pVtx, nNumVerts );
				}

				// return the vtxs back to the pool
				fvtxpool_ReturnArray( pVtx );
			}
		}
	}
}

void CFXStreamerEmitter::CreateMyHandle( u32 nIndex ) {
	_nKey++;
	FMATH_CLAMP( _nKey, 1, 0xFFFF );
	
	m_hMyHandle = (_nKey & 0x0000FFFF) << 16;
	m_hMyHandle |= (nIndex & 0x0000FFFF);
}

BOOL CFXStreamerEmitter::InitCommonFields( u32 nMaxStreamDepth, 
										   const CFWorldMesh *pWorldMesh,
										   f32 fInitialAlpha, 
										   CFTexInst *pTexture,
										   f32 fWidth_WS,
										   f32 fSamplesPerSec,
										   UseAxis_e nAxis ) {

	if( !nMaxStreamDepth || 
		fSamplesPerSec < 1.0f || 
		fInitialAlpha > 1.0f || fInitialAlpha <= _MIN_STREAMER_ALPHA ||
		fWidth_WS <= 0.0f ) {
		// invalid parameters, fail
		return FALSE;
	}

	m_pWorldMesh = pWorldMesh;
	m_pOrientation = (m_pWorldMesh) ? &m_pWorldMesh->m_Xfm.m_MtxF : NULL;
	m_pTexInst = pTexture;
	m_fInitialAlpha = fInitialAlpha;
	m_fHalfWidth_WS = (fWidth_WS * 0.5f);
	m_nOldestIndex = 0;
	m_nUsedCount = 0;
	m_fOOSamplesPerSec = 1.0f / fSamplesPerSec;
	m_fSecsTillNextSample = 0.0f;// start sampling right away
	m_bNearingEnd = FALSE;
	m_fPercentTillEnd = 1.0f;
	m_nPtsPerStream = nMaxStreamDepth;
	m_fFadePerSec = ( fSamplesPerSec / (m_nPtsPerStream-1) ) * (m_fInitialAlpha - _MIN_STREAMER_ALPHA);
	m_bEmitFromBones = FALSE;
	m_Offset_MS.Zero();
	m_nNumPts = 0;
	m_paPts = NULL;
	m_nNumStreams = 0;
	m_bEmitStreamer = FALSE;
	m_nAxis = nAxis;
	m_nFlags = FLAGS_1ST_POINT_STILL_AROUND;

	return TRUE;
}

//==================
// private functions

static CFXStreamerEmitter *_ConvertHandleToPtr( FXStreamerHandle_t hHandle ) {

	if( hHandle == FXSTREAMER_INVALID_HANDLE ) {
		return NULL;
	}
	u32 nIndex = (hHandle & 0x0000FFFF);
	if( nIndex >= FXSTREAMER_NUM_STREAMER_EMITTERS ) {
		return NULL;
	}
	CFXStreamerEmitter *pStreamerEmitter = &_paStreamerEmitters[nIndex];
	if( pStreamerEmitter->GetMyHandle() == hHandle ) {
		return pStreamerEmitter;
	}
	return NULL;
}


