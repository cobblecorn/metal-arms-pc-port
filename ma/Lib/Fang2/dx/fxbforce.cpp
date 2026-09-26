//////////////////////////////////////////////////////////////////////////////////////
// fxbforce.h - Xbox force feedback system to handle high-level waveforms.
//
// Author: Steve Ranck     
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
// 03/13/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fforce.h"
#include "fres.h"
#include "floop.h"
#include "fpadio.h"
#include "fmath.h"


typedef enum {
	_MOTOR_TYPE_COARSE,
	_MOTOR_TYPE_FINE,

	_MOTOR_TYPE_COUNT
} _MotorType_e;

typedef struct {
	_MotorType_e nMotorType;	// _MOTOR_TYPE_COUNT means this slot is free
	u8 nDeviceIndex;
	u8 nDomain;					// See FForceDomain_e for info
	u32 nKey;

	f32 fProgressSecs;			// Amount of time that's gone by since this envelope was activated

	f32 fStartSustainTime;
	f32 fEndSustainTime;
	f32 fEndDecayTime;

	f32 fIntensityOverAttackSecs;
	f32 fSustainIntensity;
	f32 fIntensityOverDecaySecs;
} _Env_t;


static BOOL _bModuleInitialized;
static u32 _nMaxEnvelopes;
static u32 _nCurrentKey;

static f32 _afMasterUnitIntensity[FPADIO_MAX_DEVICES];
static u32 _aanActiveEnvelopeCount[FFORCE_DOMAIN_COUNT][FPADIO_MAX_DEVICES];
static f32 _aaafUnitAccumulatedVibe[FFORCE_DOMAIN_COUNT][FPADIO_MAX_DEVICES][_MOTOR_TYPE_COUNT];

static _Env_t *_apEnvTable[FPADIO_MAX_DEVICES];


BOOL fxbforce_ImpulseCoarse( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
BOOL fxbforce_ImpulseFine( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
BOOL fxbforce_DecayCoarse( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
BOOL fxbforce_DecayFine( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );

BOOL fxbforce_EnvCoarse( u32 nDeviceIndex, f32 fAttackSecs, f32 fSustainSecs, f32 fDecaySecs, f32 fAttackUnitIntensity, f32 fSustainUnitIntensity, f32 fDecayUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
BOOL fxbforce_EnvFine( u32 nDeviceIndex, f32 fAttackSecs, f32 fSustainSecs, f32 fDecaySecs, f32 fAttackUnitIntensity, f32 fSustainUnitIntensity, f32 fDecayUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );


BOOL fforce_ModuleStartup( void ) {
	FResFrame_t ResFrame;
	u32 i, j;

	FASSERT( !_bModuleInitialized );

	ResFrame = fres_GetFrame();

	_nMaxEnvelopes = Fang_ConfigDefs.nForce_MaxActiveEnvelopes;

	for( j=0; j<FPADIO_MAX_DEVICES; j++ ) {
		_apEnvTable[j] = (_Env_t *)fres_Alloc( sizeof(_Env_t) * _nMaxEnvelopes );

		if( _apEnvTable[j] == NULL ) {
			DEVPRINTF( "fforce_ModuleStartup(): Could not allocate %u bytes for force envelope table.\n", sizeof(_Env_t) * _nMaxEnvelopes );
			DEVPRINTF( "                        Force-feedback waveform generator has been shut down.\n" );

			for( i=0; i<FPADIO_MAX_DEVICES; i++ ) {
				_apEnvTable[i] = NULL;
			}

			_nMaxEnvelopes = 0;
			fres_ReleaseFrame( ResFrame );

			break;
		}
	}

	for( j=0; j<FPADIO_MAX_DEVICES; j++ ) {
		_afMasterUnitIntensity[j] = 1.0f;

		for( i=0; i<_nMaxEnvelopes; i++ ) {
			_apEnvTable[j][i].nDeviceIndex = j;
			_apEnvTable[j][i].nDomain = FFORCE_DOMAIN_NORMAL;
		}
	}

	_nCurrentKey = 1;

	_bModuleInitialized = TRUE;

	fforce_Reset();

	return TRUE;
}

void fforce_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );
	_bModuleInitialized = FALSE;
}

void fforce_Reset( void ) {
	u32 i, j;

	FASSERT( _bModuleInitialized );

	for( j=0; j<FPADIO_MAX_DEVICES; j++ ) {
		for( i=0; i<FFORCE_DOMAIN_COUNT; ++i ) {
			_aanActiveEnvelopeCount[i][j] = 0;
			_aaafUnitAccumulatedVibe[i][j][0] = 0.0f;
			_aaafUnitAccumulatedVibe[i][j][1] = 0.0f;
		}

		// Free all slots...
		for( i=0; i<_nMaxEnvelopes; i++ ) {
			_apEnvTable[j][i].nMotorType = _MOTOR_TYPE_COUNT;
		}
	}

	// Note: We don't reset _nCurrentKey intentionally!
}

f32 fforce_SetMasterIntensity( u32 nDeviceIndex, f32 fUnitIntensity ) {
	f32 fPrevValue;

	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex < FPADIO_MAX_DEVICES );
	FASSERT_UNIT_FLOAT( fUnitIntensity );

	fPrevValue = _afMasterUnitIntensity[nDeviceIndex];

	_afMasterUnitIntensity[nDeviceIndex] = fUnitIntensity;

	return fPrevValue;
}

f32 fforce_GetMasterIntensity( u32 nDeviceIndex ) {
	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex < FPADIO_MAX_DEVICES );
	return _afMasterUnitIntensity[nDeviceIndex];
}

// Fills out the handle and returns TRUE if successful (note that this may only be partial success if only
// one of two envelopes could be created).
// Returns FALSE if there are too many pulses already active (the caller shouldn't consider this an error).
BOOL fforce_Play( u32 nDeviceIndex, FForce_Effect_e nEffect, FForceHandle_t *phDestHandle, FForceDomain_e nDomain ) {
	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex >= 0 && nDeviceIndex < FPADIO_MAX_DEVICES );
	FASSERT( nEffect > FFORCE_EFFECT_NONE && nEffect < FFORCE_MAX_FEEDBACK_EFFECTS );

	if ( phDestHandle )
	{
		phDestHandle->apEnv[0] = NULL;
		phDestHandle->apEnv[1] = NULL;
	}

	BOOL bSuccess = FALSE;

	switch ( nEffect )
	{
		case FFORCE_EFFECT_ARROW_THUNK_LIGHT:
			bSuccess |= fxbforce_ImpulseCoarse( nDeviceIndex, 0.02f, 1.0f, phDestHandle, nDomain );

			bSuccess |= fxbforce_EnvFine( nDeviceIndex,
				0.0f, 0.03f, 0.5f,
				0.0f, 1.0f, 0.5f,
				phDestHandle
			);

			break;
		
		case FFORCE_EFFECT_ARROW_THUNK_MED:
			bSuccess |= fxbforce_ImpulseCoarse( nDeviceIndex, 0.02f, 1.0f, phDestHandle, nDomain );

			bSuccess |= fxbforce_EnvFine( nDeviceIndex,
				0.0f, 0.05f, 0.6f,
				0.0f, 1.0f, 0.5f,
				phDestHandle
			);

			break;

		
		case FFORCE_EFFECT_ARROW_THUNK_HEAVY:
			bSuccess |= fxbforce_ImpulseCoarse( nDeviceIndex, 0.1f, 1.0f, phDestHandle, nDomain );

			bSuccess |= fxbforce_EnvFine( nDeviceIndex,
				0.0f, 0.1f, 0.8f,
				0.0f, 1.0f, 0.5f,
				phDestHandle
			);
			break;

		case FFORCE_EFFECT_LIGHT_PULSE:
			bSuccess |= fxbforce_DecayFine( nDeviceIndex, 0.15f, 0.7f, phDestHandle, nDomain );
			break;

		case FFORCE_EFFECT_ROCKET_THRUST_LIGHT:
			bSuccess |= fxbforce_EnvCoarse(
							nDeviceIndex,
							0.0f, 0.1f, 0.90f,
							0.0f, 1.0f, 0.55f,
							phDestHandle,
							nDomain
						);

			bSuccess |= fxbforce_EnvFine(
							nDeviceIndex,
							0.0f, 0.1f, 0.90f,
							0.0f, 1.0f, 0.55f,
							phDestHandle,
							nDomain
						);

			break;

		case FFORCE_EFFECT_ROCKET_THRUST_HEAVY:
			bSuccess |= fxbforce_EnvCoarse(
							nDeviceIndex,
							0.0f, 0.1f, 0.90f,
							0.0f, 1.0f, 0.75f,
							phDestHandle,
							nDomain
						);

			bSuccess |= fxbforce_EnvFine(
							nDeviceIndex,
							0.0f, 0.1f, 0.90f,
							0.0f, 1.0f, 0.75f,
							phDestHandle,
							nDomain
						);

			break;

		case FFORCE_EFFECT_ROUGH_RUMBLE:
			bSuccess |= fxbforce_ImpulseCoarse( nDeviceIndex, 0.2f, 1.0f, phDestHandle, nDomain );
			bSuccess |= fxbforce_ImpulseFine( nDeviceIndex, 0.5f, 1.0f, phDestHandle, nDomain );

			break;

		case FFORCE_EFFECT_ROUGH_RUMBLE_LOW:
			bSuccess |= fxbforce_DecayCoarse( nDeviceIndex, 0.2f, 0.55f, phDestHandle, nDomain );
			bSuccess |= fxbforce_DecayFine( nDeviceIndex, 0.2f, 0.65f, phDestHandle, nDomain );

			break;

		case FFORCE_EFFECT_ROUGH_RUMBLE_MED:
			bSuccess |= fxbforce_DecayCoarse( nDeviceIndex, 0.2f, 0.65f, phDestHandle, nDomain );
			bSuccess |= fxbforce_DecayFine( nDeviceIndex, 0.2f, 0.7f, phDestHandle, nDomain );

			break;

		case FFORCE_EFFECT_ROUGH_RUMBLE_HIGH:
			bSuccess |= fxbforce_DecayCoarse( nDeviceIndex, 0.2f, 0.75f, phDestHandle, nDomain );
			bSuccess |= fxbforce_DecayFine( nDeviceIndex, 0.2f, 0.75f, phDestHandle, nDomain );

			break;

		case FFORCE_EFFECT_ROUGH_RUMBLE_MAX:
			bSuccess |= fxbforce_DecayCoarse( nDeviceIndex, 0.2f, 1.0f, phDestHandle, nDomain );
			bSuccess |= fxbforce_DecayFine( nDeviceIndex, 0.2f, 1.0f, phDestHandle, nDomain );

			break;

		case FFORCE_EFFECT_LIGHT_BUZZ:
			bSuccess |= fxbforce_ImpulseFine( nDeviceIndex, 3.0f, 0.8f, phDestHandle, nDomain );

			break;

		case FFORCE_EFFECT_MENU_RUMBLE:
			bSuccess |= fxbforce_ImpulseCoarse( nDeviceIndex, 3.4f, 1.0f, phDestHandle, nDomain );
			bSuccess |= fxbforce_ImpulseFine( nDeviceIndex, 3.5f, 1.0f, phDestHandle, nDomain );
			break;

		case FFORCE_EFFECT_ENERGY_PULSE:
			bSuccess |= fxbforce_EnvCoarse(
							nDeviceIndex,
							0.0f, 0.0f, 0.5f,
							1.0f, 1.0f, 1.0f,
							phDestHandle,
							nDomain
						);

			bSuccess |= fxbforce_EnvFine(
							nDeviceIndex,
							0.2f, 0.2f, 0.5f,
							1.0f, 1.0f, 1.0f,
							phDestHandle,
							nDomain
						);

			break;

		default:
			DEVPRINTF( "FFORCE_PLAY() - ERROR - Unsupported force-feedback effect requested.\n" );
			break;
	}

	if ( phDestHandle && phDestHandle->apEnv[0] )
	{
		phDestHandle->nKey = _nCurrentKey++;
	}

	return bSuccess;
}

// Returns a non-NULL handle if successful.
// Returns NULL if there are too many pulses already active (the caller shouldn't consider this an error).
BOOL fxbforce_ImpulseCoarse( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle, FForceDomain_e nDomain ) {
	return fxbforce_EnvCoarse( nDeviceIndex, 0.0f, fSecs, 0.0f, 0.0f, fUnitIntensity, 0.0f, phDestHandle, nDomain );
}

// Returns a non-NULL handle if successful.
// Returns NULL if there are too many pulses already active (the caller shouldn't consider this an error).
BOOL fxbforce_ImpulseFine( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle, FForceDomain_e nDomain ) {
	return fxbforce_EnvFine( nDeviceIndex, 0.0f, fSecs, 0.0f, 0.0f, fUnitIntensity, 0.0f, phDestHandle, nDomain );
}

// Returns a non-NULL handle if successful.
// Returns NULL if there are too many pulses already active (the caller shouldn't consider this an error).
BOOL fxbforce_DecayCoarse( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle, FForceDomain_e nDomain ) {
	return fxbforce_EnvCoarse( nDeviceIndex, 0.0f, 0.0f, fSecs, 0.0f, 0.0f, fUnitIntensity, phDestHandle, nDomain );
}

// Returns a non-NULL handle if successful.
// Returns NULL if there are too many pulses already active (the caller shouldn't consider this an error).
BOOL fxbforce_DecayFine( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle, FForceDomain_e nDomain ) {
	return fxbforce_EnvFine( nDeviceIndex, 0.0f, 0.0f, fSecs, 0.0f, 0.0f, fUnitIntensity, phDestHandle, nDomain );
}

// Returns TRUE if successful.
// Returns FALSE if there are too many pulses already active (the caller shouldn't consider this an error).
BOOL fxbforce_EnvCoarse( u32 nDeviceIndex, f32 fAttackSecs, f32 fSustainSecs, f32 fDecaySecs,
						 f32 fAttackUnitIntensity, f32 fSustainUnitIntensity, f32 fDecayUnitIntensity, FForceHandle_t *phDestHandle, FForceDomain_e nDomain ) {
	_Env_t *pEnv;
	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex < FPADIO_MAX_DEVICES );
	FASSERT( fAttackSecs>=0.0f && fSustainSecs>=0.0f && fDecaySecs>=0.0f );

	for( i=0, pEnv=_apEnvTable[nDeviceIndex]; i<_nMaxEnvelopes; i++, pEnv++ ) {
		if( pEnv->nMotorType == _MOTOR_TYPE_COUNT ) {
			// Found unused entry...

			pEnv->nMotorType = _MOTOR_TYPE_COARSE;
			pEnv->nDomain = nDomain;
			pEnv->nKey = _nCurrentKey;
			pEnv->fProgressSecs = 0.0f;
			pEnv->fStartSustainTime = fAttackSecs;
			pEnv->fEndSustainTime = fAttackSecs + fSustainSecs;
			pEnv->fEndDecayTime = pEnv->fEndSustainTime + fDecaySecs;
			pEnv->fIntensityOverAttackSecs = (fAttackSecs > 0.0f) ? fmath_Div( fAttackUnitIntensity, fAttackSecs ) : 0.0f;
			pEnv->fSustainIntensity = fSustainUnitIntensity;
			pEnv->fIntensityOverDecaySecs = (fDecaySecs > 0.0f) ? fmath_Div( fDecayUnitIntensity, fDecaySecs ) : 0.0f;

			_aanActiveEnvelopeCount[nDomain][nDeviceIndex]++;

			if( phDestHandle ) {
				if ( phDestHandle->apEnv[0] != NULL )
				{
					// Shouldn't be trying to link more than two envelopes
					FASSERT( phDestHandle->apEnv[1] == NULL );
					phDestHandle->apEnv[1] = pEnv;
				}
				else
				{
					// Init the handle...
					phDestHandle->apEnv[0] = pEnv;
				}
			}

			return TRUE;
		}
	}

	// All entries currently active...

	return FALSE;
}

// Returns a non-NULL handle if successful.
// Returns NULL if there are too many pulses already active (the caller shouldn't consider this an error).
BOOL fxbforce_EnvFine( u32 nDeviceIndex, f32 fAttackSecs, f32 fSustainSecs, f32 fDecaySecs,
					   f32 fAttackUnitIntensity, f32 fSustainUnitIntensity, f32 fDecayUnitIntensity, FForceHandle_t *phDestHandle, FForceDomain_e nDomain ) {
	_Env_t *pEnv;
	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( nDeviceIndex < FPADIO_MAX_DEVICES );
	FASSERT( fAttackSecs>=0.0f && fSustainSecs>=0.0f && fDecaySecs>=0.0f );

	for( i=0, pEnv=_apEnvTable[nDeviceIndex]; i<_nMaxEnvelopes; i++, pEnv++ ) {
		if( pEnv->nMotorType == _MOTOR_TYPE_COUNT ) {
			// Found unused entry...
			pEnv->nMotorType = _MOTOR_TYPE_FINE;
			pEnv->nDomain = nDomain;
			pEnv->nKey = _nCurrentKey;
			pEnv->fProgressSecs = 0.0f;
			pEnv->fStartSustainTime = fAttackSecs;
			pEnv->fEndSustainTime = fAttackSecs + fSustainSecs;
			pEnv->fEndDecayTime = pEnv->fEndSustainTime + fDecaySecs;
			pEnv->fIntensityOverAttackSecs = (fAttackSecs > 0.0f) ? fmath_Div( fAttackUnitIntensity, fAttackSecs ) : 0.0f;
			pEnv->fSustainIntensity = fSustainUnitIntensity;
			pEnv->fIntensityOverDecaySecs = (fDecaySecs > 0.0f) ? fmath_Div( fDecayUnitIntensity, fDecaySecs ) : 0.0f;

			_aanActiveEnvelopeCount[nDomain][nDeviceIndex]++;

			if( phDestHandle ) {
				if ( phDestHandle->apEnv[0] != NULL )
				{
					// Shouldn't be trying to link more than two envelopes
					FASSERT( phDestHandle->apEnv[1] == NULL );
					phDestHandle->apEnv[1] = pEnv;
				}
				else
				{
					// Init the handle...
					phDestHandle->apEnv[0] = pEnv;
				}
			}

			return TRUE;
		}
	}

	// All entries currently active...

	return FALSE;
}

void fforce_Kill( FForceHandle_t *phHandle ) {
	_Env_t *pEnv;

	if( phHandle && phHandle->apEnv[0] ) {
		// Valid handle...

		FASSERT( _bModuleInitialized );

		// Kill the primary envelope
		pEnv = (_Env_t *)phHandle->apEnv[0];

		if( pEnv->nKey == phHandle->nKey ) {
			// Keys match...

			if( pEnv->nMotorType != _MOTOR_TYPE_COUNT ) {
				// Envelope is still active.
				// Kill it...

				pEnv->nMotorType = _MOTOR_TYPE_COUNT;
				_aanActiveEnvelopeCount[pEnv->nDomain][ pEnv->nDeviceIndex ]--;

				// Invalidate the handle...
				phHandle->apEnv[0] = NULL;
			}
		}

		// Kill the secondary envelope, if any
		pEnv = (_Env_t *)phHandle->apEnv[1];

		if( pEnv && pEnv->nKey == phHandle->nKey ) {
			// Keys match...

			if( pEnv->nMotorType != _MOTOR_TYPE_COUNT ) {
				// Envelope is still active.
				// Kill it...

				pEnv->nMotorType = _MOTOR_TYPE_COUNT;
				_aanActiveEnvelopeCount[pEnv->nDomain][ pEnv->nDeviceIndex ]--;

				// Invalidate the handle...
				phHandle->apEnv[1] = NULL;
			}
		}
	}
}

void fforce_Work( void ) {
	_Env_t *pEnv;
	u32 j, nRemainingEnvelopeCount, nDomain;
	f32 fUnitIntensity;

	FASSERT( _bModuleInitialized );

	for( nDomain=0; nDomain<FFORCE_DOMAIN_COUNT; ++nDomain ) {
		if( !FLoop_bGamePaused ) {
			// Game is not paused...

			if( nDomain == FFORCE_DOMAIN_PAUSED ) {
				// Skip this domain...
				continue;
			}
		} else {
			// Game is paused...

			if( nDomain == FFORCE_DOMAIN_NORMAL ) {
				// Skip this domain...
				continue;
			}
		}

		for( j=0; j<FPADIO_MAX_DEVICES; j++ ) {
			FMATH_CLAMPMAX( _aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_COARSE], 1.0f );
			FMATH_CLAMPMAX( _aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_FINE], 1.0f );

			for( pEnv=_apEnvTable[j], nRemainingEnvelopeCount=_aanActiveEnvelopeCount[nDomain][j]; nRemainingEnvelopeCount; pEnv++ ) {
				if( pEnv->nMotorType != _MOTOR_TYPE_COUNT ) {
					if( pEnv->nDomain == nDomain ) {
						FASSERT( pEnv->nMotorType>=0 && pEnv->nMotorType<_MOTOR_TYPE_COUNT );

						if( pEnv->fProgressSecs < pEnv->fStartSustainTime ) {
							// Attack...
							fUnitIntensity = pEnv->fProgressSecs * pEnv->fIntensityOverAttackSecs;
						} else if( pEnv->fProgressSecs < pEnv->fEndSustainTime ) {
							// Sustain...
							fUnitIntensity = pEnv->fSustainIntensity;
						} else {
							// Decay...
							fUnitIntensity = (pEnv->fEndDecayTime - pEnv->fProgressSecs) * pEnv->fIntensityOverDecaySecs;

							if( fUnitIntensity <= 0.0f ) {
								// Kill envelope...
								fUnitIntensity = 0.0f;
								pEnv->nMotorType = _MOTOR_TYPE_COUNT;
								_aanActiveEnvelopeCount[nDomain][j]--;
							}
						}

						_aaafUnitAccumulatedVibe[nDomain][j][ pEnv->nMotorType ] += fUnitIntensity;

						pEnv->fProgressSecs += FLoop_bGamePaused ? FLoop_fRealPreviousLoopSecs : FLoop_fPreviousLoopSecs;

						nRemainingEnvelopeCount--;
					}
				}
			}

			fpadio_SetForcefeedback_XB(
				j,
				_afMasterUnitIntensity[j] * _aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_COARSE] * _aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_COARSE],
				_afMasterUnitIntensity[j] * _aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_FINE] * _aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_FINE]
			);

			_aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_COARSE] = 0.0f;
			_aaafUnitAccumulatedVibe[nDomain][j][_MOTOR_TYPE_FINE] = 0.0f;
		}
	}
}

