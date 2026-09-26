//////////////////////////////////////////////////////////////////////////////////////
// AcuTimer.h - Timer functions acurate to a single CPU cycle.
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
// 09/25/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _ACUTIMER_H_
#define _ACUTIMER_H_

#include "fangtypes.h"



typedef struct {
	u32 nAcuTimerStartTicksLow;
	u32 nAcuTimerStartTicksHigh;
	u32 nAcuTimerDeltaTicksLow;
	u32 nAcuTimerDeltaTicksHigh;
} AcuTimer_t;


#define ACUTIMER_START( AcuTimer )									\
	__asm {															\
		__asm push ebx												\
		__asm push eax												\
		__asm lea ebx, AcuTimer										\
		__asm push edx												\
		__asm rdtsc													\
		__asm mov [ebx].nAcuTimerDeltaTicksLow, 0					\
		__asm mov [ebx].nAcuTimerDeltaTicksHigh, 0					\
		__asm mov [ebx].nAcuTimerStartTicksLow, eax					\
		__asm mov [ebx].nAcuTimerStartTicksHigh, edx				\
		__asm pop edx												\
		__asm pop eax												\
		__asm pop ebx												\
	}

#define ACUTIMER_START_P( pAcuTimer )								\
	__asm {															\
		__asm push ebx												\
		__asm push eax												\
		__asm mov ebx, pAcuTimer									\
		__asm push edx												\
		__asm rdtsc													\
		__asm mov [ebx].nAcuTimerDeltaTicksLow, 0					\
		__asm mov [ebx].nAcuTimerDeltaTicksHigh, 0					\
		__asm mov [ebx].nAcuTimerStartTicksLow, eax					\
		__asm mov [ebx].nAcuTimerStartTicksHigh, edx				\
		__asm pop edx												\
		__asm pop eax												\
		__asm pop ebx												\
	}

#define ACUTIMER_SAMPLE( AcuTimer )									\
	__asm {															\
		__asm push ebx												\
		__asm push ecx												\
		__asm push eax												\
		__asm lea ebx, AcuTimer										\
		__asm push edx												\
		__asm rdtsc													\
		__asm xor ecx, ecx											\
		__asm sub eax, [ebx].nAcuTimerStartTicksLow					\
		__asm sbb edx, [ebx].nAcuTimerStartTicksHigh				\
		__asm sub eax, AcuTimer_nCalibrateLow						\
		__asm sbb edx, 0											\
		__asm cmovc eax, ecx										\
		__asm cmovc edx, ecx										\
		__asm mov [ebx].nAcuTimerDeltaTicksLow, eax					\
		__asm mov [ebx].nAcuTimerDeltaTicksHigh, edx				\
		__asm pop edx												\
		__asm pop eax												\
		__asm pop ecx												\
		__asm pop ebx												\
	}

#define ACUTIMER_SAMPLE_P( pAcuTimer )								\
	__asm {															\
		__asm push ebx												\
		__asm push ecx												\
		__asm push eax												\
		__asm mov ebx, pAcuTimer									\
		__asm push edx												\
		__asm rdtsc													\
		__asm xor ecx, ecx											\
		__asm sub eax, [ebx].nAcuTimerStartTicksLow					\
		__asm sbb edx, [ebx].nAcuTimerStartTicksHigh				\
		__asm sub eax, AcuTimer_nCalibrateLow						\
		__asm sbb edx, 0											\
		__asm cmovc eax, ecx										\
		__asm cmovc edx, ecx										\
		__asm mov [ebx].nAcuTimerDeltaTicksLow, eax					\
		__asm mov [ebx].nAcuTimerDeltaTicksHigh, edx				\
		__asm pop edx												\
		__asm pop eax												\
		__asm pop ecx												\
		__asm pop ebx												\
	}


#if defined(__cplusplus)
extern "C" {
#endif

extern u32 AcuTimer_nCalibrateLow;


extern BOOL AcuTimer_ModuleInit( void );
extern void AcuTimer_Calibrate( void );
extern void AcuTimer_SetSecondsPerCycle( float fSecondsPerCycle );
extern float AcuTimer_GetSecondsPerCycle( void );
extern float AcuTimer_ElapsedSeconds( AcuTimer_t *pAcuTimer );
extern void AcuTimer_SleepTicks( u32 nTicksToSleep );
extern void AcuTimer_SleepSecs( float fSecsToSleep );



#if defined(__cplusplus)
}
#endif



#endif


