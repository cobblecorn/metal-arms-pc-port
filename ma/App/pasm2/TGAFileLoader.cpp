//////////////////////////////////////////////////////////////////////////////////////
// TGAFileLoader.cpp -
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
#include "TGAFileLoader.h"
#include "fmath.h"
#include "ErrorLog.h"

#define _TGA_HEADER_BYTE_COUNT	18
#define _ERROR_HEADING			"TGA FILE LOADER "

#ifdef _MMI_TARGET_PS2
extern BOOL bAlpha;
#endif

CTgaFileLoader::CTgaFileLoader() {
	m_pImage = NULL;
	m_pMemAllocated = NULL;
	m_nBytes = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nBitsPerPixel = 0;
	m_nNumCharsInIdField = 0;
	m_nPaletteType = 0;
	m_nImageTypeCode = 0;
	m_nPaletteOrigin = 0;
	m_nPaletteLength = 0;
	m_nPaletteEntry = 0;
	m_nImageXOrigin = 0;
	m_nImageYOrigin = 0;
	m_nImageDescriptor = 0;
	m_nAlphaBits = 0;
	m_bInvertedImage = FALSE;
	m_sLastFileLoaded = "";
}

CTgaFileLoader::~CTgaFileLoader() {
	UnloadFile();
}

BOOL CTgaFileLoader::LoadTgaFile( cchar *pszFilename, BOOL bInvertAlpha, BOOL bConvertTo24BitIntensityMap/*=FALSE*/ ) {
	FILE *pFile;
	u32 nBytesPerTexel, nBytesRead, i, nNumPixels, nSum;
	u8 *pnPixel, *pnAlpha;

#ifdef _MMI_TARGET_PS2
    u32 nNeededBytes;
    u8 *OrigPixel, *NewPixel;
#endif

	if( !pszFilename ) {
		return FALSE;
	}
	// unload any file that may be loaded right now
	UnloadFile();

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	CString sErrorHeading = _ERROR_HEADING;
	sErrorHeading += pszFilename;

	pFile = fopen( pszFilename, "rb" );
	if( pFile == NULL ) {
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Could not open file for reading" );
		return FALSE;
	}
	if( !ReadTGAHeader( pFile ) ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Could not read TGA file header" );
		return FALSE;
	}
	// check for some unsupported tga types
	if( m_nPaletteType ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "paletted TGAs are not supported" );
		return FALSE;
	}
	if( m_nImageTypeCode != 1 && m_nImageTypeCode != 2 ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Invalid image type code" );
		return FALSE;
	}
	if( m_nImageXOrigin != 0 || m_nImageYOrigin != 0 ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Invalid TGA origin" );
		return FALSE;
	}
	if( !fmath_IsPowerOf2( m_nWidth, FALSE ) || !fmath_IsPowerOf2( m_nHeight, FALSE ) ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "TGA is not a power of 2" );
		return FALSE;
	}
	if( m_nBitsPerPixel != 24 && m_nBitsPerPixel != 32 ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "TGA is not 24 or 32 bit" );
		return FALSE;
	}
	if( m_nImageDescriptor & 0xc0 ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Invalid TGA image descriptor" );
		return FALSE;
	}

	switch( m_nBitsPerPixel ) {
	case 24:
		nBytesPerTexel = 3;

		if( m_nAlphaBits != 0 ) {
			fclose( pFile );
			rErrorLog.WriteErrorHeader( sErrorHeading );
			rErrorLog.WriteErrorLine( "Invalid number of alpha bits" );
			return FALSE;
		}
		break;

	case 32:
		nBytesPerTexel = 4;

		if( m_nAlphaBits != 8 ) {
	//		fclose( pFile );
	//		rErrorLog.WriteErrorHeader( sErrorHeading );
	//		rErrorLog.WriteErrorLine( "Invalid number of alpha bits" );
	//		return FALSE;
		}
		break;
	}

	nNumPixels = (m_nHeight * m_nWidth);

	m_nBytes = nNumPixels * nBytesPerTexel;
#ifdef _MMI_TARGET_PS2
	nNeededBytes = nNumPixels * 4; // NS Allocate for 32 bit data even if it is 24 bit as we'll convert to 32 bit
//    m_pImage = new char[nNeededBytes];
    m_pMemAllocated = new char[nNeededBytes + 16];
#else
	m_pMemAllocated = new char[m_nBytes + 16];
#endif
	if( !m_pMemAllocated ) {
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Could not allocate memory to hold TGA" );
		return FALSE;
	}
	ZeroMemory( m_pMemAllocated, m_nBytes + 16 );
	m_pImage = (void *)FMATH_BYTE_ALIGN_UP( (u32)m_pMemAllocated, 16 );
	// Position and read image...
	if( fseek( pFile, _TGA_HEADER_BYTE_COUNT + m_nNumCharsInIdField, SEEK_SET ) ) {
		delete[] (char *)m_pMemAllocated;
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Error reading TGA file" );
		return FALSE;
	}
	if( (nBytesRead = fread( m_pImage, 1, m_nBytes, pFile )) != m_nBytes ) {
		delete[] (char *)m_pMemAllocated;
		fclose( pFile );
		rErrorLog.WriteErrorHeader( sErrorHeading );
		rErrorLog.WriteErrorLine( "Error reading TGA file" );
		return FALSE;
	}

	fclose( pFile );

#ifdef _MMI_TARGET_PS2
	// NS If 24-bit data convert to 32-bit adding an alpha value of 0xFF
	if( nBytesPerTexel == 3 )
	{
		OrigPixel = (u8 *)m_pImage;
		OrigPixel += m_nBytes-3;
		NewPixel = (u8 *)m_pImage;
		NewPixel += nNeededBytes-4;

		for ( i=0; i< m_nBytes/3; i++ )
		{
			NewPixel[0] = OrigPixel[0];
			NewPixel[1] = OrigPixel[1];
			NewPixel[2] = OrigPixel[2];
            NewPixel[3] = 0xFF; // NS As there was no alpha in the 24-bit data set it to be full opaque
			NewPixel-=4;
			OrigPixel-=3;
		}

		nBytesPerTexel = 4;
		m_nBitsPerPixel = 32;
		m_nBytes = nNeededBytes;
		bAlpha = FALSE;
	}
//ARG - >>>>> set 32bit texture alpha to 255
	else {
		OrigPixel = (u8 *)m_pImage;

		bAlpha = FALSE;

        for ( i=0; i< m_nBytes/4; i++ )
		{
// NS Fix alpha to opaque          OrigPixel[3] = 0xFF;
//			OrigPixel[3] = 0xFF;
			if(OrigPixel[3] != 0xFF && OrigPixel[3] != 0)
				bAlpha = TRUE;

            OrigPixel += 4;
		}
	}
//ARG - <<<<<
#endif

	// Flip vertically, if need be...
	if( !m_bInvertedImage ) {
		FlipImage( nBytesPerTexel );
	}

	// invert alpha if requested
	if( bInvertAlpha && (m_nBitsPerPixel == 32) ) {
		u32 *pDst = (u32 *)m_pImage;

		for( i=0; i < nNumPixels; i++ ) {
			((u8 *)pDst)[3] = 255 - ((u8 *)pDst)[3];
			pDst++;
		}
	}

#ifndef _MMI_TARGET_PS2
	// convert to 24 bit intensity map format
	if( bConvertTo24BitIntensityMap ) {

		// will either (for 32bit images) copy the alpha map over the rgb values and will turn into a 24 bit image or
		// (for 24 bit images) will average the rgb and will copy that into the rgb values
		if( m_nBitsPerPixel == 24 ) {
			// average the rgb and overwrite whatever is store there now
			pnPixel = (u8 *)m_pImage;
			for( i=0; i < nNumPixels; i++ ) {
				nSum = pnPixel[0];
				nSum += pnPixel[1];
				nSum += pnPixel[2];
				nSum /= 3;
				pnPixel[0] = pnPixel[1] = pnPixel[2] = (u8)(nSum & 0x00FF);
				pnPixel += 3;
			}
		} else {
			// copy the alpha value into the rgb slot and then eliminate the alpha slot althogether
			pnPixel = (u8 *)m_pImage;
			pnAlpha = &pnPixel[3];
			for( i=0; i < nNumPixels; i++ ) {
				pnPixel[0] = pnPixel[1] = pnPixel[2] = *pnAlpha;
				pnPixel += 3;
				pnAlpha += 4;
			}
			// fixup the vars as if this had always been a 24 bit image
			m_nBytes = nNumPixels * 3;
			m_nBitsPerPixel = 24;
			m_nAlphaBits = 0;
		}
	}
#endif
	m_sLastFileLoaded = pszFilename;

	return TRUE;
}

BOOL CTgaFileLoader::Convert32bitAlphaImageIntoUnitFloat() {

	if( !m_pImage ) {
		return FALSE;
	}
	if( m_nBitsPerPixel != 32 ) {
		return FALSE;
	}

	u32 i;
	u8 *pnAlpha = (u8 *)m_pImage;
	pnAlpha += 3;
	f32 *pfFloat = (f32 *)m_pImage;
	f32 fValue;
	for( i=0; i < m_nBytes; i+=4 ) {
		fValue = (f32)pnAlpha[i];
		pfFloat[i>>2] = fValue * (1.0f/255.0f);
	}

	return TRUE;
}

void CTgaFileLoader::UnloadFile() {

	if( m_pMemAllocated ) {
		delete[] (char *)m_pMemAllocated;
		m_pMemAllocated = NULL;
		m_pImage = NULL;
	}
	m_sLastFileLoaded = "";
}

BOOL CTgaFileLoader::ReadTGAHeader( FILE *pFile ) {
	u8 acHeaderBuf[_TGA_HEADER_BYTE_COUNT];

	if( fread( acHeaderBuf, 1, _TGA_HEADER_BYTE_COUNT, pFile ) != _TGA_HEADER_BYTE_COUNT ) {
		return FALSE;
	}

	m_nNumCharsInIdField	= *((u8 *)&acHeaderBuf[0]);
	m_nPaletteType			= *((u8 *)&acHeaderBuf[1]);
	m_nImageTypeCode		= *((u8 *)&acHeaderBuf[2]);
	m_nPaletteOrigin		= *((u16 *)&acHeaderBuf[3]);
	m_nPaletteLength		= *((u16 *)&acHeaderBuf[5]);
	m_nPaletteEntry			= *((u8 *)&acHeaderBuf[7]);
	m_nImageXOrigin			= *((u16 *)&acHeaderBuf[8]);
	m_nImageYOrigin			= *((u16 *)&acHeaderBuf[10]);
	m_nWidth				= *((u16 *)&acHeaderBuf[12]);
	m_nHeight				= *((u16 *)&acHeaderBuf[14]);
	m_nBitsPerPixel			= *((u8 *)&acHeaderBuf[16]);
	m_nImageDescriptor		= *((u8 *)&acHeaderBuf[17]);
	m_nAlphaBits			= m_nImageDescriptor & 0x0f;
	m_bInvertedImage		= !!(m_nImageDescriptor & 0x20);

	return TRUE;
}

void CTgaFileLoader::FlipImage( u32 nBytesPerTexel ) {
	u32 i, j, k;
	u8 nTmpPix1, nTmpPix2;
	u32 nPixOffset1, nPixOffset2;
	u8 *pnImage = (u8 *)m_pImage;

	for( i=0; i<m_nHeight/2; i++ ) {
		for( j=0; j<m_nWidth; j++ ) {
			for( k=0; k<nBytesPerTexel; k++ ) {
				nPixOffset1 = i*m_nWidth*nBytesPerTexel + j*nBytesPerTexel + k;
				nPixOffset2 = (m_nHeight-1-i)*nBytesPerTexel*m_nWidth + j*nBytesPerTexel + k;

				nTmpPix1 = pnImage[ nPixOffset1 ];
				nTmpPix2 = pnImage[ nPixOffset2 ];
				pnImage[ nPixOffset2 ] = nTmpPix1;
				pnImage[ nPixOffset1 ] = nTmpPix2;
			}
		}
	}
}

u32 CTgaFileLoader::GetLargestDim() {

	if( m_nWidth >= m_nHeight ) {
		return m_nWidth;
	}
	return m_nHeight;
}

u32 CTgaFileLoader::GetSmallestDim() {

	if( m_nWidth <= m_nHeight ) {
		return m_nWidth;
	}
	return m_nHeight;
}
