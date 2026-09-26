//////////////////////////////////////////////////////////////////////////////////////
// fxbpadio.cpp - 
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
// 11/30/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fdx8.h"
#include "fmath.h"
#include "fres.h"
#include "fclib.h"
#include "floop.h"
#include "fpadio.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define _MAX_STICK_RANGE								32767.0f
#define _OO_MAX_STICK_RANGE								( 1.0f / _MAX_STICK_RANGE )

#define _SLOWEST_SAMPLING_RATE_SUPPORTED				( 10 )
#define _FASTEST_SAMPLING_RATE_SUPPORTED				( 125 )

#define _DEADZONE_STICK_LEFT_NOT_CALIBRATED				( 0.45f )
#define _DEADZONE_STICK_LEFT_CALIBRATED					( 0.38f )
#define _DEADZONE_STICK_LEFT_CALIBRATION_THRESHOLD		( 0.70f )

#define _DEADZONE_STICK_RIGHT_NOT_CALIBRATED			( 12000.0f * _OO_MAX_STICK_RANGE )
#define _DEADZONE_STICK_RIGHT_CALIBRATED				( 9000.0f * _OO_MAX_STICK_RANGE )
#define _DEADZONE_STICK_RIGHT_CALIBRATION_THRESHOLD		( 0.70f )

// Nate: Old dead zone values
//#define _DEADZONE_STICK_RIGHT_NOT_CALIBRATED			( 0.45f )
//#define _DEADZONE_STICK_RIGHT_CALIBRATED				( 0.38f )
//#define _DEADZONE_STICK_RIGHT_CALIBRATION_THRESHOLD	( 0.70f )


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum
{
	_CALIBRATION_AXIS_STICK_LEFT_X_POS = 0,
	_CALIBRATION_AXIS_STICK_LEFT_X_NEG,
	_CALIBRATION_AXIS_STICK_LEFT_Y_POS,
	_CALIBRATION_AXIS_STICK_LEFT_Y_NEG,
	_CALIBRATION_AXIS_STICK_RIGHT_X_POS,
	_CALIBRATION_AXIS_STICK_RIGHT_X_NEG,
	_CALIBRATION_AXIS_STICK_RIGHT_Y_POS,
	_CALIBRATION_AXIS_STICK_RIGHT_Y_NEG,

	_CALIBRATION_AXIS_COUNT

} _AxesToCalibrate_e;

typedef enum
{
	_CALIBRATION_STATE_CALIBRATED					= 0x00000000,
	_CALIBRATION_STATE_LEFT_CALIBRATED				= 0x00000001,
	_CALIBRATION_STATE_LEFT_MOVED_PASS_THRESHOLD	= 0x00000010,
	_CALIBRATION_STATE_LEFT_NOT_CALIBRATED			= 0x00000100,
	_CALIBRATION_STATE_RIGHT_CALIBRATED				= 0x00010000,
	_CALIBRATION_STATE_RIGHT_MOVED_PASS_THRESHOLD	= 0x00100000,
	_CALIBRATION_STATE_RIGHT_NOT_CALIBRATED			= 0x01000000

} _CalibrationState_e;

typedef struct
{
	////
	//
	HANDLE hHandle;
	XINPUT_FEEDBACK oFeedback;
	FPadio_Sample_t oPreviousSamples;
	//
	////

	////
	//
	f32 fDeadzoneStickLeft, fDeadzoneStickRight;
	u32 oeCalibrationState;									// See _CalibrationState_e.
	BOOL abMovedPassedThreshold[ _CALIBRATION_AXIS_COUNT ];	// See _AxesToCalibrate_e.
	//
	////

} _Device_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Devices.
static _Device_t _aoDevices[ FPADIO_MAX_DEVICES ];
static FPadio_DeviceInfo_t _oDeviceInfo;
static DWORD _dwInserted, _dwRemoved;

// Double sample buffers.
static volatile FPadio_Sample_t **_papaoSamplesA;
static volatile FPadio_Sample_t **_papaoSamplesB;
static volatile FPadio_Sample_t ** volatile _papaoCurrentSamples;
static volatile u32 _uValidSamples;
static u32 _uMiliSecBetweenSamples, _uSamplesPerBuffer;

// Thread.
static HANDLE _hPollingThread, _hPollingThreadMutex, _hSamplesAndDevicesMutex;
static DWORD _dwPacketNumber;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static DWORD WINAPI _fpadio_SamplingThread( LPVOID pParam );
static void _fpadio_UpdateDevices( void );

static FINLINE void _RenormalizeStick( f32 &rfX, f32 &rfY, f32 fPosDz ) {
	f32 fMag2, fAngle, fMultipler, fSin, fCos;

	// fixup the right x,y axis
	fMag2 = ( rfX * rfX ) + ( rfY * rfY );
	if( fMag2 > 0.0f ) {
		// Get the angle that x y represents.
		fAngle = fmath_Atan( rfY, rfX );

		fmath_SinCos( fAngle, &fSin, &fCos );
		fMultipler = fmath_Inv( FMATH_MAX( FMATH_FABS( fSin ), FMATH_FABS( fCos ) ) );
		fMultipler *= fmath_Inv( fPosDz );

		rfX *= fMultipler;
		rfY *= fMultipler;

		// see if we need to clamp the length
		fMag2 = ( rfX * rfX ) + ( rfY * rfY );
		if( fMag2 > 1.0f ) {
			fMultipler = fmath_InvSqrt( fMag2 );
			rfX *= fMultipler;
			rfY *= fMultipler;
		}							
	}
}

static FINLINE void _ApplyDeadzone( f32 &rfValue, f32 fPosDZ ) {
	
	if( rfValue < -fPosDZ ) {
		rfValue += fPosDZ;
	} else if( rfValue > fPosDZ ) {
		rfValue -= fPosDZ;
	} else {
		rfValue = 0.0f;
	}
}

static FINLINE void _ApplyDeadzoneScaled( f32 &rfValue, f32 fPosDZ ) {
	if( rfValue < -fPosDZ ) {
		rfValue = (-1.0f * (rfValue + fPosDZ)) * fmath_Inv(-1.0f + fPosDZ);
	} else if( rfValue > fPosDZ ) {
		rfValue = ((rfValue - fPosDZ)) * fmath_Inv(1.0f - fPosDZ);
	} else {
		rfValue = 0.0f;
	}
}

#define _SCALE_X_VALUE ( 0.85f )
#define _SCALE_Y_VALUE ( 1.3f )

static FINLINE void _RenormalizeStickSquare( f32 &rfX, f32 &rfY, f32 fPosDz ) {
	f32 fMag2, fAngle, fMultipler, fSin, fCos;

	// fixup the right x,y axis
	fMag2 = ( rfX * rfX ) + ( rfY * rfY );
	if( fMag2 > 0.0f ) {
		// Get the angle that x y represents.
		fAngle = fmath_Atan( FMATH_FABS( rfY ), FMATH_FABS( rfX ) );

		fmath_SinCos( fAngle, &fSin, &fCos );
		fMultipler = fmath_Inv( FMATH_MAX( FMATH_FABS( fSin ), FMATH_FABS( fCos ) ) );
		//fMultipler *= _OO_MAX_STICK_RANGE;

		rfX *= fMultipler;
		rfY *= fMultipler;

		// see if we need to clamp the length
		fMag2 = ( rfX * rfX ) + ( rfY * rfY );
		if( fMag2 > 1.0f ) {
			fMultipler = fmath_InvSqrt( fMag2 );
			rfX *= fMultipler;
			rfY *= fMultipler;
		}							

		// x' = x / sqrt(1 - _SCALE_X_VALUE * (y * y))
		// y' = y * _SCALE_Y_VALUE

		rfX = rfX * fmath_InvSqrt( 1 - (_SCALE_X_VALUE * FMATH_SQUARE( rfY )) );
		rfY = rfY * _SCALE_Y_VALUE;

		if( FMATH_FABS( rfX ) >= 0.9f ) {
			rfX = FMATH_FSIGN( rfX );
		}

		if( FMATH_FABS( rfY ) >= 0.9f ) {
			rfY = FMATH_FSIGN( rfY );
		}
	}
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _fpadio_UpdateDevices( void )
{
	XINPUT_CAPABILITIES oTempDevCaps;
	XINPUT_POLLING_PARAMETERS oTempPollingParam;
	fang_MemZero( &oTempPollingParam, sizeof( oTempPollingParam ) );
	oTempPollingParam.fAutoPoll = TRUE;
	oTempPollingParam.bInputInterval = _uMiliSecBetweenSamples;
	_Device_t *poTempDevice;

	for( u32 uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
	{
		if( _dwRemoved & ( 1 << uIndex ) )
		{
			poTempDevice = &( _aoDevices[ uIndex ] );

			if( poTempDevice->hHandle )
			{
				XInputClose( poTempDevice->hHandle );
			}
			poTempDevice->hHandle = NULL;
		}

		if( _dwInserted & ( 1 << uIndex ) )
		{
			poTempDevice = &( _aoDevices[ uIndex ] );

			//// Open.
			//
			poTempDevice->hHandle = XInputOpen( XDEVICE_TYPE_GAMEPAD, uIndex, XDEVICE_NO_SLOT, &oTempPollingParam );

			if( ! poTempDevice->hHandle )
			{
				DEVPRINTF( "[ FPADIO ] Error %u: XInputOpen() failed for device #%u !!!\n", __LINE__, uIndex );
				continue;
			}
			//
			////

			//// Get capabilities.
			//
			if( ERROR_SUCCESS != XInputGetCapabilities( poTempDevice->hHandle, &oTempDevCaps ) )
			{
				if( poTempDevice->hHandle )
				{
					XInputClose( poTempDevice->hHandle );
				}
				poTempDevice->hHandle = NULL;
				DEVPRINTF( "[ FPADIO ] Error %u: GetCapabilities() failed for device #%u !!!\n", __LINE__, uIndex );
				continue;
			}

			if( ( XINPUT_DEVSUBTYPE_GC_GAMEPAD     != oTempDevCaps.SubType ) &&
				( XINPUT_DEVSUBTYPE_GC_GAMEPAD_ALT != oTempDevCaps.SubType ) )
			{
				if( poTempDevice->hHandle )
				{
					XInputClose( poTempDevice->hHandle );
				}
				poTempDevice->hHandle = NULL;
				continue;
			}
			//
			////

			////
			//

#if 0
			poTempDevice->fDeadzoneStickLeft  = _DEADZONE_STICK_LEFT_NOT_CALIBRATED;
			poTempDevice->fDeadzoneStickRight = _DEADZONE_STICK_RIGHT_NOT_CALIBRATED;
			poTempDevice->oeCalibrationState  = ( _CALIBRATION_STATE_LEFT_NOT_CALIBRATED | _CALIBRATION_STATE_RIGHT_NOT_CALIBRATED );
			for( u32 uTemp = 0; uTemp < _CALIBRATION_AXIS_COUNT; ++uTemp )
			{
				poTempDevice->abMovedPassedThreshold[ uTemp ] = FALSE;
			}
#else
			poTempDevice->fDeadzoneStickLeft  = _DEADZONE_STICK_LEFT_NOT_CALIBRATED;
			poTempDevice->fDeadzoneStickRight = _DEADZONE_STICK_RIGHT_NOT_CALIBRATED;
			poTempDevice->oeCalibrationState  = ( _CALIBRATION_STATE_CALIBRATED );
			for( u32 uTemp = 0; uTemp < _CALIBRATION_AXIS_COUNT; ++uTemp )
			{
				poTempDevice->abMovedPassedThreshold[ uTemp ] = FALSE;
			}
#endif
			//
			////
		}
	}

} // _fpadio_UpdateDevices

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fpadio_ModuleStartup( void )
{
	FASSERT_MSG( ! _bModuleStarted, "[ FPADIO ] Error: System already started !!!" );

	_bModuleStarted = TRUE;
	_bModuleInstalled = FALSE;

	return TRUE;

} // fpadio_ModuleStartup

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpadio_ModuleShutdown( void )
{
	fpadio_Uninstall();

	_bModuleStarted = FALSE;

} // fpadio_ModuleShutdown

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FPadio_Error_e fpadio_Install( const FPadio_Init_t *poInit /* = NULL */ )
{
	FASSERT_MSG( _bModuleStarted,        "[ FPADIO ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled,    "[ FPADIO ] Error: System already installed !!!" );

	//// Clamping Fang_ConfigDefs values.
	//
	FMATH_CLAMP( Fang_ConfigDefs.nPadio_MaxSamplesPerSec, _SLOWEST_SAMPLING_RATE_SUPPORTED, _FASTEST_SAMPLING_RATE_SUPPORTED );

	_uMiliSecBetweenSamples = ( 1000 / Fang_ConfigDefs.nPadio_MaxSamplesPerSec );
	//
	////

	////
	//
	_hPollingThreadMutex = CreateMutex( NULL, TRUE, NULL );
	if( ! _hPollingThreadMutex )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: CreateMutex() failed !!!\n", __LINE__ );
		return FPADIO_ERROR;
	}

	_hSamplesAndDevicesMutex = CreateMutex( NULL, FALSE, NULL );
	if( ! _hSamplesAndDevicesMutex )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: CreateMutex() failed !!!\n", __LINE__ );
		CloseHandle( _hPollingThreadMutex );
		return FPADIO_ERROR;
	}
	//
	////

	u32 uIndex;

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
		fres_ReleaseFrame( oTempResFrame );
		CloseHandle( _hPollingThreadMutex );
		CloseHandle( _hSamplesAndDevicesMutex );
		return FPADIO_ERROR;
	}

	for( uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
	{
		_papaoSamplesA[ uIndex ] = (volatile FPadio_Sample_t *)fres_AllocAndZero( sizeof( FPadio_Sample_t ) * _uSamplesPerBuffer );
		_papaoSamplesB[ uIndex ] = (volatile FPadio_Sample_t *)fres_AllocAndZero( sizeof( FPadio_Sample_t ) * _uSamplesPerBuffer );

		if( ! ( _papaoSamplesA[ uIndex ] && _papaoSamplesB[ uIndex ] ) )
		{
			DEVPRINTF( "[ FPADIO ] Error %u: Can't allocate memory for buffers !!!\n", __LINE__ );
			fres_ReleaseFrame( oTempResFrame );
			CloseHandle( _hPollingThreadMutex );
			CloseHandle( _hSamplesAndDevicesMutex );
			return FPADIO_ERROR;
		}
	}
	//
	////

	_papaoCurrentSamples = _papaoSamplesA;
	_uValidSamples = 0;
	//
	//// Allocate buffers memory.

	////
	//
	DWORD dwThreadID;
	_hPollingThread = CreateThread( NULL, 0, _fpadio_SamplingThread, NULL, CREATE_SUSPENDED, &dwThreadID );
	if( ! _hPollingThread )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: CreateThread() failed !!!\n", __LINE__ );
		fres_ReleaseFrame( oTempResFrame );
		CloseHandle( _hPollingThreadMutex );
		CloseHandle( _hSamplesAndDevicesMutex );
		return FPADIO_ERROR;
	}
	SetThreadPriority( _hPollingThread, THREAD_PRIORITY_ABOVE_NORMAL );
	//
	////

	fang_MemZero( _aoDevices, sizeof( _aoDevices ) );
	fang_MemZero( &_oDeviceInfo, sizeof( _oDeviceInfo ) );

	////
	//
	XINPUT_POLLING_PARAMETERS oTempPollingParam;
	fang_MemZero( &oTempPollingParam, sizeof( oTempPollingParam ) );
	oTempPollingParam.fAutoPoll = TRUE;
	oTempPollingParam.bInputInterval = _uMiliSecBetweenSamples;
	_dwPacketNumber = 0;
	//
	////

	////
	//
	fclib_strcpy( _oDeviceInfo.szName, "Xbox controller" );

	_oDeviceInfo.oeID = FPADIO_INPUT_XB_GAMEPAD;
	_oDeviceInfo.uInputs = FPADIO_INPUT_XB_ABUTTON_WHITE;

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
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_XB_DBUTTON_BACK - 1 ) ] = FPADIO_INPUT_XB_DBUTTON_BACK;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_XB_DBUTTON_STICK_LEFT - 1 ) ] = FPADIO_INPUT_XB_DBUTTON_STICK_LEFT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT - 1 ) ] = FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_XB_ABUTTON_BLACK - 1 ) ] = FPADIO_INPUT_XB_ABUTTON_BLACK;
	_oDeviceInfo.aeInputIDs[ ( FPADIO_INPUT_XB_ABUTTON_WHITE - 1 ) ] = FPADIO_INPUT_XB_ABUTTON_WHITE;
	//
	////

	////
	//
	_dwInserted = XGetDevices( XDEVICE_TYPE_GAMEPAD );
	XINPUT_CAPABILITIES oTempDevCaps;

	for( u32 uTemp = 0; uTemp < FPADIO_MAX_DEVICES; ++uTemp )
	{
		if( _dwInserted & ( 1 << uTemp ) )
		{
			//// Open.
			//
			_aoDevices[ uTemp ].hHandle = XInputOpen( XDEVICE_TYPE_GAMEPAD, uTemp, XDEVICE_NO_SLOT, &oTempPollingParam );

			if( ! _aoDevices[ uTemp ].hHandle )
			{
				DEVPRINTF( "[ FPADIO ] Error %u: XInputOpen() failed for device #%u !!!\n", __LINE__, uTemp );
				continue;
			}
			//
			////

			//// Get capabilities.
			//
			if( ERROR_SUCCESS != XInputGetCapabilities( _aoDevices[ uTemp ].hHandle, &oTempDevCaps ) )
			{
				if( _aoDevices[ uTemp ].hHandle )
				{
					XInputClose( _aoDevices[ uTemp ].hHandle );
				}
				_aoDevices[ uTemp ].hHandle = NULL;
				DEVPRINTF( "[ FPADIO ] Error %u: GetCapabilities() failed for device #%u !!!\n", __LINE__, uTemp );
				continue;
			}

			if( ( XINPUT_DEVSUBTYPE_GC_GAMEPAD     != oTempDevCaps.SubType ) &&
				( XINPUT_DEVSUBTYPE_GC_GAMEPAD_ALT != oTempDevCaps.SubType ) )
			{
				if( _aoDevices[ uTemp ].hHandle )
				{
					XInputClose( _aoDevices[ uTemp ].hHandle );
				}
				_aoDevices[ uTemp ].hHandle = NULL;
				continue;
			}
			//
			////

			////
			//
			_aoDevices[ uTemp ].fDeadzoneStickLeft  = _DEADZONE_STICK_LEFT_NOT_CALIBRATED;
			_aoDevices[ uTemp ].fDeadzoneStickRight = _DEADZONE_STICK_RIGHT_NOT_CALIBRATED;
			_aoDevices[ uTemp ].oeCalibrationState = ( _CALIBRATION_STATE_LEFT_NOT_CALIBRATED | _CALIBRATION_STATE_RIGHT_NOT_CALIBRATED );
			for( uIndex = 0; uIndex < _CALIBRATION_AXIS_COUNT; ++uIndex )
			{
				_aoDevices[ uTemp ].abMovedPassedThreshold[ uIndex ] = FALSE;
			}
			//
			////
		}
	}
	//
	////

	_bModuleInstalled = TRUE;

	ResumeThread( _hPollingThread );

	return FPADIO_NO_ERROR;

} // fpadio_Install

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpadio_Uninstall( void )
{
	if( ! _bModuleInstalled )
	{
		return;
	}

	ReleaseMutex( _hPollingThreadMutex );

	DWORD dwExitCode;
	do
	{
		Sleep( 10 );
		GetExitCodeThread( _hPollingThread, &dwExitCode );

	} while( STILL_ACTIVE == dwExitCode );

	for( u32 uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
	{
		if( _aoDevices[ uIndex ].hHandle )
		{
			XInputClose( _aoDevices[ uIndex ].hHandle );
		}
	}

	CloseHandle( _hPollingThreadMutex );
	CloseHandle( _hSamplesAndDevicesMutex );

	_bModuleInstalled = FALSE;

} // fpadio_Uninstall

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fpadio_IsInstalled( void )
{
	return _bModuleInstalled;

} // fpadio_IsInstalled

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpadio_SetForcefeedback_XB( u32 uDeviceIndex, f32 fUnitVibeCoarse, f32 fUnitVibeFine )
{
	FASSERT_MSG( _bModuleInstalled,                 "[ FPADIO ] Error: System not installed !!!" );
	FASSERT_MSG( FPADIO_MAX_DEVICES > uDeviceIndex, "[ FPADIO ] Error: Invalid device index !!!" );

	FMATH_CLAMP_UNIT_FLOAT( fUnitVibeCoarse );
	FMATH_CLAMP_UNIT_FLOAT( fUnitVibeFine   );

	XINPUT_RUMBLE *poRumble = &( _aoDevices[ uDeviceIndex ].oFeedback.Rumble );
	poRumble->wLeftMotorSpeed  = fmath_FloatToU32( fUnitVibeCoarse * 65535.0f );
	poRumble->wRightMotorSpeed = fmath_FloatToU32( fUnitVibeFine   * 65535.0f );

} // fpadio_SetForcefeedback_XB

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpadio_ApplyAllForcefeedbacks( void )
{
	FASSERT_MSG( _bModuleStarted, "[ FPADIO ] Error: System not started !!!" );

	if( _bModuleInstalled )
	{
		_Device_t *poDevice;

		for( u32 uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
		{
			FASSERT( ( uIndex >= 0 ) && ( uIndex < FPADIO_MAX_DEVICES ) );

			poDevice = &( _aoDevices[ uIndex ] );

			if( poDevice->hHandle )
			{
				XInputSetState( poDevice->hHandle, &( poDevice->oFeedback ) );
			}
		}
	}

} // fpadio_ApplyAllForcefeedbacks

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpadio_GetSamples( u32 *puValidSamples, FPadio_Sample_t ***ppapaoSamples )
{
	FASSERT_MSG( _bModuleInstalled, "[ FPADIO ] Error: System not installed !!!" );
	FASSERT_MSG( puValidSamples,    "[ FPADIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( ppapaoSamples,     "[ FPADIO ] Error: NULL pointer !!!" );

	WaitForSingleObject( _hSamplesAndDevicesMutex, INFINITE );

	*puValidSamples = _uValidSamples;
	_uValidSamples = 0;
	*ppapaoSamples = (FPadio_Sample_t **)_papaoCurrentSamples;
	_papaoCurrentSamples = ( ( _papaoSamplesA == _papaoCurrentSamples ) ? _papaoSamplesB : _papaoSamplesA );

	ReleaseMutex( _hSamplesAndDevicesMutex );

} // fpadio_GetSamples

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpadio_GetDeviceInfo( u32 uDeviceIndex, FPadio_DeviceInfo_t *poDeviceInfo )
{
	FASSERT_MSG( _bModuleInstalled,                 "[ FPADIO ] Error: System not installed !!!" );
	FASSERT_MSG( FPADIO_MAX_DEVICES > uDeviceIndex, "[ FPADIO ] Error: Invalid device index !!!" );
	FASSERT_MSG( poDeviceInfo,                      "[ FPADIO ] Error: NULL pointer !!!" );

	WaitForSingleObject( _hSamplesAndDevicesMutex, INFINITE );

	fang_MemCopy( poDeviceInfo, &( _oDeviceInfo ), sizeof( _oDeviceInfo ) );

	ReleaseMutex( _hSamplesAndDevicesMutex );

} // fpadio_GetDeviceInfo

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

DWORD WINAPI _fpadio_SamplingThread( LPVOID pParam )
{
	_Device_t *poTempDevice;
	FPadio_Sample_t *poTempSample;
	XINPUT_STATE oTempState;
	XINPUT_GAMEPAD *poTempGamepad;
	u32 uIndex;
	f32 fX, fY;

	do {
		WaitForSingleObject( _hSamplesAndDevicesMutex, INFINITE );

		if( XGetDeviceChanges( XDEVICE_TYPE_GAMEPAD, &_dwInserted, &_dwRemoved ) ) {
			_fpadio_UpdateDevices();
		}

		for( uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex ) {
			poTempSample = (FPadio_Sample_t *)&( _papaoCurrentSamples[ uIndex ][ _uValidSamples ] );
			poTempDevice = &( _aoDevices[ uIndex ] );

			if( poTempDevice->hHandle ) {
				//// Sample.
				if( ERROR_SUCCESS != XInputGetState( poTempDevice->hHandle, &oTempState ) ) {
					DEVPRINTF( "[ FPADIO ] Error %u: XInputGetState() failed for device #%u !!!\n", __LINE__, uIndex );
					poTempSample->bValid = FALSE;
					continue;
				}

				if( _dwPacketNumber == oTempState.dwPacketNumber ) {
					fang_MemCopy( poTempSample, &( poTempDevice->oPreviousSamples ), sizeof( poTempDevice->oPreviousSamples ) );
					continue;
				}

				_dwPacketNumber = oTempState.dwPacketNumber;
				poTempSample->bValid = TRUE;

				//// Inputs.
				poTempGamepad = &( oTempState.Gamepad );
				poTempSample->afInputValues[ ( FPADIO_INPUT_START - 1 ) ] = ( ( XINPUT_GAMEPAD_START & poTempGamepad->wButtons ) ? 1.0f : 0.0f );

				// RIGHT STICK
				fX = poTempGamepad->sThumbRX * _OO_MAX_STICK_RANGE;
				fY = poTempGamepad->sThumbRY * _OO_MAX_STICK_RANGE;

				_ApplyDeadzoneScaled( fX, poTempDevice->fDeadzoneStickRight );
				_ApplyDeadzoneScaled( fY, poTempDevice->fDeadzoneStickRight );
				_RenormalizeStickSquare( fX, fY, poTempDevice->fDeadzoneStickRight );

				// Nate: Old control method
				//_ApplyDeadzone( fX, poTempDevice->fDeadzoneStickRight );
				//_ApplyDeadzone( fY, poTempDevice->fDeadzoneStickRight );
				//_RenormalizeStick( fX, fY, poTempDevice->fDeadzoneStickRight );

				poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] = fX;
				poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] = fY;

				// LEFT STICK
				fX = poTempGamepad->sThumbLX * _OO_MAX_STICK_RANGE;
				_ApplyDeadzone( fX, poTempDevice->fDeadzoneStickLeft );

				fY = poTempGamepad->sThumbLY * _OO_MAX_STICK_RANGE;
				_ApplyDeadzone( fY, poTempDevice->fDeadzoneStickLeft );

				_RenormalizeStick( fX, fY, poTempDevice->fDeadzoneStickLeft );
				
				poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] = fX;
				poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] = fY;

				poTempSample->afInputValues[ ( FPADIO_INPUT_TRIGGER_LEFT  - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_LEFT_TRIGGER  ] ) * ( 1.0f / 255.0f ) );
				poTempSample->afInputValues[ ( FPADIO_INPUT_TRIGGER_RIGHT - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_RIGHT_TRIGGER ] ) * ( 1.0f / 255.0f ) );

				poTempSample->afInputValues[ ( FPADIO_INPUT_DPAD_X - 1 ) ]  = ( ( XINPUT_GAMEPAD_DPAD_LEFT  & poTempGamepad->wButtons ) ? -1.0f : 0.0f );
				poTempSample->afInputValues[ ( FPADIO_INPUT_DPAD_X - 1 ) ] += ( ( XINPUT_GAMEPAD_DPAD_RIGHT & poTempGamepad->wButtons ) ?  1.0f : 0.0f );

				poTempSample->afInputValues[ ( FPADIO_INPUT_DPAD_Y - 1 ) ]  = ( ( XINPUT_GAMEPAD_DPAD_UP   & poTempGamepad->wButtons ) ?  1.0f : 0.0f );
				poTempSample->afInputValues[ ( FPADIO_INPUT_DPAD_Y - 1 ) ] += ( ( XINPUT_GAMEPAD_DPAD_DOWN & poTempGamepad->wButtons ) ? -1.0f : 0.0f );

				poTempSample->afInputValues[ ( FPADIO_INPUT_CROSS_TOP    - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_Y ] ) * ( 1.0f / 255.0f ) );
				poTempSample->afInputValues[ ( FPADIO_INPUT_CROSS_BOTTOM - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_A ] ) * ( 1.0f / 255.0f ) );

				poTempSample->afInputValues[ ( FPADIO_INPUT_CROSS_LEFT  - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_X ] ) * ( 1.0f / 255.0f ) );
				poTempSample->afInputValues[ ( FPADIO_INPUT_CROSS_RIGHT - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_B ] ) * ( 1.0f / 255.0f ) );

				poTempSample->afInputValues[ ( FPADIO_INPUT_XB_DBUTTON_BACK        - 1 ) ] = ( ( XINPUT_GAMEPAD_BACK        & poTempGamepad->wButtons ) ? 1.0f : 0.0f );
				poTempSample->afInputValues[ ( FPADIO_INPUT_XB_DBUTTON_STICK_LEFT  - 1 ) ] = ( ( XINPUT_GAMEPAD_LEFT_THUMB  & poTempGamepad->wButtons ) ? 1.0f : 0.0f );
				poTempSample->afInputValues[ ( FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT - 1 ) ] = ( ( XINPUT_GAMEPAD_RIGHT_THUMB & poTempGamepad->wButtons ) ? 1.0f : 0.0f );

				poTempSample->afInputValues[ ( FPADIO_INPUT_XB_ABUTTON_BLACK - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_BLACK ] ) * ( 1.0f / 255.0f ) );
				poTempSample->afInputValues[ ( FPADIO_INPUT_XB_ABUTTON_WHITE - 1 ) ] = ( (f32)( poTempGamepad->bAnalogButtons[ XINPUT_GAMEPAD_WHITE ] ) * ( 1.0f / 255.0f ) );
				//
				//// Inputs.

				//// Calibration.
				//
				if( poTempDevice->oeCalibrationState )
				{
					//// Left stick.
					//
					if( _CALIBRATION_STATE_LEFT_NOT_CALIBRATED & poTempDevice->oeCalibrationState )
					{
						////
						//
						if( ( - _DEADZONE_STICK_LEFT_CALIBRATION_THRESHOLD ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] )					
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_X_NEG ] = TRUE;
						}
						if( _DEADZONE_STICK_LEFT_CALIBRATION_THRESHOLD < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] )
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_X_POS ] = TRUE;
						}
						if( ( - _DEADZONE_STICK_LEFT_CALIBRATION_THRESHOLD ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] )
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_Y_NEG ] = TRUE;
						}
						if( _DEADZONE_STICK_LEFT_CALIBRATION_THRESHOLD < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] )
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_Y_POS ] = TRUE;
						}
						//
						////

						////
						//
						if( poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_X_POS ] &&
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_X_NEG ] &&
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_Y_POS ] &&
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_LEFT_Y_NEG ] )
						{
							poTempDevice->oeCalibrationState &= ( ~ _CALIBRATION_STATE_LEFT_NOT_CALIBRATED );
							poTempDevice->oeCalibrationState |= _CALIBRATION_STATE_LEFT_MOVED_PASS_THRESHOLD;
						}
						//
						////
					}
					else if( _CALIBRATION_STATE_LEFT_MOVED_PASS_THRESHOLD & poTempDevice->oeCalibrationState )
					{
						////
						//
						if( ( ( + _DEADZONE_STICK_LEFT_CALIBRATED ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] ) &&
							( ( - _DEADZONE_STICK_LEFT_CALIBRATED ) < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] ) &&
							( ( + _DEADZONE_STICK_LEFT_CALIBRATED ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] ) &&
							( ( - _DEADZONE_STICK_LEFT_CALIBRATED ) < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] ) )
						{
							poTempDevice->fDeadzoneStickLeft  = _DEADZONE_STICK_LEFT_CALIBRATED;
							poTempDevice->oeCalibrationState &= ( ~ _CALIBRATION_STATE_LEFT_MOVED_PASS_THRESHOLD );
							poTempDevice->oeCalibrationState |= _CALIBRATION_STATE_LEFT_CALIBRATED;
						}
						//
						////
					}
					//
					//// Left stick.

					//// Right stick.
					//
					if( _CALIBRATION_STATE_RIGHT_NOT_CALIBRATED & poTempDevice->oeCalibrationState )
					{
						////
						//
						if( ( - _DEADZONE_STICK_RIGHT_CALIBRATION_THRESHOLD ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] )
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_X_NEG ] = TRUE;
						}
						if( _DEADZONE_STICK_RIGHT_CALIBRATION_THRESHOLD < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] )
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_X_POS ] = TRUE;
						}
						if( ( - _DEADZONE_STICK_RIGHT_CALIBRATION_THRESHOLD ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] )
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_Y_NEG ] = TRUE;
						}
						if( _DEADZONE_STICK_RIGHT_CALIBRATION_THRESHOLD < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] )
						{
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_Y_POS ] = TRUE;
						}
						//
						////

						////
						//
						if( poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_X_POS ] &&
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_X_NEG ] &&
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_Y_POS ] &&
							poTempDevice->abMovedPassedThreshold[ _CALIBRATION_AXIS_STICK_RIGHT_Y_NEG ] )
						{
							poTempDevice->oeCalibrationState &= ( ~ _CALIBRATION_STATE_RIGHT_NOT_CALIBRATED );
							poTempDevice->oeCalibrationState |= _CALIBRATION_STATE_RIGHT_MOVED_PASS_THRESHOLD;
						}
						//
						////
					}
					else if( _CALIBRATION_STATE_RIGHT_MOVED_PASS_THRESHOLD & poTempDevice->oeCalibrationState )
					{
						////
						//
						if( ( ( + _DEADZONE_STICK_RIGHT_CALIBRATED ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] ) &&
							( ( - _DEADZONE_STICK_RIGHT_CALIBRATED ) < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] ) &&
							( ( + _DEADZONE_STICK_RIGHT_CALIBRATED ) > poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] ) &&
							( ( - _DEADZONE_STICK_RIGHT_CALIBRATED ) < poTempSample->afInputValues[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] ) )
						{
							poTempDevice->fDeadzoneStickRight = _DEADZONE_STICK_RIGHT_CALIBRATED;
							poTempDevice->oeCalibrationState &= ( ~ _CALIBRATION_STATE_RIGHT_MOVED_PASS_THRESHOLD );
							poTempDevice->oeCalibrationState |= _CALIBRATION_STATE_RIGHT_CALIBRATED;
						}
						//
						////
					}
					//
					//// Right stick.

					if( ( _CALIBRATION_STATE_LEFT_CALIBRATED | _CALIBRATION_STATE_RIGHT_CALIBRATED ) == poTempDevice->oeCalibrationState )
					{
						poTempDevice->oeCalibrationState = _CALIBRATION_STATE_CALIBRATED;
					}
				}
				//
				//// Calibration.
			}
			else
			{
				poTempSample->bValid = FALSE;
			}

			fang_MemCopy( &( poTempDevice->oPreviousSamples ), poTempSample, sizeof( poTempDevice->oPreviousSamples ) );
		}

		if( _uValidSamples < ( _uSamplesPerBuffer - 1 ) )
		{
			++_uValidSamples;
		}

		ReleaseMutex( _hSamplesAndDevicesMutex );

	} while( WAIT_TIMEOUT == WaitForSingleObject( _hPollingThreadMutex, _uMiliSecBetweenSamples ) );

	return 0; // no error.

} // _fpadio_SamplingThread

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
