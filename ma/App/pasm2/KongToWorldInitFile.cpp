//////////////////////////////////////////////////////////////////////////////////////
// KongToWorldInitFile.cpp - 
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
// 04/19/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "fclib.h"
#include "KongToWorldInitFile.h"
#include "ErrorLog.h"
#include "utils.h"
#include "ConvertCsvfiles.h"
#include "CompileDlg.h"

#define _SIZE_OF_FIXUP_ENTRY		16
#define _ERROR_HEADING				"KONG->WORLD INIT FILE COMPILER "

#define _COLOR_STREAM_BUFFER		32

//
//
typedef struct 
{
	CFWorldShapeLine		m_Line;
	CFWorldShapeSpline		m_Spline;
	CFWorldShapeBox			m_Box;
	CFWorldShapeSphere		m_Sphere;
	CFWorldShapeCylinder	m_Cylinder;
	CFWorldShapeMesh		m_Mesh;
} _WorldShapeSize_t;


//
//
//
CKongToWorldInitFile::CKongToWorldInitFile() 
{
	m_nNumBytesAllocated = 0;
	m_pMemAllocated = NULL;
	m_pHeader = NULL;
	m_pShapeArray = NULL;
	m_pWorkingMem = NULL;
	m_nStatsNumShapes = 0;
	m_nStatsShapeBytes = 0;
	m_bPointersAreOffsets = FALSE;
	m_bUsingOldInit = FALSE;
	m_aStrings.RemoveAll();
}


//
//
//
CKongToWorldInitFile::~CKongToWorldInitFile() 
{
	FreeData();
}


//
//
//
BOOL CKongToWorldInitFile::ConvertKongFile( CApeToKongFormat *pKong, ApeObjectLMs_t *pApeLMs, ApeObjectVertColors_t *paApeVertColors ) 
{
	FreeData();

	if( !pKong ) 
	{
		return FALSE;
	}

	m_bPointersAreOffsets = FALSE;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	if( !pKong->m_pKongMesh->bWorldFile ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Only world files have an embedded init file." );
		return FALSE;
	}

	////////////////////////////////////////
	// estimate how much memory we will need
	u32 i, j, nStringLen, nBytesNeeded, nNumApeObjects, nNumApeShapes, nNumErrors;
	CFWorldShapeInit *pShapeInit;
	char *pszStringEntry;
	CFVec3 Dir, Up;
	CString sError;
			
	nNumApeObjects = pKong->GetNumObjects();
	nBytesNeeded = nNumApeObjects * sizeof( CFWorldShapeInit );
	
	nNumApeShapes = pKong->GetNumShapes();
	nBytesNeeded += nNumApeShapes * sizeof( CFWorldShapeInit );

	m_nStatsNumShapes = nNumApeObjects + nNumApeShapes;
	if( !m_nStatsNumShapes ) 
	{
		// nothing needed, but don't fail
		return TRUE;
	}

	if ( paApeVertColors != NULL )
	{
		for( i=0; i < nNumApeObjects; i++ ) 
		{
			if ( paApeVertColors[i].nVertexCount )
			{
				nBytesNeeded += paApeVertColors[i].nVertexCount * sizeof( u32 );
				nBytesNeeded += _COLOR_STREAM_BUFFER;  // Padding
			}
		}
	}

	nBytesNeeded += _SIZE_OF_FIXUP_ENTRY * m_nStatsNumShapes;
	nBytesNeeded += sizeof( _WorldShapeSize_t ) * m_nStatsNumShapes;
	// assume that each shape has 32 tables each with 8 fields
	nBytesNeeded += sizeof( FDataGamFile_Header_t ) * m_nStatsNumShapes * 32;
	nBytesNeeded += sizeof( FDataGamFile_Table_t ) * m_nStatsNumShapes * (32 * 8);
	nBytesNeeded += (1024 * m_nStatsNumShapes);// this should give us plenty of memory
	nBytesNeeded = FMATH_BYTE_ALIGN_UP( nBytesNeeded, 4096 );// this should give us plenty of memory

	////////////////////////////////
	// allocate memory & zero it out
	m_pMemAllocated = (u8 *)fang_MallocAndZero( nBytesNeeded, 16 );
	if( !m_pMemAllocated ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not allocate enough memory for world init file image" );
		return FALSE;
	}
	m_nNumBytesAllocated = nBytesNeeded;

	//////////////////////
	// fix up our pointers
	m_pHeader = (FData_WorldInitHeader_t *)m_pMemAllocated;
	m_nStatsShapeBytes = 0;
	m_pShapeArray = (CFWorldShapeInit *)&m_pHeader[1];
	m_pWorkingMem = (u8 *)&m_pShapeArray[m_nStatsNumShapes];
	m_aStrings.RemoveAll();
#if 0// the error checking is done in the exporter so that we don't have to refixup the parent indices
	// set our working shape ptr to the very last element so we can throw out error cases easier
	pShapeInit = &m_pShapeArray[m_nStatsNumShapes];
#else
	pShapeInit = &m_pShapeArray[0];
#endif

	nNumErrors = 0;

	// Pack in the mesh shapes
	nNumErrors += !StoreMeshShapes( pKong, nNumApeObjects, pShapeInit );

	// Increment by the number of mesh shapes
	pShapeInit += nNumApeObjects;

	// Pack in the rest of the shapes (must happen after the meshes get processed
	nNumErrors += !StoreNonMeshShapes( pKong, nNumApeShapes, pShapeInit );

	m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );

	// Pack in the lightmaps, if there are any
	if ( pApeLMs )
	{
		for( i = 0; i < nNumApeObjects; i++ ) 
		{
			for ( j = 0; j < KTWI_MAX_APE_OBJECT_LMS; j++ )
			{
				// see if this object name has already been added to the string table
				pszStringEntry = NULL;
				if ( pApeLMs[i].szLMTexture[j][0] )
				{
					pszStringEntry = (char *)IsStringAlreadyInTable( pApeLMs[i].szLMTexture[j] );
					if( !pszStringEntry ) 
					{
						// string not found, copy it and add it to the table
						pszStringEntry = (char *)m_pWorkingMem;
						nStringLen = fclib_strlen( pApeLMs[i].szLMTexture[j] ) + 1;
						m_pWorkingMem += nStringLen;
						fang_MemCopy( pszStringEntry, pApeLMs[i].szLMTexture[j], nStringLen );
						m_aStrings.Add( pszStringEntry );
					}
				}

				m_pShapeArray[i].m_pMesh->m_papszLightMapName[j] = pszStringEntry;
				m_pShapeArray[i].m_pMesh->m_anLightMapMotif[j] = pApeLMs[i].nLMMotif[j];
			}
		}
	}

	m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );

	// Add the ape object vertex colors  (This must be last!!!)
	for( i=0; i < nNumApeObjects; i++ ) 
	{
		if ( paApeVertColors != NULL && paApeVertColors[i].nVertexCount )
		{
			m_pShapeArray[i].m_pMesh->m_nColorStreamCount = 1;

			m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );

			m_pShapeArray[i].m_pMesh->m_paColorStreams = (ColorStream_t *)m_pWorkingMem;
			m_pWorkingMem = (u8 *)m_pWorkingMem + sizeof( ColorStream_t );

			m_pShapeArray[i].m_pMesh->m_paColorStreams->nVBIndex = paApeVertColors[i].nVertexCount;
			m_pShapeArray[i].m_pMesh->m_paColorStreams->paVertexColors = m_pWorkingMem;
			memcpy( m_pWorkingMem, paApeVertColors[i].panVertColors, paApeVertColors[i].nVertexCount * sizeof( u32 ) );
			m_pWorkingMem = (u8 *)m_pWorkingMem + (paApeVertColors[i].nVertexCount * sizeof( u32 )) + _COLOR_STREAM_BUFFER;
		}
		else
		{
			m_pShapeArray[i].m_pMesh->m_nColorStreamCount = 0;
			m_pShapeArray[i].m_pMesh->m_paColorStreams = NULL;
		}
	}
		
	if( nNumErrors ) 
	{
		// we encountered errors, we have to fail
		FreeData();
		return FALSE;
	}

	FASSERT( (u32)m_pWorkingMem <= (u32)&m_pMemAllocated[m_nNumBytesAllocated] );

	FASSERT( m_pHeader->nNumInitStructs == m_nStatsNumShapes );

	m_bUsingOldInit = FALSE;

	// calculate our compiled size
	m_nStatsShapeBytes = (u32)m_pWorkingMem - (u32)m_pHeader;

	return TRUE;
}


//
//
//
BOOL CKongToWorldInitFile::ConvertKongFileUsingInit( CCompileDlg *pDlg, CApeToKongFormat *pKong, void *pInitData, u32 nInitDataSize ) 
{
	FreeData();

	if( !pKong || !pInitData || !pDlg ) 
	{
		return FALSE;
	}

	m_bPointersAreOffsets = FALSE;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	if( !pKong->m_pKongMesh->bWorldFile ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Only world files have an embedded init file." );
		return FALSE;
	}

	////////////////////////////////////////
	// Estimate how much memory we will need
	u32 i, j, k, nStringLen, nBytesNeeded, nNumApeObjects, nNumApeShapes, nNumErrors;
	CFWorldShapeInit *pShapeInit;
	char *pszStringEntry;
	CFVec3 Dir, Up;
	CString sError;

	nNumApeObjects = pKong->GetNumObjects();
	nBytesNeeded = nNumApeObjects * sizeof( CFWorldShapeInit );
	
	nNumApeShapes = pKong->GetNumShapes();
	nBytesNeeded += nNumApeShapes * sizeof( CFWorldShapeInit );

	m_nStatsNumShapes = nNumApeObjects + nNumApeShapes;
	if( !m_nStatsNumShapes ) 
	{
		// nothing needed, but don't fail
		return TRUE;
	}

	nBytesNeeded += _SIZE_OF_FIXUP_ENTRY * m_nStatsNumShapes;
	nBytesNeeded += sizeof( _WorldShapeSize_t ) * m_nStatsNumShapes;
	// assume that each shape has 32 tables each with 8 fields
	nBytesNeeded += sizeof( FDataGamFile_Header_t ) * m_nStatsNumShapes * 32;
	nBytesNeeded += sizeof( FDataGamFile_Table_t ) * m_nStatsNumShapes * (32 * 8);
	nBytesNeeded += (1024 * m_nStatsNumShapes);// this should give us plenty of memory
	nBytesNeeded = FMATH_BYTE_ALIGN_UP( nBytesNeeded, 4096 );// this should give us plenty of memory

	// This is just to be really safe
	if ( nBytesNeeded < nInitDataSize * 2 )
	{
		nBytesNeeded = nInitDataSize * 2;
	}
			
	////////////////////////////////
	// allocate memory & zero it out
	m_pMemAllocated = (u8 *)fang_MallocAndZero( nBytesNeeded, 16 );
	if( !m_pMemAllocated ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not allocate enough memory for world init file image" );
		return FALSE;
	}
	m_nNumBytesAllocated = nBytesNeeded;

	//////////////////////
	// fix up our pointers
	m_pHeader = (FData_WorldInitHeader_t *)m_pMemAllocated;
	m_nStatsShapeBytes = 0;
	m_pShapeArray = (CFWorldShapeInit *)&m_pHeader[1];
	m_pWorkingMem = (u8 *)&m_pShapeArray[m_nStatsNumShapes];
	m_aStrings.RemoveAll();
	pShapeInit = &m_pShapeArray[0];

	// Establish pointers to the old data
	FData_WorldInitHeader_t *pOldHeader = (FData_WorldInitHeader_t *)pInitData;
	CFWorldShapeInit *pOldShapeArray = (CFWorldShapeInit *)(pOldHeader + 1);

	// Reset the error counter
	nNumErrors = 0;

	// Pack in the mesh shapes
	nNumErrors += !StoreMeshShapes( pKong, nNumApeObjects, pShapeInit );

	// Increment by the number of mesh shapes
	pShapeInit += nNumApeObjects;

	// Pack in the rest of the shapes (must happen after the meshes get processed
	nNumErrors += !StoreNonMeshShapes( pKong, nNumApeShapes, pShapeInit );

	// Pack in the static lighting info, if there are any (First pass is lightmapping, last must be vert rad - because we reset some of the data on export)
	u32 nLightType, nCurrentlyProcessing = FMESHINST_FLAG_LM;
	for( nLightType = 0; nLightType < 2; nLightType++ )
	{
		if ( nLightType == 1 )
		{
			nCurrentlyProcessing = FMESHINST_FLAG_VERT_RADIOSITY;
		}

		for( i = 0; i < nNumApeObjects; i++ ) 
		{
			if( m_pShapeArray[i].m_nShapeType != FWORLD_SHAPETYPE_MESH )
			{
				// Not a mesh
				continue;
			}

			// Get lighting style for this mesh
			u32 nNewMeshStaticLighting = m_pShapeArray[i].m_pMesh->m_nMeshInstFlags & nCurrentlyProcessing;

			// Does this mesh have this type of static lighting?
			if( nNewMeshStaticLighting == 0 )
			{
				// This mesh is not statically lit, so no need to look to the old init data for static lighting info
				continue;
			}

			for( j = 0; j < pOldHeader->nNumInitStructs; j++ )
			{
				if( pOldShapeArray[j].m_nShapeType != FWORLD_SHAPETYPE_MESH )
				{
					continue;
				}

				if( nNewMeshStaticLighting != (pOldShapeArray[j].m_pMesh->m_nMeshInstFlags & (FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY)) )
				{
					// The older mesh does not have the same static lighting technique
					continue;
				}

				if( stricmp( m_pShapeArray[i].m_pMesh->m_pszMeshName, pOldShapeArray[j].m_pMesh->m_pszMeshName ) != 0 )
				{
					// Mesh names do not match
					continue;
				}

				CFVec3 vDiff = m_pShapeArray[i].m_Mtx43.m_vPos - pOldShapeArray[j].m_Mtx43.m_vPos;
				if( vDiff.Mag2() > 0.01f )
				{
					// This mesh is not in the same position
					continue;
				}

				if (   m_pShapeArray[i].m_Mtx43.m_vFront.Dot( pOldShapeArray[j].m_Mtx43.m_vFront ) < 0.9848f 
					|| m_pShapeArray[i].m_Mtx43.m_vRight.Dot( pOldShapeArray[j].m_Mtx43.m_vRight ) < 0.9848f )
				{
					// This mesh is oriented in a significantly different manner
					continue;
				}

				// This is a good mesh match.  Let's transfer the data

				if( nNewMeshStaticLighting & FMESHINST_FLAG_LM )
				{
					// Copy over the lightmaps
					for( k = 0; k < FWORLD_MAX_MESH_SHAPE_LIGHTMAPS; k++ )
					{
						// see if this object name has already been added to the string table
						pszStringEntry = NULL;
						if( pOldShapeArray[j].m_pMesh->m_papszLightMapName[k] )
						{
							pszStringEntry = (char *)IsStringAlreadyInTable( pOldShapeArray[j].m_pMesh->m_papszLightMapName[k] );
							if( !pszStringEntry ) 
							{
								// string not found, copy it and add it to the table
								pszStringEntry = (char *)m_pWorkingMem;
								nStringLen = fclib_strlen( pOldShapeArray[j].m_pMesh->m_papszLightMapName[k] ) + 1;
								m_pWorkingMem += nStringLen;
								fang_MemCopy( pszStringEntry, pOldShapeArray[j].m_pMesh->m_papszLightMapName[k], nStringLen );
								m_aStrings.Add( pszStringEntry );
							}
						}

						m_pShapeArray[i].m_pMesh->m_papszLightMapName[k] = pszStringEntry;
						m_pShapeArray[i].m_pMesh->m_anLightMapMotif[k] = pOldShapeArray[j].m_pMesh->m_anLightMapMotif[k];
					}
				}
				else
				{
					m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );

					m_pShapeArray[i].m_pMesh->m_nColorStreamCount = pOldShapeArray[j].m_pMesh->m_nColorStreamCount;
					m_pShapeArray[i].m_pMesh->m_paColorStreams = (ColorStream_t *)m_pWorkingMem;
					m_pWorkingMem += (sizeof( ColorStream_t ) * pOldShapeArray[j].m_pMesh->m_nColorStreamCount);

					// Copy over the vertex radiosity light streams
					for( k = 0; k < pOldShapeArray[j].m_pMesh->m_nColorStreamCount; k++ )
					{
						ColorStream_t *pSrc  = &pOldShapeArray[j].m_pMesh->m_paColorStreams[k];
						ColorStream_t *pDest = &m_pShapeArray[i].m_pMesh->m_paColorStreams[k];

	//					pDest->nVBIndex       = pSrc->nVBIndex;
	//					pDest->nColorCount    = pSrc->nColorCount;
	//					pDest->paVertexColors = m_pWorkingMem;

						// These are the color streams prior to conversion to platform specific format
						pDest->nVBIndex       = pSrc->nVBIndex;
						pDest->nColorCount    = pSrc->nColorCount;
						pDest->paVertexColors = m_pWorkingMem;
						memcpy( pDest->paVertexColors, pSrc->paVertexColors, pSrc->nVBIndex * sizeof( u32 ) );
						m_pWorkingMem = (u8 *)m_pWorkingMem + (pSrc->nVBIndex * sizeof( u32 )) + _COLOR_STREAM_BUFFER;
					}
				}

				break;
			}

			// Did we find a match
			if( j == pOldHeader->nNumInitStructs )
			{
				// No match found... wahhhh!  Remove the static lighting from this object and post a warning
				m_pShapeArray[i].m_pMesh->m_nMeshInstFlags &= ~(FMESHINST_FLAG_LM|FMESHINST_FLAG_VERT_RADIOSITY);
				pDlg->InfoString( "Unable to find a mesh init to match mesh %s. Removing static lighting.", m_pShapeArray[i].m_pMesh->m_pszMeshName );
			}
		}
	}

	if( nNumErrors ) 
	{
		// we encountered errors, we have to fail
		FreeData();
		return FALSE;
	}

	FASSERT( (u32)m_pWorkingMem <= (u32)&m_pMemAllocated[m_nNumBytesAllocated] );

	FASSERT( m_pHeader->nNumInitStructs == m_nStatsNumShapes );

	m_bUsingOldInit = TRUE;

	// calculate our compiled size
	m_nStatsShapeBytes = (u32)m_pWorkingMem - (u32)m_pHeader;

	return TRUE;
}


//
//
//
BOOL CKongToWorldInitFile::StoreMeshShapes( CApeToKongFormat *pKong, u32 nNumApeObjects, CFWorldShapeInit *pShapeInit )
{
	u32 i, j, nNumErrors = 0, nStringLen;
	ApeObject_t *pApeOb;
	BOOL bStringIsEmpty;
	char *pszStringEntry;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();
	CString sError;

	//////////////////////////////
	// run through the ape objects
	/////////////////////////////////////////////////////
	// THIS MUST HAPPEN BEFORE PROCESSING THE SHAPE ARRAY
	// OR ELSE THE PARENT INDICES WILL BE OFF
	////////////////////////////////////////
	for( i = 0; i < nNumApeObjects; i++ ) 
	{
		pApeOb = pKong->GetObjectInfo( i );
		
		m_pHeader->nNumInitStructs++;

		// fill in the common fields
		pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_MESH;
		pShapeInit->m_Mtx43 = pApeOb->Orientation;
		pShapeInit->m_nParentShapeIndex = pApeOb->nParentIndex - 1;
		// copy the user data to our memory buffer
		pShapeInit->m_pGameData = ConvertMaxUserPropsToGameData( (char *)&pApeOb[1], pApeOb->nBytesOfUserData, bStringIsEmpty );

		// check for error conditions
		if( !bStringIsEmpty && !pShapeInit->m_pGameData && pApeOb->nBytesOfUserData ) 
		{
			// the user supplied user data, but it didn't get converted for some reason
			if( nNumErrors == 0 ) 
			{
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
			}
			nNumErrors++;
			sError.Format( "Error in user data for max node: obj_%s.", pApeOb->szName );
			rErrorLog.WriteErrorLine( sError );
		}
		
		// grab a CFWorldShapeMesh
		m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );
		pShapeInit->m_pMesh = (CFWorldShapeMesh *)m_pWorkingMem;
		m_pWorkingMem += sizeof( CFWorldShapeMesh );

		// fill in the CFWorldShapeMesh
		pShapeInit->m_pMesh->m_nMeshInstFlags = utils_ConvertToMeshInitFlags( pApeOb->nFlags );
		pShapeInit->m_pMesh->m_fCullDist2 = FMATH_MAX_FLOAT;
		if( pApeOb->fCullDistance >= 1.0f ) 
		{
			pShapeInit->m_pMesh->m_fCullDist2 = pApeOb->fCullDistance * pApeOb->fCullDistance;
		}

		// see if this object name has already been added to the string table
		pszStringEntry = (char *)IsStringAlreadyInTable( pApeOb->szName );
		if( !pszStringEntry ) 
		{
			// string not found, copy it and add it to the table
			pszStringEntry = (char *)m_pWorkingMem;
			nStringLen = fclib_strlen( pApeOb->szName ) + 1;
			m_pWorkingMem += nStringLen;
			fang_MemCopy( pszStringEntry, pApeOb->szName, nStringLen );
			m_aStrings.Add( pszStringEntry );
		}

		pShapeInit->m_pMesh->m_pszMeshName = pszStringEntry;

		if ( pApeOb->nFlags & APE_OB_FLAG_TINT )
		{
			pShapeInit->m_pMesh->m_TintRGB = pApeOb->TintRGB;
		}
		else
		{
			pShapeInit->m_pMesh->m_TintRGB.Set( 1.f, 1.f, 1.f );
		}

		for ( j = 0; j < KTWI_MAX_APE_OBJECT_LMS; j++ )
		{
			pShapeInit->m_pMesh->m_papszLightMapName[j] = NULL;
			pShapeInit->m_pMesh->m_anLightMapMotif[j] = 0;
		}

		pShapeInit++;
	}

	return ( nNumErrors == 0 );
}

//
//
//
BOOL CKongToWorldInitFile::StoreNonMeshShapes( CApeToKongFormat *pKong, u32 nNumApeShapes, CFWorldShapeInit *pShapeInit )
{
	ApeShape_t *pApeShape;
	BOOL bStringIsEmpty;
	ApeSplinePt_t *pApeSplinePt;
	char *pUserProps;
	u32 i, j, nPropsBytes, nNumErrors = 0;
	CString sError;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	/////////////////////////////
	// run through the ape shapes
	////////////////////////////////////////////////////
	// THIS MUST HAPPEN AFTER PROCESSING THE MESH ARRAY
	// OR ELSE THE PARENT INDICES WILL BE OFF
	////////////////////////////////////////
	for( i = 0; i < nNumApeShapes; i++ ) 
	{
		// grab the ape shape
		pApeShape = pKong->GetShapeInfo( i );

		m_pHeader->nNumInitStructs++;

		// align up the working mem
		m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );

		switch( pApeShape->nType ) 
		{
			case APE_SHAPE_TYPE_SPEAKER:		
			case APE_SHAPE_TYPE_PARTICLE_SPHERE:
			case APE_SHAPE_TYPE_SPHERE:
				// fill in the common fields
				pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_SPHERE;
				pShapeInit->m_Mtx43 = pApeShape->Orientation;
				pShapeInit->m_nParentShapeIndex = pApeShape->nParentIndex - 1;
				
				// grab a CFWorldShapeSphere
				pShapeInit->m_pSphere = (CFWorldShapeSphere *)m_pWorkingMem;
				m_pWorkingMem += sizeof( CFWorldShapeSphere );

				// fill in the CFWorldShapeSphere
				pShapeInit->m_pSphere->m_fRadius = pApeShape->TypeData.Sphere.fRadius;

				pShapeInit->m_pSphere->m_fRadius *= pShapeInit->m_Mtx43.m_vRight.Mag();
				
				pShapeInit->m_Mtx43.m33.UnitizeWithUniformScale();
				
				// convert the user properties
				pShapeInit->m_pGameData = ConvertMaxUserPropsToGameData( (char *)&pApeShape[1], pApeShape->nBytesOfUserData, bStringIsEmpty );

				// check for error conditions
				if( !bStringIsEmpty && !pShapeInit->m_pGameData && pApeShape->nBytesOfUserData ) 
				{
					// the user supplied user data, but it didn't get converted for some reason
					if( nNumErrors == 0 ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
					}
					nNumErrors++;
					sError.Format( "Error in user data for max shape located at: (%.3f, %.3f, %.3f in max space).", 
																								pApeShape->Orientation.m_vPos.x,
																								pApeShape->Orientation.m_vPos.z,
																								pApeShape->Orientation.m_vPos.y );
					rErrorLog.WriteErrorLine( sError );
				}
				break;

			case APE_SHAPE_TYPE_PARTICLE_CYLINDER:			
			case APE_SHAPE_TYPE_CYLINDER:
				// fill in the common fields
				pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_CYLINDER;
				pShapeInit->m_Mtx43 = pApeShape->Orientation;
				pShapeInit->m_nParentShapeIndex = pApeShape->nParentIndex - 1;

				// grab a CFWorldShapeCylinder
				pShapeInit->m_pCylinder = (CFWorldShapeCylinder *)m_pWorkingMem;
				m_pWorkingMem += sizeof( CFWorldShapeCylinder );

				// fill in the CFWorldShapeCylinder
				pShapeInit->m_pCylinder->m_fDimY = pApeShape->TypeData.Cylinder.fHeight;
				pShapeInit->m_pCylinder->m_fRadius = pApeShape->TypeData.Cylinder.fRadius;
				
				pShapeInit->m_pCylinder->m_fDimY *= pShapeInit->m_Mtx43.m_vUp.Mag();
				pShapeInit->m_pCylinder->m_fRadius *= pShapeInit->m_Mtx43.m_vRight.Mag();
				
				pShapeInit->m_Mtx43.m33.UnitizeWithUniformScale();

				// convert the user properties
				pShapeInit->m_pGameData = ConvertMaxUserPropsToGameData( (char *)&pApeShape[1], pApeShape->nBytesOfUserData, bStringIsEmpty );

				// check for error conditions
				if( !bStringIsEmpty && !pShapeInit->m_pGameData && pApeShape->nBytesOfUserData ) 
				{
					// the user supplied user data, but it didn't get converted for some reason
					if( nNumErrors == 0 ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
					}
					nNumErrors++;
					sError.Format( "Error in user data for max shape located at: (%.3f, %.3f, %.3f in max space).", 
																								pApeShape->Orientation.m_vPos.x,
																								pApeShape->Orientation.m_vPos.z,
																								pApeShape->Orientation.m_vPos.y );
					rErrorLog.WriteErrorLine( sError );
				}
				break;

			case APE_SHAPE_TYPE_ROOM:
			case APE_SHAPE_TYPE_ARENA:
			case APE_SHAPE_TYPE_PARTICLE_BOX:
				// fill in the common fields
				pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_BOX;
				pShapeInit->m_Mtx43 = pApeShape->Orientation;
				pShapeInit->m_nParentShapeIndex = pApeShape->nParentIndex - 1;

				// grab a CFWorldShapeBox
				pShapeInit->m_pBox = (CFWorldShapeBox *)m_pWorkingMem;
				m_pWorkingMem += sizeof( CFWorldShapeBox );

				// fill in the CFWorldShapeBox
				pShapeInit->m_pBox->m_fDimX = pApeShape->TypeData.Room.fWidth;
				pShapeInit->m_pBox->m_fDimY = pApeShape->TypeData.Room.fHeight;
				pShapeInit->m_pBox->m_fDimZ = pApeShape->TypeData.Room.fLength;

				pShapeInit->m_pBox->m_fDimX *= pShapeInit->m_Mtx43.m_vRight.Mag();
				pShapeInit->m_pBox->m_fDimY *= pShapeInit->m_Mtx43.m_vUp.Mag();
				pShapeInit->m_pBox->m_fDimZ *= pShapeInit->m_Mtx43.m_vFront.Mag();
				
				pShapeInit->m_Mtx43.m33.UnitizeWithUniformScale();

				// convert the user properties
				pShapeInit->m_pGameData = ConvertMaxUserPropsToGameData( (char *)&pApeShape[1], pApeShape->nBytesOfUserData, bStringIsEmpty );

				// check for error conditions
				if( !bStringIsEmpty && !pShapeInit->m_pGameData && pApeShape->nBytesOfUserData ) 
				{
					// the user supplied user data, but it didn't get converted for some reason
					if( nNumErrors == 0 ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
					}
					nNumErrors++;
					sError.Format( "Error in user data for max shape located at: (%.3f, %.3f, %.3f in max space).", 
																								pApeShape->Orientation.m_vPos.x,
																								pApeShape->Orientation.m_vPos.z,
																								pApeShape->Orientation.m_vPos.y );
					rErrorLog.WriteErrorLine( sError );
				}
				break;

			case APE_SHAPE_TYPE_BOX:
				// fill in the common fields
				pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_BOX;
				pShapeInit->m_Mtx43 = pApeShape->Orientation;
				pShapeInit->m_nParentShapeIndex = pApeShape->nParentIndex - 1;

				// grab a CFWorldShapeBox
				pShapeInit->m_pBox = (CFWorldShapeBox *)m_pWorkingMem;
				m_pWorkingMem += sizeof( CFWorldShapeBox );

				// fill in the CFWorldShapeBox
				pShapeInit->m_pBox->m_fDimX = pApeShape->TypeData.Box.fWidth;
				pShapeInit->m_pBox->m_fDimY = pApeShape->TypeData.Box.fHeight;
				pShapeInit->m_pBox->m_fDimZ = pApeShape->TypeData.Box.fLength;

				pShapeInit->m_pBox->m_fDimX *= pShapeInit->m_Mtx43.m_vRight.Mag();
				pShapeInit->m_pBox->m_fDimY *= pShapeInit->m_Mtx43.m_vUp.Mag();
				pShapeInit->m_pBox->m_fDimZ *= pShapeInit->m_Mtx43.m_vFront.Mag();
				
				pShapeInit->m_Mtx43.m33.UnitizeWithUniformScale();

				// convert the user properties
				pShapeInit->m_pGameData = ConvertMaxUserPropsToGameData( (char *)&pApeShape[1], pApeShape->nBytesOfUserData, bStringIsEmpty );

				// check for error conditions
				if( !bStringIsEmpty && !pShapeInit->m_pGameData && pApeShape->nBytesOfUserData ) 
				{
					// the user supplied user data, but it didn't get converted for some reason
					if( nNumErrors == 0 ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
					}
					nNumErrors++;
					sError.Format( "Error in user data for max shape located at: (%.3f, %.3f, %.3f in max space).", 
																								pApeShape->Orientation.m_vPos.x,
																								pApeShape->Orientation.m_vPos.z,
																								pApeShape->Orientation.m_vPos.y );
					rErrorLog.WriteErrorLine( sError );
				}
				break;

			default:
			case APE_SHAPE_TYPE_CAMERA:					
			case APE_SHAPE_TYPE_SPAWN_POINT:
			case APE_SHAPE_TYPE_START_POINT:
				// fill in the common fields
				pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_POINT;
				pShapeInit->m_Mtx43 = pApeShape->Orientation;
				pShapeInit->m_nParentShapeIndex = pApeShape->nParentIndex - 1;

				pShapeInit->m_Mtx43.m33.UnitizeWithUniformScale();

				// there is no point struct
				
				// convert the user properties
				pShapeInit->m_pGameData = ConvertMaxUserPropsToGameData( (char *)&pApeShape[1], pApeShape->nBytesOfUserData, bStringIsEmpty );

				// check for error conditions
				if( !bStringIsEmpty && !pShapeInit->m_pGameData && pApeShape->nBytesOfUserData ) 
				{
					// the user supplied user data, but it didn't get converted for some reason
					if( nNumErrors == 0 ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
					}
					nNumErrors++;
					sError.Format( "Error in user data for max shape located at: (%.3f, %.3f, %.3f in max space).", 
																								pApeShape->Orientation.m_vPos.x,
																								pApeShape->Orientation.m_vPos.z,
																								pApeShape->Orientation.m_vPos.y );
					rErrorLog.WriteErrorLine( sError );
				}
				break;
				
			case APE_SHAPE_TYPE_SPLINE:
				pApeSplinePt = (ApeSplinePt_t *)&pApeShape[1];
	#if 0
				// see if we should just create a line
				if( pApeShape->TypeData.Spline.nNumPts == 2 && !pApeShape->TypeData.Spline.bClosed ) {
					// create a line shape
					pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_LINE;
					pShapeInit->m_Mtx43.Identity();
					pShapeInit->m_nParentShapeIndex = pApeShape->nParentIndex - 1;

					// grab a CFWorldShapeLine
					pShapeInit->m_pLine = (CFWorldShapeLine *)m_pWorkingMem;
					m_pWorkingMem += sizeof( CFWorldShapeLine );

					pShapeInit->m_Mtx43.m_vPos = pApeSplinePt[0].Pos;
					Dir = pApeSplinePt[1].Pos - pApeSplinePt[0].Pos;

					// fill in the CFWorldShapeLine
					pShapeInit->m_pLine->m_fLength = Dir.Mag();
					pShapeInit->m_Mtx43.m_vFront = Dir.Unitize();
					Up.Set( 0.0f, 1.0f, 0.0f );
					pShapeInit->m_Mtx43.m_vRight = Up.Cross( Dir );
					pShapeInit->m_Mtx43.m_vUp = Dir.Cross( pShapeInit->m_Mtx43.m_vRight );				
				} 
				else 
	#endif
				{
					// create a spline shape
					pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_SPLINE;
					pShapeInit->m_Mtx43.Identity();
					pShapeInit->m_Mtx43.m_vPos = pApeSplinePt[0].Pos;
					pShapeInit->m_nParentShapeIndex = pApeShape->nParentIndex - 1;

					// grab a CFWorldShapeSpline
					pShapeInit->m_pSpline = (CFWorldShapeSpline *)m_pWorkingMem;
					m_pWorkingMem += sizeof( CFWorldShapeSpline );
					
					pShapeInit->m_pSpline->m_nPointCount = pApeShape->TypeData.Spline.nNumPts;
					pShapeInit->m_pSpline->m_bClosedSpline = pApeShape->TypeData.Spline.bClosed;
					pShapeInit->m_pSpline->m_pPtArray = (CFVec3 *)m_pWorkingMem;
					m_pWorkingMem += ( sizeof( CFVec3 ) * pApeShape->TypeData.Spline.nNumPts );

					for( j=0; j < pShapeInit->m_pSpline->m_nPointCount; j++ ) 
					{
						pShapeInit->m_pSpline->m_pPtArray[j] = pApeSplinePt[j].Pos;
					}
				}

				// convert the user properties
				pUserProps = (char *)&pApeShape[1];
				pUserProps += (pApeShape->TypeData.Spline.nNumPts * sizeof( ApeSplinePt_t ));
				nPropsBytes = pApeShape->nBytesOfUserData - (pApeShape->TypeData.Spline.nNumPts * sizeof( ApeSplinePt_t ));
				pShapeInit->m_pGameData = ConvertMaxUserPropsToGameData( pUserProps, nPropsBytes, bStringIsEmpty );

				// check for error conditions
				if( !bStringIsEmpty && !pShapeInit->m_pGameData && nPropsBytes ) 
				{
					// the user supplied user data, but it didn't get converted for some reason
					if( nNumErrors == 0 ) 
					{
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + pKong->m_sLastFileLoaded );
					}
					nNumErrors++;
					sError.Format( "Error in user data for max shape located at: (%.3f, %.3f, %.3f in max space).", 
																								pApeShape->Orientation.m_vPos.x,
																								pApeShape->Orientation.m_vPos.z,
																								pApeShape->Orientation.m_vPos.y );
					rErrorLog.WriteErrorLine( sError );
				}
				break;
		}
		
		pShapeInit++;
	}

	return (nNumErrors == 0);
}


//
//
//
u32 CKongToWorldInitFile::GetCRC( u32 nCurrentCRC ) 
{
	if( !m_pMemAllocated ) 
	{
		return nCurrentCRC;
	}

	// convert our ptrs to offsets
	ConvertPointersToOffsets();

	u32 nReturnCRC = fmath_Crc32( nCurrentCRC, (u8 *)m_pHeader, m_nStatsShapeBytes );
	
	// convert offsets back to pointers
	ConvertOffsetsToPointers();
	
	return nReturnCRC;
}


//
//
//
u32 CKongToWorldInitFile::SaveInitDataToVIS( HANDLE hFile ) 
{
	if( !m_pMemAllocated ) 
	{
		return 0;
	}

	if ( hFile == INVALID_HANDLE_VALUE ) 
	{
		return 0;
	}

	// convert our ptrs to offsets
	ConvertPointersToOffsets();
	
	u32 nBytesWritten = 0;
	WriteFile( hFile, m_pHeader, m_nStatsShapeBytes, (LPDWORD)&nBytesWritten, NULL );

	// convert offsets back to pointers
	ConvertOffsetsToPointers();
	
	return nBytesWritten;
}


//
//
//
u32 CKongToWorldInitFile::LoadInitDataFromVIS( HANDLE hFile, u32 nBytes ) 
{
	if ( hFile == INVALID_HANDLE_VALUE || nBytes == 0 ) 
	{
		return 0;
	}

	m_pMemAllocated = (u8 *)fang_Malloc( nBytes, 4 );
	if ( !m_pMemAllocated )
	{
		return 0;
	}

	ReadFile( hFile, m_pMemAllocated, nBytes, (LPDWORD)&m_nStatsShapeBytes, NULL );
	if ( nBytes != m_nStatsShapeBytes )
	{
		fang_Free( m_pMemAllocated );
		m_pMemAllocated = NULL;
		return 0;
	}

	m_pHeader = (FData_WorldInitHeader_t *)m_pMemAllocated;
	m_pShapeArray = (CFWorldShapeInit *)&m_pHeader[1];
	m_pWorkingMem = NULL;
	m_bPointersAreOffsets = TRUE;

	// convert offsets back to pointers
	ConvertOffsetsToPointers();
	
	return m_nStatsShapeBytes;
}


//
//
//
BOOL CKongToWorldInitFile::WriteToFile( CCompileDlg *pDlg, cchar *pszFilename, TargetPlatform_e nPlatform, FILE *pFileStream/*=NULL*/ ) 
{
	if( !m_pMemAllocated ) 
	{
		return FALSE;
	}
	// open our file, if pFileStream == NULL
	BOOL bCloseFile = FALSE;
	if( !pFileStream ) 
	{
		if( !pszFilename ) 
		{
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if( !pFileStream ) 
		{
			return FALSE;
		}
		bCloseFile = TRUE;	
	}

	pDlg->WriteToLog( "Converting World Init Data.\n" );

	// If we're not using old init data, we need to convert to fang format
	MatchVertexColorsToMesh( pDlg, nPlatform );

	// convert our ptrs to offsets
	ConvertPointersToOffsets();
	
	if ( nPlatform == PASM_TARGET_GC )
	{
		ChangeEndian();
	}
	
	//////////////////////
	// write out the file
	fwrite( m_pHeader, m_nStatsShapeBytes, 1, pFileStream );
	
	// close file, if need be
	if( bCloseFile ) 
	{
		fclose( pFileStream );
	}
	return TRUE;
}


//
//
//
void CKongToWorldInitFile::FreeData() 
{
	m_nNumBytesAllocated = 0;
	if( m_pMemAllocated ) 
	{
		fang_Free( m_pMemAllocated );
		m_pMemAllocated = NULL;
	}
	m_nStatsShapeBytes = 0;
	m_pHeader = NULL;
	m_pShapeArray = NULL;
	m_pWorkingMem = NULL;
	m_nStatsNumShapes = 0;
	m_aStrings.RemoveAll();
}


//
//
//
cchar *CKongToWorldInitFile::IsStringAlreadyInTable( cchar *pszString ) 
{
	int i, nStringCount;
	cchar *pszTableString;
	
	nStringCount = m_aStrings.GetSize();
	for( i=0; i < nStringCount; i++ ) 
	{
		pszTableString = (cchar *)m_aStrings[i];
		if( fclib_stricmp( pszTableString, pszString ) == 0 ) 
		{
			return pszTableString;
		}
	}
	return NULL;
}


//
//
//
BOOL CKongToWorldInitFile::MatchVertexColorsToMesh( CCompileDlg *pDlg, TargetPlatform_e nPlatform ) 
{
	u32 nSupportedOldColors = 0;
	u32 *paOldColors = NULL;
	u32 nSupportedVertRemaps = 0;
	KongVertRemap_t *paVertRemaps = NULL;
	u8 *pCurrentDataPointer = NULL;
	u32 i, j;

	for( i = 0; i < m_pHeader->nNumInitStructs; i++ ) 
	{
		CString csName;
		u32 anVBSize[32], nVBCount = 0;

		if ( m_pShapeArray[i].m_nShapeType != FWORLD_SHAPETYPE_MESH )
		{
			continue;
		}

		if ( !m_pShapeArray[i].m_pMesh->m_nColorStreamCount )
		{
			continue;
		}

		// Load the vertex remap table
		if ( nPlatform == PASM_TARGET_PC || nPlatform == PASM_TARGET_XB )
		{
			csName = CPasmDlg::m_csPASMTempDirectory + m_pShapeArray[i].m_pMesh->m_pszMeshName + "_PC.rmp";
		}
		else if ( nPlatform == PASM_TARGET_GC )
		{
			csName = CPasmDlg::m_csPASMTempDirectory + m_pShapeArray[i].m_pMesh->m_pszMeshName + "_GC.rmp";
		}
//ARG - >>>>>
#ifdef _MMI_TARGET_PS2
		else if ( nPlatform == PASM_TARGET_PS2 )
		{
			csName = CPasmDlg::m_csPASMTempDirectory + m_pShapeArray[i].m_pMesh->m_pszMeshName + "_PS2.rmp";
		}
#endif
//ARG - <<<<<
		else
		{
			FASSERT_NOW;
			return FALSE;
		}
		csName.MakeLower();
		HANDLE hFile = CreateFile( csName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			return FALSE;
		}

		SYSTEMTIME SourceFileTime;
		u32 nVertCount, nBytesRead;
		ReadFile( hFile, &SourceFileTime, sizeof( SYSTEMTIME ), (LPDWORD)&nBytesRead, NULL );
		if ( nBytesRead != sizeof( SYSTEMTIME ) )
		{
			CloseHandle( hFile );
			return FALSE;
		}

		// Read in the VB count
		ReadFile( hFile, &nVBCount, sizeof( u32 ), (LPDWORD)&nBytesRead, NULL );
		if ( nVBCount == 0 || nVBCount > 32 )
		{
			m_pShapeArray[i].m_pMesh->m_nColorStreamCount = 0;
			m_pShapeArray[i].m_pMesh->m_paColorStreams = NULL;
			CloseHandle( hFile );
			continue;
		}


		// Read in the vertex count
		ReadFile( hFile, &nVertCount, sizeof( u32 ), (LPDWORD)&nBytesRead, NULL );
		if ( nVertCount == 0 )
		{
			m_pShapeArray[i].m_pMesh->m_nColorStreamCount = 0;
			m_pShapeArray[i].m_pMesh->m_paColorStreams = NULL;
			CloseHandle( hFile );
			continue;
		}

		// Make sure we have enough memory to load the remap
		if ( nSupportedVertRemaps < nVertCount )
		{
			if ( paVertRemaps )
			{
				free( paVertRemaps );
				paVertRemaps = NULL;
			}
			nSupportedVertRemaps = nVertCount + 64;
			paVertRemaps = (KongVertRemap_t *)malloc( sizeof(KongVertRemap_t) * nSupportedVertRemaps );
		}
		if ( !paVertRemaps )
		{
			CloseHandle( hFile );
			return FALSE;
		}

		// Read in the remap table for all verts
		ReadFile( hFile, paVertRemaps, sizeof( KongVertRemap_t ) * nVertCount, (LPDWORD)&nBytesRead, NULL );
		if ( nBytesRead != sizeof( KongVertRemap_t ) * nVertCount )
		{
			CloseHandle( hFile );
			free( paVertRemaps );
			return FALSE;
		}

		// Verify that the key is the last thing in the file
		u16 nKey;
		ReadFile( hFile, &nKey, sizeof( u16 ), (LPDWORD)&nBytesRead, NULL );
		if ( nBytesRead != sizeof( u16 ) || nKey != 0xBEEF )
		{
			CloseHandle( hFile );
			free( paVertRemaps );
			return FALSE;
		}

		// We're done reading
		CloseHandle( hFile );

		u32 nBadVerts = 0;
		memset( anVBSize, 0, sizeof(u32) * nVBCount );
		for ( j = 0; j < nVertCount; j++ )
		{
			if ( paVertRemaps[j].nVBIndex >= nVBCount )
			{
				nBadVerts++;
				continue;
			}
			if ( paVertRemaps[j].nVertColorIndex >= nVertCount )
			{
				DEVPRINTF( "CKongToWorldInitFile::ConvertToFangFormat() - Vert color index of %d exceeds vert count of %d for %s.\n", paVertRemaps[j].nVertColorIndex, nVertCount, m_pShapeArray[i].m_pMesh->m_pszMeshName );
				FASSERT_NOW;
			}
			if ( (u32)(paVertRemaps[j].nVertColorIndex + 1) > anVBSize[ paVertRemaps[j].nVBIndex ] )
			{
				anVBSize[ paVertRemaps[j].nVBIndex ] = paVertRemaps[j].nVertColorIndex + 1;
			}
		}

		if ( nBadVerts )
		{
			DEVPRINTF( "CKongToWorldInitFile::ConvertToFangFormat() - %d of %d verts uninitialized in remap file for %s.\n", nBadVerts, nVertCount, m_pShapeArray[i].m_pMesh->m_pszMeshName );
		}

		u32 nOldColorCount = m_pShapeArray[i].m_pMesh->m_paColorStreams[0].nVBIndex;
		if ( nOldColorCount != nVertCount )
		{
			FASSERT_NOW;
			DEVPRINTF( "CKongToWorldInitFile::ConvertToFangFormat() - Vert count of %d in remap file does not match original of %d for mesh %s.\n", nVertCount, nOldColorCount, m_pShapeArray[i].m_pMesh->m_pszMeshName );
		}

		if ( !pCurrentDataPointer )
		{
			pCurrentDataPointer = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pShapeArray[i].m_pMesh->m_paColorStreams, 4 );
		}

		// Calculate how many verts we have in the compressed VB's
		u32 nCompressedVertCount = 0;
		for ( j = 0; j < nVBCount; j++ )
		{
			nCompressedVertCount += anVBSize[j];
		}

		// Calculate the size of the converted data to make sure it will fit in the space of
		// the old data without overwriting subsequent instanced mesh data
		u32 nNewSize = (sizeof( ColorStream_t ) * nVBCount) + (nCompressedVertCount * sizeof(u32));
		u32 nOldSize = (nOldColorCount * sizeof(u32)) + sizeof( ColorStream_t ) + _COLOR_STREAM_BUFFER;

		if ( pCurrentDataPointer + nNewSize > (u8 *)m_pShapeArray[i].m_pMesh->m_paColorStreams + nOldSize )
		{
			DEVPRINTF( "CKongToWorldInitFile::ConvertToFangFormat() - Data would be overwritten if vert rad colors were put in init for  %s.  Aborting init generation.\n", m_pShapeArray[i].m_pMesh->m_pszMeshName );
			FASSERT_NOW;
			return FALSE;
		}

		// Allocate some space to make a copy of the uncompressed vertex colors
		if ( nSupportedOldColors < nOldColorCount )
		{
			if ( paOldColors )
			{
				free( paOldColors );
				paOldColors = NULL;
			}
			nSupportedOldColors = (nOldColorCount + 64);
			paOldColors = (u32 *)malloc( sizeof(u32) * nSupportedOldColors );
		}
		if ( !paOldColors )
		{
			free( paVertRemaps );
			return FALSE;
		}

		// Copy over the old colors so that we can overwrite that memory with the compressed colors
		memcpy( paOldColors, m_pShapeArray[i].m_pMesh->m_paColorStreams[0].paVertexColors, sizeof(u32) * nOldColorCount );

		// Start filling out the information for the mesh color streams
		m_pShapeArray[i].m_pMesh->m_nColorStreamCount = nVBCount;
		m_pShapeArray[i].m_pMesh->m_paColorStreams = (ColorStream_t *)pCurrentDataPointer;
		u8 *pColorStart = (u8 *)(m_pShapeArray[i].m_pMesh->m_paColorStreams + nVBCount);

		ColorStream_t *pColorStreams = m_pShapeArray[i].m_pMesh->m_paColorStreams;
		for ( j = 0; j < nVBCount; j++ )
		{
			pColorStreams[j].nVBIndex = j;
			pColorStreams[j].nColorCount = anVBSize[j];
			if ( anVBSize[j] )
			{
				pColorStreams[j].paVertexColors = pColorStart;
				memset( pColorStreams[j].paVertexColors, 0, sizeof( u32 ) * anVBSize[j] );
			}
			else
			{
				pColorStreams[j].paVertexColors = NULL;
			}
			pColorStart += sizeof( u32 ) * anVBSize[j];
		}
		pCurrentDataPointer = pColorStart;

		// Remap the colors
		u32 nUnassignedVerts = 0;
		u16 anCurrentIndex[32];
		memset( anCurrentIndex, 0, sizeof(u16) * 32 );
		for ( j = 0; j < nVertCount; j++ )
		{
			u16 nVBIdx = paVertRemaps[j].nVBIndex;
			u16 nVertIdx = paVertRemaps[j].nVertColorIndex;
//			FASSERT( nVertIdx < nOldColorCount && nVBIdx < 32 );
			if ( nVBIdx >= nVBCount || nVertIdx >= anVBSize[nVBIdx] )
			{
				nUnassignedVerts++;
				continue;
			}
			else
			{
				FASSERT( pColorStreams[nVBIdx].paVertexColors );
				u32 nPlatformColor = paOldColors[j];
				if ( nPlatform == PASM_TARGET_PC || nPlatform == PASM_TARGET_XB )
				{
					// DX is ARGB
					nPlatformColor = ((nPlatformColor & 0xffffff00) >> 8) | ((nPlatformColor & 0x000000ff) << 24);
				}
				((u32 *)pColorStreams[nVBIdx].paVertexColors)[nVertIdx] = nPlatformColor;
//				DEVPRINTF( "Color Vert %d = %X\n", nVertIdx, nPlatformColor );
			}
		}
		if ( nUnassignedVerts )
		{
			DEVPRINTF( "CKongToWorldInitFile::ConvertToFangFormat() - %d unassigned vertices detected in mesh %s.\n", nUnassignedVerts, m_pShapeArray[i].m_pMesh->m_pszMeshName );
		}
	}

	if ( paVertRemaps )
	{
		free( paVertRemaps );
		paVertRemaps = NULL;
	}

	if ( paOldColors )
	{
		free( paOldColors );
	}

	// re-calculate our compiled size
	if ( pCurrentDataPointer )
	{
		m_nStatsShapeBytes = (u32)pCurrentDataPointer - (u32)m_pHeader;
	}

	return TRUE;
}


//
//
//
void CKongToWorldInitFile::ConvertPointersToOffsets() 
{
	u32 nStartOffset = (u32)m_pShapeArray;
	u32 i, j;

	if ( m_bPointersAreOffsets )
	{
		return;
	}

	for( i=0; i < m_pHeader->nNumInitStructs; i++ ) 
	{
		m_pShapeArray[i].m_pGameData = (void *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pGameData, nStartOffset );

		switch( m_pShapeArray[i].m_nShapeType ) 
		{
			case FWORLD_SHAPETYPE_SPLINE:
				m_pShapeArray[i].m_pSpline->m_pPtArray = (CFVec3 *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pSpline->m_pPtArray, nStartOffset );
				break;

			case FWORLD_SHAPETYPE_MESH:
				m_pShapeArray[i].m_pMesh->m_pszMeshName = (cchar *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pMesh->m_pszMeshName, nStartOffset );
				for ( j = 0; j < FWORLD_MAX_MESH_SHAPE_LIGHTMAPS; j++ )
				{
					if ( m_pShapeArray[i].m_pMesh->m_papszLightMapName[j] )
					{
						m_pShapeArray[i].m_pMesh->m_papszLightMapName[j] = (cchar *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pMesh->m_papszLightMapName[j], nStartOffset );
					}
				}

				// Convert the color streams
				for ( j = 0; j < m_pShapeArray[i].m_pMesh->m_nColorStreamCount; j++ )
				{
					m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors = (void *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors, nStartOffset );
				}
				m_pShapeArray[i].m_pMesh->m_paColorStreams = (ColorStream_t *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pMesh->m_paColorStreams, nStartOffset );

				break;
		}

		m_pShapeArray[i].m_pShape = (void *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pShape, nStartOffset );		
	}	

	m_bPointersAreOffsets = TRUE;
}


//
//
//
void CKongToWorldInitFile::ConvertOffsetsToPointers() 
{
	u32 nStartOffset = (u32)m_pShapeArray;
	u32 i, j;

	if ( !m_bPointersAreOffsets )
	{
		return;
	}

	for( i=0; i < m_pHeader->nNumInitStructs; i++ ) 
	{
		m_pShapeArray[i].m_pShape = (void *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pShape, nStartOffset );

		m_pShapeArray[i].m_pGameData = (void *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pGameData, nStartOffset );

		switch( m_pShapeArray[i].m_nShapeType ) 
		{
			case FWORLD_SHAPETYPE_SPLINE:
				m_pShapeArray[i].m_pSpline->m_pPtArray = (CFVec3 *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pSpline->m_pPtArray, nStartOffset );
				break;

			case FWORLD_SHAPETYPE_MESH:
				m_pShapeArray[i].m_pMesh->m_pszMeshName = (cchar *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pMesh->m_pszMeshName, nStartOffset );

				// Convert the lightmap names
				for ( j = 0; j < FWORLD_MAX_MESH_SHAPE_LIGHTMAPS; j++ )
				{
					if ( m_pShapeArray[i].m_pMesh->m_papszLightMapName[j] )
					{
						m_pShapeArray[i].m_pMesh->m_papszLightMapName[j] = (cchar *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pMesh->m_papszLightMapName[j], nStartOffset );
					}
				}

				// Convert the color streams
				m_pShapeArray[i].m_pMesh->m_paColorStreams = (ColorStream_t *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pMesh->m_paColorStreams, nStartOffset );
				for ( j = 0; j < m_pShapeArray[i].m_pMesh->m_nColorStreamCount; j++ )
				{
					m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors = (void *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors, nStartOffset );
				}

				break;
		}
	}	

	m_bPointersAreOffsets = FALSE;
}


//
//
//
void CKongToWorldInitFile::ChangeEndian( void ) 
{
	u32 i, j, k;

	CFWorldShapeInit *m_pShapeArray = (CFWorldShapeInit *)&m_pHeader[1];
	u32 nStartOffset = (u32)m_pShapeArray;

	for( i = 0; i < m_pHeader->nNumInitStructs; i++ ) 
	{
		
		m_pShapeArray[i].m_pShape = (void *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pShape, nStartOffset );

		// convert shape specific data structures
		switch( m_pShapeArray[i].m_nShapeType ) 
		{
			case FWORLD_SHAPETYPE_POINT:
				// no data for a point
				break;

			case FWORLD_SHAPETYPE_LINE:
				m_pShapeArray[i].m_pLine->ChangeEndian();
				break;

			case FWORLD_SHAPETYPE_SPLINE:
				m_pShapeArray[i].m_pSpline->m_pPtArray = (CFVec3 *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pSpline->m_pPtArray, nStartOffset );
				for( j=0; j < m_pShapeArray[i].m_pSpline->m_nPointCount; j++ ) 
				{
					m_pShapeArray[i].m_pSpline->m_pPtArray[j].ChangeEndian();
				}
				m_pShapeArray[i].m_pSpline->m_pPtArray = (CFVec3 *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pSpline->m_pPtArray, nStartOffset );

				m_pShapeArray[i].m_pSpline->ChangeEndian();
				break;

			case FWORLD_SHAPETYPE_BOX:
				m_pShapeArray[i].m_pBox->ChangeEndian();
				break;

			case FWORLD_SHAPETYPE_SPHERE:
				m_pShapeArray[i].m_pSphere->ChangeEndian();
				break;

			case FWORLD_SHAPETYPE_CYLINDER:
				m_pShapeArray[i].m_pCylinder->ChangeEndian();
				break;

			case FWORLD_SHAPETYPE_MESH:

				m_pShapeArray[i].m_pMesh->m_paColorStreams = (ColorStream_t *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pMesh->m_paColorStreams, nStartOffset );
				for ( j = 0; j < m_pShapeArray[i].m_pMesh->m_nColorStreamCount; j++ )
				{
					m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors = (ColorStream_t *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors, nStartOffset );
					for ( k = 0; k < m_pShapeArray[i].m_pMesh->m_paColorStreams[j].nColorCount; k++ )
					{
						((u32 *)m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors)[k] = fang_ConvertEndian( ((u32 *)m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors)[k] );
					}
					m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors = (ColorStream_t *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors, nStartOffset );
					m_pShapeArray[i].m_pMesh->m_paColorStreams[j].nVBIndex = fang_ConvertEndian( m_pShapeArray[i].m_pMesh->m_paColorStreams[j].nVBIndex );
					m_pShapeArray[i].m_pMesh->m_paColorStreams[j].nColorCount = fang_ConvertEndian( m_pShapeArray[i].m_pMesh->m_paColorStreams[j].nColorCount );
					m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors = fang_ConvertEndian( m_pShapeArray[i].m_pMesh->m_paColorStreams[j].paVertexColors );
				}
				m_pShapeArray[i].m_pMesh->m_paColorStreams = (ColorStream_t *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pMesh->m_paColorStreams, nStartOffset );

				m_pShapeArray[i].m_pMesh->ChangeEndian();

				break;

			default:
				FASSERT_NOW;
		}

		m_pShapeArray[i].m_pShape = (void *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pShape, nStartOffset );		

		// endianize any gamedata that we may have (the data has already been converted to offsets)
		m_pShapeArray[i].m_pGameData = (void *)UTILS_CONVERT_OFFSET_TO_PTR( m_pShapeArray[i].m_pGameData, nStartOffset );
		CConvertCsvFile::ConvertEndian( (FDataGamFile_Header_t *)m_pShapeArray[i].m_pGameData );			
		m_pShapeArray[i].m_pGameData = (void *)UTILS_CONVERT_PTR_TO_OFFSET( m_pShapeArray[i].m_pGameData, nStartOffset );

		// endianize the shape itself
		m_pShapeArray[i].ChangeEndian();
	}
	
	// endianize the header struct
	m_pHeader->ChangeEndian();
}


//
//
//
FDataGamFile_Header_t *CKongToWorldInitFile::ConvertMaxUserPropsToGameData( void *pUserProps, u32 nNumBytes, BOOL &rPropStringWasEmpty ) 
{
	u8 *pCopyUserData, c;
	u32 i;
	FDataGamFile_Header_t *pHeader;
	FDataGamFile_Table_t *pTable;
	FDataGamFile_Field_t *pField;
	CUIntArray m_anFieldsPerTable;

	rPropStringWasEmpty = FALSE;

	if( !pUserProps || !nNumBytes ) 
	{
		return NULL;
	}

	// copy the user data into a local buffer so that we can alter it
	pCopyUserData = (u8 *)fang_Malloc( nNumBytes+1, 4 );
	if( !pCopyUserData ) 
	{
		return NULL;
	}
	fang_MemCopy( pCopyUserData, pUserProps, nNumBytes );
	pCopyUserData[nNumBytes] = 0;
	
	// replace CR, LF, and ^Z with NULL
	for( i=0; i < nNumBytes; i++ ) 
	{
		c = pCopyUserData[i];
		
		if( c == 0x1A || c == 0xA || c == 0xD ) 
		{
			pCopyUserData[i] = 0;			
		}
	}

	if( pCopyUserData[0] == 0 ) 
	{
		rPropStringWasEmpty = TRUE;
		fang_Free( pCopyUserData );
		return NULL;
	}

	// gather some info about the data
	u32 nNumValidLines = 0, nNumFields = 0, nNumFloats = 0, nFieldsInTable, nMaxFieldsPerTable = 0;
	char *pszLine = (char *)pCopyUserData;
	char *pEOF = (char *)&pCopyUserData[nNumBytes];
	CString sLine, sLeft, sRight, sValue;
	int nIndex;
	BOOL bFirstElement;
	m_anFieldsPerTable.RemoveAll();

	while( pszLine ) 
	{
		sLine = pszLine;
		
		if( !sLine.IsEmpty() ) 
		{
			// find the index
			nIndex = sLine.Find( '=', 0 );
			if( nIndex > 0 ) 
			{
				// extract the left and right parts of the string
				sLeft = sLine.Mid( 0, nIndex );
				sRight = sLine.Mid( nIndex+1 );
				sLeft.TrimLeft();
				sLeft.TrimRight();
				sRight.TrimLeft();
				sRight.TrimRight();

				if( !sLeft.IsEmpty() && 
					!sRight.IsEmpty() &&
					!CConvertCsvFile::DoesKeyStringContainInvalidChars( sLeft, TRUE ) ) 
				{
					
					bFirstElement = TRUE;
					while( !sRight.IsEmpty() ) 
					{
						nIndex = sRight.Find( ',', 0 );
						if( nIndex > 0 ) 
						{
							sValue = sRight.Mid( 0, nIndex );
							sRight.Delete( 0, nIndex+1 );
						} 
						else if( nIndex == 0 ) 
						{
							sRight.Delete( 0, 1 );
							continue;
						} 
						else 
						{
							sValue = sRight;
							sRight.Empty();
						}
						sValue.TrimLeft();
						sValue.TrimRight();
						if( sValue.IsEmpty() ) 
						{
							continue;
						}

						if( bFirstElement ) 
						{
							nNumValidLines++;	
							bFirstElement = FALSE;
							nFieldsInTable = 0;
							m_anFieldsPerTable.Add( nFieldsInTable );
						}

						// test the value string 
						if( CConvertCsvFile::IsStringNumeric( sValue ) ) 
						{
							// the value string is a float, we are ok
							nNumFloats++;
						}
						nNumFields++;
						nFieldsInTable++;
						m_anFieldsPerTable[nNumValidLines-1] = nFieldsInTable;

						if( nFieldsInTable > nMaxFieldsPerTable ) 
						{
							nMaxFieldsPerTable = nFieldsInTable;
						}
					}
				}
			}
		}
		// move to the next line
		pszLine = GetNextLine( pszLine, pEOF );
	}

	// check for various error conditions
	if( !nNumFields || 
		!nNumValidLines || 
		nMaxFieldsPerTable > FDATA_GAMFILE_MAX_FIELDS_PER_TABLE ||
		nNumValidLines > FDATA_GAMFILE_MAX_TABLES ) 
	{
		fang_Free( pCopyUserData );
		return NULL;
	}

	// grab some memory
	m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );
	pHeader = (FDataGamFile_Header_t *)m_pWorkingMem;
	pTable = (FDataGamFile_Table_t *)&pHeader[1];
	m_pWorkingMem = (u8 *)&pTable[nNumValidLines];
	
	// make sure the we don't over write memory
	FASSERT( (u32)m_pWorkingMem <= (u32)&m_pMemAllocated[m_nNumBytesAllocated] );

	pHeader->nFlags = FDATA_GAMFILE_FLAGS_NONE;
	pHeader->paTables = pTable;

	pszLine = (char *)pCopyUserData;
	while( pszLine ) 
	{
		sLine = pszLine;
		
		if( !sLine.IsEmpty() ) 
		{
			// find the index
			nIndex = sLine.Find( '=', 0 );
			if( nIndex > 0 ) 
			{
				// extract the left and right parts of the string
				sLeft = sLine.Mid( 0, nIndex );
				sRight = sLine.Mid( nIndex+1 );
				sLeft.TrimLeft();
				sLeft.TrimRight();
				sRight.TrimLeft();
				sRight.TrimRight();

				if( !sLeft.IsEmpty() && 
					!sRight.IsEmpty() &&
					!CConvertCsvFile::DoesKeyStringContainInvalidChars( sLeft, TRUE ) ) 
				{
					
					bFirstElement = TRUE;
					while( !sRight.IsEmpty() ) 
					{
						nIndex = sRight.Find( ',', 0 );
						if( nIndex > 0 ) 
						{
							sValue = sRight.Mid( 0, nIndex );
							sRight.Delete( 0, nIndex+1 );
						} 
						else if( nIndex == 0 ) 
						{
							sRight.Delete( 0, 1 );
							continue;
						} 
						else 
						{
							sValue = sRight;
							sRight.Empty();
						}
						sValue.TrimLeft();
						sValue.TrimRight();
						if( sValue.IsEmpty() ) 
						{
							continue;
						}

						if( bFirstElement ) 
						{
							// start a new table
							pTable = &pHeader->paTables[pHeader->nNumTables];
							nFieldsInTable = m_anFieldsPerTable.GetAt( pHeader->nNumTables );
							pTable->nTableIndex = (u16)pHeader->nNumTables;
							pHeader->nNumTables++;

							// set the table keystring and its length
							pTable->pszKeyString = (cchar *)m_pWorkingMem;
							fclib_strcpy( (char *)pTable->pszKeyString, (cchar *)sLeft );
							pTable->nKeyStringLen = fclib_strlen( pTable->pszKeyString );

							// advance our working memory past the string and NULL 
							m_pWorkingMem += (pTable->nKeyStringLen + 1);

							// make sure the we don't over write memory
							FASSERT( (u32)m_pWorkingMem <= (u32)&m_pMemAllocated[m_nNumBytesAllocated] );

							// set the table field array (keep this table aligned)
							m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );
							pTable->paFields = (FDataGamFile_Field_t *)m_pWorkingMem;
							m_pWorkingMem += nFieldsInTable * sizeof( FDataGamFile_Field_t );
							// make sure the we don't over write memory
							FASSERT( (u32)m_pWorkingMem <= (u32)&m_pMemAllocated[m_nNumBytesAllocated] );

							bFirstElement = FALSE;
						}

						// create a field
						pField = &pTable->paFields[ pTable->nNumFields ];
						pTable->nNumFields++;
						FASSERT( pTable->nNumFields <= nFieldsInTable );

						if( CConvertCsvFile::IsStringNumeric( sValue ) ) 
						{
							pField->nDataType = FDATA_GAMEFILE_DATA_TYPE_FLOAT;
							pField->fValue = CConvertCsvFile::ExtractFloatValue( sValue );
						} 
						else 
						{
							pField->nDataType = FDATA_GAMEFILE_DATA_TYPE_STRING;
							pField->pszValue = (cchar *)m_pWorkingMem;
							fclib_strcpy( (char *)pField->pszValue, (cchar *)sValue );
							pField->nStringLen = fclib_strlen( pField->pszValue );
							m_pWorkingMem += (pField->nStringLen + 1);
							
							// make sure the we don't over write memory
							FASSERT( (u32)m_pWorkingMem <= (u32)&m_pMemAllocated[m_nNumBytesAllocated] );
						}						
					}
				}
			}
		}
		// move to the next line
		pszLine = GetNextLine( pszLine, pEOF );
	}

	pHeader->nBytesInFile = (u32)m_pWorkingMem - (u32)pHeader;
	FASSERT( pHeader->nNumTables == nNumValidLines );
	
	CConvertCsvFile::ConvertPtrsToOffsets( pHeader );

	fang_Free( pCopyUserData );

	return pHeader;
}


//
//
//
char *CKongToWorldInitFile::GetNextLine( char *pszLine, char *pszEOF ) 
{
	if( !pszLine || !pszEOF || (pszLine >= pszEOF) ) 
	{
		return NULL;
	}
	if( pszLine[0] != NULL ) 
	{
		// move to the next NULL char
		while( pszLine < pszEOF ) 
		{
			pszLine++;
			if( pszLine[0] == NULL ) 
			{
				break;
			}
		}
		// make sure that we haven't gone too far
		if( pszLine >= pszEOF ) 
		{
			return NULL;
		}
	}

	// move to the 1st non NULL char
	while( pszLine < pszEOF ) 
	{
		pszLine++;
		if( pszLine[0] != NULL ) 
		{
			break;
		}
	}
	// make sure that we haven't gone too far
	if( pszLine >= pszEOF ) 
	{
		return NULL;
	}
	return pszLine;
}




