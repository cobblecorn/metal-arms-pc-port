//////////////////////////////////////////////////////////////////////////////////////
// eproj_Arrow.cpp - Arrow projectiles.
//
// Author: John Lafleur 
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
// 06/19/02 Lafleur		Converted to eproj from Steve's pre-eproj implementation.
// 08/27/02 Ranck		Fixed bug where arrow would mess up during wiggle when shot into
//                      and enemy.
// 03/19/03 Ranck		Took ownership and modified to support the new Rivet weapon.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eproj_arrow.h"
#include "floop.h"
#include "explosion.h"
#include "meshtypes.h"
#include "damage.h"
#include "eparticlepool.h"
#include "weapon_rivet.h"
#include "fsound.h"


#define _ARROW_VIBE_MAX_ANGLE		0.13f
#define _ARROW_VIBE_SECS			0.25f



BOOL CEProj_Arrow::Create( CEProj *pProj ) {
	return TRUE;
}


void CEProj_Arrow::Init( CEProj *pProj ) {
	m_SetupParams.pCollImpact = NULL;
	m_SetupParams.pArrowStaticParams = NULL;
}


// IMPORTANT: The contents of pParams->pCollImpact must persist through the call to CEProj::Launch().
void CEProj_Arrow::SetArrowParams( CEProj *pProj, const CEProj_Arrow_Params_t *pParams ) { 
	m_SetupParams = *pParams;
}


void CEProj_Arrow::Launched( CEProj *pProj ) {
	// Init arrow parameters...
	m_fVibePolarity = 1.0f;
	m_fLifeSecs = m_SetupParams.pArrowStaticParams->fArrowLifeSecs;
	m_fUnitSteamIntensity = 0.0f;
	m_bTerminateMode = FALSE;
	m_pParticleSteam = NULL;
	m_pAudioEmitterSteam = NULL;

	if( CGColl::ComputeProjectileReaction( m_SetupParams.pCollImpact ) == CEntity::PROJECTILE_REACTION_DEATH_TO_ALL ) {
		// Explode arrow now...

		pProj->Detonate();

		return;
	}

	CEntity *pHitEntity = CGColl::ExtractEntity( m_SetupParams.pCollImpact );
	if( pHitEntity ) {
		// We hit an entity...

		FASSERT( m_SetupParams.pCollImpact->pTag );

		CFWorldMesh *pHitWorldMesh = (CFWorldMesh *)m_SetupParams.pCollImpact->pTag;
		cchar *pszHitBoneName = NULL;

		if( pHitWorldMesh->m_pMesh->pBoneArray ) {
			// This mesh has bones. Get the name of the bone we hit...
			pszHitBoneName = pHitWorldMesh->m_pMesh->pBoneArray[ m_SetupParams.pCollImpact->nBoneIndex ].szName;
		}

		// Attach the arrow to the entity it hit...
		pProj->Attach_ToParent_WS( pHitEntity, pszHitBoneName );

		// No vibration on entities...
		m_fVibeUnitAmplitude = 0.0f;
	} else {
		m_fVibeUnitAmplitude = 1.0f;
		m_VibeRotUnitAxis_WS = pProj->MtxToWorld()->m_vRight;
	}
}


BOOL CEProj_Arrow::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact ) {
	if( m_SetupParams.pArrowStaticParams->nType == EPROJ_ARROW_TYPE_REMOTE_BOOM ) {
		_SetBotPanicSecs( 0.0f );
	}

	_KillSteamSound( pProj );

	if( m_pParticleSteam ) {
		m_pParticleSteam->DetachFromParent();
		m_pParticleSteam->RemoveFromWorld();

		m_pParticleSteam = NULL;
	}

	if( bMakeEffect ) {
		bMakeEffect = FALSE;

		if( m_SetupParams.pArrowStaticParams->nType != EPROJ_ARROW_TYPE_SHRINK ) {
			if( nEvent != CEProj::EVENT_REACHED_MAX_DIST ) {
				bMakeEffect = TRUE;
			}
		} else {
			if( (nEvent == CEProj::EVENT_DETONATE_API) && (m_fLifeSecs > 0.0f) ) {
				bMakeEffect = TRUE;
			}
		}

		if( bMakeEffect ) {
			FExplosion_GroupHandle_t hExplosion;
			u32 nExplosionIndex = 0;

			if( m_SetupParams.pArrowStaticParams->nType == EPROJ_ARROW_TYPE_REMOTE_BOOM ) {
				if( m_fUnitSteamIntensity >= (2.0f/3.0f) ) {
					nExplosionIndex = 2;
				} else if( m_fUnitSteamIntensity >= (1.0f/3.0f) ) {
					nExplosionIndex = 1;
				}
			}

			hExplosion = m_SetupParams.pArrowStaticParams->ahExplosionGroup[nExplosionIndex];

			if( hExplosion != FEXPLOSION_INVALID_HANDLE ) {
				FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

				if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
					FExplosionSpawnParams_t SpawnParams;

					SpawnParams.InitToDefaults();

					SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
					SpawnParams.pDamageProfile = pProj->GetDamageProfile();
					SpawnParams.pDamager = pProj->GetDamager();

					SpawnParams.Pos_WS.Mul( pProj->MtxToWorld()->m_vZ, -pProj->GetWorldMesh()->GetBoundingSphere().m_fRadius ).Add( pProj->MtxToWorld()->m_vPos );
					SpawnParams.UnitDir.ReceiveNegative( pProj->MtxToWorld()->m_vZ );
					SpawnParams.uSurfaceType = 0;

					CExplosion2::SpawnExplosion( hSpawner, hExplosion, &SpawnParams );
				}
			}
		}
	}

	pProj->DetachFromParent();

	// Returns FALSE to let the CEProj class know we've already make the effect...
	return FALSE;
}


void CEProj_Arrow::_SetBotPanicSecs( f32 fPanicSecs ) {
	if( m_SetupParams.pArrowIsStuckToEntity && (m_SetupParams.pArrowIsStuckToEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		// We're stuck to a bot. Make him panic...

		((CBot *)m_SetupParams.pArrowIsStuckToEntity)->SetPanicSecs( fPanicSecs );
	}
}


void CEProj_Arrow::_StartSteamSound( CEProj *pProj ) {
	_KillSteamSound( pProj );

	m_pAudioEmitterSteam = pProj->AllocAndPlaySound( m_SetupParams.pArrowStaticParams->pSoundGroupSteam );

	_UpdateSteamPitchMult( pProj );
}


void CEProj_Arrow::_KillSteamSound( CEProj *pProj ) {
	if( m_pAudioEmitterSteam ) {
		m_pAudioEmitterSteam->Destroy();
		m_pAudioEmitterSteam = NULL;
	}
}


void CEProj_Arrow::_UpdateSteamSound( CEProj *pProj ) {
	if( m_pAudioEmitterSteam ) {
		if( m_SetupParams.pArrowStaticParams->nType == EPROJ_ARROW_TYPE_REMOTE_BOOM ) {
			_UpdateSteamPitchMult( pProj );
		}

		m_pAudioEmitterSteam->SetPosition( &pProj->MtxToWorld()->m_vPos );
	}
}


void CEProj_Arrow::_UpdateSteamPitchMult( CEProj *pProj ) {
	f32 fPitchMult = 1.0f;

	if( m_pAudioEmitterSteam ) {
		CFSoundInfo *pSoundInfo = CFSoundGroup::GetSoundInfo( m_SetupParams.pArrowStaticParams->pSoundGroupSteam, 0 );

		if( pSoundInfo ) {
			CFSoundInfo::DecompressedSoundInfo_t DecompressedSoundInfo;

			pSoundInfo->Decompress( &DecompressedSoundInfo );

			if( m_SetupParams.pArrowStaticParams->nType == EPROJ_ARROW_TYPE_REMOTE_BOOM ) {
				fPitchMult = FMATH_FPOT( m_fUnitSteamIntensity, DecompressedSoundInfo.m_fMinPitchMult, DecompressedSoundInfo.m_fMaxPitchMult );
			} else {
				fPitchMult = DecompressedSoundInfo.m_fMinPitchMult;
			}

			m_pAudioEmitterSteam->SetFrequencyFactor( fPitchMult );
		}
	}
}


BOOL CEProj_Arrow::Work( CEProj *pProj ) {
	BOOL bUpdateLifeSecs = TRUE;

	if( m_SetupParams.pArrowStaticParams->nType == EPROJ_ARROW_TYPE_REMOTE_BOOM ) {
		CWeaponRivet *pWeaponRivet = (CWeaponRivet *)pProj->GetDamagerWeapon();

		if( m_bTerminateMode && pWeaponRivet->IsArrowSuspendedByTriggerDown( pProj ) ) {
			bUpdateLifeSecs = FALSE;
		}
	}

	if( bUpdateLifeSecs ) {
		m_fLifeSecs -= FLoop_fPreviousLoopSecs;
		if( m_fLifeSecs <= 0.0f ) {
			// Arrow has lived its life. Detonate it...

			m_fLifeSecs = 0.0f;

			pProj->Detonate();

			return TRUE;
		}
	} else {
		if( m_fUnitSteamIntensity < 1.0f ) {
			f32 fOldUnitSteamIntensity = m_fUnitSteamIntensity;

			m_fUnitSteamIntensity += (1.0f / 3.0f) * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitSteamIntensity, 1.0f );

			if( (fOldUnitSteamIntensity < (2.0f/3.0f)) && (m_fUnitSteamIntensity >= (2.0f/3.0f)) ) {
				_SetBotPanicSecs( 60.0f );
			}
		}
	}

	f32 fNewScale = -1.0f;

	if( !m_bTerminateMode ) {
		if( m_fLifeSecs < m_SetupParams.pArrowStaticParams->fTerminateAnimDuration ) {
			m_bTerminateMode = TRUE;

			if( m_SetupParams.pArrowStaticParams->nType != EPROJ_ARROW_TYPE_SHRINK ) {
				FASSERT( m_pParticleSteam == NULL );

				if( m_SetupParams.pArrowStaticParams->hParticleSteam ) {
					m_pParticleSteam = eparticlepool_GetEmitter();

					if( m_pParticleSteam ) {
						CFMtx43A SteamUnitMtx;
						CFVec3A SteamUnitDir_WS;

						SteamUnitDir_WS.ReceiveNegative( pProj->MtxToWorld()->m_vZ );
						SteamUnitMtx.UnitMtxFromUnitVec( &SteamUnitDir_WS );
						SteamUnitMtx.m_vPos = pProj->MtxToWorld()->m_vPos;

						m_pParticleSteam->StartEmission( m_SetupParams.pArrowStaticParams->hParticleSteam, m_fUnitSteamIntensity );

						m_pParticleSteam->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &SteamUnitMtx, 1.0f, FALSE );
						m_pParticleSteam->Attach_ToParent_WS( pProj );
					}
				}

				_StartSteamSound( pProj );
			} else {
				m_fInitialScale = pProj->ScaleToWorld();
			}
		}
	} else {
		// Terminate mode...

		if( m_SetupParams.pArrowStaticParams->nType == EPROJ_ARROW_TYPE_REMOTE_BOOM ) {
			// Update steam...

			if( m_pParticleSteam ) {
				m_pParticleSteam->SetUnitIntensity( m_fUnitSteamIntensity );
			}
		} else if( m_SetupParams.pArrowStaticParams->nType == EPROJ_ARROW_TYPE_SHRINK ) {
			// Update scale...

			fNewScale = m_fInitialScale * fmath_Div( m_fLifeSecs, m_SetupParams.pArrowStaticParams->fTerminateAnimDuration );
			FMATH_CLAMPMIN( fNewScale, 0.00001f );
		}
	}

	// Apply some wiggle during the beginning of the "thunk"...
	if( m_fVibeUnitAmplitude > 0.0f ) {
		CFMtx43A FinalMtx;
		CFQuatA Quat, VibeQuat, FinalQuat;
		f32 fVibeAngle;

		Quat.BuildQuat( *pProj->MtxToWorld() );

		m_fVibeUnitAmplitude -= FLoop_fPreviousLoopSecs * (1.0f / _ARROW_VIBE_SECS);
		FMATH_CLAMPMIN( m_fVibeUnitAmplitude, 0.0f );

		fVibeAngle = _ARROW_VIBE_MAX_ANGLE * m_fVibeUnitAmplitude * m_fVibePolarity;
		VibeQuat.BuildQuat( m_VibeRotUnitAxis_WS, fVibeAngle );
		FinalQuat.Mul( VibeQuat, Quat );
		FinalQuat.Unitize();

		FinalQuat.BuildMtx( FinalMtx );
		FinalMtx.m_vPos = pProj->MtxToWorld()->m_vPos;

		if( fNewScale < 0.0f ) {
			// No new scale...
			pProj->Relocate_RotXlatFromUnitMtx_WS( &FinalMtx );
		} else {
			// Apply new scale...
			pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &FinalMtx, fNewScale, FALSE );
		}

		m_fVibePolarity = -m_fVibePolarity;
	} else {
		if( fNewScale >= 0.0f ) {
			pProj->Relocate_Scale_WS( fNewScale );
		}
	}

	_UpdateSteamSound( pProj );

	return FALSE;
}


