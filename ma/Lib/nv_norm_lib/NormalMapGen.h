//////////////////////////////////////////////////////////////////////////////////////
// NormalMapGen.h - 
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
// 10/11/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _NORMAL_MAP_GEN_H_
#define _NORMAL_MAP_GEN_H_ 1

#include "fang.h"

enum KernelType
{
	KERNEL_4x,
	KERNEL_3x3,
	KERNEL_5x5,
	KERNEL_7x7,
	KERNEL_9x9,
	KERNEL_FORCEDWORD = 0xffffffff
};

extern BOOL ConvertTgaToNormMap( const char *pszInFile,
								 const char *pszOutFile,
								 KernelType	kerneltype=KERNEL_3x3,
								 BOOL wrap=TRUE,
								 float scale=0.0f );



#endif

