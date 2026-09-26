//////////////////////////////////////////////////////////////////////////////////////
// MG_HoldYourGround.h - Mini-Game: Hold Your Ground 
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
// 04/21/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MG_HOLD_YOUR_GROUND_H_
#define _MG_HOLD_YOUR_GROUND_H_ 1

#include "fang.h"
#include "level.h"

extern BOOL mg_holdyourground_InitSystem( void );
extern void mg_holdyourground_UninitSystem( void );

extern BOOL mg_holdyourground_LevelLoad( LevelEvent_e eEvent );
extern void mg_holdyourground_LevelUnload( void );
extern void mg_holdyourground_LevelWork( void );
extern void mg_holdyourground_LevelDraw( void );
extern void mg_holdyourground_PlayerLost( void );
extern void mg_HoldYourGround_Restore( void );

#endif