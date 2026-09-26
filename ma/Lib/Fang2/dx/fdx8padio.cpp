//////////////////////////////////////////////////////////////////////////////////////
// fdx8padio.cpp - 
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
// 11/16/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <stdio.h>
#include <windows.h>
#include <dinput.h>

#include "fang.h"
#include "fdx8.h"
#include "fmath.h"
#include "fres.h"
#include "fclib.h"
#include "fpadio.h"



#define _AXIS_RANGE_MAX						1000
#define _OO_AXIS_RANGE_MAX					( 1.0f / ( (f32)( _AXIS_RANGE_MAX ) ) )

#define _SLOWEST_SAMPLING_RATE_SUPPORTED	10
#define _FASTEST_SAMPLING_RATE_SUPPORTED	90

#define _DEADZONE_STICKS					0.2f
#define _OO_ONE_MINUS_DEADZONE				( 1.0f / ( 1.0f - _DEADZONE_STICKS ) )


typedef struct {
	BOOL bEnumerated, bAcquired;
	LPDIRECTINPUTDEVICE8 poDIDevice;

	FPadio_DeviceInfo_t oInfo;

	u32 uAxis, uButtons;

	u8 auAxesOffset[ 8 ]; // Can't have more than 8 axes.
} _Device_t;

typedef struct {
	u32 uCurrentDeviceIndex, uAxisIndex;
} _EnumAxesCallbackParam_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Devices.
static _Device_t _aoDevices[ FPADIO_MAX_DEVICES ];

// Input Emulation.
static u32 _auInputEmulationPlatform[ FPADIO_MAX_DEVICES ], _auInputEmulationMap[ FPADIO_MAX_INPUTS ];

// Handles.
static LPDIRECTINPUT8 _pDInput;

// Double sample buffers.
static FPadio_Sample_t **_papaoSamplesA;
static FPadio_Sample_t **_papaoSamplesB;
static FPadio_Sample_t **_papaoCurrentSamples;
static u32 _uValidSamples;
static u32 _uMiliSecBetweenSamples, _uSamplesPerBuffer;

// Thread.
static HANDLE _hPollingThread, _hPollingThreadMutex, _hSamplesAndDevicesMutex;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static DWORD WINAPI _fpadio_SamplingThread( LPVOID pParam );
static BOOL CALLBACK _EnumDevicesCallback( LPCDIDEVICEINSTANCE pDevInst, LPVOID pParam );
static BOOL CALLBACK _EnumAxesCallback( LPCDIDEVICEOBJECTINSTANCE pDevInst, LPVOID pParam );


static FINLINE void _RenormalizeStick( f32 &rfX, f32 &rfY ) {
	f32 fMag2, fAngle, fMultipler, fSin, fCos;

	// fixup the right x,y axis
	fMag2 = ( rfX * rfX ) + ( rfY * rfY );
	if( fMag2 > 0.0f ) {
		// Get the angle that x y represents.
		fAngle = fmath_Atan( rfY, rfX );

		fmath_SinCos( fAngle, &fSin, &fCos );
		fMultipler = fmath_Inv( FMATH_MAX( FMATH_FABS( fSin ), FMATH_FABS( fCos ) ) );
		fMultipler *= _OO_ONE_MINUS_DEADZONE;

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

static FINLINE void _ApplyDeadzone( f32 &rfValue ) {
	
	if( rfValue < -_DEADZONE_STICKS ) {
		rfValue += _DEADZONE_STICKS;
	} else if( rfValue > _DEADZONE_STICKS ) {
		rfValue -= _DEADZONE_STICKS;
	} else {
		rfValue = 0.0f;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CALLBACK _EnumDevicesCallback( LPCDIDEVICEINSTANCE pDevInst, LPVOID pParam )
{
	u32 *puDeviceIndex = (u32 *)pParam;
	_Device_t *poTempDevice = &( _aoDevices[ *puDeviceIndex ] );

	if( FPADIO_MAX_DEVICES <= *puDeviceIndex )
	{
		return DIENUM_STOP;
	}

	if( FAILED( _pDInput->CreateDevice( pDevInst->guidInstance, &( poTempDevice->poDIDevice ), NULL ) ) )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: CreateDevice() failed for device #%u (\"%s\") !!!\n", __LINE__, *puDeviceIndex, pDevInst->tszInstanceName );
		return DIENUM_CONTINUE;
	}

	poTempDevice->bEnumerated = TRUE;

#ifdef _UNICODE
	WideCharToMultiByte( CP_THREAD_ACP, WC_NO_BEST_FIT_CHARS, pDevInst->tszInstanceName, _tcslen( pDevInst->tszInstanceName ), poTempDevice->oInfo.szName, sizeof( poTempDevice->oInfo.szName ), NULL, NULL );
#else
	fclib_strcpy( poTempDevice->oInfo.szName, pDevInst->tszInstanceName );
#endif

	poTempDevice->oInfo.oeID = FPADIO_INPUT_DX_GAMEPAD;

	++(*puDeviceIndex);

	return DIENUM_CONTINUE;

} // _EnumDevicesCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CALLBACK _EnumAxesCallback( LPCDIDEVICEOBJECTINSTANCE pDevInst, LPVOID pParam )
{
	_Device_t *poTempDevice = &( _aoDevices[ ((_EnumAxesCallbackParam_t *)pParam)->uCurrentDeviceIndex ] );
	u32 *puAxisIndex = &( ((_EnumAxesCallbackParam_t *)pParam)->uAxisIndex );

	//// Set axis range.
	//
	DIPROPRANGE oPropRange;

	oPropRange.diph.dwSize       = sizeof( DIPROPRANGE );
	oPropRange.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	oPropRange.diph.dwHow        = DIPH_BYID;
	oPropRange.diph.dwObj        = pDevInst->dwType;
	oPropRange.lMin              = - _AXIS_RANGE_MAX;
	oPropRange.lMax              = + _AXIS_RANGE_MAX;

	if( FAILED( poTempDevice->poDIDevice->SetProperty( DIPROP_RANGE, &oPropRange.diph ) ) )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: SetProperty() failed for axis #%u of device #%u !!!\n", __LINE__, *puAxisIndex, ((_EnumAxesCallbackParam_t *)pParam)->uCurrentDeviceIndex );
		return DIENUM_CONTINUE;
	}
	//
	////

	//// Get axis offset in DIJOYSTATE2.
	//
	if( pDevInst->guidType == GUID_XAxis )
	{
		poTempDevice->auAxesOffset[ *puAxisIndex ] = (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, lX ) ) >> 2 );
	}
	else if( pDevInst->guidType == GUID_YAxis )
	{
		poTempDevice->auAxesOffset[ *puAxisIndex ] = (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, lY ) ) >> 2 );
	}
	else if( pDevInst->guidType == GUID_ZAxis )
	{
		poTempDevice->auAxesOffset[ *puAxisIndex ] = (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, lZ ) ) >> 2 );
	}
	else if( pDevInst->guidType == GUID_RxAxis )
	{
		poTempDevice->auAxesOffset[ *puAxisIndex ] = (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, lRx ) ) >> 2 );
	}
	else if( pDevInst->guidType == GUID_RyAxis )
	{
		poTempDevice->auAxesOffset[ *puAxisIndex ] = (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, lRy ) ) >> 2 );
	}
	else if( pDevInst->guidType == GUID_RzAxis )
	{
		poTempDevice->auAxesOffset[ *puAxisIndex ] = (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, lRz ) ) >> 2 );
	}
	else if( pDevInst->guidType == GUID_Slider )
	{
		u32 uCount = 0;

		for( ; uCount < *puAxisIndex; ++uCount )
		{
			if( poTempDevice->auAxesOffset[ uCount ] == (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, rglSlider ) ) >> 2 ) )
			{
				poTempDevice->auAxesOffset[ *puAxisIndex ] = 1 + (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, rglSlider ) ) >> 2 );
				break;
			}
		}

		if( uCount == *puAxisIndex )
		{
			poTempDevice->auAxesOffset[ *puAxisIndex ] = (u8)( (u32)( FANG_OFFSETOF( DIJOYSTATE2, rglSlider ) ) >> 2 );
		}
	}
else
{
FASSERT_NOW_MSG( "THIS IS VERY BAD!!" );
}
	//
	////

	++( *puAxisIndex );

	return DIENUM_CONTINUE;

} // _EnumAxesCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
	FASSERT_MSG( _bModuleStarted,            "[ FPADIO ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled,        "[ FPADIO ] Error: System already installed !!!" );
	FASSERT_MSG( poInit,                     "[ FPADIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( poInit->DX8ONLY_ohInstance, "[ FPADIO ] Error: NULL handle !!!" );
	FASSERT_MSG( poInit->DX8ONLY_ohWnd,      "[ FPADIO ] Error: NULL handle !!!" );
	FASSERT_MSG( poInit->DX8ONLY_uInputEmulationPlatform <= FPADIO_INPUT_EMULATION_PLATFORM_GC,
											 "[ FPADIO ] Error: Platform ID invalid !!!" );

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
	_papaoSamplesA = (FPadio_Sample_t **)fres_Alloc( sizeof( FPadio_Sample_t * ) * FPADIO_MAX_DEVICES );
	_papaoSamplesB = (FPadio_Sample_t **)fres_Alloc( sizeof( FPadio_Sample_t * ) * FPADIO_MAX_DEVICES );
	if( ! ( _papaoSamplesA && _papaoSamplesB ) )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: Can't allocate memory for buffers !!!\n", __LINE__ );
		CloseHandle( _hPollingThreadMutex );
		CloseHandle( _hSamplesAndDevicesMutex );
		return FPADIO_ERROR;
	}

	for( uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
	{
		_papaoSamplesA[ uIndex ] = (FPadio_Sample_t *)fres_AllocAndZero( sizeof( FPadio_Sample_t ) * _uSamplesPerBuffer );
		_papaoSamplesB[ uIndex ] = (FPadio_Sample_t *)fres_AllocAndZero( sizeof( FPadio_Sample_t ) * _uSamplesPerBuffer );

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

	DWORD dwExitCode;

	////
	//
	if( FAILED( DirectInput8Create( (HINSTANCE)poInit->DX8ONLY_ohInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&_pDInput, NULL ) ) )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: DirectInput8Create() failed !!!\n", __LINE__ );
		ReleaseMutex( _hPollingThreadMutex );
		ResumeThread( _hPollingThread );
		do
		{
			Sleep( 10 );
			GetExitCodeThread( _hPollingThread, &dwExitCode );

		} while( STILL_ACTIVE == dwExitCode );
		CloseHandle( _hPollingThreadMutex );
		CloseHandle( _hSamplesAndDevicesMutex );
		fres_ReleaseFrame( oTempResFrame );
		return FPADIO_ERROR;
	}
	//
	////

	////
	//
	u32 uCount = 0;
	if( FAILED( _pDInput->EnumDevices( DI8DEVCLASS_GAMECTRL, _EnumDevicesCallback, &uCount, DIEDFL_ATTACHEDONLY ) ) )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: EnumDevices() failed !!!\n", __LINE__ );
		ReleaseMutex( _hPollingThreadMutex );
		ResumeThread( _hPollingThread );
		do
		{
			Sleep( 10 );
			GetExitCodeThread( _hPollingThread, &dwExitCode );

		} while( STILL_ACTIVE == dwExitCode );
		CloseHandle( _hPollingThreadMutex );
		CloseHandle( _hSamplesAndDevicesMutex );
		FDX8_SAFE_RELEASE( _pDInput );
		fres_ReleaseFrame( oTempResFrame );
		return FPADIO_ERROR;
	}

	if( ! uCount )
	{
		DEVPRINTF( "[ FPADIO ] Error %u: Nothing has been enumerated !!!\n", __LINE__ );
		ReleaseMutex( _hPollingThreadMutex );
		ResumeThread( _hPollingThread );
		do
		{
			Sleep( 10 );
			GetExitCodeThread( _hPollingThread, &dwExitCode );

		} while( STILL_ACTIVE == dwExitCode );
		CloseHandle( _hPollingThreadMutex );
		CloseHandle( _hSamplesAndDevicesMutex );
		FDX8_SAFE_RELEASE( _pDInput );
		fres_ReleaseFrame( oTempResFrame );
		return FPADIO_ERROR;
	}
	//
	////

	u32 uTemp;

	////
	//
	HWND hWnd = (HWND)poInit->DX8ONLY_ohWnd;

	DIDEVCAPS oDevCaps;
	oDevCaps.dwSize = sizeof( DIDEVCAPS );
	_EnumAxesCallbackParam_t oEnumAxesCallbackParam;

	for( uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
	{
		if( _aoDevices[ uIndex ].bEnumerated )
		{
			//// Gamepad data format.
			//
			if( FAILED( _aoDevices[ uIndex ].poDIDevice->SetDataFormat( &c_dfDIJoystick2 ) ) )
			{
				DEVPRINTF( "[ FPADIO ] Error %u: SetDataFormat() failed for device #%u !!!\n", __LINE__, uIndex );
				FDX8_SAFE_RELEASE( _aoDevices[ uIndex ].poDIDevice );
				_aoDevices[ uIndex ].bEnumerated = FALSE;
				continue;
			}
			//
			////

			//// Cooperative level.
			//
			if( FAILED( _aoDevices[ uIndex ].poDIDevice->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND ) ) )
			{
				DEVPRINTF( "[ FPADIO ] Error %u: SetCooperativeLevel() failed for device #%u !!!\n", __LINE__, uIndex );
				FDX8_SAFE_RELEASE( _aoDevices[ uIndex ].poDIDevice );
				_aoDevices[ uIndex ].bEnumerated = FALSE;
				continue;
			}
			//
			////

			//// Capabilities.
			//
			if( FAILED( _aoDevices[ uIndex ].poDIDevice->GetCapabilities( &oDevCaps ) ) )
			{
				DEVPRINTF( "[ FPADIO ] Error %u: GetCapabilities() failed for device #%u !!!\n", __LINE__, uIndex );
				FDX8_SAFE_RELEASE( _aoDevices[ uIndex ].poDIDevice );
				_aoDevices[ uIndex ].bEnumerated = FALSE;
				continue;
			}

			_aoDevices[ uIndex ].uAxis    = FMATH_MIN( oDevCaps.dwAxes, 6 ); // Use, at the most, the first 6 axes.
			_aoDevices[ uIndex ].uButtons = FMATH_MIN( oDevCaps.dwButtons, ( FPADIO_MAX_INPUTS - oDevCaps.dwAxes ) ); // Use, at the most, the first ( FPADIO_MAX_INPUTS - oDevCaps.dwAxes ) buttons.

			////
			//
			_aoDevices[ uIndex ].oInfo.uInputs = _aoDevices[ uIndex ].uAxis + _aoDevices[ uIndex ].uButtons;

			for( uTemp = 0; uTemp < _aoDevices[ uIndex ].uAxis; ++uTemp )
			{
				_aoDevices[ uIndex ].oInfo.aeInputIDs[ uTemp ] = FPADIO_INPUT_DX_AXIS;
			}

			for( ; uTemp < _aoDevices[ uIndex ].oInfo.uInputs; ++uTemp )
			{
				_aoDevices[ uIndex ].oInfo.aeInputIDs[ uTemp ] = FPADIO_INPUT_DX_BUTTON;
			}
			//
			////
			//
			//// Capabilities.

			oEnumAxesCallbackParam.uAxisIndex = 0;
			oEnumAxesCallbackParam.uCurrentDeviceIndex = uIndex;
			if( FAILED( _aoDevices[ uIndex ].poDIDevice->EnumObjects( _EnumAxesCallback, &oEnumAxesCallbackParam, DIDFT_AXIS ) ) )
			{
				DEVPRINTF( "[ FPADIO ] Error %u: EnumObjects() failed for device #%u !!!\n", __LINE__, uIndex );
				FDX8_SAFE_RELEASE( _aoDevices[ uIndex ].poDIDevice );
				_aoDevices[ uIndex ].bEnumerated = FALSE;
				continue;
			}

			//// Acquire.
			//
			if( FAILED( _aoDevices[ uIndex ].poDIDevice->Acquire() ) )
			{
				DEVPRINTF( "[ FPADIO ] Error %u: Acquire() failed for device #%u !!!\n", __LINE__, uIndex );
				FDX8_SAFE_RELEASE( _aoDevices[ uIndex ].poDIDevice );
				_aoDevices[ uIndex ].bEnumerated = FALSE;
				continue;
			}
			_aoDevices[ uIndex ].bAcquired = TRUE;
			//
			////
		}
	}
	//
	////

	////
	//
	for( uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex )
	{
		if( ( _aoDevices[ uIndex ].bAcquired ) &&
			( FPADIO_INPUT_EMULATION_PLATFORM_NONE != poInit->DX8ONLY_uInputEmulationPlatform ) &&
			( strstr( _aoDevices[ uIndex ].oInfo.szName, (char *)poInit->DX8ONLY_pszInputEmulationDevName ) ) )
		{
			FASSERT_MSG( poInit->DX8ONLY_pauInputEmulationMap,          "[ FPADIO ] Error: NULL pointer !!!" );
			FASSERT_MSG( poInit->DX8ONLY_pszInputEmulationDevName,      "[ FPADIO ] Error: NULL pointer !!!" );
			FASSERT_MSG( *( poInit->DX8ONLY_pszInputEmulationDevName ), "[ FPADIO ] Error: Zero length string !!!" );

			_auInputEmulationPlatform[ uIndex ] = poInit->DX8ONLY_uInputEmulationPlatform;

			if( FPADIO_INPUT_EMULATION_PLATFORM_XB == _auInputEmulationPlatform[ uIndex ] ) // XB
			{
				for( uTemp = 0; uTemp < 6; ++uTemp )
				{
					_auInputEmulationMap[ uTemp ] = poInit->DX8ONLY_pauInputEmulationMap[ uTemp + 24 ];
				}

				for( uTemp = 0; uTemp < 13; ++uTemp )
				{
					_auInputEmulationMap[ uTemp + 6 ] = poInit->DX8ONLY_pauInputEmulationMap[ uTemp ];
				}

				_aoDevices[ uIndex ].oInfo.uInputs = FPADIO_MAX_INPUTS;
			}
			else if( FPADIO_INPUT_EMULATION_PLATFORM_GC == _auInputEmulationPlatform[ uIndex ] ) // GC
			{
				for( uTemp = 0; uTemp < 6; ++uTemp )
				{
					_auInputEmulationMap[ uTemp ] = poInit->DX8ONLY_pauInputEmulationMap[ uTemp + 30 ];
				}

				for( uTemp = 0; uTemp < 11; ++uTemp )
				{
					_auInputEmulationMap[ uTemp + 6 ] = poInit->DX8ONLY_pauInputEmulationMap[ uTemp + 13 ];
				}

				_aoDevices[ uIndex ].oInfo.uInputs = ( FPADIO_MAX_INPUTS - 2 );
			}
			else
			{
				FASSERT_NOW_MSG( "[ FPADIO ] Error: Unsupported platform emulation specified !!!" );
			}
		}
		else
		{
			_auInputEmulationPlatform[ uIndex ] = FPADIO_INPUT_EMULATION_PLATFORM_NONE;
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
		if( _aoDevices[ uIndex ].bAcquired )
		{
			_aoDevices[ uIndex ].poDIDevice->Unacquire();
			FDX8_SAFE_RELEASE( _aoDevices[ uIndex ].poDIDevice );
		}
	}

	FDX8_SAFE_RELEASE( _pDInput );

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

void fpadio_ApplyAllForcefeedbacks( void )
{
	FASSERT_MSG( _bModuleInstalled, "[ FPADIO ] Error: System not installed !!!" );

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
	*ppapaoSamples = _papaoCurrentSamples;
	_papaoCurrentSamples = ( _papaoSamplesA == _papaoCurrentSamples ) ? _papaoSamplesB : _papaoSamplesA;

	ReleaseMutex( _hSamplesAndDevicesMutex );

} // fpadio_GetSamples

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fpadio_GetDeviceInfo( u32 uDeviceIndex, FPadio_DeviceInfo_t *poDeviceInfo )
{
	FASSERT_MSG( _bModuleInstalled,                 "[ FPADIO ] Error: System not installed !!!" );
	FASSERT_MSG( FPADIO_MAX_DEVICES > uDeviceIndex, "[ FPADIO ] Error: Invalid device index !!!" );
	FASSERT_MSG( poDeviceInfo,                      "[ FPADIO ] Error: NULL pointer !!!" );

	WaitForSingleObject( _hSamplesAndDevicesMutex, INFINITE );

	fang_MemCopy( poDeviceInfo, &( _aoDevices[ uDeviceIndex ].oInfo ), sizeof( _aoDevices[ uDeviceIndex ].oInfo ) );

	ReleaseMutex( _hSamplesAndDevicesMutex );

} // fpadio_GetDeviceInfo

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

DWORD WINAPI _fpadio_SamplingThread( LPVOID pParam ) {
	_Device_t *poTempDevice;
	FPadio_Sample_t *poTempSample;
	DIJOYSTATE2 oTempState;
	u32 uIndex, uTemp, uCount;
	f32 *pafInputValues, fX, fY;

	do {
		WaitForSingleObject( _hSamplesAndDevicesMutex, INFINITE );

		for( uIndex = 0; uIndex < FPADIO_MAX_DEVICES; ++uIndex ) {
			poTempSample = (FPadio_Sample_t *)&( _papaoCurrentSamples[ uIndex ][ _uValidSamples ] );
			poTempDevice = &( _aoDevices[ uIndex ] );
			
			if( poTempDevice->bAcquired ) {
				//// Sample.
				if( FAILED( poTempDevice->poDIDevice->Poll() ) ) {
					DEVPRINTF( "[ FPADIO ] Error %u: Poll() failed !!!\n", __LINE__ );
					poTempDevice->poDIDevice->Unacquire();
					FDX8_SAFE_RELEASE( poTempDevice->poDIDevice );
					poTempDevice->bAcquired = FALSE;
					poTempSample->bValid = FALSE;
					continue;
				}
				
				//// Get values.
				if( FAILED( poTempDevice->poDIDevice->GetDeviceState( sizeof( DIJOYSTATE2 ), &oTempState ) ) ) {
					DEVPRINTF( "[ FPADIO ] Error %u: GetDeviceState() failed !!!\n", __LINE__ );
					poTempDevice->poDIDevice->Unacquire();
					FDX8_SAFE_RELEASE( poTempDevice->poDIDevice );
					poTempDevice->bAcquired = FALSE;
					poTempSample->bValid = FALSE;
					continue;
				}
				
				poTempSample->bValid = TRUE;
				pafInputValues = poTempSample->afInputValues;


				/////////////
				// LEFT STICK
				/////////////

				// Left X
				if( _auInputEmulationMap[0] ) {
					fX = ( (f32)( ((s32 *)&oTempState)[ poTempDevice->auAxesOffset[ _auInputEmulationMap[0] - 1 ] ] ) * _OO_AXIS_RANGE_MAX );
					_ApplyDeadzone( fX );						
				} else {
					fX = 0.0f;
				}
				// Left Y
				if( _auInputEmulationMap[1] ) {
					fY = ( (f32)( ((s32 *)&oTempState)[ poTempDevice->auAxesOffset[ _auInputEmulationMap[1] - 1 ] ] ) * _OO_AXIS_RANGE_MAX );
					fY = -fY;												
					_ApplyDeadzone( fY );
				} else {
					fY = 0.0f;
				}
				// Fixup the left x,y axis
				_RenormalizeStick( fX, fY );
				// Record the values
				pafInputValues[ ( FPADIO_INPUT_STICK_LEFT_X - 1 ) ] = fX;
				pafInputValues[ ( FPADIO_INPUT_STICK_LEFT_Y - 1 ) ] = fY;


				///////////////
				// RIGHT STICK
				///////////////

				// Right X
				if( _auInputEmulationMap[2] ) {
					fX = ( (f32)( ((s32 *)&oTempState)[ poTempDevice->auAxesOffset[ _auInputEmulationMap[2] - 1 ] ] ) * _OO_AXIS_RANGE_MAX );
					_ApplyDeadzone( fX );						
				} else {
					fX = 0.0f;
				}
				// Right Y
				if( _auInputEmulationMap[3] ) {
					fY = ( (f32)( ((s32 *)&oTempState)[ poTempDevice->auAxesOffset[ _auInputEmulationMap[3] - 1 ] ] ) * _OO_AXIS_RANGE_MAX );
					fY = -fY;												
					_ApplyDeadzone( fY );
				} else {
					fY = 0.0f;
				}
				// Fixup the right x,y axis
				_RenormalizeStick( fX, fY );
				// Record the values
				pafInputValues[ ( FPADIO_INPUT_STICK_RIGHT_X - 1 ) ] = fX;
				pafInputValues[ ( FPADIO_INPUT_STICK_RIGHT_Y - 1 ) ] = fY;
			
				
				////////////
				// D PAD
				////////////
				
				// x
				if( _auInputEmulationMap[4] ) {
					fX = ( (f32)( ((s32 *)&oTempState)[ poTempDevice->auAxesOffset[ _auInputEmulationMap[4] - 1 ] ] ) * _OO_AXIS_RANGE_MAX );
					_ApplyDeadzone( fX );
					pafInputValues[ ( FPADIO_INPUT_DPAD_X - 1 ) ] = fX * _OO_ONE_MINUS_DEADZONE;
				} else {
					pafInputValues[ ( FPADIO_INPUT_DPAD_X - 1 ) ] = 0.0f;
				}
				// y
				if( _auInputEmulationMap[5] ) {
					fY = ( (f32)( ((s32 *)&oTempState)[ poTempDevice->auAxesOffset[ _auInputEmulationMap[5] - 1 ] ] ) * _OO_AXIS_RANGE_MAX );
					fY = -fY;
					_ApplyDeadzone( fY );
					pafInputValues[ ( FPADIO_INPUT_DPAD_Y - 1 ) ] = fY * _OO_ONE_MINUS_DEADZONE;
				}
				else
				{
					pafInputValues[ ( FPADIO_INPUT_DPAD_Y - 1 ) ] = 0.0f;
				}

				if( FPADIO_INPUT_EMULATION_PLATFORM_XB == _auInputEmulationPlatform[ uIndex ] ) {
					// XB

					// handle the buttons
					pafInputValues[ ( FPADIO_INPUT_RESET                  - 1 ) ] = _auInputEmulationMap[  6 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  6 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_START                  - 1 ) ] = _auInputEmulationMap[  7 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  7 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_XB_DBUTTON_BACK        - 1 ) ] = _auInputEmulationMap[  8 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  8 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_XB_ABUTTON_BLACK       - 1 ) ] = _auInputEmulationMap[  9 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  9 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_XB_ABUTTON_WHITE       - 1 ) ] = _auInputEmulationMap[ 10 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 10 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_BOTTOM           - 1 ) ] = _auInputEmulationMap[ 11 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 11 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_RIGHT            - 1 ) ] = _auInputEmulationMap[ 12 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 12 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_LEFT             - 1 ) ] = _auInputEmulationMap[ 13 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 13 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_TOP              - 1 ) ] = _auInputEmulationMap[ 14 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 14 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_TRIGGER_LEFT           - 1 ) ] = _auInputEmulationMap[ 15 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 15 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_TRIGGER_RIGHT          - 1 ) ] = _auInputEmulationMap[ 16 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 16 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_XB_DBUTTON_STICK_LEFT  - 1 ) ] = _auInputEmulationMap[ 17 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 17 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT - 1 ) ] = _auInputEmulationMap[ 18 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 18 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;

				} else if( FPADIO_INPUT_EMULATION_PLATFORM_GC == _auInputEmulationPlatform[ uIndex ] ) {
					// GC

					// handle the buttons
					pafInputValues[ ( FPADIO_INPUT_RESET                    - 1 ) ] = _auInputEmulationMap[  6 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  6 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_START                    - 1 ) ] = _auInputEmulationMap[  7 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  7 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_BOTTOM             - 1 ) ] = _auInputEmulationMap[  8 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  8 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_LEFT               - 1 ) ] = _auInputEmulationMap[  9 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[  9 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_RIGHT              - 1 ) ] = _auInputEmulationMap[ 10 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 10 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_CROSS_TOP                - 1 ) ] = _auInputEmulationMap[ 11 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 11 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z     - 1 ) ] = _auInputEmulationMap[ 12 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 12 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_TRIGGER_LEFT             - 1 ) ] = _auInputEmulationMap[ 13 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 13 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_TRIGGER_RIGHT            - 1 ) ] = _auInputEmulationMap[ 14 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 14 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT - 1 ) ] = _auInputEmulationMap[ 15 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 15 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;
					pafInputValues[ ( FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT  - 1 ) ] = _auInputEmulationMap[ 16 ] ? ( ( oTempState.rgbButtons[ _auInputEmulationMap[ 16 ] - 1 ] & 0x80 ) ? 1.0f : 0.0f ) : 0.0f;

				} else {
					// UNKNOWN PLATFORM

					//// Axes.
					for( uTemp = 0; uTemp < poTempDevice->uAxis; ++uTemp ) {
						pafInputValues[ uTemp ] = ( (f32)( ((s32 *)&oTempState)[ poTempDevice->auAxesOffset[ uTemp ] ] ) * _OO_AXIS_RANGE_MAX );
					}
					
					uCount = uTemp;

					//// Buttons.
					for( uTemp = 0; uTemp < poTempDevice->uButtons; ++uTemp ) {
						pafInputValues[ uCount + uTemp ] = ( oTempState.rgbButtons[ uTemp ] & 0x80 ) ? 1.0f : 0.0f;
					}
				}
			} else {
				poTempSample->bValid = FALSE;
			}
		}

		if( _uValidSamples < ( _uSamplesPerBuffer - 1 ) ) {
			++_uValidSamples;
		}

		ReleaseMutex( _hSamplesAndDevicesMutex );

	} while( WAIT_TIMEOUT == WaitForSingleObject( _hPollingThreadMutex, _uMiliSecBetweenSamples ) );

	return 0; // no error.

} // _fpadio_SamplingThread

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


