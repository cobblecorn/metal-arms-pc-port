//////////////////////////////////////////////////////////////////////////////////////
// eproj_swarmer.cpp - Swarmer projectiles.
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
// 06/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eproj_swarmer.h"
#include "floop.h"
#include "fworld_coll.h"
#include "explosion.h"
#include "meshtypes.h"
#include "weapon.h"
#include "lightpool.h"
#include "fsound.h"
#include "AI/AIEnviro.h"  //this file had one to few ai files in it.



#if EPROJ_SWARMER_MERV_ROCKET_COUNT != 3
	#error <EPROJ_SWARMER_MERV_ROCKET_COUNT must be 3 unless _afMervLaunchAngle[] is changed below.>
#endif


static const f32 _afMervLaunchAngle[EPROJ_SWARMER_MERV_ROCKET_COUNT] = {
	0.0f, FMATH_DEG2RAD(-65.0f), FMATH_DEG2RAD(65.0f)
};


BOOL CEProj_Swarmer::Create( CEProj *pProj ) {
	Init( pProj );

	return TRUE;
}


void CEProj_Swarmer::Init( CEProj *pProj ) {
	u32 i;

	m_SetupParams.fSwarmAmplitude = 0.0f;
	m_SetupParams.fSwarmingBlendInInvDist = 0.0f;
	m_SetupParams.fSwarmingSpiralSpeed = 0.0f;
	m_SetupParams.pMervStaticParams = NULL;

	m_fRotateZ = 0.0f;
	m_fDistScale = 0.0f;
	m_fDistOffset = 0.0f;
	m_fStartDist = 0.0f;
	m_fMervSpawnSecs = 0.0f;
	m_fIgniteSecs = 0.0f;
	m_bMervRocket = FALSE;
	m_bMervDestIsPoint = FALSE;
	m_LaunchPos_WS.Zero();
	m_LaunchQuat.Identity();
	m_uAISoundHandle = 0;

	for( i=0; i<EPROJ_SWARMER_MERV_ROCKET_COUNT; ++i ) {
		m_apMervProj[i] = NULL;
	}
}


void CEProj_Swarmer::SetSwarmerParams( CEProj *pProj, const CEProj_Swarmer_Params_t *pParms ) {
	FASSERT( pParms );

	fang_MemCopy( &m_SetupParams, pParms, sizeof( CEProj_Swarmer_Params_t ) );
}


void CEProj_Swarmer::Launched( CEProj *pProj ) {
	if( m_SetupParams.pMervStaticParams && !m_bMervRocket ) {
		m_fMervSpawnSecs = m_SetupParams.pMervStaticParams->fMervSpawnSecs;
	}

	_StartSwarming( pProj );
}


void CEProj_Swarmer::_StartSwarming( CEProj *pProj ) {
	// Take a snapshot of the launch position...
	m_LaunchQuat.BuildQuat( *pProj->MtxToWorld() );
	m_LaunchPos_WS = pProj->MtxToWorld()->m_vPos;

	// Introduce randomness...
	m_fRotateZ = fmath_RandomFloatRange( 0.0, FMATH_2PI );
	m_fDistScale = fmath_RandomFloatRange( 0.7f, 1.3f );
	m_fDistOffset = fmath_RandomFloatRange( -10.0f, 10.0f );

	m_fStartDist = pProj->GetDistTraveled();
}


BOOL CEProj_Swarmer::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact ) {
	u32 i;

	if( m_bMervRocket ) {
		// If we're a Merv, tell our owner that we no longer exist...
		if( m_apMervProj[0] ) {
			CEProj_Swarmer *pProjOwnerSwarmer = (CEProj_Swarmer *)m_apMervProj[0]->GetExtensionObject( CEProj::PROJTYPE_SWARMER );

			pProjOwnerSwarmer->_InformOwnerRocketThatMervDetonated( pProj );

			m_apMervProj[0] = NULL;
		}
	} else {
		// If we're a Merv owner, inform our Mervs of their target point...
		if( bMakeEffect && (nEvent == CEProj::EVENT_HIT_GEO) ) {
			CEProj_Swarmer *pProjMervSwarmer;

			for( i=0; i<EPROJ_SWARMER_MERV_ROCKET_COUNT; ++i ) {
				if( m_apMervProj[i] ) {
					pProjMervSwarmer = (CEProj_Swarmer *)m_apMervProj[i]->GetExtensionObject( CEProj::PROJTYPE_SWARMER );
					pProjMervSwarmer->_InformMervOfTarget( i, m_apMervProj[i], pProj );
				}
			}
		}

		// Forget about all of our Mervs...
		for( i=0; i<EPROJ_SWARMER_MERV_ROCKET_COUNT; ++i ) {
			m_apMervProj[i] = NULL;
		}
	}

	return TRUE;
}


BOOL CEProj_Swarmer::Work( CEProj *pProj ) {
	f32 fDistTraveledThisFrame, fUnitAmp, fAdjustedTravelDist, fSwarmDevation, fMag2;
	CFVec3A SwarmPos_MS, SwarmPos_WS, PrevProjPos_WS;
	CFMtx43A NewMtx;
	FCollImpact_t CollImpact;

	// Compute linear distance we've traveled this frame...
	fDistTraveledThisFrame = pProj->GetLinSpeed() * FLoop_fPreviousLoopSecs;
	pProj->m_fDistTraveled_WS += fDistTraveledThisFrame;

	// Target seeking feature...
	if( pProj->GetTargetedEntity() ) {
		// This is a guided projectile...
		CFVec3A LinUnitDir_WS;

		// Point projectile at target...
		LinUnitDir_WS.Set( pProj->GetTargetedEntity()->MtxToWorld()->m_vPos );
		LinUnitDir_WS.Sub( pProj->MtxToWorld()->m_vPos );

		fMag2 = LinUnitDir_WS.MagSq();
		if( fMag2 > 0.0001f ) {
			LinUnitDir_WS.Mul( fmath_InvSqrt(fMag2) );
			pProj->SetLinUnitDir_WS( &LinUnitDir_WS );
		}
	}

	if( m_bMervRocket ) {
		_MervWork( pProj );
	}

	if( m_SetupParams.fSwarmAmplitude == 0.0f ) {
		// No swarming...

		NewMtx.m_vPos.Mul( *pProj->GetLinUnitDir_WS(), fDistTraveledThisFrame );
		NewMtx.m_vPos.Add( pProj->MtxToWorld()->m_vPos );

		// Compute projectile orientation...
		if( pProj->GetTargetedEntity() == NULL ) {
			// If not guided, orientation is always the same...

			NewMtx.m_vRight = pProj->MtxToWorld()->m_vRight;
			NewMtx.m_vUp = pProj->MtxToWorld()->m_vUp;
			NewMtx.m_vFront = pProj->MtxToWorld()->m_vFront;
		} else {
			// If guided, orientation has probably changed...

			NewMtx.UnitMtxFromUnitVec( pProj->GetLinUnitDir_WS() );
		}
	} else {
		CFVec3A UnitDir_WS;
		f32 fSwarmTravelDist;

		fSwarmTravelDist = pProj->GetDistTraveled() - m_fStartDist;

		// Compute swarming amplitude...
		fUnitAmp = fSwarmTravelDist * m_SetupParams.fSwarmingBlendInInvDist;
		FMATH_CLAMPMAX( fUnitAmp, 1.0f );

		// Compute our travel distance...
		fAdjustedTravelDist = m_fDistOffset + m_fDistScale * pProj->GetDistTraveled() * m_SetupParams.fSwarmingSpiralSpeed;

		// Compute our swarm deviation...
		fSwarmDevation = fUnitAmp * m_SetupParams.fSwarmAmplitude * fmath_Sin( fAdjustedTravelDist );

		// Update swarming plane rotation...
		m_fRotateZ += FLoop_fPreviousLoopSecs;

		// Compute swarm position...
		SwarmPos_MS.Set( fSwarmDevation, 0.0f, 0.0f );
		SwarmPos_WS.ReceiveRotationZ( SwarmPos_MS, m_fRotateZ );
		m_LaunchQuat.MulPoint( SwarmPos_WS );

		// Compute projectile's new position...
		NewMtx.m_vPos.Mul( *pProj->GetLinUnitDir_WS(), fSwarmTravelDist ).Add( m_LaunchPos_WS );
		NewMtx.m_vPos.Add( SwarmPos_WS );

		// Compute instantaneous unit direction...
		UnitDir_WS.Sub( NewMtx.m_vPos, pProj->MtxToWorld()->m_vPos );
		fMag2 = UnitDir_WS.MagSq();
		if( fMag2 > 0.0001f ) {
			UnitDir_WS.Mul( fmath_InvSqrt(fMag2) );
		} else {
			UnitDir_WS = *pProj->GetLinUnitDir_WS();
		}

		NewMtx.UnitMtxFromUnitVec( &UnitDir_WS );
	}

	PrevProjPos_WS = pProj->MtxToWorld()->m_vPos;

	// Relocate the projectile...
	pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx );

	// Perform collision...
	if( pProj->PerformThickRayCollisionTest( &CollImpact, &PrevProjPos_WS, &NewMtx ) ) {
		// Found collision...

		NewMtx.m_vPos.Mul( *pProj->GetLinUnitDir_WS(), -0.01f ).Add( CollImpact.ImpactPoint );
		pProj->Relocate_Xlat_WS( &NewMtx.m_vPos );

		pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, &CollImpact );

		return TRUE;
	}

	if( pProj->GetTargetedEntity() ) {
		// This is a guided projectile...
		CFVec3A vDeltaPos;

		// Find distance to target...
		vDeltaPos.Set( pProj->GetTargetedEntity()->MtxToWorld()->m_vPos );
		vDeltaPos.Sub( pProj->MtxToWorld()->m_vPos );
		if( vDeltaPos.MagSq() < 4.0f ) {
			// Detonate when close to target...

			pProj->Detonate( TRUE, CEProj::EVENT_DETONATE_API, NULL );

			return TRUE;
		}
	}

	if( m_SetupParams.pMervStaticParams ) {
		if( m_fMervSpawnSecs > 0.0f ) {
			m_fMervSpawnSecs -= FLoop_fPreviousLoopSecs;

			if( m_fMervSpawnSecs <= 0.0f ) {
				// Time to spawn Mervs...

				m_fMervSpawnSecs = 0.0f;

				_LaunchMervs( pProj );
			}
		}
	}

	//
	// Moving in the world, 
	//	notify AI system of our presence
	//  if we are from a player.
	//

	if( pProj->GetDamagerBot() &&
		pProj->GetDamagerBot()->IsPlayerBot() &&
		(!m_uAISoundHandle || !AIEnviro_ModifySound( m_uAISoundHandle,  PrevProjPos_WS, AIEnviro_fProjectileVisualRadius, 0.3f, AISOUNDTYPE_PROJECTILE, pProj->GetDamagerBot()) )) {
		m_uAISoundHandle = AIEnviro_AddSound( PrevProjPos_WS, AIEnviro_fProjectileVisualRadius, 0.3f, AISOUNDTYPE_PROJECTILE, AISOUNDCTRL_VISIBLE_ENTITY, pProj->GetDamagerBot() );
	}


	return FALSE;
}


void CEProj_Swarmer::_LaunchMervs( CEProj *pProj ) {
	CEProj_Swarmer_Params_t SwarmerParams;
	CEProj_Swarmer *pProjMerv, *pProjSwarmer;
	CFWorldLightItem *pWorldLightItem, *pParentWorldLightItem;
	CFVec3A BaseUnitDir_WS, InitialUnitDir_WS;
	CFQuatA Quat;
	CFMtx43A ProjMtx;
	CEProj *pProjRocket;
	u32 i;

	const CEProjExt::CEProj_Merv_StaticParams_t *pMervParams = m_SetupParams.pMervStaticParams;
	FASSERT( pMervParams );

	pProjSwarmer = (CEProj_Swarmer *)pProj->GetExtensionObject( CEProj::PROJTYPE_SWARMER );
	pParentWorldLightItem = pProj->GetAttachedLight();

	SwarmerParams.pMervStaticParams = pProjSwarmer->m_SetupParams.pMervStaticParams;
	SwarmerParams.fSwarmAmplitude = 0.0f;
	SwarmerParams.fSwarmingBlendInInvDist = 0.0f;
	SwarmerParams.fSwarmingSpiralSpeed = 0.0f;

	ProjMtx.m_vPos = pProj->MtxToWorld()->m_vPos;

	// Compute initial direction...
	Quat.BuildQuat( pProj->MtxToWorld()->m_vRight, pMervParams->fLaunchPitch );
	Quat.MulPoint( BaseUnitDir_WS, *pProj->GetLinUnitDir_WS() );

	for( i=0; i<EPROJ_SWARMER_MERV_ROCKET_COUNT; ++i ) {
		// Get a new rocket projectile...
		pProjRocket = CEProjPool::GetProjectileFromFreePool( CWeapon::m_ahProjPool[CWeapon::PROJ_POOL_TYPE_SWARMER_ROCKET] );
		if( pProjRocket == NULL ) {
			// No projectiles left in pool...
			break;
		}

		if( pProjRocket->m_nProjType != CEProj::PROJTYPE_SWARMER ) {
			DEVPRINTF( "CEProj_Swarmer::_LaunchMervs(): Weapon pool is dispensing the wrong type of projectiles. Swarmers required.\n" );
			pProjRocket->Detonate( FALSE );
			break;
		}

		// Remember the Merv we launched...
		pProjSwarmer->m_apMervProj[i] = pProjRocket;

		pProjMerv = (CEProj_Swarmer *)pProjRocket->GetExtensionObject( CEProj::PROJTYPE_SWARMER );

		// Initialize the new rocket projectile...
		pProjRocket->Init();
		pProjRocket->SetSwarmerParams( &SwarmerParams );

		pProjMerv->m_bMervRocket = TRUE;
		pProjMerv->m_bMervDestIsPoint = FALSE;
		pProjMerv->m_MervDest = *pProj->GetLinUnitDir_WS();
		pProjMerv->m_apMervProj[0] = pProj;

		if( pProj->GetDamager() ) {
			pProjRocket->SetDamager( pProj->GetDamager()->pWeapon, pProj->GetDamager()->pBot, pProj->GetDamager()->nDamagerPlayerIndex );
		}

		pProjRocket->SetDamageProfile( pProj->GetDamageProfile() );
		pProjRocket->SetSkipListCallback( pProj->GetSkipListCallback() );
		pProjRocket->SetMaxDistCanTravel( pProj->GetMaxDistCanTravel() );
		pProjRocket->SmokeTrailOn( pProj->GetSmokeTrailAttributes() );
		pProjRocket->SetExplosionGroup( pMervParams->hExplosionGroupImpact );
		pProjRocket->SetLinSpeed( pProj->GetLinSpeed() * fmath_RandomFloatRange( pMervParams->fLaunchSpeedMultMin, pMervParams->fLaunchSpeedMultMax ) );
		pProjRocket->LoopingSoundOn( pProj->GetLoopingSoundGroup() );

		// Compute initial direction...
		Quat.BuildQuat( pProj->MtxToWorld()->m_vFront, _afMervLaunchAngle[i] );
		Quat.MulPoint( InitialUnitDir_WS, BaseUnitDir_WS );
		pProjRocket->SetLinUnitDir_WS( &InitialUnitDir_WS );

		// Compute projectile matrix...
		ProjMtx.UnitMtxFromUnitVec( &InitialUnitDir_WS );
		pProjRocket->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, pProj->ScaleToWorld() );

		// Create corona...
		if( pParentWorldLightItem ) {
			pWorldLightItem = CLightPool::GetFromFreePool();
			if( pWorldLightItem ) {
				pWorldLightItem->m_Light.InitOmniLight( &ProjMtx.m_vPos.v3, 0.1f );

				FMATH_SETBITMASK( pWorldLightItem->m_Light.m_nFlags, FLIGHT_FLAG_CORONA | FLIGHT_FLAG_CORONA_WORLDSPACE | FLIGHT_FLAG_CORONA_ONLY );

				pWorldLightItem->m_Light.m_pCoronaTex = pParentWorldLightItem->m_Light.m_pCoronaTex;
				pWorldLightItem->m_Light.m_fCoronaScale = pParentWorldLightItem->m_Light.m_fCoronaScale;
				pWorldLightItem->m_Light.m_fDeltaScale = pParentWorldLightItem->m_Light.m_fDeltaScale;

				pWorldLightItem->m_Light.SetColor( &pParentWorldLightItem->m_Light.m_Motif.ColorRGB );
				pWorldLightItem->m_Light.SetMotif( fcolor_GetRandomMotif(FCOLORMOTIF_ROCKET0) );
				pProjRocket->SetAttachedLight( pWorldLightItem );
			}
		}

		pProjRocket->Launch();
	}

	if( pMervParams->hExplosionGroupSpawn != FEXPLOSION_INVALID_HANDLE ) {
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();

			SpawnParams.Pos_WS = pProj->MtxToWorld()->m_vPos;
			SpawnParams.UnitDir.Zero();
			SpawnParams.uSurfaceType = 0;

			CExplosion2::SpawnExplosion( hSpawner, pMervParams->hExplosionGroupSpawn, &SpawnParams );
		}
	}
}


void CEProj_Swarmer::_MervWork( CEProj *pProj ) {
	CFVec3A RotationUnitAxis_WS, DestUnitDir_WS;
	f32 fMag2;

	const CEProjExt::CEProj_Merv_StaticParams_t *pMervParams = m_SetupParams.pMervStaticParams;
	FASSERT( m_SetupParams.pMervStaticParams );

	if( m_bMervDestIsPoint ) {
		// m_MervDest is our target point...

		if( m_fIgniteSecs > 0.0f ) {
			m_fIgniteSecs -= FLoop_fPreviousLoopSecs;

			if( m_fIgniteSecs <= 0.0f ) {
				m_fIgniteSecs = 0.0f;

				CFWorldLightItem *pWorldLightItem = pProj->GetAttachedLight();
				if( pWorldLightItem ) {
					pWorldLightItem->m_Light.m_fCoronaScale = pMervParams->fIgniteCoronaScale;

					pWorldLightItem->m_Light.SetColor( pMervParams->fIgniteCoronaRed, pMervParams->fIgniteCoronaGreen, pMervParams->fIgniteCoronaBlue );
					pWorldLightItem->m_Light.SetMotif( fcolor_GetRandomMotif( pMervParams->nIgniteCoronaMotif ) );
				}

				CFSoundGroup::PlaySound( pMervParams->pSoundGroupIgnite, FALSE, &pProj->MtxToWorld()->m_vPos );
			}
		}

		pProj->m_fLinSpeed_WS += FLoop_fPreviousLoopSecs * pMervParams->fIgniteAcceleration;
		FMATH_CLAMPMAX( pProj->m_fLinSpeed_WS, pMervParams->fIgniteMaxSpeed );

		DestUnitDir_WS.Sub( m_MervDest, pProj->MtxToWorld()->m_vPos );

		fMag2 = DestUnitDir_WS.MagSq();
		if( fMag2 < 0.0001f ) {
			return;
		}

		DestUnitDir_WS.Mul( fmath_InvSqrt(fMag2) );
	} else {
		// m_MervDest is our target unit direction...

		DestUnitDir_WS = m_MervDest;
	}

	RotationUnitAxis_WS.Cross( *pProj->GetLinUnitDir_WS(), DestUnitDir_WS );

	fMag2 = RotationUnitAxis_WS.MagSq();
	if( fMag2 < 0.0001f ) {
		return;
	}

	// Adjust linear direction toward destination...

	CFQuatA Quat;
	CFVec3A NewUnitDir_WS, NewRotationAxis_WS;
	CFMtx43A NewMtx;
	BOOL bStartSwarming = FALSE;

	RotationUnitAxis_WS.Mul( fmath_InvSqrt(fMag2) );

	Quat.BuildQuat( RotationUnitAxis_WS, FLoop_fPreviousLoopSecs * pMervParams->fAngularSpeed );
	Quat.MulPoint( NewUnitDir_WS, *pProj->GetLinUnitDir_WS() );

	NewRotationAxis_WS.Cross( NewUnitDir_WS, DestUnitDir_WS );

	if( NewRotationAxis_WS.Dot( RotationUnitAxis_WS ) < 0.0f ) {
		// We stepped too far...

		NewUnitDir_WS = DestUnitDir_WS;

		bStartSwarming = !m_bMervDestIsPoint;
	}

	pProj->SetLinUnitDir_WS( &NewUnitDir_WS );

	NewMtx.UnitMtxFromUnitVec( &NewUnitDir_WS );
	NewMtx.m_vPos = pProj->MtxToWorld()->m_vPos;

	pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx, FALSE );

	if( bStartSwarming ) {
		m_SetupParams.fSwarmAmplitude = pMervParams->fSwarmingAmplitude;
		m_SetupParams.fSwarmingBlendInInvDist = pMervParams->fSwarmingBlendInInvDist;
		m_SetupParams.fSwarmingSpiralSpeed = pMervParams->fSwarmingSpiralSpeed;

		_StartSwarming( pProj );
	}
}


void CEProj_Swarmer::_InformMervOfTarget( u32 nMervIndex, CEProj *pMervProj, CEProj *pOwnerProj ) {
	const CEProjExt::CEProj_Merv_StaticParams_t *pMervParams = m_SetupParams.pMervStaticParams;
	FASSERT( pMervParams );

	CFVec3A RandPosOffset_WS;

	RandPosOffset_WS.Set( fmath_RandomFloatRange( -4.0f, 4.0f ), fmath_RandomFloatRange( -4.0f, 4.0f ), fmath_RandomFloatRange( -4.0f, 4.0f ) );

	m_MervDest.Add( pOwnerProj->MtxToWorld()->m_vPos, RandPosOffset_WS );
	m_bMervDestIsPoint = TRUE;

	pMervProj->m_LinUnitDir_WS = pMervProj->MtxToWorld()->m_vZ;
	m_SetupParams.fSwarmAmplitude = 0.0f;

	m_fIgniteSecs = (f32)nMervIndex * 0.2f + 0.0001f;

	// We're on our own now...
	m_apMervProj[0] = NULL;
}


void CEProj_Swarmer::_InformOwnerRocketThatMervDetonated( CEProj *pDetonatedMervProj ) {
	u32 i;

	for( i=0; i<EPROJ_SWARMER_MERV_ROCKET_COUNT; ++i ) {
		if( m_apMervProj[i] == pDetonatedMervProj ) {
			// Forget we launched this Merv...
			m_apMervProj[i] = NULL;
			break;
		}
	}
}

