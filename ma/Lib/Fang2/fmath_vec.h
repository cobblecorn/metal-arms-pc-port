//////////////////////////////////////////////////////////////////////////////////////
// fmath_vec.h - Fang vector library.
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
// 02/07/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FMATH_VEC_H_
#define _FMATH_VEC_H_ 1

#include "fang.h"

#if FANG_PLATFORM_DX
	#include "xmmintrin.h"
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#undef _A
	#undef _B
//ARG - <<<<<
#endif


class CFVec2;
class CFVec3;
class CFVec4;
class CFVec3A;
class CFVec4A;



//--------------------------------------------------------------------
// CFVec2 Definition:
//--------------------------------------------------------------------
class CFVec2 {
public:
	union {
		struct { f32 x, y; };
		f32 a[2];
	};

	static const CFVec2 m_Null;
	static const CFVec2 m_UnitAxisX;
	static const CFVec2 m_UnitAxisY;

	// Constructors:
	FINLINE CFVec2( void );
	FINLINE CFVec2( const f32 &fX, const f32 &fY );
	FINLINE CFVec2( const CFVec2 &v );
	FINLINE CFVec2( const CFVec3 &v );
	FINLINE CFVec2( const CFVec4 &v );

	// Assignment:
	FINLINE CFVec2 &Zero( void );
	FINLINE CFVec2 &Set( const f32 &fX, const f32 &fY );
	FINLINE CFVec2 &operator =  ( const CFVec2 &v );
	FINLINE CFVec2 &operator =  ( const CFVec3 &v );
	FINLINE CFVec2 &operator =  ( const CFVec4 &v );
	FINLINE CFVec2 &operator += ( const CFVec2 &v );
	FINLINE CFVec2 &operator -= ( const CFVec2 &v );
	FINLINE CFVec2 &operator *= ( const f32 &fS );
	FINLINE CFVec2 &operator /= ( const f32 &fS );

	// Unary Operators:
	FINLINE CFVec2 operator - ( void ) const;

	// Binary Operators:
	FINLINE CFVec2 operator + ( const f32 &fS ) const;
	FINLINE CFVec2 operator - ( const f32 &fS ) const;
	FINLINE CFVec2 operator * ( const f32 &fS ) const;
	FINLINE CFVec2 operator / ( const f32 &fS ) const;
	FINLINE CFVec2 operator + ( const CFVec2 &v ) const;
	FINLINE CFVec2 operator - ( const CFVec2 &v ) const;
	FINLINE CFVec2 operator * ( const CFVec2 &v ) const;
	FINLINE CFVec2 operator / ( const CFVec2 &v ) const;

	// Comparison:
	FINLINE BOOL operator == ( const CFVec2 &v ) const;
	FINLINE BOOL operator != ( const CFVec2 &v ) const;

	// Special Methods:
	FINLINE f32 Dot( const CFVec2 &v ) const;
	FINLINE f32 Mag() const;
	FINLINE f32 Mag2() const;
	FINLINE f32 InvMag() const;
	FINLINE f32 InvMag2() const;
	FINLINE CFVec2 &Unitize();
	FINLINE CFVec2 Unit() const;
	FINLINE f32 ExtractUnitAndMag( CFVec2 &rUnitVec ) const;
	FINLINE f32 SafeExtractUnitAndMag( CFVec2 &rUnitVec ) const;
	FINLINE f32 ExtractUnitAndInvMag( CFVec2 &rUnitVec ) const;
	FINLINE f32 SafeExtractUnitAndInvMag( CFVec2 &rUnitVec ) const;
	FINLINE CFVec2 &ReceiveLerpOf( f32 fUnitLerp, const CFVec2 &rVec0, const CFVec2 &rVec1 );
	FINLINE CFVec2 &Rotate( const f32 &fRadians );
	FINLINE CFVec2 &Min( const CFVec2 &v );
	FINLINE CFVec2 &Max( const CFVec2 &v );

	void ChangeEndian( void ) {
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
	}

	FCLASS_STACKMEM_NOALIGN( CFVec2 );
};



//--------------------------------------------------------------------
// CFVec3 Definition:
//--------------------------------------------------------------------
class CFVec3 {
public:
	union {
		struct { CFVec2 v2; };
		struct { f32 x, y, z; };
		f32 a[3];
	};

	static const CFVec3 m_Null;
	static const CFVec3 m_UnitAxisX;
	static const CFVec3 m_UnitAxisY;
	static const CFVec3 m_UnitAxisZ;

	// Constructors:
	FINLINE CFVec3( void );
	FINLINE CFVec3( const f32 &fX, const f32 &fY, const f32 &fZ );
	FINLINE CFVec3( const CFVec2 &v, f32 fZ=0.0f );
	FINLINE CFVec3( const CFVec3 &v );
	FINLINE CFVec3( const CFVec4 &v );

	// Assignment:
	FINLINE CFVec3 &Zero( void );
	FINLINE CFVec3 &Set( const f32 &fX, const f32 &fY, const f32 &fZ );
	FINLINE CFVec3 &operator =  ( const CFVec2 &v );
	FINLINE CFVec3 &operator =  ( const CFVec3 &v );
	FINLINE CFVec3 &operator =  ( const CFVec4 &v );
	FINLINE CFVec3 &operator += ( const CFVec3 &v );
	FINLINE CFVec3 &operator -= ( const CFVec3 &v );
	FINLINE CFVec3 &operator *= ( const f32 &fS );
	FINLINE CFVec3 &operator /= ( const f32 &fS );

	// Unary Operators:
	FINLINE CFVec3 operator - ( void ) const;

	// Binary Operators:
	FINLINE CFVec3 operator + ( const f32 &fS ) const;
	FINLINE CFVec3 operator - ( const f32 &fS ) const;
	FINLINE CFVec3 operator * ( const f32 &fS ) const;
	FINLINE CFVec3 operator / ( const f32 &fS ) const;
	FINLINE CFVec3 operator + ( const CFVec3 &v ) const;
	FINLINE CFVec3 operator - ( const CFVec3 &v ) const;
	FINLINE CFVec3 operator * ( const CFVec3 &v ) const;
	FINLINE CFVec3 operator / ( const CFVec3 &v ) const;

	// Comparison:
	FINLINE BOOL operator == ( const CFVec3 &v ) const;
	FINLINE BOOL operator != ( const CFVec3 &v ) const;

	// Addition:
	FINLINE CFVec3 &Add( const CFVec3 &rV1, const CFVec3 &rV2 ){ *this = (rV1+rV2); return *this;}
	FINLINE CFVec3 &Add( const CFVec3 &rV ){ *this += rV; return *this;}
	FINLINE CFVec3 &Add( const CFVec3 &rV, const f32 &fVal ){ *this = (rV+fVal); return *this;}
	FINLINE CFVec3 &Add( const f32 &fVal ){ *this =(*this + fVal); return *this;}

	// Subtraction:
	FINLINE CFVec3 &Sub( const CFVec3 &rV1, const CFVec3 &rV2 ){ *this = (rV1-rV2); return *this;}
	FINLINE CFVec3 &Sub( const CFVec3 &rV ){ *this -= (rV); return *this;}
	FINLINE CFVec3 &Sub( const CFVec3 &rV, const f32 &fVal ){ *this = (rV-fVal); return *this;}
	FINLINE CFVec3 &Sub( const f32 &fVal ){ *this =(*this - fVal); return *this;}

	// Multiplication:
	FINLINE CFVec3 &Mul( const CFVec3 &rV1, const CFVec3 &rV2 ){ *this = (rV1*rV2); return *this;}
	FINLINE CFVec3 &Mul( const CFVec3 &rV ){ *this = (*this * rV); return *this;}
	FINLINE CFVec3 &Mul( const CFVec3 &rV, const f32 &fVal ){ *this = (rV*fVal); return *this;}
	FINLINE CFVec3 &Mul( const f32 &fVal ){ *this *= (fVal); return *this;}

	// Special Methods:
	FINLINE CFVec3 Cross( const CFVec3 &v ) const;
	FINLINE CFVec3 UnitCross( const CFVec3 &v ) const;
	FINLINE f32 Dot( const CFVec3 &v ) const;
	FINLINE f32 Mag() const;
	FINLINE f32 Mag2() const;
	FINLINE f32 InvMag() const;
	FINLINE f32 InvMag2() const;
	FINLINE f32 MagXZ() const;
	FINLINE f32 MagXZ2() const;
	FINLINE f32 InvMagXZ() const;
	FINLINE f32 InvMagXZ2() const;
	FINLINE CFVec3 &Unitize();
	FINLINE CFVec3 Unit() const;
	FINLINE CFVec3 &UnitizeXZ();
	FINLINE CFVec3 UnitXZ() const;
	FINLINE f32 ExtractUnitAndMag( CFVec3 &rUnitVec ) const;
	FINLINE f32 SafeExtractUnitAndMag( CFVec3 &rUnitVec ) const;
	FINLINE f32 ExtractUnitAndInvMag( CFVec3 &rUnitVec ) const;
	FINLINE f32 SafeExtractUnitAndInvMag( CFVec3 &rUnitVec ) const;
	FINLINE CFVec3 &ReceiveLerpOf( f32 fUnitLerp, const CFVec3 &rVec0, const CFVec3 &rVec1 );
	FINLINE CFVec3 &Min( const CFVec3 &v );
	FINLINE CFVec3 &Max( const CFVec3 &v );
	FINLINE void RotateX( const f32 fRadians );
	FINLINE void RotateY( const f32 fRadians );
	FINLINE void RotateDegX( const f32 fDegrees );
	FINLINE void RotateDegY( const f32 fDegrees );
	
	// Reflection:
	FINLINE CFVec3 &Reflect( const CFVec3 &rNormal );
	FINLINE CFVec3 &ReceiveReflection( const CFVec3 &rV, const CFVec3 &rNormal );


	void ChangeEndian( void ) {
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
	}

	FCLASS_STACKMEM_NOALIGN( CFVec3 );
};



//--------------------------------------------------------------------
// CFVec4 Definition:
//--------------------------------------------------------------------
class CFVec4 {
public:
	union {
		struct { CFVec2 v2; };
		struct { CFVec3 v3; };
		struct { f32 x, y, z, w; };
		f32 a[4];
	};

	static const CFVec4 m_Null;
	static const CFVec4 m_UnitAxisX;
	static const CFVec4 m_UnitAxisY;
	static const CFVec4 m_UnitAxisZ;
	static const CFVec4 m_UnitAxisQ;

	// Constructors:
	FINLINE CFVec4( void );
	FINLINE CFVec4( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW );
	FINLINE CFVec4( const CFVec2 &v, f32 fZ=0.0f, f32 fW=0.0f );
	FINLINE CFVec4( const CFVec3 &v, f32 fW=0.0f );
	FINLINE CFVec4( const CFVec4 &v );

	// Assignment:
	FINLINE CFVec4 &Zero( void );
	FINLINE CFVec4 &Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW );
	FINLINE CFVec4 &operator =  ( const CFVec4 &v );
	FINLINE CFVec4 &operator =  ( const CFVec3 &v );
	FINLINE CFVec4 &operator =  ( const CFVec2 &v );
	FINLINE CFVec4 &operator += ( const CFVec4 &v );
	FINLINE CFVec4 &operator -= ( const CFVec4 &v );
	FINLINE CFVec4 &operator *= ( const CFVec4 &v );
	FINLINE CFVec4 &operator *= ( const f32 &fS );
	FINLINE CFVec4 &operator /= ( const f32 &fS );

	// Unary Operators:
	FINLINE CFVec4 operator - ( void ) const;

	// Binary Operators:
	FINLINE CFVec4 operator + ( const f32 &fS ) const;
	FINLINE CFVec4 operator - ( const f32 &fS ) const;
	FINLINE CFVec4 operator * ( const f32 &fS ) const;
	FINLINE CFVec4 operator / ( const f32 &fS ) const;
	FINLINE CFVec4 operator + ( const CFVec4 &v ) const;
	FINLINE CFVec4 operator - ( const CFVec4 &v ) const;
	FINLINE CFVec4 operator * ( const CFVec4 &v ) const;
	FINLINE CFVec4 operator / ( const CFVec4 &v ) const;

	// Comparison:
	FINLINE BOOL operator == ( const CFVec4 &v ) const;
	FINLINE BOOL operator != ( const CFVec4 &v ) const;

	// Special Methods:
	FINLINE f32 Dot( const CFVec4 &v ) const;
	FINLINE f32 Mag() const;
	FINLINE f32 Mag2() const;
	FINLINE f32 InvMag() const;
	FINLINE f32 InvMag2() const;
	FINLINE CFVec4 &Unitize();
	FINLINE CFVec4 Unit() const;
	FINLINE f32 ExtractUnitAndMag( CFVec4 &rUnitVec ) const;
	FINLINE f32 SafeExtractUnitAndMag( CFVec4 &rUnitVec ) const;
	FINLINE f32 ExtractUnitAndInvMag( CFVec4 &rUnitVec ) const;
	FINLINE f32 SafeExtractUnitAndInvMag( CFVec4 &rUnitVec ) const;
	FINLINE CFVec4 &ReceiveLerpOf( f32 fUnitLerp, const CFVec4 &rVec0, const CFVec4 &rVec1 );

	void ChangeEndian( void ) {
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
		w = fang_ConvertEndian( w );
	}

	FCLASS_STACKMEM_NOALIGN( CFVec4 );
};



//--------------------------------------------------------------------
// CFVec4A Definition:
//--------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFVec4A {
public:
	union {
#if FANG_PLATFORM_DX && !FANG_WINGC
		__m128 _m;								// Private SIMD form (aligns on a 16-byte boundary and allows us to use XMM intrinsics)
#elif FANG_PLATFORM_GC
		struct { __vec2x32float__ _A, _B; };
#endif
		struct { f32 x, y, z, w; };				// Individual components form
		struct { CFVec2 v2; };					// CFVec2 form
		struct { CFVec3 v3; };					// CFVec3 form
		struct { CFVec4 v4; };					// CFVec4 form
		f32 a[4];								// Array form
	};

	static const CFVec4A m_Null;				// xyzw = (  0,  0,  0,  0 )
	static const CFVec4A m_NullW1;				// xyzw = (  0,  0,  0,  1 )
	static const CFVec4A m_Ones;				// xyzw = (  1,  1,  1,  1 )
	static const CFVec4A m_NegOnes;				// xyzw = ( -1, -1, -1, -1 )
	static const CFVec4A m_NegOnesW0;			// xyzw = ( -1, -1, -1,  0 )
	static const CFVec4A m_NegOnesW1;			// xyzw = ( -1, -1, -1,  1 )
	static const CFVec4A m_UnitAxisX;			// xyzw = (  1,  0,  0,  0 )
	static const CFVec4A m_UnitAxisY;			// xyzw = (  0,  1,  0,  0 )
	static const CFVec4A m_UnitAxisZ;			// xyzw = (  0,  0,  1,  0 )
	static const CFVec4A m_UnitAxisW;			// xyzw = (  0,  0,  0,  1 )
	static const CFVec4A m_HalfVec;				// xyzw = (  0.5f,  0.5f,  0.5f,  0.5f )
	static const CFVec4A m_NegHalfVec;			// xyzw = ( -0.5f, -0.5f, -0.5f, -0.5f )
	static const CFVec4A m_DoubleVec;			// xyzw = (  2,  2,  2,  2 )
	static const CFVec4A m_NegDoubleVec;		// xyzw = ( -2, -2, -2, -2 )

protected:
	static CFVec4A m_TempVec;

public:
	// Constructors:
	FINLINE CFVec4A( void );
	FINLINE CFVec4A( const CFVec4A &rV );
	FINLINE CFVec4A( const f32 &fVal );
	FINLINE CFVec4A( const f32 &fX, const f32 &fY, const f32 &fZ );
	FINLINE CFVec4A( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW );
	FINLINE CFVec4A( const f32 *pfArray4 );
	FINLINE CFVec4A( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ, const u32 &nBitMaskW );

	// Comparison:
	FINLINE BOOL operator == ( const CFVec4A &rV ) const;
	FINLINE BOOL operator != ( const CFVec4A &rV ) const;

	// Initialization:
	FINLINE CFVec4A &Zero( void );
	FINLINE CFVec4A &ZeroW1( void );

	FINLINE CFVec4A &operator = ( const CFVec4A &rV );
	FINLINE CFVec4A &Set( const CFVec2 &rV );
	FINLINE CFVec4A &Set( const CFVec3 &rV );
	FINLINE CFVec4A &Set( const CFVec4 &rV );
	FINLINE CFVec4A &Set( const CFVec3A &rV );
	FINLINE CFVec4A &Set( const CFVec4A &rV );
	FINLINE CFVec4A &Set( const f32 &fVal );
	FINLINE CFVec4A &Set( const f32 &fX, const f32 &fY, const f32 &fZ );
	FINLINE CFVec4A &Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW );
	FINLINE CFVec4A &Set( const f32 *pfArray4 );
	FINLINE CFVec4A &SetBitMask( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ, const u32 &nBitMaskW );
	FINLINE CFVec4A &SetToInverse( const f32 &fVal );

	// Negation:
	FINLINE CFVec4A &ReceiveNegative( const CFVec4A &rV );
	FINLINE CFVec4A &Negate( void );

	// Addition:
	FINLINE CFVec4A &Add( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec4A &Add( const CFVec4A &rV );
	FINLINE CFVec4A &Add( const CFVec4A &rV, const f32 &fVal );
	FINLINE CFVec4A &Add( const f32 &fVal );

	// Subtraction:
	FINLINE CFVec4A &Sub( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec4A &Sub( const CFVec4A &rV );
	FINLINE CFVec4A &Sub( const CFVec4A &rV, const f32 &fVal );
	FINLINE CFVec4A &Sub( const f32 &fVal );

	// Reverse Subtraction:
	FINLINE CFVec4A &RevSub( const CFVec4A &rV );
	FINLINE CFVec4A &RevSub( const f32 &fVal );

	// Multiplication:
	FINLINE CFVec4A &Mul( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec4A &Mul( const CFVec4A &rV );
	FINLINE CFVec4A &Mul( const CFVec4A &rV, const f32 &fVal );
	FINLINE CFVec4A &Mul( const f32 &fVal );

	// Division:
	FINLINE CFVec4A &Div( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec4A &Div( const CFVec4A &rV );
	FINLINE CFVec4A &Div( const CFVec4A &rV, const f32 &fVal );
	FINLINE CFVec4A &Div( const f32 &fVal );

	// Reciprocal:
	FINLINE CFVec4A &ReceiveInverse( const CFVec4A &rV );
	FINLINE CFVec4A &Invert( void );

	// Min/Max:
	FINLINE CFVec4A &Min( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec4A &Max( const CFVec4A &rV1, const CFVec4A &rV2 );

	// Clamping:
	FINLINE CFVec4A &Clamp0( void );
	FINLINE CFVec4A &Clamp1( void );
	FINLINE CFVec4A &Clamp01( void );
	FINLINE CFVec4A &ClampNeg1( void );
	FINLINE CFVec4A &ClampNeg1Pos1( void );
	FINLINE CFVec4A &ClampMin( const CFVec4A &rMinV );
	FINLINE CFVec4A &ClampMax( const CFVec4A &rMaxV );
	FINLINE CFVec4A &Clamp( const CFVec4A &rMinV, const CFVec4A &rMaxV );

	FINLINE CFVec4A &Clamp0( const CFVec4A &rTestV );
	FINLINE CFVec4A &Clamp1( const CFVec4A &rTestV );
	FINLINE CFVec4A &Clamp01( const CFVec4A &rTestV );
	FINLINE CFVec4A &ClampNeg1( const CFVec4A &rTestV );
	FINLINE CFVec4A &ClampNeg1Pos1( const CFVec4A &rTestV );
	FINLINE CFVec4A &ClampMin( const CFVec4A &rTestV, const CFVec4A &rMinV );
	FINLINE CFVec4A &ClampMax( const CFVec4A &rTestV, const CFVec4A &rMaxV );
	FINLINE CFVec4A &Clamp( const CFVec4A &rTestV, const CFVec4A &rMinV, const CFVec4A &rMaxV );

	// Unit & Magnitude:
	FINLINE f32 Dist( const CFVec4A &rV ) const;
	FINLINE f32 DistSq( const CFVec4A &rV ) const;

	FINLINE f32 Mag( void ) const;
	FINLINE f32 MagSq( void ) const;
	FINLINE f32 InvMag( void ) const;
	FINLINE f32 InvMagSq( void ) const;

	FINLINE CFVec4A &ReceiveUnit( const CFVec4A &rV );
	FINLINE CFVec4A &Unitize( void );

	FINLINE f32 UnitAndMag( const CFVec4A &rV );
	FINLINE f32 UnitAndInvMag( const CFVec4A &rV );
	FINLINE f32 SafeUnitAndMag( const CFVec4A &rV );
	FINLINE f32 SafeUnitAndInvMag( const CFVec4A &rV );

	FINLINE f32 DistXZ( const CFVec4A &rV ) const;
	FINLINE f32 DistSqXZ( const CFVec4A &rV ) const;

	FINLINE f32 MagXZ( void ) const;
	FINLINE f32 MagSqXZ( void ) const;
	FINLINE f32 InvMagXZ( void ) const;
	FINLINE f32 InvMagSqXZ( void ) const;

	FINLINE CFVec4A &ReceiveUnitXZ( const CFVec4A &rV );
	FINLINE CFVec4A &UnitizeXZ( void );

	FINLINE f32 UnitAndMagXZ( const CFVec4A &rV );
	FINLINE f32 UnitAndInvMagXZ( const CFVec4A &rV );
	FINLINE f32 SafeUnitAndMagXZ( const CFVec4A &rV );
	FINLINE f32 SafeUnitAndInvMagXZ( const CFVec4A &rV );

	// Cross Product:
	FINLINE CFVec4A &Cross( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec4A &Cross( const CFVec4A &rV );
	FINLINE CFVec4A &UnitCross( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec4A &UnitCross( const CFVec4A &rV );

	FINLINE CFVec4A &Cross( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec4A &Cross( const CFVec3A &rV );
	FINLINE CFVec4A &UnitCross( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec4A &UnitCross( const CFVec3A &rV );

	// Dot Product:
	FINLINE f32 Dot( const CFVec4A &rV ) const;

	// Lerp:
	FINLINE CFVec4A &Lerp( const f32 &fUnitVal, const CFVec4A &rV1, const CFVec4A &rV2 );

	// Rotation:
	FINLINE CFVec4A &RotateX( const f32 &fRadians );
	FINLINE CFVec4A &RotateY( const f32 &fRadians );
	FINLINE CFVec4A &RotateZ( const f32 &fRadians );

	FINLINE CFVec4A &RotateX( const f32 &fSin, const f32 &fCos );
	FINLINE CFVec4A &RotateY( const f32 &fSin, const f32 &fCos );
	FINLINE CFVec4A &RotateZ( const f32 &fSin, const f32 &fCos );

	FINLINE CFVec4A &ReceiveRotationX( const CFVec4A &rV, const f32 &fRadians );
	FINLINE CFVec4A &ReceiveRotationY( const CFVec4A &rV, const f32 &fRadians );
	FINLINE CFVec4A &ReceiveRotationZ( const CFVec4A &rV, const f32 &fRadians );

	FINLINE CFVec4A &ReceiveRotationX( const CFVec4A &rV, const f32 &fSin, const f32 &fCos );
	FINLINE CFVec4A &ReceiveRotationY( const CFVec4A &rV, const f32 &fSin, const f32 &fCos );
	FINLINE CFVec4A &ReceiveRotationZ( const CFVec4A &rV, const f32 &fSin, const f32 &fCos );

	// Misc:
	FINLINE u32 GenKey( void ) const;
	void ChangeEndian( void ) {
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
		w = fang_ConvertEndian( w );
	}

	// Debug:
	FMathFcheckResult_e FCheck( void ) const;

	FCLASS_STACKMEM_ALIGN( CFVec4A );
} FCLASS_ALIGN_SUFFIX;



//--------------------------------------------------------------------
// CFVec3A Definition:
//--------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFVec3A {
public:
	union {
#if FANG_PLATFORM_DX && !FANG_WINGC
		__m128 _m;								// Private SIMD form (aligns on a 16-byte boundary and allows us to use XMM intrinsics)
#elif FANG_PLATFORM_GC
		struct { __vec2x32float__ _A, _B; };
#endif
		struct { f32 x, y, z, w; };				// Individual components form (w is always 0)
		struct { CFVec2 v2; };					// CFVec2 form
		struct { CFVec3 v3; };					// CFVec3 form
		struct { CFVec4A v4a; };				// CFVec4A form (careful using this because CFVec3A::w MUST always be 0)
		f32 a[4];								// Array form
	};

	static const CFVec3A m_Null;				// xyzw = (  0,  0,  0,  0 )
	static const CFVec3A m_Ones;				// xyzw = (  1,  1,  1,  0 )
	static const CFVec3A m_NegOnes;				// xyzw = ( -1, -1, -1,  0 )
	static const CFVec3A m_UnitAxisX;			// xyzw = (  1,  0,  0,  0 )
	static const CFVec3A m_UnitAxisY;			// xyzw = (  0,  1,  0,  0 )
	static const CFVec3A m_UnitAxisZ;			// xyzw = (  0,  0,  1,  0 )
	static const CFVec3A m_HalfVec;				// xyzw = (  0.5f,  0.5f,  0.5f )
	static const CFVec3A m_NegHalfVec;			// xyzw = ( -0.5f, -0.5f, -0.5f )
	static const CFVec3A m_DoubleVec;			// xyzw = (  2,  2,  2, 0  )
	static const CFVec3A m_NegDoubleVec;		// xyzw = ( -2, -2, -2, 0  )

protected:
	static CFVec3A m_TempVec;

public:
	// Constructors:
	FINLINE CFVec3A( void );
	FINLINE CFVec3A( const CFVec3 &rV );
	FINLINE CFVec3A( const CFVec3A &rV );
	FINLINE CFVec3A( const f32 &fVal );
	FINLINE CFVec3A( const f32 &fX, const f32 &fY, const f32 &fZ );
	FINLINE CFVec3A( const f32 *pfArray3 );
	FINLINE CFVec3A( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ );

	FINLINE f32 GetY( void );

	// Comparison:
	FINLINE BOOL operator == ( const CFVec3A &rV ) const;
	FINLINE BOOL operator != ( const CFVec3A &rV ) const;

	// Initialization:
	FINLINE CFVec3A &Zero( void );

	FINLINE CFVec3A &operator = ( const CFVec3A &rV );
	FINLINE CFVec3A &Set( const CFVec2 &rV );
	FINLINE CFVec3A &Set( const CFVec3 &rV );
	FINLINE CFVec3A &Set( const CFVec4 &rV );
	FINLINE CFVec3A &Set( const CFVec3A &rV );
	FINLINE CFVec3A &Set( const CFVec4A &rV );
	FINLINE CFVec3A &Set( const f32 &fVal );
	FINLINE CFVec3A &Set( const f32 &fX, const f32 &fY, const f32 &fZ );
	FINLINE CFVec3A &Set( const f32 *pfArray3 );
	FINLINE CFVec3A &SetBitMask( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ );
	FINLINE CFVec3A &SetToInverse( const f32 &fVal );

	CFVec3A &SetUnitRandom( void );

	// Negation:
	FINLINE CFVec3A &ReceiveNegative( const CFVec3A &rV );
	FINLINE CFVec3A &Negate( void );

	// Addition:
	FINLINE CFVec3A &Add( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &Add( const CFVec3A &rV );
	FINLINE CFVec3A &Add( const CFVec3A &rV, const f32 &fVal );
	FINLINE CFVec3A &Add( const f32 &fVal );

	// Subtraction:
	FINLINE CFVec3A &Sub( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &Sub( const CFVec3A &rV );
	FINLINE CFVec3A &Sub( const CFVec3A &rV, const f32 &fVal );
	FINLINE CFVec3A &Sub( const f32 &fVal );

	// Reverse Subtraction:
	FINLINE CFVec3A &RevSub( const CFVec3A &rV );
	FINLINE CFVec3A &RevSub( const f32 &fVal );

	// Multiplication:
	FINLINE CFVec3A &Mul( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &Mul( const CFVec3A &rV );
	FINLINE CFVec3A &Mul( const CFVec3A &rV, const f32 &fVal );
	FINLINE CFVec3A &Mul( const f32 &fVal );

	// Division:
	FINLINE CFVec3A &Div( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &Div( const CFVec3A &rV );
	FINLINE CFVec3A &Div( const CFVec3A &rV, const f32 &fVal );
	FINLINE CFVec3A &Div( const f32 &fVal );

	// Reciprocal:
	FINLINE CFVec3A &ReceiveInverse( const CFVec3A &rV );
	FINLINE CFVec3A &Invert( void );

	// Min/Max:
	FINLINE CFVec3A &Min( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &Max( const CFVec3A &rV1, const CFVec3A &rV2 );

	FINLINE CFVec3A &Clamp0( void );
	FINLINE CFVec3A &Clamp1( void );
	FINLINE CFVec3A &Clamp01( void );
	FINLINE CFVec3A &ClampNeg1( void );
	FINLINE CFVec3A &ClampNeg1Pos1( void );
	FINLINE CFVec3A &ClampMin( const CFVec3A &rMinV );
	FINLINE CFVec3A &ClampMax( const CFVec3A &rMaxV );
	FINLINE CFVec3A &Clamp( const CFVec3A &rMinV, const CFVec3A &rMaxV );

	FINLINE CFVec3A &Clamp0( const CFVec3A &rTestV );
	FINLINE CFVec3A &Clamp1( const CFVec3A &rTestV );
	FINLINE CFVec3A &Clamp01( const CFVec3A &rTestV );
	FINLINE CFVec3A &ClampNeg1( const CFVec3A &rTestV );
	FINLINE CFVec3A &ClampNeg1Pos1( const CFVec3A &rTestV );
	FINLINE CFVec3A &ClampMin( const CFVec3A &rTestV, const CFVec3A &rMinV );
	FINLINE CFVec3A &ClampMax( const CFVec3A &rTestV, const CFVec3A &rMaxV );
	FINLINE CFVec3A &Clamp( const CFVec3A &rTestV, const CFVec3A &rMinV, const CFVec3A &rMaxV );

	// Unit & Magnitude:
	FINLINE f32 Dist( const CFVec3A &rV ) const;
	FINLINE f32 DistSq( const CFVec3A &rV ) const;
	FINLINE f32 InvDist( const CFVec3A &rV ) const;
	FINLINE f32 InvDistSq( const CFVec3A &rV ) const;

	FINLINE f32 Mag( void ) const;
	FINLINE f32 MagSq( void ) const;
	FINLINE f32 InvMag( void ) const;
	FINLINE f32 InvMagSq( void ) const;

	FINLINE CFVec3A &ReceiveUnit( const CFVec3A &rV );
	FINLINE CFVec3A &Unitize( void );

	FINLINE f32 UnitAndMag( const CFVec3A &rV );
	FINLINE f32 UnitAndInvMag( const CFVec3A &rV );
	FINLINE f32 SafeUnitAndMag( const CFVec3A &rV );
	FINLINE f32 SafeUnitAndInvMag( const CFVec3A &rV );

	FINLINE f32 DistXZ( const CFVec3A &rV ) const;
	FINLINE f32 DistSqXZ( const CFVec3A &rV ) const;

	FINLINE f32 MagXZ( void ) const;
	FINLINE f32 MagSqXZ( void ) const;
	FINLINE f32 InvMagXZ( void ) const;
	FINLINE f32 InvMagSqXZ( void ) const;

	FINLINE CFVec3A &ReceiveUnitXZ( const CFVec3A &rV );
	FINLINE CFVec3A &UnitizeXZ( void );

	FINLINE f32 UnitAndMagXZ( const CFVec3A &rV );
	FINLINE f32 UnitAndInvMagXZ( const CFVec3A &rV );
	FINLINE f32 SafeUnitAndMagXZ( const CFVec3A &rV );
	FINLINE f32 SafeUnitAndInvMagXZ( const CFVec3A &rV );

	// Cross Product:
	FINLINE CFVec3A &Cross( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &Cross( const CFVec3A &rV );
	FINLINE CFVec3A &UnitCross( const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &UnitCross( const CFVec3A &rV );

	FINLINE CFVec3A &Cross( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec3A &Cross( const CFVec4A &rV );
	FINLINE CFVec3A &UnitCross( const CFVec4A &rV1, const CFVec4A &rV2 );
	FINLINE CFVec3A &UnitCross( const CFVec4A &rV );

	FINLINE CFVec3A &CrossVecWithY( const CFVec3A &rV );
	FINLINE CFVec3A &CrossYWithVec( const CFVec3A &rV );
	FINLINE CFVec3A &UnitCrossVecWithY( const CFVec3A &rV );
	FINLINE CFVec3A &UnitCrossYWithVec( const CFVec3A &rV );

	// Dot Product:
	FINLINE f32 Dot( const CFVec3A &rV ) const;

	// Lerp:
	FINLINE CFVec3A &Lerp( const f32 &fUnitVal, const CFVec3A &rV1, const CFVec3A &rV2 );
	FINLINE CFVec3A &Lerp( const f32 &fUnitVal, const CFVec3A &rV );

	// Rotation:
	FINLINE CFVec3A &RotateX( const f32 &fRadians );
	FINLINE CFVec3A &RotateY( const f32 &fRadians );
	FINLINE CFVec3A &RotateZ( const f32 &fRadians );

	FINLINE CFVec3A &RotateX( const f32 &fSin, const f32 &fCos );
	FINLINE CFVec3A &RotateY( const f32 &fSin, const f32 &fCos );
	FINLINE CFVec3A &RotateZ( const f32 &fSin, const f32 &fCos );

	FINLINE CFVec3A &ReceiveRotationX( const CFVec3A &rV, const f32 &fRadians );
	FINLINE CFVec3A &ReceiveRotationY( const CFVec3A &rV, const f32 &fRadians );
	FINLINE CFVec3A &ReceiveRotationZ( const CFVec3A &rV, const f32 &fRadians );

	FINLINE CFVec3A &ReceiveRotationX( const CFVec3A &rV, const f32 &fSin, const f32 &fCos );
	FINLINE CFVec3A &ReceiveRotationY( const CFVec3A &rV, const f32 &fSin, const f32 &fCos );
	FINLINE CFVec3A &ReceiveRotationZ( const CFVec3A &rV, const f32 &fSin, const f32 &fCos );
	
	// Reflection:
	FINLINE CFVec3A &Reflect( const CFVec3A &rNormal );
	FINLINE CFVec3A &ReceiveReflection( const CFVec3A &rV, const CFVec3A &rNormal );

	// planar projection
	FINLINE CFVec3A &PlanarProjection( const CFVec3A &rvNormal );
	FINLINE CFVec3A &ReceivePlanarProjection( const CFVec3A &rV, const CFVec3A &rvNormal );

	// yaw & pitch range check
	FINLINE BOOL InYawPitchRange( const CFVec3A &vCenter, const CFVec3A &vUp, const f32 fYawCos, const f32 fPitchCos );


	// Misc:
	FINLINE u32 GenKey( void ) const;
	void ChangeEndian( void ) {
		x = fang_ConvertEndian( x );
		y = fang_ConvertEndian( y );
		z = fang_ConvertEndian( z );
		w = fang_ConvertEndian( w );
	}

	// Debug:
	FMathFcheckResult_e FCheck( void ) const;

	FCLASS_STACKMEM_ALIGN( CFVec3A );
} FCLASS_ALIGN_SUFFIX;



#endif

