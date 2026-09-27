//////////////////////////////////////////////////////////////////////////////////////
// level.cpp - Level loader.
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

#include "fang.h"
//for fog
#include "fsh.h"

#if FANG_PLATFORM_GC
#include "fgcsh.h"
#endif

#include "level.h"
#include "fclib.h"
#include "fres.h"
#include "fresload.h"
#include "fworld.h"
#include "fmovie2.h"
#include "ai/aimain.h"
#include "ai/AIBTATable.h"
#include "FScriptSystem.h"
#include "faudio.h"
#include "MAScriptTypes.h"
#include "SpaceDock.h"
#include "fparticle.h"
#include "eproj_cleaner.h"
#include "bottalkinst.h"
#include "gcoll.h"
#include "loadingscreen.h"
#include "cutscene.h"
#include "alarmsys.h"
#include "spawnsys.h"
#include "gameloop.h"
#include "FCheckpoint.h"
#include "collectable.h"
#include "fdatastreaming.h"
#include "letterbox.h"
#include "gstring.h"

// mini-games
#include "ColiseumMiniGame.h"
#include "ZombieBossGame.h"
#include "SpyVsSpy.h"
#include "GeneralCorrosiveGame.h"
#include "MG_FinalBattle.h"
#include "MG_HoldYourGround.h"

/////////////////////////////////////////
// private definitions:
#define _GENERIC_LEVEL_TITLE		"Generic"
#define _MAX_MUSIC_CHARS			( 31 )
#define _LEVEL_MUSIC_FADE_RATE		( 0.4f )	// rate at which music fades in units per second
#define _LEVEL_MUSIC_RAMP_RATE		( 0.4f )	// rate at which music ramps in units per second
#define _LEVEL_AUDIO_DUCK_TIME      ( 1.0f )	// how long it takes to achieve target duck volume levels
#define _MAX_STREAM_FILENAME		16
#define _DEFAULT_STARTING_FADE_SECS ( 0.5f )
#define _DEFAULT_ENDING_FADE_SECS	( 0.5f )


//Level Fog Data
typedef struct {
	f32 fRed;
	f32 fGreen;
	f32 fBlue;
	f32 fDensity;
	f32 fFogNearDepth;
	f32 fFogFarDepth;
	f32 fMinHeight;
	f32 fMaxHeight;
	char *pszTexture;
} UserPropDataFog_t;

const FGameData_TableEntry_t _aUserPropVocabFog[] = {
	// fRed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fGreen:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fBlue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fNearZ:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fFarZ:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMinHeight:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMaxHeight:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
/*
	// pszTexture:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
*/
	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

//Level reflection map
typedef struct {
	char *pszTexture;
} UserPropDataReflection_t;

const FGameData_TableEntry_t _aUserPropVocabReflection[] = {
	// pszTexture:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

typedef enum {
	_STREAM_MUSIC,
	_STREAM_SPEECH,

	_STREAM_COUNT
} _Stream_e;

typedef enum {
	_LEVEL_MUSIC_STATE_STOP = 0,
	_LEVEL_MUSIC_STATE_START_TRACK,
	_LEVEL_MUSIC_STATE_PLAYING,
	_LEVEL_MUSIC_STATE_FADE_OUT,
	_LEVEL_MUSIC_STATE_PAUSED, // for barterbot interaction : not sure YOU want to use this state
} _LevelMusicState_e;

typedef struct {
	char szNewMusic[ _MAX_MUSIC_CHARS + 1];
	char szCurrentMusic[ _MAX_MUSIC_CHARS + 1];
	char szDefaultTrack[ _MAX_MUSIC_CHARS + 1];
	BOOL8 bStopMusic;
	BOOL8 bDefaultTrackIsMusic;
	BOOL8 bPlayMusicWithFadeIn;
	BOOL8 bPlayNewTrackAsMusic;
	
	BOOL8 bPauseMusic;
	u8  nMusicState;		// _LevelMusicState_e
	s16 nMusicIndex;
	
	f32 fMusicVolume;		//the current volume level of the music
	f32 fMaxMusicVolume;	//the loudest this stream will play
	
	// fade vars
	f32 fTargetFadeToVolume;
	f32 fCurrentFadeToVolume;
	f32 fFadeToVolumeFadeTime;
} _LevelMusicData_t;

typedef struct {
	s32 nDuckCount;
	f32 fDuckVolume;
} _LevelDuckData_t;

typedef struct {
	CFAudioStream *pAudioStream;
	BOOL8 bWaitForCreate;
	BOOL8 bPlayAsMusic;
	u16 nNumLoops;
	f32 fVolume;
	char szFilename[_MAX_STREAM_FILENAME];	
} _StreamingAudioInfo_t;

typedef BOOL _InitLevelFcn_t( void );
typedef void _UninitLevelFcn_t( void );

typedef struct {
	_InitLevelFcn_t *pFcnInitLevel;
	_UninitLevelFcn_t *pFcnUninitLevel;
} _LevelFunctions_t;

/////////////////////////////////////////
// public vars:
s32 Level_nCount;
s32 Level_nLoadedIndex;
FGameDataFileHandle_t Level_hLevelDataFile = FGAMEDATA_INVALID_FILE_HANDLE;
FGameDataTableHandle_t Level_hStreamingAudioTable = FGAMEDATA_INVALID_TABLE_HANDLE;
FGameDataTableHandle_t Level_hInventoryTable = FGAMEDATA_INVALID_TABLE_HANDLE;
u32 Level_nNumStreamingAudioEntries = 0;
f32 Level_fSecsInLevel;
f32 Level_fStartingFadeSecs;
f32 Level_fEndingFadeSecs;

Level_t Level_aInfo[] = {
/////////////////////////////////////////////////////////////////////////////////////
// DEVELOPMENT LEVELS
/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////
	// a generic entry to get level skip working
	// THIS ENTRY SHOULD ALWAYS BE FIRST
	_GENERIC_LEVEL_TITLE,			// pszTitle
	NULL,							// pszWorldResName (will be specified by the user)
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"Level01",						// pszCSVFile
	"ms_gtest_01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////////////
	// Level 0A: Test Bed - Jump Test
	"1 Test",						// pszTitle
	"jump_test",					// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"JumpTest",						// pszCSVFile
	"ms_test_01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	50.0f,							// fProjOffs
	4.0f,							// fProjRngAdj
	#endif
	////////////////////////////////
	// Level 0B: Test Bed - Ramptest
	"2 Test",						// pszTitle
	"RampTest4",					// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"RampTest2",					// pszCSVFile
	"ms_test_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////////////
	// Level 0C: Test Bed - Door AI Test
	"3 Test",						// pszTitle
	"doorai_test",					// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"doorai_test",					// pszCSVFile
	"ms_test_03",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Level 0D: Test Bed - Dave
	"4 Test",						// pszTitle
	"testlevel01",					// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"testlevel01",					// pszCSVFile
	"ms_test_04",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Level 0E: Test Bed - John's
	"5 Test",						// pszTitle
	"WEGT_test01",					// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"testlevel05",					// pszCSVFile
	"ms_test_05",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////
	// Level 0a: Hall o Bots
	"7 Hall o Bots",				// pszTitle
	"hall_o_bots",					// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"hall_o_bots",					// pszCSVFile
	"ms_hall_01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////
	// Level 0b: Hall o Bots
	"8 Hall o Bots",				// pszTitle
	"hall_o_bot2",					// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"hall_o_bot2",					// pszCSVFile
	"ms_hall_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////
	// Level 0D: Temp
	"9 Temp.wld",					// pszTitle
	"temp",							// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"Level01",						// pszCSVFile
	"ms_temp_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Level cine 1
	"10 Cinematic Slot",			// pszTitle
	"wc00cine",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"wc00cine",						// pszCSVFile
	"ms_temp_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Level cine 2
	"11 Cinematic Slot",			// pszTitle
	"wc01cine",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"wc01cine",						// pszCSVFile
	"ms_temp_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Level cine 3
	"12 Cinematic Slot",			// pszTitle
	"wc02cine",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"wc02cine",						// pszCSVFile
	"ms_temp_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Level cine 4
	"13 Cinematic Slot",			// pszTitle
	"wc03cine",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"wc03cine",						// pszCSVFile
	"ms_temp_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Level cine 5
	"14 Cinematic Slot",			// pszTitle
	"wc04cine",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"wc04cine",						// pszCSVFile
	"ms_temp_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
/////////////////////////////////////////////////////////////////////////////////////
// E3/DEMO LEVELS
/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////
	// Level 0A: E3 level
	"15 E3 Levels",					// pszTitle
	"WE_E3_01",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"WE_E3_01",						// pszCSVFile
	"MS_E3_01",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Level 0B: E3 level
	"16 E3 Levels",					// pszTitle
	"WE_E3_02",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"WE_E3_02",						// pszCSVFile
	"MS_E3_02",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Level 0C: E3 level
	"17 E3 Levels",					// pszTitle
	"WE_E3_03",						// pszWorldResName
	LEVEL_DEVELOPMENT_LEVEL,		// nLevel
	"WE_E3_03",						// pszCSVFile
	"MS_E3_03",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw	
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
/////////////////////////////////////////////////////////////////////////////////////
// SINGLE PLAYER LEVELS
/////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////
	// Seal the Mines 1
	"1 Seal the Mines",			// pszTitle
	"WEDMmines01",					// pszWorldResName
	LEVEL_SEAL_THE_MINES_1,			// nLevel
	"WEDMmines01",					// pszCSVFile
	"ms_mines01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Seal the Mines 2
	"2 Seal the Mines",			// pszTitle
	"WEDMmines02",					// pszWorldResName
	LEVEL_SEAL_THE_MINES_2,			// nLevel
	"WEDMmines02",					// pszCSVFile
	"ms_mines02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Seal the Mines 3 
	"3 Seal the Mines",			// pszTitle
	"WEDMmines03",					// pszWorldResName
	LEVEL_SEAL_THE_MINES_3,			// nLevel
	"WEDMmines03",					// pszCSVFile
	"ms_mines03",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	10.0f,							// fProjOffs
	4.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Clean Up
	"4 Clean Up",					// pszTitle
	"WEDTtown_01",					// pszWorldResName
	LEVEL_CLEAN_UP,					// nLevel
	"WEDTtown_01",					// pszCSVFile
	"ms_town_01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// RAT race
	"5 RAT race",					// pszTitle
	"WEWTrace_01",					// pszWorldResName
	LEVEL_RAT_RACE,					// nLevel
	"WEWTrace_01",					// pszCSVFile
	"ms_race_01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	50.0f,							// fProjOffs
	4.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// Wasteland Journey 1
	"6 Wasteland Journey",			// pszTitle
	"WEWJjourn01",					// pszWorldResName
	LEVEL_WASTELAND_JOURNEY_1,		// nLevel
	"WEWJjourn01",					// pszCSVFile
	"ms_journ01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif	//////////////////////////////
	// Wasteland Journey 2
	"7 Wasteland Journey",			// pszTitle
	"WEWJjourn02",					// pszWorldResName
	LEVEL_WASTELAND_JOURNEY_2,		// nLevel
	"WEWJjourn02",					// pszCSVFile
	"ms_journ02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	10.0f,							// fProjOffs
	4.0f,							// fProjRngAdj
	#endif	
	//////////////////////////////
	// Wasteland Journey 3
	"8 Wasteland Journey",			// pszTitle
	"WEWJjourn03",					// pszWorldResName
	LEVEL_WASTELAND_JOURNEY_3,		// nLevel
	"WEWJjourn03",					// pszCSVFile
	"ms_journ03",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// ZombieBot Boss
	"9 ZombieBot Boss",			// pszTitle
	"WEWZzombi01",					// pszWorldResName
	LEVEL_ZOMBIEBOT_BOSS,			// nLevel
	"WEWZzombi01",					// pszCSVFile
	"ms_zombi01",					// pszMaterialCSVFile
	CZombieBossGame::LoadLevel,		// pFcnLoad
	CZombieBossGame::UnloadLevel,	// pFcnUnload
	CZombieBossGame::Work,			// pFcnWork
	CZombieBossGame::Draw,			// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////////////////////////
	// Destroy the Communications Array
	"10 Destroy Comm Cntr",			// pszTitle
	"WEWCcomm_01",					// pszWorldResName
	LEVEL_DESTROY_THE_COMMUNICATIONS_ARRAY_1,// nLevel
	"WEWCcomm_01",					// pszCSVFile
	"ms_comm_01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////////////////////////
	// Destroy the Communications Array 1
	"11 Destroy Comm Cntr",			// pszTitle
	"WEWCcomm_02",					// pszWorldResName
	LEVEL_DESTROY_THE_COMMUNICATIONS_ARRAY_2,// nLevel
	"WEWCcomm_02",					// pszCSVFile
	"ms_comm_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////////////////////////
	// Destroy the Communications Array 2
	"12 Destroy Comm Cntr",			// pszTitle
	"WEWCcomm_03",					// pszWorldResName
	LEVEL_DESTROY_THE_COMMUNICATIONS_ARRAY_3,// nLevel
	"WEWCcomm_03",					// pszCSVFile
	"ms_comm_03",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Hold Your Ground
	"13 Hold Your Ground",			// pszTitle
	"WEWChold_01",					// pszWorldResName
	LEVEL_HOLD_YOUR_GROUND,			// nLevel
	"WEWChold_01",					// pszCSVFile
	"ms_hold_01",					// pszMaterialCSVFile
	mg_holdyourground_LevelLoad,	// pFcnLoad
	mg_holdyourground_LevelUnload,	// pFcnUnload
	mg_holdyourground_LevelWork,	// pFcnWork
	mg_holdyourground_LevelDraw,	// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// Research Facility 1
	"14 R & D",						// pszTitle
	"WEWRresrch1",					// pszWorldResName
	LEVEL_RESEARCH_FACILITY_1,		// nLevel
	"WEWRresrch1",					// pszCSVFile
	"ms_resrch1",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// Research Facility 2
	"15 R & D",						// pszTitle
	"WEWRresrch2",					// pszWorldResName
	LEVEL_RESEARCH_FACILITY_2,		// nLevel
	"WEWRresrch2",					// pszCSVFile
	"ms_resrch2",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// Research Facility 3
	"16 R & D",						// pszTitle
	"WEWRresrch3",					// pszWorldResName
	LEVEL_RESEARCH_FACILITY_3,		// nLevel
	"WEWRresrch3",					// pszCSVFile
	"ms_resrch3",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// Research Facility 3
	"17 R & D",						// pszTitle
	"WEWRresrch4",					// pszWorldResName
	LEVEL_RESEARCH_FACILITY_4,		// nLevel
	"WEWRresrch4",					// pszCSVFile
	"ms_resrch4",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////////
	// Wasteland Chase
	"18 Wasteland Chase",			// pszTitle
	"WEWHchase01",					// pszWorldResName
	LEVEL_WASTELAND_CHASE,			// nLevel
	"WEWHchase01",					// pszCSVFile
	"ms_chase01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
    ///////////////////////////
	// Morbot Region 1
	"19 Morbot Region",				// pszTitle
	"WERMmorbot1",					// pszWorldResName
	LEVEL_MORBOT_REGION_1,			// nLevel
	"WERMmorbot1",					// pszCSVFile
	"ms_morbot1",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	10.0f,							// fProjOffs
	4.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Morbot Region 2
	"20 Morbot Region",				// pszTitle
	"WERMmorbot2",					// pszWorldResName
	LEVEL_MORBOT_REGION_2,			// nLevel
	"WERMmorbot2",					// pszCSVFile
	"ms_morbot2",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////
	// Reactor 1
	"21 Reactor",					// pszTitle
	"WERRreactr1",					// pszWorldResName
	LEVEL_REACTOR_1,				// nLevel
	"WERRreactr1",					// pszCSVFile
	"ms_reactr1",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	10.0f,							// fProjOffs
	4.0f,							// fProjRngAdj
	#endif
	//////////////////////
	// Reactor 2
	"22 Reactor",					// pszTitle
	"WERRreactr2",					// pszWorldResName
	LEVEL_REACTOR_2,				// nLevel
	"WERRreactr2",					// pszCSVFile
	"ms_reactr2",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////////
	// Mill City Hub 1
	"23 Mil City Hub",				// pszTitle
	"WEMCcity_01",					// pszWorldResName
	LEVEL_MIL_CITY_HUB,				// nLevel
	"WEMCcity_01",					// pszCSVFile
	"ms_city_01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////////
	// Mill City Hub 2
	"24 Mil City Hub",				// pszTitle
	"WEMCcity_03",					// pszWorldResName
	LEVEL_FIND_WRENCH,				// nLevel
	"WEMCcity_03",					// pszCSVFile
	"ms_city_03",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////
	// Spy vs. Spy
	"25 Spy vs. Spy",				// pszTitle
	"WECFfacty01",					// pszWorldResName
	LEVEL_SPY_VS_SPY,				// nLevel
	"WECFfacty01",					// pszCSVFile
	"ms_facty01",					// pszMaterialCSVFile
	CSpyVsSpy::LoadLevel,			// pFcnLoad
	CSpyVsSpy::UnloadLevel,			// pFcnUnload
	CSpyVsSpy::Work,				// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Ruins 1
	"26 Ruins",						// pszTitle
	"WEMCcity_05",					// pszWorldResName
	LEVEL_ACCESS_THE_RUINS,			// nLevel
	"WEMCcity_05",					// pszCSVFile
	"ms_city_05",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Ruins 2
	"27 Ruins",						// pszTitle
	"WECRruins01",					// pszWorldResName
	LEVEL_RUINS_1,					// nLevel
	"WECRruins01",					// pszCSVFile
	"ms_ruins01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////
	// Ruins 3
	"28 Ruins",						// pszTitle
	"WECRruins02",					// pszWorldResName
	LEVEL_RUINS_2,					// nLevel
	"WECRruins02",					// pszCSVFile
	"ms_ruins02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// Secret Rendezvous
	"29 Secret Rendezvous",			// pszTitle
	"WEMCcity_02",					// pszWorldResName
	LEVEL_SECRET_RENDEZVOUS,		// nLevel
	"WEMCcity_02",					// pszCSVFile
	"ms_city_02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////
	// Night Sneak 1
	"30 Night Sneak",				// pszTitle
	"WECDsneak01",					// pszWorldResName
	LEVEL_NIGHT_SNEAK_1,			// nLevel
	"WECDsneak01",					// pszCSVFile
	"ms_sneak01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////
	// Night Sneak 2
	"31 Night Sneak",				// pszTitle
	"WECDsneak02",					// pszWorldResName
	LEVEL_NIGHT_SNEAK_2,			// nLevel
	"WECDsneak02",					// pszCSVFile
	"ms_sneak02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	////////////////////////
	// Invasion
	"32 Invasion",					// pszTitle
	"WEDiinvas01",					// pszWorldResName
	LEVEL_INVASION_1,				// nLevel
	"WEDiinvas01",					// pszCSVFile
	"ms_invas01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////
	// Coliseum 1
	"33 Coliseum",					// pszTitle
	"WEBCcolis01",					// pszWorldResName
	LEVEL_COLISEUM_1,				// nLevel
	"WEBCcolis01",					// pszCSVFile
	"ms_colis01",					// pszMaterialCSVFile
	CColiseumMiniGame::LoadLevel1,	// pFcnLoad
	CColiseumMiniGame::UnloadLevel,	// pFcnUnload
	CColiseumMiniGame::Work,		// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////
	// Coliseum 2
	"34 Coliseum",					// pszTitle
	"WEBCcolis02",					// pszWorldResName
	LEVEL_COLISEUM_2,				// nLevel
	"WEBCcolis02",					// pszCSVFile
	"ms_colis02",					// pszMaterialCSVFile
	CColiseumMiniGame::LoadLevel2,	// pFcnLoad
	CColiseumMiniGame::UnloadLevel,	// pFcnUnload
	CColiseumMiniGame::Work,		// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////
	// Coliseum 3
	"35 Coliseum",					// pszTitle
	"WEBCcolis03",					// pszWorldResName
	LEVEL_COLISEUM_3,				// nLevel
	"WEBCcolis03",					// pszCSVFile
	"ms_colis03",					// pszMaterialCSVFile
	CColiseumMiniGame::LoadLevel3,	// pFcnLoad
	CColiseumMiniGame::UnloadLevel,	// pFcnUnload
	CColiseumMiniGame::Work,		// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////
	// Coliseum 4
	"36 Coliseum",					// pszTitle
	"WEBCcolis04",					// pszWorldResName
	LEVEL_COLISEUM_4,				// nLevel
	"WEBCcolis04",					// pszCSVFile
	"ms_colis04",					// pszMaterialCSVFile
	CColiseumMiniGame::LoadLevel4,	// pFcnLoad
	CColiseumMiniGame::UnloadLevel,	// pFcnUnload
	CColiseumMiniGame::Work,		// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////////
	// Race to the Rocket
	"37 Race to the Rocket",		// pszTitle
	"WEWKrockt01",					// pszWorldResName
	LEVEL_RACE_TO_THE_ROCKET,		// nLevel
	"WEWKrockt01",					// pszCSVFile
	"ms_rockt01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////
	// Space Dock
	"38 Space Dock",				// pszTitle
	"WESHhangr01",					// pszWorldResName
	LEVEL_SPACE_DOCK,				// nLevel
	"WESHhangr01",					// pszCSVFile
	"ms_hangr01",					// pszMaterialCSVFile
	CSpaceDock::InitLevel,			// pFcnLoad
	CSpaceDock::UninitLevel,		// pFcnUnload
	CSpaceDock::Work,				// pFcnWork
	CSpaceDock::Draw,				// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Space Station 1
	"39 Space Station",				// pszTitle
	"WESSstatn01",					// pszWorldResName
	LEVEL_SPACE_STATION_1,			// nLevel
	"WESSstatn01",					// pszCSVFile
	"ms_statn01",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Space Station 2
	"40 Space Station",				// pszTitle
	"WESSstatn02",					// pszWorldResName
	LEVEL_SPACE_STATION_2,			// nLevel
	"WESSstatn02",					// pszCSVFile
	"ms_statn02",					// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork 
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	//////////////////////////////
	// General Corrosive
	"41 Gen. Corrosive",				// pszTitle
	"WESRrepair1",						// pszWorldResName
	LEVEL_GENERAL_CORROSIVE,			// nLevel
	"WESRrepair1",						// pszCSVFile
	"ms_repair1",						// pszMaterialCSVFile
	CGeneralCorrosiveGame::LoadLevel,	// pFcnLoad
	CGeneralCorrosiveGame::UnloadLevel,	// pFcnUnload
	CGeneralCorrosiveGame::Work,		// pFcnWork
	NULL,//CGeneralCorrosiveGame::Draw,	// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	/////////////////////////
	// Final Battle
	"42 Final Battle",				// pszTitle
	"WESCcorros1",					// pszWorldResName
	LEVEL_FINAL_BATTLE,				// nLevel
	"WESCcorros1",					// pszCSVFile
	"ms_corros1",					// pszMaterialCSVFile
	CMGFinalBattle::LoadLevel,		// pFcnLoad
	CMGFinalBattle::UnloadLevel,	// pFcnUnload
	CMGFinalBattle::Work,			// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
/////////////////////////////////////////////////////////////////////////////////////
// MULTIPLAYER LEVELS
/////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////
	// Level MP1: Multiplayer 1
	"1 MP Level 1",					// pszTitle
	"WE01multi01",					// pszWorldResName
	LEVEL_MULTIPLAYER_1,			// nLevel
	"WE01multi01",					// pszCSVFile
	"ms_mp_01",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP2: Multiplayer 2
	"2 MP Level 2",					// pszTitle
	"WE02multi02",					// pszWorldResName
	LEVEL_MULTIPLAYER_2,			// nLevel
	"WE02multi02",					// pszCSVFile
	"ms_mp_02",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP3: Multiplayer 3
	"3 MP Level 3",					// pszTitle
	"WE03multi03",					// pszWorldResName
	LEVEL_MULTIPLAYER_3,			// nLevel
	"WE03multi03",					// pszCSVFile
	"ms_mp_03",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP4: Multiplayer 4
	"4 MP Level 4",					// pszTitle
	"WE04multi04",					// pszWorldResName
	LEVEL_MULTIPLAYER_4,			// nLevel
	"WE04multi04",					// pszCSVFile
	"ms_mp_04",						// pszMaterialCSVFile
	CColiseumMiniGame::LoadMP,		// pFcnLoad
	CColiseumMiniGame::UnloadMP,	// pFcnUnload
	CColiseumMiniGame::WorkMP,		// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP5: Multiplayer 5
	"5 MP Level 5",					// pszTitle
	"WE05multi05",					// pszWorldResName
	LEVEL_MULTIPLAYER_5,			// nLevel
	"WE05multi05",					// pszCSVFile
	"ms_mp_05",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP6: Multiplayer 6
	"6 MP Level 6",					// pszTitle
	"WE06multi06",					// pszWorldResName
	LEVEL_MULTIPLAYER_6,			// nLevel
	"WE06multi06",					// pszCSVFile
	"ms_mp_06",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP7: Multiplayer 7
	"7 MP Level 7",					// pszTitle
	"WE07multi07",					// pszWorldResName
	LEVEL_MULTIPLAYER_7,			// nLevel
	"WE07multi07",					// pszCSVFile
	"ms_mp_07",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP8: Multiplayer 8
	"8 MP Level 8",					// pszTitle
	"WE08multi08",					// pszWorldResName
	LEVEL_MULTIPLAYER_8,			// nLevel
	"WE08multi08",					// pszCSVFile
	"ms_mp_08",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP9: Multiplayer 9
	"9 MP Level 9",					// pszTitle
	"WE09multi09",					// pszWorldResName
	LEVEL_MULTIPLAYER_9,			// nLevel
	"WE09multi09",					// pszCSVFile
	"ms_mp_09",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP10: Multiplayer 10
	"10 MP Level 10",				// pszTitle
	"WE10multi10",					// pszWorldResName
	LEVEL_MULTIPLAYER_10,			// nLevel
	"WE10multi10",					// pszCSVFile
	"ms_mp_10",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP11: Multiplayer 11
	"11 MP Level 11",				// pszTitle
	"WE11multi11",					// pszWorldResName
	LEVEL_MULTIPLAYER_11,			// nLevel
	"WE11multi11",					// pszCSVFile
	"ms_mp_11",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP12: Multiplayer 12
	"12 MP Level 12",				// pszTitle
	"WE12multi12",					// pszWorldResName
	LEVEL_MULTIPLAYER_12,			// nLevel
	"WE12multi12",					// pszCSVFile
	"ms_mp_12",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP13: Multiplayer 13
	"13 MP Level 13",				// pszTitle
	"WE13multi13",					// pszWorldResName
	LEVEL_MULTIPLAYER_13,			// nLevel
	"WE13multi13",					// pszCSVFile
	"ms_mp_13",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP14: Multiplayer 14
	"14 MP Level 14",				// pszTitle
	"WE14multi14",					// pszWorldResName
	LEVEL_MULTIPLAYER_14,			// nLevel
	"WE14multi14",					// pszCSVFile
	"ms_mp_14",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	///////////////////////////
	// Level MP15: Multiplayer 15
	"15 MP Level 15",				// pszTitle
	"WE15multi15",					// pszWorldResName
	LEVEL_MULTIPLAYER_15,			// nLevel
	"WE15multi15",					// pszCSVFile
	"ms_mp_15",						// pszMaterialCSVFile
	NULL,							// pFcnLoad
	NULL,							// pFcnUnload
	NULL,							// pFcnWork
	NULL,							// pFcnDraw
	#if FANG_PLATFORM_GC
	0.0f,							// fProjOffs
	1.0f,							// fProjRngAdj
	#endif
	// MUST BE NULL TERMINATED
	NULL,	
};

static const _LevelFunctions_t _aLevelFunctionArray[] = {
	CEProj_Cleaner::InitLevel,		CEProj_Cleaner::UnInitLevel,
	NULL,							NULL
};

/////////////////////////////////////////
// private vars:

static BOOL _bSystemInitialized = FALSE;
static FResFrame_t _ResFrameWorld;
static char _szTempResName[16];

static _LevelMusicData_t _LMData;
static _LevelDuckData_t _LDData;
static _StreamingAudioInfo_t _aStreamingAudioInfo[_STREAM_COUNT];
static cutscene_Handle_t _hIntroMovie;
static BOOL _bPlayIntroMovie;
static cwchar **_ppwszMissionText;
static u8 _nMissionTextCount;	// Number of mission text entries for the currently-loaded level
static u8 _nMissionTextIndex;	// Index of the currently-displayed mission text

////////////////////////////////////////
// private prototypes:
static BOOL _CallInitLevelFunctions( void );
static void _CallUninitLevelFunctions( void );
static void _LoadSfxBanks( void );
static void _LoadMusicData( void );
static void _LoadMovieData( void );
static void _LoadWireData( void );
static void _LoadMissionText( void );
static void _LoadBotTalkData( void );
static void _LoadCollectableOverrideCSV( void );
static void _InitAndPlayFirstMusic( void );
static void _MusicWork( void );
static void _StopStream( _Stream_e nStreamIndex );
BOOL		_level_IsPlaying (_Stream_e eWhichStream);
//////////////////////////////////////
// public functions:

BOOL level_InitSystem( void ) {
	u32 i;

    FASSERT_MSG( !_bSystemInitialized, "level_InitSystem(): Cannot call this function more than once.\n" );

	// Count the number of levels in the table...
	for( Level_nCount=0; Level_aInfo[Level_nCount].pszTitle; Level_nCount++ ) {}

	Level_nLoadedIndex = -1;
	_bSystemInitialized = TRUE;

	Level_aInfo[0].pszWorldResName = _szTempResName;

	Level_hLevelDataFile = FGAMEDATA_INVALID_FILE_HANDLE;
	Level_hStreamingAudioTable = FGAMEDATA_INVALID_TABLE_HANDLE;
	Level_hInventoryTable = FGAMEDATA_INVALID_TABLE_HANDLE;
	Level_nNumStreamingAudioEntries = 0;
	for( i=0; i < _STREAM_COUNT; i++ ) {
		_aStreamingAudioInfo[i].pAudioStream = NULL;
		_aStreamingAudioInfo[i].fVolume = 1.0f;
		_aStreamingAudioInfo[i].nNumLoops = 0;
	}

	_hIntroMovie = CUTSCENE_INVALID_HANDLE;
	_bPlayIntroMovie = FALSE;

	Level_fStartingFadeSecs = _DEFAULT_STARTING_FADE_SECS;
	Level_fEndingFadeSecs = _DEFAULT_ENDING_FADE_SECS;

	_ppwszMissionText = NULL;
	_nMissionTextCount = 0;
	_nMissionTextIndex = 0;

	return TRUE;
}


void level_UninitSystem( void ) {
	if( _bSystemInitialized ) {
		_bSystemInitialized = FALSE;
	}
}


BOOL level_IsInitialized( void ) {
	return _bSystemInitialized;
}


// Returns the index of the level with the matching title.
// Returns -1 if the title could not be found.
// The search is not case sensative.
s32 level_Find( cchar *pszTitle ) {
	s32 i;

	FASSERT( _bSystemInitialized );

	for( i=0; i<Level_nCount; i++ ) {
		if( !fclib_stricmp( pszTitle, Level_aInfo[i].pszTitle ) ) {
			return i;
		}
	}

	return -1;
}


// Returns the index of the level with the the matching level id
// Returns -1 if it can't be found.
s32 level_Find( Level_e nLevel ) {
	s32 i;

	FASSERT( _bSystemInitialized );

	for( i=0; i<Level_nCount; i++ ) {
		if( Level_aInfo[i].nLevel == nLevel ) {
			return i;
		}
	}

	return -1;
}


// Call to load the specified level.
// Returns TRUE if successful, or FALSE otherwise.
// The title is not case sensative.
#if FANG_PLATFORM_XB && FANG_TEST_BUILD
extern cchar* fang_pszDevPrintLogFileName;
char _szErrLogFileName[128];

void InitDevPrintLogFile(cchar* pszFileName)
{
	u32 uFileNameLen = fclib_strlen(pszFileName);
	if (uFileNameLen < 115)
	{
		fclib_strcpy(_szErrLogFileName, "D:\\LOG_");
		fclib_strcat(_szErrLogFileName, pszFileName);
		fclib_strcat(_szErrLogFileName, "00.txt");
		uFileNameLen = fclib_strlen(_szErrLogFileName);
		u32 uCount = 0;
		FILE* fpTmp = NULL;
		while ((fpTmp = fopen(_szErrLogFileName,"r")) && uCount < 99)  //99 is infinite loop safety
		{
			fclose(fpTmp);
			uCount++;
			_szErrLogFileName[uFileNameLen-6] = '0' + (uCount%100/10);
			_szErrLogFileName[uFileNameLen-5] = '0' + (uCount%10)/1;
			_szErrLogFileName[uFileNameLen] = '\0';
		}
		FILE* fp;
		if (fp = fopen(_szErrLogFileName, "wt"))
		{
			fang_pszDevPrintLogFileName = _szErrLogFileName;
			fclose(fp);
		}
	}


}
#else

void InitDevPrintLogFile(cchar* pszFileName)
{
}

#endif

BOOL level_Load( cchar *pszLevelTitle, BOOL bShowLoadingScreen, cwchar *pwszLoadScreenHeading/*=NULL*/ ) {
	FASSERT( _bSystemInitialized );
	FASSERT_MSG( Level_nLoadedIndex==-1, "level_Load(): Previous level must be unloaded first." );

	ffile_ResetTimer();

	f32 fStartSeconds = ftimer_Clock_GetSeconds();

	CFTimer LoadTimer;
	LoadTimer.Reset();

	_ResFrameWorld = fres_GetFrame();
	fres_AllocAndZero(fres_GetFreeBytes()-64);
	fres_ReleaseFrame(_ResFrameWorld);

	Level_nLoadedIndex = level_Find( pszLevelTitle );
	if( Level_nLoadedIndex == -1 ) {
		DEVPRINTF( "level_Load(): Level '%s' not found.\n", pszLevelTitle );
		goto _LevelLoadError;
	}

	//In certain debug versions and platforms, this will cause all devprints to be logged in a file named by level
	InitDevPrintLogFile(Level_aInfo[Level_nLoadedIndex].pszWorldResName);


	CGColl::LoadMaterialTable( Level_aInfo[Level_nLoadedIndex].pszMaterialCSVFile );

	DEVPRINTF( "---TIMING: level_Load() - LoadMaterialTable() time: %f\n", LoadTimer.SampleSeconds( TRUE ) );

	// load the level specific csv file into fres mem (do this before the world is loaded)
	Level_hLevelDataFile = (FGameDataFileHandle_t)fresload_Load( FGAMEDATA_RESTYPE, Level_aInfo[Level_nLoadedIndex].pszCSVFile );

	DEVPRINTF( "---TIMING: level_Load() - Load level CSV time: %f\n", LoadTimer.SampleSeconds( TRUE ) );

	// This needs to be prior to the movie being played so that the movie can use the memory cache
	level_LoadMemoryCacheData( Level_hLevelDataFile );

		if( bShowLoadingScreen ) 
		{
//ARG - >>>>>
#if FANG_PLATFORM_PS2
//CPS 5.7.03 (temp loading screen) -->
			#include "wpr_drawutils.h"

			fvid_Swap();
			fvid_Begin();

			fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0 );

			fvid_End();
			
			static CFTexInst *_pBackgroundTexInst = fnew CFTexInst;
			_pBackgroundTexInst->SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, "loading00" ) );	

			static FViewport_t *_pViewportOrtho3D = fviewport_Create();
			fviewport_InitOrtho3D( _pViewportOrtho3D, 0.1f, 100.0f );

			fviewport_SetActive( _pViewportOrtho3D );
			FXfm_Identity.InitStackWithView();

			fvid_Swap();
			fvid_Begin();

			// draw fdraw objects
			frenderer_Push( FRENDERER_DRAW, NULL );
			// draw the background texture map
			wpr_drawutils_DrawTextureToScreen( TRUE,
								_pBackgroundTexInst,
								0.0f, -0.3f,
								2.5f,
								0.0f,
								_pViewportOrtho3D->HalfRes.x,
								_pViewportOrtho3D->HalfRes.y );
			// push the fdraw renderer off 
			frenderer_Pop();
			
			fdelete( _pBackgroundTexInst );

			fvid_End();

//<-- CPS 5.7.03 (temp loading screen)
			#elif FANG_PLATFORM_GC
//ARG - <<<<<
//ARG			#if FANG_PLATFORM_GC
				if( !loadingscreen_Init( "load620x340GC.bik", pwszLoadScreenHeading ) ) {
					goto _LevelLoadError;
				}
			#else
				if( !loadingscreen_Init( "load620x340.bik", pwszLoadScreenHeading ) ) {
					goto _LevelLoadError;
				}
			#endif
		}

		// This needs to happen *before* the world is loaded, because the entities being created may
	//   want to decode event names.
	loadingscreen_Update("Initing Fang Level scripts");

		if( !CFScriptSystem::LevelInit(Level_aInfo[Level_nLoadedIndex].pszCSVFile) ) {
			goto _LevelLoadError;
		}

		CFScriptSystem::SetMonitorsOn( Gameloop_bDrawDebugInfo );

	loadingscreen_Update("Initing MA Level scripts");

	LoadTimer.Reset();

		//same here
		CMAScriptTypes::InitLevel();

		//same here
		AlarmSys_InitLevel();

		// same again
		CSpawnSys::InitLevel();

	loadingscreen_Update("Initing talkSystem");
		//and, entites being created may want to load bta's and create instances
		CTalkSystem2::InitLevel();

	loadingscreen_Update("Initing AI system");

		// Init the AI  (needs to be done before any AI based Entites are created)
		if( !aimain_InitSystem(Level_aInfo[Level_nLoadedIndex].pszWorldResName) ) {
			DEVPRINTF( "level_Load(): Unable to init AI.\n" );
			goto _LevelLoadError;
		}

	loadingscreen_Update("Initing smoketrail system");
		if( !smoketrail_Allocate( 10, 200 ) ) {
			DEVPRINTF( "level_Load(): Unable to create smoketrail buffers.\n" );
			goto _LevelLoadError;
		}

	loadingscreen_Update("Loading SFX sound banks");

		// load the level sound fx banks
		LoadTimer.Reset();
		_LoadSfxBanks();
		DEVPRINTF( "---TIMING: level_Load() - _LoadSfxBanks(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

		// load the level streaming audio names
	loadingscreen_Update("Loading music data");
		_LoadMusicData();

		_LoadMovieData();

		_LoadWireData();

		_LoadMissionText();

		Level_hInventoryTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, LEVEL_INVENTORY_TABLE_NAME );

		loadingscreen_Update("Loading bot talk data");
		LoadTimer.Reset();
		_LoadBotTalkData();
		DEVPRINTF( "---TIMING: level_Load() - _LoadBotTalkData(): %f\n", LoadTimer.SampleSeconds( TRUE ) );

	loadingscreen_Update("Loading collectable override data");
		
		_LoadCollectableOverrideCSV();

		level_LoadFogData( Level_hLevelDataFile );

		level_LoadReflectionData( Level_hLevelDataFile );

		level_LoadFadeData( Level_hLevelDataFile );

	loadingscreen_Update("Loading WORLD!");

	LoadTimer.Reset();

	if( !fresload_Load( FWORLD_RESTYPE, Level_aInfo[Level_nLoadedIndex].pszWorldResName ) ) {
			// Could not load world...
			DEVPRINTF( "---TIMING: level_Load(): Unable to fresload_Load %s.\n", Level_aInfo[Level_nLoadedIndex].pszWorldResName );
			goto _LevelLoadError;
		}

		DEVPRINTF( "---TIMING: level_Load() - WLD load: %f\n", LoadTimer.SampleSeconds( TRUE ) );

		if( Level_aInfo[Level_nLoadedIndex].pFcnLoad ) {
			loadingscreen_Update("Loading pFcnLoad level loads!");
			if( !Level_aInfo[Level_nLoadedIndex].pFcnLoad( LEVEL_EVENT_POST_WORLD_LOAD ) ) {
				// Level loader function returned an error...
				goto _LevelLoadError;
			}
		}

	loadingscreen_Update("Calling InitLevelFunctions");
		if( !_CallInitLevelFunctions() ) {
			goto _LevelLoadError;
		}

		//set up the audio ducking variables here.
		_LDData.fDuckVolume = 1.0f;
		_LDData.nDuckCount = 0;

	loadingscreen_Update("Initializing and playing music");
		_InitAndPlayFirstMusic();

		//trigger the intro movie!
		level_PlayIntroMovie();

	loadingscreen_Update("Leaving level Load");

		CFAudioStream::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_NONE ); //let the audio system know that NO (or sounds) streams should be paused right now.
		CFAudioEmitter::SetGlobalPauseLevel( FAUDIO_PAUSE_LEVEL_NONE );
	
	//I need to do a per-level adjustment for the perspective texcoord bug on GC. See fgcsh.h "GC Hardware perspective texcoord bug" for a description.
#if FANG_PLATFORM_GC
	fgcsh_SetProjectorOffset(Level_aInfo[Level_nLoadedIndex].fProjOffs);
	fgcsh_SetProjectorRangeAdj(Level_aInfo[Level_nLoadedIndex].fProjRngAdj);
#endif

	// Tell FVis to block on the next render until the streaming
	// data is in the cache so that we don't end up missing a bunch of crap.
	fvis_BlockOnStreamingData();

	DEVPRINTF( "\n---TIMING: Total file system time: %f\n", ffile_GetTime() );
	DEVPRINTF( "---TIMING: Total level load time : %f\n", ftimer_Clock_GetSeconds() - fStartSeconds );

	// Success...
	return TRUE;

	// Error...
_LevelLoadError:
	// The alarm and spawn networks live in the frame released below; tear them down first, as
	// game_UnloadLevel() does before level_Unload(). Its later calls then find nothing to do.
	AlarmSys_UninitLevel();
	CSpawnSys::UninitLevel();
	CMAScriptTypes::UninitLevel();
	CTalkSystem2::UninitLevel();
	CFScriptSystem::LevelUninit();
	fres_ReleaseFrame( _ResFrameWorld );

	Level_nLoadedIndex = -1;

	return FALSE;
}


// Call to load the generic level using the specified world file.
// Returns TRUE if successful, or FALSE otherwise.
// The world res name is not case sensative.
BOOL level_LoadGenericLevel( cchar *pszWorldResName ) {
	FASSERT( _bSystemInitialized );
	FASSERT_MSG( Level_nLoadedIndex==-1, "level_Load(): Previous level must be unloaded first." );
	FASSERT( pszWorldResName );

	_ResFrameWorld = fres_GetFrame();

	// Copy the level resource name into the array.
	fclib_strcpy(_szTempResName, pszWorldResName);

	Level_nLoadedIndex = level_Find( _GENERIC_LEVEL_TITLE );
	if( Level_nLoadedIndex == -1 ) {
		DEVPRINTF( "level_Load(): The generic level is not supported.\n" );
		goto _LevelLoadError;
	}

	CGColl::LoadMaterialTable( Level_aInfo[Level_nLoadedIndex].pszMaterialCSVFile );

	// This needs to happen *before* the world is loaded, because the entities being created may
	//   want to decode event names.
	if( !CFScriptSystem::LevelInit(Level_aInfo[Level_nLoadedIndex].pszCSVFile) ) {
		goto _LevelLoadError;
	}
	//same here
	CMAScriptTypes::InitLevel();

	//same here
	AlarmSys_InitLevel();

	CSpawnSys::InitLevel();

	//and, entites being created may want to load bta's and create instances
	CTalkSystem2::InitLevel();

	// Init the AI  (needs to be done before any AI based Entites are created)
	if( !aimain_InitSystem(Level_aInfo[Level_nLoadedIndex].pszWorldResName) ) {
		DEVPRINTF( "level_Load(): Unable to init AI.\n" );
		goto _LevelLoadError;
	}

	// load the level specific csv file into fres mem (do this before the world is loaded)
	Level_hLevelDataFile = (FGameDataFileHandle_t)fresload_Load( FGAMEDATA_RESTYPE, Level_aInfo[Level_nLoadedIndex].pszCSVFile );

	// load the level sound fx banks
	_LoadSfxBanks();

	// load the level streaming audio names
	_LoadMusicData();

	_LoadMovieData();

	_LoadWireData();

	_LoadMissionText();

	Level_hInventoryTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, LEVEL_INVENTORY_TABLE_NAME );

	if( !fresload_Load( FWORLD_RESTYPE, pszWorldResName ) ) {
		// Could not load world...
		goto _LevelLoadError;
	}

	if( Level_aInfo[Level_nLoadedIndex].pFcnLoad ) {
		if( !Level_aInfo[Level_nLoadedIndex].pFcnLoad( LEVEL_EVENT_POST_WORLD_LOAD ) ) {
			// Level loader function returned an error...
			goto _LevelLoadError;
		}
	}

	if( !_CallInitLevelFunctions() ) {
		goto _LevelLoadError;
	}

	_InitAndPlayFirstMusic();
	
	// Success...
	return TRUE;

	// Error...
_LevelLoadError:
	// See level_Load().
	AlarmSys_UninitLevel();
	CSpawnSys::UninitLevel();
	CMAScriptTypes::UninitLevel();
	CTalkSystem2::UninitLevel();
	CFScriptSystem::LevelUninit();
	fres_ReleaseFrame( _ResFrameWorld );

	Level_nLoadedIndex = -1;

	return FALSE;	
}


// Unloads the current level and frees all of its resources.
void level_Unload( void ) {
	FASSERT( _bSystemInitialized );

	if( Level_nLoadedIndex != -1 ) {
		_CallUninitLevelFunctions();

		if( Level_aInfo[Level_nLoadedIndex].pFcnUnload ) {
			Level_aInfo[Level_nLoadedIndex].pFcnUnload();
		}

		FDS_StreamMgr.Uninit();

		CFScriptSystem::LevelUninit();

		CEntity::RemoveAndDestroyAll();

		fres_ReleaseFrame( _ResFrameWorld );
		fres_AllocAndZero(fres_GetFreeBytes()-64);
		fres_ReleaseFrame(_ResFrameWorld);

		// kill all particles that may still be in the work and didn't get killed from the frame release
		fparticle_KillAllEmitters();


		Level_nLoadedIndex = -1;

		Level_hLevelDataFile = FGAMEDATA_INVALID_FILE_HANDLE;

		//stop any sounds that might still be playing.
		//CFAudioEmitter::DestroyAll();  //Currently problematic
		level_StopAllStreams();
		Level_hStreamingAudioTable = FGAMEDATA_INVALID_TABLE_HANDLE;
		Level_hInventoryTable = FGAMEDATA_INVALID_TABLE_HANDLE;
		Level_nNumStreamingAudioEntries = 0;

	//	CEZipLine::Destroy();

		CGColl::ClearMaterialTable();

		_hIntroMovie = CUTSCENE_INVALID_HANDLE;
	}
}


void level_Work( void ) {
	FASSERT( _bSystemInitialized );

	if( Level_nLoadedIndex != -1 ) {
		FASSERT( Level_nLoadedIndex>=0 && Level_nLoadedIndex<Level_nCount );

		// increase the level timer (only if not viewing a cutscene)
		if( letterbox_GetUnitSlideOnAmount() == 0.0f ) {
            Level_fSecsInLevel += FLoop_fPreviousLoopSecs;
		}

		if( Level_aInfo[Level_nLoadedIndex].pFcnWork ) {
			Level_aInfo[Level_nLoadedIndex].pFcnWork();
		}

		//check to see if the level intro movie needs to be played.
		if( _bPlayIntroMovie ) {
			//play the level intro cutscene
#if FANG_WINGC
			// most levels have no intro movie; asking for one logged "invalid cutscene handle" each load
			if( _hIntroMovie != CUTSCENE_INVALID_HANDLE )
#endif
			cutscene_Start( _hIntroMovie, 1.0f );

			_bPlayIntroMovie = FALSE;
			return;
		}

		_MusicWork();		
	}
}


void level_Draw( void ) {
	FASSERT( _bSystemInitialized );

	if( Level_nLoadedIndex != -1 ) {
		FASSERT( Level_nLoadedIndex>=0 && Level_nLoadedIndex<Level_nCount );

		if( Level_aInfo[Level_nLoadedIndex].pFcnDraw ) {
			Level_aInfo[Level_nLoadedIndex].pFcnDraw();
		}
	}
}


static BOOL _CallInitLevelFunctions( void ) {
	s32 i;

	Level_fSecsInLevel = 0.0f;

	// Init game levels...
	for( i=0; _aLevelFunctionArray[i].pFcnInitLevel; i++ ) {
	   loadingscreen_Update();
		if( !_aLevelFunctionArray[i].pFcnInitLevel() ) {
			// Error initializing a game level.
			// Uninit all levels we've already initialized in reverse order...
			for( i--; i >= 0; i-- ) {
				_aLevelFunctionArray[i].pFcnUninitLevel();
			}

			// Return back error...
			return FALSE;
		}
	}

	// All game levels initialized successfully...
	return TRUE;
}


static void _CallUninitLevelFunctions( void ) {
	s32 i;

	// Count the number of entries in _aLevelFunctionArray[]...
	for( i=0; _aLevelFunctionArray[i].pFcnInitLevel; i++ ) {}

	// Uninit game levels...
	for( i--; i>=0; i-- ) {
		_aLevelFunctionArray[i].pFcnUninitLevel();
	}
}


static void _LoadSfxBanks( void ) {

	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}

	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "sndfx_banks" );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return;
	}
	
	u32 nNumFields = fgamedata_GetNumFields( hTable );
	u32 i;
	cchar *pszBank2Load;
	FGameData_VarType_e nVarType;

	for( i=0; i < nNumFields; i++ ) {
		pszBank2Load = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nVarType );
		if( !pszBank2Load || 
			(nVarType != FGAMEDATA_VAR_TYPE_STRING) ) {
			DEVPRINTF( "Level::_LoadSfxBanks(): Entry %d of the sndfx_banks table is invalid, skipping.\n", i );
			continue;
		}
		if( !fresload_Load( FSNDFX_RESTYPE, pszBank2Load ) ) {
			DEVPRINTF( "Level::_LoadSfxBanks(): Trouble loading the level sfx bank '%s'.\n", pszBank2Load );
		}
	}	
}

static void _LoadMovieData( void ) {
	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}

	FGameDataFileHandle_t hMovieTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "StartMovie" );
	if( hMovieTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return;
	}

	int nNumEntries = fgamedata_GetNumFields( hMovieTable );
	if( nNumEntries == 0) {
		DEVPRINTF( "Level::_LoadMovieData(): StartMovie table declared but zero movie entries in table!\n" );
		return;
	}

	if( nNumEntries > 1 ) {
		DEVPRINTF( "Level::_LoadMovieData(): More than one entry in the StartMovie table.  Ignoring all but first entry!\n" );
	}

	FGameData_VarType_e nVarType;
	cchar *pszFilename;

	// get the first entry in the table and create a cutscene handle for it.
	// use this handle as the intro movie handle.
	pszFilename = (cchar *)fgamedata_GetPtrToFieldData( hMovieTable, 0, nVarType );
	if( !pszFilename || (nVarType != FGAMEDATA_VAR_TYPE_STRING) ) {
		DEVPRINTF( "Level::_LoadMovieData(): StartMovie table entry is invalid.  Ignoring Table.\n" );
		return;
	}		

	//we have a valid filename, get a handle for the cutscene id...
	cutscene_Handle_t hIntroHandle = cutscene_AcquireHandle( pszFilename, TRUE );
	level_SetIntroMovieHandle( hIntroHandle );
}


static void _LoadWireData( void ) {
	// Set default wire pool count...
	CEntity::SetWirePoolCount();

	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}

	FGameDataFileHandle_t hWireTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "WireCount" );
	if( hWireTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return;
	}

	u32 nFieldCount = fgamedata_GetNumFields( hWireTable );
	if( nFieldCount != 1 ) {
		DEVPRINTF( "Level::_LoadWireData(): Invalid number of parameters specified for WireCount command.\n" );
		return;
	}

	FGameData_VarType_e nVarType;
	f32 fWirePoolCount;

	fWirePoolCount = *(f32 *)fgamedata_GetPtrToFieldData( hWireTable, 0, nVarType );
	if( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
		DEVPRINTF( "Level::_LoadWireData(): Invalid parameter type specified for WireCount command.\n" );
		return;
	}

	FMATH_CLAMP( fWirePoolCount, 1.0f, 1000.0f );

	CEntity::SetWirePoolCount( (u32)fWirePoolCount );
}


static void _LoadMissionText( void ) {
	FGameData_VarType_e nVarType;
	cwchar *pwszText;
	u32 i, nFieldCount;

	FResFrame_t ResFrame = fres_GetFrame();
	_nMissionTextIndex = 0;

	// No mission text if no level CSV...
	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		goto _ExitWithNoMissionText;
	}

	// Find the mission text table...
	FGameDataFileHandle_t hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "MissionText" );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		goto _ExitWithNoMissionText;
	}

	// Get the number of mission text entries...
	nFieldCount = fgamedata_GetNumFields( hTable );
	if( nFieldCount == 0 ) {
		goto _ExitWithNoMissionText;
	}

	// Allocate an array of string pointers, one per mission text entry...
	_ppwszMissionText = (cwchar **)fres_Alloc( sizeof(cwchar *) * nFieldCount );
	if( _ppwszMissionText == NULL ) {
		DEVPRINTF( "Level::_LoadMissionText(): Not enough memory to allocate _ppwszMissionText.\n" );
		goto _ExitWithNoMissionText;
	}

	// Import all mission text entries to the string table and point to them in the array...
	for( i=0; i<nFieldCount; ++i ) {
		pwszText = (cwchar *)fgamedata_GetPtrToFieldData( hTable, i, nVarType );
		if( nVarType != FGAMEDATA_VAR_TYPE_WIDESTRING ) {
			DEVPRINTF( "Level::_LoadMissionText(): Invalid parameter type specified for MissionText command.\n" );
			goto _ExitWithNoMissionText;
		}

		if( fclib_wcsicmp( pwszText, L"None" ) ) {
			_ppwszMissionText[i] = gstring_Main.AddString( pwszText );
			if( _ppwszMissionText[i] == NULL ) {
				DEVPRINTF( "Level::_LoadMissionText(): Not enough memory to store mission text.\n" );
				goto _ExitWithNoMissionText;
			}
		} else {
			_ppwszMissionText[i] = NULL;
		}
	}

	// Success...
	_nMissionTextCount = nFieldCount;

	return;

_ExitWithNoMissionText:
	fres_ReleaseFrame( ResFrame );

	_ppwszMissionText = NULL;
	_nMissionTextCount = 0;
}


// Returns the number of mission text entries for the current level.
u32 level_GetMissionTextCount( void ) {
	return _nMissionTextCount;
}


// Returns the current mission text index.
u32 level_GetCurrentMissionTextIndex( void ) {
	return _nMissionTextIndex;
}


// Sets the current mission text index.
// Returns TRUE if successful, or FALSE if an invalid index was passed in.
BOOL level_SetCurrentMissionTextIndex( u32 nMissionTextIndex ) {
	if( nMissionTextIndex < _nMissionTextCount ) {
		_nMissionTextIndex = nMissionTextIndex;
		return TRUE;
	} else {
		return FALSE;
	}
}


// Returns the current mission text string.
// Returns NULL if there is no mission text.
cwchar *level_GetCurrentMissionText( void ) {
	if( _nMissionTextCount ) {
		return _ppwszMissionText[_nMissionTextIndex];
	} else {
		return NULL;
	}
}


void level_LoadMemoryCacheData( FGameDataFileHandle_t hCSVFile ) {

	if( hCSVFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		FDS_StreamMgr.Init( (u32)FDS_DEFAULT_LOCAL_CACHE_SIZE );
		return;
	}
	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( hCSVFile, "Mem_Cache" );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		FDS_StreamMgr.Init( (u32)FDS_DEFAULT_LOCAL_CACHE_SIZE );
		return;
	}

	// Found table...
	FGameData_VarType_e nVarType;
	f32 *pfCacheSize = (f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nVarType );

	if ( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
		FDS_StreamMgr.Init( (u32)FDS_DEFAULT_LOCAL_CACHE_SIZE );
		return;
	}

	FDS_StreamMgr.Init( (u32)*pfCacheSize );
}

void level_LoadFogData( FGameDataFileHandle_t hCSVFile ) {

	UserPropDataFog_t UserPropData;

	if( hCSVFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		fsh_Fog_Enable(FALSE);
		return;
	}
	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( hCSVFile, "Fog" );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		fsh_Fog_Enable(FALSE);
		return;
	}

	// Found table...
	if( !fgamedata_GetTableData( hTable, _aUserPropVocabFog, &UserPropData, sizeof(UserPropData) ) ) {
		fsh_Fog_Enable(FALSE);
		return;
	}

	CFColorRGB fogClr;

	f32 fOO255 = (1.0f/255.0f);
	fogClr.fRed = UserPropData.fRed*fOO255;
	fogClr.fGreen = UserPropData.fGreen*fOO255;
	fogClr.fBlue = UserPropData.fBlue*fOO255;

	fsh_Fog_SetParam(UserPropData.fFogNearDepth, UserPropData.fFogFarDepth, UserPropData.fMinHeight, UserPropData.fMaxHeight, UserPropData.fDensity, fogClr, NULL);

	if (UserPropData.fDensity > 0.0f)
	{
		fsh_Fog_Enable(TRUE);
	}
}

//Loads level reflection data. Right now its just a texture but there may be more data in the future.
void level_LoadReflectionData( FGameDataFileHandle_t hCSVFile ) {

	UserPropDataReflection_t UserPropData;

	if( hCSVFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}
	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( hCSVFile, "Reflection" );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return;
	}

	// Found table...
	if( !fgamedata_GetTableData( hTable, _aUserPropVocabReflection, &UserPropData, sizeof(UserPropData) ) ) {
		return;
	}

	ftex_SetLevelReflectionMap( "demorefl", UserPropData.pszTexture );
}

void level_LoadFadeData( FGameDataFileHandle_t hCSVFile ) {

	// set the fade to the defaults 
	Level_fStartingFadeSecs = _DEFAULT_STARTING_FADE_SECS;
	Level_fEndingFadeSecs = _DEFAULT_ENDING_FADE_SECS;

	if( hCSVFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}
	// find the starting fade table
	FGameData_VarType_e nVarType;
	f32 fTime;
	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( hCSVFile, "start_fade_secs" );
	if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		// Found table...	
		fTime = *(f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nVarType );
		if( nVarType == FGAMEDATA_VAR_TYPE_FLOAT ) {
			FMATH_CLAMP( fTime, 0.0f, 10.0f );
			Level_fStartingFadeSecs = fTime;
		}
	}
	// find the ending fade table
	hTable = fgamedata_GetFirstTableHandle( hCSVFile, "end_fade_secs" );
	if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		// Found table...	
		fTime = *(f32 *)fgamedata_GetPtrToFieldData( hTable, 0, nVarType );
		if( nVarType == FGAMEDATA_VAR_TYPE_FLOAT ) {
			FMATH_CLAMP( fTime, 0.0f, 10.0f );
			Level_fEndingFadeSecs = fTime;
		}
	}	
}

static void _LoadBotTalkData( void ) {
	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}

	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "bottalkgroups" );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return;
	}
	
	u32 nNumFields = fgamedata_GetNumFields( hTable );
	cchar *pszBotTalk;
	FGameData_VarType_e nVarType;

	for( u32 i = 0; i < nNumFields; ++i ) {
		pszBotTalk = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nVarType );

		if( !pszBotTalk || 
			(nVarType != FGAMEDATA_VAR_TYPE_STRING) ) {
			DEVPRINTF( "Level::_LoadBotTalkData(): Entry %d of the bottalks table is invalid, skipping.\n", i );
			continue;
		}

		if( !AIBTA_LoadBTAGroup( pszBotTalk ) ) {
			DEVPRINTF( "Level::_LoadBotTalkData(): Trouble loading the level BTA group '%s'.\n", pszBotTalk );
		}
	}	
}

static void _LoadCollectableOverrideCSV( void ) {
	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}

	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "collectable_override" );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return;
	}

	u32 uNumFields = fgamedata_GetNumFields( hTable );

	if( uNumFields != 1 ) {
		return;
	}

	cchar *pszCSVName;
	FGameData_VarType_e nVarType;

	pszCSVName = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nVarType );

	if( !pszCSVName || 
		(nVarType != FGAMEDATA_VAR_TYPE_STRING) ) {
		DEVPRINTF( "Level::_LoadCollectableOverrideCSV(): Entry for 'collectable_override' is invalid.\n" );
		return;
	}

	if( !CCollectable::LoadOverrideCSV( pszCSVName ) ) {
		DEVPRINTF( "Level::_LoadCollectableOverrideCSV(): CCollectable::LoadOverrideCSV() failed.\n" );
		return;
	}
}

void level_SetIntroMovieHandle ( cutscene_Handle_t hIntroMovie ) {
	_hIntroMovie = hIntroMovie;
}


void level_PlayIntroMovie( void ) {
	_bPlayIntroMovie = TRUE;
}

void level_CheckpointSave( void ) {
	u32 i;
	_StreamingAudioInfo_t EmptyInfo;

	FANG_ZEROSTRUCT( EmptyInfo );

	//save the time into the level
	//CFCheckPoint::SaveData( Level_fSecsInLevel );

	// Save the current mission text index...
	CFCheckPoint::SaveData( _nMissionTextIndex );

	//save off the music info data...
	CFCheckPoint::SaveData( (void*) &_LMData, sizeof( _LevelMusicData_t ) );

	//now, save off the stream info
	for( i=0; i < _STREAM_COUNT; i++ ) {
		if( i == _STREAM_SPEECH ) {
			// We don't want to save the speech info, write 0s
			CFCheckPoint::SaveData( (void*) &EmptyInfo, sizeof( _StreamingAudioInfo_t ) );
		} else {
			CFCheckPoint::SaveData( (void*) &_aStreamingAudioInfo[i], sizeof( _StreamingAudioInfo_t ) ); //tells whether there is a stream here...
		}
	}
}


void level_CheckpointRestore( void ) {
	u32 i;

	// exit out of any debug cameras
	gamecam_ExitAnyDebugCam();

	// restore the time into the level
	//CFCheckPoint::LoadData( Level_fSecsInLevel );

	// Restore the current mission text index...
	CFCheckPoint::LoadData( _nMissionTextIndex );

	//restore the music info data...
	CFCheckPoint::LoadData( (void*) &_LMData, sizeof( _LevelMusicData_t ) );

	//restore the stream info
	for( i=0; i < _STREAM_COUNT; i++ ) {

		//first, stop and destroy the current stream...
		if( _aStreamingAudioInfo[i].pAudioStream ) {
			_aStreamingAudioInfo[i].pAudioStream->Stop( FALSE );
			_aStreamingAudioInfo[i].pAudioStream->Destroy();
			_aStreamingAudioInfo[i].pAudioStream = NULL;
		}
		CFCheckPoint::LoadData( (void*) &_aStreamingAudioInfo[i], sizeof( _StreamingAudioInfo_t ) ); //tells whether there is a stream here...

		//the pAudioStream value represents wheter a stream was playing or not... lets check it and act appropriately
		if( _aStreamingAudioInfo[i].pAudioStream ) {

			//Null out the previous interface pointer and restart the stream!
			_aStreamingAudioInfo[i].pAudioStream = NULL;

			// Start the stream in the same place it was saved to
			level_StartStream( _aStreamingAudioInfo[i].szFilename,
				_aStreamingAudioInfo[i].nNumLoops,
				_aStreamingAudioInfo[i].fVolume,
				i,
				_aStreamingAudioInfo[i].bPlayAsMusic );
		}
	}
}


////////////////////////////////////
// streaming interface functions
////////////////////////////////////

cchar *level_GetSteamingAudioFilename( u32 nIndex, BOOL &rbTreatAsMusicStream ) {
	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ||
		Level_hStreamingAudioTable == FGAMEDATA_INVALID_TABLE_HANDLE ||
		!Level_nNumStreamingAudioEntries ) {
		return NULL;
	}
	FGameData_VarType_e nVarType;
	nIndex <<= 1;
	cchar *pszFilename = (cchar *)fgamedata_GetPtrToFieldData( Level_hStreamingAudioTable, nIndex, nVarType );
	if( pszFilename ) {
        cchar *pszMusic = (cchar *)fgamedata_GetPtrToFieldData( Level_hStreamingAudioTable, nIndex+1, nVarType );
		if( !pszMusic || fclib_stricmp( pszMusic, "sfx" ) != 0 ) {
			// the default is music
            rbTreatAsMusicStream = TRUE;
		} else {
			rbTreatAsMusicStream = FALSE;
		}
	}

	return pszFilename;
}

int level_StartStream( cchar *pszFilename,
					  u32 nNumLoops/*=1*/,
					  f32 fVolume/*=1.0f*/,
					  s32 nStreamIndex/*=-1*/, 
					  BOOL bTreatAsMusicStream/*=TRUE*/ ) {
	// find a free slot
	u32 i;

	if( nStreamIndex < 0 ) {
		for( i=0; i < _STREAM_COUNT; i++ ) {
			if( !_aStreamingAudioInfo[i].pAudioStream ) {
				break;
			}
		}
		if( i == _STREAM_COUNT ) {
			// no more slots, stop something first
			return -1;
		}
	} else {
		FASSERT( nStreamIndex < _STREAM_COUNT );

		i = nStreamIndex;

		if( _aStreamingAudioInfo[i].pAudioStream ) {
			// Stream in use...
			return -1;
		}
	}

	_aStreamingAudioInfo[i].pAudioStream = CFAudioStream::Create( pszFilename, bTreatAsMusicStream );
	if( !_aStreamingAudioInfo[i].pAudioStream ) {
		return -1;
	}
	// copy the loops, vol, & name for later
	fclib_strncpy( _aStreamingAudioInfo[i].szFilename, pszFilename, _MAX_STREAM_FILENAME );
	_aStreamingAudioInfo[i].nNumLoops = nNumLoops;
	_aStreamingAudioInfo[i].fVolume = fVolume;
	_aStreamingAudioInfo[i].bWaitForCreate = TRUE;
	_aStreamingAudioInfo[i].bPlayAsMusic = bTreatAsMusicStream;

	DEVPRINTF( "level_StartStream(): Starting streaming audio file '%s'.\n", pszFilename );

	return i;
}

void level_StopStream( cchar *pszFilename ) {
	// find the slot
	u32 i;
	for( i=0; i < _STREAM_COUNT; i++ ) {
		if( _aStreamingAudioInfo[i].pAudioStream && fclib_stricmp( pszFilename, _aStreamingAudioInfo[i].szFilename ) == 0 ) {
			DEVPRINTF( "level_StopStream(): Stopping streaming audio file '%s'.\n", pszFilename );

			_aStreamingAudioInfo[i].pAudioStream->Stop( FALSE );
			_aStreamingAudioInfo[i].pAudioStream->Destroy();
			_aStreamingAudioInfo[i].pAudioStream = NULL;

			return;
		}
	}
}

void level_StopAllStreams( void ) {
	// stop all slots
	u32 i;
	for( i=0; i < _STREAM_COUNT; i++ ) {
		if( _aStreamingAudioInfo[i].pAudioStream ) {
			DEVPRINTF( "level_StopAllStreams(): Stopping streaming audio file '%s'.\n", _aStreamingAudioInfo[i].szFilename );
			_aStreamingAudioInfo[i].pAudioStream->Stop( FALSE );
			_aStreamingAudioInfo[i].pAudioStream->Destroy();
			_aStreamingAudioInfo[i].pAudioStream = NULL;
		}
	}
	// if all the streams are stopped, the current music should be consider null, and state is stopped
	_LMData.nMusicIndex = -1;
	_LMData.szCurrentMusic[0] = NULL;
	_LMData.nMusicState = _LEVEL_MUSIC_STATE_STOP;
}

// don't use this function for setting music volume
void level_SetCurStreamVolume( f32 fNewVolume ) {
	u32 i;
	for( i=0; i < _STREAM_COUNT; i++ ) {
		if( _aStreamingAudioInfo[i].pAudioStream ) {
			if( _aStreamingAudioInfo[i].pAudioStream->GetState() == FAUDIO_STREAM_STATE_PLAYING ) {
				_aStreamingAudioInfo[i].pAudioStream->SetVolume( fNewVolume );
			}
		}
	}
}

// Restart all streams
void level_RestartAllStreams( void ) {
	u32 i;

	for( i=0; i < _STREAM_COUNT; i++ ) {
		if( _aStreamingAudioInfo[i].pAudioStream ) {
			// NKM
			// This is done since we can't stop and then start a stream in the same frame, tell it to start over or
			// set its play position.  The best solution without modifying faudio is to destroy it and then
			// re-create the stream.
			_aStreamingAudioInfo[i].pAudioStream->Stop( FALSE );
			_aStreamingAudioInfo[i].pAudioStream->Destroy();
			_aStreamingAudioInfo[i].pAudioStream = NULL;

			_aStreamingAudioInfo[i].pAudioStream = CFAudioStream::Create( _aStreamingAudioInfo[i].szFilename,
																		  _aStreamingAudioInfo[i].bPlayAsMusic );

			if( _aStreamingAudioInfo[i].pAudioStream ) {
				_aStreamingAudioInfo[i].bWaitForCreate = TRUE;
			}
		}
	}
}

// Plays the specified streaming file from the streaming SPEECH channel.
// If the channel is already playing a track, that track is aborted.
void level_PlayStreamingSpeech( cchar *pszName, f32 fMaxVol, u32 nLoopCount/*=1*/ ) {
	_StopStream( _STREAM_SPEECH );
	level_StartStream( pszName, nLoopCount, fMaxVol, _STREAM_SPEECH, FALSE );
}

// Stops the streaming SPEECH channel immediately.
void level_StopStreamingSpeech( void ) {
	_StopStream( _STREAM_SPEECH );
}

// Returns TRUE if the streaming SPEECH channel is currently playing.
BOOL level_IsStreamingSpeechPlaying( void ) {

	return _level_IsPlaying(_STREAM_SPEECH);
}

// Returns TRUE if the streaming MUSIC channel is currently playing.
BOOL level_IsMusicPlaying( void ) {

	return _level_IsPlaying(_STREAM_MUSIC);
}

BOOL _level_IsPlaying (_Stream_e eWhichStream) {
	if( _aStreamingAudioInfo[eWhichStream].pAudioStream == NULL ) {
		return FALSE;
	}

	if(    _aStreamingAudioInfo[eWhichStream].pAudioStream->GetState() != FAUDIO_STREAM_STATE_PLAYING 
		&& _aStreamingAudioInfo[eWhichStream].pAudioStream->GetState() != FAUDIO_STREAM_STATE_CREATING 
		&& _aStreamingAudioInfo[eWhichStream].pAudioStream->GetState() != FAUDIO_STREAM_STATE_PAUSED ) {
			return FALSE;
		}

		return TRUE;

}

CFAudioStream *level_GetStreamingSpeechAudioStream( void ) {
	return _aStreamingAudioInfo[_STREAM_SPEECH].pAudioStream;
}

CFAudioStream *level_GetMusicAudioStream( void ) {
	return _aStreamingAudioInfo[_STREAM_MUSIC].pAudioStream;
}

// Everything played with the music interface will be registered as a music stream.
//
// Plays the specified streaming file from the streaming MUSIC channel.
// If the channel is already playing a track, that track is faded out
// and the new track is faded in.
BOOL level_PlayMusic( cchar *pszName, f32 fVolLevel, BOOL bFadeIn ) {

	if( pszName ) {
		if( fclib_stricmp( pszName, _LMData.szCurrentMusic ) ) {
			fclib_strncpy( _LMData.szNewMusic, pszName, _MAX_MUSIC_CHARS );
			_LMData.szNewMusic[_MAX_MUSIC_CHARS] = 0;
		}

		_LMData.fMaxMusicVolume = fVolLevel;
		_LMData.bPlayMusicWithFadeIn = bFadeIn;
		_LMData.bPlayNewTrackAsMusic = TRUE;
	}

	return TRUE;
}
// temporarily fade-in/out whatever music is going, but actually restarts...
BOOL level_PauseMusic( BOOL bPause )
{
	if (bPause && (bPause != _LMData.bPauseMusic))
	{
		if (_LMData.szNewMusic[0]) 
			DEVPRINTF( "level_PauseStream(): Stopping streaming audio file '%s'.\n", _LMData.szNewMusic );
		else if (_LMData.szCurrentMusic[0]) 
			DEVPRINTF( "level_PauseStream(): Stopping streaming audio file '%s'.\n", _LMData.szCurrentMusic );
		else
			DEVPRINTF( "level_PauseStream(): Stopping streaming audio file None.\n");
	}
	else if (bPause != _LMData.bPauseMusic)
		DEVPRINTF( "level_PauseStream(): Resuming streaming audio file '%s'.\n", _LMData.szNewMusic );

	_LMData.bPauseMusic = bPause;
	return TRUE;
}

// everything played with the sfx interface will be registered as a sfx stream
//
// Plays the specified streaming file from the streaming MUSIC channel.
// If the channel is already playing a track, that track is faded out
// and the new track is faded in.
BOOL level_PlaySfxStream( cchar *pszName, f32 fVolLevel, BOOL bFadeIn ) {
	
	if( pszName ) {
		if( fclib_stricmp( pszName, _LMData.szCurrentMusic ) ) {
			fclib_strncpy( _LMData.szNewMusic, pszName, _MAX_MUSIC_CHARS );
			_LMData.szNewMusic[_MAX_MUSIC_CHARS] = 0;
		}

		_LMData.fMaxMusicVolume = fVolLevel;
		_LMData.bPlayMusicWithFadeIn = bFadeIn;
		_LMData.bPlayNewTrackAsMusic = FALSE;
	}

	return TRUE;
}

BOOL level_StopMusic( void ) {
	_LMData.bStopMusic = TRUE;
	return TRUE;
}

BOOL level_PlayDefaultStreamingTrack( f32 fVolLevel, BOOL bFadeIn ) {
	if( _LMData.bDefaultTrackIsMusic ) {
		return level_PlayMusic( _LMData.szDefaultTrack, fVolLevel, bFadeIn );
	} else {
		return level_PlaySfxStream( _LMData.szDefaultTrack, fVolLevel, bFadeIn );
	}
}

// set a FadeTo volume for music,
// and a time to fade to that volume.
void level_FadeMusicVolume( f32 fTargetUnitVolume, f32 fFadeTime ) {
	// clamp inputs to valid values
	FMATH_CLAMPMIN( fFadeTime, 0.0f );
	FMATH_CLAMPMIN( fTargetUnitVolume, 0.0f );
	FMATH_CLAMPMAX( fTargetUnitVolume, 1.0f );

	if( fFadeTime == 0.0f ) {
		// change volume almost immediately if fade time is zero
		fFadeTime = 0.016f;
	}

	// set target volume and fade time.
	// _MusicWork() will fade volume over time.
	_LMData.fTargetFadeToVolume = fTargetUnitVolume;
	_LMData.fFadeToVolumeFadeTime = fFadeTime;
}

//this function decreases the Audio Emitter values to the target volume,
//as well as fades the music.
void level_DuckAudio( f32 fDuckVol/*= LEVEL_DEFAULT_DUCK_VOLUME*/ ) {
	FMATH_CLAMP_UNIT_FLOAT( fDuckVol );
	
	_LDData.nDuckCount++;
#if FANG_WINGC
	if( Fang_bPortDiag ) DEVPRINTF( "PORT-DUCK duck %.2f (count %d)\n", fDuckVol, _LDData.nDuckCount );
#endif

	if( fDuckVol != _LDData.fDuckVolume ) {
		_LDData.fDuckVolume = fDuckVol;
		level_FadeMusicVolume( _LDData.fDuckVolume, _LEVEL_AUDIO_DUCK_TIME );
		CFAudioEmitter::DuckAll( _LDData.fDuckVolume );
	}
}

void level_UnduckAudio( void ) {
	FASSERT( _LDData.nDuckCount > 0 );
	_LDData.nDuckCount--;
#if FANG_WINGC
	if( Fang_bPortDiag ) DEVPRINTF( "PORT-DUCK unduck (count %d)\n", _LDData.nDuckCount );
#endif
	if( _LDData.nDuckCount == 0 ) {
		_LDData.fDuckVolume = 1.0f;
		level_FadeMusicVolume( _LDData.fDuckVolume, _LEVEL_AUDIO_DUCK_TIME );
		CFAudioEmitter::DuckAll( _LDData.fDuckVolume );
	}
}

static void _StopStream( _Stream_e nStreamIndex ) {
	FASSERT( nStreamIndex>=0 && nStreamIndex<_STREAM_COUNT );

	if( _aStreamingAudioInfo[nStreamIndex].pAudioStream ) {
		DEVPRINTF( "_StopStream(): Stopping streaming audio file '%s'.\n", _aStreamingAudioInfo[nStreamIndex].szFilename );

		_aStreamingAudioInfo[nStreamIndex].pAudioStream->Stop( FALSE );
		_aStreamingAudioInfo[nStreamIndex].pAudioStream->Destroy();
		_aStreamingAudioInfo[nStreamIndex].pAudioStream = NULL;

		return;
	}
}

static void _InitAndPlayFirstMusic( void ) {
	cchar *pszFirstFile;
	BOOL bTreatAsMusic;

	// init the level music data
	_LMData.fCurrentFadeToVolume = 1.0f;
	_LMData.fFadeToVolumeFadeTime = 0.0f;
	_LMData.fTargetFadeToVolume = 1.0f;
	_LMData.fMusicVolume = LEVEL_DEFAULT_MUSIC_VOLUME;
	_LMData.nMusicState = _LEVEL_MUSIC_STATE_STOP;
	_LMData.nMusicIndex = -1;
	_LMData.bStopMusic = FALSE;
	
	_LMData.szCurrentMusic[0] = 0;
	_LMData.szDefaultTrack[0] = 0;

	_LMData.fMaxMusicVolume = LEVEL_DEFAULT_MUSIC_VOLUME;
	_LMData.bPlayMusicWithFadeIn = FALSE;
	_LMData.szNewMusic[0] = 0;

	_LMData.bDefaultTrackIsMusic = TRUE;
	
	// start the first streaming music file, if any
	pszFirstFile = level_GetSteamingAudioFilename( 0, bTreatAsMusic );
	if( pszFirstFile ) {
		FASSERT( fclib_strlen( pszFirstFile ) <= _MAX_MUSIC_CHARS );
		fclib_strcpy( _LMData.szDefaultTrack, pszFirstFile );
		_LMData.bDefaultTrackIsMusic = bTreatAsMusic;

		level_PlayDefaultStreamingTrack( LEVEL_DEFAULT_MUSIC_VOLUME, FALSE );
	}
}


static void _MusicWork( void ) {
	u32 i;

	switch( _LMData.nMusicState ) {
		case _LEVEL_MUSIC_STATE_STOP:
			if( _LMData.szNewMusic[0] ) {
				// new track to play
				_LMData.nMusicState = _LEVEL_MUSIC_STATE_START_TRACK;
			}
			if( _LMData.bPauseMusic ) {
				// pause playing current track
                _LMData.nMusicState = _LEVEL_MUSIC_STATE_PAUSED;
			}
			break;

		case _LEVEL_MUSIC_STATE_START_TRACK:
			FASSERT( _LMData.szNewMusic[0] );
						
			if( _LMData.bPlayMusicWithFadeIn ) {
				_LMData.fMusicVolume = 0.0f;
			} else {
				_LMData.fMusicVolume = _LMData.fMaxMusicVolume;
			}

			_LMData.nMusicIndex = level_StartStream( _LMData.szNewMusic,
													0,
													_LMData.fMusicVolume * _LMData.fCurrentFadeToVolume,
													_STREAM_MUSIC,
													_LMData.bPlayNewTrackAsMusic );
			_aStreamingAudioInfo[ _LMData.nMusicIndex ].fVolume = _LMData.fMaxMusicVolume;

			if( _LMData.nMusicIndex != -1 ) {
				// stream start successful
				fclib_strcpy( _LMData.szCurrentMusic, _LMData.szNewMusic );
				_LMData.nMusicState = _LEVEL_MUSIC_STATE_PLAYING;
			} else {
				// Stream start failed...
				_LMData.szCurrentMusic[0] = 0;
				_LMData.nMusicState = _LEVEL_MUSIC_STATE_STOP;
			}

			_LMData.szNewMusic[0] = 0;
 
			break;

		case _LEVEL_MUSIC_STATE_PLAYING:
			FASSERT( _LMData.nMusicIndex != -1 );

			if( _LMData.szNewMusic[0] ) {
				// New track to play...
				_LMData.nMusicState = _LEVEL_MUSIC_STATE_FADE_OUT;
			} else if( _LMData.bStopMusic ) {
				// stop playing current track
				_LMData.nMusicState = _LEVEL_MUSIC_STATE_FADE_OUT;
			}
			else if( _LMData.bPauseMusic ) {
				// pause playing current track
                _LMData.nMusicState = _LEVEL_MUSIC_STATE_PAUSED;
				// save the track in the start up slot
				fclib_strncpy( _LMData.szNewMusic, _LMData.szCurrentMusic, _MAX_MUSIC_CHARS );
				_LMData.szNewMusic[_MAX_MUSIC_CHARS] = 0;
			} else if( _LMData.bPlayMusicWithFadeIn && ( _LMData.fMusicVolume < _LMData.fMaxMusicVolume ) ) { 
				// increase the music volume to appropriate levels
				_LMData.fMusicVolume += _LEVEL_MUSIC_RAMP_RATE * _LMData.fMaxMusicVolume * FLoop_fPreviousLoopSecs;

				FMATH_CLAMPMAX( _LMData.fMusicVolume, _LMData.fMaxMusicVolume );

				_aStreamingAudioInfo[ _LMData.nMusicIndex ].pAudioStream->SetVolume( _LMData.fMusicVolume * _LMData.fCurrentFadeToVolume );
			}

			break;

		case _LEVEL_MUSIC_STATE_FADE_OUT:
			FASSERT( _LMData.nMusicIndex != -1 );

			if( _LMData.bPauseMusic ) {
				// pause playing current track
                _LMData.nMusicState = _LEVEL_MUSIC_STATE_PAUSED;
				// save the track in the start up slot
				if (!_LMData.szNewMusic[0])
				{
					fclib_strncpy( _LMData.szNewMusic, _LMData.szCurrentMusic, _MAX_MUSIC_CHARS );
					_LMData.szNewMusic[_MAX_MUSIC_CHARS] = 0;
				}
			}
			else if( _LMData.fMusicVolume > 0.0f ) {
				_LMData.fMusicVolume -= _LEVEL_MUSIC_FADE_RATE * _LMData.fMaxMusicVolume * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( _LMData.fMusicVolume, 0.0f );

				_aStreamingAudioInfo[ _LMData.nMusicIndex ].pAudioStream->SetVolume( _LMData.fMusicVolume * _LMData.fCurrentFadeToVolume );
			} else {
				// music faded out, stop stream
				level_StopStream( _LMData.szCurrentMusic );

				_LMData.szCurrentMusic[0] = 0;
				_LMData.nMusicIndex = -1;
				_LMData.bStopMusic = FALSE;

				_LMData.nMusicState = _LEVEL_MUSIC_STATE_STOP;
			}

			break;
		
		case _LEVEL_MUSIC_STATE_PAUSED:

			if( _LMData.bPauseMusic == FALSE ) {
				// unpause playing current track
				if (_LMData.szNewMusic[0])
					_LMData.nMusicState = _LEVEL_MUSIC_STATE_START_TRACK;
				else
					_LMData.nMusicState = _LEVEL_MUSIC_STATE_STOP;

				if ( _aStreamingAudioInfo[ _STREAM_MUSIC ].pAudioStream )
				{
					_aStreamingAudioInfo[_STREAM_MUSIC].pAudioStream->Destroy();
					_aStreamingAudioInfo[_STREAM_MUSIC].pAudioStream = NULL;
				}
				return;
			}
			if (_LMData.nMusicIndex != -1) {
				if( _LMData.fMusicVolume > 0.0f ) {
					_LMData.fMusicVolume -= _LEVEL_MUSIC_FADE_RATE * _LMData.fMaxMusicVolume * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMIN( _LMData.fMusicVolume, 0.0f );

					_aStreamingAudioInfo[ _LMData.nMusicIndex ].pAudioStream->SetVolume( _LMData.fMusicVolume * _LMData.fCurrentFadeToVolume );
				} else {
					// music faded out, stop stream
					level_StopStream( _LMData.szCurrentMusic );
				}
			
// Don't get rid state data in the pause case
//				_LMData.szCurrentMusic[0] = 0;
//				_LMData.nMusicIndex = -1;
//				_LMData.bStopMusic = FALSE;
//				_LMData.nMusicState = _LEVEL_MUSIC_STATE_STOP;
			}

			break;

	}

	// master volume work
	if( _LMData.fFadeToVolumeFadeTime > 0.0f ) {
		// move towards target volume
		_LMData.fCurrentFadeToVolume += ( _LMData.fTargetFadeToVolume - _LMData.fCurrentFadeToVolume ) * 
										( FLoop_fPreviousLoopSecs / _LMData.fFadeToVolumeFadeTime );
		FMATH_CLAMPMIN( _LMData.fCurrentFadeToVolume, 0.0f );
		FMATH_CLAMPMAX( _LMData.fCurrentFadeToVolume, 1.0f );

		// calculate remaining fade time
		_LMData.fFadeToVolumeFadeTime -= FLoop_fPreviousLoopSecs;

		FMATH_CLAMPMIN( _LMData.fFadeToVolumeFadeTime, 0.0f );

		if( _LMData.nMusicIndex != -1 ) {
			_aStreamingAudioInfo[ _LMData.nMusicIndex ].pAudioStream->SetVolume( _LMData.fCurrentFadeToVolume );
		}
	}

	// see if we need to actually start playing any streaming audio files
	for( i=0; i < _STREAM_COUNT; i++ ) {
		if( _aStreamingAudioInfo[i].pAudioStream ) {

			if( _aStreamingAudioInfo[i].bWaitForCreate ) {
				if( _aStreamingAudioInfo[i].pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
					_aStreamingAudioInfo[i].pAudioStream->SetVolume( _aStreamingAudioInfo[i].fVolume );		
					_aStreamingAudioInfo[i].pAudioStream->Play( _aStreamingAudioInfo[i].nNumLoops );
					_aStreamingAudioInfo[i].bWaitForCreate = FALSE;	
				}
			} else {
				// wait to be done playing
				if( _aStreamingAudioInfo[i].pAudioStream->GetState() == FAUDIO_STREAM_STATE_STOPPED ) {
					_aStreamingAudioInfo[i].pAudioStream->Destroy();
					_aStreamingAudioInfo[i].pAudioStream = NULL;
				}
			}
		}
	}
}

static void _LoadMusicData( void ) {
	if( Level_hLevelDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return;
	}

	Level_hStreamingAudioTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, "streaming_audio" );
	if( Level_hStreamingAudioTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return;
	}

	Level_nNumStreamingAudioEntries = fgamedata_GetNumFields( Level_hStreamingAudioTable );
	u32 i;
	FGameData_VarType_e nVarType;
	cchar *pszFilename;

	// init our stream ptrs to NULL
	for( i=0; i < _STREAM_COUNT; i++ ) {
		_aStreamingAudioInfo[i].pAudioStream = NULL;
		_aStreamingAudioInfo[i].fVolume = 1.0f;
		_aStreamingAudioInfo[i].nNumLoops = 0;
		_aStreamingAudioInfo[i].bWaitForCreate = FALSE;
	}

	// error check the data in the table
	for( i=0; i < Level_nNumStreamingAudioEntries; i++ ) {
		pszFilename = (cchar *)fgamedata_GetPtrToFieldData( Level_hStreamingAudioTable, i, nVarType );
		if( !pszFilename || 
			(nVarType != FGAMEDATA_VAR_TYPE_STRING) ) {
			DEVPRINTF( "Level::_LoadMusicData(): Entry %d of the streaming_audio table is invalid, disabling streaming audio.\n", i );
			Level_hStreamingAudioTable = FGAMEDATA_INVALID_TABLE_HANDLE;
			Level_nNumStreamingAudioEntries = 0;
			return;
		}		
	}
}


BOOL level_IsRacingLevel( void ) {
	if( Level_aInfo[Level_nLoadedIndex].nLevel == LEVEL_RAT_RACE || 
		Level_aInfo[Level_nLoadedIndex].nLevel == LEVEL_WASTELAND_CHASE ||
		Level_aInfo[Level_nLoadedIndex].nLevel == LEVEL_RACE_TO_THE_ROCKET ||
		Level_aInfo[Level_nLoadedIndex].nLevel == LEVEL_HOLD_YOUR_GROUND ) {
		return TRUE;
	}

	return FALSE;
}
