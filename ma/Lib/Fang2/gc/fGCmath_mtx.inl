//////////////////////////////////////////////////////////////////////////////////////
// fGCmath_mtx.inl - Fang matrix library.
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
// 02/18/02 Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#if FANG_DEBUG_BUILD
#pragma optimization_level 1 
#pragma global_optimizer on
#endif



//--------------------------------------------------------------------
// CFMtx33 Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx33::CFMtx33( void ) {}

//
//
FINLINE CFMtx33::CFMtx33( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront ) 
{ 
	m_vRight = vRight; 
	m_vUp = vUp; 
	m_vFront = vFront; 
}

//
//
FINLINE CFMtx33::CFMtx33( const CFMtx33 &m ) 
{ 
	a[0] = m.a[0]; a[1] = m.a[1]; a[2] = m.a[2]; 
	a[3] = m.a[3]; a[4] = m.a[4]; a[5] = m.a[5]; 
	a[6] = m.a[6]; a[7] = m.a[7]; a[8] = m.a[8]; 
}

//
//
FINLINE CFMtx33::CFMtx33( const CFMtx43 &m ) 
{ 
	*this = m.m33; 
}

//
//
FINLINE CFMtx33::CFMtx33( const CFMtx44 &m ) 
{ 
	m_vRight = m.m_vRight;
	m_vUp = m.m_vUp;
	m_vFront = m.m_vFront;
}

//
//
FINLINE CFMtx33::CFMtx33( const f32 &f00, const f32 &f01, const f32 &f02, const f32 &f10, const f32 &f11, const f32 &f12, const f32 &f20, const f32 &f21, const f32 &f22 ) 
{
	aa[0][0]=f00; aa[0][1]=f01; aa[0][2]=f02; 
	aa[1][0]=f10; aa[1][1]=f11; aa[1][2]=f12;
	aa[2][0]=f20; aa[2][1]=f21; aa[2][2]=f22;
}

//
//
FINLINE CFMtx33 &CFMtx33::operator = ( const CFMtx33 &m ) 
{ 
	m_vRight = m.m_vRight; 
	m_vUp = m.m_vUp; 
	m_vFront = m.m_vFront; 
	return *this; 
}

//
//
FINLINE CFMtx33 &CFMtx33::operator = ( const CFMtx43 &m ) 
{ 
	*this = m.m33;
	return *this; 
}

//
//
FINLINE CFMtx33 &CFMtx33::operator += ( const CFMtx33 &m ) 
{ 
	m_vRight += m.m_vRight;
	m_vUp += m.m_vUp;
	m_vFront += m.m_vFront;
	return *this;
}

//
//
FINLINE CFMtx33 &CFMtx33::operator -= ( const CFMtx33 &m ) 
{ 
	m_vRight -= m.m_vRight;
	m_vUp -= m.m_vUp;
	m_vFront -= m.m_vFront;
	return *this; 
}

//
//
FINLINE CFMtx33 &CFMtx33::operator *= ( const f32 &fS ) 
{ 
	m_vRight *= fS;
	m_vUp *= fS;
	m_vFront *= fS;
	return *this; 
}

//
//
FINLINE CFMtx33 &CFMtx33::operator *= ( const CFMtx33 &m ) 
{
	CFMtx33 mtxResult;
	mtxResult.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	mtxResult.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	mtxResult.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	mtxResult.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	mtxResult.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	mtxResult.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	mtxResult.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	mtxResult.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	mtxResult.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	*this = mtxResult;
	return *this;
}

//
//
FINLINE CFMtx33 CFMtx33::operator + ( const CFMtx33 &m ) const 
{ 
	return CFMtx33( m_vX+m.m_vX, m_vY+m.m_vY, m_vZ+m.m_vZ ); 
}

//
//
FINLINE CFMtx33 CFMtx33::operator - ( const CFMtx33 &m ) const 
{ 
	return CFMtx33( m_vX-m.m_vX, m_vY-m.m_vY, m_vZ-m.m_vZ ); 
}

//
//
FINLINE CFMtx33 CFMtx33::operator * ( const CFMtx33 &m ) const 
{
	CFMtx33 mtxResult;
	mtxResult.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	mtxResult.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	mtxResult.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	mtxResult.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	mtxResult.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	mtxResult.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	mtxResult.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	mtxResult.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	mtxResult.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	return mtxResult;
}

//
//
FINLINE CFMtx33 CFMtx33::operator * ( const f32 &fS ) const 
{ 
	return CFMtx33( m_vX*fS, m_vY*fS, m_vZ*fS ); 
}

//
//
FINLINE CFVec3 CFMtx33::operator * ( const CFVec3 &v ) const 
{
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z );
}

//
//
FINLINE CFVec4 CFMtx33::operator * ( const CFVec4 &v ) const 
{
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z,
					0 );
}

//
//
FINLINE BOOL CFMtx33::operator == ( const CFMtx33 &m ) const 
{ 
	return (m_vX==m.m_vX) && (m_vY==m.m_vY) && (m_vZ==m.m_vZ); 
}

//
//
FINLINE BOOL CFMtx33::operator != ( const CFMtx33 &m ) const 
{ 
	return (m_vX!=m.m_vX) || (m_vY!=m.m_vY) || (m_vZ!=m.m_vZ); 
}

//
//
FINLINE CFMtx33 &CFMtx33::Zero( void ) 
{ 
	m_vX.Zero(); 
	m_vY.Zero(); 
	m_vZ.Zero(); 
	return *this; 
}

//
//
FINLINE CFMtx33 &CFMtx33::Identity( void ) 
{ 
	m_vX.Set(1.0f, 0.0f, 0.0f); 
	m_vY.Set(0.0f, 1.0f, 0.0f); 
	m_vZ.Set(0.0f, 0.0f, 1.0f); 
	return *this; 
}

//
//
FINLINE CFMtx33 &CFMtx33::Transpose( CFMtx33 &rDestMtx ) const 
{
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

//
//
FINLINE CFMtx33 &CFMtx33::Transpose( void ) 
{
	CFMtx33 NewMtx = *this;
	NewMtx.Transpose( *this );
	return *this;
}



//--------------------------------------------------------------------
// CFMtx43 Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx43::CFMtx43( void ) {}

//
//
FINLINE CFMtx43::CFMtx43( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront, const CFVec3 &vPos ) 
{ 
	m_vRight = vRight; 
	m_vUp = vUp; 
	m_vFront = vFront; 
	m_vPos = vPos; 
}

//
//
FINLINE CFMtx43::CFMtx43( const CFMtx33 &m ) 
{ 
	this->m33 = m; 
}

//
//
FINLINE CFMtx43::CFMtx43( const CFMtx43 &m ) 
{ 
	*this = m; 
}

//
//
FINLINE CFMtx43::CFMtx43( const CFMtx44 &m ) 
{ 
	m_vX = m.m_vX; 
	m_vY = m.m_vY; 
	m_vZ = m.m_vZ; 
	m_vPos = m.m_vPos; 
}

//
//
FINLINE CFMtx43::CFMtx43( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 ) 
{
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; 
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32;
}

//
//
FINLINE CFMtx43 &CFMtx43::operator = ( const CFMtx33 &m ) 
{ 
	this->m33 = m; 
	return *this; 
}

//
//
FINLINE CFMtx43 &CFMtx43::operator = ( const CFMtx43 &m ) 
{ 
	a[0]=m.a[0]; a[1]=m.a[1]; a[2]=m.a[2]; 
	a[3]=m.a[3]; a[4]=m.a[4]; a[5]=m.a[5]; 
	a[6]=m.a[6]; a[7]=m.a[7]; a[8]=m.a[8]; 
	a[9]=m.a[9]; a[10]=m.a[10]; a[11]=m.a[11]; 
	return *this; 
}

//
//
FINLINE CFMtx43 &CFMtx43::operator = ( const CFMtx44 &m ) 
{ 
	m_vRight = m.m_vRight; 
	m_vUp = m.m_vUp; 
	m_vFront = m.m_vFront; 
	m_vPos = m.m_vPos; 
	return *this; 
}

//
//
FINLINE CFMtx43 &CFMtx43::operator += ( const CFMtx43 &m ) 
{ 
	m_vX += m.m_vX; 
	m_vY += m.m_vY; 
	m_vZ += m.m_vZ; 
	m_vPos += m.m_vPos; 
	return *this; 
}

//
//
FINLINE CFMtx43 &CFMtx43::operator -= ( const CFMtx43 &m ) 
{ 
	m_vX -= m.m_vX; 
	m_vY -= m.m_vY; 
	m_vZ -= m.m_vZ; 
	m_vPos -= m.m_vPos; 
	return *this;
}

//
//
FINLINE CFMtx43 &CFMtx43::operator *= ( const f32 &fS ) 
{ 
	m_vX *= fS; 
	m_vY *= fS; 
	m_vZ *= fS; 
	m_vPos *= fS; 
	return *this; 
}

//
//
FINLINE CFMtx43 &CFMtx43::operator *= ( const CFMtx43 &m ) 
{
	CFMtx43 mtxResult;
	mtxResult.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	mtxResult.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	mtxResult.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	mtxResult.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	mtxResult.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	mtxResult.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	mtxResult.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	mtxResult.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	mtxResult.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	mtxResult.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x;
	mtxResult.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y;
	mtxResult.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z;
	*this = mtxResult;
	return *this;
}

//
//
FINLINE CFMtx43 CFMtx43::operator + ( const CFMtx43 &m ) const 
{ 
	return CFMtx43( m_vX+m.m_vX, m_vY+m.m_vY, m_vZ+m.m_vZ, m_vPos+m.m_vPos ); 
}

//
//
FINLINE CFMtx43 CFMtx43::operator - ( const CFMtx43 &m ) const 
{ 
	return CFMtx43( m_vX-m.m_vX, m_vY-m.m_vY, m_vZ-m.m_vZ, m_vPos-m.m_vPos ); 
}

//
//
FINLINE CFMtx43 CFMtx43::operator * ( const CFMtx43 &m ) const 
{
	CFMtx43 Result;
	Result.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z;
	Result.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z;
	Result.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z;
	Result.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z;
	Result.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z;
	Result.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z;
	Result.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z;
	Result.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z;
	Result.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z;
	Result.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x;
	Result.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y;
	Result.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z;
	return Result;
}

//
//
FINLINE CFMtx43 CFMtx43::operator * ( const f32 &fS ) const 
{ 
	return CFMtx43( m_vX*fS, m_vY*fS, m_vZ*fS, m_vPos*fS ); 
}

//
//
FINLINE CFVec3 CFMtx43::MultPoint( const CFVec3 &v ) const 
{
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z );
}

//
//
FINLINE CFVec4 CFMtx43::MultPoint( const CFVec4 &v ) const 
{
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x*v.w,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y*v.w,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z*v.w,
					0 );
}

//
//
FINLINE CFVec3 CFMtx43::MultDir( const CFVec3 &v ) const 
{
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z );
}

//
//
FINLINE CFVec4 CFMtx43::MultDir( const CFVec4 &v ) const 
{
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z,
					0 );
}

//
//
FINLINE BOOL CFMtx43::operator == ( const CFMtx43 &m ) const 
{ 
	return (m_vX==m.m_vX) && (m_vY==m.m_vY) && (m_vZ==m.m_vZ) && (m_vPos==m.m_vPos); 
}

//
//
FINLINE BOOL CFMtx43::operator != ( const CFMtx43 &m ) const 
{ 
	return (m_vX!=m.m_vX) || (m_vY!=m.m_vY) || (m_vZ!=m.m_vZ) || (m_vPos!=m.m_vPos); 
}

//
//
FINLINE BOOL CFMtx43::TestTolerance( const CFMtx43 &m, f32 fTolerance/*=FMATH_POS_EPSILON*/ ) const 
{
	u32 i, *pnMtx1, *pnMtx2;
	f32 fDiff, fPosTolerance, fNegTolerance;

	pnMtx1 = (u32 *)&a[0];
	pnMtx2 = (u32 *)&m.a[0];

	if ( fTolerance < 0.0f ) 
	{
		fPosTolerance = -fTolerance;
		fNegTolerance = fTolerance;
	} 
	else 
	{
		fPosTolerance = fTolerance;
		fNegTolerance = -fTolerance;
	}

    for ( i = 0; i < 12; i++ ) 
    {
		// first do a bitwise compare
		if ( pnMtx1[i] == pnMtx2[i] )  
		{
			// these elements are the same, keep looking for differences
			continue;
		}
		
		// next do an epsilon compare
		fDiff = a[i] - m.a[i];
		if ( fDiff > fPosTolerance || fDiff < fNegTolerance ) 
		{
			return FALSE;
		}		
    }
    
	return TRUE;
}

//
//
FINLINE CFMtx43 &CFMtx43::Zero( void ) 
{ 
	m_vX.Zero(); 
	m_vY.Zero(); 
	m_vZ.Zero(); 
	m_vPos.Zero(); 
	return *this; 
}

//
//
FINLINE CFMtx43 &CFMtx43::Identity( void ) 
{ 
	m_vX.Set(1.0f, 0.0f, 0.0f); 
	m_vY.Set(0.0f, 1.0f, 0.0f); 
	m_vZ.Set(0.0f, 0.0f, 1.0f); 
	m_vPos.Zero(); 
	return *this; 
}
/*
//
//
FINLINE CFMtx43 CFMtx43::Inverse( void ) const 
{ 
	return GetInverse(); 
};

//
// RotateX - Rotates the matrix by "fDegrees" around the X axis.
FINLINE void CFMtx43::RotateDegX( const f32 fDegrees ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );

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

//
//	RotateY - Rotates the matrix by "fDegrees" around the y axis
FINLINE void CFMtx43::RotateDegY( const f32 fDegrees ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );

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

//
// RotateY - Rotates the matrix by "fDegrees" around the y axis.
FINLINE void CFMtx43::RotateDegZ( const f32 fDegrees ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );

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
*/

//
// RotateX - Rotates the matrix by "fRadians" around the X axis.
FINLINE void CFMtx43::RotateX( const f32 fRadians ) 
{
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

//
//	RotateY - Rotates the matrix by "fRadians" around the y axis
FINLINE void CFMtx43::RotateY( const f32 fRadians ) 
{
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

//
// RotateY - Rotates the matrix by "fRadians" around the y axis.
FINLINE void CFMtx43::RotateZ( const f32 fRadians ) 
{
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

//
// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43::SetRotationX( const f32 fRadians ) 
{
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

//
// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43::SetRotationY( const f32 fRadians ) 
{
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

//
// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43::SetRotationZ( const f32 fRadians ) 
{
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

/*
//
// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43::SetRotationDegX( const f32 fDegrees ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

//
// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43::SetRotationDegY( const f32 fDegrees ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vRight.x = m_vFront.z;

	m_vRight.z = -m_vFront.x;
};

//
// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43::SetRotationDegZ( const f32 fDegrees ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};
*/
//
// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43::SetRotationX( const f32 fRadians, f32 fScale ) 
{
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vUp.z *= fScale;
	m_vUp.y *= fScale;
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

//
// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43::SetRotationY( const f32 fRadians, f32 fScale ) 
{
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vFront.x *= fScale;
	m_vFront.z *= fScale;
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

//
// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43::SetRotationZ( const f32 fRadians, f32 fScale ) 
{
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vRight.y *= fScale;
	m_vRight.x *= fScale;
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

/*
//
// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43::SetRotationDegX( const f32 fDegrees, f32 fScale ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vUp.z *= fScale;
	m_vUp.y *= fScale;
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

//
// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43::SetRotationDegY( const f32 fDegrees, f32 fScale ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vFront.x *= fScale;
	m_vFront.z *= fScale;
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

//
// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43::SetRotationDegZ( const f32 fDegrees, f32 fScale ) 
{
	f32 fRadians = FMATH_DEG2RAD( fDegrees );
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vRight.y *= fScale;
	m_vRight.x *= fScale;
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};
*/
//
//
FINLINE void CFMtx43::SetPitchYawRoll( f32 &rfPitchRadians, f32 &rfYawRadians, f32 &rfRollRadians ) 
{
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

//FINLINE CFQuat CFMtx43::GetQuaternion( void ) const { CFQuat Quat; return Quat.BuildQuat( *this ); }

//
// NOTE: the Pitch, Yaw and Roll are negated, I assume because it is left handed.
FINLINE void CFMtx43::GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const 
{
//	FASSERT_NOW;	// NOTE: We don't have an asin() function yet!!!

	rfPitchDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vUp.z, m_vUp.y ) );
	rfYawDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vFront.x, m_vFront.z ) );
	rfRollDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vRight.y, m_vRight.x ) );
}



//--------------------------------------------------------------------
// CFMtx44 Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx44::CFMtx44( void ) {}

//
//
FINLINE CFMtx44::CFMtx44( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront, const CFVec3 &vPos ) 
{ 
	m_vRight = vRight; 
	m_vUp = vUp; 
	m_vFront = vFront; 
	m_vPos = vPos; 
}

//
//
FINLINE CFMtx44::CFMtx44( const CFVec4 &vRight, const CFVec4 &vUp, const CFVec4 &vFront, const CFVec4 &vPos ) 
{ 
	m_vRight = vRight; 
	m_vUp = vUp; 
	m_vFront = vFront; 
	m_vPos = vPos; 
}

//
//
FINLINE CFMtx44::CFMtx44( const CFMtx33 &m ) 
{ 
	m_vRight = m.m_vRight;
	m_vUp = m.m_vUp;
	m_vFront = m.m_vFront; 
	m_vPos.Zero(); 
}

//
//
FINLINE CFMtx44::CFMtx44( const CFMtx43 &m ) 
{ 
	m_vRight = m.m_vRight;
	m_vUp = m.m_vUp;
	m_vFront = m.m_vFront;
	m_vPos = m.m_vPos;
}

//
//
FINLINE CFMtx44::CFMtx44( const CFMtx44 &m ) 
{ 
	*this = m;
}

//
//
FINLINE CFMtx44::CFMtx44( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 ) 
{
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=ff03;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=ff13;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=ff23;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=ff33;
}

//
//
FINLINE CFMtx44 &CFMtx44::operator = ( const CFMtx33 &m ) 
{ 
	m_vRight = m.m_vRight; 
	m_vUp = m.m_vUp; 
	m_vFront = m.m_vFront; 
	m_vPos.Zero(); 
	return *this;
}

//
//
FINLINE CFMtx44 &CFMtx44::operator = ( const CFMtx43 &m ) 
{ 
	m_vRight = m.m_vRight; 
	m_vUp = m.m_vUp; 
	m_vFront = m.m_vFront; 
	m_vPos = m.m_vPos; 
	return *this; 
}

//
//
FINLINE CFMtx44 &CFMtx44::operator = ( const CFMtx44 &m ) 
{ 
	a[0] = m.a[0]; a[1] = m.a[1]; a[2] = m.a[2]; a[3] = m.a[3]; 
	a[4] = m.a[4]; a[5] = m.a[5]; a[6] = m.a[6]; a[7] = m.a[7]; 
	a[8] = m.a[8]; a[9] = m.a[9]; a[10] = m.a[10]; a[11] = m.a[11]; 
	a[12] = m.a[12]; a[13] = m.a[13]; a[14] = m.a[14]; a[15] = m.a[15]; 
	return *this; 
}

//
//
FINLINE CFMtx44 &CFMtx44::operator += ( const CFMtx44 &m ) 
{ 
	m_vX += m.m_vX; 
	m_vY += m.m_vY; 
	m_vZ += m.m_vZ; 
	m_vPos += m.m_vPos; 
	return *this; 
}

//
//
FINLINE CFMtx44 &CFMtx44::operator -= ( const CFMtx44 &m ) 
{ 
	m_vX -= m.m_vX; 
	m_vY -= m.m_vY; 
	m_vZ -= m.m_vZ; 
	m_vPos -= m.m_vPos; 
	return *this; 
}

//
//
FINLINE CFMtx44 &CFMtx44::operator *= ( const f32 &fS ) 
{ 
	m_vX *= fS; 
	m_vY *= fS; 
	m_vZ *= fS; 
	m_vPos *= fS; 
	return *this; 
}

//
//
FINLINE CFMtx44 &CFMtx44::operator *= ( const CFMtx44 &m ) 
{
	CFMtx44 mtxResult;
	mtxResult.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z + m_vPos.x*m.m_vX.w;
	mtxResult.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z + m_vPos.y*m.m_vX.w;
	mtxResult.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z + m_vPos.z*m.m_vX.w;
	mtxResult.m_vX.w = m_vX.w*m.m_vX.x + m_vY.w*m.m_vX.y + m_vZ.w*m.m_vX.z + m_vPos.w*m.m_vX.w;
	mtxResult.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z + m_vPos.x*m.m_vY.w;
	mtxResult.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z + m_vPos.y*m.m_vY.w;
	mtxResult.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z + m_vPos.z*m.m_vY.w;
	mtxResult.m_vY.w = m_vX.w*m.m_vY.x + m_vY.w*m.m_vY.y + m_vZ.w*m.m_vY.z + m_vPos.w*m.m_vY.w;
	mtxResult.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z + m_vPos.x*m.m_vZ.w;
	mtxResult.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z + m_vPos.y*m.m_vZ.w;
	mtxResult.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z + m_vPos.z*m.m_vZ.w;
	mtxResult.m_vZ.w = m_vX.w*m.m_vZ.x + m_vY.w*m.m_vZ.y + m_vZ.w*m.m_vZ.z + m_vPos.w*m.m_vZ.w;
	mtxResult.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x*m.m_vPos.w;
	mtxResult.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y*m.m_vPos.w;
	mtxResult.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z*m.m_vPos.w;
	mtxResult.m_vPos.w = m_vX.w*m.m_vPos.x + m_vY.w*m.m_vPos.y + m_vZ.w*m.m_vPos.z + m_vPos.w*m.m_vPos.w;
	*this = mtxResult;
	return *this;
}

//
//
FINLINE CFMtx44 CFMtx44::operator + ( const CFMtx44 &m ) const 
{ 
	return CFMtx44( m_vX+m.m_vX, m_vY+m.m_vY, m_vZ+m.m_vZ, m_vPos+m.m_vPos ); 
}

//
//
FINLINE CFMtx44 CFMtx44::operator - ( const CFMtx44 &m ) const 
{ 
	return CFMtx44( m_vX-m.m_vX, m_vY-m.m_vY, m_vZ-m.m_vZ, m_vPos-m.m_vPos ); 
}

//
//
FINLINE CFMtx44 CFMtx44::operator * ( const CFMtx44 &m ) const 
{
	CFMtx44 mtxResult;
	mtxResult.m_vX.x = m_vX.x*m.m_vX.x + m_vY.x*m.m_vX.y + m_vZ.x*m.m_vX.z + m_vPos.x*m.m_vX.w;
	mtxResult.m_vX.y = m_vX.y*m.m_vX.x + m_vY.y*m.m_vX.y + m_vZ.y*m.m_vX.z + m_vPos.y*m.m_vX.w;
	mtxResult.m_vX.z = m_vX.z*m.m_vX.x + m_vY.z*m.m_vX.y + m_vZ.z*m.m_vX.z + m_vPos.z*m.m_vX.w;
	mtxResult.m_vX.w = m_vX.w*m.m_vX.x + m_vY.w*m.m_vX.y + m_vZ.w*m.m_vX.z + m_vPos.w*m.m_vX.w;
	mtxResult.m_vY.x = m_vX.x*m.m_vY.x + m_vY.x*m.m_vY.y + m_vZ.x*m.m_vY.z + m_vPos.x*m.m_vY.w;
	mtxResult.m_vY.y = m_vX.y*m.m_vY.x + m_vY.y*m.m_vY.y + m_vZ.y*m.m_vY.z + m_vPos.y*m.m_vY.w;
	mtxResult.m_vY.z = m_vX.z*m.m_vY.x + m_vY.z*m.m_vY.y + m_vZ.z*m.m_vY.z + m_vPos.z*m.m_vY.w;
	mtxResult.m_vY.w = m_vX.w*m.m_vY.x + m_vY.w*m.m_vY.y + m_vZ.w*m.m_vY.z + m_vPos.w*m.m_vY.w;
	mtxResult.m_vZ.x = m_vX.x*m.m_vZ.x + m_vY.x*m.m_vZ.y + m_vZ.x*m.m_vZ.z + m_vPos.x*m.m_vZ.w;
	mtxResult.m_vZ.y = m_vX.y*m.m_vZ.x + m_vY.y*m.m_vZ.y + m_vZ.y*m.m_vZ.z + m_vPos.y*m.m_vZ.w;
	mtxResult.m_vZ.z = m_vX.z*m.m_vZ.x + m_vY.z*m.m_vZ.y + m_vZ.z*m.m_vZ.z + m_vPos.z*m.m_vZ.w;
	mtxResult.m_vZ.w = m_vX.w*m.m_vZ.x + m_vY.w*m.m_vZ.y + m_vZ.w*m.m_vZ.z + m_vPos.w*m.m_vZ.w;
	mtxResult.m_vPos.x = m_vX.x*m.m_vPos.x + m_vY.x*m.m_vPos.y + m_vZ.x*m.m_vPos.z + m_vPos.x*m.m_vPos.w;
	mtxResult.m_vPos.y = m_vX.y*m.m_vPos.x + m_vY.y*m.m_vPos.y + m_vZ.y*m.m_vPos.z + m_vPos.y*m.m_vPos.w;
	mtxResult.m_vPos.z = m_vX.z*m.m_vPos.x + m_vY.z*m.m_vPos.y + m_vZ.z*m.m_vPos.z + m_vPos.z*m.m_vPos.w;
	mtxResult.m_vPos.w = m_vX.w*m.m_vPos.x + m_vY.w*m.m_vPos.y + m_vZ.w*m.m_vPos.z + m_vPos.w*m.m_vPos.w;
	return mtxResult;
}

//
//
FINLINE CFMtx44 CFMtx44::operator * ( const f32 &fS ) const 
{ 
	return CFMtx43( m_vX*fS, m_vY*fS, m_vZ*fS, m_vPos*fS ); 
}

//
//
FINLINE CFVec3 CFMtx44::MultPoint( const CFVec3 &v ) const 
{
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z );
}

//
//
FINLINE CFVec4 CFMtx44::MultPoint( const CFVec4 &v ) const 
{
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z + m_vPos.x*v.w,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z + m_vPos.y*v.w,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z + m_vPos.z*v.w,
					m_vX.w*v.x + m_vY.w*v.y + m_vZ.w*v.z + m_vPos.w*v.w );
}

//
//
FINLINE CFVec3 CFMtx44::MultDir( const CFVec3 &v ) const 
{
	return CFVec3(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z );
}

//
//
FINLINE CFVec4 CFMtx44::MultDir( const CFVec4 &v ) const 
{
	return CFVec4(	m_vX.x*v.x + m_vY.x*v.y + m_vZ.x*v.z,
					m_vX.y*v.x + m_vY.y*v.y + m_vZ.y*v.z,
					m_vX.z*v.x + m_vY.z*v.y + m_vZ.z*v.z,
					m_vX.w*v.x + m_vY.w*v.y + m_vZ.w*v.z );
}

//
//
FINLINE BOOL CFMtx44::operator == ( const CFMtx44 &m ) const 
{ 
	return (m_vX==m.m_vX) && (m_vY==m.m_vY) && (m_vZ==m.m_vZ) && (m_vPos==m.m_vPos); 
}

//
//
FINLINE BOOL CFMtx44::operator != ( const CFMtx44 &m ) const 
{ 
	return (m_vX!=m.m_vX) || (m_vY!=m.m_vY) || (m_vZ!=m.m_vZ) || (m_vPos!=m.m_vPos); 
}

//
//
FINLINE CFMtx44 &CFMtx44::Zero( void ) 
{ 
	m_vX.Zero(); 
	m_vY.Zero(); 
	m_vZ.Zero(); 
	m_vPos.Zero(); 
	return *this;
}

//
//
FINLINE CFMtx44 &CFMtx44::Identity( void ) 
{ 
	m_vX.Set(1.0f, 0.0f, 0.0f, 0.0f); 
	m_vY.Set(0.0f, 1.0f, 0.0f, 0.0f); 
	m_vZ.Set(0.0f, 0.0f, 1.0f, 0.0f); 
	m_vPos.Set(0.0f, 0.0f, 0.0f, 1.0f);
	return *this;
}

//
//
FINLINE CFMtx44 &CFMtx44::Transpose( CFMtx44 &rDestMtx ) const 
{
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

//
//
FINLINE CFMtx44 &CFMtx44::Transpose( void )
{
	CFMtx44 NewMtx = *this;
	NewMtx.Transpose( *this );
	return *this;
}




//--------------------------------------------------------------------
// CFMtx44A Implementation:
//--------------------------------------------------------------------
FINLINE CFMtx44A::CFMtx44A( void ) {}

//
//
FINLINE CFMtx44A::CFMtx44A( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) 
{ 
	Set( rRight, rUp, rFront, rPos );
}

//
//
FINLINE CFMtx44A::CFMtx44A( const CFVec4A &rRight, const CFVec4A &rUp, const CFVec4A &rFront, const CFVec4A &rPos ) 
{ 
	Set( rRight, rUp, rFront, rPos );
}

//
//
FINLINE CFMtx44A::CFMtx44A( const CFMtx44A &rM ) 
{ 
	Set( rM );
}

//
//
FINLINE CFMtx44A::CFMtx44A( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 ) 
{
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=ff03;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=ff13;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=ff23;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=ff33;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Zero( void ) 
{
	Set( m_ZeroMtx );
	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Identity( void ) 
{
	Set( m_IdentityMtx );
	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::operator = ( const CFMtx44A &rM ) 
{ 
	Set( rM ); 
	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx33 &rM ) 
{
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vPos = CFVec4A::m_UnitAxisW;

	m_vX.w = 0.0f;
	m_vY.w = 0.0f;
	m_vZ.w = 0.0f;

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx43 &rM ) 
{
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vPos.v3 = rM.m_vPos;

	m_vX.w = 0.0f;
	m_vY.w = 0.0f;
	m_vZ.w = 0.0f;
	m_vPos.w = 1.0f;

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx43A &rM ) 
{
	m_vX.Set( rM.m_vX );
	m_vY.Set( rM.m_vY );
	m_vZ.Set( rM.m_vZ );
	m_vPos.Set( rM.m_vPos );

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx44 &rM ) 
{
	m_vX.v4 = rM.m_vX;
	m_vY.v4 = rM.m_vY;
	m_vZ.v4 = rM.m_vZ;
	m_vPos.v4 = rM.m_vPos;

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const CFMtx44A &rM ) 
{
	m_vX.Set( rM.m_vX );
	m_vY.Set( rM.m_vY );
	m_vZ.Set( rM.m_vZ );
	m_vPos.Set( rM.m_vPos );

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) 
{
	m_vX.Set( rRight );
	m_vY.Set( rUp );
	m_vZ.Set( rFront );
	m_vPos.Set( rPos );

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const CFVec4A &rRight, const CFVec4A &rUp, const CFVec4A &rFront, const CFVec4A &rPos ) 
{
	m_vX.Set( rRight );
	m_vY.Set( rUp );
	m_vZ.Set( rFront );
	m_vPos.Set( rPos );

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Set( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 ) 
{
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=ff03;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=ff13;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=ff23;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=ff33;

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM1, const CFMtx44A &rM2 ) 
{
	m_vX.x = rM1.m_vX.x*rM2.m_vX.x + rM1.m_vY.x*rM2.m_vX.y + rM1.m_vZ.x*rM2.m_vX.z + rM1.m_vPos.x*rM2.m_vX.w;
	m_vX.y = rM1.m_vX.y*rM2.m_vX.x + rM1.m_vY.y*rM2.m_vX.y + rM1.m_vZ.y*rM2.m_vX.z + rM1.m_vPos.y*rM2.m_vX.w;
	m_vX.z = rM1.m_vX.z*rM2.m_vX.x + rM1.m_vY.z*rM2.m_vX.y + rM1.m_vZ.z*rM2.m_vX.z + rM1.m_vPos.z*rM2.m_vX.w;
	m_vX.w = rM1.m_vX.w*rM2.m_vX.x + rM1.m_vY.w*rM2.m_vX.y + rM1.m_vZ.w*rM2.m_vX.z + rM1.m_vPos.w*rM2.m_vX.w;
	m_vY.x = rM1.m_vX.x*rM2.m_vY.x + rM1.m_vY.x*rM2.m_vY.y + rM1.m_vZ.x*rM2.m_vY.z + rM1.m_vPos.x*rM2.m_vY.w;
	m_vY.y = rM1.m_vX.y*rM2.m_vY.x + rM1.m_vY.y*rM2.m_vY.y + rM1.m_vZ.y*rM2.m_vY.z + rM1.m_vPos.y*rM2.m_vY.w;
	m_vY.z = rM1.m_vX.z*rM2.m_vY.x + rM1.m_vY.z*rM2.m_vY.y + rM1.m_vZ.z*rM2.m_vY.z + rM1.m_vPos.z*rM2.m_vY.w;
	m_vY.w = rM1.m_vX.w*rM2.m_vY.x + rM1.m_vY.w*rM2.m_vY.y + rM1.m_vZ.w*rM2.m_vY.z + rM1.m_vPos.w*rM2.m_vY.w;
	m_vZ.x = rM1.m_vX.x*rM2.m_vZ.x + rM1.m_vY.x*rM2.m_vZ.y + rM1.m_vZ.x*rM2.m_vZ.z + rM1.m_vPos.x*rM2.m_vZ.w;
	m_vZ.y = rM1.m_vX.y*rM2.m_vZ.x + rM1.m_vY.y*rM2.m_vZ.y + rM1.m_vZ.y*rM2.m_vZ.z + rM1.m_vPos.y*rM2.m_vZ.w;
	m_vZ.z = rM1.m_vX.z*rM2.m_vZ.x + rM1.m_vY.z*rM2.m_vZ.y + rM1.m_vZ.z*rM2.m_vZ.z + rM1.m_vPos.z*rM2.m_vZ.w;
	m_vZ.w = rM1.m_vX.w*rM2.m_vZ.x + rM1.m_vY.w*rM2.m_vZ.y + rM1.m_vZ.w*rM2.m_vZ.z + rM1.m_vPos.w*rM2.m_vZ.w;
	m_vPos.x = rM1.m_vX.x*rM2.m_vPos.x + rM1.m_vY.x*rM2.m_vPos.y + rM1.m_vZ.x*rM2.m_vPos.z + rM1.m_vPos.x*rM2.m_vPos.w;
	m_vPos.y = rM1.m_vX.y*rM2.m_vPos.x + rM1.m_vY.y*rM2.m_vPos.y + rM1.m_vZ.y*rM2.m_vPos.z + rM1.m_vPos.y*rM2.m_vPos.w;
	m_vPos.z = rM1.m_vX.z*rM2.m_vPos.x + rM1.m_vY.z*rM2.m_vPos.y + rM1.m_vZ.z*rM2.m_vPos.z + rM1.m_vPos.z*rM2.m_vPos.w;
	m_vPos.w = rM1.m_vX.w*rM2.m_vPos.x + rM1.m_vY.w*rM2.m_vPos.y + rM1.m_vZ.w*rM2.m_vPos.z + rM1.m_vPos.w*rM2.m_vPos.w;
	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM ) 
{
	CFMtx44A mtxResult;
	mtxResult.Mul( *this, rM );
	*this = mtxResult;
	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Mul( const CFMtx44A &rM, const f32 &fVal ) 
{
	
	m_vX.Mul( rM.m_vX, fVal );
	m_vY.Mul( rM.m_vY, fVal );
	m_vZ.Mul( rM.m_vZ, fVal );
	m_vPos.Mul( rM.m_vPos, fVal );

	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Mul( const f32 &fVal ) 
{
	m_vX.Mul( fVal );
	m_vY.Mul( fVal );
	m_vZ.Mul( fVal );
	m_vPos.Mul( fVal );

	return *this;
}

//
//
FINLINE CFVec3A &CFMtx44A::MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const 
{
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z + m_vPos.x;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z + m_vPos.y;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z + m_vPos.z;

	return rRV;
}

//
//
FINLINE CFVec4A &CFMtx44A::MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const 
{
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z + m_vPos.x*rV.w;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z + m_vPos.y*rV.w;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z + m_vPos.z*rV.w;
	rRV.w = m_vX.w*rV.x + m_vY.w*rV.y + m_vZ.w*rV.z + m_vPos.w*rV.w;

	return rRV;
}

//
//
FINLINE CFVec3A &CFMtx44A::MulPoint( CFVec3A &rV ) const 
{
	CFVec3A rRV;
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z + m_vPos.x;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z + m_vPos.y;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z + m_vPos.z;
	rV = rRV;

	return rV;
}

//
//
FINLINE CFVec4A &CFMtx44A::MulPoint( CFVec4A &rV ) const 
{
	CFVec4A rRV;
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z + m_vPos.x*rV.w;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z + m_vPos.y*rV.w;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z + m_vPos.z*rV.w;
	rRV.w = m_vX.w*rV.x + m_vY.w*rV.y + m_vZ.w*rV.z + m_vPos.w*rV.w;
	rV = rRV;

	return rV;
}

//
//
FINLINE CFVec3A &CFMtx44A::MulDir( CFVec3A &rRV, const CFVec3A &rV ) const 
{
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z;

	return rRV;
}

//
//
FINLINE CFVec4A &CFMtx44A::MulDir( CFVec4A &rRV, const CFVec4A &rV ) const 
{
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z;
	rRV.w = m_vX.w*rV.x + m_vY.w*rV.y + m_vZ.w*rV.z;

	return rRV;
}

//
//
FINLINE CFVec3A &CFMtx44A::MulDir( CFVec3A &rV ) const 
{
	CFVec3A rRV;
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z;
	rV = rRV;

	return rV;
}

//
//
FINLINE CFVec4A &CFMtx44A::MulDir( CFVec4A &rV ) const 
{
	CFVec4A rRV;
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z;
	rRV.w = m_vX.w*rV.x + m_vY.w*rV.y + m_vZ.w*rV.z;
	rV = rRV;

	return rV;
}

//
//
FINLINE CFMtx44A &CFMtx44A::ReceiveTranspose( const CFMtx44A &rM ) 
{
	aa[0][0] = rM.aa[0][0];
	aa[1][0] = rM.aa[0][1];
	aa[2][0] = rM.aa[0][2];
	aa[3][0] = rM.aa[0][3];
	aa[0][1] = rM.aa[1][0];
	aa[1][1] = rM.aa[1][1];
	aa[2][1] = rM.aa[1][2];
	aa[3][1] = rM.aa[1][3];
	aa[0][2] = rM.aa[2][0];
	aa[1][2] = rM.aa[2][1];
	aa[2][2] = rM.aa[2][2];
	aa[3][2] = rM.aa[2][3];
	aa[0][3] = rM.aa[3][0];
	aa[1][3] = rM.aa[3][1];
	aa[2][3] = rM.aa[3][2];
	aa[3][3] = rM.aa[3][3];
	
	return *this;
}

//
//
FINLINE CFMtx44A &CFMtx44A::Transpose( void ) 
{
	CFMtx44A rM( *this );
	aa[0][0] = rM.aa[0][0];
	aa[1][0] = rM.aa[0][1];
	aa[2][0] = rM.aa[0][2];
	aa[3][0] = rM.aa[0][3];
	aa[0][1] = rM.aa[1][0];
	aa[1][1] = rM.aa[1][1];
	aa[2][1] = rM.aa[1][2];
	aa[3][1] = rM.aa[1][3];
	aa[0][2] = rM.aa[2][0];
	aa[1][2] = rM.aa[2][1];
	aa[2][2] = rM.aa[2][2];
	aa[3][2] = rM.aa[2][3];
	aa[0][3] = rM.aa[3][0];
	aa[1][3] = rM.aa[3][1];
	aa[2][3] = rM.aa[3][2];
	aa[3][3] = rM.aa[3][3];

	return *this;
}

//--------------------------------------------------------------------
// CFMtx43A Implementation:
//--------------------------------------------------------------------

//
//
FINLINE CFMtx43A::CFMtx43A( void ) 
{ 
//	aa[0][3] = aa[1][3] = aa[2][3] = aa[3][3] = 0.0f; 
}

//
//
FINLINE CFMtx43A::CFMtx43A( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) 
{ 
	Set( rRight, rUp, rFront, rPos ); 
}

//
//
FINLINE CFMtx43A::CFMtx43A( const CFMtx43A &rM ) 
{ 
	Set( rM ); 
}

//
//
FINLINE CFMtx43A::CFMtx43A( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 ) 
{
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02; aa[0][3]=0.0f;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12; aa[1][3]=0.0f;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22; aa[2][3]=0.0f;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32; aa[3][3]=0.0f;
}

//
//
FINLINE BOOL CFMtx43A::operator == ( const CFMtx43A &rM ) const 
{
	for ( int i = 0; i < 16; i++ )
	{
		if ( rM.a[i] != a[i] )
			return FALSE;
	}
	
	return TRUE;
}

//
//
FINLINE BOOL CFMtx43A::operator != ( const CFMtx43A &rM ) const 
{
	for ( int i = 0; i < 16; i++ )
	{
		if ( rM.a[i] != a[i] )
			return TRUE;
	}
	
	return FALSE;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Zero( void ) 
{
	Set( m_ZeroMtx );
	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Identity( void ) 
{
	Set( m_IdentityMtx );
	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Identity33( void ) 
{
	m_vRight = CFVec3A::m_UnitAxisX;
	m_vUp = CFVec3A::m_UnitAxisY;
	m_vFront = CFVec3A::m_UnitAxisZ;
	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::operator = ( const CFMtx43A &rM ) 
{ 
	Set( rM ); 
	return *this; 
}

//
//
FINLINE CFMtx43A &CFMtx43A::Set( const CFMtx33 &rM ) 
{
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vPos.Zero();

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Set( const CFMtx43 &rM ) 
{
	m_vX.v3 = rM.m_vX;
	m_vY.v3 = rM.m_vY;
	m_vZ.v3 = rM.m_vZ;
	m_vPos.v3 = rM.m_vPos;

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Set( const CFMtx43A &rM ) 
{
	m_vX.Set( rM.m_vX );
	m_vY.Set( rM.m_vY );
	m_vZ.Set( rM.m_vZ );
	m_vPos.Set( rM.m_vPos );

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Set( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos ) 
{
	m_vX.Set( rRight );
	m_vY.Set( rUp );
	m_vZ.Set( rFront );
	m_vPos.Set( rPos );

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Set( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 ) 
{
	aa[0][0]=ff00; aa[0][1]=ff01; aa[0][2]=ff02;
	aa[1][0]=ff10; aa[1][1]=ff11; aa[1][2]=ff12;
	aa[2][0]=ff20; aa[2][1]=ff21; aa[2][2]=ff22;
	aa[3][0]=ff30; aa[3][1]=ff31; aa[3][2]=ff32;

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Mul( register const CFMtx43A &rM1, register const CFMtx43A &rM2 ) 
{
#if 0
	m_vX.x = rM1.m_vX.x*rM2.m_vX.x + rM1.m_vY.x*rM2.m_vX.y + rM1.m_vZ.x*rM2.m_vX.z;
	m_vX.y = rM1.m_vX.y*rM2.m_vX.x + rM1.m_vY.y*rM2.m_vX.y + rM1.m_vZ.y*rM2.m_vX.z;
	m_vX.z = rM1.m_vX.z*rM2.m_vX.x + rM1.m_vY.z*rM2.m_vX.y + rM1.m_vZ.z*rM2.m_vX.z;
	m_vY.x = rM1.m_vX.x*rM2.m_vY.x + rM1.m_vY.x*rM2.m_vY.y + rM1.m_vZ.x*rM2.m_vY.z;
	m_vY.y = rM1.m_vX.y*rM2.m_vY.x + rM1.m_vY.y*rM2.m_vY.y + rM1.m_vZ.y*rM2.m_vY.z;
	m_vY.z = rM1.m_vX.z*rM2.m_vY.x + rM1.m_vY.z*rM2.m_vY.y + rM1.m_vZ.z*rM2.m_vY.z;
	m_vZ.x = rM1.m_vX.x*rM2.m_vZ.x + rM1.m_vY.x*rM2.m_vZ.y + rM1.m_vZ.x*rM2.m_vZ.z;
	m_vZ.y = rM1.m_vX.y*rM2.m_vZ.x + rM1.m_vY.y*rM2.m_vZ.y + rM1.m_vZ.y*rM2.m_vZ.z;
	m_vZ.z = rM1.m_vX.z*rM2.m_vZ.x + rM1.m_vY.z*rM2.m_vZ.y + rM1.m_vZ.z*rM2.m_vZ.z;
	m_vPos.x = rM1.m_vX.x*rM2.m_vPos.x + rM1.m_vY.x*rM2.m_vPos.y + rM1.m_vZ.x*rM2.m_vPos.z + rM1.m_vPos.x;
	m_vPos.y = rM1.m_vX.y*rM2.m_vPos.x + rM1.m_vY.y*rM2.m_vPos.y + rM1.m_vZ.y*rM2.m_vPos.z + rM1.m_vPos.y;
	m_vPos.z = rM1.m_vX.z*rM2.m_vPos.x + rM1.m_vY.z*rM2.m_vPos.y + rM1.m_vZ.z*rM2.m_vPos.z + rM1.m_vPos.z;
#else
	// matrix 1 (aka "a")
	register CFMtx43A *pThis = this;
	register f32 r0a01, r0a23;
	register f32 r1a01, r1a23;
	register f32 r2a01, r2a23;
	register f32 r3a01, r3a23;
	// matrix 2 (aka "b")
	register f32 r0b01, r0b23;
	register f32 r1b01, r1b23;
	register f32 r2b01, r2b23;
	register f32 r3b01, r3b23;
	// result
	register f32 f0_01, f01_33;
	register f32 f1_01;
	register f32 f2_01, f23_33;
	register f32 f3_01;
	
	register f32 temp0, temp1, temp2;

	asm
	{
		psq_l		r0a01,	0(rM1), 	0,		0;		// a00,a01
		psq_l		r0b01,	0(rM2), 	0,		0;		// b00,b01
		ps_muls0	f0_01,	r0a01,		r0b01;			// a00b00,a01b00
	    psq_l		r1a01,	16(rM1), 	0,		0;		// a10,a11
	    psq_l		r2a01,	32(rM1),	0,		0;		// a20,a21
	    ps_madds1	f0_01, 	r1a01,		r0b01,	f0_01;	// a00b00+a10b01,a01b00+a11b01
	    psq_l		r0b23,  8(rM2),		0, 		0;		// b02,b03
	    psq_l		r3a01, 	48(rM1),	0,		0;		// a30,a31

		ps_madds0	f0_01,	r2a01,		r0b23,	f0_01;	// a00b00+a10b01+a20b02,a01b00+a11b01+a21b02
		
		////	Done with row 0, x and y
		
	    psq_l		r1b01,	16(rM2),	0, 		0;		// b10,b11
		psq_l		r1b23,	24(rM2),	0,		0;		// b12,b13
		
		ps_muls0	f1_01,	r0a01,		r1b01;			// a00b10,a01b10
		    psq_st	f0_01,	0(pThis), 	0,		0;
		ps_madds1	f1_01,	r1a01,		r1b01,	f1_01;	// a00b10+a10b11,a01b11+a11b11
		psq_l		r2b01,	32(rM2),	0,		0;		// b20,b21
		ps_madds0	f1_01,	r2a01,		r1b23,	f1_01;	// a00b10+a10b11+a20b12,a01b11+a11b11+a21b12
		
		////	Done with row 1, x and y

		psq_l		r2b23,	40(rM2),	0,		0;		// b22,b23
		ps_muls0	f2_01,	r0a01,		r2b01;          // a00b20,a01b20
			ps_merge00	temp0,	r0b01,		r1b01;
		ps_madds1	f2_01,	r1a01,		r2b01,	f2_01;	// a00b20+a10b21,a01b20+a11b21
		psq_l		r3b01,	48(rM2),	0,		0;		// b30,b31
		ps_madds0	f2_01,	r2a01,		r2b23,	f2_01;	// a00b20+a10b21+a20b22,a01b20+a11b21+a21b22
		
		////	Done with row 2, x and y

		psq_l		r3b23,	56(rM2),	0,		0;		// b32,b33

		ps_muls0	f3_01,	r0a01,		r3b01;			// a00b30,a01b30
		psq_l		r0a23,	8(rM1),		0,		0;		// a02,a03
		ps_madds1	f3_01,	r1a01,		r3b01,	f3_01;	// a00b30+a10b31,a01b30+a11b31
			ps_merge11	temp1,	r0b01,		r1b01;
		ps_madds0	f3_01,	r2a01,		r3b23,	f3_01;	// a00b30+a10b31+a20b32,a01b30+a11b31+a21b32
		psq_l		r1a23,	24(rM1),	0,		0;		// a12,a13
		ps_add		f3_01,	f3_01,		r3a01;			// a00b30+a10b31+a20b32+a30,a01b30+a11b31+a21b32+a31
		
		////	Done with row 3, x and y
	    
		psq_l		r2a23,	40(rM1),	0,		0;		// a22,a23
	    
		ps_merge00	temp2,	r0b23,		r1b23;
			psq_st	f1_01,	16(pThis), 	0,		0;
		ps_muls0	f01_33,	temp0,		r0a23;			// a02b00,a02b10
			psq_st	f2_01,	32(pThis), 	0,		0;
		ps_madds0	f01_33,	temp1,		r1a23,	f01_33;	// a02b00+a12b01,a02b10+a12b11
			psq_st	f3_01,	48(pThis), 	0,		0;
		ps_madds0	f01_33,	temp2,		r2a23,	f01_33;	// a02b00+a12b01+a22b02,a02b10+a12b11+a22b12
		
		////	Done with z of rows 0 and 1
	    
		ps_merge00	temp0,	r2b01,		r3b01;
			psq_st	f01_33,	8(pThis), 	1,		0;
		ps_merge11	temp1,	r2b01,		r3b01;
			ps_merge11	f01_33,	f01_33,		f01_33;
		ps_merge00	temp2,	r2b23,		r3b23;
		ps_muls0	f23_33,	temp0,		r0a23;			// a02b20,a02b30
			psq_st	f01_33,	24(pThis), 	1,		0;
		ps_madds0	f23_33,	temp1,		r1a23,	f23_33;	// a02b20+a12b21,a02b30+a12b31
		ps_madds0	f23_33,	temp2,		r2a23,	f23_33;	// a02b20+a12b21+a22b22,a02b30+a12b31+a22b32
	    
		psq_l		r3a23,	56(rM1),	0,		0;		// a32,a33
	    
			psq_st		f23_33,	40(pThis), 	1,		0;	// Store the z of row 2
			
			ps_merge11	f23_33,	f23_33,		f23_33;
			ps_add		f23_33, f23_33, 	r3a23;		// a02b30+a12b31+a22b32+a32,unused
			psq_st		f23_33,	56(pThis), 	1,		0;	// Store the z of row 3
    }

#endif
	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Mul( register const CFMtx43A &rM ) 
{
#if 0
	CFMtx43A mtxResult;
	mtxResult.m_vX.x = m_vX.x*rM.m_vX.x + m_vY.x*rM.m_vX.y + m_vZ.x*rM.m_vX.z;
	mtxResult.m_vX.y = m_vX.y*rM.m_vX.x + m_vY.y*rM.m_vX.y + m_vZ.y*rM.m_vX.z;
	mtxResult.m_vX.z = m_vX.z*rM.m_vX.x + m_vY.z*rM.m_vX.y + m_vZ.z*rM.m_vX.z;
	mtxResult.m_vY.x = m_vX.x*rM.m_vY.x + m_vY.x*rM.m_vY.y + m_vZ.x*rM.m_vY.z;
	mtxResult.m_vY.y = m_vX.y*rM.m_vY.x + m_vY.y*rM.m_vY.y + m_vZ.y*rM.m_vY.z;
	mtxResult.m_vY.z = m_vX.z*rM.m_vY.x + m_vY.z*rM.m_vY.y + m_vZ.z*rM.m_vY.z;
	mtxResult.m_vZ.x = m_vX.x*rM.m_vZ.x + m_vY.x*rM.m_vZ.y + m_vZ.x*rM.m_vZ.z;
	mtxResult.m_vZ.y = m_vX.y*rM.m_vZ.x + m_vY.y*rM.m_vZ.y + m_vZ.y*rM.m_vZ.z;
	mtxResult.m_vZ.z = m_vX.z*rM.m_vZ.x + m_vY.z*rM.m_vZ.y + m_vZ.z*rM.m_vZ.z;
	mtxResult.m_vPos.x = m_vX.x*rM.m_vPos.x + m_vY.x*rM.m_vPos.y + m_vZ.x*rM.m_vPos.z + m_vPos.x;
	mtxResult.m_vPos.y = m_vX.y*rM.m_vPos.x + m_vY.y*rM.m_vPos.y + m_vZ.y*rM.m_vPos.z + m_vPos.y;
	mtxResult.m_vPos.z = m_vX.z*rM.m_vPos.x + m_vY.z*rM.m_vPos.y + m_vZ.z*rM.m_vPos.z + m_vPos.z;
	*this = mtxResult;
	return *this;
#else
	Mul( *this, rM );
#endif
	return *this;
}


//
//
FINLINE CFMtx43A &CFMtx43A::RevMul( register const CFMtx43A &rM ) 
{
	Mul( rM, *this );
	
	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Mul( const CFMtx43A &rM, const f32 &fVal ) 
{
	m_vX.Mul( rM.m_vX, fVal );
	m_vY.Mul( rM.m_vY, fVal );
	m_vZ.Mul( rM.m_vZ, fVal );
	m_vPos.Mul( rM.m_vPos, fVal );
/*
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
*/
	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Mul( const f32 &fVal ) 
{
	m_vX.Mul( fVal );
	m_vY.Mul( fVal ); 
	m_vZ.Mul( fVal ); 
	m_vPos.Mul( fVal ); 
/*
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
*/
	return *this;
}

//
//
FINLINE CFVec3A &CFMtx43A::MulPoint( register CFVec3A &rRV, register const CFVec3A &rV ) const 
{
#if 0
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z + m_vPos.x;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z + m_vPos.y;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z + m_vPos.z;
#else
	register const CFMtx43A *pThis = this;
	register f32 r0m01, r0m23;
	register f32 r1m01, r1m23;
	register f32 r2m01, r2m23;
	register f32 r3m01, r3m23;
	// vector
	register f32 fVxy, fVzw;
	
	register f32 fTemp1, fTemp2;

	asm
	{
		psq_l		r0m01,	0(pThis),	0,		0

		psq_l		fVxy,	0(rV),		0,		0 

		psq_l		r1m01,	16(pThis),	0,		0 

		ps_muls0	fTemp1,	r0m01,		fVxy
		psq_l		r2m01,	32(pThis),	0,		0 
		
		psq_l		fVzw,	8(rV),		1,		0
		ps_madds1	fTemp1, r1m01,		fVxy,	fTemp1

		psq_l   	r0m23,	8(pThis),	1,		0
		ps_madds0	fTemp1,	r2m01,		fVzw,	fTemp1

		psq_l   	r3m01,	48(pThis),	0,		0
		ps_muls0	fTemp2,	r0m23,		fVxy
	    
		psq_l   	r1m23,	24(pThis),	1,		0
		ps_add		fTemp1, r3m01,		fTemp1
		
		psq_l		r2m23,	40(pThis),	1,		0
		ps_madds1	fTemp2,	r1m23,		fVxy,	fTemp2

		psq_l		r3m23,	56(pThis),	1,		0
		ps_madds0	fTemp2,	r2m23,		fVzw,	fTemp2
	    
		psq_st		fTemp1,	0(rRV),		0,		0
		ps_add		fTemp2,	r3m23,		fTemp2
	    
		psq_st		fTemp2,	8(rRV),		1,		0
    }
#endif		

	return rRV;
}


//
//
FINLINE CFVec3A &CFMtx43A::MulPoint( register CFVec3A &rV ) const 
{
#if 0
	CFVec3A vResult;
	vResult.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z + m_vPos.x;
	vResult.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z + m_vPos.y;
	vResult.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z + m_vPos.z;
	rV.Set( vResult );
#else
	register const CFMtx43A *pThis = this;
	register f32 r0m01, r0m23;
	register f32 r1m01, r1m23;
	register f32 r2m01, r2m23;
	register f32 r3m01, r3m23;
	// vector
	register f32 fVxy, fVzw;
	
	register f32 fTemp1, fTemp2;

	asm
	{
		psq_l		r0m01,	0(pThis),	0,		0

		psq_l		fVxy,	0(rV),		0,		0 

		psq_l		r1m01,	16(pThis),	0,		0 

		ps_muls0	fTemp1,	r0m01,		fVxy
		psq_l		r2m01,	32(pThis),	0,		0 
		
		psq_l		fVzw,	8(rV),		1,		0
		ps_madds1	fTemp1, r1m01,		fVxy,	fTemp1

		psq_l   	r0m23,	8(pThis),	1,		0
		ps_madds0	fTemp1,	r2m01,		fVzw,	fTemp1

		psq_l   	r3m01,	48(pThis),	0,		0
		ps_muls0	fTemp2,	r0m23,		fVxy
	    
		psq_l   	r1m23,	24(pThis),	1,		0
		ps_add		fTemp1, r3m01,		fTemp1
		
		psq_l		r2m23,	40(pThis),	1,		0
		ps_madds1	fTemp2,	r1m23,		fVxy,	fTemp2

		psq_l		r3m23,	56(pThis),	1,		0
		ps_madds0	fTemp2,	r2m23,		fVzw,	fTemp2
	    
		psq_st		fTemp1,	0(rV),		0,		0
		ps_add		fTemp2,	r3m23,		fTemp2
	    
		psq_st		fTemp2,	8(rV),		1,		0
    }
#endif		

	return rV;
}

FINLINE CFVec3&  CFMtx43A::MulPoint( CFVec3 &rRV, const CFVec3 &rV ) const 
{
	FASSERT( &rRV != &rV );
	rRV.x = m44.m_vX.x*rV.x + m44.m_vY.x*rV.y + m44.m_vZ.x*rV.z + m_vPos.x;
	rRV.y = m44.m_vX.y*rV.x + m44.m_vY.y*rV.y + m44.m_vZ.y*rV.z + m_vPos.y;
	rRV.z = m44.m_vX.z*rV.x + m44.m_vY.z*rV.y + m44.m_vZ.z*rV.z + m_vPos.z;
	
	return rRV;
}

//
//
FINLINE CFVec3A &CFMtx43A::MulDir( register CFVec3A &rRV, register const CFVec3A &rV ) const 
{
#if 0
	rRV.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z;
	rRV.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z;
	rRV.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z;
#else
	register const CFMtx43A *pThis = this;
	register f32 r0m01, r0m23;
	register f32 r1m01, r1m23;
	register f32 r2m01, r2m23;
	// vector
	register f32 fVxy, fVzw;
	
	register f32 fTemp1, fTemp2;

	asm
	{
		psq_l		r0m01,	0(pThis),	0,		0

		psq_l		fVxy,	0(rV),		0,		0 

		psq_l		r1m01,	16(pThis),	0,		0 

		ps_muls0	fTemp1,	r0m01,		fVxy
		psq_l		r2m01,	32(pThis),	0,		0 
		
		psq_l		fVzw,	8(rV),		1,		0
		ps_madds1	fTemp1, r1m01,		fVxy,	fTemp1

		psq_l   	r0m23,	8(pThis),	1,		0
		ps_madds0	fTemp1,	r2m01,		fVzw,	fTemp1

		psq_l   	r1m23,	24(pThis),	1,		0
		ps_muls0	fTemp2,	r0m23,		fVxy
	    
		psq_l		r2m23,	40(pThis),	1,		0
		ps_madds1	fTemp2,	r1m23,		fVxy,	fTemp2

		psq_st		fTemp1,	0(rRV),		0,		0
		ps_madds0	fTemp2,	r2m23,		fVzw,	fTemp2
	    
		psq_st		fTemp2,	8(rRV),		1,		0
    }
#endif

	return rRV;
}

//
//
FINLINE CFVec3A &CFMtx43A::MulDir( register CFVec3A &rV ) const 
{
#if 0
	CFVec3A vResult;
	vResult.x = m_vX.x*rV.x + m_vY.x*rV.y + m_vZ.x*rV.z;
	vResult.y = m_vX.y*rV.x + m_vY.y*rV.y + m_vZ.y*rV.z;
	vResult.z = m_vX.z*rV.x + m_vY.z*rV.y + m_vZ.z*rV.z;
	rV.Set( vResult );
#else
	register const CFMtx43A *pThis = this;
	register f32 r0m01, r0m23;
	register f32 r1m01, r1m23;
	register f32 r2m01, r2m23;
	// vector
	register f32 fVxy, fVzw;
	
	register f32 fTemp1, fTemp2;

	asm
	{
		psq_l		r0m01,	0(pThis),	0,		0

		psq_l		fVxy,	0(rV),		0,		0 

		psq_l		r1m01,	16(pThis),	0,		0 

		ps_muls0	fTemp1,	r0m01,		fVxy
		psq_l		r2m01,	32(pThis),	0,		0 
		
		psq_l		fVzw,	8(rV),		1,		0
		ps_madds1	fTemp1, r1m01,		fVxy,	fTemp1

		psq_l   	r0m23,	8(pThis),	1,		0
		ps_madds0	fTemp1,	r2m01,		fVzw,	fTemp1

		psq_l   	r1m23,	24(pThis),	1,		0
		ps_muls0	fTemp2,	r0m23,		fVxy
	    
		psq_l		r2m23,	40(pThis),	1,		0
		ps_madds1	fTemp2,	r1m23,		fVxy,	fTemp2

		psq_st		fTemp1,	0(rV),		0,		0
		ps_madds0	fTemp2,	r2m23,		fVzw,	fTemp2
	    
		psq_st		fTemp2,	8(rV),		1,		0
    }
#endif

	return rV;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Mul33( register const CFMtx43A &rM1, register const CFMtx43A &rM2 ) 
{
	// matrix 1 (aka "a")
	register CFMtx43A *pThis = this;
	register f32 r0a01, r0a23;
	register f32 r1a01, r1a23;
	register f32 r2a01, r2a23;
	// matrix 2 (aka "b")
	register f32 r0b01, r0b23;
	register f32 r1b01, r1b23;
	register f32 r2b01, r2b23;
	// result
	register f32 f0_01, f01_33;
	register f32 f1_01;
	register f32 f2_01, f23_33;
	
	register f32 temp0, temp1, temp2;

	asm
	{
		psq_l		r0a01,	0(rM1), 	0,		0;		// a00,a01
		psq_l		r0b01,	0(rM2), 	0,		0;		// b00,b01
		ps_muls0	f0_01,	r0a01,		r0b01;			// a00b00,a01b00
	    psq_l		r1a01,	16(rM1), 	0,		0;		// a10,a11
	    psq_l		r2a01,	32(rM1),	0,		0;		// a20,a21
	    ps_madds1	f0_01, 	r1a01,		r0b01,	f0_01;	// a00b00+a10b01,a01b00+a11b01
	    psq_l		r0b23,  8(rM2),		0, 		0;		// b02,b03

		ps_madds0	f0_01,	r2a01,		r0b23,	f0_01;	// a00b00+a10b01+a20b02,a01b00+a11b01+a21b02
		
		////	Done with row 0, x and y
		
	    psq_l		r1b01,	16(rM2),	0, 		0;		// b10,b11
		psq_l		r1b23,	24(rM2),	0,		0;		// b12,b13
		
		ps_muls0	f1_01,	r0a01,		r1b01;			// a00b10,a01b10
		    psq_st	f0_01,	0(pThis), 	0,		0;
		ps_madds1	f1_01,	r1a01,		r1b01,	f1_01;	// a00b10+a10b11,a01b11+a11b11
		psq_l		r2b01,	32(rM2),	0,		0;		// b20,b21
		ps_madds0	f1_01,	r2a01,		r1b23,	f1_01;	// a00b10+a10b11+a20b12,a01b11+a11b11+a21b12
		
		////	Done with row 1, x and y

		psq_l		r2b23,	40(rM2),	0,		0;		// b22,b23
		ps_muls0	f2_01,	r0a01,		r2b01;          // a00b20,a01b20
			ps_merge00	temp0,	r0b01,		r1b01;
		ps_madds1	f2_01,	r1a01,		r2b01,	f2_01;	// a00b20+a10b21,a01b20+a11b21
		ps_madds0	f2_01,	r2a01,		r2b23,	f2_01;	// a00b20+a10b21+a20b22,a01b20+a11b21+a21b22
		
		////	Done with row 2, x and y
		
		psq_l		r0a23,	8(rM1),		0,		0;		// a02,a03
		psq_l		r1a23,	24(rM1),	0,		0;		// a12,a13

			ps_merge11	temp1,	r0b01,		r1b01;
			
		psq_l		r2a23,	40(rM1),	0,		0;		// a22,a23
	    
		ps_merge00	temp2,	r0b23,		r1b23;
			psq_st	f1_01,	16(pThis), 	0,		0;
		ps_muls0	f01_33,	temp0,		r0a23;			// a02b00,a02b10
			psq_st	f2_01,	32(pThis), 	0,		0;
		ps_madds0	f01_33,	temp1,		r1a23,	f01_33;	// a02b00+a12b01,a02b10+a12b11
		ps_madds0	f01_33,	temp2,		r2a23,	f01_33;	// a02b00+a12b01+a22b02,a02b10+a12b11+a22b12
		
		////	Done with z of rows 0 and 1
	    
			psq_st	f01_33,	8(pThis), 	1,		0;
			ps_merge11	f01_33,	f01_33,		f01_33;
		ps_muls0	f23_33,	temp0,		r0a23;			// a02b20,a02b30
			psq_st	f01_33,	24(pThis), 	1,		0;
		ps_madds0	f23_33,	temp1,		r1a23,	f23_33;	// a02b20+a12b21,a02b30+a12b31
		ps_madds0	f23_33,	temp2,		r2a23,	f23_33;	// a02b20+a12b21+a22b22,a02b30+a12b31+a22b32
	    
			psq_st		f23_33,	40(pThis), 	1,		0;	// Store the z of row 2
    }

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Mul33( const CFMtx43A &rM1 ) 
{
	Mul33( *this, rM1 );

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::RevMul33( const CFMtx43A &rM1 ) 
{
	Mul33( rM1, *this );

	return *this;
}

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


//
//
FINLINE CFMtx43A &CFMtx43A::ReceiveAffineInverse( const CFMtx43A &rM, BOOL bProvidedMatrixMayNotHaveUnitScale ) 
{
	CFVec3A NegPos;

	ReceiveTranspose33( rM );

	if ( bProvidedMatrixMayNotHaveUnitScale ) 
	{
		NegPos.Set( rM.m_vRight.InvMagSq() );

		m_vRight.Mul( NegPos );
		m_vUp.Mul( NegPos );
		m_vFront.Mul( NegPos );
	}

	NegPos.ReceiveNegative( rM.m_vPos );
	MulDir( m_vPos, NegPos );

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::AffineInvert( BOOL bProvidedMatrixMayNotHaveUnitScale ) 
{
	Transpose33();

	if ( bProvidedMatrixMayNotHaveUnitScale ) 
	{
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

//
//
FINLINE CFMtx43A &CFMtx43A::ReceiveAffineInverse_KnowScale2( const CFMtx43A &rM, const f32 &fScaleOfSourceMtx2 ) 
{
	CFVec3A NegPos;

	ReceiveTranspose33( rM );

	if ( fScaleOfSourceMtx2 != 1.0f ) 
	{
		NegPos.Set( fmath_Inv( fScaleOfSourceMtx2 ) );

		m_vRight.Mul( NegPos );
		m_vUp.Mul( NegPos );
		m_vFront.Mul( NegPos );
	}

	NegPos.ReceiveNegative( rM.m_vPos );
	MulDir( m_vPos, NegPos );

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::AffineInvert_KnowScale2( const f32 &fScaleOfSourceMtx2 ) 
{
	Transpose33();

	if ( fScaleOfSourceMtx2 != 1.0f ) 
	{
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

//
//
FINLINE CFMtx43A &CFMtx43A::ReceiveAffineInverse_KnowOOScale2( const CFMtx43A &rM, const f32 &fOOScaleOfSourceMtx2 ) 
{
	CFVec3A NegPos;

	ReceiveTranspose33( rM );

	if ( fOOScaleOfSourceMtx2 != 1.0f ) 
	{
		NegPos.Set( fOOScaleOfSourceMtx2 );

		m_vRight.Mul( NegPos );
		m_vUp.Mul( NegPos );
		m_vFront.Mul( NegPos );
	}

	NegPos.ReceiveNegative( rM.m_vPos );
	MulDir( m_vPos, NegPos );

	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::AffineInvert_KnowOOScale2( const f32 &fOOScaleOfSourceMtx2 ) 
{
	Transpose33();

	if ( fOOScaleOfSourceMtx2 != 1.0f ) 
	{
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

//
//
FINLINE CFMtx43A &CFMtx43A::ReceiveTranspose33( const CFMtx43A &rM ) 
{
	aa[0][0] = rM.aa[0][0];
	aa[0][1] = rM.aa[1][0];
	aa[0][2] = rM.aa[2][0];
	aa[1][0] = rM.aa[0][1];
	aa[1][1] = rM.aa[1][1];
	aa[1][2] = rM.aa[2][1];
	aa[2][0] = rM.aa[0][2];
	aa[2][1] = rM.aa[1][2];
	aa[2][2] = rM.aa[2][2];
/*
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
*/
	return *this;
}

//
//
FINLINE CFMtx43A &CFMtx43A::Transpose33( void ) 
{
	f32 fTemp;
	fTemp = aa[1][0];
	aa[1][0] = aa[0][1];
	aa[0][1] = fTemp;
	
	fTemp = aa[2][0];
	aa[2][0] = aa[0][2];
	aa[0][2] = fTemp;
	
	fTemp = aa[2][1];
	aa[2][1] = aa[1][2];
	aa[1][2] = fTemp;
/*
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
*/
	return *this;
}

//
//
FINLINE u32 CFMtx43A::GenKey( void ) const 
{
	FASSERT_NOW;
	u32 nKey;

/*
	__asm {
		mov			eax, this
		movaps		xmm0, [eax]
		xorps	xmm1, [eax+0x10]
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
*/
	return nKey;
}

//
// RotateX - Rotates the matrix by "fRadians" around the X axis.
FINLINE void CFMtx43A::RotateX( const f32 fRadians ) 
{
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

//
//	RotateY - Rotates the matrix by "fRadians" around the y axis
FINLINE void CFMtx43A::RotateY( const f32 fRadians ) 
{
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

//
// RotateY - Rotates the matrix by "fRadians" around the y axis.
FINLINE void CFMtx43A::RotateZ( const f32 fRadians ) 
{
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

//
// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43A::SetRotationX( const f32 fRadians ) 
{
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

//
// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43A::SetRotationY( const f32 fRadians ) 
{
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

//
// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43A::SetRotationZ( const f32 fRadians ) 
{
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

//
// SetYRotation - Set the Rotation around X component.
FINLINE void CFMtx43A::SetRotationX( const f32 fRadians, f32 fScale ) 
{
	fmath_SinCos( fRadians, &m_vUp.z, &m_vUp.y );
	m_vUp.z *= fScale;
	m_vUp.y *= fScale;
	m_vFront.y = -m_vUp.z;
	m_vFront.z = m_vUp.y;
};

//
// SetYRotation - Set the Rotation around Y component.
FINLINE void CFMtx43A::SetRotationY( const f32 fRadians, f32 fScale ) 
{
	fmath_SinCos( fRadians, &m_vFront.x, &m_vFront.z );
	m_vFront.x *= fScale;
	m_vFront.z *= fScale;
	m_vRight.x = m_vFront.z;
	m_vRight.z = -m_vFront.x;
};

//
// SetYRotation - Set the Rotation around Z component.
FINLINE void CFMtx43A::SetRotationZ( const f32 fRadians, f32 fScale ) 
{
	fmath_SinCos( fRadians, &m_vRight.y, &m_vRight.x );
	m_vRight.y *= fScale;
	m_vRight.x *= fScale;
	m_vUp.x = -m_vRight.y;
	m_vUp.y = m_vRight.x;
};

//
// NOTE: the Pitch, Yaw and Roll are negated, I assume because it is left handed.
FINLINE void CFMtx43A::GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const 
{
	rfPitchDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vUp.z, m_vUp.y ) );
	rfYawDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vFront.x, m_vFront.z ) );
	rfRollDegrees = (f32)FMATH_RAD2DEG( fmath_Atan( m_vRight.y, m_vRight.x ) );
}

#if FANG_DEBUG_BUILD
#pragma global_optimizer off
#endif



