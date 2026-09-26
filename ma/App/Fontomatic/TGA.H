//////////////////////////////////////////////////////////////////////////////////////
// tga.h - "reads and writes 32bit tgas, using the ImageData struct"
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
#ifndef _TGA_H_
#define _TGA_H_ 1

#include "fang.h"

typedef struct
{
	u32 *pImageData;
	u16 nWidth;
	u16 nHeight;
} ImageData_t;

extern BOOL tga_ModuleInit( void );
extern ImageData_t *tga_Get32BitImageDataFromFile( LPCTSTR pszFileName, ImageData_t *pImageData );
extern BOOL tga_WriteOut32BitImageDataToFile( LPCTSTR pszFileName, ImageData_t *pImageData );
extern void tga_FreeImageData( ImageData_t *pImageData );
extern LPCTSTR tga_GetLastError( void );

#endif
