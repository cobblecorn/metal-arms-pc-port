//////////////////////////////////////////////////////////////////////////////////////
// fcoll.h - Fang collision module.
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

#ifndef _FCOLL_H_
#define _FCOLL_H_ 1

#include "fang.h"
#include "fmath.h"


#define FCOLL_USER_TYPE_BITS_ALL		0xffffffffffffffff


class CFMeshInst;
class CFWorldMesh;
class CFWorldTracker;
struct FMesh_Coll_TransformedkDOP_t;

//-------------------------------------------------------------------------------------------------------------------
// FCollInfo_t:
//-------------------------------------------------------------------------------------------------------------------

typedef enum {
	FMESH_COLLTESTTYPE_SPHERE = 0,			// Perform sphere test
	FMESH_COLLTESTTYPE_PROJSPHERE,			// Perform projected sphere test
	FMESH_COLLTESTTYPE_RAY,					// Perform ray test
//	FMESH_COLLTESTTYPE_CYLINDER,			// Perform cylinder test
	FMESH_COLLTESTTYPE_CAPSULE,				// Perform capsule test using cylinder

	FMESH_COLLTESTTYPE_COUNT
} FMeshCollTestType_t;


//
// COLLISION MASK SETTINGS
//
enum
{
	FCOLL_MASK_NONE								= 0x0000,
	FCOLL_MASK_COLLIDE_WITH_PLAYER				= 0x0001,
	FCOLL_MASK_COLLIDE_WITH_NPCS				= 0x0002,
	FCOLL_MASK_OBSTRUCT_LINE_OF_SIGHT			= 0x0004,
	FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES	= 0x0008,
	FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES	= 0x0010,
	FCOLL_MASK_COLLIDE_WITH_CAMERA				= 0x0020,
	FCOLL_MASK_COLLIDE_WITH_OBJECTS				= 0x0040,
	FCOLL_MASK_WALKABLE							= 0x0080,
	FCOLL_MASK_OBSTRUCT_SPLASH_DAMAGE			= 0x0100,
	FCOLL_MASK_COLLIDE_WITH_DEBRIS				= 0x0200,
	FCOLL_MASK_COLLIDE_WITH_VEHICLES			= 0x0400,
	FCOLL_MASK_HOVER_COLLIDABLE					= 0x0800,
	
	FCOLL_MASK_CHECK_ALL						= 0xffff
};


//
// RESULTS LEVEL-OF-DETAIL SETTINGS - Use these to specify the depth of the test
//
enum
{
	FCOLL_LOD_LOWEST	= 0x01,	//	Basic bounding volume (i.e. sphere) 
	FCOLL_LOD_LOW		= 0x02,	//	Root of collision tree (i.e. kDOP)
	FCOLL_LOD_MEDIUM	= 0x04,	//	Middle of collision tree (i.e. mid level kDOP)
	FCOLL_LOD_HIGH		= 0x08,	//	Leaf node in collision tree (i.e. leaf kDOP)
	FCOLL_LOD_HIGHEST	= 0x10	//	Triangle collision (i.e. leaf kDOP node triangle packets)
};


//
// COLLISION TYPE REFERENCE - suggested collision types
//
/*	
	Bits 0-2: Surface Type Index
	Bits 3-6: Material Type Index
	Bit 7: Reserved for expansion

	Surface Type Index:
	0 = Normal
	1 = Slippery
	2 = Low hurt
	3 = Medium hurt
	4 = Instant kill
	5 = Vulnerable
	6-7 = Reserved for expansion

	Material Type Index:
	0 = None
	1-15 = Level-specific
*/


//
//
//
FCLASS_ALIGN_PREFIX class CFCollInfo
{
	public:
		// Always used:
		FMeshCollTestType_t nCollTestType;		// Collision test type
		u16 nCollMask;							// Only objects/triangles that include one or more of these mask bits will be considered
		u8 nResultsLOD;							// Definition of the level of detail that is requested for the results (See above)

		// Used for projected sphere test:
		BOOL8 bCalculateImpactData;				// TRUE = Collision system will return WS normal, tri vertices, etc.
												// FALSE = Collision system will only return distance to collision
		
		u64 nTrackerUserTypeBitsMask;			// Used to auto-exclude certain trackers based on the tracker user type bitfield (Set to FCOLL_USER_TYPE_BITS_ALL if you desire no pre-culling of trackers)

		// Used for all tests
		u16 nStopOnFirstOfCollMask;				// Sets the mask that will cause the test to stop.  Set to FCOLL_MASK_CHECK_ALL to stop on any mask.  Note that this is not necessarily stopping at the closest impact
		
		// Used for ray AND projected sphere:
		BOOL8 bFindClosestImpactOnly;			// TRUE = Collision system will clip the ray or radius, internally, as it finds impacts, resulting in a faster test (Caller can assume last impact is the closest)
		BOOL8 bCullBacksideCollisions;			// TRUE = Don't collide with triangles that the ray/sphere enters from their back side
		
		// Used for sphere test:
		CFSphere *pSphere_WS;					// Pointer to the current master sphere in world space
//		CFSphere *pCurSphereList_WS;			// Pointer to the current sphere list (spheres are in world space) or NULL if nSphereListCount is 0
//		u32 nSphereListCount;					// Number of spheres in the sphere list (0=collide with master sphere only) (will not exceed Fang_ConfigDefs.nMesh_MaxCollSpheres)

		// Used for ray test:
		CFRay Ray;								// Ray
		
		// Used for projected sphere test:
		CFProjSphere ProjSphere;				// projected sphere test

		// Used for cylinder test:
//		CFCylinder Cylinder;					// Cylinder

		void *pTag;								// Stored in FCollImpact_t::pTag

	FCLASS_STACKMEM_ALIGN( CFCollInfo );
} FCLASS_ALIGN_SUFFIX;




//-------------------------------------------------------------------------------------------------------------------
// FCollImpact_t:
//-------------------------------------------------------------------------------------------------------------------
#if FANG_PLATFORM_DX
typedef struct {
	CFMeshInst *pMeshInst;	// Pointer to mesh instance that contains this triangle
	void *pCollSet;			// Pointer to the collision set (FDX8MeshHW_CollSet_t *) that contains this triangle
	u16 nCollTriIndex;		// Index into FDX8MeshHW_CollSet_t::pCollTriArray of the triangle
	u16 nSegIndex;			// Index of the segment that contains this triangle (0 for world meshes)
} FDX8MeshCollTriCookie_t;
#endif


// Collision Impact: See descriptions in the fcoll_Check() section, below, regarding
//					 how that function call fills out the impacts
FCLASS_ALIGN_PREFIX struct FCollImpact_t 
{
	CFVec3A aTriVtx[3];		// The triangle's 3 vertices in world space
	CFVec3A UnitFaceNormal;	// The triangle's unit face normal in world space
	CFVec3A ImpactPoint;	// Point of impact in world space (lies on the intersecting triangle). For cylinders
							//   this is the point that the centerline intersects with the triangle. 
							//   fImpactDistInfo = FMATH_MAX_FLOAT indicates that there is no cylinder 
							//   centerline intersection.
	
	CFVec3A PushUnitVec;	// Ray: Not used
							// Sphere(s): The unit vector along which the sphere must be displaced
							//   in order to not be penatrating (multiply this by fImpactDistInfo for full push vec)
							// Projected Sphere: Not used
							// Cylinder: The unit vector along which the cylinder must be displaced
							//   in order to move it out of the current radial collision.
/*
	// Only used for cylinder collision:
	CFVec3A vHeightImpact;	// Cylinder: Closest intersection of the tri to the start point of the cylinder
	CFVec3A vRadialImpact;	// Cylinder: Point of furthest pentration of the triangle across the radius of the cylinder
	f32 fRadialDistInfo;	// Cylinder: The 2D distance from the opposite edge of the cylinder to the radial impact
	f32 fHeightDistInfo;	// Cylinder: The 2D distance from the start point to the height impact
*/

	// Applies to fcoll_Check() calls
	f32 fUnitImpactTime;	// Specifies the unit time along CFCollData::pMovement when collision occurs or -1 if primitive starts in collision state

	// Applies to all:						
	f32 fImpactDistInfo;	// Ray: The unit distance from the start point along the ray to ImpactPoint
							// Sphere(s): The distance from the impact point to the sphere's outside
							// Projected Sphere: The distance the sphere can move in the requested direction without colliding
							// Cylinder: The distance down the centerline where the triangle collision occurs, or FMATH_MAX_FLOAT if the centerline does not intersect

	// Applies to all:
	u16 nUserType;			// User-defined collision type
	u8 nBoneIndex;			// Which bone in a segmented mesh this impact is describing (0 when not used)
	u8 nSourceBoneIndex;	// See fcoll_Check() descriptions, below
//	u8 nSphereIndex;		// Which sphere in the sphere-list this impact is describing (0 for ray collisions)
	
	void *pTag;				// Value of FCollInfo_t::pTag

	FCLASS_STACKMEM_ALIGN( FCollImpact_t );

} FCLASS_ALIGN_SUFFIX;


#if !FANG_PRODUCTION_BUILD
	extern u32 FColl_nImpactCountOverLimit; // Totals up the number of impacts over the limit
#endif

extern u32 FColl_nMaxCollImpacts;				// Maximum number of impacts that FColl_aCollImpactBuf can hold
extern u32 FColl_nImpactCount;					// Number of impacts currently in FColl_aCollImpactBuf
extern u16 FColl_nImpactCollMask;				// The accumulation of all the coll masks of the tris in the impact buff

extern FCollImpact_t *FColl_aImpactBuf;			// Impact buffer
extern FCollImpact_t *FColl_pNextImpactSlot;	// Next empty slot in impact buffer
extern FCollImpact_t *FColl_pImpactBufEnd;		// The slot beyond the last in the FColl_aCollImpactBuf

extern FCollImpact_t **FColl_apSortedImpactBuf;	// Impact buffer


extern BOOL fcoll_ModuleStartup( void );
extern void fcoll_ModuleShutdown( void );

extern void fcoll_Clear( void );
extern void fcoll_Next( void );

extern void fcoll_Sort( BOOL bAscend=TRUE );
extern void fcoll_SortDynamic( BOOL bAscend=TRUE );
extern FCollImpact_t *fcoll_FindClosest( FCollImpact_t *pStartImpact=FColl_aImpactBuf, FCollImpact_t *pEndImpact=FColl_pNextImpactSlot );

extern void fcoll_Renderer_Open( void );
extern void fcoll_Renderer_Close( void );
extern u32 fcoll_Renderer_GetStateSize( void );
extern void fcoll_Renderer_GetState( void *pDestState );
extern void fcoll_Renderer_SetState( const void *pState );
extern void fcoll_Renderer_SetDefaultState( void );



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Functions and structures for testing objects and primitives for collision
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FCollIntersectingTrackerCallback_t return values
typedef enum
{
	// These are mutually exclusive
	FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER	= 0x00000001,	// Will cause fcoll_Check() not to check this tracker but collision query will continue
	FCOLL_CHECK_CB_FIRST_IMPACT_ONLY	= 0x00000002,	// Will cause fcoll_Check() to check this tracker for the first impact only and continue
	FCOLL_CHECK_CB_ALL_IMPACTS			= 0x00000004,	// Will cause fcoll_Check() to check this tracker for all impacts and continue

	// These are mutually exclusive
	FCOLL_CHECK_CB_EXIT_AFTER_TRACKER	= 0x20000000,	// Will cause fcoll_Check() to exit after checking this tracker
	FCOLL_CHECK_CB_EXIT_ON_IMPACT		= 0x40000000,	// Will cause fcoll_Check() to exit if an impact is determined
	FCOLL_CHECK_CB_EXIT_IMMEDIATELY		= 0x80000000,	// Will cause fcoll_Check() to immediately exit

};

// Tracker verification callback for fcoll_Check() calls.  The callback function
// must return one of the callback return values specified above.
typedef u32 FCollIntersectingTrackerCallback_t( CFWorldTracker *pTracker );


//
//	These flags are enum'ed such that the most common settings would most likely require nFlags == 0
//	
enum
{
	FCOLL_DATA_FLAGS_NONE				= 0x0000,	
	FCOLL_DATA_IGNORE_WORLD				= 0x0002,	// World geometry will not be tested for collision
	FCOLL_DATA_IGNORE_STATIC_OBJECTS	= 0x0004,	// Static objects will not be tested for collision
	FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS	= 0x0008,	// Dynamic objects will not be tested for collision
	FCOLL_DATA_IGNORE_BACKSIDE			= 0x0010,	// Collision checks will not be performed against polys facing away from the movement (note that this may cause issues if there is rotational velocity)
	FCOLL_DATA_IGNORE_COLLISION_FLAG	= 0x0020,	// Trackers that are set as non-collidable will still be tested
	FCOLL_DATA_DONT_CALCULATE_IMPACT	= 0x0040,	// If set, the collision impact buffer will not calculate the actual impact, but all other info will be valid.  This will be much faster

	FCOLL_DATA_USE_OBJECT_SPHERE		= 0x1000,	// If checking the collision of a mesh, it will use the mesh's collision sphere(s) to detect collision (default is kDOP)
	FCOLL_DATA_USE_OBJECT_CAPSULE		= 0x2000,	// If checking the collision of a mesh, it will use the mesh's collision capsule(s) to detect collision (default is kDOP)
};


//
//
//
FCLASS_NOALIGN_PREFIX class CFCollData
{
	public:
		// Used for all tests
		u16		nFlags;							// See FCOLL_DATA_* flags, above
		u16		nCollMask;						// Only objects/triangles that include one or more of these mask bits will be considered
		CFVec3A *pMovement;						// Proposed movement (or NULL) for the tested primitive (If NULL, static collision will be performed)
		
		// Used for all tests
		u16 nStopOnFirstOfCollMask;				// Sets the mask that will cause the test to stop.  Set to FCOLL_MASK_CHECK_ALL to stop on any mask.  Note that this is not necessarily stopping at the closest impact

		// Only used for tests that include static or dynamic objects
		u64		nTrackerUserTypeBitsMask;				// Used to auto-exclude certain trackers based on the tracker user type bitfield (Set to 0xffffffffffffffff if you desire no pre-culling of trackers)
		FCollIntersectingTrackerCallback_t *pCallback;	// Callback that will be called to verify that tracker should be tested. (NULL = all trackers that match nFlags will be tested)
														// If callback returns TRUE, then tracker will be tested, otherwise skipped.

		// Used to specify a tracker skip list
		u32 nTrackerSkipCount;								// Number of trackers in the skip list
		const CFWorldTracker * const *ppTrackerSkipList;	// pointer to an array of tracker pointers to skip
		
		// Used for simple primitive (sphere, capsule, etc) collision tests
		CFWorldTracker *pLocationHint;					// This tracker should provide a hint as to what volume the submitted primitive resides in.

	FCLASS_STACKMEM_NOALIGN( CFCollData );
} FCLASS_NOALIGN_SUFFIX;

//
// The projected collision functions will fill in the following info in the impact buffer
//
//		CFVec3A aTriVtx[3];		// The triangle's 3 vertices in world space
//		CFVec3A UnitFaceNormal;	// The triangle's unit face normal in world space
//		CFVec3A PushUnitVec		// The unit direction that results in the shortest push to get this object out of the reported impact
//		f32 fImpactDistInfo;	// The magnitude along PushUnitVector to offset pMove to prevent this collision impact
//		f32 fUnitImpactTime;	// Specifies the unit time along CFCollData::pMovement when collision occurs, or -1 if the mesh/primitive starts in a collision state
//		u16 nUserType;			// User-defined collision type of the triangle
//		u8 nSourceBoneIndex;	// The bone of the projected mesh that collided, if applicable
//		void *pTag;				// Pointer to the CFWorldTracker that caused this collision, if applicable
//		u8 nBoneIndex;			// The bone index for the CFWorldMesh that caused the collision, if applicable
//
// And if FCOLL_DATA_DONT_CALCULATE_IMPACT is not set:
//
//		CFVec3A ImpactPoint;	// First point of contact between the triangle and the mesh/primitive in world space (lies on the intersecting triangle).
//								// Note that this point might be an average of many points - as in the case where the face or edge of the triangle contacts a face or edge of the kDOP
//
//	Note that applying PushUnitVec * fImpactDistInfo has the effect of sliding the submitted mesh/primitive along the plane of contact with
//			the impact.  However, it is not guaranteed that this will not result in passing through another potential collision.  To accomplish
//			this, you should limit the mesh/primitive's movement to the point of impact (using fUnitImpactTime), then resubmit a collision check 
//			from the point of impact to the point the mesh/primitive would slide to if you applied PushUnitVec * fImpactDistInfo.  This does not
//			apply, of course, if the mesh/primitive starts in a collision state (fUnitImpactTime == -1.f) which is always the case with impacts
//			resulting from static collision checks.
//

// It is the responsibility of the calling function to call fcoll_clear(), if desired prior to these functions:
// returns TRUE if a collision occurred
//
//	fcoll_Check( CFWorldMesh ) - Will check a mesh's bounding volume (as specified by the nFlags parameter)
//					to determine whether or not the mesh is or will become in a collision state.  The
//					pMeshMtx parameter can be used to override the mesh's m_Xfm when determining the mesh
//					location and orientation.  This is useful when iterating through the collision system.
//					However, the matrix cannot be used as an override for animating boned matrices since
//					the mesh's m_Xfm is built into the bone matrix pallette.
extern BOOL fcoll_Check( CFCollData *pInfo, CFWorldMesh *pMeshInst, CFMtx43A *pMeshMtx = NULL, s32 nBone = -1 );
extern BOOL fcoll_Check( CFCollData *pInfo, CFSphere *pSphere );
extern BOOL fcoll_Check( CFCollData *pInfo, CFCapsule *pCapsule );
//extern BOOL fcoll_Check( CFCollData *pInfo, CFBox *pBox );

//
// This is mainly intended as a utility function for fang.  Returns true if the point is in the triangle
//
FINLINE BOOL fcoll_PointIsInTri( const CFVec3A &vPointMinusV0, const CFVec3A &vEdgeV1MinusV0, const CFVec3A &vEdgeV2MinusV0, const CFVec3A &vTriNormal )
{
	static CFVec3A __vCross;
	
	f32 fDet = __vCross.Cross( vEdgeV1MinusV0, vEdgeV2MinusV0 ).MagSq();
	if ( fDet == 0.f )
	{
		return FALSE;
	}
	
	fDet = fmath_InvSqrt( fDet );
	
	__vCross.Cross( vPointMinusV0, vEdgeV2MinusV0 );
	f32 fU = __vCross.Dot( vTriNormal ) * fDet;
	if ( fU < 0.f || fU > 1.f )
	{
		return FALSE;
	}
	
	__vCross.Cross( vEdgeV1MinusV0, vPointMinusV0 );
	f32 fV = __vCross.Dot( vTriNormal ) * fDet;
	if ( fV < 0.f || fU + fV > 1.f )
	{
		return FALSE;
	}
	
	return TRUE;
}


#endif

