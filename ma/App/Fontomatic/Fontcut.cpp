//////////////////////////////////////////////////////////////////////////////////////
// fontcut.c - "cuts up an ImageData image into seperate letter images" 
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
// 07/30/99 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "fontcut.h"
#include "tga.h"
#include <stdlib.h>
#include <string.h>

//====================
// private definitions

#define _TRANSPARENT_PIXEL_COLOR		0x00

//=================
// public variables

//==================
// private variables

static TCHAR _pszErrorString[256];

//===================
// private prototypes

static u32 _FindNextColumnWithAnOpaquePixel( ImageData_t *pImage, u32 nStartingCol );
static u32 _FindNextFullyTransparentColumn( ImageData_t *pImage, u32 nStartingCol );
static u32 _FindFirstNonTransparentRowFromTop( ImageData_t *pImage, u32 nStartingCol, u32 nEndingCol );
static u32 _FindLastFullyTransparentRowFromBottom( ImageData_t *pImage, u32 nStartingCol, u32 nEndingCol );
static u32 _FindFirstNonTransparentLineFromTop( ImageData_t *pImage );
static BOOL _TopBottomRightEdgesAreTransparent( ImageData_t *pImage );

//=================
// public functions

BOOL fontcut_ModuleInit( void ) {

	_pszErrorString[0] = NULL;

	return TRUE;
}

FontData_t *fontcut_CutUpImageIntoAFont( ImageData_t *pImage, FontData_t *pFont ) {
	u32 i, nAnchor, nNumChars, nLeft, nRight, nTop, nBottom, nTotalSpace, nMaxWidth, nMaxHeight;
	Letter_t *pLetter;

	if( !pImage || !pFont ) {
		_tcscpy( _pszErrorString, _T( "No valid image data to cut font out of!" ) );
		return NULL;
	}

	// scan file to see how many letters we have so that we can allocate enough 
	// memory for all of them

	// find the special pixel in the first column (BGRA	pixel format)
	nAnchor = _FindFirstNonTransparentRowFromTop( pImage, 0, 1 );
	if( !nAnchor ) {
		_tcscpy( _pszErrorString, _T( "No font anchor point!" ) );
		return NULL;
	}
	// make sure the first and last line, as well as the last column contains ONLY background pixels (alpha == _TRANSPARENT_PIXEL_COLOR)
	if( !_TopBottomRightEdgesAreTransparent( pImage ) ) {
		_tcscpy( _pszErrorString, _T( "Input TGA invalid: characters cannot touch edges of TGA!" ) );
		return NULL;
	}
	// find out how many letters there are in this image
	nNumChars = 0;
	nLeft = nRight = 1;
	while( nLeft ) {
		nLeft = _FindNextColumnWithAnOpaquePixel( pImage, nRight );
		nRight = _FindNextFullyTransparentColumn( pImage, nLeft );
		nTop = _FindFirstNonTransparentRowFromTop( pImage, nLeft, nRight );
		nBottom = _FindLastFullyTransparentRowFromBottom( pImage, nLeft, nRight );
		if( nLeft && nRight ) {
			nNumChars++;
		}
	}
	if( !nNumChars ) {
		_tcscpy( _pszErrorString, _T( "No letters, no font!" ) );
		return NULL;
	}
	// allocate memory for all of the letters
	pLetter = (Letter_t *)malloc( sizeof( Letter_t ) * nNumChars );
	if( !pLetter ) {
		_tcscpy( _pszErrorString, _T( "Could not allocate memory for letter data!" ) );
		return NULL;
	}
	// fill in our letter array
	nLeft = nRight = 1;
	nTotalSpace = nMaxWidth = nMaxHeight = 0;
	for( i=0; i < nNumChars; i++ ) {
		nLeft = _FindNextColumnWithAnOpaquePixel( pImage, nRight );
		nRight = _FindNextFullyTransparentColumn( pImage, nLeft );
		nTop = _FindFirstNonTransparentRowFromTop( pImage, nLeft, nRight );
		nBottom = _FindLastFullyTransparentRowFromBottom( pImage, nLeft, nRight );

		pLetter[i].nLeft = nLeft;
		pLetter[i].nRight = nRight;
		pLetter[i].nTop = nTop;
		pLetter[i].nBottom = nBottom;
		pLetter[i].nWidth = nRight - nLeft;
		pLetter[i].nHeight = nBottom - nTop;
		if( pLetter[i].nWidth > nMaxWidth ) {
			nMaxWidth = pLetter[i].nWidth;
		}
		if( pLetter[i].nHeight > nMaxHeight ) {
			nMaxHeight = pLetter[i].nHeight;
		}
		pLetter[i].nVAlignment = (nBottom - 1) - nAnchor;

		if( i ) {
			pLetter[i-1].nRightSideSpacing = pLetter[i].nLeft - pLetter[i-1].nRight + 1;
			nTotalSpace += pLetter[i-1].nRightSideSpacing;
		}
	}
	pFont->nNumLetters = nNumChars;
	pFont->pLetterArray = pLetter;
	pFont->nAvgSpaceBetweenLetters = nTotalSpace/nNumChars;
	pFont->nWordSpacing = 3 * pFont->nAvgSpaceBetweenLetters;
	pFont->nHeight = nMaxHeight;
	pFont->nWidth = nMaxWidth;
	pFont->nAlignmentRow = nAnchor;
	pFont->uVertialRuler = nAnchor - _FindFirstNonTransparentLineFromTop( pImage );
	// don't forget to fill in spacing for the last letter
	pLetter[nNumChars-1].nRightSideSpacing = pFont->nAvgSpaceBetweenLetters;

	return pFont;
}

void fontcut_FreeFontData( FontData_t *pFont ) {
	if( !pFont ) {
		return;
	}
	if( pFont->pLetterArray ) {
		free( pFont->pLetterArray );
		pFont->pLetterArray = NULL;
	}
}

LPCTSTR fontcut_GetLastError( void ) {
	return _pszErrorString;	
}

//==================
// private functions

static u32 _FindNextColumnWithAnOpaquePixel( ImageData_t *pImage, u32 nStartingCol ) {
	u32 i, j;
	u8 *pAlpha;

	if( nStartingCol >= pImage->nWidth ) {
		return 0;
	}
	for( i=nStartingCol; i < pImage->nWidth; i++ ) {
		pAlpha = (u8 *)&pImage->pImageData[i];
		pAlpha += 3;
		for( j=0; j < pImage->nHeight; j++ ) {
			if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
				return i;
			}
			pAlpha += pImage->nWidth * sizeof( u32 );
		}
	}
	return 0;
}

static u32 _FindNextFullyTransparentColumn( ImageData_t *pImage, u32 nStartingCol ) {
	u32 i, j;
	u8 *pAlpha;

	if( nStartingCol >= pImage->nWidth ) {
		return 0;
	}
	for( i=nStartingCol; i < pImage->nWidth; i++ ) {
		pAlpha = (u8 *)&pImage->pImageData[i];
		pAlpha += 3;
		for( j=0; j < pImage->nHeight; j++ ) {
			if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
				break;
			}
			pAlpha += pImage->nWidth * sizeof( u32 );
		}
		if( j == pImage->nHeight ) {
			return i;	
		}
	}
	return 0;
}

static u32 _FindFirstNonTransparentRowFromTop( ImageData_t *pImage, u32 nStartingCol, u32 nEndingCol ) {
	u32 i, j, k;
	u8 *pAlpha;

	if( nStartingCol >= pImage->nWidth || nEndingCol > pImage->nWidth ) {
		return 0;
	}
	for( i=0; i < pImage->nHeight; i++ ) {
		k = i * pImage->nWidth;
		for( j=nStartingCol; j < nEndingCol; j++ ) {
			pAlpha = (u8 *)&pImage->pImageData[k + j];
			pAlpha += 3;
			if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
				return i;
			}
		}
	}
	return 0;
}

static u32 _FindLastFullyTransparentRowFromBottom( ImageData_t *pImage, u32 nStartingCol, u32 nEndingCol ) {
	s32 i;
	u32 j, k;
	u8 *pAlpha;

	if( nStartingCol >= pImage->nWidth || nEndingCol > pImage->nWidth ) {
		return 0;
	}
	for( i=(pImage->nHeight-1); i >= 0; i-- ) {
		k = i * pImage->nWidth;
		for( j=nStartingCol; j < nEndingCol; j++ ) {
			pAlpha = (u8 *)&pImage->pImageData[k + j];
			pAlpha += 3;
			if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
				if( i == (pImage->nHeight-1) ) {
					return 0; 
				} else {
					return (i+1);
				}
			}
		}
	}
	return 0;	
}

static u32 _FindFirstNonTransparentLineFromTop( ImageData_t *pImage ) {
	u32 i, j, k;
	u8 *pAlpha;

	for( i=0; i < pImage->nHeight; i++ ) {
		k = ( i * pImage->nWidth );
		for( j=0; j < pImage->nWidth; j++ ) {
			pAlpha = (u8 *)&pImage->pImageData[k + j];
			pAlpha += 3;
			if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
				return i;
			}
		}
	}
	return 0;
}

static BOOL _TopBottomRightEdgesAreTransparent( ImageData_t *pImage ) {
	u32 j, k;
	u8 *pAlpha;

	k = ( ( pImage->nHeight - 1 ) * pImage->nWidth );
	for( j=0; j < pImage->nWidth; j++ ) {
		// top edge
		pAlpha = (u8 *)&pImage->pImageData[j];
		pAlpha += 3;
		if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
			return FALSE;
		}

		// bottom edge
		pAlpha = (u8 *)&pImage->pImageData[k + j];
		pAlpha += 3;
		if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
			return FALSE;
		}
	}

	for( j=1; j <= pImage->nHeight; j++ ) {
		// right edge
		pAlpha = (u8 *)&pImage->pImageData[ ( j * pImage->nWidth ) - 1 ];
		pAlpha += 3;
		if( *pAlpha != _TRANSPARENT_PIXEL_COLOR ) {
			return FALSE;
		}
	}

	return TRUE;
}
