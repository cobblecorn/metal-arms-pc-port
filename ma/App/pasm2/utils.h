//////////////////////////////////////////////////////////////////////////////////////
// utils.h - 
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
// 01/24/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _UTILS_H_
#define _UTILS_H_ 1

#include "fang.h"
#include "fmath.h"
#include "KongDef.h"

#define UTILS_PASM_TEMP_FILENAME_TAG		".pasmtemp"

// used to convert pointers in the data to offsets from the base address
#define UTILS_CONVERT_PTR_TO_OFFSET( nAddress, nStartingOffset )	( (nAddress) ? ((u32)(nAddress) - (u32)(nStartingOffset)) : 0 )
// used to convert offsets in the data to pointers from the base address
#define UTILS_CONVERT_OFFSET_TO_PTR( nOffset, nStartingOffset )		( (nOffset) ? ((u32)(nOffset) + (u32)(nStartingOffset)) : 0 )

#define UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER	1.001f

#if defined(__cplusplus)
extern "C" {
#endif

FINLINE void utils_GetBoundingSphereFromMinMax( CFVec3 &Center, f32 &fRadius, const CFVec3 &Min, const CFVec3 &Max ) {
	
	CFVec3 Diagional;

	Diagional = Max - Min;
	Diagional *= 0.5f;
	// fill in our calculated fields
	Center = Min + Diagional;
	fRadius = Diagional.Mag() * UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;	
}

// P... are the 3d points
// VN... are the vertex normals
FINLINE void utils_ComputeFaceNormal( CFVec3 &DstFaceUnitNorm, 
					    const CFVec3 &P1, const CFVec3 &P2, const CFVec3 &P3,
						const CFVec3 &VN1, const CFVec3 &VN2, const CFVec3 &VN3 ) {

	CFVec3 V12( P2 );
	V12 -= P1;

	CFVec3 V13( P3 );
	V13 -= P1;
	
	DstFaceUnitNorm = V12.Cross( V13 );
	f32 fMag2 = DstFaceUnitNorm.Mag2();
	DstFaceUnitNorm *= -( fmath_AcuInvSqrt( fMag2 ) );

	// now check this normal with the average vert norm
	CFVec3 V0( VN1 );
	V0 += VN2;
	V0 += VN3;
	V0 *= (1.0f/3.0f);
	
	if( V0.Dot( DstFaceUnitNorm ) < 0.0f ) {
		DstFaceUnitNorm *= -1.0f;
	}	

#if 0
	// this is the test the game will do on the face normal
	f32 fUnitNormMagErr = DstFaceUnitNorm.Mag() - 1.0f;
	if( FMATH_FABS(fUnitNormMagErr) > 0.001f ) {
		DEVPRINTF( "Non-unit face normal found, error = %f.\n", fUnitNormMagErr );
	}
#endif
}

extern void utils_ComputeTriBound( CFSphere &rBoundSphere, 
							 const CFVec3 &rP1, const CFVec3 &rP2, const CFVec3 &rP3 );

FINLINE void utils_ComputeTriCentroid( CFVec3 &rCentroid, const CFVec3 &rP1, const CFVec3 &rP2, const CFVec3 &rP3 ) {
	rCentroid = rP1;
	rCentroid += rP2;
	rCentroid += rP3;
	rCentroid *= (1.0f/3.0f);
}

extern u32 utils_ComputeUniqueNumFromString( cchar *pszString );
extern u32 utils_HowManyVertsAreShared( KongTri_t &rTri1, KongTri_t &rTri2 );

// assumes that both tris are part of the same material
// tries to be as quick as possible
FINLINE BOOL utils_DoTheseTrisShareAnEdge( KongTri_t &rTri1, KongTri_t &rTri2 ) {
//	ApeVert_t **ppVertList1, **ppVertList2;
	u32 nCount = 0;

//	ppVertList1 = rTri1.paApeVerts;
//	ppVertList2 = rTri2.paApeVerts;

	if( rTri1.apKongVerts[0] == rTri2.apKongVerts[0] ||
		rTri1.apKongVerts[0] == rTri2.apKongVerts[1] ||
		rTri1.apKongVerts[0] == rTri2.apKongVerts[2]  ) {
		nCount++;
	}

	if( rTri1.apKongVerts[1] == rTri2.apKongVerts[0] ||
		rTri1.apKongVerts[1] == rTri2.apKongVerts[1] ||
		rTri1.apKongVerts[1] == rTri2.apKongVerts[2]  ) {
		nCount++;

		if( nCount >= 2 ) {
			return TRUE;
		}
	}

	if( rTri1.apKongVerts[2] == rTri2.apKongVerts[0] ||
		rTri1.apKongVerts[2] == rTri2.apKongVerts[1] ||
		rTri1.apKongVerts[2] == rTri2.apKongVerts[2]  ) {
		nCount++;

		if( nCount >= 2 ) {
			return TRUE;
		}
	}
	
	return FALSE;
}

// it is assumed that if these tris already know about each other, then they would 
// both contain each other in their edge list
FINLINE BOOL utils_DoTheseTrisAlreadyShareAnEdge( KongTri_t &rTri1, KongTri_t &rTri2 ) {
	u32 i;

	for( i=0; i < rTri2.nNumEdgeTris; i++ ) {
		if( rTri2.paEdgeTris[i] == &rTri1 ) {
			return TRUE;
		}
	}

	return FALSE;
}

// it is assumed that these tris bounding spheres intersect and that they don't already know about each other
FINLINE BOOL utils_AreTheseTrisAdjacentToEachOther( KongTri_t &rTri1, KongTri_t &rTri2, f32 fPointTolerance2 ) {
	u32 i, j, anIndex1[2], anIndex2[2], nCloseCount = 0;
	CFVec3A Delta;

	// find 2 pts in rTri1 that are fPointTolerance2 away from 2 pts in rTri2
	for( i=0; i < 3; i++ ) {
		for( j=0; j < 3; j++ ) {
			Delta.Sub( rTri1.apKongVerts[i]->Pos, rTri2.apKongVerts[j]->Pos );
			if( Delta.MagSq() <= fPointTolerance2 ) {
				anIndex1[nCloseCount] = i;
				anIndex2[nCloseCount] = j;
				nCloseCount++;
				if( nCloseCount == 2 ) {
					// we found 2 pts that are close enough, make sure that we have found an edge and not a pt
					if( anIndex2[0] != anIndex2[1] ) {
						return TRUE;
					} else {
						nCloseCount--;
					}
				} else {
					break;
				}
			}
		}
		if( i == 1 && nCloseCount == 0 ) {
			// no need to continue;
			return FALSE;
		}
	}

	return FALSE;
}

extern u32 utils_ConvertToMeshInitFlags( u32 nApeObjectFlags );
extern BOOL utils_AreStarCommandsEqual( ApeCommands_t *p1, ApeCommands_t *p2 );
extern BOOL utils_AreLayersEqual( ApeLayer_t *pL1, ApeLayer_t *pL2, BOOL bCompareDiffuseColor );
extern BOOL utils_AreMatsEqual( ApeMaterial_t *pMat1, ApeMaterial_t *pMat2 );
extern void utils_CalculateIntWeights( ApeVert_t *pVert, u8 *pnWeights );
extern void utils_QuantizeVec3( CFVec3 &rVec3, f32 fQuantizeValue, f32 fOneOverQValue );
extern void utils_QuantizeVec2( CFVec2 &rVec2, f32 fQuantizeValue, f32 fOneOverQValue );
extern void utils_ComputeQuantizeValues( u32 nSigDigits, f32 &rfQuantizeValue, f32 &rfOneOverQValue );
extern BOOL utils_PadFileSize( FILE *pFile, u32 nBytes );

#if defined(__cplusplus)
}
#endif

#endif



