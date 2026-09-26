//////////////////////////////////////////////////////////////////////////////////////
// fGCanim.inl - Fang platform specific animation inline module.
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
// 07/15/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////



//
//
//
FINLINE f32 fanim_GenerateRatio_8bit( f32 fValue, u8 nLow, u8 nHigh )
{
	return fmath_Div( fValue - (f32)nLow, (f32)nHigh - (f32)nLow );
}


//
//
//
FINLINE f32 fanim_GenerateRatio_16bit( f32 fValue, u16 nLow, u16 nHigh )
{
	return fmath_Div( fValue - (f32)nLow, (f32)nHigh - (f32)nLow );
}


//
//
//
FINLINE f32 fanim_GenerateRatio_32bit( f32 fValue, f32 fLow, f32 fHigh )
{
	return fmath_Div( fValue - fLow, fHigh - fLow );
}


