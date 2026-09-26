//////////////////////////////////////////////////////////////////////////////////////
// VWeights.h - 
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
#ifndef _VWEIGHTS_H_
#define _VWEIGHTS_H_ 1

#include "fang.h"
#include "ape_file_def.h"

#define VWEIGHTS_MAX_VERTEX_WEIGHTS		( MAX_WEIGHTS_PER_VERT * 3 + MAX_WEIGHTS_PER_FACE )

class CVertexWeights
{
public:
	CVertexWeights( u32 nMaxWeightsPerVert=MAX_WEIGHTS_PER_VERT, u32 nMaxWeightsPerFace=MAX_WEIGHTS_PER_FACE );
	~CVertexWeights();

	void Reset();// resets the class to accept a weight list
	void Insert( u32 nBoneID, f32 fWeight );
	u32 GetNumUsedWeights();
	const ApeWeight_t &GetWeight( u32 nIndex );
	void SortWeightsByBoneIndex();
	void SortWeightsByBoneWeight();
	void LimitAndScaleVertexWeights();	
	void ProcessFace( ApeVert_t &V1, ApeVert_t &V2, ApeVert_t &V3 );
	u32 ProcessFaceList( ApeVert_t *pVertList, u32 nNumVerts, ApeVertIndex_t *pIndexList, u32 nNumIndices );

private:
	u32 m_nMaxWeightsPerFace;
	u32 m_nMaxWeightsPerVert;

	u32 m_nNumWeights;
	ApeWeight_t m_aWeights[VWEIGHTS_MAX_VERTEX_WEIGHTS];

	void SwapWeightLocations( ApeWeight_t &W1, ApeWeight_t &W2 );
	void EnsureBoneIDsAreValid( ApeVert_t &Vert );
	void FindFaceWithGreatestNumBoneInfluences( ApeVert_t *pVertList, 
												ApeVertIndex_t *pIndexList, u32 nNumIndices,
												u32 &ruNumBoneInfluences, u32 &ru1stIndexOfTri );
	BOOL AreVec3Equal( CFVec3 *pV1, CFVec3 *pV2 );
	
protected:
	
};




#endif

