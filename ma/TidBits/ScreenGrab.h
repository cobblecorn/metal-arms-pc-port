//////////////////////////////////////////////////////////////////////////////////////
// ScreenGrab.h - screen capture utility
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
#ifndef _SCREENGRAB_H_
#define _SCREENGRAB_H_ 1

#include "fang.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern BOOL screengrab_Init( cchar *pszDirectory );
extern void screengrab_Shutdown();
extern BOOL screengrab_CaptureTheScreen( BOOL bSingle );

#if defined(__cplusplus)
}
#endif

#endif
