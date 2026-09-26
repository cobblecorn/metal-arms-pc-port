//////////////////////////////////////////////////////////////////////////////////////
// ConvertFprFiles.cpp - 
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
// 04/03/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "ConvertFprFiles.h"
#include "ErrorLog.h"
#include "fdata.h"
#include "fparticle.h"

//====================
// private definitions

#define _ERROR_HEADING				".FPR FILE COMPILER "

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions

CConvertFprFile::CConvertFprFile()  {
	m_nBytesAllocated = 0;
	m_pMemAllocated = NULL;
	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;
}

CConvertFprFile::~CConvertFprFile() {
	FreeData();
}

BOOL CConvertFprFile::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ ) {
	CString sFilepath;
	u32 nNumBytesInFile;
	FILE *pFile;
	FParticleDef_t *pDef;
		
	FreeData();

	if( !pFileInfo ) {
		return FALSE;
	}

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// gather some info about the file
	sFilepath = pFileInfo->GetFilePath();
	nNumBytesInFile = pFileInfo->GetLength();
	
	if( nNumBytesInFile != sizeof( FParticleDef_t ) ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "The size of the file is not the size of the structs, perhaps this file is a wrong version" );
		return FALSE;
	}

	// allocate memory
	m_nBytesAllocated = nNumBytesInFile;
	m_pMemAllocated = (u8 *)fang_MallocAndZero( m_nBytesAllocated, 16 );
	if( !m_pMemAllocated ) {
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	// read the entire file into memory
	pFile = fopen( sFilepath, "rb" );
	if( !pFile ) {
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not open file for reading" );
		return FALSE;
	}
	if( fread( m_pMemAllocated, 1, nNumBytesInFile, pFile ) != nNumBytesInFile ) {
		fclose( pFile );
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Trouble reading the file" );
		return FALSE;
	}
	fclose( pFile );

	// fix up our pointers
	m_nCompiledBytes = nNumBytesInFile;
	m_pCompiledFile = m_pMemAllocated;
	pDef = (FParticleDef_t *)m_pCompiledFile;
		
	if( pDef->nVersion != FPARTICLE_FILE_VERSION ) {
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "The file is the wrong version" );
		return FALSE;
	}

	if( bConvertToBigEndian ) {
		pDef->ChangeEndian();
	}

	return TRUE;
}

u32 CConvertFprFile::GetDataCRC() {

	if( !m_pCompiledFile ) {
		return FALSE;
	}

	u32 nReturnCRC = fmath_Crc32( 0, (u8 *)m_pCompiledFile, m_nCompiledBytes );

	return nReturnCRC;
}

u32 CConvertFprFile::GetSizeOfConvertedFile() {

	if( !m_pMemAllocated ) {
		return 0;
	}
	return m_nCompiledBytes;
}

BOOL CConvertFprFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

	if( !m_pMemAllocated ) {
		return FALSE;
	}
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
	// write out our file
	fwrite( m_pCompiledFile, m_nCompiledBytes, 1, pFileStream );
	// close our file
	if( bCloseFile ) {
		fclose( pFileStream );
	}

	return TRUE;
}

void CConvertFprFile::FreeData() {

	if( m_pMemAllocated ) {
		fang_Free( m_pMemAllocated );
		m_pMemAllocated = NULL;
	}
	m_nBytesAllocated = 0;
	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;	
}

//==================
// private functions

