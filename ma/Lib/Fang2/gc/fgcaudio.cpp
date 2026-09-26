//////////////////////////////////////////////////////////////////////////////////////
// fgcaudio.cpp - 
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
#include <stdio.h>
#include <dolphin.h>
#include "musyx.h"

#include "fang.h"
#include "fgc.h"
#include "floop.h"
#include "faudio.h"
#include "fresload.h"
#include "fclib.h"
#include "ffile.h"
#include "floop.h"
#include "fsysinfo.h"
#include "fgcdata.h"
#include "fperf.h"


#define _EMITTERS_LISTENERS_WORK_DELAY					( 0.10f ) //  10 fps.

#define _STREAMS_WORK_DELAY								( 0.00f ) // full fps.

// Made this delay longer so that there is time to kill the sound
#define _3D_EMITTER_MAX_START_DELAY						( 5.0f )

#define _MUSYX_REAL_EMITTERS							( SYNTH_MAX_VOICES )
#define _MUSYX_REAL_EMITTERS_MAX_3D						( 32 )
#define _MUSYX_REAL_EMITTERS_MAX_2D						( 32 )

// Musyx callback is invoked about every 0.020 sec.
// 14 samples takes 8 bytes of GC ADPCM. Rounded up to 32 bytes: 56 samples.
// 20ms at 44100hz, takes 882 samples, rounded up: 896. Double buffered: 1792.
// 1792 samples takes 1024 bytes of GC ADPCM.
#define _MUSYX_SAMPLES_BUFFERED							( 88200 ) // 50400 bytes.

// Does not include bank headers, but does include wave headers and wave data.
#define _MAX_TOTAL_WAVE_DATA_SIZE						( 16 * 1024 * 1024 )

#define _MAX_TOTAL_STREAMS								( 4 )

#define _SIGNIFICANT_DISTANCE_CHANGE					( 0.1f )
#define _SIGNIFICANT_DISTANCE_CHANGE_SQ					( _SIGNIFICANT_DISTANCE_CHANGE * _SIGNIFICANT_DISTANCE_CHANGE )

#define _SIGNIFICANT_DOT_CHANGE							( 1.0f - 0.001f ) //+- 2.562 degrees
#define _SIGNIFICANT_DOT_CHANGE_SQ						( _SIGNIFICANT_DOT_CHANGE * _SIGNIFICANT_DOT_CHANGE )

#define _SIGNIFICANT_FACTOR_CHANGE						( 0.001f )
#define _SIGNIFICANT_FACTOR_CHANGE_SQ					( _SIGNIFICANT_FACTOR_CHANGE * _SIGNIFICANT_FACTOR_CHANGE )

#define _SIGNIFICANT_VOLUME_CHANGE						( 1.0f / 127.0f )
#define _SIGNIFICANT_VOLUME_CHANGE_SQ					( _SIGNIFICANT_VOLUME_CHANGE * _SIGNIFICANT_VOLUME_CHANGE )

#define _MUSYX_ADPCM_SAMPLES_TO_BYTES( s )				( ( ( s ) << 2 ) / 7 )
#define _MUSYX_BYTES_TO_ADPCM_SAMPLES( b )				( ( ( b ) * 7 ) >> 2 )

#if 1
	#define _EMITTERS_CREATE_MAX_ERRORS					( 5 )
#else
	#define _EMITTERS_CREATE_MAX_ERRORS					( 5000000 )
#endif

#define _SND_MASTER_VOLUME 								( 127 )

// !!Nate - These are only used for 3D sounds.  These scales are for when the sounds are added via the 
// 			sndAddEmitter() or the sndUpdateEmitter().  The scaled values are not stored in the sound
//			structures.
#define _3D_SOUND_RADIUS_SCALE							( 1.25f )
#define _3D_SOUND_VOLUME_SCALE							( 0.80f )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

enum _EmitterProperties_e {
	_EMITTER_PROPERTIES_3D					= 0x0001,
	_EMITTER_PROPERTIES_FIRENFORGET			= 0x0002,
	_EMITTER_PROPERTIES_AUTOPOSUPDATE 		= 0x0004,
	_EMITTER_PROPERTIES_DUCKABLE			= 0x0008,
	_EMITTER_PROPERTIES_LOOPING				= 0x0010,
	_EMITTER_PROPERTIES_IGNOREINPAUSEMODE	= 0x0020,
			
	_EMITTER_PROPERTIES_NONE				= 0x0000
}; // _EmitterProperties_e

enum _EmitterStateChange_e {
	// Consumed by faudio_Work().
	_EMITTER_STATE_CHANGE_NONE				= 0x00000000,
	_EMITTER_STATE_CHANGE_PLAY				= 0x00000001,
	_EMITTER_STATE_CHANGE_STOP				= 0x00000002,
	_EMITTER_STATE_CHANGE_PAUSE				= 0x00000004,
	_EMITTER_STATE_CHANGE_UNPAUSE			= 0x00000008,
	_EMITTER_STATE_CHANGE_VOLUME			= 0x00000010,
	_EMITTER_STATE_CHANGE_FREQUENCY			= 0x00000020,
	_EMITTER_STATE_CHANGE_POSITION			= 0x00000040,
	_EMITTER_STATE_CHANGE_PAN				= 0x00000080,
	_EMITTER_STATE_CHANGE_DOPPLER			= 0x00000100,
	_EMITTER_STATE_CHANGE_REVERB			= 0x00000200,

	// Consumed by CFAudioEmitter::SetVolume().
	_EMITTER_STATE_CHANGE_DUCKABLE			= 0x00000400,
	_EMITTER_STATE_CHANGE_DUCKING			= 0x00000800
}; // _EmitterStateChange_e

enum _StreamStateChange_e {
	// Consumed by faudio_Work().
	_STREAM_STATE_CHANGE_NONE				= 0x00000000,
	_STREAM_STATE_CHANGE_PLAY				= 0x00000001,
	_STREAM_STATE_CHANGE_STOP				= 0x00000002,
	_STREAM_STATE_CHANGE_PAUSE				= 0x00000004,
	_STREAM_STATE_CHANGE_UNPAUSE			= 0x00000008,
	_STREAM_STATE_CHANGE_VOLUME				= 0x00000010,
	_STREAM_STATE_CHANGE_FREQUENCY			= 0x00000020,
	_STREAM_STATE_CHANGE_PAN				= 0x00000040

}; // _StreamStateChange_e

enum _StreamFileChange_e {
	// Consumed by faudio_Work().
	_STREAM_FILE_CHANGE_NONE				= 0x00000000,
	_STREAM_FILE_CHANGE_READ_HEADER			= 0x00000001,
	_STREAM_FILE_CHANGE_READ_BODY			= 0x00000002,
	_STREAM_FILE_CHANGE_READ_BODY_PENDING	= 0x00000003,
	_STREAM_FILE_CHANGE_EOF					= 0x00000004,
	_STREAM_FILE_CHANGE_PAD_WITH_SILENCE	= 0x00000005 //Use for padding the end of streams with silence so garbage doesn't get played
}; // _StreamStateChange_e

enum _ListenerStateChange_e {
	_LISTENER_STATE_CHANGE_NONE				= 0x00000000,
	_LISTENER_STATE_CHANGE_ORIENTATION		= 0x00000001
}; // _ListenerStateChange_e

struct _VirtualEmitter_t;
struct _RealEmitter_t;

struct _VirtualListener_t {
	CFXfm *poXfmCurrentOrientation_WS;
	CFVec3A *poVecPreviousPosition_WS;	// Used for velocity / Doppler.
	CFVec3A *poVecVelocity_WS;			// Used for velocity / Doppler.

	u32 uStateChange; // See _ListenerStateChange_e.
}; // _VirtualListener_t

struct _RealEmitter_t {
	SND_VOICEID oMusyxVoice;
	SND_EMITTER oMusyxEmitterAttrib;

	//// 3D only.
	//
	BOOL b3DStartedAtPreviousFrame;
	f32 f3DStartDelay;
	_VirtualEmitter_t *poVirtualEmitter;
	//
	////

	FLink_t oLink;
}; // _RealEmitter_t

struct _RealEmitterLimit_t {
	u32 uPlaying;
	u32 uPlayable;
}; // _RealEmitterLimit_t

struct _VirtualEmitter_t {
	CFAudioEmitter *poAudioEmitter; // Derives from CFWorldUser.

	FAudio_WaveHandle_t oWaveHandle;

	_RealEmitter_t *poRealEmitter;
	CFAudioEmitter **ppUserAudioEmitter;	// NULL=none

	//// 3D only.
	//
	_VirtualListener_t	*poVirtualListener;
	FLinkRoot_t *paoVirtualEmittersListActive; // Optimization.
	//
	////

	f32 fRadiusOuter;
	
	// These volumes are the volumes that were passed to the sound system
	// The mapping using the _GetVolume() method is done when the sound is played
	f32 fVolume;
	f32 fVolumeDucked;
	f32 fPanLeftRight;
	f32 fFrequencyFactor;
	f32 fDopplerFactor;
	f32 fReverb;
	f32 fSecondsPlayed;
	f32 fSecondsToPlay;


	u32 uStateChanges;	// See _EmitterStateChange_e.

	u8 uPriority;		// culling.
	u8 oeState;
	u8 uProperties;		// See _EmitterProperties_e.
	u8 uPauseLevel;		//represents current pause mode level;
	
	CFVec3A *poVecCurrentPosition_WS;
	CFVec3A *poVecPreviousPosition_WS;		// Used for velocity / Doppler.
	const CFVec3A *poVecAutoPosition_WS;	// Stores the pointer for an AutoUpdate3D sound  
	
	CFAudioEmitter::FAudio_EmitterEndOfPlayCallback_t *pEndOfPlayCallback;

	FLink_t oLink;
}; // _VirtualEmitter_t

struct _Stream_t
{
	//// DO NOT CHANGE ORDER (Must preserve 32 byte alignment).
	//
	u8 aaauFileBuff[ 2 /* Double buffered */ ][ 2 * FGCDATA_WVS_MAX_CHUNK_BYTES /* Left, right */ ];
	//
	////

	//// DO NOT CHANGE SIZE (Must preserve 32 byte alignment).
	//
	FFileHandle ohFile;

	SND_STREAMID oMusyxStreamLeft;
	SND_STREAMID oMusyxStreamRight;

	CFAudioStream *poAudioStream;

	f32	fVolume;
	f32 fPanLeftRight;
	f32 fFrequencyInHz;
	f32 fFrequencyFactor;
	f32 fLengthInSeconds;
	f32 fSecondsPlayed;
	f32 fSecondsToPlay;

	BOOL bIsActive;
	BOOL bIsLooping;
	BOOL bIgnoreInPauseMode;
	BOOL bTreatAsSfx;	// use the sfx volume multiplier instead of the music

	u32	uChannels;
	u32 uCurrentReadBuff;
	u32 uFileSize;
	u32 uFilePos;
	u32 aauBuffReadyToConsume[ 2 /* Double buffered */ ][ 2 /* Left, right */ ];
	u32 aauBuffBytesRead[ 2 /* Double buffered */ ][ 2 /* Left, right */ ];
	u32 aauBuffBaseOffset[ 2 /* Double buffered */ ][ 2 /* Left, right */ ];
	u32 uStateChanges;		// See _StreamStateChange_e.
	u32 uFileChanges;		// See _StreamFileChange_e.

	char szName[ FDATA_AUDIO_FILENAME_LEN ];

	u16 oeState;// FAudio_StreamState_e
	u16 ePauseLevel;// FAudio_PauseLevel_e
	
	CFAudioStream::FAudio_StreamEndOfPlayCallback_t *pEndOfPlayCallback;

	volatile BOOL bIsInUse;		// Musyx.

	u32 	uBufferBytesFilled;
	//
	////

	//// THE SIZE OF THIS STRUCT MUST BE A MULTIPLE OF 32 BYTES, ADD PADDING TO GET THERE
	//
//	u8 auPadding[ 4 ];
	//
	////
}; // _Stream_t

#if FHEAP_TRACK_MEM_ALLOCATIONS
	struct _WaveBankMramUsage_t
	{
		u32 uBytes;
		FLink_t oLink;

	};
#endif

// global vars
u8 FAudio_EmitterDefaultPriorityLevel = 0;
BOOL8 FAudio_bModuleStarted = FALSE;
BOOL8 FAudio_bModuleInstalled = FALSE;
BOOL8 FAudio_bMasterSfxVolChanged = FALSE;
BOOL8 FAudio_bMasterMusicVolChanged = FALSE;
f32 FAudio_fMasterSfxUnitVol = 1.0f;
f32 FAudio_fMasterMusicUnitVol = 1.0f;

// System variables
static BOOL _bSurroundEnabled;

// Wave banks.
static FLinkRoot_t _oWaveBanksList;
#if FHEAP_TRACK_MEM_ALLOCATIONS
	static FLinkRoot_t _oWaveBanksMramUsageList;
	static _WaveBankMramUsage_t *_poTempWaveBankMramUsage;
#endif
static FResLoadReg_t _oLoadReg;
static void *_pMusyxPool, *_pMusyxProject, *_pMusyxSdir, *_pMusyxSamplesBuff;
static BOOL _bMusyxSamplesLoadCallbackFailed;
static u32 _uMusyxSamplesBuff, _uMaxSoundBytes;
static FFileHandle _ohMusyxSamplesFile;

// Real Listeners.
static SND_LISTENER _oMusyxRealListener;

// Virtual Listeners.
static u32 _uMaxVirtualListeners, _uActiveVirtualListeners;
static _VirtualListener_t _aoVirtualListeners[ FAUDIO_MAX_LISTENERS ];

// Real Emitters.
static u32 _uMaxRealEmitters2D;
static _RealEmitter_t *_paoRealEmitters2D;
static FLinkRoot_t _oRealEmittersListFree2D;
static FLinkRoot_t _oRealEmittersListActive2D;

static _RealEmitter_t _aoRealEmitters3D[ _MUSYX_REAL_EMITTERS_MAX_3D ];
static FLinkRoot_t _oRealEmittersListFree3D;
static FLinkRoot_t _oRealEmittersListActive3D;

static _RealEmitterLimit_t *_paoRealEmittersLimits; // Pointer to an array of _uMaxPriorityLevels elements.

static SND_AUX_REVERBSTD _oMusyxReverbParam;

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

static f32 _fStreamsWorkDelay; // (use fangdef instead?) Should be replaced by a scheduler eventually to avoid running in synch with other low-fps work()!!!!!!!!!!
static BOOL _bSkipStreamsWorkDelay;

// Streams.
static u32 _uMaxStreams;
static _Stream_t *_paoStreams;
static u8 *_pauStreamBuff;
static u32 _uStreamBuffSize;

static FAudio_PauseLevel_e _ePauseLevelStreams;

// Temp.
static CFVec3A _oTempVec3A, _oTempVec3A_Velocity;
static CFSphere _oTempSphere;
static _VirtualListener_t *_poTempVirtualListener;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void *_MusyxMalloc( u32 uSize );
static void _MusyxFree( void *pData );
static void *_MusyxSamplesLoadCallback( u32 uOffset, u32 uBytes );
static u32 _MusyxStreamCallback( void *pData1, u32 uSamples1, void *pData2, u32 uSamples2, u32 uContext );
static void _FFileAsyncReadHeaderCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser );
static void _FFileAsyncReadBodyCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser );
static BOOL _BankLoadCallback( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _BankUnloadCallback( void *pResMem );
static BOOL _WorldLoadAndUnloadCallback( FWorldEvent_e oeEvent );
static void _AutoDestroyEmitterEndOfPlayCallback( CFAudioEmitter *poAudioEmitter );
static void _AutoDestroyStreamEndOfPlayCallback( CFAudioStream *poAudioStream );
static void _TrackEmittersProgress( FLinkRoot_t *poVirtualEmittersListActive );
static void _TrackStreamsProgress( void );
static void _ApplyRealEmittersChanges( FLinkRoot_t *poVirtualEmittersListActive );
static void _InvokeEmittersEndofplayCallbacks( FLinkRoot_t *poVirtualEmittersListActive );
static void _AllocateRealEmittersToSingleVirtualListener( void );
static void _AllocateRealEmittersToMultipleVirtualListener( void );
static void _DestroyAllEmittersFromABank( FAudio_BankHandle_t hBank );

// - -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// !!Nate
// Only use this volume mapping function when you are passing volumes to MusyX
// Volumes stored in the data structures should NOT use this mapping function
static FINLINE f32 _GetVolume( f32 fVol )
{
	fVol = fmath_Sqrt( fVol );
	return ( 0.5f * 0.76f * ( fmath_Sqrt( fVol ) + fVol ) );
}

BOOL faudio_ModuleStartup( void )
{
	FASSERT_MSG( ( ! FAudio_bModuleStarted ), "[ FAUDIO ] Error: System already started !!!" );

	AIInit( NULL );

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
	FASSERT_MSG( Fang_ConfigDefs.nAudio_MaxSoundBytes,                                                             "[ FAUDIO ] Error: Invalid uMaxSoundBytes !!!" );
	FASSERT_MSG( poInit->uMaxPriorityLevels,                                                                       "[ FAUDIO ] Error: Invalid uMaxPriorityLevels !!!" );
	FASSERT_MSG( poInit->uMaxBanks,                                                                                "[ FAUDIO ] Error: Invalid uMaxBanks !!!" );
	FASSERT_MSG( ( _MUSYX_REAL_EMITTERS_MAX_2D >= ( 2 * poInit->uMaxStreams ) + 1 ),                               "[ FAUDIO ] Error: Invalid uMaxStreams !!!" );
	FASSERT_MSG( ( poInit->uPriorityLimits ? ( !! poInit->paoPriorityLimits ) : ( ! poInit->paoPriorityLimits ) ), "[ FAUDIO ] Error: Invalid priority limits !!!" );
	FASSERT_MSG( ( ! ( sizeof( _Stream_t ) & 0x1f ) ),                                                             "[ FAUDIO ] Error: size of _Stream_t must be a multiple of 32 !!!" );

	//// Register .Wvb loader.
	//
	if( ! fresload_IsRegistered( FAUDIOBANK_RESTYPE ) )
	{
		fang_MemZero( &_oLoadReg, sizeof( _oLoadReg ) );
		fres_CopyType( _oLoadReg.sResType, FAUDIOBANK_RESTYPE );
		_oLoadReg.pszFileExtension = "rdg";
		_oLoadReg.nMemType         = FRESLOAD_MEMTYPE_PERM;
		_oLoadReg.nAlignment       = 32;
		_oLoadReg.pFcnCreate       = _BankLoadCallback;
		_oLoadReg.pFcnDestroy      = _BankUnloadCallback;

		if( ! fresload_RegisterHandler( &_oLoadReg ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: fresload_RegisterHandler() failed !!!\n", __LINE__ );
			return FAUDIO_ERROR;
		}
	}
	//
	////

	//// Init static values.
	//
	_ePauseLevelEmitters              = FAUDIO_PAUSE_LEVEL_NONE;
	_ePauseLevelStreams               = FAUDIO_PAUSE_LEVEL_NONE;
	_fDuckingFactor                  = 1.0f;

	_fEmittersListenersWorkDelay     = 0.0f;
	_fOOEmittersListenersWorkDelay   = 0.0f;
	_bSkipEmittersListenersWorkDelay = TRUE;

	_fStreamsWorkDelay               = 0.0f;
	_bSkipStreamsWorkDelay           = TRUE;

	_uEmittersCreateErrors2D         = _EMITTERS_CREATE_MAX_ERRORS;
	_uEmittersCreateErrors3D         = _EMITTERS_CREATE_MAX_ERRORS;
	_uMaxVirtualListeners            = poInit->uMaxListeners;
	_uMaxVirtualEmitters             = poInit->uMaxEmitters;
	_uMaxPriorityLevels              = poInit->uMaxPriorityLevels;
	_uMaxSoundBytes                  = Fang_ConfigDefs.nAudio_MaxSoundBytes;
	_uMaxRealEmitters2D              = _MUSYX_REAL_EMITTERS_MAX_2D - ( 2 * poInit->uMaxStreams );
	_uMaxStreams                     = poInit->uMaxStreams;

	if( _MAX_TOTAL_STREAMS < _uMaxStreams )
	{
		DEVPRINTF( "[ FAUDIO ] Warning %u: Clamping max streams to %u.\n", __LINE__, _MAX_TOTAL_STREAMS );
		_uMaxStreams = _MAX_TOTAL_STREAMS;
	}

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

	FFileHandle ohFile = FFILE_INVALID_HANDLE;
	u32 uSize;

	FResFrame_t oResFrame = fres_GetFrame();

	//// Musyx.
	//

	char cLanguageChar, cAudioLanguageChar;
	ffile_GetLanguageChars( &cLanguageChar, &cAudioLanguageChar );
	
	// First, see if we can load the localized version of the snd_init file...
	if( cAudioLanguageChar )
	{
		char szLocalizedFilename[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
		sprintf( szLocalizedFilename, "snd_init%c.rdg", cAudioLanguageChar );
		ohFile = ffile_Open( szLocalizedFilename, FFILE_OPEN_RONLY_SEQUENTIAL, FALSE );
	}

	if( FFILE_INVALID_HANDLE == ohFile )
	{
		// Try and load the non-localized filename version
		ohFile = ffile_Open( "snd_init.rdg", FFILE_OPEN_RONLY_SEQUENTIAL, FALSE );
	}

	if( FFILE_INVALID_HANDLE == ohFile )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not open \"snd_init.rdg\" file !!!\n", __LINE__ );
		return FAUDIO_ERROR;
	}

	uSize = (u32)ffile_GetFileSize( ohFile );
	if( ! uSize )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: \"snd_init.rdg\" file invalid !!!\n", __LINE__ );
		ffile_Close( ohFile );
		return FAUDIO_ERROR;
	}

	FGCData_MusyXDataHeader_t *poHeader = (FGCData_MusyXDataHeader_t *)fres_Alloc( uSize );
	if( ! poHeader )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fres_AlignedAlloc() failed !!!\n", __LINE__ );
		ffile_Close( ohFile );
		return FAUDIO_ERROR;
	}

	if( uSize != ffile_Read( ohFile, uSize, poHeader ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: ffile_Read() failed !!!\n", __LINE__ );
		ffile_Close( ohFile );
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}

	ffile_Close( ohFile );
	//
	////

	////
	//
	_pMusyxProject = (void *)( (u32)poHeader + poHeader->nProjFileOffset );
	_pMusyxPool    = (void *)( (u32)poHeader + poHeader->nPoolFileOffset );
	_pMusyxSdir    = (void *)( (u32)poHeader + poHeader->nSDirFileOffset );
	//
	////

	//// Musyx samples.
	//
	_uMusyxSamplesBuff  = ARQGetChunkSize();
	_pMusyxSamplesBuff  = NULL;
	_ohMusyxSamplesFile = FFILE_INVALID_HANDLE;
	//
	////

	//// Initialize Musyx.
	//
	SND_HOOKS aoSndHooks[ 2 ] = { _MusyxMalloc, _MusyxFree };
	sndSetHooks( aoSndHooks );

	u32 uSysInfo = fsysinfo_GetCapabilities();

	if( sndInit( _MUSYX_REAL_EMITTERS, 0, _MUSYX_REAL_EMITTERS, 1, ( SND_FLAGS_EMITTERGROUPSUSEMAXVOICES | ( ( FSYSINFO_AUDIO_MONO & uSysInfo ) ? SND_FLAGS_DEFAULT : SND_FLAGS_DEFAULT_STUDIO_DPL2 ) ), _uMaxSoundBytes ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: sndInit() failed !!!\n", __LINE__ );
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}

	// Start in mono/stereo mode, and if the game later commands us to go into surround, we will
	sndOutputMode( ( ( FSYSINFO_AUDIO_MONO & uSysInfo ) ? SND_OUTPUTMODE_MONO : SND_OUTPUTMODE_STEREO ) );
	_bSurroundEnabled = FALSE;
	//
	////

	//// Initialize volumes.
	//
	sndMasterVolume( _SND_MASTER_VOLUME, 0, FALSE, TRUE );
	sndVolume( _SND_MASTER_VOLUME, 0, SND_FX_VOLGROUPS );
	//
	////

	//// Set default Musyx listener attributes.
	//
	SND_FVECTOR oMusyxVecPosition, oMusyxVecVelocity, oMusyxVecFront, oMusyxVecTop;

	fang_MemZero( &oMusyxVecPosition, sizeof( oMusyxVecPosition ) );
	fang_MemZero( &oMusyxVecVelocity, sizeof( oMusyxVecVelocity ) );
	fang_MemZero( &oMusyxVecFront,    sizeof( oMusyxVecFront ) );
	fang_MemZero( &oMusyxVecTop,      sizeof( oMusyxVecTop ) );

	oMusyxVecFront.z = -1.0f;
	oMusyxVecTop.y   = 1.0f;

	if( ! sndAddListener( &_oMusyxRealListener, &oMusyxVecPosition, &oMusyxVecVelocity, &oMusyxVecFront, &oMusyxVecTop, 250.0f, 250.0f, 1110.0f, SND_LISTENER_DOPPLERFX, 127, NULL ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: sndAddListener() failed !!!\n", __LINE__ );
		sndSilence();
		while( ! sndIsIdle() ){};
		sndQuit();
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}
	//
	////

	//// Set default Musyx emitter reverb parameters.
	//
/*
// Re-enable later. Keep a counter of number of emitters / streams requiring reverb. If none, disable reverb.
	fang_MemZero( &_oMusyxReverbParam, sizeof( _oMusyxReverbParam ) );
	_oMusyxReverbParam.tempDisableFX = FALSE;
	_oMusyxReverbParam.coloration    = 0.9f;
	_oMusyxReverbParam.mix           = 0.5f;
	_oMusyxReverbParam.time          = 5.0f;
	_oMusyxReverbParam.damping       = 0.6f;
	_oMusyxReverbParam.preDelay      = 0.1f;

	sndAuxCallbackPrepareReverbSTD( &_oMusyxReverbParam );
	sndSetAuxProcessingCallbacks(	SND_STUDIO_DEFAULT,
									sndAuxCallbackReverbSTD, &_oMusyxReverbParam, SND_MIDI_NONE, 0,	// Aux A.
									NULL, NULL, SND_MIDI_NONE, 0 );									// Aux B.
*/
	//
	////

	//// Init arrays and lists.
	//
	// Virtual listeners.
	fang_MemZero( _aoVirtualListeners, sizeof( _aoVirtualListeners ) );

	// Virtual emitters.
	flinklist_InitRoot( &_oVirtualEmittersListFree, (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );

	// Real emitters.
	flinklist_InitRoot( &_oRealEmittersListFree2D,   (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );
	flinklist_InitRoot( &_oRealEmittersListActive2D, (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );

	fang_MemZero( _aoRealEmitters3D, sizeof( _aoRealEmitters3D ) );
	flinklist_InitRoot( &_oRealEmittersListFree3D,   (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );
	flinklist_InitRoot( &_oRealEmittersListActive3D, (s32)FANG_OFFSETOF( _RealEmitter_t, oLink ) );

	// Banks.
	flinklist_InitRoot( &_oWaveBanksList, (s32)FANG_OFFSETOF( FDataWvbFile_Bank_t, oLink ) );
#if FHEAP_TRACK_MEM_ALLOCATIONS
	flinklist_InitRoot( &_oWaveBanksMramUsageList, (s32)FANG_OFFSETOF( _WaveBankMramUsage_t, oLink ) );
#endif
	//
	////

	//// Allocate memory.
	//
	_uStreamBuffSize = sndStreamAllocLength( _MUSYX_SAMPLES_BUFFERED, SND_STREAM_ADPCM );

	_paoVirtualEmitters             = (_VirtualEmitter_t *)fres_AllocAndZero( sizeof( _VirtualEmitter_t ) * _uMaxVirtualEmitters );
	_paoVirtualEmittersListActive2D = (FLinkRoot_t *)fres_AllocAndZero( sizeof( FLinkRoot_t ) * _uMaxPriorityLevels );
	_paoVirtualEmittersListActive3D = (FLinkRoot_t *)fres_AllocAndZero( sizeof( FLinkRoot_t ) * _uMaxPriorityLevels );
	_paoRealEmittersLimits          = (_RealEmitterLimit_t *)fres_AllocAndZero( sizeof( _RealEmitterLimit_t ) * _uMaxPriorityLevels );
	_paoRealEmitters2D              = (_RealEmitter_t *)fres_AllocAndZero( sizeof( _RealEmitter_t ) * _uMaxRealEmitters2D );
	_paoStreams                     = (_Stream_t *)fres_AlignedAllocAndZero( sizeof( _Stream_t ) * _uMaxStreams, 32 );
	_pauStreamBuff                  = (u8 *)fres_Alloc( ( sizeof( u8 ) * _uStreamBuffSize * 2 * _uMaxStreams ) );
	if( ! ( _paoVirtualEmitters &&
			_paoVirtualEmittersListActive2D &&
			_paoVirtualEmittersListActive3D &&
			_paoRealEmittersLimits &&
			_paoRealEmitters2D &&
			_paoStreams &&
			_pauStreamBuff ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fres_AllocAndZero() failed !!!\n", __LINE__ );
		sndSilence();
		while( ! sndIsIdle() ){};
		sndQuit();
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}

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
	_Stream_t *poStream                   = &( _paoStreams[ 0 ] );

	//// Allocate memory.
	//
	poVirtualListener->poXfmCurrentOrientation_WS = (CFXfm *)fnew CFXfm[ _uMaxVirtualListeners ];
	poVirtualListener->poVecPreviousPosition_WS   = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualListeners ];
	poVirtualListener->poVecVelocity_WS           = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualListeners ];

	poVirtualEmitter->poAudioEmitter              = (CFAudioEmitter *)fnew CFAudioEmitter[ _uMaxVirtualEmitters ];
	poVirtualEmitter->poVecCurrentPosition_WS     = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualEmitters ];
	poVirtualEmitter->poVecPreviousPosition_WS    = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualEmitters ];

	poStream->poAudioStream                       = (CFAudioStream *)fnew CFAudioStream[ _uMaxStreams ];

	if( ! (	poVirtualListener->poXfmCurrentOrientation_WS &&
			poVirtualListener->poVecPreviousPosition_WS &&
			poVirtualListener->poVecVelocity_WS &&

			poVirtualEmitter->poAudioEmitter &&
			poVirtualEmitter->poVecCurrentPosition_WS &&
			poVirtualEmitter->poVecPreviousPosition_WS &&

			poStream->poAudioStream ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fnew() failed !!!\n", __LINE__ );

		fdelete_array( poStream->poAudioStream );

		fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
		fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
		fdelete_array( poVirtualEmitter->poAudioEmitter );

		fdelete_array( poVirtualListener->poVecVelocity_WS );
		fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
		fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );

		sndSilence();
		while( ! sndIsIdle() ){};
		sndQuit();
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
	//
	////

	//// Fix-up.
	//
	for( uIndex = 1; uIndex < _uMaxVirtualListeners; ++uIndex )
	{
		poVirtualListener                             = &( _aoVirtualListeners[ uIndex ] );
		poVirtualListener->poXfmCurrentOrientation_WS = ( _aoVirtualListeners[ 0 ].poXfmCurrentOrientation_WS + uIndex );
		poVirtualListener->poVecPreviousPosition_WS   = ( _aoVirtualListeners[ 0 ].poVecPreviousPosition_WS + uIndex );
		poVirtualListener->poVecVelocity_WS           = ( _aoVirtualListeners[ 0 ].poVecVelocity_WS + uIndex );

		poVirtualListener->poXfmCurrentOrientation_WS->Identity();
		poVirtualListener->poVecPreviousPosition_WS->Zero();
		poVirtualListener->poVecVelocity_WS->Zero();
	}

	//// Init virtual emitter.
	//
	poVirtualEmitter->poVecCurrentPosition_WS->Zero();
	poVirtualEmitter->poVecPreviousPosition_WS->Zero();
	poVirtualEmitter->poVecAutoPosition_WS = NULL;
	poVirtualEmitter->poAudioEmitter->m_nUser	= FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE;
	poVirtualEmitter->poAudioEmitter->m_pUser	= poVirtualEmitter;
	poVirtualEmitter->poRealEmitter				= NULL;
	poVirtualEmitter->poVirtualListener			= NULL;
	poVirtualEmitter->fRadiusOuter				= 1.0f;
//	poVirtualEmitter->fRadiusInner				= FAUDIO_MIN_RADIUS;
	poVirtualEmitter->fVolume					= 1.0f;
	poVirtualEmitter->fVolumeDucked				= 1.0f;
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
		poVirtualEmitter							= &( _paoVirtualEmitters[ uIndex ] );
		poVirtualEmitter->poAudioEmitter			= ( _paoVirtualEmitters[ 0 ].poAudioEmitter + uIndex );
		poVirtualEmitter->poVecCurrentPosition_WS	= ( _paoVirtualEmitters[ 0 ].poVecCurrentPosition_WS + uIndex );
		poVirtualEmitter->poVecPreviousPosition_WS	= ( _paoVirtualEmitters[ 0 ].poVecPreviousPosition_WS + uIndex );

		//// Init virtual emitter.
		//
		poVirtualEmitter->poVecCurrentPosition_WS->Zero();
		poVirtualEmitter->poVecPreviousPosition_WS->Zero();
		poVirtualEmitter->poVecAutoPosition_WS = NULL;
		poVirtualEmitter->poAudioEmitter->m_nUser	= FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE;
		poVirtualEmitter->poAudioEmitter->m_pUser	= poVirtualEmitter;
		poVirtualEmitter->poRealEmitter				= NULL;
		poVirtualEmitter->poVirtualListener			= NULL;
		poVirtualEmitter->fRadiusOuter				= 1.0f;
//		poVirtualEmitter->fRadiusInner				= FAUDIO_MIN_RADIUS;
		poVirtualEmitter->fVolume					= 1.0f;
		poVirtualEmitter->fVolumeDucked				= 1.0f;
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

	//// Init streams.
	//
	poStream->poAudioStream->m_uData = (u32)poStream;
	poStream->bIsActive              = FALSE;
	poStream->bIsInUse               = FALSE;
	poStream->ohFile                 = FFILE_INVALID_HANDLE;

	poStream->oMusyxStreamLeft       = sndStreamAllocEx( 255, ( _pauStreamBuff + ( ( ( 0 * 2 ) + 0 ) * _uStreamBuffSize ) ), _MUSYX_SAMPLES_BUFFERED, 8000, 127,   0, 0, 0 /*a*/, 0 /*b*/, SND_STUDIO_DEFAULT, ( SND_STREAM_ADPCM | SND_STREAM_INACTIVE ), _MusyxStreamCallback, ( ( 0 << 16 ) | ( 0 & 0xffff ) ), NULL );
	poStream->oMusyxStreamRight      = sndStreamAllocEx( 255, ( _pauStreamBuff + ( ( ( 0 * 2 ) + 1 ) * _uStreamBuffSize ) ), _MUSYX_SAMPLES_BUFFERED, 8000, 127, 127, 0, 0 /*a*/, 0 /*b*/, SND_STUDIO_DEFAULT, ( SND_STREAM_ADPCM | SND_STREAM_INACTIVE ), _MusyxStreamCallback, ( ( 1 << 16 ) | ( 0 & 0xffff ) ), NULL );

	if( ( SND_ID_ERROR == poStream->oMusyxStreamLeft ) || ( SND_ID_ERROR == poStream->oMusyxStreamRight ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: sndStreamAllocEx() failed !!!\n", __LINE__ );

		fdelete_array( poStream->poAudioStream );

		poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
		fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
		fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
		fdelete_array( poVirtualEmitter->poAudioEmitter );

		poVirtualListener = &( _aoVirtualListeners[ 0 ] );
		fdelete_array( poVirtualListener->poVecVelocity_WS );
		fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
		fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );

		sndSilence();
		while( ! sndIsIdle() ){};
		sndQuit();
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}

	for( uIndex = 1; uIndex < _uMaxStreams; ++uIndex )
	{
		poStream                         = &( _paoStreams[ uIndex ] );
		poStream->poAudioStream          = ( _paoStreams[ 0 ].poAudioStream + uIndex );
		poStream->poAudioStream->m_uData = (u32)poStream;
		poStream->bIsActive              = FALSE;
		poStream->bIsInUse               = FALSE;
		poStream->ohFile                 = FFILE_INVALID_HANDLE;

		poStream->oMusyxStreamLeft       = sndStreamAllocEx( 255, ( _pauStreamBuff + ( ( ( uIndex * 2 ) + 0 ) * _uStreamBuffSize ) ), _MUSYX_SAMPLES_BUFFERED, 8000, 127,   0, 0, 0 /*a*/, 0 /*b*/, SND_STUDIO_DEFAULT, ( SND_STREAM_ADPCM | SND_STREAM_INACTIVE ), _MusyxStreamCallback, ( ( 0 << 16 ) | ( uIndex & 0xffff ) ), NULL );
		poStream->oMusyxStreamRight      = sndStreamAllocEx( 255, ( _pauStreamBuff + ( ( ( uIndex * 2 ) + 1 ) * _uStreamBuffSize ) ), _MUSYX_SAMPLES_BUFFERED, 8000, 127, 127, 0, 0 /*a*/, 0 /*b*/, SND_STUDIO_DEFAULT, ( SND_STREAM_ADPCM | SND_STREAM_INACTIVE ), _MusyxStreamCallback, ( ( 1 << 16 ) | ( uIndex & 0xffff ) ), NULL );

		if( ( SND_ID_ERROR == poStream->oMusyxStreamLeft ) || ( SND_ID_ERROR == poStream->oMusyxStreamRight ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: sndStreamAllocEx() failed !!!\n", __LINE__ );

			fdelete_array( _paoStreams[ 0 ].poAudioStream );

			poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
			fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
			fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
			fdelete_array( poVirtualEmitter->poAudioEmitter );

			poVirtualListener = &( _aoVirtualListeners[ 0 ] );
			fdelete_array( poVirtualListener->poVecVelocity_WS );
			fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
			fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );

			sndSilence();
			while( ! sndIsIdle() ){};
			sndQuit();
			fres_ReleaseFrame( oResFrame );
			return FAUDIO_ERROR;
		}
	}
	//
	////

	//// Add to free lists.
	//
	_RealEmitter_t *poRealEmitter;

	//// 2D.
	//
	for( uIndex = 0; uIndex < _uMaxRealEmitters2D; ++uIndex )
	{
		poRealEmitter = &( _paoRealEmitters2D[ uIndex ] );

		flinklist_AddTail( &_oRealEmittersListFree2D, poRealEmitter );
	}
	//
	////

	//// 3D.
	//
	for( uIndex = 0; uIndex < _MUSYX_REAL_EMITTERS_MAX_3D; ++uIndex )
	{
		poRealEmitter = &( _aoRealEmitters3D[ uIndex ] );

		flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitter );
	}
	//
	////
	//
	//// Add to free lists.

	FAudio_bModuleInstalled = TRUE;

	return FAUDIO_NO_ERROR;

} // faudio_Install

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void faudio_Uninstall( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	if( fworld_IsWorldCallbackFunctionRegistered( _WorldLoadAndUnloadCallback ) )
	{
		fworld_UnregisterWorldCallbackFunction( _WorldLoadAndUnloadCallback );
	}
	
	//// Uninitialize Musyx.
	//
	sndSilence();
	while( ! sndIsIdle() ){};
	sndQuit();
	//
	////

	//// Free memory.
	//
	fdelete_array( _paoStreams[ 0 ].poAudioStream );

	_VirtualEmitter_t *poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
	fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
	fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
	fdelete_array( poVirtualEmitter->poAudioEmitter );

	_VirtualListener_t *poVirtualListener = &( _aoVirtualListeners[ 0 ] );
	fdelete_array( poVirtualListener->poVecVelocity_WS );
	fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
	fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );
	//
	////

	FAudio_bModuleInstalled = FALSE;

} // faudio_Uninstall

static BOOL _BankLoadCallback( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName )
{
//	DEVPRINTF( "Load %s\n", pszResName );
	
	if( ! FAudio_bModuleInstalled )
	{
		return FALSE;
	}

	FASSERT_MSG( hRes,                                            "[ FAUDIO ] Error: Invalid handle !!!" );
	FASSERT_MSG( pLoadedBase,                                     "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( pszResName,                                      "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pszResName,                                     "[ FAUDIO ] Error: Zero length string !!!" );
	FASSERT_MSG( _pMusyxSamplesBuff,                              "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( _uMusyxSamplesBuff,                              "[ FAUDIO ] Error: Zero length buffer !!!" );
	FASSERT_MSG( ( _uMusyxSamplesBuff == ARQGetChunkSize() ),     "[ FAUDIO ] Error: Invalid buffer length !!!" );
	FASSERT_MSG( ( FFILE_INVALID_HANDLE != _ohMusyxSamplesFile ), "[ FAUDIO ] Error: Invalid file handle !!!" );

	////
	//
	if( ( sizeof( FDataWvbFile_Bank_t ) + sizeof( FDataWvbFile_Wave_t ) + sizeof( u32 ) ) > nLoadedBytes )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Invalid bank !!!\n", __LINE__ );
		return FALSE;
	}
	//
	////

	////
	//
	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)pLoadedBase;
	FDataWvbFile_Wave_t *poWave = (FDataWvbFile_Wave_t *)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) );
	//
	////

	////
	//
	poBank->pData = (void *)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) + ( poBank->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );

	for( u32 uIndex = 0; uIndex < poBank->uWaves; ++uIndex, ++poWave )
	{
		poWave->oBankHandle = (FAudio_BankHandle_t)poBank;
	}
	//
	////

	////
	//
#if FHEAP_TRACK_MEM_ALLOCATIONS
	_poTempWaveBankMramUsage = (_WaveBankMramUsage_t *)fres_AllocAndZero( sizeof( _WaveBankMramUsage_t ) );
	if( ! _poTempWaveBankMramUsage )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fres_AllocAndZero() failed !!!\n", __LINE__ );
		return FALSE;
	}
#endif

	if( ( ! sndPushGroup( _pMusyxProject, *((u32 *)poBank->pData), NULL, _pMusyxSdir, _pMusyxPool ) ) || _bMusyxSamplesLoadCallbackFailed )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: sndPushGroup() failed !!!\n", __LINE__ );
		return FALSE;
	}

#if FHEAP_TRACK_MEM_ALLOCATIONS
	flinklist_AddTail( &_oWaveBanksMramUsageList, _poTempWaveBankMramUsage );
	Fheap_nTotalSoundMemTracked += _poTempWaveBankMramUsage->uBytes;
	f32 fRatio = (f32)Fheap_nTotalSoundMemTracked * (1.f / Fang_ConfigDefs.nAudio_MaxSoundBytes);
	fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "SND", fRatio, 7.f );
#endif
	//
	////

	////
	//
	flinklist_AddTail( &_oWaveBanksList, poBank );
	fres_SetBase( hRes, poBank );
	//
	////

	return TRUE;

} // _BankLoadCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _BankUnloadCallback( void *pResMem )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	FASSERT_MSG( pResMem, "[ FAUDIO ] Error: NULL pointer !!!" );

	// !!Nate
	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)pResMem;

//	DEVPRINTF( "%s unload\n", poBank->szName );
	
	// kill all sounds that are currently using this bank
	_DestroyAllEmittersFromABank( (FAudio_BankHandle_t)poBank );
	
	////
	//
	sndPopGroup();

	flinklist_RemoveTail( &_oWaveBanksList );

#if FHEAP_TRACK_MEM_ALLOCATIONS
	_poTempWaveBankMramUsage = (_WaveBankMramUsage_t *)flinklist_RemoveTail( &_oWaveBanksMramUsageList );
	Fheap_nTotalSoundMemTracked -= _poTempWaveBankMramUsage->uBytes;
	f32 fRatio = (f32)Fheap_nTotalSoundMemTracked * (1.f / Fang_ConfigDefs.nAudio_MaxSoundBytes);
	fperf_Render_AddPerfBar( FPERF_TYPE_MEMORY, "SND", fRatio, 7.f );
#endif
	//
	////

} // _BankUnloadCallback


static BOOL _WorldLoadAndUnloadCallback( FWorldEvent_e oeEvent )
{
	if( ! FAudio_bModuleInstalled )
	{
		return TRUE;
	}

	u32 uIndex;

	if( FWORLD_EVENT_WORLD_POSTLOAD == oeEvent )
	{
		// Nate - No idea why we don't do this on the GC
		/*FLinkRoot_t *poVirtualEmittersList;
		_VirtualListener_t *poVirtualListener;
		_VirtualEmitter_t *poVirtualEmitter;

		//// Insert virtual listeners in the world.
		//
		for( uIndex = 0; uIndex < _uMaxVirtualListeners; ++uIndex )
		{
			poVirtualListener      = &( _aoVirtualListeners[ uIndex ] );
			_oTempSphere.m_Pos     = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos.v3;
			_oTempSphere.m_fRadius = FAUDIO_MIN_RADIUS;

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
		}*/
		//
		////
	}
	else if( FWORLD_EVENT_WORLD_PREDESTROY == oeEvent )
	{
		//// Remove virtual listeners in the world.
		//
		// Nate - No idea why we don't do this on the GC
		/*for( uIndex = 0; uIndex < _uMaxVirtualListeners; ++uIndex )
		{
			_aoVirtualListeners[ uIndex ].poWorldUser->RemoveFromWorld();
		}*/
		//
		////

		FLinkRoot_t *poVirtualEmittersList;
		_VirtualEmitter_t *poVirtualEmitter, *pNextVEmitter;

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
				pNextVEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				poVirtualEmitter->poAudioEmitter->Destroy();

				// move to the next emitter
				poVirtualEmitter = pNextVEmitter;
			}

			// Nate - Stop any 2D sounds that may be playing
			// 2D.
			poVirtualEmittersList = &_paoVirtualEmittersListActive2D[ uIndex ];
			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter ) {
				// grab the next virtual emitter
				pNextVEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );

				if( ( poVirtualEmitter->fSecondsToPlay - poVirtualEmitter->fSecondsPlayed ) > 1.0f ) {
					poVirtualEmitter->poAudioEmitter->Destroy();
				}
			
				// move to the next emitter
				poVirtualEmitter = pNextVEmitter;
			}
		}
	}

	return TRUE;

} // _WorldLoadAndUnloadCallback
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void faudio_Work( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	////
	//
	_TrackEmittersProgress( _paoVirtualEmittersListActive2D );
	_TrackEmittersProgress( _paoVirtualEmittersListActive3D );
	_TrackStreamsProgress();
	//
	////

	FLinkRoot_t *poVirtualEmittersList;
	_VirtualListener_t *poVirtualListener;
	_VirtualEmitter_t *poVirtualEmitter;
	_RealEmitter_t *poRealEmitter;
	u32 uIndex;
	s32 nList;

	////
	//
	poRealEmitter = (_RealEmitter_t *)flinklist_GetHead( &_oRealEmittersListActive3D );

	while( poRealEmitter )
	{
		if( poRealEmitter->b3DStartedAtPreviousFrame )
		{
			poRealEmitter->oMusyxVoice = sndEmitterVoiceID( &( poRealEmitter->oMusyxEmitterAttrib ) );

			if( SND_ID_ERROR != poRealEmitter->oMusyxVoice )
			{
				poRealEmitter->b3DStartedAtPreviousFrame = FALSE;
				poRealEmitter->f3DStartDelay             = 0.0f;
				_bSkipEmittersListenersWorkDelay         = FALSE;
			}
			else
			{
				poRealEmitter->f3DStartDelay += FLoop_fRealPreviousLoopSecs;

				poRealEmitter->poVirtualEmitter->fSecondsPlayed = 0.0f;

				// !!Nate
				if( _3D_EMITTER_MAX_START_DELAY <= poRealEmitter->f3DStartDelay )
				{
					poRealEmitter->f3DStartDelay = 0.0f;
					#if 0 // !FANG_PRODUCTION_BUILD
						if (   poRealEmitter->poVirtualEmitter->poAudioEmitter
							&& poRealEmitter->poVirtualEmitter->poAudioEmitter->m_pszModuleOwner )
						{
							DEVPRINTF( "faudio_Work() - An emitter has been allocated to %s (line %d) but not used in %f seconds and"
								" not released by the caller!\n", 
								poRealEmitter->poVirtualEmitter->poAudioEmitter->m_pszModuleOwner,
								poRealEmitter->poVirtualEmitter->poAudioEmitter->m_nOwnerModuleLine,
								_3D_EMITTER_MAX_START_DELAY );
						}
						else
					#endif // !FANG_PRODUCTION_BUILD
					{
						DEVPRINTF( "faudio_Work() - An emitter has been allocated but not used in %f seconds and"
							" not released by the caller!\n", _3D_EMITTER_MAX_START_DELAY );
					}
						
					// Nate - DO NOT do this, the game may still have the pointer!!!!!
					
					//sndRemoveEmitter( &( poRealEmitter->oMusyxEmitterAttrib ) );
					//flinklist_Remove( &_oRealEmittersListActive3D, poRealEmitter );
					//flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitter );

					//--( _paoRealEmittersLimits[ poRealEmitter->poVirtualEmitter->uPriority ].uPlaying );
					//poRealEmitter->poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;
					//poRealEmitter->poVirtualEmitter->poRealEmitter = NULL;
					
					//flinklist_Remove( poRealEmitter->poVirtualEmitter->paoVirtualEmittersListActive, poRealEmitter->poVirtualEmitter );
					//flinklist_AddTail( &_oVirtualEmittersListFree, poRealEmitter->poVirtualEmitter );
					//poRealEmitter->poVirtualEmitter                = NULL;
					//poRealEmitter->b3DStartedAtPreviousFrame       = FALSE;
				}
			}
		}

		poRealEmitter = (_RealEmitter_t *)flinklist_GetNext( &_oRealEmittersListActive3D, poRealEmitter );
	}
	//
	////

	//// Emitters and listeners work.
	//
	_fEmittersListenersWorkDelay += FLoop_fRealPreviousLoopSecs;

	if( ( ! _bSkipEmittersListenersWorkDelay ) || ( _EMITTERS_LISTENERS_WORK_DELAY < _fEmittersListenersWorkDelay ) )
	{
		////
		//
		if( FWorld_pWorld )
		{
			//// Apply velocity changes in the world for virtual listeners.
			//
			for( uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
			{
				poVirtualListener = &( _aoVirtualListeners[ uIndex ] );

				if( _LISTENER_STATE_CHANGE_ORIENTATION & poVirtualListener->uStateChange )
				{
					// Listener velocity.
					*( poVirtualListener->poVecVelocity_WS ) = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
					poVirtualListener->poVecVelocity_WS->Sub( *( poVirtualListener->poVecPreviousPosition_WS ) );
				}
				else
				{
					poVirtualListener->poVecVelocity_WS->Zero();
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
						sndFXKeyOff( poVirtualEmitter->poRealEmitter->oMusyxVoice );
						flinklist_Remove( &_oRealEmittersListActive2D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree2D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter->poVirtualEmitter = NULL;
						poVirtualEmitter->poRealEmitter                   = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
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
						sndRemoveEmitter( &( poVirtualEmitter->poRealEmitter->oMusyxEmitterAttrib ) );
						flinklist_Remove( &_oRealEmittersListActive3D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree3D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter->poVirtualEmitter = NULL;
						poVirtualEmitter->poRealEmitter                   = NULL;
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
						sndRemoveEmitter( &( poVirtualEmitter->poRealEmitter->oMusyxEmitterAttrib ) );
						//sndFXPitchBend( poVirtualEmitter->poRealEmitter->oMusyxVoice, 0 ); //see if we can stop the sound!

						flinklist_Remove( &_oRealEmittersListActive3D, poVirtualEmitter->poRealEmitter );
						flinklist_AddTail( &_oRealEmittersListFree3D, poVirtualEmitter->poRealEmitter );
						poVirtualEmitter->poRealEmitter->poVirtualEmitter = NULL;
						poVirtualEmitter->poRealEmitter                   = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

						poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;
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
		_RealEmitter_t *poRealEmitterCull;
		_VirtualEmitter_t *poVirtualEmitterCull;
		u32 uPriority;

		//// Allocate real emitters.
		//
		// 2D.
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
							flinklist_AddTail( &_oRealEmittersListActive2D, poRealEmitter );
							poVirtualEmitter->poRealEmitter                   = poRealEmitter;
							poVirtualEmitter->poRealEmitter->poVirtualEmitter = poVirtualEmitter;
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
							sndFXKeyOff( poRealEmitterCull->oMusyxVoice );
							poVirtualEmitterCull->poRealEmitter->poVirtualEmitter = NULL;
							poVirtualEmitterCull->poRealEmitter                   = NULL;
							--( _paoRealEmittersLimits[ poVirtualEmitterCull->uPriority ].uPlaying );

							poVirtualEmitter->poRealEmitter                   = poRealEmitterCull;
							poVirtualEmitter->poRealEmitter->poVirtualEmitter = poVirtualEmitter;
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
			if( 2 > _uActiveVirtualListeners )
			{
				_AllocateRealEmittersToSingleVirtualListener();
			}
			else
			{
				_AllocateRealEmittersToMultipleVirtualListener();
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

		_bSkipEmittersListenersWorkDelay = TRUE;

		////
		//
		for( nList = 0; nList < (s32)_uMaxPriorityLevels; ++nList )
		{
			poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ nList ] );
			poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			while( poVirtualEmitter )
			{
				if( ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_LOOPING ) &&
					( FAUDIO_EMITTER_STATE_STOPPED == poVirtualEmitter->oeState ) )
				{
					//// Restart looping emitters.
					//
					poVirtualEmitter->oeState       = FAUDIO_EMITTER_STATE_PLAYING;
					poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_PLAY;

					_bSkipEmittersListenersWorkDelay = FALSE;
					//
					////
				}
				else
				{
					poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;
				}

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

					if( poVirtualEmitter->poRealEmitter &&
						( poVirtualEmitter->poRealEmitter->b3DStartedAtPreviousFrame ) )
					{
						poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_UNPAUSE | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_POSITION ) );
					}
					else
					{
						if( ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_LOOPING ) &&
							( FAUDIO_EMITTER_STATE_STOPPED == poVirtualEmitter->oeState ) )
						{
							//// Restart looping emitters.
							//
							poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_PLAYING;
							poVirtualEmitter->uStateChanges  = _EMITTER_STATE_CHANGE_PLAY;
							_bSkipEmittersListenersWorkDelay = FALSE;
							//
							////
						}
						else
						{
							poVirtualEmitter->uStateChanges = _EMITTER_STATE_CHANGE_NONE;
						}
					}

					poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}
			}
			//
			////
		}
		//
		////

		_fEmittersListenersWorkDelay = 0.0f;
	}
	//
	//// Emitters and listeners work.

	// Streams work.
	_fStreamsWorkDelay += FLoop_fRealPreviousLoopSecs;

	if( ( ! _bSkipStreamsWorkDelay ) || ( _STREAMS_WORK_DELAY < _fStreamsWorkDelay ) )
	{
		_Stream_t *poStream;
		f32 fVolume;

		for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
		{
			poStream = &_paoStreams[ uIndex ];

			if( ( ! poStream->bIsActive ) || ( FAUDIO_STREAM_STATE_ERROR == poStream->oeState ) ) {
				continue;
			}
			
            poStream->bIsInUse = TRUE;
            
            u32 uStateChanges = poStream->uStateChanges;
            poStream->uStateChanges = 0; // Zero out the state changes, and only apply THE above state changes this frame...

			// see if the master volume has changed and the volume needs to be adjusted
			if( poStream->bTreatAsSfx ) {
				if( FAudio_bMasterSfxVolChanged ) {
					// flag the stream to have the volume changed
					uStateChanges |= _STREAM_STATE_CHANGE_VOLUME;
				}
			} else if( FAudio_bMasterMusicVolChanged ) {
                // flag the stream to have the volume changed
				uStateChanges |= _STREAM_STATE_CHANGE_VOLUME;
			}
			
			if( uStateChanges )
			{
				if( 1 == poStream->uChannels ) // Mono.
				{
					// Volume / Pan.
					if( uStateChanges & ( _STREAM_STATE_CHANGE_VOLUME | _STREAM_STATE_CHANGE_PAN ) ) {
						fVolume = ( poStream->bTreatAsSfx ) ? FAudio_fMasterSfxUnitVol : FAudio_fMasterMusicUnitVol;
						fVolume *= poStream->fVolume;
						sndStreamMixParameter( poStream->oMusyxStreamLeft, fmath_FloatToU32( 127.0f * _GetVolume( fVolume ) ), fmath_FloatToU32( 63.5f * ( 1.0f + poStream->fPanLeftRight ) ), 64, 0 );
					}

					// Frequency.
					if( _STREAM_STATE_CHANGE_FREQUENCY & uStateChanges ) {
						sndStreamFrq( poStream->oMusyxStreamLeft, fmath_FloatToU32( poStream->fFrequencyInHz * poStream->fFrequencyFactor ) );
					}

					// Stop.
					if( _STREAM_STATE_CHANGE_STOP & uStateChanges ) {
						sndStreamDeactivate( poStream->oMusyxStreamLeft );
					}

					// Play / unpause.
					if( ( _STREAM_STATE_CHANGE_PLAY & uStateChanges ) && ( poStream->uBufferBytesFilled == _uStreamBuffSize ) ) {
						sndStreamActivate( poStream->oMusyxStreamLeft );
					}
					if( _STREAM_STATE_CHANGE_UNPAUSE & uStateChanges ) {
						//restore the frequency to the streams original freq, effectively unpausing the stream.
						sndStreamFrq( poStream->oMusyxStreamLeft, fmath_FloatToU32( poStream->fFrequencyInHz * poStream->fFrequencyFactor ) );
					}
					if( _STREAM_STATE_CHANGE_PAUSE & uStateChanges ) {
						//because there is no pause API, and stop will reset the play pointer to the begining
						//of the stream, just set the frequency to zero.
						sndStreamFrq( poStream->oMusyxStreamLeft, fmath_FloatToU32( poStream->fFrequencyInHz * 0.0f ) );
					}
				}
				else // Stereo.
				{
					//// Volume / Pan.
					//
					if( uStateChanges & ( _STREAM_STATE_CHANGE_VOLUME | _STREAM_STATE_CHANGE_PAN ) ) {
						fVolume = ( poStream->bTreatAsSfx ) ? FAudio_fMasterSfxUnitVol : FAudio_fMasterMusicUnitVol;
						fVolume *= poStream->fVolume;

						// !!Nate - Scale by 0.6f is a hack
						if( 0.0f <= poStream->fPanLeftRight ) {
							sndStreamMixParameter( poStream->oMusyxStreamLeft,  fmath_FloatToU32( 127.0f * 0.6f * _GetVolume( fVolume * ( 1.0f - poStream->fPanLeftRight ) ) ), 0, 64, 0 );
							sndStreamMixParameter( poStream->oMusyxStreamRight, fmath_FloatToU32( 127.0f * 0.6f * _GetVolume( fVolume ) ), 127, 64, 0 );
						} else {
							sndStreamMixParameter( poStream->oMusyxStreamLeft,   fmath_FloatToU32( 127.0f * 0.6f * _GetVolume( fVolume ) ), 0, 64, 0 );
							sndStreamMixParameter( poStream->oMusyxStreamRight,  fmath_FloatToU32( 127.0f * 0.6f * _GetVolume( fVolume * ( 1.0f + poStream->fPanLeftRight ) ) ), 127, 64, 0 );
						}
					}
					//
					////

					//// Frequency.
					//
					if( _STREAM_STATE_CHANGE_FREQUENCY & uStateChanges )
					{
						sndStreamFrq( poStream->oMusyxStreamLeft,  fmath_FloatToU32( poStream->fFrequencyInHz * poStream->fFrequencyFactor ) );
						sndStreamFrq( poStream->oMusyxStreamRight, fmath_FloatToU32( poStream->fFrequencyInHz * poStream->fFrequencyFactor ) );
					}
					//
					////

					//// Stop.
					//
					if( _STREAM_STATE_CHANGE_STOP & uStateChanges )
					{
						sndStreamDeactivate( poStream->oMusyxStreamLeft );
						sndStreamDeactivate( poStream->oMusyxStreamRight );
					}
					//
					////

					//// Play / unpause.
					//
					if( ( _STREAM_STATE_CHANGE_PLAY & uStateChanges) && ( poStream->uBufferBytesFilled == _uStreamBuffSize ) )
					{
						sndStreamActivate( poStream->oMusyxStreamLeft );
						sndStreamActivate( poStream->oMusyxStreamRight );
					}
					if( _STREAM_STATE_CHANGE_UNPAUSE & uStateChanges )
					{
						//restore the frequency to the streams original freq, effectively unpausing the stream.
						sndStreamFrq( poStream->oMusyxStreamLeft,  fmath_FloatToU32( poStream->fFrequencyInHz * poStream->fFrequencyFactor ) );
						sndStreamFrq( poStream->oMusyxStreamRight, fmath_FloatToU32( poStream->fFrequencyInHz * poStream->fFrequencyFactor ) );
					}
					if( _STREAM_STATE_CHANGE_PAUSE & uStateChanges )
					{
						//because there is no pause API, and stop will reset the play pointer to the begining
						//of the stream, just set the frequency to zero.
						sndStreamFrq( poStream->oMusyxStreamLeft,  fmath_FloatToU32( poStream->fFrequencyInHz * 0.0f ) );
						sndStreamFrq( poStream->oMusyxStreamRight, fmath_FloatToU32( poStream->fFrequencyInHz * 0.0f ) );
					}


					// invoke end of stream callbacks...
					if( _STREAM_STATE_CHANGE_STOP & uStateChanges )
					{
						if( poStream->pEndOfPlayCallback )
						{
							poStream->pEndOfPlayCallback( poStream->poAudioStream );
						}
					}
					//
					////
				}
			}
			//
			////

			////
			//
			if( poStream->uFileChanges )
			{
				if( _STREAM_FILE_CHANGE_READ_HEADER == poStream->uFileChanges )
				{
					poStream->uFileChanges = _STREAM_FILE_CHANGE_NONE;
					if( FFILE_ERROR_RET == ffile_Read( poStream->ohFile, sizeof( FGCData_WvsFile_Header_t ), poStream->aaauFileBuff, _FFileAsyncReadHeaderCallback ) )
					{
						poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
					}
					/* poStream->uFileSize = */(u32)ffile_GetFileSize( poStream->ohFile );
					//poStream->uFilePos  = sizeof( FGCData_WvsFile_Header_t );
				}
				else if( _STREAM_FILE_CHANGE_READ_BODY == poStream->uFileChanges )
				{
					poStream->uFileChanges = _STREAM_FILE_CHANGE_READ_BODY_PENDING;
					if( FFILE_ERROR_RET == ffile_Read( poStream->ohFile, ( ( 1 == poStream->uChannels ) ? FGCDATA_WVS_MAX_CHUNK_BYTES : ( 2 * FGCDATA_WVS_MAX_CHUNK_BYTES ) ), poStream->aaauFileBuff[ poStream->uCurrentReadBuff ], _FFileAsyncReadBodyCallback ) )
					{
						poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
					}
				}
				else if( _STREAM_FILE_CHANGE_EOF == poStream->uFileChanges )
				{
					if( poStream->bIsLooping ) {
						poStream->uFileChanges = _STREAM_FILE_CHANGE_NONE;					
						if( FFILE_ERROR_RET == ffile_Seek( poStream->ohFile, sizeof( FGCData_WvsFile_Header_t ), FFILE_SEEK_SET ) )
						{
							poStream->oeState      = FAUDIO_STREAM_STATE_ERROR;
						}
						//u32 uOldFilePos = poStream->uFilePos;
						poStream->uFilePos  = sizeof( FGCData_WvsFile_Header_t );
					} 
					else {
						// This is the end of a sound.  Fill the rest of the buffer with silence
						// until the audio system terminates the sound.
						poStream->uFileChanges = _STREAM_FILE_CHANGE_PAD_WITH_SILENCE;
					}
					//DEVPRINTF( "Reseting FilePos from %d to %d\n", uOldFilePos, poStream->uFilePos );
				}
			}
			//
			////

			////
			//
			poStream->bIsInUse = FALSE;
			//
			////
		}

#if 0
		// Commented out by RAF because the end of stream callback is called above.
		// THis logic would break with the new CACHED state change theory...
		// All operations on state changes need to be done in the code block above...
		
		
		//// Invoke end-of-play callbacks.
		//
		for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
		{
			////
			//
			poStream = &( _paoStreams[ uIndex ] );

			if( ! poStream->bIsActive )
			{
				continue;
			}
			//
			////

			////
			//
			if( _STREAM_STATE_CHANGE_STOP & poStream->uStateChanges )
			{
				if( poStream->pEndOfPlayCallback )
				{
					poStream->pEndOfPlayCallback( poStream->poAudioStream );
				}
			}
			//
			////

			poStream->uStateChanges = _STREAM_STATE_CHANGE_NONE;
		}
		//
		////
#endif

		_bSkipStreamsWorkDelay = TRUE;

		_fStreamsWorkDelay = 0.0f;
	}
	//
	//// Streams work.

	// reset the master volume change vars
	FAudio_bMasterSfxVolChanged = FALSE;
	FAudio_bMasterMusicVolChanged = FALSE;

} // faudio_Work

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


FAudio_PauseLevel_e CFAudioEmitter::SetGlobalPauseLevel( FAudio_PauseLevel_e eNewPauseLevel )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_PAUSE_LEVEL_NONE;
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

/*
	if( bEnable )
	{
		if( ! _bPauseModeEmitters )
		{
			for( u32 uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
			{
				// 2D.
				poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ uIndex ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
					poVirtualEmitter->poAudioEmitter->Pause( TRUE );
					poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
					poVirtualEmitter                     = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}

				// 3D.
				poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
					poVirtualEmitter->poAudioEmitter->Pause( TRUE );
					poVirtualEmitter->uProperties |= _EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
					poVirtualEmitter                     = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}
			}

			_bPauseModeEmitters = TRUE;
		}
	}
	else
	{
		if( _bPauseModeEmitters )
		{
			for( u32 uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
			{
				// 2D.
				poVirtualEmittersList = &( _paoVirtualEmittersListActive2D[ uIndex ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
					if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_IGNOREINPAUSEMODE )
					{
						poVirtualEmitter->poAudioEmitter->Pause( FALSE );
						poVirtualEmitter->uProperties &= ~_EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
					}

					poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}

				// 3D.
				poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				while( poVirtualEmitter )
				{
					if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_IGNOREINPAUSEMODE )
					{
						poVirtualEmitter->poAudioEmitter->Pause( FALSE );
						poVirtualEmitter->uProperties &= ~_EMITTER_PROPERTIES_IGNOREINPAUSEMODE;
					}

					poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
				}
			}

			_bPauseModeEmitters = FALSE;
		}
	}
*/

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

	return _uMaxSoundBytes;

} // faudio_GetBankMemoryAvailable

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 faudio_GetBankMemoryUsed( void )
{
	return 0;

} // faudio_GetBankMemoryUsed

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_Error_e faudio_LoadBanks( cchar **papszNames, FAudio_BankHandle_t *paoBankHandles )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_ERROR;
	}

	FASSERT_MSG( papszNames,                                      "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( *papszNames,                                     "[ FAUDIO ] Error: Zero length array !!!" );
	FASSERT_MSG( **papszNames,                                    "[ FAUDIO ] Error: Zero length string !!!" );
	FASSERT_MSG( paoBankHandles,                                  "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( ( ! _pMusyxSamplesBuff ),                        "[ FAUDIO ] Error: Non-NULL pointer !!!" );
	FASSERT_MSG( _uMusyxSamplesBuff,                              "[ FAUDIO ] Error: Zero length buffer !!!" );
	FASSERT_MSG( ( _uMusyxSamplesBuff == ARQGetChunkSize() ),     "[ FAUDIO ] Error: Invalid buffer length !!!" );
	FASSERT_MSG( ( FFILE_INVALID_HANDLE == _ohMusyxSamplesFile ), "[ FAUDIO ] Error: Invalid file handle !!!" );

	FMemFrame_t oMemFrame = fmem_GetFrame();

	// Determine if we should even try and load a localized version...
	char cLanguageChar, cAudioLanguageChar;
	ffile_GetLanguageChars( &cLanguageChar, &cAudioLanguageChar );

	//// Musyx samples.
	//

	// First, see if we can load the localized version of the snd_smpls file...
	if( cAudioLanguageChar )
	{
		char szLocalizedFilename[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
		sprintf( szLocalizedFilename, "snd_smpls%c.rdg", cAudioLanguageChar );
		_ohMusyxSamplesFile = ffile_Open( szLocalizedFilename, FFILE_OPEN_RONLY );
	}

	if( FFILE_INVALID_HANDLE == _ohMusyxSamplesFile )
	{
		// Try and load the non-localized filename version
		_ohMusyxSamplesFile = ffile_Open( "snd_smpls.rdg", FFILE_OPEN_RONLY );
	}

	if( FFILE_INVALID_HANDLE == _ohMusyxSamplesFile )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not open \"snd_smpls.rdg\" file !!!\n", __LINE__ );
		return FAUDIO_ERROR;
	}

	FASSERT_MSG( ffile_GetFileSize( _ohMusyxSamplesFile ), "[ FAUDIO ] Error: Invalid file !!!" );

	_pMusyxSamplesBuff = fmem_Alloc( _uMusyxSamplesBuff, 32 );
	if( ! _pMusyxSamplesBuff )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fmem_Alloc() failed !!!\n", __LINE__ );
		ffile_Close( _ohMusyxSamplesFile );
		_ohMusyxSamplesFile = FFILE_INVALID_HANDLE;
		return FAUDIO_ERROR;
	}
	//
	////

	sndSetSampleDataUploadCallback( _MusyxSamplesLoadCallback, _uMusyxSamplesBuff );

	////
	//
	cchar **pszName = papszNames;
	FAudio_BankHandle_t *poBankHandle = paoBankHandles;

	FResFrame_t oResFrame = fres_GetFrame();

	do
	{
		FASSERT_MSG( ( FAUDIO_MAX_ASSET_NAME_LENGTH >= fclib_strlen( *pszName ) ), "[ FAUDIO ] Error: Invalid bank name !!!" );

		*poBankHandle = NULL;
		
		if( cAudioLanguageChar != 0 ) 
		{
			// Try and load a localized version of the file...
			char szLocalizedFilename[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
			fclib_strcpy( szLocalizedFilename, *pszName );
			
			u32 nStringLen = fclib_strlen( szLocalizedFilename );
			if( nStringLen < FAUDIO_MAX_ASSET_NAME_LENGTH )
			{
				szLocalizedFilename[ nStringLen ] = cAudioLanguageChar;
				szLocalizedFilename[ nStringLen + 1 ] = 0x00;

				// Now, try and load this filename
				*poBankHandle = (FAudio_BankHandle_t)fresload_Load( FAUDIOBANK_RESTYPE, szLocalizedFilename );
			}
		}

		if( !*poBankHandle )
		{
			// Try and load a NON-LOCALIZED version of this audio bank
			*poBankHandle = (FAudio_BankHandle_t)fresload_Load( FAUDIOBANK_RESTYPE, *pszName );
		}

		if( ! *poBankHandle )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: fresload_Load() failed !!!\n", __LINE__ );
			ffile_Close( _ohMusyxSamplesFile );
			_ohMusyxSamplesFile = FFILE_INVALID_HANDLE;
			fres_ReleaseFrame( oResFrame );
			fmem_ReleaseFrame( oMemFrame );
			_pMusyxSamplesBuff = NULL;
			return FAUDIO_ERROR;
		}

		++pszName;
		++poBankHandle;

	} while( *pszName );
	//
	////

	ffile_Close( _ohMusyxSamplesFile );
	_ohMusyxSamplesFile = FFILE_INVALID_HANDLE;
	fmem_ReleaseFrame( oMemFrame );
	_pMusyxSamplesBuff = NULL;
	sndSetSampleDataUploadCallback( NULL, 0 );

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
	FASSERT_MSG( ( ! _pMusyxSamplesBuff ),                                    "[ FAUDIO ] Error: Non-NULL pointer !!!" );
	FASSERT_MSG( _uMusyxSamplesBuff,                                          "[ FAUDIO ] Error: Zero length buffer !!!" );
	FASSERT_MSG( ( _uMusyxSamplesBuff == ARQGetChunkSize() ),                 "[ FAUDIO ] Error: Invalid buffer length !!!" );
	FASSERT_MSG( ( FFILE_INVALID_HANDLE == _ohMusyxSamplesFile ),             "[ FAUDIO ] Error: Invalid file handle !!!" );
	FASSERT_MSG( ( FAUDIO_MAX_ASSET_NAME_LENGTH >= fclib_strlen( pszName ) ), "[ FAUDIO ] Error: Invalid bank name !!!" );

	FMemFrame_t oMemFrame = fmem_GetFrame();
	// Determine if we should even try and load a localized version...
	char cLanguageChar, cAudioLanguageChar;
	ffile_GetLanguageChars( &cLanguageChar, &cAudioLanguageChar );

	//// Musyx samples.
	//
	// First, see if we can load the localized version of the snd_smpls file...
	if( cAudioLanguageChar )
	{
		char szLocalizedFilename[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
		sprintf( szLocalizedFilename, "snd_smpls%c.rdg", cAudioLanguageChar );
		_ohMusyxSamplesFile = ffile_Open( szLocalizedFilename, FFILE_OPEN_RONLY );
	}

	if( FFILE_INVALID_HANDLE == _ohMusyxSamplesFile )
	{
		// Try and load the non-localized filename version
		_ohMusyxSamplesFile = ffile_Open( "snd_smpls.rdg", FFILE_OPEN_RONLY );
	}

	if( FFILE_INVALID_HANDLE == _ohMusyxSamplesFile )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not open \"snd_smpls.rdg\" file !!!\n", __LINE__ );
		return FAUDIO_INVALID_HANDLE;
	}

	FASSERT_MSG( ffile_GetFileSize( _ohMusyxSamplesFile ), "[ FAUDIO ] Error: Invalid file !!!" );

	_pMusyxSamplesBuff = fmem_Alloc( _uMusyxSamplesBuff, 32 );
	if( ! _pMusyxSamplesBuff )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fmem_Alloc() failed !!!\n", __LINE__ );
		ffile_Close( _ohMusyxSamplesFile );
		_ohMusyxSamplesFile = FFILE_INVALID_HANDLE;
		return FAUDIO_INVALID_HANDLE;
	}
	//
	////

	sndSetSampleDataUploadCallback( _MusyxSamplesLoadCallback, _uMusyxSamplesBuff );

	////
	//
	FAudio_BankHandle_t oBankHandle = FAUDIO_INVALID_HANDLE;
	FResFrame_t oResFrame = fres_GetFrame();

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
		// We have failed to load both the localized version and the non-localized version
		DEVPRINTF( "[ FAUDIO ] Error %u: fresload_Load() failed !!!\n", __LINE__ );
		ffile_Close( _ohMusyxSamplesFile );
		_ohMusyxSamplesFile = FFILE_INVALID_HANDLE;
		fres_ReleaseFrame( oResFrame );
		fmem_ReleaseFrame( oMemFrame );
		_pMusyxSamplesBuff = NULL;
		return FAUDIO_INVALID_HANDLE;
	}
	//
	////

	ffile_Close( _ohMusyxSamplesFile );
	_ohMusyxSamplesFile = FFILE_INVALID_HANDLE;
	fmem_ReleaseFrame( oMemFrame );
	_pMusyxSamplesBuff = NULL;
	sndSetSampleDataUploadCallback( NULL, 0 );

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
		_oTempSphere.m_fRadius = FAUDIO_MIN_RADIUS;

		for( u32 uIndex = 0; uIndex < _uActiveVirtualListeners; ++uIndex )
		{
			poVirtualListener               = &( _aoVirtualListeners[ uIndex ] );
			poVirtualListener->uStateChange = _LISTENER_STATE_CHANGE_NONE;
			poVirtualListener->poXfmCurrentOrientation_WS->Identity();
			poVirtualListener->poVecPreviousPosition_WS->Zero();
			poVirtualListener->poVecVelocity_WS->Zero();
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

void faudio_EnableSurround_GC( BOOL bSurroundEnabled )
{

	u32 uSysInfo = fsysinfo_GetCapabilities();

	if( bSurroundEnabled && ( uSysInfo & FSYSINFO_AUDIO_STEREO ) ) {
		// Only enable if we are in stereo mode
		sndOutputMode( SND_OUTPUTMODE_SURROUND );
		_bSurroundEnabled = TRUE;
	} else if( !bSurroundEnabled && ( uSysInfo & FSYSINFO_AUDIO_STEREO ) ) { 
		// disable surround... go to either mono or stereo
		sndOutputMode( SND_OUTPUTMODE_STEREO );
		_bSurroundEnabled = FALSE;
	} else {
		_bSurroundEnabled = FALSE;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL faudio_IsSurroundEnabled_GC( void ) 
{
	return _bSurroundEnabled;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CFAudioEmitter *CFAudioEmitter::Create2D( FAudio_WaveHandle_t oWaveHandle, u8 uPriority/*=FAudio_EmitterDefaultPriorityLevel*/, CFAudioEmitter **ppUserAudioEmitter )
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
		poVirtualEmitter->poVirtualListener            = NULL;
		poVirtualEmitter->fVolume                      = 1.0f;
		poVirtualEmitter->fVolumeDucked                = 1.0f;
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
	FASSERT_MSG( ( FAUDIO_MIN_RADIUS <= fRadiusOuter ),   "[ FAUDIO ] Error: Invalid fRadiusOuter !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)flinklist_RemoveHead( &_oVirtualEmittersListFree );
	if( poVirtualEmitter )
	{	
		// !!Nate
		poVirtualEmitter->poRealEmitter 			   = NULL;
		poVirtualEmitter->ppUserAudioEmitter           = ppUserAudioEmitter;
		poVirtualEmitter->oWaveHandle                  = oWaveHandle;
		poVirtualEmitter->uPriority                    = uPriority;
		poVirtualEmitter->poVirtualListener            = NULL;
		poVirtualEmitter->fVolume                      = 1.0f;
		poVirtualEmitter->fVolumeDucked                = 1.0f;
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
		poVirtualEmitter->fRadiusOuter                 = fRadiusOuter;
//		poVirtualEmitter->fRadiusInner                 = FAUDIO_MIN_RADIUS;
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

			if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_3D )
			{
				sndRemoveEmitter( &( poVirtualEmitter->poRealEmitter->oMusyxEmitterAttrib ) );
			}
			else
			{
				sndFXKeyOff( poVirtualEmitter->poRealEmitter->oMusyxVoice );
			}

			--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
		}

		if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_3D )
		{
			flinklist_Remove( &( _oRealEmittersListActive3D ), poVirtualEmitter->poRealEmitter );
			flinklist_AddTail( &( _oRealEmittersListFree3D ), poVirtualEmitter->poRealEmitter );
		}
		else
		{
			flinklist_Remove( &( _oRealEmittersListActive2D ), poVirtualEmitter->poRealEmitter );
			flinklist_AddTail( &( _oRealEmittersListFree2D ), poVirtualEmitter->poRealEmitter );
		}

		// !!Nate
		poVirtualEmitter->uStateChanges 				  				 = _EMITTER_STATE_CHANGE_NONE;
		poVirtualEmitter->poRealEmitter->b3DStartedAtPreviousFrame       = FALSE;
		
		poVirtualEmitter->poRealEmitter->poVirtualEmitter = NULL;
		poVirtualEmitter->poRealEmitter                   = NULL;		
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

			poVirtualEmitter->poAudioEmitter->Destroy();
		}

		// 3D.
		poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ uIndex ] );

		while( poVirtualEmittersList->nCount )
		{
			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

			poVirtualEmitter->poAudioEmitter->Destroy();
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
//	_oTempVec3A = *poVecPosition_WS;
//	_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );

//	if( _SIGNIFICANT_DISTANCE_CHANGE_SQ < _oTempVec3A.MagSq() )
//	{
		poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_POSITION;

		_oTempVec3A_Velocity = *( poVirtualEmitter->poVecCurrentPosition_WS );
		_oTempVec3A_Velocity.Sub( *( poVirtualEmitter->poVecPreviousPosition_WS ) );
		_oTempVec3A = *( poVecPosition_WS );
		_oTempVec3A.Sub( _oTempVec3A_Velocity );

		*( poVirtualEmitter->poVecPreviousPosition_WS ) = _oTempVec3A;
		*( poVirtualEmitter->poVecCurrentPosition_WS )  = *( poVecPosition_WS );
//	}

} // CFAudioEmitter::SetPositionAndMaintainDoppler

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioEmitter::SetRadius( f32 fRadiusOuter, f32 fRadiusInner /* = FAUDIO_MIN_RADIUS */ )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                                  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D, "[ FAUDIO ] Error: Illegal call !!!" );
	FASSERT_MSG( ( fRadiusInner <= fRadiusOuter ),                   "[ FAUDIO ] Error: Invalid fRadiusOuter !!!" );
	FASSERT_MSG( ( FAUDIO_MIN_RADIUS <= fRadiusInner ),              "[ FAUDIO ] Error: Invalid fRadiusInner !!!" );

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
	FASSERT_MSG( ( ! (((_VirtualEmitter_t *)m_pUser)->uProperties & _EMITTER_PROPERTIES_3D )), "[ FAUDIO ] Error: Illegal call !!!" );
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
//	f32 fTemp = ( poVirtualEmitter->fDopplerFactor - fDopplerFactor );
//	fTemp = ( fTemp * fTemp );

//	if( _SIGNIFICANT_FACTOR_CHANGE_SQ < fTemp )
//	{
		poVirtualEmitter->uStateChanges  |= _EMITTER_STATE_CHANGE_DOPPLER;
		poVirtualEmitter->fDopplerFactor  = fDopplerFactor;
//	}
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

	return !!( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_DUCKABLE );

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
			poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_UNPAUSE;
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
//										f32 fRadiusInner = FAUDIO_MIN_RADIUS,
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
//		poAudioEmitter->SetRadius( fRadiusOuter, FAUDIO_MIN_RADIUS );
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

void _AutoDestroyStreamEndOfPlayCallback( CFAudioStream *poAudioStream )
{
	poAudioStream->Destroy();

} // _AutoDestroyStreamEndOfPlayCallback

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

				if( 0.0f <= poVirtualEmitter->fSecondsToPlay )
				{
					// Non-infinitely looping.
					if( poVirtualEmitter->fSecondsToPlay <= poVirtualEmitter->fSecondsPlayed )
					{
						poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_STOPPED;
						poVirtualEmitter->uStateChanges  = _EMITTER_STATE_CHANGE_STOP;
						poVirtualEmitter->uProperties   &= ~_EMITTER_PROPERTIES_LOOPING;
						_bSkipEmittersListenersWorkDelay = FALSE;
					}
					else
					{
						if( ( poVirtualEmitter->poRealEmitter ) &&
							( ! poVirtualEmitter->poRealEmitter->b3DStartedAtPreviousFrame ) )
						{
							if( SND_ID_ERROR == sndFXCheck( poVirtualEmitter->poRealEmitter->oMusyxVoice ) )
							{
								poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_STOPPED;
								poVirtualEmitter->uStateChanges  = _EMITTER_STATE_CHANGE_STOP;
								_bSkipEmittersListenersWorkDelay = FALSE;
							}
						}
					}
				}
				else
				{
					// Infinitely looping.
					if( ( poVirtualEmitter->poRealEmitter ) &&
						( ! poVirtualEmitter->poRealEmitter->b3DStartedAtPreviousFrame ) )
					{
						if( SND_ID_ERROR == sndFXCheck( poVirtualEmitter->poRealEmitter->oMusyxVoice ) )
						{
							poVirtualEmitter->oeState        = FAUDIO_EMITTER_STATE_STOPPED;
							poVirtualEmitter->uStateChanges  = _EMITTER_STATE_CHANGE_STOP;
							_bSkipEmittersListenersWorkDelay = FALSE;
						}
					}
				}
			}

			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

} // _TrackEmittersProgress

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _TrackStreamsProgress( void )
{
	_Stream_t *poStream;

	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		////
		//
		poStream = &( _paoStreams[ uIndex ] );

		if( ! poStream->bIsActive )
		{
			continue;
		}
		//
		////

		////
		//
		if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState )
		{
			poStream->fSecondsPlayed += ( FLoop_fRealPreviousLoopSecs * poStream->fFrequencyFactor );

			if( 0.0f <= poStream->fSecondsToPlay )
			{
				// Non-infinitely looping.
				if( poStream->fSecondsToPlay <= poStream->fSecondsPlayed )
				{
					poStream->oeState       = FAUDIO_STREAM_STATE_STOPPED;
					poStream->uStateChanges = _STREAM_STATE_CHANGE_STOP;
					poStream->bIsLooping    = FALSE;
					_bSkipStreamsWorkDelay  = FALSE;
				}
			}
		}
		//
		////
	}

} // _TrackStreamsProgress

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _ApplyRealEmittersChanges( FLinkRoot_t *poVirtualEmittersListActive )
{
	FLinkRoot_t *poVirtualEmittersList;
	_VirtualEmitter_t *poVirtualEmitter;
	FDataWvbFile_Bank_t *poBank;
	FDataWvbFile_Wave_t *poWave;
	SND_FVECTOR oMusyxVecPosition, oMusyxVecVelocity;
	u32 uList;
	f32 fVolume;

	for( uList = 0; uList < _uMaxPriorityLevels; ++uList )
	{
		poVirtualEmittersList = &( poVirtualEmittersListActive[ uList ] );

		poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );
		while( poVirtualEmitter )
		{
			if( poVirtualEmitter->poRealEmitter &&
				( ! poVirtualEmitter->poRealEmitter->b3DStartedAtPreviousFrame ) )
			{
				// calculate the volume here one time
				fVolume = _GetVolume( poVirtualEmitter->fVolumeDucked * FAudio_fMasterSfxUnitVol );

				if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_3D )
				{
					if( poVirtualEmitter->poVirtualListener )
					{
						//// Play / unpause.
						//
						if( _EMITTER_STATE_CHANGE_PLAY & poVirtualEmitter->uStateChanges )
						{
							//// Position.
							//
							poVirtualEmitter->poVirtualListener->poXfmCurrentOrientation_WS->TransformPointR( _oTempVec3A.v3, poVirtualEmitter->poVecCurrentPosition_WS->v3 );

							oMusyxVecPosition.x = _oTempVec3A.x;
							oMusyxVecPosition.y = _oTempVec3A.y;
							oMusyxVecPosition.z = - _oTempVec3A.z;
							//
							////

							//// Velocity.
							//
							// Emitter velocity.
							_oTempVec3A = *( poVirtualEmitter->poVecCurrentPosition_WS );
							_oTempVec3A.Sub( *( poVirtualEmitter->poVecPreviousPosition_WS ) );

							// Emitter and listener combined velocity.
							_oTempVec3A.Sub( *( poVirtualEmitter->poVirtualListener->poVecVelocity_WS ) );
							_oTempVec3A.Mul( _fOOEmittersListenersWorkDelay );

							poVirtualEmitter->poVirtualListener->poXfmCurrentOrientation_WS->TransformDirR( _oTempVec3A_Velocity.v3, _oTempVec3A.v3 );

							oMusyxVecVelocity.x = _oTempVec3A_Velocity.x;
							oMusyxVecVelocity.y = _oTempVec3A_Velocity.y;
							oMusyxVecVelocity.z = - _oTempVec3A_Velocity.z;
							//
							////

							////
							//
							poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
							poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);
					
							if( poWave->uLength >= 100000 ) {
								DEVPRINTF( "Known audio bug has occurred!  The GameCube will now auto-pause.\n" );
							}
							
							// !!Nate
							poVirtualEmitter->poRealEmitter->oMusyxVoice = sndAddEmitter( &( poVirtualEmitter->poRealEmitter->oMusyxEmitterAttrib ),
								&oMusyxVecPosition, &oMusyxVecVelocity,
								( poVirtualEmitter->fRadiusOuter * _3D_SOUND_RADIUS_SCALE ),
								0.0f,
								( SND_EMITTER_ITD | SND_EMITTER_CONTINUOUS | SND_EMITTER_DOPPLERFX ),
								*( ((u32 *)poBank->pData) + 1 + poWave->uIndex ),
								fmath_FloatToU32( 127.0f * _3D_SOUND_VOLUME_SCALE * fVolume ), 0.0f, NULL );

							if( SND_ID_ERROR == poVirtualEmitter->poRealEmitter->oMusyxVoice )
							{
								poVirtualEmitter->poRealEmitter->b3DStartedAtPreviousFrame = TRUE;
								poVirtualEmitter->poRealEmitter->f3DStartDelay = 0.0f;
								poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
								continue;
							}
							//
							////
						}
						//
						////

						//// Position.
						//
						//first, update the position if the listener is in a auto update mode...
						if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_AUTOPOSUPDATE ) {
							poVirtualEmitter->poAudioEmitter->SetPosition( poVirtualEmitter->poVecAutoPosition_WS );
						}
						if( ( _EMITTER_STATE_CHANGE_POSITION & poVirtualEmitter->uStateChanges ) ||
							( _LISTENER_STATE_CHANGE_ORIENTATION & poVirtualEmitter->poVirtualListener->uStateChange ) )
						{
							//// Position.
							//
							poVirtualEmitter->poVirtualListener->poXfmCurrentOrientation_WS->TransformPointR( _oTempVec3A.v3, poVirtualEmitter->poVecCurrentPosition_WS->v3 );

							oMusyxVecPosition.x = _oTempVec3A.x;
							oMusyxVecPosition.y = _oTempVec3A.y;
							oMusyxVecPosition.z = - _oTempVec3A.z;
							//
							////

							//// Velocity.
							//
							// Emitter velocity.
							_oTempVec3A = *( poVirtualEmitter->poVecCurrentPosition_WS );
							_oTempVec3A.Sub( *( poVirtualEmitter->poVecPreviousPosition_WS ) );

							// Emitter and listener combined velocity.
							_oTempVec3A.Sub( *( poVirtualEmitter->poVirtualListener->poVecVelocity_WS ) );
							_oTempVec3A.Mul( _fOOEmittersListenersWorkDelay );

							poVirtualEmitter->poVirtualListener->poXfmCurrentOrientation_WS->TransformDirR( _oTempVec3A_Velocity.v3, _oTempVec3A.v3 );

							oMusyxVecVelocity.x = _oTempVec3A_Velocity.x;
							oMusyxVecVelocity.y = _oTempVec3A_Velocity.y;
							oMusyxVecVelocity.z = - _oTempVec3A_Velocity.z;
							//
							////

							////
							//					
							// !!Nate
							sndUpdateEmitter(	&( poVirtualEmitter->poRealEmitter->oMusyxEmitterAttrib ),
												&oMusyxVecPosition, &oMusyxVecVelocity,
												fmath_FloatToU32( 127.0f * _3D_SOUND_VOLUME_SCALE * fVolume ), NULL );
							//
							////
						}
						else
						{
							//// Position.
							//
							poVirtualEmitter->poVirtualListener->poXfmCurrentOrientation_WS->TransformPointR( _oTempVec3A.v3, poVirtualEmitter->poVecCurrentPosition_WS->v3 );

							oMusyxVecPosition.x = _oTempVec3A.x;
							oMusyxVecPosition.y = _oTempVec3A.y;
							oMusyxVecPosition.z = - _oTempVec3A.z;
							//
							////

							//// Velocity.
							//
							oMusyxVecVelocity.x = 0.0f;
							oMusyxVecVelocity.y = 0.0f;
							oMusyxVecVelocity.z = 0.0f;
							//
							////

							////
							//				
							// !!Nate
							sndUpdateEmitter( &poVirtualEmitter->poRealEmitter->oMusyxEmitterAttrib,
                                &oMusyxVecPosition,
								&oMusyxVecVelocity,
								fmath_FloatToU32( 127.0f * _3D_SOUND_VOLUME_SCALE * fVolume ),
								NULL );
							//
							////
						}
					}
					//
					////

					//// Reverb.
					//
					if( _EMITTER_STATE_CHANGE_REVERB & poVirtualEmitter->uStateChanges )
					{
						sndFXReverb( poVirtualEmitter->poRealEmitter->oMusyxVoice, fmath_FloatToU32( 127.0f * poVirtualEmitter->fReverb ) );
					}
					//
					////

					//// Doppler.
					//
					if( _EMITTER_STATE_CHANGE_DOPPLER & poVirtualEmitter->uStateChanges )
					{
						// Valid range for Musyx Doppler is 0 - 65535.  8192 is no doppler.  So we have to convert our 0-10 range
						sndFXDoppler( poVirtualEmitter->poRealEmitter->oMusyxVoice, fmath_FloatToU32( 8192.0f + (poVirtualEmitter->fDopplerFactor * 5734) ) ); 
					}
					//
					////
				}
				else // 2D.
				{
					//// Play / unpause.
					//
					if( _EMITTER_STATE_CHANGE_PLAY & poVirtualEmitter->uStateChanges )
					{				
						poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
						poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);
						
						// !!Nate
						poVirtualEmitter->poRealEmitter->oMusyxVoice = sndFXStart( *( ((u32 *)poBank->pData) + 1 + poWave->uIndex ), 
							fmath_FloatToU32( 127.0f * fVolume ),
							fmath_FloatToU32( 63.5f * ( 1.0f + poVirtualEmitter->fPanLeftRight ) ) );
							
						if( SND_ID_ERROR == poVirtualEmitter->poRealEmitter->oMusyxVoice )
						{
							poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
							continue;
						}
					}
					//
					////
					

					//// Pan.
					//
					if( _EMITTER_STATE_CHANGE_PAN & poVirtualEmitter->uStateChanges )
					{
						sndFXPanning( poVirtualEmitter->poRealEmitter->oMusyxVoice, fmath_FloatToU32( 63.5f * ( 1.0f + poVirtualEmitter->fPanLeftRight ) ) );
						sndFXSurroundPanning( poVirtualEmitter->poRealEmitter->oMusyxVoice, 64 ); //set the sound equadistant forward / rear
					}
					//
					////

					//// Volume (only set the volume for 2d sounds, because sndUpdateEmitter takes a volume param for 3D sounds).
					//
					if( FAudio_bMasterSfxVolChanged ||
						_EMITTER_STATE_CHANGE_VOLUME & poVirtualEmitter->uStateChanges )
					{
						// !!Nate
						sndFXVolume( poVirtualEmitter->poRealEmitter->oMusyxVoice, fmath_FloatToU32( 127.0f * fVolume ) );
					}
					//
					////
				}


				//// Frequency.
				//
				if( _EMITTER_STATE_CHANGE_FREQUENCY & poVirtualEmitter->uStateChanges )
				{
					sndFXPitchBend( poVirtualEmitter->poRealEmitter->oMusyxVoice, fmath_FloatToU32( 8192.0f * poVirtualEmitter->fFrequencyFactor ) );
				}
				//
				////
				
				//// Pause
				//
				if( _EMITTER_STATE_CHANGE_PAUSE & poVirtualEmitter->uStateChanges )
				{
					sndFXDoppler( poVirtualEmitter->poRealEmitter->oMusyxVoice, fmath_FloatToU32( 8192.0f * 0 ) );
				}
				//
				////

				//// Unpause
				//
				if( _EMITTER_STATE_CHANGE_UNPAUSE & poVirtualEmitter->uStateChanges )
				{
					sndFXDoppler( poVirtualEmitter->poRealEmitter->oMusyxVoice, fmath_FloatToU32( 8192.0f * poVirtualEmitter->fFrequencyFactor ) );
				}
				//
				////
			}

			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

} // _ApplyRealEmittersChanges

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _AllocateRealEmittersToSingleVirtualListener( void )
{
	FLinkRoot_t *poVirtualEmittersList;
	FLinkRoot_t *poVirtualEmittersListCull;
	_VirtualEmitter_t *poVirtualEmitter;
	_VirtualEmitter_t *poVirtualEmitterCull;
	_RealEmitter_t *poRealEmitterCull;
	_RealEmitter_t *poRealEmitter;
	u32 uIndex;
	s32 nList;
	u32 uPriority;

	for( nList = ( (s32)_uMaxPriorityLevels - 1 ); 0 <= nList ; --nList )
	{
		poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ nList ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			if( ( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState ) &&
				( ! poVirtualEmitter->poRealEmitter ) )
			{
				uPriority = poVirtualEmitter->uPriority;

				if( _oRealEmittersListFree3D.nCount )
				{
					if( _paoRealEmittersLimits[ uPriority ].uPlaying < _paoRealEmittersLimits[ uPriority ].uPlayable )
					{
						poRealEmitter = (_RealEmitter_t *)flinklist_RemoveHead( &_oRealEmittersListFree3D );
						flinklist_AddTail( &_oRealEmittersListActive3D, poRealEmitter );
						poVirtualEmitter->poRealEmitter                   = poRealEmitter;
						poVirtualEmitter->poRealEmitter->poVirtualEmitter = poVirtualEmitter;
						poVirtualEmitter->uStateChanges     |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
						poVirtualEmitter->poVirtualListener  = _aoVirtualListeners;
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
						sndRemoveEmitter( &( poRealEmitterCull->oMusyxEmitterAttrib ) );
						poVirtualEmitterCull->poRealEmitter->poVirtualEmitter = NULL;
						poVirtualEmitterCull->poRealEmitter                   = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitterCull->uPriority ].uPlaying );

						poVirtualEmitter->poRealEmitter                   = poRealEmitterCull;
						poVirtualEmitter->poRealEmitter->poVirtualEmitter = poVirtualEmitter;
						poVirtualEmitter->uStateChanges     |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
						poVirtualEmitter->poVirtualListener  = _aoVirtualListeners;
						++( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );
					}
				}
			}

			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

} // _AllocateRealEmittersToSingleVirtualListener

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _AllocateRealEmittersToMultipleVirtualListener( void )
{
	FLinkRoot_t *poVirtualEmittersList;
	FLinkRoot_t *poVirtualEmittersListCull;
	_VirtualListener_t *poVirtualListener;
	_VirtualEmitter_t *poVirtualEmitter;
	_VirtualEmitter_t *poVirtualEmitterCull;
	_RealEmitter_t *poRealEmitterCull;
	_RealEmitter_t *poRealEmitter;
	u32 uIndex;
	s32 nList;
	f32 fTempDistanceSqMin, fTempDistanceSq;
	u32 uPriority;

	for( nList = ( (s32)_uMaxPriorityLevels - 1 ); 0 <= nList ; --nList )
	{
		poVirtualEmittersList = &( _paoVirtualEmittersListActive3D[ nList ] );
		poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

		while( poVirtualEmitter )
		{
			if( ( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState ) &&
				( ! poVirtualEmitter->poRealEmitter ) )
			{
				uPriority = poVirtualEmitter->uPriority;

				if( _oRealEmittersListFree3D.nCount )
				{
					if( _paoRealEmittersLimits[ uPriority ].uPlaying < _paoRealEmittersLimits[ uPriority ].uPlayable )
					{
						poRealEmitter = (_RealEmitter_t *)flinklist_RemoveHead( &_oRealEmittersListFree3D );
						flinklist_AddTail( &_oRealEmittersListActive3D, poRealEmitter );
						poVirtualEmitter->poRealEmitter                   = poRealEmitter;
						poVirtualEmitter->poRealEmitter->poVirtualEmitter = poVirtualEmitter;
						poVirtualEmitter->uStateChanges     |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
						poVirtualEmitter->poVirtualListener  = _aoVirtualListeners;
						++( _paoRealEmittersLimits[ uPriority ].uPlaying );

						_oTempVec3A = _aoVirtualListeners->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
						_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );
						fTempDistanceSqMin = _oTempVec3A.MagSq();

						for( uIndex = 1; uIndex < _uActiveVirtualListeners; ++uIndex )
						{
							poVirtualListener = &( _aoVirtualListeners[ uIndex ] );

							_oTempVec3A = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
							_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );
							fTempDistanceSq = _oTempVec3A.MagSq();

							if( fTempDistanceSqMin > fTempDistanceSq )
							{
								fTempDistanceSqMin                  = fTempDistanceSq;
								poVirtualEmitter->poVirtualListener = poVirtualListener;
							}
						}
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
						sndRemoveEmitter( &( poRealEmitterCull->oMusyxEmitterAttrib ) );
						poVirtualEmitterCull->poRealEmitter->poVirtualEmitter = NULL;
						poVirtualEmitterCull->poRealEmitter                   = NULL;
						--( _paoRealEmittersLimits[ poVirtualEmitterCull->uPriority ].uPlaying );

						poVirtualEmitter->poRealEmitter                   = poRealEmitterCull;
						poVirtualEmitter->poRealEmitter->poVirtualEmitter = poVirtualEmitter;
						poVirtualEmitter->uStateChanges     |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
						poVirtualEmitter->poVirtualListener  = _aoVirtualListeners;
						++( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

						_oTempVec3A = _aoVirtualListeners->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
						_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );
						fTempDistanceSqMin = _oTempVec3A.MagSq();

						for( uIndex = 1; uIndex < _uActiveVirtualListeners; ++uIndex )
						{
							poVirtualListener = &( _aoVirtualListeners[ uIndex ] );

							_oTempVec3A = poVirtualListener->poXfmCurrentOrientation_WS->m_MtxF.m_vPos;
							_oTempVec3A.Sub( *( poVirtualEmitter->poVecCurrentPosition_WS ) );
							fTempDistanceSq = _oTempVec3A.MagSq();

							if( fTempDistanceSqMin > fTempDistanceSq )
							{
								fTempDistanceSqMin                   = fTempDistanceSq;
								poVirtualEmitter->poVirtualListener  = poVirtualListener;
							}
						}
					}
				}
			}

			poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
		}
	}

} // _AllocateRealEmittersToMultipleVirtualListener

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void *_MusyxMalloc( u32 uSize )
{
	void *pMem = fang_Malloc( uSize, 32 );
	
	if( !pMem ) {
		OSReport( "_MusyxMalloc - fang_Malloc() returned NULL!\n" );
	}
	
	return pMem;

} // _MusyxMalloc

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _MusyxFree( void *pData )
{
	fang_Free( pData );

} // _MusyxFree

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void *_MusyxSamplesLoadCallback( u32 uOffset, u32 uBytes )
{
	FASSERT_MSG( FAudio_bModuleInstalled,        "[ FAUDIO ] Error: System not installed !!!" );
#if FHEAP_TRACK_MEM_ALLOCATIONS
	FASSERT_MSG( _poTempWaveBankMramUsage, "[ FAUDIO ] Error: Invalid _poTempWaveBankMramUsage !!!" );
#endif

	////
	//
	if( FFILE_ERROR_RET == ffile_Seek( _ohMusyxSamplesFile, uOffset, FFILE_SEEK_SET ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: ffile_Seek() failed !!!\n", __LINE__ );
		_bMusyxSamplesLoadCallbackFailed = TRUE;
		return NULL;
	}

	if( uBytes != ffile_Read( _ohMusyxSamplesFile, uBytes, _pMusyxSamplesBuff ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: ffile_Read() failed !!!\n", __LINE__ );
		_bMusyxSamplesLoadCallbackFailed = TRUE;
		return NULL;
	}
	//
	////

#if FHEAP_TRACK_MEM_ALLOCATIONS
	_poTempWaveBankMramUsage->uBytes += uBytes;
#endif

	_bMusyxSamplesLoadCallbackFailed = FALSE;

	return _pMusyxSamplesBuff;

} // _MusyxSamplesLoadCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 _MusyxStreamCallback( void *pData1, u32 uSamples1, void *pData2, u32 uSamples2, u32 uContext )
{
	_Stream_t *poStream = &( _paoStreams[ uContext & 0xffff ] );

	if( poStream->bIsInUse )
	{
		return 0;
	}

	if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState )
	{
		u32 uChannel         = ( uContext >> 16 ),
			uBuffToConsume   = ( ( poStream->uCurrentReadBuff + 1 ) & 1 ),
			*puBuffAvailable = &( poStream->aauBuffReadyToConsume[ uBuffToConsume ][ uChannel ] ),
			uBuffBaseOffset  = ( poStream->aauBuffBaseOffset[ uBuffToConsume ][ uChannel ] ),
			uValidBuffSize   = ( poStream->aauBuffBytesRead[ uBuffToConsume ][ uChannel ] );

		////
		//
		u32 uSize = FMATH_MIN( *puBuffAvailable, _MUSYX_ADPCM_SAMPLES_TO_BYTES( uSamples1 ) );
		fang_MemCopy( pData1, &( poStream->aaauFileBuff[ uBuffToConsume ][ uBuffBaseOffset + uValidBuffSize - *puBuffAvailable ] ), uSize );
		*puBuffAvailable -= uSize;
		u32 uSize2 = 0;
//		if( pData2 && *puBuffAvailable) {
//			uSize2 = FMATH_MIN( *puBuffAvailable, _MUSYX_ADPCM_SAMPLES_TO_BYTES( uSamples2 ) );
//			fang_MemCopy( pData2, &( poStream->aaauFileBuff[ uBuffToConsume ][ uBuffBaseOffset + uValidBuffSize - *puBuffAvailable ] ), uSize2 );
//			*puBuffAvailable -= uSize2;
//		}

		if( _STREAM_FILE_CHANGE_NONE == poStream->uFileChanges )
		{
			if( 1 == poStream->uChannels )
			{
				if( 0 == poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] )
				{
					poStream->uFileChanges = _STREAM_FILE_CHANGE_READ_BODY;
				}
				else if( ( 0 == poStream->aauBuffReadyToConsume[ uBuffToConsume ][ 0 ] ) &&
						 ( poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] ) )
				{
					poStream->uCurrentReadBuff = ( ( poStream->uCurrentReadBuff + 1 ) & 1 );
				}
			}
			else
			{
				if( ( 0 == poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] ) &&
					( 0 == poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 1 ] ) )
				{
					poStream->uFileChanges = _STREAM_FILE_CHANGE_READ_BODY;
				}
				else if( ( 0 == poStream->aauBuffReadyToConsume[ uBuffToConsume ][ 0 ] ) &&
						 ( 0 == poStream->aauBuffReadyToConsume[ uBuffToConsume ][ 1 ] ) &&
						 ( poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] ) &&
						 ( poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 1 ] ) )
				{
					poStream->uCurrentReadBuff = ( ( poStream->uCurrentReadBuff + 1 ) & 1 );
				}
			}
		}

		if( _STREAM_FILE_CHANGE_PAD_WITH_SILENCE == poStream->uFileChanges && uSize == 0 && uSize2 == 0 ) {
			// We have reached the end of the stream, and we don't have any data to fill the stream with
			// so fill the buffer with silence...
			fang_MemZero( pData1, _MUSYX_ADPCM_SAMPLES_TO_BYTES( uSamples1 ) );
			if( uSamples2 )
			{
				fang_MemZero( pData2, _MUSYX_ADPCM_SAMPLES_TO_BYTES( uSamples2 ) );
			}
			return ( uSamples1 + uSamples2 );
		} else {
			return _MUSYX_BYTES_TO_ADPCM_SAMPLES( uSize + uSize2 );
		}
		//
		////
	} else {
		fang_MemZero( pData1, _MUSYX_ADPCM_SAMPLES_TO_BYTES( uSamples1 ) );

		if( uSamples2 )
		{
			fang_MemZero( pData2, _MUSYX_ADPCM_SAMPLES_TO_BYTES( uSamples2 ) );
		}

		return ( uSamples1 + uSamples2 );
	}

	return 0;

} // _MusyxStreamCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _FFileAsyncReadHeaderCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser )
{
	_Stream_t *poStream;

	u32 uIndex;

	for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		poStream = &( _paoStreams[ uIndex ] );

		if( ohFile == poStream->ohFile )
		{
			break;
		}
	}

	FASSERT_MSG( ( uIndex < _uMaxStreams ),      "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( ohFile == poStream->ohFile ), "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( poStream->bIsActive,            "[ FAUDIO ] Error: Invalid stream !!!" );

	if( FFILE_ERROR_RET == nErrorCode )
	{
		poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
		return;
	}

	FGCData_WvsFile_Header_t *poStreamHeader = (FGCData_WvsFile_Header_t *)poStream->aaauFileBuff;

	poStream->uChannels        = poStreamHeader->nNumChannels;
	poStream->fFrequencyInHz   = poStreamHeader->fFreqHz;
	poStream->fLengthInSeconds = poStreamHeader->fSecs;
	poStream->uCurrentReadBuff = 0;
	poStream->uFileChanges     = _STREAM_FILE_CHANGE_READ_BODY;
	poStream->uFileSize = sizeof( FGCData_WvsFile_Header_t ) + poStream->uChannels * poStreamHeader->nNumBytesPerChannel;
	poStream->uFilePos  = sizeof( FGCData_WvsFile_Header_t );

	FASSERT_MSG( ( sizeof( FGCData_WvsFile_Header_t ) == uBytesRead ),           "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( pDestBuffer == poStream->aaauFileBuff ),                      "[ FAUDIO ] Error: Invalid stream !!!" );

	if( ! ( FGCDATA_WVS_MAX_CHUNK_BYTES == poStreamHeader->nChunkBytes ) )
	{
		poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
		return;
	}

	if( 1 == poStream->uChannels )
	{
		sndStreamADPCMParameter( poStream->oMusyxStreamLeft, (SND_ADPCMSTREAM_INFO *)&( poStreamHeader->aChannelInfo[ 0 ] ) );
	}
	else
	{
		sndStreamADPCMParameter( poStream->oMusyxStreamLeft, (SND_ADPCMSTREAM_INFO *)&( poStreamHeader->aChannelInfo[ 0 ] ) );
		sndStreamADPCMParameter( poStream->oMusyxStreamRight, (SND_ADPCMSTREAM_INFO *)&( poStreamHeader->aChannelInfo[ 1 ] ) );
	}

} // _FFileAsyncReadHeaderCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _FFileAsyncReadBodyCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser )
{
	_Stream_t *poStream;

	u32 uIndex;

	for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		poStream = &( _paoStreams[ uIndex ] );

		if( ohFile == poStream->ohFile )
		{
			break;
		}
	}

	if ( uIndex >= _uMaxStreams )
	{
		DEVPRINTF( "[ FAUDIO ]	Error - Invalid stream !!!\n" );
		DEVPRINTF( "			Destination buffer:  %X\n", pDestBuffer );
		DEVPRINTF( "			Bytes Read        :  %d\n", uBytesRead );
		DEVPRINTF( "			User pointer      :  %X\n", pUser );
		DEVPRINTF( "			FFile Handle      :  %d\n", ohFile );
		DEVPRINTF( "			Discarding read results.\n" );
		return;
	}

	if ( !poStream->bIsActive )
	{
		DEVPRINTF( "[ FAUDIO ] Warning - Received data for an audio stream that is not or is no longer active.\n" );
		return;
	}
	
	if ( _STREAM_FILE_CHANGE_READ_BODY_PENDING != poStream->uFileChanges )
	{
		DEVPRINTF( "[ FAUDIO ] Error - Stream in improper state to receive data !!!" );
		return;
	}
	
	if ( pDestBuffer != poStream->aaauFileBuff[ poStream->uCurrentReadBuff ] )
	{
		DEVPRINTF( "[ FAUDIO ] Error - Destination buffer does not match stream buffer !!!" );
		return;
	}

//RAF -- commented this out because when the EOF is reached, this will generate an error
// Checking for for EOF condition through ffile_EOF doesn't seem to help.

//	if( FFILE_ERROR_RET == nErrorCode )
//	{
//		poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
//		return;
//	}
//	BOOL bTriggerEOF = FALSE;
	poStream->uFilePos += uBytesRead;
	if(poStream->uFilePos >= poStream->uFileSize ) {
		//just bail out and discard the oddball technology
		//for some reason, musyx does not seem to like sound samples that are not aligned, or a certain byte amount, so
		//i just skip the last one for now.  Yes, it's a hack, but it's next to impossible to tell, and it's better than
		//scratchy white noise!
		poStream->uFileChanges = _STREAM_FILE_CHANGE_EOF;
		return;

/*
		//we have read PAST the calculated size of the buffer, so fix it!
		uBytesRead -= (poStream->uFilePos - poStream->uFileSize );

		//first, copy the second channel from it's unaligned position into it's aligned position
		fang_MemCopy( &poStream->aaauFileBuff[ poStream->uCurrentReadBuff ][ FGCDATA_WVS_MAX_CHUNK_BYTES ], 
		              &poStream->aaauFileBuff[ poStream->uCurrentReadBuff ][ uBytesRead >> 1 ],
		              uBytesRead >> 1 );
		              
		//now, copy silence into the end of the left channel
		fang_MemZero( &poStream->aaauFileBuff[ poStream->uCurrentReadBuff ][ uBytesRead >> 1 ], FGCDATA_WVS_MAX_CHUNK_BYTES - (uBytesRead >> 1) );
		//now, copy silence into the end of the right channel
		fang_MemZero( &poStream->aaauFileBuff[ poStream->uCurrentReadBuff ][ FGCDATA_WVS_MAX_CHUNK_BYTES + (uBytesRead >> 1) ], FGCDATA_WVS_MAX_CHUNK_BYTES - (uBytesRead >> 1) );
		bTriggerEOF = TRUE;
		uBytesRead = FGCDATA_WVS_MAX_CHUNK_BYTES * 2;		
*/		
	}
	if( 1 == poStream->uChannels )
	{
		if( poStream->uBufferBytesFilled < _uStreamBuffSize /*_MUSYX_ADPCM_SAMPLES_TO_BYTES(_MUSYX_SAMPLES_BUFFERED)*/ )
		{
			// Buffer hasn't been initially filled yet, so immediately copy this chunk into the sample buffer, and
			// flag uFileChanges to keep reading.
			//first, figure out how much we should copy!
			
			u32 uSize = FMATH_MIN( uBytesRead, _uStreamBuffSize - poStream->uBufferBytesFilled );
			fang_MemCopy((_pauStreamBuff + ( ( ( uIndex * 2 ) + 0 ) * _uStreamBuffSize ))+poStream->uBufferBytesFilled, &( poStream->aaauFileBuff[ poStream->uCurrentReadBuff ][ 0 ] ), uSize );
			poStream->uBufferBytesFilled += uSize;
			
			if( poStream->uBufferBytesFilled == _uStreamBuffSize ) {
				//we have filled the buffer up... take the remaining amount and set the ready to consume value
				poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] = uBytesRead - uSize;
				//change the current read buffer so that the callback will use the rest of this buffer
				//and at the same time, command the new buffer to be filled
				poStream->uCurrentReadBuff = ( ( poStream->uCurrentReadBuff + 1 ) & 1 );
				if( poStream->oeState == FAUDIO_STREAM_STATE_PLAYING ) {
					sndStreamActivate( poStream->oMusyxStreamLeft );
				}
			}
			poStream->uFileChanges = ( ( ( FGCDATA_WVS_MAX_CHUNK_BYTES > uBytesRead )  ) ? _STREAM_FILE_CHANGE_EOF : ( poStream->uBufferBytesFilled < _MUSYX_ADPCM_SAMPLES_TO_BYTES(_MUSYX_SAMPLES_BUFFERED) ? _STREAM_FILE_CHANGE_READ_BODY : _STREAM_FILE_CHANGE_NONE) );
		}
		else
		{
			poStream->uFileChanges = ( ( ( FGCDATA_WVS_MAX_CHUNK_BYTES > uBytesRead )  ) ? _STREAM_FILE_CHANGE_EOF : _STREAM_FILE_CHANGE_NONE );
			poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] = uBytesRead;
		}
		poStream->aauBuffBytesRead[ poStream->uCurrentReadBuff ][ 0 ] = uBytesRead;
		poStream->aauBuffBaseOffset[ poStream->uCurrentReadBuff ][ 0 ] = 0;
	}
	else
	{
		if( poStream->uBufferBytesFilled < _uStreamBuffSize /*_MUSYX_ADPCM_SAMPLES_TO_BYTES(_MUSYX_SAMPLES_BUFFERED)*/ )
		{

			u32 uSize = FMATH_MIN( uBytesRead >> 1, _uStreamBuffSize - poStream->uBufferBytesFilled );
			fang_MemCopy((_pauStreamBuff + ( ( ( uIndex * 2 ) + 0 ) * _uStreamBuffSize ))+poStream->uBufferBytesFilled, &( poStream->aaauFileBuff[ poStream->uCurrentReadBuff ][ 0 ] ), uSize );
			fang_MemCopy((_pauStreamBuff + ( ( ( uIndex * 2 ) + 1 ) * _uStreamBuffSize ))+poStream->uBufferBytesFilled, &( poStream->aaauFileBuff[ poStream->uCurrentReadBuff ][ uBytesRead >> 1 ] ), uSize );
			poStream->uBufferBytesFilled += uSize;
			if( poStream->uBufferBytesFilled == _uStreamBuffSize ) {
				//we have filled the buffer up... take the remaining amount and set the ready to consume value
				poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] = ( uBytesRead >> 1 ) - uSize;
				poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 1 ] = ( uBytesRead >> 1 ) - uSize;
				//change the current read buffer so that the callback will use the rest of this buffer
				//and at the same time, command the new buffer to be filled
				poStream->uCurrentReadBuff = ( ( poStream->uCurrentReadBuff + 1 ) & 1 );
				if( poStream->oeState == FAUDIO_STREAM_STATE_PLAYING ) {
					sndStreamActivate( poStream->oMusyxStreamLeft );
					sndStreamActivate( poStream->oMusyxStreamRight );
				}
			}
			poStream->uFileChanges = ( ( ( (2 * FGCDATA_WVS_MAX_CHUNK_BYTES) > uBytesRead )  ) ? _STREAM_FILE_CHANGE_EOF : ( poStream->uBufferBytesFilled < _MUSYX_ADPCM_SAMPLES_TO_BYTES(_MUSYX_SAMPLES_BUFFERED) ? _STREAM_FILE_CHANGE_READ_BODY : _STREAM_FILE_CHANGE_NONE) );
		}
		else
		{
			poStream->uFileChanges = ( ( ( ( 2 * FGCDATA_WVS_MAX_CHUNK_BYTES ) > uBytesRead ) ) ? _STREAM_FILE_CHANGE_EOF : _STREAM_FILE_CHANGE_NONE );
			poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 0 ] = ( uBytesRead >> 1 );
			poStream->aauBuffReadyToConsume[ poStream->uCurrentReadBuff ][ 1 ] = ( uBytesRead >> 1 );
		}
		poStream->aauBuffBytesRead[ poStream->uCurrentReadBuff ][ 0 ] = uBytesRead >> 1;
		poStream->aauBuffBytesRead[ poStream->uCurrentReadBuff ][ 1 ] = uBytesRead >> 1;
		poStream->aauBuffBaseOffset[ poStream->uCurrentReadBuff ][ 0 ] = 0;
		poStream->aauBuffBaseOffset[ poStream->uCurrentReadBuff ][ 1 ] = uBytesRead >> 1;

	}

	if( FAUDIO_STREAM_STATE_CREATING == poStream->oeState )
	{
		poStream->bIgnoreInPauseMode              = FALSE;
		poStream->fVolume                         = 1.0f;
		poStream->fPanLeftRight                   = 0.0f;
		poStream->fFrequencyFactor                = 1.0f;
		poStream->pEndOfPlayCallback              = NULL;
		poStream->aauBuffReadyToConsume[ 1 ][ 0 ] = 0;
		poStream->aauBuffReadyToConsume[ 1 ][ 1 ] = 0;
		if( poStream->uBufferBytesFilled == _uStreamBuffSize ) {
			//it's time to transition this buffer from a creating state to a ready state!
			poStream->oeState                         = FAUDIO_STREAM_STATE_STOPPED;
			poStream->uStateChanges                   = ( _STREAM_STATE_CHANGE_VOLUME | _STREAM_STATE_CHANGE_PAN | _STREAM_STATE_CHANGE_FREQUENCY );
		}
	}
} // _FFileAsyncReadBodyCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CFAudioStream *CFAudioStream::Create( cchar *pszName, BOOL bWillBeUsedForMusic/*=TRUE*/ )
{
	if( ! FAudio_bModuleInstalled )
	{
		return NULL;
	}

	if( ! pszName )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioStream::Create() failed !!!\n", __LINE__ );
		return NULL;
	}

	if( fclib_strlen( pszName ) > FAUDIO_MAX_ASSET_NAME_LENGTH ) {
		DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioStream::Create() failed. Stream name '%s' is longer than %u characters.\n", __LINE__, pszName, FAUDIO_MAX_ASSET_NAME_LENGTH );
		return NULL;
	}

	_Stream_t *poStream;

	u32 uIndex;

	for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		poStream = &( _paoStreams[ uIndex ] );

		if( ! poStream->bIsActive )
		{
			break;
		}
	}

	if( uIndex < _uMaxStreams )
	{
		char szTemp[ FAUDIO_MAX_ASSET_NAME_LENGTH + 3 + 4 + 1 ];

		fclib_strcpy( szTemp, pszName );
		fclib_strcat( szTemp, ".wvs" );

		poStream->ohFile = ffile_Open( szTemp, FFILE_OPEN_RONLY, TRUE );
		if( FFILE_INVALID_HANDLE == poStream->ohFile )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: Could not open \"%s\" file !!!\n", __LINE__, szTemp );
			return NULL;
		}

		poStream->bTreatAsSfx	 = (bWillBeUsedForMusic) ? FALSE : TRUE;
		poStream->bIsActive      = TRUE;
		poStream->uStateChanges  = _STREAM_STATE_CHANGE_NONE;
		poStream->uFileChanges   = _STREAM_FILE_CHANGE_READ_HEADER;
		poStream->oeState        = FAUDIO_STREAM_STATE_CREATING;
		poStream->fSecondsPlayed = 0.0f;
		poStream->fSecondsToPlay = 0.0f;
		poStream->uBufferBytesFilled = 0;
		poStream->ePauseLevel 	 = FAUDIO_PAUSE_LEVEL_1; //default pause level
		
		//clear out the buffer for it's transfer to ARAM
		//This will invoke a period of silence
		fang_MemZero( &_pauStreamBuff[( uIndex * 2  + 0 ) * _uStreamBuffSize ], _uStreamBuffSize );
		fang_MemZero( &_pauStreamBuff[( uIndex * 2  + 1 ) * _uStreamBuffSize ], _uStreamBuffSize );
		
		return poStream->poAudioStream;
	}
	else
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioStream::Create() failed !!!\n", __LINE__ );
		return NULL;
	}

} // CFAudioStream::Create

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Destroy( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	_Stream_t *poStream = (_Stream_t *)m_uData;

	if( ! poStream->bIsActive )
	{
		return;
	}

	//Call CFAudioStream::Stop just in case
	Stop( FALSE ); //stop immediately
	
	poStream->bIsActive = FALSE;

	if( poStream->uStateChanges && _STREAM_STATE_CHANGE_STOP )
	{
		//invoke the callback and stop the streams
		if( poStream->pEndOfPlayCallback )
		{
			poStream->pEndOfPlayCallback( poStream->poAudioStream );
		}

		if( 1 == poStream->uChannels )
		{
			// Mono.
			sndStreamDeactivate( poStream->oMusyxStreamLeft );
		}
		else
		{
			// Stereo.
			sndStreamDeactivate( poStream->oMusyxStreamLeft );
			sndStreamDeactivate( poStream->oMusyxStreamRight );
		}
	}

	if ( ffile_Close( poStream->ohFile ) != 0 )
	{
		DEVPRINTF( "CFAudioStream::Destroy() - Error while closing audio stream.\n" );
	}
	
	poStream->ohFile = FFILE_INVALID_HANDLE;

} // CFAudioStream::Destroy

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::DestroyAll( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	_Stream_t *poStream;

	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		////
		//
		poStream = &( _paoStreams[ uIndex ] );

		if( ! poStream->bIsActive )
		{
			continue;
		}
		//
		////

		poStream->poAudioStream->Destroy();
	}

} // CFAudioStream::DestroyAll

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetVolume( f32 fVolume )
{
	FASSERT_MSG( FAudio_bModuleInstalled,              "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( FMATH_IS_UNIT_FLOAT( fVolume ), "[ FAUDIO ] Error: Invalid fVolume !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	if( ( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	if( fVolume != poStream->fVolume )
	{
		poStream->uStateChanges |= _STREAM_STATE_CHANGE_VOLUME;
		poStream->fVolume        = fVolume;
	}	
} // CFAudioStream::SetVolume

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetPan( f32 fPanLeftRight )
{
	FASSERT_MSG( FAudio_bModuleInstalled,                            "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( FMATH_IS_BIPOLAR_UNIT_FLOAT( fPanLeftRight ), "[ FAUDIO ] Error: Invalid fPanLeftRight !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	if( ( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	////
	//
	if( fPanLeftRight != poStream->fPanLeftRight )
	{
		poStream->uStateChanges |= _STREAM_STATE_CHANGE_PAN;
		poStream->fPanLeftRight  = fPanLeftRight;
	}
	//
	////

} // CFAudioStream::SetPan

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetEndOfPlayCallback( FAudio_StreamEndOfPlayCallback_t *pEndOfPlayCallback )
{
	FASSERT_MSG( FAudio_bModuleInstalled,  "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( pEndOfPlayCallback, "[ FAUDIO ] Error: NULL pointer !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	if( ( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	poStream->pEndOfPlayCallback = pEndOfPlayCallback;

} // CFAudioStream::SetEndOfPlayCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::SetFrequencyFactor( f32 fFrequencyFactor )
{
	FASSERT_MSG( FAudio_bModuleInstalled,            "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ( 0.0f <= fFrequencyFactor ), "[ FAUDIO ] Error: Invalid fFrequencyFactor !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	if( ( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	////
	//
	if( fFrequencyFactor != poStream->fFrequencyFactor )
	{
		poStream->uStateChanges    |= _STREAM_STATE_CHANGE_FREQUENCY;
		poStream->fFrequencyFactor  = fFrequencyFactor;
	}
	//
	////

} // CFAudioStream::SetFrequencyFactor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Play( u32 uLoops /* = 1 */ )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	if( ( FAUDIO_STREAM_STATE_PLAYING  == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	poStream->oeState        = FAUDIO_STREAM_STATE_PLAYING;
	poStream->uStateChanges &= ( ~ ( _STREAM_STATE_CHANGE_STOP | _STREAM_STATE_CHANGE_PAUSE | _STREAM_STATE_CHANGE_UNPAUSE ) );
	poStream->uStateChanges |= _STREAM_STATE_CHANGE_PLAY;

	poStream->fSecondsPlayed = 0.0f;

	if( uLoops )
	{
		poStream->bIsLooping     = ( 1 < uLoops );
		poStream->fSecondsToPlay = ( poStream->fLengthInSeconds * (f32)uLoops );
	}
	else
	{
		poStream->bIsLooping     = TRUE;
		poStream->fSecondsToPlay = FAUDIO_UNLIMITED_SECONDS;
	}

	_bSkipStreamsWorkDelay = FALSE;

} // CFAudioStream::Play

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Pause( BOOL bEnabled )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	if( ( FAUDIO_STREAM_STATE_STOPPED  == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	if( bEnabled )
	{
		if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState )
		{
			poStream->oeState        = FAUDIO_STREAM_STATE_PAUSED;
			poStream->uStateChanges &= ( ~ ( _STREAM_STATE_CHANGE_PLAY | _STREAM_STATE_CHANGE_UNPAUSE ) );
			poStream->uStateChanges |= _STREAM_STATE_CHANGE_PAUSE;
		}
	}
	else
	{
		if( FAUDIO_STREAM_STATE_PAUSED == poStream->oeState )
		{
			poStream->oeState        = FAUDIO_STREAM_STATE_PLAYING;
			poStream->uStateChanges &= ( ~ ( _STREAM_STATE_CHANGE_PLAY | _STREAM_STATE_CHANGE_PAUSE ) );
			poStream->uStateChanges |= _STREAM_STATE_CHANGE_UNPAUSE;
		}
	}

} // CFAudioStream::Pause

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CFAudioStream::Stop( BOOL bAfterCurrentLoop )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	if( ( FAUDIO_STREAM_STATE_STOPPED  == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) )
	{
		return;
	}

	poStream->bIsLooping = FALSE;

	if( ! bAfterCurrentLoop )
	{
		ffile_CancelFileIO( poStream->ohFile );
	
		poStream->oeState        = FAUDIO_STREAM_STATE_STOPPED;
		poStream->uStateChanges &= ( ~ ( _STREAM_STATE_CHANGE_PLAY | _STREAM_STATE_CHANGE_PAUSE | _STREAM_STATE_CHANGE_UNPAUSE ) );
		poStream->uStateChanges |= _STREAM_STATE_CHANGE_STOP;
	}
	else
	{
		poStream->fSecondsToPlay = ( ( poStream->fLengthInSeconds * (f32)( (u32)( poStream->fSecondsPlayed / poStream->fLengthInSeconds ) ) ) + poStream->fLengthInSeconds );
	}

	_bSkipStreamsWorkDelay = FALSE;

} // CFAudioStream::Stop

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_StreamState_e CFAudioStream::GetState( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	return (FAudio_StreamState_e)poStream->oeState;

} // CFAudioStream::GetState

CFAudioStream *CFAudioStream::GetStream( u32 uStreamIdx )
{
	if( uStreamIdx < _uMaxStreams )
	{
		return _paoStreams[ uStreamIdx ].poAudioStream;
	}
	return NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

f32 CFAudioStream::GetSecondsPlayed( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	return poStream->fSecondsPlayed;

} // CFAudioStream::GetSecondsPlayed

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

f32 CFAudioStream::GetSecondsToPlay( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	return poStream->fSecondsToPlay;

} // CFAudioStream::GetSecondsToPlay

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_PauseLevel_e CFAudioStream::SetGlobalPauseLevel( FAudio_PauseLevel_e eNewPauseLevel )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_PAUSE_LEVEL_NONE;
	}

	FAudio_PauseLevel_e eOldPauseLevel = _ePauseLevelStreams;

	_Stream_t *poStream;


	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		////
		//
		poStream = &( _paoStreams[ uIndex ] );

		if( ! poStream->bIsActive )
		{
			continue;
		}
		//
		////

		//check to see if we should pause this stream...
		if( poStream->ePauseLevel <= eNewPauseLevel ) { 
			//we need to pause this stream!
			poStream->poAudioStream->Pause( TRUE );
			poStream->bIgnoreInPauseMode = TRUE;
		} else if ( poStream->bIgnoreInPauseMode ) {
			//we need to unpause this stream!
			poStream->poAudioStream->Pause( FALSE );
			poStream->bIgnoreInPauseMode = FALSE;
		}
	}
	_ePauseLevelStreams = eNewPauseLevel;	

	return eOldPauseLevel;
		
} // CFAudioStream::PauseMode

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FAudio_PauseLevel_e CFAudioStream::GetGlobalPauseLevel( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return FAUDIO_PAUSE_LEVEL_NONE;
	}

	return ( _ePauseLevelStreams );

} // CFAudioStream::IsPauseModeActive

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

u32 CFAudioStream::GetNumActive( void )
{
	if( ! FAudio_bModuleInstalled )
	{
		return 0;
	}

	_Stream_t *poStream;
	u32 uCount = 0;

	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		poStream = &( _paoStreams[ uIndex ] );

		if( poStream->bIsActive )
		{
			++uCount;
		}
	}

	return uCount;

} // CFAudioStream::GetNumActive

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

cchar *CFAudioStream::GetName( void )
{
	FASSERT_MSG( FAudio_bModuleInstalled, "[ FAUDIO ] Error: System not installed !!!" );

	////
	//
	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	//
	////

	return poStream->szName;

} // CFAudioStream::GetName

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

cchar *CFAudioStream::GetName( u32 uIndex )
{
	if( ! FAudio_bModuleInstalled )
	{
		return NULL;
	}

	_Stream_t *poStream;
	u32 uTemp, uCount = 0;

	for( uTemp = 0; uTemp < _uMaxStreams; ++uTemp )
	{
		poStream = &( _paoStreams[ uTemp ] );

		if( poStream->bIsActive )
		{
			if( uCount == uIndex )
			{
				break;
			}
			else
			{
				++uCount;
			}
		}
	}

	FASSERT_MSG( ( uTemp < _uMaxStreams ), "[ FAUDIO ] Error: Invalid stream !!!" );

	return poStream->szName;

} // CFAudioStream::GetName

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