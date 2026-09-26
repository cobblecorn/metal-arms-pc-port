//////////////////////////////////////////////////////////////////////////////////////
// wpr_levelcomplete.h - The End Of Level Screens
//
// Author: Michael Starich
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/30/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WPR_LEVEL_COMPLETE_H_
#define _WPR_LEVEL_COMPLETE_H_ 1

#include "fang.h"


extern BOOL wpr_levelcomplete_InitSystem( void );
extern void wpr_levelcomplete_UninitSystem( void );

extern void wpr_levelcomplete_ScheduleSinglePlayerScreen( void );

#endif