//////////////////////////////////////////////////////////////////////////////////////
// Convert.cpp - 
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
// 04/20/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "Convert.h"
#include "ape_file_def.h"
#include "OldApeFileLoader.h"

CConvertApeFile::CConvertApeFile() {
}

CConvertApeFile::~CConvertApeFile() {
}

BOOL CConvertApeFile::Convert( COldApeFileLoader &rOldApeFile, cchar *pszFilename ) {
	FILE *pFileStream;
	ApeMesh_t Mesh;
	ApeBone_t Bone;
	ApeLight_t Light;
	ApeObject_t Obj;
	ApeFog_t Fog;
	ApeShape_t Shape;
	ApeVisVolume_t Volume;
	ApeVisPortal_t Portal;
	ApeSegment_t Segment;
	ApeMaterial_t Material;
	ApeVert_t Vert;
	ApeVertIndex_t VIndex;
	
	OldApeMesh_t *pOldMesh;
	OldApeBone_t *pOldBone;
	OldApeLight_t *pOldLight;
	OldApeObject_t *pOldObj;
	OldApeFog_t *pOldFog;
	OldApeShape_t *pOldShape;
	OldApeVisVolume_t *pOldVolume;
	OldApeVisPortal_t *pOldPortal;
	OldApeSegment_t *pOldSegment;
	OldApeMaterial_t *pOldMaterial;
	OldApeVert_t *pOldVert;
	OldApeVertIndex_t *pOldVIndex;
	u32 i, j;
	
	if( !pszFilename ) {
		return FALSE;
	}
	/////////////////////////////////
	// open the new file for writting
	pFileStream = _tfopen( pszFilename, _T("wb") );
	if( !pFileStream ) {
		return FALSE;
	}
	////////////////////////////
	// write out the mesh header
	pOldMesh = rOldApeFile.GetMeshInfo();
	ConvertStruct( &Mesh, pOldMesh );
	// compute our file size	
	Mesh.nBytesInFile += (Mesh.nNumBoneNames * sizeof( ApeBone_t ) ) + 
						 (Mesh.nNumLights * sizeof( ApeLight_t ) );
	for( i=0; i < Mesh.nNumObjects; i++ ) {
		pOldObj = rOldApeFile.GetObjectInfo( i );
		Mesh.nBytesInFile += sizeof( ApeObject_t ) + pOldObj->nBytesOfUserData;
	}
	for( i=0; i < Mesh.nNumShapes; i++ ) {
		pOldShape = rOldApeFile.GetShapeInfo( i );
		Mesh.nBytesInFile += sizeof( ApeShape_t ) + pOldShape->nBytesOfUserData;
	}
	Mesh.nBytesInFile += (Mesh.nNumFogs * sizeof( ApeFog_t ) ) + 
						 (Mesh.nNumVisVolumes * sizeof( ApeVisVolume_t ) ) +
						 (Mesh.nNumVisPortals * sizeof( ApeVisPortal_t ) ) +
						 (Mesh.nNumSegments * sizeof( ApeSegment_t ) ) + 
						 (rOldApeFile.GetNumMeshMaterials() * sizeof( ApeMaterial_t ) ) + 
						 (rOldApeFile.GetNumMeshVerts() * sizeof( ApeVert_t ) ) +
						 (rOldApeFile.GetNumMeshIndices() * sizeof( ApeVertIndex_t ) );
	fwrite( &Mesh, sizeof( ApeMesh_t ), 1, pFileStream );
	
	//////////////////////
	// write out the bones
	for( i=0; i < Mesh.nNumBoneNames; i++ ) {
		pOldBone = rOldApeFile.GetMeshBone( i );
		ConvertStruct( &Bone, pOldBone );
		fwrite( &Bone, sizeof( ApeBone_t ), 1, pFileStream );
	}
	
	///////////////////////
	// write out the lights
	for( i=0; i < Mesh.nNumLights; i++ ) {
		pOldLight = rOldApeFile.GetLightInfo( i );
		ConvertStruct( &Light, pOldLight );
		fwrite( &Light, sizeof( ApeLight_t ), 1, pFileStream );
	}
	
	////////////////////////
	// write out the objects
	for( i=0; i < Mesh.nNumObjects; i++ ) {
		pOldObj = rOldApeFile.GetObjectInfo( i );
		ConvertStruct( &Obj, pOldObj );
		fwrite( &Obj, sizeof( ApeObject_t ), 1, pFileStream );
		if( Obj.nBytesOfUserData > 0 ) {
			fwrite( &pOldObj[1], Obj.nBytesOfUserData, 1, pFileStream );
		}
	}
	
	/////////////////////
	// write out the fogs
	for( i=0; i < Mesh.nNumFogs; i++ ) {
		pOldFog = rOldApeFile.GetFogInfo( i );
		ConvertStruct( &Fog, pOldFog );
		fwrite( &Fog, sizeof( ApeFog_t ), 1, pFileStream );		
	}

	///////////////////////
	// write out the shapes
	for( i=0; i < Mesh.nNumShapes; i++ ) {
		pOldShape = rOldApeFile.GetShapeInfo( i );
		ConvertStruct( &Shape, pOldShape );
		fwrite( &Shape, sizeof( ApeShape_t ), 1, pFileStream );
		if( Shape.nBytesOfUserData > 0 ) {
			fwrite( &pOldShape[1], Shape.nBytesOfUserData, 1, pFileStream );
		}
	}

	////////////////////////////
	// write out the vis volumes
	for( i=0; i < Mesh.nNumVisVolumes; i++ ) {
		pOldVolume = rOldApeFile.GetVisVol( i );
		ConvertStruct( &Volume, pOldVolume );
		fwrite( &Volume, sizeof( ApeVisVolume_t ), 1, pFileStream );
	}

	////////////////////////////
	// write out the vis portals
	for( i=0; i < Mesh.nNumVisPortals; i++ ) {
		pOldPortal = rOldApeFile.GetVisPortal( i );
		ConvertStruct( &Portal, pOldPortal );
		fwrite( &Portal, sizeof( ApeVisPortal_t ), 1, pFileStream );
	}

	/////////////////////////
	// write out the segments
	for( i=0; i < Mesh.nNumSegments; i++ ) {
		pOldSegment = rOldApeFile.GetSegmentInfo( i );
		ConvertStruct( &Segment, pOldSegment );
		fwrite( &Segment, sizeof( ApeSegment_t ), 1, pFileStream );
		// write out the materials
		for( j=0; j < Segment.nNumMaterials; j++ ) {
			pOldMaterial = rOldApeFile.GetMaterialInfo( i, j );
			ConvertStruct( &Material, pOldMaterial );
			fwrite( &Material, sizeof( ApeMaterial_t ), 1, pFileStream );
		}
		// write out the verts
		pOldVert = rOldApeFile.GetVertData( i );
		for( j=0; j < Segment.nNumVerts; j++ ) {
			ConvertStruct( &Vert, &pOldVert[j] );
			fwrite( &Vert, sizeof( ApeVert_t ), 1, pFileStream );
		}
		// write out the vert indices
		pOldVIndex = rOldApeFile.GetVertIndex( i, 0 );
		for( j=0; j < Segment.nNumIndices; j++ ) {
			ConvertStruct( &VIndex, &pOldVIndex[j] );
			fwrite( &VIndex, sizeof( ApeVertIndex_t ), 1, pFileStream );
		}
	}
	
	// close our file
	fclose( pFileStream );
	
	return TRUE;
}

void CConvertApeFile::ConvertStruct( ApeMesh_t *pNew, OldApeMesh_t *pOld ) {
	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeMesh_t ) );
	
	pNew->Header.nSignature = FVERSION_FILE_SIGNATURE;
	pNew->Header.nVersion = FVERSION_APE_VERSION;
	pNew->nBytesInFile = sizeof( ApeMesh_t );
	strncpy( pNew->szMeshName, pOld->szMeshName, MESH_NAME_LEN-1 );
	pNew->bWorldFile = pOld->bWorldFile;
	pNew->nNumBoneNames = pOld->nNumBoneNames;
	pNew->nNumVisVolumes = pOld->nNumVisVolumes;
	pNew->nNumLights = pOld->nNumLights;
	pNew->nNumVisPortals = pOld->nNumVisPortals;
	pNew->nNumObjects = pOld->nNumObjects;
	pNew->nNumFogs = pOld->nNumFogs;
	pNew->nNumSegments = pOld->nNumSegments;
	pNew->nNumShapes = pOld->nNumShapes;
	pNew->nSizeOfBoneStruct = sizeof( ApeBone_t );
	pNew->nSizeOfLightStruct = sizeof( ApeLight_t );
	pNew->nSizeOfObjectStruct = sizeof( ApeObject_t );
	pNew->nSizeOfFogStruct = sizeof( ApeFog_t );
	pNew->nSizeOfSegmentStruct = sizeof( ApeSegment_t );
	pNew->nSizeOfMaterialStruct = sizeof( ApeMaterial_t );
	pNew->nSizeOfVertStruct = sizeof( ApeVert_t );
	pNew->nSizeOfVertIndexStruct = sizeof( ApeVertIndex_t );
	pNew->nNumShapes = pOld->nNumShapes;
	pNew->nSizeOfShapeStruct = sizeof( ApeShape_t );
}

void CConvertApeFile::ConvertStruct( ApeBone_t *pNew, OldApeBone_t *pOld ) {
	u32 i;

	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeBone_t ) );
	
	strncpy( pNew->szBoneName, pOld->szBoneName, BONE_NAME_LEN-1 );
	pNew->nFlags = pOld->nFlags;
	pNew->nBoneIndex = pOld->nBoneIndex;
	pNew->nParentIndex = pOld->nParentIndex;
	pNew->AtRestModelToBoneMtx = pOld->AtRestModelToBoneMtx;
	pNew->nNumChildren = pOld->nNumChildren;
	for( i=0; i < pNew->nNumChildren; i++ ) {
		pNew->auChildIndices[i] = pOld->auChildIndices[i];
	}
}

//
//
//
FINLINE void _BuildLightMatrixFromDirection( CFMtx43 *pMtx, f32 fX, f32 fY, f32 fZ )
{
	pMtx->m_vFront.x = fX;
	pMtx->m_vFront.y = fY;
	pMtx->m_vFront.z = fZ;
	pMtx->m_vFront.Unitize();

	if ( fmath_Abs( pMtx->m_vFront.y) > 0.999f )
	{
		pMtx->m_vRight = CFVec3::m_UnitAxisX;
		pMtx->m_vUp = -CFVec3::m_UnitAxisZ;
	}
	else
	{
		pMtx->m_vRight = CFVec3::m_UnitAxisY.Cross( pMtx->m_vFront );
		pMtx->m_vRight.Unitize();
		pMtx->m_vUp = pMtx->m_vFront.Cross( pMtx->m_vRight );
	}
	pMtx->m_vPos.Set( 0.f, 0.f, 0.f );
}

void CConvertApeFile::ConvertStruct( ApeLight_t *pNew, OldApeLight_t *pOld ) {
	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeLight_t ) );
	
	pNew->nType = (ApeLightType_e)pOld->nType;
	strncpy( pNew->szLightName, pOld->szLightName, LIGHT_NAME_LEN-1 );	
	pNew->Sphere = pOld->Sphere;
	pNew->Dir = pOld->Dir;
	pNew->Color = pOld->Color;
	pNew->fIntensity = pOld->fIntensity;
	pNew->fSpotInnerAngle = pOld->fSpotInnerAngle;
	pNew->fSpotOuterAngle = pOld->fSpotOuterAngle;
	pNew->nFlags = pOld->nFlags;
	pNew->nMotifID = pOld->nMotifID;
	pNew->nLightID = 0xffff;
	pNew->fCoronaScale = 1.f;
	if ( pOld->nType == APE_LIGHT_TYPE_SPOT ||  pOld->nType == APE_LIGHT_TYPE_DIR )
	{
		_BuildLightMatrixFromDirection( &pNew->mtxOrientation, pOld->Dir.x, pOld->Dir.y, pOld->Dir.z ) ; // Orientation of the light
	}
	else
	{
		pNew->mtxOrientation.Identity();
	}

	pNew->szCoronaTexture[0] = 0;
	pNew->szPerPixelTexture[0] = 0;
}

void CConvertApeFile::ConvertStruct( ApeObject_t *pNew, OldApeObject_t *pOld ) {
	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeObject_t ) );
	
	strncpy( pNew->szName, pOld->szName, OBJECT_NAME_LEN-1 );	
	pNew->nFlags = pOld->nFlags;
	pNew->Orientation = pOld->Orientation;
	pNew->nBytesOfUserData = pOld->nBytesOfUserData;	
	pNew->fCullDistance = pOld->fCullDistance;
	pNew->nParentIndex = pOld->nParentIndex;	
}

void CConvertApeFile::ConvertStruct( ApeFog_t *pNew, OldApeFog_t *pOld ) {
	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeFog_t ) );

	pNew->fStartDist = pOld->fStartDist;
	pNew->fEndDist = pOld->fEndDist;
	pNew->fUnitDensity = pOld->fUnitDensity;
	pNew->Color = pOld->Color;
	pNew->nMotifID = pOld->nMotifID;
}

void CConvertApeFile::ConvertStruct( ApeShape_t *pNew, OldApeShape_t *pOld ) {
	FASSERT( sizeof( ApeShape_t ) == sizeof( OldApeShape_t ) );

	// since the struct haven't changed size and are the same format, just copy them
	fang_MemCopy( pNew, pOld, sizeof( ApeShape_t ) );	
}

void CConvertApeFile::ConvertStruct( ApeVisVolume_t *pNew, OldApeVisVolume_t *pOld ) {
	FASSERT( sizeof( ApeVisVolume_t ) == sizeof( OldApeVisVolume_t ) );

	// since the struct haven't changed size and are the same format, just copy them
	fang_MemCopy( pNew, pOld, sizeof( ApeVisVolume_t ) );
}

void CConvertApeFile::ConvertStruct( ApeVisPortal_t *pNew, OldApeVisPortal_t *pOld ) {
	FASSERT( sizeof( ApeVisPortal_t ) == sizeof( OldApeVisPortal_t ) );

	// since the struct haven't changed size and are the same format, just copy them
	fang_MemCopy( pNew, pOld, sizeof( ApeVisPortal_t ) );
}

void CConvertApeFile::ConvertStruct( ApeSegment_t *pNew, OldApeSegment_t *pOld ) {
	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeSegment_t ) );
	
	strncpy( pNew->szNodename, pOld->szNodename, SEGMENT_NAME_LEN-1 );
	pNew->bSkinned = pOld->bSkinned;
	pNew->nNumMaterials = pOld->nNumMaterials;
	pNew->nNumVerts = pOld->nNumVerts;
	pNew->nNumIndices = pOld->nNumIndices;
}

void CConvertApeFile::ConvertStruct( ApeMaterial_t *pNew, OldApeMaterial_t *pOld ) {
	u32 i;

	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeMaterial_t ) );

	pNew->nLayersUsed = pOld->nLayersUsed;
	for( i=0; i < pNew->nLayersUsed; i++ ) {
		pNew->aLayer[i].bTextured = pOld->aLayer[i].bTextured;
		strncpy( pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_DIFFUSE], pOld->aLayer[i].szTexname, TEXTURE_NAME_LEN-1 );
		strncpy( pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_SPECULAR_MASK], pOld->aLayer[i].szSpecularTexname, TEXTURE_NAME_LEN-1 );
		strncpy( pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_EMISSIVE_MASK], pOld->aLayer[i].szSelfIllumTexname, TEXTURE_NAME_LEN-1 );
		strncpy( pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_ALPHA_MASK], pOld->aLayer[i].szOpacityTexname, TEXTURE_NAME_LEN-1 );
		strncpy( pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_BUMP], pOld->aLayer[i].szBumpTexname, TEXTURE_NAME_LEN-1 );
		strncpy( pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT], pOld->aLayer[i].szReflectivityTexname, TEXTURE_NAME_LEN-1 );
		strncpy( pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_DETAIL], pOld->aLayer[i].szDisplacementTexname, TEXTURE_NAME_LEN-1 );
		pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_UNUSED_3][0] = 0;
		pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_UNUSED_2][0] = 0;
		pNew->aLayer[i].szTexnames[APE_LAYER_TEXTURE_UNUSED_1][0] = 0;
		pNew->aLayer[i].fUnitAlphaMultiplier = pOld->aLayer[i].fUnitAlphaMultiplier;
		pNew->aLayer[i].bDrawAsWire = pOld->aLayer[i].bDrawAsWire;
		pNew->aLayer[i].bTwoSided = pOld->aLayer[i].bTwoSided;
		pNew->aLayer[i].abTile[0] = pOld->aLayer[i].abTile[0];
		pNew->aLayer[i].abTile[1] = pOld->aLayer[i].abTile[1];
		pNew->aLayer[i].SpecularRGB = pOld->aLayer[i].SpecularRGB;
		pNew->aLayer[i].SelfIllumRGB = pOld->aLayer[i].SelfIllumRGB;
		pNew->aLayer[i].DiffuseRGB = pOld->aLayer[i].DiffuseRGB;
		pNew->aLayer[i].fShininess = pOld->aLayer[i].fShininess;
		pNew->aLayer[i].fShinStr = pOld->aLayer[i].fShinStr;
		ConvertStruct( &pNew->aLayer[i].StarCommands, &pOld->aLayer[i].StarCommands );
		pNew->aLayer[i].nFlags = pOld->aLayer[i].nFlags;
	}
	pNew->nFirstIndex = pOld->nFirstIndex;
	pNew->nNumIndices = pOld->nNumIndices;
	ConvertStruct( &pNew->StarCommands, &pOld->StarCommands );
}

void CConvertApeFile::ConvertStruct( ApeVert_t *pNew, OldApeVert_t *pOld ) {
	u32 i;

	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeVert_t ) );

	pNew->Pos = pOld->Pos;
	pNew->Norm = pOld->Norm;
	pNew->Color = pOld->Color;
	for( i=0; i < OLD_MAX_LAYERS_PER_MAT; i++ ) {
		pNew->aUV[i] = pOld->aUV[i];
	}
	pNew->fNumWeights = pOld->fNumWeights;
	for( i=0; i < (u32)pOld->fNumWeights; i++ ) {
		pNew->aWeights[i].fBoneIndex = pOld->aWeights[i].fBoneIndex;
		pNew->aWeights[i].fWeight = pOld->aWeights[i].fWeight;
	}
}

void CConvertApeFile::ConvertStruct( ApeVertIndex_t *pNew, OldApeVertIndex_t *pOld ) {
	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeVertIndex_t ) );

	pNew->nVertIndex = pOld->nVertIndex;
}

void CConvertApeFile::ConvertStruct( ApeCommands_t *pNew, OldApeCommands_t *pOld ) {
	// zero out the new struct
	fang_MemZero( pNew, sizeof( ApeCommands_t ) );
	
	pNew->bSort = pOld->bSort;
	pNew->nOrderNum = pOld->nOrderNum;
	pNew->nShaderNum = pOld->nShaderNum;
	pNew->nEmissiveMotifID = pOld->nEmissiveMotifID;
	pNew->nSpecularMotifID = pOld->nSpecularMotifID;
	pNew->nDiffuseMotifID = pOld->nDiffuseMotifID;
	pNew->bUseEmissiveColor = pOld->bUseEmissiveColor;
	pNew->bUseSpecularColor = pOld->bUseSpecularColor;
	pNew->bUseDiffuseColor = pOld->bUseDiffuseColor;
	pNew->nNumTexFrames = pOld->nNumTexFrames;
	pNew->fFramesPerSecs = pOld->fFramesPerSecs;
	pNew->fDeltaUPerSec = pOld->fDeltaUPerSec;
	pNew->fDeltaVPerSec = pOld->fDeltaVPerSec;
	pNew->nZTugValue = pOld->nZTugValue;
	pNew->nID = pOld->nID;
	pNew->bNoColl = pOld->bNoColl;
	pNew->nCollID = pOld->nCollID;
	pNew->nFlags = pOld->nFlags;
	pNew->nCollMask = pOld->nCollMask | 0xff00;
	pNew->nReactType = pOld->nReactType;
	pNew->nSurfaceType = pOld->nSurfaceType;
	pNew->TintRGB.Set( 1.f, 1.f, 1.f );
	pNew->LightRGBI.Set( 0.f, 0.f, 0.f, 0.f );
}
