//////////////////////////////////////////////////////////////////////////////////////
// ApeToWorldFile.cpp - 
//
// Author: Michael Starich   
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
// 05/14/02 Starich     Created.
// 11/14/02 Lafleur     Generalized for all platforms from GC specific file
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#ifdef _MMI_TARGET_PS2
#include "ps2\resource.h"
#else
#include "resource.h"
#endif
#include "PasmDlg.h"
#include "fang.h"
#include "ApeToWorldFile.h"
#include "ErrorLog.h"
#include "fdata.h"
#include "utils.h"
#include "MasterFileCompile.h"
#include "LightMapGen.h"
#include "CompileDlg.h"
#include "pasm.h"

#define _ERROR_HEADING				"APE->WORLD FILE COMPILER "
#define _LIGHTMAPPING_ENABLED		TRUE
#define _MAX_LM_COMPILE_FILES		1024

extern CPasmApp theApp;

//
//
//
CApeToWorldFile::CApeToWorldFile( void ) 
{
	m_bConverted = FALSE;
	fang_MemZero( &m_FileHeader, sizeof( FData_WorldFileHeader_t ) );
	
	m_anMeshOffsets.RemoveAll();
	m_anMeshSizes.RemoveAll();
	m_anMeshPadBytes.RemoveAll();
	m_nWorldPadBytes = 0;
	m_nWorldInitPadBytes = 0;

	m_pFirstStaticObject = NULL;
	m_nStaticObjectCount = 0;
	m_nStaticObjectInstanceCount = 0;

	m_sTempMeshFile.Empty();
	m_nTempMeshFileSize = 0;

	m_nStreamingDataBytes = 0;
	m_pStreamingData = NULL;

	m_nApeObjectCount = 0;
	m_paApeLMs = NULL;
	m_paApeVertColors = NULL;

	m_nStripCount = m_nStripTriCount = m_nListTriCount = 0;

	m_nMeshCRC = 0;
}


//
//
//
CApeToWorldFile::~CApeToWorldFile() 
{
	FreeData();
}


//
//
//
BOOL CApeToWorldFile::ConvertToWorldFile( CCompileDlg *pDlg, CKongToVisFile &Vis, CAidFile &rAidFile, cchar *pszFilename, TargetPlatform_e nPlatform ) 
{
	FASSERT( m_paApeLMs == NULL );
	FASSERT( m_paApeVertColors == NULL );
	u32  nLightMapDetail = 0;
	u32 nLightMapSkyLightDetail=0;
	f32  fSubSample = 0;
	f32  fLightMapMemory = 0;
	f32 fLightMapSkyLightItens = 1.0f;
	BOOL bLightMapped = FALSE;
	CFColorRGB LightMapSkyLightHoriz(0.5f, 0.25f, 0.0f);
	CFColorRGB LightMapSkyLightZenith(0.1f, 0.1f, 0.5f);

	FreeData();

	// Make sure we have already converted the vis data
	if ( !Vis.m_bK2VConverted ) 
	{
		return FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE THE WORLD INIT DATA
	if ( !Vis.m_InitFile.ConvertKongFile( &Vis, NULL, NULL ) ) 
	{
		pDlg->ErrorString( "Could not create the shape init array." );
		return FALSE;
	}

#if _LIGHTMAPPING_ENABLED
	u32 i;

	// See if there are any lightmap lights
	BOOL bLightMapLightExists = FALSE;
	for ( i = 0; i < Vis.GetNumLights(); i++ )
	{
		ApeLight_t *pApeLight = Vis.GetLightInfo(i);

		if ( pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_LIGHT && !(pApeLight->nFlags & APE_LIGHT_FLAG_LIGHT_SELF) )
		{
			bLightMapLightExists = TRUE;
			break;
		}
	}

	if ( bLightMapLightExists )
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CREATE LIGHTMAPS FOR THE GEOMETRY
		nLightMapDetail = ((CPasmDlg *)theApp.m_pMainWnd)->m_nLightingQuality;
		fLightMapMemory = ((CPasmDlg *)theApp.m_pMainWnd)->m_fMaxLMapSize;
		fSubSample = ((CPasmDlg *)theApp.m_pMainWnd)->m_fSubSample;

		// check the aid file for user supplied settings
		if ( rAidFile.m_bDataValid ) 
		{
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].bValueWasFound )
			{
				nLightMapDetail = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].nValue;
			}
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_MBS].bValueWasFound )
			{
				fLightMapMemory = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_MBS].fValue;
			}
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE].bValueWasFound )
			{
				fSubSample = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE].fValue;
			}
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_DETAIL].bValueWasFound )
			{
				nLightMapSkyLightDetail = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_DETAIL].nValue;
			}

			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_RED].bValueWasFound )
			{
				LightMapSkyLightHoriz.fRed = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_RED].nValue / 255.0f;
			}
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_GREEN].bValueWasFound )
			{
				LightMapSkyLightHoriz.fGreen = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_GREEN].nValue / 255.0f;
			}
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_BLUE].bValueWasFound )
			{
				LightMapSkyLightHoriz.fBlue = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_BLUE].nValue / 255.0f;
			}

			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_RED].bValueWasFound )
			{
				LightMapSkyLightZenith.fRed = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_RED].nValue / 255.0f;
			}
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_GREEN].bValueWasFound )
			{
				LightMapSkyLightZenith.fGreen = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_GREEN].nValue / 255.0f;
			}
			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_BLUE].bValueWasFound )
			{
				LightMapSkyLightZenith.fBlue = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_BLUE].nValue / 255.0f;
			}

			if ( rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_INTENSITY].bValueWasFound )
			{
				fLightMapSkyLightItens = rAidFile.m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_INTENSITY].fValue;
			}
		}

		pDlg->m_bBuildingLightMaps = TRUE;
		m_nApeObjectCount = Vis.GetNumObjects();

		m_paApeLMs = (ApeObjectLMs_t *)malloc( sizeof(ApeObjectLMs_t) * m_nApeObjectCount );
		if ( !m_paApeLMs )
		{
			pDlg->ErrorString( "Could not allocate memory for the object lightmaps." );
			return FALSE;
		}
		memset( m_paApeLMs, 0, sizeof(ApeObjectLMs_t) * m_nApeObjectCount );

		m_paApeVertColors = (ApeObjectVertColors_t *)malloc( sizeof(ApeObjectVertColors_t) * m_nApeObjectCount );
		if ( !m_paApeVertColors )
		{
			pDlg->ErrorString( "Could not allocate memory for the object vertex colors." );
			return FALSE;
		}
		memset( m_paApeVertColors, 0, sizeof(ApeObjectVertColors_t) * m_nApeObjectCount );

#ifdef _MMI_TARGET_PS2
		if ( !CreateLightmaps( pDlg, Vis, pszFilename, nLightMapDetail, fLightMapMemory, fSubSample, TRUE, nLightMapSkyLightDetail, &LightMapSkyLightHoriz, &LightMapSkyLightZenith, fLightMapSkyLightItens ) )	//NS Always force vertex radiosity only on PS2
#else
		if ( !CreateLightmaps( pDlg, Vis, pszFilename, nLightMapDetail, fLightMapMemory, fSubSample, nLightMapDetail == 0, nLightMapSkyLightDetail, &LightMapSkyLightHoriz, &LightMapSkyLightZenith, fLightMapSkyLightItens ) )
#endif
		{
			pDlg->m_bBuildingLightMaps = FALSE;
			return FALSE;
		}
		bLightMapped = TRUE;
		pDlg->m_bBuildingLightMaps = FALSE;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// RECREATE THE WORLD INIT DATA (this time with the lighting data)
		if ( !Vis.m_InitFile.m_bUsingOldInit && !Vis.m_InitFile.ConvertKongFile( &Vis, m_paApeLMs, m_paApeVertColors ) ) 
		{
			pDlg->ErrorString( "Could not create the shape init array." );
			return FALSE;
		}
	}
#endif // _LIGHTMAPPING_ENABLED

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SAVE DATA AT THIS POINT
	if ( !Vis.m_InitFile.m_bUsingOldInit )
	{
		Vis.SaveVisFile( pDlg, pszFilename, nLightMapDetail, fLightMapMemory );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CONVERT VOLUME MESHES
	if ( !ConvertMeshesToFANGFormat( pDlg, Vis, pszFilename, nPlatform ) )
	{
		return FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE FILE HEADER
	if ( m_bConverted )
	{
		CreateFileHeader( Vis );
	}

	return m_bConverted;
}


//
//  This method is used if there is an up-to-date .vis file
//
BOOL CApeToWorldFile::ConvertToWorldFileFromVis( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszFilename, TargetPlatform_e nPlatform ) 
{
	u32 i;

	FASSERT( pDlg && pszFilename );

	FreeData();

	// Build the name for this world, without path or extension
	char szWldName[32];
	for ( i = 0; pszFilename[i] != '.' && (s32)i < fclib_strlen( pszFilename ); i++ )
	{
		szWldName[i] = pszFilename[i];
	}
	szWldName[i] = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MAKE SURE ASSET DEPENDENCIES ARE UP TO DATE
	pDlg->m_bBuildingLightMaps = TRUE;
	if ( Vis.m_nVisFileStatus == VIS_FILE_UP_TO_DATE_LIBRARY )
	{
		pDlg->m_bCheckLibraryKNGFirst = TRUE;
	}
	else
	{
		pDlg->m_bCheckLibraryKNGFirst = FALSE;
	}
#ifdef _MMI_TARGET_PS2
	BuildApeDependencyList( pDlg, Vis, TRUE );
#else
	BuildApeDependencyList( pDlg, Vis, (Vis.FileHeader.nLMLevel == 0) );
#endif
	CompileApeDependencyList( pDlg, Vis );

	Vis.m_bUsedOldVisData = FALSE;
	Vis.m_nStaticLightingQualityLevel = Vis.FileHeader.nLMLevel;

	CompileLightMaps( pDlg, Vis, szWldName, pDlg->m_bCheckLibraryKNGFirst, NULL );

	FreeApeDependencyList();
	pDlg->m_bBuildingLightMaps = FALSE;
	pDlg->m_bCheckLibraryKNGFirst = FALSE;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CONVERT VOLUME MESHES
	if ( !ConvertMeshesToFANGFormat( pDlg, Vis, pszFilename, nPlatform ) )
	{
		return FALSE;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE FILE HEADER
	if ( m_bConverted )
	{
		CreateFileHeader( Vis );
	}

	return m_bConverted;
}


//
//
//
BOOL CApeToWorldFile::CreateFileHeader( CKongToVisFile &Vis ) 
{
	u32 i, nSize;

	// Fill in our file header
	fang_MemZero( &m_FileHeader, sizeof( FData_WorldFileHeader_t ) );

	m_FileHeader.nNumMeshes = Vis.m_nStatsNumMeshes;
	m_FileHeader.nWorldBytes = Vis.GetConvertedDataSize();// the world + world header + fixups
	m_FileHeader.nStreamingDataBytes = m_nStreamingDataBytes;
	m_FileHeader.nInitBytes = Vis.m_InitFile.GetConvertedDataSize();// the init data + header + fixups
			
	nSize = sizeof( FData_WorldFileHeader_t );
	m_FileHeader.nOffsetToMeshInits = nSize;
	nSize += (sizeof( u32 ) * Vis.m_nStatsNumMeshes);// for mesh offset array
	m_FileHeader.nOffsetToMeshSizes = nSize;
	nSize += (sizeof( u32 ) * Vis.m_nStatsNumMeshes);// for mesh size array

	for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
	{
		m_anMeshPadBytes[i] = FMATH_BYTE_ALIGN_UP( nSize, FDATA_WORLD_MESH_ALIGNMENT );// align up so that the mesh header starts on a FDATA_WORLD_MESH_ALIGNMENT byte boundry
		m_anMeshPadBytes[i] -= nSize;

		nSize += m_anMeshPadBytes[i];
		m_anMeshOffsets[i] = nSize;
		
		nSize += m_anMeshSizes[i];

		if ( i > 0 ) 
		{
			m_FileHeader.nMeshBytes += m_anMeshSizes[i] + m_anMeshPadBytes[i]; 
		} 
		else 
		{
			m_FileHeader.nMeshBytes += m_anMeshSizes[i]; 
		}
	}
	FASSERT( m_FileHeader.nMeshBytes == m_nTempMeshFileSize );

	m_nWorldPadBytes = FMATH_BYTE_ALIGN_UP( nSize, 16 );// align up so that the world header starts on a 16 byte boundry
	m_nWorldPadBytes -= nSize;
	nSize += m_nWorldPadBytes;
	m_FileHeader.nWorldOffset = nSize;
	
	nSize += m_FileHeader.nWorldBytes;
	m_nStreamingDataPadBytes = FMATH_BYTE_ALIGN_UP( nSize, 32 );// align up so that the world header starts on a 16 byte boundry
	m_nStreamingDataPadBytes -= nSize;
	nSize += m_nStreamingDataPadBytes;
	m_FileHeader.nStreamingDataOffset = nSize;
	
	nSize += m_FileHeader.nStreamingDataBytes;
	m_nWorldInitPadBytes = FMATH_BYTE_ALIGN_UP( nSize, 16 );
	m_nWorldInitPadBytes -= nSize;// align up so that the world header starts on a 16 byte boundry
	nSize += m_nWorldInitPadBytes;
	m_FileHeader.nInitOffsets = nSize;

	nSize += m_FileHeader.nInitBytes;
	
	m_FileHeader.nNumBytes = nSize;

	return TRUE;
}


//
//
//
BOOL CApeToWorldFile::ConvertMeshesToFANGFormat( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszFilename, TargetPlatform_e nPlatform ) 
{
	u32 i, nSize;
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	CFVec3 *pDirLight = NULL;

	// Find the directional light
	for( i = 0; i < Vis.GetNumLights(); i++ ) 
	{
		ApeLight_t *pApeLight = Vis.GetLightInfo( i );

		if( pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_ONLY_LIGHT ) 
		{
			// Don't use the lightmap directional
			continue;	
		}
		
		if ( pApeLight->nType == APE_LIGHT_TYPE_DIR )
		{
			if ( pDirLight )
			{
				pDlg->ErrorString( "ERROR: Multiple dynamic directional lights detected in WLD file." );
				pDlg->ErrorString( "       This may result in aesthetic errors." );
			}

			pDirLight = &pApeLight->Dir;
		}
	}

	m_anMeshOffsets.SetSize( Vis.m_nStatsNumMeshes );
	m_anMeshSizes.SetSize( Vis.m_nStatsNumMeshes );
	m_anMeshPadBytes.SetSize( Vis.m_nStatsNumMeshes );
	CKongToMLMesh KongToMLMesh( FALSE );
	
	m_sTempMeshFile = Vis.m_sLastFileLoaded;
	m_sTempMeshFile += UTILS_PASM_TEMP_FILENAME_TAG;
	FILE *pFileStream = _tfopen( m_sTempMeshFile, _T("wb") );
	if ( !pFileStream ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + Vis.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not create a temporary file to hold all the compiled meshes." );
		return FALSE;
	}
	m_nTempMeshFileSize = 0;

	if ( pDlg )
	{
		char szTemp[128];
		sprintf( szTemp, "Converting %d volume meshes to FANG platform specific format...", Vis.m_nStatsNumMeshes );
		pDlg->ResetSubOp( szTemp, Vis.m_nStatsNumMeshes );
		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
		pDlg->InfoString( szTemp );
	}

	MLManager.AccumStreamingData();

	m_nMeshCRC = 0;
	u32 nTotalLibTime = 0;
	u32 nTotalStrippingTime = 0;
	m_nStripCount = m_nStripTriCount = m_nListTriCount = 0;
	for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
	{
		if ( pDlg )
		{
			pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
		}
		// pad between meshes
		nSize = FMATH_BYTE_ALIGN_UP( m_nTempMeshFileSize, FDATA_WORLD_MESH_ALIGNMENT );
		nSize -= m_nTempMeshFileSize;

		if ( nSize ) 
		{
			utils_PadFileSize( pFileStream, nSize );
			m_nTempMeshFileSize += nSize;
		}

		if ( pDlg->m_bAbortCompile )
		{
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + Vis.m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Compiling aborted by user." );
			fclose( pFileStream );
			return FALSE;
		}

		((CApeToKongFormat *)Vis.m_apVolKongMeshs[i])->m_bVolumeMesh = TRUE;
		m_bConverted = KongToMLMesh.Convert( pDlg, *((CApeToKongFormat *)Vis.m_apVolKongMeshs[i]), pszFilename, NULL, nPlatform, pDlg->m_bGenerateMeshStrips, TRUE, FALSE, pDirLight );
		if ( !m_bConverted ) 
		{
			break;
		}
		m_anMeshSizes[i] = KongToMLMesh.GetSizeOfConvertedFile();// the mesh + mesh header + fixups 

		m_nStripCount += KongToMLMesh.m_nStripCount;
		m_nStripTriCount += KongToMLMesh.m_nStripTriCount;
		m_nListTriCount += KongToMLMesh.m_nListTriCount;

		// Update the CRC
		m_nMeshCRC = KongToMLMesh.GetCRC( m_nMeshCRC );

		if ( !KongToMLMesh.WriteConvertedFile( m_sTempMeshFile, pFileStream ) ) 
		{
			m_bConverted = FALSE;
			break;
		}
		m_nTempMeshFileSize += m_anMeshSizes[i];

		nTotalLibTime += KongToMLMesh.m_nMeshLibTime;
		nTotalStrippingTime += KongToMLMesh.m_nStrippingTime;

		// we no longer need the actual mesh anymore, free the memory
		KongToMLMesh.FreeData();
	}

	m_nStreamingDataBytes = 0;
	m_pStreamingData = MLManager.GetStreamingData( &m_nStreamingDataBytes );
	pDlg->InfoString( "Streaming Data Bytes: %d", m_nStreamingDataBytes );

	fclose( pFileStream );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	pDlg->InfoString( "Stripped volume geometry (%f seconds)", (f32)(nTotalStrippingTime) / 1000.f );
	pDlg->InfoString( "Converted to FANG format (%f seconds)", (f32)(nTotalLibTime) / 1000.f );

	return TRUE;
}


//
//
//
u32 CApeToWorldFile::BuildApeDependencyList( CCompileDlg *pDlg, CKongToVisFile &Vis, BOOL bForceVertexRadiosity ) 
{
	u32 i;
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	FASSERT( m_pFirstStaticObject == NULL );

	m_nStaticObjectCount = 0;
	m_nStaticObjectInstanceCount = 0;

	if ( !Vis.m_InitFile.m_pHeader )
	{
		return 0;
	}

	// Find the objects that will contribute to lightmaps (either by casting shadows, actually receiving lightmaps or both)
	for ( i = 0; i < Vis.m_InitFile.m_pHeader->nNumInitStructs; i++ )
	{
		CFWorldShapeInit *pShape = &Vis.m_InitFile.m_pShapeArray[i];
		if ( pShape->m_nShapeType != FWORLD_SHAPETYPE_MESH )
		{
			continue;
		}

		if ( !pShape->m_pMesh->m_pszMeshName )
		{
			continue;
		}
        
		if ( pShape->m_pMesh->m_nMeshInstFlags & FMESHINST_FLAG_DONT_DRAW )
		{
			continue;
		}
        
		// Get a pointer to the corresponding ApeObject
		ApeObject_t *pApeObject = Vis.GetObjectInfo( i );
		if ( !pApeObject )
		{
			continue;
		}

#ifndef _MMI_TARGET_PS2
	#if 0
		// If we are forcing vertex radiosity and the object is not lightmapped or vert rad lit,
		// we skip it to cause the lighting to calc much faster (But not for PS2 because they
		// actually force vert rad for the final product).
		if ( bForceVertexRadiosity && !(pShape->m_pMesh->m_nMeshInstFlags & (FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY)) )
		{
			continue;
		}
	#else
		// If we are forcing vertex radiosity, the level is either being "quick-compiled" for
		// development purposes, or we are compiling for PS2.  If not PS2, then we will only
		// vert rad light the world.  Objects will be dynamically lit in the engine.
		if ( bForceVertexRadiosity )
		{
			// Remove the static lighting from this object because it will now be dynamically lit in the engine
			pApeObject->nFlags &= ~(APE_OB_FLAG_VERT_RADIOSITY|APE_OB_FLAG_LM);
			continue;
		}
	#endif
#endif
        // Did artists flag this object as not contributing to static lighting phase?
        if ( pApeObject->nFlags & APE_OB_FLAG_NO_LM_USE )
		{
			continue;
		}

		// Object must be static 
		if ( !(pShape->m_pMesh->m_nMeshInstFlags & FMESHINST_FLAG_STATIC) )
		{
			continue;
		}

		// Check to see if the object is in the local files list
		CString csTemp = pShape->m_pMesh->m_pszMeshName;
		csTemp += ".ape";
		void *pLookup;
		if ( !((CPasmDlg *)theApp.m_pMainWnd)->m_MapLocalFiles.Lookup( csTemp, pLookup ) ) 
		{
			if ( pShape->m_pMesh->m_nMeshInstFlags & (FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY) )
			{
				// If the user wanted to lightmap or vert rad this object, then issue an error
				pDlg->ErrorString( "ERROR!!! APE file not found for object %s.", pShape->m_pMesh->m_pszMeshName );
			}
			continue;
		}

		LMG_ObjectInstance_t *pAdd = new LMG_ObjectInstance_t;
		if ( !pAdd )
		{
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + Vis.m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Could not allocate memory to compile dependencies." );
			return FALSE;
		}

//		DEVPRINTF( "Compiling %s.\n", csTemp );

		BOOL bVertRadLight = !!(pShape->m_pMesh->m_nMeshInstFlags & FMESHINST_FLAG_VERT_RADIOSITY);

		// If this is a static object, and we are forcing vertex radiosity (most likely
		// because we are not applying lightmapping), then we will apply vertex 
		// radiosity to this object.
		bVertRadLight |= (bForceVertexRadiosity && (pShape->m_pMesh->m_nMeshInstFlags & FMESHINST_FLAG_LM) );

		m_nStaticObjectInstanceCount++;
		pAdd->nApeObjectIdx = i;
		pAdd->nLMVolumeIdx = -1;
		pAdd->pNextInstance = NULL;
		pAdd->bAcceptsLightmaps = (!!(pShape->m_pMesh->m_nMeshInstFlags & FMESHINST_FLAG_LM) && !bForceVertexRadiosity);

		// Append the object to the list
		LMG_Object_t *pTest = m_pFirstStaticObject;
		while ( pTest )
		{
			CFWorldShapeInit *pTestShape = &Vis.m_InitFile.m_pShapeArray[pTest->pFirstInstance->nApeObjectIdx];
			if ( !stricmp( pShape->m_pMesh->m_pszMeshName, pTestShape->m_pMesh->m_pszMeshName ) )
			{
				// These objects are the same, so we want to add to this list
				pAdd->pNextInstance = pTest->pFirstInstance;
				pTest->pFirstInstance = pAdd;
				pTest->bAtLeastOneInstanceLightmapped |= pAdd->bAcceptsLightmaps;
				pTest->bAtLeastOneInstanceVertRadLit |= bVertRadLight;
				pTest->nInstanceCount++;
				break;
			}

			pTest = pTest->pNextObject;
		}

		if ( !pTest )
		{
			// We didn't find an object match, so put this one in the list
			LMG_Object_t *pNewObject = new LMG_Object_t;
			if ( !pNewObject )
			{
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + Vis.m_sLastFileLoaded );
				rErrorLog.WriteErrorLine( "Could not allocate memory to compile dependencies." );
				return FALSE;
			}

			pNewObject->bAtLeastOneInstanceLightmapped = pAdd->bAcceptsLightmaps;
			pNewObject->bAtLeastOneInstanceVertRadLit = bVertRadLight;
			pNewObject->nInstanceCount = 1;
			pNewObject->nMaterialCount = 0;
			pNewObject->nVertexCount = 0;
			pNewObject->panMaterialVertexCount = NULL;
			pNewObject->pFirstInstance = pAdd;
			pNewObject->pNextObject = m_pFirstStaticObject;
			m_pFirstStaticObject = pNewObject;

			m_nStaticObjectCount++;
		}
	}

	return m_nStaticObjectCount;
}


//
//
//
BOOL CApeToWorldFile::CompileApeDependencyList( CCompileDlg *pDlg, CKongToVisFile &Vis ) 
{
	u32 j;
	char szTemp[128];
	if ( !m_pFirstStaticObject )
	{
		return TRUE;
	}

	sprintf( szTemp, "Compiling %d APE dependencies...", m_nStaticObjectCount );
	pDlg->ResetSubOp( szTemp, m_nStaticObjectCount + 1 );
	pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	pDlg->InfoString( "%s", szTemp );

//	BOOL bPriorForceSetting = pDlg->m_bForceCompile;
//	pDlg->m_bForceCompile = FALSE;
	u16 nAssetsNotFound = 0;
	u16 nCompileSuccessful = 0;
	u16 nCompileFailed = 0;
	u16 nCompileNotNeeded = 0;
	LMG_Object_t *pLMObject = m_pFirstStaticObject;
	while ( pLMObject )
	{
		FASSERT( pLMObject->pFirstInstance );
		CFWorldShapeInit *pShape = &Vis.m_InitFile.m_pShapeArray[pLMObject->pFirstInstance->nApeObjectIdx];
		CString csTemp = pShape->m_pMesh->m_pszMeshName;
		csTemp += ".ape";

		void *pLookup;
		if ( ((CPasmDlg *)theApp.m_pMainWnd)->m_MapLocalFiles.Lookup( csTemp, pLookup ) ) 
		{
			pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
			pDlg->m_bGenerateLMSTs = pLMObject->bAtLeastOneInstanceLightmapped;
			pDlg->m_bGenerateVertRad = pLMObject->bAtLeastOneInstanceVertRadLit;
			CallConverter( (CFileInfo *)pLookup, pDlg, j );
			pLMObject->nCompileResult = pDlg->m_nCompileResult;
			if ( pDlg->m_nCompileResult == CDLG_RESULT_COMPILE_SUCCESSFUL )
			{
				nCompileSuccessful++;
			}
			else if ( pDlg->m_nCompileResult == CDLG_RESULT_UP_TO_DATE_LIBRARY || pDlg->m_nCompileResult == CDLG_RESULT_UP_TO_DATE_LOCALLY )
			{
				nCompileNotNeeded++;
			}
			else
			{
				pDlg->ErrorString( "ERROR!!! APE file for object %s failed to compile.", pShape->m_pMesh->m_pszMeshName );
				nCompileFailed++;
			}
			pDlg->m_bGenerateLMSTs = FALSE;
			pDlg->m_bGenerateVertRad = FALSE;
		}
		else
		{
			pLMObject->nCompileResult = CDLG_RESULT_ASSET_LOAD_UNSUCCESSFUL;
			pDlg->ErrorString( "ERROR!!! APE file not found for object %s.", pShape->m_pMesh->m_pszMeshName );
			nAssetsNotFound++;
		}

		pLMObject = pLMObject->pNextObject;
	}

	// Set the dialog settings to previous values
//	pDlg->m_bForceCompile = bPriorForceSetting;
	pDlg->m_bGenerateLMSTs = FALSE;

	pDlg->InfoString( "APE dependencies: %d failed to compile. %d up-to-date. %d compiled.", nCompileFailed, nCompileNotNeeded, nCompileSuccessful );

	return TRUE;
}


//
//
//
void CApeToWorldFile::FreeApeDependencyList( void ) 
{
	LMG_Object_t *pLMNextObject, *pLMObject = m_pFirstStaticObject;
	while ( pLMObject )
	{
		LMG_ObjectInstance_t *pLMNextInstance, *pLMInstance = pLMObject->pFirstInstance;
		while ( pLMInstance )
		{
			pLMNextInstance = pLMInstance->pNextInstance;
			delete pLMInstance;
			m_nStaticObjectInstanceCount--;
			pLMInstance = pLMNextInstance;
		}
		pLMNextObject = pLMObject->pNextObject;
		if ( pLMObject->panMaterialVertexCount )
		{
			free( pLMObject->panMaterialVertexCount );
		}
		delete pLMObject;
		m_nStaticObjectCount--;
		pLMObject = pLMNextObject;
	}

	FASSERT( m_nStaticObjectCount == 0 && m_nStaticObjectInstanceCount == 0 );

	m_pFirstStaticObject = NULL;
}


//
//
//
BOOL CApeToWorldFile::CheckForReusableVIS( CCompileDlg *pDlg, cchar *pszWldName, CKongToVisFile &Vis, u32 nLightMapDetail, f32 fLightMapMBs ) 
{
    FASSERT( pDlg );
	CKongToWorldInitFile InitFile;
	u32 i, j, k, nBytesRead, nKey;
	VisFileHeader_t VisHeader;
	CString csFilePath;
	HANDLE hFile;
	CString csReuseProblem;
	void *pConvertedData = NULL;
	KongMesh_t *paSrcKongMesh = NULL;

	BOOL bLibraryGood = FALSE, bLocalGood = FALSE;

	csFilePath = *CPasmDlg::m_pStaticLocalDir + "\\VIS\\" + pszWldName + ".vis";
	csFilePath.MakeLower();
	hFile = CreateFile( csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		// Read the VIS header
		ReadFile( hFile, &VisHeader, sizeof( VisFileHeader_t ), (LPDWORD)&nBytesRead, NULL );
		if ( nBytesRead == sizeof( VisFileHeader_t ) )
		{
			// Check version numbers
			if (   VisHeader.nSignature       == 0x2B00B1E5
				&& VisHeader.nVisVersion      == VIS_FILE_VERSION
				&& VisHeader.nKongVersion     == KONG_FILE_VERSION
				&& VisHeader.nVisDataVersion  == VIS_DATA_VERSION
				&& VisHeader.nInitDataVersion == INIT_DATA_VERSION )
			{
				if ( VisHeader.nVolumeMeshCount == Vis.m_nStatsNumMeshes )
				{
					if ( VisHeader.nLMLevel >= nLightMapDetail && VisHeader.fLMMemory >= fLightMapMBs )
					{
						// This VIS is good so far and contains lightmaps
						bLocalGood = TRUE;
					}
				}
			}
		}
	}

	if ( !bLocalGood )
	{
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( hFile );
			Vis.RemoveLocalVISData( csFilePath, pszWldName );
		}

		// Couldn't find a good VIS locally so check the library
		csFilePath = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\VIS\\" + pszWldName + ".vis";
		csFilePath.MakeLower();
		hFile = CreateFile( csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			// Read the VIS header
			ReadFile( hFile, &VisHeader, sizeof( VisFileHeader_t ), (LPDWORD)&nBytesRead, NULL );
			if ( nBytesRead == sizeof( VisFileHeader_t ) )
			{
				// Check version numbers
				if (   VisHeader.nSignature       == 0x2B00B1E5
					&& VisHeader.nVisVersion      == VIS_FILE_VERSION
					&& VisHeader.nKongVersion     == KONG_FILE_VERSION
					&& VisHeader.nVisDataVersion  == VIS_DATA_VERSION
					&& VisHeader.nInitDataVersion == INIT_DATA_VERSION )
				{
					if ( VisHeader.nVolumeMeshCount == Vis.m_nStatsNumMeshes )
					{
						if ( VisHeader.nLMLevel >= nLightMapDetail && VisHeader.fLMMemory >= fLightMapMBs )
						{
							// This VIS is good so far and contains lightmaps
							bLibraryGood = TRUE;
						}
					}
				}
			}
		}
	}

	if ( !bLocalGood && !bLibraryGood )
	{
		csReuseProblem = "";
		goto _VISDIDNOTMATCH;
	}

	// Allocate some kong meshes to hold the old data
	paSrcKongMesh = new KongMesh_t[VisHeader.nVolumeMeshCount];
	if ( !paSrcKongMesh )
	{
		csReuseProblem = "Error allocating memory for old volume mesh data.";
		goto _VISDIDNOTMATCH;
	}

	// Read each of the volume Kong meshes
	for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
	{	
		nBytesRead = paSrcKongMesh[i].LoadKong( hFile );
		if ( nBytesRead == 0 )
		{
			csReuseProblem = "Error loading old volume mesh data.";
			goto _VISDIDNOTMATCH;
		}

		KongMesh_t *pDstKongMesh = ((CApeToKongFormat *)Vis.m_apVolKongMeshs[i])->m_pKongMesh;

		// Make sure all of the triangle and material counts are still the same
		if ( paSrcKongMesh[i].nMaterialCount != pDstKongMesh->nMaterialCount )
		{
			csReuseProblem = "Volume material counts did not match.";
			goto _VISDIDNOTMATCH;
		}
		if ( paSrcKongMesh[i].nTriangleCount != pDstKongMesh->nTriangleCount )
		{
			csReuseProblem = "Volume triangle counts did not match.";
			goto _VISDIDNOTMATCH;
		}

		// Check to see if the vertex positions and normals are the same
		for ( j = 0; j < paSrcKongMesh[i].nTriangleCount; j++ )
		{
			if (   (paSrcKongMesh[i].paTriangles[j].apKongVerts[0]->Pos  - pDstKongMesh->paTriangles[j].apKongVerts[0]->Pos ).Mag2() > 0.001f 
				|| (paSrcKongMesh[i].paTriangles[j].apKongVerts[0]->Norm - pDstKongMesh->paTriangles[j].apKongVerts[0]->Norm).Mag2() > 0.001f
				|| (paSrcKongMesh[i].paTriangles[j].apKongVerts[1]->Pos  - pDstKongMesh->paTriangles[j].apKongVerts[1]->Pos ).Mag2() > 0.001f
				|| (paSrcKongMesh[i].paTriangles[j].apKongVerts[1]->Norm - pDstKongMesh->paTriangles[j].apKongVerts[1]->Norm).Mag2() > 0.001f
				|| (paSrcKongMesh[i].paTriangles[j].apKongVerts[2]->Pos  - pDstKongMesh->paTriangles[j].apKongVerts[2]->Pos ).Mag2() > 0.001f
				|| (paSrcKongMesh[i].paTriangles[j].apKongVerts[2]->Norm - pDstKongMesh->paTriangles[j].apKongVerts[2]->Norm).Mag2() > 0.001f )
			{
				// Vertex position or normal does not match
				csReuseProblem = "Volume vertex data did not match.";
				goto _VISDIDNOTMATCH;
			}
		}
	}

	// Read the progress key
	ReadFile( hFile, &nKey, sizeof( u32 ), (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != sizeof( u32 ) )
	{
		csReuseProblem = "Error loading old data key.";
		goto _VISDIDNOTMATCH;
	}

	if ( nKey != 0xAB00B000 )
	{
		csReuseProblem = "Error old data key does not match.";
		goto _VISDIDNOTMATCH;
	}

	// Read the FVis data
	pConvertedData = fang_Malloc( VisHeader.nVisDataBytes, 4 );
	if ( !pConvertedData )
	{
		csReuseProblem = "Error allocating memory for old FVIS data.";
		goto _VISDIDNOTMATCH;
	}
	ReadFile( hFile, pConvertedData, VisHeader.nVisDataBytes, (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != VisHeader.nVisDataBytes )
	{
		csReuseProblem = "Error loading old FVIS data.";
		goto _VISDIDNOTMATCH;
	}
/*
	// Why is this commented out?  Well, since we only store the dynamic lights in the VIS files, we have
	// no idea what the static light set was.  It makes no sense to verify the dynamic set since that
	// has no bearing on the static lighting.

	const FVisData_t *pNewVisData = Vis.GetConvertedFVISData();
	const FVisData_t *pOldVisData = (FVisData_t *)pConvertedData;
	if ( pNewVisData->nLightCount != pOldVisData->nLightCount )
	{
		csReuseProblem = "Light count doesn't match.";
		goto _VISDIDNOTMATCH;
	}

	FLightInit_t *pOldLight = (FLightInit_t *)((u32)pOldVisData->paLights + (u32)pOldVisData);
	FLightInit_t *pNewLight = (FLightInit_t *)((u32)pNewVisData->paLights + (u32)pNewVisData);
	for ( i = 0; i < pOldVisData->nLightCount; i++, pOldLight++, pNewLight++ )
	{
		if( memcmp( pOldLight, pNewLight, sizeof(FLightInit_t) ) != 0 )
		{
			csReuseProblem = "Light data doesn't match.";
			goto _VISDIDNOTMATCH;
		}
	}
*/
	// Read shape data
	nBytesRead = InitFile.LoadInitDataFromVIS( hFile, VisHeader.nInitDataBytes );
	if ( nBytesRead != VisHeader.nInitDataBytes )
	{
		csReuseProblem = "Error loading old shape init data.";
		goto _VISDIDNOTMATCH;
	}

	// Convert our shape init with some assistance from the prior shape init array
	if ( !Vis.m_InitFile.ConvertKongFileUsingInit( pDlg, &Vis, InitFile.m_pHeader, VisHeader.nInitDataBytes ) )
	{
		csReuseProblem = "Error converting shape init data using old data.";
		goto _VISDIDNOTMATCH;
	}

	// Copy over the volume mesh lighting data from the prior file
	for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
	{	
		KongMesh_t *pDstKongMesh = ((CApeToKongFormat *)Vis.m_apVolKongMeshs[i])->m_pKongMesh;

		// Copy over the vertex colors and lightmap UV data
		for ( j = 0; j < paSrcKongMesh[i].nTriangleCount; j++ )
		{
			u32 nVert;
			for ( nVert = 0; nVert < 3; nVert++ )
			{
				KongVert_t *pDestVert = pDstKongMesh->paTriangles[j].apKongVerts[nVert];
				KongVert_t *pSrcVert  = paSrcKongMesh[i].paTriangles[j].apKongVerts[nVert];

				pDestVert->Color = pSrcVert->Color;
				pDestVert->nBaseUVCount = pSrcVert->nBaseUVCount;
				pDestVert->nLightMapUVCount = pSrcVert->nLightMapUVCount;

				for ( k = 0; k < pSrcVert->nLightMapUVCount; k++ )
				{
					pDestVert->aUV[pSrcVert->nBaseUVCount + k].x = pSrcVert->aUV[pSrcVert->nBaseUVCount + k].x;
					pDestVert->aUV[pSrcVert->nBaseUVCount + k].y = pSrcVert->aUV[pSrcVert->nBaseUVCount + k].y;
				}
			}
		}

		pDstKongMesh->KHeader.bHasLightmapsSTs = TRUE;
/*
		// All vertex positions and normals are the same, so lightmapping uv's should be about the same
		for ( j = 0; j < pSavedMesh->nTriangleCount; j++ )
		{
			for ( k = 0; k < pSavedMesh->paTriangles[j].apKongVerts[0]->nLightMapUVCount; k++ )
			{
				KongVert_t *pVert = pKongToConvert->paTriangles[j].apKongVerts[0];
				KongVert_t *pSavedVert = pSavedMesh->paTriangles[j].apKongVerts[0];
				pVert->aUV[pVert->nBaseUVCount + k].x = pSavedVert->aUV[pSavedVert->nBaseUVCount + k].x;
				pVert->aUV[pVert->nBaseUVCount + k].y = pSavedVert->aUV[pSavedVert->nBaseUVCount + k].y;
				pVert = pKongToConvert->paTriangles[j].apKongVerts[1];
				pSavedVert = pSavedMesh->paTriangles[j].apKongVerts[1];
				pVert->aUV[pVert->nBaseUVCount + k].x = pSavedVert->aUV[pSavedVert->nBaseUVCount + k].x;
				pVert->aUV[pVert->nBaseUVCount + k].y = pSavedVert->aUV[pSavedVert->nBaseUVCount + k].y;
				pVert = pKongToConvert->paTriangles[j].apKongVerts[2];
				pSavedVert = pSavedMesh->paTriangles[j].apKongVerts[2];
				pVert->aUV[pVert->nBaseUVCount + k].x = pSavedVert->aUV[pSavedVert->nBaseUVCount + k].x;
				pVert->aUV[pVert->nBaseUVCount + k].y = pSavedVert->aUV[pSavedVert->nBaseUVCount + k].y;
			}
		}
*/
		// Copy over the material lightmap information
		for ( j = 0; j < paSrcKongMesh[i].nMaterialCount; j++ )
		{
			pDstKongMesh->paMaterials[j].nLightMapCount = paSrcKongMesh[i].paMaterials[j].nLightMapCount;
			for ( k = 0; k < FSH_MAX_LIGHTMAPS; k++ )
			{
				pDstKongMesh->paMaterials[j].anLightMapMotifID[k] = paSrcKongMesh[i].paMaterials[j].anLightMapMotifID[k];
				strcpy( pDstKongMesh->paMaterials[j].szLightMapTexname[k], paSrcKongMesh[i].paMaterials[j].szLightMapTexname[k] );
			}
		}
	}

	if ( paSrcKongMesh )
	{
		delete[] paSrcKongMesh;
	}

	pDlg->InfoString( "WARNING: Reusing static lighting data from older VIS file." );
	pDlg->InfoString( "         Quality of lighting might be compromised." );

	fang_Free( pConvertedData );
	CloseHandle( hFile );

	#if LMG_INCLUDE_STATIC_APE_OBJECTS
		#ifndef _MMI_TARGET_PS2
			BuildApeDependencyList( pDlg, Vis, (VisHeader.nLMLevel == 0) );
		#else
			// PS2 Always uses vertex radiosity
			BuildApeDependencyList( pDlg, Vis, TRUE );
		#endif

		// Compile object dependencies
		if ( m_nStaticObjectCount )
		{
			if ( !CompileApeDependencyList( pDlg, Vis ) )
			{
				return FALSE;
			}
		}
	#endif

	CompileLightMaps( pDlg, Vis, pszWldName, bLibraryGood, NULL, TRUE );

	Vis.m_bUsedOldVisData = TRUE;
	Vis.m_nStaticLightingQualityLevel = VisHeader.nLMLevel;

	return TRUE;

_VISDIDNOTMATCH:

	if ( csReuseProblem.GetLength() > 0 )
	{
		pDlg->InfoString( "" );
		if ( bLibraryGood )
		{
			pDlg->InfoString( "Attempt to reuse static lighting data from an older library VIS" );
		}
		else
		{
			pDlg->InfoString( "Attempt to reuse static lighting data from an older local VIS" );
		}
		pDlg->InfoString( "failed.  %s", csReuseProblem );
		pDlg->InfoString( "" );
	}

	if ( paSrcKongMesh )
	{
		delete[] paSrcKongMesh;
	}

	if ( pConvertedData )
	{
		fang_Free( pConvertedData );
	}

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile );
	}

	return FALSE;
}


//
//
//
BOOL CApeToWorldFile::CreateLightmaps( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszFilename, u32 nLightMapDetail, f32 fLightMapMBs, f32 fSubSample, BOOL bForceVertexRadiosity, u32 nLightMapSkyLightDetail, CFColorRGB *pSkyHoriz, CFColorRGB *pSkyZenith, f32 fLightMapSkyLightItens ) 
{
	FASSERT( pDlg && pszFilename && m_paApeLMs && m_paApeVertColors );

	u32 i, ii, j;
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	char szTemp[512];

	u32 nStartTime = timeGetTime();

	// Build the name for this world, without path or extension
	char szWldName[32];
	for ( i = 0; pszFilename[i] != '.' && (s32)i < fclib_strlen( pszFilename ); i++ )
	{
		szWldName[i] = pszFilename[i];
	}
	szWldName[i] = 0;

	// Check for a VIS file that we can draw our lighting information from
	if ( !pDlg->m_bIgnoreVIS && !pDlg->m_bReusePerfectVISMatchOnly && CheckForReusableVIS( pDlg, szWldName, Vis, nLightMapDetail, fLightMapMBs ) )
	{
		// We found a vis file that we can use the data from
		return TRUE;
	}

#if LMG_INCLUDE_STATIC_APE_OBJECTS
	// Find the objects that will contribute to lightmaps (either by casting shadows, actually receiving lightmaps or both)
	BuildApeDependencyList( pDlg, Vis, bForceVertexRadiosity );

	// Compile object dependencies
	if ( m_nStaticObjectCount )
	{
		if ( !CompileApeDependencyList( pDlg, Vis ) )
		{
			return FALSE;
		}
	}
#endif // LMG_INCLUDE_STATIC_APE_OBJECTS

	pDlg->ResetSubOp( "Preparing Data for static lighting...", (m_nStaticObjectCount * 2) + 3 );
	pDlg->PostMessage( CDLG_UPDATE_CONTROLS );

	// Create a new lightmap generator
	CLightMapGen m_LightMapGen;

	// Set the parameters and build the lightmap
	m_LightMapGen.SetLocalDir( CPasmDlg::m_pStaticLocalDir->GetString() );
	//1  = radiosity, no raytrace,      1 iteration.
	//2  = radiosity, raytracing,       1 iteration.
	//3  = radiosity, raytracing,      10 iterations.
	//4  = radiosity, raytracing,      50 iterations.
	//5  = radiosity, raytracing,     100 iterations.
	//6  = radiosity, raytracing,   1,000 iterations.
	//7  = radiosity, raytracing,  10,000 iterations.
	//8  = radiosity, raytracing, 100,000 iterations.
	//9  = radiosity, raytracing, 200,000 iterations.
	//10 = radiosity, raytracing, 300,000 iterations.
	if ( bForceVertexRadiosity )
	{
		// PS2 forces vert rad and in that case, we want to use ray tracing.  But
		// in other situations, we want to use the simplest lighting available because
		// this is likely just processing of the WLD for dev purposes
//ARG - this doesn't work for us (causes weird vertex lighting values)
//ARG		#ifdef _MMI_TARGET_PS2
//ARG			// Should probably consider a higher level of iterations for PS2
//ARG			Vis.m_bUsedOldVisData = FALSE;
//ARG			Vis.m_nStaticLightingQualityLevel = 2;
//ARG			m_LightMapGen.SetParam( 2, fLightMapMBs, fSubSample, szWldName, TRUE );
//ARG		#else
			Vis.m_bUsedOldVisData = FALSE;
			Vis.m_nStaticLightingQualityLevel = 1;
			m_LightMapGen.SetParam( 1, fLightMapMBs, fSubSample, szWldName, TRUE );
//ARG		#endif
	}
	else
	{
		Vis.m_bUsedOldVisData = FALSE;
		Vis.m_nStaticLightingQualityLevel = nLightMapDetail;
		m_LightMapGen.SetParam( nLightMapDetail, fLightMapMBs, fSubSample, szWldName, TRUE );
	}

	if (nLightMapSkyLightDetail > 0 && pSkyHoriz && pSkyZenith)
	{
		m_LightMapGen.SetSkyLightParam(nLightMapSkyLightDetail, *pSkyHoriz, *pSkyZenith, fLightMapSkyLightItens);
	}

	pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );

	m_LightMapGen.InitWithWorldData( &Vis, pDlg, m_pFirstStaticObject );

	pDlg->InfoString( "Prepared data for static lighting phase (%f seconds)", (f32)(timeGetTime() - nStartTime) / 1000.f );

	if ( pDlg->m_bAbortCompile )
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + Vis.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Compiling aborted by user." );
		return FALSE;
	}

	nStartTime = timeGetTime();

	BOOL bRet = m_LightMapGen.Build( pDlg, FALSE, bForceVertexRadiosity );

	if ( pDlg->m_bAbortCompile )
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + Vis.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Compiling aborted by user." );
		return FALSE;
	}

	if ( !bRet )
	{
		pDlg->InfoString( "No static lights identified for use in this world." );
	}
	else
	{
		pDlg->InfoString( "Generated static lighting data (%f seconds)", (f32)(timeGetTime() - nStartTime) / 1000.f );
		// Now add the lightmap data into the mesh data
		u32 nMtlVtxOffset = 0;
		CLightingSolution::LN_MaterialExport_t *pMtl;
		for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
		{
			// Walk the segments
			CApeToKongFormat *pKong = ((CApeToKongFormat *)Vis.m_apVolKongMeshs[i]);
			if ( !pKong )
			{
				continue;
			}

			pKong->m_pKongMesh->KHeader.bHasLightmapsSTs = TRUE;

			for ( ii = 0; ii < pKong->m_pKongMesh->nSegmentCount; ii++ )
			{
				KongSeg_t *pKSeg = &pKong->m_pKongMesh->paSegs[ii];
				if ( !pKSeg )
				{
					continue;
				}

				// Walk the materials	
				j = 0;
				KongMat_t *pKMat = pKong->m_pKongMesh->GetFirstMaterial( pKSeg );
				while ( pKMat )
				{
					if ( !(pKMat->nMatFlags & KONGMAT_FLAGS_STATICALLY_LIT) )
					{
						pKMat = pKong->m_pKongMesh->GetNextMaterial( pKMat );
						continue;
					}
/*
					if ( (pKMat->pProperties->nFlags & APE_MAT_FLAGS_NO_LM_USE) && !(pKMat->pProperties->nFlags & APE_MAT_FLAGS_VERT_RADIOSITY) )
					{
						pKMat = pKong->m_pKongMesh->GetNextMaterial( pKMat );
						continue;
					}

					if ( (pKMat->pProperties->nFlags & APE_MAT_FLAGS_NO_DRAW) || (pKMat->pProperties->nShaderNum == APE_SHADER_TYPE_ADD_BASE) || (pKMat->pProperties->nShaderNum == APE_SHADER_TYPE_ADD_vBASE) )
					{
						pKMat = pKong->m_pKongMesh->GetNextMaterial( pKMat );
						continue;
					}
*/
					if ( (pKMat->pProperties->nFlags & APE_MAT_FLAGS_VERT_RADIOSITY) || bForceVertexRadiosity )
					{
						pKMat->nLightMapCount = 0;

						u32 nMtlVtxIdx;
						for ( nMtlVtxIdx=0; nMtlVtxIdx<(u32)(pKMat->nNumTris*3); nMtlVtxIdx++)
						{
							u32 nTriIdx = nMtlVtxIdx / 3;
							u32 nVertIdx = nMtlVtxIdx % 3;
			
							KongTri_t *pTri = pKong->m_pKongMesh->GetTriAtIdx( pKMat, nTriIdx );
							f32 fR, fG, fB, fA;

							m_LightMapGen.GetVtxClr(i, j, nMtlVtxIdx, fR, fG, fB, fA);

							// For the VIS (World) geometry, we overwrite the existing vert colors with the new colors
							pTri->apKongVerts[nVertIdx]->Color.fRed = fR;
							pTri->apKongVerts[nVertIdx]->Color.fGreen = fG;
							pTri->apKongVerts[nVertIdx]->Color.fBlue = fB;
							pTri->apKongVerts[nVertIdx]->Color.fAlpha = fA;
						}
					}
					else
					{
						pMtl = m_LightMapGen.ExportMaterial( i, j );
						if ( pMtl )
						{
							u32 nMtlLMIdx;
							u32 nLMVtxCount = 0;
							pKMat->nLightMapCount = 0;
							u32 nBaseSTSets = FShaders_aShaderRegs[pKMat->pProperties->nSurfaceShaderID].nUVCount;
							for ( nMtlLMIdx = 0; nMtlLMIdx < pMtl->nNumLightMaps; nMtlLMIdx++ )
							{
								// Copy lightmap texture name into kong material
								fclib_strcpy( pKMat->szLightMapTexname[pKMat->nLightMapCount], m_LightMapGen.GetLightMap_Volume( i, pMtl->anLightMapIdx[nMtlLMIdx] ) );
								pKMat->anLightMapMotifID[pKMat->nLightMapCount] = pMtl->anMotifList[nMtlLMIdx];

								u32 nMtlVtxIdx = 0;
								while ( nMtlVtxIdx < pMtl->nVtx && pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].nLMIndex == pMtl->anLightMapIdx[nMtlLMIdx] )
								{
									u32 nTriIdx = (pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].nVtxIndex - nMtlVtxOffset) / 3;
									u32 nVertIdx = (pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].nVtxIndex - nMtlVtxOffset) % 3;
									
									FASSERT( nTriIdx < pKMat->nNumTris );
									KongTri_t *pTri = pKong->m_pKongMesh->GetTriAtIdx( pKMat, nTriIdx );
									if ( !pTri ) 
									{
										rErrorLog.WriteErrorHeader( _ERROR_HEADING + Vis.m_sLastFileLoaded );
										rErrorLog.WriteErrorLine( "Error matching triangles to lightmap ST's." );
										return FALSE;
									}

									FASSERT( pTri );

									// Add the vertex info
									pTri->apKongVerts[nVertIdx]->nBaseUVCount = nBaseSTSets;
									pTri->apKongVerts[nVertIdx]->aUV[nBaseSTSets + pKMat->nLightMapCount].a[0] = pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].fU;
									pTri->apKongVerts[nVertIdx]->aUV[nBaseSTSets + pKMat->nLightMapCount].a[1] = pMtl->aUVSet[nMtlLMIdx][nMtlVtxIdx].fV;
									pTri->apKongVerts[nVertIdx]->nLightMapUVCount = (u8)pMtl->nNumLightMaps;

									nMtlVtxIdx++;
								}

								pKMat->nLightMapCount++;
							}

							m_LightMapGen.FreeMaterial( pMtl );
						}
					}

					// Increment the material vertex offset so that future materials do not index past their end
					nMtlVtxOffset += (pKMat->nNumTris * 3);

					pKMat = pKong->m_pKongMesh->GetNextMaterial( pKMat );
					j++;
				}
			}
		}

		// Copy over static lighting info for instanced objects
		LMG_Object_t *pLMObject = m_pFirstStaticObject;
		while ( pLMObject )
		{
			// Check each instance of this object
			LMG_ObjectInstance_t *pLMInstance = pLMObject->pFirstInstance;
			while ( pLMInstance )
			{
				CFWorldShapeInit *pShape = &Vis.m_InitFile.m_pShapeArray[pLMInstance->nApeObjectIdx];

//				DEVPRINTF( "Object %s:\n", pShape->m_pMesh->m_pszMeshName );

				// Determine whether or not we have vertex radiosity lighting
				BOOL bVertRadLight = !!(pShape->m_pMesh->m_nMeshInstFlags & FMESHINST_FLAG_VERT_RADIOSITY);
				bVertRadLight |= (bForceVertexRadiosity && (pShape->m_pMesh->m_nMeshInstFlags & FMESHINST_FLAG_LM) );

				if ( pLMInstance->bAcceptsLightmaps )
				{
					// This object should receive lightmaps, so get the LM name
					u32 iLMIdx = 0;
					while ( iLMIdx < KTWI_MAX_APE_OBJECT_LMS )
					{
						char *pszLMName = m_LightMapGen.GetLightMap_Volume( pLMInstance->nLMVolumeIdx, iLMIdx );
						if ( !pszLMName )
						{
							break;
						}

						strcpy( m_paApeLMs[pLMInstance->nApeObjectIdx].szLMTexture[iLMIdx], pszLMName );
//						DEVPRINTF( "Assigning lightmap %s to object %s.\n", pszLMName, pShape->m_pMesh->m_pszMeshName );
						m_paApeLMs[pLMInstance->nApeObjectIdx].nLMMotif[iLMIdx] = m_LightMapGen.GetLightMapMotif_Volume( pLMInstance->nLMVolumeIdx, iLMIdx );

						iLMIdx++;
					}
				}
				else if ( bVertRadLight )
				{
					u32 nLitVertCount = 0;
					// Since we're applying vertex radiosity, make sure the flag is set
					pShape->m_pMesh->m_nMeshInstFlags |= FMESHINST_FLAG_VERT_RADIOSITY;
					m_paApeVertColors[pLMInstance->nApeObjectIdx].nVertexCount = pLMObject->nVertexCount;
					m_paApeVertColors[pLMInstance->nApeObjectIdx].panVertColors = (u32 *)malloc( sizeof(u32) * pLMObject->nVertexCount );
					u32 *pColor = m_paApeVertColors[pLMInstance->nApeObjectIdx].panVertColors;
					memset( pColor, 0, sizeof( u32 ) * pLMObject->nVertexCount );
					for ( i = 0; i < pLMObject->nMaterialCount; i++ )
					{
						for ( ii = 0; ii < pLMObject->panMaterialVertexCount[i]; ii++ )
						{
							f32 fR, fG, fB, fA;
							m_LightMapGen.GetVtxClr(pLMInstance->nLMVolumeIdx, i, ii, fR, fG, fB, fA);
							u32 nVertIdx = m_LightMapGen.GetVtxUserData( pLMInstance->nLMVolumeIdx, i, ii );
							if ( nVertIdx >= pLMObject->nVertexCount )
							{
								pDlg->InfoString( "Error! LMG returned vert index of %d, which exceeds object vert count of %d.", nVertIdx, pLMObject->nVertexCount );
							}
							else
							{
								FASSERT( pColor[nVertIdx] == 0 );
								pColor[nVertIdx] = ((u8)(fR * 255.f) << 24) | ((u8)(fG * 255.f) << 16) | ((u8)(fB * 255.f) << 8) | ((u8)(fA * 255.f));
							}
							nLitVertCount++;
//							DEVPRINTF( " - Vertex %d Color R = %f, G = %f, B = %f, A = %f\n", nVertIdx, fR, fG, fB, fA );
						}
					}
					FASSERT( nLitVertCount == pLMObject->nVertexCount );
				}

				pLMInstance = pLMInstance->pNextInstance;
			}

			pLMObject = pLMObject->pNextObject;
		}

		pDlg->InfoString( "Building update batch files." );

		cchar **paszLightMapNameArray;
		u32  nLightMapCount = CompileLightMaps( pDlg, Vis, szWldName, FALSE, &paszLightMapNameArray );

		// Build a hacky little batch file to copy data up to the server:
		CString csName = *CPasmDlg::m_pStaticLocalDir + "\\VIS\\" + szWldName + ".bat";
		FILE *pFile = fopen( csName.GetBuffer(0), "w" );
		if ( pFile )
		{
			CString csSourceName, csDestName, csCommand;
			strcpy( szTemp, "echo off\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			csSourceName = *CPasmDlg::m_pStaticLocalDir + "\\VIS\\" + szWldName + ".vis";
			sprintf( szTemp, "if not exist %s goto _ERROR\n", csSourceName );
			strlwr( szTemp );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			// Make sure the directories exist in the server library
			sprintf( szTemp, "MD %s\\VIS\n", ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir );
			strupr( szTemp );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );
			sprintf( szTemp, "MD %s\\LIGHTMAPS\\%s\n", ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir, szWldName );
			strupr( szTemp );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			// Move the VIS file
			csSourceName = *CPasmDlg::m_pStaticLocalDir + "\\VIS\\" + szWldName + ".vis";
			csDestName = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\VIS\\" + szWldName + ".vis";
			csSourceName.MakeLower();
			csDestName.MakeLower();
			sprintf( szTemp, "move %s %s\n", csSourceName, csDestName );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			// Get rid of any old lightmaps
			sprintf( szTemp, "del /Q %s\\LIGHTMAPS\\%s\\*.*\n", ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir, szWldName );
			strlwr( szTemp );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			// Move the lightmaps and lightmap aid files
#if 0
			csSourceName = *CPasmDlg::m_pStaticLocalDir + "\\LightMaps\\" + szWldName + "\\*.tga";
			csDestName = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\LightMaps\\" + szWldName;
			csSourceName.MakeLower();
			csDestName.MakeLower();
			sprintf( szTemp, "move %s %s\n", csSourceName, csDestName );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );
			csSourceName = *CPasmDlg::m_pStaticLocalDir + "\\LightMaps\\" + szWldName + "\\*.aid";
			csDestName = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\LightMaps\\" + szWldName;
			csSourceName.MakeLower();
			csDestName.MakeLower();
			sprintf( szTemp, "move %s %s\n", csSourceName, csDestName );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );
#else
			for ( i = 0; i < (u32)nLightMapCount; i++ )
			{
				csSourceName = *CPasmDlg::m_pStaticLocalDir + "\\LightMaps\\" + szWldName + "\\" + paszLightMapNameArray[i] + ".tga";
				csDestName = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\LightMaps\\" + szWldName + "\\" + paszLightMapNameArray[i] + ".tga";
				csSourceName.MakeLower();
				csDestName.MakeLower();
				sprintf( szTemp, "move %s %s\n", csSourceName, csDestName );
				fwrite( szTemp, strlen( szTemp ), 1, pFile );
				csSourceName = *CPasmDlg::m_pStaticLocalDir + "\\LightMaps\\" + szWldName + "\\" + paszLightMapNameArray[i] + ".aid";
				csDestName = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\LightMaps\\" + szWldName + "\\" + paszLightMapNameArray[i] + ".aid";
				csSourceName.MakeLower();
				csDestName.MakeLower();
				sprintf( szTemp, "move %s %s\n", csSourceName, csDestName );
				fwrite( szTemp, strlen( szTemp ), 1, pFile );
			}
#endif

			strcpy( szTemp, "echo OPERATION SUCCESSFUL.\n\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );
			strcpy( szTemp, "goto _END\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			strcpy( szTemp, ":_ERROR\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );
			strcpy( szTemp, "echo ERROR - Vis file not found.\nOPERATION ABORTED.\n\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			strcpy( szTemp, ":_END\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );
			strcpy( szTemp, "pause\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );
			strcpy( szTemp, "echo on\n" );
			fwrite( szTemp, strlen( szTemp ), 1, pFile );

			fclose( pFile );
		}
	}

	// Free the dependency list
	FreeApeDependencyList();

	return TRUE;
}


//
//
//
u32 CApeToWorldFile::CompileLightMaps( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszWLDName, BOOL bUseLibrary, cchar ***paszFileName, BOOL bUseMeshInit/*=FALSE*/ )
{
	u32 i, j, k, nAsset = 0;
	static const char *__apFileToCompile[_MAX_LM_COMPILE_FILES];
	u32  nPrecompileFileCount = 0;
	CFileInfo FileInfo;
	char szTemp[128];

	FASSERT( pDlg && pszWLDName );

	if ( paszFileName )
	{
		(*paszFileName) = __apFileToCompile;
	}

	// Determine all of the lightmaps used by the world geo
	for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
	{
		// Walk the segments
		CApeToKongFormat *pKong = ((CApeToKongFormat *)Vis.m_apVolKongMeshs[i]);
		if ( !pKong )
		{
			continue;
		}

		for ( j = 0; j < pKong->m_pKongMesh->nSegmentCount; j++ )
		{
			KongSeg_t *pKSeg = &pKong->m_pKongMesh->paSegs[j];
			if ( !pKSeg )
			{
				continue;
			}

			// Walk the materials	
			KongMat_t *pKMat = pKong->m_pKongMesh->GetFirstMaterial( pKSeg );
			while ( pKMat )
			{
				if ( pKMat->pProperties->nFlags & APE_MAT_FLAGS_NO_LM_USE )
				{
					pKMat = pKong->m_pKongMesh->GetNextMaterial( pKMat );
					continue;
				}

				for ( k = 0; k < pKMat->nLightMapCount; k++ )
				{
					// See if there is a match for this asset
					for ( nAsset = 0; (s32)nAsset < nPrecompileFileCount; nAsset++ )
					{
						if ( stricmp( __apFileToCompile[nAsset], pKMat->szLightMapTexname[k] ) == 0 )
						{
							break;
						}
					}

					// If we didn't find a match, add it to the list
					if ( nAsset == nPrecompileFileCount && nPrecompileFileCount < _MAX_LM_COMPILE_FILES )
					{
						__apFileToCompile[nPrecompileFileCount++] = pKMat->szLightMapTexname[k];
					}
				}

				pKMat = pKong->m_pKongMesh->GetNextMaterial( pKMat );
			}
		}
	}

	if ( !bUseMeshInit )
	{
		// Determine all of the lightmaps used by the instanced geo
		LMG_Object_t *pLMObject = m_pFirstStaticObject;
		while ( pLMObject )
		{
			LMG_ObjectInstance_t *pLMInstance = pLMObject->pFirstInstance;
			while ( pLMInstance )
			{
				CFWorldShapeInit *pShape = &Vis.m_InitFile.m_pShapeArray[pLMInstance->nApeObjectIdx];

				for ( i = 0; i < KTWI_MAX_APE_OBJECT_LMS; i++ )
				{
					cchar *pszLMName;
					if ( m_paApeLMs )
					{
						pszLMName = m_paApeLMs[pLMInstance->nApeObjectIdx].szLMTexture[i];
					}
					else
					{
						pszLMName = pShape->m_pMesh->m_papszLightMapName[i];
					}

					if ( !pszLMName || pszLMName[0] == 0 )
					{
						continue;
					}

					// See if there is a match for this asset
					for ( nAsset = 0; (s32)nAsset < nPrecompileFileCount; nAsset++ )
					{
						if ( stricmp( __apFileToCompile[nAsset], pszLMName ) == 0 )
						{
							break;
						}
					}

					// If we didn't find a match, add it to the list
					if ( nAsset == nPrecompileFileCount && nPrecompileFileCount < _MAX_LM_COMPILE_FILES )
					{
						__apFileToCompile[nPrecompileFileCount++] = pszLMName;
					}
				}

				pLMInstance = pLMInstance->pNextInstance;
			}

			pLMObject = pLMObject->pNextObject;
		}
	}
	else
	{
		FASSERT( Vis.m_InitFile.m_pHeader );

		for( i = 0; i < Vis.m_InitFile.m_pHeader->nNumInitStructs; i++ )
		{
			if( Vis.m_InitFile.m_pShapeArray[i].m_nShapeType != FWORLD_SHAPETYPE_MESH )
			{
				continue;
			}

			// Set pointers to this object's lightmaps, if any
			for( j = 0; j < FWORLD_MAX_MESH_SHAPE_LIGHTMAPS; j++ )
			{
				if( Vis.m_InitFile.m_pShapeArray[i].m_pMesh->m_papszLightMapName[j] )
				{
					__apFileToCompile[nPrecompileFileCount++] = Vis.m_InitFile.m_pShapeArray[i].m_pMesh->m_papszLightMapName[j];
				}
			}
		}
	}

	// If we didn't find any lightmaps, don't bother going through the compile process
	if ( !nPrecompileFileCount )
	{
		return 0;
	}

	sprintf( szTemp, "Compiling %d LightMap TGAs...", nPrecompileFileCount );
	pDlg->ResetSubOp( szTemp, nPrecompileFileCount );
	pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	pDlg->InfoString( "%s", szTemp );

	// Force compilation of the lightmaps
	BOOL bPriorForceSetting = pDlg->m_bForceCompile;
	pDlg->m_bForceCompile = TRUE;
	pDlg->m_bCompilingPASMGeneratedAsset = TRUE;
//	CString csDir = m_LightMapGen.GetLightMapDirectory();

	for ( nAsset = 0; nAsset < nPrecompileFileCount; nAsset++ )
	{
		CString csTemp;
		if ( bUseLibrary )
		{
			csTemp = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\LIGHTMAPS\\" + pszWLDName + "\\" + __apFileToCompile[nAsset] + ".tga";
		}
		else
		{
			csTemp = *CPasmDlg::m_pStaticLocalDir + "\\LIGHTMAPS\\" + pszWLDName + "\\" + __apFileToCompile[nAsset] + ".tga";
		}
//		CString csTemp = csDir + apFileToCompile[nAsset] + ".tga";

		FileInfo.Create( csTemp );
		if ( FileInfo.GetLength() ) 
		{
			CallConverter( &FileInfo, pDlg, j );
		}
		pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
	}

	pDlg->m_bCompilingPASMGeneratedAsset = FALSE;
	pDlg->m_bForceCompile = bPriorForceSetting;

	return nPrecompileFileCount;
}



//
//
//
u32 CApeToWorldFile::GetSizeOfConvertedFile( void ) 
{

	if ( !m_bConverted ) 
	{
		return 0;
	}
	return m_FileHeader.nNumBytes;
}

//
//
//
u32 CApeToWorldFile::GetWorldCRC( CKongToVisFile &Vis ) 
{
	if ( !m_bConverted ) 
	{
		return 0;
	}

	u32 nReturnCRC = fmath_Crc32( m_nMeshCRC, (u8 *)&m_FileHeader, sizeof( FData_WorldFileHeader_t ) );

	nReturnCRC = Vis.GetCRC( nReturnCRC );

	if ( m_pStreamingData )
	{
		nReturnCRC = fmath_Crc32( nReturnCRC, (u8 *)m_pStreamingData, m_nStreamingDataBytes );
	}

	nReturnCRC = Vis.m_InitFile.GetCRC( nReturnCRC );

	return nReturnCRC;
}


//
//
//
BOOL CApeToWorldFile::WriteWorldFile( CCompileDlg *pDlg, cchar *pszFilename, CKongToVisFile &Vis, TargetPlatform_e nPlatform, FILE *pFileStream/*=NULL*/ ) 
{
	if ( !m_bConverted ) 
	{
		return FALSE;
	}

	// open the file if pFileStream == NULL
	BOOL bCloseFile = FALSE;
	if ( !pFileStream ) 
	{
		if ( !pszFilename ) 
		{
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if ( !pFileStream ) 
		{
			return FALSE;
		}
		bCloseFile = TRUE;	
	}

	u32 i, nValue;
	u8 nZero = 0;
		
	FData_WorldFileHeader_t Header;
	Header = m_FileHeader;

	if ( nPlatform == PASM_TARGET_GC ) 
	{
		// endianize our header
		Header.ChangeEndian();
	}

	// write out our header
	fwrite( &Header, sizeof( FData_WorldFileHeader_t ), 1, pFileStream );

	// write out the mesh offsets
	for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
	{
		nValue = m_anMeshOffsets[i];
		if ( nPlatform == PASM_TARGET_GC ) 
		{
			nValue = fang_ConvertEndian( nValue );
		}
		fwrite( &nValue, sizeof( u32 ), 1, pFileStream );
	}

	// write out the mesh sizes
	for ( i = 0; i < Vis.m_nStatsNumMeshes; i++ ) 
	{
		nValue = m_anMeshSizes[i];
		if ( nPlatform == PASM_TARGET_GC ) 
		{
			nValue = fang_ConvertEndian( nValue );
		}
		fwrite( &nValue, sizeof( u32 ), 1, pFileStream );
	}

	// write out our mesh (pad so that they start on FDATA_WORLD_MESH_ALIGNMENT byte boundary)
	if ( m_anMeshPadBytes[0] ) 
	{
		utils_PadFileSize( pFileStream, m_anMeshPadBytes[0] );
	}

	// now we need to copy the entire mesh temp file to the master file
	FILE *pMeshFile;
	pMeshFile = _tfopen( m_sTempMeshFile, _T("rb") );
	if ( !pMeshFile ) 
	{
		return FALSE;
	}

	if ( !CMasterFileCompile::CopyFile( pFileStream, pMeshFile, 0 ) ) 
	{
		fclose( pMeshFile );
		return FALSE;
	}
	fclose( pMeshFile );
	
	// write out our world
	if ( m_nWorldPadBytes ) 
	{
		fwrite( &nZero, 1, m_nWorldPadBytes, pFileStream );
	}
	Vis.WriteWorldFile( pszFilename, nPlatform == PASM_TARGET_GC, pFileStream );

	// write out our streamed data
	if ( m_nStreamingDataPadBytes ) 
	{
		fwrite( &nZero, 1, m_nStreamingDataPadBytes, pFileStream );
	}
	fwrite( m_pStreamingData, m_nStreamingDataBytes, 1, pFileStream );

	// write out our shape init data
	if ( m_nWorldInitPadBytes ) 
	{
		fwrite( &nZero, 1, m_nWorldInitPadBytes, pFileStream );
	}
	Vis.m_InitFile.WriteToFile( pDlg, pszFilename, nPlatform, pFileStream );
	
	// close file, if need be
	if ( bCloseFile ) 
	{
		fclose( pFileStream );
	}

	return TRUE;
}


//
//
//
void CApeToWorldFile::FreeData( void ) 
{
	m_bConverted = FALSE;
	fang_MemZero( &m_FileHeader, sizeof( FData_WorldFileHeader_t ) );

	m_anMeshOffsets.RemoveAll();
	m_anMeshSizes.RemoveAll();
	m_anMeshPadBytes.RemoveAll();
	m_nWorldPadBytes = 0;
	m_nWorldInitPadBytes = 0;

	FreeApeDependencyList();

	MLManager.FreeStreamingData();
	m_nStreamingDataBytes = 0;
	m_pStreamingData = NULL;

	if ( m_paApeLMs )
	{
		free( m_paApeLMs );
		m_paApeLMs = NULL;
	}

	if ( m_paApeVertColors )
	{
		u32 i;
		for ( i = 0; i < m_nApeObjectCount; i++ )
		{
			if ( m_paApeVertColors[i].panVertColors )
			{
				free( m_paApeVertColors[i].panVertColors );
				m_paApeVertColors[i].panVertColors = NULL;
			}
		}

		free( m_paApeVertColors );
	}

	if ( !m_sTempMeshFile.IsEmpty() ) 
	{
		// delete our temp mesh file
		DeleteFile( m_sTempMeshFile );
		m_sTempMeshFile.Empty();
	}
	m_nTempMeshFileSize = 0;
}
