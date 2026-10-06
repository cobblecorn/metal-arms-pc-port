//////////////////////////////////////////////////////////////////////////////////////
// MG_HoldYourGround.cpp - Mini-Game: Hold Your Ground 
//
// Author: Michael Starich
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/21/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "mg_holdyourground.h"
#include "fgamedata.h"
#include "vehiclerat.h"
#include "vehiclesentinel.h"
#include "espline.h"
#include "entity.h"
#include "botAAGun.h"
#include "fres.h"
#include "player.h"
#if FANG_WINGC
#include "MultiplayerMgr.h"
#endif
#include "AI\AiApi.h"
#include "AI\AIEnviro.h"
#include "AI\AIGameUtils.h"
#include "AI\AIMain.h"
#include "AI\AIGraph.h"
#include "AI\AIBrain.h"
#include "AI\AIBrainUtils.h"
#include "AI\AIBrainman.h"
#include "AI\AIGroup.h"
#include "game.h"
#include "ebox.h"
#include "fresload.h"


#define _NUM_SPAWN_PTS			3
#define _NUM_WAVES				3
#define _NUM_BRIDGES			2

#define _EXPERT_SECS			1.0f
#define _SUCK_SECS				7.0f
#define _SUCK_RANGE				( _SUCK_SECS - _EXPERT_SECS )
#define _OO_SUCK_RANGE			( 1.0f / _SUCK_RANGE )

#define _SECS_TO_FADE_OUT		3.0f
#define _TOTAL_LOST_SECS		5.0f

#define _VERBOSE_DEBUGGING_MSG	FALSE



static cchar *_CSV_TABLE_NAME = "Mini_Game";
static cchar *_HUGE_RAT_EXPLOSION = "HugeRatExplosion";
static cchar *_LG_RAT_EXPLOSION = "LargeRatExplosion";

typedef struct {
	u32 nNumPaths;
	cchar *psz1stPath;
	cchar *pszVehicleSpawnPt;
	cchar *pszBotSpawnPt;
} _SpawnPtInit_t;

typedef struct {
	f32 fPercentTime;
	f32 fMinSecsBetweenSpawns;
	f32 fMaxSecsBetweenSpawns;
	f32 fMinSpeed;
	f32 fMaxSpeed;
	f32 fSecsBetweenWaves;
	u32 nMinConvoy;
	u32 nMaxConvoy;
	f32 fChanceOfDifferentPath;
	f32 fMinWaitBetweenConvoy;
	f32 fMaxWaitBetweenConvoy;
	f32 fMaxSecsWithNoTarget;
} _WaveInit_t;

typedef struct {
	u32 nNumRats;
	cchar *psz1stRatDriver;
	cchar *psz1stRat;

	u32 nNumTanks;
	cchar *psz1stTankDriver;
	cchar *psz1stTank;

	cchar *pszGunName;

	_SpawnPtInit_t aSpawnInits[_NUM_SPAWN_PTS];	

	f32 fTotalSecs;
	_WaveInit_t aWaveInits[_NUM_WAVES];

	cchar *apszTriggerBoxes[_NUM_BRIDGES];
	cchar *pszMusicTrigger;

	cchar *pszAlarmLoop;
	cchar *pszAlloy;
	cchar *pszJumpPoint;

} _InitData_t;

#define _NUM_CSV_FIELDS		( sizeof( _InitData_t ) / 4 )

#if FANG_WINGC
// The retail Mini_Game table (wewchold_01.csv) has 104 fields. Fields 0-2 and 6-61 match this
// source's 62; retail fields 3-5 (a float, "HugeRatExplosion", "pred_music") took the place of the
// tank fields, whose code is compiled out here, and fields 62-103 are retail additions (predators,
// an intro cutscene, timed radio lines) that this source does not implement.
#define _RETAIL_NUM_CSV_FIELDS	( 104 )

// Skips retail fields the source does not read; always TRUE so it can sit in the parse chain.
static BOOL _SkipRetailFields( u32 &rnIndex, u32 nCount ) {
	rnIndex += nCount;
	return TRUE;
}
#endif

typedef struct {
	u32 nNumPaths;
	CESpline **papSplines;

	CEntity *pVehicleSpawn;
	CEntity *pBotSpawn;

	BOOL8 bReadyForNewConvoy;	// we are ready to start a new convoy
	BOOL8 bAllowNewPath;		// should we try to randomly pick a new path
	u8 nPathIndex;				// what path should the spawned vehicles take
	u8 nNumRemainingToSpawn;	// how many more spawns are in left in the current convoy
    
	f32 fSecsTillNextSpawn;		// how many seconds till the next spawn in the convoy
	f32 fSecsBetweenSpawns;		// set when the 1st car is spawned
	f32 fSpeedFactorOfEachSpawn;// what speed factor should each spawned vehicle be at	
} _SpawnPtData_t;

typedef struct {
	f32 fSecsToLast;
	_WaveInit_t *pInit;
} _WaveData_t;

typedef enum {
	_STATE_WAIT_TO_GET_INTO_GUN = 0,
	_STATE_ACTIVE,
	_STATE_WAIT,
	_STATE_LOSE,
	_STATE_WIN,

	_STATE_COUNT

} _State_e;

typedef enum {
	_WALK_STATES_START = 0,
	_WALK_STATES_WAIT,
	_WALK_STATES_WALK,
	_WALK_STATES_ACTION,
	_WALK_STATES_DONE,
	
	_WALK_STATES_COUNT
} _WalkStates_e;

typedef struct {
	// init vars
	_InitData_t InitData;

	// general vars
	f32 fTimer;	// the global timer
	f32 fTimeInWave;
	f32 fTimeTillNextSpawn;
 	_State_e nState;
	u32 nWaveNum;
	u32 nLastSpawnIndex;
   
	// rat vars
	u32 nNumRats;
	CVehicleRat **papRats;
	f32 *pafRatSinkY;
	u32 nNumRatDrivers;
	CBot **papRatDrivers;

	// tank vars
	u32 nNumTanks;
	CVehicleSentinel **papTanks;
	u32 nNumTankDrivers;
	CBot **papTankDrivers;

	// the human gun
	CBotAAGun *pAAGun;
#if FANG_WINGC
	CBotAAGun *apDefenseGuns[4];
	s32 nDefensePlayers;
	BOOL bDefenseIntroFinished;
#endif

	// data on the spawn pts & their paths
	_SpawnPtData_t aSpawnData[_NUM_SPAWN_PTS];

	// data on the waves of attack
	_WaveData_t aWaveData[_NUM_WAVES];

	CEBox *apTriggerBoxes[_NUM_BRIDGES];
	CEBox *pMusicTriggerBox;

	f32 fUnitSuck;// 0.0f means the player is kicking ass, 1.0f means the player is sucking
	f32 *pafRatTimeSinceBirth;
	f32 fRatsKilled;
	f32 fSecsAlive;

	FExplosion_GroupHandle_t hRatExplosionGroup;// a large explosion for the rats
	FExplosion_GroupHandle_t hHugeRatExplosionGroup;// a huge explosion for the rats

	FSndFx_FxHandle_t hAlarm;
	CFAudioEmitter *pAlarmEmitter;
	f32 fAlarmVolume;
	CFVec3 RightUnitXZ;

	FSndFx_FxHandle_t hAlloy;

	CEntity *pJumpPoint;
	_WalkStates_e nWalkState;
	BOOL bPlayedFirstTime;
} _LevelData_t;

static _LevelData_t *_pLevelData;

static s32 _FindAnAvailableEntityIndex( CEntity **papEntities, u32 nNumEntities );
static void _UpdateStringName( char *pszString, u32 nStringLen, u32 nIndex );
static BOOL _PullAllBotsFromTheWorld( void );
static void _Work_SinkDeadRats( void );
static BOOL _ReleaseRatIfPossible( u32 nSpawnIndex, u32 nPathIndex, f32 fUnitSpeedFactor );
static BOOL _AreAnyVehiclesAliveAndInTheWorld( void );

static void _Work_Convoys( void );
static void _ResetAllSpawnPtConvoys( void );
static BOOL _Start_NewConvoy( void );
static BOOL _AreAnyRatsInSpawnRadius( CFVec3A &rPos, f32 fRadius );
static void _RelocateDriverBot( CBot *pBot, const CFVec3A &rPos );
static BOOL _CheckTriggerBoxForLiveRats( void );
static void _ResetSuckMeterData( void );
static void _UpdateSuckMeter( void );
static f32 _RandomWithBiasTowardMinValue( f32 fUnitBiasFactor, f32 fMin, f32 fMax, f32 fPercentOfRangeToBias );
static void _MusicWork( void );
static BOOL _AreAllSpawnPtsUnused( void );
static BOOL _WalkToJumpPoint( void );

#if FANG_WINGC
static BOOL _CreateCoopDefenseGuns( void ) {
	_pLevelData->nDefensePlayers = MultiplayerMgr.IsLocalCoop() ? CPlayer::m_nPlayerCount : 1;
	if( _pLevelData->nDefensePlayers < 1 || _pLevelData->nDefensePlayers > 4 ) {
		return FALSE;
	}
	_pLevelData->apDefenseGuns[0] = _pLevelData->pAAGun;
	static const char *apszNames[4] = { "biggun", "coop_defense_p2", "coop_defense_p3", "coop_defense_p4" };
	for( s32 i=1; i < _pLevelData->nDefensePlayers; ++i ) {
		CFMtx43A Mtx = *_pLevelData->pAAGun->MtxToWorld();
		// Alternating sides leaves the original gun and scripted jump point intact.
		f32 fOffset = (i & 1) ? 16.0f * ((i + 1) / 2) : -16.0f * (i / 2);
		Mtx.m_vPos.x += Mtx.m_vRight.x * fOffset;
		Mtx.m_vPos.z += Mtx.m_vRight.z * fOffset;
		CBotAAGun *pGun = fnew CBotAAGun;
		if( !pGun || !pGun->PortCreateDefenseGun( _pLevelData->pAAGun, apszNames[i], &Mtx ) ) {
			fdelete( pGun );
			for( s32 j=1; j < i; ++j ) {
				fdelete( _pLevelData->apDefenseGuns[j] );
				_pLevelData->apDefenseGuns[j] = NULL;
			}
			return FALSE;
		}
		_pLevelData->apDefenseGuns[i] = pGun;
		DEVPRINTF( "Co-op defense: P%d gun at %.2f %.2f %.2f\n", i + 1, Mtx.m_vPos.x, Mtx.m_vPos.y, Mtx.m_vPos.z );
	}
	return TRUE;
}

static BOOL _CoopDefenseTeamReady( void ) {
	_pLevelData->pAAGun->EnableDriverExit( FALSE );
	if( _pLevelData->nDefensePlayers <= 1 ) {
		return _pLevelData->pAAGun->IsGunInUse();
	}
	BOOL bReady = TRUE;
	for( s32 i=0; i < _pLevelData->nDefensePlayers; ++i ) {
		CBot *pBot = (CBot *)Player_aPlayer[i].m_pEntityOrig;
		CBotAAGun *pGun = _pLevelData->apDefenseGuns[i];
		pGun->EnableDriverExit( FALSE );
		pGun->EnableDriverMovement( TRUE );
		pGun->EnableEnterMsgDisplay( FALSE );
		if( i > 0 ) {
			pGun->PortBoardDefensePlayer( pBot );
		}
		if( !pGun->PortDefensePlayerReady( pBot ) ) {
			bReady = FALSE;
		}
	}
	return bReady;
}
#endif

/////////////////////
// public functions:

BOOL mg_holdyourground_InitSystem( void ) {
	_pLevelData = NULL;

	return TRUE;
}

void mg_holdyourground_UninitSystem( void ) {
	mg_holdyourground_LevelUnload();
}

BOOL mg_holdyourground_LevelLoad( LevelEvent_e eEvent ) {
	FResFrame_t ResFrame;
	FMemFrame_t MemFrame;
	u32 nNumFields, i, nStringLen, nIndex, j;
	FGameDataTableHandle_t hTable;
	char szTempString[32];
	CVehicleRat *pRat;
	CBot *pBot;
//	CVehicleSentinel *pTank;
	CESpline *pSpline;
	_SpawnPtData_t *pSpawnData;
	_SpawnPtInit_t *pSpawnInit;
	CFVec3A Temp;

    
	// WE ARE ONLY INTERESTED IN INITING THE GAME AFTER ALL THE ENTITIES ARE RESOLVED
	if( eEvent != LEVEL_EVENT_POST_ENTITY_FIXUP ) {
		return TRUE;
	}
	FASSERT( !_pLevelData );

	// grab a frame
	ResFrame = fres_GetFrame();
	MemFrame = fmem_GetFrame();

	// allocate room for our level data
	_pLevelData = (_LevelData_t *)fres_AllocAndZero( sizeof( _LevelData_t ) );
	if( !_pLevelData ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : unable to allocate required memory.\n" );
		goto _EXIT_WITH_ERROR;
	}

	// load our sfx bank
	fresload_Load( FSNDFX_RESTYPE, "Level_08" );

	hTable = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, _CSV_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : Could not find the csv table named '%s'.\n", _CSV_TABLE_NAME );
		goto _EXIT_WITH_ERROR;
	}
	
	// read our data from the level csv file
	nNumFields = fgamedata_GetNumFields( hTable );
#if FANG_WINGC
	if( nNumFields != _RETAIL_NUM_CSV_FIELDS ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : The level csv table '%s' contains %d fields but the retail layout has %d.\n", _CSV_TABLE_NAME, nNumFields, _RETAIL_NUM_CSV_FIELDS );
		goto _EXIT_WITH_ERROR;
	}
#else
	if( !nNumFields || (nNumFields != _NUM_CSV_FIELDS) ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : The level csv table '%s' contains %d fields but is required to have %d.\n", _CSV_TABLE_NAME, nNumFields, _NUM_CSV_FIELDS );
		goto _EXIT_WITH_ERROR;
	}
#endif
	FGameData_TableEntry_t FloatEntry, StringEntry, U32Entry;
	
	U32Entry.nFlags = (FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_CONVERT_TO_U32);
	U32Entry.nBytesForData = sizeof( f32 );

	FloatEntry.nFlags = (FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_FLOAT_X);
	FloatEntry.nBytesForData = sizeof( f32 );

	StringEntry.nFlags = (FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_ONLY);
	StringEntry.nBytesForData = sizeof( cchar * );

	nIndex = 0;
	if( !fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.nNumRats ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.psz1stRatDriver ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.psz1stRat ) ||
#if FANG_WINGC
		!_SkipRetailFields( nIndex, 3 ) ||
#else
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.nNumTanks ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.psz1stTankDriver ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.psz1stTank ) ||
#endif
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.pszGunName ) ||
		// spawn 0
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aSpawnInits[0].nNumPaths ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[0].psz1stPath ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[0].pszVehicleSpawnPt ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[0].pszBotSpawnPt ) ||
		// spawn 1
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aSpawnInits[1].nNumPaths ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[1].psz1stPath ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[1].pszVehicleSpawnPt ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[1].pszBotSpawnPt ) ||
		// spawn 2
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aSpawnInits[2].nNumPaths ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[2].psz1stPath ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[2].pszVehicleSpawnPt ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.aSpawnInits[2].pszBotSpawnPt ) ||
		
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.fTotalSecs ) ||

		// wave 0
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fPercentTime ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fMinSecsBetweenSpawns ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fMaxSecsBetweenSpawns ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fMinSpeed ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fMaxSpeed ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fSecsBetweenWaves ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aWaveInits[0].nMinConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aWaveInits[0].nMaxConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fChanceOfDifferentPath ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fMinWaitBetweenConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fMaxWaitBetweenConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[0].fMaxSecsWithNoTarget ) ||
		
		// wave 1
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fPercentTime ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fMinSecsBetweenSpawns ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fMaxSecsBetweenSpawns ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fMinSpeed ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fMaxSpeed ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fSecsBetweenWaves ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aWaveInits[1].nMinConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aWaveInits[1].nMaxConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fChanceOfDifferentPath ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fMinWaitBetweenConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fMaxWaitBetweenConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[1].fMaxSecsWithNoTarget ) ||

		// wave 2
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fPercentTime ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fMinSecsBetweenSpawns ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fMaxSecsBetweenSpawns ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fMinSpeed ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fMaxSpeed ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fSecsBetweenWaves ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aWaveInits[2].nMinConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &U32Entry, &_pLevelData->InitData.aWaveInits[2].nMaxConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fChanceOfDifferentPath ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fMinWaitBetweenConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fMaxWaitBetweenConvoy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &_pLevelData->InitData.aWaveInits[2].fMaxSecsWithNoTarget ) ||

		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.apszTriggerBoxes[0] ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.apszTriggerBoxes[1] ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.pszMusicTrigger ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.pszAlarmLoop ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.pszAlloy ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &_pLevelData->InitData.pszJumpPoint ) ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : Trouble parsing the level csv table '%s', check the syntax.\n", _CSV_TABLE_NAME );
		goto _EXIT_WITH_ERROR;	
	}
	
#if FANG_WINGC
	// The tank code is compiled out; one slot keeps the tank arrays allocated as before.
	_pLevelData->InitData.nNumTanks = 1;
	_pLevelData->InitData.psz1stTankDriver = NULL;
	_pLevelData->InitData.psz1stTank = NULL;
#endif

	// do some sanity checks on our init data
	if( !_pLevelData->InitData.nNumRats ||
		!_pLevelData->InitData.nNumTanks ||
		!_pLevelData->InitData.aSpawnInits[0].nNumPaths ||
		!_pLevelData->InitData.aSpawnInits[1].nNumPaths ||
		!_pLevelData->InitData.aSpawnInits[2].nNumPaths ||
		_pLevelData->InitData.fTotalSecs <= 0.0f ) {
		// one of our counts was invalid, inform the user
		DEVPRINTF( "mg_holdyourground_LevelLoad() : One of the required csv fields was 0, check your counts.\n" );
		goto _EXIT_WITH_ERROR;	
	}
	
	// allocate room to hold our entity ptrs
	_pLevelData->papRats = (CVehicleRat **)fres_AllocAndZero( sizeof( CVehicleRat * ) * _pLevelData->InitData.nNumRats );
	_pLevelData->pafRatSinkY = (f32 *)fres_AllocAndZero( sizeof( f32 ) * _pLevelData->InitData.nNumRats );
	_pLevelData->papRatDrivers = (CBot **)fres_AllocAndZero( sizeof( CBot * ) * _pLevelData->InitData.nNumRats );
	_pLevelData->papTanks = (CVehicleSentinel **)fres_AllocAndZero( sizeof( CVehicleSentinel * ) * _pLevelData->InitData.nNumTanks );
	_pLevelData->papTankDrivers = (CBot **)fres_AllocAndZero( sizeof( CBot * ) * _pLevelData->InitData.nNumTanks );
	_pLevelData->aSpawnData[0].papSplines = (CESpline **)fres_AllocAndZero( sizeof( CESpline * ) * _pLevelData->InitData.aSpawnInits[0].nNumPaths );
	_pLevelData->aSpawnData[1].papSplines = (CESpline **)fres_AllocAndZero( sizeof( CESpline * ) * _pLevelData->InitData.aSpawnInits[1].nNumPaths );
	_pLevelData->aSpawnData[2].papSplines = (CESpline **)fres_AllocAndZero( sizeof( CESpline * ) * _pLevelData->InitData.aSpawnInits[2].nNumPaths );
	_pLevelData->pafRatTimeSinceBirth = (f32 *)fres_AllocAndZero( sizeof( f32 ) * _pLevelData->InitData.nNumRats );
	if( !_pLevelData->papRats ||
		!_pLevelData->pafRatSinkY ||
		!_pLevelData->papRatDrivers ||
		!_pLevelData->papTanks ||
		!_pLevelData->papTankDrivers ||
		!_pLevelData->aSpawnData[0].papSplines ||
		!_pLevelData->aSpawnData[1].papSplines ||
		!_pLevelData->aSpawnData[2].papSplines ||
		!_pLevelData->pafRatTimeSinceBirth ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : unable to allocate required memory.\n" );
		goto _EXIT_WITH_ERROR;
	}
		
	fang_MemZero( szTempString, sizeof( szTempString ) );
	
	// find all of the rats
	fclib_strncpy( szTempString, _pLevelData->InitData.psz1stRat, sizeof( szTempString ) );
	nStringLen = fclib_strlen( szTempString );
	nIndex = szTempString[nStringLen-1] - '0';
	FMATH_CLAMP( nIndex, 0, 9 );
	for( i=0; i < _pLevelData->InitData.nNumRats; i++ ) {
		// find the entity
		pRat = (CVehicleRat *)CEntity::Find( szTempString );
		if( !pRat ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the rat named '%s'.\n", szTempString );
			goto _EXIT_WITH_ERROR;
		} else if( !(pRat->TypeBits() & ENTITY_BIT_VEHICLERAT) ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a Rat, please fix.\n", szTempString );
			goto _EXIT_WITH_ERROR;
		} else {
			// set some properties on our rats
			pRat->SetSpotLightOff( TRUE );
			pRat->m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;
			pRat->DisableAllMeshCollisionsIfDeadOrDying();
			pRat->DestroyDustAndCloudParticles();
			pRat->SetRatFlipOnDeath( TRUE /*bEnable*/, TRUE /*bProjectileOnly*/ );
			pRat->SetSoundFxRadiusMultiplier( 2.0f );
			pRat->DisableDriverKillingOnDeath();

			_pLevelData->papRats[ _pLevelData->nNumRats ] = pRat;
			_pLevelData->nNumRats++;
		}
		
		// update the name string
		nIndex++;
		_UpdateStringName( szTempString, nStringLen, nIndex );
	}

	// find all of the rat drivers
	fclib_strncpy( szTempString, _pLevelData->InitData.psz1stRatDriver, sizeof( szTempString ) );
	nStringLen = fclib_strlen( szTempString );
	nIndex = szTempString[nStringLen-1] - '0';
	FMATH_CLAMP( nIndex, 0, 9 );
	for( i=0; i < _pLevelData->InitData.nNumRats; i++ ) {
		// find the entity
		pBot = (CBot *)CEntity::Find( szTempString );
		if( !pBot ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the bot named '%s'.\n", szTempString );
			goto _EXIT_WITH_ERROR;
		} else if( !(pBot->TypeBits() & ENTITY_BIT_BOT) ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a Bot, please fix.\n", szTempString );
			goto _EXIT_WITH_ERROR;
		} else {
			pBot->SetSpotLightOff( TRUE );
			pBot->m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;
			pBot->m_pWorldMesh->SetCollisionFlag( FALSE );
			
			_pLevelData->papRatDrivers[ _pLevelData->nNumRatDrivers ] = pBot;
			_pLevelData->nNumRatDrivers++;
		}
		
		// update the name string
		nIndex++;
		_UpdateStringName( szTempString, nStringLen, nIndex );
	}
#if 0	
	// find all of the tanks
	fclib_strncpy( szTempString, _pLevelData->InitData.psz1stTank, sizeof( szTempString ) );
	nStringLen = fclib_strlen( szTempString );
	nIndex = szTempString[nStringLen-1] - '0';
	FMATH_CLAMP( nIndex, 0, 9 );
	for( i=0; i < _pLevelData->InitData.nNumTanks; i++ ) {
		// find the entity
		pTank = (CVehicleSentinel *)CEntity::Find( szTempString );
		if( !pTank ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the tank named '%s'.\n", szTempString );
			
		} else if( !(pTank->TypeBits() & ENTITY_BIT_VEHICLESENTINEL) ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a Tank, please fix.\n", szTempString );
			
		} else {
			pTank->SetSpotLightOff( TRUE );
			pTank->m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;

			_pLevelData->papTanks[ _pLevelData->nNumTanks ] = pTank;
			_pLevelData->nNumTanks++;
		}
		
		// update the name string
		nIndex++;
		_UpdateStringName( szTempString, nStringLen, nIndex );
	}

	// find all of the tank drivers
	fclib_strncpy( szTempString, _pLevelData->InitData.psz1stTankDriver, sizeof( szTempString ) );
	nStringLen = fclib_strlen( szTempString );
	nIndex = szTempString[nStringLen-1] - '0';
	FMATH_CLAMP( nIndex, 0, 9 );
	for( i=0; i < _pLevelData->InitData.nNumTanks; i++ ) {
		// find the entity
		pBot = (CBot *)CEntity::Find( szTempString );
		if( !pBot ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the bot named '%s'.\n", szTempString );
			
		} else if( !(pBot->TypeBits() & ENTITY_BIT_BOT) ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a Bot, please fix.\n", szTempString );
			
		} else {
			pBot->SetSpotLightOff( TRUE );
			pBot->m_pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_CAST_SHADOWS;

			_pLevelData->papTankDrivers[ _pLevelData->nNumTankDrivers ] = pBot;
			_pLevelData->nNumTankDrivers++;
		}
		
		// update the name string
		nIndex++;
		_UpdateStringName( szTempString, nStringLen, nIndex );
	}
#endif
	// init all the data for a spawn pt
	for( i=0; i < _NUM_SPAWN_PTS; i++ ) {
		pSpawnData = &_pLevelData->aSpawnData[i];
		pSpawnInit = &_pLevelData->InitData.aSpawnInits[i];

		// find all of the paths
		fclib_strncpy( szTempString, pSpawnInit->psz1stPath, sizeof( szTempString ) );
		nStringLen = fclib_strlen( szTempString );
		nIndex = szTempString[nStringLen-1] - '0';
		FMATH_CLAMP( nIndex, 0, 9 );
		for( j=0; j < pSpawnInit->nNumPaths; j++ ) {
			// find the entity
			pSpline = (CESpline *)CEntity::Find( szTempString );
			if( !pSpline ) {
				DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the spline named '%s'.\n", szTempString );
				goto _EXIT_WITH_ERROR;
			} else if( !(pSpline->TypeBits() & ENTITY_BIT_SPLINE) ) {
				DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a Spline, please fix.\n", szTempString );
				goto _EXIT_WITH_ERROR;
			} else {
				pSpawnData->papSplines[pSpawnData->nNumPaths] = pSpline;
				pSpawnData->nNumPaths++;
			}
			
			// update the name string
			nIndex++;
			_UpdateStringName( szTempString, nStringLen, nIndex );
		}

		// find the Vehicle Spawn Pt
		pSpawnData->pVehicleSpawn = CEntity::Find( pSpawnInit->pszVehicleSpawnPt );
		if( !pSpawnData->pVehicleSpawn ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the vehicle spawn pt named '%s'.\n", pSpawnInit->pszVehicleSpawnPt );
			goto _EXIT_WITH_ERROR;
		}
		
		// find the Bot Spawn Pt
		pSpawnData->pBotSpawn = CEntity::Find( pSpawnInit->pszBotSpawnPt );
		if( !pSpawnData->pBotSpawn ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the bot spawn pt named '%s'.\n", pSpawnInit->pszBotSpawnPt );
			goto _EXIT_WITH_ERROR;
		}

		pSpawnData->bReadyForNewConvoy = TRUE;
		pSpawnData->bAllowNewPath = TRUE;
		pSpawnData->nPathIndex = 0;
		pSpawnData->nNumRemainingToSpawn = 0;
		pSpawnData->fSecsTillNextSpawn = 0.0f;
		pSpawnData->fSecsBetweenSpawns = 0.0f;
	}
	
	// find the AA Gun
	_pLevelData->pAAGun = (CBotAAGun *)CEntity::Find( _pLevelData->InitData.pszGunName );
	if( !_pLevelData->pAAGun ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the aagun named '%s'.\n", _pLevelData->InitData.pszGunName );
		goto _EXIT_WITH_ERROR;	
	} else if( !(_pLevelData->pAAGun->TypeBits() & ENTITY_BIT_BOTAAGUN) ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a BotAAGun, please fix.\n", _pLevelData->InitData.pszGunName );
		goto _EXIT_WITH_ERROR;
	}
	_pLevelData->pAAGun->EnableDriverExit( FALSE );

	// find the jump point entity
	_pLevelData->pJumpPoint = (CEntity *)CEntity::Find( _pLevelData->InitData.pszJumpPoint );
	if( !_pLevelData->pJumpPoint ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the jump point entity named '%s'.\n", _pLevelData->InitData.pszJumpPoint );
		goto _EXIT_WITH_ERROR;	
	}
	
	// find the 2 trigger boxes
	for( i=0; i < _NUM_BRIDGES; i++ ) {
		_pLevelData->apTriggerBoxes[i] = (CEBox *)CEntity::Find( _pLevelData->InitData.apszTriggerBoxes[i] );
		if( !_pLevelData->apTriggerBoxes[i] ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the trigger box named '%s'.\n", _pLevelData->InitData.apszTriggerBoxes[i] );
			goto _EXIT_WITH_ERROR;	
		} else if( !(_pLevelData->apTriggerBoxes[i]->TypeBits() & ENTITY_BIT_BOX) ) {
			DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a EBox, please fix.\n", _pLevelData->InitData.apszTriggerBoxes[i] );
			goto _EXIT_WITH_ERROR;
		}
	}
	// find the music trigger box
	_pLevelData->pMusicTriggerBox = (CEBox *)CEntity::Find( _pLevelData->InitData.pszMusicTrigger );
	if( !_pLevelData->pMusicTriggerBox ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : could not find the trigger box named '%s'.\n", _pLevelData->InitData.pszMusicTrigger );
		goto _EXIT_WITH_ERROR;	
	} else if( !(_pLevelData->pMusicTriggerBox->TypeBits() & ENTITY_BIT_BOX) ) {
		DEVPRINTF( "mg_holdyourground_LevelLoad() : the entity named '%s' is not a EBox, please fix.\n", _pLevelData->InitData.pszMusicTrigger );
		goto _EXIT_WITH_ERROR;
	}	

	// init the wave data
	for( i=0; i < _NUM_WAVES; i++ ) {
		_pLevelData->aWaveData[i].fSecsToLast = _pLevelData->InitData.aWaveInits[i].fPercentTime * _pLevelData->InitData.fTotalSecs;
		_pLevelData->aWaveData[i].pInit = &_pLevelData->InitData.aWaveInits[i];
	}	
		
	// do some final error checking
	if( !_pLevelData->nNumRats ||
		!_pLevelData->nNumRatDrivers ) {
		// couldn't find a required element for the mini-game
		DEVPRINTF( "mg_holdyourground_LevelLoad() : unable to find all of the required entities that are listed in the csv file.\n" );
		goto _EXIT_WITH_ERROR;
	}

	CVehicleRat::OverWriteDeathSmoke( (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, "RatSmk_huge" ) );

	// load our huge rat explosion group and overwrite the rat default
	_pLevelData->hHugeRatExplosionGroup = CExplosion2::GetExplosionGroup( _HUGE_RAT_EXPLOSION );
	CVehicleRat::m_hBigFinalExplosion = _pLevelData->hHugeRatExplosionGroup;
	// load our large rat explosion group and overwrite the rat default
	_pLevelData->hRatExplosionGroup = CExplosion2::GetExplosionGroup( _LG_RAT_EXPLOSION );
	CVehicleRat::m_hExplosionGroup = _pLevelData->hRatExplosionGroup;	
		
	// init some state vars
	_pLevelData->fTimer = 0.0f;	
	_pLevelData->fTimeInWave = 0.0f;
	_pLevelData->fTimeTillNextSpawn = 0.0f;
	_pLevelData->nWaveNum = 0;
	_pLevelData->nState = _STATE_WAIT_TO_GET_INTO_GUN;
	_pLevelData->nLastSpawnIndex = 0;
	_pLevelData->hAlarm = fsndfx_GetFxHandle( _pLevelData->InitData.pszAlarmLoop );
	_pLevelData->pAlarmEmitter = NULL;
	_pLevelData->fAlarmVolume = 0.0f;
	_pLevelData->hAlloy = fsndfx_GetFxHandle( _pLevelData->InitData.pszAlloy );
	_pLevelData->nWalkState = _WALK_STATES_START;
	_pLevelData->bPlayedFirstTime = FALSE;

	Temp.Sub( _pLevelData->aSpawnData[1].pBotSpawn->MtxToWorld()->m_vPos, _pLevelData->pAAGun->MtxToWorld()->m_vPos );
	Temp.UnitizeXZ();
	_pLevelData->RightUnitXZ.Set( Temp.z, 0.0f, -Temp.x );
    
	_ResetSuckMeterData();

#if FANG_WINGC
	if( !_CreateCoopDefenseGuns() ) {
		goto _EXIT_WITH_ERROR;
	}
#endif

	// start the transmission
#if FANG_WINGC
	CHud2::GetHudForPlayer(0)->TransmissionMsg_Start( CHud2::TRANSMISSION_AUTHOR_COLONEL_ALLOY, _pLevelData->hAlloy, 1.0f, FALSE );
#else
	CHud2::GetCurrentHud()->TransmissionMsg_Start( CHud2::TRANSMISSION_AUTHOR_COLONEL_ALLOY, _pLevelData->hAlloy, 1.0f, FALSE );
#endif

	return TRUE;

_EXIT_WITH_ERROR:
	_pLevelData = NULL;
	fres_ReleaseFrame( ResFrame );
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}

void mg_holdyourground_LevelUnload( void ) {

	if( !_pLevelData ) {
		return;
	}

	if( _pLevelData->pAlarmEmitter ) {
		_pLevelData->pAlarmEmitter->Destroy();
		_pLevelData->pAlarmEmitter = NULL;
	}
	_pLevelData->fAlarmVolume = 0.0f;

	_pLevelData = NULL;
}

void mg_holdyourground_LevelWork( void ) {

	FASSERT( _pLevelData );

	switch( _pLevelData->nState ) {

	case _STATE_WAIT_TO_GET_INTO_GUN:
		// wait for the player to get into the gun and have all of the bots pulled from the world
		if( _PullAllBotsFromTheWorld() && _WalkToJumpPoint() ) {
#if FANG_WINGC
			if( _CoopDefenseTeamReady() ) {
#else
			if( _pLevelData->pAAGun->IsGunInUse() ) {
#endif
				// time to move to the active state
				_pLevelData->nState = _STATE_ACTIVE;
				_pLevelData->fTimer = 0.0f;
				_pLevelData->fTimeTillNextSpawn = 0.0f;
				_pLevelData->nWaveNum = 0;
				_pLevelData->fTimeInWave = 0.0f;

				_ResetAllSpawnPtConvoys();

				_ResetSuckMeterData();
#if _VERBOSE_DEBUGGING_MSG
				DEVPRINTF( "MG_HoldYourGround_Work() : switching to active mode\n" );
				DEVPRINTF( "MG_HoldYourGround_Work() : wave %d\n", _pLevelData->nWaveNum + 1 );
#endif
			}
		}
		_pLevelData->pAAGun->EnableDriverExit( FALSE );
        break;

	case _STATE_WAIT:
		// advance the timer
		_pLevelData->fTimer += FLoop_fPreviousLoopSecs;

		// see if any bots can be pulled under the sand
		_Work_SinkDeadRats();

		_MusicWork();
		
		if( !_AreAnyVehiclesAliveAndInTheWorld() ) {
			// only time the idle when all of the bots have been killed
			_pLevelData->fTimeInWave += FLoop_fPreviousLoopSecs;
			if( _pLevelData->fTimeInWave >= _pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fSecsBetweenWaves ) {
				// move to the next wave
				_pLevelData->fTimeInWave = 0.0f;
				_pLevelData->nWaveNum++;
				_pLevelData->nState = _STATE_ACTIVE;
				_pLevelData->fTimeTillNextSpawn = 0.0f;

				_ResetSuckMeterData();
#if _VERBOSE_DEBUGGING_MSG
				DEVPRINTF( "MG_HoldYourGround_Work() : wave %d\n", _pLevelData->nWaveNum + 1 );
#endif
			}
		}
		break;

	case _STATE_ACTIVE:
		// advance the timer
		_pLevelData->fTimer += FLoop_fPreviousLoopSecs;

		// see if any bots can be pulled under the sand
		_Work_SinkDeadRats();

		_MusicWork();
#if _VERBOSE_DEBUGGING_MSG
		ftext_Printf( 0.5f, 0.08f, L"~f9~C80809299~w0~al~s0.65Wave %d\nNext Bot %.2f\nTime Left %.2f\nSuck Factor %.2f", _pLevelData->nWaveNum + 1,
			_pLevelData->fTimeTillNextSpawn,
			_pLevelData->InitData.fTotalSecs - _pLevelData->fTimer,
			_pLevelData->fUnitSuck );
#endif
		if( _pLevelData->fTimer < _pLevelData->InitData.fTotalSecs ) {

			_UpdateSuckMeter();

			// see if it time to advance to the next wave
			_pLevelData->fTimeInWave += FLoop_fPreviousLoopSecs;
			if( _pLevelData->nWaveNum < (_NUM_WAVES-1) &&
				_pLevelData->fTimeInWave >= _pLevelData->aWaveData[_pLevelData->nWaveNum].fSecsToLast ) {
				// move to the wait state between waves
				_pLevelData->nState = _STATE_WAIT;
				_pLevelData->fTimeInWave = 0.0f;
#if _VERBOSE_DEBUGGING_MSG
				DEVPRINTF( "MG_HoldYourGround_Work() : waiting between waves for %.2f seconds\n", _pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fSecsBetweenWaves );
#endif

				// cancel all current convoys
				_ResetAllSpawnPtConvoys();
			} else {
				// do the convoy work
				_Work_Convoys();
				
				// if there are not vehicles in the world, make sure the user doesn't wait too long till a new convoy is spawned
				if( _AreAllSpawnPtsUnused() && !_AreAnyVehiclesAliveAndInTheWorld() ) {
					// no bots in the world
					if( _pLevelData->fTimeTillNextSpawn > _pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fMaxSecsWithNoTarget ) {
#if _VERBOSE_DEBUGGING_MSG
						DEVPRINTF( "shaved %.2f secs off the wait time.\n", _pLevelData->fTimeTillNextSpawn - _pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fMaxSecsWithNoTarget );
#endif
						_pLevelData->fTimeTillNextSpawn = _pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fMaxSecsWithNoTarget;						
					}
				}

				// spawn a bot and vehicle
				_pLevelData->fTimeTillNextSpawn -= FLoop_fPreviousLoopSecs;
				if( _pLevelData->fTimeTillNextSpawn <= 0.0f ) {
					if( _Start_NewConvoy() ) {
						// reset the timer
						_pLevelData->fTimeTillNextSpawn = _RandomWithBiasTowardMinValue( 1.0f - _pLevelData->fUnitSuck,
															_pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fMinSecsBetweenSpawns,
															_pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fMaxSecsBetweenSpawns,
															0.80f );
					}
				}
			}

		} else {
			// the player wins once all of the bots are out of the world
			if( !_AreAnyVehiclesAliveAndInTheWorld() ) {
                _pLevelData->nState = _STATE_WIN;
				_pLevelData->fTimer = 0.0f;
#if _VERBOSE_DEBUGGING_MSG
				DEVPRINTF( "MG_HoldYourGround_Work() : switching to win mode\n" );
#endif
			}
		}
		break;

	case _STATE_LOSE:
		_pLevelData->fTimer += FLoop_fPreviousLoopSecs;
		if( _pLevelData->fTimer >= _TOTAL_LOST_SECS ) {
#if _VERBOSE_DEBUGGING_MSG
			DEVPRINTF( "MG_HoldYourGround_Work() : restarting the level\n" );
#endif
			// restart the level
			checkpoint_Restore( 0, FALSE, "hold-your-ground:restart" );
		}
		break;

	case _STATE_WIN:
		_pLevelData->fTimer += FLoop_fPreviousLoopSecs;
		if( _pLevelData->fTimer >= 0.5f ) {
#if _VERBOSE_DEBUGGING_MSG
			DEVPRINTF( "MG_HoldYourGround_Work() : level complete\n" );
#endif
			game_GotoLevel( "next" );
		}
		break;
	}		
}

void mg_holdyourground_LevelDraw( void ) {
	FASSERT( _pLevelData );
	
	if( _pLevelData->nState == _STATE_LOSE ) {
		f32 fAlpha;
			
		if( _pLevelData->fTimer <= _SECS_TO_FADE_OUT ) {
			fAlpha = 1.0f - (_pLevelData->fTimer * (1.0f/_SECS_TO_FADE_OUT));
		} else {
			fAlpha = 0.0f;
		}
		game_DrawSolidFullScreenOverlay( fAlpha, 0.0f );

		ftext_ClearAllPending();

		if( _pLevelData->fTimer >= 1.0f ) {
            ftext_Printf( 0.5, 0.35f, L"~f1~C75050580~w0~aC~s1.65%ls", Game_apwszPhrases[GAMEPHRASE_MISSION_FAILED] );
		}
	}
}

void mg_holdyourground_PlayerLost( void ) {
	FASSERT( _pLevelData );

	if( _pLevelData->nState == _STATE_ACTIVE ||
		_pLevelData->nState == _STATE_WAIT ) {

		if( !_CheckTriggerBoxForLiveRats() ) {
			// couldn't find a live rat inside the trigger areas, ignore the command
#if _VERBOSE_DEBUGGING_MSG
			DEVPRINTF( "MG_HoldYourGround_Work() : couldn't find a live rat in the trigger region, ignoring the tripwire\n" );
#endif
			return;
		}

		// the player has lost the game...	
		_pLevelData->nState = _STATE_LOSE;
		_pLevelData->fTimer = 0.0f;
#if _VERBOSE_DEBUGGING_MSG
		DEVPRINTF( "MG_HoldYourGround_Work() : switching to lose mode\n" );
#endif

		// don't allow the driver to control the gun anymore
		_pLevelData->pAAGun->EnableDriverMovement( FALSE );

		_pLevelData->pAAGun->ReticleEnable( FALSE );
#if FANG_WINGC
		for( s32 nPlayer=1; nPlayer < _pLevelData->nDefensePlayers; ++nPlayer ) {
			_pLevelData->apDefenseGuns[nPlayer]->EnableDriverMovement( FALSE );
			_pLevelData->apDefenseGuns[nPlayer]->ReticleEnable( FALSE );
		}
#endif

		// slow all rats down 
		u32 i;
		f32 fMaxSpeed;
		for( i=0; i < _pLevelData->nNumRats; i++ ) {
			if( _pLevelData->papRats[i]->IsInWorld() &&
				!_pLevelData->papRats[i]->IsDeadOrDying() &&
				_pLevelData->papRats[i]->GetSplinePath() ) {
				
				fMaxSpeed = _pLevelData->papRats[i]->GetMaxUnitSpeed();
				_pLevelData->papRats[i]->SetMaxUnitSpeed( fMaxSpeed * 0.33f, FALSE );
				_pLevelData->papRats[i]->SetSplinePath( NULL );
			}
		}
	}
}

void mg_HoldYourGround_Restore( void ) {

	if( _pLevelData ) {
		// since there are no check points, just restart
		_pLevelData->fTimer = 0.0f;	
		_pLevelData->fTimeInWave = 0.0f;
		_pLevelData->fTimeTillNextSpawn = 0.0f;
		_pLevelData->nWaveNum = 0;
		_pLevelData->nState = _STATE_WAIT_TO_GET_INTO_GUN;
		_pLevelData->nLastSpawnIndex = 0;

		if( _pLevelData->pAlarmEmitter ) {
			_pLevelData->pAlarmEmitter->Destroy();
			_pLevelData->pAlarmEmitter = NULL;
		}
		_pLevelData->fAlarmVolume = 0.0f;

		_pLevelData->nWalkState = _WALK_STATES_START;
#if FANG_WINGC
		_pLevelData->bDefenseIntroFinished = FALSE;
#endif
		
		u32 i;
		for( i=0; i < _pLevelData->nNumRats; i++ ) {
            _pLevelData->pafRatSinkY[i] = 0.0f;
		}
#if _VERBOSE_DEBUGGING_MSG
		DEVPRINTF( "MG_HoldYourGround_Work() : restoring to beginning of level\n" );
#endif
		_ResetAllSpawnPtConvoys();
	}
}


//////////////////////
// private functions:

// returns the index of the first entity that is not in the world currently
// -1 means that all entities were in the world
static s32 _FindAnAvailableEntityIndex( CEntity **papEntities, u32 nNumEntities ) {
	u32 i;

	for( i=0; i < nNumEntities; i++ ) {
		if( !papEntities[i]->IsInWorld() ) {
			return i;
		}
	}
	return -1;
}

static void _UpdateStringName( char *pszString, u32 nStringLen, u32 nIndex ) {
	
	if( nIndex < 10 ) {
		pszString[nStringLen-1] = '0' + nIndex;	
	} else {
		pszString[nStringLen-1] = nIndex / 10;
		pszString[nStringLen] = nIndex % 10;
	}
}

static BOOL _PullAllBotsFromTheWorld( void ) {
	u32 i;
	CBot *pBot;
	BOOL bEveryoneOutOfWorld = TRUE;
	CVehicleRat *pRat;

	for( i=0; i < _pLevelData->nNumRatDrivers; i++ ) {
		pBot = _pLevelData->papRatDrivers[i];
		if( pBot->IsInWorld() ) {
			if( !pBot->IsMarkedForWorldRemove() ) {
				pBot->RemoveFromWorld();
			}
			bEveryoneOutOfWorld = FALSE;
		}
	}
	for( i=0; i < _pLevelData->nNumRats; i++ ) {
		pRat = _pLevelData->papRats[i];
		if( pRat->IsInWorld() ) {
			if( !pRat->IsMarkedForWorldRemove() ) {
				pRat->RemoveFromWorld();
#if _VERBOSE_DEBUGGING_MSG
				DEVPRINTF( "mg_holdyourground : yanking rat %s from the world.\n", pRat->Name() );
#endif
			}
			bEveryoneOutOfWorld = FALSE;
		}
	}

	return bEveryoneOutOfWorld;
}

// sinks the rats under the sand and once they are completely under, removes them (and their driver) from the world
static void _Work_SinkDeadRats( void ) {
	u32 i;
	CVehicleRat *pRat;
	CFVec3A Pos;
	CBot *pDriverBot;

	f32 fAmountToMovePerFrame = 2.0f * FLoop_fPreviousLoopSecs;

	for( i=0; i < _pLevelData->nNumRats; i++ ) {
		pRat = _pLevelData->papRats[i];
		pDriverBot = _pLevelData->papRatDrivers[i];

		pRat->SetSpotLightOff( TRUE );

		if( pRat->IsInWorld() && !pRat->GetDriverBot() ) {
			if( !_pLevelData->papRatDrivers[i]->IsInWorld() ) {
				// the driver isn't in the world, why?  Try to readd him to the world
				Pos.Mul( pRat->MtxToWorld()->m_vRight, 7.0f );
				Pos.Add( pRat->MtxToWorld()->m_vPos );
				_RelocateDriverBot( pDriverBot, Pos );
				continue;
			}
		}		
		
		if( _pLevelData->pafRatSinkY[i] == 0.0f ) {
			// we are not currently sinking, see if we should start
			if( pRat->IsInWorld() ) {
				if( !pRat->IsRealPhysicsRunning() &&
					!pRat->IsMarkedForWorldRemove() ) {
					// start the sink
					_pLevelData->pafRatSinkY[i] = fAmountToMovePerFrame;
					Pos = *pRat->m_PhysObj.GetPosition();
					Pos.y -= fAmountToMovePerFrame;
					pRat->m_PhysObj.SetPosition( &Pos );
					
					// no need to be targetable anymore
					pDriverBot->SetTargetable( FALSE );
					pRat->SetRatTargetable( FALSE );
#if _VERBOSE_DEBUGGING_MSG					
					DEVPRINTF( "mg_holdyourground : Starting to sink rat %s.\n", pRat->Name() );
#endif
				} else if( pRat->IsDeadOrDying() ) {
					// no need to be targetable anymore
					
					pDriverBot->SetTargetable( FALSE );
					pRat->SetRatTargetable( FALSE );
				}
			}
#if 0
			} else if( pRat->IsInWorld() &&
				pRat->IsNotInAnyVolume() &&
				!pRat->IsMarkedForWorldRemove() &&
				pRat->GetDriverBot() ) {
				// the rat is in the world but not in any volume, start sinking it
			//	_pLevelData->pafRatSinkY[i] = fAmountToMovePerFrame;				
			}		
#endif
		} else {
			// we are already sinking, see if we have sunk far enough
			_pLevelData->pafRatSinkY[i] += fAmountToMovePerFrame;
			if( _pLevelData->pafRatSinkY[i] > (pRat->m_pWorldMesh->GetBoundingSphere().m_fRadius * 0.85f) ) {
				pDriverBot->RemoveFromWorld();
				pRat->RemoveFromWorld();				
				_pLevelData->pafRatSinkY[i] = 0.0f;
#if _VERBOSE_DEBUGGING_MSG
				DEVPRINTF( "mg_holdyourground : Finished sinking rat %s.\n", pRat->Name() );
#endif

			} else {
                // pull under the sand
				Pos = *pRat->m_PhysObj.GetPosition();
				Pos.y -= fAmountToMovePerFrame;
				pRat->m_PhysObj.SetPosition( &Pos );			
			}			
		}
	}
}

// returns TRUE if a rat was released into the world
static BOOL _ReleaseRatIfPossible( u32 nSpawnIndex, u32 nPathIndex, f32 fUnitSpeedFactor ) {

	s32 nRatIndex = _FindAnAvailableEntityIndex( (CEntity **)_pLevelData->papRats, _pLevelData->nNumRats );
	if( nRatIndex >= 0 ) {
		CVehicleRat *pRat = _pLevelData->papRats[nRatIndex];
		CBot *pDriverBot = _pLevelData->papRatDrivers[nRatIndex];

		FASSERT( pRat && pDriverBot );
				
		_SpawnPtData_t *pSpawnData = &_pLevelData->aSpawnData[nSpawnIndex];

		// make sure that the spawn pt doesn't have any rats in it already
		if( _AreAnyRatsInSpawnRadius( pSpawnData->pVehicleSpawn->MtxToWorld()->m_vPos,
									  pRat->m_pWorldMesh->GetBoundingSphere().m_fRadius * 1.01f ) ) {
			// too close to another rat, wait for a while
			return FALSE;
		}

		// add the rat driver
		_RelocateDriverBot( pDriverBot, pSpawnData->pBotSpawn->MtxToWorld()->m_vPos );
#if _VERBOSE_DEBUGGING_MSG
		DEVPRINTF( "mg_holdyourground : Recycling Rat '%s' on path = '%s' at speed %.2f from spawn pt %d.\n", pRat->Name(), pSpawnData->papSplines[nPathIndex]->Name(), fUnitSpeedFactor, nSpawnIndex );
#endif

		// recycle the vehicle back into the world
		pRat->RecycleVehicle( *pSpawnData->pVehicleSpawn->MtxToWorld(),
			pSpawnData->papSplines[nPathIndex],
			Player_aPlayer[0].m_pEntityCurrent->Guid(),
			fUnitSpeedFactor );
		pRat->SetRatTargetable( TRUE );
		pDriverBot->SetTargetable( TRUE );
		
		// record that this rat isn't sunk below the sand
		_pLevelData->pafRatSinkY[nRatIndex] = 0.0f;

		// record that this rat was just put into the world
		_pLevelData->pafRatTimeSinceBirth[nRatIndex] = 0.0f;

		return TRUE;
	}
	return FALSE;
}

// returns TRUE if any vehicle is still alive and in the world
// FALSE is all bots are either out of the world or have been killed
static BOOL _AreAnyVehiclesAliveAndInTheWorld( void ) {
	u32 i;
	CBot *pBot;
	BOOL bAllBotsAreDead = TRUE;
	
	for( i=0; i < _pLevelData->nNumRats; i++ ) {
		pBot = _pLevelData->papRats[i];
		if( pBot->IsInWorld() ) {
			if( !pBot->IsDeadOrDying() ) {
				bAllBotsAreDead = FALSE;
			}			
		}
	}

	return !bAllBotsAreDead;
}

static void _Work_Convoys( void ) {
	u32 i;
	_SpawnPtData_t *pSpawnPt;

	for( i=0; i < _NUM_SPAWN_PTS; i++ ) {
		pSpawnPt = &_pLevelData->aSpawnData[i];
		if( !pSpawnPt->bReadyForNewConvoy ) {
			// this convoy needs to have work done on it
			pSpawnPt->fSecsTillNextSpawn -= FLoop_fPreviousLoopSecs;
			if( pSpawnPt->fSecsTillNextSpawn <= 0.0f ) {
				// we've waited long enough between spawns, either spawn or make the spawn pt available for new convoys
				if( !pSpawnPt->nNumRemainingToSpawn ) {
					// we are free to start a new convoy 
					pSpawnPt->bReadyForNewConvoy = TRUE;
#if _VERBOSE_DEBUGGING_MSG
					DEVPRINTF( "mg_holdyourground : Convoy on spawn pt %d finished.\n", i );
#endif
				} else {
					// attempt to spawn a new vehicle
					if( pSpawnPt->bAllowNewPath && 
						_pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fChanceOfDifferentPath > 0.0f && 
						fmath_RandomChance( _pLevelData->aWaveData[_pLevelData->nWaveNum].pInit->fChanceOfDifferentPath ) ) {
						// pick a different path to send this vehicle down
						pSpawnPt->nPathIndex++;
						if( pSpawnPt->nPathIndex >= pSpawnPt->nNumPaths ) {
							// wrap around
							pSpawnPt->nPathIndex = 0;
						}
						pSpawnPt->bAllowNewPath = FALSE;
					}
					if( _ReleaseRatIfPossible( i, pSpawnPt->nPathIndex, pSpawnPt->fSpeedFactorOfEachSpawn ) ) {
						// we released a bot
						pSpawnPt->fSecsTillNextSpawn = pSpawnPt->fSecsBetweenSpawns;
						pSpawnPt->nNumRemainingToSpawn--;
#if _VERBOSE_DEBUGGING_MSG
						DEVPRINTF( "mg_holdyourground : Released a convoy vehicle at spawn pt %d, %d more to go.\n", i, pSpawnPt->nNumRemainingToSpawn );
#endif
					}
				}
			}
		}			
	}
}

static void _ResetAllSpawnPtConvoys( void ) {
	u32 i;

	for( i=0; i < _NUM_SPAWN_PTS; i++ ) {
		_pLevelData->aSpawnData[i].bReadyForNewConvoy = TRUE;
		_pLevelData->aSpawnData[i].nPathIndex = 0;
	}
	_pLevelData->nLastSpawnIndex = 0;
}

static BOOL _Start_NewConvoy( void ) {

	// find a spawn pt available for spawning a new convoy
	u32 nSpawnIndex = fmath_RandomChoice( _NUM_SPAWN_PTS );
	if( nSpawnIndex == _pLevelData->nLastSpawnIndex || !_pLevelData->aSpawnData[nSpawnIndex].bReadyForNewConvoy ) {
		// we picked the same spawn index as last time, try 1 other before failing till the next call	
		nSpawnIndex++;
		if( nSpawnIndex == _NUM_SPAWN_PTS ) {
			nSpawnIndex = 0;
		}
	}
	_SpawnPtData_t *pSpawnPt = &_pLevelData->aSpawnData[nSpawnIndex];
	if( !pSpawnPt->bReadyForNewConvoy ) {
		// couldn't find a new point that was ready to spawn new convoys...maybe next time
		return FALSE;
	}
	
	// determine how many vehicles to spawn, what path to take, what speed each vehicle is at, how much time between spawns
	_WaveInit_t *pWaveInit = _pLevelData->aWaveData[_pLevelData->nWaveNum].pInit;
	pSpawnPt->bReadyForNewConvoy = FALSE;
	pSpawnPt->fSecsBetweenSpawns = fmath_RandomFloatRange( pWaveInit->fMinWaitBetweenConvoy, pWaveInit->fMaxWaitBetweenConvoy );
	pSpawnPt->fSecsTillNextSpawn = 0.0f;
	pSpawnPt->nNumRemainingToSpawn = fmath_RandomRange( pWaveInit->nMinConvoy, pWaveInit->nMaxConvoy );
	pSpawnPt->fSpeedFactorOfEachSpawn = _RandomWithBiasTowardMinValue( 1.0f - _pLevelData->fUnitSuck,
															pWaveInit->fMaxSpeed,
															pWaveInit->fMinSpeed,
															0.70f );
	if( !_pLevelData->nWaveNum ) {
		// for the 1st wave, just pick a random path
		pSpawnPt->nPathIndex = fmath_RandomChoice( pSpawnPt->nNumPaths );
		pSpawnPt->bAllowNewPath = FALSE;
	} else {
		// for all other ways, cycle between paths
		pSpawnPt->nPathIndex++;
		if( pSpawnPt->nPathIndex >= pSpawnPt->nNumPaths ) {
			// wrap around
			pSpawnPt->nPathIndex = 0;
		}
        pSpawnPt->bAllowNewPath = TRUE;
	}	
#if _VERBOSE_DEBUGGING_MSG
	DEVPRINTF( "mg_holdyourground : Starting convoy of %d vehicles at spawn pt %d, with %.2f secs between spawns on spline '%s'.\n", pSpawnPt->nNumRemainingToSpawn, nSpawnIndex, pSpawnPt->fSecsBetweenSpawns, pSpawnPt->papSplines[pSpawnPt->nPathIndex]->Name() );
#endif

	// mark that this was the last spawn point
	_pLevelData->nLastSpawnIndex = nSpawnIndex;

	return TRUE;
}

static BOOL _AreAnyRatsInSpawnRadius( CFVec3A &rPos, f32 fRadius ) {
	u32 i;
	CVehicleRat *pRat;
	CFVec3A Delta;
	fRadius *= 2.0f;
	f32 fR2 = fRadius * fRadius;

	for( i=0; i < _pLevelData->nNumRats; i++ ) {
		pRat = _pLevelData->papRats[i];
		if( pRat->IsInWorld() ) {
			Delta.Sub( rPos, pRat->MtxToWorld()->m_vPos );
			if( Delta.MagSq() <= fR2 ) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

static void _RelocateDriverBot( CBot *pBot, const CFVec3A &rPos ) {
	pBot->Relocate_Xlat_WS( &rPos );
	pBot->AddToWorld();
	pBot->DrawEnable( TRUE, TRUE );
	ai_AssignJob_Wait( pBot->AIBrain(), FALSE, rPos, 0, 0 );
}

// returns TRUE if a rat is alive and in one of the trigger boxes
static BOOL _CheckTriggerBoxForLiveRats( void ) {
	u32 i, j;
	CVehicleRat *pRat;
	
	for( i=0; i < _NUM_BRIDGES; i++ ) {

		for( j=0; j < _pLevelData->nNumRats; j++ ) {
			pRat = _pLevelData->papRats[j];

			if( pRat->IsInWorld() && !pRat->IsDeadOrDying() ) {
				// see if this rat is in the trigger region
				if( _pLevelData->apTriggerBoxes[i]->IsPointWithinBox( pRat->MtxToWorld()->m_vPos ) ) {
					return TRUE;
				}				
			}
		}
	}
	return FALSE;
}

static void _ResetSuckMeterData( void ) {
	u32 i;

	_pLevelData->fUnitSuck = 1.0f;
	for( i=0; i < _pLevelData->nNumRats; i++ ) {
        _pLevelData->pafRatTimeSinceBirth[i] = -1.0f;
	}
	_pLevelData->fRatsKilled = 0.0f;
	_pLevelData->fSecsAlive = 0.0f;
}

static void _UpdateSuckMeter( void ) {
	u32 i;
	CVehicleRat *pRat;

	for( i=0; i < _pLevelData->nNumRats; i++ ) {
		if( _pLevelData->pafRatTimeSinceBirth[i] >= 0.0f ) {
			_pLevelData->pafRatTimeSinceBirth[i] += FLoop_fPreviousLoopSecs;

			pRat = _pLevelData->papRats[i];
			if( !pRat->IsInWorld() || pRat->IsDeadOrDying() ) {
				// we killed another one

				if( _pLevelData->fRatsKilled > 8.0f ) {
					// subtract out 1 of our kills so that the average represents our local history, not the global history
					_pLevelData->fRatsKilled -= 1.0f;
					_pLevelData->fSecsAlive -= (_pLevelData->fUnitSuck * _SUCK_RANGE);
				}
				_pLevelData->fRatsKilled += 1.0f;
				FMATH_CLAMP( _pLevelData->pafRatTimeSinceBirth[i], _EXPERT_SECS, _SUCK_SECS );
				_pLevelData->pafRatTimeSinceBirth[i] -= _EXPERT_SECS;
				
				_pLevelData->fSecsAlive += _pLevelData->pafRatTimeSinceBirth[i];

				_pLevelData->pafRatTimeSinceBirth[i] = -1.0f;
			}
		}
	}

	if( _pLevelData->fRatsKilled > 1.0f ) {
		// start calculating the suck factor
		f32 fUnitAverage = fmath_Div( _pLevelData->fSecsAlive, _pLevelData->fRatsKilled );
		_pLevelData->fUnitSuck = fUnitAverage * _OO_SUCK_RANGE;
	}
}

static f32 _RandomWithBiasTowardMinValue( f32 fUnitBiasFactor, f32 fMin, f32 fMax, f32 fPercentOfRangeToBias ) {
	
	f32 fBiasPercent = fPercentOfRangeToBias * (1.0f - fUnitBiasFactor);
	f32 fMaxPercent = (1.0f-fPercentOfRangeToBias) + fBiasPercent;

	f32 fRandomBiasedPercent = fmath_RandomFloat() * fMaxPercent;

	f32 fNum = FMATH_FPOT( fRandomBiasedPercent, fMin, fMax );
	
	return fNum;
}

static void _MusicWork( void ) {

	if( _pLevelData->hAlarm == FSNDFX_INVALID_FX_HANDLE ) {
		return;
	}
	u32 i;
	CVehicleRat *pRat;
	BOOL bCalc, bRatsInTriggerRegion = FALSE;
	CFVec3A Right, Temp;
	f32 fDot = 0.0f, fNumInRegion = 0.0f, fPan;

	Right.Set( _pLevelData->RightUnitXZ );
	    	
	for( i=0; i < _pLevelData->nNumRats; i++ ) {
		pRat = _pLevelData->papRats[i];

		bCalc = FALSE;
		if( pRat->IsInWorld() && !pRat->IsDeadOrDying() ) {
			// see if this rat is in the trigger region
			if( _pLevelData->pMusicTriggerBox->IsPointWithinBox( pRat->MtxToWorld()->m_vPos ) ) {
				bRatsInTriggerRegion = TRUE;				
				bCalc = TRUE;
			} else if( _pLevelData->pMusicTriggerBox->IsPointWithinBox( _pLevelData->papRatDrivers[i]->MtxToWorld()->m_vPos ) ) {
                // see if the rat driver is in the trigger region
				bRatsInTriggerRegion = TRUE;
				bCalc = TRUE;
			}

			if( bCalc ) {
				Temp.Sub( pRat->MtxToWorld()->m_vPos, _pLevelData->pAAGun->MtxToParent()->m_vPos );
				Temp.UnitizeXZ();
				fDot += Temp.Dot( Right );
				fNumInRegion += 1.0f;
			}
		}
	}

	if( bRatsInTriggerRegion ) {
		fPan = fmath_Div( fDot, fNumInRegion );
		FMATH_CLAMP( fPan, -1.0f, 1.0f );

		if( !_pLevelData->pAlarmEmitter ) {
			_pLevelData->pAlarmEmitter = FSNDFX_ALLOCNPLAY2D( _pLevelData->hAlarm, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );			
			_pLevelData->fAlarmVolume = 1.0f;
		} else if( _pLevelData->fAlarmVolume < 1.0f ) {
			_pLevelData->pAlarmEmitter->SetVolume( 1.0f );
			_pLevelData->fAlarmVolume = 1.0f;
		}

		if( _pLevelData->pAlarmEmitter ) {
            _pLevelData->pAlarmEmitter->SetPan( fPan );
		}

	} else if( _pLevelData->pAlarmEmitter ) {
		// stop the music
		_pLevelData->fAlarmVolume -= (1.0f/1.5f) * FLoop_fPreviousLoopSecs;
		if( _pLevelData->fAlarmVolume < 0.05f ) {
			_pLevelData->pAlarmEmitter->Destroy();
			_pLevelData->pAlarmEmitter = NULL;
			_pLevelData->fAlarmVolume = 0.0f;
		} else { 
			_pLevelData->pAlarmEmitter->SetVolume( _pLevelData->fAlarmVolume );
		}
	}
}

static BOOL _AreAllSpawnPtsUnused( void ) {
	u32 i;

	for( i=0; i < _NUM_SPAWN_PTS; i++ ) {
		if( !_pLevelData->aSpawnData[i].bReadyForNewConvoy ) {
			// since not ready for new convoy, it must be used
			return FALSE;
		}
	}
	return TRUE;
}

// returns TRUE when the bot has walked to the point and jumped into the gun
static BOOL _WalkToJumpPoint( void ) {
	CFVec3A Temp;
#if FANG_WINGC
	CPlayer *pPlayer = &Player_aPlayer[0];
	CHud2 *pIntroHud = CHud2::GetHudForPlayer(0);
	if( _pLevelData->bDefenseIntroFinished ) {
		return TRUE;
	}
#else
	CPlayer *pPlayer = CPlayer::m_pCurrent;
	CHud2 *pIntroHud = CHud2::GetCurrentHud();
#endif
	CBot *pPlayerBot = (CBot *)pPlayer->m_pEntityCurrent;
	FASSERT( pPlayerBot->TypeBits() & ENTITY_BIT_BOT );
	CAIBrain *pBrain = pPlayerBot->AIBrain();

	switch( _pLevelData->nWalkState ) {

	case _WALK_STATES_START:
		// configure the player for a cutscene
#if FANG_WINGC
		for( s32 i=1; i < _pLevelData->nDefensePlayers; ++i ) {
			Player_aPlayer[i].DisableEntityControl();
			Player_aPlayer[i].ZeroControls();
		}
#endif
		pPlayer->DisableEntityControl();
		pPlayerBot->HeadStopLook();
		aibrainman_Activate( pBrain );
		ai_NotifyCutSceneBegin();
		CBot::DisableBotDamageGlobally();
		CBot::SetCutscenePlaying( TRUE );
		game_EnterLetterbox( NULL, TRUE, _pLevelData->bPlayedFirstTime );

		_pLevelData->fTimer = 0.0f;
		_pLevelData->nWalkState = _WALK_STATES_WAIT;
		_pLevelData->pAAGun->EnableEnterMsgDisplay( FALSE );
		break;

	case _WALK_STATES_WAIT:
		_pLevelData->fTimer += FLoop_fPreviousLoopSecs;
		if( _pLevelData->fTimer >= 6.5f || _pLevelData->bPlayedFirstTime ) {
			// tell the player to walk to the jump point
			ai_AssignGoal_Goto( pBrain, _pLevelData->pJumpPoint->MtxToWorld()->m_vPos, 0, _pLevelData->bPlayedFirstTime ? 100 : 50 );
			_pLevelData->nWalkState = _WALK_STATES_WALK;
		}
		break;
		
	case _WALK_STATES_WALK:
		// test for the bot being close enough to the JumpPoint
		Temp.Sub( pPlayerBot->MtxToWorld()->m_vPos, _pLevelData->pJumpPoint->MtxToWorld()->m_vPos );
		if( Temp.MagSq() <= (2.0f * 2.0f) ) {
			// close enough
			_pLevelData->nWalkState = _WALK_STATES_ACTION;
		}
		break;
		
	case _WALK_STATES_ACTION:
		_pLevelData->pAAGun->ActionNearby( pPlayerBot );
		// undo all of the cutscene stuff
		aibrainman_Deactivate( pBrain );
		pPlayer->EnableEntityControl();
		aibrainman_ConfigurePlayerBotBrain( pBrain, 0 );
		ai_NotifyCutSceneEnd();
		pPlayerBot->HeadLook();
		CBot::EnableBotDamageGlobally();
		
		_pLevelData->nWalkState = _WALK_STATES_DONE;
		break;

	case _WALK_STATES_DONE:
		if( pIntroHud->TransmissionMsg_IsDonePlaying() ) {
			CBot::SetCutscenePlaying( FALSE );
			game_LeaveLetterbox();         
			_pLevelData->bPlayedFirstTime = TRUE;
#if FANG_WINGC
			for( s32 i=1; i < _pLevelData->nDefensePlayers; ++i ) {
				Player_aPlayer[i].EnableEntityControl();
			}
			_pLevelData->bDefenseIntroFinished = TRUE;
#endif
			return TRUE;	
		}		
		break;
	}

	return FALSE;
}

