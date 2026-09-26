//////////////////////////////////////////////////////////////////////////////////////
// fcoll_Sphere.cpp - C code for handling sphere collision
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


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static void _TestStaticSphereWithMesh( const CFVec3A *pCenter, f32 fRadius );
static BOOL _IntersectStaticSphereAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pCenter, f32 fRadius );
static void _TestDynamicSphereWithMesh( const CFVec3A *pCenter, f32 fRadius );
static BOOL _IntersectDynamicSphereAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pCenter, f32 fRadius );
static void _ProjectSphereAgainstEdge( const CFVec3A &vSphereCenter, f32 fRadius, const CFVec3A &vTravel, const CFVec3A &vVert0, const CFVec3A &vVert1, f32 &fT, CFVec3A &vImpact );

static FINLINE void _TransformWorldImpacts( u16 nStart );
static FINLINE void _TransformImpacts( u16 nStart, u16 nBoneIdx );

//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fcoll_TestWorldMeshAndSphere( const CFMeshInst *pWorldMeshInst, const CFSphere *pSphere )
{
	FASSERT( pSphere && pWorldMeshInst );
	
	if ( !pWorldMeshInst->m_pMesh->paCollTree )
	{
		return FALSE;
	}
	
	CFVec3A vCenter;
	FColl_pTestToWorld = NULL;
	FColl_pTestMeshInst = pWorldMeshInst;
	FColl_pTestWorldMesh = NULL;
	FMesh_t *pMesh1 = pWorldMeshInst->m_pMesh;

	u16 nStartingImpactCount = FColl_nImpactCount;

	vCenter.Set( pSphere->m_Pos );

	if ( FColl_fMoveMagnitude == 0.f )
	{
		FColl_vTransMovement.Set( 0.f, 0.f, 0.f );
	}
	else
	{
		FColl_vTransMovement.Set( *FColl_pCollData->pMovement );
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
			_TestStaticSphereWithMesh( &vCenter, pSphere->m_fRadius );
		}
		else
		{
			_TestDynamicSphereWithMesh( &vCenter, pSphere->m_fRadius );
		}
			
		#if FMESH_COLL_ENABLE_COLLISION_DRAWS
			if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
			{
				CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
				frenderer_Push( FRENDERER_DRAW, NULL );
				fdraw_FacetedWireSphere( &vCenter.v3, pSphere->m_fRadius, &Color );
				frenderer_Pop();
			}
		#endif // FMESH_DRAW_COLLIDED_KDOPS

		if ( nStartingImpactCount != FColl_nImpactCount && (FColl_pCollData->nStopOnFirstOfCollMask & FColl_nAccumCollMasks ) )
		{
			break;
		}
	}
	
	_TransformWorldImpacts( nStartingImpactCount );

	return (nStartingImpactCount < FColl_nImpactCount);
}


//
//
//
BOOL fcoll_TestMeshAndSphere( const CFWorldMesh *pTestWorldMesh, const CFSphere *pSphere )
{
	FASSERT( pSphere && pTestWorldMesh );

	if ( !pTestWorldMesh->m_pMesh->paCollTree )
	{
		return FALSE;
	}

	CFVec3A vCenter, vMovement;
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

		// Get the bone info for the test mesh
		nBoneIdx = fmesh_coll_CalculateBoneMatrix( FColl_pTestMeshInst, nkDOPTree1Idx, &FColl_mtxTestBone, &FColl_mtxTestInvBone, &FColl_fBoneScale, &FColl_fInvBoneScale );

		// Record the current impact count so we can detect a change
		u16 nStartingImpactCount = FColl_nImpactCount;

		// Bring the sphere into bone space
		FColl_mtxTestInvBone.MulPoint( vCenter, pSphere->m_Pos );

		// Adjust for scale
		fRadius = pSphere->m_fRadius * FColl_fInvBoneScale;

		// Prime the node stack and start the test
		FMesh_Coll_nNodeStackIdx = 0;
		FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
		
		if ( FColl_fMoveMagnitude == 0.f )
		{
			_TestStaticSphereWithMesh( &vCenter, fRadius );
		}
		else
		{
			FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude * FColl_fInvBoneScale;
			FColl_mtxTestInvBone.MulDir( FColl_vTransMovement, *FColl_pCollData->pMovement );
			_TestDynamicSphereWithMesh( &vCenter, fRadius );
		}
			
		#if FMESH_COLL_ENABLE_COLLISION_DRAWS
			if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
			{
				CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
				CFVec3A vNewCenter;
				FColl_pTestToWorld->MulPoint( vNewCenter, vCenter );
				f32 fNewRadius = fRadius * FColl_fBoneScale;
				frenderer_Push( FRENDERER_DRAW, NULL );
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
	}
	
	return bFoundImpacts;
}


//
//
//
BOOL fcoll_TestWorldMeshAndObjectSpheres( const CFMeshInst *pWorldMeshInst, const CFMeshInst *pMeshInst, s32 nBone )
{
	FASSERT( pMeshInst && pWorldMeshInst );
	
	if ( !pWorldMeshInst->m_pMesh->paCollTree )
	{
		return FALSE;
	}
	
	CFVec3A vCenter;
	FMeshSeg_t *pSeg;
	FColl_pTestToWorld = NULL;
	FColl_pTestMeshInst = pWorldMeshInst;
	FColl_pTestWorldMesh = NULL;
	FMesh_t *pMesh1 = pWorldMeshInst->m_pMesh;
	FMesh_t *pMesh2 = pMeshInst->m_pMesh;

	u16 nStartingImpactCount = FColl_nImpactCount;

	if ( FColl_fMoveMagnitude == 0.f )
	{
		FColl_vTransMovement.Set( 0.f, 0.f, 0.f );
	}
	else
	{
		FColl_vTransMovement.Set( *FColl_pCollData->pMovement );
	}
	FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude;

	u16 nkDOPTree1Idx, nkDOPTree2Idx;
	for ( nkDOPTree1Idx = 0; nkDOPTree1Idx < pMesh1->nCollTreeCount; nkDOPTree1Idx++ )
	{
		FColl_pkDOPTree1 = &pMesh1->paCollTree[nkDOPTree1Idx];

		for ( nkDOPTree2Idx = 0; nkDOPTree2Idx < pMesh2->nCollTreeCount; nkDOPTree2Idx++ )
		{
			FColl_pkDOPTree2 = &pMesh2->paCollTree[nkDOPTree2Idx];

			if ( !(FColl_pkDOPTree2->nMasterCollMask & FColl_pCollData->nCollMask) )
			{
				continue;
			}
			
			pSeg = &pMesh2->aSeg[FColl_pkDOPTree2->nSegmentIdx];
			FASSERT( pSeg->nBoneMtxCount < 2 );
			FColl_nCurrentBoneIdx = pSeg->anBoneMtxIndex[0];
			
			if ( nBone != -1 && nBone != FColl_nCurrentBoneIdx )
			{
				continue;
			}

			vCenter.Set( pSeg->BoundSphere_MS.m_Pos );

			fmesh_coll_CalculateBoneMatrix( pMeshInst, nkDOPTree2Idx, &FColl_mtxBone );
			FColl_mtxBone.MulPoint( vCenter );

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
			
			if ( FColl_fMoveMagnitude == 0.f )
			{
				_TestStaticSphereWithMesh( &vCenter, pSeg->BoundSphere_MS.m_fRadius );
			}
			else
			{
				_TestDynamicSphereWithMesh( &vCenter, pSeg->BoundSphere_MS.m_fRadius );
			}
			
			#if FMESH_COLL_ENABLE_COLLISION_DRAWS
				if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
				{
					CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
					frenderer_Push( FRENDERER_DRAW, NULL );
					fdraw_FacetedWireSphere( &vCenter.v3, pSeg->BoundSphere_MS.m_fRadius, &Color );
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
BOOL fcoll_TestMeshAndObjectSpheres( const CFWorldMesh *pTestWorldMesh, const CFMeshInst *pMeshInst, s32 nBone )
{
	FASSERT( pMeshInst && pTestWorldMesh );

	if ( !pTestWorldMesh->m_pMesh->paCollTree )
	{
		return FALSE;
	}

	CFVec3A vCenter;
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

		FMesh_t *pMesh = FColl_pTestMeshInst->m_pMesh;

		FColl_mtxTestBone, FColl_mtxTestInvBone, FColl_mtxBone, FColl_mtxTemp;

		// Get the bone info for the test mesh
		nBoneIdx = fmesh_coll_CalculateBoneMatrix( FColl_pTestMeshInst, nkDOPTree1Idx, &FColl_mtxTestBone, &FColl_mtxTestInvBone, &FColl_fBoneScale, &FColl_fInvBoneScale );

		// Record the current impact count so we can detect a change
		u16 nStartingImpactCount = FColl_nImpactCount;

		for ( nkDOPTree2Idx = 0; nkDOPTree2Idx < pMesh2->nCollTreeCount; nkDOPTree2Idx++ )
		{
			FColl_pkDOPTree2 = &pMesh2->paCollTree[nkDOPTree2Idx];

			if ( !(FColl_pkDOPTree2->nMasterCollMask & FColl_pCollData->nCollMask) )
			{
				continue;
			}
			
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
			FColl_mtxTemp.MulPoint( vCenter, pSeg->BoundSphere_MS.m_Pos );

			// Adjust for scale
			fRadius = pSeg->BoundSphere_MS.m_fRadius * fScale * FColl_fInvBoneScale;

			// Prime the node stack and start the test
			FMesh_Coll_nNodeStackIdx = 0;
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = FColl_pkDOPTree1->pakDOPNodes;
			
			if ( FColl_fMoveMagnitude == 0.f )
			{
				_TestStaticSphereWithMesh( &vCenter, fRadius );
			}
			else
			{
				FColl_fScaledMoveMagnitude = FColl_fMoveMagnitude * FColl_fInvBoneScale;
				FColl_mtxTestInvBone.MulDir( FColl_vTransMovement, *FColl_pCollData->pMovement );
				_TestDynamicSphereWithMesh( &vCenter, fRadius );
			}
			
			#if FMESH_COLL_ENABLE_COLLISION_DRAWS
				if ( FMesh_Coll_bShowCollision && nStartingImpactCount != FColl_nImpactCount )
				{
					CFColorRGBA	Color( 1.f, 1.f, 1.f, 1.f );
					CFVec3A vNewCenter;
					FColl_pTestToWorld->MulPoint( vNewCenter, vCenter );
					f32 fNewRadius = fRadius * FColl_fBoneScale;
					frenderer_Push( FRENDERER_DRAW, NULL );
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
static FINLINE void _TransformWorldImpacts( u16 nStart )
{
	u32 i;
	for ( i = nStart; i < FColl_nImpactCount; i++ )
	{
		#if FMESH_COLL_ENABLE_COLLISION_DRAWS && FMESH_DRAW_COLLIDED_TRIS
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
		
		#if FMESH_COLL_ENABLE_COLLISION_DRAWS && FMESH_DRAW_COLLIDED_TRIS
			CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
			fmesh_Coll_DrawMeshTri( FColl_aImpactBuf[i].aTriVtx, &FColl_aImpactBuf[i].UnitFaceNormal, NULL, &Color, TRUE );
			fmesh_Coll_AddCollDrawImpact( &FColl_aImpactBuf[i].ImpactPoint.v3, 255, 255, 0 );
		#endif					
	}
}


//
//
//
static f32 _IntersectSphereAndTri( const CFVec3A *avTri, const CFVec3A *pTriNormal, CFVec3A *pContact, const CFVec3A *pCenter, f32 fRadiusSq, f32 fDist )
{
	// Test to see if collision point is inside tri
	FColl_vDiff1.Sub( avTri[1], avTri[0] );
	FColl_vDiff2.Sub( avTri[2], avTri[0] );
	
	FColl_vCross.Cross( FColl_vDiff2, *pTriNormal );
	f32 fDet = FColl_vDiff1.Dot( FColl_vCross );
	
	f32 fU = FColl_vODiff.Dot( FColl_vCross );
	if ( fU < 0.f )
	{
		// The center projects behind edge V2 - V0.  We should
		// check to see if the edge is within the radius
		f32 fPosition = fmath_Div( FColl_vODiff.Dot( FColl_vDiff2 ), FColl_vDiff2.MagSq() );
		if ( fPosition <= 0 )
		{
			pContact->Set( avTri[0] );
		}
		else if ( fPosition >= 1 )
		{
			pContact->Set( avTri[2] );
		}
		else
		{
			pContact->Add( avTri[0], FColl_vDiff2.Mul( fPosition ) );
		}
		FColl_pNextImpactSlot->PushUnitVec.Sub( *pCenter, *pContact );
		fDist = FColl_pNextImpactSlot->PushUnitVec.MagSq();
		if ( fDist > fRadiusSq )
		{
			return FMATH_MAX_FLOAT;
		}
		fDist = fmath_Sqrt( fDist );
		if ( fDist > 0.0001f )
		{
			FColl_pNextImpactSlot->PushUnitVec.Mul( fmath_Inv( fDist ) );
		}
	}
	else
	{
		FColl_vCross.Cross( FColl_vDiff1, FColl_vODiff );
		f32 fV = pTriNormal->Dot( FColl_vCross );
		if ( fV < 0.f )
		{
			// The center projects behind edge V1 - V0.  We should
			// check to see if the edge is within the radius
			f32 fPosition = fmath_Div( FColl_vODiff.Dot( FColl_vDiff1 ), FColl_vDiff1.MagSq() );
			if ( fPosition <= 0 )
			{
				pContact->Set( avTri[0] );
			}
			else if ( fPosition >= 1 )
			{
				pContact->Set( avTri[1] );
			}
			else
			{
				pContact->Add( avTri[0], FColl_vDiff1.Mul( fPosition) );
			}
			FColl_pNextImpactSlot->PushUnitVec.Sub( *pCenter, *pContact );
			fDist = FColl_pNextImpactSlot->PushUnitVec.MagSq();
			if ( fDist > fRadiusSq )
			{
				return FMATH_MAX_FLOAT;
			}
			fDist = fmath_Sqrt( fDist );
			if ( fDist > 0.0001f )
			{
				FColl_pNextImpactSlot->PushUnitVec.Mul( fmath_Inv( fDist ) );
			}
		}
		else if ( fU + fV > fDet )
		{
			// The center projects behind edge V1 - V2.  We should
			// check to see if the edge is within the radius
			FColl_vDiff2.Sub( avTri[2], avTri[1] );
			FColl_vODiff.Sub( *pCenter, avTri[1] );
			f32 fPosition = fmath_Div( FColl_vODiff.Dot( FColl_vDiff2 ), FColl_vDiff2.MagSq() );
			if ( fPosition <= 0 )
			{
				pContact->Set( avTri[1] );
			}
			else if ( fPosition >= 1 )
			{
				pContact->Set( avTri[2] );
			}
			else
			{
				pContact->Add( avTri[1], FColl_vDiff2.Mul( fPosition) );
			}
			FColl_pNextImpactSlot->PushUnitVec.Sub( *pCenter, *pContact );
			fDist = FColl_pNextImpactSlot->PushUnitVec.MagSq();
			if ( fDist > fRadiusSq )
			{
				return FMATH_MAX_FLOAT;
			}
			fDist = fmath_Sqrt( fDist );
			if ( fDist > 0.0001f )
			{
				FColl_pNextImpactSlot->PushUnitVec.Mul( fmath_Inv( fDist ) );
			}
		}
		else
		{
			pContact->Mul( *pTriNormal, fDist );
			pContact->Negate();
			pContact->Add( *pCenter );
			FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
		}
	}

	return fDist;
}


//
//
//
static BOOL _IntersectStaticSphereAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pCenter, f32 fRadius )
{
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
		
	f32 fRadiusSq = fRadius * fRadius;

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
				pVB->GetPoint( pNodeVertIdx[nNodeVert++], avTri[0].v3 );
			#elif FANG_PLATFORM_DX
				pTriNormal->Set( *pPacketNormals );
				avTri[0].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				pTriNormal->Set( *pPacketNormals );
				avTri[0].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			FColl_vODiff.Sub( *pCenter, avTri[0] );
			f32 fDist = FColl_vODiff.Dot( *pTriNormal );
			if ( (FColl_pCollData->nFlags & FCOLL_DATA_IGNORE_BACKSIDE) && fDist < 0.f )
			{
				continue;
			}

			if ( fDist < -fRadius || fDist > fRadius )
			{
				continue;
			}

			// We'll need the rest of the verts for the remaining tests
			#if FANG_PLATFORM_GC
				pVB->GetPoint( pNodeVertIdx[nNodeVert++], avTri[1].v3 );
				pVB->GetPoint( pNodeVertIdx[nNodeVert],   avTri[2].v3 );
			#elif FANG_PLATFORM_DX
				avTri[1].v3 = pVerts[ pNodeVertIdx[nNodeVert++] ];
				avTri[2].v3 = pVerts[ pNodeVertIdx[nNodeVert] ];
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				avTri[1].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
				avTri[2].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert] ]);
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			fDist = _IntersectSphereAndTri( avTri, pTriNormal, pContact, pCenter, fRadiusSq, fDist );
			if ( fDist == FMATH_MAX_FLOAT )
			{
				continue;
			}

			FColl_pNextImpactSlot->fUnitImpactTime = -1.f;
			FColl_pNextImpactSlot->fImpactDistInfo = fRadius - fDist;
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


// Given an edge of a polygon and a moving sphere, find the first contact the sphere 
//	makes with the edge, if any.  Note that fT must be primed with a value of 1
//	before calling this function the first time.  It will then maintain the closest 
//	collision in subsequent calls.
//
//	vSphereCenter:	start point (center) of sphere
//	vTravel:		path of sphere during frame (includes magnitude)
//	fRadius:		radius of sphere
//	vVert0:			vert 0 of the edge
//	vVert1:			vert 1 of the edge
//	fT:				Filled in with unit time at which sphere collides with polygon edge
//	vImpact:		Filled in with the  point on edge that is hit
//
static void _ProjectSphereAgainstEdge( const CFVec3A &vSphereCenter, f32 fRadius, const CFVec3A &vTravel, 
							  const CFVec3A &vVert0, const CFVec3A &vVert1, f32 &fT, CFVec3A &vImpact )
{
	static CFVec3A __vTempHit;
	static CFVec3A __vCenterDelta;
	static CFVec3A __vEdge;

	__vEdge.Sub( vVert1, vVert0 );
	__vCenterDelta.Sub( vSphereCenter, vVert0 );
	f32 fDCenterDotEdge = __vCenterDelta.Dot( __vEdge );
	f32 fDCenterDotTravel = __vCenterDelta.Dot( vTravel );
	f32 fDCenterMagSq = __vCenterDelta.MagSq();
	f32 fEdgeDotTravel = __vEdge.Dot( vTravel );
	f32 fEdgeMagSq = __vEdge.MagSq();
	f32 fTravelMagSq = vTravel.MagSq();

	f32 fTemp;
	f32 fA, fB, fC, fRoot, fDiscriminant;
	f32 fRoot1 = 0.f;
	f32 fRoot2 = 0.f;

	fA = fEdgeDotTravel * fEdgeDotTravel - fEdgeMagSq * fTravelMagSq;
	fB = 2.f * (fDCenterDotEdge * fEdgeDotTravel - fDCenterDotTravel * fEdgeMagSq);
	fC = fDCenterDotEdge * fDCenterDotEdge + fRadius * fRadius * fEdgeMagSq - fDCenterMagSq * fEdgeMagSq;

	if ( fmath_Abs( fA ) > 0.0001f ) 
	{
		// Sphere is not travelling parallel to the edge (if it is, we test the verts only)
		fDiscriminant = fB * fB - 4.f * fA * fC;
		if ( fDiscriminant > 0.f ) 
		{
			fRoot = fmath_Sqrt( fDiscriminant );
			fRoot1 = fmath_Div( -fB + fRoot, 2.f * fA );
			fRoot2 = fmath_Div( -fB - fRoot, 2.f * fA );

			// Sort fRoot1 and fRoot2, use the earliest intersection.  The larger root 
			// corresponds to the final contact of the sphere with the edge on its 
			// way out.
			if ( fRoot2 < fRoot1 ) 
			{
				fTemp = fRoot1;
				fRoot1 = fRoot2;
				fRoot2 = fTemp;
			}

			// fRoot1 should be a unit time, check that it's in our currently valid range
			if ( fRoot1 >= 0 ) 
			{
				if ( fRoot1 >= fT )
				{
					// Sphere does not reach the edge
					return;
				}

				// Find sphere and edge positions
				__vTempHit.Mul( vTravel, fRoot1 ).Add( vSphereCenter );

				// Check if hit is between vVert0 and vVert1
				f32 fRatio = fmath_Div( __vTempHit.Sub( vVert0 ).Dot( __vEdge ), fEdgeMagSq );
				if ( (fRatio >= 0.f) && (fRatio <= 1.f) ) 
				{
					// Setup the valid hit
					fT = fRoot1;
					vImpact.Mul( __vEdge, fRatio ).Add( vVert0 );
					return;
				}
			}
		} 
		else 
		{
			// If fDiscriminant is negative, sphere passed edge too far away
			return;
		}
	}

	// Sphere missed the edge, check for a collision with the first vertex.  note
	// that we only need to check one vertex per call to check all vertices.
	fA = fTravelMagSq;
	fB = 2.f * fDCenterDotTravel;
	fC = fDCenterMagSq - fRadius * fRadius;

	fDiscriminant = fB * fB - 4.f * fA * fC;
	if ( fDiscriminant > 0.f ) 
	{
		fRoot = fmath_Sqrt( fDiscriminant );
		fRoot1 = fmath_Div( -fB + fRoot, 2.f * fA );
		fRoot2 = fmath_Div( -fB - fRoot, 2.f * fA );

		// Sort the solutions
		if ( fRoot1 > fRoot2 ) 
		{
			fTemp = fRoot1;
			fRoot1 = fRoot2;
			fRoot2 = fTemp;
		}

		// Check hit vertex for validity and ensure earlier than what we already have
		if ( (fRoot1 < 0.f) || (fRoot1 >= fT) ) 
		{
			return;
		}

	    // Sphere collided with vertex
		fT = fRoot1;
		vImpact = vVert0;
		return;
	} 
	
	// fDiscriminant is negative so sphere misses vertex, too
}


//
//
//
static BOOL _IntersectDynamicSphereAndTriangles( const FkDOP_Node_t *pTestNode, const CFVec3A *pCenter, f32 fRadius )
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
		
	f32 fRadiusSq = fRadius * fRadius;

	// Uncompress the verts & normals
	BOOL bInitiallyInContact;
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

			// Get the normal 
			#if FANG_PLATFORM_GC
				pPacketNormals->DecompressTo( *pTriNormal );
			#elif FANG_PLATFORM_DX
				pTriNormal->Set( *pPacketNormals );
//ARG - >>>>>
			#elif FANG_PLATFORM_PS2
				pTriNormal->Set( *pPacketNormals );
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			f32 fFaceDot = FColl_vTransMovement.Dot( *pTriNormal );

			// Get the verts in preparation for checks
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
				pTriNormal->Set( *pPacketNormals );
				avTri[0].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
				avTri[1].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert++] ]);
				avTri[2].v3 = *((CFVec3 *)pVerts[ pNodeVertIdx[nNodeVert] ]);
//ARG - <<<<<
			#else
				FASSERT_NOW;
			#endif

			f32 fDist, fDist1, fDist2;

			FColl_vODiff.Sub( *pCenter, avTri[0] );
			fDist2 = FColl_vODiff.Dot( *pTriNormal );

			if (   fmath_Abs(avTri[0].x - -107.0f) < 1.f
				&& fmath_Abs(avTri[1].x - -85.4f) < 1.f
				&& fmath_Abs(avTri[2].x - -87.7f) < 1.f )
			{
				fDist2 = fDist2;
			}

			// If we're supposed to be ignoring backsides, do so
			if ( (FColl_pCollData->nFlags & FCOLL_DATA_IGNORE_BACKSIDE) && fDist2 < 0.f )
			{
				continue;
			}

			bInitiallyInContact = FALSE;

			// First does the sphere start in a collided state?
			if ( fmath_Abs( fDist2 ) < fRadius )
			{
				fDist2 = _IntersectSphereAndTri( avTri, pTriNormal, pContact, pCenter, fRadiusSq, fDist2 );
				if ( fDist2 != FMATH_MAX_FLOAT )
				{
					// Initial sphere is in a collision state
					bInitiallyInContact = TRUE;
					FColl_pNextImpactSlot->fUnitImpactTime = -1.f;
					FColl_pNextImpactSlot->fImpactDistInfo = fRadius - fDist2;
					// PushUnitVec is already established in _IntersectSphereAndTri()
//					FColl_pNextImpactSlot->PushUnitVec.Sub( *pCenter, *pContact ).Unitize();
				}
			}

			if ( !bInitiallyInContact )
			{
				if ( fmath_Abs( fFaceDot ) < 0.0001f )
				{
					// If the dot is zero, then the face is parallel with the ray
					// We still need to test to see if the radius intersects
					if ( fDist2 > fRadius || fDist2 < -fRadius )
					{
						// Plane is beyond radius
						continue;
					}

					fDist = -1.f;
				}
				else
				{
					// Create a vector from the point where collision would occur on the sphere projected through the height
					if ( fFaceDot > 0.f )
					{
						vTemp1.Mul( *pTriNormal, fRadius ).Add( *pCenter );
					}
					else
					{
						vTemp1.Mul( *pTriNormal, -fRadius ).Add( *pCenter );
					}					
					
					vTemp2.Add( vTemp1, FColl_vTransMovement );
					
					// Determine if this line intersects the plane
					FColl_vDiff1.Sub( vTemp1, avTri[0] );
					fDist = FColl_vDiff1.Dot( *pTriNormal );
					FColl_vDiff2.Sub( vTemp2, avTri[0] );
					fDist1 = FColl_vDiff2.Dot( *pTriNormal );
 
					if ( (fDist > 0.f) == (fDist1 > 0.f) )
					{
						if ( fmath_Abs( fDist ) > fRadius && fmath_Abs( fDist1 ) > fRadius )
						{
							// Projected sphere will never reach the tri plane
							continue;
						}
						fDist = -1.f;
					}
					else
					{
						// The line intersects the plane, so determine the distance to the 
						// intersection and the impact point
						FColl_pNextImpactSlot->fUnitImpactTime = fmath_Div(fDist, fDist - fDist1);
						fDist = FColl_fScaledMoveMagnitude * FColl_pNextImpactSlot->fUnitImpactTime;
						pContact->Sub( vTemp2, vTemp1 ).Mul( FColl_pNextImpactSlot->fUnitImpactTime ).Add( vTemp1 );
						FASSERT( fDist >= 0.f );

						// Determine if the impact point actually is inside the triangle					
						FColl_vDiff1.Sub( avTri[1], avTri[0] );
						FColl_vDiff2.Sub( avTri[2], avTri[0] );
						FColl_vODiff.Sub( *pContact, avTri[0] );
						if ( !fcoll_PointIsInTri( FColl_vODiff, FColl_vDiff1, FColl_vDiff2, *pTriNormal ) )
						{
							// This point is not in the tri, so we need to test the edges
							fDist = -1.f;
						}
						else
						{
							// Sphere collides with the face, not an edge, so we can easily calculate this info:
							FColl_pNextImpactSlot->PushUnitVec.Set( *pTriNormal );
							FColl_pNextImpactSlot->fImpactDistInfo = fmath_Abs( fDist1 );
							FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.0f && FColl_pNextImpactSlot->fImpactDistInfo <= (FColl_fScaledMoveMagnitude * 1.02f) );
							fDist = 0.f;
						}
					}
				}

				if ( fDist == -1.f )
				{
					FColl_pNextImpactSlot->fUnitImpactTime = 1.f;
					_ProjectSphereAgainstEdge( *pCenter, fRadius, FColl_vTransMovement, avTri[0], avTri[1], FColl_pNextImpactSlot->fUnitImpactTime, *pContact );
					_ProjectSphereAgainstEdge( *pCenter, fRadius, FColl_vTransMovement, avTri[1], avTri[2], FColl_pNextImpactSlot->fUnitImpactTime, *pContact );
					_ProjectSphereAgainstEdge( *pCenter, fRadius, FColl_vTransMovement, avTri[2], avTri[0], FColl_pNextImpactSlot->fUnitImpactTime, *pContact );
					
					if ( FColl_pNextImpactSlot->fUnitImpactTime == 1.f )
					{
						// Projected sphere did not impact the edges
						continue;
					}
					else
					{
						if ( (FColl_pCollData->nFlags & FCOLL_DATA_IGNORE_BACKSIDE) && fFaceDot > 0.f )
						{
							continue;
						}

						// Projected sphere impacted one of the edges (or verts)
						FColl_pNextImpactSlot->PushUnitVec.Sub( *pCenter, *pContact ).Unitize();
						fFaceDot = -FColl_vTransMovement.Dot( FColl_pNextImpactSlot->PushUnitVec );
						FColl_pNextImpactSlot->fImpactDistInfo = fFaceDot * (1.f - FColl_pNextImpactSlot->fUnitImpactTime);
						FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.0f && FColl_pNextImpactSlot->fImpactDistInfo <= (FColl_fScaledMoveMagnitude * 1.02f) );
					}
				}
			}

			// WE'VE GOT COLLISION!!!

			FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
			FColl_pNextImpactSlot->pTag = (void *)FColl_pTestWorldMesh;
			FColl_pNextImpactSlot->nSourceBoneIndex = FColl_nCurrentBoneIdx;
			
			FASSERT( FColl_pNextImpactSlot->UnitFaceNormal.MagSq() <= 1.02f && FColl_pNextImpactSlot->UnitFaceNormal.MagSq() >= -1.02f );
			FASSERT( FColl_pNextImpactSlot->fUnitImpactTime == -1.f || (FColl_pNextImpactSlot->fUnitImpactTime >= 0.f && FColl_pNextImpactSlot->fUnitImpactTime < 1.02f) );

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
//	Function used to test static sphere against kDOP trees
//
static void _TestStaticSphereWithMesh( const CFVec3A *pCenter, f32 fRadius )
{
	FASSERT( FColl_pTestMeshInst && FColl_pkDOPTree1 && FColl_pCollData );
	FASSERT( FMesh_Coll_nNodeStackIdx );
	
	// Pre calculate the center intervals
	f32 afCenterIntervals[FKDOP_MAX_AXES];
	u16 i, nNorms = FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].nNormalCount;
	for ( i = 0; i < nNorms; i++ )
	{
		afCenterIntervals[i] = pCenter->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
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
		
		// Make sure this sphere overlaps the intervals in all directions, taking into account movement
		for ( i = 0; i < nNorms; i++ )
		{
			if ( afCenterIntervals[i] + fRadius < pWorldInt[i].fMin || afCenterIntervals[i] - fRadius > pWorldInt[i].fMax )
			{
				// Sphere does not collide with kDOP, so we can stop traversal down this branch
				break;
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

		// This is a leaf, so we should check the world mesh tris against the sphere
		if ( !_IntersectStaticSphereAndTriangles( pTestNode, pCenter, fRadius ) )
		{
			// Function indicated we should cease our search
			return;
		}
	}
}


//
//	Function used to test sphere against kDOP trees
//
static void _TestDynamicSphereWithMesh( const CFVec3A *pCenter, f32 fRadius )
{
	FASSERT( FColl_pTestMeshInst && FColl_pkDOPTree1 && FColl_pCollData );
	FASSERT( FMesh_Coll_nNodeStackIdx );

	// Pre calculate the center intervals
	f32 afCenterIntervals[FKDOP_MAX_AXES];
	f32 afMoveAlongAxis[FKDOP_MAX_AXES];
	u16 i, nNorms = FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].nNormalCount;
	for ( i = 0; i < nNorms; i++ )
	{
		afCenterIntervals[i] = pCenter->Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
		afMoveAlongAxis[i] = FColl_vTransMovement.Dot( FkDOP_aDesc[FColl_pkDOPTree1->nTreekDOPType].avNormals[i] );
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
		
		// Make sure this sphere overlaps the intervals in all directions, taking into account movement
		for ( i = 0; i < nNorms; i++ )
		{
			if ( afMoveAlongAxis[i] < 0.f )
			{
				if ( afCenterIntervals[i] + fRadius < pWorldInt[i].fMin || afCenterIntervals[i] - fRadius + afMoveAlongAxis[i] > pWorldInt[i].fMax )
				{
					// Sphere does not collide with kDOP, so we can stop traversal down this branch
					break;
				}
			}
			else
			{
				if ( afCenterIntervals[i] + fRadius + afMoveAlongAxis[i] < pWorldInt[i].fMin || afCenterIntervals[i] - fRadius > pWorldInt[i].fMax )
				{
					// Sphere does not collide with kDOP, so we can stop traversal down this branch
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
//			fmesh_Coll_AddCollDrawDOP( pWorldInt, NULL, FkDOP_14_DOP );
		
			// The world node is not a leaf, so push its children onto the stack
			FASSERT( pTestNode->nStartChildIdx + 1 < FColl_pkDOPTree1->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FColl_pkDOPTree1->pakDOPNodes[pTestNode->nStartChildIdx + 1];
			continue;
		}

		// This is a leaf, so we should check the world mesh tris against the sphere
		if ( !_IntersectDynamicSphereAndTriangles( pTestNode, pCenter, fRadius ) )
		{
			// Function indicated we should cease our search
			return;
		}
	}
}


