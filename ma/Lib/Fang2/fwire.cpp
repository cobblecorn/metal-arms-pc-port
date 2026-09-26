//////////////////////////////////////////////////////////////////////////////////////
// FWire.cpp - Wire rendering module.
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
// 03/30/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fwire.h"
#include "fmeshpool.h"
#include "fresload.h"
#include "fkdop.h"
#include "fmesh_coll.h"
#include "fdebris.h"
#include "fsound.h"


BOOL CFWire::m_bSystemInitialized;
BOOL CFWire::m_bCreated;

u32 CFWire::m_nWirePoolCount;
CFWire *CFWire::m_pWireArray;

CFMeshPool *CFWire::m_pMeshPool;

f32 CFWire::m_fWireVersionDeltaT;
u32 CFWire::m_nWiresPerTexture;

f32 CFWire::m_fInvGeoLength;
FMeshTexLayerHandle_t CFWire::m_hTexLayer;

CFSoundGroup *CFWire::m_pSoundGroupSlice;

CFDebrisDef CFWire::m_DebrisDef;
SmokeTrailAttrib_t CFWire::m_SmokeTrailAttrib;
CFWire::CWireTargetInfo CFWire::m_aWireTargetInfo[FWIRE_MAX_WIRE_TARGETS];

CFTrackerCollideRayInfo CFWire::m_TrackerCollideRayInfo;
const CFVec3A *CFWire::m_pStartPoint_WS;
const CFVec3A *CFWire::m_pEndPoint_WS;
CFVec3A CFWire::m_StartToEndUnitVec_WS;
u32 CFWire::m_nWireTargetCount;



void CFWire::CCreateInfo::SetToDefaults( void ) {
	m_pszWireMeshName = NULL;
	m_pszDebrisMeshName = NULL;
	m_pszSoundGroupSlice = NULL;
	m_nWiresPerTexture = 0;
	m_nWirePoolCount = 0;
	m_nTexLayerID = 0;
}


BOOL CFWire::ModuleStartup( void ) {
	FASSERT( !m_bSystemInitialized );

	_ClearStaticMembers();

	m_bSystemInitialized = TRUE;

	return TRUE;
}


void CFWire::ModuleShutdown( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


BOOL CFWire::Create( const CCreateInfo *pCreateInfo ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );

	CFWorldMeshItem *pWorldMeshItem;
	FMeshInit_t MeshInit;
	CFVec2 InitialST;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	if( pCreateInfo->m_pszWireMeshName == NULL ) {
		DEVPRINTF( "CFWire::Create(): No wire mesh name provided.\n" );
		goto _ExitWithError;
	}

	if( pCreateInfo->m_nWiresPerTexture == 0 ) {
		DEVPRINTF( "CFWire::Create(): Wires per texture is 0.\n" );
		goto _ExitWithError;
	}

	if( pCreateInfo->m_nWirePoolCount == 0 ) {
		DEVPRINTF( "CFWire::Create(): Wire pool count is 0.\n" );
		goto _ExitWithError;
	}

	m_nWirePoolCount = pCreateInfo->m_nWirePoolCount;

	m_pMeshPool = CFMeshPool::CreatePool( m_nWirePoolCount );
	if( m_pMeshPool == NULL ) {
		DEVPRINTF( "CFWire::Create(): Could not create mesh pool.\n" );
		goto _ExitWithError;
	}

	m_pWireArray = fnew CFWire[ m_nWirePoolCount ];
	if( m_pWireArray == NULL ) {
		DEVPRINTF( "CFWire::Create(): Not enough memory to create m_pWireArray.\n" );
		goto _ExitWithError;
	}

	MeshInit.pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pCreateInfo->m_pszWireMeshName );
	if( MeshInit.pMesh == NULL ) {
		DEVPRINTF( "CFWire::Create(): Could not load wire mesh '%s'.\n", pCreateInfo->m_pszWireMeshName );
		goto _ExitWithError;
	}

	MeshInit.fCullDist = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Identity();
	MeshInit.nFlags = FMESHINST_FLAG_NOBONES | FMESHINST_FLAG_FORCE_CLIP;

	for( i=0; i<m_nWirePoolCount; ++i ) {
		pWorldMeshItem = m_pMeshPool->GetItemByIndex( i );
		pWorldMeshItem->Init( &MeshInit );
	}

	m_fInvGeoLength = fmath_Inv( 2.0f * pWorldMeshItem->m_BoundSphere_MS.m_fRadius );

	m_hTexLayer = pWorldMeshItem->GetTexLayerHandle( pCreateInfo->m_nTexLayerID );
	if( m_hTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) {
		DEVPRINTF( "CFWire::Create(): Could not find texture layer ID %u in mesh '%s'.\n", pCreateInfo->m_nTexLayerID, pCreateInfo->m_pszWireMeshName );
		goto _ExitWithError;
	}

	m_nWiresPerTexture = pCreateInfo->m_nWiresPerTexture;
	FMATH_CLAMPMAX( m_nWiresPerTexture, 255 );
	m_fWireVersionDeltaT = 1.0f / (f32)m_nWiresPerTexture;

	InitialST.Set( 0.0f, 0.0f );

	for( i=0; i<m_nWirePoolCount; ++i ) {
		pWorldMeshItem = m_pMeshPool->GetItemByIndex( i );

		pWorldMeshItem->AnimTC_ApplyToShader( m_hTexLayer, TRUE );
		pWorldMeshItem->AnimTC_AnimateScroll( m_hTexLayer, FALSE );
		pWorldMeshItem->AnimTC_SetScrollST( m_hTexLayer, InitialST );

		pWorldMeshItem->RemoveFromWorld();
	}

	fang_MemCopy( &m_DebrisDef, &CFDebrisDef::m_DefaultDebrisDef, sizeof(CFDebrisDef) );

	m_DebrisDef.m_nFlags = CFDebrisDef::FLAG_ROTATIONAL_MOTION;
	m_DebrisDef.m_nPriority = 1;
	m_DebrisDef.m_nCollType = CFDebrisDef::COLL_TYPE_NONE;
	m_DebrisDef.m_nFadeType = CFDebrisDef::FADE_TYPE_SHRINK;
	m_DebrisDef.m_nLightingType = CFDebrisDef::LIGHTING_TYPE_AMBIENT_ONLY;
	m_DebrisDef.m_fMinSpeedToStartFade2 = 1000.0f * 1000.0f;
	m_DebrisDef.m_fSecsBeforeMinSpeedValid = 0.0f;
	m_DebrisDef.m_fSecsUnderMinSpeedToStartFade = 0.0f;
	m_DebrisDef.m_fCullDist = 300.0f;

	m_DebrisDef.m_pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pCreateInfo->m_pszDebrisMeshName );

	m_DebrisDef.m_LinVel_WS.Zero();
	m_DebrisDef.m_fRotSpeed = 6.0f;
	m_DebrisDef.m_fGravity = -32.0f;
	m_DebrisDef.m_fFadeSecs = 1.0f;
	m_DebrisDef.m_fAliveSecs = 0.0f;

	_SetSmokeTrailAttributes();

	m_pSoundGroupSlice = CFSoundGroup::RegisterGroup( pCreateInfo->m_pszSoundGroupSlice );

	m_TrackerCollideRayInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	m_TrackerCollideRayInfo.nTrackerUserTypeBitsMask = -1;
	m_TrackerCollideRayInfo.bIgnoreCollisionFlag = FALSE;
	m_TrackerCollideRayInfo.bComputeIntersection = FALSE;
	m_TrackerCollideRayInfo.bSort = FALSE;
	m_TrackerCollideRayInfo.pCallback = _FindTrackersCallback;

	m_bCreated = TRUE;

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CFWire::Destroy( void ) {
	if( m_bCreated ) {
		CFMeshPool::DestroyPool( m_pMeshPool );
		fdelete( m_pWireArray );

		_ClearStaticMembers();

		m_bCreated = FALSE;
	}
}


void CFWire::_ClearStaticMembers( void ) {
	m_nWirePoolCount = 0;
	m_pWireArray = NULL;
	m_pMeshPool = NULL;
	m_nWiresPerTexture = 0;
	m_fWireVersionDeltaT = 0.0f;
	m_fInvGeoLength = 1.0f;
	m_hTexLayer = FMESH_TEXLAYERHANDLE_INVALID;
	m_pSoundGroupSlice = NULL;

	m_DebrisDef.m_pMesh = NULL;
}


void CFWire::_SetSmokeTrailAttributes( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1steam01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CFWire::_SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 0.8f;
	m_SmokeTrailAttrib.fScaleMax_WS = 1.0f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.1f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.1f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.3f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.0f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 0.2f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.45f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.55f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 0.8f, 0.8f, 0.8f );
	m_SmokeTrailAttrib.EndColorRGB.White();
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.2f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.6f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
}


CFWire *CFWire::GetFromFreePool( BOOL bMakeCollidable, u8 nWireGeoIndex, void *pOwner ) {
	if( !IsCreated() ) {
		return NULL;
	}

	CFWorldMeshItem *pWorldMeshItem;
	CFWire *pWire;

	pWorldMeshItem = m_pMeshPool->GetFromFreePool();
	if( pWorldMeshItem == NULL ) {
		pWorldMeshItem = _StealWire();

		if( pWorldMeshItem == NULL ) {
			return NULL;
		}
	}

	// Found a wire...

	pWire = m_pWireArray + pWorldMeshItem->GetPoolIndex();
	pWire->m_nMeshPoolIndex = pWorldMeshItem->GetPoolIndex();
	pWire->m_pOwner = pOwner;
	pWire->m_nWireGeoIndex = nWireGeoIndex;

	FMATH_CLAMPMAX( pWire->m_nWireGeoIndex, m_nWiresPerTexture );

	pWorldMeshItem->m_nUser = FWORLD_USERTYPE_WIRE;
	pWorldMeshItem->m_pUser = pWire;

	pWorldMeshItem->AnimTC_ApplyToShader( m_hTexLayer, TRUE );
	pWorldMeshItem->AnimTC_AnimateScroll( m_hTexLayer, FALSE );

	pWorldMeshItem->SetCollisionFlag( bMakeCollidable );

	if( bMakeCollidable ) {
		pWorldMeshItem->SetCollPrepCallback( _CollPrepCallback );
	} else {
		pWorldMeshItem->SetCollPrepCallback( NULL );
	}

	return pWire;
}


CFWorldMeshItem *CFWire::_StealWire( void ) {
	return NULL;
}


void CFWire::ReturnToFreePool( CFWire *pWire ) {
	if( !IsCreated() ) {
		return;
	}

	if( pWire == NULL ) {
		return;
	}

	CFWorldMeshItem *pWorldMeshItem = m_pMeshPool->GetItemByIndex( pWire->m_nMeshPoolIndex );
	m_pMeshPool->ReturnToFreePool( pWorldMeshItem );
}


// Returns TRUE if the wire was returned to the free pool.
BOOL CFWire::UpdateEndPoints( const CFVec3A *pStart, const CFVec3A *pEnd, BOOL bReturnToPoolIfTooSmall ) {
	if( !IsCreated() ) {
		return FALSE;
	}

	CFQuatA CurrentQuat, RotQuat, NewQuat;
	CFVec3A StartToEndUnitVec_WS, BoundSphereCenter_WS;
	CFMtx43A NewMtx;
	CFVec2 ScrollST;
	CFWorldMeshItem *pWorldMeshItem;
	f32 fS, fT, fMag2, fInvMag, fWireLength;

	pWorldMeshItem = m_pMeshPool->GetItemByIndex( m_nMeshPoolIndex );

	StartToEndUnitVec_WS.Sub( *pEnd, *pStart );

	fMag2 = StartToEndUnitVec_WS.MagSq();
	if( fMag2 < 0.1f ) {
		if( bReturnToPoolIfTooSmall ) {
			ReturnToFreePool( this );
			return TRUE;
		}

		pWorldMeshItem->RemoveFromWorld();

		return FALSE;
	}

	fInvMag = fmath_InvSqrt(fMag2);
	StartToEndUnitVec_WS.Mul( fInvMag );

	fWireLength = fmath_Inv( fInvMag );

	CurrentQuat.BuildQuat( pWorldMeshItem->m_Xfm.m_MtxF );

	RotQuat.BuildQuat( pWorldMeshItem->m_Xfm.m_MtxF.m_vZ, StartToEndUnitVec_WS );

	NewQuat.Mul( RotQuat, CurrentQuat );
	NewQuat.Unitize();

	NewQuat.BuildMtx33( NewMtx );

	NewMtx.m_vZ = StartToEndUnitVec_WS;
	NewMtx.m_vX.Cross( NewMtx.m_vY, NewMtx.m_vZ ).Unitize();
	NewMtx.m_vY.Cross( NewMtx.m_vZ, NewMtx.m_vX );

	NewMtx.m_vPos = *pStart;

	// Update bounding sphere...
	pWorldMeshItem->m_BoundSphere_MS.m_fRadius = 0.5f * fWireLength;

	pWorldMeshItem->m_BoundSphere_MS.m_Pos.z = pWorldMeshItem->m_BoundSphere_MS.m_fRadius;

	pWorldMeshItem->m_Xfm.BuildFromMtx( NewMtx );
	pWorldMeshItem->UpdateTracker();

	// Update texture coordinates...
	fS = 0.5f - fWireLength * m_fInvGeoLength;
	fT = (f32)m_nWireGeoIndex * m_fWireVersionDeltaT;

	ScrollST.Set( fS, fT );

	pWorldMeshItem->AnimTC_SetScrollST( m_hTexLayer, ScrollST );

	return FALSE;
}


void CFWire::EnableCollision( BOOL bEnableCollision ) {
	if( !IsCreated() ) {
		return;
	}

	CFWorldMeshItem *pWorldMeshItem = m_pMeshPool->GetItemByIndex( m_nMeshPoolIndex );

	pWorldMeshItem->SetCollisionFlag( bEnableCollision );
}


void CFWire::_CollPrepCallback( CFWorldMesh *pWorldMesh ) {
	FASSERT( pWorldMesh->m_nUser == FWORLD_USERTYPE_WIRE );
	FASSERT( pWorldMesh->GetType() == FWORLD_TRACKERTYPE_MESH );

	CFWire *pWire = (CFWire *)pWorldMesh->m_pUser;

	FkDOP_Interval_t *pIntervalZ = &pWorldMesh->m_pMesh->paCollTree->paIntervals[2];

	pIntervalZ->fMax = pWorldMesh->GetBoundingSphere().m_fRadius * 2.0f;
}


void CFWire::Shatter( CFWire *pWire ) {
	if( pWire == NULL ) {
		return;
	}

	CFWorldMesh *pWorldMesh;
	f32 fWireLength, fDebrisLength;
	u32 i, nDebrisCount, nDebrisCountMinusOne;
	CFMtx43A Mtx;
	CFQuatA RotQuat;
	CFVec3A DeltaPos_WS, SoundPos_WS;

	if( m_DebrisDef.m_pMesh == NULL ) {
		goto _ReturnToFreePoolAndExit;
	}

	pWorldMesh = m_pMeshPool->GetItemByIndex( pWire->m_nMeshPoolIndex );

	fWireLength = pWorldMesh->GetBoundingSphere().m_fRadius * 2.0f;
	fDebrisLength = m_DebrisDef.m_pMesh->BoundSphere_MS.m_fRadius * 2.0f;

	nDebrisCount = (u32)fmath_Div( fWireLength, fDebrisLength );
	if( nDebrisCount == 0 ) {
		goto _ReturnToFreePoolAndExit;
	}

	FMATH_CLAMPMAX( nDebrisCount, 20 );

	nDebrisCountMinusOne = nDebrisCount - 1;

	Mtx.m_vX = pWorldMesh->m_Xfm.m_MtxF.m_vX;
	Mtx.m_vY = pWorldMesh->m_Xfm.m_MtxF.m_vY;
	Mtx.m_vZ = pWorldMesh->m_Xfm.m_MtxF.m_vZ;

	Mtx.m_vPos.Mul( pWorldMesh->m_Xfm.m_MtxF.m_vZ, m_DebrisDef.m_pMesh->BoundSphere_MS.m_fRadius ).Add( pWorldMesh->m_Xfm.m_MtxF.m_vPos );

	DeltaPos_WS.Mul( pWorldMesh->m_Xfm.m_MtxF.m_vZ, 2.0f * m_DebrisDef.m_pMesh->BoundSphere_MS.m_fRadius );

	for( i=0; i<nDebrisCount; ++i ) {
		m_DebrisDef.m_LinVel_WS.x = fmath_RandomFloatRange( -2.0f, 2.0f );
		m_DebrisDef.m_LinVel_WS.y = fmath_RandomFloatRange( 10.0f, 30.0f );
		m_DebrisDef.m_LinVel_WS.z = fmath_RandomFloatRange( -2.0f, 2.0f );

		m_DebrisDef.m_fRotSpeed = fmath_RandomFloatRange( 6.0f, 9.0f );
		m_DebrisDef.m_fGravity = fmath_RandomFloatRange( -32.0f, -42.0f );
		m_DebrisDef.m_fFadeSecs = fmath_RandomFloatRange( 0.8f, 1.2f );

		RotQuat.BuildQuat( pWorldMesh->m_Xfm.m_MtxF.m_vZ, fmath_RandomFloatRange( 0.0f, FMATH_2PI ) );
		RotQuat.MulPoint( m_DebrisDef.m_UnitRotAxis_WS, pWorldMesh->m_Xfm.m_MtxF.m_vX );

		CFDebris::Spawn( &m_DebrisDef, &Mtx );

		if( i < nDebrisCountMinusOne ) {
			Mtx.m_vPos.Add( DeltaPos_WS );
		}
	}

	smoketrail_SpawnMuzzlePuff( &m_SmokeTrailAttrib, &pWorldMesh->m_Xfm.m_MtxF.m_vPos, &pWorldMesh->m_Xfm.m_MtxF.m_vZ, fWireLength * 0.5f );

	SoundPos_WS.Set( pWorldMesh->GetBoundingSphere().m_Pos );
	CFSoundGroup::PlaySound( m_pSoundGroupSlice, FALSE, &SoundPos_WS );

_ReturnToFreePoolAndExit:
	ReturnToFreePool( pWire );
}


// Returns TRUE if pTargetedPointOnWire_WS and ppTargetedWireWorldMesh have been
// targeted.
BOOL CFWire::Targeting( const CFVec3A *pRayStart_WS, const CFVec3A *pRayEnd_WS, f32 fCamAdjustZ, CFVec3A *pTargetedPointOnWire_WS, CFWorldMesh **ppTargetedWireWorldMesh ) {
	f32 fMag2;

	m_StartToEndUnitVec_WS.Sub( *pRayEnd_WS, *pRayStart_WS );
	fMag2 = m_StartToEndUnitVec_WS.MagSq();
	if( fMag2 < 0.0001f ) {
		return FALSE;
	}
	m_StartToEndUnitVec_WS.Mul( fmath_InvSqrt(fMag2) );

	CFVec3A AdjustedStart_WS, NewStartToEndVec_WS;

	AdjustedStart_WS.Mul( m_StartToEndUnitVec_WS, fCamAdjustZ ).Add( *pRayStart_WS );

	NewStartToEndVec_WS.Sub( *pRayEnd_WS, AdjustedStart_WS );
	if( NewStartToEndVec_WS.Dot( m_StartToEndUnitVec_WS ) < 0.5f ) {
		return FALSE;
	}

	m_TrackerCollideRayInfo.StartPoint_WS = AdjustedStart_WS;
 	m_TrackerCollideRayInfo.EndPoint_WS = *pRayEnd_WS;
 
	m_pStartPoint_WS = &AdjustedStart_WS;
	m_pEndPoint_WS = pRayEnd_WS;

	m_nWireTargetCount = 0;
	fworld_CollideWithTrackers( &m_TrackerCollideRayInfo );

	CWireTargetInfo *pWireTargetInfo, *pBestWireTargetInfo;
	u32 i;
	s32 nTrackerSkipIndex;

	pBestWireTargetInfo = NULL;
	nTrackerSkipIndex = -1;

	if( FWorld_nTrackerSkipListCount < (FWORLD_MAX_SKIPLIST_ENTRIES - 1) ) {
		nTrackerSkipIndex = FWorld_nTrackerSkipListCount;
		++FWorld_nTrackerSkipListCount;
	}

	for( i=0; i<m_nWireTargetCount; ++i ) {
		pWireTargetInfo = m_aWireTargetInfo + i;

		if( pBestWireTargetInfo && (pWireTargetInfo->m_fDistToCamera2 >= pBestWireTargetInfo->m_fDistToCamera2) ) {
			continue;
		}

		if( nTrackerSkipIndex != -1 ) {
			FWorld_apTrackerSkipList[nTrackerSkipIndex] = pWireTargetInfo->m_pWorldMesh;
		}

		if( fworld_IsLineOfSightObstructed( m_pStartPoint_WS, &pWireTargetInfo->m_PointOnWire_WS, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList ) ) {
			// Wire obstructed from view...
			continue;
		}

		pBestWireTargetInfo = pWireTargetInfo;
	}

	if( nTrackerSkipIndex != -1 ) {
		--FWorld_nTrackerSkipListCount;
	}

	if( pBestWireTargetInfo ) {
		// We found a wire target...

		*pTargetedPointOnWire_WS = pBestWireTargetInfo->m_PointOnWire_WS;
		*ppTargetedWireWorldMesh = pBestWireTargetInfo->m_pWorldMesh;

		return TRUE;
	}

	// No wire target...

	return FALSE;
}


BOOL CFWire::_FindTrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection ) {
	if( pTracker->m_nUser != FWORLD_USERTYPE_WIRE ) {
		// Not a wire...
		return TRUE;
	}

	if( !pTracker->IsCollisionFlagSet() ) {
		return TRUE;
	}

	if( m_nWireTargetCount >= FWIRE_MAX_WIRE_TARGETS ) {
		return FALSE;
	}

	CWireTargetInfo *pWireTargetInfo = m_aWireTargetInfo + m_nWireTargetCount;
	CFWire *pWire = (CFWire *)pTracker->m_pUser;
	pWireTargetInfo->m_pWorldMesh = m_pMeshPool->GetItemByIndex( pWire->m_nMeshPoolIndex );
	const CFMtx43A *pWireMtx = &pWireTargetInfo->m_pWorldMesh->m_Xfm.m_MtxF;

	f32 fDistFromReticleRayToWire = _ComputeDistBetweenLines( m_pStartPoint_WS, &m_StartToEndUnitVec_WS, &pWireMtx->m_vPos, &pWireMtx->m_vZ );

	if( fDistFromReticleRayToWire > 1.0f ) {
		return TRUE;
	}

	CFMtx43A ReticleRayMtx;
	CFVec3A WireEndPos_WS, WireEndPos_RS, WireStartPos_RS;
	CFVec2 WireUnitDirXY_RS, WireUnitPerpXY_RS, PosXY_RS;
	f32 fDist2, fOODist, fT, fWireLength;

	fWireLength = 2.0f * pWireTargetInfo->m_pWorldMesh->GetBoundingSphere().m_fRadius;

	WireEndPos_WS.Mul( pWireMtx->m_vZ, fWireLength ).Add( pWireMtx->m_vPos );

	ReticleRayMtx.m_vPos = *m_pStartPoint_WS;
	ReticleRayMtx.UnitMtxFromUnitVec( &m_StartToEndUnitVec_WS ).AffineInvert( FALSE );

	ReticleRayMtx.MulPoint( WireStartPos_RS, pWireMtx->m_vPos );
	ReticleRayMtx.MulPoint( WireEndPos_RS, WireEndPos_WS );

	WireUnitDirXY_RS = WireEndPos_RS.v2 - WireStartPos_RS.v2;

	fDist2 = WireUnitDirXY_RS.Mag2();
	if( fDist2 < 0.01f ) {
		// Lines are nearly parallel...
		pWireTargetInfo->m_PointOnWire_WS.Set( pWireTargetInfo->m_pWorldMesh->GetBoundingSphere().m_Pos );
	} else {
		// Lines are not parallel...

		fOODist = fmath_InvSqrt( fDist2 );

		WireUnitDirXY_RS *= fOODist;
		WireUnitPerpXY_RS.x = -WireUnitDirXY_RS.y;
		WireUnitPerpXY_RS.y = WireUnitDirXY_RS.x;

		PosXY_RS = WireUnitPerpXY_RS * WireStartPos_RS.v2.Dot( WireUnitPerpXY_RS );

		fT = fOODist * (PosXY_RS - WireStartPos_RS.v2).Mag();

		if( (fT >= 0.0f) && (fT <= 1.0f ) ) {
			// Wire in range of reticle...
			pWireTargetInfo->m_PointOnWire_WS.Mul( pWireMtx->m_vZ, fT * fWireLength ).Add( pWireMtx->m_vPos );
		} else {
			// Wire out of range of reticle...
			return TRUE;
		}
	}

	pWireTargetInfo->m_fDistToCamera2 = pWireTargetInfo->m_PointOnWire_WS.DistSq( *m_pStartPoint_WS );

	++m_nWireTargetCount;

	return TRUE;
}


f32 CFWire::_ComputeDistBetweenLines( const CFVec3A *pPosA_WS, const CFVec3A *pUnitDirA_WS, const CFVec3A *pPosB_WS, const CFVec3A *pUnitDirB_WS ) {
	CFVec3A DirACrossDirB_WS, DeltaPos_WS;
	f32 fMag2, fDot;

	DirACrossDirB_WS.Cross( *pUnitDirA_WS, *pUnitDirB_WS );
	DeltaPos_WS.Sub( *pPosB_WS, *pPosA_WS );

	fMag2 = DirACrossDirB_WS.MagSq();
	if( fMag2 < 0.0001f ) {
		// Lines are parallel...

		fDot = DeltaPos_WS.Dot( *pUnitDirA_WS );

		DeltaPos_WS.Mul( *pUnitDirA_WS, fDot ).Add( *pPosA_WS );

		return pPosB_WS->Dist( DeltaPos_WS );
	}

	// Lines are not parallel...

	fDot = DeltaPos_WS.Dot( DirACrossDirB_WS );

	return fmath_Abs(fDot) * fmath_InvSqrt( fMag2 );
}


