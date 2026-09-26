//////////////////////////////////////////////////////////////////////////////////////
// ApeToKongFormat.h - loads and converts an ape file to the kong format
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
#ifndef _APE_TO_KONG_FORMAT_H_
#define _APE_TO_KONG_FORMAT_H_ 1

#include "fang.h"
#include "KongDef.h"
#include "ApeFileLoader.h"

class CCompileDlg;

class CApeToKongFormat : public CApeFileLoader
{
public:
	CApeToKongFormat();
	~CApeToKongFormat();
	BOOL ConvertApeFile( CCompileDlg *pDlg, cchar *pszFilename, u32 nSigDigits=3 );
	void ResetTriAddedVar( KongMat_t &Mat ) const;
	void ResetTriDist2Var( KongMat_t &Mat ) const;
	void ResetMatAddedVar( KongSeg_t &Seg ) const;
	void ComputeMeshBoundingBoxFromKongTris( CFVec3 &vMin, CFVec3 &vMax ) const;
	void ComputeMeshBoundingSphereFromKongTris( CFVec3 &Center, f32 &fRadius, BOOL bVolumeMesh = FALSE ) const;
	void ComputeSegBoundingSphereFromKongTris( u32 nSegmentNum, CFVec3 &Center, f32 &fRadius ) const;
	void FindMinAndMaxPosFromKongTris( CFVec3 &Min, CFVec3 &Max, KongTri_t *pTri, u32 nNumTris, BOOL bUseMinMax=FALSE ) const;
	void ResetEdgeInfo();
	void BuildEdgeDataPerMaterial();
	void BuildEdgeDataAcrossEntireKong( CFVec3 &Min, CFVec3 &Max, CCompileDlg *pDlg = NULL );
	BOOL LoadApeFile( cchar *pszFilename );

	BOOL m_bA2KConverted;
	KongMesh_t *m_pKongMesh;
	BOOL m_bSkinned;// TRUE if any segment has greater than 1 bone influencing it, FALSE if 0 or 1 bones affect every segment
	BOOL m_bVolumeMesh;// TRUE if this mesh is a volume (world) mesh

protected:
	virtual void FreeData();
	static void ComputeEdgeInfo( CPtrArray &rKTris, u32 uNumTris, u32 uStartingIndex );
	CPtrArray *AllocatePtrArray();

	CPtrArray m_aPtrArrays;// CPtrArray *

private:
	BOOL ConvertToKongFormat( CCompileDlg *pDlg, u32 nSigDigits );
	void MapTriBoneInfluences( KongTri_t &Tri );
	KongTri_t *FindBestTriToBaseASegment( u32 nStartIndex, u32 nNumTris );
	BOOL CanTriBeAddedToSeg( KongTri_t &Tri, KongSeg_t &Seg );
	BOOL BiggieSizeSegment( KongSeg_t &Seg, u32 nStartIndex, u32 nNumTris );
	u32 HowManyTrisFitIntoSeg( KongSeg_t &Seg, u32 nStartIndex, u32 nLastIndex );	
};

#endif

