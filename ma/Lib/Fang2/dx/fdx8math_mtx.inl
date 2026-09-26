//////////////////////////////////////////////////////////////////////////////////////
// fmath_mtx.inl - Fang matrix library.
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



//--------------------------------------------------------------------
// CFMtx33 Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx33::CFMtx33( void ) {}
FINLINE CFMtx33::CFMtx33( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront ) { m_vRight=vRight; m_vUp=vUp; m_vFront=vFront; }
FINLINE CFMtx33::CFMtx33( const CFMtx33 &m ) { a[0]=m.a[0]; a[1]=m.a[1]; a[2]=m.a[2]; a[3]=m.a[3]; a[4]=m.a[4]; a[5]=m.a[5]; a[6]=m.a[6]; a[7]=m.a[7]; a[8]=m.a[8]; }
FINLINE CFMtx33::CFMtx33( const CFMtx43 &m ) { *this = m.m33; }
FINLINE CFMtx33::CFMtx33( const CFMtx44 &m ) { m_vX=m.m_vX; m_vY=m.m_vY; m_vZ=m.m_vZ; }
FINLINE CFMtx33::CFMtx33( const f32 &f00, const f32 &f01, const f32 &f02, const f32 &f10, const f32 &f11, const f32 &f12, const f32 &f20, const f32 &f21, const f32 &f22 ) {
	aa[0][0]=f00; aa[0][1]=f01; aa[0][2]=f02; 
	aa[1][0]=f10; aa[1][1]=f11; aa[1][2]=f12;
	aa[2][0]=f20; aa[2][1]=f21; aa[2][2]=f22;
}
FINLINE CFMtx33 &CFMtx33::operator = ( const CFMtx33 &m ) { m_vRight=m.m_vRight; m_vUp=m.m_vUp; m_vFront=m.m_vFront; return *this; }
FINLINE CFMtx33 &CFMtx33::operator = ( const CFMtx43 &m ) { *this = m.m33; return *this; }
FINLINE CFMtx33 &CFMtx33::operator += ( const CFMtx33 &m ) { m_vX+=m.m_vX; m_vY+=m.m_vY; m_vZ+=m.m_vZ; return *this; }
FINLINE CFMtx33 &CFMtx33::operator -= ( const CFMtx33 &m ) { m_vX-=m.m_vX; m_vY-=m.m_vY; m_vZ-=m.m_vZ; return *this; }
FINLINE CFMtx33 &CFMtx33::operator *= ( const f32 &fS ) { m_vX*=fS; m_vY*=fS; m_vZ*=fS; return *this; }
FINLINE CFMtx33 &CFMtx33::operator *= ( const CFMtx33 &m ) {
	CFMtx33 Mtx;
	Mtx.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	Mtx.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	Mtx.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	Mtx.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	Mtx.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	Mtx.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	Mtx.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	Mtx.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	Mtx.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	*this = Mtx;
	return *this;
}
FINLINE CFMtx33 CFMtx33::operator + ( const CFMtx33 &m ) const { return CFMtx33( m_vX+m.m_vX, m_vY+m.m_vY, m_vZ+m.m_vZ ); }
FINLINE CFMtx33 CFMtx33::operator - ( const CFMtx33 &m ) const { return CFMtx33( m_vX-m.m_vX, m_vY-m.m_vY, m_vZ-m.m_vZ ); }
FINLINE CFMtx33 CFMtx33::operator * ( const CFMtx33 &m ) const {
	CFMtx33 Mtx;
	Mtx.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	Mtx.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	Mtx.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	Mtx.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	Mtx.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	Mtx.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	Mtx.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	Mtx.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	Mtx.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	return Mtx;
}
FINLINE CFMtx33 CFMtx33::operator * ( const f32 &fS ) const { return CFMtx33( m_vX*fS, m_vY*fS, m_vZ*fS ); }
FINLINE CFVec3 CFMtx33::operator * ( const CFVec3 &v ) const {
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z );
}
FINLINE CFVec4 CFMtx33::operator * ( const CFVec4 &v ) const {
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z,
					0 );
}
FINLINE BOOL CFMtx33::operator == ( const CFMtx33 &m ) const { return (m_vX==m.m_vX) && (m_vY==m.m_vY) && (m_vZ==m.m_vZ); }
FINLINE BOOL CFMtx33::operator != ( const CFMtx33 &m ) const { return (m_vX!=m.m_vX) || (m_vY!=m.m_vY) || (m_vZ!=m.m_vZ); }
FINLINE CFMtx33 &CFMtx33::Zero( void ) { m_vX.Zero(); m_vY.Zero(); m_vZ.Zero(); return *this; }
FINLINE CFMtx33 &CFMtx33::Identity( void ) { m_vX.Set(1.0f, 0.0f, 0.0f); m_vY.Set(0.0f, 1.0f, 0.0f); m_vZ.Set(0.0f, 0.0f, 1.0f); return *this; }
FINLINE CFMtx33 &CFMtx33::Transpose( CFMtx33 &rDestMtx ) const {
	rDestMtx.aa[0][0] = aa[0][0];
	rDestMtx.aa[1][0] = aa[0][1];
	rDestMtx.aa[2][0] = aa[0][2];
	rDestMtx.aa[0][1] = aa[1][0];
	rDestMtx.aa[1][1] = aa[1][1];
	rDestMtx.aa[2][1] = aa[1][2];
	rDestMtx.aa[0][2] = aa[2][0];
	rDestMtx.aa[1][2] = aa[2][1];
	rDestMtx.aa[2][2] = aa[2][2];
	return rDestMtx;
}
FINLINE CFMtx33 &CFMtx33::Transpose( void ) {
	CFMtx33 NewMtx = *this;
	NewMtx.Transpose( *this );
	return *this;
}

FINLINE CFMtx33 &CFMtx33::UnitizeWithUniformScale( void ) {
	f32 fOOScale = m_vRight.InvMag();
	m_vRight *= fOOScale;
	m_vUp *= fOOScale;
	m_vFront *= fOOScale;

	return *this;
}




//--------------------------------------------------------------------
// CFMtx43 Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx43::CFMtx43( void ) {}
FINLINE CFMtx43::CFMtx43( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront, const CFVec3 &vPos ) { m_vRight=vRight; m_vUp=vUp; m_vFront=vFront; m_vPos=vPos; }
FINLINE CFMtx43::CFMtx43( const CFMtx33 &m ) { this->m33 = m; }
FINLINE CFMtx43::CFMtx43( const CFMtx43 &m ) { *this = m; }
FINLINE CFMtx43::CFMtx43( const CFMtx44 &m ) { m_vX=m.m_vX; m_vY=m.m_vY; m_vZ=m.m_vZ; m_vPos=m.m_vPos; }
FINLINE CFMtx43::CFMtx43( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 ) {
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; 
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32;
}
FINLINE CFMtx43 &CFMtx43::operator = ( const CFMtx33 &m ) { this->m33 = m; return *this; }
FINLINE CFMtx43 &CFMtx43::operator = ( const CFMtx43 &m ) { a[0]=m.a[0]; a[1]=m.a[1]; a[2]=m.a[2]; a[3]=m.a[3]; a[4]=m.a[4]; a[5]=m.a[5]; a[6]=m.a[6]; a[7]=m.a[7]; a[8]=m.a[8]; a[9]=m.a[9]; a[10]=m.a[10]; a[11]=m.a[11]; return *this; }
FINLINE CFMtx43 &CFMtx43::operator = ( const CFMtx44 &m ) { m_vRight=m.m_vRight; m_vUp=m.m_vUp; m_vFront=m.m_vFront; m_vPos=m.m_vPos; return *this; }
FINLINE CFMtx43 &CFMtx43::operator += ( const CFMtx43 &m ) { m_vX+=m.m_vX; m_vY+=m.m_vY; m_vZ+=m.m_vZ; m_vPos+=m.m_vPos; return *this; }
FINLINE CFMtx43 &CFMtx43::operator -= ( const CFMtx43 &m ) { m_vX-=m.m_vX; m_vY-=m.m_vY; m_vZ-=m.m_vZ; m_vPos-=m.m_vPos; return *this; }
FINLINE CFMtx43 &CFMtx43::operator *= ( const f32 &fS ) { m_vX*=fS; m_vY*=fS; m_vZ*=fS; m_vPos*=fS; return *this; }
FINLINE CFMtx43 &CFMtx43::operator *= ( const CFMtx43 &m ) {
	CFMtx43 Mtx;
	Mtx.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	Mtx.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	Mtx.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	Mtx.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	Mtx.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	Mtx.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	Mtx.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	Mtx.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	Mtx.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	Mtx.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x;
	Mtx.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y;
	Mtx.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z;
	*this = Mtx;
	return *this;
}
FINLINE CFMtx43 CFMtx43::operator + ( const CFMtx43 &m ) const { return CFMtx43( m_vX+m.m_vX, m_vY+m.m_vY, m_vZ+m.m_vZ, m_vPos+m.m_vPos ); }
FINLINE CFMtx43 CFMtx43::operator - ( const CFMtx43 &m ) const { return CFMtx43( m_vX-m.m_vX, m_vY-m.m_vY, m_vZ-m.m_vZ, m_vPos-m.m_vPos ); }
FINLINE CFMtx43 CFMtx43::operator * ( const CFMtx43 &m ) const {
	CFMtx43 Mtx;
	Mtx.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	Mtx.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	Mtx.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	Mtx.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	Mtx.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	Mtx.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	Mtx.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	Mtx.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	Mtx.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	Mtx.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x;
	Mtx.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y;
	Mtx.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z;
	return Mtx;
}
FINLINE CFMtx43 CFMtx43::operator * ( const f32 &fS ) const { return CFMtx43( m_vX*fS, m_vY*fS, m_vZ*fS, m_vPos*fS ); }
FINLINE CFVec3 CFMtx43::MultPoint( const CFVec3 &v ) const {
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z );
}
FINLINE CFVec4 CFMtx43::MultPoint( const CFVec4 &v ) const {
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x*v.w,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y*v.w,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z*v.w,
					0 );
}
FINLINE CFVec3 CFMtx43::MultDir( const CFVec3 &v ) const {
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z );
}
FINLINE CFVec4 CFMtx43::MultDir( const CFVec4 &v ) const {
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z,
					0 );
}
FINLINE BOOL CFMtx43::operator == ( const CFMtx43 &m ) const { return (m_vX==m.m_vX) && (m_vY==m.m_vY) && (m_vZ==m.m_vZ) && (m_vPos==m.m_vPos); }
FINLINE BOOL CFMtx43::operator != ( const CFMtx43 &m ) const { return (m_vX!=m.m_vX) || (m_vY!=m.m_vY) || (m_vZ!=m.m_vZ) || (m_vPos!=m.m_vPos); }
FINLINE BOOL CFMtx43::TestTolerance( const CFMtx43 &m, f32 fTolerance/*=FMATH_POS_EPSILON*/ ) const {
	u32 i, *pnMtx1, *pnMtx2;
	f32 fDiff, fPosTolerance, fNegTolerance;

	pnMtx1 = (u32 *)&a[0];
	pnMtx2 = (u32 *)&m.a[0];

	if( fTolerance < 0.0f ) {
		fPosTolerance = -fTolerance;
		fNegTolerance = fTolerance;
	} else {
		fPosTolerance = fTolerance;
		fNegTolerance = -fTolerance;
	}

    for( i=0; i < 12; i++ ) {
		// first do a bitwise compare
		if( pnMtx1[i] == pnMtx2[i] )  {
			// these elements are the same, keep looking for differences
			continue;
		}
		
		// next do an epsilon compare
		fDiff = a[i] - m.a[i];
		if( fDiff > fPosTolerance || fDiff < fNegTolerance ) {
			return FALSE;
		}		
    }
	return TRUE;
}
FINLINE CFMtx43 &CFMtx43::Zero( void ) { m_vX.Zero(); m_vY.Zero(); m_vZ.Zero(); m_vPos.Zero(); return *this; }
FINLINE CFMtx43 &CFMtx43::Identity( void ) { m_vX.Set(1.0f, 0.0f, 0.0f); m_vY.Set(0.0f, 1.0f, 0.0f); m_vZ.Set(0.0f, 0.0f, 1.0f); m_vPos.Zero(); return *this; }

// RotateX - Rotates the matrix by "fRadians" around the X axis.
FINLINE void CFMtx43::RotateX( const f32 fRadians ) {
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
FINLINE void CFMtx43::RotateY( const f32 fRadians ) {
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
FINLINE void CFMtx43::RotateZ( const f32 fRadians ) {
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
FINLINE void CFMtx43::SetRotationX( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43::SetRotationY( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43::SetRotationZ( const f32 fRadians ) {
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43::SetRotationX( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vUp.z *= fScale;
	m_vUp.y *= fScale;
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43::SetRotationY( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vFront.x *= fScale;
	m_vFront.z *= fScale;
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43::SetRotationZ( const f32 fRadians, f32 fScale ) {
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vRight.y *= fScale;
	m_vRight.x *= fScale;
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

FINLINE void CFMtx43::SetPitchYawRoll( f32 &rfPitchRadians, f32 &rfYawRadians, f32 &rfRollRadians ) {
	// Precompute the sin and cos values...
	f32 fPitchCos( fmath_Cos( rfPitchRadians ) );
	f32 fPitchSin( fmath_Sin( rfPitchRadians ) );
	f32 fYawCos( fmath_Cos( rfYawRadians ) );
	f32 fYawSin( fmath_Sin( rfYawRadians ) );
	f32 fRollCos( fmath_Cos( rfRollRadians ) );
	f32 fRollSin( fmath_Sin( rfRollRadians ) );

	// Init rotation matrices to identity matrix...
	CFMtx43 PitchMat( m_IdentityMtx );
	CFMtx43 YawMat( m_IdentityMtx );
	CFMtx43 RollMat( m_IdentityMtx );
	
	// Rotate around the X axis...
	PitchMat.m_vFront.y = -fPitchSin;
	PitchMat.m_vFront.z = fPitchCos;
	PitchMat.m_vUp.y = fPitchCos;
	PitchMat.m_vUp.z = fPitchSin;

	// Rotate around the Y axis...
	YawMat.m_vFront.x = fYawSin;
	YawMat.m_vFront.z = fYawCos;
	YawMat.m_vRight.x = fYawCos;
	YawMat.m_vRight.z = -fYawSin;

	// Rotate around the Z axis...
	RollMat.m_vRight.x = fRollCos;
	RollMat.m_vRight.y = fRollSin;
	RollMat.m_vUp.x = -fRollSin;
	RollMat.m_vUp.y = fRollCos;

	// Concatenates the 3 rotations...
	*this = RollMat * YawMat * PitchMat;
}

// NOTE: the Pitch, Yaw and Roll are negated, I assume because it is left handed.
FINLINE void CFMtx43::GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const {
	rfPitchDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vUp.z, m_vUp.y ) );
	rfYawDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vFront.x, m_vFront.z ) );
	rfRollDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vRight.y, m_vRight.x ) );
}



//--------------------------------------------------------------------
// CFMtx44 Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx44::CFMtx44( void ) {}
FINLINE CFMtx44::CFMtx44( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront, const CFVec3 &vPos ) { m_vRight=vRight; m_vUp=vUp; m_vFront=vFront; m_vPos=vPos; }
FINLINE CFMtx44::CFMtx44( const CFVec4 &vRight, const CFVec4 &vUp, const CFVec4 &vFront, const CFVec4 &vPos ) { m_vRight=vRight; m_vUp=vUp; m_vFront=vFront; m_vPos=vPos; }
FINLINE CFMtx44::CFMtx44( const CFMtx33 &m ) { m_vRight=m.m_vRight; m_vUp=m.m_vUp; m_vFront=m.m_vFront; m_vPos.Zero(); }
FINLINE CFMtx44::CFMtx44( const CFMtx43 &m ) { m_vRight=m.m_vRight; m_vUp=m.m_vUp; m_vFront=m.m_vFront; m_vPos=m.m_vPos; }
FINLINE CFMtx44::CFMtx44( const CFMtx44 &m ) { *this = m; }
FINLINE CFMtx44::CFMtx44( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 ) {
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=ff03;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=ff13;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=ff23;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=ff33;
}
FINLINE CFMtx44 &CFMtx44::operator = ( const CFMtx33 &m ) { m_vRight=m.m_vRight; m_vUp=m.m_vUp; m_vFront=m.m_vFront; m_vPos.Zero(); return *this; }
FINLINE CFMtx44 &CFMtx44::operator = ( const CFMtx43 &m ) { m_vRight=m.m_vRight; m_vUp=m.m_vUp; m_vFront=m.m_vFront; m_vPos=m.m_vPos; return *this; }
FINLINE CFMtx44 &CFMtx44::operator = ( const CFMtx44 &m ) { a[0]=m.a[0]; a[1]=m.a[1]; a[2]=m.a[2]; a[3]=m.a[3]; a[4]=m.a[4]; a[5]=m.a[5]; a[6]=m.a[6]; a[7]=m.a[7]; a[8]=m.a[8]; a[9]=m.a[9]; a[10]=m.a[10]; a[11]=m.a[11]; a[12]=m.a[12]; a[13]=m.a[13]; a[14]=m.a[14]; a[15]=m.a[15]; return *this; }
FINLINE CFMtx44 &CFMtx44::operator += ( const CFMtx44 &m ) { m_vX+=m.m_vX; m_vY+=m.m_vY; m_vZ+=m.m_vZ; m_vPos+=m.m_vPos; return *this; }
FINLINE CFMtx44 &CFMtx44::operator -= ( const CFMtx44 &m ) { m_vX-=m.m_vX; m_vY-=m.m_vY; m_vZ-=m.m_vZ; m_vPos-=m.m_vPos; return *this; }
FINLINE CFMtx44 &CFMtx44::operator *= ( const f32 &fS ) { m_vX*=fS; m_vY*=fS; m_vZ*=fS; m_vPos*=fS; return *this; }
FINLINE CFMtx44 &CFMtx44::operator *= ( const CFMtx44 &m ) {
	CFMtx44 Mtx;
	Mtx.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z + m_vPos.x*m.m_vX.w;
	Mtx.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z + m_vPos.y*m.m_vX.w;
	Mtx.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z + m_vPos.z*m.m_vX.w;
	Mtx.m_vX.w = m_vX.w*m.m_vX.x + m_vY.w*m.m_vX.y + m_vZ.w*m.m_vX.z + m_vPos.w*m.m_vX.w;
	Mtx.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z + m_vPos.x*m.m_vY.w;
	Mtx.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z + m_vPos.y*m.m_vY.w;
	Mtx.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z + m_vPos.z*m.m_vY.w;
	Mtx.m_vY.w = m_vX.w*m.m_vY.x + m_vY.w*m.m_vY.y + m_vZ.w*m.m_vY.z + m_vPos.w*m.m_vY.w;
	Mtx.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z + m_vPos.x*m.m_vZ.w;
	Mtx.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z + m_vPos.y*m.m_vZ.w;
	Mtx.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z + m_vPos.z*m.m_vZ.w;
	Mtx.m_vZ.w = m_vX.w*m.m_vZ.x + m_vY.w*m.m_vZ.y + m_vZ.w*m.m_vZ.z + m_vPos.w*m.m_vZ.w;
	Mtx.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x*m.m_vPos.w;
	Mtx.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y*m.m_vPos.w;
	Mtx.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z*m.m_vPos.w;
	Mtx.m_vPos.w = m_vX.w*m.m_vPos.x + m_vY.w*m.m_vPos.y + m_vZ.w*m.m_vPos.z + m_vPos.w*m.m_vPos.w;
	*this = Mtx;
	return *this;
}
FINLINE CFMtx44 CFMtx44::operator + ( const CFMtx44 &m ) const { return CFMtx44( m_vX+m.m_vX, m_vY+m.m_vY, m_vZ+m.m_vZ, m_vPos+m.m_vPos ); }
FINLINE CFMtx44 CFMtx44::operator - ( const CFMtx44 &m ) const { return CFMtx44( m_vX-m.m_vX, m_vY-m.m_vY, m_vZ-m.m_vZ, m_vPos-m.m_vPos ); }
FINLINE CFMtx44 CFMtx44::operator * ( const CFMtx44 &m ) const {
	CFMtx44 Mtx;
	Mtx.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z + m_vPos.x*m.m_vX.w;
	Mtx.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z + m_vPos.y*m.m_vX.w;
	Mtx.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z + m_vPos.z*m.m_vX.w;
	Mtx.m_vX.w = m_vX.w*m.m_vX.x + m_vY.w*m.m_vX.y + m_vZ.w*m.m_vX.z + m_vPos.w*m.m_vX.w;
	Mtx.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z + m_vPos.x*m.m_vY.w;
	Mtx.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z + m_vPos.y*m.m_vY.w;
	Mtx.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z + m_vPos.z*m.m_vY.w;
	Mtx.m_vY.w = m_vX.w*m.m_vY.x + m_vY.w*m.m_vY.y + m_vZ.w*m.m_vY.z + m_vPos.w*m.m_vY.w;
	Mtx.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z + m_vPos.x*m.m_vZ.w;
	Mtx.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z + m_vPos.y*m.m_vZ.w;
	Mtx.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z + m_vPos.z*m.m_vZ.w;
	Mtx.m_vZ.w = m_vX.w*m.m_vZ.x + m_vY.w*m.m_vZ.y + m_vZ.w*m.m_vZ.z + m_vPos.w*m.m_vZ.w;
	Mtx.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x*m.m_vPos.w;
	Mtx.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y*m.m_vPos.w;
	Mtx.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z*m.m_vPos.w;
	Mtx.m_vPos.w = m_vX.w*m.m_vPos.x + m_vY.w*m.m_vPos.y + m_vZ.w*m.m_vPos.z + m_vPos.w*m.m_vPos.w;
	return Mtx;
}
FINLINE CFMtx44 CFMtx44::operator * ( const f32 &fS ) const { return CFMtx43( m_vX*fS, m_vY*fS, m_vZ*fS, m_vPos*fS ); }
FINLINE CFVec3 CFMtx44::MultPoint( const CFVec3 &v ) const {
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z );
}
FINLINE CFVec4 CFMtx44::MultPoint( const CFVec4 &v ) const {
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x*v.w,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y*v.w,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z*v.w,
					m_vX.w*v.x + m_vY.w*v.y + m_vZ.w*v.z + m_vPos.w*v.w );
}
FINLINE CFVec3 CFMtx44::MultDir( const CFVec3 &v ) const {
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z );
}
FINLINE CFVec4 CFMtx44::MultDir( const CFVec4 &v ) const {
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z,
					m_vX.w*v.x + m_vY.w*v.y + m_vZ.w*v.z );
}
FINLINE BOOL CFMtx44::operator == ( const CFMtx44 &m ) const { return (m_vX==m.m_vX) && (m_vY==m.m_vY) && (m_vZ==m.m_vZ) && (m_vPos==m.m_vPos); }
FINLINE BOOL CFMtx44::operator != ( const CFMtx44 &m ) const { return (m_vX!=m.m_vX) || (m_vY!=m.m_vY) || (m_vZ!=m.m_vZ) || (m_vPos!=m.m_vPos); }
FINLINE CFMtx44 &CFMtx44::Zero( void ) { m_vX.Zero(); m_vY.Zero(); m_vZ.Zero(); m_vPos.Zero(); return *this; }
FINLINE CFMtx44 &CFMtx44::Identity( void ) { m_vX.Set(1.0f, 0.0f, 0.0f, 0.0f); m_vY.Set(0.0f, 1.0f, 0.0f, 0.0f); m_vZ.Set(0.0f, 0.0f, 1.0f, 0.0f); m_vPos.Set(0.0f, 0.0f, 0.0f, 1.0f); return *this; }
FINLINE CFMtx44 &CFMtx44::Transpose( CFMtx44 &rDestMtx ) const {
	rDestMtx.aa[0][0] = aa[0][0];
	rDestMtx.aa[1][0] = aa[0][1];
	rDestMtx.aa[2][0] = aa[0][2];
	rDestMtx.aa[3][0] = aa[0][3];
	rDestMtx.aa[0][1] = aa[1][0];
	rDestMtx.aa[1][1] = aa[1][1];
	rDestMtx.aa[2][1] = aa[1][2];
	rDestMtx.aa[3][1] = aa[1][3];
	rDestMtx.aa[0][2] = aa[2][0];
	rDestMtx.aa[1][2] = aa[2][1];
	rDestMtx.aa[2][2] = aa[2][2];
	rDestMtx.aa[3][2] = aa[2][3];
	rDestMtx.aa[0][3] = aa[3][0];
	rDestMtx.aa[1][3] = aa[3][1];
	rDestMtx.aa[2][3] = aa[3][2];
	rDestMtx.aa[3][3] = aa[3][3];
	return rDestMtx;
}
FINLINE CFMtx44 &CFMtx44::Transpose( void ) {
	CFMtx44 NewMtx = *this;
	NewMtx.Transpose( *this );
	return *this;
}



#if !FANG_WINGC

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

#pragma warning( disable : 4035 )
FINLINE BOOL CFMtx44A::operator == ( const CFMtx44A &rM ) const {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm1, [eax+0x10]
		movaps		xmm2, [eax+0x20]
		movaps		xmm3, [eax+0x30]

		mov			eax, rM
		cmpneqps	xmm0, [eax]
		cmpneqps	xmm1, [eax+0x10]
		cmpneqps	xmm2, [eax+0x20]
		cmpneqps	xmm3, [eax+0x30]

		orps		xmm0, xmm1
		orps		xmm0, xmm2
		orps		xmm0, xmm3

		movmskps	eax, xmm0
		sub			eax, 1
		sbb			eax, eax
	}
}
#pragma warning( default : 4035 )

#pragma warning( disable : 4035 )
FINLINE BOOL CFMtx44A::operator != ( const CFMtx44A &rM ) const {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm1, [eax+0x10]
		movaps		xmm2, [eax+0x20]
		movaps		xmm3, [eax+0x30]

		mov			eax, rM
		cmpneqps	xmm0, [eax]
		cmpneqps	xmm1, [eax+0x10]
		cmpneqps	xmm2, [eax+0x20]
		cmpneqps	xmm3, [eax+0x30]

		orps		xmm0, xmm1
		orps		xmm0, xmm2
		orps		xmm0, xmm3

		movmskps	eax, xmm0
	}
}
#pragma warning( default : 4035 )


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

FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM1, const CFMtx44A &rM2 ) {
    __asm {
		mov			edx, rM1
		mov			ecx, rM2
		mov			eax, this

		movlps		xmm0,[ecx+0x00]
		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x0C]
		shufps      xmm3,xmm3,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]
		movaps      xmm7,[edx+0x30]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [eax+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x1C]
		shufps      xmm3,xmm3,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [eax+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x2C]
		shufps      xmm3,xmm3,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [eax+0x20], xmm2


		movlps		xmm0,[ecx+0x30]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x34]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x38]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x3C]
		shufps      xmm3,xmm3,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [eax+0x30], xmm2
	}

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM ) {
    __asm {
		mov			edx, this
		mov			ecx, rM

		movlps		xmm0,[ecx+0x00]
		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x0C]
		shufps      xmm3,xmm3,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]
		movaps      xmm7,[edx+0x30]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [edx+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x1C]
		shufps      xmm3,xmm3,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [edx+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x2C]
		shufps      xmm3,xmm3,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [edx+0x20], xmm2


		movlps		xmm0,[ecx+0x30]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x34]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x38]
		shufps      xmm2,xmm2,0x00
		movlps	    xmm3,[ecx+0x3C]
		shufps      xmm3,xmm3,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6
		mulps       xmm3,xmm7

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		addps       xmm2,xmm3
		movaps      [edx+0x30], xmm2
	}

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM, const f32 &fVal ) {
	FDX8Math_fTemp = fVal;

	__asm {
		movss	xmm1, FDX8Math_fTemp
		shufps	xmm1, xmm1, 00h

		movaps	xmm0, xmm1
		mov		edx, rM
		mulps	xmm0, [edx + 16*0]
		mov		eax, this
		movaps	[eax + 16*0], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [edx + 16*1]
		movaps	[eax + 16*1], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [edx + 16*2]
		movaps	[eax + 16*2], xmm0

		mulps	xmm1, [edx + 16*3]
		movaps	[eax + 16*3], xmm1
	}

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Mul( const f32 &fVal ) {
	FDX8Math_fTemp = fVal;

	__asm {
		movss	xmm1, FDX8Math_fTemp
		shufps	xmm1, xmm1, 00h

		movaps	xmm0, xmm1
		mov		eax, this
		mulps	xmm0, [eax + 16*0]
		movaps	[eax + 16*0], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [eax + 16*1]
		movaps	[eax + 16*1], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [eax + 16*2]
		movaps	[eax + 16*2], xmm0

		mulps	xmm1, [eax + 16*3]
		movaps	[eax + 16*3], xmm1
	}

	return *this;
}

FINLINE CFVec3A &CFMtx44A::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, [ecx+30h]
		addps		xmm2, xmm1
		addps		xmm2, xmm3

		andps		xmm2, FDX8Math_nnnnMask_XYZ1_W0

		mov 		eax, rRV
		movaps		[eax], xmm2
	}

	return rRV;
}

FINLINE CFVec4A &CFMtx44A::MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const {
	__asm {
		mov 		eax,  rV
		mov 		ecx,  this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax]
		shufps		xmm1, xmm1, 55h
		mulps		xmm1, [ecx+10h]

		movhps		xmm0, [eax]
		shufps		xmm0, xmm0, 0AAh
		mulps		xmm0, [ecx+20h]

		addps		xmm2, xmm1

		movhps		xmm1, [eax]
		shufps		xmm1, xmm1, 0FFh
		mulps		xmm1, [ecx+30h]

		addps		xmm2, xmm0
		addps		xmm2, xmm1

		mov			eax, rRV
		movaps		[eax], xmm2
	}

	return rRV;
}

FINLINE CFVec3A &CFMtx44A::MulPoint( CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, [ecx+30h]
		addps		xmm2, xmm1
		addps		xmm2, xmm3

		andps		xmm2, FDX8Math_nnnnMask_XYZ1_W0

		movaps		[eax], xmm2
	}

	return rV;
}

FINLINE CFVec4A &CFMtx44A::MulPoint( CFVec4A &rV ) const {
	__asm {
		mov 		eax,  rV
		mov 		ecx,  this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax]
		shufps		xmm1, xmm1, 55h
		mulps		xmm1, [ecx+10h]

		movhps		xmm0, [eax]
		shufps		xmm0, xmm0, 0AAh
		mulps		xmm0, [ecx+20h]

		addps		xmm2, xmm1

		movhps		xmm1, [eax]
		shufps		xmm1, xmm1, 0FFh
		mulps		xmm1, [ecx+30h]

		addps		xmm2, xmm0
		addps		xmm2, xmm1

		movaps		[eax], xmm2
	}

	return rV;
}

FINLINE CFVec3A &CFMtx44A::MulDir( CFVec3A &rRV, const CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, xmm1
		addps		xmm2, xmm3

		andps		xmm2, FDX8Math_nnnnMask_XYZ1_W0

		mov 		eax, rRV
		movaps		[eax], xmm2
	}

	return rRV;
}

FINLINE CFVec4A &CFMtx44A::MulDir( CFVec4A &rRV, const CFVec4A &rV ) const {
	__asm {
		mov 		eax,  rV
		mov 		ecx,  this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax]
		shufps		xmm1, xmm1, 55h
		mulps		xmm1, [ecx+10h]

		movhps		xmm0, [eax]
		shufps		xmm0, xmm0, 0AAh
		mulps		xmm0, [ecx+20h]

		addps		xmm2, xmm1
		addps		xmm2, xmm0

		mov			eax, rRV
		movaps		[eax], xmm2
	}

	return rRV;
}

FINLINE CFVec3A &CFMtx44A::MulDir( CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, xmm1
		addps		xmm2, xmm3

		andps		xmm2, FDX8Math_nnnnMask_XYZ1_W0

		movaps		[eax], xmm2
	}

	return rV;
}

FINLINE CFVec4A &CFMtx44A::MulDir( CFVec4A &rV ) const {
	__asm {
		mov 		eax,  rV
		mov 		ecx,  this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax]
		shufps		xmm1, xmm1, 55h
		mulps		xmm1, [ecx+10h]

		movhps		xmm0, [eax]
		shufps		xmm0, xmm0, 0AAh
		mulps		xmm0, [ecx+20h]

		addps		xmm2, xmm1
		addps		xmm2, xmm0

		movaps		[eax], xmm2
	}

	return rV;
}

FINLINE CFMtx44A &CFMtx44A::ReceiveTranspose( const CFMtx44A &rM ) {
	__asm {
		mov			ecx, rM
		mov			eax, this
		movaps		xmm0, [ecx]
		movaps		xmm2, [ecx+10h]
		movaps		xmm3, [ecx+20h]
		movaps		xmm5, [ecx+30h]
		movaps		xmm1, xmm0
		movaps		xmm4, xmm3
		unpcklps	xmm0, xmm2
		unpckhps	xmm1, xmm2
		unpcklps	xmm3, xmm5
		unpckhps	xmm4, xmm5
		movlps		[eax], xmm0
		movlps		[eax+8], xmm3
		movhps		[eax+10h], xmm0
		movhps		[eax+18h], xmm3
		movlps		[eax+20h], xmm1
		movlps		[eax+28h], xmm4
		movhps		[eax+30h], xmm1
		movhps		[eax+38h], xmm4
	}

	return *this;
}

FINLINE CFMtx44A &CFMtx44A::Transpose( void ) {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm2, [eax+10h]
		movaps		xmm3, [eax+20h]
		movaps		xmm5, [eax+30h]
		movaps		xmm1, xmm0
		movaps		xmm4, xmm3
		unpcklps	xmm0, xmm2
		unpckhps	xmm1, xmm2
		unpcklps	xmm3, xmm5
		unpckhps	xmm4, xmm5
		movlps		[eax], xmm0
		movlps		[eax+8], xmm3
		movhps		[eax+10h], xmm0
		movhps		[eax+18h], xmm3
		movlps		[eax+20h], xmm1
		movlps		[eax+28h], xmm4
		movhps		[eax+30h], xmm1
		movhps		[eax+38h], xmm4
	}

	return *this;
}

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

#pragma warning( disable : 4035 )
FINLINE BOOL CFMtx43A::operator == ( const CFMtx43A &rM ) const {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm1, [eax+0x10]
		movaps		xmm2, [eax+0x20]
		movaps		xmm3, [eax+0x30]

		mov			eax, rM
		cmpneqps	xmm0, [eax]
		cmpneqps	xmm1, [eax+0x10]
		cmpneqps	xmm2, [eax+0x20]
		cmpneqps	xmm3, [eax+0x30]

		orps		xmm0, xmm1
		orps		xmm0, xmm2
		orps		xmm0, xmm3

		movmskps	eax, xmm0
		sub			eax, 1
		sbb			eax, eax
	}
}
#pragma warning( default : 4035 )

#pragma warning( disable : 4035 )
FINLINE BOOL CFMtx43A::operator != ( const CFMtx43A &rM ) const {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm1, [eax+0x10]
		movaps		xmm2, [eax+0x20]
		movaps		xmm3, [eax+0x30]

		mov			eax, rM
		cmpneqps	xmm0, [eax]
		cmpneqps	xmm1, [eax+0x10]
		cmpneqps	xmm2, [eax+0x20]
		cmpneqps	xmm3, [eax+0x30]

		orps		xmm0, xmm1
		orps		xmm0, xmm2
		orps		xmm0, xmm3

		movmskps	eax, xmm0
	}
}
#pragma warning( default : 4035 )


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

FINLINE CFMtx43A &CFMtx43A::Mul( const CFMtx43A &rM1, const CFMtx43A &rM2 ) {
	__asm {
		mov			edx, rM1
		mov			ecx, rM2
		mov			eax, this

		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [eax+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [eax+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [eax+0x20], xmm2


		movlps		xmm0,[ecx+0x30]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x34]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x38]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		addps       xmm0,[edx+0x30]
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [eax+0x30], xmm2
	}

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Mul( const CFMtx43A &rM ) {
	__asm {
		mov			edx, this
		mov			ecx, rM

		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [edx+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [edx+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [edx+0x20], xmm2


		movlps		xmm0,[ecx+0x30]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x34]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x38]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		addps       xmm0,[edx+0x30]
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [edx+0x30], xmm2
	}

	return *this;
}


FINLINE CFMtx43A &CFMtx43A::RevMul( const CFMtx43A &rM ) {
	__asm {
		mov			ecx, this
		mov			edx, rM

		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [ecx+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [ecx+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [ecx+0x20], xmm2


		movlps		xmm0,[ecx+0x30]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x34]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x38]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		addps       xmm0,[edx+0x30]
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [ecx+0x30], xmm2
	}

	return *this;
}

#if 0
FINLINE CFMtx43A &CFMtx43A::MulTranspose33( const CFMtx43A &rM1, const CFMtx43A &rTM2 ) {
	__asm {
		mov			edx, rM1
		mov			ecx, rM2
		mov			eax, this

		movaps		xmm0, [edx+0x00]
		mulps		xmm0, [ecx+0x00]

		movaps		xmm1, [edx+0x10]
		mulps		xmm1, [ecx+0x10]

		movaps		xmm2, [edx+0x20]
		mulps		xmm2, [ecx+0x20]

		addps		xmm0, xmm1
		addps		xmm0, xmm2
	}

	return *this;
}
#endif

FINLINE CFMtx43A &CFMtx43A::Mul33( const CFMtx43A &rM1, const CFMtx43A &rM2 ) {
	__asm {
		mov			edx, rM1
		mov			ecx, rM2
		mov			eax, this

		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [eax+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [eax+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [eax+0x20], xmm2
	}

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Mul33( const CFMtx43A &rM ) {
	__asm {
		mov			edx, this
		mov			ecx, rM

		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [edx+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [edx+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [edx+0x20], xmm2
	}

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::RevMul33( const CFMtx43A &rM ) {
	__asm {
		mov			ecx, this
		mov			edx, rM

		movlps		xmm0,[ecx+0x00]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x04]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x08]
		shufps      xmm2,xmm2,0x00

		movaps      xmm4,[edx+0x00]
		movaps      xmm5,[edx+0x10]
		movaps      xmm6,[edx+0x20]

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [ecx+0x00], xmm2


		movlps		xmm0,[ecx+0x10]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x14]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x18]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [ecx+0x10], xmm2


		movlps		xmm0,[ecx+0x20]
		shufps      xmm0,xmm0,0x00
		movlps		xmm1,[ecx+0x24]
		shufps      xmm1,xmm1,0x00
		movlps		xmm2,[ecx+0x28]
		shufps      xmm2,xmm2,0x00

		mulps       xmm0,xmm4
		mulps       xmm1,xmm5
		mulps       xmm2,xmm6

		addps       xmm1,xmm0
		addps       xmm2,xmm1
		movaps      [ecx+0x20], xmm2
	}

	return *this;
}


FINLINE CFMtx43A &CFMtx43A::Mul( const CFMtx43A &rM, const f32 &fVal ) {
	FDX8Math_fTemp = fVal;

	__asm {
		movss	xmm1, FDX8Math_fTemp
		shufps	xmm1, xmm1, 00h

		movaps	xmm0, xmm1
		mov		edx, rM
		mulps	xmm0, [edx + 16*0]
		mov		eax, this
		movaps	[eax + 16*0], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [edx + 16*1]
		movaps	[eax + 16*1], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [edx + 16*2]
		movaps	[eax + 16*2], xmm0

		mulps	xmm1, [edx + 16*3]
		movaps	[eax + 16*3], xmm1
	}

	return *this;
}


FINLINE CFMtx43A &CFMtx43A::Mul( const f32 &fVal ) {
	FDX8Math_fTemp = fVal;

	__asm {
		movss	xmm1, FDX8Math_fTemp
		shufps	xmm1, xmm1, 00h

		movaps	xmm0, xmm1
		mov		eax, this
		mulps	xmm0, [eax + 16*0]
		movaps	[eax + 16*0], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [eax + 16*1]
		movaps	[eax + 16*1], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [eax + 16*2]
		movaps	[eax + 16*2], xmm0

		mulps	xmm1, [eax + 16*3]
		movaps	[eax + 16*3], xmm1
	}

	return *this;
}


FINLINE CFMtx43A &CFMtx43A::Mul33( const CFMtx43A &rM, const f32 &fVal ) {
	FDX8Math_fTemp = fVal;

	__asm {
		movss	xmm1, FDX8Math_fTemp
		shufps	xmm1, xmm1, 00h

		movaps	xmm0, xmm1
		mov		edx, rM
		mulps	xmm0, [edx + 16*0]
		mov		eax, this
		movaps	[eax + 16*0], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [edx + 16*1]
		movaps	[eax + 16*1], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [edx + 16*2]
		movaps	[eax + 16*2], xmm0
	}

	return *this;
}


FINLINE CFMtx43A &CFMtx43A::Mul33( const f32 &fVal ) {
	FDX8Math_fTemp = fVal;

	__asm {
		movss	xmm1, FDX8Math_fTemp
		shufps	xmm1, xmm1, 00h

		movaps	xmm0, xmm1
		mov		eax, this
		mulps	xmm0, [eax + 16*0]
		movaps	[eax + 16*0], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [eax + 16*1]
		movaps	[eax + 16*1], xmm0

		movaps	xmm0, xmm1
		mulps	xmm0, [eax + 16*2]
		movaps	[eax + 16*2], xmm0
	}

	return *this;
}


FINLINE CFVec3A &CFMtx43A::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, [ecx+30h]
		addps		xmm2, xmm1
		addps		xmm2, xmm3

		mov 		eax, rRV
		movaps		[eax], xmm2
	}

	return rRV;
}

FINLINE CFVec3&  CFMtx43A::MulPoint( CFVec3 &rRV, const CFVec3 &rV ) const 
{
	__asm 
	{
		mov 		eax, rV
		mov 		ecx, this

		movss		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movss		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movss		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, [ecx+30h]
		addps		xmm2, xmm1
		addps		xmm2, xmm3

		mov 		eax, rRV
		movlps		[eax], xmm2
		unpckhps	xmm2, xmm2
		movss		[eax+8], xmm2
	}
	return rRV;
}
FINLINE CFVec3A &CFMtx43A::MulPoint( CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, [ecx+30h]
		addps		xmm2, xmm1
		addps		xmm2, xmm3

		movaps		[eax], xmm2
	}

	return rV;
}

FINLINE CFVec3A &CFMtx43A::MulDir( CFVec3A &rRV, const CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, xmm1
		addps		xmm2, xmm3

		mov 		eax, rRV
		movaps		[eax], xmm2
	}

	return rRV;
}

FINLINE CFVec3A &CFMtx43A::MulDir( CFVec3A &rV ) const {
	__asm {
		mov 		eax, rV
		mov 		ecx, this

		movlps		xmm2, [eax]
		shufps		xmm2, xmm2, 0
		mulps		xmm2, [ecx]

		movlps		xmm1, [eax+4]
		shufps		xmm1, xmm1, 0
		mulps		xmm1, [ecx+10h]

		movlps		xmm3, [eax+8]
		shufps		xmm3, xmm3, 0
		mulps		xmm3, [ecx+20h]

		addps		xmm2, xmm1
		addps		xmm2, xmm3

		movaps		[eax], xmm2
	}

	return rV;
}

FINLINE CFMtx43A &CFMtx43A::ReceiveAffineInverse( const CFMtx43A &rM, BOOL bProvidedMatrixMayNotHaveUnitScale ) {
	CFVec3A NegPos;

	ReceiveTranspose33( rM );

	if( bProvidedMatrixMayNotHaveUnitScale ) {
		NegPos.Set( rM.m_vRight.InvMagSq() );

		m_vRight.Mul( NegPos );
		m_vUp.Mul( NegPos );
		m_vFront.Mul( NegPos );
	}

	NegPos.ReceiveNegative( rM.m_vPos );
	MulDir( m_vPos, NegPos );

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::AffineInvert( BOOL bProvidedMatrixMayNotHaveUnitScale ) {
	Transpose33();

	if( bProvidedMatrixMayNotHaveUnitScale ) {
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
	__asm {
		mov			ecx, rM
		mov			eax, this
		movaps		xmm0, [ecx]
		movaps		xmm2, [ecx+10h]
		movaps		xmm3, [ecx+20h]
		xorps		xmm5, xmm5
		movaps		xmm1, xmm0
		movaps		xmm4, xmm3
		unpcklps	xmm0, xmm2
		unpckhps	xmm1, xmm2
		unpcklps	xmm3, xmm5
		unpckhps	xmm4, xmm5
		movlps		[eax], xmm0
		movlps		[eax+8], xmm3
		movhps		[eax+10h], xmm0
		movhps		[eax+18h], xmm3
		movlps		[eax+20h], xmm1
		movlps		[eax+28h], xmm4
	}

	return *this;
}

FINLINE CFMtx43A &CFMtx43A::Transpose33( void ) {
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		movaps		xmm2, [eax+10h]
		movaps		xmm3, [eax+20h]
		xorps		xmm5, xmm5
		movaps		xmm1, xmm0
		movaps		xmm4, xmm3
		unpcklps	xmm0, xmm2
		unpckhps	xmm1, xmm2
		unpcklps	xmm3, xmm5
		unpckhps	xmm4, xmm5
		movlps		[eax], xmm0
		movlps		[eax+8], xmm3
		movhps		[eax+10h], xmm0
		movhps		[eax+18h], xmm3
		movlps		[eax+20h], xmm1
		movlps		[eax+28h], xmm4
	}

	return *this;
}

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


#endif	// !FANG_WINGC
