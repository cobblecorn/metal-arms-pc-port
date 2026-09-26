//////////////////////////////////////////////////////////////////////////////////////
// fxbaudio.cpp - 
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
// 04/16/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fdx8.h"
#include "floop.h"
#include "faudio.h"
#include "fresload.h"
#include "fclib.h"
#include "ffile.h"
#include "floop.h"
#include "fdx8data.h"
#include "fexception.h"

#include <xtl.h>
#include <dsound.h>
#include <dsstdfx.h>

#define _EMITTERS_LISTENERS_WORK_DELAY				( 0.06666667f ) // 15 fps.

#define _STREAMS_WORK_DELAY							( 0.00f ) // full fps.

#define _UNIQUE_VOLUME_LEVELS						( 128 )
#define _UNIQUE_FLOAT_VOL_LEVEL_INDICES				( (f32)( _UNIQUE_VOLUME_LEVELS - 1 ) )

#define _DSOUND_REAL_EMITTERS						( 256 ) // 4 + 64 + 188 = 256.
#define _DSOUND_REAL_EMITTERS_RESERVED_INTERNALLY	( 4 )
#define _DSOUND_REAL_EMITTERS_MAX_3D				( 64 )
#define _DSOUND_REAL_EMITTERS_MAX_2D				( 188 )

// Does not include bank headers, but does include wave headers and wave data.
#define _MAX_TOTAL_WAVE_DATA_SIZE					( 2047 * ( 4 * 1024 ) )

#define _MAX_TOTAL_STREAMS							( 4 )

#define _SIGNIFICANT_DISTANCE_CHANGE				( 0.001f )
#define _SIGNIFICANT_DISTANCE_CHANGE_SQ				( _SIGNIFICANT_DISTANCE_CHANGE * _SIGNIFICANT_DISTANCE_CHANGE )

#define _SIGNIFICANT_DOT_CHANGE						( 1.0f - 0.00001f ) //+- 2.562 degrees
#define _SIGNIFICANT_DOT_CHANGE_SQ					( _SIGNIFICANT_DOT_CHANGE * _SIGNIFICANT_DOT_CHANGE )

#define _SIGNIFICANT_FACTOR_CHANGE					( 0.0005f )
#define _SIGNIFICANT_FACTOR_CHANGE_SQ				( _SIGNIFICANT_FACTOR_CHANGE * _SIGNIFICANT_FACTOR_CHANGE )

#define _SIGNIFICANT_VOLUME_CHANGE					( 1.0f / 127.0f )
#define _SIGNIFICANT_VOLUME_CHANGE_SQ				( _SIGNIFICANT_VOLUME_CHANGE * _SIGNIFICANT_VOLUME_CHANGE )

#define _EMITTERS_CREATE_MAX_ERRORS					( 5 )

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
	_EMITTER_STATE_CHANGE_NONE				= 0x00000000,
	_EMITTER_STATE_CHANGE_PLAY				= 0x00000001,
	_EMITTER_STATE_CHANGE_STOP				= 0x00000002,
	_EMITTER_STATE_CHANGE_PAUSE				= 0x00000004,
	_EMITTER_STATE_CHANGE_UNPAUSE			= 0x00000008,
	_EMITTER_STATE_CHANGE_VOLUME			= 0x00000010,
	_EMITTER_STATE_CHANGE_FREQUENCY			= 0x00000020,
	_EMITTER_STATE_CHANGE_POSITION			= 0x00000040,
	_EMITTER_STATE_CHANGE_PAN				= 0x00000080,
//	_EMITTER_STATE_CHANGE_RADIUS			= 0x00000100,
	_EMITTER_STATE_CHANGE_DOPPLER			= 0x00000200,
	_EMITTER_STATE_CHANGE_REVERB			= 0x00000400,

	// Consumed by CFAudioEmitter::SetVolume().
	_EMITTER_STATE_CHANGE_DUCKABLE			= 0x00000800,
	_EMITTER_STATE_CHANGE_DUCKING			= 0x00001000

}; // _EmitterStateChange_e

enum _StreamStateChange_e
{
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

enum _StreamFileChange_e
{
	// Consumed by faudio_Work().
	_STREAM_FILE_CHANGE_NONE				= 0x00000000,
	_STREAM_FILE_CHANGE_READ_HEADER			= 0x00000001,
	_STREAM_FILE_CHANGE_READ_BODY			= 0x00000002,
	_STREAM_FILE_CHANGE_READ_BODY_PENDING	= 0x00000003,
	_STREAM_FILE_CHANGE_EOF					= 0x00000004

}; // _StreamStateChange_e

enum _ListenerStateChange_e
{
	_LISTENER_STATE_CHANGE_NONE				= 0x00000000,
	_LISTENER_STATE_CHANGE_ORIENTATION		= 0x00000001

}; // _ListenerStateChange_e

enum _ListenerIntersection_e
{
	_LISTENER_INTERSECTION_NONE				= 0x00000000,
	_LISTENER_INTERSECTION_ENTERED			= 0x00000001,
	_LISTENER_INTERSECTION_EXITED			= 0x00000002,
	_LISTENER_INTERSECTION_SWITCHED			= 0x00000004,
	_LISTENER_INTERSECTION_PRESENT			= 0x00000008

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
			/*fRadiusInner,*/
			fVolume, fVolumeDucked,
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

struct _Stream_t
{
	CFAudioStream *poAudioStream;

	FFileHandle ohFile;

	LPDIRECTSOUNDSTREAM poDSStream;

	f32	fVolume;
	f32 fPanLeftRight;
	f32 fFrequencyInHz;
	f32 fFrequencyFactor;
	f32 fLengthInSeconds;
	f32 fSecondsPlayed;
	f32 fSecondsToPlay;

	BOOL8 bIsActive;
	BOOL8 bIsLooping;
	BOOL8 bIgnoreInPauseMode;
	BOOL8 bTreatAsSfx;	// use the sfx volume multiplier instead of the music

	u32 uCurrentReadBuff;
	u32 uFileSize;
	u32 uFilePos;
	u32 uStateChanges;	// See _StreamStateChange_e.
	u32 uFileChanges;	// See _StreamFileChange_e.

	HANDLE	ohMutex;	// FFile.

	char	szName[ FDATA_AUDIO_FILENAME_LEN ];

	//// Double buffered.
	//
	u8		*apauFileBuff[ 2 ];
	u32		auReadyToConsume[ 2 ];
	DWORD	auStatus[ 2 ];
	XMEDIAPACKET oMediaPacket[ 2 ];
	//
	////

	FAudio_StreamState_e oeState;
	FAudio_PauseLevel_e ePauseLevel;
	
	CFAudioStream::FAudio_StreamEndOfPlayCallback_t *pEndOfPlayCallback;
}; // _Stream_t


// global vars
u8 FAudio_EmitterDefaultPriorityLevel = 0;
BOOL8 FAudio_bModuleStarted = FALSE;
BOOL8 FAudio_bModuleInstalled = FALSE;
BOOL8 FAudio_bMasterSfxVolChanged = FALSE;
BOOL8 FAudio_bMasterMusicVolChanged = FALSE;
f32 FAudio_fMasterSfxUnitVol = 1.0f;
f32 FAudio_fMasterMusicUnitVol = 1.0f;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Wave banks.
static u32 _uSoundBytes, _uMaxSoundBytes, _uMaxBanks;
static FLinkRoot_t _oWaveBanksList;
static FResLoadReg_t _oLoadReg;

// Real Listeners.
static LPDIRECTSOUND8 _poDSRealListener;

// Virtual Listeners.
static u32 _uMaxVirtualListeners, _uActiveVirtualListeners;
static _VirtualListener_t _aoVirtualListeners[ FAUDIO_MAX_LISTENERS ];

// Real Emitters.
static u32 _uMaxRealEmitters2D;
static _RealEmitter_t *_paoRealEmitters2D;
static FLinkRoot_t _oRealEmittersListFree2D;
static FLinkRoot_t _oRealEmittersListActive2D;

static _RealEmitter_t _aoRealEmitters3D[ _DSOUND_REAL_EMITTERS_MAX_3D ];
static FLinkRoot_t _oRealEmittersListFree3D;
static FLinkRoot_t _oRealEmittersListActive3D;

static DS3DBUFFER _oDSEmitterAttributes;
static DSI3DL2BUFFER _oDSEmitterReverbParam;
static DSMIXBINVOLUMEPAIR _aoDSMixbinsVolumepair[ 2 ];
static DSMIXBINS _oDSMixbins = { ( sizeof( _aoDSMixbinsVolumepair ) / sizeof( DSMIXBINVOLUMEPAIR ) ), _aoDSMixbinsVolumepair };

static _RealEmitterLimit_t *_paoRealEmittersLimits; // Pointer to an array of _uMaxPriorityLevels elements.

// New rolloff curve
static f32 _afRolloffCurve[] =	{ 
	1.0f,
	0.9f,
	0.8f,
	0.7f,
	0.5f,
	0.35f,
	0.3f,
	0.25f,
	0.075f,
	0.05f,
	0.0f
};

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
static BOOL _bSkipEmittersListenersWorkDelay=FALSE;

static f32 _fStreamsWorkDelay; // (use fangdef instead?) Should be replaced by a scheduler eventually to avoid running in synch with other low-fps work()!!!!!!!!!!
static BOOL _bSkipStreamsWorkDelay;

static s32 _anVolumes[ _UNIQUE_VOLUME_LEVELS ]; // Optimization.
static u32 _uTimeStamp;

// Streams.
static u32 _uMaxStreams;
static _Stream_t *_paoStreams;

static FAudio_PauseLevel_e _ePauseLevelStreams;

// Temp.
static CFVec3A _oTempVec3A, _oTempVec3A_Velocity;
static CFSphere _oTempSphere;
static _VirtualListener_t *_poTempVirtualListener;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _FFileAsyncReadHeaderCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser );
static void _FFileAsyncReadBodyCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser );
static BOOL _BankLoadCallback( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _BankUnloadCallback( void *pResMem );
static BOOL _WorldLoadAndUnloadCallback( FWorldEvent_e oeEvent );
static void _AutoDestroyEmitterEndOfPlayCallback( CFAudioEmitter *poAudioEmitter );
static void _AutoDestroyStreamEndOfPlayCallback( CFAudioStream *poAudioStream );
static BOOL _EmitterIntersectionCallback( CFWorldTracker *poWorldTracker, FVisVolume_t *pVolume );
static void _TrackEmittersProgress( FLinkRoot_t *poVirtualEmittersListActive );
static void _TrackStreamsProgress( void );
static void _ApplyRealEmittersChanges( FLinkRoot_t *poVirtualEmittersListActive );
static void _InvokeEmittersEndofplayCallbacks( FLinkRoot_t *poVirtualEmittersListActive );
static void _DestroyAllEmittersFromABank( FAudio_BankHandle_t hBank );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static FINLINE void _SetPlayBufferAndRegion( LPDIRECTSOUNDBUFFER8 pDSBuffer, FDataWvbFile_Bank_t *pBank, FDataWvbFile_Wave_t *pWave ) {
	FASSERT( pDSBuffer );
	FASSERT( pBank );
	FASSERT( pWave );
	u32 uPadLen;

	uPadLen = pWave->uLength / 72;
	uPadLen *= 72;

	if( uPadLen != pWave->uLength ) {
		// we need to pad
		uPadLen = pWave->uLength - uPadLen;
		uPadLen = 72 - uPadLen;
	} else {
		uPadLen = 0;
	}

	pDSBuffer->SetFormat( (WAVEFORMATEX *)( (u32)pBank->pData + ( pWave->uIndex * sizeof( XBOXADPCMWAVEFORMAT ) ) ) );
	pDSBuffer->SetBufferData( (void *) ( (u32) pBank->pData + pWave->uOffset + ( pBank->uWaves * sizeof( XBOXADPCMWAVEFORMAT ) ) ), pWave->uLength + uPadLen  );
	pDSBuffer->SetPlayRegion( 0, pWave->uLength );
}	

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
	FASSERT_MSG( FAudio_bModuleStarted,                                                                            "[ FAUDIO ] Error: System not started !!!" );
	FASSERT_MSG( ( ! FAudio_bModuleInstalled ),                                                                          "[ FAUDIO ] Error: System already installed !!!" );
	FASSERT_MSG( poInit,                                                                                           "[ FAUDIO ] Error: NULL pointer !!!" );
	FASSERT_MSG( ( poInit->uMaxListeners && ( FAUDIO_MAX_LISTENERS >= poInit->uMaxListeners ) ),                   "[ FAUDIO ] Error: Invalid uMaxListeners !!!" );
	FASSERT_MSG( poInit->uMaxEmitters,                                                                             "[ FAUDIO ] Error: Invalid uMaxEmitters !!!" );
	FASSERT_MSG( poInit->uMaxStreams,                                                                              "[ FAUDIO ] Error: Invalid uMaxStreams !!!" );
	FASSERT_MSG( Fang_ConfigDefs.nAudio_MaxSoundBytes,                                                                           "[ FAUDIO ] Error: Invalid uMaxSoundBytes !!!" );
	FASSERT_MSG( poInit->uMaxPriorityLevels,                                                                       "[ FAUDIO ] Error: Invalid uMaxPriorityLevels !!!" );
	FASSERT_MSG( poInit->uMaxBanks,                                                                                "[ FAUDIO ] Error: Invalid uMaxBanks !!!" );
	FASSERT_MSG( ( _DSOUND_REAL_EMITTERS_MAX_2D >= ( poInit->uMaxBanks + 2 ) ),                                    "[ FAUDIO ] Error: Invalid uMaxBanks !!!" );
	FASSERT_MSG( ( _DSOUND_REAL_EMITTERS_MAX_2D >= ( poInit->uMaxStreams + 2 ) ),                                  "[ FAUDIO ] Error: Invalid uMaxStreams !!!" );
	FASSERT_MSG( ( _DSOUND_REAL_EMITTERS_MAX_2D >= ( poInit->uMaxBanks + poInit->uMaxStreams + 1 ) ),              "[ FAUDIO ] Error: Invalid uMaxBanks and uMaxStreams combination !!!" );
	FASSERT_MSG( ( poInit->uPriorityLimits ? ( !! poInit->paoPriorityLimits ) : ( ! poInit->paoPriorityLimits ) ), "[ FAUDIO ] Error: Invalid priority limits !!!" );

	if( FAILED( DirectSoundCreate( NULL, &( _poDSRealListener ), NULL ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: DirectSoundCreate() failed !!!\n", __LINE__ );
		return FAUDIO_ERROR;
	}

	DirectSoundUseFullHRTF();
	_poDSRealListener->SetDistanceFactor( FMATH_FEET2METERS( 1.0f ), DS3D_DEFERRED );

	//// Register .Wvb loader.
	if( ! fresload_IsRegistered( FAUDIOBANK_RESTYPE ) )
	{
		fang_MemZero( &_oLoadReg, sizeof( _oLoadReg ) );
		fres_CopyType( _oLoadReg.sResType, FAUDIOBANK_RESTYPE );
		_oLoadReg.pszFileExtension = "wvb";
		_oLoadReg.nMemType         = FRESLOAD_MEMTYPE_PERM;
		_oLoadReg.nAlignment       = 16;
		_oLoadReg.pFcnCreate       = _BankLoadCallback;
		_oLoadReg.pFcnDestroy      = _BankUnloadCallback;

		if( ! fresload_RegisterHandler( &_oLoadReg ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: fresload_RegisterHandler() failed !!!\n", __LINE__ );
			FDX8_SAFE_RELEASE( _poDSRealListener );
			return FAUDIO_ERROR;
		}
	}

	//// Init static values.
	_ePauseLevelEmitters              = FAUDIO_PAUSE_LEVEL_NONE;
	_ePauseLevelStreams               = FAUDIO_PAUSE_LEVEL_NONE;
	_fDuckingFactor                  = 1.0f;

	_fEmittersListenersWorkDelay     = 0.0f;
	_fOOEmittersListenersWorkDelay   = 0.0f;
	_bSkipEmittersListenersWorkDelay = TRUE;

	_fStreamsWorkDelay               = 0.0f;
	_bSkipStreamsWorkDelay           = TRUE;

	_uTimeStamp                      = 10;
	_uSoundBytes                     = 0;
	_uEmittersCreateErrors2D         = _EMITTERS_CREATE_MAX_ERRORS;
	_uEmittersCreateErrors3D         = _EMITTERS_CREATE_MAX_ERRORS;
	_uMaxVirtualListeners            = poInit->uMaxListeners;
	_uMaxVirtualEmitters             = poInit->uMaxEmitters;
	_uMaxPriorityLevels              = poInit->uMaxPriorityLevels;
	_uMaxSoundBytes                  = Fang_ConfigDefs.nAudio_MaxSoundBytes;
	_uMaxBanks                       = poInit->uMaxBanks;
	_uMaxRealEmitters2D              = _DSOUND_REAL_EMITTERS_MAX_2D - poInit->uMaxBanks - poInit->uMaxStreams;
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

	//// Set default DS emitter reverb parameters.
	fang_MemZero( &_oDSEmitterReverbParam, sizeof( _oDSEmitterReverbParam ) );
	_oDSEmitterReverbParam.lDirect               = DSI3DL2BUFFER_MAXDIRECT;
	_oDSEmitterReverbParam.lDirectHF             = DSI3DL2BUFFER_MAXDIRECTHF;
	_oDSEmitterReverbParam.lRoom                 = DSI3DL2BUFFER_MINROOM;
	_oDSEmitterReverbParam.lRoomHF               = DSI3DL2BUFFER_MINROOMHF;
	_oDSEmitterReverbParam.flRoomRolloffFactor   = DSI3DL2BUFFER_DEFAULTROOMROLLOFFFACTOR;
	_oDSEmitterReverbParam.Obstruction.lHFLevel  = DSI3DL2BUFFER_DEFAULTOBSTRUCTION;
	_oDSEmitterReverbParam.Obstruction.flLFRatio = DSI3DL2BUFFER_DEFAULTOBSTRUCTIONLFRATIO;
	_oDSEmitterReverbParam.Occlusion.lHFLevel    = DSI3DL2BUFFER_DEFAULTOCCLUSION;
	_oDSEmitterReverbParam.Occlusion.flLFRatio   = DSI3DL2BUFFER_DEFAULTOCCLUSIONLFRATIO;

	//// Set default DS emitter attributes.
	fang_MemZero( &_oDSEmitterAttributes, sizeof( _oDSEmitterAttributes ) );
	_oDSEmitterAttributes.dwSize             = sizeof( DS3DBUFFER );
	_oDSEmitterAttributes.vPosition.x        = DS3D_DEFAULTPOSITION_X;
	_oDSEmitterAttributes.vPosition.y        = DS3D_DEFAULTPOSITION_Y;
	_oDSEmitterAttributes.vPosition.z        = DS3D_DEFAULTPOSITION_Z;
	_oDSEmitterAttributes.vVelocity.x        = DS3D_DEFAULTVELOCITY_X;
	_oDSEmitterAttributes.vVelocity.y        = DS3D_DEFAULTVELOCITY_Y;
	_oDSEmitterAttributes.vVelocity.z        = DS3D_DEFAULTVELOCITY_Z;
	_oDSEmitterAttributes.dwInsideConeAngle  = DS3D_DEFAULTCONEANGLE;
	_oDSEmitterAttributes.dwOutsideConeAngle = DS3D_DEFAULTCONEANGLE;
	_oDSEmitterAttributes.vConeOrientation.x = DS3D_DEFAULTCONEORIENT_X;
	_oDSEmitterAttributes.vConeOrientation.y = DS3D_DEFAULTCONEORIENT_Y;
	_oDSEmitterAttributes.vConeOrientation.z = DS3D_DEFAULTCONEORIENT_Z;
	_oDSEmitterAttributes.lConeOutsideVolume = DS3D_DEFAULTCONEOUTSIDEVOLUME;
	_oDSEmitterAttributes.flMinDistance      = FAUDIO_MIN_RADIUS; // DS3D_DEFAULTMINDISTANCE!!!!!!!!!!!!!!!!!?????????????
	_oDSEmitterAttributes.flMaxDistance      = DS3D_DEFAULTMAXDISTANCE;
	_oDSEmitterAttributes.dwMode             = DS3DMODE_HEADRELATIVE; // DS3DMODE_NORMAL???????? NO!!!!!!!!!!!!!
	_oDSEmitterAttributes.flDistanceFactor   = FMATH_FEET2METERS( 1.0f );
	_oDSEmitterAttributes.flRolloffFactor    = DS3D_DEFAULTROLLOFFFACTOR;
	_oDSEmitterAttributes.flDopplerFactor    = DS3D_DEFAULTDOPPLERFACTOR;

	//// Init arrays and lists.
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

	u32 uIndex;

	//// Send default DSP effects to MCP.
	FMemFrame_t oMemFrame = fmem_GetFrame();

	FFileHandle ohFile = ffile_Open( "dsp_sfx.rdx", FFILE_OPEN_RONLY );
	if( FFILE_INVALID_HANDLE == ohFile )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Could not open \"dsp_sfx.rdx\" file !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDSRealListener );
		return FAUDIO_ERROR;
	}

	uIndex = ffile_GetFileSize( ohFile );
	if( ! uIndex )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: \"dsp_sfx.rdx\" file invalid !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDSRealListener );
		ffile_Close( ohFile );
		return FAUDIO_ERROR;
	}

	void *pDspData = fmem_Alloc( uIndex );
	if( ! pDspData )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fmem_Alloc() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDSRealListener );
		ffile_Close( ohFile );
		return FAUDIO_ERROR;
	}

	if( uIndex != ffile_Read( ohFile, uIndex, pDspData ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: ffile_Read() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDSRealListener );
		ffile_Close( ohFile );
		fmem_ReleaseFrame( oMemFrame );
		return FAUDIO_ERROR;
	}

	ffile_Close( ohFile );

	DSEFFECTIMAGELOC oEffectImageLoc;
	oEffectImageLoc.dwI3DL2ReverbIndex = I3DL2_CHAIN_I3DL2_REVERB;
	oEffectImageLoc.dwCrosstalkIndex   = I3DL2_CHAIN_XTALK;

	_poDSRealListener->DownloadEffectsImage( pDspData, uIndex, &oEffectImageLoc, NULL );

	fmem_ReleaseFrame( oMemFrame );

	FResFrame_t oResFrame = fres_GetFrame();

	//// Allocate memory.
	_paoVirtualEmitters                = (_VirtualEmitter_t *)fres_AllocAndZero( sizeof( _VirtualEmitter_t ) * _uMaxVirtualEmitters );
	_paoVirtualEmittersListActive2D    = (FLinkRoot_t *)fres_AllocAndZero( sizeof( FLinkRoot_t ) * _uMaxPriorityLevels );
	_paoVirtualEmittersListActive3D    = (FLinkRoot_t *)fres_AllocAndZero( sizeof( FLinkRoot_t ) * _uMaxPriorityLevels );
	_paoRealEmittersLimits             = (_RealEmitterLimit_t *)fres_AllocAndZero( sizeof( _RealEmitterLimit_t ) * _uMaxPriorityLevels );
	_paoRealEmitters2D                 = (_RealEmitter_t *)fres_AllocAndZero( sizeof( _RealEmitter_t ) * _uMaxRealEmitters2D );
	_paoStreams                        = (_Stream_t *)fres_AllocAndZero( sizeof( _Stream_t ) * _uMaxStreams );
	_paoStreams[ 0 ].apauFileBuff[ 0 ] = (u8 *)XPhysicalAlloc( ( FDX8DATA_WVS_MAX_CHUNK_BYTES * 2 * _uMaxStreams ), MAXULONG_PTR, 0, ( PAGE_READWRITE | PAGE_NOCACHE ) );
	if( ! ( _paoVirtualEmitters &&
			_paoVirtualEmittersListActive2D &&
			_paoVirtualEmittersListActive3D &&
			_paoRealEmittersLimits &&
			_paoRealEmitters2D &&
			_paoStreams &&
			_paoStreams[ 0 ].apauFileBuff[ 0 ] ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: fres_AllocAndZero() failed !!!\n", __LINE__ );
		FDX8_SAFE_RELEASE( _poDSRealListener );
		XPhysicalFree( _paoStreams[ 0 ].apauFileBuff[ 0 ] );
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}

	for( uIndex = 0; uIndex < _uMaxPriorityLevels; ++uIndex )
	{
		flinklist_InitRoot( &( _paoVirtualEmittersListActive2D[ uIndex ] ), (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );
		flinklist_InitRoot( &( _paoVirtualEmittersListActive3D[ uIndex ] ), (s32)FANG_OFFSETOF( _VirtualEmitter_t, oLink ) );

		_paoRealEmittersLimits[ uIndex ].uPlayable = FAUDIO_UNLIMITED_EMITTERS;
	}

	//// Init priority limits.
	if( poInit->paoPriorityLimits )
	{
		FAudio_PriorityLimit_t *poPriorityLimit;

		for( uIndex = 0; uIndex < poInit->uPriorityLimits; ++uIndex )
		{
			poPriorityLimit = &( poInit->paoPriorityLimits[ uIndex ] );

			FASSERT_MSG( ( _uMaxPriorityLevels > poPriorityLimit->uPriorityLevel ), "[ FAUDIO ] Error: Invalid uPriorityLevel !!!" );

			_paoRealEmittersLimits[ poPriorityLimit->uPriorityLevel ].uPlayable = poPriorityLimit->uMaxPlayable;
		}
	}

	_VirtualListener_t *poVirtualListener = &( _aoVirtualListeners[ 0 ] );
	_VirtualEmitter_t *poVirtualEmitter   = &( _paoVirtualEmitters[ 0 ] );
	_Stream_t *poStream                   = &( _paoStreams[ 0 ] );

	//// Allocate memory.
	poVirtualListener->poWorldUser                = (CFWorldUser *)fnew CFWorldUser[ _uMaxVirtualListeners ];
	poVirtualListener->poXfmCurrentOrientation_WS = (CFXfm *)fnew CFXfm[ _uMaxVirtualListeners ];
	poVirtualListener->poVecPreviousPosition_WS   = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualListeners ];
	poVirtualListener->poVecVelocity_WS           = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualListeners ];

	poVirtualEmitter->poAudioEmitter              = (CFAudioEmitter *)fnew CFAudioEmitter[ _uMaxVirtualEmitters ];
	poVirtualEmitter->poVecCurrentPosition_WS     = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualEmitters ];
	poVirtualEmitter->poVecPreviousPosition_WS    = (CFVec3A *)fnew CFVec3A[ _uMaxVirtualEmitters ];

	poStream->poAudioStream                       = (CFAudioStream *)fnew CFAudioStream[ _uMaxStreams ];

	if( ! (	poVirtualListener->poWorldUser &&
			poVirtualListener->poXfmCurrentOrientation_WS &&
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
		fdelete_array( poVirtualListener->poWorldUser );

		FDX8_SAFE_RELEASE( _poDSRealListener );
		XPhysicalFree( _paoStreams[ 0 ].apauFileBuff[ 0 ] );
		fres_ReleaseFrame( oResFrame );
		return FAUDIO_ERROR;
	}

	//// Init 1st listener.
	_uActiveVirtualListeners = 1;
	poVirtualListener->poXfmCurrentOrientation_WS->Identity();
	poVirtualListener->poVecPreviousPosition_WS->Zero();
	poVirtualListener->poVecVelocity_WS->Zero();
	poVirtualListener->poWorldUser->m_nUser = FWORLD_USERTYPE_AUDIO_LISTENER;
	poVirtualListener->poWorldUser->m_pUser = poVirtualListener;

	//// Fix-up.
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

	flinklist_AddTail( &_oVirtualEmittersListFree, poVirtualEmitter );

	for( uIndex = 1; uIndex < _uMaxVirtualEmitters; ++uIndex )
	{
		poVirtualEmitter                            = &( _paoVirtualEmitters[ uIndex ] );
		poVirtualEmitter->poAudioEmitter            = ( _paoVirtualEmitters[ 0 ].poAudioEmitter + uIndex );
		poVirtualEmitter->poVecCurrentPosition_WS   = ( _paoVirtualEmitters[ 0 ].poVecCurrentPosition_WS + uIndex );
		poVirtualEmitter->poVecPreviousPosition_WS  = ( _paoVirtualEmitters[ 0 ].poVecPreviousPosition_WS + uIndex );

		//// Init virtual emitter.
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

		flinklist_AddTail( &_oVirtualEmittersListFree, poVirtualEmitter );
	}

	//// Create DS emitters.
	XBOXADPCMWAVEFORMAT oWaveFormat;
	DSBUFFERDESC oBufferDescription;

	fang_MemZero( &oWaveFormat, sizeof( oWaveFormat ) );
	fang_MemZero( &oBufferDescription, sizeof( oBufferDescription ) );

	//// Dummy values.
	XAudioCreateAdpcmFormat( 1, 44100, &oWaveFormat );

	//// Mixbins.
	_aoDSMixbinsVolumepair[ 0 ].dwMixBin = DSMIXBIN_FRONT_LEFT;
	_aoDSMixbinsVolumepair[ 0 ].lVolume  = DSBVOLUME_MAX;
	_aoDSMixbinsVolumepair[ 1 ].dwMixBin = DSMIXBIN_FRONT_RIGHT;
	_aoDSMixbinsVolumepair[ 1 ].lVolume  = DSBVOLUME_MAX;

	//// DS emitter description.
	oBufferDescription.dwSize      = sizeof( oBufferDescription );
	oBufferDescription.lpwfxFormat = (LPWAVEFORMATEX)&oWaveFormat;
	oBufferDescription.dwFlags     = ( DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME );
	oBufferDescription.lpMixBins   = &_oDSMixbins;

	_RealEmitter_t *poRealEmitter;

	//// 2D.
	for( uIndex = 0; uIndex < _uMaxRealEmitters2D; ++uIndex )
	{
		poRealEmitter = &( _paoRealEmitters2D[ uIndex ] );

		if( DS_OK != DirectSoundCreateBuffer( &oBufferDescription, &( poRealEmitter->poDSBuffer ) ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: DirectSoundCreateBuffer() failed !!!\n", __LINE__ );

			// 2D.
			for( u32 uTemp = 0; uTemp < uIndex; ++uTemp )
			{
				FDX8_SAFE_RELEASE( _paoRealEmitters2D[ uTemp ].poDSBuffer );
			}

			fdelete_array( _paoStreams[ 0 ].poAudioStream );

			poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
			fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
			fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
			fdelete_array( poVirtualEmitter->poAudioEmitter );

			poVirtualListener = &( _aoVirtualListeners[ 0 ] );
			fdelete_array( poVirtualListener->poVecVelocity_WS );
			fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
			fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );
			fdelete_array( poVirtualListener->poWorldUser );

			FDX8_SAFE_RELEASE( _poDSRealListener );
			XPhysicalFree( _paoStreams[ 0 ].apauFileBuff[ 0 ] );
			fres_ReleaseFrame( oResFrame );

			return FAUDIO_ERROR;
		}

//create then set all defaults for sound buffer!!

		flinklist_AddTail( &_oRealEmittersListFree2D, poRealEmitter );
	}

	//// 3D.
	// 5.1 surround with reverb.
	DSMIXBINVOLUMEPAIR aoDSMixbinsVolumepair[] = { DSMIXBINVOLUMEPAIRS_DEFAULT_5CHANNEL_3D_PLUS_LFE };

	DSMIXBINS oDSMixbins = { ( sizeof( aoDSMixbinsVolumepair ) / sizeof( DSMIXBINVOLUMEPAIR ) ), aoDSMixbinsVolumepair };

	oBufferDescription.lpMixBins  = &oDSMixbins;
	oBufferDescription.dwFlags   |= DSBCAPS_CTRL3D;

	for( uIndex = 0; uIndex < _DSOUND_REAL_EMITTERS_MAX_3D; ++uIndex )
	{
		poRealEmitter = &( _aoRealEmitters3D[ uIndex ] );

		if( DS_OK != DirectSoundCreateBuffer( &oBufferDescription, &( poRealEmitter->poDSBuffer ) ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: DirectSoundCreateBuffer() failed !!!\n", __LINE__ );

			u32 uTemp;

			// 2D.
			for( uTemp = 0; uTemp < _uMaxRealEmitters2D; ++uTemp )
			{
				FDX8_SAFE_RELEASE( _paoRealEmitters2D[ uTemp ].poDSBuffer );
			}

			// 3D.
			for( uTemp = 0; uTemp < uIndex; ++uTemp )
			{
				FDX8_SAFE_RELEASE( _aoRealEmitters3D[ uTemp ].poDSBuffer );
			}

			fdelete_array( _paoStreams[ 0 ].poAudioStream );

			poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
			fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
			fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
			fdelete_array( poVirtualEmitter->poAudioEmitter );

			poVirtualListener = &( _aoVirtualListeners[ 0 ] );
			fdelete_array( poVirtualListener->poVecVelocity_WS );
			fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
			fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );
			fdelete_array( poVirtualListener->poWorldUser );

			FDX8_SAFE_RELEASE( _poDSRealListener );
			XPhysicalFree( _paoStreams[ 0 ].apauFileBuff[ 0 ] );
			fres_ReleaseFrame( oResFrame );

			return FAUDIO_ERROR;
		}

		////
		//
		poRealEmitter->poDSBuffer->SetI3DL2Source( &_oDSEmitterReverbParam, DS3D_DEFERRED );
		poRealEmitter->poDSBuffer->SetRolloffCurve( _afRolloffCurve, ( sizeof( _afRolloffCurve ) / sizeof( f32 ) ), DS3D_DEFERRED );
		poRealEmitter->poDSBuffer->SetAllParameters( &_oDSEmitterAttributes, DS3D_DEFERRED );
		//
		////

		flinklist_AddTail( &_oRealEmittersListFree3D, poRealEmitter );
	}

	//// Init streams.
	DSSTREAMDESC oStreamDescription;
	fang_MemZero( &oStreamDescription, sizeof( oStreamDescription ) );

	oStreamDescription.dwMaxAttachedPackets = 2;
	oStreamDescription.lpwfxFormat          = (LPWAVEFORMATEX)&oWaveFormat;
	oStreamDescription.lpMixBins            = &_oDSMixbins;

	poStream->poAudioStream->m_uData        = (u32)poStream;
	poStream->bIsActive                     = FALSE;
	poStream->ohFile                        = FFILE_INVALID_HANDLE;
	poStream->apauFileBuff[ 0 ]             = ( _paoStreams[ 0 ].apauFileBuff[ 0 ] + ( ( ( 0 * 2 ) + 0 ) * FDX8DATA_WVS_MAX_CHUNK_BYTES ) );
	poStream->apauFileBuff[ 1 ]             = ( _paoStreams[ 0 ].apauFileBuff[ 0 ] + ( ( ( 0 * 2 ) + 1 ) * FDX8DATA_WVS_MAX_CHUNK_BYTES ) );
	poStream->oMediaPacket[ 0 ].pvBuffer    = poStream->apauFileBuff[ 0 ];
	poStream->oMediaPacket[ 1 ].pvBuffer    = poStream->apauFileBuff[ 1 ];
	poStream->oMediaPacket[ 0 ].dwMaxSize   = FDX8DATA_WVS_MAX_CHUNK_BYTES;
	poStream->oMediaPacket[ 1 ].dwMaxSize   = FDX8DATA_WVS_MAX_CHUNK_BYTES;
	poStream->oMediaPacket[ 0 ].pdwStatus   = &( poStream->auStatus[ 0 ] );
	poStream->oMediaPacket[ 1 ].pdwStatus   = &( poStream->auStatus[ 1 ] );

	poStream->ohMutex = CreateMutex( NULL, FALSE, NULL );

	if( ( ! poStream->ohMutex ) || ( DS_OK != DirectSoundCreateStream( &oStreamDescription, &( poStream->poDSStream ) ) ) )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Stream initialization failed !!!\n", __LINE__ );

		u32 uTemp;

		// 2D.
		for( uTemp = 0; uTemp < _uMaxRealEmitters2D; ++uTemp )
		{
			FDX8_SAFE_RELEASE( _paoRealEmitters2D[ uTemp ].poDSBuffer );
		}

		// 3D.
		for( uTemp = 0; uTemp < _DSOUND_REAL_EMITTERS_MAX_3D; ++uTemp )
		{
			FDX8_SAFE_RELEASE( _aoRealEmitters3D[ uTemp ].poDSBuffer );
		}

		fdelete_array( _paoStreams[ 0 ].poAudioStream );

		poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
		fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
		fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
		fdelete_array( poVirtualEmitter->poAudioEmitter );

		poVirtualListener = &( _aoVirtualListeners[ 0 ] );
		fdelete_array( poVirtualListener->poVecVelocity_WS );
		fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
		fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );
		fdelete_array( poVirtualListener->poWorldUser );

		FDX8_SAFE_RELEASE( _poDSRealListener );
		XPhysicalFree( _paoStreams[ 0 ].apauFileBuff[ 0 ] );
		fres_ReleaseFrame( oResFrame );

		return FAUDIO_ERROR;
	}

	for( uIndex = 1; uIndex < _uMaxStreams; ++uIndex )
	{
		poStream                                = &( _paoStreams[ uIndex ] );
		poStream->poAudioStream                 = ( _paoStreams[ 0 ].poAudioStream + uIndex );

		poStream->poAudioStream->m_uData        = (u32)poStream;
		poStream->bIsActive                     = FALSE;
		poStream->ohFile                        = FFILE_INVALID_HANDLE;
		poStream->apauFileBuff[ 0 ]             = ( _paoStreams[ 0 ].apauFileBuff[ 0 ] + ( ( ( uIndex * 2 ) + 0 ) * FDX8DATA_WVS_MAX_CHUNK_BYTES ) );
		poStream->apauFileBuff[ 1 ]             = ( _paoStreams[ 0 ].apauFileBuff[ 0 ] + ( ( ( uIndex * 2 ) + 1 ) * FDX8DATA_WVS_MAX_CHUNK_BYTES ) );
		poStream->oMediaPacket[ 0 ].pvBuffer    = poStream->apauFileBuff[ 0 ];
		poStream->oMediaPacket[ 1 ].pvBuffer    = poStream->apauFileBuff[ 1 ];
		poStream->oMediaPacket[ 0 ].dwMaxSize   = FDX8DATA_WVS_MAX_CHUNK_BYTES;
		poStream->oMediaPacket[ 1 ].dwMaxSize   = FDX8DATA_WVS_MAX_CHUNK_BYTES;
		poStream->oMediaPacket[ 0 ].pdwStatus   = &( poStream->auStatus[ 0 ] );
		poStream->oMediaPacket[ 1 ].pdwStatus   = &( poStream->auStatus[ 1 ] );

		poStream->ohMutex = CreateMutex( NULL, FALSE, NULL );

		if( ( ! poStream->ohMutex ) || ( DS_OK != DirectSoundCreateStream( &oStreamDescription, &( poStream->poDSStream ) ) ) )
		{
			DEVPRINTF( "[ FAUDIO ] Error %u: Stream initialization failed !!!\n", __LINE__ );

			u32 uTemp;

			// 2D.
			for( uTemp = 0; uTemp < _uMaxRealEmitters2D; ++uTemp )
			{
				FDX8_SAFE_RELEASE( _paoRealEmitters2D[ uTemp ].poDSBuffer );
			}

			// 3D.
			for( uTemp = 0; uTemp < _DSOUND_REAL_EMITTERS_MAX_3D; ++uTemp )
			{
				FDX8_SAFE_RELEASE( _aoRealEmitters3D[ uTemp ].poDSBuffer );
			}

			fdelete_array( _paoStreams[ 0 ].poAudioStream );

			poVirtualEmitter = &( _paoVirtualEmitters[ 0 ] );
			fdelete_array( poVirtualEmitter->poVecPreviousPosition_WS );
			fdelete_array( poVirtualEmitter->poVecCurrentPosition_WS );
			fdelete_array( poVirtualEmitter->poAudioEmitter );

			poVirtualListener = &( _aoVirtualListeners[ 0 ] );
			fdelete_array( poVirtualListener->poVecVelocity_WS );
			fdelete_array( poVirtualListener->poVecPreviousPosition_WS );
			fdelete_array( poVirtualListener->poXfmCurrentOrientation_WS );
			fdelete_array( poVirtualListener->poWorldUser );

			FDX8_SAFE_RELEASE( _poDSRealListener );
			XPhysicalFree( _paoStreams[ 0 ].apauFileBuff[ 0 ] );
			fres_ReleaseFrame( oResFrame );

			return FAUDIO_ERROR;
		}
	}

	//// Apply all deffered changes.
	_poDSRealListener->CommitDeferredSettings();

	//// Volume look-up table.
	double dTempVolume;
	for( uIndex = 1; uIndex < _UNIQUE_VOLUME_LEVELS; ++uIndex )
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

	// removed zero index from loop above as it was causing floating point divide by zero exception. -cjm
	_anVolumes[ 0 ] = DSBVOLUME_MIN;

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

	u32 uIndex;

	//// Stop and release DS emitters.
	//
	// 2D.
	for( uIndex = 0; uIndex < _uMaxRealEmitters2D; ++uIndex )
	{
		FDX8_SAFE_RELEASE( _paoRealEmitters2D[ uIndex ].poDSBuffer );
	}

	// 3D.
	for( uIndex = 0; uIndex < _DSOUND_REAL_EMITTERS_MAX_3D; ++uIndex )
	{
		FDX8_SAFE_RELEASE( _aoRealEmitters3D[ uIndex ].poDSBuffer );
	}
	//
	////

	//// Stop and release DS streams.
	//
	for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
	{
		FDX8_SAFE_RELEASE( _paoStreams[ uIndex ].poDSStream );
		CloseHandle( _paoStreams[ uIndex ].ohMutex );
	}
	//
	////

	DirectSoundDoWork();

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
	fdelete_array( poVirtualListener->poWorldUser );
	//
	////

	////
	//
	if( fworld_IsWorldCallbackFunctionRegistered( _WorldLoadAndUnloadCallback ) )
	{
		fworld_UnregisterWorldCallbackFunction( _WorldLoadAndUnloadCallback );
	}
	//
	////

	FDX8_SAFE_RELEASE( _poDSRealListener );
	XPhysicalFree( _paoStreams[ 0 ].apauFileBuff[ 0 ] );

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

	////
	//
	if( ( sizeof( FDataWvbFile_Bank_t ) + sizeof( FDataWvbFile_Wave_t ) + sizeof( XBOXADPCMWAVEFORMAT ) + 1 ) > nLoadedBytes )
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
	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)pLoadedBase;
	FDataWvbFile_Wave_t *poWave = (FDataWvbFile_Wave_t *)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) );

	u32 uFormatDataSize = ( sizeof( XBOXADPCMWAVEFORMAT ) * poBank->uWaves );
	u32 uWaveDataSize = ( poBank->uDataLength - uFormatDataSize );
	//
	////

	////
	//
	if( ( uWaveDataSize + _uSoundBytes ) > _uMaxSoundBytes )
	{
		DEVPRINTF( "[ FAUDIO ] Error %u: Bank %s is too large for available audio memory !!!\n", __LINE__, pszResName );
		return FALSE;
	}
	//
	////

	////
	//
	poBank->pData                     = (void *)( (u32)poBank + sizeof( FDataWvbFile_Bank_t ) + ( poBank->uWaves * sizeof( FDataWvbFile_Wave_t ) ) );
	XBOXADPCMWAVEFORMAT *poWaveFormat = (XBOXADPCMWAVEFORMAT *)( poBank->pData );

	for( u32 uIndex = 0; uIndex < poBank->uWaves; ++uIndex, ++poWave, ++poWaveFormat )
	{
		poWave->oBankHandle = (FAudio_BankHandle_t)poBank;
		FASSERT_MSG( ( poWaveFormat->wfx.wFormatTag == WAVE_FORMAT_XBOX_ADPCM ),   "[ FAUDIO ] Error: Invalid wave !!!" );
		FASSERT_MSG( ( poWave->nNumChannels == poWaveFormat->wfx.nChannels ),      "[ FAUDIO ] Error: Invalid wave !!!" );
		FASSERT_MSG( ( (u32)poWave->fFreqHz == poWaveFormat->wfx.nSamplesPerSec ), "[ FAUDIO ] Error: Invalid wave !!!" );
	}

	_uSoundBytes += uWaveDataSize;
	flinklist_AddTail( &_oWaveBanksList, poBank );
	fres_SetBase( hRes, poBank );
	//
	////

	// !!Nate
	//DEVPRINTF( "[ FAUDIO ] Bank Load %15s - Bank Size %d - Sounds Loaded %d\n", pszResName, uWaveDataSize, _uSoundBytes );

	return TRUE;

} // _BankLoadCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _BankUnloadCallback( void *pResMem )
{
	FASSERT_MSG( pResMem, "[ FAUDIO ] Error: NULL pointer !!!" );

	if( !FAudio_bModuleInstalled ) {
		return;
	}

	////
	//
	FDataWvbFile_Bank_t *poBank = (FDataWvbFile_Bank_t *)pResMem;

	_uSoundBytes -= ( poBank->uDataLength - ( sizeof( XBOXADPCMWAVEFORMAT ) * poBank->uWaves ) );

	// kill all sounds that are currently using this bank
	_DestroyAllEmittersFromABank( (FAudio_BankHandle_t)poBank );

	flinklist_RemoveTail( &_oWaveBanksList );
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

				if( ( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_LOOPING ) || ( poVirtualEmitter->fSecondsToPlay - poVirtualEmitter->fSecondsPlayed ) > 1.0f ) {
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
					_oTempSphere.m_fRadius = FAUDIO_MIN_RADIUS;
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
						poVirtualEmitter->poRealEmitter->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
						// !!Nate
						//poVirtualEmitter->poRealEmitter->poDSBuffer->SetBufferData( NULL, 0 );

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
						poVirtualEmitter->poRealEmitter->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
						// !!Nate
						//poVirtualEmitter->poRealEmitter->poDSBuffer->SetBufferData( NULL, 0 );

						//commented out by RAF -- we are only pausing it.. we will restart it later. 
//						flinklist_Remove( &_oRealEmittersListActive2D, poVirtualEmitter->poRealEmitter );
//						flinklist_AddTail( &_oRealEmittersListFree2D, poVirtualEmitter->poRealEmitter );
//						poVirtualEmitter->poRealEmitter = NULL;
//						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

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
						poVirtualEmitter->poRealEmitter->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
						// !!Nate
						//poVirtualEmitter->poRealEmitter->poDSBuffer->SetBufferData( NULL, 0 );

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
						poVirtualEmitter->poRealEmitter->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
						// !!Nate
						//poVirtualEmitter->poRealEmitter->poDSBuffer->SetBufferData( NULL, 0 );

						//commented out by RAF -- we are only pausing it.. we will restart it later. 
//						flinklist_Remove( &_oRealEmittersListActive3D, poVirtualEmitter->poRealEmitter );
//						flinklist_AddTail( &_oRealEmittersListFree3D, poVirtualEmitter->poRealEmitter );
//						poVirtualEmitter->poRealEmitter = NULL;
//						--( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

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
						poVirtualEmitter->poRealEmitter->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
						// !!Nate
						//poVirtualEmitter->poRealEmitter->poDSBuffer->SetBufferData( NULL, 0 );

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
		FDataWvbFile_Bank_t *poBank;
		FDataWvbFile_Wave_t *poWave;
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
							poVirtualEmitter->poRealEmitter  = poRealEmitter;
							poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_PAN );
							++( _paoRealEmittersLimits[ uPriority ].uPlaying );

							poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
							poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);

							FASSERT( poWave->uIndex < poBank->uWaves );
							FASSERT( poWave->uLength );
							FASSERT( (poWave->uOffset + poWave->uLength + ( poBank->uWaves * sizeof( XBOXADPCMWAVEFORMAT ) )) <= poBank->uDataLength );

							/*poRealEmitter->poDSBuffer->SetFormat( (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( XBOXADPCMWAVEFORMAT ) ) ) );
							poRealEmitter->poDSBuffer->SetBufferData( (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( XBOXADPCMWAVEFORMAT ) ) ), 
																	  ( poBank->uDataLength - ( sizeof( XBOXADPCMWAVEFORMAT ) * poBank->uWaves ) ) );
							poRealEmitter->poDSBuffer->SetPlayRegion( poWave->uOffset, poWave->uLength );*/
							_SetPlayBufferAndRegion( poRealEmitter->poDSBuffer, poBank, poWave );
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
							poRealEmitterCull->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
							// !!Nate
							//poRealEmitterCull->poDSBuffer->SetBufferData( NULL, 0 );

							poVirtualEmitterCull->poRealEmitter = NULL;
							--( _paoRealEmittersLimits[ poVirtualEmitterCull->uPriority ].uPlaying );

							poVirtualEmitter->poRealEmitter  = poRealEmitterCull;
							poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_PAN );
							++( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

							poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
							poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);
							/*poRealEmitterCull->poDSBuffer->SetFormat( (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( XBOXADPCMWAVEFORMAT ) ) ) );
							poRealEmitterCull->poDSBuffer->SetBufferData( (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( XBOXADPCMWAVEFORMAT ) ) ), ( poBank->uDataLength - ( sizeof( XBOXADPCMWAVEFORMAT ) * poBank->uWaves ) ) );
							poRealEmitterCull->poDSBuffer->SetPlayRegion( poWave->uOffset, poWave->uLength );*/
							_SetPlayBufferAndRegion( poRealEmitterCull->poDSBuffer, poBank, poWave );
						}
					}
				}

				poVirtualEmitter = (_VirtualEmitter_t *)flinklist_GetNext( poVirtualEmittersList, poVirtualEmitter );
			}
		}

		if( FWorld_pWorld )
		{
			// 3D.
			for( nList = ( (s32)_uMaxPriorityLevels - 1 ); nList >= 0; nList-- ) {
				poVirtualEmittersList = &_paoVirtualEmittersListActive3D[ nList ];
				poVirtualEmitter      = (_VirtualEmitter_t *)flinklist_GetHead( poVirtualEmittersList );

				// walk the virtual emitter list for this priority
				while( poVirtualEmitter ) {

					if( ( _LISTENER_INTERSECTION_ENTERED == poVirtualEmitter->oeListenerIntersection ) &&
						( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState ) &&
						( poVirtualEmitter->poRealEmitter == NULL ) )
					{
						uPriority = poVirtualEmitter->uPriority;

						if( _oRealEmittersListFree3D.nCount )
						{
							if( _paoRealEmittersLimits[ uPriority ].uPlaying < _paoRealEmittersLimits[ uPriority ].uPlayable )
							{
								poRealEmitter = (_RealEmitter_t *)flinklist_RemoveHead( &_oRealEmittersListFree3D );
								FASSERT( poRealEmitter );
								flinklist_AddTail( &_oRealEmittersListActive3D, poRealEmitter );
								poVirtualEmitter->poRealEmitter  = poRealEmitter;
//								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_RADIUS | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								++_paoRealEmittersLimits[ uPriority ].uPlaying;

								poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
								poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);

								FASSERT( poWave->uIndex < poBank->uWaves );
								FASSERT( poWave->uLength );
								//FASSERT( (poWave->uOffset + poWave->uLength + ( poBank->uWaves * sizeof( XBOXADPCMWAVEFORMAT ) )) < poBank->uDataLength );

								/*poRealEmitter->poDSBuffer->SetFormat( (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( XBOXADPCMWAVEFORMAT ) ) ) );
								poRealEmitter->poDSBuffer->SetBufferData( (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( XBOXADPCMWAVEFORMAT ) ) ),
																		  ( poBank->uDataLength - ( sizeof( XBOXADPCMWAVEFORMAT ) * poBank->uWaves ) ) );
								poRealEmitter->poDSBuffer->SetPlayRegion( poWave->uOffset, poWave->uLength );*/
								_SetPlayBufferAndRegion( poRealEmitter->poDSBuffer, poBank, poWave );
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
								poRealEmitterCull->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
								// !!Nate
								//poRealEmitterCull->poDSBuffer->SetBufferData( NULL, 0 );

								poVirtualEmitterCull->poRealEmitter = NULL;
								--( _paoRealEmittersLimits[ poVirtualEmitterCull->uPriority ].uPlaying );

								poVirtualEmitter->poRealEmitter  = poRealEmitterCull;
//								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_RADIUS | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_FREQUENCY | _EMITTER_STATE_CHANGE_POSITION | _EMITTER_STATE_CHANGE_DOPPLER | _EMITTER_STATE_CHANGE_REVERB );
								++( _paoRealEmittersLimits[ poVirtualEmitter->uPriority ].uPlaying );

								poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
								poBank = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);
								/*poRealEmitterCull->poDSBuffer->SetFormat( (WAVEFORMATEX *)( (u32)poBank->pData + ( poWave->uIndex * sizeof( XBOXADPCMWAVEFORMAT ) ) ) );
								poRealEmitterCull->poDSBuffer->SetBufferData( (void *)( (u32)poBank->pData + ( poBank->uWaves * sizeof( XBOXADPCMWAVEFORMAT ) ) ), ( poBank->uDataLength - ( sizeof( XBOXADPCMWAVEFORMAT ) * poBank->uWaves ) ) );
								poRealEmitterCull->poDSBuffer->SetPlayRegion( poWave->uOffset, poWave->uLength );*/
								_SetPlayBufferAndRegion( poRealEmitterCull->poDSBuffer, poBank, poWave );
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
	

	// Streams work.
	_fStreamsWorkDelay += FLoop_fRealPreviousLoopSecs;

	if( ( ! _bSkipStreamsWorkDelay ) || ( _STREAMS_WORK_DELAY < _fStreamsWorkDelay ) )
	{
		_Stream_t *poStream;
		LPDIRECTSOUNDSTREAM pDSStream;
		f32 fVolume;

		for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex )
		{
			poStream = &_paoStreams[ uIndex ];

			if( ( ! poStream->bIsActive ) || ( FAUDIO_STREAM_STATE_ERROR == poStream->oeState ) ) {
				continue;
			}

			pDSStream = poStream->poDSStream;
			
			WaitForSingleObject( poStream->ohMutex, INFINITE );

			// see if the master volume has changed and the volume needs to be adjusted
			if( poStream->bTreatAsSfx ) {
				if( FAudio_bMasterSfxVolChanged ) {
					// flag the stream to have the volume changed
					poStream->uStateChanges |= _STREAM_STATE_CHANGE_VOLUME;
				}
			} else if( FAudio_bMasterMusicVolChanged ) {
                // flag the stream to have the volume changed
				poStream->uStateChanges |= _STREAM_STATE_CHANGE_VOLUME;
			}
			
			if( poStream->uStateChanges ) {
				// Volume.
				if( poStream->uStateChanges & _STREAM_STATE_CHANGE_VOLUME ) {
					fVolume = ( poStream->bTreatAsSfx ) ? FAudio_fMasterSfxUnitVol : FAudio_fMasterMusicUnitVol;
					fVolume *= poStream->fVolume;
					pDSStream->SetVolume( _anVolumes[ fmath_FloatToU32( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * fVolume ) ] );
				}
				
				// Pan.
				if( _STREAM_STATE_CHANGE_PAN & poStream->uStateChanges ) {
					if( 0.0f <= poStream->fPanLeftRight ) {
						_aoDSMixbinsVolumepair[ 0 ].lVolume = _anVolumes[ fmath_FloatToU32( ( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * 0.5f * ( 1.0f - poStream->fPanLeftRight ) ) ) ];
						_aoDSMixbinsVolumepair[ 1 ].lVolume = DSBVOLUME_MAX;
					} else {
						_aoDSMixbinsVolumepair[ 0 ].lVolume = DSBVOLUME_MAX;
						_aoDSMixbinsVolumepair[ 1 ].lVolume = _anVolumes[ fmath_FloatToU32( ( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * 0.5f * ( 1.0f + poStream->fPanLeftRight ) ) ) ];
					}
					
					pDSStream->SetMixBinVolumes( &_oDSMixbins );
				}

				// Frequency.
				if( _STREAM_STATE_CHANGE_FREQUENCY & poStream->uStateChanges ) {
					u32 uNewFreq = fmath_FloatToU32( poStream->fFrequencyFactor * poStream->fFrequencyInHz );
					FMATH_CLAMP( uNewFreq, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX );
					pDSStream->SetFrequency( uNewFreq );
				}
				
				if( _STREAM_STATE_CHANGE_UNPAUSE & poStream->uStateChanges ) {
					//resume the stream
					pDSStream->Pause( DSSTREAMPAUSE_RESUME );
				}
				if( _STREAM_STATE_CHANGE_PAUSE & poStream->uStateChanges ) {
					//pause the stream.
					pDSStream->Pause( DSSTREAMPAUSE_PAUSE );
				}
			}

			if( poStream->uFileChanges )
			{
				if( _STREAM_FILE_CHANGE_READ_HEADER == poStream->uFileChanges )
				{
					poStream->uFileChanges = _STREAM_FILE_CHANGE_NONE;
					if( FFILE_ERROR_RET == ffile_Read( poStream->ohFile, sizeof( FDX8Data_WvsFile_Header_t ), poStream->apauFileBuff[ 0 ], _FFileAsyncReadHeaderCallback ) )
					{
						poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
					}
					poStream->uFileSize = (u32)ffile_GetFileSize( poStream->ohFile );
					poStream->uFilePos  = sizeof( FDX8Data_WvsFile_Header_t );
				}
				else if( _STREAM_FILE_CHANGE_READ_BODY == poStream->uFileChanges )
				{
					poStream->uFileChanges = _STREAM_FILE_CHANGE_READ_BODY_PENDING;

					if( FFILE_ERROR_RET == ffile_Read( poStream->ohFile, FDX8DATA_WVS_MAX_CHUNK_BYTES, poStream->apauFileBuff[ poStream->uCurrentReadBuff ], _FFileAsyncReadBodyCallback ) )
					{
						poStream->oeState = FAUDIO_STREAM_STATE_ERROR;
					}
				}
				else if( _STREAM_FILE_CHANGE_EOF == poStream->uFileChanges )
				{
					poStream->uFileChanges = _STREAM_FILE_CHANGE_NONE;
					if( FFILE_ERROR_RET == ffile_Seek( poStream->ohFile, sizeof( FDX8Data_WvsFile_Header_t ), FFILE_SEEK_SET ) )
					{
						poStream->uFileChanges = _STREAM_FILE_CHANGE_NONE;
						poStream->oeState      = FAUDIO_STREAM_STATE_ERROR;
					} else {
						// We are at the start again plus the offset
						poStream->uFilePos = sizeof( FDX8Data_WvsFile_Header_t );
					}
				}
			}
			
            if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState )
			{
				u32 uBuffToConsume = ( ( poStream->uCurrentReadBuff + 1 ) & 1 );

				if( XMEDIAPACKET_STATUS_PENDING != poStream->auStatus[ uBuffToConsume ] )
				{
					// Packet is not waiting to be played in DSStream's queue.
					u32 *puBuffAvailable = &( poStream->auReadyToConsume[ uBuffToConsume ] );

					if( *puBuffAvailable )
					{
						// Data available to be submitted to DSStream's play-queue.
						if( FDX8DATA_WVS_MAX_CHUNK_BYTES <= *puBuffAvailable )
						{
							poStream->oMediaPacket[ uBuffToConsume ].dwMaxSize  = FDX8DATA_WVS_MAX_CHUNK_BYTES;
							*puBuffAvailable                                   -= FDX8DATA_WVS_MAX_CHUNK_BYTES;

							// Submit packet DSStream's play-queue.
							pDSStream->Process( &( poStream->oMediaPacket[ uBuffToConsume ] ), NULL );
						}
						else
						{
							poStream->oMediaPacket[ uBuffToConsume ].dwMaxSize = *puBuffAvailable;
							*puBuffAvailable                                   = 0;

							// Submit packet DSStream's play-queue.
							pDSStream->Process( &( poStream->oMediaPacket[ uBuffToConsume ] ), NULL );
							pDSStream->Discontinuity();
						}
					}
				}

				if( _STREAM_FILE_CHANGE_NONE == poStream->uFileChanges )
				{
					// No read operations pending.
					if( 0 == poStream->auReadyToConsume[ poStream->uCurrentReadBuff ] ) {
						// No data available in next buffer: Queue next read.
						
						// NKM - Only queue a read if the packet that corresponds to the buffer that will
						//		 be read into is finished being processed.
						if( XMEDIAPACKET_STATUS_PENDING != poStream->auStatus[ poStream->uCurrentReadBuff ] ) {
							poStream->uFileChanges = _STREAM_FILE_CHANGE_READ_BODY;
						}
					} else if( ( 0 == poStream->auReadyToConsume[ uBuffToConsume ] ) &&
							   ( poStream->auReadyToConsume[ poStream->uCurrentReadBuff ] ) ) {
						// No data available in current buffer, and data available in next buffer: Switch buffers.
						poStream->uCurrentReadBuff = ( ( poStream->uCurrentReadBuff + 1 ) & 1 );
					}
				}
			}
			
			ReleaseMutex( poStream->ohMutex );			
		}

		DirectSoundDoWork();
		ffile_CheckFileReadStatus( NULL, TRUE );

		// Invoke end-of-play callbacks.
		for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex ) {
			poStream = &_paoStreams[ uIndex ];

			if( ! poStream->bIsActive ) {
				continue;
			}

			if( _STREAM_STATE_CHANGE_STOP & poStream->uStateChanges ) {
				if( poStream->pEndOfPlayCallback ) {
					poStream->pEndOfPlayCallback( poStream->poAudioStream );
				}
			}
			poStream->uStateChanges = _STREAM_STATE_CHANGE_NONE;
		}

		_bSkipStreamsWorkDelay = TRUE;

		_fStreamsWorkDelay = 0.0f;
	}

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

void faudio_UsingHeadphones_XB( BOOL bHeadphones )
{
	if( ! FAudio_bModuleInstalled )
	{
		return;
	}

	_poDSRealListener->EnableHeadphones( bHeadphones );

} // faudio_UsingHeadphones_XB

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
//			DEVPRINTF( "faudio_LoadBank: loading bank %s, approximate size %d\n", pszName, fresload_GetObjectSizeOnDisk( FAUDIOBANK_RESTYPE, szLocalizedFilename ) );
			oBankHandle = (FAudio_BankHandle_t)fresload_Load( FAUDIOBANK_RESTYPE, szLocalizedFilename );
		}
	}

	if( !oBankHandle )
	{
//		DEVPRINTF( "faudio_LoadBank: loading bank %s, approximate size %d\n", pszName, fresload_GetObjectSizeOnDisk( FAUDIOBANK_RESTYPE, pszName ) );
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

	DEVPRINTF( "faudio_LoadBank: Loaded %s.  %d bytes remain for audio banks\n", pszName, faudio_GetBankMemoryAvailable() );
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

CFAudioEmitter *CFAudioEmitter::Create2D( FAudio_WaveHandle_t oWaveHandle, u8 uPriority, CFAudioEmitter **ppUserAudioEmitter )
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

CFAudioEmitter *CFAudioEmitter::Create3D( FAudio_WaveHandle_t oWaveHandle, u8 uPriority, const CFVec3A *poVecPosition_WS, f32 fRadiusOuter, BOOL bAutoUpdatePosition, CFAudioEmitter **ppUserAudioEmitter )
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
		poVirtualEmitter->oWaveHandle                  = oWaveHandle;
		poVirtualEmitter->ppUserAudioEmitter           = ppUserAudioEmitter;
		poVirtualEmitter->uPriority                    = uPriority;
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
		poVirtualEmitter->poVirtualListenerCurrent     = NULL;
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

	if (!poVirtualEmitter->poAudioEmitter) return;

	if( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE == poVirtualEmitter->poAudioEmitter->m_nUser )
	{
		return;
	}

	poVirtualEmitter->poAudioEmitter->m_nUser = FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE;

	if( poVirtualEmitter->poRealEmitter )
	{
		if( ( FAUDIO_EMITTER_STATE_PLAYING == poVirtualEmitter->oeState ) || ( FAUDIO_EMITTER_STATE_PAUSED == poVirtualEmitter->oeState ) )
		{
			if( poVirtualEmitter->pEndOfPlayCallback )
			{
				poVirtualEmitter->pEndOfPlayCallback( poVirtualEmitter->poAudioEmitter );
			}

			poVirtualEmitter->poRealEmitter->poDSBuffer->StopEx( 0, DSBSTOPEX_IMMEDIATE );
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

		// !!Nate - Make sure to release
		//poVirtualEmitter->poRealEmitter->poDSBuffer->SetBufferData( NULL, 0);
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

void CFAudioEmitter::SetVolume( f32 fVolume ) {

	FASSERT_MSG( FAudio_bModuleInstalled,              "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( FMATH_IS_UNIT_FLOAT( fVolume ), "[ FAUDIO ] Error: Invalid fVolume !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );

#if !FANG_PRODUCTION_BUILD
	FASSERT(flinklist_IsLinkInList(poVirtualEmitter->paoVirtualEmittersListActive, flinklist_GetLinkPointer(poVirtualEmitter->paoVirtualEmittersListActive, poVirtualEmitter)));
#endif

	f32 fTemp = ( fVolume - poVirtualEmitter->fVolume );
	BOOL bVolumeIncreased = ( fTemp >= 0.0f ) ? TRUE : FALSE;

	fTemp = ( fTemp * fTemp );
	if( ( ! ( ( _EMITTER_STATE_CHANGE_DUCKABLE | _EMITTER_STATE_CHANGE_DUCKING ) & poVirtualEmitter->uStateChanges ) ) && // Ducking state change.
		( _SIGNIFICANT_VOLUME_CHANGE_SQ > fTemp ) ) {
		// Indistinguishable difference.
		return;
	}

	poVirtualEmitter->uStateChanges |= _EMITTER_STATE_CHANGE_VOLUME;
	poVirtualEmitter->fVolume        = fVolume;
	if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_DUCKABLE ) {
		poVirtualEmitter->fVolumeDucked = fVolume * _fDuckingFactor;
	} else {
		poVirtualEmitter->fVolumeDucked = fVolume;
	}

	// Nothing to sort.
	if( 1 == poVirtualEmitter->paoVirtualEmittersListActive->nCount ) {
		poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_DUCKABLE | _EMITTER_STATE_CHANGE_DUCKING ) );
		return;
	}

	if( _EMITTER_STATE_CHANGE_DUCKING & poVirtualEmitter->uStateChanges ) {
		poVirtualEmitter->uStateChanges &= ( ~ ( _EMITTER_STATE_CHANGE_DUCKABLE | _EMITTER_STATE_CHANGE_DUCKING ) );
		return;
	}

	// Keep list in volume order.
	_VirtualEmitter_t *poVirtualEmitterIterator;
	fTemp = poVirtualEmitter->fVolumeDucked;
	FLinkRoot_t *poList = poVirtualEmitter->paoVirtualEmittersListActive;

	if( bVolumeIncreased ) {
		// Look to move this element to the left.
		poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetPrev( poList, poVirtualEmitter );
		flinklist_Remove( poList, poVirtualEmitter );

		while( poVirtualEmitterIterator ) {
			// See if our volume is less than the previous one.
			if( poVirtualEmitterIterator->fVolumeDucked >= fTemp ) {
				break;
			}

			// Get the next item to the left.
			poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetPrev( poList, poVirtualEmitterIterator );
		}

		// Insert after poVirtualEmitterIterator.
		if( poVirtualEmitterIterator ) {
			flinklist_AddAfter( poList, poVirtualEmitterIterator, poVirtualEmitter );
		} else {
			flinklist_AddHead( poList, poVirtualEmitter );
		}
	} else {
		// Look to move this element to the right.
		poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetNext( poList, poVirtualEmitter );
		flinklist_Remove( poList, poVirtualEmitter );

		while( poVirtualEmitterIterator ) {
			// See if our volume is greater than the previous one.
			if( poVirtualEmitterIterator->fVolumeDucked <= fTemp ) {
				break;
			}

			// Get the next item to the right.
			poVirtualEmitterIterator = (_VirtualEmitter_t *)flinklist_GetNext( poList, poVirtualEmitterIterator );
		}

		// Insert before poVirtualEmitterIterator.
		if( poVirtualEmitterIterator ) {
			flinklist_AddBefore( poList, poVirtualEmitterIterator, poVirtualEmitter );
		} else {
			flinklist_AddTail( poList, poVirtualEmitter );
		}
	}
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

void CFAudioEmitter::SetFrequencyFactor( f32 fFrequencyFactor ) {

	FASSERT_MSG( FAudio_bModuleInstalled,            "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( ( 0.0f <= fFrequencyFactor ), "[ FAUDIO ] Error: Invalid fFrequencyFactor !!!" );

	_VirtualEmitter_t *poVirtualEmitter = (_VirtualEmitter_t *)m_pUser;

	FASSERT_MSG( ( FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE != poVirtualEmitter->poAudioEmitter->m_nUser ), "[ FAUDIO ] Error: Invalid emitter !!!" );
	
	// all frequency changes are applied if there is any difference
	if( poVirtualEmitter->fFrequencyFactor != fFrequencyFactor )
	{
		poVirtualEmitter->uStateChanges    |= _EMITTER_STATE_CHANGE_FREQUENCY;
		poVirtualEmitter->fFrequencyFactor  = fFrequencyFactor;
	}
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
	poVirtualEmitter->uStateChanges |= ( _EMITTER_STATE_CHANGE_VOLUME | _EMITTER_STATE_CHANGE_PLAY );

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

void _TrackStreamsProgress( void ) {
	_Stream_t *poStream;
	DWORD dwStatus;

	for( u32 uIndex = 0; uIndex < _uMaxStreams; ++uIndex ) {
		poStream = &( _paoStreams[ uIndex ] );

		if( ! poStream->bIsActive ) {
			continue;
		}
		
		if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState ) {
			poStream->fSecondsPlayed += ( FLoop_fRealPreviousLoopSecs * poStream->fFrequencyFactor );

			if( 0.0f <= poStream->fSecondsToPlay ) {
				// Non-infinitely looping.
				if( poStream->fSecondsToPlay <= poStream->fSecondsPlayed ) {
					poStream->oeState       = FAUDIO_STREAM_STATE_STOPPED;
					poStream->uStateChanges = _STREAM_STATE_CHANGE_STOP;
					poStream->bIsLooping    = FALSE;
					_bSkipStreamsWorkDelay  = FALSE;
					poStream->poDSStream->Flush();
					poStream->poDSStream->Discontinuity();

					// NKM
					do {
						poStream->poDSStream->GetStatus( &dwStatus );
					} while( dwStatus & DSSTREAMSTATUS_PLAYING );
				}
			}
		}
	}
} // _TrackStreamsProgress

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _ApplyRealEmittersChanges( FLinkRoot_t *poVirtualEmittersListActive ) {
	FLinkRoot_t *poVirtualEmittersList;
	_VirtualEmitter_t *poVirtualEmitter;
	FDataWvbFile_Bank_t *poBank;
	FDataWvbFile_Wave_t *poWave;
	XBOXADPCMWAVEFORMAT *poWaveFormat;
	u32 uList, uNewFreq;
	LPDIRECTSOUNDBUFFER8 pDSBuffer;
	f32 fVolume;//, fOOLoopSecs;

#if 0
	if( !FLoop_bGamePaused ) {
		fOOLoopSecs = FLoop_fPreviousLoopOOSecs;
	} else {
		fOOLoopSecs = 1.0f;
	}
#endif

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

						//Note here:
						//We need to treat "head space" similar to view space in that the z axis is negative.
						//To fix this we simply negate the z values in the post transformed vectors.

						// SER: Ignore the "Note here" comment above. It's lying to you!

						// Position.
						poVirtualEmitter->poVirtualListenerCurrent->poXfmCurrentOrientation_WS->TransformPointR( _oTempVec3A.v3, poVirtualEmitter->poVecCurrentPosition_WS->v3 );

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

//						_oTempVec3A_Velocity.Mul( fOOLoopSecs );

						_oDSEmitterAttributes.vVelocity.x = _oTempVec3A_Velocity.x;
						_oDSEmitterAttributes.vVelocity.y = _oTempVec3A_Velocity.y;
						_oDSEmitterAttributes.vVelocity.z = _oTempVec3A_Velocity.z;
						
						// Radius, Doppler.
//						_oDSEmitterAttributes.flMinDistance   = poVirtualEmitter->fRadiusInner;
						_oDSEmitterAttributes.flMaxDistance   = poVirtualEmitter->fRadiusOuter;
						_oDSEmitterAttributes.flDopplerFactor = poVirtualEmitter->fDopplerFactor;

						pDSBuffer->SetAllParameters( &_oDSEmitterAttributes, DS3D_DEFERRED );
					} else {
						pDSBuffer->SetVelocity( 0.0f, 0.0f, 0.0f, DS3D_DEFERRED );
					}
				}

				// Volume.
				if( FAudio_bMasterSfxVolChanged ||
					(poVirtualEmitter->uStateChanges & _EMITTER_STATE_CHANGE_VOLUME) ) {
					fVolume = FAudio_fMasterSfxUnitVol * poVirtualEmitter->fVolumeDucked;
					pDSBuffer->SetVolume( _anVolumes[ fmath_FloatToU32( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * fVolume ) ] );
				}
				
				// Frequency.
				if( _EMITTER_STATE_CHANGE_FREQUENCY & poVirtualEmitter->uStateChanges ) {
					poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
					uNewFreq = fmath_FloatToU32( poVirtualEmitter->fFrequencyFactor * poWave->fFreqHz );
					FMATH_CLAMP( uNewFreq, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX );
					pDSBuffer->SetFrequency( uNewFreq );
				}

				// Pan.
				if( _EMITTER_STATE_CHANGE_PAN & poVirtualEmitter->uStateChanges ) {
					if( poVirtualEmitter->fPanLeftRight >= 0.0f ) {
						_aoDSMixbinsVolumepair[ 0 ].lVolume = _anVolumes[ fmath_FloatToU32( ( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * 0.5f * ( 1.0f - poVirtualEmitter->fPanLeftRight ) ) ) ];
						_aoDSMixbinsVolumepair[ 1 ].lVolume = DSBVOLUME_MAX;
					} else {
						_aoDSMixbinsVolumepair[ 0 ].lVolume = DSBVOLUME_MAX;
						_aoDSMixbinsVolumepair[ 1 ].lVolume = _anVolumes[ fmath_FloatToU32( ( _UNIQUE_FLOAT_VOL_LEVEL_INDICES * 0.5f * ( 1.0f + poVirtualEmitter->fPanLeftRight ) ) ) ];
					}
					pDSBuffer->SetMixBinVolumes( &_oDSMixbins );
				}
				
				// Reverb.
				if( _EMITTER_STATE_CHANGE_REVERB & poVirtualEmitter->uStateChanges ) {
					// -11000.0f = DSI3DL2BUFFER_MINROOM - DSI3DL2BUFFER_MAXROOM;
					_oDSEmitterReverbParam.lRoom   = fmath_FloatToS32( 1000.0f + ( -11000.0f * ( 1.0f - poVirtualEmitter->fReverb ) ) );
					_oDSEmitterReverbParam.lRoomHF = fmath_FloatToS32( -10000.0f * ( 1.0f - poVirtualEmitter->fReverb ) );

					pDSBuffer->SetI3DL2Source( &_oDSEmitterReverbParam, DS3D_DEFERRED );
				}

				// Play / unpause.
				if( ( _EMITTER_STATE_CHANGE_PLAY | _EMITTER_STATE_CHANGE_UNPAUSE ) & poVirtualEmitter->uStateChanges ) {
					poWave = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;

					if( poVirtualEmitter->uProperties & _EMITTER_PROPERTIES_LOOPING ) {
						poWave       = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
						poBank       = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);
						poWaveFormat = (XBOXADPCMWAVEFORMAT *)( (u32)( poBank->pData ) + ( poWave->uIndex * sizeof( XBOXADPCMWAVEFORMAT ) ) );

						if( _EMITTER_STATE_CHANGE_PLAY & poVirtualEmitter->uStateChanges) {
							// NKM - I added a check here for the case that we wait for a time longer than the length of the sound.
							//		 So if we try to set the buffer position past the size of the sound buffer we set our position to
							//		 0 so we don't crash.
							DWORD uPos = ( poWaveFormat->wfx.nBlockAlign * ( fmath_FloatToU32( ( poVirtualEmitter->fSecondsPlayed - ( (f32)( (u32)( poVirtualEmitter->fSecondsPlayed / poWave->fLengthInSeconds ) ) * poWave->fLengthInSeconds ) ) * (f32)poWaveFormat->wfx.nAvgBytesPerSec ) / poWaveFormat->wfx.nBlockAlign ) );

							if( uPos >= poWave->uLength ) {
								uPos = 0;
							}

							pDSBuffer->SetCurrentPosition( uPos );
						}
						pDSBuffer->PlayEx( 0, DSBPLAY_LOOPING );
					} else {
						poWave       = (FDataWvbFile_Wave_t *)poVirtualEmitter->oWaveHandle;
						poBank       = (FDataWvbFile_Bank_t *)(poWave->oBankHandle);
						poWaveFormat = (XBOXADPCMWAVEFORMAT *)( (u32)( poBank->pData ) + ( poWave->uIndex * sizeof( XBOXADPCMWAVEFORMAT ) ) );

						FMATH_DEBUG_FCHECK( poVirtualEmitter->fSecondsPlayed );
						// RAF -- Commented out because I don't think this is necessary
						//if( _EMITTER_STATE_CHANGE_PLAY & poVirtualEmitter->uStateChanges) {
						//	pDSBuffer->SetCurrentPosition( ( poWaveFormat->wfx.nBlockAlign * ( fmath_FloatToU32( poVirtualEmitter->fSecondsPlayed * (f32)poWaveFormat->wfx.nAvgBytesPerSec ) / poWaveFormat->wfx.nBlockAlign ) ) );
						//}
						pDSBuffer->PlayEx( 0, 0 );	
					}
				}
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

void _FFileAsyncReadHeaderCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser )
{
	if ( !FAudio_bModuleInstalled )
	{
		return;
	}

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

	if ( !poStream->bIsActive )
	{
		return;
	}

	WaitForSingleObject( poStream->ohMutex, INFINITE );

	FASSERT_MSG( ( uIndex < _uMaxStreams ),      "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( ohFile == poStream->ohFile ), "[ FAUDIO ] Error: Invalid stream !!!" );
//	FASSERT_MSG( poStream->bIsActive,            "[ FAUDIO ] Error: Invalid stream !!!" );

	if( FFILE_ERROR_RET == nErrorCode )
	{
		poStream->oeState = FAUDIO_STREAM_STATE_ERROR;

		ReleaseMutex( poStream->ohMutex );
		return;
	}

	FDX8Data_WvsFile_Header_t *poStreamHeader = (FDX8Data_WvsFile_Header_t *)poStream->apauFileBuff[ 0 ];

	poStream->fFrequencyInHz   = poStreamHeader->fFreqHz;
	poStream->fLengthInSeconds = poStreamHeader->fSecs;
	poStream->uCurrentReadBuff = 0;
	poStream->uFileChanges     = _STREAM_FILE_CHANGE_READ_BODY;

	FASSERT_MSG( ( sizeof( FDX8Data_WvsFile_Header_t ) == uBytesRead ),                                     "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( pDestBuffer == poStream->apauFileBuff[ 0 ] ),                                            "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( poStreamHeader->WavFormat.WaveFormatEx.wFormatTag == WAVE_FORMAT_XBOX_ADPCM ),           "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( (u32)poStreamHeader->fFreqHz == poStreamHeader->WavFormat.WaveFormatEx.nSamplesPerSec ), "[ FAUDIO ] Error: Invalid stream !!!" );

	poStream->poDSStream->SetFormat( (LPCWAVEFORMATEX)&( poStreamHeader->WavFormat ) );

	ReleaseMutex( poStream->ohMutex );

} // _FFileAsyncReadHeaderCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void _FFileAsyncReadBodyCallback( s32 nErrorCode, FFileHandle ohFile, void *pDestBuffer, u32 uBytesRead, void *pUser )
{
	if ( !FAudio_bModuleInstalled )
	{
		return;
	}

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

	if ( !poStream->bIsActive )
	{
		return;
	}

	WaitForSingleObject( poStream->ohMutex, INFINITE );

	FASSERT_MSG( ( uIndex < _uMaxStreams ),                                               "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( ohFile == poStream->ohFile ),                                          "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( poStream->bIsActive,                                                     "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( _STREAM_FILE_CHANGE_READ_BODY_PENDING == poStream->uFileChanges ),     "[ FAUDIO ] Error: Invalid stream !!!" );
	FASSERT_MSG( ( pDestBuffer == poStream->apauFileBuff[ poStream->uCurrentReadBuff ] ), "[ FAUDIO ] Error: Invalid stream !!!" );

	if( FFILE_ERROR_RET == nErrorCode )
	{
		poStream->oeState = FAUDIO_STREAM_STATE_ERROR;

		ReleaseMutex( poStream->ohMutex );
		return;
	}

	if( FAUDIO_STREAM_STATE_CREATING == poStream->oeState )
	{
		poStream->bIgnoreInPauseMode    = FALSE;
		poStream->fVolume               = 1.0f;
		poStream->fPanLeftRight         = 0.0f;
		poStream->fFrequencyFactor      = 1.0f;
		poStream->pEndOfPlayCallback    = NULL;
		poStream->auReadyToConsume[ 0 ] = 0;
		poStream->auReadyToConsume[ 1 ] = 0;
		poStream->oeState               = FAUDIO_STREAM_STATE_STOPPED;
		poStream->uStateChanges         = ( _STREAM_STATE_CHANGE_VOLUME | _STREAM_STATE_CHANGE_PAN | _STREAM_STATE_CHANGE_FREQUENCY );
	}

	poStream->uFileChanges = ( ( ( FDX8DATA_WVS_MAX_CHUNK_BYTES > uBytesRead ) && ( poStream->bIsLooping ) ) ? _STREAM_FILE_CHANGE_EOF : _STREAM_FILE_CHANGE_NONE );
	poStream->auReadyToConsume[ poStream->uCurrentReadBuff ] = uBytesRead;
	poStream->uFilePos += uBytesRead;

	if( ( poStream->uFilePos >= poStream->uFileSize ) && poStream->uFileChanges != _STREAM_FILE_CHANGE_EOF ) {
 		poStream->uFileChanges = _STREAM_FILE_CHANGE_EOF;
	}

	ReleaseMutex( poStream->ohMutex );

} // _FFileAsyncReadBodyCallback

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CFAudioStream *CFAudioStream::GetStream( u32 uStreamIdx ) {
	if( uStreamIdx < _uMaxStreams ) {
		return _paoStreams[ uStreamIdx ].poAudioStream;
	}
	return NULL;
}

CFAudioStream *CFAudioStream::Create( cchar *pszName, BOOL bWillBeUsedForMusic/*=TRUE*/ ) {
	
	if( ! FAudio_bModuleInstalled ) {
		return NULL;
	}

	if( ! pszName ) {
		DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioStream::Create() failed !!!\n", __LINE__ );
		return NULL;
	}

	if( fclib_strlen( pszName ) > FAUDIO_MAX_ASSET_NAME_LENGTH ) {
		DEVPRINTF( "[ FAUDIO ] Error %u: CFAudioStream::Create() failed. Stream name '%s' is longer than %u characters.\n", __LINE__, pszName, FAUDIO_MAX_ASSET_NAME_LENGTH );
		return NULL;
	}

	_Stream_t *poStream;

	u32 uIndex;

	for( uIndex = 0; uIndex < _uMaxStreams; ++uIndex ) {
		poStream = &_paoStreams[ uIndex ];

		if( ! poStream->bIsActive ) {
			break;
		}
	}

	if( uIndex < _uMaxStreams ) {
		char szTemp[ FAUDIO_MAX_ASSET_NAME_LENGTH + 128 ];

		fclib_strcpy( szTemp, Fang_ConfigDefs.pszFile_GameRootPathName );
		fclib_strcat( szTemp, pszName );
		fclib_strcat( szTemp, ".wvs" );

		poStream->ohFile = ffile_Open( szTemp, FFILE_OPEN_RONLY, TRUE );
		if( FFILE_INVALID_HANDLE == poStream->ohFile ) {
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
		poStream->ePauseLevel 	 = FAUDIO_PAUSE_LEVEL_1; //default pause level

		return poStream->poAudioStream;
	} else {
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

	poStream->bIsActive = FALSE;

	if( FAUDIO_STREAM_STATE_PLAYING == poStream->oeState )
	{
		if( poStream->pEndOfPlayCallback )
		{
			poStream->pEndOfPlayCallback( poStream->poAudioStream );
		}

		poStream->poDSStream->Flush();
		poStream->poDSStream->Discontinuity();

		// NKM
		DWORD dwStatus;
		do {
			poStream->poDSStream->GetStatus( &dwStatus );
		} while( dwStatus & DSSTREAMSTATUS_PLAYING );
	}

	ffile_Close( poStream->ohFile );
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

void CFAudioStream::SetVolume( f32 fVolume ) {

	FASSERT_MSG( FAudio_bModuleInstalled,              "[ FAUDIO ] Error: System not installed !!!" );
	FASSERT_MSG( FMATH_IS_UNIT_FLOAT( fVolume ), "[ FAUDIO ] Error: Invalid fVolume !!!" );

	_Stream_t *poStream = (_Stream_t *)m_uData;

	FASSERT_MSG( poStream->bIsActive, "[ FAUDIO ] Error: Invalid stream !!!" );
	
	if( ( FAUDIO_STREAM_STATE_ERROR    == poStream->oeState ) ||
		( FAUDIO_STREAM_STATE_CREATING == poStream->oeState ) ) {
		return;
	}

	if( fVolume != poStream->fVolume ) {
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

//void CFAudioStream::Play( u32 uLoops /* = 1 */, f32 fFirstLoopStartOffsetInSeconds /* = 0.0f */ )
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

	////
	//
//	FASSERT_MSG( ( fFirstLoopStartOffsetInSeconds < poWave->fLengthInSeconds ), "[ FAUDIO ] Error: Invalid fFirstLoopStartOffsetInSeconds !!!" );
	//
	////

	poStream->oeState        = FAUDIO_STREAM_STATE_PLAYING;
	poStream->uStateChanges &= ( ~ ( _STREAM_STATE_CHANGE_STOP | _STREAM_STATE_CHANGE_PAUSE | _STREAM_STATE_CHANGE_UNPAUSE ) );
	poStream->uStateChanges |= ( _STREAM_STATE_CHANGE_PLAY | _STREAM_STATE_CHANGE_VOLUME );

//	poStream->fSecondsPlayed = fFirstLoopStartOffsetInSeconds;
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
		poStream->oeState        = FAUDIO_STREAM_STATE_STOPPED;
		poStream->uStateChanges &= ( ~ ( _STREAM_STATE_CHANGE_PLAY | _STREAM_STATE_CHANGE_PAUSE | _STREAM_STATE_CHANGE_UNPAUSE ) );
		poStream->uStateChanges |= _STREAM_STATE_CHANGE_STOP;
		
		poStream->poDSStream->FlushEx( 0, DSSTREAMFLUSHEX_IMMEDIATE );
		poStream->poDSStream->Discontinuity();

		// NKM
		DWORD dwStatus;
		do {
			poStream->poDSStream->GetStatus( &dwStatus );
		} while( dwStatus & DSSTREAMSTATUS_PLAYING );
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

	return poStream->oeState;

} // CFAudioStream::GetState

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
