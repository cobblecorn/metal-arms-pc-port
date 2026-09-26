//////////////////////////////////////////////////////////////////////////////////////
// fcamstun.cpp - Camera stun generator.
//
// Author: Nathan Miller
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
// 12/23/02 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fcamstun.h"
#include "floop.h"
#include "fmath.h"
#include "fxfm.h"

#define _AMP_RATE_UP	( 2.0f )					// How fast to grow the amplitude up
#define _AMP_ANGLE_MAX  ( FMATH_HALF_PI * 0.15f )	// Largest angle to use
#define _ROT_RATE_UP	( FMATH_HALF_PI )			// How fast to rotate when amping up
#define _ROT_RATE_DOWN	( FMATH_HALF_PI )			// How fast to rotate when amping down

CFCamStun::CFCamStun() {
	Reset();
}

void CFCamStun::Reset( void ) {
	m_bEnabled = FALSE;
	
	m_fDuration = 0.0f;
	m_fAmp = 0.0f;
	m_fAmpRate = _AMP_RATE_UP;
	m_fAngle = 0.0f;
	m_fAngleRate = _ROT_RATE_UP;
}

// If we get a negative number, shut off the stun
void CFCamStun::EnableStun( const f32 &fDuration ) {
	Reset();

	if( fDuration > 0.0f ) {
		m_fDuration = fDuration;
		m_bEnabled = TRUE;
	}
}

void CFCamStun::_Work( void ) {
	FASSERT( m_bEnabled );

	m_fDuration -= FLoop_fPreviousLoopSecs;

	if( m_fDuration <= 0.0f ) {
		m_bEnabled = FALSE;
		return;
	}

	m_fAmp += ( m_fAmpRate * FLoop_fPreviousLoopSecs );

	// We reached our higest amplitude, fall off now
	if( m_fAmp > 1.0f ) {
		m_fAmpRate = -fmath_Inv( m_fDuration ); // We want to fall off over the time we have left
		m_fAngleRate = _ROT_RATE_DOWN;
		m_fAmp = 1.0f;
	}

	FMATH_CLAMP( m_fAmp, 0.0f, 1.0f );

	m_fAngle += ( m_fAngleRate * FLoop_fPreviousLoopSecs );
}

void CFCamStun::_AddStunToXfm( CFXfm *pXfm ) {
	FASSERT( m_bEnabled );
	FASSERT( pXfm );

	CFXfm StunXfm;
	CFVec3A Dir = CFVec3A::m_UnitAxisZ;

	Dir.RotateX( ( m_fAmp * _AMP_ANGLE_MAX ) );
	Dir.RotateZ( m_fAngle );

	CFMtx43A::m_Temp.m_vPos.Zero();
	CFMtx43A::m_Temp.m_vFront = Dir;
	CFMtx43A::m_Temp.m_vRight.Cross( CFVec3A::m_UnitAxisY, CFMtx43A::m_Temp.m_vFront );
	CFMtx43A::m_Temp.m_vUp.Cross( CFMtx43A::m_Temp.m_vFront, CFMtx43A::m_Temp.m_vRight );

	StunXfm.BuildFromMtx( CFMtx43A::m_Temp );

	pXfm->ReceiveProductOf( StunXfm, *pXfm );
}