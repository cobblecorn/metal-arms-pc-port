//////////////////////////////////////////////////////////////////////////////////////
// GenMipMaps.cpp - 
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
#include "stdafx.h"
#include "GenMipMaps.h"
#include "fmath.h"
#include "ErrorLog.h"

#define _ERROR_HEADING			"TGA MIPMAP GENERATOR "

CGenMipMaps::CGenMipMaps() {
	m_pImage = NULL;
	m_nBytes = 0;
	m_nNumLODs = 0;
	m_nBitsPerPixel = 0;
	m_nHighestLODWidth = 0;
	m_nHighestLODHeight = 0;
	m_bCutOutImage = FALSE;
	m_sLastConvertedFile = "";
}

CGenMipMaps::~CGenMipMaps() {
	UnloadFile();
}

// set nLargestDim = 0 to not limit the largest dimension
BOOL CGenMipMaps::GenerateMipMaps( CTgaFileLoader &TGA,
								   u32 nLargestDim/*=0*/,
								   u32 nSmallestDim/*=1*/,
								   BOOL bMaintainAspect/*=FALSE*/,
								   BOOL bOnlyGenerateTopLOD/*=FALSE*/,
								   BOOL bForceCutout/*=FALSE*/,
								   BOOL bMipToZero/*=TRUE*/ ) {
	u32 i, nW, nH, nBytesPerPixel;

	// unload any previous mipmaps
	UnloadFile();

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// record the w, h of the largest LOD image
	m_nHighestLODWidth = TGA.GetWidth();
	m_nHighestLODHeight = TGA.GetHeight();
	m_nBitsPerPixel = TGA.GetBitsPerPixel();
	
	// do some checking of the dimensions (both W & H must be power of 2)
	if( !fmath_IsPowerOf2( m_nHighestLODWidth, FALSE ) ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + TGA.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Width is not a power of 2 (8,16,32,64,128,256...)" );
		return FALSE;
	}
	if( !fmath_IsPowerOf2( m_nHighestLODHeight, FALSE ) ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + TGA.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Height is not a power of 2 (8,16,32,64,128,256...)" );
		return FALSE;
	}
	if( !fmath_IsPowerOf2( nSmallestDim, FALSE ) ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + TGA.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "The smallest dimension is not a power of 2 (8,16,32,64,128,256...)" );
		return FALSE;
	}
	
	// enforce the max dimensions, if requested
	if( nLargestDim ) {
		if( !fmath_IsPowerOf2( nLargestDim, FALSE ) ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + TGA.m_sLastFileLoaded );
			rErrorLog.WriteErrorLine( "The largest dimension is not a power of 2 (8,16,32,64,128,256...)" );
			return FALSE;
		}
		if( bMaintainAspect ) {
			while( m_nHighestLODWidth > nLargestDim || 
				   m_nHighestLODHeight > nLargestDim ) {
				m_nHighestLODWidth >>= 1;
				m_nHighestLODHeight >>= 1;
			}
#ifdef _MMI_TARGET_PS2	//ARG
			// NS Hardwired PS2 max Y dim to 256 irregardless of nLargestDim

			while( m_nHighestLODHeight > 256 ) {
				m_nHighestLODHeight >>= 1;
			}
#endif
		} else {
			// don't worry about the aspect ratio, clamp dimensions to nLargestDim
			FMATH_CLAMPMAX( m_nHighestLODWidth, nLargestDim );
			FMATH_CLAMPMAX( m_nHighestLODHeight, nLargestDim );
		}
	}
	
	// calculate number of LODs needed
	m_nNumLODs = (bOnlyGenerateTopLOD) ? 1 : ComputeLODCount( TGA, nSmallestDim, bMaintainAspect );
	if( !m_nNumLODs ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + TGA.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "A error occured while computing the number of lods" );
		return FALSE;
	}
	
	// calculate the memory needed
	nW = m_nHighestLODWidth;
	nH = m_nHighestLODHeight;
	nBytesPerPixel = m_nBitsPerPixel >> 3;
	m_nBytes = 0;
	for( i=0; i < m_nNumLODs; i++ ) {
		m_nBytes += nW * nH * nBytesPerPixel;
		if( nW > 1 ) {
			nW >>= 1;
		}
		if( nH > 1 ) {
			nH >>= 1;
		}
	}
	// allocate memory
	m_pImage = new char[m_nBytes];
	if( !m_pImage ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + TGA.m_sLastFileLoaded );
		rErrorLog.WriteErrorLine( "Could not allocate memory for the file image" );
		return FALSE;
	}
	ZeroMemory( m_pImage, m_nBytes );

	m_bCutOutImage = WillTGABeACutout( TGA, bForceCutout );
		
	// create the LODs
	nW = m_nHighestLODWidth;
	nH = m_nHighestLODHeight;
	u8 *pDest = (u8 *)m_pImage;
	for( i=0; i < m_nNumLODs; i++ ) {
		GenerateMipMapLOD( TGA, pDest, nW, nH, bMipToZero );
		pDest += (nW * nH * nBytesPerPixel);
		if( nW > 1 ) {
			nW >>= 1;
		}
		if( nH > 1 ) {
			nH >>= 1;
		}
	}

	m_sLastConvertedFile = TGA.m_sLastFileLoaded;
	
	return TRUE;
}

BOOL CGenMipMaps::WillTGABeACutout( CTgaFileLoader &TGA, BOOL bForceCutout/*=FALSE*/ ) {
	
	if( bForceCutout && TGA.GetBitsPerPixel() != 24) {
		return TRUE;
	}
	return DoesImageContainOnly01AlphaValues( TGA );
}

u32 CGenMipMaps::ComputeLODCount( CTgaFileLoader &TGA,
								  u32 nSmallestDim/*=1*/,
								  BOOL bMaintainAspect/*=FALSE*/ ) {
	u32 nWidth, nHeight, nLodCount, nMinS, nMinT;

	nWidth = m_nHighestLODWidth;
	nHeight = m_nHighestLODHeight;
	nMinS = nSmallestDim;
	nMinT = nSmallestDim;
		
	if( nWidth < nMinS ) {
		return 0;
	}
	if( nHeight < nMinT ) {
		return 0;
	}
	nLodCount = 0;

	if( bMaintainAspect ) {
		while( nWidth>=nMinS && nHeight>=nMinT ) {
			nLodCount++;
			nWidth >>= 1;
			nHeight >>= 1;
		}
	} else {
		while( nWidth>=nMinS || nHeight>=nMinT ) {
			nLodCount++;
			nWidth >>= 1;
			nHeight >>= 1;
		}
	}
#ifdef _MMI_TARGET_PS2
	if (nLodCount > 7)
		nLodCount = 7;	// NS only allow a maximum total of 7 textures on PS2
#endif
	return nLodCount;
}

void CGenMipMaps::UnloadFile() {

	if( m_pImage ) {
		delete[] (char *)m_pImage;
		m_pImage = NULL;
	}
	m_sLastConvertedFile = "";
}

void *CGenMipMaps::GetImage( u32 nLodIndex, u32 &rnHeight, u32 &rnWidth ) {
	u32 i, nBytesPerPixel;
	
	if( !m_pImage || nLodIndex >= m_nNumLODs ) {
		return NULL;
	}

	rnWidth = m_nHighestLODWidth;
	rnHeight = m_nHighestLODHeight;
	nBytesPerPixel = (m_nBitsPerPixel >> 3);
	u8 *pDest = (u8 *)m_pImage;
	for( i=0; i < nLodIndex; i++ ) {
		pDest += (rnWidth * rnHeight * nBytesPerPixel);
		if( rnWidth > 1 ) {
			rnWidth >>= 1;
		}
		if( rnHeight > 1 ) {
			rnHeight >>= 1;
		}
	}
	return pDest;
}

// takes whatever value is stored and divides it by 2
void CGenMipMaps::CutAllPixelValuesByHalf() {

	if( !m_pImage ) {
		return;
	}
	u32 i;
	u8 *pPixel = (u8 *)m_pImage;
	for( i=0; i < m_nBytes; i++ ) {
		pPixel[i] >>= 1;
	}
}

// will bias each lod a bit more, till the lowest lod is made up completely of nBias values
// Note: the topmost Lod will not be touched
void CGenMipMaps::BiasLowerLodsToValue( u8 nBias ) {
	
	if( !m_pImage ) {
		return;
	}
	u32 i, nW, nH, j;
	u8 *pnPixels;
	f32 fPercentTowardBias, fDelta, fValue, fBias = (f32)nBias;
	u32 nBytes;
	for( i=1; i < m_nNumLODs; i++ ) {
		pnPixels = (u8 *)GetImage( i, nH, nW );
		fPercentTowardBias = 2.0f * (f32)i / (f32)(m_nNumLODs - 1);
		
		nBytes = (m_nBitsPerPixel >> 3) * nW * nH;
		for( j=0; j < nBytes; j++ ) {
			fValue = (f32)pnPixels[j];
			fDelta = fBias - fValue;
			fValue += (fPercentTowardBias * fDelta);

			if (fValue > 255.0f)
			{
				fValue = 255.0f;
			}

			pnPixels[j] = (u8)fValue;
		}
	}
}

void CGenMipMaps::GenerateMipMapLOD( CTgaFileLoader &SourceTga, u8 *pDest, u32 nWidth, u32 nHeight, BOOL bMipToZero ) {
	u32 nDeltaWidth, nDeltaHeight, nDestRow, nDestCol, nSourceDeltaRow, x, y, nSrcW, nSrcH;
	float fOoTexelsPerBlock;
	f32 afColors[4];
	u8 *pSrc, *pSample;
	BOOL b32Bit;
	u8 cMipCutoutTestValue;
	
	// grab the source's width/height
	nSrcW = SourceTga.GetWidth();
	nSrcH = SourceTga.GetHeight();

	FASSERT( fmath_IsPowerOf2( nWidth, FALSE ) );
	FASSERT( fmath_IsPowerOf2( nHeight, FALSE ) );
	FASSERT( nSrcW >= nWidth );
	FASSERT( nSrcH >= nHeight );

	nDeltaWidth = nSrcW / nWidth;
	nDeltaHeight = nSrcH / nHeight;
	fOoTexelsPerBlock = 1.0f / (float)(nDeltaWidth * nDeltaHeight);

	b32Bit = (m_nBitsPerPixel == 32);
	pSrc = (u8 *)SourceTga.GetPixelData();
	nSourceDeltaRow = nSrcW - nDeltaWidth;

	cMipCutoutTestValue = (bMipToZero) ? 192 : 64;

	for( nDestRow=0; nDestRow < nHeight; nDestRow++ ) {
		for( nDestCol=0; nDestCol < nWidth; nDestCol++ ) {
			// compute our new pixel from the source
			afColors[0] = 0.0f;
			afColors[1] = 0.0f;
			afColors[2] = 0.0f;
			afColors[3] = 0.0f;

			pSample = pSrc;

			for( y=0; y < nDeltaHeight; y++ ) {
				for( x=0; x < nDeltaWidth; x++ ) {
					afColors[0] += (f32)pSample[0];
					afColors[1] += (f32)pSample[1];
					afColors[2] += (f32)pSample[2];
					if( b32Bit ) {
						afColors[3] += (f32)pSample[3];
						pSample += 4;
					} else {
						pSample += 3;
					}
				}
				if( b32Bit ) {
					pSample += (nSourceDeltaRow * 4);
				} else {
					pSample += (nSourceDeltaRow * 3);
				}
			}
			// write our computed pixel into the dest
			pDest[0] = (u8)(afColors[0] * fOoTexelsPerBlock);
			pDest[1] = (u8)(afColors[1] * fOoTexelsPerBlock);
			pDest[2] = (u8)(afColors[2] * fOoTexelsPerBlock);
			if( b32Bit ) {
				pDest[3] = (u8)(afColors[3] * fOoTexelsPerBlock);
				if( m_bCutOutImage ) {
					// this is a cut, preserve the 0,1 ness of the original image
					pDest[3] = ( pDest[3] < cMipCutoutTestValue ) ? 0 : 255;					
				}
				pDest += 4;
				pSrc += (nDeltaWidth * 4);
			} else {
				pDest += 3;
				pSrc += (nDeltaWidth * 3);
			}						
		}
		if( b32Bit ) {
			pSrc += ( (nDeltaHeight-1) * nSrcW * 4 );
		} else {
			pSrc += ( (nDeltaHeight-1) * nSrcW * 3 );
		}
	}
}

BOOL CGenMipMaps::DoesImageContainOnly01AlphaValues( CTgaFileLoader &SourceTga ) {
	
	if( SourceTga.GetBitsPerPixel() == 24 ) {
		// no alpha channel
		return FALSE;
	}
	u8 *pPixels = (u8 *)SourceTga.GetPixelData();
	pPixels+=3;// alpha
	u32 nPixels = SourceTga.GetNumBytes() >> 2;
	for( u32 i=0; i < nPixels; i++ ) {
		if( *pPixels != 0 && *pPixels != 255 ) {
			// there are pixels that are not 0 or 255
			return FALSE;
		}
		pPixels+=4;
	}
	return TRUE;
}


