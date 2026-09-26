//////////////////////////////////////////////////////////////////////////////////////
// VWeights.cpp - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/18/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "VWeights.h"
#include "fmath.h"

CVertexWeights::CVertexWeights( u32 nMaxWeightsPerVert/*=MAX_WEIGHTS_PER_VERT*/,
							    u32 nMaxWeightsPerFace/*=MAX_WEIGHTS_PER_FACE*/ ) {
	m_nNumWeights = 0;

	FASSERT( nMaxWeightsPerFace >= nMaxWeightsPerVert ); 
	m_nMaxWeightsPerFace = FMATH_MIN( nMaxWeightsPerFace, MAX_WEIGHTS_PER_FACE );
	m_nMaxWeightsPerVert = FMATH_MIN( nMaxWeightsPerVert, MAX_WEIGHTS_PER_VERT );
}

CVertexWeights::~CVertexWeights() {
}

void CVertexWeights::Reset() {
	
	m_nNumWeights = 0;
	// zero out our vertex weights
	fang_MemZero( &m_aWeights, sizeof( ApeWeight_t ) * VWEIGHTS_MAX_VERTEX_WEIGHTS );
}

void CVertexWeights::Insert( u32 nBoneID, f32 fWeight ) {
	u32 i;
	
	for( i=0; i < m_nNumWeights; i++ ) {
		if( (u32)m_aWeights[i].fBoneIndex == nBoneID ) {
			m_aWeights[i].fWeight += fWeight;// since this weight is listed already, just add the weights
			return;
		}
	}
	FASSERT( m_nNumWeights < VWEIGHTS_MAX_VERTEX_WEIGHTS );
	
	m_aWeights[m_nNumWeights].fBoneIndex = (f32)nBoneID;
	m_aWeights[m_nNumWeights].fWeight = fWeight;
	m_nNumWeights++;
}

u32 CVertexWeights::GetNumUsedWeights() {
	return m_nNumWeights;
}

const ApeWeight_t &CVertexWeights::GetWeight( u32 nIndex ) {
	FASSERT( nIndex < m_nNumWeights );

	return m_aWeights[nIndex];
}

// sort the weights by fBoneIndex, from lowest to highest
void CVertexWeights::SortWeightsByBoneIndex() {
	
	switch( m_nNumWeights ) {
	case 0:
	case 1:
		// nothing to sort
		break;
	case 2:
		// buble sort won't work, just compare
		if( m_aWeights[1].fBoneIndex < m_aWeights[0].fBoneIndex ) {
			// swap the spots
			SwapWeightLocations( m_aWeights[0], m_aWeights[1] );
		}
		break;
	default:
		{
			// more than 2 bones, bubble sort by index from lowest to heightest
			u32 i, j, n1stLoop, n2ndLoop;
			n1stLoop = m_nNumWeights - 1;
			n2ndLoop = m_nNumWeights;
			for( i=0; i < n1stLoop; i++ ) {
				for( j=i+1; j < n2ndLoop; j++ ) {
					if( m_aWeights[j].fBoneIndex < m_aWeights[i].fBoneIndex ) {
						// swap the spots
						SwapWeightLocations( m_aWeights[i], m_aWeights[j] );
					}
				}
			}
		}
		break;
	}
}

// sort the weights by fWeight, from highest to lowest
void CVertexWeights::SortWeightsByBoneWeight() {
	
	switch( m_nNumWeights ) {
	case 0:
	case 1:
		// nothing to sort
		break;
	case 2:
		// buble sort won't work, just compare
		if( m_aWeights[1].fWeight > m_aWeights[0].fWeight ) {
			// swap the spots
			SwapWeightLocations( m_aWeights[0], m_aWeights[1] );
		}
		break;
	default:
		{
			// more than 2 bones, bubble sort by weight from heightest to lowest
			u32 i, j, n1stLoop, n2ndLoop;
			n1stLoop = m_nNumWeights - 1;
			n2ndLoop = m_nNumWeights;
			for( i=0; i < n1stLoop; i++ ) {
				for( j=i+1; j < n2ndLoop; j++ ) {
					if( m_aWeights[j].fWeight > m_aWeights[i].fWeight ) {
						// swap the spots
						SwapWeightLocations( m_aWeights[i], m_aWeights[j] );
					}
				}
			}
		}
		break;
	}
}

// takes the largest m_nMaxWeightsPerVert vertex weights and throws away 
// all other vert weights, and then sorts the valid weights
void CVertexWeights::LimitAndScaleVertexWeights() {
	u32 i;

	if( m_nNumWeights <= 0 ) {
		// no weights, no work :-)
		return;
	}
	SortWeightsByBoneWeight();
	if( m_nNumWeights > m_nMaxWeightsPerVert ) {
		// there are more than m_nMaxWeightsPerVert, reduce the number of weights
		m_nNumWeights = m_nMaxWeightsPerVert;
	}
	// make sure that our weights add up to 1.0f
	f32 fTotal = 0.0f;
	for( i=0; i < m_nNumWeights; i++ ) {
		fTotal += m_aWeights[i].fWeight;
	}
	FASSERT( fTotal > 0.0f );// should never be the case
	if( fTotal != 1.0f ) {
		f32 fOOTotal = 1.0f/fTotal;
		for( i=0; i < m_nNumWeights; i++ ) {
			m_aWeights[i].fWeight *= fOOTotal;
		}
	}
}

// make sure our face only uses m_nMaxWeightsPerFace bone influences
void CVertexWeights::ProcessFace( ApeVert_t &V1, ApeVert_t &V2, ApeVert_t &V3 ) {
	u32 i, j;
	BOOL bFound;

	FASSERT( V1.fNumWeights > 0.0f );
	FASSERT( V2.fNumWeights > 0.0f );
	FASSERT( V3.fNumWeights > 0.0f );

	Reset();

	// copy the greatest weight from each vert first
	m_aWeights[0] = V1.aWeights[0];
	m_nNumWeights = 1;
	if( V2.aWeights[0].fBoneIndex != m_aWeights[0].fBoneIndex ) {
		m_aWeights[ m_nNumWeights++ ] = V2.aWeights[0];
	} else {
		m_aWeights[0].fWeight += V2.aWeights[0].fWeight;
	}
	for( i=0; i < m_nNumWeights; i++ ) {
		if( V3.aWeights[0].fBoneIndex == m_aWeights[i].fBoneIndex ) {
			m_aWeights[i].fWeight += V3.aWeights[0].fWeight;
			break;
		}
	}
	if( i == m_nNumWeights ) {
		m_aWeights[ m_nNumWeights++ ] = V3.aWeights[0];
	}
	// add a factor to each of the weights so that these bones definitely are in the final tri list
	// this will ensure that every face will contribute at least 1 weight
	for( i=0; i < m_nNumWeights; i++ ) {
		m_aWeights[i].fWeight += 3.0f;
	}
	// copy V1 into m_aWeights[]
	for( i=1; i < (u32)V1.fNumWeights; i++ ) {
		// first see if this bone is in the list already
		bFound = FALSE;
		for( j=0; j < m_nNumWeights; j++ ) {
			if( m_aWeights[j].fBoneIndex == V1.aWeights[i].fBoneIndex ) {
				bFound = TRUE;
				// add the weight
				m_aWeights[j].fWeight += V1.aWeights[i].fWeight;
				break;
			}
		}
		if( !bFound ) {
			// new bone, add it to our list
			m_aWeights[ m_nNumWeights++ ] = V1.aWeights[i];
		}
	}
	// copy V2 into m_aWeights[]
	for( i=1; i < (u32)V2.fNumWeights; i++ ) {
		// first see if this bone is in the list already
		bFound = FALSE;
		for( j=0; j < m_nNumWeights; j++ ) {
			if( m_aWeights[j].fBoneIndex == V2.aWeights[i].fBoneIndex ) {
				bFound = TRUE;
				// add the weight
				m_aWeights[j].fWeight += V2.aWeights[i].fWeight;
				break;
			}
		}
		if( !bFound ) {
			// new bone, add it to our list
			m_aWeights[ m_nNumWeights++ ] = V2.aWeights[i];
		}
	}
	// copy V3 into m_aWeights[]
	for( i=1; i < (u32)V3.fNumWeights; i++ ) {
		// first see if this bone is in the list already
		bFound = FALSE;
		for( j=0; j < m_nNumWeights; j++ ) {
			if( m_aWeights[j].fBoneIndex == V3.aWeights[i].fBoneIndex ) {
				bFound = TRUE;
				// add the weight
				m_aWeights[j].fWeight += V3.aWeights[i].fWeight;
				break;
			}
		}
		if( !bFound ) {
			// new bone, add it to our list
			m_aWeights[ m_nNumWeights++ ] = V3.aWeights[i];
		}
	}
	// sort the bones by weight
	SortWeightsByBoneWeight();

	// only the first x bone influences are enforced
	if( m_nNumWeights > m_nMaxWeightsPerFace ) {
		m_nNumWeights = m_nMaxWeightsPerFace;
	}
	EnsureBoneIDsAreValid( V1 );
	EnsureBoneIDsAreValid( V2 );
	EnsureBoneIDsAreValid( V3 );

	FASSERT( V1.fNumWeights > 0.0f );
	FASSERT( V2.fNumWeights > 0.0f );
	FASSERT( V3.fNumWeights > 0.0f );
}

// fixes a face list by ensuring that all faces are influenced by no more
// than m_nMaxWeightsPerFace unique weights.
// Returns the number of faces that had to be adjusted to get every face valid
u32 CVertexWeights::ProcessFaceList( ApeVert_t *pVertList, u32 nNumVerts,
									 ApeVertIndex_t *pIndexList, u32 nNumIndices ) {
	u32 i, uNumBoneInfluences, uTriIndex, uFacesFixed = 0;
	ApeVert_t *p1, *p2, *p3, *pTest;

	FindFaceWithGreatestNumBoneInfluences( pVertList, pIndexList, nNumIndices, uNumBoneInfluences, uTriIndex );
	while( uNumBoneInfluences > m_nMaxWeightsPerFace ) {
		p1 = &pVertList[ pIndexList[ uTriIndex   ].nVertIndex ];
		p2 = &pVertList[ pIndexList[ uTriIndex+1 ].nVertIndex ];
		p3 = &pVertList[ pIndexList[ uTriIndex+2 ].nVertIndex ];
		ProcessFace( *p1, *p2, *p3 );
		// now we must find any other ape verts that share a Pos but got put into other ape verts
		// because of TCs, Norms, RGB...
		for( i=0; i < nNumVerts; i++ ) {
			pTest = &pVertList[i];
			if( (u32)pTest != (u32)p1 && AreVec3Equal( &p1->Pos, &pTest->Pos ) ) {
				pTest->fNumWeights = p1->fNumWeights;
				fang_MemCopy( &pTest->aWeights[0], &p1->aWeights[0], (sizeof( ApeWeight_t ) * m_nMaxWeightsPerVert) );
			}
			if( (u32)pTest != (u32)p2 && AreVec3Equal( &p2->Pos, &pTest->Pos ) ) {
				pTest->fNumWeights = p2->fNumWeights;
				fang_MemCopy( &pTest->aWeights[0], &p2->aWeights[0], (sizeof( ApeWeight_t ) * m_nMaxWeightsPerVert) );
			}
			if( (u32)pTest != (u32)p3 && AreVec3Equal( &p3->Pos, &pTest->Pos ) ) {
				pTest->fNumWeights = p3->fNumWeights;
				fang_MemCopy( &pTest->aWeights[0], &p3->aWeights[0], (sizeof( ApeWeight_t ) * m_nMaxWeightsPerVert) );
			}
		}
		uFacesFixed++;
		// find the new worst tri
		FindFaceWithGreatestNumBoneInfluences( pVertList, pIndexList, nNumIndices, uNumBoneInfluences, uTriIndex );
	}
#if 0
	// now run through all of the faces and process their weights
	for( i=0; i < nNumIndices; i+=3 ) {
		ProcessWeights( pVertList[ pIndexList[i].nVertIndex ],
						pVertList[ pIndexList[i+1].nVertIndex ],
						pVertList[ pIndexList[i+2].nVertIndex ] );
	}
#endif

	return uFacesFixed;
}

void CVertexWeights::SwapWeightLocations( ApeWeight_t &W1, ApeWeight_t &W2 ) {
	ApeWeight_t TempW;

	TempW = W1;
	W1 = W2;
	W2 = TempW;
}

// make sure that Vert doesn't use any bone ids not contained in m_aWeights[]
void CVertexWeights::EnsureBoneIDsAreValid( ApeVert_t &Vert ) {
	u32 i, j, nBonesUsed;
	ApeWeight_t *apWeights[VWEIGHTS_MAX_VERTEX_WEIGHTS];
	
	// find the used bones
	nBonesUsed = 0;
	for( i=0; i < (u32)Vert.fNumWeights; i++ ) {
		for( j=0; j < m_nNumWeights; j++ ) {
			if( Vert.aWeights[i].fBoneIndex == m_aWeights[j].fBoneIndex ) {
				// bone is used
				apWeights[ nBonesUsed++ ] = &Vert.aWeights[i];
				break;
			}
		}		
	}
	// copy the used verts, rearranging the order
	f32 fTotal = 0.0f;
	for( i=0; i < m_nMaxWeightsPerFace; i++ ) {
		if( i < nBonesUsed ) {
			if( &Vert.aWeights[i] != apWeights[i] ) {
				Vert.aWeights[i] = *apWeights[i];
			}
			fTotal += Vert.aWeights[i].fWeight;
		} else {
			// zero out the unused bone entry
			fang_MemZero( &Vert.aWeights[i], sizeof( ApeWeight_t ) );
		}
	}
	Vert.fNumWeights = (f32)nBonesUsed;
	// renormalize our weights
	FASSERT( fTotal > 0.0f );// should never be the case
	if( fTotal != 1.0f ) {
		f32 fOOTotal = 1.0f/fTotal;
		for( i=0; i < nBonesUsed; i++ ) {
			Vert.aWeights[i].fWeight *= fOOTotal;
		}
	}
}

void CVertexWeights::FindFaceWithGreatestNumBoneInfluences( ApeVert_t *pVertList, 
														    ApeVertIndex_t *pIndexList, u32 nNumIndices,
														    u32 &ruNumBoneInfluences, u32 &ru1stIndexOfTri ) {
	u32 i, j, k, s, uNumBones;
	ApeVert_t *pVert;
	static u32 auBoneID[VWEIGHTS_MAX_VERTEX_WEIGHTS];

	ruNumBoneInfluences = 0;
	ru1stIndexOfTri = 0;
	// walk the tris
	for( i=0; i < nNumIndices; i+=3 ) {
		uNumBones = 0;
		// walk the verts in the face
		for( j=0; j < 3; j++ ) {
			pVert = &pVertList[ pIndexList[i+j].nVertIndex ];
			// walk the weights
			for( k=0; k < (u32)pVert->fNumWeights; k++ ) {
				// add the weights to our list, only if they haven't been added yet
				for( s=0; s < uNumBones; s++ ) {
					if( (u32)pVert->aWeights[k].fBoneIndex == auBoneID[s] ) {
						break;
					}
				}
				if( s == uNumBones ) {
					auBoneID[ uNumBones++ ] = (u32)pVert->aWeights[k].fBoneIndex;
				}
			}
		}
		// see if this face has more weights than any previous one
		if( uNumBones > ruNumBoneInfluences ) {
			ruNumBoneInfluences = uNumBones;
			ru1stIndexOfTri = i;
		}
	}
}

BOOL CVertexWeights::AreVec3Equal( CFVec3 *pV1, CFVec3 *pV2 ) {
	CFVec3 Diff;

	Diff = *pV1 - *pV2;
	if( Diff.Mag2() <= (0.001f * 0.001f) ) {
		return TRUE;
	}
	return FALSE;
}



