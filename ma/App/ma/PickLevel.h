//////////////////////////////////////////////////////////////////////////////////////
// PickLevel.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/29/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _PICKLEVEL_H_
#define _PICKLEVEL_H_ 1

#include "fang.h"
#include "launcher.h"

extern BOOL picklevel_InitSystem( void );
extern void picklevel_UninitSystem( void );

#if LAUNCHER_INCLUDE_DEV_MENU

extern BOOL picklevel_Start( void );
extern void picklevel_End( void );
extern cchar *picklevel_GetNameOfSelectedLevel( void );

extern void picklevel_MeasureLoadTime( BOOL bStart );

#endif

#endif

