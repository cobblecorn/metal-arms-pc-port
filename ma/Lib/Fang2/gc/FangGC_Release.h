//////////////////////////////////////////////////////////////////////////////////////
// FangGC_Release.h - Fang release build Codewarrior compiler prefix.
//
// Author: John Lafleur
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/01/01 Lafleur     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FANGGC_RELEASE_H_
#define _FANGGC_RELEASE_H_ 1

#include "HW2_Release_Prefix.h"

#pragma always_inline on 

#define _FANGDEF_PLATFORM_GC
#define _FANGDEF_ENABLE_DEV_FEATURES
#define _FANGDEF_RELEASE_BUILD

#endif

