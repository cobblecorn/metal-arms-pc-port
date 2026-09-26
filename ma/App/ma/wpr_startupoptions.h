//////////////////////////////////////////////////////////////////////////////////////
// wpr_startupoptions.h - The game startup options.
//
// Author: Russell A. Foushee   
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
// 05/28/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WPR_STARTUP_OPTIONS_H_
#define _WPR_STARTUP_OPTIONS_H_ 1

#include "fang.h"


extern BOOL wpr_startupoptions_InitSystem( void );
extern void wpr_startupoptions_UninitSystem( void );

extern void wpr_startupoptions_CaptureAppStartButtons( void );

extern void wpr_startupoptions_Start( void );

#endif

