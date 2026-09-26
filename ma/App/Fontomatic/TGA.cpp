//////////////////////////////////////////////////////////////////////////////////////
// tga.c - "reads and writes 32bit tgas, using the ImageData struct" 
//
// Author: Michael Starich   
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
// 07/29/99 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "tga.h"
#include "fang.h"
#include "fclib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//====================
// private definitions

#define _TGA_SIGNATURE				"TRUEVISION-XFILE.\0"
#define _TGA_HEADER_SIZE_IN_BYTES	18

#pragma pack( push, 1 )

// TGA File Header (must not be padded to be 32 bit aligned)
typedef struct
{                           
    u8 nIdLength;           // Image ID Field Length
    u8 nColorMapType;       // Color Map Type
    u8 nImageType;          // Image Type
    // Color Map Specification
    u16 nColorMapIndex;		// First Entry Index
    u16 nColorMapLength;	// Color Map Length
    u8 nColorMapEntrySize;  // Color Map Entry Size
    // Image Specification
    u16 nX_Origin;			// X-origin of Image
    u16 nY_Origin;			// Y-origin of Image
    u16 nImageWidth;		// Image Width
    u16 nImageHeight;		// Image Height
    u8 nPixelDepth;         // Pixel Depth
    u8 nImagDesc;           // Image Descriptor
} TgaHeader;

// TGA File Footer
typedef struct
{
    u32 nExtensionOffset;	// Extension Area Offset
    u32 nDeveloperOffset;	// Developer Directory Offset
    char pszSignature[18];	// Signature, dot, and NULL
} TgaFooter;

#pragma pack( pop )

//=================
// public variables

//==================
// private variables

static TCHAR _pszErrorString[256];

//===================
// private prototypes

//=================
// public functions

BOOL tga_ModuleInit( void ) {

	_pszErrorString[0] = NULL;

	return TRUE;
}

// opens a tga ensures that it is "Photoshop" tga (aka lower left origin, 8 bit alpha, 
// no compression, no color palette, etc), allocs memory, flips the image so that the first
// pixel is the upper left cornor (easier to work with this way)
ImageData_t *tga_Get32BitImageDataFromFile( LPCTSTR pszFileName, ImageData_t *pImageData ) {
	FILE *pFile = NULL;
	TgaHeader Header;
	u32 nItemsRead;
	u32 *pHead = NULL;
	s32 i, j;

	// clear out pImageData
	pImageData->nHeight = pImageData->nWidth = 0;
	pImageData->pImageData = NULL;
	// open the file for reading
	pFile = _tfopen( pszFileName, _T( "rb" ) );
	if( !pFile ) {
		_tcscpy( _pszErrorString, _T( "TGA could not be opened for reading!" ) );
		return NULL;
	}
	// read the header so that we can validate this image
	nItemsRead = fread( &Header, _TGA_HEADER_SIZE_IN_BYTES, 1, pFile );
	if( nItemsRead != 1 ) {
		_tcscpy( _pszErrorString, _T( "TGA header could not be read!" ) );
		fclose( pFile );
		return NULL;
	}
	// ensure this is a 32 bit, non paletted, 8bit alpha, origin in the lower left, uncompressed
	// type image (Photoshop's 32 bit tga will do just fine)
	if( Header.nIdLength != 0 ||
		Header.nColorMapType != 0 ||
		Header.nImageType != 2 ||
		Header.nColorMapIndex != 0 ||
		Header.nColorMapLength != 0 ||
		Header.nX_Origin != 0 ||
		Header.nY_Origin != 0 ||
		Header.nColorMapEntrySize != 0 ||
		Header.nPixelDepth != 32 ||
		Header.nImagDesc != 8 ) {
		// something is wrong with this image, don't waste my motherfuckin time

		_tcscpy( _pszErrorString, _T( "TGA must be in Photoshop 32bit format!" ) );
		fclose( pFile );
		return NULL;
	}
	// allocate memory for image
	pHead = (u32 *)malloc( sizeof( u32 ) * Header.nImageWidth * Header.nImageHeight );
	if( !pHead ) {
		_tcscpy( _pszErrorString, _T( "Could not allocate memory for TGA data!" ) );
		fclose( pFile );
		return NULL;
	}
	// read the image into our memory( scanline by scanline converting our image to top up in memory)
	j = sizeof( u32 ) * Header.nImageWidth;
	for( i=(Header.nImageHeight-1); i >= 0; i-- ) {
		nItemsRead = fread( &pHead[i *  Header.nImageWidth], j, 1, pFile );
		if( nItemsRead != 1 ) {
			_tcscpy( _pszErrorString, _T( "Could not read TGA image data!" ) );
			fclose( pFile );
			free( pHead );
			return NULL;
		}
	}
	fclose( pFile );

	// fill in the image data struct and return it
	pImageData->nHeight = Header.nImageHeight;
	pImageData->nWidth = Header.nImageWidth;
	pImageData->pImageData = pHead;

	return pImageData;
}

// writes out the image contained in pImageData out to disk.  This writes out 
// a "Photoshop" 32bit tga, it is assumed that pImageData is 32bit so pImageData
// should only have been created by calling tga_Get32BitImageDataFromFile()
tga_WriteOut32BitImageDataToFile( LPCTSTR pszFileName, ImageData_t *pImageData ) {
	FILE *pFile = NULL;
	s32 i;
	TgaFooter Footer;
	TgaHeader Header;

	pFile = _tfopen( pszFileName, _T( "wb" ) );
    if( !pFile ) {
		_tcscpy( _pszErrorString, _T( "Could not open TGA for writing!" ) );
		return FALSE;
	}
	// fill in the header
	Header.nIdLength = 0;
    Header.nColorMapType = 0;
    Header.nImageType = 2;
    Header.nColorMapIndex = 0;
    Header.nColorMapLength = 0;
    Header.nColorMapEntrySize = 0;
    Header.nX_Origin = 0;
    Header.nY_Origin = 0;
    Header.nImageWidth = pImageData->nWidth;
    Header.nImageHeight = pImageData->nHeight;
    Header.nPixelDepth = 32;
    Header.nImagDesc = 0x8;
	// fill in the footer
	Footer.nExtensionOffset = 0;
	Footer.nDeveloperOffset = 0;
	strcpy( &Footer.pszSignature[0], _TGA_SIGNATURE );

	// copy the header to the file
	if( fwrite( &Header, _TGA_HEADER_SIZE_IN_BYTES, 1, pFile ) != 1 ) {
		_tcscpy( _pszErrorString, _T( "Could not write TGA header to file!" ) );
		fclose( pFile );
		return FALSE;
	}
	// copy the image to the file, line by line
	for( i=(pImageData->nHeight-1); i >= 0; i-- ) {
		if( fwrite( &pImageData->pImageData[i * pImageData->nWidth], sizeof( u32 ) * pImageData->nWidth, 1, pFile ) != 1 ) {
			_tcscpy( _pszErrorString, _T( "Could not write TGA data to file!" ) );
			fclose( pFile );
			return FALSE;
		}
	}
	// copy the footer to the file
	if( fwrite( &Footer, sizeof( TgaFooter ), 1, pFile ) != 1 ) {
		_tcscpy( _pszErrorString, _T( "Could not write TGA footer to file!" ) );
	}
	fclose( pFile );
	return TRUE;
}

// frees memory allocated when this image was read in
void tga_FreeImageData( ImageData_t *pImageData ) {
	if( !pImageData ) {
		return;
	}
	if( pImageData->pImageData ) {
		free( pImageData->pImageData );
		pImageData->pImageData = NULL;
	}
}

LPCTSTR tga_GetLastError( void ) {
	return _pszErrorString;	
}


//==================
// private functions
