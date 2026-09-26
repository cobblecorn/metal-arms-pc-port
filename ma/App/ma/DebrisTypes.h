//////////////////////////////////////////////////////////////////////////////////////
// DebrisTypes.h - Hold unique IDs for the fdebris system.
//
// Author: Steve Ranck     
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
// 01/02/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DEBRIS_TYPES_H_
#define _DEBRIS_TYPES_H_ 1

#include "fang.h"


typedef enum {
	DEBRISTYPES_UNKNOWN = 0,
	DEBRISTYPES_BOT_PART_SYSTEM,		// Debris belongs to the bot part system.
	
	DEBRISTYPES_COUNT
} DebrisTypes_e;




#endif

