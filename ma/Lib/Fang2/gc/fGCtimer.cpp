//////////////////////////////////////////////////////////////////////////////////////
// fGCtimer.cpp - Fang timer module (GaneCube version).
//
// Author: John Lafleur
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
// 02/18/02	Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "ftimer.h"
#include "fvid.h"

#include <os.h>


u32 FTimer_nClockFrequency;
f32 FTimer_fClockPeriod;


static BOOL _bModuleInitialized;

static u64 _nClockTicks64;


//
//
//
BOOL ftimer_ModuleStartup( void )
{
	FASSERT( !_bModuleInitialized );

	// Store frequency & period...
	FTimer_nClockFrequency = (u32)OSSecondsToTicks( 1 );
	FTimer_fClockPeriod = 1.0f / (f32)FTimer_nClockFrequency;

	_bModuleInitialized = TRUE;

	CFTimer::InitFrequencyAndPeriod( (u32)FTimer_nClockFrequency );
	ftimer_Clock_Reset();

	return TRUE;
}


//
//
//
void ftimer_ModuleShutdown( void )
{
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}


//
//
//
void ftimer_Clock_Reset( u64 uResetVal )
{
	FASSERT( _bModuleInitialized );

	_nClockTicks64 = OSGetTime() - uResetVal;
}


//
//
//
f32 ftimer_Clock_GetSeconds( void )
{
	u64 nDelta;

	FASSERT( _bModuleInitialized );

	nDelta = ftimer_Clock_GetTicks();

	return ( (f32)nDelta * FTimer_fClockPeriod );
}


//
//
//
u64 ftimer_Clock_GetTicks( void )
{
	FASSERT( _bModuleInitialized );

	return OSGetTime() - _nClockTicks64;
}



//-------------------------------------------------------------------------------------------------------------------
// CFTimer:
//
// This timer is only updated every 12 CPU cycles.  There are more precise timers,
// but they are hardware based and could not exist in many instances.  To use the
// Gekko hardware based timers, use the following calls:
//		PPCMtpmc1(0); PPCMtmmcr0( MMCR0_PMC1_CYCLE ); // Resets the counter and tells the CPU to count cycles
//		PPCMtpmc1(0); // Resets the counter
//		x = PPCMfpmc1(); // Samples the current cycle count
//-------------------------------------------------------------------------------------------------------------------

u32 CFTimer::m_nTimerFrequency = 1;
f32 CFTimer::m_fTimerPeriod = 1.0f;

#define	_CPU_CLOCK_ADJUST	(1.0 / OS_TIMER_CLOCK)

//
//
//
void CFTimer::Reset( void ) 
{
	FASSERT( _bModuleInitialized );
	m_nTimerTicks = OSGetTime();
}


//
//
//
f32 CFTimer::SampleSeconds( BOOL bResetTimer ) 
{
	FASSERT( _bModuleInitialized );

	u64 nCurrentTime = OSGetTime();
	f32 fResult =  (f32)(nCurrentTime - m_nTimerTicks) * _CPU_CLOCK_ADJUST;
	
	if( bResetTimer ) 
	{
		m_nTimerTicks = nCurrentTime;
	}

	return fResult;
}


//
//
//
u32 CFTimer::SampleTicks( BOOL bResetTimer ) 
{
	FASSERT( _bModuleInitialized );

	u64 nCurrentTime = OSGetTime();
	u32 nResult = (u32)(nCurrentTime - m_nTimerTicks);
	
	if( bResetTimer ) 
	{
		m_nTimerTicks = nCurrentTime;
	}

	return nResult;
}


//
//
//
void CFTimer::InitFrequencyAndPeriod( u32 nFrequency ) 
{
	FASSERT( _bModuleInitialized );
	m_nTimerFrequency = nFrequency;
	m_fTimerPeriod = 1.0f / (f32)nFrequency;
}


