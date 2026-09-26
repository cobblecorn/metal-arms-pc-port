//////////////////////////////////////////////////////////////////////////////////////
// MusyXData.cpp - 
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
// 06/14/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "MusyXData.h"
#include "fdata.h"

CMusyXData::CMusyXData() {
}

CMusyXData::~CMusyXData() {
}

BOOL CMusyXData::CreateMusyXDataFile( CFileInfo *pProjFile,
									  CFileInfo *pSDirFile,
									  CFileInfo *pPoolFile,
									  CString &rsOutputFilename ) {
	u32 nBytesNeeded;
	u8 *pMemAllocated, *pMem;
	FGCData_MusyXDataHeader_t *pHeader;
	FILE *pFile;

	// allocate memory
	nBytesNeeded = sizeof( FGCData_MusyXDataHeader_t ) +
				   pProjFile->GetLength() + 
				   pPoolFile->GetLength() + 
				   pSDirFile->GetLength();
	pMemAllocated = (u8 *)malloc( nBytesNeeded );
	if( !pMemAllocated ) {
		return FALSE;
	}
	
	// zero out memory
	fang_MemZero( pMemAllocated, nBytesNeeded );

	// fixup the header
	pHeader = (FGCData_MusyXDataHeader_t *)pMemAllocated;
	pMem = (u8 *)&pHeader[1];
	pHeader->nProjFileBytes = pProjFile->GetLength();
	pHeader->nProjFileOffset = sizeof( FGCData_MusyXDataHeader_t );

	pHeader->nPoolFileBytes = pPoolFile->GetLength();
	pHeader->nPoolFileOffset = pHeader->nProjFileOffset + pHeader->nProjFileBytes;

	pHeader->nSDirFileBytes = pSDirFile->GetLength();
	pHeader->nSDirFileOffset = pHeader->nPoolFileOffset + pHeader->nPoolFileBytes;

	// read in the .proj file
	FASSERT( ( (u32)pMem - (u32)pMemAllocated ) == pHeader->nProjFileOffset );
	pFile = fopen( pProjFile->GetFilePath(), "rb" );
	if( !pFile ) {
		free( pMemAllocated );
		return FALSE;
	}
	if( fread( pMem, pHeader->nProjFileBytes, 1, pFile ) != 1 ) {
		free( pMemAllocated );
		return FALSE;
	}
	fclose( pFile );
	pMem += pHeader->nProjFileBytes;

	// read in the .pool file
	FASSERT( ( (u32)pMem - (u32)pMemAllocated ) == pHeader->nPoolFileOffset );
	pFile = fopen( pPoolFile->GetFilePath(), "rb" );
	if( !pFile ) {
		free( pMemAllocated );
		return FALSE;
	}
	if( fread( pMem, pHeader->nPoolFileBytes, 1, pFile ) != 1 ) {
		free( pMemAllocated );
		return FALSE;
	}
	fclose( pFile );
	pMem += pHeader->nPoolFileBytes;

	// read in the .sdir file
	FASSERT( ( (u32)pMem - (u32)pMemAllocated ) == pHeader->nSDirFileOffset );
	pFile = fopen( pSDirFile->GetFilePath(), "rb" );
	if( !pFile ) {
		free( pMemAllocated );
		return FALSE;
	}
	if( fread( pMem, pHeader->nSDirFileBytes, 1, pFile ) != 1 ) {
		free( pMemAllocated );
		return FALSE;
	}
	fclose( pFile );
	pMem += pHeader->nSDirFileBytes;
	FASSERT( ( (u32)pMem - (u32)pMemAllocated ) == nBytesNeeded );

	// endianize the header
	pHeader->ChangeEndian();

	// now write out the entire file
	pFile = _tfopen( rsOutputFilename, _T("wb") );
	if( !pFile ) {
		free( pMemAllocated );
		return FALSE;
	}
	fwrite( pMemAllocated, nBytesNeeded, 1, pFile );
	fclose( pFile );
	
	free( pMemAllocated );

	return TRUE;
}


	

	
