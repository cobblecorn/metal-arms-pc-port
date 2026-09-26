//////////////////////////////////////////////////////////////////////////////////////
// fdx8math.inl - Fang math module (DX version).
//
// Author: Steve Ranck     
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
// 10/06/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"



//---------------------------------------------------------------------------------------------
// Fast approximation functions:
//---------------------------------------------------------------------------------------------

// Approximate fast sin.
FINLINE f32 fmath_Sin( f32 fAngle ) {
#if 0
	return fmath_AcuSin(fAngle);
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
#if 0
	return fmath_AcuCos(fAngle);
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
FINLINE void fmath_SinCos( f32 fAngle, f32 *pfSin, f32 *pfCos ) {
	*pfSin = fmath_Sin( fAngle );
	*pfCos = fmath_Cos( fAngle );
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


// Approximate fast square-root.
FINLINE f32 fmath_Sqrt( f32 fVal ) {
	f32 fRetVal;

	FASSERT_POS_FLOAT( fVal );

	__asm {
		rsqrtss	xmm0, fVal
		rcpss	xmm0, xmm0
		movss	fRetVal, xmm0
	}

	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
}


// Approximate fast inverse square-root.
FINLINE f32 fmath_InvSqrt( f32 fVal ) {
	f32 fRetVal;

	FASSERT_POS_FLOAT( fVal );

	__asm {
		rsqrtss	xmm0, fVal
		movss	fRetVal, xmm0
	}

	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
}


// Approximate fast reciprocal.
FINLINE f32 fmath_Inv( f32 fVal ) {
	f32 fRetVal;

	FASSERT( fVal != 0.0f );

	__asm {
		rcpss	xmm0, fVal
		movss	fRetVal, xmm0
	}

	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
}


// Approximate fast division.
FINLINE f32 fmath_Div( f32 fNumerator, f32 fDenominator ) {
	f32 fRetVal;

	FASSERT( fDenominator != 0.0f );

	__asm {
		rcpss	xmm0, fDenominator
		mulss	xmm0, fNumerator
		movss	fRetVal, xmm0
	}

	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
}


FINLINE s32 fmath_FloatToS32( f32 fVal ) {
	s32 nVal;

	FMATH_DEBUG_FCHECK( fVal );
	FASSERT( fVal>(f32)(s32)0x81000000 && fVal<(f32)(s32)0x7effffff );

	__asm {
		fld fVal					; Store value in ST(0)
		fistp nVal;					; Pop it off fp stack, convert it to an integer, and store it in nVal
	}

	return nVal;
}


FINLINE u32 fmath_FloatToU32( f32 fVal ) {
	u32 nVal;

	FMATH_DEBUG_FCHECK( fVal );
	FASSERT( fVal >= 0.0f );
	FASSERT( fVal < (f32)(u32)0xfeffffff );

	__asm {
		fld fVal					; Store value in ST(0)
		fistp nVal;					; Pop it off fp stack, convert it to an integer, and store it in nVal
	}

	return nVal;
}



//---------------------------------------------------------------------------------------------
// ANSI compatible, accurate, slow functions:
//---------------------------------------------------------------------------------------------

// ANSI compatible square-root.
FINLINE f32 fmath_AcuSqrt( f32 fVal ) {
	f32 fRetVal;

	FASSERT_POS_FLOAT( fVal );

	__asm {
		fld fVal					; Store value in ST(0)
		fsqrt						; Compute square-root and put into ST(0)
		fstp fRetVal				; Pop value off fp stack and put into fRetValue
	}

	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
}


// ANSI compatible inverse square-root.
FINLINE f32 fmath_AcuInvSqrt( f32 fVal ) {
#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	FASSERT_POS_FLOAT( fVal );

	f32 fRetVal = 1.0f / fmath_AcuSqrt( fVal );
	FMATH_DEBUG_FCHECK( fRetVal );

	return fRetVal;
#else
	FASSERT_POS_FLOAT( fVal );
	return 1.0f / fmath_AcuSqrt( fVal );
#endif
}


// Accurate slow sin.
FINLINE f32 fmath_AcuSin( f32 fAngle ) {
	f32 fRetValue;

	__asm {
		fld fAngle					; Store angle in ST(0)
		fsin						; Compute sine and put into ST(0)
		fstp fRetValue;				; Pop value off fp stack and put into fRetValue
	}

	return fRetValue;
}


// Accurate slow cos.
FINLINE f32 fmath_AcuCos( f32 fAngle ) {
	f32 fRetValue;

	__asm {
		fld fAngle					; Store angle in ST(0)
		fcos						; Compute cosine and put into ST(0)
		fstp fRetValue;				; Pop value off fp stack and put into fRetValue
	}

	return fRetValue;
}


// Accurate slow sin & cos.
FINLINE void fmath_AcuSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos ) {
	f32 fSinResult, fCosResult;

	__asm {
		fld fAngle					; Store angle in ST(0)
		fsincos						; Compute sine and cosine and put into ST(1) and ST(0), respectively
		fstp fCosResult				; Pop cosine value off fp stack and put into fCosResult
		fstp fSinResult				; Pop sine value off fp stack and put into fSinResult
	}

	*pfSin = fSinResult;
	*pfCos = fCosResult;
}


// Accurate slow sin biased from 0 to +1.
FINLINE f32 fmath_AcuPositiveSin( f32 fAngle ) {
	f32 fRetValue;

	__asm {
		fld fAngle					; Store angle in ST(0)
		fsin						; Compute sine and put into ST(0)
		fstp fRetValue;				; Pop value off fp stack and put into fRetValue
	}

	return 0.5f + 0.5f*fRetValue;
}


// Accurate slow cos biased from 0 to +1.
FINLINE f32 fmath_AcuPositiveCos( f32 fAngle ) {
	f32 fRetValue;

	__asm {
		fld fAngle					; Store angle in ST(0)
		fcos						; Compute cosine and put into ST(0)
		fstp fRetValue;				; Pop value off fp stack and put into fRetValue
	}

	return 0.5f + 0.5f*fRetValue;
}


// Accurate slow sin & cos biased from 0 to +1.
FINLINE void fmath_AcuPositiveSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos ) {
	f32 fSinResult, fCosResult;

	__asm {
		fld fAngle					; Store angle in ST(0)
		fsincos						; Compute sine and cosine and put into ST(1) and ST(0), respectively
		fstp fCosResult				; Pop cosine value off fp stack and put into fCosResult
		fstp fSinResult				; Pop sine value off fp stack and put into fSinResult
	}

	*pfSin = 0.5f + 0.5f*fSinResult;
	*pfCos = 0.5f + 0.5f*fCosResult;
}




