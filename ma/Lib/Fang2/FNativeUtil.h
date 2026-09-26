//////////////////////////////////////////////////////////////////////////////////////
// FNativeUtil.h - Native scripting function utilities.
//
// Author: Justin Link
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
// 04/02/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FNATIVEUTIL_H_
#define _FNATIVEUTIL_H_ 1

#include "fang.h"
#include "amx.h"

FINLINE cell ConvertF32ToCell(f32 fValue)
{
	cell oTemp;
	f32 *pf32 = (f32 *)(&oTemp);
	*pf32 = fValue;

	return(oTemp);
}

FINLINE f32 ConvertCellToF32(cell oCell)
{
	f32 fTemp;
	f32 *pf32 = (f32 *)(&oCell);
	fTemp = *pf32;

	return(fTemp);
}

void FillStringFromCell(char *pszDestString, u32 uMaxStringSize, AMX *pAMX, cell oParam);

#endif