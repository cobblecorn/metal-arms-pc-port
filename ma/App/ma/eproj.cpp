//////////////////////////////////////////////////////////////////////////////////////
// eproj.cpp - Projectile entity class.
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
// 06/10/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eproj.h"
#include "entity.h"
#include "flinklist.h"
#include "fmesh.h"
#include "fworld.h"
#include "fresload.h"
#include "fres.h"
#include "meshtypes.h"
#include "fcoll.h"
#include "floop.h"
#include "eproj_swarmer.h"
#include "eproj_grenade.h"
#include "eproj_arrow.h"
#include "eproj_saw.h"
#include "eproj_cleaner.h"
#include "eproj_slower.h"
#include "eproj_mortar.h"
#include "damage.h"
#include "LightPool.h"
#include "fsound.h"








//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProjBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEProjBuilder _EProjBuilder;


void CEProjBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_PROJECTILE, pszEntityType );

	m_pszEC_ArmorProfile = "Projectile";
	m_nProjType = CEProj::PROJTYPE_COUNT;
	m_pszMeshResName = NULL;
	m_pWorldMesh = NULL;
}


BOOL CEProjBuilder::InterpretTable( void ) {
	return CEntityBuilder::InterpretTable();
}


BOOL CEProjBuilder::PostInterpretFixup( void ) {
	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	// We don't currently support building of CEProj entities through the user properties...

	DEVPRINTF( "CEProjBuilder::PostInterpretFixup(): User-placed projectile entities are not currently supported.\n" );

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CFVec3A CEProj::m_CollRayStart_WS;
const CFMtx43A *CEProj::m_pCollUnitMtx;
f32 CEProj::m_fCollRadius;
f32 CEProj::m_fCollDeltaAngle;
f32 CEProj::m_fCollOverlapDistZ;
u32 CEProj::m_nCollPerimeterPointCount;
u16 CEProj::m_nCollMask;

static BOOL _bCollFound;



CEProj::CEProj() {
	_ClearDataMembers();
}


CEProj::~CEProj() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CEProj::Create( ProjType_e nProjType, CFWorldMesh *pWorldMesh, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( nProjType>=0 && nProjType<PROJTYPE_COUNT );

	// Get pointer to the leaf class's builder object...
	CEProjBuilder *pBuilder = (CEProjBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_EProjBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_LINE );
	}

	// Set our builder parameters...

	pBuilder->m_nProjType = nProjType;
	pBuilder->m_pWorldMesh = pWorldMesh;

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CEProj::Create( ProjType_e nProjType, cchar *pszMeshResName, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( nProjType>=0 && nProjType<PROJTYPE_COUNT );

	// Get pointer to the leaf class's builder object...
	CEProjBuilder *pBuilder = (CEProjBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_EProjBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_LINE );
	}

	// Set our builder parameters...

	pBuilder->m_nProjType = nProjType;
	pBuilder->m_pszMeshResName = pszMeshResName;

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}


CEntityBuilder *CEProj::GetLeafClassBuilder( void ) {
	return &_EProjBuilder;
}


BOOL CEProj::ClassHierarchyBuild( void ) {
	FMeshInit_t MeshInit;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEProjBuilder *pBuilder = (CEProjBuilder *)GetLeafClassBuilder();
	FASSERT( pBuilder->m_pszMeshResName==NULL || pBuilder->m_pWorldMesh==NULL );
	FASSERT( pBuilder->m_pszMeshResName!=NULL || pBuilder->m_pWorldMesh!=NULL );

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Initialize from builder object...

	_ClearDataMembers();

	m_nProjType = pBuilder->m_nProjType;

	if( pBuilder->m_pszMeshResName ) {
		// Build from mesh resource name...

		// Load mesh resource...
		FMesh_t *pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pBuilder->m_pszMeshResName );
		if( pMesh == NULL ) {
			goto _ExitWithError;
		}

		// Create world mesh...
		m_pWorldMesh = fnew CFWorldMesh;
		if( m_pWorldMesh == NULL ) {
			DEVPRINTF( "CEProj::ClassHierarchyBuild(): Not enough memory to allocate a new projectile.\n" );
			goto _ExitWithError;
		}

		// Init world mesh...
		MeshInit.pMesh = pMesh;
		MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist = 500.0f;
		MeshInit.Mtx = *MtxToWorld();
		m_pWorldMesh->Init( &MeshInit );
		m_pWorldMesh->RemoveFromWorld();
		m_pWorldMesh->SetCollisionFlag( FALSE );
		m_pWorldMesh->SetUserTypeBits ( TypeBits());
		m_pWorldMesh->SetLineOfSightFlag( FALSE );
		m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
		m_pWorldMesh->m_pUser = this;
		m_pWorldMesh->SetUserTypeBits( TypeBits() );
	} else {
		// Build from existing world mesh...
		m_pWorldMesh = pBuilder->m_pWorldMesh;
	}

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CEProj::ClassHierarchyBuilt( void ) {
	FResFrame_t ResFrame = fres_GetFrame();

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	DisableOurWorkBit();
	RemoveFromWorld();

	// Allocate projectile extension class...
	switch( m_nProjType ) {
	case PROJTYPE_ARROW:
		m_pProjExt = fnew CEProj_Arrow;
		break;

	case PROJTYPE_SWARMER:
		m_pProjExt = fnew CEProj_Swarmer;
		break;

	case PROJTYPE_GRENADE:
		m_pProjExt = fnew CEProj_Grenade;
		break;

	case PROJTYPE_SAW:
		m_pProjExt = fnew CEProj_Saw;
		break;

	case PROJTYPE_CLEANER:
		m_pProjExt = fnew CEProj_Cleaner;
		break;

	case PROJTYPE_SLOWER:
		m_pProjExt = fnew CEProj_Slower;
		break;

	case PROJTYPE_MORTAR:
		m_pProjExt = fnew CEProj_Mortar;
		break;

	default:
		FASSERT_NOW;
	}

	if( m_pProjExt == NULL ) {
		DEVPRINTF( "CEProj::ClassHierarchyBuilt(): Not enough memory to allocate projectile extension class.\n" );
		goto _ExitWithError;
	}

	// Init projectile extension class...
	if( !m_pProjExt->Create( this ) ) {
		DEVPRINTF( "CEProj::ClassHierarchyBuilt(): Could not create projectile extension class.\n" );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEProj::ClassHierarchyDestroy( void ) {
	_KillLoopingSound();
	_KillStreamer();
	_KillSmokeTrail();
	_KillAttachedLight();
	_KillAttachedLight2();

	if( m_pProjExt ) {
		m_pProjExt->Destroy( this );
		fdelete( m_pProjExt );
	}

	if( m_pProjPool ) {
		CEProjPool::RemoveProjectileFromAllPools( this );
	}

	_ClearDataMembers();

	CEntity::ClassHierarchyDestroy();
}


void CEProj::_ClearDataMembers( void ) {
	m_nProjType = PROJTYPE_COUNT;
	m_pProjExt = NULL;
	m_nProjFlags = PROJFLAG_DESTROYED;
	m_pProjPool = NULL;
	m_pFcnDamageCallback = NULL;
	m_pFcnDeathCallback = NULL;
	m_pFcnDetonateCallback = NULL;
	m_pFcnHitGeoCallback = NULL;
	m_pFcnBuildSkipListCallback = NULL;
	m_pWorldMesh = NULL;
	m_Damager.pEntity = this;
	m_Damager.pWeapon = NULL;
	m_Damager.pBot = NULL;
	m_Damager.nDamagerPlayerIndex = -1;
	m_pDamageProfile = NULL;
	m_fMaxDistCanTravel_WS = 100.0f;
	m_fDistTraveled_WS = 0.0f;
	m_fMaxLifeSecs = -1.0f;
	m_fLifeSecs = 0.0f;
	m_fLinSpeed_WS = 0.0f;
	m_fRotSpeed_WS = 0.0f;
	m_LinUnitDir_WS = CFVec3A::m_UnitAxisZ;
	m_RotUnitAxis_WS = CFVec3A::m_UnitAxisZ;
	m_SmokePos_MS.Zero();
	m_hSmokeTrail = SMOKETRAIL_NULLHANDLE;
	m_pLoopingSoundGroup = NULL;
	m_pLoopingAudioEmitter = NULL;
	m_pSmokeAttrib = NULL;
	m_hStreamer = FXSTREAMER_INVALID_HANDLE;
	m_pStreamerTexInst = NULL;
	m_nStreamerVtxCount = 5;
	m_fStreamerAlpha = 1.0f;
	m_fStreamerWidth = 1.0f;
	m_fStreamerSamplesPerSec = 15.0f;
	m_nStreamerAxis = CFXStreamerEmitter::USE_RIGHT_AXIS;
	m_pTargetedEntity = NULL;
	m_pLightInit = NULL;
	m_pWorldLightItem = NULL;
	m_pWorldLightItem2 = NULL;
	m_hExplosion = FEXPLOSION_INVALID_HANDLE;
}


void CEProj::_UpdateXfm( void ) {
	if( !(m_nProjFlags & PROJFLAG_MANUAL_XFM) && m_pWorldMesh ) {
		if( m_fScaleToWorld == 1.0f ) {
			m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
		} else {
			m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		}

		m_pWorldMesh->UpdateTracker();
	}
}


void CEProj::ClassHierarchyRelocated( void *pIdentifier ) {
	CEntity::ClassHierarchyRelocated( pIdentifier );

	if( pIdentifier != GetRelocateIdentifier() ) {
		// Ah, we were not responsible for this relocation...

		if( !IsInWorld() ) {
			// We're not in the world...
			return;
		}

		_UpdateXfm();
	}
}


void CEProj::ClassHierarchyAddToWorld( void ) {
	CEntity::ClassHierarchyAddToWorld();

	if( !(m_nProjFlags & PROJFLAG_DESTROYED) ) {
		_UpdateXfm();

		if( m_nProjFlags & PROJFLAG_LAUNCHED ) {
			FMATH_CLEARBITMASK( m_nProjFlags, PROJFLAG_LAUNCHED );

			m_pProjExt->Frozen( this );
		}
	}

	DisableOurWorkBit();
}


void CEProj::ClassHierarchyRemoveFromWorld( void ) {
	if( m_pWorldMesh ) {
		m_pWorldMesh->RemoveFromWorld();
	}

	CEntity::ClassHierarchyRemoveFromWorld();
}



void CEProj::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CEntity::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this mesh...

		if( m_pWorldMesh ) {
			FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
	} else {
		// Disable drawing of this mesh...

		if( m_pWorldMesh ) {
			FMATH_SETBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
	}

	m_pProjExt->DrawEnable( bDrawingHasBeenEnabled );
}


void CEProj::LoopingSoundOn( CFSoundGroup *pLoopingSoundGroup ) {
	FASSERT( IsCreated() );

	_KillLoopingSound();

	if( pLoopingSoundGroup ) {
		m_pLoopingSoundGroup = pLoopingSoundGroup;
	}
}


void CEProj::SmokeTrailOn( SmokeTrailAttrib_t *pSmokeTrailAttributes ) {
	FASSERT( IsCreated() );

	_KillSmokeTrail();

	if( pSmokeTrailAttributes ) {
		m_pSmokeAttrib = pSmokeTrailAttributes;

		FMATH_SETBITMASK( m_nProjFlags, PROJFLAG_CREATE_SMOKETRAIL );
	}
}


void CEProj::StreamerOn( CFTexInst *pTexInst, f32 fAlpha, CFXStreamerEmitter::UseAxis_e nAxis, f32 fWidth, u32 nVtxCount, f32 fSamplesPerSec ) {
	FASSERT( IsCreated() );

	_KillStreamer();

	m_pStreamerTexInst = pTexInst;
	m_nStreamerVtxCount = nVtxCount;
	m_fStreamerAlpha = fAlpha;
	m_fStreamerWidth = fWidth;
	m_fStreamerSamplesPerSec = fSamplesPerSec;
	m_nStreamerAxis = nAxis;
}


void CEProj::SetAttachedLight( CFWorldLightItem *pWorldLightItem ) {
	FASSERT( IsCreated() );

	_KillAttachedLight();

	m_pWorldLightItem = pWorldLightItem;
}


void CEProj::SetAttachedLight2( CFWorldLightItem *pWorldLightItem ) {
	FASSERT( IsCreated() );

	_KillAttachedLight2();

	m_pWorldLightItem2 = pWorldLightItem;
}


void CEProj::Init( void ) {
	FASSERT( IsCreated() );

	FMATH_CLEARBITMASK( m_nProjFlags, (PROJFLAG_LAUNCHED | PROJFLAG_DESTROYED | PROJFLAG_MANUAL_XFM | PROJFLAG_THIN_PROJECTILE | PROJFLAG_DETONATE_ON_IMPACT | PROJFLAG_CREATE_SMOKETRAIL) );

	m_pProjExt->Init( this );
	DisableOurWorkBit();

	m_pFcnDamageCallback = NULL;
	m_pFcnDeathCallback = NULL;
	m_pFcnDetonateCallback = NULL;
	m_pFcnHitGeoCallback = NULL;
	m_pFcnBuildSkipListCallback = NULL;
	m_Damager.pEntity = this;
	m_Damager.pWeapon = NULL;
	m_Damager.pBot = NULL;
	m_Damager.nDamagerPlayerIndex = -1;
	m_pDamageProfile = NULL;
	m_fMaxDistCanTravel_WS = 100.0f;
	m_fDistTraveled_WS = 0.0f;
	m_fMaxLifeSecs = -1.0f;
	m_fLifeSecs = 0.0f;
	m_fLinSpeed_WS = 0.0f;
	m_fRotSpeed_WS = 0.0f;
	m_LinUnitDir_WS = CFVec3A::m_UnitAxisZ;
	m_RotUnitAxis_WS = CFVec3A::m_UnitAxisZ;
	m_SmokePos_MS.Zero();
	m_hExplosion = FEXPLOSION_INVALID_HANDLE;
	m_pTargetedEntity = NULL;

	_KillLoopingSound();
	_KillStreamer();
	_KillSmokeTrail();
	_KillAttachedLight();
	_KillAttachedLight2();

	SetCollisionFlag( FALSE );

	DrawEnable( TRUE );

	RemoveFromWorld();
}


void CEProj::Launch( void ) {
	FASSERT( IsCreated() );

	if( !(m_nProjFlags & PROJFLAG_DESTROYED) ) {
		AddToWorld();

		FMATH_SETBITMASK( m_nProjFlags, PROJFLAG_LAUNCHED );

		EnableOurWorkBit();
		m_pProjExt->Launched( this );
	}
}


void CEProj::Freeze( void ) {
	FASSERT( IsCreated() );

	if( (m_nProjFlags & (PROJFLAG_LAUNCHED | PROJFLAG_DESTROYED)) == PROJFLAG_DESTROYED ) {
		FMATH_CLEARBITMASK( m_nProjFlags, PROJFLAG_LAUNCHED );

		m_pProjExt->Frozen( this );
		DisableOurWorkBit();
	}

	AddToWorld();
}


// Detonates the projectile without asking permission from any callback.
void CEProj::Detonate( BOOL bMakeEffect, Event_e nEvent, FCollImpact_t *pCollImpact ) {
	FASSERT( IsCreated() );

	if( m_pFcnDetonateCallback && !m_pFcnDetonateCallback( this, bMakeEffect, nEvent, pCollImpact ) ) {
		// We should not make our default detonation effect...
		bMakeEffect = FALSE;
	}

	if( !m_pProjExt->Detonated( this, bMakeEffect, nEvent, pCollImpact ) ) {
		// We should not make our default detonation effect...
		bMakeEffect = FALSE;
	}

	if( bMakeEffect ) {
		SpawnExplosionEffect( nEvent, pCollImpact );
	}

	_DestroyProjectile();
}


void CEProj::SpawnExplosionEffect( Event_e nEvent, const FCollImpact_t *pImpact ) {
	if( m_hExplosion == FEXPLOSION_INVALID_HANDLE ) {
		return;
	}

	BOOL bMakeExplosion, bImpactExplosion;

	bMakeExplosion = FALSE;
	bImpactExplosion = FALSE;

	switch( nEvent ) {
	case EVENT_HIT_GEO:
		if( pImpact ) {
			bMakeExplosion = TRUE;
			bImpactExplosion = TRUE;
			break;
		}

		// Fall into...

	case EVENT_REACHED_MAX_DIST:
	case EVENT_LIFE_TIME_OVER:
	case EVENT_DETONATE_API:
		bMakeExplosion = TRUE;
		break;
	};

	if( bMakeExplosion ) {
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();

			SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
			SpawnParams.pDamageProfile = GetDamageProfile();
			SpawnParams.pDamager = GetDamager();

			if( bImpactExplosion ) {
				SpawnParams.Pos_WS = m_MtxToWorld.m_vPos;
				SpawnParams.UnitDir = pImpact->UnitFaceNormal;
				SpawnParams.uSurfaceType = pImpact->nUserType;

				CFWorldMesh *pImpactedWorldMesh = (CFWorldMesh *)pImpact->pTag;

				if( pImpactedWorldMesh == NULL ) {
					CExplosion2::SpawnExplosion( hSpawner, m_hExplosion, &SpawnParams );
				} else {
					CDamageForm::TriData_t DamageImpactTriData;

					DamageImpactTriData.ImpactUnitNormal_WS = pImpact->UnitFaceNormal;
					DamageImpactTriData.aTriVtx_WS[0] = pImpact->aTriVtx[0];
					DamageImpactTriData.aTriVtx_WS[1] = pImpact->aTriVtx[1];
					DamageImpactTriData.aTriVtx_WS[2] = pImpact->aTriVtx[2];
					DamageImpactTriData.nDamageeBoneIndex = pImpact->nBoneIndex;
					DamageImpactTriData.pDamageeWorldMesh = pImpactedWorldMesh;
					DamageImpactTriData.eSurfaceType = CGColl::GetSurfaceType( pImpact );

					CExplosion2::SpawnExplosion( hSpawner, m_hExplosion, &SpawnParams, &DamageImpactTriData );
				}
			} else {
				SpawnParams.Pos_WS = m_MtxToWorld.m_vPos;
				SpawnParams.UnitDir.Zero();
				SpawnParams.uSurfaceType = 0;

				CExplosion2::SpawnExplosion( hSpawner, m_hExplosion, &SpawnParams );
			}
		}
	}
}


void CEProj::ClassHierarchyWork() {
	CEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	FASSERT( m_nProjFlags & PROJFLAG_LAUNCHED );

	// Start a looping sound...
	if( m_pLoopingSoundGroup && !m_pLoopingAudioEmitter ) {
		m_pLoopingAudioEmitter = AllocAndPlaySound( m_pLoopingSoundGroup );

		if( m_pLoopingAudioEmitter ) {
			m_pLoopingAudioEmitter->SetDopplerFactor( 2.0f );
		}
	}

	// Start a smoke trail...
	if( m_nProjFlags & PROJFLAG_CREATE_SMOKETRAIL ) {
		if( m_hSmokeTrail == SMOKETRAIL_NULLHANDLE ) {
			if( m_pSmokeAttrib ) {
				m_hSmokeTrail = smoketrail_GetFromFreePoolAndSetAttributes( m_pSmokeAttrib );

				if( m_hSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
					CFVec3A SmokePos_WS;
					m_MtxToWorld.MulPoint( SmokePos_WS, m_SmokePos_MS );
					smoketrail_Puff( m_hSmokeTrail, &SmokePos_WS );

					FMATH_CLEARBITMASK( m_nProjFlags, PROJFLAG_CREATE_SMOKETRAIL );
				}
			}
		}
	}

	// Start a streamer...
	if( m_pStreamerTexInst ) {
		// Enable streamer...
		_StartStreamer();
	}

	// Build tracker skip list...
	FWorld_nTrackerSkipListCount = 0;
	if( m_pFcnBuildSkipListCallback ) {
		m_pFcnBuildSkipListCallback( this );
	}

	FASSERT( FWorld_nTrackerSkipListCount < FWORLD_MAX_SKIPLIST_ENTRIES );
	if( m_pWorldMesh ) {
		FWorld_apTrackerSkipList[ FWorld_nTrackerSkipListCount++ ] = m_pWorldMesh;
	}

	// Move projectile...
	if( m_pProjExt->Work( this ) ) {
		// Projectile has been destroyed...
		return;
	}

	// See if projectile has traveled past its maximum allowed distance...
	if( m_fDistTraveled_WS >= m_fMaxDistCanTravel_WS ) {
		// Projectile has traveled it's maximum allowed distance...

		Detonate( TRUE, EVENT_REACHED_MAX_DIST, NULL );
		return;
	}

	// Handle sound...
	if( m_pLoopingAudioEmitter ) {
		m_pLoopingAudioEmitter->SetPosition( &m_MtxToWorld.m_vPos );
	}

	// Handle smoke...
	if( m_hSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
		CFVec3A SmokePos_WS;
		m_MtxToWorld.MulPoint( SmokePos_WS, m_SmokePos_MS );
		smoketrail_Puff( m_hSmokeTrail, &SmokePos_WS );
	}

	// Handle attached lights...
	if( m_pWorldLightItem ) {
		m_pWorldLightItem->m_Light.SetPosition( &m_MtxToWorld.m_vPos );
		m_pWorldLightItem->UpdateTracker();
	}
	if( m_pWorldLightItem2 ) {
		m_pWorldLightItem2->m_Light.SetPosition( &m_MtxToWorld.m_vPos );
		m_pWorldLightItem2->UpdateTracker();
	}
}


void CEProj::_DestroyProjectile( void ) {
	if( !(m_nProjFlags & PROJFLAG_DESTROYED) ) {
		FMATH_SETBITMASK( m_nProjFlags, PROJFLAG_DESTROYED );
		FMATH_CLEARBITMASK( m_nProjFlags, PROJFLAG_LAUNCHED );

		_KillLoopingSound();
		_KillStreamer();
		_KillSmokeTrail();
		_KillAttachedLight();
		_KillAttachedLight2();

		if( m_pProjPool ) {
			// This projectile came from a pool. Return it...
			m_pProjPool->ReturnProjectileToFreePool( this );
		} else {
			// This projectile didn't come from a pool...
			RemoveFromWorld();
		}

		DisableOurWorkBit();
	}
}


void CEProj::_KillStreamer( void ) {
	if( m_hStreamer != FXSTREAMER_INVALID_HANDLE ) {
		// We currently have a streamer. Stop it from emitting...

		if( CFXStreamerEmitter::IsValidHandle( m_hStreamer ) ) {
			CFXStreamerEmitter::EnableStreamerEmission( m_hStreamer, FALSE );
		}

		m_hStreamer = FXSTREAMER_INVALID_HANDLE;
	}

	m_pStreamerTexInst = NULL;
}


void CEProj::_KillLoopingSound( void ) {
	if( m_pLoopingAudioEmitter ) {
		m_pLoopingAudioEmitter->Destroy();
		m_pLoopingAudioEmitter = NULL;
	}
}


void CEProj::_KillSmokeTrail( void ) {
	if( m_hSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
		// We currently have a smoke trail. Stop it from emitting...

		smoketrail_ReturnToFreePool( m_hSmokeTrail, TRUE );

		m_hSmokeTrail = SMOKETRAIL_NULLHANDLE;
	}

	m_pSmokeAttrib = NULL;
	FMATH_CLEARBITMASK( m_nProjFlags, PROJFLAG_CREATE_SMOKETRAIL );
}


void CEProj::_KillAttachedLight( void ) {
	CLightPool::ReturnToFreePool( m_pWorldLightItem );
	m_pWorldLightItem = NULL;
}


void CEProj::_KillAttachedLight2( void ) {
	CLightPool::ReturnToFreePool( m_pWorldLightItem2 );
	m_pWorldLightItem2 = NULL;
}


void CEProj::_StartStreamer( void ) {
	m_hStreamer = CFXStreamerEmitter::SpawnFromMtx43A(
		m_nStreamerVtxCount,
		&GetWorldMesh()->m_Xfm.m_MtxF,
		m_fStreamerAlpha,
		m_pStreamerTexInst,
		NULL,
		m_fStreamerWidth*GetWorldMesh()->m_Xfm.m_fScaleR,
		m_fStreamerSamplesPerSec,
		m_nStreamerAxis
	);

	m_pStreamerTexInst = NULL;

	EmitStreamerPoint();
}


void CEProj::EmitStreamerPoint( void ) {
	if( m_hStreamer != FXSTREAMER_INVALID_HANDLE ) {
		CFXStreamerEmitter::EmitPoint( m_hStreamer );
	}
}


void CEProj::PlaySound( CFSoundGroup *pSoundGroup ) {
	CFSoundGroup::PlaySound( pSoundGroup, FALSE, &MtxToWorld()->m_vPos );
}


CFAudioEmitter *CEProj::AllocAndPlaySound( CFSoundGroup *pSoundGroup ) {
	return CFSoundGroup::AllocAndPlaySound( pSoundGroup, FALSE, &MtxToWorld()->m_vPos );
}


BOOL CEProj::PerformThinRayCollisionTest( FCollImpact_t *pCollImpact, const CFVec3A *pPrevPos_WS, const CFMtx43A *pUnitMtx, BOOL bIgnoreWires, f32 fOverlapDistZ ) {
	m_CollRayStart_WS.Mul( pUnitMtx->m_vZ, -fOverlapDistZ ).Add( *pPrevPos_WS );

	if( fworld_FindClosestImpactPointToRayStart( pCollImpact, &m_CollRayStart_WS, &pUnitMtx->m_vPos, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
		// We hit something...

		return TRUE;
	}

	return FALSE;
}


// Performs a swept-sphere collision test against hurt-me entities, and a simple ray collision
// test for everything else.
BOOL CEProj::PerformThickRayCollisionTest( FCollImpact_t *pCollImpact, const CFVec3A *pPrevPos_WS, const CFMtx43A *pUnitMtx,
										   BOOL bIgnoreWires, f32 fOverlapDistZ, f32 fCollRadius ) {
	CFVec3A NewPos_WS;
	BOOL bFoundCollision = FALSE;

	m_CollRayStart_WS.Mul( pUnitMtx->m_vZ, -fOverlapDistZ ).Add( *pPrevPos_WS );

	if( fworld_FindClosestImpactPointToRayStart( pCollImpact, &m_CollRayStart_WS, &pUnitMtx->m_vPos, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES ) ) {
		// We hit world geo...

		bFoundCollision = TRUE;
		NewPos_WS = pCollImpact->ImpactPoint;
	} else {
		// We didn't hit any world geo...

		NewPos_WS = pUnitMtx->m_vPos;
	}

	CEProjPool::m_CollInfoFat.ProjSphere.Init( pPrevPos_WS, &NewPos_WS, fCollRadius );
	CEProjPool::m_bCollIgnoreWires = bIgnoreWires;

	CEProjPool::m_CollProjSphereInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	CEProjPool::m_CollProjSphereInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;

	fcoll_Clear();
	fworld_CollideWithTrackers( &CEProjPool::m_CollProjSphereInfo );

	if( FColl_nImpactCount ) {
		// We hit a hurt-me entity...

		bFoundCollision = TRUE;

		fang_MemCopy( pCollImpact, fcoll_FindClosest(), sizeof(FCollImpact_t) );
	}

	return bFoundCollision;
}


BOOL CEProj::_TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode ) {
#if 0
	u32 i;

	for( i=0; i<FWorld_nTrackerSkipListCount; ++i ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}
#endif

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
		// This tracker is an entity...

		CEntity *pEntity = (CEntity *)pTracker->m_pUser;

		if( pEntity->GetProjectileReaction() == CEntity::PROJECTILE_REACTION_HURT_ME ) {
			// Found a candidate for thick collision...

			CEProjPool::m_CollInfoFat.pTag = pTracker;
			((CFWorldMesh *)pTracker)->CollideWithMeshTris( &CEProjPool::m_CollInfoFat );
		}

		return TRUE;
	}

	if( pTracker->m_nUser == FWORLD_USERTYPE_WIRE ) {
		// This tracker is a wire...

		if( !CEProjPool::m_bCollIgnoreWires ) {
			// Don't ignore wires...

			CEProjPool::m_CollInfoFat.pTag = pTracker;
			((CFWorldMesh *)pTracker)->CollideWithMeshTris( &CEProjPool::m_CollInfoFat );
		}

		return TRUE;
	}

	return TRUE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProjPool
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CEProjPool::m_bSystemInitialized;		// TRUE: InitSystem() has been called
FLinkRoot_t CEProjPool::m_PoolLinkRoot;		// Linklist of all CEProjPool's

CFCollInfo CEProjPool::m_CollInfoFat;
CFTrackerCollideProjSphereInfo CEProjPool::m_CollProjSphereInfo;
BOOL CEProjPool::m_bCollIgnoreWires;


BOOL CEProjPool::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	_Pool_Init();
	fworld_RegisterWorldCallbackFunction( _WorldCallback );

	m_CollInfoFat.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfoFat.bFindClosestImpactOnly = TRUE;
	m_CollInfoFat.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	m_CollInfoFat.bCullBacksideCollisions = TRUE;
	m_CollInfoFat.bCalculateImpactData = TRUE;
	m_CollInfoFat.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfoFat.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfoFat.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;

	m_CollProjSphereInfo.pProjSphere = &m_CollInfoFat.ProjSphere;
	m_CollProjSphereInfo.bIgnoreCollisionFlag = FALSE;
	m_CollProjSphereInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	m_CollProjSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	m_CollProjSphereInfo.pCallback = CEProj::_TrackersCallbackProjSphere;

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CEProjPool::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		fworld_UnregisterWorldCallbackFunction( _WorldCallback );
		m_bSystemInitialized = FALSE;
	}
}


CEProjPool::CEProjPool() {
	m_pMesh = NULL;
	m_pProjArray = NULL;
	m_pWorldMeshArray = NULL;
	m_bAddedToPoolList = FALSE;
	m_bBuiltBeforeWorld = FALSE;
	m_bCreated = FALSE;
	m_uMeshInstFlags = FMESHINST_FLAG_NONE;
}


CEProjPool::~CEProjPool() {
	_Destroy();
}


CEProjPool::PoolHandle_t CEProjPool::Create( CEProj::ProjType_e nProjType, cchar *pszMeshResName, u32 nProjCount, u32 m_uFlags ) {
	PoolHandle_t hPool;

	FASSERT( IsSystemInitialized() );
	FASSERT( nProjCount > 0 );

	FResFrame_t ResFrame = fres_GetFrame();

	// Load the mesh resource...
	FMesh_t *pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshResName );
	if( pMesh == NULL ) {
		DEVPRINTF( "CEProjPool::Create(): Could not load the projectile mesh '%s'.\n", pszMeshResName );
		goto _ExitWithError;
	}

	// Create the pool...
	hPool = Create( nProjType, pMesh, nProjCount, m_uFlags );
	if( hPool == EPROJPOOL_NULL_HANDLE ) {
		goto _ExitWithError;
	}

	// Success...

	return hPool;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return EPROJPOOL_NULL_HANDLE;
}


CEProjPool::PoolHandle_t CEProjPool::Create( CEProj::ProjType_e nProjType, FMesh_t *pMeshRes, u32 nProjCount, u32 m_uFlags ) {
	CEProjPool *pProjPool;

	FASSERT( IsSystemInitialized() );
	FASSERT( nProjCount > 0 );
	FASSERT( nProjType>=0 && nProjType<CEProj::PROJTYPE_COUNT );

	FResFrame_t ResFrame = fres_GetFrame();

	// Create an fres resource...
	FResHandle_t hRes = fres_CreateWithCallback( NULL, NULL, _ResDestroyedCallback );
	if( hRes == FRES_NULLHANDLE ) {
		DEVPRINTF( "CEProjPool::Create(): Could not create projectile resource object.\n" );
		goto _ExitWithError;
	}

	// Create a new projectile pool...
	pProjPool = fnew CEProjPool;
	if( pProjPool == NULL ) {
		DEVPRINTF( "CEProjPool::Create(): Not enough memory to create projectile pool.\n" );
		goto _ExitWithError;
	}

	// Set resource base pointer...
	fres_SetBase( hRes, pProjPool );

	// Init new pool object...
	pProjPool->_FreeProj_Init();
	pProjPool->_ActiveProj_Init();
	pProjPool->m_pMesh = pMeshRes;
	pProjPool->m_uMeshInstFlags = m_uFlags;
	pProjPool->m_nProjCount = nProjCount;
	pProjPool->m_nProjType = nProjType;

	if( FWorld_pWorld ) {
		// The world is loaded...

		if( !pProjPool->_AllocatePoolArrays() ) {
			goto _ExitWithError;
		}

		pProjPool->m_bCreated = TRUE;
	} else {
		// The world is not loaded...

		pProjPool->m_bBuiltBeforeWorld = TRUE;
	}

	// Add pool to our pool linklist...
	pProjPool->_Pool_Add();
	pProjPool->m_bAddedToPoolList = TRUE;

	// Success...

	return (PoolHandle_t)pProjPool;

	// Error...
_ExitWithError:
	fdelete( pProjPool );
	fres_ReleaseFrame( ResFrame );
	return EPROJPOOL_NULL_HANDLE;
}


void CEProjPool::_Destroy( void ) {
	if( IsSystemInitialized() ) {
		fdelete_array( m_pProjArray );
		fdelete_array( m_pWorldMeshArray );

		if( m_bAddedToPoolList ) {
			_Pool_Remove();
		}
	}

	m_pMesh = NULL;
	m_pProjArray = NULL;
	m_pWorldMeshArray = NULL;
	m_bAddedToPoolList = FALSE;
	m_bBuiltBeforeWorld = FALSE;
	m_uMeshInstFlags = FMESHINST_FLAG_NONE;
	m_bCreated = FALSE;
}


BOOL CEProjPool::_AllocatePoolArrays( void ) {
	FMeshInit_t MeshInit;
	u32 i;

	FASSERT( !m_bCreated );
	FASSERT( m_pProjArray == NULL );
	FASSERT( m_pWorldMeshArray == NULL );
	FASSERT( FWorld_pWorld );
	FASSERT( CEntity::IsSystemInitialized() );

	FResFrame_t ResFrame = fres_GetFrame();

	// Create array of projectile objects...
	m_pProjArray = fnew CEProj[m_nProjCount];
	if( m_pProjArray == NULL ) {
		DEVPRINTF( "CEProjPool::_AllocatePoolArrays(): Not enough memory to create array of projectile objects.\n" );
		goto _ExitWithError;
	}

	// Create array of world meshes...
	m_pWorldMeshArray = fnew CFWorldMesh[m_nProjCount];
	if( m_pWorldMeshArray == NULL ) {
		DEVPRINTF( "CEProjPool::_AllocatePoolArrays(): Not enough memory to create array of projectile world meshes.\n" );
		goto _ExitWithError;
	}

	// Initialize our projectiles...
	for( i=0; i<m_nProjCount; i++ ) {
		// Init world mesh...
		MeshInit.pMesh = m_pMesh;
		MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist = 500.0f;
		MeshInit.Mtx.Identity();
		m_pWorldMeshArray[i].Init( &MeshInit, FALSE );
		m_pWorldMeshArray[i].RemoveFromWorld();
		m_pWorldMeshArray[i].SetCollisionFlag( FALSE );
		m_pWorldMeshArray[i].SetUserTypeBits ( ENTITY_BIT_PROJECTILE );
		m_pWorldMeshArray[i].SetLineOfSightFlag( FALSE );
		m_pWorldMeshArray[i].m_nUser = MESHTYPES_ENTITY;
		m_pWorldMeshArray[i].m_pUser = &m_pProjArray[i];

		if( !m_pProjArray[i].Create( m_nProjType, &m_pWorldMeshArray[i] ) ) {
			// Trouble building projectile...
			break;
		}
		FMATH_SETBITMASK( m_pWorldMeshArray[i].m_nFlags, m_uMeshInstFlags ); 
	}
	if( i < m_nProjCount ) {
		DEVPRINTF( "CEProjPool::_AllocatePoolArrays(): Could not create all projectile objects.\n" );
		goto _ExitWithError;
	}

	// Build the free projectile linklist...
	for( i=0; i<m_nProjCount; i++ ) {
		m_pProjArray[i].m_pProjPool = this;
		m_pProjArray[i].ClearActivePoolFlag();
		_FreeProj_AddTail( &m_pProjArray[i] );
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEProjPool::_ResDestroyedCallback( void *pResMem ) {
	CEProjPool *pProjPool = (CEProjPool *)pResMem;

	fdelete( pProjPool );
}


u32 CEProjPool::GetFreePoolProjectileCount( PoolHandle_t hPool ) {
	FASSERT( IsSystemInitialized() );

	CEProjPool *pProjPool = (CEProjPool *)hPool;
	FASSERT( pProjPool->m_bCreated );

	return pProjPool->m_FreeProjLinkRoot.nCount;
}


CEProj *CEProjPool::GetProjectileFromFreePool( PoolHandle_t hPool ) {
	CEProj *pProj;

	FASSERT( IsSystemInitialized() );

	CEProjPool *pProjPool = (CEProjPool *)hPool;
	FASSERT( pProjPool->m_bCreated );

	pProj = pProjPool->_FreeProj_RemoveTail();
	if( pProj ) {
		pProjPool->_ActiveProj_AddTail( pProj );
		pProj->SetActivePoolFlag();
	}

	return pProj;
}


void CEProjPool::ReturnProjectileToFreePool( CEProj *pProj ) {
	FASSERT( IsSystemInitialized() );

	if( pProj && pProj->m_pProjPool && pProj->IsActivePoolFlagSet() ) {
		pProj->RemoveFromWorld(TRUE);
		if( pProj->m_pProjExt ) {
			pProj->m_pProjExt->Removed( pProj );
		}
		pProj->m_pProjPool->_ActiveProj_Remove( pProj );
		pProj->m_pProjPool->_FreeProj_AddTail( pProj );
		pProj->ClearActivePoolFlag();
	}
}


void CEProjPool::ReturnAllProjectilesToFreePoolForAllPools( void ) {
	CEProjPool *pProjPool;

	if( IsSystemInitialized() ) {
		for( pProjPool=_Pool_GetHead(); pProjPool; pProjPool=pProjPool->_Pool_GetNext() ) {
			ReturnAllProjectilesToFreePool( (PoolHandle_t)pProjPool );
		}
	}
}


void CEProjPool::ReturnAllProjectilesToFreePool( PoolHandle_t hPool ) {
	CEProj *pProj;

	if( IsSystemInitialized() ) {
		CEProjPool *pProjPool = (CEProjPool *)hPool;
		FASSERT( pProjPool->m_bCreated );

		while( pProj = pProjPool->_ActiveProj_GetTail() ) {
			pProj->Detonate( FALSE );
//			ReturnProjectileToFreePool( pProj );
		}
	}
}


void CEProjPool::RemoveProjectileFromAllPools( CEProj *pProj ) {
	if( IsSystemInitialized() ) {
		if( pProj->m_pProjPool ) {
			pProj->RemoveFromWorld();

			if( pProj->IsActivePoolFlagSet() ) {
				pProj->m_pProjPool->_ActiveProj_Remove( pProj );
			} else {
				pProj->m_pProjPool->_FreeProj_Remove( pProj );
			}

			pProj->m_pProjPool = NULL;
			pProj->ClearActivePoolFlag();
		}
	}
}


BOOL CEProjPool::_WorldCallback( FWorldEvent_e nEvent ) {
	CEProjPool *pProjPool;

	switch( nEvent ) {
	case FWORLD_EVENT_WORLD_POSTLOAD:
		// The world has been loaded. Finish creating all projectile pools
		// that have been built prior to now...

		for( pProjPool=_Pool_GetHead(); pProjPool; pProjPool=pProjPool->_Pool_GetNext() ) {
			FASSERT( !pProjPool->m_bCreated );
			FASSERT( pProjPool->m_bBuiltBeforeWorld );

			if( !pProjPool->_AllocatePoolArrays() ) {
				goto _ExitWithError;
			}

			pProjPool->m_bCreated = TRUE;
		}

		break;

	case FWORLD_EVENT_WORLD_PREDESTROY:
		for( pProjPool=_Pool_GetHead(); pProjPool; pProjPool=pProjPool->_Pool_GetNext() ) {
			if( pProjPool->m_bBuiltBeforeWorld ) {
				// This projectile pool was created before the world
				// was created. Now that the world is being destroyed, we must
				// restore this pool's state to the way it was just before the
				// world was created...

				FASSERT( pProjPool->m_bCreated );

				fdelete_array( pProjPool->m_pProjArray );
				fdelete_array( pProjPool->m_pWorldMeshArray );

				pProjPool->m_pProjArray = NULL;
				pProjPool->m_pWorldMeshArray = NULL;
				pProjPool->m_bCreated = FALSE;
			}
		}

		break;
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	return FALSE;
}


void CEProj::InflictDamage( CDamageData *pDamageData ) {
	if( m_pFcnDamageCallback ) {
		m_pFcnDamageCallback( this, pDamageData );
	} else {
		CEntity::InflictDamage( pDamageData );
	}
}


void CEProj::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies ) {
	if( m_pFcnDeathCallback ) {
		m_pFcnDeathCallback( this );
	} else {
		Detonate( FALSE, CEProj::EVENT_DETONATE_API, NULL );
		CEntity::Die( bSpawnDeathEffects, FALSE );
	}
}

