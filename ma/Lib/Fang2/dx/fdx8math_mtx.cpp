//////////////////////////////////////////////////////////////////////////////////////
// fmath_mtx.cpp - Fang matrix library.
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

#include "fang.h"
#include "fmath.h"




//--------------------------------------------------------------------
// CFMtx33 Implementation:
//--------------------------------------------------------------------

const CFMtx33 CFMtx33::m_IdentityMtx(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
);

CFMtx33 &CFMtx33::Invert( void ) {
	*this = GetInverse();

	return *this;
}

CFMtx33 CFMtx33::GetInverse( void ) const {
	CFMtx33 Inv;
	f32 fTemp, fScalar;

	fTemp = this->aa[0][0] * this->aa[1][1] * this->aa[2][2];
	fTemp -= (this->aa[0][0] * this->aa[1][2] * this->aa[2][1]);
	fTemp -= (this->aa[0][1] * this->aa[1][0] * this->aa[2][2]);
	fTemp += (this->aa[0][1] * this->aa[1][2] * this->aa[2][0]);
	fTemp += (this->aa[0][2] * this->aa[1][0] * this->aa[2][1]);
	fTemp -= (this->aa[0][2] * this->aa[1][1] * this->aa[2][0]);

	fScalar = 1.0f/fTemp;

	Inv.aa[0][0] = fScalar * ( (this->aa[1][1] * this->aa[2][2]) - (this->aa[1][2] * this->aa[2][1]) );
	Inv.aa[0][1] = fScalar * ( (-this->aa[0][1] * this->aa[2][2]) + (this->aa[0][2] * this->aa[2][1]) );
	Inv.aa[0][2] = fScalar * ( (this->aa[0][1] * this->aa[1][2]) - (this->aa[0][2] * this->aa[1][1]) );

	Inv.aa[1][0] = fScalar * ( (-this->aa[1][0] * this->aa[2][2]) + (this->aa[1][2] * this->aa[2][0]) );
	Inv.aa[1][1] = fScalar * ( (this->aa[0][0] * this->aa[2][2]) - (this->aa[0][2] * this->aa[2][0]) );
	Inv.aa[1][2] = fScalar * ( (-this->aa[0][0] * this->aa[1][2]) + (this->aa[0][2] * this->aa[1][0]) );

	Inv.aa[2][0] = fScalar * ( (this->aa[1][0] * this->aa[2][1]) - (this->aa[1][1] * this->aa[2][0]) );
	Inv.aa[2][1] = fScalar * ( (-this->aa[0][0] * this->aa[2][1]) + (this->aa[0][1] * this->aa[2][0]) );
	Inv.aa[2][2] = fScalar * ( (this->aa[0][0] * this->aa[1][1]) - (this->aa[0][1] * this->aa[1][0]) );

	return Inv;
}


//--------------------------------------------------------------------
// CFMtx43 Implementation:
//--------------------------------------------------------------------

const CFMtx43A FMath_aRotMtx43A[FMTX_ROTTYPE_COUNT] = {
	// FMATH_ROTMTXTYPE_0
	CFMtx43A(	1.0f,	0.0f,	0.0f,
				0.0f,	1.0f,	0.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_90X
	CFMtx43A(	1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	-1.0f,	0.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_180X
	CFMtx43A(	1.0f,	0.0f,	0.0f,
				0.0f,	-1.0f,	0.0f,
				0.0f,	0.0f,	-1.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_270X
	CFMtx43A(	1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	-1.0f,
				0.0f,	1.0f,	0.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_90Y
	CFMtx43A(	0.0f,	0.0f,	-1.0f,
				0.0f,	1.0f,	0.0f,
				1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_180Y
	CFMtx43A(	-1.0f,	0.0f,	0.0f,
				0.0f,	1.0f,	0.0f,
				0.0f,	0.0f,	-1.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_270Y
	CFMtx43A(	0.0f,	0.0f,	1.0f,
				0.0f,	1.0f,	0.0f,
				-1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_90Z
	CFMtx43A(	0.0f,	1.0f,	0.0f,
				-1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_180Z
	CFMtx43A(	-1.0f,	0.0f,	0.0f,
				0.0f,	-1.0f,	0.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_270Z
	CFMtx43A(	0.0f,	-1.0f,	0.0f,
				1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	0.0f,	0.0f	),

	// FMATH_ROTMTXTYPE_Z180X90
	CFMtx43A(	-1.0f,	0.0f,	0.0f,
				0.0f,	0.0f,	1.0f,
				0.0f,	1.0f,	0.0f,
				0.0f,	0.0f,	0.0f	),

};

const CFMtx43 CFMtx43::m_IdentityMtx(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43 &CFMtx43::Invert( void ) {
	*this = GetInverse();

	return *this;
}

CFMtx43 CFMtx43::GetInverse( void ) const {
	CFMtx43 Inv;
	f32 fTemp, fScalar;

	fTemp = this->aa[0][0] * this->aa[1][1] * this->aa[2][2];
	fTemp -= (this->aa[0][0] * this->aa[1][2] * this->aa[2][1]);
	fTemp -= (this->aa[0][1] * this->aa[1][0] * this->aa[2][2]);
	fTemp += (this->aa[0][1] * this->aa[1][2] * this->aa[2][0]);
	fTemp += (this->aa[0][2] * this->aa[1][0] * this->aa[2][1]);
	fTemp -= (this->aa[0][2] * this->aa[1][1] * this->aa[2][0]);

	fScalar = 1.0f/fTemp;

	Inv.aa[0][0] = fScalar * ( (this->aa[1][1] * this->aa[2][2]) - (this->aa[1][2] * this->aa[2][1]) );
	Inv.aa[0][1] = fScalar * ( (-this->aa[0][1] * this->aa[2][2]) + (this->aa[0][2] * this->aa[2][1]) );
	Inv.aa[0][2] = fScalar * ( (this->aa[0][1] * this->aa[1][2]) - (this->aa[0][2] * this->aa[1][1]) );

	Inv.aa[1][0] = fScalar * ( (-this->aa[1][0] * this->aa[2][2]) + (this->aa[1][2] * this->aa[2][0]) );
	Inv.aa[1][1] = fScalar * ( (this->aa[0][0] * this->aa[2][2]) - (this->aa[0][2] * this->aa[2][0]) );
	Inv.aa[1][2] = fScalar * ( (-this->aa[0][0] * this->aa[1][2]) + (this->aa[0][2] * this->aa[1][0]) );

	Inv.aa[2][0] = fScalar * ( (this->aa[1][0] * this->aa[2][1]) - (this->aa[1][1] * this->aa[2][0]) );
	Inv.aa[2][1] = fScalar * ( (-this->aa[0][0] * this->aa[2][1]) + (this->aa[0][1] * this->aa[2][0]) );
	Inv.aa[2][2] = fScalar * ( (this->aa[0][0] * this->aa[1][1]) - (this->aa[0][1] * this->aa[1][0]) );

	Inv.aa[3][0] = (-this->aa[1][0] * this->aa[2][1] * this->aa[3][2]) + 
				 (this->aa[1][0] * this->aa[3][1] * this->aa[2][2]) +
				 (this->aa[1][1] * this->aa[2][0] * this->aa[3][2]) -
				 (this->aa[1][1] * this->aa[3][0] * this->aa[2][2]) -
				 (this->aa[1][2] * this->aa[2][0] * this->aa[3][1]) +
				 (this->aa[1][2] * this->aa[3][0] * this->aa[2][1]);
	Inv.aa[3][0] *= fScalar;

	Inv.aa[3][1] = (this->aa[0][0] * this->aa[2][1] * this->aa[3][2]) - 
				 (this->aa[0][0] * this->aa[3][1] * this->aa[2][2]) -
				 (this->aa[0][1] * this->aa[2][0] * this->aa[3][2]) +
				 (this->aa[0][1] * this->aa[3][0] * this->aa[2][2]) +
				 (this->aa[0][2] * this->aa[2][0] * this->aa[3][1]) -
				 (this->aa[0][2] * this->aa[3][0] * this->aa[2][1]);
	Inv.aa[3][1] *= fScalar;

	Inv.aa[3][2] = (-this->aa[0][0] * this->aa[1][1] * this->aa[3][2]) + 
				 (this->aa[0][0] * this->aa[3][1] * this->aa[1][2]) +
				 (this->aa[0][1] * this->aa[1][0] * this->aa[3][2]) -
				 (this->aa[0][1] * this->aa[3][0] * this->aa[1][2]) -
				 (this->aa[0][2] * this->aa[1][0] * this->aa[3][1]) +
				 (this->aa[0][2] * this->aa[3][0] * this->aa[1][1]);
	Inv.aa[3][2] *= fScalar;

	return Inv;
}




//--------------------------------------------------------------------
// CFMtx44 Implementation:
//--------------------------------------------------------------------

const CFMtx44 CFMtx44::m_IdentityMtx(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);





//--------------------------------------------------------------------
// CFMtx44A Implementation:
//--------------------------------------------------------------------

const CFMtx44A CFMtx44A::m_ZeroMtx(
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);

const CFMtx44A CFMtx44A::m_IdentityMtx(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);



#if !FANG_WINGC

#pragma warning( disable : 4700 )
CFMtx44A &CFMtx44A::ReceiveInverse( const CFMtx44A &rM ) {
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&rM.a[0])), (__m64*)(&rM.a[4]));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(&rM.a[8])), (__m64*)(&rM.a[12]));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&rM.a[2])), (__m64*)(&rM.a[6]));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(&rM.a[10])), (__m64*)(&rM.a[14]));
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_mul_ps(row1, tmp1);
	minor1 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row1, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2 = _mm_shuffle_ps(row2, row2, 0x4E);
	minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
	// -----------------------------------------------
	det = _mm_mul_ps(row0, minor0);
	det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1 = _mm_rcp_ss(det);
	det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
	det = _mm_shuffle_ps(det, det, 0x00);
	minor0 = _mm_mul_ps(det, minor0);
	_mm_storel_pi((__m64*)(&a[0]), minor0);
	_mm_storeh_pi((__m64*)(&a[2]), minor0);
	minor1 = _mm_mul_ps(det, minor1);
	_mm_storel_pi((__m64*)(&a[4]), minor1);
	_mm_storeh_pi((__m64*)(&a[6]), minor1);
	minor2 = _mm_mul_ps(det, minor2);
	_mm_storel_pi((__m64*)(&a[8]), minor2);
	_mm_storeh_pi((__m64*)(&a[10]), minor2);
	minor3 = _mm_mul_ps(det, minor3);
	_mm_storel_pi((__m64*)(&a[12]), minor3);
	_mm_storeh_pi((__m64*)(&a[14]), minor3);

	return *this;
}
#pragma warning( default : 4700 )


#pragma warning( disable : 4700 )
CFMtx44A &CFMtx44A::Invert( void ) {
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&a[0])), (__m64*)(&a[4]));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(&a[8])), (__m64*)(&a[12]));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&a[2])), (__m64*)(&a[6]));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(&a[10])), (__m64*)(&a[14]));
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_mul_ps(row1, tmp1);
	minor1 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row1, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2 = _mm_shuffle_ps(row2, row2, 0x4E);
	minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
	// -----------------------------------------------
	det = _mm_mul_ps(row0, minor0);
	det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1 = _mm_rcp_ss(det);
	det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
	det = _mm_shuffle_ps(det, det, 0x00);
	minor0 = _mm_mul_ps(det, minor0);
	_mm_storel_pi((__m64*)(&a[0]), minor0);
	_mm_storeh_pi((__m64*)(&a[2]), minor0);
	minor1 = _mm_mul_ps(det, minor1);
	_mm_storel_pi((__m64*)(&a[4]), minor1);
	_mm_storeh_pi((__m64*)(&a[6]), minor1);
	minor2 = _mm_mul_ps(det, minor2);
	_mm_storel_pi((__m64*)(&a[8]), minor2);
	_mm_storeh_pi((__m64*)(&a[10]), minor2);
	minor3 = _mm_mul_ps(det, minor3);
	_mm_storel_pi((__m64*)(&a[12]), minor3);
	_mm_storeh_pi((__m64*)(&a[14]), minor3);

	return *this;
}
#pragma warning( default : 4700 )

#else

#endif


//--------------------------------------------------------------------
// CFMtx43A Implementation:
//--------------------------------------------------------------------

const CFMtx43A CFMtx43A::m_ZeroMtx(
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f
);

const CFMtx43A CFMtx43A::m_IdentityMtx(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_Temp(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_Xlat(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_RotX(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_RotY(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_RotZ(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_XlatRotX(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_XlatRotY(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);

CFMtx43A CFMtx43A::m_XlatRotZ(
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
);


// Builds a unit matrix from a unit direction vector.
// The vector becomes the matrix's Z axis. The X and Y
// axis are arbitrary.
//
// NOTE: This function does not write to the matrix's
//       position component!
CFMtx43A &CFMtx43A::UnitMtxFromUnitVec( const CFVec3A *pUnitVecZ ) {
	f32 fMag2;

	m_vZ = *pUnitVecZ;

	m_vX.CrossYWithVec( m_vZ );
	fMag2 = m_vX.MagSq();
	if( fMag2 > 0.0001f ) {
		m_vX.Mul( fmath_InvSqrt( fMag2 ) );
		m_vY.Cross( m_vZ, m_vX );
	} else {
		if( m_vZ.y > 0.0f ) {
			// Z axis is pointing straignt up...

			m_vX = CFVec3A::m_UnitAxisX;
			m_vY.Set( 0.0f, 0.0f, -1.0f );
			m_vZ = CFVec3A::m_UnitAxisY;
		} else {
			// Z axis is pointing straight down...

			m_vX = CFVec3A::m_UnitAxisX;
			m_vY = CFVec3A::m_UnitAxisZ;
			m_vZ.Set( 0.0f, -1.0f, 0.0f );
		}
	}

	return *this;
}


// Same as UnitMtxFromUnitVec(), but accepts a non-unit vector.
// If a NULL vector is provided, the resulting matrix is the
// identity matrix.
//
// NOTE: This function does not write to the matrix's
//       position component!
CFMtx43A &CFMtx43A::UnitMtxFromNonUnitVec( const CFVec3A *pVecZ ) {
	f32 fMag2;

	fMag2 = pVecZ->MagSq();
	if( fMag2 < 0.0001f ) {
		Identity33();
		return *this;
	}

	m_vZ.Mul( *pVecZ, fmath_InvSqrt(fMag2) );

	m_vX.CrossYWithVec( m_vZ );
	fMag2 = m_vX.MagSq();
	if( fMag2 > 0.0001f ) {
		m_vX.Mul( fmath_InvSqrt( fMag2 ) );
		m_vY.Cross( m_vZ, m_vX );
	} else {
		if( m_vZ.y > 0.0f ) {
			// Z axis is pointing straignt up...

			m_vX = CFVec3A::m_UnitAxisX;
			m_vY.Set( 0.0f, 0.0f, -1.0f );
			m_vZ = CFVec3A::m_UnitAxisY;
		} else {
			// Z axis is pointing straight down...

			m_vX = CFVec3A::m_UnitAxisX;
			m_vY = CFVec3A::m_UnitAxisZ;
			m_vZ.Set( 0.0f, -1.0f, 0.0f );
		}
	}

	return *this;
}

// Builds a unit matrix from a unit direction vector.
// The vector becomes the matrix's Z axis. The X and Y
// axis are minimally rotated, although they will be
// arbitrary if necessary...
//
// NOTE: This function does not write to the matrix's
//       position component! And the matrix needs good initialization
CFMtx43A &CFMtx43A::UnitMtxFromUnitVecPreserveAxes( const CFVec3A *pUnitVecZ ) {
	
	FASSERT(m_vY.FCheck() == FMATH_FCHECK_RESULT_OK); // You need at least a valid Y Vector
	FASSERT(fmath_Abs(m_vY.MagSq()-1.0f) < 0.1f); // You call that a unit vector?

	m_vZ = *pUnitVecZ;

	m_vX.Cross(m_vY, m_vZ );
	f32 fMag2 = m_vX.MagSq();
	if( fMag2 > 0.0001f ) {
		m_vX.Mul( fmath_InvSqrt( fMag2 ) );
		m_vY.Cross( m_vZ, m_vX );
	} else {
		this->UnitMtxFromUnitVec(pUnitVecZ);
	}
	return *this;
}




#if !FANG_WINGC

#pragma warning( disable : 4700 )
CFMtx43A &CFMtx43A::ReceiveInverse( const CFMtx43A &rM ) {
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&rM.a[0])), (__m64*)(&rM.a[4]));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(&rM.a[8])), (__m64*)(&rM.a[12]));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&rM.a[2])), (__m64*)(&rM.a[6]));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(&rM.a[10])), (__m64*)(&rM.a[14]));
	row3 = _mm_add_ps( row3, CFVec4A::m_UnitAxisW._m );
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_mul_ps(row1, tmp1);
	minor1 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row1, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2 = _mm_shuffle_ps(row2, row2, 0x4E);
	minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
	// -----------------------------------------------
	det = _mm_mul_ps(row0, minor0);
	det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1 = _mm_rcp_ss(det);
	det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
	det = _mm_shuffle_ps(det, det, 0x00);
	minor0 = _mm_mul_ps(det, minor0);
	_mm_storel_pi((__m64*)(&a[0]), minor0);
	_mm_storeh_pi((__m64*)(&a[2]), minor0);
	minor1 = _mm_mul_ps(det, minor1);
	_mm_storel_pi((__m64*)(&a[4]), minor1);
	_mm_storeh_pi((__m64*)(&a[6]), minor1);
	minor2 = _mm_mul_ps(det, minor2);
	_mm_storel_pi((__m64*)(&a[8]), minor2);
	_mm_storeh_pi((__m64*)(&a[10]), minor2);
	minor3 = _mm_mul_ps(det, minor3);
	_mm_storel_pi((__m64*)(&a[12]), minor3);
	_mm_storeh_pi((__m64*)(&a[14]), minor3);

	return *this;
}
#pragma warning( default : 4700 )


#pragma warning( disable : 4700 )
CFMtx43A &CFMtx43A::Invert( void ) {
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&a[0])), (__m64*)(&a[4]));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(&a[8])), (__m64*)(&a[12]));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(&a[2])), (__m64*)(&a[6]));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(&a[10])), (__m64*)(&a[14]));
	row3 = _mm_add_ps( row3, CFVec4A::m_UnitAxisW._m );
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_mul_ps(row1, tmp1);
	minor1 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row1, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2 = _mm_shuffle_ps(row2, row2, 0x4E);
	minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
	// -----------------------------------------------
	det = _mm_mul_ps(row0, minor0);
	det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1 = _mm_rcp_ss(det);
	det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
	det = _mm_shuffle_ps(det, det, 0x00);
	minor0 = _mm_mul_ps(det, minor0);
	_mm_storel_pi((__m64*)(&a[0]), minor0);
	_mm_storeh_pi((__m64*)(&a[2]), minor0);
	minor1 = _mm_mul_ps(det, minor1);
	_mm_storel_pi((__m64*)(&a[4]), minor1);
	_mm_storeh_pi((__m64*)(&a[6]), minor1);
	minor2 = _mm_mul_ps(det, minor2);
	_mm_storel_pi((__m64*)(&a[8]), minor2);
	_mm_storeh_pi((__m64*)(&a[10]), minor2);
	minor3 = _mm_mul_ps(det, minor3);
	_mm_storel_pi((__m64*)(&a[12]), minor3);
	_mm_storeh_pi((__m64*)(&a[14]), minor3);

	return *this;
}
#pragma warning( default : 4700 )

#else

CFMtx43A &CFMtx43A::ReceiveInverse( const CFMtx43A &rM ) {
	f32 fTemp, fScalar;

	fTemp = rM.aa[0][0] * rM.aa[1][1] * rM.aa[2][2];
	fTemp -= (rM.aa[0][0] * rM.aa[1][2] * rM.aa[2][1]);
	fTemp -= (rM.aa[0][1] * rM.aa[1][0] * rM.aa[2][2]);
	fTemp += (rM.aa[0][1] * rM.aa[1][2] * rM.aa[2][0]);
	fTemp += (rM.aa[0][2] * rM.aa[1][0] * rM.aa[2][1]);
	fTemp -= (rM.aa[0][2] * rM.aa[1][1] * rM.aa[2][0]);

	fScalar = 1.0f/fTemp;

	aa[0][0] = fScalar * ( (rM.aa[1][1] * rM.aa[2][2]) - (rM.aa[1][2] * rM.aa[2][1]) );
	aa[0][1] = fScalar * ( (-rM.aa[0][1] * rM.aa[2][2]) + (rM.aa[0][2] * rM.aa[2][1]) );
	aa[0][2] = fScalar * ( (rM.aa[0][1] * rM.aa[1][2]) - (rM.aa[0][2] * rM.aa[1][1]) );

	aa[1][0] = fScalar * ( (-rM.aa[1][0] * rM.aa[2][2]) + (rM.aa[1][2] * rM.aa[2][0]) );
	aa[1][1] = fScalar * ( (rM.aa[0][0] * rM.aa[2][2]) - (rM.aa[0][2] * rM.aa[2][0]) );
	aa[1][2] = fScalar * ( (-rM.aa[0][0] * rM.aa[1][2]) + (rM.aa[0][2] * rM.aa[1][0]) );

	aa[2][0] = fScalar * ( (rM.aa[1][0] * rM.aa[2][1]) - (rM.aa[1][1] * rM.aa[2][0]) );
	aa[2][1] = fScalar * ( (-rM.aa[0][0] * rM.aa[2][1]) + (rM.aa[0][1] * rM.aa[2][0]) );
	aa[2][2] = fScalar * ( (rM.aa[0][0] * rM.aa[1][1]) - (rM.aa[0][1] * rM.aa[1][0]) );

	aa[3][0] = (-rM.aa[1][0] * rM.aa[2][1] * rM.aa[3][2]) + 
				 (rM.aa[1][0] * rM.aa[3][1] * rM.aa[2][2]) +
				 (rM.aa[1][1] * rM.aa[2][0] * rM.aa[3][2]) -
				 (rM.aa[1][1] * rM.aa[3][0] * rM.aa[2][2]) -
				 (rM.aa[1][2] * rM.aa[2][0] * rM.aa[3][1]) +
				 (rM.aa[1][2] * rM.aa[3][0] * rM.aa[2][1]);
	aa[3][0] *= fScalar;

	aa[3][1] = (rM.aa[0][0] * rM.aa[2][1] * rM.aa[3][2]) - 
				 (rM.aa[0][0] * rM.aa[3][1] * rM.aa[2][2]) -
				 (rM.aa[0][1] * rM.aa[2][0] * rM.aa[3][2]) +
				 (rM.aa[0][1] * rM.aa[3][0] * rM.aa[2][2]) +
				 (rM.aa[0][2] * rM.aa[2][0] * rM.aa[3][1]) -
				 (rM.aa[0][2] * rM.aa[3][0] * rM.aa[2][1]);
	aa[3][1] *= fScalar;

	aa[3][2] = (-rM.aa[0][0] * rM.aa[1][1] * rM.aa[3][2]) + 
				 (rM.aa[0][0] * rM.aa[3][1] * rM.aa[1][2]) +
				 (rM.aa[0][1] * rM.aa[1][0] * rM.aa[3][2]) -
				 (rM.aa[0][1] * rM.aa[3][0] * rM.aa[1][2]) -
				 (rM.aa[0][2] * rM.aa[1][0] * rM.aa[3][1]) +
				 (rM.aa[0][2] * rM.aa[3][0] * rM.aa[1][1]);
	aa[3][2] *= fScalar;

	return *this;
}


CFMtx43A &CFMtx43A::Invert( void ) { CFMtx43A Mtx = *this; ReceiveInverse( Mtx ); return *this; }

#endif




void CFMtx43A::SetRotationYXZ( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ ) {
	f32 fSinX, fSinY, fSinZ;
	f32 fCosX, fCosY, fCosZ;
	f32 fSinXSinZ, fSinXCosZ;

	fmath_SinCos( fRadiansX, &fSinX, &fCosX );
	fmath_SinCos( fRadiansY, &fSinY, &fCosY );
	fmath_SinCos( fRadiansZ, &fSinZ, &fCosZ );

	fSinXSinZ = fSinX * fSinZ;
	fSinXCosZ = fSinX * fCosZ;

	aa[0][0] = fCosY*fCosZ + fSinXSinZ*fSinY;
	aa[0][1] = fCosX*fSinZ;
	aa[0][2] = fSinXSinZ*fCosY - fSinY*fCosZ;

	aa[1][0] = fSinXCosZ*fSinY - fCosY*fSinZ;
	aa[1][1] = fCosX*fCosZ;
	aa[1][2] = fSinXCosZ*fCosY + fSinY*fSinZ;

	aa[2][0] = fSinY*fCosX;
	aa[2][1] = -fSinX;
	aa[2][2] = fCosY*fCosX;
}


