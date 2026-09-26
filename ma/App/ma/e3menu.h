//////////////////////////////////////////////////////////////////////////////////////
// e3menu.h - 
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
// 02/25/03 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _E3MENU_H_
#define _E3MENU_H_ 1

#include "fang.h"
#include "game.h"
#include "launcher.h"


extern BOOL e3menu_InitSystem( void );
extern void e3menu_UninitSystem( void );

#if LAUNCHER_GO_DIRECTLY_TO_E3_WRAPPERS		
	extern BOOL e3menu_Start( void );
	extern void e3menu_End( void );
	extern cchar *e3menu_GetNameOfSelectedLevel( void );
	extern const GameInitInfo_t *e3menu_GetInitInfo( void );
#endif

#endif

