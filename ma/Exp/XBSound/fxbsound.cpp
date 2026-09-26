//////////////////////////////////////////////////////////////////////////////////////
// fxbsound.cpp - XBox Sound.
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
// 04/17/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _REMOVE_REDUNDANT_CHECKS		0

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if FANG_PLATFORM_XB

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _FSOUND_STUB_OUT	0

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "dsound.h"
#include "fsound.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/*
TODO
----

try playing an ADPCM (compressed) FILE!!!!!!!!!!!!

COMMENT EVERYTHING!!!!!!!!!!

strings functions( strcmp / strcpy / etc ) -> fang version

/ -> >>

vol+pan -> shift / add

remove commented code out!!
add #include fang.h into fsound.h
[core][voice] -> [voices] (one flat array) (*OR* *ONE* pointer to element!!) (*AND* *ADD* to reach NEXT ELEMENT OF ARRAY / STRUCT!!)
flink_list use MAYBE!!
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _FXBSOUND_MCP_VOLUME_MIN						DSBVOLUME_MIN
#define _FXBSOUND_MCP_VOLUME_MAX						DSBVOLUME_MAX

#define _FXBSOUND_CHANGE_ONOFF							( 1 << 0 )
#define _FXBSOUND_CHANGE_WITH_ENVELOPE					( 1 << 1 )
#define _FXBSOUND_CHANGE_PAUSEUNPAUSE					( 1 << 2 )
#define _FXBSOUND_CHANGE_VOLUME							( 1 << 3 )
#define _FXBSOUND_CHANGE_FREQUENCY						( 1 << 4 )
#define _FXBSOUND_CHANGE_ENVELOPE						( 1 << 5 )
#define _FXBSOUND_CHANGE_ADDRESS						( 1 << 6 )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	char szName[ FSOUND_MAX_UNIQUE_SOUND_NAME_LEN ];
	void *pSoundData;
	u32 uSize;
	BOOL bLoop;
	WAVEFORMATEX oFormat;
	DSBUFFERDESC oDescription;

} _FXBSoundDataInfo_t; // WAV sound sample info struct.

typedef struct
{
	LPDIRECTSOUNDBUFFER8 pDSoundBuff;
	DWORD uCurPos;

	//// Step ID of last ON and OFF.
	//
	u32 uStepLastOn;
	u32 uStepLastOff;
	//
	////

	//// New values to apply if dictated by uPendingChanges bits.
	//
	u8 uPendingChanges;

	u8 uVolume;
	u8 uPan;
	u16 uFrequency; // in Hz.
	BOOL bPause;
	u16 uPosToUnPause; // Value to restore when resuming play.
	u32 uEnvelope1; // 0: default envelope. Else: custom envelope.
	u32 uEnvelope2; // 0: default envelope. Else: custom envelope.
	FSoundID_t oSoundID; // -1: not used. Else: index in array of sounds.
	//
	////

} _FXBSoundCookieInfo_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static FSoundFrame_t _fxbsound_oFirstFreeSound; // Also used as frame index.
static BOOL _fxbsound_bHasBeenInitialized;
static u32 _fxbsound_uStep; // "Timer" / "Stepper".

static BOOL _fxbsound_bUpdateMasterVolume;
static u8 _fxbsound_uMasterVolume;

static LPDIRECTSOUND8 _fxbsound_pDSound; // DirectSound object

static _FXBSoundDataInfo_t _fxbsound_oaSounds[ FSOUND_MAX_UNIQUE_SOUNDS ];
static _FXBSoundCookieInfo_t _fxbsound_oaCookies[ FSOUND_MAX_VOICES ];

static s32 _fxbsound_naVolumes[ 256 ];

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static s32 _fxbsound_CookieHasBeenReleased( const FSoundCookie_t oID );

BOOL fsound_ModuleStartup( void );
void fsound_ModuleShutdown( void );

s32 fsound_GetAvailSoundBytes( void );
FSoundFrame_t fsound_FrameGet( void );
FSoundError_e fsound_FrameRelease( const FSoundFrame_t oID );
FSoundError_e fsound_SetMasterVolume( const u8 uVolume );
s32 fsound_GetMasterVolume( void );
FSoundError_e fsound_FlushCommands( void );
FSoundError_e _fsound_Update( void );
FSoundID_t fsound_SoundLoad( const char *pszName, const void *pData, const u32 uSize, u32 *pSampleRate, const BOOL bLoopingSample );
FSoundID_t fsound_SoundGetID( const char *pszName );
FSoundError_e fsound_SoundGetName( const FSoundID_t oID, char *pszName );
FSoundError_e fsound_SoundGetSampleRate( const FSoundID_t oID, u32 *pSampleRate );
FSoundCookie_t fsound_CookiePlay( const FSoundID_t oID, const u8 uVolume, const u8 uPan, const u16 uFrequency, const u32 uEnvelope );
s32 fsound_CookieIsPlaying( const FSoundCookie_t oID );
FSoundError_e fsound_CookieStopAndRelease( const FSoundCookie_t oID );
FSoundError_e fsound_CookieStopAndReleaseWithEnvelope( const FSoundCookie_t oID );
FSoundError_e fsound_CookiePause( const FSoundCookie_t oID, const BOOL bPause );
s32 fsound_CookieIsPaused( const FSoundCookie_t oID );
FSoundError_e fsound_CookieSetVolume( const FSoundCookie_t oID, const u8 uVolume );
s32 fsound_CookieGetVolume( const FSoundCookie_t oID );
FSoundError_e fsound_CookieSetPan( const FSoundCookie_t oID, const u8 uPan );
s32 fsound_CookieGetPan( const FSoundCookie_t oID );
FSoundError_e fsound_CookieSetFrequency( const FSoundCookie_t oID, const u16 uFrequency );
s32 fsound_CookieGetFrequency( const FSoundCookie_t oID );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fsound_ModuleStartup( void )
//
//	Description:
//		Initialize FSOUND.
//
//	Arguments:
//		None.
//
//	Return values:
//		FSOUND_NO_ERROR : Initialized correctly.
//		FSOUND_ERROR    : A problem occured while initializing.
//
//	Notes:
//		None.
//
{
#if( _FSOUND_STUB_OUT )
	return FALSE;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has already been initialized !!!\r\n", __LINE__ );
		return FALSE;
	}

//CHECK OTHER STUFF IS LOADED+INITIALIZED, LIKE DMA CHANNEL, RPC, SIF, ETC. AND MODULES!!??

#endif

	if( FAILED( DirectSoundCreate( NULL, &_fxbsound_pDSound, NULL ) ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] Error %d: DirectSoundCreate() failed !!!\r\n", __LINE__ );
		return FALSE;
	}

	fang_MemZero( _fxbsound_oaSounds, sizeof( _fxbsound_oaSounds ) );
	fang_MemZero( _fxbsound_oaCookies, sizeof( _fxbsound_oaCookies ) );
	fang_MemZero( _fxbsound_naVolumes, sizeof( _fxbsound_naVolumes ) );

	//// Pre-calculate table of volume.
	//
	double dTempVolume;
	for( u32 uIndex = 0; uIndex < 256; ++uIndex )
	{
		dTempVolume = 2000.0f * log10( (double)uIndex / (double)FSOUND_MAX_VOLUME );

		if( dTempVolume < DSBVOLUME_MIN )
		{
			dTempVolume = DSBVOLUME_MIN;
		}
		else if( dTempVolume > DSBVOLUME_MAX )
		{
			dTempVolume = DSBVOLUME_MAX;
		}

		_fxbsound_naVolumes[ uIndex ] = (s32)( dTempVolume );
	}
	//
	////

	for( u32 uIndex = 0; uIndex < ( sizeof( _fxbsound_oaCookies ) / sizeof( _FXBSoundCookieInfo_t ) ); ++uIndex )
	{
		_fxbsound_oaCookies[ uIndex ].oSoundID = -1;
		_fxbsound_oaCookies[ uIndex ].uStepLastOff = 1;
	}

	////
	// _fxbsound_uStep init to 2 because: uStepLastOn init to 0 and uStepLastOff init to 1 to indicate a OFF.
	// We want to start at the next step (2).
	_fxbsound_uStep = 2;
	//
	////

	_fxbsound_uMasterVolume = FSOUND_MAX_VOLUME;

	_fxbsound_oFirstFreeSound = 0;

	_fxbsound_bHasBeenInitialized = TRUE;

	return TRUE;

#endif

} // fsound_ModuleStartup

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fsound_ModuleShutdown( void )
//
//	Description:
//		Uninitialize FSOUND.
//
//	Arguments:
//		None.
//
//	Return values:
//		None.
//
//	Notes:
//		None.
//
{
#if ! ( _FSOUND_STUB_OUT )
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return;
	}
#endif

	for( u32 uIndex = 0; uIndex < ( sizeof( _fxbsound_oaCookies ) / sizeof( _FXBSoundCookieInfo_t ) ); ++uIndex )
	{
		if( _fxbsound_oaCookies[ uIndex ].pDSoundBuff )
		{
			if( ! _fxbsound_CookieHasBeenReleased( uIndex ) )
			{
//				_fxbsound_oaCookies[ uIndex ].pDSoundBuff->Stop( DSXB_STOP_IMMEDIATE );
				_fxbsound_oaCookies[ uIndex ].pDSoundBuff->Stop();
			}

			_fxbsound_oaCookies[ uIndex ].pDSoundBuff->Release();
		}
	}

	for( uIndex = 0; uIndex < ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ); ++uIndex )
	{
		if( _fxbsound_oaSounds[ uIndex ].pSoundData )
		{
			free( _fxbsound_oaSounds[ uIndex ].pSoundData );
		}
	}

	if( _fxbsound_pDSound )
	{
		_fxbsound_pDSound->Release();
	}

	_fxbsound_bHasBeenInitialized = FALSE;

#endif

} // fsound_ModuleShutdown

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_Install( FSoundInit_t *pInit )
{
	return FSOUND_NO_ERROR;

} // fsound_Install

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fsound_Uninstall( void )
{

} // fsound_Uninstall

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 fsound_GetAvailSoundBytes( void )
//
//	Description:
//		Get the size, in bytes, of the memory available for additional samples.
//
//	Arguments:
//		None.
//
//	Return values:
//		>= 0 : The size, in bytes, of the memory available for additional samples.
//
//	Notes:
//		None.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	MEMORYSTATUS oMemStatus;
	GlobalMemoryStatus( &oMemStatus );

	return oMemStatus.dwAvailPhys;

#endif

} // fsound_GetAvailSoundBytes

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundFrame_t fsound_FrameGet( void )
//
//	Description:
//		Get a sound frame start index.
//
//	Arguments:
//		None.
//
//	Return values:
//		FSOUND_ERROR : None available.
//		>= 0   : The frame index.
//
//	Notes:
//		None.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	if( ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ) <= _fxbsound_oFirstFreeSound )
	{
		return FSOUND_ERROR;
	}
	else
	{
		return _fxbsound_oFirstFreeSound;
	}

#endif

} // fsound_FrameGet

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_FrameRelease( const FSoundFrame_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( 0 > oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Frame ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ) <= oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Frame ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_oFirstFreeSound < oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Frame ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	if( _fxbsound_oFirstFreeSound == oID )
	{
		// Nothing to do.
		return FSOUND_NO_ERROR;
	}

	s32 uIndex, uEnd = ( ( ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ) > _fxbsound_oFirstFreeSound ) ? _fxbsound_oFirstFreeSound : ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ) );

	//// Stop all the voices using the freed sounds.
	//
	for( uIndex = 0; uIndex < ( sizeof( _fxbsound_oaCookies ) / sizeof( _FXBSoundCookieInfo_t ) ); ++uIndex )
	{
		if( ( _fxbsound_oaCookies[ uIndex ].oSoundID < oID ) || ( _fxbsound_oaCookies[ uIndex ].oSoundID >= uEnd ) ) continue;

		if( _fxbsound_oaCookies[ uIndex ].pDSoundBuff )
		{
			if( ! _fxbsound_CookieHasBeenReleased( uIndex ) )
			{
//				_fxbsound_oaCookies[ uIndex ].pDSoundBuff->Stop( DSXB_STOP_IMMEDIATE );
				_fxbsound_oaCookies[ uIndex ].pDSoundBuff->Stop();
			}

			_fxbsound_oaCookies[ uIndex ].pDSoundBuff->Release();
			_fxbsound_oaCookies[ uIndex ].pDSoundBuff = NULL;
			_fxbsound_oaCookies[ uIndex ].oSoundID = -1;
		}
	}

	for( uIndex = oID; uIndex < uEnd; ++uIndex )
	{
		if( _fxbsound_oaSounds[ uIndex ].pSoundData )
		{
			free( _fxbsound_oaSounds[ uIndex ].pSoundData );
			_fxbsound_oaSounds[ uIndex ].pSoundData = NULL;
		}
	}
	//
	////

	_fxbsound_oFirstFreeSound = oID;

	return FSOUND_NO_ERROR;

#endif

} // fsound_FrameRelease

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_SetMasterVolume( const u8 uVolume )
//
//	Description:
//		Set the master volume.
//
//	Arguments:
//		uVolume : The volume to set.
//
//	Return values:
//		FSOUND_NO_ERROR : Master volume assigned correctly.
//		FSOUND_ERROR    : A problem occured while assigning the master volume.
//
//	Notes:
//		uVolume is scaled internally to the xb's volume scale.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	_fxbsound_bUpdateMasterVolume = TRUE;
	_fxbsound_uMasterVolume       = uVolume;

	return FSOUND_NO_ERROR;

#endif

} // fsound_SetMasterVolume

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 fsound_GetMasterVolume( void )
//
//	Description:
//		Get the master volume.
//
//	Arguments:
//		None.
//
//	Return values:
//		The master volume.
//
//	Notes:
//		None.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	return _fxbsound_uMasterVolume;

#endif

} // fsound_GetMasterVolume

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_FlushCommands( void )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	return _fsound_Update();

#endif

} // fsound_FlushCommands

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e _fsound_Update( void )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	u32 uVoice = 0;
	s32 uMCPVolumeLeft, uMCPVolumeRight;
//	u32 uMCPEnvelope1, uMCPEnvelope2;

	////
	//
	if( _fxbsound_bUpdateMasterVolume )
	{
		uMCPVolumeLeft = ( _fxbsound_uMasterVolume * _FXBSOUND_MCP_VOLUME_MAX ) / FSOUND_MAX_VOLUME;
/*
		if( -1 == sceSdRemote( 1, rSdSetParam, SD_CORE_1 | SD_P_MVOLL, uMCPVolumeLeft ) )
		{
			DEVPRINTF( "*** [ FXBSOUND ] Error %d: Setting Master Vol Core 1 (L) !!!\r\n", __LINE__ );
			return FSOUND_ERROR;
		}

		if( -1 == sceSdRemote( 1, rSdSetParam, SD_CORE_1 | SD_P_MVOLR, uMCPVolumeLeft ) )
		{
			DEVPRINTF( "*** [ FXBSOUND ] Error %d: Setting Master Vol Core 1 (R) !!!\r\n", __LINE__ );
			return FSOUND_ERROR;
		}
*/
		_fxbsound_bUpdateMasterVolume = FALSE;
	}

	for( uVoice = 0; uVoice < ( sizeof( _fxbsound_oaCookies ) / sizeof( _FXBSoundCookieInfo_t ) ); ++uVoice )
	{
		if( ! _fxbsound_CookieHasBeenReleased( uVoice ) ) // ON
		{
			if( ! ( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_ONOFF ) )
			{
if( _fxbsound_oaCookies[ uVoice ].pDSoundBuff )
				_fxbsound_oaCookies[ uVoice ].pDSoundBuff->GetCurrentPosition( &_fxbsound_oaCookies[ uVoice ].uCurPos, NULL );
			}
		}

		if( ! _fxbsound_oaCookies[ uVoice ].uPendingChanges ) continue;

		if( _fxbsound_oaCookies[ uVoice ].uStepLastOn == _fxbsound_oaCookies[ uVoice ].uStepLastOff )
		{
			//// ON and OFF was issued during the same step -- ignore changes.
			//
			_fxbsound_oaCookies[ uVoice ].uPendingChanges = 0;
			_fxbsound_oaCookies[ uVoice ].uStepLastOn--;
			continue;
			//
			////
		}

		if( _fxbsound_CookieHasBeenReleased( uVoice ) )
		{
			if( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_ONOFF )
			{
//				_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Stop( ( ( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_WITH_ENVELOPE ) ? DSXB_STOP_ENVELOPE : DSXB_STOP_IMMEDIATE ) );
//				_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Stop( ( ( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_WITH_ENVELOPE ) ? DSBSTOPEX_ENVELOPE : 0 ) );
				_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Stop();

				_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Release();
				_fxbsound_oaCookies[ uVoice ].pDSoundBuff = NULL;
			}
		}
		else
		{
			if( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_ADDRESS )
			{
				if( DS_OK != _fxbsound_pDSound->CreateSoundBuffer( &_fxbsound_oaSounds[ _fxbsound_oaCookies[ uVoice ].oSoundID ].oDescription, &_fxbsound_oaCookies[ uVoice ].pDSoundBuff, NULL ) )
				{
					DEVPRINTF( "*** [ FXBSOUND ] Error %d: CreateSoundBuffer() failed for sound \"%s\"!!!\r\n", __LINE__, _fxbsound_oaSounds[ _fxbsound_oaCookies[ uVoice ].oSoundID ].szName );
					return FSOUND_ERROR;
				}

				if( DS_OK != _fxbsound_oaCookies[ uVoice ].pDSoundBuff->SetBufferData( _fxbsound_oaSounds[ _fxbsound_oaCookies[ uVoice ].oSoundID ].pSoundData, _fxbsound_oaSounds[ _fxbsound_oaCookies[ uVoice ].oSoundID ].uSize ) )
				{
					DEVPRINTF( "*** [ FXBSOUND ] Error %d: SetBufferData() failed for sound \"%s\"!!!\r\n", __LINE__, _fxbsound_oaSounds[ _fxbsound_oaCookies[ uVoice ].oSoundID ].szName );
					return FSOUND_ERROR;
				}
			}

			if( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_VOLUME )
			{
uMCPVolumeRight = uMCPVolumeLeft = _fxbsound_naVolumes[ _fxbsound_oaCookies[ uVoice ].uVolume ];

#if 0
char szDebugText[ 2048 ];
sprintf( szDebugText, "VOLUME: %d\r\n", uMCPVolumeRight );
OutputDebugString( szDebugText );
#endif

/*
				if( FSOUND_PAN_MIDDLE == _fxbsound_oaCookies[ uVoice ].uPan )
				{
					uMCPVolumeRight = uMCPVolumeLeft = ( ( _FXBSOUND_MCP_VOLUME_MAX * _fxbsound_oaCookies[ uVoice ].uVolume ) / FSOUND_MAX_VOLUME );
				}
				else if( FSOUND_PAN_MIDDLE > _fxbsound_oaCookies[ uVoice ].uPan )
				{
					uMCPVolumeLeft  = ( ( _FXBSOUND_MCP_VOLUME_MAX * _fxbsound_oaCookies[ uVoice ].uVolume ) / FSOUND_MAX_VOLUME );
					uMCPVolumeRight = ( ( _fxbsound_oaCookies[ uVoice ].uPan * ( ( _FXBSOUND_MCP_VOLUME_MAX * _fxbsound_oaCookies[ uVoice ].uVolume ) / FSOUND_MAX_VOLUME ) ) / ( FSOUND_PAN_MIDDLE - 1 ) );
				}
				else
				{
					uMCPVolumeLeft  = ( ( ( _FXBSOUND_MCP_VOLUME_MAX * _fxbsound_oaCookies[ uVoice ].uVolume ) / FSOUND_MAX_VOLUME ) - ( ( ( ( _FXBSOUND_MCP_VOLUME_MAX * _fxbsound_oaCookies[ uVoice ].uVolume ) / FSOUND_MAX_VOLUME ) * ( _fxbsound_oaCookies[ uVoice ].uPan - FSOUND_PAN_MIDDLE + 1 ) ) / ( FSOUND_MAX_PAN_RIGHT - FSOUND_PAN_MIDDLE + 1 ) ) );
					uMCPVolumeRight = ( ( _FXBSOUND_MCP_VOLUME_MAX * _fxbsound_oaCookies[ uVoice ].uVolume ) / FSOUND_MAX_VOLUME );
				}
*/
//TODO: control vol on left / right (panning) !!!!!!!!!!!!
				if( DS_OK != _fxbsound_oaCookies[ uVoice ].pDSoundBuff->SetVolume( uMCPVolumeLeft ) )
				{
					DEVPRINTF( "*** [ FXBSOUND ] Error %d: Can't set (L) volume to %d for voice %d !!!\r\n", __LINE__, uMCPVolumeLeft, uVoice );
					return FSOUND_ERROR;
				}
/*
				if( DS_OK != _fxbsound_oaCookies[ uVoice ].pDSoundBuff->SetVolume( uMCPVolumeRight ) )
				{
					DEVPRINTF( "*** [ FXBSOUND ] Error %d: Can't set (R) volume to %d for voice %d !!!\r\n", __LINE__, uMCPVolumeRight, uVoice );
					return FSOUND_ERROR;
				}
*/
			}

			if( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_FREQUENCY )
			{
				if( DS_OK != _fxbsound_oaCookies[ uVoice ].pDSoundBuff->SetFrequency( _fxbsound_oaCookies[ uVoice ].uFrequency ) )
				{
					DEVPRINTF( "*** [ FXBSOUND ] Error %d: Can't set pitch to %d for voice %d !!!\r\n", __LINE__, _fxbsound_oaCookies[ uVoice ].uFrequency, uVoice );
					return FSOUND_ERROR;
				}
			}

			if( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_ENVELOPE )
			{
				if( 0 == _fxbsound_oaCookies[ uVoice ].uEnvelope1 )
				{
//					uMCPEnvelope1 = ( ( FXBSOUND_MCP_ENVELOPE_SQUARE_1STHALF << 16 ) | FXBSOUND_MCP_ENVELOPE_SQUARE_2NDHALF ); // default
				}
				else
				{
//					uMCPEnvelope1 = ( ( FXBSOUND_MCP_ENVELOPE_SQUARE_1STHALF << 16 ) | FXBSOUND_MCP_ENVELOPE_SQUARE_2NDHALF );
				}

				if( 0 == _fxbsound_oaCookies[ uVoice ].uEnvelope2 )
				{
//					uMCPEnvelope2 = ( ( FXBSOUND_MCP_ENVELOPE_SQUARE_1STHALF << 16 ) | FXBSOUND_MCP_ENVELOPE_SQUARE_2NDHALF ); // default
				}
				else
				{
//					uMCPEnvelope2 = ( ( FXBSOUND_MCP_ENVELOPE_SQUARE_1STHALF << 16 ) | FXBSOUND_MCP_ENVELOPE_SQUARE_2NDHALF );
				}

//TODO: A LOT MORE NEEDS TO BE SETUP HERE BEFORE SetEG1/2() !!!!!!!!!!!!
/*
				if( DS_OK != _fxbsound_oaCookies[ uVoice ].pDSoundBuff->SetEG1( _fxbsound_oaCookies[ uVoice ].uEnvelope1 ) )
				{
					DEVPRINTF( "*** [ FXBSOUND ] Error %d: Can't set envelope1 to %d for voice %d !!!\r\n", __LINE__, uMCPEnvelope1, uVoice );
					return FSOUND_ERROR;
				}

				if( DS_OK != _fxbsound_oaCookies[ uVoice ].pDSoundBuff->SetEG2( _fxbsound_oaCookies[ uVoice ].uEnvelope2 ) )
				{
					DEVPRINTF( "*** [ FXBSOUND ] Error %d: Can't set envelope2 to %d for voice %d !!!\r\n", __LINE__, uMCPEnvelope2, uVoice );
					return FSOUND_ERROR;
				}
*/
			}

			if( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_PAUSEUNPAUSE )
			{
				if( _fxbsound_oaCookies[ uVoice ].bPause )
				{
					_fxbsound_oaCookies[ uVoice ].pDSoundBuff->GetCurrentPosition( &_fxbsound_oaCookies[ uVoice ].uCurPos, NULL );

//					_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Stop( DSXB_STOP_IMMEDIATE );
					_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Stop();
				}
				else
				{
					_fxbsound_oaCookies[ uVoice ].pDSoundBuff->SetCurrentPosition( _fxbsound_oaCookies[ uVoice ].uCurPos );
					_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Play( 0, 0, ( _fxbsound_oaSounds[ _fxbsound_oaCookies[ uVoice ].oSoundID ].bLoop ? DSBPLAY_LOOPING : 0 ) );
				}
			}

			if( _fxbsound_oaCookies[ uVoice ].uPendingChanges & _FXBSOUND_CHANGE_ONOFF )
			{
				_fxbsound_oaCookies[ uVoice ].pDSoundBuff->Play( 0, 0, ( _fxbsound_oaSounds[ _fxbsound_oaCookies[ uVoice ].oSoundID ].bLoop ? DSBPLAY_LOOPING : 0 ) );
			}
		}

		_fxbsound_oaCookies[ uVoice ].uPendingChanges = 0;
	}
	//
	////

	++_fxbsound_uStep;

	return FSOUND_NO_ERROR;

#endif

} // _fsound_Update

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundID_t fsound_SoundLoad( const char *pszName, const void *pData, const u32 uSize, u32 *pSampleRate, const BOOL bLoopingSample )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ! pszName )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: pszName is a NULL pointer !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( FSOUND_MAX_UNIQUE_SOUND_NAME_LEN <= strlen( pszName ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound name \"%s\" is more than %d characters !!!\r\n", __LINE__, pszName, FSOUND_MAX_UNIQUE_SOUND_NAME_LEN );
		return FSOUND_ERROR;
	}

	if( ! pData )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: pData is a NULL pointer !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ( *( (u32 *)pData + 0 ) != 'FFIR' ) ||
		( *( (u32 *)pData + 2 ) != 'EVAW' ) ||
		( *( (u32 *)pData + 3 ) != ' tmf' ) ||
		( *( (u32 *)pData + 4 ) != 0x0010 ) ||
		( *( (u32 *)pData + 9 ) != 'atad' ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound \"%s\" is not EXACTLY the supported type !!!\r\n", __LINE__, pszName );
		return FSOUND_ERROR;
	}

	if( FSOUND_ERROR == fsound_FrameGet() )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Can't load any more sounds !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ! uSize )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound \"%s\" is size 0 !!!\r\n", __LINE__, pszName );
		return FSOUND_ERROR;
	}

	for( u32 uIndex = 0; uIndex < FSOUND_MAX_UNIQUE_SOUNDS; ++uIndex )
	{
		if( 0 == strcmp( _fxbsound_oaSounds[ uIndex ].szName, pszName ) )
		{
			DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound \"%s\" has already been loaded !!!\r\n", __LINE__, pszName );
			return FSOUND_ERROR;
		}
	}
#endif

	if( (u32)fsound_GetAvailSoundBytes() < *( (u32 *)pData + 10 ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] Error %d: Available sound bytes: %d. Sound \"%s\" is size %d !!!\r\n", __LINE__, fsound_GetAvailSoundBytes(), pszName, *( (u32 *)pData + 10 ) );
		return FSOUND_ERROR;
	}

	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].pSoundData = malloc( *( (u32 *)pData + 10 ) );
	if( _fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].pSoundData == NULL )
	{
		DEVPRINTF( "*** [ FXBSOUND ] Error %d: Couldn't allocate %d bytes for sound data from \"%s\" !!!\r\n", __LINE__, *( (u32 *)pData + 10 ), pszName );
		return FSOUND_ERROR;
	}

	fang_MemCopy( _fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].pSoundData, (u32 *)pData + 11, *( (u32 *)pData + 10 ) );
	fang_MemCopy( &_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oFormat, (u32 *)pData + 5, 0x0010 );

	strcpy( _fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].szName, pszName );

	fang_MemZero( &_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oDescription, sizeof( _fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oDescription ) );

	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oDescription.dwSize = sizeof( DSBUFFERDESC );

//	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oDescription.dwFlags = DSBCAPS_LOCDEFER;

	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oDescription.lpwfxFormat = &_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oFormat;

// NOTE: Retail dsound will have problems if we pass in dwBufferBytes=0. So in retail we patch this to 1.
#if ( _REMOVE_REDUNDANT_CHECKS )
	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oDescription.dwBufferBytes = 1;
#else
	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oDescription.dwBufferBytes = 0;
#endif

	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].uSize = *( (u32 *)pData + 10 );

	_fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].bLoop = bLoopingSample;

	if( pSampleRate )
	{
		*pSampleRate = _fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].oFormat.nSamplesPerSec;
	}

//TEMP
char szDebugText[ 2048 ];
sprintf( szDebugText, "Loaded: \"%s\", Sampling rate: %d, Size: %d\r\n", pszName, *( (u32 *)pData + 6 ), _fxbsound_oaSounds[ _fxbsound_oFirstFreeSound ].uSize );
OutputDebugString( szDebugText );
//TEMP

	return _fxbsound_oFirstFreeSound++;

#endif

} // fsound_SoundLoad

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundID_t fsound_SoundGetID( const char *pszName )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ! pszName )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: pszName is a NULL pointer !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( FSOUND_MAX_UNIQUE_SOUND_NAME_LEN <= strlen( pszName ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound name \"%s\" is more than %d characters !!!\r\n", __LINE__, pszName, FSOUND_MAX_UNIQUE_SOUND_NAME_LEN );
		return FSOUND_ERROR;
	}
#endif

	for( u32 uSound = 0; uSound < FSOUND_MAX_UNIQUE_SOUNDS; ++uSound )
	{
		if( ( NULL != _fxbsound_oaSounds[ uSound ].pSoundData ) && ( 0 == strcmp( _fxbsound_oaSounds[ uSound ].szName, pszName ) ) )
		{
			return uSound;
		}
	}

	return FSOUND_ERROR;

#endif

} // fsound_SoundGetID

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_SoundGetName( const FSoundID_t oID, char *pszName )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( 0 > oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ) <= oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_oFirstFreeSound < oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ! pszName )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: pszName is a NULL pointer !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ! _fxbsound_oaSounds[ oID ].pSoundData )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	strcpy( pszName, _fxbsound_oaSounds[ oID ].szName );

	return FSOUND_NO_ERROR;

#endif

} // fsound_SoundGetName

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_SoundGetSampleRate( const FSoundID_t oID, u32 *pSampleRate )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( 0 > oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ) <= oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_oFirstFreeSound < oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ! pSampleRate )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: pSampleRate is a NULL pointer !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ! _fxbsound_oaSounds[ oID ].pSoundData )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	*pSampleRate = _fxbsound_oaSounds[ oID ].oFormat.nSamplesPerSec;

	return FSOUND_ERROR;

#endif

} // fsound_SoundGetSampleRate

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundCookie_t fsound_CookiePlay( const FSoundID_t oID, const u8 uVolume, const u8 uPan, const u16 uFrequency, const u32 uEnvelope, const float fStartPos/*= 0.0f*/ )
//
//	Description:
//		.
//
//	Arguments:
//		.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( 0 > oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ( sizeof( _fxbsound_oaSounds ) / sizeof( _FXBSoundDataInfo_t ) ) <= oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_oFirstFreeSound < oID )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Sound ID invalid !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	u32 uVoice = 0;
	u32 uStep = -1;
	FSoundCookie_t oCookie;
	BOOL bCookieFound = FALSE;

	//// Find the cookie that's been available the longest.
	//

// RESERVE COOKIES FOR STREAMING!!!!!!!!????????

	for( uVoice = 0; uVoice < ( sizeof( _fxbsound_oaCookies ) / sizeof( _FXBSoundCookieInfo_t ) ); ++uVoice )
	{
		if( ( _fxbsound_CookieHasBeenReleased( uVoice ) ) &&
			( uStep >= _fxbsound_oaCookies[ uVoice ].uStepLastOff ) )
		{
			if( 0 < ( _fxbsound_uStep - _fxbsound_oaCookies[ uVoice ].uStepLastOff ) )
			{
				uStep        = _fxbsound_oaCookies[ uVoice ].uStepLastOff;
				oCookie      = uVoice;
				bCookieFound = TRUE;
			}
		}
	}

	if( ! bCookieFound )
	{
		DEVPRINTF( "*** [ FXBSOUND ] Error %d: No cookies available to play !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
	//
	////

	////
	//
	_fxbsound_oaCookies[ oCookie ].oSoundID         = oID;
	_fxbsound_oaCookies[ oCookie ].uPendingChanges |= _FXBSOUND_CHANGE_ADDRESS;
	//
	////

	////
	//
	_fxbsound_oaCookies[ oCookie ].uVolume          = uVolume;
	_fxbsound_oaCookies[ oCookie ].uPendingChanges |= _FXBSOUND_CHANGE_VOLUME;
	//
	////

	////
	//
	_fxbsound_oaCookies[ oCookie ].uPan             = uPan;
	_fxbsound_oaCookies[ oCookie ].uPendingChanges |= _FXBSOUND_CHANGE_VOLUME;
	//
	////

	////
	//
	_fxbsound_oaCookies[ oCookie ].uFrequency       = uFrequency;
	_fxbsound_oaCookies[ oCookie ].uPendingChanges |= _FXBSOUND_CHANGE_FREQUENCY;
	//
	////

	////
	//
	_fxbsound_oaCookies[ oCookie ].uEnvelope1       = uEnvelope;
	_fxbsound_oaCookies[ oCookie ].uEnvelope2       = uEnvelope;
	_fxbsound_oaCookies[ oCookie ].uPendingChanges |= _FXBSOUND_CHANGE_ENVELOPE;
	//
	////

	//// ON.
	//
	_fxbsound_oaCookies[ oCookie ].uStepLastOn      = _fxbsound_uStep;
	_fxbsound_oaCookies[ oCookie ].uPendingChanges |= _FXBSOUND_CHANGE_ONOFF;
	_fxbsound_oaCookies[ oCookie ].uCurPos = -1;
	//
	////

	return oCookie;

#endif

} // fsound_CookiePlay

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 fsound_CookieIsPlaying( const FSoundCookie_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	return _fxbsound_oaCookies[ oID ].uCurPos != 0;

#endif

} // fsound_CookieIsPlaying

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 _fxbsound_CookieHasBeenReleased( const FSoundCookie_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}
#endif

	return ( _fxbsound_oaCookies[ oID ].uStepLastOff > _fxbsound_oaCookies[ oID ].uStepLastOn );

#endif

} // _fxbsound_CookieHasBeenReleased

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_CookieStopAndRelease( const FSoundCookie_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d has already been released !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	//// OFF.
	//
	_fxbsound_oaCookies[ oID ].uStepLastOff     = _fxbsound_uStep;
	_fxbsound_oaCookies[ oID ].uPendingChanges |= _FXBSOUND_CHANGE_ONOFF;
	_fxbsound_oaCookies[ oID ].uCurPos = 0;
	//
	////

	return FSOUND_NO_ERROR;

#endif

} // fsound_CookieStopAndRelease

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_CookieStopAndReleaseWithEnvelope( const FSoundCookie_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d has already been released !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	//// OFF.
	//
	_fxbsound_oaCookies[ oID ].uStepLastOff     = _fxbsound_uStep;
	_fxbsound_oaCookies[ oID ].uPendingChanges |= _FXBSOUND_CHANGE_ONOFF;
	_fxbsound_oaCookies[ oID ].uPendingChanges |= _FXBSOUND_CHANGE_WITH_ENVELOPE;
	_fxbsound_oaCookies[ oID ].uCurPos = 0;
	//
	////

	return FSOUND_NO_ERROR;

#endif

} // fsound_CookieStopAndReleaseWithEnvelope

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_CookiePause( const FSoundCookie_t oID, const BOOL bPause )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( ( bPause ) && ( fsound_CookieIsPaused( oID ) ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d already paused !!!\r\n", __LINE__, oID );
		return FSOUND_NO_ERROR;
	}

	if( ( ! bPause ) && ( ! fsound_CookieIsPaused( oID ) ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not paused !!!\r\n", __LINE__, oID );
		return FSOUND_NO_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	_fxbsound_oaCookies[ oID ].bPause = bPause;
	_fxbsound_oaCookies[ oID ].uPendingChanges |= _FXBSOUND_CHANGE_PAUSEUNPAUSE;

	return FSOUND_NO_ERROR;

#endif

} // fsound_CookiePause

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 fsound_CookieIsPaused( const FSoundCookie_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	return _fxbsound_oaCookies[ oID ].bPause;

#endif

} // fsound_CookieIsPaused

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_CookieSetVolume( const FSoundCookie_t oID, const u8 uVolume )
//
//	Description:
//		Set the Left / Right volume of a cookie (voice).
//
//	Arguments:
//		oID : The cookie ID.
//		uVolume : The volume to set.
//
//	Return values:
//		FSOUND_NO_ERROR : Volume assigned correctly.
//		FSOUND_ERROR    : A problem occured while assigning the volume.
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	_fxbsound_oaCookies[ oID ].uVolume          = uVolume;
	_fxbsound_oaCookies[ oID ].uPendingChanges |= _FXBSOUND_CHANGE_VOLUME;

	return FSOUND_NO_ERROR;

#endif

} // fsound_CookieSetVolume

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 fsound_CookieGetVolume( const FSoundCookie_t oID )
//
//	Description:
//		Get the volume of a cookie (voice).
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		FSOUND_NO_ERROR : Volume assigned correctly.
//		FSOUND_ERROR    : A problem occured while assigning the volume.
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	return _fxbsound_oaCookies[ oID ].uVolume;

#endif

} // fsound_CookieGetVolume

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_CookieSetPan( const FSoundCookie_t oID, const u8 uPan )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	_fxbsound_oaCookies[ oID ].uPan             = uPan;
	_fxbsound_oaCookies[ oID ].uPendingChanges |= _FXBSOUND_CHANGE_VOLUME;

	return FSOUND_NO_ERROR;

#endif

} // fsound_CookieSetPan

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 fsound_CookieGetPan( const FSoundCookie_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	return _fxbsound_oaCookies[ oID ].uPan;

#endif

} // fsound_CookieGetPan

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FSoundError_e fsound_CookieSetFrequency( const FSoundCookie_t oID, const u16 uFrequency )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		None.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	_fxbsound_oaCookies[ oID ].uFrequency       = uFrequency;
	_fxbsound_oaCookies[ oID ].uPendingChanges |= _FXBSOUND_CHANGE_FREQUENCY;

	return FSOUND_NO_ERROR;

#endif

} // fsound_CookieSetFrequency

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

s32 fsound_CookieGetFrequency( const FSoundCookie_t oID )
//
//	Description:
//		.
//
//	Arguments:
//		oID : The cookie ID.
//
//	Return values:
//		. : .
//
//	Notes:
//		None.
//
{
#if( _FSOUND_STUB_OUT )
	return FSOUND_ERROR;
#else
#if ! ( _REMOVE_REDUNDANT_CHECKS )
	if( ! _fxbsound_bHasBeenInitialized )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Has not been initialized !!!\r\n", __LINE__ );
		return FSOUND_ERROR;
	}

	if( _fxbsound_CookieHasBeenReleased( oID ) )
	{
		DEVPRINTF( "*** [ FXBSOUND ] BAD BAD BAD Error %d: Voice %d is not a valid cookie !!!\r\n", __LINE__, oID );
		return FSOUND_ERROR;
	}
#endif

	return _fxbsound_oaCookies[ oID ].uFrequency;

#endif

} // fsound_CookieGetFrequency

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
