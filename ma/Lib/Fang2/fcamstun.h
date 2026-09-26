//////////////////////////////////////////////////////////////////////////////////////
// fcamstun.h - Camera stun generator.
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
#ifndef _FCAMSTUN_H_
#define _FCAMSTUN_H_ 1

#include "fang.h"
#include "fmath.h"

class CFXfm;

FCLASS_ALIGN_PREFIX class CFCamStun {

public:
	CFCamStun();

	void Reset( void );
	// If fDuration is -1.0f, the stun will be disabled
	void EnableStun( const f32 &fDuration );

	FINLINE void Work( void )					{ if( m_bEnabled ) { _Work(); } }
	FINLINE void AddStunToXfm( CFXfm *pXfm )	{ if( m_bEnabled ) { _AddStunToXfm( pXfm ); } }

private:
	BOOL m_bEnabled;
	f32 m_fDuration;
	f32 m_fAmp;
	f32 m_fAmpRate;
	f32 m_fAngle;
	f32 m_fAngleRate;

	void _Work( void );
	void _AddStunToXfm( CFXfm *pXfm );

	FCLASS_STACKMEM_ALIGN( CFCamStun );
} FCLASS_ALIGN_SUFFIX;




#endif

