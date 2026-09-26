//////////////////////////////////////////////////////////////////////////////////////
// AICorrosive.cpp - 
//
// Author: Pat MacKellar / Russ Foushee 
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
// 03/21/03 Foushee		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

#include "AIApi.h"
#include "AICorrosive.h"
#include "AIFSM.h"
#include "AIThoughts3DBot.h"
#include "AIThoughtsGeneric.h"
#include "AIThoughtsGround.h"
#include "AINodePools.h"
#include "AIMover.h"
#include "AIBrain.h"
#include "AIGameUtils.h"
#include "AITactics.h"
#include "AIBTATable.h"
#include "AIGroup.h"

#include "../BotCorrosive.h"
#include "../BotTalkInst.h"
#include "../BotTalkData.h"
#include "weapon.h"
#include "sas_user.h"// you must include this if you are going to use sas_active_user!!!


extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );


#if( !FANG_PRODUCTION_BUILD && SAS_ACTIVE_USER == SAS_USER_RUSS )
	#define _OUTPUT_DEBUG_MESSAGES 1
#else
	#define _OUTPUT_DEBUG_MESSAGES 0
#endif

#define _STRATEGY_REEVALUATE_TIME	  5.0f	// How ofter to re-evalute strategies based on when LOS is achieved
#define _MELEE_ROCKET_FIRE_DELAY_TIME 8.0f	// How long to wait between firing rockets when in the MELEE strategy state...
#define _RANGE_ROCKET_FIRE_DELAY_TIME 3.0f	// How long to wait between firing rockets when in the RANGE strategy state...
#define _ATTACK_RETRY_TIME			  1.0f	// How long to wait between trying to determine if a new attack should be initiated.
#define _NO_LOS_ROAR_TIME			 15.0f	// How long to wait before roaring if LOS is lost
#define _BEAT_CHEST_CHECK_DELTA		 20.0f	// How often to make a random determination if we should beat chest/roar
#define _HINT_TIMEOUT_TIME			  3.0f	// How long to wait for a hint as to the position of the player.
#define _PAUSE_AFTER_HINT_ATTACK_TIME 1.0f
#define _FINGER_FLICK_THRESHOLD		 45.0f  // Distance threshold: is Corrosive close enough to do a finger flick?
#define _SEARCH_HINT_REQUEST_TIME	  5.0f  // How often to request a hint as to glitches location when in hint mode...
#define _NEW_PATH_REQUEST_TIME		  1.5f
#define _BOT_TOSS_CHECK_DELAY		 10.0f	// How often we should check to see if a bot should be tossed....
#define _OUCH_BTA_TIMEOUT			 10.0f  // The minimum time between OUCHES that we should wait.  Losing a limb overrides this timer.

static const f32 kfOOHun = 1.0f/100.0f;

CCorrosiveCombat* CCorrosiveCombat::m_pCorrosiveCombat = NULL;
BOOL CCorrosiveCombat::m_bRoarThisFrame = FALSE;
BOOL CCorrosiveCombat::m_bLaughThisFrame = FALSE;
BOOL CCorrosiveCombat::m_bFreeToFireRockets = TRUE;
BOOL CCorrosiveCombat::m_bFreeToBotToss = TRUE;

BehaviorRequestCallback_t *CCorrosiveCombat::m_pBehaviorRequestCallbackFcn = NULL;
f32 CCorrosiveCombat::m_aSinTable[ 24 ];
f32 CCorrosiveCombat::m_aCosTable[ 24 ];

BOOL CCorrosiveCombat::m_bUseAlternateAggressionCalculations;
f32 CCorrosiveCombat::m_fAggressiveness = 1.0f;
f32 CCorrosiveCombat::m_fFireDelayMul = 1.0f; 
f32 CCorrosiveCombat::m_fFireOddsMul = 1.0f;
f32 CCorrosiveCombat::m_fNormRunSpeed = 0.0f;
f32 CCorrosiveCombat::m_fRunSpeed = 1.0f;

BOOL CCorrosiveCombat::m_bForceRangeAttack = FALSE;

const f32 CCorrosiveCombat::m_fMeleeRange[CCorrosiveCombat::PREFERED_MELEE_COUNT] =
{
	45.0f,
	50.0f
};

const u8 _kuAttackMOdeTalkRequestTimeOutSecs = 1;
const u8 _kuSecsBetweenOuchBTAPlays = 10;

CCorrosiveCombat::CCorrosiveCombat( void )
 : CGroundCombat()
{
	m_pCorrosiveCombat = this;

	// Static variables, that should only be initialized at class creation time...
	m_bRoarThisFrame = FALSE;
	m_bLaughThisFrame = FALSE;

	m_ePreferedMeleeAttack = PREFERED_MELEE_STOMP;

	// Set up the investigate mark sin/cos table
	fmath_SinCos( FMATH_DEG2RAD(   0 ), &m_aSinTable[ 0 ], &m_aCosTable[ 0 ] );
	fmath_SinCos( FMATH_DEG2RAD(  15 ), &m_aSinTable[ 1 ], &m_aCosTable[ 1 ] );
	fmath_SinCos( FMATH_DEG2RAD( 345 ), &m_aSinTable[ 2 ], &m_aCosTable[ 2 ] );
	fmath_SinCos( FMATH_DEG2RAD(  30 ), &m_aSinTable[ 3 ], &m_aCosTable[ 3 ] );
	fmath_SinCos( FMATH_DEG2RAD( 330 ), &m_aSinTable[ 4 ], &m_aCosTable[ 4 ] );
	fmath_SinCos( FMATH_DEG2RAD(  45 ), &m_aSinTable[ 5 ], &m_aCosTable[ 5 ] );
	fmath_SinCos( FMATH_DEG2RAD( 315 ), &m_aSinTable[ 6 ], &m_aCosTable[ 6 ] );
	fmath_SinCos( FMATH_DEG2RAD(  60 ), &m_aSinTable[ 7 ], &m_aCosTable[ 7 ] );
	fmath_SinCos( FMATH_DEG2RAD( 300 ), &m_aSinTable[ 8 ], &m_aCosTable[ 8 ] );
	fmath_SinCos( FMATH_DEG2RAD(  75 ), &m_aSinTable[ 9 ], &m_aCosTable[ 9 ] );
	fmath_SinCos( FMATH_DEG2RAD( 285 ), &m_aSinTable[ 10 ], &m_aCosTable[ 10 ] );
	fmath_SinCos( FMATH_DEG2RAD(  90 ), &m_aSinTable[ 11 ], &m_aCosTable[ 11 ] );
	fmath_SinCos( FMATH_DEG2RAD( 270 ), &m_aSinTable[ 12 ], &m_aCosTable[ 12 ] );
	fmath_SinCos( FMATH_DEG2RAD( 105 ), &m_aSinTable[ 13 ], &m_aCosTable[ 13 ] );
	fmath_SinCos( FMATH_DEG2RAD( 255 ), &m_aSinTable[ 14 ], &m_aCosTable[ 14 ] );
	fmath_SinCos( FMATH_DEG2RAD( 120 ), &m_aSinTable[ 15 ], &m_aCosTable[ 15 ] );
	fmath_SinCos( FMATH_DEG2RAD( 240 ), &m_aSinTable[ 16 ], &m_aCosTable[ 16 ] );
	fmath_SinCos( FMATH_DEG2RAD( 135 ), &m_aSinTable[ 17 ], &m_aCosTable[ 17 ] );
	fmath_SinCos( FMATH_DEG2RAD( 225 ), &m_aSinTable[ 18 ], &m_aCosTable[ 18 ] );
	fmath_SinCos( FMATH_DEG2RAD( 150 ), &m_aSinTable[ 19 ], &m_aCosTable[ 19 ] );
	fmath_SinCos( FMATH_DEG2RAD( 210 ), &m_aSinTable[ 20 ], &m_aCosTable[ 20 ] );
	fmath_SinCos( FMATH_DEG2RAD( 165 ), &m_aSinTable[ 21 ], &m_aCosTable[ 21 ] );
	fmath_SinCos( FMATH_DEG2RAD( 195 ), &m_aSinTable[ 22 ], &m_aCosTable[ 22 ] );
	fmath_SinCos( FMATH_DEG2RAD( 180 ), &m_aSinTable[ 23 ], &m_aCosTable[ 23 ] );

	m_bUseAlternateAggressionCalculations = FALSE;
	m_fAggressiveness = 1.0f;
	m_fFireDelayMul = 1.0f; 
	m_fFireOddsMul = 1.0f;
	m_fNormRunSpeed = 0.0f;
	m_fRunSpeed = 1.0f;

	m_bGoDirectlyToRangeAttack = FALSE;

	m_pAttackCompleteCallback = NULL;
	m_pRangeNoRocketsCallback = NULL;

	m_uTauntTimeMin = 5;
	m_uTauntTimeMax	= 10;

	m_pRoarCallback = NULL;

	m_bForceRangeAttack = FALSE;
}


CCorrosiveCombat::~CCorrosiveCombat( void )
{
	m_pBehaviorRequestCallbackFcn = NULL;
}


void CCorrosiveCombat::Init( CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack /*= NULL*/ )
{
	CGroundCombat::Init( pBrain, uThoughtFlags, uThoughtType, pParamPack );

	// Re-initialize all our internal timers
	m_fTimeSinceLastFiredRockets = 0.0f;
	m_fTimeSinceLastTriedToAttack = 0.0f;
	m_nNextTagPointToLOSCheck = 0;

	ClearBotTossTimers();

	m_fTimeSinceLastBeatChest = 0.0f;
	m_fTimeSinceLastCheckedToBeatChest = 0.0f;
	m_bRoaredBecauseNoLOS = FALSE;
	m_bPlayLostLimbOuch = FALSE;

	m_eOverrideState = OVERRIDE_STATE_NONE;
	m_bNewOverrideState = FALSE;

	// Search variables
	m_fSearchTimeSinceRequestedHint = 0.0f;

	// Pathing variables
	m_uCurrPathReason = CGroundCombat::SEARCHREASON_DODGEIN;
	m_uNextPathReason = CGroundCombat::SEARCHREASON_DODGEOUT;
	m_bNewPathDestination = FALSE;
	m_fTimeSpentOnPath = 0.0f;

	m_uIMPC = 0;
	m_fIMPD = 20.0f;

	m_bAllowRandomRoar = TRUE;
	m_fTimeSinceOuchBTAPlayed = 0.0f;

	m_fTimeEnemyOnMe = 0.0f;
}


void CCorrosiveCombat::Cleanup(void)
{
	CGroundCombat::Cleanup();
}


void CCorrosiveCombat::Work(void)
{
	CGroundCombat::Work();	   //will do some setup, then call Ruleset_corrosive0 (once per frame)

}

f32 _kafCorrosiveMinRange[] = { 20.0f,  0.0f,  0.0f };
f32 _kafCorrosiveMaxRange[] = { 100.0f, 10.0f, 10.0f };

BOOL InitRuleSet_Corrosive0( u32 uControl, void* pParam1, void* pParam2 )
{
	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeRuleSetState( CGroundCombat::ARS_CORROSIVE0_BASE );	   // First state of ruleset_corrosive0 is always ARS_CORROSIVE0_BASE... for now.

	return AIFSM_INIT_CB_DONE;
}


BOOL DoRuleSet_Corrosive0( u32 uControl, void* pParam1, void* pParam2 )
{
	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	CBotCorrosive* pCorrosive = NULL;

	// First, cast a pointer to our bot as Corrosive
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}

	// See if we should trip a chest beating behavior!
	pT->BeatChestWork();

	// Check to see if we have any overrides this frame
	if( pT->m_bNewOverrideState ) {
		if( pT->m_eOverrideState != CCorrosiveCombat::OVERRIDE_STATE_NONE ) {
			switch( pT->m_eOverrideState ) {
				case CCorrosiveCombat::OVERRIDE_STATE_ATTACK_LOCATION:
					pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_ATTACKLOCATION);
					break;
				case CCorrosiveCombat::OVERRIDE_STATE_FINGER_FLICK:
					pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_FINGERFLICK);
					break;
				case CCorrosiveCombat::OVERRIDE_STATE_PEER_HERE:
					pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_PEERHERE);
					break;

				default:
					FASSERT_NOW; // Should not be here..
			}
		}
		pT->m_bNewOverrideState = FALSE;
	}

	// Check to see if the enemy has jumped on corrosive for any length of time...
	pT->IsEnemyOnMeWork();

	pT->m_RuleSetStateFSM.Work();

	pT->DoCorrosiveTauntWork();
	pT->DoOuchWork();
	pT->DoAggressionWork();

	_pWD->pMover->m_uMoverFlags |= CAIMover::MOVERFLAG_DONT_AVOID_PLAYER_BOTS;

#if _OUTPUT_DEBUG_MESSAGES == 1
	if( pT->m_pEnemy ) {
		fdraw_DevSphere( &_pWD->EnemyMark.v3, 3.0f );
	}
#endif

	return AIFSM_WORK_CB_RETURN;
}



BOOL InRules_ARS_Corrosive0_Base( u32 uControl, void* pParam1, void* pParam2 )
{
	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//first state is Base
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	pT->ChangeTactic( CGroundCombat::TACTIC_RANGEATTACK );

	if( pT->m_bGoDirectlyToRangeAttack ) {
		pT->m_eBaseStrategyState = CCorrosiveCombat::BASE_STRATEGY_STATE_RANGE_ATTACK; // Always perfer melee attack
	} else {
		pT->m_eBaseStrategyState = CCorrosiveCombat::BASE_STRATEGY_STATE_MELEE_ATTACK; // Always perfer melee attack
	}
	pT->m_fTimeInCurrentStrategyState = 0.0f;
	pT->m_vCachedEnemyMark = _pWD->EnemyMark;

	pT->ClearBotTossTimers();

	pT->m_bAllowRandomRoar = TRUE;

	return AIFSM_INIT_CB_DONE;
}


// This base ruleset is the base ruleset for Corrosive AI in the "General Corrosive" minigame level
BOOL DoRules_ARS_Corrosive0_Base( u32 uControl, void* pParam1, void* pParam2 )
{
	CFVec3A vDeltaEnemyMarkDist;
	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	CBotCorrosive* pCorrosive = NULL;

	// First, cast a pointer to our bot as Corrosive
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}
	FASSERT( pCorrosive );

	// Update our strategy timers
	pT->m_fTimeSinceLastFiredRockets += FLoop_fPreviousLoopSecs;
	pT->m_fTimeSinceLastTriedToAttack += FLoop_fPreviousLoopSecs;
	pT->m_fTimeInCurrentStrategyState += FLoop_fPreviousLoopSecs;

	BOOL bWayAboveHead = FALSE;
	BOOL bBelowFeet = FALSE;
	if( ( _pWD->EnemyMark.y - pCorrosive->MtxToWorld()->m_vPos.y ) > 55.0f ) { // used to be 75
		bWayAboveHead = TRUE;
	}
	if( ( _pWD->EnemyMark.y - pCorrosive->MtxToWorld()->m_vPos.y ) < -2.0f ) {
		bBelowFeet = TRUE;
	}

	// Figure out if we should switch strategies:
	CCorrosiveCombat::BaseStrategyState_e eNewStrategyState = pT->m_eBaseStrategyState;
	if ( pT->m_fTimeWithoutLOS < 2.0f ) {
		// We still have line of sight...
		if( pT->m_eBaseStrategyState == CCorrosiveCombat::BASE_STRATEGY_STATE_MELEE_ATTACK ) {
			// See if we should switch out of this state...

			// Conditions: Range attack is enforced, or Can't get to close range, 
			// or the player is so high up that we have no hope of achieving a melee attack......
			if( pT->IsRangeAttackForced() || ( pT->m_uRangePathFailureCount > 20 ) ) {
				eNewStrategyState = CCorrosiveCombat::BASE_STRATEGY_STATE_RANGE_ATTACK;
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "Switching Base Strategy state to RANGE ATTACK\n" );
#endif
			}
		} else if( pT->m_eBaseStrategyState == CCorrosiveCombat::BASE_STRATEGY_STATE_RANGE_ATTACK ) {
			// See if we should switch out of this state
			// Condition 1: The enemy mark has moved 10 feet since entering this state...
			vDeltaEnemyMarkDist.Sub( pT->m_vCachedEnemyMark, _pWD->EnemyMark );
			if( !pT->IsRangeAttackForced() && ( vDeltaEnemyMarkDist.MagSqXZ() > ( 10.0f*10.0f ) ) ) {
				// Revert back to melee state -- This guy is moving...
				eNewStrategyState = CCorrosiveCombat::BASE_STRATEGY_STATE_MELEE_ATTACK;
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "Switching Base Strategy state to MELEE ATTACK\n" );
#endif
			}
			else if ( !CCorrosiveCombat::m_bFreeToFireRockets && pT->m_pRangeNoRocketsCallback ) {
				pT->m_eOverrideState = CCorrosiveCombat::OVERRIDE_STATE_NONE;
				pT->m_pRangeNoRocketsCallback(0);
			}
		}
	}

	if( eNewStrategyState != pT->m_eBaseStrategyState ) {
		pT->m_eBaseStrategyState = eNewStrategyState;
		pT->m_fTimeInCurrentStrategyState = 0.0f;
		pT->m_vCachedEnemyMark = _pWD->EnemyMark;
	}

	// Determine what ranges we should be in based on where our enemy is and what our strategy is...
	f32 fMinRange, fMaxRange;
	u32 nERaptMode;

	switch( pT->m_eBaseStrategyState ) {
		case CCorrosiveCombat::BASE_STRATEGY_STATE_MELEE_ATTACK:
			if( pT->m_pEnemy && ( ( pT->m_pEnemy->MtxToWorld()->m_vPos.y - pCorrosive->MtxToWorld()->m_vPos.y ) > 30.0f ) )  {
				// Pick a fist smash range
				fMinRange = 25.0f;
				fMaxRange = 37.0f;
			} else {
				// Pick a stomp / swipe range
				fMinRange = 10.0f;
				fMaxRange = 30.0f;
			}
			nERaptMode = CGroundCombat::ERAPTUSEAGECTRL_NEVER;
			break;

		case CCorrosiveCombat::BASE_STRATEGY_STATE_RANGE_ATTACK:
			if( bBelowFeet ) {
				// Get really close and rain rockets down from above!
				fMinRange = 20;
				fMaxRange = 75;
			} else if( bWayAboveHead ) {
				// Put some distance between ourself and the player because he's up pretty high...
				fMinRange = 100;
				fMaxRange = 150;
			} else {
				// Normal range mode, perhaps here after establishing LOS from an investigate mark...
				fMinRange = 20;
				fMaxRange = 150;
			};
			nERaptMode = CGroundCombat::ERAPTUSEAGECTRL_NEVER;
			break;

		case CCorrosiveCombat::BASE_STRATEGY_STATE_NONE:
			nERaptMode = CGroundCombat::ERAPTUSEAGECTRL_NEVER;
			fMinRange = 0.0f;
			fMaxRange = 100.0f;
			break;

		default:
			FASSERT_NOW; // Should not be here
	}

	_GARangeSetParams(	pT,
						4,										//u8 uDodgeTimeMin,   
						0,										//u8 uDodgeTimeMax,   
						4,										//u8 uDodgeOutTimeMin,
						0,										//u8 uDodgeOutTimeMax,
						1,										//u8 uDamageDodgeTimeMin
						fMinRange,								//f32 fRangeMin
						fMaxRange,								//f32 fRangeMax
						nERaptMode,
						0.0f
						);

	// Give range attack time
	f32 fCorrosiveInvestigateMarkAfterTime = 5.0f - pT->GetAggressiveness()*2.0f; //put a random number here
	FMATH_CLAMP_MIN0( fCorrosiveInvestigateMarkAfterTime );

	//if
	//	 Haven't seen the enemy for some time (aggression is a factor) &&
	//   currently am more than _kfInvestigateMarkMinDist feet from enemymark  (prevents rule from firing repeatedly)
	//then
	//   Investigate the mark
	//
	if ( pT->m_fTimeWithoutLOS > fCorrosiveInvestigateMarkAfterTime &&
//		 !pT->FindRequestSlotInUse(CGroundCombat::RESULTSFOR_NEWATTACKPOS) &&  // checks if he is on his way or searching for an erapt point
//		 pT->m_AttackPtSearch.m_uFailureCount > 60 && 
		 _pWD->fDistToEnemyMarkXZ >= _kfInvestigateMarkMinDist )
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_CORROSIVE0_INVESTIGATEMARK );
	}

	if( ( pT->m_fTimeEnemyOnMe ) > 1.5f ) {
		// Go into our super duper special get the enemy off me attack ruleset state
		pT->ChangeRuleSetState( CGroundCombat::ARS_CORROSIVE0_GETENEMYOFFME );
	}

	// Do Bot Toss work here
	pT->BotTossWork();

	return AIFSM_WORK_CB_RETURN;
}


BOOL OutRules_ARS_Corrosive0_Base( u32 uControl, void* pParam1, void* pParam2 )
{
	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	pT->m_eBaseStrategyState = CCorrosiveCombat::BASE_STRATEGY_STATE_NONE;

	return AIFSM_WORK_CB_RETURN;
}


void CCorrosiveCombat::ClearBotTossTimers( void ) {
	m_fTimeSinceLastBotToss = 0.0f;
	m_fTimeSinceLastCheckedToTossBots = 0.0f;
}

// This routine determines if a bot should be spawned.
// The criteria includes 
//  1) a fixed base chance
//  2) how long it's been since a bot toss previously occured
//  3) how much damage has been accumulated
void CCorrosiveCombat::BotTossWork( void ) {

	CBotCorrosive* pCorrosive = NULL;
	if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) s_pWD->pBot;
	}

	// Only do this work if we have an enemy
	if( !s_pWD->pEnemyBot ) {
		return;
	}

	// First, increment our timers...
	m_fTimeSinceLastBotToss += FLoop_fPreviousLoopSecs;
	m_fTimeSinceLastCheckedToTossBots += FLoop_fPreviousLoopSecs;

	if( !m_bFreeToBotToss ) {
		// We are not allowed to bot toss...!
		return;
	}

	// Now, check to see if we should even try to toss a bot...
	if( m_fTimeSinceLastCheckedToTossBots > _BOT_TOSS_CHECK_DELAY ) {
		BOOL bClearCheckTimer = TRUE;
		// We should see if we can even do a bot toss
		if( pCorrosive && ( pCorrosive->GetBehaviorState() != CBotCorrosive::BEHAVIOR_STATE_NONE ) ) {
			// Another action is currently in progress, don't reset our check timer
			bClearCheckTimer = FALSE;
		} else if( pCorrosive && pCorrosive->CanTossABot() ) {
			// We can do a bot toss, compute a random chance that we WILL do a bot toss...
			u32 uTossOdds = 25;  // Initial one in four chance that a bot will be tossed.

			uTossOdds += (u32) ( m_fTimeSinceLastBotToss * 1.5f ); // Take into account how long it's been since we tossed a bot
			uTossOdds += (u32) ( ( 1.0f - pCorrosive->NormHealth() ) * 30.0f ); // As corrosive gets more damamged, he has a better chance of spawning a bot 

			if ( fmath_RandomChoice( 100 ) < uTossOdds ) {
				// Toss a bot, and clear the timer
				if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_BOT_TOSS ) ) {
					m_fTimeSinceLastBotToss = 0.0f;
				}
			}
		}
		if( bClearCheckTimer ) {
			// Clear the check timer...
			m_fTimeSinceLastCheckedToTossBots = 0.0f;
		}
	}
}

void CGroundCombat::DoWeaponAimAndFireWork_Corrosive( void ) {
	CCorrosiveCombat* pT = (CCorrosiveCombat*) this;

	// First, cast a pointer to our bot as Corrosive
	CBotCorrosive* pCorrosive = NULL;
	if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) s_pWD->pBot;
	}

	// What we want to do here is determine the distance from the enemy to ourself.
	// This distance will determine what type of an attack we use...

	if( pT->m_fTimeSinceLastTriedToAttack > _ATTACK_RETRY_TIME ) {
		// Check here to see what our range is.
		// If our range is outside of Melee distance, perfer a rocket shot 
		// over a melee attack (while closing the distance)
		BOOL bFreeToAttack = FALSE;
		if ( m_pEnemy ) { // HasTargetLock means that the weapon will fire. Angle is within some internal tolerance to be considered a "lock" circa 20degrees

			// First, we should check to see where the enemy is in relation to Corrosive.
			// If Then enemy is BELOW OUR FEET, then we should forget the stomp and go straight for
			// the rocket attacks...
			if( ( pT->m_eBaseStrategyState == CCorrosiveCombat::BASE_STRATEGY_STATE_MELEE_ATTACK ) &&
				( s_pWD->fDistToEnemyMarkXZ <= CCorrosiveCombat::m_fMeleeRange[ pT->m_ePreferedMeleeAttack ] ) ) {
				// We only do Melee attacks if we are in Melee_attack base state...
				bFreeToAttack = TRUE;
				pT->DoWeaponFireWork_Melee();
			}
			else if( pT->m_eBaseStrategyState != CCorrosiveCombat::BASE_STRATEGY_STATE_NONE ) {
				// We only fire if we are in either a range attack or a melee attack (which failed the melee condition)
				// If we are in range attack mode, just fire away...!
				if( pT->m_eBaseStrategyState == CCorrosiveCombat::BASE_STRATEGY_STATE_RANGE_ATTACK ) {
					bFreeToAttack = TRUE;
				}
				if( !bFreeToAttack ) {
					bFreeToAttack = pT->HasClearRocketShot();
				}

				if( bFreeToAttack ) {
					// Okay to Fire Rockets!
					pT->DoWeaponFireWork_Rocket();
				}
			}
		}
		// Reset the Attack Re-try timer
		if( bFreeToAttack ) {
			pT->m_fTimeSinceLastTriedToAttack = 0.0f;
		}
	}

	// AIM Logic

	// Stomping is such a slow process, that there will be no inaccuracy in the aim.  It will be 
	// relatively easy for the player to get out of the way, because the stomp itself will have
	// a high degree of inaccuracy.

	m_LastAim = s_pWD->EnemyMarkTagPos;

	FMATH_DEBUG_FCHECK(m_LastAim.x);
	FMATH_DEBUG_FCHECK(m_LastAim.y);
	FMATH_DEBUG_FCHECK(m_LastAim.z);

	s_pWD->pMover->m_Controls.AimAt(m_LastAim);


	// Let the bot layer know who the currently targeted enemy is (This is usefull for both Homing rockets and providing information to spawned tossed bots)...
	((CBotCorrosive*) s_pWD->pMover->GetEntity())->SetTargetedEntity(m_pEnemy);

}



BOOL InRules_ARS_Corrosive0_InvestigateMark( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic( CGroundCombat::TACTIC_NONE );
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	pT->m_uIMPC = 0;
	pT->m_fIMPD = 10.0f;

	// We lost line of sight, so see if we should get a special behavior instruction...
	if( pT->m_pBehaviorRequestCallbackFcn ) {
		pT->m_pBehaviorRequestCallbackFcn( AI_BEHAVIOR_REASON_LOST_LOS );
	}

	pT->m_bAllowRandomRoar = TRUE;


#if _OUTPUT_DEBUG_MESSAGES == 1
	DEVPRINTF( "LOST LOS - Investigating Mark!\n" );
#endif
	return AIFSM_INIT_CB_DONE;
}


BOOL DoRules_ARS_Corrosive0_InvestigateMark(u32 uControl, void* pParam1, void* pParam2) {
	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	CBotCorrosive* pCorrosive = NULL;
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}
	FASSERT( pCorrosive );

	//pT->m_uAttackFlags |= CGroundCombat::FLAG_HEAD_SCAN;

	// If we establish line of sight at any point in time, bail out of this
	// ruleset and go back to the base attack state!
	if ( pT->m_fTimeWithLOS > 0.0f && pT->HasClearRocketShot() ) {
 		pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_BASE);
#if _OUTPUT_DEBUG_MESSAGES == 1
		DEVPRINTF( "LOS Re-Established -- going back to base ruleset!\n" );
#endif
	}
#if 0
	// If we have not had line of sight for 15 seconds, and we are not following a path to establish los.
	if ( ( pT->m_fTimeWithoutLOS > 15.0f ) && !pT->IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS) ) {
 		pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_SEARCH);
#if _OUTPUT_DEBUG_MESSAGES == 1
		DEVPRINTF( "Long time without LOS, going into search mode!\n" );
#endif
	}
#endif

	// NEW 24 point search stuff!
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_DIR_ENEMY;  // always be looking toward the enemy mark...
	if( !pT->FindInvestigatePt() ) {  // And then go there!
		// We couldn't find a point to investigate that was appropriate, so recurse back
		// to the search state.
		pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_SEARCH);
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_Corrosive0_Search( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic( CGroundCombat::TACTIC_SEARCH );
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	_GASearchSetParams( pT );

	pT->m_fSearchTimeSinceRequestedHint = 0.0f;

	pT->m_bAllowRandomRoar = FALSE;

	return AIFSM_INIT_CB_DONE;
}

BOOL DoRules_ARS_Corrosive0_Search( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	//
	//If 
	//		sees the enemy
	//then
	//		go back to base state
	//
	if (pT->m_fTimeWithoutLOS < 0.7f)
	{
		pT->ChangeRuleSetState( CGroundCombat::ARS_CORROSIVE0_BASE );
	}

	pT->m_fSearchTimeSinceRequestedHint += FLoop_fPreviousLoopSecs;

	// See if we should request a hint (cheater!)
	if( pT->m_fSearchTimeSinceRequestedHint > _SEARCH_HINT_REQUEST_TIME ) {
		// Get a clue man!
		BOOL bGotExternalBehavior = FALSE;
		if( pT->m_pBehaviorRequestCallbackFcn ) {
			pT->m_pBehaviorRequestCallbackFcn( AI_BEHAVIOR_REASON_LOST_LOS );
		}
		pT->m_fSearchTimeSinceRequestedHint = 0.0f;
	}

	return AIFSM_WORK_CB_RETURN;
}


BOOL InRules_ARS_Corrosive0_AttackLocation( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic( CGroundCombat::TACTIC_NONE );
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()

	pT->m_eAttackLocationState = CCorrosiveCombat::ATTACKLOCATION_STATE_GOTOATTACKPOINT;
	pT->PathSetDestination( & pT->m_vOverrideAttackLocation );

	pT->m_bAllowRandomRoar = FALSE;

	return AIFSM_INIT_CB_DONE;
}

// This rule set has corrosive attack a specific location.
// The enemy is completely ignored while corrosive is in this mode....
BOOL DoRules_ARS_Corrosive0_AttackLocation( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	CBotCorrosive* pCorrosive = NULL;
	BOOL bGotExternalBehavior;
	CFVec3A vTemp;

	// First, cast a pointer to our bot as Corrosive
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}

	// If we establish line of sight at any point in time, bail out of this
	// ruleset and go back to the base attack state (assuming we are allowed to)!
	if ( pT->m_bAllowEnemyLOSToAbortOverride && ( (u16) pT->m_fTimeWithoutLOS < ( _pWD->uNowTime - pT->m_uTacticInitTime ) ) ) {
		pT->HandlePathing( TRUE ); // kill our current path
 		pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_BASE);
#if _OUTPUT_DEBUG_MESSAGES == 1
		DEVPRINTF( "LOS Re-Established -- going back to base ruleset!\n" );
#endif
	}

	// Run through our attack location state machine and issue the attack...
	switch ( pT->m_eAttackLocationState ) {
		case CCorrosiveCombat::ATTACKLOCATION_STATE_GOTOATTACKPOINT:
			pT->HandlePathing( FALSE );

			// Check to see if we are close enough
			vTemp.Sub( pCorrosive->MtxToWorld()->m_vPos, pT->m_vOverrideAttackLocation );
			if( vTemp.MagXZ() <= 30.0f ) {
				// We are close enough..
				pT->HandlePathing( TRUE ); // kill our current path
				pT->m_eAttackLocationState = CCorrosiveCombat::ATTACKLOCATION_STATE_FACEATTACKPOINT;
			}
			break;
	
		case CCorrosiveCombat::ATTACKLOCATION_STATE_FACEATTACKPOINT:
			// Here, we want to face the location we want to attack!
			pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
			if( _pWD->pMover->FaceToward( pT->m_vOverrideAttackLocation ) ) {
				// We are facing the proper direction... Move on!
				pT->m_eAttackLocationState = CCorrosiveCombat::ATTACKLOCATION_STATE_ISSUEATTACK;
			}
			break;
	
		case CCorrosiveCombat::ATTACKLOCATION_STATE_ISSUEATTACK:

			// Keep issuing the facetowards while we are waiting for the attack be to issued
			pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
			_pWD->pMover->FaceToward( pT->m_vOverrideAttackLocation );

			// Wait until we successfully have requested the attack
			if( ( pT->m_vOverrideAttackLocation.y - pCorrosive->MtxToWorld()->m_vPos.y ) > 30.0f ) {
				if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_FIST_SMASH ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
					DEVPRINTF( "Fist Smashing!\n" );
#endif
					pT->m_eAttackLocationState = CCorrosiveCombat::ATTACKLOCATION_STATE_WAITFORATTACKTOCOMPLETE;
				}
			} else {
				// Determine what type of stomp we should execute!
				vTemp.Sub( pCorrosive->MtxToWorld()->m_vPos, pT->m_vOverrideAttackLocation );
				if( vTemp.MagXZ() <= 15.0f ) {
					// Go Classic..
					if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_STOMP ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
						DEVPRINTF( "Stomping!\n" );
#endif
						pT->m_eAttackLocationState = CCorrosiveCombat::ATTACKLOCATION_STATE_WAITFORATTACKTOCOMPLETE;
					}
				} else {
					// Go new style forward stomp
					if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_FORWARD_STOMP ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
						DEVPRINTF( "Forward Stomping!\n" );
#endif
						pT->m_eAttackLocationState = CCorrosiveCombat::ATTACKLOCATION_STATE_WAITFORATTACKTOCOMPLETE;
					}
				}
			}
			break;
	
		case CCorrosiveCombat::ATTACKLOCATION_STATE_WAITFORATTACKTOCOMPLETE:

			// Keep issuing the facetowards while we are waiting for the attack to complete 
			pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
			_pWD->pMover->FaceToward( pT->m_vOverrideAttackLocation );

			// Wait for the attack to finish
			if( pCorrosive->GetBehaviorState() == CBotCorrosive::BEHAVIOR_STATE_NONE ) {
				pT->m_eAttackLocationState = CCorrosiveCombat::ATTACKLOCATION_STATE_ATTACKCOMPLETE;
			}
			break;
	
		case CCorrosiveCombat::ATTACKLOCATION_STATE_ATTACKCOMPLETE:
			// If we are here, we just finished the attack.
			// We should request a behavior command, letting the system know we just completed the attack.
			pT->m_eOverrideState = CCorrosiveCombat::OVERRIDE_STATE_NONE;
			bGotExternalBehavior = FALSE;
			if( pT->m_pBehaviorRequestCallbackFcn ) {
				bGotExternalBehavior = pT->m_pBehaviorRequestCallbackFcn( AI_BEHAVIOR_REASON_ATTACK_COMPLETE );
			}
			if( !bGotExternalBehavior ) {
				// We didn't get a commanded behavior, so just go into our base attack state
				pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_BASE);
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "No behavior commanded -- Going to base attack state!\n" );
#endif
			}
			break;
	
		default:
			FASSERT_NOW;
	}

	return AIFSM_WORK_CB_RETURN;
}

BOOL InRules_ARS_Corrosive0_FingerFlick( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic( CGroundCombat::TACTIC_NONE );
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()

	pT->m_eFingerFlickState = CCorrosiveCombat::FINGERFLICK_STATE_GOTOATTACKPOINT;

	pT->m_bAllowRandomRoar = FALSE;

	return AIFSM_INIT_CB_DONE;
}

// This rule set has corrosive attack a specific location.
// The enemy is completely ignored while corrosive is in this mode....
BOOL DoRules_ARS_Corrosive0_FingerFlick( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	CBotCorrosive* pCorrosive = NULL;
	BOOL bGotExternalBehavior;

	// First, cast a pointer to our bot as Corrosive
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}

	// Run through our attack location state machine and issue the attack...
	switch ( pT->m_eFingerFlickState ) {

		case CCorrosiveCombat::FINGERFLICK_STATE_GOTOATTACKPOINT:
		{
			//pT->HandlePathing( FALSE ); // kill our current path
			// First check to see if we are close enough already.
			CFVec3A vPos = _pWD->pMover->GetLoc();
			CFVec3A vGoalLoc;
			CFVec3 vOffs = pT->m_FingerFlickSphere.m_Pos - vPos.v3;
			vGoalLoc.v3 = pT->m_FingerFlickSphere.m_Pos;

			vOffs.y = 0.0f;
			vOffs.UnitizeXZ();
			vGoalLoc.v3.x += pT->m_FingerFlickSphere.m_fRadius*vOffs.z*3.0f;
			vGoalLoc.v3.z -= pT->m_FingerFlickSphere.m_fRadius*vOffs.x*3.0f;

			vOffs = vGoalLoc.v3 - vPos.v3;

			if ( vOffs.MagXZ() < _FINGER_FLICK_THRESHOLD+pT->m_FingerFlickSphere.m_fRadius )
			{
				pT->m_eFingerFlickState = CCorrosiveCombat::FINGERFLICK_STATE_FACEATTACKPOINT;
				_pWD->pMover->StopAndClearCmds();

				//pT->HandlePathing(TRUE);
			}
			else
			{
				_pWD->pMover->MoveToward( vGoalLoc );
				//pT->PathSetDestination( &vGoalLoc );
			}
			break;
		}
		case CCorrosiveCombat::FINGERFLICK_STATE_FACEATTACKPOINT:
			// Here, we want to face the location we want to attack!
			pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
			if( _pWD->pMover->FaceToward( pT->m_vOverrideAttackLocation ) ) {
				// We are facing the proper direction... Move on!
				pT->m_eFingerFlickState = CCorrosiveCombat::FINGERFLICK_STATE_ISSUEATTACK;
			}
			break;
	
		case CCorrosiveCombat::FINGERFLICK_STATE_ISSUEATTACK:
			// Wait until we successfully have requested the attack
			if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_FINGER_FLICK ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "Finger Flickin'\n" );
#endif
				pT->m_eFingerFlickState = CCorrosiveCombat::FINGERFLICK_STATE_WAITFORATTACKTOCOMPLETE;
			}
			break;
	
		case CCorrosiveCombat::FINGERFLICK_STATE_WAITFORATTACKTOCOMPLETE:

			if( pCorrosive->IsFingerFlicked() )
			{
				//Fire off my callback... I'm done with the attack.
				if ( pT->m_pAttackCompleteCallback )
				{
					pT->m_pAttackCompleteCallback(pT->m_nCallbackUserData);
				}
			}

			// Wait for the attack to finish
			if( pCorrosive->GetBehaviorState() == CBotCorrosive::BEHAVIOR_STATE_NONE ) {
				pT->m_eFingerFlickState = CCorrosiveCombat::FINGERFLICK_STATE_ATTACKCOMPLETE;
			}
			break;
	
		case CCorrosiveCombat::FINGERFLICK_STATE_ATTACKCOMPLETE:
			//Revert back to our base attack state at this point
			pT->m_eOverrideState = CCorrosiveCombat::OVERRIDE_STATE_NONE;

			bGotExternalBehavior = FALSE;
			if( pT->m_pBehaviorRequestCallbackFcn ) {
				bGotExternalBehavior = pT->m_pBehaviorRequestCallbackFcn( AI_BEHAVIOR_REASON_FINGERFLICK_COMPLETE );
			}
			if( !bGotExternalBehavior ) {
				// We didn't get a commanded behavior, so just go into our base attack state
				pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_BASE);
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "No behavior commanded -- Going to base attack state!\n" );
#endif
			}

			break;
	
		default:
			FASSERT_NOW;
	}

	return AIFSM_WORK_CB_RETURN;
}

u32 _uStartFailureCount;

BOOL InRules_ARS_Corrosive0_PeerHere( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic( CGroundCombat::TACTIC_NONE );
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()

	pT->m_ePeerHereState = CCorrosiveCombat::PEERHERE_STATE_GOTOPOINT;
	pT->PathSetDestination( & pT->m_vPeerHereLocation );

	_uStartFailureCount = pT->m_uRangePathFailureCount;

	pT->m_bAllowRandomRoar = FALSE;

	return AIFSM_INIT_CB_DONE;
}

// This rule set has corrosive attack a specific location.
// The enemy is completely ignored while corrosive is in this mode....
BOOL DoRules_ARS_Corrosive0_PeerHere( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	CBotCorrosive* pCorrosive = NULL;
	BOOL bGotExternalBehavior = FALSE;
	f32 fMagXZ;

	CFVec3A vTemp;
	// First, cast a pointer to our bot as Corrosive
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}

	// If we establish line of sight at any point in time, bail out of this
	// ruleset and go back to the base attack state (assuming we are allowed to)!
	if ( pT->m_bAllowEnemyLOSToAbortOverride && ( (u16) pT->m_fTimeWithoutLOS < ( _pWD->uNowTime - pT->m_uTacticInitTime ) ) ) {
		pT->HandlePathing( TRUE ); // kill our current path
 		pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_BASE);
#if _OUTPUT_DEBUG_MESSAGES == 1
		DEVPRINTF( "LOS Re-Established -- going back to base ruleset!\n" );
#endif
	}

#if _OUTPUT_DEBUG_MESSAGES == 1
		fdraw_DevSphere( &pT->m_vPeerHereLocation.v3, 5.0f );
#endif

	// Run through our attack location state machine and issue the attack...
	switch ( pT->m_ePeerHereState ) {
		case CCorrosiveCombat::PEERHERE_STATE_GOTOPOINT:
			pT->HandlePathing( FALSE );

			// Check to see if we are close enough
			vTemp.Sub( pCorrosive->MtxToWorld()->m_vPos, pT->m_vPeerHereLocation );
			fMagXZ = vTemp.MagXZ();
			if( vTemp.MagXZ() <= 20.0f ) {
				// Too close... Abort the peer
				pT->HandlePathing( TRUE ); // kill our current path
				pT->m_ePeerHereState = CCorrosiveCombat::PEERHERE_STATE_PEERCOMPLETE;
			} else if( fMagXZ <= 50.0f ) {
				// We are close enough..
				pT->HandlePathing( TRUE ); // kill our current path
				pT->m_ePeerHereState = CCorrosiveCombat::PEERHERE_STATE_FACEPEERPOINT;
			}
			else if ( pT->m_uRangePathFailureCount - _uStartFailureCount > 20 ) {
				pT->m_ePeerHereState = CCorrosiveCombat::PEERHERE_STATE_PEERCOMPLETE;
			}
			break;
	
		case CCorrosiveCombat::PEERHERE_STATE_FACEPEERPOINT:
			// Here, we want to face the location we want to peer at
			pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
			if( _pWD->pMover->FaceToward( pT->m_vPeerHereLocation ) ) {
				// We are facing the proper direction... Move on!
				pT->m_ePeerHereState = CCorrosiveCombat::PEERHERE_STATE_ISSUEPEER;
			}
			break;
	
		case CCorrosiveCombat::PEERHERE_STATE_ISSUEPEER:

			// Keep issuing the facetowards while we are waiting for the peer be to issued
			pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
			_pWD->pMover->FaceToward( pT->m_vPeerHereLocation );

			// Wait until we successfully have requested the peer
			if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_PEER_UNDER ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "Peering!\n" );
#endif
				pT->m_ePeerHereState = CCorrosiveCombat::PEERHERE_STATE_WAITFORPEERTOCOMPLETE;
			}
			break;
	
		case CCorrosiveCombat::PEERHERE_STATE_WAITFORPEERTOCOMPLETE:

			// Keep issuing the facetowards while we are waiting for the peer to complete 
			pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
			_pWD->pMover->FaceToward( pT->m_vPeerHereLocation );

			// Wait for the peer to finish
			if( pCorrosive->GetBehaviorState() == CBotCorrosive::BEHAVIOR_STATE_NONE ) {
				pT->m_ePeerHereState = CCorrosiveCombat::PEERHERE_STATE_PEERCOMPLETE;
			}
			break;
	
		case CCorrosiveCombat::PEERHERE_STATE_PEERCOMPLETE:

			// If we are here, we just finished the peer.
			// We should request a behavior command, letting the system know we just completed the peer.
			pT->m_eOverrideState = CCorrosiveCombat::OVERRIDE_STATE_NONE;
			bGotExternalBehavior = FALSE;
			if( pT->m_pBehaviorRequestCallbackFcn ) {
				bGotExternalBehavior = pT->m_pBehaviorRequestCallbackFcn( AI_BEHAVIOR_REASON_PEER_COMPLETE );
			}
			if( !bGotExternalBehavior ) {
				// We didn't get a commanded behavior, so just go into our base attack state
				pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_BASE);
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "No behavior commanded -- Going to base attack state!\n" );
#endif
			}
			break;
	
		default:
			FASSERT_NOW;
	}

	return AIFSM_WORK_CB_RETURN;
}


void CCorrosiveCombat::DoWeaponFireWork_Rocket( void ) {

	if ((m_uAttackFlags & FLAG_PANIC_ON) ||
		(s_pWD->pBot && (s_pWD->pBot->IsHopping() || s_pWD->pBot->IsRolling())) ||
		(s_pWD->pEnemyBot && !s_pWD->pEnemyBot->IsTargetable()) ||
		(!m_pEnemy && m_pBrain->GetReactionState() != CAIBrain::REACTIONSTATE_ATTACK_IMPULSE) ||	 //don't blind fire, unless on impulse reactio state
		!s_pWD->pMover->CanFirePrimary() ) {
		return;
	}

	BOOL bFireRocket = FALSE;

	//if 
	//	 recently had LOS	&&
	//   BotLayer weapon system returned that a lock is met, which means the bullet will go within some degrees of where I told it to
	//then
	//   calculate some odds that a shot will be fired
	if (m_pEnemy && ( m_fTimeWithoutLOS < 1.0f ) && s_pWD->pMover->HasTargetLock() ) { // HasTargetLock means that the weapon will fire. Angle is within some internal tolerance to be considered a "lock" circa 20degrees

		u32 uFireOdds = 0;

		f32 fStrategyFireDelayTime;
		if( m_eBaseStrategyState == BASE_STRATEGY_STATE_MELEE_ATTACK ) {
			fStrategyFireDelayTime = _MELEE_ROCKET_FIRE_DELAY_TIME;
		}
		else if ( m_eBaseStrategyState == BASE_STRATEGY_STATE_RANGE_ATTACK ) {
			fStrategyFireDelayTime = _RANGE_ROCKET_FIRE_DELAY_TIME;
		}
		else {
			FASSERT_NOW;
		}

		if ( ( m_uAttackFlags & FLAG_HASTARGETLOS ) && ( m_fTimeSinceLastFiredRockets > fStrategyFireDelayTime*m_fFireDelayMul ) ) {
			// We are not going to fire a rocket unless at least _ROCKET_FIRE_DELAY_TIME seconds have passed...
			// At most, 10 seconds will go by without firing a rocket!
			uFireOdds = 25 + (u32) ( ( m_fTimeSinceLastFiredRockets - fStrategyFireDelayTime*m_fFireDelayMul ) * 10.0f );
			uFireOdds = (u32)( uFireOdds * m_fFireOddsMul );
		} else if ( m_fTimeWithoutLOS < 5.0f ) {
			// Five percent chance of firing if we recently lost LOS
			uFireOdds = 5;
		}

		if ( fmath_RandomChoice( 100 ) < uFireOdds ) {
			bFireRocket = TRUE;
		} else {
			bFireRocket = FALSE;
		}
	}

	if ( bFireRocket && m_bFreeToFireRockets ) {  
		CBotCorrosive* pCorrosive = NULL;

		// First, cast a pointer to our bot as Corrosive
		if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
			pCorrosive = (CBotCorrosive*) s_pWD->pBot;
		}

		if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_CHEST_ROCKETLAUNCH ) ) {
			// Request to fire a rocket was successfull,
			// Reset the amount of time that has passed since rockets have been fired...
			m_fTimeSinceLastFiredRockets = 0.0f; 
#if _OUTPUT_DEBUG_MESSAGES == 1
			DEVPRINTF( "Rocket Firing!\n" );
#endif
		}
	}		
}


void CCorrosiveCombat::DoWeaponFireWork_Melee( void ) {

	if ((s_pWD->pBot && (s_pWD->pBot->IsHopping() || s_pWD->pBot->IsRolling())) ||
		(s_pWD->pEnemyBot && !s_pWD->pEnemyBot->IsTargetable()) ||
		m_pBrain->IsTalking() ||
		(!m_pEnemy && m_pBrain->GetReactionState() != CAIBrain::REACTIONSTATE_ATTACK_IMPULSE) ||	 //don't blind fire, unless on impulse reactio state
		!s_pWD->pMover->CanFirePrimary() ) {
		return;
	}

	//if 
	//	 recently had LOS	&&
	//   BotLayer weapon system returned that a lock is met, which means the bullet will go within some degrees of where I told it to
	//then
	//   calculate some odds that a shot will be fired
	u32 uFireOdds = 0;
	if ( m_pEnemy && ( m_fTimeWithoutLOS < 1.0f ) && s_pWD->pMover->HasTargetLock() ) { // HasTargetLock means that the weapon will fire. Angle is within some internal tolerance to be considered a "lock" circa 20degrees
		// Check to see if we are within stomping distance!
		uFireOdds = 100;
		if( s_pWD->fDistToEnemyMarkXZ <= m_fMeleeRange[m_ePreferedMeleeAttack] ) {
			uFireOdds = 90;
		} else {
			// don't do the stomp.
			uFireOdds = 0;
		}

	} else if (!m_pEnemy) {
		// RAFHACK -- Perhaps put a roar here instead!
		uFireOdds = 0;
	} else if ( m_pEnemy && !s_pWD->pEnemyBot ) { //enemy is not a bot
		uFireOdds = 0;
	}

	u32 uRandomFireOdds = fmath_RandomChoice( 100 );

	if( uFireOdds > uRandomFireOdds ) { 
		// Now, fire if we need to
		// Capture the value and request the appropriate behavior!
		CBotCorrosive* pCorrosive = NULL;

		// First, cast a pointer to our bot as Corrosive
		if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
			pCorrosive = (CBotCorrosive*) s_pWD->pBot;
		}

		// Figure out where our enemy is...  If our enemy is at chest altitude, then use the
		// first smash, otherwise use the foot stomp!
		if( ( m_pEnemy->MtxToWorld()->m_vPos.y - pCorrosive->MtxToWorld()->m_vPos.y ) > 40.0f ) {
			if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_FIST_SMASH ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
				DEVPRINTF( "Fist Smashing!\n" );
#endif
			}

		} else {
			// Check to see what type of stomp we should perform!
			if ( m_ePreferedMeleeAttack == PREFERED_MELEE_STOMP ) {
				if( s_pWD->fDistToEnemyMarkXZ < 15.0f ) {
					// Do the foot stomp
					if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_STOMP ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
						DEVPRINTF( "Stomping!\n" );
#endif
					}
				} else if( s_pWD->fDistToEnemyMarkXZ < 30.0f ){
					// We are inside of 30 feet, do the forward stomp...
					if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_FORWARD_STOMP ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
						DEVPRINTF( "Forward Stomping!\n" );
#endif
					}
				}
			}
			else if ( m_ePreferedMeleeAttack == PREFERED_MELEE_SWIPE ) {
				// Swat at the enemy.
				if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_SWAT ) ) {
#if _OUTPUT_DEBUG_MESSAGES == 1
					DEVPRINTF( "Swatin' at flies!\n" );
#endif
				}
			}
		}
	}
}


void CCorrosiveCombat::BeatChestWork( void ) {
	CBotCorrosive* pCorrosive = NULL;
	u32 uRoarOdds = 0, uLaughOdds = 0;

	// First, cast a pointer to our bot as Corrosive
	if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) s_pWD->pBot;
	}
	FASSERT( pCorrosive );

	// If we are not allowed to randomly roar at this time, and we have
	// not been explicitly ordered to roar from an external source, then bail baby.
	if( !m_bAllowRandomRoar && !m_bRoarThisFrame && !m_bLaughThisFrame ) {
		return; 
	}

	// Do some Housekeeping!
	m_fTimeSinceLastBeatChest += FLoop_fPreviousLoopSecs;
	m_fTimeSinceLastCheckedToBeatChest += FLoop_fPreviousLoopSecs;

	// Set up some internal boolean variables:
	BOOL bTakeRandomChance = FALSE;
	BOOL bSetRoarBoolean = FALSE;

	if( m_fTimeWithLOS > 0.05 ) {
		m_bRoaredBecauseNoLOS = FALSE;
	}

	//
	// Look at the conditions that could trigger an angry roar
	//
	
	// Check to see how long the player has been out of sight.
	if( ( m_fTimeWithoutLOS > _NO_LOS_ROAR_TIME ) && !m_bRoaredBecauseNoLOS ) {
		// Can't find the player... Lets get pissed!
		bTakeRandomChance = TRUE;
		uRoarOdds += 100; //set a 100 percent chance that the roar will happen!
		bSetRoarBoolean = TRUE;
	}

	if( m_bRoarThisFrame ) {
		bTakeRandomChance = TRUE;
		uRoarOdds += 100; //set a 100 percent chance that the roar will happen!
	}

	if( m_fTimeSinceLastCheckedToBeatChest > _BEAT_CHEST_CHECK_DELTA ) {
		bTakeRandomChance = TRUE;
	}

	//
	// Now, if we want to see if we should roar, nows the time to do it!
	//

	// Check to see if we want to beat our chest every twenty seconds.
	if( !m_pBrain->IsTalking() && bTakeRandomChance && ( pCorrosive->GetBehaviorState() == CBotCorrosive::BEHAVIOR_STATE_NONE ) ) {

		// Figure out the odds of beating our chest...
		uRoarOdds += (u32) m_fTimeSinceLastBeatChest;
		uRoarOdds += (u32) ( ( m_fAggressiveness - 1.0f ) * 20.0f );

		u32 uRandomOdds = fmath_RandomChoice( 100 );
	
		if( uRoarOdds > uRandomOdds ) {
			if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_BEAT_CHEST_AND_ROAR ) ) {
				// Reset the timer and clear the roar this frame flag since we already roared
				m_fTimeSinceLastBeatChest = 0.0f;
				m_bRoarThisFrame = FALSE;
				if( bSetRoarBoolean ) {
					// We are here because we were roaring due to lack of LOS...
					m_bRoaredBecauseNoLOS = TRUE;
				}
 
				if (m_pRoarCallback) { 
					m_pRoarCallback(0);
				}
			}
		}

		m_fTimeSinceLastCheckedToBeatChest = 0.0f;
	}

	// Look at some conditions that could trigger a laugh!
	bTakeRandomChance = FALSE;
	if( m_bLaughThisFrame ) {
		bTakeRandomChance = TRUE;
		uLaughOdds += 100; //set a 100 percent chance that the laugh will happen!
	}
	if( bTakeRandomChance && ( pCorrosive->GetBehaviorState() == CBotCorrosive::BEHAVIOR_STATE_NONE ) ) {

		u32 uRandomOdds = fmath_RandomChoice( 100 );
	
		if( uLaughOdds > uRandomOdds ) {
			if( pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_BEAT_CHEST_AND_LAUGH ) ) {
				// Reset the timer and clear the roar this frame flag since we already roared
				m_fTimeSinceLastBeatChest = 0.0f;
				m_bLaughThisFrame = FALSE;

				if (m_pRoarCallback) { 
					m_pRoarCallback(1);
				}
			}
		}
		m_fTimeSinceLastCheckedToBeatChest = 0.0f;
	}
}


void CCorrosiveCombat::AttackThisLocation( CFVec3A *pLocationToAttack, BOOL bLOSOverride ) {
	m_pCorrosiveCombat->m_bNewOverrideState = TRUE;
	m_pCorrosiveCombat->m_eOverrideState = OVERRIDE_STATE_ATTACK_LOCATION;
	m_pCorrosiveCombat->m_vOverrideAttackLocation = *pLocationToAttack;
	m_pCorrosiveCombat->m_bAllowEnemyLOSToAbortOverride = bLOSOverride;	
}


void CCorrosiveCombat::PeerAtThisLocation( CFVec3A *pLocationToPeerAt, BOOL bLOSOverride ) {

	if (!CGroundCombat::s_pWD) { return; }

	m_pCorrosiveCombat->m_bNewOverrideState = TRUE;
	m_pCorrosiveCombat->m_eOverrideState = OVERRIDE_STATE_PEER_HERE;
	m_pCorrosiveCombat->m_vPeerHereLocation = *pLocationToPeerAt;
	m_pCorrosiveCombat->m_bAllowEnemyLOSToAbortOverride = bLOSOverride;	
}


void CCorrosiveCombat::DoFingerFlick(const CFSphere& rSphere, u32 nUserData, AttackCompleteCallback_t pAttackCallback) {

	if (!CGroundCombat::s_pWD) { return; }

	if ( m_pCorrosiveCombat->m_eOverrideState != OVERRIDE_STATE_FINGER_FLICK ) {
		m_pCorrosiveCombat->m_bNewOverrideState = TRUE;
		m_pCorrosiveCombat->m_eOverrideState = OVERRIDE_STATE_FINGER_FLICK;

		m_pCorrosiveCombat->m_nCallbackUserData = nUserData;
		m_pCorrosiveCombat->m_pAttackCompleteCallback = pAttackCallback;
		m_pCorrosiveCombat->m_FingerFlickSphere = rSphere; 

		m_pCorrosiveCombat->m_vOverrideAttackLocation.v3 = rSphere.m_Pos;
	}
}

void CCorrosiveCombat::CancelFingerFlick() {

	if (!CGroundCombat::s_pWD) { return; }

	if ( m_pCorrosiveCombat->m_eOverrideState == OVERRIDE_STATE_FINGER_FLICK ) {
		if ( m_pCorrosiveCombat->m_eFingerFlickState == FINGERFLICK_STATE_GOTOATTACKPOINT ) {
			m_pCorrosiveCombat->m_eFingerFlickState = FINGERFLICK_STATE_ATTACKCOMPLETE;
		}
	}
}

void CCorrosiveCombat::CancelPeer(BOOL bForce) {
	if ( m_pCorrosiveCombat->m_eOverrideState == OVERRIDE_STATE_PEER_HERE ) {
		//Cancel the peer only if I'm not actually doing the peer animation.
		CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
		CBotCorrosive* pCorrosive = NULL;

		if (!_pWD) { return; }

		// First, cast a pointer to our bot as Corrosive
		if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
			pCorrosive = (CBotCorrosive*) _pWD->pBot;
		}

		if (pCorrosive->GetBehaviorState() != CBotCorrosive::BEHAVIOR_STATE_PEER_UNDER || bForce) {
			m_pCorrosiveCombat->m_ePeerHereState = PEERHERE_STATE_PEERCOMPLETE;
		}
	}
}

void CCorrosiveCombat::SetPreferredMeleeAttack(PreferredMeleeAttack_e eAttack) {
	m_pCorrosiveCombat->m_ePreferedMeleeAttack = eAttack;
}

void CCorrosiveCombat::SetRoarCallback(AttackCompleteCallback_t pRoarCallback) {
	m_pCorrosiveCombat->m_pRoarCallback = pRoarCallback;
}

void CCorrosiveCombat::SetRangeNoRocketsCallback(AttackCompleteCallback_t pRangeNoRocketsCallback) {
	m_pCorrosiveCombat->m_pRangeNoRocketsCallback = pRangeNoRocketsCallback;
}

void CCorrosiveCombat::PathSetDestination( CFVec3A *pNewDestination ) {

	if( *pNewDestination != m_vPathDestination ) {
		m_vPathDestination = *pNewDestination;
		m_bNewPathDestination = TRUE;
	}
}

void CCorrosiveCombat::HandlePathing( BOOL bStop ) {
	u8 uNextPathSlot;
	u8 uCurrPathSlot;

	if ( bStop ) { // Free up our path resources
		if (IsFollowingPath( TRUE, m_uCurrPathReason ) ) {
			FindRequestSlotInUse( m_uCurrPathReason, &uCurrPathSlot );
			FreeRequestSlot( uCurrPathSlot );
		}
		if ( IsFollowingPath( TRUE, m_uNextPathReason ) ) {
			FASSERT(0); // why'd this state happen?
			FindRequestSlotInUse( m_uNextPathReason, &uNextPathSlot );
			FreeRequestSlot( uNextPathSlot );
		}
		return;
	}

	BOOL bRequested = FALSE;
	BOOL bPathBusy = FindRequestSlotInUse( m_uCurrPathReason );
	if ( bPathBusy ) { // means we're on it
		m_fTimeSpentOnPath += FLoop_fPreviousLoopSecs;
	} else { // we're not on the current path
		bRequested = RequestSearch( m_vPathDestination, m_uCurrPathReason, 1.0f + ( (f32)m_uRangePathFailureCount ) );
	}
	
	if ( !bRequested && m_bNewPathDestination && ( m_fTimeSpentOnPath > _NEW_PATH_REQUEST_TIME ) ) { // request next path
		if ( FindRequestSlotInUse( m_uNextPathReason ) == FALSE ) {
			bRequested = RequestSearch( m_vPathDestination, m_uNextPathReason, 1.0f+( (f32)m_uRangePathFailureCount ) );
		}
	}

	if ( !IsFollowingPath( TRUE, m_uCurrPathReason ) ) {
		// not on a path?
		if ( IsSearchDone( m_uCurrPathReason, &uCurrPathSlot ) ) {
			Attack_AssignPath(uCurrPathSlot);
			m_fTimeSpentOnPath = 0.0f;
		} else if ( IsSearchFailed( m_uCurrPathReason,&uCurrPathSlot ) ) {
			u16 uPathStatus = m_aSearchQuery[ uCurrPathSlot ].GetStatus();
			m_uRangePathFailureCount++;
			FreeRequestSlot( uCurrPathSlot );
			// if we can't find our next path, try, try again...
			bRequested = RequestSearch( m_vPathDestination, m_uCurrPathReason, 1.0f+( (f32)m_uRangePathFailureCount ) );
		}
	}
	
	if ( IsSearchDone( m_uNextPathReason, &uNextPathSlot ) ) { // implies success
		if ( FindRequestSlotInUse(m_uCurrPathReason, &uCurrPathSlot) ) {
			FreeRequestSlot(uCurrPathSlot);
		}
		Attack_AssignPath(uNextPathSlot);
		m_fTimeSpentOnPath = 0.0f;
		u8 temp = m_uCurrPathReason;
		m_uCurrPathReason = m_uNextPathReason;
		m_uNextPathReason = temp;
	} else if ( IsSearchFailed( m_uNextPathReason,&uNextPathSlot ) ) {
		u16 uPathStatus = m_aSearchQuery[uNextPathSlot].GetStatus();
		m_uRangePathFailureCount++;
		FreeRequestSlot(uNextPathSlot);
		// if we can't find our next path try again, if not searching on 1,,
		if (!bRequested) {
			bRequested = RequestSearch( m_vPathDestination, m_uNextPathReason, 1.0f+((f32)m_uRangePathFailureCount ) );
		}
	}
	
	if ( m_bPathJustCompleted ) {
		if ( FindRequestSlotInUse( m_uCurrPathReason, &uCurrPathSlot ) ) {
			FreeRequestSlot(uCurrPathSlot);
		}
	}
}

#if _OUTPUT_DEBUG_MESSAGES == 1
static CFVec3A _InvestigateMarkPt;
#endif

BOOL CCorrosiveCombat::FindInvestigatePt()
{
	CGroundCombatWorkContext* _pWD = s_pWD;
	u8 uAttackPtSlot =0;
	CFVec3A vSearchDir, vSearchLoc;

	if( !m_pEnemy ) {
		return TRUE;
	}
	// First, cast a pointer to our bot as Corrosive
	CBotCorrosive* pCorrosive = NULL;
	if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) s_pWD->pBot;
	}

	//if
	//		No Path ready to a new attack pt
	//      
	//then
	//
	//		try to find a new Enemy Relative attack pt
	//
	if ( !FindRequestSlotInUse( CGroundCombat::SEARCHREASON_NEWATTACKPOS ) ) {

		// Build and unitize the rotated vector
		vSearchDir.Sub( _pWD->EnemyMark, _pWD->pMover->GetLoc() );
		f32 fMag = vSearchDir.MagSq();
		if( fMag > 0.00001f ) {
			vSearchDir.Mul( fmath_InvSqrt( fMag ) );
		} else {
			vSearchDir = CFVec3A::m_UnitAxisY;
		}

		// Only do 12 raycasts a frame till we find one...
		for( u32 i = 0; i < 12; i++ ) {

			// Compose the new search location here...
			vSearchLoc.ReceiveRotationY( vSearchDir, m_aSinTable[ m_uIMPC ], m_aCosTable[ m_uIMPC ] );
			vSearchLoc.Mul( m_fIMPD );
			vSearchLoc.Add( _pWD->EnemyMarkTagPos );
			vSearchLoc.y = pCorrosive->MtxToWorld()->m_vPos.y + 50.0f;  // Approximate height of corrosives head...

			m_bGoDirectlyToRangeAttack = ( m_fIMPD >= 20.0f );

			m_uIMPC++;
			if( m_uIMPC == 24 ) {
				m_uIMPC = 0;
				m_fIMPD += 10.0f;
			}
			if( m_fIMPD == 160.0f ) {
				m_fIMPD = 0.0f;
			}

			BOOL bHasLOS = !aiutils_IsLOSObstructed_IgnoreBots( vSearchLoc, *s_pWD->pEnemyBot->GetTagPoint( 0 ), s_pWD->pEnemyBot );

			if( bHasLOS ) {
				// We have found a point that has LOS, so lets see if we can get a path there...

				vSearchLoc.y -= 50.0f;  // remove the Approximate height of corrosives head...
#if _OUTPUT_DEBUG_MESSAGES == 1
				_InvestigateMarkPt = vSearchLoc;
#endif
				RequestSearch(vSearchLoc, CGroundCombat::SEARCHREASON_NEWATTACKPOS, 5.0f );
				break;
			}
		}
	
	}

	//if
	//		an attack pt path fails || 
	//      bot is stuck
	//then
	//		reset clear the search slot
	//		reset the new attack pt selection vbls
	//
	if (IsSearchFailed(CGroundCombat::SEARCHREASON_NEWATTACKPOS) ||
		( _pWD->pMover->IsStuck( 3.0f ))
		)
	{
		if (FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			FreeRequestSlot(uAttackPtSlot);
		}
	}

	//if    
	//		just completed an attackpt path
	//then
	//      reset the attack path frequency timers and such
	//
	if (m_bPathJustCompleted &&
		m_nLastPathReason == CGroundCombat::SEARCHREASON_NEWATTACKPOS)
	{
//		if (pbNewPtReached)
//		{
//			*pbNewPtReached = TRUE;
//		}

		if (FindRequestSlotInUse(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			FreeRequestSlot(uAttackPtSlot);
		}
	}

	//if
	//		an attack pt path is ready && 
	//      not currently following attack pt
	//then
	//		follow the new tc path

	if (!IsFollowingPath(TRUE, CGroundCombat::SEARCHREASON_NEWATTACKPOS))
	{
		if (IsSearchDone(CGroundCombat::SEARCHREASON_NEWATTACKPOS, &uAttackPtSlot))
		{
			Attack_AssignPath(uAttackPtSlot);
		}
	}

#if _OUTPUT_DEBUG_MESSAGES == 1
	fdraw_DevSphere( &_InvestigateMarkPt.v3, 5.0f );
#endif
	return TRUE;
}

void CCorrosiveCombat::SetAggressiveness(f32 fAggressive) 
{
	m_fAggressiveness = fAggressive;
	if( m_bUseAlternateAggressionCalculations ) {
		// Jeremeys aggression calculations
		m_fFireDelayMul = 1.0f/m_fAggressiveness; 
		m_fFireDelayMul *= m_fFireDelayMul;
		m_fFireOddsMul = m_fAggressiveness*m_fAggressiveness;
	} else {
		// Russess aggression calculations
		m_fFireDelayMul = 1.0f;
		m_fFireOddsMul = m_fAggressiveness; // Just increase the odds of firing...
	}

	m_fRunSpeed = m_fNormRunSpeed * fAggressive;  // make him run faster...
}


void CCorrosiveCombat::DoCorrosiveTauntWork(void)
{
	CGroundCombatWorkContext* _pWD = s_pWD;
	CAIBrain* pBrain = GetBrain();
	CBotCorrosive* pCorrosive = NULL;
	if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) s_pWD->pBot;
	}


	// Early out if we are in the middle of some form of behavior that has a verbal sound associated with it...
	if( pCorrosive->GetBehaviorState() != CBotCorrosive::BEHAVIOR_STATE_NONE ) {
		return;
	}


	cchar* pszBTAName = NULL;

	// 	  just went on attack  and
	//	  I'm not the first one on pT attack and
	//	  there is a player n enemy
	//	  and can see enemy
    //	  haven't taken any damage
	f32 fTimeSinceNewAttack = aiutils_GetCurTimeSecs() - m_fAttackThoughtInitTime;
	if (	!pBrain->IsTalking() &&
			(fTimeSinceNewAttack > 0.25f && fTimeSinceNewAttack < 0.5f) &&
			s_pWD->pEnemyBot && 
			s_pWD->pEnemyBot->m_nPossessionPlayerIndex != -1 &&
			(m_uAttackFlags & CGroundCombat::FLAG_HASTARGETLOS) &&
			!s_pWD->pDamagedByMem &&
			(pszBTAName = AIBTA_EventToBTAName("P*E_SEES", pBrain)))
	{
		if (ai_TalkModeBegin(pBrain,
						AIBrain_TalkModeCB,
						(void*) pszBTAName,
						pBrain->GetAIMover()->GetEntity(),
						_kuAttackMOdeTalkRequestTimeOutSecs,
						FALSE, // dont bother to stop...  Stopping to taunt is for weenies
						FALSE, // no looking
						0))
		{
			//did it
			int i =2;
		}
	} else if (!pBrain->IsTalking() &&
		!(m_uAttackFlags & CGroundCombat::FLAG_PANIC_ON)  &&
		(AIBTA_GroupLaughTime(pBrain) || (m_uTauntTimeOut < _pWD->uNowTime)) &&
		((m_fDamageDealtPerSec > 0.10f && m_fDamageTakenPerSec < 0.3f) || (m_fDamageTakenPerSec < 0.4f && m_fEnemyDamageTakenPerSec > 0.10f)) &&   //taunt
		s_pWD->pEnemyBot && 
		s_pWD->pEnemyBot->m_nPossessionPlayerIndex != -1 &&
		(m_fTimeWithoutLOS < 0.7f))
	{
		//lay down a taunt, wer're kicking butt and not taking much damage
		if (s_pWD->fDistToEnemyMark > 0.0f &&
			s_pWD->fDistToEnemyMark < (500.0f+s_pWD->pMover->GetRadiusXZ()) &&
			m_pEnemy->MtxToWorld()->m_vFront.Dot(s_pWD->DeltaToEnemyMarkUnit) < -0.75f)
		{
			pszBTAName = AIBTA_EventToBTAName("P*E_TAUN", pBrain);
		}

		if (!pszBTAName)
		{
			pszBTAName = AIBTA_EventToBTAName("P*E_LAFF", pBrain);
		}

		if (pszBTAName)
		{
			if (ai_TalkModeBegin(pBrain,
							AIBrain_TalkModeCB,
							(void*) pszBTAName,
							pBrain->GetAIMover()->GetEntity(),
							_kuAttackMOdeTalkRequestTimeOutSecs,
							FALSE, // No need to STOP
							FALSE, //DON"T LOOK
							0))
			{
				if (m_uTauntTimeMax > m_uTauntTimeMin)
				{
					m_uTauntTimeOut =  aiutils_GetCurTimeSecsU16() + (u16) (m_uTauntTimeMin + (u8) fmath_RandomChoice(m_uTauntTimeMax - m_uTauntTimeMin));
				}
			}
		}
		else
		{
			m_uTauntTimeOut =  aiutils_GetCurTimeSecsU16() + (u16) (1 + (u8) fmath_RandomChoice(2));
		}
	}
	else if (	m_fDamageTakenPerSec >= 0.1f &&  //stops taunting
				pBrain->GetAIMover()->IsTalkingWithAnim() &&
				_pWD->pMover->GetEntity()->TypeBits() & ENTITY_BIT_BOT &&
				((CBot*) _pWD->pMover->GetEntity())->m_pActiveTalkInst &&
				AIBTA_IsBTATaunt(((CBot*) _pWD->pMover->m_pEntity)->m_pActiveTalkInst->GetTalkData()->m_pszSourceFileName))
	{
		ai_TalkModeEnd(pBrain);
	}
}



void CCorrosiveCombat::DoOuchWork(void) {
	CGroundCombatWorkContext* _pWD = s_pWD;
	CAIBrain* pBrain = GetBrain();
	CBotCorrosive* pCorrosive = NULL;
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}
	// Check to see if we lost a limb... If so, then do an OUCH BTA for sure
	if( !m_bPlayLostLimbOuch ) {
		m_bPlayLostLimbOuch = pCorrosive->LostLimbThisFrame();
	}

	cchar* pszBTAName = NULL;

	m_fTimeSinceOuchBTAPlayed += FLoop_fPreviousLoopSecs;

	BOOL bPlayOuchBTA = FALSE;

	if( !pBrain->IsTalking() ) {
		if( m_bPlayLostLimbOuch && ( pszBTAName = AIBTA_EventToBTAName("***_OUCH", pBrain) ) ) {
			bPlayOuchBTA = TRUE;
		}
		else if ( _pWD->pDamagedByMem && !(_pWD->pDamagedByMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED) && 
				 ( m_fTimeSinceOuchBTAPlayed > _OUCH_BTA_TIMEOUT ) &&
				 ( pszBTAName = AIBTA_EventToBTAName("***_OUCH", pBrain) ) ) {
			bPlayOuchBTA = TRUE;
			_pWD->pDamagedByMem->m_uControlFlags |= CAIMemorySmall::CONTROL_FLAG_ACKNOWLEDGED;
		}

	}
	if( bPlayOuchBTA ) {
		if (ai_TalkModeBegin(pBrain,
						AIBrain_TalkModeCB,
						(void*) pszBTAName,
						pBrain->GetAIMover()->GetEntity(),
						_kuAttackMOdeTalkRequestTimeOutSecs,
						FALSE, //DON"T STOP
						(FVid_nFrameCounter & 1), //RANDOM LOOK or DON"T LOOK
						0))
		{
			// Successfully issued the play
			m_bPlayLostLimbOuch = FALSE;
			m_fTimeSinceOuchBTAPlayed = 0.0f;
		}
	}
}


void CCorrosiveCombat::IsEnemyOnMeWork( void ) {
	// This routine determines if the enemy is physically on corrosive, and if so, for how long...
	CGroundCombatWorkContext* _pWD = s_pWD;
	CAIBrain* pBrain = GetBrain();
	CBotCorrosive* pCorrosive = NULL;
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}

	// Run through corrosives children and see if the enemy is one of corrosives children
	if( !pCorrosive->GetChildCount() ) {
		m_fTimeEnemyOnMe = 0.0f;
		return;
	}

	BOOL bEnemyOnMe = FALSE;
	// If we are here, we have children.. see if our enemy is one of them...
	CEntity *pCurrentChild = pCorrosive->GetFirstChild();
	while( pCurrentChild ) {
		if( pCurrentChild == m_pEnemy ) {
			bEnemyOnMe = TRUE;
			break;
		}
		pCurrentChild = pCorrosive->GetNextChild( pCurrentChild );
	}

	if( bEnemyOnMe ) {
		m_fTimeEnemyOnMe += FLoop_fPreviousLoopSecs;
	} else {
		m_fTimeEnemyOnMe = 0.0f;
	}
}


void CCorrosiveCombat::DoAggressionWork( void ) {
	// Aggression modifications
	if ( m_fNormRunSpeed == 0.0f ) {
		m_fNormRunSpeed = s_pWD->pBot->GetRunSpeed();
	} else {
		s_pWD->pBot->SetRunSpeed( m_fRunSpeed );
		s_pWD->pMover->SetForwardGoalSpeedPctCapable( 1.0f ); // I think this is necessary to update ai as to how fast I can go...
	}
}


BOOL InRules_ARS_Corrosive0_GetEnemyOffMe( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();

	pT->ChangeTactic( CGroundCombat::TACTIC_NONE );
	pT->ClearTorsoFlags();
	pT->ClearHeadFlags();
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()

	pT->m_bAllowRandomRoar = FALSE;

	return AIFSM_INIT_CB_DONE;
}

// This rule set has corrosive attack the space inbetween his feet in the hopes of
// flushing the enemy off of him...
BOOL DoRules_ARS_Corrosive0_GetEnemyOffMe( u32 uControl, void* pParam1, void* pParam2 ) {

	CCorrosiveCombat* pT = (CCorrosiveCombat*) pParam1;
	CGroundCombatWorkContext* _pWD = CGroundCombat::s_pWD;
	CAIBrain* pBrain = pT->GetBrain();
	CBotCorrosive* pCorrosive = NULL;
	CFVec3A vTemp;

	// First, cast a pointer to our bot as Corrosive
	if ( _pWD->pBot && ( _pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) _pWD->pBot;
	}

	// Check to see if we should bail out of this state...
	if( pT->m_fTimeEnemyOnMe < 0.5f ) {
		// Bail out.
 		pT->ChangeRuleSetState(CGroundCombat::ARS_CORROSIVE0_BASE);
	}

	// Otherwise, face the ground and fire away (rapidly and without mercy!)
	pT->m_uAttackFlags |= CGroundCombat::FLAG_TORSO_NO_WORK;  // Prevents base class from doing a facetoward()
	_pWD->pMover->FaceToward( _pWD->pBot->MtxToWorld()->m_vPos );
	pCorrosive->RequestBehavior( CBotCorrosive::BEHAVIOR_STATE_CHEST_ROCKETLAUNCH );	

	return AIFSM_WORK_CB_RETURN;
}



//
// Where is Enemy
//	  - the whole point of this, is to
//      prioritize the current stimuli and
//		if possible, return the mount position of where we currently believe
//      the player is
BOOL CCorrosiveCombat::WhereIsEnemy(CAIMemoryLarge* pSeenMem,
								 CAIMemoryLarge* pHeardMem,
								 CAIMemoryLarge* pDamagedByMem,
								 CFVec3A* pEnemyMountPos,
								 BOOL *pbHasLOSThisFrame)
{
	u16 uNowTime = aiutils_GetCurTimeSecsU16();
	BOOL bEnemyMountPosSet = TRUE;
	CAIMover* pMover = m_pBrain->GetAIMover();

	FASSERT(pEnemyMountPos && pbHasLOSThisFrame);
	*pbHasLOSThisFrame = FALSE;


	// We are either going to run off of the eyes or the ears, and we are going to
	// choose the memory which is more recent...
	u16 uTimeSinceSeen = 0xFFFF;
	u16 uTimeSinceHeard = 0xFFFF;

	if( pSeenMem ) {
		uTimeSinceSeen = uNowTime - pSeenMem->m_uWhenTimeSecs;
		if( pSeenMem->m_uControlFlags & CAIMemorySmall::CONTROL_FLAG_NEW_MEMORY ) {
			// Check to see if we have LOS this frame
			*pbHasLOSThisFrame = TRUE;

			m_pLastKnownEnemyVehicle = NULL;
			aiutils_GetCurMech(m_pEnemy, &m_pLastKnownEnemyVehicle);
			if (m_pLastKnownEnemyVehicle && ((CBot*)m_pLastKnownEnemyVehicle)->IsPillBox())
			{   //don't target the vehicle if it is a pillbox
				m_pLastKnownEnemyVehicle = NULL;
			}
		}
	}
	if( pHeardMem ) {
		uTimeSinceHeard = uNowTime - pHeardMem->m_uWhenTimeSecs;
	}

	//Now, choose the memory which is most recent
	if( pSeenMem || pHeardMem ) {
		if( uTimeSinceSeen <= uTimeSinceHeard ) {  // Give sight priority over hearing....
			pEnemyMountPos->Set(pSeenMem->m_EntityLoc);  //enemy position is where I last saw him!

			if (m_pLastKnownEnemyVehicle)
			{   //If I know the player is in a vehicle, then set the mountpos to be the vehicle pos
				pEnemyMountPos->Set(m_pLastKnownEnemyVehicle->MtxToWorld()->m_vPos);
			}

		} else if( uTimeSinceHeard < uTimeSinceSeen ) {
			pEnemyMountPos->Set(pHeardMem->m_EntityLoc);  //enemy position is where I last heard him!

			if (!s_pWD->pMover->CanSee())
			{
				*pbHasLOSThisFrame = TRUE; //run off of ears
			}
		}
	} else {
		// We don't have any sight or audible memories... check damage memories...
		if (	pDamagedByMem &&
				(s_pWD->uNowTime - pDamagedByMem->m_uWhenTimeSecs < 1 ) &&
				pDamagedByMem->m_uWhenTimeSecs > m_uBlindResponseToDamageTimeOut)
		{
			pEnemyMountPos->Set(pDamagedByMem->m_EntityLoc);
			m_uBlindResponseToDamageTimeOut = s_pWD->uNowTime + 3;

			if (!s_pWD->pMover->CanSee())
			{
				*pbHasLOSThisFrame = TRUE; //run off of ears
			}

		}
		else if (m_pEnemy && !(m_pEnemy->TypeBits() & ENTITY_BIT_BOT))
		{
			//cheat when attacking non-bots
			*pEnemyMountPos = m_pEnemy->MtxToWorld()->m_vPos;
			*pbHasLOSThisFrame = TRUE; //run off of ears
			//no Idea where the enemy is
			bEnemyMountPosSet = TRUE;
		}
		else
		{
			// We can't find squat... bummer
			*pEnemyMountPos = this->m_LastEnemyMark;
			//no Idea where the enemy is
			bEnemyMountPosSet = FALSE;
		}
	}

	return bEnemyMountPosSet;
}



BOOL CCorrosiveCombat::HasClearRocketShot( void ) {

	BOOL bRetVal = FALSE;
	CFVec3A vMuzzlePt, vLocationToCheck;
	CBotCorrosive* pCorrosive = NULL;

	// First, cast a pointer to our bot as Corrosive
	if ( s_pWD->pBot && ( s_pWD->pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		pCorrosive = (CBotCorrosive*) s_pWD->pBot;
	}

	pCorrosive->ComputeExactMuzzlePoint_WS( &vMuzzlePt );

	//Determine the tag point we want...
	u32 nNumEnemyTagPoints = m_pEnemy->GetTagPointCount();
	if( nNumEnemyTagPoints >= m_nNextTagPointToLOSCheck ) {
		m_nNextTagPointToLOSCheck = 0;
	}
	if( nNumEnemyTagPoints ) {
		vLocationToCheck = *m_pEnemy->GetTagPoint( m_nNextTagPointToLOSCheck );
		m_nNextTagPointToLOSCheck++;
	} else {
		// Just check the enemy mount position
		vLocationToCheck = m_pEnemy->MtxToWorld()->m_vPos;
	}
	bRetVal = !aiutils_IsLOSObstructed_IgnoreBots( vMuzzlePt, vLocationToCheck, pCorrosive );

	return bRetVal;
}