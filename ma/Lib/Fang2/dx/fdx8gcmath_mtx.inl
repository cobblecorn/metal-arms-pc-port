//////////////////////////////////////////////////////////////////////////////////////
// fdx8gcmath_mtx.inl - Fang matrix library.
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
// CFMtx44A Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx44A::CFMtx44A( void ) {}
FINLINE CFMtx44A::CFMtx44A( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) { Set( rRight, rUp, rFront, rPos ); }
FINLINE CFMtx44A::CFMtx44A( const CFVec4A &rRight, const CFVec4A &rUp, const CFVec4A &rFront, const CFVec4A &rPos ) { Set( rRight, rUp, rFront, rPos ); }
FINLINE CFMtx44A::CFMtx44A( const CFMtx44A &rM ) { Set( rM ); }
FINLINE CFMtx44A::CFMtx44A( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 ) {
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=ff03;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=ff13;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=ff23;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=ff33;
}

FINLINE BOOL CFMtx44A::operator == ( const CFMtx44A &rM ) const { return (m44 == rM.m44); }
FINLINE BOOL CFMtx44A::operator != ( const CFMtx44A &rM ) const { return (m44 != rM.m44); }

FINLINE CFMtx44A &CFMtx44A::Zero( void ) {
	Set( m_ZeroMtx );
	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Identity( void ) {
	Set( m_IdentityMtx );
	return *this;
}

FINLINE CFMtx44A &CFMtx44A::operator = ( const CFMtx44A &rM ) { Set( rM ); return *this; }

FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx33 &rM ) {
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vP = CFVec4A::m_UnitAxisW;

	m_vX.w = 0.0f;
	m_vY.w = 0.0f;
	m_vZ.w = 0.0f;

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx43 &rM ) {
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vP.v3 = rM.m_vPos;

	m_vX.w = 0.0f;
	m_vY.w = 0.0f;
	m_vZ.w = 0.0f;
	m_vP.w = 1.0f;

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx44 &rM ) {
	m_vX.v4 = rM.m_vX;
	m_vY.v4 = rM.m_vY;
	m_vZ.v4 = rM.m_vZ;
	m_vP.v4 = rM.m_vPos;

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx43A &rM ) {
	m_vX.Set( rM.m_vX );
	m_vY.Set( rM.m_vY );
	m_vZ.Set( rM.m_vZ );
	m_vP.Set( rM.m_vP );

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx44A &rM ) {
	m_vX.Set( rM.m_vX );
	m_vY.Set( rM.m_vY );
	m_vZ.Set( rM.m_vZ );
	m_vP.Set( rM.m_vP );

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Set( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) {
	m_vX.Set( rRight );
	m_vY.Set( rUp );
	m_vZ.Set( rFront );
	m_vP.Set( rPos );

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Set( const CFVec4A &rRight, const CFVec4A &rUp, const CFVec4A &rFront, const CFVec4A &rPos ) {
	m_vX.Set( rRight );
	m_vY.Set( rUp );
	m_vZ.Set( rFront );
	m_vP.Set( rPos );

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Set( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 ) {
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=ff03;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=ff13;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=ff23;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=ff33;

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM1, const CFMtx44A &rM2 ) { m44 = rM1.m44 * rM2.m44; return *this; }
FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM ) { m44 *= rM.m44; return *this; }
// Scales all 16 elements, as on the GameCube and the SSE build. Not m44 * fVal: CFMtx44::operator*( f32 )
// returns a CFMtx43, which drops the w column.
FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM, const f32 &fVal ) {
	m_vRight.Mul( rM.m_vRight, fVal );
	m_vUp.Mul( rM.m_vUp, fVal );
	m_vFront.Mul( rM.m_vFront, fVal );
	m_vPos.Mul( rM.m_vPos, fVal );

	return *this;
}
FINLINE CFMtx44A &CFMtx44A::Mul( const f32 &fVal ) { m44 *= fVal; return *this; }

FINLINE CFVec3A &CFMtx44A::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const { rRV.v3 = m44.MultPoint( rV.v3 ); return rRV; }
FINLINE CFVec4A &CFMtx44A::MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const { rRV.v4 = m44.MultPoint( rV.v4 ); return rRV; }
FINLINE CFVec3A &CFMtx44A::MulPoint( CFVec3A &rV ) const { CFVec3A Result; Result.v3 = m44.MultPoint( rV.v3 ); rV=Result; return rV; }
FINLINE CFVec4A &CFMtx44A::MulPoint( CFVec4A &rV ) const { CFVec4A Result; Result.v4 = m44.MultPoint( rV.v4 ); rV=Result; return rV; }

FINLINE CFVec3A &CFMtx44A::MulDir( CFVec3A &rRV, const CFVec3A &rV ) const { rRV.v3 = m44.MultDir( rV.v3 ); return rRV; }
FINLINE CFVec4A &CFMtx44A::MulDir( CFVec4A &rRV, const CFVec4A &rV ) const { rRV.v4 = m44.MultDir( rV.v4 ); return rRV; }
FINLINE CFVec3A &CFMtx44A::MulDir( CFVec3A &rV ) const { CFVec3A Result; Result.v3 = m44.MultDir( rV.v3 ); rV=Result; return rV; }
FINLINE CFVec4A &CFMtx44A::MulDir( CFVec4A &rV ) const { CFVec4A Result; Result.v4 = m44.MultDir( rV.v4 ); rV=Result; return rV; }

FINLINE CFMtx44A &CFMtx44A::ReceiveTranspose( const CFMtx44A &rM ) { rM.m44.Transpose( m44 ); return *this; }
FINLINE CFMtx44A &CFMtx44A::Transpose( void ) { m44.Transpose(); return *this; }

#if 0
FINLINE u32 CFMtx44A::GenKey( void ) const {
	u32 nKey;

	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		xorps		xmm1, [eax+0x10]
		xorps		xmm2, [eax+0x20]
		xorps		xmm3, [eax+0x30]
		movaps		xmm1, xmm0
		movaps		xmm2, xmm0
		movaps		xmm3, xmm0
		shufps		xmm1, xmm1, 0x55
		shufps		xmm2, xmm2, 0xaa
		shufps		xmm3, xmm3, 0xff
		xorps		xmm0, xmm1
		xorps		xmm0, xmm2
		xorps		xmm0, xmm3
		movss		nKey, xmm0
	}

	return nKey;
}
#endif

// RotateX - Rotates the matrix by "fRadians" around the X axis.
FINLINE void CFMtx44A::RotateX( const f32 fRadians ) {
	// Precalculate sin and cos for efficiency
	f32	fSin( fmath_Sin( fRadians ) );
	f32	fCos( fmath_Cos( fRadians ) );

	// Calculate new coords
	f32	RightY( (m_vRight.y	* fCos)	+ (m_vRight.z * -fSin) );
	f32	UpY( (m_vUp.y * fCos)	+ (m_vUp.z	* -fSin) );
	f32	FrontY( (m_vFront.y	* fCos)	+ (m_vFront.z * -fSin) );

	f32	RightZ( (m_vRight.y	* fSin) + (m_vRight.z * fCos) );
	f32	UpZ( (m_vUp.y * fSin) + (m_vUp.z	* fCos) );
	f32	FrontZ( (m_vFront.y	* fSin) + (m_vFront.z * fCos) );

	f32	PositionY( (m_vPos.y * fCos) + (m_vPos.z * -fSin) );
	f32	PositionZ( (m_vPos.y * fSin) + (m_vPos.z * fCos) );

	// Assign new coords
	m_vRight.y = RightY;
	m_vRight.z = RightZ;
	m_vUp.y	 = UpY;
	m_vUp.z	 = UpZ;
	m_vFront.y = FrontY;
	m_vFront.z = FrontZ;
	m_vPos.y = PositionY;
	m_vPos.z = PositionZ;
};

//	RotateY - Rotates the matrix by "fRadians" around the y axis
FINLINE void CFMtx44A::RotateY( const f32 fRadians ) {
	// Precalculate sin and cos for efficiency
	f32	fSin = fmath_Sin( fRadians );
	f32	fCos = fmath_Cos( fRadians );

	// Calculate new coords
	f32	RightX( (m_vRight.x	* fCos)	+ (m_vRight.z * fSin) );
	f32	UpX( (m_vUp.x * fCos)	+ (m_vUp.z	* fSin) );
	f32	FrontX( (m_vFront.x	* fCos)	+ (m_vFront.z * fSin) );

	f32	RightZ( (m_vRight.x	* -fSin)	+ (m_vRight.z * fCos) );
	f32	UpZ( (m_vUp.x * -fSin)	+ (m_vUp.z	* fCos) );
	f32	FrontZ( (m_vFront.x	* -fSin)	+ (m_vFront.z * fCos) );
	
	f32	PositionX( (m_vPos.x * fCos)	+ (m_vPos.z * fSin) );
	f32	PositionZ( (m_vPos.x * -fSin)	+ (m_vPos.z * fCos) );

	// Assign new coords
	m_vRight.x = RightX;
	m_vRight.z = RightZ;
	m_vUp.x	 = UpX;
	m_vUp.z	 = UpZ;
	m_vFront.x = FrontX;
	m_vFront.z = FrontZ;
	m_vPos.x = PositionX;
	m_vPos.z = PositionZ;
};

// RotateY - Rotates the matrix by "fRadians" around the y axis.
FINLINE void CFMtx44A::RotateZ( const f32 fRadians ) {
	// Precalculate sin and cos for efficiency
	f32	fSin = fmath_Sin( fRadians );
	f32	fCos = fmath_Cos( fRadians );

	// Calculate new coords
	f32	RightX( (m_vRight.x	* fCos)	+ (m_vRight.y * -fSin) );
	f32	UpX( (m_vUp.x	* fCos)	+ (m_vUp.y	* -fSin) );
	f32	FrontX( (m_vFront.x	* fCos)	+ (m_vFront.y * -fSin) );

	f32	RightY( (m_vRight.x	* fSin)	+ (m_vRight.y * fCos) );
	f32	UpY( (m_vUp.x	* fSin)	+ (m_vUp.y	* fCos) );
	f32	FrontY( (m_vFront.x	* fSin)	+ (m_vFront.y * fCos) );

	f32	PositionX( (m_vPos.x * fCos)	+ (m_vPos.y * -fSin) );
	f32	PositionY( (m_vPos.x * fSin)	+ (m_vPos.y * fCos) );

	// Assign new coords
	m_vRight.x = RightX;
	m_vRight.y = RightY;
	m_vUp.x	 = UpX;
	m_vUp.y	 = UpY;
	m_vFront.x = FrontX;
	m_vFront.y = FrontY;
	m_vPos.x = PositionX;
	m_vPos.y = PositionY;
};

// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx44A::SetRotationX( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx44A::SetRotationY( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx44A::SetRotationZ( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx44A::SetRotationX( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vUp.z *= fScale;
	m_vUp.y *= fScale;
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx44A::SetRotationY( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vFront.x *= fScale;
	m_vFront.z *= fScale;
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx44A::SetRotationZ( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vRight.y *= fScale;
	m_vRight.x *= fScale;
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

// NOTE: the Pitch, Yaw and Roll are negated, I assume because it is left handed.
FINLINE void CFMtx44A::GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const {
	rfPitchDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vUp.z, m_vUp.y ) );
	rfYawDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vFront.x, m_vFront.z ) );
	rfRollDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vRight.y, m_vRight.x ) );
}




//--------------------------------------------------------------------
// CFMtx43A Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx43A::CFMtx43A( void ) { aa[0][3] = aa[1][3] = aa[2][3] = aa[3][3] = 0.0f; }
FINLINE CFMtx43A::CFMtx43A( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) { Set( rRight, rUp, rFront, rPos ); }
FINLINE CFMtx43A::CFMtx43A( const CFMtx43A &rM ) { Set( rM ); }
FINLINE CFMtx43A::CFMtx43A( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 ) {
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=0.0f;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=0.0f;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=0.0f;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=0.0f;
}

FINLINE BOOL CFMtx43A::operator == ( const CFMtx43A &rM ) const { return (m44a == rM.m44a); }
FINLINE BOOL CFMtx43A::operator != ( const CFMtx43A &rM ) const { return (m44a != rM.m44a); }

FINLINE CFMtx43A &CFMtx43A::Zero( void ) {
	Set( m_ZeroMtx );
	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Identity( void ) {
	Set( m_IdentityMtx );
	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Identity33( void ) {
	m_vRight = CFVec3A::m_UnitAxisX;
	m_vUp = CFVec3A::m_UnitAxisY;
	m_vFront = CFVec3A::m_UnitAxisZ;
	return *this;
}

FINLINE CFMtx43A &CFMtx43A::operator = ( const CFMtx43A &rM ) { Set( rM ); return *this; }

FINLINE CFMtx43A &CFMtx43A::Set( const CFMtx33 &rM ) {
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vP.Zero();

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Set( const CFMtx43 &rM ) {
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vP.v3 = rM.m_vPos;

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Set( const CFMtx43A &rM ) {
	m_vX.Set( rM.m_vX );
	m_vY.Set( rM.m_vY );
	m_vZ.Set( rM.m_vZ );
	m_vP.Set( rM.m_vP );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Set( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) {
	m_vX.Set( rRight );
	m_vY.Set( rUp );
	m_vZ.Set( rFront );
	m_vP.Set( rPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Set( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 ) {
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32;

	return *this;
}

// Port: affine concatenation, matching the SSE versions in fdx8math_mtx.inl. this = rM1 * rM2
// (rM2 applied first): each row of rM2 is re-expressed in rM1's basis, and the position row also
// gets rM1's translation. These used to go through CFMtx44's 4x4 product, which scales rM1's
// translation by rM2.m_vPos.w - but the w lanes of a CFMtx43A are not maintained in this build
// (the constructor zeroes them), so every parent translation was dropped. That left bone
// palettes in model space, drawing boned meshes (e.g. Glitch) at the world origin.
// The temporaries make these safe when the destination aliases an operand.
FINLINE CFMtx43A &CFMtx43A::Mul( const CFMtx43A &rM1, const CFMtx43A &rM2 ) {
	f32 aafResult[4][3];
	for( u32 i = 0; i < 4; i++ ) {
		const f32 fX = rM2.aa[i][0], fY = rM2.aa[i][1], fZ = rM2.aa[i][2];
		for( u32 j = 0; j < 3; j++ ) {
			aafResult[i][j] = fX*rM1.aa[0][j] + fY*rM1.aa[1][j] + fZ*rM1.aa[2][j] + ( i == 3 ? rM1.aa[3][j] : 0.0f );
		}
	}
	for( u32 i = 0; i < 4; i++ ) {
		aa[i][0] = aafResult[i][0]; aa[i][1] = aafResult[i][1]; aa[i][2] = aafResult[i][2]; aa[i][3] = 0.0f;
	}
	return *this;
}
FINLINE CFMtx43A &CFMtx43A::Mul( const CFMtx43A &rM ) { return Mul( *this, rM ); }

// Rotation-only concatenation; the destination keeps its own position (as the SSE version).
FINLINE CFMtx43A &CFMtx43A::Mul33( const CFMtx43A &rM1, const CFMtx43A &rM2 ) {
	f32 aafResult[3][3];
	for( u32 i = 0; i < 3; i++ ) {
		const f32 fX = rM2.aa[i][0], fY = rM2.aa[i][1], fZ = rM2.aa[i][2];
		for( u32 j = 0; j < 3; j++ ) {
			aafResult[i][j] = fX*rM1.aa[0][j] + fY*rM1.aa[1][j] + fZ*rM1.aa[2][j];
		}
	}
	for( u32 i = 0; i < 3; i++ ) {
		aa[i][0] = aafResult[i][0]; aa[i][1] = aafResult[i][1]; aa[i][2] = aafResult[i][2]; aa[i][3] = 0.0f;
	}
	return *this;
}
FINLINE CFMtx43A &CFMtx43A::Mul33( const CFMtx43A &rM ) { return Mul33( *this, rM ); }

//
//
FINLINE CFMtx43A &CFMtx43A::Mul33( const CFMtx43A &rM1, const f32 &fVal ) 
{
	m_vRight.Mul( rM1.m_vRight, fVal );
	m_vUp.Mul( rM1.m_vUp, fVal );
	m_vFront.Mul( rM1.m_vFront, fVal );
	m_vPos.Set( rM1.m_vPos );

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Mul33( const f32 &fVal ) 
{
	m_vRight.Mul( fVal );
	m_vUp.Mul( fVal );
	m_vFront.Mul( fVal );
	
	return *this;
}


FINLINE CFMtx43A &CFMtx43A::Mul( const CFMtx43A &rM, const f32 &fVal ) { m44a.m44 = rM.m44a.m44 * fVal; return *this; }
FINLINE CFMtx43A &CFMtx43A::Mul( const f32 &fVal ) { m44a.m44 *= fVal; return *this; }

FINLINE CFVec3A &CFMtx43A::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const { rRV.v3 = m44a.m44.MultPoint( rV.v3 ); return rRV; }
FINLINE CFVec3A &CFMtx43A::MulPoint( CFVec3A &rV ) const { CFVec3A Result; Result.v3 = m44a.m44.MultPoint( rV.v3 ); rV=Result; return rV; }
FINLINE CFVec3&  CFMtx43A::MulPoint( CFVec3 &rRV, const CFVec3 &rV ) const 
{
	rRV.x = m44.m_vX.x*rV.x + m44.m_vY.x*rV.y + m44.m_vZ.x*rV.z + m_vPos.x;
	rRV.y = m44.m_vX.y*rV.x + m44.m_vY.y*rV.y + m44.m_vZ.y*rV.z + m_vPos.y;
	rRV.z = m44.m_vX.z*rV.x + m44.m_vY.z*rV.y + m44.m_vZ.z*rV.z + m_vPos.z;
	return rRV;
}

FINLINE CFVec3A &CFMtx43A::MulDir( CFVec3A &rRV, const CFVec3A &rV ) const { rRV.v3 = m44a.m44.MultDir( rV.v3 ); return rRV; }
FINLINE CFVec3A &CFMtx43A::MulDir( CFVec3A &rV ) const { CFVec3A Result; Result.v3 = m44a.m44.MultDir( rV.v3 ); rV=Result; return rV; }

FINLINE CFMtx43A &CFMtx43A::ReceiveAffineInverse( const CFMtx43A &rM, BOOL bRecomputeScale ) {
	CFVec3A NegPos;

	ReceiveTranspose33( rM );

	if( bRecomputeScale ) {
		NegPos.Set( rM.m_vRight.InvMagSq() );

		m_vRight.Mul( NegPos );
		m_vUp.Mul( NegPos );
		m_vFront.Mul( NegPos );
	}

	NegPos.ReceiveNegative( rM.m_vPos );
	MulDir( m_vPos, NegPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::AffineInvert( BOOL bRecomputeScale ) {
	Transpose33();

	if( bRecomputeScale ) {
		CFVec3A OOScaleVec2;
		OOScaleVec2.Set( m_vRight.InvMagSq() );

		m_vRight.Mul( OOScaleVec2 );
		m_vUp.Mul( OOScaleVec2 );
		m_vFront.Mul( OOScaleVec2 );
	}

	m_vPos.Negate();
	MulDir( m_vPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::ReceiveAffineInverse_KnowScale2( const CFMtx43A &rM, const f32 &fScaleOfSourceMtx2 ) {
	CFVec3A NegPos;

	ReceiveTranspose33( rM );

	if( fScaleOfSourceMtx2 != 1.0f ) {
		NegPos.Set( fmath_Inv( fScaleOfSourceMtx2 ) );

		m_vRight.Mul( NegPos );
		m_vUp.Mul( NegPos );
		m_vFront.Mul( NegPos );
	}

	NegPos.ReceiveNegative( rM.m_vPos );
	MulDir( m_vPos, NegPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::AffineInvert_KnowScale2( const f32 &fScaleOfSourceMtx2 ) {
	Transpose33();

	if( fScaleOfSourceMtx2 != 1.0f ) {
		CFVec3A OOScaleVec2;
		OOScaleVec2.Set( fmath_Inv( fScaleOfSourceMtx2 ) );

		m_vRight.Mul( OOScaleVec2 );
		m_vUp.Mul( OOScaleVec2 );
		m_vFront.Mul( OOScaleVec2 );
	}

	m_vPos.Negate();
	MulDir( m_vPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::ReceiveAffineInverse_KnowOOScale2( const CFMtx43A &rM, const f32 &fOOScaleOfSourceMtx2 ) {
	CFVec3A NegPos;

	ReceiveTranspose33( rM );

	if( fOOScaleOfSourceMtx2 != 1.0f ) {
		NegPos.Set( fOOScaleOfSourceMtx2 );

		m_vRight.Mul( NegPos );
		m_vUp.Mul( NegPos );
		m_vFront.Mul( NegPos );
	}

	NegPos.ReceiveNegative( rM.m_vPos );
	MulDir( m_vPos, NegPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::AffineInvert_KnowOOScale2( const f32 &fOOScaleOfSourceMtx2 ) {
	Transpose33();

	if( fOOScaleOfSourceMtx2 != 1.0f ) {
		CFVec3A OOScaleVec2;
		OOScaleVec2.Set( fOOScaleOfSourceMtx2 );

		m_vRight.Mul( OOScaleVec2 );
		m_vUp.Mul( OOScaleVec2 );
		m_vFront.Mul( OOScaleVec2 );
	}

	m_vPos.Negate();
	MulDir( m_vPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::ReceiveTranspose33( const CFMtx43A &rM ) {
	aa[0][0] = rM.aa[0][0];
	aa[1][0] = rM.aa[0][1];
	aa[2][0] = rM.aa[0][2];

	aa[0][1] = rM.aa[1][0];
	aa[1][1] = rM.aa[1][1];
	aa[2][1] = rM.aa[1][2];

	aa[0][2] = rM.aa[2][0];
	aa[1][2] = rM.aa[2][1];
	aa[2][2] = rM.aa[2][2];

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Transpose33( void ) {
	f32 aaf[3][3];

	aaf[1][0] = aa[0][1];
	aaf[2][0] = aa[0][2];

	aaf[0][1] = aa[1][0];
	aaf[2][1] = aa[1][2];

	aaf[0][2] = aa[2][0];
	aaf[1][2] = aa[2][1];

	aa[0][1] = aaf[0][1];
	aa[0][2] = aaf[0][2];

	aa[1][0] = aaf[1][0];
	aa[1][2] = aaf[1][2];

	aa[2][0] = aaf[2][0];
	aa[2][1] = aaf[2][1];

	return *this;
}

#if 0
FINLINE u32 CFMtx43A::GenKey( void ) const {
	u32 nKey;

	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		xorps		xmm1, [eax+0x10]
		xorps		xmm2, [eax+0x20]
		xorps		xmm3, [eax+0x30]
		movaps		xmm1, xmm0
		movaps		xmm2, xmm0
		movaps		xmm3, xmm0
		shufps		xmm1, xmm1, 0x55
		shufps		xmm2, xmm2, 0xaa
		shufps		xmm3, xmm3, 0xff
		xorps		xmm0, xmm1
		xorps		xmm0, xmm2
		xorps		xmm0, xmm3
		movss		nKey, xmm0
	}

	return nKey;
}
#endif

// RotateX - Rotates the matrix by "fRadians" around the X axis.
FINLINE void CFMtx43A::RotateX( const f32 fRadians ) {
	// Precalculate sin and cos for efficiency
	f32	fSin( fmath_Sin( fRadians ) );
	f32	fCos( fmath_Cos( fRadians ) );

	// Calculate new coords
	f32	RightY( (m_vRight.y	* fCos)	+ (m_vRight.z * -fSin) );
	f32	UpY( (m_vUp.y * fCos)	+ (m_vUp.z	* -fSin) );
	f32	FrontY( (m_vFront.y	* fCos)	+ (m_vFront.z * -fSin) );

	f32	RightZ( (m_vRight.y	* fSin) + (m_vRight.z * fCos) );
	f32	UpZ( (m_vUp.y * fSin) + (m_vUp.z	* fCos) );
	f32	FrontZ( (m_vFront.y	* fSin) + (m_vFront.z * fCos) );

	f32	PositionY( (m_vPos.y * fCos) + (m_vPos.z * -fSin) );
	f32	PositionZ( (m_vPos.y * fSin) + (m_vPos.z * fCos) );

	// Assign new coords
	m_vRight.y = RightY;
	m_vRight.z = RightZ;
	m_vUp.y	 = UpY;
	m_vUp.z	 = UpZ;
	m_vFront.y = FrontY;
	m_vFront.z = FrontZ;
	m_vPos.y = PositionY;
	m_vPos.z = PositionZ;
};

//	RotateY - Rotates the matrix by "fRadians" around the y axis
FINLINE void CFMtx43A::RotateY( const f32 fRadians ) {
	// Precalculate sin and cos for efficiency
	f32	fSin = fmath_Sin( fRadians );
	f32	fCos = fmath_Cos( fRadians );

	// Calculate new coords
	f32	RightX( (m_vRight.x	* fCos)	+ (m_vRight.z * fSin) );
	f32	UpX( (m_vUp.x * fCos)	+ (m_vUp.z	* fSin) );
	f32	FrontX( (m_vFront.x	* fCos)	+ (m_vFront.z * fSin) );

	f32	RightZ( (m_vRight.x	* -fSin)	+ (m_vRight.z * fCos) );
	f32	UpZ( (m_vUp.x * -fSin)	+ (m_vUp.z	* fCos) );
	f32	FrontZ( (m_vFront.x	* -fSin)	+ (m_vFront.z * fCos) );
	
	f32	PositionX( (m_vPos.x * fCos)	+ (m_vPos.z * fSin) );
	f32	PositionZ( (m_vPos.x * -fSin)	+ (m_vPos.z * fCos) );

	// Assign new coords
	m_vRight.x = RightX;
	m_vRight.z = RightZ;
	m_vUp.x	 = UpX;
	m_vUp.z	 = UpZ;
	m_vFront.x = FrontX;
	m_vFront.z = FrontZ;
	m_vPos.x = PositionX;
	m_vPos.z = PositionZ;
};

// RotateY - Rotates the matrix by "fRadians" around the y axis.
FINLINE void CFMtx43A::RotateZ( const f32 fRadians ) {
	// Precalculate sin and cos for efficiency
	f32	fSin = fmath_Sin( fRadians );
	f32	fCos = fmath_Cos( fRadians );

	// Calculate new coords
	f32	RightX( (m_vRight.x	* fCos)	+ (m_vRight.y * -fSin) );
	f32	UpX( (m_vUp.x	* fCos)	+ (m_vUp.y	* -fSin) );
	f32	FrontX( (m_vFront.x	* fCos)	+ (m_vFront.y * -fSin) );

	f32	RightY( (m_vRight.x	* fSin)	+ (m_vRight.y * fCos) );
	f32	UpY( (m_vUp.x	* fSin)	+ (m_vUp.y	* fCos) );
	f32	FrontY( (m_vFront.x	* fSin)	+ (m_vFront.y * fCos) );

	f32	PositionX( (m_vPos.x * fCos)	+ (m_vPos.y * -fSin) );
	f32	PositionY( (m_vPos.x * fSin)	+ (m_vPos.y * fCos) );

	// Assign new coords
	m_vRight.x = RightX;
	m_vRight.y = RightY;
	m_vUp.x	 = UpX;
	m_vUp.y	 = UpY;
	m_vFront.x = FrontX;
	m_vFront.y = FrontY;
	m_vPos.x = PositionX;
	m_vPos.y = PositionY;
};

// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43A::SetRotationX( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43A::SetRotationY( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43A::SetRotationZ( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43A::SetRotationX( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vUp.z *= fScale;
	m_vUp.y *= fScale;
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43A::SetRotationY( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vFront.x *= fScale;
	m_vFront.z *= fScale;
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43A::SetRotationZ( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vRight.y *= fScale;
	m_vRight.x *= fScale;
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

// NOTE: the Pitch, Yaw and Roll are negated, I assume because it is left handed.
FINLINE void CFMtx43A::GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const {
	rfPitchDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vUp.z, m_vUp.y ) );
	rfYawDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vFront.x, m_vFront.z ) );
	rfRollDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vRight.y, m_vRight.x ) );
}



//--------------------------------------------------------------------
// Port additions (see fdx8gcmath_vec.inl).
//--------------------------------------------------------------------

// this = rM * this. (Mul(rM) is this = this * rM.) The destination aliases an
// operand, so go through a temporary.
FINLINE CFMtx43A &CFMtx43A::RevMul( const CFMtx43A &rM ) {
	CFMtx43A mtxTemp;
	mtxTemp.Mul( rM, *this );
	return Set( mtxTemp );
}

#endif	// FANG_WINGC
