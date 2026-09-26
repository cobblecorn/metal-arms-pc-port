//////////////////////////////////////////////////////////////////////////////////////
// fontpack.h - "creates a new tga(s) packing a font as tight as possible"
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
#ifndef _FONTPACK_H_
#define _FONTPACK_H_ 1

#include "fang.h"
#include "fontcut.h"
#include "tga.h"

typedef struct
{
	u32 nUnPackedIndex;	// where was this letter in the unpacked letter array
	u32 nTexturePageIndex;// which page is this letter now contained on
	u32 nLeft;	// leftmost column containing a pixel
	u32 nRight;	// right column after last pixel (does not contain a pixel)
	u32 nTop;	// topmost row containing a pixel
	u32 nBottom;// bottom row after the last pixel (does not contain a pixel)
} PackedLetter_t;

typedef struct
{
	FontData_t *pOriginalData;
	PackedLetter_t *pPackedLetterArray;
	u32 nNumPages;
	TCHAR **pszFilenames;
	u16 *pauWidth;
	u16 *pauHeight;

} PackData_t;

extern BOOL fontpack_ModuleInit( void );
extern PackData_t *fontpack_CreatePackedTGAs( const bool bWriteOutput, LPCTSTR pszBaseFileName, LPCTSTR pszPath, ImageData_t *pImage, FontData_t *pFont );
extern void fontpack_FreePackData( PackData_t *pPack );
extern LPCTSTR fontpack_GetLastError( void );

#endif
