//////////////////////////////////////////////////////////////////////////////////////
// sound.h - General purpose sound functions.
//
// Author: Steve Ranck     
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
// 03/2/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SOUND_H_
#define _SOUND_H_ 1

#include "fang.h"


BOOL sound_LoadSoundGroups( void );

BOOL sound_InitSystem( void );
void sound_UninitSystem( void );



#endif

