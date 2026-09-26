//////////////////////////////////////////////////////////////////////////////////////
// utils.cpp - 
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
#include "stdafx.h"
#include "fang.h"
#include "utils.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions

void utils_ComputeTriBound( CFSphere &rBoundSphere, 
					  const CFVec3 &rP1, const CFVec3 &rP2, const CFVec3 &rP3 ) {
	CFVec3 v12( rP1 ), v13( rP1 ), v23( rP2 );
	v12 -= rP2;
	v13 -= rP3;
	v23 -= rP3;
	f32 fDist12 = v12.Mag2();
	f32 fDist13 = v13.Mag2();
	f32 fDist23 = v23.Mag2();

	f32 fDiameter = fDist12;
	CFVec3 *pLongest = &v12;
	const CFVec3 *pLeftOut = &rP3;
	CFVec3 Base( rP2 );

	if( fDist13 > fDiameter ) {
		fDiameter = fDist13;
		pLongest = &v13;
		pLeftOut = &rP2;
		Base = rP3;
	}
	if( fDist23 > fDiameter ) {
		fDiameter = fDist23;
		pLongest = &v23;
		pLeftOut = &rP1;
		Base = rP3;
	}
	f32 fRadius = fmath_Sqrt( fDiameter ) * 0.5f;
	*pLongest *= 0.5f;
	Base += *pLongest;

	// make sure that the radius covers the 3rd point too
	v13 = Base - *pLeftOut;
	f32 fR2 = v13.Mag2();
	if( fR2 > (fRadius * fRadius) ) {
		fRadius = fmath_Sqrt( fR2 );
	}
	rBoundSphere.m_fRadius = fRadius * UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;// add a fudge factor
	rBoundSphere.m_Pos = Base;
}

u32 utils_ComputeUniqueNumFromString( cchar *pszString ) {
	CString s = pszString;
	u32 nNum = 0;

	s.MakeLower();
	for( int i=0; i < s.GetLength(); i++ ) {
		nNum |= ( (u32)s.GetAt( i ) ) << i;	
	}
	return nNum;
}

/*
u32 utils_HowManyVertsAreShared( KongTri_t &rTri1, KongTri_t &rTri2 ) {
	
	if( rTri1.pKMat != rTri2.pKMat ) {
		// different materials, can't share any tris
		return 0;
	}
//	ApeVert_t **ppVertList1, **ppVertList2;
//	ppVertList1 = rTri1.paApeVerts;
//	ppVertList2 = rTri2.paApeVerts;

	u32 i, j, nCount = 0;
	for( i=0; i < 3; i++ ) {
		for( j=0; j < 3; j++ ) {
			if( rTri1.apKongVerts[i] == rTri2.apKongVerts[j] ) {
				nCount++;
				break;
			}
		}
	}
	return nCount;
}
*/
u32 utils_ConvertToMeshInitFlags( u32 nApeObjectFlags ) {
	u32 nFlags = FMESHINST_FLAG_NONE;

	if( nApeObjectFlags & APE_OB_FLAG_STATIC ) {
		nFlags |= FMESHINST_FLAG_STATIC;

		if( nApeObjectFlags & APE_OB_FLAG_LM ) {
			nFlags |= FMESHINST_FLAG_LM;
		}
		if( nApeObjectFlags & APE_OB_FLAG_VERT_RADIOSITY ) {
			nFlags |= FMESHINST_FLAG_VERT_RADIOSITY;
		}
	}
	if( nApeObjectFlags & APE_OB_FLAG_POSTER_Y ) {
		nFlags |= FMESHINST_FLAG_POSTER_Y;
	}
	if( nApeObjectFlags & APE_OB_FLAG_POSTER_X ) {
		nFlags |= FMESHINST_FLAG_POSTER_X;
	}
	if( nApeObjectFlags & APE_OB_FLAG_POSTER_Z ) {
		nFlags |= FMESHINST_FLAG_POSTER_Z;
	}
	if( nApeObjectFlags & APE_OB_FLAG_NO_COLL ) {
		nFlags |= FMESHINST_FLAG_NOCOLLIDE;
	}
	if( nApeObjectFlags & APE_OB_FLAG_NO_LIGHT ) {
		nFlags |= FMESHINST_FLAG_NOLIGHT;
	}
	if( nApeObjectFlags & APE_OB_FLAG_NO_DRAW ) {
		nFlags |= FMESHINST_FLAG_DONT_DRAW;
	}
	if( nApeObjectFlags & APE_OB_FLAG_PER_PIXEL ) {
		nFlags |= FMESHINST_FLAG_LIGHT_PER_PIXEL;
	}
	if( nApeObjectFlags & APE_OB_FLAG_ACCEPT_SHADOWS ) {
		nFlags |= FMESHINST_FLAG_ACCEPT_SHADOWS;
	}
	if( nApeObjectFlags & APE_OB_FLAG_CAST_SHADOWS ) {
		nFlags |= FMESHINST_FLAG_CAST_SHADOWS;
	}
	if( nApeObjectFlags & APE_OB_FLAG_TINT ) {
		nFlags |= FMESHINST_FLAG_TINT;
	}
	return nFlags;
}

BOOL utils_AreStarCommandsEqual( ApeCommands_t *p1, ApeCommands_t *p2 ) {
	if( p1->bSort == p2->bSort &&
		p1->nOrderNum == p2->nOrderNum &&
		p1->nShaderNum == p2->nShaderNum &&
		p1->nEmissiveMotifID == p2->nEmissiveMotifID &&
		p1->nSpecularMotifID == p2->nSpecularMotifID &&
		p1->nDiffuseMotifID == p2->nDiffuseMotifID &&
		p1->bUseEmissiveColor == p2->bUseEmissiveColor &&
		p1->bUseSpecularColor == p2->bUseSpecularColor &&
		p1->bUseDiffuseColor == p2->bUseDiffuseColor &&
		p1->nNumTexFrames == p2->nNumTexFrames &&
		p1->fFramesPerSecs == p2->fFramesPerSecs &&
		p1->fDeltaUPerSec == p2->fDeltaUPerSec &&
		p1->fDeltaVPerSec == p2->fDeltaVPerSec &&
		p1->nZTugValue == p2->nZTugValue &&
		p1->nFlags == p2->nFlags &&
		p1->nCollMask == p2->nCollMask &&
		p1->nReactType == p2->nReactType &&
		p1->nSurfaceType == p2->nSurfaceType &&
		p1->bNoColl == p2->bNoColl &&
		p1->TintRGB == p2->TintRGB &&
		p1->LightRGBI == p2->LightRGBI &&
		p1->nCollID == p2->nCollID ) {
		
		// see if we should check ids
		BOOL bCheckID = FALSE;
		if( p1->nID < 128 || p2->nID < 128 ) {
			// one of these layers has an assigned ID, we need to check the ids
			bCheckID = TRUE;
		} else if( p1->nID == 255 || p2->nID == 255 ) {
			// one or both of these don't have an id, we wouldn't want to compress if one did and the other didn't
			bCheckID = TRUE;
		}
		if( bCheckID ) {
			if( p1->nID != p2->nID ) {
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}

BOOL utils_AreLayersEqual( ApeLayer_t *pL1, ApeLayer_t *pL2, BOOL bCompareDiffuseColor ) {
	if( strcmp( pL1->szTexnames[APE_LAYER_TEXTURE_DIFFUSE], pL2->szTexnames[APE_LAYER_TEXTURE_DIFFUSE] ) == 0 && 
		utils_AreStarCommandsEqual( &pL1->StarCommands, &pL2->StarCommands ) &&
		pL1->bTextured == pL2->bTextured &&
		pL1->bDrawAsWire == pL2->bDrawAsWire &&
		pL1->bTwoSided == pL2->bTwoSided &&
		pL1->fShininess == pL2->fShininess &&
		pL1->fShinStr == pL2->fShinStr &&
		pL1->SpecularRGB == pL2->SpecularRGB &&
		pL1->abTile[0] == pL2->abTile[0] &&
		pL1->abTile[1] == pL2->abTile[1] &&
		pL1->SelfIllumRGB == pL2->SelfIllumRGB ) {

		if( bCompareDiffuseColor ) {
			// we only need to check the diffuse color if it is being used
			if( !(pL1->DiffuseRGB == pL2->DiffuseRGB) ) {
				return FALSE;
			}
		}

		return TRUE;
	}
	return FALSE;	
}

BOOL utils_AreMatsEqual( ApeMaterial_t *pMat1, ApeMaterial_t *pMat2 ) {

	if( pMat1->nLayersUsed != pMat2->nLayersUsed ) {
		return FALSE;
	}
	if( pMat1->nLODIndex != pMat2->nLODIndex ) {
		return FALSE;
	}
	if( !utils_AreStarCommandsEqual( &pMat1->StarCommands, &pMat2->StarCommands ) ) {
		return FALSE;
	}
	u32 i;
	for( i=0; i < pMat1->nLayersUsed; i++ ) {
		if( !utils_AreLayersEqual( &pMat1->aLayer[i], &pMat2->aLayer[i], (i==0) ? pMat1->StarCommands.bUseDiffuseColor : FALSE ) ) {
			return FALSE;
		}
	}
	return TRUE;
}

// pnWeights must point to MAX_WEIGHTS_PER_VERT u8's
void utils_CalculateIntWeights( ApeVert_t *pVert, u8 *pnWeights ) {
	u32 i, nNumWeights, nTotal, nMaxDeltaIndex;
	s32 nError;
	f32 fDelta, fMaxDelta;

	if( !pnWeights ) {
		return;
	}
	
	nNumWeights = (u32)pVert->fNumWeights;

	// first calculate the int weights
	nTotal = 0;
	for( i=0; i < nNumWeights; i++ ) {
		pnWeights[i] = (u8)( pVert->aWeights[i].fWeight * 255.0f );
		nTotal += pnWeights[i];
	}
	nError = 255 - nTotal;
	
	// distribute the error digits
	while( nError > 0 ) {
		
		// find the weight that is the furthest from its original value
		fMaxDelta = 0.0f;
		nMaxDeltaIndex = 0;
		for( i=0; i < nNumWeights; i++ ) {
			fDelta = pVert->aWeights[i].fWeight - (pnWeights[i] * (1.0f/255.0f));
			if( fDelta > fMaxDelta ) {
				fMaxDelta = fDelta;
				nMaxDeltaIndex = i;
			}
		}
		// give the most "out of whack" weight an error digit
		pnWeights[nMaxDeltaIndex]++;

		nError--;
	}
}

void utils_QuantizeVec3( CFVec3 &rVec3, f32 fQuantizeValue, f32 fOneOverQValue ) {
			
	if( fQuantizeValue <= 1.0f ) {
		// nothing to do
		return;
	}
	s32 nTemp = (s32)(rVec3.x * fQuantizeValue);
	rVec3.x = (f32)nTemp;
	rVec3.x *= fOneOverQValue;
	
	nTemp = (s32)(rVec3.y * fQuantizeValue);
	rVec3.y = (f32)nTemp;
	rVec3.y *= fOneOverQValue;
	
	nTemp = (s32)(rVec3.z * fQuantizeValue);
	rVec3.z = (f32)nTemp;
	rVec3.z *= fOneOverQValue;
}

void utils_QuantizeVec2( CFVec2 &rVec2, f32 fQuantizeValue, f32 fOneOverQValue ) {
			
	if( fQuantizeValue <= 1.0f ) {
		// nothing to do
		return;
	}
	s32 nTemp = (s32)(rVec2.x * fQuantizeValue);
	rVec2.x = (f32)nTemp;
	rVec2.x *= fOneOverQValue;
	
	nTemp = (s32)(rVec2.y * fQuantizeValue);
	rVec2.y = (f32)nTemp;
	rVec2.y *= fOneOverQValue;
}

void utils_ComputeQuantizeValues( u32 nSigDigits, f32 &rfQuantizeValue, f32 &rfOneOverQValue ) {
	
	rfQuantizeValue = 1.0f;
	for( u32 i=0; i < nSigDigits; i++ ) {
		rfQuantizeValue *= 10.0f;
	}
	rfOneOverQValue = (1.0f/rfQuantizeValue);
}

BOOL utils_PadFileSize( FILE *pFile, u32 nBytes ) {
	u32 i;
	char cZero = 0;

	for( i=0; i < nBytes; i++ ) {
		if( fwrite( &cZero, 1, 1, pFile ) != 1 ) {
			return FALSE;
		}
	}
	return TRUE;
}

//==================
// private functions



