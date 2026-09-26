//////////////////////////////////////////////////////////////////////////////////////
// FCamShake.cpp - Camera shake waveform generator.
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
// 11/06/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fcamshake.h"
#include "floop.h"
#include "fmath.h"
#include "fxfm.h"


#define _SHAKER_DECAY_SECS	0.25f

#define _MIN_SHAKE_ANGLE_X	(FMATH_2PI * 8.0f)
#define _MIN_SHAKE_ANGLE_Y	(FMATH_2PI * 4.0f)
#define _MIN_SHAKE_ANGLE_Z	(FMATH_2PI * 3.0f)

#define _MAX_SHAKE_ANGLE_X	(FMATH_2PI * 12.0f)
#define _MAX_SHAKE_ANGLE_Y	(FMATH_2PI * 7.0f)
#define _MAX_SHAKE_ANGLE_Z	(FMATH_2PI * 5.0f)

#define _MAX_SHAKE_AMP_X	0.1f
#define _MAX_SHAKE_AMP_Y	0.1f
#define _MAX_SHAKE_AMP_Z	0.05f




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFCamShake
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

const CFVec3A CFCamShake::m_MinShakeAngleSpeedVec( _MIN_SHAKE_ANGLE_X, _MIN_SHAKE_ANGLE_Y, _MIN_SHAKE_ANGLE_Z );
const CFVec3A CFCamShake::m_MaxShakeAngleSpeedVec( _MAX_SHAKE_ANGLE_X, _MAX_SHAKE_ANGLE_Y, _MAX_SHAKE_ANGLE_Z );


CFCamShake::CFCamShake() {
	Reset();
}


void CFCamShake::Reset( void ) {
	u32 i;

	m_nActiveShakerCount = FALSE;
	m_ShakeAngle = 0.0f;
	m_fUnitIntensity = 0.0f;

	for( i=0; i<_MAX_SHAKERS; ++i ) {
		m_aShaker[i].fMaxUnitIntensity = 0.0f;
		m_aShaker[i].fUnitIntensity = 0.0f;
		m_aShaker[i].fFullSecsRemaining = 0.0f;
		m_aShaker[i].fDecaySecsRemaining = 0.0f;
	}
}


void CFCamShake::_Work( void ) {
	u32 i;

	m_fUnitIntensity = 0.0f;

	for( i=0; i<_MAX_SHAKERS; ++i ) {
		if( m_aShaker[i].fMaxUnitIntensity > 0.0f ) {
			// Shaker is active...

			if( m_aShaker[i].fFullSecsRemaining > 0.0f ) {
				// We're in full-intensity mode...

				m_aShaker[i].fFullSecsRemaining -= FLoop_fPreviousLoopSecs;

				if( m_aShaker[i].fFullSecsRemaining <= 0.0f ) {
					// Start decay mode...
					m_aShaker[i].fFullSecsRemaining = 0.0f;
					m_aShaker[i].fDecaySecsRemaining = _SHAKER_DECAY_SECS;
				}
			} else {
				// We're in decay mode...

				m_aShaker[i].fDecaySecsRemaining -= FLoop_fPreviousLoopSecs;

				if( m_aShaker[i].fDecaySecsRemaining > 0.0f ) {
					// This shaker is still decaying...
					m_aShaker[i].fUnitIntensity = m_aShaker[i].fMaxUnitIntensity * m_aShaker[i].fDecaySecsRemaining * (1.0f / _SHAKER_DECAY_SECS);
				} else {
					// This shaker is finished...
					m_aShaker[i].fDecaySecsRemaining = 0.0f;
					m_aShaker[i].fUnitIntensity = 0.0f;
					m_aShaker[i].fMaxUnitIntensity = 0.0f;
					--m_nActiveShakerCount;
				}
			}

			// Compute overall intensity of camera shake...
			if( m_aShaker[i].fUnitIntensity > m_fUnitIntensity ) {
				m_fUnitIntensity = m_aShaker[i].fUnitIntensity;
			}
		}
	}

	if( m_nActiveShakerCount ) {
		CFVec3A ShakeAngleSpeed, ShakeAngleDelta;

		ShakeAngleSpeed.Lerp( m_fUnitIntensity, m_MinShakeAngleSpeedVec, m_MaxShakeAngleSpeedVec );

		// Adjust angles...
		ShakeAngleDelta.Mul( ShakeAngleSpeed, FLoop_fPreviousLoopSecs );
		m_ShakeAngle.Add( ShakeAngleDelta );

		// Normalize all 3 angles from 0 to 2PI...
		for( i=0; i<3; ++i ) {
			while( m_ShakeAngle.a[i] > FMATH_2PI ) {
				m_ShakeAngle.a[i] -= FMATH_2PI;
			}
			while( m_ShakeAngle.a[i] < 0.0f ) {
				m_ShakeAngle.a[i] += FMATH_2PI;
			}
		}
	}
}


void CFCamShake::AddShaker( f32 fUnitIntensity, f32 fDuration ) {
	if( (fUnitIntensity == 0.0f) || (fDuration == 0.0f) ) {
		return;
	}

	u32 i;

	if( m_nActiveShakerCount < _MAX_SHAKERS ) {
		// There's room in our shaker array for a new one...

		for( i=0; i<_MAX_SHAKERS; ++i ) {
			if( m_aShaker[i].fMaxUnitIntensity == 0.0f ) {
				_InitNewShaker( i, fUnitIntensity, fDuration );
				++m_nActiveShakerCount;
				return;
			}
		}

		FASSERT_NOW;
	}

	// No more room. Replace the slot with the smallest intensity...

	f32 fSmallestUnitIntensity = 0.0f;
	s32 nSmallestSlotIndex = -1;

	for( i=0; i<_MAX_SHAKERS; ++i ) {
		FASSERT( m_aShaker[i].fMaxUnitIntensity > 0.0f );

		if( (nSmallestSlotIndex < 0) || (m_aShaker[i].fUnitIntensity < fSmallestUnitIntensity) ) {
			if( m_aShaker[i].fUnitIntensity < fUnitIntensity ) {
				nSmallestSlotIndex = i;
				fSmallestUnitIntensity = m_aShaker[i].fUnitIntensity;
			}
		}
	}

	if( nSmallestSlotIndex >= 0 ) {
		_InitNewShaker( nSmallestSlotIndex, fUnitIntensity, fDuration );
	}
}


void CFCamShake::_InitNewShaker( u32 nSlotIndex, f32 fUnitIntensity, f32 fDuration ) {
	m_aShaker[nSlotIndex].fMaxUnitIntensity = fUnitIntensity;
	m_aShaker[nSlotIndex].fUnitIntensity = fUnitIntensity;
	m_aShaker[nSlotIndex].fFullSecsRemaining = fDuration;
	m_aShaker[nSlotIndex].fDecaySecsRemaining = 0.0f;
}


void CFCamShake::_AddShakeToXfm( CFXfm *pXfm ) {
	CFXfm ShakeXfm;
	f32 fAngleX, fAngleY, fAngleZ;

	fAngleX = _MAX_SHAKE_AMP_X * fmath_Sin( m_ShakeAngle.x ) * m_fUnitIntensity;
	fAngleY = _MAX_SHAKE_AMP_Y * fmath_Sin( m_ShakeAngle.y ) * m_fUnitIntensity;
	fAngleZ = _MAX_SHAKE_AMP_Z * fmath_Sin( m_ShakeAngle.z ) * m_fUnitIntensity;

	CFMtx43A::m_Temp.SetRotationYXZ( fAngleY, fAngleX, fAngleZ );
	CFMtx43A::m_Temp.m_vPos.Zero();
	ShakeXfm.BuildFromMtx( CFMtx43A::m_Temp );

	pXfm->ReceiveProductOf( ShakeXfm, *pXfm );
}




