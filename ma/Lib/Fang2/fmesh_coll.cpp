//////////////////////////////////////////////////////////////////////////////////////
// fmesh_coll.cpp - Generic C Fang mesh collision module
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
// 08/28/02	Lafleur		Adapted from fgcmesh_coll.cpp & fdx8mesh_coll.cpp
//////////////////////////////////////////////////////////////////////////////////////


#include "fmesh_coll.h"
#include "fmesh.h"

#if FANG_PLATFORM_GC
	#include "fgcvb.h"
	#include "fgcmesh.h"
#elif FANG_PLATFORM_DX
	#include "dx\fdx8mesh.h"
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#include "ps2\fps2mesh.h"
//ARG - <<<<<
#endif

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	#include "frenderer.h"
	#include "fdraw.h"
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
#if FANG_PLATFORM_PC
	#define _MAX_KDOP_DRAWS				256
	#define _MAX_COLL_TRIS_IN_BUFFER	256
	#define _MAX_TRIS_IN_BUFFER			256
#else
	#define _MAX_KDOP_DRAWS				64
	#define _MAX_COLL_TRIS_IN_BUFFER	64
	#define _MAX_TRIS_IN_BUFFER			128
#endif
#endif

//////////////////////////////////////////////////////////////////////////////////////
// Local classes and structures:
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

// This is the primary node stack used for all collision checks
FkDOP_Node_t *FMesh_Coll_apNodeStack[ FMESH_COLL_NODE_STACK_SIZE ];
u32 FMesh_Coll_nNodeStackIdx;

#if !FANG_PRODUCTION_BUILD
	u32 FMesh_Coll_nDeepestNodeStackIdx = 0;
	u32 FMesh_Coll_nNodesChecked = 0;

	u32 FMesh_Coll_nDOPTransforms = 0;
	
	// Collision culling performance testing
	u32 FMesh_Coll_nTests;
	u32 FMesh_Coll_nTestsLevel1;
	u32 FMesh_Coll_nTestsLevel2;
	u32 FMesh_Coll_nHits;
#endif

FMesh_Coll_TransformedkDOP_t	*FKDOP_paTransformedkDOPBuffer;
u32								FKDOP_nTransformedkDOPIdx = 0;

//const CFMtx43A		*FMesh_pMeshToCylinder;
const CFMtx43A		*FMesh_pModelToWorld;
const CFCollInfo	*FMesh_Coll_pCurrCollInfo;
const CFMeshInst	*FMesh_Coll_pCurrMeshInst;
FkDOP_Tree_t		*FMesh_Coll_pCurrkDOPTree;
FkDOP_Tree_t		*FMesh_Coll_pCurrkDOPTree2;
u8					FMesh_Coll_nCurrBoneIndex;
u16					FMesh_Coll_nCurrCollMask;
FCollImpact_t 		*FMesh_Coll_pClosestImpactSlot;
u16					Fmesh_Coll_nAccumCollMask;
BOOL8				FMesh_Coll_bShowCollision = FALSE;


f32 				FMesh_Coll_fBoneScaleR;
f32					FMesh_Coll_fBoneScale;

//FMesh_Coll_SphereTest_t FMesh_Coll_spListXfm[FMESH_COLL_MAX_SPHERES_IN_LIST];


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

static u32 _nCurrSegmentColl;

static const CFMtx43A	*_pWorldToModel;

static CFVec3A	_vTestRayEnd;
static f32		_fTestRayLength;

static BOOL		_bModuleInitialized = FALSE;



#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	static CFXfm _xfmIdentity;
	static FDrawVtx_t _vTriVerts[ _MAX_TRIS_IN_BUFFER * 3 ];
	static FDrawVtx_t _vLineVerts[ _MAX_TRIS_IN_BUFFER * 6 ];
	static u32 _nDrawTriCount = 0;

	static const FkDOP_Interval_t *_apkDOPIntervals[ _MAX_KDOP_DRAWS ];
	static u8 _ankDOPType[ _MAX_KDOP_DRAWS ];
	static CFMtx43A _aMtxkDOPTransform[ _MAX_KDOP_DRAWS ];
	static u32 _nkDOPDrawCount = 0;

	static CFVec3A _vImpactPoint[ _MAX_COLL_TRIS_IN_BUFFER ];
	static u32 _nImpactColor[ _MAX_COLL_TRIS_IN_BUFFER ];
	static u32 _nDrawImpactCount = 0;

	static FDrawVtx_t _vCollTriVerts[ _MAX_COLL_TRIS_IN_BUFFER * 3 ];
	static FDrawVtx_t _vCollLineVerts[ _MAX_COLL_TRIS_IN_BUFFER * 6 ];
	static u32 _nDrawCollTriCount = 0;
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static void _Sphere_ConvertModelImpactsToWorld( u32 nStart, const CFMtx43A *pMtx );
static void _Sphere_ConvertModelImpactsToWorldWithScale( u32 nStart, const CFMtx43A *pMtx );
static void _Ray_ConvertModelImpactsToWorld( u32 nStart, const CFMtx43A *pMtx );
static void _Ray_ConvertModelImpactsToWorldWithScale( u32 nStart, const CFMtx43A *pMtx );
static void _ProjSphere_ConvertModelImpactsToWorld( u32 nStart, const CFMtx43A *pMtx );
static void _ProjSphere_ConvertModelImpactsToWorldWithScale( u32 nStart, const CFMtx43A *pMtx );

#if !FANG_PRODUCTION_BUILD
inline void _BeginTriDraw( void );
inline void _EndTriDraw( void );
static void _AddCollDrawTri( const CFVec3 *pVertArray, const CFMtx43A *pTransform );
inline void _DrawMeshTri( const CFVec3 *pVertArray, const CFVec3 *pNormal, CFMtx43A *pTransform, CFColorRGBA *pColor, BOOL bLight );
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////


//
//
//
BOOL fmesh_coll_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	_bModuleInitialized = FALSE;

	FKDOP_paTransformedkDOPBuffer = NULL;

	FKDOP_paTransformedkDOPBuffer = (FMesh_Coll_TransformedkDOP_t *)fres_AlignedAlloc( sizeof( FMesh_Coll_TransformedkDOP_t ) * FKDOP_TRANSFORMED_KDOP_BUFFER_SIZE, FCLASS_BYTE_ALIGN );
	if ( !FKDOP_paTransformedkDOPBuffer )
	{
		return FALSE;
	}
	FKDOP_nTransformedkDOPIdx = 0;

	Fmesh_Coll_nAccumCollMask = 0;

	_bModuleInitialized = TRUE;

	return TRUE;
}

//
//
//
void fmesh_coll_ModuleShutdown( void ) 
{
	_bModuleInitialized = FALSE;
}


//
//
//
u32 fmesh_coll_CalculateBoneMatrix( const CFMeshInst *pMeshInst, u32 nkDOPTreeIdx, CFMtx43A *pBoneMtx, CFMtx43A *pInvBoneMtx/*=NULL*/, f32 *pfBoneScale/*=NULL*/, f32 *pfInvBoneScale/*=NULL*/ )
{
	FASSERT( pMeshInst && pBoneMtx );

	FMesh_t *pMesh = pMeshInst->m_pMesh;

	// Calculate the test mesh's bone matrix
	if ( pMesh->nUsedBoneCount == 0 )
	{
		// This object has no bones
		pBoneMtx->Set( pMeshInst->m_Xfm.m_MtxF );

		if ( pInvBoneMtx )
		{
			pInvBoneMtx->Set( pMeshInst->m_Xfm.m_MtxR );
		}
		if ( pfBoneScale )
		{
			*pfBoneScale =  pMeshInst->m_Xfm.m_fScaleF;
			if ( pfInvBoneScale )
			{
				*pfInvBoneScale = fmath_Inv( *pfBoneScale );
			}
		}

		return 0;
	}

	u8 nBoneIdx = pMesh->aSeg[pMesh->paCollTree[nkDOPTreeIdx].nSegmentIdx].anBoneMtxIndex[0];
	if ( pMeshInst->m_nFlags & FMESHINST_FLAG_NOBONES ) 
	{
		// This object is not animating
		pBoneMtx->Mul( pMeshInst->m_Xfm.m_MtxF, pMesh->pBoneArray[nBoneIdx].AtRestBoneToModelMtx );
	}
	else
	{
		// This object is animating, so it's bone palette will be cool
		CFMtx43A **apBoneMtxList = pMeshInst->GetBoneMtxPalette();
		pBoneMtx->Set( *apBoneMtxList[nBoneIdx] );
	}

	if ( pInvBoneMtx )
	{
        pInvBoneMtx->ReceiveAffineInverse( *pBoneMtx, TRUE );
	}
	if ( pfBoneScale )
	{
		*pfBoneScale = pBoneMtx->m_vRight.Mag();
		if ( pfInvBoneScale )
		{
			*pfInvBoneScale = fmath_Inv( *pfBoneScale );
		}
	}

	return nBoneIdx;
}


//
//
//
BOOL _TestPointForContainment( CFVec3A *pCenter, f32 fTestRadius )
{
	FASSERT( pCenter && fTestRadius >= 0.f );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );
	
	u32 i, ii;
	CFVec3A vNormal, vVertex, vDiff;

	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test this node's kDOP for collision
		if ( !fkdop_IntersectkDOPAndSphere( pCenter, fTestRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
		{
			continue;
		}
	
		if ( !pNode->paPackets )
		{
			// This node is not a leaf so push its children onto the stack
			FASSERT( FMesh_Coll_nNodeStackIdx + 2 <= FMESH_COLL_NODE_STACK_SIZE );
			FASSERT( pNode->nStartChildIdx + 1 < FMesh_Coll_pCurrkDOPTree->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx + 1];
			continue;
		}
	
		// This node is a leaf so we need to test collision against the tris
		
		#if FANG_PLATFORM_GC
			FkDOP_CNormal_t *pNormals = (FkDOP_CNormal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		#elif FANG_PLATFORM_DX
			FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
//ARG - >>>>>
		#elif FANG_PLATFORM_PS2
			FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
//ARG - <<<<<
		#else
			FASSERT_NOW;
		#endif
		
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			#if FANG_PLATFORM_GC
				FGCVB_t *pVB = &FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->aVB[pPacket->nVBIdx];
			#elif FANG_PLATFORM_DX
				CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				u32 *pVerts = (u32 *)FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			for ( ii = 0; ii < pPacket->nTriCount; ii++, pNormals++ )
			{
				// Get the normal 
				#if FANG_PLATFORM_GC
					pNormals->DecompressTo( vNormal );
				#elif FANG_PLATFORM_DX
					vNormal.Set( *pNormals );
//ARG - >>>>>
				#elif FANG_PLATFORM_PS2
					vNormal.Set( *pNormals );
//ARG - <<<<<
				#else
					FASSERT_NOW;
				#endif

				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				#if FANG_PLATFORM_GC
					pVB->GetPoint( pVertIdx[nVert], vVertex.v3 );
				#elif FANG_PLATFORM_DX
					vVertex.v3 = pVerts[ pVertIdx[nVert] ];
//ARG - >>>>>
				#elif FANG_PLATFORM_PS2
					vVertex.v3 = *((CFVec3 *)pVerts[ pVertIdx[nVert] ]);
//ARG - <<<<<
				#else
					FASSERT_NOW;
				#endif

				vDiff.Sub( *pCenter, vVertex );
				f32 fDist = vDiff.Dot( vNormal );
				if ( fDist - fTestRadius > 0.f )
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


//
//
//
s32 CFMeshInst::IsPointInMesh( CFVec3A *pPoint, f32 fRadius/*=0.f*/ )
{
	FASSERT( m_pMesh && pPoint );

	if ( !m_pMesh->paCollTree )
	{
		return 0;
	}

	if ( m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		return -1;
	}

	// Mesh is not in world space. Let's transform its bound sphere into world space...
	CFSphere spTest;
	m_Xfm.TransformSphereF( spTest, m_BoundSphere_MS );

	u32 i;
	f32 fTestRadius;
	CFVec3A vCenter;

	// Do a quick test against the mesh's bounding sphere
	vCenter.v3 = spTest.m_Pos - pPoint->v3;
	fTestRadius = vCenter.MagSq();
	f32 fRadiusSq = (spTest.m_fRadius + fRadius);
	fRadiusSq *= fRadiusSq;
	if ( fTestRadius > fRadiusSq )
	{
		return 0;
	}

	fTestRadius = 0.f;
	FMesh_Coll_pCurrMeshInst = this;

	if ( m_pMesh->nBoneCount == 0 )
	{
		// This object has no bones so we don't check the bone matrix palette
		FASSERT( m_pMesh->nSegCount == 1 );
	
		FMesh_Coll_pCurrkDOPTree = m_pMesh->paCollTree;
		
		// Set radius based on bone scale
		if ( fRadius != 0.f )
		{
			FMesh_Coll_fBoneScaleR = m_Xfm.m_fScaleR;
			if ( FMesh_Coll_fBoneScaleR > 0.99f && FMesh_Coll_fBoneScaleR < 1.01f )
			{
				fTestRadius = fRadius;
			}
			else
			{
				fTestRadius = fRadius * FMesh_Coll_fBoneScaleR;
			}
		}

		m_Xfm.m_MtxR.MulPoint( vCenter, *pPoint );

		// Prime the node stack and start the collision tests
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

		if ( _TestPointForContainment( &vCenter, fTestRadius) == FALSE )
		{
			return 0;
		}
	}
	else if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) )
	{
		// This mesh has bones and IS animating so we can use the bone matrix palette
		CFMtx43A **apBoneMtxList, mtxInvBone;
		apBoneMtxList = GetBoneMtxPalette();
		FASSERT( apBoneMtxList );
		
		for ( i = 0; i < m_pMesh->nCollTreeCount; i++ )	
		{
			FMesh_Coll_pCurrkDOPTree = &m_pMesh->paCollTree[i];
			FASSERT( FMesh_Coll_pCurrkDOPTree );

			// Get the current segment and bone
			FMesh_Coll_nCurrBoneIndex = m_pMesh->aSeg[FMesh_Coll_pCurrkDOPTree->nSegmentIdx].anBoneMtxIndex[0];

			// Make sure this is part of the active parts mask
			if ( !(Parts_GetDrawnPartsMask() & Parts_GetCollPartsMask() & (1 << m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
			{
				// The bone is not part of the active parts so skip it
				continue;
			}

			// Get the root node in the kDOP tree
			FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
			CFMtx43A *pBoneMtx = apBoneMtxList[FMesh_Coll_nCurrBoneIndex];
			
			// Set radius based on bone scale
			FMesh_Coll_fBoneScaleR = pBoneMtx->m_vFront.MagSq();
			if ( FMesh_Coll_fBoneScaleR < 0.00001f )
			{
				continue;
			}

			FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScaleR );
			if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
			{
				fTestRadius = fRadius;
			}
			else
			{
				fTestRadius = fRadius * FMesh_Coll_fBoneScaleR;
			}
			
			// Bring the sphere into bone space
			mtxInvBone.ReceiveAffineInverse_KnowOOScale2( *pBoneMtx, FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
			mtxInvBone.MulPoint( vCenter, *pPoint );
			
			// Prime the node stack and start the collision tests
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;

			if ( _TestPointForContainment( &vCenter, fTestRadius) == FALSE )
			{
				return 0;
			}
		}
	}
	else
	{
		// This mesh has bones but is not animating, so we cannot use the bone matrix palette
		CFMtx43A mtxInvBone;
		CFMtx43A mtxTransform;
		
		for ( i = 0; i < m_pMesh->nCollTreeCount; i++ )	
		{
			FMesh_Coll_pCurrkDOPTree = &m_pMesh->paCollTree[i];
			FASSERT( FMesh_Coll_pCurrkDOPTree );

			// Get the current segment and bone index
			FMesh_Coll_nCurrBoneIndex = m_pMesh->aSeg[FMesh_Coll_pCurrkDOPTree->nSegmentIdx].anBoneMtxIndex[0];

			// Make sure this is part of the active parts mask
			if ( !(Parts_GetDrawnPartsMask() & Parts_GetCollPartsMask() & (1 << m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
			{
				// The bone is not part of the active parts so skip it
				continue;
			}

			// Get the root node in the kDOP tree
			FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

			// Set radius based on bone scale
			mtxTransform.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].AtRestBoneToModelMtx );
			FMesh_Coll_fBoneScale = mtxTransform.m_vFront.MagSq();
			if ( FMesh_Coll_fBoneScale < 0.00001f )
			{
				continue;
			}

			FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
			if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
			{
				fTestRadius = fRadius;
			}
			else
			{
				fTestRadius = fRadius * FMesh_Coll_fBoneScaleR;
			}

			// Calculate the model to world matrix and bring the sphere into bone space
			mtxInvBone.ReceiveAffineInverse_KnowOOScale2( mtxTransform, FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
			mtxInvBone.MulPoint( vCenter, *pPoint );

			// Prime the node stack and start the collision tests
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;

			if ( _TestPointForContainment( &vCenter, fTestRadius) == FALSE )
			{
				return 0;
			}
		}
	}

	return 1;
}


//
//
//
void fmesh_Coll_TestSphere( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo ) 
{
	FASSERT( pMeshInst && pCollInfo );
//	FASSERT( pCollInfo->nSphereListCount + 1 < FMESH_COLL_MAX_SPHERES_IN_LIST );
	
	u32 i, nEnteringCount;
	FMesh_Coll_SphereTest_t Test;
	FMesh_t *pMesh = pMeshInst->m_pMesh;

	// Make sure this mesh's mask matches
	if ( !(pMesh->nMeshCollMask & pCollInfo->nCollMask) )
	{
		return;
	}

	// Set the globals
#if !FANG_PRODUCTION_BUILD
	FMesh_Coll_nNodesChecked = 0;
#endif
	FMesh_Coll_pCurrCollInfo = pCollInfo;
	FMesh_Coll_pCurrMeshInst = pMeshInst;
	FMesh_Coll_nCurrBoneIndex = 0;
	Fmesh_Coll_nAccumCollMask = 0;

	if ( pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		// This mesh is a volume mesh so we can assume all data is in worldspace
		FASSERT( pMesh->nSegCount == 1 );
		
		_nCurrSegmentColl = 0;
		FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;
		FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
		
		// Setup test parameters
		Test.fRadius = pCollInfo->pSphere_WS->m_fRadius;
		Test.fRadiusSq = Test.fRadius * Test.fRadius;
		Test.vCenter.Set( pCollInfo->pSphere_WS->m_Pos );

		// Clear the bone matrix info
		FMesh_pModelToWorld = NULL;
		_pWorldToModel = NULL;
		FMesh_Coll_fBoneScale = FMesh_Coll_fBoneScaleR = 1.f;

		// Prime the node stack and start the collision tests
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
		nEnteringCount = FColl_nImpactCount;	
/*
		if ( pCollInfo->nSphereListCount > 0 )
		{
			fmesh_Coll_TestSphereListWithDOPs( &Test );
		}
		else
*/
		{
			fmesh_Coll_TestSphereWithDOPs( &Test );
		}

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			// Submit the collision triangles for drawing
			for ( i = nEnteringCount; i < FColl_nImpactCount; i++ )
			{
				fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
				fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
			}
		}
#endif // FMESH_DRAW_COLLIDED_TRIS
	}
	else
	{
		CFVec3A vCenter;
		vCenter.Set( pCollInfo->pSphere_WS->m_Pos );

		if ( pMeshInst->m_pMesh->nBoneCount == 0 )
		{
			// This object has no bones so we don't check the bone matrix palette
			FASSERT( pMesh->nSegCount == 1 );
		
			_nCurrSegmentColl = 0;
			FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;
			FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
			
			// Set radius based on bone scale
			FMesh_Coll_fBoneScaleR = pMeshInst->m_Xfm.m_fScaleR;
			FMesh_Coll_fBoneScale = pMeshInst->m_Xfm.m_fScaleF;
			if ( FMesh_Coll_fBoneScaleR > 0.99f && FMesh_Coll_fBoneScaleR < 1.01f )
			{
				FMesh_Coll_fBoneScale = FMesh_Coll_fBoneScaleR = 1.f;
				Test.fRadius = pCollInfo->pSphere_WS->m_fRadius;
				Test.fRadiusSq = Test.fRadius * Test.fRadius;
			}
			else
			{
				Test.fRadius = pCollInfo->pSphere_WS->m_fRadius * FMesh_Coll_fBoneScaleR;
				Test.fRadiusSq = Test.fRadius * Test.fRadius;
			}

			// We can use the mesh's Xfm for quicker processing
			_pWorldToModel = &pMeshInst->m_Xfm.m_MtxR;
			_pWorldToModel->MulPoint( Test.vCenter, vCenter );
			FMesh_pModelToWorld = &pMeshInst->m_Xfm.m_MtxF;
			
			// Prime the node stack and start the collision tests
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
			nEnteringCount = FColl_nImpactCount;
/*
			if ( pCollInfo->nSphereListCount > 0 )
			{
				fmesh_Coll_TestSphereListWithDOPs( &Test );
			}
			else
*/
			{
				fmesh_Coll_TestSphereWithDOPs( &Test );
			}

			// If we have collisions, we need to convert them from model to world space
			if ( nEnteringCount < FColl_nImpactCount)
			{
				if ( FMesh_Coll_fBoneScaleR == 1.f )
				{
					_Sphere_ConvertModelImpactsToWorld( nEnteringCount, FMesh_pModelToWorld );
				}
				else
				{
					_Sphere_ConvertModelImpactsToWorldWithScale( nEnteringCount, FMesh_pModelToWorld );
				}
			}
		}
		else if ( !(pMeshInst->m_nFlags & FMESHINST_FLAG_NOBONES) )
		{
			// This mesh has bones and IS animating so we can use the bone matrix palette
			CFMtx43A **apBoneMtxList, mtxInvBone;
			apBoneMtxList = pMeshInst->GetBoneMtxPalette();
			FASSERT( apBoneMtxList );
			
			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );

				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				// Get the current segment and bone
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
				CFMtx43A *pBoneMtx = apBoneMtxList[FMesh_Coll_nCurrBoneIndex];
				
				// Set radius based on bone scale
				FMesh_Coll_fBoneScale = pBoneMtx->m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
				{
					FMesh_Coll_fBoneScale = FMesh_Coll_fBoneScaleR = 1.f;
					Test.fRadius = pCollInfo->pSphere_WS->m_fRadius;
					Test.fRadiusSq = Test.fRadius * Test.fRadius;
				}
				else
				{
					Test.fRadius = pCollInfo->pSphere_WS->m_fRadius * FMesh_Coll_fBoneScaleR;
					Test.fRadiusSq = Test.fRadius * Test.fRadius;
				}
				
				// Bring the sphere into bone space
				FMesh_pModelToWorld = pBoneMtx;
				mtxInvBone.ReceiveAffineInverse_KnowOOScale2( *pBoneMtx, FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				mtxInvBone.MulPoint( Test.vCenter, vCenter );
				_pWorldToModel = &mtxInvBone;
				
				// Prime the node stack and start the collision tests
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				nEnteringCount = FColl_nImpactCount;
/*
				if ( pCollInfo->nSphereListCount > 0 )
				{
					fmesh_Coll_TestSphereListWithDOPs( &Test );
				}
				else
*/
				{
					fmesh_Coll_TestSphereWithDOPs( &Test );
				}
				
				// If we have collisions, we need to convert them from model to world space
				if ( nEnteringCount < FColl_nImpactCount)
				{
					if ( FMesh_Coll_fBoneScaleR == 1.f )
					{
						_Sphere_ConvertModelImpactsToWorld( nEnteringCount, FMesh_pModelToWorld );
					}
					else
					{
						_Sphere_ConvertModelImpactsToWorldWithScale( nEnteringCount, FMesh_pModelToWorld );
					}

					if ( pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask )
					{
						break;
					}
				}
			}
		}
		else
		{
			// This mesh has bones but is not animating, so we cannot use the bone matrix palette
			CFMtx43A mtxInvBone;
			CFMtx43A mtxTransform;
			FMesh_pModelToWorld = &mtxTransform;
			
			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );

				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				// Get the current segment and bone index
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

				// Set radius based on bone scale
				mtxTransform.Mul( pMeshInst->m_Xfm.m_MtxF, pMeshInst->m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].AtRestBoneToModelMtx );
				FMesh_Coll_fBoneScale = mtxTransform.m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
				{
					FMesh_Coll_fBoneScale = FMesh_Coll_fBoneScaleR = 1.f;
					Test.fRadius = pCollInfo->pSphere_WS->m_fRadius;
					Test.fRadiusSq = Test.fRadius * Test.fRadius;
				}
				else
				{
					Test.fRadius = pCollInfo->pSphere_WS->m_fRadius * FMesh_Coll_fBoneScaleR;
					Test.fRadiusSq = Test.fRadius * Test.fRadius;
				}

				// Calculate the model to world matrix and bring the sphere into bone space
				mtxInvBone.ReceiveAffineInverse_KnowOOScale2( mtxTransform, FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				mtxInvBone.MulPoint( Test.vCenter, vCenter );
				_pWorldToModel = &mtxInvBone;

				// Prime the node stack and start the collision tests
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				nEnteringCount = FColl_nImpactCount;	
/*
				if ( pCollInfo->nSphereListCount > 0 )
				{
					fmesh_Coll_TestSphereListWithDOPs( &Test );
				}
				else
*/
				{
					fmesh_Coll_TestSphereWithDOPs( &Test );
				}
				
				
				// If we have collisions, we need to convert them from model to world space
				if ( nEnteringCount < FColl_nImpactCount)
				{
					if ( FMesh_Coll_fBoneScaleR == 1.f )
					{
						_Sphere_ConvertModelImpactsToWorld( nEnteringCount, &mtxTransform );
					}
					else
					{
						_Sphere_ConvertModelImpactsToWorldWithScale( nEnteringCount, &mtxTransform );
					}

					if ( pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask )
					{
						break;
					}
				}
			}
		}
	}

	FColl_nImpactCollMask |= Fmesh_Coll_nAccumCollMask;
}

/*
//
//
//
void fmesh_Coll_TransformSphereList( FMesh_Coll_SphereTest_t *pTest )
{
	u32 i;
	FMesh_Coll_SphereTest_t *pSphere = FMesh_Coll_spListXfm;

	if ( !_pWorldToModel )
	{
		for ( i = 0; i < FMesh_Coll_pCurrCollInfo->nSphereListCount; i++, pSphere++ )
		{
			pSphere->vCenter.Set( FMesh_Coll_pCurrCollInfo->pCurSphereList_WS[i].m_Pos );
			pSphere->fRadius = FMesh_Coll_pCurrCollInfo->pCurSphereList_WS[i].m_fRadius;
			pSphere->fRadiusSq = pSphere->fRadius * pSphere->fRadius;
		}
	}
	else
	{
		for ( i = 0; i < FMesh_Coll_pCurrCollInfo->nSphereListCount; i++, pSphere++ )
		{
			pSphere->vCenter.v3 =  _pWorldToModel->m44.MultPoint( FMesh_Coll_pCurrCollInfo->pCurSphereList_WS[i].m_Pos );
			pSphere->fRadius = FMesh_Coll_pCurrCollInfo->pCurSphereList_WS[i].m_fRadius * FMesh_Coll_fBoneScaleR;
			pSphere->fRadiusSq = pSphere->fRadius * pSphere->fRadius;
		}
	}
}
*/

//
//
//
void fmesh_Coll_TestRay( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo ) 
{
	FASSERT( pMeshInst && pCollInfo );

	u32 i;
	FMesh_Coll_RayTest_t Test;
	u32 nEnteringCount;
	FMesh_t *pMesh = pMeshInst->m_pMesh;

	// Verify that there is some ray length submitted
	if ( pCollInfo->Ray.fLength < 0.0001f )
	{
		return;
	}

	// Make sure this mesh's mask matches
	if ( !(pMesh->nMeshCollMask & pCollInfo->nCollMask) )
	{
		return;
	}

	// Set the globals
#if !FANG_PRODUCTION_BUILD
	FMesh_Coll_nNodesChecked = 0;
#endif
	FMesh_Coll_pCurrMeshInst = pMeshInst;
	FMesh_Coll_pCurrCollInfo = pCollInfo;
	FMesh_Coll_nCurrBoneIndex = 0;
	Fmesh_Coll_nAccumCollMask = 0;

	u32 nStartingImpactCount = FColl_nImpactCount;

	// Set the starting ray info
	_vTestRayEnd.Set( pCollInfo->Ray.vEnd_WS );
	_fTestRayLength = pCollInfo->Ray.fLength;

	if ( pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		// This mesh is a volume mesh so we can assume all data is in worldspace
		FASSERT( pMesh->nSegCount == 1 );
		
		// Set the test parameters
		Test.vStart.Set( pCollInfo->Ray.vStart_WS );
		Test.vEnd.Set( _vTestRayEnd );
		Test.vNormal.Set( pCollInfo->Ray.vNormal_WS );
		Test.fDistance = _fTestRayLength;
		Test.fOriginalDistance = pCollInfo->Ray.fLength;

		FMesh_pModelToWorld = NULL;
		_pWorldToModel = NULL;
		FMesh_Coll_fBoneScale = FMesh_Coll_fBoneScaleR = 1.f;
		
		FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

		FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
		
		// Prime the node stack and start the test
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
		nEnteringCount = FColl_nImpactCount;	
		if ( pCollInfo->bCullBacksideCollisions )
		{
			fmesh_Coll_TestRayWithDOPs_NoBF( &Test );
		}
		else
		{
			fmesh_Coll_TestRayWithDOPs_BF( &Test );
		}
		
#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			for ( i = nEnteringCount; i < FColl_nImpactCount; i++ )
			{
				fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
				fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
			}
		}
#endif // FMESH_DRAW_COLLIDED_TRIS
	}
	else
	{
		// This object is not a volume mesh, so it could be oriented

		if ( pMeshInst->m_pMesh->nBoneCount == 0 )
		{
			// This object has no bones so we don't check the bone matrix palette
			FASSERT( pMesh->nSegCount == 1 );
		
			FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

			FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
			
			pMeshInst->m_Xfm.m_MtxR.MulPoint( Test.vStart, pCollInfo->Ray.vStart_WS );
			pMeshInst->m_Xfm.m_MtxR.MulPoint( Test.vEnd, _vTestRayEnd );
			pMeshInst->m_Xfm.m_MtxR.MulDir( Test.vNormal, pCollInfo->Ray.vNormal_WS );
			FMesh_pModelToWorld = &pMeshInst->m_Xfm.m_MtxF;
			_pWorldToModel = &pMeshInst->m_Xfm.m_MtxR;

			// Set length and bone scale
			FMesh_Coll_fBoneScale = pMeshInst->m_Xfm.m_fScaleF;
			FMesh_Coll_fBoneScaleR = pMeshInst->m_Xfm.m_fScaleR;
			if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
			{
				FMesh_Coll_fBoneScaleR = 1.f;
				Test.fDistance = _fTestRayLength;
				Test.fOriginalDistance = pCollInfo->Ray.fLength;
			}
			else
			{
				Test.vNormal.Mul( FMesh_Coll_fBoneScale );
				Test.fDistance = _fTestRayLength * FMesh_Coll_fBoneScaleR;
				Test.fOriginalDistance = pCollInfo->Ray.fLength * FMesh_Coll_fBoneScaleR;
			}

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
			nEnteringCount = FColl_nImpactCount;	
			if ( pCollInfo->bCullBacksideCollisions )
			{
				fmesh_Coll_TestRayWithDOPs_NoBF( &Test );
			}
			else
			{
				fmesh_Coll_TestRayWithDOPs_BF( &Test );
			}
		
			// If we have collisions, we need to convert them from model to world space
			if ( nEnteringCount < FColl_nImpactCount )
			{
				if ( FMesh_Coll_fBoneScaleR == 1.f )
				{
					_Ray_ConvertModelImpactsToWorld( nEnteringCount, FMesh_pModelToWorld );
				}
				else
				{
					_Ray_ConvertModelImpactsToWorldWithScale( nEnteringCount, FMesh_pModelToWorld );
				}
			}
		}
		else if ( !(pMeshInst->m_nFlags & FMESHINST_FLAG_NOBONES) )
		{
			// This mesh has bones and IS animating so we can use the bone matrix palette
			CFMtx43A **apBoneMtxList, mtxInvBone;
			apBoneMtxList = pMeshInst->GetBoneMtxPalette();
			FASSERT( apBoneMtxList );
			
			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );

				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Bring the ray into bone space
				FMesh_pModelToWorld = apBoneMtxList[FMesh_Coll_nCurrBoneIndex];
				FMesh_Coll_fBoneScale = FMesh_pModelToWorld->m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				mtxInvBone.ReceiveAffineInverse_KnowOOScale2( *apBoneMtxList[FMesh_Coll_nCurrBoneIndex], FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				mtxInvBone.MulPoint( Test.vStart, pCollInfo->Ray.vStart_WS );
				mtxInvBone.MulPoint( Test.vEnd, _vTestRayEnd );
				mtxInvBone.MulDir( Test.vNormal, pCollInfo->Ray.vNormal_WS );
				_pWorldToModel = &mtxInvBone;

				// Set length and bone scale
				if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
				{
					FMesh_Coll_fBoneScaleR = 1.f;
					Test.fDistance = _fTestRayLength;
					Test.fOriginalDistance = pCollInfo->Ray.fLength;
				}
				else
				{
					Test.vNormal.Mul( FMesh_Coll_fBoneScale );
					Test.fDistance = _fTestRayLength * FMesh_Coll_fBoneScaleR;
					Test.fOriginalDistance = pCollInfo->Ray.fLength * FMesh_Coll_fBoneScaleR;
				}

				// Prime the node stack and start the test
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				nEnteringCount = FColl_nImpactCount;	
				if ( pCollInfo->bCullBacksideCollisions )
				{
					fmesh_Coll_TestRayWithDOPs_NoBF( &Test );
				}
				else
				{
					fmesh_Coll_TestRayWithDOPs_BF( &Test );
				}

				// If we have collisions, we need to convert them from model to world space
				if ( nEnteringCount < FColl_nImpactCount)
				{
					if ( FMesh_Coll_fBoneScaleR == 1.f )
					{
						_Ray_ConvertModelImpactsToWorld( nEnteringCount, FMesh_pModelToWorld );
					}
					else
					{
						_Ray_ConvertModelImpactsToWorldWithScale( nEnteringCount, FMesh_pModelToWorld );
					}

					if ( pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask )
					{
						break;
					}
				}
			}
		}
		else
		{
			// This mesh has bones but is not animating, so we cannot use the bone matrix palette
			CFMtx43A mtxInvBone;
			CFMtx43A mtxTransform;
			FMesh_pModelToWorld = &mtxTransform;

			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );
				
				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				// Get the segment and bone indices
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
				
				// Calculate the model to world matrix and bring the ray into bone space
				mtxInvBone.Mul( pMeshInst->m_Xfm.m_MtxF, pMeshInst->m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].AtRestBoneToModelMtx );
				mtxTransform.Set( mtxInvBone );
				FMesh_Coll_fBoneScale = mtxTransform.m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				mtxInvBone.AffineInvert_KnowOOScale2( FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				mtxInvBone.MulPoint( Test.vStart, pCollInfo->Ray.vStart_WS );
				mtxInvBone.MulPoint( Test.vEnd, _vTestRayEnd );
				mtxInvBone.MulDir( Test.vNormal, pCollInfo->Ray.vNormal_WS );
				_pWorldToModel = &mtxInvBone;

				// Set length and bone scale
				if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
				{
					FMesh_Coll_fBoneScaleR = 1.f;
					Test.fDistance = _fTestRayLength;
					Test.fOriginalDistance = pCollInfo->Ray.fLength;
				}
				else
				{
					Test.vNormal.Mul( FMesh_Coll_fBoneScale );
					Test.fDistance = _fTestRayLength * FMesh_Coll_fBoneScaleR;
					Test.fOriginalDistance = pCollInfo->Ray.fLength * FMesh_Coll_fBoneScaleR;
				}

				// Prime the node stack and start the test
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				nEnteringCount = FColl_nImpactCount;	
				if ( pCollInfo->bCullBacksideCollisions )
				{
					fmesh_Coll_TestRayWithDOPs_NoBF( &Test );
				}
				else
				{
					fmesh_Coll_TestRayWithDOPs_BF( &Test );
				}
			
				// If we have collisions, we need to convert them from model to world space
				if ( nEnteringCount < FColl_nImpactCount )
				{
					if ( FMesh_Coll_fBoneScaleR == 1.f )
					{
						_Ray_ConvertModelImpactsToWorld( nEnteringCount, &mtxTransform );
					}
					else
					{
						_Ray_ConvertModelImpactsToWorldWithScale( nEnteringCount, &mtxTransform );
					}

					if ( pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask )
					{
						break;
					}
				}
			}
		}
	}

	FColl_nImpactCollMask |= Fmesh_Coll_nAccumCollMask;
}

/*
//
//
//
void fmesh_Coll_TestCylinder( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo ) 
{
	FASSERT( pMeshInst && pCollInfo );

	// Capsule collision is not currently supported.  Use projected sphere 
	// or, if capsule is truly desired, speak with the engine team.
//	FASSERT( pCollInfo->nCollTestType != FMESH_COLLTESTTYPE_CAPSULE );

	u32 i;
	FMesh_Coll_CylinderTest_s Test;
	u32 nEnteringCount;
	FMesh_t *pMesh = pMeshInst->m_pMesh;

	// Verify that there is some cylinder height submitted
	if ( pCollInfo->Cylinder.fHeight < 0.0001f )
	{
		return;
	}

	// Make sure this mesh's mask matches
	if ( !(pMesh->nMeshCollMask & pCollInfo->nCollMask) )
	{
		return;
	}

	// Set the globals
#if !FANG_PRODUCTION_BUILD
	FMesh_Coll_nNodesChecked = 0;
#endif
	FMesh_Coll_pCurrMeshInst = pMeshInst;
	FMesh_Coll_pCurrCollInfo = pCollInfo;
	FMesh_Coll_nCurrBoneIndex = 0;
	Fmesh_Coll_nAccumCollMask = 0;

	u32 nStartingImpactCount = FColl_nImpactCount;
	
	if ( pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		// This mesh is a volume mesh so we can assume all data is in worldspace
		FASSERT( pMesh->nSegCount == 1 );

		// Set the test parameters
		Test.vStart.Set( pCollInfo->Cylinder.vStart_WS );
		Test.vEnd.Set( pCollInfo->Cylinder.vEnd_WS );
		Test.fRadius = pCollInfo->Cylinder.fRadius;
		Test.fRadiusSq = pCollInfo->Cylinder.fRadiusSq;
		Test.vNormal.Set( pCollInfo->Cylinder.vNormal_WS );
		Test.fHeight = pCollInfo->Cylinder.fHeight;
		Test.vCenterPoint.Set( pCollInfo->Cylinder.vCenterPoint_WS );
		Test.pMeshToCylinder = &pCollInfo->Cylinder.mtxInvOrient;
		Test.pMeshToCylinderNoScale = &pCollInfo->Cylinder.mtxInvOrient;
		Test.pCylinderToWorld = &pCollInfo->Cylinder.mtxOrient;
		Test.bWorldGeo = TRUE;

		FMesh_pModelToWorld = NULL;
		_pWorldToModel = NULL;
		FMesh_Coll_fBoneScale = FMesh_Coll_fBoneScaleR = 1.f;

		FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

		FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

		// Prime the node stack and start the test
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
		
		if ( pCollInfo->nCollTestType == FMESH_COLLTESTTYPE_CYLINDER )
		{
			fmesh_Coll_TestCylinderWithDOPs( &Test );
		}
		else
		{
			fmesh_Coll_TestCapsuleWithDOPs( &Test );
		}
	}
	else
	{
		// This object is not a volume mesh, so it could be oriented
		CFMtx43A mtxMeshToCylinder, mtxMeshToCylinderNoScale;
		
		Test.bWorldGeo = FALSE;
		Test.pMeshToCylinder = &mtxMeshToCylinder;
		Test.pMeshToCylinderNoScale = &mtxMeshToCylinderNoScale;
		Test.pCylinderToWorld = &pCollInfo->Cylinder.mtxOrient;

		if ( pMeshInst->m_pMesh->nBoneCount == 0 )
		{
			// This object has no bones so we don't check the bone matrix palette
			FASSERT( pMesh->nSegCount == 1 );

			FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

			FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

			// Set the test parameters
			pMeshInst->m_Xfm.m_MtxR.MulPoint( Test.vStart, pCollInfo->Cylinder.vStart_WS );
			pMeshInst->m_Xfm.m_MtxR.MulPoint( Test.vEnd, pCollInfo->Cylinder.vEnd_WS );
			pMeshInst->m_Xfm.m_MtxR.MulPoint( Test.vCenterPoint, pCollInfo->Cylinder.vCenterPoint_WS );
			pMeshInst->m_Xfm.m_MtxR.MulDir( Test.vNormal, pCollInfo->Cylinder.vNormal_WS );
			
			mtxMeshToCylinder.Mul( pCollInfo->Cylinder.mtxInvOrient, pMeshInst->m_Xfm.m_MtxF );

			FMesh_pModelToWorld = &pMeshInst->m_Xfm.m_MtxF;
			_pWorldToModel = &pMeshInst->m_Xfm.m_MtxR;

			// Set length and bone scale
			FMesh_Coll_fBoneScale = pMeshInst->m_Xfm.m_fScaleF;
			FMesh_Coll_fBoneScaleR = pMeshInst->m_Xfm.m_fScaleR;
			if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
			{
				FMesh_Coll_fBoneScaleR = 1.f;
				Test.fRadius = pCollInfo->Cylinder.fRadius;
				Test.fRadiusSq = pCollInfo->Cylinder.fRadiusSq;
				Test.fHeight = pCollInfo->Cylinder.fHeight;
				Test.pMeshToCylinderNoScale = &mtxMeshToCylinder;
			}
			else
			{
				Test.fRadius = pCollInfo->Cylinder.fRadius * FMesh_Coll_fBoneScaleR;
				Test.fRadiusSq = Test.fRadius * Test.fRadius;
				Test.fHeight = pCollInfo->Cylinder.fHeight * FMesh_Coll_fBoneScaleR;
				Test.vNormal.Mul( FMesh_Coll_fBoneScale );
				Test.pMeshToCylinderNoScale = &mtxMeshToCylinderNoScale;
				mtxMeshToCylinderNoScale.Mul( mtxMeshToCylinder, FMesh_Coll_fBoneScaleR );
			}

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
			nEnteringCount = FColl_nImpactCount;	
			if ( pCollInfo->nCollTestType == FMESH_COLLTESTTYPE_CYLINDER )
			{
				fmesh_Coll_TestCylinderWithDOPs( &Test );
			}
			else
			{
				fmesh_Coll_TestCapsuleWithDOPs( &Test );
			}
		}
		else if ( !(pMeshInst->m_nFlags & FMESHINST_FLAG_NOBONES) )
		{
			// This mesh has bones and IS animating so we can use the bone matrix palette
			CFMtx43A **apBoneMtxList, mtxInvBone;
			apBoneMtxList = pMeshInst->GetBoneMtxPalette();
			FASSERT( apBoneMtxList );
			
			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );

				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Bring the ray into bone space
				FMesh_pModelToWorld = apBoneMtxList[FMesh_Coll_nCurrBoneIndex];
				FMesh_Coll_fBoneScale = FMesh_pModelToWorld->m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				mtxInvBone.ReceiveAffineInverse_KnowOOScale2( *apBoneMtxList[FMesh_Coll_nCurrBoneIndex], FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				mtxInvBone.MulPoint( Test.vStart, pCollInfo->Cylinder.vStart_WS );
				mtxInvBone.MulPoint( Test.vEnd, pCollInfo->Cylinder.vEnd_WS );
				mtxInvBone.MulPoint( Test.vCenterPoint, pCollInfo->Cylinder.vCenterPoint_WS );
				mtxInvBone.MulDir( Test.vNormal, pCollInfo->Cylinder.vNormal_WS );
				_pWorldToModel = &mtxInvBone;

				mtxMeshToCylinder.Mul( pCollInfo->Cylinder.mtxInvOrient, *FMesh_pModelToWorld );

				// Set length and bone scale
				if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
				{
					FMesh_Coll_fBoneScaleR = 1.f;
					Test.fRadius = pCollInfo->Cylinder.fRadius;
					Test.fRadiusSq = pCollInfo->Cylinder.fRadiusSq;
					Test.fHeight = pCollInfo->Cylinder.fHeight;
					Test.pMeshToCylinderNoScale = &mtxMeshToCylinder;
				}
				else
				{
					Test.fRadius = pCollInfo->Cylinder.fRadius * FMesh_Coll_fBoneScaleR;
					Test.fRadiusSq = Test.fRadius * Test.fRadius;
					Test.fHeight = pCollInfo->Cylinder.fHeight * FMesh_Coll_fBoneScaleR;
					Test.vNormal.Mul( FMesh_Coll_fBoneScale );
					Test.pMeshToCylinderNoScale = &mtxMeshToCylinderNoScale;
					mtxMeshToCylinderNoScale.Mul( mtxMeshToCylinder, FMesh_Coll_fBoneScaleR );
				}

				// Prime the node stack and start the test
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				nEnteringCount = FColl_nImpactCount;	
				if ( pCollInfo->nCollTestType == FMESH_COLLTESTTYPE_CYLINDER )
				{
					fmesh_Coll_TestCylinderWithDOPs( &Test );
				}
				else
				{
					fmesh_Coll_TestCapsuleWithDOPs( &Test );
				}

				// If we have collisions, and we only want the first impact, bail
				if ( nEnteringCount < FColl_nImpactCount && (pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask) )
				{
					break;
				}
			}
		}
		else
		{
			// This mesh has bones but is not animating, so we cannot use the bone matrix palette
			CFMtx43A mtxInvBone;
			CFMtx43A mtxTransform;
			FMesh_pModelToWorld = &mtxTransform;

			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );
				
				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				// Get the segment and bone indices
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
				
				// Calculate the model to world matrix and bring the ray into bone space
				mtxInvBone.Mul( pMeshInst->m_Xfm.m_MtxF, pMeshInst->m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].AtRestBoneToModelMtx );
				mtxTransform.Set( mtxInvBone );
				FMesh_Coll_fBoneScale = mtxTransform.m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				mtxInvBone.AffineInvert_KnowOOScale2( FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				mtxInvBone.MulPoint( Test.vStart, pCollInfo->Cylinder.vStart_WS );
				mtxInvBone.MulPoint( Test.vEnd, pCollInfo->Cylinder.vEnd_WS );
				mtxInvBone.MulPoint( Test.vCenterPoint, pCollInfo->Cylinder.vCenterPoint_WS );
				mtxInvBone.MulDir( Test.vNormal, pCollInfo->Cylinder.vNormal_WS );
				_pWorldToModel = &mtxInvBone;

				mtxMeshToCylinder.Mul( pCollInfo->Cylinder.mtxInvOrient, *FMesh_pModelToWorld );

				// Set length and bone scale
				if ( fmath_Abs( 1.0f - FMesh_Coll_fBoneScaleR ) < 0.01f )
				{
					FMesh_Coll_fBoneScaleR = 1.f;
					Test.fRadius = pCollInfo->Cylinder.fRadius;
					Test.fRadiusSq = pCollInfo->Cylinder.fRadiusSq;
					Test.fHeight = pCollInfo->Cylinder.fHeight;
					Test.pMeshToCylinderNoScale = &mtxMeshToCylinder;
				}
				else
				{
					Test.fRadius = pCollInfo->Cylinder.fRadius * FMesh_Coll_fBoneScaleR;
					Test.fRadiusSq = Test.fRadius * Test.fRadius;
					Test.fHeight = pCollInfo->Cylinder.fHeight * FMesh_Coll_fBoneScaleR;
					Test.vNormal.Mul( FMesh_Coll_fBoneScale );
					Test.pMeshToCylinderNoScale = &mtxMeshToCylinderNoScale;
					mtxMeshToCylinderNoScale.Mul( mtxMeshToCylinder, FMesh_Coll_fBoneScaleR );
				}

				// Prime the node stack and start the test
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				nEnteringCount = FColl_nImpactCount;	
				if ( pCollInfo->nCollTestType == FMESH_COLLTESTTYPE_CYLINDER )
				{
					fmesh_Coll_TestCylinderWithDOPs( &Test );
				}
				else
				{
					fmesh_Coll_TestCapsuleWithDOPs( &Test );
				}
			
				// If we have collisions, and we're only interested in one, bail
				if ( nEnteringCount < FColl_nImpactCount && (pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask) )
				{
					break;
				}
			}
		}
	}
	
#if FMESH_DRAW_COLLIDED_TRIS
	if ( FMesh_Coll_bShowCollision )
	{
		for ( i = nStartingImpactCount; i < FColl_nImpactCount; i++ )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
			if ( FColl_aImpactBuf[i].fImpactDistInfo != FMATH_MAX_FLOAT )
			{
				fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
			}
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].vRadialImpact.v3 );
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].vHeightImpact.v3 );
		}
	}
#endif // FMESH_DRAW_COLLIDED_TRIS

	FColl_nImpactCollMask |= Fmesh_Coll_nAccumCollMask;
}
*/

//
//
//
FINLINE void _SetupTestValues( FMesh_Coll_ProjSphereTest_t *pTest, const CFMtx43A *pTransform, f32 fMtxScale, f32 &fInvMtxScale )
{
	// Bring the relevant data into the matrix space
	pTransform->MulPoint( pTest->vStart, FMesh_Coll_pCurrCollInfo->ProjSphere.m_vCenterStart_WS );
	pTransform->MulPoint( pTest->vEnd, FMesh_Coll_pCurrCollInfo->ProjSphere.m_vCenterEnd_WS );
	pTransform->MulDir( pTest->vMoveNormal, FMesh_Coll_pCurrCollInfo->ProjSphere.m_vMoveNormal_WS );
	pTransform->MulDir( pTest->vMoveDelta, FMesh_Coll_pCurrCollInfo->ProjSphere.m_vMoveDelta_WS );

	if ( fmath_Abs( 1.0f - fInvMtxScale ) < 0.01f )
	{
		// This matrix is not scaled, so we can accept the submitted values
		fInvMtxScale = 1.f;
		pTest->fRadius = FMesh_Coll_pCurrCollInfo->ProjSphere.m_fRadius;
		pTest->fRadiusSq = FMesh_Coll_pCurrCollInfo->ProjSphere.m_fRadiusSq;
		pTest->fMoveDistance = FMesh_Coll_pCurrCollInfo->ProjSphere.m_fMoveDistance;
		return;
	}

	// The transform matrix is scaled so we need to adjust the values to reflect this
	pTest->fRadius = FMesh_Coll_pCurrCollInfo->ProjSphere.m_fRadius * fInvMtxScale;
	pTest->fRadiusSq = pTest->fRadius * pTest->fRadius;
	pTest->fMoveDistance = FMesh_Coll_pCurrCollInfo->ProjSphere.m_fMoveDistance * fInvMtxScale;

	// Scale the normal back so it is unit length
	pTest->vMoveNormal.Mul( fMtxScale );
}


//
//
//
void fmesh_Coll_TestProjSphere( const CFMeshInst *pMeshInst, const CFCollInfo *pCollInfo ) 
{
	FASSERT( pMeshInst && pCollInfo );

	u32 i;
	FMesh_Coll_ProjSphereTest_t Test;
	u32 nEnteringCount;
	FMesh_t *pMesh = pMeshInst->m_pMesh;

	// Verify that there is some cylinder height submitted
	if ( pCollInfo->ProjSphere.m_fMoveDistance < 0.0001f )
	{
		return;
	}

	// Make sure this mesh's mask matches
	if ( !(pMesh->nMeshCollMask & pCollInfo->nCollMask) )
	{
		return;
	}

	// If the caller requested closest impact, only, perform some optimizations
	if ( pCollInfo->bFindClosestImpactOnly )
	{
		if ( FColl_nMaxCollImpacts == FColl_nImpactCount )
		{
			return;
		}

		// Setup the next impact to receive the closest impact
		FMesh_Coll_pClosestImpactSlot = NULL;
	}

	// Set the globals
#if !FANG_PRODUCTION_BUILD
	FMesh_Coll_nNodesChecked = 0;
#endif
	FMesh_Coll_pCurrMeshInst = pMeshInst;
	FMesh_Coll_pCurrCollInfo = pCollInfo;
	FMesh_Coll_nCurrBoneIndex = 0;
	Fmesh_Coll_nAccumCollMask = 0;

	u32 nStartingImpactCount = FColl_nImpactCount;
	
	if ( pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		// This mesh is a volume mesh so we can assume all data is in worldspace
		FASSERT( pMesh->nSegCount == 1 );

		// Set the test parameters
		Test.vStart.Set( pCollInfo->ProjSphere.m_vCenterStart_WS );
		Test.vEnd.Set( pCollInfo->ProjSphere.m_vCenterEnd_WS );
		Test.fRadius = pCollInfo->ProjSphere.m_fRadius;
		Test.fRadiusSq = pCollInfo->ProjSphere.m_fRadiusSq;
		Test.vMoveNormal.Set( pCollInfo->ProjSphere.m_vMoveNormal_WS );
		Test.vMoveDelta.Set( pCollInfo->ProjSphere.m_vMoveDelta_WS );
		Test.fMoveDistance = pCollInfo->ProjSphere.m_fMoveDistance;

		FMesh_pModelToWorld = NULL;
		_pWorldToModel = NULL;
		FMesh_Coll_fBoneScale = FMesh_Coll_fBoneScaleR = 1.f;

		FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

		FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

		// Prime the node stack and start the test
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
		
		fmesh_Coll_TestProjSphereWithDOPs( &Test );
	}
	else
	{
		// This object is not a volume mesh, so it could be oriented
		
		if ( pMeshInst->m_pMesh->nBoneCount == 0 )
		{
			// This object has no bones so we don't check the bone matrix palette
			FASSERT( pMesh->nSegCount == 1 );

			FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

			FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

			FMesh_pModelToWorld = &pMeshInst->m_Xfm.m_MtxF;
			_pWorldToModel = &pMeshInst->m_Xfm.m_MtxR;

			// Set bone scale
			FMesh_Coll_fBoneScale = pMeshInst->m_Xfm.m_fScaleF;
			FMesh_Coll_fBoneScaleR = pMeshInst->m_Xfm.m_fScaleR;
			
			// Set the test parameters
			_SetupTestValues( &Test, &pMeshInst->m_Xfm.m_MtxR, FMesh_Coll_fBoneScale, FMesh_Coll_fBoneScaleR );

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
			nEnteringCount = FColl_nImpactCount;	
			
			fmesh_Coll_TestProjSphereWithDOPs( &Test );

/*
			// If we have collisions, we need to convert them from model to world space
			if ( nEnteringCount < FColl_nImpactCount && FMesh_Coll_pCurrCollInfo->bCalculateImpactData )
			{
				if ( FMesh_Coll_fBoneScaleR == 1.f )
				{
					_ProjSphere_ConvertModelImpactsToWorld( nEnteringCount, FMesh_pModelToWorld );
				}
				else
				{
					_ProjSphere_ConvertModelImpactsToWorldWithScale( nEnteringCount, FMesh_pModelToWorld );
				}
			}
*/
		}
		else if ( !(pMeshInst->m_nFlags & FMESHINST_FLAG_NOBONES) )
		{
			// This mesh has bones and IS animating so we can use the bone matrix palette
			CFMtx43A **apBoneMtxList, mtxInvBone;
			apBoneMtxList = pMeshInst->GetBoneMtxPalette();
			FASSERT( apBoneMtxList );
			
			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );

				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;

				// Get the segment and bone indices
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Setup bone scale and matrices
				FMesh_pModelToWorld = apBoneMtxList[FMesh_Coll_nCurrBoneIndex];
				FMesh_Coll_fBoneScale = FMesh_pModelToWorld->m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				mtxInvBone.ReceiveAffineInverse_KnowOOScale2( *apBoneMtxList[FMesh_Coll_nCurrBoneIndex], FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				_pWorldToModel = &mtxInvBone;

				// Set the test parameters
				_SetupTestValues( &Test, &mtxInvBone, FMesh_Coll_fBoneScale, FMesh_Coll_fBoneScaleR );

				// Prime the node stack and start the test
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;

				// If we have a collision, see if we should bail
				if ( fmesh_Coll_TestProjSphereWithDOPs( &Test ) && (pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask) )
				{
					break;
				}
			}
		}
		else
		{
			// This mesh has bones but is not animating, so we cannot use the bone matrix palette
			CFMtx43A mtxInvBone;
			CFMtx43A mtxTransform;
			FMesh_pModelToWorld = &mtxTransform;

			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				FASSERT( FMesh_Coll_pCurrkDOPTree );
				
				// Make sure this tree's mask matches
				if ( !(FMesh_Coll_pCurrkDOPTree->nMasterCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
				{
					continue;
				}

				// Get the root node in the kDOP tree
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
				
				// Get the segment and bone indices
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];

				// Make sure this is part of the active parts mask
				if ( !(pMeshInst->Parts_GetDrawnPartsMask() & pMeshInst->Parts_GetCollPartsMask() & (1 << pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].nPartID)) )
				{
					// The bone is not part of the active parts so skip it
					continue;
				}

				// Setup bone scale and matrices
				mtxInvBone.Mul( pMeshInst->m_Xfm.m_MtxF, pMeshInst->m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].AtRestBoneToModelMtx );
				mtxTransform.Set( mtxInvBone );
				FMesh_Coll_fBoneScale = mtxTransform.m_vFront.MagSq();
				if ( FMesh_Coll_fBoneScale < 0.00001f )
				{
					continue;
				}

				FMesh_Coll_fBoneScaleR = fmath_InvSqrt( FMesh_Coll_fBoneScale );
				FMesh_Coll_fBoneScale = fmath_Inv( FMesh_Coll_fBoneScaleR );
				mtxInvBone.AffineInvert_KnowOOScale2( FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				_pWorldToModel = &mtxInvBone;

				// Set the test parameters
				_SetupTestValues( &Test, &mtxInvBone, FMesh_Coll_fBoneScale, FMesh_Coll_fBoneScaleR );

				// Prime the node stack and start the test
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;

				// If we have collisions, see if we need to bail
				if ( fmesh_Coll_TestProjSphereWithDOPs( &Test ) && (pCollInfo->nStopOnFirstOfCollMask & Fmesh_Coll_nAccumCollMask) )
				{
					break;
				}
			}
		}
	}
	
#if FMESH_DRAW_COLLIDED_TRIS
	if ( FMesh_Coll_bShowCollision )
	{
		for ( i = nStartingImpactCount; i < FColl_nImpactCount; i++ )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
			if ( FColl_aImpactBuf[i].fImpactDistInfo != FMATH_MAX_FLOAT )
			{
				fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
			}
//			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].vRadialImpact.v3 );
//			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].vHeightImpact.v3 );
		}
	}
#endif // FMESH_DRAW_COLLIDED_TRIS

	FColl_nImpactCollMask |= Fmesh_Coll_nAccumCollMask;
}


//
//
//
static void _Sphere_ConvertModelImpactsToWorld( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;
	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal );
		pMtx->MulDir( FColl_aImpactBuf[i].PushUnitVec );

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
		}
#endif
	}
}


//
//
//
static void _Sphere_ConvertModelImpactsToWorldWithScale( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;
	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal.Mul( FMesh_Coll_fBoneScaleR ) );
		pMtx->MulDir( FColl_aImpactBuf[i].PushUnitVec.Mul( FMesh_Coll_fBoneScaleR ) );
		FColl_aImpactBuf[i].fImpactDistInfo *= FMesh_Coll_fBoneScale;

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
		}
#endif
	}
}


//
//
//
static void _Ray_ConvertModelImpactsToWorld( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;

	if ( FMesh_Coll_pCurrCollInfo->bFindClosestImpactOnly )
	{
		// If the client is only interested in the closest impact, we can discard
		// all others and save us some calculation time.  Since we clip the ray
		// as we detect collision, we always know that the last collision is the
		// closest colision
		u32 nReplaceIdx = FColl_nImpactCount - 1;
		pMtx->MulPoint( FColl_aImpactBuf[nStart].aTriVtx[0], FColl_aImpactBuf[nReplaceIdx].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[nStart].aTriVtx[1], FColl_aImpactBuf[nReplaceIdx].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[nStart].aTriVtx[2], FColl_aImpactBuf[nReplaceIdx].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[nStart].ImpactPoint, FColl_aImpactBuf[nReplaceIdx].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[nStart].UnitFaceNormal, FColl_aImpactBuf[nReplaceIdx].UnitFaceNormal );

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[nStart].aTriVtx, NULL );
		}
#endif
		// Reset the current vars used for checks
		_vTestRayEnd.Set( FColl_aImpactBuf[nStart].ImpactPoint );
		// All dist info calculations (unit distance) is relative to the original length
		_fTestRayLength = FMesh_Coll_pCurrCollInfo->Ray.fLength * FColl_aImpactBuf[nStart].fImpactDistInfo;

		// Set the impact count back to the original plus one
		FColl_pNextImpactSlot = &FColl_aImpactBuf[nStart + 1];
		FColl_nImpactCount = nStart + 1;
		return;
	}

	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal );

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
		}
#endif
	}

}


//
//
//
static void _Ray_ConvertModelImpactsToWorldWithScale( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;

	if ( FMesh_Coll_pCurrCollInfo->bFindClosestImpactOnly )
	{
		// If the client is only interested in the closest impact, we can discard
		// all others and save us some calculation time.  Since we clip the ray
		// as we detect collision, we always know that the last collision is the
		// closest colision
		u32 nReplaceIdx = FColl_nImpactCount - 1;
		pMtx->MulPoint( FColl_aImpactBuf[nStart].aTriVtx[0], FColl_aImpactBuf[nReplaceIdx].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[nStart].aTriVtx[1], FColl_aImpactBuf[nReplaceIdx].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[nStart].aTriVtx[2], FColl_aImpactBuf[nReplaceIdx].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[nStart].ImpactPoint, FColl_aImpactBuf[nReplaceIdx].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[nStart].UnitFaceNormal, FColl_aImpactBuf[nReplaceIdx].UnitFaceNormal.Mul( FMesh_Coll_fBoneScaleR ) );

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[nStart].aTriVtx, NULL );
		}
#endif
		// Reset the current vars used for checks
		_vTestRayEnd.Set( FColl_aImpactBuf[nStart].ImpactPoint );
		// All dist info calculations (unit distance) is relative to the original length
		_fTestRayLength = FMesh_Coll_pCurrCollInfo->Ray.fLength * FColl_aImpactBuf[nStart].fImpactDistInfo;

		// Set the impact count back to the original plus one
		FColl_pNextImpactSlot = &FColl_aImpactBuf[nStart + 1];
		FColl_nImpactCount = nStart + 1;
		return;
	}

	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal.Mul( FMesh_Coll_fBoneScaleR ) );

#if FMESH_DRAW_COLLIDED_TRIS	
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
		}
#endif
	}
}


//
//
//
static void _ProjSphere_ConvertModelImpactsToWorld( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;

	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal );

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
		}
#endif
	}

}


//
//
//
static void _ProjSphere_ConvertModelImpactsToWorldWithScale( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;

	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal.Mul( FMesh_Coll_fBoneScaleR ) );

#if FMESH_DRAW_COLLIDED_TRIS	
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
		}
#endif
	}
}


//
//
//
static void _kDOP_ConvertModelImpactsToWorld( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;
	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal );
		pMtx->MulDir( FColl_aImpactBuf[i].PushUnitVec );

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
		}
#endif
	}
}


//
//
//
static void _kDOP_ConvertModelImpactsToWorldWithScale( u32 nStart, const CFMtx43A *pMtx )
{
	u32 i;
	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		pMtx->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		pMtx->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		pMtx->MulDir( FColl_aImpactBuf[i].UnitFaceNormal.Mul( FMesh_Coll_fBoneScaleR ) );
		pMtx->MulDir( FColl_aImpactBuf[i].PushUnitVec.Mul( FMesh_Coll_fBoneScaleR ) );
		FColl_aImpactBuf[i].fImpactDistInfo *= FMesh_Coll_fBoneScale;

#if FMESH_DRAW_COLLIDED_TRIS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawTri( FColl_aImpactBuf[i].aTriVtx, NULL );
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3 );
		}
#endif
	}
}


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//	Drawing routines used to debug collision
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
//
//
//
inline void _DrawMeshTri( const CFVec3 *pVertArray, const CFVec3 *pNormal, CFMtx43A *pTransform, CFColorRGBA *pColor, BOOL bLight )
{
	CFVec3A vVert[3], vNormal;
	vVert[0].Set( pVertArray[0] );
	vVert[1].Set( pVertArray[1] );
	vVert[2].Set( pVertArray[2] );
	vNormal.Set( *pNormal );
	fmesh_Coll_DrawMeshTri( vVert, &vNormal, pTransform, pColor, bLight );
}


//
//
//
inline void _BeginTriDraw( void )
{
	// Setup fdraw for rendering
	frenderer_Push( FRENDERER_DRAW, NULL );

	fdraw_SetTexture( NULL );

	// Set up an identity Xfm to push
	_xfmIdentity.Identity();
	_xfmIdentity.PushModel();

	// Set the states we need
	fdraw_SetCullDir( FDRAW_CULLDIR_CCW );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_Depth_EnableWriting( TRUE );

	_nDrawTriCount = 0;
}


//
//
//
inline void _FlushTriDraw( void )
{	
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _vTriVerts, _nDrawTriCount * 3 );
//	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_Depth_SetBiasLevel( 1 );
	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, _vLineVerts, _nDrawTriCount * 6 );
	_nDrawTriCount = 0;
}


//
//
//
inline void _EndTriDraw( void )
{	
	if ( _nDrawTriCount )
	{
		_FlushTriDraw();
	}

	// Pop the model matrix	
	_xfmIdentity.PopModel();
	
	// Pop the render
	frenderer_Pop();
}


//
//
//
void fmesh_Coll_DrawMeshTri( const CFVec3A *pVertArray, const CFVec3A *pNormal, const CFMtx43A *pTransform, CFColorRGBA *pColor, BOOL bLight )
{
	if ( _nDrawTriCount == _MAX_TRIS_IN_BUFFER )
	{
		// Setup fdraw for rendering
		frenderer_Push( FRENDERER_DRAW, NULL );

		fdraw_SetTexture( NULL );

		// Set up an identity Xfm to push
		_xfmIdentity.Identity();
		_xfmIdentity.PushModel();

		// Set the states we need
		fdraw_SetCullDir( FDRAW_CULLDIR_CCW );
		fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
		fdraw_Depth_EnableWriting( TRUE );

		_xfmIdentity.PopModel();

		_FlushTriDraw();

		frenderer_Pop();
	}

	FDrawVtx_t *pTriVerts  = &_vTriVerts[_nDrawTriCount * 3];
	FDrawVtx_t *pLineVerts = &_vLineVerts[_nDrawTriCount * 6];
	_nDrawTriCount++;
	
	// Calculate color by simulating a light
	if ( bLight )
	{
		CFVec3A vDefaultLight( 0.57735f, -0.57735f, -0.57735f );
		f32 fMod = -pNormal->Dot( vDefaultLight ) * 0.65f;
		if ( fMod < 0.f )
		{
			fMod = 0.f;
		}
		fMod += 0.35f;
		FMATH_CLAMP( fMod, 0.f, 1.f );
		pColor->fRed *= fMod;
		pColor->fGreen *= fMod;
		pColor->fBlue *= fMod;
	}
	if ( pTransform )
	{
		CFVec3A vTemp;
		pTriVerts[0].Pos_MS = pTransform->MulPoint( vTemp, pVertArray[0] ).v3;
		pTriVerts[1].Pos_MS = pTransform->MulPoint( vTemp, pVertArray[1] ).v3;
		pTriVerts[2].Pos_MS = pTransform->MulPoint( vTemp, pVertArray[2] ).v3;
	}
	else
	{
		pTriVerts[0].Pos_MS = pVertArray[0].v3;
		pTriVerts[1].Pos_MS = pVertArray[1].v3;
		pTriVerts[2].Pos_MS = pVertArray[2].v3;
	}
	pTriVerts[0].ColorRGBA.Set( *pColor );
	pTriVerts[1].ColorRGBA.Set( *pColor );
	pTriVerts[2].ColorRGBA.Set( *pColor );

	pLineVerts[0].Pos_MS = pTriVerts[0].Pos_MS;
	pLineVerts[0].ColorRGBA.Set( 0.75, 0.75, 0, 1.f );
	pLineVerts[1].Pos_MS = pTriVerts[1].Pos_MS;
	pLineVerts[1].ColorRGBA = pLineVerts[0].ColorRGBA;

	pLineVerts[2].Pos_MS = pTriVerts[1].Pos_MS;
	pLineVerts[2].ColorRGBA = pLineVerts[0].ColorRGBA;
	pLineVerts[3].Pos_MS = pTriVerts[2].Pos_MS;
	pLineVerts[3].ColorRGBA = pLineVerts[0].ColorRGBA;

	pLineVerts[4].Pos_MS = pTriVerts[2].Pos_MS;
	pLineVerts[4].ColorRGBA = pLineVerts[0].ColorRGBA;
	pLineVerts[5].Pos_MS = pTriVerts[0].Pos_MS;
	pLineVerts[5].ColorRGBA = pLineVerts[0].ColorRGBA;
}


//
//
//
void _AddCollDrawTri( const CFVec3 *pVertArray, const CFMtx43A *pTransform )
{
	CFVec3A vVerts[3];
	vVerts[0].Set( pVertArray[0] );
	vVerts[1].Set( pVertArray[1] );
	vVerts[2].Set( pVertArray[2] );
	fmesh_Coll_AddCollDrawTri( vVerts, pTransform );
}

//
//
//
void fmesh_Coll_AddCollDrawTri( const CFVec3A *pVertArray, const CFMtx43A *pTransform )
{
	if ( _nDrawCollTriCount == _MAX_COLL_TRIS_IN_BUFFER )
	{
		return;
	}

	FDrawVtx_t *pTriVerts  = &_vCollTriVerts[_nDrawCollTriCount * 3];
	FDrawVtx_t *pLineVerts = &_vCollLineVerts[_nDrawCollTriCount * 6];
	_nDrawCollTriCount++;

	if ( pTransform )
	{
		CFVec3A vTemp;
		pTriVerts[0].Pos_MS = pTransform->MulPoint( vTemp, pVertArray[0] ).v3;
		pTriVerts[1].Pos_MS = pTransform->MulPoint( vTemp, pVertArray[1] ).v3;
		pTriVerts[2].Pos_MS = pTransform->MulPoint( vTemp, pVertArray[2] ).v3;
	}
	else
	{
		pTriVerts[0].Pos_MS = pVertArray[0].v3;
		pTriVerts[1].Pos_MS = pVertArray[1].v3;
		pTriVerts[2].Pos_MS = pVertArray[2].v3;
	}
	pTriVerts[0].ColorRGBA.Set( 1.f, 1.f, 1.f, 0.8f );
	pTriVerts[1].ColorRGBA.Set( 1.f, 1.f, 1.f, 0.8f );
	pTriVerts[2].ColorRGBA.Set( 1.f, 1.f, 1.f, 0.8f );

	pLineVerts[0].Pos_MS = pTriVerts[0].Pos_MS;
	pLineVerts[0].ColorRGBA.Set( 0.75, 0.75, 0, 1.f );
	pLineVerts[1].Pos_MS = pTriVerts[1].Pos_MS;
	pLineVerts[1].ColorRGBA = pLineVerts[0].ColorRGBA;

	pLineVerts[2].Pos_MS = pTriVerts[1].Pos_MS;
	pLineVerts[2].ColorRGBA = pLineVerts[0].ColorRGBA;
	pLineVerts[3].Pos_MS = pTriVerts[2].Pos_MS;
	pLineVerts[3].ColorRGBA = pLineVerts[0].ColorRGBA;

	pLineVerts[4].Pos_MS = pTriVerts[2].Pos_MS;
	pLineVerts[4].ColorRGBA = pLineVerts[0].ColorRGBA;
	pLineVerts[5].Pos_MS = pTriVerts[0].Pos_MS;
	pLineVerts[5].ColorRGBA = pLineVerts[0].ColorRGBA;
}


//
//
//
void fmesh_Coll_AddCollDrawDOP( const FkDOP_Interval_t *pIntervals, const CFMtx43A *pMtx, u32 nkDOPType )
{
	if ( _nkDOPDrawCount == _MAX_KDOP_DRAWS )
	{
		return;
	}

	_apkDOPIntervals[ _nkDOPDrawCount ] = pIntervals;
	if ( pMtx )
	{
		_aMtxkDOPTransform[ _nkDOPDrawCount ].Set( *pMtx );
	}
	else
	{
		_aMtxkDOPTransform[ _nkDOPDrawCount ].Identity();
	}

	_ankDOPType[ _nkDOPDrawCount ] = nkDOPType;

	_nkDOPDrawCount++;
}

//
//
//
void fmesh_Coll_AddCollDrawImpact( const CFVec3 *pImpact, u8 nRed, u8 nGreen, u8 nBlue )
{
	if ( _nDrawImpactCount == _MAX_COLL_TRIS_IN_BUFFER )
	{
		return;
	}

	_vImpactPoint[_nDrawImpactCount].Set( *pImpact );
	_nImpactColor[_nDrawImpactCount] = (nRed << 24) + (nGreen << 16) + (nBlue << 8 ) + 255;
	_nDrawImpactCount++;
}


//
//
//
void fmesh_Coll_DrawCollisions( void )
{
//	_BeginTriDraw();

	// Setup fdraw for rendering
	frenderer_Push( FRENDERER_DRAW, NULL );

	// Set up an identity Xfm to push
	_xfmIdentity.Identity();
	_xfmIdentity.PushModel();

	// Set the states we need
	fdraw_SetCullDir( FDRAW_CULLDIR_CCW );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_Depth_EnableWriting( TRUE );
	
	_FlushTriDraw();

//	fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _vCollTriVerts, _nDrawCollTriCount * 3 );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
	fdraw_Depth_SetBiasLevel( 1 );
	fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, _vCollLineVerts, _nDrawCollTriCount * 6 );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );

	u32 i;
	for ( i = 0; i < _nkDOPDrawCount; i++ )
	{
		FkDOP_DrawkDOP( _apkDOPIntervals[i], &_aMtxkDOPTransform[i], _ankDOPType[i] );
	}

	CFColorRGBA Color;
	
	for ( i = 0; i < _nDrawImpactCount; i++  )
	{
		Color.Set(  (f32)((_nImpactColor[i] & 0xff000000) >> 24) / 255.f,
					(f32)((_nImpactColor[i] & 0x00ff0000) >> 16) / 255.f,
					(f32)((_nImpactColor[i] & 0x0000ff00) >>  8) / 255.f,
					1.f );
		fdraw_FacetedWireSphere( &_vImpactPoint[i].v3, 0.1f, &Color );
	}

	_nkDOPDrawCount = 0;
	_nDrawImpactCount = 0;
	_nDrawCollTriCount = 0;

	_EndTriDraw();
}


//
//
//
void fmesh_Coll_DrawCollGeo( CFMeshInst *pMeshInst, u32 nDrawFlags )
{
	u32 i;
	FMesh_t *pMesh = pMeshInst->m_pMesh;
	if ( !pMesh->paCollTree )
	{
		return;
	}

#if !FANG_PRODUCTION_BUILD
	FMesh_Coll_nNodesChecked = 0;
#endif
	FMesh_Coll_pCurrMeshInst = pMeshInst;
	FMesh_Coll_nCurrBoneIndex = 0;
	
	_BeginTriDraw();	
	
	if ( pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		FASSERT( pMesh->nSegCount == 1 );
		
		FMesh_pModelToWorld = NULL;
		_pWorldToModel = NULL;
		FMesh_Coll_fBoneScaleR = 1.f;
		FMesh_Coll_fBoneScale = 1.f;
		
		FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

		// Draw the Geo
		FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
		fmesh_Coll_WalkTreeAndDraw( nDrawFlags, NULL );
	}
	else
	{
		CFMtx43A mtxTransform;
		FMesh_pModelToWorld = &mtxTransform;

		if ( pMeshInst->m_pMesh->nBoneCount == 0 )
		{
			FASSERT( pMesh->nSegCount == 1 );
		
			FMesh_Coll_pCurrkDOPTree = pMesh->paCollTree;

			// Calculate bone transform matrix
			mtxTransform.Set( pMeshInst->m_Xfm.m_MtxF );
			_pWorldToModel = &pMeshInst->m_Xfm.m_MtxR;
			FMesh_Coll_fBoneScaleR = 1.f;
			FMesh_Coll_fBoneScale = 1.f;
			
			// Draw the geo
			FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
			fmesh_Coll_WalkTreeAndDraw( nDrawFlags, &mtxTransform );
		}
		else if ( !(pMeshInst->m_nFlags & FMESHINST_FLAG_NOBONES) )
		{
			CFMtx43A **apBoneMtxList, mtxInvBone;
			apBoneMtxList = pMeshInst->GetBoneMtxPalette();
			FASSERT( apBoneMtxList );
			
			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				if ( !FMesh_Coll_pCurrkDOPTree )
				{
					continue;
				}
				
				// Get bone index
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];
				
				// Calculate bone transform matrix
				mtxTransform.Set( *apBoneMtxList[FMesh_Coll_nCurrBoneIndex] );
				FMesh_Coll_fBoneScale = mtxTransform.m_vFront.Mag();
				FMesh_Coll_fBoneScaleR = fmath_Inv( FMesh_Coll_fBoneScale );
				mtxInvBone.ReceiveAffineInverse_KnowOOScale2( *apBoneMtxList[FMesh_Coll_nCurrBoneIndex], FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				_pWorldToModel = &mtxInvBone;

				// Draw the geo
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				fmesh_Coll_WalkTreeAndDraw( nDrawFlags, &mtxTransform );
			}
		}
		else
		{
			CFMtx43A mtxInvBone;
			
			for ( i = 0; i < pMeshInst->m_pMesh->nCollTreeCount; i++ )	
			{
				FMesh_Coll_pCurrkDOPTree = &pMesh->paCollTree[i];
				if ( !FMesh_Coll_pCurrkDOPTree )
				{
					continue;
				}

				// Get bone index
				_nCurrSegmentColl = FMesh_Coll_pCurrkDOPTree->nSegmentIdx;
				FMesh_Coll_nCurrBoneIndex = pMesh->aSeg[_nCurrSegmentColl].anBoneMtxIndex[0];
				
				// Set bone transform matrix
				mtxInvBone.Mul( pMeshInst->m_Xfm.m_MtxF, pMeshInst->m_pMesh->pBoneArray[FMesh_Coll_nCurrBoneIndex].AtRestBoneToModelMtx );
				mtxTransform.Set( mtxInvBone );
				FMesh_Coll_fBoneScale = mtxTransform.m_vFront.Mag();
				FMesh_Coll_fBoneScaleR = fmath_Inv( FMesh_Coll_fBoneScale );
				mtxInvBone.AffineInvert_KnowOOScale2( FMesh_Coll_fBoneScaleR * FMesh_Coll_fBoneScaleR );
				_pWorldToModel = &mtxInvBone;

				// Test it for intersection				
				FkDOP_Node_t *pNode = FMesh_Coll_pCurrkDOPTree->pakDOPNodes;
				FMesh_Coll_nNodeStackIdx = 0;
				FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = pNode;
				fmesh_Coll_WalkTreeAndDraw( nDrawFlags, &mtxTransform );
			}
		}
	}

	_EndTriDraw();
}

#endif // !FANG_PRODUCTION_BUILD


//
//
FMesh_Coll_TransformedkDOP_t* fmesh_Coll_CalculateDOPinWS( CFMeshInst *pMeshInst, u16 nTreeIdx, const CFMtx43A *pMeshMtx )
{
	FASSERT( pMeshInst && pMeshInst->m_pMesh );

	const FkDOP_Tree_t *pCollTree = &pMeshInst->m_pMesh->paCollTree[nTreeIdx];
	FASSERT( pCollTree && pCollTree->pakDOPNodes );

	if ( pMeshInst->m_pMesh->nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		return NULL;
	}

	// Do we have kDOP verts	
	if ( !pCollTree->paRootkDOPVerts )
	{
		// Without kDOP verts, we cannot calculate the new kDOP		
		return NULL;
	}

	FMesh_Coll_TransformedkDOP_t *pTranskDOP = NULL;

	// Get the last used kDOP in the ring buffer
	if ( pMeshInst->m_nkDOPBufferStartIdx != 0xffff )
	{
		FASSERT( pMeshInst->m_nkDOPBufferStartIdx + nTreeIdx <= FKDOP_TRANSFORMED_KDOP_BUFFER_SIZE );
		pTranskDOP = &FKDOP_paTransformedkDOPBuffer[pMeshInst->m_nkDOPBufferStartIdx + nTreeIdx];

		if ( pTranskDOP->pOwner != pCollTree )
		{
			pTranskDOP = NULL;
		}
		else if ( pTranskDOP->nGenerationFrame == FVid_nFrameCounter )
		{
			// This kDOP has already been transformed, this frame so there is no need to transform it again
//			return pTranskDOP;
		}
	}

	if ( !pTranskDOP )
	{
		// This kDOP has not been transformed before or the prior transformed
		// kDOP is no longer owned by this node, so we need to get a new
		// transformed kDOP index
		if ( FKDOP_nTransformedkDOPIdx + pMeshInst->m_pMesh->nCollTreeCount > FKDOP_TRANSFORMED_KDOP_BUFFER_SIZE )
		{
			FKDOP_nTransformedkDOPIdx = 0;
			if ( FKDOP_nTransformedkDOPIdx + pMeshInst->m_pMesh->nCollTreeCount > FKDOP_TRANSFORMED_KDOP_BUFFER_SIZE )
			{
				FASSERT_NOW;
				return NULL;
			}
		}

		pMeshInst->m_nkDOPBufferStartIdx = FKDOP_nTransformedkDOPIdx;
		pTranskDOP = &FKDOP_paTransformedkDOPBuffer[FKDOP_nTransformedkDOPIdx];
		u32 i;
		for ( i = FKDOP_nTransformedkDOPIdx; i < FKDOP_nTransformedkDOPIdx + pCollTree->nTreeNodeCount; i++ )
		{
			pTranskDOP->pOwner = pCollTree;
			pTranskDOP->nGenerationFrame = 0xffffffff;
		}

		FKDOP_nTransformedkDOPIdx += pMeshInst->m_pMesh->nCollTreeCount;
	}

	// Transform the DOP --- THIS IS COSTLY CODE SO AVOID IT IF AT ALL POSSIBLE!
	fmesh_Coll_GeneratekDOPIntervals( pTranskDOP, pMeshInst, pCollTree, pMeshMtx );

	return pTranskDOP;
}


//
//
//
void fmesh_Coll_GeneratekDOPIntervals( FMesh_Coll_TransformedkDOP_t *pTransDOP, const CFMeshInst *pMeshInst, const FkDOP_Tree_t *pCollTree, const CFMtx43A *pMeshMtx/*=NULL*/, const CFMtx43A *pWorldToBone/*=NULL*/ )
{
	FASSERT( pTransDOP && pCollTree && pMeshInst );
	
#if !FANG_PRODUCTION_BUILD
	FMesh_Coll_nDOPTransforms++;
#endif	

	const CFMtx43A *pBone;
	static CFMtx43A mtxBone, mtxBoneToBone;
	u16 i, nBoneIdx, nVertIdx = 0;
	
	// Determine transform matrix to get the kDOP into WS	
	FMesh_t *pMesh = pMeshInst->m_pMesh;
	if ( pMesh->nUsedBoneCount == 0 )
	{
		if ( pMeshMtx )
		{
			pBone = pMeshMtx;
		}
		else
		{
			pBone = &pMeshInst->m_Xfm.m_MtxF;
		}
	}
	else
	{
		nBoneIdx = pMesh->aSeg[pCollTree->nSegmentIdx].anBoneMtxIndex[0];
		if ( pMeshInst->m_nFlags & FMESHINST_FLAG_NOBONES ) 
		{
			// This object is not animating
			pBone = &mtxBone;
			if ( pMeshMtx )
			{
				mtxBone.Mul( *pMeshMtx, pMesh->pBoneArray[nBoneIdx].AtRestBoneToModelMtx );
			}
			else
			{
				mtxBone.Mul( pMeshInst->m_Xfm.m_MtxF, pMesh->pBoneArray[nBoneIdx].AtRestBoneToModelMtx );
			}
		}
		else
		{
			FASSERT( pMeshInst->GetBoneMtxPalette() );
			pBone = pMeshInst->GetBoneMtxPalette()[nBoneIdx];
		}
	}

	// If the requestor provided a world to bone matrix, then we should add that in:	
	if ( pWorldToBone )
	{
		mtxBoneToBone.Mul( *pWorldToBone, *pBone );
		pBone = &mtxBoneToBone;
	}
	
	// Transform the kDOP verts
	for ( nVertIdx = 0; nVertIdx < pCollTree->nRootkDOPVertCount; nVertIdx++ )
	{
		pBone->MulPoint( pTransDOP->avTransVerts[nVertIdx], pCollTree->paRootkDOPVerts[nVertIdx] );
	}

	for ( i = 0; i < FkDOP_aDesc[pCollTree->nTreekDOPType].nNormalCount; i++ )
	{
		pBone->MulDir( pTransDOP->aTransAxes[i], FkDOP_aDesc[pCollTree->nTreekDOPType].avNormals[i] );
		f32 fDot = pBone->m_vPos.Dot( pTransDOP->aTransAxes[i] );
		pTransDOP->aTransIntervals[i].fMin = pCollTree->paIntervals[i].fMin + fDot;
		pTransDOP->aTransIntervals[i].fMax = pCollTree->paIntervals[i].fMax + fDot;
		FASSERT( pTransDOP->aTransIntervals[i].fMin < pTransDOP->aTransIntervals[i].fMax );
	}

	for ( i = 0; i < FkDOP_aDesc[pCollTree->nTreekDOPType].nEdgeNormalCount; i++ )
	{
		pBone->MulDir( pTransDOP->aTransEdges[i], FkDOP_aDesc[pCollTree->nTreekDOPType].avEdgeNormals[i] );
	}

	// Generate new intervals based on the transformed kDOP points
	fkdop_GenerateIntervals( pTransDOP->avTransVerts, nVertIdx, pTransDOP->aIntervals, pCollTree->nTreekDOPType );

	pTransDOP->nGenerationFrame = FVid_nFrameCounter;
}

