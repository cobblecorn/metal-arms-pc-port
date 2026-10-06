//////////////////////////////////////////////////////////////////////////////////////
// fDX8anim.inl - Fang platform specific animation inline module.
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
	// Duplicate timestamps represent a held key, not a division by zero.
	if( nHigh <= nLow || fValue <= (f32)nLow ) return 0.0f;
	if( fValue >= (f32)nHigh ) return 1.0f;

	return fmath_Div( fValue - (f32)nLow, (f32)nHigh - (f32)nLow );
}


//
//
//
FINLINE f32 fanim_GenerateRatio_16bit( f32 fValue, u16 nLow, u16 nHigh )
{
	// Duplicate timestamps represent a held key, not a division by zero.
	if( nHigh <= nLow || fValue <= (f32)nLow ) return 0.0f;
	if( fValue >= (f32)nHigh ) return 1.0f;

	return fmath_Div( fValue - (f32)nLow, (f32)nHigh - (f32)nLow );
}


//
//
//
FINLINE f32 fanim_GenerateRatio_32bit( f32 fValue, f32 fLow, f32 fHigh )
{
	// Duplicate timestamps represent a held key, not a division by zero.
	if( fHigh <= fLow || fValue <= (f32)fLow ) return 0.0f;
	if( fValue >= (f32)fHigh ) return 1.0f;

	return fmath_Div( fValue - fLow, fHigh - fLow );
}


