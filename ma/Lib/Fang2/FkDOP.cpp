//////////////////////////////////////////////////////////////////////////////////////
// FkDOP.cpp - Fang kDOP utility functions
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
// 05/08/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "FkDOP.h"
#include "fmesh_coll.h"
#include "fxfm.h"

#if !FANG_PRODUCTION_BUILD
	#include "fdraw.h"
	#include "frenderer.h"
#endif

//////////////////////////////////////////////////////////////////////////////////////
// Local defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _TRACK_USAGE	FALSE


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

// These are the normals used for determining the kDOP
// intervals for a particular piece of geometry.
const FkDOP_DOPDescriptor_t FkDOP_aDesc[FkDOP_MAX_kDOPS] =
{
	/////////
	// 6-DOP
	FkDOP_DOPDescriptor_t( 
	6,	// 6 sides
	3,	// 3 actual DOP normals
	3,	// 3 actual Edge normals
	// Use the standard box normals
	CFVec3A(1.f,			0.f,			0.f),
	CFVec3A(0.f,			1.f,			0.f),
	CFVec3A(0.f,			0.f,			1.f),
	// Dummy
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	// Use the standard box edge normals
	CFVec3A(1.f,			0.f,			0.f),
	CFVec3A(0.f,			1.f,			0.f),
	CFVec3A(0.f,			0.f,			1.f),
	// Dummy
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f),
	CFVec3A(0.f,			0.f,			0.f) ),

	/////////
	// 14-DOP
	FkDOP_DOPDescriptor_t( 
	14,	// 14 sides
	7,	// 7 actual DOP normals
	9,	// 9 actual DOP edge normals
	// Use the standard box normals
	CFVec3A( 1.f,			0.f,			0.f ),
	CFVec3A( 0.f,			1.f,			0.f ),
	CFVec3A( 0.f,			0.f,			1.f ),
	// Chop off the corners of the box	
	CFVec3A( 0.577350269f,	0.577350269f,	0.577350269f ),
	CFVec3A( 0.577350269f,	0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	0.577350269f ),
	// Dummy
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	// Edge normals for a 14-DOP
	CFVec3A( 1.f,			0.f,			0.f ),
	CFVec3A( 0.f,			1.f,			0.f ),
	CFVec3A( 0.f,			0.f,			1.f ),
	CFVec3A( 0.f,			0.707106781f,	0.707106781f ),//
	CFVec3A( 0.f,			0.707106781f,	-0.707106781f ),//
	CFVec3A( 0.707106781f,	0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			0.707106781f ),
	CFVec3A( 0.707106781f,	-0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			-0.707106781f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ) ),
	
	/////////
	// 18-DOP
	FkDOP_DOPDescriptor_t( 
	18,	// 18 sides
	9,	// 9 actual DOP normals
	13,	// 13 actual DOP edge normals
	// Use the standard box normals
	CFVec3A( 1.f,			0.f,			0.f ),
	CFVec3A( 0.f,			1.f,			0.f ),
	CFVec3A( 0.f,			0.f,			1.f ),
	// Chop off the edges
	CFVec3A( 0.f,			0.707106781f,	0.707106781f ),
	CFVec3A( 0.f,			0.707106781f,	-0.707106781f ),
	CFVec3A( 0.707106781f,	0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			0.707106781f ),
	CFVec3A( 0.707106781f,	-0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			-0.707106781f ),
	// Dummy
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	CFVec3A( 0.f,			0.f,			0.f ),
	// Edge normals for a 18-DOP
	CFVec3A( 1.f,			0.f,			0.f ),
	CFVec3A( 0.f,			1.f,			0.f ),
	CFVec3A( 0.f,			0.f,			1.f ),
	CFVec3A( 0.f,			0.707106781f,	0.707106781f ),
	CFVec3A( 0.f,			0.707106781f,	-0.707106781f ),
	CFVec3A( 0.707106781f,	0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			0.707106781f ),
	CFVec3A( 0.707106781f,	-0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			-0.707106781f ),
	CFVec3A( 0.577350269f,	0.577350269f,	0.577350269f ),
	CFVec3A( 0.577350269f,	0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	0.577350269f ) ),
	
	/////////
	// 26-DOP
	FkDOP_DOPDescriptor_t( 
	26,	// 26 sides
	13,	// 13 actual DOP normals
	13,	// 13 actual DOP edge normals
	// Use the standard box normals
	CFVec3A( 1.f,			0.f,			0.f ),
	CFVec3A( 0.f,			1.f,			0.f ),
	CFVec3A( 0.f,			0.f,			1.f ),
	// Chop off the corners of the box	
	CFVec3A( 0.577350269f,	0.577350269f,	0.577350269f ),
	CFVec3A( 0.577350269f,	0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	0.577350269f ),
	// Chop off the edges
	CFVec3A( 0.f,			0.707106781f,	0.707106781f ),
	CFVec3A( 0.f,			0.707106781f,	-0.707106781f ),
	CFVec3A( 0.707106781f,	0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			0.707106781f ),
	CFVec3A( 0.707106781f,	-0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			-0.707106781f ),
	// Edge normals for a 26-DOP
	CFVec3A( 1.f,			0.f,			0.f ),
	CFVec3A( 0.f,			1.f,			0.f ),
	CFVec3A( 0.f,			0.f,			1.f ),
	CFVec3A( 0.f,			0.707106781f,	0.707106781f ),
	CFVec3A( 0.f,			0.707106781f,	-0.707106781f ),
	CFVec3A( 0.707106781f,	0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			0.707106781f ),
	CFVec3A( 0.707106781f,	-0.707106781f,	0.f ),
	CFVec3A( 0.707106781f,	0.f,			-0.707106781f ),
	CFVec3A( 0.577350269f,	0.577350269f,	0.577350269f ),
	CFVec3A( 0.577350269f,	0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	-0.577350269f ),
	CFVec3A( 0.577350269f,	-0.577350269f,	0.577350269f ) )
};


#if !FANG_PRODUCTION_BUILD
	// Externally accessible global variables
	FConvHull_Plane_t FConvHull_PlaneBuffer[FCONVHULL_MAX_PLANES];
	CFVec3A FConvHull_vPointBuffer[FCONVHULL_MAX_HULL_POINTS];
	u32 FConvHull_nPointBufferCount;
#endif // !FANG_PRODUCTION BUILD


//////////////////////////////////////////////////////////////////////////////////////
// Static variables:
//////////////////////////////////////////////////////////////////////////////////////

#if _TRACK_USAGE
	static u32 _nkDOPAndSphereCalls = 0;
	static u32 _nkDOPAndSphereDepth = 0;
	static u32 _nkDOPAndRayCalls = 0;
	static u32 _nkDOPAndRayDepth = 0;
#endif

static CFVec3A _vPlanePoint[FCONVHULL_MAX_PLANES];
static CFVec3A _vPlaneNorm[FCONVHULL_MAX_PLANES];


//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

//static BOOL _PointIsInkDOP( const CFVec3A *pTestPoint, const FkDOP_Interval_t *pIntervals, u32 nkDOPType );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//	Function returns true if the given point is within the intervals
//	supplied for each axis of the active kDOP.
//
BOOL fkdop_PointIsInkDOP( const CFVec3A *pTestPoint, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( pTestPoint && pIntervals );
	
	f32 fDot;
	u32 iKookyMcGrooky;
	CFVec3A vDiff;
	
	u32 nNorms = FkDOP_aDesc[nkDOPType].nNormalCount;
	const CFVec3A *pNormals = FkDOP_aDesc[nkDOPType].avNormals;
	
	// Make sure this point is contained within all of the planes
	for ( iKookyMcGrooky = 0; iKookyMcGrooky < nNorms; iKookyMcGrooky++ )
	{
		fDot = pTestPoint->Dot( pNormals[iKookyMcGrooky] );
		if ( fDot + 0.001f < pIntervals[iKookyMcGrooky].fMin || fDot - 0.001f > pIntervals[iKookyMcGrooky].fMax )
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


//
//
//
BOOL fkdop_IntersectkDOPAndkDOP( const FkDOP_Interval_t *pIntervals1, u32 nkDOPType1, const FkDOP_Interval_t *pIntervals2, u32 nkDOPType2, CFVec3A *pkDOP2Movement )
{
	u16 i, nNorms, nCompareType = nkDOPType1;
	
	// First we need to find the least common denominator
	if (   (nkDOPType1 == FkDOP_14_DOP && nkDOPType2 == FkDOP_18_DOP)
		|| (nkDOPType2 == FkDOP_14_DOP && nkDOPType1 == FkDOP_18_DOP) )
	{
		// 18 sided kDOPs do not build upon the same normals as the 14 kDOP.  They only have the 6 sided kDOP in common
		nCompareType = FkDOP_6_DOP;
	}
	else if ( nCompareType > nkDOPType2 )
	{
		// All other kDOP combinations build upon one another (26 kDOP has all of the 
		// normals of the 18, 14 and 6; 18 and 14 kDOPs have the normals contained in the 6)
		nCompareType = nkDOPType2;
	}

	nNorms = FkDOP_aDesc[nCompareType].nNormalCount;
	
	// Make sure the two kDOPs overlap in all intervals
	f32 fNormalMovement;
	for ( i = 0; i < nNorms; i++ )
	{
		fNormalMovement = pkDOP2Movement->Dot( FkDOP_aDesc[nCompareType].avNormals[i] );

		if ( fNormalMovement > 0 )
		{
			if ( pIntervals1[i].fMax < pIntervals2[i].fMin || pIntervals1[i].fMin > (pIntervals2[i].fMax + fNormalMovement) )
			{
				// Found an interval where they do not overlap, so there can be no collision
				return FALSE;
			}
		}
		else
		{
			if ( pIntervals1[i].fMax < (pIntervals2[i].fMin + fNormalMovement) || pIntervals1[i].fMin > pIntervals2[i].fMax )
			{
				// Found an interval where they do not overlap, so there can be no collision
				return FALSE;
			}
		}

	}
	
	return TRUE;
}


//
//
//
BOOL fkdop_IntersectkDOPAndkDOP( const FkDOP_Interval_t *pIntervals1, u32 nkDOPType1, const FkDOP_Interval_t *pIntervals2, u32 nkDOPType2 )
{
	u16 i, nNorms, nCompareType = nkDOPType1;
	
	// First we need to find the least common denominator
	if (   (nkDOPType1 == FkDOP_14_DOP && nkDOPType2 == FkDOP_18_DOP)
		|| (nkDOPType2 == FkDOP_14_DOP && nkDOPType1 == FkDOP_18_DOP) )
	{
		// 18 sided kDOPs do not build upon the same normals as the 14 kDOP.  They only have the 6 sided kDOP in common
		nCompareType = FkDOP_6_DOP;
	}
	else if ( nCompareType > nkDOPType2 )
	{
		// All other kDOP combinations build upon one another (26 kDOP has all of the 
		// normals of the 18, 14 and 6; 18 and 14 kDOPs have the normals contained in the 6)
		nCompareType = nkDOPType2;
	}

	nNorms = FkDOP_aDesc[nCompareType].nNormalCount;
	
	// Make sure the two kDOPs overlap in all intervals
	for ( i = 0; i < nNorms; i++ )
	{
		if ( pIntervals1[i].fMax < pIntervals2[i].fMin || pIntervals1[i].fMin > pIntervals2[i].fMax )
		{
			// Found an interval where they do not overlap, so there can be no collision
			return FALSE;
		}
	}
	
	return TRUE;
}


//
//	Returns TRUE if the sphere POTENTIALLY intersects the kDOP (including wholly contained)
//	In limited scenarios (near the corners of the kDOP, this function may return false 
//	positives (return TRUE where there is no collision).
//
BOOL fkdop_IntersectkDOPAndSphere( f32 *pafCenterIntervals, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( pafCenterIntervals && pIntervals );
	
	#if _TRACK_USAGE
		_nkDOPAndSphereCalls++;
	#endif

	u16 i;
	u16 nNorms = FkDOP_aDesc[nkDOPType].nNormalCount;
	
	// Make sure this sphere overlaps the intervals in all directions
	for ( i = 0; i < nNorms; i++, pafCenterIntervals++ )
	{
//		fCenter = pCenter->Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		
		if ( *pafCenterIntervals + fRadius < pIntervals[i].fMin || *pafCenterIntervals - fRadius > pIntervals[i].fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndSphereDepth += i;
			#endif
			return FALSE;
		}
	}
	
	#if _TRACK_USAGE
		_nkDOPAndSphereDepth += i;
	#endif
	
	return TRUE;
}


//
//	Returns TRUE if the sphere POTENTIALLY intersects the kDOP (including wholly contained)
//	In limited scenarios (near the corners of the kDOP, this function may return false 
//	positives (return TRUE where there is no collision).
//
BOOL fkdop_IntersectkDOPAndSphere( const CFVec3A *pCenter, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( pCenter && pIntervals );
	
	#if _TRACK_USAGE
		_nkDOPAndSphereCalls++;
	#endif

	u16 i;
	u16 nNorms = FkDOP_aDesc[nkDOPType].nNormalCount;
	f32 fCenter;
	
	// Make sure this sphere overlaps the intervals in all directions
	for ( i = 0; i < nNorms; i++ )
	{
		fCenter = pCenter->Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		
		if (   fCenter + fRadius < pIntervals[i].fMin 
			|| fCenter - fRadius > pIntervals[i].fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndSphereDepth += i;
			#endif
			return FALSE;
		}
	}
	
	#if _TRACK_USAGE
		_nkDOPAndSphereDepth += i;
	#endif
	
	return TRUE;
}


//
//	Returns unit distance along ray if the ray intersects the 
//	kDOP (including wholly contained), and -1.f if it does not.
//
f32 fkdop_IntersectkDOPAndRay( f32 *pafStartIntervals, f32 *pafEndIntervals, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( pafStartIntervals && pafEndIntervals && pIntervals );
	
	u8 i;
	u8 nNorms = (u8)FkDOP_aDesc[nkDOPType].nNormalCount;
	f32 fSwap, fInvSpan;
	f32 fMax = 1.f;
	f32 fMin = 0.f;
	
	// Make sure this ray overlaps the intervals in all directions
	for ( i = 0; i < nNorms; i++, pafStartIntervals++, pafEndIntervals++ )
	{
		// If the end is greater than the start, swap for easier comparisons
		if ( *pafEndIntervals < *pafStartIntervals )
		{
			if ( *pafEndIntervals > pIntervals[i].fMax || *pafStartIntervals < pIntervals[i].fMin )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
				
			f32 fDiff = *pafStartIntervals - *pafEndIntervals;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( *pafEndIntervals < pIntervals[i].fMin )
			{
				fSwap = (*pafStartIntervals - pIntervals[i].fMin) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( *pafStartIntervals > pIntervals[i].fMax )
			{
				fSwap = (*pafStartIntervals - pIntervals[i].fMax) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		else
		{
			if ( *pafEndIntervals < pIntervals[i].fMin || *pafStartIntervals > pIntervals[i].fMax )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
			
			f32 fDiff = *pafEndIntervals - *pafStartIntervals;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( *pafEndIntervals > pIntervals[i].fMax )
			{
				fSwap = (pIntervals[i].fMax - *pafStartIntervals) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( *pafStartIntervals < pIntervals[i].fMin )
			{
				fSwap = (pIntervals[i].fMin - *pafStartIntervals) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		
		if ( fMin > fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndRayDepth += i;
			#endif
			return -1.f;
		}
	}

	return fMin;
}


//
//	Returns unit distance along ray if the ray intersects the 
//	kDOP (including wholly contained), and -1.f if it does not.
//
f32 fkdop_IntersectkDOPAndRay( const CFVec3A *pStart, const CFVec3A *pEnd, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( pStart && pEnd && pIntervals );
	
	#if _TRACK_USAGE
		_nkDOPAndRayCalls++;
	#endif

	u8 i;
	u8 nNorms = (u8)FkDOP_aDesc[nkDOPType].nNormalCount;
	f32 fStart, fEnd, fSwap, fInvSpan;
	f32 fMax = 1.f;
	f32 fMin = 0.f;
	
	// Make sure this ray overlaps the intervals in all directions
	for ( i = 0; i < 3; i++ )
	{
		fStart 	= pStart->a[i];
		fEnd 	= pEnd->a[i];

		// If the end is greater than the start, swap for easier comparisons
		if ( fEnd < fStart )
		{
			if ( fEnd > pIntervals[i].fMax || fStart < pIntervals[i].fMin )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
			
			f32 fDiff = fStart - fEnd;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd < pIntervals[i].fMin )
			{
				fSwap = (fStart - pIntervals[i].fMin) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart > pIntervals[i].fMax )
			{
				fSwap = (fStart - pIntervals[i].fMax) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		else
		{
			if ( fEnd < pIntervals[i].fMin || fStart > pIntervals[i].fMax )

			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
			
			f32 fDiff = fEnd - fStart;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd > pIntervals[i].fMax )
			{
				fSwap = (pIntervals[i].fMax - fStart) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart < pIntervals[i].fMin )
			{
				fSwap = (pIntervals[i].fMin - fStart) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		
		if ( fMin > fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndRayDepth += i;
			#endif
			return -1.f;
		}
	}
	
	// Make sure this ray overlaps the intervals in all directions
	for ( ; i < nNorms; i++ )
	{
		fStart 	= pStart->Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		fEnd 	= pEnd->Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );

		// If the end is greater than the start, swap for easier comparisons
		if ( fEnd < fStart )
		{
			if ( fEnd > pIntervals[i].fMax || fStart < pIntervals[i].fMin )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
				
			f32 fDiff = fStart - fEnd;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd < pIntervals[i].fMin )
			{
				fSwap = (fStart - pIntervals[i].fMin) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart > pIntervals[i].fMax )
			{
				fSwap = (fStart - pIntervals[i].fMax) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		else

		{
			if ( fEnd < pIntervals[i].fMin || fStart > pIntervals[i].fMax )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
			
			f32 fDiff = fEnd - fStart;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd > pIntervals[i].fMax )
			{
				fSwap = (pIntervals[i].fMax - fStart) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart < pIntervals[i].fMin )
			{
				fSwap = (pIntervals[i].fMin - fStart) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		
		if ( fMin > fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndRayDepth += i;
			#endif
			return -1.f;
		}
	}

	#if _TRACK_USAGE
		_nkDOPAndRayDepth += i;
	#endif
	return fMin;
}


//
//	Returns TRUE if the ray intersects the kDOP (including wholly contained)
//
BOOL fkdop_IntersectkDOPAndCapsule( f32 *pafStart, f32 *pafEnd, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( pafStart && pafEnd && pIntervals );
	
	#if _TRACK_USAGE
		_nkDOPAndRayCalls++;
	#endif

	u32 i;
	u32 nNorms = FkDOP_aDesc[nkDOPType].nNormalCount;
	f32 fSwap, fInvSpan;
	f32 fMax = 1.f;
	f32 fMin = 0.f;
	f32 fIntervalMin, fIntervalMax;
	
	// Make sure this ray overlaps the intervals in all directions
	for ( i = 0; i < nNorms; i++, pafStart++, pafEnd++ )
	{
		fIntervalMin = pIntervals[i].fMin - fRadius;
		fIntervalMax = pIntervals[i].fMax + fRadius;
		
		// If the end is greater than the start, swap for easier comparisons
		if ( (*pafEnd) < (*pafStart) )
		{
			if ( (*pafEnd) > fIntervalMax || (*pafStart) < fIntervalMin )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return FALSE;
			}
				
			f32 fDiff = (*pafStart) - (*pafEnd);
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( (*pafEnd) < fIntervalMin )
			{
				fSwap = ((*pafStart) - fIntervalMin) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( (*pafStart) > fIntervalMax )
			{
				fSwap = ((*pafStart) - fIntervalMax) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		else
		{
			if ( (*pafEnd) < fIntervalMin || (*pafStart) > fIntervalMax )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return FALSE;
			}
			
			f32 fDiff = (*pafEnd) - (*pafStart);
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( (*pafEnd) > fIntervalMax )
			{
				fSwap = (fIntervalMax - (*pafStart)) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( (*pafStart) < fIntervalMin )
			{
				fSwap = (fIntervalMin - (*pafStart)) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		
		if ( fMin > fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndRayDepth += i;
			#endif
			return FALSE;
		}
	}
	
	#if _TRACK_USAGE
		_nkDOPAndRayDepth += i;
	#endif
	return TRUE;
}


//
//	Returns TRUE if the ray intersects the kDOP (including wholly contained)
//
BOOL fkdop_IntersectkDOPAndCapsule( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fRadius, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( pStart && pEnd && pIntervals );
	
	#if _TRACK_USAGE
		_nkDOPAndRayCalls++;
	#endif

	u32 i;
	u32 nNorms = FkDOP_aDesc[nkDOPType].nNormalCount;
	f32 fStart, fEnd, fSwap, fInvSpan;
	f32 fMax = 1.f;
	f32 fMin = 0.f;
	f32 fIntervalMin, fIntervalMax;
	
	// Make sure this ray overlaps the intervals in all directions
	for ( i = 0; i < 3; i++ )
	{
		fStart 	= pStart->a[i];//pStart->Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		fEnd 	= pEnd->a[i];//Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );

		fIntervalMin = pIntervals[i].fMin - fRadius;
		fIntervalMax = pIntervals[i].fMax + fRadius;
		
		// If the end is greater than the start, swap for easier comparisons
		if ( fEnd < fStart )
		{
			if ( fEnd > fIntervalMax || fStart < fIntervalMin )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return FALSE;
			}
			
			f32 fDiff = fStart - fEnd;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd < fIntervalMin )
			{
				fSwap = (fStart - fIntervalMin) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart > fIntervalMax )
			{
				fSwap = (fStart - fIntervalMax) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		else
		{
			if ( fEnd < fIntervalMin || fStart > fIntervalMax )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return FALSE;
			}
			
			f32 fDiff = fEnd - fStart;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd > fIntervalMax )
			{
				fSwap = (fIntervalMax - fStart) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart < fIntervalMin )
			{
				fSwap = (fIntervalMin - fStart) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		
		if ( fMin > fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndRayDepth += i;
			#endif
			return FALSE;
		}
	}
	
	// Make sure this ray overlaps the intervals in all directions
	for ( ; i < nNorms; i++ )
	{
		fStart 	= pStart->Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		fEnd 	= pEnd->Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );

		fIntervalMin = pIntervals[i].fMin - fRadius;
		fIntervalMax = pIntervals[i].fMax + fRadius;
		
		// If the end is greater than the start, swap for easier comparisons
		if ( fEnd < fStart )
		{
			if ( fEnd > fIntervalMax || fStart < fIntervalMin )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return FALSE;
			}
				
			f32 fDiff = fStart - fEnd;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd < fIntervalMin )
			{
				fSwap = (fStart - fIntervalMin) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart > fIntervalMax )
			{
				fSwap = (fStart - fIntervalMax) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		else
		{
			if ( fEnd < fIntervalMin || fStart > fIntervalMax )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return FALSE;
			}
			
			f32 fDiff = fEnd - fStart;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd > fIntervalMax )
			{
				fSwap = (fIntervalMax - fStart) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart < fIntervalMin )
			{
				fSwap = (fIntervalMin - fStart) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		
		if ( fMin > fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndRayDepth += i;
			#endif
			return FALSE;
		}
	}
	
	#if _TRACK_USAGE
		_nkDOPAndRayDepth += i;
	#endif
	return TRUE;
}

/*
//
//	
//
f32 fkdop_IntersectkDOPAndEdge( f32 *afStartDots, f32 *afEndDots, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	FASSERT( afStartDots && afEndDots && pIntervals );
	
	#if _TRACK_USAGE
		_nkDOPAndRayCalls++;
	#endif

	u8 i;
	u8 nNorms = (u8)FkDOP_aDesc[nkDOPType].nNormalCount;
	f32 fStart, fEnd, fSwap, fInvSpan;
	f32 fMax = 1.f;
	f32 fMin = 0.f;
	
	// Make sure this ray overlaps the intervals in all directions
	for ( i = 0; i < nNorms; i++ )
	{
		fStart = afStartDots[i];
		fEnd = afEndDots[i];
		
		// If the end is greater than the start, swap for easier comparisons
		if ( fEnd < fStart )
		{
			if ( fEnd > pIntervals[i].fMax || fStart < pIntervals[i].fMin )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
			
			f32 fDiff = fStart - fEnd;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd < pIntervals[i].fMin )
			{
				fSwap = (fStart - pIntervals[i].fMin) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart > pIntervals[i].fMax )
			{
				fSwap = (fStart - pIntervals[i].fMax) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		else
		{
			if ( fEnd < pIntervals[i].fMin || fStart > pIntervals[i].fMax )
			{
				#if _TRACK_USAGE
					_nkDOPAndRayDepth += i;
				#endif
				return -1.f;
			}
			
			f32 fDiff = fEnd - fStart;
			if ( fmath_Abs( fDiff ) < 0.0001f )
			{
				continue;
			}
			
			FASSERT( fDiff != 0.f );
			fInvSpan = fmath_Inv( fDiff );
			
			if ( fEnd > pIntervals[i].fMax )
			{
				fSwap = (pIntervals[i].fMax - fStart) * fInvSpan;
				if ( fSwap < fMax )
				{
					fMax = fSwap;
				}
			}
			
			if ( fStart < pIntervals[i].fMin )
			{
				fSwap = (pIntervals[i].fMin - fStart) * fInvSpan;
				if ( fSwap > fMin )
				{
					fMin = fSwap;
				}
			}
		}
		
		if ( fMin > fMax )
		{
			#if _TRACK_USAGE
				_nkDOPAndRayDepth += i;
			#endif
			return -1.f;
		}
	}
	
	#if _TRACK_USAGE
		_nkDOPAndRayDepth += i;
	#endif

	return fMin;
}

//
//	Returns TRUE if the ray intersects the kDOP (including wholly contained)
//
BOOL fkdop_IntersectkDOPAndTriangle( const CFVec3A *avTri, const CFVec3A *pTriNormal, const FkDOP_Tree_t *pTree, const FkDOP_Interval_t *pIntervals, u32 nkDOPType )
{
	CFVec3A vTemp;
	f32 afDots[3][FKDOP_MAX_AXES];
	
	u8 i;
	u8 nNorms = (u8)FkDOP_aDesc[nkDOPType].nNormalCount;
	
	// Test the triangle to see if all points are on one side of a plane
	
	// First three axes are easy
	for ( i = 0; i < 3; i++ )
	{
		afDots[0][i] = avTri[0].a[i];
		afDots[1][i] = avTri[1].a[i];
		afDots[2][i] = avTri[2].a[i];
		if (   afDots[0][i] < pIntervals[i].fMin
			&& afDots[1][i] < pIntervals[i].fMin
			&& afDots[2][i] < pIntervals[i].fMin )
		{
			return FALSE;
		}
		if (   afDots[0][i] > pIntervals[i].fMax
			&& afDots[1][i] > pIntervals[i].fMax
			&& afDots[2][i] > pIntervals[i].fMax )
		{
			return FALSE;
		}
	}
	
	// Remainder require a dot product
	for ( ; i < nNorms; i++ )
	{
		afDots[0][i] = avTri[0].Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		afDots[1][i] = avTri[1].Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		afDots[2][i] = avTri[2].Dot( FkDOP_aDesc[nkDOPType].avNormals[i] );
		if (   afDots[0][i] < pIntervals[i].fMin
			&& afDots[1][i] < pIntervals[i].fMin
			&& afDots[2][i] < pIntervals[i].fMin )
		{
			return FALSE;
		}
		if (   afDots[0][i] > pIntervals[i].fMax
			&& afDots[1][i] > pIntervals[i].fMax
			&& afDots[2][i] > pIntervals[i].fMax )
		{
			return FALSE;
		}
	}
	
	f32 fDist;
	f32 afDistance[3];
	afDistance[0] = fkdop_IntersectkDOPAndEdge( afDots[0], afDots[1], pIntervals, nkDOPType );
	afDistance[1] = fkdop_IntersectkDOPAndEdge( afDots[1], afDots[2], pIntervals, nkDOPType );
	afDistance[2] = fkdop_IntersectkDOPAndEdge( afDots[2], afDots[0], pIntervals, nkDOPType );
	
	if ( afDistance[0] == -1.f && afDistance[1] == -1.f && afDistance[2] == -1.f )
	{
		// None of the edges intersect (though we still could have intersection....
		
		// Find two kDOP points on opposite sides of the plane of the poly:
		CFVec3A vPointBelow, vPointAbove;
		BOOL8 bBelowNeeded = TRUE, bAboveNeeded = TRUE;
		for ( i = 0; i < pTree->nRootkDOPVertCount; i++ )
		{
			fDist = (avTri[0].v3 - pTree->paRootkDOPVerts[i]).Dot( pTriNormal->v3 );
			if ( bBelowNeeded && fDist < 0.f  )
			{
				vPointBelow.Set( pTree->paRootkDOPVerts[i] );
				bBelowNeeded = FALSE;
			}
			else if ( bAboveNeeded && fDist > 0.f )
			{
				vPointAbove.Set( pTree->paRootkDOPVerts[i] );
				bAboveNeeded = FALSE;
			}
			
			// If we have found two points, use them
			if ( !(bAboveNeeded | bBelowNeeded) )
			{
				break;
			}
		}

		if ( i == pTree->nRootkDOPVertCount )
		{
			// All points were either above or below
			return FALSE;
		}

		// Get some ray info
		static CFVec3A vRayNormal, vODiff, vDiff1, vDiff2, vCross;
		vRayNormal.Sub( vPointBelow, vPointAbove );
		fDist = vRayNormal.Mag();
		vRayNormal.Mul( fmath_Inv( fDist ) );
		
		// See if the ray formed by the point above and the point below intersects the tri
		
		f32 fFaceDot = -vRayNormal.Dot( *pTriNormal );
		
		vODiff.Sub( vPointAbove, avTri[0] );
		f32 fDistFromPlane = vODiff.Dot( *pTriNormal );

		FASSERT( fFaceDot >= 0.0001f );
		fDistFromPlane *= fmath_Inv( fFaceDot );
		if ( fDistFromPlane > fDist )
		{
			// The ray is not long enough to intersect the plane
			return FALSE;
		}

		vDiff1.Sub( avTri[1], avTri[0] );
		vDiff2.Sub( avTri[2], avTri[0] );

		vCross.Cross( *pTriNormal, vDiff2 );
		f32 fDet = vDiff1.Dot( vCross );
		if ( fmath_Abs( fDet ) < 0.0001f )
		{
			return FALSE;
		}
		
		FASSERT( fDet != 0.0f );
		fDet = fmath_Inv( fDet );
		
		f32 fU = vODiff.Dot( vCross ) * fDet;
		if ( fU < 0.f || fU > 1.f )
		{
			return FALSE;
		}
		
		vCross.Cross( vODiff, vDiff1 );
		f32 fV = vRayNormal.Dot( vCross ) * fDet;
		if ( fV < 0.f || fU + fV > 1.f )
		{
			return FALSE;
		}
		
		return TRUE;
	}
	
	return TRUE;
}
*/

#if !FANG_PRODUCTION_BUILD
//
//
//
CFVec3A* fkdop_CalculateVerts( const FkDOP_Interval_t *pIntervals, BOOL bGenPlanes, u32 nkDOPType )
{
	u32 i, ii, iii, nVert;
	CFVec3A vResult, vTemp;
	CFVec3A vDiff;
	
	FConvHull_nPointBufferCount = 0;
	
	const FkDOP_DOPDescriptor_t *pDesc = &FkDOP_aDesc[nkDOPType];
	u32 nNorms = pDesc->nNormalCount;
	u32 nPlanes = nNorms << 1;
	
	if ( bGenPlanes )
	{
		fang_MemSet( FConvHull_PlaneBuffer, 0, sizeof( FConvHull_Plane_t ) * nPlanes );
	}
	
	for ( i = 0; i < nNorms; i++ )
	{
		_vPlaneNorm[i].Set( pDesc->avNormals[i] );
		_vPlanePoint[i].Mul( _vPlaneNorm[i], pIntervals[i].fMax );
		_vPlaneNorm[i + nNorms].Set( pDesc->avNormals[i] );
		_vPlanePoint[i + nNorms ].Mul( _vPlaneNorm[i], pIntervals[i].fMin );

		if ( bGenPlanes )
		{
			// Set the plane normals		
			FConvHull_PlaneBuffer[i].vNormal.Set( pDesc->avNormals[i] );
			FConvHull_PlaneBuffer[i + nNorms].vNormal.ReceiveNegative( pDesc->avNormals[i] );
		}
	}
	
	for ( i = 0; i < nPlanes - 2; i++ )
	{
		for ( ii = i + 1; ii < nPlanes - 1; ii++ )
		{
			for ( iii = ii + 1; iii < nPlanes; iii++ )
			{
				// Calculate the determinant
				f32 fDet = vTemp.Cross( _vPlaneNorm[i], _vPlaneNorm[ii] ).Dot( _vPlaneNorm[iii] );
				if ( fDet > -0.001f && fDet < 0.001f )
				{
					continue;
				}
					
				vResult.Cross( _vPlaneNorm[ii], _vPlaneNorm[iii] ).Mul( _vPlanePoint[i].Dot( _vPlaneNorm[i] ) );
				vTemp.Cross( _vPlaneNorm[iii], _vPlaneNorm[i] ).Mul( _vPlanePoint[ii].Dot( _vPlaneNorm[ii] ) );
				vResult.Add( vTemp );
				vTemp.Cross( _vPlaneNorm[i], _vPlaneNorm[ii] ).Mul( _vPlanePoint[iii].Dot( _vPlaneNorm[iii] ) );
				vResult.Add( vTemp ).Mul( 1.f / fDet );
				
				if ( !fkdop_PointIsInkDOP( &vResult, pIntervals, nkDOPType ) )
				{
					continue;
				}
				
				// Verify that we don't already have this point
				for ( nVert = 0; nVert < FConvHull_nPointBufferCount; nVert++ )
				{
					vDiff.Sub( FConvHull_vPointBuffer[nVert], vResult );
					f32 fTest = vDiff.MagSq();
					if ( fTest < 0.001f )
						break;
				}
				
				// Add this point to the list, if we need to
				if ( nVert == FConvHull_nPointBufferCount )
				{
					FASSERT( FConvHull_nPointBufferCount < FCONVHULL_MAX_HULL_POINTS );
					FConvHull_vPointBuffer[FConvHull_nPointBufferCount++].Set( vResult );
				}

				if ( bGenPlanes	)
				{
					FASSERT( FConvHull_PlaneBuffer[i].nPointCount < FCONVHULL_MAX_PLANE_POINTS );
					FASSERT( FConvHull_PlaneBuffer[ii].nPointCount < FCONVHULL_MAX_PLANE_POINTS );
					FASSERT( FConvHull_PlaneBuffer[iii].nPointCount < FCONVHULL_MAX_PLANE_POINTS );
					FConvHull_PlaneBuffer[i].AddIndex( nVert );
					FConvHull_PlaneBuffer[ii].AddIndex( nVert );
					FConvHull_PlaneBuffer[iii].AddIndex( nVert );
				}
			}
		}
	}

	if ( bGenPlanes )
	{	
		CFVec3A vRandomDiff, vCross;
		for ( i = 0; i < nPlanes; i++ )
		{
			f32 fSortVal[FCONVHULL_MAX_PLANE_POINTS];
			
			if ( !FConvHull_PlaneBuffer[i].nPointCount )
			{
				continue;
			}
				
			u16 *pPoints = FConvHull_PlaneBuffer[i].nPointIdx;
			
			if ( FConvHull_PlaneBuffer[i].nPointCount < 3 )
			{
				continue;
			}

			vRandomDiff.Sub( FConvHull_vPointBuffer[ pPoints[1] ], FConvHull_vPointBuffer[ pPoints[0] ] );
			vRandomDiff.Unitize();
			vCross.Cross( vRandomDiff, _vPlaneNorm[i] );
			
			fSortVal[0] = -1.570796f;
			fSortVal[1] = 1.570796f;
			
			for ( ii = 2; ii < FConvHull_PlaneBuffer[i].nPointCount; ii++ )
			{
				vDiff.Sub( FConvHull_vPointBuffer[ pPoints[ii] ], FConvHull_vPointBuffer[ pPoints[0] ] );
				vDiff.Unitize();
				f32 x = vCross.Dot( vDiff );
				f32 y = vRandomDiff.Dot( vDiff );
				fSortVal[ii] = fmath_Atan( y, x );
			}
			
			for ( ii = 0; ii < FConvHull_PlaneBuffer[i].nPointCount - 1; ii++ )
			{
				for ( iii = ii + 1; iii < FConvHull_PlaneBuffer[i].nPointCount; iii++ )
				{
					if ( fSortVal[iii] > fSortVal[ii] )
					{
						continue;
					}
					
					f32 fTemp = fSortVal[ii];
					u16 nTemp = pPoints[ii];
					fSortVal[ii] = fSortVal[iii];
					pPoints[ii] = pPoints[iii];
					fSortVal[iii] = fTemp;
					pPoints[iii] = nTemp;
				}
			}
		}
	}

	return FConvHull_vPointBuffer;
}
#endif // !FANG_PRODUCTION_BUILD


//
//
//
void fkdop_GenerateIntervals( CFVec3A *pVerts, u32 nVertCount, FkDOP_Interval_t *pInt, u32 nkDOPType )
{
	FASSERT( pVerts && pInt && nVertCount > 0 );
	
	f32 fCurrent, fMin, fMax;
	u32 i, ii;
	
	// Determine the intervals along the x, y, z axes of the kDOP (we can do this without a dot product)
	for ( i = 0; i < 3; i++ )
	{
		fMin = FMATH_MAX_FLOAT;
		fMax = -FMATH_MAX_FLOAT;
		
		for ( ii = 0; ii < nVertCount; ii++ )
		{
			fCurrent = pVerts[ii].a[i];

			if ( fCurrent < fMin )
			{
				fMin = fCurrent;
			}
			if ( fCurrent > fMax )
			{
				fMax = fCurrent;
			}
		}

		// This COULD be bad (not really sure, yet)
		FASSERT( fMin != fMax );

		// Store the intervals
		pInt[i].fMin = fMin;
		pInt[i].fMax = fMax;
	}

	// Determine the intervals along the axes of the kDOP
	const CFVec3A *pNormals = FkDOP_aDesc[nkDOPType].avNormals;
	for ( i = 3; i < FkDOP_aDesc[nkDOPType].nNormalCount; i++ )
	{
		fMin = FMATH_MAX_FLOAT;
		fMax = -FMATH_MAX_FLOAT;
		
		for ( ii = 0; ii < nVertCount; ii++ )
		{
			fCurrent = pNormals[i].Dot( pVerts[ii] );
			
			if ( fCurrent < fMin )
			{
				fMin = fCurrent;
			}
			if ( fCurrent > fMax )
			{
				fMax = fCurrent;
			}
		}

		// This COULD be bad (not really sure, yet)
		FASSERT( fMin != fMax );

		// Store the intervals
		pInt[i].fMin = fMin;
		pInt[i].fMax = fMax;
	}
}

#if !FANG_PRODUCTION_BUILD

#define	_MAX_kDOP_DRAW_TRIS			80
#define _MAX_kDOP_DRAW_LINES		16

//
//
//
void FkDOP_DrawkDOP( const FkDOP_Interval_t *pInt, const CFMtx43A *pTransMtx, u32 nkDOPType, BOOL bTranslateOnly/*=FALSE*/, CFColorRGBA *pColor/*=NULL*/ ) 
{
	FASSERT( pInt );
	
	// Setup fdraw for rendering of the volumes
	frenderer_Push( FRENDERER_DRAW, NULL );

	// Set up an identity Xfm to push
	CFXfm xfmDOP;
	if ( pTransMtx )
	{
		if ( bTranslateOnly )
		{
			xfmDOP.Identity();
			xfmDOP.BuildTranslation( pTransMtx->m_vPos );
		}
		else
		{
			xfmDOP.BuildFromMtx( *pTransMtx );
		}
	}
	else
	{
		xfmDOP.Identity();
	}
	xfmDOP.PushModel();

	// Set the states we need
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	fkdop_CalculateVerts( pInt, TRUE, nkDOPType );

	u32 nPlanes = FkDOP_aDesc[nkDOPType].nNormalCount << 1;
	
	FDrawVtx_t _vkDOPTriVerts[_MAX_kDOP_DRAW_TRIS * 3];
	FDrawVtx_t _vkDOPLineVerts[_MAX_kDOP_DRAW_LINES * 2];
	FDrawVtx_t *pTriVerts = _vkDOPTriVerts;
	FDrawVtx_t *pLineVerts = _vkDOPLineVerts;
	u32 nLineCount = 0;
	u32 nTriCount = 0;
	
	// Set tri colors
	CFColorRGBA kDOPColor;
	if ( pColor )
	{
		kDOPColor.Set( *pColor );
	}
	else
	{
		kDOPColor.Set( 0.5f, 0.5f, 0.5f, 0.3f );
	}
	
	// Set line colors
	CFColorRGBA LineColor;
	if ( pColor )
	{
		LineColor.Set( *pColor );
		LineColor *= 1.5f;
	}
	else
	{
		LineColor.Set( 0.7f, 0.7f, 0.7f, 0.7f );
	}
	
	u32 i, ii;
	CFVec3A vTemp;
	for ( i = 0; i < nPlanes; i++ )
	{
		if ( FConvHull_PlaneBuffer[i].nPointCount < 3 )
		{
			continue;
		}

		FASSERT( FConvHull_PlaneBuffer[i].nPointCount < _MAX_kDOP_DRAW_LINES );
		
		// If we're going to exceed the buffer, then flush it
		if ( nTriCount + (FConvHull_PlaneBuffer[i].nPointCount - 2) > _MAX_kDOP_DRAW_TRIS )
		{
			fdraw_Depth_EnableWriting( FALSE );
			fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _vkDOPTriVerts, nTriCount * 3 );
			nTriCount = 0;
			pTriVerts = _vkDOPTriVerts;
		}

		u32 nLastVert = 1;
		u32 nPointCount = 0;
		for ( ii = 2; ii < FConvHull_PlaneBuffer[i].nPointCount; ii++ )
		{
			pTriVerts->Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[0] ].v3;
			pTriVerts->ColorRGBA.Set( kDOPColor );
			pTriVerts++;

			pTriVerts->Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[nLastVert] ].v3;
			pTriVerts->ColorRGBA.Set( kDOPColor );
			pTriVerts++;

			pTriVerts->Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[ii] ].v3;
			pTriVerts->ColorRGBA.Set( kDOPColor );
			pTriVerts++;

			nLastVert = ii;
			nTriCount++;
		}
			
		// This will draw line borders around kDOPs
		nPointCount = 0;
		nLastVert = FConvHull_PlaneBuffer[i].nPointCount - 1;
		for ( ii = 0; ii < FConvHull_PlaneBuffer[i].nPointCount; ii++ )
		{
			pLineVerts->Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[nLastVert] ].v3;
			pLineVerts->ColorRGBA.Set( LineColor );
			pLineVerts++;

			pLineVerts->Pos_MS = FConvHull_vPointBuffer[ FConvHull_PlaneBuffer[i].nPointIdx[ii] ].v3;
			pLineVerts->ColorRGBA.Set( LineColor );
			pLineVerts++;

			nLastVert = ii;
			nLineCount++;
		}

		// Flush the lines
		fdraw_Depth_EnableWriting( TRUE );
		fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, _vkDOPLineVerts, nLineCount * 2 );
		nLineCount = 0;
		pLineVerts = _vkDOPLineVerts;
	}
	
	// Flush anything remaining in the buffers
	if ( nTriCount  )
	{
		fdraw_Depth_EnableWriting( FALSE );
		fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _vkDOPTriVerts, nTriCount * 3 );
	}

	// Pop the model matrix	
	xfmDOP.PopModel();
	
	// Pop the render
	frenderer_Pop();
}
#endif !FANG_PRODUCTION_BUILD


