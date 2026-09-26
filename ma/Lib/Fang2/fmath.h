//////////////////////////////////////////////////////////////////////////////////////
// fmath.h - Fang math module.
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
// 09/15/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FMATH_H_
#define _FMATH_H_ 1

#include "fang.h"
#include "fsintbl.h"


#define FMATH_MAX_ANGLE_BEFORE_MODULATING	(FMATH_2PI * 100.0f)
#define FMATH_SINTBL_ROUND_BIT_COUNT		( 8 )
#define FMATH_SINTBL_ROUND_TABLE_COUNT		(1 << FMATH_SINTBL_ROUND_BIT_COUNT)




//-------------------------------------------------------------------------------------------------------------
// General Definitions
//-------------------------------------------------------------------------------------------------------------
//#define FMATH_DEFAULT_RANDOM_SEED	0x5324879f

typedef struct {
	f32 fCoef;
	f32 fOffset;
} FMath_AtanCoefInfo_t;




//-------------------------------------------------------------------------------------------------------------
// Angle and Numeric Definitions
//-------------------------------------------------------------------------------------------------------------
#define FMATH_PI			(3.1415926535897932384626433832795f)
#define FMATH_2PI			(FMATH_PI * 2.0f)
#define FMATH_4PI			(FMATH_PI * 4.0f)
#define FMATH_HALF_PI		(FMATH_PI / 2.0f)
#define FMATH_QUARTER_PI	(FMATH_PI / 4.0f)

#define FMATH_DEG2RAD( fDeg ) ((fDeg)*(FMATH_2PI/360.0f))
#define FMATH_RAD2DEG( fRad ) ((fRad)*(360.0f/FMATH_2PI))

#define FMATH_SQRT2			(1.4142135623730950488016887242097f)




//-------------------------------------------------------------------------------------------------------------
// General Math Macros
//-------------------------------------------------------------------------------------------------------------
#define FMATH_IABS( a ) fmath_Abs( a )
#define FMATH_FABS( a ) fmath_Abs( a )
#define FMATH_ISIGN( a ) ( (a)>=0 ? 1 : -1 )
#define FMATH_FSIGN( a ) ( (a)>=0.0f ? 1.0f : -1.0f )
#define FMATH_MIN( a, b ) ( (a)<(b) ? (a) : (b) )
#define FMATH_MAX( a, b ) ( (a)>(b) ? (a) : (b) )
#define FMATH_MIN3( a, b, c ) ( (((a)<=(b)) & ((a)<=(c))) ? (a) : ( (((b)<=(a)) & ((b)<=(c))) ? (b) : (c) ) )
#define FMATH_MAX3( a, b, c ) ( (((a)>=(b)) & ((a)>=(c))) ? (a) : ( (((b)>=(a)) & ((b)>=(c))) ? (b) : (c) ) )
#define FMATH_CLAMPMIN( v, min ) { if((v)<(min)) (v)=(min); }
#define FMATH_CLAMPMAX( v, max ) { if((v)>(max)) (v)=(max); }
#define FMATH_CLAMP( v, min, max ) { if((v)<(min)) (v)=(min); else if((v)>(max)) (v)=(max); }
#define FMATH_BIPOLAR_CLAMPMIN( v, min ) { if((v)>=0) { FMATH_CLAMPMIN((v),(min)) } else { FMATH_CLAMPMAX((v),-(min)) } }
#define FMATH_BIPOLAR_CLAMPMAX( v, max ) { if((v)>=0) { FMATH_CLAMPMAX((v),(max)) } else { FMATH_CLAMPMIN((v),-(max)) } }
#define FMATH_BIPOLAR_CLAMP( v, min, max ) { if((v)>=0) { FMATH_CLAMP((v),(min),(max)) } else { FMATH_CLAMP((v),-(max),-(min)) } }
#define FMATH_FPOT( fUnitControl, f0, f1 ) ( (fUnitControl)*(f1) + (1.0f-(fUnitControl))*(f0) )
#define FMATH_FPOTD( fUnitControl, f0, fDelta ) ( (fUnitControl)*(fDelta) + (f0) )
#define FMATH_ISWAP( i1, i2 ) {	int tmp; tmp=(i1); (i1)=(i2); (i2)=tmp; }
#define FMATH_FSWAP( f1, f2 ) {	f32 tmp; tmp=(f1); (f1)=(f2); (f2)=tmp; }
#define FMATH_ISAMESIGN( i1, i2 ) ( ((s32)(i1) ^ (s32)(i2)) >= 0 )
#define FMATH_FSAMESIGN( f1, f2 ) ( ((*(s32 *)&(f1)) ^ (*(s32 *)&(f2))) >= 0 )
#define FMATH_MPH2KPH( fMph ) ((fMph)*(1.0f/0.621f))
#define FMATH_KPH2MPH( fKph ) ((fKph)*0.621f)
#define FMATH_FEET2METERS( feet ) ((feet)*0.3048f)
#define FMATH_METERS2FEET( meters ) ((meters)*(1.0f/0.3048f))
#define FMATH_SQUARE( k ) ( (k) * (k) )
#define FMATH_CUBE( k ) ( (k) * (k) * (k) )
#define FMATH_SETBITMASK( var, bitmask ) ((var) |= (bitmask))
#define FMATH_CLEARBITMASK( var, bitmask ) ((var) &= ~(bitmask))
#define FMATH_SETBIT( var, bitnum ) ((var) |= (1<<(bitnum)))
#define FMATH_CLEARBIT( var, bitnum ) ((var) &= ~(1<<(bitnum)))
#define FMATH_CLAMP_MIN0( v ) FMATH_CLAMPMIN( (v), 0.0f )
#define FMATH_CLAMP_MAX1( v ) FMATH_CLAMPMAX( (v), 1.0f )
#define FMATH_CLAMP_UNIT_FLOAT( a ) FMATH_CLAMP( (a), 0.0f, 1.0f )
#define FMATH_IS_UNIT_FLOAT( v ) ( ( 1.0f >= (v) ) && ( 0.0f <= (v) ) )
#define FMATH_IS_BIPOLAR_UNIT_FLOAT( v ) ( ( 1.0f >= (v) ) && ( -1.0f <= (v) ) )

#define FMATH_WRITEBIT( bSetTheBit, nVariable, nBitNum ) { if( (bSetTheBit) ) { (nVariable) |= (nBitNum); } else { (nVariable) &= ~(nBitNum); } }




//-------------------------------------------------------------------------------------------------------------
// Alignment Macros
//
// Returns the adjusted address.
// For byte alignment, nAlignmentByte must be a power of 2 that's greater than 0.
// For bit alignment, nAlignmentBit must be greater than or equal to 0.
//-------------------------------------------------------------------------------------------------------------
#define FMATH_BYTE_ALIGN_UP( nAddress, nAlignmentByte ) ( ((nAddress) + (nAlignmentByte)-1) & (~((nAlignmentByte)-1)) )
#define FMATH_BYTE_ALIGN_DOWN( nAddress, nAlignmentByte ) ( (nAddress) & (~((nAlignmentByte)-1)) )
#define FMATH_BIT_ALIGN_UP( nAddress, nAlignmentBit ) FMATH_BYTE_ALIGN_UP( nAddress, 1<<(nAlignmentBit) )
#define FMATH_BIT_ALIGN_DOWN( nAddress, nAlignmentBit ) FMATH_BYTE_ALIGN_DOWN( nAddress, 1<<(nAlignmentBit) )




//-------------------------------------------------------------------------------------------------------------
// Floating Point Specialty Macros
//-------------------------------------------------------------------------------------------------------------
#define FMATH_POS_EPSILON						(0.0000001f)
#define FMATH_NEG_EPSILON						(-(FMATH_POS_EPSILON))

#define FMATH_IS_CLOSE_TO_ZERO( fVal )			( ((fVal)<FMATH_POS_EPSILON) && ((fVal)>FMATH_NEG_EPSILON) )

#define	FMATH_FLOAT_TEST_NOT_ZERO( fVal )		((*(u32 *)&fVal)&0x7fffffff)
#define	FMATH_POS_FLOAT_TEST_NOT_ZERO( fVal )	(*(u32 *)&fVal)

#define FMATH_CLOSEST_TO_ZERO					(1.0E-20f)
#define FMATH_POS_CLOSEST_TO_ZERO				FMATH_CLOSEST_TO_ZERO
#define FMATH_NEG_CLOSEST_TO_ZERO				(-FMATH_CLOSEST_TO_ZERO)
#define FMATH_POS_ONE_OVER_ZERO					(1.0f / FMATH_POS_CLOSEST_TO_ZERO)
#define FMATH_NEG_ONE_OVER_ZERO					(1.0f / FMATH_NEG_CLOSEST_TO_ZERO)
#define FMATH_ZERO_CLAMP(v)						if((v)<FMATH_POS_CLOSEST_TO_ZERO && (v)>FMATH_NEG_CLOSEST_TO_ZERO) (v)=0.0f
#define FMATH_SAFE_INV(v)						((v)>=FMATH_POS_CLOSEST_TO_ZERO || (v)<=FMATH_NEG_CLOSEST_TO_ZERO ? 1.0f/(v) \
														: ((v)>=0 ? FMATH_POS_ONE_OVER_ZERO : FMATH_NEG_ONE_OVER_ZERO))
#define FMATH_TOOSMALL_ASSERT(v)				FASSERT((v)>=FMATH_POS_CLOSEST_TO_ZERO || (v)<=FMATH_NEG_CLOSEST_TO_ZERO)

#define FMATH_MAX_FLOAT							(1e+30f)
#define FMATH_MIN_FLOAT							(-1e+30f)
#define FMATH_INFSMALL_FLOAT					(1e-30f)

typedef enum {
    FMATH_FCHECK_RESULT_OK = 0,	// Floating point binary format is ok
    FMATH_FCHECK_RESULT_SNAN,	// Not-a-number
    FMATH_FCHECK_RESULT_QNAN,	// Quiet-not-a-number
    FMATH_FCHECK_RESULT_INF,	// Infinity
    FMATH_FCHECK_RESULT_NINF,	// Negative infinity

	FMATH_FCHECK_RESULT_COUNT
} FMathFcheckResult_e;

#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	#define FMATH_DEBUG_FCHECK(f) FASSERT( fmath_Fcheck(f) == FMATH_FCHECK_RESULT_OK )
	#define FMATH_CLASS_DEBUG_FCHECK(rClass) FASSERT( (rClass).FCheck() == FMATH_FCHECK_RESULT_OK )
#else
	#define FMATH_DEBUG_FCHECK(f)
	#define FMATH_CLASS_DEBUG_FCHECK(rClass)
#endif


class CFVec3A;




//-------------------------------------------------------------------------------------------------------------
// General and Utility Functions
//-------------------------------------------------------------------------------------------------------------
extern BOOL fmath_ModuleStartup( void );
extern void fmath_ModuleShutdown( void );

extern FMathFcheckResult_e fmath_Fcheck( f32 f );

extern u32 fmath_TableLookup_u32( const u32 *pnTable, f32 fIndex, int nTableEntries );
extern s32 fmath_TableLookup_s32( const s32 *pnTable, f32 fIndex, int nTableEntries );
extern f32 fmath_TableLookup_f32( const f32 *pfTable, f32 fIndex, int nTableEntries );

extern u32 fmath_Crc32( u32 nCrc, const u8 *pnData, u32 nDataSize );



//-------------------------------------------------------------------------------------------------------------
// Trig Functions
//-------------------------------------------------------------------------------------------------------------

// These functions are approximations, but FAST:
FINLINE f32 fmath_Sin( f32 fAngle );
FINLINE f32 fmath_Cos( f32 fAngle );
FINLINE void fmath_SinCos( f32 fAngle, f32 *pfSin, f32 *pfCos );
FINLINE f32 fmath_PositiveSin( f32 fAngle );
FINLINE f32 fmath_PositiveCos( f32 fAngle );
FINLINE void fmath_PositiveSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos );
extern f32 fmath_Atan( f32 fNumeratorY, f32 fDenominatorX );

// These functions are more accurate, but SLOW:
FINLINE f32 fmath_AcuSin( f32 fAngle );
FINLINE f32 fmath_AcuCos( f32 fAngle );
FINLINE void fmath_AcuSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos );
FINLINE f32 fmath_AcuPositiveSin( f32 fAngle );
FINLINE f32 fmath_AcuPositiveCos( f32 fAngle );
FINLINE void fmath_AcuPositiveSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos );



//-------------------------------------------------------------------------------------------------------------
// Square Root, Reciprocal, Division, And Power Functions
//-------------------------------------------------------------------------------------------------------------

// These functions are approximations, but FAST:
FINLINE f32 fmath_Sqrt( f32 fVal );
FINLINE f32 fmath_InvSqrt( f32 fVal );
FINLINE f32 fmath_Inv( f32 fVal );
FINLINE f32 fmath_Div( f32 fNumerator, f32 fDenominator );
FINLINE f32 fmath_ApproxUnitPow( f32 fUnitBase, f32 fExp );
FINLINE f32 fmath_ApproxUnitPow( f32 fUnitBase, f32 fExp2, f32 fExp2MinusExp );

// These functions are more accurate, but SLOW:
FINLINE f32 fmath_AcuSqrt( f32 fVal );
FINLINE f32 fmath_AcuInvSqrt( f32 fVal );



//-------------------------------------------------------------------------------------------------------------
// Conversion Functions
//-------------------------------------------------------------------------------------------------------------
FINLINE s32 fmath_FloatToS32( f32 fVal );
FINLINE u32 fmath_FloatToU32( f32 fVal );



//-------------------------------------------------------------------------------------------------------------
// Bitwise Functions
//-------------------------------------------------------------------------------------------------------------
FINLINE s32 fmath_Log2( u32 nVal );
FINLINE BOOL fmath_IsPowerOf2( u32 nVal, BOOL bConsiderZeroAPowerOfTwo );
FINLINE u32 fmath_CountBits( u32 nVal );
FINLINE u32 fmath_LowestBitMask( u32 nVal );
FINLINE u32 fmath_FindFirstSetBit( u32 nVal );
FINLINE u32 fmath_FindPrevPowerOf2( u32 nVal );
FINLINE u32 fmath_FindNextPowerOf2( u32 nVal );



//-------------------------------------------------------------------------------------------------------------
// Random Number Generation Functions
//-------------------------------------------------------------------------------------------------------------
extern void fmath_SetRandomSeed( u32 nNewSeed );
FINLINE u16 fmath_RandomInt16( void );
FINLINE u32 fmath_RandomInt24( void );
FINLINE u32 fmath_RandomInt32( void );
FINLINE f32 fmath_RandomFloat( void );
FINLINE f32 fmath_RandomUnitFloatLessThan1( void );
FINLINE u32 fmath_RandomChoice( u32 nChoice );
FINLINE s32 fmath_RandomRange( s32 nMin, s32 nMax );
FINLINE f32 fmath_RandomBipolarUnitFloat( void );
FINLINE f32 fmath_RandomBipolarUnitFloatWithGap( f32 fPosGap );
FINLINE f32 fmath_RandomFloatRange( f32 fMin, f32 fMax );
FINLINE u32 fmath_RandomChoice( u32 nChoice );
FINLINE BOOL fmath_RandomChance( f32 fChanceOfSuccess );
FINLINE void fmath_ScatterUnitVec( CFVec3A *pUnitVec, f32 fScatterFactor );
FINLINE void fmath_ScatterUnitVec( CFVec3A *pDestUnitVec, const CFVec3A *pSrcUnitVec, f32 fScatterFactor );


//-------------------------------------------------------------------------------------------------------------
// Misc Functions
//-------------------------------------------------------------------------------------------------------------
FINLINE f32 fmath_Fmod( f32 fNumerator, f32 fDenominator );
FINLINE u32 fmath_ExtendedMod( s32 nNumerator, u32 nDenominator );
FINLINE f32 fmath_UnitLinearToSCurve( f32 fUnitVal );
FINLINE f32 fmath_Abs( f32 fVal );
FINLINE s32 fmath_Abs( s32 nVal );



//-------------------------------------------------------------------------------------------------------------
// Global Data:
//-------------------------------------------------------------------------------------------------------------
//extern u32 FMath_nRandomSeed1;
//extern u32 FMath_nRandomSeed2;
extern const f32 FMath_afAtanTable[];
extern const FMath_AtanCoefInfo_t FMath_aAtanQuadrantTable[];
extern f32 FMath_afInterpTable1[];
extern f32 FMath_afInterpTable2[];




//-------------------------------------------------------------------------------------------------------------
// Math Class Declarations:
//-------------------------------------------------------------------------------------------------------------
#include "fmath_vec.h"
#include "fmath_mtx.h"
#include "fmath_quat.h"
#include "fmath_geo.h"

extern u32 fmath_GenRandInt32( void );
extern f32 fmath_GenRandFloat( void );
extern f32 fmath_GenRandFloatLessThanOne( void );

#if FANG_PLATFORM_DX
	extern const CFVec4A FDX8Math_nnnnMask_XYZ1_W0;		// Mask: xyzw = ( 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 )
	extern const CFVec4A FDX8Math_nnnnNegMaskW;			// Mask: xyzw = ( 0x00000000, 0x00000000, 0x00000000, 0x80000000 )
	extern const CFVec4A FDX8Math_NegOnesW1;			// xyzw = ( -1, -1, -1,  1 )
	extern CFVec3A FDX8Math_TempVec3A;					// Used to hold temporary results at times
	extern CFVec4A FDX8Math_TempVec4A;					// Used to hold temporary results at times
	extern f32 FDX8Math_fTemp;							// Used to hold temporary results at times
#elif FANG_PLATFORM_GC
	extern const CFVec4A FGCMath_NegOnesW1;			// xyzw = ( -1, -1, -1,  1 )
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	extern const CFVec4A FPS2Math_NegOnesW1;			// xyzw = ( -1, -1, -1,  1 )
//ARG - <<<<<
#endif



//-------------------------------------------------------------------------------------------------------------
// Inline Function Implementations:
//-------------------------------------------------------------------------------------------------------------
#include "fmath_geo.inl"


#if FANG_PLATFORM_DX
	#include "dx/fdx8math.inl"

	#include "dx/fdx8math_vec.inl"
	#include "dx/fdx8gcmath_vec.inl"

	#include "dx/fdx8math_mtx.inl"
	#include "dx/fdx8gcmath_mtx.inl"

	#include "dx/fdx8math_quat.inl"
	#include "dx/fdx8gcmath_quat.inl"
#elif FANG_PLATFORM_GC
	#include "fGCmath.inl"
	#include "fGCmath_vec.inl"
	#include "fGCmath_mtx.inl"
	#include "fGCmath_quat.inl"
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#include "fps2math.inl"
	#include "fps2math_vec.inl"
	#include "fps2math_mtx.inl"
	#include "fps2math_quat.inl"
//ARG - <<<<<
#else
	#error <Include platform-specific inline functions here>
#endif


// Returns the log (base-2) of n, or zero if n is zero.
FINLINE s32 fmath_Log2( u32 nVal ) {
	int i = 0;

	while( nVal>>=1 ) i++;

	return( i );
}

// Returns TRUE if nVal is a power of 2.
// Returns FALSE otherwise.
//
// If nVal is zero, then bConsiderZeroAPowerOfTwo is returned.
FINLINE BOOL fmath_IsPowerOf2( u32 nVal, BOOL bConsiderZeroAPowerOfTwo ) {
	if( nVal == 0 ) {
		return bConsiderZeroAPowerOfTwo;
	}

	return !(nVal & (nVal - 1));
}

// Returns the number of bits that are 1.
FINLINE u32 fmath_CountBits( u32 nVal ) {
	u32 i, nCount;

	for( i=nCount=0; i<32; i++, nVal>>=1 ) {
		nCount += (nVal & 1);
	}

	return nCount;
}

// Returns the bitmask of the least significant bit that is set.
// If no bits are set, returns 0.
FINLINE u32 fmath_LowestBitMask( u32 nVal ) {
	return (nVal & -(s32)nVal);
}

// Returns the number of least-significant zeros before the first 1.
FINLINE u32 fmath_FindFirstSetBit( u32 nVal ) {
	u32 nCount;

	for( nCount=0; (nCount<32) && !(nVal & 1); nVal>>=1, nCount++ ) ;

	return nCount;
}

// If nVal is not a power-of-2, this function returns the next lower power-of-2.
// If nVal is a power-of-2, this function returns nVal.
// Returns 0 to represent 2^32.
FINLINE u32 fmath_FindPrevPowerOf2( u32 nVal ) {
	u32 i, nBit;

	for( i=0, nBit=1; i<32; i++, nBit<<=1 ) {
		if( nBit > nVal ) {
			return (nBit>>=1);
		}
	}

	return 0;
}

// If nVal is not a power-of-2, this function returns the next higher power-of-2.
// If nVal is a power-of-2, this function returns nVal.
// Returns 0 to represent 2^32.
FINLINE u32 fmath_FindNextPowerOf2( u32 nVal ) {
	u32 i, nBit;

	for( i=0, nBit=1; i<32; i++, nBit<<=1 ) {
		if( nBit > nVal ) {
			return nBit;
		}
	}

	return 0;
}

FINLINE f32 fmath_Fmod( f32 fNumerator, f32 fDenominator ) {
	FASSERT( fDenominator != 0.0f );
	return fNumerator - fDenominator*(f32)( (s32)( fNumerator / fDenominator ) );
}


// Approximates fUnitBase^fExp, where fUnitBase is from 0.0f to 1.0f, and fExp is >= 0.0f.
FINLINE f32 fmath_ApproxUnitPow( f32 fUnitBase, f32 fExp ) {
	f32 fExp2 = fExp * fExp;
	f32 fK = fExp2 - (fExp2-fExp)*fUnitBase;
	return fUnitBase / (fK - fK*fUnitBase + fUnitBase);
}


// If fExp2 and fExp2MinusExp can be precomputed, this is a faster version of the algorithm.
//
// fExp2 = fExp*fExp
// fExp2MinusExp = fExp2 - fExp
FINLINE f32 fmath_ApproxUnitPow( f32 fUnitBase, f32 fExp2, f32 fExp2MinusExp ) {
	f32 fK = fExp2 - (fExp2MinusExp)*fUnitBase;
	return fUnitBase / (fK - fK*fUnitBase + fUnitBase);
}

// Approximate fast sin biased from 0 to +1.
FINLINE f32 fmath_PositiveSin( f32 fAngle ) {
	return 0.5f + 0.5f*fmath_Sin( fAngle );
}

// Approximate fast cos biased from 0 to +1.
FINLINE f32 fmath_PositiveCos( f32 fAngle ) {
	return 0.5f + 0.5f*fmath_Cos( fAngle );
}


// Approximate fast sin & cos biased from 0 to +1.
FINLINE void fmath_PositiveSinCos( f32 fAngle, f32 *pfSin, f32 *pfCos ) {
	*pfSin = fmath_PositiveSin( fAngle );
	*pfCos = fmath_PositiveCos( fAngle );
}

// Returns the arctan of Y/X, in radians.
// Note that atan(y=0/x=0) will return 0.
//
// Note from Mike:
// This function now behaves just like atan2f in that
// it returns the shortest angle in radians.  So the 
// first 2 quadrants are positive angles counter clockwise 
// from the positive x axis, while the second 2 quadrants
// return a negative angle, clockwise from the positive x axis
FINLINE f32 fmath_Atan( f32 fNumeratorY, f32 fDenominatorX ) {
	s32 nOctant;
	f32 fAtanTable;

    nOctant = 0;
    if( fDenominatorX < 0.0f ) {
        nOctant = 2;
        fDenominatorX = -fDenominatorX;
    }
    if( fNumeratorY < 0.0f ) {
        nOctant += 4;
        fNumeratorY = -fNumeratorY;
    }
    if( fNumeratorY > fDenominatorX ) {
		FMATH_FSWAP( fDenominatorX, fNumeratorY );
        nOctant++;
    }

	if( fDenominatorX == 0.0f ) {
		return (nOctant>3 ? 90.0f : 0.0f);
	}

	fAtanTable = fmath_TableLookup_f32( FMath_afAtanTable, fmath_Div( fNumeratorY, fDenominatorX ) * 128.0f, 129 );
	fAtanTable *= FMath_aAtanQuadrantTable[nOctant].fCoef;

	return (fAtanTable + FMath_aAtanQuadrantTable[nOctant].fOffset);
}

// Similar to % (modulo), but doesn't mirror the result around the origin.
FINLINE u32 fmath_ExtendedMod( s32 nNumerator, u32 nDenominator ) {
	if( nNumerator >= 0 ) {
		return nNumerator % nDenominator;
	} else {
		return ( ( nNumerator + nDenominator*( (((u32)(-nNumerator))/nDenominator) + 1 )) % nDenominator );
	}
}

FINLINE f32 fmath_Abs( f32 fVal ) {
	u32 nBits = *(u32 *)&fVal & 0x7fffffff;
	return *(f32 *)&nBits;
}

FINLINE s32 fmath_Abs( s32 nVal ) {
	return nVal & 0x7fffffff;
}

// Performs a linear interpolation on table pnTable.
// fIndex is a floating point index into the table.
// nTableEntries holds the number of entries in the table.
FINLINE u32 fmath_TableLookup_u32( const u32 *pnTable, f32 fIndex, int nTableEntries ) {
	int i;

	i = (int)fIndex;
	if( fIndex <= 0.0f ) return pnTable[0];
	if( i >= nTableEntries-1 ) return pnTable[nTableEntries-1];

	fIndex -= (f32)i;
	return (u32)( pnTable[i]*(1.0f-fIndex) + pnTable[i+1]*fIndex );
}

// See fmath_TableLookup_u32 for details.
FINLINE s32 fmath_TableLookup_s32( const s32 *pnTable, f32 fIndex, int nTableEntries ) {
	int i;

	i = (int)fIndex;
	if( fIndex <= 0.0f ) return pnTable[0];
	if( i >= nTableEntries-1 ) return pnTable[nTableEntries-1];

	fIndex -= (f32)i;
	return (s32)( pnTable[i]*(1.0f-fIndex) + pnTable[i+1]*fIndex );
}

// See fmath_TableLookup_u32 for details.
FINLINE f32 fmath_TableLookup_f32( const f32 *pfTable, f32 fIndex, int nTableEntries ) {
	int i;

	i = (int)fIndex;
	if( fIndex <= 0.0f ) return pfTable[0];
	if( i >= nTableEntries-1 ) return pfTable[nTableEntries-1];

	fIndex -= (f32)i;
	return ( pfTable[i]*(1.0f-fIndex) + pfTable[i+1]*fIndex );
}

FINLINE f32 fmath_UnitLinearToSCurve( f32 fUnitVal ) {
	return fmath_PositiveCos( FMATH_PI * (1.0f + fUnitVal) );
}

// Returns a random unsigned 16-bit integer.
FINLINE u16 fmath_RandomInt16( void ) {
	return fmath_RandomInt32() & 0xffff;
}

// Returns a random unsigned 24-bit integer.
FINLINE u32 fmath_RandomInt24( void ) {
	return fmath_RandomInt32() & 0xffffff;
}

// Returns a random unsigned 32-bit integer.
FINLINE u32 fmath_RandomInt32( void ) {
	return fmath_GenRandInt32();
}

// Returns a random floating point number from 0.0f to 1.0f.
FINLINE f32 fmath_RandomFloat( void ) {
	return fmath_GenRandFloat();
}

// Returns a random floating point number from 0.0f to 1.0f, but not including 1.0f.
FINLINE f32 fmath_RandomUnitFloatLessThan1( void ) {
	return fmath_GenRandFloatLessThanOne();
}

// Returns a random floating point number from -1.0f to 1.0f.
FINLINE f32 fmath_RandomBipolarUnitFloat( void ) {
	return fmath_RandomFloat()*2.0f - 1.0f;
}

// Returns a random floating point number from -1.0f to -fPosGap or fPosGap to 1.0f
FINLINE f32 fmath_RandomBipolarUnitFloatWithGap( f32 fPosGap ) {
	FASSERT( fPosGap <= 1.0f && fPosGap >= 0.0f );

	f32 fTemp = 2.0f * fPosGap;
	f32 fRand = fmath_RandomFloatRange( fTemp - 1.0f, 1.0f );
	if( fRand < fPosGap ) {
		fRand -= fTemp;
	}
	return fRand;
}

// Returns a random floating point number from fMin to fMax, inclusive.
FINLINE f32 fmath_RandomFloatRange( f32 fMin, f32 fMax ) {
	f32 fRandUnitFloat;

	fRandUnitFloat = fmath_RandomFloat();

	return FMATH_FPOT( fRandUnitFloat, fMin, fMax );
}

// Returns a random integer in the range 0<=rand<nChoice.
// nChoice must be a value from 0 to 65535 (16-bits).
FINLINE u32 fmath_RandomChoice( u32 nChoice ) {
	FASSERT( nChoice <= 0xffff );
	return ((u32)fmath_RandomInt16()*nChoice) >> 16;
}

#if 0
// Fills pDestVec with a random bipolar unit vector.
FINLINE void fmath_RandomUnitVec2( CFVec2 *pDestVec ) {
	pDestVec->x = fmath_RandomBipolarUnitFloat();
	pDestVec->y = fmath_RandomBipolarUnitFloat();

	if( pDestVec->x==0.0f && pDestVec->y==0.0f ) {
		pDestVec->x = 1.0f;
	}

	pDestVec->Unitize();
}

// Fills pDestVec with a random bipolar unit vector.
FINLINE void fmath_RandomUnitVec3( CFVec3 *pDestVec ) {
	pDestVec->x = fmath_RandomBipolarUnitFloat();
	pDestVec->y = fmath_RandomBipolarUnitFloat();
	pDestVec->z = fmath_RandomBipolarUnitFloat();

	if( pDestVec->x==0.0f && pDestVec->y==0.0f && pDestVec->z==0.0f ) {
		pDestVec->x = 1.0f;
	}

	pDestVec->Unitize();
}
#endif

// Returns a random integer whose range is from nMin to nMax, inclusive.
FINLINE s32 fmath_RandomRange( s32 nMin, s32 nMax ) {
	f32 fRange;
	s32 nRandInt;

	FASSERT( nMax >= nMin );

	fRange = (f32)(nMax-nMin+1);
	nRandInt = nMin + (s32)(fmath_RandomUnitFloatLessThan1()*fRange);

	return nRandInt;
}


// Returns a BOOL based on the probability of chance:
//   If fChanceOfSuccess is 0.0f, FALSE is always returned.
//   If fChanceOfSuccess is 1.0f, TRUE is always returned.
//   If fChanceOfSuccess is between 0.0f and 1.0f, a probability of TRUE/FALSE is returned.
FINLINE BOOL fmath_RandomChance( f32 fChanceOfSuccess ) {
	FASSERT( fChanceOfSuccess>=0.0f && fChanceOfSuccess<=1.0f );
	return fmath_RandomFloat() < fChanceOfSuccess;
}


FINLINE void fmath_ScatterUnitVec( CFVec3A *pUnitVec, f32 fScatterFactor ) {
	pUnitVec->x += fScatterFactor * fmath_RandomBipolarUnitFloat();
	pUnitVec->y += fScatterFactor * fmath_RandomBipolarUnitFloat();
	pUnitVec->z += fScatterFactor * fmath_RandomBipolarUnitFloat();
	pUnitVec->Unitize();
}


FINLINE void fmath_ScatterUnitVec( CFVec3A *pDestUnitVec, const CFVec3A *pSrcUnitVec, f32 fScatterFactor ) {
	pDestUnitVec->x = pSrcUnitVec->x + fScatterFactor * fmath_RandomBipolarUnitFloat();
	pDestUnitVec->y = pSrcUnitVec->y + fScatterFactor * fmath_RandomBipolarUnitFloat();
	pDestUnitVec->z = pSrcUnitVec->z + fScatterFactor * fmath_RandomBipolarUnitFloat();
	pDestUnitVec->Unitize();
}


#endif

