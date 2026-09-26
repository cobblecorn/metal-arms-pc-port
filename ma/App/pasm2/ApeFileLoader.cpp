//////////////////////////////////////////////////////////////////////////////////////
// ApeFileLoader.cpp - 
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
#include "stdafx.h"
#include "ApeFileLoader.h"
#include "ErrorLog.h"
#include "fversion.h"
#include "fclib.h"

#define _ERROR_HEADING			"APE FILE LOADER "

#define _NO_WORLD_FILES			0	// set to 1 to turn all ape files into non-world files, 0 will load the ape file regularly

typedef enum {
	NONE = 0,
	NO_FILENAME,
	NO_HEADER,
	COULD_NOT_OPEN_FILE,
	INVALID_FILE_TYPE,
	COULD_NOT_READ_ENTIRE_FILE,
	NOT_ENOUGH_MEMORY,
	NO_FILE_LOADED,
	INVALID_SEGMENT_INDEX,
	INVALID_MATERIAL_INDEX,
	INVALID_VERT_INDEX,
	INVALID_INDICE_INDEX,
	FILE_LOADED_OK,
	INVALID_MATERIAL_POINTER,
	INVALID_LIGHT_INDEX,
	INVALID_BONE_INDEX,
	INVALID_OBJECT_INDEX,
	INVALID_FOG_INDEX,
	INVALID_SHAPE_INDEX,
	INVALID_VOL_INDEX,
	INVALID_PORTAL_INDEX,

	NUM_ERROR_CODES
} _ErrorCodes_e;

CApeFileLoader::CApeFileLoader() {
	m_pAllocMem = NULL;
	m_pMeshInfo = NULL;
	m_nNumBones = 0;
	m_pBones = NULL;
	m_nNumLights = 0;
	m_pLights = NULL;
	
	m_nNumObjects = 0;
	m_aObjects.RemoveAll();
	
	m_nNumSegments = 0;
	m_aSegments.RemoveAll();
	
	m_aMaterials.RemoveAll();
	UpdateErrorString( NONE, m_sLastError );
	m_sLastFileLoaded = "";

	m_nNumShapes = 0;
	m_aShapes.RemoveAll();

	m_nNumVisVols = 0;
	m_aVisVols.RemoveAll();

	m_nNumVisPortals = 0;
	m_aVisPortals.RemoveAll();

	m_nFileCRCValue = m_nAIDFileCRCValue = 0;
}

CApeFileLoader::~CApeFileLoader() {
	UnloadFile();
}

BOOL CApeFileLoader::LoadAndValidateApeHeader( cchar *pszFilename, ApeMesh_t &rMeshInfo,
											   CString *psLastError/*=NULL*/, BOOL bLogErrors/*=FALSE*/ ) {
	CString sErrorHeading, s;
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	
	if( bLogErrors ) {
		sErrorHeading = _ERROR_HEADING;
		sErrorHeading += pszFilename;
	}
	// make sure we have a valid filename
	if( !pszFilename ) {
		if( bLogErrors ) {
			UpdateErrorString( NO_FILENAME, *psLastError );
			rErrorLog.WriteErrorHeader( sErrorHeading );
			rErrorLog.WriteErrorLine( *psLastError );
		}
		return FALSE;
	}

	// Open the filestream for reading out of
	FILE *pFileStream = _tfopen( pszFilename, _T("rb") );
	if( !pFileStream ) {
		if( bLogErrors ) {
			UpdateErrorString( COULD_NOT_OPEN_FILE, *psLastError );
			rErrorLog.WriteErrorHeader( sErrorHeading );
			rErrorLog.WriteErrorLine( *psLastError );
		}
		return FALSE;
	}

	// read the header in
	int nItemsRead = fread( &rMeshInfo, sizeof( ApeMesh_t ), 1, pFileStream );
	fclose( pFileStream );
	if( nItemsRead != 1 ) {		
		if( bLogErrors ) {
			UpdateErrorString( NO_HEADER, *psLastError );
			rErrorLog.WriteErrorHeader( sErrorHeading );
			rErrorLog.WriteErrorLine( *psLastError );
		}
		return FALSE;
	}

	// verify a valid .ape file
	if( rMeshInfo.Header.nSignature != FVERSION_FILE_SIGNATURE || rMeshInfo.nBytesInFile == 0 ) {
		// invalid file
		if( bLogErrors ) {
			UpdateErrorString( INVALID_FILE_TYPE, *psLastError );
			rErrorLog.WriteErrorHeader( sErrorHeading );
			rErrorLog.WriteErrorLine( *psLastError );
		}
		return FALSE;
	}

	if( rMeshInfo.Header.nVersion != FVERSION_APE_VERSION ) {
		// invalid file version
		if( bLogErrors ) {
			UpdateErrorString( INVALID_FILE_TYPE, *psLastError );
			rErrorLog.WriteErrorHeader( sErrorHeading );
			s.Format( "Invalid ape file version( %d.%d.%d ), PASM needs version( %d.%d.%d )",
				FVERSION_GET_MAJOR_NUMBER( rMeshInfo.Header.nVersion ),
				FVERSION_GET_MINOR_NUMBER( rMeshInfo.Header.nVersion ),
				FVERSION_GET_SUB_NUMBER( rMeshInfo.Header.nVersion ), 
				FVERSION_APE_MAJOR, FVERSION_APE_MINOR, FVERSION_APE_SUB );
			rErrorLog.WriteErrorLine( s );
		}
		return FALSE;
	}

	return TRUE;
}
 
BOOL CApeFileLoader::LoadApeFile( cchar *pszFilename ) {

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	CString sErrorHeading = _ERROR_HEADING;
	sErrorHeading += pszFilename;
	CString s;

	// unload any file that may be loaded right now
	UnloadFile();

	ApeMesh_t MeshInfo;
	if( !LoadAndValidateApeHeader( pszFilename, MeshInfo, &m_sLastError, TRUE ) ) {
		return FALSE;
	}
	// allocate memory for the entire file
	m_pAllocMem = new char[MeshInfo.nBytesInFile];
	if( !m_pAllocMem ) {
		UpdateErrorString( NOT_ENOUGH_MEMORY, m_sLastError );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( m_sLastError );
		return FALSE;
	}
	// Open the filestream for reading out of
	FILE *pFileStream = _tfopen( pszFilename, _T("rb") );
	if( !pFileStream ) {
		UpdateErrorString( COULD_NOT_OPEN_FILE, m_sLastError );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( m_sLastError );
		return FALSE;
	}	
	// read the entire file into our allocated memory
	int nItemsRead = fread( m_pAllocMem, sizeof( char ), MeshInfo.nBytesInFile, pFileStream );
	fclose( pFileStream );// file is not longer needed, so close it
	if( nItemsRead != (int)MeshInfo.nBytesInFile ) {
		delete[] (char *)m_pAllocMem;
		m_pAllocMem = NULL;
		UpdateErrorString( COULD_NOT_READ_ENTIRE_FILE, m_sLastError );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( m_sLastError );
		return FALSE;
	}

	m_nFileCRCValue = fmath_Crc32( 0, (u8 *)m_pAllocMem, MeshInfo.nBytesInFile ); 

	// finally fixup our pointers
	if( !FixupPointers() ) {
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( m_sLastError );
		return FALSE;
	}
	
	// fixup invalid st coordinates
	if( !FixupInvalidSTCoords( sErrorHeading ) ) {
		// error has already been logged, don't fail
//		return FALSE;
	}

	UpdateErrorString( FILE_LOADED_OK, m_sLastError );
	
	m_sLastFileLoaded = pszFilename;
	
	return TRUE;
}

void CApeFileLoader::UnloadFile() {

	if( m_pAllocMem ) {
		delete[] (char *)m_pAllocMem;
		m_pAllocMem = NULL;		
	}
	m_pMeshInfo = NULL;
	m_nNumBones = 0;
	m_pBones = NULL;
	m_nNumLights = 0;
	m_pLights = NULL;
	m_nNumObjects = 0;
	m_aObjects.RemoveAll();
	m_aSegments.RemoveAll();
	m_aMaterials.RemoveAll();
	m_aVerts.RemoveAll();
	m_aVertIndices.RemoveAll();
	m_nNumSegments = 0;
	m_pFog = NULL;
	m_sLastFileLoaded = "";
	UpdateErrorString( NONE, m_sLastError );
	m_nNumShapes = 0;
	m_aShapes.RemoveAll();
	m_nNumVisVols = 0;
	m_aVisVols.RemoveAll();
	m_nNumVisPortals = 0;
	m_aVisPortals.RemoveAll();
}

BOOL CApeFileLoader::CloneFromLoadedFile( const CApeFileLoader &rFile ) {
	
	UnloadFile();

	// we have to allocate some memory to have all of the function behave correctly
	m_pAllocMem = new char[4];
	if( !m_pAllocMem ) {
		return FALSE;
	}

	m_pMeshInfo = rFile.m_pMeshInfo;
	m_nNumBones = rFile.m_nNumBones;
	m_pBones = rFile.m_pBones;
	m_nNumLights = rFile.m_nNumLights;
	m_pLights = rFile.m_pLights;
	m_nNumObjects = rFile.m_nNumObjects;
	m_aObjects.Copy( rFile.m_aObjects );
	m_pFog = rFile.m_pFog;
	m_nNumShapes = rFile.m_nNumShapes;
	m_aShapes.Copy( rFile.m_aShapes );
	m_nNumVisVols = rFile.m_nNumVisVols;
	m_aVisVols.Copy( rFile.m_aVisVols );
	m_nNumVisPortals = rFile.m_nNumVisPortals;
	m_aVisPortals.Copy( rFile.m_aVisPortals );
	m_nNumSegments = rFile.m_nNumSegments;
	m_aSegments.Copy( rFile.m_aSegments );
	m_aMaterials.Copy( rFile.m_aMaterials );
	m_aVerts.Copy( rFile.m_aVerts );
	m_aVertIndices.Copy( rFile.m_aVertIndices );

	return TRUE;
}

ApeMesh_t * CApeFileLoader::GetMeshInfo() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	return m_pMeshInfo;
}

u32 CApeFileLoader::GetNumMeshBones() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return FALSE;
	}
	return m_nNumBones;
}

u32 CApeFileLoader::GetNumLights() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	return m_nNumLights;
}

u32 CApeFileLoader::GetNumObjects() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	return m_nNumObjects;
}

u32 CApeFileLoader::GetNumFogs() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	return m_pMeshInfo->nNumFogs;
}

u32 CApeFileLoader::GetNumShapes() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	return m_nNumShapes;
}

u32 CApeFileLoader::GetNumVisVols() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	return m_nNumVisVols;
}

u32 CApeFileLoader::GetNumVisPortals() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	return m_nNumVisPortals;
}

u32 CApeFileLoader::GetNumVisCells() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	u32 i, nNumCells( 0 );
	ApeVisVolume_t *pVol;
	for( i=0; i < m_nNumVisVols; i++ ) {
		pVol = (ApeVisVolume_t *)m_aVisVols[i];
		nNumCells += pVol->nNumCells;
	}
	return nNumCells;
}

u32 CApeFileLoader::GetNumVisPlanes() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	u32 i, j, nNumPlanes( 0 );
	ApeVisVolume_t *pVol;
	for( i=0; i < m_nNumVisVols; i++ ) {
		pVol = (ApeVisVolume_t *)m_aVisVols[i];
		for( j=0; j < pVol->nNumCells; j++ ) {
			// cell faces are planes
			nNumPlanes += pVol->aCells[j].nNumFaces;
		}
	}
	return nNumPlanes;
}

u32 CApeFileLoader::GetNumSegments() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	return m_nNumSegments;
}

u32 CApeFileLoader::GetNumMeshMaterials() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	u32 nNumMats( 0 );
	for( u32 i=0; i < m_nNumSegments; i++ ) {
		nNumMats += GetNumSegmentMaterials( i );
	}
	return nNumMats;
}

u32 CApeFileLoader::GetNumMeshVerts() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	u32 nNumVerts( 0 );
	for( u32 i=0; i < m_nNumSegments; i++ ) {
		nNumVerts += GetNumSegmentVerts( i );
	}
	return nNumVerts;	
}

u32 CApeFileLoader::GetNumMeshIndices() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	u32 nNumIndices( 0 );
	for( u32 i=0; i < m_nNumSegments; i++ ) {
		nNumIndices += GetNumSegmentIndices( i );
	}
	return nNumIndices;
}

// returns the segment number from a mesh material index,
// this is a number from 0-GetNumMeshMaterials(),
// returns -1 if there are not nMeshMatNum materials in this mesh
s32 CApeFileLoader::GetSegNumFromMeshMatIndex( u32 nMeshMatIndex ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return -1;
	}
	if( nMeshMatIndex >= GetNumMeshMaterials() ) {
		UpdateErrorString( INVALID_MATERIAL_INDEX, m_sLastError );
		return -1;
	}
	u32 i, nSegMats;
	for( i=0; i < m_nNumSegments; i++ ) {
		nSegMats = GetNumSegmentMaterials( i );
		if( nSegMats <= nMeshMatIndex ) {
			nMeshMatIndex -= nSegMats;
		} else {
			return i;
		}
	}
	return -1;
}

ApeMaterial_t *CApeFileLoader::GetMatInfoFromMeshMatIndex( u32 nMeshMatIndex ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nMeshMatIndex >= GetNumMeshMaterials() ) {
		UpdateErrorString( INVALID_MATERIAL_INDEX, m_sLastError );
		return NULL;
	}
	u32 i, nSegMats;
	for( i=0; i < m_nNumSegments; i++ ) {
		nSegMats = GetNumSegmentMaterials( i );
		if( nSegMats <= nMeshMatIndex ) {
			nMeshMatIndex -= nSegMats;
		} else {
			return GetMaterialInfo( i, nMeshMatIndex );
		}
	}
	return NULL;
}

BOOL CApeFileLoader::IsAnyMaterialSkinned() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return FALSE;
	}
	ApeSegment_t *pSeg;
	for( u32 i=0; i < m_nNumSegments; i++ ) {
		pSeg = GetSegmentInfo( i );
		if( pSeg->bSkinned ) {
			return TRUE;
		}
	}
	return FALSE;
}

ApeBone_t *CApeFileLoader::GetMeshBone( u32 nBoneIndex ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return FALSE;
	}
	if( nBoneIndex >= m_nNumBones ) {
		UpdateErrorString( INVALID_BONE_INDEX, m_sLastError );
		return FALSE;
	}
	return &m_pBones[nBoneIndex];
}

ApeLight_t *CApeFileLoader::GetLightInfo( u32 nLightNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nLightNum >= m_nNumLights ) {
		UpdateErrorString( INVALID_LIGHT_INDEX, m_sLastError );
		return NULL;
	}
	return &m_pLights[nLightNum];
}

ApeObject_t *CApeFileLoader::GetObjectInfo( u32 nObjectNum )  {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nObjectNum >= m_nNumObjects ) {
		UpdateErrorString( INVALID_OBJECT_INDEX, m_sLastError );
		return NULL;
	}
	return (ApeObject_t *)m_aObjects[nObjectNum];
}

ApeFog_t *CApeFileLoader::GetFogInfo( u32 nFogNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nFogNum >= m_pMeshInfo->nNumFogs ) {
		UpdateErrorString( INVALID_FOG_INDEX, m_sLastError );
		return NULL;
	}
	return &m_pFog[nFogNum];
}

ApeShape_t *CApeFileLoader::GetShapeInfo( u32 nShapeNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nShapeNum >= m_nNumShapes ) {
		UpdateErrorString( INVALID_SHAPE_INDEX, m_sLastError );
		return NULL;
	}
	return (ApeShape_t *)m_aShapes[nShapeNum];
}

ApeVisVolume_t *CApeFileLoader::GetVisVol( u32 nVolNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nVolNum >= m_nNumVisVols ) {
		UpdateErrorString( INVALID_VOL_INDEX, m_sLastError );
		return NULL;
	}
	return (ApeVisVolume_t *)m_aVisVols[nVolNum];
}

ApeVisPortal_t *CApeFileLoader::GetVisPortal( u32 nPortalNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nPortalNum >= m_nNumVisPortals ) {
		UpdateErrorString( INVALID_PORTAL_INDEX, m_sLastError );
		return NULL;
	}
	return (ApeVisPortal_t *)m_aVisPortals[nPortalNum];
}

u32 CApeFileLoader::GetNumBoneChildren() {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return FALSE;
	}
	u32 nCount = 0;
	for( u32 i=0; i < m_nNumBones; i++ ) {
		nCount += m_pBones[i].nNumChildren;
	}
	return nCount;	
}

ApeSegment_t * CApeFileLoader::GetSegmentInfo( u32 nSegmentNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	if( nSegmentNum >= m_nNumSegments ) {
		UpdateErrorString( INVALID_SEGMENT_INDEX, m_sLastError );
		return NULL;
	}
	return (ApeSegment_t *)m_aSegments.GetAt( nSegmentNum );
}

u32 CApeFileLoader::GetNumSegmentMaterials( u32 nSegmentNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	ApeSegment_t *pSegment = GetSegmentInfo( nSegmentNum );
	if( !pSegment ) {
		UpdateErrorString( INVALID_SEGMENT_INDEX, m_sLastError );
		return 0;
	}
	return pSegment->nNumMaterials;
}

u32 CApeFileLoader::GetNumSegmentVerts( u32 nSegmentNum ) {

	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	ApeSegment_t *pSegment = GetSegmentInfo( nSegmentNum );
	if( !pSegment ) {
		UpdateErrorString( INVALID_SEGMENT_INDEX, m_sLastError );
		return 0;
	}
	return pSegment->nNumVerts;
}

u32 CApeFileLoader::GetNumSegmentIndices( u32 nSegmentNum ) {

	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	ApeSegment_t *pSegment = GetSegmentInfo( nSegmentNum );
	if( !pSegment ) {
		UpdateErrorString( INVALID_SEGMENT_INDEX, m_sLastError );
		return 0;
	}
	return pSegment->nNumIndices;
}

ApeMaterial_t * CApeFileLoader::GetMaterialInfo( u32 nSegmentNum, u32 nMaterialNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	u32 nNumMaterials = GetNumSegmentMaterials( nSegmentNum );
	if( !nNumMaterials || (nMaterialNum >= nNumMaterials) ) {
		UpdateErrorString( INVALID_MATERIAL_INDEX, m_sLastError );
		return NULL;
	}
	ApeMaterial_t *pMat = (ApeMaterial_t *)m_aMaterials[nSegmentNum];
	return (ApeMaterial_t *)&pMat[ nMaterialNum ];
}

u32 CApeFileLoader::GetNumMaterialIndices( u32 nSegmentNum, u32 nMaterialNum ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	ApeMaterial_t *pMat = GetMaterialInfo( nSegmentNum, nMaterialNum );
	if( !pMat ) {
		return 0;
	}
	return pMat->nNumIndices;
}

ApeVert_t * CApeFileLoader::GetVertData( u32 nSegmentNum, u32 nVertNum/*=0*/ ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return NULL;
	}
	ApeSegment_t *pSeg = GetSegmentInfo( nSegmentNum );
	if( !pSeg ) {
		return NULL;
	}
	if( nVertNum >= pSeg->nNumVerts ) {
		UpdateErrorString( INVALID_VERT_INDEX, m_sLastError );
		return NULL;
	}
	ApeVert_t *pVertData = (ApeVert_t *)m_aVerts[nSegmentNum];
	return &pVertData[nVertNum];
}

ApeVertIndex_t * CApeFileLoader::GetVertIndex( u32 nSegmentNum, u32 nMaterialNum, u32 nIndex/*=0*/ ) {
	if( !m_pAllocMem ) {
		UpdateErrorString( NO_FILE_LOADED, m_sLastError );
		return 0;
	}
	ApeMaterial_t *pMat = GetMaterialInfo( nSegmentNum, nMaterialNum );
	if( !pMat ) {
		return NULL;
	}
	if( nIndex >= pMat->nNumIndices ) {
		UpdateErrorString( INVALID_INDICE_INDEX, m_sLastError );
		return NULL;
	}
	ApeVertIndex_t *pIndex = (ApeVertIndex_t *)m_aVertIndices[nSegmentNum];
	return &pIndex[ pMat->nFirstIndex + nIndex ];
}

cchar * CApeFileLoader::GetLastError() {
	return m_sLastError;
}

void CApeFileLoader::UpdateErrorString( u32 nErrorCode, CString &rsErrorString ) {

	switch( nErrorCode ) {
	case NONE:
		rsErrorString = "no error";
		break;
	case NO_FILENAME:
		rsErrorString = "No filename specified";
		break;
	case NO_HEADER:
		rsErrorString = "Could not read file header";
		break;
	case COULD_NOT_OPEN_FILE:
		rsErrorString = "Could not open file";
		break;
	case INVALID_FILE_TYPE:
		rsErrorString = "Invalid ape file";
		break;
	case COULD_NOT_READ_ENTIRE_FILE:
		rsErrorString = "Error reading entire file";
		break;
	case NOT_ENOUGH_MEMORY:
		rsErrorString = "Could not allocate enough memory";
		break;
	case NO_FILE_LOADED:
		rsErrorString = "No file loaded";
		break;
	case INVALID_SEGMENT_INDEX:
		rsErrorString = "Invalid segment index";
		break;
	case INVALID_MATERIAL_INDEX:
		rsErrorString = "Invalid material index";
		break;
	case INVALID_VERT_INDEX:
		rsErrorString = "Invalid vertex index";
		break;
	case INVALID_INDICE_INDEX:
		rsErrorString = "Invalid indice";
		break;
	case FILE_LOADED_OK:
		rsErrorString = "File Loaded OK";
		break;
	case INVALID_MATERIAL_POINTER:
		rsErrorString = "Invalid material pointer";
		break;
	case INVALID_LIGHT_INDEX:
		rsErrorString = "Invalid light index";
		break;
	case INVALID_BONE_INDEX:
		rsErrorString = "Invalid bone index";
		break;
	case INVALID_OBJECT_INDEX:
		rsErrorString = "Invalid object index";
		break;
	case INVALID_FOG_INDEX:
		rsErrorString = "Invalid fog index";
		break;
	case INVALID_SHAPE_INDEX:
		rsErrorString = "Invalid shape index";
		break;
	case INVALID_VOL_INDEX:
		rsErrorString = "Invalid volume index";
		break;
	case INVALID_PORTAL_INDEX:
		rsErrorString = "Invalid portal index";
		break;
	}
}

BOOL CApeFileLoader::FixupPointers() {
	void *pNext;
	u32 i, j, k;
	ApeObject_t *pApeOb;
	ApeShape_t *pApeShape;
	ApeVisVolume_t *pVol;
	ApeVisPortal_t *pPortals;

	// set our mesh pointer and a pointer to the data right after the mesh struct
	m_pMeshInfo = (ApeMesh_t *)m_pAllocMem;
#if _NO_WORLD_FILES
	m_pMeshInfo->bWorldFile = FALSE;
#endif
	pNext = &m_pMeshInfo[1];
	// set the number of bones
	m_nNumBones = m_pMeshInfo->nNumBoneNames;
	if( m_nNumBones > 0 ) {
		m_pBones = (ApeBone_t *)pNext;
		pNext = &m_pBones[m_nNumBones];
	} else {
		m_pBones = NULL;
	}
	// set the number of lights	
	m_nNumLights = m_pMeshInfo->nNumLights;
	if( m_nNumLights > 0 ) {
		m_pLights = (ApeLight_t *)pNext;
		pNext = &m_pLights[m_nNumLights];
	} else {
		m_pLights = NULL;
	}
	// set the number of objects
	m_nNumObjects = m_pMeshInfo->nNumObjects;
	m_aObjects.RemoveAll();
	if( m_nNumObjects > 0 ) {
		m_aObjects.SetSize( m_nNumObjects );		
		for( i=0; i < m_nNumObjects; i++ ) {
			pApeOb = (ApeObject_t *)pNext;
			// make sure that the object name is not too long
			if( fclib_strlen( pApeOb->szName ) >= (OBJECT_NAME_LEN-4) ) {
				m_sLastError.Format( "There are object reference names that are too long, the first one is '%s'.\nPlease make all object name less than %d characters long.", pApeOb->szName, (OBJECT_NAME_LEN-4) );
				return FALSE;
			}
			m_aObjects[i] = pApeOb;
			pNext = (void *)((u32)&pApeOb[1] + pApeOb->nBytesOfUserData);
		}		
	}
	// set the fog pointer
	if( m_pMeshInfo->nNumFogs ) {
		m_pFog = (ApeFog_t *)pNext;
		pNext = (void *)&m_pFog[m_pMeshInfo->nNumFogs];
	}
	// set the number of shapes
	m_nNumShapes = m_pMeshInfo->nNumShapes;
	m_aShapes.RemoveAll();
	if( m_nNumShapes > 0 ) {
		m_aShapes.SetSize( m_nNumShapes );
		for( i=0; i < m_nNumShapes; i++ ) {
			pApeShape = (ApeShape_t *)pNext;
			m_aShapes[i] = pApeShape;
			pNext = (void *)((u32)&pApeShape[1] + pApeShape->nBytesOfUserData);
		}
	}

	// set the number of volumes
	m_nNumVisVols = m_pMeshInfo->nNumVisVolumes;
	m_aVisVols.RemoveAll();
	if( m_nNumVisVols ) {
		m_aVisVols.SetSize( m_nNumVisVols );
		pVol = (ApeVisVolume_t *)pNext;
		for( i=0; i < m_nNumVisVols; i++ ) {
			m_aVisVols[i] = &pVol[i];			
		}
		pNext = (void *)&pVol[m_nNumVisVols];
	}
	
	// set the number of portals
	m_nNumVisPortals = m_pMeshInfo->nNumVisPortals;
	m_aVisPortals.RemoveAll();
	if( m_nNumVisPortals ) {
		m_aVisPortals.SetSize( m_nNumVisPortals );
		pPortals = (ApeVisPortal_t *)pNext;
		for( i=0; i < m_nNumVisPortals; i++ ) { 
			m_aVisPortals[i] = &pPortals[i];
		}
		pNext = (void *)&pPortals[m_nNumVisPortals];
	}

	m_nNumSegments = m_pMeshInfo->nNumSegments;
	
	// set the size of each CPtrArray
	m_aSegments.SetSize( m_nNumSegments );
	m_aMaterials.SetSize( m_nNumSegments );
	m_aVerts.SetSize( m_nNumSegments );
	m_aVertIndices.SetSize( m_nNumSegments );
			
	// walk the segments storing pointers to the materials and segments
	ApeSegment_t *pSegment = (ApeSegment_t *)pNext;
	ApeMaterial_t *pMaterial;
	ApeVert_t *pVert;
	ApeVertIndex_t *pVertIndex;
	for( i=0; i < m_nNumSegments; i++ ) {
		// insert our segment pointer into the segment pointer array
		m_aSegments[i] = pSegment;
		FASSERT( pSegment->nNumMaterials );
		FASSERT( pSegment->nNumIndices );
		FASSERT( pSegment->nNumVerts );
		// the material[] follows the segment
		pMaterial = (ApeMaterial_t *)&pSegment[1];
		// make sure that the layer's texture names are not too long in any material in this segment
		for( j=0; j < pSegment->nNumMaterials; j++ ) {
			for( k=0; k < pMaterial[j].nLayersUsed; k++ ) {
				if( fclib_strlen( pMaterial[j].aLayer[k].szTexnames[APE_LAYER_TEXTURE_DIFFUSE] ) >= (TEXTURE_NAME_LEN-4) ) {
					m_sLastError.Format( "There are texture names that are too long, the first one is '%s'.\nPlease make all texture name less than %d characters long.", pMaterial[j].aLayer[k].szTexnames[APE_LAYER_TEXTURE_DIFFUSE], (TEXTURE_NAME_LEN-4) );
					return FALSE;
				}	
			}
		}
		m_aMaterials[i] = pMaterial;
		// the verts[] follow the material[]
		pVert = (ApeVert_t *)&pMaterial[ pSegment->nNumMaterials ];
		m_aVerts[i] = pVert;
		// the vert index[] follow the vert[]
		pVertIndex = (ApeVertIndex_t *)&pVert[ pSegment->nNumVerts ];
		m_aVertIndices[i] = pVertIndex;
		// the next segment follows the last vert index
		pSegment = (ApeSegment_t *)&pVertIndex[ pSegment->nNumIndices ];
	}	
	return TRUE;
}

// returns TRUE if coordinates were all valid and didn't require fixing
// returns FALSE if at least 1 coordinate was bad and needed fixing
BOOL CApeFileLoader::FixupInvalidSTCoords(  const CString &rsErrorHeading  ) {
	u32 i, j, k;
	ApeSegment_t *pSeg;
	ApeVert_t *pVertData;
	BOOL bReturn = TRUE;
	BOOL bHeaderWritten = FALSE;
	BOOL bLogVert;
	BOOL bSegNameWritten;
	s32 nLastVertIndexLogged;
	CString sError;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	for( i=0; i < m_nNumSegments; i++ ) {
		pSeg = GetSegmentInfo( i );
		FASSERT( pSeg );
		
		pVertData = (ApeVert_t *)m_aVerts[i];

		bSegNameWritten = FALSE;
		nLastVertIndexLogged = -1;

		for( j=0; j < pSeg->nNumVerts; j++ ) {
			
			for( k=0; k < MAX_LAYERS_PER_MAT; k++ ) {
				
				bLogVert = FALSE;
				if( fmath_Fcheck( pVertData->aUV[k].x ) != FMATH_FCHECK_RESULT_OK ) {
					// zero this float out
					pVertData->aUV[k].x = 0.0f;
					bLogVert = TRUE;
				}

				if( fmath_Fcheck( pVertData->aUV[k].y ) != FMATH_FCHECK_RESULT_OK ) {
					// zero this float out
					pVertData->aUV[k].y = 0.0f;
					bLogVert = TRUE;
				}

				if( bLogVert && (nLastVertIndexLogged != (s32)j) ) {
					bReturn = FALSE;
					nLastVertIndexLogged = j;

					// turn the vert to opaque red
					pVertData->Color.OpaqueRed();

					if( !bHeaderWritten ) {
						rErrorLog.WriteErrorHeader( rsErrorHeading );
						bHeaderWritten = TRUE;
					}
					if( !bSegNameWritten ) {
						sError.Format( "\tMax node '%s' has at least 1 invalid texture coordinate at the following 3ds Max vertices.", pSeg->szNodename );
						rErrorLog.WriteErrorLine( sError );
						bSegNameWritten = TRUE;
					}
					sError.Format( "\t\t[x, y, z] = %.3f, %.3f, %.3f", pVertData->Pos.x, pVertData->Pos.z, pVertData->Pos.y );
					rErrorLog.WriteErrorLine( sError );
				}
			}
			pVertData++;
		}
	}
	
	return bReturn;
}

