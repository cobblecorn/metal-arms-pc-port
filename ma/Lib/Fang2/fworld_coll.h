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

#ifndef _FWORLD_COLL_H_
#define _FWORLD_COLL_H_ 1

#include "fang.h"
#include "fworld.h"
#include "fvis.h"
#include "fcoll.h"

// Forward declarations
class CFVec3A;
class CFSphere;
class CFVec3;
class CFCollInfo;
struct FVisVolume_t;
struct FVisCell_t;
struct FCollImpact_t;


enum
{
	FWORLD_COLL_TRACKER_TYPE_RAY = 0,
	FWORLD_COLL_TRACKER_TYPE_SPHERES,
	FWORLD_COLL_TRACKER_TYPE_PROJSPHERE,
};


//
//
//
FCLASS_ALIGN_PREFIX class CFTrackerCollideInfo
{
	protected:
		u32  m_nCollideType;
		
	public:		
		u32  nTrackerTypeBits;			// One or more FWORLD_TRACKERTYPEBIT_* flags indicating which tracker types to collide with
		u64  nTrackerUserTypeBitsMask;	// A bit mask which is set in the tracker by the app
		BOOL bIgnoreCollisionFlag;		// TRUE=ignore the tracker collision flag and assume all trackers are collidable (helpful for development)

		u32 nTrackerSkipCount;								// Number of trackers in the skip list
		const CFWorldTracker * const *ppTrackerSkipList;	// pointer to an array of tracker pointers to skip
		
		u32 GetType( void ) const { return m_nCollideType; };
	
} FCLASS_ALIGN_SUFFIX;


//
//
//
FCLASS_ALIGN_PREFIX class CFTrackerCollideRayInfo : public CFTrackerCollideInfo
{
	public:
		CFTrackerCollideRayInfo( void )
		{
			m_nCollideType = FWORLD_COLL_TRACKER_TYPE_RAY;
		}
		
		CFVec3A StartPoint_WS;			// Start point of the ray in world space
		CFVec3A EndPoint_WS;				// End point of the ray in world space

		BOOL bComputeIntersection;		// TRUE=pass intersection info to callback function (FALSE is faster as long as bSort is also FALSE)
		BOOL bSort;						// TRUE=sort objects from nearest-to-startpoint to farthest-from-startpoint (if TRUE, forces bComputeIntersection to TRUE)

		FWorldCollideRayWithTrackersCallback_t *pCallback;	// Pointer to callback function that's called once per tracker that intersects the ray
	
	FCLASS_STACKMEM_ALIGN( CFTrackerCollideRayInfo );
} FCLASS_ALIGN_SUFFIX;


//
//
//
FCLASS_ALIGN_PREFIX class CFTrackerCollideSpheresInfo : public CFTrackerCollideInfo
{
	public:
		CFTrackerCollideSpheresInfo( void )
		{
			m_nCollideType = FWORLD_COLL_TRACKER_TYPE_SPHERES;
		}
		
		CFSphere *pMasterSphere_WS;		// Pointer to master sphere. If colliding with only one sphere, point to it here
//		CFSphere *pSphereList_WS;		// Pointer to sphere list. All spheres in this list must be bounded by *pMasterSphere_WS
//		u32 nSphereListCount;			// Number of spheres in pSphereList_WS. If 0, only collision with master sphere is performed

		FWorldCollideSpheresWithTrackersCallback_t *pCallback;// Pointer to callback function that's called once per tracker that intersects the sphere list
	
	FCLASS_STACKMEM_ALIGN( CFTrackerCollideSpheresInfo );
} FCLASS_ALIGN_SUFFIX;


//
//
//
FCLASS_ALIGN_PREFIX class CFTrackerCollideProjSphereInfo : public CFTrackerCollideInfo
{
	public:
		CFTrackerCollideProjSphereInfo( void )
		{
			m_nCollideType = FWORLD_COLL_TRACKER_TYPE_PROJSPHERE;
		}
		
		CFProjSphere *pProjSphere;

		FWorldIntersectingTrackerCallback_t *pCallback;// Pointer to callback function that's called once per tracker that intersects
	
	FCLASS_STACKMEM_ALIGN( CFTrackerCollideProjSphereInfo );
} FCLASS_ALIGN_SUFFIX;


//
// Collision Interface:
//
// These are the work-horse functions for the collision system.  They will be the
// functions most accessed at the app layer and return TRUE is collision was detected.
//
extern BOOL fworld_IsLineOfSightObstructed(	const CFVec3A *pRayStart_WS, 
											const CFVec3A *pRayEnd_WS, 
											u32 nTrackerSkipCount = 0, 
											const CFWorldTracker * const * ppTrackerSkipList = NULL, 
											const CFWorldTracker *pStartHint = NULL,
											u64 nUserTypeBitMask = 0xffffffffffffffff,
											u16 nCollisionMask = FCOLL_MASK_CHECK_ALL,
											u8 nCollisionLOD = FCOLL_LOD_HIGHEST,
											u16 *pAccumTriMasks = NULL );

extern BOOL fworld_FindClosestImpactPointToRayStart( FCollImpact_t *pClosestImpact, 
													 const CFVec3A *pRayStart_WS, 
													 const CFVec3A *pRayEnd_WS, 
													 u32 nTrackerSkipCount = 0, 
													 const CFWorldTracker * const *ppTrackerSkipList = NULL, 
													 BOOL bCullBackSides = TRUE, 
													 const CFWorldTracker *pStartHint = NULL,
													 u64 nUserTypeBitMask = 0xffffffffffffffff,
													 u16 nCollisionMask = FCOLL_MASK_CHECK_ALL,
													 u8 nCollisionLOD = FCOLL_LOD_HIGHEST,
													 BOOL bIgnoreWires=TRUE );


extern BOOL fworld_CollideWithWorldTris( CFCollInfo *pCollInfo, const CFWorldTracker *pStartHint = NULL );
extern BOOL fworld_CollideWithTrackers( const CFTrackerCollideInfo *pCollideRayInfo, const CFWorldTracker *pStartHint = NULL );

//
// Volume Interface:
//
extern f32  fworld_GetVolumeContainingPoint( const CFVec3A *pPos, FVisVolume_t **pResult, const CFWorldTracker *pStartHint = NULL );

extern BOOL fworld_GetVolumesIntersectingSphere( const CFSphere *pSphere, 
												 FWorldVolCallbackFcn_t *pCallback, 
												 const CFWorldTracker *pStartHint = NULL, 
												 s32 nKey = -1,
												 u32 nUsePortalsFlag = FVIS_PORTAL_FLAG_VISIBILITY );

extern BOOL fworld_GetVolumesIntersectingProjectedSphere( const CFSphere *pStartSphere, 
												  const CFVec3A *pMovement, 
												  f32 fMoveMag, 
												  FWorldVolCallbackFcn_t *pCallbackFcn, 
												  const CFWorldTracker *pStartHint = NULL, 
												  s32 nKey = -1,
												  u32 nUsePortalsFlag = FVIS_PORTAL_FLAG_VISIBILITY );
												 
extern BOOL fworld_GetVolumesIntersectingRay( const CFVec3A *pStart, 
											  const CFVec3A *pEnd, 
											  FWorldVolCallbackFcn_t *pCallbackFcn, 
											  const CFWorldTracker *pStartHint = NULL, 
											  s32 nKey = -1,
											  u32 nUsePortalsFlag = FVIS_PORTAL_FLAG_VISIBILITY );

//
// Tracker Interface
//
extern BOOL fworld_FindTrackersIntersectingVolume( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback, FWorldTrackerType_e nTypeToFind );
extern BOOL fworld_FindTrackersIntersectingVolume( FVisVolume_t *pVolume, FWorldIntersectingTrackerCallback_t *pCallback );
extern BOOL fworld_FindTrackersIntersectingSphere(  const CFSphere *pSphere_WS, 
													FWorldTrackerType_e nTypeToFind, 
													FWorldIntersectingTrackerCallback_t *pTrackerCallback, 
													u32 nTrackerSkipCount = 0, 
													const CFWorldTracker * const *ppTrackerSkipList = NULL, 
													const CFWorldTracker *pStartHint = NULL, 
													u32 nUserType = -1,
													u64 nUserTypeBitMask = 0xffffffffffffffff, 
													u32 nUsePortalsFlag = FVIS_PORTAL_FLAG_VISIBILITY );

//
// Cell Interface:
//
extern f32  fworld_GetCellContainingPoint( const CFVec3A &vPos, FVisCell_t **pResult );

//
// Initialization:
//
extern BOOL fworld_coll_Startup( void );
extern BOOL fworld_coll_Shutdown( void );


#endif