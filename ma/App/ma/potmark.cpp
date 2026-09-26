//////////////////////////////////////////////////////////////////////////////////////
// potmark.cpp - 
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
// 08/29/01 Ranck		Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#include "fang.h"
#include "potmark.h"
#include "fdraw.h"
#include "fresload.h"
#include "floop.h"
#include "ftex.h"
#include "frenderer.h"

//====================
// private definitions

typedef struct {
	f32 fUnitGlow;
	f32 fUnitOpacity;
} _Potmark_t;

#define _POTMARK_DIM				0.375f

//=================
// public variables

//==================
// private variables

static u32 _anActivePotmarkCount[POTMARKTYPE_COUNT];
static CFTexInst _aPotmarkTexInst[POTMARKTYPE_COUNT];
static CFVec3 _aPotmarkVtx[3];
static cchar *_apszTexName[POTMARKTYPE_COUNT] =				{ "tfp1spark01", "tfdmlaser03", "tfd1rivet01", "TFDMscatr03", "TFDMspew_01"};
static f32 _afCoolSpeed[POTMARKTYPE_COUNT] =				{  0.5f,			0.5f,			0.5f,			0.0f,		0.0f		};
static f32 _afFadeSpeed[POTMARKTYPE_COUNT] =				{  0.25f,			0.25f,			0.25f,			0.15f,		0.1f		};
static FDrawVtx_t *_apPotmarkVtxBuf[POTMARKTYPE_COUNT] =	{ NULL,				NULL,			NULL,			NULL,		NULL		};
static _Potmark_t *_apPotmarkBuf[POTMARKTYPE_COUNT] =		{ NULL,				NULL,			NULL,			NULL,		NULL		};
static u32 _nVertsPerPotmark[POTMARKTYPE_COUNT] =			{	3,				3,				6,				3,			3			};
static u32 _anMaxPotmarks[POTMARKTYPE_COUNT] =				{	400,			400,			100,			300,		400			};

//===================
// private prototypes

//=================
// public functions

BOOL potmark_InitSystem( void ) {

#if !FANG_PRODUCTION_BUILD
	u32 _uMem = fres_GetFreeBytes();
#endif

	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	u32 uCurBufIdx;
	for(uCurBufIdx = 0; uCurBufIdx < POTMARKTYPE_COUNT; ++uCurBufIdx)
	{
		_apPotmarkVtxBuf[uCurBufIdx] = NULL;
		_apPotmarkBuf[uCurBufIdx] = NULL;
		_anActivePotmarkCount[uCurBufIdx] = 0;

		// JUSTIN: These need to go into a loop.
		_apPotmarkVtxBuf[uCurBufIdx] = (FDrawVtx_t *)fres_Alloc( _anMaxPotmarks[uCurBufIdx] * _nVertsPerPotmark[uCurBufIdx] * sizeof(FDrawVtx_t) );
		if( _apPotmarkVtxBuf[uCurBufIdx] == NULL ) {
			DEVPRINTF( "potmark_InitSystem(): Could not allocate memory for potmarks.\n" );
			goto _ExitInitWithError;
		}

		_apPotmarkBuf[uCurBufIdx] = (_Potmark_t *)fres_Alloc( _anMaxPotmarks[uCurBufIdx] * sizeof(_Potmark_t) );
		if( _apPotmarkBuf[uCurBufIdx] == NULL ) {
			DEVPRINTF( "potmark_InitSystem(): Could not allocate memory for potmarks.\n" );
			goto _ExitInitWithError;
		}

		_aPotmarkTexInst[uCurBufIdx].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _apszTexName[uCurBufIdx] ) );
		if( _aPotmarkTexInst[uCurBufIdx].GetTexDef() == NULL ) {
			DEVPRINTF( "potmark_InitSystem(): Could not load potmark texture.\n" );
			goto _ExitInitWithError;
		}

		_aPotmarkTexInst[uCurBufIdx].SetFlags( CFTexInst::FLAG_NONE );
	}

	_aPotmarkVtx[0].Set( 0.0f,  _POTMARK_DIM, 0.0f );
	_aPotmarkVtx[1].Set(  _POTMARK_DIM,  -0.5f*_POTMARK_DIM, 0.0f );
	_aPotmarkVtx[2].Set( -_POTMARK_DIM, -0.5f*_POTMARK_DIM, 0.0f );
	

	for(uCurBufIdx = 0; uCurBufIdx < POTMARKTYPE_COUNT; ++uCurBufIdx)
	{
		// JUSTIN: Double check this.
		//////////////////////////////////////////////////////////////////////
		// Initialize the color and TCs of our vertex buffer once.
		for( i=0; i<(_anMaxPotmarks[uCurBufIdx]*3); i+=3 ) {
			_apPotmarkVtxBuf[uCurBufIdx][i].ColorRGBA.OpaqueWhite();
			_apPotmarkVtxBuf[uCurBufIdx][i+1].ColorRGBA.OpaqueWhite();
			_apPotmarkVtxBuf[uCurBufIdx][i+2].ColorRGBA.OpaqueWhite();
			
			_apPotmarkVtxBuf[uCurBufIdx][i].ST.Set( 0.5f, -0.25f );
			_apPotmarkVtxBuf[uCurBufIdx][i+1].ST.Set( 1.25f, 1.0f );
			_apPotmarkVtxBuf[uCurBufIdx][i+2].ST.Set( -0.25f, 1.0f );		
		}
		//
		//////////////////////////////////////////////////////////////////////
	}


#if !FANG_PRODUCTION_BUILD
	DEVPRINTF( "Potmark system used %d bytes of memory\n", _uMem -  fres_GetFreeBytes() );
#endif

	return TRUE;

	// Error:
_ExitInitWithError:
	potmark_UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void potmark_UninitSystem( void ) {
	_apPotmarkBuf[0] = NULL;
	_apPotmarkBuf[1] = NULL;
	_apPotmarkVtxBuf[0] = NULL;
	_apPotmarkVtxBuf[1] = NULL;
}

void potmark_KillAll( void ) {
	_anActivePotmarkCount[0] = 0;	
	_anActivePotmarkCount[1] = 0;	
}

void potmark_NewPotmark( const CFVec3A *pCenterPos_WS, const CFVec3A *pUnitNormal_WS, f32 fSize/* = 0.375f*/, PotMarkType_e eType/* = POTMARKTYPE_LASER1*/ ) {
	f32 fDot;
	const CFVec3A *pOtherAxis;
	CFMtx43 Mtx43;
	_Potmark_t *pPotmark;
	FDrawVtx_t *pVtx;

	if( _anActivePotmarkCount[eType] >= _anMaxPotmarks[eType] ) {
		return;
	}

	pPotmark = _apPotmarkBuf[eType] + _anActivePotmarkCount[eType];

	pOtherAxis = &CFVec3A::m_UnitAxisY;
	fDot = pUnitNormal_WS->Dot( *pOtherAxis );
	if( fDot>=0.9f || fDot<=-0.9f ) {
		pOtherAxis = &CFVec3A::m_UnitAxisX;
	}

	Mtx43.m_vRight = pOtherAxis->v3.Cross( pUnitNormal_WS->v3 );
	Mtx43.m_vUp = pUnitNormal_WS->v3.Cross( Mtx43.m_vRight );
	Mtx43.m_vFront = pUnitNormal_WS->v3;
	Mtx43.m_vPos = pCenterPos_WS->v3 + pUnitNormal_WS->v3 * 0.05f;

	pPotmark->fUnitGlow = 1.0f;
	pPotmark->fUnitOpacity = 1.0f;

/*	pVtx[0].Pos_MS = Mtx43.MultPoint( _aPotmarkVtx[0] );
	pVtx[1].Pos_MS = Mtx43.MultPoint( _aPotmarkVtx[1] );
	pVtx[2].Pos_MS = Mtx43.MultPoint( _aPotmarkVtx[2] );*/

	if ( eType == POTMARKTYPE_RIVET_EUK2 )
	{
		f32 fXAdjust = (f32)fmath_RandomChoice( 2 ) * 0.5f;
		f32 fYAdjust = (f32)fmath_RandomChoice( 2 ) * 0.5f;
		
		pVtx = _apPotmarkVtxBuf[eType] + (6 * _anActivePotmarkCount[eType]);
		pVtx[0].Pos_MS.Set( -fSize, fSize, 0.0f );
		pVtx[0].Pos_MS = Mtx43.MultPoint( pVtx[0].Pos_MS );
		pVtx[0].ST.Set( fXAdjust, fYAdjust );
		pVtx[0].ColorRGBA.OpaqueWhite();

		pVtx[1].Pos_MS.Set( fSize, fSize, 0.0f);
		pVtx[1].Pos_MS = Mtx43.MultPoint( pVtx[1].Pos_MS );
		pVtx[1].ST.Set( fXAdjust + 0.5f, fYAdjust );
		pVtx[1].ColorRGBA.OpaqueWhite();

		pVtx[2].Pos_MS.Set( -fSize, -fSize, 0.0f);
		pVtx[2].Pos_MS = Mtx43.MultPoint( pVtx[2].Pos_MS );
		pVtx[2].ST.Set( fXAdjust, fYAdjust + 0.5f );
		pVtx[2].ColorRGBA.OpaqueWhite();

		pVtx[3] = pVtx[2];
		
		pVtx[4] = pVtx[1];

		pVtx[5].Pos_MS.Set( fSize, -fSize, 0.0f);
		pVtx[5].Pos_MS = Mtx43.MultPoint( pVtx[5].Pos_MS );
		pVtx[5].ST.Set( fXAdjust + 0.5f, fYAdjust + 0.5f );
		pVtx[5].ColorRGBA.OpaqueWhite();
	}
	else
	{
		pVtx = _apPotmarkVtxBuf[eType] + (3 * _anActivePotmarkCount[eType]);
		pVtx[0].Pos_MS.Set( 0.0f, fSize, 0.0f );
		pVtx[0].Pos_MS = Mtx43.MultPoint( pVtx[0].Pos_MS );

		pVtx[1].Pos_MS.Set( fSize, -0.5f * fSize, 0.0f);
		pVtx[1].Pos_MS = Mtx43.MultPoint( pVtx[1].Pos_MS );

		pVtx[2].Pos_MS.Set( -fSize, -0.5f * fSize, 0.0f);
		pVtx[2].Pos_MS = Mtx43.MultPoint( pVtx[2].Pos_MS );

		pVtx[0].ColorRGBA.OpaqueWhite();
		pVtx[1].ColorRGBA.OpaqueWhite();
		pVtx[2].ColorRGBA.OpaqueWhite();
	}

	_anActivePotmarkCount[eType]++;
}

// Assumes camera xfm is set up with an identity model matrix.
void potmark_Draw( void ) {
	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Depth_SetBiasLevel( 1 );
	fdraw_SetCullDir( FDRAW_CULLDIR_CW );

	u32 uCurBufIdx;
	for(uCurBufIdx = 0; uCurBufIdx < POTMARKTYPE_COUNT; ++uCurBufIdx)
	{
		if( _anActivePotmarkCount[uCurBufIdx] ) {

			// JUSTIN: Need someway to cache this for the different textures.
			fdraw_SetTexture( &_aPotmarkTexInst[uCurBufIdx] );

			if ( uCurBufIdx == POTMARKTYPE_RIVET_EUK2 )
			{
				fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _apPotmarkVtxBuf[uCurBufIdx], _anActivePotmarkCount[uCurBufIdx]*6 );
			}
			else
			{
				fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _apPotmarkVtxBuf[uCurBufIdx], _anActivePotmarkCount[uCurBufIdx]*3 );
			}
		}
	}

	frenderer_Pop();
}

void potmark_Work( void ) {
	_Potmark_t *pPotmark;
	FDrawVtx_t *pVtx, *pLastVtx;
	u32 i;

	u32 uCurBufIdx;
	for(uCurBufIdx = 0; uCurBufIdx < POTMARKTYPE_COUNT; ++uCurBufIdx)
	{
		for( i=0; i<_anActivePotmarkCount[uCurBufIdx]; i++ ) {
			pPotmark = _apPotmarkBuf[uCurBufIdx] + i;
			
			if ( uCurBufIdx == POTMARKTYPE_RIVET_EUK2 )
			{
				pVtx = _apPotmarkVtxBuf[uCurBufIdx] + i * 6;
				
				if ( pPotmark->fUnitGlow ) 
				{
					// Glowing hot...
					pPotmark->fUnitGlow -= (_afCoolSpeed[uCurBufIdx] / 3.0f) * FLoop_fPreviousLoopSecs;

					if ( pPotmark->fUnitGlow <= 0.0f ) 
					{
						pPotmark->fUnitGlow = 0.0f;
					}

					f32 fRed = 1.f;
					f32 fGreen = 1.3f - (pPotmark->fUnitGlow * pPotmark->fUnitGlow);
					f32 fBlue = 1.2f - pPotmark->fUnitGlow;
					FMATH_CLAMP( fGreen, 0.f, 1.f );
					FMATH_CLAMP( fBlue, 0.f, 1.f );
					pVtx[0].ColorRGBA.ColorRGB.Set( fRed, fGreen, fBlue );
					pVtx[1].ColorRGBA.ColorRGB = pVtx[0].ColorRGBA.ColorRGB;
					pVtx[2].ColorRGBA.ColorRGB = pVtx[0].ColorRGBA.ColorRGB;
					pVtx[3].ColorRGBA.ColorRGB = pVtx[0].ColorRGBA.ColorRGB;
					pVtx[4].ColorRGBA.ColorRGB = pVtx[0].ColorRGBA.ColorRGB;
					pVtx[5].ColorRGBA.ColorRGB = pVtx[0].ColorRGBA.ColorRGB;
				}
				else
				{
					// Cooled...
					pPotmark->fUnitOpacity -= _afFadeSpeed[uCurBufIdx] * FLoop_fPreviousLoopSecs;

					if ( pPotmark->fUnitOpacity > 0.0f ) 
					{
						pVtx[0].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
						pVtx[1].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
						pVtx[2].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
						pVtx[3].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
						pVtx[4].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
						pVtx[5].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
					} 
					else 
					{
						if ( --_anActivePotmarkCount[uCurBufIdx] == i ) 
						{
							break;
						}

						*pPotmark = _apPotmarkBuf[uCurBufIdx][_anActivePotmarkCount[uCurBufIdx]];

						pLastVtx = &_apPotmarkVtxBuf[uCurBufIdx][_anActivePotmarkCount[uCurBufIdx]*6];

						pVtx[0].Pos_MS = pLastVtx[0].Pos_MS;
						pVtx[1].Pos_MS = pLastVtx[1].Pos_MS;
						pVtx[2].Pos_MS = pLastVtx[2].Pos_MS;
						pVtx[3].Pos_MS = pLastVtx[3].Pos_MS;
						pVtx[4].Pos_MS = pLastVtx[4].Pos_MS;
						pVtx[5].Pos_MS = pLastVtx[5].Pos_MS;

						pVtx[0].ColorRGBA = pLastVtx[0].ColorRGBA;
						pVtx[1].ColorRGBA = pLastVtx[1].ColorRGBA;
						pVtx[2].ColorRGBA = pLastVtx[2].ColorRGBA;
						pVtx[3].ColorRGBA = pLastVtx[3].ColorRGBA;
						pVtx[4].ColorRGBA = pLastVtx[4].ColorRGBA;
						pVtx[5].ColorRGBA = pLastVtx[5].ColorRGBA;

						pVtx[0].ST = pLastVtx[0].ST;
						pVtx[1].ST = pLastVtx[1].ST;
						pVtx[2].ST = pLastVtx[2].ST;
						pVtx[3].ST = pLastVtx[3].ST;
						pVtx[4].ST = pLastVtx[4].ST;
						pVtx[5].ST = pLastVtx[5].ST;

						i--;
					}
				}
			}
			else
			{
				pVtx = _apPotmarkVtxBuf[uCurBufIdx] + i*3;
				
				if( pPotmark->fUnitGlow != 0.0f && _afCoolSpeed[uCurBufIdx] != 0.0f) {
					// Glowing hot...
					pPotmark->fUnitGlow -= _afCoolSpeed[uCurBufIdx] * FLoop_fPreviousLoopSecs;

					if( pPotmark->fUnitGlow <= 0.0f ) {
						pPotmark->fUnitGlow = 0.0f;
					}

					pVtx[0].ColorRGBA.ColorRGB.Set( pPotmark->fUnitGlow );
					pVtx[1].ColorRGBA.ColorRGB.Set( pPotmark->fUnitGlow );
					pVtx[2].ColorRGBA.ColorRGB.Set( pPotmark->fUnitGlow );
				} else {
					// Cooled...
					pPotmark->fUnitOpacity -= _afFadeSpeed[uCurBufIdx] * FLoop_fPreviousLoopSecs;

					if( pPotmark->fUnitOpacity > 0.0f ) {
						pVtx[0].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
						pVtx[1].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
						pVtx[2].ColorRGBA.fAlpha = pPotmark->fUnitOpacity;
					} else {
						if( --_anActivePotmarkCount[uCurBufIdx] == i ) {
							break;
						}

						*pPotmark = _apPotmarkBuf[uCurBufIdx][_anActivePotmarkCount[uCurBufIdx]];

						pLastVtx = &_apPotmarkVtxBuf[uCurBufIdx][_anActivePotmarkCount[uCurBufIdx]*3];

						pVtx[0].Pos_MS = pLastVtx[0].Pos_MS;
						pVtx[1].Pos_MS = pLastVtx[1].Pos_MS;
						pVtx[2].Pos_MS = pLastVtx[2].Pos_MS;

						pVtx[0].ColorRGBA = pLastVtx[0].ColorRGBA;
						pVtx[1].ColorRGBA = pLastVtx[1].ColorRGBA;
						pVtx[2].ColorRGBA = pLastVtx[2].ColorRGBA;

						i--;
					}
				}
			}
		}
	}
}

//==================
// private functions



#endif