//////////////////////////////////////////////////////////////////////////////////////
// AIRooms.h - 
//
// Author: Pat MacKellar 
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
// 04/12/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _AIROOMS_H_
#define _AIROOMS_H_ 1

#include "fmath.h"


class CAIGraph;

extern BOOL airooms_InitSystem(CAIGraph* pGraph);
extern void airooms_CleanupSystem(void);

extern u32 airooms_FindRoomID( const CFVec3A &Pos_WS );
extern u32 airooms_IsVertInRoom(u16 nVertId);

#endif _AIROOMS_H_
