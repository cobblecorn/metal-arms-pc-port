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

#include "fang.h"
#include "ftimer.h"
#include "fcpu.h"
#include "fvid.h"
#include "fmath.h"


#define _USE_RDTSC_INSTEAD_OF_QPC		FALSE


#if !_USE_RDTSC_INSTEAD_OF_QPC
	#if _FANGDEF_PLATFORM_XB
		#include "xtl.h"
	#else
		#include "windows.h"
	#endif
#endif



u32 FTimer_nClockFrequency;
f32 FTimer_fClockPeriod;


static BOOL _bModuleInitialized;

#if _USE_RDTSC_INSTEAD_OF_QPC
	static u64 _ClockTicks;
#else
	static LARGE_INTEGER _nClockFreqLargeInt;
	static u64 _nBaseClockTicks64;

	static LARGE_INTEGER _nLastQPC;		// Last QueryPerformanceCounter value
	static DWORD _nLastGTC;				// Last GetTickCount value
#endif


static inline u32 _RDTSC32( void );
static inline u64 _RDTSC64( void );



//
//
//
BOOL ftimer_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	if( !FCpu_IntelInfo.CPUID.CPUID1.bTimeStampCounter ) 
	{
		// No RDTSC instruction...
		DEVPRINTF( "ftimer_ModuleStartup(): RDTSC instruction is not supported by this CPU.\n" );
		return FALSE;
	}

	if( FCpu_IntelInfo.fCyclesPerSecond_RDTSC <= 0.0f ) 
	{
		// Invalid RDTSC frequency reported...
		DEVPRINTF( "ftimer_ModuleStartup(): Invalid RDTSC frequency detected (%f).\n", FCpu_IntelInfo.fCyclesPerSecond_RDTSC );
		return FALSE;
	}

	#if _USE_RDTSC_INSTEAD_OF_QPC
		// Store clock frequency & period...
		FTimer_nClockFrequency = (u32)FCpu_IntelInfo.fCyclesPerSecond_RDTSC;
		FTimer_fClockPeriod = 1.0f / (f32)FTimer_nClockFrequency;
	#else
		// Test performance counter...
		if( !QueryPerformanceCounter( &_nClockFreqLargeInt ) ) 
		{
			// No performance counter...
			DEVPRINTF( "ftimer_ModuleStartup(): The operating system does not support a performance counter.\n" );
			return FALSE;
		}

		// Get frequency of performance counter...
		if( !QueryPerformanceFrequency( &_nClockFreqLargeInt ) ) 
		{
			// No performance counter...
			DEVPRINTF( "ftimer_ModuleStartup(): The operating system does not support a performance counter.\n" );
			return FALSE;
		}

		// Test frequency...
		if( _nClockFreqLargeInt.QuadPart > 0xffffffff ) 
		{
			// Frequency too high...
			DEVPRINTF( "ftimer_ModuleStartup(): The performance counter's frequency is too high for the current Fang implementation.\n" );
			return FALSE;
		}

		// Store clock frequency & period...
		FTimer_nClockFrequency = (u32)_nClockFreqLargeInt.QuadPart;
		FTimer_fClockPeriod = 1.0f / (f32)FTimer_nClockFrequency;
	#endif

	_bModuleInitialized = TRUE;

	#if FANG_PLATFORM_XB
		LARGE_INTEGER nTFrequency;
		QueryPerformanceFrequency( &nTFrequency );
		CFTimer::InitFrequencyAndPeriod( nTFrequency.LowPart );
	#else
		CFTimer::InitFrequencyAndPeriod( (u32)FCpu_IntelInfo.fCyclesPerSecond_RDTSC );
	#endif

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
	#if _USE_RDTSC_INSTEAD_OF_QPC
		_ClockTicks = _RDTSC64();
	#else
		LARGE_INTEGER nPerfCounter;

		FASSERT( _bModuleInitialized );

		QueryPerformanceCounter( &_nLastQPC );
		_nLastGTC = GetTickCount();

		nPerfCounter = _nLastQPC;
		_nBaseClockTicks64 = nPerfCounter.QuadPart - uResetVal;
	#endif
}


//
//
//
f32 ftimer_Clock_GetSeconds( void ) 
{
	#if _USE_RDTSC_INSTEAD_OF_QPC
		u64 nSample64, nDelta64;

		FASSERT( _bModuleInitialized );

		nSample64 = _RDTSC64();
		nDelta64 = nSample64 - _ClockTicks;

		return ( ((f32)nDelta64 * FTimer_fClockPeriod) );
	#else
		return ( (f32)ftimer_Clock_GetTicks() * FTimer_fClockPeriod );
	#endif
}


//
//
//
u64 ftimer_Clock_GetTicks( void ) 
{
	#if _USE_RDTSC_INSTEAD_OF_QPC
		u64 nSample64, nDelta64;

		FASSERT( _bModuleInitialized );

		nSample64 = _RDTSC64();
		nDelta64 = nSample64 - _ClockTicks;

		return nDelta64;
	#else
		LARGE_INTEGER nPerfCounter;
		DWORD nTickCount;
		f32 fDeltaQPC_Secs, fDeltaGTC_Secs, fDelta_Secs;
		u64 nDelta;
		s64 nAdjust;

		FASSERT( _bModuleInitialized );

		QueryPerformanceCounter( &nPerfCounter );
		nTickCount = GetTickCount();

		fDeltaQPC_Secs = (f32)( nPerfCounter.QuadPart - _nLastQPC.QuadPart ) * FTimer_fClockPeriod;
		fDeltaGTC_Secs = (f32)( nTickCount - _nLastGTC ) * 0.001f;

		#if 0
			//xxxxxxxxxxxx
			if( fDeltaGTC_Secs > 0.05f ) 
			{
				DEVPRINTF( "GTC %f ms\n", fDeltaGTC_Secs * 1000.0f );
			}
		#endif

		fDelta_Secs = fDeltaQPC_Secs - fDeltaGTC_Secs;

		if( fmath_Abs( fDelta_Secs ) > 0.05f ) 
		{
			nAdjust = (s64)( fDelta_Secs * (f32)FTimer_nClockFrequency );
			_nBaseClockTicks64 += nAdjust;
//DEVPRINTF( "Adjusted %i\n", (s32)nAdjust );
		}

		nDelta = nPerfCounter.QuadPart - _nBaseClockTicks64;

		_nLastQPC = nPerfCounter;
		_nLastGTC = nTickCount;

		return nDelta;
	#endif
}



//-------------------------------------------------------------------------------------------------------------------
// CFTimer:
//-------------------------------------------------------------------------------------------------------------------

u32 CFTimer::m_nTimerFrequency = 1;
f32 CFTimer::m_fTimerPeriod = 1.0f;


//
//
//
void CFTimer::Reset( void ) 
{
	FASSERT( _bModuleInitialized );

	#if FANG_PLATFORM_XB
		LARGE_INTEGER Ticks;
		QueryPerformanceCounter( &Ticks );
		m_nTimerTicks = Ticks.LowPart;
	#else
		m_nTimerTicks = _RDTSC32();
	#endif
}


//
//
//
f32 CFTimer::SampleSeconds( BOOL bResetTimer ) 
{
	u32 nSampleLow, nDeltaLow;

	FASSERT( _bModuleInitialized );

	#if FANG_PLATFORM_XB
		LARGE_INTEGER Ticks;
		QueryPerformanceCounter( &Ticks );
		nSampleLow = Ticks.LowPart;
	#else
		nSampleLow = _RDTSC32();
	#endif

	nDeltaLow = nSampleLow - (u32)m_nTimerTicks;

	if( bResetTimer ) 
	{
		m_nTimerTicks = nSampleLow;
	}

	return ( ((f32)nDeltaLow * m_fTimerPeriod) );
}


//
//
//
u32 CFTimer::SampleTicks( BOOL bResetTimer ) 
{
	u32 nSampleLow, nDeltaLow;

	FASSERT( _bModuleInitialized );

	#if FANG_PLATFORM_XB
		LARGE_INTEGER Ticks;
		QueryPerformanceCounter( &Ticks );
		nSampleLow = Ticks.LowPart;
	#else
		nSampleLow = _RDTSC32();
	#endif

	nDeltaLow = nSampleLow - (u32)m_nTimerTicks;

	if( bResetTimer ) 
	{
		m_nTimerTicks = nSampleLow;
	}

	return nDeltaLow;
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



#if !FANG_PLATFORM_XB
	// Executes the CPU RDTSC instruction and returns the 32-bit low result.
	static inline u32 _RDTSC32( void ) 
	{
		u32 nLow;

		__asm 
		{
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
	static inline u64 _RDTSC64( void ) 
	{
		u32 nLow, nHigh;

		__asm 
		{
			push eax
			push edx

			rdtsc
			mov nLow, eax
			mov nHigh, edx

			pop edx
			pop eax
		}

		return (u64)nLow + ( ((u64)(nHigh)) << 32 );
	}
#endif
