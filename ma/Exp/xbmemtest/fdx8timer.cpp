//////////////////////////////////////////////////////////////////////////////////////
// fdx8timer.cpp - Fang timer module (DX8 version).
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/28/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "ftimer.h"

float FTimer_fClockFrequency;


static inline unsigned int _RDTSC32( void );
static inline unsigned __int64 _RDTSC64( void );



bool ftimer_ModuleStartup( void ) {
	FTimer_fClockFrequency = 733333333.333333333333333333333f;
	CFTimer::InitFrequencyAndPeriod( (unsigned int)FTimer_fClockFrequency );

	return 1;
}



//-------------------------------------------------------------------------------------------------------------------
// CFTimer:
//-------------------------------------------------------------------------------------------------------------------

unsigned int CFTimer::m_nTimerFrequency = 1;
float CFTimer::m_fTimerPeriod = 1.0f;


void CFTimer::Reset( void ) {
	m_nTimerTicks = _RDTSC32();
}

float CFTimer::SampleSeconds( bool bResetTimer ) {
	unsigned int nSampleLow, nDeltaLow;

	nSampleLow = _RDTSC32();
	nDeltaLow = nSampleLow - m_nTimerTicks;

	if( bResetTimer ) {
		m_nTimerTicks = nSampleLow;
	}

	return ( ((float)nDeltaLow * m_fTimerPeriod) );
}

unsigned int CFTimer::SampleTicks( bool bResetTimer ) {
	unsigned int nSampleLow, nDeltaLow;

	nSampleLow = _RDTSC32();
	nDeltaLow = nSampleLow - m_nTimerTicks;

	if( bResetTimer ) {
		m_nTimerTicks = nSampleLow;
	}

	return nDeltaLow;
}

void CFTimer::InitFrequencyAndPeriod( unsigned int nFrequency ) {
	m_nTimerFrequency = nFrequency;
	m_fTimerPeriod = 1.0f / (float)nFrequency;
}


// Executes the CPU RDTSC instruction and returns the 32-bit low result.
static inline unsigned int _RDTSC32( void ) {
	unsigned int nLow;

	__asm {
		push eax
		push edx

		rdtsc
		mov nLow, eax

		pop edx
		pop eax
	}

	return nLow;
}

// Executes the CPU RDTSC instruction and returns the 64-bit result.
static inline unsigned __int64 _RDTSC64( void ) {
	unsigned int nLow, nHigh;

	__asm {
		push eax
		push edx

		rdtsc
		mov nLow, eax
		mov nHigh, edx

		pop edx
		pop eax
	}

	return (unsigned __int64)nLow + ( ((unsigned __int64)(nHigh)) << 32 );
}

