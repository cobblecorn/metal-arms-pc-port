////////////////////////////////////////////////////////////////////////////////////////////
//
//	COLLISIONTEST.CPP
//
//		Collision testing routines for FANG test app
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdraw.h"
#include "fcoll.h"
#include "fworld_coll.h"
#include "fmesh_coll.h"
#include "frenderer.h"

#include "CollisionTest.h"

static CFCollInfo CollInfo;
static CFWorldMesh *_pLastColl;

//
//
//
static BOOL _TrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection )
{
	_pLastColl = (CFWorldMesh *)pTracker;
	((CFWorldMesh *)pTracker)->CollideWithMeshTris( &CollInfo );
	return TRUE;
}


//
//
//
static BOOL _TrackersCallback2( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode, u32 nSphereIndex )
{
	_pLastColl = (CFWorldMesh *)pTracker;
	((CFWorldMesh *)pTracker)->CollideWithMeshTris( &CollInfo );
	return TRUE;
}

//
//
//
static BOOL _TrackersCallback3( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode )
{
	_pLastColl = (CFWorldMesh *)pTracker;
	((CFWorldMesh *)pTracker)->CollideWithMeshTris( &CollInfo );
	return TRUE;
}


extern CFVec3A _vInfPoint;
extern s32 _nNormalIdx;
extern s32 _nBoneIdx;
extern BOOL8 FMesh_Coll_bShowCollision;

//
//
//
void TestNewCollision( CFXfm *pCamXfm )
{
	FMesh_Coll_bShowCollision = TRUE;
	
	CFSphere CollSphere;
	CollSphere.m_Pos = pCamXfm->m_MtxR.m_vPos.v3 + (pCamXfm->m_MtxR.m_vFront.v3 * 7);
	CollSphere.m_fRadius = 2.f;
	fworld_FindTrackersIntersectingSphere( &CollSphere, FWORLD_TRACKERTYPE_MESH, _TrackersCallback3 );
	frenderer_Push( FRENDERER_DRAW, NULL );
	fdraw_FacetedWireSphere( &CollSphere.m_Pos, CollSphere.m_fRadius );
	frenderer_Pop();	
}

/*
fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_MESH, _FindTrackersCallback_Blast, NULL, 0x7fffffffffffffff );
*/

//
//
//
void TestCollision( CFXfm *pCamXfm )
{
	u32 i;
	CFSphere CollSphere;

	FMesh_Coll_bShowCollision = TRUE;
	
	FMesh_Coll_nTests = 0;
	FMesh_Coll_nTestsLevel1 = 0;
	FMesh_Coll_nTestsLevel2 = 0;
	FMesh_Coll_nHits = 0;

	CFTrackerCollideInfo *pTrackerCollInfo;
	CFTrackerCollideSpheresInfo CollSphereInfo;
	CFTrackerCollideProjSphereInfo CollProjSphereInfo;
	CFTrackerCollideRayInfo CollRayInfo;
	CFTrackerCollideCylinderInfo CollCylinderInfo;
	
	
//	CollInfo.nCollTestType = FMESH_COLLTESTTYPE_SPHERE;
//	CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	CollInfo.nCollTestType = FMESH_COLLTESTTYPE_RAY;
//	CollInfo.nCollTestType = FMESH_COLLTESTTYPE_CYLINDER;
//	CollInfo.nCollTestType = FMESH_COLLTESTTYPE_CAPSULE;
	
	CollInfo.nStopOnFirstOfCollMask = FALSE;
	CollInfo.bCullBacksideCollisions = TRUE;
	CollInfo.nCollMask = FCOLL_MASK_NONE;
	CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	CollInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	
	if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_SPHERE )
	{
		// Sphere specific collisions:
		CollSphere.m_Pos = pCamXfm->m_MtxR.m_vPos.v3 + (pCamXfm->m_MtxR.m_vFront.v3 * 7);
		CollSphere.m_fRadius = 1.f;
		CollInfo.pCurMasterSphere_WS = &CollSphere;
		CollInfo.nSphereListCount = 0;
		
		pTrackerCollInfo = &CollSphereInfo;
		CollSphereInfo.pMasterSphere_WS = &CollSphere;
		CollSphereInfo.pSphereList_WS = NULL;
		CollSphereInfo.bIgnoreCollisionFlag = TRUE;
		CollSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
		CollSphereInfo.nSphereListCount = 0;
		CollSphereInfo.pCallback = _TrackersCallback2;
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_RAY )
	{
		CFVec3A vRayStart_CS( 0.f, 2.f, 7.f );
		CFVec3A vRayEnd_CS( 0.f, -2.f, 7.f );
	
		// Ray specific collisions
		CollInfo.Ray.Init( &pCamXfm->m_MtxR.MulPoint( vRayStart_CS ), &pCamXfm->m_MtxR.MulPoint( vRayEnd_CS ) );
	
		pTrackerCollInfo = &CollRayInfo;
		CollRayInfo.StartPoint_WS = CollInfo.Ray.vStart_WS;
		CollRayInfo.EndPoint_WS = CollInfo.Ray.vEnd_WS;
		CollRayInfo.bComputeIntersection = TRUE;
		CollRayInfo.bIgnoreCollisionFlag = FALSE;
		CollRayInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
		CollRayInfo.pCallback = _TrackersCallback;
		CollRayInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_CYLINDER )
	{
		CFVec3A vRayStart_CS( 0.f, 0.8f, 7.f );
		CFVec3A vRayEnd_CS( 0.f, -0.8f, 7.f );
	
		// Cylinder specific collisions
		CollInfo.Cylinder.Init( &pCamXfm->m_MtxR.MulPoint( vRayStart_CS ), &pCamXfm->m_MtxR.MulPoint( vRayEnd_CS ), 0.9f );
		
		pTrackerCollInfo = &CollCylinderInfo;
		CollCylinderInfo.pCylinder = &CollInfo.Cylinder;
		CollCylinderInfo.bIgnoreCollisionFlag = FALSE;
		CollCylinderInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
		CollCylinderInfo.pCallback = _TrackersCallback3;
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_CAPSULE )
	{
		CFVec3A vRayStart_CS( 2.01f, 3.f, 7.f );
		CFVec3A vRayEnd_CS( -2.01f, -3.f, 7.f );
	
		// Cylinder specific collisions
		CollInfo.Cylinder.Init( &pCamXfm->m_MtxR.MulPoint( vRayStart_CS ), &pCamXfm->m_MtxR.MulPoint( vRayEnd_CS ), 1.f );
		
		pTrackerCollInfo = &CollCylinderInfo;
		CollCylinderInfo.pCylinder = &CollInfo.Cylinder;
		CollCylinderInfo.bIgnoreCollisionFlag = FALSE;
		CollCylinderInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
		CollCylinderInfo.pCallback = _TrackersCallback3;
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_PROJSPHERE )
	{
		CFVec3A vRayStart_CS( .01f, 0.f, 7.f );
		CFVec3A vRayEnd_CS( .01f, -3.f, 7.f );

		// Cylinder specific collisions
		CollInfo.ProjSphere.Init( &pCamXfm->m_MtxR.MulPoint( vRayStart_CS ), &pCamXfm->m_MtxR.MulPoint( vRayEnd_CS ), 1.f );
		CollInfo.bFindClosestImpactOnly = TRUE;
		CollInfo.bCalculateImpactData = FALSE;
		CollInfo.bCullBacksideCollisions = TRUE;

		pTrackerCollInfo = &CollProjSphereInfo;
		CollProjSphereInfo.pProjSphere = &CollInfo.ProjSphere;
		CollProjSphereInfo.bIgnoreCollisionFlag = FALSE;
		CollProjSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
		CollProjSphereInfo.pCallback = _TrackersCallback3;
	}
	
	fcoll_Clear();
	fworld_CollideWithWorldTris( &CollInfo );
	fworld_CollideWithTrackers( pTrackerCollInfo );

	CFVec3A vEnd;
	int iLow = -1;
	f32 fLowest = FMATH_MAX_FLOAT;
	for ( i = 0; i < FColl_nImpactCount; i++ )
	{
		if ( fLowest > FColl_aImpactBuf[i].fImpactDistInfo )
		{
			iLow = i;
			fLowest = FColl_aImpactBuf[i].fImpactDistInfo;
		}
	}
	
	frenderer_Push( FRENDERER_DRAW, NULL );
//	CFVec3 vTest( -88.f, 13.f, 131.f );
//	fdraw_FacetedWireSphere( &vTest, 10.f );
	
	if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_SPHERE )
	{
		fdraw_FacetedWireSphere( &CollInfo.pCurMasterSphere_WS->m_Pos, CollInfo.pCurMasterSphere_WS->m_fRadius );
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_RAY )
	{
		CFColorRGBA Color;
		Color.Set( 1.f, 1.f, 0.f, 1.f );
		if ( iLow != -1 )
		{
			fdraw_SolidLine( &CollRayInfo.StartPoint_WS.v3, &FColl_aImpactBuf[iLow].ImpactPoint.v3, &Color );
			fdraw_FacetedWireSphere( &FColl_aImpactBuf[iLow].ImpactPoint.v3, 1.f, &Color );
		}
		else
		{
			fdraw_SolidLine( &CollRayInfo.StartPoint_WS.v3, &CollRayInfo.EndPoint_WS.v3 );
		}
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_CAPSULE )
	{
		vEnd.Mul( CollInfo.Cylinder.vNormal_WS, fLowest ).Add( CollInfo.Cylinder.vStart_WS );
		fdraw_FacetedCylinder( &CollInfo.Cylinder.vStart_WS.v3, &CollInfo.Cylinder.vNormal_WS.v3, CollInfo.Cylinder.fRadius, fLowest );
		fdraw_FacetedWireSphere( &CollInfo.Cylinder.vStart_WS.v3, CollInfo.Cylinder.fRadius );
		fdraw_FacetedWireSphere( &vEnd.v3, CollInfo.Cylinder.fRadius );
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_PROJSPHERE )
	{
		if ( iLow == -1 )
		{
			fLowest = CollInfo.ProjSphere.m_fMoveDistance;
		}
		vEnd.Mul( CollInfo.ProjSphere.m_vMoveNormal_WS, fLowest ).Add( CollInfo.ProjSphere.m_vCenterStart_WS );
		fdraw_FacetedCylinder( &CollInfo.ProjSphere.m_vCenterStart_WS.v3, &CollInfo.ProjSphere.m_vMoveNormal_WS.v3, CollInfo.ProjSphere.m_fRadius, fLowest );
		fdraw_FacetedWireSphere( &CollInfo.ProjSphere.m_vCenterStart_WS.v3, CollInfo.ProjSphere.m_fRadius );
		fdraw_FacetedWireSphere( &vEnd.v3, CollInfo.ProjSphere.m_fRadius );
	}
	else if ( CollInfo.nCollTestType == FMESH_COLLTESTTYPE_CYLINDER )
	{
		fdraw_FacetedCylinder( &CollInfo.Cylinder.vStart_WS.v3, &CollInfo.Cylinder.vNormal_WS.v3, CollInfo.Cylinder.fRadius, CollInfo.Cylinder.fHeight );
	}
	frenderer_Pop();
}


