//////////////////////////////////////////////////////////////////////////////////////
// MLSegment.h - Classes used to convert generic mesh data into Fang platform specific data
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
// 03/17/02 Lafleur		Created.
// 08/15/02 Lafleur		Adapted from GCSegment.h to support multi-platform
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLSEGMENT_H_
#define __MLSEGMENT_H_

#include <math.h>
#include <mmsystem.h>
#include "fang.h"
#include "fmesh_coll.h"
#include "gc\fGCmesh.h"
#include "dx\fdx8vb.h"
#include "Dolphin\GX\GXEnum.h"
#include "ML.h"
#include "MLMaterial.h"



#define REORDER_STRIPS_AND_LISTS			FALSE

#define MIN_FRAC_COMPONENT					5
#define OPTIMIZE_MEM_FOR_CONSTANT_COLOR		FALSE //TRUE

#define TC_VERT_PADDING						128



class MLMesh;
class MLTriContainer;
struct MLTriPacket;

extern BOOL MLMesh_bVolumeMesh;

#define MLSEGMENT_MAX_MATERIALS_PER_SEGMENT		512


//
//
//
class MLSegment
{
	public:
		FMeshSeg_t		m_FMeshSeg;

		u32				m_nTotalVerts;
		u32				m_nMtxWeightCount;

		s32				m_nPosType;
		s32				m_nPosFrac;

		MLTriContainer	*m_pFirstTriContainer;
		MLTriContainer	*m_pLastTriContainer;
		u32				m_nTriContainerCount;

		s32				m_nMatrixIdx;
		u32				m_nSegmentIdx;		// Index into the mesh's segment array for this segment

		MLSegment		*m_pNext;

		MLMaterial		*m_pMaterial[MLSEGMENT_MAX_MATERIALS_PER_SEGMENT];
		u16				m_nMaterialCount;

	public:
		MLSegment( KongSeg_t *pKongSeg );
		MLSegment::~MLSegment( void );

		BOOL AddVerts( MLTriPacket *pPacket );
		BOOL AddMaterialTris( KongMesh_t *pKongMesh, KongMat_t *pKongMat, MLMaterial *pMLMat, CFVec3 *pDirLightNorm );
		BOOL SetBoundingSphere( CFSphere *pBoundingSphere );
		void ColorVerts( u8 nRed, u8 nGreen, u8 nBlue, u8 nAlpha );
		void ProcessVerts( FMeshBone_t *pBoneArray );
		void Quantize( u32 nBias, u32 nUsedBoneCount );
		MLTriContainer* AllocateVertContainer( void );
};



//
// Stucture used to submit a packet of triangles to the MLSegment
//
struct MLTriPacket
{
	KongVert_t	**apKongVerts;
	MLMaterial	*pMaterial;			// MLMaterial pointer
	u16			nVertCount;
	u8			nBaseSTSets;		// Number of valid base ST's in the verts
	u8			nLightMapSTSets;	// Number of valid lightmap ST's in the verts
	u16			nCollMask;			// Collision mask (see FMesh_Coll.h)
	u16			nCollType;			// Collision type field (see FMesh_Coll.h)
	BOOL		bInBoneSpace;		// Indicates that the verts are provided in bone space coordinates
	BOOL		bConvertToBackface;	// Indicates that the verts should be converted to be drawn as backface polys
	BOOL		bUseSpecular;		// Renderer will need to generate specular color
};


//
//
FINLINE u32 GetBits( u32 nGCType )
{
	if ( nGCType == GX_S8 )
	{
		return 8;
	}
	else if ( nGCType == GX_S16 )
	{
		return 16;
	}
	else if ( nGCType == GX_F32 )
	{
		return 32;
	}

	FASSERT_NOW;

	return 0;
}

//
//
FINLINE u32 GetGCType( u32 nBits, u32 nSign )
{
	if ( nBits == 8 && nSign )
	{
		return GX_S8;
	}

	else if ( nBits == 16 && nSign )
	{
		return GX_S16;
	}

	else if ( nBits == 32 )
	{
		return GX_F32;
	}

	FASSERT_NOW;

	return 0;
}

//
//
FINLINE u32 BitsToRepresent( u32 nValue )
{
	u32 nBits = 0;
	while ( nValue )
	{
		nBits++;
		nValue >>= 1;
	}

	return nBits;
}


//
//
//
class MLTriContainer
{
	public:
		void			*m_pVertAbstr;

		u16				m_nArraySize;
		u16				m_nVertCount;
		BOOL			m_bInBoneSpace;
		u32				m_nVBKey;
		s16				m_nHashTableIdx;
		u16				m_nMtxWeightCount;

		u16				m_nPosFrac;
		u16				m_nPosType;

		u16				m_nLODID;
		u16				m_nPartID;
		u8				m_nBaseSTSets;
		u8				m_nLightMapSTSets;

		u16				m_nCollisionMask;
		u16				m_nCollisionType;

		u16				__PAD;

		u32				m_nSTFrac[FGCVB_MAX_ST_SETS];

		BOOL			m_bBumpMapped;

		BOOL			m_bFacingDirectionalLight;	// FALSE if we are processing a world and these triangles face away from the directional light

		BOOL			m_bConstantColor;
		CFColorRGBA		m_ConstantColor;

		CFVec3			m_vAveragePosition;

		MLMaterial		*m_pMaterial;
		s32				m_nVBIndex;
		s32				m_nMatrixIdx;
		s32				m_nSegmentIdx;

		KongVert_t		*m_paKongVerts;

		MLTriContainer	*m_pNext;

		BOOL			m_bInAttributeArray;

	public:
		MLTriContainer( void )
		{
			m_pNext = NULL;
			m_pVertAbstr = NULL;
			m_nArraySize = 0;
			m_nVertCount = 0;
			m_paKongVerts = NULL;
			m_pMaterial = NULL;
			m_nVBIndex = -1;
			m_nMatrixIdx = -1;
			m_nSegmentIdx = -1;
			m_nHashTableIdx = -1;

			m_nCollisionMask = FCOLL_MASK_CHECK_ALL;
			m_nCollisionType = 0;

			m_nVBKey = 0;
			m_nMtxWeightCount = 0;
			m_nPosFrac = 0;
			m_nPosType = 0;
			m_nBaseSTSets = 0;
			m_nLightMapSTSets = 0;

			m_nLODID = 0;
			m_nPartID = 0;

			m_bBumpMapped = FALSE;

			m_bFacingDirectionalLight = TRUE;

			m_bInBoneSpace = FALSE;
			m_bConstantColor = FALSE;
			m_bInAttributeArray = FALSE;

			for ( u32 i = 0; i < FGCVB_MAX_ST_SETS; i++ )
			{
				m_nSTFrac[i] = 8;
			}
		}

		~MLTriContainer( void )
		{
			if ( m_pVertAbstr )
			{
				fang_Free( m_pVertAbstr );
			}
			m_pVertAbstr = NULL;

			FreeVerts();
		}

		//
		//
		void FreeVerts( void )
		{
			if ( m_paKongVerts )
			{
				delete m_paKongVerts;
			}
			m_paKongVerts = NULL;
		}

		BOOL ProcessVert( KongVert_t *pVert, u8 *anSegmentBoneIdx, u16 nBoneCount, u8 nBaseUVCount, u8 nLightMapUVCount );
		BOOL Create( KongMesh_t *pKongMesh, KongMat_t *pKongMat, MLMaterial *pMLMat, u32 nMtxWeightCount, 
					u8 *anSegmentBoneIdx, u16 nBoneCount, u32 nRelevantTriCount, CFVec3 *pDirLightNorm );
		BOOL AddVerts( KongMesh_t *pKongMesh, KongMat_t *pKongMat, MLMaterial *pMLMat, u8 *anSegmentBoneIdx, 
					u16 nBoneCount, u32 nRelevantTriCount, CFVec3 *pDirLightNorm  );
		u32  CalculateVBKey( TargetPlatform_e nPlatform );
		void Quantize( u32 nBias );

		//
		//
		void ColorVerts( u8 nRed, u8 nGreen, u8 nBlue, u8 nAlpha )
		{
			for ( u32 i = 0; i < m_nVertCount; i++ )
			{
				m_paKongVerts[i].Color.fRed   = (f32)nRed * (1/255.f);
				m_paKongVerts[i].Color.fGreen = (f32)nGreen * (1/255.f);
				m_paKongVerts[i].Color.fBlue  = (f32)nBlue * (1/255.f);
				m_paKongVerts[i].Color.fAlpha = (f32)nAlpha * (1/255.f);
			}
		}

		//
		//
		void CalculateAveragePosition( void )
		{
			m_vAveragePosition.Zero();
			for ( u32 i = 0; i < m_nVertCount; i++ )
			{
				m_vAveragePosition += m_paKongVerts[i].Pos;
			}

			if ( m_nVertCount )
			{
				m_vAveragePosition /= (f32)m_nVertCount;
			}
		}

		//
		//
		void TransformVerts( CFMtx43A *pTransformMtx, CFMtx43A *pNormalTransformMtx )
		{
			FASSERT( pTransformMtx && pNormalTransformMtx );

			// If the verts are already in bone space, bail out
			if ( m_bInBoneSpace )
			{
				return;
			}

			for ( u32 i = 0; i < m_nVertCount; i++ )
			{
				m_paKongVerts[i].Pos = pTransformMtx->m44.MultPoint( m_paKongVerts[i].Pos );
				m_paKongVerts[i].Norm = pNormalTransformMtx->m44.MultDir( m_paKongVerts[i].Norm );
				m_paKongVerts[i].Norm.Unitize();
			}

			m_bInBoneSpace = TRUE;
		}

};



#endif