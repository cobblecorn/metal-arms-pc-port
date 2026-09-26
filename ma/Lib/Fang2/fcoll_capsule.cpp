//////////////////////////////////////////////////////////////////////////////////////
// fcoll_Capule.cpp - C code for handling capsule collision
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
#include "fmesh_coll.h"
#include "fworld.h"
#include "fkdop.h"
#if FMESH_COLL_ENABLE_COLLISION_DRAWS
	#include "frenderer.h"
	#include "fdraw.h"
#endif // FMESH_COLL_ENABLE_COLLISION_DRAWS


#if FANG_PLATFORM_GC
	#include "fgcvb.h"
	#include "fgcmesh.h"
#elif FANG_PLATFORM_DX
	#include "dx\fdx8mesh.h"
//ARG - <<<<<
#elif FANG_PLATFORM_PS2
	#include "ps2\fps2mesh.h"
//ARG - >>>>>
#endif


//////////////////////////////////////////////////////////////////////////////////////
// External Dependencies:
//////////////////////////////////////////////////////////////////////////////////////

extern CFCollData	 	*FColl_pCollData;
extern CFVec3A			FColl_vUnitizedMovement;
extern f32				FColl_fMoveMagnitude;
extern u16				FColl_nAccumCollMasks;

extern CFMtx43A			FColl_mtxTestBone, FColl_mtxTestInvBone, FColl_mtxBone, FColl_mtxTemp;
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

static CFVec3A			_vCapsuleNormal;


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static void _TestStaticCapsuleWithMesh( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius );
static void _TestDynamicCapsuleWithMesh( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius );
static BOOL _IntersectDynamicCapsuleAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius );
static BOOL _IntersectStaticCapsuleAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius );

static FINLINE void _TransformWorldImpacts( u16 nStart );
static FINLINE void _TransformImpacts( u16 nStart, u16 nBoneIdx );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////


//
//
//
BOOL fcoll_TestWorldMeshAndObjectCapsules( CFMeshInst *pWorldMeshInst, CFMeshInst *pMeshInst, s32 nBone )
{
	FASSERT( pMeshInst && pWorldMeshInst );
	
	if ( !pWorldMeshInst->m_pMesh->paCollTree )
	{
		return FALSE;
	}
	
	CFVec3A vStart, vEnd;
	FColl_pTestToWorld = NULL;
	FColl_pTestMeshInst = pWorldMeshInst;
	FColl_pTestWorldMesh = NULL;
	FMesh_t *pMesh1 = pWorldMeshInst->m_pMesh;
	FMesh_t *pMesh2 = pMeshInst->m_pMesh;

	u16 nStartingImpactCount = FColl_nImpactCount;

	if ( FColl_fMoveMagnitude )
	{
		FColl_vTransMovement.Set( *FColl_pCollData->pMovement );
	}
	else
	{
		FColl_vTransMovement.Set( 0.f, 0.f, 0.f );
	}
	FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude;

	u16 nkDOPTree1Idx, nkDOPTree2Idx;
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

			fmesh_coll_CalculateBoneMatrix( pMeshInst, nkDOPTree2Idx, &FColl_mtxBone );
			vStart.Set( FColl_pkDOPTree2->BoundingCapsule.vStart );
			FColl_mtxBone.MulPoint( vStart );
			vEnd.Set( FColl_pkDOPTree2->BoundingCapsule.vEnd );
			FColl_mtxBone.MulPoint( vEnd );

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
			
			if ( FColl_fMoveMagnitude == 0.f )
			{
				_TestStaticCapsuleWithMesh( &vStart, &vEnd, FColl_pkDOPTree2->BoundingCapsule.fRadius );
			}
			else
			{
				_TestDynamicCapsuleWithMesh( &vStart, &vEnd, FColl_pkDOPTree2->BoundingCapsule.fRadius );
			}
			
			#if FMESH_COLL_ENABLE_COLLISION_DRAWS
				if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
				{
					CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
					frenderer_Push( FRENDERER_DRAW, NULL );
					fdraw_FacetedWireSphere( &vStart.v3, FColl_pkDOPTree2->BoundingCapsule.fRadius, &Color );
					fdraw_FacetedWireSphere( &vEnd.v3, FColl_pkDOPTree2->BoundingCapsule.fRadius, &Color );
					frenderer_Pop();
				}
			#endif // FMESH_DRAW_COLLIDED_KDOPS

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
BOOL fcoll_TestMeshAndObjectCapsules( CFWorldMesh *pTestWorldMesh, CFMeshInst *pMeshInst, s32 nBone )
{
	FASSERT( pMeshInst && pTestWorldMesh );

	if ( !pTestWorldMesh->m_pMesh->paCollTree )
	{
		return FALSE;
	}

	CFVec3A vStart, vEnd;
	FMeshSeg_t *pSeg;
	f32 fScale, fRadius;
	BOOL bFoundImpacts = FALSE;
	FColl_pTestToWorld = &FColl_mtxTestBone;
	FColl_pTestMeshInst = pTestWorldMesh;
	FColl_pTestWorldMesh = pTestWorldMesh;
	FMesh_t *pMesh1 = pTestWorldMesh->m_pMesh;
	FMesh_t *pMesh2 = pMeshInst->m_pMesh;

	u16 nkDOPTree1Idx, nkDOPTree2Idx, nBoneIdx;
	for ( nkDOPTree1Idx = 0; nkDOPTree1Idx < pMesh1->nCollTreeCount; nkDOPTree1Idx++ )
	{
		FColl_pkDOPTree1 = &pMesh1->paCollTree[nkDOPTree1Idx];

		if ( !(FColl_pkDOPTree1->nMasterCollMask & FColl_pCollData->nCollMask) )
		{
			continue;
		}
		
		FMesh_t *pMesh = FColl_pTestMeshInst->m_pMesh;

		FColl_mtxTestBone, FColl_mtxTestInvBone, FColl_mtxBone, FColl_mtxTemp;

		// Get the bone info for the test mesh
		nBoneIdx = fmesh_coll_CalculateBoneMatrix( FColl_pTestMeshInst, nkDOPTree1Idx, &FColl_mtxTestBone, &FColl_mtxTestInvBone, &FColl_fBoneScale, &FColl_fInvBoneScale );

		// Record the current impact count so we can detect a change
		u16 nStartingImpactCount = FColl_nImpactCount;

		for ( nkDOPTree2Idx = 0; nkDOPTree2Idx < pMesh2->nCollTreeCount; nkDOPTree2Idx++ )
		{
			FColl_pkDOPTree2 = &pMesh2->paCollTree[nkDOPTree2Idx];

			pSeg = &pMesh2->aSeg[FColl_pkDOPTree2->nSegmentIdx];
			FASSERT( pSeg->nBoneMtxCount < 2 );
			FColl_nCurrentBoneIdx = pSeg->anBoneMtxIndex[0];
			
			if ( nBone != -1 && nBone != FColl_nCurrentBoneIdx )
			{
				continue;
			}

			// Calculate the matrix to bring this bone into the other bone's space
			fmesh_coll_CalculateBoneMatrix( pMeshInst, nkDOPTree2Idx, &FColl_mtxBone, NULL, &fScale );
			FColl_mtxTemp.Mul( FColl_mtxTestInvBone, FColl_mtxBone );
			fRadius = FColl_pkDOPTree2->BoundingCapsule.fRadius * fScale * FColl_fInvBoneScale;
			vStart.Set( FColl_pkDOPTree2->BoundingCapsule.vStart );
			FColl_mtxTemp.MulPoint( vStart );
			vEnd.Set( FColl_pkDOPTree2->BoundingCapsule.vEnd );
			FColl_mtxTemp.MulPoint( vEnd );

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
			
			if ( FColl_fMoveMagnitude == 0.f )
			{
				FColl_vTransMovement.Set( 0.f, 0.f, 0.f );
				FColl_fScaledMoveMagnitude = 0.f;
				_TestStaticCapsuleWithMesh( &vStart, &vEnd, fRadius );
			}
			else
			{
				FColl_mtxTestInvBone.MulDir( FColl_vTransMovement, *FColl_pCollData->pMovement );
				FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude * FColl_fInvBoneScale;
				_TestDynamicCapsuleWithMesh( &vStart, &vEnd, fRadius );
			}
			
			#if FMESH_COLL_ENABLE_COLLISION_DRAWS
				if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
				{
					frenderer_Push( FRENDERER_DRAW, NULL );
					CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
					CFVec3A vNewCenter;
					f32 fNewRadius = fRadius * FColl_fBoneScale;
					FColl_pTestToWorld->MulPoint( vNewCenter, vStart );
					fdraw_FacetedWireSphere( &vNewCenter.v3, fNewRadius, &Color );
					FColl_pTestToWorld->MulPoint( vNewCenter, vEnd );
					fdraw_FacetedWireSphere( &vNewCenter.v3, fNewRadius, &Color );
					frenderer_Pop();
				}
			#endif // FMESH_DRAW_COLLIDED_KDOPS

			// If we're only looking for the first impact, bail
			if ( nStartingImpactCount != FColl_nImpactCount && (FColl_pCollData->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
			{
				break;
			}
			
			if ( nBone != -1 )
			{
				// We've tested the bone we came here to test
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
BOOL fcoll_TestWorldMeshAndCapsule( CFMeshInst *pWorldMeshInst, CFCapsule *pCapsule )
{
	FASSERT( pCapsule && pWorldMeshInst );
	
	if ( !pWorldMeshInst->m_pMesh->paCollTree )
	{
		return FALSE;
	}
	
	CFVec3A vStart, vEnd;
	FColl_pTestToWorld = NULL;
	FColl_pTestMeshInst = pWorldMeshInst;
	FColl_pTestWorldMesh = NULL;
	FMesh_t *pMesh1 = pWorldMeshInst->m_pMesh;

	u16 nStartingImpactCount = FColl_nImpactCount;

	if ( FColl_fMoveMagnitude )
	{
		FColl_vTransMovement.Set( *FColl_pCollData->pMovement );
	}
	else
	{
		FColl_vTransMovement.Set( 0.f, 0.f, 0.f );
	}
	FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude;

	u16 nkDOPTree1Idx;
	for ( nkDOPTree1Idx = 0; nkDOPTree1Idx < pMesh1->nCollTreeCount; nkDOPTree1Idx++ )
	{
		FColl_pkDOPTree1 = &pMesh1->paCollTree[nkDOPTree1Idx];

		if ( !(FColl_pkDOPTree1->nMasterCollMask & FColl_pCollData->nCollMask) )
		{
			continue;
		}
		
		// Prime the node stack and start the test
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
		
		if ( FColl_fMoveMagnitude == 0.f )
		{
			_TestStaticCapsuleWithMesh( &pCapsule->m_vPoint1, &pCapsule->m_vPoint2, pCapsule->m_fRadius );
		}
		else
		{
			_TestDynamicCapsuleWithMesh( &pCapsule->m_vPoint1, &pCapsule->m_vPoint2, pCapsule->m_fRadius );
		}
			
		#if FMESH_COLL_ENABLE_COLLISION_DRAWS
			if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
			{
				CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
				frenderer_Push( FRENDERER_DRAW, NULL );
				fdraw_FacetedWireSphere( &vStart.v3, FColl_pkDOPTree2->BoundingCapsule.fRadius, &Color );
				fdraw_FacetedWireSphere( &vEnd.v3, FColl_pkDOPTree2->BoundingCapsule.fRadius, &Color );
				frenderer_Pop();
			}
		#endif // FMESH_DRAW_COLLIDED_KDOPS

		if ( nStartingImpactCount != FColl_nImpactCount && (FColl_pCollData->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
		{
			_TransformWorldImpacts( nStartingImpactCount );
			return TRUE;
		}
	}
	
	_TransformWorldImpacts( nStartingImpactCount );

	return (nStartingImpactCount < FColl_nImpactCount);
}


//
//
//
BOOL fcoll_TestMeshAndCapsule( CFWorldMesh *pTestWorldMesh, CFCapsule *pCapsule )
{
	FASSERT( pCapsule && pTestWorldMesh );

	if ( !pTestWorldMesh->m_pMesh->paCollTree )
	{
		return FALSE;
	}

	CFVec3A vStart, vEnd;
	f32 fRadius;
	BOOL bFoundImpacts = FALSE;
	FColl_pTestToWorld = &FColl_mtxTestBone;
	FColl_pTestMeshInst = pTestWorldMesh;
	FColl_pTestWorldMesh = pTestWorldMesh;
	FMesh_t *pMesh1 = pTestWorldMesh->m_pMesh;

	u16 nkDOPTree1Idx, nBoneIdx;
	for ( nkDOPTree1Idx = 0; nkDOPTree1Idx < pMesh1->nCollTreeCount; nkDOPTree1Idx++ )
	{
		FColl_pkDOPTree1 = &pMesh1->paCollTree[nkDOPTree1Idx];

		if ( !(FColl_pkDOPTree1->nMasterCollMask & FColl_pCollData->nCollMask) )
		{
			continue;
		}
		
		FMesh_t *pMesh = FColl_pTestMeshInst->m_pMesh;

		FColl_mtxTestBone, FColl_mtxTestInvBone, FColl_mtxBone, FColl_mtxTemp;

		// Get the bone info for the test mesh
		nBoneIdx = fmesh_coll_CalculateBoneMatrix( FColl_pTestMeshInst, nkDOPTree1Idx, &FColl_mtxTestBone, &FColl_mtxTestInvBone, &FColl_fBoneScale, &FColl_fInvBoneScale );

		// Record the current impact count so we can detect a change
		u16 nStartingImpactCount = FColl_nImpactCount;

		// Calculate the matrix to bring this bone into the other bone's space
		fRadius = pCapsule->m_fRadius * FColl_fInvBoneScale;
		vStart.Set( pCapsule->m_vPoint1 );
		FColl_mtxTestInvBone.MulPoint( vStart );
		vEnd.Set( pCapsule->m_vPoint2 );
		FColl_mtxTestInvBone.MulPoint( vEnd );

		// Prime the node stack and start the test
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
		
		if ( FColl_fMoveMagnitude == 0.f )
		{
			FColl_vTransMovement.Set( 0.f, 0.f, 0.f );
			FColl_fScaledMoveMagnitude = 0.f;
			_TestStaticCapsuleWithMesh( &vStart, &vEnd, fRadius );
		}
		else
		{
			FColl_mtxTestInvBone.MulDir( FColl_vTransMovement, *FColl_pCollData->pMovement );
			FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude * FColl_fInvBoneScale;
			_TestDynamicCapsuleWithMesh( &vStart, &vEnd, fRadius );
		}
			
		#if FMESH_COLL_ENABLE_COLLISION_DRAWS
			if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
			{
				frenderer_Push( FRENDERER_DRAW, NULL );
				CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
				CFVec3A vNewCenter;
				f32 fNewRadius = fRadius * FColl_fBoneScale;
				FColl_pTestToWorld->MulPoint( vNewCenter, vStart );
				fdraw_FacetedWireSphere( &vNewCenter.v3, fNewRadius, &Color );
				FColl_pTestToWorld->MulPoint( vNewCenter, vEnd );
				fdraw_FacetedWireSphere( &vNewCenter.v3, fNewRadius, &Color );
				frenderer_Pop();
			}
		#endif // FMESH_DRAW_COLLIDED_KDOPS

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

		// If we're only looking for the first impact, bail
		if ( nStartingImpactCount != FColl_nImpactCount && (FColl_pCollData->nStopOnFirstOfCollMask & FColl_nAccumCollMasks) )
		{
			break;
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
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3, 255, 255, 0 );
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
		FColl_pTestToWorld->MulDir( FColl_aImpactBuf[i].PushUnitVec.Mul( FColl_fInvBoneScale ) );
		FColl_aImpactBuf[i].nBoneIndex = (u8)nBoneIdx;
		FColl_aImpactBuf[i].fImpactDistInfo *= FColl_fBoneScale;
		
#if FANG_DEBUG_BUILD
		f32 fDist = FColl_aImpactBuf[i].PushUnitVec.Mag();
		if ( fDist < 0.997f || fDist > 1.003f )
		{
			FASSERT_NOW;
		}
#endif
		
		#if FMESH_DRAW_COLLIDED_TRIS
			CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
			fmesh_Coll_DrawMeshTri( FColl_aImpactBuf[i].aTriVtx, &FColl_aImpactBuf[i].UnitFaceNormal, NULL, &Color, TRUE );
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3, 255, 255, 0 );
		#endif					
	}
}


//
//	Function used to test capsule against kDOP trees
//
static void _TestDynamicCapsuleWithMesh( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius )
{
	FASSERT( FColl_pTestMeshInst && FColl_pkDOPTree1 && FColl_pCollData );
	FASSERT( FMesh_Coll_nNodeStackIdx );

	_vCapsuleNormal.Sub( *pEnd, *pStart );
	f32 fMagSq = _vCapsuleNormal.MagSq();
	if ( fMagSq < 0.001f )
	{
		return;
	}
	_vCapsuleNormal.Mul( fmath_InvSqrt( fMagSq ) );
	
	// Determine start and end intervals
	u16 i, nNorms = FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].nNormalCount;
	f32 afStartIntervals[FKDOP_MAX_AXES];
	f32 afEndIntervals[FKDOP_MAX_AXES];
	f32 afMoveAlongAxis[FKDOP_MAX_AXES];
	for ( i = 0; i < nNorms; i++ )
	{
		afStartIntervals[i] = pStart->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
		afEndIntervals[i]   = pEnd->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
		afMoveAlongAxis[i]  = FColl_vTransMovement.Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
	}

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
	
		// Make sure this capsule overlaps the intervals in all directions, taking into account movement
//		u16 i, nNorms = FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].nNormalCount;
		f32 fMax, fMin;
		for ( i = 0; i < nNorms; i++ )
		{
//			f32 fStart = pStart->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
//			f32 fEnd   = pEnd->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
			if ( afStartIntervals[i] < afEndIntervals[i] )
			{
				fMin = afStartIntervals[i] - fRadius;
				fMax = afEndIntervals[i]   + fRadius;
			}
			else
			{
				fMin = afEndIntervals[i]   - fRadius;
				fMax = afStartIntervals[i] + fRadius;
			}

//			f32 fMoveAlongAxis = FColl_vTransMovement.Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
			if ( afMoveAlongAxis[i] < 0.f )
			{
				if ( fMax < pWorldInt[i].fMin || fMin + afMoveAlongAxis[i] > pWorldInt[i].fMax )
				{
					// Capsule does not collide with kDOP, so we can stop traversal down this branch
					break;
				}
			}
			else
			{
				if ( fMax + afMoveAlongAxis[i] < pWorldInt[i].fMin || fMin > pWorldInt[i].fMax )
				{
					// Capsule does not collide with kDOP, so we can stop traversal down this branch
					break;
				}
			}
		}
		
		if ( i != nNorms )
		{
			continue;
		}
		
		if ( !pTestNode->paPackets )
		{
			// The world node is not a leaf, so push its children onto the stack
			FASSERT( pTestNode->nStartChildIdx + 1 < FColl_pkDOPTree1->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx + 1];
			continue;
		}

//		fmesh_Coll_AddCollDrawDOP( pWorldInt, NULL, FColl_pkDOPTree1->nTreekDOPType );

		// This is a leaf, so we should check the world mesh tris against the capsule
		if ( !_IntersectDynamicCapsuleAndTriangles( pTestNode, pStart, pEnd, fRadius ) )
		{
			// The function indicated we should stop searching
			return;
		}
	}
}


//
//
//
static void _TestStaticCapsuleWithMesh( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius )
{
	FASSERT( FColl_pTestMeshInst && FColl_pkDOPTree1 && FColl_pCollData );
	FASSERT( FMesh_Coll_nNodeStackIdx );

	_vCapsuleNormal.Sub( *pEnd, *pStart );
	f32 fMagSq = _vCapsuleNormal.MagSq();
	if ( fMagSq < 0.001f )
	{
		return;
	}
	_vCapsuleNormal.Mul( fmath_InvSqrt( fMagSq ) );
	
	u16 i, nNorms = FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].nNormalCount;
	f32 afStartIntervals[FKDOP_MAX_AXES];
	f32 afEndIntervals[FKDOP_MAX_AXES];
	for ( i = 0; i < nNorms; i++ )
	{
		afStartIntervals[i] = pStart->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
		afEndIntervals[i]   = pEnd->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
	}

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
	
		// Make sure this capsule overlaps the intervals in all directions, taking into account movement
//		u16 i, nNorms = FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].nNormalCount;
		f32 fMax, fMin;
		for ( i = 0; i < nNorms; i++ )
		{
//			f32 fStart = pStart->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
//			f32 fEnd   = pEnd->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
			if ( afStartIntervals[i] < afEndIntervals[i] )
			{
				fMin = afStartIntervals[i] - fRadius;
				fMax = afEndIntervals[i]   + fRadius;
			}
			else
			{
				fMin = afEndIntervals[i]   - fRadius;
				fMax = afStartIntervals[i] + fRadius;
			}

			if ( fMax < pWorldInt[i].fMin || fMin > pWorldInt[i].fMax )
			{
				// Capsule does not collide with kDOP, so we can stop traversal down this branch
				continue;
			}
		}
		
		if ( !pTestNode->paPackets )
		{
			// The world nodeis not a leaf, so push its children onto the stack
			FASSERT( pTestNode->nStartChildIdx + 1 < FColl_pkDOPTree1->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx + 1];
			continue;
		}

		// This is a leaf, so we should check the world mesh tris against the capsule
		if ( !_IntersectStaticCapsuleAndTriangles( pTestNode, pStart, pEnd, fRadius ) )
		{
			// The function indicated that we should stop searching
			return;
		}
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
//
//
static BOOL _IntersectDynamicCapsuleAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius )
{
	static CFVec3A vTemp1, vTemp2;
	CFVec3A *avTri, *pContact, *pTriNormal;
	CFVec3A vInCollPushUnitVec;

	#if FANG_PLATFORM_GC
		FkDOP_CNormal_t *pPacketNormals = (FkDOP_CNormal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
	#elif FANG_PLATFORM_DX
		FkDOP_Normal_t *pPacketNormals = (FkDOP_Normal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		FkDOP_Normal_t *pPacketNormals = (FkDOP_Normal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
//ARG - <<<<<
	#else
		FASSERT_NOW;
	#endif
		
	// Uncompress the verts & normals
	u8 nPack, nTri;
	u16 nNodeVert, *pNodeVertIdx = (u16 *)pTestNode->pTriData;
	for ( nPack = 0; nPack < pTestNode->nTriPacketCount; nPack++ )
	{
		// Check for collision buffer overflow
		if ( FColl_pNextImpactSlot == FColl_pImpactBufEnd )
		{
			#if !FANG_PRODUCTION_BUILD
				FColl_nImpactCountOverLimit++;
			#endif
			return FALSE;
		}
		
		FkDOP_TriPacket_t *pPacket = &pTestNode->paPackets[nPack];

		// Make sure this packet's mask matches
		if ( !(pPacket->nCollMask & FColl_pCollData->nCollMask) )
		{
			pPacketNormals += pPacket->nTriCount;
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

		// Setup initial collision pointers
		avTri     = FColl_pNextImpactSlot->aTriVtx;
		pContact  = &FColl_pNextImpactSlot->ImpactPoint;
		pTriNormal= &FColl_pNextImpactSlot->UnitFaceNormal;
		for ( nTri = 0; nTri < pPacket->nTriCount; nTri++, pPacketNormals++ )
		{
			nNodeVert = (u16)(pPacket->nStartVert + (nTri * 3));

			// Get the normal and verts in preparation for checks
			#if FANG_PLATFORM_GC
				pPacketNormals->DecompressTo( *pTriNormal );
			#elif FANG_PLATFORM_DX
				// Get the normal and verts in preparation for checks
				pTriNormal->Set( *pPacketNormals );
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				// Get the normal and verts in preparation for checks
				pTriNormal->Set( *pPacketNormals );
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			f32 fVelocityAlongTriNormal = FColl_vTransMovement.Dot( *pTriNormal );
			if ( (FColl_pCollData->nFlags & FCOLL_DATA_IGNORE_BACKSIDE) && fVelocityAlongTriNormal > 0.f )
			{
				continue;
			}

			#if FANG_PLATFORM_GC
				pVB->GetPoint( pNodeVertIdx[nNodeVert++], avTri[0].v3 );
				pVB->GetPoint( pNodeVertIdx[nNodeVert++], avTri[1].v3 );
				pVB->GetPoint( pNodeVertIdx[nNodeVert],   avTri[2].v3 );
			#elif FANG_PLATFORM_DX
				avTri[0].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
				avTri[1].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
				avTri[2].v3 = pVerts[ pNodeVertIdx[nNodeVert] ];
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				avTri[0].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
				avTri[1].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
				avTri[2].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert] ]);
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			u8  nTriVertContacts = 0;
			u8	nCapsuleContacts = 0;
			u8  nMax, nMin;
			BOOL8 bContactMade = TRUE, bInitiallyInContact = TRUE;
			f32 fMin, fMax, fInCollisionDistInfo;

			// Determine whether or not the capsule can intersect the plane
			FColl_vDiff1.Sub( *pStart, avTri[0] );
			FColl_vDiff2.Sub( *pEnd, avTri[0] );

			FColl_pNextImpactSlot->fUnitImpactTime = -FMATH_MAX_FLOAT;
			FColl_pNextImpactSlot->fImpactDistInfo = FMATH_MAX_FLOAT;
			fInCollisionDistInfo = FMATH_MAX_FLOAT;

			f32 fDist1 = FColl_vDiff1.Dot( *pTriNormal );
			f32 fDist2 = FColl_vDiff2.Dot( *pTriNormal );
			if ( fmath_Abs( fDist1 - fDist2 ) < 0.05f )
			{
				fMin = fDist1 - fRadius;
				nMin = 3;
				fMax = fDist2 + fRadius;
				nMax = 3;
			}
			else if ( fDist1 < fDist2 )
			{
				fMin = fDist1 - fRadius;
				nMin = 1;
				fMax = fDist2 + fRadius;
				nMax = 2;
			}
			else
			{
				fMin = fDist2 - fRadius;
				nMin = 2;
				fMax = fDist1 + fRadius;
				nMax = 1;
			}

			if ( fMax < 0.f )
			{
				if ( fVelocityAlongTriNormal <= 0.f )
				{
					continue;
				}
				else if ( fMax + fVelocityAlongTriNormal < 0.f )
				{
					continue;
				}

				bInitiallyInContact = FALSE;
				FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( *pTriNormal );
				FColl_pNextImpactSlot->fUnitImpactTime = fmath_Div( -fMax, fVelocityAlongTriNormal );
				FColl_pNextImpactSlot->fImpactDistInfo = fVelocityAlongTriNormal + fMax;
				nTriVertContacts = 0x07;
				nCapsuleContacts |= nMax;
			}
			else if ( fMin > 0.f )
			{
				if ( fVelocityAlongTriNormal >= 0.f )
				{
					continue;
				}
				else if ( fMin + fVelocityAlongTriNormal > 0.f )
				{
					continue;
				}

				bInitiallyInContact = FALSE;
				FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
				FColl_pNextImpactSlot->fUnitImpactTime = fmath_Div( -fMin, fVelocityAlongTriNormal );
				FColl_pNextImpactSlot->fImpactDistInfo = -fVelocityAlongTriNormal - fMin;
				nTriVertContacts = 0x07;
				nCapsuleContacts |= nMin;
			}
			else
			{
				// Already in collision state
				if ( fMin + fVelocityAlongTriNormal >= 0.f )
				{
					// Tri will leave contact due to velocity
					continue;
				}
				vInCollPushUnitVec.Set( *pTriNormal );
				fInCollisionDistInfo = -fMin;
				FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
				FColl_pNextImpactSlot->fImpactDistInfo = -fMin - fVelocityAlongTriNormal;
				nTriVertContacts = 0x07;
				nCapsuleContacts |= nMin;
			}

			FColl_vNormal.Sub( *pEnd, *pStart );
			FColl_vNormal.Unitize();

			u8 nEdgeVert1, nAxis, nMinTriVertPoints, nMaxTriVertPoints, nEdgeVert0 = 2;
			f32 fTriCrossDot[3], fMinInterval, fMaxInterval, fTest1, fTest2;

			// First test the axis created by crossing the face normal with the normal created by the capsule centerline
			FColl_vCross.Cross( *pTriNormal, FColl_vNormal );
			fTest1 = FColl_vCross.MagSq();
			if ( fTest1 > 0.0001f )
			{
				FColl_vCross.Mul( fmath_InvSqrt( fTest1 ) );
				
				f32 fVelocityAlongAxis = FColl_vTransMovement.Dot( FColl_vCross );
				
				// Get min and max intervals for triangle
				fTriCrossDot[0] = avTri[0].Dot( FColl_vCross );
				fTriCrossDot[1] = avTri[1].Dot( FColl_vCross );
				fTriCrossDot[2] = avTri[2].Dot( FColl_vCross );
				fMinInterval = _GetSmallestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMinTriVertPoints );
				fMaxInterval = _GetLargestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMaxTriVertPoints );

				fTest1 = pStart->Dot( FColl_vCross );
				fTest2 = pEnd->Dot( FColl_vCross );
				if ( fmath_Abs( fTest1 - fTest2 ) < 0.05f )
				{
					fMin = fTest1 - fRadius;
					nMin = 3;
					fMax = fTest2 + fRadius;
					nMax = 3;
				}
				else if ( fTest1 < fTest2 )
				{
					fMin = fTest1 - fRadius;
					nMin = 1;
					fMax = fTest2 + fRadius;
					nMax = 2;
				}
				else
				{
					fMin = fTest2 - fRadius;
					nMin = 2;
					fMax = fTest1 + fRadius;
					nMax = 1;
				}

				if ( fMax < fMinInterval )
				{
					if ( fVelocityAlongAxis < 0.f )
					{
						// This axis separates the tri and the capsule
						continue;
					}
					else if ( fMax + fVelocityAlongAxis < fMinInterval )
					{
						// This axis separates the tri and the capsule
						continue;
					}
		            
					fTest1 = fmath_Div( fMinInterval - fMax, fVelocityAlongAxis );
					if ( fTest1 > FColl_pNextImpactSlot->fUnitImpactTime )
					{
						bInitiallyInContact = FALSE;
						FColl_pNextImpactSlot->fUnitImpactTime = fTest1;
						nTriVertContacts = nMinTriVertPoints;
						nCapsuleContacts = nMax;
					}
					fTest1 = fVelocityAlongAxis - (fMinInterval - fMax);
					if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
					{
						FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
						FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
					}
				}
				else if ( fMin > fMaxInterval )
				{
					if ( fVelocityAlongAxis > 0.f )
					{
						// This axis separates the tri and the capsule
						continue;
					}
					else if ( fMin + fVelocityAlongAxis > fMaxInterval )
					{
						// This axis separates the tri and the capsule
						continue;
					}
		            
					fTest1 = fmath_Div( fMaxInterval - fMin, fVelocityAlongAxis );
					if ( fTest1 > FColl_pNextImpactSlot->fUnitImpactTime  )
					{
						bInitiallyInContact = FALSE;
						FColl_pNextImpactSlot->fUnitImpactTime = fTest1;
						nTriVertContacts = nMaxTriVertPoints;
						nCapsuleContacts = nMin;
					}
					fTest1 = -fVelocityAlongAxis - (fMin - fMaxInterval);
					if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
					{
						FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
						FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
					}
				}
				else// if ( bInitiallyInContact )
				{
					// Already in collision state
					if ( fmath_Abs( fMin - fMaxInterval ) < fmath_Abs( fMax - fMinInterval ) )
					{
						if ( fMin + fVelocityAlongAxis > fMaxInterval )
						{
							// capsule will leave contact due to velocity
							continue;
						}
						fTest1 = fmath_Abs( fMaxInterval - fMin );
						if ( bInitiallyInContact && fTest1 < fInCollisionDistInfo  )
						{
							fInCollisionDistInfo = fTest1;
							vInCollPushUnitVec.Set( FColl_vCross );
							nTriVertContacts = nMaxTriVertPoints;
							nCapsuleContacts |= nMin;
						}
						fTest1 = fTest1 - fVelocityAlongAxis;
						if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
						{
							FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
							FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
						}
					}
					else
					{
						if ( fMax + fVelocityAlongAxis < fMinInterval )
						{
							// Capsule will leave contact due to velocity
							continue;
						}
						fTest1 = fmath_Abs( fMinInterval - fMax );
						if ( bInitiallyInContact && fTest1 < fInCollisionDistInfo )
						{
							fInCollisionDistInfo = fTest1;
							vInCollPushUnitVec.ReceiveNegative( FColl_vCross );
							nTriVertContacts = nMinTriVertPoints;
							nCapsuleContacts |= nMax;
						}
						fTest1 = fTest1 + fVelocityAlongAxis;
						if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
						{
							FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
							FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
						}
					}
				}
			}

			CFVec3A *pAxis[2];
			pAxis[0] = pTriNormal;
			pAxis[1] = &FColl_vNormal;
			
			for ( nEdgeVert1 = 0; nEdgeVert1 < 3; nEdgeVert0 = nEdgeVert1, nEdgeVert1++ )
			{
				for ( nAxis = 0; nAxis < 2; nAxis++ )
				{
					FColl_vDiff1.Sub( avTri[nEdgeVert1], avTri[nEdgeVert0] );

					FColl_vCross.Cross( *pAxis[nAxis], FColl_vDiff1 );
					fTest1 = FColl_vCross.MagSq();
					if ( fTest1 < 0.001f )
					{
						continue;
					}
					FColl_vCross.Mul( fmath_InvSqrt( fTest1 ) );
					
					f32 fVelocityAlongAxis = FColl_vTransMovement.Dot( FColl_vCross );
					
					// Get min and max intervals for triangle
					fTriCrossDot[0] = avTri[0].Dot( FColl_vCross );
					fTriCrossDot[1] = avTri[1].Dot( FColl_vCross );
					fTriCrossDot[2] = avTri[2].Dot( FColl_vCross );
					fMinInterval = _GetSmallestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMinTriVertPoints );
					fMaxInterval = _GetLargestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMaxTriVertPoints );

					fTest1 = pStart->Dot( FColl_vCross );
					fTest2 = pEnd->Dot( FColl_vCross );
					if ( fmath_Abs( fTest1 - fTest2 ) < 0.05f )
					{
						fMin = fTest1 - fRadius;
						nMin = 3;
						fMax = fTest2 + fRadius;
						nMax = 3;
					}
					else if ( fTest1 < fTest2 )
					{
						fMin = fTest1 - fRadius;
						nMin = 1;
						fMax = fTest2 + fRadius;
						nMax = 2;
					}
					else
					{
						fMin = fTest2 - fRadius;
						nMin = 2;
						fMax = fTest1 + fRadius;
						nMax = 1;
					}

					if ( fMax < fMinInterval )
					{
						if ( fVelocityAlongAxis < 0.f )
						{
							// Tri will not contact
							bContactMade = FALSE;
							break;
						}
						else if ( fMax + fVelocityAlongAxis < fMinInterval )
						{
							// Tri will not contact
							bContactMade = FALSE;
							break;
						}
	                    
						fTest1 = fmath_Div( fMinInterval - fMax, fVelocityAlongAxis );
						if ( fTest1 > FColl_pNextImpactSlot->fUnitImpactTime )
						{
							bInitiallyInContact = FALSE;
							FColl_pNextImpactSlot->fUnitImpactTime = fTest1;
							nTriVertContacts = nMinTriVertPoints;
							nCapsuleContacts = nMax;
						}
						fTest1 = fVelocityAlongAxis - (fMinInterval - fMax);
						if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
						{
							FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
							FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
						}
					}
					else if ( fMin > fMaxInterval )
					{
						if ( fVelocityAlongAxis > 0.f )
						{
							// Tri will not contact
							bContactMade = FALSE;
							break;
						}
						else if ( fMin + fVelocityAlongAxis > fMaxInterval )
						{
							// Tri will not contact
							bContactMade = FALSE;
							break;
						}
	                    
						fTest1 = fmath_Div( fMaxInterval - fMin, fVelocityAlongAxis );
						if ( fTest1 > FColl_pNextImpactSlot->fUnitImpactTime  )
						{
							bInitiallyInContact = FALSE;
							FColl_pNextImpactSlot->fUnitImpactTime = fTest1;
							nTriVertContacts = nMaxTriVertPoints;
							nCapsuleContacts = nMin;
						}
						fTest1 = -fVelocityAlongAxis - (fMin - fMaxInterval);
						if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
						{
							FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
							FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
						}
					}
					else// if ( bInitiallyInContact )
					{
						// Already in collision state
						if ( fmath_Abs( fMin - fMaxInterval ) < fmath_Abs( fMax - fMinInterval ) )
						{
							if ( fMin + fVelocityAlongAxis > fMaxInterval )
							{
								// kDOP will leave contact due to velocity
								bContactMade = FALSE;
								break;
							}
							fTest1 = fmath_Abs( fMaxInterval - fMin );
							if ( bInitiallyInContact && fTest1 < fInCollisionDistInfo  )
							{
								fInCollisionDistInfo = fTest1;
								vInCollPushUnitVec.Set( FColl_vCross );
								nTriVertContacts = nMaxTriVertPoints;
								nCapsuleContacts |= nMin;
							}
							fTest1 = fTest1 - fVelocityAlongAxis;
							if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
							{
								FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
								FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
							}
						}
						else
						{
							if ( fMax + fVelocityAlongAxis < fMinInterval )
							{
								// kDOP will leave contact due to velocity
								bContactMade = FALSE;
								break;
							}
							fTest1 = fmath_Abs( fMinInterval - fMax );
							if ( bInitiallyInContact && fTest1 < fInCollisionDistInfo )
							{
								fInCollisionDistInfo = fTest1;
								vInCollPushUnitVec.ReceiveNegative( FColl_vCross );
								nTriVertContacts = nMinTriVertPoints;
								nCapsuleContacts |= nMax;
							}
							fTest1 = fTest1 + fVelocityAlongAxis;
							if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
							{
								FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
								FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
							}
						}
					}
				}

				if ( !bContactMade )
				{
					break;
				}
			}

			if ( !bContactMade )
			{
				continue;
			}

			if ( bInitiallyInContact )
			{
				FColl_pNextImpactSlot->fUnitImpactTime = -1.f;
				FColl_pNextImpactSlot->PushUnitVec.Set( vInCollPushUnitVec );
				FColl_pNextImpactSlot->fImpactDistInfo = fInCollisionDistInfo;
			}

			if ( FColl_pNextImpactSlot->fUnitImpactTime == -FMATH_MAX_FLOAT )
			{
//				FASSERT_NOW;
				continue;
			}

			// WE'VE GOT COLLISION!!!

			// Check the validity of some results
			FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.f );
			FASSERT( FColl_pNextImpactSlot->fUnitImpactTime >= -0.001f || FColl_pNextImpactSlot->fUnitImpactTime == -1.f );

			if ( FColl_pNextImpactSlot->fUnitImpactTime != -1.f )
			{
				FASSERT( FColl_pNextImpactSlot->fUnitImpactTime <= 1.01f );
				FMATH_CLAMP( FColl_pNextImpactSlot->fUnitImpactTime, 0.f, 1.f );
			}

			// Calculate the contact point
			if ( !(FColl_pCollData->nFlags & FCOLL_DATA_DONT_CALCULATE_IMPACT) )
			{
				if ( nTriVertContacts == 0x07 )
				{
					// There was a collision with the face...
					pContact->Mul( FColl_pNextImpactSlot->PushUnitVec, -fRadius);
					if ( nCapsuleContacts == 3 )
					{
						FColl_vDiff1.Add( *pStart, *pEnd ).Mul( 0.5f );
						pContact->Add( FColl_vDiff1 );
					}
					else if ( nCapsuleContacts == 1 )
					{
						pContact->Add( *pStart );
					}
					else
					{
						pContact->Add( *pEnd );
					}
				}
				else if ( nTriVertContacts == 0x01 || nTriVertContacts == 0x02 || nTriVertContacts == 0x04 )
				{
					// One point of the triangle collides with the capsule, so this is the collision point.
					if ( nTriVertContacts & 0x01 )
					{
						pContact->Set( avTri[0] );
					}
					else if ( nTriVertContacts & 0x02 )
					{
						pContact->Set( avTri[1] );
					}
					else if ( nTriVertContacts & 0x04 )
					{
						pContact->Set( avTri[2] );
					}
				}
				else
				{
					// Otherwise, one of the triangle's edges contacts the capsule, so calculate the
					// closest point on the edge to the capsule centerline.  That is the collision.
					CFVec3A *pEdgeVert[2];
					nEdgeVert0 = 0;
					if ( nTriVertContacts & 0x01 )
					{
						pEdgeVert[nEdgeVert0++] = &avTri[0];
					}
					if ( nTriVertContacts & 0x02 )
					{
						pEdgeVert[nEdgeVert0++] = &avTri[1];
					}
					if ( nTriVertContacts & 0x04 )
					{
						FASSERT( nEdgeVert0 == 1 );
						pEdgeVert[nEdgeVert0++] = &avTri[2];
					}

					FColl_vODiff.Sub( *pEnd, *pStart );
					FColl_vNormal.Sub( *pEdgeVert[1], *pEdgeVert[0] );

					FColl_vCross.Cross( FColl_vODiff, FColl_vNormal );
					FColl_vCross.Cross( FColl_vNormal );
					fDist1 = FColl_vCross.MagSq();
					if ( fDist1 == 0.f )
					{
						// Edges are parallel - Use the average of the centerline
						pContact->Add( *pStart, *pEnd ).Mul( 0.5f );
					}
					else
					{
						FColl_vCross.Mul( fmath_InvSqrt( fDist1 ) );

						FColl_vDiff1.Sub( *pStart, *pEdgeVert[0] );
						fDist1 = FColl_vCross.Dot( FColl_vDiff1 );
						FColl_vDiff1.Sub( *pEnd, *pEdgeVert[0] );
						fDist2 = FColl_vCross.Dot( FColl_vDiff1 );

						if ( (fDist1 > 0) != (fDist2 > 0) )
						{
							FColl_vODiff.Mul( fmath_Abs(fmath_Div( fDist1, fDist1 - fDist2 )) ).Add( *pStart );
							FColl_vDiff1.Sub( FColl_vODiff, *pEdgeVert[0] );
						}
						else if ( fmath_Abs( fDist1 ) < fmath_Abs( fDist2 ) )
						{
							FColl_vDiff1.Sub( *pStart, *pEdgeVert[0] );
						}
						else
						{
							FColl_vDiff1.Sub( *pEnd, *pEdgeVert[0] );
						}
						FColl_vNormal.Unitize();
						pContact->Mul( FColl_vNormal, FColl_vNormal.Dot( FColl_vDiff1 ) ).Add( *pEdgeVert[0] );
					}
				}
			}

			FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
			FColl_pNextImpactSlot->pTag = (void *)FColl_pTestWorldMesh;
			FColl_pNextImpactSlot->nSourceBoneIndex = FColl_nCurrentBoneIdx;
			
			FColl_nAccumCollMasks |= pPacket->nCollMask;

			FColl_pNextImpactSlot++;
			FColl_nImpactCount++;
			
			// If only first impact was requested, bail out
			if ( FColl_pCollData->nStopOnFirstOfCollMask & pPacket->nCollMask )
			{
				return FALSE;
			}

			// Check for collision buffer overflow
			if ( FColl_pNextImpactSlot == FColl_pImpactBufEnd )
			{
				#if !FANG_PRODUCTION_BUILD
					FColl_nImpactCountOverLimit++;
				#endif
				return FALSE;
			}

			// Setup new collision pointers
			avTri     = FColl_pNextImpactSlot->aTriVtx;
			pContact  = &FColl_pNextImpactSlot->ImpactPoint;
			pTriNormal= &FColl_pNextImpactSlot->UnitFaceNormal;
		}
	}

	return TRUE;
}


//
//	Function used to test sphere against kDOP trees
//
static BOOL _IntersectStaticCapsuleAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius )
{
	static CFVec3A vTemp1, vTemp2;
	CFVec3A *avTri, *pContact, *pTriNormal;

	#if FANG_PLATFORM_GC
		FkDOP_CNormal_t *pPacketNormals = (FkDOP_CNormal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
	#elif FANG_PLATFORM_DX
		FkDOP_Normal_t *pPacketNormals = (FkDOP_Normal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
//ARG - >>>>>
	#elif FANG_PLATFORM_PS2
		FkDOP_Normal_t *pPacketNormals = (FkDOP_Normal_t *)((u32)pTestNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pTestNode->nTriCount * 3, 4));
//ARG - <<<<<
	#else
		FASSERT_NOW;
	#endif
		
	// Uncompress the verts & normals
	u8 nPack, nTri;
	u16 nNodeVert, *pNodeVertIdx = (u16 *)pTestNode->pTriData;
	for ( nPack = 0; nPack < pTestNode->nTriPacketCount; nPack++ )
	{
		// Check for collision buffer overflow
		if ( FColl_pNextImpactSlot == FColl_pImpactBufEnd )
		{
			#if !FANG_PRODUCTION_BUILD
				FColl_nImpactCountOverLimit++;
			#endif
			return FALSE;
		}
		
		FkDOP_TriPacket_t *pPacket = &pTestNode->paPackets[nPack];

		// Make sure this packet's mask matches
		if ( !(pPacket->nCollMask & FColl_pCollData->nCollMask) )
		{
			pPacketNormals += pPacket->nTriCount;
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

		// Setup initial collision pointers
		avTri     = FColl_pNextImpactSlot->aTriVtx;
		pContact  = &FColl_pNextImpactSlot->ImpactPoint;
		pTriNormal= &FColl_pNextImpactSlot->UnitFaceNormal;
		for ( nTri = 0; nTri < pPacket->nTriCount; nTri++, pPacketNormals++ )
		{
			nNodeVert = (u16)(pPacket->nStartVert + (nTri * 3));

			// Get the normal and first vert in preparation for checks
			#if FANG_PLATFORM_GC
				pPacketNormals->DecompressTo( *pTriNormal );
				pVB->GetPoint( pNodeVertIdx[nNodeVert++], avTri[0].v3 );
				pVB->GetPoint( pNodeVertIdx[nNodeVert++], avTri[1].v3 );
				pVB->GetPoint( pNodeVertIdx[nNodeVert],   avTri[2].v3 );
			#elif FANG_PLATFORM_DX
				pTriNormal->Set( *pPacketNormals );
				avTri[0].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
				avTri[1].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
				avTri[2].v3 = pVerts[ pNodeVertIdx[nNodeVert] ];
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				pTriNormal->Set( *pPacketNormals );
				avTri[0].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
				avTri[1].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
				avTri[2].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert] ]);
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			// Determine whether or not the capsule can intersect the plane
			FColl_vDiff1.Sub( *pStart, avTri[0] );
			FColl_vDiff2.Sub( *pEnd, avTri[0] );

			u8  nTriVertContacts = 0;
			u8	nCapsuleContacts = 0;
			u8  nMax, nMin;
			BOOL8 bBackside, bContactMade = TRUE;
			f32 fMin, fMax;
			f32 fDist1 = FColl_vDiff1.Dot( *pTriNormal );
			f32 fDist2 = FColl_vDiff2.Dot( *pTriNormal );
			if ( fDist1 < fDist2 )
			{
				fMin = fDist1 - fRadius;
				nMin = 1;
				fMax = fDist2 + fRadius;
				nMax = 2;
			}
			else
			{
				fMin = fDist2 - fRadius;
				nMin = 2;
				fMax = fDist1 + fRadius;
				nMax = 1;
			}

			if ( fMax < 0.f || fMin > 0.f )
			{
				continue;
			}

			bBackside = FALSE;
			FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
			FColl_pNextImpactSlot->fImpactDistInfo = -fMin;
			nTriVertContacts = 0x07;
			nCapsuleContacts = nMin;

			FColl_vNormal.Sub( *pEnd, *pStart );
			FColl_vNormal.Unitize();

			u8 nEdgeVert1, nAxis, nMinTriVertPoints, nMaxTriVertPoints, nEdgeVert0 = 2;
			f32 fTriCrossDot[3], fMinInterval, fMaxInterval, fTest1, fTest2;

			FColl_vCross.Cross( *pTriNormal, FColl_vNormal );
			fTest1 = FColl_vCross.MagSq();
			if ( fTest1 > 0.0001f )
			{
				FColl_vCross.Mul( fmath_InvSqrt( fTest1 ) );
				
				// Get min and max intervals for triangle
				fTriCrossDot[0] = avTri[0].Dot( FColl_vCross );
				fTriCrossDot[1] = avTri[1].Dot( FColl_vCross );
				fTriCrossDot[2] = avTri[2].Dot( FColl_vCross );
				fMinInterval = _GetSmallestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMinTriVertPoints );
				fMaxInterval = _GetLargestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMaxTriVertPoints );

				fTest1 = pStart->Dot( FColl_vCross );
				fTest2 = pEnd->Dot( FColl_vCross );
				if ( fTest1 < fTest2 )
				{
					fMin = fTest1 - fRadius;
					nMin = 1;
					fMax = fTest2 + fRadius;
					nMax = 2;
				}
				else
				{
					fMin = fTest2 - fRadius;
					nMin = 2;
					fMax = fTest1 + fRadius;
					nMax = 1;
				}

				if ( fMax < fMinInterval || fMin > fMaxInterval )
				{
					// This axis separates the capsule and the triangle
					continue;
				}

				if ( fmath_Abs( fMax - fMinInterval ) < fmath_Abs( fMin - fMaxInterval ) )
				{
					fTest1 = fmath_Abs( fMax - fMinInterval );
					if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
					{
						FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
						FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
						nTriVertContacts = nMinTriVertPoints;
						nCapsuleContacts = nMax;
					}
				}
				else
				{
					fTest1 = fmath_Abs( fMaxInterval - fMin );
					if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
					{
						FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
						FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
						nTriVertContacts = nMaxTriVertPoints;
						nCapsuleContacts = nMin;
					}
				}
			}

			CFVec3A *pAxis[2];
			pAxis[0] = pTriNormal;
			pAxis[1] = &FColl_vNormal;
			
			for ( nEdgeVert1 = 0; nEdgeVert1 < 3; nEdgeVert0 = nEdgeVert1, nEdgeVert1++ )
			{
				for ( nAxis = 0; nAxis < 2; nAxis++ )
				{
					FColl_vDiff1.Sub( avTri[nEdgeVert1], avTri[nEdgeVert0] );

					FColl_vCross.Cross( *pAxis[nAxis], FColl_vDiff1 );
					fTest1 = FColl_vCross.MagSq();
					if ( fTest1 < 0.001f )
					{
						continue;
					}
					FColl_vCross.Mul( fmath_InvSqrt( fTest1 ) );
					
					// Get min and max intervals for triangle
					fTriCrossDot[0] = avTri[0].Dot( FColl_vCross );
					fTriCrossDot[1] = avTri[1].Dot( FColl_vCross );
					fTriCrossDot[2] = avTri[2].Dot( FColl_vCross );
					fMinInterval = _GetSmallestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMinTriVertPoints );
					fMaxInterval = _GetLargestVertDot( fTriCrossDot[0], fTriCrossDot[1], fTriCrossDot[2], &nMaxTriVertPoints );

					fTest1 = pStart->Dot( FColl_vCross );
					fTest2 = pEnd->Dot( FColl_vCross );
					if ( fTest1 < fTest2 )
					{
						fMin = fTest1 - fRadius;
						nMin = 1;
						fMax = fTest2 + fRadius;
						nMax = 2;
					}
					else
					{
						fMin = fTest2 - fRadius;
						nMin = 2;
						fMax = fTest1 + fRadius;
						nMax = 1;
					}

					if ( fMax < fMinInterval || fMin > fMaxInterval )
					{
						// Tri is not in contact
						bContactMade = FALSE;
						break;
					}

					if ( fmath_Abs( fMax - fMinInterval ) < fmath_Abs( fMin - fMaxInterval ) )
					{
						fTest1 = fmath_Abs( fMax - fMinInterval );
						if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
						{
							FColl_pNextImpactSlot->PushUnitVec.ReceiveNegative( FColl_vCross );
							FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
							nTriVertContacts = nMinTriVertPoints;
							nCapsuleContacts = nMax;
						}
					}
					else
					{
						fTest1 = fmath_Abs( fMaxInterval - fMin );
						if ( fTest1 < FColl_pNextImpactSlot->fImpactDistInfo )
						{
							FColl_pNextImpactSlot->PushUnitVec.Set( FColl_vCross );
							FColl_pNextImpactSlot->fImpactDistInfo = fTest1;
							nTriVertContacts = nMaxTriVertPoints;
							nCapsuleContacts = nMin;
						}
					}
				}

				if ( !bContactMade )
				{
					break;
				}
			}

			if ( !bContactMade )
			{
				continue;
			}

			// WE'VE GOT COLLISION!!!

			// Calculate the contact point
			if ( !(FColl_pCollData->nFlags & FCOLL_DATA_DONT_CALCULATE_IMPACT) )
			{
				if ( nTriVertContacts == 0x07 )
				{
					// There was a collision with the face...
					pContact->Mul( FColl_pNextImpactSlot->PushUnitVec, -fRadius);
					if ( nCapsuleContacts == 3 )
					{
						FColl_vDiff1.Add( *pStart, *pEnd ).Mul( 0.5f );
						pContact->Add( FColl_vDiff1 );
					}
					else if ( nCapsuleContacts == 1 )
					{
						pContact->Add( *pStart );
					}
					else
					{
						pContact->Add( *pEnd );
					}
				}
				else if ( nTriVertContacts == 0x01 || nTriVertContacts == 0x02 || nTriVertContacts == 0x04 )
				{
					// One point of the triangle collides with the capsule, so this is the collision point.
					if ( nTriVertContacts & 0x01 )
					{
						pContact->Set( avTri[0] );
					}
					else if ( nTriVertContacts & 0x02 )
					{
						pContact->Set( avTri[1] );
					}
					else if ( nTriVertContacts & 0x04 )
					{
						pContact->Set( avTri[2] );
					}
				}
				else
				{
					// Otherwise, one of the triangle's edges contacts the capsule, so calculate the
					// closest point on the edge to the capsule centerline.  That is the collision.
					CFVec3A *pEdgeVert[2];
					nEdgeVert0 = 0;
					if ( nTriVertContacts & 0x01 )
					{
						pEdgeVert[nEdgeVert0++] = &avTri[0];
					}
					if ( nTriVertContacts & 0x02 )
					{
						pEdgeVert[nEdgeVert0++] = &avTri[1];
					}
					if ( nTriVertContacts & 0x04 )
					{
						FASSERT( nEdgeVert0 == 1 );
						pEdgeVert[nEdgeVert0++] = &avTri[2];
					}
					FColl_vODiff.Sub( *pEnd, *pStart );
					FColl_vNormal.Sub( *pEdgeVert[1], *pEdgeVert[0] );

					FColl_vCross.Cross( FColl_vODiff, FColl_vNormal );
					FColl_vCross.Cross( FColl_vNormal );
					fDist1 = FColl_vCross.MagSq();
					if ( fDist1 == 0.f )
					{
						// Edges are parallel - Use the average of the centerline
						pContact->Add( *pStart, *pEnd ).Mul( 0.5f );
					}
					else
					{
						FColl_vCross.Mul( fmath_InvSqrt( fDist1 ) );

						FColl_vDiff1.Sub( *pStart, *pEdgeVert[0] );
						fDist1 = FColl_vCross.Dot( FColl_vDiff1 );
						FColl_vDiff1.Sub( *pEnd, *pEdgeVert[0] );
						fDist2 = FColl_vCross.Dot( FColl_vDiff1 );

						if ( (fDist1 > 0) != (fDist2 > 0) )
						{
							FColl_vODiff.Mul( fmath_Abs(fmath_Div( fDist1, fDist1 - fDist2 )) ).Add( *pStart );
							FColl_vDiff1.Sub( FColl_vODiff, *pEdgeVert[0] );
						}
						else if ( fmath_Abs( fDist1 ) < fmath_Abs( fDist2 ) )
						{
							FColl_vDiff1.Sub( *pStart, *pEdgeVert[0] );
						}
						else
						{
							FColl_vDiff1.Sub( *pEnd, *pEdgeVert[0] );
						}
						FColl_vNormal.Unitize();
						pContact->Mul( FColl_vNormal, FColl_vNormal.Dot( FColl_vDiff1 ) ).Add( *pEdgeVert[0] );
					}
				}
			}

			// All static collisions occur at impact time == -1.f (they must be initially in a collision state)
			FColl_pNextImpactSlot->fUnitImpactTime = -1.f;

			FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
			FColl_pNextImpactSlot->pTag = (void *)FColl_pTestWorldMesh;
			FColl_pNextImpactSlot->nSourceBoneIndex = FColl_nCurrentBoneIdx;
			
			FColl_nAccumCollMasks |= pPacket->nCollMask;

			FColl_pNextImpactSlot++;
			FColl_nImpactCount++;
			
			// If only first impact was requested, bail out
			if ( FColl_pCollData->nStopOnFirstOfCollMask & pPacket->nCollMask )
			{
				return FALSE;
			}

			// Check for collision buffer overflow
			if ( FColl_pNextImpactSlot == FColl_pImpactBufEnd )
			{
				#if !FANG_PRODUCTION_BUILD
					FColl_nImpactCountOverLimit++;
				#endif
				return FALSE;
			}

			// Setup new collision pointers
			avTri     = FColl_pNextImpactSlot->aTriVtx;
			pContact  = &FColl_pNextImpactSlot->ImpactPoint;
			pTriNormal= &FColl_pNextImpactSlot->UnitFaceNormal;
		}
	}

	return TRUE;
}
