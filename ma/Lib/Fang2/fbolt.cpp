//////////////////////////////////////////////////////////////////////////////////////
// fbolt.cpp - Draws lightning bolts.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 05/01/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fbolt.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fxfm.h"
#include "fvtxpool.h"



CFVec3A CFBolt::m_UnitDir_WS;
CFVec3A CFBolt::m_RightUnitVec_WS;
CFVec3A CFBolt::m_CurveUnitVec_WS;
CFVec3A CFBolt::m_BasePos_WS;
CFVec3A CFBolt::m_BaseDeltaVec_WS;
CFVec3A CFBolt::m_CurvedBasePos_WS;
CFVec3A CFBolt::m_DisplacementVec_WS;
CFVec3A CFBolt::m_VtxPos_WS;
CFQuatA CFBolt::m_RotQuat;
u32 CFBolt::m_nJointCount;
u32 CFBolt::m_nJointCountMinusOne;
f32 CFBolt::m_fCurveStep;
f32 CFBolt::m_fOOUnitDistPinch;
FDrawVtx_t *CFBolt::m_pVtxArray;
CFBolt::CFBoltData *CFBolt::m_pBoltData;



// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void CFBolt::Draw( CFBoltData *pBoltData ) {
	f32 fBoltDist, fMag2;
	u32 nJointCount;
	CFVec3A StartToCam_WS;

	m_pVtxArray = NULL;

	m_UnitDir_WS.Sub( pBoltData->m_EndPos_WS, pBoltData->m_StartPos_WS );
	fBoltDist = m_UnitDir_WS.Mag();

	if( fBoltDist < 0.0001f ) {
		goto _Exit;
	}

	m_UnitDir_WS.Div( fBoltDist );

	nJointCount = (u32)fmath_Div( fBoltDist, pBoltData->m_fDistBetweenJoints ) + 1;

	if( nJointCount < 2 ) {
		goto _Exit;
	}

	FMATH_CLAMPMAX( nJointCount, pBoltData->m_nMaxJointCount );

	m_pVtxArray = fvtxpool_GetArray( nJointCount << 1 );
	if( m_pVtxArray == NULL ) {
		// Couldn't get a pool of FDrawVtx_t's.
		// We'll try again next frame...
		goto _Exit;
	}

	StartToCam_WS.Sub( FXfm_pModelView->m_MtxR.m_vPos, pBoltData->m_StartPos_WS );
	m_RightUnitVec_WS.Cross( StartToCam_WS, m_UnitDir_WS );

	fMag2 = m_RightUnitVec_WS.MagSq();
	if( fMag2 < 0.0001f ) {
		goto _Exit;
	}

	m_RightUnitVec_WS.Mul( fmath_InvSqrt( fMag2 ) );

	if( fmath_Abs( m_UnitDir_WS.y ) < 0.999f ) {
		m_CurveUnitVec_WS.CrossYWithVec( m_UnitDir_WS ).Unitize();
	} else {
		// Direction is straight up or straight down...
		m_CurveUnitVec_WS = CFVec3A::m_UnitAxisX;
	}

	m_pBoltData = pBoltData;
	m_nJointCount = nJointCount;
	m_nJointCountMinusOne = m_nJointCount - 1;
	m_fCurveStep = fmath_Inv( (f32)m_nJointCount );

	if( pBoltData->m_fUnitDistPinch <= 0.001f ) {
		m_fOOUnitDistPinch = 1.0f;
	} else {
		m_fOOUnitDistPinch = fmath_Inv( pBoltData->m_fUnitDistPinch );
	}

	frenderer_SetDefaultState();

	fdraw_SetTexture( &pBoltData->m_TexInst );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );
	fdraw_Depth_EnableWriting( FALSE );

	for( u32 i=0; i<pBoltData->m_nBoltCount; ++i ) {
		_DrawOneBolt();
	}

_Exit:
	fvtxpool_ReturnArray( m_pVtxArray );
	m_pVtxArray = NULL;
}


void CFBolt::_DrawOneBolt( void ) {
	f32 fDisplacement, fUnitCurve, fCurveDisplacement, fUnit, fUnitPinch, fBoltThickness, fPinchedBoltThickness;
	u32 i;
	FDrawVtx_t *pDrawVtx;

	m_RotQuat.BuildQuat( m_UnitDir_WS, fmath_RandomFloatRange( 0.0f, FMATH_2PI ) );
	m_RotQuat.MulPoint( m_CurveUnitVec_WS );

	m_BasePos_WS = m_pBoltData->m_StartPos_WS;
	m_BaseDeltaVec_WS.Mul( m_UnitDir_WS, m_pBoltData->m_fDistBetweenJoints );

	pDrawVtx = m_pVtxArray;

	fUnitCurve = 0.0f;
	fCurveDisplacement = fmath_RandomFloatRange( 0.0f, m_pBoltData->m_fTrumpetRadius );

	fBoltThickness = fmath_RandomFloatRange( m_pBoltData->m_fMinBoltThickness, m_pBoltData->m_fMaxBoltThickness );

	for( i=0; i<m_nJointCount; ++i ) {
		fUnit = fUnitCurve * fUnitCurve;

		if( fUnit >= m_pBoltData->m_fUnitDistPinch ) {
			fUnitPinch = 1.0f;
		} else {
			fUnitPinch = fUnit * m_fOOUnitDistPinch;
		}

		fPinchedBoltThickness = fBoltThickness * fUnitPinch;

		m_CurvedBasePos_WS.Mul( m_CurveUnitVec_WS, fUnit * fCurveDisplacement ).Add( m_BasePos_WS );

		fUnit = FMATH_FPOT( fUnit, m_pBoltData->m_fMinBoltDisplacement, m_pBoltData->m_fMaxBoltDisplacement );
		fDisplacement = fmath_RandomFloatRange( -fUnit, fUnit );

		m_DisplacementVec_WS.Mul( m_RightUnitVec_WS, fDisplacement + fPinchedBoltThickness );
		m_VtxPos_WS.Add( m_CurvedBasePos_WS, m_DisplacementVec_WS );

		pDrawVtx->Pos_MS = m_VtxPos_WS.v3;
		pDrawVtx->ST.Set( 0.0f, 0.0f );
		pDrawVtx->ColorRGBA = m_pBoltData->m_Color;

		++pDrawVtx;

		m_DisplacementVec_WS.Mul( m_RightUnitVec_WS, fDisplacement - fPinchedBoltThickness );
		m_VtxPos_WS.Add( m_CurvedBasePos_WS, m_DisplacementVec_WS );

		pDrawVtx->Pos_MS = m_VtxPos_WS.v3;
		pDrawVtx->ST.Set( 0.0f, 1.0f );
		pDrawVtx->ColorRGBA = m_pBoltData->m_Color;

		if( i==0 || i==m_nJointCountMinusOne ) {
			pDrawVtx->ColorRGBA.fAlpha = 0.0f;
			(pDrawVtx-1)->ColorRGBA.fAlpha = 0.0f;
		}

		++pDrawVtx;

		m_BasePos_WS.Add( m_BaseDeltaVec_WS );

		fUnitCurve += m_fCurveStep;
	}

	fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, m_pVtxArray, m_nJointCount << 1 );
}
