//////////////////////////////////////////////////////////////////////////////////////
// FDebris.cpp - Debris system.
//
// Author: Steve Ranck   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 12/04/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdata.h"
#include "fdebris.h"
#include "floop.h"
#include "fworld_coll.h"
#include "fmeshpool.h"
#include "fstringtable.h"
#include "fclib.h"
#include "fsound.h"


#define _TIME_BETWEEN_IMPACT_SOUNDS		0.1f
#define _LIGHT_FRAMES_PER_SAMPLE		5



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebris
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CFDebris::m_bSystemInitialized;
FLinkRoot_t CFDebris::m_RootFree;
FLinkRoot_t CFDebris::m_RootActive;
u32 CFDebris::m_nMaxPoolElementCount;
CFDebris *CFDebris::m_pDebrisPool;
CFDebris **CFDebris::m_ppWorkList;
CFMeshPool *CFDebris::m_pMeshPool;
u32 CFDebris::m_nSampleLightInstance;
f32 CFDebris::m_fImpactSoundTimer;


BOOL CFDebris::InitDebrisSystem( u32 nMaxDebrisCount ) {
	FASSERT( !IsDebrisSystemInitialized() );

	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	#if !FANG_PRODUCTION_BUILD
		u32 nFreeBytes = fres_GetFreeBytes();
	#endif

	m_bSystemInitialized = TRUE;

	flinklist_InitRoot( &m_RootFree, FANG_OFFSETOF( CFDebris, m_Link ) );
	flinklist_InitRoot( &m_RootActive, FANG_OFFSETOF( CFDebris, m_Link ) );

	if( nMaxDebrisCount == 0 ) {
		goto _ExitSuccessfully;
	}

	// Create mesh pool...
	m_pMeshPool = CFMeshPool::CreatePool( nMaxDebrisCount );
	if( m_pMeshPool == NULL ) {
		DEVPRINTF( "CFDebris::InitDebrisSystem(): Not enough memory to allocate mesh pool.\n" );
		DEVPRINTF( "                              No debris will be generated.\n" );
		goto _ExitWithError;
	}

	// Allocate our work list...
	m_ppWorkList = (CFDebris **)fres_Alloc( sizeof(CFDebris *) * nMaxDebrisCount );
	if( m_ppWorkList == NULL ) {
		DEVPRINTF( "CFDebris::InitDebrisSystem(): Not enough memory to allocate debris work list.\n" );
		DEVPRINTF( "                              No debris will be generated.\n" );
		goto _ExitWithError;
	}

	// Allocate our debris pool...
	m_pDebrisPool = fnew CFDebris[ nMaxDebrisCount ];
	if( m_pDebrisPool == NULL ) {
		DEVPRINTF( "CFDebris::InitDebrisSystem(): Not enough memory to allocate debris pool.\n" );
		DEVPRINTF( "                              No debris will be generated.\n" );
		goto _ExitWithError;
	}

	flinklist_InitPool( &m_RootFree, m_pDebrisPool, sizeof(CFDebris), nMaxDebrisCount );

	for( i=0; i<nMaxDebrisCount; ++i ) {
		m_pDebrisPool[i].m_nListMember = LIST_MEMBER_FREE_POOL;
	}

	m_nMaxPoolElementCount = nMaxDebrisCount;
	m_nSampleLightInstance = 0;

	m_fImpactSoundTimer = 0.0f;
	
_ExitSuccessfully:

	#if !FANG_PRODUCTION_BUILD
		DEVPRINTF( "Debris System used %u bytes of memory for %u debris objects.\n", nFreeBytes - fres_GetFreeBytes(), nMaxDebrisCount );
	#endif

	// Success...
	return TRUE;

_ExitWithError:
	UninitDebrisSystem();
	fres_ReleaseFrame( ResFrame );
	m_bSystemInitialized = FALSE;

	return FALSE;
}


void CFDebris::UninitDebrisSystem( void ) {
	if( IsDebrisSystemInitialized() ) {
		CFMeshPool::DestroyPool( m_pMeshPool );
		fdelete_array( m_pDebrisPool );

		m_pMeshPool = NULL;
		m_pDebrisPool = NULL;
		m_ppWorkList = NULL;

		flinklist_InitRoot( &m_RootFree, FANG_OFFSETOF( CFDebris, m_Link ) );
		flinklist_InitRoot( &m_RootActive, FANG_OFFSETOF( CFDebris, m_Link ) );

		m_bSystemInitialized = FALSE;
	}
}


// Note that *pDebrisDef does not need to persist beyond this call. The
// data is copied into the debris internals.
CFDebris *CFDebris::Spawn( const CFDebrisDef *pDebrisDef, const CFMtx43A *pInitialMtx ) {
	FASSERT( IsDebrisSystemInitialized() );

	#if !FANG_PRODUCTION_BUILD
		if( pDebrisDef->m_nBoneIndex >= 0 ) {
			if( !(pDebrisDef->m_nFlags & CFDebrisDef::FLAG_USE_PROVIDED_WORLDMESH) ) {
				DEVPRINTF( "CFDebris::Spawn(): FLAG_USE_PROVIDED_WORLDMESH must be specified when m_nBoneIndex is provided.\n" );
				return NULL;
			}
		}
	#endif

	if( m_pDebrisPool == NULL ) {
		return NULL;
	}

	CFDebris *pDebris;

	// Get debris object from free list...
	pDebris = _GetFreeDebris( pDebrisDef );
	if( pDebris == NULL ) {
		return NULL;
	}

	// Add to active list...
	flinklist_AddTail( &m_RootActive, pDebris );
	pDebris->m_nListMember = LIST_MEMBER_ACTIVE_POOL;

	fang_MemCopy( &pDebris->m_DebrisDef, pDebrisDef, sizeof(CFDebrisDef) );

	if( pInitialMtx ) {
		pDebris->m_DebrisDef.m_Pos_WS = pInitialMtx->m_vPos;
	}

	if( pDebris->m_DebrisDef.m_pFcnCallback == NULL ) {
		FMATH_CLEARBITMASK( pDebris->m_DebrisDef.m_nFlags, CFDebrisDef::FLAG_BUILD_TRACKER_SKIP_LIST );
	}

	pDebris->m_hSmokeTrail = SMOKETRAIL_NULLHANDLE;

	if( pDebris->m_DebrisDef.m_pSmokeTrailAttrib ) {
		// Create a smoketrail...
		pDebris->m_hSmokeTrail = smoketrail_GetFromFreePoolAndSetAttributes( pDebris->m_DebrisDef.m_pSmokeTrailAttrib, FALSE );
		smoketrail_Puff( pDebris->m_hSmokeTrail, &pDebris->m_DebrisDef.m_Pos_WS );
	}

	pDebris->m_pAudioEmitterFlaming = NULL;

	pDebris->m_hParticle1 = FPARTICLE_INVALID_HANDLE;
	pDebris->m_hParticle2 = FPARTICLE_INVALID_HANDLE;

	if( pDebris->m_DebrisDef.m_nBoneIndex >= 0 ) {
		// Debris that drives a bone...
		pDebris->_SpawnBoned();
		return pDebris;
	}

	switch( pDebris->m_DebrisDef.m_nFadeType ) {
	case CFDebrisDef::FADE_TYPE_ALPHA:
		FMATH_CLAMPMIN( pDebris->m_DebrisDef.m_fFadeSecs, 0.001f );
		pDebris->m_fFadeFactor = fmath_Div( pDebris->m_DebrisDef.m_fMeshUnitOpacity, pDebris->m_DebrisDef.m_fFadeSecs );
		break;

	case CFDebrisDef::FADE_TYPE_SHRINK:
		FMATH_CLAMPMIN( pDebris->m_DebrisDef.m_fFadeSecs, 0.001f );
		FMATH_CLAMPMIN( pDebris->m_DebrisDef.m_fMeshScale, 0.001f );
		pDebris->m_fFadeFactor = fmath_Div( pDebris->m_DebrisDef.m_fMeshScale, pDebris->m_DebrisDef.m_fFadeSecs );
		break;

	default:
		pDebris->m_fFadeFactor = 0.0f;
	};

	CFWorldMesh *pWorldMesh;
	FMeshInit_t MeshInit;

	// Fill in a few fields in the MeshInit structure...
	if( pInitialMtx == NULL ) {
		if( pDebris->m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_RANDOM_ORIENTATION ) {
			CFQuatA Quat;

			Quat.Set( fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );

			if( Quat.v.MagSq() >= 0.00001f ) {
				Quat.Unitize();
				Quat.BuildMtx( MeshInit.Mtx );
			} else {
				MeshInit.Mtx.Identity33();
			}
		} else {
			MeshInit.Mtx.Identity33();
		}

		MeshInit.Mtx.m_vPos = pDebris->m_DebrisDef.m_Pos_WS;
	} else {
		MeshInit.Mtx = *pInitialMtx;
	}

	MeshInit.nFlags = FMESHINST_FLAG_NOCOLLIDE;

	switch( pDebris->m_DebrisDef.m_nLightingType ) {
	case CFDebrisDef::LIGHTING_TYPE_NONE:
		FMATH_SETBITMASK( MeshInit.nFlags, FMESHINST_FLAG_NOLIGHT );
		break;

	case CFDebrisDef::LIGHTING_TYPE_AMBIENT_ONLY:
		FMATH_SETBITMASK( MeshInit.nFlags, FMESHINST_FLAG_NOLIGHT_DYNAMIC );
		break;
	};

	// Init world mesh...
	if( pDebris->m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_USE_PROVIDED_WORLDMESH ) {
		// Use provided world mesh...

		pWorldMesh = pDebris->m_DebrisDef.m_pWorldMesh;
		pDebris->m_DebrisDef.m_pMesh = pWorldMesh->m_pMesh;

		pWorldMesh->m_fCullDist = pDebris->m_DebrisDef.m_fCullDist;

		if( pDebris->m_DebrisDef.m_fMeshScale == 1.0f ) {
			pWorldMesh->m_Xfm.BuildFromMtx( MeshInit.Mtx, FALSE );
		} else {
			pWorldMesh->m_Xfm.BuildFromMtx( MeshInit.Mtx, pDebris->m_DebrisDef.m_fMeshScale );
		}

		pWorldMesh->m_nFlags = MeshInit.nFlags;
	} else {
		// Use provided mesh...

		pWorldMesh = pDebris->m_pMeshPool->GetFromFreePool();
		FASSERT( pWorldMesh );

		pDebris->m_DebrisDef.m_pWorldMesh = pWorldMesh;

		MeshInit.pMesh = pDebris->m_DebrisDef.m_pMesh;
		MeshInit.fCullDist = pDebris->m_DebrisDef.m_fCullDist;

		pWorldMesh->Init( &MeshInit, TRUE, -1, -1, TRUE );

		if( !(pDebris->m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_OVERRIDE_MESH_TINT) ) {
			// No mesh tinting...
			pWorldMesh->SetMeshTint( 1.0f, 1.0f, 1.0f );
		} else {
			// Apply mesh tinting...

			pWorldMesh->SetMeshTint(
				(f32)pDebris->m_DebrisDef.m_nTintRed * (1.0f / 255.0f),
				(f32)pDebris->m_DebrisDef.m_nTintGreen * (1.0f / 255.0f),
				(f32)pDebris->m_DebrisDef.m_nTintBlue * (1.0f / 255.0f)
			);
		}
	}

	pWorldMesh->SetLineOfSightFlag( FALSE );
	pWorldMesh->SetCollisionFlag( FALSE );

	pDebris->m_Quat.BuildQuat( MeshInit.Mtx );

	pDebris->m_nLifeState = LIFE_STATE_NORMAL;
	pDebris->m_fCountdownSecs = pDebris->m_DebrisDef.m_fAliveSecs;
	pDebris->m_fMovingSlowlyCountdownSecs = pDebris->m_DebrisDef.m_fSecsUnderMinSpeedToStartFade;

	// Set initial opacity of mesh...
	pWorldMesh->SetMeshAlpha( pDebris->m_DebrisDef.m_fMeshUnitOpacity );

	if( pDebris->m_DebrisDef.m_nLightingType != CFDebrisDef::LIGHTING_TYPE_NONE ) {
		pDebris->m_nSampleLightCountdown = (++m_nSampleLightInstance) & 3;
		fworld_LightPoint( &pDebris->m_DebrisDef.m_Pos_WS, &pDebris->m_DebrisDef.m_pWorldMesh->m_AmbientRGB, FALSE, pDebris->m_DebrisDef.m_pWorldMesh );
	}

	pDebris->m_nFlags = FLAG_NONE;

	// Is this a new lowest priority?	
	if ( pDebris->m_DebrisDef.m_nPriority < CFDebrisSpawner::m_nLowestDebrisWeight )
	{
		CFDebrisSpawner::m_nLowestDebrisWeight = pDebris->m_DebrisDef.m_nPriority;
	}

	// Add it to the world...
	pWorldMesh->UpdateTracker();

	return pDebris;
}


void CFDebris::_SpawnBoned( void ) {
	CFMtx43A InitialBoneUnitMtx;

	FMATH_CLEARBITMASK( m_DebrisDef.m_nFlags, (CFDebrisDef::FLAG_FLAT_OBJECT | CFDebrisDef::FLAG_RANDOM_ORIENTATION) );

	InitialBoneUnitMtx = *m_DebrisDef.m_pWorldMesh->GetBoneMtxPalette()[ m_DebrisDef.m_nBoneIndex ];
	InitialBoneUnitMtx.Mul33( InitialBoneUnitMtx.m_vX.InvMag() );

	m_DebrisDef.m_fMeshScale = 1.0f;
	m_DebrisDef.m_Pos_WS = InitialBoneUnitMtx.m_vPos;
	m_DebrisDef.m_pMesh = m_DebrisDef.m_pWorldMesh->m_pMesh;

	if( m_DebrisDef.m_nFadeType == CFDebrisDef::FADE_TYPE_ALPHA ) {
		m_DebrisDef.m_nFadeType = CFDebrisDef::FADE_TYPE_SHRINK;
	}

	if( m_DebrisDef.m_nFadeType == CFDebrisDef::FADE_TYPE_SHRINK ) {
		FMATH_CLAMPMIN( m_DebrisDef.m_fFadeSecs, 0.001f );
		FMATH_CLAMPMIN( m_DebrisDef.m_fMeshScale, 0.001f );
		m_fFadeFactor = fmath_Div( m_DebrisDef.m_fMeshScale, m_DebrisDef.m_fFadeSecs );
	} else {
		m_fFadeFactor = 0.0f;
	}

	m_Quat.BuildQuat( InitialBoneUnitMtx );
	m_DebrisDef.m_nLightingType = CFDebrisDef::LIGHTING_TYPE_NONE;
	m_nLifeState = LIFE_STATE_NORMAL;
	m_fCountdownSecs = m_DebrisDef.m_fAliveSecs;
	m_fMovingSlowlyCountdownSecs = m_DebrisDef.m_fSecsUnderMinSpeedToStartFade;
	m_nFlags = FLAG_NONE;
}


CFDebris *CFDebris::_GetFreeDebris( const CFDebrisDef *pDebrisDef ) {
	FASSERT( IsDebrisSystemInitialized() );

	CFDebris *pDebris;

	pDebris = (CFDebris *)flinklist_RemoveTail( &m_RootFree );
	if( pDebris ) {
		return pDebris;
	}

	// We're out of free debris objects...
	
	if ( CFDebrisSpawner::m_nLowestDebrisWeight != 0xffffffff && pDebrisDef->m_nPriority < CFDebrisSpawner::m_nLowestDebrisWeight )	{
		// There is no debris of lower priority
		return NULL;
	}

	if( pDebrisDef->m_nPriority == 0 ) {
		// Lowest priority...

		if( fmath_RandomChance( 0.5f ) ) {
			return NULL;
		}
	}

	u32 nBestCandidateWeight, nWeight;
	CFDebris *pBestCandidateDebris;

	nBestCandidateWeight = 0xffffffff;
	pBestCandidateDebris = NULL;
	
	for( pDebris=(CFDebris *)flinklist_GetHead( &m_RootActive ); pDebris; pDebris=(CFDebris *)flinklist_GetNext( &m_RootActive, pDebris ) ) {
		if( pDebris->m_DebrisDef.m_nPriority > pDebrisDef->m_nPriority ) {
			continue;
		}

		nWeight = !(pDebris->m_nFlags & FLAG_FINAL_REST_POS);
		nWeight |= (pDebris->m_DebrisDef.m_pWorldMesh->HaveDrawn() << 1);
		nWeight |= pDebris->m_DebrisDef.m_nPriority;

		if( nWeight < nBestCandidateWeight ) {
			nBestCandidateWeight = nWeight;
			pBestCandidateDebris = pDebris;

			if( nWeight == 0 ) {
				break;
			}
		}
	}

	if( pBestCandidateDebris ) {
		pBestCandidateDebris->Kill( TRUE );

		if( pBestCandidateDebris->m_nListMember == LIST_MEMBER_FREE_POOL ) {
			pDebris = (CFDebris *)flinklist_RemoveTail( &m_RootFree );
			FASSERT( pDebris );

			return pDebris;
		}
	}

	return NULL;
}


void CFDebris::Kill( BOOL bCallTheCallback ) {
	if( m_nListMember == LIST_MEMBER_FREE_POOL ) {
		return;
	}

	FASSERT( m_nListMember == LIST_MEMBER_ACTIVE_POOL );

	if( m_nFlags & FLAG_IN_CALLBACK ) {
		// Kill is being called from within the callback...
		FMATH_SETBITMASK( m_nFlags, FLAG_CALLBACK_KILLED_US );
		return;
	}

	if( bCallTheCallback ) {
		if( m_DebrisDef.m_pFcnCallback ) {
			if( _CallCallback( CFDebrisDef::CALLBACK_REASON_DEAD, NULL ) ) {
				return;
			}
		}
	}

	_FinishKill();
}


void CFDebris::_FinishKill( void ) {
	if( m_pAudioEmitterFlaming ) {
		m_pAudioEmitterFlaming->Destroy();
		m_pAudioEmitterFlaming = NULL;
	}

	if( m_hSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
		smoketrail_Puff( m_hSmokeTrail, &m_DebrisDef.m_Pos_WS );
		smoketrail_ReturnToFreePool( m_hSmokeTrail, TRUE );
		m_hSmokeTrail = SMOKETRAIL_NULLHANDLE;
	}

	if( m_hParticle1 != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hParticle1 );
		m_hParticle1 = FPARTICLE_INVALID_HANDLE;
	}

	if( m_hParticle2 != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hParticle2 );
		m_hParticle2 = FPARTICLE_INVALID_HANDLE;
	}

	if( m_DebrisDef.m_nBoneIndex < 0 ) {
		m_DebrisDef.m_pWorldMesh->RemoveFromWorld();
	}

	if( !(m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_USE_PROVIDED_WORLDMESH) ) {
		m_pMeshPool->ReturnToFreePool( (CFWorldMeshItem *)m_DebrisDef.m_pWorldMesh );
	}

	flinklist_Remove( &m_RootActive, this );
	flinklist_AddTail( &m_RootFree, this );
	m_nListMember = LIST_MEMBER_FREE_POOL;
}


// Returns TRUE if the callback killed us.
BOOL CFDebris::_CallCallback( CFDebrisDef::CallbackReason_e nReason, FCollImpact_t *pCollImpact ) {
	FMATH_SETBITMASK( m_nFlags, FLAG_IN_CALLBACK );

	m_DebrisDef.m_pFcnCallback( this, nReason, pCollImpact );

	FMATH_CLEARBITMASK( m_nFlags, FLAG_IN_CALLBACK );

	if( !(m_nFlags & FLAG_CALLBACK_KILLED_US) ) {
		// The callback didn't kill us...
		return FALSE;
	}

	// The callback killed us. Finish Kill() function...
	_FinishKill();

	return TRUE;
}


void CFDebris::KillAll( BOOL bCallTheCallbacks ) {
	CFDebris *pDebris;

	while( pDebris = (CFDebris *)flinklist_GetTail( &m_RootActive ) ) {
		pDebris->Kill( bCallTheCallbacks );
		FASSERT( pDebris->m_nListMember != CFDebris::LIST_MEMBER_ACTIVE_POOL );
	}
}


// Kills all debris objects whose pFcnFilterCallback function returns TRUE for.
void CFDebris::KillAllSelective( u8 nOwnerID, KillSelectiveCallback *pFcnFilterCallback, BOOL bCallTheCallbacks ) {
	CFDebris *pDebris, *pNextDebris;

	pDebris = (CFDebris *)flinklist_GetHead( &m_RootActive );

	while( pDebris ) {
		pNextDebris = (CFDebris *)flinklist_GetNext( &m_RootActive, pDebris );

		if( pDebris->m_DebrisDef.m_nOwnerID == nOwnerID ) {
			if( !pFcnFilterCallback || pFcnFilterCallback( pDebris ) ) {
				pDebris->Kill( bCallTheCallbacks );
				FASSERT( pDebris->m_nListMember != CFDebris::LIST_MEMBER_ACTIVE_POOL );
			}
		}

		pDebris = pNextDebris;
	}
}


void CFDebris::Work( void ) {
	FASSERT( IsDebrisSystemInitialized() );
	FASSERT( m_RootActive.nCount <= m_nMaxPoolElementCount );

	CFDebrisSpawner::Work();

	if( m_RootActive.nCount == 0 ) {
		return;
	}

#if 0
	if( m_fImpactSoundTimer > 0.0f ) {
		m_fImpactSoundTimer -= FLoop_fPreviousLoopSecs;

		if( m_fImpactSoundTimer <= 0.0f ) {
			m_fImpactSoundTimer = 0.0f;
		}
	}
#endif

	CFDebris *pDebris, **ppDebris;
	u32 i, nActiveCount;

	// Build our work list...
	ppDebris = m_ppWorkList;
	nActiveCount = m_RootActive.nCount;

	for( pDebris=(CFDebris *)flinklist_GetHead( &m_RootActive ); pDebris; pDebris=(CFDebris *)flinklist_GetNext( &m_RootActive, pDebris ) ) {
		*ppDebris++ = pDebris;
	}

	// Now, do work for each debris object...
	for( i=0, ppDebris=m_ppWorkList; i<nActiveCount; ++i, ++ppDebris ) {
		pDebris = *ppDebris;

		if( pDebris->m_nListMember == LIST_MEMBER_ACTIVE_POOL ) {
			// Still alive...
			pDebris->_Work();
		}
	}
}


// Returns TRUE if the object was killed.
BOOL CFDebris::StartFading( BOOL bCallTheCallback ) {
	if( m_nLifeState == LIFE_STATE_FADING_OUT ) {
		// Already fading out...
		return FALSE;
	}

	if( m_DebrisDef.m_nFadeType == CFDebrisDef::FADE_TYPE_POP_OFF ) {
		Kill( bCallTheCallback );
		return TRUE;
	}

	m_nLifeState = LIFE_STATE_FADING_OUT;
	m_fCountdownSecs = m_DebrisDef.m_fFadeSecs;

	if( bCallTheCallback ) {
		if( m_DebrisDef.m_pFcnCallback ) {
			if( _CallCallback( CFDebrisDef::CALLBACK_REASON_START_FADE, NULL ) ) {
				// The callback killed us...
				return TRUE;
			}
		}
	}

	return FALSE;
}


void CFDebris::_Work( void ) {
	f32 fTimeStepSecs, fUnitIntensity;

	if( m_hSmokeTrail ) {
		// Create a smoketrail...
		smoketrail_Puff( m_hSmokeTrail, &m_DebrisDef.m_Pos_WS );
	}

	// Compute time step...
	fTimeStepSecs = FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMAX( fTimeStepSecs, (1.0f / 30.0f) );

	// Update auto-kill-when-stopped counter...
	if( m_DebrisDef.m_fSecsBeforeMinSpeedValid > 0.0f ) {
		m_DebrisDef.m_fSecsBeforeMinSpeedValid -= fTimeStepSecs;
	}

	// Update life state...
	switch( m_nLifeState ) {
	case LIFE_STATE_NORMAL:
		m_fCountdownSecs -= fTimeStepSecs;

		if( m_fCountdownSecs <= 0.0f ) {
			if( StartFading( TRUE ) ) {
				// We have been killed...
				return;
			}
		}

		break;

	case LIFE_STATE_FADING_OUT:
		m_fCountdownSecs -= fTimeStepSecs;

		if( m_fCountdownSecs <= 0.0f ) {
			Kill( TRUE );
			return;
		}

		// Update fade effect...
		switch( m_DebrisDef.m_nFadeType ) {
		case CFDebrisDef::FADE_TYPE_SHRINK:
			m_DebrisDef.m_fMeshScale = m_fCountdownSecs * m_fFadeFactor;
			FMATH_CLAMPMIN( m_DebrisDef.m_fMeshScale, 0.001f );

			if( m_hParticle1 != FPARTICLE_INVALID_HANDLE ) {
				fUnitIntensity = m_DebrisDef.m_fMeshScale * 0.5f;
				FMATH_CLAMPMAX( fUnitIntensity, 1.0f );

				if( m_pAudioEmitterFlaming ) {
					m_pAudioEmitterFlaming->SetVolume( fUnitIntensity );
					m_pAudioEmitterFlaming->SetPosition( &m_DebrisDef.m_Pos_WS );
				}

				fparticle_SetIntensity( m_hParticle1, fUnitIntensity );
			}

			if( m_hParticle2 != FPARTICLE_INVALID_HANDLE ) {
				fUnitIntensity = m_DebrisDef.m_fMeshScale * 0.25f;
				FMATH_CLAMPMAX( fUnitIntensity, 1.0f );

				fparticle_SetIntensity( m_hParticle2, fUnitIntensity );
			}

			break;

		case CFDebrisDef::FADE_TYPE_ALPHA:
			m_DebrisDef.m_fMeshUnitOpacity = m_fCountdownSecs * m_fFadeFactor;
			FMATH_CLAMPMAX( m_DebrisDef.m_fMeshUnitOpacity, 1.0f );
			m_DebrisDef.m_pWorldMesh->SetMeshAlpha( m_DebrisDef.m_fMeshUnitOpacity );

			break;
		};

		break;

	default:
		FASSERT_NOW;
	};

	BOOL bParticleKilled = FALSE;
	
	// Update lowest debris weight tracker
	if ( m_DebrisDef.m_nPriority < CFDebrisSpawner::m_nLowestDebrisWeight )
	{
		CFDebrisSpawner::m_nLowestDebrisWeight = m_DebrisDef.m_nPriority;
	}

	// Update linear motion...
	switch( m_DebrisDef.m_nCollType ) {
	case CFDebrisDef::COLL_TYPE_NONE:
		bParticleKilled = _MoveLinear_NoColl( fTimeStepSecs );
		break;

	case CFDebrisDef::COLL_TYPE_RAY:
		bParticleKilled = _MoveLinear_RayColl( fTimeStepSecs );
		break;
	};

	if( bParticleKilled ) {
		// We have been killed...
		return;
	}

	CFMtx43A Mtx;

	// Update rotational motion...
	if( m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_ROTATIONAL_MOTION ) {
		if( !(m_nFlags & FLAG_FLATTEN_MODE) ) {
			if( m_DebrisDef.m_fRotSpeed != 0.0f ) {
				CFQuatA OrigQuat, RotateQuat;
				f32 fHalfAngle, fSin, fCos;

				OrigQuat = m_Quat;

				fHalfAngle = m_DebrisDef.m_fRotSpeed * fTimeStepSecs;
				fmath_SinCos( fHalfAngle, &fSin, &fCos );

				RotateQuat.x = m_DebrisDef.m_UnitRotAxis_WS.x * fSin;
				RotateQuat.y = m_DebrisDef.m_UnitRotAxis_WS.y * fSin;
				RotateQuat.z = m_DebrisDef.m_UnitRotAxis_WS.z * fSin;
				RotateQuat.w = fCos;

				m_Quat.Mul( RotateQuat, OrigQuat );
			}

			m_Quat.BuildMtx( Mtx );
		} else {
			CFQuatA OrigQuat, RotateQuat;
			CFVec3A NewRotAxis;
			f32 fDot;

			RotateQuat.BuildQuat( m_FlattenModeUnitRotAxis, m_DebrisDef.m_fRotSpeed * FLoop_fPreviousLoopSecs );

			OrigQuat = m_Quat;
			m_Quat.Mul( RotateQuat, OrigQuat );
			m_Quat.BuildMtx( Mtx );

			NewRotAxis.Cross( Mtx.m_vY, m_FlattenModeUnitFaceNorm );

			fDot = NewRotAxis.Dot( m_FlattenModeUnitRotAxis );

			if( fDot < 0.0f ) {
				// We stepped too far...

				f32 fMagSq = m_DebrisDef.m_pWorldMesh->m_Xfm.m_MtxF.m_vY.MagSq();
				if( fMagSq > 0.00001f ) {
					CFVec3A CurrentUnitAxisY;
					CurrentUnitAxisY.Mul( m_DebrisDef.m_pWorldMesh->m_Xfm.m_MtxF.m_vY, fmath_InvSqrt(fMagSq) );

					RotateQuat.BuildQuat( CurrentUnitAxisY, m_FlattenModeUnitFaceNorm );
					m_Quat.Mul( RotateQuat, OrigQuat );
					m_Quat.BuildMtx( Mtx );
				}

				FMATH_CLEARBITMASK( m_nFlags, FLAG_FLATTEN_MODE );
				m_DebrisDef.m_fRotSpeed = 0.0f;
			}
		}
	} else {
		m_Quat.BuildMtx( Mtx );
	}

	// Update xfm...
	Mtx.m_vPos = m_DebrisDef.m_Pos_WS;

	if( m_DebrisDef.m_nBoneIndex < 0 ) {
		// Non-boned debris...

		if( m_DebrisDef.m_nCollType == CFDebrisDef::COLL_TYPE_RAY ) {
			if( !(m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_FLAT_OBJECT) ) {
				Mtx.m_vPos.y += m_DebrisDef.m_pWorldMesh->GetBoundingSphere().m_fRadius;
			} else {
				f32 fUnitDisplacementY = 1.0f - (fmath_Abs(m_DebrisDef.m_pWorldMesh->m_Xfm.m_MtxF.m_vY.y) * m_DebrisDef.m_pWorldMesh->m_Xfm.m_fScaleR);

				Mtx.m_vPos.y += m_DebrisDef.m_pWorldMesh->GetBoundingSphere().m_fRadius * fUnitDisplacementY;
			}
		}

		if( m_DebrisDef.m_fMeshScale == 1.0f ) {
			m_DebrisDef.m_pWorldMesh->m_Xfm.BuildFromMtx( Mtx, FALSE );
		} else {
			m_DebrisDef.m_pWorldMesh->m_Xfm.BuildFromMtx( Mtx, m_DebrisDef.m_fMeshScale );
		}

		m_DebrisDef.m_pWorldMesh->UpdateTracker( (m_DebrisDef.m_LinVel_WS.MagSq() > 0.0001f) );
	} else {
		// Boned debris...

		CFMtx43A *pBoneMtx = m_DebrisDef.m_pWorldMesh->GetBoneMtxPalette()[ m_DebrisDef.m_nBoneIndex ];

		if( m_DebrisDef.m_nCollType == CFDebrisDef::COLL_TYPE_RAY ) {
			const CFSphere *pBoneBoundSphere_BS;
			CFVec3A NegBoneSpherePos_WS;

			pBoneBoundSphere_BS = &m_DebrisDef.m_pWorldMesh->m_pMesh->pBoneArray[ m_DebrisDef.m_nBoneIndex ].SegmentedBoundSphere_BS;
			NegBoneSpherePos_WS.ReceiveNegative( pBoneBoundSphere_BS->m_Pos );

			Mtx.m_vPos.Add( pBoneBoundSphere_BS->m_Pos );
			Mtx.MulPoint( NegBoneSpherePos_WS );
			Mtx.m_vPos = NegBoneSpherePos_WS;

			Mtx.m_vPos.y += pBoneBoundSphere_BS->m_fRadius * m_DebrisDef.m_fMeshScale;
		}

		pBoneMtx->Mul33( Mtx, m_DebrisDef.m_fMeshScale );
		pBoneMtx->m_vPos = Mtx.m_vPos;
	}

	// Update lighting...
	if( m_DebrisDef.m_nLightingType != CFDebrisDef::LIGHTING_TYPE_NONE ) {
		if( m_nSampleLightCountdown == 0 ) {
			fworld_LightPoint( &m_DebrisDef.m_Pos_WS, &m_DebrisDef.m_pWorldMesh->m_AmbientRGB, TRUE, m_DebrisDef.m_pWorldMesh );
			m_nSampleLightCountdown = _LIGHT_FRAMES_PER_SAMPLE;
		}

		--m_nSampleLightCountdown;
	}
}


// Returns TRUE if the particle has been killed.
BOOL CFDebris::_MoveLinear_NoColl( f32 fTimeStepSecs ) {
	CFVec3A TempVec;

	// Compute new linear velocity...
	m_DebrisDef.m_LinVel_WS.y += m_DebrisDef.m_fGravity * fTimeStepSecs;

	// Compute new position...
	TempVec.Mul( m_DebrisDef.m_LinVel_WS, fTimeStepSecs );
	m_DebrisDef.m_Pos_WS.Add( TempVec );

	return FALSE;
}


// Returns TRUE if the particle has been killed.
BOOL CFDebris::_MoveLinear_RayColl( f32 fTimeStepSecs ) {
	if( !(m_nFlags & FLAG_FINAL_REST_POS) ) {
		CFVec3A TempVec, PrevPos_WS;
		FCollImpact_t CollImpact;

		// Save old position...
		PrevPos_WS = m_DebrisDef.m_Pos_WS;

		// Compute new linear velocity...
		m_DebrisDef.m_LinVel_WS.y += m_DebrisDef.m_fGravity * fTimeStepSecs;

		// Compute new position...
		TempVec.Mul( m_DebrisDef.m_LinVel_WS, fTimeStepSecs );
		m_DebrisDef.m_Pos_WS.Add( TempVec );

		FWorld_nTrackerSkipListCount = 0;

		if( m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_BUILD_TRACKER_SKIP_LIST ) {
			if( _CallCallback( CFDebrisDef::CALLBACK_REASON_BUILD_TRACKER_SKIP_LIST, NULL ) ) {
				// The callback killed us...
				return TRUE;
			}
		}

		if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &PrevPos_WS, &m_DebrisDef.m_Pos_WS, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, m_DebrisDef.m_pWorldMesh, -1, FCOLL_MASK_COLLIDE_WITH_DEBRIS, FCOLL_LOD_HIGHEST ) ) {
			// We hit something!

			f32 fSurfaceNormComponentOfSpeed_WS, fSpeedOnSurfacePlane2_WS, fOOSpeedOnSurfacePlane_WS, fFrictionMag, fUnitIntensity;
			CFVec3A FrictionVec_WS, SurfaceNormComponentOfVel_WS, UnitDirOnSurfacePlane_WS;

			// If particle emitters have been specified, spawn them...
			if( m_DebrisDef.m_hParticleDef1 != FPARTICLE_INVALID_HANDLE ) {
				if( m_hParticle1 == NULL ) {
					fUnitIntensity = m_DebrisDef.m_fMeshScale * 0.5f;
					FMATH_CLAMPMAX( fUnitIntensity, 1.0f );

					if( m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_PLAY_FLAMING_SOUND ) {
						if( m_DebrisDef.m_pSoundGroupFlaming ) {
							if( m_pAudioEmitterFlaming == NULL ) {
								m_pAudioEmitterFlaming = CFSoundGroup::AllocAndPlaySound( m_DebrisDef.m_pSoundGroupFlaming, FALSE, &m_DebrisDef.m_Pos_WS, fUnitIntensity );
							}
						}
					}

					m_hParticle1 = fparticle_SpawnEmitter( m_DebrisDef.m_hParticleDef1, &m_DebrisDef.m_Pos_WS.v3, NULL, NULL, fUnitIntensity );
				}
			}

			if( m_DebrisDef.m_hParticleDef2 != FPARTICLE_INVALID_HANDLE ) {
				if( m_hParticle2 == NULL ) {
					fUnitIntensity = m_DebrisDef.m_fMeshScale * 0.25f;
					FMATH_CLAMPMAX( fUnitIntensity, 1.0f );

					m_hParticle2 = fparticle_SpawnEmitter( m_DebrisDef.m_hParticleDef2, &m_DebrisDef.m_Pos_WS.v3, NULL, NULL, fUnitIntensity );
				}
			}

			if( m_DebrisDef.m_pFcnCallback ) {
				if( _CallCallback( CFDebrisDef::CALLBACK_REASON_COLLISION, &CollImpact ) ) {
					// The callback killed us...
					return TRUE;
				}
			}

			// Compute new position...
			TempVec.Set( CollImpact.UnitFaceNormal );
			TempVec.Mul( 0.01f );
			m_DebrisDef.m_Pos_WS.Set( CollImpact.ImpactPoint );
			m_DebrisDef.m_Pos_WS.Add( TempVec );

			// Flatten linear velocity to surface plane...
			fSurfaceNormComponentOfSpeed_WS = -m_DebrisDef.m_LinVel_WS.Dot( CollImpact.UnitFaceNormal );
			SurfaceNormComponentOfVel_WS.Mul( CollImpact.UnitFaceNormal, fSurfaceNormComponentOfSpeed_WS );
			m_DebrisDef.m_LinVel_WS.Add( SurfaceNormComponentOfVel_WS );

			// Apply friction to linear velocity...
			fSpeedOnSurfacePlane2_WS = m_DebrisDef.m_LinVel_WS.MagSq();

			if( fSpeedOnSurfacePlane2_WS > 0.001f ) {
				fOOSpeedOnSurfacePlane_WS = fmath_InvSqrt( fSpeedOnSurfacePlane2_WS );
				fFrictionMag = (-2.5f*fmath_Inv( fOOSpeedOnSurfacePlane_WS ) - 60.0f) * fTimeStepSecs;

				UnitDirOnSurfacePlane_WS.Mul( m_DebrisDef.m_LinVel_WS, fOOSpeedOnSurfacePlane_WS );
				FrictionVec_WS.Mul( UnitDirOnSurfacePlane_WS, fFrictionMag );

				if( fFrictionMag*fFrictionMag < fSpeedOnSurfacePlane2_WS ) {
					m_DebrisDef.m_LinVel_WS.Add( FrictionVec_WS );
				} else {
					m_DebrisDef.m_LinVel_WS.Zero();

					if( CollImpact.pTag == NULL ) {
						if( m_DebrisDef.m_fRotSpeed == 0.0f ) {
							// We've come to rest on world geo...
							FMATH_SETBITMASK( m_nFlags, FLAG_FINAL_REST_POS );
						}
					}
				}
			} else {
				// Our linear velocity has come to rest...

				m_DebrisDef.m_LinVel_WS.Zero();

				// Kill any smoke...
				if( m_hSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
					smoketrail_ReturnToFreePool( m_hSmokeTrail, TRUE );
					m_hSmokeTrail = SMOKETRAIL_NULLHANDLE;
				}

				if( CollImpact.pTag == NULL ) {
					if( m_DebrisDef.m_fRotSpeed == 0.0f ) {
						// We've come to rest on world geo...
						FMATH_SETBITMASK( m_nFlags, FLAG_FINAL_REST_POS );
					}
				}
			}

			if( m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_FLAT_OBJECT ) {
				FMATH_SETBITMASK( m_nFlags, FLAG_FLATTEN_MODE );
			} else {
				FMATH_CLEARBITMASK( m_nFlags, FLAG_FLATTEN_MODE );
			}

			// Add a little bounce to linear velocity...
			if( fSurfaceNormComponentOfSpeed_WS > 15.0f ) {
				FMATH_CLEARBITMASK( m_nFlags, FLAG_FLATTEN_MODE );

				SurfaceNormComponentOfVel_WS.Mul( 0.25f + fmath_RandomFloatRange( -0.08f, 0.08f ) );
				m_DebrisDef.m_LinVel_WS.Add( SurfaceNormComponentOfVel_WS );

				if( m_DebrisDef.m_fRotSpeed < 2.0f ) {
					m_DebrisDef.m_fRotSpeed += m_DebrisDef.m_fRotSpeed * fmath_RandomFloatRange( 0.5f, 1.0f );
				}

				if( m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_PLAY_IMPACT_SOUND ) {
					if( m_DebrisDef.m_pSoundGroupImpact ) {
							if( CollImpact.UnitFaceNormal.y > 0.5f ) {
								f32 fUnitVolume = (fSurfaceNormComponentOfSpeed_WS - 15.0f) * (1.0f / (35.0f - 15.0f));
								FMATH_CLAMP_UNIT_FLOAT( fUnitVolume );

//								if( m_fImpactSoundTimer <= 0.0f ) {
//									m_fImpactSoundTimer = _TIME_BETWEEN_IMPACT_SOUNDS;

									CFSoundGroup::PlaySound( m_DebrisDef.m_pSoundGroupImpact, FALSE, &m_DebrisDef.m_Pos_WS, -1, FALSE, fUnitVolume );
//								}
							}
//						}
					}
				}
			}

			if( m_DebrisDef.m_nFlags & CFDebrisDef::FLAG_ROTATIONAL_MOTION ) {
				if( fSurfaceNormComponentOfSpeed_WS > 15.0f ) {
					m_DebrisDef.m_UnitRotAxis_WS.SetUnitRandom();
				}

				// Apply friction to rotational velocity...

				FASSERT( m_DebrisDef.m_fRotSpeed >= 0 );

				m_DebrisDef.m_fRotSpeed -= 40.0f * fTimeStepSecs;
				FMATH_CLAMPMIN( m_DebrisDef.m_fRotSpeed, 0.0f );
			}

			if( m_nFlags & FLAG_FLATTEN_MODE ) {
				if( m_DebrisDef.m_pWorldMesh->m_Xfm.m_MtxF.m_vY.Dot( CollImpact.UnitFaceNormal ) >= 0.0f ) {
					m_FlattenModeUnitFaceNorm = CollImpact.UnitFaceNormal;
				} else {
					m_FlattenModeUnitFaceNorm.ReceiveNegative( CollImpact.UnitFaceNormal );
				}

				m_FlattenModeUnitRotAxis.Cross( m_DebrisDef.m_pWorldMesh->m_Xfm.m_MtxF.m_vY, m_FlattenModeUnitFaceNorm );

				f32 fMag2 = m_FlattenModeUnitRotAxis.MagSq();

				if( fMag2 > 0.00001f ) {
					m_FlattenModeUnitRotAxis.Mul( fmath_InvSqrt(fMag2) );
					m_DebrisDef.m_fRotSpeed = FMATH_2PI * 2.0f;
				} else {
					FMATH_CLEARBITMASK( m_nFlags, FLAG_FLATTEN_MODE );
				}
			}
		}
	}

	// Start fade-out if particle has fallen below specified velocity...

	if( m_DebrisDef.m_fMinSpeedToStartFade2 >= 0.0f ) {
		if( m_DebrisDef.m_fSecsBeforeMinSpeedValid < 0.0f ) {
			if( m_DebrisDef.m_LinVel_WS.MagSq() < m_DebrisDef.m_fMinSpeedToStartFade2 ) {
				// We're moving slowly. Update timer...
				m_fMovingSlowlyCountdownSecs -= FLoop_fPreviousLoopSecs;

				if( m_fMovingSlowlyCountdownSecs <= 0.0f ) {
					// Start fading out...

					if( StartFading( TRUE ) ) {
						// We have been killed...
						return TRUE;
					}

					m_DebrisDef.m_fMinSpeedToStartFade2 = -1.0f;
				}
			} else {
				m_fMovingSlowlyCountdownSecs = m_DebrisDef.m_fSecsUnderMinSpeedToStartFade;
			}
		}
	}

	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisGroup
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CFDebrisGroup::m_bModuleStartedUp;
FLinkRoot_t CFDebrisGroup::m_LinkRoot;

const FGameData_TableEntry_t CFDebrisGroup::m_aGameDataVocab[] = {
	// pszMeshSet:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupImpact
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFlaming

	// nPriority:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
#if FANG_WINGC
	F32_DATATABLE_5,	// retail clamps priority to [0, 4] and uses 3 (this float table has no 4.0)
#else
	F32_DATATABLE_2,
#endif

	// bIgnoreFlatObjectFlag:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// bRotationalMotion:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

#if !FANG_WINGC
	// bRandomOrientation: absent from the retail GameCube deb_group.csv (24 fields). Its
	// retail loader stores priority and the two flags above at the same CFDebrisGroup
	// offsets as this source and never writes m_bRandomOrientation.
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,
#endif

	// fUnitDustKickUp:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fUnitHitpointDamage:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// pszCollType:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszFadeType:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszLightingType:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMinSpeedToStartFade2:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X_SQUARED | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fSecsBeforeMinSpeedValid:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fSecsUnderMinSpeedToStartFade:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fMinAliveSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fMaxAliveSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fFadeSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fMinScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000000000,

	// fMaxScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000000000,

	// fMinGravity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMaxGravity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMinRotSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fMaxRotSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


CFDebrisDef CFDebrisGroup::m_DebrisDef;


BOOL CFDebrisGroup::ModuleStartup( void ) {
	FASSERT( !IsModuleStartedUp() );

	flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CFDebrisGroup, m_Link ) );

	m_bModuleStartedUp = TRUE;

	return TRUE;
}


void CFDebrisGroup::ModuleShutdown( void ) {
	if( IsModuleStartedUp() ) {
		m_bModuleStartedUp = FALSE;
	}
}


CFDebrisGroup *CFDebrisGroup::Find( cchar *pszDebrisGroupName ) {
	FASSERT( IsModuleStartedUp() );

	if( pszDebrisGroupName == NULL ) {
		return NULL;
	}

	CFDebrisGroup *pDebrisGroup;

	for( pDebrisGroup=(CFDebrisGroup *)flinklist_GetHead(&m_LinkRoot); pDebrisGroup; pDebrisGroup=(CFDebrisGroup *)flinklist_GetNext(&m_LinkRoot, pDebrisGroup) ) {
		if( !fclib_stricmp( pszDebrisGroupName, pDebrisGroup->m_pszDebrisGroupName ) ) {
			// Found the mesh set...
			return pDebrisGroup;
		}
	}

	// Mesh set not found...
	return NULL;
}


CFDebrisGroup *CFDebrisGroup::AllocEmpty( cchar *pszDebrisGroupName ) {
	FASSERT( IsModuleStartedUp() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( pszDebrisGroupName && Find( pszDebrisGroupName ) ) {
		DEVPRINTF( "CFDebrisGroup::AllocEmpty(): Debris group '%s' redefinition.\n", pszDebrisGroupName );
		goto _ExitWithError;
	}

	CFDebrisGroup *pDebrisGroup;

	// Create a resource...
	pDebrisGroup = (CFDebrisGroup *)fres_CreateAndAlloc( NULL, NULL, _ResDestroyedCallback, sizeof(CFDebrisGroup), NULL );
	if( pDebrisGroup == NULL ) {
		if( pszDebrisGroupName ) {
			DEVPRINTF( "CFDebrisGroup::AllocEmpty(): Not enough memory to allocate debris group '%s'.\n", pszDebrisGroupName );
		} else {
			DEVPRINTF( "CFDebrisGroup::AllocEmpty(): Not enough memory to allocate debris group '<unnamed>'.\n" );
		}

		goto _ExitWithError;
	}

	// We've successfully created our debris group resource!

	// Add to the end of our linklist...
	flinklist_AddTail( &m_LinkRoot, pDebrisGroup );

	if( pszDebrisGroupName ) {
		pDebrisGroup->m_pszDebrisGroupName = CFStringTable::AddString( NULL, pszDebrisGroupName );
		if( pDebrisGroup->m_pszDebrisGroupName == NULL ) {
			DEVPRINTF( "CFDebrisGroup::AllocEmpty(): Not enough memory to store name for debris group '%s'.\n", pszDebrisGroupName );
			goto _ExitWithError;
		}
	} else {
		pDebrisGroup->m_pszDebrisGroupName = "<unnamed>";
	}

	return pDebrisGroup;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


CFDebrisGroup *CFDebrisGroup::LoadFromGameData( FGameDataTableHandle_t hTable, cchar *pszMeshSetFileName ) {
	cchar *pszDebrisGroupName;
	CFDebrisGroup *pMeshGroup;
	CFDebrisMeshSet *pMeshSet;
	_UserProps_t UserProps;

	FASSERT( IsModuleStartedUp() );

	FResFrame_t ResFrame = fres_GetFrame();

	pszDebrisGroupName = fgamedata_GetTableName( hTable );

	if( !fgamedata_GetTableData( hTable, m_aGameDataVocab, &UserProps, sizeof(_UserProps_t) ) ) {
		DEVPRINTF( "CFDebrisGroup::LoadFromGameData(): Trouble parsing debris group data for '%s.csv'.\n", pszDebrisGroupName );
		goto _ExitWithError;
	}

	pMeshGroup = AllocEmpty( pszDebrisGroupName );
	if( pMeshGroup == NULL ) {
		goto _ExitWithError;
	}

	if( UserProps.pszMeshSet ) {
		pMeshSet = CFDebrisMeshSet::LoadFromGameData( pszMeshSetFileName, UserProps.pszMeshSet );
		if( pMeshSet == NULL ) {
			goto _ExitWithError;
		}

		pMeshGroup->m_pDebrisMeshArray = pMeshSet->m_pDebrisMeshArray;
		pMeshGroup->m_nDebrisMeshCount = pMeshSet->m_nDebrisMeshCount;
	} else {
		pMeshGroup->m_pDebrisMeshArray = NULL;
		pMeshGroup->m_nDebrisMeshCount = 0;
	}

	pMeshGroup->m_nPriority = UserProps.nPriority;
	pMeshGroup->m_bIgnoreFlatObjectFlag = UserProps.bIgnoreFlatObjectFlag;
	pMeshGroup->m_bRotationalMotion = UserProps.bRotationalMotion;
#if FANG_WINGC
	pMeshGroup->m_bRandomOrientation = FALSE;
#else
	pMeshGroup->m_bRandomOrientation = UserProps.bRandomOrientation;
#endif
	pMeshGroup->m_pSoundGroupImpact = UserProps.pSoundGroupImpact;
	pMeshGroup->m_pSoundGroupFlaming = UserProps.pSoundGroupFlaming;

	pMeshGroup->m_fUnitDustKickUp = UserProps.fUnitDustKickUp;
	pMeshGroup->m_fUnitHitpointDamage = UserProps.fUnitHitpointDamage;

	if( !fclib_stricmp( UserProps.pszCollType, "None" ) ) {
		pMeshGroup->m_nCollType = CFDebrisDef::COLL_TYPE_NONE;
	} else if( !fclib_stricmp( UserProps.pszCollType, "Ray" ) ) {
		pMeshGroup->m_nCollType = CFDebrisDef::COLL_TYPE_RAY;
	} else {
		DEVPRINTF( "CFDebrisGroup::LoadFromGameData(): Unrecognized collision type '%s' specified in '%s.csv'.\n", UserProps.pszCollType, pszDebrisGroupName );
		pMeshGroup->m_nCollType = CFDebrisDef::COLL_TYPE_NONE;
	}

	if( !fclib_stricmp( UserProps.pszFadeType, "Shrink" ) ) {
		pMeshGroup->m_nFadeType = CFDebrisDef::FADE_TYPE_SHRINK;
	} else if( !fclib_stricmp( UserProps.pszFadeType, "Alpha" ) ) {
		pMeshGroup->m_nFadeType = CFDebrisDef::FADE_TYPE_ALPHA;
	} else if( !fclib_stricmp( UserProps.pszFadeType, "Pop" ) ) {
		pMeshGroup->m_nFadeType = CFDebrisDef::FADE_TYPE_POP_OFF;
	} else {
		DEVPRINTF( "CFDebrisGroup::LoadFromGameData(): Unrecognized fade-out type '%s' specified in '%s.csv'.\n", UserProps.pszFadeType, pszDebrisGroupName );
		pMeshGroup->m_nFadeType = CFDebrisDef::FADE_TYPE_SHRINK;
	}

	if( !fclib_stricmp( UserProps.pszLightingType, "Ambient" ) ) {
		pMeshGroup->m_nLightingType = CFDebrisDef::LIGHTING_TYPE_AMBIENT_ONLY;
	} else if( !fclib_stricmp( UserProps.pszLightingType, "Full" ) ) {
		pMeshGroup->m_nLightingType = CFDebrisDef::LIGHTING_TYPE_FULL_DYNAMIC;
	} else if( !fclib_stricmp( UserProps.pszLightingType, "None" ) ) {
		pMeshGroup->m_nLightingType = CFDebrisDef::LIGHTING_TYPE_NONE;
	} else {
		DEVPRINTF( "CFDebrisGroup::LoadFromGameData(): Unrecognized lighting type '%s' specified in '%s.csv'.\n", UserProps.pszLightingType, pszDebrisGroupName );
		pMeshGroup->m_nLightingType = CFDebrisDef::LIGHTING_TYPE_AMBIENT_ONLY;
	}

	pMeshGroup->m_fMinSpeedToStartFade2 = UserProps.fMinSpeedToStartFade2;
	pMeshGroup->m_fSecsBeforeMinSpeedValid = UserProps.fSecsBeforeMinSpeedValid;
	pMeshGroup->m_fSecsUnderMinSpeedToStartFade = UserProps.fSecsUnderMinSpeedToStartFade;

	pMeshGroup->m_fMinAliveSecs = UserProps.fMinAliveSecs;
	pMeshGroup->m_fMaxAliveSecs = UserProps.fMaxAliveSecs;
	pMeshGroup->m_fFadeSecs = UserProps.fFadeSecs;
	pMeshGroup->m_fCullDist = UserProps.fCullDist;

	pMeshGroup->m_fMinScale = UserProps.fMinScale;
	pMeshGroup->m_fMaxScale = UserProps.fMaxScale;

	pMeshGroup->m_fMinGravity = UserProps.fMinGravity;
	pMeshGroup->m_fMaxGravity = UserProps.fMaxGravity;

	pMeshGroup->m_fMinRotSpeed = UserProps.fMinRotSpeed;
	pMeshGroup->m_fMaxRotSpeed = UserProps.fMaxRotSpeed;

	return pMeshGroup;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


// Loads one debris group from the file of the same name.
CFDebrisGroup *CFDebrisGroup::LoadFromGameData( cchar *pszFileName, cchar *pszDebrisGroupName, cchar *pszMeshSetFileName ) {
	FASSERT( IsModuleStartedUp() );

	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	CFDebrisGroup *pDebrisGroup;

	pDebrisGroup = Find( pszDebrisGroupName );
	if( pDebrisGroup ) {
		// Already exists...
		return pDebrisGroup;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();

	hFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFDebrisGroup::LoadFromGameData(): Could not load debris group file '%s.csv'.\n", pszFileName );
		return FALSE;
	}

	pDebrisGroup = NULL;

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		if( !fclib_stricmp( fgamedata_GetTableName( hTable ), pszDebrisGroupName ) ) {
			// Found table...

			pDebrisGroup = LoadFromGameData( hTable, pszMeshSetFileName );

			break;
		}
	}

	if( pDebrisGroup == NULL ) {
		DEVPRINTF( "CFDebrisGroup::LoadFromGameData(): Could not locate debris group '%s' in file '%s.csv'.\n", pszDebrisGroupName, pszFileName );
	}

	fmem_ReleaseFrame( MemFrame );

	return pDebrisGroup;
}


// Loads an entire set of debris groups from a single file.
// Returns TRUE if all were loaded successfully.
// If an error is encountered, this function will continue to try and
// load the other debris groups from the file.
BOOL CFDebrisGroup::LoadAllFromGameData( cchar *pszFileName, cchar *pszMeshSetFileName ) {
	FASSERT( IsModuleStartedUp() );

	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	BOOL bSuccess;

	FMemFrame_t MemFrame = fmem_GetFrame();

	bSuccess = TRUE;

	hFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFDebrisGroup::LoadAllFromGameData(): Could not load debris group file '%s.csv'.\n", pszFileName );
		return FALSE;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		if( !LoadFromGameData( hTable, pszMeshSetFileName ) ) {
			bSuccess = FALSE;
		}
	}

	fmem_ReleaseFrame( MemFrame );

	return bSuccess;
}


void CFDebrisGroup::_ResDestroyedCallback( void *pResMem ) {
	CFDebrisGroup *pDebrisGroup = (CFDebrisGroup *)pResMem;

	FASSERT( pDebrisGroup == (CFDebrisGroup *)flinklist_GetStructurePointer( &m_LinkRoot, m_LinkRoot.pTailLink ) );

	flinklist_Remove( &m_LinkRoot, pDebrisGroup );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisSpawner
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CFDebrisSpawner::m_bSystemInitialized;
u32 CFDebrisSpawner::m_nLowestDebrisWeight;
FLinkRoot_t CFDebrisSpawner::m_RootFree;
FLinkRoot_t CFDebrisSpawner::m_RootActive;
CFDebrisSpawner *CFDebrisSpawner::m_pSpawnerPool;
CFDebrisSpawner **CFDebrisSpawner::m_ppSpawnerWorkList;
CFDebrisSpawner CFDebrisSpawner::m_DefaultSpawner;

BOOL CFDebrisSpawner::InitSpawnerSystem( u32 nMaxSpawnerCount ) {
	FASSERT( !IsSpawnerSystemInitialized() );

	u32 i;

	flinklist_InitRoot( &m_RootFree, FANG_OFFSETOF( CFDebrisSpawner, m_Link ) );
	flinklist_InitRoot( &m_RootActive, FANG_OFFSETOF( CFDebrisSpawner, m_Link ) );

	FResFrame_t ResFrame = fres_GetFrame();

	m_pSpawnerPool = fnew CFDebrisSpawner[nMaxSpawnerCount];
	if( m_pSpawnerPool == NULL ) {
		DEVPRINTF( "CFDebrisSpawner::InitSpawnerSystem(): Not enough memory to allocate debris spawner pool.\n" );
		goto _ExitWithError;
	}

	flinklist_InitPool( &m_RootFree, m_pSpawnerPool, sizeof(CFDebrisSpawner), nMaxSpawnerCount );

	for( i=0; i<nMaxSpawnerCount; ++i ) {
		m_pSpawnerPool[i].m_bCameFromFreePool = TRUE;
		m_pSpawnerPool[i].m_nListMember = LIST_MEMBER_FREE;
	}

	m_ppSpawnerWorkList = (CFDebrisSpawner **)fres_Alloc( sizeof(CFDebrisSpawner *) * nMaxSpawnerCount );
	if( m_ppSpawnerWorkList == NULL ) {
		DEVPRINTF( "CFDebrisSpawner::InitSpawnerSystem(): Not enough memory to allocate debris spawner work list.\n" );
		goto _ExitWithError;
	}

	m_DefaultSpawner.m_Mtx.Identity();
	m_DefaultSpawner.m_fPlaneDimX = 0.0f;
	m_DefaultSpawner.m_fPlaneDimY = 0.0f;
	m_DefaultSpawner.m_nEmitterType = EMITTER_TYPE_POINT;
	m_DefaultSpawner.m_pDebrisGroup = NULL;
	m_DefaultSpawner.m_pFcnCallback = NULL;
	m_DefaultSpawner.m_pUser = NULL;
	m_DefaultSpawner.m_nUser = 0;
	m_DefaultSpawner.m_nOwnerID = 0;
	m_DefaultSpawner.m_pDebrisMesh = NULL;
	m_DefaultSpawner.m_fSpawnerAliveSecs = 0.0f;
	m_DefaultSpawner.m_fSecsBetweenSpawns = 0.0f;
	m_DefaultSpawner.m_fSpawnerFadeSecs = 0.0f;
	m_DefaultSpawner.m_fMinSpeed = 10.0f;
	m_DefaultSpawner.m_fMaxSpeed = 15.0f;
	m_DefaultSpawner.m_fUnitDirSpread = 0.0f;
	m_DefaultSpawner.m_fScaleMul = 1.0f;
	m_DefaultSpawner.m_fGravityMul = 1.0f;
	m_DefaultSpawner.m_fRotSpeedMul = 1.0f;
	m_DefaultSpawner.m_nFlags = FLAG_NONE;
	m_DefaultSpawner.m_nMinDebrisCount = 1;
	m_DefaultSpawner.m_nMaxDebrisCount = 1;
	m_DefaultSpawner.m_nTintRed = 255;
	m_DefaultSpawner.m_nTintGreen = 255;
	m_DefaultSpawner.m_nTintBlue = 255;
	m_DefaultSpawner.m_pSmokeTrailAttrib = NULL;
	m_DefaultSpawner.m_hParticleDef1 = FPARTICLE_INVALID_HANDLE;
	m_DefaultSpawner.m_hParticleDef2 = FPARTICLE_INVALID_HANDLE;

	m_DefaultSpawner.m_bFading = FALSE;
	m_DefaultSpawner.m_bCameFromFreePool = FALSE;
	m_DefaultSpawner.m_nListMember = LIST_MEMBER_NONE;
	m_DefaultSpawner.m_fCountdownTimer = 0.0f;
	m_DefaultSpawner.m_fSecsBeforeNextSpawn = 0.0f;

	m_DefaultSpawner.m_Link.pNextLink = NULL;
	m_DefaultSpawner.m_Link.pPrevLink = NULL;
	
	m_nLowestDebrisWeight = 0xffffffff;

	m_bSystemInitialized = TRUE;

	return TRUE;

_ExitWithError:
	if( m_pSpawnerPool ) {
		fdelete_array( m_pSpawnerPool );
		m_pSpawnerPool = NULL;
	}
	fres_ReleaseFrame( ResFrame );
	m_bSystemInitialized = FALSE;
	return FALSE;
}


void CFDebrisSpawner::UninitSpawnerSystem( void ) {
	if( IsSpawnerSystemInitialized() ) {
		if( m_pSpawnerPool ) {
			fdelete_array( m_pSpawnerPool );
			m_pSpawnerPool = NULL;
		}
		m_ppSpawnerWorkList = NULL;
		m_bSystemInitialized = FALSE;
	}
}


CFDebrisSpawner *CFDebrisSpawner::GetFreeSpawner( void ) {
	FASSERT( IsSpawnerSystemInitialized() );

	CFDebrisSpawner *pSpawner;

	pSpawner = (CFDebrisSpawner *)flinklist_RemoveTail( &m_RootFree );
	if( pSpawner == NULL ) {
		return NULL;
	}

	FASSERT( pSpawner->m_nListMember == LIST_MEMBER_FREE );
	FASSERT( pSpawner->m_bCameFromFreePool );

	fang_MemCopy( pSpawner, &m_DefaultSpawner, sizeof(CFDebrisSpawner) );
	FASSERT( pSpawner->m_nListMember == LIST_MEMBER_NONE );

	pSpawner->m_bCameFromFreePool = TRUE;

	return pSpawner;
}


void CFDebrisSpawner::Kill( void ) {
	FASSERT( IsSpawnerSystemInitialized() );

	if( m_nListMember == LIST_MEMBER_ACTIVE ) {
		// Remove from active list...
		flinklist_Remove( &m_RootActive, this );

		if( m_bCameFromFreePool ) {
			// Return to free pool...
			flinklist_AddTail( &m_RootFree, this );
			m_nListMember = LIST_MEMBER_FREE;
		} else {
			// Return to no pool...
			m_nListMember = LIST_MEMBER_NONE;
		}
	}
}


BOOL CFDebrisSpawner::IsSpawning( void ) const {
	FASSERT( IsSpawnerSystemInitialized() );

	if( m_nListMember == LIST_MEMBER_ACTIVE ) {
		return TRUE;
	}

	return FALSE;
}


f32 CFDebrisSpawner::GetUnitIntensity( void ) const {
	FASSERT( IsSpawnerSystemInitialized() );

	if( !IsSpawning() ) {
		return 0.0f;
	}

	if( !m_bFading ) {
		return 1.0f;
	}

	f32 fUnitIntensity = fmath_Div( m_fCountdownTimer, m_fSpawnerFadeSecs );
	FMATH_CLAMPMAX( fUnitIntensity, 1.0f );

	return fUnitIntensity;
}


void CFDebrisSpawner::KillAll( void ) {
	FASSERT( IsSpawnerSystemInitialized() );

	CFDebrisSpawner *pSpawner;

	while( pSpawner = (CFDebrisSpawner *)flinklist_GetTail( &m_RootActive ) ) {
		pSpawner->Kill();
		FASSERT( pSpawner->m_nListMember != CFDebrisSpawner::LIST_MEMBER_ACTIVE );
	}
}


void CFDebrisSpawner::Spawn( void ) {
	FASSERT( IsSpawnerSystemInitialized() );
	FASSERT_MSG( m_pSmokeTrailAttrib != (SmokeTrailAttrib_t *)0xffffffff, "You must call CFDebrisSpawner::InitToDefaults() before using a Debris Spawner." );

	if( m_pDebrisGroup == NULL ) {
		Kill();
		return;
	}

	if( (m_pDebrisGroup->m_nDebrisMeshCount == 0) && (m_pDebrisMesh == NULL) ) {
		Kill();
		return;
	}

	FASSERT( m_fSpawnerAliveSecs >= 0.0f );
	FASSERT( m_fMinSpeed >= 0.0f );
	FASSERT( m_fMaxSpeed >= 0.0f );

	FASSERT( m_nEmitterType>=0 && m_nEmitterType<EMITTER_TYPE_COUNT );

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
		if( m_nEmitterType == EMITTER_TYPE_BOUNDED_PLANE ) {
			FASSERT( m_fPlaneDimX >= 0.0f );
			FASSERT( m_fPlaneDimY >= 0.0f );
		}
	#endif

	if( m_fSpawnerAliveSecs == 0.0f ) {
		_Spawn();
		Kill();
		return;
	}

	FASSERT( m_nListMember == LIST_MEMBER_NONE );
	FASSERT( m_fSecsBetweenSpawns >= 0.0f );
	FASSERT( m_fSpawnerFadeSecs >= 0.0f );

	m_bFading = FALSE;
	m_fSecsBeforeNextSpawn = 0.0f;
	m_fCountdownTimer = m_fSpawnerAliveSecs;

	flinklist_AddTail( &m_RootActive, this );
	m_nListMember = LIST_MEMBER_ACTIVE;
}


// Returns FALSE if the debris was not spawned (always tries to spawn 1 particle).
BOOL CFDebrisSpawner::Spawn( CFWorldMesh *pWorldMesh, u32 nBoneIndex ) {
	FASSERT( IsSpawnerSystemInitialized() );
	FASSERT_MSG( m_pSmokeTrailAttrib != (SmokeTrailAttrib_t *)0xffffffff, "You must call CFDebrisSpawner::InitToDefaults() before using a Debris Spawner." );
	if( m_pDebrisGroup == NULL ) {
		Kill();
		return FALSE;
	}

	FASSERT( m_fSpawnerAliveSecs >= 0.0f );
	FASSERT( m_fMinSpeed >= 0.0f );
	FASSERT( m_fMaxSpeed >= 0.0f );

	FASSERT( m_nEmitterType>=0 && m_nEmitterType<EMITTER_TYPE_COUNT );

	if( m_fSpawnerAliveSecs != 0.0f ) {
		Kill();
		return FALSE;
	}

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
		if( m_nEmitterType == EMITTER_TYPE_BOUNDED_PLANE ) {
			FASSERT( m_fPlaneDimX >= 0.0f );
			FASSERT( m_fPlaneDimY >= 0.0f );
		}
	#endif

	m_bFading = FALSE;
	m_fSecsBeforeNextSpawn = 0.0f;
	m_fCountdownTimer = m_fSpawnerAliveSecs;

	BOOL bSpawnedOk;

	bSpawnedOk = _Spawn( pWorldMesh, nBoneIndex );

	Kill();

	return bSpawnedOk;
}


void CFDebrisSpawner::Work( void ) {
	FASSERT( IsSpawnerSystemInitialized() );

	if( m_RootActive.nCount == 0 ) {
		return;
	}

	// Reset the lowest debris
	m_nLowestDebrisWeight = 0xffffffff;

	CFDebrisSpawner *pSpawner;
	u32 i, nWorkCount;

	i = 0;
	for( pSpawner=(CFDebrisSpawner *)flinklist_GetHead(&m_RootActive); pSpawner; pSpawner=(CFDebrisSpawner *)flinklist_GetNext( &m_RootActive, pSpawner ) ) {
		m_ppSpawnerWorkList[i++] = pSpawner;
	}

	FASSERT( i == m_RootActive.nCount );

	nWorkCount = m_RootActive.nCount;

	for( i=0; i<nWorkCount; ++i ) {
		pSpawner = m_ppSpawnerWorkList[i];

		if( !pSpawner->m_bFading ) {
			// Not currently fading...

			pSpawner->m_fCountdownTimer -= FLoop_fPreviousLoopSecs;

			if( pSpawner->m_fCountdownTimer <= 0.0f ) {
				if( pSpawner->m_fSpawnerFadeSecs <= 0.001f ) {
					// No fading...

					pSpawner->Kill();
					continue;
				} else {
					// Fade...

					pSpawner->m_bFading = TRUE;
					pSpawner->m_fCountdownTimer = pSpawner->m_fSpawnerFadeSecs;
				}
			}
		} else {
			// We're fading...

			pSpawner->m_fCountdownTimer -= FLoop_fPreviousLoopSecs;

			if( pSpawner->m_fCountdownTimer <= 0.0f ) {
				pSpawner->Kill();
				continue;
			}
		}

		pSpawner->m_fSecsBeforeNextSpawn -= FLoop_fPreviousLoopSecs;

		if( pSpawner->m_fSecsBeforeNextSpawn <= 0.0f ) {
			pSpawner->m_fSecsBeforeNextSpawn = pSpawner->m_fSecsBetweenSpawns;

			pSpawner->_Spawn();
		}
	}
}


// Returns FALSE if no particles could be spawned.
// Returns TRUE if at least one particle was spawned.
BOOL CFDebrisSpawner::_Spawn( CFWorldMesh *pWorldMesh, s32 nBoneIndex ) {
	FASSERT( IsSpawnerSystemInitialized() );
	FASSERT( pWorldMesh==NULL || nBoneIndex>=0 );

	if( pWorldMesh==NULL && m_pDebrisGroup->m_nDebrisMeshCount==0 && m_pDebrisMesh==NULL ) {
		return FALSE;
	}

	CFDebrisDef DebrisDef;
	const CFDebrisMesh *pDebrisMesh;
	CFVec3A PerturbVec_WS, DistVecX;
	u32 i, nDebrisCount;
	f32 fSpeed, fPerturbFactor, fNegPerturbFactor, fNegHalfDimX, fHalfDimX, fNegHalfDimY, fHalfDimY, fPlanePosX, fPlanePosY;
	BOOL bParticleSpawned = FALSE;

	BOOL bRotationalMotion = m_pDebrisGroup->m_bRotationalMotion && (m_fRotSpeedMul != 0.0f) && ( (m_pDebrisGroup->m_fMinRotSpeed != 0.0f) || (m_pDebrisGroup->m_fMaxRotSpeed != 0.0f) );

	// Set up constant fields...
	DebrisDef.m_nBoneIndex = -1;
	DebrisDef.m_nPriority = m_pDebrisGroup->m_nPriority;
	DebrisDef.m_nCollType = m_pDebrisGroup->m_nCollType;
	DebrisDef.m_nFadeType = m_pDebrisGroup->m_nFadeType;
	DebrisDef.m_nLightingType = m_pDebrisGroup->m_nLightingType;
	DebrisDef.m_fMinSpeedToStartFade2 = m_pDebrisGroup->m_fMinSpeedToStartFade2;
	DebrisDef.m_fSecsBeforeMinSpeedValid = m_pDebrisGroup->m_fSecsBeforeMinSpeedValid;
	DebrisDef.m_fSecsUnderMinSpeedToStartFade = m_pDebrisGroup->m_fSecsUnderMinSpeedToStartFade;
	DebrisDef.m_fAliveSecs = fmath_RandomFloatRange( m_pDebrisGroup->m_fMinAliveSecs, m_pDebrisGroup->m_fMaxAliveSecs );
	DebrisDef.m_fFadeSecs = m_pDebrisGroup->m_fFadeSecs;
	DebrisDef.m_fCullDist = m_pDebrisGroup->m_fCullDist;
	DebrisDef.m_pFcnCallback = m_pFcnCallback;
	DebrisDef.m_pUser = m_pUser;
	DebrisDef.m_nUser = m_nUser;
	DebrisDef.m_nOwnerID = m_nOwnerID;
	DebrisDef.m_fMeshUnitOpacity = 1.0f;
	DebrisDef.m_pSoundGroupImpact = m_pDebrisGroup->m_pSoundGroupImpact;
	DebrisDef.m_pSoundGroupFlaming = m_pDebrisGroup->m_pSoundGroupFlaming;
	DebrisDef.m_pSmokeTrailAttrib = m_pSmokeTrailAttrib;
	DebrisDef.m_hParticleDef1 = m_hParticleDef1;
	DebrisDef.m_hParticleDef2 = m_hParticleDef2;

	DebrisDef.m_nFlags = CFDebrisDef::FLAG_NONE;

	if( m_nFlags & FLAG_PLAY_IMPACT_SOUND ) {
		DebrisDef.m_nFlags |= CFDebrisDef::FLAG_PLAY_IMPACT_SOUND;
	}

	if( m_nFlags & FLAG_PLAY_FLAMING_SOUND ) {
		DebrisDef.m_nFlags |= CFDebrisDef::FLAG_PLAY_FLAMING_SOUND;
	}

	if( m_nFlags & FLAG_BUILD_TRACKER_SKIP_LIST ) {
		DebrisDef.m_nFlags |= CFDebrisDef::FLAG_BUILD_TRACKER_SKIP_LIST;
	}

	if( bRotationalMotion ) {
		FMATH_SETBITMASK( DebrisDef.m_nFlags, CFDebrisDef::FLAG_ROTATIONAL_MOTION );
	}

	if( m_pDebrisGroup->m_bRandomOrientation ) {
		FMATH_SETBITMASK( DebrisDef.m_nFlags, CFDebrisDef::FLAG_RANDOM_ORIENTATION );
	}

	fPerturbFactor = FMATH_FPOT( m_fUnitDirSpread, 0.0f, 0.5f );
	fNegPerturbFactor = -fPerturbFactor;

	// Compute the number of debris particles we will spawn...
	if( pWorldMesh == NULL ) {
		if( m_nEmitterType == EMITTER_TYPE_POINT ) {
			DebrisDef.m_Pos_WS = m_Mtx.m_vPos;
		} else {
			fHalfDimX = 0.5f * m_fPlaneDimX;
			fHalfDimY = 0.5f * m_fPlaneDimY;

			fNegHalfDimX = -fHalfDimX;
			fNegHalfDimY = -fHalfDimY;
		}

		nDebrisCount = fmath_RandomRange( m_nMinDebrisCount, m_nMaxDebrisCount );

		if( m_bFading ) {
			f32 fUnitFade = fmath_Div( m_fCountdownTimer, m_fSpawnerFadeSecs );

			nDebrisCount = fmath_FloatToU32( fUnitFade * (f32)nDebrisCount );
		}
	} else {
		DebrisDef.m_Pos_WS = pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vPos;
		nDebrisCount = 1;
	}

	for( i=0; i<nDebrisCount; ++i ) {
		DebrisDef.m_fUnitDustKickUp = 0.0f;
		DebrisDef.m_fUnitHitpointDamage = 0.0f;

		if( pWorldMesh == NULL ) {
			// Pick a mesh...

			if( m_pDebrisMesh == NULL ) {
				// Obtain debris mesh from debris group...

				pDebrisMesh = &m_pDebrisGroup->m_pDebrisMeshArray[ fmath_RandomChoice( m_pDebrisGroup->m_nDebrisMeshCount ) ];
			} else {
				// Debris mesh explicitely specified...

				pDebrisMesh = m_pDebrisMesh;
			}

			if( !(m_nFlags & FLAG_OVERRIDE_MESH_TINT) || !(pDebrisMesh->m_nFlags & CFDebrisMesh::FLAG_OVERRIDEABLE_TINT) ) {
				// Spawner does not wish to (or can not) override the mesh tint...

				if( !(pDebrisMesh->m_nFlags & CFDebrisMesh::FLAG_TINT_IS_WHITE) ) {
					// The mesh set's tint is not white...

					DebrisDef.m_nTintRed = pDebrisMesh->m_nTintRed;
					DebrisDef.m_nTintGreen = pDebrisMesh->m_nTintGreen;
					DebrisDef.m_nTintBlue = pDebrisMesh->m_nTintBlue;

					DebrisDef.m_nFlags |= CFDebrisDef::FLAG_OVERRIDE_MESH_TINT;
				}
			} else {
				// Spawner wishes to (and can) override the mesh tint...

				if( (m_nTintRed & m_nTintGreen & m_nTintBlue) < 255 ) {
					// The spawner's tint override color is not white...

					DebrisDef.m_nTintRed = m_nTintRed;
					DebrisDef.m_nTintGreen = m_nTintGreen;
					DebrisDef.m_nTintBlue = m_nTintBlue;

					DebrisDef.m_nFlags |= CFDebrisDef::FLAG_OVERRIDE_MESH_TINT;
				}
			}

			DebrisDef.m_pMesh = pDebrisMesh->m_pMesh;
			DebrisDef.m_fMeshScale = pDebrisMesh->m_fScale * fmath_RandomFloatRange( m_pDebrisGroup->m_fMinScale, m_pDebrisGroup->m_fMaxScale ) * m_fScaleMul;

			if( pDebrisMesh->m_nFlags & CFDebrisMesh::FLAG_CAN_KICKUP_DUST ) {
				DebrisDef.m_fUnitDustKickUp = m_pDebrisGroup->m_fUnitDustKickUp;
			}

			if( pDebrisMesh->m_nFlags & CFDebrisMesh::FLAG_CAN_DAMAGE ) {
				DebrisDef.m_fUnitHitpointDamage = m_pDebrisGroup->m_fUnitHitpointDamage;
			}

			if( !m_pDebrisGroup->m_bIgnoreFlatObjectFlag ) {
				if( pDebrisMesh->m_nFlags & CFDebrisMesh::FLAG_FLAT_OBJECT ) {
					FMATH_SETBITMASK( DebrisDef.m_nFlags, CFDebrisDef::FLAG_FLAT_OBJECT );
				}
			}

			if( m_nEmitterType == EMITTER_TYPE_BOUNDED_PLANE ) {
				fPlanePosX = fmath_RandomFloatRange( fNegHalfDimX, fHalfDimX );
				fPlanePosY = fmath_RandomFloatRange( fNegHalfDimY, fHalfDimY );

				DistVecX.Mul( m_Mtx.m_vX, fPlanePosX );
				DebrisDef.m_Pos_WS.Mul( m_Mtx.m_vY, fPlanePosY ).Add( DistVecX ).Add( m_Mtx.m_vPos );
			}
		} else {
			// Boned mesh...

			DebrisDef.m_pWorldMesh = pWorldMesh;
			DebrisDef.m_nBoneIndex = nBoneIndex;

			FMATH_SETBITMASK( DebrisDef.m_nFlags, CFDebrisDef::FLAG_USE_PROVIDED_WORLDMESH );
		}

		fSpeed = fmath_RandomFloatRange( m_fMinSpeed, m_fMaxSpeed );

		PerturbVec_WS.Set(
			fmath_RandomFloatRange( fNegPerturbFactor, fPerturbFactor ),
			fmath_RandomFloatRange( fNegPerturbFactor, fPerturbFactor ),
			fmath_RandomFloatRange( fNegPerturbFactor, fPerturbFactor )
		);

		DebrisDef.m_LinVel_WS.Add( m_Mtx.m_vZ, PerturbVec_WS ).Unitize().Mul( fSpeed );

		if( bRotationalMotion ) {
			DebrisDef.m_UnitRotAxis_WS.SetUnitRandom();
			DebrisDef.m_fRotSpeed = fmath_RandomFloatRange( m_pDebrisGroup->m_fMinRotSpeed, m_pDebrisGroup->m_fMaxRotSpeed ) * m_fRotSpeedMul;
		}

		DebrisDef.m_fGravity = fmath_RandomFloatRange( m_pDebrisGroup->m_fMinGravity, m_pDebrisGroup->m_fMaxGravity ) * m_fGravityMul;

		if( CFDebris::Spawn( &DebrisDef ) ) {
			// Spawned successfully...
			bParticleSpawned = TRUE;
		}
	}

	return bParticleSpawned;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisMeshSet
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CFDebrisMeshSet::m_bModuleStartedUp;
FLinkRoot_t CFDebrisMeshSet::m_LinkRoot;

const FGameData_TableEntry_t CFDebrisMeshSet::m_aGameDataVocab[] = {
	// m_pMesh:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_MESH,
	sizeof( FMesh_t * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// m_fScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// m_bFlatObject:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// m_bCanKickUpDust:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// m_bCanDamage:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// m_bOverrideableTint:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// m_fTintUnitRed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// m_fTintUnitGreen:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// m_fTintUnitBlue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


BOOL CFDebrisMeshSet::ModuleStartup( void ) {
	FASSERT( !IsModuleStartedUp() );

	flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CFDebrisMeshSet, m_Link ) );

	m_bModuleStartedUp = TRUE;

	return TRUE;
}


void CFDebrisMeshSet::ModuleShutdown( void ) {
	if( IsModuleStartedUp() ) {
		m_bModuleStartedUp = FALSE;
	}
}


CFDebrisMeshSet *CFDebrisMeshSet::Find( cchar *pszMeshSetName ) {
	FASSERT( IsModuleStartedUp() );

	CFDebrisMeshSet *pMeshSet;

	for( pMeshSet=(CFDebrisMeshSet *)flinklist_GetHead(&m_LinkRoot); pMeshSet; pMeshSet=(CFDebrisMeshSet *)flinklist_GetNext(&m_LinkRoot, pMeshSet) ) {
		if( !fclib_stricmp( pszMeshSetName, pMeshSet->m_pszMeshSetName ) ) {
			// Found the mesh set...
			return pMeshSet;
		}
	}

	// Mesh set not found...
	return NULL;
}


// The array must be terminated by setting CFDebrisMesh::m_pMesh to NULL.
u32 CFDebrisMeshSet::CountDebrisMesh( const CFDebrisMesh *pDebrisMeshArray ) {
	FASSERT( IsModuleStartedUp() );

	u32 nDebrisMeshCount;

	for( nDebrisMeshCount=0; pDebrisMeshArray[nDebrisMeshCount].m_pMesh; ++nDebrisMeshCount ) {}

	return nDebrisMeshCount;
}


// Note that pDebrisMeshArray will be copied to permanent memory.
CFDebrisMeshSet *CFDebrisMeshSet::Build( cchar *pszMeshSetName, const CFDebrisMeshUncompressed *pUncompressedDebrisMeshArray, u32 nDebrisMeshCount ) {
	FASSERT( IsModuleStartedUp() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( pszMeshSetName && Find( pszMeshSetName ) ) {
		DEVPRINTF( "CFDebrisMeshSet::Build(): Mesh set '%s' redefinition.\n", pszMeshSetName );
		goto _ExitWithError;
	}

	CFDebrisMesh *pDebrisMesh;
	CFDebrisMeshSet *pMeshSet;
	u32 i, nResByteCount;

	nResByteCount = sizeof(CFDebrisMeshSet) + sizeof(CFDebrisMesh)*nDebrisMeshCount;

	// Create a resource...
	pMeshSet = (CFDebrisMeshSet *)fres_CreateAndAlloc( NULL, NULL, _ResDestroyedCallback, nResByteCount, NULL );
	if( pMeshSet == NULL ) {
		if( pszMeshSetName ) {
			DEVPRINTF( "CFDebrisMeshSet::Build(): Not enough memory to allocate mesh set '%s'.\n", pszMeshSetName );
		} else {
			DEVPRINTF( "CFDebrisMeshSet::Build(): Not enough memory to allocate mesh set '<unnamed>'.\n" );
		}

		goto _ExitWithError;
	}

	// We've successfully created our mesh set resource!

	pMeshSet->m_pDebrisMeshArray = (CFDebrisMesh *)( (u8 *)pMeshSet + sizeof(CFDebrisMeshSet) );

	for( i=0; i<nDebrisMeshCount; ++i ) {
		pDebrisMesh = (CFDebrisMesh *)&pMeshSet->m_pDebrisMeshArray[i];

		pDebrisMesh->m_pMesh = pUncompressedDebrisMeshArray[i].m_pMesh;
		pDebrisMesh->m_fScale = pUncompressedDebrisMeshArray[i].m_fScale;

		pDebrisMesh->m_nFlags = CFDebrisMesh::FLAG_NONE;

		if( pUncompressedDebrisMeshArray[i].m_bFlatObject ) {
			FMATH_SETBITMASK( pDebrisMesh->m_nFlags, CFDebrisMesh::FLAG_FLAT_OBJECT );
		}

		if( pUncompressedDebrisMeshArray[i].m_bCanKickUpDust ) {
			FMATH_SETBITMASK( pDebrisMesh->m_nFlags, CFDebrisMesh::FLAG_CAN_KICKUP_DUST );
		}

		if( pUncompressedDebrisMeshArray[i].m_bCanDamage ) {
			FMATH_SETBITMASK( pDebrisMesh->m_nFlags, CFDebrisMesh::FLAG_CAN_DAMAGE );
		}

		if( pUncompressedDebrisMeshArray[i].m_bOverrideableTint ) {
			FMATH_SETBITMASK( pDebrisMesh->m_nFlags, CFDebrisMesh::FLAG_OVERRIDEABLE_TINT );
		}

		pDebrisMesh->m_nTintRed = (u8)(255.0f * pUncompressedDebrisMeshArray[i].m_fTintUnitRed);
		pDebrisMesh->m_nTintGreen = (u8)(255.0f * pUncompressedDebrisMeshArray[i].m_fTintUnitGreen);
		pDebrisMesh->m_nTintBlue = (u8)(255.0f * pUncompressedDebrisMeshArray[i].m_fTintUnitBlue);

		if( (pDebrisMesh->m_nTintRed & pDebrisMesh->m_nTintGreen & pDebrisMesh->m_nTintBlue) == 255 ) {
			// Tint color is white...

			FMATH_SETBITMASK( pDebrisMesh->m_nFlags, CFDebrisMesh::FLAG_TINT_IS_WHITE );
		}
	}

	// Add to the end of our linklist...
	flinklist_AddTail( &m_LinkRoot, pMeshSet );

	if( pszMeshSetName ) {
		pMeshSet->m_pszMeshSetName = CFStringTable::AddString( NULL, pszMeshSetName );
		if( pMeshSet->m_pszMeshSetName == NULL ) {
			DEVPRINTF( "CFDebrisMeshSet::Build(): Not enough memory to store name for mesh set '%s'.\n", pszMeshSetName );
			goto _ExitWithError;
		}
	} else {
		pMeshSet->m_pszMeshSetName = "<unnamed>";
	}

	pMeshSet->m_nDebrisMeshCount = nDebrisMeshCount;

	return pMeshSet;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


CFDebrisMeshSet *CFDebrisMeshSet::LoadFromGameData( FGameDataTableHandle_t hTable ) {
	u32 nTableFieldCount, nArrayElementCount, nStructElementCount;
	cchar *pszMeshSetName;
	CFDebrisMeshUncompressed *pUncompressedDebrisMeshArray;
	CFDebrisMeshSet *pMeshSet;

	FASSERT( IsModuleStartedUp() );

	pszMeshSetName = fgamedata_GetTableName( hTable );

	pMeshSet = Find( pszMeshSetName );
	if( pMeshSet ) {
		// Already exists...
		return pMeshSet;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();

	nStructElementCount = sizeof( m_aGameDataVocab ) / sizeof( FGameData_TableEntry_t );
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;

	nTableFieldCount = fgamedata_GetNumFields( hTable );
	if( (nTableFieldCount % nStructElementCount) ) {
		DEVPRINTF( "CFDebrisMeshSet::LoadFromGameData(): Invalid number of fields in '%s.csv'. Must be a multiple of %u.\n", pszMeshSetName, nStructElementCount );
		goto _ExitWithError;
	}

	nArrayElementCount = nTableFieldCount / nStructElementCount;

	// Load uncompressed debris mesh array into temp memory...
	pUncompressedDebrisMeshArray = (CFDebrisMeshUncompressed *)fmem_Alloc( sizeof(CFDebrisMeshUncompressed) * nArrayElementCount );
	if( pUncompressedDebrisMeshArray == NULL ) {
		DEVPRINTF( "CFDebrisMeshSet::LoadFromGameData(): Not enough memory to allocate debris meshes for '%s.csv'.\n", pszMeshSetName );
		goto _ExitWithError;
	}

	if( !fgamedata_GetTableData_ArrayOfStructures( hTable, m_aGameDataVocab, pUncompressedDebrisMeshArray, sizeof(CFDebrisMeshUncompressed), nArrayElementCount ) ) {
		DEVPRINTF( "CFDebrisMeshSet::LoadFromGameData(): Trouble parsing debris mesh data for '%s.csv'.\n", pszMeshSetName );
		goto _ExitWithError;
	}

	pMeshSet = Build( pszMeshSetName, pUncompressedDebrisMeshArray, nArrayElementCount );

	if( pMeshSet == NULL ) {
		goto _ExitWithError;
	}

	return pMeshSet;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	return NULL;
}


// Loads one debris mesh set from the file of the same name.
CFDebrisMeshSet *CFDebrisMeshSet::LoadFromGameData( cchar *pszFileName, cchar *pszMeshSetName ) {
	FASSERT( IsModuleStartedUp() );

	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	CFDebrisMeshSet *pMeshSet;

	pMeshSet = Find( pszMeshSetName );
	if( pMeshSet ) {
		// Already exists...
		return pMeshSet;
	}

	FMemFrame_t MemFrame = fmem_GetFrame();

	hFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFDebrisMeshSet::LoadFromGameData(): Could not load debris mesh set file '%s.csv'.\n", pszFileName );
		return FALSE;
	}

	pMeshSet = NULL;

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		if( !fclib_stricmp( fgamedata_GetTableName( hTable ), pszMeshSetName ) ) {
			// Found table...

			pMeshSet = LoadFromGameData( hTable );

			break;
		}
	}

	if( pMeshSet == NULL ) {
		DEVPRINTF( "CFDebrisMeshSet::LoadFromGameData(): Could not locate mesh set '%s' in file '%s.csv'.\n", pszMeshSetName, pszFileName );
	}

	fmem_ReleaseFrame( MemFrame );

	return pMeshSet;
}


// Loads an entire set of debris mesh sets from a single file.
// Returns TRUE if all were loaded successfully.
// If an error is encountered, this function will continue to try and
// load the other debris mesh sets from the file.
BOOL CFDebrisMeshSet::LoadAllFromGameData( cchar *pszFileName ) {
	FASSERT( IsModuleStartedUp() );

	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	BOOL bSuccess;

	FMemFrame_t MemFrame = fmem_GetFrame();

	bSuccess = TRUE;

	hFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFDebrisMeshSet::LoadAllFromGameData(): Could not load debris mesh set file '%s.csv'.\n", pszFileName );
		return FALSE;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		if( !LoadFromGameData( hTable ) ) {
			bSuccess = FALSE;
		}
	}

	fmem_ReleaseFrame( MemFrame );

	return bSuccess;
}


void CFDebrisMeshSet::_ResDestroyedCallback( void *pResMem ) {
	CFDebrisMeshSet *pMeshSet = (CFDebrisMeshSet *)pResMem;

	FASSERT( pMeshSet == (CFDebrisMeshSet *)flinklist_GetStructurePointer( &m_LinkRoot, m_LinkRoot.pTailLink ) );

	flinklist_Remove( &m_LinkRoot, pMeshSet );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFDebrisDef
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CFDebrisDef::m_bModuleStartedUp;
CFDebrisDef CFDebrisDef::m_DefaultDebrisDef;


BOOL CFDebrisDef::ModuleStartup( void ) {
	FASSERT( !IsModuleStartedUp() );

	m_DefaultDebrisDef.m_nFlags = FLAG_NONE;
	m_DefaultDebrisDef.m_nPriority = 0;
	m_DefaultDebrisDef.m_nBoneIndex = -1;
	m_DefaultDebrisDef.m_pFcnCallback = NULL;
	m_DefaultDebrisDef.m_pUser = NULL;
	m_DefaultDebrisDef.m_nUser = 0;
	m_DefaultDebrisDef.m_nOwnerID = 0;
	m_DefaultDebrisDef.m_Pos_WS.Zero();
	m_DefaultDebrisDef.m_LinVel_WS.Zero();
	m_DefaultDebrisDef.m_UnitRotAxis_WS = CFVec3A::m_UnitAxisX;
	m_DefaultDebrisDef.m_fRotSpeed = 0.0f;
	m_DefaultDebrisDef.m_nCollType = COLL_TYPE_NONE;
	m_DefaultDebrisDef.m_nFadeType = FADE_TYPE_SHRINK;
	m_DefaultDebrisDef.m_nLightingType = LIGHTING_TYPE_AMBIENT_ONLY;
	m_DefaultDebrisDef.m_nTintRed = 255;
	m_DefaultDebrisDef.m_nTintGreen = 255;
	m_DefaultDebrisDef.m_nTintBlue = 255;
	m_DefaultDebrisDef.m_pMesh = NULL;
	m_DefaultDebrisDef.m_pWorldMesh = NULL;
	m_DefaultDebrisDef.m_fMeshScale = 1.0f;
	m_DefaultDebrisDef.m_fMeshUnitOpacity = 1.0f;
	m_DefaultDebrisDef.m_fGravity = -32.0f;
	m_DefaultDebrisDef.m_fMinSpeedToStartFade2 = (2.0f * 2.0f);
	m_DefaultDebrisDef.m_fSecsBeforeMinSpeedValid = 1.0f;
	m_DefaultDebrisDef.m_fSecsUnderMinSpeedToStartFade = 5.0f;
	m_DefaultDebrisDef.m_fAliveSecs = 20.0f;
	m_DefaultDebrisDef.m_fFadeSecs = 5.0f;
	m_DefaultDebrisDef.m_fCullDist = 300.0f;
	m_DefaultDebrisDef.m_fUnitDustKickUp = 0.0f;
	m_DefaultDebrisDef.m_fUnitHitpointDamage = 0.0f;
	m_DefaultDebrisDef.m_pSoundGroupImpact = NULL;
	m_DefaultDebrisDef.m_pSoundGroupFlaming = NULL;
	m_DefaultDebrisDef.m_pSmokeTrailAttrib = NULL;
	m_DefaultDebrisDef.m_hParticleDef1 = FPARTICLE_INVALID_HANDLE;
	m_DefaultDebrisDef.m_hParticleDef2 = FPARTICLE_INVALID_HANDLE;

	m_bModuleStartedUp = TRUE;

	return TRUE;
}


void CFDebrisDef::ModuleShutdown( void ) {
	if( IsModuleStartedUp() ) {
		m_bModuleStartedUp = FALSE;
	}
}




