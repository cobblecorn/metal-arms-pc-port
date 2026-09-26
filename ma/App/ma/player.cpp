//////////////////////////////////////////////////////////////////////////////////////
// player.cpp - Human player module.
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
// 03/29/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "player.h"
#include "bot.h"
#include "fviewport.h"
#include "fscriptsystem.h"
#include "gamepad.h"
#include "ItemInst.h"
#include "entity.h"
#include "frenderer.h"
#include "fdraw.h"
#include "skybox.h"
#include "level.h"
#include "hud2.h"
#include "pausescreen.h"
#include "AI\AIBrain.h"
#include "AI\AIBrainman.h"
#include "AI\AiApi.h"
#include "gamesave.h"
#include "gamecam.h"
#include "game.h"
#include "vehicle.h"
#include "MultiplayerMgr.h"

#if FANG_PLATFORM_WIN && !FANG_PRODUCTION_BUILD
	#include "gameloop.h"
	#include "botglitch.h"
//#elif !FANG_PRODUCTION_BUILD
//	#include "gameloop.h"	// For turning off debug text when debugging multiplayer
#endif


#define _RESTART_TBOX_WIDTH					0.9f
#define _RESTART_TBOX_HEIGHT				0.030f

#define PLAYER_HIGH_TARGET_ASSIST_SETTING	1.0f

#define _SENSITIVITY_MULT_MIN				0.25f
#define _SENSITIVITY_MULT_MAX				4.0f


CPlayer Player_aPlayer[MAX_PLAYERS];
s32 CPlayer::m_nPlayerCount;
s32 CPlayer::m_nCurrent;
CPlayer *CPlayer::m_pCurrent;
BOOL CPlayer::m_bSystemInitialized;
CInventory *CPlayer::m_pInvSave=NULL;
f32 CPlayer::m_fStartOfLevelMusicVol;
f32 CPlayer::m_fStartOfLevelSfxVol;
static s32 m_nPlayerKillEntityEvent = -1;

f32 CPlayer::m_fMusicVolumeCache; // -1 not cached, otherwise volume that should be saved to profile instead of asking the audio system (cause we probably faded it out)
f32 CPlayer::m_fSfxVolumeCache;	// -1 not cached, otherwise volume that should be saved to profile instead of asking the audio system (cause we probably faded it out)



#if FANG_PLATFORM_WIN && !FANG_PRODUCTION_BUILD
	static u32 _nLastFrameDesiredLegServoLevel;
	static u32 _nLastFrameDesiredArmServoLevel;
#endif

void Player_GameStats_t::InitLevel( BOOL bSinglePlayer )
{
	m_bIsSinglePlayer = bSinglePlayer;
	if (bSinglePlayer) {
		m_SPStats.m_nWashersCollected = 0;
		m_SPStats.m_nUnitsDestroyed = 0;
		m_SPStats.m_nSecretAreasUncovered = 0;
		m_SPStats.m_nSecretChipsCollected = 0;
	}
	else {
		m_MPStats.m_nKills = 0;
		m_MPStats.m_nDeaths = 0;
	}
	m_nPlayerKillEntityEvent = -1;
}

BOOL CPlayer::InitSystem( void ) {
	FTextArea_t TextArea, *pTextArea;
	CPlayer *pPlayer;
	u32 i, j;

	FASSERT( !IsSystemInitialized() );

	ftext_SetToDefaults( &TextArea );
	TextArea.bVisible = FALSE;
	TextArea.fUpperLeftX = 0.0f;
	TextArea.fLowerRightX = 0.5;
	TextArea.fUpperLeftY = 0.0f;
	TextArea.fLowerRightY = 0.5f;
	TextArea.fNumberOfLines = 2;
	TextArea.fBorderThicknessX = 0.0f;
	TextArea.fBorderThicknessY = 0.0f;
	TextArea.oColorForeground.OpaqueWhite();
	TextArea.oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.5f );
	TextArea.oColorBorder.Set( 0.1f, 0.1f, 1.0f, 1.0f );
	TextArea.ohFont = '1';
	TextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	for( i=0, pPlayer=Player_aPlayer; i<MAX_PLAYERS; i++, pPlayer++ ) {
		pPlayer->m_nPlayerIndex = i;
		pPlayer->m_nPlayerNum = i + 1;
		pPlayer->m_nControllerIndex = i;

		pPlayer->m_pEntityOrig = NULL;
		pPlayer->m_pEntityCurrent = NULL;
		pPlayer->m_pViewportPersp3D = NULL;
		pPlayer->m_pViewportOrtho3D = NULL;
		pPlayer->m_pViewportSafeOrtho3D = NULL;
		pPlayer->m_uPlayerFlags = PF_HAS_ENTITY_CONTROL;

		// Set up possession out-of-range text box...
		pPlayer->m_hTextBoxOutOfRange = ftext_Create( &TextArea );
		pPlayer->m_hTextBoxScopeZoom = ftext_Create( &TextArea );
		pPlayer->m_hTextBoxLowNoAmmo = ftext_Create( &TextArea );
		pPlayer->m_hTextBoxRestart = ftext_Create( &TextArea );

		pPlayer->m_bInvertLook = TRUE;
		pPlayer->m_bAutoCenter = FALSE;
		pPlayer->m_bFourWayQuickSelect = FALSE;
		pPlayer->m_fUnitTargetingAssistance = PLAYER_HIGH_TARGET_ASSIST_SETTING;
		pPlayer->m_bHidePlayerEntity = FALSE;
		pPlayer->m_fLookSensitivity = 0.5f;

		for( j=0; j<RETICLE_SCOPE_INFO_SLOT_COUNT; ++j ) {
			pPlayer->m_ahTextBoxScopeInfo[j] = ftext_Create( &TextArea );
		}

		pPlayer->m_GameStats.m_SPStats.m_nWashersCollected = 0;
		pPlayer->m_GameStats.m_SPStats.m_nUnitsDestroyed = 0;
		pPlayer->m_GameStats.m_SPStats.m_nSecretAreasUncovered = 0;
		pPlayer->m_GameStats.m_SPStats.m_nSecretChipsCollected = 0;

		pPlayer->ResetTargetAssistance();

		pPlayer->m_nInDebugCameraResetDelay = 0;
		pPlayer->m_bRestoreTimerSuspended = FALSE;
		pPlayer->m_bRestoreTimerRunning = FALSE;
		pPlayer->m_fRestoreCheckpointTimer = 0.f;
		pPlayer->m_vPosWhenLeavingGround.Set( 0.f, 0.f, 0.f );
		
		pPlayer->m_bForceReady = FALSE;
	}

	// Init the "Press A to Continue" text box...
	for( i=0, pPlayer=Player_aPlayer; i<MAX_PLAYERS; i++, pPlayer++ ) {
		pTextArea = ftext_GetAttributes( Player_aPlayer[ i ].m_hTextBoxRestart );

		pTextArea->fUpperLeftX = 0.5f - 0.5f*_RESTART_TBOX_WIDTH;
		pTextArea->fLowerRightX = 0.5f + 0.5f*_RESTART_TBOX_WIDTH;
		pTextArea->fUpperLeftY = 0.645f - 0.5f*_RESTART_TBOX_HEIGHT;
		pTextArea->fLowerRightY = 0.645f + 0.5f*_RESTART_TBOX_HEIGHT;
		pTextArea->oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
		pTextArea->fNumberOfLines = 1;
		pTextArea->oColorForeground.Set( 0.25f, 0.25f, 1.0f, 1.0f );
		pTextArea->oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.0f );
		pTextArea->oColorBorder.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	}

	m_nPlayerCount = 0;

	m_nCurrent = -1;
	m_pCurrent = NULL;

	m_pInvSave = NULL;

	fvid_SetDrawOverlayFcn( _FVidDrawOverlay );

	#if FANG_PLATFORM_WIN && !FANG_PRODUCTION_BUILD
		_nLastFrameDesiredLegServoLevel = 0;
		_nLastFrameDesiredArmServoLevel = 0;
	#endif

	m_fStartOfLevelMusicVol = 1.0f;
	m_fStartOfLevelSfxVol = 1.0f;
	
	m_fMusicVolumeCache = -1.0f;
	m_fSfxVolumeCache = -1.0f;

	m_bSystemInitialized = TRUE;

	return TRUE;
}

void CPlayer::UninitSystem( void ) {
	u32 i;

	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;

		for( i=0; i<MAX_PLAYERS; i++ ) {
			Player_aPlayer[i].m_pViewportPersp3D = NULL;
			Player_aPlayer[i].m_pViewportOrtho3D = NULL;
			Player_aPlayer[i].m_pViewportSafeOrtho3D = NULL;

			Player_aPlayer[i].m_Reticle.Destroy();
		}
	}

	fvid_SetDrawOverlayFcn( NULL );
}


BOOL CPlayer::InitLevel( const GameInitInfo_t *pGameInit, Level_e nLevel ) {
	s32 i, j, nIndex;
	CPlayerProfile *pProfile;
	CPlayer *pPlayer;
	u32 nPadMap;
	BOOL bSinglePlayerGame;
	const GameSave_MPRules_t* pMPRules = NULL;
	
	FASSERT( IsSystemInitialized() );

	if ( pGameInit ) {
		m_nPlayerCount = pGameInit->nNumPlayers;
		bSinglePlayerGame = pGameInit->bSinglePlayer;
		if( !bSinglePlayerGame ) {
			pMPRules = pGameInit->pMultiplayerRules;
		}
	} else {
		m_nPlayerCount = 1;
		bSinglePlayerGame = TRUE;
	}

	FASSERT( (m_nPlayerCount>0) && (m_nPlayerCount<=MAX_PLAYERS) );

	SetCurrent( -1 );

	for( i=0; i < m_nPlayerCount; ++i ) {
		pPlayer = &Player_aPlayer[i];

		pPlayer->m_uPlayerFlags = PF_HAS_ENTITY_CONTROL;
		pPlayer->m_pFcnDrawOverlay = NULL;
		
		pPlayer->m_bForceReady = FALSE;
		
		pPlayer->StopViewFade();

		pPlayer->m_SkyBox.CreateFromCSV( Level_hLevelDataFile );

		// get pointer to profile object (if any) from shell
		pProfile = playerprofile_Get( i );
		if( pProfile ) {
			// there is a profile, use it
			pPlayer->m_pPlayerProfile = pProfile;
		
			if( bSinglePlayerGame && nLevel >= 0 && nLevel < LEVEL_SINGLE_PLAYER_COUNT ) {
				// copy the saved inventory to the player struct
				if( !CMemCardInventory::RestoreInventory( &pProfile->m_Data.aLevelProgress[nLevel].Inventory, pPlayer->GetInventory( i ) ) ) {
					DEVPRINTF( "Player::InitLevel() : Failed while restore the player's inventory, setting to defaults.\n" );
					pPlayer->m_BotInv.SetToDefaults();
				}
			} else {
				// If this is a multiplayer game we need to handle our weapon restrictions.
				if( pMPRules ) {
					pPlayer->m_BotInv.InitForMultiplayer( Level_hInventoryTable, pMPRules->nLimitPrimaryWeapon, pMPRules->nLimitSecondaryWeapon );
				} else if( !pPlayer->m_BotInv.InitFromCSVTable( Level_hInventoryTable ) ) {
					// there isn't an inventory in the profile, maybe the loaded level csv has one
					pPlayer->m_BotInv.SetToDefaults();
				}
			}	
			pPlayer->m_PosInv.SetToDefaults();
			
			pPlayer->m_nControllerIndex = pProfile->m_nControllerIndex;
			pPlayer->m_bInvertLook = ( pProfile->m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG );
			pPlayer->m_bAutoCenter = ( pProfile->m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_AUTO_CENTER );
			pPlayer->m_bFourWayQuickSelect = ( pProfile->m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT );
			if ( pProfile->m_Data.nFlags & GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING ) {
				pPlayer->m_fUnitTargetingAssistance = PLAYER_HIGH_TARGET_ASSIST_SETTING;
			} else {
				pPlayer->m_fUnitTargetingAssistance = 0.0f;
			}
			pPlayer->m_fLookSensitivity = pProfile->m_Data.fUnitLookSensitivity;
			fforce_SetMasterIntensity( pPlayer->m_nControllerIndex, pProfile->m_Data.fUnitVibrationIntensity );

			if( i == 0 ) {
				m_fStartOfLevelMusicVol = faudio_SetMusicMasterVol( pProfile->m_Data.fUnitMusicLevel );
				m_fStartOfLevelSfxVol = faudio_SetSfxMasterVol( pProfile->m_Data.fUnitSoundLevel );			
			}

			nPadMap = pProfile->m_Data.nControllerConfigIndex;
			FMATH_CLAMP( nPadMap, 0, (u32)GAMEPAD_MAP_MAIN1 );

			// Get the team number for multiplayer
			if( pMPRules ) {
				pPlayer->m_nTeamNum = 1 - ((pGameInit->nTeamAHumanMask >> i) & 0x01);
			} else {
				pPlayer->m_nTeamNum = 0;
			}
		} else {
			// there was no profile, set the defaults
			pPlayer->m_pPlayerProfile = NULL;
			pPlayer->m_nControllerIndex = i;
			pPlayer->m_nTeamNum = 0;
			pPlayer->m_bInvertLook = TRUE;
			pPlayer->m_bAutoCenter = FALSE;
			pPlayer->m_bFourWayQuickSelect = FALSE;
			pPlayer->m_fUnitTargetingAssistance = PLAYER_HIGH_TARGET_ASSIST_SETTING;
			pPlayer->m_fLookSensitivity = 0.5f;
	
			// maybe the loaded level csv has an inventory
			if( !pPlayer->m_BotInv.InitFromCSVTable( Level_hInventoryTable ) ) {
				pPlayer->m_BotInv.SetToDefaults();
			}
			pPlayer->m_PosInv.SetToUnarmed(TRUE);

			nPadMap = GAMEPAD_MAP_MAIN1;

			if( i == 0 ) {
				m_fStartOfLevelMusicVol = faudio_SetMusicMasterVol( 1.0f );
				m_fStartOfLevelSfxVol = faudio_SetSfxMasterVol( 1.0f );			
			}
		}

		// set the desired gamepad mapping
		gamepad_SetMapping( pPlayer->m_nControllerIndex, (GamepadMap_e)nPadMap );

		// assign a unique name to this player
		if( pPlayer->m_pPlayerProfile && !pPlayer->m_pPlayerProfile->IsVirtual()  ) {
			pPlayer->m_pwszPlayerName = pPlayer->m_pPlayerProfile->m_SaveInfo.wszProfileName;
			if( fclib_wcslen( pPlayer->m_pwszPlayerName ) < 1 ) {
				// assign a random name to this player, they don't have a valid name
				nIndex = fmath_RandomRange( GAMEPHRASE_MULTIPLAYER_NAME_1, GAMEPHRASE_MULTIPLAYER_NAME_10 );
				pPlayer->m_pwszPlayerName = Game_apwszPhrases[nIndex];
			}
		} else {
			nIndex = fmath_RandomRange( GAMEPHRASE_MULTIPLAYER_NAME_1, GAMEPHRASE_MULTIPLAYER_NAME_10 );
			pPlayer->m_pwszPlayerName = Game_apwszPhrases[nIndex];
		}
		// now make sure that there are no duplicates
		j = 0;
		while( j < i ) {
			if( fclib_wcsicmp( Player_aPlayer[j].m_pwszPlayerName, pPlayer->m_pwszPlayerName ) == 0 ) {
				// the name is not unique, pick a new one
				nIndex = fmath_RandomRange( GAMEPHRASE_MULTIPLAYER_NAME_1, GAMEPHRASE_MULTIPLAYER_NAME_10 );
				pPlayer->m_pwszPlayerName = Game_apwszPhrases[nIndex];
				// start over
				j = 0;
			} else {
				j++;
			}
		}
		
		// If this is a multiplayer game, change the text attributes of the
		// "Press A to Continue" message
		FTextArea_t* pTextArea = ftext_GetAttributes( pPlayer->m_hTextBoxRestart );
		if ( !bSinglePlayerGame ) {
			pTextArea->bNoScale = TRUE;
			pTextArea->ohFont = '9';
		} else {
			pTextArea->bNoScale = FALSE;
			pTextArea->ohFont = '1';
		}

		pPlayer->m_bHidePlayerEntity = FALSE;
		pPlayer->m_GameStats.InitLevel( bSinglePlayerGame );

		pPlayer->m_pRacingVehicle = NULL;

		pPlayer->m_nInDebugCameraResetDelay = 0;
		pPlayer->m_bRestoreTimerSuspended = FALSE;
		pPlayer->m_bRestoreTimerRunning = FALSE;
		pPlayer->m_fRestoreCheckpointTimer = 0.f;
		pPlayer->m_vPosWhenLeavingGround.Set( 0.f, 0.f, 0.f );

		pPlayer->ResetTargetAssistance();
	}

	#if FANG_PLATFORM_WIN && !FANG_PRODUCTION_BUILD
		_nLastFrameDesiredLegServoLevel = 0;
		_nLastFrameDesiredArmServoLevel = 0;
	#endif

	return TRUE;
}


void CPlayer::UninitLevel( Level_e nLevel, BOOL bLevelCompleted ) {
	s32 i;
	CPlayerProfile *pProfile;
	BOOL bUpdateProfileStats, bReplaying;
	CPlayer *pPlayer;
	
	GameTypes_e nGameType = game_GetCurrentType();
	bReplaying = (nGameType == GAME_TYPES_REPLAY_LEVEL);
	if( nGameType == GAME_TYPES_ADVENTURE || bReplaying ) {
		if( (nLevel >= 0) && (nLevel < LEVEL_SINGLE_PLAYER_COUNT ) && (m_nPlayerCount == 1) ) {
			bUpdateProfileStats = bLevelCompleted;
		} else {
			bUpdateProfileStats = FALSE;
		}
	} else {
		bUpdateProfileStats = FALSE;
	}

	for( i=0; i < m_nPlayerCount; ++i ) {
		pPlayer = (CPlayer *)&Player_aPlayer[i];

		pPlayer->m_SkyBox.Destroy();
		pPlayer->StopViewFade();
		
		pPlayer->m_bForceReady = FALSE;

		// copy profiled items to profile object before returning to shell
		pProfile = pPlayer->m_pPlayerProfile;
		if( pProfile ) {
			if( bUpdateProfileStats ) {
				if( !bReplaying ) {
					if( nLevel < (LEVEL_SINGLE_PLAYER_COUNT-1) ) {
						// not the last level in the single player game
						// save our current inventory so we can start with it in the next level
						if ( m_pInvSave ) {
							CMemCardInventory::SaveInventory( &pProfile->m_Data.aLevelProgress[nLevel+1].Inventory, m_pInvSave );
							m_pInvSave = NULL;
						} else {
							CMemCardInventory::SaveInventory( &pProfile->m_Data.aLevelProgress[nLevel+1].Inventory, pPlayer->GetInventory(i) );
						}

					} else {
						// the last level in the single player game, don't save the inventory
						pProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER;					
					}
					// record our stats
					pProfile->m_Data.aLevelProgress[nLevel].fTimeToComplete = Level_fSecsInLevel;
					pProfile->m_Data.aLevelProgress[nLevel].nWashersCollected = pPlayer->m_GameStats.m_SPStats.m_nWashersCollected;
					pProfile->m_Data.aLevelProgress[nLevel].nSecretChipsCollected = pPlayer->m_GameStats.m_SPStats.m_nSecretChipsCollected;
					pProfile->m_Data.aLevelProgress[nLevel].nEnemiesKilled = pPlayer->m_GameStats.m_SPStats.m_nUnitsDestroyed;
					
					// advance to the next level
					pProfile->m_Data.nCurrentLevel++;
					if( pProfile->m_Data.nCurrentLevel == LEVEL_SINGLE_PLAYER_COUNT ) {
						pProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER;
					}

					if( !pProfile->IsVirtual() ) { 
						// flag to save the profile
						pProfile->m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
					}
				} else {
					// update our level stats
					if( Level_fSecsInLevel < pProfile->m_Data.aLevelProgress[nLevel].fTimeToComplete ) {
						pProfile->m_Data.aLevelProgress[nLevel].fTimeToComplete = Level_fSecsInLevel;
					}
					if( pPlayer->m_GameStats.m_SPStats.m_nWashersCollected > pProfile->m_Data.aLevelProgress[nLevel].nWashersCollected ) {
						pProfile->m_Data.aLevelProgress[nLevel].nWashersCollected = pPlayer->m_GameStats.m_SPStats.m_nWashersCollected;
					}
					if( pPlayer->m_GameStats.m_SPStats.m_nSecretChipsCollected > pProfile->m_Data.aLevelProgress[nLevel].nSecretChipsCollected ) {
						pProfile->m_Data.aLevelProgress[nLevel].nSecretChipsCollected = pPlayer->m_GameStats.m_SPStats.m_nSecretChipsCollected;
					}
					if( pPlayer->m_GameStats.m_SPStats.m_nUnitsDestroyed > pProfile->m_Data.aLevelProgress[nLevel].nEnemiesKilled ) {
						pProfile->m_Data.aLevelProgress[nLevel].nEnemiesKilled = pPlayer->m_GameStats.m_SPStats.m_nUnitsDestroyed;
					}

					if( !pProfile->IsVirtual() ) { 
						// flag to save the profile
						pProfile->m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
					}
				}
			}

			// update profile user settings
			pPlayer->UpdateProfileUserSettings();		
		}
	}

	// restore the master volume
	faudio_SetMusicMasterVol( m_fStartOfLevelMusicVol );
	faudio_SetSfxMasterVol( m_fStartOfLevelSfxVol );	

	// at this point, there are no more valid players, set the count to 0
	m_nPlayerCount = 0;
}

void CPlayer::UpdateProfileUserSettings( void ) {
	
	if( !m_pPlayerProfile ) {
		return;
	}

	// cache a few vars so that we can see if anything changed
	u32 nFlags = m_pPlayerProfile->m_Data.nFlags;
	f32 fLookSensitivity = m_pPlayerProfile->m_Data.fUnitLookSensitivity;
	f32 fMusicVol = m_pPlayerProfile->m_Data.fUnitMusicLevel;
	f32 fSoundVol = m_pPlayerProfile->m_Data.fUnitSoundLevel;
	f32 fVibrations = m_pPlayerProfile->m_Data.fUnitVibrationIntensity;
    
	if( m_bInvertLook ) {
        m_pPlayerProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG;
	} else {
		m_pPlayerProfile->m_Data.nFlags &= ~GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG;
	}
	if( m_bAutoCenter ) {
		m_pPlayerProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_AUTO_CENTER;
	} else {
		m_pPlayerProfile->m_Data.nFlags &= ~GAMESAVE_PROFILE_FLAGS_AUTO_CENTER;
	}
	if( m_bFourWayQuickSelect ) {
		m_pPlayerProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT;
	} else {
		m_pPlayerProfile->m_Data.nFlags &= ~GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT;
	}
	if( m_fUnitTargetingAssistance == 0.0f ) {
		m_pPlayerProfile->m_Data.nFlags &= ~GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING;
	} else {
		m_pPlayerProfile->m_Data.nFlags |= GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING;
	}
	m_pPlayerProfile->m_Data.fUnitLookSensitivity = m_fLookSensitivity;

	m_pPlayerProfile->m_Data.fUnitVibrationIntensity = fforce_GetMasterIntensity( m_nControllerIndex );

	m_pPlayerProfile->m_Data.fUnitMusicLevel = (m_fMusicVolumeCache < 0.0f) ? faudio_GetMusicMasterVol() : m_fMusicVolumeCache;
	m_pPlayerProfile->m_Data.fUnitSoundLevel = (m_fSfxVolumeCache < 0.0f) ? faudio_GetSfxMasterVol() : m_fSfxVolumeCache;
	
	if( nFlags != m_pPlayerProfile->m_Data.nFlags ||
		fLookSensitivity != m_pPlayerProfile->m_Data.fUnitLookSensitivity ||
        fMusicVol != m_pPlayerProfile->m_Data.fUnitMusicLevel ||
        fSoundVol != m_pPlayerProfile->m_Data.fUnitSoundLevel ||
		fVibrations != m_pPlayerProfile->m_Data.fUnitVibrationIntensity ) {
		// flag to save the profile
		if( !m_pPlayerProfile->IsVirtual() ) { 
			m_pPlayerProfile->m_SaveInfo.nFlags |= GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING;
		}
	}		
}

void CPlayer::CacheAudioLevels( void ) {
	m_fMusicVolumeCache = faudio_GetMusicMasterVol();
	m_fSfxVolumeCache = faudio_GetSfxMasterVol();
}

void CPlayer::SetCurrent( s32 nCurrentPlayerIndex ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( nCurrentPlayerIndex==-1 || (nCurrentPlayerIndex>=0 && nCurrentPlayerIndex<m_nPlayerCount) );

	m_nCurrent = nCurrentPlayerIndex;
	m_pCurrent = (nCurrentPlayerIndex != -1) ? &Player_aPlayer[nCurrentPlayerIndex] : NULL;
}


CInventory *CPlayer::GetInventory( s32 nPlayerIndex, BOOL bPossession/*=FALSE*/ ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( nPlayerIndex >= -1 && nPlayerIndex < m_nPlayerCount );
	
	if( nPlayerIndex == -1 ) {
		return NULL;
	}

	if( !bPossession ) {
		return &Player_aPlayer[nPlayerIndex].m_BotInv;
	} else {
		return &Player_aPlayer[nPlayerIndex].m_PosInv;
	}
}


void CPlayer::DisableEntityControl( void ) {
	if( m_uPlayerFlags & PF_HAS_ENTITY_CONTROL ) {
		m_uPlayerFlags &= ~PF_HAS_ENTITY_CONTROL;

		if( m_pEntityCurrent ) {
			if( m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT ) {
				CBot *pBot;
				pBot = (CBot*) m_pEntityCurrent;
				if( pBot->m_pDrivingVehicle && pBot->m_pDrivingVehicle->GetDriverBot() ) {
					pBot->m_pDrivingVehicle->SetControls( NULL ); // if player is currently driving a vehicle, then the vehicle will already have the controls
				} else {
					m_pEntityCurrent->SetControls( NULL );
				}

			} else {
				m_pEntityCurrent->SetControls( NULL );
			}
		}
	}
}


void CPlayer::EnableEntityControl( void ) {
	m_uPlayerFlags |= PF_HAS_ENTITY_CONTROL;
	if( m_pEntityCurrent ) {
		if( m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT ) {
			CBot *pBot;
			pBot = (CBot*) m_pEntityCurrent;
			if( pBot->m_pDrivingVehicle && pBot->m_pDrivingVehicle->GetDriverBot() ) {
				pBot->m_pDrivingVehicle->SetControls( &m_HumanControl ); // if player is currently driving a vehicle, then the vehicle will already have the controls
			} else {
				m_pEntityCurrent->SetControls( &m_HumanControl );
			}

		} else {
			m_pEntityCurrent->SetControls( &m_HumanControl );
		}
	}
}

void CPlayer::ZeroControls( void )
{
	FPad_Sample_t* pSample = NULL;
	for (u32 i = 0; i < GAMEPAD_MAX_ACTIONS; i++){
		pSample	= Gamepad_aapSample[m_nControllerIndex][i];
		if (pSample)
			pSample->fCurrentState = 0.0f;
	}

	ResetTargetAssistance();
}

void CPlayer::AbortAllPlayerScopeZoomMode( BOOL bImmediate ) {
	CPlayer *pPlayer;
	u32 i;

	for( i=0, pPlayer=Player_aPlayer; i<(u32)m_nPlayerCount; ++i, ++pPlayer ) {
		if( pPlayer->m_pEntityCurrent ) {
			if( pPlayer->m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT ) {
				((CBot *)pPlayer->m_pEntityCurrent)->AbortScopeMode( bImmediate );
			}
		}
	}
}

// If this player is currently possessing someone, yank them out. Also, 
// yank out of zoom mode.
void CPlayer::ReturnToOriginalBot(void)
{
	if (m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) {
		// End possession
		if (IsPossessingMil()) {
			((CBot*) m_pEntityCurrent)->ForceQuickDataPortUnPlug();
		}

		// Make sure the scope is not active
		((CBot*) m_pEntityCurrent)->AbortScopeMode( TRUE );
	}

	ResetTargetAssistance();
}

void CPlayer::ResetTargetAssistance( void ) {
	// Setup default targeting assist data
	m_bReticleCenterOverTarget = FALSE;

	// Setup default biasing data
	m_pBiasingMesh = NULL;
	m_vBiasMeshPosLastFrame.Set( 0.f, 0.f, 0.f );
	fang_MemZero( m_afFrameYawVel, sizeof( f32 ) * PLAYER_AIM_BIASING_MESH_SAMPLES );
	fang_MemZero( m_afFramePitchVel, sizeof( f32 ) * PLAYER_AIM_BIASING_MESH_SAMPLES );
	m_nCurrentSample = 0;
	m_fYawVelAvg = 0.f;
	m_fPitchVelAvg = 0.f;
	m_fYawAdjust = 0.f;
	m_fPitchAdjust = 0.f;
}

// Call just before rendering this player's main view
void CPlayer::PreRender(void)
{
	if (m_bHidePlayerEntity)
		m_pEntityCurrent->DrawEnable( FALSE );

	// Get the player bot and notify him
	FASSERT(m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT);
	CBot* pPlayerBot = (CBot*)m_pEntityCurrent;
	pPlayerBot->PreUserRender();

	// If the player is operating a mech (vehicle, sentry gun, etc.) give it a chance
	// to change anything specific to this player's viewport
	CBot* pMechBot = pPlayerBot->GetCurMech();
	if (pMechBot) {
		pMechBot->PreUserRender();
	}
}

// Call just after rendering this player's main view
void CPlayer::PostRender(void)
{
	if (m_bHidePlayerEntity)
		m_pEntityCurrent->DrawEnable( TRUE );

	// Get the player bot and notify him
	FASSERT(m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT);
	CBot* pPlayerBot = (CBot*)m_pEntityCurrent;
	pPlayerBot->PostUserRender();

	// If the player is operating a mech (vehicle, sentry gun, etc.) give it a chance
	// to change anything specific to this player's viewport
	CBot* pMechBot = pPlayerBot->GetCurMech();
	if (pMechBot) {
		pMechBot->PostUserRender();
	}
}

// wrapper for logic to detect if "Press A to Continue" should be displayed
BOOL CPlayer::IsReadyToContinue( void ) {
	CBot *pBot = NULL;

	if( m_pEntityCurrent && (m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) ) {
		pBot = (CBot*) m_pEntityCurrent;
	}

	if( !m_pEntityCurrent->IsInWorld() || m_bForceReady || (pBot && pBot->IsBuried()) ||
		(pBot && pBot->m_pDrivingVehicle && pBot->m_pDrivingVehicle->IsBuried()) ) {
		return TRUE;
	}

	return FALSE;
}

void CPlayer::DrawText( void ) {
	// If this player is dead and waiting to resurrect, draw "Press A to Continue"
	if( IsReadyToContinue() ) {
		f32 fUnitVal = fmath_PositiveSin( FMATH_2PI * CHud2::GetAmmoAlertUnitCountdownTimer() ) * 0.6f + 0.199f;

		FTextArea_t *pTextArea = ftext_GetAttributes( m_hTextBoxRestart );
		pTextArea->oColorForeground.Set( fUnitVal, fUnitVal, 0.8f, 1.0f );

		ftext_PrintString( m_hTextBoxRestart, Game_apwszPhrases[GAMEPHRASE_PRESS_A_TO_CONTINUE] );
	}
}

void CPlayer::Work( void ) {

	m_SkyBox.Work();

	if (m_nPlayerKillEntityEvent == -1)
	{
		m_nPlayerKillEntityEvent = CFScriptSystem::GetEventNumFromName( "PlayerKills" );
		if (m_nPlayerKillEntityEvent == -1)
		{
			m_nPlayerKillEntityEvent = -2;
		}
	}

	if( HasEntityControl() ) {
		// ME:  I changed this to ignore controls instead of DeadOrDying() because glitch needs to receive input while dying
		if (!(m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) || (!((CBot*)m_pEntityCurrent)->IgnoreControls() && !MultiplayerMgr.IgnoreControls(m_nPlayerIndex)))
		{
			m_HumanControl.m_fForward = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_FORWARD_BACKWARD]->fCurrentState;
			m_HumanControl.m_fStrafeRight = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_STRAFE_LEFT_RIGHT]->fCurrentState;

			m_HumanControl.m_fAimDown = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_LOOK_UP_DOWN]->fCurrentState;
			m_HumanControl.m_fRotateCW = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_ROTATE_LEFT_RIGHT]->fCurrentState;

			if ( m_Hud.IsWSActive() ) {
				m_HumanControl.m_fFire1 = 0.0f;
				m_HumanControl.m_fFire2 = 0.0f;
			}
			else {
				m_HumanControl.m_fFire1 = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_FIRE_PRIMARY]->fCurrentState;
				m_HumanControl.m_fFire2 = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_FIRE_SECONDARY]->fCurrentState;
			}

			m_HumanControl.m_fCrossUp = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_SELECT_SECONDARY]->fCurrentState;
			m_HumanControl.m_fCrossDown = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_JUMP]->fCurrentState;

			m_HumanControl.m_nPadFlagsAction = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_ACTION]->uLatches;
			m_HumanControl.m_nPadFlagsJump = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches;
			m_HumanControl.m_nPadFlagsMelee = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_MELEE]->uLatches;
			m_HumanControl.m_nPadFlagsMelee |=  Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_MELEE2]->uLatches;
			m_HumanControl.m_nPadFlagsPause = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches;
			m_HumanControl.m_nPadFlagsSelect1 = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches;
			m_HumanControl.m_nPadFlagsSelect2 = Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_SELECT_SECONDARY]->uLatches;
		}
		else
		{
			m_HumanControl.Zero();
		}

		if( (!m_pEntityCurrent->IsInWorld() ||
			  ((m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) && ((CBot*)m_pEntityCurrent)->IsDead())) &&
			m_pEntityCurrent != m_pEntityOrig) {
			//send player bot back to original entity
			if (m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT)
			{
				if( ((CBot*) m_pEntityCurrent)->IsPossessionExitable() ) {
					((CBot*) m_pEntityCurrent)->ForceQuickDataPortUnPlug();
				}
			}
		}

		if( !MultiplayerMgr.HasQuit(m_nPlayerIndex) && IsReadyToContinue() ) {
			m_Hud.TransmissionMsg_Stop( FALSE );

			if( Gamepad_aapSample[m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches & FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT ) {
				//bring player bot back to life
				if (m_pEntityCurrent != m_pEntityOrig &&
					(m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) && 
					((CBot*) m_pEntityCurrent)->IsPossessionExitable() ) {
					//send player bot back to original entity
					((CBot*) m_pEntityCurrent)->ForceQuickDataPortUnPlug();
					FASSERT(m_pEntityCurrent == m_pEntityOrig);//EntityCurrent is nowEntityOrig
				} else {
					// Prepare to respawn
					Resurrect();

					if( m_pEntityCurrent->AIBrain() ) {
						aibrainman_ConfigurePlayerBotBrain(m_pEntityCurrent->AIBrain(), m_nPlayerIndex );
					}

					m_pEntityOrig->EnableAutoWork( FALSE );
					m_pEntityOrig->SetControls( &m_HumanControl );

					// If this is a multiplayer game, we just respawn this player
					// at a new spawn point. If it is single player, we restore
					// the last checkpoint.
					if( !MultiplayerMgr.RespawnBot( (CBot*)m_pEntityCurrent ) && !gamecam_GetCameraBeingDebugged() ) {
						if( checkpoint_Saved( 1 ) ) {
							// player has passed a checkpoint on this level,
							// so restore to that.
							checkpoint_Restore( 1, TRUE );
						} else if( checkpoint_Saved( 0 ) ) {
							// otherwise restore to beginning-of-level checkpoint
							checkpoint_Restore( 0, TRUE );
						}
					}
				}
			}
		}

		FASSERT( m_pEntityOrig || m_pEntityCurrent );

		if( m_pEntityCurrent) {

			#if FANG_ENABLE_DEV_FEATURES && FANG_PRODUCTION_BUILD
				if ( (m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) && ((CBot *)m_pEntityCurrent)->m_pWorldMesh )
				{
					ftext_DebugPrintf( 0.553f, 0.053f, "~ac~w1~C00000099%5.1f, %5.1f, %5.1f", ((CBot *)m_pEntityCurrent)->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.x, 
						((CBot *)m_pEntityCurrent)->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.y, ((CBot *)m_pEntityCurrent)->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.z );
					ftext_DebugPrintf( 0.55f, 0.05f, "~ac~w1~C99999999%5.1f, %5.1f, %5.1f", ((CBot *)m_pEntityCurrent)->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.x, 
						((CBot *)m_pEntityCurrent)->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.y, ((CBot *)m_pEntityCurrent)->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.z );
				} 
			#endif

			// Check to see if the player bot has been falling "in-place" or out of the world 
			// for a long time, at which point we will restore to a check point.  We will exclude 
			// the jump trooper and the predator from the falling "in-place" determination because 
			// they can stay in the air for long amounts of time.

			// Get some info on the camera
			GameCamType_e nCamType;
			gamecam_GetCameraManByIndex( (GameCamPlayer_e)m_nPlayerIndex, &nCamType );

			// We will only reset if the player is in first or third person camera and we're not in debug camera mode
			BOOL bValidCamera = ((nCamType == GAME_CAM_TYPE_ROBOT_3RD || nCamType == GAME_CAM_TYPE_ROBOT_1ST) && !gamecam_IsInDebugMode());

			if ( (m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) && bValidCamera ) {
				CBot *pPlayerBot = (CBot *)m_pEntityCurrent;

				// Determine player bot's out of world state
				BOOL bOutOfWorldExperience = FALSE;
				if ( pPlayerBot->m_pWorldMesh && !pPlayerBot->IsDeadOrDying() ) {
					FVisVolume_t *pCenterVol = pPlayerBot->m_pWorldMesh->GetCenterpointVolume();
					bOutOfWorldExperience = (pCenterVol == NULL || (pCenterVol->nVolumeID == FVIS_SLOP_BUCKET_ID && FWorld_pWorld->nVolumeCount != 1) );
#if FANG_WINGC
					// Port diagnostic: which of the two NULL-volume cases is this, and where is the
					// player actually standing? Remove once the out-of-world loop is understood.
					{
						static u32 nLogged = 0;
						if( nLogged < 12 ) {
	DEVPRINTF( "PORT-DIAG player pos=(%.1f,%.1f,%.1f) pCenterVol=%p volID=%d intersects=%u out=%d\n",
								pPlayerBot->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.x, pPlayerBot->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.y, pPlayerBot->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.z,
								pCenterVol, pCenterVol ? pCenterVol->nVolumeID : -1,
								pPlayerBot->m_pWorldMesh->GetIntersectCount(), bOutOfWorldExperience );
							nLogged++;
						}
					}
#endif
				}

				if ( bOutOfWorldExperience ) {
					// Player bot is out of the world, so check the timers for a restore state...
					if ( !m_bRestoreTimerRunning ) {
						// First time through...
						if ( m_nInDebugCameraResetDelay != 0 ) {
							// Player was previously in debug cam which means he most likely just dropped the player bot 
							// for repositioning.  In this case, we do not want the timer to toll
							m_bRestoreTimerSuspended = TRUE;
						}
						m_bRestoreTimerRunning = TRUE;
						m_fRestoreCheckpointTimer = 0.f;
					} else if ( !m_bRestoreTimerSuspended ) {
						m_fRestoreCheckpointTimer += FLoop_fPreviousLoopSecs;

						if ( m_fRestoreCheckpointTimer > 3.f ) {
							if( !MultiplayerMgr.RespawnBot( pPlayerBot ) ) {
								if( checkpoint_Saved( 1 ) ) {
									// player has passed a checkpoint on this level, so restore to that.
									checkpoint_Restore( 1, TRUE );
								} else if( checkpoint_Saved( 0 ) ) {
									// otherwise restore to beginning-of-level checkpoint
									checkpoint_Restore( 0, TRUE );
								}
							}
						}
					}
				}
				else if (   !(m_pEntityCurrent->TypeBits() & (ENTITY_BIT_BOTPRED|ENTITY_BIT_BOTJUMPER|ENTITY_BIT_VEHICLE|ENTITY_BIT_SITEWEAPON)) // We're not one of these excluded bots
							&& pPlayerBot->IsInAir()			// We're in the air
							&& !pPlayerBot->IsPanicOn()			// We're not in panic (usually means someone has the player in a loader, which is okay)
							&& pPlayerBot->m_pBotInfo_Gen ) {	// We've got the necessary information to make the restore determination
					// Player is of valid bot and is in the air, so see if he is stuck

					if ( !m_bRestoreTimerRunning ) {
						// First time through...
						if ( m_nInDebugCameraResetDelay != 0 ) {
							// Player was previously in debug cam which means he most likely just dropped the player bot 
							// for repositioning.  In this case, we do not want the timer to toll
							m_bRestoreTimerSuspended = TRUE;
						}
						m_bRestoreTimerRunning = TRUE;
						m_vPosWhenLeavingGround = pPlayerBot->MtxToWorld()->m_vPos.v3;
						m_fRestoreCheckpointTimer = 0.f;
					} else if ( !m_bRestoreTimerSuspended ) {
						// Incrememt the timer
						m_fRestoreCheckpointTimer += FLoop_fPreviousLoopSecs;

						CFVec3 vDiff = m_vPosWhenLeavingGround - pPlayerBot->MtxToWorld()->m_vPos.v3;
						if ( vDiff.Mag2() > (pPlayerBot->m_pBotInfo_Gen->fCollSphere1Radius_MS * pPlayerBot->m_pBotInfo_Gen->fCollSphere1Radius_MS) * 2.f ) {
							// Reset the timer because the player has experienced significant enough movement
							m_vPosWhenLeavingGround = pPlayerBot->MtxToWorld()->m_vPos.v3;
							m_fRestoreCheckpointTimer = 0.f;
						} else if ( m_fRestoreCheckpointTimer > 10.f ) {
							if( !MultiplayerMgr.RespawnBot( pPlayerBot ) ) {
								if( checkpoint_Saved( 1 ) ) {
									// player has passed a checkpoint on this level, so restore to that.
									checkpoint_Restore( 1, TRUE );
								} else if( checkpoint_Saved( 0 ) ) {
									// otherwise restore to beginning-of-level checkpoint
									checkpoint_Restore( 0, TRUE );
								}
							}
						}
					}
				} else {
					// Player bot is not out of the world or does not fall within the "in-the-air" criteria so reset the timers
					m_bRestoreTimerSuspended = FALSE;
					m_bRestoreTimerRunning = FALSE;
					m_fRestoreCheckpointTimer = 0.f;
				}
			} else {
				// Player is not in a bot or is in an improper camera state so reset the timers
				m_bRestoreTimerSuspended = FALSE;
				m_bRestoreTimerRunning = FALSE;
				m_fRestoreCheckpointTimer = 0.f;
			}

#if !FANG_PRODUCTION_BUILD
			// Record the camera's debug status for use next frame...
			if ( gamecam_IsInDebugMode() )
			{
				m_nInDebugCameraResetDelay = 10;
			}
			else if ( m_nInDebugCameraResetDelay )
			{
				m_nInDebugCameraResetDelay--;
			}
#endif

			if( m_pEntityCurrent->IsInWorld() ) {
				aibrainman_PlayerBotBrainWork( m_pEntityCurrent->AIBrain(), m_nPlayerIndex );
			}

			if( !( m_uPlayerFlags & PF_DONT_CALL_WORK ) ) {
				m_pEntityCurrent->Work();
			}
		}		

		if( (m_pEntityOrig != m_pEntityCurrent) && m_pEntityOrig->AIBrain() && m_pEntityOrig->AIBrain()->IsLODActive() ) {
			m_pEntityOrig->AIBrain()->ClearLODOverrideForOneWork();
			m_pEntityOrig->Work();
		}

		#if FANG_PLATFORM_WIN && !FANG_PRODUCTION_BUILD
			if( m_nPlayerIndex == 0 ) {
				if( m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
					CBotGlitch *pBotBlink = (CBotGlitch *)m_pEntityCurrent;
					u32 nCurrentLevel, nDesiredLevel;

					nCurrentLevel = pBotBlink->GetServoLevel( CBotGlitch::SERVO_TYPE_LEGS ) + 1;
					nDesiredLevel = gameloop_GetLegServoLevel();

					if( nDesiredLevel != _nLastFrameDesiredLegServoLevel ) {
						// GUI value has changed...

						if( nDesiredLevel ) {
							if( nCurrentLevel != nDesiredLevel ) {
								pBotBlink->SetServoLevel( CBotGlitch::SERVO_TYPE_LEGS, nDesiredLevel - 1 );
							}
						}
					}

					nCurrentLevel = pBotBlink->GetServoLevel( CBotGlitch::SERVO_TYPE_ARMS ) + 1;
					nDesiredLevel = gameloop_GetArmServoLevel();

					if( nDesiredLevel != _nLastFrameDesiredArmServoLevel ) {
						// GUI value has changed...

						if( nDesiredLevel ) {
							if( nCurrentLevel != nDesiredLevel ) {
								pBotBlink->SetServoLevel( CBotGlitch::SERVO_TYPE_ARMS, nDesiredLevel - 1 );
							}
						}
					}
				}
			}
		#endif
	}
}

void CPlayer::Resurrect( void )
{
	if( (m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) && ((CBot*)m_pEntityCurrent)->IsDead() ) {
		((CBot*)m_pEntityCurrent)->UnDie();

		#if FANG_PLATFORM_WIN && !FANG_PRODUCTION_BUILD
			_nLastFrameDesiredLegServoLevel = 0;
			_nLastFrameDesiredArmServoLevel = 0;
		#endif
	}

	m_pEntityCurrent->SetMaxHealth();
	m_pEntityCurrent->AddToWorld();

	if( m_pEntityCurrent->AIBrain() ) {
		aibrainman_ConfigurePlayerBotBrain(m_pEntityCurrent->AIBrain(), m_nPlayerIndex );
	}

	m_pEntityOrig->EnableAutoWork( FALSE );
	m_pEntityOrig->SetControls( &m_HumanControl );
}

void CPlayer::HandleTogglingOfOnscreenText( void ) {
	if( !(m_uPlayerFlags & PF_TEXT_REMOVED) ) {
		if( m_Hud.IsWSPaused() || CPauseScreen::IsActive() ) {
			FMATH_SETBITMASK( m_uPlayerFlags, PF_TEXT_REMOVED );
			_EnableText( FALSE );
		}
	} else {
		if( !m_Hud.IsWSPaused() && !CPauseScreen::IsActive() ) {
			FMATH_CLEARBITMASK( m_uPlayerFlags, PF_TEXT_REMOVED );
			_EnableText( TRUE );
		}
	}
}


// Called by the fvid system after everything's been drawn.
void CPlayer::_FVidDrawOverlay( void ) {
	CPlayer *pPlayer;
	u32 i;

	for( i=0, pPlayer=Player_aPlayer; i<(u32)m_nPlayerCount; ++i, ++pPlayer ) {
		if( pPlayer->m_pFcnDrawOverlay ) {
			pPlayer->m_pFcnDrawOverlay( i, pPlayer->m_pDrawOverlayUser );
		}

		if( pPlayer->m_uPlayerFlags & PF_FADING_VIEW ) {
			pPlayer->_ViewFadeDraw();
		}
	}
}


void CPlayer::StartViewFade( CFColorRGBA *pStartColorRGBA, CFColorRGBA *pEndColorRGBA, f32 fFadeTime ) {
	FMATH_SETBITMASK( m_uPlayerFlags, PF_FADING_VIEW );

	m_ViewFadeStartColorRGBA = *pStartColorRGBA;
	m_ViewFadeEndColorRGBA = *pEndColorRGBA;

	if( fFadeTime > 0.00001f ) {
		m_fDeltaUnitFade = fmath_Inv( fFadeTime );
		m_fUnitFade = 0.0f;
	} else {
		m_fDeltaUnitFade = 0.0f;
		m_fUnitFade = 1.0f;
	}
}


void CPlayer::StopViewFade( void ) {
	FMATH_CLEARBITMASK( m_uPlayerFlags, PF_FADING_VIEW );
}


void CPlayer::_ViewFadeDraw( void ) {
	CFColorRGBA ColorRGBA;
	CFVec3 Vtx1, Vtx2, Vtx3, Vtx4;
	f32 fZ;

	fZ = 0.5f * (m_pViewportOrtho3D->fNearZ + m_pViewportOrtho3D->fFarZ);

	if( m_fUnitFade < 1.0f ) {
		m_fUnitFade += FLoop_fPreviousLoopSecs * m_fDeltaUnitFade;
		FMATH_CLAMPMAX( m_fUnitFade, 1.0f );
	}

	if( m_fUnitFade == 1.0f ) {
		ColorRGBA = m_ViewFadeEndColorRGBA;
	} else if( m_fUnitFade == 0.0f ) {
		ColorRGBA = m_ViewFadeStartColorRGBA;
	} else {
		ColorRGBA.fRed = FMATH_FPOT( m_fUnitFade, m_ViewFadeStartColorRGBA.fRed, m_ViewFadeEndColorRGBA.fRed );
		ColorRGBA.fGreen = FMATH_FPOT( m_fUnitFade, m_ViewFadeStartColorRGBA.fGreen, m_ViewFadeEndColorRGBA.fGreen );
		ColorRGBA.fBlue = FMATH_FPOT( m_fUnitFade, m_ViewFadeStartColorRGBA.fBlue, m_ViewFadeEndColorRGBA.fBlue );
		ColorRGBA.fAlpha = FMATH_FPOT( m_fUnitFade, m_ViewFadeStartColorRGBA.fAlpha, m_ViewFadeEndColorRGBA.fAlpha );
	}

	Vtx1.Set( -m_pViewportOrtho3D->HalfRes.x,  m_pViewportOrtho3D->HalfRes.y, fZ );
	Vtx2.Set(  m_pViewportOrtho3D->HalfRes.x,  m_pViewportOrtho3D->HalfRes.y, fZ );
	Vtx3.Set(  m_pViewportOrtho3D->HalfRes.x, -m_pViewportOrtho3D->HalfRes.y, fZ );
	Vtx4.Set( -m_pViewportOrtho3D->HalfRes.x, -m_pViewportOrtho3D->HalfRes.y, fZ );

	fviewport_SetActive( m_pViewportOrtho3D );
	CFXfm::InitStack();

	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_SetTexture( NULL );

	fdraw_SolidQuad( &Vtx1, &Vtx2, &Vtx3, &Vtx4, &ColorRGBA );

	frenderer_Pop();
}


void CPlayer::_EnableText( BOOL bEnable ) {
	u32 j;

	if( bEnable ) {
		if( m_uPlayerFlags & PF_TEXT_BOX_VISIBLE_SAVE1 ) {
			ftext_GetAttributes( m_hTextBoxOutOfRange )->bVisible = TRUE;
		}

		if( m_uPlayerFlags & PF_TEXT_BOX_VISIBLE_SAVE2 ) {
			ftext_GetAttributes( m_hTextBoxScopeZoom )->bVisible = TRUE;

			for( j=0; j<RETICLE_SCOPE_INFO_SLOT_COUNT; ++j ) {
				ftext_GetAttributes( m_ahTextBoxScopeInfo[j] )->bVisible = TRUE;
			}
		}

		if( m_uPlayerFlags & PF_TEXT_BOX_VISIBLE_SAVE3 ) {
			ftext_GetAttributes( m_hTextBoxLowNoAmmo )->bVisible = TRUE;
		}

		if( m_uPlayerFlags & PF_TEXT_BOX_VISIBLE_SAVE4 ) {
			ftext_GetAttributes( m_hTextBoxRestart )->bVisible = TRUE;
		}

		FMATH_CLEARBITMASK( m_uPlayerFlags, PF_TEXT_BOX_VISIBLE_SAVE1 | PF_TEXT_BOX_VISIBLE_SAVE2 | PF_TEXT_BOX_VISIBLE_SAVE3 | PF_TEXT_BOX_VISIBLE_SAVE4 );
	} else {
		FMATH_CLEARBITMASK( m_uPlayerFlags, PF_TEXT_BOX_VISIBLE_SAVE1 | PF_TEXT_BOX_VISIBLE_SAVE2 | PF_TEXT_BOX_VISIBLE_SAVE3 | PF_TEXT_BOX_VISIBLE_SAVE4 );

		if( ftext_GetAttributes( m_hTextBoxOutOfRange )->bVisible ) {
			FMATH_SETBITMASK( m_uPlayerFlags, PF_TEXT_BOX_VISIBLE_SAVE1 );
		}

		if( ftext_GetAttributes( m_hTextBoxScopeZoom )->bVisible ) {
			FMATH_SETBITMASK( m_uPlayerFlags, PF_TEXT_BOX_VISIBLE_SAVE2 );
		}

		if( ftext_GetAttributes( m_hTextBoxLowNoAmmo )->bVisible ) {
			FMATH_SETBITMASK( m_uPlayerFlags, PF_TEXT_BOX_VISIBLE_SAVE3 );
		}

		if( ftext_GetAttributes( m_hTextBoxRestart )->bVisible ) {
			FMATH_SETBITMASK( m_uPlayerFlags, PF_TEXT_BOX_VISIBLE_SAVE4 );
		}

		ftext_GetAttributes( m_hTextBoxOutOfRange )->bVisible = FALSE;
		ftext_GetAttributes( m_hTextBoxScopeZoom )->bVisible = FALSE;
		ftext_GetAttributes( m_hTextBoxLowNoAmmo )->bVisible = FALSE;
		ftext_GetAttributes( m_hTextBoxRestart )->bVisible = FALSE;

		for( j=0; j<RETICLE_SCOPE_INFO_SLOT_COUNT; ++j ) {
			ftext_GetAttributes( m_ahTextBoxScopeInfo[j] )->bVisible = FALSE;
		}
	}
}


f32 CPlayer::ComputeLookSensitivityMultiplier( void ) const {
	f32 fUnitInterp;

	if( m_fLookSensitivity > 0.5f ) {
		fUnitInterp = (m_fLookSensitivity - 0.5f) * 2.0f;
		return FMATH_FPOT( fUnitInterp, 1.0f, 4.0f );
	} else if( m_fLookSensitivity < 0.5f ) {
		fUnitInterp = m_fLookSensitivity * 2.0f;
		return FMATH_FPOT( fUnitInterp, 0.25f, 1.0f );
	}

	return 1.0f;
}


// use this function to place a player in a vehicle for the duration of a level
BOOL CPlayer::SetInVehicle( CVehicle *pVehicle, CVehicle::Station_e eStation)
{
	if( pVehicle )
	{
		m_pRacingVehicle = pVehicle;
		FASSERT( m_pEntityOrig->TypeBits() & ENTITY_BIT_BOT );
		FASSERT( m_pRacingVehicle->TypeBits() & ENTITY_BIT_VEHICLE );
		CBot *pDrivingBot = (CBot*) m_pEntityOrig;
		CVehicle *pVehicle = (CVehicle*) m_pRacingVehicle;
		CFMtx43A mEntryPoint;
		CFVec3A vEntryPoint;
		CVehicle::StationStatus_e eStatus;

		ResetTargetAssistance();

		if( pVehicle->GetStationEntryPoint( eStation, &vEntryPoint ) )
		{
			mEntryPoint.Set( *pVehicle->MtxToWorld() );
			mEntryPoint.m_vPos.Set( vEntryPoint );
			pDrivingBot->Relocate_RotXlatFromUnitMtx_WS(  &mEntryPoint );
			eStatus = pVehicle->EnterStation( pDrivingBot, eStation, FALSE /*bProximityCheck*/, TRUE /*bImmediately*/ );
			if( eStatus == CVehicle::STATION_STATUS_ENTERING )
			{
				pVehicle->SnapCameraTransition();
				return TRUE;
			}
			else
			{
				FASSERT_NOW;
			}
		}
		else
		{
			FASSERT_NOW;
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// For setting game stats
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Credit this player with a kill. If it is team play and he killed a teammate,
// debit one kill.
void CPlayer::CreditKill( s32 nDeadPlayer, const CEntity* pWhat )
{
	if (m_GameStats.m_bIsSinglePlayer) {
		// Don't credit a suicide
		if (nDeadPlayer != m_nPlayerIndex)
			m_GameStats.m_SPStats.m_nUnitsDestroyed++;
	}
	else if (nDeadPlayer >= 0) {	// If this is multiplayer, ignore kills of unpossessed bots.
		// Don't credit a suicide
		if (nDeadPlayer != m_nPlayerIndex) {
			if (MultiplayerMgr.IsTeamPlay() && (m_nTeamNum == Player_aPlayer[nDeadPlayer].m_nTeamNum))
				m_GameStats.m_MPStats.m_nKills--;
			else
				m_GameStats.m_MPStats.m_nKills++;
		}

		// Notify the multiplayer game of the kill
		MultiplayerMgr.NotifyKill(m_nPlayerIndex, nDeadPlayer);
	}

	if (pWhat && m_nPlayerKillEntityEvent >=0)
	{
		CFScriptSystem::TriggerEvent(m_nPlayerKillEntityEvent, -1, (u32) m_pEntityCurrent, (u32) pWhat);
	}
}

//-----------------------------------------------------------------------------
// This player has died, count it. Multiplayer only.
void CPlayer::CreditDeath( void )
{
	if (!m_GameStats.m_bIsSinglePlayer)
		m_GameStats.m_MPStats.m_nDeaths++;
}

//-----------------------------------------------------------------------------
// The player picked up a washer. Single player only.
void CPlayer::CreditWasher( void )
{
	if (m_GameStats.m_bIsSinglePlayer)
		m_GameStats.m_SPStats.m_nWashersCollected++;
}

//-----------------------------------------------------------------------------
// Retrieve kills and deaths for multiplayer scoring. 
void CPlayer::GetMPScore(s8& nKills, u8& nDeaths)
{
	if (m_GameStats.m_bIsSinglePlayer) {	// Should never happen
		nKills = 0;
		nDeaths = 0;
	}
	else {
		nKills = m_GameStats.m_MPStats.m_nKills;
		nDeaths = m_GameStats.m_MPStats.m_nDeaths;
	}
}

void CPlayer::GetSPStats( u32 &rnUnitsDestroyed, u32 &rnWashersCollected ) {
	FASSERT( m_GameStats.m_bIsSinglePlayer );

	rnUnitsDestroyed = m_GameStats.m_SPStats.m_nUnitsDestroyed;
	rnWashersCollected = m_GameStats.m_SPStats.m_nWashersCollected;
}

//-----------------------------------------------------------------------------
// Override the number of kills, use for ColiseumMiniGame.
void CPlayer::OverrideUnitsDestroyed( u32 nUnitDestroyed )
{
	FASSERT( m_GameStats.m_bIsSinglePlayer );

	m_GameStats.m_SPStats.m_nUnitsDestroyed = nUnitDestroyed;
}

//-----------------------------------------------------------------------------
// Fill in the number of kills/deaths for each of the two teams.
// Returns the team which is in the lead, or -1 if they are tied.
s32 CPlayer::GetTeamScores(s32* pnKills, u32* pnDeaths)
{
	pnKills[0] = pnKills[1] = 0;
	pnDeaths[0] = pnDeaths[1] = 0;
	s32 nLeadingTeam = 0;

	if (MultiplayerMgr.IsMultiplayer()) {
		for (int i = m_nPlayerCount - 1; i >= 0; i--) {
			CPlayer& rPlayer = Player_aPlayer[i];
			FASSERT((rPlayer.m_nTeamNum == 0) || (rPlayer.m_nTeamNum == 1));
			pnKills[rPlayer.m_nTeamNum]  += rPlayer.m_GameStats.m_MPStats.m_nKills;
			pnDeaths[rPlayer.m_nTeamNum] += rPlayer.m_GameStats.m_MPStats.m_nDeaths;
		}

		// Pick the leading team
		if (pnKills[0] > pnKills[1])
			nLeadingTeam = 0;
		else if (pnKills[1] > pnKills[0])
			nLeadingTeam = 1;
		else if (pnDeaths[0] < pnDeaths[1])
			nLeadingTeam = 0;
		else if (pnDeaths[1] < pnDeaths[0])
			nLeadingTeam = 1;
		else
			nLeadingTeam = -1;	// Tie
	}

	return nLeadingTeam;
}

//-----------------------------------------------------------------------------
// Player found a secret area. Single player only.
void CPlayer::CreditSecretArea( void )
{
	if (m_GameStats.m_bIsSinglePlayer)
		m_GameStats.m_SPStats.m_nSecretAreasUncovered++;
}

//-----------------------------------------------------------------------------
// Player found a secret chip. Single player only.
void CPlayer::CreditSecretChip( void )
{
	if (m_GameStats.m_bIsSinglePlayer)
		m_GameStats.m_SPStats.m_nSecretChipsCollected++;
}
