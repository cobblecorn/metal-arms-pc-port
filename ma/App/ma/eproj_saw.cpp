//////////////////////////////////////////////////////////////////////////////////////
// eproj_saw.cpp - Saw projectiles.
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
#include "eproj_saw.h"
#include "gcoll.h"
#include "floop.h"
#include "fworld_coll.h"
#include "muzzleflash.h"
#include "weapon.h"
#include "fclib.h"
#include "fresload.h"
#include "fres.h"
#include "smoketrail.h"
#include "meshtypes.h"
#include "damage.h"
#include "fsound.h"



#define _DEFAULT_CARVE_DAMAGE_PROFILE	(&CDamage::m_DamageProfileNone)
#define _DEFAULT_CARVE_DAMAGE_SECS		3.5f
#define _DEFAULT_CARVE_INITIAL_SPEED	7.0f
#define _DEFAULT_MIN_RICOCHET_COUNT		4
#define _DEFAULT_MAX_RICOCHET_COUNT		10
#define _DEFAULT_SOUND_RADIUS			400.0f
#define _DEFAULT_CARVING_ALLOWED		FALSE


#define _STICK_SECS						2.0f
#define _SHRINK_SECS					2.0f
#define _CARVEMODE_QUAT_ROT_SPEED		(FMATH_2PI * 1.0f)
#define _CARVEMODE_SPIN_SPEED			(3.0f * FMATH_2PI)
#define _CARVEMODE_SPIN_DECELERATION	(4.0f * FMATH_2PI)
#define _INITIAL_ROT_VELOCITY_RANGE		0.5f
#define _RICOCHET_ROT_VELOCITY_RANGE	10.0f
#define _SPAWN_CARVING_SPARKS_SECS		0.125f



u32 CEProj_Saw::m_nCarvingCount;
SmokeTrailAttrib_t CEProj_Saw::m_ImpactSmokeTrailAttrib;
SmokeTrailAttrib_t CEProj_Saw::m_CarveSmokeTrailAttrib;
CFCollInfo CEProj_Saw::m_CollInfo;
CFCollInfo CEProj_Saw::m_CollInfoNearbySphereTest;
CFTrackerCollideRayInfo CEProj_Saw::m_CollRayInfo;
FCollImpact_t CEProj_Saw::m_CollImpact;
CFVec3A CEProj_Saw::m_ImpactNormal;
CFVec3A CEProj_Saw::m_ImpactPoint;
CEntity *CEProj_Saw::m_pHitEntity;
CEProj *CEProj_Saw::m_pCollProj;
const CDamageProfile *CEProj_Saw::m_pCollDamageProfile;
CDamageForm *CEProj_Saw::m_apDamageFormArray[DAMAGE_FORM_COUNT];
u32 CEProj_Saw::m_nDamageFormCount;



BOOL CEProj_Saw::InitSystem( void ) {
	_SetSmokeTrailAttributes();

	m_nCarvingCount = 0;

	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
	m_CollInfo.bFindClosestImpactOnly = FALSE;
	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_CollInfo.bCullBacksideCollisions = TRUE;

	m_CollInfoNearbySphereTest.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
	m_CollInfoNearbySphereTest.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfoNearbySphereTest.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfoNearbySphereTest.bCalculateImpactData = TRUE;
	m_CollInfoNearbySphereTest.nTrackerUserTypeBitsMask = -1;
	m_CollInfoNearbySphereTest.nStopOnFirstOfCollMask = 0;
	m_CollInfoNearbySphereTest.bFindClosestImpactOnly = TRUE;
	m_CollInfoNearbySphereTest.bCullBacksideCollisions = FALSE;
	m_CollInfoNearbySphereTest.pSphere_WS = NULL;
	m_CollInfoNearbySphereTest.pTag = NULL;

	m_CollRayInfo.pCallback = _CollRayWithTrackersCallback;
	m_CollRayInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	m_CollRayInfo.bIgnoreCollisionFlag = FALSE;
	m_CollRayInfo.bComputeIntersection = FALSE;
	m_CollRayInfo.bSort = FALSE;
	m_CollRayInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	m_CollRayInfo.nTrackerSkipCount = 0;
	m_CollRayInfo.ppTrackerSkipList = NULL;

	return TRUE;
}


void CEProj_Saw::UninitSystem( void ) {
}


BOOL CEProj_Saw::Create( CEProj *pProj ) {
	Init( pProj );

	return TRUE;
}


void CEProj_Saw::Destroy( CEProj *pProj ) {
	if( m_nState == STATE_CARVING_SURFACE ) {
		FASSERT( m_nCarvingCount );
		--m_nCarvingCount;
	}
}


void CEProj_Saw::Init( CEProj *pProj ) {
	m_nState = STATE_DEAD;
	m_fCountdownTimer = 0.0f;
	m_fCarveDamageCountdownTimer = 0.0f;
	m_pCarveDamageProfile = NULL;
	m_fCarveSurfaceJumpAngle = 0.0f;
	m_fCarveDeceleration = 0.0f;
	m_fCarveSparkTimer = 0.0f;
	m_pAudioEmitterCarving = NULL;
	m_fSawSpinAngle = 0.0f;
	m_nRicochetCount = 0;
	m_fDeltaShrinkRate = 0.0f;
	m_pCarveEntity = NULL;
	m_CarveMeshQuat.Identity();

	CEProj_Saw::GetDefaultSawParams( pProj, &m_SetupParms );
	m_fCarveDeceleration = fmath_Div( m_SetupParms.fCarveInitialSpeed, m_SetupParms.fCarveDamageSecs );

	pProj->m_RotUnitAxis_WS = CFVec3A::m_UnitAxisZ;
	pProj->m_fRotSpeed_WS = fmath_RandomFloatRange( -_INITIAL_ROT_VELOCITY_RANGE, _INITIAL_ROT_VELOCITY_RANGE );

	FMATH_CLEARBITMASK( pProj->m_nProjFlags, CEProj::PROJFLAG_MANUAL_XFM );
}


void CEProj_Saw::GetDefaultSawParams( CEProj *pProj, CEProj_Saw_Params_t *pParms ) {
	pParms->nMinRicochetCount = _DEFAULT_MIN_RICOCHET_COUNT;
	pParms->nMaxRicochetCount = _DEFAULT_MAX_RICOCHET_COUNT;
	pParms->bCarvingAllowed = _DEFAULT_CARVING_ALLOWED;
	pParms->pCarveDamageProfile = _DEFAULT_CARVE_DAMAGE_PROFILE;
	pParms->fCarveDamageSecs = _DEFAULT_CARVE_DAMAGE_SECS;
	pParms->fCarveInitialSpeed = _DEFAULT_CARVE_INITIAL_SPEED;

	pParms->pSoundGroupRicochet = NULL;
	pParms->pSoundGroupStick = NULL;
	pParms->pSoundGroupCutStart = NULL;
	pParms->pSoundGroupCutStop = NULL;
	pParms->pSoundGroupCutLoop = NULL;
}


void CEProj_Saw::SetSawParams( CEProj *pProj, const CEProj_Saw_Params_t *pParms ) {
	fang_MemCopy( &m_SetupParms, pParms, sizeof(CEProj_Saw_Params_t) );

	m_fCarveDeceleration = fmath_Div( m_SetupParms.fCarveInitialSpeed, m_SetupParms.fCarveDamageSecs );
	m_pCarveDamageProfile = pParms->pCarveDamageProfile;
}


void CEProj_Saw::Launched( CEProj *pProj ) {
	m_nState = STATE_FLYING;
}


BOOL CEProj_Saw::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact ) {
	_KillCarvingSound( pProj );

	if( m_nState == STATE_CARVING_SURFACE ) {
		FASSERT( m_nCarvingCount );
		--m_nCarvingCount;
	}

	pProj->SmokeTrailOff();
	pProj->DetachFromParent();

	if( pProj->m_pWorldMesh ) {
		pProj->m_pWorldMesh->RemoveFromWorld();
	}

	m_nState = STATE_DEAD;

	return TRUE;
}


// Returns TRUE if the projectile has been destroyed.
BOOL CEProj_Saw::Work( CEProj *pProj ) {
	f32 fDistTraveledThisFrame;
	BOOL bRemovedFromWorld = FALSE;

	// Compute linear distance we've traveled this frame...
	fDistTraveledThisFrame = pProj->GetLinSpeed() * FLoop_fPreviousLoopSecs;
	pProj->m_fDistTraveled_WS += fDistTraveledThisFrame;

	// Update scale...
	if( m_fDeltaShrinkRate > 0.0f ) {
		f32 fNewScale = pProj->m_fScaleToWorld - m_fDeltaShrinkRate * FLoop_fPreviousLoopSecs;

		if( fNewScale <= 0.0001f ) {
			pProj->Detonate( FALSE );
			return TRUE;
		}

		pProj->Relocate_Scale_WS( fNewScale );
	}

	switch( m_nState ) {
	case STATE_FLYING:
		bRemovedFromWorld = _Work_Flying( pProj, fDistTraveledThisFrame );
		break;

	case STATE_CARVING_SURFACE:
		bRemovedFromWorld = _Work_Carving( pProj );
		break;

	case STATE_WAITING_TO_SHRINK:
		_Work_Sticking( pProj );
		break;
	}

	return bRemovedFromWorld;
}


// Returns TRUE if the projectile has been destroyed.
BOOL CEProj_Saw::_Work_Flying( CEProj *pProj, f32 fDistTraveledThisFrame ) {
	CFVec3A StartCollPoint_WS, EndCollPoint_WS;
	CFMtx43A NewMtx, RotMtx;
	CFQuatA Quat;
	BOOL bShatterAfterRicochet;

	NewMtx.m_vPos.Mul( pProj->m_LinUnitDir_WS, fDistTraveledThisFrame );
	NewMtx.m_vPos.Add( pProj->MtxToWorld()->m_vPos );

	// Compute projectile orientation...
	Quat.BuildQuat( pProj->m_RotUnitAxis_WS, 2.0f * pProj->m_fRotSpeed_WS * FLoop_fPreviousLoopSecs );
	Quat.BuildMtx( RotMtx );
	NewMtx.Mul33( *pProj->MtxToWorld(), RotMtx );

	// Perform collision...
	StartCollPoint_WS = pProj->MtxToWorld()->m_vPos;
	EndCollPoint_WS = NewMtx.m_vPos;

	if( !pProj->PerformThickRayCollisionTest( &m_CollImpact, &pProj->MtxToWorld()->m_vPos, &NewMtx, FALSE ) ) {
		// No collision. Simply update our projectile matrix...

		pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx );
		return FALSE;
	}

	// We hit something!

	CFWire *pHitWire = CGColl::ExtractWire( &m_CollImpact );
	if( pHitWire ) {
		// We hit a wire. Shatter it and let saw live...

		CEntity::ShatterWire( pHitWire );

		pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx );
		return FALSE;
	}

	bShatterAfterRicochet = FALSE;

	m_ImpactNormal = m_CollImpact.UnitFaceNormal;
	m_ImpactPoint = m_CollImpact.ImpactPoint;
	m_pHitEntity = CGColl::ExtractEntity( &m_CollImpact );

	u8 nProjReact = CGColl::ComputeProjectileReaction( &m_CollImpact );

	if( nProjReact == CEntity::PROJECTILE_REACTION_DEFAULT ) {
		if( (m_nRicochetCount >= m_SetupParms.nMaxRicochetCount) || 
			((m_nRicochetCount >= m_SetupParms.nMinRicochetCount) && (fmath_RandomFloat() > 0.5f)) )
		{
			// Stick saw to surface...
			nProjReact = CEntity::PROJECTILE_REACTION_STICKY;
		} else {
			// Ricochet saw off surface...
			nProjReact = CEntity::PROJECTILE_REACTION_RICOCHET;
		}
	} else if( nProjReact == CEntity::PROJECTILE_REACTION_DEATH_TO_ALL ) {
		// Shatter saw...
		nProjReact = CEntity::PROJECTILE_REACTION_RICOCHET;
		bShatterAfterRicochet = TRUE;
	}

	if( (nProjReact == CEntity::PROJECTILE_REACTION_STICKY) || (nProjReact == CEntity::PROJECTILE_REACTION_HURT_ME) ) {
		if( !CGColl::GetMaterial( &m_CollImpact )->HasLooseDust() ) {
			// Try to start carve mode...
			if( _StartCarveMode( pProj ) ) {
				// Carve mode started ok...

				return _DealImpactDamageToEntity( pProj, TRUE );
			}
		}

		// Could not start carve mode. Try to stick saw to the entity we hit...
		if( _StartStickModeFromImpact( pProj ) ) {
			// Stick mode started ok...

			return _DealImpactDamageToEntity( pProj, TRUE );
		}

		// Could not set stick mode. Shatter saw...
		bShatterAfterRicochet = TRUE;
	}

	// Ricochet (and possibly shatter)...

	// Move saw out of the surface...
	NewMtx.m_vPos.Mul( m_ImpactNormal, m_CollImpact.fImpactDistInfo + 0.02f ).Add( m_ImpactPoint );

	// Compute new velocity vector...
	pProj->m_LinUnitDir_WS.Reflect( m_ImpactNormal );

	// Comute new rotational velocity...
	pProj->m_fRotSpeed_WS = fmath_RandomFloatRange( -_RICOCHET_ROT_VELOCITY_RANGE, _RICOCHET_ROT_VELOCITY_RANGE );

	// Make a flash at the impact point...
	_DrawFlashAtImpact( &NewMtx.m_vPos );

	// Draw some sparks...
	_DrawImpactParticles( &NewMtx.m_vPos, &pProj->m_LinUnitDir_WS, &pProj->m_LinUnitDir_WS, 1.0f, 1.0f );

	// Make ricochet sound...
	CFSoundGroup::PlaySound( m_SetupParms.pSoundGroupRicochet, FALSE, &m_ImpactPoint, -1, TRUE );

	pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx );

	if( _DealImpactDamageToEntity( pProj, FALSE ) ) {
		// Projectile has been destroyed...
		return TRUE;
	}

	if( bShatterAfterRicochet ) {
		pProj->Detonate();
		return TRUE;
	}

	++m_nRicochetCount;

	return FALSE;
}


// Returns TRUE if the saw was destroyed as a result of dealing the damage.
BOOL CEProj_Saw::_DealImpactDamageToEntity( CEProj *pProj, BOOL bDealDamageToNearbyEntities ) {
	BOOL bProjDestroyed = FALSE;

	// First, deal damage to the entity that the saw actually hit...
	if( m_pHitEntity ) {
		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		if( pDamageForm ) {
			// Fill out the form...

			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile = pProj->GetDamageProfile();
			pDamageForm->m_Damager = *pProj->GetDamager();
			pDamageForm->m_pDamageeEntity = m_pHitEntity;
			pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *)m_CollImpact.pTag, &m_CollImpact, &pProj->m_LinUnitDir_WS );

			CDamage::SubmitDamageForm( pDamageForm );

			bProjDestroyed = !pProj->IsInWorld();
		}
	}

	// Next, deal damage to nearby entities...
	if( bDealDamageToNearbyEntities && !bProjDestroyed ) {
		bProjDestroyed = _DamageNearbyEntities( pProj, pProj->GetDamageProfile() );
	}

	return bProjDestroyed;
}


// m_pHitEntity, m_ImpactNormal, m_ImpactPoint, and m_CollImpact must be set before calling.
// Returns FALSE if stick mode could not be started.
BOOL CEProj_Saw::_StartStickModeFromImpact( CEProj *pProj ) {
	CFVec3A ReflectionVector;
	cchar *pszName;
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)m_CollImpact.pTag;

	if( m_fDeltaShrinkRate > 0.0f ) {
		// Can't stick when shrinking...
		return FALSE;
	}

	if( m_pHitEntity && !m_pHitEntity->IsInWorld() ) {
		return FALSE;
	}

	// Move saw out of surface...
	pProj->Relocate_Xlat_WS( &m_ImpactPoint );

	_StartStickMode( pProj );

	// Attach saw to hit entity...
	if( m_pHitEntity ) {
		pszName = NULL;
		if( pWorldMesh->m_pMesh->pBoneArray ) {
			pszName = &pWorldMesh->m_pMesh->pBoneArray[m_CollImpact.nBoneIndex].szName[0];
		}

		pProj->Attach_ToParent_WS( m_pHitEntity, pszName );
	}

	ReflectionVector.ReceiveReflection( pProj->m_LinUnitDir_WS, m_ImpactNormal );

	// Make a flash at the impact point...
	_DrawFlashAtImpact( &m_ImpactPoint );

	// Draw some particles...
	_DrawImpactParticles( &m_ImpactPoint, &ReflectionVector, &m_ImpactNormal, 1.0f, 1.0f );

	// Make stick sound...
	CFSoundGroup::PlaySound( m_SetupParms.pSoundGroupStick, FALSE, &m_ImpactPoint, -1, TRUE );

	return TRUE;
}


void CEProj_Saw::_StartStickMode( CEProj *pProj ) {
	if( m_nState == STATE_CARVING_SURFACE ) {
		FASSERT( m_nCarvingCount );
		--m_nCarvingCount;

		FMATH_CLEARBITMASK( pProj->m_nProjFlags, CEProj::PROJFLAG_MANUAL_XFM );
	}

	m_nState = STATE_WAITING_TO_SHRINK;
	m_fCountdownTimer = _STICK_SECS;

	pProj->m_fLinSpeed_WS = 0.0f;
}


void CEProj_Saw::_Work_Sticking( CEProj *pProj ) {
	if( m_fCountdownTimer > 0.0f ) {
		m_fCountdownTimer -= FLoop_fPreviousLoopSecs;
		if( m_fCountdownTimer <= 0.0f ) {
			m_fCountdownTimer = 0.0f;

			m_fDeltaShrinkRate = pProj->m_fScaleToWorld * (1.0f / _SHRINK_SECS);
		}
	}
}


// m_pHitEntity, m_ImpactNormal, m_ImpactPoint, and m_CollImpact must be set before calling.
// Returns FALSE if carve mode could not be started.
BOOL CEProj_Saw::_StartCarveMode( CEProj *pProj ) {
	cchar *pszName;
	CFVec3A ReflectionVector;
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)m_CollImpact.pTag;

	if( !m_SetupParms.bCarvingAllowed ) {
		// Carving not allowed with this projectile...
		return FALSE;
	}

	if( m_fDeltaShrinkRate > 0.0f ) {
		// Can't carve when shrinking...
		return FALSE;
	}

	if( m_nCarvingCount >= MAX_CARVE_PROJ_COUNT ) {
		// Too many carvers active...
		return FALSE;
	}

	if( m_pHitEntity && !m_pHitEntity->IsInWorld() ) {
		return FALSE;
	}

	// Compute new matrix...
	if( !_ComputeCarveModeMtxAndRelocate( pProj ) ) {
		return FALSE;
	}

	m_fCarveSparkTimer = _SPAWN_CARVING_SPARKS_SECS;

	// Set carving speed...
	pProj->m_fLinSpeed_WS = m_SetupParms.fCarveInitialSpeed;

	// Init other variables...
	pProj->StreamerOff();
	FMATH_SETBITMASK( pProj->m_nProjFlags, CEProj::PROJFLAG_MANUAL_XFM );
	m_fCarveSurfaceJumpAngle = 0.0f;
	m_fCarveDamageCountdownTimer = m_pCarveDamageProfile->m_fDamagePeriod;
	pProj->m_fRotSpeed_WS = _CARVEMODE_SPIN_SPEED;

	// Build current quaternion...
	if( pProj->m_pWorldMesh->m_Xfm.m_fScaleR == 1.0f ) {
		m_CarveMeshQuat.BuildQuat( pProj->m_pWorldMesh->m_Xfm.m_MtxF );
	} else {
		m_CarveMeshQuat.BuildQuat( pProj->m_pWorldMesh->m_Xfm.m_MtxF, pProj->m_pWorldMesh->m_Xfm.m_fScaleR );
	}

	// Set new state...
	m_nState = STATE_CARVING_SURFACE;

	m_pCarveEntity = m_pHitEntity;

	if( m_pHitEntity ) {
		// Attach saw to hit entity...
		pszName = NULL;
		if( pWorldMesh->m_pMesh->pBoneArray ) {
			pszName = &pWorldMesh->m_pMesh->pBoneArray[m_CollImpact.nBoneIndex].szName[0];
		}

		pProj->Attach_ToParent_WS( m_pHitEntity, pszName );
	}

	// Account for new carver...
	++m_nCarvingCount;

	ReflectionVector.ReceiveReflection( pProj->m_LinUnitDir_WS, m_ImpactNormal );

	// Make a flash at the impact point...
	_DrawFlashAtImpact( &pProj->MtxToWorld()->m_vPos );

	// Draw some particles...
	_DrawImpactParticles( &pProj->MtxToWorld()->m_vPos, &ReflectionVector, &m_ImpactNormal, 1.0f, 1.0f );

	// Make stick sound...
	CFSoundGroup::PlaySound( m_SetupParms.pSoundGroupStick, FALSE, &m_ImpactPoint, -1, TRUE );

	// Start spawning carve smoke...
	pProj->SetSmokePos_MS( &CFVec3A( 0.1f, 0.0f, 0.5f ) );
	pProj->SmokeTrailOn( &m_CarveSmokeTrailAttrib );

	// Start carving sound...
	_StartCarvingSound( pProj );

	return TRUE;
}


void CEProj_Saw::_DrawImpactParticles( const CFVec3A *pPos_WS, const CFVec3A *pSparksUnitDir_WS, const CFVec3A *pDustUnitDir_WS, f32 fSparksUnitIntensity, f32 fDustUnitIntensity ) {
	const CGCollMaterial *pMaterial = CGColl::GetMaterial( &m_CollImpact );

	pMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST, pPos_WS, pSparksUnitDir_WS, fSparksUnitIntensity );
	pMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST, pPos_WS, pDustUnitDir_WS, fDustUnitIntensity );
}


void CEProj_Saw::_DrawFlashAtImpact( const CFVec3A *pPos_WS ) {
	if( !CGColl::GetMaterial( &m_CollImpact )->IsImpactFlashEnabled() ) {
		return;
	}

	CFVec3A ImpactFlashPos;
	ImpactFlashPos.Mul( m_ImpactNormal, m_SetupParms.fMuzzleOffset_Impact ).Add( *pPos_WS );

	// Make a flash at the impact point...
	CMuzzleFlash::AddFlash_CardPosterXY( CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1], ImpactFlashPos, m_SetupParms.fMuzzleScale_Impact, m_SetupParms.fMuzzleAlpha_Impact );

	// Create a flash light at the impact point...
	ImpactFlashPos.Mul( m_ImpactNormal, 2.0f ).Add( *pPos_WS );
	CMuzzleFlash::MuzzleLight( &ImpactFlashPos, 5.0f );
}


void CEProj_Saw::_KillCarvingSound( CEProj *pProj ) {
	if( m_pAudioEmitterCarving ) {
		m_pAudioEmitterCarving->Destroy();
		m_pAudioEmitterCarving = NULL;
	}
}


void CEProj_Saw::_WindDownCarvingSound( CEProj *pProj ) {
	_KillCarvingSound( pProj );

	pProj->PlaySound( m_SetupParms.pSoundGroupCutStop );
}


void CEProj_Saw::_StartCarvingSound( CEProj *pProj ) {
	_KillCarvingSound( pProj );

	m_pAudioEmitterCarving = pProj->AllocAndPlaySound( m_SetupParms.pSoundGroupCutLoop );
}


void CEProj_Saw::_UpdateCarvingSound( CEProj *pProj ) {
	if( m_pAudioEmitterCarving ) {
		m_pAudioEmitterCarving->SetPosition( &pProj->MtxToWorld()->m_vPos );
	}
}


// m_pHitEntity, m_ImpactNormal, m_ImpactPoint, and m_CollImpact must be set before calling.
// Returns FALSE if the matrix could not be computed.
BOOL CEProj_Saw::_UpdateCarveModeMtxAndRelocate( CEProj *pProj ) {
	if( !_ComputeCarveModeMtxAndRelocate( pProj ) ) {
		return FALSE;
	}

	cchar *pszName, *pszCurrentAttachedBoneName;
	BOOL bAttach = TRUE;

	// Update mesh xfm...
	_UpdateCarveModeXfm( pProj );

	pszName = NULL;

	if( m_pHitEntity ) {
		// Attach saw to hit entity...
		CFWorldMesh *pWorldMesh = (CFWorldMesh *)m_CollImpact.pTag;

		if( pWorldMesh->m_pMesh->pBoneArray ) {
			pszName = &pWorldMesh->m_pMesh->pBoneArray[m_CollImpact.nBoneIndex].szName[0];
		}

		if( m_pHitEntity == pProj->GetParent() ) {
			pszCurrentAttachedBoneName = pProj->GetAttachBoneName();

			if( pszName && pszCurrentAttachedBoneName ) {
				bAttach = fclib_stricmp( pszName, pszCurrentAttachedBoneName );
			} else {
				bAttach = (u32)pszName | (u32)pszCurrentAttachedBoneName;
			}
		}

		if( bAttach ) {
			pProj->Attach_ToParent_WS( m_pHitEntity, pszName );
		}
	} else {
		// Attach saw to hit terrain...
		pProj->DetachFromParent();
	}

	m_pCarveEntity = m_pHitEntity;

	// Update sound position...
	_UpdateCarvingSound( pProj );

	return TRUE;
}


// m_ImpactNormal and m_ImpactPoint must be set before calling.
// Returns FALSE if the matrix could not be computed.
BOOL CEProj_Saw::_ComputeCarveModeMtxAndRelocate( CEProj *pProj ) {
	CFMtx43A NewMtx;
	f32 fMag2;

	NewMtx.m_vUp.Cross( pProj->m_LinUnitDir_WS, m_ImpactNormal );

	fMag2 = NewMtx.m_vUp.MagSq();
	if( fMag2 < 0.0001f ) {
		return FALSE;
	}

	NewMtx.m_vUp.Mul( fmath_InvSqrt(fMag2) );

	NewMtx.m_vFront.Cross( m_ImpactNormal, NewMtx.m_vUp );
	NewMtx.m_vRight = m_ImpactNormal;
	NewMtx.m_vPos = m_ImpactPoint;

	pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx );

	return TRUE;
}


void CEProj_Saw::_UpdateCarveModeXfm( CEProj *pProj ) {
	if( pProj->m_pWorldMesh == NULL ) {
		return;
	}

	CFQuatA CurrentQuat, DesiredQuat, SpinQuat, FinalQuat;

	// Build current quaternion...
	CurrentQuat = m_CarveMeshQuat;

	// Build desired quaternion...
	DesiredQuat.BuildQuat( *pProj->MtxToWorld() );

	// Compute new quaternion by taking a step from the current to the desired...
	m_CarveMeshQuat.ReceiveSlerpedStep( _CARVEMODE_QUAT_ROT_SPEED * FLoop_fPreviousLoopSecs, CurrentQuat, DesiredQuat );

	// Compute spin quaterion...
	SpinQuat.BuildQuatRotY( m_fSawSpinAngle );

	// Compute final quaterion...
	FinalQuat.Mul( m_CarveMeshQuat, SpinQuat );

	// Update Xfm...
	if( pProj->m_fScaleToWorld == 1.0f ) {
		pProj->m_pWorldMesh->m_Xfm.BuildFromQuat( FinalQuat, pProj->MtxToWorld()->m_vPos );
	} else {
		pProj->m_pWorldMesh->m_Xfm.BuildFromQuat( FinalQuat, pProj->MtxToWorld()->m_vPos, pProj->m_fScaleToWorld );
	}

	// Update tracker...
	pProj->m_pWorldMesh->UpdateTracker();
}


// Returns TRUE if the projectile has been destroyed.
BOOL CEProj_Saw::_Work_Carving( CEProj *pProj ) {
	BOOL bCauseDamage = FALSE;
	BOOL bSpawnSparks = FALSE;

	// Update damage timer...
	m_fCarveDamageCountdownTimer -= FLoop_fPreviousLoopSecs;
	if( m_fCarveDamageCountdownTimer <= 0.0f ) {
		m_fCarveDamageCountdownTimer = m_pCarveDamageProfile->m_fDamagePeriod;
		bCauseDamage = TRUE;
	}

	// Slow down our speed...
	if( pProj->m_fLinSpeed_WS > 0.0f ) {
		pProj->m_fLinSpeed_WS -= m_fCarveDeceleration * FLoop_fPreviousLoopSecs;
		if( pProj->m_fLinSpeed_WS < 0.0f ) {
			pProj->m_fLinSpeed_WS = 0.0f;
			pProj->SmokeTrailOff();
			_WindDownCarvingSound( pProj );
		}
	} else {
		pProj->m_fRotSpeed_WS -= _CARVEMODE_SPIN_DECELERATION * FLoop_fPreviousLoopSecs;
		if( pProj->m_fRotSpeed_WS <= 0.0f ) {
			_StartStickMode( pProj );
			return FALSE;
		}
	}

	m_fCarveSparkTimer -= FLoop_fPreviousLoopSecs;
	if( m_fCarveSparkTimer <= 0.0f ) {
		m_fCarveSparkTimer = _SPAWN_CARVING_SPARKS_SECS;
		bSpawnSparks = TRUE;
	}

	// Update spin angle...
	m_fSawSpinAngle -= pProj->m_fRotSpeed_WS * FLoop_fPreviousLoopSecs;
	while( m_fSawSpinAngle < 0.0f ) {
		m_fSawSpinAngle += FMATH_2PI;
	}

	// Update our velocity unit vector to account for parent's motion...
	pProj->m_LinUnitDir_WS = pProj->MtxToWorld()->m_vFront;

	if( m_fCarveSurfaceJumpAngle == 0.0f ) {
		// Take a step and see if we're still stuck to a surface...
		if( _FindCarveModeSurfaceColl( pProj, &pProj->MtxToWorld()->m_vRight, bCauseDamage, bSpawnSparks ) ) {
			// We're still stuck to a surface...
			return FALSE;
		}

		if( !pProj->IsInWorld() ) {
			// We have been destroyed...
			return TRUE;
		}

		// We couldn't find a nearby surface. Try making our step -45 degrees next frame...
		m_fCarveSurfaceJumpAngle = FMATH_DEG2RAD( -45.0f );

		return FALSE;
	}

	CFQuatA Quat;
	CFVec3A NewRight;

	// Rotate velocity unit vector and surface normal and see if we find a surface...
	Quat.BuildQuat( pProj->MtxToWorld()->m_vUp, m_fCarveSurfaceJumpAngle );
	Quat.MulPoint( pProj->m_LinUnitDir_WS );
	Quat.MulPoint( NewRight, pProj->MtxToWorld()->m_vRight );

	if( _FindCarveModeSurfaceColl( pProj, &NewRight, bCauseDamage, bSpawnSparks ) ) {
		// We found a surface to stick to...
		return FALSE;
	}

	if( !pProj->IsInWorld() ) {
		// We have been destroyed...
		return TRUE;
	}

	// We couldn't find a nearby surface...

	if( m_fCarveSurfaceJumpAngle == FMATH_DEG2RAD( -45.0f ) ) {
		// Try making our step +45 degrees next frame...
		m_fCarveSurfaceJumpAngle = FMATH_DEG2RAD( 45.0f );
		return FALSE;
	}

	// We couldn't find a surface to stick to. Stick projectile...

	_WindDownCarvingSound( pProj );
	_StartStickMode( pProj );

	return FALSE;
}


// Returns TRUE if we found a new point to stick to.
// Returns FALSE if we either didn't find a new point to stick to, or if we were destroyed.
BOOL CEProj_Saw::_FindCarveModeSurfaceColl( CEProj *pProj, const CFVec3A *pRightVec, BOOL bCauseDamage, BOOL bSpawnSparks ) {
	CFVec3A NewPos, StartCollPoint_WS, EndCollPoint_WS, TempVec, TempVec2;

	// Take a step along our Z axis...
	NewPos.Mul( pProj->m_LinUnitDir_WS, pProj->m_fLinSpeed_WS * FLoop_fPreviousLoopSecs ).Add( pProj->MtxToWorld()->m_vPos );

	StartCollPoint_WS.Mul( *pRightVec, CFVec3A::m_HalfVec ).Add( NewPos );
	EndCollPoint_WS.Mul( *pRightVec, CFVec3A::m_NegHalfVec ).Add( NewPos );

	fcoll_Clear();

	m_CollInfo.pTag = NULL;
	m_CollInfo.Ray.Init( &StartCollPoint_WS, &EndCollPoint_WS );
	m_CollRayInfo.StartPoint_WS = StartCollPoint_WS;
	m_CollRayInfo.EndPoint_WS = EndCollPoint_WS;

	m_CollRayInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	m_CollRayInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;

	if( m_pCarveEntity ) {
		// We're carving an entity...
		fworld_CollideWithTrackers( &m_CollRayInfo );
	} else {
		// We're carving the world...
		fworld_CollideWithWorldTris( &m_CollInfo );
	}

	if( FColl_nImpactCount ) {
		fang_MemCopy( &m_CollImpact, FColl_aImpactBuf, sizeof(FCollImpact_t) );
		// Found collision...

		m_pHitEntity = NULL;
		m_ImpactNormal.Set( m_CollImpact.UnitFaceNormal );
		m_ImpactPoint.Set( m_CollImpact.ImpactPoint );

		m_pHitEntity = CGColl::ExtractEntity( &m_CollImpact );

		if( bCauseDamage ) {
			// Deal damage to the entity we're stuck to...
			if( m_pHitEntity ) {
				// Get an empty damage form...
				CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

				if( pDamageForm ) {
					// Fill out the form...

					TempVec.ReceiveNegative( m_CollImpact.UnitFaceNormal );

					pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
					pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
					pDamageForm->m_pDamageProfile = m_pCarveDamageProfile;
					pDamageForm->m_Damager = *pProj->GetDamager();
					pDamageForm->m_pDamageeEntity = m_pHitEntity;
					pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *)m_CollImpact.pTag, &m_CollImpact, &TempVec );

					CDamage::SubmitDamageForm( pDamageForm );

					if( !pProj->IsInWorld() ) {
						// We were destroyed as a result of destroying the entity we were attached to...
						return FALSE;
					}
				}
			}

			// Deal damage to nearby entities...
			if( _DamageNearbyEntities( pProj, m_pCarveDamageProfile ) ) {
				// The saw has been destroyed...
				return FALSE;
			}
		}

		u8 nProjReact = CGColl::ComputeProjectileReaction( &m_CollImpact );

		if( nProjReact == CEntity::PROJECTILE_REACTION_DEATH_TO_ALL ) {
			// We touched a surface that kills us...
			pProj->Detonate();
			return FALSE;
		}

		if( (nProjReact == CEntity::PROJECTILE_REACTION_RICOCHET) || (nProjReact == CEntity::PROJECTILE_REACTION_SHIELD) ) {
			// We can't carve onto this type of surface...
			return FALSE;
		}

		if( _UpdateCarveModeMtxAndRelocate( pProj ) ) {
			const CGCollMaterial *pMaterial = CGColl::GetMaterial( &m_CollImpact );
			m_fCarveSurfaceJumpAngle = 0.0f;

			// Spawn sparks if it's time...
			if( bSpawnSparks ) {
				TempVec.ReceiveNegative( pProj->m_LinUnitDir_WS );

#if 0
				if( fmath_RandomChance(0.8f) ) {
					pMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST, &pProj->MtxToWorld()->m_vPos, &TempVec, fmath_RandomFloatRange( 0.5f, 1.0f ) );
				} else {
					pMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_SHOWER, &pProj->MtxToWorld()->m_vPos, &TempVec, 1.0f );
				}
#else
				pMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_SHOWER, &pProj->MtxToWorld()->m_vPos, &TempVec, 1.0f );
#endif
			}

			if( pMaterial->IsImpactFlashEnabled() ) {
				if( pProj->m_fLinSpeed_WS > 0.0f ) {
					// Make a flash at the impact point...
					TempVec2.Mul( pProj->MtxToWorld()->m_vFront, 0.6f );
					TempVec.Mul( pProj->MtxToWorld()->m_vRight, 0.1f ).Add( TempVec2 ).Add( pProj->MtxToWorld()->m_vPos );

					CMuzzleFlash::AddFlash_CardPosterXY(
						CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
						TempVec,
						fmath_RandomFloatRange( 0.2f, 0.6f ),
						fmath_RandomFloatRange( 0.05f, 1.0f )
					);
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}


BOOL CEProj_Saw::_CollRayWithTrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection ) {
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

#if 0
	u32 i;

	for( i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}
#endif

	m_CollInfo.pTag = pTracker;
	if( pWorldMesh->CollideWithMeshTris( &m_CollInfo ) ) {
		return FALSE;
	} else {
		return TRUE;
	}
}


// m_pHitEntity must have previously been set up, and is the
// entity that has already been damaged (will be skipped by this function).
// Returns TRUE if the saw was destroyed.
BOOL CEProj_Saw::_DamageNearbyEntities( CEProj *pProj, const CDamageProfile *pDamageProfile ) {
	CFSphere DamageSphere;

	DamageSphere.m_Pos = pProj->MtxToWorld()->m_vPos.v3;
	DamageSphere.m_fRadius = m_SetupParms.fDamageSphereRadius;

	m_pCollProj = pProj;
	m_pCollDamageProfile = pDamageProfile;
	m_CollInfoNearbySphereTest.pSphere_WS = &DamageSphere;
	m_nDamageFormCount = 0;

	fworld_FindTrackersIntersectingSphere( &DamageSphere, FWORLD_TRACKERTYPE_MESH, _DamageNearbyTrackerCallback );

	if( m_nDamageFormCount ) {
		u32 i;

		for( i=0; i<m_nDamageFormCount; ++i ) {
			CDamage::SubmitDamageForm( m_apDamageFormArray[i] );
		}
	}

	return !pProj->IsInWorld();
}


BOOL CEProj_Saw::_DamageNearbyTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		return TRUE;
	}

	CEntity *pEntity = (CEntity *)pTracker->m_pUser;

	if( pEntity == m_pHitEntity ) {
		// Skip this entity since we've already dealt damage to it...
		return TRUE;
	}

	// Determine if damage sphere hits any triangles on this entity...

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	fcoll_Clear();
	m_CollInfoNearbySphereTest.pTag = pTracker;
	pWorldMesh->CollideWithMeshTris( &m_CollInfoNearbySphereTest );

	if( FColl_nImpactCount == 0 ) {
		// Didn't collide with any entities...
		return TRUE;
	}

	CDamageForm *pDamageForm;
	CFVec3A AttackUnitDir_WS;

	pDamageForm = CDamage::GetEmptyDamageFormFromPool();
	if( pDamageForm ) {
		// Fill out the form...

		AttackUnitDir_WS.ReceiveNegative( FColl_aImpactBuf->UnitFaceNormal );

		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_pDamageProfile = m_pCollDamageProfile;
		pDamageForm->m_Damager = *m_pCollProj->GetDamager();
		pDamageForm->m_pDamageeEntity = pEntity;
		pDamageForm->InitTriDataFromCollImpact( pWorldMesh, FColl_aImpactBuf, &AttackUnitDir_WS );

		FASSERT( m_nDamageFormCount < DAMAGE_FORM_COUNT );
		m_apDamageFormArray[ m_nDamageFormCount ] = pDamageForm;

		++m_nDamageFormCount;
	}

	if( m_nDamageFormCount >= DAMAGE_FORM_COUNT ) {
		return FALSE;
	}

	return TRUE;
}


void CEProj_Saw::_SetSmokeTrailAttributes( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1steam01" );
	if ( pTexDef == NULL ) {
		DEVPRINTF( "CEProj_Saw::SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	// Impact smoke attributes:
	m_ImpactSmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_ImpactSmokeTrailAttrib.pTexDef = pTexDef;

	m_ImpactSmokeTrailAttrib.fScaleMin_WS = 0.4f;
	m_ImpactSmokeTrailAttrib.fScaleMax_WS = 0.6f;
	m_ImpactSmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_ImpactSmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_ImpactSmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_ImpactSmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_ImpactSmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_ImpactSmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_ImpactSmokeTrailAttrib.fDistBetweenPuffs_WS = 0.2f;
	m_ImpactSmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.05f;

	m_ImpactSmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_ImpactSmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_ImpactSmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_ImpactSmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_ImpactSmokeTrailAttrib.fUnitOpaqueMin_WS = 0.45f;
	m_ImpactSmokeTrailAttrib.fUnitOpaqueMax_WS = 0.55f;
	m_ImpactSmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.6f;
	m_ImpactSmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.8f;
	m_ImpactSmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_ImpactSmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_ImpactSmokeTrailAttrib.StartColorRGB.Set( 0.8f, 0.8f, 0.8f );
	m_ImpactSmokeTrailAttrib.EndColorRGB.White();
	m_ImpactSmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_ImpactSmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_ImpactSmokeTrailAttrib.fEndColorUnitIntensityMin = 0.2f;
	m_ImpactSmokeTrailAttrib.fEndColorUnitIntensityMax = 0.6f;

	m_ImpactSmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_ImpactSmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_ImpactSmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_ImpactSmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	// Carve smoke attributes:
	m_CarveSmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_CarveSmokeTrailAttrib.pTexDef = pTexDef;

	m_CarveSmokeTrailAttrib.fScaleMin_WS = 0.3f;
	m_CarveSmokeTrailAttrib.fScaleMax_WS = 0.35f;
	m_CarveSmokeTrailAttrib.fScaleSpeedMin_WS = 1.0f;
	m_CarveSmokeTrailAttrib.fScaleSpeedMax_WS = 1.3f;
	m_CarveSmokeTrailAttrib.fScaleAccelMin_WS = -0.7f;
	m_CarveSmokeTrailAttrib.fScaleAccelMax_WS = -0.95f;

	m_CarveSmokeTrailAttrib.fXRandSpread_WS = 0.1f;
	m_CarveSmokeTrailAttrib.fYRandSpread_WS = 0.1f;
	m_CarveSmokeTrailAttrib.fDistBetweenPuffs_WS = 0.1f;
	m_CarveSmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.025f;

	m_CarveSmokeTrailAttrib.fYSpeedMin_WS = 1.0f;
	m_CarveSmokeTrailAttrib.fYSpeedMax_WS = 1.5f;
	m_CarveSmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_CarveSmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_CarveSmokeTrailAttrib.fUnitOpaqueMin_WS = 0.7f;
	m_CarveSmokeTrailAttrib.fUnitOpaqueMax_WS = 1.0f;
	m_CarveSmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.8f;
	m_CarveSmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -1.2f;
	m_CarveSmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_CarveSmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_CarveSmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_CarveSmokeTrailAttrib.EndColorRGB.Set( 0.85f, 0.85f, 0.85f );
	m_CarveSmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_CarveSmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_CarveSmokeTrailAttrib.fEndColorUnitIntensityMin = 0.1f;
	m_CarveSmokeTrailAttrib.fEndColorUnitIntensityMax = 0.5f;

	m_CarveSmokeTrailAttrib.fColorUnitSliderSpeedMin = 2.5f;
	m_CarveSmokeTrailAttrib.fColorUnitSliderSpeedMax = 3.5f;
	m_CarveSmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_CarveSmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
}
