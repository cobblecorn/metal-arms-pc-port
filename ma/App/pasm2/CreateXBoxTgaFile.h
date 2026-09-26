//////////////////////////////////////////////////////////////////////////////////////
// CreateXBoxTgaFile.h - 
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
#ifndef _CREATE_XBOX_TGAFILE_H_
#define _CREATE_XBOX_TGAFILE_H_ 1

#include "fang.h"
#include "GenMipMaps.h"
#include "ftex.h"

class CCreateXBoxTgaFile
{
public:
	CCreateXBoxTgaFile();
	~CCreateXBoxTgaFile();

	typedef enum {
		XB_FORMAT_DXT1 = 0,
		XB_FORMAT_DXT3,
		XB_FORMAT_DXT5,
		XB_FORMAT_RAW32,// covers argb8888 & rgb888
		XB_FORMAT_RAW16,// covers argb4444, argb1555, & rgb565
		
		XB_FORMAT_COUNT
	} Xb_Format_e;

	static CCreateXBoxTgaFile::Xb_Format_e GetFormatFromString( BOOL bCutoutOr24Bit, const CString &rsDesiredFormat );
	BOOL ConvertTgaFile( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither, Xb_Format_e nFormat=XB_FORMAT_DXT3  );
	u32  GetDataCRC( void );
	u32  GetSizeOfConvertedFile( void );
	BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL, BOOL bOutputDebugPics=FALSE );
	void FreeData( void );

private:

protected:
	u32 m_nBytesAllocated;
	u8 *m_pMemAllocated;
	u8 *m_pActualMemAllocated;
	u32 m_nHeight;
	u32 m_nWidth;
					
	FTexInfo_t m_TexInfo;

	Xb_Format_e m_nDesiredFormat;
	
	BOOL GenerateRaw24or32File( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither );
	BOOL GenerateRaw16File( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither );
	BOOL GenerateDxtFile( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither );
	void WriteDDSFile( FILE *pFile, u8 *pImage, u32 nW, u32 nH );
};

#endif

