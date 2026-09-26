//////////////////////////////////////////////////////////////////////////////////////
// fmath_quat.cpp - Fang quaternion library.
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



//--------------------------------------------------------------------
// Old, Obsolete CFQuat Implementation:
//--------------------------------------------------------------------
#if _ENABLE_OLD_QUATS
const CFQuat CFQuat::m_Null;
const CFQuat CFQuat::m_Identity;


CFMtx33 &CFQuat::BuildMtx( CFMtx33 &rDestMtx ) const {
	f32 f1Minus2Vy2, f2Vx2, f2Vz2;
	f32 f2SVx, f2SVy, f2SVz;
	f32 f2VxVy, f2VxVz, f2VyVz;
	f32 fDot;

	fDot = Mag2();

	if( fDot >= _EPSILON ) {
		fDot = 2.0f / fDot;

		f1Minus2Vy2 = 1.0f - fDot*a[1]*a[1];
		f2Vx2 = fDot * a[0] * a[0];
		f2Vz2 = fDot * a[2] * a[2];

		f2SVx = fDot * a[3] * a[0];
		f2SVy = fDot * a[3] * a[1];
		f2SVz = fDot * a[3] * a[2];
		f2VxVy = fDot * a[0] * a[1];
		f2VxVz = fDot * a[0] * a[2];
		f2VyVz = fDot * a[1] * a[2];

		rDestMtx.aa[0][0] = f1Minus2Vy2 - f2Vz2;
		rDestMtx.aa[1][0] = f2VxVy - f2SVz;
		rDestMtx.aa[2][0] = f2VxVz + f2SVy;

		rDestMtx.aa[0][1] = f2VxVy + f2SVz;
		rDestMtx.aa[1][1] = 1.0f - f2Vx2 - f2Vz2;
		rDestMtx.aa[2][1] = f2VyVz - f2SVx;

		rDestMtx.aa[0][2] = f2VxVz - f2SVy;
		rDestMtx.aa[1][2] = f2VyVz + f2SVx;
		rDestMtx.aa[2][2] = f1Minus2Vy2 - f2Vx2;
	} else {
		rDestMtx.Identity();
	}

	return rDestMtx;
}

CFMtx33 &CFQuat::BuildMtx( CFMtx33 &rDestMtx, f32 fScale ) const {
	f32 f1Minus2Vy2, f2Vx2, f2Vz2;
	f32 f2SVx, f2SVy, f2SVz;
	f32 f2VxVy, f2VxVz, f2VyVz;
	f32 fScale2, fDot;

	fDot = Mag2();

	if( fDot >= _EPSILON ) {
		fScale2 = 2.0f * fScale / fDot;

		f1Minus2Vy2 = fScale - fScale2*a[1]*a[1];
		f2Vx2 = fScale2 * a[0] * a[0];
		f2Vz2 = fScale2 * a[2] * a[2];

		f2SVx = fScale2 * a[3] * a[0];
		f2SVy = fScale2 * a[3] * a[1];
		f2SVz = fScale2 * a[3] * a[2];
		f2VxVy = fScale2 * a[0] * a[1];
		f2VxVz = fScale2 * a[0] * a[2];
		f2VyVz = fScale2 * a[1] * a[2];

		rDestMtx.aa[0][0] = f1Minus2Vy2 - f2Vz2;
		rDestMtx.aa[1][0] = f2VxVy - f2SVz;
		rDestMtx.aa[2][0] = f2VxVz + f2SVy;

		rDestMtx.aa[0][1] = f2VxVy + f2SVz;
		rDestMtx.aa[1][1] = fScale - f2Vx2 - f2Vz2;
		rDestMtx.aa[2][1] = f2VyVz - f2SVx;

		rDestMtx.aa[0][2] = f2VxVz - f2SVy;
		rDestMtx.aa[1][2] = f2VyVz + f2SVx;
		rDestMtx.aa[2][2] = f1Minus2Vy2 - f2Vx2;
	} else {
		rDestMtx.aa[0][0] = fScale;
		rDestMtx.aa[1][0] = 0.0f;
		rDestMtx.aa[2][0] = 0.0f;

		rDestMtx.aa[0][1] = 0.0f;
		rDestMtx.aa[1][1] = fScale;
		rDestMtx.aa[2][1] = 0.0f;

		rDestMtx.aa[0][2] = 0.0f;
		rDestMtx.aa[1][2] = 0.0f;
		rDestMtx.aa[2][2] = fScale;
	}

	return rDestMtx;
}

CFVec3 &CFQuat::BuildAxisX( CFVec3 &rDestVec ) const {
	rDestVec.a[0] = 1.0f - 2.0f * (a[1]*a[1] + a[2]*a[2]);
	rDestVec.a[1] = 2.0f * (a[0]*a[1] + a[3]*a[2]);
	rDestVec.a[2] = 2.0f * (a[0]*a[2] - a[3]*a[1]);

	return rDestVec;
}

CFVec3 &CFQuat::BuildAxisY( CFVec3 &rDestVec ) const {
	rDestVec.a[0] = 2.0f * (a[0]*a[1] - a[3]*a[2]);
	rDestVec.a[1] = 1.0f - 2.0f * (a[0]*a[0] + a[2]*a[2]);
	rDestVec.a[2] = 2.0f * (a[1]*a[2] + a[3]*a[0]);

	return rDestVec;
}

CFVec3 &CFQuat::BuildAxisZ( CFVec3 &rDestVec ) const {
	rDestVec.a[0] = 2.0f * (a[0]*a[2] + a[3]*a[1]);
	rDestVec.a[1] = 2.0f * (a[1]*a[2] - a[3]*a[0]);
	rDestVec.a[2] = 1.0f - 2.0f * (a[1]*a[1] + a[0]*a[0]);

	return rDestVec;
}

CFQuat &CFQuat::BuildQuat( const CFMtx33 &rSrcMtx ) {
	f32 fT, fS;
	u32 i;

	fT = rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2];

	if( fT >= 0.0f ) {
		fS = fmath_Sqrt( fT + 1.0f );
		a[3] = 0.5f * fS;
		fS = 0.5f / fS;
		a[0] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
		a[1] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
		a[2] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
	} else {
		i = 0;
		if( rSrcMtx.aa[1][1] > rSrcMtx.aa[0][0] ) {
			i = 1;
		}
		if( rSrcMtx.aa[2][2] > rSrcMtx.aa[i][i] ) {
			i = 2;
		}

		switch( i ) {
		case 0:
			fS = fmath_Sqrt( rSrcMtx.aa[0][0] - (rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2]) + 1.0f );
			a[0] = 0.5f * fS;
			fS = 0.5f / fS;
			a[1] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[3] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
			break;
		case 1:
			fS = fmath_Sqrt( rSrcMtx.aa[1][1] - (rSrcMtx.aa[2][2] + rSrcMtx.aa[0][0]) + 1.0f );
			a[1] = 0.5f * fS;
			fS = 0.5f / fS;
			a[2] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[0] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[3] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
			break;
		case 2:
			fS = fmath_Sqrt( rSrcMtx.aa[2][2] - (rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1]) + 1.0f );
			a[2] = 0.5f * fS;
			fS = 0.5f / fS;
			a[0] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[1] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
			break;
		}
	}

	return *this;
}

CFQuat &CFQuat::ReceiveSlerpOf( f32 fUnitSlerp, const CFQuat &rQuat0, const CFQuat &rQuat1 ) {
	f32 fCosAngleBetween, fSinAngleBetween, fOOSinAngleBetween, fScale0, fScale1, fRadians, fScaledAngle;

	#define __DELTA 0.0001f

	fCosAngleBetween = rQuat0.a[0]*rQuat1.a[0]
					 + rQuat0.a[1]*rQuat1.a[1]
					 + rQuat0.a[2]*rQuat1.a[2]
					 + rQuat0.a[3]*rQuat1.a[3];

	if( (1.0f-fCosAngleBetween) > __DELTA ) {
		FMATH_CLAMPMIN( fCosAngleBetween, -1.0f );
		fSinAngleBetween = fmath_Sqrt( 1.0f - fCosAngleBetween*fCosAngleBetween );
		FASSERT( fSinAngleBetween != 0.f );
		fOOSinAngleBetween = 1.0f / fSinAngleBetween;

		fRadians = fmath_Atan( fSinAngleBetween, FMATH_FABS(fCosAngleBetween) );
		fScaledAngle = fRadians * fUnitSlerp;

		fScale0 = fmath_Sin( fRadians - fScaledAngle ) * fOOSinAngleBetween;
		fScale1 = fmath_Sin( fScaledAngle ) * fOOSinAngleBetween;
	} else {
		fScale0 = 1.0f - fUnitSlerp;
		fScale1 = fUnitSlerp;
	}

	if( fCosAngleBetween < 0.0f ) {
		a[0] = fScale0*rQuat0.a[0] - fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] - fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] - fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] - fScale1*rQuat1.a[3];
	} else {
		a[0] = fScale0*rQuat0.a[0] + fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] + fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] + fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] + fScale1*rQuat1.a[3];
	}

	return *this;

	#undef __DELTA
}

CFQuat &CFQuat::ReceiveSlerpOf( f32 fUnitSlerp0, f32 fUnitSlerp1, const CFQuat &rQuat0, const CFQuat &rQuat1 ) {
	f32 fCosAngleBetween, fSinAngleBetween, fOOSinAngleBetween, fScale0, fScale1, fRadians;

	#define __DELTA 0.0001f

	fCosAngleBetween = rQuat0.a[0]*rQuat1.a[0]
					 + rQuat0.a[1]*rQuat1.a[1]
					 + rQuat0.a[2]*rQuat1.a[2]
					 + rQuat0.a[3]*rQuat1.a[3];

	if( (1.0f-fCosAngleBetween) > __DELTA ) {
		FMATH_CLAMPMIN( fCosAngleBetween, -1.0f );
		fSinAngleBetween = fmath_Sqrt( 1.0f - fCosAngleBetween*fCosAngleBetween );
		fOOSinAngleBetween = 1.0f / fSinAngleBetween;

		fRadians = fmath_Atan( fSinAngleBetween, FMATH_FABS(fCosAngleBetween) );

		fScale0 = fmath_Sin( fRadians * (1.0f - fUnitSlerp0) ) * fOOSinAngleBetween;
		fScale1 = fmath_Sin( fRadians * fUnitSlerp1 ) * fOOSinAngleBetween;
	} else {
		fScale0 = fUnitSlerp0;
		fScale1 = fUnitSlerp1;
	}

	if( fCosAngleBetween < 0.0f ) {
		a[0] = fScale0*rQuat0.a[0] - fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] - fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] - fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] - fScale1*rQuat1.a[3];
	} else {
		a[0] = fScale0*rQuat0.a[0] + fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] + fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] + fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] + fScale1*rQuat1.a[3];
	}

	return *this;

	#undef __DELTA
}
#endif


//--------------------------------------------------------------------
// CFQuatA Implementation:
//--------------------------------------------------------------------

// Matrix must have unit scale.
CFQuatA &CFQuatA::BuildQuat( const CFMtx33 &rSrcMtx ) {
	f32 fT, fS;
	u32 i;

	fT = rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2];

	if( fT <= 0.0f ) {
		i = 0;
		if( rSrcMtx.aa[1][1] > rSrcMtx.aa[0][0] ) {
			i = 1;
		}
		if( rSrcMtx.aa[2][2] > rSrcMtx.aa[i][i] ) {
			i = 2;
		}

		switch( i ) {
		case 0:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[0][0] - (rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2]) + 1.0f );
			a[0] = fmath_Div( 0.25f, fS );
			a[1] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[3] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
			break;
		case 1:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[1][1] - (rSrcMtx.aa[2][2] + rSrcMtx.aa[0][0]) + 1.0f );
			a[0] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[1] = fmath_Div( 0.25f, fS );
			a[2] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
			break;
		case 2:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[2][2] - (rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1]) + 1.0f );
			a[0] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[1] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[2] = fmath_Div( 0.25f, fS );
			a[3] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
			break;
		}
	} else {
		fS = 0.5f * fmath_InvSqrt( fT + 1.0f );
		a[0] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
		a[1] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
		a[2] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
		a[3] = fmath_Div( 0.25f, fS );
	}

	return *this;
}


// Matrix must have unit scale.
CFQuatA &CFQuatA::BuildQuat( const CFMtx44 &rSrcMtx ) {
	f32 fT, fS;
	u32 i;

	fT = rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2];

	if( fT <= 0.0f ) {
		i = 0;
		if( rSrcMtx.aa[1][1] > rSrcMtx.aa[0][0] ) {
			i = 1;
		}
		if( rSrcMtx.aa[2][2] > rSrcMtx.aa[i][i] ) {
			i = 2;
		}

		switch( i ) {
		case 0:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[0][0] - (rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2]) + 1.0f );
			a[0] = fmath_Div( 0.25f, fS );
			a[1] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[3] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
			break;
		case 1:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[1][1] - (rSrcMtx.aa[2][2] + rSrcMtx.aa[0][0]) + 1.0f );
			a[0] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[1] = fmath_Div( 0.25f, fS );
			a[2] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
			break;
		case 2:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[2][2] - (rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1]) + 1.0f );
			a[0] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[1] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[2] = fmath_Div( 0.25f, fS );
			a[3] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
			break;
		}
	} else {
		fS = 0.5f * fmath_InvSqrt( fT + 1.0f );
		a[0] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
		a[1] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
		a[2] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
		a[3] = fmath_Div( 0.25f, fS );
	}

	return *this;
}

// Matrix must have unit scale.
CFQuatA &CFQuatA::BuildQuat( const CFMtx43A &rSrcMtx ) {
	f32 fT, fS;
	u32 i;

	fT = rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2];

	if( fT <= 0.0f ) {
		i = 0;
		if( rSrcMtx.aa[1][1] > rSrcMtx.aa[0][0] ) {
			i = 1;
		}
		if( rSrcMtx.aa[2][2] > rSrcMtx.aa[i][i] ) {
			i = 2;
		}

		switch( i ) {
		case 0:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[0][0] - (rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2]) + 1.0f );
			a[0] = fmath_Div( 0.25f, fS );
			a[1] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[3] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
			break;
		case 1:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[1][1] - (rSrcMtx.aa[2][2] + rSrcMtx.aa[0][0]) + 1.0f );
			a[0] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[1] = fmath_Div( 0.25f, fS );
			a[2] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
			break;
		case 2:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[2][2] - (rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1]) + 1.0f );
			a[0] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[1] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[2] = fmath_Div( 0.25f, fS );
			a[3] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
			break;
		}
	} else {
		fS = 0.5f * fmath_InvSqrt( fT + 1.0f );
		a[0] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
		a[1] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
		a[2] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
		a[3] = fmath_Div( 0.25f, fS );
	}

	return *this;
}

// More accurate version of this incarnation of BuildQuat for clean rotations
// Matrix must have unit scale.
CFQuatA &CFQuatA::AcuBuildQuat( const CFMtx43A &rSrcMtx ) {
	f32 fT, fS;
	u32 i;

	fT = rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2];

	if( fT <= 0.0f ) {
		i = 0;
		if( rSrcMtx.aa[1][1] > rSrcMtx.aa[0][0] ) {
			i = 1;
		}
		if( rSrcMtx.aa[2][2] > rSrcMtx.aa[i][i] ) {
			i = 2;
		}
		// fmath_AcuInvSqrt()

		switch( i ) {
		case 0:
			fS = 0.5f * fmath_AcuInvSqrt( rSrcMtx.aa[0][0] - (rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2]) + 1.0f );
			a[0] = 0.25f / fS;
			a[1] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[3] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
			break;
		case 1:
			fS = 0.5f * fmath_AcuInvSqrt( rSrcMtx.aa[1][1] - (rSrcMtx.aa[2][2] + rSrcMtx.aa[0][0]) + 1.0f );
			a[0] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[1] = 0.25f / fS;
			a[2] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
			break;
		case 2:
			fS = 0.5f * fmath_AcuInvSqrt( rSrcMtx.aa[2][2] - (rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1]) + 1.0f );
			a[0] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[1] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[2] = 0.25f / fS;
			a[3] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
			break;
		}
	} else {
		fS = 0.5f * fmath_AcuInvSqrt( fT + 1.0f );
		a[0] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
		a[1] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
		a[2] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
		a[3] = 0.25f / fS;
	}

	return *this;
}

CFQuatA &CFQuatA::BuildQuat( const CFMtx43A &rSrcMtx, f32 fOOMtxScale ) {
	f32 fT, fS;
	u32 i;

	FASSERT( fOOMtxScale > 0.0f );

	fT = rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2];

	if( fT <= 0.0f ) {
		i = 0;
		if( rSrcMtx.aa[1][1] > rSrcMtx.aa[0][0] ) {
			i = 1;
		}
		if( rSrcMtx.aa[2][2] > rSrcMtx.aa[i][i] ) {
			i = 2;
		}

		switch( i ) {
		case 0:
			fS = 0.5f * fmath_InvSqrt( fOOMtxScale * (rSrcMtx.aa[0][0] - (rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2])) + 1.0f );
			a[0] = fmath_Div( 0.25f, fS );
			fS *= fOOMtxScale;
			a[1] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[3] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
			break;
		case 1:
			fS = 0.5f * fmath_InvSqrt( fOOMtxScale * (rSrcMtx.aa[1][1] - (rSrcMtx.aa[2][2] + rSrcMtx.aa[0][0])) + 1.0f );
			a[1] = fmath_Div( 0.25f, fS );
			fS *= fOOMtxScale;
			a[0] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
			break;
		case 2:
			fS = 0.5f * fmath_InvSqrt( fOOMtxScale * (rSrcMtx.aa[2][2] - (rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1])) + 1.0f );
			a[2] = fmath_Div( 0.25f, fS );
			fS *= fOOMtxScale;
			a[0] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[1] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
			break;
		}
	} else {
		fS = 0.5f * fmath_InvSqrt( fOOMtxScale*fT + 1.0f );
		a[3] = fmath_Div( 0.25f, fS );
		fS *= fOOMtxScale;
		a[0] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
		a[1] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
		a[2] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
	}

	return *this;
}

// Matrix must have unit scale.
CFQuatA &CFQuatA::BuildQuat( const CFMtx44A &rSrcMtx ) {
	f32 fT, fS;
	u32 i;

	fT = rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2];

	if( fT <= 0.0f ) {
		i = 0;
		if( rSrcMtx.aa[1][1] > rSrcMtx.aa[0][0] ) {
			i = 1;
		}
		if( rSrcMtx.aa[2][2] > rSrcMtx.aa[i][i] ) {
			i = 2;
		}

		switch( i ) {
		case 0:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[0][0] - (rSrcMtx.aa[1][1] + rSrcMtx.aa[2][2]) + 1.0f );
			a[0] = fmath_Div( 0.25f, fS );
			a[1] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[2] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[3] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
			break;
		case 1:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[1][1] - (rSrcMtx.aa[2][2] + rSrcMtx.aa[0][0]) + 1.0f );
			a[0] = (rSrcMtx.aa[1][0] + rSrcMtx.aa[0][1]) * fS;
			a[1] = fmath_Div( 0.25f, fS );
			a[2] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[3] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
			break;
		case 2:
			fS = 0.5f * fmath_InvSqrt( rSrcMtx.aa[2][2] - (rSrcMtx.aa[0][0] + rSrcMtx.aa[1][1]) + 1.0f );
			a[0] = (rSrcMtx.aa[0][2] + rSrcMtx.aa[2][0]) * fS;
			a[1] = (rSrcMtx.aa[2][1] + rSrcMtx.aa[1][2]) * fS;
			a[2] = fmath_Div( 0.25f, fS );
			a[3] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
			break;
		}
	} else {
		fS = 0.5f * fmath_InvSqrt( fT + 1.0f );
		a[0] = (rSrcMtx.aa[1][2] - rSrcMtx.aa[2][1]) * fS;
		a[1] = (rSrcMtx.aa[2][0] - rSrcMtx.aa[0][2]) * fS;
		a[2] = (rSrcMtx.aa[0][1] - rSrcMtx.aa[1][0]) * fS;
		a[3] = fmath_Div( 0.25f, fS );
	}

	return *this;
}

CFQuatA &CFQuatA::ReceiveSlerpOf( const f32 &fUnitSlerp, const CFQuatA &rQuat0, const CFQuatA &rQuat1 ) {
	f32 fTemp, fCosAngleBetween, fSinAngleBetween, fOOSinAngleBetween, fScale0, fScale1, fRadians, fScaledAngle;

	#define __DELTA 0.0001f

	fCosAngleBetween = rQuat0.a[0]*rQuat1.a[0]
					 + rQuat0.a[1]*rQuat1.a[1]
					 + rQuat0.a[2]*rQuat1.a[2]
					 + rQuat0.a[3]*rQuat1.a[3];

	FMATH_CLAMP( fCosAngleBetween, -1.0f, 1.0f );
	fTemp = 1.0f - fCosAngleBetween*fCosAngleBetween;

	if( fTemp > __DELTA ) {
		fOOSinAngleBetween = fmath_InvSqrt( fTemp );
		fSinAngleBetween = fmath_Inv( fOOSinAngleBetween );

		fRadians = fmath_Atan( fSinAngleBetween, FMATH_FABS(fCosAngleBetween) );
		fScaledAngle = fRadians * fUnitSlerp;

		fScale0 = fmath_Sin( fRadians - fScaledAngle ) * fOOSinAngleBetween;
		fScale1 = fmath_Sin( fScaledAngle ) * fOOSinAngleBetween;
	} else {
		fScale0 = 1.0f - fUnitSlerp;
		fScale1 = fUnitSlerp;
	}

	if( fCosAngleBetween < 0.0f ) {
		a[0] = fScale0*rQuat0.a[0] - fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] - fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] - fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] - fScale1*rQuat1.a[3];
	} else {
		a[0] = fScale0*rQuat0.a[0] + fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] + fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] + fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] + fScale1*rQuat1.a[3];
	}

	return *this;

	#undef __DELTA
}

CFQuatA &CFQuatA::ReceiveSlerpOf( const f32 &fUnitSlerp0, const f32 &fUnitSlerp1, const CFQuatA &rQuat0, const CFQuatA &rQuat1 ) {
	f32 fTemp, fCosAngleBetween, fSinAngleBetween, fOOSinAngleBetween, fScale0, fScale1, fRadians;

	#define __DELTA 0.0001f

	fCosAngleBetween = rQuat0.a[0]*rQuat1.a[0]
					 + rQuat0.a[1]*rQuat1.a[1]
					 + rQuat0.a[2]*rQuat1.a[2]
					 + rQuat0.a[3]*rQuat1.a[3];

	FMATH_CLAMP( fCosAngleBetween, -1.0f, 1.0f );
	fTemp = 1.0f - fCosAngleBetween*fCosAngleBetween;

	if( fTemp > __DELTA ) {
		fOOSinAngleBetween = fmath_InvSqrt( fTemp );
		fSinAngleBetween = fmath_Inv( fOOSinAngleBetween );

		fRadians = fmath_Atan( fSinAngleBetween, FMATH_FABS(fCosAngleBetween) );

		fScale0 = fmath_Sin( fRadians * (1.0f - fUnitSlerp0) ) * fOOSinAngleBetween;
		fScale1 = fmath_Sin( fRadians * fUnitSlerp1 ) * fOOSinAngleBetween;
	} else {
		fScale0 = fUnitSlerp0;
		fScale1 = fUnitSlerp1;
	}

	if( fCosAngleBetween < 0.0f ) {
		a[0] = fScale0*rQuat0.a[0] - fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] - fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] - fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] - fScale1*rQuat1.a[3];
	} else {
		a[0] = fScale0*rQuat0.a[0] + fScale1*rQuat1.a[0];
		a[1] = fScale0*rQuat0.a[1] + fScale1*rQuat1.a[1];
		a[2] = fScale0*rQuat0.a[2] + fScale1*rQuat1.a[2];
		a[3] = fScale0*rQuat0.a[3] + fScale1*rQuat1.a[3];
	}

	return *this;

	#undef __DELTA
}


// Takes a step equal to fRadiansToStep from rQuat0 to rQuat1. Clamps the result to rQuat1.
CFQuatA &CFQuatA::ReceiveSlerpedStep( f32 fRadiansToStep, const CFQuatA &rQuat0, const CFQuatA &rQuat1 ) {
	f32 fTemp, fCosAngleBetween, fSinAngleBetween, fOOSinAngleBetween, fScale0, fScale1, fMaxRadians;

	#define __DELTA 0.0001f

	fCosAngleBetween = rQuat0.a[0]*rQuat1.a[0]
					 + rQuat0.a[1]*rQuat1.a[1]
					 + rQuat0.a[2]*rQuat1.a[2]
					 + rQuat0.a[3]*rQuat1.a[3];

	FMATH_CLAMP( fCosAngleBetween, -1.0f, 1.0f );
	fTemp = 1.0f - fCosAngleBetween*fCosAngleBetween;

	if( fTemp > __DELTA ) {
		fOOSinAngleBetween = fmath_InvSqrt( fTemp );
		fSinAngleBetween = fmath_Inv( fOOSinAngleBetween );

		fMaxRadians = fmath_Atan( fSinAngleBetween, FMATH_FABS(fCosAngleBetween) );
		fRadiansToStep *= 0.5f;

		if( fRadiansToStep < fMaxRadians ) {
			// Step size is within limit...

			fScale0 = fmath_Sin( fMaxRadians - fRadiansToStep ) * fOOSinAngleBetween;
			fScale1 = fmath_Sin( fRadiansToStep ) * fOOSinAngleBetween;

			if( fCosAngleBetween < 0.0f ) {
				a[0] = fScale0*rQuat0.a[0] - fScale1*rQuat1.a[0];
				a[1] = fScale0*rQuat0.a[1] - fScale1*rQuat1.a[1];
				a[2] = fScale0*rQuat0.a[2] - fScale1*rQuat1.a[2];
				a[3] = fScale0*rQuat0.a[3] - fScale1*rQuat1.a[3];
			} else {
				a[0] = fScale0*rQuat0.a[0] + fScale1*rQuat1.a[0];
				a[1] = fScale0*rQuat0.a[1] + fScale1*rQuat1.a[1];
				a[2] = fScale0*rQuat0.a[2] + fScale1*rQuat1.a[2];
				a[3] = fScale0*rQuat0.a[3] + fScale1*rQuat1.a[3];
			}

			return *this;
		}
	}

	// Step size is outside limit. Clamp to rQuat1...

	*this = rQuat1;

	return *this;

	#undef __DELTA
}


FMathFcheckResult_e CFQuatA::FCheck( void ) const {
	return v.FCheck();
}



//--------------------------------------------------------------------
// CFTQuatA Implementation:
//--------------------------------------------------------------------

FMathFcheckResult_e CFTQuatA::FCheck( void ) const {
	FMathFcheckResult_e nRetValue;

	if( (nRetValue=v.FCheck()) != FMATH_FCHECK_RESULT_OK ) {
		return nRetValue;
	}

	return m_PosScale.FCheck();
}


