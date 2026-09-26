//////////////////////////////////////////////////////////////////////////////////////
// FkDOP.h - Fang kDOP utility functions
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
// 05/08/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FKDOP_H_
#define _FKDOP_H_ 1

#include "fang.h"
#include "fmath.h"


class CFColorRGBA;
struct FMesh_t;

//
enum
{
	FkDOP_6_DOP = 0,
	FkDOP_14_DOP,
	FkDOP_18_DOP,
	FkDOP_26_DOP,
	FkDOP_MAX_kDOPS
};

//
enum
{
	FkDOP_6_DOP_AXES = 3,
	FkDOP_14_DOP_AXES = 7,
	FkDOP_18_DOP_AXES = 9,
	FkDOP_26_DOP_AXES = 13
};


#define FKDOP_DEFAULT_COLL_DOP			FkDOP_14_DOP
#define FKDOP_DEFAULT_PHYS_DOP			FkDOP_14_DOP
#define FKDOP_MAX_AXES					FkDOP_14_DOP_AXES
#define FKDOP_MAX_EDGE_NORMALS			9
#define FKDOP_MAX_VERTS					((FKDOP_MAX_AXES - 1) * 4)
#define FCONVHULL_MAX_PLANES			FKDOP_MAX_AXES * 2
#define FCONVHULL_MAX_PLANE_POINTS		16
#define FCONVHULL_MAX_HULL_POINTS		(FCONVHULL_MAX_PLANES * FCONVHULL_MAX_PLANE_POINTS)


// Descriptor Structure for the kDOPs
struct FkDOP_DOPDescriptor_t
{
	u8		nSideCount;
	u8		nNormalCount;
	u8		nEdgeNormalCount;
	u8		__PAD;
//ARG	CFVec3A	avNormals[FKDOP_MAX_AXES];
//ARG	CFVec3A	avEdgeNormals[FKDOP_MAX_EDGE_NORMALS];
//ARG - >>>>>
//ARG - BUG not allocating enough space ???
	CFVec3A	avNormals[FkDOP_26_DOP_AXES];
	CFVec3A	avEdgeNormals[FkDOP_26_DOP_AXES];
//ARG - <<<<<
	
	FkDOP_DOPDescriptor_t( u8 nSides, u8 nNormals, u8 nEdgeNormals,
	const CFVec3A &v0,
	const CFVec3A &v1,
	const CFVec3A &v2,
	const CFVec3A &v3,
	const CFVec3A &v4,
	const CFVec3A &v5,
	const CFVec3A &v6,
	const CFVec3A &v7,
	const CFVec3A &v8,
	const CFVec3A &v9,
	const CFVec3A &v10,
	const CFVec3A &v11,
	const CFVec3A &v12,
	const CFVec3A &ve0,
	const CFVec3A &ve1,
	const CFVec3A &ve2,
	const CFVec3A &ve3,
	const CFVec3A &ve4,
	const CFVec3A &ve5,
	const CFVec3A &ve6,
	const CFVec3A &ve7,
	const CFVec3A &ve8,
	const CFVec3A &ve9,
	const CFVec3A &ve10,
	const CFVec3A &ve11,
	const CFVec3A &ve12
	 )
	{
		nSideCount = nSides;
		nNormalCount = nNormals;
		nEdgeNormalCount = nEdgeNormals;
		avNormals[0].Set(v0);
		avNormals[1].Set(v1);
		avNormals[2].Set(v2);
		avNormals[3].Set(v3);
		avNormals[4].Set(v4);
		avNormals[5].Set(v5);
		avNormals[6].Set(v6);
		avNormals[7].Set(v7);
		avNormals[8].Set(v8);
		avNormals[9].Set(v9);
		avNormals[10].Set(v10);
		avNormals[11].Set(v11);
		avNormals[12].Set(v12);
		avEdgeNormals[0].Set(ve0);
		avEdgeNormals[1].Set(ve1);
		avEdgeNormals[2].Set(ve2);
		avEdgeNormals[3].Set(ve3);
		avEdgeNormals[4].Set(ve4);
		avEdgeNormals[5].Set(ve5);
		avEdgeNormals[6].Set(ve6);
		avEdgeNormals[7].Set(ve7);
		avEdgeNormals[8].Set(ve8);
		avEdgeNormals[9].Set(ve9);
		avEdgeNormals[10].Set(ve10);
		avEdgeNormals[11].Set(ve11);
		avEdgeNormals[12].Set(ve12);
	}
	
};


//
// FkDOP_Interval_t - Establishes the min and max values for a kDOP along a given axis
//
struct FkDOP_Interval_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  This structure is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	f32	fMin;
	f32	fMax;
	
	void ChangeEndian( void )
	{
		fMin = fang_ConvertEndian( fMin );
		fMax = fang_ConvertEndian( fMax );
	}
};

//
// This structure is mainly used to create plane descriptions that can
// be used to visualize (render) the kDOP geometry
//
struct FConvHull_Plane_t
{
	CFVec3A vNormal;
	u16	nPointIdx[FCONVHULL_MAX_PLANE_POINTS];
	u32 nPointCount;
	
	BOOL AddIndex( u32 nNew )
	{
		u32 i;
		for ( i = 0; i < nPointCount; i++ )
		{
			if ( nPointIdx[i] == nNew )
			{
				return FALSE;
			}
		}
		
		nPointIdx[nPointCount++] = (u16)nNew;
		
		return TRUE;
	}
};


// Array of kDOP descriptors for the different kDOPs supported in the engine
extern const FkDOP_DOPDescriptor_t FkDOP_aDesc[FkDOP_MAX_kDOPS];

// Collision helper functions
extern BOOL fkdop_PointIsInkDOP( const CFVec3A *pTestPoint, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );
extern BOOL fkdop_IntersectkDOPAndkDOP( const FkDOP_Interval_t *pIntervals1, u32 nkDOPType1, const FkDOP_Interval_t *pIntervals2, u32 nkDOPType2 );
extern BOOL fkdop_IntersectkDOPAndkDOP( const FkDOP_Interval_t *pIntervals1, u32 nkDOPType1, const FkDOP_Interval_t *pIntervals2, u32 nkDOPType2, CFVec3A *pkDOP2Movement );
extern f32  fkdop_IntersectkDOPAndRay( const CFVec3A *pStart, const CFVec3A *pEnd, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );
extern BOOL fkdop_IntersectkDOPAndSphere( const CFVec3A *pCenter, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );
extern BOOL fkdop_IntersectkDOPAndCapsule( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );
//extern BOOL fkdop_IntersectkDOPAndTriangle( const CFVec3A *avTri, const CFVec3A *pTriNormal, const FkDOP_Tree_t *pTree, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );
//extern f32 fkdop_IntersectkDOPAndEdge( f32 *afStartDots, f32 *afEndDots, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );

// Collision helper functions where the intervals for the submitted primitive are pre-calculated
extern f32  fkdop_IntersectkDOPAndRay( f32 *pafStartIntervals, f32 *pafEndIntervals, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );
extern BOOL fkdop_IntersectkDOPAndSphere( f32 *pafCenterIntervals, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );
extern BOOL fkdop_IntersectkDOPAndCapsule( f32 *pafStart, f32 *pafEnd, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );

// Given a set of points, this function will generate the intervals along the
// active kDOP's axis for the geometry.  It will place those intervals in a static
// buffer in the function or it will place it in *pInt, if specified.  FkDOP_Interval_t 
// *pInt MUST point to an array of FkDOP_Interval_t's at least 13 in size!!
extern void fkdop_GenerateIntervals( CFVec3A *pVerts, u32 nVertCount, FkDOP_Interval_t *pInt, u32 nkDOPType );


#if !FANG_PRODUCTION_BUILD
	extern FConvHull_Plane_t FConvHull_PlaneBuffer[FCONVHULL_MAX_PLANES];
	extern CFVec3A FConvHull_vPointBuffer[FCONVHULL_MAX_HULL_POINTS];
	extern u32 FConvHull_nPointBufferCount;

	// This function fills in FkDOP_vPointBuffer[] with the points that bound a kDOP
	extern CFVec3A* fkdop_CalculateVerts( const FkDOP_Interval_t *pIntervals, BOOL bGenPlanes, u32 nkDOPType );
	extern void FkDOP_DrawkDOP( const FkDOP_Interval_t *pInt, const CFMtx43A *pTransMtx, u32 nkDOPType, BOOL bTranslateOnly = FALSE, CFColorRGBA *pColor = NULL );
#endif // !FANG_PRODUCTION_BUILD


//
//
//
FINLINE void fkdop_GetkDOPNormal( u32 nNormalIdx, CFVec3A *pResult, u32 nkDOPType )
{
	FASSERT( pResult );
	FASSERT( nNormalIdx >= 0 && nNormalIdx < (u32)(2*FkDOP_aDesc[nkDOPType].nNormalCount) );

	if ( nNormalIdx < FkDOP_aDesc[nkDOPType].nNormalCount )
	{
		pResult->Set( FkDOP_aDesc[nkDOPType].avNormals[nNormalIdx] );
	}
	else
	{
		nNormalIdx -= FkDOP_aDesc[nkDOPType].nNormalCount; 
		pResult->Set( FkDOP_aDesc[nkDOPType].avNormals[nNormalIdx] );
		pResult->Negate();
	}
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//	Structs and Functions for getting geo that represents the volume contained by the kDOP.
//	This can be used for debugging collision routines or for special effects.
//
#define FKDOP_MAX_DOP_FACE_VERTS	17

enum
{
	FKDOP_FACE_FLAGS_NONE = 0,
	FKDOP_FACE_FLAGS_INUSE = 0x00000001,
	FKDOP_FACE_FLAGS_INVERT_NORMAL = 0x00000002,
};

struct FkDOP_Face_t
{
	u8 nFlags;
	u8 nFaceNormalIdx;						// Index into the kDOP normal array indicating the face normal
	u8 nVertCount;							// Number of verts that make up this face
	u8 anVertIdx[FKDOP_MAX_DOP_FACE_VERTS]; // Indices into 
};

enum
{
	FKDOP_GEO_FLAGS_NONE = 0,
	FKDOP_GEO_FLAGS_INUSE = 0x00000001,
};

//
struct FkDOP_Geo_t
{
	u8 				nFlags;			// See flags, above
	u8 				nFaceCount;		// number of faces in the aFaces array
	u8 				nVertCount;		// number of verts in the aVerts array
	u8 				nBoneIdx;		// Index into the FMesh_t bone array
	FMesh_t 		*pMesh;			// Pointer to the mesh that uses this kDOP
	FkDOP_Face_t 	*aFaces;		// An array of the faces that make up the geo
	CFVec3 			*aVerts;		// An array of the verts that make up the geo
};


#endif // ndef _FKDOP_H_