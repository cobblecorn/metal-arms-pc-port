//////////////////////////////////////////////////////////////////////////////////////
// fdx8math_vec.cpp - Fang vector library.
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
// CFVec2 Implementation:
//--------------------------------------------------------------------
const CFVec2 CFVec2::m_Null( 0.0f, 0.0f );
const CFVec2 CFVec2::m_UnitAxisX( 1.0f, 0.0f );
const CFVec2 CFVec2::m_UnitAxisY( 0.0f, 1.0f );



//--------------------------------------------------------------------
// CFVec3 Implementation:
//--------------------------------------------------------------------
const CFVec3 CFVec3::m_Null( 0.0f, 0.0f, 0.0f );
const CFVec3 CFVec3::m_UnitAxisX( 1.0f, 0.0f, 0.0f );
const CFVec3 CFVec3::m_UnitAxisY( 0.0f, 1.0f, 0.0f );
const CFVec3 CFVec3::m_UnitAxisZ( 0.0f, 0.0f, 1.0f );



//--------------------------------------------------------------------
// CFVec4 Implementation:
//--------------------------------------------------------------------
const CFVec4 CFVec4::m_Null( 0.0f, 0.0f, 0.0f, 0.0f );
const CFVec4 CFVec4::m_UnitAxisX( 1.0f, 0.0f, 0.0f, 0.0f );
const CFVec4 CFVec4::m_UnitAxisY( 0.0f, 1.0f, 0.0f, 0.0f );
const CFVec4 CFVec4::m_UnitAxisZ( 0.0f, 0.0f, 1.0f, 0.0f );
const CFVec4 CFVec4::m_UnitAxisQ( 0.0f, 0.0f, 0.0f, 1.0f );



//--------------------------------------------------------------------
// CFVec3A Implementation:
//--------------------------------------------------------------------
const CFVec3A CFVec3A::m_Null( 0.0f, 0.0f, 0.0f );
const CFVec3A CFVec3A::m_Ones( 1.0f, 1.0f, 1.0f );
const CFVec3A CFVec3A::m_NegOnes( -1.0f, -1.0f, -1.0f );
const CFVec3A CFVec3A::m_UnitAxisX( 1.0f, 0.0f, 0.0f );
const CFVec3A CFVec3A::m_UnitAxisY( 0.0f, 1.0f, 0.0f );
const CFVec3A CFVec3A::m_UnitAxisZ( 0.0f, 0.0f, 1.0f );
const CFVec3A CFVec3A::m_HalfVec( 0.5f, 0.5f, 0.5f );
const CFVec3A CFVec3A::m_NegHalfVec( -0.5f, -0.5f, -0.5f );
const CFVec3A CFVec3A::m_DoubleVec( 2.0f, 2.0f, 2.0f );
const CFVec3A CFVec3A::m_NegDoubleVec( -2.0f, -2.0f, -2.0f );

CFVec3A CFVec3A::m_TempVec;

FMathFcheckResult_e CFVec3A::FCheck( void ) const {
	FASSERT( w == 0.0f );
	return v4a.FCheck();
}


CFVec3A &CFVec3A::SetUnitRandom( void ) {
	x = fmath_RandomBipolarUnitFloat();
	y = fmath_RandomBipolarUnitFloat();
	z = fmath_RandomBipolarUnitFloat();

	f32 fMag2 = MagSq();

	if( fMag2 >= 0.00001f ) {
		Mul( fmath_InvSqrt( fMag2 ) );
	} else {
		Set( m_UnitAxisX );
	}

	return *this;
}




//--------------------------------------------------------------------
// CFVec4A Implementation:
//--------------------------------------------------------------------
const CFVec4A CFVec4A::m_Null( 0.0f, 0.0f, 0.0f, 0.0f );
const CFVec4A CFVec4A::m_NullW1( 0.0f, 0.0f, 0.0f, 1.0f );
const CFVec4A CFVec4A::m_Ones( 1.0f, 1.0f, 1.0f, 1.0f );
const CFVec4A CFVec4A::m_NegOnes( -1.0f, -1.0f, -1.0f, -1.0f );
const CFVec4A CFVec4A::m_NegOnesW0( -1.0f, -1.0f, -1.0f, 0.0f );
const CFVec4A CFVec4A::m_NegOnesW1( -1.0f, -1.0f, -1.0f, 1.0f );
const CFVec4A CFVec4A::m_UnitAxisX( 1.0f, 0.0f, 0.0f, 0.0f );
const CFVec4A CFVec4A::m_UnitAxisY( 0.0f, 1.0f, 0.0f, 0.0f );
const CFVec4A CFVec4A::m_UnitAxisZ( 0.0f, 0.0f, 1.0f, 0.0f );
const CFVec4A CFVec4A::m_UnitAxisW( 0.0f, 0.0f, 0.0f, 1.0f );
const CFVec4A CFVec4A::m_HalfVec( 0.5f, 0.5f, 0.5f, 0.5f );
const CFVec4A CFVec4A::m_NegHalfVec( -0.5f, -0.5f, -0.5f, -0.5f );
const CFVec4A CFVec4A::m_DoubleVec( 2.0f, 2.0f, 2.0f, 2.0f );
const CFVec4A CFVec4A::m_NegDoubleVec( -2.0f, -2.0f, -2.0f, -2.0f );

CFVec4A CFVec4A::m_TempVec;

FMathFcheckResult_e CFVec4A::FCheck( void ) const {
	FMathFcheckResult_e nRetValue;

	if( (nRetValue=fmath_Fcheck(x)) != FMATH_FCHECK_RESULT_OK ) {
		return nRetValue;
	}

	if( (nRetValue=fmath_Fcheck(y)) != FMATH_FCHECK_RESULT_OK ) {
		return nRetValue;
	}

	if( (nRetValue=fmath_Fcheck(z)) != FMATH_FCHECK_RESULT_OK ) {
		return nRetValue;
	}

	if( (nRetValue=fmath_Fcheck(w)) != FMATH_FCHECK_RESULT_OK ) {
		return nRetValue;
	}

	return FMATH_FCHECK_RESULT_OK;
}

