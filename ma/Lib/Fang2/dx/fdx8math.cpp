//////////////////////////////////////////////////////////////////////////////////////
// fdx8math.cpp - 
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
// 01/18/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdx8math.h"



const CFVec4A FDX8Math_nnnnMask_XYZ1_W0( (u32)0xffffffff, (u32)0xffffffff, (u32)0xffffffff, (u32)0x00000000 );
const CFVec4A FDX8Math_nnnnNegMaskW( (u32)0x00000000, (u32)0x00000000, (u32)0x00000000, (u32)0x80000000 );
const CFVec4A FDX8Math_NegOnesW1( -1.0f, -1.0f, -1.0f, 1.0f );
CFVec3A FDX8Math_TempVec3A;
CFVec4A FDX8Math_TempVec4A;
f32 FDX8Math_fTemp;

