//////////////////////////////////////////////////////////////////////////////////////
// fGCmath.inl - Fang math module (GameCube version).
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
// 02/18/02 Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

#include <math.h>
#include "dtrigf.h"


#define _USE_GC_FAST_SIN_COS	TRUE


#if !_USE_GC_FAST_SIN_COS
	static const f32 _afInterpTable1[4] = { 1.00f, 0.75f, 0.50f, 0.25f };
	static const f32 _afInterpTable2[4] = { 0.00f, 0.25f, 0.50f, 0.75f };
#endif

//---------------------------------------------------------------------------------------------
// Fast approximation functions:
//---------------------------------------------------------------------------------------------

// Approximate fast sin.
FINLINE f32 fmath_Sin( f32 fAngle ) {
#if _USE_GC_FAST_SIN_COS
	if ( fmath_Abs( fAngle ) > FMATH_2PI )
	{
		fAngle = fmath_Fmod( fAngle, FMATH_2PI );
	}
	
	if ( fAngle < 0 )
	{
		fAngle = FMATH_2PI + fAngle;
	}
	
	return sinf_f( fAngle );		
#else
	s32 a1, a2, i, i1, i2, nAngle;
	f32 v1, v2;

	if( fAngle<-FMATH_MAX_ANGLE_BEFORE_MODULATING || fAngle>FMATH_MAX_ANGLE_BEFORE_MODULATING ) {
		fAngle = fmath_Fmod( fAngle, FMATH_2PI );
	}

	nAngle = fmath_FloatToS32( fAngle * (65536.0f * (f32)FMATH_SINTBL_ROUND_TABLE_COUNT / FMATH_2PI) );

	i = nAngle & (FMATH_SINTBL_ROUND_TABLE_COUNT-1);
	a1 = nAngle >> FMATH_SINTBL_ROUND_BIT_COUNT;
	a2 = a1 + 1;
	i1 = a1 & 16383;
	i2 = a2 & 16383;

	v1 = SinTbl_afTable[ (a1 & 16384) ? (16384-i1) : i1 ];
	v2 = SinTbl_afTable[ (a2 & 16384) ? (16384-i2) : i2 ];
	if( a1 & 32768 ) v1 *= -1.0f;
	if( a2 & 32768 ) v2 *= -1.0f;

	return v1*FMath_afInterpTable1[i] + v2*FMath_afInterpTable2[i];
#endif
}


// Approximate fast cos.
FINLINE f32 fmath_Cos( f32 fAngle ) {
#if _USE_GC_FAST_SIN_COS
	if ( fmath_Abs( fAngle ) > FMATH_2PI )
	{
		fAngle = fmath_Fmod( fAngle, FMATH_2PI );
	}
	
	if ( fAngle < 0 )
	{
		fAngle = FMATH_2PI + fAngle;
	}
	
	return cosf_f( fAngle );		
#else
	s32 a1, a2, i, i1, i2, nAngle;
	f32 v1, v2;

	if( fAngle<-FMATH_MAX_ANGLE_BEFORE_MODULATING || fAngle>FMATH_MAX_ANGLE_BEFORE_MODULATING ) {
		fAngle = fmath_Fmod( fAngle, FMATH_2PI );
	}

	nAngle = (16384 * FMATH_SINTBL_ROUND_TABLE_COUNT) + fmath_FloatToS32( fAngle * (65536.0f * (f32)FMATH_SINTBL_ROUND_TABLE_COUNT / FMATH_2PI) );

	i = nAngle & (FMATH_SINTBL_ROUND_TABLE_COUNT-1);
	a1 = nAngle >> FMATH_SINTBL_ROUND_BIT_COUNT;
	a2 = a1 + 1;
	i1 = a1 & 16383;
	i2 = a2 & 16383;

	v1 = SinTbl_afTable[ (a1 & 16384) ? (16384-i1) : i1 ];
	v2 = SinTbl_afTable[ (a2 & 16384) ? (16384-i2) : i2 ];
	if( a1 & 32768 ) v1 *= -1.0f;
	if( a2 & 32768 ) v2 *= -1.0f;

	return v1*FMath_afInterpTable1[i] + v2*FMath_afInterpTable2[i];
#endif
}


// Approximate fast sin & cos.
FINLINE void fmath_SinCos( f32 fAngle, f32 *pfSin, f32 *pfCos ) 
{
#if _USE_GC_FAST_SIN_COS
	if ( fmath_Abs( fAngle ) > FMATH_2PI )
	{
		fAngle = fmath_Fmod( fAngle, FMATH_2PI );
	}
	
	if ( fAngle < 0 )
	{
		fAngle = FMATH_2PI + fAngle;
	}
	
	union SinCosAngle
	{
		__vec2x32float__ _A;
		struct { f32 fSin, fCos; };
	};
	
	SinCosAngle Test, Result;
	Test.fSin = fAngle;
	Test.fCos = fAngle;
	
	Result._A = sincosf_f( Test._A );
	*pfSin = Result.fSin;
	*pfCos = Result.fCos;
#else
	*pfSin = fmath_Sin( fAngle );
	*pfCos = fmath_Cos( fAngle );
#endif
}

// fast approximate inverse cosine. From magic-software.com
FINLINE f32 fmath_InvCos( f32 fCos )
{
	BOOL bNegative = FALSE;

	FMATH_CLAMP( fCos, -1.0f, 1.0f );

	if( fCos < 0.0f )
	{
		fCos = -fCos;
		bNegative = TRUE;
	}

    f32 fRoot = fmath_Sqrt( 1.0f - fCos );

    f32 fResult = -0.0187293f;
    fResult *= fCos;
    fResult += 0.0742610f;
    fResult *= fCos;
    fResult -= 0.2121144f;
    fResult *= fCos;
    fResult += 1.5707288f;
    fResult *= fRoot;

	if( bNegative )
	{
		fResult = FMATH_PI - fResult;
	}

    return fResult;
}

//
// Approximate fast square-root.
//
FINLINE f32 fmath_Sqrt( register f32 fVal ) 
{
#if 1
	FASSERT_POS_FLOAT( fVal );

	register f32 fRetVal;
	__asm 
	{
		frsqrte	fRetVal, fVal
		fres	fRetVal, fRetVal
	}
	
	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
#else
	return (f32)sqrt( fVal );
#endif
}


//
// Approximate fast inverse square-root.
//
FINLINE f32 fmath_InvSqrt( register f32 fVal ) 
{
#if 1
	register f32 fRetVal;
//	register f32 fTemp;

	FASSERT_POS_FLOAT( fVal );

	__asm
	{
		frsqrte	fRetVal, fVal
	}
		
	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
#else
	return 1.f / fmath_Sqrt( fVal );
#endif
}


//
// Approximate fast reciprocal.
//
FINLINE f32 fmath_Inv( register f32 fVal ) 
{
#if 1
	register f32 fRetVal;

	FASSERT( fVal != 0.0f );

	__asm 
	{
		fres	fRetVal, fVal
	}

	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
#else
	return 1.f / fVal;
#endif
}


//
// Approximate fast division.
//
FINLINE f32 fmath_Div( register f32 fNumerator, register f32 fDenominator ) 
{
#if 1
	register f32 fRetVal;

	FASSERT( fDenominator != 0.0f );

	__asm 
	{
		fres	fRetVal, fDenominator
		fmuls	fRetVal, fNumerator, fRetVal
	}

	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
#else
	return fNumerator / fDenominator;
#endif
}


//
//	Fast cast
//
FINLINE s32 fmath_FloatToS32( register f32 fVal ) 
{
#if 0
	register f32 fTemp;
	s32 nVal;
	register s32 *pTemp = &nVal;

	FMATH_DEBUG_FCHECK( fVal );
	FASSERT( fVal>(f32)(s32)0x81000000 && fVal<(f32)(s32)0x7effffff );

	__asm 
	{
		fctiwz	fTemp, fVal
		stfiwx	fTemp, 0, pTemp
	}

	return nVal;
#else
	return (s32)fVal;
#endif  // USE_ASSEMBLY_MATH
}


//
//
//
FINLINE u32 fmath_FloatToU32( f32 fVal ) 
{
#if 0
	u32 nVal;

	FMATH_DEBUG_FCHECK( fVal );
	FASSERT( fVal >= 0.0f );
	FASSERT( fVal < (f32)(u32)0xfeffffff );

	__asm 
	{
		fld fVal					; Store value in ST(0)
		fistp nVal;					; Pop it off fp stack, convert it to an integer, and store it in nVal
	}

	return nVal;
#else
	return (u32)fVal;
#endif // USE_ASSEMBLY_MATH
}



//---------------------------------------------------------------------------------------------
// ANSI compatible, accurate, slow functions:
//---------------------------------------------------------------------------------------------

//
// ANSI compatible square-root.
//
FINLINE f32 fmath_AcuSqrt( f32 fVal ) 
{
	return (f32)sqrt( fVal );
}


//
// ANSI compatible inverse square-root.
//
FINLINE f32 fmath_AcuInvSqrt( f32 fVal ) 
{
#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	FASSERT_POS_FLOAT( fVal );

	f32 fRetVal = 1.0f / fmath_Sqrt( fVal );
	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
#else
	FASSERT_POS_FLOAT( fVal );
	return 1.0f / fmath_Sqrt( fVal );
#endif
}


//
// Accurate slow sin.
//
FINLINE f32 fmath_AcuSin( f32 fAngle ) 
{
	return (f32)sin( fAngle );
}


//
// Accurate slow cos.
//
FINLINE f32 fmath_AcuCos( f32 fAngle ) 
{
	return (f32)cos( fAngle );
}


//
// Accurate slow sin & cos.
//
FINLINE void fmath_AcuSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos ) 
{
	*pfSin = fmath_AcuSin( fAngle );
	*pfCos = fmath_AcuCos( fAngle );
}


//
// Accurate slow sin biased from 0 to +1.
//
FINLINE f32 fmath_AcuPositiveSin( f32 fAngle ) 
{
	return 0.5f + 0.5f * fmath_AcuSin( fAngle );
}


//
// Accurate slow cos biased from 0 to +1.
//
FINLINE f32 fmath_AcuPositiveCos( f32 fAngle ) 
{
	return 0.5f + 0.5f * fmath_AcuCos( fAngle );
}


//
// Accurate slow sin & cos biased from 0 to +1.
//
FINLINE void fmath_AcuPositiveSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos ) 
{
	*pfSin = 0.5f + 0.5f * fmath_AcuSin( fAngle );
	*pfCos = 0.5f + 0.5f * fmath_AcuCos( fAngle );
}




