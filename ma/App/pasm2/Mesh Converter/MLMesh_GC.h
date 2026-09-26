//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_GC.h - Classes used to convert generic mesh data into Fang GC specific data
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
// 03/17/02 Lafleur		Created as GCMesh.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMESH_GC_H_
#define __MLMESH_GC_H_

#include "fang.h"
#include "gc\fGCmesh.h"

#include "ML.h"
#include "MLMesh.h"
#include "MLMaterial_GC.h"


// This structure is used as an abstraction layer between the
// indices to be rendered by a strip and the actual indices into the 
// respective vertex buffer attribute arrays.  Given a particular 
// vert index, we can get attribute indices for that vert.
struct GCVertAbstr_t
{
	u16	nPosIdx;
	u16	nNormIdx;
	u16	nBinormIdx;
	u16	nTangIdx;
	u16	nDiffuseIdx;
	u16	nSpecularIdx;
	u16 nSTIdx[FGCVB_MAX_ST_SETS];
};


#define MLMESH_MAX_GC_ATTRIBUTE_INDICES			32768
#define MLMESH_MAX_GC_TRANS_DESC				(MLMESH_MAX_GC_ATTRIBUTE_INDICES >> 2)

#define MLMESHGC_USE_NORMAL_SPHERE_FOR_STREAMING_NBTS	TRUE


//
//
//
class MLMesh_GC : public MLMesh
{
	protected:
		FGCMesh_t		m_FGCMesh;

		u32				m_nIndexedVertCount;
		u8				*m_pDLContainerBuffer;
		u8				*m_pDLDataBuffer;
		u32				m_nDLDataSize;

		FGCVB_t			*m_paFGCVB;
	protected:
		// Protected Methods

		// GCMesh construction
		MLResults* GenerateExportData( BOOL bGenerateCollisionData );
		u32  CreateVertexBuffers( u32 *pVBCount );
		u32  CreateVertexAttributeArrays( void );
		BOOL StoreTriContainerSkinned( u32 nMatrixCountLimit, u32 *pProcessedCounter );
		BOOL StoreTriContainer( MLTriContainer *pTriCont );
		BOOL ResolveTriContainerNormals( MLTriContainer *pTriCont );
		u32  CreateDLContainers( u32 nDLCount );
		BOOL AddSkinnedApeVert( MLTriContainer *pTriCont, u32 i, f32 *pWeights, u32 nWeights );
		u32  ResolveMaterials( u32 *pDLCount = NULL );

		//
		//
		void FreeScratchMemory( void )
		{
			if ( m_pDLContainerBuffer )
			{
				fang_Free( m_pDLContainerBuffer );
				m_pDLContainerBuffer = NULL;
			}

			if ( m_pDLDataBuffer )
			{
				fang_Free( m_pDLDataBuffer );
				m_pDLDataBuffer = NULL;
			}

			if ( m_paFGCVB )
			{
				free( m_paFGCVB );
				m_paFGCVB = NULL;
			}

			MLMesh::FreeScratchMemory();
		}

		//
		//
		BOOL Next3VertsAreUnique( MLTriContainer *pVC, u32 iStartIdx ) 
		{ 
			GCVertAbstr_t *pAbstr = &((GCVertAbstr_t *)pVC->m_pVertAbstr)[iStartIdx];
			if (   pAbstr[0].nPosIdx == pAbstr[1].nPosIdx
				|| pAbstr[0].nPosIdx == pAbstr[2].nPosIdx
				|| pAbstr[2].nPosIdx == pAbstr[1].nPosIdx )
			{
				return FALSE; 
			}

			return TRUE;
		}

		//
		//
		u16 GetAbstrPosIdx( MLTriContainer *pVC, u32 iAbstrIdx ) 
		{
			return ((GCVertAbstr_t *)pVC->m_pVertAbstr)[iAbstrIdx].nPosIdx;
		}

	public:
		//
		//
		MLMesh_GC( char *pszName, BOOL bWorldGeo ) : MLMesh( pszName, bWorldGeo )
		{
			m_FGCMesh.pMesh = NULL;
			m_FGCMesh.nFlags = 0;
			m_FGCMesh.AtRestBoundSphere_MS.m_Pos.x = 0.f;
			m_FGCMesh.AtRestBoundSphere_MS.m_Pos.y = 0.f;
			m_FGCMesh.AtRestBoundSphere_MS.m_Pos.z = 0.f;
			m_FGCMesh.AtRestBoundSphere_MS.m_fRadius = 0.f;
			m_FGCMesh.nVBCount = 0;
			m_FGCMesh.aVB = NULL;

			m_paFGCVB = NULL;

			m_nIndexedVertCount = 0;
			m_pDLContainerBuffer = NULL;
			m_pDLDataBuffer = NULL;
			m_nDLDataSize = 0;
		}

		//
		//
		~MLMesh_GC( void ) {}

		//
		// Sets the at rest bounding sphere for the mesh
		BOOL SetBoundingSphere( CFSphere *pBoundingSphere )
		{
			FASSERT( pBoundingSphere );
			m_FGCMesh.AtRestBoundSphere_MS = *pBoundingSphere;
			m_FMesh.BoundSphere_MS = *pBoundingSphere;
			return TRUE;
		}

		//
		//
		u16 AllocateMLMaterial( KongMat_t *pKMat )
		{
			MLMaterial_GC *pNewMaterial = new MLMaterial_GC;
			if ( !pNewMaterial )
			{
				return 0xffff;
			}

			u16 nIndex = 0;

			// Add the new material to the mesh and return it
			if ( !AddMaterial( pNewMaterial, pKMat, nIndex ) )
			{
				delete pNewMaterial;
				return nIndex;
			}

			pNewMaterial->Set( this, pKMat );

			return nIndex;
		}
};	


#endif
