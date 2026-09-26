//////////////////////////////////////////////////////////////////////////////////////
// faudio.h - Fang audio.
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
// 04/17/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FAUDIO_H_
#define _FAUDIO_H_ 1

#include "fang.h"
#include "fworld.h"
#include "fdata.h"
#include "flinklist.h"


#define FAUDIO_MAX_ASSET_NAME_LENGTH		( 11 )
#define FAUDIO_MAX_LISTENERS				( 4 )
#define FAUDIO_UNLIMITED_EMITTERS			( (u32)-1 )
#define FAUDIO_UNLIMITED_SECONDS			( -1.0f )
#define FAUDIO_MIN_RADIUS					( 0.0001f )
#define FAUDIO_INVALID_HANDLE				( NULL )

typedef enum {
	FAUDIO_NO_ERROR	=	0,
	FAUDIO_ERROR	=	-1
} FAudio_Error_e;

typedef enum {
	FAUDIO_PAUSE_LEVEL_NONE			= 0,
	FAUDIO_PAUSE_LEVEL_1			= 1, //This is the default pause level for emitters and streams
	FAUDIO_PAUSE_LEVEL_2			= 2,
	FAUDIO_PAUSE_LEVEL_3			= 3,
	FAUDIO_PAUSE_LEVEL_4			= 4,
	FAUDIO_PAUSE_LEVEL_5			= 5
} FAudio_PauseLevel_e;

// Nate - Please note that these values will be stored into a u8 on all platforms
typedef enum {
	FAUDIO_EMITTER_STATE_NONE		= 0x00000000,
	FAUDIO_EMITTER_STATE_PLAYING	= 0x00000001,
	FAUDIO_EMITTER_STATE_PAUSED		= 0x00000002,
	FAUDIO_EMITTER_STATE_STOPPED	= 0x00000003
} FAudio_EmitterState_e;

typedef enum {
	FAUDIO_STREAM_STATE_NONE		= 0x00000000,
	FAUDIO_STREAM_STATE_ERROR		= 0x00000001,
	FAUDIO_STREAM_STATE_CREATING	= 0x00000002,
	FAUDIO_STREAM_STATE_PLAYING		= 0x00000003,
	FAUDIO_STREAM_STATE_PAUSED		= 0x00000004,
	FAUDIO_STREAM_STATE_STOPPED		= 0x00000005
} FAudio_StreamState_e;

///////////////////
// GLOBAL VARIABLES
extern u8 FAudio_EmitterDefaultPriorityLevel; // Priority 0 is the absolute lowest.
extern BOOL8 FAudio_bModuleStarted;
extern BOOL8 FAudio_bModuleInstalled;
extern BOOL8 FAudio_bMasterSfxVolChanged;
extern BOOL8 FAudio_bMasterMusicVolChanged;
extern f32 FAudio_fMasterSfxUnitVol;
extern f32 FAudio_fMasterMusicUnitVol;



///////////////////////////////////////////////////////////////////////////////////////
FCLASS_NOALIGN_PREFIX class CFAudioEmitter : public CFWorldUser
{
public:
	// callback function prototype
	typedef void FAudio_EmitterEndOfPlayCallback_t( CFAudioEmitter *poAudioEmitter );
	
	///////////////////////////////////////////////
	// static interface - operates on all emitters
    //
	static CFAudioEmitter *Create2D( FAudio_WaveHandle_t oWaveHandle,
									 u8 uPriority = FAudio_EmitterDefaultPriorityLevel, CFAudioEmitter **ppUserAudioEmitter=NULL );
	static CFAudioEmitter *Create3D( FAudio_WaveHandle_t oWaveHandle,
									 u8 uPriority = FAudio_EmitterDefaultPriorityLevel,
									 const CFVec3A *poVecPosition_WS = NULL,
									 f32 fRadiusOuter = FAUDIO_MIN_RADIUS,
									 BOOL bAutoUpdatePosition = FALSE,	// If TRUE, then poVecPosition_WS will be assumed to PRESIST and be used for updates every frame
									 CFAudioEmitter **ppUserAudioEmitter = NULL );

	static void DestroyAll( void );
	
	static void DuckAll( f32 fVolumeFactor );
	
	static void StopAll( void ); //This function will stop all sounds, and Destroy all FireNForget Sounds
	
	static FAudio_Error_e Play2D( FAudio_WaveHandle_t oWaveHandle,
								  f32 fVolume = 1.0f,
								  u8 uPriority = FAudio_EmitterDefaultPriorityLevel,
								  BOOL bDuckable = TRUE,
								  u32 uLoops = 1,							// 0 : infinite loop.
								  f32 fPanLeftRight = 0.0f,
								  f32 fFrequencyFactor = 1.0f,
								  CFAudioEmitter **ppUserAudioEmitter=NULL );
	// 3D only (will not actually do anything until a world is present).
	static FAudio_Error_e Play3D( FAudio_WaveHandle_t oWaveHandle,
								  const CFVec3A *poVecPosition_WS,
								  f32 fRadiusOuter,
								  f32 fVolume = 1.0f,
								  u8 uPriority = FAudio_EmitterDefaultPriorityLevel,
								  BOOL bDuckable = TRUE,
								  u32 uLoops = 1,							// 0 : infinite loop.
								  f32 fFrequencyFactor = 1.0f,
								  f32 fSpawnRadiusFactor = -1.0f,			// -1.0f : Ignore fSpawnRadiusFactor.
								  BOOL bAutoUpdatePosition = FALSE,			// If TRUE, then poVecPosition_WS will be assumed to PRESIST and be used for updates every frame
								  CFAudioEmitter **ppUserAudioEmitter=NULL );
	
	static FAudio_PauseLevel_e SetGlobalPauseLevel( FAudio_PauseLevel_e eNewPauseLevel ); //sets a new pause level, and returns the old pause level
	static FAudio_PauseLevel_e GetGlobalPauseLevel( void ); //gets the current global pause level


	////////////////////////////////////////////////////
	// emitter interface - operates on a single emitter
	//
	void Play( u32 uLoops = 1 ); // 0 : infinite loop.
	void Stop( BOOL bAfterCurrentLoop );
	void Destroy( void );
		
	void SetVolume( f32 fVolume );														// Changes are applied only if they're significant.

	void SetEndOfPlayCallback( FAudio_EmitterEndOfPlayCallback_t *pEndOfPlayCallback );	// Set to NULL to disable (NULL by default).

	// 1.0f normal.
	// Changes are applied only if they're significant.
	void SetFrequencyFactor( f32 fFrequencyFactor );

	void SetDuckable( BOOL bDuckable );
	BOOL GetDuckable( void );

	void SetPriority( u8 uPriority ); // Valid range: 0 to FAudio_Init_t::uMaxPriorityLevels.
	u32 GetPriority( void );
	
	// 2D only.
		// Bi-polar unitfloat. -1.0f left, 0.0f center, 1.0f right.
		// Changes are applied only if they're significant.
		void SetPan( f32 fPanLeftRight );
	
	// 3D only (will not actually do anything until a world is present).
		// Changes are applied only if they're significant.  
		// If this is a sound with FAUDIO_EMITTER_PROPERTIES_AUTOPOSUPDATE then this will represent
		// the new vector pointer to use as a new autoupdate pointer
		void SetPosition( const CFVec3A *poVecPosition_WS );
		
		// Useful when moving across a great distance.
		// Changes are applied only if they're significant.
		// If this is a sound with FAUDIO_EMITTER_PROPERTIES_AUTOPOSUPDATE then this will represent
		// the new vector pointer to use as a new autoupdate pointer
		void SetPositionAndMaintainDoppler( const CFVec3A *poVecPosition_WS );

		// 0.0f none, 1.0f normal, 10.0f max.
		// Changes are applied only if they're significant.
		void SetDopplerFactor( f32 fDopplerFactor );

		// 0.0f none, 1.0f max.
		// Changes are applied only if they're significant.
		void SetReverb( f32 fReverb );
	
	FAudio_EmitterState_e GetState( void );
	f32 GetSecondsPlayed( void );
	f32 GetSecondsToPlay( void ); // FAUDIO_UNLIMITED_SECONDS : infinite loop.
	
	BOOL Is3D( void );
	BOOL IsFireNForget( void );

	FAudio_PauseLevel_e GetPauseLevel( void );
	void SetPauseLevel( FAudio_PauseLevel_e eNewPauseLevel ); //This does NOT pause the emitter, just set's it's pauselevel for PauseMode()	

public:
	#if !FANG_PRODUCTION_BUILD
	cchar	*m_pszModuleOwner;  // Points to name of the module that currently has this emitter reserved
	u32		m_nOwnerModuleLine; // Indicates the line in the module that caused this emitter to be reserved
	#endif

protected:

private:

	void Pause( BOOL bEnabled );

	// Ignored on GC.
	// Changes are applied only if they're significant.
	void SetRadius( f32 fRadiusOuter, f32 fRadiusInner = FAUDIO_MIN_RADIUS );

	FCLASS_STACKMEM_NOALIGN( CFAudioEmitter );
} FCLASS_NOALIGN_SUFFIX; // CFAudioEmitter


//////////////////////////////////////////////////////////////////////////////////////////////
FCLASS_NOALIGN_PREFIX class CFAudioStream
{
public:
	// callback function prototype
	typedef void FAudio_StreamEndOfPlayCallback_t( CFAudioStream *poAudioStream );
	
	//////////////////////////////////////////////
	// static interface - operates on all streams
    //
	static CFAudioStream *Create( cchar *pszName, BOOL bWillBeUsedForMusic=TRUE );
	
	static void DestroyAll( void );

	static CFAudioStream *GetStream( u32 uStreamIdx );

	static FAudio_PauseLevel_e SetGlobalPauseLevel( FAudio_PauseLevel_e eNewPauseLevel ); // sets a new stream pause level, and returns the old stream pause level
	static FAudio_PauseLevel_e GetGlobalPauseLevel( void );

	static u32 GetNumActive( void );
	static cchar *GetName( u32 uIndex );

	
	/////////////////////////////////////////////////
	// stream interface - operates on a single stream
	//
	void Play( u32 uLoops = 1 );	// 0 : infinite loop.
	void Stop( BOOL bAfterCurrentLoop );
	void Pause( BOOL bEnabled );	// this actually pauses the stream
	void Destroy( void );
		
	void SetVolume( f32 fVolume );	// Changes are applied only if they're significant.
	
	void SetEndOfPlayCallback( FAudio_StreamEndOfPlayCallback_t *pEndOfPlayCallback );	// Set to NULL to disable (NULL by default).

	// 1.0f normal.
	// Changes are applied only if they're significant.
	void SetFrequencyFactor( f32 fFrequencyFactor );

	// Bi-polar unitfloat. -1.0f left, 0.0f center, 1.0f right.
	// Changes are applied only if they're significant.
	void SetPan( f32 fPanLeftRight );
	
	FAudio_PauseLevel_e GetPauseLevel( void );
	void SetPauseLevel( FAudio_PauseLevel_e eNewPauseLevel ); //This does NOT pause the stream, just set's it's pauselevel for PauseMode	

	FAudio_StreamState_e GetState( void );
	f32 GetSecondsPlayed( void );
	f32 GetSecondsToPlay( void ); // FAUDIO_UNLIMITED_SECONDS : infinite loop.
	
	cchar *GetName( void );
	
	u32 m_uData; // DO NOT MODIFY UNLESS YOU KNOW WHAT YOU'RE DOING.

protected:

private:

	FCLASS_STACKMEM_NOALIGN( CFAudioStream );

} FCLASS_NOALIGN_SUFFIX; // CFAudioStream

typedef struct {
	u32 uPriorityLevel;
	u32 uMaxPlayable; // Can be FAUDIO_UNLIMITED_EMITTERS.
} FAudio_PriorityLimit_t;

typedef struct {
	u32 uMaxListeners;		// Maximum at any one time.
	u32 uMaxEmitters;		// Maximum at any one time.
	u32 uMaxStreams;		// Maximum at any one time.
	u32 uMaxBanks;			// Maximum at any one time.
	u8 uMaxPriorityLevels;	// Maximum total.

	u32 uPriorityLimits;	// Can be 0.
	FAudio_PriorityLimit_t *paoPriorityLimits; // Pointer to an array of uPriorityLimits elements. Can be NULL.

	u32 DX8ONLY_ohWnd;		// [DX8] Handle of window. [else] ignored.
	u32 DX8ONLY_ohInstance;	// [DX8] HInstance of app. [else] ignored.
} FAudio_Init_t;

typedef struct {
	char szName[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
	u32 uWaves;
} FAudio_BankInfo_t;

typedef struct {
	char szName[ FAUDIO_MAX_ASSET_NAME_LENGTH + 1 ];
	u32 uChannels;
	f32 fFrequency;
	f32 fLengthInSeconds;
} FAudio_WaveInfo_t;


//////////////////////
// FUNCTION PROTOTYPES
extern BOOL faudio_ModuleStartup( void );
extern void faudio_ModuleShutdown( void );

extern FAudio_Error_e faudio_Install( const FAudio_Init_t *poInit );
extern void faudio_Uninstall( void );
extern BOOL faudio_IsInstalled( void );

f32 faudio_SetSfxMasterVol( f32 fNewVol );
f32 faudio_GetSfxMasterVol( void );

f32 faudio_SetMusicMasterVol( f32 fNewVol );
f32 faudio_GetMusicMasterVol( void );

extern void faudio_Work( void ); // Applies most changes at lower fps. (on/off: full fps. move xyz: lower fps.)

extern void faudio_SetPlayableLimitPerPriorityLevel( u32 uPriorityLevel, u32 uMaxPlayable );
extern u32 faudio_GetPlayableLimitPerPriorityLevel( u32 uPriorityLevel );
#if FANG_PLATFORM_XB
extern void faudio_UsingHeadphones_XB( BOOL bHeadphones );
#endif
#if FANG_PLATFORM_GC
extern void faudio_EnableSurround_GC( BOOL bSurroundEnabled );
extern BOOL faudio_IsSurroundEnabled_GC( void );
#endif

extern u32 faudio_GetBankMemoryAvailable( void );	// Based on uMaxSoundBytes, from faudio_Install().
extern u32 faudio_GetBankMemoryUsed( void );		// Based on uMaxSoundBytes, from faudio_Install().

// To release the memory allocated by faudio_LoadBanks(), get a frame handle using fres_GetFrame() before calling faudio_LoadBanks().
// Banks pointed to by papszNames will be stacked in the order received.
// Return values : TRUE : No errors, FALSE: Error.
extern FAudio_Error_e faudio_LoadBanks( cchar **papszNames, FAudio_BankHandle_t *paoBankHandles );
extern FAudio_BankHandle_t faudio_LoadBank( cchar *pszName );

extern void faudio_GetBankAttributes( FAudio_BankHandle_t oBankHandle, FAudio_BankInfo_t *poBankInfo );
extern FAudio_WaveHandle_t faudio_GetWaveHandle( FAudio_BankHandle_t oBankHandle, cchar *pszName );
extern FAudio_WaveHandle_t faudio_GetWaveHandle( FAudio_BankHandle_t oBankHandle, u32 uWaveIndex );
extern void faudio_GetWaveAttributes( FAudio_WaveHandle_t oWaveHandle, FAudio_WaveInfo_t *poWaveInfo );
extern void faudio_GetWaveAttributes( FAudio_BankHandle_t oBankHandle, u32 uWaveIndex, FAudio_WaveInfo_t *poWaveInfo );
extern BOOL faudio_IsValidBankHandle( FAudio_BankHandle_t oBankHandle );
extern BOOL faudio_IsValidWaveHandle( FAudio_WaveHandle_t oWaveHandle );

extern void faudio_SetActiveListenerCount( u32 uCount );
extern u32 faudio_GetActiveListenerCount( void );

// 3D only (will not actually do anything until a world is present).
// Changes are applied only if they're significant.
extern void faudio_SetListenerOrientation( u32 uListenerIndex, const CFXfm *poXfmOrientation_WS );

// Useful when moving across a great distance.
// Changes are applied only if they're significant.
extern void faudio_SetListenerOrientationAndMaintainDoppler( u32 uListenerIndex, const CFXfm *poXfmOrientation_WS );

#endif // _FAUDIO_H_
