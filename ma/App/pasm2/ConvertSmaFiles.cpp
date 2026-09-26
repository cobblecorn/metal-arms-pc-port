//////////////////////////////////////////////////////////////////////////////////////
// ConvertSmaFiles.cpp - 
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
// 10/25/01 Link     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "ConvertSmaFiles.h"
#include "ErrorLog.h"
#include "fmath.h"
#include "fdata.h"
#include "fclib.h"
#include "sc.h"
#include <sys/stat.h>
#include <direct.h>

#define _ERROR_HEADING				".SMA FILE COMPILER " 

CConvertSmaFile::CConvertSmaFile() {
	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;
}

CConvertSmaFile::~CConvertSmaFile() {
	FreeData();
}

BOOL CConvertSmaFile::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ ) {
	CString sFilepath;
	
	FreeData();

	if( !pFileInfo ) {
		return FALSE;
	}

	// Cache the current directory, and set a new one.
	char strWorkingDir[MAX_PATH];
	getcwd(strWorkingDir, MAX_PATH);
	chdir(pFileInfo->GetFileRoot());

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// gather some info about the file
	sFilepath = pFileInfo->GetFilePath();

	char szFoo[256], szStackSize[8];
	fclib_strcpy( szFoo, sFilepath );
	fclib_strcpy( szStackSize, "-S72" ); // Set default stack size to 72 cells (288 bytes) Small requires > 64 and a cell is 4 bytes, so 72 cells = 288 bytes which is 32 byte aligned size
	char *pTemp[3];
	pTemp[0] = NULL;
	pTemp[1] = szFoo;
	pTemp[2] = szStackSize;

	// sc_compile returns 0 for no problems, 1 for warning(s), and 2 for error(s).
	if(sc_compile( 3, pTemp ) == 2)
	{
		rErrorLog.WriteErrorHeader(_ERROR_HEADING + sFilepath);
		rErrorLog.WriteErrorLine("Error during script compilation.");
		chdir(strWorkingDir);
		return(FALSE);
	}

	u32 uStrLen = sFilepath.GetLength();
	szFoo[uStrLen - 3] = 'a';
	szFoo[uStrLen - 2] = 'm';
	szFoo[uStrLen - 1] = 'x';

	FILE *fInFile;
	fInFile = fopen(szFoo, "rb");
	if(fInFile == NULL)
	{
		rErrorLog.WriteErrorHeader(_ERROR_HEADING + sFilepath);
		rErrorLog.WriteErrorLine("Could not open output file of script compiler.");
		chdir(strWorkingDir);
		return(FALSE);
	}

	struct _stat oBuf;
	if(_stat(szFoo, &oBuf) != 0)
	{
		rErrorLog.WriteErrorHeader(_ERROR_HEADING + sFilepath);
		rErrorLog.WriteErrorLine("Error retrieving statistics for output file of script compiler.");
		chdir(strWorkingDir);
		return(FALSE);
	}
	m_nCompiledBytes = oBuf.st_size;
	m_pCompiledFile = new u8[m_nCompiledBytes];
	if(m_pCompiledFile == NULL)
	{
		rErrorLog.WriteErrorHeader(_ERROR_HEADING + sFilepath);
		rErrorLog.WriteErrorLine("Error allocating memory.");
		chdir(strWorkingDir);
		return(FALSE);
	}
	if(fread(m_pCompiledFile, 1, m_nCompiledBytes, fInFile) != m_nCompiledBytes)
	{
		FreeData();
		rErrorLog.WriteErrorHeader(_ERROR_HEADING + sFilepath);
		rErrorLog.WriteErrorLine("Error reading output file of script compiler.");
		chdir(strWorkingDir);
		return(FALSE);
	}
	fclose(fInFile);
	if(remove(szFoo) == -1)
	{
		FreeData();
		rErrorLog.WriteErrorHeader(_ERROR_HEADING + sFilepath);
		rErrorLog.WriteErrorLine("Error deleting output file of script compiler.");
		chdir(strWorkingDir);
		return(FALSE);
	}

	chdir(strWorkingDir);

	if( bConvertToBigEndian ) {

	}

	return TRUE;
}

u32 CConvertSmaFile::GetDataCRC() {
	
	if( !m_pCompiledFile ) {
		return 0;
	}

	u32 nReturnCRC = fmath_Crc32( 0, (u8 *)m_pCompiledFile, m_nCompiledBytes );

	return nReturnCRC;
}

u32 CConvertSmaFile::GetSizeOfConvertedFile() {
	
	if( !m_pCompiledFile ) {
		return 0;
	}
	return m_nCompiledBytes;
}

BOOL CConvertSmaFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

	if( !m_pCompiledFile ) {
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

void CConvertSmaFile::FreeData() {
	if(m_pCompiledFile != NULL)
		delete [] m_pCompiledFile;
	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;
}
