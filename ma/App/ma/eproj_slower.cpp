//////////////////////////////////////////////////////////////////////////////////////
// eproj_slower.cpp - slowdown projectiles.
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
// 02.07.03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eproj_slower.h"
#include "floop.h"
#include "fresload.h"
#include "explosion.h"
#include "meshtypes.h"
#include "fsndfx.h"
#include "damage.h"

#define _TRAIL_PARTICLE_NAME	"stf_prt01" 
#define _STREAMER_TEXTURE_NAME	"TFMEStream1"
#define _STREAMER_INITIAL_ALPHA	1.00f

#define _SPLASH_MESH		"gfme_disc"
#define _PROJ_TEXLAYER_ID		50
#define _SPLASH_TEXLAYER_ID		50
#define _MAX_SPLASHES			20
#define _SPLASH_INITIAL_ALPHA	1.0f
#define _SPLASH_FINAL_ALPHA		0.02f
#define _EFFECT_TIMESCALE		1.25f		// (1.0f/0.75f)

BOOL		CEProj_Slower::m_bSystemInitialized	= FALSE;
BOOL		CEProj_Slower::m_bEffectsLoaded		= FALSE;

FParticle_DefHandle_t	CEProj_Slower::m_hTrailParticleDef = FPARTICLE_INVALID_HANDLE;
CFTexInst				CEProj_Slower::_StreamerTexInst;
FLinkRoot_t				CEProj_Slower::m_FreeSplashList;
_ShieldSplash_t*		CEProj_Slower::m_aSplashes	= NULL;
f32						CEProj_Slower::m_fMaxDevLengthSquared=999.f;

static CEProj* _pThisProj = NULL;

struct _ShieldSplash_t 
{
	f32				fUnitTime;				//how long it's been active 1.0 = dead
	CFVec3A			 vDisplacement;			//displacement from center
	CEProj_Slower	*pShield;				//which shield we came from
	CFWorldMesh		*pMesh;					//our splash worldmesh inst
	FMeshTexLayerHandle_t	hTexLayer;	//the tex layer we're animating
	FLink_t			link;					//link to other _ShieldSplashes
};


BOOL CEProj_Slower::InitSystem( f32 fMaxDeviationSquared )
{
	FASSERT( !m_bSystemInitialized );
	m_bSystemInitialized = TRUE;
	
	m_fMaxDevLengthSquared = fMaxDeviationSquared;

	m_hTrailParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _TRAIL_PARTICLE_NAME );
	
	_StreamerTexInst.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, _STREAMER_TEXTURE_NAME ) );
	if( !_StreamerTexInst.GetTexDef() ) 
	{
		goto _ExitInitSystemWithError;
	}

	return TRUE;
_ExitInitSystemWithError:
	return FALSE;
}

void CEProj_Slower::UninitSystem( void )
{
	FASSERT( m_bSystemInitialized );
	m_bSystemInitialized = FALSE;
}


BOOL CEProj_Slower::Create( CEProj *pProj )
{
	flinklist_InitRoot( &m_SplashList, FANG_OFFSETOF( _ShieldSplash_t, link ) );

	return TRUE;
}

void CEProj_Slower::Destroy( CEProj *pProj )
{
}
//
//
//
void CEProj_Slower::Init( CEProj *pProj )
{
	m_vWorldCollideOffset.Set(CFVec3A::m_Null);
	m_bAbortCollisionTests = FALSE;
}

void CEProj_Slower::Removed( CEProj *pProj )
{
	FMATH_CLEARBITMASK( pProj->m_nProjFlags, CEProj::PROJFLAG_MANUAL_XFM );

	pProj->StopAmbientSFX();
	pProj->DetachFromParent();

	if( pProj->m_pWorldMesh ) 
	{
		pProj->m_pWorldMesh->RemoveFromWorld();
		FASSERT(pProj->m_pWorldMesh->GetAttachedLightCount()==1);
		pProj->m_pWorldMesh->RemoveAttachedLight(&m_WorldLight);
	}

	if (m_hTrailParticle != FPARTICLE_INVALID_HANDLE)
	{
		fparticle_StopEmitter(m_hTrailParticle);
		m_hTrailParticle = FPARTICLE_INVALID_HANDLE;
	}
	FASSERT(!m_WorldLight.IsAddedToWorld());

	_KillSplash(0.0f,TRUE);
}

//
//
//
const f32 _fInitialSpeedBoost = 25.0f;
const f32 _fSpeedBoostDecelPerSec = 10.0f;
const f32 _fSlowerSeekAbilityBase = 0.2f;
const f32 _fSlowerSeekAbilityHighSpeedPenalty = -0.18f;

void CEProj_Slower::Launched( CEProj *pProj )
{
	pProj->SetMaxHealth();
	pProj->SetCollisionFlag(TRUE);
	FASSERT(((CFWorldTracker*)pProj->GetWorldMesh())->IsCollisionFlagSet());
	pProj->SetDeathCallback(CEProj_Slower::DeathCallback);
	pProj->PlayAmbientSFX();
	pProj->StreamerOn( &_StreamerTexInst, _STREAMER_INITIAL_ALPHA, CFXStreamerEmitter::USE_UP_AXIS, pProj->GetWorldMesh()->GetBoundingSphere().m_fRadius, 16 );
	m_fSpeedBoost = _fInitialSpeedBoost;
	m_fStationaryTimer = 3.0f+fmath_RandomFloat()*5.0f;
	m_bGoneStationary = FALSE;

	m_hTrailParticle = fparticle_SpawnEmitter( m_hTrailParticleDef, &pProj->GetWorldMesh()->m_Xfm.m_MtxF.m_vPos.v3, &pProj->MtxToWorld()->m_vUp.v3); 
	if (m_hTrailParticle != FPARTICLE_INVALID_HANDLE)
	{
		fparticle_SetDirection(m_hTrailParticle, &pProj->MtxToWorld()->m_vUp.v3);
	}
	FLightInit_t* pLightInit = pProj->GetLightInit();
	if (pLightInit)
	{
		pLightInit->spInfluence.m_Pos.Set(0.f,0.f,0.f);
		m_WorldLight.Init(pLightInit, pProj->GetWorldMesh());
		FASSERT(pProj->m_pWorldMesh->GetAttachedLightCount()==0);
		pProj->GetWorldMesh()->AddAttachedLight(&m_WorldLight);
	}
}


//
//	Explodes the arrow on the target
//
BOOL CEProj_Slower::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact )
{
	m_bAbortCollisionTests = TRUE;

	return TRUE;
}


//
//	Performs the effects for the arrow hitting the target, return TRUE on destruction, FALSE otherwise
//

BOOL CEProj_Slower::Work( CEProj *pProj )
{
		
	// handle the splashes
	_ShieldSplash_t *pSplash = (_ShieldSplash_t*)flinklist_GetHead( &m_SplashList );
	while( pSplash != NULL ) 
	{
		if( pSplash->fUnitTime >= 1.0f ) 
		{
			pSplash = _ReleaseSplash( pSplash );
		} 
		else 
		{
			_UpdateSplashPosition( pProj->m_pWorldMesh->GetBoundingSphere().m_Pos, pSplash );
			pSplash->fUnitTime += FLoop_fPreviousLoopSecs * _EFFECT_TIMESCALE;
			pSplash = (_ShieldSplash_t*)flinklist_GetNext( &m_SplashList, pSplash );
		}
	}

	if (!m_bGoneStationary)
	{
		//
		// Move
		//

		// target seeking feature
		if( pProj->GetTargetedEntity() )
		{
			// this is a guided projectile
			CFVec3A LinUnitDir_WS;

			// point projectile at target
			CFVec3A Goal;
			if (pProj->GetTargetedEntity()->GetTagPoint(0))
			{
				Goal = *(pProj->GetTargetedEntity()->GetTagPoint(0));
			}
			else
			{
				Goal = pProj->GetTargetedEntity()->MtxToWorld()->m_vPos;
			}
			LinUnitDir_WS.Sub(Goal , pProj->MtxToWorld()->m_vPos );
			if (LinUnitDir_WS.SafeUnitAndMag(LinUnitDir_WS) > 0.0f && pProj->GetLinUnitDir_WS())
			{
				CFVec3A tmp;
				tmp = *(pProj->GetLinUnitDir_WS());
				f32 fSeekUnit = _fSlowerSeekAbilityBase;

				f32 fCtrl = fmath_Div(m_fSpeedBoost, _fInitialSpeedBoost);
				fSeekUnit += FMATH_FPOT(fCtrl, 0.0f, _fSlowerSeekAbilityHighSpeedPenalty);
				tmp.Mul(1.0f- fSeekUnit);
				LinUnitDir_WS.Mul(fSeekUnit);
				LinUnitDir_WS.Add(tmp);


				pProj->SetLinUnitDir_WS( &LinUnitDir_WS );
			}

			// set new orientation
			pProj->MtxToWorld()->m_vFront = *pProj->GetLinUnitDir_WS();
			pProj->MtxToWorld()->m_vRight.UnitCrossYWithVec( *pProj->GetLinUnitDir_WS() );	// doesn't handle asymptotic case
			pProj->MtxToWorld()->m_vUp.Cross( pProj->MtxToWorld()->m_vFront, pProj->MtxToWorld()->m_vRight );
		}

		// Compute linear distance we've traveled this frame...
		if (m_fSpeedBoost> 0.0f)
		{
			m_fSpeedBoost -= _fSpeedBoostDecelPerSec*FLoop_fPreviousLoopSecs;
		}
		else
		{
			m_fSpeedBoost = 0.0f;
		}
		f32 fDistTraveledThisFrame = (m_fSpeedBoost+pProj->GetLinSpeed()) * FLoop_fPreviousLoopSecs;
		pProj->m_fDistTraveled_WS += fDistTraveledThisFrame;

		CFVec3A vTravel;
		vTravel.Mul(*pProj->GetLinUnitDir_WS(), fDistTraveledThisFrame);
		
		CFMtx43A NewMtx;
		NewMtx.m_vPos.Set(vTravel);
		NewMtx.m_vPos.Add( pProj->MtxToWorld()->m_vPos );

		// Compute projectile orientation...
		NewMtx.m_vRight = pProj->MtxToWorld()->m_vRight;
		NewMtx.m_vUp = pProj->MtxToWorld()->m_vUp;
		NewMtx.m_vFront = pProj->MtxToWorld()->m_vFront;

		const CFMtx43A* pMtxToWorld =  pProj->MtxToWorld();

	//	const CFSphere& rBSphere = pProj->GetWorldMesh()->GetBoundingSphere();
	//	fdraw_DevSphere(&rBSphere.m_Pos,rBSphere.m_fRadius);

		FWorld_nTrackerSkipListCount = 0;
		if (pProj->GetDamagerBot())
		{
			pProj->GetDamagerBot()->AppendTrackerSkipList();
		}

		//****************
		//* Collide our collision sphere with trackers, detonating cases of non-world collision
		CFTrackerCollideSpheresInfo trackerCollSpheresInfo;
		trackerCollSpheresInfo.bIgnoreCollisionFlag		= FALSE;
		trackerCollSpheresInfo.nTrackerTypeBits			= FWORLD_TRACKERTYPEBIT_MESH;
		trackerCollSpheresInfo.nTrackerUserTypeBitsMask	= ENTITY_BITS_ALLBOTBITS;
		trackerCollSpheresInfo.pMasterSphere_WS			= (CFSphere*)&pProj->GetWorldMesh()->GetBoundingSphere();
		trackerCollSpheresInfo.pCallback				= _CollideSphereCB;
		trackerCollSpheresInfo.nTrackerSkipCount		= FWorld_nTrackerSkipListCount;
		trackerCollSpheresInfo.ppTrackerSkipList		= FWorld_apTrackerSkipList;

		fcoll_Clear(); // anything in the collion buffer comes from us now

		_pThisProj = pProj;
		fworld_CollideWithTrackers( &trackerCollSpheresInfo, pProj->GetWorldMesh() );
		_pThisProj = NULL;

		if (m_bAbortCollisionTests) // no point in continuing
			return TRUE;

		//****************
		//* Collide our collision sphere with the world, adjust draw position in intersecting cases,
		CFCollInfo collInfo;
		collInfo.nCollTestType		= FMESH_COLLTESTTYPE_SPHERE;
		collInfo.nCollMask			= FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
		collInfo.nResultsLOD		= FCOLL_LOD_HIGHEST;
		collInfo.nTrackerUserTypeBitsMask = -1;
		collInfo.nStopOnFirstOfCollMask	  = FCOLL_MASK_NONE;
		collInfo.bFindClosestImpactOnly	  = FALSE;
		collInfo.bCullBacksideCollisions  = FALSE;
		collInfo.bCalculateImpactData	  = TRUE;
		collInfo.pSphere_WS				  = (CFSphere*) &(pProj->GetWorldMesh()->GetBoundingSphere());
		collInfo.pTag					  = NULL; // so, don't try to get a meshentity out of it, it's a world geo check
		FCollImpact_t* pImpact = NULL;
		fworld_CollideWithWorldTris(&collInfo, pProj->GetWorldMesh());
		CFVec3A vCollide;
		if( FColl_nImpactCount > 0 ) 
		{
			fcoll_Sort( FALSE );
			pImpact = FColl_apSortedImpactBuf[0];
			vCollide.Mul(pImpact->UnitFaceNormal,pImpact->fImpactDistInfo);
			m_vWorldCollideOffset.Add(vCollide);
			FMATH_SETBITMASK( pProj->m_nProjFlags, CEProj::PROJFLAG_MANUAL_XFM );
			
			// Restrict deviation by a pre-computed amount
			if (m_vWorldCollideOffset.MagSq() > m_fMaxDevLengthSquared)
			{
				// act like we hit geo
	//			pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, pImpact );
				m_bGoneStationary = TRUE;
				return TRUE;
			}
		}

		//****************
		//* Collide our collision ray with the world, detonate intersection cases
		//f32 fRadius = ((CFWorldTracker*)pProj->GetWorldMesh())->GetBoundingSphere().m_fRadius;
		CFVec3A StartCollPoint_WS,EndCollPoint_WS;
		StartCollPoint_WS.Set(pProj->MtxToWorld()->m_vPos);
		EndCollPoint_WS.Set(StartCollPoint_WS);
		EndCollPoint_WS.Add(vTravel);
	/*
		CFColorRGBA green,red;green.OpaqueGreen();red.OpaqueRed();
		fdraw_DevLine(&StartCollPoint_WS,&EndCollPoint_WS,&green,&red);
		fdraw_DevSphere(&StartCollPoint_WS,.1f);
		fdraw_DevSphere(&EndCollPoint_WS,.1f, &red);
		
		CFColorRGBA blue,magenta;blue.OpaqueBlue();magenta.OpaqueMagenta();
		EndCollPoint_WS.Mul(*pProj->GetLinUnitDir_WS(),10.f).Add(StartCollPoint_WS);
		fdraw_DevLine(&StartCollPoint_WS,&EndCollPoint_WS,&blue,&magenta);
		fdraw_DevSphere(&pProj->MtxToWorld()->m_vPos,4.0f);
	*/
		FCollImpact_t CollImpact;
		if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &StartCollPoint_WS, &EndCollPoint_WS, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES ) ) {
			// Found collision...

			//NewMtx.m_vPos.Mul( *pProj->GetLinUnitDir_WS(), -0.01f ).Add( CollImpact.ImpactPoint );
			NewMtx.m_vPos.Set( CollImpact.ImpactPoint );
			pProj->Relocate_Xlat_WS( &NewMtx.m_vPos );
			
	//		CFColorRGBA red;red.OpaqueRed();
	//		fdraw_DevSphere(&CollImpact.ImpactPoint,.25f,&red);

	//		pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, &CollImpact );
			m_bGoneStationary = TRUE;
			return TRUE;
		}

		// update position of projectile
		pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx );
		
		// if boosted, manually update mesh location in world
		if (pProj->m_nProjFlags & CEProj::PROJFLAG_MANUAL_XFM)
		{
			NewMtx.m_vPos.Add(m_vWorldCollideOffset);
			pProj->m_pWorldMesh->m_Xfm.BuildFromMtx(NewMtx,pProj->m_fScaleToWorld);
			pProj->m_pWorldMesh->UpdateTracker();
		}
	/*
		CFColorRGBA blue;blue.OpaqueBlue();
		fdraw_DevSphere(&pProj->MtxToWorld()->m_vPos,1.0f,&blue);
		
		CFColorRGBA white;white.OpaqueWhite();
		CFVec3A bsPos(pProj->GetWorldMesh()->GetBoundingSphere().m_Pos);
		fdraw_DevSphere(&bsPos,pProj->GetWorldMesh()->GetBoundingSphere().m_fRadius,&white);

		CFColorRGBA red;red.OpaqueRed();
		CFVec3A bsPos2(m_WorldLight.GetBoundingSphere().m_Pos);
		fdraw_DevSphere(&bsPos2,m_WorldLight.GetBoundingSphere().m_fRadius,&red);
	*/
	}
	return FALSE;
}

BOOL CEProj_Slower::_CollideSphereCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex )
{
	FASSERT(_pThisProj);

	if (pTracker->m_nUser == FWORLD_USERTYPE_WIRE)
	{
		// ignore collision with wires
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
	collInfo.pSphere_WS				  = (CFSphere*) &_pThisProj->GetWorldMesh()->GetBoundingSphere();
	collInfo.pTag = pTracker;	  
	if( ((CFWorldMesh*)pTracker)->CollideWithMeshTris( &collInfo ) ) 
	{
		// we hit an entity...
//		_pThisProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, &FColl_aImpactBuf[0]);	
		((CEProj_Slower*) _pThisProj)->m_bGoneStationary = TRUE;
		return FALSE;
	}

	return TRUE;
}

void CEProj_Slower::DamageCallback( CEProj *pProj, CDamageData *pDamageData )
{
	FCollImpact_t CollImpact;
	// pDamageData->InitCollImpactFromTriData(&CollImpact);

	// put flash here.
	CEProj_Slower *pSlowerProjectile = (CEProj_Slower*) pProj->GetExtensionObject( CEProj::PROJTYPE_SLOWER );
	pSlowerProjectile->_AddSplashEffect( pProj, pDamageData->m_ImpactPoint_WS );

	// take the damage
	pProj->CEntity::InflictDamage(pDamageData);
	
}

void CEProj_Slower::DeathCallback( CEProj* pProj )
{
	// kill this projectile
//	pProj->Detonate( TRUE, CEProj::EVENT_INTERCEPTED, NULL );

	//somebody must have shot the thing in the air, or somehting.
	((CEProj_Slower*) pProj)->m_bGoneStationary = TRUE;
}

BOOL CEProj_Slower::InitEffects( void )
{
	FASSERT( m_bSystemInitialized );
	if (m_bEffectsLoaded == TRUE)
		return TRUE;
	
	m_bEffectsLoaded =TRUE;
	FMeshInit_t meshInit;
	FMesh_t		*pMesh;
	
	FResFrame_t frame = fres_GetFrame();
	
	m_aSplashes = (_ShieldSplash_t*)fres_AlignedAlloc( _MAX_SPLASHES * sizeof( _ShieldSplash_t ), 16 );
	if( m_aSplashes == NULL ) {
		DEVPRINTF( "CEShield::InitSystem(): Unable to allocate array of splashes\n" );
		goto _ExitInitEffectsWithError;
	}

	flinklist_InitRoot( &m_FreeSplashList, FANG_OFFSETOF( _ShieldSplash_t, link ) );

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) {
		m_aSplashes[i].fUnitTime	= 1.0f;
		m_aSplashes[i].pShield		= NULL;
		m_aSplashes[i].pMesh		= NULL;
		m_aSplashes[i].vDisplacement.Zero();

		flinklist_AddTail( &m_FreeSplashList, &m_aSplashes[i] );
	}


	//init all the splashes
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _SPLASH_MESH );
	if( pMesh == NULL ) {
		DEVPRINTF( "CEShield::InitSystem():  Unable to load mesh %s\n", _SPLASH_MESH );
		goto _ExitInitEffectsWithError;
	}

	
	meshInit.fCullDist	= 400.f;
	meshInit.nFlags		= 0;
	meshInit.pMesh		= pMesh;
	meshInit.Mtx.Identity();

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) {
		m_aSplashes[i].fUnitTime	= 1.0f;
		m_aSplashes[i].pShield		= NULL;
		m_aSplashes[i].vDisplacement.Zero();
		m_aSplashes[i].pMesh = fnew CFWorldMesh;
		m_aSplashes[i].pMesh->Init( &meshInit );
		m_aSplashes[i].pMesh->SetCollisionFlag( FALSE );
		m_aSplashes[i].pMesh->m_nUser = NULL;
		m_aSplashes[i].pMesh->m_pUser = NULL;
		m_aSplashes[i].pMesh->SetUserTypeBits( 0 );
		m_aSplashes[i].pMesh->UpdateTracker();

		m_aSplashes[i].hTexLayer = m_aSplashes[i].pMesh->GetTexLayerHandle( _SPLASH_TEXLAYER_ID );
		if( m_aSplashes[i].hTexLayer < 0 ) {
			DEVPRINTF( "CEShield::_InitEffects():  Error getting tex layer handle\n" );
			goto _ExitInitEffectsWithError;
		}

		
		m_aSplashes[i].pMesh->SetCullDirection( FMESH_CULLDIR_NONE );
		m_aSplashes[i].pMesh->AnimTC_AnimateScroll( m_aSplashes[i].hTexLayer, FALSE );

		m_aSplashes[i].pMesh->RemoveFromWorld();
	}

	return TRUE;

	// Error:
_ExitInitEffectsWithError:
	fres_ReleaseFrame( frame );
	return FALSE;
}

void CEProj_Slower::DestroyEffects( void )
{
	FASSERT( m_bSystemInitialized );
	if( !m_bEffectsLoaded ) 
	{
		return;
	}

	m_bEffectsLoaded = FALSE;

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) 
	{
		fdelete( m_aSplashes[i].pMesh );
		m_aSplashes[i].pMesh = NULL;
	}
}


// returns the next available splash, off the list of free splashes if possible		
_ShieldSplash_t* CEProj_Slower::_GetNextSplash( void )
{
	FASSERT( m_bSystemInitialized );
	_ShieldSplash_t *pSplash;

	pSplash = (_ShieldSplash_t*)flinklist_RemoveHead( &m_FreeSplashList );
	if( pSplash != NULL ) 
	{
		return pSplash;
	}

	f32 fHighestTime = 0.0f;
	u32 uHighestIdx	 = 0;

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) 
	{
		if( m_aSplashes[i].fUnitTime > fHighestTime ) 
		{
			fHighestTime = m_aSplashes[i].fUnitTime;
			uHighestIdx  = i;
		}
	}

	m_aSplashes[uHighestIdx].pShield->_KillSplash( m_aSplashes[uHighestIdx].fUnitTime );		// will kill one and put it on the free list
	pSplash = (_ShieldSplash_t*)flinklist_RemoveHead( &m_FreeSplashList );						// get it and return it

	return pSplash;

}

void CEProj_Slower::_AddSplashEffect(CEProj *pProj, const CFVec3A &rvPos, f32 fMagnitude)
{
	FASSERT( m_bSystemInitialized );

	_ShieldSplash_t *pSplash = _GetNextSplash();
	
	pSplash->fUnitTime		= 0.0f;
	pSplash->pShield		= this;
	pSplash->vDisplacement.Sub( rvPos, pProj->m_pWorldMesh->GetBoundingSphere().m_Pos );
	//let's try this instead:
	if( pSplash->vDisplacement.MagSq() < 0.01f ) {
		// not a lot we can do, just bail, put the splash back first
		flinklist_AddTail( &m_FreeSplashList, pSplash );
		return;
	}

	pSplash->vDisplacement.Unitize();
	pSplash->vDisplacement.Mul( pProj->m_pWorldMesh->GetBoundingSphere().m_fRadius );
	
	_UpdateSplashPosition( pProj->m_pWorldMesh->GetBoundingSphere().m_Pos, pSplash );

	pSplash->pMesh->m_Xfm.BuildScale( pProj->m_fScaleToWorld );
	pSplash->pMesh->AddToWorld();
	pSplash->pMesh->SetMeshAlpha( _SPLASH_INITIAL_ALPHA );

//	m_fShieldAlpha = _ALPHA_SHIELD_HIT;

	flinklist_AddTail( &m_SplashList, pSplash );
}

// Release the (first if !bReleaseAll) splash in the list that is at least as old as fTime
void CEProj_Slower::_KillSplash( f32 fTime, BOOL bReleaseAll)
{
	FASSERT( m_bSystemInitialized );

	_ShieldSplash_t *pSplash = (_ShieldSplash_t*)flinklist_GetHead( &m_SplashList );

	while( pSplash != NULL ) 
	{
		if( pSplash->fUnitTime >= fTime ) 
		{
			pSplash = _ReleaseSplash( pSplash );
			if( !bReleaseAll ) 
			{
				return;
			}
		} 
		else 
		{
			pSplash = (_ShieldSplash_t*)flinklist_GetNext( &m_SplashList, pSplash );
		}
	}
}

void CEProj_Slower::_UpdateSplashPosition( const CFVec3A &vShieldPos, _ShieldSplash_t *pSplash )
{
	CFMtx43A mtxTemp;
	mtxTemp.Identity();

	CFVec3A vSplashPosWS;
	vSplashPosWS.Add( vShieldPos, pSplash->vDisplacement );

	mtxTemp.m_vFront.Set(pSplash->vDisplacement);
	mtxTemp.m_vFront.Unitize();

	mtxTemp.m_vRight.CrossYWithVec( mtxTemp.m_vFront);

	if( mtxTemp.m_vRight.MagSq() > 0.0001f ) 
	{
		mtxTemp.m_vRight.Unitize();
	}
	else 
	{
		mtxTemp.m_vRight = CFVec3A::m_UnitAxisX;
	}

	mtxTemp.m_vUp.Cross( mtxTemp.m_vFront, mtxTemp.m_vRight );

	mtxTemp.m_vPos.Add(vShieldPos,pSplash->vDisplacement);

	pSplash->pMesh->m_Xfm.BuildFromMtx( mtxTemp );
	pSplash->pMesh->UpdateTracker();

	//Update alpha
	CFVec2	vTmp;
	f32		fUnitTime;

	fUnitTime = pSplash->fUnitTime;
	FMATH_CLAMPMAX( fUnitTime, 1.0f );
	pSplash->pMesh->SetMeshAlpha( FMATH_FPOT( fUnitTime, 0.5f, 0.0f ) );
	fUnitTime = fmath_Sqrt( fUnitTime );
    vTmp.Set( FMATH_FPOT( fUnitTime, 1.0f, 0.75f ), 0.0f );
	pSplash->pMesh->AnimTC_SetScrollST( pSplash->hTexLayer, vTmp );
}

// turns it off, removes from our list and adds back to free list.  Returns next in our work list or NULL
_ShieldSplash_t* CEProj_Slower::_ReleaseSplash( _ShieldSplash_t *pSplash )
{
	FASSERT( pSplash != NULL );

	_ShieldSplash_t *pRetSplash = (_ShieldSplash_t*)flinklist_GetNext( &m_SplashList, pSplash );

	if( pSplash->pMesh ) 
	{
		pSplash->pMesh->RemoveFromWorld();
	}

	flinklist_Remove( &m_SplashList, pSplash );
	flinklist_AddTail( &m_FreeSplashList, pSplash );

	return pRetSplash;
}