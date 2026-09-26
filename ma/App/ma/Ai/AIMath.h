//////////////////////////////////////////////////////////////////////////////////////
// AIMath.h - 
//
// Author: Pat MacKellar 
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
// 04/11/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIMATH_H_
#define _AIMATH_H_ 1

#include "fang.h"
#include "fmath.h"
#include "aimath.h"

#if defined(__cplusplus)
extern "C" {
#endif



//
//  Cylinder, origin is at one of the ends.
//
class CFCyln
{
public:
	CFVec3 m_Origin;
	CFVec3 m_Axis;
	f32 m_fWidth;
	f32 m_fHeight;

	BOOL IntersectSphereApprox(const CFSphere &Sphere)
	{
		CFVec3 LocalSphereOrigin;
 		f32 fMaxDim = m_fWidth;

		if (m_fWidth < m_fHeight)
		{
			fMaxDim = m_fHeight;
		}

		//bounding sphere first
		f32 fDist2 = (Sphere.m_Pos - m_Origin).Mag2();
		if (fDist2 < (fMaxDim+Sphere.m_fRadius)*(Sphere.m_fRadius+fMaxDim))
		{
			LocalSphereOrigin = (Sphere.m_Pos - m_Origin);
			if (m_fWidth < m_fHeight*0.5f)
			{
				fMaxDim = m_fHeight*0.5f;
			}
			f32 fDot = m_Axis.Dot(LocalSphereOrigin);
			if (fDot> 0.0f)
			{
				LocalSphereOrigin = Sphere.m_Pos - m_Axis*fDot;
				if ((LocalSphereOrigin - m_Origin).Mag2() < (m_fWidth+Sphere.m_fRadius)*(m_fWidth+Sphere.m_fRadius))
				{
					//hit
					return TRUE;
				}
			}
		}
		return FALSE;
	};

	FCLASS_STACKMEM_NOALIGN( CFCyln );
};


FCLASS_ALIGN_PREFIX class CFCylnA
{
public:
	CFVec3A m_Origin;
	CFVec3A m_Axis;
	f32 m_fWidth;
	f32 m_fHeight;

	BOOL IntersectSphereApprox(const CFSphereA &Sphere)
	{
		CFVec3A LocalSphereOrigin;

 		f32 fMaxDim = m_fWidth;
		if (m_fWidth < m_fHeight)
		{
			fMaxDim = m_fHeight;
		}
		//bounding sphere first
		f32 fDist2 = Sphere.m_Pos.DistSq(m_Origin);
		if (fDist2 < (fMaxDim+Sphere.m_fRadius)*(fMaxDim+Sphere.m_fRadius))
		{
			if (m_fWidth < m_fHeight*0.5f)
			{
				fMaxDim = m_fHeight*0.5f;
			}
			LocalSphereOrigin.Sub(Sphere.m_Pos, m_Origin);
			f32 fDot = m_Axis.Dot(LocalSphereOrigin);
			if (fDot> 0.0f)
			{
				LocalSphereOrigin.Mul(m_Axis, -1.0f*fDot);
				LocalSphereOrigin.Add(Sphere.m_Pos);

				if (LocalSphereOrigin.DistSq(m_Origin) < (m_fWidth+Sphere.m_fRadius)*(m_fWidth+Sphere.m_fRadius))
				{
					//hit
					return TRUE;
				}
			}
		}
		return FALSE;
	};

	FCLASS_STACKMEM_ALIGN( CFCylnA );
} FCLASS_ALIGN_SUFFIX;



#if defined(__cplusplus)
}
#endif



#endif

