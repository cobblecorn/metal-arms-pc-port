//////////////////////////////////////////////////////////////////////////////////////
// fgcforce.h - GameCube force feedback system to handle high-level waveforms.
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
// 06/07/02 Lafleur		Created
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fforce.h"
#include "fpadio.h"
#include "fres.h"
#include "floop.h"
#include "fmath.h"

#define _GC_MOTOR_ON_TIME_COUNT			12
//#define _GC_MOTOR_FINE_OFF_TIME			0.025f
#define _GC_MOTOR_FINE_OFF_TIME			0.008f
#define _GC_MOTOR_COARSE_OFF_TIME		0.075f


//
//
//
f32 _GCMotor_CoarseOnTimes[ _GC_MOTOR_ON_TIME_COUNT ] =
{
	0.017f, 0.025f, 0.033f, 0.042f, 0.058f, 0.075f, 0.083f, 0.1f,  0.125f, 0.15f, 0.175f, 0.2f
};

#if 1
//
//
//
f32 _GCMotor_FineOnTimes[ _GC_MOTOR_ON_TIME_COUNT ] =
{
	0.008f, 0.017f, 0.025f, 0.033f, 0.042f, 0.050f,  0.058f, 0.067f, 0.083f, 0.1f,  0.15f, 0.2f
};

#else
//
//
//
f32 _GCMotor_FineOnTimes[ _GC_MOTOR_ON_TIME_COUNT ] =
{
	0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.10f,  0.1f, 0.1f, 0.1f, 0.1f,  0.15f, 0.2f
};
#endif

//
//
//
struct _Env_t
{
	FForce_Effect_e nEffectType;
	
	u8 nDeviceIndex;
	u8 nDomain;					// See FForceDomain_e for info
	u32 nKey;

	BOOL bCoarse;				// If not set, then use fine
	
	f32 fAttackIntensityOverTime;
	f32 fDecayStartTime;
	f32 fOODecayTime;
	f32 fSustainStartTime;
	f32 fSustainIntensity;
	f32 fReleaseStartTime;
	f32 fReleaseEndTime;
	f32 fReleaseIntensityOverTime;
	
	f32 fProgressSecs;			// Amount of time that's gone by since this envelope was activated
	
	_Env_t *pPrior;
	_Env_t *pNext;

};


//
//
//
struct _MotorState_t
{
	FPadio_GCFFMode_e nState;
	f32 fSecsInState;
	
	u32 nIntensity;
	
	BOOL bWindingUp;
};


static BOOL _bModuleInitialized;
static u32 _nMaxEnvelopes;
static u32 _nCurrentKey;

static f32 _afMasterUnitIntensity[FPADIO_MAX_DEVICES];
static u32 _anActiveEnvelopeCount[FFORCE_DOMAIN_COUNT][FPADIO_MAX_DEVICES];
//static f32 _afUnitAccumulatedVibe[FFORCE_DOMAIN_COUNT][FPADIO_MAX_DEVICES];
static _Env_t *_paEnvelopePool;
static _Env_t *_pDeviceEnv[FPADIO_MAX_DEVICES];
static _MotorState_t _fGCMotor_CurrState[FPADIO_MAX_DEVICES];

static void _Kill( _Env_t *pEnv );

//
//
//
BOOL fforce_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	FResFrame_t ResFrame = fres_GetFrame();

	_nMaxEnvelopes =  Fang_ConfigDefs.nForce_MaxActiveEnvelopes * FPADIO_MAX_DEVICES;

	// Attempt to allocate memory for the pool
	_paEnvelopePool = (_Env_t *)fres_Alloc( sizeof(_Env_t) * _nMaxEnvelopes );
	if ( _paEnvelopePool == NULL ) 
	{
		DEVPRINTF( "fforce_ModuleStartup(): Could not allocate %u bytes for force envelope pool.\n", sizeof(_Env_t) * _nMaxEnvelopes );
		DEVPRINTF( "                        Force-feedback waveform generator has been shut down.\n" );

		_paEnvelopePool = NULL;
		_nMaxEnvelopes = 0;
		
		fres_ReleaseFrame( ResFrame );
	}

	u16 j;
	
	// Set the initial intensity to 1.f
	for ( j = 0; j < FPADIO_MAX_DEVICES; j++ ) 
	{
		_afMasterUnitIntensity[j] = 1.f;
	}
		
	_nCurrentKey = 1;

	_bModuleInitialized = TRUE;

	fforce_Reset();

	return TRUE;
}


//
//
//
void fforce_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}


//
//	fforce_Reset() - Makes sure all controllers have force-feedback
//	turned off and clears the envelope pool.
//
void fforce_Reset( void ) 
{
	FASSERT( _bModuleInitialized );

	u16 i, j;
	
	// Clear the link list head for the devices
	for ( j = 0; j < FPADIO_MAX_DEVICES; j++ ) 
	{
		for( i=0; i<FFORCE_DOMAIN_COUNT; ++i ) {
			_anActiveEnvelopeCount[i][j] = 0;
//			_afUnitAccumulatedVibe[i][j] = 0.0f;
		}
		
		if ( _pDeviceEnv[j] )
		{
			_pDeviceEnv[j] = NULL;
			fpadio_SetForcefeedback_GC( j, FPADIO_GCFFMODE_HARD_OFF );
		}
		
		_fGCMotor_CurrState[j].fSecsInState = 0.f;
		_fGCMotor_CurrState[j].nState = FPADIO_GCFFMODE_HARD_OFF;
		_fGCMotor_CurrState[j].nIntensity = 0.f;
		_fGCMotor_CurrState[j].bWindingUp = FALSE;
	}

	// If we don't have a pool, bail
	if ( !_paEnvelopePool )
	{
		return;
	}
	
	// Mark all of the envelopes as available
	for ( j = 0; j < _nMaxEnvelopes; j++ ) 
	{
		_paEnvelopePool[j].nDomain = FFORCE_DOMAIN_NORMAL;
		_paEnvelopePool[j].nEffectType = FFORCE_EFFECT_NONE;
	}
}


//
//
//
f32 fforce_SetMasterIntensity( u32 nDeviceIndex, f32 fUnitIntensity ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex < FPADIO_MAX_DEVICES );
	FASSERT_UNIT_FLOAT( fUnitIntensity );

	f32 fPrevValue;
	
	fPrevValue = _afMasterUnitIntensity[nDeviceIndex];

	_afMasterUnitIntensity[nDeviceIndex] = fUnitIntensity;

	return fPrevValue;
}


//
//
//
f32 fforce_GetMasterIntensity( u32 nDeviceIndex ) 
{

	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex < FPADIO_MAX_DEVICES );
	
	return _afMasterUnitIntensity[nDeviceIndex];
}


//
//
//
static void _Kill( _Env_t *pEnv )
{
	// Do some sanity checks, first				
	FASSERT( pEnv->nDeviceIndex >= 0 && pEnv->nDeviceIndex < FPADIO_MAX_DEVICES );
	
	// Remove the envelope from its Device list
	
	if ( pEnv->pPrior )
	{
		// If there is a prior, that means that this envelope is not
		// at the head of the device's envelope list
		FASSERT( pEnv->nDeviceIndex == pEnv->pPrior->nDeviceIndex );
		pEnv->pPrior->pNext = pEnv->pNext;
	}
	else
	{
		// There is no prior, so that means that this envelope MUST 
		// be at the beginning of the device list
		FASSERT( _pDeviceEnv[pEnv->nDeviceIndex] == pEnv );
		_pDeviceEnv[pEnv->nDeviceIndex] = pEnv->pNext;
	}
	
	if ( pEnv->pNext )
	{
		FASSERT( pEnv->nDeviceIndex == pEnv->pNext->nDeviceIndex );
		pEnv->pNext->pPrior = pEnv->pPrior;
	}
	
	// Decrement the device's envelope count
	_anActiveEnvelopeCount[pEnv->nDomain][pEnv->nDeviceIndex]--;
	
	// Flag the envelope as available
	pEnv->nEffectType = FFORCE_EFFECT_NONE;
}


//
//
//
void fforce_Kill( FForceHandle_t *phHandle )
{
	if ( phHandle && phHandle->apEnv[0] ) 
	{
		// Valid handle...

		FASSERT( _bModuleInitialized );

		// Kill the primary envelope
		_Env_t *pEnv = (_Env_t *)phHandle->apEnv[0];
	
		if ( pEnv->nKey == phHandle->nKey ) 
		{
			// Keys match...
	
			if ( pEnv->nEffectType != FFORCE_EFFECT_NONE ) 
			{
				// Envelope is still active.
				// Kill it...
				
				_Kill( pEnv );
			}
		}
		
		// Invalidate the handle...
		phHandle->apEnv[0] = NULL;
		
		// Kill the secondary envelope, if any
		pEnv = (_Env_t *)phHandle->apEnv[1];
	
		if ( pEnv && pEnv->nKey == phHandle->nKey ) 
		{
			// Keys match...
	
			if ( pEnv->nEffectType != FFORCE_EFFECT_NONE ) 
			{
				// Envelope is still active.
				// Kill it...
				
				_Kill( pEnv );
			}
		}
		
		// Invalidate the handle...
		phHandle->apEnv[1] = NULL;
	}
};


//
//
//
static void _SetEnvelope ( _Env_t *pEnv, f32 fAttackIntensity, f32 fDecayStartTime, f32 fSustainStartTime,
					f32 fSustainIntensity, f32 fReleaseStartTime, f32 fReleaseEndTime, BOOL bCoarse )
{
	pEnv->bCoarse = bCoarse;
	
	if ( fDecayStartTime != 0.f )
	{	
		pEnv->fAttackIntensityOverTime = fmath_Div( fAttackIntensity, fDecayStartTime );
	}
	else
	{
		pEnv->fAttackIntensityOverTime = 0.f;
	}
	
	pEnv->fDecayStartTime = fDecayStartTime;
	
	if ( (fSustainStartTime - fDecayStartTime) != 0.f )
	{	
		pEnv->fOODecayTime  = fmath_Div( fAttackIntensity - fSustainIntensity, fSustainStartTime - fDecayStartTime );
	}
	else
	{
		pEnv->fOODecayTime  = 0.f;
	}
	
	pEnv->fSustainStartTime = fSustainStartTime;
	pEnv->fSustainIntensity = fSustainIntensity;
	
	pEnv->fReleaseStartTime = fReleaseStartTime;
	pEnv->fReleaseEndTime   = fReleaseEndTime;
	if ( (fReleaseEndTime - fReleaseStartTime) != 0.f )
	{	
		pEnv->fReleaseIntensityOverTime = fmath_Div( fSustainIntensity, (fReleaseEndTime - fReleaseStartTime) );
	}
	else
	{
		pEnv->fReleaseIntensityOverTime  = 0.f;
	}
}


//
//
//
BOOL fforce_Play( u32 nDeviceIndex, FForce_Effect_e nEffect, FForceHandle_t *phDestHandle, FForceDomain_e nDomain )
{
	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex >= 0 && nDeviceIndex < FPADIO_MAX_DEVICES );
	FASSERT( nEffect > FFORCE_EFFECT_NONE && nEffect < FFORCE_MAX_FEEDBACK_EFFECTS );
	u32 i;
	
	// NULL the handle...
	if ( phDestHandle ) 
	{
		phDestHandle->apEnv[0] = NULL;
		phDestHandle->apEnv[1] = NULL;
	}

	// Find an available envelope
	for ( i = 0; i < _nMaxEnvelopes; i++ ) 
	{
		if ( _paEnvelopePool[i].nEffectType == FFORCE_EFFECT_NONE ) 
		{
			break;
		}
	}

	// If we couldn't find an open envelope slot, bail out	
	if ( i == _nMaxEnvelopes )
	{
		return FALSE;
	}
	
	_Env_t *pEnv = &_paEnvelopePool[i];

	fpadio_SetForcefeedback_GC( nDeviceIndex, FPADIO_GCFFMODE_ON );
	_fGCMotor_CurrState[nDeviceIndex].nState = FPADIO_GCFFMODE_ON;
	
	// Fill in the general info
	pEnv->nEffectType = nEffect;
	pEnv->nKey = _nCurrentKey;
	pEnv->nDeviceIndex = nDeviceIndex;
	pEnv->fProgressSecs = 0.f;
	pEnv->pPrior = NULL;
	pEnv->nDomain = nDomain;
	
	// Set up the envelope based on the effect requested
	switch ( nEffect )
	{
		case FFORCE_EFFECT_ARROW_THUNK_LIGHT:
		case FFORCE_EFFECT_ARROW_THUNK_MED:
		case FFORCE_EFFECT_ARROW_THUNK_HEAVY:
		
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.110f,		// Decay Start Time
							0.125f,		// Sustain Start Time
							0.200f,		// Sustain Intensity
							0.175f, 	// Release Start Time
							0.200f,		// Release End Time
							TRUE );		// Coarse
			break;
		
		case FFORCE_EFFECT_LIGHT_PULSE:
			_SetEnvelope(	pEnv, 
							0.06f,		// Attack Intensity
							0.04f,		// Decay Start Time
							0.04f,		// Sustain Start Time
							0.06f,		// Sustain Intensity
							0.12f, 		// Release Start Time
							0.20f,		// Release End Time
							FALSE );	// Coarse
			break;
			
		case FFORCE_EFFECT_ROCKET_THRUST_LIGHT:
		
			_SetEnvelope(	pEnv, 
							0.70f,		// Attack Intensity
							0.110f,		// Decay Start Time
							0.125f,		// Sustain Start Time
							0.200f,		// Sustain Intensity
							0.135f, 	// Release Start Time
							0.400f,		// Release End Time
							FALSE );	// Coarse
			break;
		
		case FFORCE_EFFECT_ROCKET_THRUST_HEAVY:
		
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.030f,		// Decay Start Time
							0.045f,		// Sustain Start Time
							0.800f,		// Sustain Intensity
							0.075f, 	// Release Start Time
							0.800f,		// Release End Time
							FALSE );	// Coarse
			break;

		case FFORCE_EFFECT_MENU_RUMBLE:
			
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.110f,		// Decay Start Time
							0.125f,		// Sustain Start Time
							1.000f,		// Sustain Intensity
							0.175f, 	// Release Start Time
							3.000f,		// Release End Time
							TRUE );		// Coarse
			break;
			
		// NKM added the rumbles below
		case FFORCE_EFFECT_ROUGH_RUMBLE:
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.030f,		// Decay Start Time
							0.045f,		// Sustain Start Time
							0.500f,		// Sustain Intensity
							0.075f, 	// Release Start Time
							0.800f,		// Release End Time
							TRUE );		// Coarse
			break;

		case FFORCE_EFFECT_ROUGH_RUMBLE_LOW:
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.001f,		// Decay Start Time
							0.001f,		// Sustain Start Time
							0.001f,		// Sustain Intensity
							0.200f, 	// Release Start Time
							0.220f,		// Release End Time
							FALSE );	// Coarse	
			break;
			
					
		case FFORCE_EFFECT_ROUGH_RUMBLE_MED:
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.010f,		// Decay Start Time
							0.010f,		// Sustain Start Time
							0.010f,		// Sustain Intensity
							0.200f, 	// Release Start Time
							0.500f,		// Release End Time
							FALSE );	// Coarse			
			break;

		case FFORCE_EFFECT_ROUGH_RUMBLE_HIGH:
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.010f,		// Decay Start Time
							0.010f,		// Sustain Start Time
							0.500f,		// Sustain Intensity
							0.200f, 	// Release Start Time
							0.500f,		// Release End Time
							FALSE );	// Coarse							
			break;
				
		case FFORCE_EFFECT_ROUGH_RUMBLE_MAX:
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.010f,		// Decay Start Time
							0.010f,		// Sustain Start Time
							0.900f,		// Sustain Intensity
							0.200f, 	// Release Start Time
							0.500f,		// Release End Time
							FALSE );	// Coarse							
			break;
			
		case FFORCE_EFFECT_LIGHT_BUZZ:
			_SetEnvelope(	pEnv, 
							1.00f,		// Attack Intensity
							0.001f,		// Decay Start Time
							0.001f,		// Sustain Start Time
							0.030f,		// Sustain Intensity
							0.200f, 	// Release Start Time
							0.220f,		// Release End Time
							FALSE );	// Coarse

			break;
		
		default:
			DEVPRINTF( "FFORCE_PLAY() - ERROR - Unsupported force-feedback effect requested.\n" );
			break;
		
	}

	// Append this envelope to the beginning of the device's envelope linklist
	if ( _pDeviceEnv[nDeviceIndex] == NULL )
	{
		_pDeviceEnv[nDeviceIndex] = pEnv;
		pEnv->pNext = NULL;
	}
	else
	{
		pEnv->pNext = _pDeviceEnv[nDeviceIndex];
		_pDeviceEnv[nDeviceIndex]->pPrior = pEnv;
		_pDeviceEnv[nDeviceIndex] = pEnv;
	}
	
	_anActiveEnvelopeCount[nDomain][pEnv->nDeviceIndex]++;
	
	// Init the handle...
	if ( phDestHandle ) 
	{
		phDestHandle->nKey = _nCurrentKey++;
		phDestHandle->apEnv[0] = pEnv;
	}

	return TRUE;
}


//
//
//
static void _SetMotorIntensity( u32 nDevice, f32 fIntensityRequested, BOOL bCoarse )
{
	FASSERT( fIntensityRequested >= 0.f && fIntensityRequested <= 1.f );
	_fGCMotor_CurrState[nDevice].fSecsInState += FLoop_fPreviousLoopSecs;
	
	if ( fIntensityRequested < 0.001f )
	{
		if ( _fGCMotor_CurrState[nDevice].nState == FPADIO_GCFFMODE_ON )
		{
			fpadio_SetForcefeedback_GC( nDevice, FPADIO_GCFFMODE_HARD_OFF );
			_fGCMotor_CurrState[nDevice].nState = FPADIO_GCFFMODE_HARD_OFF;
			_fGCMotor_CurrState[nDevice].fSecsInState = 0.f;
		}
		
		_fGCMotor_CurrState[nDevice].nIntensity = 0.f;
		_fGCMotor_CurrState[nDevice].bWindingUp = FALSE;
		
		return;
	}
	
	if ( _fGCMotor_CurrState[nDevice].nIntensity == 0.f && _fGCMotor_CurrState[nDevice].fSecsInState > 0.25f )
	{
		fpadio_SetForcefeedback_GC( nDevice, FPADIO_GCFFMODE_ON );
		_fGCMotor_CurrState[nDevice].nState = FPADIO_GCFFMODE_ON;
		_fGCMotor_CurrState[nDevice].fSecsInState = 0.f;
		_fGCMotor_CurrState[nDevice].bWindingUp = TRUE;
		_fGCMotor_CurrState[nDevice].nIntensity = fIntensityRequested;
		
		return;
	}
	
	if ( _fGCMotor_CurrState[nDevice].bWindingUp == TRUE && _fGCMotor_CurrState[nDevice].fSecsInState < 0.05f )
	{
		// Motor is currently winding up, so let it continue
		return;
	}

	// Motor is not winding up and some intensity was requested, so check against the on/off times
	if ( _fGCMotor_CurrState[nDevice].nState == FPADIO_GCFFMODE_ON )
	{
		f32 fOnTime;
		
		f32 fIndex = fIntensityRequested * (_GC_MOTOR_ON_TIME_COUNT - 1) + 0.5f;
		
		// Calculate On Time
		if ( bCoarse )
		{
			fOnTime = _GCMotor_CoarseOnTimes[ (u32)(fIndex) ];
		}
		else
		{
			fOnTime = _GCMotor_FineOnTimes[ (u32)(fIndex) ];
		}
		
		if ( _fGCMotor_CurrState[nDevice].fSecsInState > fOnTime )
		{
			fpadio_SetForcefeedback_GC( nDevice, FPADIO_GCFFMODE_HARD_OFF );
			_fGCMotor_CurrState[nDevice].nState = FPADIO_GCFFMODE_HARD_OFF;
			_fGCMotor_CurrState[nDevice].fSecsInState = 0.f;
		}
	}
	else
	{
		// Check against off time
		if ( bCoarse )
		{
			if ( _fGCMotor_CurrState[nDevice].fSecsInState > _GC_MOTOR_COARSE_OFF_TIME )
			{
				fpadio_SetForcefeedback_GC( nDevice, FPADIO_GCFFMODE_ON );
				_fGCMotor_CurrState[nDevice].nState = FPADIO_GCFFMODE_ON;
				_fGCMotor_CurrState[nDevice].fSecsInState = 0.f;
			}
		}
		else
		{
			if ( _fGCMotor_CurrState[nDevice].fSecsInState > _GC_MOTOR_FINE_OFF_TIME )
			{
				fpadio_SetForcefeedback_GC( nDevice, FPADIO_GCFFMODE_ON );
				_fGCMotor_CurrState[nDevice].nState = FPADIO_GCFFMODE_ON;
				_fGCMotor_CurrState[nDevice].fSecsInState = 0.f;
			}
		}
	}		
		
	_fGCMotor_CurrState[nDevice].bWindingUp = FALSE;
	_fGCMotor_CurrState[nDevice].nIntensity = fIntensityRequested;
}


//
//
//
void fforce_Work( void ) 
{
	FASSERT( _bModuleInitialized );
	u32 j;
//	f32 fUnitIntensity;

	FASSERT( _bModuleInitialized );

	if ( FLoop_bGamePaused ) 
	{
		// Gameloop is paused...
		for ( j = 0; j < FPADIO_MAX_DEVICES; j++ ) 
		{
			_SetMotorIntensity( j, 0.f, FALSE );
		}
	}
	
	// We're not paused, so process the envelopes
	for ( j = 0; j < FPADIO_MAX_DEVICES; j++ ) 
	{
		f32 fIntensityRequested = 0.f, fUnitIntensity;
		
		if ( _afMasterUnitIntensity[j] != 0.f )
		{
			_Env_t *pEnv = _pDeviceEnv[j];
			while ( pEnv )
			{
				// Update the envelope
				if (   (pEnv->nDomain == FFORCE_DOMAIN_PAUSED && FLoop_bGamePaused)
					|| (pEnv->nDomain == FFORCE_DOMAIN_NORMAL && !FLoop_bGamePaused) )
				{
					if ( pEnv->fProgressSecs > pEnv->fDecayStartTime )
					{
						if ( pEnv->fProgressSecs > pEnv->fSustainStartTime )
						{
							if ( pEnv->fProgressSecs > pEnv->fReleaseStartTime )
							{
								if ( pEnv->fProgressSecs > pEnv->fReleaseEndTime )
								{
									_Kill( pEnv );
									fUnitIntensity = 0.f;
								}
								else
								{
									fUnitIntensity = pEnv->fReleaseIntensityOverTime * (pEnv->fReleaseEndTime - pEnv->fProgressSecs);
								}
							}
							else
							{
								fUnitIntensity = pEnv->fSustainIntensity;
							}
						}
						else
						{
							fUnitIntensity = (pEnv->fOODecayTime * (pEnv->fProgressSecs - pEnv->fDecayStartTime)) + pEnv->fSustainIntensity;
						}
					}
					else
					{
						fUnitIntensity = pEnv->fAttackIntensityOverTime * pEnv->fProgressSecs;
					}
					
					fIntensityRequested += fUnitIntensity;
					
					pEnv->fProgressSecs += FLoop_fPreviousLoopSecs;
				}
				
				pEnv = pEnv->pNext;
			}
		}
		
		FMATH_CLAMPMAX( fIntensityRequested, 1.0f );
		_SetMotorIntensity( j, fIntensityRequested, FALSE );
	}
}

