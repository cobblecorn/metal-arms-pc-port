//////////////////////////////////////////////////////////////////////////////////////
// fcoll.cpp - Fang collision module.
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
// 02/21/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fcoll.h"
#include "fres.h"
#include "fclib.h"
#include "fmesh_coll.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fvis.h"


//////////////////////////////////////////////////////////////////////////////////////
// Global Variables:
//////////////////////////////////////////////////////////////////////////////////////

#if !FANG_PRODUCTION_BUILD
	u32 FColl_nImpactCountOverLimit;
	u32 _nMaxImpactCountOverLimit;
#endif

u32 FColl_nMaxCollImpacts;					// Maximum number of impacts that FColl_aCollImpactBuf can hold
u32 FColl_nImpactCount;						// Number of impacts currently in FColl_aCollImpactBuf
u16 FColl_nImpactCollMask;					// The accumulation of all the coll masks of the tris in the impact buff

FCollImpact_t *FColl_aImpactBuf;			// Impact buffer
FCollImpact_t *FColl_pNextImpactSlot;		// Next empty slot in impact buffer
FCollImpact_t *FColl_pImpactBufEnd;			// The slot beyond the last in the FColl_aCollImpactBuf

FCollImpact_t **FColl_apSortedImpactBuf;	// Impact buffer


//////////////////////////////////////////////////////////////////////////////////////
// Local Variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;


//////////////////////////////////////////////////////////////////////////////////////
// Local Functions:
//////////////////////////////////////////////////////////////////////////////////////

static int _SortCallback_Ascend( const void *pElement1, const void *pElement2 );
static int _SortCallback_Descend( const void *pElement1, const void *pElement2 );
static int _SortDynamicCallback_Ascend( const void *pElement1, const void *pElement2 );
static int _SortDynamicCallback_Descend( const void *pElement1, const void *pElement2 );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fcoll_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

#if !FANG_PRODUCTION_BUILD
	FColl_nImpactCountOverLimit = 0;
	_nMaxImpactCountOverLimit = 0;
#endif

	FColl_nMaxCollImpacts = Fang_ConfigDefs.nColl_MaxImpacts;
//	FColl_aImpactBuf = (FCollImpact_t *)fres_Alloc( FColl_nMaxCollImpacts * sizeof(FCollImpact_t) );
//	FColl_aImpactBuf = (FCollImpact_t *)fres_AlignedAlloc( FColl_nMaxCollImpacts * sizeof(FCollImpact_t), 16 );
	FColl_aImpactBuf = fnew FCollImpact_t[FColl_nMaxCollImpacts];
	if ( FColl_aImpactBuf == NULL ) 
	{
		// Out of memory...
		DEVPRINTF( "fcoll_ModuleStartup(): Could not allocate %u bytes.\n", FColl_nMaxCollImpacts * sizeof(FCollImpact_t) );
		return FALSE;
	}

	fang_MemZero( FColl_aImpactBuf, sizeof( FCollImpact_t ) * FColl_nMaxCollImpacts );

	FColl_apSortedImpactBuf = (FCollImpact_t **)fres_Alloc( FColl_nMaxCollImpacts * sizeof(FCollImpact_t *) );
	if ( FColl_apSortedImpactBuf == NULL ) 
	{
		// Out of memory...
		DEVPRINTF( "fcoll_ModuleStartup(): Could not allocate %u bytes.\n", FColl_nMaxCollImpacts * sizeof(FCollImpact_t *) );
		return FALSE;
	}

	FColl_nImpactCount = 0;
	FColl_pNextImpactSlot = FColl_aImpactBuf;
	FColl_pImpactBufEnd = FColl_aImpactBuf + FColl_nMaxCollImpacts;

	_bModuleInitialized = TRUE;

	return fmesh_coll_ModuleStartup();
}


//
//
//
void fcoll_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fdelete_array( FColl_aImpactBuf );
	FColl_aImpactBuf = NULL;

	_bModuleInitialized = FALSE;

	fmesh_coll_ModuleShutdown();
}


//
//
//
void fcoll_Clear( void ) 
{
	FASSERT( _bModuleInitialized );

#if !FANG_PRODUCTION_BUILD
	// See if the last use exceeded the limit
	if ( FColl_nImpactCountOverLimit )
	{
#if !FANG_DEBUG_BUILD
		// In non-debug build, we only show output the error when we get a new max impact count overage.
		if ( _nMaxImpactCountOverLimit < FColl_nImpactCountOverLimit )
#endif
		{
			DEVPRINTF( "fcoll_Clear() - ERROR - Last use of FColl_aImpactBuf exceeded limit by %d impacts.  Increase collision buffer size.\n", FColl_nImpactCountOverLimit );
		}

		if ( _nMaxImpactCountOverLimit < FColl_nImpactCountOverLimit )
		{
			_nMaxImpactCountOverLimit = FColl_nImpactCountOverLimit;
		}
	}
	FColl_nImpactCountOverLimit = 0;
#endif

	FColl_nImpactCollMask = 0;
	FColl_nImpactCount = 0;
	FColl_pNextImpactSlot = FColl_aImpactBuf;
}


//
//
//
void fcoll_Next( void ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( FColl_nImpactCount <= FColl_nMaxCollImpacts );

	FColl_nImpactCount++;
	FColl_pNextImpactSlot++;
}


// This function sorts the current impact buffer based on FCollImpact::fImpactDistInfo.
//
// Note that FColl_aImpactBuf is not modified. Instead, this function generates a sorted
// array of pointers into FColl_aImpactBuf. The sorted array of pointers is held in FColl_apSortedImpactBuf.
// The number of entries in FColl_apSortedImpactBuf is identical to the number of entries
// in FColl_aImpactBuf which is FColl_nImpactCount.
//
// Set bAscend to TRUE to sort from smallest to largest fImpactDistInfo.
// Set bAscend to FALSE to sort from largest to smallest fImpactDistInfo.
void fcoll_Sort( BOOL bAscend ) 
{
	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( FColl_nImpactCount <= FColl_nMaxCollImpacts );

	if ( FColl_nImpactCount ) 
	{
		for ( i=0; i<FColl_nImpactCount; i++ ) 
		{
			FColl_apSortedImpactBuf[i] = &FColl_aImpactBuf[i];
		}

		fclib_QSort( FColl_apSortedImpactBuf, FColl_nImpactCount, sizeof(FCollImpact_t *), bAscend ? _SortCallback_Ascend : _SortCallback_Descend );
	}
}


//
// Set bAscend to TRUE to sort from smallest to largest fUnitImpactTime. If fUnitImpactTimes are the same, it will sort from smallest to largest fImpactDistInfo.
// Set bAscend to FALSE to sort from largest to smallest fUnitImpactTime. If fUnitImpactTimes are the same, it will sort from largest to smallest fImpactDistInfo.
//
// NOTE:  This behavior is different than fcoll_Sort()
//
void fcoll_SortDynamic( BOOL bAscend ) 
{
	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( FColl_nImpactCount <= FColl_nMaxCollImpacts );

	if ( FColl_nImpactCount ) 
	{
		for ( i = 0; i < FColl_nImpactCount; i++ ) 
		{
			FColl_apSortedImpactBuf[i] = &FColl_aImpactBuf[i];
		}

		fclib_QSort( FColl_apSortedImpactBuf, FColl_nImpactCount, sizeof(FCollImpact_t *), bAscend ? _SortDynamicCallback_Ascend : _SortDynamicCallback_Descend );
	}
}


//
// Returns a pointer to the impact that has the smallest FCollImpact::fImpactDistInfo, or NULL if there are no impacts.
//
FCollImpact_t *fcoll_FindClosest( FCollImpact_t *pStartImpact, FCollImpact_t *pEndImpact ) 
{
	FCollImpact_t *pImpact, *pClosestImpact;
	f32 fSmallestDistInfo;

	FASSERT( _bModuleInitialized );
	FASSERT( FColl_nImpactCount <= FColl_nMaxCollImpacts );

	pClosestImpact = NULL;
	fSmallestDistInfo = FMATH_MAX_FLOAT;

	for ( pImpact=pStartImpact; pImpact<pEndImpact; pImpact++ ) 
	{
		if ( pImpact->fImpactDistInfo < fSmallestDistInfo ) 
		{
			fSmallestDistInfo = pImpact->fImpactDistInfo;
			pClosestImpact = pImpact;
		}
	}

	return pClosestImpact;
}


//
//
//
void fcoll_Renderer_Open( void ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
void fcoll_Renderer_Close( void ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
u32 fcoll_Renderer_GetStateSize( void ) 
{
	FASSERT( _bModuleInitialized );
	return 0;
}


//
//
//
void fcoll_Renderer_GetState( void *pDestState ) 
{
	FASSERT( _bModuleInitialized );
	pDestState;		// To prevent CodeWarrior from generating a warning
}


//
//
//
void fcoll_Renderer_SetState( const void *pState ) 
{
	FASSERT( _bModuleInitialized );
	pState;			// To prevent CodeWarrior from generating a warning
}


//
//
//
void fcoll_Renderer_SetDefaultState( void ) 
{
	FASSERT( _bModuleInitialized );
}


//
//
//
static int _SortCallback_Ascend( const void *pElement1, const void *pElement2 ) 
{
	const FCollImpact_t *pCollImpact1, *pCollImpact2;

	pCollImpact1 = *(const FCollImpact_t **)pElement1;
	pCollImpact2 = *(const FCollImpact_t **)pElement2;

	if ( pCollImpact1->fImpactDistInfo < pCollImpact2->fImpactDistInfo ) 
	{
		return -1;
	} 
	else if ( pCollImpact1->fImpactDistInfo > pCollImpact2->fImpactDistInfo ) 
	{
		return 1;
	} 
	else 
	{
		return 0;
	}
}


//
//
//
static int _SortCallback_Descend( const void *pElement1, const void *pElement2 ) 
{
	const FCollImpact_t *pCollImpact1, *pCollImpact2;

	pCollImpact1 = *(const FCollImpact_t **)pElement1;
	pCollImpact2 = *(const FCollImpact_t **)pElement2;

	if ( pCollImpact1->fImpactDistInfo < pCollImpact2->fImpactDistInfo ) 
	{
		return 1;
	} 
	else if ( pCollImpact1->fImpactDistInfo > pCollImpact2->fImpactDistInfo ) 
	{
		return -1;
	} 
	else 
	{
		return 0;
	}
}


//
//
//
static int _SortDynamicCallback_Ascend( const void *pElement1, const void *pElement2 ) 
{
	const FCollImpact_t *pCollImpact1, *pCollImpact2;

	pCollImpact1 = *(const FCollImpact_t **)pElement1;
	pCollImpact2 = *(const FCollImpact_t **)pElement2;

	if ( pCollImpact1->fUnitImpactTime < pCollImpact2->fUnitImpactTime ) 
	{
		return -1;
	} 
	else if ( pCollImpact1->fUnitImpactTime > pCollImpact2->fUnitImpactTime ) 
	{
		return 1;
	} 
	else 
	{
		if ( pCollImpact1->fImpactDistInfo < pCollImpact2->fImpactDistInfo )
		{
			return 1;
		}
		else if ( pCollImpact1->fImpactDistInfo > pCollImpact2->fImpactDistInfo )
		{
			return -1;
		}

		return 0;
	}
}


//
//
//
static int _SortDynamicCallback_Descend( const void *pElement1, const void *pElement2 ) 
{
	const FCollImpact_t *pCollImpact1, *pCollImpact2;

	pCollImpact1 = *(const FCollImpact_t **)pElement1;
	pCollImpact2 = *(const FCollImpact_t **)pElement2;

	if ( pCollImpact1->fUnitImpactTime < pCollImpact2->fUnitImpactTime ) 
	{
		return 1;
	} 
	else if ( pCollImpact1->fUnitImpactTime > pCollImpact2->fUnitImpactTime ) 
	{
		return -1;
	} 
	else 
	{
		if ( pCollImpact1->fImpactDistInfo < pCollImpact2->fImpactDistInfo )
		{
			return -1;
		}
		else if ( pCollImpact1->fImpactDistInfo > pCollImpact2->fImpactDistInfo )
		{
			return 1;
		}

		return 0;
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Functions for testing projected objects and primitives for collision
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define			_MAX_VOLUMES_IN_BUFFER		64


static FVisVolume_t		*_aVolumeIntersectBuffer[_MAX_VOLUMES_IN_BUFFER];
static u16				_nVolumeIntersectCount;


// Globals set locally and used by external collision functions
CFCollData	 	*FColl_pCollData;
f32 			FColl_fMoveMagnitude;
CFVec3A			FColl_vUnitizedMovement;
u16				FColl_nAccumCollMasks;

// Global variables intended only for shared use in collision modules
CFMtx43A			FColl_mtxTestBone, FColl_mtxTestInvBone, FColl_mtxBone, FColl_mtxTemp;
CFVec3A				FColl_vODiff, FColl_vDiff1, FColl_vDiff2, FColl_vCross, FColl_vPush, FColl_vNormal, FColl_vTransMovement;
CFMtx43A 			*FColl_pTestToWorld;
const CFMeshInst 	*FColl_pTestMeshInst;
const CFWorldMesh 	*FColl_pTestWorldMesh;
FkDOP_Tree_t		*FColl_pkDOPTree1;
FkDOP_Tree_t		*FColl_pkDOPTree2;
u8					FColl_nCurrentBoneIdx;
f32					FColl_fBoneScale;
f32					FColl_fInvBoneScale;
f32					FColl_fScaledMoveMagnitude;


// For kDOP collision:
extern BOOL fcoll_TestMeshAndkDOP( CFWorldMesh *pTestMeshInst, CFMeshInst *pMeshInst, const CFMtx43A *pMeshMtx, s32 nBone );
extern BOOL fcoll_TestWorldMeshAndkDOP( CFMeshInst *pWorldMeshInst, CFMeshInst *pMeshInst, const CFMtx43A *pMeshMtx, s32 nBone );
// For Sphere collision:
extern BOOL fcoll_TestMeshAndObjectSpheres( const CFWorldMesh *pWorldMesh, const CFMeshInst *pMeshInst, s32 nBone );
extern BOOL fcoll_TestWorldMeshAndObjectSpheres( const CFMeshInst *pWorldMeshInst, const CFMeshInst *pMeshInst, s32 nBone );
extern BOOL fcoll_TestMeshAndSphere( const CFWorldMesh *pWorldMesh, const CFSphere *pSphere );
extern BOOL fcoll_TestWorldMeshAndSphere( const CFMeshInst *pWorldMeshInst, const CFSphere *pSphere );
// For Sphere collision:
extern BOOL fcoll_TestMeshAndObjectCapsules( CFWorldMesh *pWorldMesh, CFMeshInst *pMeshInst, s32 nBone );
extern BOOL fcoll_TestWorldMeshAndObjectCapsules( CFMeshInst *pWorldMeshInst, CFMeshInst *pMeshInst, s32 nBone );
extern BOOL fcoll_TestMeshAndCapsule( CFWorldMesh *pWorldMesh, CFCapsule *pCapsule );
extern BOOL fcoll_TestWorldMeshAndCapsule( CFMeshInst *pWorldMeshInst, CFCapsule *pCapsule );


//
//
//
BOOL fcoll_Check( CFCollData *pInfo, CFWorldMesh *pWorldMesh, CFMtx43A *pMeshMtx/*=NULL*/, s32 nBone/*=-1*/ )
{
	FASSERT( pInfo && pWorldMesh );

	if ( !pWorldMesh->IsAddedToWorld() || !pWorldMesh->m_pMesh || !pWorldMesh->m_pMesh->paCollTree )
	{
		return FALSE;
	}
	
	FColl_nAccumCollMasks = 0;
	
	FColl_pCollData = pInfo;

	if ( !FColl_pCollData->pMovement )
	{
		FColl_fMoveMagnitude = 0.f;
	}
	else
	{
		FColl_fMoveMagnitude = FColl_pCollData->pMovement->MagSq();
	}

	if ( FColl_fMoveMagnitude < 0.001f )
	{
		FColl_vUnitizedMovement.Set( 0.f, 0.f, 0.f );
		FColl_fMoveMagnitude = 0.f;
	}
	else
	{
		FColl_fMoveMagnitude = fmath_Sqrt( FColl_fMoveMagnitude );
		FColl_vUnitizedMovement.Mul( *FColl_pCollData->pMovement, fmath_Inv( FColl_fMoveMagnitude ) );
	}

	// Record the start impact count for later use	
	u32 nStartingImpactCount = FColl_nImpactCount;
	
	u32 nKeyIndex = CFWorldKey::OpenKey();

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH];
			
	// Check the volumes this tracker resides in for collision
	BOOL bCollCheck;
	u32 nCallbackReturn;
	FVisVolume_t *pVol;	
	CFWorldTracker *pTracker;
	CFWorldIntersect *pIntersect = NULL, *pTrackerIntersect;
	u16 nIndex, nOrigStopCollMasks = pInfo->nStopOnFirstOfCollMask;
	while ( (pIntersect = (CFWorldIntersect *)pWorldMesh->GetNextIntersect( pIntersect )) ) 
	{
		pVol = pIntersect->GetVolume();
		
		FASSERT( pVol );

		if ( !(pInfo->nFlags & FCOLL_DATA_IGNORE_WORLD) && pVol->pWorldGeo )
		{
			// Check collision against the volume mesh
			if ( pInfo->nFlags & FCOLL_DATA_USE_OBJECT_SPHERE )
			{
				bCollCheck = fcoll_TestWorldMeshAndObjectSpheres( pVol->pWorldGeo, pWorldMesh, nBone );
			}
			else if ( pInfo->nFlags & FCOLL_DATA_USE_OBJECT_CAPSULE )
			{
				bCollCheck = fcoll_TestWorldMeshAndObjectCapsules( pVol->pWorldGeo, pWorldMesh, nBone );
			}
			else
			{
				bCollCheck = fcoll_TestWorldMeshAndkDOP( pVol->pWorldGeo, pWorldMesh, pMeshMtx, nBone );
			}

			if ( bCollCheck && (pInfo->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
			{
				FColl_nImpactCollMask |= FColl_nAccumCollMasks;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}
		}
		
		// If the caller wants to ignore objects, do so
		if ( (pInfo->nFlags & (FCOLL_DATA_IGNORE_STATIC_OBJECTS | FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS)) == (FCOLL_DATA_IGNORE_STATIC_OBJECTS | FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS) )
		{
			continue;
		}
		
		// Check the trackers in this volume for collision
		pTrackerIntersect = NULL;
		while ( (pTrackerIntersect = (CFWorldIntersect *)flinklist_GetNext( &pVol->aTrackerIntersects[FWORLD_TRACKERTYPE_MESH], pTrackerIntersect )) ) 
		{
			pTracker = pTrackerIntersect->GetTracker();

			// Don't check against yourself
			if ( pTracker == pWorldMesh )
			{
				continue;
			}

			// Check to see if we've already visited this tracker...
			if ( pTracker->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
			{
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pTracker->m_VisitedKey.FlagAsVisited( nKeyIndex );

			if ( !(pInfo->nFlags & FCOLL_DATA_IGNORE_COLLISION_FLAG) && !pTracker->IsCollisionFlagSet() ) 
			{
				// Tracker isn't collideable...
				continue;
			}

			// Check against user type bits
			if ( !(pTracker->GetUserTypeBits() & pInfo->nTrackerUserTypeBitsMask) )
			{
				continue;
			}

			CFWorldMesh *pMesh = (CFWorldMesh *)pTracker;
			if ( pMesh->m_nFlags & FMESHINST_FLAG_STATIC )
			{
				if ( pInfo->nFlags & FCOLL_DATA_IGNORE_STATIC_OBJECTS )
				{
					continue;
				}
			}
			else if ( pInfo->nFlags & FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS )
			{
				continue;
			}

			// Make sure the tracker is not in the skip list
			for ( nIndex = 0; nIndex < pInfo->nTrackerSkipCount; nIndex++ )
			{
				if ( pTracker == pInfo->ppTrackerSkipList[nIndex] )
				{
					// Tracker is in the skip list, can't collide with this guy
					break;
				}
			}
			if ( nIndex != pInfo->nTrackerSkipCount )
			{
				continue;
			}
			
			// We've found a potentially-intersecting mesh so check collision

			if ( !pTracker->GetBoundingSphere().IsIntersecting( pWorldMesh->GetBoundingSphere() ) )
			{
				// Bounding spheres do not intersect
				continue;
			}

			// See how our caller wants us to test this tracker
			if ( pInfo->pCallback )
			{
				nCallbackReturn = pInfo->pCallback( pTracker );
				if ( nCallbackReturn & FCOLL_CHECK_CB_EXIT_IMMEDIATELY )
				{
					FColl_nImpactCollMask |= FColl_nAccumCollMasks;
					CFWorldKey::CloseKey( nKeyIndex );
					return TRUE;
				}
				else if ( nCallbackReturn & FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER )
				{
					continue;
				}
				else if ( nCallbackReturn & FCOLL_CHECK_CB_FIRST_IMPACT_ONLY )
				{
					pInfo->nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
				}
			}
			else
			{
				nCallbackReturn = FCOLL_CHECK_CB_ALL_IMPACTS;
			}

			if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}

			if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}

			pMesh->CallPreCollCallback();

			// Verify that the callback did not change the tracker counts
			FASSERT( nVolTrackerKey == FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] );

			// Perform the collision check
			if ( pInfo->nFlags & FCOLL_DATA_USE_OBJECT_SPHERE )
			{
				bCollCheck = fcoll_TestMeshAndObjectSpheres( pMesh, pWorldMesh, nBone );
			}
			else if ( pInfo->nFlags & FCOLL_DATA_USE_OBJECT_CAPSULE )
			{
				bCollCheck = fcoll_TestMeshAndObjectCapsules( pMesh, pWorldMesh, nBone );
			}
			else
			{
				bCollCheck = fcoll_TestMeshAndkDOP( pMesh, pWorldMesh, pMeshMtx, nBone );
			}

			// Make sure we return the coll masks to their original value (The mask may have been changed as a result of the callback return)
			pInfo->nStopOnFirstOfCollMask = nOrigStopCollMasks;

			if ( bCollCheck && ((nCallbackReturn & FCOLL_CHECK_CB_EXIT_ON_IMPACT) || (pInfo->nStopOnFirstOfCollMask & FColl_nAccumCollMasks))
				|| (nCallbackReturn & FCOLL_CHECK_CB_EXIT_AFTER_TRACKER) )
			{
				FColl_nImpactCollMask |= FColl_nAccumCollMasks;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}
		}
	}

	FColl_nImpactCollMask |= FColl_nAccumCollMasks;

	CFWorldKey::CloseKey( nKeyIndex );
	
	return (nStartingImpactCount < FColl_nImpactCount);
}


//
//
//
static BOOL _AccumulateVolumesUsingVisCallback( FVisVolume_t *pVolume, BOOL bUseVisData )
{
	FASSERT( _bModuleInitialized );

	if ( _nVolumeIntersectCount > _MAX_VOLUMES_IN_BUFFER || !bUseVisData )
	{
		return FALSE;
	}

	_aVolumeIntersectBuffer[_nVolumeIntersectCount++] = pVolume;

	return TRUE;
}


//
//
//
BOOL fcoll_Check( CFCollData *pInfo, CFSphere *pSphere )
{
	FASSERT( pInfo && pSphere );

	if ( pSphere->m_fRadius < 0.0001f )
	{
		return FALSE;
	}
	
	FColl_nAccumCollMasks = 0;
	
	FColl_pCollData = pInfo;

	// Record the start impact count for later use	
	u32 nStartingImpactCount = FColl_nImpactCount;
	
	CFSphere spTestSphere;

	if ( !FColl_pCollData->pMovement )
	{
		FColl_fMoveMagnitude = 0.f;
	}
	else
	{
		FColl_fMoveMagnitude = FColl_pCollData->pMovement->MagSq();
	}

	_nVolumeIntersectCount = 0;
	if ( FColl_fMoveMagnitude < 0.00001f )
	{
		FColl_fMoveMagnitude = 0.f;
		FColl_vUnitizedMovement.Set( 0.f, 0.f, 0.f );
		spTestSphere.m_Pos = pSphere->m_Pos;
		spTestSphere.m_fRadius = pSphere->m_fRadius + (FColl_fMoveMagnitude * 0.5f);
		fworld_GetVolumesIntersectingSphere( &spTestSphere, _AccumulateVolumesUsingVisCallback, pInfo->pLocationHint );
	}
	else
	{
		FColl_fMoveMagnitude = fmath_Sqrt( FColl_fMoveMagnitude );
		FColl_vUnitizedMovement.Mul( *FColl_pCollData->pMovement, fmath_Inv( FColl_fMoveMagnitude ) );
		spTestSphere.m_Pos = pSphere->m_Pos + (pInfo->pMovement->v3 * 0.5f);
		spTestSphere.m_fRadius = pSphere->m_fRadius + (FColl_fMoveMagnitude * 0.5f);
		fworld_GetVolumesIntersectingProjectedSphere( pSphere, pInfo->pMovement, FColl_fMoveMagnitude, _AccumulateVolumesUsingVisCallback, pInfo->pLocationHint );
	}

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH];

	// Walk through the trackers in each volume determining collision
	BOOL bCollCheck;
	FVisVolume_t *pVol;
	CFWorldTracker *pTracker;
	CFWorldIntersect *pIntersect;
	u16 nIndex, nOrigStopCollMasks = pInfo->nStopOnFirstOfCollMask;
	u32 i, nCallbackReturn, nKeyIndex = CFWorldKey::OpenKey();
	for ( i = 0; i < _nVolumeIntersectCount; i++ )
	{
		pVol = _aVolumeIntersectBuffer[i];

		if ( !(pInfo->nFlags & FCOLL_DATA_IGNORE_WORLD) && pVol->pWorldGeo )
		{
			// Check collision against the volume mesh
			if ( fcoll_TestWorldMeshAndSphere( pVol->pWorldGeo, pSphere ) && (pInfo->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
			{
				FColl_nImpactCollMask |= FColl_nAccumCollMasks;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}
		}
		
		// If the caller wants to ignore objects, do so
		if ( (pInfo->nFlags & (FCOLL_DATA_IGNORE_STATIC_OBJECTS | FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS)) == (FCOLL_DATA_IGNORE_STATIC_OBJECTS | FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS) )
		{
			continue;
		}
		
		// Check the trackers in this volume for collision
		pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( &pVol->aTrackerIntersects[FWORLD_TRACKERTYPE_MESH], pIntersect )) ) 
		{
			pTracker = pIntersect->GetTracker();

			// Check to see if we've already visited this tracker...
			if ( pTracker->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
			{
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pTracker->m_VisitedKey.FlagAsVisited( nKeyIndex );

			if ( !(pInfo->nFlags & FCOLL_DATA_IGNORE_COLLISION_FLAG) && !pTracker->IsCollisionFlagSet() ) 
			{
				// Tracker isn't collideable...
				continue;
			}

			// Check against user type bits
			if ( !(pTracker->GetUserTypeBits() & pInfo->nTrackerUserTypeBitsMask) )
			{
				continue;
			}

			CFWorldMesh *pMesh = (CFWorldMesh *)pTracker;
			if ( pMesh->m_nFlags & FMESHINST_FLAG_STATIC )
			{
				if ( pInfo->nFlags & FCOLL_DATA_IGNORE_STATIC_OBJECTS )
				{
					continue;
				}
			}
			else if ( pInfo->nFlags & FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS )
			{
				continue;
			}
			
			// Make sure the tracker is not in the skip list
			for ( nIndex = 0; nIndex < pInfo->nTrackerSkipCount; nIndex++ )
			{
				if ( pTracker == pInfo->ppTrackerSkipList[nIndex] )
				{
					// Tracker is in the skip list, can't collide with this guy
					break;
				}
			}
			if ( nIndex != pInfo->nTrackerSkipCount )
			{
				continue;
			}
			
			// We've found a potentially-intersecting mesh so check collision

			if ( !pTracker->GetBoundingSphere().IsIntersecting( spTestSphere ) )
			{
				// Bounding spheres do not intersect
				continue;
			}

			// See how our caller wants us to test this tracker
			if ( pInfo->pCallback )
			{
				nCallbackReturn = pInfo->pCallback( pTracker );
				if ( nCallbackReturn & FCOLL_CHECK_CB_EXIT_IMMEDIATELY )
				{
					FColl_nImpactCollMask |= FColl_nAccumCollMasks;
					CFWorldKey::CloseKey( nKeyIndex );
					return TRUE;
				}
				else if ( nCallbackReturn & FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER )
				{
					continue;
				}
				else if ( nCallbackReturn & FCOLL_CHECK_CB_FIRST_IMPACT_ONLY )
				{
					pInfo->nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
				}
			}
			else
			{
				nCallbackReturn = FCOLL_CHECK_CB_ALL_IMPACTS;
			}

			if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}

			pMesh->CallPreCollCallback();

			if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}

			// Perform the collision check against this mesh
			bCollCheck = fcoll_TestMeshAndSphere( pMesh, pSphere );

			// Make sure we return the coll masks to their original value (The mask may have been changed as a result of the callback return)
			pInfo->nStopOnFirstOfCollMask = nOrigStopCollMasks;

			if ( bCollCheck && ((nCallbackReturn & FCOLL_CHECK_CB_EXIT_ON_IMPACT) || (pInfo->nStopOnFirstOfCollMask & FColl_nAccumCollMasks))
				|| (nCallbackReturn & FCOLL_CHECK_CB_EXIT_AFTER_TRACKER) )
			{
				FColl_nImpactCollMask |= FColl_nAccumCollMasks;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}
		}
	}

	FColl_nImpactCollMask |= FColl_nAccumCollMasks;

	CFWorldKey::CloseKey( nKeyIndex );
	
	return (nStartingImpactCount < FColl_nImpactCount);
}


//
//
//
BOOL fcoll_Check( CFCollData *pInfo, CFCapsule *pCapsule )
{
	FASSERT( pInfo && pCapsule );

	if ( pCapsule->m_fRadius < 0.0001f )
	{
		return FALSE;
	}
	
	FColl_nAccumCollMasks = 0;
	
	FColl_pCollData = pInfo;

	CFVec3A vDiff;
	vDiff.Sub( pCapsule->m_vPoint1, pCapsule->m_vPoint2 );
	f32 fDist = vDiff.MagSq();
	if ( fDist <= 0.000001f )
	{
		return FALSE;
	}

	// Record the start impact count for later use	
	u32 nStartingImpactCount = FColl_nImpactCount;
	
	CFSphere spTestSphere, spStartSphere, spEndSphere;

	fDist = fmath_Sqrt( fDist );
	spStartSphere.m_Pos = (pCapsule->m_vPoint1.v3 + pCapsule->m_vPoint2.v3) * 0.5f;
	spStartSphere.m_fRadius = pCapsule->m_fRadius + (fDist * 0.5f);

	if ( !FColl_pCollData->pMovement )
	{
		FColl_fMoveMagnitude = 0.f;
	}
	else
	{
		FColl_fMoveMagnitude = FColl_pCollData->pMovement->MagSq();
	}

	_nVolumeIntersectCount = 0;
	if ( FColl_fMoveMagnitude < 0.0001f )
	{
		FColl_vUnitizedMovement.Set( 0.f, 0.f, 0.f );
		FColl_fMoveMagnitude = 0.f;
		spTestSphere.m_Pos = spStartSphere.m_Pos;
		spTestSphere.m_fRadius = spStartSphere.m_fRadius + FColl_fMoveMagnitude;
		fworld_GetVolumesIntersectingSphere( &spTestSphere, _AccumulateVolumesUsingVisCallback, pInfo->pLocationHint );
	}
	else
	{
		FColl_fMoveMagnitude = fmath_Sqrt( FColl_fMoveMagnitude );
		FColl_vUnitizedMovement.Mul( *FColl_pCollData->pMovement, fmath_Inv( FColl_fMoveMagnitude ) );
		spTestSphere.m_Pos = spStartSphere.m_Pos + (pInfo->pMovement->v3 * 0.5f);
		spTestSphere.m_fRadius = spStartSphere.m_fRadius + FColl_fMoveMagnitude;
		fworld_GetVolumesIntersectingProjectedSphere( &spStartSphere, pInfo->pMovement, FColl_fMoveMagnitude, _AccumulateVolumesUsingVisCallback, pInfo->pLocationHint );
	}

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH];

	// Walk through the trackers in each volume determining collision
	BOOL bCollCheck;
	FVisVolume_t *pVol;
	CFWorldTracker *pTracker;
	CFWorldIntersect *pIntersect;
	u16 nIndex, nOrigStopCollMasks = pInfo->nStopOnFirstOfCollMask;
	u32 i, nCallbackReturn, nKeyIndex = CFWorldKey::OpenKey();
	for ( i = 0; i < _nVolumeIntersectCount; i++ )
	{
		pVol = _aVolumeIntersectBuffer[i];

		if ( !(pInfo->nFlags & FCOLL_DATA_IGNORE_WORLD) && pVol->pWorldGeo )
		{
			// Check collision against the volume mesh
			if ( fcoll_TestWorldMeshAndCapsule( pVol->pWorldGeo, pCapsule ) && (pInfo->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
			{
				FColl_nImpactCollMask |= FColl_nAccumCollMasks;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}
		}
		
		// If the caller wants to ignore objects, do so
		if ( (pInfo->nFlags & (FCOLL_DATA_IGNORE_STATIC_OBJECTS | FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS)) == (FCOLL_DATA_IGNORE_STATIC_OBJECTS | FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS) )
		{
			continue;
		}
		
		// Check the trackers in this volume for collision
		pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( &pVol->aTrackerIntersects[FWORLD_TRACKERTYPE_MESH], pIntersect )) ) 
		{
			pTracker = pIntersect->GetTracker();

			// Check to see if we've already visited this tracker...
			if ( pTracker->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
			{
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pTracker->m_VisitedKey.FlagAsVisited( nKeyIndex );

			if ( !(pInfo->nFlags & FCOLL_DATA_IGNORE_COLLISION_FLAG) && !pTracker->IsCollisionFlagSet() ) 
			{
				// Tracker isn't collideable...
				continue;
			}

			// Check against user type bits
			if ( !(pTracker->GetUserTypeBits() & pInfo->nTrackerUserTypeBitsMask) )
			{
				continue;
			}

			CFWorldMesh *pMesh = (CFWorldMesh *)pTracker;
			if ( pMesh->m_nFlags & FMESHINST_FLAG_STATIC )
			{
				if ( pInfo->nFlags & FCOLL_DATA_IGNORE_STATIC_OBJECTS )
				{
					continue;
				}
			}
			else if ( pInfo->nFlags & FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS )
			{
				continue;
			}
			
			// Make sure the tracker is not in the skip list
			for ( nIndex = 0; nIndex < pInfo->nTrackerSkipCount; nIndex++ )
			{
				if ( pTracker == pInfo->ppTrackerSkipList[nIndex] )
				{
					// Tracker is in the skip list, can't collide with this guy
					break;
				}
			}
			if ( nIndex != pInfo->nTrackerSkipCount )
			{
				continue;
			}
			
			// We've found a potentially-intersecting mesh so check collision

			if ( !pTracker->GetBoundingSphere().IsIntersecting( spTestSphere ) )
			{
				// Bounding spheres do not intersect
				continue;
			}

			// See how our caller wants us to test this tracker
			if ( pInfo->pCallback )
			{
				nCallbackReturn = pInfo->pCallback( pTracker );
				if ( nCallbackReturn & FCOLL_CHECK_CB_EXIT_IMMEDIATELY )
				{
					FColl_nImpactCollMask |= FColl_nAccumCollMasks;
					CFWorldKey::CloseKey( nKeyIndex );
					return TRUE;
				}
				else if ( nCallbackReturn & FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER )
				{
					continue;
				}
				else if ( nCallbackReturn & FCOLL_CHECK_CB_FIRST_IMPACT_ONLY )
				{
					pInfo->nStopOnFirstOfCollMask = FCOLL_MASK_CHECK_ALL;
				}
			}
			else
			{
				nCallbackReturn = FCOLL_CHECK_CB_ALL_IMPACTS;
			}

			if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}

			pMesh->CallPreCollCallback();

			if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}

			// Perform the collision check against this mesh
			bCollCheck = fcoll_TestMeshAndCapsule( pMesh, pCapsule );

			// Make sure we return the coll masks to their original value (The mask may have been changed as a result of the callback return)
			pInfo->nStopOnFirstOfCollMask = nOrigStopCollMasks;

			if ( bCollCheck && ((nCallbackReturn & FCOLL_CHECK_CB_EXIT_ON_IMPACT) || (pInfo->nStopOnFirstOfCollMask & FColl_nAccumCollMasks))
				|| (nCallbackReturn & FCOLL_CHECK_CB_EXIT_AFTER_TRACKER) )
			{
				FColl_nImpactCollMask |= FColl_nAccumCollMasks;
				CFWorldKey::CloseKey( nKeyIndex );
				return TRUE;
			}
		}
	}

	FColl_nImpactCollMask |= FColl_nAccumCollMasks;

	CFWorldKey::CloseKey( nKeyIndex );
	
	return (nStartingImpactCount < FColl_nImpactCount);
}


/*
//
//
//
BOOL fcoll_Check( CFCollProjInfo *pInfo, CFCapsule *pCapsule )
{
}


//
//
//
BOOL fcoll_Check( CFCollProjInfo *pInfo, CFBox *pBox )
{
}
*/

