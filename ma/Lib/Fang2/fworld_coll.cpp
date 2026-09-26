//////////////////////////////////////////////////////////////////////////////////////
// fworld_coll.h - Fang module to handle world collision requests.
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
// 09/02/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fworld_coll.h"
#include "fmesh_coll.h"
#include "fclib.h"
#include "fdraw.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local defines
//////////////////////////////////////////////////////////////////////////////////////

#define	_STATICF	static

//////////////////////////////////////////////////////////////////////////////////////
// Local structure and classes
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
FCLASS_ALIGN_PREFIX struct _RayWalker
{
	const CFVec3A *pStart;
	const CFVec3A *pEnd;
	CFVec3A vUnitDir;
	FWorldVolCallbackFcn_t *pCallbackFcn;
	u32 nKeyIndex;
	u32 nPortalFlags;
	
	FCLASS_STACKMEM_ALIGN( _RayWalker );
		
} FCLASS_ALIGN_SUFFIX;


//
//
//
FCLASS_NOALIGN_PREFIX struct _SphereWalker
{
	const CFSphere *pSphere;
	f32 fRadiusSq;
	FWorldVolCallbackFcn_t *pCallbackFcn;
	u32 nKeyIndex;
	u32 nPortalFlags;
	
	FCLASS_STACKMEM_NOALIGN( _SphereWalker );
		
} FCLASS_NOALIGN_SUFFIX;


//
//
//
struct _CollidedTracker_t
{
	CFWorldTracker *pTracker;		// The tracker that the ray collided with
	CFVec3 IntersectionPoint_WS;	// The point of intersection
	f32 fUnitDistToIntersection;	// Unit distance from the start point to the point of intersection, along the ray
};




//////////////////////////////////////////////////////////////////////////////////////
// Global vars
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Local Global vars
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized = FALSE;
static FVisVolume_t *_pEndVol;
static BOOL _bReachedEnd;

// Frequently used CollInfo's with static parameters
static CFCollInfo _LOSCollInfo;
static CFCollInfo _CIRSCollInfo;

static CFCollInfo *_pCollInfo;

// Tracker CollInfo tracking
static FWorldTrackerType_e _nCollideTrackerType;
static const CFTrackerCollideInfo *_pTrackerCollideInfo;
static const CFTrackerCollideSpheresInfo *_pCollideSpheresInfo;
static const CFTrackerCollideProjSphereInfo *_pCollideProjSphereInfo;
//static CFTrackerCollideRayInfo _CollideRayInfo;
static CFVec3 *_pCollideRay_IntersectionPoint;
static f32 *_pfCollideRay_UnitDistToIntersection;
static s32 _nCollideRay_KeyIndex;

static _CollidedTracker_t *_aCollidedTrackers;
static _CollidedTracker_t **_apCollidedTrackers;
static u32 _nCollidedTrackerMax;
static u32 _nCollidedTrackerCount;

static const CFVec3 *_pCollidePoint_WS;
static FWorldIntersectingTrackerCallback_t *_pCollidePointCallback;

static CFCollInfo _CollInfo;
static u32 _nTrackerSkipCount;
static const CFWorldTracker * const *_ppTrackerSkipList;

static FVisVolume_t **_papVolumeIntersectBuffer;
static u32 _nVolumeIntersectCount;
static u32 _nMaxVolumeIntersects;


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes
//////////////////////////////////////////////////////////////////////////////////////

_STATICF f32  _IntersectNodeAndPoint( u16 nIndex, const CFVec3A &vPos, u16 *pnResult );
_STATICF u32  _IntersectNodeAndRay( const CFVec3A *pStart, const CFVec3A *pEnd, FWorldVolCallbackFcn_t *pCallbackFcn, u32 nCellIdx, u32 nKeyIndex, BOOL &bEarlyOut );
_STATICF u32  _IntersectNodeAndSphere( const CFSphere *pSphere, FWorldVolCallbackFcn_t *pCallbackFcn, u32 nCellIdx, u32 nKeyIndex, BOOL &bEarlyOut );
_STATICF BOOL _WalkVis_Ray( const _RayWalker *pWalker, FVisVolume_t *pVolume, u32 nEnteredFromPortal );
_STATICF BOOL _WalkVis_Sphere( const _SphereWalker *pWalker, FVisVolume_t *pVolume, u32 nEnteredFromPortal );
_STATICF BOOL _AccumulateVolumesCallback( FVisVolume_t *pVolume, BOOL bUseVisData );
_STATICF BOOL _AccumulateVolumesUsingVisCallback( FVisVolume_t *pVolume, BOOL bUseVisData );

_STATICF BOOL _FindIntersectingTrackers( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback, FWorldTrackerType_e nTypeToFind, s32 nKeyIndex );
_STATICF BOOL _FindIntersectingTrackers( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback, s32 nKeyIndex );
_STATICF FWorldTrackerType_e _ComputeSingleTrackerTypeFromBitList( u32 nTrackerBits );
_STATICF BOOL _CollideWithWorldTrisCallback( FVisVolume_t *pVolume, BOOL bUsingVisData );
_STATICF BOOL _CollideSphereWithTrackersCallback( FVisVolume_t *pVolume, BOOL bUsingVisData );// CFWorldTracker *pTracker, FVisVolume_t *pVolume );
_STATICF BOOL _CollideProjSphereWithTrackersCallback( FVisVolume_t *pVolume, BOOL bUsingVisData );//  CFWorldTracker *pTracker, FVisVolume_t *pVolume ) ;
_STATICF BOOL _CollideRayWithTrackersCallback1( FVisVolume_t *pVolume, BOOL bUsingVisData );
_STATICF BOOL _CollideRayWithTrackersCallback2( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
_STATICF BOOL _CollideRayWithTrackersCallback3( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fworld_coll_Startup( void )
{
	// Common Line Of Sight collision info parameters
	_LOSCollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	_LOSCollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_OBSTRUCT_LINE_OF_SIGHT;
	_LOSCollInfo.bFindClosestImpactOnly = FALSE;
	_LOSCollInfo.nCollMask = FCOLL_MASK_OBSTRUCT_LINE_OF_SIGHT;
	_LOSCollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	_LOSCollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	_LOSCollInfo.bCullBacksideCollisions = TRUE;

	// Common Closest Impact to Ray Start collision info parameters
	_CIRSCollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
	_CIRSCollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	_CIRSCollInfo.bFindClosestImpactOnly = TRUE;
	_CIRSCollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
	_CIRSCollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	_CIRSCollInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;

	_nTrackerSkipCount = 0;
	_ppTrackerSkipList = NULL; 

	// Allocate an array of pointers to hold the cell collision buffer
	_nMaxVolumeIntersects = Fang_ConfigDefs.nWorld_MaxIntersects;
	_papVolumeIntersectBuffer = (FVisVolume_t **)fres_AllocAndZero( _nMaxVolumeIntersects * sizeof(FVisVolume_t *) );
	if ( _papVolumeIntersectBuffer == NULL ) 
	{
		DEVPRINTF( "fvis_ModuleStartup(): Insufficient memory to allocate %u bytes for Draw Mesh Buffer.\n", _nMaxVolumeIntersects * sizeof(FVisVolume_t *) );
		return FALSE;
	}
	_nVolumeIntersectCount = 0;

	_nCollidedTrackerMax = Fang_ConfigDefs.nWorld_MaxSortedObjectsPerNode;
	_aCollidedTrackers = (_CollidedTracker_t *)fres_AllocAndZero( _nCollidedTrackerMax * sizeof(_CollidedTracker_t) );
	if( _aCollidedTrackers == NULL ) 
	{
		DEVPRINTF( "fworld_coll_Startup(): Insufficient memory to allocate %u bytes for world tracker collision buffer.\n", _nCollidedTrackerMax * sizeof(_CollidedTracker_t) );
		return FALSE;
	}
	_apCollidedTrackers = (_CollidedTracker_t **)fres_AllocAndZero( _nCollidedTrackerMax * sizeof(_CollidedTracker_t *) );
	if( _apCollidedTrackers == NULL ) 
	{
		DEVPRINTF( "fworld_coll_Startup(): Insufficient memory to allocate %u bytes for world tracker collision pointer buffer.\n", _nCollidedTrackerMax * sizeof(_CollidedTracker_t *) );
		return FALSE;
	}
	_nCollidedTrackerCount = 0;

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
BOOL fworld_coll_Shutdown( void )
{
	_bModuleInitialized = FALSE;

	return TRUE;
}


//
//
//
FINLINE BOOL _IsThereQuickOut( CFWorldTracker *pTracker )
{
	u16 nIndex;

	if( !_pTrackerCollideInfo->bIgnoreCollisionFlag && !pTracker->IsCollisionFlagSet() ) 
	{
		// Tracker isn't collideable...
		return TRUE;
	}

	if( !( (1<<pTracker->GetType()) & _pTrackerCollideInfo->nTrackerTypeBits ) ) 
	{
		// Not permitted to collide with this tracker type...
		return TRUE;
	}

	// Check the tracker's user type bits against the user bits provided
	if ( !(pTracker->GetUserTypeBits() & _pTrackerCollideInfo->nTrackerUserTypeBitsMask) )
	{
		// Not permitted to collide with this tracker type...
		return TRUE;
	}

	// Make sure the tracker is not in the skip list
	for ( nIndex = 0; nIndex < _pTrackerCollideInfo->nTrackerSkipCount; nIndex++ )
	{
		if ( pTracker == _pTrackerCollideInfo->ppTrackerSkipList[nIndex] )
		{
			// Tracker is in the skip list, can't collide with this guy
			return TRUE;
		}
	}

	return FALSE;
}


//
// Return FALSE if the callback signalled to bail early.
//
BOOL fworld_FindTrackersIntersectingSphere( const CFSphere *pSphere_WS, 
											FWorldTrackerType_e nTypeToFind, 
											FWorldIntersectingTrackerCallback_t *pTrackerCallback, 
											u32 nTrackerSkipCount/*=0*/, 
											const CFWorldTracker * const *ppTrackerSkipList/*=NULL*/, 
											const CFWorldTracker *pStartHint/*=NULL*/, 
											u32 nUserType/*=-1*/,
											u64 nUserTypeBitMask/*=0xffffffffffffffff*/, 
											u32 nUsePortalsFlag/*=FVIS_PORTAL_FLAG_VISIBILITY*/ )
{
	FASSERT( nTypeToFind >= 0 && nTypeToFind < FWORLD_TRACKERTYPE_COUNT );
	FASSERT( pSphere_WS && pTrackerCallback );
	
	_nVolumeIntersectCount = 0;
	_pEndVol = NULL;
	
	fworld_GetVolumesIntersectingSphere( pSphere_WS, _AccumulateVolumesCallback, pStartHint, -1, nUsePortalsFlag );

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[nTypeToFind];

	// Walk through the trackers in each volume determining collision
	FVisVolume_t *pVol;
	CFWorldTracker *pTracker;
	CFWorldIntersect *pIntersect;
	u32 i, j, nKeyIndex = CFWorldKey::OpenKey();
	for ( i = 0; i < _nVolumeIntersectCount; i++ )
	{
		pVol = _papVolumeIntersectBuffer[i];

		// Check the trackers in this volume for collision
		pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( &pVol->aTrackerIntersects[nTypeToFind], pIntersect )) ) 
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

			// Check user type...
			if( (nUserType != -1) && (nUserType != pTracker->m_nUser) )
			{
				continue; 
			}

			// Check against user type bits
			if ( !(pTracker->GetUserTypeBits() & nUserTypeBitMask) )
			{
				continue;
			}

			// Make sure the tracker is not in the skip list
			for ( j = 0; j < nTrackerSkipCount; j++ )
			{
				if ( pTracker == ppTrackerSkipList[j] )
				{
					// Tracker is in the skip list, can't collide with this guy
					break;
				}
			}
			if ( j != nTrackerSkipCount )
			{
				continue;
			}

			// We've found a potentially-intersecting tracker so check collision

			if ( pTracker->GetBoundingSphere().IsIntersecting( *pSphere_WS ) ) 
			{
				// This tracker intersects, so return it
				if ( !pTrackerCallback( pTracker, pVol ) )
				{
					// Callback function requested that we quit searching
					CFWorldKey::CloseKey( nKeyIndex );
					return FALSE;
				}					
				else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[nTypeToFind] )
				{
					// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
					FASSERT_NOW;
					CFWorldKey::CloseKey( nKeyIndex );
					return TRUE;
				}
			}
		}
	}

	CFWorldKey::CloseKey( nKeyIndex );
	
	return TRUE;
}


//
// Return FALSE if the callback signalled to bail early.
//
_STATICF BOOL _FindTrackersOfTypeInVolume( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback, FWorldTrackerType_e nTypeToFind ) 
{
	FASSERT( _bModuleInitialized );

	if( pVolume->anTotalTrackerCount[nTypeToFind] == 0 ) 
	{
		// No trackers of the desired type in this world node...
		return TRUE;
	}

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[nTypeToFind];

	FLinkRoot_t *pRootIntersect = &pVolume->aTrackerIntersects[nTypeToFind];

	CFWorldIntersect *pIntersect = NULL;
	while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
	{
		CFWorldTracker *pTracker = pIntersect->GetTracker();

		// Check to see if we've already visited this tracker...
		if ( pTracker->m_VisitedKey.HaveVisited( _nCollideRay_KeyIndex ) ) 
		{
			// We've already visited the intersecting tracker, so let's skip it...
			continue;
		}

		// Flag as visited...
		pTracker->m_VisitedKey.FlagAsVisited( _nCollideRay_KeyIndex );

		// Check CollInfo for quick out
		if ( _IsThereQuickOut( pTracker ) ) 
		{
			continue;
		}

		// We've found a potentially-intersecting tracker!

		if ( !pCallback( pTracker, pIntersect->GetVolume() ) ) 
		{
			// Caller returned FALSE, which means we should not continue this function...
			return FALSE;
		}
		else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[nTypeToFind] )
		{
			// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
			FASSERT_NOW;
			return TRUE;
		}
	}

	return TRUE;
}


//
// Return FALSE if the callback signalled to bail early.
//
_STATICF BOOL _FindTrackersInVolume( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback ) 
{
	FASSERT( _bModuleInitialized );

	u32 i;
	for ( i = 0; i < FWORLD_TRACKERTYPE_COUNT; i++ )
	{
		FLinkRoot_t *pRootIntersect = &pVolume->aTrackerIntersects[i];

		u32 nVolTrackerKey = FVis_anVolumeTrackerKey[i];

		CFWorldIntersect *pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
		{
			CFWorldTracker *pTracker = pIntersect->GetTracker();

			// Check to see if we've already visited this tracker...
			if ( pTracker->m_VisitedKey.HaveVisited( _nCollideRay_KeyIndex ) ) 
			{
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pTracker->m_VisitedKey.FlagAsVisited( _nCollideRay_KeyIndex );

			// Check CollInfo for quick out
			if ( _IsThereQuickOut( pTracker ) ) 
			{
				continue;
			}

			// We've found a potentially-intersecting tracker!

			if ( !pCallback( pTracker, pIntersect->GetVolume() ) ) 
			{
				// Caller returned FALSE, which means we should not continue this function...
				return FALSE;
			}
			else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[i] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				return TRUE;
			}
		}
	}

	return TRUE;
}


//
// Return FALSE if the callback signalled to bail early.
//
BOOL fworld_FindTrackersIntersectingVolume( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback, FWorldTrackerType_e nTypeToFind ) 
{
	FASSERT( _bModuleInitialized );

	BOOL bRetValue = TRUE;

	if( pVolume->anTotalTrackerCount[nTypeToFind] == 0 ) 
	{
		// No trackers of the desired type in this world node...
		return TRUE;
	}

	#if FANG_DEBUG_BUILD
		s32 nKeyIndex = CFWorldKey::OpenKey();
		if( nKeyIndex == -1 ) 
		{
			return FALSE;
		}
	#endif // FANG_DEBUG_BUILD

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[nTypeToFind];

	FLinkRoot_t *pRootIntersect = &pVolume->aTrackerIntersects[nTypeToFind];

	CFWorldIntersect *pIntersect = NULL;
	while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
	{
		CFWorldTracker *pTracker = pIntersect->GetTracker();

		#if FANG_DEBUG_BUILD
			// Check to see if we've already visited this tracker...
			if ( pTracker->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
			{
				// The same tracker should not be added to the volume twice
				FASSERT_NOW;
			}

			// Flag as visited...
			pTracker->m_VisitedKey.FlagAsVisited( nKeyIndex );
		#endif // FANG_DEBUG_BUILD

		// We've found a potentially-intersecting tracker!

		if ( !pCallback( pTracker, pIntersect->GetVolume() ) ) 
		{
			// Caller returned FALSE, which means we should not continue this function...
			bRetValue = FALSE;
			break;
		}
		else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[nTypeToFind] )
		{
			// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
			FASSERT_NOW;
			bRetValue = TRUE;
			break;
		}
	}

	#if FANG_DEBUG_BUILD
		CFWorldKey::CloseKey( nKeyIndex );
	#endif // FANG_DEBUG_BUILD

	return bRetValue;
}


//
// Return FALSE if the callback signalled to bail early.
//
BOOL fworld_FindTrackersIntersectingVolume( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback ) 
{
	FASSERT( _bModuleInitialized );

	BOOL bRetValue = TRUE;

	#if FANG_DEBUG_BUILD
		s32 nKeyIndex = CFWorldKey::OpenKey();
		if( nKeyIndex == -1 ) 
		{
			return FALSE;
		}
	#endif // FANG_DEBUG_BUILD

	u32 i;
	for ( i = 0; i < FWORLD_TRACKERTYPE_COUNT; i++ )
	{
		FLinkRoot_t *pRootIntersect = &pVolume->aTrackerIntersects[i];

		u32 nVolTrackerKey = FVis_anVolumeTrackerKey[i];

		CFWorldIntersect *pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
		{
			CFWorldTracker *pTracker = pIntersect->GetTracker();

			#if FANG_DEBUG_BUILD
				// Check to see if we've already visited this tracker...
				if ( pTracker->m_VisitedKey.HaveVisited( nKeyIndex ) ) 
				{
					// A tracker shouldn't exist in a volume more than once
					FASSERT_NOW;
				}

				// Flag as visited...
				pTracker->m_VisitedKey.FlagAsVisited( nKeyIndex );
			#endif // FANG_DEBUG_BUILD

			// We've found a potentially-intersecting tracker!

			if ( !pCallback( pTracker, pIntersect->GetVolume() ) ) 
			{
				// Caller returned FALSE, which means we should not continue this function...
				bRetValue = FALSE;
				break;
			}
			else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[i] )
			{
				// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
				FASSERT_NOW;
				bRetValue = TRUE;
				break;
			}
		}

		if ( pIntersect != NULL )
		{
			// Callback returned FALSE
			break;
		}
	}

	#if FANG_DEBUG_BUILD
		CFWorldKey::CloseKey( nKeyIndex );
	#endif // FANG_DEBUG_BUILD

	return bRetValue;
}


//
// Returns TRUE if FColl_nImpactCount changed during this call.
//
BOOL fworld_CollideWithWorldTris( CFCollInfo *pCollInfo, const CFWorldTracker *pStartHint/*=NULL*/ ) 
{
	FASSERT( _bModuleInitialized );

	u32 nPrevImpactCount;

	#if FPERF_ENABLE
		FPerf_Timer.Reset();
	#endif

	nPrevImpactCount = FColl_nImpactCount;
	_pCollInfo = pCollInfo;

	switch( pCollInfo->nCollTestType ) 
	{
		case FMESH_COLLTESTTYPE_SPHERE:

			fworld_GetVolumesIntersectingSphere( pCollInfo->pSphere_WS, _CollideWithWorldTrisCallback, pStartHint );

			break;

		case FMESH_COLLTESTTYPE_RAY:

			fworld_GetVolumesIntersectingRay( &pCollInfo->Ray.vStart_WS, &pCollInfo->Ray.vEnd_WS, _CollideWithWorldTrisCallback, pStartHint );

			break;

		case FMESH_COLLTESTTYPE_PROJSPHERE:
		{
			CFSphere spTemp;
			spTemp.m_Pos = pCollInfo->ProjSphere.m_vCenterStart_WS.v3;
			spTemp.m_fRadius = pCollInfo->ProjSphere.m_fRadius;
			fworld_GetVolumesIntersectingProjectedSphere( &spTemp, &pCollInfo->ProjSphere.m_vMoveDelta_WS, pCollInfo->ProjSphere.m_fMoveDistance, _CollideWithWorldTrisCallback, pStartHint );

			break;
		}
		default:
			FASSERT_NOW;
	}

	#if FPERF_ENABLE
		FPerf_fCollWithWorldTris += FPerf_Timer.SampleSeconds();
	#endif

	return (FColl_nImpactCount > nPrevImpactCount);
}


//
// Return FALSE if the callback signalled to bail early.
//
BOOL fworld_CollideWithTrackers( const CFTrackerCollideInfo *pCollideInfo, const CFWorldTracker *pStartHint/*=NULL*/ ) 
{
	FASSERT( _bModuleInitialized );
	
	BOOL bRetValue = TRUE;
	
	_pTrackerCollideInfo = pCollideInfo;

	switch ( pCollideInfo->GetType() )
	{
	
		case FWORLD_COLL_TRACKER_TYPE_RAY:
		{
			CFVec3 IntersectionPoint;
			f32 fUnitDistToIntersection;

			if ( ((CFTrackerCollideRayInfo *)pCollideInfo)->bSort ) 
			{
				((CFTrackerCollideRayInfo *)pCollideInfo)->bComputeIntersection = TRUE;
			}

			_pfCollideRay_UnitDistToIntersection = &fUnitDistToIntersection;
			if ( ((CFTrackerCollideRayInfo *)pCollideInfo)->bComputeIntersection ) 
			{
				_pCollideRay_IntersectionPoint = &IntersectionPoint;
			} 
			else 
			{
				fUnitDistToIntersection = 0.0f;
				_pCollideRay_IntersectionPoint = NULL;
			}

			_nCollideTrackerType = _ComputeSingleTrackerTypeFromBitList( ((CFTrackerCollideRayInfo *)pCollideInfo)->nTrackerTypeBits );

			_nCollideRay_KeyIndex = CFWorldKey::OpenKey();
			if ( _nCollideRay_KeyIndex == -1 ) 
			{
				return TRUE;
			}

			bRetValue = fworld_GetVolumesIntersectingRay( &((CFTrackerCollideRayInfo *)pCollideInfo)->StartPoint_WS, &((CFTrackerCollideRayInfo *)pCollideInfo)->EndPoint_WS, _CollideRayWithTrackersCallback1, pStartHint, _nCollideRay_KeyIndex );

			CFWorldKey::CloseKey( _nCollideRay_KeyIndex );

			break;
		}
		
		case FWORLD_COLL_TRACKER_TYPE_SPHERES:
		{
			_pCollideSpheresInfo = (CFTrackerCollideSpheresInfo *)pCollideInfo;

			_nCollideTrackerType = _ComputeSingleTrackerTypeFromBitList( _pCollideSpheresInfo->nTrackerTypeBits );

			_nCollideRay_KeyIndex = CFWorldKey::OpenKey();
			if ( _nCollideRay_KeyIndex == -1 ) 
			{
				return TRUE;
			}

			bRetValue = fworld_GetVolumesIntersectingSphere( _pCollideSpheresInfo->pMasterSphere_WS, _CollideSphereWithTrackersCallback, pStartHint, _nCollideRay_KeyIndex );

			CFWorldKey::CloseKey( _nCollideRay_KeyIndex );

			break;
		}
		
		case FWORLD_COLL_TRACKER_TYPE_PROJSPHERE:
		{
			_pCollideProjSphereInfo = (CFTrackerCollideProjSphereInfo *)pCollideInfo;

			_nCollideTrackerType = _ComputeSingleTrackerTypeFromBitList( _pCollideProjSphereInfo->nTrackerTypeBits );

			_nCollideRay_KeyIndex = CFWorldKey::OpenKey();
			if ( _nCollideRay_KeyIndex == -1 ) 
			{
				return TRUE;
			}

			CFSphere spTemp;
			spTemp.m_Pos = _pCollideProjSphereInfo->pProjSphere->m_vCenterStart_WS.v3;
			spTemp.m_fRadius = _pCollideProjSphereInfo->pProjSphere->m_fRadius;
			bRetValue = fworld_GetVolumesIntersectingProjectedSphere( &spTemp, &_pCollideProjSphereInfo->pProjSphere->m_vMoveDelta_WS, 
												_pCollideProjSphereInfo->pProjSphere->m_fMoveDistance, _CollideProjSphereWithTrackersCallback, 
												pStartHint, _nCollideRay_KeyIndex );

			CFWorldKey::CloseKey( _nCollideRay_KeyIndex );

			break;
		}
		
		default:
			FASSERT_NOW;
			break;
	}

	return bRetValue;
}


//
// Returns FWORLD_TRACKERTYPE_COUNT if multiple bits are set in nTrackerBits (or if no bits are set).
// Otherwise, returns the tracker type for the one and only set bit.
_STATICF FWorldTrackerType_e _ComputeSingleTrackerTypeFromBitList( u32 nTrackerBits ) 
{
	FASSERT( _bModuleInitialized );

	FWorldTrackerType_e nTrackerType;
	u32 i, nWalkingBit;

	nTrackerType = FWORLD_TRACKERTYPE_COUNT;

	// Find first set bit...
	for( i=0, nWalkingBit=1; i<FWORLD_TRACKERTYPE_COUNT; i++, nWalkingBit<<=1 ) 
	{
		if( nTrackerBits & nWalkingBit ) 
		{
			nTrackerType = (FWorldTrackerType_e)i;
			break;
		}
	}

	// Find another set bit...
	for( i++, nWalkingBit<<=1; i<FWORLD_TRACKERTYPE_COUNT; i++, nWalkingBit<<=1 ) 
	{
		if( nTrackerBits & nWalkingBit ) 
		{
			// Found a second set bit...
			nTrackerType = FWORLD_TRACKERTYPE_COUNT;
			break;
		}
	}

	return nTrackerType;
}


//
//
//
_STATICF BOOL _CollideWithWorldTrisCallback( FVisVolume_t *pVolume, BOOL bUsingVisData ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( pVolume );

	if ( !pVolume->pWorldGeo )
	{
		return TRUE;
	}
	
	// Collide the ray with the triangles in the mesh...
	if ( pVolume->pWorldGeo->CollideWithMeshTris( _pCollInfo ) ) 
	{
		// Found a collision...
		if ( (_pCollInfo->nStopOnFirstOfCollMask & FColl_nImpactCollMask) // Caller is only interested in if there is any impact of this type
			|| (_pCollInfo->bFindClosestImpactOnly && bUsingVisData) ) // Caller is interested in closest, only and we're walking visibility
		{
			// No need to continue searching nodes...
			return FALSE;
		}
	}

	return TRUE;
}


//
//
//
_STATICF int _SortTrackersCallback( const void *pElement1, const void *pElement2 ) 
{
	FASSERT( _bModuleInitialized );

	const _CollidedTracker_t *pCollInfo1, *pCollInfo2;

	pCollInfo1 = *(const _CollidedTracker_t **)pElement1;
	pCollInfo2 = *(const _CollidedTracker_t **)pElement2;

	if( pCollInfo1->fUnitDistToIntersection < pCollInfo2->fUnitDistToIntersection ) 
	{
		return -1;
	} 
	else if( pCollInfo1->fUnitDistToIntersection > pCollInfo2->fUnitDistToIntersection ) 
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
_STATICF BOOL _CollideRayWithTrackersCallback1( FVisVolume_t *pVolume, BOOL bUsingVisData ) 
{
	FASSERT( _bModuleInitialized );

	u32 i;

	// Called once for every node that intersects the ray...

	if ( !((CFTrackerCollideRayInfo *)_pTrackerCollideInfo)->bSort ) 
	{
		// Non-sort mode...
		if( _nCollideTrackerType != FWORLD_TRACKERTYPE_COUNT ) 
		{
			return _FindTrackersOfTypeInVolume( pVolume, _CollideRayWithTrackersCallback2, _nCollideTrackerType );
		} 
		else 
		{
			return _FindTrackersInVolume( pVolume, _CollideRayWithTrackersCallback2 );
		}
	} 
	else 
	{
		// Sort mode...

		_nCollidedTrackerCount = 0;

		// Build array of tracker collision data in _aCollidedTrackers[]...
		if( _nCollideTrackerType != FWORLD_TRACKERTYPE_COUNT ) 
		{
			_FindTrackersOfTypeInVolume( pVolume, _CollideRayWithTrackersCallback3, _nCollideTrackerType );
		} 
		else 
		{
			_FindTrackersInVolume( pVolume, _CollideRayWithTrackersCallback3 );
		}

		if( _nCollidedTrackerCount ) 
		{
			// Sort the list...
			for( i=0; i<_nCollidedTrackerCount; i++ ) 
			{
				_apCollidedTrackers[i] = &_aCollidedTrackers[i];
			}
			fclib_QSort( _apCollidedTrackers, _nCollidedTrackerCount, sizeof(_CollidedTracker_t *), _SortTrackersCallback );

			// Call the user's callback with the sorted list of trackers...
			for( i=0; i<_nCollidedTrackerCount; i++ ) 
			{
				if( !((CFTrackerCollideRayInfo *)_pTrackerCollideInfo)->pCallback(
							_apCollidedTrackers[i]->pTracker,
							pVolume,
							&_apCollidedTrackers[i]->IntersectionPoint_WS,
							_apCollidedTrackers[i]->fUnitDistToIntersection
						) ) 
				{
					// Callback wishes to terminate...
					return FALSE;
				}
			}
		}

		return TRUE;
	}
}


//
//
//
_STATICF BOOL _CollideRayWithTrackersCallback2( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) 
{
	FASSERT( _bModuleInitialized );

	// Called one or more times for each tracker that intersects the ray...

	if( pTracker->GetBoundingSphere().IsIntersecting( ((CFTrackerCollideRayInfo *)_pTrackerCollideInfo)->StartPoint_WS.v3, ((CFTrackerCollideRayInfo *)_pTrackerCollideInfo)->EndPoint_WS.v3, _pCollideRay_IntersectionPoint, _pfCollideRay_UnitDistToIntersection ) ) 
	{
		// Ray is intersecting this tracker...

		return ((CFTrackerCollideRayInfo *)_pTrackerCollideInfo)->pCallback( pTracker, pVolume, _pCollideRay_IntersectionPoint, *_pfCollideRay_UnitDistToIntersection );
	}

	// No intersection. Keep looking...

	return TRUE;
}


//
//
//
_STATICF BOOL _CollideRayWithTrackersCallback3( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) 
{
	FASSERT( _bModuleInitialized );

	// Called one or more times for each tracker that intersects the ray...

	if( _nCollidedTrackerCount >= _nCollidedTrackerMax ) 
	{
		// No more room in tracker collision buffer...
		DEVPRINTF( "FWorld::_CollideRayWithTrackersCallback3(): Out of room in tracker collision buffer.\n" );
		return FALSE;
	}

	if( pTracker->GetBoundingSphere().IsIntersecting(
					((CFTrackerCollideRayInfo *)_pTrackerCollideInfo)->StartPoint_WS.v3,
					((CFTrackerCollideRayInfo *)_pTrackerCollideInfo)->EndPoint_WS.v3,
					&_aCollidedTrackers[ _nCollidedTrackerCount ].IntersectionPoint_WS,
					&_aCollidedTrackers[ _nCollidedTrackerCount ].fUnitDistToIntersection
				) ) 
	{
		// Collision...
		_aCollidedTrackers[ _nCollidedTrackerCount ].pTracker = pTracker;
		_nCollidedTrackerCount++;
	}

	return TRUE;
}


//
//
//
_STATICF BOOL _CollideSphereWithTrackersCallback( FVisVolume_t *pVolume, BOOL bUsingVisData )//CFWorldTracker *pTracker, FVisVolume_t *pVolume ) 
{
	FASSERT( _bModuleInitialized );

	u32 nStart, nEnd, nTrackerType;
	BOOL bRetValue = TRUE;

	if ( _nCollideTrackerType == FWORLD_TRACKERTYPE_COUNT )
	{
		nStart = 0;
		nEnd = _nCollideTrackerType;
	}
	else
	{
		if( pVolume->anTotalTrackerCount[_nCollideTrackerType] == 0 ) 
		{
			// No trackers of the desired type in this world node...
			return TRUE;
		}

		nStart = _nCollideTrackerType;
		nEnd = _nCollideTrackerType + 1;
	}

	for ( nTrackerType = nStart; nTrackerType < nEnd; nTrackerType++ )
	{
		FLinkRoot_t *pRootIntersect = &pVolume->aTrackerIntersects[nTrackerType];

		u32 nVolTrackerKey = FVis_anVolumeTrackerKey[nTrackerType];

		CFWorldIntersect *pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
		{
			CFWorldTracker *pTracker = pIntersect->GetTracker();

			// Check to see if we've already visited this tracker...
			if ( pTracker->m_VisitedKey.HaveVisited( _nCollideRay_KeyIndex ) ) 
			{
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pTracker->m_VisitedKey.FlagAsVisited( _nCollideRay_KeyIndex );

			// Check CollInfo for quick out
			if ( _IsThereQuickOut( pTracker ) ) 
			{
				continue;
			}

/*
			if ( _pCollideSpheresInfo->nSphereListCount > 0 ) 
			{
				// Sphere list provided. Collide with spheres in list...

				CFSphere *pSphere = _pCollideSpheresInfo->pSphereList_WS;
				for( i = 0; i<_pCollideSpheresInfo->nSphereListCount; i++, pSphere++ ) 
				{
					if( pTracker->GetBoundingSphere().IsIntersecting( *pSphere ) ) 
					{
						if ( !_pCollideSpheresInfo->pCallback( pTracker, pVolume, i ) )
						{
							return FALSE;
						}
						else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[nTrackerType] )
						{
							// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
							FASSERT_NOW;
							return TRUE;
						}
					}
				}
			}
			else
*/
			{
				// No sphere list. Collide only with master sphere...
				if( pTracker->GetBoundingSphere().IsIntersecting( *_pCollideSpheresInfo->pMasterSphere_WS ) ) 
				{
					if ( !_pCollideSpheresInfo->pCallback( pTracker, pVolume, 0 ) )
					{
						return FALSE;
					}
					else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[nTrackerType] )
					{
						// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
						FASSERT_NOW;
						return TRUE;
					}
				}
			} 
		}
	}

	return bRetValue;
}


//
//
//
_STATICF BOOL _CollideProjSphereWithTrackersCallback( FVisVolume_t *pVolume, BOOL bUsingVisData )//CFWorldTracker *pTracker, FVisVolume_t *pVolume ) 
{
	FASSERT( _bModuleInitialized );

	u32 nTrackerType, nStart, nEnd;

	if ( _nCollideTrackerType == FWORLD_TRACKERTYPE_COUNT )
	{
		nStart = 0;
		nEnd = _nCollideTrackerType;
	}
	else
	{
		if ( pVolume->anTotalTrackerCount[_nCollideTrackerType] == 0 ) 
		{
			// No trackers of the desired type in this world node...
			return TRUE;
		}

		nStart = _nCollideTrackerType;
		nEnd = _nCollideTrackerType + 1;
	}

	CFCapsule TestCapsule;
	TestCapsule.m_vPoint1.Set( _pCollideProjSphereInfo->pProjSphere->m_vCenterStart_WS );
	TestCapsule.m_vPoint2.Set( _pCollideProjSphereInfo->pProjSphere->m_vCenterEnd_WS );
	TestCapsule.m_fRadius = _pCollideProjSphereInfo->pProjSphere->m_fRadius;

	for ( nTrackerType = nStart; nTrackerType < nEnd; nTrackerType++ )
	{
		FLinkRoot_t *pRootIntersect = &pVolume->aTrackerIntersects[nTrackerType];

		u32 nVolTrackerKey = FVis_anVolumeTrackerKey[nTrackerType];

		CFWorldIntersect *pIntersect = NULL;
		while ( (pIntersect = (CFWorldIntersect *)flinklist_GetNext( pRootIntersect, pIntersect )) ) 
		{
			CFWorldTracker *pTracker = pIntersect->GetTracker();

			// Check to see if we've already visited this tracker...
			if ( pTracker->m_VisitedKey.HaveVisited( _nCollideRay_KeyIndex ) ) 
			{
				// We've already visited the intersecting tracker, so let's skip it...
				continue;
			}

			// Flag as visited...
			pTracker->m_VisitedKey.FlagAsVisited( _nCollideRay_KeyIndex );

			// Check CollInfo for quick out
			if ( _IsThereQuickOut( pTracker ) ) 
			{
				continue;
			}

			// No sphere list. Collide only with master sphere...
			if( pTracker->GetBoundingSphere().IsIntersecting( &TestCapsule ) ) 
			{
				if ( !_pCollideProjSphereInfo->pCallback( pTracker, pVolume ) )
				{
					return FALSE;
				}
				else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[nTrackerType] )
				{
					// The callback changed the state of the trackers in the volume lists.  We must bail out for safety
					FASSERT_NOW;
					return TRUE;
				}
			}
		}
	}

	return TRUE;
}

//
//	fworld_GetCellContainingPoint()
//
//	Given a point, fills in pResult with the FVisCell_t that contains the point
//	and returns the distance from the point to the closest plane of the cell.
//	Fills it in with NULL and returns -FMATH_MAX_FLOAT if no point contains.
//
f32 fworld_GetCellContainingPoint( const CFVec3A &vPos, FVisCell_t **pResult )
{
	FASSERT( _bModuleInitialized );
	FASSERT_MSG( FVis_pCellTree, "fworld_GetCellContainingPoint() - ERROR - Checking world containment parameters prior to loading world." );

	u16 nCellIdx;
	f32 fPenetrationDepth = _IntersectNodeAndPoint( FVis_pCellTree->nFirstNodeIndex, vPos, &nCellIdx );
	
	if ( nCellIdx == FVIS_INVALID_CELL_ID )
	{
		*pResult = NULL;	
		return -FMATH_MAX_FLOAT;
	}
	
	FASSERT( nCellIdx < FVis_pVisData->nCellCount );
	
	*pResult = &FVis_pVisData->paCells[ nCellIdx ];
	return fPenetrationDepth;
}


//
//	fworld_GetVolumeContainingPoint()
//
//	Given a point, fills in the volume that contains the point in pResult and returns the
//	distance of the point to the closest cell plane (may be a plane internal to the volume)
//
//	If pHint is non-null, the function will test the FVisVolume_t that pHint resides in
//	before performing the longer test.
//
f32 fworld_GetVolumeContainingPoint( const CFVec3A *pPos, FVisVolume_t **pResult, const CFWorldTracker *pStartHint/*=NULL*/ )
{
	FASSERT( _bModuleInitialized );
	FASSERT_MSG( FVis_pCellTree, "fworld_GetVolumeContainingPoint() - ERROR - Checking world containment parameters prior to establishing visibility data." );

	f32 fPenetrationDepth = -FMATH_MAX_FLOAT;
	if ( pStartHint && pStartHint->GetFirstIntersect() )
	{
		FVisVolume_t *pVolHint = pStartHint->GetFirstIntersect()->GetVolume();
		if ( pVolHint->nVolumeID != FVIS_SLOP_BUCKET_ID )
		{
			fPenetrationDepth = pVolHint->IntersectPoint( pPos );
			if ( fPenetrationDepth != -FMATH_MAX_FLOAT )
			{
				*pResult = pVolHint;
				return fPenetrationDepth;
			}
		}
	}

	u16 nCellIdx;
	fPenetrationDepth = _IntersectNodeAndPoint( FVis_pCellTree->nFirstNodeIndex, *pPos, &nCellIdx );
	
	if ( nCellIdx == FVIS_INVALID_CELL_ID )
	{
		*pResult = NULL;	
		return -FMATH_MAX_FLOAT;
	}
	
	FASSERT( nCellIdx < FVis_pVisData->nCellCount );
	FASSERT( FVis_pVisData->paCells[ nCellIdx ].nParentVolumeIdx < FVis_pVisData->nVolumeCount );
	
	*pResult = &FVis_pVisData->paVolumes[FVis_pVisData->paCells[ nCellIdx ].nParentVolumeIdx];
	return fPenetrationDepth;
}

//
//	_IntersectNodeAndPoint()
//
//		Given an index into the portal data cell tree node array and
//		a 3D point, this function will recursively search the node
//		and all of its children and return the cell index for the 
//		cell that contains the point, or FVIS_INVALID_CELL_ID if no 
//		cell contains the point.
//
_STATICF f32 _IntersectNodeAndPoint( u16 nIndex, const CFVec3A &vPos, u16 *pnResult )
{
	FASSERT( _bModuleInitialized );
	FASSERT( FVis_pCellTree && nIndex >= 0 && nIndex < FVis_pCellTree->nNodeCount );
	
	FVisCellTreeNode_t *pNode = &FVis_pCellTree->paNodes[ nIndex ];
	
	// Test against AABB to determine if it is contained
	if (   vPos.x <= pNode->AABB.vMinXYZ.x || vPos.x >= pNode->AABB.vMaxXYZ.x
		|| vPos.y <= pNode->AABB.vMinXYZ.y || vPos.y >= pNode->AABB.vMaxXYZ.y
		|| vPos.z <= pNode->AABB.vMinXYZ.z || vPos.z >= pNode->AABB.vMaxXYZ.z )
	{
		// Point is outside the bounds, so we don't have containment
		*pnResult = FVIS_INVALID_CELL_ID;
		return -FMATH_MAX_FLOAT;
	}
	
	// Test children to determine if they contain the point
	if ( pNode->nChildNodeIdx1 != FVIS_INVALID_NODE_ID )
	{
		u16 nChild1Cell, nChild2Cell = FVIS_INVALID_CELL_ID;
		f32 fChild1Penetration, fChild2Penetration;
		fChild1Penetration = _IntersectNodeAndPoint( pNode->nChildNodeIdx1, vPos, &nChild1Cell );
		if ( pNode->nChildNodeIdx2 != FVIS_INVALID_NODE_ID )
		{
			fChild2Penetration = _IntersectNodeAndPoint( pNode->nChildNodeIdx2, vPos, &nChild2Cell );
		}
		
		// If one of the children had a cell containing the point, no need
		// to check the cells in this node; just return the child cell
		if ( nChild1Cell != FVIS_INVALID_CELL_ID && nChild2Cell != FVIS_INVALID_CELL_ID )
		{
			// Pick the cell with the smallest radius
			if ( fChild1Penetration < fChild2Penetration || (fChild1Penetration == fChild2Penetration &&
					FVis_pVisData->paCells[nChild1Cell].spBoundingWS.m_fRadius < FVis_pVisData->paCells[nChild2Cell].spBoundingWS.m_fRadius) )
			{
				(*pnResult) = nChild1Cell;
				return fChild1Penetration;
			}

			(*pnResult) = nChild2Cell;
			return fChild2Penetration;

		}
		else if ( nChild1Cell != FVIS_INVALID_CELL_ID )
		{
			(*pnResult) = nChild1Cell;
			return fChild1Penetration;
		}
		else if ( nChild2Cell != FVIS_INVALID_CELL_ID )
		{
			(*pnResult) = nChild2Cell;
			return fChild2Penetration;
		}
	}
	
	// The children didn't contain the point, so it's probably in this node's local cells
	
	if ( pNode->nContainedCellIdx == 0xffff )
	{
		// No cell resides in this node, so bail
		*pnResult = FVIS_INVALID_CELL_ID;
		return -FMATH_MAX_FLOAT;
	}
	
	// Test for containment
	// Verify that the cell index is within legal bounds
	FASSERT( pNode->nContainedCellIdx >= 0 && pNode->nContainedCellIdx < FVis_pVisData->nCellCount );
	FVisCell_t *pCell = &FVis_pVisData->paCells[pNode->nContainedCellIdx];
	f32 fDistanceToClosestCellWall = pCell->IntersectPoint( &vPos );
	if ( fDistanceToClosestCellWall < 0 )
	{
		// Point is outside the bounds, so we don't have containment
		*pnResult = FVIS_INVALID_CELL_ID;
		return -FMATH_MAX_FLOAT;
	}

	// We've got a new cell identified		
	(*pnResult) = pNode->nContainedCellIdx;
	
	return fDistanceToClosestCellWall;
}


enum
{
	_RAY_NOT_CONTAINED 			= 0,
	_RAY_PARTIALLY_CONTAINED 	= 0x0001,
	_RAY_ENTIRELY_CONTAINED		= 0x0002
};



#if FANG_DEBUG_BUILD
	BOOL FWorld_bShowPortalIntersections;
#endif

//
// Returns FALSE if the callback returned FALSE
//
BOOL fworld_GetVolumesIntersectingRay( const CFVec3A *pStart, 
									  const CFVec3A *pEnd, 
									  FWorldVolCallbackFcn_t *pCallbackFcn, 
									  const CFWorldTracker *pStartHint/*=NULL*/, 
									  s32 nWorldKey/*=-1*/,
									  u32 nUsePortalsFlag/*=FVIS_PORTAL_FLAG_VISIBILITY*/ )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pStart && pEnd && pCallbackFcn );

#if FANG_DEBUG_BUILD
	if ( FWorld_bShowPortalIntersections )
	{
		CFColorRGBA Color( 1.f, 1.f, 0.f, .5f );
		fdraw_DevLine( &pStart->v3, &pEnd->v3, &Color );
	}
#endif

	FVisVolume_t *pOriginVol = NULL;
	if ( nUsePortalsFlag != FVIS_PORTAL_FLAG_NONE )
	{
		if ( pStartHint && pStartHint->GetFirstIntersect() )
		{
			pOriginVol = pStartHint->GetFirstIntersect()->GetVolume();
		}
		
		CFVec3A vDiff;
		vDiff.Sub( *pEnd, *pStart );
		f32 fRayLength = vDiff.Mag();

		f32 fDepthInVol = -FMATH_MAX_FLOAT;

		// Do not use hint if it is in the slop bucket
		if ( pOriginVol && pOriginVol->nVolumeID != FVIS_SLOP_BUCKET_ID )
		{	
			fDepthInVol = pOriginVol->IntersectPoint( pStart );
		}
		if ( fDepthInVol == -FMATH_MAX_FLOAT )
		{
			fDepthInVol = fworld_GetVolumeContainingPoint( pStart, &pOriginVol );
		}

		if ( pOriginVol )
		{
			if ( pOriginVol->nVolumeID == FVIS_SLOP_BUCKET_ID )
			{
				// Let callback know about collision in the slop bucket
				if ( !pCallbackFcn( pOriginVol, TRUE ) )
				{
					return FALSE;
				}

				// Since the slop bucket can have no portals, we can bail now
				return TRUE;
			}
			
			// Test volumes for collision
			_RayWalker Walker;
			Walker.pStart = pStart;
			Walker.pEnd = pEnd;
			Walker.vUnitDir.Sub( *pEnd, *pStart );
			Walker.nPortalFlags = nUsePortalsFlag;
			f32 fLength = Walker.vUnitDir.MagSq();
			if ( fLength == 0.f )
			{
				return TRUE;
			}
			Walker.vUnitDir.Mul( fmath_InvSqrt( fLength ) );
			Walker.pCallbackFcn = pCallbackFcn;
			if ( nWorldKey == -1 )
			{
				Walker.nKeyIndex = CFWorldKey::OpenKey();
				if ( Walker.nKeyIndex == -1 )
				{
					return TRUE;
				}
			}
			else
			{
				Walker.nKeyIndex = nWorldKey;
			}
			BOOL bReturn = _WalkVis_Ray( &Walker, pOriginVol, 0xffff );
			CFWorldKey::CloseKey( Walker.nKeyIndex );
			return bReturn;
		}
	}
	
	// Were not using visibility
	if ( nWorldKey == -1 )
	{
		nWorldKey = CFWorldKey::OpenKey();
		if ( nWorldKey == -1 )
		{
			return TRUE;
		}
	}
	BOOL bEarlyOut = FALSE;
	u32 rVal = _IntersectNodeAndRay( pStart, pEnd, pCallbackFcn, FVis_pCellTree->nFirstNodeIndex, nWorldKey, bEarlyOut );
	CFWorldKey::CloseKey( nWorldKey );
	
	if (!pOriginVol && rVal == _RAY_NOT_CONTAINED)
	{
	   _bReachedEnd = TRUE;
	}

	return !bEarlyOut;
}


//
// Returns FALSE if the callback signalled to bail early
//
_STATICF BOOL _WalkVis_Ray( const _RayWalker *pWalker, FVisVolume_t *pVolume, u32 nEnteredFromPortal )
{
	FASSERT( _bModuleInitialized );

	u32 i;
	CFVec3 vDiff;

	if ( pVolume->Key.HaveVisited( pWalker->nKeyIndex ) )
	{
		return TRUE;
	}
	
	pVolume->Key.FlagAsVisited( pWalker->nKeyIndex );

	// Let callback know about collision
	if ( !pWalker->pCallbackFcn( pVolume, TRUE ) )
	{
		// Callback wants us to cease processing
		return FALSE;
	}

	// Check this volume's portals to determine intersections
	for ( i = 0; i < pVolume->nPortalCount; i++ )
	{
		// Skip the portal that we are looking into this room from
		if ( i == nEnteredFromPortal )
		{
			continue;
		}
		
		FVisPortal_t *pPortal = &FVis_pVisData->paPortals[pVolume->paPortalIndices[i]];
		
		// If this portal is not something the caller is interested in, we can skip it
		if ( !(pWalker->nPortalFlags & pPortal->nFlags) )
		{
			continue;
		}
		
		// Check facing of portal
		f32 fDot;
		if ( pPortal->anAdjacentVolume[0] == pVolume->nVolumeID )
		{
			fDot = -pPortal->vNormal.Dot( pWalker->vUnitDir.v3 );
		}
		else
		{
			fDot = pPortal->vNormal.Dot( pWalker->vUnitDir.v3 );
		}
		
		if ( fDot < 0.f )
		{
			// Ray is not facing the portal
			continue;
		}
		
		// Check for intersection with the portal
		if ( !pPortal->IntersectRay( &pWalker->pStart->v3, &pWalker->pEnd->v3, &pWalker->vUnitDir.v3 ) )
		{
			continue;
		}

#if FANG_DEBUG_BUILD
		if ( FWorld_bShowPortalIntersections )
		{
			CFColorRGBA Color( 0.f, 1.f, 0.f, .5f );
			fdraw_DevTriangle( &pPortal->avVertices[0], &pPortal->avVertices[1], &pPortal->avVertices[2], &Color );
			fdraw_DevTriangle( &pPortal->avVertices[0], &pPortal->avVertices[2], &pPortal->avVertices[3], &Color );
		}
#endif

		// Portal intersects, so determine the adjacent portal index
		FVisVolume_t *pAdjVolume;
		if ( pPortal->anAdjacentVolume[1] != pVolume->nVolumeID )
		{
			pAdjVolume = &FVis_pVisData->paVolumes[pPortal->anAdjacentVolume[1]];
			if ( !_WalkVis_Ray( pWalker, pAdjVolume, pPortal->anIdxInVolume[1] ) )
			{
				return FALSE;
			}
		}
		else
		{
			pAdjVolume = &FVis_pVisData->paVolumes[pPortal->anAdjacentVolume[0]];
			if ( !_WalkVis_Ray( pWalker, pAdjVolume, pPortal->anIdxInVolume[0] ) )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


//
//
//
u32 _IntersectNodeAndRay( const CFVec3A *pStart, const CFVec3A *pEnd, 
							FWorldVolCallbackFcn_t *pCallbackFcn, u32 nCellIdx, u32 nKeyIndex, BOOL &bEarlyOut )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pStart && pEnd && pCallbackFcn );
	FASSERT( FVis_pCellTree && nCellIdx >= 0 && nCellIdx < FVis_pCellTree->nNodeCount );

	FVisCellTreeNode_t *pNode = &FVis_pCellTree->paNodes[ nCellIdx ];

	f32 tMin = -FMATH_MAX_FLOAT;
	f32 tMax = FMATH_MAX_FLOAT;

	// Quickly determine if the ray intersects the AABB.  We use a "Slabs method"
	// approach with some optimizations based on assumptions we can make
	// for axis-aligned bounding boxes.
	u32 i;
	f32 fTest, fDiff;
	for ( i = 0; i < 3; i++ )
	{
		if ( pStart->a[i] < pEnd->a[i] )
		{
			if ( pStart->a[i] > pNode->AABB.vMaxXYZ.a[i] || pEnd->a[i] < pNode->AABB.vMinXYZ.a[i] )
			{
				return _RAY_NOT_CONTAINED;
			}

			fDiff = pEnd->a[i] - pStart->a[i];
			if ( fDiff < 0.0001f && fDiff > -0.0001f )
			{
				continue;
			}
			fDiff = fmath_Inv( fDiff );
			 
			if ( pStart->a[i] < pNode->AABB.vMinXYZ.a[i] )
			{
				fTest = (pNode->AABB.vMinXYZ.a[i] - pStart->a[i]) * fDiff;
				if ( fTest > tMin )
				{
					tMin = fTest;
				}
			}

			if ( pEnd->a[i] > pNode->AABB.vMaxXYZ.a[i] )
			{
				fTest = (pNode->AABB.vMaxXYZ.a[i] - pStart->a[i]) * fDiff;
				if ( fTest < tMax )
				{
					tMax = fTest;
				}
			}
		}
		else
		{
			if ( pStart->a[i] < pNode->AABB.vMinXYZ.a[i] || pEnd->a[i] > pNode->AABB.vMaxXYZ.a[i] )
			{
				return _RAY_NOT_CONTAINED;
			}
			
			fDiff = pStart->a[i] - pEnd->a[i];
			if ( fDiff < 0.0001f && fDiff > -0.0001f )
			{
				continue;
			}
			fDiff = fmath_Inv( fDiff );
			 
			if ( pEnd->a[i] < pNode->AABB.vMinXYZ.a[i] )
			{
				fTest = (pStart->a[i] - pNode->AABB.vMinXYZ.a[i]) * fDiff;
				if ( fTest < tMax )
				{
					tMax = fTest;
				}
			}

			if ( pStart->a[i] > pNode->AABB.vMaxXYZ.a[i] )
			{
				fTest = (pStart->a[i] - pNode->AABB.vMaxXYZ.a[i]) * fDiff;
				if ( fTest > tMin )
				{
					tMin = fTest;
				}
			}
		}
			
		if ( tMin > tMax )
		{
			return _RAY_NOT_CONTAINED;
		}
	}

	// The ray intersects the AABB for this node.  Now we need to see if 
	// any of the children or cells within this node have intersection:
	
	u32 nResults = _RAY_NOT_CONTAINED;
	
	// Test children to determine if they intersect the ray
	if ( pNode->nChildNodeIdx1 != FVIS_INVALID_NODE_ID )
	{
		nResults |= _IntersectNodeAndRay( pStart, pEnd, pCallbackFcn, pNode->nChildNodeIdx1, nKeyIndex, bEarlyOut );
		if ( pNode->nChildNodeIdx2 != FVIS_INVALID_NODE_ID )
		{
			nResults |= _IntersectNodeAndRay( pStart, pEnd, pCallbackFcn, pNode->nChildNodeIdx2, nKeyIndex, bEarlyOut );
		}
		
		// If one of the children wholly contained the ray, no need to check the cells in this node.
		if ( nResults & _RAY_ENTIRELY_CONTAINED || bEarlyOut )
		{
			return nResults;
		}
	}
	
	// The children didn't wholly contain the ray, so it could hit this node's local cell
	
	if ( pNode->nContainedCellIdx == 0xffff )
	{
		// No cell resides in this node, so bail
		return nResults;
	}
	
	FASSERT( pNode->nContainedCellIdx < FVis_pVisData->nCellCount );
	FVisCell_t *pCell = &FVis_pVisData->paCells[pNode->nContainedCellIdx];
	
	FASSERT( pCell->nParentVolumeIdx < FVis_pVisData->nVolumeCount );
	FVisVolume_t *pVolume = &FVis_pVisData->paVolumes[pCell->nParentVolumeIdx];

	if ( pVolume->Key.HaveVisited( nKeyIndex ) )
	{
		// This volume has already been reported as colliding, so bail
		return nResults;
	}
		
	f32 fDistanceToClosestCellWall = pCell->IntersectRay( pStart, pEnd, FALSE );
	
	if ( fDistanceToClosestCellWall == -FMATH_MAX_FLOAT )
	{
		return nResults;
	}
	
	if ( fDistanceToClosestCellWall == -1.f )
	{
		nResults |= _RAY_ENTIRELY_CONTAINED;
	}
	else
	{
		nResults |= _RAY_PARTIALLY_CONTAINED;
	}
	
	// Flag the volume and report it to the callback
	pVolume->Key.FlagAsVisited( nKeyIndex );
	if ( !pCallbackFcn( pVolume, FALSE ) )
	{
		bEarlyOut = TRUE;
	}
	
	return nResults;
}


//
//
//
BOOL fworld_GetVolumesIntersectingProjectedSphere( const CFSphere *pStartSphere, 
												  const CFVec3A *pMovement, 
												  f32 fMoveMag, 
												  FWorldVolCallbackFcn_t *pCallbackFcn, 
												  const CFWorldTracker *pStartHint/*=NULL*/, 
												  s32 nWorldKey/*=-1*/,
												  u32 nUsePortalsFlag/*=FVIS_PORTAL_FLAG_VISIBILITY*/ )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pStartSphere && pMovement && pCallbackFcn );

	CFSphere spTestSphere;
	spTestSphere.m_Pos = pStartSphere->m_Pos + (pMovement->v3 * 0.5f);
	spTestSphere.m_fRadius = (fMoveMag * 0.5f) + pStartSphere->m_fRadius;

	if ( nUsePortalsFlag != FVIS_PORTAL_FLAG_NONE )
	{
		FVisVolume_t *pOriginVol = NULL;
		if ( pStartHint && pStartHint->GetFirstIntersect() )
		{
			pOriginVol = pStartHint->GetFirstIntersect()->GetVolume();
		}
		
		CFVec3A vTemp;
		vTemp.Set( pStartSphere->m_Pos );

		f32 fDepthInVol = -FMATH_MAX_FLOAT;

		// Do not use hint if it is in the slop bucket
		if ( pOriginVol && pOriginVol->nVolumeID != FVIS_SLOP_BUCKET_ID )
		{
			// If we have a starting place, then we can use it as a hint of where
			// the sphere might be.
			fDepthInVol = pOriginVol->IntersectPoint( &vTemp );
		}

		if ( fDepthInVol == -FMATH_MAX_FLOAT )
		{
			fDepthInVol = fworld_GetVolumeContainingPoint( &vTemp, &pOriginVol );
		}

		if ( pOriginVol )
		{
			if ( pOriginVol->nVolumeID == FVIS_SLOP_BUCKET_ID )
			{
				// Let callback know about collision in the slop bucket
				pCallbackFcn( pOriginVol, TRUE );

				// Since the slop bucket can have no portals, we can bail now
				return TRUE;
			}
			
			_SphereWalker Walker;
			Walker.pSphere = &spTestSphere;
			Walker.nPortalFlags = nUsePortalsFlag;
			Walker.fRadiusSq = spTestSphere.m_fRadius * spTestSphere.m_fRadius;
			Walker.pCallbackFcn = pCallbackFcn;
			if ( nWorldKey == -1 )
			{
				Walker.nKeyIndex = CFWorldKey::OpenKey();
				if ( Walker.nKeyIndex == -1 )
				{
					return TRUE;
				}
			}
			else
			{
				Walker.nKeyIndex = nWorldKey;
			}
			Walker.nPortalFlags = nUsePortalsFlag;
			if ( pStartHint && pStartHint->IsTraverseClosedPortalsSet() )
			{
				Walker.nPortalFlags |= FVIS_PORTAL_FLAG_CLOSED;
			}
			BOOL bReturn = _WalkVis_Sphere( &Walker, pOriginVol, 0xffff );
			CFWorldKey::CloseKey( Walker.nKeyIndex );

			return bReturn;
		}
	}

	// Caller doesn't want to use vis data, or we couldn't find a start place
	if ( nWorldKey == -1 )
	{
		nWorldKey = CFWorldKey::OpenKey();
		if ( nWorldKey == -1 )
		{
			return TRUE;
		}
	}
	BOOL bEarlyOut = FALSE;
	_IntersectNodeAndSphere( &spTestSphere, pCallbackFcn, FVis_pCellTree->nFirstNodeIndex, nWorldKey, bEarlyOut );
	CFWorldKey::CloseKey( nWorldKey );
	
	return !bEarlyOut;
}


//
// Returns FALSE if callback signalled an early bail out
//
BOOL fworld_GetVolumesIntersectingSphere( const CFSphere *pSphere, 
										 FWorldVolCallbackFcn_t *pCallbackFcn, 
										 const CFWorldTracker *pStartHint/*=NULL*/, 
										 s32 nWorldKey/*=-1*/,
										 u32 nUsePortalsFlag/*=FVIS_PORTAL_FLAG_VISIBILITY*/ )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pSphere && pCallbackFcn );
	
	if ( nUsePortalsFlag != FVIS_PORTAL_FLAG_NONE )
	{
		FVisVolume_t *pOriginVol = NULL;
		if ( pStartHint && pStartHint->GetFirstIntersect() )
		{
			pOriginVol = pStartHint->GetFirstIntersect()->GetVolume();
		}
		
		CFVec3A vTemp;
		vTemp.Set( pSphere->m_Pos );

		f32 fDepthInVol = -FMATH_MAX_FLOAT;

		// Do not use hint if it is in the slop bucket
		if ( pOriginVol && pOriginVol->nVolumeID != FVIS_SLOP_BUCKET_ID )
		{
			// If we have a starting place, then we can use it as a hint of where
			// the sphere might be.
			fDepthInVol = pOriginVol->IntersectPoint( &vTemp );
		}

		if ( fDepthInVol == -FMATH_MAX_FLOAT )
		{
			fDepthInVol = fworld_GetVolumeContainingPoint( &vTemp, &pOriginVol );
		}

		if ( pOriginVol )
		{
			if ( pOriginVol->nVolumeID == FVIS_SLOP_BUCKET_ID )
			{
				// Let callback know about collision in the slop bucket
				if ( !pCallbackFcn( pOriginVol, TRUE ) )
				{
					return FALSE;
				}

				// Since the slop bucket can have no portals, we can bail now
				return TRUE;
			}
			
			_SphereWalker Walker;
			Walker.pSphere = pSphere;
			Walker.nPortalFlags = nUsePortalsFlag;
			Walker.fRadiusSq = pSphere->m_fRadius * pSphere->m_fRadius;
			Walker.pCallbackFcn = pCallbackFcn;
			if ( nWorldKey == -1 )
			{
                Walker.nKeyIndex = CFWorldKey::OpenKey();
				if ( Walker.nKeyIndex == -1 )
				{
					return TRUE;
				}
			}
			else
			{
                Walker.nKeyIndex = nWorldKey;
			}
			Walker.nPortalFlags = nUsePortalsFlag;
			if ( pStartHint && pStartHint->IsTraverseClosedPortalsSet() )
			{
				Walker.nPortalFlags |= FVIS_PORTAL_FLAG_CLOSED;
			}
			BOOL bReturn = _WalkVis_Sphere( &Walker, pOriginVol, 0xffff );
			CFWorldKey::CloseKey( Walker.nKeyIndex );
			return bReturn;
		}
	}

	// Caller doesn't want to use vis data, or we couldn't find a start place
	if ( nWorldKey == -1 )
	{
        nWorldKey = CFWorldKey::OpenKey();
		if ( nWorldKey == -1 )
		{
			return TRUE;
		}
	}
	BOOL bEarlyOut = FALSE;
	_IntersectNodeAndSphere( pSphere, pCallbackFcn, FVis_pCellTree->nFirstNodeIndex, nWorldKey, bEarlyOut );
	CFWorldKey::CloseKey( nWorldKey );
	
	return !bEarlyOut;
}


//
// Returns FALSE if the callback signalled to bail early
//
_STATICF BOOL _WalkVis_Sphere( const _SphereWalker *pWalker, FVisVolume_t *pVolume, u32 nEnteredFromPortal )
{
	FASSERT( _bModuleInitialized );

	u32 i;
	CFVec3 vDiff;
	
	// Make sure we don't visit a vol twice
	if ( pVolume->Key.HaveVisited( pWalker->nKeyIndex ) )
	{
		return TRUE;
	}
	
	pVolume->Key.FlagAsVisited( pWalker->nKeyIndex );

	// Let the callback function know about this collision
	if ( !pWalker->pCallbackFcn( pVolume, TRUE ) )
	{
		return FALSE;
	}

	// Check this volume's portals to determine intersections
	for ( i = 0; i < pVolume->nPortalCount; i++ )
	{
		// Skip the portal that we are looking into this room from
		if ( i == nEnteredFromPortal )
		{
			continue;
		}
		
		FVisPortal_t *pPortal = &FVis_pVisData->paPortals[pVolume->paPortalIndices[i]];
		
		// If this portal is not something the caller is interested in, we can skip it
		if ( !(pWalker->nPortalFlags & pPortal->nFlags) )
		{
			continue;
		}
		
		// Check for portal intersection with the sphere (this is a close approximation)
		
		// First is it close enough to the plane that contains the portal?
		vDiff = pWalker->pSphere->m_Pos - pPortal->spBoundingWS.m_Pos;
		f32 fDist = vDiff.Dot( pPortal->vNormal );
		if ( fDist > pWalker->pSphere->m_fRadius || fDist < -pWalker->pSphere->m_fRadius )
		{
			continue;
		}
		
		// Next is it close enough to the portal bounding sphere
		f32 fTotal = pWalker->pSphere->m_fRadius + pPortal->spBoundingWS.m_fRadius;
		if ( vDiff.Mag2() > (fTotal * fTotal) )
		{
			continue;
		}

		// Portal intersects, so determine the adjacent portal index
		FVisVolume_t *pAdjVolume;
		if ( pPortal->anAdjacentVolume[1] != pVolume->nVolumeID )
		{
			pAdjVolume = &FVis_pVisData->paVolumes[pPortal->anAdjacentVolume[1]];
			if ( !_WalkVis_Sphere( pWalker, pAdjVolume, pPortal->anIdxInVolume[1] ) )
			{
				return FALSE;
			}
		}
		else
		{
			pAdjVolume = &FVis_pVisData->paVolumes[pPortal->anAdjacentVolume[0]];
			if ( !_WalkVis_Sphere( pWalker, pAdjVolume, pPortal->anIdxInVolume[0] ) )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


//
//
//
u32 _IntersectNodeAndSphere( const CFSphere *pSphere, FWorldVolCallbackFcn_t *pCallbackFcn, u32 nCellIdx, u32 nKeyIndex, BOOL &bEarlyOut )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pSphere && pCallbackFcn );
	FASSERT( FVis_pCellTree && nCellIdx >= 0 && nCellIdx < FVis_pCellTree->nNodeCount );

	FVisCellTreeNode_t *pNode = &FVis_pCellTree->paNodes[ nCellIdx ];

	f32 tMin = -FMATH_MAX_FLOAT;
	f32 tMax = FMATH_MAX_FLOAT;

	// Quickly determine if the sphere intersects the AABB.
	if (   pSphere->m_Pos.x + pSphere->m_fRadius < pNode->AABB.vMinXYZ.x 
		|| pSphere->m_Pos.x - pSphere->m_fRadius > pNode->AABB.vMaxXYZ.x 
		|| pSphere->m_Pos.y + pSphere->m_fRadius < pNode->AABB.vMinXYZ.y
		|| pSphere->m_Pos.y - pSphere->m_fRadius > pNode->AABB.vMaxXYZ.y
		|| pSphere->m_Pos.z + pSphere->m_fRadius < pNode->AABB.vMinXYZ.z
		|| pSphere->m_Pos.z - pSphere->m_fRadius > pNode->AABB.vMaxXYZ.z )
	{
		return _RAY_NOT_CONTAINED;
	}

	// The sphere intersects the AABB for this node.  Now we need to see if 
	// any of the children or cells within this node have intersection:

	u32 nResults = _RAY_NOT_CONTAINED;

	// Test children to determine if they intersect the sphere
	if ( pNode->nChildNodeIdx1 != FVIS_INVALID_NODE_ID )
	{
		nResults |= _IntersectNodeAndSphere( pSphere, pCallbackFcn, pNode->nChildNodeIdx1, nKeyIndex, bEarlyOut );
		if ( pNode->nChildNodeIdx2 != FVIS_INVALID_NODE_ID )
		{
			nResults |= _IntersectNodeAndSphere( pSphere, pCallbackFcn, pNode->nChildNodeIdx2, nKeyIndex, bEarlyOut );
		}

		// If one of the children wholly contained the sphere, no need to check the cells in this node.
		if ( (nResults & _RAY_ENTIRELY_CONTAINED) || bEarlyOut )
		{
			return nResults;
		}
	}

	// The children didn't wholly contain the ray, so it could hit this node's local cell

	if ( pNode->nContainedCellIdx == 0xffff )
	{
		// No cell resides in this node, so bail
		return nResults;
	}
	
	FASSERT( pNode->nContainedCellIdx < FVis_pVisData->nCellCount );
	FVisCell_t *pCell = &FVis_pVisData->paCells[pNode->nContainedCellIdx];
	
	FASSERT( pCell->nParentVolumeIdx < FVis_pVisData->nVolumeCount );
	FVisVolume_t *pVolume = &FVis_pVisData->paVolumes[pCell->nParentVolumeIdx];

	if ( pVolume->Key.HaveVisited( nKeyIndex ) )
	{
		// This volume has already been reported as colliding, so bail
		return nResults;
	}
		
	CFVec3A vCenter;
	vCenter.Set( pSphere->m_Pos );
	f32 fDistanceToClosestCellWall = pCell->IntersectSphere( &vCenter, pSphere->m_fRadius );
	if ( fDistanceToClosestCellWall == -FMATH_MAX_FLOAT )
	{
		// No collision so let's bail
		return nResults;
	}
	
	if ( fDistanceToClosestCellWall >= 0.f )
	{
		nResults |= _RAY_ENTIRELY_CONTAINED;
	}
	else
	{
		nResults |= _RAY_PARTIALLY_CONTAINED;
	}
	
	// Flag the volume and report it to the callback
	pVolume->Key.FlagAsVisited( nKeyIndex );
	if ( !pCallbackFcn( pVolume, FALSE ) )
	{
		bEarlyOut = TRUE;
	}
	
	return nResults;
}


//
// fworld_IsLineOfSightObstructed()
//
// Tests the ray for collision against the terrain and CFWorldMesh objects that have their collision flag enabled and their line-of-site flag set.
//
// Returns TRUE if the ray is obstructed.
// Returns FALSE if the ray is not obstructed.
//
// ppTrackerSkipList and nTrackerSkipCount describe a buffer of CFWorldTracker's that will
// not be collided with.
//
// The collision impact buffer is cleared during this call.
//
BOOL fworld_IsLineOfSightObstructed( const CFVec3A *pRayStart_WS, 
									 const CFVec3A *pRayEnd_WS, 
									 u32 nTrackerSkipCount/*=0*/, 
									 const CFWorldTracker * const * ppTrackerSkipList/*=NULL*/, 
									 const CFWorldTracker *pStartHint/*=NULL*/,
									 u64 nUserTypeBitMask/*=0xffffffffffffffff*/,
									 u16 nCollisionMask/*=FCOLL_MASK_CHECK_ALL*/,
									 u8 nCollisionLOD/*=FCOLL_LOD_HIGHEST*/,
									 u16 *pAccumTriMasks/*=NULL*/ )
{
	FASSERT( _bModuleInitialized );

	u32 i, ii;

	// Clear the global vars
	_nVolumeIntersectCount = 0;
	_bReachedEnd = FALSE;
	_pEndVol = NULL;

	fworld_GetVolumeContainingPoint( pRayEnd_WS, &_pEndVol, pStartHint );

	// First gather up a list of the volumes intersecting the ray
	fworld_GetVolumesIntersectingRay( pRayStart_WS, pRayEnd_WS, _AccumulateVolumesUsingVisCallback, pStartHint );

	// If we didn't reach the volume containing the end point, then we can assume that
	// the visibility system determined that there is no clear line of sight to the volume
	if ( !_bReachedEnd )
	{
		return TRUE;
	}

	// Setup the CollInfo for testing against the meshes
	if ( pAccumTriMasks )
	{
		_LOSCollInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
	}
	else
	{
		_LOSCollInfo.nCollMask = FCOLL_MASK_OBSTRUCT_LINE_OF_SIGHT;
	}

	_LOSCollInfo.nCollMask = nCollisionMask;
	_LOSCollInfo.nResultsLOD = nCollisionLOD;
	_LOSCollInfo.Ray.Init( pRayStart_WS, pRayEnd_WS );
	_LOSCollInfo.pTag = NULL;

	// Walk through the collected volumes determining collision
	fcoll_Clear();
	FVisVolume_t *pVol;
	for ( i = 0; i < _nVolumeIntersectCount; i++ )
	{
		pVol = _papVolumeIntersectBuffer[i];

		// Make sure we have some geo to test against
		if ( !pVol->pWorldGeo )
		{
			continue;
		}

		if ( pVol->pWorldGeo->CollideWithMeshTris( &_LOSCollInfo ) && (FColl_nImpactCollMask & FCOLL_MASK_OBSTRUCT_LINE_OF_SIGHT) )
		{
			// We found a collision with this volume, so there obviously isn't a clean LOS
			return TRUE;
		}
	}

	u32 nVolTrackerKey = FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH];

	if ( nUserTypeBitMask )
	{
		fcoll_Clear();

		u32 nKeyIndex = CFWorldKey::OpenKey();

		// Walk through the trackers determining collision
		CFWorldTracker *pTracker;
		CFWorldIntersect *pIntersect;
		for ( i = 0; i < _nVolumeIntersectCount; i++ )
		{
			pVol = _papVolumeIntersectBuffer[i];

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

				if ( !pTracker->IsCollisionFlagSet() ) 
				{
					// Tracker isn't collideable...
					continue;
				}

				if ( !pTracker->IsLineOfSightFlagSet() ) 
				{
					continue;
				}

				// Check against user type bits
				if ( !(pTracker->GetUserTypeBits() & nUserTypeBitMask) )
				{
					continue;
				}

				// Make sure the tracker is not in the skip list
				for ( ii = 0; ii < nTrackerSkipCount; ii++ )
				{
					if ( pTracker == ppTrackerSkipList[ii] )
					{
						// Tracker is in the skip list, so go on to the next one
						break;
					}
				}

				// If we didn't reach the end of the skip list, that means that we've found the tracker in the skip list
				if ( ii != nTrackerSkipCount )
				{
					continue;
				}

				// We've found a potentially-intersecting mesh so check collision

				if ( !pTracker->GetBoundingSphere().IsIntersecting( pRayStart_WS->v3, pRayEnd_WS->v3 ) ) 
				{
					// Ray is not intersecting this tracker...
					continue;
				}

				CFWorldMesh *pMesh = (CFWorldMesh *)pTracker;

				if ( pMesh->CollideWithMeshTris( &_LOSCollInfo ) && (FColl_nImpactCollMask & FCOLL_MASK_OBSTRUCT_LINE_OF_SIGHT) )
				{
					// We found a collision with this mesh, so there obviously isn't a clean LOS
					CFWorldKey::CloseKey( nKeyIndex );
					return TRUE;
				}
				else if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
				{
					// The CollideWithMeshTris() call changed the state of the trackers in the volume lists.  We must bail out for safety
					FASSERT_NOW;
					CFWorldKey::CloseKey( nKeyIndex );
					return FALSE;
				}
			}
		}

		CFWorldKey::CloseKey( nKeyIndex );
	}

	if ( pAccumTriMasks )
	{
		(*pAccumTriMasks) = FColl_nImpactCollMask;
	}

	// We didn't find any collision so there is a clear line of sight
	return FALSE;
}


//
// fworld_FindClosestImpactPointToRayStart()
//
// Tests the ray for collision against the terrain and CFWorldMesh objects that have 
// their collision flag enabled.  Finds the impact point closest to the start of the ray.
//
// Returns TRUE if an impact was found
// Returns FALSE if an impact was not found
//
// If an impact was found, fills *pClosestImpact with the impact that is closest to the ray start.
//
// For terrain, pClosestImpact->pTag is set to NULL.
// For objects, pClosestImpact->pTag is set to the CFWorldTracker of the object.
//
// ppTrackerSkipList and nTrackerSkipCount describe a buffer of CFWorldTracker's that 
// will not be collided with.
//
// The collision impact buffer is cleared during this call.
//
// If bIgnoreWires is set, then trackers with an m_nUser equal to FWORLD_USERTYPE_WIRE will be skipped.
//
BOOL fworld_FindClosestImpactPointToRayStart( FCollImpact_t *pClosestImpact, 
											  const CFVec3A *pRayStart_WS, 
											  const CFVec3A *pRayEnd_WS, 
											  u32 nTrackerSkipCount/*=0*/, 
											  const CFWorldTracker * const *ppTrackerSkipList/*=NULL*/, 
											  BOOL bCullBackSides/*=TRUE*/, 
											  const CFWorldTracker *pStartHint/*=NULL*/,
											  u64 nUserTypeBitMask/*=0xffffffffffffffff*/,
											  u16 nCollisionMask/*=FCOLL_MASK_CHECK_ALL*/,
											  u8 nCollisionLOD/*=FCOLL_LOD_HIGHEST*/,
											  BOOL bIgnoreWires/*=TRUE*/ )
{
	FASSERT( _bModuleInitialized );

	u32 i, ii;

	// Clear the global vars
	_nVolumeIntersectCount = 0;
	_pEndVol = NULL;

	fworld_GetVolumeContainingPoint( pRayEnd_WS, &_pEndVol, pStartHint );

	// First gather up a list of the volumes intersecting the ray
	fworld_GetVolumesIntersectingRay( pRayStart_WS, pRayEnd_WS, _AccumulateVolumesCallback, pStartHint );

	fcoll_Clear();

	_CIRSCollInfo.Ray.Init( pRayStart_WS, pRayEnd_WS );
	if ( _CIRSCollInfo.Ray.fLength < 0.001f )
	{
		return FALSE;
	}

	// Setup the CollInfo for testing against the meshes
	_CIRSCollInfo.nCollMask = nCollisionMask;
	_CIRSCollInfo.nResultsLOD = nCollisionLOD;
	_CIRSCollInfo.bCullBacksideCollisions = bCullBackSides;
	_CIRSCollInfo.pTag = NULL;

	f32 fOriginalDistance = _CIRSCollInfo.Ray.fLength;
	f32 fCurrentDistance = fOriginalDistance;

	// Walk through the collected volumes determining collision
	u32 nFarthestVolume = 0;
	FVisVolume_t *pVol;
	for ( nFarthestVolume = 0; nFarthestVolume < _nVolumeIntersectCount; nFarthestVolume++ )
	{
		pVol = _papVolumeIntersectBuffer[nFarthestVolume];

		// Make sure we have some geo to test against
		if ( !pVol->pWorldGeo )
		{
			continue;
		}

		if ( pVol->pWorldGeo->CollideWithMeshTris( &_CIRSCollInfo ) )
		{
			// Cut the ray to the impact point since we're only interested in the closest collision
			_CIRSCollInfo.Ray.vEnd_WS.Set( FColl_aImpactBuf[FColl_nImpactCount-1].ImpactPoint );
			_CIRSCollInfo.Ray.fLength = FColl_aImpactBuf[FColl_nImpactCount-1].fImpactDistInfo * fCurrentDistance;
			fCurrentDistance = _CIRSCollInfo.Ray.fLength;

			// Since our volumes are ordered from closest to farthest, then
			// we know we can bail out if we already have collision
			nFarthestVolume++;
			break;
		}
	}

	if ( nUserTypeBitMask )
	{
		BOOL bHitObject = FALSE;
		u32 nKeyIndex = CFWorldKey::OpenKey();

		u32 nVolTrackerKey = FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH];

		// Walk through the trackers determining collision
		CFWorldTracker *pTracker;
		CFWorldIntersect *pIntersect;
		for ( i = 0; i < nFarthestVolume; i++ )
		{
			pVol = _papVolumeIntersectBuffer[i];

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

				if ( !pTracker->IsCollisionFlagSet() ) 
				{
					// Tracker isn't collideable...
					continue;
				}

				// Check against user type bits
				if ( !(pTracker->GetUserTypeBits() & nUserTypeBitMask) )
				{
					continue;
				}

				// Skip wires
				if ( bIgnoreWires && (pTracker->m_nUser == FWORLD_USERTYPE_WIRE) )
				{
					continue;
				}

				// Make sure the tracker is not in the skip list
				for ( ii = 0; ii < nTrackerSkipCount; ii++ )
				{
					if ( pTracker == ppTrackerSkipList[ii] )
					{
						// Tracker is in the skip list, so go on to the next one
						break;
					}
				}

				// If we didn't reach the end of the skip list, that means that we've found the tracker in the skip list
				if ( ii != nTrackerSkipCount )
				{
					continue;
				}

				// We've found a potentially-intersecting mesh so check collision

				if ( !pTracker->GetBoundingSphere().IsIntersecting( _CIRSCollInfo.Ray.vStart_WS.v3, _CIRSCollInfo.Ray.vEnd_WS.v3 ) ) 
				{
					// Ray is not intersecting this tracker...
					continue;
				}

				// Check collision
				_CIRSCollInfo.pTag = pTracker;
				if ( ((CFWorldMesh *)pTracker)->CollideWithMeshTris( &_CIRSCollInfo ) )
				{
					// Cut the ray to the impact point since we're only interested in the closest collision
					_CIRSCollInfo.Ray.vEnd_WS.Set( FColl_aImpactBuf[FColl_nImpactCount-1].ImpactPoint );
					_CIRSCollInfo.Ray.fLength = FColl_aImpactBuf[FColl_nImpactCount-1].fImpactDistInfo * fCurrentDistance;
					bHitObject = TRUE;
					if ( _CIRSCollInfo.Ray.fLength < 0.0001f )
					{
						// We have no more length left to our ray, so we break out of the object tests, which will
						// also result in a break out of the volume tests since bHitObject is TRUE
						break;
					}

					_CIRSCollInfo.Ray.fLengthSq = _CIRSCollInfo.Ray.fLength * _CIRSCollInfo.Ray.fLength;
					fCurrentDistance = _CIRSCollInfo.Ray.fLength;
				}

				if ( nVolTrackerKey != FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_MESH] )
				{
					// The CollideWithMeshTris() call changed the state of the trackers in the volume lists.  We must bail out for safety
					FASSERT_NOW;
					CFWorldKey::CloseKey( nKeyIndex );
					return FALSE;
				}
			}

			if ( bHitObject )
			{
				// If we hit an object in the current volume, then there is no reason to
				// traverse to additional volumes, since the volumes are organized by distance
				break;
			}
		}

		CFWorldKey::CloseKey( nKeyIndex );
	}

	if ( FColl_nImpactCount == 0 ) 
	{
		// No impacts...
		return FALSE;
	}

	// Copy the closest collision (it will always be the last)...
	fang_MemCopy( pClosestImpact, &FColl_aImpactBuf[FColl_nImpactCount-1], sizeof( FCollImpact_t ) );

#if !FANG_PRODUCTION_BUILD
//	fmesh_Coll_AddCollDrawImpact( &pClosestImpact->ImpactPoint );
#endif

	// Adjust the ImpactDistInfo since it is relative to a scaled ray
	pClosestImpact->fImpactDistInfo = fmath_Div( fCurrentDistance, fOriginalDistance );
	FMATH_CLAMP( pClosestImpact->fImpactDistInfo, 0.f, 1.f );

	return TRUE;
}


//
//
//
_STATICF BOOL _AccumulateVolumesCallback( FVisVolume_t *pVolume, BOOL bUseVisData )
{
	FASSERT( _bModuleInitialized );

	if ( _nVolumeIntersectCount > _nMaxVolumeIntersects )
	{
		return FALSE;
	}

	_papVolumeIntersectBuffer[_nVolumeIntersectCount++] = pVolume;

	if ( pVolume == _pEndVol )
	{
		_bReachedEnd = TRUE;
	}

	return TRUE;
}


//
//
//
_STATICF BOOL _AccumulateVolumesUsingVisCallback( FVisVolume_t *pVolume, BOOL bUseVisData )
{
	FASSERT( _bModuleInitialized );

	if ( _nVolumeIntersectCount > _nMaxVolumeIntersects || !bUseVisData )
	{
		return FALSE;
	}

	_papVolumeIntersectBuffer[_nVolumeIntersectCount++] = pVolume;

	if ( pVolume == _pEndVol )
	{
		_bReachedEnd = TRUE;
	}

	return TRUE;
}


