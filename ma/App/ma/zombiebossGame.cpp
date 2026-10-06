
#include "fang.h"
#include "zombiebossGame.h"
#include "botzombieboss.h"
#include "fworld.h"
#include "player.h"
#include "entity.h"
#include "meshentity.h"
#include "bot.h"
#include "botgrunt.h"
#include "door.h"
#include "gamesave.h"
#include "./AI/AiApi.h"
#include "game.h"
#include "EBoomer.h" 
#include "BotMozer.h" 
#include "explosion.h"
#include "gamepad.h"

CZombieBossGame* CZombieBossGame::m_pZombieBossGame=NULL;

const f32 _SECS_TO_FADE_OUT = 3.0f;
const f32 _OO_SECS_TO_FADE_OUT = 1.0f/_SECS_TO_FADE_OUT;

#define _TOTAL_LOST_SECS		5.0f

typedef enum {
	_STATE_ACTIVE,
	_STATE_LOSE,
	_STATE_WIN,
	_STATE_COUNT
} _State_e;


BOOL CZombieBossGame::LoadLevel( LevelEvent_e eEvent )
{
	// Only load the data on LEVEL_EVENT_POST_WORLD_LOAD, this was the original behavior
	if (eEvent != LEVEL_EVENT_POST_WORLD_LOAD)
	{
		return TRUE;
	}

	BOOL bOK;			
	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();

	m_pZombieBossGame = fnew CZombieBossGame();

	if (!m_pZombieBossGame)
	{
		DEVPRINTF("Failed to CZombieBossGame::LoadLevel( void )");
		goto _ExitWithError;
	}
	
	bOK = m_pZombieBossGame->Create();
	if (!bOK)
		goto _ExitWithError;



	return TRUE;
_ExitWithError:
	fdelete(m_pZombieBossGame);
	fres_ReleaseFrame(ResFrame);
	return FALSE;
}


void CZombieBossGame::UnloadLevel( void ) 
{
	m_pZombieBossGame->Destroy();
	fdelete(m_pZombieBossGame);
	m_pZombieBossGame = NULL;
}

void CZombieBossGame::Restore( void )
{
	if (m_pZombieBossGame)
	{
		m_pZombieBossGame->m_nState = _STATE_ACTIVE;
		m_pZombieBossGame->m_fTimer = 0.0f;
		game_EndCutScene();
		floop_SetTimeScale(1.0f);
	}
}

void CZombieBossGame::Work( void )
{
	FASSERT(m_pZombieBossGame);
	m_pZombieBossGame->_Work();
}
void CZombieBossGame::_Work( void )
{
	m_fTimer += FLoop_fPreviousLoopSecs;
	
	if (m_nState == _STATE_ACTIVE)
	{
		if (m_pMozerCage && m_pMozer && m_pMozerCage->NormHealth() <= 0.0f)
		{
			if (m_pMozer->NormHealth() > 0.0f)
			{
				FExplosion_GroupHandle_t m_hDetonateGroup = FEXPLOSION_INVALID_HANDLE;
				m_hDetonateGroup = CExplosion2::GetExplosionGroup( "MozerDie" );
				if( m_hDetonateGroup == FEXPLOSION_INVALID_HANDLE ) 
				{
					DEVPRINTF( "CBotMozer::Die(): Could not find explosion definition '%s'.\n", "MozerDie" );
				}

				FExplosionSpawnParams_t SpawnParams;
				SpawnParams.InitToDefaults();
				SpawnParams.pDamager= NULL;
				SpawnParams.Pos_WS  = m_pMozer->MtxToWorld()->m_vPos;
				SpawnParams.UnitDir = m_pMozer->MtxToWorld()->m_vUp;

				FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
				if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
				{
					CExplosion2::SpawnExplosion( hSpawner, m_hDetonateGroup, &SpawnParams );
				}

				m_nState = _STATE_LOSE;
				m_fTimer = 0.0f;
				floop_SetTimeScale(0.5f);
				game_BeginCutScene("");
				m_pMozer->SetNormHealth(0.0f);
				m_pMozer->Die(FALSE,FALSE);
			}
		}
		if (m_pBossBot->DeathIsOver())
			EndGame();
	}
	if (m_nState == _STATE_LOSE)
	{
		BOOL bTerminateCutscene = FALSE;
		if( Gamepad_aapSample[CPlayer::m_pCurrent->m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) 
		{
			bTerminateCutscene = TRUE;
		}
		if( Gamepad_aapSample[CPlayer::m_pCurrent->m_nControllerIndex][GAMEPAD_MAIN_PAUSE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK ) 
		{
			bTerminateCutscene = TRUE;
		}
		if (m_fTimer > _TOTAL_LOST_SECS)
		{
			bTerminateCutscene = TRUE;
		}
		if (bTerminateCutscene)
		{
			// restart the level
			checkpoint_Restore( 0, FALSE, "zombie-boss:restart" );
		}
	}
}

BOOL CZombieBossGame::Create( void )
{
	CEntity* pEntity = CEntity::Find( "ZombieBotBoss" );
	if (!pEntity)
	{
		DEVPRINTF( "CZombieBossGame::Create() Cannot find 'target' entity '%s'.\n", "ZombieBotBoss" );
		goto _ExitWithError;
	}

	if (! (pEntity->TypeBits() & ENTITY_BIT_BOTZOMBIEBOSS) )
	{
		DEVPRINTF( "CZombieBossGame::Create() Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", pEntity->Name(), pEntity->MtxToWorld()->m_vPos.x, pEntity->MtxToWorld()->m_vPos.z, pEntity->MtxToWorld()->m_vPos.y );
		DEVPRINTF( "entity is not of type zombieboss, failure\n");
		goto _ExitWithError;
	}
	m_pBossBot = (CBotZombieBoss*)pEntity;

#if FANG_WINGC
	// The retail GameCube CZombieBossGame looks up only the boss and ends the level when its
	// death is over; the retail world has no 'mozer01' and the cage/Mozer lose rule is gone.
	m_pMozerCage = NULL;
	m_pMozer = NULL;
	m_nState = _STATE_ACTIVE;
	m_fTimer = 0.0f;
	return TRUE;
#endif

	pEntity = CEntity::Find( "mozercage" );
	if (!pEntity)
	{
		DEVPRINTF( "CZombieBossGame::Create() Cannot find 'target' entity '%s'.\n", "mozercage" );
		goto _ExitWithError;
	}
	if (! (pEntity->TypeBits() & ENTITY_BIT_BOOMER) )
	{
		DEVPRINTF( "CZombieBossGame::Create() Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", pEntity->Name(), pEntity->MtxToWorld()->m_vPos.x, pEntity->MtxToWorld()->m_vPos.z, pEntity->MtxToWorld()->m_vPos.y );
		DEVPRINTF( "entity is not of type eboomer, failure\n");
		goto _ExitWithError;
	}
	m_pMozerCage = (CEBoomer*)pEntity;
	pEntity = CEntity::Find( "mozer01" );
	if (!pEntity)
	{
		DEVPRINTF( "CZombieBossGame::Create() Cannot find 'target' entity '%s'.\n", "mozer01"  );
		goto _ExitWithError;
	}
	if (! (pEntity->TypeBits() & ENTITY_BIT_BOTMOZER) )
	{
		DEVPRINTF( "CZombieBossGame::Create() Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", pEntity->Name(), pEntity->MtxToWorld()->m_vPos.x, pEntity->MtxToWorld()->m_vPos.z, pEntity->MtxToWorld()->m_vPos.y );
		DEVPRINTF( "entity is not of type botmozer, failure\n");
		goto _ExitWithError;
	}
	m_pMozer = (CBotMozer*)pEntity;
	
	m_nState = _STATE_ACTIVE;
	m_fTimer = 0.0f;

	return TRUE;
_ExitWithError:
	return FALSE;
}

void CZombieBossGame::EndGame( void )
{
	game_GotoLevel( "next" );
}

void CZombieBossGame::Destroy( void )
{
}

void CZombieBossGame::Draw( void )
{
	if( m_pZombieBossGame->m_nState == _STATE_LOSE ) 
	{
		f32 fAlpha;
		if( m_pZombieBossGame->m_fTimer <= _SECS_TO_FADE_OUT ) 
		{
			fAlpha = 1.0f - (m_pZombieBossGame->m_fTimer * _OO_SECS_TO_FADE_OUT);
		} 
		else 
		{
			fAlpha = 0.0f;
		}
		game_DrawSolidFullScreenOverlay( fAlpha, 0.0f );

		ftext_ClearAllPending();

		if( m_pZombieBossGame->m_fTimer >= 1.0f ) 
		{
			ftext_Printf( 0.5, 0.35f, L"~f1~C75050580~w0~aC~s1.65%ls", Game_apwszPhrases[GAMEPHRASE_MISSION_FAILED] );
		}
	}
}