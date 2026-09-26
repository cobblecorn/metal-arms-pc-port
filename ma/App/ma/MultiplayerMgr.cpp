//////////////////////////////////////////////////////////////////////////////////////
// MultiplayerMgr.cpp - Multiplayer manager for spawn points etc.
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

#include "fang.h"
#include <stdio.h>

#include "game.h"
#include "MultiplayerMgr.h"
#include "entity.h"
#include "botglitch.h"
#include "player.h"
#include "gamepad.h"
#include "frenderer.h"
#include "fsound.h"
#include "fresload.h"
#include "botgrunt.h"
#include "MeshTypes.h"
#include "ShadowedText.h"

#include "AI/AIApi.h"
#include "AI/AIBrain.h"
#include "AI/AIMover.h"
#include "AI/aibtatable.h"

//-----------------------------------------------------------------------------
// Our one global instance
//-----------------------------------------------------------------------------
CMultiplayerMgr MultiplayerMgr;

//-----------------------------------------------------------------------------
// Constants and macros
//-----------------------------------------------------------------------------
#define _BUDDY_REWARD			TRUE	// Whether to get AI buddies as a reward in death match

#define _TIME_COUNTDOWN			10.0f	// How long before exit do we start the countdown
#define _TIME_EXITING			 5.0f	// Delay from exit decision to actual exit

#define _KILL_MSG_TIME			 5.0f	// How long to display "You killed Tootle!" message
#define _BONUS_MSG_TIME			 3.0f	// How long to display time bonus message (tag)
#define _BONUS_MSG_FADE			 1.0f	// How long we should spend fading the bonus message

#define _MP_PLAYER_SPEED_NORMAL		1.25f	// Run speed multiplier for a normal player, multiplayer only
#define _MP_PLAYER_SPEED_SLOW		0.9f	// Run speed multiplier for a slowed-down player
#define _MP_PLAYER_SPEED_TAG_NORMAL	1.0f	// Run speed multiplier for non-IT players in tag
#define _MP_PLAYER_SPEED_TAG_FAST	1.25f	// Run speed for the IT player in tag

#define _UPLEFT						0	// Upper left index
#define _LOWRIGHT					1	// Lower right index

// Event sounds
#define _SND_GRP_KILLING_SPREE	"KillSpree"		// You're just a killing machine... (spree reward)
#define _SND_GRP_HILL_MINUTE	"HillMinute"	// One more minute to win
#define _SND_GRP_HILL_THIRTY	"HillThirty"	// Thirty seconds to win
#define _SND_GRP_HILL_TEN		"HillTen"		// Ten seconds to win
#define _SND_GRP_HILL_MOVE		"HillMove"		// The hill is changing locations
#define _SND_GRP_HILL_TICK		"HillTick"		// Periodic sound tick when hill is possessed
#define _SND_GRP_TAG_BONUS		"TimeBonus"		// Just scored a time bonus from killing someone else
#define _SND_GRP_BALL_DROPPED	"BallDropped"	// No one is IT anymore!

#define _HILL_TICK_FREQUENCY 5					// How often the hill tick should sound (integer, seconds)

// Other resources
#define _INDICATOR_TEXTURE_NAME "tfh1hudall2"
//#define _INDICATOR_GLOW_TEXTURE "tf_1glowlit"

// Colors
#define _MP_COLORS_FIRST	GAMESAVE_MP_COLORS_YELLOW
#define _MP_COLORS_LAST		GAMESAVE_MP_COLORS_BLACK

// Index into indicator parameter arrays
enum _IndicatorType_e {
	INDICATOR_HILL_CENTER = 0,
	INDICATOR_IT_PLAYER,
	INDICATOR_TEAMMATE,

	INDICATOR_NUM_TYPES,		// Number of actual types, not counting low-res versions
	INDICATOR_LOW_OFFSET = INDICATOR_NUM_TYPES,

	INDICATOR_HILL_CENTER_LOW = INDICATOR_NUM_TYPES,
	INDICATOR_IT_PLAYER_LOW,
	INDICATOR_TEAMMATE_LOW,

	INDICATOR_NUM_ICONS			// Number of actual icons
};

// Which string to print out for player placement
static const s32 _RankingString[] = {
	GAMEPHRASE_YOU_WON,
	GAMEPHRASE_SECOND_PLACE,
	GAMEPHRASE_THIRD_PLACE,
	GAMEPHRASE_YOU_LOST,
	GAMEPHRASE_TIED_FOR_FIRST,
	GAMEPHRASE_TIED_FOR_SECOND,
	GAMEPHRASE_TIED_FOR_LAST,
	GAMEPHRASE_TIED_FOR_LAST
};

// U,V coordinates for indicator icons
//ARG - PS2 compiler complains about const
static CFVec2 _IndicatorST[INDICATOR_NUM_ICONS][2] = {
	  // Upper Left							// Lower right
	{ CFVec2(456.0f/511.0f,   3.0f/511.0f), CFVec2(504.0f/511.0f,  45.0f/511.0f) },		// INDICATOR_HILL_CENTER
	{ CFVec2(393.0f/511.0f,  72.0f/511.0f), CFVec2(445.0f/511.0f, 121.0f/511.0f) },		// INDICATOR_IT_PLAYER
	{ CFVec2(386.0f/511.0f,  13.0f/511.0f), CFVec2(451.0f/511.0f,  71.0f/511.0f) },		// INDICATOR_TEAMMATE

	{ CFVec2(459.0f/511.0f,  71.0f/511.0f), CFVec2(502.0f/511.0f,  93.0f/511.0f) },		// INDICATOR_HILL_CENTER_LOW
	{ CFVec2(444.0f/511.0f, 122.0f/511.0f), CFVec2(472.0f/511.0f, 155.0f/511.0f) },		// INDICATOR_IT_PLAYER_LOW
	{ CFVec2(385.0f/511.0f, 128.0f/511.0f), CFVec2(406.0f/511.0f, 153.0f/511.0f) }		// INDICATOR_TEAMMATE_LOW
};

// Apparent height in world of indicators, meters (or whatever the world units are)
static const f32 _IndicatorWorldHeight[INDICATOR_NUM_TYPES] = {
	2.0f,				// INDICATOR_HILL_CENTER
	1.0f,				// INDICATOR_IT_PLAYER
	1.0f				// INDICATOR_TEAMMATE
};

// Minimum screen size of indicators, pixels
static const f32 _IndicatorMinScreenSize[INDICATOR_NUM_TYPES] = {
	8.0f,				// INDICATOR_HILL_CENTER
	12.0f,				// INDICATOR_IT_PLAYER
	12.0f				// INDICATOR_TEAMMATE
};

// Aspect of indicator icons
static const f32 _IndicatorAspectHOW[INDICATOR_NUM_ICONS] = {
	(_IndicatorST[0][1].y - _IndicatorST[0][0].y) / (_IndicatorST[0][1].x - _IndicatorST[0][0].x),		// INDICATOR_HILL_CENTER
	(_IndicatorST[1][1].y - _IndicatorST[1][0].y) / (_IndicatorST[1][1].x - _IndicatorST[1][0].x),		// INDICATOR_IT_PLAYER
	(_IndicatorST[2][1].y - _IndicatorST[2][0].y) / (_IndicatorST[2][1].x - _IndicatorST[2][0].x),		// INDICATOR_TEAMMATE

	(_IndicatorST[3][1].y - _IndicatorST[3][0].y) / (_IndicatorST[3][1].x - _IndicatorST[3][0].x),		// INDICATOR_HILL_CENTER_LOW
	(_IndicatorST[4][1].y - _IndicatorST[4][0].y) / (_IndicatorST[4][1].x - _IndicatorST[4][0].x),		// INDICATOR_IT_PLAYER_LOW
	(_IndicatorST[5][1].y - _IndicatorST[5][0].y) / (_IndicatorST[5][1].x - _IndicatorST[5][0].x)		// INDICATOR_TEAMMATE_LOW
};

// Colors for indicators. Team indicators are given in team color, so that one is ignored
static const CFColorRGBA _IndicatorColors[INDICATOR_NUM_TYPES] = {
	CFColorRGBA(0.8f, 0.6f, 0.0f, 0.8f),		// INDICATOR_HILL_CENTER
	CFColorRGBA(0.3f, 0.9f, 0.6f, 0.8f),		// INDICATOR_IT_PLAYER
	CFColorRGBA(0.3f, 0.9f, 0.6f, 0.8f)			// INDICATOR_TEAMMATE
};

// Colors for Glitch
static const CFColorRGBA _GlitchColors[] = {
	CFColorRGBA(220.0f/255.0f, 168.0f/250.0f,  28.0f/255.0f, 1.0f),		// GAMESAVE_MP_COLORS_YELLOW
	CFColorRGBA( 96.0f/255.0f, 102.0f/255.0f, 221.0f/255.0f, 1.0f),		// GAMESAVE_MP_COLORS_BLUE,
	CFColorRGBA(168.0f/255.0f, 128.0f/255.0f, 199.0f/255.0f, 1.0f),		// GAMESAVE_MP_COLORS_PURPLE,
	CFColorRGBA(179.0f/255.0f,  84.0f/255.0f,  72.0f/255.0f, 1.0f),		// GAMESAVE_MP_COLORS_RED,
	CFColorRGBA( 79.0f/255.0f, 157.0f/255.0f,  90.0f/255.0f, 1.0f),		// GAMESAVE_MP_COLORS_GREEN,
	CFColorRGBA(196.0f/255.0f, 196.0f/255.0f, 196.0f/255.0f, 1.0f),		// GAMESAVE_MP_COLORS_BLACK for indicators
	CFColorRGBA( 64.0f/255.0f,  64.0f/255.0f,  64.0f/255.0f, 1.0f)		// GAMESAVE_MP_COLORS_BLACK for bot color
};

// Colors for messages

// Score header
static const CFColorRGBA	_HdrColor(0.9f, 0.9f, 0.9f, 1.0f);
static const CFColorRGBA	_DullHdrColor(0.5f, 0.5f, 0.5f, 1.0f);

// Unhilighted team colors for text
static const CFColorRGBA	_TeamColors[2] = {	CFColorRGBA( 96.0f/255.0f, 102.0f/255.0f, 221.0f/255.0f, 1.0f),
												CFColorRGBA(179.0f/255.0f,  84.0f/255.0f,  72.0f/255.0f, 1.0f) };

// Highlighted team colors for text
static const CFColorRGBA	_HLTeamColors[2] = { CFColorRGBA(144.0f/255.0f, 153.0f/255.0f, 255.0f/255.0f, 1.0f),
												 CFColorRGBA(255.0f/255.0f, 126.0f/255.0f, 108.0f/255.0f, 1.0f) };

#define _OtherPlayerColor _DullHdrColor
#define _ThisPlayerColor _HdrColor
//static const CFColorRGBA	_OtherPlayerColor(0.667f, 0.667f, 0.0f, 1.0f);
//static const CFColorRGBA	_ThisPlayerColor(1.0f, 1.0f, 1.0f, 1.0f);

//-----------------------------------------------------------------------------
// Local prototypes
//-----------------------------------------------------------------------------
static void _CrawlEntities( const GameInitInfo_t *pGameInit );
static void _GetHMS(f32 fSeconds, int& nHours, int& nMinutes, int& nSeconds);
static s32 _GetWarningSound(u32 nTimeRemaining, s32 nOldWarning);
static void _VictoryDance( void );
static void _StartDance( s32 nPlayer, cchar* pszDance );

FINLINE s32 _GetWarningSound(u32 nTime, u32 nVictoryTime, s32 nOldWarning) {
	return _GetWarningSound( nVictoryTime - nTime, nOldWarning );
}

static void _DrawEdgePointer(f32 fPx, f32 fPy, f32 fHalfWidth, f32 fHalfHeight, const CFColorRGBA* pColor);
static void _DrawIndicator(f32 fLeft, f32 fTop, f32 fWidth, _IndicatorType_e eType, const CFColorRGBA* pColor);
static void _DrawTargetIndicator(s32 nPlayer, const CFVec3A& vPos, _IndicatorType_e eType, const CFColorRGBA* pColor);

FINLINE void _DrawTargetIndicator(s32 nPlayer, const CEntity* pEnt, _IndicatorType_e eType, const CFColorRGBA* pColor) {
	CFVec3A vPos = pEnt->MtxToWorld()->m_vPos;

	if ( pEnt->TypeBits() & ENTITY_BIT_BOT )
		vPos.y += ((CBot*)pEnt)->m_fCollCylinderHeight_WS + 1.0f;
	else
		vPos.y += 6.0f;

	_DrawTargetIndicator(nPlayer, vPos, eType, pColor);
}

//-----------------------------------------------------------------------------
// File Statics
//-----------------------------------------------------------------------------
// Text area definition for individual viewport messages. Each player has to
// have his own copy, because the actual text area definition gets stored with
// the printed text.
static CShadowedText	_MsgHeader[MAX_PLAYERS];
static CShadowedText	_MsgArea[MAX_PLAYERS][MP_MSG_LINES][2];

// Text area definitions for Resume/Quit buttons
static FTextAreaHandle_t _hResumeArea[MAX_PLAYERS];
static FTextAreaHandle_t _hQuitArea[MAX_PLAYERS];

// Text area definition for full screen messages
static CShadowedText	_FullScreenArea;

// Sounds
static CFSoundGroup*	_pHillMoveSound = NULL;		// Sound to play when a KOH hill moves
static CFSoundGroup*	_pHillTickSound = NULL;		// Tick sound every _HILL_TICK_FREQUENCY seconds when hill is in possession
static CFSoundGroup*	_pKillingSpreeSound = NULL;	// Killing spree!
static CFSoundGroup*	_apTimeOutSounds[3] = {NULL,NULL,NULL};	// One minute to win, 30 seconds, 10 seconds
static CFSoundGroup*	_pTagBonusSound = NULL;		// Just scored a time bonus in tag
static CFSoundGroup*	_pDroppedBallSound = NULL;	// No one is IT; next one to kill someone is IT

// Textures
static CFTexInst		_texIndicators;
//static CFTexInst		_texGlow;

// Index into _RankingString for each player
static s32 _RankingIndex[MAX_PLAYERS];

// Spawn point manager
CBot* CStartPtMgr::ms_pSpawningBot = NULL;					// Current bot searching for a start point

//-----------------------------------------------------------------------------
// Constructor just clears members. It should not make any non-system function calls.
CMultiplayerMgr::CMultiplayerMgr()
{
	m_pGame = NULL;
	m_nInitCount = 0;
	m_bIsSinglePlayer = TRUE;
	m_eState = MP_STATE_UNDEFINED;
	m_bPausedGame = FALSE;
	m_eGameType = GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH;
}

//-----------------------------------------------------------------------------
// Initialize after game has started. OK to call other systems.
void CMultiplayerMgr::InitSystem( void )
{
	// We consider ourself uninitialized; until level is initialized
	m_nInitCount = 0;

	m_pGame = NULL;
	m_bIsSinglePlayer = TRUE;
	m_eState = MP_STATE_UNDEFINED;
	m_bPausedGame = FALSE;
	m_eGameType = GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH;

	for (u32 i = 0; i < MAX_BUDDIES; i++) {
		m_apBuddyPool[i] = NULL;
	}

	// File statics

	// Note: the sounds hang around forever, so we could allocate them here,
	// but let's not load them unless they are needed.
	_pHillMoveSound = NULL;
	_pHillTickSound = NULL;
}

//-----------------------------------------------------------------------------
// Initialize what we can before the world is loaded
void CMultiplayerMgr::PreLoadInitLevel( const GameInitInfo_t *pGameInit )
{
	FASSERT(m_nInitCount == 0);
	FASSERT(m_pGame == NULL);

	m_nInitCount++;

	m_eState = MP_STATE_RUNNING;
	m_bPausedGame = FALSE;
	m_uUsedColors = 0;

	u32 i;
	for ( i = 0; i < MAX_PLAYERS; i++ ) {
		m_bExclusiveText[i] = FALSE;
	}

	for ( i = 0; i < MAX_BUDDIES; i++ ) {
		m_apBuddyPool[i] = NULL;
	}

	// Defaults. Make sure these are all good values for single player
	m_bIsSinglePlayer = TRUE;
	m_bIsTeamGame = FALSE;
	m_ePrimaryRestriction = GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT;
	m_eSecondaryRestriction = GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT;

	m_bDrawMarkers = TRUE;
	m_bDrawRadar = FALSE;
	m_bPossessionEnabled = TRUE;
	m_bPowerUpUnpossessed = TRUE;
	m_bFriendlyFireDamage = FALSE;

	if (pGameInit) {
		m_bIsSinglePlayer = pGameInit->bSinglePlayer;
		m_bIsTeamGame = pGameInit->bTeamPlay;
		if (pGameInit->pMultiplayerRules) {
			m_eGameType = (Game_Multiplayer_Game_Types_e)pGameInit->pMultiplayerRules->nGameType;

			m_ePrimaryRestriction = (GameSave_PrimaryWeaponLimit_e)pGameInit->pMultiplayerRules->nLimitPrimaryWeapon;
			m_eSecondaryRestriction = (GameSave_SecondaryWeaponLimit_e)pGameInit->pMultiplayerRules->nLimitSecondaryWeapon;

			m_bDrawMarkers        = (pGameInit->pMultiplayerRules->nFlags & GAMESAVE_RULE_FLAGS_MARKERS_ON) != 0;
			m_bDrawRadar          = (pGameInit->pMultiplayerRules->nFlags & GAMESAVE_RULE_FLAGS_RADAR_ON) != 0;
			m_bPossessionEnabled  = (pGameInit->pMultiplayerRules->nFlags & GAMESAVE_RULE_FLAGS_POSSESSABLE_BOTS) != 0;
			m_bPowerUpUnpossessed = (pGameInit->pMultiplayerRules->nFlags & GAMESAVE_RULE_FLAGS_BOTS_ON_WHEN_NOT_TETHERED) != 0;
			m_bFriendlyFireDamage = (pGameInit->pMultiplayerRules->nFlags & GAMESAVE_RULE_FLAGS_FRIENDLY_FIRE_DAMAGE) != 0;
		}
	}

	// Set up damage profiles appropriately
	CDamage::SwapMultiplayerData( !m_bIsSinglePlayer );
}

//-----------------------------------------------------------------------------
// Re-initialize after a new level has been loaded.
BOOL CMultiplayerMgr::PostLoadInitLevel( const GameInitInfo_t *pGameInit )
{
	FASSERT(m_nInitCount == 1);
	m_nInitCount++;

	m_StartPts.InitLevel();

	// Clear out the player menu states
	s32 nPlayer;
	for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++) {
		m_eMenuStates[nPlayer] = MP_MENU_STATE_OFF;
	}

	// Create our multiplayer game
	if (!m_bIsSinglePlayer) {
		_CreateTypedGame();

		// If we didn't get a game, something is really wrong
		if (m_pGame == NULL) {
			DEVPRINTF("CMultiplayerMgr::PostLoadInitLevel: Unable to create game!!");
			return FALSE;
		}

		// Set up the game. Ignore error for now because our caller can't
		// deal with it without crashing.
		//if ( !m_pGame->Create( pGameInit ) )
		//	return FALSE;
		m_pGame->Create( pGameInit );

		// If we are not playing a King of the Hill game, we need to remove any
		// hills from the world
		if ( !m_pGame->IsType(MP_GAME_TYPE_KING_OF_THE_HILL) )
			CHillGame::ParseHills( TRUE );

		// Deal with vehicle restrictions, set bot teams, etc.
		_CrawlEntities( pGameInit );

		// Set up our indicator textures. For now don't bother checking for
		// errors, just let the game run.
		_texIndicators.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, _INDICATOR_TEXTURE_NAME)) );
//		_texGlow.SetTexDef( (FTexDef_t*)(fresload_Load(FTEX_RESNAME, _INDICATOR_GLOW_TEXTURE)) );

		// Create a pool of AI buddies for rewards
#if _BUDDY_REWARD
		for (nPlayer = 0; nPlayer < MAX_BUDDIES; nPlayer++) {
			CBotGrunt* pGrunt = fnew CBotGrunt;
			if (pGrunt == NULL)
				break;

			char szName[8];
			sprintf(szName, "Buddy%d", nPlayer);
			if ( !pGrunt->Create(-1, FALSE, szName, &CFMtx43A::m_IdentityMtx, "Default") ) {
				fdelete(pGrunt);
				break;
			}

			// Remove buddy from the world for now, and save
			pGrunt->RemoveFromWorld(TRUE);
			m_apBuddyPool[nPlayer] = pGrunt;
		}
#endif
	}

	// Initialize out text message areas for later drawing in individual viewports
	// Remember that these messages may be transformed for different sized windows.

	// Main in-game score screen. This is defined as a table containing
	// seven lines by 3 columns, with one for each player.
	#define __LEFT_EDGE_1	0.125f
	#define __LEFT_EDGE_2	0.650f
	#define __RIGHT_EDGE	0.875f
	#define __LEFT_EDGE_3	__RIGHT_EDGE
	#define __LINE_HEIGHT	0.040f

	f32 fTop = 0.13f;
	FTextArea_t textArea;
	ftext_SetToDefaults(&textArea);
	textArea.fNumberOfLines = 1.0f;
	textArea.ohFont = '1';
	textArea.bVisible = FALSE;

	// The header
	textArea.fUpperLeftX = __LEFT_EDGE_1;
	textArea.fLowerRightX = __RIGHT_EDGE;
	textArea.fUpperLeftY = fTop;
	textArea.fLowerRightY = fTop + 1.25f * __LINE_HEIGHT;
	textArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
	for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
		_MsgHeader[nPlayer].Create(&textArea);

	fTop += (2.0f * __LINE_HEIGHT);

	textArea.ohFont = '8';
	// The individual lines
	for (int nLine = 0; nLine < MP_MSG_LINES; nLine++) {
		textArea.fUpperLeftY = fTop;
		textArea.fLowerRightY = fTop + __LINE_HEIGHT;

		textArea.fUpperLeftX = __LEFT_EDGE_1;
		textArea.fLowerRightX = __LEFT_EDGE_2;
		textArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
		textArea.bFixedWidthFont = FALSE;
		textArea.bNoScale = TRUE;
		for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
			_MsgArea[nPlayer][nLine][0].Create(&textArea);

		textArea.fUpperLeftX = __LEFT_EDGE_2;
		textArea.fLowerRightX = __LEFT_EDGE_3;
		textArea.oHorzAlign = FTEXT_HORZ_ALIGN_RIGHT;
		textArea.bFixedWidthFont = FALSE;
		for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
			_MsgArea[nPlayer][nLine][1].Create(&textArea);

		//textArea.fUpperLeftX = __LEFT_EDGE_3;
		//textArea.fLowerRightX = __RIGHT_EDGE;
		//for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
		//	_hMsgArea[nPlayer][nLine][2] = ftext_Create(&textArea);

		fTop += __LINE_HEIGHT + 0.01f;
	}

	// Text message area for resume and quit buttons
	ftext_SetToDefaults(&textArea);
	textArea.fNumberOfLines = 1.0f;
	textArea.ohFont = '1';
	textArea.fUpperLeftX = __LEFT_EDGE_1;
	textArea.fUpperLeftY = 0.6f;
	textArea.fLowerRightX = 0.5f;
	textArea.fLowerRightY = 0.67f;
	textArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
	textArea.oColorForeground.Set(0.667f, 0.667f, 0.0f, 1.0f);
	textArea.oColorBorder.Set(0.667f, 0.667f, 0.0f, 1.0f);
	for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
		_hResumeArea[nPlayer] = ftext_Create(&textArea);

	textArea.oHorzAlign = FTEXT_HORZ_ALIGN_RIGHT;
	textArea.fUpperLeftX = 0.5f;
	textArea.fLowerRightX = 0.8f;
	for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
		_hQuitArea[nPlayer] = ftext_Create(&textArea);

	// Initialize our text message area for full-screen messages.
	#define __FS_TEXT_HEIGHT 0.05f
	#define __FS_TEXT_CENTER 0.375f
	ftext_SetToDefaults(&textArea);
	textArea.fNumberOfLines = 1.0f;
	textArea.ohFont = '1';
	textArea.fUpperLeftX  = 0.08672f;
	textArea.fLowerRightX = 0.91328f;
	textArea.fUpperLeftY = __FS_TEXT_CENTER - 0.01f - __FS_TEXT_HEIGHT; // - 0.5f * __FS_TEXT_HEIGHT;
	textArea.fLowerRightY = __FS_TEXT_CENTER - 0.01f; // + 0.5f * __FS_TEXT_HEIGHT;
	textArea.bVisible = FALSE;
	textArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	textArea.oColorForeground.Set(0.3f, 0.6f, 0.9f, 1.0f);
	textArea.oColorBackground.Set(0.0f, 0.0f, 0.1f, 1.0f);
	_FullScreenArea.Create( &textArea );

	#undef __FS_TEXT_CENTER
	#undef __FS_TEXT_HEIGHT

	#undef __LEFT_EDGE
	#undef __RIGHT_EDGE

	return TRUE;
}

//-----------------------------------------------------------------------------
// Caution: this can be called when InitLevel hasn't been
void CMultiplayerMgr::UninitLevel( void )
{
	u32 i;
	for (i = 0; i < MAX_BUDDIES; i++) {
		if (m_apBuddyPool[i]) {
			fdelete(m_apBuddyPool[i]);
			m_apBuddyPool[i] = NULL;
		}
	}

	if (m_pGame) {
		m_pGame->Destroy();
		fdelete(m_pGame);
		m_pGame = NULL;
	}
	m_nInitCount = 0;
	m_bIsTeamGame = FALSE;
}

//-----------------------------------------------------------------------------
// Setup the color and other aspects of the player for multiplayer
void CMultiplayerMgr::SetupPlayer( s32 nPlayerID )
{
	FASSERT(m_nInitCount == 2);

	// Set the default color value. Not needed for single player, but we do
	// this just so it is not uninitialized
	Player_aPlayer[nPlayerID].m_nMultiplayerColor = GAMESAVE_MP_COLORS_YELLOW;

	// Don't do anything else in single player
	if (m_bIsSinglePlayer)
		return;

	FASSERT(Player_aPlayer[nPlayerID].m_pEntityCurrent && (Player_aPlayer[nPlayerID].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
	CBot* pBot = (CBot*)(Player_aPlayer[nPlayerID].m_pEntityCurrent);

	// Allow the loader to pick up this bot
	pBot->m_pBotInfo_Gen->uLoaderPickupEnabled = TRUE;

	// Set this bot's speed for multiplayer
	pBot->SetRunSpeed( m_pGame->GetSpeedMultiplier( nPlayerID ) );

	// Setup the team. If it is a team game, set the correct team from the Player array.
	// If it is not team play, put each bot on its own team (helps AI).
	if ( IsTeamPlay() )
		pBot->InitialTeam(Player_aPlayer[nPlayerID].m_nTeamNum);
	else
		pBot->InitialTeam(nPlayerID);

	// Enable or disable radar depending on our options
	if (m_bDrawRadar)
		CHud2::GetHudForPlayer(nPlayerID)->EnableDrawFlags( CHud2::DRAW_RADAR );
	else
		CHud2::GetHudForPlayer(nPlayerID)->DisableDrawFlags( CHud2::DRAW_RADAR );

	// Only change the color of Glitch
	if ( !(pBot->TypeBits() & ENTITY_BIT_BOTGLITCH) || !pBot->GetMesh() )
		return;

	// Get the player profile so we can figure out the color
	s32 nColor = -1;
	if ( IsTeamPlay() )
		nColor = Player_aPlayer[nPlayerID].m_nTeamNum ? GAMESAVE_MP_COLORS_RED : GAMESAVE_MP_COLORS_BLUE;
	else {
		CPlayerProfile *pProfile = Player_aPlayer[nPlayerID].m_pPlayerProfile;
		if ( pProfile )
			nColor = pProfile->m_Data.nColorIndex;

		if ( (nColor < 0) || (nColor > GAMESAVE_MP_COLORS_BLACK) )
			nColor = GAMESAVE_MP_COLORS_YELLOW;

		// Make sure the color is unique
		nColor = _MakeUniqueColor( nColor );
	}

	// Set the color. If the color is black, we need to change the index from the
	// indicator color to the bot color
	s32 nBotColor = (nColor == GAMESAVE_MP_COLORS_BLACK) ? nColor + 1 : nColor;
	pBot->GetMesh()->SetMeshTint(_GlitchColors[nBotColor].fRed, _GlitchColors[nBotColor].fGreen, _GlitchColors[nBotColor].fBlue);
	Player_aPlayer[nPlayerID].m_nMultiplayerColor = nColor;

	// Experiment...
	//SpawnNewBuddy( nPlayerID );
}

//-----------------------------------------------------------------------------
// Once per frame work function
void CMultiplayerMgr::Work( void )
{
	FASSERT(m_nInitCount == 2);

	// If this is a single player game, we have nothing to do
	if (m_bIsSinglePlayer)
		return;

	FASSERT(m_eState < MP_STATE_UNDEFINED);
	switch (m_eState) {
		case MP_STATE_RUNNING:
			// Update our game
			m_pGame->Work();

			// Update the menu states
			_MenuWork();

			// Update our rankings
			_ComputeRankings();

			// Check for game state transitions

			// See if our game wants to quit
			if ( m_pGame->TimeToQuit() || _QuitRequested() ) {
				// Transition to the exiting state
				m_eState = MP_STATE_EXITING;
				m_fStateTimer = _TIME_EXITING;

				// Disable everyone's hud, because the screen is too busy
				for (s32 nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; nPlayer++)
					CHud2::GetHudForPlayer(nPlayer)->SetDrawEnabled(FALSE);

				// Start the Glitch victory animation
				_VictoryDance();
			}

			break;
		case MP_STATE_EXITING:
			m_fStateTimer -= FLoop_fPreviousLoopSecs;

			if (m_fStateTimer <= 0.0f) {
				// All players must be either alive or buried before we exit. 
				// This keeps the game from kicking out instantly when the last death
				// occurs.
				s32 nPlayers = CPlayer::m_nPlayerCount;
				s32 nReadyPlayers = 0;
				for (int i = 0; i < nPlayers; i++) {
					CBot* pBot = (CBot*)Player_aPlayer[i].m_pEntityCurrent;
					if (pBot->IsBuried() || !pBot->IsDeadOrDying())
						nReadyPlayers++;
				}

				// If everyone is ready for exit, bail
				if (nReadyPlayers == nPlayers)
					m_eState = MP_STATE_EXIT_CONFIRM;
			}
			break;
		case MP_STATE_EXIT_CONFIRM:
			{
				s32 nPlayer;
				for (nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; nPlayer++) {
					if( Gamepad_aapSample[Player_aPlayer[nPlayer].m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT ) {
						game_GotoLevel("next");
						break;
					}
				}
			}
			break;
	}
}

//-----------------------------------------------------------------------------
void CMultiplayerMgr::_ComputeRankings()
{
	FASSERT(m_nInitCount == 2);

	// First get and sort the individual rankings
	int i;
	for (i = 0; i < CPlayer::m_nPlayerCount; i++) {
		m_aRankings[i].m_nPlayerIndex = i;
		m_aRankings[i].m_bQuit = (m_eMenuStates[i] == MP_MENU_STATE_HAS_LEFT);
		m_pGame->GetPlayerScore( i, &m_aRankings[i] );
		_RankingIndex[i] = 0;
	}
	fclib_QSort(m_aRankings, CPlayer::m_nPlayerCount, sizeof(RankInfo_t), _CompareRankings);

	// Now look for ties and set a place for each player. The place number is 
	// the actual ranking, plus MAX_PLAYERS if it is tied.
	s32 nPlace = 0;
	BOOL bTiedLast = FALSE;
	for (i = 0; i < (CPlayer::m_nPlayerCount - 1); i++) {
		if (_CompareRankings(&m_aRankings[i], &m_aRankings[i+1]) == 0) {
			_RankingIndex[ m_aRankings[i].m_nPlayerIndex ] = nPlace + MAX_PLAYERS;
			bTiedLast = TRUE;
		}
		else {
			if (bTiedLast)
				_RankingIndex[ m_aRankings[i].m_nPlayerIndex ] = nPlace + MAX_PLAYERS;
			else
				_RankingIndex[ m_aRankings[i].m_nPlayerIndex ] = nPlace;
			nPlace++;
			bTiedLast = FALSE;
		}
	}
	_RankingIndex[ m_aRankings[CPlayer::m_nPlayerCount - 1].m_nPlayerIndex ] = bTiedLast ? nPlace + MAX_PLAYERS : nPlace;

	// Now get the team rankings.
	if (IsTeamPlay()) {
		m_pGame->GetTeamScores( &m_TeamRankings );
	}
}

//-----------------------------------------------------------------------------
// The Draw function is for printing out full screen messages to all players
void CMultiplayerMgr::Draw( void )
{
	FASSERT(m_nInitCount == 2);

	// Let the current game do any drawing it wants to do
	if ( (m_eState == MP_STATE_RUNNING) && m_pGame)
		m_pGame->Draw();

	// If any players have quit, do an overlay for that player. And if any
	// players have a menu up, draw that.
	BOOL bDrewSomething = FALSE;

	for (int i = CPlayer::m_nPlayerCount-1; i >= 0; i--) {
		// If the player has quit, just do a simple overlay with no other info
		if (m_eMenuStates[i] == MP_MENU_STATE_HAS_LEFT) {
			if (!bDrewSomething) {
				bDrewSomething = TRUE;
				frenderer_Push(FRENDERER_DRAW, NULL);
			}
			FViewport_t* pVP = Player_aPlayer[i].m_pViewportOrtho3D;
			fviewport_SetActive(pVP);
			CFXfm::InitStack();
			const CFColorRGBA bgColor(0.0f, 0.0f, 0.1f, 0.8f);
			CFVec3 v0(-pVP->HalfRes.x,  pVP->HalfRes.y, 1.0f);
			CFVec3 v1( pVP->HalfRes.x,  pVP->HalfRes.y, 1.0f);
			CFVec3 v2( pVP->HalfRes.x, -pVP->HalfRes.y, 1.0f);
			CFVec3 v3(-pVP->HalfRes.x, -pVP->HalfRes.y, 1.0f);
			fdraw_Depth_EnableWriting(FALSE);
			fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
			fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);
			fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);
			fdraw_SolidQuad(&v0, &v1, &v2, &v3, &bgColor);

			if ( m_eState != MP_STATE_EXIT_CONFIRM ) {
				// Just reuse the text area for the quit button since we won't be needing it
				FTextArea_t* pTA = ftext_GetAttributes(_hQuitArea[i]);
				pTA->fUpperLeftX = 0.1f;
				pTA->fLowerRightX = 0.9f;
				pTA->oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

				// Format string of form L"%s Quit"
				fviewport_SetActive(Player_aPlayer[i].m_pViewportSafeOrtho3D);
				ftext_Printf(_hQuitArea[i], Game_apwszPhrases[GAMEPHRASE_PLAYER_QUIT], Player_aPlayer[i].m_pwszPlayerName);
			}
		}
		else if ((m_eState == MP_STATE_EXIT_CONFIRM) || (m_eMenuStates[i] >= MP_MENU_STATE_STARTING)) {
			// The game is over or the menu is up, so draw the scores
			if (!bDrewSomething) {
				bDrewSomething = TRUE;
				frenderer_Push(FRENDERER_DRAW, NULL);
			}

			FViewport_t* pVP = Player_aPlayer[i].m_pViewportSafeOrtho3D;
			fviewport_SetActive(pVP);
			CFXfm::InitStack();
			CFXfm xfmScale, xfmTotal;

			// Set up a transform depending on the screen size
			f32 fScale;
			if (CPlayer::m_nPlayerCount > 1)
				fScale = 0.25f * (f32)FVid_Mode.nPixelsAcross;
			else
				fScale = pVP->HalfRes.x;
			xfmScale.BuildScale(fScale);
			xfmTotal.BuildTranslation(0.0f, 0.0f, 1.0f);
			xfmTotal.ReceiveProductOf(xfmTotal, xfmScale);
			xfmTotal.PushModel();

			// Draw messages for this user
			PrintMessages(i);

			CFXfm::PopModel();
		}
	}

	// If the game is waiting for someone to press a button, let them know
	if (m_eState == MP_STATE_EXIT_CONFIRM) {
		fviewport_SetActive(FViewport_pDefaultOrtho);

		_FullScreenArea.SetAlignment( FTEXT_HORZ_ALIGN_CENTER );
		_FullScreenArea.PrintString( Game_apwszPhrases[GAMEPHRASE_PRESS_A_TO_CONTINUE] );
	}

	if (bDrewSomething) {
		frenderer_Pop();
	}
}

//-----------------------------------------------------------------------------
// This PrintMessages function is intended to draw inside a single player's
// viewport. It is called from the hud, and the hud does the correct transformations.
// It is assumed that the draw renderer is active.
void CMultiplayerMgr::PrintMessages( s32 nPlayerIdx )
{
	FASSERT(m_nInitCount == 2);

	int i;
	static cwchar *_pszSelectedTextBlinkAndColor   = L"~B9~C99999999";

	// Draw a dark backdrop to make the text more visible
	const CFColorRGBA bgColor(0.0f, 0.0f, 0.1f, 0.5f);
	f32 fLeft = -0.8f;
	f32 fTop = 0.6f;

	CFVec3 v0( fLeft,  fTop, 1.0f);
	CFVec3 v1(-fLeft,  fTop, 1.0f);
	CFVec3 v2(-fLeft, -fTop, 1.0f);
	CFVec3 v3( fLeft, -fTop, 1.0f);
	fdraw_Depth_EnableWriting(FALSE);
	fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DECAL_AI);
	fdraw_SolidQuad(&v0, &v1, &v2, &v3, &bgColor);

	// Draw a solid line between the header and the scores
	v0.Set( -0.75f, 0.20f, 1.0f );
	v1.Set(  0.75f, 0.20f, 1.0f );
	v2.Set(  0.75f, 0.19f, 1.0f );
	v3.Set( -0.75f, 0.19f, 1.0f );
	fdraw_SolidQuad(&v0, &v1, &v2, &v3, &_HdrColor);

	// Only print the resume and quit buttons if we are running
	if (m_eState == MP_STATE_RUNNING) {
		// Print the "Resume" and "Quit" buttons
		if (m_eMenuStates[nPlayerIdx] == MP_MENU_STATE_RESUME_SEL) {
			Hud2_XFormPrintf(_hResumeArea[nPlayerIdx], L"%ls%ls",
				_pszSelectedTextBlinkAndColor, Game_apwszPhrases[GAMEPHRASE_RESUME_BUTTON]);
			Hud2_PrintString(_hQuitArea[nPlayerIdx], Game_apwszPhrases[GAMEPHRASE_QUIT_BUTTON]);
		}
		else {
			Hud2_PrintString(_hResumeArea[nPlayerIdx], Game_apwszPhrases[GAMEPHRASE_RESUME_BUTTON]);
			Hud2_XFormPrintf(_hQuitArea[nPlayerIdx], L"%ls%ls",
				_pszSelectedTextBlinkAndColor, Game_apwszPhrases[GAMEPHRASE_QUIT_BUTTON]);
		}
	}

	// Print out the total team scores if appropriate
	if (IsTeamPlay()) {
		s32 nTeam = m_TeamRankings.m_nLeader;

		if (nTeam < 0) {
			_MsgHeader[nPlayerIdx].SetColor( _DullHdrColor );
			_MsgHeader[nPlayerIdx].Xform_PrintString( Game_apwszPhrases[GAMEPHRASE_TEAMS_TIED] );
		}
		else {

			// If the score is final, just say who won
			if (m_eState != MP_STATE_RUNNING) {
				// This player's team color
				_MsgHeader[nPlayerIdx].SetColor( _TeamColors[Player_aPlayer[nPlayerIdx].m_nTeamNum] );

				if (Player_aPlayer[nPlayerIdx].m_nTeamNum == nTeam)
					_MsgHeader[nPlayerIdx].Xform_Printf( L"~B0%ls", Game_apwszPhrases[GAMEPHRASE_YOUR_TEAM_WON] );
				else
					_MsgHeader[nPlayerIdx].Xform_PrintString( Game_apwszPhrases[GAMEPHRASE_YOUR_TEAM_LOST] );
			}
			else {
				// Winning team color
				_MsgHeader[nPlayerIdx].SetColor( _TeamColors[nTeam] );

				if (m_pGame->IsType(MP_GAME_TYPE_TIMED)) {
					int nHours, nMinutes, nSeconds;
					_GetHMS(m_TeamRankings.m_afTimeScore[nTeam], nHours, nMinutes, nSeconds);
					_MsgHeader[nPlayerIdx].Xform_Printf( Game_apwszPhrases[GAMEPHRASE_TIME_BLU_AHEAD + nTeam],
									nMinutes, nSeconds );
				}
				else {
					_MsgHeader[nPlayerIdx].Xform_Printf( Game_apwszPhrases[GAMEPHRASE_DM_BLU_AHEAD + nTeam],
									m_TeamRankings.m_anKills[nTeam] );
				}
			}
		}
	}
	else if (m_eState != MP_STATE_RUNNING) {
		cwchar* pwszPlaceString = Game_apwszPhrases[ _RankingString[ _RankingIndex[nPlayerIdx] ] ];

		// Not team play, but the game is exiting, so print who won
		if ( (_RankingIndex[nPlayerIdx] == 0) || (_RankingIndex[nPlayerIdx] == MAX_PLAYERS) ) {
			_MsgHeader[nPlayerIdx].SetColor( _HdrColor );
			_MsgHeader[nPlayerIdx].Xform_Printf( L"~B9%ls", pwszPlaceString );
		}
		else {
			_MsgHeader[nPlayerIdx].SetColor( _DullHdrColor );
			_MsgHeader[nPlayerIdx].Xform_PrintString( pwszPlaceString );
		}
	}

	int nLine = 0;		// Current line we are printing

	// Header line for player scores
	_MsgArea[nPlayerIdx][nLine][0].SetColor(_HdrColor);
	_MsgArea[nPlayerIdx][nLine][1].SetColor(_HdrColor);

	_MsgArea[nPlayerIdx][nLine][0].Xform_PrintString( Game_apwszPhrases[GAMEPHRASE_HDR_PLAYER] );
	if (m_pGame->IsType(MP_GAME_TYPE_TIMED))
		_MsgArea[nPlayerIdx][nLine][1].Xform_PrintString( Game_apwszPhrases[GAMEPHRASE_HDR_TIME] );
	else {
		_MsgArea[nPlayerIdx][nLine][1].Xform_PrintString( Game_apwszPhrases[GAMEPHRASE_HDR_KILLS] );
	}
	nLine++;

	nLine++;

	// Print out the individual player scores
	for (i = 0; i < CPlayer::m_nPlayerCount; i++) {
		if (IsTeamPlay()) {
			s32 nTeam = Player_aPlayer[m_aRankings[i].m_nPlayerIndex].m_nTeamNum;
			if (m_aRankings[i].m_nPlayerIndex == nPlayerIdx) {
				_MsgArea[nPlayerIdx][nLine][0].SetColor( _HLTeamColors[nTeam] );
				_MsgArea[nPlayerIdx][nLine][1].SetColor( _HLTeamColors[nTeam] );
			}
			else {
				_MsgArea[nPlayerIdx][nLine][0].SetColor( _TeamColors[nTeam] );
				_MsgArea[nPlayerIdx][nLine][1].SetColor( _TeamColors[nTeam] );
			}
		}
		else {
			if (m_aRankings[i].m_nPlayerIndex == nPlayerIdx) {
				_MsgArea[nPlayerIdx][nLine][0].SetColor( _ThisPlayerColor );
				_MsgArea[nPlayerIdx][nLine][1].SetColor( _ThisPlayerColor );
			}
			else {
				_MsgArea[nPlayerIdx][nLine][0].SetColor( _OtherPlayerColor );
				_MsgArea[nPlayerIdx][nLine][1].SetColor( _OtherPlayerColor );
			}
		}

		FASSERT(Player_aPlayer[m_aRankings[i].m_nPlayerIndex].m_pwszPlayerName);
		
		if (m_pGame->IsPlayerBlessed( m_aRankings[i].m_nPlayerIndex )) {
			f32 fTop = 0.75f - 2.0f * _MsgArea[nPlayerIdx][nLine][0].GetTop();
			_DrawIndicator( -0.78f, fTop, 0.07f, INDICATOR_HILL_CENTER_LOW, _MsgArea[nPlayerIdx][nLine][0].GetColor() );
		}
		_MsgArea[nPlayerIdx][nLine][0].Xform_PrintString( Player_aPlayer[m_aRankings[i].m_nPlayerIndex].m_pwszPlayerName );
		if ( m_pGame->IsType( MP_GAME_TYPE_TIMED ) ) {
			int nHours, nMinutes, nSeconds;
			_GetHMS( m_aRankings[i].m_fTimeScore, nHours, nMinutes, nSeconds );
			_MsgArea[nPlayerIdx][nLine][1].Xform_Printf( Game_apwszPhrases[GAMEPHRASE_TIME_FORMAT], nHours, nMinutes, nSeconds );
		}
		else {	// Deathmatch
			_MsgArea[nPlayerIdx][nLine][1].Xform_Printf( L"%d", (int)m_aRankings[i].m_nKills);
		}

		nLine++;
	}
}

//-----------------------------------------------------------------------------
// ExclusiveText means that the multiplayer manager wants exclusive use
// of text on the screen. Use this function to decide whether to draw text or not.
// A return value of TRUE means that the multiplayer manager doesn't want anyone
// else to draw text on the screen.
BOOL CMultiplayerMgr::ExclusiveText( s32 nPlayerIndex )
{
	// Never disallow in single player
	if (m_bIsSinglePlayer)
		return FALSE;

	// If the game is over, request that no one draw
	if (m_eState != MP_STATE_RUNNING)
		return TRUE;

	// If this player has a menu up or has quit request no text draw
	if ( m_eMenuStates[nPlayerIndex] != MP_MENU_STATE_OFF )
		return TRUE;

	// Finally return the current state of our exclusive flag
	return m_bExclusiveText[nPlayerIndex];
}

//-----------------------------------------------------------------------------
void CMultiplayerMgr::SetExclusiveText( s32 nPlayerIndex, BOOL bExclusive )
{
	m_bExclusiveText[nPlayerIndex] = bExclusive;
}

//-----------------------------------------------------------------------------
// If the multiplayer manager requests you ignore controls, returns TRUE
BOOL CMultiplayerMgr::IgnoreControls( s32 nPlayerIndex )
{
	// Never ignore in single player
	if (m_bIsSinglePlayer)
		return FALSE;

	// If the game is over, please ignore controls
	if (m_eState != MP_STATE_RUNNING)
		return TRUE;

	// A quit player can do nothing
	if ( m_eMenuStates[nPlayerIndex] == MP_MENU_STATE_HAS_LEFT )
		return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
// Start the player menu for the given player
BOOL CMultiplayerMgr::StartMenu( s32 nPlayerIndex )
{
	FASSERT(m_nInitCount == 2);

	FASSERT(nPlayerIndex >= 0);
	FASSERT(m_eMenuStates[nPlayerIndex] == MP_MENU_STATE_OFF);

	m_eMenuStates[nPlayerIndex] = MP_MENU_STATE_STARTING;

	// Turn off weapon selection and don't let this bot move
	CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIndex);
	pHud->SetWSEnable( FALSE );
	pHud->SetDrawEnabled( FALSE );
	FASSERT(Player_aPlayer[nPlayerIndex].m_pEntityCurrent && (Player_aPlayer[nPlayerIndex].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
	((CBot*)(Player_aPlayer[nPlayerIndex].m_pEntityCurrent))->ImmobilizeBot();
	return TRUE;
}

//-----------------------------------------------------------------------------
// Test to see if the player menu is still active
BOOL CMultiplayerMgr::MenuDone( s32 nPlayerIndex )
{
	FASSERT(m_nInitCount == 2);
	FASSERT(nPlayerIndex >= 0);

	if (m_bIsSinglePlayer)
		return TRUE;

	return (m_eMenuStates[nPlayerIndex] == MP_MENU_STATE_OFF);
}

//-----------------------------------------------------------------------------
// Let caller know if the given player has quit
BOOL CMultiplayerMgr::HasQuit( s32 nPlayerIndex )
{
	FASSERT(m_nInitCount == 2);
	FASSERT(nPlayerIndex >= 0);

	if (m_bIsSinglePlayer)
		return FALSE;

	return (m_eMenuStates[nPlayerIndex] == MP_MENU_STATE_HAS_LEFT);
}

//-----------------------------------------------------------------------------
// Make a new buddy grunt
void CMultiplayerMgr::SpawnNewBuddy( s32 nBuddyPlayer )
{
	// Find a spare buddy
	CBot* pBuddy = NULL;
	s32 i;
	for (i = 0; i < MAX_BUDDIES; i++) {
		if (m_apBuddyPool[i] && !m_apBuddyPool[i]->IsInWorld()) {
			pBuddy = m_apBuddyPool[i];
			break;
		}
	}

	// Just bail if no buddies are available
	if (pBuddy == NULL)
		return;

	// Place the buddy in the world
//	if ( pBuddy->IsDead() )  //pgm commented out since AddToWorld() does this automatically and doing it early scares me a bit.
//		pBuddy->UnDie();

	pBuddy->AddToWorld();
	pBuddy->SetMaxHealth();


	u32 nColor = Player_aPlayer[nBuddyPlayer].m_nMultiplayerColor;
	if (nColor == GAMESAVE_MP_COLORS_BLACK)
		nColor++;
	pBuddy->GetMesh()->SetMeshTint(_GlitchColors[nColor].fRed, _GlitchColors[nColor].fGreen, _GlitchColors[nColor].fBlue);

	pBuddy->InitialTeam( ((CBot*)(Player_aPlayer[nBuddyPlayer].m_pEntityOrig))->GetOrigTeam() );

	// Set up the AI
	CAIBrain *pBrain = pBuddy->AIBrain();
	ai_ChangeBuddyCtrl(pBrain, AI_BUDDY_CTRL_AUTO);

	ai_SetRace(pBrain, AIRACE_DROID);
	ai_FollowLeader(pBrain, Player_aPlayer[nBuddyPlayer].m_pEntityOrig->Guid(), 0);

	pBrain->AttribStack_Push( CAIBrain::ATTRIB_AGGRESSION, 100 );
	pBrain->AttribStack_Push( CAIBrain::ATTRIB_INTELLIGENCE, 100 );
	pBrain->AttribStack_Push( CAIBrain::ATTRIB_COURAGE, 100 );
	if( pBrain->GetAIMover()->GetWeaponCtrlPtr(0) ) {
		pBrain->GetAIMover()->GetWeaponCtrlPtr(0)->m_fAccuracy = 1.0f;
	}

	m_StartPts.RespawnBot( pBuddy );
}

//-----------------------------------------------------------------------------
void CMultiplayerMgr::_MenuWork( void )
{
	FASSERT(m_nInitCount == 2);

	s32	nPlayerCount = CPlayer::m_nPlayerCount;
	s32 nMenuScreenCount = 0;	// How many of the players have the menu up

	for (s32 nPlayerID = 0; nPlayerID < nPlayerCount; nPlayerID++) {
		if (m_eMenuStates[nPlayerID] < MP_MENU_STATE_STARTING)
			continue;

		u32 nControllerID = Player_aPlayer[nPlayerID].m_nControllerIndex;
		GamepadMap_e eLastMapping = gamepad_GetMapping(nControllerID);

		// Get the key presses
		gamepad_SetMapping(nControllerID, GAMEPAD_MAP_MENU);
		BOOL bCancel = ((Gamepad_aapSample[nControllerID][GAMEPAD_MENU_BACK]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT) ||
						(Gamepad_aapSample[nControllerID][GAMEPAD_MENU_START]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT));
		BOOL bToggle = ((Gamepad_aapSample[nControllerID][GAMEPAD_MENU_LEFT_ANALOG_X]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY) ||
						(Gamepad_aapSample[nControllerID][GAMEPAD_MENU_DPAD_X]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY));
		BOOL bExecute = (Gamepad_aapSample[nControllerID][GAMEPAD_MENU_ACCEPT]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT);
		gamepad_SetMapping(nControllerID, eLastMapping);

		// Now deal with it
		switch (m_eMenuStates[nPlayerID]) {
			case MP_MENU_STATE_STARTING:
				// Ignore input if we are just starting, and transition for the next frame
				m_eMenuStates[nPlayerID] = MP_MENU_STATE_RESUME_SEL;
				break;
			case MP_MENU_STATE_RESUME_SEL:
				// If the resume button is selected, take execute over toggle
				if (bExecute || bCancel) {
					m_eMenuStates[nPlayerID] = MP_MENU_STATE_OFF;
				}
				else if (bToggle) {
					m_eMenuStates[nPlayerID] = MP_MENU_STATE_QUIT_SEL;
				}
				break;
			case MP_MENU_STATE_QUIT_SEL:
				// If the quit button is selected, take toggle over execute
				if (bCancel) {
					m_eMenuStates[nPlayerID] = MP_MENU_STATE_OFF;
				}
				else if (bToggle) {
					m_eMenuStates[nPlayerID] = MP_MENU_STATE_RESUME_SEL;
				}
				else if (bExecute) {
					if (Player_aPlayer[nPlayerID].m_pEntityCurrent != Player_aPlayer[nPlayerID].m_pEntityOrig)
						((CBot*)Player_aPlayer[nPlayerID].m_pEntityCurrent)->ForceQuickDataPortUnPlug();
					Player_aPlayer[nPlayerID].m_pEntityOrig->Die();
					m_eMenuStates[nPlayerID] = MP_MENU_STATE_HAS_LEFT;
				}
				break;
			default:
				FASSERT_NOW;
		}

		// If we have transitioned off, re-enable weapons select and motion
		if (m_eMenuStates[nPlayerID] == MP_MENU_STATE_OFF) {
			CHud2* pHud = CHud2::GetHudForPlayer(nPlayerID);
			pHud->SetWSEnable(TRUE);
			pHud->SetDrawEnabled(TRUE);
			FASSERT(Player_aPlayer[nPlayerID].m_pEntityCurrent && (Player_aPlayer[nPlayerID].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
			((CBot*)(Player_aPlayer[nPlayerID].m_pEntityCurrent))->MobilizeBot();
		}

		// If we are still in the menu, count this screen for the pause check below
		if (m_eMenuStates[nPlayerID] > MP_MENU_STATE_STARTING)
			nMenuScreenCount++;
	}

	// If all the players have their menus up, pause the game
	if (!m_bPausedGame && (nMenuScreenCount == nPlayerCount)) {
		floop_PauseGame(TRUE);
		m_bPausedGame = TRUE;
	}
	else if (m_bPausedGame && (nMenuScreenCount < nPlayerCount)) {
		floop_PauseGame(FALSE);
		m_bPausedGame = FALSE;
	}
}

//-----------------------------------------------------------------------------
// Sort the rankings first by quit status, then time score, then number of kills,
// then by number of deaths. (time scores are always the same if not relevant).
// Return -1 if pA is ahead, 1 if pB is ahead, 0 if they are tied.
int CMultiplayerMgr::_CompareRankings( const void *pElement1, const void *pElement2 )
{
	RankInfo_t* pA = (RankInfo_t*)pElement1;
	RankInfo_t* pB = (RankInfo_t*)pElement2;

	if (pA->m_bQuit != pB->m_bQuit)
		return pA->m_bQuit ? 1 : -1;
	else if (pA->m_fTimeScore > pB->m_fTimeScore)
		return -1;
	else if (pA->m_fTimeScore < pB->m_fTimeScore)
		return 1;
	else if (pA->m_nKills > pB->m_nKills)
		return -1;
	else if (pA->m_nKills < pB->m_nKills)
		return 1;
	else if (pA->m_nDeaths < pB->m_nDeaths)
		return -1;
	else if (pA->m_nDeaths > pB->m_nDeaths)
		return 1;
	else
		return 0;
}

//-----------------------------------------------------------------------------
// A quit is "requested" when all players[teams] have quit except for one, or, if
// there was only one player to begin with, when that player quits
BOOL CMultiplayerMgr::_QuitRequested( void )
{
	FASSERT(m_nInitCount == 2);

	s32 nPlayerCount = CPlayer::m_nPlayerCount;

	if (IsTeamPlay() && (nPlayerCount > 1)) {
		// For team play, we consider a quit requested when all members of one
		// team have quit.
		s32 nRequests[2] = {0, 0};
		s32 nPlayers[2] = {0, 0};

		for (int i = nPlayerCount - 1; i >= 0; i--) {
			s32 nTeam = Player_aPlayer[i].m_nTeamNum;
			nPlayers[nTeam]++;
			if (m_eMenuStates[i] == MP_MENU_STATE_HAS_LEFT)
				nRequests[nTeam]++;
		}

		// Bit of a cheat here; if one of the teams has quit, mark the other 
		// as the winner
		if ((nRequests[0] > 0) && (nRequests[0] == nPlayers[0])) {
			m_TeamRankings.m_nLeader = 1;
			return TRUE;
		}
		else if ((nRequests[1] > 0) && (nRequests[1] == nPlayers[1])) {
			m_TeamRankings.m_nLeader = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else {
		s32 nRequests = 0;
		for (int i = nPlayerCount - 1; i >= 0; i--) {
			if (m_eMenuStates[i] == MP_MENU_STATE_HAS_LEFT)
				nRequests++;
		}
		return (nPlayerCount == 1) ? (nRequests > 0) : (nRequests >= (nPlayerCount - 1));
	}
}

//-----------------------------------------------------------------------------
BOOL CMultiplayerMgr::_GameTied( void )
{
	FASSERT(m_nInitCount == 2);

	if (IsTeamPlay()) {
		return (m_TeamRankings.m_nLeader == -1);
	}
	else {
		return ((CPlayer::m_nPlayerCount > 1) && (_CompareRankings(&m_aRankings[0], &m_aRankings[1]) == 0));
	}
}

//-----------------------------------------------------------------------------
s32 CMultiplayerMgr::_MakeUniqueColor( s32 nColorIn )
{
	FASSERT(m_nInitCount == 2);

	// To avoid an infinite loop, there must be more colors than players
	FASSERT( _MP_COLORS_LAST > MAX_PLAYERS );

	s32 nColorOut = nColorIn;
	while ( (1 << nColorOut) & m_uUsedColors ) {
		nColorOut++;
		if ( nColorOut > _MP_COLORS_LAST )
			nColorOut = _MP_COLORS_FIRST;
	}

	m_uUsedColors |= (1 << nColorOut);
	return nColorOut;
}

//-----------------------------------------------------------------------------
// Create our typed multiplayer game
void CMultiplayerMgr::_CreateTypedGame( void )
{
	FASSERT(m_pGame == NULL);

	if (m_bIsSinglePlayer)
		return;

	switch (m_eGameType) {
		case GAME_MULTIPLAYER_BASE_TYPES_KING_OF_THE_HILL:
			m_pGame = fnew CHillGame;
			break;
		case GAME_MULTIPLAYER_BASE_TYPES_TAG:
			m_pGame = fnew CTagGame;
			break;
		case GAME_MULTIPLAYER_BASE_TYPES_REVERSE_TAG:
			m_pGame = fnew CReverseTagGame;
			break;
		case GAME_MULTIPLAYER_BASE_TYPES_DEATHMATCH:
		default:
			m_pGame = fnew CDeathMatch;
			break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Generic multiplayer game
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CMultiplayerGame::Create( const GameInitInfo_t* /*pGameInfo*/ )
{
	s32 nPlayer;
	
	// Clear out kill/death message
	for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++) {
		fang_MemSet(m_awszKillDeathMsg[nPlayer], 0, MP_MSG_BUFLEN * sizeof(wchar));
		m_afKillDeathMsgTimer[nPlayer] = 0.0f;
	}

	m_nBonusPlayer = -1;
	m_fBonusMessageTime = 0.0f;

	// Register our sounds
	_pTagBonusSound = CFSoundGroup::RegisterGroup( _SND_GRP_TAG_BONUS );
	_pDroppedBallSound = CFSoundGroup::RegisterGroup( _SND_GRP_BALL_DROPPED );

	// Set up the default message text area. Subclasses can override
#define __GAME_LEFT_EDGE	0.03f
#define __GAME_RIGHT_EDGE	0.97f

	f32 fTop = 0.1f;
	FTextArea_t textArea;
	for (s32 n = 0; n < MSG_LINE_COUNT; n++) {
		ftext_SetToDefaults(&textArea);
		textArea.fNumberOfLines = ((n == MSG_LINE_KILL_DEATH_STATUS) ? 2.0f : 1.0f);
		textArea.ohFont = '8';
		textArea.fUpperLeftX = __GAME_LEFT_EDGE;
		textArea.fUpperLeftY = fTop;
		textArea.fLowerRightX = __GAME_RIGHT_EDGE;
		textArea.fLowerRightY = fTop + ((n == MSG_LINE_KILL_DEATH_STATUS) ? 0.12f : 0.06f);
		textArea.bVisible = FALSE;
		textArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
		textArea.oColorForeground.White();
		textArea.bNoScale = TRUE;

		for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
			m_GameMsgs[nPlayer][n].Create(&textArea);

		fTop = textArea.fLowerRightY;
	}
#undef __GAME_LEFT_EDGE
#undef __GAME_RIGHT_EDGE

	// Set up a text area for player names
	ftext_SetToDefaults(&textArea);
	textArea.fNumberOfLines = 1.0f;
	textArea.ohFont = '8';
	textArea.fUpperLeftX = 0.5f;
	textArea.fLowerRightX = 0.97f;
	textArea.fUpperLeftY = 0.20f;
	textArea.fLowerRightY = 0.25f;
	textArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
	textArea.oColorForeground.White();
	textArea.bNoScale = TRUE;
	for (nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
		m_PlayerName[nPlayer].Create(&textArea);



	// Set up the text area for printing bonus messages
	ftext_SetToDefaults(&textArea);
	textArea.fNumberOfLines = 2.0f;
	textArea.bVisible = FALSE;
	textArea.ohFont = '1';
	textArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	textArea.fUpperLeftX = 0.075f;
	textArea.fLowerRightX = 0.925f;
	textArea.fUpperLeftY = 0.375f - 0.08f;
	textArea.fLowerRightY = 0.375f + 0.08f;
	textArea.oColorForeground.Set(0.3f, 0.6f, 0.9f, 1.0f);
	m_BonusMsgArea.Create(&textArea);

	return TRUE;
}

//-----------------------------------------------------------------------------
void CMultiplayerGame::Work( void )
{
	s32 nPlayer;
	
	for (nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; nPlayer++) {
		if (m_afKillDeathMsgTimer[nPlayer] > 0.0f) {
			m_afKillDeathMsgTimer[nPlayer] -= FLoop_fPreviousLoopSecs;
			MultiplayerMgr.SetExclusiveText( nPlayer, TRUE );
		}
		else {
			MultiplayerMgr.SetExclusiveText( nPlayer, FALSE );
		}
	}

	if (m_fBonusMessageTime >= 0.0f)
		m_fBonusMessageTime -= FLoop_fPreviousLoopSecs;
}

//-----------------------------------------------------------------------------
void CMultiplayerGame::Draw( void )
{
	// Draw messages and indicators for each player
	for (s32 nPlayer = CPlayer::m_nPlayerCount; nPlayer--; ) {
		// Don't draw anything if his menu is up
		if ( !MultiplayerMgr.MenuDone(nPlayer) )
			continue;

		// If this is a team game, draw an indicator for each teammate
		if (MultiplayerMgr.IsTeamPlay()) {
			for (s32 nMate = CPlayer::m_nPlayerCount; nMate--; ) {
				if (nMate == nPlayer)
					continue;
				if (Player_aPlayer[nPlayer].m_nTeamNum == Player_aPlayer[nMate].m_nTeamNum) {
					// If the original robot (not the one being possessed) is dead or dying,
					// don't draw it
					if ( !((CBot*)(Player_aPlayer[nMate].m_pEntityOrig))->IsDeadOrDying() && !PlayerHasIndicator(nMate) )
						_DrawTargetIndicator(nPlayer, Player_aPlayer[nMate].m_pEntityCurrent, INDICATOR_TEAMMATE, NULL);
				}
			}
		}

		// If there is someone under our reticle, draw their name.
		// NOTE: the targeted bot does not get cleared when we die, so just
		// don't do this if the player is dead
		CBot* pPlayerBot = (CBot*)Player_aPlayer[nPlayer].m_pEntityCurrent;
		if ( !pPlayerBot->IsDeadOrDying() ) {
			CBot* pTargetedBot = pPlayerBot->GetTargetedBot();
			if ( pTargetedBot ) {
				s32 nTargetedPlayer = pTargetedBot->m_nPossessionPlayerIndex;
				if ( nTargetedPlayer >= 0 ) {
					fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportOrtho3D);
					m_PlayerName[nPlayer].PrintString( Player_aPlayer[nTargetedPlayer].m_pwszPlayerName );
				}
			}
		}

		// The screen is getting too busy when we have both a kill message and a bonus
		// message, so if there is a bonus message specific to this player, don't 
		// draw the kill message
		if ( (m_fBonusMessageTime > 0.0f) && (m_nBonusPlayer == nPlayer) ) {
			_DrawBonusMessage( Player_aPlayer[nPlayer].m_pViewportSafeOrtho3D );
		}
		else if (m_afKillDeathMsgTimer[nPlayer] > 0.0f) {
			// If we are displaying a kill message, display that now
			fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportSafeOrtho3D);
			m_GameMsgs[nPlayer][MSG_LINE_KILL_DEATH_STATUS].PrintString( m_awszKillDeathMsg[nPlayer] );
		}
	}

	// Display any full-screen bonus messages
	if ( (m_fBonusMessageTime > 0.0f) && (m_nBonusPlayer < 0) ) {
		_DrawBonusMessage(FViewport_pDefaultOrtho);
	}
}

//-----------------------------------------------------------------------------
void CMultiplayerGame::_DrawBonusMessage( FViewport_t* pVP )
{
	static const CFColorRGBA colorStart( 0.3f, 0.6f, 0.9f, 1.0f );
	static const CFColorRGBA colorEnd( 0.3f, 0.6f, 0.9f, 0.0f );

	if ( m_fBonusMessageTime > _BONUS_MSG_FADE ) {
		m_BonusMsgArea.SetColor( colorStart );
	}
	else {
		f32 fFade = 1.0f - m_fBonusMessageTime / _BONUS_MSG_FADE;
		fFade = 1.0f - fFade * fFade;

		// There is probably a function somewhere to do this, but I couldn't find it...
		CFColorRGBA textColor;
		textColor.fRed   = fFade * colorStart.fRed   + (1.0f - fFade) * colorEnd.fRed;
		textColor.fGreen = fFade * colorStart.fGreen + (1.0f - fFade) * colorEnd.fGreen;
		textColor.fBlue  = fFade * colorStart.fBlue  + (1.0f - fFade) * colorEnd.fBlue;
		textColor.fAlpha = fFade * colorStart.fAlpha + (1.0f - fFade) * colorEnd.fAlpha;
		m_BonusMsgArea.SetColor( textColor );
	}
	fviewport_SetActive( pVP );
	m_BonusMsgArea.PrintString( m_wszBonusMessage );
}

//-----------------------------------------------------------------------------
f32 CMultiplayerGame::GetSpeedMultiplier( s32 nPlayerIndex )
{
	return (nPlayerIndex >= 0) ? _MP_PLAYER_SPEED_NORMAL : 1.0f;
}

//-----------------------------------------------------------------------------
void CMultiplayerGame::NotifyKill( s32 nKiller, s32 nKillee )
{
	// Don't print anything if it was a non-player bot that got killed
	if (nKillee < 0)
		return;

	if (nKiller == nKillee) {
		_snwprintf(m_awszKillDeathMsg[nKiller], MP_MSG_BUFLEN-1, Game_apwszPhrases[GAMEPHRASE_SUICIDE]);
		m_afKillDeathMsgTimer[nKiller] = _KILL_MSG_TIME;
	}
	else {
		// Fetch a random kill message and put it in the message buffer
		s32 nPhrase = GAMEPHRASE_KILL_1 + fmath_RandomChoice(GAMEPHRASE_KILL_LAST - GAMEPHRASE_KILL_1 + 1);
		FASSERT((wcslen(Game_apwszPhrases[nPhrase]) + wcslen(Player_aPlayer[nKillee].m_pwszPlayerName)) < MP_MSG_BUFLEN);
		_snwprintf(m_awszKillDeathMsg[nKiller], MP_MSG_BUFLEN-1, Game_apwszPhrases[nPhrase], Player_aPlayer[nKillee].m_pwszPlayerName);
		m_afKillDeathMsgTimer[nKiller] = _KILL_MSG_TIME;

		// and a random death message
		nPhrase = GAMEPHRASE_KILLED_1 + fmath_RandomChoice(GAMEPHRASE_KILLED_LAST - GAMEPHRASE_KILLED_1 + 1);
		_snwprintf(m_awszKillDeathMsg[nKillee], MP_MSG_BUFLEN-1, Game_apwszPhrases[nPhrase], Player_aPlayer[nKiller].m_pwszPlayerName);
		m_afKillDeathMsgTimer[nKillee] = _KILL_MSG_TIME;
	}

	// If the killee recruited the killer, the killer is now liberated
	CBot* pBot = (CBot*)Player_aPlayer[nKiller].m_pEntityOrig;
	FASSERT(pBot && (pBot->TypeBits() & ENTITY_BIT_BOT));
	if ( pBot->Recruit_IsRecruited() && (pBot->Recruit_GetRecruiter() == nKillee) ) {
		pBot->Unrecruit();

		// If there isn't already a bonus message up, give him one
		if (m_fBonusMessageTime <= 0.0f) {
			// Cue audio to let the player know something happened
			CFSoundGroup::PlaySound(_pTagBonusSound);

			_snwprintf(m_wszBonusMessage, BONUS_MSG_LEN-1, Game_apwszPhrases[GAMEPHRASE_YOU_ARE_FREE]);
			m_nBonusPlayer = nKiller;
			m_fBonusMessageTime = _BONUS_MSG_TIME;
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Standard Death Match
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CDeathMatch::Create( const GameInitInfo_t* pGameInfo )
{
	CMultiplayerGame::Create( pGameInfo );

	m_nMaxKills = 0;
	m_nMaxMinutes = 0;
	m_nLastKiller = -1;
	m_nStreakCount = 0;

	if (pGameInfo && pGameInfo->pMultiplayerRules) {
		m_nMaxMinutes = pGameInfo->pMultiplayerRules->nTimeLimit;
		m_nMaxKills = pGameInfo->pMultiplayerRules->nDMKillsPerRound;
	}

	if (m_nMaxMinutes)
		m_fGameTimer = 60.0f * m_nMaxMinutes;

	// Register our sounds if necessary
	_pKillingSpreeSound = CFSoundGroup::RegisterGroup( _SND_GRP_KILLING_SPREE );
	_apTimeOutSounds[0] = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_MINUTE );
	_apTimeOutSounds[1] = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_THIRTY );
	_apTimeOutSounds[2] = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_TEN );

	return TRUE;
}

//-----------------------------------------------------------------------------
void CDeathMatch::Destroy( void )
{
}

//-----------------------------------------------------------------------------
void CDeathMatch::Work( void )
{
	CMultiplayerGame::Work();

	// Update our timer if necessary
	if ( m_nMaxMinutes ) {
		u32 nTime1 = (u32)m_fGameTimer;
		m_fGameTimer -= FLoop_fPreviousLoopSecs;
		if (m_fGameTimer < 0.0f)
			m_fGameTimer = 0.0f;	// To make the printout look good
		u32 nTime2 = (u32)m_fGameTimer;
		if (nTime1 != nTime2) {
			s32 nWarning = _GetWarningSound(nTime2, -1);
			if (nWarning >= 0)
				CFSoundGroup::PlaySound( _apTimeOutSounds[nWarning] );
		}
	}
}

//-----------------------------------------------------------------------------
void CDeathMatch::Draw( void )
{
	CMultiplayerGame::Draw();

	// If we are almost out of time, put a countdown on the screen
	if (m_nMaxMinutes && (m_fGameTimer < _TIME_COUNTDOWN)) {
		fviewport_SetActive(FViewport_pDefaultOrtho);
		_FullScreenArea.SetAlignment( FTEXT_HORZ_ALIGN_LEFT );

		// Format string is of form L"Time Remaining: 00:%04.1f"
		_FullScreenArea.Printf( Game_apwszPhrases[GAMEPHRASE_TIME_REMAINING], m_fGameTimer );
	}
}

//-----------------------------------------------------------------------------
void CDeathMatch::NotifyKill( s32 nKiller, s32 nKillee )
{
	CMultiplayerGame::NotifyKill(nKiller, nKillee);

	if ( nKiller == m_nLastKiller ) {
		// Suicide resets to no kills
		if ( nKiller == nKillee ) {
			m_nLastKiller = -1;
			m_nStreakCount = 0;
		}
		
		// If the killer killed an unpossessed bot, leave his current streak
		// count intact but don't give him a credit
		else if (nKillee >= 0) {
			m_nStreakCount++;

			// Check for a killing spree
			if (m_nStreakCount == 3) {
				// Cue audio to let the player know something happened
				CFSoundGroup::PlaySound(_pKillingSpreeSound);
				_snwprintf(m_wszBonusMessage, BONUS_MSG_LEN-1, Game_apwszPhrases[GAMEPHRASE_KILLING_SPREE_1]);
				m_nBonusPlayer = nKiller;
				m_fBonusMessageTime = _BONUS_MSG_TIME;

				// Give him his reward
				FASSERT(Player_aPlayer[nKiller].m_pEntityCurrent && (Player_aPlayer[nKiller].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
				CBot* pBot = (CBot*)(Player_aPlayer[nKiller].m_pEntityCurrent);
				if (pBot->TypeBits() & ENTITY_BIT_BOTGLITCH) {
					((CBotGlitch*)pBot)->DeployBuddy();
				}
			}
#if _BUDDY_REWARD
			else if (m_nStreakCount == 5) {
				CFSoundGroup::PlaySound(_pKillingSpreeSound);
				_snwprintf(m_wszBonusMessage, BONUS_MSG_LEN-1, Game_apwszPhrases[GAMEPHRASE_KILLING_SPREE_2]);
				m_nBonusPlayer = nKiller;
				m_fBonusMessageTime = _BONUS_MSG_TIME;

				// Give him some buddies
				MultiplayerMgr.SpawnNewBuddy(nKiller);
				MultiplayerMgr.SpawnNewBuddy(nKiller);
				MultiplayerMgr.SpawnNewBuddy(nKiller);
			}
#endif
		}
	}
	else if ( (nKiller != nKillee) && (nKillee >= 0) ) {
		m_nLastKiller = nKiller;
		m_nStreakCount = 1;
	}
}

//-----------------------------------------------------------------------------
// Look for exit conditions: max number of kills or time limit reached.
BOOL CDeathMatch::TimeToQuit( void ) const
{
	// Determine whether we have reached the maximum number of kills, if so set.
	// For team play, we test the number of kills for the team, not the player.
	if (m_nMaxKills) {
		s32 nKMax;
		if (MultiplayerMgr.IsTeamPlay()) {
			TeamRank_t* pTeamRankings = MultiplayerMgr.GetTeamRankings();
			s32 nWinner = pTeamRankings->m_nLeader;
			if (nWinner < 0)
				nWinner = 0;
			nKMax = pTeamRankings->m_anKills[nWinner];
		}
		else {
			nKMax = MultiplayerMgr.GetRankings()->m_nKills;
		}
		if (nKMax >= m_nMaxKills)
			return TRUE;
	}

	// Now check to see if we have hit the time limit
	if ( m_nMaxMinutes && (m_fGameTimer <= 0.0f) )
		return TRUE;

	return FALSE;
}

//-----------------------------------------------------------------------------
void CDeathMatch::GetPlayerScore( s32 nPlayer, RankInfo_t* pRankInfo )
{
	FASSERT((nPlayer >= 0) && (nPlayer < CPlayer::m_nPlayerCount));

	s8 nKills;
	u8 nDeaths;
	Player_aPlayer[nPlayer].GetMPScore(nKills, nDeaths);
	pRankInfo->m_nDeaths = nDeaths;
	pRankInfo->m_nKills = nKills;
	pRankInfo->m_fTimeScore = 0.0f;
}

//-----------------------------------------------------------------------------
void CDeathMatch::GetTeamScores( TeamRank_t* pRankings )
{
	pRankings->m_nLeader = CPlayer::GetTeamScores(pRankings->m_anKills, pRankings->m_anDeaths);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// General timed game
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CTimedGame::Create( const GameInitInfo_t* pGameInfo )
{
	CMultiplayerGame::Create( pGameInfo );

	m_fVictoryTime = 0.0f;
	m_afTeamTimes[0] = m_afTeamTimes[1] = 0.0f;
	for (s32 nPlayer = 0; nPlayer < MAX_PLAYERS; nPlayer++)
		m_afPlayerTimes[nPlayer] = 0.0f;

	// The subclass overrides this default value
	m_fVictoryTime = 120.0f;

	return TRUE;
}

//-----------------------------------------------------------------------------
BOOL CTimedGame::TimeToQuit( void ) const
{
	f32 fTestTime = 0.0f;
	if (MultiplayerMgr.IsTeamPlay()) {
		if (m_afTeamTimes[0] > fTestTime)
			fTestTime = m_afTeamTimes[0];
		if (m_afTeamTimes[1] > fTestTime)
			fTestTime = m_afTeamTimes[1];
	}
	else {
		for (s32 i = CPlayer::m_nPlayerCount - 1; i >= 0; i--) {
			if (m_afPlayerTimes[i] > fTestTime)
				fTestTime = m_afPlayerTimes[i];
		}
	}
	return (fTestTime >= m_fVictoryTime);
}

//-----------------------------------------------------------------------------
// Fill in kills and deaths, even though we don't use them in a timed game
void CTimedGame::GetPlayerScore( s32 nPlayer, RankInfo_t* pRankInfo )
{
	FASSERT((nPlayer >= 0) && (nPlayer < CPlayer::m_nPlayerCount));

	s8 nKills;
	u8 nDeaths;
	Player_aPlayer[nPlayer].GetMPScore(nKills, nDeaths);
	pRankInfo->m_nDeaths = nDeaths;
	pRankInfo->m_nKills = nKills;
	pRankInfo->m_fTimeScore = m_afPlayerTimes[nPlayer];
}

//-----------------------------------------------------------------------------
// Fill in kills and deaths, even though we don't use them in a timed game
void CTimedGame::GetTeamScores( TeamRank_t* pRankings )
{
	CPlayer::GetTeamScores(pRankings->m_anKills, pRankings->m_anDeaths);

	pRankings->m_afTimeScore[0] = m_afTeamTimes[0];
	pRankings->m_afTimeScore[1] = m_afTeamTimes[1];

	if (m_afTeamTimes[0] > m_afTeamTimes[1])
		pRankings->m_nLeader = 0;
	else if (m_afTeamTimes[1] > m_afTeamTimes[0])
		pRankings->m_nLeader = 1;
	else
		pRankings->m_nLeader = -1;
}

//-----------------------------------------------------------------------------
void CTimedGame::DrawTime( s32 nPlayer )
{
	int nHours, nMinutes, nSeconds;

	if (MultiplayerMgr.IsTeamPlay()) {
		s32 nTeam = Player_aPlayer[nPlayer].m_nTeamNum;
		_GetHMS(m_afTeamTimes[nTeam], nHours, nMinutes, nSeconds);

		if (IsPlayerBlessed(nPlayer))
			m_GameMsgs[nPlayer][MSG_LINE_TIME_COUNTER].SetColor( _HLTeamColors[ nTeam ] );
		else
			m_GameMsgs[nPlayer][MSG_LINE_TIME_COUNTER].SetColor( _TeamColors[ nTeam ] );
	}
	else {
		_GetHMS(m_afPlayerTimes[nPlayer], nHours, nMinutes, nSeconds);

		if (IsPlayerBlessed(nPlayer))
			m_GameMsgs[nPlayer][MSG_LINE_TIME_COUNTER].SetColor( _ThisPlayerColor );
		else
			m_GameMsgs[nPlayer][MSG_LINE_TIME_COUNTER].SetColor( _OtherPlayerColor );
	}

	m_GameMsgs[nPlayer][MSG_LINE_TIME_COUNTER].Printf( Game_apwszPhrases[GAMEPHRASE_TIME_FORMAT], nHours, nMinutes, nSeconds );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// King of the Hill
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CHillGame::Create( const GameInitInfo_t *pGameInit )
{
	CTimedGame::Create( pGameInit );

	m_fSwapTime = 0.0f;
	m_fSwapTimer = 0.0f;
	m_nCurrentHill = 0;
	m_nHillsLoaded = 0;
	m_eHillState = HILL_STATE_EMPTY;
	m_nKing = -1;
	fang_MemSet(m_apTestHills, 0, MAX_HILL_COUNT * sizeof(CEntity*));
	fang_MemSet(m_apVisHills, 0, MAX_HILL_COUNT * sizeof(CEntity*));
	fang_MemSet(m_abIsInside, 0, MAX_PLAYERS * sizeof(BOOL));

	// Determine whether this is a King of the Hill game and get the parameters
	if (pGameInit) {
		m_fVictoryTime = 60.0f * (f32)pGameInit->pMultiplayerRules->nTimeLimit;
		if (pGameInit->pMultiplayerRules->nTimeBetweenHillSwaps > 0)
			m_fSwapTime = 60.0f * (f32)pGameInit->pMultiplayerRules->nTimeBetweenHillSwaps;
		else
			m_fSwapTime = -1.0f;
	}
	else {
		m_fVictoryTime = 120.0f;
		m_fSwapTime = 60.0f;
	}

	// Get ptrs to all of the hills
	m_nHillsLoaded = ParseHills( FALSE, m_apVisHills, m_apTestHills );

	// If no hills were loaded, we can't go on
	if ( m_nHillsLoaded == 0 ) {
		DEVPRINTF("WARNING: King of the Hill Game requested but no hills found!!!!!\n");
		return FALSE;
	}

	// If there is only one hill loaded, disable the swap check
	if (m_nHillsLoaded == 1) {
		m_fSwapTime = m_fSwapTimer = -1.0f;
		m_nCurrentHill = 0;
		_MakeActive(0);
	}
	else {
		// Start the timer if appropriate
		if (m_fSwapTime > 0.0f) {
			m_fSwapTimer = m_fSwapTime;

			// If the hill swap sound is not already set up, set it up now
			if (_pHillMoveSound == NULL) {
				_pHillMoveSound = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_MOVE );
			}
		}

		// OK, now we need to set up our first hill and hide the others
		m_nCurrentHill = fmath_RandomChoice(m_nHillsLoaded);
		for (s32 nHill = 0; nHill < m_nHillsLoaded; nHill++) {
			if (nHill == m_nCurrentHill)
				_MakeActive(nHill);
			else
				_MakeInactive(nHill);
		}
	}

	// Load required hill sounds
	_pHillTickSound = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_TICK );
	_apTimeOutSounds[0] = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_MINUTE );
	_apTimeOutSounds[1] = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_THIRTY );
	_apTimeOutSounds[2] = CFSoundGroup::RegisterGroup( _SND_GRP_HILL_TEN );

	return TRUE;
}

//-----------------------------------------------------------------------------
// Small utility function for Hill games
static s32 _GetWarningSound(u32 nTimeRemaining, s32 nOldWarning) {
	s32 nWarning;

	switch (nTimeRemaining) {
		case 60:
			nWarning = 0;
			break;
		case 30:
			nWarning = 1;
			break;
		case 10:
			nWarning = 2;
			break;
		default:
			nWarning = -1;
			break;
	}

	return FMATH_MAX( nWarning, nOldWarning );
}

//-----------------------------------------------------------------------------
void CHillGame::Work( void )
{
	CTimedGame::Work();

	// This should never happen in the released game: hill geometry is missing
	if ( m_nHillsLoaded == 0 )
		return;

	FASSERT(m_apTestHills[m_nCurrentHill] && m_apTestHills[m_nCurrentHill]->GetMesh());
	

	// For teams, you get credit if any players are inside, but don't add players together
	BOOL	abTeamInside[2] = {FALSE, FALSE};

	// Count how many are inside so we can track the hill possession state
	s32		nInsideCount = 0;
	s32		nNewKing = -1;			// Who is the new king, if anyone

	BOOL			bPlayTick = FALSE;		// Play the tick sound if anyone passes a give time mark
	BOOL			bPlayWarning = FALSE;	// Play a time warning
	s32				nWarning = -1;			// Which warning to play

	s32 nPlayer;

	// Set everyone as outside first, so we can track people being pulled inside
	// by recruitment
	for (nPlayer = CPlayer::m_nPlayerCount - 1; nPlayer >= 0; nPlayer--)
		m_abIsInside[nPlayer] = FALSE;

	// Test to see if any players are inside the hill
	for (nPlayer = CPlayer::m_nPlayerCount - 1; nPlayer >= 0; nPlayer--) {
		FASSERT(Player_aPlayer[nPlayer].m_pEntityCurrent && (Player_aPlayer[nPlayer].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
		FASSERT(Player_aPlayer[nPlayer].m_nTeamNum <= 2);	// unsigned

		// Make sure we don't count time if we are dead!
		CBot* pBot = (CBot*)(Player_aPlayer[nPlayer].m_pEntityCurrent);
		if (pBot->IsDeadOrDying()) {
			m_abIsInside[nPlayer] = FALSE;
			continue;
		}

		// If we are already flagged as inside, it is because we recruited someone
		// who already tested positive for being inside, and we have been accounted for
		if ( m_abIsInside[nPlayer] )
			continue;

		CFVec3A vStart = pBot->MtxToWorld()->m_vPos;

		// Returns -1 on error, 0 if not inside, 1 if inside
		m_abIsInside[nPlayer] = (m_apTestHills[m_nCurrentHill]->GetMesh()->IsPointInMesh(&vStart, 0.0f) > 0);

		// If the player is currently inside, give him credit for the previous frame
		if (m_abIsInside[nPlayer]) {
			_TimeCredit(nPlayer, &bPlayTick, &nWarning);
			abTeamInside[Player_aPlayer[nPlayer].m_nTeamNum] = TRUE;
			nInsideCount++;
			nNewKing = nPlayer;

			// Now, let's see if this player has been recruited, in which case
			// we need to credit the recruiter also
			s32 nRecruiter = pBot->Recruit_GetRecruiter();
			if (nRecruiter >= 0) {
				m_abIsInside[nRecruiter] = TRUE;
				_TimeCredit(nRecruiter, &bPlayTick, &nWarning);
				abTeamInside[Player_aPlayer[nRecruiter].m_nTeamNum] = TRUE;
				nInsideCount++;
			}
		}
	}

	// If this is team play, add to the team scores and check for team events
	if (MultiplayerMgr.IsTeamPlay()) {
		// For teams, the inside count is based only on the teams
		nInsideCount = 0;
		nNewKing = -1;
		nWarning = -1;	// Reset

		u32 nTime1 = 0;
		u32 nTime2 = 0;
		s32 nDelta = 1000;
		if (abTeamInside[0]) {
			nInsideCount++;
			nNewKing = 0;
			nTime1 = (u32)m_afTeamTimes[0];
			m_afTeamTimes[0] += FLoop_fPreviousLoopSecs;
			nTime2 = (u32)m_afTeamTimes[0];
			if (nTime1 != nTime2) {
				nWarning = _GetWarningSound(nTime2, (u32)m_fVictoryTime, nWarning);
			}
		}
		if (abTeamInside[1]) {
			nInsideCount++;
			nNewKing = 1;
			nTime1 = (u32)m_afTeamTimes[1];
			m_afTeamTimes[1] += FLoop_fPreviousLoopSecs;
			nTime2 = (u32)m_afTeamTimes[1];
			if (nTime1 != nTime2) {
				nWarning = _GetWarningSound(nTime2, (u32)m_fVictoryTime, nWarning);
			}
		}
	}

	// Set the hill state and the new king
	_HillState_e eNewState;
	if (nInsideCount == 0) {
		eNewState = HILL_STATE_EMPTY;
		m_nKing = -1;
	}
	else if (nInsideCount == 1) {
		eNewState = HILL_STATE_CONTROL;
		m_nKing = nNewKing;
	}
	else {
		eNewState = HILL_STATE_CONTESTED;
		m_nKing = -1;
	}

	// If the hill state has changed, let the user know
	if (eNewState != m_eHillState) {

		m_eHillState = eNewState;
	}

	// If we should play a tick sound, do it
	if (bPlayTick)
		CFSoundGroup::PlaySound(_pHillTickSound);

	// And a warning sound
	if (nWarning >= 0)
		CFSoundGroup::PlaySound(_apTimeOutSounds[nWarning]);

	// If our swap time has expired, swap the hill
	if (m_fSwapTime > 0.0f) {
		FASSERT(m_nHillsLoaded > 1);

		m_fSwapTimer -= FLoop_fPreviousLoopSecs;
		if (m_fSwapTimer <= 0.0f) {
			m_fSwapTimer = m_fSwapTime;
			u32 nOldHill = m_nCurrentHill;

			// If we have only 2 hills, always swap
			if (m_nHillsLoaded == 2)
				m_nCurrentHill = 1 - m_nCurrentHill;
			else
				m_nCurrentHill = fmath_RandomChoice(m_nHillsLoaded);

			if (nOldHill != m_nCurrentHill) {
				_MakeInactive(nOldHill);
				_MakeActive(m_nCurrentHill);

				// Play a sound
				CFSoundGroup::PlaySound(_pHillMoveSound);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CHillGame::Draw( void )
{
	CTimedGame::Draw();

	// Let developers know if this level has no hills...
#if !FANG_PRODUCTION_BUILD
	if (m_nHillsLoaded == 0) {
		fviewport_SetActive(FViewport_pDefaultOrtho);
		_FullScreenArea.SetAlignment( FTEXT_HORZ_ALIGN_CENTER );
		_FullScreenArea.PrintString( L"No Hills Loaded!!" );
	}
#endif

	// Should never happen in the final game, but can easily happen during development
	if ( m_nHillsLoaded == 0 )
		return;

	FASSERT(m_apVisHills[m_nCurrentHill]);

	// Pick a color for the hill indicator
	const CFColorRGBA* pColor = NULL;
	if ( m_eHillState == HILL_STATE_CONTROL ) {
		FASSERT(m_nKing >= 0);
		pColor = MultiplayerMgr.IsTeamPlay() ? &_TeamColors[m_nKing] : &_GlitchColors[ Player_aPlayer[m_nKing].m_nMultiplayerColor ];
	}

	// Paint up a marker for the active hill if outside, score if inside
	for (s32 nPlayer = CPlayer::m_nPlayerCount - 1; nPlayer >= 0; nPlayer--) {
		if (MultiplayerMgr.MenuDone(nPlayer)) {
			fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportSafeOrtho3D);
			DrawTime(nPlayer);
			if ( !m_abIsInside[nPlayer])
				_DrawTargetIndicator(nPlayer, m_apVisHills[m_nCurrentHill], INDICATOR_HILL_CENTER, pColor);
		}
	}
}

//-----------------------------------------------------------------------------
// Indicate whether the player should be highlighted in the score screens
BOOL CHillGame::IsPlayerBlessed( s32 nPlayer ) const
{
	return m_abIsInside[nPlayer];
}

//-----------------------------------------------------------------------------
void CHillGame::_TimeCredit(s32 nPlayer, BOOL* pbPlayTick, s32* pnWarning)
{
	u32 nTime1 = (u32)m_afPlayerTimes[nPlayer];
	m_afPlayerTimes[nPlayer] += FLoop_fPreviousLoopSecs;
	u32 nTime2 = (u32)m_afPlayerTimes[nPlayer];

	// If nTime1 and nTime2 are different, we crossed over a second boundary,
	// so check for periodic events
	if (nTime1 != nTime2) {
		if (!(*pbPlayTick)) {
			if ((nTime2 % _HILL_TICK_FREQUENCY) == 0)
				*pbPlayTick = TRUE;
		}
		if (!MultiplayerMgr.IsTeamPlay()) {
			*pnWarning = _GetWarningSound(nTime2, (u32)m_fVictoryTime, *pnWarning);
		}
	}
}

//-----------------------------------------------------------------------------
// Make the given hill inactive (hide it)
void CHillGame::_MakeInactive(u32 nHill)
{
	m_apVisHills[nHill]->DrawEnable( FALSE, TRUE );
}

//-----------------------------------------------------------------------------
// Make the given hill active (show and test it)
void CHillGame::_MakeActive(u32 nHill)
{
	m_apVisHills[nHill]->DrawEnable( TRUE, TRUE );
}

//-----------------------------------------------------------------------------
// Parse the hills. This function is static so it can be used to remove
// hills from the world when we don't have a KOH game. If bRemoveAll is
// TRUE, removes all hills from the world. If it is FALSE, the hill information
// is returned in the two provided entity arrays. Returns the number
// of hills loaded or removed.
s32 CHillGame::ParseHills( BOOL bRemoveAll, CEntity** apVisHills, CEntity** apTestHills )
{
	// Get ptrs to all of the hills
	s32 nHillCount = 0;
	s32 nHill;
	for (nHill = 0; nHill < MAX_HILL_COUNT; nHill++) {
		char	szName[16];

		// First, find the visible entity.
		sprintf(szName, "HillVis%02.2d", nHill + 1);
		CEntity* pVisEntity = CEntity::Find( szName );

		// And the test volume entity
		sprintf(szName, "Hill%02.2d", nHill+1);
		CEntity* pTestEntity = CEntity::Find( szName );

		// If either entity doesn't exist or this is not a king game, pretend it wasn't there
		if ( bRemoveAll || (pVisEntity == NULL) || (pTestEntity == NULL) ) {
			if (pVisEntity)
				pVisEntity->RemoveFromWorld( TRUE );
			if (pTestEntity)
				pTestEntity->RemoveFromWorld( TRUE );
		}
		else {
			// We need to save these for later
			apVisHills[nHillCount] = pVisEntity;
			apTestHills[nHillCount] = pTestEntity;
			nHillCount++;
		}
	}

	return nHillCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Reverse Tag
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CReverseTagGame::Create( const GameInitInfo_t* pGameInit )
{
	CTimedGame::Create( pGameInit );

	m_nItPlayer = -1;

	// Set up victory time here?
	if (pGameInit && pGameInit->pMultiplayerRules) {
		m_fVictoryTime = 60.0f * pGameInit->pMultiplayerRules->nTimeLimit;
	}

	// Since no one is it, start off with a message to that effect
	_snwprintf(m_wszBonusMessage, BONUS_MSG_LEN-1, Game_apwszPhrases[GAMEPHRASE_NO_IT_PLAYER]);
	m_nBonusPlayer = -1;
	m_fBonusMessageTime = 2.0f * _BONUS_MSG_TIME;


	return TRUE;
}

//-----------------------------------------------------------------------------
void CReverseTagGame::Work( void )
{
	CTimedGame::Work();

	if (m_nItPlayer >= 0) {
		// Make sure the bot didn't die during the last frame (if another player
		// didn't kill him, our NotifyKill function won't pick it up).
		FASSERT(Player_aPlayer[m_nItPlayer].m_pEntityCurrent && (Player_aPlayer[m_nItPlayer].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
		if (((CBot*)(Player_aPlayer[m_nItPlayer].m_pEntityCurrent))->IsDeadOrDying()) {
			m_nItPlayer = -1;
			CFSoundGroup::PlaySound( _pDroppedBallSound );
			_snwprintf(m_wszBonusMessage, BONUS_MSG_LEN-1, Game_apwszPhrases[GAMEPHRASE_NO_IT_PLAYER]);
			m_nBonusPlayer = -1;
			m_fBonusMessageTime = _BONUS_MSG_TIME;
		}
		else {
			m_afPlayerTimes[m_nItPlayer] += FLoop_fPreviousLoopSecs;

			if (MultiplayerMgr.IsTeamPlay())
				m_afTeamTimes[Player_aPlayer[m_nItPlayer].m_nTeamNum] += FLoop_fPreviousLoopSecs;
		}
	}
}

//-----------------------------------------------------------------------------
void CReverseTagGame::Draw( void )
{
	CTimedGame::Draw();

	for (s32 nPlayer = CPlayer::m_nPlayerCount - 1; nPlayer >= 0; nPlayer--) {
		if ( MultiplayerMgr.MenuDone( nPlayer ) ) {
			fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportSafeOrtho3D);

			// Draw the player's time
			DrawTime( nPlayer );

			// Print a message on the IT player's screen letting him know he's it
			if ( nPlayer == m_nItPlayer ) {
				m_GameMsgs[m_nItPlayer][MSG_LINE_PLAYER_STATUS].PrintString( Game_apwszPhrases[GAMEPHRASE_YOU_ARE_IT] );
			}
			else if ( m_nItPlayer >= 0 ) {
				// Draw the indicator in the color of the IT player.
				_DrawTargetIndicator(nPlayer, Player_aPlayer[m_nItPlayer].m_pEntityCurrent,
										INDICATOR_IT_PLAYER, &_GlitchColors[Player_aPlayer[m_nItPlayer].m_nMultiplayerColor]);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Notify the current game of a kill
void CReverseTagGame::NotifyKill( s32 nKiller, s32 nKillee )
{
	CMultiplayerGame::NotifyKill(nKiller, nKillee);

	if (((m_nItPlayer < 0) || (nKillee == m_nItPlayer)) &&
		(nKillee >= 0) && (nKiller >= 0) && (nKiller != nKillee))
	{
		// Reset the run speed of the guy being killed
		if (m_nItPlayer >= 0) {
			FASSERT(Player_aPlayer[m_nItPlayer].m_pEntityCurrent && (Player_aPlayer[m_nItPlayer].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
			((CBot*)(Player_aPlayer[m_nItPlayer].m_pEntityCurrent))->SetRunSpeed( GetSpeedMultiplier(nKillee));
		}

		// For team play: should you get to be IT if you kill your teammate? 
		// Or should you "drop the ball," leaving no one it?
		if (MultiplayerMgr.IsTeamPlay() && (Player_aPlayer[nKiller].m_nTeamNum == Player_aPlayer[nKillee].m_nTeamNum)) {
			m_nItPlayer = -1;
			CFSoundGroup::PlaySound( _pDroppedBallSound );
			_snwprintf(m_wszBonusMessage, BONUS_MSG_LEN-1, Game_apwszPhrases[GAMEPHRASE_NO_IT_PLAYER]);
			m_nBonusPlayer = -1;
			m_fBonusMessageTime = _BONUS_MSG_TIME;
		}
		else {
			m_nItPlayer = nKiller;

			// Get the bot of the new IT player so we can slow him down.
			FASSERT(Player_aPlayer[m_nItPlayer].m_pEntityCurrent && (Player_aPlayer[m_nItPlayer].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT));
			CBot* pItBot = (CBot*)(Player_aPlayer[nKiller].m_pEntityCurrent);

			// Slow him down
			pItBot->SetRunSpeed( GetSpeedMultiplier( nKiller ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Indicate whether the player should be highlighted in the score screens
BOOL CReverseTagGame::IsPlayerBlessed( s32 nPlayer ) const
{
	return (nPlayer == m_nItPlayer);
}

//-----------------------------------------------------------------------------
f32 CReverseTagGame::GetSpeedMultiplier( s32 nPlayerIndex )
{
	if ( (nPlayerIndex >= 0) && (nPlayerIndex == m_nItPlayer) )
		return _MP_PLAYER_SPEED_SLOW;
	else
		return CTimedGame::GetSpeedMultiplier( nPlayerIndex );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Regular Tag
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
BOOL CTagGame::Create( const GameInitInfo_t* pGameInit )
{
	CTimedGame::Create( pGameInit );

	m_nItPlayer = -1;

	// Set up victory time
	if (pGameInit && pGameInit->pMultiplayerRules) {
		m_fVictoryTime = 60.0f * pGameInit->pMultiplayerRules->nTimeLimit;
	}

	// Set up each player to start at the time limit
	s32 nPlayer;
	for (nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; nPlayer++) {
		m_afPlayerTimes[nPlayer] = m_fVictoryTime;
	}

	if (MultiplayerMgr.IsTeamPlay()) {
		m_afTeamTimes[0] = m_afTeamTimes[1] = m_fVictoryTime;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
void CTagGame::Work( void )
{
	CTimedGame::Work();

	// Loop through all the players. If any of them are dead, we take away
	// double time. The IT player gets normal time decrement.
	s32 nPlayer;
	for (nPlayer = CPlayer::m_nPlayerCount; nPlayer--;) {
		if ( MultiplayerMgr.HasQuit(nPlayer) )
			continue;

		if (nPlayer == m_nItPlayer) {
			m_afPlayerTimes[m_nItPlayer] -= FLoop_fPreviousLoopSecs;

			if (MultiplayerMgr.IsTeamPlay())
				m_afTeamTimes[Player_aPlayer[m_nItPlayer].m_nTeamNum] -= FLoop_fPreviousLoopSecs;
		}
		else if ( ((CBot*)(Player_aPlayer[nPlayer].m_pEntityOrig))->IsBuried() ) {
			m_afPlayerTimes[nPlayer] -= 2.0f * FLoop_fPreviousLoopSecs;

			if (MultiplayerMgr.IsTeamPlay())
				m_afTeamTimes[Player_aPlayer[nPlayer].m_nTeamNum] -= 2.0f * FLoop_fPreviousLoopSecs;
		}
	}
}

//-----------------------------------------------------------------------------
void CTagGame::Draw( void )
{
	CTimedGame::Draw();

	// Print out each player's time, and the IT player's message
	s32 nPlayer;
	for (nPlayer = CPlayer::m_nPlayerCount; nPlayer--; ) {
		if (MultiplayerMgr.MenuDone(nPlayer)) {
			fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportSafeOrtho3D);
			DrawTime(nPlayer);
			if (nPlayer == m_nItPlayer) {
				m_GameMsgs[nPlayer][MSG_LINE_PLAYER_STATUS].PrintString( Game_apwszPhrases[GAMEPHRASE_YOU_ARE_IT] );

				// The IT player gets an indicator showing where the player with the most
				// remaining time is.
				RankInfo_t* aRankings = MultiplayerMgr.GetRankings();
				s32 i, nTargetPlayer;
				for (i = 0; i < CPlayer::m_nPlayerCount; i++) {
					nTargetPlayer = aRankings[i].m_nPlayerIndex;
					if (nTargetPlayer == nPlayer)
						continue;
					if ( ((CBot*)Player_aPlayer[nTargetPlayer].m_pEntityCurrent)->IsDeadOrDying() )
						continue;

					if (!MultiplayerMgr.IsTeamPlay() || (Player_aPlayer[nTargetPlayer].m_nTeamNum != Player_aPlayer[nPlayer].m_nTeamNum))
						break;
				}
				if (i < CPlayer::m_nPlayerCount) {
					_DrawTargetIndicator(nPlayer, Player_aPlayer[nTargetPlayer].m_pEntityCurrent,
						INDICATOR_IT_PLAYER, &_GlitchColors[Player_aPlayer[nTargetPlayer].m_nMultiplayerColor]);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Notify the current game of a kill
void CTagGame::NotifyKill( s32 nKiller, s32 nKillee )
{
	CMultiplayerGame::NotifyKill(nKiller, nKillee);

	// If the killer is not a player or someone committed suicide, just bail
	if ( (nKiller < 0) || (nKillee < 0) || (nKiller == nKillee) )
		return;

	// If the IT player did the killing or if no one is IT, then the player
	// that just died is now IT. Since being IT is bad, go ahead and let one
	// teammate do it to another.
	if ( (m_nItPlayer < 0) || (nKiller == m_nItPlayer) ) {
		// Make sure to change the IT player before calling GetSpeedMultiplier!
		m_nItPlayer = nKillee;

		// Speed up the new IT player, and slow down the guy he killed
		CPlayer* pDeadPlayer = &Player_aPlayer[nKillee];
		CPlayer* pKiller = &Player_aPlayer[nKiller];

		((CBot*)(pKiller->m_pEntityCurrent))->SetRunSpeed( GetSpeedMultiplier( nKiller ) );
		((CBot*)(pDeadPlayer->m_pEntityCurrent))->SetRunSpeed( GetSpeedMultiplier( nKillee ) );

		// If the player killed wasn't a teammate, credit the killer with
		// 25% of the remaining time of the killee
		BOOL bTeams = MultiplayerMgr.IsTeamPlay();
		if (!bTeams || (pKiller->m_nTeamNum != pDeadPlayer->m_nTeamNum)) {
			f32 fBump =  0.25f * m_afPlayerTimes[nKillee];

			// First limit: never credit more than a minute
			if ( fBump > 60.0f )
				fBump = 60.0f;

			// Second limit: don't go over our initial time
			f32 fNewTime = m_afPlayerTimes[nKiller] + fBump;
			if (fNewTime > m_fVictoryTime) {
				fBump = m_fVictoryTime - m_afPlayerTimes[nKiller];
			}

			// If we are playing with teams, limit the bump even further for the team time
			if (bTeams) {
				fNewTime = m_afTeamTimes[ pKiller->m_nTeamNum ] + fBump;
				if (fNewTime > m_fVictoryTime)
					fBump = m_fVictoryTime - m_afTeamTimes[ pKiller->m_nTeamNum ];
			}

			if (fBump > 0.0f) {
				// Give the player/team a bonus
				m_afPlayerTimes[nKiller] += fBump;
				if (bTeams)
					m_afTeamTimes[ pKiller->m_nTeamNum ] += fBump;

				// If the bump was more than one second, print a message and play a sound
				s32 nBump = (s32)fBump;
				if (nBump > 0) {
					// Cue audio to let the player know something happened
					CFSoundGroup::PlaySound(_pTagBonusSound);

					_snwprintf(m_wszBonusMessage, BONUS_MSG_LEN-1, Game_apwszPhrases[GAMEPHRASE_TIME_BONUS], (int)nBump);
					m_nBonusPlayer = nKiller;
					m_fBonusMessageTime = _BONUS_MSG_TIME;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Indicate whether the player should be highlighted in the score screens
BOOL CTagGame::IsPlayerBlessed( s32 nPlayer ) const
{
	return (nPlayer == m_nItPlayer);
}

//-----------------------------------------------------------------------------
BOOL CTagGame::TimeToQuit( void ) const
{
	if (MultiplayerMgr.IsTeamPlay()) {
		if ( (m_afTeamTimes[0] <= 0.0f) || (m_afTeamTimes[1] <= 0.0f) )
			return TRUE;
	}
	else {
		for (s32 i = CPlayer::m_nPlayerCount - 1; i >= 0; i--) {
			if (m_afPlayerTimes[i] <= 0.0f)
				return TRUE;
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
f32 CTagGame::GetSpeedMultiplier( s32 nPlayerIndex )
{
	if ( (nPlayerIndex >= 0) && (nPlayerIndex == m_nItPlayer) )
		return _MP_PLAYER_SPEED_TAG_FAST;
	else
		return _MP_PLAYER_SPEED_TAG_NORMAL;
}

//-----------------------------------------------------------------------------
// Break time up into hours, minutes, and seconds for printing. Uses native
// int format (int), not fang types.
static void _GetHMS(f32 fSeconds, int& nHours, int& nMinutes, int& nSeconds)
{
	int nTime = (int)fSeconds;
	nHours = nTime / 3600;
	nTime -= (nHours * 3600);
	nMinutes = nTime / 60;
	nSeconds = nTime - (nMinutes * 60);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Start Point Manager
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CStartPtMgr::CStartPtMgr()
{
	m_nStartPtCount = 0;
	m_nNextPoint = 0;
}

//-----------------------------------------------------------------------------
void CStartPtMgr::InitLevel( void )
{
	// Make sure at least the first point is initialized
	m_aStartPoints[0].Identity();
	m_anStartIndex[0] = 0;
	m_nStartPtCount = 0;
	m_nNextPoint = 0;

	// Loop through and find all start points for this level
	char szStart[16];
	CEntity *pStartEntity = NULL;

	int i;
	for (i = 0; i < MAX_SPAWN_POINTS; i++) {
		sprintf(szStart, "Start%02.2d", i+1);
		pStartEntity = CEntity::Find( szStart );

		if (pStartEntity) {
			if( !(pStartEntity->LeafTypeBit() & ENTITY_BIT_POINT) ) {
				DEVPRINTF( "CMultiplayerMgr::InitLevel(): '%s' must be a point object.\n", szStart );
				continue;
			}
			m_aStartPoints[m_nStartPtCount] = *pStartEntity->MtxToWorld();
			m_anStartIndex[m_nStartPtCount] = m_nStartPtCount;
			m_nStartPtCount++;
		}
	}

	// If we didn't find any start points, let the developer know
	if (m_nStartPtCount == 0) {
		DEVPRINTF("CMultiplayer::InitLevel(): No start points found!!!!\n");
		m_aStartPoints[0].Identity();
		m_anStartIndex[0] = 0;
	}

	// If we are multiplayer and there are less than four start points, hack
	// in a few extras so we don't start on top of each other.
	if (m_nStartPtCount < 4) {
		CFVec3A v;
		switch (m_nStartPtCount) {
			// Note: all falls through are intentional
			case 1:
				m_aStartPoints[1] = m_aStartPoints[0];
				v = m_aStartPoints[1].m_vRight;
				v.Mul(-10.0f);
				m_aStartPoints[1].m_vPos.Add(v);
				m_anStartIndex[1] = 1;
			case 2:
				m_aStartPoints[2] = m_aStartPoints[0];
				v = m_aStartPoints[2].m_vRight;
				v.Mul(10.0f);
				m_aStartPoints[2].m_vPos.Add(v);
				m_anStartIndex[2] = 2;
			case 3:
				m_aStartPoints[3] = m_aStartPoints[0];
				v = m_aStartPoints[3].m_vFront;
				v.Mul(10.0f);
				m_aStartPoints[3].m_vPos.Add(v);
				m_anStartIndex[3] = 3;
				break;
		}
		m_nStartPtCount = 4;
	}

	// Randomly shuffle the start points
	_ShuffleStartPoints();
}

//-----------------------------------------------------------------------------
// Get the next start point for spawning/respawning
const CFMtx43A* CStartPtMgr::NextPoint( BOOL bShuffleOK )
{
	if (m_nNextPoint >= m_nStartPtCount) {
		if ( bShuffleOK )
			_ShuffleStartPoints();

		m_nNextPoint = 0;
	}

	return &m_aStartPoints[ m_anStartIndex[ m_nNextPoint++ ] ];
}

//-----------------------------------------------------------------------------
// Respawn the given player at a new start point. Returns TRUE if the
// player is successfully respawned.
BOOL CStartPtMgr::RespawnBot( CBot* pBot )
{
	// Don't respawn if this is a single player game, let the player manager
	// restore from a checkpoint.
	if (MultiplayerMgr.IsSinglePlayer())
		return FALSE;

	// Restore this bot from the checkpoint. We can only restore a Glitch...
	if ( pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
		// Restore from checkpoint 0, the startup condition
		if ( !CFCheckPoint::SetCheckPoint( 0 ) )
			return FALSE;

		// set checkpoint system state to "load"
		if ( !CFCheckPoint::SetLoadState() )
			return FALSE;

		((CBotGlitch*)pBot)->CheckpointRestoreThisBot();
	}

	// Get the next start point
	const CFMtx43A* pFirstPt = NextPoint( TRUE );
	const CFMtx43A* pStartPt = pFirstPt;

	// Add a dummy tracker to the world
	CFWorldUser UserTracker;

	// Setup for our callback.
	ms_pSpawningBot = pBot;	// Input to bot check: skip this bot

	// Search for a start point that doesn't have another bot there already
	do {
		// Create a sphere to check for intersecting robots.
		CFSphere sphTest;
		sphTest.m_Pos = pStartPt->m_vPos.v3;
		sphTest.m_fRadius = 10.0f;

		// Put our tracker in the world where we want it
		UserTracker.MoveTracker(sphTest);

		// Do the search. If we are returned TRUE, that means that we
		// didn't find anyone nearby, so it is OK to spawn here
		if ( UserTracker.FindIntersectingTrackers( _NearbyBotCheck ) )
			break;

		// Next point...
		pStartPt = NextPoint( FALSE );
	} while ( pStartPt != pFirstPt );

	// Don't need this guy any more
	UserTracker.RemoveFromWorld();

	// Move to our new starting point
	pBot->Relocate_RotXlatFromUnitMtx_WS( pStartPt );

	return TRUE;
}

//-----------------------------------------------------------------------------
void CStartPtMgr::_ShuffleStartPoints( void )
{
	if ( MultiplayerMgr.IsMultiplayer() ) {
		for (u32 i = m_nStartPtCount - 1; i; i--) {
			u32 nChoice = fmath_RandomChoice( i + 1);
			u32 nTmp = m_anStartIndex[i];
			m_anStartIndex[i] = m_anStartIndex[nChoice];
			m_anStartIndex[nChoice] = nTmp;
		}
	}
}

//-----------------------------------------------------------------------------
BOOL CStartPtMgr::_NearbyBotCheck( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	// Check to see if it's a CEntity...
	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		return TRUE;
	}

	// Make sure it is a bot
	CEntity *pEntity = (CEntity *)pTracker->m_pUser;
	if ( !(pEntity->TypeBits() & ENTITY_BIT_BOT) )
		return TRUE;


	// Don't count my own body
	CBot* pBot = (CBot*)pEntity;
	if ( pBot == ms_pSpawningBot )
		return TRUE;

	// Don't count a dead bot
	if ( pBot->IsDeadOrDying() )
		return TRUE;

	// Found someone; we can stop checking now
	return FALSE;
}

//-----------------------------------------------------------------------------
// Get the color for the given bot. If bRemapForText is TRUE, a black bot
// returns a lighter gray color.
const CFColorRGBA& Multiplayer_PlayerColor(const CBot* pBot, BOOL bRemapForText)
{
	// If this is single player or no bot was given, just return the default yellow.
	if ( MultiplayerMgr.IsSinglePlayer() || (pBot == NULL) )
		return _GlitchColors[GAMESAVE_MP_COLORS_YELLOW];

	u32 nColor;
	if ( pBot->m_nPossessionPlayerIndex < 0 ) {
		// If the bot is not owned by a player, use its team.
		u32 nTeam = pBot->GetTeam();
		if (MultiplayerMgr.IsTeamPlay()) {
			switch (nTeam) {
				case 0:
					nColor = GAMESAVE_MP_COLORS_BLUE;
					break;
				case 1:
					nColor = GAMESAVE_MP_COLORS_RED;
					break;
				default:
					nColor = GAMESAVE_MP_COLORS_YELLOW;
					break;
			}
		}
		else {
			// nTeam will give us a player index if he is allied with anyone
			if (nTeam < (u32)CPlayer::m_nPlayerCount)
				nColor = Player_aPlayer[ nTeam ].m_nMultiplayerColor;
			else
				nColor = GAMESAVE_MP_COLORS_YELLOW;
		}
	}
	else {
		nColor = Player_aPlayer[ pBot->m_nPossessionPlayerIndex ].m_nMultiplayerColor;
	}

	FASSERT( (nColor >= _MP_COLORS_FIRST) && (nColor <= _MP_COLORS_LAST) );

	// If they don't want to remap black, we need to bump up to the bot color
	// instead of the player color.
	if ( !bRemapForText && (nColor == GAMESAVE_MP_COLORS_BLACK) )
		nColor++;

	return _GlitchColors[nColor];
}

//-----------------------------------------------------------------------------
static void _CrawlEntities( const GameInitInfo_t *pGameInit )
{
	// Sanity check; shouldn't happen
	if ( pGameInit == NULL )
		return;
	if ( pGameInit->pMultiplayerRules == NULL )
		return;

	// Get our vehicle restriction information
	GameSave_AllowedVehicles_e eAllowed = (GameSave_AllowedVehicles_e)pGameInit->pMultiplayerRules->nLimitVehicles;

	// Set up our bit mask for testing
	u64 uVehicleBits = 0;
	switch (eAllowed) {
		case GAMESAVE_VEHICLE_LIMIT_LOADER_ONLY:
			uVehicleBits = ENTITY_BIT_VEHICLELOADER;
			break;
		case GAMESAVE_VEHICLE_LIMIT_SENTINEL_ONLY:
			uVehicleBits = ENTITY_BIT_VEHICLESENTINEL;
			break;
		case GAMESAVE_VEHICLE_LIMIT_RAT_ONLY:
			uVehicleBits = ENTITY_BIT_VEHICLERAT;
			break;
		case GAMESAVE_VEHICLE_LIMIT_ALL_VEHICLES:
		default:
			uVehicleBits = ENTITY_BIT_VEHICLELOADER | ENTITY_BIT_VEHICLESENTINEL | ENTITY_BIT_VEHICLERAT;
			break;
	}

	// Crawl through the entity list, finding vehicles and applying our restrictions
	for ( CEntity* pEntity = CEntity::InWorldList_GetHead(); pEntity; pEntity = pEntity->CEntity::InWorldList_GetNext() ) {
		u64 uBits = pEntity->TypeBits();

		if ( (uBits & ENTITY_BIT_VEHICLE ) && !(uBits & uVehicleBits) ) {
			// A vehicle which has been disallowed, just remove it
			pEntity->RemoveFromWorld();
		}
		else if ( uBits & ENTITY_BIT_BOT ) {
			// This is a bot. Let's set its team
			CBot* pBot = (CBot*)pEntity;
			if ( pBot->m_nPossessionPlayerIndex < 0 ) {
				pBot->InitialTeam(NEUTRAL_TEAM);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Utility to help draw the target indicators
FINLINE f32 _SwitchViewportsX(f32 x, FViewport_t* pVP1, FViewport_t* pVP2) {
	return x + pVP1->HalfRes.x + pVP1->ScreenCorners.UpperLeft.x - pVP2->ScreenCorners.UpperLeft.x - pVP2->HalfRes.x;
}
FINLINE f32 _SwitchViewportsY(f32 y, FViewport_t* pVP1, FViewport_t* pVP2) {
	return y - pVP1->HalfRes.y - pVP1->ScreenCorners.UpperLeft.y + pVP2->ScreenCorners.UpperLeft.y + pVP2->HalfRes.y;
}

//-----------------------------------------------------------------------------
// Draw a target indicator pointing to the given point in the given 
// player's viewport
static void _DrawTargetIndicator(s32 nPlayer, const CFVec3A& vPos, _IndicatorType_e eType, const CFColorRGBA* pColor)
{
	FASSERT(nPlayer >= 0);

	// Bail if markers are not enabled
	if ( !MultiplayerMgr.MarkersEnabled() )
		return;

	// If this player's menu is up, don't draw it
	if ( !MultiplayerMgr.MenuDone(nPlayer) )
		return;

	// Get our color
	if (pColor == NULL) {
		if (eType == INDICATOR_TEAMMATE)
			pColor = &_TeamColors[Player_aPlayer[nPlayer].m_nTeamNum];
		else
			pColor = &_IndicatorColors[eType];
	}

	// Transform the point to screen space
	FViewport_t* pVpPersp = Player_aPlayer[nPlayer].m_pViewportPersp3D;
	CFCamera* pCamera = fcamera_GetCameraByIndex(nPlayer);

	CFSphere sSphere_WS(vPos.v3, _IndicatorWorldHeight[eType]);
	CFSphere sSphere_SS;
	fviewport_ComputeScreenPointAndSize_WS( pVpPersp, &pCamera->GetFinalXfm()->m_MtxF, &sSphere_WS, &sSphere_SS );
	f32 fPz = sSphere_SS.m_Pos.z;
	f32 fPx = sSphere_SS.m_Pos.x * pVpPersp->HalfRes.x;
	f32 fPy = sSphere_SS.m_Pos.y * pVpPersp->HalfRes.y;

	f32 fLeft = -pVpPersp->HalfRes.x;
	f32 fRight = -fLeft;
	f32 fTop = pVpPersp->HalfRes.y;
	f32 fBottom = -fTop;

	// Use eDispType to change our uv coords for small icons
	_IndicatorType_e eDispType = eType;

	// The default values here are unimportant, as they are not really needed
	// if we fail the z test below; they are just here to keep the values from
	// being uninitialized when used for the fX1 etc. calcs.
	f32 fWidth = 0.0f;
	f32 fHeight = 0.0f;
	if (fPz > pVpPersp->fNearZ) {
		fWidth = sSphere_SS.m_fRadius * pVpPersp->Res.x;

		// If the width is less than the minimum allowed, we should switch to
		// a different icon (different uv coords). Otherwise it looks funny.
		if (fWidth < _IndicatorMinScreenSize[eType]) {
			fWidth = _IndicatorMinScreenSize[eType];
			eDispType = (_IndicatorType_e)(eType + INDICATOR_LOW_OFFSET);
		}

		fHeight = fWidth * _IndicatorAspectHOW[eDispType];
	}

	// Get the corners of our indicator
	f32 fX1 = fPx - fWidth;
	f32 fX2 = fPx + fWidth;
	f32 fY1 = fPy + fHeight + fHeight;
	f32 fY2 = fPy;

	// Prepare to draw
	frenderer_Push(FRENDERER_DRAW, NULL);
	CFXfm::InitStack();
	fdraw_Depth_EnableWriting(FALSE);
	fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_SRC_PLUS_DST);
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AI);
	fdraw_SetTexture(&_texIndicators);

	if ((fPz > pVpPersp->fNearZ) && (fX1 <= fRight) && (fX2 >= fLeft) && (fY1 >= fBottom) && (fY2 <= fTop)) {
		// Icon is inside our viewport, so draw it
		fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportOrtho3D);
		_DrawIndicator(fX1, fY1, 2.0f * fWidth, eDispType, pColor);
	}
	else if (eType != INDICATOR_TEAMMATE) {
		// Point is outside the viewport, so draw a pointer on the edge of our screen
		FViewport_t* pVP = Player_aPlayer[nPlayer].m_pViewportSafeOrtho3D;
		fPx = _SwitchViewportsX(fPx, pVpPersp, pVP);
		fPy = _SwitchViewportsY(fPy, pVpPersp, pVP);
		fviewport_SetActive(pVP);
		_DrawEdgePointer(fPx, fPy, pVP->HalfRes.x, pVP->HalfRes.y, pColor);
	}
	frenderer_Pop();
}

//-----------------------------------------------------------------------------
// Draw the textured quad for the given indicator
static void _DrawIndicator(f32 fLeft, f32 fTop, f32 fWidth, _IndicatorType_e eType, const CFColorRGBA* pColor)
{
	f32 fRight = fLeft + fWidth;
	f32 fBottom = fTop - (fWidth * _IndicatorAspectHOW[eType]);

	FDrawVtx_t aVerts[4];

	aVerts[0].Pos_MS.Set(  fLeft,    fTop, 1.0f );
	aVerts[1].Pos_MS.Set(  fLeft, fBottom, 1.0f );
	aVerts[2].Pos_MS.Set( fRight,    fTop, 1.0f );
	aVerts[3].Pos_MS.Set( fRight, fBottom, 1.0f );

	aVerts[0].ST.Set(_IndicatorST[eType][_UPLEFT].x,   _IndicatorST[eType][_UPLEFT].y);
	aVerts[1].ST.Set(_IndicatorST[eType][_UPLEFT].x,   _IndicatorST[eType][_LOWRIGHT].y);
	aVerts[2].ST.Set(_IndicatorST[eType][_LOWRIGHT].x, _IndicatorST[eType][_UPLEFT].y);
	aVerts[3].ST.Set(_IndicatorST[eType][_LOWRIGHT].x, _IndicatorST[eType][_LOWRIGHT].y);

	aVerts[0].ColorRGBA = *pColor;
	aVerts[1].ColorRGBA = *pColor;
	aVerts[2].ColorRGBA = *pColor;
	aVerts[3].ColorRGBA = *pColor;

	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, aVerts, 4);
}

//-----------------------------------------------------------------------------
// Draw a pointer on the edge of the screen pointing towards the off-screen point.
// The screen goes from -HalfWidth to HalfWidth (left to right) and HalfHeight
// to -HalfHeight (top to bottom).
static void _DrawEdgePointer(f32 fPx, f32 fPy, f32 fHalfWidth, f32 fHalfHeight, const CFColorRGBA* pColor)
{
	// Get a unit vector in the direction of the point
	f32 fOOLen = fmath_InvSqrt(fPx*fPx + fPy*fPy);
	f32 fDx = fPx * fOOLen;
	f32 fDy = fPy * fOOLen;

	// Project the point onto the edge of the screen
	f32 fTestX, fTestY;
	if (fPx > 0.0001f)
		fTestX = fHalfWidth / fDx;
	else if (fPx < -0.0001f)
		fTestX = -fHalfWidth / fDx;
	else
		fTestX = 1.0e35f;

	if (fPy > 0.0001f)
		fTestY = fHalfHeight / fDy;
	else if (fPy < -0.0001f)
		fTestY = -fHalfHeight / fDy;
	else
		fTestY = 1.0e35f;

	if (fTestX < fTestY) {
		fPy = fTestX * fDy;
		fPx = fTestX * fDx;
	}
	else {
		fPx = fTestY * fDx;
		fPy = fTestY * fDy;
	}

	// Set up the uv coords first
	FDrawVtx_t aVerts[3];
	aVerts[0].ST.Set( 481.0f/511.0f, 121.0f/511.0f );
	aVerts[1].ST.Set( 454.0f/511.0f,  71.0f/511.0f );
	aVerts[2].ST.Set( 508.0f/511.0f,  71.0f/511.0f );

	// Calculate our arrow vertices, with our edge point as the arrow tip
	#define __ARROW_LEN 40.0f
	f32 fArrowWidth = __ARROW_LEN * (aVerts[0].ST.x - aVerts[1].ST.x) / (aVerts[0].ST.y - aVerts[1].ST.y);

	// Use that as the point of an arrow
	f32 fNx = fArrowWidth * fDy;
	f32 fNy = -fArrowWidth * fDx;
	fDx *= __ARROW_LEN;
	fDy *= __ARROW_LEN;

	#undef __ARROW_LEN

	aVerts[0].Pos_MS.Set( fPx, fPy, 1.0f );
	aVerts[1].Pos_MS.Set( fPx - fDx + fNx, fPy - fDy + fNy, 1.0f );
	aVerts[2].Pos_MS.Set( fPx - fDx - fNx, fPy - fDy - fNy, 1.0f );

	aVerts[0].ColorRGBA = *pColor;
	aVerts[1].ColorRGBA = *pColor;
	aVerts[2].ColorRGBA = *pColor;

	fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, aVerts, 3);
}

// Play back a victory animation for all first place bots
static void _VictoryDance( void )
{
	int i;
	if ( MultiplayerMgr.IsTeamPlay() ) {
		TeamRank_t*	pRankings = MultiplayerMgr.GetTeamRankings();

		for ( i = 0; i < CPlayer::m_nPlayerCount; i++ ) {
			// What the heck. If the teams are tied, then let's all dance
			if ( (pRankings->m_nLeader < 0) || (pRankings->m_nLeader == Player_aPlayer[i].m_nTeamNum) ) {
				_StartDance( i, "***_VICT" );
			}
			else {
				_StartDance( i, "***_LOSE" );
			}
		}
	}
	else {
		for ( i = 0; i < CPlayer::m_nPlayerCount; i++ ) {
			if ( (_RankingIndex[i] == 0) || (_RankingIndex[i] == MAX_PLAYERS) ) {
				// This player won, try to kick off the victory dance
				_StartDance( i, "***_VICT" );
			}
			else {
				_StartDance( i, "***_LOSE" );
			}
		}
	}
}

static void _StartDance( s32 nPlayer, cchar* pszDance )
{
	CBot* pBot = (CBot*)Player_aPlayer[nPlayer].m_pEntityCurrent;
	FASSERT(pBot->TypeBits() & ENTITY_BIT_BOT);

	// This dance only works with Glitch as of now, but try to call it for
	// any bot type. That way it can be added later if desired. A dancing
	// Titan might be cool.
	cchar* pszBTAName = AIBTA_EventToBTAName( pszDance, pBot->AIBrain() );
	if ( pszBTAName ) {
		pBot->StartReverseFacingMode();
		pBot->StartBotTalkByName( pszBTAName );
	}
}






