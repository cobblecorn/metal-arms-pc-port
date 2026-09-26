//////////////////////////////////////////////////////////////////////////////////////
// KongDef.h - contains the structures that make up the "Kong" format
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
#ifndef _KONGDEF_H_
#define _KONGDEF_H_ 1

#include "fang.h"
#include "fdata.h"
#include "ape_file_def.h"
#include "fsh.h"

#define KONG_FILE_VERSION			0x00000008

#define KONG_MAX_MATERIALS			1024
#define KONG_MAX_UVS_PER_MAT		12 // 4 surface ST's, 8 light map ST's
#define KONG_MAX_LAYERS_PER_MAT		4

#define KONG_INVALID_MAT_INDEX		-1
#define KONG_INVALID_TRI_INDEX		-1

////////////////////////////////////////////////////////////////////////////////////////
// THE KONG FORMAT IS AN EXPANDED APE FILE, BROKEN UP INTO SEGMENTS, MATERIALS, AND TRIS
// THIS IS EASIER TO DEAL WITH FOR SKINNED ANIMATION AND STRIPS

struct KongSeg_t;
struct KongMat_t;
struct KongMatProperties_t;
struct KongVert_t;
struct KongTri_t;

class CFileInfo;

//
//
//
struct KongVertRemap_t
{
	u16 nVBIndex;
	u16 nVertColorIndex;
};


//
//
//
struct KongFileHeader_t
{
	FVersionHeader_t Header;

	SYSTEMTIME APELastWriteTime;
	BOOL bHasAidFile;
	SYSTEMTIME AIDLastWriteTime;

	u32  nFileDataSize;		// Size of the kong data minus the header size
	BOOL bHasLightmapsSTs;
};


BOOL SavedKNGIsUpToDate( const CFileInfo *pFileInfo, const CFileInfo *pAidFileInfo, BOOL bLightmapsNeeded );
BOOL ReadKongHeader( cchar *pszKongFilePath, KongFileHeader_t *pKongHeader );


//
//
//
struct KongMesh_t 
{
	public:
		KongFileHeader_t KHeader;

		BOOL bWorldFile;

		BOOL bSegmentsLocallyAllocated;
		u32 nSegmentsAllocated;
		u32 nSegmentCount;
		KongSeg_t *paSegs;

		BOOL bMaterialsLocallyAllocated;
		u32 nMaterialsAllocated;
		u32 nMaterialCount;
		KongMat_t *paMaterials;

		BOOL bMatPropertiesLocallyAllocated;
		u32 nMatPropertiesAllocated;
		u32 nMatPropertiesCount;
		KongMatProperties_t *paMatProperties;

		BOOL bTrianglesLocallyAllocated;
		u32 nTrianglesAllocated;
		u32 nTriangleCount;
		KongTri_t *paTriangles;

		BOOL bVertsLocallyAllocated;
		u32 nVertsAllocated;
		u32 nVertCount;
		KongVert_t *paVertices;

	// Functions:
		KongMesh_t( void )
		{
			memset( &KHeader, 0, sizeof( KongFileHeader_t ));
			KHeader.Header.nSignature = FVERSION_FILE_SIGNATURE;
			KHeader.Header.nVersion = KONG_FILE_VERSION;
			KHeader.bHasLightmapsSTs = FALSE;
			KHeader.nFileDataSize = 0;

			bWorldFile = FALSE;

			bSegmentsLocallyAllocated = FALSE;
			nSegmentsAllocated = 0;
			nSegmentCount = 0;
			paSegs = NULL;

			bMaterialsLocallyAllocated = FALSE;
			nMaterialsAllocated = 0;
			nMaterialCount = 0;
			paMaterials = NULL;

			bMatPropertiesLocallyAllocated = FALSE;
			nMatPropertiesAllocated = 0;
			nMatPropertiesCount = 0;
			paMatProperties = NULL;

			bTrianglesLocallyAllocated = FALSE;
			nTrianglesAllocated = 0;
			nTriangleCount = 0;
			paTriangles = NULL;

			bVertsLocallyAllocated = FALSE;
			nVertsAllocated = 0;
			nVertCount = 0;
			paVertices = NULL;
		}

		~KongMesh_t( void )
		{
			FreeMemory();
		}

		BOOL AllocateArrays( u32 nSegmentCount, u32 nMaterialCount, u32 nMatPropertiesCount, u32 nTriangleCount, u32 nVertCount );
		BOOL FreeMemory( void );
		void ComputeBoundingSphere( CFVec3 &Center, f32 &fRadius, BOOL bVolumeMesh = FALSE ) const;

		KongMatProperties_t* GetIdenticalProperties( KongMatProperties_t *pProperties );

		// Segment Material List functions
		KongMat_t* GetIdenticalMaterial( KongSeg_t *pSeg, KongMatProperties_t *pProperties );
		FINLINE KongMat_t* GetFirstMaterial( KongSeg_t *pSeg ) const;
		FINLINE KongMat_t* GetNextMaterial( KongMat_t *pMat ) const;
		void AddMatToSeg( KongSeg_t *pSeg, KongMat_t *pMat );
		BOOL RemoveMatFromSeg( KongSeg_t *pSeg, KongMat_t *pMat );
		BOOL VerifySegMatList( KongSeg_t *pSeg );
		void RemoveEmptyMaterials( void );

		// Material Triangle list functions
		void AddTriToMat( KongMat_t *pMat, KongTri_t *pTri );
		BOOL VerifyMatTriList( KongMat_t *pMat );
		FINLINE KongTri_t* GetFirstTri( KongMat_t *pMat ) const;
		FINLINE KongTri_t* GetNextTri( KongTri_t *pTri ) const;
		FINLINE KongTri_t* GetTriAtIdx( KongMat_t *pMat, u32 i ) const;

		// File IO
		u32  CreateFileAndSaveKong( cchar *pszFilename );
		u32  SaveKong( HANDLE hFile );
		u32  OpenFileAndLoadKong( cchar *pszFilename );
		u32  LoadKong( HANDLE hFile );
};


//
//
//
struct KongSeg_t 
{
	KongSeg_t( void )
	{
		nNumBonesUsed = 0;
		memset( anBoneID, 0, sizeof( u8 ) * FDATA_VW_COUNT_PER_VTX );
		nNumMats = 0;
		nFirstMatIdx = KONG_INVALID_MAT_INDEX;
		nLastMatIdx = KONG_INVALID_MAT_INDEX;
	}

	u32 nNumBonesUsed;
	u8 anBoneID[FDATA_VW_COUNT_PER_VTX];
	
	u32 nNumMats;
	s32 nFirstMatIdx;
	s32 nLastMatIdx;
};


//
//
//
struct KongLayer_t 
{
	char szTexnames[APE_LAYER_TEXTURE_MAX][TEXTURE_NAME_LEN];// no path or extension is included

	u8 nID;					// default = 255, manual assigned = 0-127, auto assigned = 128-254
	u8 __PAD;

	//	UV related cars
	BOOL8  abTile[2];		// [0] is U, [1] is V
	f32    fUVRotation;		// UV rotation speed in degrees per second (negative indicates counter-clock-wise)
	CFVec2 vUVRotAnchor;	// UV coord around which the UV's rotate (compressed unit value, no negative)
	f32    fDeltaUPerSec;	// how much should the u tc move per second
	f32    fDeltaVPerSec;	// how much should the v tc move per second

	// Aniamtion related vars
	u32 nNumTexFrames;		// how many texture frames are there
	f32 fFramesPerSecs;		// how many frames should be played per second
};


//
//
//
struct KongMatProperties_t 
{
	u32 nFlags;				// see APE_MAT_FLAGS_...

	BOOL8 bTransparent;
	BOOL8 bDrawAsWire;
	BOOL8 bSort;
	BOOL8 bTwoSided;		// tells PASM to generate 2 tris

	// *nocoll (legacy)
	BOOL8 bNoColl;			// default = FALSE, don't collide with this material

	// *motif
	BOOL8 bUseEmissiveColor; // should we use the emissive color
	BOOL8 bUseSpecularColor; // should we use the specular color
	BOOL8 bUseDiffuseColor;	 // should we use the diffuse color

	CFColorRGB EmissiveRGB;// aka emmissiveness, each element is a unit float
	CFColorRGB SpecularRGB;	// each element is a unit float
	CFColorRGB DiffuseRGB;	// each element is a unit float

	u16  nEmissiveMotifID;	// what is the emissive motif ID
	u16  nDiffuseMotifID;	// what is the diffuse motif ID
	u16  nSpecularMotifID;	// what is the specular motif ID

	// *Z
	u16 nZTugValue;			// 0 is no tug at all

	f32  fUnitAlphaMultiplier;// unit float, amount to scale vertex alpha when using some shader types

	// *bumptile
	f32  fBumpMapTileFactor;

	// *detailtile
	f32  fDetailMapTileFactor;

	// *order
	u16 nOrderNum;			// should we rearrange this material's order in the mesh, 0 means no rearranging

	// *shader
	s16 nShaderNum;			// what is the shader number, -1 means none was set

	// Specular related vars
	f32 fShininess;			// not a unit f32, represents an exponent, range(0.0f - 127.0f)
	f32 fShinStr;			// unit f32 (used to scale the specualar color)

	CFColorRGB  TintRGB;	// Color to tint the material
	CFColorRGBA LightRGBI;	// Light value for this surface (RGB, intensity), if used as a light surface that emits into the light map scene

	// Shader Values
	BOOL8 bEmissiveOn;
	BOOL8 bSpecularOn;
	BOOL8 bEnviroMotifOn;
	s8    nEnvironmentMap;
	s8    nTransparencyMask;
	s8    nSpecularMask;
	s16   nEmissiveMask;
	u16   nSurfaceShaderID;
	u16   nLightShaderID;
	u16   nSpecularShaderID;

	u16   nCollMask;		// see APE_MAT_COLL_FLAGS_, default APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING
	u16   nReactType;		// 0 - 7, default 0
	u16   nSurfaceType;	// 0 - 15, default 0

	// *collid (legacy)
	u16 nCollID;			// default = 0, manual assigned = 1-63

	// Layer specific material values from Max
	u16 nLayersUsed;
	KongLayer_t aLayer[KONG_MAX_LAYERS_PER_MAT];

	u8 nLODIndex;

	u8 nAffectAngle;	// For materials that have angular emissive or translucency

	u8 __PAD[2];

	void Set( ApeMaterial_t *pApeMat );
	BOOL Equals( KongMatProperties_t *pProperties, BOOL bMLCompare );
};


//
//
enum
{
	KONGMAT_FLAGS_NONE				= 0x00000000,
	KONGMAT_FLAGS_ADDED_TO_SEGMENT	= 0x00000001,
	KONGMAT_FLAGS_STATICALLY_LIT	= 0x00000002,
};

//
//
//
struct KongMat_t 
{
	KongMat_t( void )
	{
		pProperties = NULL;

		nNumTris = 0;
		nFirstTriIdx = KONG_INVALID_TRI_INDEX;
		nLastTriIdx = KONG_INVALID_TRI_INDEX;

		pKSeg = NULL;
		nLightMapCount = 0;

		memset( anLightMapMotifID, 0, FSH_MAX_LIGHTMAPS );
		memset( szLightMapTexname, 0, FSH_MAX_LIGHTMAPS * TEXTURE_NAME_LEN );

		nMatFlags = KONGMAT_FLAGS_NONE;
		nMatIdx = KONG_INVALID_MAT_INDEX;
		nNextMatIdx = KONG_INVALID_MAT_INDEX;
	}

	KongMatProperties_t *pProperties;

	u16 nMLMeshMatIndex;  // Used to record the index of this material in the MLMesh's material list

	u16 nNumTris;
	u32 nFirstTriIdx;
	u32 nLastTriIdx;
	
	KongSeg_t *pKSeg;// what kong segment is this material apart of
	
	// LightMap names
	u16  nLightMapCount;
	u16  anLightMapMotifID[FSH_MAX_LIGHTMAPS];
	char szLightMapTexname[FSH_MAX_LIGHTMAPS][TEXTURE_NAME_LEN];// no path or extension is included

	u32 nMatFlags;	// this field can be used by any functions to mark when a Mat has been added
					// to some process, other functions may reset this value for their own use.	

	s16 nMatIdx; // This material's index in the mesh material list
	s16 nNextMatIdx; // The index to the next material in the segment

	//
	//
	BOOL Equals( KongMat_t *pMat, BOOL bMLCompare )
	{
		if ( nLightMapCount != pMat->nLightMapCount )
		{
			return FALSE;
		}

		u32 i;
		for ( i = 0; i < nLightMapCount; i++ )
		{
			if (   anLightMapMotifID[i] != pMat->anLightMapMotifID[i] 
				|| stricmp( szLightMapTexname[i], pMat->szLightMapTexname[i] ) != 0 )
			{
				return FALSE;
			}
		}

		FASSERT( pProperties && pMat->pProperties );
		if ( pProperties == pMat->pProperties )
		{
			return TRUE;
		}
		return pProperties->Equals( pMat->pProperties, bMLCompare );
	}

	//
	//
	void SetWithEmptyTris( KongMat_t *pMat )
	{
		memcpy( this, pMat, sizeof( KongMat_t ) );

		nNumTris = 0;
		nFirstTriIdx = KONG_INVALID_TRI_INDEX;
		nLastTriIdx = KONG_INVALID_TRI_INDEX;

	}

	void SetShaderTypes( void );
};


//
//
//
struct KongVert_t 
{
	CFVec3 Pos;
	CFVec3 Norm;
	//NBTs are here.
	CFVec3 Binorm, Tangent;
	//
	u8 nBaseUVCount; // Number of active UV's (only accurate in the MLMesh library)
	u8 nLightMapUVCount; // Number of active UV's (only accurate in the MLMesh Library)
	u8 nNumWeights;	// will be > 0.0f if the segment bSkinned == TRUE
	u8 __PAD;

	u32 nMeshVertIdx; // Index of this vert in the mesh's Ape vert array

	CFColorRGBA Color;	// each element is a unit float
	CFVec2 aUV[KONG_MAX_UVS_PER_MAT];

	// Weight information
	s8  nWeightBoneIdx[MAX_WEIGHTS_PER_VERT];
	f32 fWeight[MAX_WEIGHTS_PER_VERT];// the first nNumWeights elements will be used

	//
	//
	void Set( ApeVert_t *pApeVert )
	{
		Pos = pApeVert->Pos;
		Norm = pApeVert->Norm;
		Color = pApeVert->Color;
		for ( u32 i = 0; i < MAX_LAYERS_PER_MAT; i++ )
		{
			aUV[i] = pApeVert->aUV[i];
		}
		nNumWeights = (u8)pApeVert->fNumWeights;
		for ( u32 i = 0; i < MAX_WEIGHTS_PER_VERT; i++ )
		{
			nWeightBoneIdx[i] = (s8)pApeVert->aWeights[i].fBoneIndex;
			fWeight[i] = pApeVert->aWeights[i].fWeight;
		}
	}
};


//
//
enum
{
	KONGTRI_FLAG_TRI_ADDED_TO_SEGMENT	= 0x01,
	KONGTRI_FLAG_TRI_ADDED_TO_VOLUME	= 0x02,
	KONGTRI_FLAG_TRI_ADDED_TO_MATERIAL	= 0x04,
	KONGTRI_FLAG_BACKFACE				= 0x80,	// This tri was created as a backface for a material that was flagged as desiring backfaces
};


//
//
//
struct KongTri_t 
{
	KongVert_t *apKongVerts[3];

	u8    nNumBonesUsed;
	u8    nNumEdgeTris;		 // how many valid edge tris are there, (valid values are 0,1,2,3)	
	u8	  nTriFlags;

	u8 anBoneID[FDATA_VW_COUNT_PER_VTX];
	
	CFVec3 FaceUnitNorm;
	CFSphere BoundSphere;

	KongTri_t *paEdgeTris[3];// these are Kong tris (in the same KongMat) that share edges with this tri

	u32 nTriIdx;		// This triangles index in paTriangles for the KongMesh
	u32 nNextTriIdx;	// Index to the next triangle in the material tri list

	// some user data that may be used by different modules
	f32 fDist2;	
	u16 nUser[2];
	void *pUser;
};


//
//
//
FINLINE KongMat_t* KongMesh_t::GetFirstMaterial( KongSeg_t *pSeg ) const
{
	FASSERT( pSeg );

	if ( pSeg->nFirstMatIdx == KONG_INVALID_MAT_INDEX )
	{
		return NULL;
	}

	FASSERT( pSeg->nFirstMatIdx < (s32)nMaterialCount );
	return &paMaterials[pSeg->nFirstMatIdx];
}


//
//
//
FINLINE KongMat_t* KongMesh_t::GetNextMaterial( KongMat_t *pMat ) const
{
	FASSERT( pMat );

	if ( pMat->nNextMatIdx == KONG_INVALID_MAT_INDEX )
	{
		return NULL;
	}

	FASSERT( pMat->nNextMatIdx < (s32)nMaterialCount );
	return &paMaterials[pMat->nNextMatIdx];
}


//
//
//
FINLINE KongTri_t* KongMesh_t::GetFirstTri( KongMat_t *pMat ) const
{
	FASSERT( pMat );

	if ( pMat->nFirstTriIdx == KONG_INVALID_TRI_INDEX )
	{
		return NULL;
	}

	FASSERT( pMat->nFirstTriIdx < (s32)nTriangleCount );
	return &paTriangles[pMat->nFirstTriIdx];
}


//
//
//
FINLINE KongTri_t* KongMesh_t::GetNextTri( KongTri_t *pTri ) const 
{
	FASSERT( pTri );

	if ( pTri->nNextTriIdx == KONG_INVALID_TRI_INDEX )
	{
		return NULL;
	}

	FASSERT( pTri->nNextTriIdx < nTriangleCount );
	return &paTriangles[pTri->nNextTriIdx];
}



//
//
//
FINLINE KongTri_t* KongMesh_t::GetTriAtIdx( KongMat_t *pMat, u32 nIdx ) const 
{
	FASSERT( pMat );

	u32 nCounter = 0;
	KongTri_t *pTri = GetFirstTri( pMat );
	while ( pTri )
	{
		if ( nCounter == nIdx )
		{
			return pTri;
		}
		nCounter++;
		pTri = GetNextTri( pTri );
	}

	return NULL;
}



/////////////////////////////////////////////////////////////////////////////////////////////


#endif

