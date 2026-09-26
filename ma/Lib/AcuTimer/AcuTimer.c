//////////////////////////////////////////////////////////////////////////////////////
// AcuTimer.c - Timer functions acurate to a single CPU cycle.
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

#include "fangtypes.h"
#include "acutimer.h"


#define _CALIBRATION_ITERATIONS 25



u32 AcuTimer_nCalibrateLow;

static float _fSecondsPerCycle;

typedef struct {
	u32 nTicks;
	u32 nOccurances;
} _CalibrateData_t;


BOOL AcuTimer_ModuleInit( void ) {
	_fSecondsPerCycle = 1.0f;
	AcuTimer_nCalibrateLow = 0;

	return TRUE;
}

void AcuTimer_Calibrate( void ) {
	static AcuTimer_t CalibrateTimer;
	static _CalibrateData_t aCalData[_CALIBRATION_ITERATIONS];
	u32 i, j, nTicks, nMaxOccurances;

	AcuTimer_nCalibrateLow = 0;

	for( i=0; i<_CALIBRATION_ITERATIONS; i++ ) {
		aCalData[i].nTicks = 0xffffffff;
		aCalData[i].nOccurances = 0;
	}

	for( i=0; i<_CALIBRATION_ITERATIONS; i++ ) {
		ACUTIMER_START( CalibrateTimer );
		ACUTIMER_SAMPLE( CalibrateTimer );
		nTicks = CalibrateTimer.nAcuTimerDeltaTicksLow;

		for( j=0; j<_CALIBRATION_ITERATIONS; j++ ) {
			if( aCalData[j].nTicks == nTicks ) {
				aCalData[j].nOccurances++;
				break;
			}
		}
		if( j == _CALIBRATION_ITERATIONS ) {
			for( j=0; j<_CALIBRATION_ITERATIONS; j++ ) {
				if( aCalData[j].nTicks == 0xffffffff ) {
					aCalData[j].nTicks = nTicks;
					aCalData[j].nOccurances = 1;
					break;
				}
			}
		}
	}

	nMaxOccurances = 0;
	AcuTimer_nCalibrateLow = 0;
	for( i=0; i<_CALIBRATION_ITERATIONS; i++ ) {
		if( aCalData[i].nOccurances > nMaxOccurances ) {
			nMaxOccurances = aCalData[i].nOccurances;
			AcuTimer_nCalibrateLow = aCalData[i].nTicks;
		}
	}
}

void AcuTimer_SetSecondsPerCycle( float fSecondsPerCycle ) {
	_fSecondsPerCycle = fSecondsPerCycle;
}

float AcuTimer_GetSecondsPerCycle( void ) {
	return _fSecondsPerCycle;
}

float AcuTimer_ElapsedSeconds( AcuTimer_t *pAcuTimer ) {
	return _fSecondsPerCycle * ((float)pAcuTimer->nAcuTimerDeltaTicksLow + (float)pAcuTimer->nAcuTimerDeltaTicksHigh*4294967296.0f);
}

void AcuTimer_SleepTicks( u32 nTicksToSleep ) {
	AcuTimer_t SleepTimer;

	ACUTIMER_START( SleepTimer );
	do {
		ACUTIMER_SAMPLE( SleepTimer );
	} while( SleepTimer.nAcuTimerDeltaTicksLow < nTicksToSleep );
}

void AcuTimer_SleepSecs( float fSecsToSleep ) {
	AcuTimer_SleepTicks( (u32)(fSecsToSleep / _fSecondsPerCycle) );
}

