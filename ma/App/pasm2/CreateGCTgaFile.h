//////////////////////////////////////////////////////////////////////////////////////
// CreateGCTgaFile.h - 
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
// 11/16/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CREATE_GC_TGAFILE_H_
#define _CREATE_GC_TGAFILE_H_ 1

#include "fang.h"
#include "GenMipMaps.h"
#include "fdata.h"
#include "ftex.h"

#define GC_TEX_LIB_USE_NVIDIA_LIB		TRUE// set to TRUE to use nvidia libs, FALSE to use s3tc libs

typedef enum {
	SRC_FMT_R8G8B8A8,
	SRC_FMT_R8G8B8,

	SRC_FMT_COUNT
} SrcFormat_e;


typedef struct {
	char *pcConvertedData;
	u32 nNumBytes;
} GCTexConvInfo_t;

class CCreateGCTgaFile
{
public:
	CCreateGCTgaFile();
	~CCreateGCTgaFile();

	typedef enum {
		GC_FORMAT_R8G8B8A8 = 0,			// 32-bit, 8-bit alpha
		GC_FORMAT_R8G8B8X8,				// 32-bit, opaque
		GC_FORMAT_R4G4B4A3,				// 16-bit, 3-bit alpha
		GC_FORMAT_R5G6B5,				// 16-bit, opaque
		GC_FORMAT_S3TC,					// 4-bit compressed RGB, opaque
		GC_FORMAT_S3TCA1,				// 4-bit compressed RGB, 1-bit alpha
		GC_FORMAT_S3TCx2,				// S3TC pair, 4-bit RGB representing color, 4-bit rep. Alpha in green channel
		GC_FORMAT_I8,					// 8-bit intensity map	
		GC_FORMAT_IA8,					// 8-bit intensity + 8-bit alpha map	
		GC_FORMAT_IA4,					// 4-bit intensity + 4-bit alpha map	
		
		GC_FORMAT_COUNT
	} GC_Format_e;

	static CCreateGCTgaFile::GC_Format_e GetFormatFromString( BOOL bCutout, const CString &rsDesiredFormat, u32 nBitsPerPixel );
	BOOL ConvertTgaFile( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither, GC_Format_e nFormat=GC_FORMAT_S3TC  );
	u32  GetDataCRC( void );
	u32  GetSizeOfConvertedFile( void );
	BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	void FreeData( void );

private:

protected:
	FTexInfo_t m_TexInfo;
	GC_Format_e m_nDesiredFormat;
	u32 m_nTotalFileBytes;
	u32 m_nNumMipLevels;
	GCTexConvInfo_t *m_paConvertedDataInfos;
	
	FData_TexFmt_e TranslateFormatCode( GC_Format_e nFormat );
	BOOL ConvertTgaImage( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither );
};

#endif

