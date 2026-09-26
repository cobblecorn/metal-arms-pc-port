//////////////////////////////////////////////////////////////////////////////////////
// fquat.inl - Fang quaternion library.
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


#if _ENABLE_OLD_QUATS
//--------------------------------------------------------------------
// CFQuat Implementation:
//--------------------------------------------------------------------
FINLINE CFQuat::CFQuat() {}
FINLINE CFQuat::CFQuat( const CFQuat &rQuat ) { x=rQuat.x; y=rQuat.y; z=rQuat.z; w=rQuat.w; }
FINLINE CFQuat::CFQuat( const f32 fX, const f32 fY, const f32 fZ, const f32 fW ) { x=fX; y=fY; z=fZ; w=fW; }
FINLINE CFQuat::CFQuat( const CFMtx33 &rSrcMtx ) { BuildQuat(rSrcMtx); }
FINLINE CFQuat::CFQuat( const CFMtx43 &rSrcMtx ) { BuildQuat(rSrcMtx); }

FINLINE CFQuat &CFQuat::BuildQuat( const CFMtx43 &rSrcMtx ) { return BuildQuat(rSrcMtx.m33); }
FINLINE void CFQuat::Zero( void ) { Set( 0.0f, 0.0f, 0.0f, 1.0f ); }
FINLINE CFQuat &CFQuat::Unitize( void ) { CFVec4::Unitize(); return *this; }
FINLINE CFQuat CFQuat::Unit( void ) const { CFQuat q=*this; return q.Unitize(); }
FINLINE CFQuat CFQuat::Inverse( void ) const { CFQuat q=*this; return q.Invert(); }
FINLINE CFQuat &CFQuat::operator = ( const CFQuat &rQuat ) { x=rQuat.x; y=rQuat.y; z=rQuat.z; w=rQuat.w; return *this; }
FINLINE BOOL CFQuat::operator == ( const CFQuat &rQuat ) const { return (x==rQuat.x && y==rQuat.y && z==rQuat.z && w==rQuat.w); }
FINLINE CFQuat CFQuat::operator + ( const CFQuat &rQuat ) const { return CFQuat(x+rQuat.x, y+rQuat.y, z+rQuat.z, w+rQuat.w); }
FINLINE CFQuat CFQuat::operator - ( const CFQuat &rQuat ) const { return CFQuat(x-rQuat.x, y-rQuat.y, z-rQuat.z, w-rQuat.w); }
FINLINE CFQuat &CFQuat::operator += ( const CFQuat &rQuat ) { x+=rQuat.x; y+=rQuat.y; z+=rQuat.z; w+=rQuat.w; return *this; }
FINLINE CFQuat &CFQuat::operator -= ( const CFQuat &rQuat ) { x-=rQuat.x; y-=rQuat.y; z-=rQuat.z; w-=rQuat.w; return *this; }
FINLINE CFQuat &CFQuat::operator *= ( const CFQuat &rQuat ) { *this = *this * rQuat; return *this; }
FINLINE CFQuat CFQuat::operator - ( void ) { return CFQuat( -x, -y, -z, -w); }

FINLINE CFQuat &CFQuat::BuildQuat( const CFVec3 &rUnitVecToRotateAbout, f32 fRadiansToRotateBy ) {
	if( FMATH_IS_CLOSE_TO_ZERO(fRadiansToRotateBy) ) {
		Set( 0.0f, 0.0f, 0.0f, 1.0f );
	} else {
		f32 fSin;

		fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

		x = fSin * rUnitVecToRotateAbout.x;
		y = fSin * rUnitVecToRotateAbout.y;
		z = fSin * rUnitVecToRotateAbout.z;
	}

	return *this;
}

FINLINE CFQuat CFQuat::operator * ( const CFQuat &rQuat ) const {
	CFQuat DestQuat;

	DestQuat.a[3] = a[3]*rQuat.a[3] - v3.Dot( rQuat.v3 );
	DestQuat.v3 = v3.Cross( rQuat.v3 ) + rQuat.v3*a[3] + v3*rQuat.a[3];

	return DestQuat;
}

FINLINE CFQuat &CFQuat::Invert( void ) {
	f32 fOOQTimesConjQ;

	fOOQTimesConjQ = fmath_Inv( a[3]*a[3] + a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );

	v3 *= -fOOQTimesConjQ;
	a[3] *= fOOQTimesConjQ;

	return *this;
}
#endif




#if !FANG_WINGC

#include "xmmintrin.h"


static const CFVec4A _nnnnNegMaskW( (u32)0x00000000, (u32)0x00000000, (u32)0x00000000, (u32)0x80000000 );
static const CFVec4A _nnnnZeroMaskW( (u32)0xffffffff, (u32)0xffffffff, (u32)0xffffffff, (u32)0x00000000 );



//--------------------------------------------------------------------
// CFQuatA Implementation:
//--------------------------------------------------------------------
FINLINE CFQuatA::CFQuatA( void ) {}
FINLINE CFQuatA::CFQuatA( const CFQuatA &rQ ) { Set( rQ ); }
FINLINE CFQuatA::CFQuatA( const CFVec4A &rV ) { Set( rV ); }
FINLINE CFQuatA::CFQuatA( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { Set( fX, fY, fZ, fW ); }

FINLINE BOOL CFQuatA::operator == ( const CFQuatA &rQ ) const { return (v == rQ.v); }
FINLINE BOOL CFQuatA::operator != ( const CFQuatA &rQ ) const { return (v != rQ.v); }

FINLINE CFQuatA &CFQuatA::Zero( void ) { v.Zero(); return *this; }
FINLINE CFQuatA &CFQuatA::Identity( void ) { v.ZeroW1(); return *this; }

FINLINE CFQuatA &CFQuatA::operator = ( const CFQuatA &rQ ) { v = rQ.v; return *this; }
FINLINE CFQuatA &CFQuatA::Set( const CFQuatA &rQ ) { v = rQ.v; return *this; }
FINLINE CFQuatA &CFQuatA::Set( const CFVec3A &rV ) { v._m = rV._m; return *this; }	// Important: This must set the quat's W to 0
FINLINE CFQuatA &CFQuatA::Set( const CFVec4A &rV ) { v = rV; return *this; }
FINLINE CFQuatA &CFQuatA::Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { v.Set( fX, fY, fZ, fW );  return *this; }

FINLINE CFQuatA &CFQuatA::BuildQuat( const FPackedQuat_t &rPackedQuat ) {
	x = (f32)rPackedQuat.x * (1.0f/127.0f);
	y = (f32)rPackedQuat.y * (1.0f/127.0f);
	z = (f32)rPackedQuat.z * (1.0f/127.0f);
	w = (f32)rPackedQuat.w * (1.0f/127.0f);

	return *this;
}

FINLINE CFQuatA &CFQuatA::BuildQuat( const CFMtx43 &rMtx ) { return BuildQuat( rMtx.m33 ); }

FINLINE CFQuatA &CFQuatA::BuildQuat( const CFVec4A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy ) {
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = fSin * rUnitVecToRotateAbout.x;
	y = fSin * rUnitVecToRotateAbout.y;
	z = fSin * rUnitVecToRotateAbout.z;

	return *this;
}

FINLINE CFQuatA &CFQuatA::BuildQuat( const CFVec3A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy ) {
	return BuildQuat( rUnitVecToRotateAbout.v4a, fRadiansToRotateBy );
}

FINLINE CFQuatA &CFQuatA::BuildQuatRotX( const f32 &fRadiansToRotateBy ) {
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = fSin;
	y = 0.0f;
	z = 0.0f;

	return *this;
}

FINLINE CFQuatA &CFQuatA::BuildQuatRotY( const f32 &fRadiansToRotateBy ) {
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = 0.0f;
	y = fSin;
	z = 0.0f;

	return *this;
}

FINLINE CFQuatA &CFQuatA::BuildQuatRotZ( const f32 &fRadiansToRotateBy ) {
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = 0.0f;
	y = 0.0f;
	z = fSin;

	return *this;
}

FINLINE CFQuatA &CFQuatA::BuildQuat( const CFVec3A &rUnitVecToRotateFrom, const CFVec3A &rUnitVecToRotateTo ) {
	CFVec3A RotationUnitAxis;
	f32 fRotationAxisMag, fCosAngleBetweenVecs, fHalfSin;

	fCosAngleBetweenVecs = rUnitVecToRotateFrom.Dot( rUnitVecToRotateTo );

	w = fmath_Sqrt( (1.00030f + fCosAngleBetweenVecs) * 0.5f );

	fCosAngleBetweenVecs = 1.0f - fCosAngleBetweenVecs;
	if( fCosAngleBetweenVecs > 0.00001f ) {
		fHalfSin = fmath_Sqrt( fCosAngleBetweenVecs * 0.5f );

		RotationUnitAxis.Cross( rUnitVecToRotateFrom, rUnitVecToRotateTo );
		fRotationAxisMag = RotationUnitAxis.Mag();

		if( fRotationAxisMag > 0.00001f ) {
			RotationUnitAxis.Div( fRotationAxisMag );
			v.v3 = RotationUnitAxis.v3 * fHalfSin;
		} else {
			v.v3.Zero();
		}
	} else {
		v.v3.Zero();
	}

	FMATH_CLASS_DEBUG_FCHECK( *this );

	return *this;
}

FINLINE CFQuatA &CFQuatA::ReceiveNegative( const CFQuatA &rQ ) { v.Mul( rQ.v, FDX8Math_NegOnesW1 ); return *this; }
FINLINE CFQuatA &CFQuatA::Negate( void ) { v.Mul( FDX8Math_NegOnesW1 ); return *this; }

FINLINE CFQuatA &CFQuatA::Mul( const CFQuatA &rQ1, const CFQuatA &rQ2 ) {
	__asm {
		mov		eax, rQ1
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		mov		eax, rQ2
		shufps	xmm3, xmm3, 0xff					; xmm3 =  w1, w1, w1, w1
		movaps	xmm2, [eax]							; xmm2 =  w2, z2, y2, x2
		shufps	xmm4, xmm4, 0x24					; xmm4 =  x1, z1, y1, x1
		mulps	xmm3, xmm2							; xmm3 =  w1*v2
		xorps	xmm4, FDX8Math_nnnnNegMaskW			; xmm4 = -x1, z1, y1, x1
		movaps	xmm5, xmm2							; xmm5 =  w2, z2, y2, x2
		movaps	xmm6, xmm1							; xmm6 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x3f					; xmm5 =  x2, w2, w2, w2
		movaps	xmm0, xmm2							; xmm0 =  w2, z2, y2, x2
		mulps	xmm4, xmm5							; xmm4 = (-x1*x2, z1*w2, y1*w2, x1*w2)
		shufps	xmm6, xmm6, 0x49					; xmm6 =  y1, x1, z1, y1
		addps	xmm3, xmm4							; xmm3 =  w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2)
		xorps	xmm6, FDX8Math_nnnnNegMaskW			; xmm6 = -y1, x1, z1, y1
		shufps	xmm0, xmm0, 0x52					; xmm0 =  y2, y2, x2, z2
		shufps	xmm2, xmm2, 0x89					; xmm2 =  z2, x2, z2, y2
		movaps	xmm7, xmm1							; xmm7 =  w1, z1, y1, x1
		mulps	xmm6, xmm0							; xmm6 = (-y1*y2, x1*y2, z1*x2, y1*z2)
		shufps	xmm7, xmm7, 0x92					; xmm7 =  z1, y1, x1, z1
		addps	xmm3, xmm6							; xmm3 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2)
		mulps	xmm7, xmm2							; xmm7 = (z1*z2, y1*x2, x1*z2, z1*y2)
		subps	xmm3, xmm7							; xmm3 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2) - (z1*z2, y1*x2, x1*z2, z1*y2)
		mov		eax, this
		movaps	[eax], xmm3							; xmm3.x = w1*x2 + x1*w2 + y1*z2 - z1*y2
													; xmm3.y = w1*y2 + y1*w2 + z1*x2 - x1*z2,
													; xmm3.z = w1*z2 + z1*w2 + x1*y2 - y1*x2,
													; xmm3.w = w1*w2 - x1*x2 - y1*y2 - z1*z2,
	}

	return *this;
}

FINLINE CFQuatA &CFQuatA::Mul( const CFQuatA &rQ ) {
	__asm {
		mov		eax, this
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		mov		edx, rQ
		shufps	xmm3, xmm3, 0xff					; xmm3 =  w1, w1, w1, w1
		movaps	xmm2, [edx]							; xmm2 =  w2, z2, y2, x2
		shufps	xmm4, xmm4, 0x24					; xmm4 =  x1, z1, y1, x1
		mulps	xmm3, xmm2							; xmm3 =  w1*v2
		xorps	xmm4, FDX8Math_nnnnNegMaskW			; xmm4 = -x1, z1, y1, x1
		movaps	xmm5, xmm2							; xmm5 =  w2, z2, y2, x2
		movaps	xmm6, xmm1							; xmm6 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x3f					; xmm5 =  x2, w2, w2, w2
		movaps	xmm0, xmm2							; xmm0 =  w2, z2, y2, x2
		mulps	xmm4, xmm5							; xmm4 = (-x1*x2, z1*w2, y1*w2, x1*w2)
		shufps	xmm6, xmm6, 0x49					; xmm6 =  y1, x1, z1, y1
		addps	xmm3, xmm4							; xmm3 =  w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2)
		xorps	xmm6, FDX8Math_nnnnNegMaskW			; xmm6 = -y1, x1, z1, y1
		shufps	xmm0, xmm0, 0x52					; xmm0 =  y2, y2, x2, z2
		shufps	xmm2, xmm2, 0x89					; xmm2 =  z2, x2, z2, y2
		movaps	xmm7, xmm1							; xmm7 =  w1, z1, y1, x1
		mulps	xmm6, xmm0							; xmm6 = (-y1*y2, x1*y2, z1*x2, y1*z2)
		shufps	xmm7, xmm7, 0x92					; xmm7 =  z1, y1, x1, z1
		addps	xmm3, xmm6							; xmm3 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2)
		mulps	xmm7, xmm2							; xmm7 = (z1*z2, y1*x2, x1*z2, z1*y2)
		subps	xmm3, xmm7							; xmm3 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2) - (z1*z2, y1*x2, x1*z2, z1*y2)
		movaps	[eax], xmm3							; xmm3.x = w1*x2 + x1*w2 + y1*z2 - z1*y2
													; xmm3.y = w1*y2 + y1*w2 + z1*x2 - x1*z2,
													; xmm3.z = w1*z2 + z1*w2 + x1*y2 - y1*x2,
													; xmm3.w = w1*w2 - x1*x2 - y1*y2 - z1*z2,
	}

	return *this;
}

FINLINE CFVec3A &CFQuatA::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const { MulPoint( rRV.v4a, rV.v4a ); return rRV; }

FINLINE CFVec4A &CFQuatA::MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const {
	__asm {
		mov		eax, this
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1
		mov		eax, rV
		movaps	xmm2, [eax]							; xmm2 =  w2, z2, y2, x2

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm5, xmm1							; xmm5 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x92					; xmm5 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm5, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm5							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x3f					; xmm4 =  x1, w1, w1, w1
		mulps	xmm4, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm4
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		shufps	xmm1, xmm1, 0x89					; xmm1 =  z1, x1, z1, y1
		mulps	xmm3, xmm1
		addps	xmm0, xmm3

		andps	xmm0, FDX8Math_nnnnMask_XYZ1_W0		; Zero W component of xmm0
		mov		eax, rRV
		movaps	[eax], xmm0
	}

	return rRV;
}

FINLINE CFVec3A &CFQuatA::MulPoint( CFVec3A &rV ) const { MulPoint( rV.v4a ); return rV; }

FINLINE CFVec4A &CFQuatA::MulPoint( CFVec4A &rV ) const {
	__asm {
		mov		eax, this
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1
		mov		eax, rV
		movaps	xmm2, [eax]							; xmm2 =  w2, z2, y2, x2

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm5, xmm1							; xmm5 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x92					; xmm5 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm5, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm5							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x3f					; xmm4 =  x1, w1, w1, w1
		mulps	xmm4, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm4
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		shufps	xmm1, xmm1, 0x89					; xmm1 =  z1, x1, z1, y1
		mulps	xmm3, xmm1
		addps	xmm0, xmm3

		andps	xmm0, FDX8Math_nnnnMask_XYZ1_W0		; Zero W component of xmm0
		movaps	[eax], xmm0
	}

	return rV;
}

FINLINE CFVec3A &CFQuatA::InvMulPoint( CFVec3A &rRV, const CFVec3A &rV ) const { InvMulPoint( rRV.v4a, rV.v4a ); return rRV; }

FINLINE CFVec4A &CFQuatA::InvMulPoint( CFVec4A &rRV, const CFVec4A &rV ) const {
	__asm {
		mov		eax, this
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1
		mov		eax, rV
		movaps	xmm2, [eax]							; xmm2 =  w2, z2, y2, x2

		mulps	xmm1, FDX8Math_NegOnesW1			; xmm1 =  w1, -z1, -y1, -x1

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm5, xmm1							; xmm5 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x92					; xmm5 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm5, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm5							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x3f					; xmm4 =  x1, w1, w1, w1
		mulps	xmm4, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm4
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		shufps	xmm1, xmm1, 0x89					; xmm1 =  z1, x1, z1, y1
		mulps	xmm3, xmm1
		addps	xmm0, xmm3

		andps	xmm0, FDX8Math_nnnnMask_XYZ1_W0		; Zero W component of xmm0
		mov		eax, rRV
		movaps	[eax], xmm0
	}

	return rRV;
}

FINLINE CFVec3A &CFQuatA::InvMulPoint( CFVec3A &rV ) const { InvMulPoint( rV.v4a ); return rV; }

FINLINE CFVec4A &CFQuatA::InvMulPoint( CFVec4A &rV ) const {
	__asm {
		mov		eax, this
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1
		mov		eax, rV
		movaps	xmm2, [eax]							; xmm2 =  w2, z2, y2, x2

		mulps	xmm1, FDX8Math_NegOnesW1			; xmm1 =  w1, -z1, -y1, -x1

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm5, xmm1							; xmm5 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x92					; xmm5 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm5, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm5							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x3f					; xmm4 =  x1, w1, w1, w1
		mulps	xmm4, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm4
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		shufps	xmm1, xmm1, 0x89					; xmm1 =  z1, x1, z1, y1
		mulps	xmm3, xmm1
		addps	xmm0, xmm3

		andps	xmm0, FDX8Math_nnnnMask_XYZ1_W0		; Zero W component of xmm0
		movaps	[eax], xmm0
	}

	return rV;
}

FINLINE CFQuatA &CFQuatA::ReceiveInverse( const CFQuatA &rQ ) { v.Mul( rQ.v, FDX8Math_NegOnesW1 ); return *this; }
FINLINE CFQuatA &CFQuatA::Invert( void ) { v.Mul( FDX8Math_NegOnesW1 ); return *this; }

FINLINE CFQuatA &CFQuatA::ReceiveNonUnitInverse( const CFQuatA &rQ ) {
	f32 fMag2 = rQ.x*rQ.x + rQ.y*rQ.y + rQ.z*rQ.z + rQ.w*rQ.w;

	__asm {
		rcpss	xmm0, fMag2
		shufps	xmm0, xmm0, 00h
		mulps	xmm0, FDX8Math_NegOnesW1
		mov		eax, rQ
		mulps	xmm0, [eax]
		mov		eax, this
		movaps	[eax], xmm0
	}

	FMATH_CLASS_DEBUG_FCHECK( *this );

	return *this;
}


FINLINE CFQuatA &CFQuatA::NonUnitInvert( void ) {
	f32 fMag2 = x*x + y*y + z*z + w*w;

	__asm {
		rcpss	xmm0, fMag2
		shufps	xmm0, xmm0, 00h
		mulps	xmm0, FDX8Math_NegOnesW1
		mov		eax, this
		mulps	xmm0, [eax]
		movaps	[eax], xmm0
	}

	FMATH_CLASS_DEBUG_FCHECK( *this );

	return *this;
}

FINLINE CFQuatA &CFQuatA::ReceiveUnit( const CFQuatA &rQ ) { v.ReceiveUnit( rQ.v ); return *this; }
FINLINE CFQuatA &CFQuatA::Unitize( void ) { v.Unitize(); return *this; }

FINLINE CFMtx33 &CFQuatA::BuildMtx( CFMtx33 &rDestMtx ) const {
	f32 f2X = 2.0f*x;
	f32 f2Y = 2.0f*y;
	f32 f2Z = 2.0f*z;
	f32 f2WX = f2X*w;
	f32 f2WY = f2Y*w;
	f32 f2WZ = f2Z*w;
	f32 f2XX = f2X*x;
	f32 f2XY = f2Y*x;
	f32 f2XZ = f2Z*x;
	f32 f2YY = f2Y*y;
	f32 f2YZ = f2Z*y;
	f32 f2ZZ = f2Z*z;

	rDestMtx.aa[0][0] = 1.0f - (f2YY + f2ZZ);
	rDestMtx.aa[0][1] = f2XY + f2WZ;
	rDestMtx.aa[0][2] = f2XZ - f2WY;
	rDestMtx.aa[1][0] = f2XY - f2WZ;
	rDestMtx.aa[1][1] = 1.0f - (f2XX + f2ZZ);
	rDestMtx.aa[1][2] = f2YZ + f2WX;
	rDestMtx.aa[2][0] = f2XZ + f2WY;
	rDestMtx.aa[2][1] = f2YZ - f2WX;
	rDestMtx.aa[2][2] = 1.0f - (f2XX + f2YY);

	return rDestMtx;
}


FINLINE CFMtx33 &CFQuatA::BuildMtx( CFMtx33 &rDestMtx, const f32 &fScale ) const {
	f32 f2Scale = 2.0f * fScale;
	f32 f2ScaleX = f2Scale*x;
	f32 f2ScaleY = f2Scale*y;
	f32 f2ScaleZ = f2Scale*z;
	f32 f2ScaleWX = f2ScaleX*w;
	f32 f2ScaleWY = f2ScaleY*w;
	f32 f2ScaleWZ = f2ScaleZ*w;
	f32 f2ScaleXX = f2ScaleX*x;
	f32 f2ScaleXY = f2ScaleY*x;
	f32 f2ScaleXZ = f2ScaleZ*x;
	f32 f2ScaleYY = f2ScaleY*y;
	f32 f2ScaleYZ = f2ScaleZ*y;
	f32 f2ScaleZZ = f2ScaleZ*z;

	rDestMtx.aa[0][0] = fScale - (f2ScaleYY + f2ScaleZZ);
	rDestMtx.aa[0][1] = f2ScaleXY + f2ScaleWZ;
	rDestMtx.aa[0][2] = f2ScaleXZ - f2ScaleWY;
	rDestMtx.aa[1][0] = f2ScaleXY - f2ScaleWZ;
	rDestMtx.aa[1][1] = fScale - (f2ScaleXX + f2ScaleZZ);
	rDestMtx.aa[1][2] = f2ScaleYZ + f2ScaleWX;
	rDestMtx.aa[2][0] = f2ScaleXZ + f2ScaleWY;
	rDestMtx.aa[2][1] = f2ScaleYZ - f2ScaleWX;
	rDestMtx.aa[2][2] = fScale - (f2ScaleXX + f2ScaleYY);

	return rDestMtx;
}


FINLINE CFMtx43A &CFQuatA::BuildMtx( CFMtx43A &rDestMtx ) const {
	f32 f2X = 2.0f*x;
	f32 f2Y = 2.0f*y;
	f32 f2Z = 2.0f*z;
	f32 f2WX = f2X*w;
	f32 f2WY = f2Y*w;
	f32 f2WZ = f2Z*w;
	f32 f2XX = f2X*x;
	f32 f2XY = f2Y*x;
	f32 f2XZ = f2Z*x;
	f32 f2YY = f2Y*y;
	f32 f2YZ = f2Z*y;
	f32 f2ZZ = f2Z*z;

	rDestMtx.aa[0][0] = 1.0f - (f2YY + f2ZZ);
	rDestMtx.aa[0][1] = f2XY + f2WZ;
	rDestMtx.aa[0][2] = f2XZ - f2WY;
	rDestMtx.aa[1][0] = f2XY - f2WZ;
	rDestMtx.aa[1][1] = 1.0f - (f2XX + f2ZZ);
	rDestMtx.aa[1][2] = f2YZ + f2WX;
	rDestMtx.aa[2][0] = f2XZ + f2WY;
	rDestMtx.aa[2][1] = f2YZ - f2WX;
	rDestMtx.aa[2][2] = 1.0f - (f2XX + f2YY);
	rDestMtx.m_vPos = CFVec3A::m_Null;

	return rDestMtx;
}


FINLINE CFMtx43A &CFQuatA::BuildMtx( CFMtx43A &rDestMtx, const f32 &fScale ) const {
	f32 f2Scale = 2.0f * fScale;
	f32 f2ScaleX = f2Scale*x;
	f32 f2ScaleY = f2Scale*y;
	f32 f2ScaleZ = f2Scale*z;
	f32 f2ScaleWX = f2ScaleX*w;
	f32 f2ScaleWY = f2ScaleY*w;
	f32 f2ScaleWZ = f2ScaleZ*w;
	f32 f2ScaleXX = f2ScaleX*x;
	f32 f2ScaleXY = f2ScaleY*x;
	f32 f2ScaleXZ = f2ScaleZ*x;
	f32 f2ScaleYY = f2ScaleY*y;
	f32 f2ScaleYZ = f2ScaleZ*y;
	f32 f2ScaleZZ = f2ScaleZ*z;

	rDestMtx.aa[0][0] = fScale - (f2ScaleYY + f2ScaleZZ);
	rDestMtx.aa[0][1] = f2ScaleXY + f2ScaleWZ;
	rDestMtx.aa[0][2] = f2ScaleXZ - f2ScaleWY;
	rDestMtx.aa[1][0] = f2ScaleXY - f2ScaleWZ;
	rDestMtx.aa[1][1] = fScale - (f2ScaleXX + f2ScaleZZ);
	rDestMtx.aa[1][2] = f2ScaleYZ + f2ScaleWX;
	rDestMtx.aa[2][0] = f2ScaleXZ + f2ScaleWY;
	rDestMtx.aa[2][1] = f2ScaleYZ - f2ScaleWX;
	rDestMtx.aa[2][2] = fScale - (f2ScaleXX + f2ScaleYY);
	rDestMtx.m_vPos = CFVec3A::m_Null;

	return rDestMtx;
}


FINLINE CFMtx43A &CFQuatA::BuildMtx33( CFMtx43A &rDestMtx33 ) const {
	f32 f2X = 2.0f*x;
	f32 f2Y = 2.0f*y;
	f32 f2Z = 2.0f*z;
	f32 f2WX = f2X*w;
	f32 f2WY = f2Y*w;
	f32 f2WZ = f2Z*w;
	f32 f2XX = f2X*x;
	f32 f2XY = f2Y*x;
	f32 f2XZ = f2Z*x;
	f32 f2YY = f2Y*y;
	f32 f2YZ = f2Z*y;
	f32 f2ZZ = f2Z*z;

	rDestMtx33.aa[0][0] = 1.0f - (f2YY + f2ZZ);
	rDestMtx33.aa[0][1] = f2XY + f2WZ;
	rDestMtx33.aa[0][2] = f2XZ - f2WY;
	rDestMtx33.aa[1][0] = f2XY - f2WZ;
	rDestMtx33.aa[1][1] = 1.0f - (f2XX + f2ZZ);
	rDestMtx33.aa[1][2] = f2YZ + f2WX;
	rDestMtx33.aa[2][0] = f2XZ + f2WY;
	rDestMtx33.aa[2][1] = f2YZ - f2WX;
	rDestMtx33.aa[2][2] = 1.0f - (f2XX + f2YY);

	return rDestMtx33;
}


FINLINE CFMtx43A &CFQuatA::BuildMtx33( CFMtx43A &rDestMtx33, const f32 &fScale ) const {
	f32 f2Scale = 2.0f * fScale;
	f32 f2ScaleX = f2Scale*x;
	f32 f2ScaleY = f2Scale*y;
	f32 f2ScaleZ = f2Scale*z;
	f32 f2ScaleWX = f2ScaleX*w;
	f32 f2ScaleWY = f2ScaleY*w;
	f32 f2ScaleWZ = f2ScaleZ*w;
	f32 f2ScaleXX = f2ScaleX*x;
	f32 f2ScaleXY = f2ScaleY*x;
	f32 f2ScaleXZ = f2ScaleZ*x;
	f32 f2ScaleYY = f2ScaleY*y;
	f32 f2ScaleYZ = f2ScaleZ*y;
	f32 f2ScaleZZ = f2ScaleZ*z;

	rDestMtx33.aa[0][0] = fScale - (f2ScaleYY + f2ScaleZZ);
	rDestMtx33.aa[0][1] = f2ScaleXY + f2ScaleWZ;
	rDestMtx33.aa[0][2] = f2ScaleXZ - f2ScaleWY;
	rDestMtx33.aa[1][0] = f2ScaleXY - f2ScaleWZ;
	rDestMtx33.aa[1][1] = fScale - (f2ScaleXX + f2ScaleZZ);
	rDestMtx33.aa[1][2] = f2ScaleYZ + f2ScaleWX;
	rDestMtx33.aa[2][0] = f2ScaleXZ + f2ScaleWY;
	rDestMtx33.aa[2][1] = f2ScaleYZ - f2ScaleWX;
	rDestMtx33.aa[2][2] = fScale - (f2ScaleXX + f2ScaleYY);

	return rDestMtx33;
}


FINLINE CFVec3A &CFQuatA::BuildAxisX( CFVec3A &rDestVec ) const {
	rDestVec.a[0] = 1.0f - 2.0f * (a[1]*a[1] + a[2]*a[2]);
	rDestVec.a[1] = 2.0f * (a[0]*a[1] + a[3]*a[2]);
	rDestVec.a[2] = 2.0f * (a[0]*a[2] - a[3]*a[1]);

	return rDestVec;
}


FINLINE CFVec3A &CFQuatA::BuildAxisY( CFVec3A &rDestVec ) const {
	rDestVec.a[0] = 2.0f * (a[0]*a[1] - a[3]*a[2]);
	rDestVec.a[1] = 1.0f - 2.0f * (a[0]*a[0] + a[2]*a[2]);
	rDestVec.a[2] = 2.0f * (a[1]*a[2] + a[3]*a[0]);

	return rDestVec;
}


FINLINE CFVec3A &CFQuatA::BuildAxisZ( CFVec3A &rDestVec ) const {
	rDestVec.a[0] = 2.0f * (a[0]*a[2] + a[3]*a[1]);
	rDestVec.a[1] = 2.0f * (a[1]*a[2] - a[3]*a[0]);
	rDestVec.a[2] = 1.0f - 2.0f * (a[1]*a[1] + a[0]*a[0]);

	return rDestVec;
}


//--------------------------------------------------------------------
// CFTQuat Implementation:
//--------------------------------------------------------------------
FINLINE CFTQuatA::CFTQuatA( void ) {}
FINLINE CFTQuatA::CFTQuatA( const CFTQuatA &rTQ ) { Set( rTQ ); }
FINLINE CFTQuatA::CFTQuatA( const CFQuatA &rQ, const CFVec3A &rPos, const f32 &fScale ) { Set( rQ, rPos, fScale ); }
FINLINE CFTQuatA::CFTQuatA( const CFVec4A &rV, const CFVec3A &rPos, const f32 &fScale ) { Set( rV, rPos, fScale ); }

FINLINE CFTQuatA::CFTQuatA( const f32 &fQX, const f32 &fQY, const f32 &fQZ, const f32 &fQW, const f32 &fPX, const f32 &fPY, const f32 &fPZ, const f32 &fScale ) {
	Set( fQX, fQY, fQZ, fQW, fPX, fPY, fPZ, fScale );
}

#pragma warning( disable : 4035 )
FINLINE BOOL CFTQuatA::operator == ( const CFTQuatA &rQ ) const {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm1, [eax+0x10]

		mov			eax, rQ
		cmpneqps	xmm0, [eax]
		cmpneqps	xmm1, [eax+0x10]

		orps		xmm0, xmm1
		movmskps	eax, xmm0
		sub			eax, 1
		sbb			eax, eax
	}
}
#pragma warning( default : 4035 )

#pragma warning( disable : 4035 )
FINLINE BOOL CFTQuatA::operator != ( const CFTQuatA &rQ ) const {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm1, [eax+0x10]

		mov			eax, rQ
		cmpneqps	xmm0, [eax]
		cmpneqps	xmm1, [eax+0x10]

		orps		xmm0, xmm1
		movmskps	eax, xmm0
	}
}
#pragma warning( default : 4035 )

FINLINE CFTQuatA &CFTQuatA::Zero( void ) { CFQuatA::Zero(); m_PosScale.Zero(); return *this; }
FINLINE CFTQuatA &CFTQuatA::Identity( void ) { CFQuatA::Identity(); m_PosScale=CFVec4A::m_UnitAxisW; return *this; }

FINLINE CFTQuatA &CFTQuatA::operator = ( const CFTQuatA &rTQ ) { v=rTQ.v; m_PosScale=rTQ.m_PosScale; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFTQuatA &rTQ ) { v=rTQ.v; m_PosScale=rTQ.m_PosScale; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFQuatA &rQ ) { v=rQ.v; m_PosScale=CFVec4A::m_UnitAxisW; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFQuatA &rQ, const CFVec3A &rPos, const f32 &fScale ) { v=rQ.v; m_PosScale=rPos.v4a; m_fScale=fScale; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFVec4A &rV, const CFVec3A &rPos, const f32 &fScale ) { v=rV; m_PosScale=rPos.v4a; m_fScale=fScale; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const f32 &fQX, const f32 &fQY, const f32 &fQZ, const f32 &fQW, const f32 &fPX, const f32 &fPY, const f32 &fPZ, const f32 &fScale ) {
	v.Set( fQX, fQY, fQZ, fQW );
	m_Pos.Set( fPX, fPY, fPZ );
	m_fScale = fScale;

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFMtx33 &rSrcMtx, const CFVec3A &rPos, BOOL bMtxHasNonUnitScale ) {
	m_Pos = rPos.v3;

	if( !bMtxHasNonUnitScale ) {
		m_fScale = 1.0f;
		CFQuatA::BuildQuat( rSrcMtx );
	} else {
		CFMtx33 UnitMtx;
		m_fScale = rSrcMtx.m_vRight.Mag();
		UnitMtx = rSrcMtx * fmath_Inv(m_fScale);
		CFQuatA::BuildQuat( UnitMtx );
	}

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFMtx43 &rSrcMtx, BOOL bMtxHasNonUnitScale ) {
	m_Pos = rSrcMtx.m_vPos;

	if( !bMtxHasNonUnitScale ) {
		m_fScale = 1.0f;
		CFQuatA::BuildQuat( rSrcMtx );
	} else {
		CFMtx33 UnitMtx;
		m_fScale = rSrcMtx.m_vRight.Mag();
		UnitMtx = rSrcMtx.m33 * fmath_Inv(m_fScale);
		CFQuatA::BuildQuat( UnitMtx );
	}

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFMtx44 &rSrcMtx, BOOL bMtxHasNonUnitScale ) {
	m_Pos = rSrcMtx.m_vPos;

	if( !bMtxHasNonUnitScale ) {
		m_fScale = 1.0f;
		CFQuatA::BuildQuat( rSrcMtx );
	} else {
		CFMtx44 UnitMtx;
		m_fScale = rSrcMtx.m_vRight.Mag();
		UnitMtx = rSrcMtx * fmath_Inv(m_fScale);
		CFQuatA::BuildQuat( UnitMtx );
	}

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFMtx44A &rSrcMtx, BOOL bMtxHasNonUnitScale ) {
	m_Pos = rSrcMtx.m_vPos.v3;

	if( !bMtxHasNonUnitScale ) {
		m_fScale = 1.0f;
		CFQuatA::BuildQuat( rSrcMtx );
	} else {
		CFMtx44A UnitMtx;
		m_fScale = rSrcMtx.m_vRight.Mag();
		UnitMtx.Mul( rSrcMtx, fmath_Inv(m_fScale) );
		CFQuatA::BuildQuat( UnitMtx );
	}

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFMtx43A &rSrcMtx, BOOL bMtxHasNonUnitScale ) {
	m_Pos = rSrcMtx.m_vPos.v3;

	if( !bMtxHasNonUnitScale ) {
		m_fScale = 1.0f;
		CFQuatA::BuildQuat( rSrcMtx );
	} else {
		CFMtx43A UnitMtx;
		m_fScale = rSrcMtx.m_vRight.Mag();
		UnitMtx.Mul( rSrcMtx, fmath_Inv(m_fScale) );
		CFQuatA::BuildQuat( UnitMtx );
	}

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFVec3A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy, const CFVec3A &rPos, const f32 &fScale ) {
	return BuildQuat( rUnitVecToRotateAbout.v4a, fRadiansToRotateBy, rPos, fScale );
}

FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFVec4A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy, const CFVec3A &rPos, const f32 &fScale ) {
	CFQuatA::BuildQuat( rUnitVecToRotateAbout, fRadiansToRotateBy );
	m_Pos = rPos.v3;
	m_fScale = fScale;

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::Mul( const CFTQuatA &rQ1, const CFTQuatA &rQ2 ) {
	__asm {
		;--------------------------------------------------------------------
		; Compute p3 = s1*Q1*p2*Q1' + p1
		;     and s3 = s1*s2
		;--------------------------------------------------------------------

		mov		edx, rQ2							; ( Q2, p2, s2 )
		movaps	xmm2, [edx + 16]					; xmm2 =  w2, z2, y2, x2

		mov		eax, rQ1							; ( Q1, p1, s1 )
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		movaps	xmm6, xmm4							; xmm6 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm0, xmm1							; xmm0 =  w1, z1, y1, x1
		shufps	xmm0, xmm0, 0x92					; xmm0 =  z1, y1, x1, z1
		movaps	xmm7, xmm0							; xmm7 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm0, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm0							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0xff					; xmm2 =  w1, w1, w1, w1
		movaps	xmm4, xmm2							; xmm4 =  w1, w1, w1, w1
		mulps	xmm2, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm2
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x89					; xmm2 =  z1, x1, z1, y1
		mulps	xmm3, xmm2
		addps	xmm0, xmm3							; xmm0 = Q1*p2*Q1'

		movss	xmm2, [eax + 16 + 12]				; xmm2 = 0, 0, 0, s1
		shufps	xmm2, xmm2, 0x00					; xmm2 = s1, s1, s1, s1
		mulps	xmm0, xmm2							; xmm0 = s1*Q1*p2*Q1'
		addps	xmm0, [eax + 16]					; xmm0 = s1*Q1*p2*Q1' + p1
		mulss	xmm2, [edx + 16 + 12]				; xmm2 = s1, s1, s1, s1*s2

		mov		eax, this
		movaps	[eax + 16], xmm0					; Store new p1 = s1*Q1*p2*Q1' + s2*p1
		movss	[eax + 16 + 12], xmm2				; Store new s1 = s1*s2

		;--------------------------------------------------------------------
		; Compute Q3 = Q1*Q2:
		;--------------------------------------------------------------------

		shufps	xmm1, xmm1, 0x24					; xmm1 =  x1, z1, y1, x1
		xorps	xmm1, FDX8Math_nnnnNegMaskW			; xmm1 = -x1, z1, y1, x1
		xorps	xmm6, FDX8Math_nnnnNegMaskW			; xmm6 = -y1, x1, z1, y1

		mulps	xmm4, [edx]							; xmm4 =  w1*v2

		movaps	xmm0, [edx]							; xmm0 =  w2, z2, y2, x2
		shufps	xmm0, xmm0, 0x3f					; xmm0 =  x2, w2, w2, w2
		mulps	xmm1, xmm0							; xmm1 = (-x1*x2, z1*w2, y1*w2, x1*w2)
		addps	xmm4, xmm1							; xmm4 =  w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2)

		movaps	xmm0, [edx]							; xmm0 =  w2, z2, y2, x2
		shufps	xmm0, xmm0, 0x52					; xmm0 =  y2, y2, x2, z2
		mulps	xmm6, xmm0							; xmm6 = (-y1*y2, x1*y2, z1*x2, y1*z2)
		addps	xmm4, xmm6							; xmm4 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2)

		shufps	xmm0, xmm0, 0x12					; xmm0 =  z2, x2, z2, y2
		mulps	xmm7, xmm0							; xmm7 = (z1*z2, y1*x2, x1*z2, z1*y2)
		subps	xmm4, xmm7							; xmm4 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2) - (z1*z2, y1*x2, x1*z2, z1*y2)

		movaps	[eax], xmm4							; xmm4.x = w1*x2 + x1*w2 + y1*z2 - z1*y2
													; xmm4.y = w1*y2 + y1*w2 + z1*x2 - x1*z2,
													; xmm4.z = w1*z2 + z1*w2 + x1*y2 - y1*x2,
													; xmm4.w = w1*w2 - x1*x2 - y1*y2 - z1*z2,

	}

	return *this;
}


FINLINE CFTQuatA &CFTQuatA::Mul( const CFTQuatA &rQ ) {
	__asm {
		;--------------------------------------------------------------------
		; Compute p1 = s1*Q1*p2*Q1' + p1
		;     and s1 = s1*s2
		;--------------------------------------------------------------------

		mov		edx, rQ								; ( Q2, p2, s2 )
		movaps	xmm2, [edx + 16]					; xmm2 =  w2, z2, y2, x2

		mov		eax, this							; ( Q1, p1, s1 )
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		movaps	xmm6, xmm4							; xmm6 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm0, xmm1							; xmm0 =  w1, z1, y1, x1
		shufps	xmm0, xmm0, 0x92					; xmm0 =  z1, y1, x1, z1
		movaps	xmm7, xmm0							; xmm7 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm0, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm0							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0xff					; xmm2 =  w1, w1, w1, w1
		movaps	xmm4, xmm2							; xmm4 =  w1, w1, w1, w1
		mulps	xmm2, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm2
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x89					; xmm2 =  z1, x1, z1, y1
		mulps	xmm3, xmm2
		addps	xmm0, xmm3							; xmm0 = Q1*p2*Q1'

		movss	xmm2, [eax + 16 + 12]				; xmm2 = 0, 0, 0, s1
		shufps	xmm2, xmm2, 0x00					; xmm2 = s1, s1, s1, s1
		mulps	xmm0, xmm2							; xmm0 = s1*Q1*p2*Q1'
		addps	xmm0, [eax + 16]					; xmm0 = s1*Q1*p2*Q1' + p1
		mulss	xmm2, [edx + 16 + 12]				; xmm2 = s1, s1, s1, s1*s2

		movaps	[eax + 16], xmm0					; Store new p1 = s1*Q1*p2*Q1' + s2*p1
		movss	[eax + 16 + 12], xmm2				; Store new s1 = s1*s2

		;--------------------------------------------------------------------
		; Compute Q1 = Q1*Q2:
		;--------------------------------------------------------------------

		shufps	xmm1, xmm1, 0x24					; xmm1 =  x1, z1, y1, x1
		xorps	xmm1, FDX8Math_nnnnNegMaskW			; xmm1 = -x1, z1, y1, x1
		xorps	xmm6, FDX8Math_nnnnNegMaskW			; xmm6 = -y1, x1, z1, y1

		mulps	xmm4, [edx]							; xmm4 =  w1*v2

		movaps	xmm0, [edx]							; xmm0 =  w2, z2, y2, x2
		shufps	xmm0, xmm0, 0x3f					; xmm0 =  x2, w2, w2, w2
		mulps	xmm1, xmm0							; xmm1 = (-x1*x2, z1*w2, y1*w2, x1*w2)
		addps	xmm4, xmm1							; xmm4 =  w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2)

		movaps	xmm0, [edx]							; xmm0 =  w2, z2, y2, x2
		shufps	xmm0, xmm0, 0x52					; xmm0 =  y2, y2, x2, z2
		mulps	xmm6, xmm0							; xmm6 = (-y1*y2, x1*y2, z1*x2, y1*z2)
		addps	xmm4, xmm6							; xmm4 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2)

		shufps	xmm0, xmm0, 0x12					; xmm0 =  z2, x2, z2, y2
		mulps	xmm7, xmm0							; xmm7 = (z1*z2, y1*x2, x1*z2, z1*y2)
		subps	xmm4, xmm7							; xmm4 = w1*v2 + (-x1*x2, z1*w2, y1*w2, x1*w2) + (-y1*y2, x1*y2, z1*x2, y1*z2) - (z1*z2, y1*x2, x1*z2, z1*y2)

		movaps	[eax], xmm4							; xmm4.x = w1*x2 + x1*w2 + y1*z2 - z1*y2
													; xmm4.y = w1*y2 + y1*w2 + z1*x2 - x1*z2,
													; xmm4.z = w1*z2 + z1*w2 + x1*y2 - y1*x2,
													; xmm4.w = w1*w2 - x1*x2 - y1*y2 - z1*z2,
	}

	return *this;
}


FINLINE CFVec3A &CFTQuatA::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const {
	MulPoint( rRV.v4a, rV.v4a );
	return rRV;
}

FINLINE CFVec4A &CFTQuatA::MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const {
	__asm {
		;--------------------------------------------------------------------
		; Compute p3 = s1*Q1*p2*Q1' + p1
		;--------------------------------------------------------------------

		mov		eax, rV								; p2
		movaps	xmm2, [eax]							; xmm2 =  w2, z2, y2, x2
		mov		eax, this							; ( Q1, p1 )
		movaps	xmm1, [eax]							; xmm1 =  w1, z1, y1, x1

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm5, xmm1							; xmm5 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x92					; xmm5 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm5, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm5							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x3f					; xmm4 =  x1, w1, w1, w1
		mulps	xmm4, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm4
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		shufps	xmm1, xmm1, 0x89					; xmm1 =  z1, x1, z1, y1
		mulps	xmm3, xmm1
		addps	xmm0, xmm3							; xmm0 = Q1*p2*Q1'

		movss	xmm2, [eax + 16 + 12]				; xmm2 = 0, 0, 0, s1
		shufps	xmm2, xmm2, 0x00					; xmm2 = s1, s1, s1, s1
		mulps	xmm0, xmm2							; xmm0 = s1*Q1*p2*Q1'

		addps	xmm0, [eax + 16]					; xmm0 = s1*Q1*p2*Q1' + p1
		andps	xmm0, FDX8Math_nnnnMask_XYZ1_W0		; Zero W component of xmm0
		mov		eax, rRV
		movaps	[eax], xmm0							; Store p3
	}

	return rRV;
}

FINLINE CFVec3A &CFTQuatA::MulPoint( CFVec3A &rV ) const {
	MulPoint( rV.v4a );
	return rV;
}

FINLINE CFVec4A &CFTQuatA::MulPoint( CFVec4A &rV ) const {
	__asm {
		;--------------------------------------------------------------------
		; Compute p3 = s1*Q1*p2*Q1' + p1
		;--------------------------------------------------------------------

		mov		eax, rV								; p2
		movaps	xmm2, [eax]							; xmm2 =  w2, z2, y2, x2
		mov		edx, this							; ( Q1, p1 )
		movaps	xmm1, [edx]							; xmm1 =  w1, z1, y1, x1

		shufps	xmm2, xmm2, 0x24					; xmm2 =  x2, z2, y2, x2
		movaps	xmm3, xmm1							; xmm3 =  w1, z1, y1, x1
		shufps	xmm3, xmm3, 0x3f					; xmm3 =  x1, w1, w1, w1
		mulps	xmm3, xmm2

		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x49					; xmm4 =  y1, x1, z1, y1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y2, y2, x2, z2
		mulps	xmm4, xmm2

		movaps	xmm5, xmm1							; xmm5 =  w1, z1, y1, x1
		shufps	xmm5, xmm5, 0x92					; xmm5 =  z1, y1, x1, z1
		shufps	xmm2, xmm2, 0x12					; xmm2 =  z2, x2, z2, y2
		xorps	xmm2, FDX8Math_nnnnNegMaskW			; xmm2 = -z2, x2, z2, y2
		mulps	xmm5, xmm2

		addps	xmm3, xmm4
		subps	xmm3, xmm5							; xmm3 =   D,  C,  B,  A

		movaps	xmm0, xmm3							; xmm0 =   D,  C,  B,  A
		shufps	xmm0, xmm0, 0xff					; xmm0 =   D,  D,  D,  D
		mulps	xmm0, xmm1

		shufps	xmm3, xmm3, 0x24					; xmm3 =   A,  C,  B,  A
		movaps	xmm4, xmm1							; xmm4 =  w1, z1, y1, x1
		shufps	xmm4, xmm4, 0x3f					; xmm4 =  x1, w1, w1, w1
		mulps	xmm4, xmm3

		shufps	xmm3, xmm3, 0x49					; xmm3 =   B,  A,  C,  B
		addps	xmm0, xmm4
		movaps	xmm2, xmm1							; xmm2 =  w1, z1, y1, x1
		shufps	xmm2, xmm2, 0x52					; xmm2 =  y1, y1, x1, z1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2

		shufps	xmm3, xmm3, 0x49					; xmm3 =   C,  B,  A,  C
		shufps	xmm1, xmm1, 0x89					; xmm1 =  z1, x1, z1, y1
		mulps	xmm3, xmm1
		addps	xmm0, xmm3							; xmm0 = Q1*p2*Q1'

		movss	xmm2, [edx + 16 + 12]				; xmm2 = 0, 0, 0, s1
		shufps	xmm2, xmm2, 0x00					; xmm2 = s1, s1, s1, s1
		mulps	xmm0, xmm2							; xmm0 = s1*Q1*p2*Q1'

		addps	xmm0, [edx + 16]					; xmm0 = s1*Q1*p2*Q1' + p1
		andps	xmm0, FDX8Math_nnnnMask_XYZ1_W0		; Zero W component of xmm0
		movaps	[eax], xmm0							; Store p3
	}

	return rV;
}

FINLINE CFMtx43 &CFTQuatA::BuildMtx( CFMtx43 &rDestMtx ) const {
	f32 f2Scale = 2.0f * m_fScale;
	f32 f2ScaleX = f2Scale*x;
	f32 f2ScaleY = f2Scale*y;
	f32 f2ScaleZ = f2Scale*z;
	f32 f2ScaleWX = f2ScaleX*w;
	f32 f2ScaleWY = f2ScaleY*w;
	f32 f2ScaleWZ = f2ScaleZ*w;
	f32 f2ScaleXX = f2ScaleX*x;
	f32 f2ScaleXY = f2ScaleY*x;
	f32 f2ScaleXZ = f2ScaleZ*x;
	f32 f2ScaleYY = f2ScaleY*y;
	f32 f2ScaleYZ = f2ScaleZ*y;
	f32 f2ScaleZZ = f2ScaleZ*z;

	rDestMtx.aa[0][0] = m_fScale - (f2ScaleYY + f2ScaleZZ);
	rDestMtx.aa[0][1] = f2ScaleXY + f2ScaleWZ;
	rDestMtx.aa[0][2] = f2ScaleXZ - f2ScaleWY;
	rDestMtx.aa[1][0] = f2ScaleXY - f2ScaleWZ;
	rDestMtx.aa[1][1] = m_fScale - (f2ScaleXX + f2ScaleZZ);
	rDestMtx.aa[1][2] = f2ScaleYZ + f2ScaleWX;
	rDestMtx.aa[2][0] = f2ScaleXZ + f2ScaleWY;
	rDestMtx.aa[2][1] = f2ScaleYZ - f2ScaleWX;
	rDestMtx.aa[2][2] = m_fScale - (f2ScaleXX + f2ScaleYY);

	rDestMtx.m_vPos = m_Pos;

	return rDestMtx;
}

FINLINE CFMtx44 &CFTQuatA::BuildMtx( CFMtx44 &rDestMtx ) const {
	f32 f2Scale = 2.0f * m_fScale;
	f32 f2ScaleX = f2Scale*x;
	f32 f2ScaleY = f2Scale*y;
	f32 f2ScaleZ = f2Scale*z;
	f32 f2ScaleWX = f2ScaleX*w;
	f32 f2ScaleWY = f2ScaleY*w;
	f32 f2ScaleWZ = f2ScaleZ*w;
	f32 f2ScaleXX = f2ScaleX*x;
	f32 f2ScaleXY = f2ScaleY*x;
	f32 f2ScaleXZ = f2ScaleZ*x;
	f32 f2ScaleYY = f2ScaleY*y;
	f32 f2ScaleYZ = f2ScaleZ*y;
	f32 f2ScaleZZ = f2ScaleZ*z;

	rDestMtx.aa[0][0] = m_fScale - (f2ScaleYY + f2ScaleZZ);
	rDestMtx.aa[0][1] = f2ScaleXY + f2ScaleWZ;
	rDestMtx.aa[0][2] = f2ScaleXZ - f2ScaleWY;
	rDestMtx.aa[0][3] = 0.0f;
	rDestMtx.aa[1][0] = f2ScaleXY - f2ScaleWZ;
	rDestMtx.aa[1][1] = m_fScale - (f2ScaleXX + f2ScaleZZ);
	rDestMtx.aa[1][2] = f2ScaleYZ + f2ScaleWX;
	rDestMtx.aa[1][3] = 0.0f;
	rDestMtx.aa[2][0] = f2ScaleXZ + f2ScaleWY;
	rDestMtx.aa[2][1] = f2ScaleYZ - f2ScaleWX;
	rDestMtx.aa[2][2] = m_fScale - (f2ScaleXX + f2ScaleYY);
	rDestMtx.aa[2][3] = 0.0f;

	rDestMtx.m_vPos = m_Pos;
	rDestMtx.aa[3][3] = 1.0f;

	return rDestMtx;
}

FINLINE CFMtx44A &CFTQuatA::BuildMtx( CFMtx44A &rDestMtx ) const {
	f32 f2Scale = 2.0f * m_fScale;
	f32 f2ScaleX = f2Scale*x;
	f32 f2ScaleY = f2Scale*y;
	f32 f2ScaleZ = f2Scale*z;
	f32 f2ScaleWX = f2ScaleX*w;
	f32 f2ScaleWY = f2ScaleY*w;
	f32 f2ScaleWZ = f2ScaleZ*w;
	f32 f2ScaleXX = f2ScaleX*x;
	f32 f2ScaleXY = f2ScaleY*x;
	f32 f2ScaleXZ = f2ScaleZ*x;
	f32 f2ScaleYY = f2ScaleY*y;
	f32 f2ScaleYZ = f2ScaleZ*y;
	f32 f2ScaleZZ = f2ScaleZ*z;

	rDestMtx.aa[0][0] = m_fScale - (f2ScaleYY + f2ScaleZZ);
	rDestMtx.aa[0][1] = f2ScaleXY + f2ScaleWZ;
	rDestMtx.aa[0][2] = f2ScaleXZ - f2ScaleWY;
	rDestMtx.aa[0][3] = 0.0f;
	rDestMtx.aa[1][0] = f2ScaleXY - f2ScaleWZ;
	rDestMtx.aa[1][1] = m_fScale - (f2ScaleXX + f2ScaleZZ);
	rDestMtx.aa[1][2] = f2ScaleYZ + f2ScaleWX;
	rDestMtx.aa[1][3] = 0.0f;
	rDestMtx.aa[2][0] = f2ScaleXZ + f2ScaleWY;
	rDestMtx.aa[2][1] = f2ScaleYZ - f2ScaleWX;
	rDestMtx.aa[2][2] = m_fScale - (f2ScaleXX + f2ScaleYY);
	rDestMtx.aa[2][3] = 0.0f;

	rDestMtx.m_vPos = m_PosScale;

	return rDestMtx;
}

FINLINE CFMtx43A &CFTQuatA::BuildMtx( CFMtx43A &rDestMtx ) const {
	f32 f2Scale = 2.0f * m_fScale;
	f32 f2ScaleX = f2Scale*x;
	f32 f2ScaleY = f2Scale*y;
	f32 f2ScaleZ = f2Scale*z;
	f32 f2ScaleWX = f2ScaleX*w;
	f32 f2ScaleWY = f2ScaleY*w;
	f32 f2ScaleWZ = f2ScaleZ*w;
	f32 f2ScaleXX = f2ScaleX*x;
	f32 f2ScaleXY = f2ScaleY*x;
	f32 f2ScaleXZ = f2ScaleZ*x;
	f32 f2ScaleYY = f2ScaleY*y;
	f32 f2ScaleYZ = f2ScaleZ*y;
	f32 f2ScaleZZ = f2ScaleZ*z;

	rDestMtx.aa[0][0] = m_fScale - (f2ScaleYY + f2ScaleZZ);
	rDestMtx.aa[0][1] = f2ScaleXY + f2ScaleWZ;
	rDestMtx.aa[0][2] = f2ScaleXZ - f2ScaleWY;
	rDestMtx.aa[1][0] = f2ScaleXY - f2ScaleWZ;
	rDestMtx.aa[1][1] = m_fScale - (f2ScaleXX + f2ScaleZZ);
	rDestMtx.aa[1][2] = f2ScaleYZ + f2ScaleWX;
	rDestMtx.aa[2][0] = f2ScaleXZ + f2ScaleWY;
	rDestMtx.aa[2][1] = f2ScaleYZ - f2ScaleWX;
	rDestMtx.aa[2][2] = m_fScale - (f2ScaleXX + f2ScaleYY);

	rDestMtx.m_vPos.v3 = m_PosScale.v3;

	return rDestMtx;
}

FINLINE CFTQuatA &CFTQuatA::ReceiveSlerpOf( const f32 &fUnitSlerp, const CFTQuatA &rQuat0, const CFTQuatA &rQuat1 ) {
	CFQuatA::ReceiveSlerpOf( fUnitSlerp, rQuat0, rQuat1 );
	m_Pos.ReceiveLerpOf( fUnitSlerp, rQuat0.m_Pos, rQuat1.m_Pos );
	m_fScale = FMATH_FPOT( fUnitSlerp, rQuat0.m_fScale, rQuat1.m_fScale );

	return *this;
}

FINLINE CFTQuatA &CFTQuatA::ReceiveSlerpOf( const f32 &fUnitSlerp0, const f32 &fUnitSlerp1, const CFTQuatA &rQuat0, const CFTQuatA &rQuat1 ) {
	CFQuatA::ReceiveSlerpOf( fUnitSlerp0, fUnitSlerp1, rQuat0, rQuat1 );

	m_Pos.x = fUnitSlerp0*rQuat0.m_Pos.x + fUnitSlerp1*rQuat1.m_Pos.x;
	m_Pos.y = fUnitSlerp0*rQuat0.m_Pos.y + fUnitSlerp1*rQuat1.m_Pos.y;
	m_Pos.z = fUnitSlerp0*rQuat0.m_Pos.z + fUnitSlerp1*rQuat1.m_Pos.z;

	m_fScale = fUnitSlerp0*rQuat0.m_fScale + fUnitSlerp1*rQuat1.m_fScale;

	return *this;
}

#endif	// !FANG_WINGC


