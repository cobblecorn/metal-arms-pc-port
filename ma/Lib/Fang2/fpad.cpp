//////////////////////////////////////////////////////////////////////////////////////
// fpad.cpp - 
//
// Author: Albert Yale
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
// 01/28/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <stdio.h>

#include "fang.h"
#include "fmath.h"
#include "fres.h"
#include "fclib.h"
#include "floop.h"
#include "fpadio.h"
#include "fpad.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Samples.
static FPad_Sample_t _aoSamples[ FPADIO_MAX_DEVICES ][ FPADIO_MAX_INPUTS ];
static f32 _afInitialDelay[ FPADIO_MAX_DEVICES ][ FPADIO_MAX_INPUTS ], _afDelay[ FPADIO_MAX_DEVICES ][ FPADIO_MAX_INPUTS ];
static BOOL _abFirstInitialDelay[ FPADIO_MAX_DEVICES ][ FPADIO_MAX_INPUTS ];
static f32 _fRepeatInitialDelay, _fRepeatDelay, _fHysteresisThresholdLow, _fHysteresisThresholdHigh;
static u32 _uPreviousConnections, _uPreviousSamples;

static FPad_Sample_t _oAlwaysZeroSample; // Used for actions that are not mapped.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fpad_ModuleStartup( void )
{
	FASSERT_MSG( ! _bModuleStarted, "[ FPAD ] Error: System already started !!!" );

	_bModuleStarted   = TRUE;
	_bModuleInstalled = FALSE;

	return TRUE;

} // fpad_ModuleStartup

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpad_ModuleShutdown( void )
{
	fpad_Uninstall();

	_bModuleStarted = FALSE;

} // fpad_ModuleShutdown

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FPad_Error_e fpad_Install( f32 fRepeatInitialDelay /* = 1.0f */, f32 fRepeatDelay /* = 1.0f */, f32 fHysteresisThresholdLow /* = 0.1f */, f32 fHysteresisThresholdHigh /* = 0.2f */ )
{
	FASSERT_MSG( _bModuleStarted,     "[ FPAD ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled, "[ FPAD ] Error: System already installed !!!" );

	fang_MemZero( _aoSamples, sizeof( _aoSamples ) );
	fang_MemZero( _afInitialDelay, sizeof( _afInitialDelay ) );
	fang_MemZero( _abFirstInitialDelay, sizeof( _abFirstInitialDelay ) );
	fang_MemZero( _afDelay, sizeof( _afDelay ) );
	fang_MemZero( &_oAlwaysZeroSample, sizeof( _oAlwaysZeroSample ) ); // Make sure that the empty sample data is always 0.

	_fRepeatInitialDelay = fRepeatInitialDelay;
	_fRepeatDelay = fRepeatDelay;

	_fHysteresisThresholdLow = fHysteresisThresholdLow;
	_fHysteresisThresholdHigh = fHysteresisThresholdHigh;

	_bModuleInstalled = TRUE;

	return FPAD_NO_ERROR;

} // fpad_Install

void fpad_ZeroAllControllerSamples( void ) {
	FASSERT_MSG( _bModuleInstalled,              "[ FPAD ] Error: System not installed !!!" );

	fang_MemZero( _aoSamples, sizeof( _aoSamples ) );

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpad_Uninstall( void )
{
	if( ! _bModuleInstalled ) return;

	_bModuleInstalled = FALSE;

} // fpad_Uninstall

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fpad_IsInstalled( void )
{
	return _bModuleInstalled;

} // fpad_IsInstalled

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpad_Map( u32 uPadIndex, u32 uNumIDs, const FPadio_InputID_e *paeInputIDs, FPad_Sample_t **papoSamples )
{
	FASSERT_MSG( _bModuleInstalled,              "[ FPAD ] Error: System not installed !!!" );
	FASSERT_MSG( FPADIO_MAX_DEVICES > uPadIndex, "[ FPAD ] Error: Invalid pad index !!!" );
	FASSERT_MSG( uNumIDs,                        "[ FPAD ] Error: Zero input types !!!" );
	FASSERT_MSG( paeInputIDs,                    "[ FPAD ] Error: NULL pointer !!!" );
	FASSERT_MSG( papoSamples,                    "[ FPAD ] Error: NULL pointer !!!" );

	FPad_Sample_t *paoSamplesDev = _aoSamples[ uPadIndex ];

	for( u32 uInput = 0; uInput < uNumIDs; ++uInput )
	{
		FASSERT_MSG( FPADIO_MAX_INPUTS >= paeInputIDs[ uInput ], "[ FPAD ] Error: Invalid input type !!!" );

		if( paeInputIDs[ uInput ] )
		{
			papoSamples[ uInput ] = &( paoSamplesDev[ (u32)( paeInputIDs[ uInput ] ) - 1 ] );
		}
		else
		{
			papoSamples[ uInput ] = &_oAlwaysZeroSample;
		}
	}

} // fpad_Map

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 fpad_UpdateSamples( void )
{
	FASSERT_MSG( _bModuleInstalled,    "[ FPAD ] Error: System not installed !!!" );
	FASSERT_MSG( fpadio_IsInstalled(), "[ FPAD ] Error: FPADIO not installed !!!" );

	f32 fTemp, fTempMin, fTempMax, *pafDelay, *pfDelay, *pafInitialDelay, *pfInitialDelay;
	BOOL *pabFirstInitialDelay, *pbFirstInitialDelay;
	u32 uValidSamples, uConnected, uIndex, uDev;
	s32 nTemp, _nTemp2;
	FPadio_Sample_t **paoIoSamples, *paoIoSamplesDev, *poIoSample;
	FPad_Sample_t *paoSamplesDev, *poSample;

	fpadio_GetSamples( &uValidSamples, &paoIoSamples );

	////
	//
	if( ! uValidSamples )
	{
		for( uDev = 0; FPADIO_MAX_DEVICES > uDev; ++uDev )
		{
			paoIoSamplesDev      = paoIoSamples[ uDev ];
			pafDelay             = _afDelay[ uDev ];
			pafInitialDelay      = _afInitialDelay[ uDev ];
			pabFirstInitialDelay = _abFirstInitialDelay[ uDev ];

			for( nTemp = (s32)( _uPreviousSamples - 1 ); 0 <= nTemp; --nTemp )
			{
				if( ! paoIoSamplesDev[ nTemp ].bValid )
				{
					continue;
				}

				paoSamplesDev = _aoSamples[ uDev ];

				for( uIndex = 0; FPADIO_MAX_INPUTS > uIndex; ++uIndex )
				{
					poSample = &( paoSamplesDev[ uIndex ] );

					poSample->fVelocity  = 0.0f;
					poSample->uLatches  &= FPAD_LATCH_ON;

					if( poSample->uLatches )
					{
						pfInitialDelay = &( pafInitialDelay[ uIndex ] );

						// "Still On".
						*pfInitialDelay += FLoop_fRealPreviousLoopSecs;
						pbFirstInitialDelay = &( pabFirstInitialDelay[ uIndex ] );
						if( *pbFirstInitialDelay )
						{
							if( *pfInitialDelay >= _fRepeatInitialDelay )
							{
								*pfInitialDelay = 0.0f;
								poSample->uLatches = ( FPAD_LATCH_ON | FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY );
								*pbFirstInitialDelay = FALSE;
							}
						}
						else
						{
							if( *pfInitialDelay >= _fRepeatDelay )
							{
								*pfInitialDelay = 0.0f;
								poSample->uLatches = ( FPAD_LATCH_ON | FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY );
							}
						}

						pfDelay = &( pafDelay[ uIndex ] );
						*pfDelay += FLoop_fRealPreviousLoopSecs;
						if( *pfDelay >= _fRepeatDelay )
						{
							*pfDelay = 0.0f;
							poSample->uLatches = ( FPAD_LATCH_ON | FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_NO_INITIAL_DELAY );
						}
					}
				}
			}
		}

		return _uPreviousConnections;
	}

	_uPreviousSamples = uValidSamples;
	//
	////

	////
	//
	uConnected = 0;

	for( uDev = 0; FPADIO_MAX_DEVICES > uDev; ++uDev )
	{
		paoIoSamplesDev      = paoIoSamples[ uDev ];
		paoSamplesDev        = _aoSamples[ uDev ];
		pafDelay             = _afDelay[ uDev ];
		pafInitialDelay      = _afInitialDelay[ uDev ];
		pabFirstInitialDelay = _abFirstInitialDelay[ uDev ];

		for( nTemp = (s32)( uValidSamples - 1 ); 0 <= nTemp; --nTemp )
		{
			poIoSample = &( paoIoSamplesDev[ nTemp ] );

			if( ! poIoSample->bValid )
			{
				fang_MemZero( paoSamplesDev, sizeof( _aoSamples[ uDev ] ) );
				fang_MemZero( pafDelay, sizeof( _afDelay[ uDev ] ) );
				break;
			}

			uConnected |= ( 1 << uDev );

			for( uIndex = 0; FPADIO_MAX_INPUTS > uIndex; ++uIndex )
			{
				//// Set FPAD_LATCH_ON, FPAD_LATCH_CHANGED, FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT, FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_NO_INITIAL_DELAY, FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY.
				//
				poSample            = &( paoSamplesDev[ uIndex ] );
				fTemp               = poIoSample->afInputValues[ uIndex ];
				pbFirstInitialDelay = &( pabFirstInitialDelay[ uIndex ] );

				if( FPAD_LATCH_ON & poSample->uLatches )
				{
					pfInitialDelay = &( pafInitialDelay[ uIndex ] );
					pfDelay = &( pafDelay[ uIndex ] );

					if( ( + _fHysteresisThresholdLow >= fTemp ) &&
						( - _fHysteresisThresholdLow <= fTemp ) )
					{
						// "Turned Off".
						poSample->uLatches = FPAD_LATCH_CHANGED;
						*pfInitialDelay = 0.0f;
						*pfDelay = 0.0f;
						*pbFirstInitialDelay = FALSE;
					}
					else
					{
						// "Still On".
						poSample->uLatches = FPAD_LATCH_ON;

						*pfInitialDelay += FLoop_fRealPreviousLoopSecs;
						if( *pbFirstInitialDelay )
						{
							if( *pfInitialDelay >= _fRepeatInitialDelay )
							{
								*pfInitialDelay = 0.0f;
								poSample->uLatches |= FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY;
								*pbFirstInitialDelay = FALSE;
							}
						}
						else
						{
							if( *pfInitialDelay >= _fRepeatDelay )
							{
								*pfInitialDelay = 0.0f;
								poSample->uLatches |= FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY;
							}
						}

						*pfDelay += FLoop_fRealPreviousLoopSecs;
						if( *pfDelay >= _fRepeatDelay )
						{
							*pfDelay = 0.0f;
							poSample->uLatches |= FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_NO_INITIAL_DELAY;
						}
					}
				}
				else
				{
					if( ( + _fHysteresisThresholdHigh <= fTemp ) ||
						( - _fHysteresisThresholdHigh >= fTemp ) )
					{
						// "Turned On".
						poSample->uLatches = ( FPAD_LATCH_ON | FPAD_LATCH_CHANGED | FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT | FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_NO_INITIAL_DELAY | FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY );
						*pbFirstInitialDelay = TRUE;
					}
					else
					{
						// "Still Off".
						poSample->uLatches = 0;
					}
				}
				//
				////

				//// Set FPAD_LATCH_SPIKED.
				//
				if( ! ( FPAD_LATCH_CHANGED & poSample->uLatches ) )
				{
					if( FPAD_LATCH_ON & poSample->uLatches )
					{
						for( _nTemp2 = (s32)( uValidSamples - 1 ); 0 <= _nTemp2; --_nTemp2 )
						{
							if( ! paoIoSamplesDev[ _nTemp2 ].bValid ) continue;

							if( ( + _fHysteresisThresholdLow >= fTemp ) &&
								( - _fHysteresisThresholdLow <= fTemp ) )
							{
								poSample->uLatches |= FPAD_LATCH_SPIKED;
								break;
							}
						}
					}
					else
					{
						for( _nTemp2 = (s32)( uValidSamples - 1 ); 0 <= _nTemp2; --_nTemp2 )
						{
							if( ! paoIoSamplesDev[ _nTemp2 ].bValid ) continue;

							if( ( + _fHysteresisThresholdHigh <= fTemp ) ||
								( - _fHysteresisThresholdHigh >= fTemp ) )
							{
								poSample->uLatches |= FPAD_LATCH_SPIKED;
								break;
							}
						}
					}
				}
				//
				////

				////
				//
				fTempMin = fTempMax = 0.0f;

				for( _nTemp2 = (s32)( uValidSamples - 1 ); 0 <= _nTemp2; --_nTemp2 )
				{
					if( ! paoIoSamplesDev[ _nTemp2 ].bValid ) continue;

					if( fTempMin > fTemp )
					{
						fTempMin = fTemp;
					}
					else if( fTempMax < fTemp )
					{
						fTempMax = fTemp;
					}
				}

				if( ( - fTempMin ) > fTempMax )
				{
					fTempMax = fTempMin;
				}

				poSample->fVelocity = poSample->fCurrentState - fTempMax;
				poSample->fVelocity = FMATH_FABS( poSample->fVelocity );

				poSample->fCurrentState = fTempMax;
				//
				////
			}

			break;
		}
	}
	//
	////

	_uPreviousConnections = uConnected;

	return uConnected;

} // fpad_UpdateSamples

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
