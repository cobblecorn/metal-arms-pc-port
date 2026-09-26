//////////////////////////////////////////////////////////////////////////////////////
// CreateXBoxTgaFile.cpp - 
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
#include "stdafx.h"
#include "fang.h"
#include "CreateXBoxTgaFile.h"
#include "fdata.h"
#include "ErrorLog.h"
#include "dxtlib.h"
#include "d3d8-xbox.h"
#include "ddraw.h"
#include "dds.h"
#include "xgraphics.h"

#define _ERROR_HEADING								"TGA->XBOX TEXTURE MAP FILE COMPILER "

//// Default font data.
//
// Note: copy fdx8fonttex.h (generated in the current directory) to fang2\dx.
#define _OUTPUT_DEFAULT_FONTTEX_CPP_DATA_TO_CUR_DIR		FALSE
//
////

// private prototypes
static HRESULT _DxtMipCallBack( void *pData, int nMipLevel, DWORD nSizeInBytes );

// private vars (used during dxt creation)
static u8 *_pDxtMemory;
static u32 _nDxtByteOffset;
static u32 _nTotalDxtMemory;

typedef struct DDS_HEADER _DDSHeader_t;

CCreateXBoxTgaFile::CCreateXBoxTgaFile() {
	m_nBytesAllocated = 0;
	m_pActualMemAllocated = NULL;
	m_pMemAllocated = NULL;
	ZeroMemory( &m_TexInfo, sizeof( FTexInfo_t ) );
	m_nHeight = 0;
	m_nWidth = 0;
	m_nDesiredFormat = XB_FORMAT_DXT3;	
}

CCreateXBoxTgaFile::~CCreateXBoxTgaFile() {
	FreeData();
}

CCreateXBoxTgaFile::Xb_Format_e CCreateXBoxTgaFile::GetFormatFromString( BOOL bCutoutOr24Bit, const CString &rsDesiredFormat ) {

	if( rsDesiredFormat.CompareNoCase( "auto" ) == 0 ) {
		// if 24 bit or cutout, then dxt1, otherwise dxt3
		if( bCutoutOr24Bit ) {
			return XB_FORMAT_DXT1;
		} else {
			return XB_FORMAT_DXT3;
		}
	} else if( rsDesiredFormat.CompareNoCase( "dxt1" ) == 0 ) {
		return XB_FORMAT_DXT1;
	} else if( rsDesiredFormat.CompareNoCase( "dxt3" ) == 0 ) {
		return XB_FORMAT_DXT3;
	} else if( rsDesiredFormat.CompareNoCase( "dxt5" ) == 0 ) {
		return XB_FORMAT_DXT5;
	} else if( rsDesiredFormat.CompareNoCase( "argb8888" ) == 0 ) {
		return XB_FORMAT_RAW32;
	} else if( rsDesiredFormat.CompareNoCase( "rgb888" ) == 0 ) {
		return XB_FORMAT_RAW32;
	} else if( rsDesiredFormat.CompareNoCase( "argb4444" ) == 0 ) {
		return XB_FORMAT_RAW16;
	} else if( rsDesiredFormat.CompareNoCase( "argb1555" ) == 0 ) {
		return XB_FORMAT_RAW16;
	} else if( rsDesiredFormat.CompareNoCase( "argb565" ) == 0 ) {
		return XB_FORMAT_RAW16;
	}
	
	return XB_FORMAT_DXT3;
}

BOOL CCreateXBoxTgaFile::ConvertTgaFile( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither, Xb_Format_e nFormat/*=XB_FORMAT_DXT3*/ ) {
	BOOL bReturn;

	FreeData();

	if( nFormat >= XB_FORMAT_COUNT ) {
		nFormat = XB_FORMAT_DXT3;
	}
	m_nDesiredFormat = nFormat;

	switch( m_nDesiredFormat ) {
	default:
	case XB_FORMAT_DXT1:
	case XB_FORMAT_DXT3:
	case XB_FORMAT_DXT5:
		bReturn = GenerateDxtFile( MipMap, pszTextureName, bDither );
		break;
	case XB_FORMAT_RAW32:
		bReturn = GenerateRaw24or32File( MipMap, pszTextureName, bDither );
		break;
	case XB_FORMAT_RAW16:
		bReturn = GenerateRaw16File( MipMap, pszTextureName, bDither );
		break;
	}
	
	return bReturn;	
}

u32 CCreateXBoxTgaFile::GetDataCRC( void ) 
{
	if ( !m_pMemAllocated ) 
	{
		return 0;
	}

	u32 nReturnCRC = fmath_Crc32( 0, (u8 *)&m_TexInfo, sizeof( FTexInfo_t ) );
	nReturnCRC = fmath_Crc32( nReturnCRC, (u8 *)m_pMemAllocated, m_nBytesAllocated );

	return nReturnCRC;
}

u32 CCreateXBoxTgaFile::GetSizeOfConvertedFile( void ) 
{

	if( !m_pMemAllocated ) {
		return 0;
	}
	u32 nSize;
	nSize = FMATH_BYTE_ALIGN_UP( sizeof( FTexInfo_t ), 128 );
	nSize += m_nBytesAllocated;

	return nSize;
}

BOOL CCreateXBoxTgaFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/, BOOL bOutputDebugPics/*=FALSE*/ ) {

	if( !m_pMemAllocated ) {
		return FALSE;
	}

	BOOL bCloseFile = FALSE;
	if( !pFileStream ) {
		if( !pszFilename ) {
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T( "wb" ) );
		if( !pFileStream ) {
			return FALSE;
		}
		bCloseFile = TRUE;
	}

	// writeout the header info
	if ( FMATH_BYTE_ALIGN_UP( sizeof( FTexInfo_t ), 128 ) != sizeof( FTexInfo_t ) )
	{
		// Pad the header to be of size D3DTEXTURE_ALIGNMENT
		char szBuffer[128];
		memset( szBuffer, 0, 128 );
		u32 nPad = FMATH_BYTE_ALIGN_UP( sizeof( FTexInfo_t ), 128 ) - sizeof( FTexInfo_t );
		fwrite( &m_TexInfo, sizeof( FTexInfo_t ), 1, pFileStream );
		fwrite( szBuffer, nPad, 1, pFileStream );
	}
	else
	{
		fwrite( &m_TexInfo, sizeof( FTexInfo_t ), 1, pFileStream );
	}

	// writeout the image data
	fwrite( m_pMemAllocated, m_nBytesAllocated, 1, pFileStream );
	// close our file
	if( bCloseFile ) {
		fclose( pFileStream );
	}

#if( _OUTPUT_DEFAULT_FONTTEX_CPP_DATA_TO_CUR_DIR )
	//// Open the output file.
	//
	FILE *pFileStream2 = fopen( "fdx8fonttex.h", "wb" );
	if( ! pFileStream2 ) return FALSE;
	//
	////

	//// Write out our file.
	//
	for( u32 uIndex = 0; uIndex < sizeof( FTexInfo_t ); ++uIndex )
	{
		fprintf( pFileStream2, "0x%02x,%s", ((u8 *)&( m_TexInfo ))[ uIndex ], ( ( 0 == ( ( uIndex + 1 ) % 20 ) ) ? "\r\n" : " " ) );
	}
	for( u32 uTemp = 0; uTemp < m_nBytesAllocated; ++uTemp, ++uIndex )
	{
		fprintf( pFileStream2, "0x%02x,%s", ((u8 *)m_pMemAllocated)[ uTemp ], ( ( 0 == ( ( uIndex + 1 ) % 20 ) ) ? "\r\n" : " " ) );
	}
	//
	////

	// close our file
	fclose( pFileStream2 );
#endif

	if( bOutputDebugPics ) {
		if( !pszFilename ) {
			// invalid filename
			return TRUE;
		}
		// test code to spit out each mipmap level as a seperate file
		CString s;				
		u32 nH, nW, j;
		nH = m_nHeight;
		nW = m_nWidth;
		u8 *pSrc = m_pMemAllocated, naRGBA[4];
		for( u32 i=0; i < m_TexInfo.nLodCount; i++ ) {
			
			if( m_nDesiredFormat == XB_FORMAT_RAW32 ) {
				// this section of code writes out the 24 or 32 image as .raw
				s.Format( "%s Level%d %dx%d.raw", pszFilename, i, nW, nH );
				FILE *pFile = fopen( (cchar *)s, "wb" );
				if( !pFile ) {
					return TRUE;
				}
				for( j=0; j < (nW * nH); j++ ) {
					// data is in XRGB or ARGB format, raw needs to be rgba (remember little endian)
					naRGBA[0] = pSrc[2];
					naRGBA[1] = pSrc[1];
					naRGBA[2] = pSrc[0];
					naRGBA[3] = pSrc[3];
					fwrite( &naRGBA, sizeof( u8 ) * 4, 1, pFile );
					pSrc+=4;				
				}		
				fclose( pFile );
			} else if( m_nDesiredFormat == XB_FORMAT_DXT1 ||
					   m_nDesiredFormat == XB_FORMAT_DXT3 ||
					   m_nDesiredFormat == XB_FORMAT_DXT5 ) {
				// this section of code writes out a dds dds so that it can be loaded into photoshop
				s.Format( "%s Level%d %dx%d.dds", pszFilename, i, nW, nH );
				FILE *pFile = fopen( (cchar *)s, "wb" );
				if( !pFile ) {
					return TRUE;
				}
				WriteDDSFile( pFile, pSrc, nW, nH );
				if( m_nDesiredFormat == XB_FORMAT_DXT1 ) {
					pSrc += ((nW * nH) >> 1);
				} else {
					pSrc += (nW * nH);				
				}
				fclose( pFile );
			}

			if( nW > 1 ) {
				nW >>= 1;
			}
			if( nH > 1 ) {
				nH >>= 1;
			}
		}
	}
	return TRUE;
}

void CCreateXBoxTgaFile::FreeData() {
	
	if( m_pActualMemAllocated ) {
		delete[] (u8 *)m_pActualMemAllocated;
		m_pActualMemAllocated = NULL;
		m_pMemAllocated = NULL;
	}
}

BOOL CCreateXBoxTgaFile::GenerateRaw24or32File( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither ) {
	u32 i, j, nBytesPerPixel;
	u8 *pTGASrc;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	nBytesPerPixel = MipMap.GetBytesPerPixel();
	FASSERT( nBytesPerPixel == 3 || nBytesPerPixel == 4 );
	m_nHeight = MipMap.GetHighLODHeight();
	m_nWidth = MipMap.GetHighLODWidth();

	// fill in our header info
	m_TexInfo.nTexFmt = (nBytesPerPixel == 3) ? FDXDATA_TEXFMT_X8R8G8B8 : FDXDATA_TEXFMT_A8R8G8B8;
	m_TexInfo.nPalFmt = FDXDATA_PALFMT_NONE;
	m_TexInfo.nTexelsAcross = (u16)m_nWidth;
	m_TexInfo.nTexelsDown = (u16)m_nHeight;
	m_TexInfo.nLodCount = (u8)MipMap.GetNumLODs();
	m_TexInfo.nFlags = FTEX_FLAG_LOAD_IN_PLACE;
	m_TexInfo.pUserData = NULL;
	ZeroMemory( &m_TexInfo.szName, FDATA_TEXNAME_LEN+1 );
	strncpy( m_TexInfo.szName, pszTextureName, FDATA_TEXNAME_LEN );	
	
	// calculate the memory we will need
	m_nBytesAllocated = MipMap.GetNumPixels();
	m_nBytesAllocated <<= 2;// we need 4 bytes per pixel
//	m_nBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nBytesAllocated, 128 );

	// allocate our memory
	m_pActualMemAllocated = new u8[m_nBytesAllocated + 16];
	if( !m_pActualMemAllocated ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + MipMap.m_sLastConvertedFile );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	// Zero out the memory
	memset( m_pActualMemAllocated, 0, m_nBytesAllocated + 16 );

	m_pMemAllocated = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pActualMemAllocated, 16 );
	
	// fixup our pointers
	pTGASrc = (u8 *)MipMap.GetImage();
	if( nBytesPerPixel == 4 ) 
	{
#if 1
		u8 *pTempMem = new u8[m_nBytesAllocated + 16];
		// Swizzle all of the mip levels
		u32 nByte = 0; 
		u16 nHeight = m_nHeight;
		u16 nWidth = m_nWidth;
		for ( i = 0; i < m_TexInfo.nLodCount; i++ )
		{
			XGSwizzleRect( &pTGASrc[nByte], 0, NULL, &m_pMemAllocated[nByte], nWidth, nHeight, NULL, 4 );
			nByte += nHeight * nWidth * 4;
			nHeight = (u16)(nHeight >> 1);
			nWidth = (u16)(nWidth >> 1);
		}
		delete pTempMem;
#else
		// we can just copy the pixels straight over, rearranging properly ARGB (remember little endian)
		for( i=0; i < m_nBytesAllocated; i+=4 ) 
		{
			m_pMemAllocated[i+3] = pTGASrc[i+3];// a
			m_pMemAllocated[i+2] = pTGASrc[i+2];// r
			m_pMemAllocated[i+1] = pTGASrc[i+1];// g
			m_pMemAllocated[i] = pTGASrc[i];// b
		}
#endif
	} 
	else 
	{
#if 1
		u8 *pTempMem = new u8[m_nBytesAllocated];
		if( !pTempMem ) {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + MipMap.m_sLastConvertedFile );
			rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
			return FALSE;
		}

		// 24 bit images, stuff a 255 alpha into the X8 spot
		for( i=0, j=0; i < m_nBytesAllocated; i+=4, j+=3 ) 
		{
			pTempMem[i+3] = 255;// a
			pTempMem[i+2] = pTGASrc[j+2];// r
			pTempMem[i+1] = pTGASrc[j+1];// g
			pTempMem[i] = pTGASrc[j];// b
		}	
		
		// Swizzle all of the mip levels
		u32 nByte = 0; 
		u16 nHeight = m_nHeight;
		u16 nWidth = m_nWidth;
		for ( i = 0; i < m_TexInfo.nLodCount; i++ )
		{
			XGSwizzleRect( &pTempMem[nByte], 0, NULL, &m_pMemAllocated[nByte], nWidth, nHeight, NULL, 4 );
			nByte += nHeight * nWidth * 4;
			nHeight = (u16)(nHeight >> 1);
			nWidth = (u16)(nWidth >> 1);
		}

		delete pTempMem;
#else
		// 24 bit images, stuff a 255 alpha into the X8 spot
		for( i=0, j=0; i < m_nBytesAllocated; i+=4, j+=3 ) 
		{
			m_pMemAllocated[i+3] = 255;// a
			m_pMemAllocated[i+2] = pTGASrc[j+2];// r
			m_pMemAllocated[i+1] = pTGASrc[j+1];// g
			m_pMemAllocated[i] = pTGASrc[j];// b
		}	
#endif
	}	
	return TRUE;	
}

BOOL CCreateXBoxTgaFile::GenerateRaw16File( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither ) {
	u32 i, j, nBytesPerPixel;
	u8 *pTGASrc;
	u16 *pDest, nColor; 

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	nBytesPerPixel = MipMap.GetBytesPerPixel();
	FASSERT( nBytesPerPixel == 3 || nBytesPerPixel == 4 );
	m_nHeight = MipMap.GetHighLODHeight();
	m_nWidth = MipMap.GetHighLODWidth();

	// fill in our header info
	m_TexInfo.nPalFmt = FDXDATA_PALFMT_NONE;
	m_TexInfo.nTexelsAcross = (u16)m_nWidth;
	m_TexInfo.nTexelsDown = (u16)m_nHeight;
	m_TexInfo.nLodCount = (u8)MipMap.GetNumLODs();
	// map 32bit cutouts to 1555, 32bit non-cutouts to 4444, 24bit to 565
	if( nBytesPerPixel == 3 ) {
		m_TexInfo.nTexFmt = FDXDATA_TEXFMT_R5G6B5;
	} else if( MipMap.IsImageACutout() ) {
		m_TexInfo.nTexFmt = FDXDATA_TEXFMT_A1R5G5B5;
	} else {
		m_TexInfo.nTexFmt = FDXDATA_TEXFMT_A4R4G4B4;
	}	
	m_TexInfo.nFlags = FTEX_FLAG_LOAD_IN_PLACE;
	m_TexInfo.pUserData = NULL;
	ZeroMemory( &m_TexInfo.szName, FDATA_TEXNAME_LEN+1 );
	strncpy( m_TexInfo.szName, pszTextureName, FDATA_TEXNAME_LEN );	
	
	// calculate the memory we will need
	m_nBytesAllocated = MipMap.GetNumPixels();
	m_nBytesAllocated <<= 1;
//	m_nBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nBytesAllocated, 128 );// we need 2 bytes per pixel
	
	// allocate our memory
	m_pActualMemAllocated = new u8[m_nBytesAllocated + 16];
	if( !m_pActualMemAllocated ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + MipMap.m_sLastConvertedFile );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	// Zero out the memory
	memset( m_pActualMemAllocated, 0, m_nBytesAllocated + 16 );

	m_pMemAllocated = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pActualMemAllocated, 16 );

	u8 *pTempMem = new u8[m_nBytesAllocated];
	if( !pTempMem ) 
	{
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + MipMap.m_sLastConvertedFile );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	// fixup our pointers
	pTGASrc = (u8 *)MipMap.GetImage();
	pDest = (u16 *)pTempMem;
	if( nBytesPerPixel == 4 ) {
		// 32 bit images
		if( MipMap.IsImageACutout() ) {
			// cutouts go to 1555
			for( i=0, j=0; i < m_nBytesAllocated; i+=2, j+=4 ) {
				// a
				*pDest = (pTGASrc[j+3] >> 7);
				*pDest <<= 15;
				// r
				nColor = (pTGASrc[j+2] >> 3) << 10;
				*pDest |= nColor;
				// g
				nColor = (pTGASrc[j+1] >> 3) << 5;
				*pDest |= nColor;
				// b
				*pDest |= (pTGASrc[j] >> 3);

				pDest++;			
			}
		} else {
			// non-cutouts go to 4444
			for( i=0, j=0; i < m_nBytesAllocated; i+=2, j+=4 ) {
				// a
				*pDest = (pTGASrc[j+3] >> 4);
				*pDest <<= 12;
				// r
				nColor = (pTGASrc[j+2] >> 4) << 8;
				*pDest |= nColor;
				// g
				nColor = (pTGASrc[j+1] >> 4) << 4;
				*pDest |= nColor;
				// b
				*pDest |= (pTGASrc[j] >> 4);

				pDest++;			
			}
		}
	} else {
		// 24 bit images (565)
		for( i=0, j=0; i < m_nBytesAllocated; i+=2, j+=3 ) {
			// r
			*pDest = (pTGASrc[j+2] >> 3);
			*pDest <<= 11;
			// g
			nColor = (pTGASrc[j+1] >> 2) << 5;
			*pDest |= nColor;
			// b
			*pDest |= (pTGASrc[j] >> 3);

			pDest++;			
		}	
	}

	// Swizzle all of the mip levels
	u32 nByte = 0; 
	u16 nHeight = m_nHeight;
	u16 nWidth = m_nWidth;
	for ( i = 0; i < m_TexInfo.nLodCount; i++ )
	{
		XGSwizzleRect( &pTempMem[nByte], 0, NULL, &m_pMemAllocated[nByte], nWidth, nHeight, NULL, 2 );
		nByte += nHeight * nWidth * 2;
		nHeight = (u16)(nHeight >> 1);
		nWidth = (u16)(nWidth >> 1);
	}

	delete pTempMem;

	return TRUE;
}

// required that m_nDesiredFormat is filled in
BOOL CCreateXBoxTgaFile::GenerateDxtFile( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither ) {
	u32 i, nBytesPerPixel, nW, nH;
	u8 *pTGASrc;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	nBytesPerPixel = MipMap.GetBytesPerPixel();
	FASSERT( nBytesPerPixel == 3 || nBytesPerPixel == 4 );
	m_nHeight = MipMap.GetHighLODHeight();
	m_nWidth = MipMap.GetHighLODWidth();

	// setup the compression options
	CompressionOptions CompressOptions;
	CompressOptions.bMipMapsInImage = FALSE;
	CompressOptions.MipMapType = dNoMipMaps;
	CompressOptions.MIPFilterType = dMIPFilterBox;
	CompressOptions.bBinaryAlpha = (MipMap.IsImageACutout()) ? 1 : 0;
	CompressOptions.bNormalMap = FALSE;
    CompressOptions.bDuDvMap = FALSE;
	CompressOptions.bAlphaBorder = FALSE;
    CompressOptions.bBorder = FALSE;
    CompressOptions.BorderColor.u = 0;
	CompressOptions.bFade = FALSE;
    CompressOptions.bFadeAlpha = FALSE;
    CompressOptions.FadeToColor.u = 0;
    CompressOptions.FadeAmount = 0;
	CompressOptions.bDitherColor = !!bDither;
	CompressOptions.TextureType = dTextureType2D;
	CompressOptions.bSwapRGB = FALSE;
	
	// fill in our header info
	switch( m_nDesiredFormat ) {
	
	case XB_FORMAT_DXT1:
		CompressOptions.TextureFormat = (MipMap.IsImageACutout()) ? dDXT1a : dDXT1;
		m_TexInfo.nTexFmt = FDXDATA_TEXFMT_DXT1;
		m_nBytesAllocated = MipMap.GetNumPixels() >> 1;
//		m_nBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nBytesAllocated, 128 );// 8 to 1 compression
		break;
	default:
	case XB_FORMAT_DXT3:
		CompressOptions.TextureFormat = dDXT3;
		m_TexInfo.nTexFmt = FDXDATA_TEXFMT_DXT3;
		m_nBytesAllocated = MipMap.GetNumPixels();
//		m_nBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nBytesAllocated, 128 );// 4 to 1 compression
		break;
	case XB_FORMAT_DXT5:
		CompressOptions.TextureFormat = dDXT5;
		m_TexInfo.nTexFmt = FDXDATA_TEXFMT_DXT5;
		m_nBytesAllocated = MipMap.GetNumPixels();
//		m_nBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nBytesAllocated, 128 );// 4 to 1 compression
		break;
	}	
	m_TexInfo.nPalFmt = FDXDATA_PALFMT_NONE;
	m_TexInfo.nTexelsAcross = (u16)m_nWidth;
	m_TexInfo.nTexelsDown = (u16)m_nHeight;
	m_TexInfo.nLodCount = (u8)MipMap.GetNumLODs();
	m_TexInfo.nFlags = FTEX_FLAG_LOAD_IN_PLACE;
	m_TexInfo.pUserData = NULL;
	ZeroMemory( &m_TexInfo.szName, FDATA_TEXNAME_LEN+1 );
	strncpy( m_TexInfo.szName, pszTextureName, FDATA_TEXNAME_LEN );

	// allocate our memory
	m_pActualMemAllocated = new u8[m_nBytesAllocated + 16];
	if( !m_pActualMemAllocated ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + MipMap.m_sLastConvertedFile );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	// Zero out the memory
	memset( m_pActualMemAllocated, 0, m_nBytesAllocated + 16 );

	m_pMemAllocated = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pActualMemAllocated, 16 );

	// setup the vars used during the dxt callback process
	_pDxtMemory = m_pMemAllocated;
	_nDxtByteOffset = 0;
	_nTotalDxtMemory = m_nBytesAllocated;
	
	// process each LOD seperately
	nW = m_nWidth;
	nH = m_nHeight;
	pTGASrc = (u8 *)MipMap.GetImage();
	for( i=0; i < m_TexInfo.nLodCount; i++ ) {
		nvDXTcompress( (unsigned char *)pTGASrc, nW, nH, nW * nBytesPerPixel, &CompressOptions, nBytesPerPixel, _DxtMipCallBack );
		pTGASrc += (nW * nH * nBytesPerPixel);

		if( nW > 1 ) {
			nW >>= 1;
		}
		if( nH > 1 ) {
			nH >>= 1;
		}
	}	
	return TRUE;
}

static HRESULT _DxtMipCallBack( void *pData, int nMipLevel, DWORD nSizeInBytes ) {
	
	FASSERT( (_nDxtByteOffset + nSizeInBytes) <= _nTotalDxtMemory );

	u8 *pDest = (u8 *)&_pDxtMemory[_nDxtByteOffset];
	fang_MemCopy( pDest, pData, nSizeInBytes );
	_nDxtByteOffset += nSizeInBytes;

	return 0;
}

#if 0// because the gctexlib defines this now
void WriteDTXnFile( DWORD count, void *buffer ) {
    // stubbed out so that we can use the nvidia library
}

void ReadDTXnFile( DWORD count, void *buffer ) {
    // stubbed out so that we can use the nvidia library
}
#endif

// required that m_nDesiredFormat is filled in
void CCreateXBoxTgaFile::WriteDDSFile( FILE *pFile, u8 *pImage, u32 nW, u32 nH ) {
    DDS_HEADER ddsh;
    DWORD dwMagic;
       
	// write out the 4 byte signature
    dwMagic = MAKEFOURCC('D','D','S',' ');
    fwrite( &dwMagic, sizeof( dwMagic ), 1, pFile );

    // Fill in the DDS header structure
    ZeroMemory( &ddsh, sizeof( ddsh ) );
    ddsh.dwSize = sizeof( ddsh );
    ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE;
    ddsh.dwWidth = nW;
    ddsh.dwHeight = nH;
    ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;
    
    switch( m_nDesiredFormat ) {

    case XB_FORMAT_DXT1:
        ddsh.ddspf = DDSPF_DXT1;
        ddsh.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.dwPitchOrLinearSize = (nW * nH) >> 1;
        break;
    
	case XB_FORMAT_DXT3:
        ddsh.ddspf = DDSPF_DXT3;
        ddsh.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.dwPitchOrLinearSize = (nW * nH);
        break;
    
	case XB_FORMAT_DXT5:
        ddsh.ddspf = DDSPF_DXT5;
        ddsh.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh.dwPitchOrLinearSize = (nW * nH);
        break;
    }

	// write out the dds header
    fwrite( &ddsh, sizeof( ddsh ), 1, pFile );

	// write out the file data
	fwrite( pImage, ddsh.dwPitchOrLinearSize, 1, pFile );
}
	

