//////////////////////////////////////////////////////////////////////////////////////
// eproj_grenade.cpp - Grenade projectiles.
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
#include "eproj_grenade.h"
#include "floop.h"
#include "fworld_coll.h"
#include "explosion.h"
#include "meshtypes.h"
#include "entity.h"
#include "weapon.h"
#include "weapon_gren.h"
#include "damage.h"
#include "AI/AIEnviro.h"
#include "fxempblast.h"
#include "bot.h"
#include "fsound.h"



#define _SECS_BETWEEN_PING_SOUNDS	( 0.2f )
#define _MAX_GRENADE_MAG			( 50.0f )
#define _MAX_GRENADE_MAGSQ			( _MAX_GRENADE_MAG * _MAX_GRENADE_MAG )
#define _OO_MAX_GRENADE_MAG			( 1.0f / _MAX_GRENADE_MAG )
#define _OO_MAX_GRENADE_MAGSQ		( 1.0f / _MAX_GRENADE_MAGSQ )
#define _MISS_THROWER_TIME			( 0.5f )
#define _BUMP_SIZE					( 5.0f )
#define _EMP_SPHERE_TEST_TIME		( 0.2f )



CEProj *CEProj_Grenade::m_pCallbackProj;
CFCollInfo CEProj_Grenade::m_CollInfo;
CFCollInfo CEProj_Grenade::m_CollInfoFat;
CFCollInfo CEProj_Grenade::m_CollInfoEMP;
CFTrackerCollideProjSphereInfo CEProj_Grenade::m_CollProjSphereInfo;
f32 CEProj_Grenade::m_fCallbackBotShutdownTime;



BOOL CEProj_Grenade::InitSystem( void ) {
	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.bFindClosestImpactOnly = FALSE;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	m_CollInfo.bCullBacksideCollisions = TRUE;
	m_CollInfo.bCalculateImpactData = TRUE;
	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;

	fang_MemCopy( &m_CollInfoFat, &m_CollInfo, sizeof(CFCollInfo) );

	m_CollInfoEMP.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
	m_CollInfoEMP.bFindClosestImpactOnly = FALSE;
	m_CollInfoEMP.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	m_CollInfoEMP.bCullBacksideCollisions = TRUE;
	m_CollInfoEMP.bCalculateImpactData = FALSE;
	m_CollInfoEMP.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfoEMP.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfoEMP.nTrackerUserTypeBitsMask = 0xffffffffffffffff;

	m_CollProjSphereInfo.pProjSphere = &m_CollInfo.ProjSphere;
	m_CollProjSphereInfo.bIgnoreCollisionFlag = FALSE;
	m_CollProjSphereInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_CollProjSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	m_CollProjSphereInfo.pCallback = _TrackersCallbackProjSphere;

	return TRUE;
}


void CEProj_Grenade::UninitSystem( void ) {
}


BOOL CEProj_Grenade::Create( CEProj *pProj ) {
	return TRUE;
}


void CEProj_Grenade::Init( CEProj *pProj ) {
	f32 fRadius, fMass;

	fRadius = pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius;
	fMass = 50.0f;

	m_Motion.Simple_InitBody( pProj->MtxToWorld(), CFMotion::ComputeInvUniformInertiaTensorMag( fRadius, fMass ), fMass, -32.0f );
	m_fSecsUntilNextRecruit = 0.0f;
	m_fSecsUntilNextSound = 0.0f;
	m_uAISoundHandle = 0;

	m_bStopped = FALSE;
	m_bEMPDet = FALSE;
}


void CEProj_Grenade::Launched( CEProj *pProj ) {
	m_Motion.m_AngularVelocity_WS.Mul( *pProj->GetRotUnitAxis_WS(), pProj->GetRotSpeed() );
	m_Motion.m_LinearVelocity_WS.Mul( *pProj->GetLinUnitDir_WS(), pProj->GetLinSpeed() );

	m_Motion.Simple_UpdateOrientation( pProj->MtxToWorld() );

	m_fSecsUntilNextRecruit = 0.0f;
	m_fSecsUntilNextSound = 0.0f;
	m_fCantHitThrowerTime = _MISS_THROWER_TIME;
	m_bEMPDet = FALSE;
}


BOOL CEProj_Grenade::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact ) {
	m_bStopped = TRUE;

	return TRUE;
}


BOOL CEProj_Grenade::_EMPTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode ) {
	CBot *pBot = (CBot *) pTracker->m_pUser;

	if( pBot->TypeBits() & ENTITY_BIT_VEHICLE ) {
		return TRUE;
	}

	if( pBot->TypeBits() & ENTITY_BIT_WEAPON ) {
		return TRUE;
	}

	CEProj_Grenade *pProjGren = (CEProj_Grenade *)m_pCallbackProj->GetExtensionObject( CEProj::PROJTYPE_GRENADE );

	if( (pProjGren->m_SetupParams.nGrenadeType == CEPROJ_GRENADE_TYPE_EMP) && !pBot->Power_IsPoweredUp() ) {
		return TRUE;
	}

	// Corrosive doesn't do EMP's
	if( (pProjGren->m_SetupParams.nGrenadeType == CEPROJ_GRENADE_TYPE_EMP) && ( pBot->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		return TRUE;
	}

	FASSERT( m_CollInfoEMP.pSphere_WS );

	CFVec3A Dis;
	CFVec3A Center;

	Center.Set( CFVec3A::m_UnitAxisY ).Mul( pBot->m_fCollCylinderHeight_WS * 0.5f );
	Center.Add( pBot->MtxToWorld()->m_vPos );

	Dis.Sub( m_CollInfoEMP.pSphere_WS->m_Pos, Center );

	if( Dis.Dot( Dis ) <= FMATH_SQUARE( m_CollInfoEMP.pSphere_WS->m_fRadius ) ) {
		if( pProjGren->m_SetupParams.nGrenadeType == CEPROJ_GRENADE_TYPE_RECRUITER ) {
			CFVec3A BumpUp = CFVec3A::m_UnitAxisY;
			BumpUp.Mul( _BUMP_SIZE );
			BumpUp.Add( m_pCallbackProj->MtxToWorld()->m_vPos );

			if( pBot->Recruit( m_pCallbackProj->GetDamagerBot(), &BumpUp ) ) {
				// Recruit effect started...

				m_pCallbackProj->PlaySound( pProjGren->m_SetupParams.pSoundGroupRecruiterBolt );

				pProjGren->m_fSecsUntilNextRecruit = 0.5f;

				return FALSE;
			}
		} else {
			// If the bot is possessed, stun them, otherwise turn them off
			if( pBot->m_nPossessionPlayerIndex == -1 && !( pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
				pBot->Power( FALSE, m_fCallbackBotShutdownTime );
			} else {
				// Need to check possession here since Glitch can be placed in the world and controlled by scripts
				if( !pBot->IsStunned() && pBot->m_nPossessionPlayerIndex != -1 ) {
					pBot->StunBot( m_fCallbackBotShutdownTime );
				}
			}
		}
	}

	return TRUE;
}


BOOL CEProj_Grenade::_WorkDetonatedEMP( CEProj *pProj ) {
	CFSphere Sphere;
	CFVec3A BumpUp = CFVec3A::m_UnitAxisY;

	// Bump up the sphere since when we draw the effect it is bumped up as well
	BumpUp.Mul( _BUMP_SIZE );
	BumpUp.Add( pProj->m_MtxToWorld.m_vPos.v3 );

	Sphere.m_Pos = BumpUp.v3;
	Sphere.m_fRadius = m_SetupParams.fDetonationRadius;
	m_CollInfoEMP.pSphere_WS = &Sphere;
	m_fCallbackBotShutdownTime = m_SetupParams.fShutdownTime;

	m_pCallbackProj = pProj;
	fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_MESH, _EMPTrackerCallback, 0, NULL, NULL, MESHTYPES_ENTITY, ENTITY_BIT_BOT );

	return FALSE;
}

u8 CEProj_Grenade::ComputeProjectileReaction( const FCollImpact_t *pCollImpact ) {

	u8 nProjReact = CGColl::ComputeProjectileReaction( pCollImpact );
	switch (m_SetupParams.nGrenadeType) {
		case CEProjExt::CEPROJ_GRENADE_TYPE_CORING_CHARGE:
		case CEProjExt::CEPROJ_GRENADE_TYPE_EMP:
		case CEProjExt::CEPROJ_GRENADE_TYPE_RECRUITER:
		default:
			if (nProjReact == CEntity::PROJECTILE_REACTION_SHIELD)
				return CEntity::PROJECTILE_REACTION_HURT_ME;
			else
                return nProjReact;

		case CEProjExt::CEPROJ_GRENADE_TYPE_MAGMABOMB: {
			const CGCollMaterial *pMaterial = CGColl::GetMaterial( pCollImpact );
			if  ( ( nProjReact != CEntity::PROJECTILE_REACTION_SHIELD) &&
				(pMaterial->m_nFlags & CGCollMaterial::FLAG_CAN_CATCH_FIRE))
				return CEntity::PROJECTILE_REACTION_HURT_ME;
			else
				return nProjReact;
		}
	}
}

BOOL CEProj_Grenade::Work( CEProj *pProj ) {
	FASSERT( pProj );

	f32 fDistTraveledThisFrame, fMag2;
	CFSphere *pSphere;
	CFVec3A PrevPos;
	CFVec3A PrevToCurr;
	FCollImpact_t *pNearImpact = NULL;

	if( m_bEMPDet ) {
		m_fSecsUntilNextRecruit -= FLoop_fPreviousLoopSecs;
	}

	m_fSecsUntilNextSound -= FLoop_fPreviousLoopSecs;
	m_fCantHitThrowerTime -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fCantHitThrowerTime, 0.0f );

	// Handle life time...
	if( pProj->m_fMaxLifeSecs >= 0.0f ) {
		if( !m_bEMPDet && m_Motion.m_LinearVelocity_WS.MagSq() > (5.0f * 5.0f) ) {
			// Grenade is still moving. Reset life timer...
			pProj->m_fLifeSecs = 0.0f;
		} else {
			// Either the grenade has stopped moving or this is an EMP grenade...

			pProj->m_fLifeSecs += FLoop_fPreviousLoopSecs;

			// Projectile has lived its life time...
			if( pProj->m_fLifeSecs >= pProj->m_fMaxLifeSecs ) {
				return _HandleDetonation( pProj, NULL );
			}
		}

		if( m_bEMPDet ) {
			FASSERT( (m_SetupParams.nGrenadeType == CEProjExt::CEPROJ_GRENADE_TYPE_EMP) || (m_SetupParams.nGrenadeType == CEProjExt::CEPROJ_GRENADE_TYPE_RECRUITER) );

			if( m_SetupParams.nGrenadeType == CEProjExt::CEPROJ_GRENADE_TYPE_EMP ) {
				_WorkDetonatedEMP( pProj );
			} else {
				if( m_fSecsUntilNextRecruit <= 0.0f ) {
					_WorkDetonatedEMP( pProj );
				}
			}
		}
	}

	if( m_bStopped ) {
		return FALSE;
	}

	pSphere = (CFSphere *) &pProj->GetWorldMesh()->GetBoundingSphere();

	// Compute linear distance we've traveled this frame...
	fDistTraveledThisFrame = pProj->GetLinSpeed() * FLoop_fPreviousLoopSecs;
	pProj->m_fDistTraveled_WS += fDistTraveledThisFrame;

	// Move the body...
	PrevPos.Set( m_Motion.m_Mtx.m_vPos );
	pSphere->m_Pos = m_Motion.m_Mtx.m_vPos.v3;
	m_Motion.Simple_Simulate( FLoop_fPreviousLoopSecs );

	PrevToCurr.Sub( m_Motion.m_Mtx.m_vPos, PrevPos );

	// Compute current points...
	if( m_SetupParams.nGrenadeType == CEProjExt::CEPROJ_GRENADE_TYPE_CORING_CHARGE ) {
		m_CurP1.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius *  0.9f ).Add( m_Motion.m_Mtx.m_vPos );
		m_CurP2.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius * -0.9f ).Add( m_Motion.m_Mtx.m_vPos );
	} else {
		m_CurP1.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius *  0.5f ).Add( m_Motion.m_Mtx.m_vPos );
		m_CurP2.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius * -0.5f ).Add( m_Motion.m_Mtx.m_vPos );
	}

	// Perform collision...
	m_CollInfo.ProjSphere.Init( &PrevPos, &m_Motion.m_Mtx.m_vPos, pSphere->m_fRadius );
	m_CollInfoFat.ProjSphere.Init( &PrevPos, &m_Motion.m_Mtx.m_vPos, pSphere->m_fRadius + m_SetupParams.fFatRadiusDelta );

	if( m_fCantHitThrowerTime > 0.0f ) {
		// Ignore thrower and grenade...
		m_CollProjSphereInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
		m_CollProjSphereInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;
	} else {
		// Ignore just the grenade...
		m_CollProjSphereInfo.nTrackerSkipCount = 1;
		m_CollProjSphereInfo.ppTrackerSkipList = (const CFWorldTracker * const *)&pProj->m_pWorldMesh;
	}

	m_CollInfo.pTag = NULL;
	fcoll_Clear();
	fworld_CollideWithWorldTris( &m_CollInfo );
	fworld_CollideWithTrackers( &m_CollProjSphereInfo, pProj->GetWorldMesh() );

	BOOL bFoundCollision = FALSE;

	if( FColl_nImpactCount ) {
		f32 fLowest;
		u32 i;

		for( i=0; i<FColl_nImpactCount; i++ ) {
			if( !bFoundCollision || (FColl_aImpactBuf[i].fImpactDistInfo < fLowest) ) {
#if 0
				// Ignore collisions with the thrower bot if it's too soon from launch time...

				if( FColl_aImpactBuf[i].pTag ) {
					// This is a tracker...

					pWorldTracker = (CFWorldTracker *) FColl_aImpactBuf[i].pTag;

					if( pWorldTracker->m_nUser == MESHTYPES_ENTITY ) {
						// This is an entity...

						pEntity = (CEntity *)pWorldTracker->m_pUser;

						if( pEntity == pProj->GetDamager()->pBot ) {
							// This is the throwing entity...
							if( m_fCantHitThrowerTime > 0.0f ) {
								// Ignore this collision...
								continue;
							}
						}
					}
				}
#endif

				pNearImpact = &FColl_aImpactBuf[i];
				fLowest = FColl_aImpactBuf[i].fImpactDistInfo;

				bFoundCollision = TRUE;
			}
		}
	}

	if( bFoundCollision ) {
		if( pProj->WillDetonateOnImpact() ) {
			return _HandleDetonation( pProj, pNearImpact );
		}

		u8 nProjReact = ComputeProjectileReaction( pNearImpact);
		if( (nProjReact == CEntity::PROJECTILE_REACTION_DEATH_TO_ALL) || (nProjReact == CEntity::PROJECTILE_REACTION_HURT_ME)) {
			return _HandleDetonation( pProj, pNearImpact );
		}

		// Move close to the point of impact...

		fMag2 = PrevToCurr.MagSq();
		if( fMag2 > 0.0001f ) {
			CFVec3A PushVec;

			PrevToCurr.Mul( fmath_InvSqrt(fMag2) );
			PushVec.Mul( PrevToCurr, pNearImpact->fImpactDistInfo - 0.09f );

			m_Motion.m_Mtx.m_vPos.Add( PrevPos, PushVec );
			_HandleCollision( pProj, pNearImpact );
		} else {
			m_Motion.m_Mtx.m_vPos = PrevPos;
		}
	} else {
		// We didn't collide...

		if( pProj->GetDamagerBot() && (pProj->GetDamagerBot()->m_nPossessionPlayerIndex > -1) ) {
			if( m_fSecsUntilNextSound <=0.0f ) {   //pgm believe this means every time we bounce....
				//every time we bounce, declare it to the AI system.
				if( !m_uAISoundHandle || !AIEnviro_ModifySound( m_uAISoundHandle,  m_Motion.m_Mtx.m_vPos, AIEnviro_fProjectileVisualRadius, 0.3f, AISOUNDTYPE_PROJECTILE, pProj->GetDamagerBot()) ) {
					m_uAISoundHandle = AIEnviro_AddSound( m_Motion.m_Mtx.m_vPos, AIEnviro_fProjectileVisualRadius, 0.3f, AISOUNDTYPE_PROJECTILE, AISOUNDCTRL_VISIBLE_ENTITY, pProj->GetDamagerBot() );
				}
			}
		}
	}

	pProj->Relocate_RotXlatFromUnitMtx_WS( &m_Motion.m_Mtx );
	pSphere->m_Pos = m_Motion.m_Mtx.m_vPos.v3;

	return FALSE;
}


void CEProj_Grenade::SetGrenadeParams( const CEProj_Grenade_Params_t *pParms ) {
	FASSERT( pParms );
	FASSERT( pParms->nGrenadeType == CEPROJ_GRENADE_TYPE_CORING_CHARGE || 
			 pParms->nGrenadeType == CEPROJ_GRENADE_TYPE_EMP ||
			 pParms->nGrenadeType == CEPROJ_GRENADE_TYPE_RECRUITER ||
			 pParms->nGrenadeType == CEPROJ_GRENADE_TYPE_MAGMABOMB 
			 );

	fang_MemCopy( &m_SetupParams, pParms, sizeof( CEProj_Grenade_Params_t ) );
}


BOOL CEProj_Grenade::_HandleDetonation( CEProj *pProj, FCollImpact_t *pNearImpact ) {
	m_bStopped = TRUE;

	if( (m_SetupParams.nGrenadeType != CEProjExt::CEPROJ_GRENADE_TYPE_EMP) && (m_SetupParams.nGrenadeType != CEProjExt::CEPROJ_GRENADE_TYPE_RECRUITER) )  {
		// Non-EMP grenade...

		if( pNearImpact ) {
			pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, pNearImpact );
		} else {
			pProj->Detonate( TRUE, CEProj::EVENT_LIFE_TIME_OVER, NULL );
		}

		return TRUE;
	} else {
		// EMP grenade...

		if( !m_bEMPDet ) {
			// Spawn EMP effect...

			pProj->SpawnExplosionEffect( CEProj::EVENT_LIFE_TIME_OVER, NULL );

			pProj->DrawEnable( FALSE );

			CFVec3A BumpUp = CFVec3A::m_UnitAxisY;

			BumpUp.Mul( _BUMP_SIZE );
			BumpUp.Add( m_Motion.m_Mtx.m_vPos );

			m_bEMPDet = TRUE;
			m_fSecsUntilNextRecruit = 0.5f;

			pProj->m_fLifeSecs = 0.0f;
			pProj->m_fMaxLifeSecs = m_SetupParams.fDetonationLife;

			BOOL bRecruiter = (m_SetupParams.nGrenadeType == CEProjExt::CEPROJ_GRENADE_TYPE_RECRUITER);

			CFXEMPBlast::SpawnBlast( pProj->m_fMaxLifeSecs, m_SetupParams.fDetonationRadius, BumpUp, m_SetupParams.pSoundGroupEMPEffect, bRecruiter );

			return FALSE;
		} else {
			// Destroy the actual projectile...

			m_bEMPDet = FALSE;
			pProj->Detonate( FALSE, CEProj::EVENT_LIFE_TIME_OVER, NULL );

			return TRUE;
		}
	}

	return TRUE;
}


void CEProj_Grenade::_HandleCollision( CEProj *pProj, FCollImpact_t *pNearImpact ) {
	FASSERT( pProj );
	FASSERT( pNearImpact );

	CFVec3A UnitFaceNormal, AbsNormal;
	CFVec3A VelSurface;
	CFVec3A VelN, VelT; // velocity normal and tangent to plane
	f32 fPushDist1, fPushDist2;

	UnitFaceNormal.Set( pNearImpact->UnitFaceNormal );

	// Find push vec for spinning
	f32 fPlaneD = -UnitFaceNormal.Dot( pNearImpact->ImpactPoint );

	fPushDist1 = m_CurP1.Dot( UnitFaceNormal ) + fPlaneD;
	fPushDist2 = m_CurP2.Dot( UnitFaceNormal ) + fPlaneD;

	// Fake spinning
	m_Motion.m_AngularMomentum_WS.Set( 50.0f * fPushDist2, pNearImpact->fImpactDistInfo * 60.0f, fPushDist1 * 50.0f );
	m_Motion.m_AngularVelocity_WS.Set( 70.0f * fPushDist1, 70.0f * pNearImpact->fImpactDistInfo, 70.0f * fPushDist2 );

	AbsNormal.x = FMATH_FABS( UnitFaceNormal.x );
	AbsNormal.y = FMATH_FABS( UnitFaceNormal.y );
	AbsNormal.z = FMATH_FABS( UnitFaceNormal.z );
	
	// Reflect the velocity about the plane's normal
	VelN = UnitFaceNormal;
	VelN.Mul( UnitFaceNormal.Dot( m_Motion.m_LinearVelocity_WS ) );
	VelT.Sub( m_Motion.m_LinearVelocity_WS, VelN );
	VelSurface.Sub( VelT, VelN );

	// Ground surface
	if( (AbsNormal.y >= FMATH_MAX( AbsNormal.x, AbsNormal.z )) && (UnitFaceNormal.y > 0.0f) ) {
		CFVec3A SubVec;

		SubVec = VelSurface;
		SubVec.Mul( m_SetupParams.FloorDampening );
		VelSurface.Sub( SubVec );

		FMATH_CLAMP(VelSurface.y, -20.0f, 20.0f );
	} else {
		// Wall surface
		CFVec3A SubVec;
		CFVec3A VelNorm;
		f32 fScale, fMag2;

		SubVec = VelSurface;
		SubVec.Mul( m_SetupParams.FloorDampening );
		VelSurface.Sub( SubVec );

		FMATH_CLAMP(VelSurface.y, -20.0f, 20.0f );

		// Dampen the velocity more based on the angle we hit the wall at...
		if( m_SetupParams.bUseAngularWallDampening ) {
			fMag2 = m_Motion.m_LinearVelocity_WS.MagSq();
			if( fMag2 > 0.0001f ) {
				VelNorm.Mul( m_Motion.m_LinearVelocity_WS, fmath_InvSqrt(fMag2) );

				fScale = ( 1.0f - FMATH_FABS( VelNorm.Dot( UnitFaceNormal ) ) );
				FMATH_CLAMP( fScale, m_SetupParams.afAngularWallDampeningRange[ 0 ], m_SetupParams.afAngularWallDampeningRange[ 1 ] );

				if( AbsNormal.x > AbsNormal.z ) {
					VelSurface.x *= fScale;	
				} else {
					VelSurface.z *= fScale;
				}
			}
		}
	}

	// Don't go too fast
	if( VelSurface.MagSq() > _MAX_GRENADE_MAGSQ ) {
		VelSurface.Unitize();
		VelSurface.Mul( _MAX_GRENADE_MAG );
	}

	m_Motion.m_LinearVelocity_WS = VelSurface;

	f32 fMagSq;

	fMagSq = m_Motion.m_LinearVelocity_WS.MagSq();

	if( fMagSq <= 110.0f ) {
		// Only stop completely on a gound surface that points up
		if( AbsNormal.y > FMATH_MAX( AbsNormal.x, AbsNormal.z ) && UnitFaceNormal.y > 0.0f ) {
			CFMtx43A Mtx;
			CFVec3A NewGrenUp;

			m_Motion.m_LinearVelocity_WS.Zero();
			m_Motion.m_AngularVelocity_WS.Zero();
			m_Motion.m_AngularMomentum_WS.Zero();	
			m_Motion.m_Quat.Identity();

			// Align grenade to surface
			NewGrenUp = m_Motion.m_Mtx.m_vUp;
			NewGrenUp.y = 0.0f;

			if( NewGrenUp.MagSq() < 0.001f ) {
				NewGrenUp.Set( CFVec3A::m_UnitAxisX );
			} else {
				NewGrenUp.Unitize();
			}

			Mtx.m_vFront = UnitFaceNormal;
			Mtx.m_vUp.Cross( Mtx.m_vFront, NewGrenUp );
			Mtx.m_vRight.Cross( Mtx.m_vFront, Mtx.m_vUp );
			Mtx.m_vPos = m_Motion.m_Mtx.m_vPos;

			m_Motion.m_Mtx = Mtx;

			// Stick grenade to surface
			f32 fDistanceToPlane = UnitFaceNormal.Dot( m_Motion.m_Mtx.m_vPos ) - UnitFaceNormal.Dot( pNearImpact->ImpactPoint );
			f32 fRadiusScale = pProj->GetWorldMesh()->GetBoundingSphere().m_fRadius * m_SetupParams.fRadiusPercentUse;
			f32 fAddOn;

			if( fDistanceToPlane > fRadiusScale ) {
				fAddOn = -( fDistanceToPlane - fRadiusScale );
			} else {
				if( fDistanceToPlane < 0.0f ) {
					fAddOn = -fDistanceToPlane + fRadiusScale;
				} else {
					fAddOn = fRadiusScale;
				}
			}

			CFVec3A MoveDir = UnitFaceNormal;
			
			MoveDir.Mul( fAddOn );
			m_Motion.m_Mtx.m_vPos.Add( MoveDir );

			// We are stopped, no more gravity
			m_Motion.m_fGravity = 0.0f;

			m_bStopped = TRUE;
		} else {
			// If we didn't hit a surface we can stop on and our velocity is low, 
			// boost off of surface so we don't stick to it
			CFVec3A VelAdd;

			VelAdd = UnitFaceNormal;
			VelAdd.Mul( fmath_Sqrt( fMagSq ) );

			m_Motion.m_LinearVelocity_WS.Add( VelAdd );
		}
	}

	if( fMagSq > 3.0f ) {
		if( m_fSecsUntilNextSound <= 0.0f ) {
			f32 fUnitSoundVol = fMagSq * _OO_MAX_GRENADE_MAGSQ;
			FMATH_CLAMP( fUnitSoundVol, 0.0f, 1.0f );
			fUnitSoundVol = FMATH_FPOT( fUnitSoundVol, 0.5f, 1.0f );

			CFSoundGroup::PlaySound( m_SetupParams.pSoundGroupBounce, FALSE, &pNearImpact->ImpactPoint, pProj->m_Damager.nDamagerPlayerIndex, TRUE, fUnitSoundVol );

			m_fSecsUntilNextSound = _SECS_BETWEEN_PING_SOUNDS;
		}
	}
}


BOOL CEProj_Grenade::_TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode ) {
	if( pTracker->m_nUser == FWORLD_USERTYPE_WIRE ) {
		// Ignore collision with wires...

		return TRUE;
	}

	CFCollInfo *pCollInfo = &m_CollInfo;

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
		// This is an entity...

		CEntity *pEntity = (CEntity *)pTracker->m_pUser;

		if( pEntity->GetProjectileReaction() == CEntity::PROJECTILE_REACTION_HURT_ME ) {
			// This entity will detonate the grenade...

			pCollInfo = &m_CollInfoFat;
		}
	}

	pCollInfo->pTag = pTracker;
	((CFWorldMesh *)pTracker)->CollideWithMeshTris( pCollInfo );

	return TRUE;
}



