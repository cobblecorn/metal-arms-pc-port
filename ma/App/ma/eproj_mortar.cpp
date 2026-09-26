//////////////////////////////////////////////////////////////////////////////////////
// eproj_mortar.cpp - cannonballs for mortar projectiles
//
// Author: Michael Scholz
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
// 02/25/03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eproj_mortar.h"
#include "floop.h"
#include "fresload.h"
#include "explosion.h"
#include "meshtypes.h"
#include "fsndfx.h"
#include "damage.h"

// #define _TRAIL_PARTICLE_NAME	"stf_prt01"
#define _TRAIL_PARTICLE_NAME	"PRMMball_01"
#define _STREAMER_TEXTURE_NAME	"TFMEStream1"
#define _STREAMER_INITIAL_ALPHA	1.00f


BOOL					CEProj_Mortar::m_bSystemLoaded		= FALSE;
BOOL					CEProj_Mortar::m_bEffectsLoaded		= FALSE;
u32						CEProj_Mortar::m_nClassClientCount  = 0;
FParticle_DefHandle_t	CEProj_Mortar::m_hTrailParticleDef = FPARTICLE_INVALID_HANDLE;
CFTexInst				CEProj_Mortar::_StreamerTexInst;

static CEProj* _pThisProj = NULL;

BOOL CEProj_Mortar::LoadSharedResources( void )
{
	FASSERT( m_nClassClientCount != 0xffffffff );

	++m_nClassClientCount;

	if( m_nClassClientCount > 1 ) 
	{
		// Resources already loaded...
		return TRUE;
	}
	FASSERT( !m_bSystemLoaded );
	m_bSystemLoaded = TRUE;

	m_hTrailParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _TRAIL_PARTICLE_NAME );
	if( m_hTrailParticleDef == FPARTICLE_INVALID_HANDLE) 
	{
		goto _ExitInitSystemWithError;
	}
	return TRUE;

_ExitInitSystemWithError:
	return FALSE;
}

void CEProj_Mortar::UnloadSharedResources( void )
{
	FASSERT( m_nClassClientCount > 0 );

	--m_nClassClientCount;

	if( m_nClassClientCount > 0 ) 
	{
		return;
	}
	FASSERT( m_bSystemLoaded );
	m_bSystemLoaded = FALSE;
}


BOOL CEProj_Mortar::Create( CEProj *pProj )
{
	return TRUE;
}

void CEProj_Mortar::Destroy( CEProj *pProj )
{
}
//
//
//
void CEProj_Mortar::Init( CEProj *pProj )
{
	m_vWorldCollideOffset.Set(CFVec3A::m_Null);
	m_bAbortCollisionTests = FALSE;
	f32 fRadius = pProj->GetWorldMesh()->m_BoundSphere_MS.m_fRadius;
	f32 fMass = 1.0f;//50.0f;

	m_Motion.Simple_InitBody( pProj->MtxToWorld(), CFMotion::ComputeInvUniformInertiaTensorMag( fRadius, fMass ), fMass, -32.0f );
}

void CEProj_Mortar::Removed( CEProj *pProj )
{
	if (m_hTrailParticle != FPARTICLE_INVALID_HANDLE)
	{
		fparticle_StopEmitter(m_hTrailParticle);
		m_hTrailParticle = FPARTICLE_INVALID_HANDLE;
	}
	pProj->StopAmbientSFX();
/*
	pProj->StopAmbientSFX();
	pProj->DetachFromParent();

	if( pProj->m_pWorldMesh )
	{
		pProj->m_pWorldMesh->RemoveFromWorld();
		FASSERT(pProj->m_pWorldMesh->GetAttachedLightCount()==1);
		pProj->m_pWorldMesh->RemoveAttachedLight(&m_WorldLight);
	}

	FASSERT(!m_WorldLight.IsAddedToWorld());
*/
}

//
//
//
void CEProj_Mortar::Launched( CEProj *pProj )
{
	m_Motion.m_AngularVelocity_WS.Mul( *pProj->GetRotUnitAxis_WS(), pProj->GetRotSpeed() );
	m_Motion.m_LinearVelocity_WS.Mul( *pProj->GetLinUnitDir_WS(), pProj->GetLinSpeed() );

	m_Motion.Simple_UpdateOrientation( pProj->MtxToWorld() );

	m_hTrailParticle = fparticle_SpawnEmitter( m_hTrailParticleDef, &pProj->GetWorldMesh()->m_Xfm.m_MtxF.m_vPos.v3, &pProj->MtxToWorld()->m_vUp.v3);
	if (m_hTrailParticle != FPARTICLE_INVALID_HANDLE)
	{
		fparticle_SetDirection(m_hTrailParticle, &pProj->MtxToWorld()->m_vUp.v3);
	}
	pProj->PlayAmbientSFX();
/*
	pProj->SetMaxHealth();
	pProj->SetCollisionFlag(TRUE);
	FASSERT(((CFWorldTracker*)pProj->GetWorldMesh())->IsCollisionFlagSet());
	pProj->SetDeathCallback(CEProj_Mortar::DeathCallback);
	pProj->PlayAmbientSFX();
	pProj->StreamerOn( &_StreamerTexInst, _STREAMER_INITIAL_ALPHA, CFXStreamerEmitter::USE_UP_AXIS, pProj->GetWorldMesh()->GetBoundingSphere().m_fRadius, 16 );

	FLightInit_t* pLightInit = pProj->GetLightInit();
	if (pLightInit)
	{
		pLightInit->spInfluence.m_Pos.Set(0.f,0.f,0.f);
		m_WorldLight.Init(pLightInit, pProj->GetWorldMesh());
		FASSERT(pProj->m_pWorldMesh->GetAttachedLightCount()==0);
		pProj->GetWorldMesh()->AddAttachedLight(&m_WorldLight);
	}
*/
}


//
//	Explodes the arrow on the target
//
BOOL CEProj_Mortar::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact )
{
	m_bAbortCollisionTests = TRUE;

	return TRUE;
}


//
//	Performs the effects for the arrow hitting the target, return TRUE on destruction, FALSE otherwise
//
BOOL CEProj_Mortar::Work( CEProj *pProj )
{
	FASSERT( pProj );
	FASSERT( pProj->IsInWorld() );
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

	CFSphere *pSphere = (CFSphere *) &pProj->GetWorldMesh()->GetBoundingSphere();

	//****************
	//* Collide our collision sphere with trackers, detonating cases of non-world collision
	CFTrackerCollideSpheresInfo trackerCollSpheresInfo;
	trackerCollSpheresInfo.bIgnoreCollisionFlag		= FALSE;
	trackerCollSpheresInfo.nTrackerTypeBits			= FWORLD_TRACKERTYPEBIT_MESH;
	trackerCollSpheresInfo.nTrackerUserTypeBitsMask	= ENTITY_BITS_ALLBOTBITS;
	trackerCollSpheresInfo.pMasterSphere_WS			= (CFSphere*)&pProj->GetWorldMesh()->GetBoundingSphere();
	trackerCollSpheresInfo.pCallback				= _CollideSphereCB;
	trackerCollSpheresInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	trackerCollSpheresInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;

	fcoll_Clear(); // anything in the collion buffer comes from us now
	_pThisProj = pProj;
	fworld_CollideWithTrackers( &trackerCollSpheresInfo, pProj->GetWorldMesh() );
	_pThisProj = NULL;

	if (m_bAbortCollisionTests) // no point in continuing
		return TRUE;

	// Compute linear distance we've traveled this frame...
	f32 fDistTraveledThisFrame = pProj->GetLinSpeed() * FLoop_fPreviousLoopSecs;
	pProj->m_fDistTraveled_WS += fDistTraveledThisFrame;

	// Move the body...
	CFVec3A PrevPos;
	CFVec3A PrevToCurr;
	PrevPos.Set( m_Motion.m_Mtx.m_vPos );

	m_Motion.Simple_Simulate( FLoop_fPreviousLoopSecs );
	
	PrevToCurr.Sub( m_Motion.m_Mtx.m_vPos, PrevPos );

	//****************
	//* Collide our collision sphere with the world
	CFCollInfo collInfo;
	collInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	collInfo.bFindClosestImpactOnly = FALSE;
	collInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	collInfo.bCullBacksideCollisions = TRUE;
	collInfo.bCalculateImpactData = TRUE;
	collInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	collInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	collInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	collInfo.pTag = NULL;// so, don't try to get a meshentity out of it, it's a world geo check
	collInfo.ProjSphere.Init( &PrevPos, &m_Motion.m_Mtx.m_vPos, pSphere->m_fRadius );
	FCollImpact_t* pImpact = NULL;
	fworld_CollideWithWorldTris(&collInfo, pProj->GetWorldMesh());
	if( FColl_nImpactCount > 0 ) 
	{
		fcoll_Sort( FALSE );
		pImpact = FColl_apSortedImpactBuf[0];
		pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, pImpact );
		return TRUE;
	}



	pProj->Relocate_RotXlatFromUnitMtx_WS( &m_Motion.m_Mtx );

	return FALSE;
}

BOOL CEProj_Mortar::_CollideSphereCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex )
{
	FASSERT(_pThisProj);

	if (pTracker->m_nUser == FWORLD_USERTYPE_WIRE)
	{
		// ignore collision with wires
		return TRUE;
	}

	if (pTracker == _pThisProj->GetDamager()->pBot->m_pWorldMesh)
	{
		// don't collide with the launcher
		return TRUE;
	}

	CFCollInfo collInfo;
	collInfo.nCollTestType		= FMESH_COLLTESTTYPE_SPHERE;
	collInfo.nCollMask			= FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	collInfo.nResultsLOD		= FCOLL_LOD_HIGHEST;
	collInfo.nTrackerUserTypeBitsMask = -1;
	collInfo.nStopOnFirstOfCollMask	  = FCOLL_MASK_CHECK_ALL;
	collInfo.bFindClosestImpactOnly	  = FALSE;
	collInfo.bCullBacksideCollisions  = FALSE;
	collInfo.bCalculateImpactData	  = TRUE;
	collInfo.pSphere_WS	  = (CFSphere*) &_pThisProj->GetWorldMesh()->GetBoundingSphere();
	collInfo.pTag = pTracker;
	if( ((CFWorldMesh*)pTracker)->CollideWithMeshTris( &collInfo ) )
	{
		// we hit an entity...
		_pThisProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, &FColl_aImpactBuf[0]);
		return FALSE;
	}

	return TRUE;
}


