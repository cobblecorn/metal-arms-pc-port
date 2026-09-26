//////////////////////////////////////////////////////////////////////////////////////
// fontcut.h - "cuts up an ImageData image into seperate letter images" 
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
#ifndef _FONTCUT_H_
#define _FONTCUT_H_ 1

#include "fang.h"
#include "tga.h"

typedef struct
{
	u32 nLeft;	// leftmost column containing a pixel
	u32 nRight;	// right column after last pixel (does not contain a pixel)
	u32 nTop;	// topmost row containing a pixel
	u32 nBottom;// bottom row after the last pixel (does not contain a pixel)
	u32 nWidth;	// number of pixels across
	u32 nHeight;// number of pixels high
	u32 nRightSideSpacing;// number of pixels to the next letter
	s32 nVAlignment;// pixels that the letter must be translated from the font's allignment row
} Letter_t;

typedef struct
{
	u32 nNumLetters;
	Letter_t *pLetterArray;
	u32 nAvgSpaceBetweenLetters;
	u32 nAlignmentRow;	// what row of the image has been desiginated as the vertical alignment
	u32 nWidth; // Width of largest letter.
	u32 nHeight; // Height of largest letter.
	u32 nWordSpacing;
	u32 uVertialRuler;

} FontData_t;

extern BOOL fontcut_ModuleInit( void );
extern FontData_t *fontcut_CutUpImageIntoAFont( ImageData_t *pImage, FontData_t *pFont );
extern void fontcut_FreeFontData( FontData_t *pFont );
extern LPCTSTR fontcut_GetLastError( void );

#endif
