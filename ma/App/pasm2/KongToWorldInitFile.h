//////////////////////////////////////////////////////////////////////////////////////
// KongToWorldInitFile.h - 
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
#ifndef _KONG_TO_WORLD_INIT_FILE_H_
#define _KONG_TO_WORLD_INIT_FILE_H_ 1

#include "fang.h"
#include "fworld.h"
#include "ApeToKongFormat.h"
#include "fdata.h"
#include "PasmDlg.h"


#define INIT_DATA_VERSION			0x00000001
#define KTWI_MAX_APE_OBJECT_LMS		4

//
//
//
struct ApeObjectLMs_t
{
	char szLMTexture[KTWI_MAX_APE_OBJECT_LMS][TEXTURE_NAME_LEN];
	u16  nLMMotif[KTWI_MAX_APE_OBJECT_LMS];
};

//
//
//
struct ApeObjectVertColors_t
{
	u32 nVertexCount;
	u32 *panVertColors;
};

class CKongToWorldInitFile
{
public:
	FData_WorldInitHeader_t *m_pHeader;
	CFWorldShapeInit *m_pShapeArray;
	u32 m_nStatsNumShapes;
	u32 m_nStatsShapeBytes;
	BOOL m_bUsingOldInit;

	CKongToWorldInitFile();
	~CKongToWorldInitFile();

	void FreeData();

	// Data conversion routines
	BOOL ConvertKongFile( CApeToKongFormat *pKong, ApeObjectLMs_t *pApeLMs, ApeObjectVertColors_t *paApeVertColors );
	BOOL ConvertKongFileUsingInit( CCompileDlg *pDlg, CApeToKongFormat *pKong, void *pInitData, u32 nInitDataSize );

	void* GetConvertedData( void ) {return m_pHeader; };
	BOOL WriteToFile( CCompileDlg *pDlg, cchar *pszFilename, TargetPlatform_e nPlatform, FILE *pFileStream=NULL );
	u32  SaveInitDataToVIS( HANDLE hFile );
	u32  LoadInitDataFromVIS( HANDLE hFile, u32 nBytes );
	u32  GetCRC( u32 nCurrentCRC );
	u32  GetConvertedDataSize( void )	
	{
		if( !m_pMemAllocated ) {
			return 0;
		}
		return m_nStatsShapeBytes;
	}

protected:

private:
	BOOL m_bPointersAreOffsets;
	u32 m_nNumBytesAllocated;
	u8 *m_pMemAllocated;
	
	u8 *m_pWorkingMem;
	CPtrArray m_aStrings;// array of string ptrs

	BOOL StoreMeshShapes( CApeToKongFormat *pKong, u32 nNumApeShapes, CFWorldShapeInit *pShapeInit );
	BOOL StoreNonMeshShapes( CApeToKongFormat *pKong, u32 nNumApeShapes, CFWorldShapeInit *pShapeInit );
	BOOL MatchVertexColorsToMesh( CCompileDlg *pDlg, TargetPlatform_e nPlatform );

	cchar *IsStringAlreadyInTable( cchar *pszString );
	void ConvertPointersToOffsets();
	void ConvertOffsetsToPointers();
	void ChangeEndian( void );

	char *GetNextLine( char *pszLine, char *pszEOF );
	FDataGamFile_Header_t *ConvertMaxUserPropsToGameData( void *pUserProps, u32 nNumBytes, BOOL &rPropStringWasEmpty );
};

#endif

