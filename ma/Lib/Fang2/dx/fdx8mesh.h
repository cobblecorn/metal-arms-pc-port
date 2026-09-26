//////////////////////////////////////////////////////////////////////////////////////
// fdx8mesh.h - Fang mesh module (DX8 version).
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 09/13/00 Ranck       Created.
// 08/15/02	Lafleur		Adapted for PASM created mesh and kDOP collision data
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8MESH_H_
#define _FDX8MESH_H_ 1

#include "fang.h"
#include "fdx8.h"
#include "fdata.h"
#include "fmesh.h"


struct FDX8VB_t;


//
// Single Strip Definition:
//
struct FDX8MeshStrip_t
{
	u8  nTriCount;						// Number of triangles in this strip
	u8  __PAD;							// (for alignment)
	u16 nStartVindex;					// Starting vindex into DX Index Buffer
	u16 nVtxIndexMin;					// Minimum vertex index referenced by this strip
	u16 nVtxIndexRange;					// (Maximum vertex index referenced by this strip + 1) - nVtxIndexMin
};


//
//
//
struct FDX8MeshTriList_t
{
	u16 nTriCount;						// Number of single triangles
	u16 nStartVindex;					// Starting vindex into DX Index Buffer
	u16 nVtxIndexMin;					// Minimum vertex index referenced by this triangle list
	u16 nVtxIndexRange;					// (Maximum vertex index referenced by this triangle list + 1) - nVtxIndexMin
};

//
//
enum
{
	CLUSTER_FACING_OPP_DIR_LIGHT = 0x01,	// The polys in this cluster are facing away from the directional light
};

//
//
struct FDX8MeshCluster_t
{
	u16 nStripCount;
	u8	nFlags;
	u8	nSegmentIdx;
	u8  nVBIndex;
	u8  nIBIndex;
	u8  nPartID;
	u8  nLODID;

	void *pPushBuffer;					// Push buffer for this cluster (if non-NULL)
	FDX8MeshTriList_t TriList;			// List of triangles
	FDX8MeshStrip_t *paStripBuffer;		// Pointer to the strips for the cluster
};


//
//
struct FDX8MeshMaterial_t
{
	FDX8MeshCluster_t *aCluster;
	u32 nClusterCount;
};




//-------------------------------------------------------------------------------------------------------------------
// General Mesh Definitions:
//-------------------------------------------------------------------------------------------------------------------


typedef struct FDX8Mesh_s
{
	u16			nFlags;						// See FDX8MESH_FLAG_* for info
	u8			nVBCount;					// Number of vertex buffers used by this mesh
	u8			nIBCount;					// Number of index buffers used by this mesh
	u32			nDisposableOffset;			// The address offset for the temporary portion of the file when loaded (this portion is converted to DX resources).

	CFSphere	AtRestBoundSphere_MS;		// Used only when nSegCount is 0

	FMesh_t		*pMesh;						// Pointer to platform-independent base object

	FDX8VB_t	*aVB;						// Array of vertex buffer descriptors
	CFVec3		**apCollVertBuffer;			// Array of Collision vertex buffers
	u16			*anIndicesCount;			// Number of indices used by this mesh in each IB
	void		**apDXIB;					// Pointer to an array of index buffers (array of u16s)

#if FANG_WINGC
	u32			nGCDiffuseCount;			// Colors a GameCube color stream must hold (VB 0's diffuse count, as the GameCube checked)
#endif
} FDX8Mesh_t;



extern BOOL fdx8mesh_ModuleStartup( void );
extern void fdx8mesh_ModuleShutdown( void );

extern FMeshCullDir_e fmesh_GetCullDir( void );
extern void fmesh_FlipCullDir( BOOL bFlip );

//===================================================================================================================


#endif

