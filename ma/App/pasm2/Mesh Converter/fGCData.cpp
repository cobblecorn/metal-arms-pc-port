//////////////////////////////////////////////////////////////////////////////////////
// fGCData.cpp - 
//
// Author: John Lafleur
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
// 03/16/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fres.h"
#include "gc\fgcdata.h"
//#include "gc\fGCnormalsphere.h"

FGCData_VtxFmt_t FGCData_VertexFormatDesc[FGCDATA_FIXED_VERTEX_FORMATS] =
{
	// Full f32 position accuracy (usually used by world geo)
	32,
	0,
	8,
	6,
	16,
	8,
	
	// 16 bit position with 5 fractional bits (compressed world geo format)
	16,
	5,
	8,
	6,
	16,
	8,
	
	// 16 bit position with 12 fractional bits
	16,
	12,
	8,
	6,
	16,
	8,
	
	// 16 bit position with 11 fractional bits
	16,
	11,
	8,
	6,
	16,
	8,
	
	// 8 bit position with 7 fractional bits
	8,
	7,
	8,
	6,
	16,
	8,
	
	// 8 bit position with 6 fractional bits
	8,
	6,
	8,
	6,
	16,
	8,
	
	// 8 bit position with 5 fractional bits
	8,
	5,
	8,
	6,
	16,
	8,
};



