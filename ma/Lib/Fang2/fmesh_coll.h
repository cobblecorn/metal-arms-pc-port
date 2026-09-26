//////////////////////////////////////////////////////////////////////////////////////
// fmesh_coll.h - Fang mesh collision module.
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
// 07/11/02 Lafleur		Accumulated from collision code in existing modules
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FMESH_COLL_H_
#define _FMESH_COLL_H_ 1

#include "fang.h"
#include "fkdop.h"
#include "fcoll.h"


class CFMeshInst;


//#define FMESH_COLL_MAX_SPHERES_IN_LIST			32
#define FMESH_COLL_NODE_STACK_SIZE				128

#if !FANG_PRODUCTION_BUILD
	#if FANG_PLATFORM_WIN	
		#define FMESH_COLL_ENABLE_COLLISION_DRAWS	FALSE
	#else
		#define FMESH_COLL_ENABLE_COLLISION_DRAWS	FALSE
	#endif
	#if FMESH_COLL_ENABLE_COLLISION_DRAWS
		#define FMESH_DRAW_COLLIDED_TRIS			TRUE
		#define FMESH_DRAW_CONSIDERED_TRIS 			FALSE
		#define FMESH_DRAW_COLLIDED_KDOPS 			TRUE
	#else
		#define FMESH_DRAW_COLLIDED_TRIS			FALSE
		#define FMESH_DRAW_CONSIDERED_TRIS 			FALSE
		#define FMESH_DRAW_COLLIDED_KDOPS 			FALSE
	#endif
#else
	#define FMESH_COLL_ENABLE_COLLISION_DRAWS	FALSE
#endif

//
//	kDOP vert types - Used for dynamic models
//

//
//
struct FkDOP_Vert32_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FkDOP_Vert32_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	f32	x, y, z;
	void AssignTo( CFVec3A *pDest )
	{
		pDest->x = x;
		pDest->y = y;
		pDest->z = z;
		pDest->w = 0.f;
	}
	void ChangeEndian( void )
	{
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
	}
	
};

//
//
struct FkDOP_Vert16_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FkDOP_Vert16_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	s16	x, y, z;
	void DecompressTo( CFVec3A *pDest, f32 fMod )
	{
		pDest->x = (f32)x * fMod;
		pDest->y = (f32)y * fMod;
		pDest->z = (f32)z * fMod;
		pDest->w = 0.f;
	}
	void ChangeEndian( void )
	{
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
	}
	
};

//
//
struct FkDOP_Vert8_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FkDOP_Vert8_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	s8	x, y, z;
	void DecompressTo( CFVec3A *pDest, f32 fMod )
	{
		pDest->x = (f32)x * fMod;
		pDest->y = (f32)y * fMod;
		pDest->z = (f32)z * fMod;
		pDest->w = 0.f;
	}
	void ChangeEndian( void )
	{
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
	}
	
};


//
// Compressed normal (14 bits of fractional component)
// Note that an 8-bit compressed normal is not very effective as 
// it results in visible collision error on larger polys
//
#define FMESH_NORMAL_COMPRESS_MOD		16384
//ARG#define FMESH_NORMAL_DECOMPRESS_MOD		(1.f / FMESH_NORMAL_COMPRESS_MOD.f)
#define FMESH_NORMAL_DECOMPRESS_MOD		(1.f / (float)FMESH_NORMAL_COMPRESS_MOD)	//ARG - SN compiler .f ERROR
struct FkDOP_CNormal_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FkDOP_CNormal_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	s16 x, y, z;
	
	void DecompressTo( CFVec3A &vResult ) const
	{
		vResult.x = (f32)x * FMESH_NORMAL_DECOMPRESS_MOD;
		vResult.y = (f32)y * FMESH_NORMAL_DECOMPRESS_MOD;
		vResult.z = (f32)z * FMESH_NORMAL_DECOMPRESS_MOD;
	}
	
	void CompressFrom( const CFVec3 &vInput )
	{
		x = (s16)(vInput.x * FMESH_NORMAL_COMPRESS_MOD);
		y = (s16)(vInput.y * FMESH_NORMAL_COMPRESS_MOD);
		z = (s16)(vInput.z * FMESH_NORMAL_COMPRESS_MOD);
	}
	
	void ChangeEndian( void )
	{
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
	}
};


//
// Uncompressed normal
//
typedef CFVec3 FkDOP_Normal_t;


//
//	
//
struct FkDOP_TriPacket_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FkDOP_TriPacket_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u16	nCollMask;					// See mask settings (COLLMASK_*), in fcoll.h
	u16	nCollType;					// See type reference, in fcoll.h
	u16 nStartVert;					// Start vert index in pTriData
	u8  nVBIdx;						// Index into the FGCMesh_t VB array
	u8  nTriCount;					// Number of tris in this packet
	
	void ChangeEndian( void )
	{
		nCollMask = fang_ConvertEndian( nCollMask );
		nCollType = fang_ConvertEndian( nCollType );
		nVBIdx = fang_ConvertEndian( nVBIdx );
		nStartVert = fang_ConvertEndian( nStartVert );
		nTriCount = fang_ConvertEndian( nTriCount );
	}
};

//
//	FkDOP_Node_t - A tree node for the kDOPTree.  If nTriPacketCount is non-zero, this node is a leaf
//
struct FkDOP_Node_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FkDOP_Node_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	FkDOP_TriPacket_t *paPackets;	// paPackets points to the array of tri-packets in this leaf
	void *pTriData;					// If paPackets is non-NULL, pTriData points to the beginning of verts and normals

	u16	nCollMask;
	u16	nStartkDOPInterval; // Start index for the kDOP intervals. The number of intervals is always KDOP_SIDES_COUNT / 2;
	
	u8 nTriPacketCount;		// If this node is a leaf, the number of tri packets in the leaf
	u8 __PAD;
	
	union
	{
		// If paPackets is NULL
		u16 nStartChildIdx;		// The first index of the child DOPs in the binary kDOP tree
		// If paPackets is non-NULL
		u16	nTriCount;			// If this is a leaf, then this is the number of tris
	};
	
	void ChangeEndian( void )
	{
		nCollMask = fang_ConvertEndian( nCollMask );
		nStartkDOPInterval = fang_ConvertEndian( nStartkDOPInterval );
		
		nTriPacketCount = fang_ConvertEndian( nTriPacketCount );
		nTriCount = fang_ConvertEndian( nTriCount );
		
		paPackets = (FkDOP_TriPacket_t *)fang_ConvertEndian( paPackets );
		pTriData = (void *)fang_ConvertEndian( pTriData );
	}
};


//
//
//
struct FMesh_Coll_Capsule_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMesh_Coll_Capsule_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	CFVec3 vStart;
	CFVec3 vEnd;
	f32	   fRadius;

	void ChangeEndian( void )
	{
		vStart.ChangeEndian();
		vEnd.ChangeEndian();
		fRadius = fang_ConvertEndian( fRadius );
	}
};


//
enum
{
	FMESH_KDOP_8BIT_VERTS		= 0x0001,
	FMESH_KDOP_16BIT_VERTS		= 0x0002,
};


//
//	Collision Data structure
//
struct FkDOP_Tree_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FkDOP_Tree_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u8				nFlags;
	u8				nSegmentIdx;		// Index into the mesh's segment array indicating which segment this tree is for
	u16				nMasterCollMask;	// Or'ing of all node CollMask bits
	
	u8				nFracBits;			// Number of bits in the fractional component, if compressed.  This applies to kDOP verts as well as collision geo verts
	u8				nTreekDOPType;		// Type of kDOP for the kDOP tree (using the enum in fKDOP.h)
	u16				nTreeNodeCount;		// Number of kDOP nodes in this tree
	FkDOP_Node_t	*pakDOPNodes;		// Array of kDOP nodes in the tree
	
	u8				__PAD[2];
	
	u8				nRootkDOPType;			// Type of kDOP for the root kDOP (using the enum in fKDOP.h) 
	u8				nRootkDOPVertCount;		// Number of uncompressed kDOP verts in the root node
	CFVec3			*paRootkDOPVerts;		// Uncompressed kDOP verts for the root node
	CFSphere		spRootkDOPBounding;		// Sphere that bounds the RootkDOP, in model-space (Note that this bounds the kDOP verts, 
												//		so it is a little larger than necessary to bound the mesh verts).  Use segment BS for a tighter one.

	FMesh_Coll_Capsule_t	BoundingCapsule;
	
	// Count always = (KDOP_SIDES_COUNT / 2) * nTreeNodeCount:
	FkDOP_Interval_t	*paIntervals;	// Initial kDOP intervals along the axes.  This may be overridden by the mesh instance if the mesh is not static.
	
	void ChangeEndian( void )
	{
		nFlags = fang_ConvertEndian( nFlags );
		nMasterCollMask = fang_ConvertEndian( nMasterCollMask );
		
		nFracBits = fang_ConvertEndian( nFracBits );
		nTreekDOPType = fang_ConvertEndian( nTreekDOPType );
		nTreeNodeCount = fang_ConvertEndian( nTreeNodeCount );
		pakDOPNodes = (FkDOP_Node_t *)fang_ConvertEndian( pakDOPNodes );
		
		nSegmentIdx = fang_ConvertEndian( nSegmentIdx );
		
		nRootkDOPType = fang_ConvertEndian( nRootkDOPType );
		nRootkDOPVertCount = fang_ConvertEndian( nRootkDOPVertCount );
		paRootkDOPVerts = (CFVec3 *)fang_ConvertEndian( paRootkDOPVerts );
		spRootkDOPBounding.ChangeEndian();

		BoundingCapsule.ChangeEndian();
		
		paIntervals = (FkDOP_Interval_t *)fang_ConvertEndian( paIntervals );
	}
};


//
//
typedef struct FMesh_Coll_RayTest_s
{
	// These parameters are all in bone space (for world geo
	// that is the same as world space.  Also, the parameters
	// could be scaled if the object or bone has scale in it)
	CFVec3A 		vStart;
	CFVec3A 		vEnd;
	CFVec3A 		vNormal;
	f32				fDistance;				// This distance my be clipped if there is "find closest impact" flag set
	f32				fOriginalDistance;		// This distance is always the original distance submitted to the coll routine
} FMesh_Coll_RayTest_t;
	
//
//
typedef struct FMesh_Coll_SphereTest_s
{
	// These parameters are all in bone space (for world geo
	// that is the same as world space.  Also, the parameters
	// could be scaled if the object or bone has scale in it)
	CFVec3A 		vCenter;
	f32				fRadius;
	f32				fRadiusSq;
} FMesh_Coll_SphereTest_t;
	
//
//
typedef struct FMesh_Coll_ProjSphereTest_s
{
	// These parameters are all in bone space (for world geo
	// that is the same as world space.  Also, the parameters
	// could be scaled if the object or bone has scale in it)
	CFVec3A 		vStart;
	CFVec3A 		vEnd;
	CFVec3A 		vMoveNormal;
	CFVec3A 		vMoveDelta;
	f32				fMoveDistance;
	f32				fRadius;
	f32				fRadiusSq;
} FMesh_Coll_ProjSphereTest_t;
/*	
//
//
typedef struct FMesh_Coll_CylinderTest_s
{
	const CFMtx43A	*pMeshToCylinder;
	const CFMtx43A	*pMeshToCylinderNoScale;
	const CFMtx43A	*pCylinderToWorld;
	
	// These parameters are all in bone space (for world geo
	// that is the same as world space.  Also, the parameters
	// could be scaled if the object or bone has scale in it)
	CFVec3A 		vStart;
	CFVec3A 		vEnd;
	CFVec3A 		vCenterPoint;
	CFVec3A 		vNormal;
	f32				fHeight;
	f32				fRadius;
	f32				fRadiusSq;
	BOOL			bWorldGeo;
} FMesh_Coll_CylinderTest_t;
*/

// "Stack" variables for traversing kDOP nodes:
extern FkDOP_Node_t *FMesh_Coll_apNodeStack[ FMESH_COLL_NODE_STACK_SIZE ];
extern u32 FMesh_Coll_nNodeStackIdx;
#if !FANG_PRODUCTION_BUILD
	extern u32 FMesh_Coll_nDeepestNodeStackIdx;
	extern u32 FMesh_Coll_nNodesChecked;
	extern u32 FMesh_Coll_nDOPTransforms;
	// Collision culling analysis
	extern u32 FMesh_Coll_nTests;
	extern u32 FMesh_Coll_nTestsLevel1;
	extern u32 FMesh_Coll_nTestsLevel2;
	extern u32 FMesh_Coll_nHits;
#endif

// Current collision request vars
extern const CFMtx43A		*FMesh_pModelToWorld;
extern const CFCollInfo		*FMesh_Coll_pCurrCollInfo;
extern const CFMeshInst		*FMesh_Coll_pCurrMeshInst;
extern FkDOP_Tree_t			*FMesh_Coll_pCurrkDOPTree;
extern FkDOP_Tree_t			*FMesh_Coll_pCurrkDOPTree2;
extern u8					FMesh_Coll_nCurrBoneIndex;
extern u16					FMesh_Coll_nCurrCollMask;
extern u32					FMesh_Coll_nClosestImpactIdx;
extern f32 					FMesh_Coll_fBoneScale;
extern f32 					FMesh_Coll_fBoneScaleR;
extern FCollImpact_t 		*FMesh_Coll_pClosestImpactSlot;

extern CFMtx43A	Fmesh_Coll_mtxkDOPToWorld0;
extern CFMtx43A	Fmesh_Coll_mtxWorldTokDOP0;
extern f32		Fmesh_Coll_kDOPBoneScale0;
extern CFMtx43A	Fmesh_Coll_mtxkDOPToWorld1;
extern CFMtx43A	Fmesh_Coll_mtxWorldTokDOP1;
extern f32		Fmesh_Coll_kDOPBoneScale1;
extern u16		Fmesh_Coll_nAccumCollMask;



//
//
//
FCLASS_ALIGN_PREFIX struct FMesh_Coll_TransformedkDOP_t
{
	CFVec3A				avTransVerts[FKDOP_MAX_VERTS];
	CFVec3A 			aTransAxes[FKDOP_MAX_AXES];
	CFVec3A 			aTransEdges[FKDOP_MAX_EDGE_NORMALS];
	FkDOP_Interval_t 	aTransIntervals[FKDOP_MAX_AXES];
	FkDOP_Interval_t 	aIntervals[FKDOP_MAX_AXES];
	u32					nGenerationFrame;
	const FkDOP_Tree_t	*pOwner;
	
	FCLASS_STACKMEM_ALIGN( FMesh_Coll_TransformedkDOP_t );
	
} FCLASS_ALIGN_SUFFIX;


// Sphere list transformed into object space
//extern FMesh_Coll_SphereTest_t FMesh_Coll_spListXfm[ FMESH_COLL_MAX_SPHERES_IN_LIST ];

// General Implementation:
extern u32  fmesh_coll_CalculateBoneMatrix( const CFMeshInst *pMeshInst, u32 nkDOPTreeIdx, CFMtx43A *pBoneMtx, CFMtx43A *pInvBoneMtx = NULL, f32 *pfBoneScale = NULL, f32 *pfInvBoneScale = NULL );
extern void fmesh_Coll_TestSphere( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo );
extern void fmesh_Coll_TestRay( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo );
//extern void fmesh_Coll_TestCylinder( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo );
extern void fmesh_Coll_TestProjSphere( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo );
extern void fmesh_Coll_TestkDOPWithVelocity( CFMeshInst *pMeshInst, CFMeshInst *pTestMeshInst, CFVec3A *pVelocity, u16 nCollisionMask );
extern void fmesh_Coll_TestDOPWithWorldMesh( FMesh_Coll_TransformedkDOP_t *pkDOP );

//extern void fmesh_Coll_TransformSphereList( FMesh_Coll_SphereTest_t *pTest );

// Platform specific implementation:
//extern void fmesh_Coll_TestSphereListWithDOPs( FMesh_Coll_SphereTest_t *pTest );
extern void fmesh_Coll_TestSphereWithDOPs( const FMesh_Coll_SphereTest_t *pTest );
extern void fmesh_Coll_TestRayWithDOPs_NoBF( FMesh_Coll_RayTest_t *pTest );
extern void fmesh_Coll_TestRayWithDOPs_BF( FMesh_Coll_RayTest_t *pTest );
//extern void fmesh_Coll_TestCylinderWithDOPs( FMesh_Coll_CylinderTest_t *pTest );
//extern void fmesh_Coll_TestCapsuleWithDOPs( FMesh_Coll_CylinderTest_t *pTest );
extern BOOL fmesh_Coll_TestProjSphereWithDOPs( FMesh_Coll_ProjSphereTest_t *pTest );
//extern void fmesh_Coll_TriTriIntersect( FMesh_t *pMesh0, FkDOP_Node_t *pNode0, FMesh_t *pMesh1, FkDOP_Node_t *pNode1 );

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	extern BOOL8 FMesh_Coll_bShowCollision;

	extern void fmesh_Coll_DrawCollGeo( CFMeshInst *pMeshInst, u32 nDrawFlags );
	extern void fmesh_Coll_DrawMeshTri( const CFVec3A *pVertArray, const CFVec3A *pNormal, const CFMtx43A *pTransform, CFColorRGBA *pColor, BOOL bLight );
	extern void fmesh_Coll_AddCollDrawTri( const CFVec3A *pVertArray, const CFMtx43A *pTransform );
	extern void fmesh_Coll_AddCollDrawDOP( const FkDOP_Interval_t *pIntervals, const CFMtx43A *pMtx, u32 nkDOPType );
	extern void fmesh_Coll_AddCollDrawImpact( const CFVec3 *pImpact, u8 nRed = 255, u8 nGreen = 255, u8 nBlue = 255 );
	extern void fmesh_Coll_DrawCollisions( void );
	extern void fmesh_Coll_WalkTreeAndDraw( u32 nDrawFlags, CFMtx43A *pResultToWorld );
#endif


#define FKDOP_TRANSFORMED_KDOP_BUFFER_SIZE		32

extern FMesh_Coll_TransformedkDOP_t				*FKDOP_paTransformedkDOPBuffer;
extern u32										FKDOP_nTransformedkDOPIdx;

extern FMesh_Coll_TransformedkDOP_t* fmesh_Coll_CalculateDOPinWS( CFMeshInst *pMeshInst, u16 nTreeIdx, const CFMtx43A *pMeshMtx = NULL );
extern void fmesh_Coll_GeneratekDOPIntervals( FMesh_Coll_TransformedkDOP_t *pTransDOP, const CFMeshInst *pMeshInst, const FkDOP_Tree_t *pCollTree, const CFMtx43A *pMeshMtx = NULL, const CFMtx43A *pWorldToBone = NULL );


extern BOOL fmesh_coll_ModuleStartup( void );
extern void fmesh_coll_ModuleShutdown( void );

#endif