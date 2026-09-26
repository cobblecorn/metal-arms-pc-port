//////////////////////////////////////////////////////////////////////////////////////
// fgcpadio.cpp - 
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
// 12/12/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <dolphin.h>

#include "fang.h"
#include "fgc.h"
#include "fmath.h"
#include "fres.h"
#include "fclib.h"
#include "fpadio.h"


#define _SLOWEST_SAMPLING_RATE_SUPPORTED	10
#define _FASTEST_SAMPLING_RATE_SUPPORTED	125

#define _ALL_SOCKETS		( PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT )


// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Devices.
static u32 _afForcefeedback[ FPADIO_MAX_DEVICES ];
static PADStatus _aoStatuses[ FPADIO_MAX_DEVICES ];
static FPadio_DeviceInfo_t _oDeviceInfo;

// Double sample buffers.
static volatile FPadio_Sample_t **_papaoSamplesA;
static volatile FPadio_Sample_t **_papaoSamplesB;
static volatile FPadio_Sample_t ** volatile _papaoCurrentSamples;
static volatile u32 _uValidSamples, _uResetPads, _uConnectedSockets;
static u32 _uSamplesPerBuffer;


static void _fpadio_SamplingCallback( void );
static FINLINE void _NormalizeInputStick( f32 *pfSX, f32 *pfSY, const f32 &fMin, const f32 &fMax, const f32 &fOOMin, const f32 &fOOMax );
static FINLINE void _RenormalizeStickSquare( f32 &rfX, f32 &rfY );


// Values for these #defines were taken from /gamecube/sdk/man/prog/pad/PADClamp.html
#define _LSTICK_RANGE_MIN 	 ( 40.0f )
#define _LSTICK_RANGE_MAX 	 ( 72.0f )
#define _LSTICK_RANGE_OO_MIN ( 1.0f / _LSTICK_RANGE_MIN )
#define _LSTICK_RANGE_OO_MAX ( 1.0f / _LSTICK_RANGE_MAX )

#define _RSTICK_RANGE_MIN 	 ( 31.0f )
#define _RSTICK_RANGE_MAX 	 ( 59.0f )
#define _RSTICK_RANGE_OO_MIN ( 1.0f / _RSTICK_RANGE_MIN )
#define _RSTICK_RANGE_OO_MAX ( 1.0f / _RSTICK_RANGE_MAX )

static FINLINE void _NormalizeInputStick( f32 *pfSX, f32 *pfSY, const f32 &fMin, const f32 &fMax, const f32 &fOOMin, const f32 &fOOMax )
{
	f32 fX, fY;
	f32 fAngle, fScaledX, fScaledY;
	
	fX = FMATH_FABS( *pfSX );
	fY = FMATH_FABS( *pfSY );
	
	fAngle = fmath_Atan( fY, fX );
	
	if( fAngle <= FMATH_QUARTER_PI ) 
	{
		fScaledX = ( fX * fOOMax );
		fScaledY = ( fY * fOOMin );
	}
	else 
	{
		fScaledX = ( fX * fOOMin );
		fScaledY = ( fY * fOOMax );
	}

	fX = fmath_Div( fX, ( fScaledY * fMin ) + ( ( 1.0f - fScaledY ) * fMax ) );
	fY = fmath_Div( fY, ( fScaledX * fMin ) + ( ( 1.0f - fScaledX ) * fMax ) );
				
	f32 fMag2 = ( fX * fX ) + ( fY * fY );

	if( fMag2 > 1.0f )
	{
		fMag2 = fmath_InvSqrt( fMag2 );				
		fX *= fMag2;
		fY *= fMag2;
	}
	
	*pfSX = fX * FMATH_FSIGN( *pfSX );
	*pfSY = fY * FMATH_FSIGN( *pfSY );
}

#define _SCALE_X_VALUE ( 0.85f )
#define _SCALE_Y_VALUE ( 1.3f )

static FINLINE void _RenormalizeStickSquare( f32 &rfX, f32 &rfY )
{
	f32 fMag2, fAngle, fMultipler, fSin, fCos;

	// fixup the right x,y axis
	fMag2 = ( rfX * rfX ) + ( rfY * rfY );
	if( fMag2 > 0.0f )
	{
		// Get the angle that x y represents.
		fAngle = fmath_Atan( FMATH_FABS( rfY ), FMATH_FABS( rfX ) );

		fmath_SinCos( fAngle, &fSin, &fCos );
		fMultipler = fmath_Inv( FMATH_MAX( FMATH_FABS( fSin ), FMATH_FABS( fCos ) ) );
		//fMultipler *= _OO_MAX_STICK_RANGE;

		rfX *= fMultipler;
		rfY *= fMultipler;

		// see if we need to clamp the length
		fMag2 = ( rfX * rfX ) + ( rfY * rfY );
		if( fMag2 > 1.0f )
		{
			fMultipler = fmath_InvSqrt( fMag2 );
			rfX *= fMultipler;
			rfY *= fMultipler;
		}							

		// x' = x / sqrt(1 - _SCALE_X_VALUE * (y * y))
		// y' = y * _SCALE_Y_VALUE

		rfX = rfX * fmath_InvSqrt( 1 - (_SCALE_X_VALUE * FMATH_SQUARE( rfY )) );
		rfY = rfY * _SCALE_Y_VALUE;

		if( FMATH_FABS( rfX ) >= 0.9f )
		{
			rfX = FMATH_FSIGN( rfX );
		}

		if( FMATH_FABS( rfY ) >= 0.9f )
		{
			rfY = FMATH_FSIGN( rfY );
		}
	}
}

BOOL fpadio_ModuleStartup( void ) {

	FASSERT_MSG( ! _bModuleStarted, "[ FPADIO ] Error: System already started !!!" );

	_bModuleStarted = TRUE;
	_bModuleInstalled = FALSE;

	return TRUE;
} // fpadio_ModuleStartup

void fpadio_ModuleShutdown( void ) {
	fpadio_Uninstall();

	_bModuleStarted = FALSE;
} // fpadio_ModuleShutdown

FPadio_Error_e fpadio_Install( const FPadio_Init_t *poInit /* = NULL */ ) {
#pragma unused( poInit )

	FASSERT_MSG( _bModuleStarted,        "[ FPADIO ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled,    "[ FPADIO ] Error: System already installed !!!" );
	
	//// Clamping Fang_ConfigDefs values.
	//
	FMATH_CLAMP( Fang_ConfigDefs.nPadio_MaxSamplesPerSec, _SLOWEST_SAMPLING_RATE_SUPPORTED, _FASTEST_SAMPLING_RATE_SUPPORTED );
	//
	////

	//// Allocate buffers memory.
	//
	_uSamplesPerBuffer = ( Fang_ConfigDefs.nPadio_MaxSamplesPerSec / _SLOWEST_SAMPLING_RATE_SUPPORTED );
	if( ( _uSamplesPerBuffer * _SLOWEST_SAMPLING_RATE_SUPPORTED ) != Fang_ConfigDefs.nPadio_MaxSamplesPerSec )
	{
		++_uSamplesPerBuffer;
	}

	FResFrame_t oTempResFrame = fres_GetFrame();

	////
	//
	_papaoSamplesA = (volatile FPadio_Sample_t **)fres_Alloc( sizeof( FPadio_Sample_t * ) * FPADIO_MAX_DEVICES );
	_papaoSamplesB = (volatile FPadio_Sample_t **)fres_Alloc( sizeof( FPadio_Sample_t * ) * FPADIO_MAX_DEVICES );
	if( ! ( _papaoSamplesA && _papaoSamplesB ) )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: Can't allocate memory for buffers !!!\n", __LINE__ );
		return FPADIO_ERROR;
	}

	for( u32 uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
	{
		_papaoSamplesA[ uIndex ] = (volatile FPadio_Sample_t *)fres_AllocAndZero( sizeof( FPadio_Sample_t ) * _uSamplesPerBuffer );
		_papaoSamplesB[ uIndex ] = (volatile FPadio_Sample_t *)fres_AllocAndZero( sizeof( FPadio_Sample_t ) * _uSamplesPerBuffer );

		if( ! ( _papaoSamplesA[ uIndex ] && _papaoSamplesB[ uIndex ] ) )
		{
			DEVPRINTF( "[ FPADIO ] Error %u: Can't allocate memory for buffers !!!\n", __LINE__ );
			fres_ReleaseFrame( oTempResFrame );
			return FPADIO_ERROR;
		}
	}
	//
	////

	_papaoCurrentSamples = _papaoSamplesA;
	_uValidSamples       = 0;
	_uResetPads          = _ALL_SOCKETS;
	_uConnectedSockets	 = 0;
	//
	//// Allocate buffers memory.

	////
	//
	fang_MemZero( &_oDeviceInfo, sizeof( _oDeviceInfo ) );

	fclib_strcpy( _oDeviceInfo.szName, "GameCube controller" );

	_oDeviceInfo.oeID = FPADIO_INPUT_GC_GAMEPAD;
	_oDeviceInfo.uInputs = FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z;

	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_RESET - 1 ) ] = FPADIO_INPUT_RESET;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_START - 1 ) ] = FPADIO_INPUT_START;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] = FPADIO_INPUT_STICK_LEFT_X;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] = FPADIO_INPUT_STICK_LEFT_Y;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] = FPADIO_INPUT_STICK_RIGHT_X;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] = FPADIO_INPUT_STICK_RIGHT_Y;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_TRIGGER_LEFT - 1 ) ] = FPADIO_INPUT_TRIGGER_LEFT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_TRIGGER_RIGHT - 1 ) ] = FPADIO_INPUT_TRIGGER_RIGHT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_DPAD_X - 1 ) ] = FPADIO_INPUT_DPAD_X;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_DPAD_Y - 1 ) ] = FPADIO_INPUT_DPAD_Y;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_CROSS_TOP - 1 ) ] = FPADIO_INPUT_CROSS_TOP;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_CROSS_BOTTOM - 1 ) ] = FPADIO_INPUT_CROSS_BOTTOM;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_CROSS_LEFT - 1 ) ] = FPADIO_INPUT_CROSS_LEFT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_CROSS_RIGHT - 1 ) ] = FPADIO_INPUT_CROSS_RIGHT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT - 1 ) ] = FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT - 1 ) ] = FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z - 1 ) ] = FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_GC_ABUTTON_A - 1 ) ] = FPADIO_INPUT_GC_ABUTTON_A;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_GC_ABUTTON_B - 1 ) ] = FPADIO_INPUT_GC_ABUTTON_B;
	//
	////

	fang_MemZero( _afForcefeedback, sizeof( _afForcefeedback ) );

	PADInit();
	PADSetSamplingCallback( _fpadio_SamplingCallback );
	u32 nMilliSecond = 1000 / Fang_ConfigDefs.nPadio_MaxSamplesPerSec;
	if( nMilliSecond > 11 ) {
		nMilliSecond = 0;
	}
	SISetSamplingRate( nMilliSecond );

	_bModuleInstalled = TRUE;

	return FPADIO_NO_ERROR;

} // fpadio_Install

void fpadio_Uninstall( void ) {

	if( ! _bModuleInstalled ) {
		return;
	}

	PADSetSamplingCallback( NULL );

	_bModuleInstalled = FALSE;
} // fpadio_Uninstall

BOOL fpadio_IsInstalled( void ) {
	return _bModuleInstalled;
} // fpadio_IsInstalled

void fpadio_SetForcefeedback_GC( u32 uDeviceIndex, FPadio_GCFFMode_e oeMode ) {
	FASSERT_MSG( _bModuleInstalled,                                         "[ FPADIO ] Error: System not installed !!!" );
	FASSERT_MSG( FPADIO_MAX_DEVICES > uDeviceIndex,                         "[ FPADIO ] Error: Invalid device index !!!" );
	FASSERT_MSG( ( ( oeMode >= 0 ) && ( oeMode < FPADIO_GCFFMODE_COUNT ) ), "[ FPADIO ] Error: Invalid mode sent to fpadio_SetForcefeedback_GC() !!!" );

	static const u32 __auMotorControl[ FPADIO_GCFFMODE_COUNT ] = {
		PAD_MOTOR_STOP,
		PAD_MOTOR_STOP_HARD,
		PAD_MOTOR_RUMBLE
	};

	_afForcefeedback[ uDeviceIndex ] = __auMotorControl[ oeMode ];
} // fpadio_SetForcefeedback_GC

void fpadio_ApplyAllForcefeedbacks( void ) {
	FASSERT_MSG( _bModuleStarted, "[ FPADIO ] Error: System not installed !!!" );

	// If the app has not installed fpadio yet, don't apply force feedback.
	if( ! _bModuleInstalled ) {
		return;
	}

	PADControlAllMotors( _afForcefeedback );
} // fpadio_ApplyAllForcefeedbacks

void fpadio_GetSamples( u32 *puValidSamples, FPadio_Sample_t ***ppapaoSamples ) {

	FASSERT_MSG( _bModuleInstalled, "[ FPADIO ] Error: System not installed !!!" );
	FASSERT_MSG( puValidSamples,    "[ FPADIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( ppapaoSamples,     "[ FPADIO ] Error: NULL pointer !!!" );

	OSDisableInterrupts();

	*puValidSamples = _uValidSamples;
	_uValidSamples = 0;
	*ppapaoSamples = (FPadio_Sample_t **)_papaoCurrentSamples;
	_papaoCurrentSamples = ( ( _papaoSamplesA == _papaoCurrentSamples ) ? _papaoSamplesB : _papaoSamplesA );

	if( _uResetPads ) {
		PADReset( _uResetPads );
		if( _uConnectedSockets ) {
			// prevents reset from getting called again before the next callback
			// (unless there are no controllers plugged in, which means the callback won't get called if we don't call reset often)
			_uResetPads = 0;
		}
	}

	OSEnableInterrupts();
} // fpadio_GetSamples

void fpadio_GetDeviceInfo( u32 uDeviceIndex, FPadio_DeviceInfo_t *poDeviceInfo ) {

	FASSERT_MSG( _bModuleInstalled,                 "[ FPADIO ] Error: System not installed !!!" );
	FASSERT_MSG( FPADIO_MAX_DEVICES > uDeviceIndex, "[ FPADIO ] Error: Invalid device index !!!" );
	FASSERT_MSG( poDeviceInfo,                      "[ FPADIO ] Error: NULL pointer !!!" );

	OSDisableInterrupts();

	fang_MemCopy( poDeviceInfo, &( _oDeviceInfo ), sizeof( _oDeviceInfo ) );

	OSEnableInterrupts();
} // fpadio_GetDeviceInfo

void _fpadio_SamplingCallback( void ) {
	FPadio_Sample_t *poSample;
	PADStatus *poStatus;
	f32 fReset, *pafValues;
	f32 fX, fY;
	
	_uResetPads = 0;
	_uConnectedSockets = 0;

	PADRead( _aoStatuses );
	PADClamp( _aoStatuses );

	fReset = ( OSGetResetButtonState() ? 1.0f : 0.0f );

	for( u32 uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex ) {
		poSample = (FPadio_Sample_t *)&_papaoCurrentSamples[ uIndex ][ _uValidSamples ];
		pafValues = poSample->afInputValues;
		poStatus = &_aoStatuses[ uIndex ];

		switch( poStatus->err ) {
		
		case PAD_ERR_NONE:
			poSample->bValid = TRUE;

			pafValues[ ( FPADIO_INPUT_RESET - 1 ) ] = fReset;

			pafValues[ ( FPADIO_INPUT_START - 1 ) ] = ( ( poStatus->button & PAD_BUTTON_START ) ? 1.0f : 0.0f );

			// New input method
			fX = (f32) poStatus->stickX;
			fY = (f32) poStatus->stickY;
			_NormalizeInputStick( &fX, &fY, _LSTICK_RANGE_MIN, _LSTICK_RANGE_MAX, _LSTICK_RANGE_OO_MIN, _LSTICK_RANGE_OO_MAX );
			
			pafValues[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] = fX;
			pafValues[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] = fY;
			
			fX = (f32) poStatus->substickX;
			fY = (f32) poStatus->substickY;
			_NormalizeInputStick( &fX, &fY, _RSTICK_RANGE_MIN, _RSTICK_RANGE_MAX, _RSTICK_RANGE_OO_MIN, _RSTICK_RANGE_OO_MAX );
			_RenormalizeStickSquare( fX, fY );
			
			pafValues[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] = fX;
			pafValues[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] = fY;
			// End new input method
			
			pafValues[ ( FPADIO_INPUT_TRIGGER_LEFT - 1 ) ] = ( poStatus->triggerLeft * ( 1.0f / 150.0f ) );
			pafValues[ ( FPADIO_INPUT_TRIGGER_RIGHT - 1 ) ] = ( poStatus->triggerRight * ( 1.0f / 150.0f ) );

			pafValues[ ( FPADIO_INPUT_DPAD_X - 1 ) ]  = ( ( poStatus->button & PAD_BUTTON_LEFT  ) ? -1.0f : 0.0f );
			pafValues[ ( FPADIO_INPUT_DPAD_X - 1 ) ] += ( ( poStatus->button & PAD_BUTTON_RIGHT ) ?  1.0f : 0.0f );

			pafValues[ ( FPADIO_INPUT_DPAD_Y - 1 ) ]  = ( ( poStatus->button & PAD_BUTTON_DOWN ) ? -1.0f : 0.0f );
			pafValues[ ( FPADIO_INPUT_DPAD_Y - 1 ) ] += ( ( poStatus->button & PAD_BUTTON_UP   ) ?  1.0f : 0.0f );

			pafValues[ ( FPADIO_INPUT_CROSS_TOP - 1 ) ] = ( ( poStatus->button & PAD_BUTTON_Y ) ? 1.0f : 0.0f );
			pafValues[ ( FPADIO_INPUT_CROSS_BOTTOM - 1 ) ] = ( ( poStatus->button & PAD_BUTTON_A ) ? 1.0f : 0.0f );
			pafValues[ ( FPADIO_INPUT_CROSS_LEFT - 1 ) ] = ( ( poStatus->button & PAD_BUTTON_B ) ? 1.0f : 0.0f );
			pafValues[ ( FPADIO_INPUT_CROSS_RIGHT - 1 ) ] = ( ( poStatus->button & PAD_BUTTON_X ) ? 1.0f : 0.0f );

			pafValues[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT - 1 ) ] = ( ( poStatus->button & PAD_TRIGGER_L ) ? 1.0f : 0.0f );
			pafValues[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT - 1 ) ] = ( ( poStatus->button & PAD_TRIGGER_R ) ? 1.0f : 0.0f );
			pafValues[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z - 1 ) ] = ( ( poStatus->button & PAD_TRIGGER_Z ) ? 1.0f : 0.0f );
			
			_uConnectedSockets |= ( PAD_CHAN0_BIT >> uIndex );
			break;
			
		case PAD_ERR_NO_CONTROLLER:
			// need to reset the pad
			_uResetPads |= ( PAD_CHAN0_BIT >> uIndex );
			poSample->bValid = FALSE;
			break;
			
		case PAD_ERR_TRANSFER:
			// consider this connected, but the sample data is not valid
			poSample->bValid = FALSE;
			_uConnectedSockets |= ( PAD_CHAN0_BIT >> uIndex );
			break;
			
		default:
		case PAD_ERR_NOT_READY:
			poSample->bValid = FALSE;	
			break;
		}
	}

	if( _uValidSamples < ( _uSamplesPerBuffer - 1 ) ) {
		++_uValidSamples;
	}

	if( 0.0f < fReset ) {
		PADRecalibrate( _ALL_SOCKETS );
	}
} // _fpadio_SamplingCallback
