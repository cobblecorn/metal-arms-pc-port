//////////////////////////////////////////////////////////////////////////////////////
// eproj_cleaner.cpp - Cleaner canister projectiles.
//
// Author: Chris MacDonald
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
// 09/09/02 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eproj_cleaner.h"
#include "floop.h"
#include "fworld_coll.h"
#include "explosion.h"
#include "meshtypes.h"
#include "entity.h"
#include "weapon.h"
#include "weapon_cleaner.h"
#include "AI/AIEnviro.h"
#include "fcamera.h"
#include "gamecam.h"
#include "AI/AIGameUtils.h"
#include "fvtxpool.h"
#include "frenderer.h"
#include "bot.h"
#include "game.h"
#include "fsound.h"



#define _RETICLE_DEFAULT_HEIGHT		6.0f

CEProj_Cleaner *CEProj_Cleaner::m_apTargetingCleaner[MAX_PLAYERS];
FTextAreaHandle_t CEProj_Cleaner::m_hTextBoxSelectTargets;
FTextAreaHandle_t CEProj_Cleaner::m_hTextBoxNumTargets;
CFVec3A CEProj_Cleaner::m_avReticleVertex[ CLEANER_RETICLE_NUM_VERTICES ];

BOOL CEProj_Cleaner::InitSystem( void )
{
	FTextArea_t TextArea;

	ftext_SetToDefaults( &TextArea );
	TextArea.bVisible = FALSE;
	TextArea.fUpperLeftX = 0.0f;
	TextArea.fLowerRightX = 0.3f;
	TextArea.fUpperLeftY = 0.54f;
	TextArea.fLowerRightY = 0.58f;
	TextArea.fNumberOfLines = 2;
	TextArea.fBorderThicknessX = 0.0f;
	TextArea.fBorderThicknessY = 0.0f;
	TextArea.oColorForeground.SetColor( 1.0f, 1.0f, 0.0f );
	TextArea.oColorForeground.SetAlpha( 0.6f );
	TextArea.oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.5f );
	TextArea.oColorBorder.Set( 0.1f, 0.1f, 1.0f, 1.0f );
	TextArea.ohFont = '9';
	TextArea.bNoScale = TRUE;
	TextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	// Set up select targets text box...
	m_hTextBoxSelectTargets = ftext_Create( &TextArea );

	ftext_SetToDefaults( &TextArea );
	TextArea.bVisible = FALSE;
	TextArea.fUpperLeftX = 0.0f;
	TextArea.fLowerRightX = 0.3f;
	TextArea.fUpperLeftY = 0.59f;
	TextArea.fLowerRightY = 0.64f;
	TextArea.fNumberOfLines = 2;
	TextArea.fBorderThicknessX = 0.0f;
	TextArea.fBorderThicknessY = 0.0f;
	TextArea.oColorForeground.SetColor( 1.0f, 1.0f, 0.0f );
	TextArea.oColorForeground.SetAlpha( 0.6f );
	TextArea.oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.5f );
	TextArea.oColorBorder.Set( 0.1f, 0.1f, 1.0f, 1.0f );
	TextArea.ohFont = '9';
	TextArea.bNoScale = TRUE;
	TextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;

	// Set up num targets text box...
	m_hTextBoxNumTargets = ftext_Create( &TextArea );


	for (s32 i = 0; i < MAX_PLAYERS; i++)
		m_apTargetingCleaner[i] = NULL;

	m_avReticleVertex[  0 ].Set( -0.6f,  0.8f, 0.0f );
	m_avReticleVertex[  1 ].Set( -0.6f,  1.0f, 0.0f );
	m_avReticleVertex[  2 ].Set(  0.6f,  0.8f, 0.0f );
	m_avReticleVertex[  3 ].Set(  0.6f,  0.8f, 0.0f );
	m_avReticleVertex[  4 ].Set( -0.6f,  1.0f, 0.0f );
	m_avReticleVertex[  5 ].Set(  0.6f,  1.0f, 0.0f );
					    
	m_avReticleVertex[  6 ].Set(  0.8f,  0.6f, 0.0f );
	m_avReticleVertex[  7 ].Set(  1.0f,  0.6f, 0.0f );
	m_avReticleVertex[  8 ].Set(  0.8f, -0.6f, 0.0f );
	m_avReticleVertex[  9 ].Set(  0.8f, -0.6f, 0.0f );
	m_avReticleVertex[ 10 ].Set(  1.0f,  0.6f, 0.0f );
	m_avReticleVertex[ 11 ].Set(  1.0f, -0.6f, 0.0f );
					   
	m_avReticleVertex[ 12 ].Set(  0.6f, -0.8f, 0.0f );
	m_avReticleVertex[ 13 ].Set(  0.6f, -1.0f, 0.0f );
	m_avReticleVertex[ 14 ].Set( -0.6f, -0.8f, 0.0f );
	m_avReticleVertex[ 15 ].Set( -0.6f, -0.8f, 0.0f );
	m_avReticleVertex[ 16 ].Set(  0.6f, -1.0f, 0.0f );
	m_avReticleVertex[ 17 ].Set( -0.6f, -1.0f, 0.0f );
					   
	m_avReticleVertex[ 18 ].Set( -0.8f, -0.6f, 0.0f );
	m_avReticleVertex[ 19 ].Set( -1.0f, -0.6f, 0.0f );
	m_avReticleVertex[ 20 ].Set( -0.8f,  0.6f, 0.0f );
	m_avReticleVertex[ 21 ].Set( -0.8f,  0.6f, 0.0f );
	m_avReticleVertex[ 22 ].Set( -1.0f, -0.6f, 0.0f );
	m_avReticleVertex[ 23 ].Set( -1.0f,  0.6f, 0.0f );

	return TRUE;
}

BOOL CEProj_Cleaner::Create( CEProj *pProj ) {
	return TRUE;
}

void CEProj_Cleaner::Init( CEProj *pProj ) {
	f32 fRadius, fMass;

	fRadius = pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius;
	fMass = 50.0f;

	m_Motion.Simple_InitBody( pProj->MtxToWorld(), CFMotion::ComputeInvUniformInertiaTensorMag( fRadius, fMass ), fMass, -32.0f );
	m_fSecsUntilNextSound = 0.0f;
	m_fTextBlinkTimer = 0.0f;
	m_fAngle = 0.0f;
	m_uAISoundHandle = 0;

	InitTargetList();

	m_bLaunched = FALSE;

	for( s32 nRocket = 0; nRocket < CLEANER_CANISTER_MAX_ROCKETS; nRocket++ )
	{
		for( s32 nTrail = 0; nTrail < CLEANER_CANISTER_MAX_ROCKETS; nTrail++ )
		{
			m_aReticleHistory[ nRocket ][ nTrail ].vPos_WS.Zero();
			m_aReticleHistory[ nRocket ][ nTrail ].fAngle = 0.0f;
			m_aReticleHistory[ nRocket ][ nTrail ].fDelta = 0.0f;
			m_aReticleHistory[ nRocket ][ nTrail ].bDraw = FALSE;
		}

		m_nReticleHistoryIndex = 0;
	}

	m_SetupParms.fRocketMaxDist = 500.0f;
	m_SetupParms.fRocketMaxSecs = 5.0f;
	m_SetupParms.fRocketMinSpeed = 30.0f;
	m_SetupParms.fRocketMaxSpeed = 70.0f;
	m_SetupParms.fRocketScale = 1.5f;
	m_SetupParms.fRocketSwarmAmp = 0.0f;
	m_SetupParms.hRocketExplosionGroup = FEXPLOSION_INVALID_HANDLE;
	m_SetupParms.pSoundGroupWhir = NULL;

	m_pWhirSoundEmitter = NULL;
}

BOOL CEProj_Cleaner::InitLevel( void )
{
	for (s32 i = 0; i < MAX_PLAYERS; i++)
		CEProj_Cleaner::m_apTargetingCleaner[i] = NULL;
	return TRUE;
}

void CEProj_Cleaner::UnInitLevel( void )
{
}

void CEProj_Cleaner::ClearTargetingCleaners( void )
{
	for (s32 i = 0; i < MAX_PLAYERS; i++)
		m_apTargetingCleaner[i] = NULL;
}

void CEProj_Cleaner::ClearTargetingCleaner(CWeaponCleaner* pCleanerWeapon)
{
	FASSERT( pCleanerWeapon && pCleanerWeapon->GetOwner() && (pCleanerWeapon->GetOwner()->m_nPossessionPlayerIndex >= 0) );
	m_apTargetingCleaner[ pCleanerWeapon->GetOwner()->m_nPossessionPlayerIndex ] = NULL;
}

void CEProj_Cleaner::SetCleanerParams( CEProj *pProj, const CEProj_Cleaner_Params_t *pParms )
{
	fang_MemCopy( &m_SetupParms, pParms, sizeof(CEProj_Cleaner_Params_t) );
}

void CEProj_Cleaner::Launched( CEProj *pProj ) {
	m_Motion.m_AngularVelocity_WS.Mul( *pProj->GetRotUnitAxis_WS(), pProj->GetRotSpeed() );
	m_Motion.m_LinearVelocity_WS.Mul( *pProj->GetLinUnitDir_WS(), pProj->GetLinSpeed() );

	m_Motion.Simple_UpdateOrientation( pProj->MtxToWorld() );

	m_fSecsUntilNextSound = 0.0f;

	// set default canister target location
	m_vTarget_WS.Set( m_Motion.m_Mtx.m_vFront );
	m_vTarget_WS.Mul( CLEANER_CANISTER_MAX_RANGE );
	m_vTarget_WS.Add( m_Motion.m_Mtx.m_vPos );

	m_pWhirSoundEmitter = CFSoundGroup::AllocAndPlaySound( m_SetupParms.pSoundGroupWhir, FALSE, &m_Motion.m_Mtx.m_vPos );
	
	m_bLaunched = TRUE;
}


BOOL CEProj_Cleaner::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact ) {
	CEProj *pRocket;
	CWeaponCleaner *pCleaner;

	pCleaner = (CWeaponCleaner*) pProj->GetDamagerWeapon();
	CFVec3A vRocketDir;
	CFVec3A vTargetPos;
	s32 nRocket = 0;
	CEntity *pEntity;
	s32 nIndex = 0;
	BOOL bTargetFound;

	if ( m_pWhirSoundEmitter )
	{
		m_pWhirSoundEmitter->Destroy();
		m_pWhirSoundEmitter = NULL;
	}

	// loop through and assign all rockets to a target
	while( nRocket < CLEANER_CANISTER_MAX_ROCKETS )
	{
		pRocket = CEProjPool::GetProjectileFromFreePool( CWeapon::m_ahProjPool[CWeapon::PROJ_POOL_TYPE_SWARMER_ROCKET] );
		if( pRocket == NULL )
		{
			// no projectiles left in pool
			break;
		}

		pRocket->Init();

		bTargetFound = FALSE;

		if( NumTargetsInList() )
		{
			// get target position
			pEntity = GetTargetByIndex( nIndex );
			if( pEntity )
			{
				if( pEntity->IsInWorld() && TargetListGuidMatches( pEntity ) )
				{
					// target the entity.
					// rocket will be a guided projectile in this case.
					vTargetPos.Set( pEntity->MtxToWorld()->m_vPos );
					pRocket->SetTargetedEntity( pEntity );
					bTargetFound = TRUE;
				}
			}
		}

		if( !bTargetFound )
		{
			// Set a default target position just below the rocket,
			// just to get it moving in the right direction.  Actual 
			// postion unimportant because rocket will be unguided
			// in this case.
			vTargetPos.Set( m_Motion.m_Mtx.m_vPos );
			vTargetPos.Sub( CFVec3A::m_UnitAxisY );
		}

		CFMtx43A ProjMtx;
		CFVec3A ProjUnitDir_WS;
		f32 fMag2;

		// set up the rocket
		pRocket->SetDamager( pProj->GetDamagerWeapon(), pProj->GetDamagerBot() );
		pRocket->SetDamageProfile( pProj->GetDamageProfile() );
		pRocket->SetMaxDistCanTravel( m_SetupParms.fRocketMaxDist );
		pRocket->SetLinSpeed( fmath_RandomFloatRange( m_SetupParms.fRocketMinSpeed, m_SetupParms.fRocketMaxSpeed ) );
		pRocket->SetExplosionGroup( m_SetupParms.hRocketExplosionGroup );

		CEProj_Swarmer_Params_t SwarmerParams;
		SwarmerParams.fSwarmAmplitude = m_SetupParms.fRocketSwarmAmp;
		SwarmerParams.fSwarmingBlendInInvDist = (1.0f / 20.0f);
		SwarmerParams.fSwarmingSpiralSpeed = (FMATH_PI / 100.0f);
		SwarmerParams.pMervStaticParams = NULL;
		pRocket->SetSwarmerParams( &SwarmerParams );

		// point rocket in direction of target
		vRocketDir.Set( vTargetPos );
		vRocketDir.Sub( m_Motion.m_Mtx.m_vPos );

		fMag2 = vRocketDir.MagSq();
		if( fMag2 >= 0.0001f )
		{
			vRocketDir.Mul( fmath_InvSqrt( fMag2 ) );
		}
		else
		{
			vRocketDir = CFVec3A::m_UnitAxisX;
		}

		fmath_ScatterUnitVec( &vRocketDir, 0.05f );
		pRocket->SetLinUnitDir_WS( &vRocketDir );

		// orient rocket to direction of travel
		ProjMtx.UnitMtxFromUnitVec( &vRocketDir );
		ProjMtx.m_vPos.Set( 0.5f, 0.5f, 0.5f );
		fmath_ScatterUnitVec( &ProjMtx.m_vPos, 1.0f );
		ProjMtx.m_vPos.Add( m_Motion.m_Mtx.m_vPos );
		pRocket->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, m_SetupParms.fRocketScale );

		pRocket->SetMaxLifeSecs( m_SetupParms.fRocketMaxSecs );
		pRocket->SmokeTrailOn( pCleaner->GetRocketSmokeTrailAttrib() );
		pRocket->SetSmokePos_MS( &CFVec3A( 0.0f, 0.0f, -0.5f ) );
		pRocket->SetExplosionGroup( m_SetupParms.hRocketExplosionGroup );

		pRocket->Launch();

		++nRocket;

		// if there are more rockets than targets,
		// retraverse target list until all rockets are assigned
		++nIndex;
		if( nIndex >= NumTargetsInList() )
		{
			nIndex = 0;
		}
	}

	// NULL reference current canister (used by static functions)
	FASSERT( pProj->GetDamagerBot() && (pProj->GetDamagerBot()->m_nPossessionPlayerIndex >= 0) );
	m_apTargetingCleaner[pProj->GetDamagerBot()->m_nPossessionPlayerIndex] = NULL;

	return TRUE;
}


#define _CLEANER_CANISTER_MAX_ANGULAR_VEL	( FMATH_DEG2RAD( 360.0f ) )	// radians per second
#define _CLEANER_CANISTER_MAX_LINEAR_VEL	( 200.0f )					// feet per second
#define _CLEANER_CANISTER_MOTOR_START_TIME	( 0.2f )					// seconds

BOOL CEProj_Cleaner::Work( CEProj *pProj ) {
	CFVec3A PrevP1, PrevP2, CurP1, CurP2, PrevToCur1, PrevToCur2;
	CFVec3A UnitFaceNormal, ImpactPoint, PushVec;
	FCollImpact_t CollImpact1, CollImpact2;
	BOOL bColl1, bColl2;
	f32 fPushDist1, fPushDist2, fImpulse;
	f32 fDistTraveledThisFrame;
	CFWorldTracker *pWorldTracker;
	CEntity *pEntity;

	m_fSecsUntilNextSound -= FLoop_fPreviousLoopSecs;

	CFVec3A vForce;
	CFVec3A vTargetUnit;
	CFVec3A vRotAxis;
	CFVec3A vCurrentVelUnit;
	CFQuatA RotQuat;

	// wait a short while after launch before activating canister motor
	if( pProj->m_fLifeSecs >= _CLEANER_CANISTER_MOTOR_START_TIME )
	{
		CFVec3A vAverage, vDeltaToTarget;
		s32 nTargets = 0;

		vAverage.Zero();

		if( NumTargetsInList() )
		{
			CEntity *pEntity;
			BOOL bResult;

			// find average position of bot targets
			for( s32 nIndex = 0; nIndex < NumTargetsInList(); nIndex++ )
			{
				pEntity = GetTargetByIndex( nIndex );

				if( pEntity == NULL )
				{
					continue;
				}
				else if( !pEntity->IsInWorld() || !TargetListGuidMatches( pEntity ) )
				{
					// bot dead or not the same bot we targeted
					bResult = RemoveFromTargetList( pEntity );
					FASSERT( bResult );
					continue;
				}

				// accumulate the position vector of this entity
				vAverage.Add( pEntity->MtxToWorld()->m_vPos );
				++nTargets;
			}
		}

		if( nTargets )
		{
			// divide to get average of entity positions
			vAverage.Mul( fmath_Inv( (f32) NumTargetsInList() ) );

			// set canister target point to average point plus detonation height
			m_vTarget_WS.Set( vAverage );
			m_vTarget_WS.y += CLEANER_CANISTER_MAX_DETONATION_HEIGHT;

			// detonate canister if within range of target
			vDeltaToTarget.Set( vAverage );
			vDeltaToTarget.Sub( m_Motion.m_Mtx.m_vPos );
			if( vDeltaToTarget.MagSqXZ() < CLEANER_CANISTER_DETONATION_RADIUS_SQUARED )
			{
				pProj->Detonate( TRUE, CEProj::EVENT_LIFE_TIME_OVER, NULL );
				return TRUE;
			}
		}
		else
		{
			// no targets in list

			// get pointer to canister's weapon
			CWeaponCleaner *pCleaner = (CWeaponCleaner*) pProj->GetDamagerWeapon();
			FASSERT( pCleaner );

			if( pCleaner )
			{
				// get pointer to weapon's owner
				CBot* pCleanerOwner = pCleaner->GetOwner();
				FASSERT( pCleanerOwner );

				if( pCleanerOwner )
				{
					// compute a default target postion in world space
					// based on owner's mount direction and position
					m_vTarget_WS.Set( pCleanerOwner->m_MountUnitFrontXZ_WS );
					m_vTarget_WS.Mul( 50.0f );
					m_vTarget_WS.Add( pCleanerOwner->MtxToWorld()->m_vPos );
					m_vTarget_WS.y += CLEANER_CANISTER_MAX_DETONATION_HEIGHT;

					// explode soon if no targets
					if( pProj->m_fLifeSecs < pProj->m_fMaxLifeSecs - 1.0f )
					{
						pProj->m_fLifeSecs = pProj->m_fMaxLifeSecs - 1.0f;
					}
				}
			}
		}

		if( m_Motion.m_LinearVelocity_WS == 0.0f )
		{
			vCurrentVelUnit.Set( CFVec3A::m_UnitAxisY );
		}
		else
		{
			vCurrentVelUnit.ReceiveUnit( m_Motion.m_LinearVelocity_WS );
		}

		// construct unit vector from canister to target
		vTargetUnit.Sub( m_vTarget_WS, m_Motion.m_Mtx.m_vPos );
		vTargetUnit.Unitize();

		// construct target velocity vector
		vTargetUnit.Mul( _CLEANER_CANISTER_MAX_LINEAR_VEL );

		// construct target acceleration vector
		vTargetUnit.Sub( m_Motion.m_LinearVelocity_WS );

		// construct target force vector
		vTargetUnit.Mul( m_Motion.m_fMass );

		m_Motion.ApplyForce_WS( &m_Motion.m_Mtx.m_vPos, &vTargetUnit );
	}


	// Handle life time...
	if( pProj->m_fMaxLifeSecs >= 0.0f )
	{
		pProj->m_fLifeSecs += FLoop_fPreviousLoopSecs;
		if( pProj->m_fLifeSecs >= pProj->m_fMaxLifeSecs )
		{
			// Canister has lived its life time...
			pProj->Detonate( TRUE, CEProj::EVENT_LIFE_TIME_OVER, NULL );
			return TRUE;
		}
	}

	// Compute linear distance we've traveled this frame...
	fDistTraveledThisFrame = pProj->GetLinSpeed() * FLoop_fPreviousLoopSecs;
	pProj->m_fDistTraveled_WS += fDistTraveledThisFrame;

	// Compute previous points and their velocities...
	PrevP1.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius *  0.9f ).Add( m_Motion.m_Mtx.m_vPos );
	PrevP2.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius * -0.9f ).Add( m_Motion.m_Mtx.m_vPos );

	// Move the body...
	m_Motion.Simple_Simulate( FLoop_fPreviousLoopSecs );

	if( m_pWhirSoundEmitter )
	{
		m_pWhirSoundEmitter->SetPosition( &m_Motion.m_Mtx.m_vPos );
	}

	// Compute current points...
	CurP1.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius *  0.9f ).Add( m_Motion.m_Mtx.m_vPos );;
	CurP2.Mul( m_Motion.m_Mtx.m_vUp, pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius * -0.9f ).Add( m_Motion.m_Mtx.m_vPos );;

	PrevToCur1.Sub( CurP1, PrevP1 );
	PrevToCur2.Sub( CurP2, PrevP2 );

	// Collide point 1 with geometry...
	bColl1 = fworld_FindClosestImpactPointToRayStart( &CollImpact1, &PrevP1, &CurP1, 0, NULL, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES );
	if( bColl1 && CollImpact1.pTag )
	{
		pWorldTracker = (CFWorldTracker *)CollImpact1.pTag;

		if( pWorldTracker->m_nUser == MESHTYPES_ENTITY )
		{
			pEntity = (CEntity *)pWorldTracker->m_pUser;

			if( pEntity->TypeBits() & ENTITY_BIT_BOT )
			{
				// Hit a bot directly...

				pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, &CollImpact1 );
				return TRUE;
			}
		}
	}

	// collide point 2 with geometry
	bColl2 = fworld_FindClosestImpactPointToRayStart( &CollImpact2, &PrevP2, &CurP2, 0, NULL, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES );
	if( bColl2 && CollImpact2.pTag )
	{
		pWorldTracker = (CFWorldTracker *)CollImpact2.pTag;

		if( pWorldTracker->m_nUser == MESHTYPES_ENTITY )
		{
			pEntity = (CEntity *)pWorldTracker->m_pUser;

			if( pEntity->TypeBits() & ENTITY_BIT_BOT )
			{
				// Hit a bot directly...

				pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, &CollImpact2 );
				return TRUE;
			}
		}
	}

	// point moving away from collision face
	if( bColl1 )
	{
		if( CollImpact1.UnitFaceNormal.Dot( PrevToCur1 ) >= 0.0f )
		{
			bColl1 = FALSE;
		}
	}

	if( bColl2 )
	{
		if( CollImpact2.UnitFaceNormal.Dot( PrevToCur2 ) >= 0.0f )
		{
			bColl2 = FALSE;
		}
	}

	if( bColl1 | bColl2 )
	{
		// There was a collision...

		if( bColl1 && bColl2 )
		{
			// Both points collided. Find first impact...

			fPushDist1 = -PrevToCur1.Dot(CollImpact1.UnitFaceNormal) * (1.01f - CollImpact1.fImpactDistInfo);
			fPushDist2 = -PrevToCur2.Dot(CollImpact2.UnitFaceNormal) * (1.01f - CollImpact2.fImpactDistInfo);

			if( fPushDist1 > fPushDist2 )
			{
				UnitFaceNormal.Set( CollImpact1.UnitFaceNormal );
				PushVec.Mul( UnitFaceNormal, fPushDist1 + 0.05f );
				ImpactPoint.Add( CurP1, PushVec );
			} 
			else
			{
				UnitFaceNormal.Set( CollImpact2.UnitFaceNormal );
				PushVec.Mul( UnitFaceNormal, fPushDist2 + 0.05f );
				ImpactPoint.Add( CurP2, PushVec );
			}
		}
		else if( bColl1 )
		{
			UnitFaceNormal.Set( CollImpact1.UnitFaceNormal );
			fPushDist1 = -PrevToCur1.Dot(UnitFaceNormal) * (1.01f - CollImpact1.fImpactDistInfo);
			PushVec.Mul( UnitFaceNormal, fPushDist1 + 0.05f );
			ImpactPoint.Add( CurP1, PushVec );
		}
		else
		{
			UnitFaceNormal.Set( CollImpact2.UnitFaceNormal );
			fPushDist2 = -PrevToCur1.Dot(UnitFaceNormal) * (1.01f - CollImpact2.fImpactDistInfo);
			PushVec.Mul( UnitFaceNormal, fPushDist2 + 0.05f );
			ImpactPoint.Add( CurP2, PushVec );
		}

		m_Motion.m_Mtx.m_vPos.Add( PushVec );

		fImpulse = m_Motion.ApplySolidWallImpulse( &ImpactPoint, &UnitFaceNormal, 1.0f, 0.0f );

		CFMtx43A Mtx;

		Mtx.m_vUp = UnitFaceNormal;
		Mtx.m_vRight.Cross( UnitFaceNormal, m_Motion.m_LinearVelocity_WS );
		if( Mtx.m_vRight.MagSq() > 0.001f )
		{
			Mtx.m_vRight.Unitize();
			Mtx.m_vFront.Cross( Mtx.m_vRight, UnitFaceNormal );
			Mtx.m_vPos.Zero();

			CFVec3A Vel_PS;

			Mtx.MulDir( Vel_PS, m_Motion.m_LinearVelocity_WS );
			f32 fTempY = Vel_PS.y;
			Vel_PS.y = 0.0f;

			f32 fMag_PS = Vel_PS.MagXZ();
			if( fMag_PS < 0.001f )
			{
				Vel_PS.Zero();
			} 
			else
			{
				Vel_PS.Unitize();
				fMag_PS -= 1100.0f * UnitFaceNormal.y * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( fMag_PS, 0.0f );
				Vel_PS.Mul( fMag_PS );
				Vel_PS.y = fTempY;
			}

			Mtx.Transpose33();
			Mtx.MulDir( m_Motion.m_LinearVelocity_WS, Vel_PS );
		}
	}

	pProj->Relocate_RotXlatFromUnitMtx_WS( &m_Motion.m_Mtx );

	return FALSE;
}


BOOL CEProj_Cleaner::AssignTarget( s32 nPlayer, CEntity *pCleanerEntity, CEntity *pTargetEntity, CFSoundGroup *pSoundGroupLock )
{
	// Set a pointer to this cleaner in the static array so the static functions can
	// find it. NOTE: Cleaner can only be used by a player!
	FASSERT(nPlayer >= 0);
	m_apTargetingCleaner[nPlayer] = this;

	if( pTargetEntity )
	{
		if( !aiutils_IsFriendly( pCleanerEntity, pTargetEntity ) )
		{
			// it's an enemy entity

			if( pTargetEntity->TypeBits() & ENTITY_BIT_BOT )
			{
				CBot *pBot = (CBot*) pTargetEntity;
				if( pBot->IsDeadOrDying() )
				{
					// targeted bot is dead or dying
					return FALSE;
				}
			}

			// check new target against existing targets
			if( InTargetList( pTargetEntity ) )
			{
				if( TargetListGuidMatches( pTargetEntity ) )
				{
					// this bot is already targeted
					return FALSE;
				}
				else
				{
					// previously targeted entity must have died
					RemoveFromTargetList( pTargetEntity );
				}
			}

			CBot *pBot = NULL;
			if( pTargetEntity->TypeBits() & ENTITY_BIT_BOT )
			{
				CBot *pBot = (CBot*) pTargetEntity;

				if( pBot->m_pDrivingVehicle )
				{
					// don't target vehicle drivers as the vehicle itself will be targeted
					return FALSE;
				}
			}

			CFVec3A vRayStart, vRayEnd, vCamPosA;
			BOOL bFoundImpact;

			// cast a ray to test visibility
			vCamPosA.Set( *(gamecam_GetActiveCamera()->GetPos()) );
			vRayStart.Set( vCamPosA );

			//Old Method: Use the head position.
			//This broke the targeting if the head was blown off of an entity
			//vRayEnd.Set( *(pTargetEntity->GetApproxEyePoint()) );

			//new method: Use the entity position + 75% of a (bot) cylinder height.
			f32 fHeight = _RETICLE_DEFAULT_HEIGHT;
			if( pBot )
			{
				fHeight = pBot->m_fCollCylinderHeight_WS;
			}

			vRayEnd.Set( pTargetEntity->MtxToWorld()->m_vPos );
			vRayEnd.y += fHeight * 0.75f;
	
			FWorld_nTrackerSkipListCount = 0;
			pTargetEntity->AppendTrackerSkipList();
			bFoundImpact = fworld_IsLineOfSightObstructed( &vRayStart, &vRayEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList );

			if( !bFoundImpact )
			{
				AddToTargetList( pTargetEntity );

				CFSoundGroup::PlaySound( pSoundGroupLock, TRUE );

	 			return TRUE;
			}
		}
	}

	return FALSE;
}


void CEProj_Cleaner::InitTargetList( void )
{
	m_nTargets = 0;

	// step through target list
	for( s32 nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		// init list values
		m_aTargets[ nIndex ].pEntity = NULL;
		m_aTargets[ nIndex ].nGuid = 0;
		m_aTargets[ nIndex ].fInsertionTime = 0.0f;

	}
}

BOOL CEProj_Cleaner::AddToTargetList( CEntity *pEntity )
{
	s32 nIndex;
	// step through target list

	if( pEntity == NULL )
	{
		return FALSE;
	}

	for( nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		if( m_aTargets[ nIndex ].pEntity == pEntity )
		{
			if( m_aTargets[ nIndex ].nGuid == pEntity->Guid() )
			{
				// this entity already in target list
				return TRUE;
			}
			else
			{
				// entity must have been reborn, update guid
				m_aTargets[ nIndex ].nGuid = pEntity->Guid();
				m_aTargets[ nIndex ].fInsertionTime = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
				return TRUE;
			}
		}
	}

	// step through target list
	for( nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		// find unused list entry
		if( m_aTargets[ nIndex ].pEntity == NULL )
		{
			// add entity to list
			m_aTargets[ nIndex ].pEntity = pEntity;
			m_aTargets[ nIndex ].nGuid = pEntity->Guid();
			m_aTargets[ nIndex ].fInsertionTime = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
			++m_nTargets;
			FASSERT( m_nTargets <= CLEANER_CANISTER_MAX_ROCKETS );
			return TRUE;
		}
	}

	// no list entries were available
	return FALSE;
}

BOOL CEProj_Cleaner::RemoveFromTargetList( CEntity *pEntity )
{
	if( pEntity == NULL )
	{
		return FALSE;
	}

	// step through target list
	for( s32 nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		// look for entity in target list
		if( m_aTargets[ nIndex ].pEntity == pEntity )
		{
			// found it, remove from list
			m_aTargets[ nIndex ].pEntity = NULL;
			m_aTargets[ nIndex ].nGuid = 0;
			m_aTargets[ nIndex ].fInsertionTime = 0.0f;

			--m_nTargets;
			FASSERT( m_nTargets >= 0 );
			return TRUE;
		}
	}

	// couldn't find entity in list
	return FALSE;
}

CEntity *CEProj_Cleaner::GetTargetByIndex( s32 nIndex )
{
	FASSERT( nIndex >= 0 && nIndex < CLEANER_CANISTER_MAX_ROCKETS );
	if( nIndex >= 0 && nIndex < CLEANER_CANISTER_MAX_ROCKETS )
	{
		// return entity pointer for the requested list entry
		return m_aTargets[ nIndex ].pEntity;
	}

	// nIndex was out of range
	return NULL;
}

f32 CEProj_Cleaner::GetTargetTimeByIndex( s32 nIndex )
{
	FASSERT( nIndex >= 0 && nIndex < CLEANER_CANISTER_MAX_ROCKETS );
	if( nIndex >= 0 && nIndex < CLEANER_CANISTER_MAX_ROCKETS )
	{
		return m_aTargets[ nIndex ].fInsertionTime;
	}

	// nIndex was out of range
	return -1.0f;
}



BOOL CEProj_Cleaner::InTargetList( CEntity *pEntity )
{
	if( pEntity == NULL )
	{
		return FALSE;
	}

	// find in list by entity pointer.
	for( s32 nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		// look for entity in target list
		if( m_aTargets[ nIndex ].pEntity == pEntity )
		{
			// found it
			// (returns TRUE even if Guids don't match)
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CEProj_Cleaner::TargetListGuidMatches( CEntity *pEntity )
{
	if( pEntity == NULL )
	{
		return FALSE;
	}

	for( s32 nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		// look for entity in target list
		if( m_aTargets[ nIndex ].pEntity == pEntity )
		{
			// found it, check guids
			if( m_aTargets[ nIndex ].nGuid == pEntity->Guid() )
			{
				// guids match
				return TRUE;
			}

			// guids didn't match
			return FALSE;
		}
	}

	// pEntity not in target list
	return FALSE;
}

static void _DrawCleanerReticle( FDrawVtx_t *pVtxArray, const CFVec3A *pSourceVerts, const CFMtx43A &rmInvCam, 
						 const CFVec3A &rPos_WS, const f32 fDeltaT, const f32 fAngle, const f32 fAlpha )
{
	CFXfm Xfm;
	CFMtx43A mReticle;
	f32 fScale;
	f32 fTranslate = 0.6f * ( 0.8f - fDeltaT );
	for( s32 nVertex = 0; nVertex < CLEANER_RETICLE_NUM_VERTICES; nVertex++ )
	{
		// animate individual reticle bars
		if( nVertex < 6 )
		{
			pVtxArray[ nVertex ].Pos_MS = pSourceVerts[ nVertex ].v3;
			pVtxArray[ nVertex ].Pos_MS.x -= fTranslate; 
			pVtxArray[ nVertex ].Pos_MS.y -= fTranslate; 
			pVtxArray[ nVertex ].ColorRGBA.fAlpha = fAlpha;
		}
		else if( nVertex < 12 )
		{
			pVtxArray[ nVertex ].Pos_MS = pSourceVerts[ nVertex ].v3;
			pVtxArray[ nVertex ].Pos_MS.x -= fTranslate; 
			pVtxArray[ nVertex ].Pos_MS.y += fTranslate; 
			pVtxArray[ nVertex ].ColorRGBA.fAlpha = fAlpha;
		}
		else if( nVertex < 18 )
		{
			pVtxArray[ nVertex ].Pos_MS = pSourceVerts[ nVertex ].v3;
			pVtxArray[ nVertex ].Pos_MS.x += fTranslate; 
			pVtxArray[ nVertex ].Pos_MS.y += fTranslate; 
			pVtxArray[ nVertex ].ColorRGBA.fAlpha = fAlpha;
		}
		else
		{
			pVtxArray[ nVertex ].Pos_MS = pSourceVerts[ nVertex ].v3;
			pVtxArray[ nVertex ].Pos_MS.x += fTranslate; 
			pVtxArray[ nVertex ].Pos_MS.y -= fTranslate; 
			pVtxArray[ nVertex ].ColorRGBA.fAlpha = fAlpha;
		}
	}

	mReticle.Identity();

	// homebrew matrix scale
	fScale = 6.0f - (5.0f * fDeltaT);
	mReticle.m_vX.x = fScale;
	mReticle.m_vY.y = fScale;
	mReticle.m_vZ.z = fScale;

	mReticle.RotateZ( fAngle );

	mReticle.RevMul( rmInvCam );
	mReticle.m_vPos.Set( rPos_WS );
	Xfm.BuildFromMtx( mReticle );
	Xfm.PushModel();
	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pVtxArray, CLEANER_RETICLE_NUM_VERTICES );
	CFXfm::PopModel();
}

#define _TARGET_TEXT_BLINK_TIME		( 0.6f )
// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.

// draws cleaner target reticles and target info text.
// also removes dead and out-of-range targets from target list.
void CEProj_Cleaner::DrawTargets( s32 nPlayer )
{
	f32 fGreen;
	s8 szText[ 16 ];

	CEProj_Cleaner* pCleaner = m_apTargetingCleaner[nPlayer];
	if( !pCleaner )
	{
		// no canister active
		return;
	}

	// handle blinky text timer
	pCleaner->m_fTextBlinkTimer += FLoop_fPreviousLoopSecs;
	if( pCleaner->m_fTextBlinkTimer > _TARGET_TEXT_BLINK_TIME )
	{
		pCleaner->m_fTextBlinkTimer = 0.0f;
	}

	if( CHud2::GetHudForPlayer( nPlayer )->IsWSActive() ) {
		//set the blink time to 70% past the blink cycle...  (so we don't get quick blinks...)
		//This also ensures that the cleaner text will not be visible on the Weapon Select screens 
		pCleaner->m_fTextBlinkTimer = _TARGET_TEXT_BLINK_TIME * 0.71f;
	}

	if( !pCleaner->IsLaunched() )
	{
		// canister is active but not yet launched
		if( pCleaner->m_fTextBlinkTimer < _TARGET_TEXT_BLINK_TIME * 0.7f )
		{
			// Put the text inside the safe area
			fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportSafeOrtho3D);

			// print blinking text
			if( pCleaner->NumTargetsInList() == CLEANER_CANISTER_MAX_ROCKETS )
			{
				ftext_PrintString( pCleaner->m_hTextBoxSelectTargets, Game_apwszPhrases[ GAMEPHRASE_LAUNCH_CLEANER ] );
			}
			else
			{
				ftext_PrintString( pCleaner->m_hTextBoxSelectTargets, Game_apwszPhrases[ GAMEPHRASE_SELECT_TARGETS ] );
			}

			// need to find string functions...
			for( s32 nChar = 0; nChar < 16; nChar++ )
			{
				if( nChar & 1 )
				{
					szText[ nChar ] = ' ';
				}
				else 
				{
					if( ( nChar >> 1 ) < pCleaner->NumTargetsInList() )
					{
						szText[ nChar ] = '1' + (s8)(nChar >> 1);
					}
					else
					{
						szText[ nChar ] = 0;
						break;
					}
				}
			}
			szText[ 15 ] = 0;	//JIC

			ftext_PrintString( pCleaner->m_hTextBoxNumTargets, (const char*) szText );
		}
	}

	if( !pCleaner->NumTargetsInList() )
	{
		// canister launched but no has no targets 
		return;
	}

	CFVec3A vPos_WS, vDeltaPos, vCamPosA;
	_ReticleHistory_t *pHistory;
	CEntity *pEntity;
	f32 fHeight, fDeltaT;
	int nIndex;
	CFCamera *pCurCam;
	FDrawVtx_t *pVtxArray;
	s32 nVertex;

	pVtxArray = fvtxpool_GetArray( CLEANER_RETICLE_NUM_VERTICES );

	if( pVtxArray == NULL )
	{
		return;
	}

	pCurCam = gamecam_GetActiveCamera();
	// animate reticle color
	fGreen = fmath_Sin( pCleaner->m_fAngle );
	pCleaner->m_fAngle += FMATH_DEG2RAD( 540.0f * FLoop_fPreviousLoopSecs );
	if( pCleaner->m_fAngle > FMATH_DEG2RAD( 360.0f ) )
	{
		pCleaner->m_fAngle -= FMATH_DEG2RAD( 360.0f );
	}
	fGreen = (( fGreen + 1.0f ) * 0.25f ) + 0.5f;

	for( nVertex = 0; nVertex < CLEANER_RETICLE_NUM_VERTICES; nVertex++ )
	{
		pVtxArray[ nVertex ].ColorRGBA.Set( 1.0f, fGreen, 0.2f, 0.99f );
	}

	// set up drawing modes
	frenderer_SetDefaultState();
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fviewport_SetActive(Player_aPlayer[nPlayer].m_pViewportPersp3D);


	// turn off draw flags for current trail index of each reticle history
	// if reticle is alive this frame, draw flag will be turned back on.
	for( nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		pCleaner->m_aReticleHistory[ nIndex ][ pCleaner->m_nReticleHistoryIndex ].bDraw = FALSE;
	}

	// step through target array and draw target reticles around active targets
	for( nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		pEntity = pCleaner->GetTargetByIndex( nIndex );
		if( pEntity == NULL )
		{
			continue;
		}

		if( pEntity->TypeBits() & ENTITY_BIT_BOT )
		{
			CBot *pBot = (CBot*) pEntity;
			if( pBot->IsDeadOrDying() )
			{
				// targeted bot is dead, remove from target list
				pCleaner->RemoveFromTargetList( pEntity );
				continue;
			}
		}

		if( !pEntity->IsInWorld() || !pCleaner->TargetListGuidMatches( pEntity ) )
		{
			// targeted entity is dead, remove from target list
			pCleaner->RemoveFromTargetList( pEntity );
			continue;
		}

		vCamPosA.Set( *pCurCam->GetPos() );
		vDeltaPos.Set( pEntity->MtxToWorld()->m_vPos );
		vDeltaPos.Sub( vCamPosA );
		if( vDeltaPos.MagSq() > CLEANER_CANISTER_MAX_RANGE_SQ )
		{
			// targeted bot is out of range, remove from list
			pCleaner->RemoveFromTargetList( pEntity );
			continue;
		}

		// find height above mount point at which to draw reticle
		if( pEntity->TypeBits() & ENTITY_BIT_BOT )
		{
			CBot *pBot = (CBot*) pEntity;
			fHeight = pBot->m_fCollCylinderHeight_WS;
		}
		else
		{
			fHeight = _RETICLE_DEFAULT_HEIGHT;
		}

		// find delta time since this entity was targeted
		fDeltaT = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick - pCleaner->GetTargetTimeByIndex( nIndex );
		if( fDeltaT > 1.0f )
		{
			fDeltaT = 1.0f;
		}

		// find pointer to reticle history data for this target.
		// (2nd dimension of array implements a set of history data that
		// is cycled through to create a reticle trail effect for each target.)
		pHistory = &(pCleaner->m_aReticleHistory[ nIndex ][ pCleaner->m_nReticleHistoryIndex ]);

		// update animation data for this reticle
		pHistory->vPos_WS.Set( pEntity->MtxToWorld()->m_vPos );
		pHistory->vPos_WS.y += fHeight * 0.75f;
		pHistory->fDelta = fDeltaT;
		pHistory->fAngle = pCleaner->m_fAngle;
		pHistory->bDraw = TRUE;
	}

	f32 fAlpha;
	for( nIndex = 0; nIndex < CLEANER_CANISTER_MAX_ROCKETS; nIndex++ )
	{
		// step through all targets

		for( s32 nTrail = 0; nTrail < CLEANER_RETICLE_MAX_TRAIL; nTrail++ )
		{
			// step through reticles (used to create the trail effect)

			pHistory = &(pCleaner->m_aReticleHistory[ nIndex ][ nTrail ]);

			if( nTrail != pCleaner->m_nReticleHistoryIndex && pHistory->fDelta >= 1.0f )
			{
				// don't draw trail after entity has been targeted for fDelta seconds,
				// (only draw one reticle in that case.)
				pHistory->bDraw = FALSE;
			}

			// animate the alpha of the reticle trail
			if( pCleaner->m_nReticleHistoryIndex < nTrail )
			{
				fAlpha = (f32) (pCleaner->m_nReticleHistoryIndex + CLEANER_RETICLE_MAX_TRAIL - nTrail); 
				fAlpha = 1.0f - ( fAlpha * fmath_Inv( (f32) ( CLEANER_RETICLE_MAX_TRAIL - 1 ) ) );
				fAlpha *= 0.7f;
			}
			else
			{
				fAlpha = (f32) (pCleaner->m_nReticleHistoryIndex - nTrail); 
				fAlpha = 1.0f - ( fAlpha * fmath_Inv( (f32) ( CLEANER_RETICLE_MAX_TRAIL - 1 ) ) );
				fAlpha *= 0.7f;
			}

			if( pHistory->bDraw )
			{
				// draw reticles
				_DrawCleanerReticle( pVtxArray, pCleaner->m_avReticleVertex, pCurCam->GetFinalXfm()->m_MtxR, 
							pHistory->vPos_WS, pHistory->fDelta, pHistory->fAngle, fAlpha );
			}
		}
	}

	// increment index into reticle trail array
	pCleaner->m_nReticleHistoryIndex++;
	if( pCleaner->m_nReticleHistoryIndex >= CLEANER_RETICLE_MAX_TRAIL )
	{
		pCleaner->m_nReticleHistoryIndex = 0;
	}

	fvtxpool_ReturnArray( pVtxArray );
}
