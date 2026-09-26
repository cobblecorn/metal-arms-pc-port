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

	AMX_HEADER hdr;

	FFileHandle hInFile;

	hInFile = ffile_Open(pszFileName, FFILE_OPEN_RONLY);
	if(!FFILE_IS_VALID_HANDLE(hInFile))
	{
		return(FALSE);
	}

	if(ffile_Read(hInFile, sizeof(AMX_HEADER), &hdr) < 0)
	{
		return(FALSE);
	}

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
		return(FALSE);
	}

	ffile_Seek(hInFile, 0, FFILE_SEEK_SET);
	
	if(ffile_Read(hInFile, hdr.size, m_pProgram) < 0)
	{
		fres_ReleaseFrame( Frame );
		return(FALSE);
	}
	if(ffile_Close(hInFile) < 0)
	{
		fres_ReleaseFrame( Frame );
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

	return(TRUE);
}

// =============================================================================================================