//////////////////////////////////////////////////////////////////////////////////////
// fdx8audio.cpp - 
//
// Author: Albert Yale
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
// 05/20/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fdx8.h"
#include "floop.h"
#include "faudio.h"
#include "fresload.h"
#include "fclib.h"
#include "ffile.h"
#include "floop.h"

#if FANG_WINGC
#include "gcaudio.h"
#endif

#include <dsound.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define _EMITTERS_LISTENERS_WORK_DELAY				( 0.10f ) //  10 fps.

#define _DSOUND_REAL_EMITTERS_MAX_3D				( 64 )
#define _DSOUND_REAL_EMITTERS_MAX_2D				( 188 )

// Does not include bank headers, but does include wave headers and wave data.
#define _MAX_TOTAL_WAVE_DATA_SIZE					( 2047 * ( 4 * 1024 ) )

#define _UNIQUE_VOLUME_LEVELS						( 128 )
#define _UNIQUE_FLOAT_VOL_LEVEL_INDICES				( (f32)( _UNIQUE_VOLUME_LEVELS - 1 ) )

#define _SIGNIFICANT_DISTANCE_CHANGE				( 0.001f )
#define _SIGNIFICANT_DISTANCE_CHANGE_SQ				( _SIGNIFICANT_DISTANCE_CHANGE * _SIGNIFICANT_DISTANCE_CHANGE )
#define _SIGNIFICANT_DOT_CHANGE						( 1.0f - 0.00001f ) //+- 2.562 degrees
#define _SIGNIFICANT_DOT_CHANGE_SQ					( _SIGNIFICANT_DOT_CHANGE * _SIGNIFICANT_DOT_CHANGE )
#define _SIGNIFICANT_FACTOR_CHANGE					( 0.001f )
#define _SIGNIFICANT_FACTOR_CHANGE_SQ				( _SIGNIFICANT_FACTOR_CHANGE * _SIGNIFICANT_FACTOR_CHANGE )
#define _SIGNIFICANT_VOLUME_CHANGE					( 1.0f / 127.0f )
#define _SIGNIFICANT_VOLUME_CHANGE_SQ				( _SIGNIFICANT_VOLUME_CHANGE * _SIGNIFICANT_VOLUME_CHANGE )

#if 1
#define _EMITTERS_CREATE_MAX_ERRORS					( 5 )
#else
#define _EMITTERS_CREATE_MAX_ERRORS					( 5000000 )
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

enum _EmitterProperties_e
{
	_EMITTER_PROPERTIES_3D					= 0x0001,
	_EMITTER_PROPERTIES_FIRENFORGET			= 0x0002,
	_EMITTER_PROPERTIES_AUTOPOSUPDATE 		= 0x0004,
	_EMITTER_PROPERTIES_DUCKABLE			= 0x0008,
	_EMITTER_PROPERTIES_LOOPING				= 0x0010,
	_EMITTER_PROPERTIES_IGNOREINPAUSEMODE	= 0x0020,
			
	_EMITTER_PROPERTIES_NONE				= 0x0000
}; // _EmitterProperties_e

enum _EmitterStateChange_e
{
	// Consumed by faudio_Work().
	_EMITTER_STATE_CHANGE_NONE			= 0x00000000,
	_EMITTER_STATE_CHANGE_PLAY			= 0x00000001,
	_EMITTER_STATE_CHANGE_STOP			= 0x00000002,
	_EMITTER_STATE_CHANGE_PAUSE			= 0x00000004,
	_EMITTER_STATE_CHANGE_UNPAUSE		= 0x00000008,
	_EMITTER_STATE_CHANGE_VOLUME		= 0x00000010,
	_EMITTER_STATE_CHANGE_FREQUENCY		= 0x00000020,
	_EMITTER_STATE_CHANGE_POSITION		= 0x00000040,
	_EMITTER_STATE_CHANGE_PAN			= 0x00000080,
//	_EMITTER_STATE_CHANGE_RADIUS		= 0x00000100,
	_EMITTER_STATE_CHANGE_DOPPLER		= 0x00000200,
	_EMITTER_STATE_CHANGE_REVERB		= 0x00000400,

	// Consumed by CFAudioEmitter::SetVolume().
	_EMITTER_STATE_CHANGE_DUCKABLE		= 0x00000800,
	_EMITTER_STATE_CHANGE_DUCKING		= 0x00001000

}; // _EmitterStateChange_e

enum _StreamStateChange_e
{
	// Consumed by faudio_Work().
	_STREAM_STATE_CHANGE_NONE			= 0x00000000,
	_STREAM_STATE_CHANGE_PLAY			= 0x00000001,
	_STREAM_STATE_CHANGE_STOP			= 0x00000002,
	_STREAM_STATE_CHANGE_PAUSE			= 0x00000004,
	_STREAM_STATE_CHANGE_UNPAUSE		= 0x00000008,
	_STREAM_STATE_CHANGE_VOLUME			= 0x00000010,
	_STREAM_STATE_CHANGE_FREQUENCY		= 0x00000020,
	_STREAM_STATE_CHANGE_PAN			= 0x00000040

}; // _StreamStateChange_e

enum _ListenerStateChange_e
{
	_LISTENER_STATE_CHANGE_NONE			= 0x00000000,
	_LISTENER_STATE_CHANGE_ORIENTATION  = 0x00000001

}; // _ListenerStateChange_e

enum _ListenerIntersection_e
{
	_LISTENER_INTERSECTION_NONE			= 0x00000000,
	_LISTENER_INTERSECTION_ENTERED		= 0x00000001,
	_LISTENER_INTERSECTION_EXITED		= 0x00000002,
	_LISTENER_INTERSECTION_SWITCHED		= 0x00000004,
	_LISTENER_INTERSECTION_PRESENT		= 0x00000008

}; // _ListenerIntersection_e

struct _VirtualEmitter_t;
struct _RealEmitter_t;

struct _VirtualListener_t
{
	CFWorldUser *poWorldUser;
	CFXfm *poXfmCurrentOrientation_WS;
	CFVec3A *poVecPreviousPosition_WS;	// Used for velocity / Doppler.
	CFVec3A *poVecVelocity_WS;			// Used for velocity / Doppler.

	u32 uStateChange; // See _ListenerStateChange_e.

}; // _VirtualListener_t

struct _RealEmitter_t
{
	LPDIRECTSOUNDBUFFER8 poDSBuffer;
	LPDIRECTSOUND3DBUFFER8 poDS3DBuffer;

	FLink_t oLink;

}; // _RealEmitter_t

struct _RealEmitterLimit_t
{
	u32 uPlaying, uPlayable;

}; // _RealEmitterLimit_t

struct _VirtualEmitter_t
{
	CFAudioEmitter *poAudioEmitter; // Derives from CFWorldUser.

	FAudio_WaveHandle_t oWaveHandle;

	_RealEmitter_t *poRealEmitter;
	CFAudioEmitter **ppUserAudioEmitter;	// NULL=none

	//// 3D only.
	//
	u32		uTimeStampCurrent,
			uTimeStampPrevious;

	_VirtualListener_t	*poVirtualListenerCurrent,
						*poVirtualListenerPrevious;

	_ListenerIntersection_e oeListenerIntersection;
	f32 fVirtualListenerDistanceSq;
	FLinkRoot_t *paoVirtualEmittersListActive; // Optimization.
	//
	////

	f32		fRadiusOuter,
			fRadiusInner,
			fVolume, fVolumeDucked,
			fDistanceGain,		// GameCube-layout 3D distance gain (< 0: not computed yet)
			fPanLeftRight,
			fFrequencyFactor, fDopplerFactor, fReverb,
			fSecondsPlayed, fSecondsToPlay;

	u32		uStateChanges;	// See _EmitterStateChange_e.

	u8 uPriority;
	u8 oeState;
	u8 uProperties;	// See _EmitterProperties_e.
	u8 uPauseLevel; //represents current pause mode level;

	CFVec3A			*poVecCurrentPosition_WS,
					*poVecPreviousPosition_WS;	// Used for velocity / Doppler.
	const CFVec3A 	*poVecAutoPosition_WS;		// Stores the pointer for an AutoUpdate3D sound  

	CFAudioEmitter::FAudio_EmitterEndOfPlayCallback_t *pEndOfPlayCallback;

	FLink_t oLink;

}; // _VirtualEmitter_t

// global vars
u8 FAudio_EmitterDefaultPriorityLevel = 0;
BOOL8 FAudio_bModuleStarted = FALSE;
BOOL8 FAudio_bModuleInstalled = FALSE;
BOOL8 FAudio_bMasterSfxVolChanged = FALSE;
BOOL8 FAudio_bMasterMusicVolChanged = FALSE;
f32 FAudio_fMasterSfxUnitVol = 1.0f;
f32 FAudio_fMasterMusicUnitVol = 1.0f;

// Wave banks.
static u32 _uSoundBytes, _uMaxSoundBytes, _uMaxBanks;
static FLinkRoot_t _oWaveBanksList;
static FResLoadReg_t _oLoadReg;

// Directsound.
static LPDIRECTSOUND8 _poDS;

#if FANG_WINGC
// The game's DirectSound device, for Bink to play movie audio through (fdx8movie2.cpp).
void *fdx8audio_GetDirectSound( void )
{
	return _poDS;
}
#endif

// Real Listeners.
static LPDIRECTSOUND3DLISTENER _poDSRealListener;

// Virtual Listeners.
static u32 _uMaxVirtualListeners, _uActiveVirtualListeners;
static _VirtualListener_t _aoVirtualListeners[ FAUDIO_MAX_LISTENERS ];

// Real Emitters.
static _RealEmitter_t _aoRealEmitters2D[ _DSOUND_REAL_EMITTERS_MAX_2D ];
static FLinkRoot_t _oRealEmittersListFree2D;
static FLinkRoot_t _oRealEmittersListActive2D;

static _RealEmitter_t _aoRealEmitters3D[ _DSOUND_REAL_EMITTERS_MAX_3D ];
static FLinkRoot_t _oRealEmittersListFree3D;
static FLinkRoot_t _oRealEmittersListActive3D;

static DS3DBUFFER _oDSEmitterAttributes;
static DSBUFFERDESC _oBufferDescription;

static _RealEmitterLimit_t *_paoRealEmittersLimits; // Pointer to an array of _uMaxPriorityLevels elements.

// Virtual Emitters.
static u32 _uMaxPriorityLevels; // !!Nate - This will only store an 8-bit value
static u32 _uMaxVirtualEmitters;

static _VirtualEmitter_t *_paoVirtualEmitters; // Pointer to an array of _uMaxVirtualEmitters elements.
static FLinkRoot_t _oVirtualEmittersListFree;
static FLinkRoot_t *_paoVirtualEmittersListActive2D; // Pointer to an array of _uMaxPriorityLevels elements. Each lists are sorted by volumes.
static FLinkRoot_t *_paoVirtualEmittersListActive3D; // Pointer to an array of _uMaxPriorityLevels elements. Each lists are sorted by volumes.

static FAudio_PauseLevel_e _ePauseLevelEmitters;
static f32 _fDuckingFactor;
static u32 _uEmittersCreateErrors2D, _uEmittersCreateErrors3D;

// Work.
static f32 _fEmittersListenersWorkDelay, _fOOEmittersListenersWorkDelay; // (use fangdef instead?) Should be replaced by a scheduler eventually to avoid running in synch with other low-fps work()!!!!!!!!!!
static BOOL _bSkipEmittersListenersWorkDelay;
static u32 _uTimeStamp;
static s32 _anVolumes[ _UNIQUE_VOLUME_LEVELS ]; // Optimization.

// Codec.
static HINSTANCE _ohCodecInstance;
static ACMDRIVERPROC _oCodecDriverProc;
static HACMDRIVERID _ohCodecDriverID;
static HACMDRIVER _ohCodecDriver;

// Temp.
static CFVec3A _oTempVec3A, _oTempVec3A_Velocity;
static CFSphere _oTempSphere;
static _VirtualListener_t *_poTempVirtualListener;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _BankLoadCallback( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _BankUnloadCallback( void *pResMem );
static BOOL _WorldLoadAndUnloadCallback( FWorldEvent_e oeEvent );
static void _AutoDestroyEmitterEndOfPlayCallback( CFAudioEmitter *poAudioEmitter );
static BOOL _EmitterIntersectionCallback( CFWorldTracker *poWorldTracker, FVisVolume_t *pVolume );
static void _TrackEmittersProgress( FLinkRoot_t *poVirtualEmittersListActive );
static void _ApplyRealEmittersChanges( FLinkRoot_t *poVirtualEmittersListActive );
static void _InvokeEmittersEndofplayCallbacks( FLinkRoot_t *poVirtualEmittersListActive );
static void _DestroyAllEmittersFromABank( FAudio_BankHandle_t hBank );
#if FANG_WINGC
static void _InitStreams( u32 uMaxStreams );
static void _StreamsWork( void );
#endif

// Unloads the Xbox ADPCM codec, if it was loaded, and the GameCube sound data.
static void _ReleaseCodec( void )
{
	if( _ohCodecDriverID )
	{
		acmDriverRemove( _ohCodecDriverID, 0 );
		_ohCodecDriverID = NULL;
	}
	if( _ohCodecInstance )
	{
		FreeLibrary( _ohCodecInstance );
		_ohCodecInstance = NULL;
	}
#if FANG_WINGC
	gcaudio_Uninit();
#endif
}

#if FANG_WINGC
// The retail GameCube mix, which the data was balanced for (gc/fgcaudio.cpp and MusyX):
// - Every volume handed to MusyX goes through fgcaudio's _GetVolume() curve first (effects: the
//   emitter volume after ducking and the master volume; streams: the stream volume), scaled by 0.8
//   for 3D effects and 0.6 for stereo streams (music), as a MIDI volume (x 127).
// - MusyX fades a 3D effect's MIDI volume linearly to silence at 1.25x the emitter radius
//   (sndAddEmitter comp 0).
// - MusyX turns MIDI volumes into amplitude through its DLS table (main.dol 0x3de80c: entry i =
//   (i/127)^2), so the amplitude is the square of all of the above.
// Full-volume music therefore plays at about 0.21 of full scale and full-volume speech and effects
// near 0.58; the data's balance between them depends on this.
#define _GC_3D_VOLUME_SCALE		( 0.80f )
#define _GC_3D_RADIUS_SCALE		( 1.25f )
#define _GC_STEREO_STREAM_SCALE	( 0.6f )

// fgcaudio.cpp's _GetVolume(): unit volume -> MusyX unit (MIDI / 127) volume.
static f32 _GCMusyxVolume( f32 fVolume )
{
	FMATH_CLAMP( fVolume, 0.0f, 1.0f );
	const f32 fRoot = fmath_Sqrt( fVolume );
	return 0.5f * 0.76f * ( fmath_Sqrt( fRoot ) + fRoot );
}

// MusyX's 3D fade of the MIDI volume, including the 0.8 3D scale.
static f32 _GC3DDistanceGain( f32 fDistance, f32 fRadiusOuter )
{
	const f32 fMaxDistance = fRadiusOuter * _GC_3D_RADIUS_SCALE;
	if( !( fMaxDistance > 0.0f ) || fDistance >= fMaxDistance )
	{
		return 0.0f;
	}
	return _GC_3D_VOLUME_SCALE * ( 1.0f - fDistance / fMaxDistance );
}

// A stream's amplitude.
static f32 _GCStreamGain( f32 fVolume, u32 uChannels )
{
	f32 fMidi = _GCMusyxVolume( fVolume );
	if( uChannels > 1 )
	{
		fMidi *= _GC_STEREO_STREAM_SCALE;
	}
	return fMidi * fMidi;
}

// DirectSound volume (hundredths of a decibel) for an amplitude gain.
// -mute (main_win.cpp): audio loads and runs as usual but every buffer is set to silence, so test runs
// exercise (and log) the whole sound system without being heard.
BOOL FAudio_bPortMuteOutput = FALSE;

static s32 _GainToDSVolume( f32 fGain )
{
	if( fGain <= 0.0001f || FAudio_bPortMuteOutput )
	{
		return DSBVOLUME_MIN;
	}
	s32 nVolume = (s32)( 2000.0 * log10( (double)fGain ) );
	FMATH_CLAMP( nVolume, DSBVOLUME_MIN, DSBVOLUME_MAX );
	return nVolume;
}
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL faudio_ModuleStartup( void )
{
	FASSERT_MSG( ( ! FAudio_bModuleStarted ), "[ FAUDIO ] Error: System already started !!!" );

	FAudio_bModuleStarted   = TRUE;
	FAudio_bModuleInstalled = FALSE;

	FAudio_bMasterSfxVolChanged = FALSE;
	FAudio_bMasterMusicVolChanged = FALSE;
	FAudio_fMasterSfxUnitVol = 1.0f;
	FAudio_fMasterMusicUnitVol = 1.0f;

	return TRUE;

} // faudio_ModuleStartup

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void faudio_ModuleShutdown( void )
{
	faudio_Uninstall();

	FAudio_bModuleStarted = FALSE;

} // faudio_ModuleShutdown

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_Error_e faudio_Install( const FAudio_Init_t *poInit )
{
	FASSERT_MSG( FAudio_bModuleStarted,                                                                                  "[ FAUDIO ] Error: System not started !!!" );
	FASSERT_MSG( ( ! FAudio_bModuleInstalled ),                                                                          "[ FAUDIO ] Error: System already installed !!!" );
	FASSERT_MSG( poInit,                                                                                           "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( ( poInit->uMaxListeners && ( FAUDIO_MAX_LISTENERS >= poInit->uMaxListeners ) ),                   "[ FAUDIO ] Error: Invalid uMaxListeners !!!" );
	FASSERT_MSG( poInit->uMaxEmitters,                                                                             "[ FAUDIO ] Error: Invalid uMaxEmitters !!!" );
	FASSERT_MSG( poInit->uMaxStreams,                                                                              "[ FAUDIO ] Error: Invalid uMaxStreams !!!" );
	FASSERT_MSG( Fang_ConfigDefs.nAudio_MaxSoundBytes,                                                            "[ FAUDIO ] Error: Invalid uMaxSoundBytes !!!" );
	FASSERT_MSG( poInit->uMaxPriorityLevels,                                                                       "[ FAUDIO ] Error: Invalid uMaxPriorityLevels !!!" );
	FASSERT_MSG( poInit->uMaxBanks,                                                                                "[ FAUDIO ] Error: Invalid uMaxBanks !!!" );
	FASSERT_MSG( ( poInit->uPriorityLimits ? ( !! poInit->paoPriorityLimits ) : ( ! poInit->paoPriorityLimits ) ), "[ FAUDIO ] Error: Invalid priority limits !!!" );
	FASSERT_MSG( poInit->DX8ONLY_ohWnd,                                                                            "[ FAUDIO ] Error: Invalid DX8ONLY_ohWnd !!!" );
	FASSERT_MSG( poInit->DX8ONLY_ohInstance,                                                                       "[ FAUDIO ] Error: Invalid DX8ONLY_ohInstance !!!" );

	////
	//
#if FANG_WINGC
	// GameCube wave banks are DSP-ADPCM, decoded by gcaudio instead of the Xbox ADPCM codec.
	if( ! gcaudio_Init() )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: The GameCube sound data could not be read !!!\n", __LINE__ );
		return FAUDIO_ERROR;
	}
#else
	_ohCodecInstance = LoadLibraryA( "xbadpcm.acm" );
	if( ! _ohCodecInstance )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: LoadLibrary() failed !!!\n", __LINE__ );
		return FAUDIO_ERROR;
	}

	_oCodecDriverProc = (ACMDRIVERPROC)GetProcAddress( _ohCodecInstance, "DriverProc" );
	if( ! _oCodecDriverProc )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: GetProcAddress() failed !!!\n", __LINE__ );
		FreeLibrary( _ohCodecInstance );
		return FAUDIO_ERROR;
	}

	if( 0 != acmDriverAdd( &_ohCodecDriverID, _ohCodecInstance, (LPARAM)_oCodecDriverProc, 0, ( ACM_DRIVERADDF_FUNCTION | ACM_DRIVERADDF_LOCAL ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: acmDriverAdd() failed !!!\n", __LINE__ );
		FreeLibrary( _ohCodecInstance );
		return FAUDIO_ERROR;
	}

	if( 0 != acmDriverOpen( &_ohCodecDriver, _ohCodecDriverID, 0 ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: acmDriverOpen() failed !!!\n", __LINE__ );
		_ReleaseCodec();
		return FAUDIO_ERROR;
	}
#endif
	//
	////

	////
	//
	if( FAILED( DirectSoundCreate8( NULL, &_poDS, NULL ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: DirectSoundCreate8() failed !!!\n", __LINE__ );
		_ReleaseCodec();
		return FAUDIO_ERROR;
	}

	if( FAILED( _poDS->SetCooperativeLevel( (HWND)poInit->DX8ONLY_ohWnd, DSSCL_PRIORITY ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: SetCooperativeLevel() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDS );
		_ReleaseCodec();
		return FAUDIO_ERROR;
	}

	LPDIRECTSOUNDBUFFER poDSBufferPrimary;

	fang_MemZero( &_oBufferDescription, sizeof( _oBufferDescription ) );
	_oBufferDescription.dwSize  = sizeof( _oBufferDescription );
	_oBufferDescription.dwFlags = ( DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER );

	if( FAILED( _poDS->CreateSoundBuffer( &_oBufferDescription, &poDSBufferPrimary, NULL ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: CreateSoundBuffer() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDS );
		_ReleaseCodec();
		return FAUDIO_ERROR;
	}

	WAVEFORMATEX oWaveformatex;

	fang_MemZero( &oWaveformatex, sizeof( oWaveformatex ) );
	oWaveformatex.wFormatTag      = WAVE_FORMAT_PCM;
	oWaveformatex.nChannels       = 2;
	oWaveformatex.nSamplesPerSec  = 44100;
	oWaveformatex.wBitsPerSample  = 16;
	oWaveformatex.nBlockAlign     = 4;
	oWaveformatex.nAvgBytesPerSec = 176400;

	if( FAILED( poDSBufferPrimary->SetFormat( &oWaveformatex ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: SetFormat() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( poDSBufferPrimary );
		FDX8_SAFE_RELEASE( _poDS );
		_ReleaseCodec();
		return FAUDIO_ERROR;
	}

	if( FAILED( poDSBufferPrimary->QueryInterface( IID_IDirectSound3DListener, (void **)&_poDSRealListener ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: SetFormat() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( poDSBufferPrimary );
		FDX8_SAFE_RELEASE( _poDS );
		_ReleaseCodec();
		return FAUDIO_ERROR;
	}

	FDX8_SAFE_RELEASE( poDSBufferPrimary );
	_poDSRealListener->SetDistanceFactor( FMATH_FEET2METERS( 1.0f ), DS3D_IMMEDIATE );
#if FANG_WINGC
	// Distance attenuation follows MusyX instead (see _GC3DDistanceGain); DirectSound only pans.
	_poDSRealListener->SetRolloffFactor( DS3D_MINROLLOFFFACTOR, DS3D_IMMEDIATE );
#endif
	//
	////

	//// Register .Wvb loader.
	//
	if( ! fresload_IsRegistered( FAUDIOBANK_RESTYPE ) )
	{
		fang_MemZero( &_oLoadReg, sizeof( _oLoadReg ) );
		fres_CopyType( _oLoadReg.sResType, FAUDIOBANK_RESTYPE );
#if FANG_WINGC
		_oLoadReg.pszFileExtension = "rdg";
#else
		_oLoadReg.pszFileExtension = "wvb";
#endif
		_oLoadReg.nMemType         = FRESLOAD_MEMTYPE_PERM;
		_oLoadReg.nAlignment       = 16;
		_oLoadReg.pFcnCreate       = _BankLoadCallback;
		_oLoadReg.pFcnDestroy      = _BankUnloadCallback;

		if( ! fresload_RegisterHandler( &_oLoadReg ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: fresload_RegisterHandler() failed !!!\n", __LINE__ );
			FDX8_SAFE_RELEASE( _poDSRealListener );
			FDX8_SAFE_RELEASE( _poDS );
			_ReleaseCodec();
			return FAUDIO_ERROR;
		}
	}
	//
	////

	//// Init static values.
	//
	_uTimeStamp                      = 10;
	_ePauseLevelEmitters              = FAUDIO_PAUSE_LEVEL_NONE;
	_fDuckingFactor                  = 1.0f;
	_fEmittersListenersWorkDelay     = 0.0f;
	_fOOEmittersListenersWorkDelay   = 0.0f;
	_bSkipEmittersListenersWorkDelay = TRUE;
	_uSoundBytes                     = 0;
	_uEmittersCreateErrors2D         = _EMITTERS_CREATE_MAX_ERRORS;
	_uEmittersCreateErrors3D         = _EMITTERS_CREATE_MAX_ERRORS;
	_uMaxVirtualListeners            = poInit->uMaxListeners;
	_uMaxVirtualEmitters             = poInit->uMaxEmitters;
	_uMaxPriorityLevels              = poInit->uMaxPriorityLevels;
	_uMaxSoundBytes                  = Fang_ConfigDefs.nAudio_MaxSoundBytes;
	_uMaxBanks                       = poInit->uMaxBanks;
#if FANG_WINGC
	_InitStreams( poInit->uMaxStreams );
#endif

	if( _MAX_TOTAL_WAVE_DATA_SIZE < _uMaxSoundBytes )
	{
		DEVPRINTF( "[ FAUDIO ] Warning %u: Clamping max sound bytes to %u.\n", __LINE__, _MAX_TOTAL_WAVE_DATA_SIZE );
		_uMaxSoundBytes = _MAX_TOTAL_WAVE_DATA_SIZE;
	}

	if( ! fworld_IsWorldCallbackFunctionRegistered( _WorldLoadAndUnloadCallback ) )
	{
		fworld_RegisterWorldCallbackFunction( _WorldLoadAndUnloadCallback );
	}
	//
	////

	//// Set default DS emitter attributes.
	//
	fang_MemZero( &_oDSEmitterAttributes, sizeof( _oDSEmitterAttributes ) );
	_oDSEmitterAttributes.dwSize             = sizeof( _oDSEmitterAttributes );
	_oDSEmitterAttributes.vPosition.x        = 0.0f;
	_oDSEmitterAttributes.vPosition.y        = 0.0f;
	_oDSEmitterAttributes.vPosition.z        = 0.0f;
	_oDSEmitterAttributes.vVelocity.x        = 0.0f;
	_oDSEmitterAttributes.vVelocity.y        = 0.0f;
	_oDSEmitterAttributes.vVelocity.z        = 0.0f;
	_oDSEmitterAttributes.dwInsideConeAngle  = DS3D_DEFAULTCONEANGLE;
	_oDSEmitterAttributes.dwOutsideConeAngle = DS3D_DEFAULTCONEANGLE;
	_oDSEmitterAttributes.vConeOrientation.x = 0.0f;
	_oDSEmitterAttributes.vConeOrientation.y = 0.0f;
	_oDSEmitterAttributes.vConeOrientation.z = 1.0f;
	_oDSEmitterAttributes.lConeOutsideVolume = DS3D_DEFAULTCONEOUTSIDEVOLUME;
	_oDSEmitterAttributes.flMinDistance      = DS3D_DEFAULTMINDISTANCE;
	_oDSEmitterAttributes.flMaxDistance      = DS3D_DEFAULTMAXDISTANCE;
	_oDSEmitterAttributes.dwMode             = DS3DMODE_HEADRELATIVE;
	//
	////

	//// Init arrays and lists.
	//
	// Virtual listeners.
	fang_MemZero( _aoVirtualListeners, sizeof( _aoVirtualListeners ) );

	// Virtual emitters.
	flinklist_InitRoot( &_oVirtualEmittersListFree, (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );

	// Real emitters.
	fang_MemZero( _aoRealEmitters2D, sizeof( _aoRealEmitters2D ) );
	flinklist_InitRoot( &_oRealEmittersListFree2D, (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );
	flinklist_InitRoot( &_oRealEmittersListActive2D, (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );

	fang_MemZero( _aoRealEmitters3D, sizeof( _aoRealEmitters3D ) );
	flinklist_InitRoot( &_oRealEmittersListFree3D, (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );
	flinklist_InitRoot( &_oRealEmittersListActive3D, (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );

	// Banks.
	flinklist_InitRoot( &_oWaveBanksList, (s32)FANG_OFFSETOF( FDataWvbFile_Bank_t, oLink ) );
	//
	////

	FResFrame_t oResFrame = fres_GetFrame();

	//// Allocate memory.
	//
	_paoVirtualEmitters             = (_VirtualEmitter_t *)fres_AllocAndZero( sizeof( _VirtualEmitter_t ) * _uMaxVirtualEmitters );
	_paoVirtualEmittersListActive2D = (FLinkRoot_t *)fres_AllocAndZero( sizeof( FLinkRoot_t ) * _uMaxPriorityLevels );
	_paoVirtualEmittersListActive3D = (FLinkRoot_t *)fres_AllocAndZero( sizeof( FLinkRoot_t ) * _uMaxPriorityLevels );
	_paoRealEmittersLimits          = (_RealEmitterLimit_t *)fres_AllocAndZero( sizeof( _RealEmitterLimit_t ) * _uMaxPriorityLevels );
	if( ! ( _paoVirtualEmitters &&
			_paoVirtualEmittersListActive2D &&
			_paoVirtualEmittersListActive3D &&
			_paoRealEmittersLimits ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fres_AllocAndZero() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDSRealListener );
		FDX8_SAFE_RELEASE( _poDS );
		_ReleaseCodec();
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}
	//
	////

	u32 uIndex;

	////
	//
	for( uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
	{
		flinklist_InitRoot( &( _paoVirtualEmittersListActive2D[ uIndex ] ), (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );
		flinklist_InitRoot( &( _paoVirtualEmittersListActive3D[ uIndex ] ), (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );

		_paoRealEmittersLimits[ uIndex ].uPlayable = FAUDIO_UNLIMITED_EMITTERS;
	}
	//
	////

	//// Init priority limits.
	//
	if( poInit->paoPriorityLimits )
	{
		FAudio_PriorityLimit_t *poPriorityLimit;

		for( uIndex = 0; uIndex < poInit->uPriorityLimits; ++uIndex )
		{
			////
			//
			poPriorityLimit = &( poInit->paoPriorityLimits[ uIndex ] );

			FASSERT_MSG( ( _uMaxPriorityLevels > poPriorityLimit->uPriorityLevel ), "[ FAUDIO ] Error: Invalid uPriorityLevel !!!" );
			//
			////

			_paoRealEmittersLimits[ poPriorityLimit->uPriorityLevel ].uPlayable = poPriorityLimit->uMaxPlayable;
		}
	}
	//
	////

	_VirtualListener_t *poVirtualListener = &( _aoVirtualListeners[ 0 ] );
	_VirtualEmitter_t *poVirtualEmitter   = &( _paoVirtualEmitters[ 0 ] );

	//// Allocate memory.
	//
	poVirtualListener->poWorldUser                = (CFWorldUser *)fnew CFWorldUser[ _uMaxVirtualListeners ];
	poVirtualListener->poXfmCurrentOrientation_WS = (CFXfm *)fnew CFXfm[ _uMaxVirtualListeners ];
	poVirtualListener->poVecPreviousPosition_WS   = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualListeners ];
	poVirtualListener->poVecVelocity_WS           = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualListeners ];

	poVirtualEmitter->poAudioEmitter              = (CFAudioEmitter *)fnew CFAudioEmitter[ _uMaxVirtualEmitters ];
	poVirtualEmitter->poVecCurrentPosition_WS     = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualEmitters ];
	poVirtualEmitter->poVecPreviousPosition_WS    = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualEmitters ];

	if( ! (	poVirtualListener->poWorldUser &&
			poVirtualListener->poXfmCurrentOrientation_WS &&
			poVirtualListener->poVecPreviousPosition_WS &&
			poVirtualListener->poVecVelocity_WS &&

			poVirtualEmitter->poAudioEmitter &&
			poVirtualEmitter->poVecCurrentPosition_WS &&
			poVirtualEmitter->poVecPreviousPosition_WS ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fnew() failed !!!\n", __LINE__ );

		fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
		fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
		fdelete_array( poVirtualEmitter->poAudioEmitter );

		fdelete_array( poVirtualListener->poVecVelocity_WS );
		fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
		fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );
		fdelete_array( poVirtualListener->poWorldUser );

		FDX8_SAFE_RELEASE( _poDSRealListener );
		FDX8_SAFE_RELEASE( _poDS );
		_ReleaseCodec();
		fres_ReleaseFrame( oResFrame );

		return FAUDIO_ERROR;
	}
	//
	////

	//// Init 1st listener.
	//
	_uActiveVirtualListeners = 1;
	poVirtualListener->poXfmCurrentOrientation_WS->Identity();
	poVirtualListener->poVecPreviousPosition_WS->Zero();
	poVirtualListener->poVecVelocity_WS->Zero();
	poVirtualListener->poWorldUser->m_nUser = FWORLD_USERTYPE_AUDIO_LISTENER;
	poVirtualListener->poWorldUser->m_pUser = poVirtualListener;
	//
	////

	//// Fix-up.
	//
	for( uIndex = 1; uIndex < _uMaxVirtualListeners; ++uIndex )
	{
		poVirtualListener                             = &( _aoVirtualListeners[ uIndex ] );
		poVirtualListener->poWorldUser                = ( _aoVirtualListeners[ 0 ].poWorldUser + uIndex );
		poVirtualListener->poXfmCurrentOrientation_WS = ( _aoVirtualListeners[ 0 ].poXfmCurrentOrientation_WS + uIndex );
		poVirtualListener->poVecPreviousPosition_WS   = ( _aoVirtualListeners[ 0 ].poVecPreviousPosition_WS + uIndex );
		poVirtualListener->poVecVelocity_WS           = ( _aoVirtualListeners[ 0 ].poVecVelocity_WS + uIndex );

		poVirtualListener->poWorldUser->m_nUser       = FWORLD_USERTYPE_AUDIO_LISTENER;
		poVirtualListener->poWorldUser->m_pUser       = poVirtualListener;
		poVirtualListener->poXfmCurrentOrientation_WS->Identity();
		poVirtualListener->poVecPreviousPosition_WS->Zero();
		poVirtualListener->poVecVelocity_WS->Zero();
	}

	//// Init virtual emitter.
	//
	poVirtualEmitter->poVecCurrentPosition_WS->Zero();
	poVirtualEmitter->poVecPreviousPosition_WS->Zero();
	poVirtualEmitter->poVecAutoPosition_WS = NULL;
	poVirtualEmitter->poAudioEmitter->m_nUser   = FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE;
	poVirtualEmitter->poAudioEmitter->m_pUser   = poVirtualEmitter;
	poVirtualEmitter->poRealEmitter				= NULL;
	poVirtualEmitter->poVirtualListenerCurrent	= NULL;
	poVirtualEmitter->poVirtualListenerPrevious	= NULL;
	poVirtualEmitter->uTimeStampCurrent			= 0;
	poVirtualEmitter->uTimeStampPrevious		= 0;
	poVirtualEmitter->oeListenerIntersection	= _LISTENER_INTERSECTION_NONE;
	poVirtualEmitter->fRadiusOuter				= 1.0f;
	poVirtualEmitter->fRadiusInner				= DS3D_DEFAULTMINDISTANCE;
	poVirtualEmitter->fVolume					= 1.0f;
	poVirtualEmitter->fVolumeDucked				= 1.0f;
	poVirtualEmitter->fDistanceGain				= -1.0f;
	poVirtualEmitter->fPanLeftRight				= 0.0f;
	poVirtualEmitter->fFrequencyFactor			= 1.0f;
	poVirtualEmitter->fDopplerFactor			= 0.0f;
	poVirtualEmitter->fReverb					= 0.0f;
	poVirtualEmitter->fSecondsPlayed			= 0.0f;
	poVirtualEmitter->fSecondsToPlay			= 0.0f;
	poVirtualEmitter->uProperties 				= _EMITTER_PROPERTIES_DUCKABLE; //default to duckable
	poVirtualEmitter->uPauseLevel 				= FAUDIO_PAUSE_LEVEL_1; //default pause level
	poVirtualEmitter->uStateChanges				= _EMITTER_STATE_CHANGE_NONE;
	poVirtualEmitter->oeState					= FAUDIO_EMITTER_STATE_STOPPED;
	poVirtualEmitter->pEndOfPlayCallback		= NULL;
	//
	////

	flinklist_AddTail( &_oVirtualEmittersListFree, poVirtualEmitter );

	for( uIndex = 1; uIndex < _uMaxVirtualEmitters; ++uIndex )
	{
		poVirtualEmitter                            = &( _paoVirtualEmitters[ uIndex ] );
		poVirtualEmitter->poAudioEmitter            = ( _paoVirtualEmitters[ 0 ].poAudioEmitter + uIndex );
		poVirtualEmitter->poVecCurrentPosition_WS   = ( _paoVirtualEmitters[ 0 ].poVecCurrentPosition_WS + uIndex );
		poVirtualEmitter->poVecPreviousPosition_WS  = ( _paoVirtualEmitters[ 0 ].poVecPreviousPosition_WS + uIndex );

		//// Init virtual emitter.
		//
		poVirtualEmitter->poVecCurrentPosition_WS->Zero();
		poVirtualEmitter->poVecPreviousPosition_WS->Zero();
		poVirtualEmitter->poVecAutoPosition_WS = NULL;
		poVirtualEmitter->poAudioEmitter->m_nUser   = FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE;
		poVirtualEmitter->poAudioEmitter->m_pUser   = poVirtualEmitter;
		poVirtualEmitter->poRealEmitter				= NULL;
		poVirtualEmitter->poVirtualListenerCurrent	= NULL;
		poVirtualEmitter->poVirtualListenerPrevious	= NULL;
		poVirtualEmitter->uTimeStampCurrent			= 0;
		poVirtualEmitter->uTimeStampPrevious		= 0;
		poVirtualEmitter->oeListenerIntersection	= _LISTENER_INTERSECTION_NONE;
		poVirtualEmitter->fRadiusOuter				= 1.0f;
		poVirtualEmitter->fRadiusInner				= DS3D_DEFAULTMINDISTANCE;
		poVirtualEmitter->fVolume					= 1.0f;
		poVirtualEmitter->fVolumeDucked				= 1.0f;
		poVirtualEmitter->fDistanceGain				= -1.0f;
		poVirtualEmitter->fPanLeftRight				= 0.0f;
		poVirtualEmitter->fFrequencyFactor			= 1.0f;
		poVirtualEmitter->fDopplerFactor			= 0.0f;
		poVirtualEmitter->fReverb					= 0.0f;
		poVirtualEmitter->fSecondsPlayed			= 0.0f;
		poVirtualEmitter->fSecondsToPlay			= 0.0f;
		poVirtualEmitter->uProperties 				= _EMITTER_PROPERTIES_DUCKABLE; //default to duckable
		poVirtualEmitter->uPauseLevel 				= FAUDIO_PAUSE_LEVEL_1; //default pause level
		poVirtualEmitter->uStateChanges				= _EMITTER_STATE_CHANGE_NONE;
		poVirtualEmitter->oeState					= FAUDIO_EMITTER_STATE_STOPPED;
		poVirtualEmitter->pEndOfPlayCallback		= NULL;
		//
		////

		flinklist_AddTail( &_oVirtualEmittersListFree, poVirtualEmitter );
	}
	//
	////

	//// Add to free lists.
	//
	_RealEmitter_t *poRealEmitter;

	//// 2D.
	//
	for( uIndex = 0; uIndex < _DSOUND_REAL_EMITTERS_MAX_2D; ++uIndex )
	{
		poRealEmitter = &( _aoRealEmitters2D[ uIndex ] );

		flinklist_AddTail( &_oRealEmittersListFree2D, poRealEmitter );
	}
	//
	////

	//// 3D.
	//
	for( uIndex = 0; uIndex < _DSOUND_REAL_EMITTERS_MAX_3D; ++uIndex )
	{
		poRealEmitter = &( _aoRealEmitters3D[ uIndex ] );

		flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitter );
	}
	//
	////
	//
	//// Add to free lists.

	//// Volume look-up table.
	//
	double dTempVolume;
	for( uIndex = 0; uIndex < _UNIQUE_VOLUME_LEVELS; ++uIndex )
	{
		dTempVolume = 2500.0f * log10( (double)uIndex / (double)( _UNIQUE_VOLUME_LEVELS - 1 ) );

		if( dTempVolume < DSBVOLUME_MIN )
		{
			dTempVolume = DSBVOLUME_MIN;
		}
		else if( dTempVolume > DSBVOLUME_MAX )
		{
			dTempVolume = DSBVOLUME_MAX;
		}

		_anVolumes[ uIndex ] = (s32)( dTempVolume );
	}
	//
	////

	FAudio_bModuleInstalled = TRUE;

	return FAUDIO_NO_ERROR;

} // faudio_Install

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _WaitForStreamLoads( void );

void faudio_Uninstall( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	CFAudioEmitter::DestroyAll();
#if FANG_WINGC
	CFAudioStream::DestroyAll();
	_WaitForStreamLoads();
#endif

	u32 uIndex;

	////
	//
	if( fworld_IsWorldCallbackFunctionRegistered( _WorldLoadAndUnloadCallback ) )
	{
		fworld_UnregisterWorldCallbackFunction( _WorldLoadAndUnloadCallback );
	}
	//
	////

	_poDSRealListener->CommitDeferredSettings();
	FDX8_SAFE_RELEASE( _poDSRealListener );

	//// Stop and release DS emitters.
	//
	// 3D.
	for( uIndex = 0; uIndex < _DSOUND_REAL_EMITTERS_MAX_3D; ++uIndex )
	{
		FDX8_SAFE_RELEASE( _aoRealEmitters3D[ uIndex ].poDS3DBuffer );
		FDX8_SAFE_RELEASE( _aoRealEmitters3D[ uIndex ].poDSBuffer );
	}

	// 2D.
	for( uIndex = 0; uIndex < _DSOUND_REAL_EMITTERS_MAX_2D; ++uIndex )
	{
		FDX8_SAFE_RELEASE( _aoRealEmitters2D[ uIndex ].poDSBuffer );
	}
	//
	////

	FDX8_SAFE_RELEASE( _poDS );

	//// Free memory.
	//
	_VirtualEmitter_t *poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
	fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
	fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
	fdelete_array( poVirtualEmitter->poAudioEmitter );

	_VirtualListener_t *poVirtualListener = &( _aoVirtualListeners[ 0 ] );
	fdelete_array( poVirtualListener->poVecVelocity_WS );
	fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
	fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );
	fdelete_array( poVirtualListener->poWorldUser );
	//
	////

	_ReleaseCodec();

	FAudio_bModuleInstalled = FALSE;

} // faudio_Uninstall

static BOOL _BankLoadCallback( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FALSE;
	}

	FASSERT_MSG( hRes,        "[ FAUDIO ] Error: Invalid handle !!!" );
	FASSERT_MSG( pLoadedBase, "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( pszResName,  "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pszResName, "[ FAUDIO ] Error: Zero length string !!!" );

#if FANG_WINGC
	if( _oWaveBanksList.nCount == _uMaxBanks )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Maximum number of banks already loaded !!!\n", __LINE__ );
		return FALSE;
	}

	// The converted bank keeps the Windows layout: bank, waves, PCM formats, then PCM.
	FDataWvbFile_Bank_t *poConvertedBank = gcaudio_ConvertBank( pLoadedBase, nLoadedBytes, pszResName );
	if( ! poConvertedBank )
	{
		return FALSE;
	}
	flinklist_AddTail( &_oWaveBanksList, poConvertedBank );
	fres_SetBase( hRes, poConvertedBank );
	return TRUE;
#endif

	////
	//
	if( ( sizeof( FDataWvbFile_Bank_t ) + sizeof( FDataWvbFile_Wave_t ) + sizeof( FDX8Data_WaveFormatEx_t ) + 1 ) > nLoadedBytes )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Invalid bank !!!\n", __LINE__ );
		return FALSE;
	}
	//
	////

	////
	//
	if( _oWaveBanksList.nCount == _uMaxBanks )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Maximum number of banks already loaded !!!\n", __LINE__ );
		return FALSE;
	}
	//
	////

	////
	//
	FDataWvbFile_Bank_t *poBankSource = (FDataWvbFile_Bank_t *)pLoadedBase;
	u32 uFormatDataSize = ( sizeof( FDX8Data_WaveFormatEx_t ) * poBankSource->uWaves );
//relevant??????
	u32 uWaveDataSize = ( poBankSource->uDataLength - uFormatDataSize );
	//
	////

	////
	//
//relevant??????
//fmath_FloatToU32( * 3.5f (XBADPCM) or 4.0f (MSADPCM) )
	if( ( uWaveDataSize + _uSoundBytes ) > _uMaxSoundBytes )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Bank too large for available audio memory !!!\n", __LINE__ );
		return FALSE;
	}
	//
	////

	//// Find total memory required for uncompressed audio.
	//
	FDataWvbFile_Bank_t *poBankDestination;
	FDataWvbFile_Wave_t *poWaveSource, *poWaveDestination;
	FDX8Data_WaveFormatEx_t oTempWaveformat, *poWaveformatSource, *poWaveformatDestination;
	HACMSTREAM ohCodecStream;
	ACMSTREAMHEADER oCodecStreamHeader;
	u32 uIndex, uSize, uTotalSize;

	uTotalSize         = ( sizeof( FDataWvbFile_Bank_t ) + ( poBankSource->uWaves * ( sizeof( FDataWvbFile_Wave_t ) + sizeof( FDX8Data_WaveFormatEx_t ) ) ) );
	poWaveSource       = (FDataWvbFile_Wave_t *)( (u32)poBankSource + sizeof( FDataWvbFile_Bank_t ) );
	poWaveformatSource = (FDX8Data_WaveFormatEx_t *)( (u32)poWaveSource + ( poBankSource->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );

	for( uIndex = 0; uIndex < poBankSource->uWaves; ++uIndex, ++poWaveSource, ++poWaveformatSource )
	{
		oTempWaveformat                             = *poWaveformatSource;
		oTempWaveformat.WaveFormatEx.wFormatTag     = FDX8DATA_WAVEFORMATEX_ID_PCM;
		oTempWaveformat.WaveFormatEx.wBitsPerSample = 16;

		if( 0 != acmFormatSuggest( _ohCodecDriver, (WAVEFORMATEX *)poWaveformatSource, (WAVEFORMATEX *)&oTempWaveformat, sizeof( oTempWaveformat ), ( ACM_FORMATSUGGESTF_NCHANNELS | ACM_FORMATSUGGESTF_NSAMPLESPERSEC | ACM_FORMATSUGGESTF_WBITSPERSAMPLE | ACM_FORMATSUGGESTF_WFORMATTAG ) ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmFormatSuggest() !!!\n", __LINE__ );
			return FALSE;
		}

		if( 0 != acmStreamOpen( &ohCodecStream, _ohCodecDriver, (WAVEFORMATEX *)poWaveformatSource, (WAVEFORMATEX *)&oTempWaveformat, NULL, NULL, NULL, ACM_STREAMOPENF_NONREALTIME ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmStreamOpen() !!!\n", __LINE__ );
			return FALSE;
		}

		uSize = 0;
		if( 0 != acmStreamSize( ohCodecStream, poWaveSource->uLength, (unsigned long *)&uSize, ACM_STREAMSIZEF_SOURCE ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmStreamSize() !!!\n", __LINE__ );
			acmDriverClose( _ohCodecDriver, 0 );
			return FALSE;
		}

		uTotalSize += uSize;
		acmDriverClose( _ohCodecDriver, 0 );
	}
	//
	////

	//// Allocate memory for uncompressed audio.
	//
	poBankDestination = (FDataWvbFile_Bank_t *)fres_Alloc( uTotalSize );
	if( ! poBankDestination )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fres_Alloc() !!!\n", __LINE__ );
		return FALSE;
	}

	fang_MemCopy( poBankDestination, poBankSource, ( sizeof( FDataWvbFile_Bank_t ) + ( poBankSource->uWaves * sizeof( FDataWvbFile_Wave_t ) ) ) );
	//
	////

	//// Uncompress audio.
	//
	poBankSource->pData            = (void *)( (u32)poBankSource + sizeof( FDataWvbFile_Bank_t ) + ( poBankSource->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );
	poBankDestination->pData       = (void *)( (u32)poBankDestination + sizeof( FDataWvbFile_Bank_t ) + ( poBankDestination->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );
	poBankDestination->uDataLength = ( uTotalSize - sizeof( FDataWvbFile_Bank_t ) - ( poBankDestination->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );

	poWaveSource                   = (FDataWvbFile_Wave_t *)( (u32)poBankSource + sizeof( FDataWvbFile_Bank_t ) );
	poWaveformatSource             = (FDX8Data_WaveFormatEx_t *)( (u32)poWaveSource + ( poBankSource->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );

	poWaveDestination              = (FDataWvbFile_Wave_t *)( (u32)poBankDestination + sizeof( FDataWvbFile_Bank_t ) );
	poWaveformatDestination        = (FDX8Data_WaveFormatEx_t *)( (u32)poWaveDestination + ( poBankSource->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );

	for( uIndex = 0; uIndex < poBankSource->uWaves; ++uIndex, ++poWaveSource, ++poWaveDestination, ++poWaveformatSource, ++poWaveformatDestination )
	{
		*poWaveformatDestination                             = *poWaveformatSource;
		poWaveformatDestination->WaveFormatEx.wFormatTag     = FDX8DATA_WAVEFORMATEX_ID_PCM;
		poWaveformatDestination->WaveFormatEx.wBitsPerSample = 16;

		if( 0 != acmFormatSuggest( _ohCodecDriver, (WAVEFORMATEX *)poWaveformatSource, (WAVEFORMATEX *)poWaveformatDestination, sizeof( FDX8Data_WaveFormatEx_t ), ( ACM_FORMATSUGGESTF_NCHANNELS | ACM_FORMATSUGGESTF_NSAMPLESPERSEC | ACM_FORMATSUGGESTF_WBITSPERSAMPLE | ACM_FORMATSUGGESTF_WFORMATTAG ) ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmFormatSuggest() !!!\n", __LINE__ );
			return FALSE;
		}

		if( 0 != acmStreamOpen( &ohCodecStream, _ohCodecDriver, (WAVEFORMATEX *)poWaveformatSource, (WAVEFORMATEX *)poWaveformatDestination, NULL, NULL, NULL, ACM_STREAMOPENF_NONREALTIME ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmStreamOpen() !!!\n", __LINE__ );
			return FALSE;
		}

		uSize = 0;
		if( 0 != acmStreamSize( ohCodecStream, poWaveSource->uLength, (unsigned long *)&uSize, ACM_STREAMSIZEF_SOURCE ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmStreamSize() !!!\n", __LINE__ );
			acmDriverClose( _ohCodecDriver, 0 );
			return FALSE;
		}

		poWaveDestination->oBankHandle = (FAudio_BankHandle_t)poBankDestination;
		poWaveDestination->uLength = uSize;
		if( uIndex )
		{
			poWaveDestination->uOffset = ( ( poWaveDestination - 1 )->uOffset + ( poWaveDestination - 1 )->uLength );
		}
		else
		{
			poWaveDestination->uOffset = 0;
		}

		fang_MemZero( &oCodecStreamHeader, sizeof( oCodecStreamHeader ) );
		oCodecStreamHeader.cbStruct    = sizeof( oCodecStreamHeader );
		oCodecStreamHeader.pbSrc       = (unsigned char *)( (u32)poBankSource->pData + uFormatDataSize + poWaveSource->uOffset );
		oCodecStreamHeader.cbSrcLength = poWaveSource->uLength;
		oCodecStreamHeader.pbDst       = (unsigned char *)( (u32)poBankDestination->pData + uFormatDataSize + poWaveDestination->uOffset );
		oCodecStreamHeader.cbDstLength = uSize;

		if( 0 != acmStreamPrepareHeader( ohCodecStream, &oCodecStreamHeader, 0 ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmStreamPrepareHeader() !!!\n", __LINE__ );
			acmDriverClose( _ohCodecDriver, 0 );
			return FALSE;
		}

		if( 0 != acmStreamConvert( ohCodecStream, &oCodecStreamHeader, ACM_STREAMCONVERTF_START ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmStreamConvert() !!!\n", __LINE__ );
			acmStreamUnprepareHeader( ohCodecStream, &oCodecStreamHeader, 0 );
			acmDriverClose( _ohCodecDriver, 0 );
			return FALSE;
		}

		if( 0 != acmStreamUnprepareHeader( ohCodecStream, &oCodecStreamHeader, 0 ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: acmStreamUnprepareHeader() !!!\n", __LINE__ );
			acmDriverClose( _ohCodecDriver, 0 );
			return FALSE;
		}

		acmDriverClose( _ohCodecDriver, 0 );
	}
	//
	////

	////
	//
//	_uSoundBytes += uWaveDataSize;
	flinklist_AddTail( &_oWaveBanksList, poBankDestination );
	fres_SetBase( hRes, poBankDestination );
	//
	////

	return TRUE;

} // _BankLoadCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _BankUnloadCallback( void *pResMem )
{
	FASSERT_MSG( pResMem, "[ FAUDIO ] Error: NULL pointer !!!" );

	////
	//
	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)pResMem;

//	_uSoundBytes -= ( poBank->uDataLength - ( sizeof( FDX8Data_WaveFormatEx_t ) * poBank->uWaves ) );

	// kill all sounds that are currently using this bank
	_DestroyAllEmittersFromABank( (FAudio_BankHandle_t)poBank );

	flinklist_RemoveTail( &_oWaveBanksList );
	//
	////

} // _BankUnloadCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _WorldLoadAndUnloadCallback( FWorldEvent_e oeEvent )
{
	if( ! FAudio_bModuleInstalled )
	{
		return TRUE;
	}

	u32 uIndex;

	if( FWORLD_EVENT_WORLD_POSTLOAD == oeEvent )
	{
		FLinkRoot_t *poVirtualEmittersList;
		_VirtualListener_t *poVirtualListener;
		_VirtualEmitter_t *poVirtualEmitter;

		//// Insert virtual listeners in the world.
		//
		for( uIndex = 0; uIndex < _uMaxVirtualListeners; ++uIndex )
		{
			poVirtualListener      = &( _aoVirtualListeners[ uIndex ] );
			_oTempSphere.m_Pos     = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos.v3;
			_oTempSphere.m_fRadius = DS3D_DEFAULTMINDISTANCE;

			poVirtualListener->poWorldUser->MoveTracker( _oTempSphere );
		}
		//
		////

		//// Insert active 3D virtual emitters in the world.
		//
		for( uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
		{
			poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
			poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter )
			{
				_oTempSphere.m_Pos     = poVirtualEmitter->poVecCurrentPosition_WS->v3;
				_oTempSphere.m_fRadius = poVirtualEmitter->fRadiusOuter;
				poVirtualEmitter->poAudioEmitter->MoveTracker( _oTempSphere );
				poVirtualEmitter       = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			}
		}
		//
		////
	}
	else if( FWORLD_EVENT_WORLD_PREDESTROY == oeEvent )
	{
		//// Remove virtual listeners in the world.
		//
		for( uIndex = 0; uIndex < _uMaxVirtualListeners; ++uIndex )
		{
			_aoVirtualListeners[ uIndex ].poWorldUser->RemoveFromWorld();
		}
		//
		////

		FLinkRoot_t *poVirtualEmittersList;
		_VirtualEmitter_t *poVirtualEmitter;

		//// Remove active 3D virtual emitters in the world.
		for( uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex ) {
			poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
			poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter ) {
				// ALBERT WAS REMOVING THE EMITTERS FROM THE WORLD, BUT NOT KILLING THE SOUNDS, DORK.
				// MOVIE QUOTE: "THEY MOVED THE HEADSTONES, BUT DIDN'T MOVE THE BODIES" - mike
				//poVirtualEmitter->poAudioEmitter->RemoveFromWorld();
				//poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );

				// The new way removes and stops all 3d sounds when the world is unloaded, this makes sense
				poVirtualEmitter->poAudioEmitter->Destroy();
				poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );
			}
		}
	}

	return TRUE;

} // _WorldLoadAndUnloadCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if FANG_WINGC
static BOOL _bMixSnapshot;	// -port-diag: log every playing emitter this frame

// Emitters still paused by a global pause level that has since dropped below theirs (marked by
// _EMITTER_PROPERTIES_IGNOREINPAUSEMODE): sounds started while a level's intro movie had the audio
// paused were never in the lists SetGlobalPauseLevel( NONE ) walked, and stayed paused for the level.
static void _ResumeStrandedEmitters( void )
{
	for( u32 uLevel = 0; uLevel < _uMaxPriorityLevels; ++uLevel )
	{
		for( u32 u3D = 0; u3D < 2; ++u3D )
		{
			FLinkRoot_t *poList = u3D ? &_paoVirtualEmittersListActive3D[ uLevel ] : &_paoVirtualEmittersListActive2D[ uLevel ];
			for( _VirtualEmitter_t *poEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poList ); poEmitter;
				 poEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poList, poEmitter ) )
			{
				if( ( FAUDIO_EMITTER_STATE_PAUSED == poEmitter->oeState ) &&
					( poEmitter->uProperties & _EMITTER_PROPERTIES_IGNOREINPAUSEMODE ) &&
					( (u32)poEmitter->uPauseLevel > (u32)_ePauseLevelEmitters ) )
				{
					// as CFAudioEmitter::Pause( FALSE ) does (private to the class)
					poEmitter->oeState        = FAUDIO_EMITTER_STATE_PLAYING;
					poEmitter->uStateChanges &= ~( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_STOP | _EMITTER_STATE_CHANGE_PAUSE );
					poEmitter->uStateChanges |= poEmitter->poRealEmitter ? _EMITTER_STATE_CHANGE_UNPAUSE : _EMITTER_STATE_CHANGE_PLAY;
					poEmitter->uProperties   &= ~_EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
				}
			}
		}
	}
}
#endif

void faudio_Work( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}
#if FANG_WINGC
	{
		static u32 _nMixFrames = 0;
		_bMixSnapshot = Fang_bPortDiag && ( ++_nMixFrames % 120 ) == 0;
		if( _bMixSnapshot )
		{
			// how many emitters are active, how many of them hold a DirectSound voice, and the pause level
			u32 uActive = 0, uVoiced = 0;
			for( u32 uLevel = 0; uLevel < _uMaxPriorityLevels; ++uLevel )
			{
				for( u32 u3D = 0; u3D < 2; ++u3D )
				{
					FLinkRoot_t *poList = u3D ? &_paoVirtualEmittersListActive3D[ uLevel ] : &_paoVirtualEmittersListActive2D[ uLevel ];
					for( _VirtualEmitter_t *poEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poList ); poEmitter;
						 poEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poList, poEmitter ) )
					{
						++uActive;
						if( poEmitter->poRealEmitter ) ++uVoiced;
						else if( poEmitter->oWaveHandle && poEmitter->oeState == FAUDIO_EMITTER_STATE_PLAYING && poEmitter->oeListenerIntersection )
						{
							DEVPRINTF( "PORT-MIX   voiceless %s '%s' state %d changes %x pause level %u props %x listener %d\n", u3D ? "3D" : "2D",
								((FDataWvbFile_Wave_t *)poEmitter->oWaveHandle)->szName, (s32)poEmitter->oeState, (u32)poEmitter->uStateChanges,
								(u32)poEmitter->uPauseLevel, (u32)poEmitter->uProperties, (s32)poEmitter->oeListenerIntersection );
						}
					}
				}
			}
			u32 uPlaying = 0, uPlayable = 0;
			for( u32 uLevel = 0; uLevel < _uMaxPriorityLevels; ++uLevel )
			{
				uPlaying += _paoRealEmittersLimits[ uLevel ].uPlaying;
				uPlayable += _paoRealEmittersLimits[ uLevel ].uPlayable;
			}
			DEVPRINTF( "PORT-MIX snapshot (sfx master %.2f, music master %.2f, emitters %u active %u voiced, free voices 2D %d 3D %d, playing %u of %u, pause emitters %d streams %d):\n",
				FAudio_fMasterSfxUnitVol, FAudio_fMasterMusicUnitVol, uActive, uVoiced, _oRealEmittersListFree2D.nCount, _oRealEmittersListFree3D.nCount,
				uPlaying, uPlayable, (s32)_ePauseLevelEmitters, (s32)CFAudioStream::GetGlobalPauseLevel() );
		}
	}
#endif

#if FANG_WINGC
	_ResumeStrandedEmitters();
#endif

	////
	//
	_TrackEmittersProgress( _paoVirtualEmittersListActive2D );
	_TrackEmittersProgress( _paoVirtualEmittersListActive3D );
	//
	////

	u32 uIndex;

	//// Emitters and listeners work.
	//
	_fEmittersListenersWorkDelay += FLoop_fRealPreviousLoopSecs;

	if( ( ! _bSkipEmittersListenersWorkDelay ) || ( _EMITTERS_LISTENERS_WORK_DELAY < _fEmittersListenersWorkDelay ) )
	{
		FLinkRoot_t *poVirtualEmittersList;
		_VirtualListener_t *poVirtualListener;
		_VirtualEmitter_t *poVirtualEmitter;
		s32 nList;

		////
		//
		if( FWorld_pWorld )
		{
			//// Apply virtual listeners changes (position and velocity in world).
			//
			for( uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
			{
				poVirtualListener = &( _aoVirtualListeners[ uIndex ] );

				if( _LISTENER_STATE_CHANGE_ORIENTATION & poVirtualListener->uStateChange )
				{
					// Listener velocity.
					*( poVirtualListener->poVecVelocity_WS ) = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
					poVirtualListener->poVecVelocity_WS->Sub( *( poVirtualListener->poVecPreviousPosition_WS ) );

					_oTempSphere.m_Pos     = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos.v3;
					_oTempSphere.m_fRadius = DS3D_DEFAULTMINDISTANCE;
					poVirtualListener->poWorldUser->MoveTracker( _oTempSphere );
				}
				else
				{
					poVirtualListener->poVecVelocity_WS->Zero();
				}
			}
			//
			////

			//// Update 3D virtual emitters position in world.
			//
			for( uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
			{
				poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
					//first, update the position if the listener is in a auto update mode...
					if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_AUTOPOSUPDATE ) {
						poVirtualEmitter->poAudioEmitter->SetPosition( poVirtualEmitter->poVecAutoPosition_WS );
					}

					if( _EMITTER_STATE_CHANGE_POSITION & poVirtualEmitter->uStateChanges )
					{
						_oTempSphere.m_Pos     = poVirtualEmitter->poVecCurrentPosition_WS->v3;
						_oTempSphere.m_fRadius = poVirtualEmitter->fRadiusOuter;
						poVirtualEmitter->poAudioEmitter->MoveTracker( _oTempSphere );
					}

					poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}
			}
			//
			////

			//// Listener and 3D emitter intersection.
			//
			for( nList = 0; nList < (s32)_uActiveVirtualListeners; ++nList )
			{
				_poTempVirtualListener = &( _aoVirtualListeners[ nList ] );
				_poTempVirtualListener->poWorldUser->FindIntersectingTrackers( _EmitterIntersectionCallback, FWORLD_TRACKERTYPE_USER );
			}

			// Intersection states.
			for( nList = 0; nList < (s32)_uMaxPriorityLevels; ++nList )
			{
				poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ nList ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
					if( ( _uTimeStamp - 2 ) > poVirtualEmitter->uTimeStampCurrent )
					{
						poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
						continue;
					}

					if( ( _uTimeStamp - 2 ) == poVirtualEmitter->uTimeStampCurrent )
					{
						poVirtualEmitter->oeListenerIntersection = _LISTENER_INTERSECTION_NONE;
					}
					else if( ( _uTimeStamp - 1 ) == poVirtualEmitter->uTimeStampCurrent )
					{
						poVirtualEmitter->oeListenerIntersection = _LISTENER_INTERSECTION_EXITED;
					}
					else if( _uTimeStamp == poVirtualEmitter->uTimeStampCurrent )
					{
						if( poVirtualEmitter->uTimeStampPrevious != ( _uTimeStamp - 1 ) )
						{
							poVirtualEmitter->oeListenerIntersection = _LISTENER_INTERSECTION_ENTERED;
						}
						else
						{
							if( poVirtualEmitter->poVirtualListenerPrevious == poVirtualEmitter->poVirtualListenerCurrent )
							{
								poVirtualEmitter->oeListenerIntersection = _LISTENER_INTERSECTION_PRESENT;
							}
							else
							{
								poVirtualEmitter->oeListenerIntersection = _LISTENER_INTERSECTION_SWITCHED;
							}
						}
					}

					poVirtualEmitter->uTimeStampPrevious        = poVirtualEmitter->uTimeStampCurrent;
					poVirtualEmitter->poVirtualListenerPrevious = poVirtualEmitter->poVirtualListenerCurrent;

					poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}
			}
			//
			////
		}
		//
		////

		//// Deallocate real emitters.
		//
		// 2D.
		for( nList = 0; nList < (s32)_uMaxPriorityLevels; ++nList )
		{
			poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ nList ] );
			poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter )
			{
				if( poVirtualEmitter->poRealEmitter )
				{
					if( _EMITTER_STATE_CHANGE_STOP & poVirtualEmitter->uStateChanges )
					{
						//// Stop.
						//
						poVirtualEmitter->poRealEmitter->poDSBuffer->Stop();
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDSBuffer );
						flinklist_Remove( &_oRealEmittersListActive2D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree2D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
						//
						////
					}
					else if( _EMITTER_STATE_CHANGE_PAUSE & poVirtualEmitter->uStateChanges )
					{
						//// Pause.
						//

						poVirtualEmitter->poRealEmitter->poDSBuffer->Stop();
/*
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDSBuffer );
						flinklist_Remove( &_oRealEmittersListActive2D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree2D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
*/

						poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;
						//
						////
					}
				}

				poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			}
		}

		// 3D.
		for( nList = 0; nList < (s32)_uMaxPriorityLevels; ++nList )
		{
			poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ nList ] );
			poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter )
			{
				if( poVirtualEmitter->poRealEmitter )
				{
					if( _EMITTER_STATE_CHANGE_STOP & poVirtualEmitter->uStateChanges )
					{
						//// Stop.
						//
						poVirtualEmitter->poRealEmitter->poDSBuffer->Stop();
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDS3DBuffer );
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDSBuffer );
						flinklist_Remove( &_oRealEmittersListActive3D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree3D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

						// remove the tracker from the world when you stop a 3d sound, MIKE
						poVirtualEmitter->poAudioEmitter->RemoveFromWorld();
						//
						////
					}
					else if( _EMITTER_STATE_CHANGE_PAUSE & poVirtualEmitter->uStateChanges )
					{
						//// Pause.
						//
						poVirtualEmitter->poRealEmitter->poDSBuffer->Stop();
/*
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDS3DBuffer );
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDSBuffer );
						flinklist_Remove( &_oRealEmittersListActive3D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree3D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
*/
						poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;
						//
						////
					}
				}

				if( poVirtualEmitter->poRealEmitter )
				{
					if( _LISTENER_INTERSECTION_EXITED == poVirtualEmitter->oeListenerIntersection )
					{
						//// Exited intersection.
						//
						poVirtualEmitter->poRealEmitter->poDSBuffer->Stop();
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDS3DBuffer );
						FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDSBuffer );
						flinklist_Remove( &_oRealEmittersListActive3D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree3D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

						poVirtualEmitter->uStateChanges          = _EMITTER_STATE_CHANGE_NONE;
						poVirtualEmitter->oeListenerIntersection = _LISTENER_INTERSECTION_NONE;
						//
						////
					}
				}

				poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			}
		}
		//
		//// Deallocate real emitters.

		FLinkRoot_t *poVirtualEmittersListCull;
		_RealEmitter_t *poRealEmitter, *poRealEmitterCull;
		_VirtualEmitter_t *poVirtualEmitterCull;
		u32 uPriority;
		LPVOID pData;
		DWORD dwData;
		FDataWvbFile_Bank_t *poBank;
		FDataWvbFile_Wave_t *poWave;

		//// Allocate real emitters.
		//
		// 2D.
		_oBufferDescription.dwFlags = ( DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCDEFER );
		for( nList = ( (s32)_uMaxPriorityLevels - 1 ); 0 <= nList ; --nList )
		{
			poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ nList ] );
			poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter )
			{
				if( ( _EMITTER_STATE_CHANGE_PLAY & poVirtualEmitter->uStateChanges ) &&
					( ! poVirtualEmitter->poRealEmitter ) )
				{
					uPriority = poVirtualEmitter->uPriority;

					if( _oRealEmittersListFree2D.nCount )
					{
						if( _paoRealEmittersLimits[ uPriority ].uPlaying < _paoRealEmittersLimits[ uPriority ].uPlayable )
						{
							poRealEmitter = (_RealEmitter_t *)flinklist_RemoveHead( &_oRealEmittersListFree2D );

							////
							//
							poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
							poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);

							_oBufferDescription.dwBufferBytes = poWave->uLength;
							_oBufferDescription.lpwfxFormat   = (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( FDX8Data_WaveFormatEx_t ) ) );

							if( FAILED( _poDS->CreateSoundBuffer( &_oBufferDescription, (LPDIRECTSOUNDBUFFER *)&( poRealEmitter->poDSBuffer ), NULL ) ) )
							{
								DEVPRINTF( "[ FAUDIO ] Error %u: CreateSoundBuffer() failed !!!\n", __LINE__ );
								flinklist_AddHead( &_oRealEmittersListFree2D, poRealEmitter );
								poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
								continue;
							}

							if( FAILED( poRealEmitter->poDSBuffer->Lock( 0, 0, &pData, &dwData, NULL, NULL, DSBLOCK_ENTIREBUFFER ) ) )
							{
								DEVPRINTF( "[ FAUDIO ] Error %u: Lock() failed !!!\n", __LINE__ );
								FDX8_SAFE_RELEASE( poRealEmitter->poDSBuffer );
								flinklist_AddHead( &_oRealEmittersListFree2D, poRealEmitter );
								poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
								continue;
							}

							FASSERT( poWave->uLength == dwData );

							fang_MemCopy( pData, (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( FDX8Data_WaveFormatEx_t ) ) + poWave->uOffset ), poWave->uLength );

							if( FAILED( poRealEmitter->poDSBuffer->Unlock( pData, poWave->uLength, NULL, 0 ) ) )
							{
								DEVPRINTF( "[ FAUDIO ] Error %u: Unlock() failed !!!\n", __LINE__ );
								FDX8_SAFE_RELEASE( poRealEmitter->poDSBuffer );
								flinklist_AddHead( &_oRealEmittersListFree2D, poRealEmitter );
								poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
								continue;
							}
							//
							////

							flinklist_AddTail( &_oRealEmittersListActive2D, poRealEmitter );
							poVirtualEmitter->poRealEmitter  = poRealEmitter;
							poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_PAN );
							++( _paoRealEmittersLimits[ uPriority ].uPlaying );
						}
					}
					else
					{
						// Culling.
						poRealEmitterCull = NULL;
						for( uIndex = 0; ( ( uIndex < uPriority ) && ( ! poRealEmitterCull ) ); ++uIndex )
						{
							// Find lower priority emitter to cull.
							if( _paoRealEmittersLimits[ uIndex ].uPlaying )
							{
								poVirtualEmittersListCull = &( _paoVirtualEmittersListActive2D[ uIndex ] );
								poVirtualEmitterCull      = (_VirtualEmitter_t *)flinklist_GetTail( poVirtualEmittersListCull );

								do
								{
									if( poVirtualEmitterCull->poRealEmitter )
									{
										poRealEmitterCull = poVirtualEmitterCull->poRealEmitter;
										break;
									}

									poVirtualEmitterCull = (_VirtualEmitter_t *)flinklist_GetPrev( poVirtualEmittersListCull, poVirtualEmitterCull );

								} while( 1 );
							}
						}

						if( ( ! poRealEmitterCull ) && _paoRealEmittersLimits[ uPriority ].uPlaying )
						{
							// Find lower volume emitter to cull.
							poVirtualEmittersListCull = &( _paoVirtualEmittersListActive2D[ uPriority ] );
							poVirtualEmitterCull      = (_VirtualEmitter_t *)flinklist_GetTail( poVirtualEmittersListCull );

							do
							{
								if( poVirtualEmitterCull == poVirtualEmitter )
								{
									break;
								}

								if( poVirtualEmitterCull->poRealEmitter )
								{
									if( poVirtualEmitterCull->fVolumeDucked < poVirtualEmitter->fVolumeDucked )
									{
										poRealEmitterCull = poVirtualEmitterCull->poRealEmitter;
									}

									break;
								}

								poVirtualEmitterCull = (_VirtualEmitter_t *)flinklist_GetPrev( poVirtualEmittersListCull, poVirtualEmitterCull );

							} while( 1 );
						}

						if( poRealEmitterCull )
						{
							poRealEmitterCull->poDSBuffer->Stop();
							FDX8_SAFE_RELEASE( poRealEmitterCull->poDSBuffer );
							poVirtualEmitterCull->poRealEmitter = NULL;
							--( _paoRealEmittersLimits[ poVirtualEmitterCull->uPriority ].uPlaying );

							////
							//
							poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
							poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);

							_oBufferDescription.dwBufferBytes = poWave->uLength;
							_oBufferDescription.lpwfxFormat   = (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( FDX8Data_WaveFormatEx_t ) ) );

							if( FAILED( _poDS->CreateSoundBuffer( &_oBufferDescription, (LPDIRECTSOUNDBUFFER *)&( poRealEmitterCull->poDSBuffer ), NULL ) ) )
							{
								DEVPRINTF( "[ FAUDIO ] Error %u: CreateSoundBuffer() failed !!!\n", __LINE__ );
								flinklist_Remove( &_oRealEmittersListActive2D, poRealEmitterCull );
								flinklist_AddTail( &_oRealEmittersListFree2D, poRealEmitterCull );
								poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
								continue;
							}

							if( FAILED( poRealEmitterCull->poDSBuffer->Lock( 0, 0, &pData, &dwData, NULL, NULL, DSBLOCK_ENTIREBUFFER ) ) )
							{
								DEVPRINTF( "[ FAUDIO ] Error %u: Lock() failed !!!\n", __LINE__ );
								FDX8_SAFE_RELEASE( poRealEmitterCull->poDSBuffer );
								flinklist_Remove( &_oRealEmittersListActive2D, poRealEmitterCull );
								flinklist_AddTail( &_oRealEmittersListFree2D, poRealEmitterCull );
								poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
								continue;
							}

							FASSERT( poWave->uLength == dwData );

							fang_MemCopy( pData, (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( FDX8Data_WaveFormatEx_t ) ) + poWave->uOffset ), poWave->uLength );

							if( FAILED( poRealEmitterCull->poDSBuffer->Unlock( pData, poWave->uLength, NULL, 0 ) ) )
							{
								DEVPRINTF( "[ FAUDIO ] Error %u: Unlock() failed !!!\n", __LINE__ );
								FDX8_SAFE_RELEASE( poRealEmitterCull->poDSBuffer );
								flinklist_Remove( &_oRealEmittersListActive2D, poRealEmitterCull );
								flinklist_AddTail( &_oRealEmittersListFree2D, poRealEmitterCull );
								poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
								continue;
							}
							//
							////

							poVirtualEmitter->poRealEmitter  = poRealEmitterCull;
							poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_PAN );
							++( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
						}
					}
				}

				poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			}
		}

		if( FWorld_pWorld )
		{
			// 3D.
			_oBufferDescription.dwFlags = ( DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCDEFER | DSBCAPS_MUTE3DATMAXDISTANCE );
			for( nList = ( (s32)_uMaxPriorityLevels - 1 ); 0 <= nList ; --nList )
			{
				poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ nList ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
#if FANG_WINGC
					// Also a playing emitter already in range without a voice: one resumed after it entered
					// range while paused (by a movie or the pause menu) never sees "entered" again, and its
					// play request is cleared each frame, so it stayed silent until the listener left and
					// came back.
					if( ( ( _LISTENER_INTERSECTION_ENTERED | _LISTENER_INTERSECTION_PRESENT | _LISTENER_INTERSECTION_SWITCHED ) & poVirtualEmitter->oeListenerIntersection ) &&
#else
					if( ( _LISTENER_INTERSECTION_ENTERED == poVirtualEmitter->oeListenerIntersection ) &&
#endif
						( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState ) &&
						( ! poVirtualEmitter->poRealEmitter ) )
					{
						uPriority = poVirtualEmitter->uPriority;

						if( _oRealEmittersListFree3D.nCount )
						{
							if( _paoRealEmittersLimits[ uPriority ].uPlaying < _paoRealEmittersLimits[ uPriority ].uPlayable )
							{
								poRealEmitter = (_RealEmitter_t *)flinklist_RemoveHead( &_oRealEmittersListFree3D );

								////
								//
								poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
								poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);

								_oBufferDescription.dwBufferBytes = poWave->uLength;
								_oBufferDescription.lpwfxFormat   = (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( FDX8Data_WaveFormatEx_t ) ) );

								if( FAILED( _poDS->CreateSoundBuffer( &_oBufferDescription, (LPDIRECTSOUNDBUFFER *)&( poRealEmitter->poDSBuffer ), NULL ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: CreateSoundBuffer() failed !!!\n", __LINE__ );
									flinklist_AddHead( &_oRealEmittersListFree3D, poRealEmitter );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}

								if( FAILED( poRealEmitter->poDSBuffer->QueryInterface( IID_IDirectSound3DBuffer, (void **)&( poRealEmitter->poDS3DBuffer ) ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: QueryInterface() failed !!!\n", __LINE__ );
									FDX8_SAFE_RELEASE( poRealEmitter->poDSBuffer );
									flinklist_AddHead( &_oRealEmittersListFree3D, poRealEmitter );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}

								if( FAILED( poRealEmitter->poDSBuffer->Lock( 0, 0, &pData, &dwData, NULL, NULL, DSBLOCK_ENTIREBUFFER ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: Lock() failed !!!\n", __LINE__ );
									FDX8_SAFE_RELEASE( poRealEmitter->poDS3DBuffer );
									FDX8_SAFE_RELEASE( poRealEmitter->poDSBuffer );
									flinklist_AddHead( &_oRealEmittersListFree3D, poRealEmitter );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}

								FASSERT( poWave->uLength == dwData );

								fang_MemCopy( pData, (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( FDX8Data_WaveFormatEx_t ) ) + poWave->uOffset ), poWave->uLength );

								if( FAILED( poRealEmitter->poDSBuffer->Unlock( pData, poWave->uLength, NULL, 0 ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: Unlock() failed !!!\n", __LINE__ );
									FDX8_SAFE_RELEASE( poRealEmitter->poDS3DBuffer );
									FDX8_SAFE_RELEASE( poRealEmitter->poDSBuffer );
									flinklist_AddHead( &_oRealEmittersListFree3D, poRealEmitter );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}
								//
								////

								flinklist_AddTail( &_oRealEmittersListActive3D, poRealEmitter );
								poVirtualEmitter->poRealEmitter  = poRealEmitter;
//								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_RADIUS | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								++( _paoRealEmittersLimits[ uPriority ].uPlaying );
							}
						}
						else
						{
							// Culling.
							poRealEmitterCull = NULL;
							for( uIndex = 0; ( ( uIndex < uPriority ) && ( ! poRealEmitterCull ) ); ++uIndex )
							{
								// Find lower priority emitter to cull.
								if( _paoRealEmittersLimits[ uIndex ].uPlaying )
								{
									poVirtualEmittersListCull = &( _paoVirtualEmittersListActive3D[ uIndex ] );
									poVirtualEmitterCull      = (_VirtualEmitter_t *)flinklist_GetTail( poVirtualEmittersListCull );

									do
									{
										if( poVirtualEmitterCull->poRealEmitter )
										{
											poRealEmitterCull = poVirtualEmitterCull->poRealEmitter;
											break;
										}

										poVirtualEmitterCull = (_VirtualEmitter_t *)flinklist_GetPrev( poVirtualEmittersListCull, poVirtualEmitterCull );

									} while( 1 );
								}
							}

							if( ( ! poRealEmitterCull ) && _paoRealEmittersLimits[ uPriority ].uPlaying )
							{
								// Find lower volume emitter to cull.
								poVirtualEmittersListCull = &( _paoVirtualEmittersListActive3D[ uPriority ] );
								poVirtualEmitterCull      = (_VirtualEmitter_t *)flinklist_GetTail( poVirtualEmittersListCull );

								do
								{
									if( poVirtualEmitterCull == poVirtualEmitter )
									{
										break;
									}

									if( poVirtualEmitterCull->poRealEmitter )
									{
										if( poVirtualEmitterCull->fVolumeDucked < poVirtualEmitter->fVolumeDucked )
										{
											poRealEmitterCull = poVirtualEmitterCull->poRealEmitter;
										}

										break;
									}

									poVirtualEmitterCull = (_VirtualEmitter_t *)flinklist_GetPrev( poVirtualEmittersListCull, poVirtualEmitterCull );

								} while( 1 );
							}

							if( poRealEmitterCull )
							{
								poRealEmitterCull->poDSBuffer->Stop();
								FDX8_SAFE_RELEASE( poRealEmitterCull->poDS3DBuffer );
								FDX8_SAFE_RELEASE( poRealEmitterCull->poDSBuffer );
								poVirtualEmitterCull->poRealEmitter = NULL;
								--( _paoRealEmittersLimits[ poVirtualEmitterCull->uPriority ].uPlaying );

								////
								//
								poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
								poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);

								_oBufferDescription.dwBufferBytes = poWave->uLength;
								_oBufferDescription.lpwfxFormat   = (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( FDX8Data_WaveFormatEx_t ) ) );

								if( FAILED( _poDS->CreateSoundBuffer( &_oBufferDescription, (LPDIRECTSOUNDBUFFER *)&( poRealEmitterCull->poDSBuffer ), NULL ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: CreateSoundBuffer() failed !!!\n", __LINE__ );
									flinklist_Remove( &_oRealEmittersListActive3D, poRealEmitterCull );
									flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitterCull );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}

								if( FAILED( poRealEmitterCull->poDSBuffer->QueryInterface( IID_IDirectSound3DBuffer, (void **)&( poRealEmitterCull->poDS3DBuffer ) ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: QueryInterface() failed !!!\n", __LINE__ );
									FDX8_SAFE_RELEASE( poRealEmitterCull->poDSBuffer );
									flinklist_Remove( &_oRealEmittersListActive3D, poRealEmitterCull );
									flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitterCull );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}

								if( FAILED( poRealEmitterCull->poDSBuffer->Lock( 0, 0, &pData, &dwData, NULL, NULL, DSBLOCK_ENTIREBUFFER ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: Lock() failed !!!\n", __LINE__ );
									FDX8_SAFE_RELEASE( poRealEmitterCull->poDS3DBuffer );
									FDX8_SAFE_RELEASE( poRealEmitterCull->poDSBuffer );
									flinklist_Remove( &_oRealEmittersListActive3D, poRealEmitterCull );
									flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitterCull );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}

								FASSERT( poWave->uLength == dwData );

								fang_MemCopy( pData, (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( FDX8Data_WaveFormatEx_t ) ) + poWave->uOffset ), poWave->uLength );

								if( FAILED( poRealEmitterCull->poDSBuffer->Unlock( pData, poWave->uLength, NULL, 0 ) ) )
								{
									DEVPRINTF( "[ FAUDIO ] Error %u: Unlock() failed !!!\n", __LINE__ );
									FDX8_SAFE_RELEASE( poRealEmitterCull->poDS3DBuffer );
									FDX8_SAFE_RELEASE( poRealEmitterCull->poDSBuffer );
									flinklist_Remove( &_oRealEmittersListActive3D, poRealEmitterCull );
									flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitterCull );
									poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
									continue;
								}
								//
								////

								poVirtualEmitter->poRealEmitter  = poRealEmitterCull;
//								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_RADIUS | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								++( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
							}
						}
					}

					poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}
			}
		}
		//
		//// Allocate real emitters.

		////
		//
		_ApplyRealEmittersChanges( _paoVirtualEmittersListActive2D );
		if( FWorld_pWorld )
		{
			_fOOEmittersListenersWorkDelay = ( 1.0f / _fEmittersListenersWorkDelay );
			_ApplyRealEmittersChanges( _paoVirtualEmittersListActive3D );
		}
		//
		////

		//// Invoke end-of-play callbacks.
		//
		_InvokeEmittersEndofplayCallbacks( _paoVirtualEmittersListActive2D );
		_InvokeEmittersEndofplayCallbacks( _paoVirtualEmittersListActive3D );
		//
		////

		////
		//
		for( nList = 0; nList < (s32)_uMaxPriorityLevels; ++nList )
		{
			poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ nList ] );
			poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter )
			{
				poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;

				poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			}
		}

		if( FWorld_pWorld )
		{
			//// Update previous positions of virtual listeners.
			//
			for( uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
			{
				poVirtualListener = &( _aoVirtualListeners[ uIndex ] );

				if( _LISTENER_STATE_CHANGE_ORIENTATION & poVirtualListener->uStateChange )
				{
					*( poVirtualListener->poVecPreviousPosition_WS ) = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
				}

				poVirtualListener->uStateChange = _LISTENER_STATE_CHANGE_NONE;
			}
			//
			////

			//// Update previous positions of 3D virtual emitters.
			//
			for( nList = 0; nList < (s32)_uMaxPriorityLevels; ++nList )
			{
				poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ nList ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
					if( _EMITTER_STATE_CHANGE_POSITION & poVirtualEmitter->uStateChanges )
					{
						*( poVirtualEmitter->poVecPreviousPosition_WS ) = *( poVirtualEmitter->poVecCurrentPosition_WS );
					}

					poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;

					poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}
			}
			//
			////

			//// Apply all deffered changes.
			//
			_poDSRealListener->CommitDeferredSettings();
			//
			////
		}
		//
		////

		++_uTimeStamp;
		_fEmittersListenersWorkDelay     = 0.0f;
		_bSkipEmittersListenersWorkDelay = TRUE;
	}
	//
	//// Emitters and listeners work.

#if FANG_WINGC
	_StreamsWork();
#endif

	// reset the master volume change vars
	FAudio_bMasterSfxVolChanged = FALSE;
	FAudio_bMasterMusicVolChanged = FALSE;

} // faudio_Work

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL _EmitterIntersectionCallback( CFWorldTracker *poWorldTracker, FVisVolume_t *pVolume )
{
	if( poWorldTracker->m_nUser != FWORLD_USERTYPE_AUDIO_EMITTER_3D ) {
		return TRUE;
	}

	// NOTE : RAF - Added a check to see if the sound was paused because because if we return
	// from this function without updating poVirtualEmitter->uTimeStampCurrent, then the work 
	// function will deallocate our real emitter.  We don't want that because when we unpause 
	// the sound, it will start from the begining.  It's better to fool the audio system into 
	// keeping around the emitter even though it's not currently being played.  Then when it 
	// gets un-paused, the same sound buffer will be played and it will play from the point it
	// was paused at.
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)poWorldTracker->m_pUser;
	if( ( poVirtualEmitter->oeState != FAUDIO_EMITTER_STATE_PLAYING ) && ( poVirtualEmitter->oeState != FAUDIO_EMITTER_STATE_PAUSED ) ) {
		return TRUE;
	}

	//_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)poWorldTracker->m_pUser;

	//if( ( FAUDIO_EMITTER_STATE_PLAYING != poVirtualEmitter->oeState ) ||
	//	( FWORLD_USERTYPE_AUDIO_EMITTER_3D != poWorldTracker->m_nUser ) )
	//{
	//	return TRUE;
	//}

	_oTempVec3A = _poTempVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
	_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );

	f32 fTempDistanceSq = _oTempVec3A.MagSq();

	if( _uTimeStamp != poVirtualEmitter->uTimeStampCurrent )
	{
		poVirtualEmitter->uTimeStampCurrent          = _uTimeStamp;
		poVirtualEmitter->fVirtualListenerDistanceSq = fTempDistanceSq;
		poVirtualEmitter->poVirtualListenerCurrent   = _poTempVirtualListener;
	}
	else if( fTempDistanceSq < poVirtualEmitter->fVirtualListenerDistanceSq )
	{
		poVirtualEmitter->fVirtualListenerDistanceSq = fTempDistanceSq;
		poVirtualEmitter->poVirtualListenerCurrent   = _poTempVirtualListener;
	}

	return TRUE;

} // _EmitterIntersectionCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_PauseLevel_e CFAudioEmitter::SetGlobalPauseLevel( FAudio_PauseLevel_e eNewPauseLevel )
{
	if( ! FAudio_bModuleInstalled )
	{
		return eNewPauseLevel;
	}

	FAudio_PauseLevel_e eOldPauseLevel = _ePauseLevelEmitters;

	_VirtualEmitter_t *poVirtualEmitter;
	FLinkRoot_t *poVirtualEmittersList;

	//run through the list of emitters pausing and unpausing sounds based on the new
	//pause level
	for( u32 uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
	{
		// 2D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ uIndex ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			//check to see if we should pause or unpause this sound
			if( poVirtualEmitter->uPauseLevel <= eNewPauseLevel ) { 
				//this guy needs to be paused!
				poVirtualEmitter->poAudioEmitter->Pause( TRUE );
				poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
			} else if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_IGNOREINPAUSEMODE ) {
				//this guy needs to be UNPAUSED!
				poVirtualEmitter->poAudioEmitter->Pause( FALSE );
				poVirtualEmitter->uProperties &= ~_EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
			}
			poVirtualEmitter                     = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}

		// 3D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			if( poVirtualEmitter->uPauseLevel <= eNewPauseLevel ) { 
				//this guy needs to be paused!
				poVirtualEmitter->poAudioEmitter->Pause( TRUE );
				poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
			} else if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_IGNOREINPAUSEMODE ) {
				//this guy needs to be UNPAUSED!
				poVirtualEmitter->poAudioEmitter->Pause( FALSE );
				poVirtualEmitter->uProperties &= ~_EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
			}
			poVirtualEmitter                     = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

	_ePauseLevelEmitters = eNewPauseLevel;	

	return eOldPauseLevel;

} // CFAudioEmitter::PauseMode

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_PauseLevel_e CFAudioEmitter::GetGlobalPauseLevel( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_PAUSE_LEVEL_NONE;
	}

	return ( _ePauseLevelEmitters );

} // CFAudioEmitter::IsPauseModeActive

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void faudio_SetPlayableLimitPerPriorityLevel( u32 uPriorityLevel, u32 uMaxPlayable )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	FASSERT_MSG( ( _uMaxPriorityLevels > uPriorityLevel ), "[ FAUDIO ] Error: Invalid uPriorityLevel !!!" );

	_paoRealEmittersLimits[ uPriorityLevel ].uPlayable = uMaxPlayable;

} // faudio_SetPlayableLimitPerPriorityLevel

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 faudio_GetPlayableLimitPerPriorityLevel( u32 uPriorityLevel )
{
	if( ! FAudio_bModuleInstalled )
	{
		return 0;
	}

	FASSERT_MSG( ( _uMaxPriorityLevels > uPriorityLevel ), "[ FAUDIO ] Error: Invalid uPriorityLevel !!!" );

	return _paoRealEmittersLimits[ uPriorityLevel ].uPlayable;

} // faudio_GetPlayableLimitPerPriorityLevel

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 faudio_GetBankMemoryAvailable( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return 0;
	}

	return ( _uMaxSoundBytes - _uSoundBytes );

} // faudio_GetBankMemoryAvailable

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 faudio_GetBankMemoryUsed( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return 0;
	}

	return _uSoundBytes;

} // faudio_GetBankMemoryUsed

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_Error_e faudio_LoadBanks( cchar **papszNames, FAudio_BankHandle_t *paoBankHandles )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_ERROR;
	}

	FASSERT_MSG( papszNames,        "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( *papszNames,       "[ FAUDIO ] Error: Zero length array !!!" );
	FASSERT_MSG( **papszNames,      "[ FAUDIO ] Error: Zero length string !!!" );
	FASSERT_MSG( paoBankHandles,    "[ FAUDIO ] Error: NULL pointer !!!" );

	cchar **pszName = papszNames;
	FAudio_BankHandle_t *poBankHandle = paoBankHandles;

	FResFrame_t oResFrame = fres_GetFrame();

	do
	{
		FASSERT_MSG( ( FAUDIO_MAX_ASSET_NAME_LENGTH >= fclib_strlen( *pszName ) ), "[ FAUDIO ] Error: Invalid bank name !!!" );

//		*poBankHandle = (FAudio_BankHandle_t)fresload_Load( FAUDIOBANK_RESTYPE, *pszName );
		*poBankHandle = faudio_LoadBank( *pszName );
		if( ! *poBankHandle )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: fresload_Load() failed !!!\n", __LINE__ );
			fres_ReleaseFrame( oResFrame );
			return FAUDIO_ERROR;
		}

		++pszName;
		++poBankHandle;

	} while( *pszName );

	return FAUDIO_NO_ERROR;

} // faudio_LoadBanks

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_BankHandle_t faudio_LoadBank( cchar *pszName )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_INVALID_HANDLE;
	}

	FASSERT_MSG( pszName,                                                     "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pszName,                                                    "[ FAUDIO ] Error: Zero length string !!!" );
	FASSERT_MSG( ( FAUDIO_MAX_ASSET_NAME_LENGTH >= fclib_strlen( pszName ) ), "[ FAUDIO ] Error: Invalid bank name !!!" );

	FAudio_BankHandle_t oBankHandle = FAUDIO_INVALID_HANDLE;
	FResFrame_t oResFrame = fres_GetFrame();

	// Determine if we should even try and load a localized version...
	char cLanguageChar, cAudioLanguageChar;
	ffile_GetLanguageChars( &cLanguageChar, &cAudioLanguageChar );
	if( cAudioLanguageChar != 0 ) 
	{
		// Try and load a localized version of the file...
		char szLocalizedFilename[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
		fclib_strcpy( szLocalizedFilename, pszName );
		
		u32 nStringLen = fclib_strlen( szLocalizedFilename );
		if( nStringLen < FAUDIO_MAX_ASSET_NAME_LENGTH )
		{
			szLocalizedFilename[ nStringLen ] = cAudioLanguageChar;
			szLocalizedFilename[ nStringLen + 1 ] = 0x00;

			// Now, try and load this filename
			oBankHandle = (FAudio_BankHandle_t)fresload_Load( FAUDIOBANK_RESTYPE, szLocalizedFilename );
		}
	}
	
	if( !oBankHandle )
	{
		// Try and load a NON-LOCALIZED version of this audio bank
		oBankHandle = (FAudio_BankHandle_t)fresload_Load( FAUDIOBANK_RESTYPE, pszName );
	}

	if( ! oBankHandle )
	{
		// We have utterly failed... error out.
		DEVPRINTF( "[ FAUDIO ] Error %u: fresload_Load() failed !!!\n", __LINE__ );
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_INVALID_HANDLE;
	}

	return oBankHandle;

} // faudio_LoadBank

BOOL faudio_IsValidBankHandle( FAudio_BankHandle_t oBankHandle )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FALSE;
	}

	if( ( FAUDIO_INVALID_HANDLE == oBankHandle ) || ( (u32)fres_GetFrame() > (u32)oBankHandle ) )
	{
		return FALSE;
	}

	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)flinklist_GetHead( &_oWaveBanksList );
	while( poBank )
	{
		if( poBank == (FDataWvbFile_Bank_t *)oBankHandle )
		{
			return TRUE;
		}

		poBank = (FDataWvbFile_Bank_t *)flinklist_GetNext( &_oWaveBanksList, poBank );
	}

	return FALSE;

} // faudio_IsValidBankHandle

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL faudio_IsValidWaveHandle( FAudio_WaveHandle_t oWaveHandle )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FALSE;
	}

	if( ( FAUDIO_INVALID_HANDLE == oWaveHandle ) || ( (u32)fres_GetFrame() > (u32)oWaveHandle ) )
	{
		return FALSE;
	}

	u32 uIndex;
	FDataWvbFile_Wave_t *poWave;
	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)flinklist_GetHead( &_oWaveBanksList );

	while( poBank )
	{
		poWave = (FDataWvbFile_Wave_t *)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) );
		for( uIndex = 0; uIndex < poBank->uWaves; ++uIndex, ++poWave )
		{
			if( poWave == (FDataWvbFile_Wave_t *)oWaveHandle )
			{
				return TRUE;
			}
		}

		poBank = (FDataWvbFile_Bank_t *)flinklist_GetNext( &_oWaveBanksList, poBank );
	}

	return FALSE;

} // faudio_IsValidWaveHandle

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void faudio_SetActiveListenerCount( u32 uCount )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	FASSERT_MSG( uCount,                              "[ FAUDIO ] Error: Invalid uCount !!!" );
	FASSERT_MSG( ( uCount <= _uMaxVirtualListeners ), "[ FAUDIO ] Error: Invalid uCount !!!" );

	_VirtualListener_t *poVirtualListener;

	_uActiveVirtualListeners = uCount;

	if( FWorld_pWorld )
	{
		_oTempSphere.m_Pos.Zero();
		_oTempSphere.m_fRadius = DS3D_DEFAULTMINDISTANCE;

		for( u32 uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
		{
			poVirtualListener               = &( _aoVirtualListeners[ uIndex ] );
			poVirtualListener->uStateChange = _LISTENER_STATE_CHANGE_NONE;
			poVirtualListener->poXfmCurrentOrientation_WS->Identity();
			poVirtualListener->poVecPreviousPosition_WS->Zero();
			poVirtualListener->poVecVelocity_WS->Zero();

			poVirtualListener->poWorldUser->MoveTracker( _oTempSphere );
		}
	}
	else
	{
		for( u32 uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
		{
			poVirtualListener               = &( _aoVirtualListeners[ uIndex ] );
			poVirtualListener->uStateChange = _LISTENER_STATE_CHANGE_NONE;
			poVirtualListener->poXfmCurrentOrientation_WS->Identity();
			poVirtualListener->poVecPreviousPosition_WS->Zero();
			poVirtualListener->poVecVelocity_WS->Zero();
		}
	}

} // faudio_SetActiveListenerCount

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 faudio_GetActiveListenerCount( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return 0;
	}

	return _uActiveVirtualListeners;

} // faudio_GetActiveListenerCount

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void faudio_SetListenerOrientation( u32 uListenerIndex, const CFXfm *poXfmOrientation_WS )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	FASSERT_MSG( ( uListenerIndex < _uActiveVirtualListeners ), "[ FAUDIO ] Error: Invalid uCount !!!" );
	FASSERT_MSG( poXfmOrientation_WS,                           "[ FAUDIO ] Error: NULL pointer !!!" );

	_VirtualListener_t *poVirtualListener = &( _aoVirtualListeners[ uListenerIndex ] );

	const CFMtx43A *pMtx1 = &( poXfmOrientation_WS->m_MtxF );
	CFMtx43A *pMtx2 = &( poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF );

	////
	//
	_oTempVec3A = pMtx1->m_vPos;
	_oTempVec3A.Sub( pMtx2->m_vPos );

	if( ( _SIGNIFICANT_DISTANCE_CHANGE_SQ < _oTempVec3A.MagSq() ) ||
		( _SIGNIFICANT_DOT_CHANGE > pMtx1->m_vFront.Dot( pMtx2->m_vFront ) ) ||
		( _SIGNIFICANT_DOT_CHANGE > pMtx1->m_vUp.Dot( pMtx2->m_vUp ) ) )
	{
		poVirtualListener->uStateChange                    |= _LISTENER_STATE_CHANGE_ORIENTATION;
		*( poVirtualListener->poXfmCurrentOrientation_WS )  = *( poXfmOrientation_WS );
	}
	//
	////

} // faudio_SetListenerOrientation

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void faudio_SetListenerOrientationAndMaintainDoppler( u32 uListenerIndex, const CFXfm *poXfmOrientation_WS )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	FASSERT_MSG( ( uListenerIndex < _uActiveVirtualListeners ), "[ FAUDIO ] Error: Invalid uCount !!!" );
	FASSERT_MSG( poXfmOrientation_WS,                           "[ FAUDIO ] Error: NULL pointer !!!" );

	_VirtualListener_t *poVirtualListener = &( _aoVirtualListeners[ uListenerIndex ] );

	const CFMtx43A *pMtx1 = &( poXfmOrientation_WS->m_MtxF );
	CFMtx43A *pMtx2 = &( poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF );

	////
	//
	_oTempVec3A = pMtx1->m_vPos;
	_oTempVec3A.Sub( pMtx2->m_vPos );

	if( ( _SIGNIFICANT_DISTANCE_CHANGE_SQ < _oTempVec3A.MagSq() ) ||
		( _SIGNIFICANT_DOT_CHANGE > pMtx1->m_vFront.Dot( pMtx2->m_vFront ) ) ||
		( _SIGNIFICANT_DOT_CHANGE > pMtx1->m_vUp.Dot( pMtx2->m_vUp ) ) )
	{
		poVirtualListener->uStateChange |= _LISTENER_STATE_CHANGE_ORIENTATION;

		_oTempVec3A_Velocity = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
		_oTempVec3A_Velocity.Sub( *( poVirtualListener->poVecPreviousPosition_WS ) );
		_oTempVec3A = poXfmOrientation_WS->m_MtxF.m_vPos;
		_oTempVec3A.Sub( _oTempVec3A_Velocity );

		*( poVirtualListener->poVecPreviousPosition_WS )   = _oTempVec3A;
		*( poVirtualListener->poXfmCurrentOrientation_WS ) = *( poXfmOrientation_WS );
	}
	//
	////

} // faudio_SetListenerOrientationAndMaintainDoppler

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CFAudioEmitter *CFAudioEmitter::Create2D( FAudio_WaveHandle_t oWaveHandle, u8 uPriority /* = FAudio_EmitterDefaultPriorityLevel */, CFAudioEmitter **ppUserAudioEmitter )
{
	if( ! FAudio_bModuleInstalled )
	{
		return NULL;
	}

	if( ! oWaveHandle )
	{
		if( _uEmittersCreateErrors2D )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioEmitter::Create2D() failed !!!\n", __LINE__ );
			--_uEmittersCreateErrors2D;
			if( ! _uEmittersCreateErrors2D )
			{
				DEVPRINTF( "[ FAUDIO ] Warning %u: The previous error will not be issued anymore !!!\n", __LINE__ );
			}
		}
		return NULL;
	}

//	FASSERT_MSG( faudio_IsValidWaveHandle( oWaveHandle ), "[ FAUDIO ] Error: Invalid handle !!!" );
	FASSERT_MSG( ( _uMaxPriorityLevels > uPriority ),     "[ FAUDIO ] Error: Invalid uPriority !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)flinklist_RemoveHead( &_oVirtualEmittersListFree );
	if( poVirtualEmitter )
	{
		poVirtualEmitter->oWaveHandle                  = oWaveHandle;
		poVirtualEmitter->ppUserAudioEmitter           = ppUserAudioEmitter;
		poVirtualEmitter->uPriority                    = uPriority;
		poVirtualEmitter->fVolume                      = 1.0f;
		poVirtualEmitter->fVolumeDucked                = 1.0f;
		poVirtualEmitter->fDistanceGain                = -1.0f;
		poVirtualEmitter->fPanLeftRight                = 0.0f;
		poVirtualEmitter->fFrequencyFactor             = 1.0f;
		poVirtualEmitter->fDopplerFactor               = 0.0f;
		poVirtualEmitter->fReverb                      = 0.0f;
		poVirtualEmitter->fSecondsPlayed               = 0.0f;
		poVirtualEmitter->fSecondsToPlay               = 0.0f;
		poVirtualEmitter->uProperties                  = _EMITTER_PROPERTIES_DUCKABLE;
		poVirtualEmitter->uPauseLevel				   = FAUDIO_PAUSE_LEVEL_1; //default pause level
		poVirtualEmitter->uStateChanges                = _EMITTER_STATE_CHANGE_NONE;
		poVirtualEmitter->oeState                      = FAUDIO_EMITTER_STATE_STOPPED;
		poVirtualEmitter->pEndOfPlayCallback           = NULL;
		poVirtualEmitter->poVirtualListenerCurrent     = NULL;
		poVirtualEmitter->poAudioEmitter->m_nUser      = FWORLD_USERTYPE_AUDIO_EMITTER_2D;
		poVirtualEmitter->paoVirtualEmittersListActive = &( _paoVirtualEmittersListActive2D[ uPriority ] ); // Optimization.

		if( ppUserAudioEmitter ) {
			*ppUserAudioEmitter = poVirtualEmitter->poAudioEmitter;
		}

		flinklist_AddTail( poVirtualEmitter->paoVirtualEmittersListActive, poVirtualEmitter );

		return poVirtualEmitter->poAudioEmitter;
	}
	else
	{
		if( _uEmittersCreateErrors2D )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioEmitter::Create2D() failed !!!\n", __LINE__ );
			--_uEmittersCreateErrors2D;
			if( ! _uEmittersCreateErrors2D )
			{
				DEVPRINTF( "[ FAUDIO ] Warning %u: The previous error will not be issued anymore !!!\n", __LINE__ );
			}
		}
		return NULL;
	}

} // CFAudioEmitter::Create2D

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CFAudioEmitter *CFAudioEmitter::Create3D( FAudio_WaveHandle_t oWaveHandle, u8 uPriority /* = FAudio_EmitterDefaultPriorityLevel */, const CFVec3A *poVecPosition_WS /* = NULL */, f32 fRadiusOuter /* = FAUDIO_MIN_RADIUS */, BOOL bAutoUpdatePosition /* = FALSE */, CFAudioEmitter **ppUserAudioEmitter )
{
	if( ! FAudio_bModuleInstalled )
	{
		return NULL;
	}

	if( ! oWaveHandle )
	{
		if( _uEmittersCreateErrors3D )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioEmitter::Create3D() failed !!!\n", __LINE__ );
			--_uEmittersCreateErrors3D;
			if( ! _uEmittersCreateErrors3D )
			{
				DEVPRINTF( "[ FAUDIO ] Warning %u: The previous error will not be issued anymore !!!\n", __LINE__ );
			}
		}
		return NULL;
	}

//	FASSERT_MSG( faudio_IsValidWaveHandle( oWaveHandle ), "[ FAUDIO ] Error: Invalid handle !!!" );
	FASSERT_MSG( ( _uMaxPriorityLevels > uPriority ),     "[ FAUDIO ] Error: Invalid uPriority !!!" );
#if FANG_WINGC
	// Retail data gives some ambient sounds a radius below DirectSound's minimum distance (WEWJjourn01's
	// spheres); the GameCube had no such floor. Play them at the smallest radius DirectSound takes.
	if( fRadiusOuter < DS3D_DEFAULTMINDISTANCE )
	{
		fRadiusOuter = DS3D_DEFAULTMINDISTANCE;
	}
#endif
	FASSERT_MSG( ( DS3D_DEFAULTMINDISTANCE <= fRadiusOuter ),   "[ FAUDIO ] Error: Invalid fRadiusOuter !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)flinklist_RemoveHead( &_oVirtualEmittersListFree );
	if( poVirtualEmitter )
	{
		poVirtualEmitter->oWaveHandle                  = oWaveHandle;
		poVirtualEmitter->ppUserAudioEmitter           = ppUserAudioEmitter;
		poVirtualEmitter->uPriority                    = uPriority;
		poVirtualEmitter->uProperties				  |= _EMITTER_PROPERTIES_3D;
		poVirtualEmitter->fVolume                      = 1.0f;
		poVirtualEmitter->fVolumeDucked                = 1.0f;
		poVirtualEmitter->fDistanceGain                = -1.0f;
		poVirtualEmitter->fPanLeftRight                = 0.0f;
		poVirtualEmitter->fFrequencyFactor             = 1.0f;
		poVirtualEmitter->fDopplerFactor               = 0.0f;
		poVirtualEmitter->fReverb                      = 0.0f;
		poVirtualEmitter->fSecondsPlayed               = 0.0f;
		poVirtualEmitter->fSecondsToPlay               = 0.0f;
		poVirtualEmitter->uProperties				   = ( _EMITTER_PROPERTIES_3D | _EMITTER_PROPERTIES_DUCKABLE );
		if( bAutoUpdatePosition ) {
			poVirtualEmitter->uProperties			  |= _EMITTER_PROPERTIES_AUTOPOSUPDATE;
		}             
		poVirtualEmitter->uPauseLevel 				   = FAUDIO_PAUSE_LEVEL_1; //default pause level
		poVirtualEmitter->uStateChanges                = _EMITTER_STATE_CHANGE_NONE;
		poVirtualEmitter->oeState                      = FAUDIO_EMITTER_STATE_STOPPED;
		poVirtualEmitter->pEndOfPlayCallback           = NULL;
		poVirtualEmitter->poVirtualListenerCurrent     = NULL;
		poVirtualEmitter->fRadiusOuter                 = fRadiusOuter;
		poVirtualEmitter->fRadiusInner                 = DS3D_DEFAULTMINDISTANCE;
		poVirtualEmitter->poAudioEmitter->m_nUser      = FWORLD_USERTYPE_AUDIO_EMITTER_3D;
		poVirtualEmitter->paoVirtualEmittersListActive = &( _paoVirtualEmittersListActive3D[ uPriority ] ); // Optimization.

		if( poVecPosition_WS )
		{
			*( poVirtualEmitter->poVecPreviousPosition_WS ) = *( poVirtualEmitter->poVecCurrentPosition_WS ) = *( poVecPosition_WS );
			if( bAutoUpdatePosition ) {
				poVirtualEmitter->poVecAutoPosition_WS = poVecPosition_WS;
			}
		}
		else
		{
			FASSERT( !bAutoUpdatePosition ); //Doesn't make sense to get autoupdate and not pass in a ws pointer
			poVirtualEmitter->poVecAutoPosition_WS = poVirtualEmitter->poVecCurrentPosition_WS;
			poVirtualEmitter->poVecPreviousPosition_WS->Zero();
			poVirtualEmitter->poVecCurrentPosition_WS->Zero();
		}

		//// Insert 3D virtual emitters in the world.
		//
		if( FWorld_pWorld )
		{
			_oTempSphere.m_Pos     = poVirtualEmitter->poVecCurrentPosition_WS->v3;
			_oTempSphere.m_fRadius = poVirtualEmitter->fRadiusOuter;
			poVirtualEmitter->poAudioEmitter->MoveTracker( _oTempSphere );
		}
		//
		////

		if( ppUserAudioEmitter ) {
			*ppUserAudioEmitter = poVirtualEmitter->poAudioEmitter;
		}

		flinklist_AddTail( poVirtualEmitter->paoVirtualEmittersListActive, poVirtualEmitter );

		return poVirtualEmitter->poAudioEmitter;
	}
	else
	{
		if( _uEmittersCreateErrors3D )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioEmitter::Create3D() failed !!!\n", __LINE__ );
			--_uEmittersCreateErrors3D;
			if( ! _uEmittersCreateErrors3D )
			{
				DEVPRINTF( "[ FAUDIO ] Warning %u: The previous error will not be issued anymore !!!\n", __LINE__ );
			}
		}
		return NULL;
	}

} // CFAudioEmitter::Create3D

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::Destroy( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	if (!poVirtualEmitter) return;

	if( poVirtualEmitter->ppUserAudioEmitter ) {
		*poVirtualEmitter->ppUserAudioEmitter = NULL;
		poVirtualEmitter->ppUserAudioEmitter = NULL;
	}

	if( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE == poVirtualEmitter->poAudioEmitter->m_nUser )
	{
		return;
	}

	poVirtualEmitter->poAudioEmitter->m_nUser = FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE;

	if( poVirtualEmitter->poRealEmitter )
	{
		if( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState )
		{
			if( poVirtualEmitter->pEndOfPlayCallback )
			{
				poVirtualEmitter->pEndOfPlayCallback( poVirtualEmitter->poAudioEmitter );
			}

			poVirtualEmitter->poRealEmitter->poDSBuffer->Stop();
			FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDS3DBuffer );
			FDX8_SAFE_RELEASE( poVirtualEmitter->poRealEmitter->poDSBuffer );
			--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
		}

		if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_3D )
		{
			flinklist_Remove( &( _oRealEmittersListActive3D ), poVirtualEmitter->poRealEmitter );
			flinklist_AddTail( &( _oRealEmittersListFree3D ), poVirtualEmitter->poRealEmitter );

			//// Remove 3D virtual emitters in the world.
			//
			poVirtualEmitter->poAudioEmitter->RemoveFromWorld();
			//
			////
		}
		else
		{
			flinklist_Remove( &( _oRealEmittersListActive2D ), poVirtualEmitter->poRealEmitter );
			flinklist_AddTail( &( _oRealEmittersListFree2D ), poVirtualEmitter->poRealEmitter );
		}

		poVirtualEmitter->poRealEmitter = NULL;
	}

	flinklist_Remove( poVirtualEmitter->paoVirtualEmittersListActive, poVirtualEmitter );
	flinklist_AddTail( &_oVirtualEmittersListFree, poVirtualEmitter );

} // CFAudioEmitter::Destroy

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::DestroyAll( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	_VirtualEmitter_t *poVirtualEmitter;
	FLinkRoot_t *poVirtualEmittersList;

	for( u32 uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
	{
		// 2D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ uIndex ] );

		while( poVirtualEmittersList->nCount )
		{
			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			if( poVirtualEmitter ) {
				poVirtualEmitter->poAudioEmitter->Destroy();
			} else {
				DEVPRINTF( "fdx8audio() : The 2d emitter linklist is corupt, re-initing the link root, count = %d.\n", poVirtualEmittersList->nCount );
				flinklist_InitRoot( &( _paoVirtualEmittersListActive2D[ uIndex ] ), (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );
				break;
			}
		}

		// 3D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );

		while( poVirtualEmittersList->nCount )
		{
			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			if( poVirtualEmitter ) {
				poVirtualEmitter->poAudioEmitter->Destroy();
			} else {
				DEVPRINTF( "fdx8audio() : The 3d emitter linklist is corupt, re-initing the link root, count = %d.\n", poVirtualEmittersList->nCount );
				flinklist_InitRoot( &( _paoVirtualEmittersListActive3D[ uIndex ] ), (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );
				break;
			}
		}
	}

} // CFAudioEmitter::DestroyAll

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetPosition( const CFVec3A *poVecPosition_WS )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                                  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( poVecPosition_WS,                                   "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( ((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D, "[ FAUDIO ] Error: Illegal call !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	if( ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_AUTOPOSUPDATE ) && 
	    ( poVirtualEmitter->poVecAutoPosition_WS != poVecPosition_WS ) ) {
		//update the autopos pointer to this one...
		poVirtualEmitter->poVecAutoPosition_WS = poVecPosition_WS;
	}

	////
	//
	_oTempVec3A = *poVecPosition_WS;
	_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );

	if( _SIGNIFICANT_DISTANCE_CHANGE_SQ < _oTempVec3A.MagSq() )
	{
		poVirtualEmitter->uStateChanges                |= _EMITTER_STATE_CHANGE_POSITION;
		*( poVirtualEmitter->poVecCurrentPosition_WS )  = *poVecPosition_WS;
	}
	//
	////

} // CFAudioEmitter::SetPosition

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetPositionAndMaintainDoppler( const CFVec3A *poVecPosition_WS )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                                  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( poVecPosition_WS,                                   "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( ((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D, "[ FAUDIO ] Error: Illegal call !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	if( ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_AUTOPOSUPDATE ) && 
	    ( poVirtualEmitter->poVecAutoPosition_WS != poVecPosition_WS ) ) {
		//update the autopos pointer to this one...
		poVirtualEmitter->poVecAutoPosition_WS = poVecPosition_WS;
	}

	////
	//
	_oTempVec3A = *poVecPosition_WS;
	_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );

	if( _SIGNIFICANT_DISTANCE_CHANGE_SQ < _oTempVec3A.MagSq() )
	{
		poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_POSITION;

		_oTempVec3A_Velocity = *( poVirtualEmitter->poVecCurrentPosition_WS );
		_oTempVec3A_Velocity.Sub( *( poVirtualEmitter->poVecPreviousPosition_WS ) );
		_oTempVec3A = *( poVecPosition_WS );
		_oTempVec3A.Sub( _oTempVec3A_Velocity );

		*( poVirtualEmitter->poVecPreviousPosition_WS ) = _oTempVec3A;
		*( poVirtualEmitter->poVecCurrentPosition_WS )  = *( poVecPosition_WS );
	}

} // CFAudioEmitter::SetPositionAndMaintainDoppler

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetRadius( f32 fRadiusOuter, f32 fRadiusInner /* = DS3D_DEFAULTMINDISTANCE */ )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                                  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D, "[ FAUDIO ] Error: Illegal call !!!" );
	FASSERT_MSG( ( fRadiusInner <= fRadiusOuter ),                   "[ FAUDIO ] Error: Invalid fRadiusOuter !!!" );
	FASSERT_MSG( ( DS3D_DEFAULTMINDISTANCE <= fRadiusInner ),              "[ FAUDIO ] Error: Invalid fRadiusInner !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	////
	//
	f32 fTemp1 = ( poVirtualEmitter->fRadiusOuter - fRadiusOuter );
	fTemp1 = ( fTemp1 * fTemp1 );
/*
	f32 fTemp2 = ( poVirtualEmitter->fRadiusInner - fRadiusInner );
	fTemp2 = ( fTemp2 * fTemp2 );

	if( ( _SIGNIFICANT_DISTANCE_CHANGE_SQ < fTemp1 ) ||
		( _SIGNIFICANT_DISTANCE_CHANGE_SQ < fTemp2 ) )
*/
	if( _SIGNIFICANT_DISTANCE_CHANGE_SQ < fTemp1 )
	{
//		poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_RADIUS;
		poVirtualEmitter->fRadiusOuter   = fRadiusOuter;
//		poVirtualEmitter->fRadiusInner   = fRadiusInner;
	}
	//
	////

} // CFAudioEmitter::SetRadius

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetVolume( f32 fVolume )
{
	FASSERT_MSG( FAudio_bModuleInstalled,              "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( FMATH_IS_UNIT_FLOAT( fVolume ), "[ FAUDIO ] Error: Invalid fVolume !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );

#if !FANG_PRODUCTION_BUILD
	FASSERT(flinklist_IsLinkInList(poVirtualEmitter->paoVirtualEmittersListActive, flinklist_GetLinkPointer(poVirtualEmitter->paoVirtualEmittersListActive, poVirtualEmitter)));
#endif

#if !FANG_WINGC	// the GameCube chain (see _GCMusyxVolume) balances 2D and 3D itself; gc/fgcaudio.cpp has no such cut
	if( ! ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_3D ) )
	{
		fVolume *= 0.1f; // Hack to attenuate 2D sounds, which are much louder than 3D.
	}
#endif
	//
	////

	////
	//
	f32 fTemp = ( fVolume - poVirtualEmitter->fVolume );
	BOOL bVolumeIncreased = ( fTemp >= 0.0f ) ? TRUE : FALSE;

	fTemp = ( fTemp * fTemp );
	if( ( ! ( ( _EMITTER_STATE_CHANGE_DUCKABLE | _EMITTER_STATE_CHANGE_DUCKING ) & poVirtualEmitter->uStateChanges ) ) && // Ducking state change.
		( _SIGNIFICANT_VOLUME_CHANGE_SQ > fTemp ) )
	{
		// Indistinguishable difference.
		return;
	}
	//
	////

	////
	//
	poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_VOLUME;
	poVirtualEmitter->fVolume        = fVolume;
	if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_DUCKABLE )
	{
		poVirtualEmitter->fVolumeDucked = fVolume * _fDuckingFactor;
	}
	else
	{
		poVirtualEmitter->fVolumeDucked = fVolume;
	}
	//
	////

	//// Nothing to sort.
	//
	if( 1 == poVirtualEmitter->paoVirtualEmittersListActive->nCount )
	{
		poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_DUCKABLE | _EMITTER_STATE_CHANGE_DUCKING ) );
		return;
	}

	if( _EMITTER_STATE_CHANGE_DUCKING & poVirtualEmitter->uStateChanges )
	{
		poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_DUCKABLE | _EMITTER_STATE_CHANGE_DUCKING ) );
		return;
	}
	//
	////

	//// Keep list in volume order.
	//
	_VirtualEmitter_t *poVirtualEmitterIterator;
	fTemp = poVirtualEmitter->fVolumeDucked;
	FLinkRoot_t *poList = poVirtualEmitter->paoVirtualEmittersListActive;

	if( bVolumeIncreased )
	{
		// Look to move this element to the left.
		poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetPrev( poList, poVirtualEmitter );
		flinklist_Remove( poList, poVirtualEmitter );

		while( poVirtualEmitterIterator )
		{
			// See if our volume is less than the previous one.
			if( poVirtualEmitterIterator->fVolumeDucked >= fTemp )
			{
				break;
			}

			// Get the next item to the left.
			poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetPrev( poList, poVirtualEmitterIterator );
		}

		// Insert after poVirtualEmitterIterator.
		if( poVirtualEmitterIterator )
		{
			flinklist_AddAfter( poList, poVirtualEmitterIterator, poVirtualEmitter );
		}
		else
		{
			flinklist_AddHead( poList, poVirtualEmitter );
		}
	}
	else
	{
		// Look to move this element to the right.
		poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetNext( poList, poVirtualEmitter );
		flinklist_Remove( poList, poVirtualEmitter );

		while( poVirtualEmitterIterator )
		{
			// See if our volume is greater than the previous one.
			if( poVirtualEmitterIterator->fVolumeDucked <= fTemp )
			{
				break;
			}

			// Get the next item to the right.
			poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetNext( poList, poVirtualEmitterIterator );
		}

		// Insert before poVirtualEmitterIterator.
		if( poVirtualEmitterIterator )
		{
			flinklist_AddBefore( poList, poVirtualEmitterIterator, poVirtualEmitter );
		}
		else
		{
			flinklist_AddTail( poList, poVirtualEmitter );
		}
	}
	//
	////

} // CFAudioEmitter::SetVolume

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetPan( f32 fPanLeftRight )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                                        "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ( ! (((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D) ), "[ FAUDIO ] Error: Illegal call !!!" );
	FASSERT_MSG( FMATH_IS_BIPOLAR_UNIT_FLOAT( fPanLeftRight ),             "[ FAUDIO ] Error: Invalid fPanLeftRight !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	////
	//
	f32 fTemp = ( poVirtualEmitter->fPanLeftRight - fPanLeftRight );
	fTemp = ( fTemp * fTemp );

	if( _SIGNIFICANT_VOLUME_CHANGE_SQ < fTemp )
	{
		poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_PAN;
		poVirtualEmitter->fPanLeftRight  = fPanLeftRight;
	}
	//
	////

} // CFAudioEmitter::SetPan

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetEndOfPlayCallback( FAudio_EmitterEndOfPlayCallback_t *pEndOfPlayCallback )
{
	FASSERT_MSG( FAudio_bModuleInstalled,  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( pEndOfPlayCallback, "[ FAUDIO ] Error: NULL pointer !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	poVirtualEmitter->pEndOfPlayCallback = pEndOfPlayCallback;

} // CFAudioEmitter::SetEndOfPlayCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetFrequencyFactor( f32 fFrequencyFactor )
{
	FASSERT_MSG( FAudio_bModuleInstalled,            "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ( 0.0f <= fFrequencyFactor ), "[ FAUDIO ] Error: Invalid fFrequencyFactor !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	////
	//
	f32 fTemp = ( poVirtualEmitter->fFrequencyFactor - fFrequencyFactor );
	fTemp = ( fTemp * fTemp );

	if( _SIGNIFICANT_FACTOR_CHANGE_SQ < fTemp )
	{
		poVirtualEmitter->uStateChanges    |= _EMITTER_STATE_CHANGE_FREQUENCY;
		poVirtualEmitter->fFrequencyFactor  = fFrequencyFactor;
	}
	//
	////

} // CFAudioEmitter::SetFrequencyFactor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetDopplerFactor( f32 fDopplerFactor )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                                  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D, "[ FAUDIO ] Error: Illegal call !!!" );
	FASSERT_MSG( ( 0.0f <= fDopplerFactor ),                         "[ FAUDIO ] Error: Invalid fDopplerFactor !!!" );
	FASSERT_MSG( ( 10.0f >= fDopplerFactor ),                        "[ FAUDIO ] Error: Invalid fDopplerFactor !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	////
	//
	f32 fTemp = ( poVirtualEmitter->fDopplerFactor - fDopplerFactor );
	fTemp = ( fTemp * fTemp );

	if( _SIGNIFICANT_FACTOR_CHANGE_SQ < fTemp )
	{
		poVirtualEmitter->uStateChanges  |= _EMITTER_STATE_CHANGE_DOPPLER;
		poVirtualEmitter->fDopplerFactor  = fDopplerFactor;
	}
	//
	////

} // CFAudioEmitter::SetDopplerFactor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetReverb( f32 fReverb )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                                  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D, "[ FAUDIO ] Error: Illegal call !!!" );
	FASSERT_MSG( FMATH_IS_UNIT_FLOAT( fReverb ),                     "[ FAUDIO ] Error: Invalid fReverb !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	////
	//
	f32 fTemp = ( poVirtualEmitter->fReverb - fReverb );
	fTemp = ( fTemp * fTemp );

	if( _SIGNIFICANT_FACTOR_CHANGE_SQ < fTemp )
	{
		poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_REVERB;
		poVirtualEmitter->fReverb        = fReverb;
	}
	//
	////

} // CFAudioEmitter::SetReverb

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetDuckable( BOOL bDuckable )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	if( _EMITTER_PROPERTIES_DUCKABLE ^ ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_DUCKABLE ) )
	{
		poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_DUCKABLE;
		if( bDuckable ) {
			poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_DUCKABLE;
		} else {
			poVirtualEmitter->uProperties &= ~_EMITTER_PROPERTIES_DUCKABLE;
		}
		SetVolume( poVirtualEmitter->fVolume );
	}

} // CFAudioEmitter::SetDuckable

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CFAudioEmitter::GetDuckable( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	return ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_DUCKABLE );

} // CFAudioEmitter::GetDuckable

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::DuckAll( f32 fVolumeFactor )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	FASSERT_MSG( FMATH_IS_UNIT_FLOAT( fVolumeFactor ), "[ FAUDIO ] Error: Invalid fVolumeFactor !!!" );

	////
	//
	f32 fTemp = ( fVolumeFactor - _fDuckingFactor );
	fTemp = ( fTemp * fTemp );

	if( _SIGNIFICANT_VOLUME_CHANGE_SQ > fTemp )
	{
		// Indistinguishable difference.
		return;
	}
	//
	////

	_fDuckingFactor = fVolumeFactor;

	_VirtualEmitter_t *poVirtualEmitter, *poVirtualEmitterNext;
	FLinkRoot_t *poVirtualEmittersList;

	for( u32 uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
	{
		// 2D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ uIndex ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			poVirtualEmitterNext             = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_DUCKING;
			poVirtualEmitter->poAudioEmitter->SetVolume( poVirtualEmitter->fVolume );
			poVirtualEmitter                 = poVirtualEmitterNext;
		}

		// 3D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			poVirtualEmitterNext             = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_DUCKING;
			poVirtualEmitter->poAudioEmitter->SetVolume( poVirtualEmitter->fVolume );
			poVirtualEmitter                 = poVirtualEmitterNext;
		}
	}

} // CFAudioEmitter::DuckAll

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetPriority( u8 uPriority )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                   "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ( _uMaxPriorityLevels > uPriority ), "[ FAUDIO ] Error: Invalid uPriority !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	if( uPriority == poVirtualEmitter->uPriority )
	{
		return;
	}

	flinklist_Remove( poVirtualEmitter->paoVirtualEmittersListActive, poVirtualEmitter );
	--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
	poVirtualEmitter->uPriority = uPriority;
	++( _paoRealEmittersLimits[ uPriority ].uPlaying );

	if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_3D )
	{
		poVirtualEmitter->paoVirtualEmittersListActive = &( _paoVirtualEmittersListActive3D[ uPriority ] );
	}
	else
	{
		poVirtualEmitter->paoVirtualEmittersListActive = &( _paoVirtualEmittersListActive3D[ uPriority ] );
	}

	//// Keep list in volume order.
	//
	if( poVirtualEmitter->paoVirtualEmittersListActive->nCount )
	{
		// Scan right.
		_VirtualEmitter_t *poVirtualEmitterIterator;
		f32 fVolume = poVirtualEmitter->fVolumeDucked;
		FLinkRoot_t *poList = poVirtualEmitter->paoVirtualEmittersListActive;

		poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetHead( poList );

		do
		{
			if( poVirtualEmitterIterator->fVolumeDucked <= fVolume )
			{
				flinklist_AddBefore( poList, poVirtualEmitterIterator, poVirtualEmitter );
				return;
			}

			poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetNext( poList, poVirtualEmitterIterator );

		} while( poVirtualEmitterIterator );

		flinklist_AddTail( poList, poVirtualEmitter );
	}
	else
	{
		flinklist_AddTail( poVirtualEmitter->paoVirtualEmittersListActive, poVirtualEmitter );
	}
	//
	////

} // CFAudioEmitter::SetPriority

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 CFAudioEmitter::GetPriority( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	return poVirtualEmitter->uPriority;

} // CFAudioEmitter::GetPriority

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//void CFAudioEmitter::Play( u32 uLoops /* = 1 */, f32 fFirstLoopStartOffsetInSeconds /* = 0.0f */ )
void CFAudioEmitter::Play( u32 uLoops /* = 1 */ )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	if( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState )
	{
		return;
	}

	////
	//
	FDataWvbFile_Wave_t *poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;

//	FASSERT_MSG( ( fFirstLoopStartOffsetInSeconds < poWave->fLengthInSeconds ), "[ FAUDIO ] Error: Invalid fFirstLoopStartOffsetInSeconds !!!" );
	//
	////

	poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_PLAYING;
	poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_STOP | _EMITTER_STATE_CHANGE_PAUSE | _EMITTER_STATE_CHANGE_UNPAUSE ) );
	poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_PLAY;
//	poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_PLAY );

//	poVirtualEmitter->fSecondsPlayed = fFirstLoopStartOffsetInSeconds;
	poVirtualEmitter->fSecondsPlayed = 0.0f;

	if( uLoops )
	{
		if( 1 < uLoops ) {
			poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_LOOPING;
		} else {
			poVirtualEmitter->uProperties &= ~_EMITTER_PROPERTIES_LOOPING;
		}	
		poVirtualEmitter->fSecondsToPlay = ( poWave->fLengthInSeconds * (f32)uLoops );
	}
	else
	{
		poVirtualEmitter->uProperties 	|= _EMITTER_PROPERTIES_LOOPING;
		poVirtualEmitter->fSecondsToPlay = FAUDIO_UNLIMITED_SECONDS;
	}

	_bSkipEmittersListenersWorkDelay = FALSE;

} // CFAudioEmitter::Play

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::Pause( BOOL bEnabled )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	if( FAUDIO_EMITTER_STATE_STOPPED == poVirtualEmitter->oeState )
	{
		return;
	}

	if( bEnabled )
	{
		if( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState )
		{
			poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_PAUSED;
			poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_STOP | _EMITTER_STATE_CHANGE_UNPAUSE ) );
			poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_PAUSE;
		}
	}
	else
	{
		if( FAUDIO_EMITTER_STATE_PAUSED == poVirtualEmitter->oeState )
		{
			poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_PLAYING;
			poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_STOP | _EMITTER_STATE_CHANGE_PAUSE ) );
#if FANG_WINGC
			// Paused before it was ever given a DirectSound voice (a sound started on the frame a level's
			// intro movie paused the audio): unpausing has no voice to resume, so ask to play it, which
			// allocates one. Without this, level ambience stayed silent for the whole level after an
			// intro movie.
			poVirtualEmitter->uStateChanges |= poVirtualEmitter->poRealEmitter ? _EMITTER_STATE_CHANGE_UNPAUSE : _EMITTER_STATE_CHANGE_PLAY;
#else
			poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_UNPAUSE;
#endif
		}
	}

} // CFAudioEmitter::Pause

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::StopAll( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	_VirtualEmitter_t *poVirtualEmitter;
	FLinkRoot_t *poVirtualEmittersList;

	for( u32 uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
	{
		// 2D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ uIndex ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			poVirtualEmitter->poAudioEmitter->Stop( FALSE );
			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}

		// 3D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			poVirtualEmitter->poAudioEmitter->Stop( FALSE );
			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

} // CFAudioEmitter::StopAll

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::Stop( BOOL bAfterCurrentLoop )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	if( FAUDIO_EMITTER_STATE_STOPPED == poVirtualEmitter->oeState )
	{
		return;
	}

	poVirtualEmitter->uProperties &= ~_EMITTER_PROPERTIES_LOOPING;

	if( ! bAfterCurrentLoop )
	{
		poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_STOPPED;
		poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_PAUSE | _EMITTER_STATE_CHANGE_UNPAUSE ) );
		poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_STOP;
	}
	else
	{
		FDataWvbFile_Wave_t *poWave      = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
		poVirtualEmitter->fSecondsToPlay = ( ( poWave->fLengthInSeconds * (f32)( (u32)( poVirtualEmitter->fSecondsPlayed / poWave->fLengthInSeconds ) ) ) + poWave->fLengthInSeconds );
	}

	_bSkipEmittersListenersWorkDelay = FALSE;

} // CFAudioEmitter::Stop

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_EmitterState_e CFAudioEmitter::GetState( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	return (FAudio_EmitterState_e) poVirtualEmitter->oeState;

} // CFAudioEmitter::GetState

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

f32 CFAudioEmitter::GetSecondsPlayed( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	return poVirtualEmitter->fSecondsPlayed;

} // CFAudioEmitter::GetSecondsPlayed

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

f32 CFAudioEmitter::GetSecondsToPlay( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	//
	////

	return poVirtualEmitter->fSecondsToPlay;

} // CFAudioEmitter::GetSecondsToPlay

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_Error_e CFAudioEmitter::Play2D(	FAudio_WaveHandle_t oWaveHandle,
										f32 fVolume /* = 1.0f */,
										u8 uPriority /* = FAudio_EmitterDefaultPriorityLevel */,
										BOOL bDuckable /* = TRUE */,
										u32 uLoops /* = 1 */,
//										f32 fFirstLoopStartOffsetInSeconds /* = 0.0f */,
										f32 fPanLeftRight /* = 0.0f */,
										f32 fFrequencyFactor /* = 1.0f */,
										CFAudioEmitter **ppUserAudioEmitter )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_ERROR;
	}

	CFAudioEmitter *poAudioEmitter = Create2D( oWaveHandle, uPriority, ppUserAudioEmitter );
	if( poAudioEmitter )
	{
		poAudioEmitter->SetVolume( fVolume );
		poAudioEmitter->SetDuckable( bDuckable );
		poAudioEmitter->SetPan( fPanLeftRight );
		poAudioEmitter->SetFrequencyFactor( fFrequencyFactor );
		poAudioEmitter->SetEndOfPlayCallback( _AutoDestroyEmitterEndOfPlayCallback );
#if !FANG_PRODUCTION_BUILD
		poAudioEmitter->m_pszModuleOwner = NULL;
		poAudioEmitter->m_nOwnerModuleLine = 0;
#endif
//		poAudioEmitter->Play( uLoops, fFirstLoopStartOffsetInSeconds );
		poAudioEmitter->Play( uLoops );
		_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)poAudioEmitter->m_pUser;
		poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_FIRENFORGET;
		return FAUDIO_NO_ERROR;
	}
	else
	{
		return FAUDIO_ERROR;
	}

} // CFAudioEmitter::Play2D

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_Error_e CFAudioEmitter::Play3D(	FAudio_WaveHandle_t oWaveHandle,
										const CFVec3A *poVecPosition_WS,
										f32 fRadiusOuter,
//										f32 fRadiusInner = DS3D_DEFAULTMINDISTANCE,
										f32 fVolume /* = 1.0f */,
										u8 uPriority /* = FAudio_EmitterDefaultPriorityLevel */,
										BOOL bDuckable /* = TRUE */,
										u32 uLoops /* = 1 */,
										f32 fFrequencyFactor /* = 1.0f */,
//										f32 fFirstLoopStartOffsetInSeconds /* = 0.0f */ )
										f32 fSpawnRadiusFactor /* = -1.0f */,
										BOOL bAutoUpdatePosition /* = FALSE */,
										CFAudioEmitter **ppUserAudioEmitter )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_ERROR;
	}

	if( ! FWorld_pWorld )
	{
		return FAUDIO_ERROR;
	}

	_VirtualListener_t *poVirtualListener;
	u32 uIndex;

	////
	//
	/*if( 0.0f < fSpawnRadiusFactor )
	{
		for( uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
		{
			poVirtualListener = &( _aoVirtualListeners[ uIndex ] );

			_oTempVec3A = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
			_oTempVec3A.Sub( *( poVecPosition_WS ) );

			if( _oTempVec3A.MagSq() <= ( fSpawnRadiusFactor * fSpawnRadiusFactor * fRadiusOuter * fRadiusOuter ) )
			{
				break;
			}
		}
		//
		////

		if( ! ( uIndex < _uActiveVirtualListeners ) )
		{
			return FAUDIO_ERROR;
		}
	}*/
	// !!Nate
	if( fSpawnRadiusFactor > 0.0f && uLoops == 1)
	{
		for( uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
		{
			poVirtualListener = &( _aoVirtualListeners[ uIndex ] );

			_oTempVec3A = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
			_oTempVec3A.Sub( *( poVecPosition_WS ) );

			if( _oTempVec3A.MagSq() < ( fSpawnRadiusFactor * fSpawnRadiusFactor * fRadiusOuter * fRadiusOuter ) )
			{
				break;
			}
		}
		//
		////

		if( uIndex == _uActiveVirtualListeners ) 
		{
			return FAUDIO_ERROR;
		}
	}

	CFAudioEmitter *poAudioEmitter = Create3D( oWaveHandle, uPriority, poVecPosition_WS, fRadiusOuter, bAutoUpdatePosition, ppUserAudioEmitter );
	if( poAudioEmitter )
	{
//		poAudioEmitter->SetRadius( fRadiusOuter, fRadiusInner );
//		poAudioEmitter->SetRadius( fRadiusOuter, DS3D_DEFAULTMINDISTANCE );
		poAudioEmitter->SetVolume( fVolume );
		poAudioEmitter->SetDuckable( bDuckable );
		poAudioEmitter->SetFrequencyFactor( fFrequencyFactor );
		poAudioEmitter->SetEndOfPlayCallback( _AutoDestroyEmitterEndOfPlayCallback );
#if !FANG_PRODUCTION_BUILD
		poAudioEmitter->m_pszModuleOwner = NULL;
		poAudioEmitter->m_nOwnerModuleLine = 0;
#endif
//		poAudioEmitter->Play( uLoops, fFirstLoopStartOffsetInSeconds );
		poAudioEmitter->Play( uLoops );
		_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)poAudioEmitter->m_pUser;
		poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_FIRENFORGET;
		return FAUDIO_NO_ERROR;
	}
	else
	{
		return FAUDIO_ERROR;
	}

} // CFAudioEmitter::Play3D

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL CFAudioEmitter::Is3D( void ) {
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	return !!( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_3D );
} // Is3D;


BOOL CFAudioEmitter::IsFireNForget( void ) {	 
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	return !!( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_FIRENFORGET );
}

void CFAudioEmitter::SetPauseLevel( FAudio_PauseLevel_e eNewPauseLevel ) {
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	poVirtualEmitter->uPauseLevel = eNewPauseLevel;
}

FAudio_PauseLevel_e CFAudioEmitter::GetPauseLevel( void ) {
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	return (FAudio_PauseLevel_e) poVirtualEmitter->uPauseLevel;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _AutoDestroyEmitterEndOfPlayCallback( CFAudioEmitter *poAudioEmitter )
{
	poAudioEmitter->Destroy();

} // _AutoDestroyEmitterEndOfPlayCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _TrackEmittersProgress( FLinkRoot_t *poVirtualEmittersListActive )
{
	FLinkRoot_t *poVirtualEmittersList;
	_VirtualEmitter_t *poVirtualEmitter;
	u32 uList;

	for( uList = 0; uList < _uMaxPriorityLevels; ++uList )
	{
		poVirtualEmittersList = &( poVirtualEmittersListActive[ uList ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			if( ( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState ) &&
				!( poVirtualEmitter->uStateChanges & _EMITTER_STATE_CHANGE_PLAY ) )
			{
				poVirtualEmitter->fSecondsPlayed += ( FLoop_fRealPreviousLoopSecs * poVirtualEmitter->fFrequencyFactor );

				if( ( 0.0f <= poVirtualEmitter->fSecondsToPlay ) &&		// Infinitely looping emitters don't stop by themselves.
					( poVirtualEmitter->fSecondsToPlay <= poVirtualEmitter->fSecondsPlayed ) )
				{
					poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_STOPPED;
					poVirtualEmitter->uStateChanges  = _EMITTER_STATE_CHANGE_STOP;
					_bSkipEmittersListenersWorkDelay = FALSE;
				}
			}

			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

} // _TrackEmittersProgress

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _ApplyRealEmittersChanges( FLinkRoot_t *poVirtualEmittersListActive ) {
	FLinkRoot_t *poVirtualEmittersList;
	_VirtualEmitter_t *poVirtualEmitter;
	FDataWvbFile_Wave_t *poWave;
	u32 uList, uNewFreq;
	LPDIRECTSOUNDBUFFER8 pDSBuffer;
	f32 fVolume;

	for( uList = 0; uList < _uMaxPriorityLevels; ++uList ) {
		poVirtualEmittersList = &poVirtualEmittersListActive[ uList ];

		poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );
		while( poVirtualEmitter ) {

			if( poVirtualEmitter->poRealEmitter ) {
				pDSBuffer = poVirtualEmitter->poRealEmitter->poDSBuffer;

				// Position, Velocity, Radius, Doppler.
				if( poVirtualEmitter->poVirtualListenerCurrent ) {

					if( ( ( _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER ) & poVirtualEmitter->uStateChanges ) ||
						( _LISTENER_STATE_CHANGE_ORIENTATION & poVirtualEmitter->poVirtualListenerCurrent->uStateChange ) ) {
						
						// Position.
						poVirtualEmitter->poVirtualListenerCurrent->poXfmCurrentOrientation_WS->TransformPointR( _oTempVec3A.v3, poVirtualEmitter->poVecCurrentPosition_WS->v3 );
#if FANG_WINGC
						{
							const f32 fGain = _GC3DDistanceGain( _oTempVec3A.Mag(), poVirtualEmitter->fRadiusOuter );
							const f32 fChange = fGain - poVirtualEmitter->fDistanceGain;
							if( poVirtualEmitter->fDistanceGain < 0.0f || fChange * fChange > _SIGNIFICANT_VOLUME_CHANGE_SQ )
							{
								poVirtualEmitter->fDistanceGain = fGain;
								poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_VOLUME;
							}
						}
#endif

						_oDSEmitterAttributes.vPosition.x = _oTempVec3A.x;
						_oDSEmitterAttributes.vPosition.y = _oTempVec3A.y;
						_oDSEmitterAttributes.vPosition.z = _oTempVec3A.z;
						
						// Velocity.
						// Emitter velocity.
						_oTempVec3A = *( poVirtualEmitter->poVecCurrentPosition_WS );
						_oTempVec3A.Sub( *( poVirtualEmitter->poVecPreviousPosition_WS ) );

						// Emitter and listener combined velocity.
						_oTempVec3A.Sub( *( poVirtualEmitter->poVirtualListenerCurrent->poVecVelocity_WS ) );
						_oTempVec3A.Mul( _fOOEmittersListenersWorkDelay );

						poVirtualEmitter->poVirtualListenerCurrent->poXfmCurrentOrientation_WS->TransformDirR( _oTempVec3A_Velocity.v3, _oTempVec3A.v3 );

						_oDSEmitterAttributes.vVelocity.x = _oTempVec3A_Velocity.x;
						_oDSEmitterAttributes.vVelocity.y = _oTempVec3A_Velocity.y;
						_oDSEmitterAttributes.vVelocity.z = _oTempVec3A_Velocity.z;

						// Radius, Doppler.						
//						_oDSEmitterAttributes.flMinDistance = poVirtualEmitter->fRadiusInner;
#if FANG_WINGC
						_oDSEmitterAttributes.flMaxDistance = poVirtualEmitter->fRadiusOuter * _GC_3D_RADIUS_SCALE;
#else
						_oDSEmitterAttributes.flMaxDistance = poVirtualEmitter->fRadiusOuter;
#endif

						if( poVirtualEmitter->poRealEmitter->poDS3DBuffer ) {
							poVirtualEmitter->poRealEmitter->poDS3DBuffer->SetAllParameters( &_oDSEmitterAttributes, DS3D_DEFERRED );
						}
					} else {
						if( poVirtualEmitter->poRealEmitter->poDS3DBuffer ) {
							poVirtualEmitter->poRealEmitter->poDS3DBuffer->SetVelocity( 0.0f, 0.0f, 0.0f, DS3D_DEFERRED );
						}
					}
				}

				// Volume.
				if( FAudio_bMasterSfxVolChanged ||
					poVirtualEmitter->uStateChanges & _EMITTER_STATE_CHANGE_VOLUME ) {
					fVolume = FAudio_fMasterSfxUnitVol * poVirtualEmitter->fVolumeDucked;
#if FANG_WINGC
					// MIDI volume as fgcaudio.cpp and MusyX compute it, then MusyX's squared DLS law
					fVolume = _GCMusyxVolume( fVolume );
					if( poVirtualEmitter->poRealEmitter->poDS3DBuffer )
					{
						// Not computed yet (no listener update): the flat 3D scale, never silence.
						fVolume *= ( poVirtualEmitter->fDistanceGain < 0.0f ) ? _GC_3D_VOLUME_SCALE : poVirtualEmitter->fDistanceGain;
					}
					pDSBuffer->SetVolume( _GainToDSVolume( fVolume * fVolume ) );
#else
					pDSBuffer->SetVolume( _anVolumes[ fmath_FloatToU32( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * fVolume ) ] );
#endif
				}
				
#if FANG_WINGC
				if( _bMixSnapshot && poVirtualEmitter->oWaveHandle ) {
					f32 fMidi = _GCMusyxVolume( FAudio_fMasterSfxUnitVol * poVirtualEmitter->fVolumeDucked );
					const BOOL b3D = poVirtualEmitter->poRealEmitter->poDS3DBuffer != NULL;
					if( b3D ) {
						fMidi *= ( poVirtualEmitter->fDistanceGain < 0.0f ) ? _GC_3D_VOLUME_SCALE : poVirtualEmitter->fDistanceGain;
					}
					DEVPRINTF( "PORT-MIX   %s '%s' vol=%.2f ducked=%.2f dist=%.2f radius=%.0f -> amp %.3f\n", b3D ? "3D" : "2D",
						((FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle)->szName, poVirtualEmitter->fVolume, poVirtualEmitter->fVolumeDucked,
						poVirtualEmitter->fDistanceGain, poVirtualEmitter->fRadiusOuter, fMidi * fMidi );
				}
#endif
				// Frequency.
				if( _EMITTER_STATE_CHANGE_FREQUENCY & poVirtualEmitter->uStateChanges ) {
					poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
					uNewFreq = fmath_FloatToU32( poVirtualEmitter->fFrequencyFactor * poWave->fFreqHz );
					FMATH_CLAMP( uNewFreq, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX );
					pDSBuffer->SetFrequency( uNewFreq );
				}

				// Pan.
				if( _EMITTER_STATE_CHANGE_PAN & poVirtualEmitter->uStateChanges ) {
					
					if( 0.0f <= poVirtualEmitter->fPanLeftRight ) {
						pDSBuffer->SetPan( - _anVolumes[ fmath_FloatToU32( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * ( 1.0f - poVirtualEmitter->fPanLeftRight ) ) ] );
					} else {
						pDSBuffer->SetPan( + _anVolumes[ fmath_FloatToU32( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * ( 1.0f + poVirtualEmitter->fPanLeftRight ) ) ] );
					}
				}

				// Play / unpause.
				if( ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_UNPAUSE ) & poVirtualEmitter->uStateChanges ) {
					poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;

					if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_LOOPING ) {
						pDSBuffer->Play( 0, 0, DSBPLAY_LOOPING );
					} else {
						pDSBuffer->Play( 0, 0, 0 );
					}
				}
			}

			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}
} // _ApplyRealEmittersChanges

void _InvokeEmittersEndofplayCallbacks( FLinkRoot_t *poVirtualEmittersListActive )
{
	FLinkRoot_t *poVirtualEmittersList;
	_VirtualEmitter_t *poVirtualEmitter, *poVirtualEmitterNext;
	u32 uList;

	for( uList = 0; uList < _uMaxPriorityLevels; ++uList )
	{
		poVirtualEmittersList = &( poVirtualEmittersListActive[ uList ] );

		poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );
		while( poVirtualEmitter )
		{
			if( ( _EMITTER_STATE_CHANGE_STOP & poVirtualEmitter->uStateChanges ) &&
				( !( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_LOOPING ) ) )
			{
				poVirtualEmitterNext = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );

				poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;

				if( poVirtualEmitter->pEndOfPlayCallback )
				{
					poVirtualEmitter->pEndOfPlayCallback( poVirtualEmitter->poAudioEmitter );
				}

				poVirtualEmitter = poVirtualEmitterNext;
				continue;
			}

			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

} // _InvokeEmittersEndofplayCallbacks

static void _DestroyAllEmittersFromABank( FAudio_BankHandle_t hBank ) {
	_VirtualEmitter_t *poVirtualEmitter, *pNextVEmitter;
	FLinkRoot_t *poVirtualEmittersList;
	FDataWvbFile_Wave_t *pWave;

	for( u32 uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex ) {
		// 2D.
		poVirtualEmittersList = &_paoVirtualEmittersListActive2D[ uIndex ];

		poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );
		while( poVirtualEmitter ) {
			// grab the next virtual emitter
			pNextVEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );

			pWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
			if( pWave->oBankHandle == hBank ) {
				poVirtualEmitter->poAudioEmitter->Destroy();
			}
			// move to the next emitter
			poVirtualEmitter = pNextVEmitter;
		}

		// 3D.
		poVirtualEmittersList = &_paoVirtualEmittersListActive3D[ uIndex ];

		poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );
		while( poVirtualEmitter ) {
			// grab the next virtual emitter
			pNextVEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );

			pWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
			if( pWave->oBankHandle == hBank ) {
				poVirtualEmitter->poAudioEmitter->Destroy();
			}
			// move to the next emitter
			poVirtualEmitter = pNextVEmitter;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////
// CFAudioStream Methods - the windows platform has no streaming support currently
//////////////////////////////////////////////////////////////////////////////////
#if FANG_WINGC

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Streams (music and speech).
//
// The retail streams are GameCube .wvs files: mono or stereo DSP-ADPCM. Create() opens the file,
// checks its header and makes one DirectSound buffer for the whole decoded stream; a worker
// thread then reads and decodes the file into that buffer while the stream is CREATING. The game
// waits for STOPPED before it calls Play() (as it did for the GameCube's asynchronous reads), and
// from then on DirectSound plays the buffer by itself, so frame hitches never starve it.

#define _MAX_TOTAL_STREAMS							( 4 )

struct _Stream_t
{
	CFAudioStream oAudioStream;
	char szName[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
	BOOL bActive;
	BOOL bTreatAsSfx;
	BOOL bIgnoreInPauseMode;
	BOOL bLooping;					// The DirectSound buffer is playing with DSBPLAY_LOOPING.
	BOOL bEndOfPlayPending;			// Stopped; the end-of-play callback has not run yet.
	FAudio_StreamState_e oeState;
	FAudio_PauseLevel_e ePauseLevel;
	CFAudioStream::FAudio_StreamEndOfPlayCallback_t *pEndOfPlayCallback;

	GCAudioStreamInfo_t oInfo;
	LPDIRECTSOUNDBUFFER poDSBuffer;
	u32 uBufferBytes;
	u32 uBlockAlign;
	DWORD uLastPlayCursor;

	f32 fVolume;
	f32 fPanLeftRight;
	f32 fFrequencyFactor;
	f32 fSecondsPlayed;
	f32 fSecondsToPlay;

	struct _StreamJob_t *pJob;		// While FAUDIO_STREAM_STATE_CREATING: the load on its worker thread.
};

// A stream is loaded on a worker thread: its file read, its DirectSound buffer made and filled with the
// decoded track. The game thread only starts the job and, once it is done, takes the buffer; nothing on
// it waits for the disk or DirectSound (reading a stream's header on the game thread alone stalled
// frames 100+ ms when the disk was busy). A stream destroyed while loading is abandoned to its worker,
// which cleans up; the job is shared by reference count.
struct _StreamJob_t
{
	volatile LONG nRefs;			// The stream's and the worker's.
	volatile LONG nCancel;
	volatile LONG nResult;			// 0 while loading, 1 done, -1 failed. The worker touches nothing but nRefs after setting it.
	char szPath[ MAX_PATH ];
	char szName[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
	GCAudioStreamInfo_t oInfo;
	LPDIRECTSOUNDBUFFER poDSBuffer;
	u32 uBufferBytes;
	u32 uBlockAlign;
};

static volatile LONG _nStreamJobsRunning;

static _Stream_t _aoStreams[ _MAX_TOTAL_STREAMS ];
static u32 _uMaxStreams;
static FAudio_PauseLevel_e _ePauseLevelStreams;

static void _InitStreams( u32 uMaxStreams )
{
	_uMaxStreams        = FMATH_MIN( uMaxStreams, (u32)_MAX_TOTAL_STREAMS );
	_ePauseLevelStreams = FAUDIO_PAUSE_LEVEL_NONE;

	for( u32 uIndex = 0; uIndex < _MAX_TOTAL_STREAMS; ++uIndex )
	{
		_Stream_t *poStream = &( _aoStreams[ uIndex ] );
		fang_MemZero( poStream, sizeof( *poStream ) );
		poStream->oAudioStream.m_uData = (u32)poStream;
	}
}

static void _ReleaseStreamJob( _StreamJob_t *pJob )
{
	if( 0 == InterlockedDecrement( &pJob->nRefs ) )
	{
		FDX8_SAFE_RELEASE( pJob->poDSBuffer );
		free( pJob );
	}
}

static BOOL _LoadStream( _StreamJob_t *pJob )
{
	HANDLE hFile = CreateFileA( pJob->szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not open stream \"%s\" !!!\n", __LINE__, pJob->szPath );
		return FALSE;
	}
	const DWORD uFileBytes = GetFileSize( hFile, NULL );
	u8 *pFile = ( INVALID_FILE_SIZE != uFileBytes ) ? (u8 *)malloc( uFileBytes ) : NULL;
	DWORD uRead = 0;
	const BOOL bRead = pFile && ReadFile( hFile, pFile, uFileBytes, &uRead, NULL ) && ( uRead == uFileBytes );
	CloseHandle( hFile );
	if( ( ! bRead ) || ( ! gcaudio_ReadStreamHeader( pFile, uFileBytes, uFileBytes, &pJob->oInfo ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: \"%s\" is not a GameCube stream this port plays !!!\n", __LINE__, pJob->szPath );
		free( pFile );
		return FALSE;
	}
	if( pJob->nCancel )
	{
		free( pFile );
		return FALSE;
	}

	WAVEFORMATEX oFormat;
	fang_MemZero( &oFormat, sizeof( oFormat ) );
	oFormat.wFormatTag      = WAVE_FORMAT_PCM;
	oFormat.nChannels       = (WORD)pJob->oInfo.nChannels;
	oFormat.nSamplesPerSec  = pJob->oInfo.nRate;
	oFormat.wBitsPerSample  = 16;
	oFormat.nBlockAlign     = (WORD)( 2 * pJob->oInfo.nChannels );
	oFormat.nAvgBytesPerSec = pJob->oInfo.nRate * oFormat.nBlockAlign;

	DSBUFFERDESC oDescription;
	fang_MemZero( &oDescription, sizeof( oDescription ) );
	oDescription.dwSize        = sizeof( oDescription );
	oDescription.dwFlags       = ( DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 );
	oDescription.dwBufferBytes = pJob->oInfo.nSamplesPerChannel * oFormat.nBlockAlign;
	oDescription.lpwfxFormat   = &oFormat;

	void *pPcm = NULL;
	DWORD uPcmBytes = 0;
	if( FAILED( _poDS->CreateSoundBuffer( &oDescription, &pJob->poDSBuffer, NULL ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: CreateSoundBuffer() failed for stream '%s' (%u bytes) !!!\n", __LINE__, pJob->szName, oDescription.dwBufferBytes );
		pJob->poDSBuffer = NULL;
		free( pFile );
		return FALSE;
	}
	if( FAILED( pJob->poDSBuffer->Lock( 0, 0, &pPcm, &uPcmBytes, NULL, NULL, DSBLOCK_ENTIREBUFFER ) ) || ( uPcmBytes < oDescription.dwBufferBytes ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Lock() failed for stream '%s' !!!\n", __LINE__, pJob->szName );
		free( pFile );
		return FALSE;
	}
	const BOOL bDecoded = gcaudio_DecodeStream( pFile, uFileBytes, &pJob->oInfo, (s16 *)pPcm, &pJob->nCancel );
	pJob->poDSBuffer->Unlock( pPcm, uPcmBytes, NULL, 0 );
	free( pFile );
	pJob->uBufferBytes = oDescription.dwBufferBytes;
	pJob->uBlockAlign  = oFormat.nBlockAlign;
	return bDecoded;
}

static DWORD WINAPI _StreamLoadThread( void *pParam )
{
	_StreamJob_t *pJob = (_StreamJob_t *)pParam;
	const BOOL bOK = _LoadStream( pJob );
	InterlockedExchange( &pJob->nResult, bOK ? 1 : -1 );
	_ReleaseStreamJob( pJob );
	InterlockedDecrement( &_nStreamJobsRunning );
	return 0;
}

// Audio shutdown: streams still loading must finish with DirectSound before it goes away.
static void _WaitForStreamLoads( void )
{
	for( u32 uWaited = 0; ( 0 < InterlockedCompareExchange( &_nStreamJobsRunning, 0, 0 ) ) && ( uWaited < 5000 ); uWaited += 10 )
	{
		Sleep( 10 );
	}
}

static void _ApplyStreamMix( _Stream_t *poStream )
{
	if( ( ! poStream->poDSBuffer ) || ( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) || ( FAUDIO_STREAM_STATE_ERROR == poStream->oeState ) )
	{
		return;
	}

	f32 fVolume = ( poStream->bTreatAsSfx ? FAudio_fMasterSfxUnitVol : FAudio_fMasterMusicUnitVol ) * poStream->fVolume;
	poStream->poDSBuffer->SetVolume( _GainToDSVolume( _GCStreamGain( fVolume, poStream->oInfo.nChannels ) ) );

	f32 fPan = poStream->fPanLeftRight;
	FMATH_CLAMP( fPan, -1.0f, 1.0f );
	if( 0.0f <= fPan )
	{
		poStream->poDSBuffer->SetPan( - _anVolumes[ fmath_FloatToU32( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * ( 1.0f - fPan ) ) ] );
	}
	else
	{
		poStream->poDSBuffer->SetPan( + _anVolumes[ fmath_FloatToU32( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * ( 1.0f + fPan ) ) ] );
	}

	u32 uFrequency = fmath_FloatToU32( poStream->fFrequencyFactor * (f32)poStream->oInfo.nRate );
	FMATH_CLAMP( uFrequency, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX );
	poStream->poDSBuffer->SetFrequency( uFrequency );
}

static void _StopStreamBuffer( _Stream_t *poStream )
{
	if( poStream->poDSBuffer )
	{
		poStream->poDSBuffer->Stop();
		poStream->poDSBuffer->SetCurrentPosition( 0 );
	}
	poStream->uLastPlayCursor = 0;
	poStream->bLooping        = FALSE;
}

static void _EndStream( _Stream_t *poStream )
{
	_StopStreamBuffer( poStream );
	poStream->oeState           = FAUDIO_STREAM_STATE_STOPPED;
	poStream->bEndOfPlayPending = TRUE;
}

static void _StreamsWork( void )
{
	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		_Stream_t *poStream = &( _aoStreams[ uIndex ] );

		if( ! poStream->bActive )
		{
			continue;
		}

		if( FAUDIO_STREAM_STATE_CREATING == poStream->oeState )
		{
			_StreamJob_t *pJob = poStream->pJob;
			const LONG nResult = InterlockedCompareExchange( &pJob->nResult, 0, 0 );
			if( 0 == nResult )
			{
				continue;
			}
			if( 1 == nResult )
			{
				// the job is done with the buffer: take it
				poStream->oInfo        = pJob->oInfo;
				poStream->poDSBuffer   = pJob->poDSBuffer;
				poStream->uBufferBytes = pJob->uBufferBytes;
				poStream->uBlockAlign  = pJob->uBlockAlign;
				pJob->poDSBuffer       = NULL;
				DEVPRINTF( "[ FAUDIO ] Stream '%s' ready: %u channel(s), %u Hz, %.1f seconds.\n", poStream->szName, poStream->oInfo.nChannels, poStream->oInfo.nRate, poStream->oInfo.fSeconds );
				poStream->oeState = FAUDIO_STREAM_STATE_STOPPED;
				_ApplyStreamMix( poStream );
			}
			else
			{
				DEVPRINTF( "[ FAUDIO ] Error %u: Could not read or decode stream '%s' !!!\n", __LINE__, poStream->szName );
				poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
			}
			poStream->pJob = NULL;
			_ReleaseStreamJob( pJob );
			continue;
		}

		if( FAUDIO_STREAM_STATE_ERROR == poStream->oeState )
		{
			continue;
		}

		if( poStream->bTreatAsSfx ? FAudio_bMasterSfxVolChanged : FAudio_bMasterMusicVolChanged )
		{
			_ApplyStreamMix( poStream );
		}

		if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState )
		{
			if( _bMixSnapshot )
			{
				DEVPRINTF( "PORT-MIX   stream '%s' vol=%.2f channels=%u -> amp %.3f\n", poStream->szName, poStream->fVolume, poStream->oInfo.nChannels,
					_GCStreamGain( poStream->fVolume * ( poStream->bTreatAsSfx ? FAudio_fMasterSfxUnitVol : FAudio_fMasterMusicUnitVol ), poStream->oInfo.nChannels ) );
			}
			DWORD uStatus = 0, uPlayCursor = 0;
			poStream->poDSBuffer->GetStatus( &uStatus );

			if( ! ( DSBSTATUS_PLAYING & uStatus ) )
			{
				// A buffer played without looping has reached its end.
				poStream->fSecondsPlayed = poStream->fSecondsToPlay;
				_EndStream( poStream );
			}
			else if( SUCCEEDED( poStream->poDSBuffer->GetCurrentPosition( &uPlayCursor, NULL ) ) )
			{
				const u32 uPlayed = ( uPlayCursor + poStream->uBufferBytes - poStream->uLastPlayCursor ) % poStream->uBufferBytes;
				poStream->uLastPlayCursor = uPlayCursor;
				poStream->fSecondsPlayed += (f32)uPlayed / (f32)( poStream->oInfo.nRate * poStream->uBlockAlign );

				// Once the last loop has started, let the buffer stop at its end.
				if( poStream->bLooping && ( 0.0f <= poStream->fSecondsToPlay ) &&
					( poStream->fSecondsToPlay - poStream->oInfo.fSeconds <= poStream->fSecondsPlayed ) )
				{
					poStream->poDSBuffer->Play( 0, 0, 0 );
					poStream->bLooping = FALSE;
				}
			}
		}

		if( poStream->bEndOfPlayPending )
		{
			poStream->bEndOfPlayPending = FALSE;
			if( poStream->pEndOfPlayCallback )
			{
				poStream->pEndOfPlayCallback( &( poStream->oAudioStream ) );
			}
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CFAudioStream *CFAudioStream::Create( cchar *pszName, BOOL bWillBeUsedForMusic/*=TRUE*/ )
{
	if( ! FAudio_bModuleInstalled )
	{
		return NULL;
	}

	if( ( ! pszName ) || ( ! *pszName ) || ( fclib_strlen( pszName ) > FAUDIO_MAX_ASSET_NAME_LENGTH ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioStream::Create() was given an invalid stream name !!!\n", __LINE__ );
		return NULL;
	}

	u32 uIndex;
	for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		if( ! _aoStreams[ uIndex ].bActive )
		{
			break;
		}
	}
	if( uIndex == _uMaxStreams )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: No free stream for '%s' !!!\n", __LINE__, pszName );
		return NULL;
	}
	_Stream_t *poStream = &( _aoStreams[ uIndex ] );

	// Streams are loose files next to the master file, as on the GameCube. Only whether it exists is
	// checked here (so a missing stream still fails at once, as the callers expect); the load runs on
	// its own thread.
	_StreamJob_t *pJob = (_StreamJob_t *)malloc( sizeof( _StreamJob_t ) );
	if( ! pJob )
	{
		return NULL;
	}
	fang_MemZero( pJob, sizeof( *pJob ) );
	_snprintf( pJob->szPath, sizeof( pJob->szPath ), "%s%s.wvs", Fang_ConfigDefs.pszFile_GameRootPathName ? Fang_ConfigDefs.pszFile_GameRootPathName : "", pszName );
	pJob->szPath[ sizeof( pJob->szPath ) - 1 ] = 0;
	_snprintf( pJob->szName, sizeof( pJob->szName ) - 1, "%s", pszName );
	if( INVALID_FILE_ATTRIBUTES == GetFileAttributesA( pJob->szPath ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not open stream \"%s\" !!!\n", __LINE__, pJob->szPath );
		free( pJob );
		return NULL;
	}

	fang_MemZero( poStream, sizeof( *poStream ) );
	poStream->oAudioStream.m_uData = (u32)poStream;
	fclib_strcpy( poStream->szName, pszName );
	poStream->bActive          = TRUE;
	poStream->bTreatAsSfx      = ! bWillBeUsedForMusic;
	poStream->oeState          = FAUDIO_STREAM_STATE_CREATING;
	poStream->ePauseLevel      = FAUDIO_PAUSE_LEVEL_1;
	poStream->fVolume          = 1.0f;
	poStream->fFrequencyFactor = 1.0f;
	poStream->pJob             = pJob;

	pJob->nRefs = 2;
	InterlockedIncrement( &_nStreamJobsRunning );
	HANDLE hThread = CreateThread( NULL, 0, _StreamLoadThread, pJob, 0, NULL );
	if( hThread )
	{
		CloseHandle( hThread );
	}
	else
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not start loading stream '%s' !!!\n", __LINE__, pszName );
		InterlockedDecrement( &_nStreamJobsRunning );
		pJob->nRefs = 1;
		pJob->nResult = -1;
	}

	return &( poStream->oAudioStream );

} // CFAudioStream::Create

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Destroy( void )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	if( ! poStream->bActive )
	{
		return;
	}

	if( FAUDIO_STREAM_STATE_CREATING == poStream->oeState )
	{
		// leave the load to its worker, which stops early and cleans up
		InterlockedExchange( &poStream->pJob->nCancel, 1 );
		_ReleaseStreamJob( poStream->pJob );
		poStream->pJob    = NULL;
		poStream->oeState = FAUDIO_STREAM_STATE_STOPPED;
	}

	Stop( FALSE );

	if( poStream->bEndOfPlayPending && poStream->pEndOfPlayCallback )
	{
		poStream->pEndOfPlayCallback( this );
	}

	FDX8_SAFE_RELEASE( poStream->poDSBuffer );
	poStream->bEndOfPlayPending = FALSE;
	poStream->bActive           = FALSE;

} // CFAudioStream::Destroy

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::DestroyAll( void )
{
	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		if( _aoStreams[ uIndex ].bActive )
		{
			_aoStreams[ uIndex ].oAudioStream.Destroy();
		}
	}

} // CFAudioStream::DestroyAll

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetVolume( f32 fVolume )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	FMATH_CLAMP( fVolume, 0.0f, 1.0f );
	if( fVolume != poStream->fVolume )
	{
		poStream->fVolume = fVolume;
		_ApplyStreamMix( poStream );
	}

} // CFAudioStream::SetVolume

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetPan( f32 fPanLeftRight )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	FMATH_CLAMP( fPanLeftRight, -1.0f, 1.0f );
	if( fPanLeftRight != poStream->fPanLeftRight )
	{
		poStream->fPanLeftRight = fPanLeftRight;
		_ApplyStreamMix( poStream );
	}

} // CFAudioStream::SetPan

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetEndOfPlayCallback( FAudio_StreamEndOfPlayCallback_t *pEndOfPlayCallback )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	poStream->pEndOfPlayCallback = pEndOfPlayCallback;

} // CFAudioStream::SetEndOfPlayCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetFrequencyFactor( f32 fFrequencyFactor )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	FMATH_CLAMPMIN( fFrequencyFactor, 0.0f );
	if( fFrequencyFactor != poStream->fFrequencyFactor )
	{
		poStream->fFrequencyFactor = fFrequencyFactor;
		_ApplyStreamMix( poStream );
	}

} // CFAudioStream::SetFrequencyFactor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Play( u32 uLoops /* = 1 */ )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	if( ( FAUDIO_STREAM_STATE_PLAYING  == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	poStream->fSecondsPlayed = 0.0f;
	if( Fang_bPortDiag ) {
		DEVPRINTF( "PORT-SND stream '%s' vol=%.2f channels=%u music master=%.2f sfx master=%.2f\n", poStream->szName, poStream->fVolume, poStream->oInfo.nChannels, FAudio_fMasterMusicUnitVol, FAudio_fMasterSfxUnitVol );
	}
	if( uLoops )
	{
		poStream->bLooping       = ( 1 < uLoops );
		poStream->fSecondsToPlay = ( poStream->oInfo.fSeconds * (f32)uLoops );
	}
	else
	{
		poStream->bLooping       = TRUE;
		poStream->fSecondsToPlay = FAUDIO_UNLIMITED_SECONDS;
	}

	poStream->poDSBuffer->Stop();
	poStream->poDSBuffer->SetCurrentPosition( 0 );
	poStream->uLastPlayCursor = 0;
	_ApplyStreamMix( poStream );

	if( FAILED( poStream->poDSBuffer->Play( 0, 0, poStream->bLooping ? DSBPLAY_LOOPING : 0 ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not play stream '%s' !!!\n", __LINE__, poStream->szName );
		poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
		return;
	}
	poStream->oeState = FAUDIO_STREAM_STATE_PLAYING;

} // CFAudioStream::Play

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Pause( BOOL bEnabled )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	if( bEnabled && ( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState ) )
	{
		// Stop() keeps the play position.
		poStream->poDSBuffer->Stop();
		poStream->oeState = FAUDIO_STREAM_STATE_PAUSED;
	}
	else if( ( ! bEnabled ) && ( FAUDIO_STREAM_STATE_PAUSED == poStream->oeState ) )
	{
		poStream->poDSBuffer->Play( 0, 0, poStream->bLooping ? DSBPLAY_LOOPING : 0 );
		poStream->oeState = FAUDIO_STREAM_STATE_PLAYING;
	}

} // CFAudioStream::Pause

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Stop( BOOL bAfterCurrentLoop )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	if( ( FAUDIO_STREAM_STATE_PLAYING != poStream->oeState ) &&
		( FAUDIO_STREAM_STATE_PAUSED  != poStream->oeState ) )
	{
		return;
	}

	if( bAfterCurrentLoop )
	{
		poStream->fSecondsToPlay = ( poStream->oInfo.fSeconds * (f32)( (u32)( poStream->fSecondsPlayed / poStream->oInfo.fSeconds ) ) ) + poStream->oInfo.fSeconds;
		if( poStream->bLooping )
		{
			poStream->bLooping = FALSE;
			if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState )
			{
				poStream->poDSBuffer->Play( 0, 0, 0 );
			}
		}
		return;
	}

	_EndStream( poStream );

} // CFAudioStream::Stop

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_StreamState_e CFAudioStream::GetState( void )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	return poStream->oeState;

} // CFAudioStream::GetState

CFAudioStream *CFAudioStream::GetStream( u32 uStreamIdx )
{
	if( uStreamIdx < _uMaxStreams )
	{
		return &( _aoStreams[ uStreamIdx ].oAudioStream );
	}
	return NULL;
}

f32 CFAudioStream::GetSecondsPlayed( void )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	return poStream->fSecondsPlayed;
}

f32 CFAudioStream::GetSecondsToPlay( void )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	return poStream->fSecondsToPlay;
}

// Pauses every stream whose pause level is at or below the new level, and resumes the ones
// an earlier call paused. Returns the previous level.
FAudio_PauseLevel_e CFAudioStream::SetGlobalPauseLevel( FAudio_PauseLevel_e eNewPauseLevel )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_PAUSE_LEVEL_NONE;
	}

	FAudio_PauseLevel_e eOldPauseLevel = _ePauseLevelStreams;

	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		_Stream_t *poStream = &( _aoStreams[ uIndex ] );

		if( ! poStream->bActive )
		{
			continue;
		}

		if( poStream->ePauseLevel <= eNewPauseLevel )
		{
			poStream->oAudioStream.Pause( TRUE );
			poStream->bIgnoreInPauseMode = TRUE;
		}
		else if( poStream->bIgnoreInPauseMode )
		{
			poStream->oAudioStream.Pause( FALSE );
			poStream->bIgnoreInPauseMode = FALSE;
		}
	}
	_ePauseLevelStreams = eNewPauseLevel;

	return eOldPauseLevel;
}

FAudio_PauseLevel_e CFAudioStream::GetGlobalPauseLevel( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_PAUSE_LEVEL_NONE;
	}

	return _ePauseLevelStreams;
}

u32 CFAudioStream::GetNumActive( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return 0;
	}

	u32 uCount = 0;
	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		if( _aoStreams[ uIndex ].bActive )
		{
			++uCount;
		}
	}
	return uCount;
}

cchar *CFAudioStream::GetName( void )
{
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bActive, "[ FAUDIO ] Error: Invalid stream !!!" );

	return poStream->szName;
}

cchar *CFAudioStream::GetName( u32 uIndex )
{
	if( ! FAudio_bModuleInstalled )
	{
		return NULL;
	}

	u32 uCount = 0;
	for( u32 uStream = 0; uStream < _uMaxStreams; ++uStream )
	{
		if( _aoStreams[ uStream ].bActive )
		{
			if( uCount == uIndex )
			{
				return _aoStreams[ uStream ].szName;
			}
			++uCount;
		}
	}
	return NULL;
}

#else // FANG_WINGC

CFAudioStream *CFAudioStream::Create( cchar *pszName, BOOL bWillBeUsedForMusic/*=TRUE*/ ) {
	return NULL;
}

void CFAudioStream::Destroy( void ) {
}

void CFAudioStream::DestroyAll( void ) {
}

void CFAudioStream::SetVolume( f32 fVolume ) {
}

void CFAudioStream::SetPan( f32 fPanLeftRight ) {
}

void CFAudioStream::SetEndOfPlayCallback( FAudio_StreamEndOfPlayCallback_t *pEndOfPlayCallback ) {
}

void CFAudioStream::SetFrequencyFactor( f32 fFrequencyFactor ) {
}

void CFAudioStream::Play( u32 uLoops /* = 1 */ ) {
}

void CFAudioStream::Pause( BOOL bEnabled ) {
}

void CFAudioStream::Stop( BOOL bAfterCurrentLoop ) {
}

FAudio_StreamState_e CFAudioStream::GetState( void ) {
	return FAUDIO_STREAM_STATE_ERROR;
}

CFAudioStream *CFAudioStream::GetStream( u32 uStreamIdx ) {
	return NULL;
}

f32 CFAudioStream::GetSecondsPlayed( void ) {
	return 0.0f;
}

f32 CFAudioStream::GetSecondsToPlay( void ) {
	return 0.0f;
}

FAudio_PauseLevel_e CFAudioStream::SetGlobalPauseLevel( FAudio_PauseLevel_e eNewPauseLevel ) {
	return eNewPauseLevel;
}

FAudio_PauseLevel_e CFAudioStream::GetGlobalPauseLevel( void ) {
	return FAUDIO_PAUSE_LEVEL_NONE;
}

u32 CFAudioStream::GetNumActive( void ) {
	return 0;
}

cchar *CFAudioStream::GetName( void ) {
	return NULL;
}

cchar *CFAudioStream::GetName( u32 uIndex ) {
	return NULL;
}

#endif // FANG_WINGC
