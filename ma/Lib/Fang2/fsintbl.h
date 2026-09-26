//////////////////////////////////////////////////////////////////////////////////////
// fsintbl.h - 
//
// Author: Steve Ranck     
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
// 02/20/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FSINTBL_H_
#define _FSINTBL_H_ 1

#include "fang.h"


#define FSINTBL_QUAD_BITS	14
#define FSINTBL_ENTRIES		16385
#define FSINTBL_SIZE		16384
#define FSINTBL_SIGN_MASK	32768
#define FSINTBL_INV_MASK	16384

extern const f32 SinTbl_afTable[16385];



#endif

