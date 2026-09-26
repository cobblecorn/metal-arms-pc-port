//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_Coll.h - Classes used to create collision data for Fang
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
// 03/17/02 Lafleur		Created (as GCMeshColl).
// 08/15/02 Lafleur		Adapted to be platform neutral.  Changed name to MLMeshColl
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMESH_COLL_H_
#define __MLMESH_COLL_H_

#define MLMESH_MAX_KDOP_VERTS				32
#define MLMESH_MAX_TRIS_PER_STATIC_LEAF		16	// Maximum number of triangles per leaf for static models (volume geo)
#define MLMESH_MAX_TRIS_PER_DYNAMIC_LEAF	24	// Maximum number of triangles per leaf for dynamic models


#if FANG_DEBUG_BUILD
#define MLMESH_PROFILE_TREE		FALSE
#endif

//
//
//
struct CollTri
{
	CFVec3	avPos[3];

	u16		anPosIdx[3];
	u16		nCollType;
	u16		nCollMask;

//	MLMaterial *pMaterial;

	u32		nVBIdx;
	s32		nTriPacket;

	CFVec3	vFaceNormal;
	CFVec3	vCentroid;

	f32		afCentroidIntervals[FCONVHULL_MAX_PLANES/2];
	f32		afIntervals[3][FCONVHULL_MAX_PLANES/2];

	CollTri *pNext;
};


//
//
//
struct CollNode
{
#if MLMESH_PROFILE_TREE
	f32		fBoxVolume;
	f32		fPercentOfParent;
	CFVec3	vMin;
	CFVec3	vMax;
#endif
	u32		nTreeDepth;
	u16		nDOPVertCount;
	CFVec3A	vDOPVerts[MLMESH_MAX_KDOP_VERTS];
	u16		nStartkDOPInterval;
	u16		nStartChildIdx;
	u16		nTriCount;
	CollTri	*pTriList;
};


#endif