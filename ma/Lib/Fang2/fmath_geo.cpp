//////////////////////////////////////////////////////////////////////////////////////
// fmath_geo.cpp - Fang geometry math library.
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


#define _EPSILON				(1.0e-5f)


// Returns TRUE if the given line segment intersects the given sphere, or FALSE otherwise.
//
// Set pIntersectionPoint to NULL if the point of intersection is not desired, and this function
// will potentially be faster. The point of intersection is the point closest to rPoint1 that
// is intersecting the sphere.
//
// If pIntersectionPoint is not NULL and this function returns TRUE, pIntersectionPoint will be filled
// with the point of intersection closest to rPoint1. If rPoint1 is inside the sphere, the point of
// intersection will be set to rPoint1.
//
// If pfUnitDistFromStartToIntersection is also not NULL and this function returns TRUE,
// pfUnitDistFromStartToIntersection will be filled with the unit distance from the start
// point to the point of intersection, along the ray.
//
// If pIntersectionPoint is NULL, pfUnitDistFromStartToIntersection is ignored.
//
// If this function returns FALSE, the contents of pIntersectionPoint and pfUnitDistFromStartToIntersection
// are not modified.
BOOL CFSphere::IsIntersecting( const CFVec3& rPoint1, const CFVec3& rPoint2, CFVec3 *pIntersectionPoint, f32 *pfUnitDistFromStartToIntersection ) const 
{
	CFVec3 P1P2, P1ToCenter, P2ToCenter;
	f32 fOOP1P2Dist2, fV, fD2, fUnitDistFromStartToIntersection;

	if( IsIntersecting( rPoint1 ) ) 
	{
		// Point 1 is inside the sphere...

		if( pIntersectionPoint ) 
		{
			*pIntersectionPoint = rPoint1;

			if( pfUnitDistFromStartToIntersection ) 
			{
				*pfUnitDistFromStartToIntersection = 0.0f;
			}
		}

		return TRUE;
	}

	// Point 1 is outside the sphere...

	if( IsIntersecting( rPoint2 ) ) 
	{
		// Point 2 is inside the sphere...

		if( pIntersectionPoint ) 
		{
			P1P2 = rPoint2 - rPoint1;
			P1ToCenter = m_Pos - rPoint1;
			fOOP1P2Dist2 = P1P2.InvMag2();

			fV = P1ToCenter.Dot( P1P2 );
			fD2 = m_fRadius*m_fRadius - (P1ToCenter.Mag2() - fV*fV*fOOP1P2Dist2);

			if( fD2 < 0.0f ) 
			{
				// Floating point precision problems might yield a negative fD2.
				// This just means that point 2 is very close to the surface of
				// the sphere, so we can just return point 2...
				fUnitDistFromStartToIntersection = 1.0f;
				*pIntersectionPoint = rPoint2;
			} 
			else 
			{
				fUnitDistFromStartToIntersection = fV*fOOP1P2Dist2 - fmath_Sqrt(fD2*fOOP1P2Dist2);
				*pIntersectionPoint = rPoint1 + P1P2*fUnitDistFromStartToIntersection;
			}

			if( pfUnitDistFromStartToIntersection ) 
			{
				*pfUnitDistFromStartToIntersection = fUnitDistFromStartToIntersection;
			}
		}

		return TRUE;
	}

	// Neither point is inside the sphere...

	P1P2 = rPoint2 - rPoint1;
	P1ToCenter = m_Pos - rPoint1;

	fV = P1ToCenter.Dot( P1P2 );
	if( fV <= 0.0f ) 
	{
		// P1P2 is pointing away from the sphere (or P1 & P2 are the same point)...
		return FALSE;
	}

	P2ToCenter = m_Pos - rPoint2;
	if( P2ToCenter.Dot( P1P2 ) >= 0.0f ) 
	{
		// P1P2 is pointing away from the sphere...
		return FALSE;
	}

	fOOP1P2Dist2 = P1P2.InvMag2();
	fD2 = m_fRadius*m_fRadius - (P1ToCenter.Mag2() - fV*fV*fOOP1P2Dist2);

	if( fD2 <= 0.0f ) 
	{
		// Infinite line colinear with P1P2 doesn't intersect the sphere...
		return FALSE;
	}

	// Line segment intersects the sphere...

	if( pIntersectionPoint ) 
	{
		fUnitDistFromStartToIntersection = fV*fOOP1P2Dist2 - fmath_Sqrt(fD2*fOOP1P2Dist2);

		*pIntersectionPoint = rPoint1 + P1P2*fUnitDistFromStartToIntersection;

		if( pfUnitDistFromStartToIntersection ) 
		{
			*pfUnitDistFromStartToIntersection = fUnitDistFromStartToIntersection;
		}
	}

	return TRUE;
}


//
//
//
BOOL CFSphere::IsIntersecting( const CFCapsule *pCapsule ) const 
{
	FASSERT( pCapsule );

	f32 fSphPlusCapsuleRadSq = pCapsule->m_fRadius + m_fRadius;
	fSphPlusCapsuleRadSq *= fSphPlusCapsuleRadSq;

	CFVec3A vDiffC1, vDiffC2, vDiff21;
	vDiffC1.v3 = m_Pos - pCapsule->m_vPoint1.v3;
	f32 fMag = vDiffC1.MagSq();
	if ( fMag <= fSphPlusCapsuleRadSq )
	{
		// Point 1 is inside the sphere...
		return TRUE;
	}

	// Point 1 is outside the sphere...

	vDiffC2.v3 = m_Pos - pCapsule->m_vPoint2.v3;
	fMag = vDiffC2.MagSq();
	if ( fMag <= fSphPlusCapsuleRadSq )
	{
		// Point 2 is inside the sphere...
		return TRUE;
	}

	// Neither point is inside the sphere...

	if ( vDiffC1.Dot( vDiffC2 ) >= 0.f )
	{
		// Center is in the same direction for both points so it
		// must have intersected one or the other for collision
		// to take place
		return FALSE;
	}

	vDiff21.Sub( pCapsule->m_vPoint2, pCapsule->m_vPoint1 );
	f32 fV = vDiffC1.Dot( vDiff21 );

	f32 fOOP1P2Dist2 = vDiff21.InvMagSq();
	f32 fD2 = fSphPlusCapsuleRadSq - (vDiffC1.MagSq() - fV*fV*fOOP1P2Dist2);

	if( fD2 <= 0.0f ) 
	{
		// Infinite line colinear with P1P2 doesn't intersect the sphere...
		return FALSE;
	}

	return TRUE;
}


//
//
//
BOOL CFSphere::IsIntersecting( const CFCone& rCone ) const 
{
	CFVec3 vSphereToCone, vProjSphereAlongAxisToCone, vProjSphereAlongAxisToSphere, vPointOnAxis, vConeInwardUnitNormal;
	f32 fProjSphereAlongAxisToConeDist, fSin, fCos, fTan;

	FASSERT( rCone.m_fAngle>=0.0f && rCone.m_fAngle<FMATH_HALF_PI );

	// First find out if sphere is completely behind the cone...

	vSphereToCone = rCone.m_Vertex - m_Pos;
	fProjSphereAlongAxisToConeDist = vSphereToCone.Dot( rCone.m_UnitDir );

	if( fProjSphereAlongAxisToConeDist > m_fRadius ) 
	{
		// Sphere is behind cone...
		return FALSE;
	}

	// Sphere is in front of the cone. Test it against the cone edges...

	fmath_SinCos( rCone.m_fAngle, &fSin, &fCos );
	fTan = fSin / fCos;

	vProjSphereAlongAxisToCone = rCone.m_UnitDir * fProjSphereAlongAxisToConeDist;
	vProjSphereAlongAxisToSphere = vProjSphereAlongAxisToCone - vSphereToCone;
	vPointOnAxis = rCone.m_Vertex - vProjSphereAlongAxisToCone + rCone.m_UnitDir*(vProjSphereAlongAxisToSphere.Mag() * fTan);
	vConeInwardUnitNormal = vPointOnAxis - m_Pos;
	vConeInwardUnitNormal.Unitize();

	return (vConeInwardUnitNormal.Dot(vSphereToCone) < m_fRadius);
}


//
//
//
void CFSphere::BuildLooseTriangleBound( const CFVec3 &rPoint1, const CFVec3 &rPoint2, const CFVec3 &rPoint3 ) {
	CFVec3 P1P2, P1P3, P2P3, CenterToPoint;
	f32 fDistSquared12, fDistSquared13, fDistSquared23, fRadiusSquared;

	#define __EPSILON 1.00001f

	P1P2 = rPoint2 - rPoint1;
	P1P3 = rPoint3 - rPoint1;
	P2P3 = rPoint3 - rPoint2;

	fDistSquared12 = P1P2.Mag2();
	fDistSquared13 = P1P3.Mag2();
	fDistSquared23 = P2P3.Mag2();

	if( fDistSquared12 > fDistSquared13 ) {
		if( fDistSquared12 > fDistSquared23 ) {
			// P1P2 is longest...

			m_fRadius = 0.5f * fmath_Sqrt( fDistSquared12 );
			m_Pos = rPoint1 + P1P2*0.5f;

			CenterToPoint = rPoint3 - m_Pos;
			fRadiusSquared = CenterToPoint.Mag2();
			if( fRadiusSquared > m_fRadius*m_fRadius ) {
				m_fRadius = fmath_Sqrt( fRadiusSquared );
			}

			m_fRadius *= __EPSILON;
			return;
		}
	} else {
		if( fDistSquared13 > fDistSquared23 ) {
			// P1P3 is longest...

			m_fRadius = 0.5f * fmath_Sqrt( fDistSquared13 );
			m_Pos = rPoint1 + P1P3*0.5f;

			CenterToPoint = rPoint2 - m_Pos;
			fRadiusSquared = CenterToPoint.Mag2();
			if( fRadiusSquared > m_fRadius*m_fRadius ) {
				m_fRadius = fmath_Sqrt( fRadiusSquared );
			}

			m_fRadius *= __EPSILON;
			return;
		}
	}

	// P2P3 is longest...

	m_fRadius = 0.5f * fmath_Sqrt( fDistSquared23 );
	m_Pos = rPoint2 + P2P3*0.5f;

	CenterToPoint = rPoint1 - m_Pos;
	fRadiusSquared = CenterToPoint.Mag2();
	if( fRadiusSquared > m_fRadius*m_fRadius ) {
		m_fRadius = fmath_Sqrt( fRadiusSquared );
	}

	m_fRadius *= __EPSILON;

	#undef __EPSILON
}







//
//
//  CFVec3A
//
//
// Returns TRUE if the given line segment intersects the given sphere, or FALSE otherwise.
//
// Set pIntersectionPoint to NULL if the point of intersection is not desired, and this function
// will potentially be faster. The point of intersection is the point closest to rPoint1 that
// is intersecting the sphere.
//
// If pIntersectionPoint is not NULL and this function returns TRUE, pIntersectionPoint will be filled
// with the point of intersection closest to rPoint1. If rPoint1 is inside the sphere, the point of
// intersection will be set to rPoint1.
//
// If pfUnitDistFromStartToIntersection is also not NULL and this function returns TRUE,
// pfUnitDistFromStartToIntersection will be filled with the unit distance from the start
// point to the point of intersection, along the ray.
//
// If pIntersectionPoint is NULL, pfUnitDistFromStartToIntersection is ignored.
//
// If this function returns FALSE, the contents of pIntersectionPoint and pfUnitDistFromStartToIntersection
// are not modified.
BOOL CFSphereA::IsIntersecting( const CFVec3A& rPoint1, const CFVec3A& rPoint2, CFVec3A *pIntersectionPoint, f32 *pfUnitDistFromStartToIntersection ) const 
{
	CFVec3A P1P2, P1ToCenter, P2ToCenter;
	f32 fOOP1P2Dist2, fV, fD2, fUnitDistFromStartToIntersection;

	if( IsIntersecting( rPoint1 ) ) 
	{
		// Point 1 is inside the sphere...

		if( pIntersectionPoint ) 
		{
			*pIntersectionPoint = rPoint1;

			if( pfUnitDistFromStartToIntersection ) 
			{
				*pfUnitDistFromStartToIntersection = 0.0f;
			}
		}

		return TRUE;
	}

	// Point 1 is outside the sphere...

	if( IsIntersecting( rPoint2 ) ) 
	{
		// Point 2 is inside the sphere...

		if( pIntersectionPoint ) 
		{
			P1P2.Sub(rPoint2, rPoint1);
			P1ToCenter.Sub(m_Pos, rPoint1);
			fOOP1P2Dist2 = P1P2.InvMagSq();

			fV = P1ToCenter.Dot( P1P2 );
			fD2 = m_fRadius*m_fRadius - (P1ToCenter.MagSq() - fV*fV*fOOP1P2Dist2);

			if( fD2 < 0.0f ) 
			{
				// Floating point precision problems might yield a negative fD2.
				// This just means that point 2 is very close to the surface of
				// the sphere, so we can just return point 2...
				fUnitDistFromStartToIntersection = 1.0f;
				*pIntersectionPoint = rPoint2;
			} 
			else 
			{
				fUnitDistFromStartToIntersection = fV*fOOP1P2Dist2 - fmath_Sqrt(fD2*fOOP1P2Dist2);
				*pIntersectionPoint = P1P2;
				pIntersectionPoint->Mul(fUnitDistFromStartToIntersection);
				pIntersectionPoint->Add(rPoint1);
			}

			if( pfUnitDistFromStartToIntersection ) 
			{
				*pfUnitDistFromStartToIntersection = fUnitDistFromStartToIntersection;
			}
		}

		return TRUE;
	}

	// Neither point is inside the sphere...

	P1P2.Sub(rPoint2, rPoint1);
	P1ToCenter.Sub(m_Pos, rPoint1);

	fV = P1ToCenter.Dot( P1P2 );
	if( fV <= 0.0f ) 
	{
		// P1P2 is pointing away from the sphere (or P1 & P2 are the same point)...
		return FALSE;
	}

	P2ToCenter.Sub(m_Pos, rPoint2);
	if( P2ToCenter.Dot( P1P2 ) >= 0.0f ) 
	{
		// P1P2 is pointing away from the sphere...
		return FALSE;
	}

	fOOP1P2Dist2 = P1P2.InvMagSq();
	fD2 = m_fRadius*m_fRadius - (P1ToCenter.MagSq() - fV*fV*fOOP1P2Dist2);

	if( fD2 <= 0.0f ) 
	{
		// Infinite line colinear with P1P2 doesn't intersect the sphere...
		return FALSE;
	}

	// Line segment intersects the sphere...

	if( pIntersectionPoint ) 
	{
		fUnitDistFromStartToIntersection = fV*fOOP1P2Dist2 - fmath_Sqrt(fD2*fOOP1P2Dist2);

//Fang1		*pIntersectionPoint = rPoint1 + P1P2*fUnitDistFromStartToIntersection;
		pIntersectionPoint->Mul(P1P2, fUnitDistFromStartToIntersection);
		pIntersectionPoint->Add(rPoint1);


		if( pfUnitDistFromStartToIntersection ) 
		{
			*pfUnitDistFromStartToIntersection = fUnitDistFromStartToIntersection;
		}
	}

	return TRUE;
}

/*  pgm: commented out until there is a CFConeA
BOOL CFSphereA::IsIntersecting( const CFCone& rCone ) const {
	CFVec3A vSphereToCone, vProjSphereAlongAxisToCone, vProjSphereAlongAxisToSphere, vPointOnAxis, vConeInwardUnitNormal;
	f32 fProjSphereAlongAxisToConeDist, fSin, fCos, fTan;

	FASSERT( rCone.m_fAngle>=0.0f && rCone.m_fAngle<FMATH_HALF_PI );

	// First find out if sphere is completely behind the cone...

	vSphereToCone = rCone.m_Vertex - m_Pos;
	fProjSphereAlongAxisToConeDist = vSphereToCone.Dot( rCone.m_UnitDir );

	if( fProjSphereAlongAxisToConeDist > m_fRadius ) {
		// Sphere is behind cone...
		return FALSE;
	}

	// Sphere is in front of the cone. Test it against the cone edges...

	fmath_SinCos( rCone.m_fAngle, &fSin, &fCos );
	fTan = fSin / fCos;

	vProjSphereAlongAxisToCone = rCone.m_UnitDir * fProjSphereAlongAxisToConeDist;
	vProjSphereAlongAxisToSphere = vProjSphereAlongAxisToCone - vSphereToCone;
	vPointOnAxis = rCone.m_Vertex - vProjSphereAlongAxisToCone + rCone.m_UnitDir*(vProjSphereAlongAxisToSphere.Mag() * fTan);
	vConeInwardUnitNormal = vPointOnAxis - m_Pos;
	vConeInwardUnitNormal.Unitize();

	return (vConeInwardUnitNormal.Dot(vSphereToCone) < m_fRadius);
}
*/


//
//
//
void CFSphereA::BuildLooseTriangleBound( const CFVec3A &rPoint1, const CFVec3A &rPoint2, const CFVec3A &rPoint3 ) 
{
	CFVec3A P1P2, P1P3, P2P3, CenterToPoint;
	f32 fDistSquared12, fDistSquared13, fDistSquared23, fRadiusSquared;

	#define __EPSILON 1.00001f

	P1P2.Sub(rPoint2, rPoint1);
	P1P3.Sub(rPoint3, rPoint1);
	P2P3.Sub(rPoint3, rPoint2);

	fDistSquared12 = P1P2.MagSq();
	fDistSquared13 = P1P3.MagSq();
	fDistSquared23 = P2P3.MagSq();

	if( fDistSquared12 > fDistSquared13 ) 
	{
		if( fDistSquared12 > fDistSquared23 ) {
			// P1P2 is longest...

			m_fRadius = 0.5f * fmath_Sqrt( fDistSquared12 );
			//FANG1 m_Pos = rPoint1 + P1P2*0.5f;
			m_Pos.Mul(P1P2, 0.5f);
			m_Pos.Add(rPoint1);

			CenterToPoint.Sub(rPoint3, m_Pos);
			fRadiusSquared = CenterToPoint.MagSq();
			if( fRadiusSquared > m_fRadius*m_fRadius ) 
			{
				m_fRadius = fmath_Sqrt( fRadiusSquared );
			}

			m_fRadius *= __EPSILON;
			return;
		}
	} 
	else 
	{
		if( fDistSquared13 > fDistSquared23 ) 
		{
			// P1P3 is longest...

			m_fRadius = 0.5f * fmath_Sqrt( fDistSquared13 );
			//FANG1 m_Pos = rPoint1 + P1P3*0.5f;
			m_Pos.Mul(P1P3, 0.5f);
			m_Pos.Add(rPoint1);

			CenterToPoint.Sub(rPoint2, m_Pos);
			fRadiusSquared = CenterToPoint.MagSq();
			if( fRadiusSquared > m_fRadius*m_fRadius ) 
			{
				m_fRadius = fmath_Sqrt( fRadiusSquared );
			}

			m_fRadius *= __EPSILON;
			return;
		}
	}

	// P2P3 is longest...

	m_fRadius = 0.5f * fmath_Sqrt( fDistSquared23 );
	//FANG1 m_Pos = rPoint2 + P2P3*0.5f;
	m_Pos.Mul(P2P3, 0.5f);
	m_Pos.Add(rPoint2);

	CenterToPoint.Sub(rPoint1, m_Pos);
	fRadiusSquared = CenterToPoint.MagSq();
	if( fRadiusSquared > m_fRadius*m_fRadius ) 
	{
		m_fRadius = fmath_Sqrt( fRadiusSquared );
	}

	m_fRadius *= __EPSILON;

	#undef __EPSILON
}







