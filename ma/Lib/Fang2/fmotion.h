//////////////////////////////////////////////////////////////////////////////////////
// motion.h - Physical motion class.
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

#ifndef _MOTION_H_
#define _MOTION_H_ 1

#include "fang.h"
#include "fmath.h"
#include "floop.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFMotionSimple
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFMotionSimple {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	f32 m_fAngularSpeed;						// Angular speed

	CFVec3A m_RotUnitAxis;						// Unit axis of angular rotation
	CFQuatA m_OrientQuat;						// Orientation quaternion

	CFVec3A m_Vel;								// Linear velocity
	CFVec3A m_Pos;								// Position




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	void Simulate( f32 fDeltaTime = FLoop_fPreviousLoopSecs );


	FCLASS_STACKMEM_ALIGN( CFMotionSimple );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFMotion
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFMotion {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	f32 m_fMass;					// Body mass
	f32 m_fOOMass;					// Inverse body mass
	f32 m_fGravity;					// Gravity (0=none)
	f32 m_fOOUniformTensorMag;		// For bodies whose mass is equally distributed about its center of mass, this is the inverse inertia tensor

	CFMtx43A m_InertiaTensor_MS;	// Moment of inertia in model space (Icm bar)
	CFMtx43A m_OOInertiaTensor_MS;	// Inverse of m_InertiaTensor_MS (Icm^-1 bar)

	CFVec3A m_ForceSum_WS;			// Total accumulated force on the center of mass (F)
	CFVec3A m_TorqueSum_WS;			// Total accumulated torque about the center of mass (little tau)
	CFVec3A m_ImpulseForceSum_WS;	// Total accumulated impulse force on the center of mass (F)
	CFVec3A m_ImpulseTorqueSum_WS;	// Total accumulated impulse torque about the center of mass (little tau)

	CFVec3A m_LinearVelocity_WS;	// Linear velocity of the center of mass (vcm)
	CFVec3A m_AngularVelocity_WS;	// Angular velocity (little omega)
	CFVec3A m_AngularMomentum_WS;	// Angular momentum (Lcm)
	CFMtx43A m_OOInertia_WS;		// Moment of inertia in world space (Icm^-1)

	CFQuatA m_Quat;					// Rotation quaternion
	CFMtx43A m_Mtx;					// Orientation matrix
	CFMtx43A m_TransposeMtx33;		// Transpose of m_Mtx (m_vPos is always zero)




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	FINLINE CFMotion() {}


	// The simple functions operate on bodies whose mass is uniformly distributed about
	// the center of mass:
	void Simple_InitBody( const CFMtx43A *pInitialMtx, f32 fOOUniformTensorMag, f32 fMass, f32 fGravity );
	void Simple_Simulate( f32 fDeltaTime = FLoop_fPreviousLoopSecs );
	void Simple_UpdateOrientation( const CFMtx43A *pNewOrientation );

	static f32 ComputeInvUniformInertiaTensorMag( f32 fUniformBodyDim, f32 fMass );


	// For complex bodies:
	void Complex_InitBody( const CFMtx43A *pInitialMtx, const CFMtx43A *pOOInertiaTensor_MS, f32 fMass, f32 fGravity );
	void Complex_UpdateOrientation( const CFMtx43A *pNewOrientation );
	void Complex_Simulate( f32 fDeltaTime = FLoop_fPreviousLoopSecs, BOOL bApplyTransVel = TRUE, BOOL bApplyRotVel = TRUE );

	static void ComputeInvInertiaTensor_MS( CFMtx43A *pInertiaTensor, const CFVec3A *pBoxMin, const CFVec3A *pBoxMax, f32 fMassAtMinZ, f32 fMassAtMaxZ );
	static void ComputeInvInertiaTensor_MS( CFMtx43A *pInertiaTensor, const CFVec3A *pBoxMin, const CFVec3A *pBoxMax, f32 fMass );


	// For both simple and comples bodies:
	void StopMotion( void );


	// Forces:
	void ApplyForce_WS( const CFVec3A *pApplicationPoint_WS, const CFVec3A *pForce_WS );
	void ApplyForce_MS( const CFVec3A *pApplicationPoint_MS, const CFVec3A *pForce_MS );
	void ApplyForce_GS( const CFMtx43A *m_pMtx_GS, const CFVec3A *pApplicationPoint_GS, const CFVec3A *pForce_GS );

	FINLINE void ApplyForceToCM_WS( const CFVec3A *pForce_WS ) { m_ForceSum_WS.Add( *pForce_WS ); }
	void ApplyForceToCM_MS( const CFVec3A *pForce_MS );
	void ApplyForceToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pForce_GS );

	FINLINE void ApplyTorque_WS( const CFVec3A *pTorque_WS ) { m_TorqueSum_WS.Add( *pTorque_WS ); }
	void ApplyTorqueToCM_MS( const CFVec3A *pTorque_MS );
	void ApplyTorqueToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pTorque_GS );


	// Impulses:
	void ApplyImpulse_WS( const CFVec3A *pApplicationPoint_WS, const CFVec3A *pForce_WS );
	void ApplyImpulse_MS( const CFVec3A *pApplicationPoint_MS, const CFVec3A *pForce_MS );
	void ApplyImpulse_GS( const CFMtx43A *m_pMtx_GS, const CFVec3A *pApplicationPoint_GS, const CFVec3A *pForce_GS );

	FINLINE void ApplyImpulseToCM_WS( const CFVec3A *pForce_WS ) { m_ForceSum_WS.Add( *pForce_WS ); }
	void ApplyImpulseToCM_MS( const CFVec3A *pForce_MS );
	void ApplyImpulseToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pForce_GS );

	FINLINE void ApplyTorqueImpulse_WS( const CFVec3A *pTorque_WS ) { m_TorqueSum_WS.Add( *pTorque_WS ); }
	void ApplyTorqueImpulseToCM_MS( const CFVec3A *pTorque_MS );
	void ApplyTorqueImpulseToCM_GS( const CFMtx43A *pMtx_GS, const CFVec3A *pTorque_GS );


	// Velocity computation:
	void ComputeModelPointVelocity_WS( CFVec3A *pVelocity_WS, const CFVec3A *pPoint_MS );
	void ComputeModelPointVelocity_MS( CFVec3A *pVelocity_MS, const CFVec3A *pPoint_MS );
	void ComputeWorldPointVelocity_WS( CFVec3A *pVelocity_WS, const CFVec3A *pPoint_WS );
	void ComputeWorldPointVelocity_MS( CFVec3A *pVelocity_MS, const CFVec3A *pPoint_WS );

	void ComputeWorldPointAcceleration_WS( CFVec3A *pAcceleration_WS, const CFVec3A *pPoint_WS );


	// Collision:
	f32 ApplySolidWallImpulse( const CFVec3A *pImpactPoint_WS, const CFVec3A *pImpactWallNormal_WS, f32 fUnitLinRestitution, f32 fUnitRotRestitution );
	f32 ApplyImpulse( CFMotion *pMotionB, const CFVec3A *pImpactPoint_WS, const CFVec3A *pImpactWallNormal_WS, f32 fUnitRestitution );
	f32 ApplyImpulse( CFMotion *pMotionB, const CFVec3A *pImpactPointA_WS, const CFVec3A *pImpactPointB_WS, const CFVec3A *pImpactWallNormal_WS, f32 fUnitRestitution );





//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:


	FCLASS_STACKMEM_ALIGN( CFMotion );
} FCLASS_ALIGN_SUFFIX;





#endif

