//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_Coll.cpp - Classes used to create collision data for Fang GC
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
// 05/06/02 Lafleur		Created.
// 08/15/02 Lafleur		Adapted from GCMeshColl to be multi-platform capable.
//////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include <mmsystem.h>

#include "Fang.h"

#include "..\PasmDlg.h"

#include "MLMesh.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _CALCULATE_ALL_KDOPS	FALSE

#define _MAX_COLL_TRIS		24288
#define _MAX_COLL_NODES		16384
#define _MAX_COLL_INTERVALS	65536
#define	_MAX_TRI_PACKETS	128
#define _MAX_KDOP_FACES		16384


//////////////////////////////////////////////////////////////////////////////////////
// Local classes and structures:
//////////////////////////////////////////////////////////////////////////////////////

struct _PacketInfo
{
	u16 nTriCount;
	u16	nCollMask;
	u16	nCollType;
	u8	nVBIdx;
	u8	_nPad;
};

//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

u32			_nNodeCount;
CollNode	*_paNodePool = NULL;

u32			_nTriCount;
CollTri		*_paTriPool = NULL;

u32			_nIntervalCount;
FkDOP_Interval_t  *_paIntervalPool = NULL;

#if MLMESH_PROFILE_TREE
static f32 _fTotalVolume = 0;
static u32 _nTotalChildren = 0;
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static void DumpNodeData( FILE *pFile, u32 nSegment, u32 nParentIdx, u32 nNodeIdx );
static f32 DetermineTotalChildVolume( CollNode *pNode, u32 nCutAxis, f32 fCutPoint );
static u32 SetCollMask( FkDOP_Node_t *pRootNode, FkDOP_Node_t *pNode );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
u32 MLMesh::CreateMeshCollData( u32 nkDOPType )
{
#if MLMESH_PROFILE_TREE
	_fTotalVolume = 0.f;
	_nTotalChildren = 0;
#endif
	u32 nErrorCode = ML_NO_ERROR;
	u32 nTimeSettingUpData = 0;
	u32 nTimeCompressingData = 0;
	u32 nTimeFormattingData = 0;
	u32 nOpStartTime = 0;
	FMesh_Coll_Capsule_t SegmentCapsule;

	// Allocate some memory for the maximum possible Coll info structures
	m_paCollTree = (FkDOP_Tree_t *)malloc( sizeof( FkDOP_Tree_t ) * m_FMesh.nSegCount );
	if ( !m_paCollTree )
	{
		return ML_OUT_OF_MEMORY;
	}
	memset( m_paCollTree, 0, sizeof( FkDOP_Tree_t ) * m_FMesh.nSegCount );

	u32 i, ii, nCollCount = 0;

	// Allocate memory to use for calculating collision info
	_paNodePool = (CollNode *)malloc( sizeof(CollNode) * _MAX_COLL_NODES );
	memset( _paNodePool, 0, sizeof(CollNode) *  _MAX_COLL_NODES );
	_paTriPool = (CollTri *)malloc( sizeof(CollTri) * _MAX_COLL_TRIS );
	memset( _paNodePool, 0, sizeof(CollNode) *  _MAX_COLL_NODES );
	_paIntervalPool = (FkDOP_Interval_t *)malloc( sizeof(FkDOP_Interval_t) *_MAX_COLL_INTERVALS);
	memset( _paNodePool, 0, sizeof(CollNode) *  _MAX_COLL_NODES );
	if ( !_paNodePool || !_paTriPool || !_paIntervalPool )
	{

		nErrorCode = ML_OUT_OF_MEMORY;
		goto SKIP_COLLISION_GENERATION;
	}

#ifdef _MMI_TARGET_PS2
//NS PS2 LOD Removal
  if(m_FMesh.nLODCount > 1 && m_nCollisionLOD) m_nCollisionLOD--; //NS If we are discarding top LOD adjust CollisionLOD if anything other than 0
#endif

	MLSegment *pTempSegment = m_pFirstSegment;
	while ( pTempSegment )
	{
		_nIntervalCount = _nTriCount = _nNodeCount = 0;

		////////////////////////////////////////////////////////
		//	Step 1:  If not skinned, generate the collision data
		////////////////////////////////////////////////////////

		if ( pTempSegment->m_nMtxWeightCount < 2 )
		{
			nOpStartTime = timeGetTime();

			CollTri *pLastTri = NULL;

			MLTriContainer *pVC = pTempSegment->m_pFirstTriContainer;

			while ( pVC )
			{
#ifdef _MMI_TARGET_PS2
//NS PS2 LOD Removal
                if(m_FMesh.nLODCount > 1) pVC->m_nLODID--;  // NS On PS2 we are throwing away the top level LOD
#endif
				// If these tris are from a material that has no collision, then
				// do not generate collision data for them.
				if ( (pVC->m_pMaterial->GetFlags() & FMESH_MTLFLAG_NOCOLLIDE)
					|| (pVC->m_nCollisionMask == FCOLL_MASK_NONE) )
				{
					pVC = pVC->m_pNext;
					continue;
				}

				// Make sure this tri container contains the appropriate LOD
				if ( pVC->m_nLODID != m_nCollisionLOD )
				{
					pVC = pVC->m_pNext;
					continue;
				}

				// Gather the tri info
//				u32 iIncrement = 3;
//				s32 nNormSwap = 1;
//				s32 nNormDirection = -1;
//				if ( pVC->m_bStrip )
//				{
					// If we have a strip, we need to negate every other normal and
					// increment only one vert at a time:
//					nNormSwap = -1;
//					iIncrement = 1;
//				}

				if ( pVC->m_nVertCount < 3 )
				{
					pVC = pVC->m_pNext;
					continue;
				}

				CFVec3 *pVert0, *pVert1, *pVert2;
				for ( i = 0; i < (u32)(pVC->m_nVertCount - 2); i += 3 )
				{
					// Don't exceed the scratch space
					if ( _nTriCount == _MAX_COLL_TRIS )
					{
						FASSERT_NOW;
						break;
					}

					// If this tri got compressed to use two of the same point, don't
					// use it for collision.
					if ( !Next3VertsAreUnique( pVC, i ) )
					{
						continue;
					}

#if 0
					if ( nNormDirection == 1 )
					{
						// Record the tri position indices into the VB's
						_paTriPool[_nTriCount].anPosIdx[0] = GetAbstrPosIdx( pVC, i+0 );
						_paTriPool[_nTriCount].anPosIdx[1] = GetAbstrPosIdx( pVC, i+1 );
						_paTriPool[_nTriCount].anPosIdx[2] = GetAbstrPosIdx( pVC, i+2 );

						// Get the triangle's vertex positions
						pVert0 = &pVC->m_paKongVerts[i+0].Pos;
						pVert1 = &pVC->m_paKongVerts[i+1].Pos;
						pVert2 = &pVC->m_paKongVerts[i+2].Pos;
					}
					else
#endif
					{
						// Record the tri position indices into the VB's
						_paTriPool[_nTriCount].anPosIdx[0] = GetAbstrPosIdx( pVC, i+0 );
						_paTriPool[_nTriCount].anPosIdx[2] = GetAbstrPosIdx( pVC, i+1 );
						_paTriPool[_nTriCount].anPosIdx[1] = GetAbstrPosIdx( pVC, i+2 );

						// Get the triangle's vertex positions
						pVert0 = &pVC->m_paKongVerts[i+0].Pos;
						pVert2 = &pVC->m_paKongVerts[i+1].Pos;
						pVert1 = &pVC->m_paKongVerts[i+2].Pos;
					}

					_paTriPool[_nTriCount].avPos[0] = *pVert0;
					_paTriPool[_nTriCount].avPos[1] = *pVert1;
					_paTriPool[_nTriCount].avPos[2] = *pVert2;

					// Calculate the centroid
					_paTriPool[_nTriCount].vCentroid = (*pVert0 + *pVert1 + *pVert2) * 0.33333333f;

					// Calculate the face normal
					CFVec3 vDiff1 = *pVert0 - *pVert1;
					CFVec3 vDiff2 = *pVert0 - *pVert2;
					_paTriPool[_nTriCount].vFaceNormal = vDiff1.Cross( vDiff2 );

					f32 fMag = _paTriPool[_nTriCount].vFaceNormal.Mag2();

					// Verify that we have some poly face area
					if ( fMag < 0.00001f )
					{
						continue;
					}

					// Unitize the normal
					fMag = (f32)sqrt( fMag );
					_paTriPool[_nTriCount].vFaceNormal *= 1.f / fMag;

#if _DEBUG
					// Verify that we have a valid normal
					f32 fNormalMag = _paTriPool[_nTriCount].vFaceNormal.Mag();
					if ( fNormalMag < 0.99f || fNormalMag > 1.01f )
					{
						FASSERT_NOW;
						continue;
					}
#endif
					// Record the collision info
					_paTriPool[_nTriCount].nCollMask = pVC->m_nCollisionMask;
					_paTriPool[_nTriCount].nCollType = pVC->m_nCollisionType;

					// Record the vertex buffer index
					FASSERT( pVC->m_nVBIndex < 256 );
					_paTriPool[_nTriCount].nVBIdx = (u16)pVC->m_nVBIndex;

					_paTriPool[_nTriCount].pNext = NULL;
					if ( pLastTri )
					{
						// Initially, all of the verts are linked together
						pLastTri->pNext = &_paTriPool[_nTriCount];
					}

					_paTriPool[_nTriCount].nTriPacket = -1;

					pLastTri = &_paTriPool[_nTriCount];
//					nNormDirection *= nNormSwap;
					_nTriCount++;
				}

				pVC = pVC->m_pNext;
			}

			nTimeSettingUpData += timeGetTime() - nOpStartTime;

			if ( _nTriCount != 0 )
			{
				nOpStartTime = timeGetTime();

				// Setup the first node
				memset( &_paNodePool[0], 0, sizeof( CollNode ) );
				_paNodePool[0].pTriList = &_paTriPool[0];
				_paNodePool[0].nTriCount = (u16)_nTriCount;
				_paNodePool[0].nTreeDepth = 0;
#if MLMESH_PROFILE_TREE
				_paNodePool[0].fPercentOfParent = -1;
#endif
				FASSERT( _nNodeCount == 0 );
				_nNodeCount++;

				// Generate the bounding capsule
				GenerateBoundingCapsule( &_paNodePool[0], &SegmentCapsule );

				// Start the generation of the node pool (This will become a recursive
				// call to separate the polys into the different nodes).
				GenerateNode( &_paNodePool[0], nkDOPType );

				nTimeCompressingData += timeGetTime() - nOpStartTime;
			}
		}

		/////////////////////////////////////////////////////////////////////
		//	Step 2:  If there is collision data, fill out the mesh structures
		/////////////////////////////////////////////////////////////////////


		if ( !_nNodeCount )
		{
			// There were no collision nodes for this segment, so it has no collision
		}
		else
		{
			nOpStartTime = timeGetTime();

			// Record the number of nodes for this segment
			m_paCollTree[nCollCount].nRootkDOPType = nkDOPType;
			m_paCollTree[nCollCount].nTreekDOPType = nkDOPType;
			m_paCollTree[nCollCount].nTreeNodeCount	= (u16)_nNodeCount;
			m_paCollTree[nCollCount].nSegmentIdx = (u8)pTempSegment->m_nSegmentIdx;
			m_paCollTree[nCollCount].nMasterCollMask = 0;
			memcpy( &m_paCollTree[nCollCount].BoundingCapsule, &SegmentCapsule, sizeof(FMesh_Coll_Capsule_t) );

			if ( pTempSegment->m_nPosType == GX_S8 )
			{
				m_paCollTree[nCollCount].nFlags |= FMESH_KDOP_8BIT_VERTS;
			}
			else if  ( pTempSegment->m_nPosType == GX_S16 )
			{
				m_paCollTree[nCollCount].nFlags |= FMESH_KDOP_16BIT_VERTS;
			}
			m_paCollTree[nCollCount].nFracBits = (u8)pTempSegment->m_nPosFrac;

			// We've got some nodes, so copy over the kDOP tree data
			MLMesh_nTotalNodeCount += _nNodeCount;
			MLMesh_nTotalIntervalCount += _nIntervalCount;
			MLMesh_nTotalTriCount += _nTriCount;

			// Allocate memory for and copy over the intervals
			m_paCollTree[nCollCount].paIntervals = (FkDOP_Interval_t *)malloc( sizeof( FkDOP_Interval_t ) * _nIntervalCount );
			if ( !m_paCollTree[nCollCount].paIntervals )
			{
				nErrorCode = ML_OUT_OF_MEMORY;
				goto SKIP_COLLISION_GENERATION;
			}

			memcpy( m_paCollTree[nCollCount].paIntervals, _paIntervalPool, sizeof( FkDOP_Interval_t ) * _nIntervalCount );

//			m_paCollTree[nCollCount].nkDOPVertCount = 0;
//			m_paCollTree[nCollCount].pakDOPVerts = NULL;
			if ( !(m_FMesh.nFlags & FMESH_FLAGS_VOLUME_MESH) )
			{
#if !_CALCULATE_ALL_KDOPS
				// Determine verts for bounding planes for the root node
				u32 nTotalVerts = 0;
				fkdop_CalculateVerts( &_paIntervalPool[ _paNodePool[0].nStartkDOPInterval], TRUE, nkDOPType );
				memcpy( _paNodePool[0].vDOPVerts, FConvHull_vPointBuffer, sizeof( CFVec3A ) * FConvHull_nPointBufferCount );
				_paNodePool[0].nDOPVertCount = (u16)FConvHull_nPointBufferCount;
				nTotalVerts += _paNodePool[0].nDOPVertCount;

				m_paCollTree[nCollCount].nRootkDOPVertCount = (u8)_paNodePool[0].nDOPVertCount;

				// Allocate memory for and copy over the root kDOP verts
				u32 nRootVertDataSize = sizeof( CFVec3 ) * _paNodePool[0].nDOPVertCount;
				MLMesh_nTotalRootDOPVertBytes += nRootVertDataSize;
				m_paCollTree[nCollCount].paRootkDOPVerts = (CFVec3 *)malloc( nRootVertDataSize );
				for ( i = 0; i < _paNodePool[0].nDOPVertCount; i++ )
				{
					m_paCollTree[nCollCount].paRootkDOPVerts[i] = _paNodePool[0].vDOPVerts[i].v3;
				}

				// Calculate the bounding sphere

				// What are the points furthest from one another?
				f32 fFurthest = -FMATH_MAX_FLOAT, fTest;
				CFVec3 *pPoint1, *pPoint2;
				for ( i = 0; (s32)i < _paNodePool[0].nDOPVertCount-1; i++ )
				{
					for ( ii = i+1; ii < _paNodePool[0].nDOPVertCount; ii++ )
					{
						fTest = (_paNodePool[0].vDOPVerts[i].v3 - _paNodePool[0].vDOPVerts[ii].v3).Mag2();
						if ( fTest > fFurthest )
						{
							fFurthest = fTest;
							pPoint1 = &_paNodePool[0].vDOPVerts[i].v3;
							pPoint2 = &_paNodePool[0].vDOPVerts[ii].v3;
						}
					}
				}
				// The average of the two points furthest away will be the center
				m_paCollTree[nCollCount].spRootkDOPBounding.m_Pos = (*pPoint1 + *pPoint2) * 0.5f;

				// Calculate the radius
				m_paCollTree[nCollCount].spRootkDOPBounding.m_fRadius = -FMATH_MAX_FLOAT;
				for ( i = 0; i < _paNodePool[0].nDOPVertCount; i++ )
				{
					fTest = (m_paCollTree[nCollCount].spRootkDOPBounding.m_Pos - _paNodePool[0].vDOPVerts[i].v3).Mag2();
					if ( m_paCollTree[nCollCount].spRootkDOPBounding.m_fRadius < fTest )
					{
						m_paCollTree[nCollCount].spRootkDOPBounding.m_fRadius = fTest;
					}
				}
				m_paCollTree[nCollCount].spRootkDOPBounding.m_fRadius = fmath_AcuSqrt( m_paCollTree[nCollCount].spRootkDOPBounding.m_fRadius );

#else // _CALCULATE_ALL_KDOPS
				m_paCollTree[nCollCount].nkDOPVertCount = 0;
				m_paCollTree[nCollCount].pakDOPVerts = NULL;

				// Determine verts for bounding planes for all nodes
				u32 nTotalVerts = 0;
				for ( i = 0; i < _nNodeCount; i++ )
				{
					fkdop_CalculateVerts( &_paIntervalPool[ _paNodePool[i].nStartkDOPInterval], FALSE, nkDOPType );
					memcpy( _paNodePool[i].vDOPVerts, FConvHull_vPointBuffer, sizeof( CFVec3A ) * FConvHull_nPointBufferCount );
					_paNodePool[i].nDOPVertCount = (u16)FConvHull_nPointBufferCount;
					nTotalVerts += _paNodePool[i].nDOPVertCount;
				}

//				nTotalVerts -= _paNodePool[0].nDOPVertCount;
				m_paCollTree[nCollCount].nkDOPVertCount = (u16)nTotalVerts;
				m_paCollTree[nCollCount].nRootkDOPVertCount = (u8)_paNodePool[0].nDOPVertCount;

				// Allocate memory for and copy over the root kDOP verts
				u32 nRootVertDataSize = sizeof( CFVec3 ) * _paNodePool[0].nDOPVertCount;
				MLMesh_nTotalRootDOPVertBytes += nRootVertDataSize;
				m_paCollTree[nCollCount].paRootkDOPVerts = (CFVec3 *)malloc( nRootVertDataSize );
				for ( i = 0; i < _paNodePool[0].nDOPVertCount; i++ )
				{
					m_paCollTree[nCollCount].paRootkDOPVerts[i] = _paNodePool[0].vDOPVerts[i].v3;
				}

				// Allocate memory for and copy over the kDOP verts
				if ( m_paCollTree[nCollCount].nFlags & FMESH_KDOP_8BIT_VERTS )
				{
					u32 nVertDataSize = sizeof( FkDOP_Vert8_t ) * nTotalVerts;
					MLMesh_nTotalDOPVertBytes += nVertDataSize;
					m_paCollTree[nCollCount].pakDOPVerts = (FkDOP_Vert8_t *)malloc( nVertDataSize );
					s8 x, y, z;
					u32 nVertIdx = 0;
					for ( i = 0; i < _nNodeCount; i++ )
					{
						for ( ii = 0; ii < _paNodePool[i].nDOPVertCount; ii++ )
						{
							x = (s8)(_paNodePool[i].vDOPVerts[ii].x * (1 << m_paCollTree[nCollCount].nFracBits));
							y = (s8)(_paNodePool[i].vDOPVerts[ii].y * (1 << m_paCollTree[nCollCount].nFracBits));
							z = (s8)(_paNodePool[i].vDOPVerts[ii].z * (1 << m_paCollTree[nCollCount].nFracBits));
							((FkDOP_Vert8_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].x = x;
							((FkDOP_Vert8_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].y = y;
							((FkDOP_Vert8_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].z = z;
							nVertIdx++;
						}
					}
				}
				else if ( m_paCollTree[nCollCount].nFlags & FMESH_KDOP_16BIT_VERTS )
				{
					u32 nVertDataSize = sizeof( FkDOP_Vert16_t ) * nTotalVerts;
					MLMesh_nTotalDOPVertBytes += nVertDataSize;
					m_paCollTree[nCollCount].pakDOPVerts = (FkDOP_Vert16_t *)malloc( nVertDataSize );
					s16 x, y, z;
					u32 nVertIdx = 0;
					for ( i = 0; i < _nNodeCount; i++ )
					{
						for ( ii = 0; ii < _paNodePool[i].nDOPVertCount; ii++ )
						{
							x = (s16)(_paNodePool[i].vDOPVerts[ii].x * (1 << m_paCollTree[nCollCount].nFracBits));
							y = (s16)(_paNodePool[i].vDOPVerts[ii].y * (1 << m_paCollTree[nCollCount].nFracBits));
							z = (s16)(_paNodePool[i].vDOPVerts[ii].z * (1 << m_paCollTree[nCollCount].nFracBits));
							((FkDOP_Vert16_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].x = x;
							((FkDOP_Vert16_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].y = y;
							((FkDOP_Vert16_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].z = z;
							nVertIdx++;
						}
					}
				}
				else
				{
					u32 nVertDataSize = sizeof( FkDOP_Vert32_t ) * nTotalVerts;
					MLMesh_nTotalDOPVertBytes += nVertDataSize;
					m_paCollTree[nCollCount].pakDOPVerts = (FkDOP_Vert32_t *)malloc( nVertDataSize );
					u32 nVertIdx = 0;
					for ( i = 0; i < _nNodeCount; i++ )
					{
						for ( ii = 0; ii < _paNodePool[i].nDOPVertCount; ii++ )
						{
							((FkDOP_Vert32_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].x = _paNodePool[i].vDOPVerts[ii].x;
							((FkDOP_Vert32_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].y = _paNodePool[i].vDOPVerts[ii].y;
							((FkDOP_Vert32_t *)m_paCollTree[nCollCount].pakDOPVerts)[ nVertIdx ].z = _paNodePool[i].vDOPVerts[ii].z;
							nVertIdx++;
						}
					}
				}
#endif // _CALCULATE_ALL_KDOPS

			}

			// Allocate memory for and copy over the nodes
			m_paCollTree[nCollCount].pakDOPNodes = (FkDOP_Node_t *)malloc( sizeof( FkDOP_Node_t ) * _nNodeCount );
			for ( i = 0; i < _nNodeCount; i++ )
			{
				FkDOP_Node_t *pNode = &m_paCollTree[nCollCount].pakDOPNodes[i];

				// Fill in general data
				pNode->nStartkDOPInterval = _paNodePool[i].nStartkDOPInterval;
				pNode->paPackets = NULL;
				pNode->pTriData = NULL;

				// If this node is a leaf, add in the tri data
				if ( _paNodePool[i].nTriCount )
				{
					pNode->nTriCount = (u16)_paNodePool[i].nTriCount;

					_PacketInfo Packets[_MAX_TRI_PACKETS];
					u32 nPacketCount = 0;

					// Separate the tris into packets
					CollTri *pTemp = _paNodePool[i].pTriList;
					while ( pTemp )
					{
						// Check to see if we already have a packet that this tri can go in
						for ( ii = 0; ii < nPacketCount; ii++ )
						{
							if ( pTemp->nVBIdx == Packets[ii].nVBIdx
								&& pTemp->nCollMask == Packets[ii].nCollMask
								&& pTemp->nCollType == Packets[ii].nCollType
								&& Packets[ii].nTriCount < 255 )
							{
								pTemp->nTriPacket = (u16)ii;
								Packets[ii].nTriCount++;
								break;
							}
						}

						// If we didn't find a tri packet for this tri, create a new one
						if ( pTemp->nTriPacket == -1 )
						{
							// Bounds check
							if ( nPacketCount == _MAX_TRI_PACKETS )
							{
								FASSERT_NOW;
								break;
							}

							pTemp->nTriPacket = (u16)nPacketCount;

							Packets[nPacketCount].nVBIdx = (u8)pTemp->nVBIdx;
							Packets[nPacketCount].nCollMask = pTemp->nCollMask;
							Packets[nPacketCount].nCollType = pTemp->nCollType;
							Packets[nPacketCount].nTriCount = 1;
							nPacketCount++;
						}

						// On to the next tri
						pTemp = pTemp->pNext;
					}

					FASSERT( nPacketCount < 256 );
					pNode->nTriPacketCount = (u8)nPacketCount;
					pNode->paPackets = (FkDOP_TriPacket_t *)malloc( sizeof( FkDOP_TriPacket_t ) * nPacketCount );
					if ( !pNode->paPackets )
					{
						nErrorCode = ML_OUT_OF_MEMORY;
						goto SKIP_COLLISION_GENERATION;
					}

					// Allocate memory for and copy over tri data
					u32 nVertDataBytes = RoundUp4B(sizeof( u16 ) * _paNodePool[i].nTriCount * 3);
					u32 nNormDataBytes;
					if ( MLManager.GetPlatformType() == PASM_TARGET_PC )
					{
						nNormDataBytes = RoundUp8B(sizeof( FkDOP_Normal_t ) * _paNodePool[i].nTriCount);
					}
					else if ( MLManager.GetPlatformType() == PASM_TARGET_GC )
					{
						nNormDataBytes = RoundUp8B(sizeof( FkDOP_CNormal_t ) * _paNodePool[i].nTriCount);
					}
#ifdef _MMI_TARGET_PS2
                    else if ( MLManager.GetPlatformType() == PASM_TARGET_PS2 )     // KJ Begin
					{
						nNormDataBytes = RoundUp8B(sizeof( FkDOP_Normal_t ) * _paNodePool[i].nTriCount);
					}																	// KJ End
#endif
					else
					{
						FASSERT_NOW;
						nErrorCode = ML_GENERAL_ERROR;
						goto SKIP_COLLISION_GENERATION;
					}
					MLMesh_nTotalTriDataBytes += nVertDataBytes + nNormDataBytes;
					pNode->pTriData = malloc( nVertDataBytes + nNormDataBytes );
					if ( !pNode->pTriData )
					{
						nErrorCode = ML_OUT_OF_MEMORY;
						goto SKIP_COLLISION_GENERATION;
					}

					// Fill in all of the packet info and vert data
					u16 nSanityCounter = 0;
					u16 iCurrentVertIdx = 0;
					u16 *pVertIdx = (u16 *)pNode->pTriData;
					u8 *pNormal = (u8 *)((u32)pNode->pTriData + nVertDataBytes);
					for ( ii = 0; ii < nPacketCount; ii++ )
					{
						// Set up the packet
						pNode->paPackets[ii].nCollType = Packets[ii].nCollType;
						pNode->paPackets[ii].nCollMask = Packets[ii].nCollMask;
						pNode->paPackets[ii].nTriCount = (u8)Packets[ii].nTriCount;
						pNode->paPackets[ii].nStartVert = iCurrentVertIdx;
						pNode->paPackets[ii].nVBIdx = Packets[ii].nVBIdx;
						iCurrentVertIdx += Packets[ii].nTriCount * 3;

						// Add in the vert data for this packet into the node
						pTemp = _paNodePool[i].pTriList;
						while ( pTemp )
						{
							if ( pTemp->nTriPacket == (s32)ii )
							{
								// Bounds check
								FASSERT( nSanityCounter < _paNodePool[i].nTriCount );
								FASSERT( pVertIdx + 3 <= ((u16 *)pNode->pTriData) + (_paNodePool[i].nTriCount * 3) );

								(*pVertIdx) = pTemp->anPosIdx[0];
								pVertIdx++;
								(*pVertIdx) = pTemp->anPosIdx[1];
								pVertIdx++;
								(*pVertIdx) = pTemp->anPosIdx[2];
								pVertIdx++;

								if ( MLManager.GetPlatformType() == PASM_TARGET_PC )
								{
									// For PC, we store uncompressed normals
									((FkDOP_Normal_t *)pNormal)->x = pTemp->vFaceNormal.x;
									((FkDOP_Normal_t *)pNormal)->y = pTemp->vFaceNormal.y;
									((FkDOP_Normal_t *)pNormal)->z = pTemp->vFaceNormal.z;
									pNormal += sizeof( FkDOP_Normal_t );
								}
								else if ( MLManager.GetPlatformType() == PASM_TARGET_GC )
								{
									// On GameCube we compress the normals
									((FkDOP_CNormal_t *)pNormal)->CompressFrom( pTemp->vFaceNormal );
									pNormal += sizeof( FkDOP_CNormal_t );
								}
#ifdef _MMI_TARGET_PS2
								else if ( MLManager.GetPlatformType() == PASM_TARGET_PS2 )     // KJ Begin
								{
									// For PC, we store uncompressed normals
									((FkDOP_Normal_t *)pNormal)->x = pTemp->vFaceNormal.x;
									((FkDOP_Normal_t *)pNormal)->y = pTemp->vFaceNormal.y;
									((FkDOP_Normal_t *)pNormal)->z = pTemp->vFaceNormal.z;
									pNormal += sizeof( FkDOP_Normal_t );
								}																	// KJ End
#endif
								else
								{
									FASSERT_NOW;
									nErrorCode = ML_GENERAL_ERROR;
									goto SKIP_COLLISION_GENERATION;
								}

								nSanityCounter++;
							}

							pTemp = pTemp->pNext;
						}
					}

					FASSERT( nSanityCounter == _paNodePool[i].nTriCount );

					// Add in the tri packets for this node
					MLMesh_nTotalTriPackets += pNode->nTriPacketCount;
				}
				else
				{
					// This node is not a leaf so record the child info
					pNode->nStartChildIdx = _paNodePool[i].nStartChildIdx;
				}
			}

			// Recurse through the tree setting the CollMasks
			m_paCollTree[nCollCount].nMasterCollMask = (u16)SetCollMask( m_paCollTree[nCollCount].pakDOPNodes, m_paCollTree[nCollCount].pakDOPNodes );

#if MLMESH_PROFILE_TREE
			FILE *pFile = fopen( "c:\\MLMesh_TreeProfile.txt", "a+t" );
			if ( pFile )
			{
				f32 fAverageChildPerc = _fTotalVolume / (f32)_nTotalChildren * 100.f;
				f32 fAverageTotalPerc = fAverageChildPerc * 2;

				char szTemp[128];
				sprintf( szTemp, "-----------\n" );
				fwrite( szTemp, strlen( szTemp ), 1, pFile );
				sprintf( szTemp, "Average Child Volume percent of parent: %3.1f\n", fAverageChildPerc );
				fwrite( szTemp, strlen( szTemp ), 1, pFile );
				sprintf( szTemp, "Average Child total percent of parent: %3.1f\n", fAverageTotalPerc );
				fwrite( szTemp, strlen( szTemp ), 1, pFile );
				sprintf( szTemp, "-----------\n" );
				fwrite( szTemp, strlen( szTemp ), 1, pFile );
				DumpNodeData( pFile, nCollCount, 0, 0 );
				sprintf( szTemp, "-----------\n" );
				fclose( pFile );
			}
#endif
			nCollCount++;

			nTimeFormattingData += timeGetTime() - nOpStartTime;
		}

		pTempSegment = pTempSegment->m_pNext;
	}

	m_nCollTreeCount = nCollCount;
	MLMesh_nTotalTreeCount = nCollCount;

//	DEVPRINTF( "Setup: %d - Compressing: %d - Formatting: %d\n", nTimeSettingUpData, nTimeCompressingData, nTimeFormattingData );

SKIP_COLLISION_GENERATION:

	// Free Memory
	free( _paNodePool );
	_paNodePool = NULL;

	free( _paTriPool );
	_paTriPool = NULL;

	free( _paIntervalPool );
	_paIntervalPool = NULL;

	return nErrorCode;
}


//
//
//
static u32 SetCollMask( FkDOP_Node_t *pRootNode, FkDOP_Node_t *pNode )
{
	u32 i;

	pNode->nCollMask = 0;

	if ( pNode->paPackets )
	{
		// This is a leaf node, so total up the tri packets
		for ( i = 0; i < pNode->nTriPacketCount; i++ )
		{
			pNode->nCollMask |= pNode->paPackets[i].nCollMask;
		}
	}
	else
	{
		// Recurse through the children
		pNode->nCollMask |= SetCollMask( pRootNode, &pRootNode[pNode->nStartChildIdx] );
		pNode->nCollMask |= SetCollMask( pRootNode, &pRootNode[pNode->nStartChildIdx+1] );
	}

	return pNode->nCollMask;
}


#if MLMESH_PROFILE_TREE
//
//
//
static void DumpNodeData( FILE *pFile, u32 nSegment, u32 nParentIdx, u32 nNodeIdx )
{
	char szTemp[512];

	if ( _paNodePool[nNodeIdx].nTriCount )
	{
		sprintf( szTemp, "%d, %d, %d, %d, %3.1f, %d\n", nSegment, nParentIdx, nNodeIdx,
						_paNodePool[nNodeIdx].nTreeDepth, _paNodePool[nNodeIdx].fPercentOfParent * 100.f,
						_paNodePool[nNodeIdx].nTriCount );
		fwrite( szTemp, strlen( szTemp ), 1, pFile );
	}
	else
	{
		sprintf( szTemp, "%d, %d, %d, %d, %3.1f\n", nSegment, nParentIdx, nNodeIdx,
						_paNodePool[nNodeIdx].nTreeDepth, _paNodePool[nNodeIdx].fPercentOfParent * 100.f );
		fwrite( szTemp, strlen( szTemp ), 1, pFile );

		DumpNodeData( pFile, nSegment, nNodeIdx, _paNodePool[nNodeIdx].nStartChildIdx );
		DumpNodeData( pFile, nSegment, nNodeIdx, _paNodePool[nNodeIdx].nStartChildIdx + 1 );
	}
}
#endif


//
//
//
void MLMesh::GenerateBoundingCapsule( CollNode *pNode, FMesh_Coll_Capsule_t *pCapsule )
{
	FASSERT( pNode && pCapsule );

	if ( m_FMesh.nFlags & FMESH_FLAGS_VOLUME_MESH )
	{
		// No Capsules for volume meshes
		pCapsule->fRadius = 0.f;
		pCapsule->vEnd.Zero();
		pCapsule->vStart.Zero();
	}

	#define __AXES_COUNT	3
	static CFVec3A avPossibleAxes[__AXES_COUNT] =
	{
		CFVec3A( 1.f, 0.f, 0.f ),
		CFVec3A( 0.f, 1.f, 0.f ),
		CFVec3A( 0.f, 0.f, 1.f )
	};

	u32 i, ii;
	f32 fAxisMin[__AXES_COUNT];
	f32 fAxisMax[__AXES_COUNT];

	// Record the start interval
	pNode->nStartkDOPInterval = (u16)_nIntervalCount;

	f32 fDot;
	CollTri *pTemp;
	// Determine the intervals along the axes of the kDOP
	for ( i = 0; i < __AXES_COUNT; i++ )
	{
		fAxisMin[i] = FMATH_MAX_FLOAT;
		fAxisMax[i] = -FMATH_MAX_FLOAT;

		pTemp = pNode->pTriList;
		while ( pTemp )
		{
			for ( ii = 0; ii < 3; ii++ )
			{
				fDot = avPossibleAxes[i].v3.Dot( pTemp->avPos[ii] );

				if ( fDot < fAxisMin[i] )
				{
					fAxisMin[i] = fDot;
				}
				if ( fDot > fAxisMax[i] )
				{
					fAxisMax[i] = fDot;
				}
			}

			pTemp = pTemp->pNext;
		}
	}

	CFVec3A *pMaxAx;
	CFVec3A vStart, vDiff, vCross, vTest;
	f32 fRadius;

	u32 nMaxAx = 0;
	pMaxAx = &avPossibleAxes[0];
	f32 fDist = fmath_Abs( fAxisMax[0] - fAxisMin[0] );
	for ( i = 1; i < __AXES_COUNT; i++ )
	{
		if ( fmath_Abs( fAxisMax[i] - fAxisMin[i] ) > fDist )
		{
			fDist = fmath_Abs( fAxisMax[i] - fAxisMin[i] );
			pMaxAx = &avPossibleAxes[i];
			nMaxAx = i;
		}
	}

	vStart.Set( fAxisMin[0] + ((fAxisMax[0] - fAxisMin[0]) * 0.5f), fAxisMin[1] + ((fAxisMax[1] - fAxisMin[1]) * 0.5f), fAxisMin[2] + ((fAxisMax[2] - fAxisMin[2]) * 0.5f) );
	fRadius = 0.f;

	pTemp = pNode->pTriList;
	while ( pTemp )
	{
		for ( ii = 0; ii < 3; ii++ )
		{
			vDiff.v3 = pTemp->avPos[ii] - vStart.v3;

			vCross.Cross( vDiff, *pMaxAx ).Cross( *pMaxAx );
			f32 fLengthSq = vCross.MagSq();
			if ( fLengthSq < 0.0001f )
			{
				continue;
			}
			vCross.Mul( fmath_AcuInvSqrt( fLengthSq ) );
			fDot = -vDiff.Dot( vCross );

			if ( fDot > fRadius )
			{
				fRadius = fDot;
			}
		}

		pTemp = pTemp->pNext;
	}

	// Fudge the radius a little
	fRadius *= 1.01f;

	f32 fHalfLength = fmath_Abs( fAxisMax[nMaxAx] - fAxisMin[nMaxAx] ) * 0.5f;
	vDiff.Mul( *pMaxAx, fHalfLength );
	pCapsule->vStart = vStart.v3 - vDiff.v3;
	pCapsule->vEnd = vStart.v3 + vDiff.v3;
	pCapsule->fRadius = fRadius;

	// See if we can bring in the endcaps:

	f32 fRadiusSq = fRadius * fRadius;

	// Try to bring in the end point
	if ( fRadius > fHalfLength )
	{
		vTest.v3 = pCapsule->vEnd - (pMaxAx->v3 * fHalfLength);
	}
	else
	{
		vTest.v3 = pCapsule->vEnd - (pMaxAx->v3 * fRadius);
	}
	fDist = 0.f;
	pTemp = pNode->pTriList;
	while ( pTemp )
	{
		for ( ii = 0; ii < 3; ii++ )
		{
			vDiff.v3 = pTemp->avPos[ii] - vTest.v3;

			// Calculate the parallel leg
			f32 fDot = vDiff.Dot( *pMaxAx );
			if ( fDot < 0.001f )
			{
				continue;
			}

			// This gives us the hypotenuse
			f32 fTestHypSq = vDiff.MagSq();
			if ( fTestHypSq < 0.001f || fTestHypSq <= (fDot * fDot) )
			{
				continue;
			}

			// Calculate the perpendicular leg
			f32 fCompare = fmath_AcuSqrt( fTestHypSq - (fDot * fDot) );

			// What is the distance to the end of the capsule, given the length of the perpendicular leg?
			// We know the radius and the perpendicular leg:
			FASSERT( fRadiusSq > (fCompare * fCompare) );
			f32 fActualDistance = fmath_AcuSqrt( fRadiusSq - (fCompare * fCompare) );

			// The difference between the parallel distance to the end of the capsule and the parallel leg length
			// is the additional distance we need to push out the cap to include this point.
			if ( fDot - fActualDistance > fDist )
			{
				fDist = fDot - fActualDistance;
			}
		}

		pTemp = pTemp->pNext;
	}

	FASSERT( fDist <= fRadius );
	pCapsule->vEnd = vTest.v3 + (pMaxAx->v3 * fDist);

	// Now bring in the start point
	if ( fRadius > fHalfLength )
	{
		vTest.v3 = pCapsule->vStart + (pMaxAx->v3 * fHalfLength);
	}
	else
	{
		vTest.v3 = pCapsule->vStart + (pMaxAx->v3 * fRadius);
	}
	fDist = 0.f;
	pTemp = pNode->pTriList;
	while ( pTemp )
	{
		for ( ii = 0; ii < 3; ii++ )
		{
			vDiff.v3 = pTemp->avPos[ii] - vTest.v3;

			// Calculate the parallel leg
			f32 fDot = -vDiff.Dot( *pMaxAx );
			if ( fDot < 0.001f )
			{
				continue;
			}

			// This gives us the hypotenuse
			f32 fTestHypSq = vDiff.MagSq();
			if ( fTestHypSq < 0.001f || fTestHypSq <= (fDot * fDot) )
			{
				continue;
			}

			// Calculate the perpendicular leg
			f32 fCompare = fmath_AcuSqrt( fTestHypSq - (fDot * fDot) );

			// What is the distance to the end of the capsule, given the length of the perpendicular leg?
			// We know the radius and the perpendicular leg:
			FASSERT( fRadiusSq > (fCompare * fCompare) );
			f32 fActualDistance = fmath_AcuSqrt( fRadiusSq - (fCompare * fCompare) );

			// The difference between the parallel distance to the end of the capsule and the parallel leg length
			// is the additional distance we need to push out the cap to include this point.
			if ( fDot - fActualDistance > fDist )
			{
				fDist = fDot - fActualDistance;
			}
		}

		pTemp = pTemp->pNext;
	}

	FASSERT( fDist <= fRadius );
	pCapsule->vStart = vTest.v3 - (pMaxAx->v3 * fDist);

	#undef __AXES_COUNT
}

//
//
//
void MLMesh::GenerateNode( CollNode *pNode, u32 nkDOPType )
{
	// Don't exceed range!
	if ( _nIntervalCount + FkDOP_aDesc[nkDOPType].nSideCount >= _MAX_COLL_INTERVALS )
	{
		FASSERT_NOW;
		return;
	}

	u32 i, ii;
	f32 fAxisAverage[FKDOP_MAX_AXES];
	memset( fAxisAverage, 0, sizeof( f32 ) * (FKDOP_MAX_AXES) );

	// Record the start interval
	pNode->nStartkDOPInterval = (u16)_nIntervalCount;

	// Determine the intervals along the axes of the kDOP
	for ( i = 0; i < FkDOP_aDesc[nkDOPType].nNormalCount; i++ )
	{
		f32 fMin = FMATH_MAX_FLOAT;
		f32 fMax = -FMATH_MAX_FLOAT;
		CollTri *pTemp = pNode->pTriList;
		while ( pTemp )
		{
			pTemp->afCentroidIntervals[i] = FkDOP_aDesc[nkDOPType].avNormals[i].v3.Dot( pTemp->vCentroid );

			// Add in this tri's centroid axis position for later averaging
			fAxisAverage[i] += pTemp->afCentroidIntervals[i];

			for ( ii = 0; ii < 3; ii++ )
			{
				// If we're at the root of the tree, we need to calculate all of the
				// intervals.  Otherwise, we know that they have already been calculated.
				if ( pNode->nTreeDepth == 0 )
				{
					pTemp->afIntervals[ii][i] = FkDOP_aDesc[nkDOPType].avNormals[i].v3.Dot( pTemp->avPos[ii] );
				}

				if ( pTemp->afIntervals[ii][i] < fMin )
				{
					fMin = pTemp->afIntervals[ii][i];
				}
				if ( pTemp->afIntervals[ii][i] > fMax )
				{
					fMax = pTemp->afIntervals[ii][i];
				}
			}

			pTemp = pTemp->pNext;
		}

		// If our interval does not exceed some minimal threshhold, then force it to
		if ( fMin == fMax )
		{
			fMin = fMin - 0.005f;
			fMax = fMax + 0.005f;
		}

		// Calculate the average position along this axis
//		fAxisAverage[i] /= pNode->nTriCount;
		fAxisAverage[i] = fmath_Div( fAxisAverage[i], pNode->nTriCount );

		// Store the intervals
		_paIntervalPool[_nIntervalCount].fMin = fMin;
		_paIntervalPool[_nIntervalCount].fMax = fMax;

#if MLMESH_PROFILE_TREE
		if ( i < 3 )
		{
			pNode->vMin.a[i] = fMin;
			pNode->vMax.a[i] = fMax;
		}
#endif

		_nIntervalCount++;
	}

#if MLMESH_PROFILE_TREE
	pNode->fBoxVolume = (f32)(fabs(pNode->vMax.x - pNode->vMin.x)
						* fabs(pNode->vMax.y - pNode->vMin.y)
						* fabs(pNode->vMax.z - pNode->vMin.z));
#endif

	// If we are already below the max tris per leaf, then this is a leaf and we
	// do not need to sub-divide the collision data any further
	if ( !MLMesh_bVolumeMesh && pNode->nTriCount < MLMESH_MAX_TRIS_PER_DYNAMIC_LEAF )
	{
		return;
	}
	else if ( MLMesh_bVolumeMesh && pNode->nTriCount < MLMESH_MAX_TRIS_PER_STATIC_LEAF )
	{
		return;
	}

	// Subdivide this node into two child nodes

	// Do not exceed the buffer
	if ( _nNodeCount + 2 >= _MAX_COLL_NODES )
	{
		FASSERT_NOW;
		return;
	}

	enum
	{
		__CUT_X_AXIS = 0,
		__CUT_Y_AXIS,
		__CUT_Z_AXIS,
		__CUT_UNSPECIFIED = 0xffff,
	};

	// See the comments for the tree type enum in MLMesh.h for a description of the tree types
	u32 nCutAxis = __CUT_UNSPECIFIED;
	switch ( m_nTreeCalcType )
	{
		case MLMESH_TREETYPE_MIN_SUM:
		{
			nCutAxis = __CUT_X_AXIS;
			f32 fTotal = DetermineTotalChildVolume( pNode, __CUT_X_AXIS, fAxisAverage[__CUT_X_AXIS] );

			f32 fTest =  DetermineTotalChildVolume( pNode, __CUT_Y_AXIS, fAxisAverage[__CUT_Y_AXIS] );;
			if ( fTest < fTotal )
			{
				fTotal = fTest;
				nCutAxis = __CUT_Y_AXIS;
			}

			fTest =  DetermineTotalChildVolume( pNode, __CUT_Z_AXIS, fAxisAverage[__CUT_Z_AXIS] );;
			if ( fTest < fTotal )
			{
				fTotal = fTest;
				nCutAxis = __CUT_Z_AXIS;
			}

			break;
		}

		case MLMESH_TREETYPE_MIN_MAX:
		{
			FASSERT_NOW;
			break;
		}

		case MLMESH_TREETYPE_SPLATTER:
		{
			//	Splatter Implementation:
			f32 fVariance[3];
			for ( i = 0; i < 3; i++ )
			{
				f32 fMin = FMATH_MAX_FLOAT;
				f32 fMax = -FMATH_MAX_FLOAT;

				// Project the centroid of each poly to determine the extent along each axis
				CollTri *pTemp = pNode->pTriList;
				while ( pTemp )
				{
					if ( pTemp->afCentroidIntervals[i] < fMin )
					{
						fMin = pTemp->afCentroidIntervals[i];
					}
					if ( pTemp->afCentroidIntervals[i] > fMax )
					{
						fMax = pTemp->afCentroidIntervals[i];
					}

					pTemp = pTemp->pNext;
				}

				// Calculate the variance along this axis
				fVariance[i] = fMax - fMin;
			}

			// Now that we have the variances, determine the axis to cut along (the one with the greatest variance)
			if ( fVariance[0] > fVariance[1] && fVariance[0] > fVariance[2] )
			{
				nCutAxis = __CUT_X_AXIS;
			}
			else if ( fVariance[1] > fVariance[0] && fVariance[1] > fVariance[2] )
			{
				nCutAxis = __CUT_Y_AXIS;
			}
			else
			{
				nCutAxis = __CUT_Z_AXIS;
			}

			break;
		}

		case MLMESH_TREETYPE_LONG_SIDE:
		{
			FASSERT_NOW;
			break;
		}

		default:
		{
			FASSERT_NOW;
			break;
		}
	}

	if ( nCutAxis == __CUT_UNSPECIFIED )
	{
		FASSERT_NOW;
		return;
	}

	// Make sure that this axis results in some division of the tris
	u32 nChildTriCount[2] = { 0, 0 };
	CollTri *pTemp = pNode->pTriList;
	while ( pTemp )
	{
		if ( pTemp->vCentroid.a[nCutAxis] < fAxisAverage[nCutAxis] )
		{
			// Give this tri to child 1
			nChildTriCount[0]++;
		}
		else
		{
			// Give this tri to child 2
			nChildTriCount[1]++;
		}

		if ( nChildTriCount[0] != 0 && nChildTriCount[1] != 0 )
		{
			break;
		}

		pTemp = pTemp->pNext;
	}

	// If we didn't get separation, then stop trying
	if ( pTemp == NULL )
	{
		return;
	}

	pNode->nStartChildIdx = (u16)_nNodeCount;
	_nNodeCount += 2;

	u32 nChildIdx[2];
	nChildIdx[0] = pNode->nStartChildIdx;
	nChildIdx[1] = pNode->nStartChildIdx + 1;
	memset( &_paNodePool[nChildIdx[0]], 0, sizeof ( CollNode ) );
	memset( &_paNodePool[nChildIdx[1]], 0, sizeof ( CollNode ) );

	_paNodePool[nChildIdx[0]].nTreeDepth = pNode->nTreeDepth + 1;
	_paNodePool[nChildIdx[1]].nTreeDepth = pNode->nTreeDepth + 1;

	// Distribute all of the tris to the appropriate child dividing along the mean
	CollTri *pChildLast[2];
	pChildLast[0] = NULL;
	pChildLast[1] = NULL;
	pTemp = pNode->pTriList;
	while ( pTemp )
	{
		u32 nAssignIdx;

		if ( pTemp->vCentroid.a[nCutAxis] < fAxisAverage[nCutAxis] )
		{
			// Give this tri to child 1
			nAssignIdx = 0;
		}
		else
		{
			// Give this tri to child 2
			nAssignIdx = 1;
		}

		if ( _paNodePool[nChildIdx[nAssignIdx]].pTriList != NULL )
		{
			pChildLast[nAssignIdx]->pNext = pTemp;
		}
		else
		{
			_paNodePool[nChildIdx[nAssignIdx]].pTriList = pTemp;
		}

		_paNodePool[nChildIdx[nAssignIdx]].nTriCount++;
		pChildLast[nAssignIdx] = pTemp;

		pTemp = pTemp->pNext;
	}

	// We should have had SOME division
	FASSERT( pChildLast[0] && pChildLast[1] );

	pChildLast[0]->pNext = NULL;
	pChildLast[1]->pNext = NULL;

	// The total of all the child tris should equal that of the parent
	FASSERT( _paNodePool[nChildIdx[0]].nTriCount + _paNodePool[nChildIdx[1]].nTriCount == pNode->nTriCount );

	// Since we distributed all of the tris, set the parent node to have no tris
	pNode->nTriCount = 0;
	pNode->pTriList = NULL;

	// Call GenerateNode() per child
	GenerateNode( &_paNodePool[nChildIdx[0]], nkDOPType );
	GenerateNode( &_paNodePool[nChildIdx[1]], nkDOPType );

#if MLMESH_PROFILE_TREE
	_paNodePool[nChildIdx[0]].fPercentOfParent = _paNodePool[nChildIdx[0]].fBoxVolume / pNode->fBoxVolume;
	_paNodePool[nChildIdx[1]].fPercentOfParent = _paNodePool[nChildIdx[1]].fBoxVolume / pNode->fBoxVolume;

	_fTotalVolume += _paNodePool[nChildIdx[0]].fPercentOfParent + _paNodePool[nChildIdx[1]].fPercentOfParent;
	_nTotalChildren += 2;
#endif
}


//
//
//
static f32 DetermineTotalChildVolume( CollNode *pNode, u32 nCutAxis, f32 fCutPoint )
{
	f32 ChildVolumes[2] = { 1.f, 1.f };
	u32 i, ii, nAssignIdx;
	u32 nTris[2] = { 0,0 };

	for ( i = 0; i < 3; i++ )
	{
		f32 fMin[2] = { FMATH_MAX_FLOAT, FMATH_MAX_FLOAT };
		f32 fMax[2] = { -FMATH_MAX_FLOAT, -FMATH_MAX_FLOAT };
		CollTri *pTemp = pNode->pTriList;

		while ( pTemp )
		{
			if ( pTemp->afCentroidIntervals[nCutAxis] < fCutPoint )
			{
				// Give this tri to child 1
				nAssignIdx = 0;
			}
			else
			{
				// Give this tri to child 2
				nAssignIdx = 1;
			}

			if ( i == 0 )
			{
				nTris[nAssignIdx]++;
			}

			for ( ii = 0; ii < 3; ii++ )
			{
				if ( pTemp->afIntervals[ii][i] < fMin[nAssignIdx] )
				{
					fMin[nAssignIdx] = pTemp->afIntervals[ii][i];
				}
				if ( pTemp->afIntervals[ii][i] > fMax[nAssignIdx] )
				{
					fMax[nAssignIdx] = pTemp->afIntervals[ii][i];
				}
			}

			pTemp = pTemp->pNext;
		}

		ChildVolumes[0] *= (f32)fabs(fMax[0] - fMin[0]);
		ChildVolumes[1] *= (f32)fabs(fMax[1] - fMin[1]);
	}

	return ChildVolumes[0] + ChildVolumes[1];

}
