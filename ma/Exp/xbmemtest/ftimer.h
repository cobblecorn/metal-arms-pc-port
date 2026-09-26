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




class CFTimer {
public:
	inline CFTimer() {}
	inline ~CFTimer() {}

	void Reset( void );
	float SampleSeconds( bool bResetTimer = 0 );
	unsigned int SampleTicks( bool bResetTimer = 0 );

	static inline unsigned int GetFrequency( void ) { return m_nTimerFrequency; }
	static inline float GetPeriod( void ) { return m_fTimerPeriod; }
	static void InitFrequencyAndPeriod( unsigned int nFrequency );

protected:
	unsigned int m_nTimerTicks;

	static unsigned int m_nTimerFrequency;
	static float m_fTimerPeriod;
};


extern float FTimer_fClockFrequency;


extern bool ftimer_ModuleStartup( void );
extern void ftimer_ModuleShutdown( void );

#endif

