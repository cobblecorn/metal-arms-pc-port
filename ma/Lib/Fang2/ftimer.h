//////////////////////////////////////////////////////////////////////////////////////
// ftimer.h - Fang timer module.
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

#ifndef _FTIMER_H_
#define _FTIMER_H_ 1

#include "fang.h"



class CFTimer {
public:
	inline CFTimer() {}

	void Reset( void );
	f32 SampleSeconds( BOOL bResetTimer = FALSE );
	u32 SampleTicks( BOOL bResetTimer = FALSE );

	static inline u32 GetFrequency( void ) { return m_nTimerFrequency; }
	static inline f32 GetPeriod( void ) { return m_fTimerPeriod; }
	static void InitFrequencyAndPeriod( u32 nFrequency );

protected:
	u64 m_nTimerTicks;

	static u32 m_nTimerFrequency;
	static f32 m_fTimerPeriod;

	FCLASS_STACKMEM_NOALIGN( CFTimer );
};


extern u32 FTimer_nClockFrequency;
extern f32 FTimer_fClockPeriod;


extern BOOL ftimer_ModuleStartup( void );
extern void ftimer_ModuleShutdown( void );

extern void ftimer_Clock_Reset( u64 uResetVal = 0 );
extern f32 ftimer_Clock_GetSeconds( void );
extern u64 ftimer_Clock_GetTicks( void );

#endif

