//////////////////////////////////////////////////////////////////////////////////////
// BotTalkInst.cpp - 
//
// Author: Justin Link      
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
// 08/20/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fanim.h"
#include "faudio.h"
#include "BotTalkInst.h"
#include "BotTalkData.h"
#include "BotTalkAction.h"
#include "FScriptSystem.h"
#include "player.h"
#include "ItemInst.h"
#include "Hud2.h"
#include "bot.h"
#include "level.h"
#include "collectable.h"

#if FANG_PLATFORM_WIN
#include "mmsystem.h"
#endif


#define _ENABLE_VOICE_DAMAGE 1

#define _FREQ_BROKEN_MIN	0.5f
#define _FREQ_BROKEN_MAX	1.2f
#define _STUTTER_CHANCE		0.5f

#define _PITCH_FADEOUT_RATE 1.0f

#define _MIN_STUTTER_ON		0.2f
#define _MAX_STUTTER_ON		0.5f
#define _MIN_STUTTER_OFF	0.2f
#define _MAX_STUTTER_OFF	0.4f

#define _STUTTER_OUT_CHANCE	0.33f


//====================
// private definitions

//=================
// public variables

//==================
// private variables

static const f32 _fBlendTime = 0.20f;
static const f32 _fOOBlendTime = 1.0f / _fBlendTime;

//===================
// private prototypes

//=================
// public functions

CBotTalkInst::CBotTalkInst() {
	_Clear();
}

CBotTalkInst::~CBotTalkInst() {
	Destroy();
}

BOOL CBotTalkInst::Init( cchar *pszSourceFileName ) {
	FASSERT( pszSourceFileName != NULL );

	CBotTalkData *pTalkData = CTalkSystem2::GetTalkDataByFileName( pszSourceFileName );
	if( pTalkData == NULL ) {
		DEVPRINTF("CBotTalkInst::Init() : Could not find talk data '%s'.\n", pszSourceFileName);
		return(FALSE);
	}
	return Init( pTalkData );
}

BOOL CBotTalkInst::Init( CBotTalkData *pTalkData ) {
	
	FASSERT( pTalkData != NULL );

	FResFrame_t hResFrame;
	hResFrame = fres_GetFrame();

	m_pTalkData = pTalkData;

	// Get our CFAnimInst array set up.
	m_paAnimInst = NULL;
	if( pTalkData->m_uAnimCnt ) {   //pgm added this line because really bad things happen when you allocate a 0 length array
		m_paAnimInst = fnew CFAnimInst[pTalkData->m_uAnimCnt];
		if( m_paAnimInst == NULL ) {
			DEVPRINTF("CBotTalkInst::Init() : Out of memory.\n");
			goto _ExitWithError;
		}
	}


	u32 uCurAnimIdx;
	for( uCurAnimIdx = 0; uCurAnimIdx < pTalkData->m_uAnimCnt; ++uCurAnimIdx ) {
		if( !m_paAnimInst[uCurAnimIdx].Create(pTalkData->m_papAnims[uCurAnimIdx]) ) {
			DEVPRINTF("CBotTalkInst::Init() : Error create anim inst.\n");
			goto _ExitWithError;
		}
	}
	
	return TRUE;

_ExitWithError:
	fres_ReleaseFrame(hResFrame);
	return FALSE;

}

void CBotTalkInst::Destroy() {
	if( m_paAnimInst ) {
		fdelete_array( m_paAnimInst );
	}
	m_paAnimInst = NULL;

	_Clear();
}

BOOL CBotTalkInst::Start( CBot *pBotToRun, BotTalkInstDoneCallback_t* pfcnTalkDoneCallback /* =NULL*/ ) {
	
	if( pBotToRun == NULL ) {
		DEVPRINTF("CBotTalkInst::Start() : Tried to start a CBotTalkInst that doesn't have a bot.\n");
		return(FALSE);
	}
	if( m_pTalkData == NULL ) {
		DEVPRINTF("CBotTalkInst::Start() : Tried to start a CBotTalkInst that doesn't have any talk data.\n");
		return(FALSE);
	}
	if( !pBotToRun->IsInWorld() ) {
		DEVPRINTF("CBotTalkInst::Start() : Tried to start a CBotTalkInst on a bot that isn't in the world.\n");
		return FALSE;
	}

	CBotTalkInst *pBTI = pBotToRun->m_pActiveTalkInst;
	if( (pBTI && (pBTI->IsHighPriority() || !IsHighPriority() )) ||  // An equal or more important talk activity is taking place, we can't disturb it.
		pBotToRun->UserAnim_IsLocked())	   // Some other system is using the user anim slots  //FINDFIX: could utilize the abort feature in cbot  someday

	{
#if FANG_WINGC
		if( Fang_bPortDiag ) {
			DEVPRINTF( "PORT-TALK refused '%s' on '%s': %s\n", m_pTalkData->m_pszSourceFileName ? m_pTalkData->m_pszSourceFileName : "?",
				pBotToRun->Name() ? pBotToRun->Name() : "?", pBotToRun->UserAnim_IsLocked() ? "user anims locked" : "another talk is playing" );
		}
#endif
		return FALSE;
	}
#if FANG_WINGC
	if( Fang_bPortDiag ) {
		DEVPRINTF( "PORT-TALK start '%s' on '%s' (%u actions)\n", m_pTalkData->m_pszSourceFileName ? m_pTalkData->m_pszSourceFileName : "?",
			pBotToRun->Name() ? pBotToRun->Name() : "?", m_pTalkData->m_uActionCnt );
	}
#endif

	if( pBTI != NULL ) {
		//stop a BTI that is currently playing on this bot
		pBTI->End();
	}

	m_pBotTarget = pBotToRun;
	m_pfcnTalkDoneCallback = pfcnTalkDoneCallback;

	pBotToRun->UserAnim_Lock( (void*)this );	//FINDFIX: could utilize the abort feature in cbot someday
	m_pBotTarget->m_pActiveTalkInst = this;

	m_uFlags &= ~(BOTTALKINSTFLAG_HASREACHEDEND|BOTTALKINSTFLAG_AUDIO_DAMAGED);

	m_fCurTimePos = 0.0f;
	m_uNextActionIdx = 0;

	m_fUnitDamageEffect			= 0.0f;
	m_fNextDamageEffectTime		= 0.0f;

	// Set up all of our user slot arrays.
	u32 uCurIdx;
	for( uCurIdx = 0; uCurIdx < m_pTalkData->m_uAnimCnt; ++uCurIdx ) {
		m_afStartTime[uCurIdx] = -1.0f;
		m_afEndTime[uCurIdx] = -1.0f;
		m_anAnimInstIdx[uCurIdx] = -1;
	}
	
	// Clear out all of the animation slots.
	m_pBotTarget->UserAnim_DetachAll();
	m_pBotTarget->UserAnim_ResetAllControlValues();
	
	// Check to see if we should trigger an event.
	if( IsTriggerEventStart() ) {
		CFScriptSystem::TriggerEvent(CTalkSystem2::m_nTalkEvent, 0, (u32)(m_pBotTarget), 0);
	}

	//automatically duck the audio and music if this sound
	//has the appropriate flag set.
	if( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) {
		level_DuckAudio(); 
	}

	m_pCurSound = NULL;

	_SetIsWorking( TRUE );

	return( TRUE );
}

void CBotTalkInst::Work() {

	FASSERT( IsWorking() );
	FASSERT( m_pBotTarget != NULL );

	if (IsWaitingForStreamLoad())
	{
		// Search for any streaming actions, don't do nothing until such be ready 
		CBotTalkAction *pAction = NULL;
		for (u32 nAction = 0; nAction < m_pTalkData->m_uActionCnt; nAction++)
		{
			pAction = &m_pTalkData->m_paActions[nAction];
			if (pAction->m_uData1 == BOT_TALK_CSV_ACTION_TYPE_STREAM)
			{
				FASSERT(pAction->m_uData3);
				//saved off the stream pointer here
				CFAudioStream* pStream = (CFAudioStream*)pAction->m_uData3;
				// while the stream is creating, talkinsts aren't working
				if (pStream->GetState() == FAUDIO_STREAM_STATE_CREATING)
					return;
			}
		}
		// reaching this point implies that no streaming files are creating
		SetWaitingForStreamLoad(FALSE);
	}

	m_fCurTimePos += FLoop_fPreviousLoopSecs;

	// Check to see if our talking is done.
	if( m_fCurTimePos >= m_pTalkData->m_fTotalTime ) {
		if( m_pTalkData->StickAtEnd() ) {
			End( m_pTalkData->StickAtEnd() );
		} else {
			CTalkSystem2::TerminateActiveTalk(this);
		}
		return;
	}

	// update the audio with the current bot's position
	if( m_pCurSound && 
		m_pBotTarget &&
		m_uFlags & BOTTALKINSTFLAG_3DSOUND_PLAYING ) {
		// update the pos of the audio emitter
		m_pCurSound->SetPosition( &m_pBotTarget->MtxToWorld()->m_vPos );
//		ftext_DebugPrintf( 0.4f, 0.3f, "~w1BotTalk sound playing" );

		if( m_pBotTarget->IsVoiceDestroyed() ) {
			if( _ENABLE_VOICE_DAMAGE ) {
//				DEVPRINTF( "Eyes destroyed during bottalk\n" );
				m_fNextDamageEffectTime = 0.0f;
				FMATH_SETBITMASK( m_uFlags, BOTTALKINSTFLAG_AUDIO_DAMAGED | BOTTALKINSTFLAG_AUDIO_FADETOSTOP );
			} else {
				m_pCurSound->Destroy();
				m_pCurSound = NULL;
			}
		}

#if _ENABLE_VOICE_DAMAGE
		if( m_uFlags & BOTTALKINSTFLAG_AUDIO_DAMAGED ) {
			_HandleAudioDamageEffects();
		}
#endif
	}
	
	// Search for any new action that might need to get started.
	while( (m_uNextActionIdx < m_pTalkData->m_uActionCnt) &&
		   (m_pTalkData->m_paActions[m_uNextActionIdx].m_fStartTime <= m_fCurTimePos) ) {

		CBotTalkAction *pCurBTA = &m_pTalkData->m_paActions[m_uNextActionIdx];
		switch( pCurBTA->m_uData1 ) {

			case BOT_TALK_CSV_ACTION_TYPE_ANIMATION:
			{
				// It's an animation.
				u32 uAnimSlot = pCurBTA->m_uData4;
				u32 uAnimInstIdx = pCurBTA->m_uData2;
				m_pBotTarget->UserAnim_Attach(uAnimSlot, &m_paAnimInst[uAnimInstIdx]);

				u32 nBonesDriven = m_pTalkData->DrivesBones();

				if( (nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY) &&
					(nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY) ) {
					// don't need to do anything if we are going to drive all the bones

				} else {
					// Begin bone mask update session...
					m_pBotTarget->UserAnim_BatchUpdateTapBoneMask_Open( uAnimSlot, TRUE );

					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_UPPER_BODY );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_LOWER_BODY );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_UPPER_TORSO ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_UPPER_TORSO );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LOWER_TORSO ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_LOWER_TORSO );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LEFT_ARM ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_LEFT_ARM );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_RIGHT_ARM ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_RIGHT_ARM );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_HEAD ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_HEAD );
					}

					// End bone mask update session...
					m_pBotTarget->UserAnim_BatchUpdateTapBoneMask_Close();
				}

				m_afStartTime[uAnimSlot] = pCurBTA->m_fStartTime;
				m_afEndTime[uAnimSlot] = m_afStartTime[uAnimSlot] + pCurBTA->m_fLength;
				m_anAnimInstIdx[uAnimSlot] = uAnimInstIdx;

				break;
			}
			case BOT_TALK_CSV_ACTION_TYPE_SOUND:
			{
				// It's a sound clip.
				FAudio_WaveHandle_t hWave = (FAudio_WaveHandle_t)( pCurBTA->m_uData2 );
#if FANG_WINGC
				if( Fang_bPortDiag ) {
					DEVPRINTF( "PORT-TALK   wave %08x radius=%.0f 2d=%d\n", (u32)hWave, m_fRadius, (m_uFlags & BOTTALKINSTFLAG_FORCE_2D_AUDIO) || m_pTalkData->Use2DSound() );
				}
#endif

				BOOL bDuckable = ( ( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) == 0 );

				if( m_uFlags & BOTTALKINSTFLAG_FORCE_2D_AUDIO ||
					m_pTalkData->Use2DSound() ||
					(m_pBotTarget->m_nPossessionPlayerIndex != -1) ) {
					// play a 2d sound
					CFAudioEmitter::Play2D( hWave, 1.0f, FAudio_EmitterDefaultPriorityLevel, bDuckable );
					m_uFlags &= ~(BOTTALKINSTFLAG_UPDATE_3D_POS | BOTTALKINSTFLAG_3DSOUND_PLAYING);
				} else {
					// play a 3d sound
					CFAudioEmitter::Play3D( hWave, 
						&m_pBotTarget->MtxToWorld()->m_vPos, 
						m_fRadius, 1.0f, FAudio_EmitterDefaultPriorityLevel, bDuckable);
					m_uFlags |= BOTTALKINSTFLAG_3DSOUND_PLAYING;
					m_uFlags &= ~BOTTALKINSTFLAG_UPDATE_3D_POS;
				}
				break;
			}
			case BOT_TALK_CSV_ACTION_TYPE_SFX:
			{
				if( m_pBotTarget->IsVoiceDestroyed() ) {
//					DEVPRINTF( "Tried to start bottalk with broken eyes\n" );
					break;
				}

				FSndFx_FxHandle_t hFXHandle = (FSndFx_FxHandle_t)pCurBTA->m_uData2;
				BOOL bDuckable = ( ( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) == 0 );
				
				if( m_uFlags & BOTTALKINSTFLAG_FORCE_2D_AUDIO ||
					m_pTalkData->Use2DSound() ||
					(m_pBotTarget->m_nPossessionPlayerIndex != -1) ) {
					// play a 2d sound
					m_pCurSound = FSNDFX_ALLOCNPLAY2D( hFXHandle, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.0f, bDuckable );
					m_uFlags &= ~(BOTTALKINSTFLAG_UPDATE_3D_POS | BOTTALKINSTFLAG_3DSOUND_PLAYING);
				} else {
					// play a 3d sound
					m_pCurSound = FSNDFX_ALLOCNPLAY3D( hFXHandle,
						&m_pBotTarget->MtxToWorld()->m_vPos,
						m_fRadius, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, bDuckable );
					m_uFlags |= (BOTTALKINSTFLAG_3DSOUND_PLAYING | BOTTALKINSTFLAG_UPDATE_3D_POS);
				}

				if( m_pCurSound && m_pBotTarget->IsVoiceDamaged() ) {
					m_fUnitDamageEffect = fmath_RandomFloatRange( _FREQ_BROKEN_MIN, _FREQ_BROKEN_MAX );
					m_pCurSound->SetFrequencyFactor( m_fUnitDamageEffect );
					if( fmath_RandomChance( _STUTTER_CHANCE ) ) {
						FMATH_SETBITMASK( m_uFlags, BOTTALKINSTFLAG_AUDIO_DAMAGED | BOTTALKINSTFLAG_AUDIO_STUTTER );
						m_fNextDamageEffectTime = fmath_RandomFloatRange( 0.2f, 0.5f );
					} else {
						FMATH_SETBITMASK( m_uFlags, BOTTALKINSTFLAG_AUDIO_DAMAGED | BOTTALKINSTFLAG_AUDIO_FADETOSTOP );
						m_fNextDamageEffectTime = fmath_RandomFloatRange( 0.75f, 2.0f );
					}
				}
				break;
			}
			case BOT_TALK_CSV_ACTION_TYPE_STREAM:
			{
				// It's a streamed audio clip.
#if FANG_PLATFORM_WIN && !FANG_WINGC	// the GameCube data ships speech as .wvs streams, played below as on the consoles
				if( pCurBTA->m_uData2 ) {
					//causes a stutter, but atleast it works
					cchar *pszWAVName = (cchar *)pCurBTA->m_uData2;
					PlaySound( pszWAVName, 0, SND_FILENAME );
				}
#else
				level_PlayStreamingSpeech((cchar*)pCurBTA->m_uData2, 1.0f, 1);
				if ( level_GetStreamingSpeechAudioStream() )
				{
					SetWaitingForStreamLoad(TRUE);
					pCurBTA->m_uData3 = (u32)level_GetStreamingSpeechAudioStream();
				}
#endif

				break;
			}
			default:
			{
				FASSERT_NOW;
				break;
			}
		}
		++m_uNextActionIdx;
	}
	
	// Search through for any anims that are currently playing.
	u32 uCurAnimIdx;
	for( uCurAnimIdx = 0; uCurAnimIdx < CBotAnimStackDef::ANIM_USER_COUNT; ++uCurAnimIdx ) {
		if( m_afStartTime[uCurAnimIdx] != -1.0f ) {
			FASSERT( m_afEndTime[uCurAnimIdx] != -1.0f );
			FASSERT( m_anAnimInstIdx[uCurAnimIdx] != -1 );

			if( m_fCurTimePos < m_afEndTime[uCurAnimIdx] ) {
				// This animation has not ended yet.  Let's calculate its blend value.

				// The reason we have two values here is because we want to force smooth
				//   blending at the start and the end of the talk inst, without regard
				//   to whether or not the started or ended animations outside of the
				//   bottalkinst time frame.
				f32 fRealSecsSinceStart = m_fCurTimePos - m_afStartTime[uCurAnimIdx];
				f32 fBlendSecsSinceStart = FMATH_MIN(m_fCurTimePos, fRealSecsSinceStart);

				f32 fRealSecsTilEnd = m_afEndTime[uCurAnimIdx] - m_fCurTimePos;
				f32 fBlendSecsTilEnd = FMATH_MIN(m_pTalkData->m_fTotalTime - m_fCurTimePos, fRealSecsTilEnd);

				if( fBlendSecsSinceStart < _fBlendTime ) {
					// Rising edge...
					m_pBotTarget->UserAnim_SetControlValue(uCurAnimIdx, fBlendSecsSinceStart * _fOOBlendTime);
				} else if( fBlendSecsTilEnd < _fBlendTime ) {
					// Falling edge...
					if( !m_pTalkData->NoBlendOut() ) {
						m_pBotTarget->UserAnim_SetControlValue( uCurAnimIdx, fBlendSecsTilEnd * _fOOBlendTime );
					}
				} else {
					// Stable at maximum...
					m_pBotTarget->UserAnim_SetControlValue(uCurAnimIdx, 1.0f);
				}
//				m_paAnimInst[m_anAnimInstIdx[uCurAnimIdx]].UpdateTime(m_fCurTimePos - m_afStartTime[uCurAnimIdx]);
				m_paAnimInst[m_anAnimInstIdx[uCurAnimIdx]].UpdateTime(fRealSecsSinceStart);
			} else {
				// The animation has reached its finish since the last frame.
				m_afStartTime[uCurAnimIdx] = -1.0f;
				m_afEndTime[uCurAnimIdx] = -1.0f;
				m_pBotTarget->UserAnim_SetControlValue(uCurAnimIdx, 0.0f);
			}
		}
	}
}



BOOL CBotTalkInst::End( BOOL bCanStickAtEnd ) {
	BOOL bDidIt = FALSE;
	FASSERT(m_pBotTarget->m_pActiveTalkInst == this);
	FASSERT(m_pBotTarget->UserAnim_IsLocked());

	if( !(bCanStickAtEnd && m_pTalkData->StickAtEnd()) ) {
		//really and truely end

		m_pBotTarget->m_pActiveTalkInst = NULL;
		m_pBotTarget->UserAnim_UnLock();

		m_pBotTarget->UserAnim_DisableAllBones(0);
		m_pBotTarget->UserAnim_DisableAllBones(1);
		m_pBotTarget->UserAnim_DisableAllBones(2);
		m_pBotTarget->UserAnim_DisableAllBones(3);

		m_pBotTarget->UserAnim_ResetAllControlValues();
		m_pBotTarget->UserAnim_DetachAll();

		if( m_pCurSound != NULL ) {
			m_pCurSound->Destroy();
			m_pCurSound = NULL;
		}
		_SetIsWorking(FALSE);

		bDidIt = TRUE;
	}

	if( !(m_uFlags & BOTTALKINSTFLAG_HASREACHEDEND) ) {
		m_uFlags |= BOTTALKINSTFLAG_HASREACHEDEND;


		// Check to see if we should trigger an event.
		if( IsTriggerEventEnd() ) {
			CFScriptSystem::TriggerEvent(CTalkSystem2::m_nTalkEvent, (u32)(m_pBotTarget), 0, 1);
		}
		
		// Check to see if we should give the player a chip.
		if( IsGiveChip() ) {
			_GivePlayerChip();
			m_uFlags &= ~(BOTTALKINSTFLAG_GIVECHIP);
		}
		
		//if this sound required the audio to be ducked, 
		//now that it's over, it needs to be unducked.
		if( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) {
			level_UnduckAudio();
		}

		if( m_pfcnTalkDoneCallback != NULL ) {
			m_pfcnTalkDoneCallback(this, m_pBotTarget);
		}
	}

	return bDidIt;
}


void CBotTalkInst::_GivePlayerChip() {
	// DFS - TODO: How does this apply to multiplayer?
	const s32 nPlayer = 0;

	CEntity *pE = Player_aPlayer[nPlayer].m_pEntityCurrent;
	if (pE &&
		pE->TypeBits() & ENTITY_BIT_BOT) {
		CBot *pPlayer = (CBot *)(pE);
		/*if(pPlayer->m_pInventory != NULL) {
			pPlayer->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo += 1;
		}
		CHud2::GetHudForPlayer(nPlayer)-> PickupItemGeneric(CTalkSystem2::GetChipMeshInst(), ITEMTYPE_CHIP, 2.0f);*/
		// NKM
		CCollectable::GiveToPlayer( pPlayer, "chip", 2.0f );
	}
}

//==================
// private functions

void CBotTalkInst::_Clear()
{
	u32 uCurIdx;
	for( uCurIdx = 0; uCurIdx < CBotAnimStackDef::ANIM_USER_COUNT; ++uCurIdx ) {
		m_afStartTime[uCurIdx] = -1.0f;
		m_afEndTime[uCurIdx] = -1.0f;
		m_anAnimInstIdx[uCurIdx] = -1;
	}

	m_fCurTimePos = 0.0f;
	m_pBotTarget = NULL;
	m_uNextActionIdx = 0;
	m_uFlags = 0;
	m_pfcnTalkDoneCallback = NULL;
	m_paAnimInst = NULL;
	m_TS2Next.pPrevLink = NULL;
	m_TS2Next.pNextLink = NULL;
	m_eTalkActivity = TALKACTIVITY2_NONE;
	m_pTalkData = NULL;
	m_pCurSound = NULL;
	m_fRadius = 150.0f;
}


void CBotTalkInst::_HandleAudioDamageEffects( void ) {
	if( !_ENABLE_VOICE_DAMAGE ) {
		return;
	}

	m_fNextDamageEffectTime -= FLoop_fPreviousLoopSecs;

	if( m_fNextDamageEffectTime > 0.0f ) {
		return;
	}

	if( !m_pCurSound ) {
		FMATH_CLEARBITMASK( m_uFlags, BOTTALKINSTFLAG_AUDIO_FADETOSTOP | BOTTALKINSTFLAG_AUDIO_DAMAGED |  BOTTALKINSTFLAG_AUDIO_STUTTER );
		return;
	}

	// ok, we're here, do something interesting...
//	FASSERT_UNIT_FLOAT( m_fUnitDamageEffect );
	
	if( m_uFlags & BOTTALKINSTFLAG_AUDIO_FADETOSTOP ) {
		m_fUnitDamageEffect -= FLoop_fPreviousLoopSecs * _PITCH_FADEOUT_RATE;
		if( m_fUnitDamageEffect <= 0.0f ) {
			m_pCurSound->Destroy();
			m_pCurSound = NULL;
			FMATH_CLEARBITMASK( m_uFlags, BOTTALKINSTFLAG_AUDIO_FADETOSTOP | BOTTALKINSTFLAG_AUDIO_DAMAGED |  BOTTALKINSTFLAG_AUDIO_STUTTER );
			return;
		} else {
			m_pCurSound->SetFrequencyFactor( m_fUnitDamageEffect );
		}
	} else if( m_uFlags & BOTTALKINSTFLAG_AUDIO_STUTTER ) {
		if( m_fUnitDamageEffect > 0.0f ) {
			m_pCurSound->SetVolume( 0.0f );
			m_fUnitDamageEffect = 0.0f;
			m_fNextDamageEffectTime = fmath_RandomFloatRange( _MIN_STUTTER_OFF, _MAX_STUTTER_OFF );
		} else {
			m_pCurSound->SetVolume( 1.0f );
			m_fUnitDamageEffect = 1.0f;
			m_fNextDamageEffectTime = fmath_RandomFloatRange( _MIN_STUTTER_ON, _MAX_STUTTER_ON );
			
			if( fmath_RandomChance( _STUTTER_OUT_CHANCE ) ) {
				FMATH_SETBITMASK( m_uFlags, BOTTALKINSTFLAG_AUDIO_FADETOSTOP );
			}
		}
	}
}


f32 CBotTalkInst::GetCurrentUnitTime( void ) const {
	if( m_pTalkData->m_fTotalTime != 0.0f ) {
		return fmath_Div( m_fCurTimePos, m_pTalkData->m_fTotalTime );
	} else {
		return 0.0f;
	}
}

















CTalkInst::CTalkInst() {
	_Clear();
}


void CTalkInst::_Clear()
{
	u32 uCurIdx;
	for( uCurIdx = 0; uCurIdx < CBotAnimStackDef::ANIM_USER_COUNT; ++uCurIdx ) {
		m_afStartTime[uCurIdx] = -1.0f;
		m_afEndTime[uCurIdx] = -1.0f;
		m_anAnimInstIdx[uCurIdx] = -1;
	}

	m_fCurTimePos = 0.0f;
	m_uNextActionIdx = 0;
	m_uFlags = 0;
	m_pfcnDoneCallback = NULL;
	m_paAnimInst = NULL;
	m_TS2Next.pPrevLink = NULL;
	m_TS2Next.pNextLink = NULL;
	m_eTalkActivity = TALKACTIVITY2_NONE;
	m_pTalkData = NULL;
	m_WherePos_WS.Zero();
	m_uUserData = 0;
	m_fRadius = 50.0f;
	m_pCurSound = NULL;
	m_pAnimCombiner = NULL;
}

CTalkInst::~CTalkInst() {
	Destroy();
}

BOOL CTalkInst::Init( cchar *pszSourceFileName ) {
	FASSERT( pszSourceFileName != NULL );

	CBotTalkData *pTalkData = CTalkSystem2::GetTalkDataByFileName( pszSourceFileName );
	if( pTalkData == NULL ) {
		DEVPRINTF("CTalkInst::Init() : Could not find talk data '%s'.\n", pszSourceFileName);
		return(FALSE);
	}
	return Init( pTalkData );
}

BOOL CTalkInst::Init( CBotTalkData *pTalkData ) {
	
	FASSERT( pTalkData != NULL );

	FResFrame_t hResFrame;
	hResFrame = fres_GetFrame();

	m_pTalkData = pTalkData;

	// Get our CFAnimInst array set up.
	m_paAnimInst = NULL;
	if( pTalkData->m_uAnimCnt ) {   //pgm added this line because really bad things happen when you allocate a 0 length array
		m_paAnimInst = fnew CFAnimInst[pTalkData->m_uAnimCnt];
		if( m_paAnimInst == NULL ) {
			DEVPRINTF("CTalkInst::Init() : Out of memory.\n");
			goto _ExitWithError;
		}
	}

	u32 i;
	for( i = 0; i < pTalkData->m_uAnimCnt; ++i ) {
		if( !m_paAnimInst[i].Create(pTalkData->m_papAnims[i]) ) {
			DEVPRINTF("CTalkInst::Init() : Error create anim inst.\n");
			goto _ExitWithError;
		}
	}
	
	return TRUE;

_ExitWithError:
	fres_ReleaseFrame(hResFrame);
	m_paAnimInst = NULL;
	return FALSE;

}

void CTalkInst::Destroy() {
	if( m_paAnimInst ) {
		fdelete_array( m_paAnimInst );
	}
	m_paAnimInst = NULL;

	_Clear();
}


BOOL CTalkInst::Start(	CFAnimCombiner* pCombiner,
						const s32 *panTapIds,
						const s32 *panCtrlIds,
						u32 uUserData,
						TalkInstDoneCallback_t* pfcnTalkDoneCallback /*= NULL */) {
	u32 i;
	
	if( !m_pTalkData  ||
		!panTapIds ||
		!panCtrlIds) {
		DEVPRINTF("CTalkInst::Start() : Tried to start a CTalkInst with bad params.\n");
		return(FALSE);
	}

	if( m_pCurSound ) {
		m_pCurSound->Destroy();
		m_pCurSound = NULL;
	}
	m_pAnimCombiner = pCombiner;
	m_uUserData = uUserData;
	m_pfcnDoneCallback = pfcnTalkDoneCallback;
	m_uFlags &= ~BOTTALKINSTFLAG_HASREACHEDEND;
	m_fCurTimePos = 0.0f;
	m_uNextActionIdx = 0;

	// init progress vbls
	u32 uCurIdx;
	for( uCurIdx = 0; uCurIdx < m_pTalkData->m_uAnimCnt; ++uCurIdx ) {
		m_afStartTime[uCurIdx] = -1.0f;
		m_afEndTime[uCurIdx] = -1.0f;
		m_anAnimInstIdx[uCurIdx] = -1;
	}
	
	// Copy tap and ctrl info and
	// Reset the combiner
	for (i = 0; i < NUM_BT_CHANNELS; i++)
	{
		m_panTapIds[i] = panTapIds[i];
		m_panCtrlIds[i] = panCtrlIds[i];
		m_pAnimCombiner->AttachToTap( m_panTapIds[i], NULL);
		m_pAnimCombiner->SetControlValue(m_panCtrlIds[i], 0.0f);
	}

	//automatically duck the audio and music if this sound
	//has the appropriate flag set.
	if( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) {
		level_DuckAudio();
	}

	_SetIsWorking( TRUE );

	return( TRUE );
}

BOOL CTalkInst::IsAtEnd(void)
{

	return !( IsWorking() && m_pAnimCombiner) ||( m_fCurTimePos >= m_pTalkData->m_fTotalTime );
}
											

f32 CTalkInst::GetTimeUnit(void)
{
	return m_fCurTimePos / m_pTalkData->m_fTotalTime;
}


void CTalkInst::Work() {

	if	(!IsWorking())
		return;
	
	FASSERT(m_pAnimCombiner);

	if (IsWaitingForStreamLoad())
	{
		// Search for any streaming actions, don't do nothing until such be ready 
		CBotTalkAction *pAction = NULL;
		for (u32 nAction = 0; nAction < m_pTalkData->m_uActionCnt; nAction++)
		{
			pAction = &m_pTalkData->m_paActions[nAction];
			if (pAction->m_uData1 == BOT_TALK_CSV_ACTION_TYPE_STREAM)
			{
				FASSERT(pAction->m_uData3);
				//saved off the stream pointer here
				CFAudioStream* pStream = (CFAudioStream*)pAction->m_uData3;
				// while the stream is creating, talkinsts aren't working
				if (pStream->GetState() == FAUDIO_STREAM_STATE_CREATING)
					return;
			}
		}
		// reaching this point implies that no streaming files are creating
		SetWaitingForStreamLoad(FALSE);
	}
	
	m_fCurTimePos += FLoop_fPreviousLoopSecs;

	// Completed?
	if( IsAtEnd() ) {
		if( m_pTalkData->StickAtEnd() ) {
			End( m_pTalkData->StickAtEnd() );
		} else {
			if (!CTalkSystem2::TerminateActiveTalk(this))
			{
				End();
			}
		}
		return;
	}
	CFVec3A m_WherePos_WS_A = m_WherePos_WS;
	// update the audio
	if( m_pCurSound &&
		m_uFlags & BOTTALKINSTFLAG_3DSOUND_PLAYING &&
		m_uFlags & BOTTALKINSTFLAG_UPDATE_3D_POS ) {
		// update the position and radius of the sound
		m_pCurSound->SetPosition( &m_WherePos_WS_A );
		// clear the update flag
		m_uFlags &= ~BOTTALKINSTFLAG_UPDATE_3D_POS;
	}
	
	// Search for any new action that might need to get started.
	while( (m_uNextActionIdx < m_pTalkData->m_uActionCnt) &&
		   (m_pTalkData->m_paActions[m_uNextActionIdx].m_fStartTime <= m_fCurTimePos) ) {

		CBotTalkAction *pCurBTA = &m_pTalkData->m_paActions[m_uNextActionIdx];
		switch( pCurBTA->m_uData1 ) {

			case BOT_TALK_CSV_ACTION_TYPE_ANIMATION:
			{
				// It's an animation.
				u32 uAnimSlot = pCurBTA->m_uData4;
				u32 uAnimInstIdx = pCurBTA->m_uData2;
				m_pAnimCombiner->AttachToTap( m_panTapIds[uAnimSlot], &m_paAnimInst[uAnimInstIdx]);

/*  Bone masking
				u32 nBonesDriven = m_pTalkData->DrivesBones();

				if( (nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY) &&
					(nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY) ) {
					// don't need to do anything if we are going to drive all the bones

				} else {
					// Begin bone mask update session...
					m_pBotTarget->UserAnim_BatchUpdateTapBoneMask_Open( uAnimSlot, TRUE );

					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_UPPER_BODY );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_LOWER_BODY );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_UPPER_TORSO ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_UPPER_TORSO );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LOWER_TORSO ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_LOWER_TORSO );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_LEFT_ARM ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_LEFT_ARM );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_RIGHT_ARM ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_RIGHT_ARM );
					}
					if( nBonesDriven & BOT_TALK_CSV_FLAG_DRIVES_HEAD ) {
						m_pBotTarget->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_HEAD );
					}

					// End bone mask update session...
					m_pBotTarget->UserAnim_BatchUpdateTapBoneMask_Close();
				}
*/
				m_afStartTime[uAnimSlot] = pCurBTA->m_fStartTime;
				m_afEndTime[uAnimSlot] = m_afStartTime[uAnimSlot] + pCurBTA->m_fLength;
				m_anAnimInstIdx[uAnimSlot] = uAnimInstIdx;

				break;
			}
			case BOT_TALK_CSV_ACTION_TYPE_SOUND:
			{
				// It's a sound clip.
				FAudio_WaveHandle_t hWave = (FAudio_WaveHandle_t)( pCurBTA->m_uData2 );
#if FANG_WINGC
				if( Fang_bPortDiag ) {
					DEVPRINTF( "PORT-TALK   wave %08x radius=%.0f 2d=%d\n", (u32)hWave, m_fRadius, (m_uFlags & BOTTALKINSTFLAG_FORCE_2D_AUDIO) || m_pTalkData->Use2DSound() );
				}
#endif

				BOOL bDuckable = ( ( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) == 0 );

				if( m_uFlags & BOTTALKINSTFLAG_FORCE_2D_AUDIO ||
					m_pTalkData->Use2DSound() ) {
					// play a 2d sound
					CFAudioEmitter::Play2D( hWave, 1.0f, FAudio_EmitterDefaultPriorityLevel, bDuckable );
					m_uFlags &= ~(BOTTALKINSTFLAG_UPDATE_3D_POS | BOTTALKINSTFLAG_3DSOUND_PLAYING);
				} else {
					// play a 3d sound
					CFVec3A m_WherePos_WS_A = m_WherePos_WS;
					CFAudioEmitter::Play3D( hWave, &m_WherePos_WS_A, m_fRadius,
						1.0f, FAudio_EmitterDefaultPriorityLevel, bDuckable );
					m_uFlags |= BOTTALKINSTFLAG_3DSOUND_PLAYING;
					m_uFlags &= ~BOTTALKINSTFLAG_UPDATE_3D_POS;
				}
				break;
			}
			case BOT_TALK_CSV_ACTION_TYPE_SFX:
			{
				FSndFx_FxHandle_t hFXHandle = (FSndFx_FxHandle_t)pCurBTA->m_uData2;
				BOOL bDuckable = ( ( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) == 0 );

				if( m_uFlags & BOTTALKINSTFLAG_FORCE_2D_AUDIO ||
					m_pTalkData->Use2DSound() ) {
					// play a 2d sound
					m_pCurSound = FSNDFX_ALLOCNPLAY2D( hFXHandle, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.0f, bDuckable );
					m_uFlags &= ~(BOTTALKINSTFLAG_UPDATE_3D_POS | BOTTALKINSTFLAG_3DSOUND_PLAYING);
				} else {
					// play a 3d sound
					const CFVec3A m_WherePos_WS_A = m_WherePos_WS;
					m_pCurSound = FSNDFX_ALLOCNPLAY3D( hFXHandle, &m_WherePos_WS_A, m_fRadius,
						1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, bDuckable );
					m_uFlags |= BOTTALKINSTFLAG_3DSOUND_PLAYING;
					m_uFlags &= ~BOTTALKINSTFLAG_UPDATE_3D_POS;
				}			
				break;
			}
			case BOT_TALK_CSV_ACTION_TYPE_STREAM:
			{
				// It's a streamed audio clip.
#if FANG_PLATFORM_WIN && !FANG_WINGC
				if( pCurBTA->m_uData2 ) 
				{
					//causes a stutter, but atleast it works
					cchar *pszWAVName = (cchar *)pCurBTA->m_uData2;
					PlaySound( pszWAVName, 0, SND_FILENAME );
				}
#else
				level_PlayStreamingSpeech((cchar*)pCurBTA->m_uData2, 1.0f, 1);
				if ( level_GetStreamingSpeechAudioStream() )
				{
					SetWaitingForStreamLoad(TRUE);
					pCurBTA->m_uData3 = (u32)level_GetStreamingSpeechAudioStream();
				}
#endif
				break;
			} 
			default:
			{
				FASSERT_NOW;
				break;
			}
		}
		++m_uNextActionIdx;
	}
	
	// Search through for any anims that are currently playing.
	u32 uCurAnimIdx;
	for( uCurAnimIdx = 0; uCurAnimIdx < NUM_BT_CHANNELS; ++uCurAnimIdx ) {
		if( m_afStartTime[uCurAnimIdx] != -1.0f ) {
			FASSERT( m_afEndTime[uCurAnimIdx] != -1.0f );
			FASSERT( m_anAnimInstIdx[uCurAnimIdx] != -1 );

			if( m_fCurTimePos < m_afEndTime[uCurAnimIdx] ) {
				// This animation has not ended yet.  Let's calculate its blend value.

				// The reason we have two values here is because we want to force smooth
				//   blending at the start and the end of the talk inst, without regard
				//   to whether or not the started or ended animations outside of the
				//   bottalkinst time frame.
				f32 fRealSecsSinceStart = m_fCurTimePos - m_afStartTime[uCurAnimIdx];
				f32 fBlendSecsSinceStart = FMATH_MIN(m_fCurTimePos, fRealSecsSinceStart);

				f32 fRealSecsTilEnd = m_afEndTime[uCurAnimIdx] - m_fCurTimePos;
				f32 fBlendSecsTilEnd = FMATH_MIN(m_pTalkData->m_fTotalTime - m_fCurTimePos, fRealSecsTilEnd);

				if( fBlendSecsSinceStart < _fBlendTime ) {
					// Rising edge...
					m_pAnimCombiner->SetControlValue(m_panCtrlIds[uCurAnimIdx], fBlendSecsSinceStart * _fOOBlendTime);

				} else if( fBlendSecsTilEnd < _fBlendTime ) {
					// Falling edge...
					if( !m_pTalkData->NoBlendOut() ) {
						m_pAnimCombiner->SetControlValue(m_panCtrlIds[uCurAnimIdx], fBlendSecsTilEnd * _fOOBlendTime);
					}
				} else {
					// Stable at maximum...
					m_pAnimCombiner->SetControlValue(m_panCtrlIds[uCurAnimIdx], 1.0f);
				}
				m_paAnimInst[m_anAnimInstIdx[uCurAnimIdx]].UpdateTime(fRealSecsSinceStart);
			} else {
				// The animation has reached its finish since the last frame.
				m_afStartTime[uCurAnimIdx] = -1.0f;
				m_afEndTime[uCurAnimIdx] = -1.0f;
				m_anAnimInstIdx[uCurAnimIdx] = -1;
				m_pAnimCombiner->SetControlValue(m_panCtrlIds[uCurAnimIdx], 0.0f);
			}
		}
	}
}

BOOL CTalkInst::End( BOOL bCanStickAtEnd ) {
	BOOL bDidIt = FALSE;
	u32 i;

	if( !(bCanStickAtEnd && m_pTalkData->StickAtEnd()) ) {
		//really and truely end

		// Copy tap and ctrl info and
		// Reset the combiner
		for (i = 0; i < NUM_BT_CHANNELS; i++)
		{
			m_pAnimCombiner->AttachToTap( m_panTapIds[i], NULL);
			m_pAnimCombiner->SetControlValue(m_panCtrlIds[i], 0.0f);
		}


		if( m_pCurSound != NULL ) {
			m_pCurSound->Destroy();
			m_pCurSound = NULL;
		}
		_SetIsWorking(FALSE);

		bDidIt = TRUE;
	}

	if( !(m_uFlags & BOTTALKINSTFLAG_HASREACHEDEND) ) {
		m_uFlags |= BOTTALKINSTFLAG_HASREACHEDEND;


		m_uFlags &= ~(BOTTALKINSTFLAG_GIVECHIP);
		
		//if this sound required the audio to be ducked, 
		//now that it's over, it needs to be unducked.
		if( m_uFlags & BOTTALKINSTFLAG_DUCK_AUDIO_WHEN_PLAYING ) {
			level_UnduckAudio();
		}

		if( m_pfcnDoneCallback != NULL ) {
			m_pfcnDoneCallback(this, m_uUserData);
		}

	}
	m_pAnimCombiner = NULL;

	return bDidIt;
}







