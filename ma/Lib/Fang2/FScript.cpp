//////////////////////////////////////////////////////////////////////////////////////
// FScript.cpp - Script Classes for Mettle Arms.
//
// Author: Justin Link
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
// 10/24/01 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
//#include <stdlib.h>		// For malloc()
#include <string.h>		// For strcpy()
#include "FFile.h"
#include "FScript.h"
#include "fres.h"
#include "amx.h"

// =============================================================================================================
// Private definitions:
// =============================================================================================================

// =============================================================================================================
// Public variables:
// =============================================================================================================

// =============================================================================================================
// Public functions:
// =============================================================================================================

CFScript::CFScript()
{
	m_bIsLoaded = FALSE;
	m_pProgram = NULL;
	m_szScriptFileName[0] = '\0';
	m_uDataAreaSize = 0;
	m_pDataArea = NULL;
}

// =============================================================================================================

CFScript::~CFScript()
{
	Unload();
}

// =============================================================================================================

BOOL CFScript::LoadFromFile(const char *pszFileName)
{
	Unload();

	if( !pszFileName || strlen(pszFileName) >= sizeof(m_szScriptFileName) ) return FALSE;
	AMX_HEADER hdr;

	FFileHandle hInFile;

	hInFile = ffile_Open(pszFileName, FFILE_OPEN_RONLY);
	if(!FFILE_IS_VALID_HANDLE(hInFile))
	{
		return(FALSE);
	}

	if(ffile_Read(hInFile, sizeof(AMX_HEADER), &hdr) != sizeof(AMX_HEADER))
	{
		DEVPRINTF( "CFScript::LoadFromFile: incomplete AMX header in '%s'.\n", pszFileName );
		ffile_Close(hInFile);
		return(FALSE);
	}

#if FANG_PLATFORM_WIN
	// AMX files use little endian even on the GameCube. Validate before
	// allocation; amx_Init expands compact bytecode in this same buffer.
	// All comparisons are signed so negative fields cannot wrap past the checks.
	const s32 nFileSize = ffile_GetFileSize(hInFile);
	const s32 nHeaderSize = (s32)sizeof(AMX_HEADER);
	const s32 nCellSize = (s32)sizeof(cell);
	const s32 nStubSize = (s32)sizeof(AMX_FUNCSTUB);
	if( hdr.magic != AMX_MAGIC || hdr.defsize != nStubSize ||
		hdr.file_version < MIN_FILE_VERSION || hdr.file_version > CUR_FILE_VERSION ||
		hdr.amx_version < MIN_FILE_VERSION || hdr.amx_version > CUR_FILE_VERSION ||
		(hdr.flags & AMX_FLAG_CHAR16) || hdr.size < nHeaderSize || hdr.size > nFileSize ||
		hdr.cod < nHeaderSize || hdr.cod > hdr.size || hdr.dat < hdr.cod ||
		hdr.hea < hdr.dat || hdr.stp < hdr.hea || hdr.stp - hdr.hea < nCellSize ||
		hdr.size > hdr.stp || (hdr.cod % nCellSize) || (hdr.dat % nCellSize) ||
		(hdr.hea % nCellSize) || (hdr.stp % nCellSize) ||
		(!(hdr.flags & AMX_FLAG_COMPACT) && hdr.size != hdr.hea) ) {
		DEVPRINTF( "CFScript::LoadFromFile: incompatible AMX header in '%s'.\n", pszFileName );
		ffile_Close(hInFile);
		return FALSE;
	}
	const s32 offsets[] = { hdr.publics, hdr.natives, hdr.libraries, hdr.pubvars, hdr.tags };
	const s32 counts[] = { hdr.num_publics, hdr.num_natives, hdr.num_libraries, hdr.num_pubvars, hdr.num_tags };
	for( u32 i=0; i<5; ++i ) {
		if( counts[i] < 0 || offsets[i] < nHeaderSize || offsets[i] > hdr.cod ||
			counts[i] > (hdr.cod - offsets[i]) / nStubSize ) {
			DEVPRINTF( "CFScript::LoadFromFile: invalid AMX symbol table in '%s'.\n", pszFileName );
			ffile_Close(hInFile);
			return FALSE;
		}
	}
#endif

	u32 uBufferSize = hdr.stp;
	m_uDataAreaSize = (s32)(hdr.hea) - (s32)(hdr.dat);
#if FANG_PLATFORM_GC
	uBufferSize = fang_ConvertEndian( (s32)hdr.stp );
	m_uDataAreaSize = fang_ConvertEndian( (s32)(hdr.hea) ) - fang_ConvertEndian( (s32)(hdr.dat) );
#endif
	
	FResFrame_t Frame = fres_GetFrame();
	m_pProgram = fres_Alloc( uBufferSize );
	if(m_pProgram == NULL)
	{
		ffile_Close(hInFile);
		m_uDataAreaSize = 0;
		return(FALSE);
	}

	if( ffile_Seek(hInFile, 0, FFILE_SEEK_SET) < 0 ||
		ffile_Read(hInFile, hdr.size, m_pProgram) != hdr.size )
	{
		DEVPRINTF( "CFScript::LoadFromFile: incomplete program read for '%s'.\n", pszFileName );
		ffile_Close(hInFile);
		fres_ReleaseFrame( Frame );
		m_pProgram = NULL;
		m_uDataAreaSize = 0;
		return(FALSE);
	}
	if(ffile_Close(hInFile) < 0)
	{
		fres_ReleaseFrame( Frame );
		m_pProgram = NULL;
		m_uDataAreaSize = 0;
		return(FALSE);
	};


#if FANG_PLATFORM_GC
	m_pDataArea = (u8 *)(m_pProgram) + fang_ConvertEndian( (s32)(hdr.dat) );
#else
	m_pDataArea = (u8 *)(m_pProgram) + (s32)(hdr.dat);
#endif

	strcpy(m_szScriptFileName, pszFileName);

	m_bIsLoaded = TRUE;
	return(TRUE);
}

// =============================================================================================================

BOOL CFScript::SetDataArea(const void *pDataArea)
{
	fang_MemCopy(m_pDataArea, pDataArea, m_uDataAreaSize);
//	memcpy(m_pDataArea, pDataArea, m_uDataAreaSize);
	return(TRUE);
}

// =============================================================================================================

BOOL CFScript::GetDataArea(void *pDataArea)
{
	fang_MemCopy(pDataArea, m_pDataArea, m_uDataAreaSize);
//	memcpy(pDataArea, m_pDataArea, m_uDataAreaSize);
	return(TRUE);
}

// =============================================================================================================

BOOL CFScript::Unload()
{
	if(m_bIsLoaded)
	{
		FASSERT(m_pProgram != NULL);
//		free(m_pProgram);
//		fang_Free(m_pProgram);
		m_pProgram = NULL;
		m_bIsLoaded = FALSE;
	}
	m_pDataArea = NULL;
	m_uDataAreaSize = 0;
	m_szScriptFileName[0] = 0;

	return(TRUE);
}

// =============================================================================================================