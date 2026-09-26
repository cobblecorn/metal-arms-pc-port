//////////////////////////////////////////////////////////////////////////////////////
// fmath_geo.inl - Fang geometry math library.
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


FINLINE CFSphere::CFSphere() {}
FINLINE CFSphere::CFSphere( const CFVec3& rCenter, const f32 fRadius ) { m_Pos=rCenter; m_fRadius=fRadius; }

FINLINE void CFSphere::Zero( void ) { m_fRadius=0.0f; m_Pos.Zero(); }
FINLINE void CFSphere::Set( const CFVec3& rCenter, const f32 fRadius ) { m_Pos=rCenter; m_fRadius=fRadius; }

FINLINE BOOL CFSphere::operator == ( const CFSphere& s ) const { return (m_fRadius==s.m_fRadius) && (m_Pos==s.m_Pos); }
FINLINE BOOL CFSphere::operator != ( const CFSphere& s ) const { return (m_fRadius!=s.m_fRadius) || (m_Pos!=s.m_Pos); }
FINLINE BOOL CFSphere::AreEqual_XZ( const CFSphere& s ) const { return (m_fRadius==s.m_fRadius) && (m_Pos.x==s.m_Pos.x) && (m_Pos.z==s.m_Pos.z); }

FINLINE BOOL CFSphere::IsIntersecting( const CFSphere& s ) const {
	f32 fRadiusSum;

	fRadiusSum = m_fRadius + s.m_fRadius;

	return ( (m_Pos - s.m_Pos).Mag2() < (fRadiusSum*fRadiusSum) );
}

FINLINE BOOL CFSphere::IsIntersecting( const CFVec3& rPoint ) const {
	return ( (rPoint - m_Pos).Mag2() < (m_fRadius*m_fRadius) );
}

FINLINE void CFSphere::BuildFromMinMaxBox( const CFVec3 &rMin, const CFVec3 &rMax ) {
	// get the box's diag vector (the sphere's diameter)
	m_Pos = rMax - rMin;
	// scale the diag by half
	m_Pos *= 0.5f;
	// get the radius
	m_fRadius = m_Pos.Mag();	
	// compute the center of the sphere
	m_Pos += rMin;	
}


FINLINE CFSphereA::CFSphereA() {}
FINLINE CFSphereA::CFSphereA( const CFVec3A& rCenter, const f32 fRadius ) { m_Pos=rCenter; m_fRadius=fRadius; }

FINLINE void CFSphereA::Zero( void ) { m_fRadius=0.0f; m_Pos.Zero(); }
FINLINE void CFSphereA::Set( const CFVec3A& rCenter, const f32 fRadius ) { m_Pos=rCenter; m_fRadius=fRadius; }

FINLINE BOOL CFSphereA::operator == ( const CFSphereA& s ) const { return (m_fRadius==s.m_fRadius) && (m_Pos==s.m_Pos); }
FINLINE BOOL CFSphereA::operator != ( const CFSphereA& s ) const { return (m_fRadius!=s.m_fRadius) || (m_Pos!=s.m_Pos); }
FINLINE BOOL CFSphereA::AreEqual_XZ( const CFSphereA& s ) const { return (m_fRadius==s.m_fRadius) && (m_Pos.x==s.m_Pos.x) && (m_Pos.z==s.m_Pos.z); }

FINLINE BOOL CFSphereA::IsIntersecting( const CFSphereA& s ) const {
	f32 fRadiusSum;

	fRadiusSum = m_fRadius + s.m_fRadius;
	CFVec3A tmp; //FINDFIX: CFVec3A
	return ( tmp.Sub(m_Pos, s.m_Pos).MagSq() < (fRadiusSum*fRadiusSum) );
}

FINLINE BOOL CFSphereA::IsIntersecting( const CFVec3A& rPoint ) const {
	CFVec3A tmp; //FINDFIX: CFVec3A
	return ( tmp.Sub(rPoint, m_Pos).MagSq() < (m_fRadius*m_fRadius) );
}

FINLINE void CFSphereA::BuildFromMinMaxBox( const CFVec3A &rMin, const CFVec3A &rMax ) {
	// get the box's diag vector (the sphere's diameter)
	m_Pos.Sub(rMax, rMin);
	// scale the diag by half
	m_Pos.Mul(0.5f);
	// get the radius
	m_fRadius = m_Pos.Mag();	
	// compute the center of the sphere
	m_Pos.Add(rMin);
}


FINLINE CFRect2D::CFRect2D() {}
FINLINE CFRect2D::CFRect2D( const CFVec2 &UpperLeftVec, const CFVec2 &LowerRightVec ) { m_UpperLeft=UpperLeftVec; m_LowerRight=LowerRightVec; }

FINLINE CFRect2D &CFRect2D::Zero( void ) { m_UpperLeft.x = m_UpperLeft.y = m_LowerRight.x = m_LowerRight.y = 0.0f; return *this; }


// This is the most general version.  It returns a random point in the intersection of a spherical shell whose center
//   is at the origin and a cone whose tip is at the origin.
// fMaxPhi is the maximum angle by which the vector formed by the origin and the selected point can deviate from the
//   positive y-axis.
FINLINE void fmath_RandomPointInSphericalSector(CFVec3A &vecDest, f32 fRadInner, f32 fRadOuter, f32 fMaxPhi)
{
	f32 fTheta = fmath_RandomFloatRange(0.0f, FMATH_2PI);
	f32 fPhi = fmath_RandomFloatRange(0.0f, fMaxPhi);

	f32 fSinTheta, fCosTheta;
	f32 fSinPhi, fCosPhi;

	fmath_SinCos(fTheta, &fSinTheta, &fCosTheta);
	fmath_SinCos(fPhi, &fSinPhi, &fCosPhi);

	f32 fMag = fmath_RandomFloatRange(fRadInner, fRadOuter);

	vecDest.x = fSinPhi * fSinTheta;
	vecDest.y = fCosPhi;
	vecDest.z = fSinPhi * fCosTheta;
	vecDest.Mul(fMag);
}

// Simple random point in sphere (anywhere in sphere).
FINLINE void fmath_RandomPointInSphere(CFVec3A &vecDest, f32 fRadius)
{
	fmath_RandomPointInSphericalSector(vecDest, fRadius, fRadius, FMATH_2PI);
}

// 
FINLINE void fmath_RandomPointInSphereTransformed(CFVec3A &vecDest, f32 fRadius, CFMtx43A &mtxMS2WS)
{
	fmath_RandomPointInSphere(vecDest, fRadius);
	mtxMS2WS.MulPoint(vecDest);
}

FINLINE void fmath_RandomPointInSphericalSectorTransformed(CFVec3A &vecDest, f32 fRadInner, f32 fRadOuter, f32 fMaxPhi, CFMtx43A &mtxMS2WS)
{
	fmath_RandomPointInSphericalSector(vecDest, fRadInner, fRadOuter, fMaxPhi);
	mtxMS2WS.MulPoint(vecDest);
}

FINLINE void fmath_RandomDirTransformed(CFVec3A &vecDest, f32 fRadInner, f32 fRadOuter, f32 fMaxPhi, CFMtx43A &mtxMS2WS)
{
	fmath_RandomPointInSphericalSector(vecDest, fRadInner, fRadOuter, fMaxPhi);
	mtxMS2WS.MulDir(vecDest);
}

FINLINE void fmath_RandomDir(CFVec3A &vecDest, f32 fRadInner, f32 fRadOuter, f32 fMaxPhi)
{
	fmath_RandomPointInSphericalSector(vecDest, fRadInner, fRadOuter, fMaxPhi);
}

//
FINLINE void fmath_RandomPointInBox(CFVec3A &vecDest, f32 fHalfSpanX, f32 fHalfSpanY, f32 fHalfSpanZ)
{
	vecDest.x = fmath_RandomBipolarUnitFloat() * fHalfSpanX;
	vecDest.y = fmath_RandomBipolarUnitFloat() * fHalfSpanY;
	vecDest.z = fmath_RandomBipolarUnitFloat() * fHalfSpanZ;
}

//
FINLINE void fmath_RandomPointInBoxTransformed(CFVec3A &vecDest, f32 fHalfSpanX, f32 fHalfSpanY, f32 fHalfSpanZ, CFMtx43A &mtxMS2WS)
{
	fmath_RandomPointInBox(vecDest, fHalfSpanX, fHalfSpanY, fHalfSpanZ);
	mtxMS2WS.MulPoint(vecDest);
}

// This function isn't directly very useful, but I included it like this to keep it in a consistent 
//   pattern with the functions above.  You'll probably want to use the one below.
FINLINE void fmath_RandomPointOnLineSeg(CFVec3A &vecDest, f32 fLength)
{
	vecDest.x = 0.0f;
	vecDest.y = 0.0f;
	vecDest.z = fmath_RandomFloat() * fLength;
}

//
FINLINE void fmath_RandomPointOnLineSegTransformed(CFVec3A &vecDest, f32 fLength, CFMtx43A &mtxMS2WS)
{
	fmath_RandomPointOnLineSeg(vecDest, fLength);
	mtxMS2WS.MulPoint(vecDest);
}

// 
FINLINE void fmath_RandomPointInCylinder(CFVec3A &vecDest, f32 fRadius, f32 fHeight)
{
	f32 fTheta = fmath_RandomFloat() * FMATH_2PI;
	f32 fSinTheta, fCosTheta;

	fmath_SinCos(fTheta, &fSinTheta, &fCosTheta);
	
	vecDest.x = fRadius * fCosTheta;
	vecDest.y = fmath_RandomFloat() * fHeight;
	vecDest.z = fRadius * fSinTheta;
}


// 
FINLINE void fmath_RandomPointInCylinderTransformed(CFVec3A &vecDest, f32 fRadius, f32 fHeight, CFMtx43A &mtxMS2WS)
{
	fmath_RandomPointInCylinder(vecDest, fRadius, fHeight);
	mtxMS2WS.MulPoint(vecDest);
}

// If you were to need more of these functions, it would be quite easy to add them.
