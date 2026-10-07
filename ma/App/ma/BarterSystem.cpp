//////////////////////////////////////////////////////////////////////////////////////
// BarterSystem.cpp - 
//
// Author: Michael Starich   
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
// 10/15/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BarterSystem.h"
#include "fworld_coll.h"
#include "Slim.h"
#include "Shady.h"
#include "BarterSound.h"
#include "fres.h"
#include "fclib.h"
#include "frenderer.h"
#include "fresload.h"
#include "entity.h"
#include "player.h"
#include "bot.h"
#include "MultiplayerMgr.h"
#include "gamecam.h"
#include "gamepad.h"
#include "game.h"
#include "floop.h"
#include "AI\AiApi.h"
#include "AI\AIBrainMan.h"
#include "hud2.h"
#include "MeshEntity.h"
#include "fmath.h"
#include "fsndfx.h"
#include "BarterTypes.h"
#include "CamManual.h"
#include "meshtypes.h"
#include "botfx.h"
#include "launcher.h"
#include "fcheckpoint.h"
#include "pausescreen.h"
#include "ShadowedText.h"
#if defined(MA_PC_INPUT)
	#include "pc_input.h"
	#include "wpr_drawutils.h"
	#include "wpr_datatypes.h"
#endif
//====================
// private definitions

#define _ALLOW_BARTER_DROID_INTERACTION		TRUE
#define _DEBUG_BARTER_DROIDS				FALSE

static const u32 MAX_BARTER_PTS = 8;
static cchar BARTERITEMS_FILE_NAME[] = "BarterItems";

static f32 ARROW_HIGHLIGHT_TIME = 0.50f;
static f32 ARROW_ON = 0.0f;
static f32 ARROW_OFF = -1.0f;


enum BarterState_e {
	BARTERSTATE_NOT_IN_WORLD = 0,	// the barters are not in the world, but are not hidden (in this state, they are looking for somewhere to appear)
	BARTERSTATE_HIDDEN,				// the barters are not in the world and can't be put into the world till they are unhidden	

	BARTERSTATE_IDLE,				// the barters are in the world, but don't see the player 
	BARTERSTATE_ATTRACT,			// the barters are in the world and are trying to attract the player's attention

	BARTERSTATE_ACTIVE,				// wheeling and dealing with the player
		
	BARTERSTATE_ENDING,				// exiting barter mode

	BARTERSTATE_COUNT
};

enum WaitStates_e {
	WAIT_STATE_NONE = 0,
	WAIT_STATE_WELCOME_SHADY,		// waiting for shady to finish the welcome command
	WAIT_STATE_WELCOME_SLIM,		// waiting for slim to finish the welcome command
	WAIT_STATE_PURCHASE_SLIM,
	WAIT_STATE_PURCHASE_SHADY,
	WAIT_STATE_FAILED_SLIM,
	WAIT_STATE_FAILED_SHADY,
	WAIT_STATE_REJECTED_SLIM,
	WAIT_STATE_REJECTED_SHADY,
	WAIT_STATE_NEW_SALE_SLIM,
	WAIT_STATE_NEW_SALE_SHADY,
	WAIT_STATE_EXIT_SLIM,
	WAIT_STATE_EXIT_SHADY,

	WAIT_STATE_COUNT
};


static cchar *_CLICK_FX_NAME								= "HUD Click";
static cchar *_SELECT_FX_NAME								= "HUD Select";

static const f32 _DIST2_TO_SEE_PLAYER						= ( BARTER_SYSTEM_GIVEN_SEE_PLAYER_DIST * BARTER_SYSTEM_GIVEN_SEE_PLAYER_DIST );
static const f32 _FURTHEST_DIST2_TO_SEE_PLAYER				= ( BARTER_SYSTEM_SEE_PLAYER_MAX_DIST * BARTER_SYSTEM_SEE_PLAYER_MAX_DIST );

//=================
// public variables

cchar *BarterSystem_apszEventStrings[BARTER_SYSTEM_EVENTS_SLIM_COUNT] = {
	"none",						// BARTER_SYSTEM_EVENTS_NONE
	"slim_look",				// BARTER_SYSTEM_EVENTS_SLIM_LOOK
	"slim_look_and_laugh",		// BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_LAUGH
	"slim_look_and_nod",		// BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_NOD
	"slim_look_and_shake",		// BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_SHAKE
	"slim_shady_is_welcoming",	// BARTER_SYSTEM_EVENTS_SLIM_SHADY_IS_WELCOMING
	"shady_look_at_slim",		// BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM
	"slim_wave",				// BARTER_SYSTEM_EVENTS_SLIM_WAVE
};

//==================
// private variables

#if FANG_WINGC
// The vendors serve one shopper at a time; input, inventory and camera belong to that player.
static s32 _nBarterPlayer = 0;
static BOOL _abCoopEmptyOffer[MAX_PLAYERS];
static s16 _anCoopEmptyOfferPoint[MAX_PLAYERS];
static u32 _anCoopEmptyOfferTicks[MAX_PLAYERS];
class CBarterPlayerScope {
	s32 m_nPrevious;
public:
	CBarterPlayerScope( s32 nPlayer ) : m_nPrevious( CPlayer::m_nCurrent ) { CPlayer::SetCurrent( nPlayer ); }
	~CBarterPlayerScope() { CPlayer::SetCurrent( m_nPrevious ); }
};
#endif
static BOOL8 _bLevelOK = FALSE;
static BOOL8 _bFirstBarterEncounter = TRUE;
static BOOL8 _bJustPurchasedItem;
static s8 _nCurTableSlotIndex;
static CSlim *_pSlim = NULL;
static CShady *_pShady = NULL;
static CBarterSound *_pBarterSound = NULL;
static CMeshEntity *_pBoxMeshEntity = NULL;
static CEntity *_apBarterPts[MAX_BARTER_PTS];
static s16 _nBarterPtSelected = -1;
static u16 _nBarterPtCnt;
static FSndFx_FxHandle_t _hScrollSound;
static FSndFx_FxHandle_t _hSelectSound;
static BarterState_e _eState;
static WaitStates_e _nWaitState = WAIT_STATE_NONE;
static f32 _fTimer;
static CCamManualInfo _CamInfo;
static CFMtx43A _CamMtx;
static BarterTypes_TableData_t _TableData;
static u16 _nUpsaleIndex;
static u16 _nItemsPurchased;
static CBarterLevel _BarterLevel;
static CFVec3A _vecCensorBarDLU;		//diag left up, initialized in initsystem()
static CFVec3A _vecCensorBarDRU;		//diag right up
static s32 _hSaveData[FCHECKPOINT_MAX_CHECKPOINTS];				// handle to "save point" data storage
static BOOL8 _bSpotlightWasOn = FALSE;


//===================
// private prototypes

static BOOL _LoadBarterInfoFromFile( cchar *pszCSVFile );
static s32 _GetNearestUnseenPt();
static BOOL _CanPlayerSeePt( s32 nBarterPtIdx, f32 &rfDist2 );
static BOOL _IsPlayerInFrontAndLookingAtBarters( s32 nBarterPtIdx, CBot *pPlayer );
static void _InsertBartersIntoWorld( u32 nNewPosIdx );
static void _MoveCamToBarterPos();
static void _MoveCamToNormalPos();
static BOOL _ActionBarter(CEntity *pActioningEntity, CEntity *pActionedEntity);
static void _ScrollSelector( s32 nScrollDir );
static void _ScrollSalesChain( s32 nScrollDir );
static void _Call_Work( BOOL bCanSeePlayer, f32 fDistSqToPlayer );
static void _Call_AddToWorld();
static void _Call_RemoveFromWorld();
static void _Call_AppendTrackerSkipList();
static BOOL _Call_ReadyToBarter( void );
static BOOL _Call_Welcome( void );
static void _Call_SetCurrentSlot( u32 nSlotIndex, CBarterItemInst *pNewBII=NULL, BOOL bFlipDown=TRUE );
static BOOL _Call_Purchase( void );
static BOOL _Call_NewSale( void );
static BOOL _Call_FailedPurchase( void );
static BOOL _Call_RejectedItem( void );
static BOOL _Call_Exit( void );
static BOOL _Call_Available( void );
static void _HandleCancelButton( CBot *pPlayer );
static void _HandlePurchaseButton( CBot *pPlayer );
static BOOL _WaitStateWork( void );
static BOOL _InitBarterLevel( cchar *pszCSVFile );
static CFMtx43A* GetBarterPtMtx(s32 nBarterPtSelected)
{
	return _apBarterPts[nBarterPtSelected]->MtxToWorld();
}
static void _InitUserInterface(BOOL bCreateText = TRUE);
static void _DrawUserInterface(void);
static void _RefreshArrows(BOOL bUpSale=FALSE);
//=================
// public functions

BOOL bartersystem_InitSystem() {

	_bLevelOK = FALSE;
	_bFirstBarterEncounter = TRUE;	
	_bJustPurchasedItem = FALSE;
	_pSlim = NULL;
	_pShady = NULL;
	_pBarterSound = NULL;
	_pBoxMeshEntity = NULL;
	_nBarterPtSelected = -1;
	_nBarterPtCnt = 0;
	return TRUE;
}

void bartersystem_UninitSystem() {

	bartersystem_UninitLevel();	
}

BOOL bartersystem_InitLevel( cchar *pszCSVFile ) {
#if FANG_WINGC
	_nBarterPlayer = 0;
	for( s32 n=0; n<MAX_PLAYERS; ++n ) _abCoopEmptyOffer[n] = FALSE;
#endif
	
	FASSERT( !_bLevelOK );
	
	//////////////////////////////////////////////////////
	// see if the player has seen the barter droids before
	_bFirstBarterEncounter = FALSE;

	if( CPlayer::m_pCurrent->m_pPlayerProfile ) {
		if( launcher_LaunchedFromWrappers() ) {
			_bFirstBarterEncounter = !CPlayer::m_pCurrent->m_pPlayerProfile->HasVisitedBarterDroidsBefore();
		}
	}
	
#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() && launcher_LaunchedFromWrappers() ) {
		_bFirstBarterEncounter = TRUE;
		for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
			if( Player_aPlayer[n].m_pPlayerProfile && Player_aPlayer[n].m_pPlayerProfile->HasVisitedBarterDroidsBefore() ) {
				_bFirstBarterEncounter = FALSE;
				break;
			}
		}
	}
#endif
	_InitUserInterface();

	//////////////////////////
	// Find the barter points.
	_nBarterPtSelected = -1;
	_nBarterPtCnt = 0;

	u32 i;
	char szPtName[9] = "barterx";
	CEntity *pBarterPt;

	szPtName[6] = '1';
	szPtName[7] = 0;
	szPtName[8] = 0;
	pBarterPt = CEntity::Find( szPtName );
	while( pBarterPt != NULL ) {
		// record the entity ptr
		_apBarterPts[ _nBarterPtCnt++ ] = pBarterPt;

		if( _nBarterPtCnt >= MAX_BARTER_PTS ) {
			DEVPRINTF( "bartersystem_InitLevel() : Too many barter points placed, only the first %d will be used.  Tell Mike to increase.\n", MAX_BARTER_PTS );
			break;
		}

		// setup the next string		
		if( _nBarterPtCnt < 10 ) {
			szPtName[6] = (char)(_nBarterPtCnt) + '1';
		} else if( _nBarterPtCnt < 20 ) {
			szPtName[6] = '1';
			szPtName[7] = (char)(_nBarterPtCnt - 10) + '0';
		} else {
			// increase the number of supported pts
			FASSERT( MAX_BARTER_PTS < 20 );
		}

		// find the next string in the chain
		pBarterPt = CEntity::Find( szPtName );		
	} 

	if( _nBarterPtCnt == 0 ) {
		DEVPRINTF("bartersystem_InitLevel() : No barter points found for this level.\n");
		return FALSE;
	}

	// init the barter level
	if( !_InitBarterLevel( pszCSVFile ) ) {
		// if the system isn't initted, then we can't init a level
		return FALSE;
	}	

	for( i=0; i<FCHECKPOINT_MAX_CHECKPOINTS; i++ ) {
		// Handle to save data for entity, indexed by checkpoint number...
		_hSaveData[i] = CFCheckPoint::NULL_HANDLE;
	}

	// if we got here, the level is ok
	_bLevelOK = TRUE;

	// don't init a slot since we aren't in the world yet
	_nCurTableSlotIndex = -1;
	_BarterLevel.SetActiveTable( NULL );
	
	return TRUE;
}

void bartersystem_UninitLevel() {
	
	if( _bLevelOK ) {
		CBarterLevel::UninitLevel();
		CBarterItem::UninitLevel();
		CBarterItem::Shutdown();

		if( _pBoxMeshEntity ) {
			if( _pBoxMeshEntity->IsCreated() ) {
				_pBoxMeshEntity->Destroy();
			}
			fdelete( _pBoxMeshEntity );
			_pBoxMeshEntity = NULL;
		}
		if( _pBarterSound ) {
			_pBarterSound->Stop();
			fdelete( _pBarterSound );
			_pBarterSound = NULL;
		}
		if( _pShady != NULL ) {
			_pShady->Destroy();
			fdelete( _pShady );
			_pShady = NULL;
		}	
		if( _pSlim != NULL ) {
			_pSlim->Destroy();
			fdelete( _pSlim );
			_pSlim = NULL;
		}

		_bLevelOK = FALSE;		
	}
}

static void _WorkBarterLevel() {
	FViewport_t *pPrevious = fviewport_GetActive();
	fviewport_SetActive( CPlayer::m_pCurrent->m_pViewportSafeOrtho3D );
	_BarterLevel.Work();
	fviewport_SetActive( pPrevious );
}

#if FANG_WINGC
static s32 _CoopBarterListener() {
	if( _nBarterPtSelected < 0 || (u32)_nBarterPtSelected >= _nBarterPtCnt ) return -1;
	f32 fNearest = FMATH_MAX_FLOAT;
	s32 nListener = -1;
	for( s32 n=0; n<CPlayer::m_nPlayerCount; ++n ) {
		CBot *pBot = (CBot *)Player_aPlayer[n].m_pEntityCurrent;
		if( !pBot || !pBot->IsInWorld() || pBot->IsDeadOrDying() ) continue;
		CFVec3A vDist;
		vDist.Sub( pBot->MtxToWorld()->m_vPos, _apBarterPts[_nBarterPtSelected]->MtxToWorld()->m_vPos );
		if( vDist.MagSq() < fNearest ) { fNearest = vDist.MagSq(); nListener = n; }
	}
	return nListener;
}
#endif

void bartersystem_Work() {
#if FANG_WINGC
	if( _bLevelOK && _eState != BARTERSTATE_ACTIVE && _eState != BARTERSTATE_ENDING ) {
		if( MultiplayerMgr.IsLocalCoop() ) {
			const s32 nListener = _CoopBarterListener();
			if( nListener >= 0 ) _nBarterPlayer = nListener;
			else { _pBarterSound->Stop(); return; }
		} else {
			f32 fNearest = FMATH_MAX_FLOAT;
			for( s32 nPlayer=0; nPlayer < CPlayer::m_nPlayerCount; ++nPlayer ) {
				CBot *pBot = (CBot *)Player_aPlayer[nPlayer].m_pEntityCurrent;
				if( !pBot || !pBot->IsInWorld() || pBot->IsDeadOrDying() ) continue;
				for( u32 nPt=0; nPt < _nBarterPtCnt; ++nPt ) {
					CFVec3A vDist;
					vDist.Sub( pBot->MtxToWorld()->m_vPos, _apBarterPts[nPt]->MtxToWorld()->m_vPos );
					if( vDist.MagSq() < fNearest ) { fNearest = vDist.MagSq(); _nBarterPlayer = nPlayer; }
				}
			}
		}
	}
	CBarterPlayerScope playerScope( _nBarterPlayer );
#endif
//	s32 nIndex;
	f32 fDist2;
	BOOL bCanSeePlayer;
	CBot *pPlayer = (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent;
	u32 nControlIndex = CPlayer::m_pCurrent->m_nControllerIndex;

	if( !_bLevelOK ) {
		return;
	}

	_pBarterSound->Work();

	BOOL bPauseScreenActive = CPauseScreen::IsActive();
	if (bPauseScreenActive)
	{
		CBarterLevel::m_oTextArea.bVisible = FALSE;
		ftext_SetAttributes(CBarterLevel::m_hTextDisplay,&CBarterLevel::m_oTextArea);
		return;
	}

	if( _nWaitState != WAIT_STATE_NONE ) {
		if( !_WaitStateWork() ) {
			// don't do the regular state machine yet, but do call the work functions
			bCanSeePlayer = _CanPlayerSeePt( _nBarterPtSelected, fDist2 );
			_Call_Work( bCanSeePlayer, fDist2 );
			_WorkBarterLevel();
			return;
		}
	}

	// master state machine
	switch( _eState ) {
	case BARTERSTATE_NOT_IN_WORLD:
		break;

	case BARTERSTATE_HIDDEN:
		// the barters are not in the world and can't be put into the world till they are unhidden	

		// nothing to do until the barters are unhidden
		break;

	case BARTERSTATE_IDLE:
		// the barters are in the world, but don't see the player 
	case BARTERSTATE_ATTRACT:
		// the barters are in the world and are trying to attract the player's attention	
		bCanSeePlayer = _CanPlayerSeePt( _nBarterPtSelected, fDist2 );

		_eState = bCanSeePlayer ? BARTERSTATE_ATTRACT : BARTERSTATE_IDLE;

		// Let slim and shady do there work...
		_Call_Work( bCanSeePlayer, fDist2 );
		FASSERT(CBarterLevel::GetTable(_nBarterPtSelected));
		_pBarterSound->UpdateAttractMusic(
			&GetBarterPtMtx(_nBarterPtSelected)->m_vPos,
			CBarterLevel::GetTable(_nBarterPtSelected)->m_fMusicRadius,
			CPlayer::m_pCurrent->m_pEntityCurrent->MtxToWorld()
		);

		break;

	case BARTERSTATE_ACTIVE:
		// wheeling and dealing with the player
		if( _Call_ReadyToBarter() ) 
		{
			if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) 
			{
				// They have pressed the cancel button.
				_HandleCancelButton( pPlayer );	
				CBarterLevel::m_oTextArea.bVisible = FALSE;
				ftext_SetAttributes(CBarterLevel::m_hTextDisplay,&CBarterLevel::m_oTextArea);
			} 
			else if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_JUMP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) 
			{
				// try to buy the current item
				_HandlePurchaseButton( pPlayer );	
		
			} 
			else if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) 
			{
				// They are trying to change the current selection with the analog stick
				if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState < -0.1f ) 
				{
					// They scrolled left.
					_ScrollSelector( 1 );
				}
				else 
				{
					// They scrolled right.
					_ScrollSelector( -1 );
				}
			} 
			else if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) 
			{
				// They are trying to change the current selection with the analog stick
				if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_QUICK_SELECT_LEFT_RIGHT]->fCurrentState < -0.1f ) 
				{
					// They scrolled left.
					_ScrollSelector( 1 );
				}
				else 
				{
					// They scrolled right.
					_ScrollSelector( -1 );
				}
			}
			else if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_FORWARD_BACKWARD]->uLatches & FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY ) 
			{
				// They are trying to change the current selection with the analog stick
				if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState > 0.1f ) 
				{
					// They scrolled up.
					_ScrollSalesChain( 1 );
				} 
				else
				{
					// They scrolled down.
					_ScrollSalesChain( -1 );
				}
			} 
			else if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) 
			{
				// They are trying to change the current selection with the analog stick
				if( Gamepad_aapSample[nControlIndex][GAMEPAD_MAIN_QUICK_SELECT_UP_DOWN]->fCurrentState > 0.1f ) 
				{
					// They scrolled up.
					_ScrollSalesChain( 1 );
				} 
				else
				{
					// They scrolled down.
					_ScrollSalesChain( -1 );
				}
			}
		}

		bCanSeePlayer = _CanPlayerSeePt( _nBarterPtSelected, fDist2 );
		_Call_Work( bCanSeePlayer, fDist2 );
		_WorkBarterLevel();
		break;

	case BARTERSTATE_ENDING:
		if( _Call_Available() ) 
		{
			_eState = BARTERSTATE_ATTRACT;
			bartersystem_AbortActiveMode();
		}
		bCanSeePlayer = _CanPlayerSeePt( _nBarterPtSelected, fDist2 );
		_Call_Work( bCanSeePlayer, fDist2 );
		_WorkBarterLevel();
		break;

	default:
		FASSERT_NOW;
		break;
	}
}


// Returns TRUE if the barters are in the world and are actually bartering (not attract mode)
BOOL bartersystem_IsActive() {
	if( !_bLevelOK ) {
		return FALSE;
	}

	if( _eState == BARTERSTATE_NOT_IN_WORLD ||
		_eState == BARTERSTATE_HIDDEN || 
		_eState == BARTERSTATE_IDLE ||
		_eState == BARTERSTATE_ATTRACT ) {
		return FALSE;
	}
	return TRUE;
}

// Return TRUE if the barters are currently in the world somewhere, either in active or attract modes
BOOL bartersystem_AreInWorld() {
	if( !_bLevelOK ) {
		return FALSE;
	}

	if( _eState == BARTERSTATE_NOT_IN_WORLD ||
		_eState == BARTERSTATE_HIDDEN ) {
		return FALSE;
	}
	return TRUE;
}

// Forces the barters (if they are not currently Active) to appear at the point named pszPointName.
// Returns TRUE if the move was successful, FALSE if we couldn't find the point or the barters are currently Active.
BOOL bartersystem_PlaceInWorldAndStartAttracting( cchar *pszPointName ) {
	if( !_bLevelOK || !pszPointName || bartersystem_IsActive() || _eState == BARTERSTATE_HIDDEN ) {
		return FALSE;
	}

	// find a pt named pszPointName in the world
	u32 i;
	for( i=0; i < _nBarterPtCnt; i++ ) {
		// see if the i'th point is a match
		if( fclib_stricmp( _apBarterPts[i]->Name(), pszPointName ) == 0 ) {
			break;
		}
	}
	if( i == _nBarterPtCnt ) {
		// didn't find a match
		DEVPRINTF("Unable to locate '%s' barter point\n", pszPointName );
		return FALSE;
	}

	// see if we can find a table for this point
	CBarterTable *pTable = _BarterLevel.GetTable( pszPointName );
	if( !pTable ) {
		DEVPRINTF("Table named '%s' not found\n", pszPointName );
		return FALSE;
	}

	bartersystem_RemoveFromWorld( FALSE );
	bartersystem_Hide( FALSE );

	// pick the point and put the bots into the world
	_InsertBartersIntoWorld( i );

	_eState = BARTERSTATE_ATTRACT;

	return TRUE;
}

void bartersystem_AbortActiveMode(void)
{
#if FANG_WINGC
	CBarterPlayerScope playerScope( _nBarterPlayer );
#endif
	CBarterLevel::m_oTextArea.bVisible = FALSE;
	ftext_SetAttributes(CBarterLevel::m_hTextDisplay,&CBarterLevel::m_oTextArea);

	game_LeaveBarterMode();
	// switch the camera back to glitch's camera
	_MoveCamToNormalPos();

	CBot *pPlayer = (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent;
	// enable glitch's draw and movement
	pPlayer->DrawEnable( TRUE );
	pPlayer->MobilizeBot();

	// turn back on the reticle

	// CPlayer::m_pCurrent->m_Reticle.EnableDraw( TRUE );
	pPlayer->ReticleEnable(TRUE);

	if( _bSpotlightWasOn )
	{
		pPlayer->SetSpotLightOn();
	}

	// hide the washers
	CHud2::GetCurrentHud()->SetWasher( FALSE );
	
	//CHud2::GetCurrentHud()->DisableDrawFlags( CHud2::DRAW_WASHERS | CHud2::DRAW_ITEMS );

	// enable the weapon select and the rest of the hud
	CHud2::GetCurrentHud()->SetWSEnable( TRUE );
	//ME:  can assume that only glitch would be bartering (otherwise would have to save previous state)
	CHud2::GetCurrentHud()->SetDrawFlags( CHud2::DRAW_GLITCH_HUD );

	// end the "cut scene"
	ai_NotifyCutSceneEnd();

	// there is no longer an active table
	_BarterLevel.SetActiveTable( NULL );
}

void bartersystem_RemoveFromWorld( BOOL bFadeMusic ) {
	if( _eState != BARTERSTATE_NOT_IN_WORLD ) {
		if ( (_eState==BARTERSTATE_ACTIVE) || (_eState==BARTERSTATE_ENDING))
		{
			bartersystem_AbortActiveMode();
		}
		_pSlim->ResetToPostCreateState();
		_pShady->ResetToPostCreateState();
		_pBoxMeshEntity->RemoveFromWorld();

		_pBarterSound->Stop( bFadeMusic );

		_nCurTableSlotIndex = -1;
		_nBarterPtSelected = -1;
		_fTimer = 10.0f;
		_nWaitState = WAIT_STATE_NONE;

		_BarterLevel.SetActiveTable( NULL );

		_Call_RemoveFromWorld();

		_eState = BARTERSTATE_NOT_IN_WORLD;
	}
}

// Call to force the barters (regardless of what they are currently doing)
// to hide, they will not come back until they are told to unhide
void bartersystem_Hide( BOOL bHide ) {
	
	if( !_bLevelOK ) {
		return;
	}

	if( bHide ) {
		// we should pull the barters out of the world and put them in the hidden state

		bartersystem_RemoveFromWorld( TRUE );

		_eState = BARTERSTATE_HIDDEN;
	} else {
		// we should unhide the barters

		_eState = BARTERSTATE_NOT_IN_WORLD;
		_nWaitState = WAIT_STATE_NONE;
		_fTimer = 0.01f;
	}
}

void bartersystem_EventNotify( u32 nEvent, f32 fVal ) {

	if( !_bLevelOK ) {
		return;
	}

	FASSERT( nEvent < BARTER_SYSTEM_EVENTS_SLIM_COUNT );

	switch( nEvent ) {

	default:
	case BARTER_SYSTEM_EVENTS_NONE:
		break;

	case BARTER_SYSTEM_EVENTS_SLIM_LOOK:
	case BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_LAUGH:
	case BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_NOD:
	case BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_SHAKE:
	case BARTER_SYSTEM_EVENTS_SLIM_SHADY_IS_WELCOMING:
	case BARTER_SYSTEM_EVENTS_SLIM_WAVE:
		// tell slim these things
		_pSlim->EventNotify( nEvent, fVal );
		break;

	case BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM:
//		_pShady->EventNotify( nEvent, fVal );
		break;
	}	
}

static FDrawVtx_t _aavtxArrow[4][4];
static FDrawVtx_t _avtxButtonPurchase[4];
static FDrawVtx_t _avtxButtonLeave[4];
static CShadowedText _textPurchase;
static CShadowedText _textNoThanks;

static f32 _afArrowState[4]; // < 0 means off, 0 means on, > 0, means lit;

static const CFColorRGBA _argbaArrowFlash[2] = 
{ 
	CFColorRGBA(0.80f, 0.80f, 0.80f, 0.90f),
	CFColorRGBA(0.75f, 0.75f, 0.75f, 0.35f),
};

void _InitUserInterface(BOOL bCreateText)
{
	FASSERT(CPlayer::m_nCurrent < MAX_PLAYERS);
	f32 fHalfResX = Player_aPlayer[CPlayer::m_nCurrent].m_pViewportSafeOrtho3D->HalfRes.x;
	f32 fHalfResY = Player_aPlayer[CPlayer::m_nCurrent].m_pViewportSafeOrtho3D->HalfRes.y;

	const f32 fOriginX = (-236.0f / 272.0f) * fHalfResX;
	const f32 fOriginY = (-160.0f / 204.0f) * fHalfResY;
	const f32 fHorzOffset = (8.0f / 272.0f) * fHalfResX;
	const f32 fVertOffset = (8.0f /204.0f)  * fHalfResY;
	
	const f32 fWidth =  (28.0f / 272.0f) * fHalfResX;
    const f32 fHeight = (28.0f / 204.0f) * fHalfResY;
	const f32 fHalfWidth  = fWidth * 0.50f;
    const f32 fHalfHeight = fHeight* 0.50f;

	//// Set up the vertices for the arrow.
	//
	const f32 fS1 = 0.0f;//160.0f / 256.0f;
	const f32 fS2 = 0.25f;//192.0f / 256.0f;
	const f32 fT1 = 0.753f;//0.0f / 256.0f;
	const f32 fT2 = 1.0f;//32.0f / 256.0f;

	_aavtxArrow[0][0].Pos_MS.Set(fOriginX-fHalfWidth,			fOriginY+fVertOffset+fHeight,100.0f);
	_aavtxArrow[0][1].Pos_MS.Set(fOriginX-fHalfWidth + fWidth, fOriginY+fVertOffset+fHeight,100.0f);
	_aavtxArrow[0][2].Pos_MS.Set(fOriginX-fHalfWidth,			fOriginY+fVertOffset,		 100.0f);
	_aavtxArrow[0][3].Pos_MS.Set(fOriginX-fHalfWidth + fWidth, fOriginY+fVertOffset,		 100.0f);
	_afArrowState[0] = ARROW_ON;

	_aavtxArrow[1][0].Pos_MS.Set(fOriginX-fHalfWidth,			fOriginY-fVertOffset,		 100.0f);
	_aavtxArrow[1][1].Pos_MS.Set(fOriginX-fHalfWidth + fWidth, fOriginY-fVertOffset,		 100.0f);
	_aavtxArrow[1][2].Pos_MS.Set(fOriginX-fHalfWidth,			fOriginY-fVertOffset-fHeight,100.0f);
	_aavtxArrow[1][3].Pos_MS.Set(fOriginX-fHalfWidth + fWidth, fOriginY-fVertOffset-fHeight,100.0f);
	_afArrowState[1] = ARROW_ON;

	_aavtxArrow[2][0].Pos_MS.Set(fOriginX+fHorzOffset,			 fOriginY+fHalfHeight,		 100.0f);
	_aavtxArrow[2][1].Pos_MS.Set(fOriginX+fHorzOffset + fWidth, fOriginY+fHalfHeight,		 100.0f);
	_aavtxArrow[2][2].Pos_MS.Set(fOriginX+fHorzOffset,			 fOriginY-fHalfHeight,		 100.0f);
	_aavtxArrow[2][3].Pos_MS.Set(fOriginX+fHorzOffset + fWidth, fOriginY-fHalfHeight,		 100.0f);
	_afArrowState[2] = ARROW_ON;

	_aavtxArrow[3][0].Pos_MS.Set(fOriginX-fHorzOffset - fWidth, fOriginY+fHalfHeight,		 100.0f);
	_aavtxArrow[3][1].Pos_MS.Set(fOriginX-fHorzOffset,			 fOriginY+fHalfHeight,		 100.0f);
	_aavtxArrow[3][2].Pos_MS.Set(fOriginX-fHorzOffset - fWidth, fOriginY-fHalfHeight,		 100.0f);
	_aavtxArrow[3][3].Pos_MS.Set(fOriginX-fHorzOffset,			 fOriginY-fHalfHeight,		 100.0f);
	_afArrowState[3] = ARROW_ON;

	// 0 = Upward.
	_aavtxArrow[0][0].ST.Set(fS1, fT1);
	_aavtxArrow[0][1].ST.Set(fS2, fT1);
	_aavtxArrow[0][2].ST.Set(fS1, fT2);
	_aavtxArrow[0][3].ST.Set(fS2, fT2);

	// 1 = Downward.
	_aavtxArrow[1][0].ST.Set(fS1, fT2);	
	_aavtxArrow[1][1].ST.Set(fS2, fT2);
	_aavtxArrow[1][2].ST.Set(fS1, fT1);
	_aavtxArrow[1][3].ST.Set(fS2, fT1);

	// 2 = Leftward.
	_aavtxArrow[2][0].ST.Set(fS2, fT2);
	_aavtxArrow[2][1].ST.Set(fS2, fT1);
	_aavtxArrow[2][2].ST.Set(fS1, fT2);
	_aavtxArrow[2][3].ST.Set(fS1, fT1);

	// 3 = Rightward.
	_aavtxArrow[3][0].ST.Set(fS1, fT1);
	_aavtxArrow[3][1].ST.Set(fS1, fT2);
	_aavtxArrow[3][2].ST.Set(fS2, fT1);
	_aavtxArrow[3][3].ST.Set(fS2, fT2);

	f32 fU1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].x;
	f32 fV1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_A].y;
	f32 fU2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].x;
	f32 fV2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_A].y;

	_avtxButtonPurchase[0].ST.Set(fU1,fV1);
	_avtxButtonPurchase[1].ST.Set(fU2,fV1);
	_avtxButtonPurchase[2].ST.Set(fU1,fV2);
	_avtxButtonPurchase[3].ST.Set(fU2,fV2);
	_avtxButtonPurchase[0].ColorRGBA.OpaqueWhite();
	_avtxButtonPurchase[1].ColorRGBA.OpaqueWhite();
	_avtxButtonPurchase[2].ColorRGBA.OpaqueWhite();
	_avtxButtonPurchase[3].ColorRGBA.OpaqueWhite();

#if FANG_PLATFORM_DX
	const f32 fAcceptX = (-272.0f / 272.0f) * fHalfResX;
	const f32 fAcceptY = (80.0f / 204.0f) * fHalfResY;
	const f32 fAcceptWidth = (32.0f / 272.0f) * fHalfResX;
	const f32 fAcceptHeight= (32.0f / 204.0f) * fHalfResY;

	const f32 fLeaveX = (-276.0f / 272.0f) * fHalfResX;
	const f32 fLeaveY = (52.0f / 204.0f) * fHalfResY;
	const f32 fLeaveWidth = (32.0f / 272.0f) * fHalfResX;
	const f32 fLeaveHeight= (32.0f / 204.0f) * fHalfResY;

	fU1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].x;
	fV1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].y;
	fU2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].x;
	fV2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].y;

#elif FANG_PLATFORM_GC
	const f32 fAcceptX = (-275.0f / 272.0f) * fHalfResX;
	const f32 fAcceptY = (80.0f / 204.0f) * fHalfResY;
	const f32 fAcceptWidth = (32.0f / 272.0f) * fHalfResX;
	const f32 fAcceptHeight= (32.0f / 204.0f) * fHalfResY;

	const f32 fLeaveX = (-279.0f / 272.0f) * fHalfResX;
	const f32 fLeaveY = (52.0f / 204.0f) * fHalfResY;
	const f32 fLeaveWidth = (32.0f / 272.0f) * fHalfResX;
	const f32 fLeaveHeight= (32.0f / 204.0f) * fHalfResY;

	fU1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_X].x;
	fV1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_X].y;
	fU2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_X].x;
	fV2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_X].y;
#else
	const f32 fAcceptX = (-272.0f / 272.0f) * fHalfResX;
	const f32 fAcceptY = (80.0f / 204.0f) * fHalfResY;
	const f32 fAcceptWidth = (32.0f / 272.0f) * fHalfResX;
	const f32 fAcceptHeight= (32.0f / 204.0f) * fHalfResY;

	const f32 fLeaveX = (-276.0f / 272.0f) * fHalfResX;
	const f32 fLeaveY = (52.0f / 204.0f) * fHalfResY;
	const f32 fLeaveWidth = (32.0f / 272.0f) * fHalfResX;
	const f32 fLeaveHeight= (32.0f / 204.0f) * fHalfResY;

	fU1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].x;
	fV1 = CPauseScreen_avecButtonST1[PAUSESCREEN_BUTTON_ST_B].y;
	fU2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].x;
	fV2 = CPauseScreen_avecButtonST2[PAUSESCREEN_BUTTON_ST_B].y;
#endif

	_avtxButtonLeave[0].ST.Set(fU1,fV1);
	_avtxButtonLeave[1].ST.Set(fU2,fV1);
	_avtxButtonLeave[2].ST.Set(fU1,fV2);
	_avtxButtonLeave[3].ST.Set(fU2,fV2);
	
	_avtxButtonLeave[0].ColorRGBA.OpaqueWhite();
	_avtxButtonLeave[1].ColorRGBA.OpaqueWhite();
	_avtxButtonLeave[2].ColorRGBA.OpaqueWhite();
	_avtxButtonLeave[3].ColorRGBA.OpaqueWhite();

	_avtxButtonPurchase[0].Pos_MS.Set(fAcceptX,	             fAcceptY,100.0f);
	_avtxButtonPurchase[1].Pos_MS.Set(fAcceptX+fAcceptWidth, fAcceptY,100.0f);
	_avtxButtonPurchase[2].Pos_MS.Set(fAcceptX,              fAcceptY-fAcceptHeight,100.0f);
	_avtxButtonPurchase[3].Pos_MS.Set(fAcceptX+fAcceptWidth, fAcceptY-fAcceptHeight,100.0f);

	_avtxButtonLeave[0].Pos_MS.Set(fLeaveX,	            fLeaveY,100.0f);
	_avtxButtonLeave[1].Pos_MS.Set(fLeaveX+fLeaveWidth, fLeaveY,100.0f);
	_avtxButtonLeave[2].Pos_MS.Set(fLeaveX,				fLeaveY-fLeaveHeight,100.0f);
	_avtxButtonLeave[3].Pos_MS.Set(fLeaveX+fLeaveWidth, fLeaveY-fLeaveHeight,100.0f);

	if( !bCreateText ) return;
	FTextArea_t BuyTextArea;
	ftext_SetToDefaults(&BuyTextArea);
	BuyTextArea.fNumberOfLines = 1.0f;
	BuyTextArea.ohFont = '3';
	BuyTextArea.bVisible = FALSE;
	BuyTextArea.fUpperLeftX = .050f;
	BuyTextArea.fLowerRightX =.300f;
	BuyTextArea.fUpperLeftY = .247f;
	BuyTextArea.fLowerRightY = .267f;

	BuyTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
	_textPurchase.Create(&BuyTextArea);

	FTextArea_t LeaveTextArea;
	ftext_SetToDefaults(&LeaveTextArea);
	LeaveTextArea.fNumberOfLines = 1.0f;
	LeaveTextArea.ohFont = '3';
	LeaveTextArea.bVisible = FALSE;
	LeaveTextArea.fUpperLeftX = .050f;
	LeaveTextArea.fLowerRightX =.300f;
	
	LeaveTextArea.fUpperLeftY = .297f;
	LeaveTextArea.fLowerRightY = .317f;

	LeaveTextArea.oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.4f );
	LeaveTextArea.oColorBorder.Set( 0.0f, 0.0f, 0.0f, 1.0f );

	LeaveTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_LEFT;
	_textNoThanks.Create(&LeaveTextArea);
}

void _DrawUserInterface(void)
{
	if (CBarterLevel::m_oTextArea.bVisible==FALSE)
		return;

	fviewport_SetActive( CPlayer::m_pCurrent->m_pViewportSafeOrtho3D );
#if defined(MA_PC_INPUT)
	// Keyboard prompts print their labels beside the key caps instead (see below).
	if( !pcinput_UseKeyboardPromptsForPort( CPlayer::m_pCurrent->m_nControllerIndex ) )
#endif
	{
		_textPurchase.PrintString(Game_apwszPhrases[ GAMEPHRASE_PURCHASE]);
		_textNoThanks.PrintString(Game_apwszPhrases[ GAMEPHRASE_NO_THANKS]);
	}
	for(u32 m_nDrawDir = 0; m_nDrawDir < 4; ++m_nDrawDir)
	{
		for(u32 uCurIdx = 0; uCurIdx < 4; ++uCurIdx)
		{
			_aavtxArrow[m_nDrawDir][uCurIdx].ColorRGBA = ( (_afArrowState[m_nDrawDir] > 0.0f) ? _argbaArrowFlash[0] : _argbaArrowFlash[1]);
		}
	}

	frenderer_SetDefaultState();
	FASSERT(CPlayer::m_nCurrent < MAX_PLAYERS);
	fviewport_SetActive(Player_aPlayer[CPlayer::m_nCurrent].m_pViewportSafeOrtho3D);
	fdraw_Depth_EnableWriting(FALSE);
	fdraw_Depth_SetTest(FDRAW_DEPTHTEST_ALWAYS);
	fdraw_Alpha_SetBlendOp(FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
	
#if defined(MA_PC_INPUT)
	FViewport_t *pViewport = Player_aPlayer[CPlayer::m_nCurrent].m_pViewportSafeOrtho3D;
	const u32 nPromptPort = CPlayer::m_pCurrent->m_nControllerIndex;
	const FDrawVtx_t *apButtons[2] = { _avtxButtonPurchase, _avtxButtonLeave };
	cwchar *apwszKeyLabels[2] = { L"Space", L"R" };
	for( u32 nButton = 0; nButton < 2; nButton++ ) {
		const f32 fCenterX = 0.5f * (apButtons[nButton][0].Pos_MS.x + apButtons[nButton][3].Pos_MS.x);
		const f32 fCenterY = 0.5f * (apButtons[nButton][0].Pos_MS.y + apButtons[nButton][3].Pos_MS.y);
		const f32 fCenterXFrac = 0.5f * (fCenterX / pViewport->HalfRes.x + 1.0f);
		const f32 fCenterYFrac = 0.5f * (1.0f - fCenterY / pViewport->HalfRes.y);
		const f32 fRadius = 0.5f * FMATH_FABS( apButtons[nButton][0].Pos_MS.y - apButtons[nButton][2].Pos_MS.y ) /
			pViewport->Res.y;
		if( pcinput_UseKeyboardPromptsForPort( nPromptPort ) ) {
			// Key cap and its label together, in whole-screen pixels (where ftext prints), at the top
			// left of this player's safe area; a key name like Space is wider than the pad button art.
			static cwchar *apwszKeyText[2] = { NULL, NULL };
			apwszKeyText[0] = Game_apwszPhrases[ GAMEPHRASE_PURCHASE ];
			apwszKeyText[1] = Game_apwszPhrases[ GAMEPHRASE_NO_THANKS ];
			const FViewport_t *pScreen = FViewport_pDefaultOrtho;
			const f32 fLeft = pViewport->ScreenCorners.UpperLeft.x * pScreen->OORes.x + 0.01f;
			const f32 fRowY = (pViewport->ScreenCorners.UpperLeft.y + pViewport->Res.y * (0.13f + 0.08f * (f32)nButton)) * pScreen->OORes.y;
			f32 fScale = pViewport->Res.y * pScreen->OORes.y;	// smaller in a split screen
			FMATH_CLAMP( fScale, 0.5f, 1.0f );
			fScale *= 0.62f;
			fviewport_SetActive( FViewport_pDefaultOrtho );
			f32 fCapRight = fLeft + 0.08f;
			wpr_drawutils_DrawKeyCapCentered( apwszKeyLabels[nButton], fLeft, fRowY, L'L', fScale, 0.075f,
				pScreen->HalfRes.x, pScreen->HalfRes.y, NULL, NULL, &fCapRight, NULL, TRUE );
			wpr_drawutils_PrintPromptCentered( apwszKeyText[nButton], FMATH_MAX( fCapRight, fLeft + 0.075f ) + 0.012f, fRowY, L'L', fScale,
				WprDataTypes_pwszSolidWhiteTextColor );
			fviewport_SetActive( pViewport );
		} else {
			wpr_drawutils_DrawFaceButton( pcinput_UsePlayStationPromptsForPort( nPromptPort ), nButton, fCenterXFrac,
				fCenterYFrac, fRadius, pViewport->HalfRes.x, pViewport->HalfRes.y );
		}
	}
#else
	fdraw_SetTexture(CHud2::GetTexControls());
	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, _avtxButtonPurchase, 4);
	fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, _avtxButtonLeave, 4);
#endif

	// PC button/key glyphs use solid-color drawing; restore textured alpha for the navigation arrows.
	fdraw_Color_SetFunc(FDRAW_COLORFUNC_DIFFUSETEX_AIAT);
	fdraw_SetTexture(CHud2::GetTexHud());
	// selecting turn off unneeded arrows
	switch (_nCurTableSlotIndex)
	{
		case 0: // items
			if (_afArrowState[2] == ARROW_ON) // right arrow
				_afArrowState[2] = ARROW_OFF;
			break;

		case 1: // ammo
			if (_afArrowState[2] == ARROW_ON) // right arrow
				if (_BarterLevel.GetSlotItemCount(0) == 0)
					_afArrowState[2] = ARROW_ON;

			if (_afArrowState[3] == ARROW_ON) // left arrow
				if (_BarterLevel.GetSlotItemCount(2) == 0)
					_afArrowState[3] = ARROW_OFF;
			break;

		case 2:
			if (_afArrowState[3] == ARROW_ON) // left arrow
				_afArrowState[3] = ARROW_OFF;
			break;
	}

	for(u32 m_nDrawDir = 0; m_nDrawDir < 4; ++m_nDrawDir)
	{
		if (_afArrowState[m_nDrawDir] > 0.0f)
		{
			_afArrowState[m_nDrawDir] -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0(_afArrowState[m_nDrawDir])
		}
		else if (_afArrowState[m_nDrawDir] < 0.0f)
		{
			continue;
		}
		
		fdraw_PrimList(FDRAW_PRIMTYPE_TRISTRIP, _aavtxArrow[m_nDrawDir], 4);
	}
}

void _RefreshArrows(BOOL bUpSale)
{
	if( !_bLevelOK ) 
	{
		return;
	}
	FASSERT(_BarterLevel.GetActiveTable());

	if (bUpSale)
	{
		_afArrowState[0] = _afArrowState[1] = _afArrowState[2] = _afArrowState[3] =  ARROW_OFF; // all arrows off 
		return;
	}
	
	u32 nItems =  _BarterLevel.GetSlotItemCount(0);
	u32 nAmmos =  _BarterLevel.GetSlotItemCount(1);
	u32 nWeapons=  _BarterLevel.GetSlotItemCount(2);

	switch (_nCurTableSlotIndex)
	{
	case 0: // starting slot items;
		_afArrowState[0] = _afArrowState[1] = ((nItems > 1) ? ARROW_ON : ARROW_OFF); // up/down arrows
		_afArrowState[2] =  ARROW_OFF; // right arrow off
		_afArrowState[3] =  (((nAmmos > 0)||(nWeapons)>0) ? ARROW_ON : ARROW_OFF); // left arrow on
		break;
	case 1: // starting slot ammos;
		_afArrowState[0] = _afArrowState[1] = ((nAmmos > 1) ? ARROW_ON : ARROW_OFF); // up/down arrow 
		_afArrowState[2] =  ((nItems > 0) ? ARROW_ON : ARROW_OFF); // right arrow on
		_afArrowState[3] =  ((nWeapons > 0) ? ARROW_ON : ARROW_OFF); // left arrow on
		break;
	case 2: // starting slot weapons;
		_afArrowState[0] = _afArrowState[1] = ((nWeapons > 1) ? ARROW_ON : ARROW_OFF); // up arrow 
		_afArrowState[2] =  ((nAmmos > 0)|| nItems > 0) ? ARROW_ON : ARROW_OFF; // right arrow
		_afArrowState[3] =  ARROW_OFF; // left arrow off
		break;
	}
}

// Assumes the FDraw renderer is currently active.
// Assumes the ortho3D camera is set up and positioned
// at (0,0,0) and looking down the +Z axis.
// Assumes there are no model Xfms on the stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void bartersystem_Draw( void ) 
{
	if( !_bLevelOK ) 
	{
		return;
	}

	#if FANG_WINGC
	if( CPlayer::m_nCurrent == _nBarterPlayer )
	#endif
		_DrawUserInterface();

	CFVec3A vecPosition;
	if( _pSlim->ShowFilter( &vecPosition ) ) 
	{
		CFVec3A vecTL;
		CFVec3A vecTR;
		CFVec3A vecBR;
		CFVec3A vecBL;
		CFVec3A	vecFilterPoint;

		FViewport_t *pView	 = CPlayer::m_pCurrent->m_pViewportPersp3D;
		CFCamera	*pCamera = gamecam_GetActiveCamera();
		fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &pCamera->GetFinalXfm()->m_MtxF, &vecPosition, &vecFilterPoint );
		fviewport_SetActive( CPlayer::m_pCurrent->m_pViewportOrtho3D );

		frenderer_SetDefaultState();

		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );

		vecFilterPoint.x += .02f;
		vecFilterPoint.y += .055f;

		vecTL.Add( vecFilterPoint, _vecCensorBarDLU );
		vecTR.Add( vecFilterPoint, _vecCensorBarDRU );
		vecBR.Sub( vecFilterPoint, _vecCensorBarDLU );
		vecBL.Sub( vecFilterPoint, _vecCensorBarDRU );

		vecTL.x *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.x;
		vecTL.y *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.y;

		vecTR.x *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.x;
		vecTR.y *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.y;

		vecBL.x *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.x;
		vecBL.y *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.y;

		vecBR.x *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.x;
		vecBR.y *= CPlayer::m_pCurrent->m_pViewportOrtho3D->HalfRes.y;

		vecTL.z = 100.0f;
		vecTR.z = 100.0f;
		vecBR.z = 100.0f;
		vecBL.z = 100.0f;

		fdraw_SolidQuad( &vecTL.v3, &vecTR.v3, &vecBR.v3, &vecBL.v3, &FColor_MotifBlack );

		//frenderer_Fog_Enable( bFogEnabled );
//fviewport SetActive ortho
	}
}


void bartersystem_CheckpointSave( void ) {
	// get and remember a data handle for this entity's checkpoint save data.
	// all subsequent data saved with SaveData() will be assigned to this handle,
	// until the next entity calls CreateObjectDataHandle() for itself.
	_hSaveData[ CFCheckPoint::GetCheckPoint() ] = CFCheckPoint::CreateObjectDataHandle();

	CFCheckPoint::SaveData( (u32)_eState );
	CFCheckPoint::SaveData( _nBarterPtSelected );

	if( !_bLevelOK ) {
		return;
	}

	u32 nNumTables = CBarterLevel::GetNumTables();
	CBarterTable *pTable = NULL;
	for (u32 nTable=0; nTable < nNumTables; nTable++)
	{
		pTable = CBarterLevel::GetTable(nTable);
		pTable->CheckPointSave();
	}
}


// Call after the game has been restored, should
// pull the barters out of whatever state they were in,
// reset all internal vars to BARTERSTATE_NOT_IN_WORLD state
void bartersystem_CheckpointRestore( void ) {
#if FANG_WINGC
	for( s32 n=0; n<MAX_PLAYERS; ++n ) _abCoopEmptyOffer[n] = FALSE;
#endif
	BarterState_e nSavedState;
	s16 nSavedBartPointIndex;

	// tell the checkpoint system what data handle to load data from.
	// all subsequent data loaded via LoadData() will be retrieved from
	// this handle's data.
	CFCheckPoint::SetObjectDataHandle( _hSaveData[ CFCheckPoint::GetCheckPoint() ] );

	CFCheckPoint::LoadData( (u32 &)nSavedState );
	CFCheckPoint::LoadData( nSavedBartPointIndex );

	if( !_bLevelOK ) {
		return;
	}

	u32 nNumTables = CBarterLevel::GetNumTables();
	CBarterTable *pTable = NULL;
	for (u32 nTable=0; nTable < nNumTables; nTable++)
	{
		pTable = CBarterLevel::GetTable(nTable);
		pTable->CheckPointRestore();
	}

	if( !_bLevelOK ) {
		return;
	}

	const s16 nPreviousPoint = _nBarterPtSelected;
	const BOOL bPreviouslyPresent = bartersystem_AreInWorld();
	bartersystem_RemoveFromWorld( FALSE );
	bartersystem_Hide( nSavedState == BARTERSTATE_HIDDEN );
#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() && nSavedState == BARTERSTATE_NOT_IN_WORLD &&
		bPreviouslyPresent && nPreviousPoint >= 0 ) {
		nSavedState = BARTERSTATE_IDLE;
		nSavedBartPointIndex = nPreviousPoint;
	}
#endif

	if( nSavedBartPointIndex<0 || nSavedState==BARTERSTATE_NOT_IN_WORLD || nSavedState==BARTERSTATE_HIDDEN ) {
		return;
	}

	bartersystem_PlaceInWorldAndStartAttracting( _apBarterPts[nSavedBartPointIndex]->Name() );
}


//==================
// private functions

static BOOL _LoadBarterInfoFromFile( cchar *pszCSVFile ) {
	cchar *pszBarterInfoFile;
	FGameData_VarType_e nDataType;
	FGameDataTableHandle_t hTableHandle;

	if( pszCSVFile == NULL ) {
		return(FALSE);
	}

	// Read the .CSV file here. (should already be loaded from when the level was loaded up)
	FGameDataFileHandle_t hFile = (FGameDataFileHandle_t)fresload_Load(FGAMEDATA_RESTYPE, pszCSVFile);
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		goto _ExitWithError;
	}

	// find the table we are interested in
	hTableHandle = fgamedata_GetFirstTableHandle( hFile, "BarterInfo" );
	if( hTableHandle == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "_LoadBarterInfoFromFile() : No barter info found for this level.\n" );
		goto _ExitWithError;
	}

	// get the number of fields in the table
	if( fgamedata_GetNumFields(hTableHandle) != 1 ) {
		DEVPRINTF( "_LoadBarterInfoFromFile() : Wrong number of parameters for BarterInfo table.\n" );
		goto _ExitWithError;
	}

	pszBarterInfoFile = (cchar *)fgamedata_GetPtrToFieldData( hTableHandle, 0, nDataType );

	if( !_BarterLevel.InitLevel( pszBarterInfoFile ) ) {
		DEVPRINTF( "_LoadBarterInfoFromFile() : Error initializing barter info.\n" );
		goto _ExitWithError;
	}


	return TRUE;

	// Error:
_ExitWithError:
	return FALSE;
}

// returns the index of the closest point that the player can't see,
// this pt should be used as the next location for the barters.
// returns -1 if no good point was found
static s32 _GetNearestUnseenPt() {
	s32 nBestPos = -1;
	u32 i;
	f32 fDist2, fBestDist2;

#if _DEBUG_BARTER_DROIDS
	// just force the drioids to the 1st spot
	return 0;
#endif

	for( i=0; i < _nBarterPtCnt; i++ ) {
		if( !_CanPlayerSeePt( i, fDist2 ) ) {
			if( nBestPos >= 0 ) {
				if( fDist2 < fBestDist2 ) {
					fBestDist2 = fDist2;
					nBestPos = i;
				}
			} else {
				fBestDist2 = fDist2;
				nBestPos = i;
			}
		}
	}
	
	return nBestPos;
}

// Tests to see if the player can see the pt at nBarterPtIdx
static BOOL _CanPlayerSeePt( s32 nBarterPtIdx, f32 &rfDist2 ) {
	CBot *pPlayer = (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent;
	CFMtx43A *pMtxToWorld = GetBarterPtMtx(nBarterPtIdx);
	CFVec3A vecStart = pMtxToWorld->m_vPos;
	vecStart.y += 5.0f;
	CFVec3A vecEnd = *pPlayer->m_pApproxEyePoint_WS;

	// consider the barters "seen" if the player is really close to them
	CFVec3A DistBetween( vecStart );
	DistBetween.Sub( vecEnd );
	rfDist2 = DistBetween.MagSq();
	if( rfDist2 < _DIST2_TO_SEE_PLAYER ) {
		// the barters are less than _DIST2_TO_SEE_PLAYER feet away from the player
		return TRUE;
	}	

	// test the barters bounding sphere against the player's viewport frustrum
	CFSphere sphTemp;
	sphTemp.m_Pos = pMtxToWorld->m_vPos.v3;
	sphTemp.m_fRadius = 15.0f;
	if( fviewport_TestSphere_WS( CPlayer::m_pCurrent->m_pViewportPersp3D, &sphTemp, FVIEWPORT_PLANESMASK_ALL ) != -1 ) {
		// in current frustrum
		return TRUE;
	}

	// Line of sight check. (only if a reasonable distance from the player
	if( rfDist2 < _FURTHEST_DIST2_TO_SEE_PLAYER ) {
		FWorld_nTrackerSkipListCount = 0;
		_pSlim->AppendTrackerSkipList();
		_pShady->AppendTrackerSkipList();
		pPlayer->AppendTrackerSkipList();
		
		if( !fworld_IsLineOfSightObstructed( &vecStart, &vecEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList ) ) {
			// the droids are in the sight line of the player
			return TRUE;
		}
	}
	
	// THE PLAYER IS NOT IN THE PLAYER'S VIEW FRUSTRUM, LINE OF SIGHT, OR _DIST2_TO_SEE_PLAYER BOUNDING SPHERE
	return FALSE;
}

// returns TRUE if the barters are in front of us and the player is looking in their direction
static BOOL _IsPlayerInFrontAndLookingAtBarters( s32 nBarterPtIdx, CBot *pPlayer ) {
	CFVec3A vecPlayerLook = pPlayer->m_MountUnitFrontXZ_WS;
	CFVec3A vecBarterLook = GetBarterPtMtx(nBarterPtIdx)->m_vFront;
	
	// test to see if the player is looking in the direction of the barters
	if( vecPlayerLook.Dot( vecBarterLook ) > -0.25f ) {
		return FALSE;
	}
	// test to see if the player is actually infront of the barters
	vecPlayerLook = *pPlayer->m_pApproxEyePoint_WS;
	
	CFVec3A vecTemp;
	CFVec3 MidBarterPt;
	MidBarterPt.ReceiveLerpOf( 0.5f, _pSlim->m_pMesh->GetBoundingSphere_WS().m_Pos, _pShady->GetBoundingSphere().m_Pos );
	vecTemp.Set( MidBarterPt );
	
	vecPlayerLook.Sub( vecTemp );
	
	vecPlayerLook.Unitize();
	if( vecPlayerLook.Dot( vecBarterLook ) < 0.60f ) {
		return FALSE;
	}

	return TRUE;
}

static void _InsertBartersIntoWorld( u32 nNewPosIdx ) {

	if( nNewPosIdx == (u32)_nBarterPtSelected ) {
		// we are already at the requested point, don't do anything
		return;
	}
	FASSERT( nNewPosIdx < _nBarterPtCnt );

	_nBarterPtSelected = nNewPosIdx;

	// Find the positions for Slim relative to the barter point
	CFMtx43A *pMtxToWorld = GetBarterPtMtx(_nBarterPtSelected);
	CFVec3A vecTemp = pMtxToWorld->m_vRight;
	CFMtx43A NewMtx;
	
	NewMtx = *pMtxToWorld;
	vecTemp.Mul( 2.5f );
	NewMtx.m_vPos.Add( vecTemp );

	_pSlim->Relocate_WS( &NewMtx );
	
	// Find the positions for Shady relative to the barter point
	NewMtx = *pMtxToWorld;
	vecTemp.Mul( -1.05f );
	NewMtx.m_vPos.Add( vecTemp );

	vecTemp = pMtxToWorld->m_vFront;
	vecTemp.Mul( 0.45f );
	NewMtx.m_vPos.Add( vecTemp );
	
	CFMtx43A::m_RotY.SetRotationY( FMATH_DEG2RAD( 20.0f ) );
	NewMtx.Mul( CFMtx43A::m_RotY );

	// position the box real quick before putting shady on top of it
	_pBoxMeshEntity->Relocate_RotXlatFromUnitMtx_WS( &NewMtx, FALSE );

	vecTemp = pMtxToWorld->m_vUp;
	vecTemp.Mul( 1.20f );
	NewMtx.m_vPos.Add( vecTemp );

	_pShady->Relocate_WS( &NewMtx );
	
	// add slim, shady, and box to the world
	_Call_AddToWorld();

	// setup our state info
	_eState = BARTERSTATE_IDLE;
	// use the timer to time how long the barter have been in the world at a given location
	_fTimer = 0.0f;
	FASSERT(CBarterLevel::GetTable(_nBarterPtSelected));
	bartersystem_SetAttractRadius(_nBarterPtSelected, CBarterLevel::GetTable(_nBarterPtSelected)->m_fMusicRadius, CBarterLevel::GetTable(_nBarterPtSelected)->m_fSpeechRadius);
}

static void _MoveCamToBarterPos() {
	
	_CamInfo.m_fHalfFOV = FMATH_DEG2RAD( 42.5f );
	_CamInfo.m_pmtxMtx = &_CamMtx;

	_CamMtx.Identity();
	_CamMtx.RotateX( FMATH_DEG2RAD( 12.5f ) );

	CFMtx43A *pBarter = GetBarterPtMtx(_nBarterPtSelected);
	CFVec3A Dir( pBarter->m_vFront );
	Dir.Negate();
	Dir.UnitizeXZ();
	f32 fAngle = fmath_Atan( Dir.x, Dir.z );
	_CamMtx.RotateY( fAngle );

	CFVec3A Pos( pBarter->m_vPos );
	CFVec3A Temp;
	Temp.Mul( pBarter->m_vRight, 1.9f );
	Pos.Add( Temp );
	Temp.Mul( pBarter->m_vUp, 6.215f );
	Pos.Add( Temp );
	Temp.Mul( pBarter->m_vFront, 10.0f );
	Pos.Add( Temp );
	_CamMtx.m_vPos = Pos;

	gamecam_SwitchPlayerToManualCamera( PLAYER_CAM( CPlayer::m_nCurrent ), &_CamInfo );
}

static void _MoveCamToNormalPos() {
	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM( CPlayer::m_nCurrent ), (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent );
}

static BOOL _ActionBarter( CEntity *pActioningEntity, CEntity *pActionedEntity ) {

#if FANG_WINGC
	if( MultiplayerMgr.IsMultiplayer() || bartersystem_IsActive() ) return FALSE;
	s32 nShopper = -1;
	for( s32 n=0; n < CPlayer::m_nPlayerCount; ++n ) {
		if( Player_aPlayer[n].m_pEntityCurrent == pActioningEntity ) { nShopper = n; break; }
	}
	if( nShopper < 0 ) return FALSE;
	_abCoopEmptyOffer[nShopper] = FALSE;
	CBarterPlayerScope playerScope( nShopper );
#else
	if( CPlayer::m_nPlayerCount > 1 ) return FALSE;
#endif

	u32 i;
	CBot *pPlayer = (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent;

#if !_ALLOW_BARTER_DROID_INTERACTION
	return FALSE;
#endif

	if( !_bLevelOK ) {
		return FALSE;
	}

	if( !_IsPlayerInFrontAndLookingAtBarters( _nBarterPtSelected, pPlayer ) ) {
		return FALSE;
	}

	if( !_Call_Available() ) {
		// one of the bots are not available to begin barter session
		return FALSE;
	}

	////////////////////
	// Start Barter Mode
	
	// pick the current table configuration
	CBarterTable *pTable = _BarterLevel.GetTable( _apBarterPts[_nBarterPtSelected]->Name() );
	if( !pTable ) {
		// just select the first table
		DEVPRINTF( "BarterSystem() : A barter table for was not defined for the table named '%s', using the first table.\n", _apBarterPts[_nBarterPtSelected]->Name() );
		pTable = _BarterLevel.GetTable( (u32)0 );
		FASSERT( pTable );// we would not have enabled our system if we didn't have at least 1 valid table		
	}
	_BarterLevel.SetActiveTable( pTable );
	_BarterLevel.SelectStartingItems( pPlayer );
	
	// initialize to default everytime, so rechoose's center slot
	_nCurTableSlotIndex = 1;
	pTable->m_nCurrentSlot = 1;

	s32 nCurrentSlot = _BarterLevel.SetStartingSlotIndex();
	if (nCurrentSlot > -1)
		_nCurTableSlotIndex = nCurrentSlot;
	else {
		_BarterLevel.SetActiveTable( NULL );
#if FANG_WINGC
		if( MultiplayerMgr.IsLocalCoop() ) {
			_abCoopEmptyOffer[nShopper] = TRUE;
			_anCoopEmptyOfferPoint[nShopper] = _nBarterPtSelected;
			_anCoopEmptyOfferTicks[nShopper] = GetTickCount();
			DEVPRINTF( "Co-op shop: player %d has no available offers at '%s' (inventory needs/stock).\n",
				nShopper + 1, _apBarterPts[_nBarterPtSelected]->Name() );
		}
#endif
		return FALSE; // empty table!
	}
	if( !game_EnterBarterMode() ) {
		_BarterLevel.SetActiveTable( NULL );
		return FALSE;
	}
#if FANG_WINGC
	_nBarterPlayer = nShopper;
	_InitUserInterface( FALSE );
	DEVPRINTF( "Co-op shop: player %d entered.\n", nShopper + 1 );
#endif

	// ASSERT we're going into barter mode for sure at this point

	_bSpotlightWasOn = pPlayer->IsSpotLightOn();
	pPlayer->SetSpotLightOff(TRUE);

	// fill in the new table layout
	for( i=0; i < BARTER_TYPES_NUM_SLOTS; i++ ) {
		_TableData.apSlots[i] = _BarterLevel.GetSlotItemInst( i );		
	}
	_TableData.nCurrentSlot = _nCurTableSlotIndex;
	_TableData.bTableIsAnUpsale = FALSE;
	
	_RefreshArrows();

	// tell slim & shady that we are entering bartering mode
	_Call_Welcome();

	// force the player out of zoom mode
	CPlayer::AbortAllPlayerScopeZoomMode( TRUE );

	// move the camera to its barter position
	_MoveCamToBarterPos();

	// start the bartering tune
	_pBarterSound->StartBarteringTune();
	
	// compute a new orientation for the player
	CFMtx43A *pMtxToWorld = GetBarterPtMtx(_nBarterPtSelected);
	CFMtx43A NewPlayerMtx;
	NewPlayerMtx.m_vRight.v3 = -pMtxToWorld->m_vRight.v3;
	NewPlayerMtx.m_vUp = pMtxToWorld->m_vUp;
	NewPlayerMtx.m_vFront.v3 = -pMtxToWorld->m_vFront.v3;
	NewPlayerMtx.m_vPos = pMtxToWorld->m_vPos;

	// calculate how much to offset the player
	CFVec3A vecTemp = pMtxToWorld->m_vFront;
	vecTemp.Mul( 8.0f );
	NewPlayerMtx.m_vPos.Add( vecTemp );
	vecTemp = pMtxToWorld->m_vRight;
	vecTemp.Mul( 1.1f );
	NewPlayerMtx.m_vPos.Add( vecTemp );
		
	// move the player and disable his movement and kill any powerups that they may have had
	pPlayer->Relocate_RotXlatFromUnitMtx_WS( &NewPlayerMtx, FALSE );
	pPlayer->DrawEnable( FALSE );
	pPlayer->ImmobilizeBot();
	if( pPlayer->GetPowerupFx() ) {
		pPlayer->GetPowerupFx()->KillAll();
	}
	pPlayer->ZeroVelocity();

	// hide the hud except the washers
	CHud2::GetCurrentHud()->SetWasher( TRUE );
	CHud2::GetCurrentHud()->SetDrawFlags( CHud2::DRAW_WASHERS | CHud2::DRAW_ITEMS );
	
	// turn off the reticle
	//CPlayer::m_pCurrent->m_Reticle.EnableDraw( FALSE );
	pPlayer->ReticleEnable(FALSE);

	// notify the ai of the "cut scene"
	ai_NotifyCutSceneBegin();

	// init our state vars
	_fTimer = 0.0f;
	_nItemsPurchased = 0;
	_eState = BARTERSTATE_ACTIVE;

	return TRUE;
}

static void _ScrollSelector( s32 nScrollDir ) {
	s32 nOrigIndex, i;
	
	if( !_BarterLevel.IsInUpSaleMode() ) {
		// only move the selector if we are not in upsale mode
		nOrigIndex = _nCurTableSlotIndex;

		if( nScrollDir < 0 ) {
			// lower the current index to a min of 0
			for( i=(nOrigIndex-1); i >= 0; i-- ) {
				if( _BarterLevel.GetSlotItemInst( i ) ) {
					// there is a valid item being offered in this slot, set it as our current slot
					_nCurTableSlotIndex = i;
					break;
				}
			}
		} else {
			// raise the current index to a max of (BARTER_TYPES_NUM_SLOTS-1)
			for( i=(nOrigIndex+1); i < BARTER_TYPES_NUM_SLOTS; i++ ) {
				if( _BarterLevel.GetSlotItemInst( i ) ) {
					// there is a valid item being offered in this slot, set it as our current slot
					_nCurTableSlotIndex = i;
					break;
				}
			}
		}

		if( nOrigIndex != _nCurTableSlotIndex ) {
			// the current slot changed
			_BarterLevel.SetCurrentSlot( _nCurTableSlotIndex );
			_Call_SetCurrentSlot( _nCurTableSlotIndex );
			
			fsndfx_Play2D( _hScrollSound );

			switch (_nCurTableSlotIndex)
			{
			case 0: // left leaf
				_afArrowState[2] =  ARROW_HIGHLIGHT_TIME; // right arrow lit *then off*
				_afArrowState[3] =  ARROW_ON; // left arrow on
				break;
			case 1: // center leaf
				if (nOrigIndex == 0) // from left to right
				{
					_afArrowState[2] =  ARROW_ON; // right arrow on
					_afArrowState[3] =  ARROW_HIGHLIGHT_TIME; // left arrow lit
				}
				else
				{
					_afArrowState[2] =  ARROW_HIGHLIGHT_TIME; // right arrow lit
					_afArrowState[3] =  ARROW_ON; // left arrow on
				}
				break;
			case 2: // right leaf
				_afArrowState[2] =  ARROW_ON; // right arrow on
				_afArrowState[3] =  ARROW_HIGHLIGHT_TIME; // left arrow lit, then off
				break;
			}
			if (_BarterLevel.GetSlotItemCount(_nCurTableSlotIndex) > 1)
			{
				_afArrowState[0] =  ARROW_ON; // up arrow on
				_afArrowState[1] =  ARROW_ON; // down arrow on
			}
			else
			{
				_afArrowState[0] =  ARROW_OFF; // up arrow off
				_afArrowState[1] =  ARROW_OFF; // down arrow off
			}
		}
	}
}

static void _ScrollSalesChain( s32 nScrollDir ) 
{
	s32 nOrigIndex;
	BOOL bFlipDown = ( nScrollDir < 0 );
	if( !_BarterLevel.IsInUpSaleMode() ) 
	{
		// only move the selector if we are not in upsale mode
		nOrigIndex = _BarterLevel.GetCurrentSalesChainIndex();

		if( bFlipDown ) 
		{
			_BarterLevel.SetCurrentSalesChain( nOrigIndex-1 );
		}
		else if( nScrollDir > 0 ) 
		{
			_BarterLevel.SetCurrentSalesChain( nOrigIndex+1 );
		}
		
		if( nOrigIndex != _BarterLevel.GetCurrentSalesChainIndex() ) 
		{
			if (bFlipDown)
			{
				_afArrowState[1] = ARROW_HIGHLIGHT_TIME;
			}
			else
			{
				_afArrowState[0] = ARROW_HIGHLIGHT_TIME;
			}
			// the current sales chain changed
			// _BarterLevel.SetCurrentSlot( _nCurTableSlotIndex );
			_Call_SetCurrentSlot( _nCurTableSlotIndex, _BarterLevel.GetSlotItemInst(_nCurTableSlotIndex),bFlipDown );
			fsndfx_Play2D( _hScrollSound );
		}
	}
}

static void _Call_Work( BOOL bCanSeePlayer, f32 fDistSqToPlayer ) {
	_pSlim->Work( bCanSeePlayer, fDistSqToPlayer );
	_pShady->Work( bCanSeePlayer, fDistSqToPlayer );
}

static void _Call_AddToWorld() {
	_pSlim->AddToWorld();
	_pShady->AddToWorld();
	_pBoxMeshEntity->AddToWorld();	
}

static void _Call_RemoveFromWorld() {
	_pSlim->RemoveFromWorld();
	_pShady->RemoveFromWorld();
	_pBoxMeshEntity->RemoveFromWorld();
}

static void _Call_AppendTrackerSkipList() {
	FWorld_nTrackerSkipListCount = 0;
	_pSlim->AppendTrackerSkipList();
	_pShady->AppendTrackerSkipList();
}

static BOOL _Call_ReadyToBarter( void ) {
	return ( _pSlim->ReadyToBarter() && _pShady->ReadyToBarter() );
}

// returns TRUE if there is no wait needed, FALSE if we are now in a wait state
// NOTE: CALLS SHADY, WAITS FOR HIM TO FINISH, CALLS SLIM, WAITS FOR HIM TO FINISH
// assumes that the table has been setup
static void _RememberBarterIntroduction( void ) {
	if( MultiplayerMgr.IsLocalCoop() ) {
		for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
			if( Player_aPlayer[n].m_pPlayerProfile ) Player_aPlayer[n].m_pPlayerProfile->VisitedBarterDroids();
		}
	} else if( CPlayer::m_pCurrent->m_pPlayerProfile ) {
		CPlayer::m_pCurrent->m_pPlayerProfile->VisitedBarterDroids();
	}
}

static BOOL _Call_Welcome( void ) {
	FASSERT( _nWaitState == WAIT_STATE_NONE );

	_pShady->Welcome( &_TableData, _bFirstBarterEncounter );
	bartersystem_EventNotify( BARTER_SYSTEM_EVENTS_SLIM_SHADY_IS_WELCOMING, 0.0f );// let slim know not to start any long anims
	if( !_pShady->IsBusy() ) {
		// shady is done processing, tell slim
		_pSlim->Welcome( &_TableData, _bFirstBarterEncounter );
		_RememberBarterIntroduction();
		_bFirstBarterEncounter = FALSE;

		if( _pSlim->IsBusy() ) {
			// we need to wait for slim
			_nWaitState = WAIT_STATE_WELCOME_SLIM;
		} else {
			// slim is done, kill the wait mode
			_nWaitState = WAIT_STATE_NONE;
			return TRUE;
		}
	} else {
		// we have to wait for shady to finish processing the WELCOME command
		_nWaitState = WAIT_STATE_WELCOME_SHADY;
	}

	return FALSE;	
}

static void _Call_SetCurrentSlot( u32 nSlotIndex, CBarterItemInst *pNewBII, BOOL bFlipDown ) 
{
	FASSERT( _nWaitState == WAIT_STATE_NONE );
	_TableData.nCurrentSlot = nSlotIndex;
	_pSlim->SetCurrentSlot( nSlotIndex, pNewBII, bFlipDown );
	_pShady->SetCurrentSlot( nSlotIndex, pNewBII );
}

// returns TRUE if there is no wait needed, FALSE if we are now in a wait state
// NOTE: CALLS SLIM, WAITS FOR HIM TO FINISH, CALLS SHADY, WAITS FOR HIM TO FINISH
static BOOL _Call_Purchase( void ) {
	FASSERT( _nWaitState == WAIT_STATE_NONE );

	_pSlim->Purchase( _nUpsaleIndex );
	if( !_pSlim->IsBusy() ) {
		// slim is done processing the PURCHASE, tell shady
		_pShady->Purchase( _nUpsaleIndex );

		if( _pShady->IsBusy() ) {
			// we need to wait for shady
			_nWaitState = WAIT_STATE_PURCHASE_SHADY;
		} else {
			// shady is done, start a new sale
			return _Call_NewSale();
		}
	} else {
		// we have to wait for slim to finish processing the PURCHASE command
		_nWaitState = WAIT_STATE_PURCHASE_SLIM;
	}
	
	return FALSE;
}

// returns TRUE if there is no wait needed, FALSE if we are now in a wait state
// NOTE: CALLS SLIM, WAITS FOR HIM TO FINISH, CALLS SHADY, WAITS FOR HIM TO FINISH
static BOOL _Call_NewSale( void ) {
	CBot *pPlayer = (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent;
	u32 i;

	///////////////////////////////
	// fill in the new table layout
	_TableData.bTableIsAnUpsale = _BarterLevel.SelectNewItems( pPlayer, _bJustPurchasedItem );
	if( _TableData.bTableIsAnUpsale ) {
		// an upsale, set the table with an item in the middle only
		_TableData.apSlots[0] = _TableData.apSlots[2] = NULL;
		_TableData.apSlots[1] = _BarterLevel.GetCurrentItemInst();
		_nCurTableSlotIndex = 1;
		_TableData.nCurrentSlot = _nCurTableSlotIndex;
		_RefreshArrows(TRUE);
	} else {
		// not an upsale, set all of the slots of the table
		for( i=0; i < BARTER_TYPES_NUM_SLOTS; i++ ) {
			_TableData.apSlots[i] = _BarterLevel.GetSlotItemInst( i );		
		}
		s32 nCurrentSlot = _BarterLevel.SetStartingSlotIndex();
		if (nCurrentSlot > -1)
		{
			_TableData.nCurrentSlot = _BarterLevel.SetStartingSlotIndex();
			_nCurTableSlotIndex = _TableData.nCurrentSlot;
			_RefreshArrows();
		}
		else // table is empty...
		{
			// turn off the washer drawing
			CHud2::GetCurrentHud()->SetWasher( FALSE );

			_fTimer = 0.0f;
			_eState = BARTERSTATE_ENDING;
			_nWaitState = WAIT_STATE_NONE;
			return _Call_Exit();
		}
	}

	///////////////////////////////
	// tell slim about the new sale
	_pSlim->NewSale( _nUpsaleIndex, &_TableData );
	if( !_pSlim->IsBusy() ) {
		// slim is done processing the NEW SALE command, tell shady
		_pShady->NewSale( _nUpsaleIndex, &_TableData );

		if( _pShady->IsBusy() ) {
			// wait for shady to finish
			_nWaitState = WAIT_STATE_NEW_SALE_SHADY;
		} else {
			// shady is done, no need to wait further
			_nWaitState = WAIT_STATE_NONE;
			return TRUE;
		}
	} else {
		// we have to wait for slim to finish processing the NEW SALE command	
		_nWaitState = WAIT_STATE_NEW_SALE_SLIM;
	}

	return FALSE;		
}

// returns TRUE if there is no wait needed, FALSE if we are now in a wait state
// NOTE: CALLS SLIM, WAITS FOR HIM TO FINISH, CALLS SHADY, WAITS FOR HIM TO FINISH
static BOOL _Call_FailedPurchase( void ) {
	FASSERT( _nWaitState == WAIT_STATE_NONE );

	_pSlim->FailedPurchase( _nUpsaleIndex );
	if( !_pSlim->IsBusy() ) {
		// slim is done processing the FAILED PURCHASE, tell shady
		_pShady->FailedPurchase( _nUpsaleIndex );

		if( _pShady->IsBusy() ) {
			// we need to wait for shady
			_nWaitState = WAIT_STATE_FAILED_SHADY;
		} else {
			// shady is done, start a new sale
			return _Call_NewSale();
		}
	} else {
		// we have to wait for slim to finish processing the FAILED PURCHASE command
		_nWaitState = WAIT_STATE_FAILED_SLIM;
	}
	
	return FALSE;	
}

// returns TRUE if there is no wait needed, FALSE if we are now in a wait state
// NOTE: CALLS SLIM, WAITS FOR HIM TO FINISH, CALLS SHADY, WAITS FOR HIM TO FINISH
static BOOL _Call_RejectedItem( void ) {
	FASSERT( _nWaitState == WAIT_STATE_NONE );

	_pSlim->RejectedItem( _nUpsaleIndex );
	if( !_pSlim->IsBusy() ) {
		// slim is done processing the REJECTED PURCHASE, tell shady
		_pShady->RejectedItem( _nUpsaleIndex );

		if( _pShady->IsBusy() ) {
			// we need to wait for shady
			_nWaitState = WAIT_STATE_REJECTED_SHADY;
		} else {
			// shady is done, start a new sale
			return _Call_NewSale();
		}
	} else {
		// we have to wait for slim to finish processing the REJECTED PURCHASE command
		_nWaitState = WAIT_STATE_REJECTED_SLIM;
	}

	return FALSE;
}

// returns TRUE if there is no wait needed, FALSE if we are now in a wait state
// NOTE: CALLS SHADY, WAITS FOR HIM TO FINISH, CALLS SLIM, WAITS FOR HIM TO FINISH
static BOOL _Call_Exit( void ) {
	FASSERT( _nWaitState == WAIT_STATE_NONE );

	CBarterLevel::m_oTextArea.bVisible = FALSE;
	ftext_SetAttributes(CBarterLevel::m_hTextDisplay,&CBarterLevel::m_oTextArea);
	
	_pShady->Exit( _nItemsPurchased );
	if( !_pShady->IsBusy() ) {
		// shady is done processing, tell slim
		_pSlim->Exit( _nItemsPurchased );

		if( _pSlim->IsBusy() ) {
			// we need to wait for slim to finish processing the EXIT command
			_nWaitState = WAIT_STATE_EXIT_SLIM;
		} else {
			// slim is done processing the EXIT command, no need to wait further
			_nWaitState = WAIT_STATE_NONE;
			return TRUE;
		}
	} else {
		// we have to wait for shady to finish processing the EXIT command
		_nWaitState = WAIT_STATE_EXIT_SHADY;
	}

	return FALSE;	
}

static BOOL _Call_Available( void ) {
	return ( _pSlim->Available() && _pShady->Available() );
}

static void _HandleCancelButton( CBot *pPlayer ) {
		
	if( _BarterLevel.IsInUpSaleMode() ) {
		// exit the upsale mode and go back to regular sale mode

		_nUpsaleIndex = _BarterLevel.HowDeepIsCurrentItemInSaleChain();
		_bJustPurchasedItem = FALSE;

		// tell slim & shady that we have rejected the item
		_Call_RejectedItem();

	} else {
		// exit barter mode altogether
		_nUpsaleIndex = _BarterLevel.HowDeepIsCurrentItemInSaleChain();
		FASSERT( _nUpsaleIndex == 0 );
		_bJustPurchasedItem = FALSE;
		_fTimer = 0.0f;
		_eState = BARTERSTATE_ENDING;
		
		// turn off the washer drawing
		CHud2::GetCurrentHud()->SetWasher( FALSE );

		// tell slim & shady that we are exiting barter mode
		_Call_Exit();
	}
}

static void _HandlePurchaseButton( CBot *pPlayer ) {	
	
	if( _BarterLevel.PurchaseCurrentItem( pPlayer ) ) {
		// bought the current item
		_nItemsPurchased++;

		_nUpsaleIndex = _BarterLevel.HowDeepIsCurrentItemInSaleChain();
		_bJustPurchasedItem = TRUE;

		// tell slim & shady that are purchased an item		
		_Call_Purchase();
					
	} else {
		// couldn't afford the current item

		_nUpsaleIndex = _BarterLevel.HowDeepIsCurrentItemInSaleChain();
		_bJustPurchasedItem = FALSE;

		// tell slim & shady about the failed purchase
		_Call_FailedPurchase();
	}
}

// returns TRUE if we are no longer in a wait state and regular work should be done
// returns FALSE if no regular work should be done this frame
static BOOL _WaitStateWork( void ) {
	
	switch( _nWaitState ) {

	case WAIT_STATE_NONE:
		return TRUE;
		
	case WAIT_STATE_WELCOME_SHADY:
		// waiting for shady to finish processing the welcome command
		if( !_pShady->IsBusy() ) {
			// shady is done processing, welcome slim
			_pSlim->Welcome( &_TableData, _bFirstBarterEncounter );
			_RememberBarterIntroduction();
			_bFirstBarterEncounter = FALSE;

			if( _pSlim->IsBusy() ) {
				// we need to wait for slim
				_nWaitState = WAIT_STATE_WELCOME_SLIM;
			} else {
				// slim is done, kill the wait mode
				_nWaitState = WAIT_STATE_NONE;
				return TRUE;
			}
		}
		break;

	case WAIT_STATE_WELCOME_SLIM:
		// waiting for slim to finish processing the welcome command
		if( !_pSlim->IsBusy() ) {
			// slim is done, kill the wait mode
			_nWaitState = WAIT_STATE_NONE;
			return TRUE;
		}
		break;

	case WAIT_STATE_PURCHASE_SLIM:
		// wait for the slim to finish processing the purchase
		if( !_pSlim->IsBusy() ) {
			// slim is done processing the purchase, tell shady
			_pShady->Purchase( _nUpsaleIndex );

			if( _pShady->IsBusy() ) {
				// we need to wait for shady
				_nWaitState = WAIT_STATE_PURCHASE_SHADY;
			} else {
				// shady is done, start a new sale
				return _Call_NewSale();
			}
		}
		break;

	case WAIT_STATE_PURCHASE_SHADY:
		if( !_pShady->IsBusy() ) {
			// shady is done, start a new sale
			return _Call_NewSale();
		}
		break;

	case WAIT_STATE_FAILED_SLIM:
		if( !_pSlim->IsBusy() ) {
			// slim is done processing the failed purchase, tell shady
			_pShady->FailedPurchase( _nUpsaleIndex );

			if( _pShady->IsBusy() ) {
				// we need to wait for shady
				_nWaitState = WAIT_STATE_FAILED_SHADY;
			} else {
				// shady is done, start a new sale
			//	return _Call_NewSale();
				_nWaitState = WAIT_STATE_NONE;
				return TRUE;
			}
		}
		break;

	case WAIT_STATE_FAILED_SHADY:
		if( !_pShady->IsBusy() ) {
			// shady is done, start a new sale
			//return _Call_NewSale();
			_nWaitState = WAIT_STATE_NONE;
			return TRUE;
		}
		break;

	case WAIT_STATE_REJECTED_SLIM:
		if( !_pSlim->IsBusy() ) {
			// slim is done processing the REJECTED PURCHASE, tell shady
			_pShady->RejectedItem( _nUpsaleIndex );

			if( _pShady->IsBusy() ) {
				// we need to wait for shady
				_nWaitState = WAIT_STATE_REJECTED_SHADY;
			} else {
				// shady is done, start a new sale
				return _Call_NewSale();
			}
		}
		break;

	case WAIT_STATE_REJECTED_SHADY:
		if( !_pShady->IsBusy() ) {
			// shady is done, start a new sale
			return _Call_NewSale();
		}
		break;

	case WAIT_STATE_NEW_SALE_SLIM:
		if( !_pSlim->IsBusy() ) {
			// slim is done processing the new sale command, tell shady
			_pShady->NewSale( _nUpsaleIndex, &_TableData );

			if( _pShady->IsBusy() ) {
				// wait for shady to finish
				_nWaitState = WAIT_STATE_NEW_SALE_SHADY;
			} else {
				// shady is done, no need to wait further
				_nWaitState = WAIT_STATE_NONE;
				return TRUE;
			}
		}
		break;

	case WAIT_STATE_NEW_SALE_SHADY:
		// see if shady is done processing the new sale
		if( !_pShady->IsBusy() ) {
			// shady is done, no need to wait further
			_nWaitState = WAIT_STATE_NONE;
			return TRUE;
		}
		break;

	case WAIT_STATE_EXIT_SLIM:
		if( !_pSlim->IsBusy() ) {
			// slim is done processing the exit command, no need to wait further
			_nWaitState = WAIT_STATE_NONE;
			return TRUE;
		}
		break;

	case WAIT_STATE_EXIT_SHADY:
		if( !_pShady->IsBusy() ) {
			// shady is done processing, tell slim to exit
			_pSlim->Exit( _nItemsPurchased );
			if( _pSlim->IsBusy() ) {
				// we need to wait for slim to finish processing the exit command
				_nWaitState = WAIT_STATE_EXIT_SLIM;
			} else {
				// slim is done processing the exit command, no need to wait further
				_nWaitState = WAIT_STATE_NONE;
				return TRUE;
			}
		}
		break;

	default:
		FASSERT_NOW;
		break;	
	}

	return FALSE;
}

static BOOL _InitBarterLevel( cchar *pszCSVFile ) {

	FASSERT( !_bLevelOK );
	
	FResFrame_t hResFrame = fres_GetFrame();

	/////////////////////////////////////////
	// load the sound bank with our sound fxs
	if( (FSndFx_BankHandle_t)fresload_Load( FSNDFX_RESTYPE, "Barter" ) == FSNDFX_INVALID_BANK_HANDLE ) {
		DEVPRINTF("bartersystem_InitLevel() : Could not load sound effect bank 'Barter'.\n" );
	}

	if( _bFirstBarterEncounter ) {
		if( (FSndFx_BankHandle_t)fresload_Load( FSNDFX_RESTYPE, "b_intro" ) == FSNDFX_INVALID_BANK_HANDLE ) {
			DEVPRINTF("bartersystem_InitLevel() : Could not load sound effect bank 'b_intro'.\n" );
		}
	}

	///////////////////////////////////////////
	// init the barter items from the csv file
	if( !CBarterItem::InitFromFile( BARTERITEMS_FILE_NAME ) ) {
		// already logged errors
		goto _ExitWithError;
	}

	/////////////////////
	// Allocate our Slim.
	_pSlim = fnew CSlim;
	if( _pSlim == NULL ) {
		DEVPRINTF("bartersystem_InitLevel() : Out of memory.\n");
		goto _ExitWithError;
	}
	
	//////////////////////
	// Allocate our Shady.
	_pShady = fnew CShady;
	if( _pShady == NULL ) {
		DEVPRINTF("bartersystem_InitLevel() : Out of memory.\n");
		goto _ExitWithError;
	}
	
	/////////////////////////////
	// Allocate our Barter Sound.
	_pBarterSound = fnew CBarterSound;
	if( _pBarterSound == NULL ) {
		DEVPRINTF("bartersystem_InitLevel() : Out of memory.\n");
		goto _ExitWithError;
	}

	//////////////////////////////
	// Allocate our box MeshEntity
	_pBoxMeshEntity = fnew CMeshEntity;
	if( !_pBoxMeshEntity ) {
		DEVPRINTF("bartersystem_InitLevel() : Out of memory.\n");
		goto _ExitWithError;
	}
	
	///////////////////////
	// Grab sound handles.
	_hScrollSound = fsndfx_GetFxHandle( _CLICK_FX_NAME );
	if( _hScrollSound == FSNDFX_INVALID_FX_HANDLE ) {
		DEVPRINTF("bartersystem_InitLevel() : Could not load '%s' sound effect.\n", _CLICK_FX_NAME );
	}
	_hSelectSound = fsndfx_GetFxHandle( _SELECT_FX_NAME );
	if( _hSelectSound == FSNDFX_INVALID_FX_HANDLE ) {
		DEVPRINTF("bartersystem_InitLevel() : Could not load '%s' sound effect.\n", _SELECT_FX_NAME );
	}

	/////////////////////////////////////////////////////////////////
	// Get the information about what these guys should be bartering.
	if( !_LoadBarterInfoFromFile( pszCSVFile ) ) {
		// there must not be any barter droid file listed in the level csv file
		goto _ExitWithError;
	}
	
	///////////////
	// Create Slim.
	if( !_pSlim->Create(BARTER_SYSTEM_SPEAK_RADIUS) ) {
		DEVPRINTF("bartersystem_InitLevel() : Failure on creation of Slim.\n");
		goto _ExitWithError;
	}
	_pSlim->SetCallback( _ActionBarter );

	////////////////
	// Create Shady.
	if( !_pShady->Create(BARTER_SYSTEM_SPEAK_RADIUS) ) {
		DEVPRINTF("bartersystem_InitLevel() : Failure on creation of Shady.\n");
		goto _ExitWithError;
	}

	//////////////////////////
	// Create the BarterSound.
	if( !_pBarterSound->Create() ) {
		DEVPRINTF("bartersystem_InitLevel() : Failure on creation of the BarterSound.\n");
		// couldn't load some sounds, but don't fail for that
	}

	////////////////////////
	// Create the box entity
	if( !_pBoxMeshEntity->Create( "GODHsoapbx", 0, NULL, "Mike's Barter Box" ) ) {
		DEVPRINTF("bartersystem_InitLevel() : Failure on creation of Shady's box.\n");
		goto _ExitWithError;		
	}

	_Call_RemoveFromWorld();
//	_pBoxMeshEntity->RemoveFromWorld();
		
	/////////////////////////////////////
	// Set the state and misc other vars.
	_fTimer = 0.0f;			
	_eState = BARTERSTATE_NOT_IN_WORLD;
	_nWaitState = WAIT_STATE_NONE;

	_vecCensorBarDLU.x = -0.105f;
	_vecCensorBarDLU.y = 0.055f;
	_vecCensorBarDLU.z = 0.00f;

	_vecCensorBarDRU.x = -_vecCensorBarDLU.x;
	_vecCensorBarDRU.y = _vecCensorBarDLU.y;
	_vecCensorBarDRU.z = 0.00f;

	// if we got there, we are ok
	return TRUE;

_ExitWithError:
	// there was a problem
	_bLevelOK = TRUE;// set this so that we can call bartersystem_UninitLevel()
	bartersystem_UninitLevel();
		
	fres_ReleaseFrame( hResFrame );

	DEVPRINTF( "bartersystem_InitLevel() : Could not init the barter system, shutting it down.\n" );
	return FALSE;	
}

BOOL bartersystem_SetAttractRadius(u32 uWhichTable, f32 fAttractMusicRadius, f32 fAttractSpeechRadius)
{
	if (uWhichTable >= _BarterLevel.GetNumTables())
	{
		DEVPRINTF("bartersystem_SetAttractRadius() error %d table (of %d) out of bounds", uWhichTable, _BarterLevel.GetNumTables());
		return FALSE;
	}
	CBarterTable* pTable = _BarterLevel.GetTable(uWhichTable);
	pTable->m_fMusicRadius = fAttractMusicRadius;
	pTable->m_fSpeechRadius = fAttractSpeechRadius;
	_pSlim->Set3dAudioRadius(pTable->m_fMusicRadius);
	_pShady->Set3dAudioRadius(pTable->m_fSpeechRadius);
	return TRUE;
}

BOOL bartersystem_IsBarterBot(const CEntity* pEntity)
{
	FASSERT(CPlayer::m_pCurrent->m_pEntityCurrent);
	ActionFunction_t* pActionFunct = pEntity->GetActionFunction();
	if (pActionFunct == _ActionBarter)
		if (_IsPlayerInFrontAndLookingAtBarters( _nBarterPtSelected, (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent ))
			return TRUE;
	return FALSE;
}

BOOL bartersystem_HasEmptyOfferNotice( const CEntity *pPlayer ) {
#if FANG_WINGC
	if( !MultiplayerMgr.IsLocalCoop() || !_bLevelOK || _nBarterPtSelected < 0 ) return FALSE;
	for( s32 n=0; n<CPlayer::m_nPlayerCount; ++n ) {
		if( Player_aPlayer[n].m_pEntityCurrent == pPlayer ) {
			return _abCoopEmptyOffer[n] && _anCoopEmptyOfferPoint[n] == _nBarterPtSelected &&
				(u32)(GetTickCount() - _anCoopEmptyOfferTicks[n]) < 2500;
		}
	}
#endif
	return FALSE;
}
