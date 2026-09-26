//////////////////////////////////////////////////////////////////////////////////////
// ScreenGrab.c - screen capture utility
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
// 04/27/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "Stdafx.h"
#include "fang.h"
#include "ScreenGrab.h"
#include "fvid.h" 
#include "fclib.h"
#include <stdio.h>
#include "ijl.h"
 
//====================
// private definitions

#pragma comment(lib, "ijl15l.lib")

#define _TGA_SIGNATURE				"TRUEVISION-XFILE.\0"
#define _TGA_HEADER_SIZE_IN_BYTES	18
#define _TGA_FOOTER_SIZE_IN_BYTES	26

#pragma pack( push, 1 )

// TGA File Header (must not be padded to be 32 bit aligned)
typedef struct {                           
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
} TgaHeader_t;

// TGA File Footer
typedef struct {
    u32 nExtensionOffset;	// Extension Area Offset
    u32 nDeveloperOffset;	// Developer Directory Offset
    char pszSignature[18];	// Signature, dot, and NULL
} TgaFooter_t;

#pragma pack( pop )

//=================
// public variables

//==================
// private variables

static char _pszFullFileName[256];
static char _pszPathName[200];
static char _pszFileName[32];
static u32 _nSFileNum = 0;
static u32 _nMFileSetNum = 0;
static u32 _nMFileNum = 0;
static u32 _nLastFrameNum = 0;
static char *_pImageBuffer = NULL;
static u32 _nImageBufferBytes = 0;
static char *_pDiskImage = NULL;
static u32 _nNumPixels;
static u32 _nNumPixelBytes;
static BOOL _bSystemReady = FALSE;
static TgaHeader_t _TGAHeader;
static TgaFooter_t _TGAFooter;

//===================
// private prototypes

static void _Reset();
static cchar *_CreateFullFileName( BOOL bSingle );
static BOOL _SaveAsJPEG( cchar *pszPathName, const u8 *pn24BitImageData );
static void _ConvertToJpegFilename( void );

//=================
// public functions

BOOL screengrab_Init( cchar *pszDirectory ) {
	FILE *pFile;

	_Reset();

	if( !pszDirectory ) {
		return FALSE;
	}
	// get the size of the buffer needed
	_nImageBufferBytes = fvid_GetScreenGrabBufferSizeInBytes();
	if( _nImageBufferBytes == 0 ) {
		return FALSE;
	}
	// allocate enough memory to hold the screen shot
	_pImageBuffer = (char *)fang_Malloc( _nImageBufferBytes, 0 );
	if( !_pImageBuffer ) {
		return FALSE;
	}
	// allocate enough memory to hold the disk image
	_nNumPixels = _nImageBufferBytes >> 2;	
	_nNumPixelBytes = _nNumPixels * 3;
	_pDiskImage = (char *)fang_Malloc( _nNumPixelBytes, 0 );
	if( !_pDiskImage ) {
		fang_Free( _pImageBuffer );
		_pImageBuffer = NULL;
		return FALSE;
	}
	// copy our directory
	fclib_strcpy( _pszPathName, pszDirectory );
	// search _pszPathName for the last single & multi filename
	_nSFileNum = 0;
	while( TRUE ) {	
		_nSFileNum++;
		// try to open the tga file
		pFile = fopen( _CreateFullFileName( TRUE ), "rb" );
		if( pFile ) {
			fclose( pFile );
			continue;
		}
		
		// try to open the jpeg file
		_ConvertToJpegFilename();
		pFile = fopen( _pszFullFileName, "rb" );
		if( pFile ) {
			fclose( pFile );
			continue;
		}
		
		// once we fail we have found the filename
		break;
	}		
	_nMFileNum = 1;
	_nMFileSetNum = 0;
	while( TRUE ) {	
		_nMFileSetNum++;
		// try to open the tga file
		pFile = fopen( _CreateFullFileName( FALSE ), "rb" );
		if( pFile ) {
			fclose( pFile );
			continue;
		}
		
		// try to open the jpeg file
		_ConvertToJpegFilename();
		pFile = fopen( _pszFullFileName, "rb" );
		if( pFile ) {
			fclose( pFile );
			continue;
		}
		
		// once we fail we have found the filename
		break;
	}
	// setup our header
	_TGAHeader.nIdLength = 0;
    _TGAHeader.nColorMapType = 0;
    _TGAHeader.nImageType = 2;
    _TGAHeader.nColorMapIndex = 0;
    _TGAHeader.nColorMapLength = 0;
    _TGAHeader.nColorMapEntrySize = 0;
    _TGAHeader.nX_Origin = 0;
    _TGAHeader.nY_Origin = 0;
    _TGAHeader.nImageWidth = FVid_Mode.nPixelsAcross;
    _TGAHeader.nImageHeight = FVid_Mode.nPixelsDown;
    _TGAHeader.nPixelDepth = 24;
    _TGAHeader.nImagDesc = 0x20;// flip the image
	// fill in the footer
	_TGAFooter.nExtensionOffset = 0;
	_TGAFooter.nDeveloperOffset = 0;
	fclib_strcpy( _TGAFooter.pszSignature, _TGA_SIGNATURE );

	_nLastFrameNum = 0;
	_bSystemReady = TRUE;
	
	return TRUE;
}

void screengrab_Shutdown() {
	_Reset();
}

BOOL screengrab_CaptureTheScreen( BOOL bSingle, ScreenGrabOutputType_e nOutputType/*=SCREEN_GRAB_OUTPUT_TYPE_TGA*/ ) {
	FILE *pFile;
	u32 i;
	char *pSrc, *pDest;
	BOOL bReturnVal;
	
	if( !_bSystemReady ) {
		return FALSE;
	}
	if( nOutputType >= SCREEN_GRAB_OUTPUT_TYPE_COUNT ) {
		// default to TGA
		nOutputType = SCREEN_GRAB_OUTPUT_TYPE_TGA;
	}
	if( !fvid_ScreenGrab( _pImageBuffer, _nImageBufferBytes ) ) {
		return FALSE;
	}
	if( !bSingle ) {
		if( _nLastFrameNum ) {
			// see if we should continue the last set or start a new one
			if( FVid_nFrameCounter != (_nLastFrameNum+1) ) {
				// start a new multi set
				++_nMFileSetNum;
				_nMFileNum = 1;
			}
		}
		// record the frame number
		_nLastFrameNum = FVid_nFrameCounter;
	}	
	_CreateFullFileName( bSingle );
	if( !bSingle ) {
		// advance the multi file num
		_nMFileNum++;
	} else {
		_nSFileNum++;
	}
	// create our memory image
	pDest = _pDiskImage;
	pSrc = _pImageBuffer;
	for( i=0; i < _nImageBufferBytes; i+=4 ) {
		// b
		*pDest = *pSrc;
		pDest++;
		pSrc++;
		// g
		*pDest = *pSrc;
		pDest++;
		pSrc++;
		// r
		*pDest = *pSrc;
		pDest++;
		pSrc += 2;// advance past alpha as well		
	}
	
	switch( nOutputType ) {

	case SCREEN_GRAB_OUTPUT_TYPE_TGA:
		// write out a tga file
		pFile = fopen( _pszFullFileName, "wb" );
		if( !pFile ) {
			return FALSE;
		}
		// write out our file to disk
		fwrite( &_TGAHeader, _TGA_HEADER_SIZE_IN_BYTES, 1, pFile );
		fwrite( _pDiskImage, _nNumPixelBytes, 1, pFile );
		fwrite( &_TGAFooter, _TGA_FOOTER_SIZE_IN_BYTES, 1, pFile );

		fclose( pFile );

		bReturnVal = TRUE;
		break;

	case SCREEN_GRAB_OUTPUT_TYPE_JPEG:
		// write out a jpeg file
		_ConvertToJpegFilename();
		bReturnVal = _SaveAsJPEG( _pszFullFileName, (const u8 *)_pDiskImage );
		break;
	}

	return bReturnVal;
}

//==================
// private functions

static void _Reset() {
	_bSystemReady = FALSE;
	fang_MemZero( _pszFullFileName, 256 );
	fang_MemZero( _pszPathName, 200 );
	fang_MemZero( _pszFileName, 32 );
	_nSFileNum = 0;
	_nMFileSetNum = 0;
	_nMFileNum = 0;
	_nLastFrameNum = 0;
	if( _pImageBuffer ) {
		fang_Free( _pImageBuffer );
		_pImageBuffer = NULL;
	}
	if( _pDiskImage ) {
		fang_Free( _pDiskImage );
		_pDiskImage = NULL;
	}
	_nNumPixels = 0;
	_nNumPixelBytes = 0;

	_nImageBufferBytes = 0;
}

// creates an 8.3 name and stores the full file name into _pszFullFileName
static cchar *_CreateFullFileName( BOOL bSingle ) {
	if( bSingle ) {
		sprintf( _pszFileName, "S%07d.TGA", _nSFileNum );
		sprintf( _pszFullFileName, "%s\\%s", _pszPathName, _pszFileName );
	} else {
		sprintf( _pszFileName, "M%02d%05d.TGA", _nMFileSetNum, _nMFileNum );
		sprintf( _pszFullFileName, "%s\\%s", _pszPathName, _pszFileName );
	}
	return _pszFullFileName;
}

static BOOL _SaveAsJPEG( cchar *pszPathName, const u8 *pn24BitImageData ) {
    JPEG_CORE_PROPERTIES image;

    ZeroMemory( &image, sizeof( JPEG_CORE_PROPERTIES ) );

	if( ijlInit( &image ) != IJL_OK ) {
		DEVPRINTF( "Can't initialize Intel(R) JPEG library\n" );
		return FALSE;
	}

    // Setup DIB
	image.DIBWidth			= FVid_Mode.nPixelsAcross;
    image.DIBHeight			= FVid_Mode.nPixelsDown;
    image.DIBBytes			= (unsigned char *)pn24BitImageData;
							
    // Setup JPEG			
    image.JPGFile			= pszPathName;
    image.JPGWidth			= FVid_Mode.nPixelsAcross;
    image.JPGHeight			= FVid_Mode.nPixelsDown;

	image.DIBColor			= IJL_BGR;
	image.DIBChannels		= 3;
	image.DIBPadBytes		= 0;
	image.JPGColor			= IJL_YCBCR;
	image.JPGChannels		= 3;
	image.JPGSubsampling	= IJL_422;
	image.jquality			= 100;

	image.jprops.upsampling_type = IJL_TRIANGLE_FILTER; 
	image.jprops.progressive_found = 1; 
        
	if( ijlWrite( &image, IJL_JFILE_WRITEWHOLEIMAGE ) != IJL_OK ) {
		DEVPRINTF( "Can't write image\n" );
		ijlFree( &image );	
		return FALSE;
	}
	
	if( ijlFree( &image ) != IJL_OK ) {
		DEVPRINTF( "Can't free Intel(R) JPEG library\n" );
		return FALSE;
	}

    return TRUE;
}

static void _ConvertToJpegFilename( void ) {
	s32 nLen = fclib_strlen( _pszFullFileName );
	if( nLen > 3 ) {
		// assume that the last 3 characters are the extension
		_pszFullFileName[nLen-3] = 'J';
		_pszFullFileName[nLen-2] = 'P';
		_pszFullFileName[nLen-1] = 'G';
	}
}
 