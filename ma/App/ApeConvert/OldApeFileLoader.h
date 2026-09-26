//////////////////////////////////////////////////////////////////////////////////////
// ApeFileLoader.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 10/24/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef OLD_APE_FILE_LOADER_H_
#define OLD_APE_FILE_LOADER_H_ 1

#include "fang.h"
#include "old_ape_file_def.h"

// an ape file comes off of disk, it is exported from a 3d package like Max
class COldApeFileLoader
{
public:
	COldApeFileLoader();
	~COldApeFileLoader();
	
	static BOOL LoadAndValidateApeHeader( cchar *pszFilename, OldApeMesh_t &rMeshInfo, CString *psLastError=NULL, BOOL bLogErrors=FALSE );
	BOOL LoadApeFile( cchar *pszFilename );
	
	// helper functions to return loaded data
	OldApeMesh_t *GetMeshInfo();
	u32 GetNumMeshBones();
	u32 GetNumLights();
	u32 GetNumObjects();
	u32 GetNumFogs();
	u32 GetNumShapes();
	u32 GetNumVisVols();
	u32 GetNumVisPortals();
	u32 GetNumVisCells();
	u32 GetNumVisPlanes();
	u32 GetNumSegments();
	u32 GetNumMeshMaterials();
	u32 GetNumMeshVerts();
	u32 GetNumMeshIndices();
	s32 GetSegNumFromMeshMatIndex( u32 nMeshMatIndex );
	OldApeMaterial_t *GetMatInfoFromMeshMatIndex( u32 nMeshMatIndex );
	BOOL IsAnyMaterialSkinned();
	OldApeBone_t *GetMeshBone( u32 nBoneIndex );
	OldApeLight_t *GetLightInfo( u32 nLightNum );
	OldApeObject_t *GetObjectInfo( u32 nObjectNum );
	OldApeFog_t *GetFogInfo( u32 nFogNum );
	OldApeShape_t *GetShapeInfo( u32 nShapeNum );
	OldApeVisVolume_t *GetVisVol( u32 nVolNum );
	OldApeVisPortal_t *GetVisPortal( u32 nPortalNum );
	u32 GetNumBoneChildren();
	
	OldApeSegment_t *GetSegmentInfo( u32 nSegmentNum );
	u32 GetNumSegmentMaterials( u32 nSegmentNum );
	u32 GetNumSegmentVerts( u32 nSegmentNum );
	u32 GetNumSegmentIndices( u32 nSegmentNum );
	OldApeMaterial_t *GetMaterialInfo( u32 nSegmentNum, u32 nMaterialNum );
	u32 GetNumMaterialIndices( u32 nSegmentNum, u32 nMaterialNum );
	OldApeVert_t *GetVertData( u32 nSegmentNum, u32 nVertNum=0 );
	OldApeVertIndex_t *GetVertIndex( u32 nSegmentNum, u32 nMaterialNum, u32 nIndex=0 );
		
	cchar *GetLastError();

	CString m_sLastFileLoaded;

protected:
	void UnloadFile();

	BOOL CloneFromLoadedFile( const COldApeFileLoader &rFile );

private:
	void *m_pAllocMem;
	OldApeMesh_t *m_pMeshInfo;
	
	u32 m_nNumBones;
	OldApeBone_t *m_pBones;

	u32 m_nNumLights;
	OldApeLight_t *m_pLights;

	u32 m_nNumObjects;
	CPtrArray m_aObjects;// array of ApeObject_t pointers
	
	OldApeFog_t *m_pFog;

	u32 m_nNumShapes;
	CPtrArray m_aShapes;// array of ApeShape_t pointers

	u32 m_nNumVisVols;
	CPtrArray m_aVisVols;// array of ApeVisVolume_t

	u32 m_nNumVisPortals;
	CPtrArray m_aVisPortals;// array of ApeVisPortal_t
		
	u32 m_nNumSegments;
	CPtrArray m_aSegments;	// array of ApeSegment_t

	//////////////////////////////////////////////////////////////////////////////////////////
	// The following 3 CPtrArrays store the first material, vert, vert index for each segment.
	// Use the segment index to get the first element, the segment can tell you how many
	// of each element there are.
	//////////////////////////////////////////////////////////////////////////////////////////
	CPtrArray m_aMaterials;	// array of ApeMaterial_t pointers
	CPtrArray m_aVerts;		// array of ApeVert_t pointers
	CPtrArray m_aVertIndices;// array of ApeVertIndex_t pointers

	CString m_sLastError;

	static void UpdateErrorString( u32 nErrorCode, CString &rsErrorString );
	BOOL FixupPointers();
	BOOL FixupInvalidSTCoords( const CString &rsErrorHeading );
};

#endif

