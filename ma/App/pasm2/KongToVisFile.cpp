//////////////////////////////////////////////////////////////////////////////////////
// KongToVisFile.cpp - 
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
// 05/09/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <direct.h>
#include "fang.h"
#include "pasm.h"
#include "KongToVisFile.h"
#include "ErrorLog.h"
#include "ConvertApeFile.h"
#include "utils.h"
#include "fworld_coll.h"
#include "PasmDlg.h"
#include "CompileDlg.h"

#define _ERROR_HEADING				"KONG->VIS FILE COMPILER "

#define _LARGE_PERCENT_OF_NODE		0.5f

#define _MAX_TRIS_PER_VOLUME		16384
#define _MAX_PRE_CLIPPED_TRIS		150000

extern CPasmApp theApp;


CKongToVisFile::CKongToVisFile() {
	m_bK2VConverted = FALSE;
	m_nStatsNonCrossingTris = 0;
	m_nStatsCrossingTris = 0;
	m_nStatsFinalTriCount = 0;
	m_nStatsVolumes = 0;
	m_nStatsEmptyVolumes = 0;
	m_nStatsCells = 0;
	m_nStatsCellPlanes = 0;
	m_nStatsPortals = 0;
	m_nStatsAutoPortals = 0;
	m_fStatsAvgTrisPerVol = 0;
	m_nStatsMaxVolTris = 0;
	m_nStatsVolumeIndexWithMostTris = 0;
	m_nStatsBytesInFile = 0;
	m_pVisData = NULL;
	m_apVolKongMeshs.RemoveAll();
	m_pConvertedData = NULL;
	m_nLocalBytesAllocated = 0;
	m_pMemAllocated = NULL;
	m_paPortals = NULL;
	m_paVolumes = NULL;
	m_paCells = NULL;
	m_paPlanes = NULL;
	m_paCellInfos = NULL;
	m_paChimps = NULL;
	m_nStatsNumMeshes = 0;
	m_nStatsLights = 0;

	m_nStaticLightingQualityLevel = 0;
	m_bUsedOldVisData = FALSE;

	m_nVisFileStatus = VIS_FILE_NOT_USED;

	m_nNumPD_Portals = 0;
	m_apPD_Portals.RemoveAll();
	m_nNumPD_Volumes = 0;
	m_apPD_Volumes.RemoveAll();
}

CKongToVisFile::~CKongToVisFile() {
	FreeData();
}

#include  <math.h>

void CKongToVisFile::CalculateTangentSpace(KongTri_t *pTri, CFVec3& vTan, CFVec3& vBinorm)
{
	CFVec3 vNorm;
   // Clear outputs.
   vTan.x = 1.0; vTan.y = 0.0; vTan.z = 0.0;
   vBinorm.x = 0.0; vBinorm.y = 0.0; vBinorm.z = 1.0;
   vNorm.x = pTri->FaceUnitNorm.x;
   vNorm.y = pTri->FaceUnitNorm.y;
   vNorm.z = pTri->FaceUnitNorm.z;

   f64 fU1 = pTri->apKongVerts[1]->aUV[0].x - pTri->apKongVerts[0]->aUV[0].x;
   f64 fU2 = pTri->apKongVerts[2]->aUV[0].x - pTri->apKongVerts[0]->aUV[0].x;
   f64 fV1 = pTri->apKongVerts[1]->aUV[0].y - pTri->apKongVerts[0]->aUV[0].y;
   f64 fV2 = pTri->apKongVerts[2]->aUV[0].y - pTri->apKongVerts[0]->aUV[0].y;

   // Make sure we have valid data.
   if ( (fU1 == 0 && fU2 == 0) || (fV1 == 0 && fV2 == 0) )
   {
	   return;
   }
   
   // Compute edge vectors.
   f64 vec1x, vec1y, vec1z;
   vec1x = pTri->apKongVerts[1]->Pos.x - pTri->apKongVerts[0]->Pos.x;
   vec1y = pTri->apKongVerts[1]->Pos.y - pTri->apKongVerts[0]->Pos.y;
   vec1z = pTri->apKongVerts[1]->Pos.z - pTri->apKongVerts[0]->Pos.z;
   f64 vec2x, vec2y, vec2z;
   vec2x = pTri->apKongVerts[2]->Pos.x - pTri->apKongVerts[0]->Pos.x;
   vec2y = pTri->apKongVerts[2]->Pos.y - pTri->apKongVerts[0]->Pos.y;
   vec2z = pTri->apKongVerts[2]->Pos.z - pTri->apKongVerts[0]->Pos.z;
   
   // Calculate u vector
   
   f64 fA, fB;
   if ( fabs(fV2) < 0.00001 )
   {
	   fA = 0.0;
   }
   else
   {
       fA = (fU1 - fV1*fU2/fV2);
   }
   
   if (fA != 0.0)
   {
      fA = 1.0/fA;
   }

   if ( fabs(fV1) < 0.00001 )
   {
	   fB = 0.0;
   }
   else
   {
       fB = (fU2 - fV2*fU1/fV1);
   }
   if (fB != 0.0)
   {
      fB = 1.0/fB;
   }

   f64 fTmp;
   f64 duTmpx, duTmpy, duTmpz;
   duTmpx = fA*vec1x + fB*vec2x;
   duTmpy = fA*vec1y + fB*vec2y;
   duTmpz = fA*vec1z + fB*vec2z;

   if (duTmpx || duTmpy || duTmpz)
   {
	   fTmp = 1.0 / sqrt((duTmpx*duTmpx) + (duTmpy*duTmpy) + (duTmpz*duTmpz));
   }
   else
   {
	   fTmp = 1.0;
   }
   duTmpx *= fTmp;
   duTmpy *= fTmp;
   duTmpz *= fTmp;

   // Calculate v vector
   if ( fabs(fU2) < 0.00001 )
   {
	   fA = 0.0f;
   }
   else
   {
       fA = (fV1 - fU1*fV2/fU2);
   }

   if (fA != 0.0)
   {
      fA = 1.0/fA;
   }

   if ( fabs(fU1) < 0.00001 )
   {
	   fB = 0.0;
   }
   else
   {
       fB = (fV2 - fU2*fV1/fU1);
   }
   if (fB != 0.0)
   {
      fB = 1.0/fB;
   }

   f64 dvTmpx, dvTmpy, dvTmpz;
   dvTmpx = fA*vec1x + fB*vec2x;
   dvTmpy = fA*vec1y + fB*vec2y;
   dvTmpz = fA*vec1z + fB*vec2z;
   if (dvTmpx || dvTmpy || dvTmpz)
   {
       fTmp = 1.0 / sqrt((dvTmpx*dvTmpx) + (dvTmpy*dvTmpy) + (dvTmpz*dvTmpz));
   }
   else
   {
	   fTmp = 1.0;
   }

   dvTmpx *= fTmp;
   dvTmpy *= fTmp;
   dvTmpz *= fTmp;

   // Project the vector into the plane defined by the normal.
   fTmp = (duTmpx*vNorm.x) + (duTmpy*vNorm.y) + (duTmpz*vNorm.z);
   vTan.x = (f32)(duTmpx - (fTmp * vNorm.x));
   vTan.y = (f32)(duTmpy - (fTmp * vNorm.y));
   vTan.z = (f32)(duTmpz - (fTmp * vNorm.z));
   if (vTan.x || vTan.y || vTan.z)
   {
       fTmp = 1.0f / sqrt((f64)vTan.x*(f64)vTan.x + (f64)vTan.y*(f64)vTan.y + (f64)vTan.z*(f64)vTan.z);
   }
   else
   {
	   fTmp = 1.0f;
   }
   vTan.x = (f32)(vTan.x * fTmp);
   vTan.y = (f32)(vTan.y * fTmp);
   vTan.z = (f32)(vTan.z * fTmp);

   fTmp = (dvTmpx*vNorm.x) + (dvTmpy*vNorm.y) + (dvTmpz*vNorm.z);
   vBinorm.x = (f32)(dvTmpx - (fTmp * vNorm.x));
   vBinorm.y = (f32)(dvTmpy - (fTmp * vNorm.y));
   vBinorm.z = (f32)(dvTmpz - (fTmp * vNorm.z));
   if (vBinorm.x || vBinorm.y || vBinorm.z)
   {
       fTmp = 1.0f / sqrt((f64)vBinorm.x*(f64)vBinorm.x + (f64)vBinorm.y*(f64)vBinorm.y + (f64)vBinorm.z*(f64)vBinorm.z);
   }
   else
   {
	   fTmp = 1.0f;
   }
   vBinorm.x = (f32)(vBinorm.x * fTmp);
   vBinorm.y = (f32)(vBinorm.y * fTmp);
   vBinorm.z = (f32)(vBinorm.z * fTmp);
}

BOOL TriInList(KongTri_t *pTri, KongTri_t **pTriL, u8 nTri)
{
	BOOL bRet = FALSE;

	if (pTri)
	{
		u8 i;
		for (i=0; i<nTri; i++)
		{
			if (pTriL[i] == pTri)
			{
				bRet = TRUE;
				break;
			}
		}
	}
	
	return bRet;
}

void CKongToVisFile::BuildNBTs()
{
	#define __USED_TRI_BUFFER_SIZE	64

	u32 i, j, nVtx, nUsedTriCount, nEdgeTri;
	KongTri_t *pTri, *pETri, *pNextETri;
	KongTri_t *pUsedTri[__USED_TRI_BUFFER_SIZE];

	CFVec3 *pBinorm;
	CFVec3 *pTangent;

	pBinorm = fnew CFVec3[m_pKongMesh->nTriangleCount];
	if ( !pBinorm )
	{
		return;
	}
	pTangent = fnew CFVec3[m_pKongMesh->nTriangleCount];
	if ( !pTangent )
	{
		fdelete(pBinorm);
		return;
	}

	for (i=0; i<m_pKongMesh->nVertCount; i++)
	{
		m_pKongMesh->paVertices[i].Binorm.Set(0,0,0);
		m_pKongMesh->paVertices[i].Tangent.Set(0,0,0);
	}
	
	//1. Calc NBT for all triangles:
	for (i=0; i<m_pKongMesh->nTriangleCount; i++)
	{
		pTri = &m_pKongMesh->paTriangles[i];
		FASSERT( pTri->nTriIdx == i );  // We are relying on this assumption never changing
		CalculateTangentSpace(pTri, pTangent[i], pBinorm[i]);
	}

	//2. Ave NBTs for all vertices that share pos & nrml.
	u32 nBothNonZeroMag = 0, nZeroMagOne = 0, nZeroMagBoth = 0;
	for (i=0; i<m_pKongMesh->nTriangleCount; i++)
	{
		pTri = &m_pKongMesh->paTriangles[i];

		for (nVtx=0; nVtx<3; nVtx++)
		{
			pETri = pTri;

			nUsedTriCount = 0;
			fang_MemZero( pUsedTri, sizeof( KongTri_t* ) * __USED_TRI_BUFFER_SIZE );
			while ( pETri )
			{
				FASSERT( pETri->nTriIdx < m_pKongMesh->nTriangleCount );
				pTri->apKongVerts[nVtx]->Binorm += pBinorm[pETri->nTriIdx];
				pTri->apKongVerts[nVtx]->Tangent += pTangent[pETri->nTriIdx];

				// Check the edge triangles to see if they also use this vertex
				pNextETri = NULL;
				for ( nEdgeTri = 0; nEdgeTri < pETri->nNumEdgeTris; nEdgeTri++ )
				{
					pNextETri = pETri->paEdgeTris[nEdgeTri];

					if ( pNextETri != pTri )
					{
						// We haven't circled around to the first
						FASSERT( pNextETri );

						// Check to see if this is a triangle we've already tested
						BOOL bAlreadyChecked = FALSE;
						u32 nUsedTriTest = 0;
						while ( nUsedTriTest < __USED_TRI_BUFFER_SIZE && pUsedTri[nUsedTriTest] )
						{
							if ( pUsedTri[nUsedTriTest++] == pNextETri )
							{
								bAlreadyChecked = TRUE;
								break;
							}
						}

						if ( !bAlreadyChecked )
						{
							// Check each vertex for a match
							for ( j = 0; j < 3; j++ )
							{
								if (   pNextETri->apKongVerts[j] != pTri->apKongVerts[nVtx] 
									&& pNextETri->apKongVerts[j]->Pos == pTri->apKongVerts[nVtx]->Pos 
									&& pNextETri->apKongVerts[j]->Norm == pTri->apKongVerts[nVtx]->Norm )
								{
									// We've got a match, so we can continue with this triangle
									break;
								}
							}

							if ( j < 3 )
							{
								break;
							}
						}
					}

					pNextETri = NULL;
				}

				if ( nUsedTriCount >= __USED_TRI_BUFFER_SIZE )
				{
					DEVPRINTF( "CKongToVisFile::BuildNBTs() - WARNING - Exceeded NBT averaging tri buffer!\n" );
					break;
				}

				pUsedTri[nUsedTriCount] = pETri;
				nUsedTriCount = (nUsedTriCount++) % __USED_TRI_BUFFER_SIZE;
                pETri = pNextETri;
			}

			// Unitize the basis vectors
			f32 fMagBinorm = (f32)sqrt( pTri->apKongVerts[nVtx]->Binorm.Mag2() );
			f32 fMagTangent = (f32)sqrt( pTri->apKongVerts[nVtx]->Tangent.Mag2() );
			if ( fMagBinorm < 0.00001f )
			{
				if ( fMagTangent < 0.00001f )
				{
					if ( pTri->apKongVerts[nVtx]->Norm.x > pTri->apKongVerts[nVtx]->Norm.y && pTri->apKongVerts[nVtx]->Norm.x > pTri->apKongVerts[nVtx]->Norm.z )
					{
						pTri->apKongVerts[nVtx]->Binorm.Set( 0.f, 0.f, 1.f );
						pTri->apKongVerts[nVtx]->Tangent.Set( 0.f, 1.f, 0.f );
					}
					else if ( pTri->apKongVerts[nVtx]->Norm.y > pTri->apKongVerts[nVtx]->Norm.x && pTri->apKongVerts[nVtx]->Norm.y > pTri->apKongVerts[nVtx]->Norm.z )
					{
						pTri->apKongVerts[nVtx]->Binorm.Set( 1.f, 0.f, 0.f );
						pTri->apKongVerts[nVtx]->Tangent.Set( 0.f, 0.f, 1.f );
					}
					else
					{
						pTri->apKongVerts[nVtx]->Binorm.Set( 0.f, 1.f, 0.f );
						pTri->apKongVerts[nVtx]->Tangent.Set( 1.f, 0.f, 0.f );
					}
					nZeroMagBoth++;
				}
				else
				{
					// UV issue probably exists, so we'll rebuild the basis vectors based on the non-zero vector and the normal
					pTri->apKongVerts[nVtx]->Binorm = pTri->apKongVerts[nVtx]->Tangent.Cross( pTri->apKongVerts[nVtx]->Norm );
					fMagBinorm = (f32)sqrt( pTri->apKongVerts[nVtx]->Binorm.Mag2() );
					FASSERT( fMagBinorm );
					pTri->apKongVerts[nVtx]->Binorm *= 1.f / fMagBinorm;
					pTri->apKongVerts[nVtx]->Tangent = pTri->apKongVerts[nVtx]->Binorm.Cross( pTri->apKongVerts[nVtx]->Norm );
					nZeroMagOne++;
				}
			}
			else if ( fMagTangent < 0.00001f )
			{
				// UV issue probably exists, so we'll rebuild the basis vectors based on the non-zero vector and the normal
				pTri->apKongVerts[nVtx]->Tangent = pTri->apKongVerts[nVtx]->Binorm.Cross( pTri->apKongVerts[nVtx]->Norm );
				fMagTangent = (f32)sqrt( pTri->apKongVerts[nVtx]->Tangent.Mag2() );
				FASSERT( fMagTangent );
				pTri->apKongVerts[nVtx]->Tangent *= 1.f / fMagTangent;
				pTri->apKongVerts[nVtx]->Binorm = pTri->apKongVerts[nVtx]->Tangent.Cross( pTri->apKongVerts[nVtx]->Norm );
				nZeroMagOne++;
			}
			else
			{
				pTri->apKongVerts[nVtx]->Binorm *= 1.f / fMagBinorm;
				pTri->apKongVerts[nVtx]->Tangent *= 1.f / fMagTangent;
				nBothNonZeroMag++;
			}

#if FANG_DEBUG_BUILD
			f32 fNormalMag = pTri->apKongVerts[nVtx]->Binorm.Mag();
			if ( fNormalMag < 0.99f || fNormalMag > 1.01f )
			{
				FASSERT_NOW;
			}
			fNormalMag = pTri->apKongVerts[nVtx]->Tangent.Mag();
			if ( fNormalMag < 0.99f || fNormalMag > 1.01f )
			{
				FASSERT_NOW;
			}
#endif
		}
	}

	fdelete(pBinorm);
	fdelete(pTangent);

	#undef __USED_TRI_BUFFER_SIZE
}

BOOL CKongToVisFile::MeshHasBumpMap()
{
	u32 i;
	BOOL bRet = FALSE;
	ApeMaterial_t *pMat;

	for (i=0; i<m_pKongMesh->nMaterialCount; i++)
	{
		pMat = GetMatInfoFromMeshMatIndex(i);

		if (pMat)
		{
			if (pMat->aLayer[0].szTexnames[ APE_LAYER_TEXTURE_BUMP ][0])
			{
				bRet = TRUE;
				break;
			}
		}
	}
	
	return (bRet);
}


//
//
//
BOOL CKongToVisFile::SaveVisFile( CCompileDlg *pDlg, cchar *pszFilename, u32 nLMLevel, f32 fLMMemory ) 
{
	u32 i, nBytesWritten, nTotalBytesWritten = 0;
	CString csName;
	FASSERT( pDlg && pszFilename );

	// Make sure the directory exists:
	CString csDirectoryName = *CPasmDlg::m_pStaticLocalDir + "\\VIS\\";
	CreateDirectory( csDirectoryName, NULL ); // This will fail if the directory already exists

	// Create/Overwrite the file for this world
	csName = *CPasmDlg::m_pStaticLocalDir + "\\VIS\\" + pszFilename;
	csName.MakeLower();
	csName.Replace( ".wld", ".vis" );
	HANDLE hFile = CreateFile( csName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	// Fill out the file header
	FileHeader.nSignature		= 0x2B00B1E5;
	FileHeader.nVisVersion		= VIS_FILE_VERSION;
	FileHeader.nKongVersion		= KONG_FILE_VERSION;
	FileHeader.nVisDataVersion	= VIS_DATA_VERSION;
	FileHeader.nInitDataVersion	= INIT_DATA_VERSION;
	FileHeader.nWLDCRC			= m_nFileCRCValue;
	FileHeader.nAIDCRC			= m_nAIDFileCRCValue;
	FileHeader.bHasAidFile		= m_pKongMesh->KHeader.bHasAidFile;
	FileHeader.nLMLevel			= nLMLevel;
	FileHeader.fLMMemory		= fLMMemory;
	FileHeader.nVolumeMeshCount	= m_nStatsNumMeshes;
	FileHeader.nVisDataBytes	= m_nStatsBytesInFile;
	FileHeader.nInitDataBytes	= m_InitFile.GetConvertedDataSize();

	// Write out the file header
	WriteFile( hFile, &FileHeader, sizeof( VisFileHeader_t ), (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != sizeof( VisFileHeader_t ) )
	{
		CloseHandle( hFile );
		DeleteFile( csName );
		return FALSE;
	}
	nTotalBytesWritten += nBytesWritten;

	// Write out each of the volume Kong meshes
	for ( i = 0; i < m_nStatsNumMeshes; i++ ) 
	{	
		nBytesWritten = ((CApeToKongFormat *)m_apVolKongMeshs[i])->m_pKongMesh->SaveKong( hFile );
		if ( nBytesWritten == 0 )
		{
			CloseHandle( hFile );
			DeleteFile( csName );
			return FALSE;
		}
		nTotalBytesWritten += nBytesWritten;
	}

	// Write out the progress key
	u32 nKey = 0xAB00B000;
	WriteFile( hFile, &nKey, sizeof( u32 ), (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != sizeof( u32 ) )
	{
		CloseHandle( hFile );
		DeleteFile( csName );
		return FALSE;
	}
	nTotalBytesWritten += nBytesWritten;

	// Write out the FVis data
	WriteFile( hFile, m_pConvertedData, m_nStatsBytesInFile, (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != m_nStatsBytesInFile )
	{
		CloseHandle( hFile );
		DeleteFile( csName );
		return FALSE;
	}
	nTotalBytesWritten += nBytesWritten;

	// Write out shape data
	nBytesWritten = m_InitFile.SaveInitDataToVIS( hFile );
	if ( nBytesWritten != m_InitFile.m_nStatsShapeBytes )
	{
		CloseHandle( hFile );
		DeleteFile( csName );
		return FALSE;
	}
	nTotalBytesWritten += nBytesWritten;

	// Write out the progress key
	nKey = 0xAB00B000;
	WriteFile( hFile, &nKey, sizeof( u32 ), (LPDWORD)&nBytesWritten, NULL );
	if ( nBytesWritten != sizeof( u32 ) )
	{
		CloseHandle( hFile );
		DeleteFile( csName );
		return FALSE;
	}
	nTotalBytesWritten += nBytesWritten;

	CloseHandle( hFile );

	return TRUE;
}


//
//
//
u32 CKongToVisFile::VisFileUpToDate( CString &csPath, BOOL bLibrary, const CFileInfo *pAidFileInfo, u32 nLMLevel, f32 fLMMemory, 
									VisFileHeader_t &VisHeader, HANDLE &hFile, CString &csError ) 
{
	u32 nBytesRead;

	hFile = CreateFile( csPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		csError = "No VIS file.";
		return VIS_FILE_DOES_NOT_EXIST;
	}

	// Read the VIS header
	ReadFile( hFile, &VisHeader, sizeof( VisFileHeader_t ), (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != sizeof( VisFileHeader_t ) )
	{
		CloseHandle( hFile );
		csError = "VIS file is corrupt.";
		return VIS_FILE_INVALID;
	}

	// Check version numbers
	if (    VisHeader.nSignature       != 0x2B00B1E5
		 || VisHeader.nVisVersion      != VIS_FILE_VERSION
		 || VisHeader.nKongVersion     != KONG_FILE_VERSION
		 || VisHeader.nVisDataVersion  != VIS_DATA_VERSION
		 || VisHeader.nInitDataVersion != INIT_DATA_VERSION )
	{
		CloseHandle( hFile );
		csError = "VIS file is an incorrect version.";
		return VIS_FILE_OUT_OF_DATE;
	}

	if ( VisHeader.nWLDCRC != m_nFileCRCValue )
	{
		CloseHandle( hFile );
		csError = "VIS is out-of-date.";
		return VIS_FILE_OUT_OF_DATE;
	}
/*
	if ( !pAidFileInfo && VisHeader.bHasAidFile || pAidFileInfo && !VisHeader.bHasAidFile )
	{
		CloseHandle( hFile );
		csError = "VIS is out-of-date.";
		return VIS_FILE_OUT_OF_DATE;
	}

	if ( VisHeader.bHasAidFile )
	{
		if ( VisHeader.nAIDCRC, m_nAIDFileCRCValue )
		{
			CloseHandle( hFile );
			csError = "VIS is out-of-date.";
			return VIS_FILE_OUT_OF_DATE;
		}
	}
*/
	if ( VisHeader.nLMLevel < nLMLevel )
	{
		CloseHandle( hFile );
		csError = "VIS file has lower lightmap detail levels.";
		return VIS_FILE_LOWER_DETAIL;
	}

	if ( VisHeader.nLMLevel > 0 && VisHeader.nLMLevel == nLMLevel && VisHeader.fLMMemory < fLMMemory )
	{
		CloseHandle( hFile );
		csError = "VIS file has lower lightmap detail levels.";
		return VIS_FILE_LOWER_DETAIL;
	}

	if ( bLibrary )
	{
		return VIS_FILE_UP_TO_DATE_LIBRARY;
	}

	return VIS_FILE_UP_TO_DATE_LOCALLY;
}


//
//
//
BOOL CKongToVisFile::RemoveLocalVISData( const char *pszLocalPath, const char *pszWLDName ) 
{
	CString csLocalPath = pszLocalPath;
	CString csWLDName = pszWLDName;

	// Delete vis and batch file
	DeleteFile( csLocalPath );
	csLocalPath = csLocalPath.Left( csLocalPath.GetLength() - 4 ) + ".bat";
	DeleteFile( csLocalPath );

	// Remove the lightmaps
	BOOL bFileResult = TRUE;
	WIN32_FIND_DATA FileData;
	csLocalPath = *CPasmDlg::m_pStaticLocalDir + "\\LightMaps\\" + csWLDName + "\\*.*";
	HANDLE hFile = FindFirstFile( csLocalPath, &FileData );
	while ( bFileResult && hFile != INVALID_HANDLE_VALUE )
	{
		csLocalPath = *CPasmDlg::m_pStaticLocalDir + "\\LightMaps\\" + csWLDName + "\\" + FileData.cFileName;
		bFileResult = DeleteFile( csLocalPath );
		bFileResult = FindNextFile( hFile, &FileData );
	}
	FindClose( hFile );

	// Remove the lightmaps directory
	csLocalPath = *CPasmDlg::m_pStaticLocalDir + "\\LightMaps\\" + csWLDName;
	RemoveDirectory( csLocalPath );

	return TRUE;
}


//
//
//
BOOL CKongToVisFile::CheckForVisFile( CCompileDlg *pDlg, const CFileInfo *pFileInfo, const CFileInfo *pAidFileInfo, u32 nLMLevel, f32 fLMMemory ) 
{
    FASSERT( pDlg && pFileInfo );
	u32 i, nKey, nBytesRead;
	VisFileHeader_t VisHeader, LibraryVisHeader, LocalVisHeader;
	CString csFilePath, csError;
	CFileInfo FileInfo;
	HANDLE hFile, hLocalFile, hLibraryFile;
	u32 nLibraryVisFileStatus, nLocalVisFileStatus;

	m_nVisFileStatus = VIS_FILE_NOT_USED;

	CString csLocalPath = *CPasmDlg::m_pStaticLocalDir + "\\VIS\\" + pFileInfo->GetFileTitle() + ".vis";
	csLocalPath.MakeLower();
	nLocalVisFileStatus = VisFileUpToDate( csLocalPath, FALSE, pAidFileInfo, nLMLevel, fLMMemory, LocalVisHeader, hLocalFile, csError );
	if ( nLocalVisFileStatus != VIS_FILE_DOES_NOT_EXIST )
	{
		if ( nLocalVisFileStatus != VIS_FILE_UP_TO_DATE_LOCALLY )
		{
			CString csLocalError = "Local " + csError;
			pDlg->InfoString( csLocalError );
		}
	}

	CString csLibraryPath = ((CPasmDlg *)theApp.m_pMainWnd)->m_sLibDir + "\\VIS\\" + pFileInfo->GetFileTitle() + ".vis";
	csLibraryPath.MakeLower();
	nLibraryVisFileStatus = VisFileUpToDate( csLibraryPath, TRUE, pAidFileInfo, nLMLevel, fLMMemory, LibraryVisHeader, hLibraryFile, csError );
	if ( nLibraryVisFileStatus != VIS_FILE_DOES_NOT_EXIST )
	{
		if ( nLibraryVisFileStatus != VIS_FILE_UP_TO_DATE_LIBRARY )
		{
			CString csLibraryError = "Library " + csError;
			pDlg->InfoString( csLibraryError );
		}
	}

	if ( nLocalVisFileStatus != VIS_FILE_UP_TO_DATE_LOCALLY )
	{
		if ( nLibraryVisFileStatus != VIS_FILE_UP_TO_DATE_LIBRARY )
		{
			// No valid library or local VIS file
			m_nVisFileStatus = VIS_FILE_DOES_NOT_EXIST;
			return FALSE;
		}

		// Remove the local vis data since it isn't up to date but the library vis is
		RemoveLocalVISData( csLocalPath, pFileInfo->GetFileTitle() );

		// Use the valid library VIS file
		hFile = hLibraryFile;
		VisHeader = LibraryVisHeader;
		csFilePath = csLibraryPath;
		m_nVisFileStatus = nLibraryVisFileStatus;
		pDlg->InfoString( "Library VIS file is up-to-date and is same or better quality (Level %d, %2.2f MB).", LibraryVisHeader.nLMLevel, LibraryVisHeader.fLMMemory );
	}
	else if ( nLibraryVisFileStatus != VIS_FILE_UP_TO_DATE_LIBRARY )
	{
		// Use the valid local VIS file
		hFile = hLocalFile;
		VisHeader = LocalVisHeader;
		csFilePath = csLocalPath;
		m_nVisFileStatus = nLocalVisFileStatus;
		pDlg->InfoString( "Local VIS file is up-to-date and is same or better quality (Level %d, %2.2f MB).", LocalVisHeader.nLMLevel, LocalVisHeader.fLMMemory );
	}
	else
	{
		// Both the local VIS and the library VIS are valid.  See which VIS file is better
		if ( LibraryVisHeader.nLMLevel >= LocalVisHeader.nLMLevel
			|| (LibraryVisHeader.nLMLevel == LocalVisHeader.nLMLevel && LibraryVisHeader.fLMMemory >= LocalVisHeader.fLMMemory) )
		{
			if ( LibraryVisHeader.nLMLevel == LocalVisHeader.nLMLevel && LibraryVisHeader.fLMMemory == LocalVisHeader.fLMMemory )
			{
				// Library and local are the same, so remove the local
				RemoveLocalVISData( csLocalPath, pFileInfo->GetFileTitle() );
			}

			// Use the valid library VIS file
			CloseHandle( hLocalFile );
			hFile = hLibraryFile;
			VisHeader = LibraryVisHeader;
			csFilePath = csLibraryPath;
			m_nVisFileStatus = nLibraryVisFileStatus;
			pDlg->InfoString( "Library VIS file is up-to-date and is same or better quality (Level %d, %2.2f MB).", LibraryVisHeader.nLMLevel, LibraryVisHeader.fLMMemory );
		}
		else
		{
			// Use the valid local VIS file
			CloseHandle( hLibraryFile );
			hFile = hLocalFile;
			VisHeader = LocalVisHeader;
			csFilePath = csLocalPath;
			m_nVisFileStatus = nLocalVisFileStatus;
			pDlg->InfoString( "Local VIS file is up-to-date and is same or better quality (Level %d, %2.2f MB).", LocalVisHeader.nLMLevel, LocalVisHeader.fLMMemory );
		}
	}

	/////////////////////////////////////////////////////////////////////
	// The saved VIS file was created from the current WLD file, so use it

	FileHeader = VisHeader;

	m_sLastFileLoaded = csFilePath;
	m_nStatsNumMeshes = VisHeader.nVolumeMeshCount;
	m_nStatsBytesInFile = VisHeader.nVisDataBytes;
	m_pConvertedData = NULL;
	void *pInitData = NULL;

	m_paChimps = new CKongToChimp[m_nStatsNumMeshes];
	m_apVolKongMeshs.SetSize( m_nStatsNumMeshes );
	// setup chimps from our master Kong template
	for( i=0; i < m_nStatsNumMeshes; i++ ) {
		m_apVolKongMeshs[i] = &m_paChimps[i];
		m_paChimps[i].m_pKongMesh = new KongMesh_t;
	}

	// Read each of the volume Kong meshes
	for ( i = 0; i < m_nStatsNumMeshes; i++ ) 
	{	
		nBytesRead = ((CApeToKongFormat *)m_apVolKongMeshs[i])->m_pKongMesh->LoadKong( hFile );
		if ( nBytesRead == 0 )
		{
			goto _LOAD_ERROR;
		}
		((CApeToKongFormat *)m_apVolKongMeshs[i])->m_bA2KConverted = TRUE;
	}

	// Read the progress key
	ReadFile( hFile, &nKey, sizeof( u32 ), (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != sizeof( u32 ) )
	{
		goto _LOAD_ERROR;
	}

	if ( nKey != 0xAB00B000 )
	{
		goto _LOAD_ERROR;
	}

	// Read the FVis data
	m_pConvertedData = fang_Malloc( VisHeader.nVisDataBytes, 4 );
	if ( !m_pConvertedData )
	{
		goto _LOAD_ERROR;
	}
	ReadFile( hFile, m_pConvertedData, VisHeader.nVisDataBytes, (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != VisHeader.nVisDataBytes )
	{
		goto _LOAD_ERROR;
	}

	// Read shape data
	nBytesRead = m_InitFile.LoadInitDataFromVIS( hFile, VisHeader.nInitDataBytes );
	if ( nBytesRead != VisHeader.nInitDataBytes )
	{
		goto _LOAD_ERROR;
	}

	// Read the progress key
	ReadFile( hFile, &nKey, sizeof( u32 ), (LPDWORD)&nBytesRead, NULL );
	if ( nBytesRead != sizeof( u32 ) )
	{
		goto _LOAD_ERROR;
	}

	if ( nKey != 0xAB00B000 )
	{
		goto _LOAD_ERROR;
	}

	m_bK2VConverted = TRUE;
	m_bA2KConverted = TRUE;

	CloseHandle( hFile );

	return TRUE;

_LOAD_ERROR:
	CloseHandle( hFile );
	m_nVisFileStatus = VIS_FILE_INVALID;
	pDlg->ErrorString( "Error reading .vis file.  Starting compile from scratch." );
	for( i=0; i < m_nStatsNumMeshes; i++ ) 
	{
		delete m_paChimps[i].m_pKongMesh;
		m_paChimps[i].m_pKongMesh = NULL;
	}
	if ( m_pConvertedData )
	{
		fang_Free( m_pConvertedData );
		m_pConvertedData = NULL;
	}
	if ( pInitData )
	{
		m_InitFile.FreeData();
	}
	return FALSE;
}


//
//
//
BOOL CKongToVisFile::ConvertKongFile( CCompileDlg *pDlg, cchar *pszFilename, u32 nSigDigits/*=3*/ ) 
{
	BOOL bMeshHasBump=FALSE;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	if ( !pDlg->m_bBuildingLightMaps )
	{
		pDlg->InfoString( "    Converting APE to KNG..." );
	}

	if( !ConvertApeFile( pDlg, pszFilename, nSigDigits ) ) {
		FreeData();
		return FALSE;
	}

	if ( pDlg->m_bAbortCompile )
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + *pszFilename );
		rErrorLog.WriteErrorLine( "Compiling aborted by user." );
		return FALSE;
	}

	u32 nStartTime = timeGetTime();

	u32 i;
	for ( i = 0; i < m_pKongMesh->nMaterialCount; i++ )
	{
		if (m_pKongMesh->paMaterials)
		{
			if ( m_pKongMesh->paMaterials[i].pProperties->aLayer[0].szTexnames[APE_LAYER_TEXTURE_BUMP][0] )
			{
				bMeshHasBump = TRUE;
			}
		}
	}
/*
	if (MeshHasBumpMap())
	{
		bMeshHasBump = TRUE;
	}
*/
	if( !m_pKongMesh->bWorldFile && !bMeshHasBump ) 
	{
		// this ape file isn't a world, don't fail, because the kong and ape files are ok
		// we have already set m_bK2VConverted = FALSE
		return TRUE;
	}

	// calculate the min max pts of all of the tris in the world
	CFVec3 KongMin, KongMax, BoundingMin, BoundingMax, Temp;
	FindMinAndMaxPosFromKongTris( KongMin, KongMax, m_pKongMesh->paTriangles, m_pKongMesh->nTriangleCount, FALSE );
	if( m_nStatsVolumes > 1 ) {
		BoundingMin.Min( KongMin );
		BoundingMax.Max( KongMax );
	} else {
		BoundingMin = KongMin;
		BoundingMax = KongMax;
	}
	
	// Build the connectivity information for the world mesh (across materials)
	nStartTime = timeGetTime();
	BuildEdgeDataAcrossEntireKong( KongMin, KongMax, pDlg );
	pDlg->InfoString( "Built Mesh Connectivity (%f seconds)", (f32)(timeGetTime() - nStartTime) / 1000.f );

	if ( pDlg->m_bAbortCompile )
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + *pszFilename );
		rErrorLog.WriteErrorLine( "Compiling aborted by user." );
		return FALSE;
	}

	if (bMeshHasBump)
	{
		BuildNBTs();
	}

	if( !m_pKongMesh->bWorldFile ) 
	{
		// this ape file isn't a world, don't fail, because the kong and ape files are ok
		// we have already set m_bK2VConverted = FALSE
		return TRUE;
	}

	fvis_SetActiveVisData( NULL );

	nStartTime = timeGetTime();
	if ( pDlg )
	{
		pDlg->ResetSubOp( "Building Visibility Data...", 5 );
		pDlg->PostMessage( CDLG_UPDATE_CONTROLS );
	}

#if !FANG_USE_PORTAL
	// this class is only valid with portals turned on
	rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
	rErrorLog.WriteErrorLine( "Pasm must be compiled with portals turned on before fvis data can be made." );
	return FALSE;	
#else

	if( m_pKongMesh->nSegmentCount > 1 ||
		m_bSkinned == TRUE ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "World files can not be skinned or have more than 1 segment, no vis data can be created." );
		return FALSE;	
	}

	////////////////////////
	// declare vis vars here
	ApeFog_t *pApeFog;
	u32 j, k, nCellIndex, nPlaneIndex, nVolPortalIndicesBytes, nMeshIndex, nNumApeVols, nNumApePortals;
	ApeVisPortal_t *pApePort;
	FVisPortal_t *pVisPort;
	ApeVisVolume_t *pApeVol;
	FVisVolume_t *pVisVol;
	FVisCell_t *pVisCell;
	_CellInfo_t *pCellInfo;
	CPortalData_Portal *pPD_Portal;
	CPortalData_Vol *pPD_Vol, *pPD_Vol2;
	CPortalData_Cell *pPD_Cell;
	CPortalData_Face *pPD_Face;
	BOOL bErrorsFound = FALSE;
	CString sErrorLine;
	CFVec3 P1, P2;
	CFVec3A HalfBox;
	CFSphere WorldBoundingSphere;

	//////////////////////////////
	// gather counts from ape file
	nNumApeVols = GetNumVisVols();
	nNumApePortals = GetNumVisPortals();	
#if 0// allow the slop bucket to be the only volume
	if( !nNumApeVols ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "A world must contain at least 1 cell create vis data." );
		goto _ERROR;
	}
#endif
	if( nNumApeVols >= 0xFFFF ||
		nNumApePortals >= 0xFFFF ||
		GetNumVisCells() >= 0xFFFF ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "There are too many portals, cells, or volumes, please reduce." );
		goto _ERROR;
	}

	// only allow a certain number of tris in the slop bucket
		if( (nNumApeVols == 0) && (m_pKongMesh->nTriangleCount > _MAX_TRIS_PER_VOLUME) ) {
		sErrorLine.Format( "There are %d tris and no placed volumes.\n"
						   "\tThe max triangle count for the slop bucket is %d, please add more volumes.",
						   m_pKongMesh->nTriangleCount, _MAX_TRIS_PER_VOLUME );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( sErrorLine );
		goto _ERROR;
	}

	// limit the total size of the acceptable world files, anything over this needs to be chopped up
	if( m_pKongMesh->nTriangleCount > _MAX_PRE_CLIPPED_TRIS ) {
		sErrorLine.Format( "There are %d tris, this is too many for a single world.\n"
						   "\tThe max triangle count for a single world is %d, please break up into multiple chunks.",
						   m_pKongMesh->nTriangleCount, _MAX_PRE_CLIPPED_TRIS );
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( sErrorLine );
		goto _ERROR;
	}

	/////////////////////////////////////////////////////////////////////////
	// convert our ape vis data to our intermediate format (see PortalData.h)
	m_nNumPD_Portals = 0;
	for( i=0; i < nNumApePortals; i++ ) {
		pApePort = GetVisPortal( i );

		pPD_Portal = new CPortalData_Portal;
		if( !pPD_Portal ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Could not allocate memory to convert a placed portal." );
			goto _ERROR;
		}
		pPD_Portal->Create( *pApePort );

		m_apPD_Portals.Add( pPD_Portal );
		m_nNumPD_Portals++;
	}
	
	if ( pDlg )
	{
		pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
	}

	m_nStatsCells = 0;
	m_nStatsCellPlanes = 0;
	m_nNumPD_Volumes = 0;
	for( i=0; i < nNumApeVols; i++ ) {
		pApeVol = GetVisVol( i );

		pPD_Vol = new CPortalData_Vol;
		if( !pPD_Vol ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Could not allocate memory to convert a placed volume." );
			goto _ERROR;
		}
		if( !pPD_Vol->Create( *pApeVol ) ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "Could not create a volume class from a placed volume." );
			goto _ERROR;
		}

		m_apPD_Volumes.Add( pPD_Vol );
		m_nNumPD_Volumes++;

		// fix up any overlapping cells within this volume
		if( !pPD_Vol->ClipOverlappingCells( sErrorLine ) ) {
			// there was a problem clipping this cell
			if( !bErrorsFound ) {
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
				bErrorsFound = TRUE;
			}
			rErrorLog.WriteErrorLine( sErrorLine );
		}

		m_nStatsCells += pPD_Vol->m_nNumCells;
		m_nStatsCellPlanes += pPD_Vol->GetNumOfCellFaces();
	}

	if( bErrorsFound ) {
		// errors have already been logged, fail
		goto _ERROR;
	}
	
	// fix up any overlapping volumes (this may create new auto-portals)
	if( m_nNumPD_Volumes ) {
		for( i=0; i < (m_nNumPD_Volumes-1); i++ ) {
			pPD_Vol = (CPortalData_Vol *)m_apPD_Volumes[i];

			for( j=(i+1); j < m_nNumPD_Volumes; j++ ) {
				pPD_Vol2 = (CPortalData_Vol *)m_apPD_Volumes[j];

				// fix up any overlapping cells within these 2 volumes
				if( !CPortalData_Vol::ClipOverlappingVols( *pPD_Vol, *pPD_Vol2, sErrorLine, m_apPD_Portals, m_nNumPD_Portals ) ) {
					// there was a problem clipping the volume
					if( !bErrorsFound ) {
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
						bErrorsFound = TRUE;
					}
					rErrorLog.WriteErrorLine( sErrorLine );
				}
			}
		}

		if( bErrorsFound ) {
			// errors have already been logged, fail
			goto _ERROR;
		}
	}

	if ( pDlg )
	{
		pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
	}

	// recount our cells & planes (the number changed during volume clipping)
	m_nStatsCells = 0;
	m_nStatsCellPlanes = 0;
	for( i=0; i < m_nNumPD_Volumes; i++ ) {
		pPD_Vol = (CPortalData_Vol *)m_apPD_Volumes[i];

		m_nStatsCells += pPD_Vol->m_nNumCells;
		m_nStatsCellPlanes += pPD_Vol->GetNumOfCellFaces();

		pPD_Vol->CalculateBoundingBox( P1, P2 );
		BoundingMin.Min( P1 );
		BoundingMax.Max( P2 );
	}
	
	// add a fudge factor on the min/max
	BoundingMin = BoundingMin - 5.0f;
	BoundingMax = BoundingMax + 5.0f;
	
	// calculate a sphere about the bounding box
	Temp = BoundingMax - BoundingMin;
	Temp *= 0.5f;
	WorldBoundingSphere.m_Pos = BoundingMin + Temp;
	WorldBoundingSphere.m_fRadius = Temp.Mag();
	WorldBoundingSphere.m_fRadius *= UTILS_BOUNDING_SPHERE_FUDGE_FACTOR_MULTIPLIER;

	// update our counts
	m_nStatsVolumes = m_nNumPD_Volumes;
	m_nStatsEmptyVolumes = 0;
	m_nStatsPortals = m_nNumPD_Portals;
	m_nStatsAutoPortals = 0;
	m_nStatsLights = GetNumLights();
	
	// add a volume and cell for the slop volume
	m_nStatsVolumes++;
	m_nStatsCells++;
	m_nStatsCellPlanes += 6;

	/////////////////////////////////////////////////////
	// allocate memory (some needs aligned, some doesn't)
	m_nLocalBytesAllocated = sizeof( FVisData_t ) +
							 (3 * m_nStatsCells * sizeof( FVisCellTreeNode_t )) +
							 ((m_nStatsVolumes + m_nStatsPortals) * sizeof( u32 )) + // even though indices are u16, a little fudge factor never hurts
							 (3 * m_nStatsCells * m_nStatsCells * sizeof( u32 ) ) +
							 (m_nStatsLights * sizeof( FLightInit_t ) );
	m_nLocalBytesAllocated <<= 2;// a huge fudge factor
	m_nLocalBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nLocalBytesAllocated, 4096 );// a little extra fudge factor

	m_paCellInfos = (_CellInfo_t *)fang_MallocAndZero( sizeof( _CellInfo_t ) * m_nStatsCellPlanes, 16 );
	m_pMemAllocated = (u8 *)fang_MallocAndZero( m_nLocalBytesAllocated, 16 );
	m_paPortals = (FVisPortal_t *)fang_MallocAndZero( sizeof( FVisPortal_t ) * m_nStatsPortals, 16 );
	m_paVolumes = (FVisVolume_t *)fang_MallocAndZero( sizeof( FVisVolume_t ) * m_nStatsVolumes, 16 );
	m_paCells = (FVisCell_t *)fang_MallocAndZero( sizeof( FVisCell_t ) * m_nStatsCells, 16 );
	m_paPlanes = (FVisPlane_t *)fang_MallocAndZero( sizeof( FVisPlane_t ) * m_nStatsCellPlanes, 16 );
	m_paChimps = new CKongToChimp[m_nStatsVolumes];

	if( !m_pMemAllocated ||
		!m_paPortals ||
		!m_paVolumes ||
		!m_paCells ||
		!m_paPlanes ||
		!m_paCellInfos ||
		!m_paChimps ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not allocate memory to create the world vis data." );
		goto _ERROR;
	}
	
	/////////////////////
	// setup our pointers
	m_pVisData = (FVisData_t *)m_pMemAllocated;
	m_pWorkingMem = (u8 *)&m_pVisData[1];

	m_pVisData->paPortals = m_paPortals;
	m_pVisData->paVolumes = m_paVolumes;
	m_pVisData->paCells = m_paCells;	
	
	///////////////////////
	// setup our fog values
	if( GetNumFogs() ) {
		pApeFog = GetFogInfo( 0 );
		m_pVisData->fFogStartZ = pApeFog->fStartDist;
		m_pVisData->fFogEndZ = pApeFog->fEndDist;
		m_pVisData->FogMotif.ColorRGB = pApeFog->Color;
		m_pVisData->FogMotif.fAlpha = 1.0f;
		m_pVisData->FogMotif.nMotifIndex = pApeFog->nMotifID;
	}

	/////////////////////////
	// setup our light values
	CreateLightList();

	/////////////////////
	// create portal data
	for( i=0; i < m_nStatsPortals; i++ ) {
		pPD_Portal = (CPortalData_Portal *)m_apPD_Portals[i];
		
		pVisPort = &m_pVisData->paPortals[ m_pVisData->nPortalCount ];
		
		pVisPort->nFlags = (u16)pPD_Portal->m_nFlags;
		if( pPD_Portal->m_nFlags & FVIS_PORTAL_FLAG_AUTOPORTAL ) {
//			pVisPort->nFlags |= FVIS_PORTAL_FLAG_AUTOPORTAL;	
			m_nStatsAutoPortals++;
		}
		pVisPort->nPortalID = (u16)i;
		pVisPort->avVertices[0] = pPD_Portal->m_aCorners[0];
		pVisPort->avVertices[1] = pPD_Portal->m_aCorners[1];
		pVisPort->avVertices[2] = pPD_Portal->m_aCorners[2];
		pVisPort->avVertices[3] = pPD_Portal->m_aCorners[3];
		pVisPort->vNormal = pPD_Portal->m_Normal;
		pVisPort->nVertCount = 4;
		pPD_Portal->ComputeBoundingSphere( pVisPort->spBoundingWS );
		pVisPort->fBoundingRadiusSq = pVisPort->spBoundingWS.m_fRadius * pVisPort->spBoundingWS.m_fRadius;

		// Determine largest normal axis
		if ( fmath_Abs(pVisPort->vNormal.x) > fmath_Abs(pVisPort->vNormal.y) && fmath_Abs(pVisPort->vNormal.x) > fmath_Abs(pVisPort->vNormal.z) )
		{
			pVisPort->nFlags |= FVIS_PORTAL_FLAG_NORM_X_LARGEST;
		}
		else if  ( fmath_Abs(pVisPort->vNormal.y) > fmath_Abs(pVisPort->vNormal.x) && fmath_Abs(pVisPort->vNormal.y) > fmath_Abs(pVisPort->vNormal.z) )
		{
			pVisPort->nFlags |= FVIS_PORTAL_FLAG_NORM_Y_LARGEST;
		}
		else
		{
			pVisPort->nFlags |= FVIS_PORTAL_FLAG_NORM_Z_LARGEST;
		}

		// NOTE: we'll set the other fields after the cells and volumes are setup
		pVisPort->anAdjacentVolume[0] = FVIS_INVALID_VOLUME_ID;
		pVisPort->anAdjacentVolume[1] = FVIS_INVALID_VOLUME_ID;
		pVisPort->anIdxInVolume[0] = 0;
		pVisPort->anIdxInVolume[1] = 0;

		m_pVisData->nPortalCount++;
	}

	if ( pDlg )
	{
		pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
	}

	//////////////////////
	// create volume array
	nCellIndex = 0;
	nVolPortalIndicesBytes = m_nStatsPortals * sizeof( u16 );
	nVolPortalIndicesBytes = FMATH_BYTE_ALIGN_UP( nVolPortalIndicesBytes, 4 );

	// add the slop volume as the first volume
	pVisVol = &m_pVisData->paVolumes[0];
	pVisVol->nVolumeID = 0;
	pVisVol->nFlags = 0;
	pVisVol->spBoundingWS = WorldBoundingSphere;
	pVisVol->nCellCount = 1;
	pVisVol->nCellFirstIdx = (u16)nCellIndex;
	nCellIndex += 1;
	pVisVol->nActiveAdjacentSteps = 0;
	pVisVol->nActiveStepDecrement = 0;
	pVisVol->pWorldGeo = FVIS_INVALID_GEO_ID;// assume that the volumes doesn't have geometry, if it does we'll fix that later
	pVisVol->nPortalCount = 0;
	pVisVol->paPortalIndices = (u16 *)m_pWorkingMem;
	m_pWorkingMem += nVolPortalIndicesBytes;
	m_pVisData->nVolumeCount++;

	// add the rest of the volumes
	for( i=1; i < m_nStatsVolumes; i++ ) {
		pPD_Vol = (CPortalData_Vol *)m_apPD_Volumes[i-1];

		pVisVol = &m_pVisData->paVolumes[ m_pVisData->nVolumeCount ];

		pVisVol->nVolumeID = (u16)i;
		pVisVol->nFlags = 0;
		pVisVol->spBoundingWS = pPD_Vol->m_BoundingSphere;
		FASSERT( pPD_Vol->m_nNumCells < 0xFF );
		pVisVol->nCellCount = (u8)pPD_Vol->m_nNumCells;
		pVisVol->nCellFirstIdx = (u16)nCellIndex;
		nCellIndex += pPD_Vol->m_nNumCells;
		pVisVol->nActiveAdjacentSteps = 1;
		pVisVol->nActiveStepDecrement = 1;
		pVisVol->pWorldGeo = FVIS_INVALID_GEO_ID;// assume that the volumes doesn't have geometry, if it does we'll fix that later

		// allocate enough portal indices in case every one are contained in this volume
		pVisVol->nPortalCount = 0;
		pVisVol->paPortalIndices = (u16 *)m_pWorkingMem;
		m_pWorkingMem += nVolPortalIndicesBytes;
		
		// NOTE: we'll set the other fields after the cells and volumes are setup
		
		m_pVisData->nVolumeCount++;
	}

	/////////////////////
	// create cell array
	nPlaneIndex = 0;

	// add the cell for the slop volume as the first cell
	pVisCell = &m_pVisData->paCells[0];
	pCellInfo = &m_paCellInfos[0];
	pCellInfo->pPD_Cell = NULL;
	pCellInfo->pCell = pVisCell;
	pCellInfo->nIndexInCellArray = 0;
	pCellInfo->nVolIndex = 0;
	pCellInfo->BBox.vMinXYZ = BoundingMin;
	pCellInfo->BBox.vMaxXYZ = BoundingMax;
	Temp = BoundingMax - BoundingMin;
	HalfBox.Set( Temp.x, Temp.y, Temp.z );
	pCellInfo->fBBoxVol = HalfBox.x * HalfBox.y * HalfBox.z;
	pCellInfo->BBoxCenter = WorldBoundingSphere.m_Pos;
	pVisCell->spBoundingWS = WorldBoundingSphere;
	pVisCell->nParentVolumeIdx = 0;
	pVisCell->nPlaneCount = 6;
	pVisCell->paBoundingPlanes = &m_paPlanes[nPlaneIndex];
	nPlaneIndex += 6;	
	HalfBox.Mul( 0.5f );
	// the back plane
	pVisCell->paBoundingPlanes[0].vNormal.Set( CFVec3A::m_UnitAxisZ );
	pVisCell->paBoundingPlanes[0].vPoint.Set( pCellInfo->BBoxCenter );
	pVisCell->paBoundingPlanes[0].vPoint.z -= HalfBox.z;
	// the front plane
	pVisCell->paBoundingPlanes[1].vNormal.ReceiveNegative( CFVec3A::m_UnitAxisZ );
	pVisCell->paBoundingPlanes[1].vPoint.Set( pCellInfo->BBoxCenter );
	pVisCell->paBoundingPlanes[1].vPoint.z += HalfBox.z;
	// the left plane
	pVisCell->paBoundingPlanes[2].vNormal.Set( CFVec3A::m_UnitAxisX );
	pVisCell->paBoundingPlanes[2].vPoint.Set( pCellInfo->BBoxCenter );
	pVisCell->paBoundingPlanes[2].vPoint.x -= HalfBox.x;
	// the right plane
	pVisCell->paBoundingPlanes[3].vNormal.ReceiveNegative( CFVec3A::m_UnitAxisX );
	pVisCell->paBoundingPlanes[3].vPoint.Set( pCellInfo->BBoxCenter );
	pVisCell->paBoundingPlanes[3].vPoint.x += HalfBox.x;
	// the bottom plane
	pVisCell->paBoundingPlanes[4].vNormal.Set( CFVec3A::m_UnitAxisY );
	pVisCell->paBoundingPlanes[4].vPoint.Set( pCellInfo->BBoxCenter );
	pVisCell->paBoundingPlanes[4].vPoint.y -= HalfBox.y;
	// the top plane
	pVisCell->paBoundingPlanes[5].vNormal.ReceiveNegative( CFVec3A::m_UnitAxisY );
	pVisCell->paBoundingPlanes[5].vPoint.Set( pCellInfo->BBoxCenter );
	pVisCell->paBoundingPlanes[5].vPoint.y += HalfBox.y;
	m_pVisData->nCellCount++;

	// add the rest of the cells
	for( i=1; i < m_nStatsVolumes; i++ ) {
		pPD_Vol = (CPortalData_Vol *)m_apPD_Volumes[i-1];

		for( j=0; j < pPD_Vol->m_nNumCells; j++ ) {
			pPD_Cell = (CPortalData_Cell *)pPD_Vol->m_apCells[j];
			
			pVisCell = &m_pVisData->paCells[ m_pVisData->nCellCount ];

			pCellInfo = &m_paCellInfos[ m_pVisData->nCellCount ];

			// fill in the cell info (used later to create the cell node tree)
			pCellInfo->pPD_Cell = pPD_Cell;
			pCellInfo->pCell = pVisCell;
			pCellInfo->nIndexInCellArray = m_pVisData->nCellCount;
			pCellInfo->nVolIndex = i;
			ComputeCellBBoxAndVol( pCellInfo );

			// fill in the actual cell 
			pVisCell->spBoundingWS = pPD_Cell->m_BoundingSphere;
			pVisCell->nParentVolumeIdx = (u16)i;
			FASSERT( pPD_Cell->m_nNumFaces < 0xFF );
			pVisCell->nPlaneCount = (u8)pPD_Cell->m_nNumFaces;
			pVisCell->paBoundingPlanes = &m_paPlanes[nPlaneIndex];
			nPlaneIndex += pPD_Cell->m_nNumFaces;
			for( k=0; k < pPD_Cell->m_nNumFaces; k++ ) {
				pPD_Face = (CPortalData_Face *)pPD_Cell->m_apFaces[k];

				pVisCell->paBoundingPlanes[k].vNormal.Set( -pPD_Face->m_Plane.m_Normal );
				pVisCell->paBoundingPlanes[k].vPoint.Set( pPD_Face->m_Plane.m_Pt );
			}

			// NOTE: we'll set the other fields after the cells and volumes are setup
			
			m_pVisData->nCellCount++;
		}
	}

	if ( pDlg )
	{
		pDlg->PostMessage( CDLG_INCREMENT_SUBOP_PROGRESS, CCompileDlg::m_nCurrentSubOp );
	}

	////////////////////
	// sanity check time
	FASSERT( m_pVisData->nPortalCount == m_nStatsPortals );
	FASSERT( m_pVisData->nVolumeCount == m_nStatsVolumes );
	FASSERT( m_pVisData->nCellCount == m_nStatsCells );

	///////////////////
	// create cell tree
	CreateCellTree();

	//////////////////////////
	// set the active vis data
	fvis_SetActiveVisData( m_pVisData );

	///////////////////////
	// fix the portal array
	for( i=0; i < m_nStatsPortals; i++ ) {
		if( !FindVolumesContainingPortal( &m_pVisData->paPortals[i], sErrorLine	) ) {
			// there was a problem finding the volumes containing the portal
			if( !bErrorsFound ) {
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
				bErrorsFound = TRUE;
			}
			rErrorLog.WriteErrorLine( sErrorLine );
		}
	}

	if( bErrorsFound ) {
		// errors have already been logged, fail
		goto _ERROR;
	}

	pDlg->InfoString( "Built FVis Data (%f seconds)", (f32)(timeGetTime() - nStartTime) / 1000.f );

	/////////////////////////////////////////
	// breakup geometry into different meshes
	m_nStatsNumMeshes = m_nStatsVolumes;
	m_apVolKongMeshs.SetSize( m_nStatsNumMeshes );

	nStartTime = timeGetTime();
	// setup chimps from our master Kong template
	for( i=0; i < m_nStatsNumMeshes; i++ ) {
		m_apVolKongMeshs[i] = &m_paChimps[i];
	}

	u32 nErrorCode;
	nErrorCode = m_TriClipper.ClipAndSeparateKong( this, m_pKongMesh, m_paChimps, m_nStatsNumMeshes, pDlg );
	if( nErrorCode ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		if ( pDlg->m_bAbortCompile )
		{
			rErrorLog.WriteErrorLine( "Compiling aborted by user." );
			return FALSE;
		}

		switch( nErrorCode ) {
		default:
			rErrorLog.WriteErrorLine( "Could not clip and break up the world geometry into volume meshes." );
			break;
		case KTC_EXCEEDED_CELL_CLIP_BUFFER:
		case KTC_EXCEEDED_VERTEX_CLIP_BUFFER:
		case KTC_EXCEEDED_PLANE_CLIP_BUFFER:
			rErrorLog.WriteErrorLine( "Exceeded cell, vertex, or plane clip buffer." );
			break;
		case KTC_UNABLE_TO_CREATE_CHIMPS:
		case KTC_ERROR_CLOSING_CHIMPS:
			rErrorLog.WriteErrorLine( "Problem creating chimp mesh from clipped geo." );
			break;
		}
		goto _ERROR;
	}

	pDlg->InfoString( "Clipped geometry to volume boundaries (%f seconds)", (f32)(timeGetTime() - nStartTime) / 1000.f );

	m_nStatsNonCrossingTris = m_TriClipper.m_Results.m_nTrisInOneCell + m_TriClipper.m_Results.m_nTrisInOneVolume;
	m_nStatsCrossingTris = m_TriClipper.m_Results.m_nTrisClipped;
	m_nStatsFinalTriCount = m_TriClipper.m_Results.m_nTotalWorldTris;
	m_fStatsAvgTrisPerVol = (f32)m_nStatsFinalTriCount / (f32)m_nStatsNumMeshes;
	m_nStatsMaxVolTris = m_TriClipper.m_Results.m_nMostTrisPerVolume;
	m_nStatsVolumeIndexWithMostTris = m_TriClipper.m_Results.m_nVolIdxWithMostTris;

	if( m_nStatsMaxVolTris > _MAX_TRIS_PER_VOLUME ) {
		// too many tris in 1 volume, we need more volumes
		if( m_nStatsVolumeIndexWithMostTris > 0 ) {
			sErrorLine.Format( "There are %d tris in the volume centered at MAX[%f, %f, %f]\n"
							   "\tThe max triangle count per volume is %d, please add more volumes.",
							   m_nStatsMaxVolTris, 
							   m_pVisData->paVolumes[m_nStatsVolumeIndexWithMostTris].spBoundingWS.m_Pos.x, 
							   m_pVisData->paVolumes[m_nStatsVolumeIndexWithMostTris].spBoundingWS.m_Pos.z,
							   m_pVisData->paVolumes[m_nStatsVolumeIndexWithMostTris].spBoundingWS.m_Pos.y,
							   _MAX_TRIS_PER_VOLUME );
		} else {
			sErrorLine.Format( "There are %d tris in the slob bucket.\n"
							   "\tThe max triangle count for the slop bucket is %d, please add more volumes.",
							   m_nStatsMaxVolTris, _MAX_TRIS_PER_VOLUME );
		}
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( sErrorLine );
		goto _ERROR;
	}

	nMeshIndex = 0;
	for( i=0; i < m_nStatsNumMeshes; i++ ) {
		if( m_paChimps[i].m_pKongMesh && m_paChimps[i].m_pKongMesh->nTriangleCount ) {
			m_pVisData->paVolumes[i].pWorldGeo = (CFMeshInst *)nMeshIndex;
			m_apVolKongMeshs[nMeshIndex] = &m_paChimps[i];
			nMeshIndex++;
		}
	}
	if( nMeshIndex != m_nStatsNumMeshes ) {
		m_apVolKongMeshs.RemoveAt( nMeshIndex, m_nStatsNumMeshes - nMeshIndex );
		m_nStatsNumMeshes = nMeshIndex;
	}

	///////////////////////
	// another sanity check
	FASSERT( (u32)(m_pWorkingMem - m_pMemAllocated) <= m_nLocalBytesAllocated );

	/////////////////////////////////////////////////////////
	// Pack the vis data
	if ( !PackVisData() )
	{
		goto _ERROR;
	}

	m_bK2VConverted = TRUE;

	////////////////////////////////////////////////////////
	// we can free some memory to help windows behave better
	if( m_nNumPD_Portals ) {
		CPortalData_Portal *pPortal;
		for( i=0; i < m_nNumPD_Portals; i++ ) {
			pPortal = (CPortalData_Portal *)m_apPD_Portals[i];

			delete pPortal;
		}
		m_apPD_Portals.RemoveAll();
		m_nNumPD_Portals = 0;
	}
	if( m_nNumPD_Volumes ) {
		CPortalData_Vol *pVol;
		for( i=0; i < m_nNumPD_Volumes; i++ ) {
			pVol = (CPortalData_Vol *)m_apPD_Volumes[i];

			delete pVol;
		}
		m_nNumPD_Volumes = 0;
	}
	if( m_paCellInfos ) {
		fang_Free( m_paCellInfos );
		m_paCellInfos = NULL;
	}
	// give windows a slice of time
	Sleep( 0 );

	return TRUE;

_ERROR:
	// an error occurred
	FreeData();
	fvis_SetActiveVisData( NULL );
	return FALSE;
#endif
}


//
//
//
BOOL CKongToVisFile::PackVisData( void ) 
{
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	////////////////////
	// pack our vis data
	u32 nErrorCode = fvis_PackPortalData( m_pVisData, FALSE, &m_pConvertedData, &m_nStatsBytesInFile );
	if( nErrorCode != FVIS_PACK_NO_ERROR ) 
	{
		char szText[256];
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + m_sLastFileLoaded );
		switch (nErrorCode )
		{
			case FVIS_PACK_EXCEEDED_MAX_LIGHTS:
				sprintf( szText, "World has too many lights.  Max lights is %d.\n", FVIS_MAX_LIGHTS );
				break;

			case FVIS_PACK_EXCEEDED_MAX_VOLUMES:
				sprintf( szText, "World has too many volumes.  Max volumes is %d.\n", FVIS_MAX_VOLUME_COUNT );
				break;

			case FVIS_PACK_EXCEEDED_MAX_PORTALS:
				sprintf( szText, "World has too many portals.  Max portals is %d.\n", FVIS_MAX_PORTAL_COUNT );
				break;

			case FVIS_PACK_NO_MEMORY:
				sprintf( szText, "Unable to allocate memory for portal packing.\n" );
				break;

			default:
				sprintf( szText, "Could not pack vis portal data.\n" );
				break;
		}

		rErrorLog.WriteErrorLine( szText );
		return FALSE;
	}

	return TRUE;
}

u32 CKongToVisFile::GetCRC( u32 nCurrentCRC ) 
{
	if( !m_bK2VConverted ) {
		return nCurrentCRC;
	}

	u32 nReturnCRC = fmath_Crc32( nCurrentCRC, (u8 *)m_pConvertedData, m_nStatsBytesInFile );
	
	return nReturnCRC;
}

u32 CKongToVisFile::GetConvertedDataSize() {
	
	if( !m_bK2VConverted ) {
		return 0;
	}
	
	return m_nStatsBytesInFile;
}

BOOL CKongToVisFile::WriteWorldFile( cchar *pszFilename, BOOL bChangeEndian, FILE *pFileStream/*=NULL*/ ) {
	
	if( !m_bK2VConverted ) {
		return FALSE;
	}

	// open our file, if pFileStream == NULL
	BOOL bCloseFile = FALSE;
	if( !pFileStream ) {
		if( !pszFilename ) {
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if( !pFileStream ) {
			return FALSE;
		}
		bCloseFile = TRUE;	
	}

	if ( bChangeEndian )
	{
		fvis_ChangePortalDataEndian( (FVisData_t *)m_pConvertedData );
	}

	////////////////////////////////
	// write out the entire vis file
	fwrite( m_pConvertedData, m_nStatsBytesInFile, 1, pFileStream );
	
	// close file, if need be
	if( bCloseFile ) {
		fclose( pFileStream );
	}
	return TRUE;
}

void CKongToVisFile::FreeData() {
	u32 i;

	if( m_pConvertedData ) {
		fang_Free( m_pConvertedData );
		m_pConvertedData = NULL;
	}
	if( m_pMemAllocated ) {
		fang_Free( m_pMemAllocated );
		m_pMemAllocated = NULL;
	}
	if( m_paPortals ) {
		fang_Free( m_paPortals );
		m_paPortals = NULL;
	}
	if( m_paVolumes ) {
		fang_Free( m_paVolumes );
		m_paVolumes = NULL;
	}
	if( m_paCells ) {
		fang_Free( m_paCells );
		m_paCells = NULL;
	}
	if( m_paPlanes ) {
		fang_Free( m_paPlanes );
		m_paPlanes = NULL;
	}
	if( m_paCellInfos ) {
		fang_Free( m_paCellInfos );
		m_paCellInfos = NULL;
	}
	if( m_paChimps ) {
		delete [] m_paChimps;
		m_paChimps = NULL;
	}
	if( m_nNumPD_Portals ) {
		CPortalData_Portal *pPortal;
		for( i=0; i < m_nNumPD_Portals; i++ ) {
			pPortal = (CPortalData_Portal *)m_apPD_Portals[i];

			delete pPortal;
		}
		m_apPD_Portals.RemoveAll();
		m_nNumPD_Portals = 0;
	}
	if( m_nNumPD_Volumes ) {
		CPortalData_Vol *pVol;
		for( i=0; i < m_nNumPD_Volumes; i++ ) {
			pVol = (CPortalData_Vol *)m_apPD_Volumes[i];

			delete pVol;
		}
		m_nNumPD_Volumes = 0;
	}
	
	m_bK2VConverted = FALSE;
	m_nLocalBytesAllocated = 0;	
	m_nStatsNonCrossingTris = 0;
	m_nStatsCrossingTris = 0;
	m_nStatsFinalTriCount = 0;
	m_nStatsVolumes = 0;
	m_nStatsEmptyVolumes = 0;
	m_nStatsCells = 0;
	m_nStatsCellPlanes = 0;
	m_nStatsPortals = 0;
	m_nStatsAutoPortals = 0;
	m_fStatsAvgTrisPerVol = 0;
	m_nStatsMaxVolTris = 0;
	m_nStatsVolumeIndexWithMostTris = 0;
	m_nStatsBytesInFile = 0;
	m_pVisData = NULL;
	m_apVolKongMeshs.RemoveAll();
	m_nStatsNumMeshes = 0;
	m_nStatsLights = 0;

	// call the base class's implemenation too
	CApeToKongFormat::FreeData();

	m_InitFile.FreeData();
}

BOOL CKongToVisFile::FindVolumesContainingPortal( FVisPortal_t *pPortal, CString &rsErrorLine ) {
	CFVec3A VecForward, PointToTest, FrontCenter, BackCenter;
	FVisVolume_t *pPosVol, *pNegVol;
	CPortalData_Portal *pPD_Portal;
	BOOL bNeed2Volumes;
	
	// grab the ape portal
	pPD_Portal = (CPortalData_Portal *)m_apPD_Portals[pPortal->nPortalID];
	if( !pPD_Portal ) {
		rsErrorLine.Format( "Could not grab the %d'th ape portal.", pPortal->nPortalID );
		return FALSE;	
	}

	// compute our forward test vector
	VecForward.Set( pPortal->vNormal );
	VecForward.Mul( PORTALDATA_SMALL_PORTAL_DISTANCE );
	
	// compute the first positive point
	FrontCenter.v3 = pPD_Portal->m_Centroid + VecForward.v3;

	// find the volume of the postive point
	fworld_GetVolumeContainingPoint( &FrontCenter, &pPosVol );
	if( !pPosVol ) {
		rsErrorLine.Format( "Could not find the 1st volume containing the portal named '%s'.", pPD_Portal->m_pszName );
		return FALSE;
	}
	
	bNeed2Volumes = (pPortal->nFlags & (FVIS_PORTAL_FLAG_MIRROR | FVIS_PORTAL_FLAG_ONE_WAY | FVIS_PORTAL_FLAG_ANTIPORTAL)) ? FALSE : TRUE;
	if( bNeed2Volumes ) {
		// this volume requires both the front and back volumes
	
		// compute the first negative point
		BackCenter.v3 = pPD_Portal->m_Centroid - VecForward.v3;

		// find the volume of the negative point
		fworld_GetVolumeContainingPoint( &BackCenter, &pNegVol );
		if( !pNegVol ) {
			rsErrorLine.Format( "Could not find the 2nd volume containing the portal named '%s'.", pPD_Portal->m_pszName );
			return FALSE;
		}
		
		// make sure that the pos and neg points aren't the same
		if( pPosVol->nVolumeID == pNegVol->nVolumeID ) {
			rsErrorLine.Format( "The portal named '%s' has the front and back both inside volume %d.", pPD_Portal->m_pszName, pPosVol->nVolumeID );
			return FALSE;
		}
	}

	// all pos points are in the same vol, as well as all neg points being in a different vol
	pPortal->anIdxInVolume[0] = pPosVol->nPortalCount;
	pPortal->anAdjacentVolume[0] = (u8)pPosVol->nVolumeID;
	pPosVol->paPortalIndices[ pPosVol->nPortalCount++ ] = pPortal->nPortalID;

	if( bNeed2Volumes ) {
		pPortal->anIdxInVolume[1] = pNegVol->nPortalCount;
		pPortal->anAdjacentVolume[1] = (u8)pNegVol->nVolumeID;
		pNegVol->paPortalIndices[ pNegVol->nPortalCount++ ] = pPortal->nPortalID;
	}

	return TRUE;
}

// pCellInfo->pApeCell must be already setup before calling
void CKongToVisFile::ComputeCellBBoxAndVol( _CellInfo_t *pCellInfo ) {
	u32 i, nNumPts;
	CPortalData_Point *pPD_Point;

	// compute the bbox using the ape cell pts
	nNumPts = pCellInfo->pPD_Cell->m_nNumPts;
	for( i=0; i < nNumPts; i++ ) {
		pPD_Point = (CPortalData_Point *)pCellInfo->pPD_Cell->m_apPts[i];

		if( i > 0 ) {
			pCellInfo->BBox.vMinXYZ.Min( pPD_Point->m_Pos );
			pCellInfo->BBox.vMaxXYZ.Max( pPD_Point->m_Pos );
		} else {
			pCellInfo->BBox.vMinXYZ = pPD_Point->m_Pos;
			pCellInfo->BBox.vMaxXYZ = pPD_Point->m_Pos;
		}
	}
	// compute the volume of the bbox (L * W * H)
	pCellInfo->fBBoxVol = pCellInfo->BBox.vMaxXYZ.x - pCellInfo->BBox.vMinXYZ.x;
	pCellInfo->fBBoxVol *= pCellInfo->BBox.vMaxXYZ.y - pCellInfo->BBox.vMinXYZ.y;
	pCellInfo->fBBoxVol *= pCellInfo->BBox.vMaxXYZ.z - pCellInfo->BBox.vMinXYZ.z;

	// compute the center of the bbox
	pCellInfo->BBoxCenter.ReceiveLerpOf( 0.5f, pCellInfo->BBox.vMinXYZ, pCellInfo->BBox.vMaxXYZ );
}

// rapCellInfo must contain ptrs to _CellInfo_t structs
void CKongToVisFile::ComputeBBoxFromCellInfoBBox( const CPtrArray &rapCellInfo, FVisAABB_t &rBBox, f32 &rfBBoxVol ) {
	u32 i, nCellInfos;
	_CellInfo_t *pCellInfo;

	FASSERT( rapCellInfo.GetSize() >= 1 );
	nCellInfos = (u32)rapCellInfo.GetSize();

	// compute a BBox to contain all CellInfo BBoxes
	pCellInfo = (_CellInfo_t *)rapCellInfo[0];
	rBBox.vMaxXYZ = pCellInfo->BBox.vMaxXYZ;
	rBBox.vMinXYZ = pCellInfo->BBox.vMinXYZ;

	for( i=1; i < nCellInfos; i++ ) {
		pCellInfo = (_CellInfo_t *)rapCellInfo[i];
		rBBox.vMaxXYZ.Max( pCellInfo->BBox.vMaxXYZ );
		rBBox.vMinXYZ.Min( pCellInfo->BBox.vMinXYZ );
	}

	// compute the volume of the bbox (L * W * H)
	rfBBoxVol = rBBox.vMaxXYZ.x - rBBox.vMinXYZ.x;
	rfBBoxVol *= rBBox.vMaxXYZ.y - rBBox.vMinXYZ.y;
	rfBBoxVol *= rBBox.vMaxXYZ.z - rBBox.vMinXYZ.z;
}

// rapCellInfo must contain ptrs to _CellInfo_t structs
void CKongToVisFile::ComputeBBoxFromCellInfoBBoxCenters( const CPtrArray &rapCellInfo, FVisAABB_t &rBBox, f32 &rfBBoxVol ) {
	u32 i, nCellInfos;
	_CellInfo_t *pCellInfo;

	FASSERT( rapCellInfo.GetSize() >= 1 );
	nCellInfos = (u32)rapCellInfo.GetSize();

	// compute a BBox to contain all CellInfo BBox centers
	pCellInfo = (_CellInfo_t *)rapCellInfo[0];
	rBBox.vMaxXYZ = pCellInfo->BBoxCenter;
	rBBox.vMinXYZ = pCellInfo->BBoxCenter;

	for( i=1; i < nCellInfos; i++ ) {
		pCellInfo = (_CellInfo_t *)rapCellInfo[i];
		rBBox.vMaxXYZ.Max( pCellInfo->BBoxCenter );
		rBBox.vMinXYZ.Min( pCellInfo->BBoxCenter );
	}

	// compute the volume of the bbox (L * W * H)
	rfBBoxVol = rBBox.vMaxXYZ.x - rBBox.vMinXYZ.x;
	rfBBoxVol *= rBBox.vMaxXYZ.y - rBBox.vMinXYZ.y;
	rfBBoxVol *= rBBox.vMaxXYZ.z - rBBox.vMinXYZ.z;
}

// rapCellInfo must contain ptrs to _CellInfo_t structs
void CKongToVisFile::Construct2GroupsOfCells( const CPtrArray &rapCellInfo, CPtrArray &rapLeft, CPtrArray &rapRight ) {
	u32 i, nCellInfos, nCutKey;
	_CellInfo_t *pCellInfo;
	FVisAABB_t BBox;
	f32 fBBoxVol, fCutValue;
	CFVec3 Diag;
	
	// compute a bounding box of the bbox centers
	ComputeBBoxFromCellInfoBBoxCenters( rapCellInfo, BBox, fBBoxVol );

	nCellInfos = (u32)rapCellInfo.GetSize();
	
	// determine what dimension to cut on
	Diag = BBox.vMaxXYZ - BBox.vMinXYZ;
	if( (Diag.y >= Diag.x) && (Diag.y >= Diag.z) ) {
		// cut on y
		nCutKey = 1;
	} else if( (Diag.z >= Diag.x) && (Diag.z >= Diag.y) ) {
		// cut on z
		nCutKey = 2;
	} else {
		// cut on x
		nCutKey = 0;
	}
	fCutValue = 0.0f;
	for( i=0; i < nCellInfos; i++ ) {
		pCellInfo = (_CellInfo_t *)rapCellInfo[i];
		fCutValue += pCellInfo->BBoxCenter.a[nCutKey];
	}
	fCutValue /= (f32)nCellInfos;

	// cut the group into 2
	rapLeft.RemoveAll();
	rapRight.RemoveAll();

	for( i=0; i < nCellInfos; i++ ) {
		pCellInfo = (_CellInfo_t *)rapCellInfo[i];

		if( pCellInfo->BBoxCenter.a[nCutKey] <= fCutValue ) {
			rapLeft.Add( pCellInfo );
		} else {
			rapRight.Add( pCellInfo );
		}
	}
}

void CKongToVisFile::CreateCellTree() {
	u32 i, nMaxNodes;
	CPtrArray apCells;

	// create an array of all the cell infos
	apCells.SetSize( m_pVisData->nCellCount );
	for( i=0; i < m_pVisData->nCellCount; i++ ) {
		apCells[i] = &m_paCellInfos[i];
	}

	// create the largest possible node array
	nMaxNodes = (m_pVisData->nCellCount * 2) + 1;
	m_pVisData->CellTree.nFirstNodeIndex = 0;
	m_pVisData->CellTree.nNodeCount = 0;
	m_pVisData->CellTree.paNodes = (FVisCellTreeNode_t *)m_pWorkingMem;
	m_pWorkingMem += (sizeof( FVisCellTreeNode_t ) * nMaxNodes );

	// setup the 1st node
	m_pVisData->CellTree.nFirstNodeIndex = CreateCellNode( apCells, FVIS_INVALID_CELL_ID );

	FASSERT( m_pVisData->CellTree.nNodeCount <= nMaxNodes );
}

// returns the index of the newly created node
u16 CKongToVisFile::CreateCellNode( CPtrArray &rapCells, u16 nParentNodeIndex ) {
	CPtrArray apLeft, apRight;
	FVisAABB_t BBox;
	f32 fBBoxVol;
	FVisCellTreeNode_t *pNode;
	s32 i, nNumCells, nIndexToRemove;
	_CellInfo_t *pCellInfo, *pCellToAdd;
	u16 nNodeIndex;

	// compute a BBox from rapCells
	ComputeBBoxFromCellInfoBBox( rapCells, BBox, fBBoxVol );

	nNumCells = rapCells.GetSize();

	// add a node to our tree
	nNodeIndex = m_pVisData->CellTree.nNodeCount;
	m_pVisData->CellTree.nNodeCount++;
	FASSERT( m_pVisData->CellTree.nNodeCount <= ((m_pVisData->nCellCount * 2) + 1) );
	pNode = &m_pVisData->CellTree.paNodes[nNodeIndex];
	pNode->AABB = BBox;
	pNode->nParentNodeIdx = nParentNodeIndex;
	pNode->nChildNodeIdx1 = FVIS_INVALID_CELL_ID;
	pNode->nChildNodeIdx2 = FVIS_INVALID_CELL_ID;

	// go ahead and add enough space to contain all available cells in this node, just in case
	pNode->nContainedCellIdx = FVIS_INVALID_CELL_ID;
//	pNode->nContainedCellCount = 0;
//	pNode->paIdxToCellsContained = (u16 *)m_pWorkingMem;
//	m_pWorkingMem += (sizeof( u32 ) * nNumCells);
	
	// see if we should add any cells to this node
	i=0;
	pCellToAdd = NULL;
	while( i < nNumCells ) {
		pCellInfo = (_CellInfo_t *)rapCells[i];
		if( pCellInfo->fBBoxVol >= (fBBoxVol * _LARGE_PERCENT_OF_NODE) ) {
			// this cell is large enough to be contained in the node, but only take the largest one
			if( pCellToAdd ) {
				if( pCellInfo->fBBoxVol > pCellToAdd->fBBoxVol ) {
					pCellToAdd = pCellInfo;
					nIndexToRemove = i;
				}
			} else {
				pCellToAdd = pCellInfo;
				nIndexToRemove = i;
			}
		}
		i++;
	}

	if( pCellToAdd ) {
		// remove the single largest cell
//		pNode->paIdxToCellsContained[ pNode->nContainedCellCount++ ] = (u16)pCellToAdd->nIndexInCellArray;
		pNode->nContainedCellIdx = (u16)pCellToAdd->nIndexInCellArray;
		rapCells.RemoveAt( nIndexToRemove, 1 );
		nNumCells--;
	}

	if( nNumCells > 1 ) {
		// we need to break up into left and right groups
		Construct2GroupsOfCells( rapCells, apLeft, apRight );
		if( apLeft.GetSize() >= 1 ) {
			pNode->nChildNodeIdx1 = CreateCellNode( apLeft, nNodeIndex );			
		}
		if( apRight.GetSize() >= 1 ) {
			pNode->nChildNodeIdx2 = CreateCellNode( apRight, nNodeIndex );
		}		
	} else if( nNumCells == 1 ) {
		// there is only 1 more cell, we just need a left side
		apLeft.Copy( rapCells );
		pNode->nChildNodeIdx1 = CreateCellNode( apLeft, nNodeIndex );		
	}
	
	return nNodeIndex;
}

void CKongToVisFile::CreateLightList() {
	
	if( !m_nStatsLights ) {
		return;
	}

	u32 i;
	FLightInit_t *pLight;
	ApeLight_t *pApeLight;
	
	m_pVisData->nLightCount = 0;
	m_pVisData->paLights = (FLightInit_t *)m_pWorkingMem;

	for( i=0; i < GetNumLights(); i++ ) {
		// setup our ptrs
		pApeLight = GetLightInfo( i );
		pLight = (FLightInit_t *)&m_pVisData->paLights[ m_pVisData->nLightCount ];
		fang_MemZero( pLight, sizeof( FLightInit_t ) );
				
		if( pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_ONLY_LIGHT ) {
			// Don't add the lightmap only lights
			continue;	
		}
		
		strncpy( pLight->szName, pApeLight->szLightName, FLIGHT_NAME_LEN );
		pLight->szName[FLIGHT_NAME_LEN] = 0;
		pLight->nType = CConvertApeFile::ConvertLightType( pApeLight->nType );
		// set the light flags
		pLight->nFlags = 0;
		if( (pApeLight->nFlags & APE_LIGHT_FLAG_LIGHT_SELF) == 0 ) {
			pLight->nFlags |= FLIGHT_FLAG_NOLIGHT_TERRAIN;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_DYNAMIC_ONLY ) {
			pLight->nFlags |= FLIGHT_FLAG_DYNAMIC_ONLY;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_PER_PIXEL ) {
			pLight->nFlags |= FLIGHT_FLAG_PER_PIXEL;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_MESH_MUST_BE_PER_PIXEL ) {
			pLight->nFlags |= FLIGHT_FLAG_MESH_MUST_BE_PER_PIXEL;
		}
		
		if( (pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_ONLY_LIGHT) == 0 ) {
			pLight->nFlags |= FLIGHT_FLAG_ENGINE_LIGHT;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_LIGHTMAP_LIGHT ) {
			pLight->nFlags |= FLIGHT_FLAG_LIGHTMAP_LIGHT;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_UNIQUE_LIGHTMAP ) {
			pLight->nFlags |= FLIGHT_FLAG_UNIQUE_LIGHTMAP;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_CORONA ) {
			pLight->nFlags |= FLIGHT_FLAG_CORONA;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_CORONA_PROXFADE ) {
			pLight->nFlags |= FLIGHT_FLAG_CORONA_PROXFADE;	
		}
		
		if( pApeLight->nFlags & APE_LIGHT_FLAG_CAST_SHADOWS ) {
			pLight->nFlags |= FLIGHT_FLAG_CAST_SHADOWS;	
		}
		
		switch( pLight->nType ) {
		case FLIGHT_TYPE_DIR:
			pLight->nFlags |= FLIGHT_FLAG_HASDIR;
			pLight->mtxOrientation = pApeLight->mtxOrientation;
//			flight_BuildLightMatrixFromDirection( &pLight->mtxOrientation_WS, pApeLight->Dir.x, pApeLight->Dir.y, pApeLight->Dir.z );
			break;
		case FLIGHT_TYPE_OMNI:
			pLight->nFlags |= FLIGHT_FLAG_HASPOS;
			pLight->mtxOrientation.Identity();
			break;
		case FLIGHT_TYPE_SPOT:
			pLight->nFlags |= (FLIGHT_FLAG_HASPOS | FLIGHT_FLAG_HASDIR);
			pLight->mtxOrientation = pApeLight->mtxOrientation;
//			flight_BuildLightMatrixFromDirection( &pLight->mtxOrientation_WS, pApeLight->Dir.x, pApeLight->Dir.y, pApeLight->Dir.z );
			break;
		case FLIGHT_TYPE_AMBIENT:
			// don't put ambient lights into the reg list
			if( pApeLight->nFlags & APE_LIGHT_FLAG_DONT_USE_RGB ) {
				m_pVisData->AmbientLightColor.White();
			} else {
				m_pVisData->AmbientLightColor = pApeLight->Color;
			}
			m_pVisData->fAmbientLightIntensity = pApeLight->fIntensity;
			m_nStatsLights--;
			continue;

			break;
		}
		
		pLight->Motif.nMotifIndex = pApeLight->nMotifID;
		if( pApeLight->nFlags & APE_LIGHT_FLAG_DONT_USE_RGB ) {
			// don't use the light's rgb, fill with white
			pLight->Motif.White();			
		} else {
			// copy the light's rgb
			pLight->Motif.ColorRGB = pApeLight->Color;
		}
		pLight->nLightID = pApeLight->nLightID;
		pLight->fCoronaScale = pApeLight->fCoronaScale;
		pLight->Motif.fAlpha = 1.0f;
		pLight->fIntensity = pApeLight->fIntensity;
		pLight->spInfluence = pApeLight->Sphere;
		pLight->fSpotInnerRadians = pApeLight->fSpotInnerAngle;
		pLight->fSpotOuterRadians = pApeLight->fSpotOuterAngle;
		strcpy( pLight->szCoronaTexName, pApeLight->szCoronaTexture );
		strcpy( pLight->szPerPixelTexName, pApeLight->szPerPixelTexture );

		m_pVisData->nLightCount++;
	}

	if( m_pVisData->nLightCount == 0 ) {
		m_pVisData->paLights = NULL;
	} else {
		m_pWorkingMem = (u8 *)&m_pVisData->paLights[ m_pVisData->nLightCount ];
	}
	
	FASSERT( m_pVisData->nLightCount <= m_nStatsLights );		
}






