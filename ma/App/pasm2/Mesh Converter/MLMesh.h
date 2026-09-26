//////////////////////////////////////////////////////////////////////////////////////
// MLMesh.h - Classes used to convert generic mesh data into Fang platform specific data
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
// 08/15/02 Lafleur		Adapted for multiplatform support.  Changed name to MLMesh.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMESH_H_
#define __MLMESH_H_

#include "fang.h"
#include "ML.h"
#include "MLTexLayerID.h"
#include "MLSegment.h"
#include "MLMaterial.h"
#include "MLMesh_Coll.h"
#include "MLMesh_Light.h"
#include "fmesh_coll.h"
#include "fkdop.h"

//
//	Utility macros for rounding up addresses
#define RoundUp4B(x)		(((u32)(x) + 4 - 1) & ~(4 - 1))
#define RoundUp8B(x)		(((u32)(x) + 8 - 1) & ~(8 - 1))
#define RoundUp16B(x)		(((u32)(x) + 16 - 1) & ~(16 - 1))
#define RoundUp32B(x)		(((u32)(x) + 32 - 1) & ~(32 - 1))


#define MLMESH_MAX_GC_ATTRIBUTE_INDICES			32768
#define MLMESH_MAX_GC_TRANS_DESC				(MLMESH_MAX_GC_ATTRIBUTE_INDICES >> 2)


//
// There are four possible ways to choose the axis to divide the polys on.  For
// now we are only going to deal with the x, y or z axis, though you could choose
// to divide along any of the kDOP axes:
//
//	Min Sum: Choose the axis that minimizes the sum of the volumes of the
//		two resulting children. (Probably the most effective at engine time, 
//		but the slowest to calculate at tool time.)
//
//	Min Max: Choose the axis that minimizes the larger of the volumes of
//		the two resulting children. (Not as effective as Min Sum and slow)
//
//	Splatter: Project the centroids of the triangles onto each of the three
//		coordinate axis and calculate the variance of each of the resulting
//		distributions.  Choose the axis yielding the largest variance. (Good
//		balance between speed at tool time and efficiency at engine time.)
//
//	Longest Side: Choose the axis along which the kDOP is longest. (Very quick
//		at tool time, least efficient in engine)
//
//	So the best choices are probably Min Sum and Splatter
enum 
{
	MLMESH_TREETYPE_MIN_SUM = 0,
	MLMESH_TREETYPE_MIN_MAX,
	MLMESH_TREETYPE_SPLATTER,
	MLMESH_TREETYPE_LONG_SIDE,
	MLMESH_TREETYPE_MAX
};



// Flag that indicates to the lib that the current mesh being processed
// is a volume mesh and thus certain optimizations can be applied.
extern BOOL MLMesh_bVolumeMesh;

// Conversion counters for collision data
extern u32	MLMesh_nTotalTreeCount;
extern u32	MLMesh_nTotalNodeCount;
extern u32	MLMesh_nTotalIntervalCount;
extern u32	MLMesh_nTotalTriCount;
extern u32	MLMesh_nTotalTriPackets;
extern u32	MLMesh_nTotalTriDataBytes;
extern u32	MLMesh_nTotalRootDOPVertBytes;
extern u32  MLMesh_nTotalkDOPFaceCount;


//
//
//
class MLMesh
{
	friend class CMLManager;

	protected:
		FMesh_t			m_FMesh;

		// Linked list of mesh properties
		MLTexLayerID	*m_pFirstTexLayerID;
		MLMeshLight		*m_pFirstLight;
		MLSegment		*m_pFirstSegment;
		u32				m_nMaterialCount;
		MLMaterial		*m_pFirstMaterial;

		FMeshBone_t		*m_pBoneArray;
		u8				*m_pSkeletonArray;
		u32				m_nSkeletonArraySize;	
		
		u32				m_nCompressionBias;

		FkDOP_Tree_t	*m_paCollTree;
		u32				m_nCollTreeCount;

		u16				m_nTreeCalcType;
		u16				m_nCollkDOPType;

		u16				m_nCollisionLOD;

		MLMaterial		*m_apMaterialsAlloced[512];
		u32				m_nMaterialsAllocedCount;

		char			m_szVertRemapFilename[32];
		u32				m_nRemapCount;
		KongVertRemap_t *m_paVertRemaps;

		SYSTEMTIME		APELastWriteTime;

		// Used for establishing linklists in the ML manager
		MLMesh			*m_pNext;

		MLResults		m_Results;

	protected:
		// Private Methods

		virtual u32 ResolveMaterials( u32 *pDLCount = NULL );

		// Export data construction
		virtual MLResults* GenerateExportData( BOOL bGenerateCollisionData ) = 0;

		// Collision Data construction
		virtual u32  CreateMeshCollData( u32 nCollkDOPType );
		virtual void GenerateNode( CollNode *pNode, u32 nCollkDOPType );
		
		// Test to determine whether 3 vert abstractions have the same data
		virtual BOOL Next3VertsAreUnique( MLTriContainer *pVC, u32 iStartIdx ) = 0;

		virtual u16  GetAbstrPosIdx( MLTriContainer *pVC, u32 iStartIdx ) = 0;

		void GenerateBoundingCapsule( CollNode *pNode, FMesh_Coll_Capsule_t *pCapsule );

		u32 CalculateMeshLODSwitches( void );

		//
		//
		MLMaterial* GetMaterial( u32 nIndex )
		{
			if ( !m_pFirstMaterial )
			{
				return NULL;
			}

			if ( nIndex >= m_nMaterialCount )
			{
				return NULL;
			}

			u32 nCount = 0;
			MLMaterial *pMat = m_pFirstMaterial;
			while ( pMat )
			{
				if ( nCount == nIndex )
				{
					return pMat;
				}

				pMat = pMat->m_pNext;
				nCount++;
			}

			return NULL;
		}

		//
		//
		void FreeScratchMemory( void )
		{
			u32 i, ii;

			if ( m_paCollTree )
			{
				for ( i = 0; i < m_nCollTreeCount; i++ )
				{
					FkDOP_Tree_t *pColl = &m_paCollTree[i];

					for ( ii = 0; ii < pColl->nTreeNodeCount; ii++ )
					{
						FkDOP_Node_t *pNode = &pColl->pakDOPNodes[ii];

						if ( pNode->paPackets )
						{
							free( pNode->paPackets );
							pNode->paPackets = NULL;
						}
						if ( pNode->pTriData )
						{
							free( pNode->pTriData );
							pNode->pTriData = NULL;
						}
					}

					if ( pColl->pakDOPNodes )
					{
						free( pColl->pakDOPNodes );
						pColl->pakDOPNodes = NULL;
					}

					if ( pColl->paRootkDOPVerts )
					{
						free( pColl->paRootkDOPVerts );
						pColl->paRootkDOPVerts = NULL;
					}
/*
					if ( pColl->pakDOPVerts )
					{
						free( pColl->pakDOPVerts );
						pColl->pakDOPVerts = NULL;
					}
*/
					if ( pColl->paIntervals )
					{
						free( pColl->paIntervals );
						pColl->paIntervals = NULL;
					}
				}

				free( m_paCollTree );
				m_paCollTree = NULL;
				m_nCollTreeCount = 0;
			}

		}

		//
		//
		BOOL AddSegment( MLSegment *pNewSegment )
		{
			FASSERT( pNewSegment );

			pNewSegment->m_nSegmentIdx = m_FMesh.nSegCount;
			m_FMesh.nSegCount++;

			// New segments should not have any next defined, yet
			FASSERT( !pNewSegment->m_pNext );

			if ( !m_pFirstSegment )
			{
				m_pFirstSegment = pNewSegment;
				return TRUE;
			}

			MLSegment *pLast = m_pFirstSegment;
			while ( pLast->m_pNext )
			{
				pLast = pLast->m_pNext;
			}

			pLast->m_pNext = pNewSegment;

			return TRUE;
		}

		//
		// Returns true if the MLMaterial is unique
		//
		BOOL AddMaterial( MLMaterial *pNewMaterial, KongMat_t *pKMat, u16 &nIndex )
		{
			FASSERT( pNewMaterial );
			FASSERT( m_nMaterialCount < 255 );

			// New materials should not have any next defined, yet
			FASSERT( !pNewMaterial->m_pNext );

			// Is this material a lower LOD than our current count?
			if ( pKMat->pProperties->nLODIndex + 1 > m_FMesh.nLODCount )
			{
				m_FMesh.nLODCount = pKMat->pProperties->nLODIndex + 1;
			}

			nIndex = 0;
			if ( !m_pFirstMaterial )
			{
				m_pFirstMaterial = pNewMaterial;
				m_nMaterialCount++;
				m_apMaterialsAlloced[m_nMaterialsAllocedCount++] = pNewMaterial;
				return TRUE;
			}

			MLMaterial *pTest = m_pFirstMaterial;
			MLMaterial *pLast = NULL;

			while ( pTest )
			{
				if ( pTest->m_pKongMat == pKMat || pTest->m_pKongMat->Equals( pKMat, TRUE ) )
				{
					return FALSE;
				}

				nIndex++;
				pLast = pTest;
				pTest = pTest->m_pNext;
			}

			FASSERT( pLast );

			pLast->m_pNext = pNewMaterial;

			m_nMaterialCount++;
			m_apMaterialsAlloced[m_nMaterialsAllocedCount++] = pNewMaterial;

			return TRUE;
		}

	public:
		//
		//
		MLMesh( char *pszName, BOOL bWorldGeo )
		{
			FASSERT( pszName );

			strcpy( m_FMesh.szName, pszName );
			m_FMesh.BoundSphere_MS.m_Pos.x = 0.f;
			m_FMesh.BoundSphere_MS.m_Pos.y = 0.f;
			m_FMesh.BoundSphere_MS.m_Pos.z = 0.f;
			m_FMesh.BoundSphere_MS.m_fRadius = 0.f;
			m_FMesh.nUsedBoneCount = 0;
			m_FMesh.nRootBoneIndex = 255;
			m_FMesh.nBoneCount = 0;
			m_FMesh.nSegCount = 0;
			m_FMesh.nTexLayerIDCount = 0;
			m_FMesh.nTexLayerIDCount_ST = 0;
			m_FMesh.nTexLayerIDCount_Flip = 0;
			m_FMesh.nLightCount = 0;
			m_FMesh.nFlags = 0;
			m_FMesh.aSeg = NULL;
			m_FMesh.pBoneArray = NULL;
			m_FMesh.pLightArray = NULL;
			m_FMesh.pnSkeletonIndexArray = NULL;
			m_FMesh.pTexLayerIDArray = NULL;
			m_FMesh.pMeshIS = NULL;
			m_FMesh.nMaterialCount = 0;
			m_FMesh.aMtl = NULL;
			m_FMesh.nShadowLODBias = 0;
			m_FMesh.nLODCount = 1;

			u32 i;
			m_FMesh.afLODDistance[0] = 0.f;
			for ( i = 1; i < FDATA_MAX_LOD_MESH_COUNT; i++ )
			{
				m_FMesh.afLODDistance[i] = -1.f;
			}
			m_nCollisionLOD = 0;

			m_nMaterialCount = 0;
			m_pFirstMaterial = NULL;

			m_paCollTree = NULL;
			m_nCollTreeCount = 0;

			m_nMaterialsAllocedCount = 0;

			m_nCompressionBias = 10;

			m_nTreeCalcType = MLMESH_TREETYPE_SPLATTER;
//			m_nTreeCalcType = MLMESH_TREETYPE_MIN_SUM;
			if ( bWorldGeo )
			{
				m_nCollkDOPType = FkDOP_6_DOP;
//				m_nCollkDOPType = FKDOP_DEFAULT_COLL_DOP;
			}
			else
			{
				m_nCollkDOPType = FKDOP_DEFAULT_COLL_DOP;
			}

			// Reset the global volume mesh flag
			MLMesh_bVolumeMesh = FALSE;

			m_pBoneArray = NULL;
			m_pSkeletonArray = NULL;
			m_nSkeletonArraySize = 0;
			m_pFirstTexLayerID = NULL;
			m_pFirstLight = NULL;
			m_pFirstSegment = NULL;

			m_szVertRemapFilename[0] = 0;
			m_nRemapCount = 0;
			m_paVertRemaps = NULL;

			m_pNext = NULL;
		}

		//
		//
		~MLMesh( void )
		{
			FreeScratchMemory();

			if ( m_paVertRemaps )
			{
				delete [] m_paVertRemaps;
				m_paVertRemaps = NULL;
			}

			MLMaterial *pNextMat, *pMat = m_pFirstMaterial;
			while ( pMat )
			{
				pNextMat = pMat->m_pNext;
				delete pMat;
				pMat = pNextMat;
			}
			m_pFirstMaterial = NULL;

			MLSegment *pNextSeg, *pSeg = m_pFirstSegment;
			while ( pSeg )
			{
				pNextSeg = pSeg->m_pNext;
				delete pSeg;
				pSeg = pNextSeg;
			}
			m_pFirstSegment = NULL;

			MLTexLayerID *pNextTLID, *pTLID = m_pFirstTexLayerID;
			while ( pTLID )
			{
				pNextTLID = pTLID->m_pNext;
				delete pTLID;
				pTLID = pNextTLID;
			}
			m_pFirstTexLayerID = NULL;

			MLMeshLight *pNextLight, *pLight = m_pFirstLight;
			while ( pLight )
			{
				pNextLight = pLight->m_pNext;
				delete pLight;
				pLight = pNextLight;
			}
			m_pFirstLight = NULL;
		}


		virtual BOOL SetBoundingSphere( CFSphere *pBoundingSphere ) = 0;
		virtual u16 AllocateMLMaterial( KongMat_t *pKMat ) = 0;
		virtual u32 WriteVertRemapFile( u32 nVBs );

		//
		//
		virtual BOOL SetBoundingBox( CFVec3 *pvMin, CFVec3 *pvMax )
		{
			FASSERT( pvMin && pvMax );

			m_FMesh.vBoundBoxMin_MS = *pvMin;
			m_FMesh.vBoundBoxMax_MS = *pvMax;

			return TRUE;
		}

		//
		//
		BOOL SetLODSwitchDistance( u16 nLOD, f32 fDistance )
		{
			if ( nLOD >= FDATA_MAX_LOD_MESH_COUNT )
			{
				return FALSE;
			}

			m_FMesh.afLODDistance[nLOD] = fDistance;
			return TRUE;
		}

		//
		//
		BOOL SetCollisionLOD( u16 nLOD )
		{
			if ( nLOD >= FDATA_MAX_LOD_MESH_COUNT )
			{
				return FALSE;
			}

			m_nCollisionLOD = nLOD;
			return TRUE;
		}

		//
		//
		BOOL SetShadowLODBias( u16 nLODBias )
		{
			if ( nLODBias >= FDATA_MAX_LOD_MESH_COUNT )
			{
				return FALSE;
			}

			m_FMesh.nShadowLODBias = (u8)nLODBias;
			return TRUE;
		}

		//
		//
		BOOL GenerateVertexRemap( cchar *pszFilename, u32 nVertexCount, SYSTEMTIME *pAPELastModifiedTime )
		{
			strcpy( m_szVertRemapFilename, pszFilename );
			m_nRemapCount = nVertexCount;
			m_paVertRemaps = new KongVertRemap_t[m_nRemapCount];
			if ( !m_paVertRemaps )
			{
				return FALSE;
			}

			APELastWriteTime = *pAPELastModifiedTime;

			memset( m_paVertRemaps, 0xff, sizeof( KongVertRemap_t ) * m_nRemapCount );
			return TRUE;
		};

		//
		//
		MLSegment* AllocateMLSegment( KongSeg_t *pKongSeg )
		{
			MLSegment *pNewSegment = new MLSegment( pKongSeg );
			if ( !pNewSegment )
			{
				return NULL;
			}

			// Add the new segment to the mesh and return it
			AddSegment( pNewSegment );
			return pNewSegment;
		};

		//
		//
		BOOL ResetFlags( void )
		{
			m_FMesh.nFlags = 0;
			return TRUE;
		}

		//
		//
		BOOL SetCollkDOPType( u32 nType )
		{
			if ( nType < 0 || nType >= FkDOP_MAX_kDOPS )
			{
				FASSERT_NOW;
				return FALSE;
			}

			m_nCollkDOPType = nType;
			return TRUE;
		}

		//
		// See FMESH_FLAG_* for info on the flag settings
		BOOL AddFlags( u32 nFlags )
		{
			m_FMesh.nFlags |= (nFlags & 0xff);
			if ( m_FMesh.nFlags & FMESH_FLAGS_VOLUME_MESH )
			{
				MLMesh_bVolumeMesh = TRUE;
			}
			return TRUE;
		}

		//
		// 
		BOOL SetBoneArray( FMeshBone_t *pBoneArray, u8 *pSkeletonArray, u32 nBoneCount, u32 nIndexCount, u32 nRootIndex, u32 nUsedBoneCount )
		{
			FASSERT( pBoneArray && pSkeletonArray );

			m_pBoneArray = pBoneArray;
			m_pSkeletonArray = pSkeletonArray;

			m_FMesh.nBoneCount = (u8)nBoneCount;
			m_nSkeletonArraySize = nIndexCount;

			m_FMesh.nRootBoneIndex = (u8)nRootIndex;
			m_FMesh.nUsedBoneCount = (u8)nUsedBoneCount;
#if _DEBUG
			f32 fR, fU, fF, fDelta, fSum;

			for ( u32 i = 0; i < nBoneCount; i++ )
			{
				fR = pBoneArray[i].AtRestModelToBoneMtx.m_vRight.Mag();
				fU = pBoneArray[i].AtRestModelToBoneMtx.m_vUp.Mag();
				fF = pBoneArray[i].AtRestModelToBoneMtx.m_vFront.Mag();
				fSum = (fR + fU + fF);

				fDelta = FMATH_FABS( fR - fU );
				fDelta += FMATH_FABS( fR - fF );
				fDelta += FMATH_FABS( fU - fF );
				fDelta /= fSum;

				if( fDelta >= 0.01f ) 
				{
					DEVPRINTF( "MLMesh::SetBoneArray() - ERROR - Bone submitted with non-uniform scale.\n" );
					return FALSE;
				}

				fR = pBoneArray[i].AtRestBoneToModelMtx.m_vRight.Mag();
				fU = pBoneArray[i].AtRestBoneToModelMtx.m_vUp.Mag();
				fF = pBoneArray[i].AtRestBoneToModelMtx.m_vFront.Mag();
				fSum = (fR + fU + fF);

				fDelta = FMATH_FABS( fR - fU );
				fDelta += FMATH_FABS( fR - fF );
				fDelta += FMATH_FABS( fU - fF );
				fDelta /= fSum;

				if( fDelta >= 0.01f ) 
				{
					DEVPRINTF( "MLMesh::SetBoneArray() - ERROR - Bone submitted with non-uniform scale.\n" );
					return FALSE;
				}

				fR = pBoneArray[i].AtRestParentToBoneMtx.m_vRight.Mag();
				fU = pBoneArray[i].AtRestParentToBoneMtx.m_vUp.Mag();
				fF = pBoneArray[i].AtRestParentToBoneMtx.m_vFront.Mag();
				fSum = (fR + fU + fF);

				fDelta = FMATH_FABS( fR - fU );
				fDelta += FMATH_FABS( fR - fF );
				fDelta += FMATH_FABS( fU - fF );
				fDelta /= fSum;

				if( fDelta >= 0.01f ) 
				{
					DEVPRINTF( "MLMesh::SetBoneArray() - ERROR - Bone submitted with non-uniform scale.\n" );
					return FALSE;
				}

				fR = pBoneArray[i].AtRestBoneToParentMtx.m_vRight.Mag();
				fU = pBoneArray[i].AtRestBoneToParentMtx.m_vUp.Mag();
				fF = pBoneArray[i].AtRestBoneToParentMtx.m_vFront.Mag();
				fSum = (fR + fU + fF);

				fDelta = FMATH_FABS( fR - fU );
				fDelta += FMATH_FABS( fR - fF );
				fDelta += FMATH_FABS( fU - fF );
				fDelta /= fSum;

				if( fDelta >= 0.01f ) 
				{
					DEVPRINTF( "MLMesh::SetBoneArray() - ERROR - Bone submitted with non-uniform scale.\n" );
					return FALSE;
				}
			}
#endif
			return TRUE;
		}

		//
		// Compression bias must be 0 - 10.
		//		0 causes no compression to be applied
		//		10 allows for maximum compression
		// 
		void SetCompressionBias( u32 nNewBias )
		{
			FASSERT( nNewBias >= 0 && nNewBias <= 10 );

			m_nCompressionBias = nNewBias;
		}

		//
		// See comments for enum, above
		//
		void SetCollTreeType( u32 nTreeType )
		{
			FASSERT( nTreeType >= 0 && nTreeType < MLMESH_TREETYPE_MAX );

			m_nTreeCalcType = nTreeType;
		}

		//
		//
		s32 AddTexLayerID( FMeshTexLayerID_t *pTexLayerID )
		{
			FASSERT( pTexLayerID );

			if ( !m_pFirstTexLayerID )
			{
				m_pFirstTexLayerID = new MLTexLayerID( pTexLayerID );
				if ( !m_pFirstTexLayerID )
				{
					return -1;
				}
				m_FMesh.nTexLayerIDCount++;

				return 0;
			}

			s32 nReturn = 0;
			MLTexLayerID *pLast, *pTLID = m_pFirstTexLayerID;
			while ( pTLID )
			{
				pLast = pTLID;

				if ( pTLID->IsTheSame( pTexLayerID ) )
				{
					return nReturn;
				}

				nReturn++;
				pTLID = pTLID->m_pNext;
			}

			pTLID = new MLTexLayerID( pTexLayerID );
			if ( !pTLID )
			{
				return -1;
			}
			m_FMesh.nTexLayerIDCount++;
			pLast->m_pNext = pTLID;

			return nReturn;
		}

		//
		//
		BOOL AddLight( ApeLight_t *pLight )
		{
			FASSERT( pLight );

			MLMeshLight *pMeshLight = new MLMeshLight( pLight );

			m_FMesh.nLightCount++;

			if ( !m_pFirstLight )
			{
				m_pFirstLight = pMeshLight;
				return TRUE;
			}

			MLMeshLight *pLast = m_pFirstLight;
			while ( pLast->m_pNext )
			{
				pLast = pLast->m_pNext;
			}

			pLast->m_pNext = pMeshLight;

			return TRUE;
		}

		MLMaterial* GetMaterialAtIdx( u32 nAllocIdx )
		{
			FASSERT( nAllocIdx >= 0 && nAllocIdx < m_nMaterialsAllocedCount );

			return m_apMaterialsAlloced[nAllocIdx];
		}

};	

#endif
