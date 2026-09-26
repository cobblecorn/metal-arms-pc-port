//////////////////////////////////////////////////////////////////////////////////////
// CreateGCTgaFile.cpp - 
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
#include <mmsystem.h>
#include "fang.h"
#include "CreateGCTgaFile.h"
#include "fdata.h"
#include "ErrorLog.h"
#include "fclib.h"
#include "ftex.h"
#if GC_TEX_LIB_USE_NVIDIA_LIB
	#include "dxtlib.h"
#else
	#include "S3_intrf.h"
#endif

#define _ERROR_HEADING								"TGA->GC TEXTURE MAP FILE COMPILER "

//// Default font data.
//
// Note: copy fgcfonttex.h (generated in the current directory) to fang2\gc.
#define _OUTPUT_DEFAULT_FONTTEX_CPP_DATA_TO_CUR_DIR		FALSE
//
////


// private prototypes
static u32  _GCConvertTex( SrcFormat_e nSrcFmt, u32 nWidth, u32 nHeight, void *pSource, FData_TexFmt_e nDestFmt, void **pResult );
static void _GCSwizzleTexData( SrcFormat_e nSrcFmt, u32 w, u32 h, const void *pSource, FData_TexFmt_e nDstFmt, void *pResult );
static BOOL _GenerateDxtFile( SrcFormat_e nSrcFmt, u32 nWidth, u32 nHeight, void *pSource, FData_TexFmt_e nDestFmt, void **pResult, u32 &rnResultSize );


// private vars (used during dxt creation)
static u8 *_pDxtMemory;
static u32 _nDxtByteOffset;
static u32 _nTotalDxtMemory;
static u32 _nConvertTime = 0;
static u32 _nCompressTime = 0;
static u32 _nSwizzleTime = 0;
static u32 _nTempTime;


CCreateGCTgaFile::CCreateGCTgaFile() {
	ZeroMemory( &m_TexInfo, sizeof( FTexInfo_t ) );
	m_nDesiredFormat = GC_FORMAT_S3TC;
	m_nTotalFileBytes = 0;
	m_nNumMipLevels = 0;
	m_paConvertedDataInfos = NULL;
}

CCreateGCTgaFile::~CCreateGCTgaFile() {
	FreeData();
}

CCreateGCTgaFile::GC_Format_e CCreateGCTgaFile::GetFormatFromString( BOOL bCutout, 
																	 const CString &rsDesiredFormat,
																	 u32 nBitsPerPixel ) {
	FASSERT( nBitsPerPixel == 24 || nBitsPerPixel == 32 );

	GC_Format_e nFormat;

	if( rsDesiredFormat.CompareNoCase( "auto" ) == 0 ) {
		if( bCutout ) {
			nFormat = GC_FORMAT_S3TCA1;
		} else if( nBitsPerPixel == 24 ) {
			nFormat = GC_FORMAT_S3TC;
		} else {
			nFormat = GC_FORMAT_S3TCx2;
		}
	} else if( rsDesiredFormat.CompareNoCase( "s3tc" ) == 0 ) {
		nFormat = GC_FORMAT_S3TC;
	} else if( rsDesiredFormat.CompareNoCase( "s3tca1" ) == 0 ) {
		nFormat = GC_FORMAT_S3TCA1;

		if( nBitsPerPixel == 24 ) {
			// this mode require a 32 bit source, switch to a different mode
			nFormat = GC_FORMAT_S3TC;
		}
	} else if( rsDesiredFormat.CompareNoCase( "s3tcx2" ) == 0 ) {
		nFormat = GC_FORMAT_S3TCx2;

		if( nBitsPerPixel == 24 ) {
			// this mode require a 32 bit source, switch to a different mode
			nFormat = GC_FORMAT_S3TC;
		}
	} else if( rsDesiredFormat.CompareNoCase( "intensity8" ) == 0 ) {
		nFormat = GC_FORMAT_I8;
	} else if( rsDesiredFormat.CompareNoCase( "argb8888" ) == 0 ) {
		nFormat = GC_FORMAT_R8G8B8A8;

		if( nBitsPerPixel == 24 ) {
			// this mode require a 32 bit source, switch to a different mode
			nFormat = GC_FORMAT_S3TC;
		}
	} else if( rsDesiredFormat.CompareNoCase( "rgb888" ) == 0 ) {
		nFormat = GC_FORMAT_R8G8B8X8;
	} else if( rsDesiredFormat.CompareNoCase( "argb4443" ) == 0 ) {
		nFormat = GC_FORMAT_R4G4B4A3;

		if( nBitsPerPixel == 24 ) {
			// this mode require a 32 bit source, switch to a different mode
			nFormat = GC_FORMAT_S3TC;
		}
	} else if( rsDesiredFormat.CompareNoCase( "rgb565" ) == 0 ) {
		nFormat = GC_FORMAT_R5G6B5;
	} else if ( rsDesiredFormat.CompareNoCase( "ia8" ) == 0 ) {
		nFormat = GC_FORMAT_IA8;
	} else if ( rsDesiredFormat.CompareNoCase( "ia4" ) == 0 ) {
		nFormat = GC_FORMAT_IA4;
	} else {
		// default to s3tc
		nFormat = GC_FORMAT_S3TC;
	}	
	
	return nFormat;
}

BOOL CCreateGCTgaFile::ConvertTgaFile( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither, GC_Format_e nFormat/*=GC_FORMAT_S3TC*/ ) {
	BOOL bReturn;

	FreeData();

	if( nFormat >= GC_FORMAT_COUNT ) {
		nFormat = GC_FORMAT_S3TC;
	}
	m_nDesiredFormat = nFormat;

	bReturn = ConvertTgaImage( MipMap, pszTextureName, bDither );
	
	if( !bReturn ) {
		FreeData();
	}

	return bReturn;	
}

u32 CCreateGCTgaFile::GetDataCRC( void ) 
{
	if ( !m_paConvertedDataInfos ) 
	{
		return 0;
	}

	u32 i, nReturnCRC = fmath_Crc32( 0, (u8 *)&m_TexInfo, sizeof( FTexInfo_t ) );
	if( m_nDesiredFormat != GC_FORMAT_S3TCx2 ) 
	{
		for( i=0; i < m_nNumMipLevels; i++ ) 
		{
			fmath_Crc32( nReturnCRC, (u8 *)m_paConvertedDataInfos[i].pcConvertedData, m_paConvertedDataInfos[i].nNumBytes );
		}		
	} 
	else 
	{
		// write out the 1st half of the data
		for( i=0; i < m_nNumMipLevels; i++ ) 
		{
			u32 nBytes = m_paConvertedDataInfos[i].nNumBytes >> 1;
			fmath_Crc32( nReturnCRC, (u8 *)m_paConvertedDataInfos[i].pcConvertedData, nBytes );
		}
		// write out the 2nd half of the data
		for( i=0; i < m_nNumMipLevels; i++ ) 
		{
			u32 nBytes = m_paConvertedDataInfos[i].nNumBytes >> 1;
			fmath_Crc32( nReturnCRC, (u8 *)m_paConvertedDataInfos[i].pcConvertedData, nBytes );
		}
	}

	return nReturnCRC;
}

u32 CCreateGCTgaFile::GetSizeOfConvertedFile() {

	if( !m_paConvertedDataInfos ) {
		return 0;
	}	
	return m_nTotalFileBytes;
}

BOOL CCreateGCTgaFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

	if( !m_paConvertedDataInfos ) {
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
	fwrite( &m_TexInfo, sizeof( FTexInfo_t ), 1, pFileStream );
	// writeout the image data
	u32 i, nBytes;
	if( m_nDesiredFormat != GC_FORMAT_S3TCx2 ) {
		for( i=0; i < m_nNumMipLevels; i++ ) {
			fwrite( m_paConvertedDataInfos[i].pcConvertedData, m_paConvertedDataInfos[i].nNumBytes, 1, pFileStream );
		}		
	} else {
		// write out the 1st half of the data
		for( i=0; i < m_nNumMipLevels; i++ ) {
			nBytes = m_paConvertedDataInfos[i].nNumBytes >> 1;
			fwrite( m_paConvertedDataInfos[i].pcConvertedData, nBytes, 1, pFileStream );
		}
		// write out the 2nd half of the data
		for( i=0; i < m_nNumMipLevels; i++ ) {
			nBytes = m_paConvertedDataInfos[i].nNumBytes >> 1;
			fwrite( &m_paConvertedDataInfos[i].pcConvertedData[nBytes], nBytes, 1, pFileStream );
		}
	}
	// close our file
	if( bCloseFile ) {
		fclose( pFileStream );
	}

#if( _OUTPUT_DEFAULT_FONTTEX_CPP_DATA_TO_CUR_DIR )
	//// Open the output file.
	//
	FILE *pFileStream2 = fopen( "fgcfonttex.h", "wb" );
	if( ! pFileStream2 ) return FALSE;
	//
	////

	//// Write out our file.
	//
	u32 nOffset = 0;
	for( i=0; i < sizeof( FTexInfo_t ); ++i, ++nOffset ) {
		fprintf( pFileStream2, "0x%02x,%s", ((u8 *)&( m_TexInfo ))[ i ], ( ( 0 == ( ( nOffset + 1 ) % 20 ) ) ? "\r\n" : " " ) );
	}
	u32 j;
	if( m_nDesiredFormat != GC_FORMAT_S3TCx2 ) {
		for( i=0; i < m_nNumMipLevels; i++ ) {
			for( j=0; j < m_paConvertedDataInfos[i].nNumBytes; j++, ++nOffset ) {
				fprintf( pFileStream2, "0x%02x,%s", (u8)( m_paConvertedDataInfos[i].pcConvertedData[j] ), ( ( 0 == ( ( nOffset + 1 ) % 20 ) ) ? "\r\n" : " " ) );
			}
		}
	} else {
		// write out the 1st half of the data
		for( i=0; i < m_nNumMipLevels; i++ ) {
			nBytes = m_paConvertedDataInfos[i].nNumBytes >> 1;
			for( j=0; j < nBytes; j++, ++nOffset ) {
				fprintf( pFileStream2, "0x%02x,%s", (u8)( m_paConvertedDataInfos[i].pcConvertedData[j] ), ( ( 0 == ( ( nOffset + 1 ) % 20 ) ) ? "\r\n" : " " ) );
			}
		}
		// write out the 2nd half of the data
		for( i=0; i < m_nNumMipLevels; i++ ) {
			nBytes = m_paConvertedDataInfos[i].nNumBytes >> 1;
			for( j=0; j < nBytes; j++, ++nOffset ) {
				fprintf( pFileStream2, "0x%02x,%s", (u8)( m_paConvertedDataInfos[i].pcConvertedData[nBytes + j] ), ( ( 0 == ( ( nOffset + 1 ) % 20 ) ) ? "\r\n" : " " ) );
			}
		}
	}
	//
	////

	// close our file
	fclose( pFileStream2 );
#endif

	return TRUE;
}

void CCreateGCTgaFile::FreeData() {
	
	if( m_paConvertedDataInfos ) {
		// we must delete any memory that the GCTexConverter allocated
		u32 i;
		for( i=0; i < m_nNumMipLevels; i++ ) {
			if( m_paConvertedDataInfos[i].pcConvertedData ) {
				free( m_paConvertedDataInfos[i].pcConvertedData );
				m_paConvertedDataInfos[i].pcConvertedData = NULL;
			}
		}
		m_nNumMipLevels = 0;

		delete [] m_paConvertedDataInfos;
		m_paConvertedDataInfos = NULL;
	}
	ZeroMemory( &m_TexInfo, sizeof( FTexInfo_t ) );
}

FData_TexFmt_e CCreateGCTgaFile::TranslateFormatCode( GC_Format_e nFormat ) {

	switch( nFormat ) {
	case GC_FORMAT_R8G8B8A8:
		return FGCDATA_TEXFMT_R8G8B8A8;
		break;
	case GC_FORMAT_R8G8B8X8:
		return FGCDATA_TEXFMT_R8G8B8X8;
		break;
	case GC_FORMAT_R4G4B4A3:
		return FGCDATA_TEXFMT_R4G4B4A3;
		break;
	case GC_FORMAT_R5G6B5:
		return FGCDATA_TEXFMT_R5G6B5;
		break;
	case GC_FORMAT_S3TC:
		return FGCDATA_TEXFMT_S3TC;
		break;
	case GC_FORMAT_S3TCA1:
		return FGCDATA_TEXFMT_S3TCA1;
		break;
	case GC_FORMAT_S3TCx2:
		return FGCDATA_TEXFMT_S3TCx2;
		break;
	case GC_FORMAT_I8:
		return FGCDATA_TEXFMT_I8;
		break;
	case GC_FORMAT_IA8:
		return FGCDATA_TEXFMT_IA8;
		break;
	case GC_FORMAT_IA4:
		return FGCDATA_TEXFMT_IA4;
		break;
	}

	return FGCDATA_TEXFMT_S3TC;
}

BOOL CCreateGCTgaFile::ConvertTgaImage( CGenMipMaps &MipMap, cchar *pszTextureName, BOOL bDither ) {
	u32 i, j, nBytesPerPixel, nH, nW, nPixels;
	u8 *pTGASrc, *pSrc;
	u32 *pnPixel;
	SrcFormat_e nSrcFormat;
	BOOL bSwitchByteOrder;

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	nBytesPerPixel = MipMap.GetBytesPerPixel();
	nSrcFormat = (nBytesPerPixel == 3) ? SRC_FMT_R8G8B8 : SRC_FMT_R8G8B8A8;
	nH = MipMap.GetHighLODHeight();
	nW = MipMap.GetHighLODWidth();
	pTGASrc = (u8 *)MipMap.GetImage();

	m_nNumMipLevels = MipMap.GetNumLODs();
	
	// fill in our header info (it has already been set to zero)
	fclib_strncpy( m_TexInfo.szName, pszTextureName, FDATA_TEXNAME_LEN );
	m_TexInfo.nTexFmt = TranslateFormatCode( m_nDesiredFormat );
	m_TexInfo.nPalFmt = FTEX_PALFMT_NONE;
	m_TexInfo.nLodCount	= (u8)m_nNumMipLevels;
	m_TexInfo.nTexelsAcross = (u16)nW;
	m_TexInfo.nTexelsDown = (u16)nH;
	m_TexInfo.nFlags = FTEX_FLAG_LOAD_IN_PLACE;
	
	// allocate memory for our return pointers
	m_paConvertedDataInfos = new GCTexConvInfo_t[m_nNumMipLevels];
	if( !m_paConvertedDataInfos ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + MipMap.m_sLastConvertedFile );
		rErrorLog.WriteErrorLine( "Could not allocate memory for converted image pointers" );
		return FALSE;
	}

	// zero out our pointers
	for( i=0; i < m_nNumMipLevels; i++ ) {
		m_paConvertedDataInfos[i].nNumBytes = 0;
		m_paConvertedDataInfos[i].pcConvertedData = NULL;
	}
	
	if( m_TexInfo.nTexFmt != FGCDATA_TEXFMT_S3TC &&
		m_TexInfo.nTexFmt != FGCDATA_TEXFMT_S3TCA1 &&
		m_TexInfo.nTexFmt != FGCDATA_TEXFMT_S3TCx2  &&
		nBytesPerPixel == 4 ) {
		bSwitchByteOrder = TRUE;
	} else {
		bSwitchByteOrder = FALSE;
	}

	// convert each level (1 by 1)
	pSrc = pTGASrc;
	m_nTotalFileBytes = 0;
	for( i=0; i < m_nNumMipLevels; i++ ) {
		if( bSwitchByteOrder ) {
			// switch from argb to rgba
			nPixels = nW * nH;
			pnPixel = (u32 *)pSrc;
			for( j=0; j < nPixels; j++, pnPixel++ ) {
				*pnPixel = ( ((*pnPixel >> 16) & 0xff) << 24 ) | // New R
						   ( ((*pnPixel >>  8) & 0xff) << 16 ) | // New G
						   ( ((*pnPixel >>  0) & 0xff) <<  8 ) | // New B
						   ( ((*pnPixel >> 24) & 0xff) <<  0 );	// New A
			}
		}
		m_paConvertedDataInfos[i].nNumBytes = _GCConvertTex( nSrcFormat, 
															nW, nH,
															pSrc,
															(FData_TexFmt_e)m_TexInfo.nTexFmt,
															(void **)&m_paConvertedDataInfos[i].pcConvertedData );
		if( m_paConvertedDataInfos[i].nNumBytes == 0 ||
			m_paConvertedDataInfos[i].pcConvertedData == NULL ) {
			// problem during conversion
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + MipMap.m_sLastConvertedFile );
			rErrorLog.WriteErrorLine( "A problem was encountered during GC library texture conversion" );
			return FALSE;
		}
		FASSERT( !(m_paConvertedDataInfos[i].nNumBytes & 0x1) );// all num bytes should be an even number
		m_nTotalFileBytes += m_paConvertedDataInfos[i].nNumBytes;

		// advance our src pointers and size
		pSrc += (nH * nW * nBytesPerPixel);
		if( nH > 1 ) {
			nH >>= 1;
		}
		if( nW > 1 ) {
			nW >>= 1;
		}
	}

	// add the header to the total file byte count
	m_nTotalFileBytes += sizeof( FTexInfo_t );
	
	// LAST STEP - CONVERT GC TEXTURE HEADER TO BIG-ENDIAN
	m_TexInfo.ChangeEndian();
	
	return TRUE;
}


//
//	_GCConvertTex() - converts raw image data (no headers!) into GC image data (w/o headers!)
//
//	Parameters:
//		nSrcFmt	- format of source data
//		nWidth	- width of the source
//		nHeight	- height of the source
//		pSource - pointer to the source image data
//		nDestFmt- requested format of the destination data
//		pResult - NULL pointer that will be filled in with a pointer to the created destination data
//
//	Return:
//		A u32 which specifies the size of the newly created GC image data.
//
//	Notes:
//		* This function will change the values in pSource.  Keep that in mind!
//		* The requesting function has the obligation to free the memory allocated and assigned to pResult.
//		* The requesting function has the responsibility of creating appropriate headers and assigning
//			them to the texture data or group of textures.
//
//	Description of processing:
//		REQUESTED FORMAT (nDestFmt):
//			FTEX_FMT_R8G8B8A8 - accepts only SRC_FMT_R8G8B8A8 data, spits out 32-bit GC 
//									uncompressed RGBA in big-endian form
//			FTEX_FMT_R8G8B8X8 - accepts SRC_FMT_R8G8B8A8 or SRC_FMT_R8G8B8 data, 
//									spits out 32-bit GC uncompressed RGBA in big-endian  
//									form with alpha at 255
//			FTEX_FMT_R4G4B4A3 - accepts only SRC_FMT_R8G8B8A8 data, spits out 16-bit GC 
//									uncompressed 3444 (ARGB) data in big-endian form 
//									with the most significant bit reserved.
//			FTEX_FMT_R5G5B5X1 - accepts SRC_FMT_R8G8B8A8 or SRC_FMT_R8G8B8 data, 
//									spits out 16-bit GC uncompressed 555 (RGB) data in 
//									big-endian form with the most significant bit reserved. 
//									This format should not be used except by Fang, internally
//									since FTEX_FMT_R5G6B5 is preferrable.
//			FTEX_FMT_R5G6B5 - accepts SRC_FMT_R8G8B8A8 or SRC_FMT_R8G8B8 data, 
//									spits out 16-bit GC uncompressed 565 (RGB) data in 
//									big-endian form.
//			FTEX_FMT_S3TC - accepts SRC_FMT_R8G8B8A8 or SRC_FMT_R8G8B8 data, and
//									spits out S3TC compressed RGB data (4bits/texel) in
//									big-endian form.
//			FTEX_FMT_S3TCA1 - accepts only SRC_FMT_R8G8B8A8 data, spits out GC 
//									compressed RGBA (4bits/texel) with one bit of alpha.  
//									The threshhold for the alpha is 1, so any non-zero 
//									alpha value will be opaque.
//			FTEX_FMT_S3TCx2 - accepts only SRC_FMT_R8G8B8A8 data, spits out TWO GC
//									compressed (4bits/texel).  The first image is
//									compressed color information plus 1 bit alpha representation
//									based on the alpha channel and the second is
//									compressed alpha information in RGB.  The second image 
//									starts in the middle of the data returned in pResult.
//			FTEX_FMT_I8 - accepts SRC_FMT_R8G8B8A8 or SRC_FMT_R8G8B8 data, and creates a
//									GC uncompressed 8-bit intensity map.  If an RGBA texture
//									is passed in, the alpha channel is used to create
//									the intensity map.  If an RGB texture is passed in,
//									the Red channel is used.
//			FTEX_FMT_IA8 - accepts SRC_FMT_R8G8B8A8 data, and creates a
//									GC uncompressed 8-bit intensity + 8-bit alpha map. Red Channel = Intensity.
//			FTEX_FMT_IA4 - accepts SRC_FMT_R8G8B8A8 data, and creates a
//									GC uncompressed 4-bit intensity + 4-bit alpha map. Red Channel = Intensity.
//
u32 _GCConvertTex( SrcFormat_e nSrcFmt, u32 nWidth, u32 nHeight, void *pSource, FData_TexFmt_e nDestFmt, void **pResult )
{
	u32 nResultSize = 0;

	u32 nStartTime = timeGetTime();
	_nConvertTime = 0;
	_nCompressTime = 0;
	_nSwizzleTime = 0;
	_nTempTime;

	// pResult should be NULL so that we can fill it in with data
	if ( *pResult )
		return 0;

	switch ( nDestFmt )
	{
		case FGCDATA_TEXFMT_R8G8B8A8: 
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8A8 )
				return NULL;

			// Calculate the size of the source image
			u32 nSourceImageSize = nWidth * nHeight * 4;

			// Calculate the size of the resulting image
			nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight * 4, 32 );

			// Allocate space for the resulting texture
			*pResult = malloc( nResultSize );
			if( !*pResult ) {
				return NULL;
			}
			memset( *pResult, 0, nResultSize );

			// Convert the little endian source to big endian
			u32 *pTemp = (u32 *)pSource;
			for ( u32 i = 0; i < nSourceImageSize; i += 4, pTemp++ )
				*pTemp = fang_ConvertEndian( *pTemp );

			_nTempTime = timeGetTime();
			_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
			_nSwizzleTime += timeGetTime() - _nSwizzleTime;

			break;
		}
		
		case FGCDATA_TEXFMT_R8G8B8X8:
		{
			if( SRC_FMT_R8G8B8 == nSrcFmt ) {
				// 24 bit source, we have to allocate some temp memory

				// Calculate the size of the source image
				u32 nNumPixels = nWidth * nHeight;
				u32 nSourceImageSize = nNumPixels * 3;

				// Calculate the size of the resulting image
				nResultSize = FMATH_BYTE_ALIGN_UP( nNumPixels * 4, 32 );

				// Allocate space for the resulting texture
				*pResult = malloc( nResultSize );
				if( !*pResult ) {
					return NULL;
				}
				memset( *pResult, 0, nResultSize );

				// create a temp buffer to create 32 bit source data from our 24
				void *p32BitSourceBuf = malloc( nNumPixels * 4 );
				if( !p32BitSourceBuf ) {
					free( *pResult );
					*pResult = NULL;
					return NULL;
				}
				memset( p32BitSourceBuf, 0, nNumPixels * 4 );

				// copy into our new source buffer
				u32 i;
				u8 *pcOrigSrc = (u8 *)pSource;
				u32 *pnNewSrc = (u32 *)p32BitSourceBuf;
				for( i=0; i < nNumPixels; i++, pcOrigSrc+=3, pnNewSrc++ ) {
					*pnNewSrc = (pcOrigSrc[0] << 8) |  // b
								(pcOrigSrc[1] << 16) | // g
								(pcOrigSrc[2] << 24);  // r
					*pnNewSrc |= 0xFF;
					*pnNewSrc = fang_ConvertEndian( *pnNewSrc );
				}

				_nTempTime = timeGetTime();
				_GCSwizzleTexData( SRC_FMT_R8G8B8A8, nWidth, nHeight, p32BitSourceBuf, nDestFmt, *pResult );
				_nSwizzleTime += timeGetTime() - _nSwizzleTime;

				// free our temp memory
				free( p32BitSourceBuf );

			} else {
				// 32 bit source, we should set the x channel to 255
				
				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 4;

				// Calculate the size of the resulting image
				nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight * 4, 32 );

				// Allocate space for the resulting texture
				*pResult = malloc( nResultSize );
				if( !*pResult ) {
					return NULL;
				}
				memset( *pResult, 0, nResultSize );

				// Convert the little endian source to big endian
				u32 *pTemp = (u32 *)pSource;
				for ( u32 i = 0; i < nSourceImageSize; i += 4, pTemp++ ) {
					*pTemp |= 0xFF;// put 255 in whatever may be in the alpha channel
					*pTemp = fang_ConvertEndian( *pTemp );
				}

				_nTempTime = timeGetTime();
				_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
				_nSwizzleTime += timeGetTime() - _nSwizzleTime;
			}

			break;
		}

		case FGCDATA_TEXFMT_R4G4B4A3:
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8A8 )
				return NULL;

			// Calculate the size of the source image
			u32 nSourceImageSize = nWidth * nHeight * 4;

			// Convert to 4443
			u8  *pTemp8 = (u8 *)pSource;
			u16 *pTemp16 = (u16 *)pSource;
			for ( u32 i = 0; i < nSourceImageSize; i += 4, pTemp8 += 4, pTemp16++ )
			{
				*pTemp16 =((*pTemp8	      >> 5) <<  0)
						| ((*(pTemp8 + 1) >> 4) <<  4)
						| ((*(pTemp8 + 2) >> 4) <<  8)		
						| ((*(pTemp8 + 3) >> 4) << 12);
			}

			// Convert the little endian source to big endian
			u16 *pTemp = (u16 *)pSource;
			for ( i = 0; i < nSourceImageSize; i += 2, pTemp++ )
				*pTemp = fang_ConvertEndian( *pTemp );

			// Calculate the size of the resulting image
			nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight * 2, 32 );

			// Allocate memory to contain the resulting image
			*pResult = malloc( nResultSize );

			_nTempTime = timeGetTime();
			_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
			_nSwizzleTime += timeGetTime() - _nSwizzleTime;

			break;
		}

		case FGCDATA_TEXFMT_R5G5B5X1:
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8A8 && nSrcFmt != SRC_FMT_R8G8B8 )
				return NULL;

			u32 i;

			if ( nSrcFmt == SRC_FMT_R8G8B8A8 )
			{
				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 4;

				// Convert RGBA32 to 555X
				u8  *pTemp8 = (u8 *)pSource;
				u16 *pTemp16 = (u16 *)pSource;
				for ( i = 0; i < nSourceImageSize; i += 4, pTemp8 += 4, pTemp16++ )
				{
					*pTemp16 =((1)					<<  0)
							| ((*(pTemp8 + 1) >> 3) <<  1)
							| ((*(pTemp8 + 2) >> 3) <<  6)		
							| ((*(pTemp8 + 3) >> 3) << 11);
				}
			}
			else
			{
				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 3;

				// Convert RGB24 to 555X
				u8  *pTemp8 = (u8 *)pSource;
				u16 *pTemp16 = (u16 *)pSource;
				for ( i = 0; i < nSourceImageSize; i += 3, pTemp8 += 3, pTemp16++ )
				{
					*pTemp16 =((1)					<<  0)
							| ((*(pTemp8 + 0) >> 3) <<  1)
							| ((*(pTemp8 + 1) >> 3) <<  6)		
							| ((*(pTemp8 + 2) >> 3) << 11);
				}
			}

			// Calculate the size of the resulting image
			nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight * 2, 32 );

			// Convert the little endian source to big endian
			u16 *pTemp = (u16 *)pSource;
			for ( i = 0; i < nResultSize; i += 2, pTemp++ )
				*pTemp = fang_ConvertEndian( *pTemp );

			// Allocate memory to contain the resulting image
			*pResult = malloc( nResultSize );

			_nTempTime = timeGetTime();
			_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
			_nSwizzleTime += timeGetTime() - _nSwizzleTime;

			break;
		}

		case FGCDATA_TEXFMT_R5G6B5:
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8A8 && nSrcFmt != SRC_FMT_R8G8B8 )
				return NULL;

			u32 i;

			if ( nSrcFmt == SRC_FMT_R8G8B8A8 )
			{
				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 4;

				// Convert RGBA32 to 565
				u8  *pTemp8 = (u8 *)pSource;
				u16 *pTemp16 = (u16 *)pSource;
				for ( i = 0; i < nSourceImageSize; i += 4, pTemp8 += 4, pTemp16++ )
				{
					*pTemp16 =((0)					<<  0)
							| ((*(pTemp8 + 1) >> 3) <<  0)
							| ((*(pTemp8 + 2) >> 2) <<  5)		
							| ((*(pTemp8 + 3) >> 3) << 11);
				}
			}
			else
			{
				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 3;

				// Convert RGB24 to 565
				u8  *pTemp8 = (u8 *)pSource;
				u16 *pTemp16 = (u16 *)pSource;
				for ( i = 0; i < nSourceImageSize; i += 3, pTemp8 += 3, pTemp16++ )
				{
					*pTemp16 =((0)					<<  0)
							| ((*(pTemp8 + 0) >> 3) <<  0)
							| ((*(pTemp8 + 1) >> 2) <<  5)		
							| ((*(pTemp8 + 2) >> 3) << 11);
				}
			}

			// Calculate the size of the resulting image
			nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight * 2, 32 );

			// Convert the little endian source to big endian
			u16 *pTemp = (u16 *)pSource;
			for ( i = 0; i < nResultSize; i += 2, pTemp++ )
				*pTemp = fang_ConvertEndian( *pTemp );

			// Allocate memory to contain the resulting image
			*pResult = malloc( nResultSize );

			_nTempTime = timeGetTime();
			_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
			_nSwizzleTime += timeGetTime() - _nSwizzleTime;

			break;
		}

		case FGCDATA_TEXFMT_S3TCx2:
		case FGCDATA_TEXFMT_S3TC:
		case FGCDATA_TEXFMT_S3TCA1:
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8 && nSrcFmt != SRC_FMT_R8G8B8A8 )
				return NULL;

			// We need an alpha channel to do FTEX_FMT_S3TCx2 or FTEX_FMT_S3TCA1
			if ( nSrcFmt == SRC_FMT_R8G8B8 && nDestFmt != FGCDATA_TEXFMT_S3TC )
				return NULL;

#if GC_TEX_LIB_USE_NVIDIA_LIB
			// use the nvidia library
			if( !_GenerateDxtFile( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, pResult, nResultSize ) ) {
				return NULL;
			}
#else
			// Set the S3TC encoding flag
			u32 nEncodeFlag;
			if ( nDestFmt == FGCDATA_TEXFMT_S3TC )//|| nDestFmt == FGCDATA_TEXFMT_S3TCx2 )
				nEncodeFlag = S3TC_ENCODE_RGB_FULL;
			else
				nEncodeFlag = S3TC_ENCODE_RGB_ALPHA_COMPARE;

			// Threshold alpha
			int nAlphaRef = 1; 
			if ( nDestFmt == FGCDATA_TEXFMT_S3TCx2 )
				nAlphaRef = 128;

			// Quantization weights for compression.
			// Note: these are the default weights from S3TC.doc
			float fRedWgt   = 0.3086f;
			float fGreenWgt = 0.6094f;
			float fBlueWgt  = 0.0820f; 

			// Set color weights
			S3TC_SetColorWeighting( fRedWgt, fGreenWgt, fBlueWgt );

			// Set threshold for alpha compare
			S3TC_SetAlphaReference( nAlphaRef );

			// Get the required buffer size
			u32 nEncodeSize = S3TC_GetEncodeSize( nWidth, nHeight, nEncodeFlag );

			// Allocate memory to contain the compressed image
			void *pCompressed = malloc( nEncodeSize );

			S3_TEXTURE Source, Destination;

			// Setup Source info
			memset( &Destination, 0, sizeof(S3_TEXTURE) );
			memset( &Source, 0, sizeof(S3_TEXTURE) );
			Source.lWidth = nWidth;
			Source.lHeight = nHeight;
			Source.pSurface = pSource;
			if ( nSrcFmt == SRC_FMT_R8G8B8)
			{
				Source.PixelFormat.nRedMask		= 0xff0000;
				Source.PixelFormat.nGreenMask	= 0x00ff00;
				Source.PixelFormat.nBlueMask	= 0x0000ff;
				Source.PixelFormat.nAlphaMask	= 0x000000;
				Source.PixelFormat.nARGBBitCount = 24;
				Source.PixelFormat.nFlags = 0;
				Source.lPitch = nWidth * 3;
			}
			else
			{
				Source.PixelFormat.nRedMask		= 0xff000000;
				Source.PixelFormat.nGreenMask	= 0x00ff0000;
				Source.PixelFormat.nBlueMask	= 0x0000ff00;
				Source.PixelFormat.nAlphaMask	= 0x000000ff;
				Source.PixelFormat.nARGBBitCount = 32;
				Source.PixelFormat.nFlags = S3_TF_HASALPHA;
				Source.lPitch = nWidth * 4;
			}

			// Compress the texture
			if ( S3TC_Encode( &Source, &Destination, pCompressed, nEncodeFlag, NULL, NULL, NULL ) )
			{
				free( pCompressed );
				return NULL;
			}

			if ( nDestFmt == FGCDATA_TEXFMT_S3TCx2 )
			{
				// Let the requesting function know the size
				nResultSize = FMATH_BYTE_ALIGN_UP( nEncodeSize, 32 ) * 2;

				// Allocate memory to contain the compressed color image and the alpha image
				*pResult = malloc( nResultSize );

				_nTempTime = timeGetTime();
				_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pCompressed, nDestFmt, *pResult );
				_nSwizzleTime += timeGetTime() - _nSwizzleTime;

				// Generate the Alpha texture and append it to the end of the texture.

				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 4;

				// Move the Alpha channel to the green channel for max alpha range
				// and clear the other channels
				u8 *pGreen = (u8 *)pSource + 2;
				u8 *pAlpha = (u8 *)pSource;
				for ( u32 i = 0; i < nSourceImageSize; i += 4, pGreen += 4, pAlpha += 4 )
				{
					*pGreen = *pAlpha;	// Set Green to Alpha
					*pAlpha = 0;		// Clear the Alpha
					*(pAlpha + 1) = 0;	// Clear the Blue
					*(pGreen + 1) = 0;	// Clear the Red
				}

				// Compress the texture
				if ( S3TC_Encode( &Source, &Destination, pCompressed, nEncodeFlag, NULL, NULL, NULL ) )
				{
					free( pCompressed );
					return NULL;
				}

				_nTempTime = timeGetTime();
				_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pCompressed, nDestFmt, ((u8 *)*pResult + (nResultSize / 2)) );
				_nSwizzleTime += timeGetTime() - _nSwizzleTime;
			}
			else
			{
				// Let the requesting function know the size
				nResultSize = FMATH_BYTE_ALIGN_UP( nEncodeSize, 32 );

				// Allocate memory to contain the compressed image
				*pResult = malloc( nResultSize );

				_nTempTime = timeGetTime();
				_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pCompressed, nDestFmt, *pResult );
				_nSwizzleTime += timeGetTime() - _nSwizzleTime;
			}

			free( pCompressed );
#endif			
			break;
		}

		case FGCDATA_TEXFMT_I8:
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8A8 && nSrcFmt != SRC_FMT_R8G8B8 )
				return NULL;

			u32 i;

			if ( nSrcFmt == SRC_FMT_R8G8B8A8 )
			{
				// 32 BIT SOURCE DATA

				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 4;

				// Use the Alpha channel to build the I8 source image by compressing
				// all of the alpha values to the front of the texture:
				u8 *pTemp8 = (u8 *)pSource;
				u8 *pAlpha = (u8 *)pSource;
				for ( i = 0; i < nSourceImageSize; i += 4, pTemp8++, pAlpha += 4 )
					*pTemp8 = *pAlpha;
			}
			else
			{
				// 24 BIT SOURCE DATA

				// Calculate the size of the source image
				u32 nSourceImageSize = nWidth * nHeight * 3;

				// Use the Red channel to build the I8 source image by compressing
				// all of the red values to the front of the texture:
				u8 *pTemp8 = (u8 *)pSource;
				u8 *pRed   = (u8 *)pSource + 2;
				for ( i = 0; i < nSourceImageSize; i += 3, pTemp8++, pRed += 3 ) {
					*pTemp8 = *pRed;
				}
			}

			// Calculate the size of the resulting image
			nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight, 32 );

			// Allocate memory to contain the resulting image
			*pResult = malloc( nResultSize );

			_nTempTime = timeGetTime();
			_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
			_nSwizzleTime += timeGetTime() - _nSwizzleTime;

			break;
		}
		case FGCDATA_TEXFMT_IA8:
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8A8 )
				return NULL;

			// 32 BIT SOURCE DATA

			// Calculate the size of the source image
			u32 nSourceImageSize = nWidth * nHeight * 4;

			// Calculate the size of the resulting image
			nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight * 2, 32 );

			// Allocate memory to contain the resulting image
			*pResult = malloc( nResultSize );

			_nTempTime = timeGetTime();
			_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
			_nSwizzleTime += timeGetTime() - _nSwizzleTime;

			break;
		}
		case FGCDATA_TEXFMT_IA4:
		{
			if ( nSrcFmt != SRC_FMT_R8G8B8A8 )
				return NULL;

			// 32 BIT SOURCE DATA

			// Calculate the size of the source image
			u32 nSourceImageSize = nWidth * nHeight * 4;

			// Calculate the size of the resulting image
			nResultSize = FMATH_BYTE_ALIGN_UP( nWidth * nHeight, 32 );

			// Allocate memory to contain the resulting image
			*pResult = malloc( nResultSize );

			_nTempTime = timeGetTime();
			_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pSource, nDestFmt, *pResult );
			_nSwizzleTime += timeGetTime() - _nSwizzleTime;

			break;
		}

		default:
			nResultSize = 0;
			break;
	}
	
/*
	DEVPRINTF( "---------------------------\n", _nCompressTime );
	DEVPRINTF( "Compress Time     : %d\n", _nCompressTime );
	DEVPRINTF( "Swizzle Time      : %d\n", _nSwizzleTime );
	DEVPRINTF( "Total Convert Time: %d\n", timeGetTime() - nStartTime );
*/
	return nResultSize;
}


void WriteDTXnFile( DWORD count, void *buffer ) {
    // stubbed out so that we can use the nvidia library
}

void ReadDTXnFile( DWORD count, void *buffer ) {
    // stubbed out so that we can use the nvidia library
}

static HRESULT _DxtMipCallBack( void *pData, int nMipLevel, DWORD nSizeInBytes ) {
	
	FASSERT( (_nDxtByteOffset + nSizeInBytes) <= _nTotalDxtMemory );

	u8 *pDest = (u8 *)&_pDxtMemory[_nDxtByteOffset];
	fang_MemCopy( pDest, pData, nSizeInBytes );
	_nDxtByteOffset += nSizeInBytes;

	return 0;
}

BOOL _GenerateDxtFile( SrcFormat_e nSrcFmt,
					  u32 nWidth, u32 nHeight,
					  void *pSource,
					  FData_TexFmt_e nDestFmt,
					  void **pResult, 
					  u32 &rnResultSize ) {
	u32 nBytesPerPixel, nBytesAllocated, nNumPixels;
	u8 *pCompressed;
	u32 i, j;

	nBytesPerPixel = (nSrcFmt == SRC_FMT_R8G8B8) ? 3 : 4;
	FASSERT( nBytesPerPixel == 3 || nBytesPerPixel == 4 );
	nNumPixels = nWidth * nHeight;
	
	// setup the compression options
	CompressionOptions CompressOptions;
	CompressOptions.bMipMapsInImage = FALSE;
	CompressOptions.MipMapType = dNoMipMaps;
	CompressOptions.MIPFilterType = dMIPFilterBox;
	if( nBytesPerPixel == 3 ) {
		CompressOptions.bBinaryAlpha = 0;
	} else {
		// see if any pixel has any pixel other than 0 or 1	
		CompressOptions.bBinaryAlpha = 1;

		u8 *pSrcData = (u8 *)pSource;
		pSrcData += 3;
		for( i=0; i < nHeight; i++ ) {
			for( j=0; j < nWidth; j++ ) {
				if( *pSrcData != 0 || *pSrcData != 255 ) {
					CompressOptions.bBinaryAlpha = 0;
					break;
				}
				++pSrcData;
			}
			if( j != nWidth ) {
				break;
			}
		}
	}
	CompressOptions.bNormalMap = FALSE;
    CompressOptions.bDuDvMap = FALSE;
	CompressOptions.bAlphaBorder = FALSE;
    CompressOptions.bBorder = FALSE;
    CompressOptions.BorderColor.u = 0;
	CompressOptions.bFade = FALSE;
    CompressOptions.bFadeAlpha = FALSE;
    CompressOptions.FadeToColor.u = 0;
    CompressOptions.FadeAmount = 0;
	CompressOptions.bDitherColor = FALSE;
	CompressOptions.TextureType = dTextureType2D;
	CompressOptions.bSwapRGB = FALSE;
	
	switch( nDestFmt ) {
	
	case FGCDATA_TEXFMT_S3TC:
		CompressOptions.TextureFormat = dDXT1;
		nBytesAllocated = (nNumPixels >> 1);
		break;
	case FGCDATA_TEXFMT_S3TCA1:
		CompressOptions.TextureFormat = dDXT1a;
		nBytesAllocated = (nNumPixels >> 1);
		break;
	case FGCDATA_TEXFMT_S3TCx2:
		CompressOptions.TextureFormat = dDXT1;
		nBytesAllocated = (nNumPixels >> 1);
		break;
	}

	// allocate our memory
	pCompressed = (u8 *)malloc( nBytesAllocated );
	if( !pCompressed ) {
		return FALSE;
	}
	memset( pCompressed, 0, nBytesAllocated );

	// setup the vars used during the dxt callback process
	_pDxtMemory = pCompressed;
	_nDxtByteOffset = 0;
	_nTotalDxtMemory = nBytesAllocated;
	
	_nTempTime = timeGetTime();
	// compress
	if( nvDXTcompress( (unsigned char *)pSource, nWidth, nHeight, nWidth * nBytesPerPixel, &CompressOptions, nBytesPerPixel, _DxtMipCallBack ) ) {
		free( pCompressed );
		return FALSE;
	}
	_nCompressTime += timeGetTime() - _nTempTime;

	// swizzle
	if( nDestFmt != FGCDATA_TEXFMT_S3TCx2 ) {
		rnResultSize = FMATH_BYTE_ALIGN_UP( nBytesAllocated, 32 );
		*pResult = malloc( rnResultSize );
		if( !*pResult ) {
			free( pCompressed );
			return FALSE;
		}
		memset( *pResult, 0, rnResultSize );
		_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pCompressed, nDestFmt, *pResult );
	} else {
		u32 nSizePerHalf = (nNumPixels >> 1);
		CompressOptions.bBinaryAlpha = 0;
		rnResultSize = FMATH_BYTE_ALIGN_UP( nSizePerHalf, 32 ) * 2;
		*pResult = malloc( rnResultSize );
		if( !*pResult ) {
			free( pCompressed );
			return FALSE;
		}
		memset( *pResult, 0, rnResultSize );

		_nTempTime = timeGetTime();
		_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pCompressed, nDestFmt, *pResult );
		_nSwizzleTime += timeGetTime() - _nTempTime;

		// move the alpha to the green channel and zero out all other channels
		u8 *pSrc = (u8 *)pSource;
		for( i=0, j=0; i < nNumPixels; i++, j+=4 ) {
			pSrc[j] = 0;
			pSrc[j+1] = pSrc[j+3];
			pSrc[j+2] = 0;
			pSrc[j+3] = 0;
		}

		_nTempTime = timeGetTime();
		// compress
		_nDxtByteOffset = 0;		
		if( nvDXTcompress( (unsigned char *)pSource, nWidth, nHeight, nWidth * nBytesPerPixel, &CompressOptions, nBytesPerPixel, _DxtMipCallBack ) ) {
			free( pCompressed );
			free( *pResult );
			return FALSE;
		}	
		_nCompressTime += timeGetTime() - _nTempTime;
		
		_nTempTime = timeGetTime();
		_GCSwizzleTexData( nSrcFmt, nWidth, nHeight, pCompressed, nDestFmt, ((u8 *)*pResult + (rnResultSize >> 1)) );
		_nSwizzleTime += timeGetTime() - _nTempTime;
	}
	
	free( pCompressed );
	
	return TRUE;
}



void GC_Swizzle_RGBA8( s32 width, s32 height, void *pResult, const void *pSource, BOOL bFullAlpha );
static void GC_PackTile_RGBA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pResult, BOOL bFullAlpha );
static void GC_Swizzle_RGB5A3( s32 width, s32 height, void *pResult, const void *pSource, BOOL bFullAlpha );
static void GC_PackTile_RGB5A3( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pResult, BOOL bFullAlpha );
static void GC_Swizzle_R5G6B5( s32 width, s32 height, void *pResult, const void *pSource );
static void GC_PackTile_R5G6B5( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pResult );
static void GC_Swizzle_CMPR( s32 width, s32 height, void *pResult, const void *pSource );
static void GC_PackTile_CMPR( s32 width, s32 height, const void *pSource, s32 tileX, s32 tileY, u16 *pResult );
static void GC_Swizzle_I8( s32 width, s32 height, void *pResult, const void *pSource );
static void GC_PackTile_I8( s32 width, s32 height, const void *pSource, s32 x, s32 y, s32 bpp, u8 *pResult );
static void GC_Swizzle_IA8( s32 width, s32 height, void *pResult, const void *pSource );
static void GC_PackTile_IA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pResult );
static void GC_Swizzle_IA4( s32 width, s32 height, void *pResult, const void *pSource );
static void GC_PackTile_IA4( s32 width, s32 height, const void *pSource, s32 x, s32 y, s32 bpp, u8 *pResult );

//
//
//
void _GCSwizzleTexData( SrcFormat_e nSrcFmt, u32 w, u32 h, const void *pSource, FData_TexFmt_e nDstFmt, void *pResult )
{
	switch ( nDstFmt )
	{
		case FGCDATA_TEXFMT_R8G8B8A8:
			GC_Swizzle_RGBA8( w, h, pResult, pSource, FALSE );
			break;

		case FGCDATA_TEXFMT_R8G8B8X8:
			GC_Swizzle_RGBA8( w, h, pResult, pSource, TRUE );
			break;

		case FGCDATA_TEXFMT_R4G4B4A3:
			GC_Swizzle_RGB5A3( w, h, pResult, pSource, FALSE );
			break;

		case FGCDATA_TEXFMT_R5G5B5X1:
			GC_Swizzle_RGB5A3( w, h, pResult, pSource, TRUE );
			break;

		case FGCDATA_TEXFMT_R5G6B5:
			GC_Swizzle_R5G6B5( w, h, pResult, pSource );
			break;

		case FGCDATA_TEXFMT_S3TC:
		case FGCDATA_TEXFMT_S3TCA1:
		case FGCDATA_TEXFMT_S3TCx2:
			GC_Swizzle_CMPR(w, h, pResult, pSource );
			break;

		case FGCDATA_TEXFMT_I8:
			GC_Swizzle_I8(w, h, pResult, pSource);
			break;

		case FGCDATA_TEXFMT_IA8:
			GC_Swizzle_IA8(w, h, pResult, pSource);
			break;

		case FGCDATA_TEXFMT_IA4:
			GC_Swizzle_IA4(w, h, pResult, pSource);
			break;

		default:
			FASSERT_NOW;
			break;
	}
}


//
//
//
void GC_Swizzle_RGBA8( s32 width, s32 height, void *pResult, const void *pSource, BOOL bFullAlpha )
{
	u32 numTileRows, tileRow;	
	u32 numTileCols, tileCol;
	u8  *pDstPtr;
 	
	// Number of 4x4 texel tile cols, rows including any partial tiles
	numTileCols = ((width  + 3) >> 2);
	numTileRows = ((height + 3) >> 2);
	
	pDstPtr = (u8 *)pResult;
	
	// numTileRows, numTileCols includes any partial tiles
	for( tileRow=0; tileRow<numTileRows; tileRow++ )
	{
		for ( tileCol=0; tileCol<numTileCols; tileCol++)
		{
			GC_PackTile_RGBA8( width, height, pSource, (tileCol * 4), (tileRow * 4), pDstPtr, bFullAlpha );
			
			// Move to next 2 (32B) cache lines
			pDstPtr += 64;
		}	
	}
}


// 
// 4x4 tile, 16-bit texels
// x and y represent starting texel position of this tile
// pack AR in low half, GB in high half dest. buffer 
//
void GC_PackTile_RGBA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr, BOOL bFullAlpha )
{
	u32 row, col;

	// pDstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if ( realRows > 4 )
		realRows = 4;

	if ( realCols > 4 )
		realCols = 4;
			
	// pack 2 32B tiles
	for ( row = 0; row < realRows; row++ )
	{	
		// Pack 2 cache lines at once move 8 bytes (4 16-bit texels) per row
		// need to reset ptr each row to account for column padding
		u8 *pAR = pDstPtr +      (row * 8);
		u8 *pGB = pDstPtr + 32 + (row * 8);

		for ( col=0; col<realCols; col++ )
		{           
			u32 offset = ((y+row)*width + (x+col))*4;
			const u8* basePtr = (u8 *)pSource;
	
			u8 r = basePtr[offset];
			u8 g = basePtr[offset+1];
			u8 b = basePtr[offset+2];
			u8 a = basePtr[offset+3];

			if ( bFullAlpha )
				a = 255;

			// alpha is byte 0, red is byte 1
			*pAR       = a;	
			*(pAR + 1) = r;

			// green is byte 0, blue is byte 1
			*pGB       = g; 
			*(pGB + 1) = b;
			
			pAR += 2;
			pGB += 2;
		}
	}
}


//
// convert from layer to final hw format
// 4x4 texel tiles @ 8B per row, 32B per tile
//
//
void GC_Swizzle_RGB5A3( s32 width, s32 height, void *pResult, const void *pSource, BOOL bFullAlpha )
{	
	u32 tileRow, tileCol;
 		
	// number of 4x4 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 3) >> 2);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8*)(pResult);
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{
		for(tileCol=0; tileCol<numTileCols; tileCol++)
		{			
			GC_PackTile_RGB5A3( width, height, pSource, tileCol*4, tileRow*4, pDstPtr, bFullAlpha );
			pDstPtr += 32;                  // next 32B cache line
		}			
	} 	
}


//
//	4x4 tile, 16-bit texels
//	x and y represent starting texel position of this tile
//
void GC_PackTile_RGB5A3( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr, BOOL bFullAlpha )
{

	u32 row, col;

	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if ( realRows > 4 )
		realRows = 4;
		
	if ( realCols > 4 )
		realCols = 4;
				     			
	// pack 32B tile 
	for ( row = 0; row < realRows; row++ )
	{
		// Move 8 bytes (4 16-bit texels) per row
		// Need to reset ptr each row to account for column padding
		u16 *pTile = (u16 *)(pDstPtr + (row * 8));

		for ( col=0; col<realCols; col++ )
		{
			const u16 *basePtr = (u16 *)pSource;
			u16 color = basePtr[(y+row)*width + (x+col)];

			// For this format, we want ARGB so convert the source
			// RGBA to ARGB and store it away.
			if ( bFullAlpha )
			{
				*pTile =	0x80						// A
						| (((color>>11) & 0xff) << 10)	// R
						| (((color>> 6) & 0xff) <<  5)	// G
						| (((color>> 1) & 0xff) <<  0);	// B
			}
			else
			{
				*pTile =  (((color>>0) & 0x8f) << 12)	// A
						| (((color>>12) & 0xff) << 8)	// R
						| (((color>>8) & 0xff) <<  4)	// G
						| (((color>> 4) & 0xff) << 0);	// B
			}

			pTile++;
		}
	}
}


//
// convert from layer to final hw format
// 4x4 texel tiles @ 8B per row, 32B per tile
//
//
void GC_Swizzle_R5G6B5( s32 width, s32 height, void *pResult, const void *pSource )
{	
	u32 tileRow, tileCol;
 		
	// number of 4x4 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 3) >> 2);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8*)(pResult);
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{
		for(tileCol=0; tileCol<numTileCols; tileCol++)
		{			
			GC_PackTile_R5G6B5( width, height, pSource, tileCol*4, tileRow*4, pDstPtr );
			pDstPtr += 32;                  // next 32B cache line
		}			
	} 	
}


//
//	4x4 tile, 16-bit texels
//	x and y represent starting texel position of this tile
//
void GC_PackTile_R5G6B5( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr )
{

	u32 row, col;

	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if ( realRows > 4 )
		realRows = 4;
		
	if ( realCols > 4 )
		realCols = 4;
				     			
	// pack 32B tile 
	for ( row = 0; row < realRows; row++ )
	{
		// Move 8 bytes (4 16-bit texels) per row
		// Need to reset ptr each row to account for column padding
		u16 *pTile = (u16 *)(pDstPtr + (row * 8));

		for ( col=0; col<realCols; col++ )
		{
			const u16 *basePtr = (u16 *)pSource;
			u16 color = basePtr[(y+row)*width + (x+col)];

			*pTile = color;

			pTile++;
		}
	}
}


//
//
//
void GC_Swizzle_CMPR( s32 width, s32 height, void *pResult, const void *pSource )
{
	u32 tileRow, tileCol;
	u32 srcTileRows, srcTileCols;
	u16* pDstPtr;

	// each source tile is 4x4 texels, 8B
	srcTileRows   = ((height + 3) >> 2);
	srcTileCols   = ((width  + 3) >> 2);

	pDstPtr = (u16*)(pResult);

	// each dst tile is 2x2 source tiles, so move by 2 each iteration
	for(tileRow = 0; tileRow < srcTileRows; tileRow += 2 )
	{
		for(tileCol = 0; tileCol < srcTileCols; tileCol += 2 )
		{
			GC_PackTile_CMPR( width, height, pSource, tileCol, tileRow, pDstPtr );
			pDstPtr += 16; // 32B per dst tile, short ptr
		}
	}
}


//
//
//
static void FixCMPWord( u16 *data )
{
	u16 tmp;
	tmp = *data;

	// reverse tuple order within bytes
	*data = ( (tmp & 0x3 )   << 6 ) |
			( (tmp & 0xC )   << 2 ) |
			( (tmp & 0x30)   >> 2 ) |
			( (tmp & 0xC0)   >> 6 ) |

            ( (tmp & 0x300 ) << 6 ) |
			( (tmp & 0xC00 ) << 2 ) |
			( (tmp & 0x3000) >> 2 ) |
			( (tmp & 0xC000) >> 6 ) ;
}


//
//
//
void GC_PackTile_CMPR( s32 width, s32 height, const void *pSource, s32 tileX, s32 tileY, u16 *pDstPtr )
{
	u32  x, y;
	u16* srcPtr;
	u16  tmp;
	u32  srcTileOffset;
	u32  subTileRows, subRowShorts;    // number of s3 4x4 tiles
	u32  srcPadWidth, srcPadHeight;
	u16* buffPtr;

	// set the padded size of the s3 source image out to a 4-texel boundary
	srcPadWidth  = ( (width  + 3) >> 2 );
	srcPadHeight = ( (height + 3) >> 2 );

	// number of bytes in a single row of 4x4 texel source tiles
	srcTileOffset = srcPadWidth * 8;

	// number of 4x4 (source) tile rows to copy ( will be 1 or 2 )
	subTileRows = 2;
	if( (srcPadHeight - tileY) < 2 )
		subTileRows = 1;

	// number of 4x4 tile cols to copy translated into number of short values
	// ( will be 4 or 8 )
	subRowShorts = 8;
	if( (srcPadWidth - tileX) < 2 )
		subRowShorts = 4;

	for( y=0; y < subTileRows; y++ )
	{
		srcPtr  = (u16*)( (u8*)(pSource) + ((tileY + y) * srcTileOffset) + (tileX*8) ); 
		buffPtr = ( pDstPtr + (y * 8) );        // 16 bytes per subRow = 8 shorts

		// process one or both 4x4 row tiles at once- 4 short each
		for( x=0; x < subRowShorts; x++ )
		{			
			switch( x )
			{

			// color table entries - switch bytes within a 16-bit world only
			case 0:	
			case 1:
			case 4:
			case 5:
				tmp = fang_ConvertEndian( *srcPtr++ );
				*buffPtr++ = tmp;
				break;
			
			// 2-bit color tuples;
			// reverse tuple order within bytes of a word
			case 2:
			case 3:
			case 6:
			case 7:
				tmp = *srcPtr++;
				FixCMPWord( &tmp );
				*buffPtr++ = tmp;
				break;

			}
		}
	}
}


//
//
//
void GC_Swizzle_I8( s32 width, s32 height, void *pResult, const void *pSource )
{
	u32 tileRow, tileCol;

	// Number of 4x8 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 7) >> 3);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8*)pResult;
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{		
		for (tileCol=0; tileCol<numTileCols; tileCol++ )
		{						
			GC_PackTile_I8( width, height, pSource, (tileCol * 8), (tileRow * 4), 1, pDstPtr );
			pDstPtr += 32;                  // next 32B cache line
		}			
	}
}


//
//
//
static void GC_PackTile_I8( s32 width, s32 height, const void *pSource, s32 x, s32 y, s32 bpp, u8 *pDstPtr )
{
	u32 row, col;
    
	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if( realRows > 4)    
		realRows = 4;
		
	if(realCols > 8)
		realCols = 8;
		
	// pack 32B tile 
	for(row=0; row<realRows; row++)
	{	
		// Move 8 bytes (8 8-bit texels) per row.  Need 
		// to reset ptr each row to account for column padding
		u8 *pTile = pDstPtr + (row * 8);						

		for(col=0; col<realCols; col++)
		{
			u32 offset = ((y+row)*width + (x+col))*bpp;
			const u8 *basePtr = (u8 *)pSource;

			*pTile = basePtr[offset];
			pTile++;
		}				
	}				
}

/*
void TCWriteTplImage_IA8 ( TCLayer* colorLayer, TCLayer* alphaLayer, u8* tplBuffer )
{
	u32 numTileRows, tileRow;	
	u32 numTileCols, tileCol;
	u8* dstPtr;
	u32 width, height;


	width  = colorLayer->width;
	height = colorLayer->height;
 			
	// number of 4x4 texel tile cols, rows including any partial tiles
	numTileCols = ((width  + 3) >> 2);
	numTileRows = ((height + 3) >> 2);

	dstPtr = tplBuffer;
	
	// numTileRows, numTileCols includes any partial tiles
	for( tileRow=0; tileRow<numTileRows; tileRow++ )
	{		
		for(tileCol=0; tileCol<numTileCols; tileCol++)
		{			
			TCPackTile_IA8( colorLayer, alphaLayer, (tileCol * 4), (tileRow * 4), dstPtr);
			dstPtr += 32;                  // next 32B cache line
		}			
	} 	
}
	
static void TCPackTile_IA8 ( TCLayer* colorLayer, TCLayer* alphaLayer, u32 x, u32 y, u8* dstPtr)
{
	u16 ria;
	u8 g, b;
	u32 row, col;
	u32 realRows, realCols;
	u8* tilePtr;
	

	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	realRows = colorLayer->height - y;
	realCols = colorLayer->width  - x;
	
	if( realRows > 4)    
		realRows = 4;
		
	if(realCols > 4)
		realCols = 4;
			
	// pack 32B tile 
	for(row=0; row<realRows; row++)
	{	
		tilePtr = dstPtr + (row * 8);                       // move 8 bytes (4 16-bit texels) per row
		                                                    // need to reset ptr each row to account for
		                                                    // column padding
		for(col=0; col<realCols; col++)
		{
			if(alphaLayer)                                  // alpha is byte 0
			{
				TCGetLayerValue( alphaLayer, (x+col), (y+row), &ria, 0, 0 );	
					
				*tilePtr = (u8)ria;			
			}	
			else  // set byte 0 to max. alpha
			{
				*tilePtr = 0xFF;
			}
					                                       // color is byte 1     
			TCGetLayerValue( colorLayer, (x+col), (y+row), &ria, &g, &b );

			// for LY_IMAGE_COLOR_CI16, use ria (index) directly as intensity value.
			// for LY_IMAGE_COLOR_RGB24, average the 3 color values
			if( colorLayer->type == LY_IMAGE_COLOR_RGB24 )
			{
				ria = ( ( ria + g + b ) / 3 );
			}

			*(tilePtr + 1) = (u8)ria;
			
			tilePtr += 2;		
		} // end for col loop			
	} // end for row loop
}
*/

//
//
//
void GC_Swizzle_IA8( s32 width, s32 height, void *pResult, const void *pSource )
{
	u32 tileRow, tileCol;
 		
	// number of 4x4 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 3) >> 2);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8*)(pResult);
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{
		for(tileCol=0; tileCol<numTileCols; tileCol++)
		{			
			GC_PackTile_IA8( width, height, pSource, tileCol*4, tileRow*4, pDstPtr );// width, height, pSource, tileCol*4, tileRow*4, pDstPtr );
			pDstPtr += 32;                  // next 32B cache line
		}			
	} 
}


//
//
//
static void GC_PackTile_IA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr )
{
	u32 row, col;
	u32 realRows, realCols;
	u16* pTile;
	u8* pData;
	

	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	realRows = height - y;
	realCols = width  - x;
	
	if( realRows > 4)    
		realRows = 4;
		
	if(realCols > 4)
		realCols = 4;
			
	// pack 32B tile 
	for(row=0; row<realRows; row++)
	{	
		pTile = (u16*)(pDstPtr + (row * 8));                       // move 8 bytes (4 16-bit texels) per row
				                                                    // need to reset ptr each row to account for
						                                            // column padding
		for(col=0; col<realCols; col++)
		{
			u32 offset = ((y+row)*width + (x+col))*4;
			const u8* basePtr = (u8 *)pSource;
	
			u8 r = basePtr[offset];
			u8 g = basePtr[offset+1];
			u8 b = basePtr[offset+2];
			u8 a = basePtr[offset+3];

			pData = (u8*)pTile;

			pData[0] = a;
			pData[1] = r;
			pTile++;
		} // end for col loop			
	} // end for row loop
}

//
//
//
void GC_Swizzle_IA4( s32 width, s32 height, void *pResult, const void *pSource )
{
	u32 tileRow, tileCol;

	// Number of 4x8 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 7) >> 3);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8*)pResult;
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{		
		for (tileCol=0; tileCol<numTileCols; tileCol++ )
		{						
			GC_PackTile_I8( width, height, pSource, (tileCol * 8), (tileRow * 4), 1, pDstPtr );
			pDstPtr += 32;                  // next 32B cache line
		}			
	}
}


//
//
//
static void GC_PackTile_IA4( s32 width, s32 height, const void *pSource, s32 x, s32 y, s32 bpp, u8 *pDstPtr )
{
	u32 row, col;
    
	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if( realRows > 4)    
		realRows = 4;
		
	if(realCols > 8)
		realCols = 8;
		
	// pack 32B tile 
	for(row=0; row<realRows; row++)
	{	
		// Move 8 bytes (8 8-bit texels) per row.  Need 
		// to reset ptr each row to account for column padding
		u8 *pTile = pDstPtr + (row * 8);						

		for(col=0; col<realCols; col++)
		{
			u32 offset = ((y+row)*width + (x+col))*bpp;
			const u8 *basePtr = (u8 *)pSource;

			*pTile = basePtr[offset];
			pTile++;
		}				
	}				
}
