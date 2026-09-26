//////////////////////////////////////////////////////////////////////////////////////
// fGCmath_vec.inl - Fang vector library.
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
// 02/18/02	Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////



//--------------------------------------------------------------------
// CFVec2 Implementation:
//--------------------------------------------------------------------
FINLINE CFVec2::CFVec2( void ) {}
FINLINE CFVec2::CFVec2( const f32 &fX, const f32 &fY ) { x=fX; y=fY; }
FINLINE CFVec2::CFVec2( const CFVec2 &v ) { x=v.x; y=v.y; }
FINLINE CFVec2::CFVec2( const CFVec3 &v ) { x=v.x; y=v.y; }
FINLINE CFVec2::CFVec2( const CFVec4 &v ) { x=v.x; y=v.y; }

FINLINE CFVec2 &CFVec2::Zero( void ) { x = y = 0.0f; return *this; }
FINLINE CFVec2 &CFVec2::Set( const f32 &fX, const f32 &fY ) { x=fX; y=fY; return *this; }
FINLINE CFVec2 &CFVec2::operator =  ( const CFVec2 &v ) { x=v.x; y=v.y; return *this; }
FINLINE CFVec2 &CFVec2::operator = ( const CFVec3 &v ) { x=v.x; y=v.y; return *this; }
FINLINE CFVec2 &CFVec2::operator = ( const CFVec4 &v ) { x=v.x; y=v.y; return *this; }
FINLINE CFVec2 &CFVec2::operator += ( const CFVec2 &v ) { x+=v.x; y+=v.y; return *this; } 
FINLINE CFVec2 &CFVec2::operator -= ( const CFVec2 &v ) { x-=v.x; y-=v.y; return *this; } 
FINLINE CFVec2 &CFVec2::operator *= ( const f32 &fS ) { x*=fS; y*=fS; return *this; }
FINLINE CFVec2 &CFVec2::operator /= ( const f32 &fS ) { FASSERT( fS != 0.0f ); f32 fR=fmath_Inv(fS); x*=fR; y*=fR; return *this; }

FINLINE CFVec2 CFVec2::operator - ( void ) const { return CFVec2( -x, -y ); }

FINLINE CFVec2 CFVec2::operator + ( const f32 &fS ) const { return CFVec2( x+fS, y+fS ); }
FINLINE CFVec2 CFVec2::operator - ( const f32 &fS ) const { return CFVec2( x-fS, y-fS ); }
FINLINE CFVec2 CFVec2::operator * ( const f32 &fS ) const { return CFVec2( x*fS, y*fS ); }
FINLINE CFVec2 CFVec2::operator / ( const f32 &fS ) const { FASSERT( fS != 0.0f ); f32 fR = fmath_Inv(fS); return CFVec2( x*fR, y*fR ); }
FINLINE CFVec2 CFVec2::operator + ( const CFVec2 &v ) const { return CFVec2( x+v.x, y+v.y ); }
FINLINE CFVec2 CFVec2::operator - ( const CFVec2 &v ) const { return CFVec2( x-v.x, y-v.y ); }
FINLINE CFVec2 CFVec2::operator * ( const CFVec2 &v ) const { return CFVec2( x*v.x, y*v.y ); }
FINLINE CFVec2 CFVec2::operator / ( const CFVec2 &v ) const { return CFVec2( fmath_Div( x, v.x ), fmath_Div( y, v.y ) ); }

FINLINE BOOL CFVec2::operator == ( const CFVec2 &v ) const { return ((v.x==x) && (v.y==y)); }
FINLINE BOOL CFVec2::operator != ( const CFVec2 &v ) const { return ((v.x!=x) || (v.y!=y)); }

FINLINE f32 CFVec2::Dot( const CFVec2 &v ) const { return (x*v.x + y*v.y); }
FINLINE f32 CFVec2::Mag() const { return fmath_Sqrt( x*x + y*y ); }
FINLINE f32 CFVec2::Mag2() const { return (x*x + y*y); }
FINLINE f32 CFVec2::InvMag() const { return (fmath_InvSqrt( x*x + y*y )); }
FINLINE f32 CFVec2::InvMag2() const { FASSERT( x*x + y*y != 0.0f ); return (fmath_Inv(x*x + y*y)); }
FINLINE CFVec2 &CFVec2::Unitize() { (*this) *= InvMag(); return *this; }
FINLINE CFVec2 CFVec2::Unit() const { CFVec2 v=*this; return v.Unitize(); }

FINLINE f32 CFVec2::ExtractUnitAndMag( CFVec2 &rUnitVec ) const {
	f32 fMag = Mag();
	FASSERT( fMag != 0.0f ); 
	rUnitVec = *this * fmath_Inv(fMag);
	return fMag;
}

FINLINE f32 CFVec2::SafeExtractUnitAndMag( CFVec2 &rUnitVec ) const {
	f32 fMag = Mag();

	if( fMag > 0.0f ) {
		rUnitVec = *this * fmath_Inv(fMag);
		return fMag;
	} else {
		return -1.0f;
	}
}

FINLINE f32 CFVec2::ExtractUnitAndInvMag( CFVec2 &rUnitVec ) const {
	f32 fOOMag = InvMag();
	rUnitVec = *this * fOOMag;
	return fOOMag;
}

FINLINE f32 CFVec2::SafeExtractUnitAndInvMag( CFVec2 &rUnitVec ) const {
	f32 fMag = Mag();

	if( fMag > 0.0f ) {
		f32 fOOMag = fmath_Inv(fMag);
		rUnitVec = *this * fOOMag;
		return fOOMag;
	} else {
		return -1.0f;
	}
}

FINLINE CFVec2 &CFVec2::ReceiveLerpOf( f32 fUnitLerp, const CFVec2 &rVec0, const CFVec2 &rVec1 ) {
	*this = rVec0*(1.0f - fUnitLerp) + rVec1*fUnitLerp;
	return *this;
}

FINLINE CFVec2 &CFVec2::Rotate( const f32 &fRadians ) {
	f32 fCos, fSin, fX, fY;

   fX = x;
   fY = y;
   fmath_SinCos( fRadians, &fSin, &fCos );
   x = fX*fCos + fY*fSin;
   y = fY*fCos - fX*fSin;

   return *this;
}

FINLINE CFVec2 &CFVec2::Min( const CFVec2 &v ) {
	x = FMATH_MIN( x, v.x );
	y = FMATH_MIN( y, v.y );
	return *this;
}

FINLINE CFVec2 &CFVec2::Max( const CFVec2 &v ) {
	x = FMATH_MAX( x, v.x );
	y = FMATH_MAX( y, v.y );
	return *this;
}



//--------------------------------------------------------------------
// CFVec3 Implementation:
//--------------------------------------------------------------------
FINLINE CFVec3::CFVec3( void ) {}
FINLINE CFVec3::CFVec3( const f32 &fX, const f32 &fY, const f32 &fZ ) { x=fX; y=fY; z=fZ; }
FINLINE CFVec3::CFVec3( const CFVec2 &v, f32 fZ ) { x=v.x; y=v.y; z=fZ; }
FINLINE CFVec3::CFVec3( const CFVec3 &v ) { x=v.x; y=v.y; z=v.z; }
FINLINE CFVec3::CFVec3( const CFVec4 &v ) { x=v.x; y=v.y; z=v.z; }

FINLINE CFVec3 &CFVec3::Zero( void ) { x = y = z = 0.0f; return *this; }
FINLINE CFVec3 &CFVec3::Set( const f32 &fX, const f32 &fY, const f32 &fZ ) { x=fX; y=fY; z=fZ; return *this; }
FINLINE CFVec3 &CFVec3::operator =  ( const CFVec2 &v ) { x=v.x; y=v.y; z=0.0f; return *this; }
FINLINE CFVec3 &CFVec3::operator =  ( const CFVec3 &v ) { x=v.x; y=v.y; z=v.z; return *this; }
FINLINE CFVec3 &CFVec3::operator =  ( const CFVec4 &v ) { x=v.x; y=v.y; z=v.z; return *this; }
FINLINE CFVec3 &CFVec3::operator += ( const CFVec3 &v ) { x+=v.x; y+=v.y; z+=v.z; return *this; } 
FINLINE CFVec3 &CFVec3::operator -= ( const CFVec3 &v ) { x-=v.x; y-=v.y; z-=v.z; return *this; } 
FINLINE CFVec3 &CFVec3::operator *= ( const f32 &fS ) { x*=fS; y*=fS; z*=fS; return *this; }
FINLINE CFVec3 &CFVec3::operator /= ( const f32 &fS ) { FASSERT( fS != 0.0f ); f32 fR=fmath_Inv(fS); x*=fR; y*=fR; z*=fR; return *this; }

FINLINE CFVec3 CFVec3::operator - ( void ) const { return CFVec3( -x, -y, -z ); }

FINLINE CFVec3 CFVec3::operator + ( const f32 &fS ) const { return CFVec3( x+fS, y+fS, z+fS ); }
FINLINE CFVec3 CFVec3::operator - ( const f32 &fS ) const { return CFVec3( x-fS, y-fS, z-fS ); }
FINLINE CFVec3 CFVec3::operator * ( const f32 &fS ) const { return CFVec3( x*fS, y*fS, z*fS ); }
FINLINE CFVec3 CFVec3::operator / ( const f32 &fS ) const { FASSERT( fS != 0.0f ); f32 fR = fmath_Inv(fS); return CFVec3( x*fR, y*fR, z*fR ); }
FINLINE CFVec3 CFVec3::operator + ( const CFVec3 &v ) const { return CFVec3( x+v.x, y+v.y, z+v.z ); }
FINLINE CFVec3 CFVec3::operator - ( const CFVec3 &v ) const { return CFVec3( x-v.x, y-v.y, z-v.z ); }
FINLINE CFVec3 CFVec3::operator * ( const CFVec3 &v ) const { return CFVec3( x*v.x, y*v.y, z*v.z ); }
FINLINE CFVec3 CFVec3::operator / ( const CFVec3 &v ) const { return CFVec3( fmath_Div( x, v.x ), fmath_Div( y, v.y ), fmath_Div( z, v.z ) ); }

FINLINE BOOL CFVec3::operator == ( const CFVec3 &v ) const { return ((v.x==x) && (v.y==y) && (v.z==z)); }
FINLINE BOOL CFVec3::operator != ( const CFVec3 &v ) const { return ((v.x!=x) || (v.y!=y) || (v.z!=z)); }

FINLINE CFVec3 CFVec3::Cross( const CFVec3 &v ) const { return CFVec3( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x ); }
FINLINE CFVec3 CFVec3::UnitCross( const CFVec3 &v ) const { return Cross(v).Unit(); }
FINLINE f32 CFVec3::Dot( const CFVec3 &v ) const { return (x*v.x + y*v.y + z*v.z); }
FINLINE f32 CFVec3::Mag() const { return fmath_Sqrt( x*x + y*y + z*z ); }
FINLINE f32 CFVec3::Mag2() const { return (x*x + y*y + z*z); }
FINLINE f32 CFVec3::InvMag() const { return (fmath_InvSqrt( x*x + y*y + z*z )); }
FINLINE f32 CFVec3::InvMag2() const { FASSERT( x*x + y*y + z*z != 0.0f ); return (fmath_Inv(x*x + y*y + z*z)); }
FINLINE f32 CFVec3::MagXZ() const { return fmath_Sqrt( x*x + z*z ); }
FINLINE f32 CFVec3::MagXZ2() const { return (x*x + z*z); }
FINLINE f32 CFVec3::InvMagXZ() const { return (fmath_InvSqrt( x*x + z*z )); }
FINLINE f32 CFVec3::InvMagXZ2() const { FASSERT( x*x + z*z != 0.0f ); return (fmath_Inv(x*x + z*z)); }
FINLINE CFVec3 &CFVec3::Unitize() { (*this) *= InvMag(); return *this; }
FINLINE CFVec3 CFVec3::Unit() const { CFVec3 v=*this; return v.Unitize(); }
FINLINE CFVec3 &CFVec3::UnitizeXZ() { (*this) *= InvMagXZ(); return *this; }
FINLINE CFVec3 CFVec3::UnitXZ() const { CFVec3 v=*this; return v.UnitizeXZ(); }

FINLINE f32 CFVec3::ExtractUnitAndMag( CFVec3 &rUnitVec ) const {
	f32 fMag = Mag();
	FASSERT( fMag != 0.0f ); 
	rUnitVec = *this * fmath_Inv(fMag);
	return fMag;
}

FINLINE f32 CFVec3::SafeExtractUnitAndMag( CFVec3 &rUnitVec ) const {
	f32 fMag = Mag();

	if( fMag > 0.0f ) {
		rUnitVec = *this * fmath_Inv(fMag);
		return fMag;
	} else {
		return -1.0f;
	}
}

FINLINE f32 CFVec3::ExtractUnitAndInvMag( CFVec3 &rUnitVec ) const {
	f32 fOOMag = InvMag();
	rUnitVec = *this * fOOMag;
	return fOOMag;
}

FINLINE f32 CFVec3::SafeExtractUnitAndInvMag( CFVec3 &rUnitVec ) const {
	f32 fMag = Mag();
	if( fMag > 0.0f ) {
		f32 fOOMag = fmath_Inv(fMag);
		rUnitVec = *this * fOOMag;
		return fOOMag;
	} else {
		return -1.0f;
	}
}

FINLINE CFVec3 &CFVec3::ReceiveLerpOf( f32 fUnitLerp, const CFVec3 &rVec0, const CFVec3 &rVec1 ) {
	*this = rVec0*(1.0f - fUnitLerp) + rVec1*fUnitLerp;
	return *this;
}

FINLINE CFVec3 &CFVec3::Min( const CFVec3 &v ) {
	x = FMATH_MIN( x, v.x );
	y = FMATH_MIN( y, v.y );
	z = FMATH_MIN( z, v.z );
	return *this;
}

FINLINE CFVec3 &CFVec3::Max( const CFVec3 &v ) {
	x = FMATH_MAX( x, v.x );
	y = FMATH_MAX( y, v.y );
	z = FMATH_MAX( z, v.z );
	return *this;
}

//
// Rotates the vector by "fRadians" around the X axis.
FINLINE void CFVec3::RotateX( const f32 fRadians ) 
{
	f32 fSinR, fCosR, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSinR, &fCosR );

	fTemp10 = y*fCosR - z*fSinR;
	fTemp20 = y*fSinR + z*fCosR;

	y = fTemp10;
	z = fTemp20;
};

//
// Rotates the vector by "fRadians" around the Y axis.
FINLINE void CFVec3::RotateY( const f32 fRadians ) 
{
	f32 fSinR, fCosR, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSinR, &fCosR );

	fTemp10 = z*fSinR + x*fCosR;
	fTemp20 = z*fCosR - x*fSinR;

	x = fTemp10;
	z = fTemp20;
};

//
// Reflects a vector across a normal.  For example, if
// a ray intersects a poly with a normal of rReflectionNormal, 
// you can Reflect the normalized ray direction vector using 
// this function and the poly normal as the reflection normal.
// The result is the new normalized direction vector for the 
// ray reflection.  This function will work with non-normalized 
// vectors, as well.
FINLINE CFVec3 &CFVec3::Reflect( const CFVec3 &rReflectNormal )
{
	CFVec3 vTemp;
	vTemp = rReflectNormal * -2.f * Dot( rReflectNormal );
	*this += vTemp;
	return *this;
}

//
// See descrption for Reflect(), above
FINLINE CFVec3 &CFVec3::ReceiveReflection( const CFVec3 &rV, const CFVec3 &rNormal )
{
	*this = rNormal * (-2.f * rV.Dot(rNormal));
	*this += rV;
	return *this;
}




//--------------------------------------------------------------------
// CFVec4 Implementation:
//--------------------------------------------------------------------
FINLINE CFVec4::CFVec4( void ) {}
FINLINE CFVec4::CFVec4( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { x=fX; y=fY; z=fZ; w=fW; }
FINLINE CFVec4::CFVec4( const CFVec2 &v, f32 fZ, f32 fW ) { x=v.x; y=v.y; z=fZ; w=fW; }
FINLINE CFVec4::CFVec4( const CFVec3 &v, f32 fW ) { x=v.x; y=v.y; z=v.z; w=fW; }
FINLINE CFVec4::CFVec4( const CFVec4 &v ) { x=v.x; y=v.y; z=v.z; w=v.w; }

FINLINE CFVec4 &CFVec4::Zero( void ) { x = y = z = w = 0.0f; return *this; }
FINLINE CFVec4 &CFVec4::Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { x=fX; y=fY; z=fZ; w=fW; return *this; }
FINLINE CFVec4 &CFVec4::operator =  ( const CFVec4 &v ) { x=v.x; y=v.y; z=v.z; w=v.w; return *this; }
FINLINE CFVec4 &CFVec4::operator =  ( const CFVec3 &v ) { x=v.x; y=v.y; z=v.z; w=0.0f; return *this; }
FINLINE CFVec4 &CFVec4::operator =  ( const CFVec2 &v ) { x=v.x; y=v.y; z=0.0f; w=0.0f; return *this; }
FINLINE CFVec4 &CFVec4::operator += ( const CFVec4 &v ) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; } 
FINLINE CFVec4 &CFVec4::operator -= ( const CFVec4 &v ) { x-=v.x; y-=v.y; z-=v.z; w-=v.w; return *this; } 
FINLINE CFVec4 &CFVec4::operator *= ( const CFVec4 &v ) { x*=v.x; y*=v.y; z*=v.z; w*=v.w; return *this; } 
FINLINE CFVec4 &CFVec4::operator *= ( const f32 &fS ) { x*=fS; y*=fS; z*=fS; w*=fS; return *this; }
FINLINE CFVec4 &CFVec4::operator /= ( const f32 &fS ) { FASSERT( fS != 0.0f ); f32 fR=fmath_Inv(fS); x*=fR; y*=fR; z*=fR; w*=fR; return *this; }

FINLINE CFVec4 CFVec4::operator - ( void ) const { return CFVec4( -x, -y, -z, -w ); }

FINLINE CFVec4 CFVec4::operator + ( const f32 &fS ) const {	return CFVec4( x+fS, y+fS, z+fS, w+fS ); }
FINLINE CFVec4 CFVec4::operator - ( const f32 &fS ) const {	return CFVec4( x-fS, y-fS, z-fS, w-fS ); }
FINLINE CFVec4 CFVec4::operator * ( const f32 &fS ) const {	return CFVec4( x*fS, y*fS, z*fS, w*fS ); }
FINLINE CFVec4 CFVec4::operator / ( const f32 &fS ) const { FASSERT( fS != 0.0f ); f32 fR = fmath_Inv(fS); return CFVec4( x*fR, y*fR, z*fR, w*fR ); }
FINLINE CFVec4 CFVec4::operator + ( const CFVec4 &v ) const { return CFVec4( x+v.x, y+v.y, z+v.z, w+v.w ); }
FINLINE CFVec4 CFVec4::operator - ( const CFVec4 &v ) const { return CFVec4( x-v.x, y-v.y, z-v.z, w-v.w ); }
FINLINE CFVec4 CFVec4::operator * ( const CFVec4 &v ) const { return CFVec4( x*v.x, y*v.y, z*v.z, w*v.w ); }
FINLINE CFVec4 CFVec4::operator / ( const CFVec4 &v ) const { return CFVec4( fmath_Div( x, v.x ), fmath_Div( y, v.y ), fmath_Div( z, v.z ), fmath_Div( w, v.w ) ); }

FINLINE BOOL CFVec4::operator == ( const CFVec4 &v ) const { return ((v.x==x) && (v.y==y) && (v.z==z) && (v.w==w)); }
FINLINE BOOL CFVec4::operator != ( const CFVec4 &v ) const { return ((v.x!=x) || (v.y!=y) || (v.z!=z) || (v.w!=w)); }

FINLINE f32 CFVec4::Dot( const CFVec4 &v ) const { return (x*v.x + y*v.y + z*v.z + w*v.w); }
FINLINE f32 CFVec4::Mag() const { return fmath_Sqrt( x*x + y*y + z*z + w*w ); }
FINLINE f32 CFVec4::Mag2() const { return (x*x + y*y + z*z + w*w); }
FINLINE f32 CFVec4::InvMag() const { return (fmath_InvSqrt( x*x + y*y + z*z + w*w )); }
FINLINE f32 CFVec4::InvMag2() const { FASSERT( x*x + y*y + z*z + w*w != 0.0f ); return fmath_Inv(x*x + y*y + z*z + w*w); }
FINLINE CFVec4 &CFVec4::Unitize() { (*this) *= InvMag(); return *this; }
FINLINE CFVec4 CFVec4::Unit() const { CFVec4 v=*this; return v.Unitize(); }

FINLINE f32 CFVec4::ExtractUnitAndMag( CFVec4 &rUnitVec ) const {
	f32 fMag = Mag();
	FASSERT( fMag != 0.0f );
	rUnitVec = *this * fmath_Inv(fMag);
	return fMag;
}

FINLINE f32 CFVec4::SafeExtractUnitAndMag( CFVec4 &rUnitVec ) const {
	f32 fMag = Mag();

	if( fMag > 0.0f ) {
		rUnitVec = *this * fmath_Inv(fMag);
		return fMag;
	} else {
		return -1.0f;
	}
}

FINLINE f32 CFVec4::ExtractUnitAndInvMag( CFVec4 &rUnitVec ) const {
	f32 fOOMag = InvMag();
	rUnitVec = *this * fOOMag;
	return fOOMag;
}

FINLINE f32 CFVec4::SafeExtractUnitAndInvMag( CFVec4 &rUnitVec ) const {
	f32 fMag = Mag();

	if( fMag > 0.0f ) {
		f32 fOOMag = fmath_Inv(fMag);
		rUnitVec = *this * fOOMag;
		return fOOMag;
	} else {
		return -1.0f;
	}
}

FINLINE CFVec4 &CFVec4::ReceiveLerpOf( f32 fUnitLerp, const CFVec4 &rVec0, const CFVec4 &rVec1 ) {
	*this = rVec0*(1.0f - fUnitLerp) + rVec1*fUnitLerp;
	return *this;
}



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

FINLINE CFVec4A &CFVec4A::Zero( void ) { x = 0; y = 0; z = 0; w = 0; return *this; }
FINLINE CFVec4A &CFVec4A::ZeroW1( void ) { _A = CFVec4A::m_UnitAxisW._A; _B = CFVec4A::m_UnitAxisW._B; return *this; }

FINLINE CFVec4A &CFVec4A::operator = ( const CFVec4A &rV ) { _A = rV._A; _B = rV._B; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec2 &rV ) { v2 = rV; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec3 &rV ) { v3 = rV; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec4 &rV ) { v4 = rV; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec3A &rV ) { v3 = rV.v3; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const CFVec4A &rV ) { _A = rV._A; _B = rV._B; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 &fVal ) { x = fVal; y = fVal; _B = _A; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 &fX, const f32 &fY, const f32 &fZ ) { x=fX; y=fY; z=fZ; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { x=fX; y=fY; z=fZ; w=fW; return *this; }
FINLINE CFVec4A &CFVec4A::Set( const f32 *pfArray4 ) { a[0]=pfArray4[0]; a[1]=pfArray4[1]; a[2]=pfArray4[2]; a[3]=pfArray4[3]; return *this; }
//FINLINE CFVec4A &CFVec4A::SetBitMask( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ, const u32 &nBitMaskW ) { *((u32 *)&_m.m128_f32[0]) = nBitMaskX; *((u32 *)&_m.m128_f32[1]) = nBitMaskY; *((u32 *)&_m.m128_f32[2]) = nBitMaskZ; *((u32 *)&_m.m128_f32[3]) = nBitMaskW; return *this; }
FINLINE CFVec4A &CFVec4A::ReceiveNegative( const CFVec4A &rV ) { _A = -rV._A; _B = -rV._B; return *this; }
FINLINE CFVec4A &CFVec4A::Negate( void ) { _A = -_A; _B = -_B; return *this; }

//
//
FINLINE CFVec4A &CFVec4A::SetToInverse( const f32 &fVal ) 
{ 
	float fI = 1.f / fVal;
	x = fI;
	y = fI;
	_B = _A;
	return *this; 
}

//
//
FINLINE CFVec4A &CFVec4A::Add( register const CFVec4A &rV1, register const CFVec4A &rV2 ) 
{ 
	register CFVec4A *pThis = this;
	register f32 _r0, _r1, _r2, _r3;
	asm
	{
		psq_l	_r0, 0(rV1), 0, 0
		psq_l	_r2, 0(rV2), 0, 0
		
		ps_add 	_r0, _r0, _r2
		psq_st	_r0, 0(pThis), 0, 0
		
		psq_l	_r1, 8(rV1), 0, 0
		psq_l	_r3, 8(rV2), 0, 0
	
		ps_add 	_r1, _r1, _r3
		psq_st	_r1, 8(pThis), 0, 0
	}
	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Add( register const CFVec4A &rV ) 
{ 
	register CFVec4A *pThis = this;
	register f32 _r0, _r1, _r2, _r3;
	asm
	{
		psq_l	_r0, 0(rV), 0, 0
		psq_l	_r2, 0(pThis), 0, 0
		
		ps_add 	_r0, _r0, _r2
		psq_st	_r0, 0(pThis), 0, 0
		
		psq_l	_r1, 8(rV), 0, 0
		psq_l	_r3, 8(pThis), 0, 0
	
		ps_add 	_r1, _r1, _r3
		psq_st	_r1, 8(pThis), 0, 0
	}
	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Add( const CFVec4A &rV, const f32 &fVal ) 
{
	x = rV.x + fVal;
	y = rV.y + fVal;
	z = rV.z + fVal;
	w = rV.w + fVal;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Add( const f32 &fVal ) 
{
	x += fVal;
	y += fVal;
	z += fVal;
	w += fVal;

	return *this;
}

FINLINE CFVec4A &CFVec4A::Sub( const CFVec4A &rV1, const CFVec4A &rV2 ) { _A = rV1._A - rV2._A; _B = rV1._B - rV2._B ; return *this; }
FINLINE CFVec4A &CFVec4A::Sub( const CFVec4A &rV ) { _A = _A - rV._A; _B = _B - rV._B; return *this; }

//
//
FINLINE CFVec4A &CFVec4A::Sub( const CFVec4A &rV, const f32 &fVal ) 
{
	x = rV.x - fVal;
	y = rV.y - fVal;
	z = rV.z - fVal;
	w = rV.w - fVal;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Sub( const f32 &fVal ) 
{
	x -= fVal;
	y -= fVal;
	z -= fVal;
	w -= fVal;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RevSub( const f32 &fVal ) 
{
	x = fVal - x;
	y = fVal - y;
	z = fVal - z;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RevSub( const CFVec4A &rV ) 
{
	_A = rV._A - _A; 
	_B = rV._B - _B; 
	return *this; 
}

FINLINE CFVec4A &CFVec4A::Mul( const CFVec4A &rV1, const CFVec4A &rV2 ) { _A = rV1._A * rV2._A; _B = rV1._B * rV2._B; return *this; }
FINLINE CFVec4A &CFVec4A::Mul( const CFVec4A &rV ) { _A = _A * rV._A; _B = _B * rV._B; return *this; }

//
//
FINLINE CFVec4A &CFVec4A::Mul( const CFVec4A &rV, const f32 &fVal ) 
{
	x = rV.x * fVal;
	y = rV.y * fVal;
	z = rV.z * fVal;
	w = rV.w * fVal;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Mul( const f32 &fVal ) 
{
	x *= fVal;
	y *= fVal;
	z *= fVal;
	w *= fVal;

	return *this;
}


FINLINE CFVec4A &CFVec4A::Div( const CFVec4A &rV1, const CFVec4A &rV2 ) { _A = rV1._A / rV2._A; _B = rV1._B / rV2._B; return *this; }
FINLINE CFVec4A &CFVec4A::Div( const CFVec4A &rV ) {_A = _A / rV._A; _B = _B / rV._B;; return *this; }

//
//
FINLINE CFVec4A &CFVec4A::Div( const CFVec4A &rV, const f32 &fVal ) 
{
	f32 ftemp = 1.f / fVal;
	x = rV.x * ftemp;
	y = rV.y * ftemp;
	z = rV.z * ftemp;
	w = rV.w * ftemp;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Div( const f32 &fVal ) 
{
	f32 ftemp = 1.f / fVal;
	x *= ftemp;
	y *= ftemp;
	z *= ftemp;
	w *= ftemp;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveInverse( const CFVec4A &rV ) 
{ 
	x = 1.f/rV.x;
	x = 1.f/rV.y;
	x = 1.f/rV.z;
	x = 1.f/rV.w;
	
	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Invert( void ) 
{ 
	x = 1.f/x;
	x = 1.f/y;
	x = 1.f/z;
	x = 1.f/w;
	
	return *this;
}

//
//
//FINLINE CFVec4A &CFVec4A::Min( const CFVec4A &rV1, const CFVec4A &rV2 ) { _m = _mm_min_ps( rV1._m, rV2._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::Max( const CFVec4A &rV1, const CFVec4A &rV2 ) { _m = _mm_max_ps( rV1._m, rV2._m ); return *this; }

//FINLINE CFVec4A &CFVec4A::Clamp0( void ) { _m = _mm_max_ps( _m, m_Null._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::Clamp1( void ) { _m = _mm_min_ps( _m, m_Ones._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::Clamp01( void ) { _m = _mm_max_ps( _mm_min_ps( _m, m_Ones._m ), m_Null._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::ClampNeg1( void ) { _m = _mm_max_ps( _m, m_NegOnes._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::ClampNeg1Pos1( void ) { _m = _mm_max_ps( _mm_min_ps( _m, m_Ones._m ), m_NegOnes._m ); return *this; }

//
//
FINLINE CFVec4A &CFVec4A::ClampMin( const CFVec4A &rMinV ) 
{ 
	if ( x < rMinV.x )
	{
		x = rMinV.x;
	}
	if ( y < rMinV.y )
	{
		y = rMinV.y;
	}
	if ( z < rMinV.z )
	{
		z = rMinV.z;
	}
	if ( w < rMinV.w )
	{
		w = rMinV.w;
	}
		
	return *this; 
}

//
//
FINLINE CFVec4A &CFVec4A::ClampMax( const CFVec4A &rMaxV ) 
{ 
	if ( x > rMaxV.x )
	{
		x = rMaxV.x;
	}
	if ( y > rMaxV.y )
	{
		y = rMaxV.y;
	}
	if ( z > rMaxV.z )
	{
		z = rMaxV.z;
	}
	if ( w > rMaxV.w )
	{
		w = rMaxV.w;
	}
		
	return *this; 
}

//FINLINE CFVec4A &CFVec4A::Clamp( const CFVec4A &rMinV, const CFVec4A &rMaxV ) { _m = _mm_max_ps( _mm_min_ps( _m, rMaxV._m ), rMinV._m ); return *this; }

//FINLINE CFVec4A &CFVec4A::Clamp0( const CFVec4A &rTestV ) { _m = _mm_max_ps( rTestV._m, m_Null._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::Clamp1( const CFVec4A &rTestV ) { _m = _mm_min_ps( rTestV._m, m_Ones._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::Clamp01( const CFVec4A &rTestV ) { _m = _mm_max_ps( _mm_min_ps( rTestV._m, m_Ones._m ), m_Null._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::ClampNeg1( const CFVec4A &rTestV ) { _m = _mm_max_ps( rTestV._m, m_NegOnes._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::ClampNeg1Pos1( const CFVec4A &rTestV ) { _m = _mm_max_ps( _mm_min_ps( rTestV._m, m_Ones._m ), m_NegOnes._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::ClampMin( const CFVec4A &rTestV, const CFVec4A &rMinV ) { _m = _mm_max_ps( rTestV._m, rMinV._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::ClampMax( const CFVec4A &rTestV, const CFVec4A &rMaxV ) { _m = _mm_min_ps( rTestV._m, rMaxV._m ); return *this; }
//FINLINE CFVec4A &CFVec4A::Clamp( const CFVec4A &rTestV, const CFVec4A &rMinV, const CFVec4A &rMaxV ) { _m = _mm_max_ps( _mm_min_ps( rTestV._m, rMaxV._m ), rMinV._m ); return *this; }

//
//
FINLINE f32 CFVec4A::Mag( void ) const 
{
	return fmath_Sqrt( x*x + y*y + z*z + w*w );
}

//
//
FINLINE f32 CFVec4A::MagSq( void ) const 
{
	return x*x + y*y + z*z + w*w;
}

//
//
FINLINE f32 CFVec4A::InvMag( void ) const 
{
	return fmath_InvSqrt( x*x + y*y + z*z + w*w );
}

//
//
FINLINE f32 CFVec4A::InvMagSq( void ) const 
{
	FASSERT( x*x + y*y + z*z + w*w != 0.0f );
	return fmath_Inv( x*x + y*y + z*z + w*w );
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveUnit( const CFVec4A &rV ) 
{
	f32 fIM = rV.InvMag();
	
	x = rV.x * fIM;
	y = rV.y * fIM;
	z = rV.z * fIM;
	w = rV.w * fIM;
	
	return *this;
}


FINLINE CFVec4A &CFVec4A::Unitize( void ) 
{
	f32 fIM = InvMag();
	
	x = x * fIM;
	y = y * fIM;
	z = z * fIM;
	w = w * fIM;
	
	return *this;
}

/*
FINLINE f32 CFVec4A::UnitAndMag( const CFVec4A &rV ) 
{
	f32 fMag;

	__asm {
		mov		eax, rV
		movaps	xmm1, [eax]
		movaps	xmm2, xmm1

		mulps	xmm1, xmm1
		movaps	xmm0, xmm1
		shufps	xmm1, xmm1, 4eh
		addps	xmm0, xmm1
		movaps	xmm1, xmm0
		shufps	xmm1, xmm1, 11h
		addps	xmm0, xmm1
		rsqrtss	xmm0, xmm0
		rcpss	xmm1, xmm0
		movss	fMag, xmm1
		shufps	xmm0, xmm0, 00h
		mulps	xmm2, xmm0
		mov		eax, this
		movaps	[eax], xmm2
	}

	return fMag;
}


FINLINE f32 CFVec4A::UnitAndInvMag( const CFVec4A &rV ) 
{
	CFVec4A TempVec;

	_m = _mm_mul_ps( rV._m, rV._m );
	_m = _mm_add_ps( _m, _mm_shuffle_ps( _m, _m, 0x4e ) );
	_m = _mm_add_ps( _m, _mm_shuffle_ps( _m, _m, 0x11 ) );

	TempVec._m = _mm_rsqrt_ps( _m );
	_m = _mm_mul_ps( rV._m, TempVec._m );

	return TempVec._m.m128_f32[0];
}


FINLINE f32 CFVec4A::SafeUnitAndMag( const CFVec4A &rV ) 
{
	f32 fMag2 = rV.x*rV.x + rV.y*rV.y + rV.z*rV.z + rV.w*rV.w;
	f32 fMag;

	if( fMag2 > 0.0f ) {
		__asm {
			rsqrtss	xmm0, fMag2
			shufps	xmm0, xmm0, 00h
			rcpss	xmm1, xmm0
			mov		edx, rV
			mulps	xmm0, [edx]
			movss	fMag, xmm1
			mov		eax, this
			movaps	[eax], xmm0
		}

		return fMag;
	} else {
		return -1.0f;
	}
}


FINLINE f32 CFVec4A::SafeUnitAndInvMag( const CFVec4A &rV ) 
{
	f32 fMag2 = rV.x*rV.x + rV.y*rV.y + rV.z*rV.z + rV.w*rV.w;

	if( fMag2 > 0.0f ) {
		__asm {
			mov		eax, rV
			rsqrtss	xmm0, fMag2
			shufps	xmm0, xmm0, 00h
			movss	fMag2, xmm0
			mulps	xmm0, [eax]
			mov		eax, this
			movaps	[eax], xmm0
		}

		return fMag2;
	} else {
		return -1.0f;
	}
}
*/

//
//
FINLINE f32 CFVec4A::MagXZ( void ) const 
{
	return fmath_Sqrt( x*x + z*z );
}

//
//
FINLINE f32 CFVec4A::MagSqXZ( void ) const 
{
	return x*x + z*z;
}

//
//
FINLINE f32 CFVec4A::InvMagXZ( void ) const 
{
	return fmath_InvSqrt( x*x + z*z );
}

//
//
FINLINE f32 CFVec4A::InvMagSqXZ( void ) const 
{
	FASSERT( x*x + z*z != 0.0f );
	return fmath_Inv( x*x + z*z );
}

//
//
FINLINE CFVec4A &CFVec4A::UnitizeXZ( void ) 
{
	f32 fInvMagXZ = fmath_InvSqrt( x*x + z*z );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveUnitXZ( const CFVec4A &rV ) 
{
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	w = 0.0f;
	x = rV.x * fInvMagXZ;
	y = 0.0f;
	z = rV.z * fInvMagXZ;


	return *this;
}

/*
//
//
FINLINE f32 CFVec4A::UnitAndMagXZ( const CFVec4A &rV ) 
{
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

	return fMag2;
}


FINLINE f32 CFVec4A::UnitAndInvMagXZ( const CFVec4A &rV ) 
{
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	_m = rV._m;
	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return fInvMagXZ;
}

FINLINE f32 CFVec4A::SafeUnitAndMagXZ( const CFVec4A &rV ) 
{
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

		return fMag2;
	} else {
		return -1.0f;
	}
}


FINLINE f32 CFVec4A::SafeUnitAndInvMagXZ( const CFVec4A &rV ) 
{
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

		return fMag2;
	} else {
		return -1.0f;
	}
}
*/

//
//
FINLINE CFVec4A &CFVec4A::Cross( register const CFVec4A &rV1, register const CFVec4A &rV2 ) 
{
	register f32 fV1xy, fV1zw, fV2xy, fV2zw;
	register f32 fT1, fT2, fT3, fT4;
	register CFVec4A *pThis = this;
	asm
	{
		// BX | BY
		psq_l       fV2xy, 0(rV2), 0, 0
		// AZ | AZ
		lfs         fV1zw, 8(rV1)
		// AX | AY
		psq_l       fV1xy, 0(rV1), 0, 0
		// BY | BX
		ps_merge10  fT1, fV2xy, fV2xy
		// BZ | BZ
		lfs         fV2zw, 8(rV2)

		// BX*AZ | BY*AZ
		ps_mul      fT2, fV2xy, fV1zw
		// BX*AX | BY*AX
		ps_muls0    fT3, fV2xy, fV1xy
		// AX*BZ-BX*AZ | AY*BZ-BY*AZ
		ps_msub     fT2, fV1xy, fV2zw, fT2
		// AX*BY-BX*AX | AY*BX-BY*AX
		ps_msub     fT3, fV1xy, fT1, fT3

		// AY*BZ-AZ*BY | AY*BZ-AZ*BY
		ps_merge11  fT4, fT2, fT2
		// AX*BZ-AZ*BX | AY*BX-AX*BY
		ps_merge01  fT1, fT2, fT3

		psq_st      fT4, 0(pThis), 1, 0

		// AZ*BX-AX*BZ | AX*BY-AY*BX
		ps_neg      fT1, fT1
	
		psq_st      fT1, 4(pThis), 0, 0
	}

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::Cross( register const CFVec4A &rV ) 
{
	register f32 fV1xy, fV1zw, fV2xy, fV2zw;
	register f32 fT1, fT2, fT3, fT4;
	register CFVec4A *pThis = this;
	asm
	{
		// BX | BY
		psq_l       fV2xy, 0(rV), 0, 0
		// AZ | AZ
		lfs         fV1zw, 8(pThis)
		// AX | AY
		psq_l       fV1xy, 0(pThis), 0, 0
		// BY | BX
		ps_merge10  fT1, fV2xy, fV2xy
		// BZ | BZ
		lfs         fV2zw, 8(rV)

		// BX*AZ | BY*AZ
		ps_mul      fT2, fV2xy, fV1zw
		// BX*AX | BY*AX
		ps_muls0    fT3, fV2xy, fV1xy
		// AX*BZ-BX*AZ | AY*BZ-BY*AZ
		ps_msub     fT2, fV1xy, fV2zw, fT2
		// AX*BY-BX*AX | AY*BX-BY*AX
		ps_msub     fT3, fV1xy, fT1, fT3

		// AY*BZ-AZ*BY | AY*BZ-AZ*BY
		ps_merge11  fT4, fT2, fT2
		// AX*BZ-AZ*BX | AY*BX-AX*BY
		ps_merge01  fT1, fT2, fT3

		psq_st      fT4, 0(pThis), 1, 0

		// AZ*BX-AX*BZ | AX*BY-AY*BX
		ps_neg      fT1, fT1
	
		psq_st      fT1, 4(pThis), 0, 0
	}

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::UnitCross( const CFVec4A &rV1, const CFVec4A &rV2 ) 
{
	return Cross( rV1, rV2 ).Unitize();
}

//
//
FINLINE CFVec4A &CFVec4A::UnitCross( const CFVec4A &rV ) 
{
	return Cross( rV ).Unitize();
}

//
//
FINLINE f32 CFVec4A::Dot( const CFVec4A &rV1 ) const 
{ 
	return x*rV1.x + y*rV1.y + z*rV1.z + w*rV1.w; 
}

//
//
FINLINE CFVec4A &CFVec4A::Lerp( const f32 &fUnitVal, const CFVec4A &rV1, const CFVec4A &rV2 ) 
{
	x = rV1.x + ((rV2.x - rV1.x) * fUnitVal);
	y = rV1.y + ((rV2.y - rV1.y) * fUnitVal);
	z = rV1.z + ((rV2.z - rV1.z) * fUnitVal);
	w = rV1.w + ((rV2.w - rV1.w) * fUnitVal);

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RotateX( const f32 &fRadians ) 
{
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RotateY( const f32 &fRadians ) 
{
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RotateZ( const f32 &fRadians ) 
{
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RotateX( const f32 &fSin, const f32 &fCos ) 
{
	f32 fTemp10, fTemp20;

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RotateY( const f32 &fSin, const f32 &fCos ) 
{
	f32 fTemp10, fTemp20;

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::RotateZ( const f32 &fSin, const f32 &fCos ) 
{
	f32 fTemp10, fTemp20;

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveRotationX( const CFVec4A &rV, const f32 &fRadians ) 
{
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveRotationY( const CFVec4A &rV, const f32 &fRadians ) 
{
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveRotationZ( const CFVec4A &rV, const f32 &fRadians ) 
{
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveRotationX( const CFVec4A &rV, const f32 &fSin, const f32 &fCos ) 
{
	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveRotationY( const CFVec4A &rV, const f32 &fSin, const f32 &fCos ) 
{
	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

//
//
FINLINE CFVec4A &CFVec4A::ReceiveRotationZ( const CFVec4A &rV, const f32 &fSin, const f32 &fCos ) 
{
	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}



//--------------------------------------------------------------------
// CFVec3A Implementation:
//--------------------------------------------------------------------
FINLINE CFVec3A::CFVec3A( void ) {}
FINLINE CFVec3A::CFVec3A( const CFVec3 &rV ) { x = rV.x; y = rV.y; z = rV.z; w = 0.f; }
FINLINE CFVec3A::CFVec3A( const CFVec3A &rV ) { _A = rV._A; _B = rV._B; }

//
//
FINLINE CFVec3A::CFVec3A( const f32 &fVal ) 
{ 
	x = fVal; 
	y = fVal; 
	z = fVal; 
	w = 0.0f; 
}

//
//
FINLINE CFVec3A::CFVec3A( const f32 &fX, const f32 &fY, const f32 &fZ ) 
{ 
	Set( fX, fY, fZ ); 
	w = 0.0f; 
}

//
//
FINLINE CFVec3A::CFVec3A( const f32 *pfArray3 ) 
{ 
	Set( pfArray3 ); 
	w = 0.0f; 
}

//
//
//FINLINE CFVec3A::CFVec3A( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ ) { SetBitMask( nBitMaskX, nBitMaskY, nBitMaskZ ); w=0.0f; }
/*
//
//
FINLINE f32 CFVec3A::GetY( void ) 
{
	__asm {
		mov		eax, this
		movaps	xmm0, [eax]
		shufps	xmm0, xmm0, 1
		movss	FDX8Math_fTemp, xmm0
	}

	return FDX8Math_fTemp;
}
*/

//
//
FINLINE CFVec3A &CFVec3A::Zero( void ) 
{ 
	x = 0; y = 0; z = 0; w = 1;
//	_m = _mm_setzero_ps(); 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::operator = ( const CFVec3A &rV ) 
{ 
	_A = rV._A; 
	_B = rV._B; 
	return *this; 
}

//
//
FINLINE BOOL CFVec3A::operator != ( const CFVec3A &rV ) const
{
	return ( (rV.x != x) || (rV.y != y) || (rV.z != z) );

};

//
//
FINLINE BOOL CFVec3A::operator == ( const CFVec3A &rV ) const
{
	return ( (rV.x == x) && (rV.y == y) && (rV.z == z) );

};

//
//
FINLINE CFVec3A &CFVec3A::Set( const CFVec2 &rV ) 
{ 
	v2 = rV; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Set( const CFVec3 &rV ) 
{ 
	v3 = rV; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Set( const CFVec4 &rV ) 
{ 
	v3 = rV.v3; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Set( const CFVec3A &rV ) 
{ 
	_A = rV._A; 
	_B = rV._B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Set( const CFVec4A &rV ) 
{ 
	_A = rV._A; 
	z = rV.z; 
	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Set( const f32 &fVal )
{ 
	x = fVal; 
	y = fVal; 
	z = fVal; 
	return *this; 
}

//
//
//
FINLINE CFVec3A &CFVec3A::Set( const f32 &fX, const f32 &fY, const f32 &fZ ) 
{ 
	x = fX; 
	y = fY;
	z = fZ;
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Set( const f32 *pfArray3 ) 
{ 
	a[0] = pfArray3[0]; 
	a[1] = pfArray3[1]; 
	a[2] = pfArray3[2]; 
	return *this; 
}

//
//
//FINLINE CFVec3A &CFVec3A::SetBitMask( const u32 &nBitMaskX, const u32 &nBitMaskY, const u32 &nBitMaskZ ) { *((u32 *)&_m.m128_f32[0]) = nBitMaskX; *((u32 *)&_m.m128_f32[1]) = nBitMaskY; *((u32 *)&_m.m128_f32[2]) = nBitMaskZ; return *this; }

//
//
FINLINE CFVec3A &CFVec3A::SetToInverse( const f32 &fVal ) 
{
	f32 fTemp = fmath_Inv( fVal );

	x = fTemp;
	y = fTemp;
	z = fTemp;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveNegative( const CFVec3A &rV )
{
	x = -rV.x;
	y = -rV.y;
	z = -rV.z;
	
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Negate( void )
{ 
	x = -x;
	y = -y;
	z = -z;
	
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Add( register const CFVec3A &rV1, register const CFVec3A &rV2 ) 
{ 
	register CFVec3A *pThis = this;
	register f32 _r0, _r1, _r2, _r3;
	asm
	{
		psq_l	_r0, 0(rV1), 0, 0
		psq_l	_r2, 0(rV2), 0, 0
		
		ps_add 	_r0, _r0, _r2
		psq_st	_r0, 0(pThis), 0, 0
		
		psq_l	_r1, 8(rV1), 0, 0
		psq_l	_r3, 8(rV2), 0, 0
	
		ps_add 	_r1, _r1, _r3
		psq_st	_r1, 8(pThis), 0, 0
	}
	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Add( register const CFVec3A &rV ) 
{ 
	register CFVec3A *pThis = this;
	register f32 _r0, _r1, _r2, _r3;
	asm
	{
		psq_l	_r0, 0(rV), 0, 0
		psq_l	_r2, 0(pThis), 0, 0
		
		ps_add 	_r0, _r0, _r2
		psq_st	_r0, 0(pThis), 0, 0
		
		psq_l	_r1, 8(rV), 0, 0
		psq_l	_r3, 8(pThis), 0, 0
	
		ps_add 	_r1, _r1, _r3
		psq_st	_r1, 8(pThis), 0, 0
	}
	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Add( const CFVec3A &rV, const f32 &fVal ) 
{
	x = rV.x + fVal;
	y = rV.y + fVal;
	z = rV.z + fVal;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Add( const f32 &fVal ) 
{
	x = x + fVal;
	y = y + fVal;
	z = z + fVal;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Sub( const CFVec3A &rV1, const CFVec3A &rV2 ) 
{
	_A = rV1._A - rV2._A; 
	_B = rV1._B - rV2._B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Sub( const CFVec3A &rV ) 
{ 
	_A = _A - rV._A; 
	_B = _B - rV._B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Sub( const CFVec3A &rV, const f32 &fVal ) 
{
	x = rV.x - fVal;
	y = rV.y - fVal;
	z = rV.z - fVal;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Sub( const f32 &fVal ) 
{
	x = x - fVal;
	y = y - fVal;
	z = z - fVal;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RevSub( const f32 &fVal ) 
{
	x = fVal - x;
	y = fVal - y;
	z = fVal - z;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RevSub( const CFVec3A &rV ) 
{
	_A = rV._A - _A; 
	_B = rV._B - _B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Mul( const CFVec3A &rV1, const CFVec3A &rV2 ) 
{
	_A = rV1._A * rV2._A; 
	_B = rV1._B * rV2._B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Mul( const CFVec3A &rV ) 
{ 
	_A = _A * rV._A; 
	_B = _B * rV._B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Mul( const CFVec3A &rV, const f32 &fVal ) 
{
	x = rV.x * fVal;
	y = rV.y * fVal;
	z = rV.z * fVal;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Mul( const f32 &fVal ) 
{
	x = x * fVal;
	y = y * fVal;
	z = z * fVal;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Div( const CFVec3A &rV1, const CFVec3A &rV2 ) 
{
	_A = rV1._A / rV2._A; 
	_B = rV1._B / rV2._B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Div( const CFVec3A &rV ) 
{ 
	_A = _A / rV._A; 
	_B = _B / rV._B; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Div( const CFVec3A &rV, const f32 &fVal ) 
{
	f32 fTemp = 1.f / fVal;
	x = rV.x * fTemp;
	y = rV.y * fTemp;
	z = rV.z * fTemp;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Div( const f32 &fVal ) 
{
	f32 fTemp = 1.f / fVal;
	x = x * fTemp;
	y = y * fTemp;
	z = z * fTemp;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveInverse( const CFVec3A &rV ) 
{ 
	x = 1.f / rV.x;
	y = 1.f / rV.y;
	z = 1.f / rV.z;
	w = 0.f; 
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::Invert( void ) 
{ 
	x = 1.f / x;
	y = 1.f / y;
	z = 1.f / z;
	w = 0.f; 
	return *this; 
}

/*
FINLINE CFVec3A &CFVec3A::Min( const CFVec3A &rV1, const CFVec3A &rV2 ) { _m = _mm_min_ps( rV1._m, rV2._m ); return *this; }
FINLINE CFVec3A &CFVec3A::Max( const CFVec3A &rV1, const CFVec3A &rV2 ) { _m = _mm_max_ps( rV1._m, rV2._m ); return *this; }

FINLINE CFVec3A &CFVec3A::Clamp0( void ) { _m = _mm_max_ps( _m, m_Null._m ); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp1( void ) { _m = _mm_min_ps( _m, m_Ones._m ); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp01( void ) { _m = _mm_max_ps( _mm_min_ps( _m, m_Ones._m ), m_Null._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1( void ) { _m = _mm_max_ps( _m, m_NegOnes._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1Pos1( void ) { _m = _mm_max_ps( _mm_min_ps( _m, m_Ones._m ), m_NegOnes._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMin( const CFVec3A &rMinV ) { _m = _mm_max_ps( _m, rMinV._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMax( const CFVec3A &rMaxV ) { _m = _mm_min_ps( _m, rMaxV._m ); return *this; }
*/

//
//
FINLINE CFVec3A &CFVec3A::ClampMin( const CFVec3A &rMinV ) 
{ 
	if ( x < rMinV.x )
	{
		x = rMinV.x;
	}
	if ( y < rMinV.y )
	{
		y = rMinV.y;
	}
	if ( z < rMinV.z )
	{
		z = rMinV.z;
	}
		
	return *this; 
}

//
//
FINLINE CFVec3A &CFVec3A::ClampMax( const CFVec3A &rMaxV ) 
{ 
	if ( x > rMaxV.x )
	{
		x = rMaxV.x;
	}
	if ( y > rMaxV.y )
	{
		y = rMaxV.y;
	}
	if ( z > rMaxV.z )
	{
		z = rMaxV.z;
	}
		
	return *this; 
}

/*
FINLINE CFVec3A &CFVec3A::Clamp( const CFVec3A &rMinV, const CFVec3A &rMaxV ) { _m = _mm_max_ps( _mm_min_ps( _m, rMaxV._m ), rMinV._m ); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp0( const CFVec3A &rTestV ) { _m = _mm_max_ps( rTestV._m, m_Null._m ); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp1( const CFVec3A &rTestV ) { _m = _mm_min_ps( rTestV._m, m_Ones._m ); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp01( const CFVec3A &rTestV ) { _m = _mm_max_ps( _mm_min_ps( rTestV._m, m_Ones._m ), m_Null._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1( const CFVec3A &rTestV ) { _m = _mm_max_ps( rTestV._m, m_NegOnes._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampNeg1Pos1( const CFVec3A &rTestV ) { _m = _mm_max_ps( _mm_min_ps( rTestV._m, m_Ones._m ), m_NegOnes._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMin( const CFVec3A &rTestV, const CFVec3A &rMinV ) { _m = _mm_max_ps( rTestV._m, rMinV._m ); return *this; }
FINLINE CFVec3A &CFVec3A::ClampMax( const CFVec3A &rTestV, const CFVec3A &rMaxV ) { _m = _mm_min_ps( rTestV._m, rMaxV._m ); return *this; }
FINLINE CFVec3A &CFVec3A::Clamp( const CFVec3A &rTestV, const CFVec3A &rMinV, const CFVec3A &rMaxV ) { _m = _mm_max_ps( _mm_min_ps( rTestV._m, rMaxV._m ), rMinV._m ); return *this; }
*/

//
//
FINLINE f32 CFVec3A::Dist( const CFVec3A &rV ) const 
{
	return m_TempVec.Sub( rV, *this ).Mag();
}

//
//
FINLINE f32 CFVec3A::DistSq( const CFVec3A &rV ) const 
{
	return m_TempVec.Sub( rV, *this ).MagSq();
}

//
//
FINLINE f32 CFVec3A::InvDist( const CFVec3A &rV ) const 
{
	return m_TempVec.Sub( rV, *this ).InvMag();
}


//
//
FINLINE f32 CFVec3A::InvDistSq( const CFVec3A &rV ) const 
{
	return m_TempVec.Sub( rV, *this ).InvMagSq();
}


//
//
FINLINE f32 CFVec3A::DistXZ( const CFVec3A &rV ) const 
{
	return m_TempVec.Sub( rV, *this ).MagXZ();
}

//
//
FINLINE f32 CFVec3A::DistSqXZ( const CFVec3A &rV ) const 
{
	return m_TempVec.Sub( rV, *this ).MagSqXZ();
}

//
//
FINLINE f32 CFVec3A::Mag( void ) const 
{
	return fmath_Sqrt( x*x + y*y + z*z );
}

//
//
FINLINE f32 CFVec3A::MagSq( void ) const 
{
	return x*x + y*y + z*z;
}

//
//
FINLINE f32 CFVec3A::InvMag( void ) const 
{
	FASSERT( x*x + y*y + z*z != 0.f );
	return fmath_InvSqrt( x*x + y*y + z*z );
}

//
//
FINLINE f32 CFVec3A::InvMagSq( void ) const 
{
	FASSERT( x*x + y*y + z*z != 0.f );
	return fmath_Inv( x*x + y*y + z*z );
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveUnit( const CFVec3A &rV ) 
{
	f32 fTemp = rV.Mag();
	
	x = rV.x * fTemp;
	y = rV.y * fTemp;
	z = rV.z * fTemp;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Unitize( void ) 
{
	f32 fTemp = InvMag();
	
	x = x * fTemp;
	y = y * fTemp;
	z = z * fTemp;

	return *this;
}

//
//
FINLINE f32 CFVec3A::UnitAndMag( const CFVec3A &rV ) 
{
	Set( rV );
	f32 fMag = rV.Mag();
	FASSERT( fMag != 0.0f );
	Mul( fmath_Inv(fMag) );
	return fMag;
}

//
//
FINLINE f32 CFVec3A::UnitAndInvMag( const CFVec3A &rV ) 
{
	Set( rV );
	f32 fInvMag = rV.InvMag();
	Mul( fInvMag );
	return fInvMag;
}

//
//
FINLINE f32 CFVec3A::SafeUnitAndMag( const CFVec3A &rV ) 
{
	Set( rV );
	f32 fMag = rV.Mag();
	if ( fMag == 0.0f )
	{
		return -1.f;
	}
	Mul( fmath_Inv( fMag ) );
	return fMag;
}

//
//
FINLINE f32 CFVec3A::SafeUnitAndInvMag( const CFVec3A &rV ) 
{
	Set( rV );
	f32 fTemp = rV.Mag();
	if ( fTemp == 0.0f )
	{
		return -1.f;
	}
	fTemp = fmath_Inv( fTemp );
	Mul( fTemp );
	return fTemp;
}

//
//
FINLINE f32 CFVec3A::MagXZ( void ) const 
{
	return fmath_Sqrt( x*x + z*z );
}

//
//
FINLINE f32 CFVec3A::MagSqXZ( void ) const 
{
	return x*x + z*z;
}

//
//
FINLINE f32 CFVec3A::InvMagXZ( void ) const 
{
	FASSERT( x*x + z*z != 0.f );
	return fmath_InvSqrt( x*x + z*z );
}

//
//
FINLINE f32 CFVec3A::InvMagSqXZ( void ) const 
{
	FASSERT( x*x + z*z != 0.0f );
	return fmath_Inv( x*x + z*z );
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveUnitXZ( const CFVec3A &rV ) 
{
	FASSERT( rV.x*rV.x + rV.z*rV.z != 0.f );
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	x = rV.x * fInvMagXZ;
	y = 0.f;
	z = rV.z * fInvMagXZ;

	return *this;
}


//
//
FINLINE CFVec3A &CFVec3A::UnitizeXZ( void ) 
{
	FASSERT( x*x + z*z != 0.f );
	f32 fInvMagXZ = fmath_InvSqrt( x*x + z*z );

	x *= fInvMagXZ;
	z *= fInvMagXZ;

	return *this;
}

//
//
FINLINE f32 CFVec3A::UnitAndMagXZ( const CFVec3A &rV ) 
{
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	FASSERT( fMagXZ != 0.0f );
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x = rV.x * fInvMagXZ;
	z = rV.z * fInvMagXZ;
	y = 0.f;

	return fMagXZ;
}

//
//
FINLINE f32 CFVec3A::UnitAndInvMagXZ( const CFVec3A &rV ) 
{
	FASSERT( rV.x*rV.x + rV.z*rV.z != 0.f );
	f32 fInvMagXZ = fmath_InvSqrt( rV.x*rV.x + rV.z*rV.z );

	x = rV.x * fInvMagXZ;
	z = rV.z * fInvMagXZ;
	y = 0.f;

	return fInvMagXZ;
}


FINLINE f32 CFVec3A::SafeUnitAndMagXZ( const CFVec3A &rV ) 
{
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	if ( fMagXZ == 0.0f )
	{
		return -1.f;
	}
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x = rV.x * fInvMagXZ;
	z = rV.z * fInvMagXZ;
	y = 0.f;

	return fMagXZ;
}

//
//
FINLINE f32 CFVec3A::SafeUnitAndInvMagXZ( const CFVec3A &rV ) 
{
	f32 fMagXZ = fmath_Sqrt( rV.x*rV.x + rV.z*rV.z );
	if ( fMagXZ == 0.0f )
	{
		return -1.f;
	}
	f32 fInvMagXZ = fmath_Inv( fMagXZ );

	x = rV.x * fInvMagXZ;
	z = rV.z * fInvMagXZ;
	y = 0.f;

	return fInvMagXZ;
}


//
//
FINLINE CFVec3A &CFVec3A::Cross( register const CFVec3A &rV1, register const CFVec3A &rV2 ) 
{
	register f32 fV1xy, fV1zw, fV2xy, fV2zw;
	register f32 fT1, fT2, fT3, fT4;
	register CFVec3A *pThis = this;
	asm
	{
		// BX | BY
		psq_l       fV2xy, 0(rV2), 0, 0
		// AZ | AZ
		lfs         fV1zw, 8(rV1)
		// AX | AY
		psq_l       fV1xy, 0(rV1), 0, 0
		// BY | BX
		ps_merge10  fT1, fV2xy, fV2xy
		// BZ | BZ
		lfs         fV2zw, 8(rV2)

		// BX*AZ | BY*AZ
		ps_mul      fT2, fV2xy, fV1zw
		// BX*AX | BY*AX
		ps_muls0    fT3, fV2xy, fV1xy
		// AX*BZ-BX*AZ | AY*BZ-BY*AZ
		ps_msub     fT2, fV1xy, fV2zw, fT2
		// AX*BY-BX*AX | AY*BX-BY*AX
		ps_msub     fT3, fV1xy, fT1, fT3

		// AY*BZ-AZ*BY | AY*BZ-AZ*BY
		ps_merge11  fT4, fT2, fT2
		// AX*BZ-AZ*BX | AY*BX-AX*BY
		ps_merge01  fT1, fT2, fT3

		psq_st      fT4, 0(pThis), 1, 0

		// AZ*BX-AX*BZ | AX*BY-AY*BX
		ps_neg      fT1, fT1
	
		psq_st      fT1, 4(pThis), 0, 0
	}

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Cross( register const CFVec3A &rV ) 
{
	register f32 fV1xy, fV1zw, fV2xy, fV2zw;
	register f32 fT1, fT2, fT3, fT4;
	register CFVec3A *pThis = this;
	asm
	{
		// BX | BY
		psq_l       fV2xy, 0(rV), 0, 0
		// AZ | AZ
		lfs         fV1zw, 8(pThis)
		// AX | AY
		psq_l       fV1xy, 0(pThis), 0, 0
		// BY | BX
		ps_merge10  fT1, fV2xy, fV2xy
		// BZ | BZ
		lfs         fV2zw, 8(rV)

		// BX*AZ | BY*AZ
		ps_mul      fT2, fV2xy, fV1zw
		// BX*AX | BY*AX
		ps_muls0    fT3, fV2xy, fV1xy
		// AX*BZ-BX*AZ | AY*BZ-BY*AZ
		ps_msub     fT2, fV1xy, fV2zw, fT2
		// AX*BY-BX*AX | AY*BX-BY*AX
		ps_msub     fT3, fV1xy, fT1, fT3

		// AY*BZ-AZ*BY | AY*BZ-AZ*BY
		ps_merge11  fT4, fT2, fT2
		// AX*BZ-AZ*BX | AY*BX-AX*BY
		ps_merge01  fT1, fT2, fT3

		psq_st      fT4, 0(pThis), 1, 0

		// AZ*BX-AX*BZ | AX*BY-AY*BX
		ps_neg      fT1, fT1
	
		psq_st      fT1, 4(pThis), 0, 0
	}

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::UnitCross( const CFVec3A &rV1, const CFVec3A &rV2 ) 
{
	return Cross( rV1, rV2 ).Unitize();
}

//
//
FINLINE CFVec3A &CFVec3A::UnitCross( const CFVec3A &rV ) 
{
	return Cross( rV ).Unitize();
}

//
//
FINLINE CFVec3A &CFVec3A::CrossVecWithY( const CFVec3A &rV ) 
{
	x = -rV.z;
	y = 0.0f;
	z = rV.x;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::CrossYWithVec( const CFVec3A &rV ) 
{
	x = rV.z;
	y = 0.0f;
	z = -rV.x;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::UnitCrossVecWithY( const CFVec3A &rV ) 
{
	return CrossVecWithY( rV ).UnitizeXZ();
}

//
//
FINLINE CFVec3A &CFVec3A::UnitCrossYWithVec( const CFVec3A &rV ) 
{
	return CrossYWithVec( rV ).UnitizeXZ();
}

//
//
FINLINE f32 CFVec3A::Dot( register const CFVec3A &rV1 ) const 
{ 
	return (x * rV1.x) + (y * rV1.y) + (z * rV1.z);
}

//
//
FINLINE CFVec3A &CFVec3A::Lerp( const f32 &fUnitVal, const CFVec3A &rV1, const CFVec3A &rV2 ) 
{
	x = rV1.x + ((rV2.x - rV1.x) * fUnitVal);
	y = rV1.y + ((rV2.y - rV1.y) * fUnitVal);
	z = rV1.z + ((rV2.z - rV1.z) * fUnitVal);
	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::Lerp( const f32 &fUnitVal, const CFVec3A &rV ) 
{
	x = x + ((rV.x - x) * fUnitVal);
	y = y + ((rV.y - y) * fUnitVal);
	z = z + ((rV.z - z) * fUnitVal);
	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RotateX( const f32 &fRadians ) 
{
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RotateY( const f32 &fRadians ) 
{
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RotateZ( const f32 &fRadians ) 
{
	f32 fSin, fCos, fTemp10, fTemp20;

	fmath_SinCos( fRadians, &fSin, &fCos );

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RotateX( const f32 &fSin, const f32 &fCos ) 
{
	f32 fTemp10, fTemp20;

	fTemp10 = y*fCos - z*fSin;
	fTemp20 = y*fSin + z*fCos;

	y = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RotateY( const f32 &fSin, const f32 &fCos ) 
{
	f32 fTemp10, fTemp20;

	fTemp10 = z*fSin + x*fCos;
	fTemp20 = z*fCos - x*fSin;

	x = fTemp10;
	z = fTemp20;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::RotateZ( const f32 &fSin, const f32 &fCos ) 
{
	f32 fTemp10, fTemp20;

	fTemp10 = x*fCos - y*fSin;
	fTemp20 = x*fSin + y*fCos;

	x = fTemp10;
	y = fTemp20;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveRotationX( const CFVec3A &rV, const f32 &fRadians ) 
{
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveRotationY( const CFVec3A &rV, const f32 &fRadians ) 
{
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveRotationZ( const CFVec3A &rV, const f32 &fRadians ) 
{
	f32 fSin, fCos;

	fmath_SinCos( fRadians, &fSin, &fCos );

	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveRotationX( const CFVec3A &rV, const f32 &fSin, const f32 &fCos ) 
{
	x = rV.x;
	y = rV.y*fCos - rV.z*fSin;
	z = rV.y*fSin + rV.z*fCos;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveRotationY( const CFVec3A &rV, const f32 &fSin, const f32 &fCos ) 
{
	x = rV.z*fSin + rV.x*fCos;
	y = rV.y;
	z = rV.z*fCos - rV.x*fSin;

	return *this;
}

//
//
FINLINE CFVec3A &CFVec3A::ReceiveRotationZ( const CFVec3A &rV, const f32 &fSin, const f32 &fCos ) 
{
	x = rV.x*fCos - rV.y*fSin;
	y = rV.x*fSin + rV.y*fCos;
	z = rV.z;

	return *this;
}

// Reflects a vector across a normal. For example, if
// a ray intersects a poly with a normal of rNormal, 
// you can Reflect the normalized ray direction vector using 
// this function and the poly normal as the reflection normal.
// The result is the new normalized direction vector for the 
// ray reflection, and is pointing the opposite direction as
// the original ray. This function will work with non-normalized 
// vectors, as well.
FINLINE CFVec3A &CFVec3A::Reflect( const CFVec3A &rNormal ) 
{
	CFVec3A vTemp;
	vTemp.Mul( rNormal, -2.0f * Dot(rNormal) );
	Add( vTemp );
	return *this;
}

// See Reflect(), above.
FINLINE CFVec3A &CFVec3A::ReceiveReflection( const CFVec3A &rV, const CFVec3A &rNormal ) 
{
	Mul( rNormal, -2.0f * rV.Dot(rNormal) ).Add( rV );
	return *this;
}


//
//
FINLINE u32 CFVec3A::GenKey( void ) const 
{
	return (*(u32 *)&x) ^ (*(u32 *)&y) ^ (*(u32 *)&z);
}

//
// project this vector onto plane defined by unit vector rvNormal
FINLINE CFVec3A &CFVec3A::PlanarProjection( const CFVec3A &rvNormal ) 
{
	CFVec3A vTemp;

	// this = this - (rvNormal * (rvNormal dot this));
	vTemp = rvNormal;
	vTemp.Mul( rvNormal.Dot( *this ) );
	Sub( vTemp );

	return *this;
}

//
// project vector rV onto plane defined by unit vector rvNormal
FINLINE CFVec3A &CFVec3A::ReceivePlanarProjection( const CFVec3A &rV, const CFVec3A &rvNormal ) 
{
	// this = rV - (rvNormal * (rvNormal dot rV));
	Set( rvNormal );
	Mul( rvNormal.Dot( rV ) );
	Negate();
	Add( rV );

	return *this;
}

// tests vector to see if it is within pitch and yaw range of center and up vectors.
// vCenter = normalized "origin" look vector
// vUp = normalized "origin" up vector
// fYawCos = cosine of yaw range to either side of vCenter
// fPitchCos = cosine of pitch range to either side of vUp
// returns TRUE if in range, FALSE otherwise.
FINLINE BOOL CFVec3A::InYawPitchRange( const CFVec3A &vCenter, const CFVec3A &vUp, const f32 fYawCos, const f32 fPitchCos ) 
{
	CFVec3A vProjected;

	vProjected.ReceivePlanarProjection( *this, vUp );

	// check yaw constraint
	if ( vCenter.Dot( vProjected ) < fYawCos )
	{
		return FALSE;
	}

	// check pitch constraint
	if ( Dot( vProjected ) < fPitchCos )
	{
		return FALSE;
	}

	return TRUE;
}


