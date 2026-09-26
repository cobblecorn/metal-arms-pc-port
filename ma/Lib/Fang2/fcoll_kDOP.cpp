//////////////////////////////////////////////////////////////////////////////////////
// fcoll_kDOP.cpp - C code for handling kDOP collision
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
// 11/21/02	Lafleur		Created
//////////////////////////////////////////////////////////////////////////////////////


#include "fcoll.h"
#include "fmesh.h"
#include "fkdop.h"
#include "fmesh_coll.h"
#include "fworld.h"

#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	#include "frenderer.h"
	#include "fdraw.h"
	#define _SHOW_KDOP_IMPACTS		FALSE
#else
	#define _SHOW_KDOP_IMPACTS		FALSE
#endif


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



//////////////////////////////////////////////////////////////////////////////////////
// External Dependencies:
//////////////////////////////////////////////////////////////////////////////////////

extern CFCollData	 	*FColl_pCollData;
extern CFVec3A			FColl_vUnitizedMovement;
extern f32				FColl_fMoveMagnitude;
extern u16				FColl_nAccumCollMasks;

extern CFVec3A			FColl_vODiff, FColl_vDiff1, FColl_vDiff2, FColl_vCross, FColl_vPush, FColl_vNormal, FColl_vTransMovement;
extern CFMtx43A 		*FColl_pTestToWorld;
extern const CFMeshInst *FColl_pTestMeshInst;
extern const CFWorldMesh*FColl_pTestWorldMesh;
extern FkDOP_Tree_t 	*FColl_pkDOPTree1;
extern FkDOP_Tree_t 	*FColl_pkDOPTree2;
extern u8				FColl_nCurrentBoneIdx;
extern f32				FColl_fBoneScale;
extern f32				FColl_fInvBoneScale;
extern f32				FColl_fScaledMoveMagnitude;


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

static CFMtx43A			_mtxDOP;

//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static void _CalculateMeshMatrix( CFMtx43A &mtxOut, CFMeshInst *pMesh, u32 nkDOPTreeIdx );
static void _TestDOPWithMesh( FMesh_Coll_TransformedkDOP_t *pkDOP );
static f32 _IntersectDynamickDOPAndTriangle( const CFVec3A *avTri, const CFVec3A *pTriNormal, const FMesh_Coll_TransformedkDOP_t *pkDOP, CFVec3A *pContact );
static f32 _IntersectStatickDOPAndTriangle( const CFVec3A *avTri, const CFVec3A *pTriNormal, const FMesh_Coll_TransformedkDOP_t *pkDOP, CFVec3A *pContact );

static FINLINE f32 _GetSmallestVertDot( f32 fDot0, f32 fDot1, f32 fDot2, u8 *pVertPoints );
static FINLINE f32 _GetLargestVertDot( f32 fDot0, f32 fDot1, f32 fDot2, u8 *pVertPoints );
static FINLINE void _TransformWorldImpacts( u16 nStart );
static FINLINE void _TransformImpacts( u16 nStart, u16 nBoneIdx );



//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////


//
//
//
BOOL fcoll_TestWorldMeshAndkDOP( CFMeshInst *pWorldMeshInst, CFMeshInst *pMeshInst, const CFMtx43A *pMeshMtx, s32 nBone )
{
	FASSERT( pMeshInst && pWorldMeshInst );
	
	if ( !pWorldMeshInst->m_pMesh->paCollTree )
	{
		return FALSE;
	}
	
	FColl_vTransMovement.Set( *FColl_pCollData->pMovement );
	FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude;
			
	FColl_pTestToWorld = NULL;
	FColl_pTestMeshInst = pWorldMeshInst;
	FColl_pTestWorldMesh = NULL;
	FMesh_t *pMesh1 = pWorldMeshInst->m_pMesh;
	FMesh_t *pMesh2 = pMeshInst->m_pMesh;

	u16 nStartingImpactCount = FColl_nImpactCount;

	u16 nkDOPTree1Idx, nkDOPTree2Idx;
	FMesh_Coll_TransformedkDOP_t *pTransDOP;
	for ( nkDOPTree1Idx = 0; nkDOPTree1Idx < pMesh1->nCollTreeCount; nkDOPTree1Idx++ )
	{
		FColl_pkDOPTree1 = &pMesh1->paCollTree[nkDOPTree1Idx];

		if ( !(FColl_pkDOPTree1->nMasterCollMask & FColl_pCollData->nCollMask) )
		{
			continue;
		}
		
		for ( nkDOPTree2Idx = 0; nkDOPTree2Idx < pMesh2->nCollTreeCount; nkDOPTree2Idx++ )
		{
			FColl_pkDOPTree2 = &pMesh2->paCollTree[nkDOPTree2Idx];

			FASSERT( pMesh2->aSeg[FColl_pkDOPTree2->nSegmentIdx].nBoneMtxCount < 2 );
			FColl_nCurrentBoneIdx = pMesh2->aSeg[FColl_pkDOPTree2->nSegmentIdx].anBoneMtxIndex[0];
			
			if ( nBone != -1 && nBone != FColl_nCurrentBoneIdx )
			{
				continue;
			}
			
			pTransDOP = fmesh_Coll_CalculateDOPinWS( pMeshInst, nkDOPTree2Idx, pMeshMtx );
			if ( !pTransDOP )
			{
				FASSERT_NOW;
				continue;
			}

#if FMESH_DRAW_COLLIDED_KDOPS
			fmesh_coll_CalculateBoneMatrix( pMeshInst, nkDOPTree2Idx, &_mtxDOP );
#endif

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
			
			_TestDOPWithMesh( pTransDOP );
			
			if ( nStartingImpactCount != FColl_nImpactCount && (FColl_pCollData->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
			{
				_TransformWorldImpacts( nStartingImpactCount );
				return TRUE;
			}
			
			if ( nBone != -1 )
			{
				// We've tested the bone we came here to test
				break;
			}
		}
	}
	
	_TransformWorldImpacts( nStartingImpactCount );

	return (nStartingImpactCount < FColl_nImpactCount);
}


//
//
//
BOOL fcoll_TestMeshAndkDOP( CFWorldMesh *pTestWorldMesh, CFMeshInst *pMeshInst, const CFMtx43A *pMeshMtx, s32 nBone )
{
	FASSERT( pMeshInst && pTestWorldMesh );

	if ( !pTestWorldMesh->m_pMesh->paCollTree )
	{
		return FALSE;
	}

	BOOL bFoundImpacts = FALSE;
	CFMtx43A mtxTestBone;
	CFMtx43A mtxTestInvBone;
	FColl_pTestToWorld = &mtxTestBone;
	FColl_pTestMeshInst = pTestWorldMesh;
	FColl_pTestWorldMesh = pTestWorldMesh;
	FMesh_t *pMesh1 = pTestWorldMesh->m_pMesh;
	FMesh_t *pMesh2 = pMeshInst->m_pMesh;

	FColl_nAccumCollMasks = 0;

	u16 nkDOPTree1Idx, nkDOPTree2Idx, nBoneIdx;
	FMesh_Coll_TransformedkDOP_t TransDOP;
	for ( nkDOPTree1Idx = 0; nkDOPTree1Idx < pMesh1->nCollTreeCount; nkDOPTree1Idx++ )
	{
		FColl_pkDOPTree1 = &pMesh1->paCollTree[nkDOPTree1Idx];

		if ( !(FColl_pkDOPTree1->nMasterCollMask & FColl_pCollData->nCollMask) )
		{
			continue;
		}

		FMesh_t *pMesh = FColl_pTestMeshInst->m_pMesh;	

		// Get the bone info for the test mesh
		nBoneIdx = fmesh_coll_CalculateBoneMatrix( FColl_pTestMeshInst, nkDOPTree1Idx, &mtxTestBone, &mtxTestInvBone, &FColl_fBoneScale, &FColl_fInvBoneScale );

		// Record the current impact count so we can detect a change
		u16 nStartingImpactCount = FColl_nImpactCount;

		for ( nkDOPTree2Idx = 0; nkDOPTree2Idx < pMesh2->nCollTreeCount; nkDOPTree2Idx++ )
		{
			FColl_pkDOPTree2 = &pMesh2->paCollTree[nkDOPTree2Idx];

			FASSERT( pMesh2->aSeg[FColl_pkDOPTree2->nSegmentIdx].nBoneMtxCount < 2 );
			FColl_nCurrentBoneIdx = pMesh2->aSeg[FColl_pkDOPTree2->nSegmentIdx].anBoneMtxIndex[0];

			if ( nBone != -1 && nBone != FColl_nCurrentBoneIdx )
			{
				continue;
			}
			
#if FMESH_DRAW_COLLIDED_KDOPS
			fmesh_coll_CalculateBoneMatrix( pMeshInst, nkDOPTree2Idx, &_mtxDOP );
#endif

			// Adjust for scale
			mtxTestInvBone.MulDir( FColl_vTransMovement, *FColl_pCollData->pMovement );
			FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude * FColl_fInvBoneScale;

			// Generate a new set of intervals that represents the kDOP in the bone space				
			fmesh_Coll_GeneratekDOPIntervals( &TransDOP, pMeshInst, FColl_pkDOPTree2, pMeshMtx, &mtxTestInvBone );

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;

			_TestDOPWithMesh( &TransDOP );
		
			// If we're only looking for the first impact, bail
			if ( nStartingImpactCount != FColl_nImpactCount && (FColl_pCollData->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
			{
				break;
			}
			
			// If we're only testing one bone, bail
			if ( nBone != -1 )
			{
				break;
			}
		}
		
		// If we've got impacts, we need to transform them from bone space
		if ( nStartingImpactCount != FColl_nImpactCount )
		{
			_TransformImpacts( nStartingImpactCount, nBoneIdx );
			bFoundImpacts = TRUE;
			if ( FColl_pCollData->nStopOnFirstOfCollMask & FColl_nAccumCollMasks )
			{
				return TRUE;
			}
		}
	}
	
	return bFoundImpacts;
}


//
//
//
static FINLINE void _TransformWorldImpacts( u16 nStart )
{
	u32 i;
	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		#if FMESH_DRAW_COLLIDED_TRIS
			CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
			fmesh_Coll_DrawMeshTri( FColl_aImpactBuf[i].aTriVtx, &FColl_aImpactBuf[i].UnitFaceNormal, NULL, &Color, TRUE );
#if _SHOW_KDOP_IMPACTS
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3, 255, 255, 0 );
#endif
		#endif
	}
}


//
//
//
static FINLINE void _TransformImpacts( u16 nStart, u16 nBoneIdx )
{
	FASSERT( FColl_pTestToWorld );
	
	u32 i;
	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		FColl_pTestToWorld->MulPoint( FColl_aImpactBuf[i].aTriVtx[0] );
		FColl_pTestToWorld->MulPoint( FColl_aImpactBuf[i].aTriVtx[1] );
		FColl_pTestToWorld->MulPoint( FColl_aImpactBuf[i].aTriVtx[2] );
		FColl_pTestToWorld->MulPoint( FColl_aImpactBuf[i].ImpactPoint );
		FColl_pTestToWorld->MulDir( FColl_aImpactBuf[i].UnitFaceNormal.Mul( FColl_fInvBoneScale ) );
		FColl_aImpactBuf[i].PushUnitVec.Unitize();
		FColl_aImpactBuf[i].nBoneIndex = (u8)nBoneIdx;
		FColl_aImpactBuf[i].fImpactDistInfo *= FColl_fBoneScale;

#if FANG_DEBUG_BUILD
		f32 fDist = FColl_aImpactBuf[i].PushUnitVec.Mag();
		if ( fDist < 0.999f || fDist > 1.001f )
		{
			FASSERT_NOW;
		}
#endif
		
		#if FMESH_DRAW_COLLIDED_TRIS
			CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
			fmesh_Coll_DrawMeshTri( FColl_aImpactBuf[i].aTriVtx, &FColl_aImpactBuf[i].UnitFaceNormal, NULL, &Color, TRUE );
#if _SHOW_KDOP_IMPACTS
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3, 255, 255, 0 );
#endif
		#endif					
	}
}


//
//
//
static void _CalculateMeshMatrix( CFMtx43A &mtxOut, CFMeshInst *pMesh, u32 nkDOPTreeIdx )
{
	// Calculate the test mesh's bone matrix
	if ( pMesh->m_pMesh->nUsedBoneCount == 0 )
	{
		// This object has no bones
		mtxOut.Set( pMesh->m_Xfm.m_MtxF );
	}
	else
	{
		u32 nBoneIdx = pMesh->m_pMesh->aSeg[pMesh->m_pMesh->paCollTree[nkDOPTreeIdx].nSegmentIdx].anBoneMtxIndex[0];
		if ( pMesh->m_nFlags & FMESHINST_FLAG_NOBONES ) 
		{
			// This object is not animating
			mtxOut.Mul( pMesh->m_Xfm.m_MtxF, pMesh->m_pMesh->pBoneArray[nBoneIdx].AtRestBoneToModelMtx );
		}
		else
		{
			// This object is animating, so it's bone palette will be cool
			CFMtx43A **apBoneMtxList = pMesh->GetBoneMtxPalette();
			mtxOut.Set( *apBoneMtxList[nBoneIdx] );
		}
	}
}


//
//	Function used to test kDOP trees
//
static void _TestDOPWithMesh( FMesh_Coll_TransformedkDOP_t *pkDOP )
{
	FASSERT( FColl_pTestMeshInst && FColl_pkDOPTree1 && FColl_pkDOPTree2 && FColl_pCollData );
	FASSERT( FMesh_Coll_nNodeStackIdx );
	
	FkDOP_Interval_t *pWorldInt;
	FkDOP_Node_t *pTestNode;
	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the top node off the stack
		pTestNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];

		// Make sure the nodes' masks matches the coll mask
		if ( !(pTestNode->nCollMask & FColl_pCollData->nCollMask) )
		{
			continue;
		}
		
		pWorldInt = &FColl_pkDOPTree1->paIntervals[pTestNode->nStartkDOPInterval];
	
		if ( !fkdop_IntersectkDOPAndkDOP( pWorldInt, FColl_pkDOPTree1->nTreekDOPType, pkDOP->aIntervals, FColl_pkDOPTree2->nTreekDOPType, &FColl_vTransMovement ) )
		{
			// kDOPs do not collide, so we can stop traversal down this branch
			continue;
		}
	
		if ( !pTestNode->paPackets )
		{
			// The world nodeis not a leaf, so push its children onto the stack
			FASSERT( pTestNode->nStartChildIdx + 1 < FColl_pkDOPTree1->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx + 1];
			continue;
		}
		
		// This is a leaf, so we should check the world mesh tris against the kDOP

		#if FANG_PLATFORM_GC
			FkDOP_CNormal_t *pNormals = (FkDOP_CNormal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
		#elif FANG_PLATFORM_DX
			FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
//ARG - >>>>>
		#elif FANG_PLATFORM_PS2
			FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
//ARG - <<<<<
		#else
			FASSERT_NOW;
		#endif
			
		// Uncompress the verts & normals
		u8 nPack, nTri;
		u16 nNodeVert, *pNodeVertIdx = (u16 *)pTestNode->pTriData;
		for ( nPack = 0; nPack < pTestNode->nTriPacketCount; nPack++ )
		{
			FkDOP_TriPacket_t *pPacket = &pTestNode->paPackets[nPack];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FColl_pCollData->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			#if FANG_PLATFORM_GC
				FGCVB_t *pVB = &FColl_pTestMeshInst->m_pMesh->pMeshIS->aVB[pPacket->nVBIdx];
			#elif FANG_PLATFORM_DX
				CFVec3 *pVerts = FColl_pTestMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				u32 *pVerts = (u32 *)FColl_pTestMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			for ( nTri = 0; nTri < pPacket->nTriCount; nTri++, pNormals++ )
			{
				nNodeVert = (u16)(pPacket->nStartVert + (nTri * 3));

				#if FANG_PLATFORM_GC
					// Get the normal and verts in preparation for checks
					pNormals->DecompressTo( FColl_pNextImpactSlot->UnitFaceNormal );
					pVB->GetPoint( pNodeVertIdx[nNodeVert++], FColl_pNextImpactSlot->aTriVtx[0].v3 );
					pVB->GetPoint( pNodeVertIdx[nNodeVert++], FColl_pNextImpactSlot->aTriVtx[1].v3 );
					pVB->GetPoint( pNodeVertIdx[nNodeVert],   FColl_pNextImpactSlot->aTriVtx[2].v3 );
				#elif FANG_PLATFORM_DX
					// Get the normal and verts in preparation for checks
					FColl_pNextImpactSlot->UnitFaceNormal.Set( *pNormals );
					FColl_pNextImpactSlot->aTriVtx[0].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
					FColl_pNextImpactSlot->aTriVtx[1].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
					FColl_pNextImpactSlot->aTriVtx[2].v3 = pVerts[ pNodeVertIdx[nNodeVert] ];
//ARG - >>>>>
				#elif FANG_PLATFORM_PS2
					// Get the normal and verts in preparation for checks
					FColl_pNextImpactSlot->UnitFaceNormal.Set( *pNormals );
					FColl_pNextImpactSlot->aTriVtx[0].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
					FColl_pNextImpactSlot->aTriVtx[1].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
					FColl_pNextImpactSlot->aTriVtx[2].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert] ]);
//ARG - <<<<<
				#else
					FASSERT_NOW;
				#endif

				if ( FColl_fScaledMoveMagnitude == 0.f )
				{
					FColl_pNextImpactSlot->fImpactDistInfo = _IntersectStatickDOPAndTriangle( FColl_pNextImpactSlot->aTriVtx, &FColl_pNextImpactSlot->UnitFaceNormal, pkDOP, &FColl_pNextImpactSlot->ImpactPoint );
				}
				else
				{
					FColl_pNextImpactSlot->fImpactDistInfo = _IntersectDynamickDOPAndTriangle( FColl_pNextImpactSlot->aTriVtx, &FColl_pNextImpactSlot->UnitFaceNormal, pkDOP, &FColl_pNextImpactSlot->ImpactPoint );
				}

				if ( FColl_pNextImpactSlot->fImpactDistInfo != -1.f )
				{
					FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
					FColl_pNextImpactSlot->pTag = (void *)FColl_pTestWorldMesh;
					FColl_pNextImpactSlot->nSourceBoneIndex = FColl_nCurrentBoneIdx;
					
					FColl_nAccumCollMasks |= pPacket->nCollMask;

					FColl_pNextImpactSlot++;
					FColl_nImpactCount++;
					
					// Check for collision buffer overflow
					if ( FColl_pNextImpactSlot == FColl_pImpactBufEnd )
					{
						#if FANG_PRODUCTION_BUILD
							return;
						#else
							FColl_nImpactCountOverLimit++;
							continue;
						#endif
					}
					
					if ( FColl_pCollData->nStopOnFirstOfCollMask & pPacket->nCollMask )
					{
						return;
					}
				}
			}
		}
		
		#if FMESH_DRAW_COLLIDED_KDOPS
//			fmesh_Coll_AddCollDrawDOP( pWorldInt, NULL, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
			fmesh_Coll_AddCollDrawDOP( FColl_pkDOPTree2->paIntervals, &_mtxDOP, FColl_pkDOPTree2->nTreekDOPType );
//			fmesh_Coll_AddCollDrawDOP( pkDOP->aTransIntervals, NULL, FColl_pkDOPTree2->nTreekDOPType );
		#endif // FMESH_DRAW_COLLIDED_KDOPS
	}
}


//
//
//
static FINLINE f32 _GetSmallestVertDot( f32 fDot0, f32 fDot1, f32 fDot2, u8 *pVertPoints )
{
	f32 fLeastOverlap = fDot0;
	(*pVertPoints) = 0x01;
	
	if ( fmath_Abs(fDot1 - fLeastOverlap) < 0.05f )
	{
		(*pVertPoints) |= 0x02;
	}
	else if ( fDot1 < fLeastOverlap )
	{
		fLeastOverlap = fDot1;
		(*pVertPoints) = 0x02;
	}
	if ( fmath_Abs(fDot2 - fLeastOverlap) < 0.05f )
	{
		(*pVertPoints) |= 0x04;
	}
	else if ( fDot2 < fLeastOverlap )
	{
		fLeastOverlap = fDot2;
		(*pVertPoints) = 0x04;
	}
	
	return fLeastOverlap;
}
				

//
//
//
static FINLINE f32 _GetLargestVertDot( f32 fDot0, f32 fDot1, f32 fDot2, u8 *pVertPoints )
{
	f32 fLeastOverlap = fDot0;
	(*pVertPoints) = 0x01;
	
	if ( fmath_Abs(fDot1 - fLeastOverlap) < 0.05f )
	{
		(*pVertPoints) |= 0x02;
	}
	else if ( fDot1 > fLeastOverlap )
	{
		fLeastOverlap = fDot1;
		(*pVertPoints) = 0x02;
	}
	if ( fmath_Abs(fDot2 - fLeastOverlap) < 0.05f )
	{
		(*pVertPoints) |= 0x04;
	}
	else if ( fDot2 > fLeastOverlap )
	{
		fLeastOverlap = fDot2;
		(*pVertPoints) = 0x04;
	}
	
	return fLeastOverlap;
}
				

//
//	Returns -1.f if there is no collision.  Otherwise, returns the distance of allowable travel and fills out pContact
//
static f32 _IntersectDynamickDOPAndTriangle( const CFVec3A *avTri, const CFVec3A *pTriNormal, const FMesh_Coll_TransformedkDOP_t *pkDOP, CFVec3A *pContact )
{
	f32 afDots[3][FKDOP_MAX_AXES];
	f32 afAxisMovement[FKDOP_MAX_AXES];
	f32 fMin, fMax;
	
	u8 nkDOPType = FColl_pkDOPTree2->nRootkDOPType;
	u8 i;
	u8 nNorms = (u8)FkDOP_aDesc[nkDOPType].nNormalCount;

	f32 fVelocityAlongTriNormal = FColl_vTransMovement.Dot( *pTriNormal );
	if ( (FColl_pCollData->nFlags & FCOLL_DATA_IGNORE_BACKSIDE) && fVelocityAlongTriNormal > 0.f )
	{
		return -1.f;
	}

	// Project the triangle against the axes and test for separation.  This
	// basically creates a kDOP surrounding the triangle and tests that
	// against the kDOP that is provided:
	for ( i = 0; i < nNorms; i++ )
	{
		afAxisMovement[i] = FColl_vTransMovement.Dot( pkDOP->aTransAxes[i] );

		// Consider the movement along the axis
		if ( afAxisMovement[i] > 0.f )
		{
			fMax = pkDOP->aTransIntervals[i].fMax + afAxisMovement[i];
			fMin = pkDOP->aTransIntervals[i].fMin;
		}
		else
		{
			fMax = pkDOP->aTransIntervals[i].fMax;
			fMin = pkDOP->aTransIntervals[i].fMin + afAxisMovement[i];
		}

		afDots[0][i] = avTri[0].Dot( pkDOP->aTransAxes[i] );
		afDots[1][i] = avTri[1].Dot( pkDOP->aTransAxes[i] );
		afDots[2][i] = avTri[2].Dot( pkDOP->aTransAxes[i] );

		// Since kDOPs have a max and min along each axis, we actually are testing two planes
		// at once for each dot product.  If all of the verts are outside the interval
		// formed by the two planes then there cannot be a collision situation.
		if ( afDots[0][i] < fMin && afDots[1][i] < fMin && afDots[2][i] < fMin )
		{
			return -1.f;
		}
		if ( afDots[0][i] > fMax && afDots[1][i] > fMax && afDots[2][i] > fMax )
		{
			return -1.f;
		}
	}
	
	// We now know that the triangle overlaps all of the intervals of the kDOP.  But
	// this does not guarantee collision.  The kDOP could still be out of collision.

	f32 fDist, fDist1, fDist2, afPointDistance[FKDOP_MAX_VERTS];

	fMin = fMax = 0.f;

	// Let's test all of the points of the kDOP to determine if they are on one side
	// of the triangle.  If they are then, again, we cannot have collision.
	const CFVec3A *pPointBelow = NULL, *pPointAbove = NULL;
	if ( FColl_fScaledMoveMagnitude == 0.f )
	{
		fMax = 0.f;
		fMin = 0.f;
	}
	else if ( fVelocityAlongTriNormal > 0.f )
	{
		fMax = fVelocityAlongTriNormal;
		fMin = 0.f;
	}
	else
	{
		fMax = 0.f;
		fMin = fVelocityAlongTriNormal;
	}
	for ( i = 0; i < FColl_pkDOPTree2->nRootkDOPVertCount; i++ )
	{
		afPointDistance[i] = FColl_vDiff1.Sub( pkDOP->avTransVerts[i], avTri[0] ).Dot( *pTriNormal );
		if ( afPointDistance[i] + fMin <= 0.f )
		{
			pPointBelow = &pkDOP->avTransVerts[i];
		}
		if ( afPointDistance[i] + fMax >= 0.f )
		{
			pPointAbove = &pkDOP->avTransVerts[i];
		}
	}

	if ( !pPointAbove || !pPointBelow )
	{
		// All points were either above or below
		return -1.f;
	}

	
	// Now we need to determine actual impacts.  To do this, we use the velocity of
	// the kDOP and find the first point of contact.  If the kDOP and the triangle
	// are intersecting, then we determine the overlapping intervals along each 
	// separating axis and the velocity along that interval.  Using that, we can
	// determine the first overlapping intervals that will separate and how far
	// we will need to move the kDOP along the negative velocity vector to cause
	// that separation to take place.
	// 
	// The separating axes are:
	//
	// - The axes of the kDOP checked against the tri verts (already calculated, above)
	// - The axes formed by crossing each tri edge with edge kDOP axis checked against
	//		both the tri verts and the kDOP verts.
	//

	u8  nTriVertContacts = 0;
	u8  nMinTriVertPoints, nMaxTriVertPoints;
	s8  ii;
	u64 nkDOPVertContacts = 0;
	f32 fMinInterval, fMaxInterval, fTest;
	u64 nkDOPMinPoints, nkDOPMaxPoints;
	CFVec3A vInCollPushUnitVec;

	BOOL bInCollisionState = TRUE;
	FColl_pNextImpactSlot->fUnitImpactTime = -FMATH_MAX_FLOAT;
	FColl_pNextImpactSlot->fImpactDistInfo = FMATH_MAX_FLOAT;
	f32 fInCollisionDistInfo = FMATH_MAX_FLOAT;

	// Test DOP verts against the polygon face (DOT's have already been calculated, above)
	fMin = fMax = afPointDistance[0];
	nkDOPMinPoints = nkDOPMaxPoints = 0x00000001;
	for ( ii = 1; ii < FColl_pkDOPTree2->nRootkDOPVertCount; ii++ )
	{
		if ( fmath_Abs( afPointDistance[ii] - fMin ) <= 0.01f )
		{
			nkDOPMinPoints |= 1 << ii;
		}
		else if ( afPointDistance[ii] < fMin )
		{
			nkDOPMinPoints = 1 << ii;
			fMin = afPointDistance[ii];
		}

		if ( fmath_Abs( afPointDistance[ii] - fMax ) <= 0.01f )
		{
			nkDOPMaxPoints |= 1 << ii;
		}
		else if ( afPointDistance[ii] > fMax )
		{
			nkDOPMaxPoints = 1 << ii;
			fMax = afPointDistance[ii];
		}
	}

	if ( fMax < 0.f )
	{
		if ( fVelocityAlongTriNormal <= 0.f )
		{
			// Velocity will not cause contact
			return -1.f;
		}
		if ( fMax + fVelocityAlongTriNormal < 0.f )
		{
			// Velocity will not cause contact
			return -1.f;
		}

		fTest = fmath_Div( -fMax, fVelocityAlongTriNormal );
		if ( fTest > FColl_pNextImpactSlot->fUnitImpactTime )
		{
			bInCollisionState = FALSE;
			FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( *pTriNormal );
			FColl_pNextImpactSlot->fUnitImpactTime = fTest;
			FColl_pNextImpactSlot->fImpactDistInfo = fVelocityAlongTriNormal + fMax;
			nTriVertContacts = 0x00000007;
			nkDOPVertContacts = nkDOPMaxPoints;
		}
	}
	else if ( fMin > 0.f )
	{
		if ( fVelocityAlongTriNormal >= 0.f )
		{
			return -1.f;
		}
		if ( fMin + fVelocityAlongTriNormal > 0.f )
		{
			return -1.f;
		}

		fTest = fmath_Div( -fMin, fVelocityAlongTriNormal );
		if ( fTest > FColl_pNextImpactSlot->fUnitImpactTime )
		{
			bInCollisionState = FALSE;
			FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
			FColl_pNextImpactSlot->fUnitImpactTime = fTest;
			FColl_pNextImpactSlot->fImpactDistInfo = -fVelocityAlongTriNormal - fMin;
			nTriVertContacts = 0x00000007;
			nkDOPVertContacts = nkDOPMinPoints;
		}
	}
	else
	{
		// kDOP starts in a collision state and needs to be moved out
		if ( fMin + fVelocityAlongTriNormal > 0.f || fMax + fVelocityAlongTriNormal < 0.f )
		{
			// Velocity will cause collision to cease
			return -1.f;
		}

		if ( fVelocityAlongTriNormal >= 0.f && fMax < -fMin )
		{
			// kDOP and tri are initially overlapping, and the kDOP is moving towards the
			// backside of the poly AND is less than halfway across the triangle
			if ( FColl_pNextImpactSlot->fUnitImpactTime > 0.f || fMax < FColl_pNextImpactSlot->fImpactDistInfo )
			{
				FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( *pTriNormal );
				FColl_pNextImpactSlot->fImpactDistInfo = fMax + fVelocityAlongTriNormal;
				vInCollPushUnitVec.Set( FColl_pNextImpactSlot->PushUnitVec );
				fInCollisionDistInfo = fMax;
				nTriVertContacts = 0x00000007;
				nkDOPVertContacts = nkDOPMaxPoints;
			}
		}
		else
		{
			// kDOP and tri are initially overlapping, so calculate amount to push
			if ( FColl_pNextImpactSlot->fUnitImpactTime > 0.f || -fMin < FColl_pNextImpactSlot->fImpactDistInfo )
			{
				FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
				FColl_pNextImpactSlot->fImpactDistInfo = -fMin - fVelocityAlongTriNormal;
				vInCollPushUnitVec.Set( FColl_pNextImpactSlot->PushUnitVec );
				fInCollisionDistInfo = -fMin;
				nTriVertContacts = 0x00000007;
				nkDOPVertContacts = nkDOPMinPoints;
			}
		}
	}

	// Test the verts of the kDOP and the tri against the cross product of all tri edges and kDOP edges
	u8 nEdge, nLastPoint = 2;
	f32 fTriCrossDot[3];
	for ( nEdge = 0; nEdge < 3; nEdge++ )
	{
		FColl_vDiff1.Sub( avTri[nEdge], avTri[nLastPoint] );
		nLastPoint = nEdge;

		for ( i = 0; i < FkDOP_aDesc[nkDOPType].nEdgeNormalCount; i++ )
		{
			FColl_vCross.Cross( pkDOP->aTransEdges[i], FColl_vDiff1 );

			f32 fMag = FColl_vCross.MagSq();
			if ( fMag < 0.001f )
			{
				continue;
			}

			FColl_vCross.Mul( fmath_InvSqrt( fMag ) );
			
			f32 fVelocityAlongAxis = FColl_vTransMovement.Dot( FColl_vCross );

			// Get min and max intervals for triangle
			fTriCrossDot[0] = avTri[0].Dot( FColl_vCross );
			fTriCrossDot[1] = avTri[1].Dot( FColl_vCross );
			fTriCrossDot[2] = avTri[2].Dot( FColl_vCross );
			fMinInterval = _GetSmallestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMinTriVertPoints );
			fMaxInterval = _GetLargestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMaxTriVertPoints );

			// Get min and max interval for kDOP
			fMin = fMax = afPointDistance[0] = pkDOP->avTransVerts[0].Dot( FColl_vCross );
			nkDOPMinPoints = nkDOPMaxPoints = 0x00000001;
			for ( ii = 1; ii < FColl_pkDOPTree2->nRootkDOPVertCount; ii++ )
			{
				afPointDistance[ii] = pkDOP->avTransVerts[ii].Dot( FColl_vCross );

				if ( fmath_Abs( afPointDistance[ii] - fMin ) <= 0.01f )
				{
					nkDOPMinPoints |= 1 << ii;
				}
				else if ( afPointDistance[ii] < fMin )
				{
					nkDOPMinPoints = 1 << ii;
					fMin = afPointDistance[ii];
				}

				if ( fmath_Abs( afPointDistance[ii] - fMax ) <= 0.01f )
				{
					nkDOPMaxPoints |= 1 << ii;
				}
				else if ( afPointDistance[ii] > fMax )
				{
					nkDOPMaxPoints = 1 << ii;
					fMax = afPointDistance[ii];
				}
			}

			if ( fMaxInterval < fMin )
			{
				// Tri interval less than kDOP interval
				if ( fVelocityAlongAxis > 0.f )
				{
					// Intervals are not approaching so there cannot be any contact
					return -1.f;
				}

				if ( fMaxInterval - fMin < fVelocityAlongAxis )
				{
					// kDOP not traveling far enough for intervals to meet
					return -1.f;
				}

				fTest = fmath_Div( fMaxInterval - fMin, fVelocityAlongAxis );
				if ( fTest > FColl_pNextImpactSlot->fUnitImpactTime )
				{
					bInCollisionState = FALSE;
					FColl_pNextImpactSlot->fUnitImpactTime = fTest;
					nTriVertContacts = nMaxTriVertPoints;
					nkDOPVertContacts = nkDOPMinPoints;
				}
				fTest = -fVelocityAlongAxis - (fMin - fMaxInterval);
				if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
				{
					FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
					FColl_pNextImpactSlot->fImpactDistInfo = fTest;
				}
			}
			else if ( fMinInterval > fMax )
			{
				// Tri interval is greater than kDOP interval
				if ( fVelocityAlongAxis < 0.f )
				{
					// Intervals are not approaching so there cannot be any contact
					return -1.f;
				}

				if ( fMinInterval - fMax > fVelocityAlongAxis )
				{
					// kDOP not traveling far enough for intervals to meet
					return -1.f;
				}

				fTest = fmath_Div( fMinInterval - fMax, fVelocityAlongAxis );
				if ( fTest > FColl_pNextImpactSlot->fUnitImpactTime )
				{
					bInCollisionState = FALSE;
					FColl_pNextImpactSlot->fUnitImpactTime = fTest;
					nTriVertContacts = nMinTriVertPoints;
					nkDOPVertContacts = nkDOPMaxPoints;
				}
				fTest = fVelocityAlongAxis - (fMinInterval - fMax);
				if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
				{
					FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
					FColl_pNextImpactSlot->fImpactDistInfo = fTest;
				}
			}
			else //if ( bInCollisionState )
			{
				// kDOP is already in collision state

				if ( fmath_Abs( fMin - fMaxInterval ) < fmath_Abs( fMax - fMinInterval ) )
				{
					if ( fMin + fVelocityAlongAxis > fMaxInterval )
					{
						// velocity is going to cause kDOP to leave collision
						return -1.f;
					}

					fTest = fmath_Abs(fMin - fMaxInterval);
					if ( bInCollisionState && fTest < fInCollisionDistInfo )
					{
						fInCollisionDistInfo = fTest;
						vInCollPushUnitVec.Set( FColl_vCross );
						nTriVertContacts = nMaxTriVertPoints;
						nkDOPVertContacts = nkDOPMinPoints;
					}
					fTest = fTest - fVelocityAlongAxis;
					if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
					{
						FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
						FColl_pNextImpactSlot->fImpactDistInfo = fTest;
					}
				}
				else
				{
					if ( fMax + fVelocityAlongAxis < fMinInterval )
					{
						// velocity is going to cause kDOP to leave collision
						return -1.f;
					}

					fTest = fmath_Abs(fMax - fMinInterval);
					if ( bInCollisionState && fTest < FColl_pNextImpactSlot->fImpactDistInfo )
					{
						fInCollisionDistInfo = fTest;
						vInCollPushUnitVec.ReceiveNegative( FColl_vCross );
						nTriVertContacts = nMinTriVertPoints;
						nkDOPVertContacts = nkDOPMaxPoints;
					}
					fTest = fTest + fVelocityAlongAxis;
					if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
					{
						FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
						FColl_pNextImpactSlot->fImpactDistInfo = fTest;
					}
				}
			}
		}
	}

	if ( bInCollisionState )
	{
		// A return of -1.f in the unit impact time indicates to the calling function that the object is initially in collision
		FColl_pNextImpactSlot->fUnitImpactTime = -1.f;
		FColl_pNextImpactSlot->fImpactDistInfo = fInCollisionDistInfo;
		FColl_pNextImpactSlot->PushUnitVec.Set( vInCollPushUnitVec );
	}

	if ( FColl_pNextImpactSlot->fUnitImpactTime == FMATH_MAX_FLOAT )
	{
		// In the end, we had no collision
		return -1.f;
	}

	// Check the validity of some results
	FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.f );
	FASSERT( FColl_pNextImpactSlot->fUnitImpactTime >= -0.001f || FColl_pNextImpactSlot->fUnitImpactTime == -1.f );

	if ( FColl_pNextImpactSlot->fUnitImpactTime != -1.f )
	{
		FASSERT( FColl_pNextImpactSlot->fUnitImpactTime <= 1.01f );
		FMATH_CLAMP( FColl_pNextImpactSlot->fUnitImpactTime, 0.f, 1.f );
	}

	if ( FColl_pCollData->nFlags & FCOLL_DATA_DONT_CALCULATE_IMPACT )
	{
		pContact->Set( 0.f, 0.f, 0.f );
		return FColl_pNextImpactSlot->fImpactDistInfo;
	}

	f32 fPushDistance = FColl_pNextImpactSlot->fImpactDistInfo;

	// At this point we need to determine where the contact point is:

	u8 nTriContactCount = 0, nkDOPContactCount;
	u8 nkDOPContactIndices[16], nTriContactIndices[3];
	if ( nTriVertContacts & 0x01 )
	{
		nTriContactIndices[ nTriContactCount++ ] = 0;
	}
	if ( nTriVertContacts & 0x02 )
	{
		nTriContactIndices[ nTriContactCount++ ] = 1;
	}
	if ( nTriVertContacts & 0x04 )
	{
		nTriContactIndices[ nTriContactCount++ ] = 2;
	}
	FASSERT( nTriContactCount > 0 );
	if ( nTriContactCount == 1 )
	{
		// A single vert of the triangle contacts the kDOP so we can assume that is the impact point

		// Determine the index
		i = 0;
		if ( nTriVertContacts & 0x02 )
		{	
			i = 1;
		}
		else if ( nTriVertContacts & 0x04 )
		{
			i = 2;
		}

		pContact->Set( avTri[i] );
		return fPushDistance;
	}


	nkDOPContactCount = i = 0;
	while ( i < 32 )
	{
		nkDOPContactIndices[nkDOPContactCount] = i;
		nkDOPContactCount += (u8)((nkDOPVertContacts >> i) & 0x1);
		i++;
	}

	FASSERT( nkDOPContactCount > 0 );

	if ( nkDOPContactCount == 1 )
	{
		// The kDOP contacts the triangle at just one vert, so this must be the impact
		if ( FColl_pNextImpactSlot->fUnitImpactTime == -1.f )
		{
			pContact->Mul( FColl_pNextImpactSlot->PushUnitVec,  FColl_pNextImpactSlot->fImpactDistInfo ).Add( pkDOP->avTransVerts[nkDOPContactIndices[0]] );
		}
		else
		{
			pContact->Mul( FColl_vTransMovement, FColl_pNextImpactSlot->fUnitImpactTime ).Add( pkDOP->avTransVerts[nkDOPContactIndices[0]] );
		}

#if _SHOW_KDOP_IMPACTS
		fmesh_Coll_AddCollDrawImpact( &pContact->v3, 0, 255, 255 );
#endif
		return fPushDistance;
	}

	// Now we know that more than one tri vert and more than one kDOP vert
	// provide the information on the contact, so things get a little harrier (not the jet).

	if ( nkDOPContactCount == 2 && nTriContactCount == 2 )
	{
		// The triangle and the kDOP contact along an edge of each, so figure out the 
		// intersection of the kDOP edge and the triangle:

		if ( FColl_pNextImpactSlot->fUnitImpactTime == -1.f )
		{
			FColl_vODiff.Mul( FColl_pNextImpactSlot->PushUnitVec, FColl_pNextImpactSlot->fImpactDistInfo );	
		}
		else
		{
			FColl_vODiff.Mul( FColl_vTransMovement, FColl_pNextImpactSlot->fUnitImpactTime );	
		}
		FColl_vDiff1.Add( FColl_vODiff, pkDOP->avTransVerts[nkDOPContactIndices[0]] );
		FColl_vDiff2.Add( FColl_vODiff, pkDOP->avTransVerts[nkDOPContactIndices[1]] );

		FColl_vODiff.Sub( FColl_vDiff2, FColl_vDiff1 );
		FColl_vNormal.Sub( avTri[nTriContactIndices[1]], avTri[nTriContactIndices[0]] );

		FColl_vCross.Cross( FColl_vODiff, FColl_vNormal );
		FColl_vCross.Cross( FColl_vODiff );
		fDist1 = FColl_vCross.MagSq();
		if ( fDist1 == 0.f )
		{
			// Edges are parallel
			return -1.f;
		}
		FColl_vCross.Mul( fmath_InvSqrt( fDist1 ) );

		FColl_vODiff.Sub( avTri[nTriContactIndices[1]], FColl_vDiff1 );
		fDist1 = FColl_vCross.Dot( FColl_vODiff );
		FColl_vODiff.Sub( avTri[nTriContactIndices[0]], FColl_vDiff1 );
		fDist2 = FColl_vCross.Dot( FColl_vODiff );

		pContact->Mul( FColl_vNormal, fmath_Abs(fmath_Div( fDist2, fDist1 - fDist2 )) ).Add( avTri[nTriContactIndices[0]] );

#if _SHOW_KDOP_IMPACTS
		fmesh_Coll_AddCollDrawImpact( &pContact->v3, 0, 255, 0 );
#endif
		return fPushDistance;
	}

	if ( nTriContactCount == 2 )
	{
		// The edge of the triangle contacts a face or edge of the kDOP
		if ( FColl_pNextImpactSlot->fUnitImpactTime == -1.f )
		{
			FColl_vODiff.Mul( FColl_pNextImpactSlot->PushUnitVec, FColl_pNextImpactSlot->fImpactDistInfo );	
		}
		else
		{
			FColl_vODiff.Mul( FColl_vTransMovement, -FColl_pNextImpactSlot->fUnitImpactTime );	
		}
		FColl_vDiff1.Add( FColl_vODiff, avTri[nTriContactIndices[0]] );
		FColl_vDiff2.Add( FColl_vODiff, avTri[nTriContactIndices[1]] );

		FColl_vODiff.Sub( FColl_vDiff2, FColl_vDiff1 );
		fDist1 = 0.f;
		fDist2 = 1.f;
		for ( i = 0; i < nNorms; i++ )
		{
			f32 fAxisLength = fmath_Abs( FColl_vODiff.Dot( pkDOP->aTransAxes[i] ) );
			if ( fmath_Abs( fAxisLength ) < 0.001f )
			{
				continue;
			}

			fMin = FColl_vDiff1.Dot( pkDOP->aTransAxes[i] );
			fMax = FColl_vDiff2.Dot( pkDOP->aTransAxes[i] );
            if ( fMin < pkDOP->aTransIntervals[i].fMin )
			{
				fDist = fmath_Div(pkDOP->aTransIntervals[i].fMin - fMin, fAxisLength);
				if ( fDist > fDist1 )
				{
					fDist1 = fDist;
				}
			}
			else if ( fMin > pkDOP->aTransIntervals[i].fMax )
			{
				fDist = fmath_Div(fMin - pkDOP->aTransIntervals[i].fMax, fAxisLength);
				if ( fDist > fDist1 )
				{
					fDist1 = fDist;
				}
			}

			if ( fMax < pkDOP->aTransIntervals[i].fMin )
			{
				fDist = fmath_Div(fAxisLength - (pkDOP->aTransIntervals[i].fMin - fMax), fAxisLength);
				if ( fDist < fDist2 )
				{
					fDist2 = fDist;
				}
			}
			else if ( fMax > pkDOP->aTransIntervals[i].fMax )
			{
				fDist = fmath_Div(fAxisLength - (fMax - pkDOP->aTransIntervals[i].fMax), fAxisLength);
				if ( fDist < fDist2 )
				{
					fDist2 = fDist;
				}
			}
		}

		// If the following conditional is not the case, then we probably actually have a full tri face
		// contact, but the inaccurate math failed to detect it.
		if ( fDist1 <= fDist2 )
		{
			FColl_vDiff2.Sub( avTri[nTriContactIndices[1]], avTri[nTriContactIndices[0]] );
	//		fMax = FColl_vDiff2.Mag();
	//		FColl_vDiff2.Mul( fmath_Inv( fMax ) );

	//		FASSERT( fDist1 + fDist2 < fMax );

			if (  fDist2 < fDist1 )
			{
				fDist2 = fDist2;
			}
			pContact->Mul( FColl_vDiff2, (fDist1 + fDist2) * 0.5f ).Add( avTri[nTriContactIndices[0]] );
	#if _SHOW_KDOP_IMPACTS
			fmesh_Coll_AddCollDrawImpact( &pContact->v3, 255, 0, 0 );
	#endif
			return fPushDistance;
		}
	}

	// The face of the triangle contacts either an edge of the kDOP or a face of the kDOP.

	FColl_vDiff1.Zero();
	for ( i = 0; i < nkDOPContactCount; i++ )
	{
		FColl_vDiff1.Add( pkDOP->avTransVerts[nkDOPContactIndices[i]] );
	}
	if ( FColl_pNextImpactSlot->fUnitImpactTime == -1.f )
	{
		FColl_vODiff.Mul( FColl_pNextImpactSlot->PushUnitVec, FColl_pNextImpactSlot->fImpactDistInfo );	
		pContact->Div( FColl_vDiff1, nkDOPContactCount ).Add( FColl_vODiff );
	}
	else
	{
		FColl_vODiff.Mul( FColl_vTransMovement, FColl_pNextImpactSlot->fUnitImpactTime  );	
		pContact->Div( FColl_vDiff1, nkDOPContactCount ).Add( FColl_vODiff );
	}
#if _SHOW_KDOP_IMPACTS
	fmesh_Coll_AddCollDrawImpact( &pContact->v3, 255, 255, 0 );
#endif

	return fPushDistance;
}


//
//	Returns -1.f if there is no collision.  Otherwise, returns the distance of allowable travel and fills out pContact
//
static f32 _IntersectStatickDOPAndTriangle( const CFVec3A *avTri, const CFVec3A *pTriNormal, const FMesh_Coll_TransformedkDOP_t *pkDOP, CFVec3A *pContact )
{
	f32 afDots[3][FKDOP_MAX_AXES];
	f32 fMin, fMax;
	
	u8 nkDOPType = FColl_pkDOPTree2->nRootkDOPType;
	u8 i;
	u8 nNorms = (u8)FkDOP_aDesc[nkDOPType].nNormalCount;

	// Project the triangle against the axes and test for separation.  This
	// basically creates a kDOP surrounding the triangle and tests that
	// against the kDOP that is provided:
	for ( i = 0; i < nNorms; i++ )
	{
		afDots[0][i] = avTri[0].Dot( pkDOP->aTransAxes[i] );
		afDots[1][i] = avTri[1].Dot( pkDOP->aTransAxes[i] );
		afDots[2][i] = avTri[2].Dot( pkDOP->aTransAxes[i] );

		// Since kDOPs have a max and min along each axis, we actually are testing two planes
		// at once for each dot product.  If all of the verts are outside the interval
		// formed by the two planes then there cannot be a collision situation.
		if ( afDots[0][i] < pkDOP->aTransIntervals[i].fMin && afDots[1][i] < pkDOP->aTransIntervals[i].fMin && afDots[2][i] < pkDOP->aTransIntervals[i].fMin )
		{
			return -1.f;
		}
		if ( afDots[0][i] > pkDOP->aTransIntervals[i].fMax && afDots[1][i] > pkDOP->aTransIntervals[i].fMax && afDots[2][i] > pkDOP->aTransIntervals[i].fMax )
		{
			return -1.f;
		}
	}
	
	// We now know that the triangle overlaps all of the intervals of the kDOP.  But
	// this does not guarantee collision.  The kDOP could still be out of collision.

	f32 fDist, fDist1, fDist2, afPointDistance[FKDOP_MAX_VERTS];

	fMin = fMax = 0.f;

	// Let's test all of the points of the kDOP to determine if they are on one side
	// of the triangle.  If they are then, again, we cannot have collision.
	const CFVec3A *pPointBelow = NULL, *pPointAbove = NULL;
	for ( i = 0; i < FColl_pkDOPTree2->nRootkDOPVertCount; i++ )
	{
		afPointDistance[i] = FColl_vDiff1.Sub( pkDOP->avTransVerts[i], avTri[0] ).Dot( *pTriNormal );
		if ( afPointDistance[i] <= 0.f )
		{
			pPointBelow = &pkDOP->avTransVerts[i];
		}
		if ( afPointDistance[i] >= 0.f )
		{
			pPointAbove = &pkDOP->avTransVerts[i];
		}
	}

	if ( !pPointAbove || !pPointBelow )
	{
		// All points were either above or below
		return -1.f;
	}

	
	// Now we need to determine actual impacts.  To do this, we use the velocity of
	// the kDOP and find the first point of contact.  If the kDOP and the triangle
	// are intersecting, then we determine the overlapping intervals along each 
	// separating axis and the velocity along that interval.  Using that, we can
	// determine the first overlapping intervals that will separate and how far
	// we will need to move the kDOP along the negative velocity vector to cause
	// that separation to take place.
	// 
	// The separating axes are:
	//
	// - The axes of the kDOP checked against the tri verts (already calculated, above)
	// - The axes formed by crossing each tri edge with edge kDOP axis checked against
	//		both the tri verts and the kDOP verts.
	//

	u8  nTriVertContacts = 0;
	u8  nMinTriVertPoints, nMaxTriVertPoints;
	s8  fkDOPContactAxis = -1;
	s8  ii;
	u64 nkDOPVertContacts = 0;
	f32 fMinInterval, fMaxInterval, fTest;
//	f32 fFirstImpactTime = FMATH_MAX_FLOAT;
	u64 nkDOPMinPoints, nkDOPMaxPoints;
    
	FColl_pNextImpactSlot->fUnitImpactTime = -1.f;
	FColl_pNextImpactSlot->fImpactDistInfo = FMATH_MAX_FLOAT;
/*
	// Determine the smallest interval:
	for ( i = 0; i < nNorms; i++ )
	{
		fMinInterval = _GetSmallestVertDot( afDots[0][i], afDots[1][i], afDots[2][i], &nMinTriVertPoints );
		fMaxInterval = _GetLargestVertDot( afDots[0][i], afDots[1][i], afDots[2][i], &nMaxTriVertPoints );

		fTest = fmath_Abs( pkDOP->aTransIntervals[i].fMin - fMaxInterval );
		if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
		{
			FColl_pNextImpactSlot->fImpactDistInfo = fTest;
			nTriVertContacts = nMaxTriVertPoints;
			fkDOPContactAxis = i;
			FColl_vPush.ReceiveNegative( pkDOP->aTransAxes[i] );
		}

		fTest = fmath_Abs( pkDOP->aTransIntervals[i].fMax - fMinInterval );
		if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
		{
			FColl_pNextImpactSlot->fImpactDistInfo = fTest;
			nTriVertContacts = nMaxTriVertPoints;
			fkDOPContactAxis = i;
			FColl_vPush.Set( pkDOP->aTransAxes[i] );
		}
	}
*/
	// Test DOP verts against the polygon face (DOT's have already been calculated, above)
	fMin = FMATH_MAX_FLOAT; fMax = -FMATH_MAX_FLOAT;
	for ( i = 0; i < FColl_pkDOPTree2->nRootkDOPVertCount; i++ )
	{
		if ( fmath_Abs( afPointDistance[i] - fMin ) <= 0.01f )
		{
			nkDOPMinPoints |= 1 << i;
		}
		else if ( afPointDistance[i] < fMin )
		{
			nkDOPMinPoints = 1 << i;
			fMin = afPointDistance[i];
		}

		if ( fmath_Abs( afPointDistance[i] - fMax ) <= 0.01f )
		{
			nkDOPMaxPoints |= 1 << i;
		}
		if ( afPointDistance[i] > fMax )
		{
			nkDOPMaxPoints = 1 << i;
			fMax = afPointDistance[i];
		}
	}

	if ( fMin > 0.f || fMax < 0.f )
	{
		return -1.f;
	}

	if ( -fMin < fMax )
	{
		fTest = -fMin;
		if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
		{
			FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
			FColl_pNextImpactSlot->fImpactDistInfo = fTest;
			nTriVertContacts = 0x00000007;
			nkDOPVertContacts = nkDOPMinPoints;
			fkDOPContactAxis = -1;
		}
	}
	else
	{
		fTest = fMax;
		if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
		{
			FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( *pTriNormal );
			FColl_pNextImpactSlot->fImpactDistInfo = fTest;
			nTriVertContacts = 0x00000007;
			nkDOPVertContacts = nkDOPMaxPoints;
			fkDOPContactAxis = -1;
		}
	}
	
	u8 nEdge, nLastPoint = 2;
	f32 fTriCrossDot[3];
	for ( nEdge = 0; nEdge < 3; nEdge++ )
	{
		FColl_vDiff1.Sub( avTri[nEdge], avTri[nLastPoint] );
		nLastPoint = nEdge;
		
		for ( i = 0; i < FkDOP_aDesc[nkDOPType].nEdgeNormalCount; i++ )
		{
			FColl_vCross.Cross( pkDOP->aTransEdges[i], FColl_vDiff1 );
			
			f32 fMag = FColl_vCross.MagSq();
			if ( fMag < 0.001f )
			{
				continue;
			}

			FColl_vCross.Mul( fmath_InvSqrt( fMag ) );
			
			// Get min and max intervals for triangle
			fTriCrossDot[0] = avTri[0].Dot( FColl_vCross );
			fTriCrossDot[1] = avTri[1].Dot( FColl_vCross );
			fTriCrossDot[2] = avTri[2].Dot( FColl_vCross );
			fMinInterval = _GetSmallestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMinTriVertPoints );
			fMaxInterval = _GetLargestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMaxTriVertPoints );

			// Get min and max interval for kDOP
			fMin = fMax = afPointDistance[0] = pkDOP->avTransVerts[0].Dot( FColl_vCross );
			nkDOPMinPoints = nkDOPMaxPoints = 0x00000001;
			for ( ii = 1; ii < FColl_pkDOPTree2->nRootkDOPVertCount; ii++ )
			{
				afPointDistance[ii] = pkDOP->avTransVerts[ii].Dot( FColl_vCross );
				
				if ( fmath_Abs( afPointDistance[ii] - fMin ) <= 0.01f )
				{
					nkDOPMinPoints |= 1 << ii;
				}
				else if ( afPointDistance[ii] < fMin )
				{
					nkDOPMinPoints = 1 << ii;
					fMin = afPointDistance[ii];
				}
				
				if ( fmath_Abs( afPointDistance[ii] - fMax ) <= 0.01f )
				{
					nkDOPMaxPoints |= 1 << ii;
				}
				else if ( afPointDistance[ii] > fMax )
				{
					nkDOPMaxPoints = 1 << ii;
					fMax = afPointDistance[ii];
				}
			}
			
			if ( fMinInterval > fMax || fMin > fMaxInterval )
			{
				// Triangle and kDOP are separated along an axis, so there is no collision
				return -1.f;
			}

			fTest = fmath_Abs( fMaxInterval - fMin );
			if ( fTest < fmath_Abs( fMinInterval - fMax ) )
			{
				if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
				{
					FColl_pNextImpactSlot->fImpactDistInfo = fTest;
					nTriVertContacts = nMaxTriVertPoints;
					nkDOPVertContacts = nkDOPMinPoints;
					fkDOPContactAxis = -1;
					FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
				}
			}
			else
			{
				fTest = fmath_Abs( fMinInterval - fMax );
				if ( fTest < FColl_pNextImpactSlot->fImpactDistInfo )
				{
					FColl_pNextImpactSlot->fImpactDistInfo = fTest;
					nTriVertContacts = nMinTriVertPoints;
					nkDOPVertContacts = nkDOPMaxPoints;
					fkDOPContactAxis = -1;
					FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
				}
			}
		}
	}

	FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.f );

	if ( FColl_pNextImpactSlot->fImpactDistInfo == FMATH_MAX_FLOAT || FColl_pNextImpactSlot->fImpactDistInfo == 0.f )
	{
//		FASSERT( fFirstImpactTime == -FMATH_MAX_FLOAT || fFirstImpactTime == 0.f );
		return -1.f;
	}

//	FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vPush );
	f32 fPushDistance = FColl_pNextImpactSlot->fImpactDistInfo * 1.01f;

	if ( FColl_pCollData->nFlags & FCOLL_DATA_DONT_CALCULATE_IMPACT )
	{
		pContact->Set( 0.f, 0.f, 0.f );
		return fPushDistance;
	}

	// At this point we need to determine where the contact point is:
	
	u8 nTriContactCount = 0, nkDOPContactCount;
	u8 nkDOPContactIndices[16], nTriContactIndices[3];
	if ( nTriVertContacts & 0x01 )
	{
		nTriContactIndices[ nTriContactCount++ ] = 0;
	}
	if ( nTriVertContacts & 0x02 )
	{
		nTriContactIndices[ nTriContactCount++ ] = 1;
	}
	if ( nTriVertContacts & 0x04 )
	{
		nTriContactIndices[ nTriContactCount++ ] = 2;
	}
	FASSERT( nTriContactCount > 0 );
	if ( nTriContactCount == 1 )
	{
		// A single vert of the triangle contacts the kDOP so we can assume that is the impact point
		
		// Determine the index
		i = 0;
		if ( nTriVertContacts & 0x02 )
		{	
			i = 1;
		}
		else if ( nTriVertContacts & 0x04 )
		{
			i = 2;
		}
		
		pContact->Set( avTri[i] );
		return fPushDistance;
	}
	
	if ( fkDOPContactAxis == -1 )
	{
		// The triangle impacts first along an edge cross product
	
		nkDOPContactCount = i = 0;
		while ( i < 32 )
		{
			nkDOPContactIndices[nkDOPContactCount] = i;
			nkDOPContactCount += (u8)((nkDOPVertContacts >> i) & 0x1);
			i++;
		}
		
		FASSERT( nkDOPContactCount > 0 );
		
		if ( nkDOPContactCount == 1 )
		{
			// The kDOP contacts the triangle at just one vert, so this must be the impact
			pContact->Mul( FColl_pNextImpactSlot->PushUnitVec, FColl_pNextImpactSlot->fImpactDistInfo ).Add( pkDOP->avTransVerts[nkDOPContactIndices[0]] );
			return fPushDistance;
		}
		
		// Now we know that more than one tri vert and more than one kDOP vert
		// provide the information on the contact, so things get a little harrier (not the jet).
		
		if ( nkDOPContactCount == 2 && nTriContactCount == 2 )
		{
			// The triangle and the kDOP contact along an edge of each, so figure out the 
			// intersection of the kDOP edge and the triangle:
			
			FColl_vODiff.Mul( FColl_pNextImpactSlot->PushUnitVec, FColl_pNextImpactSlot->fImpactDistInfo );	
			FColl_vDiff1.Add( FColl_vODiff, pkDOP->avTransVerts[nkDOPContactIndices[0]] );
			FColl_vDiff2.Add( FColl_vODiff, pkDOP->avTransVerts[nkDOPContactIndices[1]] );

			FColl_vODiff.Sub( FColl_vDiff2, FColl_vDiff1 );
			FColl_vNormal.Sub( avTri[nTriContactIndices[1]], avTri[nTriContactIndices[0]] );

			FColl_vCross.Cross( FColl_vODiff, FColl_vNormal );
			FColl_vCross.Cross( FColl_vODiff );
			fDist1 = FColl_vCross.MagSq();
			if ( fDist1 == 0.f )
			{
				// Edges are parallel
				return -1.f;
			}
			FColl_vCross.Mul( fmath_InvSqrt( fDist1 ) );

			FColl_vODiff.Sub( avTri[nTriContactIndices[1]], FColl_vDiff1 );
			fDist1 = FColl_vCross.Dot( FColl_vODiff );
			FColl_vODiff.Sub( avTri[nTriContactIndices[0]], FColl_vDiff1 );
			fDist2 = FColl_vCross.Dot( FColl_vODiff );

			pContact->Mul( FColl_vNormal, fmath_Abs(fmath_Div( fDist2, fDist1 - fDist2 )) ).Add( avTri[nTriContactIndices[0]] );

			return fPushDistance;
		}
	}
	
	if ( nTriContactCount == 2 )
	{
		// The edge of the triangle contacts a face or edge of the kDOP
		FColl_vODiff.Mul( FColl_pNextImpactSlot->PushUnitVec, -FColl_pNextImpactSlot->fImpactDistInfo );	
		FColl_vDiff1.Add( FColl_vODiff, avTri[nTriContactIndices[0]] );
		FColl_vDiff2.Add( FColl_vODiff, avTri[nTriContactIndices[1]] );
		
		FColl_vODiff.Sub( FColl_vDiff2, FColl_vDiff1 );
		fDist1 = 0.f;
		fDist2 = 1.f;
		for ( i = 0; i < nNorms; i++ )
		{
			f32 fAxisLength = fmath_Abs( FColl_vODiff.Dot( pkDOP->aTransAxes[i] ) );

			if ( fmath_Abs( fAxisLength ) < 0.001f )
			{
				continue;
			}

			fMin = FColl_vDiff1.Dot( pkDOP->aTransAxes[i] );
			fMax = FColl_vDiff2.Dot( pkDOP->aTransAxes[i] );
            if ( fMin < pkDOP->aTransIntervals[i].fMin )
			{
				fDist = fmath_Div(pkDOP->aTransIntervals[i].fMin - fMin, fAxisLength);
				if ( fDist > fDist1 )
				{
					fDist1 = fDist;
				}
			}
			else if ( fMin > pkDOP->aTransIntervals[i].fMax )
			{
				fDist = fmath_Div(fMin - pkDOP->aTransIntervals[i].fMax, fAxisLength);
				if ( fDist > fDist1 )
				{
					fDist1 = fDist;
				}
			}

			if ( fMax < pkDOP->aTransIntervals[i].fMin )
			{
				fDist = fmath_Div(fAxisLength - (pkDOP->aTransIntervals[i].fMin - fMax), fAxisLength);
				if ( fDist < fDist2 )
				{
					fDist2 = fDist;
				}
			}
			else if ( fMax > pkDOP->aTransIntervals[i].fMax )
			{
				fDist = fmath_Div(fAxisLength - (fMax - pkDOP->aTransIntervals[i].fMax), fAxisLength);
				if ( fDist < fDist2 )
				{
					fDist2 = fDist;
				}
			}
		}

		FColl_vDiff2.Sub( avTri[nTriContactIndices[1]], avTri[nTriContactIndices[0]] );
//		fMax = FColl_vDiff2.Mag();
//		FColl_vDiff2.Mul( fmath_Inv( fMax ) );

//		FASSERT( fDist1 + fDist2 < fMax );

		pContact->Mul( FColl_vDiff2, (fDist1 + fDist2) * 0.5f ).Add( avTri[nTriContactIndices[0]] );
		return fPushDistance;
	}
	
	// The face of the triangle contacts either an edge of the kDOP or a face of the kDOP.

	FColl_vDiff1.Zero();
	for ( i = 0; i < nkDOPContactCount; i++ )
	{
		FColl_vDiff1.Add( pkDOP->avTransVerts[nkDOPContactIndices[0]] );
	}
	FColl_vODiff.Mul( FColl_pNextImpactSlot->PushUnitVec, FColl_pNextImpactSlot->fImpactDistInfo );	
	pContact->Div( FColl_vDiff1, nkDOPContactCount ).Add( FColl_vODiff );

	return fPushDistance;
}
