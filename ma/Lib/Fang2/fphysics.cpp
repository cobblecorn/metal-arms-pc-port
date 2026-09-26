//////////////////////////////////////////////////////////////////////////////////////
// fphysics.cpp -
//
// Author: Chris MacDonald
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
// 11/08/02 MacDonald     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdraw.h"
#include "frenderer.h"
#include "floop.h"
#include "fphysics.h"
#include "ftext.h"

#define _USE_MIDPOINT			( 0 )
//#define _CHECK_FLOATS

#ifdef _CHECK_FLOATS

static s32 _nLine = 0;
static s32 _nIndex = 0;
#define _CHECK_FLOAT( a, b )			{\
	if( !_nLine && fmath_Fcheck( (a) ) != FMATH_FCHECK_RESULT_OK )\
	{\
		_nLine = __LINE__; \
		_nIndex = b;\
	}}

#define _PRINT_FLOAT_CHECK_RESULT()	{ if( _nLine != 0 ) ftext_Printf( 0.1f, 0.09f, "~f1~C92929299~w1~al~s0.69Bad Float, Line %d, index %d.", _nLine, _nIndex ); }

#else	// _CHECK_FLOATS

#define _CHECK_FLOAT( a, b )
#define _PRINT_FLOAT_CHECK_RESULT()

#endif

#if _USE_MIDPOINT
#define _CONTACT_PENALTY_FORCE_COEFFICIENT	( 4.0f )
#else
#define _CONTACT_PENALTY_FORCE_COEFFICIENT	( 4.0f )
#endif

#define _DEFAULT_MASS			( 1.0 )		// mass of object when initialized
#define _DEFAULT_ELASTICITY		( 0.5 )		// elasticity of initialized object, 0.0 to 1.0
#define _DEFAULT_GRAVITY		( -32.0f )	// gravitational acceleration constant

#define _LINEAR_DAMPING			( 0.1f )	// fraction of current velocity
#define _ANGULAR_DAMPING		( 50.0f )	// fraction of current angular velocity

#define _CONTACT_VEL			( 10.0f )

// debug drawing stuff
#if !FANG_PRODUCTION_BUILD
#define _DRAW_DEBUG
#define _MAX_DEBUG_LINES		( 32 )
#define _MAX_DEBUG_SPHERES		( 32 )

typedef struct
{
	FDrawVtx_t vStart;
	FDrawVtx_t vEnd;
} _Line_t;

_Line_t _DebugLines[ _MAX_DEBUG_LINES ];
s32 _nDebugLines = 0;

typedef struct
{
	CFVec3 vPos;
	f32 fRadius;
	CFColorRGBA vColor;
} _Sphere_t;

_Sphere_t _DebugSpheres[ _MAX_DEBUG_SPHERES ];
s32 _nDebugSpheres = 0;
#endif //!FANG_PRODUCTION_BUILD

CFPhysicsObject *CFPhysicsObject::m_pPhysObj;	// pointer to current physics object for static callback functions

static s32 _nCollision = 0;
static BOOL _bPrint = FALSE;

extern f32 _fAdjust;
extern CFVec3A _vAdjust;

// debugging use only
CFPhysicsObject *_pDrivenPhysObj;

//-----------------------------------------------------------------------------
// private functions
//-----------------------------------------------------------------------------
// constructor
CFPhysicsObject::CFPhysicsObject()
{
}

//-----------------------------------------------------------------------------
// destructor
CFPhysicsObject::~CFPhysicsObject()
{
}

//-----------------------------------------------------------------------------
// outputs a smoothly decreasing version of an input value.
// If fInputVal is greater than fMaxRange, output = input.
// If fInputVal is less than fMinRange, output = 0.0f.
// if fInputVal is between fMinRange and fMaxRange, output value is proportional
// to fInputVal's fractional range between fMinRange and fMaxRange.
f32 CFPhysicsObject::_SmoothBipolarClampMin( f32 fInputVal, f32 fMinRange, f32 fMaxRange )
{
	BOOL bNegative = FALSE;

	if( fInputVal < 0.0f )
	{
		fInputVal = -fInputVal;
		bNegative = TRUE;
	}

	if( fInputVal > fMaxRange )
	{
		if( bNegative )
		{
			return -fInputVal;
		}
		else
		{
			return fInputVal;
		}

	}

	if( fInputVal < fMinRange )
	{
		return 0.0f;
	}

	FASSERT( fMaxRange > fMinRange );

	f32 fRange = fMaxRange - fMinRange;
	f32 fFraction = fmath_Div( (fInputVal - fMinRange), fRange );

	if( bNegative )
	{
		return -fInputVal * fFraction;
	}
	else
	{
		return fInputVal * fFraction;
	}
}

//-----------------------------------------------------------------------------
// returns scalar multiplier to produce vector smoothly clamped over a range to a min value.
f32 CFPhysicsObject::_SmoothVectorClampMinCoeff( CFVec3A &rvInput, f32 fMinRange, f32 fMaxRange )
{
	FASSERT( fMaxRange > fMinRange );

	f32 fMag = rvInput.Mag();

	if( fMag > fMaxRange )
	{
		// vector is unchanged
		return 1.0f;
	}

	if( fMag < fMinRange )
	{
		// vector magnitude is below minimum clamp range
		return 0.0f;
	}

	f32 fRange = fMaxRange - fMinRange;
	return fmath_Div( (fMag - fMinRange), fRange );
}

//-----------------------------------------------------------------------------
// smoothly reduce vector magnitude over a range.
void CFPhysicsObject::_SmoothVectorClampMin( CFVec3A &rvInput, f32 fMinRange, f32 fMaxRange )
{
	f32 fCoeff = _SmoothVectorClampMinCoeff( rvInput, fMinRange, fMaxRange );
	rvInput.Mul( fCoeff );
}

//-----------------------------------------------------------------------------
// set member variables to default values
void CFPhysicsObject::_SetDefaults( void )
{
	m_fMass = _DEFAULT_MASS;
	m_fInverseMass = 1.0 / _DEFAULT_MASS;
	m_vCMOffset.Zero();

	m_bVisible = TRUE;
	m_fElasticity = _DEFAULT_ELASTICITY;
	m_fOtherObjElasticity = _DEFAULT_ELASTICITY;
	m_State.vPosition.Zero();
	m_State.vMomentum.Zero();
	m_State.vVelocity.Zero();
	m_State.vAcceleration.Zero();

	m_State.mOrientation.Identity();
	m_State.vAngularMomentum.Zero();
	m_State.vAngularVelocity.Zero();
	m_State.vAngularAcceleration.Zero();

	m_State.vTotalForce.Zero();
	m_State.vTotalTorque.Zero();
	m_State.vTotalFriction.Zero();
	m_State.vTotalFrictionTorque.Zero();

	m_fGravity = _DEFAULT_GRAVITY;

	m_pCollisionCB = NULL;
	m_nColPoints = 0;
	m_paColPoints = NULL;
	m_vInertialSize.Set( 1.0f, 1.0f, 1.0f );
	_ComputeLocalInertiaTensor();
	_ComputeWorldInertiaTensor( m_State );

	m_fLinearDamping = _LINEAR_DAMPING;
	m_fAngularDamping = _ANGULAR_DAMPING;

	m_uFlags = 0;

	m_fMinMomClamping = 0.0f;
	m_fMinMomClamping = 0.0f;
	m_fMinAngMomClamping = 0.0f;
	m_fMinAngMomClamping = 0.0f;
	m_fClampCoeff = 0.0f;

	m_fDynDampVel = 0.0f;
	m_fDynDampAng = 0.0f;
	m_fDynDampForce = 0.0f;
	m_fDynDampOOVel = 0.0f;
	m_fDynDampOOAng = 0.0f;
	m_fDynDampOOForce = 0.0f;
	m_fMinLinDamping = 0.0f;
	m_fMaxLinDamping= 0.0f;
	m_fMinAngDamping = 0.0f;
	m_fMaxAngDamping = 0.0f;

}

//-----------------------------------------------------------------------------
// computes the principal moment of inertia 
// of a box for one axis.
f32 CFPhysicsObject::_ComputeBoxPMIAxis( const f32 fMass, const s32 nAxis, const CFVec3A *pvHalfSize )
{
	CFVec3A vSize;
	vSize.Set( *pvHalfSize );
	vSize.Mul( 2.0f );

	vSize.a[ nAxis ] = 0.0f;

	return (f32) ( fMass / 12.0f ) * ( vSize.a[0] * vSize.a[0] + vSize.a[1] * vSize.a[1] + vSize.a[2] * vSize.a[2] );
}

//-----------------------------------------------------------------------------
// computes a default object-space inverse inertia tensor.
void CFPhysicsObject::_ComputeLocalInertiaTensor( void )
{
	m_mLocalIT.Identity();
	m_mLocalInverseIT.Identity();

	m_mLocalIT.aa[0][0] = _ComputeBoxPMIAxis( m_fMass, 0, &m_vInertialSize );
	m_mLocalIT.aa[1][1] = _ComputeBoxPMIAxis( m_fMass, 1, &m_vInertialSize );
	m_mLocalIT.aa[2][2] = _ComputeBoxPMIAxis( m_fMass, 2, &m_vInertialSize );

	m_mLocalInverseIT.aa[0][0] = 1.0f / m_mLocalIT.aa[0][0];
	m_mLocalInverseIT.aa[1][1] = 1.0f / m_mLocalIT.aa[1][1];
	m_mLocalInverseIT.aa[2][2] = 1.0f / m_mLocalIT.aa[2][2];
}

//-----------------------------------------------------------------------------
// apply gravity force to object
void CFPhysicsObject::_ApplyGravity()
{
	m_State.vTotalForce.y += m_fGravity * m_fMass;
}

//-----------------------------------------------------------------------------
// computes the object's world space inverse inertia tensor
void CFPhysicsObject::_ComputeWorldInertiaTensor( PhysicsState_t &rState )
{
	CFMtx43A mOrT;

	mOrT.Identity();
	mOrT.ReceiveTranspose33( rState.mOrientation );

	// inverseIT = ( orientation * localInverseIT ) * Transpose( orientation );
	rState.mInverseIT.Mul( rState.mOrientation, m_mLocalInverseIT );
	rState.mInverseIT.Mul( mOrT );
}

//-----------------------------------------------------------------------------
#define _STATE_ARRAY_SIZE	( 18 )
// encode physics state as linear array
void CFPhysicsObject::_StateToArray( const PhysicsState_t &rState, f32 *pfArray )
{
	*pfArray++ = rState.vPosition.x;
	*pfArray++ = rState.vPosition.y;
	*pfArray++ = rState.vPosition.z;

	*pfArray++ = rState.mOrientation.m_vX.x;
	*pfArray++ = rState.mOrientation.m_vX.y;
	*pfArray++ = rState.mOrientation.m_vX.z;

	*pfArray++ = rState.mOrientation.m_vY.x;
	*pfArray++ = rState.mOrientation.m_vY.y;
	*pfArray++ = rState.mOrientation.m_vY.z;

	*pfArray++ = rState.mOrientation.m_vZ.x;
	*pfArray++ = rState.mOrientation.m_vZ.y;
	*pfArray++ = rState.mOrientation.m_vZ.z;

	*pfArray++ = rState.vMomentum.x;
	*pfArray++ = rState.vMomentum.y;
	*pfArray++ = rState.vMomentum.z;

	*pfArray++ = rState.vAngularMomentum.x;
	*pfArray++ = rState.vAngularMomentum.y;
	*pfArray++ = rState.vAngularMomentum.z;
}

//-----------------------------------------------------------------------------
// decode linear array into physics state
void CFPhysicsObject::_ArrayToState( PhysicsState_t &rState, f32 *pfArray )
{
	CFMtx43A mOrT;

	rState.vPosition.x = *pfArray++;
	rState.vPosition.y = *pfArray++;
	rState.vPosition.z = *pfArray++;

	rState.mOrientation.Identity();
	rState.mOrientation.m_vX.x = *pfArray++;
	rState.mOrientation.m_vX.y = *pfArray++;
	rState.mOrientation.m_vX.z = *pfArray++;

	rState.mOrientation.m_vY.x = *pfArray++;
	rState.mOrientation.m_vY.y = *pfArray++;
	rState.mOrientation.m_vY.z = *pfArray++;

	rState.mOrientation.m_vZ.x = *pfArray++;
	rState.mOrientation.m_vZ.y = *pfArray++;
	rState.mOrientation.m_vZ.z = *pfArray++;

	rState.vMomentum.x = *pfArray++;
	rState.vMomentum.y = *pfArray++;
	rState.vMomentum.z = *pfArray++;

	rState.vAngularMomentum.x = *pfArray++;
	rState.vAngularMomentum.y = *pfArray++;
	rState.vAngularMomentum.z = *pfArray++;

	//orthonormalize rotation matrix
	rState.mOrientation.m_vX.Unitize();
	rState.mOrientation.m_vZ.Cross( rState.mOrientation.m_vX, rState.mOrientation.m_vY );
	rState.mOrientation.m_vZ.Unitize();
	rState.mOrientation.m_vY.Cross( rState.mOrientation.m_vZ, rState.mOrientation.m_vX );
	rState.mOrientation.m_vY.Unitize();	// necessary?

	// update all auxiliary quantities

	// force & torque
	rState.vTotalForce.Zero();
	rState.vTotalTorque.Zero();
   	rState.vTotalFriction.Zero();
	rState.vTotalFrictionTorque.Zero();

	// update world-space inverse inertia tensor
	mOrT.Identity();
	mOrT.ReceiveTranspose33( rState.mOrientation );
	rState.mInverseIT.Mul( rState.mOrientation, m_pPhysObj->m_mLocalInverseIT );
	rState.mInverseIT.Mul( mOrT );

	// linear velocity
	rState.vVelocity.Mul( rState.vMomentum, m_pPhysObj->m_fInverseMass );

	// linear acceleration
	rState.vAcceleration.Mul( rState.vTotalForce, m_pPhysObj->m_fInverseMass );

	// angular velocity
	rState.mInverseIT.MulPoint( rState.vAngularVelocity, rState.vAngularMomentum );

	// angular acceleration 
	// (computation from "Rigid Body Simulation II" by Baraff, p. D60)
	rState.vAngularAcceleration.Cross( rState.vAngularMomentum, rState.vAngularVelocity );
	rState.vAngularAcceleration.Add( rState.vTotalTorque );
	rState.vAngularAcceleration.Add( rState.vTotalFrictionTorque );
	rState.mInverseIT.MulPoint( rState.vAngularAcceleration );
}

//-----------------------------------------------------------------------------
static s32 _nPoint = 0;
void CFPhysicsObject::_ComputeForceAndTorque( f32 fTime, PhysicsState_t &rState )
{
	CFVec3A vForce, vFriction, vTemp;
	CollisionPoint_t *pPoint;
	s32 nIndex;

	rState.vTotalForce = m_pPhysObj->m_State.vTotalForce;
	rState.vTotalTorque = m_pPhysObj->m_State.vTotalTorque;

	rState.vTotalForce.y += m_pPhysObj->m_fGravity * m_pPhysObj->m_fMass;

	if( m_pPhysObj->m_paColPoints == NULL )
	{
		return;
	}

	for( nIndex = 0; nIndex < m_pPhysObj->m_nColPoints; nIndex++ )
	{
		_nPoint = nIndex;
		pPoint = &m_pPhysObj->m_paColPoints[nIndex];

		if( pPoint->uClientFlags & CLIENT_FLAG_COLLIDED )
		{
			_ClientForce( *pPoint, &vForce );
			rState.vTotalForce.Add( vForce );
			vTemp.Cross( pPoint->vPoint, vForce );
			rState.vTotalTorque.Add( vTemp );
		}
	}

	for( nIndex = 0; nIndex < m_pPhysObj->m_nColPoints; nIndex++ )
	{
		_nPoint = nIndex;
		pPoint = &m_pPhysObj->m_paColPoints[nIndex];

		if( pPoint->uClientFlags & CLIENT_FLAG_COLLIDED || pPoint->fTimeSinceCollision < pPoint->fFrictionSmoothTime )
		{
			_Friction( *pPoint, rState, &vFriction );
			rState.vTotalForce.Add( vFriction );

			if( m_pPhysObj->m_uFlags & FPHYSICS_FLAG_IGNORE_FRICT_POS )
			{
				vTemp.Cross( pPoint->vPoint, vFriction );
			}
			else
			{
				// allow client to adjust point where friction forces are applied relative to the collision point
				vTemp.Set( pPoint->pmFrictOr->m_vPos );
				vTemp.Add( pPoint->vPoint );
				vTemp.Cross( vFriction );
			}

			rState.vTotalTorque.Add( vTemp );
		}
	}
}

//-----------------------------------------------------------------------------
// given a state, fill array with derivatives of _StateToArray array values
void CFPhysicsObject::_StateToArrayDerivative( PhysicsState_t &rState, f32 *pafDerivative )
{
	CFMtx43A mSkew;

	// position derivative is velocity
	*pafDerivative++ = rState.vVelocity.x;
	*pafDerivative++ = rState.vVelocity.y;
	*pafDerivative++ = rState.vVelocity.z;

	// rotation derivative is skew-symmetric matrix of angular velocity ("omega star") times rotation matrix
	mSkew.aa[0][0] = 0.0f;                       mSkew.aa[0][1] = rState.vAngularVelocity.z;  mSkew.aa[0][2] = -rState.vAngularVelocity.y;
	mSkew.aa[1][0] = -rState.vAngularVelocity.z; mSkew.aa[1][1] = 0.0f;                       mSkew.aa[1][2] = rState.vAngularVelocity.x;
	mSkew.aa[2][0] = rState.vAngularVelocity.y;  mSkew.aa[2][1] = -rState.vAngularVelocity.x; mSkew.aa[2][2] = 0.0f;
	mSkew.aa[3][0] = 0.0f;                       mSkew.aa[3][1] = 0.0f;                       mSkew.aa[3][2] = 0.0f;
	mSkew.Mul( rState.mOrientation );

	*pafDerivative++ = mSkew.m_vX.x;
	*pafDerivative++ = mSkew.m_vX.y;
	*pafDerivative++ = mSkew.m_vX.z;

	*pafDerivative++ = mSkew.m_vY.x;
	*pafDerivative++ = mSkew.m_vY.y;
	*pafDerivative++ = mSkew.m_vY.z;

	*pafDerivative++ = mSkew.m_vZ.x;
	*pafDerivative++ = mSkew.m_vZ.y;
	*pafDerivative++ = mSkew.m_vZ.z;

	// linear momentum derivative is force
	*pafDerivative++ = rState.vTotalForce.x;
	*pafDerivative++ = rState.vTotalForce.y;
	*pafDerivative++ = rState.vTotalForce.z;

	// angular momentum derivative is torque
	*pafDerivative++ = rState.vTotalTorque.x;
	*pafDerivative++ = rState.vTotalTorque.y;
	*pafDerivative++ = rState.vTotalTorque.z;
}

//-----------------------------------------------------------------------------
// construct a state at time t and compute state derivatives
void CFPhysicsObject::_StateArrayDerivative( f32 fTime, f32 *pafInput, f32 *pafDerivative )
{
	PhysicsState_t State;

	_ArrayToState( State, pafInput );

	_ComputeForceAndTorque( fTime, State );

	_StateToArrayDerivative( State, pafDerivative );
}


//-----------------------------------------------------------------------------
// apply client-requested force at point.
// reduce force application if in kinetic friction state.
void CFPhysicsObject::_ClientForce( CollisionPoint_t &rPoint, CFVec3A *pvForce )
{
	if( !(rPoint.uClientFlags & CLIENT_FLAG_COLLIDED) )
	{
		pvForce->Zero();
		return;
	}

	pvForce->Set( rPoint.vAppliedForce );
	if( (rPoint.uLastResultFlags & COLLISION_RESULT_KINETIC || 
		rPoint.uClientFlags & CLIENT_FLAG_KINETIC_FRICTION) &&
		!(rPoint.uClientFlags & CLIENT_FLAG_STATIC_FRICTION) )
	{
		pvForce->Mul( rPoint.fAppliedForceKineticMul );
	}

//	if( _pDrivenPhysObj == m_pPhysObj )
//	{
//		CFVec3A vStart, vEnd;
//		vStart.Set( rPoint.vPoint );
//		vStart.Add( m_pPhysObj->m_State.vPosition );
//		vStart.y += 10.0f;
//
//		vEnd.Set( *pvForce );
//		vEnd.Mul( 0.0001f );
//		vEnd.Add( vStart );
//		DrawLine( &vStart, &vEnd );
//	}
}

//-----------------------------------------------------------------------------
// apply friction at contact points.
// allow client to specify unique friction parameters for each contact.
// To use friction:
// uClientFlags & CLIENT_FLAG_APPLY_FRICTION must be set
// The Following CollisionPoint_t variables must be set:
// vPoint
// pmFrictOr
// fKineticTransition
// fStaticHysteresis
// vKineticFriction
// vStaticFriction
// fTimeSinceCollision
// fFrictionSmoothTime
void CFPhysicsObject::_Friction( CollisionPoint_t &rPoint, PhysicsState_t &rState, CFVec3A *pvForce )
{
	CFVec3A vVel, vTemp, vLateral, vRolling, vVertical;
	CFVec3A vAccel, vLateralAccel;
	CFVec3A *pvFriction;
	f32 fVelMag;
//	f32 fMul;
	BOOL bKinetic = FALSE;

	if( !(rPoint.uClientFlags & CLIENT_FLAG_COLLIDED) && rPoint.fTimeSinceCollision >= rPoint.fFrictionSmoothTime )
	{
		pvForce->Zero();
		return;
	}

	if( !(rPoint.uClientFlags & CLIENT_FLAG_APPLY_FRICTION) )
	{
		pvForce->Zero();
		return;
	}

	// find velocity at contact point
	vTemp.Cross( rState.vAngularVelocity, rPoint.vPoint );
	vVel.Add( vTemp, rState.vVelocity );
	fVelMag = vVel.Mag();

	vTemp.Cross( rState.vAngularAcceleration, rPoint.vPoint );
	vAccel.Add( vTemp, rState.vAcceleration );
	vLateralAccel.Mul( rPoint.pmFrictOr->m_vRight, vAccel.Dot( rPoint.pmFrictOr->m_vRight ) );

	vLateral.Mul( rPoint.pmFrictOr->m_vRight, vVel.Dot( rPoint.pmFrictOr->m_vRight ) );
	vRolling.Mul( rPoint.pmFrictOr->m_vFront, vVel.Dot( rPoint.pmFrictOr->m_vFront ) );
	vVertical.Mul( rPoint.pmFrictOr->m_vUp, vVel.Dot( rPoint.pmFrictOr->m_vUp ) );

	if( rPoint.uLastResultFlags & COLLISION_RESULT_KINETIC )
	{
		// was sliding last frame

		if( vLateralAccel.Mag() > rPoint.fKineticTransition * rPoint.fStaticHysteresis )
		{
			// magnitude hasn't dropped below hysteresis, keep sliding
			bKinetic = TRUE;
		}
	}
	else
	{
		// not sliding last frame

		if( vLateralAccel.Mag() > rPoint.fKineticTransition )
		{
			// magnitude exceeded kinetic threshold, start sliding
			bKinetic = TRUE;
		}
	}

	if( rPoint.uClientFlags & CLIENT_FLAG_STATIC_FRICTION )
	{
		bKinetic = FALSE;
	}

	if( ( rPoint.uClientFlags & CLIENT_FLAG_KINETIC_FRICTION ) || bKinetic )
	{
		rPoint.uResultFlags |= COLLISION_RESULT_KINETIC;
		pvFriction = &rPoint.vKineticFriction;
	}
	else
	{
		rPoint.uResultFlags &= ~COLLISION_RESULT_KINETIC;
		pvFriction = &rPoint.vStaticFriction;
	}

//if( _pDrivenPhysObj == m_pPhysObj )
//{
//	if( rPoint.uLastResultFlags & COLLISION_RESULT_KINETIC )
//	{
//		ftext_Printf( 0.3f, 0.09f + 0.02f*_nPoint, "~f1~C92929299~w1~al~s0.69%d LAST", bKinetic );
//	}
//	else
//	{
//		ftext_Printf( 0.3f, 0.09f + 0.02f*_nPoint, "~f1~C92929299~w1~al~s0.69%d", bKinetic );
//	}
//}

	vLateral.Mul( m_pPhysObj->m_fMass );
	vLateral.Mul( -pvFriction->x );
	vRolling.Mul( m_pPhysObj->m_fMass );
	vRolling.Mul( -pvFriction->z );
	vVertical.Mul( m_pPhysObj->m_fMass );
	vVertical.Mul( -pvFriction->y );

	pvForce->Set( vLateral );
	pvForce->Add( vRolling );
	pvForce->Add( vVertical );
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::_Collide( CollisionPoint_t &rPoint, PhysicsState_t &rState )
{
	CFVec3A vVel, vPCrossN;
	CFVec3A vTemp, vTemp2;
	CFVec3A vPush;
	CFVec3A vOtherPoint;
	f32 fJ, fJDenom, fNormVel, fDepth;

	rPoint.Result.uFlags = rPoint.uResultFlags;
	rPoint.Result.vForce.Zero();
	rPoint.Result.vMomentum.Zero();

	if( rPoint.uClientFlags & CLIENT_FLAG_COLLIDED )
	{
		// selectively erase only certain flags so code below can access other flags
		// possibly set during previous collision.
		rPoint.Result.uFlags &= ~( COLLISION_RESULT_CONTACT | COLLISION_RESULT_COLLIDE | COLLISION_RESULT_MAX_SPRING_COMPRESS );
	}
	else
	{
		// reset all flags when point doesn't collide for a frame
		rPoint.Result.uFlags = COLLISION_RESULT_NONE;
		return;
	}

#ifdef _CHECK_FLOATS
	s32 nIndex;
	for( nIndex = 0; nIndex < 16; nIndex++ ) _CHECK_FLOAT( rPoint.pmFrictOr->a[nIndex], nIndex );
	for( nIndex = 0; nIndex < 4; nIndex++ ) _CHECK_FLOAT( rPoint.vStaticFriction.a[nIndex], nIndex );
	for( nIndex = 0; nIndex < 4; nIndex++ ) _CHECK_FLOAT( rPoint.vKineticFriction.a[nIndex], nIndex );
	for( nIndex = 0; nIndex < 4; nIndex++ ) _CHECK_FLOAT( rPoint.vPoint.a[nIndex], nIndex );
	for( nIndex = 0; nIndex < 4; nIndex++ ) _CHECK_FLOAT( rPoint.vNormal.a[nIndex], nIndex );
	for( nIndex = 0; nIndex < 4; nIndex++ ) _CHECK_FLOAT( rPoint.vAppliedForce.a[nIndex], nIndex );
	for( nIndex = 0; nIndex < 4; nIndex++ ) _CHECK_FLOAT( rPoint.vPush.a[nIndex], nIndex );
	_CHECK_FLOAT( rPoint.fTimeSinceCollision, 0 );
	_CHECK_FLOAT( rPoint.fDepth, 0 );
	_CHECK_FLOAT( rPoint.fKineticTransition, 0 );
	_CHECK_FLOAT( rPoint.fStaticHysteresis, 0 );
	_CHECK_FLOAT( rPoint.fAppliedForceKineticMul, 0 );
#endif

	vTemp.Cross( rState.vAngularVelocity, rPoint.vPoint );
	vVel.Add( vTemp, rState.vVelocity );

	// include spring velocity in total velocity at collision point
//	if( rPoint.uClientFlags & CLIENT_FLAG_SPRING_ACTIVE && rPoint.Spring.fVelocity < 0.0f )
	if( rPoint.uClientFlags & CLIENT_FLAG_SPRING_ACTIVE )
	{
		vTemp.Mul( rState.mOrientation.m_vUp, -rPoint.Spring.fVelocity );
		vVel.Add( vTemp );
	}
	
	if( rPoint.uClientFlags & CLIENT_FLAG_ADD_COLLIDE_VELOCITY )
	{
		vVel.Add( rPoint.vCollideVel );
	}

	// if collision is with other object, subtract its velocity at the collision point
	if( rPoint.pOtherObj )
	{
		vOtherPoint.Set( rPoint.vPoint );
		vOtherPoint.Add( rState.vPosition );
		vOtherPoint.Sub( *rPoint.pOtherObj->GetPosition() );
		rPoint.pOtherObj->GetVelocity( &vOtherPoint, &vTemp );
		vVel.Sub( vTemp );
	}

	// find normal magnitude of total velocity
	fNormVel = vVel.Dot( rPoint.vNormal );

	if( fNormVel >= 0.0f )
	{
		// point is moving away from impact surface
		return;
	}

	//-------------------------------------------------------------------------
	// Apply contact force if collision is small enough. Uses penalty force algorithm (actually "penalty velocity").
	if( fNormVel > -_CONTACT_VEL && !rPoint.pOtherObj )
	{
		rPoint.Result.uFlags |= COLLISION_RESULT_CONTACT;

		vPCrossN.Cross( rPoint.vPoint, rPoint.vNormal );
		rState.mInverseIT.MulPoint( vTemp, vPCrossN );
		vTemp.Cross( rPoint.vPoint );
		fJDenom = vTemp.Dot( rPoint.vNormal );
		fJDenom += m_fInverseMass;
		fJ = ( -1.0f * fNormVel ) / fJDenom;

		// compute instantaneous force of collision
		rPoint.Result.vMomentum.Set( rPoint.vNormal );
		rPoint.Result.vMomentum.Mul( fJ );

		// find collision depth
		vPush.Set( rPoint.vPush );
		vPush.Mul( rPoint.fDepth );
		fDepth = rPoint.vNormal.Dot( vPush );

		// scale contact force by collision depth and penalty force coefficient
		rPoint.Result.vMomentum.Mul( fDepth * _CONTACT_PENALTY_FORCE_COEFFICIENT );
	}
	//-------------------------------------------------------------
	// ordinary collsion, resolve it.
	else
	{
		// colliding
		rPoint.Result.uFlags |= COLLISION_RESULT_COLLIDE;

		// compute collision impulse
		vPCrossN.Cross( rPoint.vPoint, rPoint.vNormal );
		rState.mInverseIT.MulPoint( vTemp, vPCrossN );
		vTemp.Cross( rPoint.vPoint );
		fJDenom = vTemp.Dot( rPoint.vNormal );
		fJDenom += m_fInverseMass;

		fJ = -(1.0f + m_fElasticity) * fNormVel / fJDenom;
		// compute instantaneous force of collision
		rPoint.Result.vMomentum.Set( rPoint.vNormal );
		rPoint.Result.vMomentum.Mul( fJ );

		if( rPoint.pOtherObj )
		{
			fJ = -(1.0f + m_fOtherObjElasticity) * fNormVel / fJDenom;
			vTemp.Set( rPoint.vNormal );
			vTemp.Mul( fJ );
			vTemp.Negate();
			vTemp.Mul( rPoint.pOtherObj->GetInverseMass() );
			vTemp.Mul( m_fMass );
			rPoint.pOtherObj->ApplyMomentum( &vTemp, &vOtherPoint );
		}
	}
}

#define _SPRING_ITERATIONS			( 10 )
#define _OO_SPRING_ITERATIONS		( 1.0f / (f32) _SPRING_ITERATIONS )
#define _MIN_SPRING_COLLISION_VEL	( 2.0f )
#define _SPRING_COL_CONTACT_DEPTH	( 0.5f )
//-----------------------------------------------------------------------------
void CFPhysicsObject::_UpdateSpring( CollisionPoint_t &rPoint, PhysicsState_t &rState, f32 fDeltaTime )
{
	CFVec3A vVelocity, vNormVel, vTemp, vForce;
	f32 fNormVel, fDeltaVel, fLength;
	s32 nIterations;
	
#ifdef _CHECK_FLOATS
		s32 nIndex;
		s32 nSize = sizeof( rPoint.Spring ) / sizeof( f32 );
		f32 *pfValues = (f32 *) &rPoint.Spring;
		for( nIndex = 0; nIndex < nSize; nIndex++ ) _CHECK_FLOAT( pfValues[nIndex], nIndex );
#endif

	if( !( rPoint.uClientFlags & CLIENT_FLAG_SPRING_ACTIVE) )
	{
		return;
	}

	if( rPoint.uClientFlags & CLIENT_FLAG_COLLIDED )
	{
		// find total velocity at collision point
		vTemp.Cross( rState.vAngularVelocity, rPoint.vPoint );
		vVelocity.Add( vTemp, rState.vVelocity );

		// apply collision velocity to spring
		// a positive collision velocity causes negative velocities on spring (compressing spring)

		// find velocity in direction of spring travel
		fNormVel = -vVelocity.Dot( rPoint.pmFrictOr->m_vUp );

		// find delta velocity between spring and collision (adding because spring vel is negated)
		fDeltaVel = rPoint.Spring.fVelocity + fNormVel;

		// if collision is causing spring to compress, increase spring velocity to match collision velocity
		if( fDeltaVel > 0.0f )
		{
			rPoint.Spring.fVelocity -= fDeltaVel;
		}
	}

	for( nIterations = 0; nIterations < _SPRING_ITERATIONS; nIterations++ )
	{
		// integrate spring force into velocity
		rPoint.Spring.fVelocity += rPoint.Spring.fForce * fDeltaTime * _OO_SPRING_ITERATIONS;

		// integrate spring velocity into position
		rPoint.Spring.fCurrentLength += rPoint.Spring.fVelocity * fDeltaTime * _OO_SPRING_ITERATIONS;

		if( rPoint.Spring.fCurrentLength < rPoint.Spring.fCompressedLength )
		{
			rPoint.Spring.fCurrentLength = rPoint.Spring.fCompressedLength;
			rPoint.Spring.fVelocity = 0.0f;
		}

		if( rPoint.Spring.fCurrentLength > rPoint.Spring.fStretchedLength )
		{
			rPoint.Spring.fCurrentLength = rPoint.Spring.fStretchedLength;
			rPoint.Spring.fVelocity = 0.0f;
		}

		// compute new spring force
		fLength = rPoint.Spring.fRestLength - rPoint.Spring.fCurrentLength;
		rPoint.Spring.fForce =  fLength * rPoint.Spring.fStiffness;

		// add in any forces occuring at collision point
		if( rPoint.uClientFlags & CLIENT_FLAG_COLLIDED )
		{
			vForce.Zero();
			vForce.y = m_fGravity * m_fMass;
			rPoint.Spring.fForce += m_fInverseMass * rPoint.Spring.fAppliedForceRatio * vForce.Dot( rPoint.pmFrictOr->m_vUp );
		}

		if( rPoint.Spring.fVelocity > 0.0f )
		{
			// stretching
			rPoint.Spring.fForce -=  rPoint.Spring.fVelocity * rPoint.Spring.fDamping;
		}
		else
		{
			// compressing
			rPoint.Spring.fForce -= rPoint.Spring.fVelocity * rPoint.Spring.fDamping * rPoint.Spring.fDampingRatio;
		}

//		rPoint.Spring.fForce = _SmoothBipolarClampMin( rPoint.Spring.fForce, 300.0f, 900.0f );
//		rPoint.Spring.fVelocity = _SmoothBipolarClampMin( rPoint.Spring.fVelocity, 0.5f, 2.0f );
	}
}

//-----------------------------------------------------------------------------
// applies the result of a collision to the physics object
void CFPhysicsObject::_ApplyCollisionResult( CollisionPoint_t &rPoint, PhysicsState_t &rState )
{
	CFVec3A vTemp;

	rPoint.uResultFlags = rPoint.Result.uFlags;

	if( !(rPoint.uClientFlags & CLIENT_FLAG_COLLIDED) )
	{
		return;
	}

	rPoint.uLastResultFlags = rPoint.Result.uFlags;

	rState.vMomentum.Add( rPoint.Result.vMomentum );
	vTemp.Cross( rPoint.vPoint, rPoint.Result.vMomentum );
	rState.vAngularMomentum.Add( vTemp );
	// update auxiliary quantities
	rState.vVelocity.Mul( rState.vMomentum, m_fInverseMass );
	rState.mInverseIT.MulPoint( rState.vAngularVelocity, rState.vAngularMomentum );
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::_DampVelocities( PhysicsState_t &rState )
{
	f32 fLinearDamping = m_fLinearDamping;
	f32 fAngularDamping = m_fAngularDamping;

	if( (m_uFlags & FPHYSICS_FLAG_DYN_LIN_DAMP) || (m_uFlags & FPHYSICS_FLAG_DYN_ANG_DAMP) )
	{
		f32 fFramerateAdjust = 1.0f;

		if( FLoop_fPreviousLoopSecs > (1.0f/60.0f) )
		{
			fFramerateAdjust = (1.0f/60.0f) * FLoop_fPreviousLoopOOSecs;
			FMATH_CLAMP( fFramerateAdjust, 0.25f, 1.0f );
		}

		f32 fUnitVelDamp = (m_fDynDampVel - rState.vVelocity.Mag()) * m_fDynDampOOVel;
		FMATH_CLAMP( fUnitVelDamp, 0.0f, 1.0f );

		f32 fUnitAngDamp = (m_fDynDampAng - rState.vAngularVelocity.Mag()) * m_fDynDampOOAng;
		FMATH_CLAMP( fUnitAngDamp, 0.0f, 1.0f );

		f32 fUnitForceDamp = (m_fDynDampForce - rState.vTotalForce.Mag()) * m_fDynDampOOForce;
		FMATH_CLAMP( fUnitForceDamp, 0.0f, 1.0f );

		f32 fUnitTorqueDamp = (m_fDynDampForce - rState.vTotalTorque.Mag()) * m_fDynDampOOForce;
		FMATH_CLAMP( fUnitTorqueDamp, 0.0f, 1.0f );

		f32 fDamp = fUnitVelDamp;
		if( fUnitForceDamp < fDamp )
		{
			fDamp = fUnitForceDamp;
		}
		if( fUnitTorqueDamp < fDamp )
		{
			fDamp = fUnitTorqueDamp;
		}
		if( fUnitAngDamp < fDamp )
		{
			fDamp = fUnitAngDamp;
		}

//		ftext_Printf( 0.2f, 0.14f, "~f1~C92929299~w1~al~s0.69Velocity %f", rState.vVelocity.Mag() );
//		ftext_Printf( 0.2f, 0.16f, "~f1~C92929299~w1~al~s0.69Angular %f", rState.vAngularVelocity.Mag() );
//		ftext_Printf( 0.2f, 0.18f, "~f1~C92929299~w1~al~s0.69Force %f", rState.vTotalForce.Mag() );
//		ftext_Printf( 0.2f, 0.20f, "~f1~C92929299~w1~al~s0.69Torque %f", rState.vTotalTorque.Mag() );
//		ftext_Printf( 0.2f, 0.23f, "~f1~C92929299~w1~al~s0.69Unit Damping %f", fDamp );

		if( m_uFlags & FPHYSICS_FLAG_DYN_LIN_DAMP )
		{
			fLinearDamping = m_fMaxLinDamping * fDamp * fFramerateAdjust + m_fMinLinDamping;
		}

		if( m_uFlags & FPHYSICS_FLAG_DYN_ANG_DAMP )
		{
			fAngularDamping =  m_fMaxAngDamping * fDamp * fFramerateAdjust + m_fMinAngDamping;
		}
	}

	CFVec3A vTemp;
	// damp linear velocity
	vTemp.Mul( rState.vVelocity, -fLinearDamping );
	vTemp.Mul( m_fMass );
	rState.vTotalForce.Add( vTemp );

	// damp angular velocity
	vTemp.Mul( rState.vAngularVelocity, -fAngularDamping );
	vTemp.Mul( m_fMass );
	rState.vTotalTorque.Add( vTemp );
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::_ODE_Euler( f32 fDeltaTime, 
								  f32 *pafInput, 
								  f32 *pafOutput, 
								  f32 *pafWork, 
								  s32 nSize,
								  DerivativeCallback Derivative )
{
	s32 nIndex;

	// find state derivative
	Derivative( fDeltaTime, pafInput, pafWork );

	// perform integration
	for( nIndex = 0; nIndex < nSize; nIndex++ )
	{
		pafOutput[nIndex] = pafInput[nIndex] + pafWork[nIndex] * fDeltaTime;
	}
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::_ODE_Midpoint( f32 fDeltaTime, 
								  f32 *pafInput, 
								  f32 *pafOutput, 
								  f32 *pafWork, 
								  s32 nSize,
								  DerivativeCallback Derivative )
{
	s32 nIndex;

	// find state derivative at start time
	Derivative( fDeltaTime, pafInput, pafWork );

	// perform integration to halfway point
	for( nIndex = 0; nIndex < nSize; nIndex++ )
	{
		pafOutput[nIndex] = pafInput[nIndex] + pafWork[nIndex] * fDeltaTime * 0.5f;
	}

	// find derivative at start time plus half of fDeltaTime
//	Derivative( fDeltaTime * 0.5f, pafOutput, pafWork );
	Derivative( fDeltaTime, pafOutput, pafWork );

	// perform full integration using derivative at halfway point
	for( nIndex = 0; nIndex < nSize; nIndex++ )
	{
		pafOutput[nIndex] = pafInput[nIndex] + pafWork[nIndex] * fDeltaTime;
	}

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define _MAX_ITERATION_STEP_TIME	( 1.0f / 60.f )	
void CFPhysicsObject::Simulate( f32 fDeltaTime )
{
	PhysicsState_t NewState;
	f32 afInput[ _STATE_ARRAY_SIZE ];
	f32 afOutput[ _STATE_ARRAY_SIZE ];
	f32 afWork[ _STATE_ARRAY_SIZE ];
	s32 nIndex;
	CollisionPoint_t *pPoint;
	s32 nIteration;
	s32 nMaxIterations;
	f32 fOOMaxIterations;
	BOOL bClientForce = FALSE;

	_PRINT_FLOAT_CHECK_RESULT();

//	nMaxIterations = 2;
	nMaxIterations = (s32)(fDeltaTime / _MAX_ITERATION_STEP_TIME);
	FMATH_CLAMPMIN( nMaxIterations, 1 );
	FMATH_CLAMPMAX( nMaxIterations, 4 );
	fOOMaxIterations = 1.0f / (f32) nMaxIterations;

	if( m_uFlags & FPHYSICS_FLAG_FORCE_APPLIED )
	{
		bClientForce = TRUE;
	}

	m_uFlags &= ~( FPHYSICS_FLAG_CONTACTING | FPHYSICS_FLAG_COLLIDING | FPHYSICS_FLAG_FORCE_APPLIED );

	_DampVelocities( m_State );

	// used in ODE callback functions
	m_pPhysObj = this;

	NewState = m_State;

	// take multiple Euler steps if framerate is low
	// (so debug versions will run acceptably)
	for( nIteration = 0; nIteration < nMaxIterations; nIteration++ )
	{
		_StateToArray( NewState, &afInput[0] );

#ifdef _CHECK_FLOATS
		for( nIndex = 0; nIndex < _STATE_ARRAY_SIZE; nIndex++ ) _CHECK_FLOAT( afInput[nIndex], nIndex );
#endif

#if _USE_MIDPOINT
		_ODE_Midpoint( fDeltaTime * fOOMaxIterations, &afInput[0], &afOutput[0], &afWork[0], _STATE_ARRAY_SIZE, _StateArrayDerivative );
#else
		_ODE_Euler( fDeltaTime * fOOMaxIterations, &afInput[0], &afOutput[0], &afWork[0], _STATE_ARRAY_SIZE, _StateArrayDerivative );
#endif
		_ArrayToState( NewState, &afOutput[0] );

#ifdef _CHECK_FLOATS
		s32 nSize = sizeof( NewState ) / sizeof( f32 );
		f32 *pfValues = (f32 *) &NewState;
		for( nIndex = 0; nIndex < nSize; nIndex++ ) _CHECK_FLOAT( pfValues[nIndex], nIndex );
#endif

		// call client collision detection function
		if( m_pCollisionCB )
		{
			FASSERT( m_paColPoints != NULL );
			FASSERT( m_nColPoints > 0 );

			m_pCollisionCB( &NewState );

			for( nIndex = 0; nIndex < m_nColPoints; nIndex++ )
			{
				pPoint = &m_paColPoints[nIndex];

				if( m_fClampCoeff > 0.0f )
				{
					_UpdateSpring( *pPoint, NewState, fDeltaTime * fOOMaxIterations );
				}
				_Collide( *pPoint, NewState );
				_ApplyCollisionResult( *pPoint, NewState );

				if( pPoint->uResultFlags & COLLISION_RESULT_CONTACT )
				{
					m_uFlags |= FPHYSICS_FLAG_CONTACTING;
				}

				if( pPoint->uResultFlags & COLLISION_RESULT_COLLIDE )
				{
					m_uFlags |= FPHYSICS_FLAG_COLLIDING;
				}
			}
		}
	}

	m_State = NewState;

	if( m_paColPoints != NULL )
	{
		for( nIndex = 0; nIndex < m_nColPoints; nIndex++ )
		{
			pPoint = &m_paColPoints[nIndex];
			if(	pPoint->vAppliedForce.MagSq() > 0.0f )
			{
				bClientForce = TRUE;
				break;
			}
		}
	}

//if( _pDrivenPhysObj == m_pPhysObj )
//ftext_Printf( 0.2f, 0.18f, "~f1~C92929299~w1~al~s0.69bClientForce %d", bClientForce );

	// perform momentum clamping if indicated
	if( !bClientForce && ( m_uFlags & ( FPHYSICS_FLAG_MOM_CLAMP | FPHYSICS_FLAG_ANG_MOM_CLAMP ) ) )
	{
		f32 fMomCoeff = 1.0f;
		f32 fAngCoeff = 1.0f;

		// find coefficients for each typ of clamping
		if( m_uFlags & FPHYSICS_FLAG_MOM_CLAMP )
		{
			fMomCoeff = _SmoothVectorClampMinCoeff( m_State.vMomentum, m_fMinMomClamping, m_fMaxMomClamping );
		}

		if( m_uFlags & FPHYSICS_FLAG_ANG_MOM_CLAMP )
		{
			fAngCoeff = _SmoothVectorClampMinCoeff( m_State.vAngularMomentum, m_fMinAngMomClamping, m_fMaxAngMomClamping );
		}

		// if both types of clamping are enabled, use larger coefficient for both
		if( ( m_uFlags & FPHYSICS_FLAG_MOM_CLAMP ) && ( m_uFlags & FPHYSICS_FLAG_ANG_MOM_CLAMP ) )
		{
			if( fMomCoeff > fAngCoeff )
			{
				m_fClampCoeff = fMomCoeff;
			}
			else
			{
				m_fClampCoeff = fAngCoeff;
			}
		}

		// scale momenta by clamping coefficients
		m_State.vMomentum.Mul( m_fClampCoeff );
		m_State.vVelocity.Mul( m_fClampCoeff );

		m_State.vAngularMomentum.Mul( m_fClampCoeff );
		m_State.vAngularVelocity.Mul( m_fClampCoeff );

//		ftext_Printf( 0.2f, 0.14f, "~f1~C92929299~w1~al~s0.69Linear  Coeff %f", fMomCoeff );
//		ftext_Printf( 0.2f, 0.16f, "~f1~C92929299~w1~al~s0.69Angular Coeff %f", fAngCoeff );
	}
	else
	{
		m_fClampCoeff = 1.0f;
	}

//	ftext_Printf( 0.2f, 0.09f, "~f1~C92929299~w1~al~s0.69Linear  Mom %f", m_State.vMomentum.Mag() );
//	ftext_Printf( 0.2f, 0.11f, "~f1~C92929299~w1~al~s0.69Angular Mom %f", m_State.vAngularMomentum.Mag() );

#ifdef _CHECK_FLOATS
		s32 nSize = sizeof( m_State ) / sizeof( f32 );
		f32 *pfValues = (f32 *) &m_State;
		for( nIndex = 0; nIndex < nSize; nIndex++ ) _CHECK_FLOAT( pfValues[nIndex], nIndex );
#endif
}

//-----------------------------------------------------------------------------
BOOL CFPhysicsObject::InitLevel( void )
{
#ifdef _DRAW_DEBUG
	_nDebugLines = 0;
	_nDebugSpheres = 0;
#endif

	return TRUE;
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::UninitLevel( void )
{
}

//-----------------------------------------------------------------------------
// init object to default state
BOOL CFPhysicsObject::Init( void )
{
	_SetDefaults();
	return TRUE;
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::RegisterCollisionPoints( CollisionPoint_t *paPoints, u32 uPoints )
{
	m_paColPoints = paPoints;
	m_nColPoints = (s32) uPoints;
}

//-----------------------------------------------------------------------------
// set object mass
void CFPhysicsObject::SetMass( f32 fMass )
{
	FASSERT( fMass > 0.0f );
	m_fMass = fMass;
	m_fInverseMass = 1.0f / m_fMass;
	_ComputeLocalInertiaTensor();
	_ComputeWorldInertiaTensor( m_State );
}

//-----------------------------------------------------------------------------
// Returns total velocity of object center of mass.
const CFVec3A *CFPhysicsObject::GetVelocity( void ) const
{ 
	return &m_State.vVelocity;
}

//-----------------------------------------------------------------------------
// Returns total velocity of object at point pvPoint (object space).
void CFPhysicsObject::GetVelocity( const CFVec3A *pvPoint, CFVec3A *pvVel ) const
{ 
	FASSERT( pvPoint != NULL );
	FASSERT( pvVel != NULL );

	pvVel->Cross( m_State.vAngularVelocity, *pvPoint );
	pvVel->Add( m_State.vVelocity );
}

//-----------------------------------------------------------------------------
// sets the elasticity of the object.
// valid range from 0.0 to 1.0
void CFPhysicsObject::SetElasticity( f32 fElasticity )
{
	FASSERT( fElasticity >= 0.0f && fElasticity <= 1.0f );
	m_fElasticity = fElasticity;
}

//-----------------------------------------------------------------------------
// sets the elasticity of collisions with pOtherObj objects.
// valid range from 0.0 to 1.0
void CFPhysicsObject::SetOtherObjElasticity( f32 fElasticity )
{
	FASSERT( fElasticity >= 0.0f && fElasticity <= 1.0f );
	m_fOtherObjElasticity = fElasticity;
}

//-----------------------------------------------------------------------------
// rotates the object's current orientation via three euler angles (radians).
// recomputes intertia tensor.
void CFPhysicsObject::Rotate( f32 fXRot, f32 fYRot, f32 fZRot )
{

	if( fZRot != 0.0f )
	{
		m_State.mOrientation.RotateZ( fZRot );
	}

	if( fYRot != 0.0f )
	{
		m_State.mOrientation.RotateY( fYRot );
	}

	if( fXRot != 0.0f )
	{
		m_State.mOrientation.RotateX( fXRot );
	}

	_ComputeWorldInertiaTensor( m_State );
}

//-----------------------------------------------------------------------------
// set the object' rotation via a matrix
void CFPhysicsObject::SetOrientation( const CFMtx43A *pmMatrix )
{
	m_State.mOrientation.m_vX = pmMatrix->m_vX;
	m_State.mOrientation.m_vY = pmMatrix->m_vY;
	m_State.mOrientation.m_vZ = pmMatrix->m_vZ;

	_ComputeWorldInertiaTensor( m_State );
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::SetCollisionDetectionCallback( CollDetectCallback pCB )
{
	m_pCollisionCB = pCB;
}

//-----------------------------------------------------------------------------
// Sets the inertial size of the object (as a rectangular box).
// Values are "half size", i.e. length from center-of-mass to one
// wall of the enclosing box.
// N.B. Box should enclose entire space where collision points
// could possibly occur.  Collision points which are outside the
// enclosing box should not be passed in to the object.
void CFPhysicsObject::SetInertialSize( f32 fX, f32 fY, f32 fZ )
{
	FASSERT( fX > 0.0f && fY > 0.0f && fZ > 0.0f );

	m_vInertialSize.Set( fX, fY, fZ );

	_ComputeLocalInertiaTensor();
	_ComputeWorldInertiaTensor( m_State );
}

//-----------------------------------------------------------------------------
// applies a force to the object.
// force = force vector to apply to object, 
// appPoint = object-local point of application
void CFPhysicsObject::ApplyForce( const CFVec3A *pvForce, const CFVec3A *pvAppPoint /* = NULL */ )
{
	if( pvForce->MagSq() <= 0.0f )
	{
		return;
	}

	m_uFlags |= FPHYSICS_FLAG_FORCE_APPLIED;

	m_State.vTotalForce.Add( *pvForce );

	if( pvAppPoint )
	{
		CFVec3A vTemp;
		vTemp.Cross( *pvAppPoint, *pvForce );
		m_State.vTotalTorque.Add( vTemp );
	}
}


//-----------------------------------------------------------------------------
// applies a momentum to the object.
void CFPhysicsObject::ApplyMomentum( const CFVec3A *pvMomentum, const CFVec3A *pvAppPoint /* = NULL */ )
{
	m_State.vMomentum.Add( *pvMomentum );
	// update auxiliary quantity
	m_State.vVelocity.Mul( m_State.vMomentum, m_fInverseMass );

	if( pvAppPoint != NULL )
	{
		CFVec3A vTemp;
		vTemp.Cross( *pvAppPoint, *pvMomentum );
		m_State.vAngularMomentum.Add( vTemp );
		// update auxiliary quantity
		m_State.mInverseIT.MulPoint( m_State.vAngularVelocity, m_State.vAngularMomentum );
	}
}

//-----------------------------------------------------------------------------
// applies an angular momentum to the object.
void CFPhysicsObject::ApplyAngularMomentum( const CFVec3A *pvMomentum )
{
	m_State.vAngularMomentum.Add( *pvMomentum );
	// update auxiliary quantity
	m_State.mInverseIT.MulPoint( m_State.vAngularVelocity, m_State.vAngularMomentum );
}

//-----------------------------------------------------------------------------
// applies a frictional force to the object.
// force = force vector to apply to object, 
// appPoint = object-local point of application
void CFPhysicsObject::ApplyFriction( const CFVec3A *pvForce, const CFVec3A *pvAppPoint /* = NULL */ )
{
	m_State.vTotalFriction.Add( *pvForce );

	if( pvAppPoint )
	{
		CFVec3A vTemp;
		vTemp.Cross( *pvAppPoint, *pvForce );
		m_State.vTotalFrictionTorque.Add( vTemp );
	}
}

//-----------------------------------------------------------------------------
// applies a torque to the object.
void CFPhysicsObject::ApplyTorque( const CFVec3A *pvTorque )
{
	m_State.vTotalTorque.Add( *pvTorque );
}

//-----------------------------------------------------------------------------
// zero out all forces & torques applied to object so far this frame
void CFPhysicsObject::ClearForces( PhysicsState_t &rState )
{
	// clear force and torque 
	rState.vTotalForce.Zero();
	rState.vTotalTorque.Zero();

	rState.vTotalFriction.Zero();
	rState.vTotalFrictionTorque.Zero();
}

//-----------------------------------------------------------------------------
// static function to initialize a client's collision point struct.
void CFPhysicsObject::InitCollisionPoint( CollisionPoint_t *pPoint )
{
	pPoint->pmFrictOr = NULL;
	pPoint->vStaticFriction.Zero();
	pPoint->vKineticFriction.Zero();
	pPoint->vPoint.Zero();
	pPoint->vNormal.Zero();
	pPoint->vAppliedForce.Zero();
	pPoint->vPush.Zero();
	pPoint->vCollideVel.Zero();
	pPoint->pOtherObj = NULL;
	pPoint->fTimeSinceCollision = 0.0f;
	pPoint->fDepth = 0.0f;
	pPoint->fKineticTransition = 0.0f;
	pPoint->fStaticHysteresis = 0.0f;
	pPoint->fAppliedForceKineticMul = 0.0f;
	pPoint->fFrictionSmoothTime = 0.0f;
	pPoint->uClientFlags = 0;
	pPoint->uResultFlags = 0;
	pPoint->uLastResultFlags = 0;

	pPoint->Result.uFlags = 0;
	pPoint->Result.vForce.Zero();
	pPoint->Result.vMomentum.Zero();

	pPoint->Spring.fAppliedForceRatio = 0.0f;
	pPoint->Spring.fCompressedLength = 0.0f;
	pPoint->Spring.fCurrentLength = 0.0f;
	pPoint->Spring.fDamping = 0.0f;
	pPoint->Spring.fDampingRatio = 0.0f;
	pPoint->Spring.fForce = 0.0f;
	pPoint->Spring.fRestLength = 0.0f;
	pPoint->Spring.fStiffness = 0.0f;
	pPoint->Spring.fStretchedLength = 0.0f;
	pPoint->Spring.fVelocity = 0.0f;
}


//-----------------------------------------------------------------------------
// enables and sets range for smoothly clamping low momentum values down to zero
void CFPhysicsObject::SetMomentumClamping( BOOL bEnable, f32 fMinMom, f32 fMaxMom )
{
	if( bEnable )
	{
		m_uFlags |= FPHYSICS_FLAG_MOM_CLAMP;
	}
	else
	{
		m_uFlags &= ~FPHYSICS_FLAG_MOM_CLAMP;
	}

	if( bEnable )
	{
		m_fMinMomClamping = fMinMom;
		m_fMaxMomClamping = fMaxMom;
	}
}

//-----------------------------------------------------------------------------
// enables and sets range for smoothly clamping low angular momentum values down to zero
void CFPhysicsObject::SetAngularMomentumClamping( BOOL bEnable, f32 fMinMom, f32 fMaxMom )
{
	if( bEnable )
	{
		m_uFlags |= FPHYSICS_FLAG_ANG_MOM_CLAMP;
	}
	else
	{
		m_uFlags &= ~FPHYSICS_FLAG_ANG_MOM_CLAMP;
	}

	if( bEnable )
	{
		m_fMinAngMomClamping = fMinMom;
		m_fMaxAngMomClamping = fMaxMom;
	}
}


//-----------------------------------------------------------------------------
// sets the parameters that control the amount of dynamic damping applied
void CFPhysicsObject::SetDynamicDampingParams( f32 fVelocity, f32 fAngular, f32 fForce )
{
	FASSERT( fVelocity >= 0.0f );
	FASSERT( fAngular >= 0.0f );
	FASSERT( fForce >= 0.0f );

	m_fDynDampVel = fVelocity;
	m_fDynDampAng = fAngular;
	m_fDynDampForce = fForce;

	if( fVelocity > 0.0 )
	{
		m_fDynDampOOVel = fmath_Inv( fVelocity );
	}
	else
	{
		m_fDynDampOOVel = 0.0f;
	}

	if( fAngular > 0.0 )
	{
		m_fDynDampOOAng = fmath_Inv( fAngular );
	}
	else
	{
		m_fDynDampOOAng = 0.0f;
	}

	if( fForce > 0.0 )
	{
		m_fDynDampOOForce = fmath_Inv( fForce );
	}
	else
	{
		m_fDynDampOOForce = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// enables and sets range of dynamic linear damping
void CFPhysicsObject::SetDynamicLinearDamping( BOOL bEnable, f32 fMinDamping, f32 fMaxDamping )
{
	if( bEnable )
	{
		m_uFlags |= FPHYSICS_FLAG_DYN_LIN_DAMP;

		FASSERT( fMinDamping >= 0.0f );
		FASSERT( fMaxDamping >= 0.0f );
		m_fMinLinDamping = fMinDamping;
		m_fMaxLinDamping = fMaxDamping;
	}
	else
	{
		m_uFlags &= ~FPHYSICS_FLAG_DYN_LIN_DAMP;
	}
}

//-----------------------------------------------------------------------------
// enables and sets range of dynamic angular damping
void CFPhysicsObject::SetDynamicAngularDamping( BOOL bEnable, f32 fMinDamping, f32 fMaxDamping )
{
	if( bEnable )
	{
		m_uFlags |= FPHYSICS_FLAG_DYN_ANG_DAMP;

		FASSERT( fMinDamping >= 0.0f );
		FASSERT( fMaxDamping >= 0.0f );
		m_fMinAngDamping = fMinDamping;
		m_fMaxAngDamping = fMaxDamping;
	}
	else
	{
		m_uFlags &= ~FPHYSICS_FLAG_DYN_ANG_DAMP;
	}
}

//-----------------------------------------------------------------------------
// draw axes of object
void CFPhysicsObject::DrawObject( void )
{
	CFVec3A vStart, vEnd, vColor;
	vColor.Set( 1.0f, 1.0f, 1.0f );

	vEnd = m_State.mOrientation.m_vX;
	vEnd.Mul( 10.0f );
	vEnd.Add( m_State.vPosition );
	DrawLine( &m_State.vPosition, &vEnd, &vColor );

	vEnd = m_State.mOrientation.m_vY;
	vEnd.Mul( 10.0f );
	vEnd.Add( m_State.vPosition );
	DrawLine( &m_State.vPosition, &vEnd, &vColor );

	vEnd = m_State.mOrientation.m_vZ;
	vEnd.Mul( 10.0f );
	vEnd.Add( m_State.vPosition );
	DrawLine( &m_State.vPosition, &vEnd, &vColor );

	// draw inertial object bounds

	//draw rectangle at front of object
	vStart.Set( m_vInertialSize );
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.x = -vEnd.x;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.y = -vEnd.y;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.x = -vStart.x;
	vStart.y = -vStart.y;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.y = -vEnd.y;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.x = -vStart.x;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.x = -vEnd.x;
	vEnd.y = -vEnd.y;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );


	// draw rectangle at rear of object
	vStart.Set( m_vInertialSize );
	vStart.z = -vStart.z;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.x = -vEnd.x;
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.z = -vStart.z;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.y = -vEnd.y;
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.x = -vStart.x;
	vStart.y = -vStart.y;
	vStart.z = -vStart.z;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.y = -vEnd.y;
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.x = -vStart.x;
	vStart.z = -vStart.z;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.x = -vEnd.x;
	vEnd.y = -vEnd.y;
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	// draw lines along z axis connecting rectangles above
	vStart.Set( m_vInertialSize );
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.x = -vStart.x;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.x = -vEnd.x;
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.y = -vStart.y;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.y = -vEnd.y;
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );

	vStart.Set( m_vInertialSize );
	vStart.x = -vStart.x;
	vStart.y = -vStart.y;
	m_State.mOrientation.MulPoint( vStart );
	vStart.Add( m_State.vPosition );
	vEnd.Set( m_vInertialSize );
	vEnd.x = -vEnd.x;
	vEnd.y = -vEnd.y;
	vEnd.z = -vEnd.z;
	m_State.mOrientation.MulPoint( vEnd );
	vEnd.Add( m_State.vPosition );
	DrawLine( &vStart, &vEnd, &vColor );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CFPhysicsObject::DebugDrawWork( void )
{
#ifdef _DRAW_DEBUG
	FDrawVtx_t start, end;
	frenderer_SetDefaultState();
	s32 nIndex;
	for( nIndex = 0; nIndex < _nDebugLines; nIndex++ )
	{
		fdraw_Line( &_DebugLines[ nIndex ].vStart, &_DebugLines[ nIndex ].vEnd );
	}
	_nDebugLines = 0;

	for( nIndex = 0; nIndex < _nDebugSpheres; nIndex++ )
	{
		fdraw_FacetedWireSphere( &_DebugSpheres[nIndex].vPos, _DebugSpheres[nIndex].fRadius, 2, 2, &_DebugSpheres[nIndex].vColor, 3 );
	}
	_nDebugSpheres = 0;
#endif
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::DrawLine( CFVec3A *pStart, CFVec3A *pEnd, CFVec3A *pColor )
{
#ifdef _DRAW_DEBUG
	CFVec3A vColor;

	if( _nDebugLines >= _MAX_DEBUG_LINES )
	{
		return;
	}

	if( pColor == NULL )
	{
		pColor = &vColor;
		vColor.Set( 1.0f, 1.0f, 1.0f );
	}

	_DebugLines[ _nDebugLines ].vStart.Pos_MS.Set( pStart->x, pStart->y, pStart->z );
	_DebugLines[ _nDebugLines ].vEnd.Pos_MS.Set( pEnd->x, pEnd->y, pEnd->z );
	_DebugLines[ _nDebugLines ].vStart.ColorRGBA.Set( pColor->x, pColor->y, pColor->z, 1.0f );
	_DebugLines[ _nDebugLines ].vEnd.ColorRGBA.Set( pColor->x, pColor->y, pColor->z, 1.0f );
//	_DebugLines[ _nDebugLines ].vEnd.ColorRGBA.Set( 0.5f, 0.5f, 0.5f, 1.0f );

	++_nDebugLines;
#endif
}

//-----------------------------------------------------------------------------
void CFPhysicsObject::DrawSphere( CFVec3A *pPos, f32 fRadius, CFVec3A *pColor )
{
#ifdef _DRAW_DEBUG
	CFVec3A vColor;

	if( _nDebugSpheres >= _MAX_DEBUG_SPHERES )
	{
		return;
	}

	if( pColor == NULL )
	{
		pColor = &vColor;
		vColor.Set( 1.0f, 1.0f, 1.0f );
	}

	_DebugSpheres[ _nDebugSpheres ].vPos.Set( pPos->x, pPos->y, pPos->z );
	_DebugSpheres[ _nDebugSpheres ].fRadius = fRadius;
	_DebugSpheres[ _nDebugSpheres ].vColor.Set( pColor->x, pColor->y, pColor->z, 1.0f );
	++_nDebugSpheres;
#endif
}


