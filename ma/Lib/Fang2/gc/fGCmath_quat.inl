//////////////////////////////////////////////////////////////////////////////////////
// fGCmath_quat.inl - Fang quaternion library.
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


//static const CFVec4A _nnnnNegMaskW( (f32)0x00000000, (f32)0x00000000, (f32)0x00000000, (f32)0x80000000 );
//static const CFVec4A _nnnnZeroMaskW( (f32)0xffffffff, (f32)0xffffffff, (f32)0xffffffff, (f32)0x00000000 );

#define _CHECK_MATH		FALSE

#if FANG_DEBUG_BUILD
#pragma optimization_level 1 
#pragma global_optimizer on
#endif

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

//
//
FINLINE CFQuat &CFQuat::BuildQuat( const CFVec3 &rUnitVecToRotateAbout, f32 fRadiansToRotateBy ) 
{
	if( FMATH_IS_CLOSE_TO_ZERO( fRadiansToRotateBy ) ) 
	{
		Set( 0.0f, 0.0f, 0.0f, 1.0f );
	} 
	else 
	{
		f32 fSin;

		fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

		x = fSin * rUnitVecToRotateAbout.x;
		y = fSin * rUnitVecToRotateAbout.y;
		z = fSin * rUnitVecToRotateAbout.z;
	}

	return *this;
}

//
//
FINLINE CFQuat CFQuat::operator * ( const CFQuat &rQuat ) const 
{
	CFQuat DestQuat;

	DestQuat.a[3] = a[3]*rQuat.a[3] - v3.Dot( rQuat.v3 );
	DestQuat.v3 = v3.Cross( rQuat.v3 ) + rQuat.v3*a[3] + v3*rQuat.a[3];

	return DestQuat;
}

//
//
FINLINE CFQuat &CFQuat::Invert( void ) 
{
	f32 fOOQTimesConjQ;

	FASSERT( a[3]*a[3] + a[0]*a[0] + a[1]*a[1] + a[2]*a[2] != 0.f );
	fOOQTimesConjQ = fmath_Inv( a[3]*a[3] + a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );

	v3 *= -fOOQTimesConjQ;
	a[3] *= fOOQTimesConjQ;

	return *this;
}
#endif



//--------------------------------------------------------------------
// CFQuatA Implementation:
//--------------------------------------------------------------------
FINLINE CFQuatA::CFQuatA( void ) {}
FINLINE CFQuatA::CFQuatA( const CFQuatA &rQ ) { Set( rQ ); }
FINLINE CFQuatA::CFQuatA( const CFVec4A &rV ) { Set( rV ); }
FINLINE CFQuatA::CFQuatA( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { Set( fX, fY, fZ, fW ); }

FINLINE CFQuatA &CFQuatA::Zero( void ) { v.Zero(); return *this; }
FINLINE CFQuatA &CFQuatA::Identity( void ) { v.ZeroW1(); return *this; }

FINLINE CFQuatA &CFQuatA::operator = ( const CFQuatA &rQ ) { v = rQ.v; return *this; }
FINLINE CFQuatA &CFQuatA::Set( const CFQuatA &rQ ) { v = rQ.v; return *this; }
FINLINE CFQuatA &CFQuatA::Set( const CFVec3A &rV ) { v = rV.v4a; v.w = 0.f; return *this; }	// Important: This must set the quat's W to 0
FINLINE CFQuatA &CFQuatA::Set( const CFVec4A &rV ) { v = rV; return *this; }
FINLINE CFQuatA &CFQuatA::Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW ) { v.Set( fX, fY, fZ, fW );  return *this; }

//
//
FINLINE CFQuatA &CFQuatA::BuildQuat( const FPackedQuat_t &rPackedQuat ) 
{
	x = (f32)rPackedQuat.x * (1.0f/127.0f);
	y = (f32)rPackedQuat.y * (1.0f/127.0f);
	z = (f32)rPackedQuat.z * (1.0f/127.0f);
	w = (f32)rPackedQuat.w * (1.0f/127.0f);

	return *this;
}

//
//
FINLINE CFQuatA &CFQuatA::BuildQuat( const CFMtx43 &rMtx ) 
{ 
	return BuildQuat( rMtx.m33 ); 
}

//
//
FINLINE CFQuatA &CFQuatA::BuildQuat( const CFVec4A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy ) 
{
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = fSin * rUnitVecToRotateAbout.x;
	y = fSin * rUnitVecToRotateAbout.y;
	z = fSin * rUnitVecToRotateAbout.z;

	return *this;
}

//
//
FINLINE CFQuatA &CFQuatA::BuildQuat( const CFVec3A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy ) 
{
	return BuildQuat( rUnitVecToRotateAbout.v4a, fRadiansToRotateBy );
}

//
//
FINLINE CFQuatA &CFQuatA::BuildQuat( const CFVec3A &rUnitVecToRotateFrom, const CFVec3A &rUnitVecToRotateTo ) 
{
	CFVec3A RotationUnitAxis;
	f32 fRotationAxisMag, fCosAngleBetweenVecs, fHalfSin;

	fCosAngleBetweenVecs = rUnitVecToRotateFrom.Dot( rUnitVecToRotateTo );

	if ( fCosAngleBetweenVecs < -0.9999f )
	{
		w = 0.f;
	}
	else
	{
		w = fmath_Sqrt( (1.0f + fCosAngleBetweenVecs) * 0.5f );
	}

	fCosAngleBetweenVecs = 1.0f - fCosAngleBetweenVecs;
	if( fCosAngleBetweenVecs > 0.00001f ) 
	{
		fHalfSin = fmath_Sqrt( fCosAngleBetweenVecs * 0.5f );

		RotationUnitAxis.Cross( rUnitVecToRotateFrom, rUnitVecToRotateTo );
		fRotationAxisMag = RotationUnitAxis.Mag();

		if( fRotationAxisMag > 0.00001f ) 
		{
			RotationUnitAxis.Div( fRotationAxisMag );
			v.v3 = RotationUnitAxis.v3 * fHalfSin;
		} 
		else 
		{
			v.v3.Zero();
		}
	} 
	else 
	{
		v.v3.Zero();
	}

	FMATH_CLASS_DEBUG_FCHECK( *this );

	return *this;
}

//
//
FINLINE CFQuatA &CFQuatA::BuildQuatRotX( const f32 &fRadiansToRotateBy ) 
{
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = fSin;
	y = 0.0f;
	z = 0.0f;

	return *this;
}

//
//
FINLINE CFQuatA &CFQuatA::BuildQuatRotY( const f32 &fRadiansToRotateBy ) 
{
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = 0.0f;
	y = fSin;
	z = 0.0f;

	return *this;
}

//
//
FINLINE CFQuatA &CFQuatA::BuildQuatRotZ( const f32 &fRadiansToRotateBy ) 
{
	f32 fSin;

	fmath_SinCos( 0.5f*fRadiansToRotateBy, &fSin, &w );

	x = 0.0f;
	y = 0.0f;
	z = fSin;

	return *this;
}

//
//
FINLINE CFQuatA &CFQuatA::ReceiveNegative( const CFQuatA &rQ ) 
{ 
	v.Mul( rQ.v, FGCMath_NegOnesW1 ); 
	return *this; 
}

//
//
FINLINE CFQuatA &CFQuatA::Negate( void ) 
{ 
	v.Mul( FGCMath_NegOnesW1 ); 
	return *this; 
}

//
//
FINLINE CFQuatA &CFQuatA::Mul( register const CFQuatA &rQ1, register const CFQuatA &rQ2 ) 
{
	register CFQuatA *pThis = this;
	register f32 pxy, pzw, qxy, qzw;
	register f32 pnxy, pnzw, pnxny, pnznw;
	register f32 rxy, rzw, sxy, szw;

	asm
    {
		psq_l       pxy, 0(rQ1), 0, 0			// [px][py] : Load
		psq_l       pzw, 8(rQ1), 0, 0			// [pz][pw] : Load

		psq_l       qxy, 0(rQ2), 0, 0			// [qx][qy] : Load
		ps_neg      pnxny, pxy					// [-px][-py]
		psq_l       qzw, 8(rQ2), 0, 0			// [qz][qw] : Load
		ps_neg      pnznw, pzw					// [-pz][-pw]

		ps_merge01  pnxy, pnxny, pxy			// [-px][py]

		ps_muls0    rxy, pzw, qxy				// [pz*qx][pw*qx]
		ps_muls0    rzw, pnxny, qxy				// [-px*qx][-py*qx]

		ps_merge01  pnzw, pnznw, pzw			// [-pz][pw]

		ps_muls1    szw, pnxy, qxy				// [-px*qy][py*qy]
		ps_madds0   rxy, pnxy, qzw, rxy			// [pz*qx-px*qz][pw*qx+py*qz]
		ps_muls1    sxy, pnzw, qxy				// [-pz*qy][pw*qy]
		ps_madds0   rzw, pnzw, qzw, rzw			// [-px*qx-pz*qz][-py*qx+pw*qz]
		ps_madds1   szw, pnznw, qzw, szw		// [-px*qy-pz*qw][py*qy-pw*qw]
		ps_merge10  rxy, rxy, rxy				// [pw*qx+py*qz][pz*qx-px*qz]
		ps_madds1   sxy, pxy, qzw, sxy			// [-pz*qy+px*qw][pw*qy+py*qw]
		ps_merge10  rzw, rzw, rzw				// [-py*qx+pw*qz][-px*qx-pz*qz]

		ps_add      rxy, rxy, sxy				// [pw*qx+py*qz-pz*qy+px*qw][pz*qx-px*qz+pw*qy+py*qw] : [pqx][pqy]
		psq_st      rxy, 0(pThis), 0, 0			// [pqx][pqy] : Store
		ps_sub      rzw, rzw, szw				// [-py*qx+pw*qz+px*qy+pz*qw][-px*qx-pz*qz-py*qy+pw*qw] : [pqz][pqw]
		psq_st      rzw, 8(pThis), 0, 0			// [pqz][pqw] : Store
	}
	
#if _CHECK_MATH
	CFQuat qVerify, q1, q2;
	q1.x = rQ1.x;
	q1.y = rQ1.y;
	q1.z = rQ1.z;
	q1.w = rQ1.w;
	q2.x = rQ2.x;
	q2.y = rQ2.y;
	q2.z = rQ2.z;
	q2.w = rQ2.w;

	qVerify = q1 * q2;

	FASSERT( qVerify.x == x && qVerify.y == y && qVerify.z == z && qVerify.w == w );
#endif
    return *this;
}

//
//
FINLINE CFQuatA &CFQuatA::Mul( const CFQuatA &rQ ) 
{
	return Mul( *this, rQ );
}

//
//
FINLINE CFVec3A &CFQuatA::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const 
{ 
	MulPoint( rRV.v4a, rV.v4a ); 
	return rRV; 
}

//
//
FINLINE CFVec4A &CFQuatA::MulPoint( register CFVec4A &rRV, register const CFVec4A &rV ) const 
{
#if 1
	CFQuatA qT, q2( rV );
	q2.w = 0.f;

#if 0	
	qT.x = (w * q2.x) + (x * q2.w) + ( y * q2.z ) - (z * q2.y );
	qT.y = (w * q2.y) + (y * q2.w) + ( z * q2.x ) - (x * q2.z );
	qT.z = (w * q2.z) + (z * q2.w) + ( x * q2.y ) - (y * q2.x );
	qT.w = (w * q2.w) - (x * q2.x) - ( y * q2.y ) - (z * q2.z );
#else	
	qT.x = (w * q2.x) + ( y * q2.z ) - (z * q2.y );
	qT.y = (w * q2.y) + ( z * q2.x ) - (x * q2.z );
	qT.z = (w * q2.z) + ( x * q2.y ) - (y * q2.x );
	qT.w = 0.f - (x * q2.x) - ( y * q2.y ) - (z * q2.z );
#endif	
	
	CFQuatA qI;
	qI.x = -x;
	qI.y = -y;
	qI.z = -z;
	qI.w = w;
	
	rRV.x = (qT.w * qI.x) + (qT.x * qI.w) + ( qT.y * qI.z ) - (qT.z * qI.y );
	rRV.y = (qT.w * qI.y) + (qT.y * qI.w) + ( qT.z * qI.x ) - (qT.x * qI.z );
	rRV.z = (qT.w * qI.z) + (qT.z * qI.w) + ( qT.x * qI.y ) - (qT.y * qI.x );
	rRV.w = 0;

	return rRV;
	
#elif 0
	CFVec4A vTemp;
	CFVec4A vTemp2( v );
	
	// w1 * v2
	vTemp.Mul( rV, w );
	// w2 * v1 
	// Thrown out since w2 = 0;
	// v1 x v2
	vTemp2.v3.Cross( rV.v3 );
	// p = (w2 * v1) + (w1 * v2) + (v1 x v2)
	vTemp.Add( vTemp2 );
	// w = (w1 * w2) - (v1.v2)
	vTemp.w = -v.v3.Dot( rV.v3 );
	
	// q'
	CFQuatA qTemp3;
	qTemp3.x = -x;
	qTemp3.y = -y;
	qTemp3.z = -z;
	qTemp3.w = w;
	
	// w1 * v2
	rRV.Mul( qTemp3.v, vTemp.w );
	// w2 * v1
	vTemp2.Mul( vTemp, qTemp3.w );
	// v1 x v2
	vTemp.v3.Cross( qTemp3.v.v3 );
	// p = (w2 * v1) + (w1 * v2) + (v1 x v2)
	rRV.Add( vTemp ).Add( vTemp2 );
	rRV.w = 0;
	return rRV;
#elif 0
	CFQuatA qTemp1( rV );
	qTemp1.w = 0.f;
	CFQuatA qTemp2;
	qTemp2.Mul( *this, qTemp1 );
	CFQuatA qTemp3;
	qTemp3.x = -x;
	qTemp3.y = -y;
	qTemp3.z = -z;
	qTemp3.w = w;
	qTemp2.Mul( qTemp3 );
	rRV.Set( qTemp2.v );
	return rRV;
#else
	register const CFQuatA *pThis = this;
	register f32 v2xy, v2zw;
	register f32 q1xy, q1zw;

	register f32 q1zx, v2yz;
	
	register f32 A, B, C, D;
	register f32 Axy, Azw;
	register f32 temp1, temp2, temp3, temp4, temp5, temp6;

    asm
    {
    	// Calculation of A B C D
		psq_l       q1xy, 0(pThis), 0, 0
		psq_l       v2xy, 0(rV), 0, 0
		
		psq_l       v2zw, 8(rV), 0, 0
		psq_l       q1zw, 8(pThis), 0, 0
		
			ps_mul		Axy, q1xy, v2xy
			ps_mul		Azw, q1zw, v2zw
		
		ps_muls1	temp1, v2xy, q1zw
		ps_muls1	temp2, v2zw, q1zw
		
			ps_sum1		A, Axy, Azw, Axy
			ps_sum0		A, A, A, A
		
		ps_merge00	q1zx, q1zw, q1xy
		ps_merge10	v2yz, v2xy, v2zw
		
		ps_mul		temp3, q1zx, v2xy
		ps_mul		temp4, q1xy, v2yz
		
		ps_mul		temp5, q1zx, v2yz
		ps_muls0	temp6, q1xy, v2xy

		ps_neg		temp5, temp5
		ps_neg		temp6, temp6
		
		ps_sum1		B, temp1, temp5, temp4
		ps_sum0		B, B, B, B
		
		ps_sum0		C, temp3, temp5, temp1
		ps_sum0		C, C, C, C
		
		ps_sum0		D, temp2, temp3, temp6
		ps_sum0		D, D, D, D

		// Application of A B C D

		ps_muls0	temp1, q1xy, A
		ps_muls0	temp2, q1zw, A

		ps_mul		temp3, q1zw, B
		
			ps_neg		temp4, q1xy
			
		ps_sum0		temp1, temp1, temp1, temp3
		
			ps_merge10	temp4, q1xy, temp4
		
		ps_sum1		temp1, temp3, temp1, temp1
	
			ps_neg		temp6, q1zw
		
		ps_madds0	temp1, temp4, D, temp1
				
			ps_merge01	temp6, temp6, q1zw
			
		ps_madds0	temp1, q1zw, C, temp1
		
			ps_madds1	temp2, D, q1zw, temp2
		
		psq_st		temp1, 0(rRV), 0, 0
		
			ps_madd		temp2, C, q1xy, temp2
					
			ps_merge10	temp5, temp4, temp4
			ps_madd		temp2, B, temp5, temp2
			
			psq_st		v2zw, 8(rRV), 0, 0
			psq_st		temp2, 8(rRV), 1, 0
    }
    
	return rRV;
#endif	
}

//
//
FINLINE CFVec3A &CFQuatA::MulPoint( CFVec3A &rV ) const 
{ 
	MulPoint( rV.v4a ); 
	return rV; 
}

//
//
FINLINE CFVec4A &CFQuatA::MulPoint( register CFVec4A &rV ) const 
{
#if 1
	CFQuatA qT, q2( rV );
	q2.w = 0.f;

#if 0	
	qT.x = (w * q2.x) + (x * q2.w) + ( y * q2.z ) - (z * q2.y );
	qT.y = (w * q2.y) + (y * q2.w) + ( z * q2.x ) - (x * q2.z );
	qT.z = (w * q2.z) + (z * q2.w) + ( x * q2.y ) - (y * q2.x );
	qT.w = (w * q2.w) - (x * q2.x) - ( y * q2.y ) - (z * q2.z );
#else	
	qT.x = (w * q2.x) + ( y * q2.z ) - (z * q2.y );
	qT.y = (w * q2.y) + ( z * q2.x ) - (x * q2.z );
	qT.z = (w * q2.z) + ( x * q2.y ) - (y * q2.x );
	qT.w = 0.f - (x * q2.x) - ( y * q2.y ) - (z * q2.z );
#endif	
	
	CFQuatA qI;
	qI.x = -x;
	qI.y = -y;
	qI.z = -z;
	qI.w = w;
	
	rV.x = (qT.w * qI.x) + (qT.x * qI.w) + ( qT.y * qI.z ) - (qT.z * qI.y );
	rV.y = (qT.w * qI.y) + (qT.y * qI.w) + ( qT.z * qI.x ) - (qT.x * qI.z );
	rV.z = (qT.w * qI.z) + (qT.z * qI.w) + ( qT.x * qI.y ) - (qT.y * qI.x );
	rV.w = 0;

	return rV;
	
#else
	register const CFQuatA *pThis = this;
	register f32 v2xy, v2zw;
	register f32 q1xy, q1zw;

	register f32 q1zx, v2yz;
	
	register f32 A, B, C, D;
	register f32 Axy, Azw;
	register f32 temp1, temp2, temp3, temp4, temp5, temp6;

    asm
    {
    	// Calculation of A B C D
		psq_l       q1xy, 0(pThis), 0, 0
		psq_l       v2xy, 0(rV), 0, 0
		
		psq_l       v2zw, 8(rV), 0, 0
		psq_l       q1zw, 8(pThis), 0, 0
		
			ps_mul		Axy, q1xy, v2xy
			ps_mul		Azw, q1zw, v2zw
		
		ps_muls1	temp1, v2xy, q1zw
		ps_muls1	temp2, v2zw, q1zw
		
			ps_sum1		A, Axy, Azw, Axy
			ps_sum0		A, A, A, A
		
		ps_merge00	q1zx, q1zw, q1xy
		ps_merge10	v2yz, v2xy, v2zw
		
		ps_mul		temp3, q1zx, v2xy
		ps_mul		temp4, q1xy, v2yz
		
		ps_mul		temp5, q1zx, v2yz
		ps_muls0	temp6, q1xy, v2xy

		ps_neg		temp5, temp5
		ps_neg		temp6, temp6
		
		ps_sum1		B, temp1, temp5, temp4
		ps_sum0		B, B, B, B
		
		ps_sum0		C, temp3, temp5, temp1
		ps_sum0		C, C, C, C
		
		ps_sum0		D, temp2, temp3, temp6
		ps_sum0		D, D, D, D

		// Application of A B C D

		ps_muls0	temp1, q1xy, A
		ps_muls0	temp2, q1zw, A

		ps_mul		temp3, q1zw, B
		
			ps_neg		temp4, q1xy
			
		ps_sum0		temp1, temp1, temp1, temp3
		
			ps_merge10	temp4, q1xy, temp4
		
		ps_sum1		temp1, temp3, temp1, temp1
	
			ps_neg		temp6, q1zw
		
		ps_madds0	temp1, temp4, D, temp1
				
			ps_merge01	temp6, temp6, q1zw
			
		ps_madds0	temp1, q1zw, C, temp1
		
			ps_madds1	temp2, D, q1zw, temp2
		
		psq_st		temp1, 0(rV), 0, 0
		
			ps_madd		temp2, C, q1xy, temp2
					
			ps_merge10	temp5, temp4, temp4
			ps_madd		temp2, B, temp5, temp2
			
			psq_st		v2zw, 8(rV), 0, 0
			psq_st		temp2, 8(rV), 1, 0
    }
    
	return rV;
#endif
}

//
//
FINLINE CFQuatA &CFQuatA::ReceiveInverse( const CFQuatA &rQ ) 
{ 
	v.Mul( rQ.v, FGCMath_NegOnesW1 ); 
	return *this; 
}

//
//
FINLINE CFQuatA &CFQuatA::Invert( void ) 
{ 
	v.Mul( FGCMath_NegOnesW1 ); 
	return *this; 
}

/*
FINLINE CFQuatA &CFQuatA::NonUnitInv( const CFQuatA &rQ ) 
{
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

	return *this;
}


FINLINE CFQuatA &CFQuatA::NonUnitInv( void ) 
{
	f32 fMag2 = x*x + y*y + z*z + w*w;

	__asm {
		rcpss	xmm0, fMag2
		shufps	xmm0, xmm0, 00h
		mulps	xmm0, FDX8Math_NegOnesW1
		mov		eax, this
		mulps	xmm0, [eax]
		movaps	[eax], xmm0
	}

	return *this;
}
*/

//
//
FINLINE CFQuatA &CFQuatA::ReceiveUnit( const CFQuatA &rQ ) 
{ 
	v.ReceiveUnit( rQ.v );
	return *this; 
}

//
//
FINLINE CFQuatA &CFQuatA::Unitize( void ) 
{ 
	v.Unitize(); 
	return *this; 
}

//
//
FINLINE CFMtx33 &CFQuatA::BuildMtx( CFMtx33 &rDestMtx ) const 
{
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

//
//
FINLINE CFMtx33 &CFQuatA::BuildMtx( CFMtx33 &rDestMtx, const f32 &fScale ) const 
{
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

//
//
FINLINE CFMtx43A &CFQuatA::BuildMtx( CFMtx43A &rDestMtx ) const 
{
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

//
//
FINLINE CFMtx43A &CFQuatA::BuildMtx( CFMtx43A &rDestMtx, const f32 &fScale ) const 
{
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

//
//
FINLINE CFMtx43A &CFQuatA::BuildMtx33( CFMtx43A &rDestMtx33 ) const 
{
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

//
//
FINLINE CFMtx43A &CFQuatA::BuildMtx33( CFMtx43A &rDestMtx33, const f32 &fScale ) const 
{
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

//
//
FINLINE CFVec3A &CFQuatA::BuildAxisX( CFVec3A &rDestVec ) const 
{
	rDestVec.a[0] = 1.0f - 2.0f * (a[1]*a[1] + a[2]*a[2]);
	rDestVec.a[1] = 2.0f * (a[0]*a[1] + a[3]*a[2]);
	rDestVec.a[2] = 2.0f * (a[0]*a[2] - a[3]*a[1]);

	return rDestVec;
}

//
//
FINLINE CFVec3A &CFQuatA::BuildAxisY( CFVec3A &rDestVec ) const 
{
	rDestVec.a[0] = 2.0f * (a[0]*a[1] - a[3]*a[2]);
	rDestVec.a[1] = 1.0f - 2.0f * (a[0]*a[0] + a[2]*a[2]);
	rDestVec.a[2] = 2.0f * (a[1]*a[2] + a[3]*a[0]);

	return rDestVec;
}

//
//
FINLINE CFVec3A &CFQuatA::BuildAxisZ( CFVec3A &rDestVec ) const 
{
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

FINLINE CFTQuatA::CFTQuatA( const f32 &fQX, const f32 &fQY, const f32 &fQZ, const f32 &fQW, const f32 &fPX, const f32 &fPY, const f32 &fPZ, const f32 &fScale ) 
{
	Set( fQX, fQY, fQZ, fQW, fPX, fPY, fPZ, fScale );
}

FINLINE CFTQuatA &CFTQuatA::Zero( void ) { CFQuatA::Zero(); m_PosScale.Zero(); return *this; }
FINLINE CFTQuatA &CFTQuatA::Identity( void ) { CFQuatA::Identity(); m_PosScale=CFVec4A::m_UnitAxisW; return *this; }

FINLINE CFTQuatA &CFTQuatA::operator = ( const CFTQuatA &rTQ ) { v=rTQ.v; m_PosScale=rTQ.m_PosScale; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFTQuatA &rTQ ) { v=rTQ.v; m_PosScale=rTQ.m_PosScale; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFQuatA &rQ ) { v=rQ.v; m_PosScale=CFVec4A::m_UnitAxisW; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFQuatA &rQ, const CFVec3A &rPos, const f32 &fScale ) { v=rQ.v; m_PosScale=rPos.v4a; m_fScale=fScale; return *this; }
FINLINE CFTQuatA &CFTQuatA::Set( const CFVec4A &rV, const CFVec3A &rPos, const f32 &fScale ) { v=rV; m_PosScale=rPos.v4a; m_fScale=fScale; return *this; }

//
//
FINLINE CFTQuatA &CFTQuatA::Set( const f32 &fQX, const f32 &fQY, const f32 &fQZ, const f32 &fQW, const f32 &fPX, const f32 &fPY, const f32 &fPZ, const f32 &fScale ) 
{
	v.Set( fQX, fQY, fQZ, fQW );
	m_Pos.Set( fPX, fPY, fPZ );
	m_fScale = fScale;

	return *this;
}

//
//
FINLINE CFTQuatA &CFTQuatA::BuildQuat( const CFMtx33 &rSrcMtx, const CFVec3A &rPos, BOOL bMtxHasNonUnitScale ) {
	m_Pos = rPos.v3;

	if( !bMtxHasNonUnitScale ) {
		m_fScale = 1.0f;
		CFQuatA::BuildQuat( rSrcMtx );
	} else {
		CFMtx33 UnitMtx;
		m_fScale = rSrcMtx.m_vRight.Mag();
		FASSERT( m_fScale != 0.0f );
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
		if ( m_fScale == 0.f )
		{
			u32 i;
			for ( i = 0; i < 4; i++ )
			{
				DEVPRINTF( "%f, %f, %f, %f\n", rSrcMtx.aa[i][0], rSrcMtx.aa[i][1], rSrcMtx.aa[i][2], rSrcMtx.aa[i][3] );
			}
		}
		FASSERT( m_fScale != 0.0f );
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
		FASSERT( m_fScale != 0.f );
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
		FASSERT( m_fScale != 0.0f );
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
		FASSERT( m_fScale != 0.0f );
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

//
//
FINLINE CFTQuatA &CFTQuatA::Mul( register const CFTQuatA &rQ1, register const CFTQuatA &rQ2 ) 
{
	CFQuatA::Mul( rQ1, rQ2 );
	
	register f32 v1xy, v1zw, v2xy, v2zw;
	register f32 q1xy, q1zw;

	register f32 q1zx, v2yz;
	
	register f32 A, B, C, D;
	register f32 Axy, Azw;
	register f32 temp1, temp2, temp3, temp4, temp5, temp6;
	register CFVec4A *pPosScale = &m_PosScale;

    asm
    {
    	// Calculation of A B C D
		psq_l       q1xy, 0(rQ1), 0, 0
		psq_l       v2xy, 16(rQ2), 0, 0
		
		psq_l       v2zw, 24(rQ2), 0, 0
		psq_l       q1zw, 8(rQ1), 0, 0
		
			ps_mul		Axy, q1xy, v2xy
			ps_mul		Azw, q1zw, v2zw
		
		ps_muls1	temp1, v2xy, q1zw
		ps_muls1	temp2, v2zw, q1zw
		
			ps_sum1		A, Axy, Azw, Axy
			ps_sum0		A, A, A, A
		
		ps_merge00	q1zx, q1zw, q1xy
		ps_merge10	v2yz, v2xy, v2zw
		
		ps_mul		temp3, q1zx, v2xy
		ps_mul		temp4, q1xy, v2yz
		
		ps_mul		temp5, q1zx, v2yz
		ps_muls0	temp6, q1xy, v2xy

		ps_neg		temp5, temp5
		ps_neg		temp6, temp6
		
			psq_l		v1xy, 16(rQ1), 0, 0			
			psq_l		v1zw, 24(rQ1), 0, 0
			
		ps_sum1		B, temp1, temp5, temp4
		ps_sum0		B, B, B, B
		
		ps_sum0		C, temp3, temp5, temp1
		ps_sum0		C, C, C, C
		
		ps_sum0		D, temp2, temp3, temp6
		ps_sum0		D, D, D, D

		// Application of A B C D

		ps_madds0	temp1, q1xy, A, v1xy
		ps_madds0	temp2, q1zw, A, v1zw

		ps_mul		temp3, q1zw, B
		
			ps_neg		temp4, q1xy
			
		ps_sum0		temp1, temp1, temp1, temp3
		
			ps_merge10	temp4, q1xy, temp4
		
		ps_sum1		temp1, temp3, temp1, temp1
	
			ps_neg		temp6, q1zw
		
		ps_madds0	temp1, temp4, D, temp1
				
			ps_merge01	temp6, temp6, q1zw
			
		ps_madds0	temp1, q1zw, C, temp1
		
			ps_madds1	temp2, D, q1zw, temp2
		
		psq_st		temp1, 0(pPosScale), 0, 0
		
			ps_madd		temp2, C, q1xy, temp2
					
			ps_merge10	temp5, temp4, temp4
			ps_madd		temp2, B, temp5, temp2
			
			psq_st		v2zw, 8(pPosScale), 0, 0
			psq_st		temp2, 8(pPosScale), 1, 0
    }
    
	return *this;
}

//
//
FINLINE CFTQuatA &CFTQuatA::Mul( register const CFTQuatA &rQ ) 
{
	static CFTQuatA qTemp;
	
	qTemp.Mul( *this, rQ );
	
	register f32 v1xy, v1zw, v2xy, v2zw;
	register f32 q1xy, q1zw;

	register f32 q1zx, v2yz;
	
	register f32 A, B, C, D;
	register f32 Axy, Azw;
	register f32 temp1, temp2, temp3, temp4, temp5, temp6;
	register CFVec4A *pPosScale = &qTemp.m_PosScale;
	register const CFTQuatA *pThis = this;

    asm
    {
    	// Calculation of A B C D
		psq_l       q1xy, 0(pThis), 0, 0
		psq_l       v2xy, 16(rQ), 0, 0
		
		psq_l       v2zw, 24(rQ), 0, 0
		psq_l       q1zw, 8(pThis), 0, 0
		
			ps_mul		Axy, q1xy, v2xy
			ps_mul		Azw, q1zw, v2zw
		
		ps_muls1	temp1, v2xy, q1zw
		ps_muls1	temp2, v2zw, q1zw
		
			ps_sum1		A, Axy, Azw, Axy
			ps_sum0		A, A, A, A
		
		ps_merge00	q1zx, q1zw, q1xy
		ps_merge10	v2yz, v2xy, v2zw
		
		ps_mul		temp3, q1zx, v2xy
		ps_mul		temp4, q1xy, v2yz
		
		ps_mul		temp5, q1zx, v2yz
		ps_muls0	temp6, q1xy, v2xy

		ps_neg		temp5, temp5
		ps_neg		temp6, temp6
		
			psq_l		v1xy, 16(pThis), 0, 0			
			psq_l		v1zw, 24(pThis), 0, 0
			
		ps_sum1		B, temp1, temp5, temp4
		ps_sum0		B, B, B, B
		
		ps_sum0		C, temp3, temp5, temp1
		ps_sum0		C, C, C, C
		
		ps_sum0		D, temp2, temp3, temp6
		ps_sum0		D, D, D, D

		// Application of A B C D

		ps_madds0	temp1, q1xy, A, v1xy
		ps_madds0	temp2, q1zw, A, v1zw

		ps_mul		temp3, q1zw, B
		
			ps_neg		temp4, q1xy
			
		ps_sum0		temp1, temp1, temp1, temp3
		
			ps_merge10	temp4, q1xy, temp4
		
		ps_sum1		temp1, temp3, temp1, temp1
	
			ps_neg		temp6, q1zw
		
		ps_madds0	temp1, temp4, D, temp1
				
			ps_merge01	temp6, temp6, q1zw
			
		ps_madds0	temp1, q1zw, C, temp1
		
			ps_madds1	temp2, D, q1zw, temp2
		
		psq_st		temp1, 0(pPosScale), 0, 0
		
			ps_madd		temp2, C, q1xy, temp2
					
			ps_merge10	temp5, temp4, temp4
			ps_madd		temp2, B, temp5, temp2
			
			psq_st		v2zw, 8(pPosScale), 0, 0
			psq_st		temp2, 8(pPosScale), 1, 0
    }
    
    this->Set( qTemp );
	return *this;
}

/*
FINLINE CFVec3A &CFTQuatA::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const 
{
	MulPoint( rRV.v4a, rV.v4a );
	return rRV;
}

FINLINE CFVec4A &CFTQuatA::MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const 
{
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

FINLINE CFVec3A &CFTQuatA::MulPoint( CFVec3A &rV ) const 
{
	MulPoint( rV.v4a );
	return rV;
}

FINLINE CFVec4A &CFTQuatA::MulPoint( CFVec4A &rV ) const 
{
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
*/

//
//
FINLINE CFMtx43A& CFTQuatA::BuildMtx( CFMtx43A &rDestMtx ) const 
{
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

	rDestMtx.m_vPos.Set( m_Pos );

	return rDestMtx;
}

//
//
FINLINE CFMtx44 &CFTQuatA::BuildMtx( CFMtx44 &rDestMtx ) const 
{
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

//
//
FINLINE CFMtx44A &CFTQuatA::BuildMtx( CFMtx44A &rDestMtx ) const 
{
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
	rDestMtx.aa[3][3] = 1.0f;

	return rDestMtx;
}

//
//
FINLINE CFTQuatA &CFTQuatA::ReceiveSlerpOf( const f32 &fUnitSlerp, const CFTQuatA &rQuat0, const CFTQuatA &rQuat1 ) 
{
	CFQuatA::ReceiveSlerpOf( fUnitSlerp, rQuat0, rQuat1 );
	m_Pos.ReceiveLerpOf( fUnitSlerp, rQuat0.m_Pos, rQuat1.m_Pos );
	m_fScale = FMATH_FPOT( fUnitSlerp, rQuat0.m_fScale, rQuat1.m_fScale );

	return *this;
}

//
//
FINLINE CFTQuatA &CFTQuatA::ReceiveSlerpOf( const f32 &fUnitSlerp0, const f32 &fUnitSlerp1, const CFTQuatA &rQuat0, const CFTQuatA &rQuat1 ) 
{
	CFQuatA::ReceiveSlerpOf( fUnitSlerp0, fUnitSlerp1, rQuat0, rQuat1 );

	m_Pos.x = fUnitSlerp0*rQuat0.m_Pos.x + fUnitSlerp1*rQuat1.m_Pos.x;
	m_Pos.y = fUnitSlerp0*rQuat0.m_Pos.y + fUnitSlerp1*rQuat1.m_Pos.y;
	m_Pos.z = fUnitSlerp0*rQuat0.m_Pos.z + fUnitSlerp1*rQuat1.m_Pos.z;

	m_fScale = fUnitSlerp0*rQuat0.m_fScale + fUnitSlerp1*rQuat1.m_fScale;

	return *this;
}

#if FANG_DEBUG_BUILD
#pragma global_optimizer off
#endif

