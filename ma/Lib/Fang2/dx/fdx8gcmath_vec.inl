//////////////////////////////////////////////////////////////////////////////////////
// fdx8gcmath_vec.inl - Fang vector library.
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

#if FANG_WINGC

//--------------------------------------------------------------------
// CFVec4A Implementation:
//--------------------------------------------------------------------
FINLINE CFVec4A::CFVec4A( void ) {}
FINLINE CFVec4A::CFVec4A( const CFVec4A &rV ) { Set(rV); }
FINLINE CFVec4A::CFVec4A( const f32 &fVal ) { Set(fVal); }
FINLINE CFVec4A::CFVec4A( const f32 &fX, const f32 &fY, const f32 &fZ ) { Set( fX, fY, fZ ); }
FINLINE CFVec4A::CFVec4A( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { Set( fX, fY, fZ, fW ); }
FINLINE CFVec4A::CFVec4A( const f32 *pfArray4 ) { Set(pfArray4); }
FINLINE CFVec4A::CFVec4A( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ, const u32 &nBitMaskW ) { SetBitMask( nBitMaskX, nBitMaskY, nBitMaskZ, nBitMaskW ); }

FINLINE BOOL CFVec4A::operator == ( const CFVec4A &rV ) const { return (v4 == rV.v4); }

FINLINE BOOL CFVec4A::operator != ( const CFVec4A &rV ) const { return (v4 != rV.v4); }

FINLINE CFVec4A &CFVec4A::Zero( void ) { v4.Zero(); return *this; }
FINLINE CFVec4A &CFVec4A::ZeroW1( void ) { x=y=z=0.0f; w=1.0f; return *this; }

FINLINE CFVec4A &CFVec4A::operator = ( const CFVec4A &rV ) { v4=rV.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec2 &rV ) { v2 = rV; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec3 &rV ) { v3 = rV; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec4 &rV ) { v4 = rV; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec3A &rV ) { v3 = rV.v3; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec4A &rV ) { v4 = rV.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 &fVal ) { x=y=z=w=fVal; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 &fX, const f32 &fY, const f32 &fZ ) { x=fX; y=fY; z=fZ; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { x=fX; y=fY; z=fZ; w=fW; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 *pfArray4 ) { a[0]=pfArray4[0]; a[1]=pfArray4[1]; a[2]=pfArray4[2]; a[3]=pfArray4[3]; return *this; }
FINLINE CFVec4A &CFVec4A::SetBitMask( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ, const u32 &nBitMaskW ) { *((u32 *)&x) = nBitMaskX; *((u32 *)&y) = nBitMaskY; *((u32 *)&z) = nBitMaskZ; *((u32 *)&w) = nBitMaskW; return *this; }

FINLINE CFVec4A &CFVec4A::SetToInverse( const f32 &fVal ) { f32 fInv=fmath_Inv(fVal); x=y=z=w=fInv; return *this; }

FINLINE CFVec4A &CFVec4A::ReceiveNegative( const CFVec4A &rV ) { v4=-rV.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Negate( void ) { v4=-v4; return *this; }

FINLINE CFVec4A &CFVec4A::Add( const CFVec4A &rV1, const CFVec4A &rV2 ) { v4 = rV1.v4 + rV2.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Add( const CFVec4A &rV ) { v4 += rV.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Add( const CFVec4A &rV, const f32 &fVal ) { x=rV.x+fVal; y=rV.y+fVal; z=rV.z+fVal; w=rV.w+fVal; return *this; }
FINLINE CFVec4A &CFVec4A::Add( const f32 &fVal ) { x+=fVal; y+=fVal; z+=fVal; w+=fVal; return *this; }

FINLINE CFVec4A &CFVec4A::Sub( const CFVec4A &rV1, const CFVec4A &rV2 ) { v4 = rV1.v4 - rV2.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Sub( const CFVec4A &rV ) { v4 -= rV.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Sub( const CFVec4A &rV, const f32 &fVal ) { x=rV.x-fVal; y=rV.y-fVal; z=rV.z-fVal; w=rV.w-fVal; return *this; }
FINLINE CFVec4A &CFVec4A::Sub( const f32 &fVal ) { x-=fVal; y-=fVal; z-=fVal; w-=fVal; return *this; }

FINLINE CFVec4A &CFVec4A::RevSub( const CFVec4A &rV ) { v4 = rV.v4 - v4; return *this; }

FINLINE CFVec4A &CFVec4A::RevSub( const f32 &fVal ) { v4.x=fVal-v4.x; v4.y=fVal-v4.y; v4.z=fVal-v4.z; v4.w=fVal-v4.w; return *this; }

FINLINE CFVec4A &CFVec4A::Mul( const CFVec4A &rV1, const CFVec4A &rV2 ) { v4 = rV1.v4 * rV2.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Mul( const CFVec4A &rV ) { v4 *= rV.v4; return *this; }
FINLINE CFVec4A &CFVec4A::Mul( const CFVec4A &rV, const f32 &fVal ) { x=rV.x*fVal; y=rV.y*fVal; z=rV.z*fVal; w=rV.w*fVal; return *this; }
FINLINE CFVec4A &CFVec4A::Mul( const f32 &fVal ) { x*=fVal; y*=fVal; z*=fVal; w*=fVal; return *this; }

FINLINE CFVec4A &CFVec4A::Div( const CFVec4A &rV1, const CFVec4A &rV2 ) { x=fmath_Div(rV1.x, rV2.x); y=fmath_Div(rV1.y, rV2.y); z=fmath_Div(rV1.z, rV2.z); w=fmath_Div(rV1.w, rV2.w); return *this; }
FINLINE CFVec4A &CFVec4A::Div( const CFVec4A &rV ) { x=fmath_Div(x, rV.x); y=fmath_Div(y, rV.y); z=fmath_Div(z, rV.z); w=fmath_Div(w, rV.w); return *this; }
FINLINE CFVec4A &CFVec4A::Div( const CFVec4A &rV, const f32 &fVal ) { f32 fInv=fmath_Inv(fVal); x=rV.x*fInv; y=rV.y*fInv; z=rV.z*fInv; w=rV.w*fInv; return *this; }
FINLINE CFVec4A &CFVec4A::Div( const f32 &fVal ) { f32 fInv=fmath_Inv(fVal); x*=fInv; y*=fInv; z*=fInv; w*=fInv; return *this; }

FINLINE CFVec4A &CFVec4A::ReceiveInverse( const CFVec4A &rV ) { x=fmath_Inv(rV.x); y=fmath_Inv(rV.y); z=fmath_Inv(rV.z); w=fmath_Inv(rV.w); return *this; }
FINLINE CFVec4A &CFVec4A::Invert( void ) { x=fmath_Inv(x); y=fmath_Inv(y); z=fmath_Inv(z); w=fmath_Inv(w); return *this; }

FINLINE CFVec4A &CFVec4A::Min( const CFVec4A &rV1, const CFVec4A &rV2 ) { x=FMATH_MIN(rV1.x, rV2.x); y=FMATH_MIN(rV1.y, rV2.y); z=FMATH_MIN(rV1.z, rV2.z); w=FMATH_MIN(rV1.w, rV2.w); return *this; }
FINLINE CFVec4A &CFVec4A::Max( const CFVec4A &rV1, const CFVec4A &rV2 ) { x=FMATH_MAX(rV1.x, rV2.x); y=FMATH_MAX(rV1.y, rV2.y); z=FMATH_MAX(rV1.z, rV2.z); w=FMATH_MAX(rV1.w, rV2.w); return *this; }

FINLINE CFVec4A &CFVec4A::Clamp0( void ) { FMATH_CLAMPMIN(x, 0.0f); FMATH_CLAMPMIN(y, 0.0f); FMATH_CLAMPMIN(z, 0.0f); FMATH_CLAMPMIN(w, 0.0f); return *this; }
FINLINE CFVec4A &CFVec4A::Clamp1( void ) { FMATH_CLAMPMAX(x, 1.0f); FMATH_CLAMPMAX(y, 1.0f); FMATH_CLAMPMAX(z, 1.0f); FMATH_CLAMPMAX(w, 1.0f); return *this; }
FINLINE CFVec4A &CFVec4A::Clamp01( void ) { FMATH_CLAMP(x, 0.0f, 1.0f); FMATH_CLAMP(y, 0.0f, 1.0f); FMATH_CLAMP(z, 0.0f, 1.0f); FMATH_CLAMP(w, 0.0f, 1.0f); return *this; }
FINLINE CFVec4A &CFVec4A::ClampNeg1( void ) { FMATH_CLAMPMIN(x, -1.0f); FMATH_CLAMPMIN(y, -1.0f); FMATH_CLAMPMIN(z, -1.0f); FMATH_CLAMPMIN(w, -1.0f); return *this; }
FINLINE CFVec4A &CFVec4A::ClampNeg1Pos1( void ) { FMATH_CLAMP(x, -1.0f, 1.0f); FMATH_CLAMP(y, -1.0f, 1.0f); FMATH_CLAMP(z, -1.0f, 1.0f); FMATH_CLAMP(w, -1.0f, 1.0f); return *this; }
FINLINE CFVec4A &CFVec4A::ClampMin( const CFVec4A &rMinV ) { FMATH_CLAMPMIN(x, rMinV.x); FMATH_CLAMPMIN(y, rMinV.y); FMATH_CLAMPMIN(z, rMinV.z); FMATH_CLAMPMIN(w, rMinV.w); return *this; }
FINLINE CFVec4A &CFVec4A::ClampMax( const CFVec4A &rMaxV ) { FMATH_CLAMPMAX(x, rMaxV.x); FMATH_CLAMPMAX(y, rMaxV.y); FMATH_CLAMPMAX(z, rMaxV.z); FMATH_CLAMPMAX(w, rMaxV.w); return *this; }
FINLINE CFVec4A &CFVec4A::Clamp( const CFVec4A &rMinV, const CFVec4A &rMaxV ) { FMATH_CLAMP(x, rMinV.x, rMaxV.x); FMATH_CLAMP(y, rMinV.y, rMaxV.y); FMATH_CLAMP(z, rMinV.z, rMaxV.z); FMATH_CLAMP(w, rMinV.w, rMaxV.w); return *this; }

FINLINE CFVec4A &CFVec4A::Clamp0( const CFVec4A &rTestV ) { x=FMATH_MAX(rTestV.x, 0.0f); y=FMATH_MAX(rTestV.y, 0.0f); z=FMATH_MAX(rTestV.z, 0.0f); w=FMATH_MAX(rTestV.w, 0.0f); return *this; }
FINLINE CFVec4A &CFVec4A::Clamp1( const CFVec4A &rTestV ) { x=FMATH_MIN(rTestV.x, 1.0f); y=FMATH_MIN(rTestV.y, 1.0f); z=FMATH_MIN(rTestV.z, 1.0f); w=FMATH_MIN(rTestV.w, 1.0f); return *this; }
FINLINE CFVec4A &CFVec4A::Clamp01( const CFVec4A &rTestV ) { x=FMATH_MAX(FMATH_MIN(rTestV.x, 1.0f), 0.0f); y=FMATH_MAX(FMATH_MIN(rTestV.y, 1.0f), 0.0f); z=FMATH_MAX(FMATH_MIN(rTestV.z, 1.0f), 0.0f); w=FMATH_MAX(FMATH_MIN(rTestV.w, 1.0f), 0.0f); return *this; }
FINLINE CFVec4A &CFVec4A::ClampNeg1( const CFVec4A &rTestV ) { x=FMATH_MAX(rTestV.x, -1.0f); y=FMATH_MAX(rTestV.y, -1.0f); z=FMATH_MAX(rTestV.z, -1.0f); w=FMATH_MAX(rTestV.w, -1.0f); return *this; }
FINLINE CFVec4A &CFVec4A::ClampNeg1Pos1( const CFVec4A &rTestV ) { x=FMATH_MAX(FMATH_MIN(rTestV.x, 1.0f), -1.0f); y=FMATH_MAX(FMATH_MIN(rTestV.y, 1.0f), -1.0f); z=FMATH_MAX(FMATH_MIN(rTestV.z, 1.0f), -1.0f); w=FMATH_MAX(FMATH_MIN(rTestV.w, 1.0f), -1.0f); return *this; }
FINLINE CFVec4A &CFVec4A::ClampMin( const CFVec4A &rTestV, const CFVec4A &rMinV ) { x=FMATH_MAX(rTestV.x, rMinV.x); y=FMATH_MAX(rTestV.y, rMinV.y); z=FMATH_MAX(rTestV.z, rMinV.z); w=FMATH_MAX(rTestV.w, rMinV.w); return *this; }
FINLINE CFVec4A &CFVec4A::ClampMax( const CFVec4A &rTestV, const CFVec4A &rMaxV ) { x=FMATH_MIN(rTestV.x, rMaxV.x); y=FMATH_MIN(rTestV.y, rMaxV.y); z=FMATH_MIN(rTestV.z, rMaxV.z); w=FMATH_MIN(rTestV.w, rMaxV.w); return *this; }
FINLINE CFVec4A &CFVec4A::Clamp( const CFVec4A &rTestV, const CFVec4A &rMinV, const CFVec4A &rMaxV ) { x=FMATH_MAX(FMATH_MIN(rTestV.x, rMaxV.x), rMinV.x); y=FMATH_MAX(FMATH_MIN(rTestV.y, rMaxV.y), rMinV.y); z=FMATH_MAX(FMATH_MIN(rTestV.z, rMaxV.z), rMinV.z); w=FMATH_MAX(FMATH_MIN(rTestV.w, rMaxV.w), rMinV.w); return *this; }

FINLINE f32 CFVec4A::Dist( const CFVec4A &rV ) const { return (rV.v4-v4).Mag(); }
FINLINE f32 CFVec4A::DistSq( const CFVec4A &rV ) const { return (rV.v4-v4).Mag2(); }

FINLINE f32 CFVec4A::Mag( void ) const { return v4.Mag(); }
FINLINE f32 CFVec4A::MagSq( void ) const { return v4.Mag2(); }
FINLINE f32 CFVec4A::InvMag( void ) const { return v4.InvMag(); }
FINLINE f32 CFVec4A::InvMagSq( void ) const { return v4.InvMag2(); }

FINLINE CFVec4A &CFVec4A::ReceiveUnit( const CFVec4A &rV ) { v4 = rV.v4.Unit(); return *this; }
FINLINE CFVec4A &CFVec4A::Unitize( void ) { v4.Unitize(); return *this; }

FINLINE f32 CFVec4A::UnitAndMag( const CFVec4A &rV ) { return rV.v4.ExtractUnitAndMag( v4 ); }
FINLINE f32 CFVec4A::UnitAndInvMag( const CFVec4A &rV ) { return rV.v4.ExtractUnitAndInvMag( v4 ); }
FINLINE f32 CFVec4A::SafeUnitAndMag( const CFVec4A &rV ) { return rV.v4.SafeExtractUnitAndMag( v4 ); }
FINLINE f32 CFVec4A::SafeUnitAndInvMag( const CFVec4A &rV ) { return rV.v4.SafeExtractUnitAndInvMag( v4 ); }

FINLINE f32 CFVec4A::MagXZ( void ) const { return v3.MagXZ(); }
FINLINE f32 CFVec4A::MagSqXZ( void ) const { return v3.MagXZ2(); }
FINLINE f32 CFVec4A::InvMagXZ( void ) const { return v3.InvMagXZ(); }
FINLINE f32 CFVec4A::InvMagSqXZ( void ) const { return v3.InvMagXZ2(); }

FINLINE CFVec4A &CFVec4A::ReceiveUnitXZ( const CFVec4A &rV ) { v3 = rV.v3.UnitXZ(); return *this; }
FINLINE CFVec4A &CFVec4A::UnitizeXZ( void ) { v3.UnitizeXZ(); return *this; }

FINLINE f32 CFVec4A::UnitAndMagXZ( const CFVec4A &rV ) {
#if 1
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	FASSERT( fMagXZ != 0.0f );
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return fMagXZ;
#else
	f32 fMag2 = rV.x*rV.x + rV.z*rV.z;

	__asm {
		rsqrtss	xmm1, fMag2
		mov		eax, rV
		movaps	xmm0, [eax]
		shufps	xmm1, xmm1, 00h
		mov		eax, this
		movaps	[eax], xmm0
		rcpss	xmm2, xmm1
		mulps	xmm0, xmm1
		movss	fMag2, xmm2
		movss	[eax], xmm0
		shufps	xmm0, xmm0, 02h
		movss	[eax+8], xmm0
	}

	FMATH_CLASS_DEBUG_FCHECK( *this );
	FMATH_DEBUG_FCHECK( fMag2 );

	return fMag2;
#endif
}


FINLINE f32 CFVec4A::UnitAndInvMagXZ( const CFVec4A &rV ) {
#if 1
	FASSERT( rV.x*rV.x + rV.z*rV.z != 0.f );
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	x = rV.x * fInvMagXZ;
	y = rV.y;
	z = rV.z * fInvMagXZ;

	return fInvMagXZ;
#else
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	_m = rV._m;
	x *= fInvMagXZ;
	z *= fInvMagXZ;

	FMATH_CLASS_DEBUG_FCHECK( *this );
	FMATH_DEBUG_FCHECK( fInvMagXZ );

	return fInvMagXZ;
#endif
}


FINLINE f32 CFVec4A::SafeUnitAndMagXZ( const CFVec4A &rV ) {
#if 1
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	if ( fMagXZ == 0.0f )
		return -1.f;
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return fMagXZ;
#else
	f32 fMag2 = rV.x*rV.x + rV.z*rV.z;

	if( fMag2 > 0.0f ) {
		__asm {
			rsqrtss	xmm1, fMag2
			mov		eax, rV
			movaps	xmm0, [eax]
			shufps	xmm1, xmm1, 00h
			mov		eax, this
			movaps	[eax], xmm0
			rcpss	xmm2, xmm1
			mulps	xmm0, xmm1
			movss	fMag2, xmm2
			movss	[eax], xmm0
			shufps	xmm0, xmm0, 02h
			movss	[eax+8], xmm0
		}

		FMATH_CLASS_DEBUG_FCHECK( *this );
		FMATH_DEBUG_FCHECK( fMag2 );

		return fMag2;
	} else {
		return -1.0f;
	}
#endif
}


FINLINE f32 CFVec4A::SafeUnitAndInvMagXZ( const CFVec4A &rV ) {
#if 1
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	if ( fMagXZ == 0.0f )
		return -1.f;
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return fInvMagXZ;
#else
	f32 fMag2 = rV.x*rV.x + rV.z*rV.z;

	if( fMag2 > 0.0f ) {
		__asm {
			rsqrtss	xmm1, fMag2
			mov		eax, rV
			movaps	xmm0, [eax]
			shufps	xmm1, xmm1, 00h
			mov		eax, this
			movaps	[eax], xmm0
			mulps	xmm0, xmm1
			movss	fMag2, xmm1
			movss	[eax], xmm0
			shufps	xmm0, xmm0, 02h
			movss	[eax+8], xmm0
		}

		FMATH_CLASS_DEBUG_FCHECK( *this );
		FMATH_DEBUG_FCHECK( fMag2 );

		return fMag2;
	} else {
		return -1.0f;
	}
#endif
}


FINLINE CFVec4A &CFVec4A::Cross( const CFVec4A &rV1, const CFVec4A &rV2 ) { v3 = rV1.v3.Cross( rV2.v3 ); w=0.0f; return *this; }
FINLINE CFVec4A &CFVec4A::Cross( const CFVec4A &rV ) { CFVec3 Cross = v3.Cross(rV.v3); v3=Cross; w=0.0f; return *this; }

FINLINE CFVec4A &CFVec4A::UnitCross( const CFVec4A &rV1, const CFVec4A &rV2 ) {
	return Cross( rV1, rV2 ).Unitize();
}

FINLINE CFVec4A &CFVec4A::UnitCross( const CFVec4A &rV ) {
	return Cross( rV ).Unitize();
}

FINLINE CFVec4A &CFVec4A::Cross( const CFVec3A &rV1, const CFVec3A &rV2 ) { return Cross( rV1.v4a, rV2.v4a ); }

FINLINE CFVec4A &CFVec4A::Cross( const CFVec3A &rV ) { return Cross( rV.v4a ); }

FINLINE CFVec4A &CFVec4A::UnitCross( const CFVec3A &rV1, const CFVec3A &rV2 ) { return UnitCross( rV1.v4a, rV2.v4a ); }

FINLINE CFVec4A &CFVec4A::UnitCross( const CFVec3A &rV ) { return UnitCross( rV.v4a ); }

FINLINE f32 CFVec4A::Dot( const CFVec4A &rV1 ) const { return x*rV1.x + y*rV1.y + z*rV1.z + w*rV1.w; }

FINLINE CFVec4A &CFVec4A::Lerp( const f32 &fUnitVal, const CFVec4A &rV1, const CFVec4A &rV2 ) {
	x = FMATH_FPOT( fUnitVal, rV1.x, rV2.x );
	y = FMATH_FPOT( fUnitVal, rV1.y, rV2.y );
	z = FMATH_FPOT( fUnitVal, rV1.z, rV2.z );
	w = FMATH_FPOT( fUnitVal, rV1.w, rV2.w );
	return *this;
}

FINLINE CFVec4A &CFVec4A::RotateX( const f32 &fRadians ) {
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec4A &CFVec4A::RotateY( const f32 &fRadians ) {
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec4A &CFVec4A::RotateZ( const f32 &fRadians ) {
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

FINLINE CFVec4A &CFVec4A::RotateX( const f32 &fSin, const f32 &fCos ) {
	f32 fTemp10, fTemp20;

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec4A &CFVec4A::RotateY( const f32 &fSin, const f32 &fCos ) {
	f32 fTemp10, fTemp20;

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec4A &CFVec4A::RotateZ( const f32 &fSin, const f32 &fCos ) {
	f32 fTemp10, fTemp20;

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

FINLINE CFVec4A &CFVec4A::ReceiveRotationX( const CFVec4A &rV, const f32 &fRadians ) {
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

FINLINE CFVec4A &CFVec4A::ReceiveRotationY( const CFVec4A &rV, const f32 &fRadians ) {
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

FINLINE CFVec4A &CFVec4A::ReceiveRotationZ( const CFVec4A &rV, const f32 &fRadians ) {
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}

FINLINE CFVec4A &CFVec4A::ReceiveRotationX( const CFVec4A &rV, const f32 &fSin, const f32 &fCos ) {
	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

FINLINE CFVec4A &CFVec4A::ReceiveRotationY( const CFVec4A &rV, const f32 &fSin, const f32 &fCos ) {
	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

FINLINE CFVec4A &CFVec4A::ReceiveRotationZ( const CFVec4A &rV, const f32 &fSin, const f32 &fCos ) {
	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}

FINLINE u32 CFVec4A::GenKey( void ) const {
	return (*(u32 *)&x) ^ (*(u32 *)&y) ^ (*(u32 *)&z) ^ (*(u32 *)&w);
}




//--------------------------------------------------------------------
// CFVec3A Implementation:
//--------------------------------------------------------------------
FINLINE CFVec3A::CFVec3A( void ) { w=0; }
FINLINE CFVec3A::CFVec3A( const CFVec3 &rV ) { x = rV.x; y = rV.y; z = rV.z; w = 0.f; }
FINLINE CFVec3A::CFVec3A( const CFVec3A &rV ) { v3=rV.v3; w=rV.w; }
FINLINE CFVec3A::CFVec3A( const f32 &fVal ) { x=y=z=fVal; w=0.0f; }
FINLINE CFVec3A::CFVec3A( const f32 &fX, const f32 &fY, const f32 &fZ ) { Set( fX, fY, fZ ); w=0.0f; }
FINLINE CFVec3A::CFVec3A( const f32 *pfArray3 ) { Set( pfArray3 ); w=0.0f; }
FINLINE CFVec3A::CFVec3A( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ ) { SetBitMask( nBitMaskX, nBitMaskY, nBitMaskZ ); w=0.0f; }

FINLINE f32 CFVec3A::GetY( void ) { return y; }

FINLINE BOOL CFVec3A::operator == ( const CFVec3A &rV ) const { return (v3 == rV.v3); }
FINLINE BOOL CFVec3A::operator != ( const CFVec3A &rV ) const { return (v3 != rV.v3); }

FINLINE CFVec3A &CFVec3A::Zero( void ) { v3.Zero(); return *this; }

FINLINE CFVec3A &CFVec3A::operator = ( const CFVec3A &rV ) { v3=rV.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const CFVec2 &rV ) { v2 = rV; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const CFVec3 &rV ) { v3 = rV; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const CFVec4 &rV ) { v3 = rV.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const CFVec3A &rV ) { v3 = rV.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const CFVec4A &rV ) { v3 = rV.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const f32 &fVal ) { x=fVal; y=fVal; z=fVal; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const f32 &fX, const f32 &fY, const f32 &fZ ) { x=fX; y=fY; z=fZ; return *this; }
FINLINE CFVec3A &CFVec3A::Set( const f32 *pfArray3 ) { a[0]=pfArray3[0]; a[1]=pfArray3[1]; a[2]=pfArray3[2]; return *this; }
FINLINE CFVec3A &CFVec3A::SetBitMask( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ ) { *((u32 *)&x) = nBitMaskX; *((u32 *)&y) = nBitMaskY; *((u32 *)&z) = nBitMaskZ; return *this; }

FINLINE CFVec3A &CFVec3A::SetToInverse( const f32 &fVal ) { f32 fInv=fmath_Inv(fVal); x=y=z=fInv; return *this; }

FINLINE CFVec3A &CFVec3A::ReceiveNegative( const CFVec3A &rV ) { v3=-rV.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Negate( void ) { v3=-v3; return *this; }

FINLINE CFVec3A &CFVec3A::Add( const CFVec3A &rV1, const CFVec3A &rV2 ) { v3 = rV1.v3 + rV2.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Add( const CFVec3A &rV ) { v3 += rV.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Add( const CFVec3A &rV, const f32 &fVal ) { x=rV.x+fVal; y=rV.y+fVal; z=rV.z+fVal; return *this; }
FINLINE CFVec3A &CFVec3A::Add( const f32 &fVal ) { x+=fVal; y+=fVal; z+=fVal; return *this; }

FINLINE CFVec3A &CFVec3A::Sub( const CFVec3A &rV1, const CFVec3A &rV2 ) { v3 = rV1.v3 - rV2.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Sub( const CFVec3A &rV ) { v3 -= rV.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Sub( const CFVec3A &rV, const f32 &fVal ) { x=rV.x-fVal; y=rV.y-fVal; z=rV.z-fVal; return *this; }
FINLINE CFVec3A &CFVec3A::Sub( const f32 &fVal ) { x-=fVal; y-=fVal; z-=fVal; return *this; }

FINLINE CFVec3A &CFVec3A::RevSub( const CFVec3A &rV ) { v3=rV.v3-v3; return *this; }
FINLINE CFVec3A &CFVec3A::RevSub( const f32 &fVal ) { x=fVal-x; y=fVal-y; z=fVal-z; return *this; }

FINLINE CFVec3A &CFVec3A::Mul( const CFVec3A &rV1, const CFVec3A &rV2 ) { v3 = rV1.v3 * rV2.v3; return *this; }
FINLINE CFVec3A &CFVec3A::Mul( const CFVec3A &rV ) { x*=rV.x; y*=rV.y; z*=rV.z; return *this; }
FINLINE CFVec3A &CFVec3A::Mul( const CFVec3A &rV, const f32 &fVal ) { x=rV.x*fVal; y=rV.y*fVal; z=rV.z*fVal; return *this; }
FINLINE CFVec3A &CFVec3A::Mul( const f32 &fVal ) { x*=fVal; y*=fVal; z*=fVal; return *this; }

FINLINE CFVec3A &CFVec3A::Div( const CFVec3A &rV1, const CFVec3A &rV2 ) { x=fmath_Div(rV1.x, rV2.x); y=fmath_Div(rV1.y, rV2.y); z=fmath_Div(rV1.z, rV2.z); return *this; }
FINLINE CFVec3A &CFVec3A::Div( const CFVec3A &rV ) { x=fmath_Div(x, rV.x); y=fmath_Div(y, rV.y); z=fmath_Div(z, rV.z); return *this; }
FINLINE CFVec3A &CFVec3A::Div( const CFVec3A &rV, const f32 &fVal ) { f32 fInv=fmath_Inv(fVal); x=rV.x*fInv; y=rV.y*fInv; z=rV.z*fInv; return *this; }
FINLINE CFVec3A &CFVec3A::Div( const f32 &fVal ) { f32 fInv=fmath_Inv(fVal); x*=fInv; y*=fInv; z*=fInv; return *this; }

FINLINE CFVec3A &CFVec3A::ReceiveInverse( const CFVec3A &rV ) { x=fmath_Inv(rV.x); y=fmath_Inv(rV.y); z=fmath_Inv(rV.z); return *this; }
FINLINE CFVec3A &CFVec3A::Invert( void ) { x=fmath_Inv(x); y=fmath_Inv(y); z=fmath_Inv(z); return *this; }

FINLINE CFVec3A &CFVec3A::Min( const CFVec3A &rV1, const CFVec3A &rV2 ) { x=FMATH_MIN(rV1.x, rV2.x); y=FMATH_MIN(rV1.y, rV2.y); z=FMATH_MIN(rV1.z, rV2.z); return *this; }
FINLINE CFVec3A &CFVec3A::Max( const CFVec3A &rV1, const CFVec3A &rV2 ) { x=FMATH_MAX(rV1.x, rV2.x); y=FMATH_MAX(rV1.y, rV2.y); z=FMATH_MAX(rV1.z, rV2.z); return *this; }

FINLINE CFVec3A &CFVec3A::Clamp0( void ) { FMATH_CLAMPMIN(x, 0.0f); FMATH_CLAMPMIN(y, 0.0f); FMATH_CLAMPMIN(z, 0.0f); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp1( void ) { FMATH_CLAMPMAX(x, 1.0f); FMATH_CLAMPMAX(y, 1.0f); FMATH_CLAMPMAX(z, 1.0f); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp01( void ) { FMATH_CLAMP(x, 0.0f, 1.0f); FMATH_CLAMP(y, 0.0f, 1.0f); FMATH_CLAMP(z, 0.0f, 1.0f); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1( void ) { FMATH_CLAMPMIN(x, -1.0f); FMATH_CLAMPMIN(y, -1.0f); FMATH_CLAMPMIN(z, -1.0f); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1Pos1( void ) { FMATH_CLAMP(x, -1.0f, 1.0f); FMATH_CLAMP(y, -1.0f, 1.0f); FMATH_CLAMP(z, -1.0f, 1.0f); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMin( const CFVec3A &rMinV ) { FMATH_CLAMPMIN(x, rMinV.x); FMATH_CLAMPMIN(y, rMinV.y); FMATH_CLAMPMIN(z, rMinV.z); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMax( const CFVec3A &rMaxV ) { FMATH_CLAMPMAX(x, rMaxV.x); FMATH_CLAMPMAX(y, rMaxV.y); FMATH_CLAMPMAX(z, rMaxV.z); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp( const CFVec3A &rMinV, const CFVec3A &rMaxV ) { FMATH_CLAMP(x, rMinV.x, rMaxV.x); FMATH_CLAMP(y, rMinV.y, rMaxV.y); FMATH_CLAMP(z, rMinV.z, rMaxV.z); return *this; }

FINLINE CFVec3A &CFVec3A::Clamp0( const CFVec3A &rTestV ) { x=FMATH_MAX(rTestV.x, 0.0f); y=FMATH_MAX(rTestV.y, 0.0f); z=FMATH_MAX(rTestV.z, 0.0f); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp1( const CFVec3A &rTestV ) { x=FMATH_MIN(rTestV.x, 1.0f); y=FMATH_MIN(rTestV.y, 1.0f); z=FMATH_MIN(rTestV.z, 1.0f); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp01( const CFVec3A &rTestV ) { x=FMATH_MAX(FMATH_MIN(rTestV.x, 1.0f), 0.0f); y=FMATH_MAX(FMATH_MIN(rTestV.y, 1.0f), 0.0f); z=FMATH_MAX(FMATH_MIN(rTestV.z, 1.0f), 0.0f); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1( const CFVec3A &rTestV ) { x=FMATH_MAX(rTestV.x, -1.0f); y=FMATH_MAX(rTestV.y, -1.0f); z=FMATH_MAX(rTestV.z, -1.0f); w=FMATH_MAX(rTestV.w, -1.0f); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1Pos1( const CFVec3A &rTestV ) { x=FMATH_MAX(FMATH_MIN(rTestV.x, 1.0f), -1.0f); y=FMATH_MAX(FMATH_MIN(rTestV.y, 1.0f), -1.0f); z=FMATH_MAX(FMATH_MIN(rTestV.z, 1.0f), -1.0f); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMin( const CFVec3A &rTestV, const CFVec3A &rMinV ) { x=FMATH_MAX(rTestV.x, rMinV.x); y=FMATH_MAX(rTestV.y, rMinV.y); z=FMATH_MAX(rTestV.z, rMinV.z); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMax( const CFVec3A &rTestV, const CFVec3A &rMaxV ) { x=FMATH_MIN(rTestV.x, rMaxV.x); y=FMATH_MIN(rTestV.y, rMaxV.y); z=FMATH_MIN(rTestV.z, rMaxV.z); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp( const CFVec3A &rTestV, const CFVec3A &rMinV, const CFVec3A &rMaxV ) { x=FMATH_MAX(FMATH_MIN(rTestV.x, rMaxV.x), rMinV.x); y=FMATH_MAX(FMATH_MIN(rTestV.y, rMaxV.y), rMinV.y); z=FMATH_MAX(FMATH_MIN(rTestV.z, rMaxV.z), rMinV.z); return *this; }


FINLINE f32 CFVec3A::Dist( const CFVec3A &rV ) const { return (rV.v3-v3).Mag(); }
FINLINE f32 CFVec3A::DistSq( const CFVec3A &rV ) const { return (rV.v3-v3).Mag2(); }

FINLINE f32 CFVec3A::Mag( void ) const { return v3.Mag(); }
FINLINE f32 CFVec3A::MagSq( void ) const { return v3.Mag2(); }
FINLINE f32 CFVec3A::InvMag( void ) const { return v3.InvMag(); }
FINLINE f32 CFVec3A::InvMagSq( void ) const { return v3.InvMag2(); }

FINLINE CFVec3A &CFVec3A::ReceiveUnit( const CFVec3A &rV ) { v3 = rV.v3.Unit(); return *this; }
FINLINE CFVec3A &CFVec3A::Unitize( void ) { v3.Unitize(); return *this; }

FINLINE f32 CFVec3A::UnitAndMag( const CFVec3A &rV ) { return rV.v3.ExtractUnitAndMag( v3 ); }
FINLINE f32 CFVec3A::UnitAndInvMag( const CFVec3A &rV ) { return rV.v3.ExtractUnitAndInvMag( v3 ); }
FINLINE f32 CFVec3A::SafeUnitAndMag( const CFVec3A &rV ) { return rV.v3.SafeExtractUnitAndMag( v3 ); }
FINLINE f32 CFVec3A::SafeUnitAndInvMag( const CFVec3A &rV ) { return rV.v3.SafeExtractUnitAndInvMag( v3 ); }

FINLINE f32 CFVec3A::DistXZ( const CFVec3A &rV ) const { return (rV.v3-v3).MagXZ(); }
FINLINE f32 CFVec3A::DistSqXZ( const CFVec3A &rV ) const { return (rV.v3-v3).MagXZ2(); }

FINLINE f32 CFVec3A::MagXZ( void ) const { return v3.MagXZ(); }
FINLINE f32 CFVec3A::MagSqXZ( void ) const { return v3.MagXZ2(); }
FINLINE f32 CFVec3A::InvMagXZ( void ) const { return v3.InvMagXZ(); }
FINLINE f32 CFVec3A::InvMagSqXZ( void ) const { return v3.InvMagXZ2(); }

FINLINE CFVec3A &CFVec3A::UnitizeXZ( void ) { v3.UnitizeXZ(); return *this; }

FINLINE CFVec3A &CFVec3A::ReceiveUnitXZ( const CFVec3A &rV ) { 
	f32 fInvMagXZ = rV.v3.InvMagXZ();
	
	x = rV.x * fInvMagXZ;
	y = 0.0f;
	z = rV.z * fInvMagXZ;
		
	return *this; 
}

FINLINE f32 CFVec3A::UnitAndMagXZ( const CFVec3A &rV ) {
#if 1
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	FASSERT( fMagXZ != 0.0f );
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return fMagXZ;
#else
	f32 fMag2 = rV.x*rV.x + rV.z*rV.z;

	__asm {
		rsqrtss	xmm1, fMag2
		mov		eax, rV
		movaps	xmm0, [eax]
		shufps	xmm1, xmm1, 00h
		mov		eax, this
		movaps	[eax], xmm0
		rcpss	xmm2, xmm1
		mulps	xmm0, xmm1
		movss	fMag2, xmm2
		movss	[eax], xmm0
		shufps	xmm0, xmm0, 02h
		movss	[eax+8], xmm0
	}

	FMATH_CLASS_DEBUG_FCHECK( *this );
	FMATH_DEBUG_FCHECK( fMag2 );

	return fMag2;
#endif
}


FINLINE f32 CFVec3A::UnitAndInvMagXZ( const CFVec3A &rV ) {
#if 1
	FASSERT( rV.x*rV.x + rV.z*rV.z != 0.f );
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	x = rV.x * fInvMagXZ;
	y = rV.y;
	z = rV.z * fInvMagXZ;

	return fInvMagXZ;
#else
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	_m = rV._m;
	x *= fInvMagXZ;
	z *= fInvMagXZ;

	FMATH_CLASS_DEBUG_FCHECK( *this );
	FMATH_DEBUG_FCHECK( fInvMagXZ );

	return fInvMagXZ;
#endif
}


FINLINE f32 CFVec3A::SafeUnitAndMagXZ( const CFVec3A &rV ) {
#if 1
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	if ( fMagXZ == 0.0f )
		return -1.f;
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return fMagXZ;
#else
	f32 fMag2 = rV.x*rV.x + rV.z*rV.z;

	if( fMag2 > 0.0f ) {
		__asm {
			rsqrtss	xmm1, fMag2
			mov		eax, rV
			movaps	xmm0, [eax]
			shufps	xmm1, xmm1, 00h
			mov		eax, this
			movaps	[eax], xmm0
			rcpss	xmm2, xmm1
			mulps	xmm0, xmm1
			movss	fMag2, xmm2
			movss	[eax], xmm0
			shufps	xmm0, xmm0, 02h
			movss	[eax+8], xmm0
		}

		FMATH_CLASS_DEBUG_FCHECK( *this );
		FMATH_DEBUG_FCHECK( fMag2 );

		return fMag2;
	} else {
		return -1.0f;
	}
#endif
}


FINLINE f32 CFVec3A::SafeUnitAndInvMagXZ( const CFVec3A &rV ) {
#if 1
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	if ( fMagXZ == 0.0f )
		return -1.f;
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return fInvMagXZ;
#else
	f32 fMag2 = rV.x*rV.x + rV.z*rV.z;

	if( fMag2 > 0.0f ) {
		__asm {
			rsqrtss	xmm1, fMag2
			mov		eax, rV
			movaps	xmm0, [eax]
			shufps	xmm1, xmm1, 00h
			mov		eax, this
			movaps	[eax], xmm0
			mulps	xmm0, xmm1
			movss	fMag2, xmm1
			movss	[eax], xmm0
			shufps	xmm0, xmm0, 02h
			movss	[eax+8], xmm0
		}

		FMATH_CLASS_DEBUG_FCHECK( *this );
		FMATH_DEBUG_FCHECK( fMag2 );

		return fMag2;
	} else {
		return -1.0f;
	}
#endif
}


FINLINE CFVec3A &CFVec3A::Cross( const CFVec3A &rV1, const CFVec3A &rV2 ) {
	x = rV1.y*rV2.z - rV1.z*rV2.y;
	y = rV1.z*rV2.x - rV1.x*rV2.z;
	z = rV1.x*rV2.y - rV1.y*rV2.x;

	return *this;
}


FINLINE CFVec3A &CFVec3A::Cross( const CFVec3A &rV ) { CFVec3 Cross = v3.Cross(rV.v3); v3=Cross; return *this; }


FINLINE CFVec3A &CFVec3A::UnitCross( const CFVec3A &rV1, const CFVec3A &rV2 ) {
	return Cross( rV1, rV2 ).Unitize();
}


FINLINE CFVec3A &CFVec3A::UnitCross( const CFVec3A &rV ) {
	return Cross( rV ).Unitize();
}


FINLINE CFVec3A &CFVec3A::Cross( const CFVec4A &rV1, const CFVec4A &rV2 ) {
	x = rV1.y*rV2.z - rV1.z*rV2.y;
	y = rV1.z*rV2.x - rV1.x*rV2.z;
	z = rV1.x*rV2.y - rV1.y*rV2.x;

	return *this;
}


FINLINE CFVec3A &CFVec3A::Cross( const CFVec4A &rV ) { CFVec3 Cross = v3.Cross(rV.v3); v3=Cross; return *this; }


FINLINE CFVec3A &CFVec3A::UnitCross( const CFVec4A &rV1, const CFVec4A &rV2 ) {
	return Cross( rV1, rV2 ).Unitize();
}


FINLINE CFVec3A &CFVec3A::UnitCross( const CFVec4A &rV ) {
	return Cross( rV ).Unitize();
}


FINLINE CFVec3A &CFVec3A::CrossYWithVec( const CFVec3A &rV ) {
	x = rV.z;
	y = 0.0f;
	z = -rV.x;

	return *this;
}

FINLINE f32 CFVec3A::Dot( const CFVec3A &rV1 ) const { return x*rV1.x + y*rV1.y + z*rV1.z; }


FINLINE CFVec3A &CFVec3A::Lerp( const f32 &fUnitVal, const CFVec3A &rV1, const CFVec3A &rV2 ) {
	x = FMATH_FPOT( fUnitVal, rV1.x, rV2.x );
	y = FMATH_FPOT( fUnitVal, rV1.y, rV2.y );
	z = FMATH_FPOT( fUnitVal, rV1.z, rV2.z );
	return *this;
}


FINLINE CFVec3A &CFVec3A::Lerp( const f32 &fUnitVal, const CFVec3A &rV ) {
	x = FMATH_FPOT( fUnitVal, x, rV.x );
	y = FMATH_FPOT( fUnitVal, y, rV.y );
	z = FMATH_FPOT( fUnitVal, z, rV.z );
	return *this;
}

FINLINE CFVec3A &CFVec3A::RotateX( const f32 &fRadians ) {
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec3A &CFVec3A::RotateY( const f32 &fRadians ) {
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec3A &CFVec3A::RotateZ( const f32 &fRadians ) {
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

FINLINE CFVec3A &CFVec3A::RotateX( const f32 &fSin, const f32 &fCos ) {
	f32 fTemp10, fTemp20;

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec3A &CFVec3A::RotateY( const f32 &fSin, const f32 &fCos ) {
	f32 fTemp10, fTemp20;

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

FINLINE CFVec3A &CFVec3A::RotateZ( const f32 &fSin, const f32 &fCos ) {
	f32 fTemp10, fTemp20;

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceiveRotationX( const CFVec3A &rV, const f32 &fRadians ) {
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceiveRotationY( const CFVec3A &rV, const f32 &fRadians ) {
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceiveRotationZ( const CFVec3A &rV, const f32 &fRadians ) {
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceiveRotationX( const CFVec3A &rV, const f32 &fSin, const f32 &fCos ) {
	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceiveRotationY( const CFVec3A &rV, const f32 &fSin, const f32 &fCos ) {
	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceiveRotationZ( const CFVec3A &rV, const f32 &fSin, const f32 &fCos ) {
	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}

FINLINE u32 CFVec3A::GenKey( void ) const {
	return (*(u32 *)&x) ^ (*(u32 *)&y) ^ (*(u32 *)&z);
}


//--------------------------------------------------------------------
// Port additions.
//
// The GameCube build (gc/fGCmath_vec.inl) and the SSE build (fdx8math_vec.inl)
// implement these, but this GC-layout PC configuration was only ever used by the
// data tools, which never called them. The game runtime does, so they are supplied
// here as plain scalar code identical in behavior to the GC reference.
//--------------------------------------------------------------------

FINLINE CFVec3A &CFVec3A::CrossVecWithY( const CFVec3A &rV ) {
	x = -rV.z;
	y = 0.0f;
	z = rV.x;
	return *this;
}

FINLINE CFVec3A &CFVec3A::UnitCrossVecWithY( const CFVec3A &rV ) {
	return CrossVecWithY( rV ).UnitizeXZ();
}

FINLINE CFVec3A &CFVec3A::UnitCrossYWithVec( const CFVec3A &rV ) {
	return CrossYWithVec( rV ).UnitizeXZ();
}

// Reflects this vector about rNormal. Works with non-normalized vectors.
FINLINE CFVec3A &CFVec3A::Reflect( const CFVec3A &rNormal ) {
	CFVec3A vTemp;
	vTemp.Mul( rNormal, -2.0f * Dot( rNormal ) );
	Add( vTemp );
	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceiveReflection( const CFVec3A &rV, const CFVec3A &rNormal ) {
	Mul( rNormal, -2.0f * rV.Dot( rNormal ) ).Add( rV );
	return *this;
}

// Projects this vector onto the plane defined by unit vector rvNormal.
FINLINE CFVec3A &CFVec3A::PlanarProjection( const CFVec3A &rvNormal ) {
	CFVec3A vTemp;
	vTemp = rvNormal;
	vTemp.Mul( rvNormal.Dot( *this ) );
	Sub( vTemp );
	return *this;
}

FINLINE CFVec3A &CFVec3A::ReceivePlanarProjection( const CFVec3A &rV, const CFVec3A &rvNormal ) {
	Set( rvNormal );
	Mul( rvNormal.Dot( rV ) );
	Negate();
	Add( rV );
	return *this;
}

// TRUE if this vector is within the yaw/pitch cones around vCenter/vUp (both normalized).
FINLINE BOOL CFVec3A::InYawPitchRange( const CFVec3A &vCenter, const CFVec3A &vUp, const f32 fYawCos, const f32 fPitchCos ) {
	CFVec3A vProjected;
	vProjected.ReceivePlanarProjection( *this, vUp );

	if( vCenter.Dot( vProjected ) < fYawCos ) {
		return FALSE;
	}
	if( Dot( vProjected ) < fPitchCos ) {
		return FALSE;
	}
	return TRUE;
}

#endif	// FANG_WINGC

