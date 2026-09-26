//////////////////////////////////////////////////////////////////////////////////////
// fontpack.c - 
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
// 08/01/99 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "fontpack.h"
#include "tga.h"
#include "fontcut.h"
#include <string.h>
#include <stdlib.h>

//====================
// private definitions

#define PIXEL_PAD	2

//=================
// public variables

//==================
// private variables

static TCHAR _pszErrorString[256];
static u32 _TGAMemory[256 * 256];
static TCHAR _pszPathAndFilename[256];

//===================
// private prototypes

static void _Copy32BitPixelsToAndFromPixelRegions( u32 nFromX, u32 nFromY, u32 nToX, u32 nToY,
												   u32 nWidth, u32 nHeight, u32 *pFrom, u32 *pTo,
												   u32 nFromStride, u32 nToStride );
static void _ClearTGAToWhiteTransparent( void );
static void _EliminateXPixelsFrom256PixelRegion( u32 *pHd, u32 nWidth );

//=================
// public functions

BOOL fontpack_ModuleInit( void ) {
	
	_pszErrorString[0] = NULL;
	_pszPathAndFilename[0] = NULL;

	return TRUE;
}

// breaks up the font into packed tga(s).
// if sucessful, this function will return a valid PackData_t * and will have 
// created the number of tgas required to fit all of the letters 
PackData_t *fontpack_CreatePackedTGAs( const bool bWriteOutput, LPCTSTR pszBaseFileName, LPCTSTR pszPath, ImageData_t *pImage, FontData_t *pFont ) {
	u32 *pSortedIndexArray, nNumLetters, i, j, nTemp, x, y, nTotalPageCount, nTexPage, nHeight, nRows, nLettersOnCurrentPage;
	s32 nDiff, k;
	Letter_t *pHdLetter, *pLetter;
	PackData_t *pPack;
	TCHAR *pPtr;
	ImageData_t ImageData;
	PackedLetter_t *pPackedLetter;
	BOOL bFound;

	// make sure that the base filename is no more than 10 characters long
	if( _tcslen( pszBaseFileName ) > 10 ) {
		_tcscpy( _pszErrorString, _T( "Output TGA filename must be 10 characters long!" ) );
		return NULL;
	}
	if( !pImage || !pFont ) {
		_tcscpy( _pszErrorString, _T( "Must have valid image and font data to create new TGAs!" ) );
		return NULL;
	}
	// allocate some temp memory for working
	nNumLetters = pFont->nNumLetters;
	pSortedIndexArray = (u32 *)malloc( nNumLetters * sizeof( u32 * ) );
	if( !pSortedIndexArray ) {
		_tcscpy( _pszErrorString, _T( "Could not allocate memory for sorting array!" ) );
		return NULL;
	}
	for( i=0; i < nNumLetters; i++ ) {
		pSortedIndexArray[i] = i;
	}
	// bubble sort based on height
	pHdLetter = pFont->pLetterArray;
	for( i=0; i < nNumLetters-1; i++ ) {
		for( j=i+1; j < nNumLetters; j++ ) {
			if( pHdLetter[ pSortedIndexArray[i] ].nHeight < pHdLetter[ pSortedIndexArray[j] ].nHeight ) {
				nTemp = pSortedIndexArray[i];
				pSortedIndexArray[i] = pSortedIndexArray[j];
				pSortedIndexArray[j] = nTemp;
			}
		}
	}
	// play with the list to try to complete whole rows better
	i = 0;
	x = PIXEL_PAD;
	while( i < nNumLetters ) {
		x += pHdLetter[ pSortedIndexArray[i] ].nWidth + PIXEL_PAD;
		if( x >= 256 ) {
			// we passed the row limit, see if we got the best last letter possible
			x -= pHdLetter[ pSortedIndexArray[i] ].nWidth + PIXEL_PAD;
			nDiff = 255 - x;
			bFound = FALSE;
			// now see if we have have an nTemp sized letter
			while( nDiff > 0 && !bFound ) {
				for( j=(i+1); j < nNumLetters; j++ ) {
					if( (pHdLetter[ pSortedIndexArray[j] ].nWidth + PIXEL_PAD) == (u32)nDiff ) {
						// we found a perfect fit
						bFound = TRUE;
						break;
					}
				}
				nDiff--;
			}
			if( bFound ) {
				// move j into i and slide everything else down
				nTemp = pSortedIndexArray[j];
				for( k=j; k > (s32)i; k-- ) {
					pSortedIndexArray[k] = pSortedIndexArray[k-1];
				}
				pSortedIndexArray[i] = nTemp;
				
				// reset vars for another row
				x += pHdLetter[ pSortedIndexArray[i] ].nWidth + PIXEL_PAD;
			} else {
				x = PIXEL_PAD + (pHdLetter[ pSortedIndexArray[i] ].nWidth + PIXEL_PAD);
			}			
		}
		i++;
	}

	// determine our memory needs
#if 0	// this method was flawed in that it didn't account for wasted space at the end of each texture page,
		//  which could add up to think that all letters would fit when really another page was needed
	x = PIXEL_PAD;
	nTemp = 256;
	y = PIXEL_PAD + pHdLetter[ pSortedIndexArray[0] ].nHeight + PIXEL_PAD;
	for( i=0; i < nNumLetters; i++ ) {
		x += pHdLetter[ pSortedIndexArray[i] ].nWidth + PIXEL_PAD;
		if( x >= nTemp ) {
			nTemp += 256;
			y += pHdLetter[ pSortedIndexArray[i] ].nHeight + PIXEL_PAD;
		}
	}
	nTotalPageCount = y >> 8;
	if( y%256 ) {
		nTotalPageCount++; 
	}
#else
	// the new method, which counts strips of letters and each time a new strip is needed, it verfies if a new page is needed
	u32 nStripCount = 1;
	u32 nCurStripX = PIXEL_PAD;
	u32 nCurPageY = PIXEL_PAD + pHdLetter[ pSortedIndexArray[0] ].nHeight + PIXEL_PAD;
	u32 nNumPages = 1;
	for( i=0; i < nNumLetters; i++ ) {
		nCurStripX += pHdLetter[ pSortedIndexArray[i] ].nWidth + PIXEL_PAD;
		if( nCurStripX >= 256 ) {
			// we've exceeded the max len of a strip, start another
			nStripCount++;
			nCurStripX = PIXEL_PAD + pHdLetter[ pSortedIndexArray[i] ].nWidth + PIXEL_PAD;
			nCurPageY += pHdLetter[ pSortedIndexArray[i] ].nHeight;
			if( nCurPageY >= 256 ) {
				nNumPages++;
				nCurPageY = PIXEL_PAD + pHdLetter[ pSortedIndexArray[i] ].nHeight + PIXEL_PAD;	
			}
		}
	}
	nTotalPageCount = nNumPages;
#endif
	
	// allocate memory for PackData_t struct
	nTemp = sizeof( PackData_t ) + 
			(nNumLetters * sizeof( PackedLetter_t)) + 
			(16 * sizeof( TCHAR ) * nTotalPageCount) + 
			(nTotalPageCount * sizeof( TCHAR *));
	pPack = (PackData_t *)malloc( nTemp );
	fang_MemZero( pPack, nTemp );
	if( !pPack ) {
		free( pSortedIndexArray );
		_tcscpy( _pszErrorString, _T( "Could not allocate memory for font packing data!" ) );
		return NULL;
	}
	pPack->pOriginalData = pFont;
	pPack->pPackedLetterArray = (PackedLetter_t *)( (u32)pPack + sizeof( PackData_t ) );
	pPack->nNumPages = nTotalPageCount;

	pPack->pauHeight = (u16 *)malloc( pPack->nNumPages * sizeof( u16 ) );
	if( !pPack->pauHeight ) {
		free( pSortedIndexArray );
		return NULL;
	}
	pPack->pauWidth = (u16 *)malloc( pPack->nNumPages * sizeof( u16 ) );
	if( !pPack->pauWidth ) {
		free( pSortedIndexArray );
		return NULL;
	}

	pPack->pszFilenames = (TCHAR **)( (u32)pPack->pPackedLetterArray + (nNumLetters * sizeof( PackedLetter_t)) );
	pPtr = (TCHAR *)( (u32)pPack->pszFilenames + (nTotalPageCount * sizeof( TCHAR ** )) );
	for( i=0; i < nTotalPageCount; i++ ) {
		pPack->pszFilenames[i] = pPtr;
		_tcscpy( pPtr, pszBaseFileName );
		pPtr[ _tcslen( pPtr ) ] = (TCHAR)( _T( '0' ) + i );
		_tcscat( pPtr, _T( ".tga" ) );
		pPtr += 16;
	}

	// fill in static tga with transparent white pixels
	_ClearTGAToWhiteTransparent();
	nTexPage = 0;
	x = y = PIXEL_PAD;
	nHeight = pHdLetter[ pSortedIndexArray[0] ].nHeight;
	nRows = 1;
	nLettersOnCurrentPage = 0;
	for( i=0; i < nNumLetters; i++ ) {
		pPackedLetter = &pPack->pPackedLetterArray[i];
		pLetter = &pHdLetter[ pSortedIndexArray[i] ];

		if( (x + pLetter->nWidth) < 256 ) {
			// there is enough room on the current row for this letter
			pPackedLetter->nUnPackedIndex = pSortedIndexArray[i];
			pPackedLetter->nTexturePageIndex = nTexPage;
			pPackedLetter->nLeft = x;
			pPackedLetter->nRight = x + pLetter->nWidth;
			pPackedLetter->nTop = y;
			pPackedLetter->nBottom = y + pLetter->nHeight;

			_Copy32BitPixelsToAndFromPixelRegions( pLetter->nLeft, pLetter->nTop,
												   x, y,
												   pLetter->nWidth, pLetter->nHeight,
												   pImage->pImageData, &_TGAMemory[0],
												   pImage->nWidth, 256 );
			x += pLetter->nWidth + PIXEL_PAD;
			nLettersOnCurrentPage++;
		} else {
			// start a new row
			x = PIXEL_PAD;
			y += nHeight + PIXEL_PAD;
			nHeight = pLetter->nHeight;
			if( (y+nHeight) < 256 ) {
				// there is enough room on the current page for this letter, lower i so that we redo this loop
				nRows++;
				i--;
			} else {
				// we need a new texture page, write out the current one, 
				// after we make sure that it is the smallest we can afford
				if( nRows <= 1 ) {
					// can we eliminate some x pixels
					if( x < 128 ) {
						j = 128;
						while( j > x && j > 8 ) {
							j >>= 1;
						}
						x = j << 1;
						_EliminateXPixelsFrom256PixelRegion( &_TGAMemory[0], x );
					} else {
						x = 256;
					}
				} else {
					x = 256;
				}
				if( y < 128 ) {
					j = 128;
					while( j > y && j > 8 ) {
						j >>= 1;
					}
					y = j << 1;
				} else {
					y = 256;
				}
				ImageData.pImageData = &_TGAMemory[0];
				ImageData.nWidth = (u16)x;
				ImageData.nHeight = (u16)y;

				_tcscpy( _pszPathAndFilename, pszPath );
				_tcscat( _pszPathAndFilename, pPack->pszFilenames[ nTexPage ] );
				pPack->pauHeight[ nTexPage ] = ImageData.nHeight;
				pPack->pauWidth[ nTexPage ] = ImageData.nWidth;

				if( bWriteOutput )
				{
					if( !tga_WriteOut32BitImageDataToFile( _pszPathAndFilename, &ImageData ) ) {
						free( pSortedIndexArray );
						free( pPack );
						_tcscpy( _pszErrorString, tga_GetLastError() );
						return NULL;	
					}
				}

				// reset our vars so that we can redo this loop on the next texture page 
				nTexPage++;
				x = y = PIXEL_PAD;
				nRows = 1;
				nLettersOnCurrentPage = 0;
				_ClearTGAToWhiteTransparent();
				
				i--;
			}
		}
	}
	// see if we have one last texture page to write out to disk
	if( nLettersOnCurrentPage ) {
		if( nRows <= 1 ) {
			// can we eliminate some x pixels
			if( x < 128 ) {
				j = 128;
				while( j > x && j > 8 ) {
					j >>= 1;
				}
				x = j << 1;
				_EliminateXPixelsFrom256PixelRegion( &_TGAMemory[0], x );
			} else {
				x = 256;
			}
		} else {
			x = 256;
		}
		y += nHeight + PIXEL_PAD;
		if( y < 128 ) {
			j = 128;
			while( j > y && j > 8 ) {
				j >>= 1;
			}
			y = j << 1;
		} else {
			y = 256;
		}
		ImageData.pImageData = &_TGAMemory[0];
		ImageData.nWidth = (u16)x;
		ImageData.nHeight = (u16)y;
		
		_tcscpy( _pszPathAndFilename, pszPath );
		_tcscat( _pszPathAndFilename, pPack->pszFilenames[ nTexPage ] );
		pPack->pauHeight[ nTexPage ] = ImageData.nHeight;
		pPack->pauWidth[ nTexPage ] = ImageData.nWidth;
		if( bWriteOutput )
		{
			if( !tga_WriteOut32BitImageDataToFile( _pszPathAndFilename, &ImageData ) ) {
				free( pSortedIndexArray );
				free( pPack );
				_tcscpy( _pszErrorString, tga_GetLastError() );
				return NULL;	
			}
		}
	}	
	
	free( pSortedIndexArray );
	return pPack;
}

void fontpack_FreePackData( PackData_t *pPack ) {
	if( !pPack ) {
		return;
	}
	free( pPack->pauHeight );
	free( pPack->pauWidth );
	free( pPack );
	pPack = NULL;
}

LPCTSTR fontpack_GetLastError( void ) {
	return _pszErrorString;
}

//==================
// private functions

static void _Copy32BitPixelsToAndFromPixelRegions( u32 nFromX, u32 nFromY,
												   u32 nToX, u32 nToY,
												   u32 nWidth, u32 nHeight,
												   u32 *pFrom, u32 *pTo,
												   u32 nFromStride, u32 nToStride ) {
	u32 i, j, *pTempFrom, *pTempTo, nFromOffset, nToOffset;
	
	nFromOffset = (nFromY * nFromStride) + nFromX;
	nToOffset = (nToY * nToStride) + nToX;
	for( i=0; i < nHeight; i++ ) {
		pTempFrom = &pFrom[ (i * nFromStride) + nFromOffset ];
		pTempTo = &pTo[ (i * nToStride) + nToOffset ];
		
		for( j=0; j < nWidth; j++ ) {
			pTempTo[j] = pTempFrom[j];
		}
	}
}

static void _ClearTGAToWhiteTransparent( void ) {
	u32 i;

	for( i=0; i < (256 * 256); i++ ) {
		_TGAMemory[i] = 0;
	}
}

static void _EliminateXPixelsFrom256PixelRegion( u32 *pHd, u32 nWidth ) {
	u32 i, j, *pTo, *pFrom;

	pFrom = &pHd[256];
	pTo = &pHd[nWidth];
	for( i=1; i < 256; i++ ) {
		for( j=0; j < nWidth; j++ ) {
			pTo[j] = pFrom[j];
		}
		pFrom += 256;
		pTo += nWidth;
	}
}