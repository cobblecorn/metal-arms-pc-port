//////////////////////////////////////////////////////////////////////////////////////
// motion.cpp - Physical motion class.
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
// 06/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fmotion.h"


#define _IMPULSE_DURATION	(1.0f / 60.0f)




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFMotionSimple
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CFMotionSimple::Simulate( f32 fDeltaTime ) {
	CFQuatA SrcQuat, RotateQuat;
	CFVec3A DeltaPos;
	f32 fHalfAngle, fSin, fCos;

	// Update linear position...
	DeltaPos.Mul( m_Vel, fDeltaTime );
	m_Pos.Add( DeltaPos );

	// Update rotation orientation...
	SrcQuat = m_OrientQuat;

	fHalfAngle = m_fAngularSpeed * fDeltaTime;
	fmath_SinCos( fHalfAngle, &fSin, &fCos );
	FASSERT( m_RotUnitAxis.MagSq() <= 1.01f );

	RotateQuat.x = m_RotUnitAxis.x * fSin;
	RotateQuat.y = m_RotUnitAxis.y * fSin;
	RotateQuat.z = m_RotUnitAxis.z * fSin;
	RotateQuat.w = fCos;

	m_OrientQuat.Mul( RotateQuat, SrcQuat );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFMotion
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

	void CFMotion::Simple_InitBody( const CFMtx43A *pInitialMtx, f32 fOOUniformTensorMag, f32 fMass, f32 fGravity ) {
	FASSERT( fMass > 0.0f );

	m_fMass = fMass;
	m_fOOMass = 1.0f / fMass;
	m_fGravity = fGravity;
	m_fOOUniformTensorMag = fOOUniformTensorMag;

	m_OOInertia_WS.Identity();
	m_OOInertia_WS.aa[0][0] = fOOUniformTensorMag;
	m_OOInertia_WS.aa[1][1] = fOOUniformTensorMag;
	m_OOInertia_WS.aa[2][2] = fOOUniformTensorMag;

	m_ForceSum_WS.Zero();
	m_TorqueSum_WS.Zero();
	m_ImpulseForceSum_WS.Zero();
	m_ImpulseTorqueSum_WS.Zero();
	StopMotion();

	m_TransposeMtx33.m_vPos.Zero();
	Simple_UpdateOrientation( pInitialMtx );
}


void CFMotion::Simple_Simulate( f32 fDeltaTime ) {
	CFMtx43A TempMtx;
	CFVec3A TempVec1, TempVec2, AngularMomentum_MS, LinearVelocity_MS;
	CFQuatA AngularVelocityQuat, DeltaQuat;
	f32 fDeltaTimeOverMass;

	// Add gravity to the center-of-mass...
	m_ForceSum_WS.y += (m_fGravity * m_fMass);

	// Compute new position...
	TempVec1.Mul( m_LinearVelocity_WS, fDeltaTime );
	m_Mtx.m_vPos.Add( TempVec1 );

	// Compute new linear velocity...
	fDeltaTimeOverMass = fDeltaTime * m_fOOMass;
	TempVec1.Mul( m_ForceSum_WS, fDeltaTimeOverMass );
	TempVec2.Mul( m_ImpulseForceSum_WS, _IMPULSE_DURATION );
	m_LinearVelocity_WS.Add( TempVec1 ).Add( TempVec2 );

	// Zero out accumulators...
	m_ForceSum_WS.Zero();
	m_ImpulseForceSum_WS.Zero();

	// Compute new orientation quaternion...
	AngularVelocityQuat.Set( m_AngularVelocity_WS );
	DeltaQuat.Mul( AngularVelocityQuat, m_Quat );
	DeltaQuat.v.Mul( 0.5f * fDeltaTime );
	m_Quat.v.Add( DeltaQuat.v );
	m_Quat.Unitize();
	m_Quat.BuildMtx33( m_Mtx );
	m_TransposeMtx33.ReceiveTranspose33( m_Mtx );

	if( (m_TorqueSum_WS.MagSq() > 0.01f) || (m_ImpulseTorqueSum_WS.MagSq() > 0.0f) ) {
		// Compute new angular momentum...
		TempVec1.Mul( m_TorqueSum_WS, fDeltaTime );
		TempVec2.Mul( m_ImpulseTorqueSum_WS, _IMPULSE_DURATION );
		m_AngularMomentum_WS.Add( TempVec1 ).Add( TempVec2 );

		// Compute new angular velocity...
		m_OOInertia_WS.MulDir( m_AngularVelocity_WS, m_AngularMomentum_WS );

		// Zero out accumulator...
		m_TorqueSum_WS.Zero();
		m_ImpulseTorqueSum_WS.Zero();
	}
}


void CFMotion::Simple_UpdateOrientation( const CFMtx43A *pNewOrientation ) {
	CFMtx43A TempMtx;

	m_Mtx = *pNewOrientation;
	m_TransposeMtx33.ReceiveTranspose33( *pNewOrientation );
	m_Quat.BuildQuat( *pNewOrientation );
}


f32 CFMotion::ComputeInvUniformInertiaTensorMag( f32 fUniformBodyDim, f32 fMass ) {
	return fmath_Div( 6.0f, fMass * fUniformBodyDim * fUniformBodyDim );
}


void CFMotion::Complex_InitBody( const CFMtx43A *pInitialMtx, const CFMtx43A *pOOInertiaTensor_MS, f32 fMass, f32 fGravity ) {
	FASSERT( fMass > 0.0f );

	m_fMass = fMass;
	m_fOOMass = 1.0f / fMass;
	m_fGravity = fGravity;
	m_OOInertiaTensor_MS = *pOOInertiaTensor_MS;
	m_InertiaTensor_MS.ReceiveInverse( *pOOInertiaTensor_MS );
	m_InertiaTensor_MS.m_vPos.Zero();

	m_ForceSum_WS.Zero();
	m_TorqueSum_WS.Zero();
	m_ImpulseForceSum_WS.Zero();
	m_ImpulseTorqueSum_WS.Zero();
	StopMotion();

	m_TransposeMtx33.m_vPos.Zero();
	Complex_UpdateOrientation( pInitialMtx );
}


void CFMotion::Complex_UpdateOrientation( const CFMtx43A *pNewOrientation ) {
	CFMtx43A TempMtx;

	m_Mtx = *pNewOrientation;
	m_TransposeMtx33.ReceiveTranspose33( *pNewOrientation );
	m_Quat.BuildQuat( *pNewOrientation );

	TempMtx.Mul33( m_Mtx, m_OOInertiaTensor_MS );
	TempMtx.m_vPos.Zero();
	m_OOInertia_WS.Mul33( TempMtx, m_TransposeMtx33 );
}


void CFMotion::Complex_Simulate( f32 fDeltaTime, BOOL bApplyTransVel/*=TRUE*/, BOOL bApplyRotVel/*=TRUE*/ ) {
	CFMtx43A TempMtx;
	CFVec3A TempVec1, TempVec2, AngularMomentum_MS, LinearVelocity_MS;
	CFQuatA AngularVelocityQuat, DeltaQuat;

#if 0
	{
		f32 fMag2 = m_AngularVelocity_WS.MagSq();
		if( fMag2 > (20.0f * 20.0f) ) {
			m_AngularVelocity_WS.Mul( 20.0f * fmath_InvSqrt(fMag2) );
		}
	}
#endif

	// Add gravity to the center-of-mass...
	m_ForceSum_WS.y += (m_fGravity * m_fMass);

	if ( bApplyTransVel ) {
		// Compute new position...
		TempVec1.Mul( m_LinearVelocity_WS, fDeltaTime );
		m_Mtx.m_vPos.Add( TempVec1 );
	}

	// Compute new linear velocity...
	TempVec1.Mul( m_ForceSum_WS, fDeltaTime * m_fOOMass );
	TempVec2.Mul( m_ImpulseForceSum_WS, _IMPULSE_DURATION * m_fOOMass );
	m_LinearVelocity_WS.Add( TempVec1 ).Add( TempVec2 );

	// Zero out accumulator...
	m_ForceSum_WS.Zero();
	m_ImpulseForceSum_WS.Zero();

#if 1
	if ( bApplyRotVel ) {
		// Compute new orientation quaternion...
		AngularVelocityQuat.Set( m_AngularVelocity_WS );
		DeltaQuat.Mul( AngularVelocityQuat, m_Quat );
		DeltaQuat.v.Mul( 0.5f * fDeltaTime );
		m_Quat.v.Add( DeltaQuat.v );
		m_Quat.Unitize();
		m_Quat.BuildMtx33( m_Mtx );
		m_TransposeMtx33.ReceiveTranspose33( m_Mtx );
	}

	// Compute new angular momentum...
	TempVec1.Mul( m_TorqueSum_WS, fDeltaTime );
	TempVec2.Mul( m_ImpulseTorqueSum_WS, _IMPULSE_DURATION );
	m_AngularMomentum_WS.Add( TempVec1 ).Add( TempVec2 );

	// Compute moment of inertia...
	TempMtx.Mul33( m_Mtx, m_OOInertiaTensor_MS );
	m_OOInertia_WS.Mul33( TempMtx, m_TransposeMtx33 );

	// Compute new angular velocity...
	m_OOInertia_WS.MulDir( m_AngularVelocity_WS, m_AngularMomentum_WS );

	// Zero out accumulator...
	m_TorqueSum_WS.Zero();
	m_ImpulseTorqueSum_WS.Zero();
#else
	u32 i;
	fDeltaTime *= (1.0f / 10.0f);
	m_TorqueSum_WS.Mul( 1.0f / 10.0f );
	m_ImpulseTorqueSum_WS.Mul( 1.0f / 10.0f );

	for( i=0; i<10; ++i ) {
		// Compute new orientation quaternion...
		AngularVelocityQuat.Set( m_AngularVelocity_WS );
		DeltaQuat.Mul( AngularVelocityQuat, m_Quat );
		DeltaQuat.v.Mul( 0.5f * fDeltaTime );
		m_Quat.v.Add( DeltaQuat.v );
		m_Quat.Unitize();
		m_Quat.BuildMtx33( m_Mtx );
		m_TransposeMtx33.ReceiveTranspose33( m_Mtx );

		// Compute new angular momentum...
		TempVec1.Mul( m_TorqueSum_WS, fDeltaTime );
		TempVec2.Mul( m_ImpulseTorqueSum_WS, _IMPULSE_DURATION );
		m_AngularMomentum_WS.Add( TempVec1 ).Add( TempVec2 );

		// Compute moment of inertia...
		TempMtx.Mul33( m_Mtx, m_OOInertiaTensor_MS );
		m_OOInertia_WS.Mul33( TempMtx, m_TransposeMtx33 );

		// Compute new angular velocity...
		m_OOInertia_WS.MulDir( m_AngularVelocity_WS, m_AngularMomentum_WS );
	}

	// Zero out accumulator...
	m_TorqueSum_WS.Zero();
	m_ImpulseTorqueSum_WS.Zero();
#endif
}


// Computes the inverse inertia tensor in model space using the given minimum X,Y,Z and
// maximum X,Y,Z corners of a 3D box.
void CFMotion::ComputeInvInertiaTensor_MS( CFMtx43A *pInertiaTensor, const CFVec3A *pBoxMin, const CFVec3A *pBoxMax, f32 fMassAtMinZ, f32 fMassAtMaxZ ) {
	f32 fMinX, fMaxX, fMinY, fMaxY, fMinZ, fMaxZ;
	f32 fTempX, fTempY, fTempZ, fDM, fDmPlus2M0;
	CFMtx43A InertiaTensor;

	fDM = fMassAtMaxZ - fMassAtMinZ;
	fDmPlus2M0 = fDM + 2.0f*fMassAtMinZ;

	fMinX = pBoxMin->x;
	fMinY = pBoxMin->y;
	fMinZ = pBoxMin->z;

	fMaxX = pBoxMax->x;
	fMaxY = pBoxMax->y;
	fMaxZ = pBoxMax->z;

	fTempX = (fMinX*fMinX + fMinX*fMaxX + fMaxX*fMaxX) * fDmPlus2M0 * (1.0f/6.0f);
	fTempY = (fMinY*fMinY + fMinY*fMaxY + fMaxY*fMaxY) * fDmPlus2M0 * (1.0f/6.0f);
	fTempZ = fMinZ*fMinZ*( (1.0f/12.0f)*fDM + (1.0f/3.0f)*fMassAtMinZ )
		   + fMinZ*fMaxZ*( (1.0f/ 6.0f)*fDM + (1.0f/3.0f)*fMassAtMinZ )
		   + fMaxZ*fMaxZ*( (1.0f/ 4.0f)*fDM + (1.0f/3.0f)*fMassAtMinZ );

	InertiaTensor.Identity();

	InertiaTensor.aa[0][0] = fTempY + fTempZ;
	InertiaTensor.aa[1][1] = fTempX + fTempZ;
	InertiaTensor.aa[2][2] = fTempX + fTempY;

	fTempZ = (-1.0f/12.0f)*(fMinZ*fDM + 3.0f*fMinZ*fMassAtMinZ + 2.0f*fMaxZ*fDM + 3.0f*fMaxZ*fMassAtMinZ);
	InertiaTensor.aa[0][1] = InertiaTensor.aa[1][0] = (-1.0f/8.0f)*fDmPlus2M0*(fMinX+fMaxX)*(fMinY+fMaxY);
	InertiaTensor.aa[0][2] = InertiaTensor.aa[2][0] = fTempZ * (fMinX+fMaxX);
	InertiaTensor.aa[1][2] = InertiaTensor.aa[2][1] = fTempZ * (fMinY+fMaxY);

	// Invert the matrix...
	pInertiaTensor->ReceiveInverse( InertiaTensor );
	pInertiaTensor->m_vPos.Zero();
}


// Computes the inverse inertia tensor in model space using the given minimum X,Y,Z and
// maximum X,Y,Z corners of a 3D box.
void CFMotion::ComputeInvInertiaTensor_MS( CFMtx43A *pInertiaTensor, const CFVec3A *pBoxMin, const CFVec3A *pBoxMax, f32 fMass ) {
	f32 fScaledMass, fMinX, fMaxX, fMinY, fMaxY, fMinZ, fMaxZ;

	fScaledMass = fMass * (1.0f / 3.0f);

	fMinX = pBoxMin->x;
	fMinY = pBoxMin->y;
	fMinZ = pBoxMin->z;

	fMaxX = pBoxMax->x;
	fMaxY = pBoxMax->y;
	fMaxZ = pBoxMax->z;

	pInertiaTensor->Identity();

	pInertiaTensor->aa[0][0] = 1.0f / (fScaledMass * (fMinY*fMinY + fMinY*fMaxY + fMaxX*fMaxX + fMaxX*fMinX + fMinX*fMinX + fMaxY*fMaxY));
	pInertiaTensor->aa[1][1] = 1.0f / (fScaledMass * (fMaxZ*fMaxZ + fMinZ*fMaxZ + fMaxX*fMinX + fMaxX*fMaxX + fMinX*fMinX + fMinZ*fMinZ));
	pInertiaTensor->aa[2][2] = 1.0f / (fScaledMass * (fMaxZ*fMaxZ + fMinZ*fMaxZ + fMinY*fMinY + fMaxY*fMaxY + fMinY*fMaxY + fMinZ*fMinZ));
}


void CFMotion::StopMotion( void ) {
	m_LinearVelocity_WS.Zero();
	m_AngularVelocity_WS.Zero();
	m_AngularMomentum_WS.Zero();
}


void CFMotion::ApplyForce_WS( const CFVec3A *pApplicationPoint_WS, const CFVec3A *pForce_WS ) {
	CFVec3A CMToPointVec_WS, Torque_WS;

	// Add the force to our force total...
	m_ForceSum_WS.Add( *pForce_WS );

	// Compute the torque and add it to our total...
	CMToPointVec_WS.Sub( *pApplicationPoint_WS, m_Mtx.m_vPos );
	Torque_WS.Cross( CMToPointVec_WS, *pForce_WS );
	m_TorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyForce_MS( const CFVec3A *pApplicationPoint_MS, const CFVec3A *pForce_MS ) {
	CFVec3A Torque_MS, Torque_WS, Force_WS;

	// Compute the force and add it to our total...
	m_Mtx.MulDir( Force_WS, *pForce_MS );
	m_ForceSum_WS.Add( Force_WS );

	// Compute the torque and add it to our total...
	Torque_MS.Cross( *pApplicationPoint_MS, *pForce_MS );
	m_Mtx.MulDir( Torque_WS, Torque_MS );
	m_TorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyForce_GS( const CFMtx43A *m_pMtx_GS, const CFVec3A *pApplicationPoint_GS, const CFVec3A *pForce_GS ) {
	CFVec3A Torque_GS, Torque_WS, Force_WS;

	// Compute the force and add it to our total...
	m_pMtx_GS->MulDir( Force_WS, *pForce_GS );
	m_ForceSum_WS.Add( Force_WS );

	// Compute the torque and add it to our total...
	Torque_GS.Cross( *pApplicationPoint_GS, *pForce_GS );
	m_pMtx_GS->MulDir( Torque_WS, Torque_GS );
	m_TorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyForceToCM_MS( const CFVec3A *pForce_MS ) {
	CFVec3A Force_WS;

	m_Mtx.MulDir( Force_WS, *pForce_MS );
	m_ForceSum_WS.Add( Force_WS );
}


void CFMotion::ApplyForceToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pForce_GS ) {
	CFVec3A Force_WS;

	pMtx_GS->MulDir( Force_WS, *pForce_GS );
	m_ForceSum_WS.Add( Force_WS );
}


void CFMotion::ApplyTorqueToCM_MS( const CFVec3A *pTorque_MS ) {
	CFVec3A Torque_WS;

	m_Mtx.MulDir( Torque_WS, *pTorque_MS );
	m_TorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyTorqueToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pTorque_GS ) {
	CFVec3A Torque_WS;

	pMtx_GS->MulDir( Torque_WS, *pTorque_GS );
	m_TorqueSum_WS.Add( Torque_WS );
}


// ***********



void CFMotion::ApplyImpulse_WS( const CFVec3A *pApplicationPoint_WS, const CFVec3A *pForce_WS ) {
	CFVec3A CMToPointVec_WS, Torque_WS;

	// Add the force to our force total...
	m_ImpulseForceSum_WS.Add( *pForce_WS );

	// Compute the torque and add it to our total...
	CMToPointVec_WS.Sub( *pApplicationPoint_WS, m_Mtx.m_vPos );
	Torque_WS.Cross( CMToPointVec_WS, *pForce_WS );
	m_ImpulseTorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyImpulse_MS( const CFVec3A *pApplicationPoint_MS, const CFVec3A *pForce_MS ) {
	CFVec3A Torque_MS, Torque_WS, Force_WS;

	// Compute the force and add it to our total...
	m_Mtx.MulDir( Force_WS, *pForce_MS );
	m_ImpulseForceSum_WS.Add( Force_WS );

	// Compute the torque and add it to our total...
	Torque_MS.Cross( *pApplicationPoint_MS, *pForce_MS );
	m_Mtx.MulDir( Torque_WS, Torque_MS );
	m_ImpulseTorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyImpulse_GS( const CFMtx43A *m_pMtx_GS, const CFVec3A *pApplicationPoint_GS, const CFVec3A *pForce_GS ) {
	CFVec3A Torque_GS, Torque_WS, Force_WS;

	// Compute the force and add it to our total...
	m_pMtx_GS->MulDir( Force_WS, *pForce_GS );
	m_ImpulseForceSum_WS.Add( Force_WS );

	// Compute the torque and add it to our total...
	Torque_GS.Cross( *pApplicationPoint_GS, *pForce_GS );
	m_pMtx_GS->MulDir( Torque_WS, Torque_GS );
	m_ImpulseTorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyImpulseToCM_MS( const CFVec3A *pForce_MS ) {
	CFVec3A Force_WS;

	m_Mtx.MulDir( Force_WS, *pForce_MS );
	m_ImpulseForceSum_WS.Add( Force_WS );
}


void CFMotion::ApplyImpulseToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pForce_GS ) {
	CFVec3A Force_WS;

	pMtx_GS->MulDir( Force_WS, *pForce_GS );
	m_ImpulseForceSum_WS.Add( Force_WS );
}


void CFMotion::ApplyTorqueImpulseToCM_MS( const CFVec3A *pTorque_MS ) {
	CFVec3A Torque_WS;

	m_Mtx.MulDir( Torque_WS, *pTorque_MS );
	m_ImpulseTorqueSum_WS.Add( Torque_WS );
}


void CFMotion::ApplyTorqueImpulseToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pTorque_GS ) {
	CFVec3A Torque_WS;

	pMtx_GS->MulDir( Torque_WS, *pTorque_GS );
	m_ImpulseTorqueSum_WS.Add( Torque_WS );
}

// ***********

// Given a point in model space, this function computes the velocity vector of that point
// in world space.
void CFMotion::ComputeModelPointVelocity_WS( CFVec3A *pVelocity_WS, const CFVec3A *pPoint_MS ) {
	CFVec3A TempVec;

	m_Mtx.MulDir( TempVec, *pPoint_MS );
	pVelocity_WS->Cross( m_AngularVelocity_WS, TempVec );
	pVelocity_WS->Add( m_LinearVelocity_WS );
}


// Given a point in model space, this function computes the velocity vector of that point
// in model space.
void CFMotion::ComputeModelPointVelocity_MS( CFVec3A *pVelocity_MS, const CFVec3A *pPoint_MS ) {
	CFVec3A TempVec, Velocity_WS;

	m_Mtx.MulDir( TempVec, *pPoint_MS );
	Velocity_WS.Cross( m_AngularVelocity_WS, TempVec );
	Velocity_WS.Add( m_LinearVelocity_WS );
	m_TransposeMtx33.MulDir( *pVelocity_MS, Velocity_WS );
}


// Given a point in world space, this function computes the velocity vector of that point
// in world space.
void CFMotion::ComputeWorldPointVelocity_WS( CFVec3A *pVelocity_WS, const CFVec3A *pPoint_WS ) {
	CFVec3A TempVec;

	TempVec.Sub( *pPoint_WS, m_Mtx.m_vPos );
	pVelocity_WS->Cross( m_AngularVelocity_WS, TempVec );
	pVelocity_WS->Add( m_LinearVelocity_WS );
}


// Given a point in world space, this function computes the velocity vector of that point
// in model space.
void CFMotion::ComputeWorldPointVelocity_MS( CFVec3A *pVelocity_MS, const CFVec3A *pPoint_WS ) {
	CFVec3A Velocity_WS, TempVec;

	TempVec.Sub( *pPoint_WS, m_Mtx.m_vPos );
	Velocity_WS.Cross( m_AngularVelocity_WS, TempVec );
	Velocity_WS.Add( m_LinearVelocity_WS );
	m_TransposeMtx33.MulDir( *pVelocity_MS, Velocity_WS );
}


void CFMotion::ComputeWorldPointAcceleration_WS( CFVec3A *pAcceleration_WS, const CFVec3A *pPoint_WS ) {
	CFVec3A TempVec1, TempVec2, TempVec3, AngularAccel_WS, VecR;

	VecR.Sub( *pPoint_WS, m_Mtx.m_vPos );

	TempVec1.Cross( m_AngularMomentum_WS, m_AngularVelocity_WS ).Add( m_TorqueSum_WS );
	m_OOInertia_WS.MulDir( AngularAccel_WS, TempVec1 );

	TempVec1.Cross( AngularAccel_WS, VecR );
	TempVec2.Cross( m_AngularVelocity_WS, VecR );
	TempVec3.Cross( m_AngularVelocity_WS, TempVec2 );

	TempVec2.Mul( m_ForceSum_WS, m_fOOMass );
	TempVec2.y += m_fGravity;

	pAcceleration_WS->Add( TempVec1, TempVec2 ).Add( TempVec3 );
}


f32 CFMotion::ApplySolidWallImpulse( const CFVec3A *pImpactPoint_WS, const CFVec3A *pImpactWallNormal_WS, f32 fUnitLinRestitution, f32 fUnitRotRestitution ) {
	CFVec3A VecAtoP_WS, VelocityP_WS;//, VecAtoP_MS;
	CFVec3A TempVec1, TempVec2, TempVec3;
	CFVec3A LinearVelocity_MS;//, ImpactWallNormal_MS;
	f32 fNumerator, fDenominator, fImpulse;

	ComputeWorldPointVelocity_WS( &VelocityP_WS, pImpactPoint_WS );

	VecAtoP_WS.Sub( *pImpactPoint_WS, m_Mtx.m_vPos );

	// Numerator...
	fNumerator = VelocityP_WS.Dot( *pImpactWallNormal_WS );

	// Denominator...
	TempVec1.Cross( VecAtoP_WS, *pImpactWallNormal_WS );
	m_OOInertia_WS.MulDir( TempVec2, TempVec1 );
	TempVec3.Cross( TempVec2, VecAtoP_WS );
	fDenominator = m_fOOMass + TempVec3.Dot( *pImpactWallNormal_WS );

	// Compute impulse...
	fImpulse = fmath_Div( fNumerator, fDenominator );

	// Apply impulse to linear velocities...
	TempVec3.Mul( *pImpactWallNormal_WS, -(1.0f + fUnitLinRestitution) * fImpulse * m_fOOMass );
	m_LinearVelocity_WS.Add( TempVec3 );

	// Apply impulse to rotational velocities...
	TempVec1.Cross( VecAtoP_WS, *pImpactWallNormal_WS );
	TempVec1.Mul( -(1.0f + fUnitRotRestitution) * fImpulse );
	m_AngularMomentum_WS.Add( TempVec1 );
	m_OOInertia_WS.MulDir( m_AngularVelocity_WS, m_AngularMomentum_WS );

	return -fImpulse;
}


f32 CFMotion::ApplyImpulse( CFMotion *pMotionB, const CFVec3A *pImpactPoint_WS, const CFVec3A *pImpactWallNormal_WS, f32 fUnitRestitution ) {
	CFVec3A VecAtoP_WS, VecBtoP_WS, VelocityA_WS, VelocityB_WS, VelocityAToB_WS;
	CFVec3A TempVec1, TempVec2, TempVec3;
	CFVec3A LinearVelocity_MS;
	f32 fNumerator, fDenominator, fImpulse;

	ComputeWorldPointVelocity_WS( &VelocityA_WS, pImpactPoint_WS );
	pMotionB->ComputeWorldPointVelocity_WS( &VelocityB_WS, pImpactPoint_WS );
	VelocityAToB_WS.Sub( VelocityA_WS, VelocityB_WS ).Mul( -1.0f - fUnitRestitution );

	VecAtoP_WS.Sub( *pImpactPoint_WS, m_Mtx.m_vPos );
	VecBtoP_WS.Sub( *pImpactPoint_WS, pMotionB->m_Mtx.m_vPos );

	// Numerator...
	fNumerator = VelocityAToB_WS.Dot( *pImpactWallNormal_WS );

	// Denominator...
	TempVec1.Cross( VecAtoP_WS, *pImpactWallNormal_WS );
	m_OOInertia_WS.MulDir( TempVec2, TempVec1 );
	TempVec3.Cross( TempVec2, VecAtoP_WS );

	TempVec1.Cross( VecBtoP_WS, *pImpactWallNormal_WS );
	pMotionB->m_OOInertia_WS.MulDir( TempVec2, TempVec1 );
	TempVec1.Cross( TempVec2, VecBtoP_WS );

	TempVec1.Add( TempVec3 );

	fDenominator = m_fOOMass + pMotionB->m_fOOMass + TempVec1.Dot( *pImpactWallNormal_WS );

	// Compute impulse...
	fImpulse = fmath_Div( fNumerator, fDenominator );

	// Apply impulse to linear velocities...
	TempVec1.Mul( *pImpactWallNormal_WS, fImpulse * m_fOOMass );
	m_LinearVelocity_WS.Add( TempVec1 );

	TempVec1.Mul( *pImpactWallNormal_WS, -fImpulse * pMotionB->m_fOOMass );
	pMotionB->m_LinearVelocity_WS.Add( TempVec1 );

	// Apply impulse to rotational velocities...
	TempVec2.Cross( VecAtoP_WS, *pImpactWallNormal_WS );
	TempVec2.Mul( fImpulse );
	m_AngularMomentum_WS.Add( TempVec2 );
	m_OOInertia_WS.MulDir( m_AngularVelocity_WS, m_AngularMomentum_WS );

	TempVec2.Cross( VecBtoP_WS, *pImpactWallNormal_WS );
	TempVec2.Mul( -fImpulse );
	pMotionB->m_AngularMomentum_WS.Add( TempVec2 );
	pMotionB->m_OOInertia_WS.MulDir( pMotionB->m_AngularVelocity_WS, pMotionB->m_AngularMomentum_WS );

	return -fImpulse;
}


f32 CFMotion::ApplyImpulse( CFMotion *pMotionB, const CFVec3A *pImpactPointA_WS, const CFVec3A *pImpactPointB_WS, const CFVec3A *pImpactWallNormal_WS, f32 fUnitRestitution ) {
	CFVec3A VecAtoP_WS, VecBtoP_WS, VelocityA_WS, VelocityB_WS, VelocityAToB_WS;
	CFVec3A TempVec1, TempVec2, TempVec3;
	CFVec3A LinearVelocity_MS;
	f32 fNumerator, fDenominator, fImpulse;

	ComputeWorldPointVelocity_WS( &VelocityA_WS, pImpactPointA_WS );
	pMotionB->ComputeWorldPointVelocity_WS( &VelocityB_WS, pImpactPointB_WS );
	VelocityAToB_WS.Sub( VelocityA_WS, VelocityB_WS ).Mul( -1.0f - fUnitRestitution );

	VecAtoP_WS.Sub( *pImpactPointA_WS, m_Mtx.m_vPos );
	VecBtoP_WS.Sub( *pImpactPointB_WS, pMotionB->m_Mtx.m_vPos );

	// Numerator...
	fNumerator = VelocityAToB_WS.Dot( *pImpactWallNormal_WS );

	// Denominator...
	TempVec1.Cross( VecAtoP_WS, *pImpactWallNormal_WS );
	m_OOInertia_WS.MulDir( TempVec2, TempVec1 );
	TempVec3.Cross( TempVec2, VecAtoP_WS );

	TempVec1.Cross( VecBtoP_WS, *pImpactWallNormal_WS );
	pMotionB->m_OOInertia_WS.MulDir( TempVec2, TempVec1 );
	TempVec1.Cross( TempVec2, VecBtoP_WS );

	TempVec1.Add( TempVec3 );

	fDenominator = m_fOOMass + pMotionB->m_fOOMass + TempVec1.Dot( *pImpactWallNormal_WS );

	// Compute impulse...
	fImpulse = fmath_Div( fNumerator, fDenominator );

	// Apply impulse to linear velocities...
	TempVec1.Mul( *pImpactWallNormal_WS, fImpulse * m_fOOMass );
	m_LinearVelocity_WS.Add( TempVec1 );

	TempVec1.Mul( *pImpactWallNormal_WS, -fImpulse * pMotionB->m_fOOMass );
	pMotionB->m_LinearVelocity_WS.Add( TempVec1 );

	// Apply impulse to rotational velocities...
	TempVec2.Cross( VecAtoP_WS, *pImpactWallNormal_WS );
	TempVec2.Mul( fImpulse );
	m_AngularMomentum_WS.Add( TempVec2 );
	m_OOInertia_WS.MulDir( m_AngularVelocity_WS, m_AngularMomentum_WS );

	TempVec2.Cross( VecBtoP_WS, *pImpactWallNormal_WS );
	TempVec2.Mul( -fImpulse );
	pMotionB->m_AngularMomentum_WS.Add( TempVec2 );
	pMotionB->m_OOInertia_WS.MulDir( pMotionB->m_AngularVelocity_WS, pMotionB->m_AngularMomentum_WS );

	return -fImpulse;
}


