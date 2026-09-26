//////////////////////////////////////////////////////////////////////////////////////
// TGAFileLoader.h - 
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
// 11/07/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _TGA_FILE_LOADER_H_
#define _TGA_FILE_LOADER_H_ 1

#include "fang.h"


class CTgaFileLoader
{
public:
	CTgaFileLoader();
	~CTgaFileLoader();

	BOOL LoadTgaFile( cchar *pszFilename, BOOL bInvertAlpha, BOOL bConvertTo24BitIntensityMap=FALSE );
	void UnloadFile();
	BOOL Convert32bitAlphaImageIntoUnitFloat();
	
	// helper functions to return loaded data
	void *GetPixelData()	{ return m_pImage; }
	u32 GetNumBytes()		{ return m_nBytes; }
	u32 GetWidth()			{ return m_nWidth; }
	u32 GetHeight()			{ return m_nHeight; }
	u32 GetBitsPerPixel()	{ return m_nBitsPerPixel; }
	u32 GetLargestDim();
	u32 GetSmallestDim();

	CString m_sLastFileLoaded;
private:
	BOOL ReadTGAHeader( FILE *pFile );

protected:
	void *m_pImage;
	void *m_pMemAllocated;
	u32 m_nBytes;
	u32 m_nWidth;
	u32 m_nHeight;
	u32 m_nBitsPerPixel;
	u32 m_nNumCharsInIdField;
	u32 m_nPaletteType;
	u32 m_nImageTypeCode;
	u32 m_nPaletteOrigin;
	u32 m_nPaletteLength;
	u32 m_nPaletteEntry;
	u32 m_nImageXOrigin;
	u32 m_nImageYOrigin;
	u32 m_nImageDescriptor;
	u32 m_nAlphaBits;
	BOOL m_bInvertedImage;

	void FlipImage( u32 nBytesPerTexel );
};


#endif

