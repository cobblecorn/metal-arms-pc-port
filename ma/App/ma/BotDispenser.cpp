//////////////////////////////////////////////////////////////////////////////////////
// BotDispenser.cpp - Bot Dispenser system
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
// 03/11/03 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BotDispenser.h"
#include "fresload.h"
#include "fparticle.h"
#include "fclib.h"
#include "fscriptsystem.h"
#include "gstring.h"
#include "weapon.h"
#include "MultiPlayermgr.h"
#include "AlarmSys.h"
#include "ai\aibrainman.h"
#include "AI\AiApi.h"
#include "AI\AIEnviro.h"
#include "AI\AIGameUtils.h"
#include "AI\AIMain.h"
#include "AI\AIGraph.h"
#include "AI\AIGroup.h"
#include "AI\AIBrain.h"

#define _LOG_DISPENSER_ACTIVITY		TRUE
#define _MIN_LIGHT_INTENSITY		0.25f
#define _MAX_LIGHT_INTENSITY		2.0f

CBotDispenser::CBotDispenser() {

	m_nState = STATE_UNUSED;
	m_pDispenserEntity = NULL;
	m_pBotInConstruction = NULL;
	m_pMeshBuilder = NULL;
	m_pWatchedBot = NULL;
	m_nBotGUID = 0;
	m_pBeamFxMesh = NULL;
	m_pLight = NULL;

	m_pBuildAnim = NULL;	
	m_hContactParticle = FPARTICLE_INVALID_HANDLE;
	m_hSmokeParticle = FPARTICLE_INVALID_HANDLE;	

	m_pArmSndGrp = NULL;
	m_pBeamSndGrp = NULL;
	m_pPartsSndGrp = NULL;
	m_pDingSndGrp = NULL;
	m_pArmSound = NULL;
	m_pBeamSound = NULL;		
}

CBotDispenser::~CBotDispenser() {
	Uninit();
}

void CBotDispenser::Uninit() {
	m_nState = STATE_UNUSED;

	if( m_pBuildAnim ) {
		fdelete( m_pBuildAnim );
		m_pBuildAnim = NULL;
	}
	if( m_pAnimCombiner ) {
		fdelete( m_pAnimCombiner );
		m_pAnimCombiner = NULL;
	}
	if( m_pBeamFxMesh ) {
		fdelete( m_pBeamFxMesh );
		m_pBeamFxMesh = NULL;
	}
	if( m_pArmSound ) {
		m_pArmSound->Destroy();
		m_pArmSound = NULL;
	}
	if( m_pBeamSound ) {
		m_pBeamSound->Destroy();
		m_pBeamSound = NULL;
	}	
}

void CBotDispenser::SetInitDataToDefaults( BotDispenser_Init_t *pInit ) {

	fang_MemZero( pInit, sizeof( BotDispenser_Init_t ) );

	pInit->bWaitForBotDeath = TRUE;
	pInit->nBotsPerSession = 1;
	pInit->fSecsBetweenSpawns = 5.0f;
	pInit->fSecsBetweenSession = 15.0f;
	pInit->bMatchBotNames = FALSE;

	pInit->fMinBeamScale = 1.0f;
	pInit->fMaxBeamScale = 2.5f;
	pInit->fSecsToMaxBeamScale = (4.0f/60.0f);
	pInit->fSecsToMinBeamScale = (6.0f/60.0f);

	pInit->fSpawnSecs = 3.0f;
	pInit->fUnitPartOverlap = 0.15f;
	
	pInit->fSecsBeforeInitialHandContact = (4.0f/60.0f);
	pInit->fSecsOfFinalArmStow = 1.0f;	

	pInit->fGenFxIntensity = 1.0f;

}

BOOL CBotDispenser::Init( BotDispenser_Init_t *pInitData, CEntity *pDispenserEntity ) {

	FASSERT( m_nState == STATE_UNUSED );
	
	if( !pInitData || !pDispenserEntity ) {
		return FALSE;
	}
	
    m_pDispenserEntity = (CMeshEntity *)pDispenserEntity;

	CFWorldMesh *pWorldMesh = m_pDispenserEntity->GetMeshInst();
	if( !pWorldMesh ) {
		return FALSE;
	}

	if( !InitBoneIndices( pWorldMesh, pInitData ) ) {
		return FALSE;
	}

	if( !InitBotInfo( pInitData ) ) {
		return FALSE;
	}
	
	// misc values
	m_fTimer = 0.0f;
	m_pBotInConstruction = NULL;

	m_nBotsPerSession = pInitData->nBotsPerSession;
	FMATH_CLAMP( m_nBotsPerSession, 1, 0xFFFF );
	
	m_nNumBotsThisSession = 0;
	m_fSecsBetweenSpawns = pInitData->fSecsBetweenSpawns;
	FMATH_CLAMPMIN( m_fSecsBetweenSpawns, 0.0f );

    m_fSecsBetweenSession = pInitData->fSecsBetweenSession;
	FMATH_CLAMPMIN( m_fSecsBetweenSession, 0.0f );

	m_fMinBeamScale = pInitData->fMinBeamScale;
	FMATH_CLAMPMIN( m_fMinBeamScale, 0.1f );

	m_fMaxBeamScale = pInitData->fMaxBeamScale;
	FMATH_CLAMPMIN( m_fMaxBeamScale, m_fMinBeamScale + 0.01f );

	m_fSecsToMaxBeamScale = pInitData->fSecsToMaxBeamScale;
	FMATH_CLAMPMIN( m_fSecsToMaxBeamScale, 0.1f );

	m_fSecsToMinBeamScale = pInitData->fSecsToMinBeamScale;
	FMATH_CLAMPMIN( m_fSecsToMinBeamScale, 0.1f );

	m_fSpawnSecs = pInitData->fSpawnSecs;
	FMATH_CLAMPMIN( m_fSpawnSecs, 1.0f );
	
	m_fUnitPartOverlap = pInitData->fUnitPartOverlap;
	FMATH_CLAMP_UNIT_FLOAT( m_fUnitPartOverlap );

	m_nBotIndexToDispenseNext = 0;
	m_pMeshBuilder = NULL;

	m_pWatchedBot = NULL;;
	m_nBotGUID = 0;
	
	// flags
	m_nFlags = BOT_DISPENSER_FLAGS_NONE;
	if( !pInitData->bWaitForBotDeath ) {
		m_nFlags |= BOT_DISPENSER_FLAGS_PUMP_OUT_BOTS;
	}
	if( pInitData->bRandomizeBots ) {
		m_nFlags |= BOT_DISPENSER_FLAGS_RANDOM_BOTS;
	}
	if( pInitData->bMatchBotNames ) {
		m_nFlags |= BOT_DISPENSER_FLAGS_NAME_MUST_MATCH;
	}
	
	// load the particle effects
	if( m_aBoneIDs[BOT_DISPENSER_BONE_ID_LEFT_ARM] == BOT_DISPENSER_BONE_NOT_FOUND ||
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_RIGHT_ARM] == BOT_DISPENSER_BONE_NOT_FOUND ) {
		// don't screw with particles if both arms weren't found
		m_hContactParticle = FPARTICLE_INVALID_HANDLE;
		m_hSmokeParticle = FPARTICLE_INVALID_HANDLE;
	} else {
		m_hContactParticle = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, pInitData->pszSparkResName );
		m_hSmokeParticle = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, pInitData->pszSmokeResName );
	}
	m_hGenFXParticle = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, pInitData->pszGenParticleFxResName );
	m_hGenFXEmitter = FPARTICLE_INVALID_HANDLE;
	m_fGenFXIntensity = pInitData->fGenFxIntensity;
	FMATH_CLAMP_UNIT_FLOAT( m_fGenFXIntensity );

	// load the beam effect mesh    
	if( m_aBoneIDs[BOT_DISPENSER_BONE_ID_BEAM] == BOT_DISPENSER_BONE_NOT_FOUND ) {
		m_pBeamFxMesh = NULL;
	} else {
		FMesh_t *pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pInitData->pszBeamMeshName );
		if( pMesh ) {
			m_pBeamFxMesh = fnew CFWorldMesh;
			if( m_pBeamFxMesh ) {
				FMeshInit_t MeshInit;

				// init the meshinst
				MeshInit.pMesh = pMesh;
				MeshInit.nFlags = FMESHINST_FLAG_NOCOLLIDE | FMESHINST_FLAG_POSTER_Y;
				MeshInit.fCullDist = pWorldMesh->m_fCullDist;
				MeshInit.Mtx.Set( pWorldMesh->m_Xfm.m_MtxF );

				m_pBeamFxMesh->Init( &MeshInit ); 
				m_pBeamFxMesh->RemoveFromWorld();
			}
		} else {
			m_pBeamFxMesh = NULL;
		}
	}

	// load the animation, setup an animation combiner
	m_pAnimCombiner = NULL;		
	m_pBuildAnim = NULL;
	FAnim_t *pAnim = (FAnim_t *)fresload_Load( FANIM_RESNAME, pInitData->pszArmAnimationResName );
	if( pAnim ) {
		m_pBuildAnim = fnew CFAnimInst;
		if( m_pBuildAnim ) {
			m_pBuildAnim->Create( pAnim );

			// create a combiner so that we can play the arm animation
			m_pAnimCombiner = fnew CFAnimCombiner;
			if( m_pAnimCombiner ) {
				m_pAnimCombiner->CreateSimple( m_pBuildAnim, pWorldMesh );

				// grab some info from the anim data
				m_fArmAnim_SecsOfArmStow = pInitData->fSecsOfFinalArmStow;
				m_fArmAnim_SecsTillFirstContact = pInitData->fSecsBeforeInitialHandContact;
				m_fArmAnim_SecsRegPartDrop = m_pBuildAnim->GetTotalTime() - m_fArmAnim_SecsOfArmStow - m_fArmAnim_SecsTillFirstContact;
				m_fArmAnim_PercentOfAnim1stContact = m_fArmAnim_SecsTillFirstContact * m_pBuildAnim->GetOOTotalTime();
				m_fArmAnim_PercentOfAnimPartDrop = m_fArmAnim_SecsRegPartDrop * m_pBuildAnim->GetOOTotalTime();

				EndArmAnimation();			
			} else {
				fdelete( m_pBuildAnim );
				m_pBuildAnim = NULL;
			}
		}
	}

	// load the sounds
	fresload_Load( FSNDFX_RESTYPE, pInitData->pszSoundBankToLoad );
	m_pArmSndGrp = CFSoundGroup::RegisterGroup( pInitData->pszArmSound );
	m_pBeamSndGrp = CFSoundGroup::RegisterGroup( pInitData->pszBeamSound );
	m_pPartsSndGrp = CFSoundGroup::RegisterGroup( pInitData->pszPartsSound );
	m_pDingSndGrp = CFSoundGroup::RegisterGroup( pInitData->pszDingSound );

	m_pArmSound = NULL;
	m_pBeamSound = NULL;
	
	// find the light and turn it off
	m_pLight = pWorldMesh->GetAttachedLightByID( 1 );
	if( m_pLight ) {
		m_pLight->m_Light.Enable( FALSE );
	}

	// all we need to do is resolve our bot names into bot types
	m_nState = STATE_NEED_TO_RESOLVE;

	aigroup_RegisterForAIAvoidence(m_pDispenserEntity);

	return TRUE;
}

BOOL CBotDispenser::ResolveFixups( CBot **papAutoBots, u32 uNumAutoBots, u32 &rnMaxBotBoneCount ) {
	u32 i, j, nValidTypes;

	FASSERT( m_nState == STATE_NEED_TO_RESOLVE );

	rnMaxBotBoneCount = 0;
	nValidTypes = 0;
	for( i=0; i < m_nNumTypes; i++ ) {
		for( j=0; j < uNumAutoBots; j++ ) {
			if( fclib_stricmp( papAutoBots[j]->Name(), m_apszBotNames[i] ) == 0 ) {
				m_aBotInfo[nValidTypes].nBotType = papAutoBots[j]->m_pBotDef->m_nClass;
				m_aBotInfo[nValidTypes].bTetherPortRequired = papAutoBots[j]->DataPort_IsInstalled();
				m_aBotInfo[nValidTypes].nWeaponType = (papAutoBots[j]->m_apWeapon[0]) ? papAutoBots[j]->m_apWeapon[0]->Type() : 0;
				// record the max bone count
				rnMaxBotBoneCount = FMATH_MAX( rnMaxBotBoneCount, papAutoBots[j]->m_pWorldMesh->m_pMesh->nBoneCount );
				nValidTypes++;
				break;
			}
		}
		if( j == uNumAutoBots ) {
			DEVPRINTF( "CBotDispenser::ResolveFixups() : could not find a bot in the pool named '%s'.\n", m_apszBotNames[i] );
		}
	}
	m_nNumTypes = nValidTypes;
    
	if( m_nNumTypes > 0 ) {
		// set the die callback routine
		m_pDispenserEntity->SetDieCallback( MeshEntityDieCallback, this );
		m_nState = STATE_READY;
		return TRUE;
	}
	return FALSE;
}

void CBotDispenser::TurnOn( CAlarmNet *pNet ) {

	if( m_nState >= STATE_READY ) {
		FMATH_SETBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_ACTIVE );

		if( m_nState == STATE_READY ) {
			// since we are just waiting around, go ahead and pick a bot
            SetupNeedBotVars();	

			// move to standby mode
			m_nState = STATE_STANDBY;
		}
#if _LOG_DISPENSER_ACTIVITY
		DEVPRINTF( "BotDispenser: Turning ON '%s'\n", m_pDispenserEntity->Name() );
#endif
	}
}

void CBotDispenser::TurnOff( CAlarmNet *pNet, BOOL bImmediately/*=FALSE*/ ) {

	if( m_nState >= STATE_READY ) {
		if( bImmediately ) {
			// we have to stop anything that might be happening
			if( m_nState == STATE_SPAWNING ) {
				FASSERT( m_pMeshBuilder );
				FASSERT( m_pBotInConstruction );

				m_pMeshBuilder->ForceFinish();
				m_pBotInConstruction->RelocateAllChildren( FALSE );
				SetBotProperties( FALSE, m_pBotInConstruction );
				m_pBotInConstruction->RemoveFromWorld( TRUE );
				EndArmAnimation();
				EndBeam();
			} else if( m_nState == STATE_RETRACT_ARMS ) {
				EndArmAnimation();
			}
			m_fTimer = 0.0f;	
			m_pBotInConstruction = NULL;
			m_pMeshBuilder = NULL;
			m_nBotIndexToDispenseNext = 0;
			m_nNumBotsThisSession = 0;
			m_pWatchedBot = NULL;
			m_nBotGUID = 0;
			m_nState = STATE_READY;
		}
		FMATH_CLEARBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_ACTIVE | BOT_DISPENSER_FLAGS_NEED_BOT );
#if _LOG_DISPENSER_ACTIVITY
		if( m_pDispenserEntity ) {
			DEVPRINTF( "BotDispenser: Turning OFF '%s'\n", m_pDispenserEntity->Name() );
		}
#endif
	}
}

BOOL CBotDispenser::IsWatchedBotIsDead() {

	if( m_pWatchedBot == NULL ) {
		return TRUE;
	}

	if( !m_pWatchedBot->IsInWorld() || m_pWatchedBot->Guid() != m_nBotGUID ) {
		// the bot was killed and possibly reused
		m_pWatchedBot = NULL;
		m_nBotGUID = 0;
		return TRUE;
	}
	return FALSE;
}

BOOL CBotDispenser::HasWatchedBotExitedTheDispenser() {

	if( m_pWatchedBot == NULL ) {
		return TRUE;
	}

	// grab the maxtrix palette
	CFMtx43A **ppMtxPalette = m_pDispenserEntity->GetMeshInst()->GetBoneMtxPalette();
	
	CFMtx43A *pTarget = ppMtxPalette[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_TARGET] ];
	CFMtx43A *pDrop = ppMtxPalette[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_DROP] ];

	CFVec3A Vec;
	Vec.Sub( pTarget->m_vPos, pDrop->m_vPos );
	Vec.Mul( 0.5f );
	f32 fRadius = Vec.Mag();
	Vec.Add( pDrop->m_vPos );
	CFSphere AreaToClear;
	AreaToClear.Set( Vec.v3, fRadius );

	if( !AreaToClear.IsIntersecting( m_pWatchedBot->m_pWorldMesh->GetBoundingSphere() ) ) {
		// the bounding spheres aren't intersecting anymore
		m_pWatchedBot = NULL;
		m_nBotGUID = 0;
		return TRUE;
	}
	return FALSE;
}

void CBotDispenser::SetWaitTimes() {

	FASSERT( !m_pWatchedBot );// we shouldn't be watching for a bot death at this time

	// see if we have finished this session
	if( m_nNumBotsThisSession >= m_nBotsPerSession ) {
		// finished with a session, start a new on
		m_nNumBotsThisSession = 0;
		m_fSecsToWait = m_fSecsBetweenSession;
	} else {
		// not finished with the current session
		m_fSecsToWait = m_fSecsBetweenSpawns;
	}
	m_fTimer = 0.0f;
}

void CBotDispenser::SetupNeedBotVars() {
	// figure out what bot to dispense
	FASSERT( !m_pMeshBuilder );
	FASSERT( !m_pBotInConstruction );
	
	if( m_nFlags & BOT_DISPENSER_FLAGS_RANDOM_BOTS ) {
		m_nBotIndexToDispenseNext = fmath_RandomChoice( m_nNumTypes );
	} else {
		// already set, every time we spawn we would make this right
	}

	m_fTimer = 0.0f;
    m_fSecsToWait = -1.0f;

	FMATH_SETBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_NEED_BOT );
}

void CBotDispenser::Work( CAlarmNet *pNet ) {
	f32 fUnitPercent;
	BOOL bDone;

	FASSERT( m_nState > STATE_NEED_TO_RESOLVE );

	switch( m_nState ) {
	
	default:
		// do nothing, return
		break;

	case STATE_DESTROYED:
		// keep checking the mesh entity to see if it has come back online
		if( m_pDispenserEntity->NormHealth() > 0.0f ) {
			// we came back to life, move back to the ready state
			m_nState = STATE_READY;

#if _LOG_DISPENSER_ACTIVITY
			DEVPRINTF( "BotDispenser: '%s' come back to life\n", m_pDispenserEntity->Name() );
#endif
		}
		break;

	case STATE_READY:
		// waiting to be turned on
		if( m_nFlags & BOT_DISPENSER_FLAGS_ACTIVE ) {
			m_nNumBotsThisSession = 0;
			SetupNeedBotVars();	

			// move to standby mode
			m_nState = STATE_STANDBY;
		}
		break;

	case STATE_SPAWNING:
		FASSERT( m_pMeshBuilder );
		bDone = m_pMeshBuilder->Work( fUnitPercent );
		// always update the children entities
		m_pBotInConstruction->RelocateAllChildren( FALSE );

		if( bDone ) {
			// finished building the bot
			m_nNumBotsThisSession++;
			m_nBotIndexToDispenseNext++;
			if( m_nBotIndexToDispenseNext >= m_nNumTypes ) {
				// wrap back around to the 1st bot
				m_nBotIndexToDispenseNext = 0;
			}		

			// Put the bot out into the world
			FASSERT( m_pBotInConstruction );
			ConfigureAI( m_pBotInConstruction, m_pDispenserEntity->GetMeshInst()->GetBoundingSphere().m_fRadius );
			SetBotProperties( FALSE, m_pBotInConstruction );
			
			FASSERT( !m_pWatchedBot );
            m_pWatchedBot = m_pBotInConstruction;
			m_nBotGUID = m_pBotInConstruction->Guid();

			m_pBotInConstruction = NULL;
			m_pMeshBuilder = NULL;
            
			// stop the beam animation
			EndBeam();

			// schedule the completed ding sound
			m_bPlayDingSound = TRUE;			

			// update the arm animation time, and set our next state to finish the arm animation
			UpdateArmAnimation( 1.0f );
			m_fTimer = 0.0f;
			m_nState = STATE_RETRACT_ARMS;

		} else {
			// update the arm animation time
			UpdateArmAnimation( fUnitPercent );
            
			// update the beam
			UpdateBeam( fUnitPercent );

			// update the volumes
			UpdateVolumes( fUnitPercent );
		}		
		break;

	case STATE_RETRACT_ARMS:
		m_fTimer += FLoop_fPreviousLoopSecs;
		if( !m_pBuildAnim || m_fTimer >= m_fArmAnim_SecsOfArmStow ) {

			// if the finish bell hasn't played, play it
			if( m_bPlayDingSound && m_pDingSndGrp ) {
				CFSoundGroup::PlaySound( m_pDingSndGrp, FALSE, &m_pDispenserEntity->MtxToWorld()->m_vPos );				
				m_bPlayDingSound = FALSE;
			}

			// stop the arm animtion 
			EndArmAnimation();

			// figure out where to go to
			if( m_nFlags & BOT_DISPENSER_FLAGS_ACTIVE ) {
				// we are still active, move to standby
				m_nState = STATE_STANDBY;

				FASSERT( m_pWatchedBot );
			} else {
				// we are not active anymore
				m_nNumBotsThisSession = 0;
				m_nState = STATE_READY;
			}
		} else {
			// update the arm stow animation
			m_pBuildAnim->DeltaTime( FLoop_fPreviousLoopSecs, TRUE );

			// play the finish building ding sound
			if( m_bPlayDingSound ) {
				if( m_pDingSndGrp && m_fTimer > 0.65f ) {
					CFSoundGroup::PlaySound( m_pDingSndGrp, FALSE, &m_pDispenserEntity->MtxToWorld()->m_vPos );				
					m_bPlayDingSound = FALSE;
				}
			}
		}
		break;

	case STATE_STANDBY:
		if( m_nFlags & BOT_DISPENSER_FLAGS_ACTIVE ) {
			// we are still active

			if( m_pWatchedBot ) {
				if( m_nFlags & BOT_DISPENSER_FLAGS_PUMP_OUT_BOTS ) {
					// we need to wait for the last bot to exit the dispenser
					if( HasWatchedBotExitedTheDispenser() ) {
						SetWaitTimes();
					}
				} else {
					// we are waiting for a bot to die before making any decisions
					if( IsWatchedBotIsDead() ) {
						SetWaitTimes();
					}
				}
			} else {		
				if( m_fSecsToWait >= 0.0f ) {
					m_fTimer += FLoop_fPreviousLoopSecs;
					if( m_fTimer >= m_fSecsToWait ) {
						// times up, pick a bot to dispense and just wait
						SetupNeedBotVars();
					}
				}            
			}
		} else {
			// change states back to STATE_READY
			m_nNumBotsThisSession = 0;
			m_pWatchedBot = NULL;
			m_nBotGUID = 0;
			m_nState = STATE_READY;
		}
		break;
	}	
}

// Returns TRUE If the dispenser wants an autobot
BOOL CBotDispenser::NeedsAutoBot() {

	if( m_nFlags & BOT_DISPENSER_FLAGS_ACTIVE ) {
		if (m_nFlags & BOT_DISPENSER_FLAGS_NEED_BOT) {
			CFSphere SafeArea(m_pDispenserEntity->MtxToWorld()->m_vPos.v3, 5.0f);
			BOOL bDispenseAreaObstructed = !m_pDispenserEntity->IsCollisionFlagSet() && aiutils_DoesSphereIntersectAnyBotsBesidesSkipList(SafeArea);	 //only no collide ones must check for bots hanging out in the dispense area
			return !bDispenseAreaObstructed;
		}
	}
	return FALSE;
}

BOOL CBotDispenser::ShouldBotBeDispensedByThisMachine( CBot *pBot, BotTypeInfo_t *pBotType, cchar *pszNameOfDesiredBot ) {
	
	// First check if this bot can only be dispensed by a specific dispenser
	if( pBot->m_pCanOnlyBeDispensedBy && ( pBot->m_pCanOnlyBeDispensedBy != this ) ){
		// This bot can only be dispensed by a specific dispenser, which is NOT this one.
		return FALSE;
	}

	if( m_nFlags & BOT_DISPENSER_FLAGS_NAME_MUST_MATCH ) {
		// only compare the name
		if( fclib_stricmp( pBot->Name(), pszNameOfDesiredBot ) == 0 ) {
			return TRUE;
		}				
	} else {
		// just compare the type, not the name
		u8 nWeaponType;
		nWeaponType = (pBot->m_apWeapon[0]) ? pBot->m_apWeapon[0]->Type() : 0;
		
		if( pBotType->nBotType == pBot->m_pBotDef->m_nClass &&
			!pBotType->bTetherPortRequired == !pBot->DataPort_IsInstalled() &&
			pBotType->nWeaponType == nWeaponType ) {
			return TRUE;
		}
	}
	return FALSE;
}


// Returns the index into papAutoBots that was used by the dispenser, -1 in none were used
s32 CBotDispenser::DeployAutoBot( CBot **papAutoBots, const u8 *pauAutoBotUseageFlags, u32 uNumAutoBots ) {
	BotTypeInfo_t *pBotType;
	u32 i;
	CBot *pBot;

	if( m_nFlags & BOT_DISPENSER_FLAGS_NEED_BOT ) {
		FASSERT( m_nBotIndexToDispenseNext < m_nNumTypes );
		FASSERT( !m_pBotInConstruction );
		FASSERT( !m_pMeshBuilder );	
	
		pBotType = &m_aBotInfo[m_nBotIndexToDispenseNext];

		// find a bot that matches our required type
		for( i=0; i < uNumAutoBots; i++ ) {
			pBot = papAutoBots[i];
			
			if( !pBot->IsInWorld() && pauAutoBotUseageFlags[i] == 0 ) {
				// this bot is available, see if it is what we are looking for
				if( ShouldBotBeDispensedByThisMachine( pBot, pBotType, m_apszBotNames[m_nBotIndexToDispenseNext] ) ) {
					// we found a match, dispense this bot	

					// grab the maxtrix palette
					CFMtx43A **ppMtxPalette = m_pDispenserEntity->GetMeshInst()->GetBoneMtxPalette();
					
					FASSERT( m_aBoneIDs[BOT_DISPENSER_BONE_ID_DROP] != BOT_DISPENSER_BONE_NOT_FOUND );
					FASSERT( m_aBoneIDs[BOT_DISPENSER_BONE_ID_TARGET] != BOT_DISPENSER_BONE_NOT_FOUND );

					CFMtx43A *pTarget = ppMtxPalette[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_TARGET] ];
					CFMtx43A *pDrop = ppMtxPalette[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_DROP] ];

					if (!pBot->IsInWorld() && 
						CFCheckPoint::SetCheckPoint( 0 ) )	{
						// set checkpoint system state to "load"
						if ( CFCheckPoint::SetLoadState() ) {
						pBot->CheckpointRestore();
						}
					}

						// put the bot into the world and configure it
					CFMtx43A NonScaledMtx;
					NonScaledMtx.UnitMtxFromNonUnitVec( &pTarget->m_vZ );
					NonScaledMtx.m_vPos.Set( pTarget->m_vPos );
					pBot->Relocate_RotXlatFromUnitMtx_WS( &NonScaledMtx, FALSE );

					SetBotProperties( TRUE, pBot );
										
					// grab a mesh builder manager
					m_pMeshBuilder = CFXMeshBuildPartMgr::InitMgr( pDrop->m_vPos,
																	pBot,
																	pBot->m_pWorldMesh,
																	m_fSpawnSecs,
																	m_fUnitPartOverlap );
					if( !m_pMeshBuilder ) {
						// can't grab a mesh builder
						SetBotProperties( FALSE, pBot );
						pBot->RemoveFromWorld( TRUE );
						return -1;
					}
					// make sure we relocate all childern entities
					pBot->RelocateAllChildren( FALSE );

					// start the arm animation
					StartArmAnimation();

					// start the beam animation
					StartBeam();

					// spawn gen fx particles
					m_hGenFXEmitter = fparticle_SpawnEmitter( m_hGenFXParticle, pTarget->m_vPos.v3, NULL, m_fGenFXIntensity );				

					// record the bot in progress & update states and flags
					m_pBotInConstruction = pBot;
					FMATH_CLEARBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_NEED_BOT );
					m_nState = STATE_SPAWNING;
#if _LOG_DISPENSER_ACTIVITY
					DEVPRINTF( "BotDispenser: '%s' creating bot %s (GUID = %d).\n", m_pDispenserEntity->Name(), pBot->Name(), pBot->Guid() );
#endif

					return i;
				}
			}
		}
	}

	// if we are dispensing random bots and we didn't succeed this time, pick a new random bot for next time
	if( m_nFlags & BOT_DISPENSER_FLAGS_RANDOM_BOTS ) {
		m_nBotIndexToDispenseNext = fmath_RandomChoice( m_nNumTypes );
	}

	return -1;
}

void CBotDispenser::SetBotProperties( BOOL bStartingSpawn, CBot *pBot ) {

	if( bStartingSpawn ) {
		// configure pBot for spawning

		// put the bot into the world		

		//pgm added this so that bot gets reset to the state it was in at level load
     	pBot->AddToWorld();
        pBot->DrawEnable( TRUE, TRUE );

		aibrainman_Deactivate( pBot->AIBrain(), FALSE );
		
		m_bRestoreTargertable = pBot->IsTargetable();
        pBot->SetTargetable( FALSE );
		
		m_bRestoreInvincible = pBot->IsInvincible();
		pBot->SetInvincible( TRUE );
		
		m_bRestoreActionable = pBot->IsActionable();		
		pBot->SetActionable( FALSE );

		// set the bot to the first frame of the powerup animation
		pBot->Power_SetState( FALSE, 0.0f );
		//pBot->Power( TRUE, 0.0f, 1.0f );
		pBot->ComputeMtxPalette( FALSE );
		pBot->RelocateAllChildren( FALSE );		

		pBot->SetBotFlag_UnderConstruction();
	} else {
		// the bot is done being built, turn it loose on the world
		aibrainman_Activate( pBot->AIBrain() );
		pBot->SetTargetable( m_bRestoreTargertable );
        pBot->SetInvincible( m_bRestoreInvincible );
        pBot->SetActionable( m_bRestoreActionable );

		if (MultiplayerMgr.IsSinglePlayer())
		{
			pBot->Power( TRUE, 0.0f, 1.0f );
		}
		else
		{   //multiplayer is special: very special
			pBot->InitialTeam(NEUTRAL_TEAM);

			if (MultiplayerMgr.PowerUpUnpossessedBots())
			{
				pBot->ClearBotFlag_PowerDownUntilPosessed();
				pBot->Power( TRUE );
			}
			else
			{	// Just in case...
				pBot->SetBotFlag_PowerDownUntilPosessed();
			}
		}

		pBot->ClearBotFlag_UnderConstruction();

		// stop the gen fx particle fx
		fparticle_StopEmitter( m_hGenFXEmitter );
		m_hGenFXEmitter = FPARTICLE_INVALID_HANDLE;

		TogglePartSound( FALSE );
	}
}

BOOL CBotDispenser::InitBoneIndices( CFWorldMesh *pWorldMesh, BotDispenser_Init_t *pInitData ) {
	s32 nBoneIndex;
	cchar *pszBoneName;
	
	// find the left arm bone index
	pszBoneName = pInitData->apszArmBoneNames[BOT_DISPENSER_BONE_ID_LEFT_ARM];
	if( pszBoneName ) {
        nBoneIndex = pWorldMesh->FindBone( pszBoneName );
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_LEFT_ARM] = (nBoneIndex >= 0) ? nBoneIndex : BOT_DISPENSER_BONE_NOT_FOUND;
	} else {
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_LEFT_ARM] = BOT_DISPENSER_BONE_NOT_FOUND;
	}
	// find the right arm bone index
	pszBoneName = pInitData->apszArmBoneNames[BOT_DISPENSER_BONE_ID_RIGHT_ARM];
	if( pszBoneName ) {
        nBoneIndex = pWorldMesh->FindBone( pszBoneName );
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_RIGHT_ARM] = (nBoneIndex >= 0) ? nBoneIndex : BOT_DISPENSER_BONE_NOT_FOUND;
	} else {
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_RIGHT_ARM] = BOT_DISPENSER_BONE_NOT_FOUND;
	}
	// find the drop bone index
	pszBoneName = pInitData->pszFallingPartsBoneName;
	if( pszBoneName ) {
		nBoneIndex = pWorldMesh->FindBone( pszBoneName );
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_DROP] = (nBoneIndex >= 0) ? nBoneIndex : BOT_DISPENSER_BONE_NOT_FOUND;
	} else {
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_DROP] = BOT_DISPENSER_BONE_NOT_FOUND;
	}
	// find the target bone index
	pszBoneName = pInitData->pszBotPositionBone;
	if( pszBoneName ) {
		nBoneIndex = pWorldMesh->FindBone( pszBoneName );
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_TARGET] = (nBoneIndex >= 0) ? nBoneIndex : BOT_DISPENSER_BONE_NOT_FOUND;
	} else {
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_TARGET] = BOT_DISPENSER_BONE_NOT_FOUND;
	}
	// find the beam bone index
	pszBoneName = pInitData->pszBeamBoneName;
	if( pszBoneName ) {
		nBoneIndex = pWorldMesh->FindBone( pszBoneName );
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_BEAM] = (nBoneIndex >= 0) ? nBoneIndex : BOT_DISPENSER_BONE_NOT_FOUND;
	} else {
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_BEAM] = BOT_DISPENSER_BONE_NOT_FOUND;
	}

	// we must have a target and drop bone index
	if( m_aBoneIDs[BOT_DISPENSER_BONE_ID_TARGET] == BOT_DISPENSER_BONE_NOT_FOUND ||
		m_aBoneIDs[BOT_DISPENSER_BONE_ID_DROP] == BOT_DISPENSER_BONE_NOT_FOUND ) {
		return FALSE;
	}

	return TRUE;
}

BOOL CBotDispenser::InitBotInfo( BotDispenser_Init_t *pInitData ) {
	u32 i;

	m_nNumTypes = 0;
	for( i=0; i < BOT_DISPENSER_MAX_UNIQUE_TYPES_PER_DISPENSER; i++ ) {
		if( pInitData->apszBotNamesToSpawn[i] &&
			fclib_stricmp( pInitData->apszBotNamesToSpawn[i], "none" ) != 0 ) {
			m_apszBotNames[m_nNumTypes] = gstring_Main.AddString( pInitData->apszBotNamesToSpawn[i] );
			m_nNumTypes++;
		}
	}
	return (m_nNumTypes > 0);
}

void CBotDispenser::MeshEntityDieCallback( CMeshEntity *pEntity, void *pUserParam ) {
	CBotDispenser *pDispenser = (CBotDispenser *)pUserParam;

	if( pDispenser ) {
		pDispenser->EntityIsDead();
	}
}

void CBotDispenser::EntityIsDead() {

	if( m_nState >= STATE_READY ) {
#if _LOG_DISPENSER_ACTIVITY
		DEVPRINTF( "BotDispenser: '%s' has been destroyed\n", m_pDispenserEntity->Name() );
#endif
		TurnOff( NULL, TRUE );
		m_nState = STATE_DESTROYED;
	}	
}

void CBotDispenser::StartBeam() {
	CFWorldMesh *pWorldMesh = m_pDispenserEntity->GetMeshInst();

	if( m_pBeamFxMesh ) {
		// move the beam to the bone location
		FASSERT( m_aBoneIDs[BOT_DISPENSER_BONE_ID_BEAM] != BOT_DISPENSER_BONE_NOT_FOUND );

		CFMtx43A *pBeamBone = (pWorldMesh->GetBoneMtxPalette())[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_BEAM] ];
		CFMtx43A NewMtx( *pBeamBone );
		NewMtx.Mul33( m_fMinBeamScale );

		m_pBeamFxMesh->m_Xfm.BuildFromMtx( NewMtx, pWorldMesh->m_Xfm.m_fScaleF * m_fMinBeamScale );
		m_pBeamFxMesh->AddToWorld();
		
		if (m_pDispenserEntity->IsDrawEnabled())
		{
			FMATH_SETBITMASK( m_pBeamFxMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
		else
		{
			FMATH_CLEARBITMASK( m_pBeamFxMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
		m_fBeamTimer = 0.0f;

		ToggleBeamSound( TRUE );

		// turn on the light
		if( m_pLight ) {
			m_pLight->m_Light.SetIntensity( _MIN_LIGHT_INTENSITY );
			m_pLight->m_Light.Enable( TRUE );
		}
	}	
}

void CBotDispenser::UpdateBeam( f32 fUnitDropTime ) {

	if( m_pBeamFxMesh ) {
		CFWorldMesh *pWorldMesh = m_pDispenserEntity->GetMeshInst();
		
		// advance the beam timer
		m_fBeamTimer += FLoop_fPreviousLoopSecs;

		// make sure that the beam timer is in range
		f32 fTotalTime = m_fSecsToMaxBeamScale + m_fSecsToMinBeamScale;
		while( m_fBeamTimer >= fTotalTime ) {
			m_fBeamTimer -= fTotalTime;
		}

		// compute the animated scale
		f32 fScale, fUnitTime, fCompletionFactor, fMinS, fMaxS;
		fCompletionFactor = 0.60f + ( (1.0f - fUnitDropTime) * 0.40f );
		fMinS = m_fMinBeamScale * fCompletionFactor;
		fMaxS = m_fMaxBeamScale * fCompletionFactor;
		if( m_fBeamTimer <= m_fSecsToMaxBeamScale ) {
			// scale up to the max scale
			fUnitTime = fmath_Div( m_fBeamTimer, m_fSecsToMaxBeamScale );
			fScale = FMATH_FPOT( fUnitTime, fMinS, fMaxS );
		} else {
			// scale down to min scale
			fUnitTime = fmath_Div( m_fBeamTimer-m_fSecsToMaxBeamScale, m_fSecsToMinBeamScale );
			fScale = FMATH_FPOT( fUnitTime, fMaxS, fMinS );
		}
		
		// grab the mesh bone location mtx
		CFMtx43A *pBeamBone = (pWorldMesh->GetBoneMtxPalette())[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_BEAM] ];
		CFMtx43A NewMtx( *pBeamBone );
		NewMtx.Mul33( fScale );

		// update the mesh location
		m_pBeamFxMesh->m_Xfm.BuildFromMtx( NewMtx, pWorldMesh->m_Xfm.m_fScaleF * fScale );
		m_pBeamFxMesh->UpdateTracker();

		// update the light
		if( m_pLight ) {
			f32 fUnitScale = fmath_Div( (fScale - fMinS), (fMaxS - fMinS) );
			f32 fIntensity = FMATH_FPOT( fUnitScale, _MIN_LIGHT_INTENSITY, _MAX_LIGHT_INTENSITY );
			if( fUnitDropTime >= 0.85f ) {
				f32 fFadeOff = 1.0f - fUnitDropTime;
				fFadeOff *= (1.0f/(1.0f - 0.85f));
				fIntensity *= fFadeOff;
			}
            m_pLight->m_Light.SetIntensity( fIntensity );
		}
	}
}

void CBotDispenser::EndBeam() {

	if( m_pBeamFxMesh ) {
        m_pBeamFxMesh->RemoveFromWorld();
		ToggleBeamSound( FALSE );

		// turn off the light
		if( m_pLight ) {
			m_pLight->m_Light.SetIntensity( _MIN_LIGHT_INTENSITY );
			m_pLight->m_Light.Enable( FALSE );
		}
	}		
}

void CBotDispenser::StartArmAnimation() {
	if( m_pBuildAnim ) {
		FASSERT( m_pAnimCombiner );
		m_pDispenserEntity->SetExternalCombiner( 0, m_pAnimCombiner );
		m_pDispenserEntity->UserAnim_Select( -1 );
		m_pDispenserEntity->DriveMeshWithAnim( TRUE );

		// set the animation back to the start
		m_pBuildAnim->UpdateUnitTime( 0.0f );

		// need to do some calculations based on the bot being dispensed
		FASSERT( m_pMeshBuilder );
		f32 fSecsBetweenParts, fSecsPerPart;
		CFXMeshBuildPartMgr::CalculateDropSecs( m_fSpawnSecs,
			m_fUnitPartOverlap,
			(f32)m_pMeshBuilder->GetTotalPartCount(),
			fSecsBetweenParts,
			fSecsPerPart );

		m_fBotArm_UnitSecsTillFirstContact = fmath_Div( fSecsPerPart, m_fSpawnSecs );
		m_fBotArm_OODropNormalizer = fmath_Inv( 1.0f - m_fBotArm_UnitSecsTillFirstContact );

		m_fSecsTillNextSpark = fSecsPerPart;
		m_fSecsBetweenPartLandings = fSecsBetweenParts;
		FMATH_SETBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_LEFT_HAND_SPARK );

		m_fSecsTillNextPartSound = 0.03f;
	}
}

void CBotDispenser::UpdateArmAnimation( f32 fUnitDropTime ) {
	if( m_pBuildAnim ) {
		/////////////////////////////
		// handle the arm animations
		f32 fUnitAnimTime;
		if( fUnitDropTime >= m_fBotArm_UnitSecsTillFirstContact ) {
			// playing the rest of the drop animations
			if( fUnitDropTime < 1.0f ) {
                ToggleArmSound( TRUE );				
			}
			fUnitAnimTime = fUnitDropTime - m_fBotArm_UnitSecsTillFirstContact;
			fUnitAnimTime *= m_fBotArm_OODropNormalizer;
			fUnitAnimTime *= m_fArmAnim_PercentOfAnimPartDrop;
			fUnitAnimTime += m_fArmAnim_PercentOfAnim1stContact;
		} else {
			// waiting for the 1st piece to finish dropping
			fUnitAnimTime = fmath_Div( fUnitDropTime, m_fBotArm_UnitSecsTillFirstContact );
			fUnitAnimTime *= m_fArmAnim_PercentOfAnim1stContact;
		}
		FMATH_CLAMP( fUnitAnimTime, 0.0f, (m_fArmAnim_PercentOfAnim1stContact + m_fArmAnim_PercentOfAnimPartDrop) );
		m_pBuildAnim->UpdateUnitTime( fUnitAnimTime );

		// play a part sound every 1/3 secs
		if( fUnitDropTime <= 0.95f ) {
			m_fSecsTillNextPartSound -= FLoop_fRealPreviousLoopSecs;
			if( m_fSecsTillNextPartSound <= 0.0f ) {
				TogglePartSound( TRUE );
				m_fSecsTillNextPartSound += (1.0f/5.0f);
			}
		}

		//////////////////////////////////////////////
		// handle the spark & smoke particle emissions
		if( m_hContactParticle != FPARTICLE_INVALID_HANDLE ||
			m_hSmokeParticle != FPARTICLE_INVALID_HANDLE ) {
			
			m_fSecsTillNextSpark -= FLoop_fPreviousLoopSecs;
			if( m_fSecsTillNextSpark <= 0.0f ) {
				f32 fRandom = fmath_RandomFloat();
				if( m_nFlags & BOT_DISPENSER_FLAGS_LEFT_HAND_SPARK ) {
					// emit from the left hand
					CFMtx43A *pHand = (m_pDispenserEntity->GetMeshInst()->GetBoneMtxPalette())[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_LEFT_ARM] ];

					fparticle_SpawnEmitter( m_hContactParticle, pHand->m_vPos.v3, NULL, fRandom );
					FMATH_CLEARBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_LEFT_HAND_SPARK );

					// go ahead and emit some smoke too
					fparticle_SpawnEmitter( m_hSmokeParticle, pHand->m_vPos.v3, NULL, fRandom );
				} else {
					// emit from the right hand
					CFMtx43A *pHand = (m_pDispenserEntity->GetMeshInst()->GetBoneMtxPalette())[ m_aBoneIDs[BOT_DISPENSER_BONE_ID_RIGHT_ARM] ];

					fparticle_SpawnEmitter( m_hContactParticle, pHand->m_vPos.v3, NULL, fRandom );
					FMATH_SETBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_LEFT_HAND_SPARK );

					// go ahead and emit some smoke too
					fparticle_SpawnEmitter( m_hSmokeParticle, pHand->m_vPos.v3, NULL, fRandom );
				}				
				m_fSecsTillNextSpark += m_fSecsBetweenPartLandings;
			}
		}
	}
}

void CBotDispenser::EndArmAnimation() {
	if( m_pBuildAnim ) {
		ToggleArmSound( FALSE );
		m_pDispenserEntity->SetExternalCombiner( 0, NULL );
		m_pDispenserEntity->UserAnim_Select( -1 );
		m_pDispenserEntity->DriveMeshWithAnim( FALSE );

		FMATH_CLEARBITMASK( m_nFlags, BOT_DISPENSER_FLAGS_LEFT_HAND_SPARK );
	}
}

void CBotDispenser::ConfigureAI( CBot *pBot, f32 fFeetToWalkInZDir ) {
	//dispensers act a little different in multi-player
	if (MultiplayerMgr.IsMultiplayer())
	{								   //findfix: seems like we shouldn't use ismultiplayer for this condition
		//no ai config. bot will be powered down when not posessed
	}
	else
	{
		CFVec3A Goto;
		f32 fNewY;
		
		u16 uGotoFlags = GOTOFLAG_BLIND_OK | GOTOFLAG_USE_JOB_REACT_RULES;
		Goto.Set( pBot->MtxToWorld()->m_vFront );
		Goto.Mul( fFeetToWalkInZDir );
		Goto.Add( pBot->MtxToWorld()->m_vPos );
		
		fNewY = 0.0f;
		if( aiutils_FloorElevationAt( Goto, &fNewY ) ) {
			Goto.y = fNewY;
			uGotoFlags |= GOTOFLAG_RETURN_FIRE_OK;
		}
		//aimain_pAIGraph->FindClosestLOSVert2D_AdjustWithinRange( &Goto, fFeetToWalkInZDir );


		ai_AssignGoal_Goto( pBot->AIBrain(), Goto, 0, 100, uGotoFlags);

		if (pBot->AIBrain()->GetJobThought() == CAIBrain::TT_WAIT)
		{
			ai_AssignJob_Wander( pBot->AIBrain(),
								50.0f,	// (0.0 - Inf)	Radius (in feet) to stay within while wandering
								FALSE,	// (0/1)		Use radius, but always stay in the original room
								0,		// (0-100)		Min Pct of this unit's ability that it should travel at	 (0 means wander should never change units speed)
								0,		// (0-100)		Max             "      "								 (0 means wander should never change units speed)
								0,		// (0-100)		How likely it is to stop and talk to other bots
								0,		// (0-100)		Breaks will last 5 seconds + random(curiosity)
								0,		// (0-100)		Percent Chance that that bot will take a break. (evaluated at every grid point)
								NULL );	// String table Name of espline entity that contains the path points
		}
	}

	//new (hehe) script event that gets triggered when an autobot is born.
	//I'm considering born to be when the bot is actually set free into the world
	CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("AutoBot"), AUTOBOT_BORN, (u32) pBot, (u32) CAlarmNet::FindAlarmNet(this));
}

void CBotDispenser::ToggleArmSound( BOOL bOn ) {

	if( m_pArmSndGrp ) {
		if( bOn && !m_pArmSound ) {
			// start the sound
			m_pArmSound = CFSoundGroup::AllocAndPlaySound( m_pArmSndGrp, FALSE, &m_pDispenserEntity->MtxToWorld()->m_vPos );
		} else if( !bOn && m_pArmSound ) {
			// stop the sound
			m_pArmSound->Destroy();
			m_pArmSound = NULL;
		}
	}
}

void CBotDispenser::ToggleBeamSound( BOOL bOn ) {

	if( m_pBeamSndGrp ) {
		if( bOn && !m_pBeamSound ) {
			// start the sound
			m_pBeamSound = CFSoundGroup::AllocAndPlaySound( m_pBeamSndGrp, FALSE, &m_pDispenserEntity->MtxToWorld()->m_vPos );
		} else if( !bOn && m_pBeamSound ) {
			// stop the sound
			m_pBeamSound->Destroy();
			m_pBeamSound = NULL;
		}
	}
}

void CBotDispenser::TogglePartSound( BOOL bOn ) {

	if( m_pPartsSndGrp && bOn ) {
		// start the sound
		CFSoundGroup::PlaySound( m_pPartsSndGrp, FALSE, &m_pDispenserEntity->MtxToWorld()->m_vPos );
	}
}

void CBotDispenser::UpdateVolumes( f32 fUnitDropTime ) {
	f32 fNewVolume;

	if( fUnitDropTime < 0.9f ) {
		// no need to adjust the volume
		return;
	}
	fNewVolume = 1.0f - fUnitDropTime;
	fNewVolume *= (1.0f/(1.0f - 0.9f));

	if( m_pArmSound ) {
		m_pArmSound->SetVolume( fNewVolume );
	}
	if( m_pBeamSound ) {
		m_pBeamSound->SetVolume( fNewVolume );
	}
}
