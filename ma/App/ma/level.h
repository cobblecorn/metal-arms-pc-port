//////////////////////////////////////////////////////////////////////////////////////
// level.h - Level loader.
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
// 02/18/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _LEVEL_H_
#define _LEVEL_H_ 1

#include "fang.h"
#include "fgamedata.h"
#include "cutscene.h"

#define LEVEL_INVENTORY_TABLE_NAME	"Inventory"
#define LEVEL_DEFAULT_MUSIC_VOLUME	( 1.0f )
#define LEVEL_DEFAULT_DUCK_VOLUME	( 0.25f )

class CFAudioStream;



////////////////////////////////////////////////////////////////////////////////////////////////////////
// THE ORDER OF THIS ENUM IS VERY IMPORTANT AND IF ALTERED WILL REQUIRE SOME CSV CHANGES TO THE WRAPPERS
////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	LEVEL_DEVELOPMENT_LEVEL = -1,// Use this for any level that isn't part of the shipping game

	// LIST ALL SINGLE PLAYER LEVELS IN ORDER HERE
    LEVEL_SEAL_THE_MINES_1 = 0,
	LEVEL_SEAL_THE_MINES_2,
	LEVEL_SEAL_THE_MINES_3,
	LEVEL_CLEAN_UP,
	LEVEL_RAT_RACE,
	LEVEL_WASTELAND_JOURNEY_1,
	LEVEL_WASTELAND_JOURNEY_2,
	LEVEL_WASTELAND_JOURNEY_3,
	LEVEL_ZOMBIEBOT_BOSS,
	LEVEL_DESTROY_THE_COMMUNICATIONS_ARRAY_1,
	LEVEL_DESTROY_THE_COMMUNICATIONS_ARRAY_2,
	LEVEL_DESTROY_THE_COMMUNICATIONS_ARRAY_3,
	LEVEL_HOLD_YOUR_GROUND,
	LEVEL_RESEARCH_FACILITY_1,
	LEVEL_RESEARCH_FACILITY_2,
	LEVEL_RESEARCH_FACILITY_3,
	LEVEL_RESEARCH_FACILITY_4,
	LEVEL_WASTELAND_CHASE,
	LEVEL_MORBOT_REGION_1,
	LEVEL_MORBOT_REGION_2,
	LEVEL_REACTOR_1,
	LEVEL_REACTOR_2,
	LEVEL_MIL_CITY_HUB,
	LEVEL_FIND_WRENCH,
	LEVEL_SPY_VS_SPY,
	LEVEL_ACCESS_THE_RUINS,
	LEVEL_RUINS_1,
	LEVEL_RUINS_2,
	LEVEL_SECRET_RENDEZVOUS,
	LEVEL_NIGHT_SNEAK_1,
	LEVEL_NIGHT_SNEAK_2,
	LEVEL_INVASION_1,
	LEVEL_COLISEUM_1,
	LEVEL_COLISEUM_2,
	LEVEL_COLISEUM_3,
	LEVEL_COLISEUM_4,
	LEVEL_RACE_TO_THE_ROCKET,
	LEVEL_SPACE_DOCK,
	LEVEL_SPACE_STATION_1,
	LEVEL_SPACE_STATION_2,
	LEVEL_GENERAL_CORROSIVE,
	LEVEL_FINAL_BATTLE,

	LEVEL_SINGLE_PLAYER_COUNT,

	// ADD MULTIPLAYER LEVELS HERE
	LEVEL_MULTIPLAYER_1 = LEVEL_SINGLE_PLAYER_COUNT,
	LEVEL_MULTIPLAYER_2,
	LEVEL_MULTIPLAYER_3,
	LEVEL_MULTIPLAYER_4,
	LEVEL_MULTIPLAYER_5,
	LEVEL_MULTIPLAYER_6,
	LEVEL_MULTIPLAYER_7,
	LEVEL_MULTIPLAYER_8,
	LEVEL_MULTIPLAYER_9,
	LEVEL_MULTIPLAYER_10,
	LEVEL_MULTIPLAYER_11,
	LEVEL_MULTIPLAYER_12,
	LEVEL_MULTIPLAYER_13,
	LEVEL_MULTIPLAYER_14,
	LEVEL_MULTIPLAYER_15,

	LEVEL_MULTIPLAYER_COUNT = (LEVEL_MULTIPLAYER_15 + 1 - LEVEL_MULTIPLAYER_1)
} Level_e;

typedef enum {
	LEVEL_EVENT_POST_WORLD_LOAD,		// Immediately after the world as been loaded
	LEVEL_EVENT_PRE_ENTITY_FIXUP,		// Immediately before CEntity::ResolveEntityPointerFixups() is called
	LEVEL_EVENT_PRE_PLAYER_BOT_INIT,	// After the inventory is setup but before the player bot is inited.
	LEVEL_EVENT_POST_ENTITY_FIXUP,		// Immediately after CEntity::ResolveEntityPointerFixups() is called
	LEVEL_EVENT_COUNT,
} LevelEvent_e;

typedef BOOL LevelLoadFcn_t( LevelEvent_e eEvent );
typedef void LevelUnloadFcn_t( void );
typedef void LevelWorkFcn_t( void );
typedef void LevelDrawFcn_t( void );

struct Level_t {
	cchar *pszTitle;					// Level's official title (NULL=end of table)
	cchar *pszWorldResName;				// Level's world resource name (WLD file)
	Level_e nLevel;						// A level ID
	cchar *pszCSVFile;					// CSV file containing data for this level
	cchar *pszMaterialCSVFile;			// CSV file containing material info for this level
	LevelLoadFcn_t *pFcnLoad;			// Called just after the world resource is loaded (NULL=none)
	LevelUnloadFcn_t *pFcnUnload;		// Called just before the world resource is freed (NULL=none)
	LevelWorkFcn_t *pFcnWork;			// Called as the last step in the work phase
	LevelDrawFcn_t *pFcnDraw;			// Called as the last step in the draw phase
#if FANG_PLATFORM_GC
	f32 fProjOffs;						// See fgcsh.h: fgcsh_SetProjectorOffset() for a description.
	f32 fProjRngAdj;					// See fgcsh.h: fgcsh_SetProjectorRangeAdj() for a description.
#endif
};

///////////////////////////////////////////////////////
// Global Vars:
extern s32 Level_nCount;		// Number of levels in the game
extern Level_t Level_aInfo[];	// Level info
extern s32 Level_nLoadedIndex;	// Index of currently loaded level (-1=none)
extern FGameDataFileHandle_t Level_hLevelDataFile;// the level specific data (.csv) file for the currently loaded level
extern FGameDataTableHandle_t Level_hStreamingAudioTable;
extern FGameDataTableHandle_t Level_hInventoryTable;
extern u32 Level_nNumStreamingAudioEntries;
extern f32 Level_fSecsInLevel;	// how long has the player been playing this level?
extern f32 Level_fStartingFadeSecs;// how long should we fade from black at the beginning of the level
extern f32 Level_fEndingFadeSecs;// how long should we fade from black at the beginning of the level

///////////////////////////////////////////////////////
// system functions:
extern BOOL level_InitSystem( void );
extern void level_UninitSystem( void );
extern BOOL level_IsInitialized( void );

// loading functions:
extern s32 level_Find( cchar *pszLevelTitle );
extern s32 level_Find( Level_e nLevel );
extern BOOL level_Load( cchar *pszLevelTitle, BOOL bShowLoadingScreen, cwchar *pwszLoadScreenHeading=NULL );
extern BOOL level_LoadGenericLevel( cchar *pszWorldResName );
extern void level_Unload( void );
extern void level_LoadMemoryCacheData( FGameDataFileHandle_t hCSVFile );
extern void level_LoadFogData( FGameDataFileHandle_t hCSVFile );
extern void level_LoadReflectionData( FGameDataFileHandle_t hCSVFile );
extern void level_LoadFadeData( FGameDataFileHandle_t hCSVFile );

// work/draw functions:
extern void level_Work( void );
extern void level_Draw( void );

// streaming audio functions:
extern cchar *level_GetSteamingAudioFilename( u32 nIndex, BOOL &rbTreatAsMusicStream );
extern int level_StartStream( cchar *pszFilename, u32 nNumLoops=1, f32 fVolume=1.0f, s32 nStreamIndex=-1, BOOL bTreatAsMusicStream=TRUE );
extern void level_StopStream( cchar *pszFilename );
extern CFAudioStream *level_GetStreamByName( cchar *pszFilename );
extern void level_StopAllStreams( void ); 
extern void level_SetCurStreamVolume( f32 fNewVolume );
extern void level_RestartAllStreams( void );

// everything played with the speech interface will be registered as a sound effect stream
extern void level_PlayStreamingSpeech( cchar *pszName, f32 fMaxVol, u32 nLoopCount=1 );
extern void level_StopStreamingSpeech( void );
extern BOOL level_IsStreamingSpeechPlaying( void );
extern BOOL level_IsMusicPlaying( void );
extern CFAudioStream *level_GetStreamingSpeechAudioStream( void );
extern CFAudioStream *level_GetMusicAudioStream( void );

// everything played with the music interface will be registered as a music stream
extern BOOL			  level_PlayMusic( cchar *pszName, f32 fVolLevel, BOOL bFadeIn );
// temporarily fade-in/out whatever music is going, but actually restarts...
extern BOOL			  level_PauseMusic( BOOL bPause);

// everything played with the sfx interface will be registered as a sfx stream
extern BOOL level_PlaySfxStream( cchar *pszName, f32 fVolLevel, BOOL bFadeIn );

extern BOOL level_StopMusic( void );
extern BOOL level_PlayDefaultStreamingTrack( f32 fVolLevel, BOOL bFadeIn );

extern void level_FadeMusicVolume( f32 fTargetUnitVolume, f32 fFadeTime ); //used for cutscene dialog
extern void level_DuckAudio( f32 fDuckVol = LEVEL_DEFAULT_DUCK_VOLUME); //ducks music and sound, increases nDuckCount
extern void level_UnduckAudio( void ); //decrements the nDuckCount variable

// movie functions:
extern cutscene_Handle_t level_GetEndingMovieHandle( void );
extern void level_SetIntroMovieHandle( cutscene_Handle_t hCutscene );
extern void level_PlayIntroMovie();

// checkpoint functions:
extern void level_CheckpointSave( void );
extern void level_CheckpointRestore( void );

// Mission Text:
extern u32 level_GetMissionTextCount( void );
extern u32 level_GetCurrentMissionTextIndex( void );
extern BOOL level_SetCurrentMissionTextIndex( u32 nMissionTextIndex );
extern cwchar *level_GetCurrentMissionText( void );

BOOL level_IsRacingLevel( void );

#endif

