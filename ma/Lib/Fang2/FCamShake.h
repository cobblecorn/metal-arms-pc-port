//////////////////////////////////////////////////////////////////////////////////////
// FCamShake.h - Camera shake waveform generator.
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
#ifndef _FCAMSHAKE_H_
#define _FCAMSHAKE_H_ 1

#include "fang.h"
#include "fmath.h"


class CFXfm;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFCamShake
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFCamShake {

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		_MAX_SHAKERS		= 3
	};


	typedef struct {
		f32 fMaxUnitIntensity;				// The intensity of the shake (0=shaker is inactive)
		f32 fUnitIntensity;					// The current intensity of the shake

		f32 fFullSecsRemaining;				// The time remaining at full intensity (0=decaying)
		f32 fDecaySecsRemaining;			// When fFullSecsRemaining is 0, this is the decay attenuation
	} _Shaker_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	u32 m_nActiveShakerCount;				// Number of active shakers in m_aShaker[]
	_Shaker_t m_aShaker[_MAX_SHAKERS];		// Array of shakers

	CFVec3A m_ShakeAngle;
	f32 m_fUnitIntensity;					// Overall intensity of camera shake

	static const CFVec3A m_MinShakeAngleSpeedVec;
	static const CFVec3A m_MaxShakeAngleSpeedVec;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CFCamShake();

	void Reset( void );
	FINLINE void Work( void ) { if( m_nActiveShakerCount ) { _Work(); } }

	void AddShaker( f32 fUnitIntensity, f32 fDuration );
	FINLINE void AddShakeToXfm( CFXfm *pXfm ) { if( m_fUnitIntensity > 0.0f ) { _AddShakeToXfm( pXfm ); } }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _Work( void );
	void _InitNewShaker( u32 nSlotIndex, f32 fUnitIntensity, f32 fDuration );
	void _AddShakeToXfm( CFXfm *pXfm );


	FCLASS_STACKMEM_ALIGN( CFCamShake );
} FCLASS_ALIGN_SUFFIX;




#endif

