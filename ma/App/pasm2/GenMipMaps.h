//////////////////////////////////////////////////////////////////////////////////////
// GenMipMaps.h - 
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
#ifndef _GEN_MIP_MAPS_H_
#define _GEN_MIP_MAPS_H_ 1

#include "fang.h"
#include "TGAFileLoader.h"

class CGenMipMaps
{
public:
	CGenMipMaps();
	~CGenMipMaps();

	BOOL GenerateMipMaps( CTgaFileLoader &TGA, u32 nLargestDim=0, u32 nSmallestDim=1, 
						  BOOL bMaintainAspect=FALSE, BOOL bOnlyGenerateTopLOD=FALSE,
						  BOOL bForceCutout=FALSE, BOOL bMipToZero=TRUE );
	u32 ComputeLODCount( CTgaFileLoader &TGA, u32 nSmallestDim=1, BOOL bMaintainAspect=FALSE );
	void UnloadFile();
	void *GetImage( u32 nLodIndex, u32 &rnHeight, u32 &rnWidth );
	static BOOL WillTGABeACutout( CTgaFileLoader &TGA, BOOL bForceCutout=FALSE );

	void BiasLowerLodsToValue( u8 nBias );
	void CutAllPixelValuesByHalf();
	

	void *GetImage()		{ return m_pImage; }
	u32 GetNumLODs()		{ return m_nNumLODs; }
	u32 GetNumBytes()		{ return m_nBytes; }
	u32 GetNumPixels()		{ return (m_nBytes / (m_nBitsPerPixel >> 3)); }
	u32 GetBitsPerPixel()	{ return m_nBitsPerPixel; }
	u32 GetBytesPerPixel()	{ return (m_nBitsPerPixel >> 3); }
	u32 GetHighLODWidth()	{ return m_nHighestLODWidth; }
	u32 GetHighLODHeight()	{ return m_nHighestLODHeight; }
	BOOL IsImageACutout()	{ return m_bCutOutImage; }

	CString m_sLastConvertedFile;

private:

protected:
	char *m_pImage;	// if 24 bit then the bytes are BGR, if 32 bit then the bytes are BGRA
	u32 m_nBytes;	// the total number of bytes that m_pImage points to
	u32 m_nNumLODs;	// how many LOD levels are there?
	u32 m_nBitsPerPixel;
	u32 m_nHighestLODWidth;
	u32 m_nHighestLODHeight;
	BOOL m_bCutOutImage;

	void GenerateMipMapLOD( CTgaFileLoader &SourceTga, u8 *pDest, u32 nWidth, u32 nHeight, BOOL bMipToZero );
	static BOOL DoesImageContainOnly01AlphaValues( CTgaFileLoader &SourceTga );
};



#endif

