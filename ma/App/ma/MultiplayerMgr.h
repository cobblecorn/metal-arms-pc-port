//////////////////////////////////////////////////////////////////////////////////////
// MultiplayerMgr.h - Multiplayer manager for spawn points etc.
//
// Author: Dan Stanfill, Pinniped Software     
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
// 03/01/03 Stanfill    Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _MultiplayerMgr_H_
#define _MultiplayerMgr_H_ 1

#include "fang.h"
#include "fmath.h"
#include "player.h"
#include "game.h"
#include "ShadowedText.h"

class CBot;

#define MAX_SPAWN_POINTS 20
#define MAX_HILL_COUNT	 10
#define MAX_BUDDIES		  4
#define MP_MSG_LINES	  6
#define MP_MSG_BUFLEN	 64
#define BONUS_MSG_LEN	 64

// Manager to collect, shuffle, and give out start (spawn) points
FCLASS_NOALIGN_PREFIX class CStartPtMgr {
private:
	static CBot*					ms_pSpawningBot;					// Current bot searching for a start point

	CFMtx43A						m_aStartPoints[MAX_SPAWN_POINTS];	// Cached array of start points
	u32								m_anStartIndex[MAX_SPAWN_POINTS];	// Indirect reference into start points for shuffle
	u32								m_nStartPtCount;					// Number of start points filled in
	u32								m_nNextPoint;						// Index of next point to return

	static BOOL		_NearbyBotCheck( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	void			_ShuffleStartPoints( void );

public:
	CStartPtMgr();

	void			InitLevel( void );

	// Get the next start point for spawning/respawning
	const CFMtx43A*	NextPoint( BOOL bShuffleOK = TRUE );

	// Respawn the given player at a new start point. Returns TRUE if the
	// player is successfully respawned.
	BOOL			RespawnBot( CBot* pBot );
};

// Structure defining the state of a given player for ranking
struct RankInfo_t {
	f32		m_fTimeScore;		// Time value, dependent on game type
	u8		m_nPlayerIndex;		// Index of player in player array
	s8		m_nKills;			// Kills attributed to player
	u8		m_nDeaths;			// Number of times player died
	BOOL8	m_bQuit;			// TRUE if player quit the game
};

// Structure defining team scores and ranking. Each array element represents
// one team.
struct TeamRank_t {
	s32		m_anKills[2];			// Number of kills for each team
	u32		m_anDeaths[2];			// Number of deaths for each team
	f32		m_afTimeScore[2];		// Time score for each team
	u32		m_nLeader;				// Which team is winning
};

// Multiplayer game types
enum MPGameType_e {
	MP_GAME_TYPE_MULTIPLAYER,
	MP_GAME_TYPE_DEATHMATCH,
	MP_GAME_TYPE_TIMED,
	MP_GAME_TYPE_KING_OF_THE_HILL,
	MP_GAME_TYPE_REVERSE_TAG,
	MP_GAME_TYPE_TAG
};

// Base class for all multiplayer game types
FCLASS_NOALIGN_PREFIX class CMultiplayerGame {
protected:
	// Define what messages can go in what lines of the game messages
	enum {
		MSG_LINE_TIME_COUNTER,			// Clock for timed games
		MSG_LINE_PLAYER_STATUS,			// Game-dependent status of current player
		MSG_LINE_KILL_DEATH_STATUS,		// You killed blah, or Blah killed you

		MSG_LINE_COUNT
	};

	wchar				m_awszKillDeathMsg[MAX_PLAYERS][MP_MSG_BUFLEN];	// Current kill/death message
	wchar				m_wszBonusMessage[BONUS_MSG_LEN];				// Message to display when a bonus is given
	CShadowedText		m_GameMsgs[MAX_PLAYERS][MSG_LINE_COUNT];		// For general game messages
	CShadowedText		m_PlayerName[MAX_PLAYERS];						// For display of other player names
	CShadowedText		m_BonusMsgArea;									// Description of bonus message
	f32					m_afKillDeathMsgTimer[MAX_PLAYERS];				// Timer for kill/death message
	f32					m_fBonusMessageTime;							// Time remaining for display of bonus message
	s32					m_nBonusPlayer;									// ID of player who just scored a bonus

	void			_DrawBonusMessage( FViewport_t* pVP );

public:
	virtual BOOL	Create( const GameInitInfo_t* pGameInfo );
	virtual void	Destroy( void ) {}

	// The subclass's Work function is responsible for updating the player
	// and team times.
	virtual void	Work( void );
	virtual void	Draw( void );
	virtual BOOL	TimeToQuit( void ) const { return FALSE; }
	virtual BOOL	IsType( MPGameType_e eType) {return eType == MP_GAME_TYPE_MULTIPLAYER; }
	virtual void	NotifyKill( s32 nKiller, s32 nKillee );

	// Indicate whether the player should be highlighted in the score screens
	virtual BOOL	IsPlayerBlessed( s32 nPlayer ) const {return FALSE;}

	// Indicate whether the player has a game-specific indicator over his head
	virtual BOOL	PlayerHasIndicator( s32 nPlayer ) const {return FALSE;}

	virtual void	GetPlayerScore( s32 nPlayer, RankInfo_t* pRankInfo ) {}
	virtual void	GetTeamScores( TeamRank_t* pRankings ) {}

	virtual f32		GetSpeedMultiplier( s32 nPlayerIndex );

	FCLASS_STACKMEM_NOALIGN( CMultiplayerGame );
} FCLASS_NOALIGN_SUFFIX;

// Basic death match game
FCLASS_NOALIGN_PREFIX class CDeathMatch : public CMultiplayerGame {
protected:
	s32				m_nMaxKills;						// If non-zero, exit after someone gets this many kills
	u32				m_nMaxMinutes;						// If non-zero, we exit after this time
	f32				m_fGameTimer;						// Time remaining before exit
	s32				m_nLastKiller;						// Last player to get a kill
	s32				m_nStreakCount;						// How many kills last player has gotten in a row
public:
	virtual BOOL	Create( const GameInitInfo_t* pGameInfo );
	virtual void	Destroy( void );

	// The subclass's Work function is responsible for updating the player
	// and team times.
	virtual void	Work( void );
	virtual void	Draw( void );
	virtual BOOL	TimeToQuit( void ) const;
	virtual BOOL	IsType( MPGameType_e eType) {return (eType == MP_GAME_TYPE_DEATHMATCH) ? TRUE : CMultiplayerGame::IsType(eType); }
	virtual void	NotifyKill( s32 nKiller, s32 nKillee );

	virtual void	GetPlayerScore( s32 nPlayer, RankInfo_t* pRankInfo );
	virtual void	GetTeamScores( TeamRank_t* pRankings );

	FCLASS_STACKMEM_NOALIGN( CDeathMatch );
} FCLASS_NOALIGN_SUFFIX;

// Abstract base class for games that accumulate time to win
FCLASS_NOALIGN_PREFIX class CTimedGame : public CMultiplayerGame {
protected:
	f32			m_afPlayerTimes[MAX_PLAYERS];		// Total time for each player
	f32			m_afTeamTimes[2];					// Total time for each team
	f32			m_fVictoryTime;						// When someone accumulates this much time they win
public:
	virtual BOOL	Create( const GameInitInfo_t* pGameInfo );
	virtual void	Destroy( void ) {}

	// The subclass's Work function is responsible for updating the player
	// and team times.
	virtual BOOL	TimeToQuit( void ) const;
	virtual BOOL	IsType( MPGameType_e eType) {return (eType == MP_GAME_TYPE_TIMED) ? TRUE : CMultiplayerGame::IsType(eType); }

	virtual void	GetPlayerScore( s32 nPlayer, RankInfo_t* pRankInfo );
	virtual void	GetTeamScores( TeamRank_t* pRankings );

	void			DrawTime( s32 nPlayer );

	FCLASS_STACKMEM_NOALIGN( CTimedGame );
} FCLASS_NOALIGN_SUFFIX;

// A Hill game manages information related to a King of the Hill game
FCLASS_NOALIGN_PREFIX class CHillGame : public CTimedGame {
private:
	enum _HillState_e {
		HILL_STATE_EMPTY,		// There are no players currently on the hill
		HILL_STATE_CONTROL,		// There is one player or team on the hill
		HILL_STATE_CONTESTED	// There is more than one player or team on the hill
	};

	CEntity*		m_apTestHills[MAX_HILL_COUNT];	// List of hill test area geometry
	CEntity*		m_apVisHills[MAX_HILL_COUNT];	// List of visible hill geometry
	f32				m_fSwapTime;					// Switch hills when this time elapses
	f32				m_fSwapTimer;					// Timer to test against fSwapTime
	s32				m_nCurrentHill;					// Currently active hill
	s32				m_nHillsLoaded;					// Number of hills in this level
	s32				m_nKing;						// Who is king of the hill (when hill is controlled)
	_HillState_e	m_eHillState;					// Current status of the hill
	BOOL			m_abIsInside[MAX_PLAYERS];		// Flag indicating on hill this frame or not

	void		_TimeCredit(s32 nPlayer, BOOL* pbPlayTick, s32* pnWarning);	// Credit the time for this player and determine sounds to play
	void		_MakeInactive(u32 nHill);			// Make the given hill inactive (hide it)
	void		_MakeActive(u32 nHill);				// Make the given hill active (show and test it)
public:
	virtual BOOL	Create( const GameInitInfo_t *pGameInit );
	virtual void	Work( void );
	virtual void	Draw( void );
	virtual BOOL	IsType( MPGameType_e eType) {return (eType == MP_GAME_TYPE_KING_OF_THE_HILL) ? TRUE : CTimedGame::IsType(eType); }

	// Indicate whether the player should be highlighted in the score screens
	virtual BOOL	IsPlayerBlessed( s32 nPlayer ) const;

	// Parse the hills. This function is static so it can be used to remove
	// hills from the world when we don't have a KOH game. If bRemoveAll is
	// TRUE, removes all hills from the world. If it is FALSE, the hill information
	// is returned in the two provided entity arrays. Returns the number
	// of hills loaded or removed.
	static s32		ParseHills( BOOL bRemoveAll, CEntity** apVisHills = NULL, CEntity** apTestHills = NULL);

	FCLASS_STACKMEM_NOALIGN( CHillGame );
} FCLASS_NOALIGN_SUFFIX;

// Reverse Tag game. Kill someone to become IT. First one to accumulate
// total time as IT wins.
FCLASS_NOALIGN_PREFIX class CReverseTagGame : public CTimedGame {
protected:
	s32			m_nItPlayer;			// ID of player who is "it"

public:
	virtual BOOL	Create( const GameInitInfo_t* pGameInit );
	virtual void	Work( void );
	virtual void	Draw( void );
	virtual BOOL	IsType( MPGameType_e eType ) { return (eType == MP_GAME_TYPE_REVERSE_TAG) ? TRUE : CTimedGame::IsType(eType); }
	virtual void	NotifyKill( s32 nKiller, s32 nKillee );

	// Indicate whether the player should be highlighted in the score screens
	virtual BOOL	IsPlayerBlessed( s32 nPlayer ) const;

	// Indicate whether the player has a game-specific indicator over his head
	virtual BOOL	PlayerHasIndicator( s32 nPlayer ) const {return (nPlayer == m_nItPlayer);}

	virtual f32		GetSpeedMultiplier( s32 nPlayerIndex );

	FCLASS_STACKMEM_NOALIGN( CReverseTagGame );
} FCLASS_NOALIGN_SUFFIX;

// Regular Tag game. Kill someone to make them IT. First one to accumulate
// total time as IT loses.
FCLASS_NOALIGN_PREFIX class CTagGame : public CTimedGame {
protected:
	s32					m_nItPlayer;						// ID of player who is "it"
public:
	virtual BOOL	Create( const GameInitInfo_t* pGameInit );
	virtual void	Work( void );
	virtual void	Draw( void );
	virtual BOOL	TimeToQuit( void ) const;
	virtual BOOL	IsType( MPGameType_e eType ) { return (eType == MP_GAME_TYPE_TAG) ? TRUE : CTimedGame::IsType(eType); }
	virtual void	NotifyKill( s32 nKiller, s32 nKillee );

	// Indicate whether the player should be highlighted in the score screens
	virtual BOOL	IsPlayerBlessed( s32 nPlayer ) const;

	virtual f32		GetSpeedMultiplier( s32 nPlayerIndex );

	FCLASS_STACKMEM_NOALIGN( CTagGame );
} FCLASS_NOALIGN_SUFFIX;

// Multiplayer manager is the main class for managing multiplayer games
FCLASS_NOALIGN_PREFIX class CMultiplayerMgr {
private:
	// Define the multiplayer game states
	enum _MPGameState_e {
		MP_STATE_RUNNING,			// Game is currently running
		MP_STATE_EXITING,			// Exit requested because of max score or time reached
		MP_STATE_EXIT_CONFIRM,		// Ready to exit, waiting for button press

		MP_STATE_UNDEFINED			// sic
	};

	// The menu states for each player
	enum _MPPlayerMenuState_e {
		MP_MENU_STATE_OFF,			// Menus are off for this player
		MP_MENU_STATE_HAS_LEFT,		// Player has left game, no input
		MP_MENU_STATE_STARTING,		// Just entered the menu, time to initialize
		MP_MENU_STATE_RESUME_SEL,	// Menu is on, resume button selected
		MP_MENU_STATE_QUIT_SEL		// Menu is on, quit button selected
	};

	CStartPtMgr						m_StartPts;							// Start (respawn) locations
	CMultiplayerGame*				m_pGame;							// Our game/score handler
	CBot*							m_apBuddyPool[MAX_BUDDIES];			// Our pool of available buddy bots
	RankInfo_t						m_aRankings[MAX_PLAYERS];			// Sorted ranking information for each player
	TeamRank_t						m_TeamRankings;						// Current team rankings if team play
	f32								m_fStateTimer;						// Timer for state transitions
	u32								m_nInitCount;						// Indicate current state of initialization
	Game_Multiplayer_Game_Types_e	m_eGameType;						// Type of multiplayer game being played
	_MPGameState_e					m_eState;							// Current MP game state
	_MPPlayerMenuState_e			m_eMenuStates[MAX_PLAYERS];			// Current menu state for each player
	GameSave_PrimaryWeaponLimit_e	m_ePrimaryRestriction;				// If weapons are restricted, the restriction on the primary weapon
	GameSave_SecondaryWeaponLimit_e	m_eSecondaryRestriction;			// If weapons are restricted, the restriction on the primary weapon
	BOOL							m_bIsSinglePlayer;					// TRUE if this is a single player game
	BOOL							m_bIsTeamGame;						// TRUE if we are playing with teams
	BOOL							m_bPausedGame;						// TRUE if we have paused the game
	BOOL							m_bDrawMarkers;						// Whether we should draw indicators to make it easier to find other players
	BOOL							m_bDrawRadar;						// Whether we should draw the radar
	BOOL							m_bPossessionEnabled;				// TRUE if we have auto-spawning possessable bots
	BOOL							m_bPowerUpUnpossessed;				// TRUE if unpossessed bots should be powered up
	BOOL							m_bFriendlyFireDamage;				// TRUE if we can damage friendly bots
	BOOL							m_bExclusiveText[MAX_PLAYERS];		// TRUE if we want to disable all other text on screen
	u8								m_uUsedColors;						// Bitmask indicating which player colors are in use

	static int	_CompareRankings( const void *pElement1, const void *pElement2 );
	void		_MenuWork( void );
	BOOL		_QuitRequested( void );
	BOOL		_GameTied( void );
	s32			_MakeUniqueColor( s32 nColorIn );

	// Create our typed multiplayer game
	void		_CreateTypedGame( void );

	// Collect ranking information so far and sort
	void		_ComputeRankings( void );

public:
	// Constructor just clears members. It should not make any function calls.
	CMultiplayerMgr();

	// Initialize after game has started. OK to call other systems.
	void			InitSystem( void );

	// Pre-world-load initialization
	void			PreLoadInitLevel( const GameInitInfo_t *pGameInit );

	// Initialize after a new level has been loaded.
	BOOL			PostLoadInitLevel( const GameInitInfo_t *pGameInit );

	// To catch being called in the wrong spot
	void			UninitLevel( void );

	// Setup player for multiplayer, color, etc.
	void			SetupPlayer( s32 nPlayerID );

	// Notify the current game of a kill
	void			NotifyKill( s32 nKiller, s32 nKillee ) {if (m_pGame) m_pGame->NotifyKill(nKiller, nKillee);}

	// Once per frame work function
	void			Work( void );

	// Once per frame draw function for full screen messages
	void			Draw( void );

	// Make a new buddy grunt
	void			SpawnNewBuddy( s32 nBuddyPlayer );

	//=========================================================================
	// These functions should be eliminated in favor of the start point manager
	// being used directly.
	//
	const CFMtx43A*	NextStartPoint( void ) {return m_StartPts.NextPoint();}
	BOOL			RespawnBot( CBot* pBot ) { return m_StartPts.RespawnBot(pBot); }
	//
	//
	// Functions above should be eliminated
	//=========================================================================

	// Indicate whether this game is a single-player level or a multi-player level
	FINLINE BOOL	IsSinglePlayer( void ) const {return m_bIsSinglePlayer;}
#if FANG_WINGC
	// PC local co-op: campaign rules with more than one player. Known before the level loads (the
	// world's bots are built before CPlayer::m_nPlayerCount is set).
	FINLINE BOOL	IsLocalCoop( void ) const {return m_bIsSinglePlayer && m_bLocalCoop;}
	BOOL							m_bLocalCoop;
#endif
	FINLINE BOOL	IsMultiplayer( void ) const {return !m_bIsSinglePlayer;}
	FINLINE BOOL	IsTeamPlay( void ) const { return m_bIsTeamGame; }

	// Indicate whether the game is over
	FINLINE BOOL	IsGameOver( void ) const {return m_eState == MP_STATE_EXITING;}

	// Get the current individual or team ranking information
	RankInfo_t*		GetRankings( void ) { return m_aRankings; }
	TeamRank_t*		GetTeamRankings( void ) {return &m_TeamRankings; }

	// Print ranking information to the screen. Called from the HUD to print
	// in a single player's viewport
	void			PrintMessages( s32 nPlayerIndex );

	// ExclusiveText means that the multiplayer manager wants exclusive use
	// of text on the screen. Use this function to decide whether to draw text or not.
	BOOL			ExclusiveText( s32 nPlayerIndex );
	void			SetExclusiveText( s32 nPlayerIndex, BOOL bExclusive );

	// If the multiplayer manager requests you ignore controls, returns TRUE
	BOOL			IgnoreControls( s32 nPlayerIndex );

	// Start the player menu for the given player
	BOOL			StartMenu( s32 nPlayerIndex );

	// Test to see if the player menu is still active
	BOOL			MenuDone( s32 nPlayerIndex );

	// Let caller know if the given player has quit
	BOOL			HasQuit( s32 nPlayerIndex );

	// Let the caller know what speed multipliers we want on the given player
	f32				GetSpeedMultiplier( s32 nPlayerIndex ) {return m_pGame ? m_pGame->GetSpeedMultiplier(nPlayerIndex) : 1.0f;}

	// Get weapon restriction information
	GameSave_PrimaryWeaponLimit_e	PrimaryWeaponLimit( void ) const { FASSERT(m_nInitCount); return m_ePrimaryRestriction; }
	GameSave_SecondaryWeaponLimit_e	SecondaryWeaponLimit( void ) const { FASSERT(m_nInitCount); return m_eSecondaryRestriction; }

	// Are different features enabled?
	BOOL			MarkersEnabled( void ) const { return m_bDrawMarkers; }
	BOOL			PossessionEnabled( void ) const { return m_bPossessionEnabled; }
	BOOL			PowerUpUnpossessedBots( void ) const { return m_bPowerUpUnpossessed; }
	BOOL			FriendlyFireDamage( void ) const { return m_bFriendlyFireDamage; }

	FCLASS_STACKMEM_NOALIGN( CMultiplayerMgr );
} FCLASS_NOALIGN_SUFFIX;

// Other functions

// Get the color for the given bot. If bRemapForText is TRUE, a black bot
// returns a lighter gray color.
const CFColorRGBA& Multiplayer_PlayerColor(const CBot* pBot, BOOL bRemapForText = FALSE);

// Our one global instance
extern CMultiplayerMgr MultiplayerMgr;

#endif // _MultiplayerMgr_H_
