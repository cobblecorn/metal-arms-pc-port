//////////////////////////////////////////////////////////////////////////////////////
// fvis.h - Fang visibility determination system.
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
// 04/23/02 Lafleur       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FVIS_H_
#define _FVIS_H_ 1

#include "fang.h"
#include "fworld.h"


#define FVIS_SLOP_BUCKET_ID		0x0000
#define FVIS_INVALID_VOLUME_ID	0xffff
#define FVIS_INVALID_CELL_ID	0xffff
#define FVIS_INVALID_NODE_ID	0xffff
#define FVIS_INVALID_GEO_ID		(CFMeshInst *)0xffffffff

#define FVIS_MAX_PORTAL_VERTS					4
#define FVIS_MAX_VOLUME_COUNT					1024
#define FVIS_MAX_PORTAL_COUNT					4096
#define FVIS_MAX_LIGHTS							256

#define FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS	4
#define FVIS_MAX_VISIBLE_LIGHTS_LIST_COUNT		128
#define FVIS_MAX_VISIBLE_MESH_LIST_COUNT		128

#define FVIS_MAX_SCREEN_MESH_RESULTS			48


struct FVisPortal_t;
struct FVisCell_t;
struct FVisVolume_t;

//
//	This header appears in the world file
//
typedef struct
{
	u32 	nMeshCount;
	FMesh_t **papMeshArray;

} FVisGeoHeader_t;

//
//
//
typedef struct
{
	CFVec3 vMinXYZ;		// minimum x, y and z of the node
	CFVec3 vMaxXYZ;		// maximum x, y and z of the node
	
	void ChangeEndian( void )
	{
		vMinXYZ.ChangeEndian();
		vMaxXYZ.ChangeEndian();
	}

} FVisAABB_t;


//-------------------------------------------------------------------------------------------------------------------
// FVisCellTree Related Definitions:
//
//		These structures define the Visibility Cell Tree that is used for
//		point or shape-in-cell determination tests.  This tree is basically
//		a collection of AABB's (an R-tree).  There is a parent node that defines 
//		the	entire world space.  That node has two children.  The dividing plane
//		is the plane that will cause the cells within the parent to be divided
//		as close to evenly as possible.  If there is a cell within the parent
//		that exceeds some threshhold value (like 75% of the area of the parent),
//		then that cell will be placed in the parent's list of indices into the
//		world cell array.
//-------------------------------------------------------------------------------------------------------------------

//
//
//
struct FVisCellTreeNode_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FVisCellTreeNode_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	FVisAABB_t AABB;			// Axis aligned bounding box that describes this leaf
	u16 nParentNodeIdx;			// index of the parent of this node
	u16 nChildNodeIdx1;			// index of the first child or 0xffff if there is no child
	u16 nChildNodeIdx2;			// index of the second child or 0xffff if there is no child
	u16 nContainedCellIdx;		// Index of cell contained within this node, or 0xffff
	
	void ChangeEndian( void )
	{
		AABB.ChangeEndian();
		nParentNodeIdx = fang_ConvertEndian( nParentNodeIdx );
		nChildNodeIdx1 = fang_ConvertEndian( nChildNodeIdx1 );
		nChildNodeIdx2 = fang_ConvertEndian( nChildNodeIdx2 );
		nContainedCellIdx = fang_ConvertEndian( nContainedCellIdx );
	}
	
};


//
//
//
struct FVisCellTree_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FVisCellTree_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u16 nFirstNodeIndex;			// index of the first node
	u16 nNodeCount;					// number of nodes in the tree
	FVisCellTreeNode_t *paNodes;	// pointer to the array of nodes
	
	void ChangeEndian( void )
	{
		nFirstNodeIndex = fang_ConvertEndian( nFirstNodeIndex );
		nNodeCount = fang_ConvertEndian( nNodeCount );
		paNodes = (FVisCellTreeNode_t *)fang_ConvertEndian( paNodes );
	}
	
};



//-------------------------------------------------------------------------------------------------------------------
// Portal System Data Container:
//-------------------------------------------------------------------------------------------------------------------

//
//
//
struct FVisData_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FVisData_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u16				nPortalCount;				// Number of portals in paPortals, below
	u16				nCellCount;					// Number of cells in paCells, below
	u16				nVolumeCount;				// Number of volumes in paVolumes, below
	u16				nLightCount;				// Number of world non-ambient lights in paLights, below
	
	FVisCellTree_t 	CellTree;
	
	CFColorRGB		AmbientLightColor;			// World global ambient light RGB
	f32				fAmbientLightIntensity;		// World global ambient light intensity

	f32 			fFogStartZ;					// Fog start Z (if fFogStartZ and fFogEndZ are both 0.0f, fog is disabled)
	f32 			fFogEndZ;					// Fog end Z
	CFColorMotif 	FogMotif;					// Fog motif

	FVisPortal_t	*paPortals;					// Pointer to an array of all portals in the world
	FVisVolume_t	*paVolumes;					// Pointer to an array of all volumes in the world
	FVisCell_t 		*paCells;					// Pointer to an array of all cells in the world
	FLightInit_t	*paLights;					// Pointer to an array of all non-ambient lights in the world
	
	void ChangeEndian( void )
	{
		CellTree.ChangeEndian();
		
		nPortalCount = fang_ConvertEndian( nPortalCount );
		nVolumeCount = fang_ConvertEndian( nVolumeCount );
		nCellCount = fang_ConvertEndian( nCellCount );
		nLightCount = fang_ConvertEndian( nLightCount );
		
		fAmbientLightIntensity = fang_ConvertEndian( fAmbientLightIntensity );
		AmbientLightColor.ChangeEndian();
		
		fFogStartZ = fang_ConvertEndian( fFogStartZ );
		fFogEndZ = fang_ConvertEndian( fFogEndZ );
		FogMotif.ChangeEndian();
		
		paPortals = (FVisPortal_t *)fang_ConvertEndian( paPortals );
		paVolumes = (FVisVolume_t *)fang_ConvertEndian( paVolumes );
		paCells = (FVisCell_t *)fang_ConvertEndian( paCells );
		paLights = (FLightInit_t *)fang_ConvertEndian( paLights );
	}

};


//-------------------------------------------------------------------------------------------------------------------
// FVisPortal Related Definitions:
//
//-------------------------------------------------------------------------------------------------------------------

//
//	FVisPortal flags
enum 
{
	FVIS_PORTAL_FLAG_NONE				= 0x0000,
	
	FVIS_PORTAL_FLAG_CLOSED				= 0x0001,	// There is no visibility through the portal (this state may change)
	FVIS_PORTAL_FLAG_MIRROR				= 0x0002,	// This portal functions as a mirror (reflects in the direction of the normal)
	FVIS_PORTAL_FLAG_ALWAYS_VISIBLE		= 0x0004,	// The adjacent volume is always visible (ignore frustum tests)
	FVIS_PORTAL_FLAG_ONE_WAY			= 0x0008,	// The portal only allows visibility from the cell the normal is pointed into
	FVIS_PORTAL_FLAG_SOUND				= 0x0010,	// This portal can transmit sound
	FVIS_PORTAL_FLAG_ANTIPORTAL			= 0x0020,	// This portal causes the viewport to be blocked
	FVIS_PORTAL_FLAG_VISIBILITY			= 0x0040,	// This portal allows visibility through it

	// Used for collision (DO NOT CHANGE THESE FLAGS!!!)
	FVIS_PORTAL_FLAG_NORM_X_LARGEST		= 0x1000,
	FVIS_PORTAL_FLAG_NORM_Y_LARGEST		= 0x2000,
	FVIS_PORTAL_FLAG_NORM_Z_LARGEST		= 0x3000,
	
	FVIS_PORTAL_FLAG_AUTOPORTAL			= 0x8000,	// This portal was created by PASM
};


//
// FVisPortal - Describes connectivity of volumes based on cell containment.  A portal 
//		must be co-planar with at least one cell's walls.  If it is not coplanar with 
//		the walls of TWO different
//		cells, then the portal connects the coplanar cell and the cell that the portal
//		is located inside.  If the portal is coplanar with walls of two different cells,
//		then it connects the two cells.
//
struct FVisPortal_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FVisPortal_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u16 nFlags; 				// See FVisPortal flags, above
	u16 nPortalID;				// Index of this portal in the portal data portal list (used at the app level to identify portals)

	// Connectivity description
	u16 anAdjacentVolume[2];	// The indices of the two adjacent volumes into the global volume array
	u8  anIdxInVolume[2];		// The portal's portal index in the portal arrays of the adjacent volumes
	u16 nVertCount;
	
	CFVec3 avVertices[FVIS_MAX_PORTAL_VERTS];	// The coplanar verts that form the portal (MUST WIND CLOCKWISE WHEN NORMAL FACING VIEWPOINT)
	CFVec3 vNormal;				// The world space normal of the portal pointing into volume index 0 (negate for the normal into volume 1) (Could be index into normal sphere for GC)
	CFSphere spBoundingWS;		// A world space bounding sphere
	f32 fBoundingRadiusSq;		// Square of the bounding sphere radius
	
	// Sets the portal as open or not.  Returns the prior state.  If
	// a portal is not opened, the visibility system will not traverse
	// to the adjacent volumes.
	BOOL SetOpenState( BOOL bOpen )
	{
		BOOL bPriorState = !(nFlags & FVIS_PORTAL_FLAG_CLOSED);
		if ( bOpen )
		{
			nFlags &= ~FVIS_PORTAL_FLAG_CLOSED;
		}
		else
		{
			nFlags |= FVIS_PORTAL_FLAG_CLOSED;
		}
		
		return bPriorState;
	}
	
	BOOL IntersectRay( const CFVec3 *pStart, const CFVec3 *pEnd, const CFVec3 *pUnitDir );
	
	void ChangeEndian( void )
	{
		nFlags = fang_ConvertEndian( nFlags );
		nPortalID = fang_ConvertEndian( nPortalID );
		avVertices[0].ChangeEndian();
		avVertices[1].ChangeEndian();
		avVertices[2].ChangeEndian();
		avVertices[3].ChangeEndian();
		anAdjacentVolume[0] = fang_ConvertEndian( anAdjacentVolume[0] );
		anAdjacentVolume[1] = fang_ConvertEndian( anAdjacentVolume[1] );
		nVertCount = fang_ConvertEndian( nVertCount );
		vNormal.ChangeEndian();
		spBoundingWS.ChangeEndian();
	}

};


//-------------------------------------------------------------------------------------------------------------------
// FVisCell Related Definitions:
//
//-------------------------------------------------------------------------------------------------------------------

//
//
//
FCLASS_ALIGN_PREFIX struct FVisPlane_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FVisPlane_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	CFVec3A vNormal;	// Normal of the plane in world space (Could be index into normal sphere for GC)
	CFVec3A vPoint;		// The centroid in world space of the cell surface that established this plane
	
	void ChangeEndian( void )
	{
		vNormal.ChangeEndian();
		vPoint.ChangeEndian();
	}
	
} FCLASS_ALIGN_SUFFIX;


//
//
//
struct FVisCell_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FVisCell_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	CFSphere spBoundingWS;			// World space sphere that bounds the cell geometry

	u16 nParentVolumeIdx;			// Index into the Volume array that indicates the volume that contains this cell
	u8 nPlaneCount;					// Number of planes that describe this cell
	u8 __nPad;
	
	FVisPlane_t *paBoundingPlanes;	// Pointer to the array of bounding planes for this cell
	
	void ChangeEndian( void )
	{
		spBoundingWS.ChangeEndian();
		nParentVolumeIdx = fang_ConvertEndian( nParentVolumeIdx );
		nPlaneCount = fang_ConvertEndian( nPlaneCount );
		paBoundingPlanes = (FVisPlane_t *)fang_ConvertEndian( paBoundingPlanes );
	}
	
	//
	//	Determines if point is within cell boundaries.  If not
	//	it returns -FMATH_MAX_FLOAT.  If it is, it returns the
	//	distance from the point to the closest plane
	//
	f32 IntersectPoint( const CFVec3A *pTestPoint )
	{
		FASSERT( pTestPoint );
		u32 i;
		CFVec3A vDiff;
		f32 fTest, fMin = FMATH_MAX_FLOAT;

		vDiff.v3 = pTestPoint->v3 - spBoundingWS.m_Pos;
		if ( vDiff.MagSq() > (spBoundingWS.m_fRadius * spBoundingWS.m_fRadius) )
		{
			return -FMATH_MAX_FLOAT;
		}		
		
		FVisPlane_t *pPlane = paBoundingPlanes;
		for ( i = 0; i < nPlaneCount; i++, pPlane++ )
		{
			vDiff.Sub( *pTestPoint, pPlane->vPoint );
			
			fTest = vDiff.Dot( pPlane->vNormal );
			
			// If the point is on the other side of the plane, we know it is not contained
			if ( fTest < 0 )
				return -FMATH_MAX_FLOAT;
			else if ( fTest < fMin )
				fMin = fTest;			
		}
		
//		FASSERT( fMin != FMATH_MAX_FLOAT );
		
		return fMin;
	}

	//
	//	Determines if ray intersects the cell.
	//		RETURN VALUES:
	//			0.f = Start Point in volume
	//			-1.f = Ray entirely in volume
	//			-FMATH_MAX_FLOAT = ray outside volume
	//			FMATH_MAX_FLOAT = end point in volume and distance not requested
	//			0.f < return < FMATH_MAX_FLOAT = distance along ray where collision occurs
	//
	f32 IntersectRay( const CFVec3A *pStart, const CFVec3A *pEnd, BOOL bGetImpact )
	{
		FASSERT( pStart && pEnd );
		
		// Test the start and end point to determine if they are in the cell
		f32 fStartDist = IntersectPoint( pStart );
		f32 fEndDist = IntersectPoint( pEnd );
		
		// If both the start and the endpoint are in, then we have an easy out
		if ( fStartDist >= 0.f && fEndDist >= 0.f )
		{
			return -1.f;
		}
		
		// If start is in, then we have an easy out
		if ( fStartDist >= 0.f )
		{
			return 0.f;
		}
		
		// If the impact was not requested and the end of the
		// ray was in the cell, then we have an easy out.
		if ( !bGetImpact && fEndDist >= 0.f )
		{
			return FMATH_MAX_FLOAT;
		}

		// Unfortunately, neither the start nor the end of the ray were in the
		// cell.  But there still could be collision, so we need to do the hard
		// check to see if the ray passes through the convex hull:
		
		// This code is a modified "Slabs Method" which is basically the same as 3D clipping code:
		u32 i;
		static CFVec3A vTemp;
		f32 fTest, fMin = 0.f, fMax = 1.f;

		FVisPlane_t *pPlane = paBoundingPlanes;
		for ( i = 0; i < nPlaneCount; i++, pPlane++ )
		{
			fStartDist = vTemp.Sub( *pStart, pPlane->vPoint ).Dot( pPlane->vNormal );
			fEndDist = vTemp.Sub( *pEnd, pPlane->vPoint ).Dot( pPlane->vNormal );
			
			// If both points are outside the plane, then there
			// can be no collision with this cell
			if ( fEndDist < 0.f && fStartDist < 0.f )
			{
				return -FMATH_MAX_FLOAT;
			}

			// If both points are inside the plane, then we cannot generate the
			// impact point using this plane, so go to the next.
			if ( fEndDist > 0.f && fStartDist > 0.f )
			{
				continue;
			}
			
			// Check to see if the ray is parallel			
			if ( fEndDist == fStartDist )
			{
				continue;
			}
			
			// Determine the ray min and max inside the cell
			fTest = fmath_Div( fStartDist, fStartDist - fEndDist );
			if ( fStartDist < 0 && fTest < fMax )
			{
				fMax = fTest;
			}
			else if ( fTest > fMin )
			{
				fMin = fTest;
			}

			// If the min is ever greater than the max, then the ray is outside the cell			
			if ( fMin > fMax )
			{
				return -FMATH_MAX_FLOAT;
			}
		}
		
//		FASSERT( fMin != 0.f || fMax != 1.f );
		
		if ( fMin > 0.f )
			return fMin;
			
		return fMax;
	}

	//
	//	Determines if sphere intersects cell boundaries.  If not
	//	it returns -FMATH_MAX_FLOAT.  If it is, it returns the
	//	distance to the closest plane (note that it will be negative
	//	if the plane intersects the sphere!)
	//
	f32 IntersectSphere( const CFVec3A *pCenter, f32 fRadius )
	{
		FASSERT( pCenter );
		u32 i;
		CFVec3A vDiff;
		f32 fTest, fMin = FMATH_MAX_FLOAT;
		
		FVisPlane_t *pPlane = paBoundingPlanes;
		for ( i = 0; i < nPlaneCount; i++, pPlane++ )
		{
			vDiff.Sub( *pCenter, pPlane->vPoint );
			
			fTest = vDiff.Dot( pPlane->vNormal );
			
			// If the point plus radius is on the other side of the plane, we know it is not contained
			if ( fTest + fRadius < 0 )
				return -FMATH_MAX_FLOAT;
			else if ( fTest - fRadius < fMin )
				fMin = fTest - fRadius;			
		}
		
//		FASSERT( fMin != FMATH_MAX_FLOAT );
		
		return fMin;
	}

};


//-------------------------------------------------------------------------------------------------------------------
// FVisVolume Related Definitions:
//
//-------------------------------------------------------------------------------------------------------------------

// Array to keep track of changes in the volume tracker lists
extern u32 FVis_anVolumeTrackerKey[FWORLD_TRACKERTYPE_COUNT];

//
//
//
struct FVisVolume_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FVisVolume_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u16 nVolumeID;					// Unique ID for this volume.  Should equal the index of this volume in the portal data volume array
	u8 nFlags;		
	CFWorldKey Key;					// Key to help with list processing
	CFSphere spBoundingWS;			// Bounding sphere for the volume in world space (used for frustum culling - eventually we should use a better shape)
	
	u8 nPortalCount;				// Number of portals in this volume
	u8 nCellCount;					// Number of cells that make up this volume
	u16 nCellFirstIdx;				// First Cell index in the array of cells
	
	s8 nCrossesPlanesMask;			// Used to indicate which frustum planes this volume crosses (must be set to 0 by tools)
	u8 nActiveAdjacentSteps;		// Number of steps from this volume, if drawn, that volumes will be added to the active list
	s8 nActiveStepDecrement;		// Decrement to active list stepping when adding this volume
	u8 __nPad;
	
	u16 anTotalTrackerCount[FWORLD_TRACKERTYPE_COUNT];	// Total number of trackers intersecting this volume (must be set to 0 by tools)
	FLinkRoot_t aTrackerIntersects[FWORLD_TRACKERTYPE_COUNT];	// World Intersects that indicate Trackers that currently intersect with this volume
	
	u16 *paPortalIndices;			// Pointer to the array of indices for portals that are visible in this volume

	CFMeshInst *pWorldGeo;			// World (static) geometry associated with this volume (The tool should store an index into the geo array which
									// will be resolved to a pointer to the actual geometry.  If there is no geo, this MUST be set to FVIS_INVALID_GEO_ID
									// by the tool.  The engine will then set it to NULL at load time.)
	
	void *pUserData;				// Pointer that the app can use to store information specific to this portal (ex. AI path nodes, etc)

	//
	//	Determines if point is within a cell of the volume.  If not
	//	it returns -FMATH_MAX_FLOAT.  If it is, it returns the
	//	distance from the point to the closest plane
	//
	f32 IntersectPoint( const CFVec3A *pTestPoint )
	{
		f32 fReturn = FMATH_MAX_FLOAT;
		FVisCell_t *pCell = &FWorld_pWorld->paCells[nCellFirstIdx];
		u32 i;
		for ( i = 0; i < nCellCount; i++, pCell++ )
		{
			f32 fDistInCell = pCell->IntersectPoint( pTestPoint );
			if ( fDistInCell != -FMATH_MAX_FLOAT && fDistInCell < fReturn )
			{
				fReturn = fDistInCell;
			}
		}

		if ( fReturn == FMATH_MAX_FLOAT )
		{
			return -FMATH_MAX_FLOAT;
		}
		return fReturn;
	}

//	void AddTrackerDeltaToCount( FWorldTrackerType_e nTrackerType, s32 nDeltaCount )
//	{
//		FASSERT( nTrackerType >= 0 && nTrackerType < FWORLD_TRACKERTYPE_COUNT );
//		FASSERT( nDeltaCount>=-65535 && nDeltaCount<=65535 );
//		FASSERT( (s32)anTotalTrackerCount[nTrackerType] + nDeltaCount >= 0 && (s32)anTotalTrackerCount[nTrackerType] + nDeltaCount <= 0xffff );
//		anTotalTrackerCount[nTrackerType] = (u16)(anTotalTrackerCount[nTrackerType] + nDeltaCount);
//	}

	void AddIntersect( CFWorldIntersect *pIntersect, FWorldTrackerType_e nTrackerType )
	{
		FASSERT( nTrackerType >= 0 && nTrackerType < FWORLD_TRACKERTYPE_COUNT );
		FASSERT( pIntersect );
		flinklist_AddTail( &aTrackerIntersects[nTrackerType], pIntersect );

		FASSERT( anTotalTrackerCount[nTrackerType] < 65535 );
		anTotalTrackerCount[nTrackerType]++;
	}

	void RemoveIntersect( CFWorldIntersect *pIntersect, FWorldTrackerType_e nTrackerType )
	{
		FASSERT( nTrackerType >= 0 && nTrackerType < FWORLD_TRACKERTYPE_COUNT );
		FASSERT( pIntersect );
		flinklist_Remove( &aTrackerIntersects[nTrackerType], pIntersect );

		FASSERT( anTotalTrackerCount[nTrackerType] > 0 );
		anTotalTrackerCount[nTrackerType]--;
		FVis_anVolumeTrackerKey[nTrackerType]++;
	}

	void ChangeEndian( void )
	{
		u32 i;
		nVolumeID = fang_ConvertEndian( nVolumeID );
		nFlags = fang_ConvertEndian( nFlags );
		// Key does not need to be endian converted - it is only altered and accessed during simulation
		spBoundingWS.ChangeEndian();

		nCrossesPlanesMask = fang_ConvertEndian( nCrossesPlanesMask );
		nPortalCount = fang_ConvertEndian( nPortalCount );
		
		nActiveAdjacentSteps = fang_ConvertEndian( nActiveAdjacentSteps );
		nActiveStepDecrement = fang_ConvertEndian( nActiveStepDecrement );

		nCellCount = fang_ConvertEndian( nCellCount );
		nCellFirstIdx = fang_ConvertEndian( nCellFirstIdx );
		
		paPortalIndices = (u16 *)fang_ConvertEndian( paPortalIndices );

		pWorldGeo = (CFMeshInst *)fang_ConvertEndian( pWorldGeo );
		
		for ( i = 0; i < FWORLD_TRACKERTYPE_COUNT; i++ )
		{
			anTotalTrackerCount[i] = fang_ConvertEndian( anTotalTrackerCount[i] );
		}
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FVis Draw structures used in draw arrays
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
struct FVisVisibleLight_t
{
	CFWorldLight	*pLight;		
	f32 			fDistFromCameraSq;	// Distance of the light emitter from the camera
	BOOL			bOriginVisible;		// True if the origin of the light is visible to the camera
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Basic Visibility System functions and data:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern BOOL fvis_ModuleStartup( void );
extern void fvis_ModuleShutdown( void );

extern void fvis_FrameBegin( void );  // Should be called once per frame, prior to FVis_Draw() call(s).
extern void fvis_Draw( const CFMtx43A *pCamera, BOOL bDrawEntireWorld, u32 nFVisDataBufferSlot = 0, BOOL bDrawDevShapes = TRUE ); // Could be called multiple times per frame for each viewport render of the world
extern void fvis_FrameEnd( void );  // Should be called once per frame, after all FVis_Draw() call(s).
extern void fvis_DrawLiquidVolumes( void );
//setup shadows for this frame.
extern void fvis_SetupShadows( void );

// Calling this function will set a flag that will cause the render pipeline to stall 
// after display list generation until all fDataStreaming data is in the cache.  This
// is a good call to make after suddenly moving the render camera (restoring checkpoints,
// game start, etc.).  This flag will automatically reset after fvis performs one render.
extern void fvis_BlockOnStreamingData( void );

// This function will return the closest portal to pvPos_WS.  If a pointer to
// a float is provided in pDistance, it will be filled in with the distance to the portal.
extern FVisPortal_t* fvis_GetNearestPortal( const CFVec3A *pvPos_WS, f32 *pDistance = NULL );

// Portal visibility data
extern FVisData_t *FVis_pVisData;
extern FVisCellTree_t *FVis_pCellTree;

// Data used for the current viewport render (Persists only while FVis is in the DL creation phase - during a call to FVis_Draw() )
extern CFMtx43A FVis_mtxRenderCamera;
extern FViewport_t *FVis_pCurrentViewport;

// Minimum distance an object has to be away from the camera to be rendered, used for reflections.
extern f32 FVis_fMinRenderDist;

// TRUE if liquid rendering is allowed while rendering to RenderTargets else FALSE.
extern BOOL8 FVis_bAllowLiquid_RenderTarget;

// World directional light accessors:
extern f32 FVis_fCurrDirLightWeight;
extern CFLight *fvis_GetCurrentDirectional( void );
FINLINE f32 fvis_GetCurrentDirectionalWeight( void )
{
	return FVis_fCurrDirLightWeight;
}

extern BOOL8 FVis_bInRenderTarget;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Active/Visible list flags
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	FVIS_VOLUME_NOT_IN_LIST			= 0x00,
	FVIS_VOLUME_IN_ACTIVE_LIST		= 0x01,
	FVIS_VOLUME_IN_VISIBLE_LIST		= 0x02,
};

// Volume render flags
extern u8 FVis_anVolumeFlags[FVIS_MAX_VOLUME_COUNT];
extern u8 FVis_anPreviousVolumeFlags[FVIS_MAX_VOLUME_COUNT];


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Determining visible trackers on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// For this structure, unless otherwise indicated, the measurements are screen space, with 0,0 at the
// center of the screen, -1.f a the left and 1.f at the right.  The extents along y are designed to
// be proportional to x based on the viewport's aspect ratio (for standard 4:3, -0.75 and 0.75).
//
FCLASS_ALIGN_PREFIX struct FVis_ScreenMesh_t
{
	CFWorldMesh *pWorldMesh;			// The mesh that falls within the client defined screen region
	f32			fDistanceFromRegion;	// The screen distance from the edge of the tracker's bounding sphere to the center of the supplied region
	CFSphere	spScreen;				// z indicates the distance (IN FEET) from the provided camera (distance to the edge of the bounding sphere).  x and y are screen space
	CFVec3A		vNonUnitToRegion;		// The camera space vector from the object towards the reticle.  Not unitized

	FCLASS_STACKMEM_ALIGN( FVis_ScreenMesh_t );
	
} FCLASS_ALIGN_SUFFIX;

// Visible lights list from last execution of fvis_Draw() for the given slot (Closest FVIS_MAX_VISIBLE_LIGHTS_LIST_COUNT visible lights)
extern FVisVisibleLight_t *FVis_paVisibleLights[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];
extern u16 FVis_anVisibleLightCount[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];

// Visible mesh list from last execution of fvis_Draw() for the given slot
extern CFWorldMesh **FVis_papVisibleMeshes[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];
extern u16 FVis_anVisibleMeshCount[FVIS_MAX_BUFFERED_RENDERED_DATA_SLOTS];

// Buffer filled in by calls to fvis_GetMeshesInScreenRegion()
extern u32 FVis_nMeshCountInRegion;
extern FVis_ScreenMesh_t FVis_aMeshesInRegion[FVIS_MAX_SCREEN_MESH_RESULTS];
extern FVis_ScreenMesh_t *FVis_apSortedMeshesInRegion[FVIS_MAX_SCREEN_MESH_RESULTS];

// pCamMtx should be the "forward" matrix of the camera
// Screen point should be a 2D X/Y coord with 0, 0 at the center of the screen and min of -1, -1 and max of 1, 1
// Radius should be passed in as a percentage of the screen size
// The data index refers to the index passed into fvis_Draw() for the particular render (usually the player index)
// Skip list used is the FWorld tracker skip list
// Returns the number of meshes in FVis_papVisibleMeshes that reside in the circle submitted 
extern u32 fvis_GetMeshesInScreenRegion( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, u32 nDataIndex,  
								 const CFVec2 *pScreenPoint, f32 fRadius, f32 fMaxDistance, f32 fTrackerSizeAdjust,
								 u64 nUserBitMask, BOOL bUseSkipList );
extern void fvis_SortRegionMeshResults( BOOL bAscend = TRUE );

// Interface to get the mesh pos with respect to a given screen region
extern void fvis_GetMeshPosInScreenRegion( const FViewport_t *pViewport, const CFMtx43A *pCamMtx, CFWorldMesh *pMesh, 
							     const CFVec2 *pScreenPoint, f32 fRadius, f32 fTrackerSizeAdjust, FVis_ScreenMesh_t *pResult );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Motif access for lightmaps
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern CFWorldLight* fvis_GetWorldLightByLightID( u32 nLightID );
extern CFWorldLight* fvis_GetWorldLightByMotifIndex( u32 nMotifIdx );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// World Initialization functions/vars
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CFMeshInst *FVis_pSkyBox;
extern u32 FVis_nCurLightIdx;
extern u32 FVis_nShadowBits[32];

extern  BOOL fvis_CreateVisData( FResHandle_t hRes, FVisData_t *pVisData, u32 nMaxIntersects = 0 );
extern  BOOL fvis_InitWorldStreamingData( void *pStreamingData, u32 nSize );
extern  BOOL fvis_ResolveLoadedVolumeMeshes( FVisGeoHeader_t *pGeoHeader );
FINLINE void fvis_RegisterSkyBox( CFMeshInst *pSkyBox )
{
	FVis_pSkyBox = pSkyBox;
}

FINLINE void fvis_ToggleLiquidInRenderTarget(BOOL bToggle)
{
	FVis_bAllowLiquid_RenderTarget = bToggle;
}

extern void fvis_SetNumShadows(u32 nLightIdx, u8 nValue);
extern u8 fvis_GetNumShadows(u32 nLightIdx);
extern void fvis_ClearNumShadows();

// This function should only ever be called externally by the tools!!
extern void fvis_SetActiveVisData( FVisData_t *pData );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Portal System IO functions:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	FVIS_PACK_NO_ERROR = 0,
	FVIS_PACK_EXCEEDED_MAX_LIGHTS,
	FVIS_PACK_EXCEEDED_MAX_VOLUMES,
	FVIS_PACK_EXCEEDED_MAX_PORTALS,
	FVIS_PACK_NO_PORTAL_DATA,
	FVIS_PACK_NO_MEMORY,
};

extern BOOL fvis_FixupPortalData( FVisData_t *pPortalData );
extern u32  fvis_PackPortalData( FVisData_t *pPortalData, BOOL bChangeEndian, void **pPackedData, u32 *pnDataSize );
extern BOOL fvis_ChangePortalDataEndian( FVisData_t *pRawPortalData );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !FANG_PRODUCTION_BUILD
	typedef BOOL FVisPlaneCallback_t( u32 nPlanes );
	void fvis_CalculateVolumePlanes( FVisVolume_t *pVolume, FVisPlaneCallback_t *pCallBack );
	extern BOOL fvis_VerifyTrackerCounts( void );
#endif FANG_PRODUCTION_BUILD

// Visibility system debug flags
extern BOOL8 FVis_bDrawVisibility;
extern BOOL8 FVis_bShowDrawStats;
extern BOOL8 FVis_bDrawCellsOnly;
extern BOOL8 FVis_bDontDrawObjectGeo;
extern BOOL8 FVis_bDontDrawPointSprites;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NORMAL STACK
//
//	This normal stack is used to handle viewport clipping
//	when projecting the viewport planes through portals
//	during the render phase.  Since all side planes have
//	the camera position in common, there is no need to
//	store a plane point.  Only a normal stack is needed
//	to represent the frustum planes.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FVIS_NORMAL_STACK_SIZE			512

FCLASS_ALIGN_PREFIX class CFVisNormalStack
{
	private:
		CFVec3A m_vCameraPos;
		CFVec3A m_avNormalStack[FVIS_NORMAL_STACK_SIZE];
		u8		m_nNormalCount[FVIS_NORMAL_STACK_SIZE / 4];
		u16		m_nTotalNormals;
		u16		m_nCurrentBase;
		u16		m_nPushCount;
		
	public:
		CFVisNormalStack( void )
		{	
			ResetStack( NULL );
			fang_MemSet( m_nNormalCount, 0, sizeof( u8 ) * (FVIS_NORMAL_STACK_SIZE / 4) );
		}
	
		FINLINE CFVec3A* GetCurrentBase( void )
		{
			return &m_avNormalStack[ m_nCurrentBase ];
		}
		
		FINLINE u32 GetCurrentCount( void )
		{
			return m_nNormalCount[ m_nPushCount ];
		}
		
		FINLINE void AddNormal( CFVec3A &vNormal )
		{
			FASSERT( m_nPushCount && m_nTotalNormals < FVIS_NORMAL_STACK_SIZE );
			m_nNormalCount[ m_nPushCount ]++;
			m_avNormalStack[ m_nTotalNormals++ ].Set( vNormal );
		}
		
		FINLINE void AddNormal( CFVec3 &vNormal )
		{
			FASSERT( m_nPushCount && m_nTotalNormals < FVIS_NORMAL_STACK_SIZE );
			m_nNormalCount[ m_nPushCount ]++;
			m_avNormalStack[ m_nTotalNormals++ ].Set( vNormal );
		}
		
		// Starts a new list of normals	in the stack	
		FINLINE void PushBase( void )
		{
			FASSERT( m_nPushCount < (FVIS_NORMAL_STACK_SIZE / 4) );
			m_nCurrentBase = m_nTotalNormals;
			m_nPushCount++;
			FASSERT( m_nNormalCount[m_nPushCount] == 0 );
		}
		
		// Reverts to the prior list of normals
		FINLINE u16 PopBase( void )
		{	
			FASSERT( m_nPushCount );
			m_nTotalNormals -= m_nNormalCount[m_nPushCount];
			m_nNormalCount[m_nPushCount--] = 0;
			m_nCurrentBase -= m_nNormalCount[m_nPushCount];
			return m_nTotalNormals;
		}
		
		// Returns TRUE if there was a base pushed onto the stack
		FINLINE BOOL ResetStack( CFVec3A *pCameraPos )
		{
			m_nCurrentBase = m_nTotalNormals = 0;
			m_nNormalCount[m_nPushCount] = 0;

			if ( pCameraPos )
			{
				m_vCameraPos.Set( *pCameraPos );
			}
			else
			{
				m_vCameraPos.Set( 0.f, 0.f, 0.f );
			}
			
			if ( m_nPushCount != 0 )
			{
				m_nPushCount = 0;
				return TRUE;
			}
				
			return FALSE;
		}
		
		typedef enum
		{
			OUTSIDE_FRUSTUM_BOUNDS = 0,
			WITHIN_FRUSTUM_BOUNDS,
			CROSSES_FRUSTUM_BOUNDS,
		} FrustumCheckResults_e;

		// Calculates whether a sphere is within the view frustum established by
		// the current base.  Returns FULLY_BEYOND_FRUSTUM_BOUNDS if the sphere is outside.  Otherwise
		// it returns the actual distance to the origin of the frustum taking into account fRadius and
		// returns whether or not the submitted point (and radius if applicable) is fully within the frustum.
		FINLINE FrustumCheckResults_e PointIsVisible( const CFVec3 *pPoint, f32 fRadius = 0.f, f32 *pfDistToCam = NULL )
		{
			FASSERT( pPoint );

			FrustumCheckResults_e nResult = WITHIN_FRUSTUM_BOUNDS;

			// Get Normal stack info (determines viewport)
			CFVec3A *paNormals = GetCurrentBase();
			u32 nNormalCount = GetCurrentCount();
			
			// Get difference between point and camera (since camera 
			// position will always lie on all planes of the frustum)
			CFVec3A vDiff;
			vDiff.v3 = *pPoint - m_vCameraPos.v3;
			
			// Check planes to see if point is visible
			u32 i;
			f32 fTest;
			for ( i = 0; i < nNormalCount; i++ )
			{
				fTest = vDiff.Dot( paNormals[i] );

				if ( fTest < fRadius )
				{
					// Sphere will intersect the frustum
					if ( fTest < -fRadius )
					{
						// If the radius cannot penetrate the frustum, then it isn't visible.
						return OUTSIDE_FRUSTUM_BOUNDS;
					}
					nResult = CROSSES_FRUSTUM_BOUNDS;
				}
			}
			
			f32 fDist = vDiff.Mag();

			if ( fDist < fRadius )
			{
				if ( pfDistToCam )
				{
					*pfDistToCam = 0.f;
				}
				return CROSSES_FRUSTUM_BOUNDS;
			}

			if ( pfDistToCam )
			{
				*pfDistToCam = fDist - fRadius;
			}

			return nResult;
		}
	
	FCLASS_STACKMEM_ALIGN( CFVisNormalStack );
		
} FCLASS_ALIGN_SUFFIX;

// The FVis viewport normal stack used for visibility determination
extern CFVisNormalStack FVis_VPNormalStack;

#endif