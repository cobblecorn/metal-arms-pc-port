//////////////////////////////////////////////////////////////////////////////////////
// fdx8mesh_coll.cpp - Fang mesh collision module (DX version).
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
// 07/11/02	Lafleur		Extracted from fGCMesh
// 08/15/02	Lafleur		Adapted from fgcmesh_coll.cpp
//////////////////////////////////////////////////////////////////////////////////////


#include "fmesh_coll.h"
#include "fdx8mesh.h"



#define _USE_HUNLEY_METHOD	TRUE

#define _TRACK_TESTS		FALSE


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

static CFVec3A _avUncVerts[3];
static CFVec3A _vNormal, _vODiff, _vDiff1, _vDiff2, _vCross, _vImpact, _vPush;
static f32 _afTemp0Intervals[FKDOP_MAX_AXES];	
static f32 _afTemp1Intervals[FKDOP_MAX_AXES];	



//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _IntersectSphereAndTri( CFVec3A *pCenter, f32 fRadius, f32 fRadiusSq, BOOL bMasterSphere );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
// Given a point on a plane and a triangle on the same plane, this
//    code calculated the closest point (in barycentric coords) on the 
//    to the triangle to the point specified.
//
FINLINE f32 GetUVofClosestPoint( f32 &fS, f32 &fT, CFVec3A &vPoint, CFVec3A &vTriPoint0, CFVec3A &vEdge0, CFVec3A &vEdge1 )
{
	static CFVec3A kDiff;
	kDiff.Sub( vTriPoint0, vPoint );
	f32 fA00 = vEdge0.MagSq();
	f32 fA01 = vEdge0.Dot(vEdge1);
	f32 fA11 = vEdge1.MagSq();
	f32 fB0 = kDiff.Dot(vEdge0);
	f32 fB1 = kDiff.Dot(vEdge1);
	f32 fC = kDiff.MagSq();
	f32 fDet = fmath_Abs( fA00 * fA11 - fA01 * fA01 );
	fS = fA01 * fB1 - fA11 * fB0;
	fT = fA01 * fB0 - fA00 * fB1;
	f32 fSqrDist;

	if ( fS + fT <= fDet )
	{
		if ( fS < 0.0f )
		{
			if ( fT < 0.0f )  // region 4
			{
				if ( fB0 < 0.0f )
				{
					fT = 0.0f;
					if ( -fB0 >= fA00 )
					{
						fS = 1.0f;
						fSqrDist = fA00 + 2.0f * fB0 + fC;
					}
					else
					{
						fS = fmath_Div( -fB0, fA00 );
						fSqrDist = fB0 * fS + fC;
					}
				}
				else
				{
					fS = 0.0f;
					if ( fB1 >= 0.0f )
					{
						fT = 0.0f;
						fSqrDist = fC;
					}
					else if ( -fB1 >= fA11 )
					{
						fT = 1.0f;
						fSqrDist = fA11 + 2.0f * fB1 + fC;
					}
					else
					{
						fT = fmath_Div( -fB1, fA11 );
						fSqrDist = fB1 * fT + fC;
					}
				}
			}
			else  // region 3
			{
				fS = 0.0f;
				if ( fB1 >= 0.0f )
				{
					fT = 0.0f;
					fSqrDist = fC;
				}
				else if ( -fB1 >= fA11 )
				{
					fT = 1.0f;
					fSqrDist = fA11 + 2.0f * fB1 + fC;
				}
				else
				{
					fT = fmath_Div( -fB1, fA11 );
					fSqrDist = fB1 * fT + fC;
				}
			}
		}
		else if ( fT < 0.0f )  // region 5
		{
			fT = 0.0f;
			if ( fB0 >= 0.0f )
			{
				fS = 0.0f;
				fSqrDist = fC;
			}
			else if ( -fB0 >= fA00 )
			{
				fS = 1.0f;
				fSqrDist = fA00 + 2.0f * fB0 + fC;
			}
			else
			{
				fS = fmath_Div( -fB0, fA00 );
				fSqrDist = fB0 * fS + fC;
			}
		}
		else  // region 0
		{
			fSqrDist = 0;
		}
	}
	else
	{
		f32 fTmp0, fTmp1, fNumer, fDenom;

		if ( fS < 0.0f )  // region 2
		{
			fTmp0 = fA01 + fB0;
			fTmp1 = fA11 + fB1;
			if ( fTmp1 > fTmp0 )
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00 - 2.0f * fA01 + fA11;
				if ( fNumer >= fDenom )
				{
					fS = 1.0f;
					fT = 0.0f;
					fSqrDist = fA00 + 2.0f * fB0 + fC;
				}
				else
				{
					fS = fmath_Div( fNumer, fDenom );
					fT = 1.0f - fS;
					fSqrDist = fS * (fA00 * fS + fA01 * fT + 2.0f * fB0) + fT * (fA01 * fS + fA11 * fT + 2.0f * fB1) + fC;
				}
			}
			else
			{
				fS = 0.0f;
				if ( fTmp1 <= 0.0f )
				{
					fT = 1.0f;
					fSqrDist = fA11 + 2.0f * fB1 + fC;
				}
				else if ( fB1 >= 0.0f )
				{
					fT = 0.0f;
					fSqrDist = fC;
				}
				else
				{
					fT = fmath_Div( -fB1, fA11 );
					fSqrDist = fB1 * fT + fC;
				}
			}
		}
		else if ( fT < 0.0f )  // region 6
		{
			fTmp0 = fA01 + fB1;
			fTmp1 = fA00 + fB0;
			if ( fTmp1 > fTmp0 )
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00 - 2.0f * fA01 + fA11;
				if ( fNumer >= fDenom )
				{
					fT = 1.0f;
					fS = 0.0f;
					fSqrDist = fA11+2.0f*fB1+fC;
				}
				else
				{
					fT = fmath_Div( fNumer, fDenom );
					fS = 1.0f - fT;
					fSqrDist = fS * (fA00 * fS + fA01 * fT + 2.0f * fB0) + fT * (fA01 * fS + fA11 * fT + 2.0f * fB1) + fC;
				}
			}
			else
			{
				fT = 0.0f;
				if ( fTmp1 <= 0.0f )
				{
					fS = 1.0f;
					fSqrDist = fA00+2.0f*fB0+fC;
				}
				else if ( fB0 >= 0.0f )
				{
					fS = 0.0f;
					fSqrDist = fC;
				}
				else
				{
					fS = fmath_Div( -fB0, fA00 );
					fSqrDist = fB0 * fS + fC;
				}
			}
		}
		else  // region 1
		{
			fNumer = fA11 + fB1 - fA01 - fB0;
			if ( fNumer <= 0.0f )
			{
				fS = 0.0f;
				fT = 1.0f;
				fSqrDist = fA11 + 2.0f * fB1 + fC;
			}
			else
			{
				fDenom = fA00-2.0f*fA01+fA11;
				if ( fNumer >= fDenom )
				{
					fS = 1.0f;
					fT = 0.0f;
					fSqrDist = fA00 + 2.0f * fB0 + fC;
				}
				else
				{
					fS = fmath_Div( fNumer, fDenom );
					fT = 1.0f - fS;
					fSqrDist = fS * (fA00 * fS + fA01 * fT + 2.0f * fB0) + fT * (fA01 * fS + fA11 * fT + 2.0f * fB1) + fC;
				}
			}
		}
	}
    
	return fmath_Abs( fSqrDist );
}


//
//
//
void fmesh_Coll_TestSphereWithDOPs( const FMesh_Coll_SphereTest_t *pTest )
{
	FASSERT( pTest );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );
	
	u32 i, ii;

	// Precalculate the kDOP intervals for the start and end points	
	_afTemp0Intervals[0] = pTest->vCenter.a[0];
	_afTemp0Intervals[1] = pTest->vCenter.a[1];
	_afTemp0Intervals[2] = pTest->vCenter.a[2];
	ii = FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].nNormalCount;
	for ( i = 3; i < ii; i++ )
	{
		_afTemp0Intervals[i] = pTest->vCenter.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
	}	
	
	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
#if !FANG_PRODUCTION_BUILD
		FMesh_Coll_nNodesChecked++;
#endif
	
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test this node's kDOP for collision
//		if ( !fkdop_IntersectkDOPAndSphere( &pTest->vCenter, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
		if ( !fkdop_IntersectkDOPAndSphere( _afTemp0Intervals, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
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
#if FANG_DEBUG_BUILD			
			if ( FMesh_Coll_nNodeStackIdx > FMesh_Coll_nDeepestNodeStackIdx )
			{
				FMesh_Coll_nDeepestNodeStackIdx = FMesh_Coll_nNodeStackIdx;
			}
#endif
			continue;
		}
	
#if FMESH_DRAW_COLLIDED_KDOPS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_pModelToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
		}
#endif // FMESH_DRAW_COLLIDED_KDOPS
		
		// This node is a leaf so we need to test collision against the tris
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Uncompress the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			f32 fU, fV, fSqDist;	
			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
#if _TRACK_TESTS			
				FMesh_Coll_nTests++;
#endif
				// Get the normal
				_vNormal.Set( *pNormals );

				// Note that normals are incremented across all packets.  This is correct.
				pNormals++;
				
#if FMESH_DRAW_CONSIDERED_TRIS
				nVert = pPacket->nStartVert + (ii * 3);
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
				fmesh_DrawMeshTri( vUncVerts, &_vNormal, &Color, TRUE );
#endif
				// Does it intersect the plane?
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_vODiff.Sub( pTest->vCenter, _avUncVerts[0] );
				f32 fDist = _vODiff.Dot( _vNormal );
				if ( fDist < -pTest->fRadius || fDist > pTest->fRadius )
				{
					continue;
				}

				// Test to see if collision point is inside tri
				
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
				_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );
				
#if _USE_HUNLEY_METHOD
				// Project the center of the sphere onto the plane that contains the tri
				_vImpact.Mul( _vNormal, fDist ).Negate().Add( pTest->vCenter );

#if _TRACK_TESTS			
				FMesh_Coll_nTestsLevel1++;
#endif
				// Get the U and V of the closest point on the triangle to the projected sphere center
				fSqDist = GetUVofClosestPoint( fU, fV, _vImpact, _avUncVerts[0], _vDiff1, _vDiff2 );
				if ( fSqDist > pTest->fRadiusSq )
				{
					// The projection of the sphere's center is more than the sphere's
					// radius away from the closest point on the triangle
					continue;
				}
				else if ( fSqDist == 0 )
				{
					// The sphere's center projects into the triangle
					_vPush.Set( _vNormal );
#if _TRACK_TESTS
				FMesh_Coll_nTestsLevel2++;
#endif

				}
				else
				{
#if _TRACK_TESTS
				FMesh_Coll_nTestsLevel2++;
#endif

					// The sphere's center does not project onto the triangle, but
					// we cannot trivially reject or accept the closest point on the
					// triangle, so we need to convert the UV to a 3D coordinate
					_vDiff1.Mul( _avUncVerts[0], 1 - fU - fV );
					_vDiff2.Mul( _avUncVerts[1], fU );
					_vImpact.Mul( _avUncVerts[2], fV );
					_vImpact.Add( _vDiff1 ).Add( _vDiff2 );
					
					// Do a quick test to see if the point is outside the radius
					_vPush.Sub( pTest->vCenter, _vImpact );
					fDist = _vPush.MagSq();
					if ( fDist > pTest->fRadiusSq )
					{
						continue;
					}
					// We've got a point that falls within the radius
					fDist = fmath_Sqrt( fDist );
					FASSERT( fDist != 0.0f );
					_vPush.Mul( fmath_Inv( fDist ) );
				}
#else
				_vCross.Cross( _vDiff2, _vNormal );
				f32 fDet = _vDiff1.Dot( _vCross );
				
				f32 fU = _vODiff.Dot( _vCross );
				if ( fU < 0.f )
				{
					// The center projects behind edge V2 - V0.  We should
					// check to see if the edge is within the radius
					f32 fPosition = fmath_Div( _vODiff.Dot( _vDiff2 ), _vDiff2.MagSq() );
					if ( fPosition <= 0 )
					{
						_vImpact.Set( _avUncVerts[0] );
					}
					else if ( fPosition >= 1 )
					{
						_vImpact.Set( _avUncVerts[2] );
					}
					else
					{
						_vImpact.Add( _avUncVerts[0], _vDiff2.Mul( fPosition) );
					}
					_vPush.Sub( pTest->vCenter, _vImpact );
					fDist = _vPush.MagSq();
					if ( fDist > pTest->fRadiusSq )
					{
						continue;
					}
					fDist = fmath_Sqrt( fDist );
					FASSERT( fDist != 0.0f );
					_vPush.Mul( fmath_Inv( fDist ) );
				}
				else
				{
					_vCross.Cross( _vDiff1, _vODiff );
					f32 fV = _vNormal.Dot( _vCross );
					if ( fV < 0.f )
					{
						// The center projects behind edge V1 - V0.  We should
						// check to see if the edge is within the radius
						f32 fPosition = fmath_Div( _vODiff.Dot( _vDiff1 ), _vDiff1.MagSq() );
						if ( fPosition <= 0 )
						{
							_vImpact.Set( _avUncVerts[0] );
						}
						else if ( fPosition >= 1 )
						{
							_vImpact.Set( _avUncVerts[2] );
						}
						else
						{
							_vImpact.Add( _avUncVerts[0], _vDiff1.Mul( fPosition) );
						}
						_vPush.Sub( pTest->vCenter, _vImpact );
						fDist = _vPush.MagSq();
						if ( fDist > pTest->fRadiusSq )
						{
							continue;
						}
						fDist = fmath_Sqrt( fDist );
						FASSERT( fDist != 0.0f );
						_vPush.Mul( fmath_Inv( fDist ) );
					}
					else if ( fU + fV > fDet )
					{
						// The center projects behind edge V1 - V2.  We should
						// check to see if the edge is within the radius
						_vDiff2.Sub( _avUncVerts[2], _avUncVerts[1] );
						_vODiff.Sub( pTest->vCenter, _avUncVerts[1] );
						f32 fPosition = fmath_Div( _vODiff.Dot( _vDiff2 ), _vDiff2.MagSq() );
						if ( fPosition <= 0 )
						{
							_vImpact.Set( _avUncVerts[1] );
						}
						else if ( fPosition >= 1 )
						{
							_vImpact.Set( _avUncVerts[2] );
						}
						else
						{
							_vImpact.Add( _avUncVerts[1], _vDiff2.Mul( fPosition) );
						}
						_vPush.Sub( pTest->vCenter, _vImpact );
						fDist = _vPush.MagSq();
						if ( fDist > pTest->fRadiusSq )
						{
							continue;
						}
						fDist = fmath_Sqrt( fDist );
						FASSERT( fDist != 0.0f );
						_vPush.Mul( fmath_Inv( fDist ) );
					}
					else
					{
						_vImpact.Mul( _vNormal, fDist );
						_vImpact.Negate();
						_vImpact.Add( pTest->vCenter );
						_vPush.Set( _vNormal );
					}
				}
#endif
				// We have collision

#if _TRACK_TESTS			
				FMesh_Coll_nHits++;
#endif
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

				// Setup the impact in the impact buffer
				FColl_pNextImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
				FColl_pNextImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
				FColl_pNextImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
				FColl_pNextImpactSlot->UnitFaceNormal.Set( _vNormal );
				FColl_pNextImpactSlot->ImpactPoint.Set( _vImpact );
				FColl_pNextImpactSlot->PushUnitVec.Set( _vPush );
				FColl_pNextImpactSlot->fImpactDistInfo = pTest->fRadius - fDist;
//				FColl_pNextImpactSlot->nSphereIndex = 0;
				FColl_pNextImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
				FColl_pNextImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;

				Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
				
				FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
				
				FColl_pNextImpactSlot++;
				FColl_nImpactCount++;
				
				if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask )
				{
					return;
				}
			}
		}
	}
}


//
//
//
void fmesh_Coll_TestRayWithDOPs_NoBF( FMesh_Coll_RayTest_t *pTest )
{
	FASSERT( pTest );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );

	u32 i, ii;
	f32 fDist;
	
	// Precalculate the kDOP intervals for the start and end points	
	_afTemp0Intervals[0] = pTest->vStart.a[0];
	_afTemp1Intervals[0] = pTest->vEnd.a[0];
	_afTemp0Intervals[1] = pTest->vStart.a[1];
	_afTemp1Intervals[1] = pTest->vEnd.a[1];
	_afTemp0Intervals[2] = pTest->vStart.a[2];
	_afTemp1Intervals[2] = pTest->vEnd.a[2];
	ii = FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].nNormalCount;
	for ( i = 3; i < ii; i++ )
	{
		_afTemp0Intervals[i] = pTest->vStart.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
		_afTemp1Intervals[i] = pTest->vEnd.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
	}	
	
	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
#if !FANG_PRODUCTION_BUILD
		FMesh_Coll_nNodesChecked++;
#endif
	
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test the node's kDOP for collision
		if ( fkdop_IntersectkDOPAndRay( _afTemp0Intervals, _afTemp1Intervals, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) == -1.f )
		{
			continue;
		}
		
		if ( !pNode->paPackets )
		{
			//
			// THIS NODE IS NOT A LEAF SO WE NEED TO PUSH ITS CHILDREN ONTO THE STACK
			//
			FASSERT( FMesh_Coll_nNodeStackIdx + 2 <= FMESH_COLL_NODE_STACK_SIZE );
			FASSERT( pNode->nStartChildIdx + 1 < FMesh_Coll_pCurrkDOPTree->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx + 1];
#if FANG_DEBUG_BUILD			
			if ( FMesh_Coll_nNodeStackIdx > FMesh_Coll_nDeepestNodeStackIdx )
			{
				FMesh_Coll_nDeepestNodeStackIdx = FMesh_Coll_nNodeStackIdx;
			}
#endif
			continue;
		}
		
#if FMESH_DRAW_COLLIDED_KDOPS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_pModelToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
		}
#endif // FMESH_DRAW_COLLIDED_KDOPS
		
		//
		// THIS NODE IS A LEAF SO WE NEED TO TEST FOR COLLISION AGAINST THE TRIS
		//
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Uncompress the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
#if _TRACK_TESTS			
				FMesh_Coll_nTests++;
#endif
				// get the normal in preparation for checks
				_vNormal.Set( *pNormals );

				// Note that normals are incremented across all packets.  This is intended behaviour.
				pNormals++;
				
#if FMESH_DRAW_CONSIDERED_TRIS
				nVert = pPacket->nStartVert + (ii * 3);
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
				fmesh_DrawMeshTri( vUncVerts, &_vNormal, &Color, TRUE );
#endif
				// Is it facing the poly
				f32 fFaceDot = -pTest->vNormal.Dot( _vNormal );
				if ( fFaceDot < 0.0001f )
				{
					// The requestor wants us to cull backface polys.
					continue;
				}

				// Does it intersect the plane?
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
				fDist = _vODiff.Dot( _vNormal );
				if ( fDist <= 0.f )
				{
					// If the origin is behind the poly and we're culling
					// backfaces, then we're not interested in this collision
					continue;
				}
				
				FASSERT( fFaceDot >= 0.0001f );
				fDist *= fmath_Inv( fFaceDot );
				if ( fDist > pTest->fDistance )
				{
					// The ray is not long enough to intersect the plane
					continue;
				}

#if _TRACK_TESTS			
				FMesh_Coll_nTestsLevel1++;
#endif
				// Test to see if collision point is inside tri
				
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
				_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );

				_vCross.Cross( pTest->vNormal, _vDiff2 );
				f32 fDet = _vDiff1.Dot( _vCross );
				if ( fmath_Abs( fDet ) < 0.0001f )
				{
					continue;
				}
				
				f32 fU = _vODiff.Dot( _vCross );
				if ( fU < 0.f || fU > fDet )
				{
					continue;
				}
				
				_vCross.Cross( _vODiff, _vDiff1 );
				f32 fV = pTest->vNormal.Dot( _vCross );
				if ( fV < 0.f || fU + fV > fDet )
				{
					continue; 
				}
				
				// WE'VE GOT COLLISION!!!
				
#if _TRACK_TESTS			
				FMesh_Coll_nHits++;
#endif
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

				// Setup the Impact in the impact buffer
				_vDiff1.Mul( pTest->vNormal, fDist );
				_vDiff1.Add( pTest->vStart );
				FColl_pNextImpactSlot->fImpactDistInfo = fmath_Div( fDist, pTest->fOriginalDistance );
				FMATH_CLAMP( FColl_pNextImpactSlot->fImpactDistInfo, 0.f, 1.f );
				FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.0f );
				FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
				FColl_pNextImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
				FColl_pNextImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
				FColl_pNextImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
				FColl_pNextImpactSlot->UnitFaceNormal.Set( _vNormal );
				FColl_pNextImpactSlot->ImpactPoint.Set( _vDiff1 );
				FColl_pNextImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
				FColl_pNextImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;
				
				Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
				
				FColl_pNextImpactSlot++;
				FColl_nImpactCount++;

				if ( FMesh_Coll_pCurrCollInfo->bFindClosestImpactOnly )
				{
					pTest->vEnd.Set( _vDiff1 );
					pTest->fDistance = fDist;
				}
				
				if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask )
				{
					return;
				}
			}
		}
	}
}


//
//
//
void fmesh_Coll_TestRayWithDOPs_BF( FMesh_Coll_RayTest_t *pTest )
{
	FASSERT( pTest );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );
	
	u32 i, ii;
	f32 fDist;
	
	// Precalculate the kDOP intervals for the start and end points	
	_afTemp0Intervals[0] = pTest->vStart.a[0];
	_afTemp1Intervals[0] = pTest->vEnd.a[0];
	_afTemp0Intervals[1] = pTest->vStart.a[1];
	_afTemp1Intervals[1] = pTest->vEnd.a[1];
	_afTemp0Intervals[2] = pTest->vStart.a[2];
	_afTemp1Intervals[2] = pTest->vEnd.a[2];
	ii = FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].nNormalCount;
	for ( i = 3; i < ii; i++ )
	{
		_afTemp0Intervals[i] = pTest->vStart.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
		_afTemp1Intervals[i] 	= pTest->vEnd.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
	}	
	
	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
#if !FANG_PRODUCTION_BUILD
		FMesh_Coll_nNodesChecked++;
#endif
	
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test the node's kDOP for collision
//		if ( fkdop_IntersectkDOPAndRay( &pTest->vStart, &pTest->vEnd, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) == -1.f )
		if ( fkdop_IntersectkDOPAndRay( _afTemp0Intervals, _afTemp1Intervals, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) == -1.f )
		{
			continue;
		}
		
		if ( !pNode->paPackets )
		{
			//
			// THIS NODE IS NOT A LEAF SO WE NEED TO PUSH ITS CHILDREN ONTO THE STACK
			//
			FASSERT( FMesh_Coll_nNodeStackIdx + 2 <= FMESH_COLL_NODE_STACK_SIZE );
			FASSERT( pNode->nStartChildIdx + 1 < FMesh_Coll_pCurrkDOPTree->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx + 1];
#if FANG_DEBUG_BUILD			
			if ( FMesh_Coll_nNodeStackIdx > FMesh_Coll_nDeepestNodeStackIdx )
			{
				FMesh_Coll_nDeepestNodeStackIdx = FMesh_Coll_nNodeStackIdx;
			}
#endif
			continue;
		}
		
#if FMESH_DRAW_COLLIDED_KDOPS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_pModelToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
		}
#endif // FMESH_DRAW_COLLIDED_KDOPS
		
		//
		// THIS NODE IS A LEAF SO WE NEED TO TEST FOR COLLISION AGAINST THE TRIS
		//
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Uncompress the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
#if _TRACK_TESTS			
				FMesh_Coll_nTests++;
#endif
				// Get the normal in preparation for checks
				_vNormal.Set( *pNormals );

				// Note that normals are incremented across all packets.  This is intended behaviour.
				pNormals++;
				
#if FMESH_DRAW_CONSIDERED_TRIS
				nVert = pPacket->nStartVert + (ii * 3);
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
				fmesh_DrawMeshTri( vUncVerts, &_vNormal, &Color, TRUE );
#endif
				// Does it intersect the plane?
				f32 fFaceDot = -pTest->vNormal.Dot( _vNormal );
				if ( fFaceDot == 0.0f )
				{
					// If the dot is zero, then the face is parallel with the ray
					continue;
				}
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
				fDist = _vODiff.Dot( _vNormal );
				if ( fDist < 0.f )
				{
					// The ray is not long enough to intersect the plane or pointing the wrong way
					continue;
				}

				FASSERT( fFaceDot >= 0.0001f );
				fDist *= fmath_Inv( fFaceDot );
				if ( fDist > pTest->fDistance )
				{
					// The ray is not long enough to intersect the plane
					continue;
				}

#if _TRACK_TESTS			
				FMesh_Coll_nTestsLevel1++;
#endif
				// Test to see if collision point is inside tri
				
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
				_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );

				_vCross.Cross( pTest->vNormal, _vDiff2 );
				f32 fDet = _vDiff1.Dot( _vCross );
				if ( fmath_Abs( fDet ) < 0.0001f )
				{
					continue;
				}
				
				FASSERT( fDet != 0.0f );
				fDet = fmath_Inv( fDet );
				
				f32 fU = _vODiff.Dot( _vCross ) * fDet;
				if ( fU < 0.f || fU > 1.f )
				{
					continue;
				}
				
				_vCross.Cross( _vODiff, _vDiff1 );
				f32 fV = pTest->vNormal.Dot( _vCross ) * fDet;
				if ( fV < 0.f || fU + fV > 1.f )
				{
					continue;
				}
				
				// WE'VE GOT COLLISION!!!
				
#if _TRACK_TESTS			
				FMesh_Coll_nHits++;
#endif
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

				// Setup the Impact in the impact buffer
				_vDiff1.Mul( pTest->vNormal, fDist );
				_vDiff1.Add( pTest->vStart );
				FColl_pNextImpactSlot->fImpactDistInfo = fmath_Div( fDist, pTest->fOriginalDistance );
				FMATH_CLAMP( FColl_pNextImpactSlot->fImpactDistInfo, 0.f, 1.f );
				FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.0f );
				FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
				FColl_pNextImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
				FColl_pNextImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
				FColl_pNextImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
				FColl_pNextImpactSlot->UnitFaceNormal.Set( _vNormal );
				FColl_pNextImpactSlot->ImpactPoint.Set( _vDiff1 );
				FColl_pNextImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
				FColl_pNextImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;
				
				Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
				
				FColl_pNextImpactSlot++;
				FColl_nImpactCount++;
				
				if ( FMesh_Coll_pCurrCollInfo->bFindClosestImpactOnly )
				{
					pTest->vEnd.Set( _vDiff1 );
					pTest->fDistance = fDist;
				}
				
				if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask )
				{
					return;
				}
			}
		}
	}
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
			if ( (fRoot1 < 0) || (fRoot1 >= fT) ) 
			{
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
BOOL fmesh_Coll_TestProjSphereWithDOPs( FMesh_Coll_ProjSphereTest_t *pTest )
{
	FASSERT( pTest );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );

	BOOL bCollisionOccurred = FALSE;
	
	u32 i, ii;
	f32 fDist;
	CFVec3A vTemp1, vTemp2;
	
	// Precalculate the kDOP intervals for the start and end points	
	_afTemp0Intervals[0] = pTest->vStart.a[0];
	_afTemp1Intervals[0] = pTest->vEnd.a[0];
	_afTemp0Intervals[1] = pTest->vStart.a[1];
	_afTemp1Intervals[1] = pTest->vEnd.a[1];
	_afTemp0Intervals[2] = pTest->vStart.a[2];
	_afTemp1Intervals[2] = pTest->vEnd.a[2];
	ii = FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].nNormalCount;
	for ( i = 3; i < ii; i++ )
	{
		_afTemp0Intervals[i] = pTest->vStart.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
		_afTemp1Intervals[i] = pTest->vEnd.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
	}	
	
	const CFProjSphere *pProjSphere = &FMesh_Coll_pCurrCollInfo->ProjSphere;

	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
#if !FANG_PRODUCTION_BUILD
		FMesh_Coll_nNodesChecked++;
#endif
	
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test the top node's kDOP for collision
//		if ( !fkdop_IntersectkDOPAndCapsule( &pTest->vStart, &pTest->vEnd, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
		if ( !fkdop_IntersectkDOPAndCapsule( _afTemp0Intervals, _afTemp1Intervals, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
		{
			continue;
		}
		
		if ( !pNode->paPackets )
		{
			//
			// THIS NODE IS NOT A LEAF SO WE NEED TO PUSH ITS CHILDREN ONTO THE STACK
			//
			FASSERT( FMesh_Coll_nNodeStackIdx + 2 <= FMESH_COLL_NODE_STACK_SIZE );
			FASSERT( pNode->nStartChildIdx + 1 < FMesh_Coll_pCurrkDOPTree->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx + 1];
#if FANG_DEBUG_BUILD			
			if ( FMesh_Coll_nNodeStackIdx > FMesh_Coll_nDeepestNodeStackIdx )
			{
				FMesh_Coll_nDeepestNodeStackIdx = FMesh_Coll_nNodeStackIdx;
			}
#endif
			continue;
		}
		
#if FMESH_DRAW_COLLIDED_KDOPS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_pModelToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
		}
#endif // FMESH_DRAW_COLLIDED_KDOPS
		
		//
		// THIS NODE IS A LEAF SO WE NEED TO TEST FOR COLLISION AGAINST THE TRIS
		//
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Uncompress the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			f32 fDist1, fDist2;
			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
#if _TRACK_TESTS			
				FMesh_Coll_nTests++;
#endif
				// Get the normal in preparation for checks
				_vNormal.Set( *pNormals );

				// Note that normals are incremented across all packets.  This is intended behaviour.
				pNormals++;

#if FMESH_DRAW_CONSIDERED_TRIS
				nVert = pPacket->nStartVert + (ii * 3);
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
				fmesh_DrawMeshTri( vUncVerts, &_vNormal, &Color, TRUE );
#endif
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				
				// Does it intersect the plane?
				fDist = fDist2 = -1.f;
				f32 fFaceDot = pTest->vMoveNormal.Dot( _vNormal );
				
				if ( FMesh_Coll_pCurrCollInfo->bCullBacksideCollisions && fFaceDot > 0.f )
				{
					continue;
				}

				if ( fmath_Abs( fFaceDot ) < 0.0001f )
				{
					// If the dot is zero, then the face is parallel with the ray
					// We still need to test to see if the cylinder's radius intersects
					_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
					fDist2 = _vODiff.Dot( _vNormal );

					if ( fDist2 > pTest->fRadius || fDist2 < -pTest->fRadius )
					{
						// Plane is beyond radius
						continue;
					}
					
					_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
					_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
					_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
				}
				else
				{
					// Create a vector from the point where collision would occur on the sphere
					// projected through the height
					if ( fFaceDot > 0.f )
					{
						vTemp1.Mul( _vNormal, pTest->fRadius ).Add( pTest->vStart );
					}
					else
					{
						vTemp1.Mul( _vNormal, -pTest->fRadius ).Add( pTest->vStart );
					}					
					
					vTemp2.Mul( pTest->vMoveNormal, pTest->fMoveDistance ).Add( vTemp1 );
					
					// Determine if this line intersects the plane
					_vDiff1.Sub( vTemp1, _avUncVerts[0] );
					fDist = _vDiff1.Dot( _vNormal );
					_vDiff2.Sub( vTemp2, _avUncVerts[0] );
					fDist1 = _vDiff2.Dot( _vNormal );

					_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
					_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
					if ( (fDist < 0.f) == (fDist1 < 0.f) )
					{
						fDist = -2.f;
					}
					else
					{
						// The line intersects the plane, so determine the distance to the 
						// intersection and the impact point
						fDist = pTest->fMoveDistance * fmath_Div( fDist, fDist - fDist1 );
						_vImpact.Mul( pTest->vMoveNormal, fDist ).Add( vTemp1 );
						FASSERT( fDist >= 0.f );

						// Determine if the impact point actually is inside the triangle					
						_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
						_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );
						_vODiff.Sub( _vImpact, _avUncVerts[0] );
						if ( !fcoll_PointIsInTri( _vODiff, _vDiff1, _vDiff2, _vNormal ) )
						{
							// This point is not in the tri, so we need to test the edges
							fDist = -1.f;
						}
					}
				}
				
#if _TRACK_TESTS
				FMesh_Coll_nTestsLevel1++;
#endif

				if ( fDist == -1.f )
				{
					f32 fT = 1.f;
					_ProjectSphereAgainstEdge( pTest->vStart, pTest->fRadius, pTest->vMoveDelta, _avUncVerts[0], _avUncVerts[1], fT, _vImpact );
					_ProjectSphereAgainstEdge( pTest->vStart, pTest->fRadius, pTest->vMoveDelta, _avUncVerts[1], _avUncVerts[2], fT, _vImpact );
					_ProjectSphereAgainstEdge( pTest->vStart, pTest->fRadius, pTest->vMoveDelta, _avUncVerts[2], _avUncVerts[0], fT, _vImpact );
					
					if ( fT == 1.f )
					{
						fDist = -2.f;
					}
					else
					{					
						fDist = fT * pTest->fMoveDistance;
						FASSERT( fDist >= 0.f );
					}
				}
				
				if ( fDist == -2.f )
				{
					if ( fDist2 == -1.f )
					{
						_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
						fDist2 = _vODiff.Dot( _vNormal );

						if ( fDist2 < -pTest->fRadius || fDist2 > pTest->fRadius )
						{
							continue;
						}
					}
					
					if ( !_IntersectSphereAndTri( &pTest->vStart, pTest->fRadius, pTest->fRadiusSq, TRUE ) )
					{
						continue;
					}
/*
					if ( fFaceDot > 0.f )
					{
						_vImpact.Mul( _vNormal, fmath_Abs( fDist2 ) ).Add( pTest->vStart );
					}
					else
					{
						_vImpact.Mul( _vNormal, fmath_Abs( fDist2 ) ).Add( pTest->vStart );
					}
					
					// Determine if the impact point actually is inside the triangle
					_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
					_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );
					_vODiff.Sub( _vImpact, _avUncVerts[0] );
					if ( !fcoll_PointIsInTri( _vODiff, _vDiff1, _vDiff2, _vNormal ) )
					{
						continue;
					}
*/
					fDist = 0.f;
				}

				// WE'VE GOT COLLISION!!!

				bCollisionOccurred = TRUE;

				FASSERT( fDist >= 0.f );

				fDist *= FMesh_Coll_fBoneScale;

				// Convert the impact data to world space
				if ( FMesh_pModelToWorld && FMesh_Coll_pCurrCollInfo->bCalculateImpactData )
				{
					FMesh_pModelToWorld->MulPoint( _avUncVerts[0] );
					FMesh_pModelToWorld->MulPoint( _avUncVerts[1] );
					FMesh_pModelToWorld->MulPoint( _avUncVerts[2] );
					FMesh_pModelToWorld->MulPoint( _vImpact );

					if ( FMesh_Coll_fBoneScaleR == 1.f )
					{
						FMesh_pModelToWorld->MulDir( _vNormal );
					}
					else
					{
						FMesh_pModelToWorld->MulDir( _vNormal.Mul( FMesh_Coll_fBoneScaleR ) );
					}
				}

				// If the caller requested closest impact only, see if this impact
				// is closer than the current closest:
				if ( FMesh_Coll_pCurrCollInfo->bFindClosestImpactOnly )
				{
					if ( !FMesh_Coll_pClosestImpactSlot )
					{
						// If this is NULL, then we haven't calculated a collision yet
						FMesh_Coll_pClosestImpactSlot = FColl_pNextImpactSlot;
						FColl_pNextImpactSlot++;
						FColl_nImpactCount++;
					}
					else if ( fDist > FMesh_Coll_pClosestImpactSlot->fImpactDistInfo ) 
					{
						continue;
					}

#if _TRACK_TESTS			
					FMesh_Coll_nHits++;
#endif

					// Clip the ray to the new short distance
					pTest->fMoveDistance = fDist;
					pTest->vMoveDelta.Mul( pTest->vMoveNormal, fDist );
					pTest->vEnd.Add( pTest->vStart, pTest->vMoveDelta );
					
					// Setup the Impact in the impact buffer
					FMesh_Coll_pClosestImpactSlot->fImpactDistInfo = fDist;
					FMesh_Coll_pClosestImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
					FMesh_Coll_pClosestImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;
					FMesh_Coll_pClosestImpactSlot->nUserType = pPacket->nCollType;

					Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
				
					if ( FMesh_Coll_pCurrCollInfo->bCalculateImpactData )
					{
						FMesh_Coll_pClosestImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
						FMesh_Coll_pClosestImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
						FMesh_Coll_pClosestImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
						FMesh_Coll_pClosestImpactSlot->UnitFaceNormal.Set( _vNormal );
						FMesh_Coll_pClosestImpactSlot->ImpactPoint.Set( _vImpact );
					}
				}
				else
				{
#if _TRACK_TESTS			
					FMesh_Coll_nHits++;
#endif
					// Check for collision buffer overflow
					if ( FColl_pNextImpactSlot == FColl_pImpactBufEnd )
					{
						#if FANG_PRODUCTION_BUILD
							return bCollisionOccurred;
						#else
							FColl_nImpactCountOverLimit++;
							continue;
						#endif
					}

					// Setup the Impact in the impact buffer
					FColl_pNextImpactSlot->fImpactDistInfo = fDist;
					FColl_pNextImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
					FColl_pNextImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;
					FColl_pNextImpactSlot->nUserType = pPacket->nCollType;

					if ( FMesh_Coll_pCurrCollInfo->bCalculateImpactData )
					{
						FColl_pNextImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
						FColl_pNextImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
						FColl_pNextImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
						FColl_pNextImpactSlot->UnitFaceNormal.Set( _vNormal );
						FColl_pNextImpactSlot->ImpactPoint.Set( _vImpact );
					}

					Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
				
					FColl_pNextImpactSlot++;
					FColl_nImpactCount++;
				}

				if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask )
				{
					return bCollisionOccurred;
				}
			}
		}
	}

	return bCollisionOccurred;
}


//
//
//
static BOOL _IntersectSphereAndTri( CFVec3A *pCenter, f32 fRadius, f32 fRadiusSq, BOOL bMasterSphere )
{
	f32 fSqDist, fU, fV, fDist;
	// Does it intersect the plane?
	_vODiff.Sub( *pCenter, _avUncVerts[0] );
	fDist = _vODiff.Dot( _vNormal );
	if ( fDist < -fRadius || fDist > fRadius )
	{
		return FALSE;
	}

	_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
	_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );
	
#if _USE_HUNLEY_METHOD
	// Project the center of the sphere onto the plane that contains the tri
	_vImpact.Mul( _vNormal, fDist ).Negate().Add( *pCenter );

	// Get the U and V of the closest point on the triangle to the projected sphere center
	fSqDist = GetUVofClosestPoint( fU, fV, _vImpact, _avUncVerts[0], _vDiff1, _vDiff2 );
	if ( fSqDist > fRadiusSq )
	{
		// The projection of the sphere's center is more than the sphere's
		// radius away from the closest point on the triangle
		return FALSE;
	}
	else if ( fSqDist == 0 )
	{
		if ( bMasterSphere )
		{
			return TRUE;
		}
		// The sphere's center projects into the triangle
		_vPush.Set( _vNormal );
	}
	else
	{
		// The sphere's center does not project onto the triangle, but
		// we cannot trivially reject or accept the closest point on the
		// triangle, so we need to convert the UV to a 3D coordinate
		_vDiff1.Mul( _avUncVerts[0], 1 - fU - fV );
		_vDiff2.Mul( _avUncVerts[1], fU );
		_vImpact.Mul( _avUncVerts[2], fV );
		_vImpact.Add( _vDiff1 ).Add( _vDiff2 );
		
		// Do a quick test to see if the point is outside the radius
		_vPush.Sub( *pCenter, _vImpact );
		fDist = _vPush.MagSq();
		if ( fDist > fRadiusSq )
		{
			return FALSE;
		}
		// We've got a point that falls within the radius
		if ( bMasterSphere )
		{
			return TRUE;
		}
		fDist = fmath_Sqrt( fDist );
		FASSERT( fDist != 0.0f );
		_vPush.Mul( fmath_Inv( fDist ) );
	}
#else
	_vCross.Cross( _vDiff2, _vNormal );
	f32 fDet = _vDiff1.Dot( _vCross );
	f32 fRadDet = pTest->fRadius * _vCross.Mag();
	
	f32 fU = _vODiff.Dot( _vCross );
	if ( fU + fRadDet < 0.f || fU - fRadDet > fDet )
	{
		return FALSE;
	}
	else if ( fU < 0.f )
	{
		// The center projects behind edge V2 - V0.  We should
		// check to see if the edge is within the radius
		f32 fPosition = fmath_Div( _vODiff.Dot( _vDiff2 ), _vDiff2.MagSq() );
		if ( fPosition <= 0 )
		{
			_vImpact.Set( _avUncVerts[0] );
		}
		else if ( fPosition >= 1 )
		{
			_vImpact.Set( _avUncVerts[2] );
		}
		else
		{
			_vImpact.Add( _avUncVerts[0], _vDiff2.Mul( fPosition) );
		}
		_vPush.Sub( pTest->vCenter, _vImpact );
		fDist = _vPush.MagSq();
		if ( fDist > pTest->fRadiusSq )
		{
			return FALSE;
		}
		if ( bMasterSphere )
		{
			return TRUE;
		}
		fDist = fmath_Sqrt( fDist );
		FASSERT( fDist != 0.0f );
		_vPush.Mul( fmath_Inv( fDist ) );
	}
	else
	{
		_vCross.Cross( _vDiff1, _vODiff );
		f32 fV = _vNormal.Dot( _vCross );
		if ( fV + fRadDet < 0.f || fU + fV - fRadDet > fDet )
		{
			return FALSE;
		}
		if ( fV < 0.f )
		{
			// The center projects behind edge V1 - V0.  We should
			// check to see if the edge is within the radius
			f32 fPosition = fmath_Div( _vODiff.Dot( _vDiff1 ), _vDiff1.MagSq() );
			if ( fPosition <= 0 )
			{
				_vImpact.Set( _avUncVerts[0] );
			}
			else if ( fPosition >= 1 )
			{
				_vImpact.Set( _avUncVerts[1] );
			}
			else
			{
				_vImpact.Add( _avUncVerts[0], _vDiff1.Mul( fPosition) );
			}
			_vPush.Sub( pTest->vCenter, _vImpact );
			fDist = _vPush.MagSq();
			if ( fDist > pTest->fRadiusSq )
			{
				return FALSE;
			}
			if ( bMasterSphere )
			{
				return TRUE;
			}
			fDist = fmath_Sqrt( fDist );
			FASSERT( fDist != 0.0f );
			_vPush.Mul( fmath_Inv( fDist ) );
		}
		else if ( fU + fV > fDet )
		{
			// The center projects behind edge V1 - V2.  We should
			// check to see if the edge is within the radius
			_vDiff2.Sub( _avUncVerts[2], _avUncVerts[1] );
			_vODiff.Sub( pTest->vCenter, _avUncVerts[1] );
			f32 fPosition = fmath_Div( _vODiff.Dot( _vDiff2 ), _vDiff2.MagSq() );
			if ( fPosition <= 0 )
			{
				_vImpact.Set( _avUncVerts[1] );
			}
			else if ( fPosition >= 1 )
			{
				_vImpact.Set( _avUncVerts[2] );
			}
			else
			{
				_vImpact.Add( _avUncVerts[1], _vDiff2.Mul( fPosition) );
			}
			_vPush.Sub( pTest->vCenter, _vImpact );
			fDist = _vPush.MagSq();
			if ( fDist > pTest->fRadiusSq )
			{
				return FALSE;
			}
			if ( bMasterSphere )
			{
				return TRUE;
			}
			fDist = fmath_Sqrt( fDist );
			FASSERT( fDist != 0.0f );
			_vPush.Mul( fmath_Inv( fDist ) );
		}
		else
		{
			if ( bMasterSphere )
			{
				return TRUE;
			}
			_vImpact.Mul( _vNormal, fDist );
			_vImpact.Negate();
			_vImpact.Add( pTest->vCenter );
			_vPush.Set( _vNormal );
		}
	}
#endif
	// We have collision
	
	// Check for collision buffer overflow
	if ( FColl_pNextImpactSlot == FColl_pImpactBufEnd )
	{
		#if FANG_PRODUCTION_BUILD
			return FALSE;
		#else
			FColl_nImpactCountOverLimit++;
			return FALSE;
		#endif
	}

	FColl_pNextImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
	FColl_pNextImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
	FColl_pNextImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
	FColl_pNextImpactSlot->UnitFaceNormal.Set( _vNormal );
	FColl_pNextImpactSlot->ImpactPoint.Set( _vImpact );
	FColl_pNextImpactSlot->PushUnitVec.Set( _vPush );
	FColl_pNextImpactSlot->fImpactDistInfo = fRadius - fDist;
//	FColl_pNextImpactSlot->nSphereIndex = 0;
	FColl_pNextImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
	FColl_pNextImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;
	
	return TRUE;
}
				


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DOP v. DOP TESTS
//
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
void fmesh_Coll_TestDOPWithDOPs( FkDOP_Interval_t *pkDOPIntervals, u32 nkDOPType, u8 nCollMask )
{
}


#if FMESH_COLL_ENABLE_COLLISION_DRAWS
//
//
//
void fmesh_Coll_WalkTreeAndDraw( u32 nDrawFlags, CFMtx43A *pResultToWorld )
{
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );

	CFVec3A vTemp;
	f32 fRadius = FMesh_Coll_pCurrkDOPTree->BoundingCapsule.fRadius * fmath_Inv( FMesh_Coll_fBoneScaleR );
	if ( FMesh_pModelToWorld )
	{
//		vTemp = FMesh_pModelToWorld->m44.MultPoint( FMesh_Coll_pCurrkDOPTree->BoundingCapsule.vStart );
//		fdraw_FacetedWireSphere( &vTemp.v3, fRadius, 2, 2 );
//		vTemp = FMesh_pModelToWorld->m44.MultPoint( FMesh_Coll_pCurrkDOPTree->BoundingCapsule.vEnd );
//		fdraw_FacetedWireSphere( &vTemp.v3, fRadius, 2, 2 );
	}

	BOOL bRootkDOPDrawn = FALSE;

	u32 i, ii;
	
	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
		if ( (nDrawFlags & FMESH_DRAW_COLL_GEO_ALL_DOPS) 
			|| ((nDrawFlags & FMESH_DRAW_COLL_GEO_ROOT_DOPS) && !bRootkDOPDrawn) )
		{
			FkDOP_DrawkDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], pResultToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
			bRootkDOPDrawn = TRUE;
		}
		
		if ( !pNode->paPackets )
		{
			//
			// THIS NODE IS NOT A LEAF SO WE NEED TO PUSH ITS CHILDREN ONTO THE STACK
			//
			FASSERT( FMesh_Coll_nNodeStackIdx + 2 <= FMESH_COLL_NODE_STACK_SIZE );
			FASSERT( pNode->nStartChildIdx + 1 < FMesh_Coll_pCurrkDOPTree->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx + 1];
			continue;
		}
		
		//
		// THIS NODE IS A LEAF
		//
		
		if ( nDrawFlags & FMESH_DRAW_COLL_GEO_LEAF_DOPS )
		{
			FkDOP_DrawkDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], pResultToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
		}

		if ( !(nDrawFlags & FMESH_DRAW_COLL_GEO_LEAF_TRIS) )
		{
			continue;
		}
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Uncompress the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];
			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
				// Get the normal in preparation for checks
				_vNormal.Set( *pNormals );

				// Note that normals are incremented across all packets.  This is intended behaviour.
				pNormals++;
				
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
				if ( pPacket->nCollMask != 0xffff )
				{
					Color.Set( 0.75, 0, 0, 1.f );
				}
				fmesh_Coll_DrawMeshTri( _avUncVerts, &_vNormal, pResultToWorld, &Color, TRUE );
			}
		}
	}
}
#endif // !FANG_PRODUCTION_BUILD




/*
//
//
//
void fmesh_Coll_TestSphereListWithDOPs( FMesh_Coll_SphereTest_t *pTest )
{
	FASSERT( pTest );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );
	BOOL bChildrenTransformed = FALSE;
	
	u32 i, ii, iii;

	// Precalculate the kDOP intervals for the center point	
	_afTemp0Intervals[0] = pTest->vCenter.a[0];
	_afTemp0Intervals[1] = pTest->vCenter.a[1];
	_afTemp0Intervals[2] = pTest->vCenter.a[2];
	ii = FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].nNormalCount;
	for ( i = 3; i < ii; i++ )
	{
		_afTemp0Intervals[i] = pTest->vCenter.Dot( FkDOP_aDesc[FMesh_Coll_pCurrkDOPTree->nTreekDOPType].avNormals[i] );
	}	
	
	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];

#if !FANG_PRODUCTION_BUILD
		FMesh_Coll_nNodesChecked++;
#endif
	
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test this node's kDOP for collision
//		if ( !fkdop_IntersectkDOPAndSphere( &pTest->vCenter, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
		if ( !fkdop_IntersectkDOPAndSphere( _afTemp0Intervals, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
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
#if FANG_DEBUG_BUILD			
			if ( FMesh_Coll_nNodeStackIdx > FMesh_Coll_nDeepestNodeStackIdx )
			{
				FMesh_Coll_nDeepestNodeStackIdx = FMesh_Coll_nNodeStackIdx;
			}
#endif
			continue;
		}
	
		// This node is a leaf so we need to test collision against the tris
		
		// First, Make sure that there is collision with the list spheres
		if ( !bChildrenTransformed )
		{
			fmesh_Coll_TransformSphereList( pTest );
		}
		
		u64 nChildCollFlags = 0;
		
		// Test the child spheres for collision with the node's kDOP
		FMesh_Coll_SphereTest_t *pChild = FMesh_Coll_spListXfm;
		for ( i = 0; i < FMesh_Coll_pCurrCollInfo->nSphereListCount; i++, pChild++ )
		{
			if ( fkdop_IntersectkDOPAndSphere( &pChild->vCenter, pChild->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
			{
				// Flag this child as colliding
				nChildCollFlags |= (1 << i);
			}
		}
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Test the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
				// get the normal in preparation for checks
				_vNormal.Set( *pNormals );

				// Note that normals are incrememted across all packets.  This is correct.
				pNormals++;
				
				// Does it intersect the plane?
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_vODiff.Sub( pTest->vCenter, _avUncVerts[0] );
				f32 fDist = _vODiff.Dot( _vNormal );
				if ( fDist < -pTest->fRadius || fDist > pTest->fRadius )
				{
					continue;
				}

				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				
				// Test the master sphere to see if collision point is inside tri
				if ( !_IntersectSphereAndTri( &pTest->vCenter, pTest->fRadius, pTest->fRadiusSq, TRUE ) )
				{
					continue;
				}
				
				// Test the children
				FMesh_Coll_SphereTest_t *pChild = FMesh_Coll_spListXfm;
				if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask )
				{
					for ( iii = 0; iii < FMesh_Coll_pCurrCollInfo->nSphereListCount; iii++, pChild++ )
					{
						if ( !nChildCollFlags && (1 << i) )
						{
							continue;
						}
						
						if ( _IntersectSphereAndTri( &pChild->vCenter, pChild->fRadius, pChild->fRadiusSq, FALSE ) )
						{
							Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
							FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
							FColl_pNextImpactSlot->nSphereIndex = (u8)iii;
							FColl_pNextImpactSlot++;
							FColl_nImpactCount++;
							
							if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask  )
							{
								return;
							}

							return;
						}
					}
				}
				else
				{
					for ( iii = 0; iii < FMesh_Coll_pCurrCollInfo->nSphereListCount; iii++, pChild++ )
					{
						if ( !nChildCollFlags && (1 << i) )
						{
							continue;
						}
						
						if ( _IntersectSphereAndTri( &pChild->vCenter, pChild->fRadius, pChild->fRadiusSq, FALSE ) )
						{
							if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask )
							{
								return;
							}

							Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
							FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
							FColl_pNextImpactSlot->nSphereIndex = (u8)iii;
							FColl_pNextImpactSlot++;
							FColl_nImpactCount++;
						}
					}
				}
			}
		}
	}
}
*/

/*
//
//
//
void fmesh_Coll_TestCapsuleWithDOPs( FMesh_Coll_CylinderTest_t *pTest )
{
	FASSERT( pTest );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );
	
	u32 i, ii;
	f32 fDist;
	CFVec3A vTemp1, vTemp2;
	
	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
#if !FANG_PRODUCTION_BUILD
		FMesh_Coll_nNodesChecked++;
#endif
	
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test the top node's kDOP for collision
		if ( !fkdop_IntersectkDOPAndCapsule( &pTest->vStart, &pTest->vEnd, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
		{
			continue;
		}
		
		if ( !pNode->paPackets )
		{
			//
			// THIS NODE IS NOT A LEAF SO WE NEED TO PUSH ITS CHILDREN ONTO THE STACK
			//
			FASSERT( FMesh_Coll_nNodeStackIdx + 2 <= FMESH_COLL_NODE_STACK_SIZE );
			FASSERT( pNode->nStartChildIdx + 1 < FMesh_Coll_pCurrkDOPTree->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx + 1];
#if FANG_DEBUG_BUILD			
			if ( FMesh_Coll_nNodeStackIdx > FMesh_Coll_nDeepestNodeStackIdx )
			{
				FMesh_Coll_nDeepestNodeStackIdx = FMesh_Coll_nNodeStackIdx;
			}
#endif
			continue;
		}
		
#if FMESH_DRAW_COLLIDED_KDOPS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_pModelToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
		}
#endif // FMESH_DRAW_COLLIDED_KDOPS
		
		//
		// THIS NODE IS A LEAF SO WE NEED TO TEST FOR COLLISION AGAINST THE TRIS
		//
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Uncompress the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			f32 fDist1, fDist2;
			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
#if _TRACK_TESTS			
				FMesh_Coll_nTests++;
#endif
				// Get the normal in preparation for checks
				_vNormal.Set( *pNormals );

				// Note that normals are incremented across all packets.  This is intended behaviour.
				pNormals++;

#if FMESH_DRAW_CONSIDERED_TRIS
				nVert = pPacket->nStartVert + (ii * 3);
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
				fmesh_DrawMeshTri( vUncVerts, &_vNormal, &Color, TRUE );
#endif
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				
				// Does it intersect the plane?
				fDist = fDist2 = -1.f;
				f32 fFaceDot = pTest->vNormal.Dot( _vNormal );
				if ( fmath_Abs( fFaceDot ) < 0.0001f )
				{
					// If the dot is zero, then the face is parallel with the ray
					// We still need to test to see if the cylinder's radius intersects
					_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
					fDist2 = _vODiff.Dot( _vNormal );

					if ( fDist2 > pTest->fRadius || fDist2 < -pTest->fRadius )
					{
						// Plane is beyond radius
						continue;
					}
					
					_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
				}
				else
				{
					// Create a vector from the point where collision would occur on the sphere
					// projected through the height
					if ( fFaceDot > 0.f )
					{
						vTemp1.Mul( _vNormal, pTest->fRadius ).Add( pTest->vStart );
					}
					else
					{
						vTemp1.Mul( _vNormal, -pTest->fRadius ).Add( pTest->vStart );
					}					
					
					vTemp2.Mul( pTest->vNormal, pTest->fHeight ).Add( vTemp1 );
					
					// Determine if this line intersects the plane
					_vDiff1.Sub( vTemp1, _avUncVerts[0] );
					fDist = _vDiff1.Dot( _vNormal );
					_vDiff2.Sub( vTemp2, _avUncVerts[0] );
					fDist1 = _vDiff2.Dot( _vNormal );

					if ( (fDist < 0.f) == (fDist1 < 0.f) )
					{
						fDist = -2.f;
					}
					else
					{
						// The line intersects the plane, so determine the distance to the 
						// intersection and the impact point
						fDist = pTest->fHeight * fmath_Div( fDist, fDist - fDist1 );
						_vImpact.Mul( pTest->vNormal, fDist ).Add( vTemp1 );
						FASSERT( fDist >= 0.f );

						// Determine if the impact point actually is inside the triangle					
						_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
						_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );
						_vODiff.Sub( _vImpact, _avUncVerts[0] );
						if ( !fcoll_PointIsInTri( _vODiff, _vDiff1, _vDiff2, _vNormal ) )
						{
							// This point is not in the tri, so we need to test the edges
							fDist = -1.f;
						}
					}
				}
				
#if _TRACK_TESTS
				FMesh_Coll_nTestsLevel1++;
#endif

				if ( fDist == -1.f )
				{
					CFVec3A vTravel;
					vTravel.Sub( pTest->vEnd, pTest->vStart );
					f32 fT = 1.f;
					_ProjectSphereAgainstEdge( pTest->vStart, pTest->fRadius, vTravel, _avUncVerts[0], _avUncVerts[1], fT, _vImpact );
					_ProjectSphereAgainstEdge( pTest->vStart, pTest->fRadius, vTravel, _avUncVerts[1], _avUncVerts[2], fT, _vImpact );
					_ProjectSphereAgainstEdge( pTest->vStart, pTest->fRadius, vTravel, _avUncVerts[2], _avUncVerts[0], fT, _vImpact );
					
					if ( fT == 1.f )
					{
						fDist = -2.f;
					}
					else
					{					
						fDist = fT * pTest->fHeight;
						FASSERT( fDist >= 0.f );
					}
				}
				
				if ( fDist == -2.f )
				{
					if ( fDist2 == -1.f )
					{
						_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
						fDist2 = _vODiff.Dot( _vNormal );

						if ( fDist2 < -pTest->fRadius || fDist2 > pTest->fRadius )
						{
							continue;
						}
					}
						
					if ( fFaceDot > 0.f )
					{
						_vImpact.Mul( _vNormal, fmath_Abs( fDist2 ) ).Add( pTest->vStart );
					}
					else
					{
						_vImpact.Mul( _vNormal, fmath_Abs( fDist2 ) ).Add( pTest->vStart );
					}
					
					// Determine if the impact point actually is inside the triangle					
					_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
					_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );
					_vODiff.Sub( _vImpact, _avUncVerts[0] );
					if ( !fcoll_PointIsInTri( _vODiff, _vDiff1, _vDiff2, _vNormal ) )
					{
						continue;
					}
					
					fDist = 0.f;
				}

				// WE'VE GOT COLLISION!!!
				FASSERT( fDist >= 0.f );

#if _TRACK_TESTS
				FMesh_Coll_nHits++;
#endif
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

				// Setup the Impact in the impact buffer
				FColl_pNextImpactSlot->fImpactDistInfo = fDist;
				FColl_pNextImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
				FColl_pNextImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
				FColl_pNextImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
				FColl_pNextImpactSlot->UnitFaceNormal.Set( _vNormal );
				FColl_pNextImpactSlot->ImpactPoint.Set( _vImpact );
				FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
				FColl_pNextImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
				FColl_pNextImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;

				Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
				
				FColl_pNextImpactSlot++;
				FColl_nImpactCount++;

				if ( FMesh_Coll_pCurrCollInfo->nStopOnFirstOfCollMask & pPacket->nCollMask )
				{
					return;
				}
			}
		}
	}
}
*/

/*
//
//
//
void fmesh_Coll_TestCylinderWithDOPs( FMesh_Coll_CylinderTest_t *pTest )
{
	FASSERT( pTest );
	FASSERT( FMesh_Coll_pCurrkDOPTree );
	FASSERT( FMesh_Coll_pCurrMeshInst );
	
	u32 i, ii, iGoodVerts, iVert, iLastVert;
	CFVec3A vTransPoints[3], vClippedPoints[5];
	CFVec3A vTransNorm, vTransDown, vTransRef;
	CFVec3A vPlaneImpact, vReference, vTemp, vRadialImpact, vHeightImpact;
	f32 fDist0, fDist1, fDist2, fImpactDist;
	
	const CFCylinder *pCyl = &FMesh_Coll_pCurrCollInfo->Cylinder;

	while ( FMesh_Coll_nNodeStackIdx )
	{
		// Pop the pointer to the node on the top of the stack
		FkDOP_Node_t *pNode = FMesh_Coll_apNodeStack[--FMesh_Coll_nNodeStackIdx];
		
#if !FANG_PRODUCTION_BUILD
		FMesh_Coll_nNodesChecked++;
#endif
	
		// Make sure this node's mask matches
		if ( !(pNode->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
		{
			continue;
		}

		// Test the top node's kDOP for collision
		if ( !fkdop_IntersectkDOPAndCapsule( &pTest->vStart, &pTest->vEnd, pTest->fRadius, &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_Coll_pCurrkDOPTree->nTreekDOPType ) )
		{
			continue;
		}
		
		if ( !pNode->paPackets )
		{
			//
			// THIS NODE IS NOT A LEAF SO WE NEED TO PUSH ITS CHILDREN ONTO THE STACK
			//
			FASSERT( FMesh_Coll_nNodeStackIdx + 2 <= FMESH_COLL_NODE_STACK_SIZE );
			FASSERT( pNode->nStartChildIdx + 1 < FMesh_Coll_pCurrkDOPTree->nTreeNodeCount );
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx];
			FMesh_Coll_apNodeStack[FMesh_Coll_nNodeStackIdx++] = &FMesh_Coll_pCurrkDOPTree->pakDOPNodes[pNode->nStartChildIdx + 1];
#if FANG_DEBUG_BUILD			
			if ( FMesh_Coll_nNodeStackIdx > FMesh_Coll_nDeepestNodeStackIdx )
			{
				FMesh_Coll_nDeepestNodeStackIdx = FMesh_Coll_nNodeStackIdx;
			}
#endif
			continue;
		}
		
#if FMESH_DRAW_COLLIDED_KDOPS
		if ( FMesh_Coll_bShowCollision )
		{
			fmesh_Coll_AddCollDrawDOP( &FMesh_Coll_pCurrkDOPTree->paIntervals[ pNode->nStartkDOPInterval ], FMesh_pModelToWorld, FMesh_Coll_pCurrkDOPTree->nTreekDOPType );
		}
#endif // FMESH_DRAW_COLLIDED_KDOPS
		
		//
		// THIS NODE IS A LEAF SO WE NEED TO TEST FOR COLLISION AGAINST THE TRIS
		//
		
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)((u32)pNode->pTriData + FMATH_BYTE_ALIGN_UP(sizeof(u16) * pNode->nTriCount * 3, 4));
		
		// Uncompress the verts & normals
		u16 nVert, *pVertIdx = (u16 *)pNode->pTriData;
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			FkDOP_TriPacket_t *pPacket = &pNode->paPackets[i];

			// Make sure this packet's mask matches
			if ( !(pPacket->nCollMask & FMesh_Coll_pCurrCollInfo->nCollMask) )
			{
				pNormals += pPacket->nTriCount;
				continue;
			}

			CFVec3 *pVerts = FMesh_Coll_pCurrMeshInst->m_pMesh->pMeshIS->apCollVertBuffer[pPacket->nVBIdx];

			for ( ii = 0; ii < pPacket->nTriCount; ii++ )
			{
#if _TRACK_TESTS			
				FMesh_Coll_nTests++;
#endif
				// Get the normal in preparation for checks
				_vNormal.Set( *pNormals );

				// Note that normals are incremented across all packets.  This is intended behaviour.
				pNormals++;

#if FMESH_DRAW_CONSIDERED_TRIS
				nVert = pPacket->nStartVert + (ii * 3);
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				CFColorRGBA Color( 0.75f, 0.75f, 0.75f, 1.f);
				fmesh_DrawMeshTri( vUncVerts, &_vNormal, &Color, TRUE );
#endif
				nVert = (u16)(pPacket->nStartVert + (ii * 3));
				_avUncVerts[0].v3 = pVerts[ pVertIdx[nVert++] ];
				
				// Does it intersect the plane?
				f32 fFaceDot = pTest->vNormal.Dot( _vNormal );
				BOOL bCenterline = FALSE;
//				BOOL bCenterlineColl = FALSE;
				fImpactDist = FMATH_MAX_FLOAT;
				if ( fmath_Abs( fFaceDot ) < 0.0001f )
				{
					// If the dot is zero, then the face is parallel with the ray
					// We still need to test to see if the cylinder's radius intersects
					_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
					fDist0 = _vODiff.Dot( _vNormal );

					if ( fDist0 > pTest->fRadius || fDist0 < -pTest->fRadius )
					{
						// Plane is beyond radius
						continue;
					}
					
					// Poly is within the radius so project the center of the centerline onto the poly
					vTemp.Mul( _vNormal, fDist0 ).Negate();
					_vImpact.Add( pTest->vCenterPoint, vTemp );
					
					fDist0 = fmath_Abs( fDist0 );
				}
				else
				{
					// The centerline is not parallel with the face, so see if it intersects
					// the plane that contains the triangle
					_vODiff.Sub( pTest->vStart, _avUncVerts[0] );
					fDist0 = _vODiff.Dot( _vNormal );
					_vDiff2.Sub( pTest->vEnd, _avUncVerts[0] );
					fDist1 = _vDiff2.Dot( _vNormal );

					// Calculate the impact point with the plane
					vPlaneImpact.Mul( pTest->vNormal, pTest->fHeight * fmath_Div( fDist0, fDist0 - fDist1 ) ).Add( pTest->vStart );
						
					if ( (fDist0 < 0.f) ^ (fDist1 < 0.f) )
					{
						// The start and the endpoint are on opposite sides of the plane so
						// we know that the centerline intersects the plane (It still might
						// not intersect the triangle)
						fImpactDist = fmath_Abs( fDist0 );
						
						// Set the reference to the plane impact
						vReference.Set( vPlaneImpact );
						
						// Flag this as a centerline intersection
						bCenterline = TRUE;
					}
					else if ( fmath_Abs( fDist0 ) < fmath_Abs( fDist1 ) )
					{
						if ( fmath_Abs( fDist0 ) - pTest->fRadius > 0 )
						{
							// If the intersection is beyond the radius, we know
							// the triangle cannot intersect the cylinder
							continue;
						}
						vReference.Set( pTest->vStart ); 
					}
					else 
					{
						if ( fmath_Abs( fDist1 ) - pTest->fRadius > 0 )
						{
							// If the intersection is beyond the radius, we know
							// the triangle cannot intersect the cylinder
							continue;
						}
						vReference.Set( pTest->vEnd ); 
					}
				}
				
#if _TRACK_TESTS			
				FMesh_Coll_nTestsLevel1++;
#endif

				// Bring the triangle into cylinder space
				_avUncVerts[1].v3 = pVerts[ pVertIdx[nVert++] ];
				_avUncVerts[2].v3 = pVerts[ pVertIdx[nVert] ];
				pTest->pMeshToCylinder->MulPoint( vTransPoints[0], _avUncVerts[0] );
				pTest->pMeshToCylinder->MulPoint( vTransPoints[1], _avUncVerts[1] );
				pTest->pMeshToCylinder->MulPoint( vTransPoints[2], _avUncVerts[2] );
				
				// After this point, we need to use the original cylinder parameters:

				// Check cylinder extents
				if ( vTransPoints[0].y < 0.f && vTransPoints[1].y < 0.f && vTransPoints[2].y < 0.f )
				{
					continue;
				}

				if ( vTransPoints[0].y > pCyl->fHeight && vTransPoints[1].y > pCyl->fHeight && vTransPoints[2].y > pCyl->fHeight )
				{
					continue;
				}

				if ( vTransPoints[0].x < -pCyl->fRadius && vTransPoints[1].x < -pCyl->fRadius && vTransPoints[2].x < -pCyl->fRadius )
				{
					continue;
				}

				if ( vTransPoints[0].x > pCyl->fRadius && vTransPoints[1].x > pCyl->fRadius && vTransPoints[2].x > pCyl->fRadius )
				{
					continue;
				}

				if ( vTransPoints[0].z < -pCyl->fRadius && vTransPoints[1].z < -pCyl->fRadius && vTransPoints[2].z < -pCyl->fRadius )
				{
					continue;
				}

				if ( vTransPoints[0].z > pCyl->fRadius && vTransPoints[1].z > pCyl->fRadius && vTransPoints[2].z > pCyl->fRadius )
				{
					continue;
				}

#if _TRACK_TESTS
				FMesh_Coll_nTestsLevel2++;
#endif

				// Clip the triangle to the end planes (this will result in a poly with up to 5-sides)
				iLastVert = 2;
				iGoodVerts = 0;
				for ( iVert = 0; iVert < 3; iLastVert = iVert, iVert++ )
				{
					if ( vTransPoints[iLastVert].y >= 0.f )
					{
						if ( vTransPoints[iLastVert].y <= pCyl->fHeight )
						{
							vClippedPoints[iGoodVerts].Set( vTransPoints[iLastVert] );
							iGoodVerts++;
							
							if ( vTransPoints[iVert].y > pCyl->fHeight )
							{
								fDist0 = fmath_Div( pCyl->fHeight - vTransPoints[iLastVert].y, vTransPoints[iVert].y - vTransPoints[iLastVert].y );
								_vDiff1.Mul( vTransPoints[iLastVert], 1.f - fDist0 );
								vClippedPoints[iGoodVerts].Mul( vTransPoints[iVert], fDist0 ).Add( _vDiff1 );
								vClippedPoints[iGoodVerts].y = pCyl->fHeight;
								iGoodVerts++;
							}
						}
						else if ( vTransPoints[iVert].y < pCyl->fHeight )
						{
							fDist0 = fmath_Div( pCyl->fHeight - vTransPoints[iLastVert].y, vTransPoints[iVert].y - vTransPoints[iLastVert].y );
							_vDiff1.Mul( vTransPoints[iLastVert], 1.f - fDist0 );
							vClippedPoints[iGoodVerts].Mul( vTransPoints[iVert], fDist0 ).Add( _vDiff1 );
							vClippedPoints[iGoodVerts].y = pCyl->fHeight;
							iGoodVerts++;
						}
							
						if ( vTransPoints[iVert].y < 0.f )
						{
							fDist0 = fmath_Div( vTransPoints[iLastVert].y, vTransPoints[iLastVert].y - vTransPoints[iVert].y );
							_vDiff1.Mul( vTransPoints[iLastVert], 1.f - fDist0 );
							vClippedPoints[iGoodVerts].Mul( vTransPoints[iVert], fDist0 ).Add( _vDiff1 );
							vClippedPoints[iGoodVerts].y = 0.f;
							iGoodVerts++;
						}
					}
					else if ( vTransPoints[iVert].y >= 0.f )
					{
						fDist0 = fmath_Div( vTransPoints[iLastVert].y, vTransPoints[iLastVert].y - vTransPoints[iVert].y );
						_vDiff1.Mul( vTransPoints[iLastVert], 1.f - fDist0 );
						vClippedPoints[iGoodVerts].Mul( vTransPoints[iVert], fDist0 ).Add( _vDiff1 );
						vClippedPoints[iGoodVerts].y = 0.f;
						iGoodVerts++;
						
						if ( vTransPoints[iVert].y > pCyl->fHeight )
						{
							fDist0 = fmath_Div( pCyl->fHeight - vTransPoints[iLastVert].y, vTransPoints[iVert].y - vTransPoints[iLastVert].y );
							_vDiff1.Mul( vTransPoints[iLastVert], 1.f - fDist0 );
							vClippedPoints[iGoodVerts].Mul( vTransPoints[iVert], fDist0 ).Add( _vDiff1 );
							vClippedPoints[iGoodVerts].y = pCyl->fHeight;
							iGoodVerts++;
						}
					}
				}

				// Make sure we have at least three and no more than 5 edges
				FASSERT( iGoodVerts > 2 && iGoodVerts < 6 );

				// Check the resulting edges for intersection with the cylinder
				vRadialImpact.y = FMATH_MAX_FLOAT;
				vHeightImpact.y = -FMATH_MAX_FLOAT;
				iLastVert = iGoodVerts - 1;
				BOOL bOutsideLine;
				for ( iVert = 0; iVert < iGoodVerts; iLastVert = iVert, iVert++ )
				{
					_vDiff1.ReceiveNegative( vClippedPoints[iLastVert] );
					_vDiff2.Sub( vClippedPoints[iVert], vClippedPoints[iLastVert] );
					
					// Get 2D distance to use to check for intersection with the cylinder
					fDist0 = (_vDiff2.x * _vDiff2.x) + (_vDiff2.z * _vDiff2.z);
					fDist2 = (_vDiff1.x * _vDiff2.x) + (_vDiff1.z * _vDiff2.z);
					
					if ( fDist0 < 0.0001f )
					{
						// Points are vertical, so we need to process specially
						fDist1 = (_vDiff1.x * _vDiff1.x) + (_vDiff1.z * _vDiff1.z);
						if ( fDist1 <= pCyl->fRadiusSq )
						{
							// Points are within radius so test against high/low
							if ( vClippedPoints[iLastVert].y < vRadialImpact.y )
							{
								vRadialImpact.Set( vClippedPoints[iLastVert] );
							}
							if  ( vClippedPoints[iLastVert].y > vHeightImpact.y )
							{
								vHeightImpact.Set( vClippedPoints[iLastVert] );
							}
							
							if ( vClippedPoints[iVert].y < vRadialImpact.y )
							{
								vRadialImpact.Set( vClippedPoints[iVert] );
							}
							if  ( vClippedPoints[iLastVert].y > vHeightImpact.y )
							{
								vHeightImpact.Set( vClippedPoints[iVert] );
							}
						}
						
						continue;
					}
					
					fDist2 = fmath_Div( fDist2, fDist0 );

					bOutsideLine = TRUE;
					if ( fDist2 > 0.f )
					{
						if ( fDist2 < 1.f )
						{
							// Closest point is between the edge endpoints so use the 
							// ratio of the full distance to establish the close point
							_vDiff1.Mul( _vDiff2, fDist2 ).Add( vClippedPoints[iLastVert] );
						
							fDist1 = (_vDiff1.x * _vDiff1.x) + (_vDiff1.z * _vDiff1.z);
							
							// Our close point is on the line, so flag it as such
							bOutsideLine = FALSE;
						}
						else
						{
							// Closest point on the line is after the end of the segment
							fDist1 = (vClippedPoints[iVert].x * vClippedPoints[iVert].x) + (vClippedPoints[iVert].z * vClippedPoints[iVert].z);
						}
					}
					else
					{
						// Closest point on the line is before the start of the segment
						fDist1 = (vClippedPoints[iLastVert].x * vClippedPoints[iLastVert].x) + (vClippedPoints[iLastVert].z * vClippedPoints[iLastVert].z);
					}
					
					// Test radius
					if ( fDist1 > pCyl->fRadiusSq )
					{
						// Closest point is beyond radius so edge does not intersect
						continue;
					}
					
					// This edge intersects the cylinder, so determine the relevant
					// collision info from the segment
					
					if ( bOutsideLine )
					{
						// Calculate closest point on line, ignoring the segment boundaries
						// (This was already done, above, in situations where the closest
						// point is on the line)
						_vDiff1.Mul( _vDiff2, fDist2 ).Add( vClippedPoints[iLastVert] );
					}

					// Calculate the portion of the segment that is inside cylinder
					// and check against the current radial and height impacts
					if ( vClippedPoints[iVert].y != vClippedPoints[iLastVert].y || bOutsideLine )
					{
						f32 fHalfDistRatio = fmath_Div( fmath_Sqrt( pCyl->fRadiusSq - fDist1 ), fmath_Sqrt( fDist0 ) );
						fDist0 = fDist2 + fHalfDistRatio;
						if ( fDist0 < 0.f )
						{
							fDist0 = 0.f;
						}
						else if ( fDist0 > 1.f )
						{
							fDist0 = 1.f;
						}
						vTemp.Mul( _vDiff2, fDist0 ).Add( vClippedPoints[iLastVert] );
						if ( vTemp.y < vRadialImpact.y && vTemp.y >= 0.f )
						{
							vRadialImpact.Set( vTemp );
						}
						if  ( vTemp.y > vHeightImpact.y && vTemp.y <= pCyl->fHeight )
						{
							vHeightImpact.Set( vTemp );
						}
						
						fDist0 = fDist2 - fHalfDistRatio;
						if ( fDist0 < 0.f )
						{
							fDist0 = 0.f;
						}
						else if ( fDist0 > 1.f )
						{
							fDist0 = 1.f;
						}
						vTemp.Mul( _vDiff2, fDist0 ).Add( vClippedPoints[iLastVert] );
					}
					else
					{
						vTemp.Set( _vDiff1 );
					}
					
					if ( vTemp.y < vRadialImpact.y && vTemp.y >= 0.f )
					{
						vRadialImpact.Set( vTemp );
					}
					if  ( vTemp.y > vHeightImpact.y && vTemp.y <= pCyl->fHeight )
					{
						vHeightImpact.Set( vTemp );
					}
				}


				if ( !bCenterline )
				{
					if ( vRadialImpact.y == FMATH_MAX_FLOAT )
					{
						// No edge came within the cylinder and the centerline did not
						// intersect the plane, so there can be no collision
						
						continue;
					}
				}
				else
				{
					// See if the centerline intersection is actually inside the triangle
					_vDiff1.Sub( _avUncVerts[1], _avUncVerts[0] );
					_vDiff2.Sub( _avUncVerts[2], _avUncVerts[0] );
					_vODiff.Sub( vReference, _avUncVerts[0] );
					if ( fcoll_PointIsInTri( _vODiff, _vDiff1, _vDiff2, _vNormal ) )
					{
						_vImpact.Set( vReference );
					}
					else
					{
						if ( vRadialImpact.y == FMATH_MAX_FLOAT )
						{
							// No edge came within the cylinder and the centerline did not
							// intersect the plane, so there can be no collision
							
							continue;
						}
						
						fImpactDist = FMATH_MAX_FLOAT;
					}
				}


				// WE'VE GOT COLLISION!!!
				// Let's make sure that the return data is accurate:


				// Now we have the lowest and highest edge collisions, but the highest 
				// and lowest may actually not reside on an edge.  We need to check
				// for that condition:
				
				if ( fmath_Abs( fFaceDot ) > 0.001f )
				{
					pTest->pMeshToCylinderNoScale->MulDir( vTransNorm, _vNormal );
					_vDiff1.Cross( CFVec3A::m_UnitAxisY, vTransNorm );
					vTransDown.Cross( _vDiff1, vTransNorm );
					fDist1 = vTransDown.MagXZ();
					
					if ( fDist1 < 0.001f )
					{
						vTransNorm.Set( 0.f, fFaceDot, 0.f );
						vTransDown.Set( CFVec3A::m_UnitAxisZ );
						fDist0 = pCyl->fRadius;
					}
					else
					{
						fDist0 = pCyl->fRadius * fmath_Inv( fDist1 );
					}

					pTest->pMeshToCylinder->MulPoint( vTransRef, vPlaneImpact );

					_vDiff1.Sub( vTransPoints[1], vTransPoints[0] );
					_vDiff2.Sub( vTransPoints[2], vTransPoints[0] );

					vTemp.Mul( vTransDown, fDist0 ).Add( vTransRef );
					if ( vTemp.y < vRadialImpact.y && vTemp.y > 0.001f )
					{
						_vODiff.Sub( vTemp, vTransPoints[0] );
						if ( fcoll_PointIsInTri( _vODiff, _vDiff1, _vDiff2, vTransNorm ) )
						{
							vRadialImpact.Set( vTemp );
						}
					}
					
					vTemp.Mul( vTransDown, -fDist0 ).Add( vTransRef );
					if  ( vTemp.y > vHeightImpact.y && vTemp.y <= pCyl->fHeight )
					{
						_vODiff.Sub( vTemp, vTransPoints[0] );
						if ( fcoll_PointIsInTri( _vODiff, _vDiff1, _vDiff2, vTransNorm ) )
						{
							vHeightImpact.Set( vTemp );
						}
					}
				}
				else
				{
					// We still need to transform the normal for below
					pTest->pMeshToCylinderNoScale->MulDir( vTransNorm, _vNormal );
				}
				
				// If the poly is facing away from the cylinder normal, we
				// need to swap the radial and height impacts
				if ( fFaceDot > 0.f )
				{
					vTemp.Set( vRadialImpact );
					vRadialImpact.Set( vHeightImpact );
					vHeightImpact.Set( vTemp );
				}

#if _TRACK_TESTS
				FMesh_Coll_nHits++;
#endif
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
				
				// Setup the Impact in the impact buffer

				// Fill in all of the distance info
				FColl_pNextImpactSlot->fHeightDistInfo = vHeightImpact.y;
				FColl_pNextImpactSlot->fRadialDistInfo = fmath_Sqrt( (vRadialImpact.x * vRadialImpact.x) + (vRadialImpact.y * vRadialImpact.y) );
				FColl_pNextImpactSlot->fImpactDistInfo = fImpactDist;
				FASSERT( FColl_pNextImpactSlot->fHeightDistInfo >= 0.0f );
				FASSERT( FColl_pNextImpactSlot->fRadialDistInfo >= 0.0f );
				FASSERT( FColl_pNextImpactSlot->fImpactDistInfo >= 0.0f );
				
				// Bring the radial and height impacts into world space and fill in impact
				pTest->pCylinderToWorld->MulPoint( FColl_pNextImpactSlot->vRadialImpact, vRadialImpact );
				pTest->pCylinderToWorld->MulPoint( FColl_pNextImpactSlot->vHeightImpact, vHeightImpact );
				FColl_pNextImpactSlot->ImpactPoint.Set( _vImpact );

				// Fill in vertex information
				if ( pTest->bWorldGeo )
				{
					// World geo will not need any transforming
					FColl_pNextImpactSlot->aTriVtx[0].Set( _avUncVerts[0] );
					FColl_pNextImpactSlot->aTriVtx[1].Set( _avUncVerts[1] );
					FColl_pNextImpactSlot->aTriVtx[2].Set( _avUncVerts[2] );
					FColl_pNextImpactSlot->UnitFaceNormal.Set( _vNormal );
				}
				else
				{
					pTest->pCylinderToWorld->MulPoint( FColl_pNextImpactSlot->aTriVtx[0], vTransPoints[0] );
					pTest->pCylinderToWorld->MulPoint( FColl_pNextImpactSlot->aTriVtx[1], vTransPoints[1] );
					pTest->pCylinderToWorld->MulPoint( FColl_pNextImpactSlot->aTriVtx[2], vTransPoints[2] );
					pTest->pCylinderToWorld->MulDir( FColl_pNextImpactSlot->UnitFaceNormal, vTransNorm );
				}
				
				FColl_pNextImpactSlot->nUserType = pPacket->nCollType;
				FColl_pNextImpactSlot->nBoneIndex = FMesh_Coll_nCurrBoneIndex;
				FColl_pNextImpactSlot->pTag = FMesh_Coll_pCurrCollInfo->pTag;
				
				Fmesh_Coll_nAccumCollMask |= pPacket->nCollMask;
				
				FColl_pNextImpactSlot++;
				FColl_nImpactCount++;
			}
		}
	}
}
*/

