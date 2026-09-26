//////////////////////////////////////////////////////////////////////////////////////
// screenshot.h - 
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
// 10/08/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SCREENSHOT_H_
#define _SCREENSHOT_H_ 1

#include "fang.h"

typedef enum {
	SCREENSHOT_OUTPUT_TYPE_TGA = 0,
	SCREENSHOT_OUTPUT_TYPE_JPEG,

	SCREENSHOT_OUTPUT_TYPE_COUNT
} ScreenShotOutputType_e;

extern void screenshot_Init( cchar *pszScreenShotDir, BOOL bStartSnappingSeriesOnFrame1=TRUE );
extern void screenshot_Shutdown( void );

extern void screenshot_Work( void );

extern void screenshot_SetScreenShotMode( BOOL bSingle );
extern BOOL screenshot_GetScreenShotMode( void );

extern void screenshot_SetScreenShotType( ScreenShotOutputType_e nScreenShotType );
extern ScreenShotOutputType_e screenshot_GetScreenShotType( void );

#endif

