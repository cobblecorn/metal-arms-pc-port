//////////////////////////////////////////////////////////////////////////////////////
// fphysics.h -
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
#ifndef _PHYSICS_H_
#define _PHYSICS_H_ 1

#include "fang.h"
#include "fmath.h"
#include "flinklist.h"

FCLASS_ALIGN_PREFIX class CFPhysicsObject
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public defines:
	//----------------------------------------------------------------------------------------------------------------------------------
public:

#define FPHYSICS_FLAG_CONTACTING		0x00000001	// TRUE if object currently has at least one contact collision
#define FPHYSICS_FLAG_COLLIDING			0x00000002	// TRUE if object currently has at least one colliding collision
#define FPHYSICS_FLAG_MOM_CLAMP			0x00000004	// TRUE if smoothed clamping should be performed on momentum
#define FPHYSICS_FLAG_ANG_MOM_CLAMP		0x00000010	// TRUE if smoothed clamping should be performed on angular momentum
#define FPHYSICS_FLAG_FORCE_APPLIED		0x00000020	// TRUE if client called ApplyForce() this frame
#define FPHYSICS_FLAG_DYN_LIN_DAMP		0x00000040	// TRUE if dynamic angular damping should be performed
#define FPHYSICS_FLAG_DYN_ANG_DAMP		0x00000080	// TRUE if dynamic angular damping should be performed
#define FPHYSICS_FLAG_IGNORE_FRICT_POS	0x00000100	// TRUE if position value of collision point friction matrix pmFrictOr should be ignored


	typedef struct
	{
		CFMtx43A mInverseIT;				// world space inverse inertia tensor
		CFMtx43A mOrientation;				// orientation of object
		CFVec3A vPosition;					// position of object
		CFVec3A vMomentum;					// linear momentum of object
		CFVec3A vVelocity;					// linear velocity of object (auxiliary deriveduantity)
		CFVec3A vAcceleration;				// linear acceleration of object (auxiliary derived quantity)
		CFVec3A vAngularMomentum;			// angular momentum of object
		CFVec3A vAngularVelocity;			// angular velocity of object (auxiliary deriveduantity)
		CFVec3A vAngularAcceleration;		// angular acceleration of object (auxiliary deriveduantity)
		CFVec3A vTotalForce;				// total force applied to center-of-mass this frame
		CFVec3A vTotalTorque;				// total torque applied this frame
		CFVec3A vTotalFriction;				// total friction force applied to center-of-mass this frame
		CFVec3A vTotalFrictionTorque;		// total frictional torque applied this frame
	} PhysicsState_t;

	typedef struct
	{
		f32 fRestLength;			// length of spring at rest.
		f32 fCompressedLength;		// minimum length to which spring can be compressed.
		f32 fStretchedLength;		// maximum length to which spring can be stretched.
		f32 fStiffness;				// spring coefficient
		f32 fDamping;				// damping amount
		f32 fDampingRatio;			// ratio of compression damping to stretch damping
		f32 fCurrentLength;			// current length of spring
		f32 fVelocity;				// current velocity of spring
		f32 fForce;					// current force on spring
		f32 fAppliedForceRatio;		// amount of force present at collision point which should be applied to spring
	} Spring_t;
	// Attn Programmers: when adding fields to this struct, add an initializer to InitCollisionPoint().

	typedef struct
	{
		CFVec3A vMomentum;			// momentum caused by collision
		CFVec3A vForce;				// force applied by client
		u32 uFlags;					// flags set in collision
	} CollisionResult_t;
	// Attn Programmers: when adding fields to this struct, add an initializer to InitCollisionPoint().

	// structure containing info about a collision
	typedef struct
	{
		CFMtx43A *pmFrictOr;		// client-supplied friction orientation matrix.  m_vPos is used to offset application point of friction from the collision point.
		CFVec3A vStaticFriction;	// client-supplied friction force at point of collision in mFrictOr space
		CFVec3A vKineticFriction;	// client-supplied friction force at point of collision in mFrictOr space
		CFVec3A vPoint;				// client-supplied physics-object-space point of collision.
		CFVec3A vNormal;			// client-supplied unit normal of collision surface.
		CFVec3A vAppliedForce;		// client-supplied force desired to be applied at this point
		CFVec3A	vPush;				// client-supplied unit direction to push point out of collision.
		CFVec3A	vCollideVel;		// client-supplied additional velocity at collision point. (CLIENT_FLAG_ADD_COLLIDE_VELOCITY must be set.)
		CFPhysicsObject *pOtherObj;	// pointer to other physics object if colliding with such.
		f32 fTimeSinceCollision;	// client-supplied time in seconds since last collision
		f32	fDepth;					// client-supplied depth of collision
		f32 fKineticTransition;		// Client-supplied value of acceleration at which transition from static to kinetic friction occurs.
		f32 fStaticHysteresis;		// Client-supplied fraction of fKineticTransition at which transition back to static friction will occur.
		f32 fAppliedForceKineticMul;// client-supplied fraction of vAppliedForce to apply when point is undergoing kinetic friction
		f32 fFrictionSmoothTime;	// client-supplied time since last collision less than which friction will still be applied
		u32 uClientFlags;			// client-supplied flags
		u32 uResultFlags;			// flags set by physics system
		u32 uLastResultFlags;		// flags that were valid at time of last collision
		CollisionResult_t Result;	// results of collision, filled in by physics system
		Spring_t Spring;			// shock absorber
	} CollisionPoint_t;
	// Attn Programmers: when adding fields to this struct, add an initializer to InitCollisionPoint().

	// CollDetectCallback
	// A client-provided callback to tell physics object about collisions occuring at collision points
	// during a simulation step of the physics object. Client must register list of collision points 
	// before using collision callback.  Inside the callback, the client should fill each collision 
	// point data structure with appropriate values for the current time.  Note that collision point 
	// positions should be enclosed within the box defined by the inertial size of the object.
	typedef void (*CollDetectCallback) ( const PhysicsState_t *pState );

	// callback for ODE solver
	typedef void (*DerivativeCallback) ( f32 fDeltaTime, f32 *pafInput, f32 *pafDerivative );

	// uResultFlags. Status flags set by the physics object to provide information to the client about events
	// occurring at a collision point.
	enum
	{
		COLLISION_RESULT_NONE					= 0x00000000,
		COLLISION_RESULT_CONTACT				= 0x00000001,	// point experienced resting contact with a surface
		COLLISION_RESULT_COLLIDE				= 0x00000002,	// point experienced elastic collision
		COLLISION_RESULT_KINETIC				= 0x00000004,	// point experienced kinetic friction (sliding)
		COLLISION_RESULT_MAX_SPRING_COMPRESS	= 0x00000010,	// suspension "bottomed out"
	};

	// UClientFlags. Status flags set by the client to activate features of the physics object, or inform that a collision occurred.
	enum
	{
		CLIENT_FLAG_COLLIDED					= 0x00000001,	// collision has occurred at collision point
		CLIENT_FLAG_APPLY_FRICTION				= 0x00000002,	// apply friction forces at collision point
		CLIENT_FLAG_KINETIC_FRICTION			= 0x00000004,	// use kinetic friction at collision point regardless of current friction state
		CLIENT_FLAG_STATIC_FRICTION				= 0x00000008,	// use static friction at collision point regardless of current friction state
		CLIENT_FLAG_SPRING_ACTIVE				= 0x00000010,	// activate spring at collision point
		CLIENT_FLAG_ADD_COLLIDE_VELOCITY		= 0x00000020,	// include the client-supplied collision velocity in collision calculations
	};


	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Data:
	//----------------------------------------------------------------------------------------------------------------------------------
public:

	//----------------------------------------------------------------------------------------------------------------------------------
	// Protected Data:
	//----------------------------------------------------------------------------------------------------------------------------------
protected:
	static CFPhysicsObject *m_pPhysObj;	// pointer to current physics object for static callback functions

	CFVec3A m_vCMOffset;			// Local space offset of the center of mass from the object centroid.
	f32 m_fMass;					// mass of object
	f32 m_fInverseMass;				// inverse mass of object
	f32 m_fElasticity;				// elasticity of object (0.0 to 1.0)
	f32 m_fOtherObjElasticity;		// elasticity when colliding with "pOtherObj" objects

	PhysicsState_t m_State;			// state variable set
	CFMtx43A m_mLocalIT;			// local space inertia tensor
	CFMtx43A m_mLocalInverseIT;		// local space inverse inertia tensor
	CFVec3A m_vInertialSize;		// axial size of object
	cchar *m_pszName;				// pointer to object name string
	BOOL m_bVisible;				// true if debug drawing of inertial extents should be performed
	f32 m_fGravity;					// gravitational acceleration constant.  "normal" gravity will be a negative number
	f32 m_fLinearDamping;			// overall linear damping in system
	f32 m_fAngularDamping;			// overall angular damping in system

	CollDetectCallback m_pCollisionCB;	// pointer to collision detection callback for this object
	CollisionPoint_t *m_paColPoints;// pointer to client-supplied array of collision points
	s32 m_nColPoints;				// number of elements in m_paColPoints
	f32 m_fMinMomClamping;			// minimum velocity for smoothed clamping
	f32 m_fMaxMomClamping;			// maximum velocity for smoothed clamping
	f32 m_fMinAngMomClamping;		// minimum angular velocity for smoothed clamping
	f32 m_fMaxAngMomClamping;		// maximum angular velocity for smoothed clamping
	f32 m_fClampCoeff;				// current clamping value

	f32 m_fDynDampVel;				// max velocity at which dynamic damping is applied
	f32 m_fDynDampAng;				// max angular velocity at which dynamic damping is applied
	f32 m_fDynDampForce;			// max applied force at which dynamic damping is applied
	f32 m_fDynDampOOVel;			// "one over" versions of above values
	f32 m_fDynDampOOAng;
	f32 m_fDynDampOOForce;
	f32 m_fMinLinDamping;			// minimum value of dynamic linear damping
	f32 m_fMaxLinDamping;			// maximum value of dynamic linear damping
	f32 m_fMinAngDamping;			// minimum value of dynamic angular damping
	f32 m_fMaxAngDamping;			// maximum value of dynamic angular damping

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	u32 m_uFlags;					// various state flags, see FPHYSICS_FLAG_* above

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CFPhysicsObject();
	~CFPhysicsObject();
	BOOL Init( void );
	static BOOL InitLevel( void );
	static void UninitLevel( void );

	//-------------------------------------------------------------------------
	// simulation
	//-------------------------------------------------------------------------

	// runs the physics simulation on the object for the specified time period.
	void Simulate( f32 fDeltaTime );

	// sets the collision detection callback.  See definition of CollDetectCallback above.
	void SetCollisionDetectionCallback( CollDetectCallback pCB );

	// call this function once to provide the object a list of collision points.
	void RegisterCollisionPoints( CollisionPoint_t *paPoints, u32 uPoints );

	// this function should be called once on any CollisionPoint_t struct before
	// it is passed in to a physics object for the first time.
	static void InitCollisionPoint( CollisionPoint_t *pPoint );

	// returns TRUE if any collision point is in contact with a surface.
	BOOL IsContacting( void ) const { return( m_uFlags & FPHYSICS_FLAG_CONTACTING ); };

	// returns TRUE if any collision point is colliding with a surface.
	BOOL IsColliding( void ) const { return( m_uFlags & FPHYSICS_FLAG_COLLIDING ); };

	// enables and sets range for smoothly clamping low momentum values down to zero
	void SetMomentumClamping( BOOL bEnable, f32 fMinMom = 0.0f, f32 fMaxMom = 0.0f );

	// enables and sets range for smoothly clamping low angular momentum values down to zero
	void SetAngularMomentumClamping( BOOL bEnable, f32 fMinMom = 0.0f, f32 fMaxMom = 0.0f );

	// returns the current clamping coefficient value being used by the object (0.0f = fully clamped).
	f32 GetClampingCoefficient( void ) const { return m_fClampCoeff; };

	// sets the parameters that control the amount of dynamic damping applied.  Parameters are for 60Hz
	// frame rate.  values are proportionally lowered as framerate decreases.
	void SetDynamicDampingParams( f32 fVelocity, f32 fAngular, f32 fForce );

	// enables and sets range of dynamic linear damping. If disabled, normal SetLinearDamping() value is used.
	// (when object has lowest values of damping parameters, highest damping is achieved)
	void SetDynamicLinearDamping( BOOL bEnable, f32 fMinDamping = 0.0f, f32 fMaxDamping = 0.0f );

	// enables and sets range of dynamic angular damping.  . If disabled, normal SetAngularDamping() value is used.
	// (when object has lowest values of damping parameters, highest damping is achieved)
	void SetDynamicAngularDamping( BOOL bEnable, f32 fMinDamping = 0.0f, f32 fMaxDamping = 0.0f );

	// causes position value of collision point friction matrix pmFrictOr to be ignored
	FINLINE void IgnoreFrictionPosition( BOOL bIgnore ) { if( bIgnore ) m_uFlags |= FPHYSICS_FLAG_IGNORE_FRICT_POS; else m_uFlags &= ~FPHYSICS_FLAG_IGNORE_FRICT_POS; };

	//-------------------------------------------------------------------------
	// mass
	//-------------------------------------------------------------------------

	// returns mass of object.
	f32 GetMass( void ) const { return m_fMass; };

	// sets mass of object
	void SetMass( f32 fMass );

	// returns one over mass of object.
	f32 GetInverseMass( void ) const { return m_fInverseMass; };

	// Sets the inertial size of the object (as a rectangular box).
	// Values are "half size", i.e. length from center-of-mass to one
	// wall of the enclosing box.
	// Box should enclose entire space where collision points
	// could possibly occur.  Collision points which are outside the
	// enclosing box should not be passed in to the object.
	void SetInertialSize( f32 fX, f32 fY, f32 fZ );

	const CFVec3A* GetInertialSize( void ) const { return &m_vInertialSize; }

	//-------------------------------------------------------------------------
	// linear
	//-------------------------------------------------------------------------

	// returns current position of object center-of-mass.
	const CFVec3A *GetPosition( void ) const { return &m_State.vPosition; };

	// Returns total velocity of object at center-of-mass.
	const CFVec3A *GetVelocity( void ) const;
	// fills pvVel with velocity of object at local-space point pvPoint.
	void GetVelocity( const CFVec3A *pvPoint, CFVec3A *pvVel ) const;

	// returns pointer to the object's angular momentum vector.
	const CFVec3A *GetMomentum( void ) const { return &m_State.vMomentum; };

	// sets the object's angular momentum vector.
	void SetMomentum( const CFVec3A *pvMomentum ) { m_State.vMomentum.Set( *pvMomentum ); };

	// sets position of object center-of-mass
	void SetPosition( const CFVec3A *pvPos ) { m_State.vPosition.Set( *pvPos ); };
	void SetPosition( f32 fX, f32 fY, f32 fZ ) { m_State.vPosition.Set( fX, fY, fZ ); };

	// sets overall linear damping of object
	void SetLinearDamping( f32 fDamping ) { m_fLinearDamping = fDamping; };
	f32 GetLinearDamping( void ) { return m_fLinearDamping; };

	//-------------------------------------------------------------------------
	// angular
	//-------------------------------------------------------------------------

	// returns pointer to the object's orientation matrix.
	const CFMtx43A *GetOrientation( void ) const { return &m_State.mOrientation; };

	// returns pointer to the object's angular momentum vector.
	const CFVec3A *GetAngularMomentum( void ) const { return &m_State.vAngularMomentum; };

	// returns pointer to the object's angular velocity vector.
	const CFVec3A *GetAngularVelocity( void ) const { return &m_State.vAngularVelocity; };

	// rotates the object's current orientation via three euler angles (radians).
	void Rotate( f32 fXRot, f32 fYRot, f32 fZRot );

	// allows direct setting of the object's orientation matrix.
	void SetOrientation( const CFMtx43A *pmMatrix );

	// sets the object's angular momentum vector.
	void SetAngularMomentum( const CFVec3A *pvMomentum ) { m_State.vAngularMomentum.Set( *pvMomentum ); };

	// sets overall angular damping of object
	void SetAngularDamping( f32 fDamping ) { m_fAngularDamping = fDamping; };
	f32 GetAngularDamping( void ) const { return m_fAngularDamping; };

	//-------------------------------------------------------------------------
	// applied forces
	//-------------------------------------------------------------------------

	// returns the total force accumulated on the object since the last simulation step.
	const CFVec3A *GetTotalForce( void ) const { return &m_State.vTotalForce; };

	// applies a momentum to the object.
	void ApplyMomentum( const CFVec3A *pvMomentum, const CFVec3A *pvAppPoint = NULL );

	// applies an angular momentum to the object.
	void ApplyAngularMomentum( const CFVec3A *pvMomentum );

	// applies a force to the object.  pvAppPoint is in object space.
	void ApplyForce( const CFVec3A *pvForce, const CFVec3A *pvAppPoint = NULL );

	void ApplyFriction( const CFVec3A *pvForce, const CFVec3A *pvAppPoint = NULL );

	// applies a torque to an object
	void ApplyTorque( const CFVec3A *pvTorque );

	// clears any accumulated forces.
	void ClearForces( PhysicsState_t &rState );

	//-------------------------------------------------------------------------
	// elasticity
	//-------------------------------------------------------------------------

	// returns the object's elasticity.
	f32 GetElasticity( void ) const { return m_fElasticity; };

	// sets the object's elasticity.  Valid range is 0.0 to 1.0
	void SetElasticity( f32 fElasticity );

	// returns the object's elasticity for collisions with pOtherObj objects.
	f32 GetOtherObjElasticity( void ) const { return m_fOtherObjElasticity; };

	// sets the object's elasticity for collisions with pOtherObj objects.  Valid range is 0.0 to 1.0
	void SetOtherObjElasticity( f32 fElasticity );

	//-------------------------------------------------------------------------
	// gravity
	//-------------------------------------------------------------------------

	// sets the object's gravitational acceleration. "normal" gravity will be a negative number.
	void SetGravity( f32 fGravity ) { m_fGravity = fGravity; }

	// returns the object's gravitational acceleration.
	f32 GetGravity( void ) const { return m_fGravity; }

	//-------------------------------------------------------------------------
	// name
	//-------------------------------------------------------------------------

	// get and set the object name. client provides storage for name string.
	cchar* GetName( void ) const { return m_pszName; };
	void SetName( cchar *pszName ) { m_pszName = pszName; };

	//-------------------------------------------------------------------------
	// debug drawing
	//-------------------------------------------------------------------------

	// does the frame-by-frame work of drawing any debug graphics.
	static void DebugDrawWork( void );

	// Draws a line. start and end points are in world space. Color is r,g,b, values from 0.0 to 1.0.
	static void DrawLine( CFVec3A *pStart, CFVec3A *pEnd, CFVec3A *pColor = NULL );

	// Draws a sphere.  Position is in world space.
	static void DrawSphere( CFVec3A *pPos, f32 fRadius, CFVec3A *pColor = NULL );

	// draws object axis and inertial bounds;
	void DrawObject( void );

	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _SetDefaults( void );
	void _ComputeLocalInertiaTensor();
	void _ApplyGravity( void );
	void _ComputeWorldInertiaTensor( PhysicsState_t &rState );
	f32 _ComputeBoxPMIAxis( f32 fMass, s32 nAxis, const CFVec3A *pvHalfSize );
	void _DampVelocities( PhysicsState_t &rState );
	f32 _SmoothBipolarClampMin( f32 fInputVal, f32 fMinRange, f32 fMaxRange );
	f32 _SmoothVectorClampMinCoeff( CFVec3A &rvInput, f32 fMinRange, f32 fMaxRange );
	void _SmoothVectorClampMin( CFVec3A &rvInput, f32 fMinRange, f32 fMaxRange );

	void _StateToArray( const PhysicsState_t &rState, f32 *pfArray );
	void _Collide( CollisionPoint_t &rPoint, PhysicsState_t &rState );
	void _ApplyCollisionResult( CollisionPoint_t &rPoint, PhysicsState_t &rState );
	void _UpdateSpring( CollisionPoint_t &rPoint, PhysicsState_t &rState, f32 fDeltaTime );
	void _ODE_Euler( f32 fDeltaTime, f32 *pafInput, f32 *pafOutput, f32 *pafWork, s32 nSize, DerivativeCallback Callback );
	void _ODE_Midpoint( f32 fDeltaTime, f32 *pafInput, f32 *pafOutput, f32 *pafWork, s32 nSize, DerivativeCallback Callback );
	static void _ArrayToState( PhysicsState_t &rState, f32 *pfArray );
	static void _ComputeForceAndTorque( f32 fTime, PhysicsState_t &rState );
	static void _StateToArrayDerivative( PhysicsState_t &rState, f32 *pfArrayDeriv );
	static void _StateArrayDerivative( f32 fTime, f32 *pfArray, f32 *pfArrayDeriv );
	static void _Friction( CollisionPoint_t &rPoint, PhysicsState_t &rState, CFVec3A *pForce );
	static void _ClientForce( CollisionPoint_t &rPoint, CFVec3A *pvForce );

	FCLASS_STACKMEM_ALIGN( CFPhysicsObject );
} FCLASS_ALIGN_SUFFIX;

#endif // _PHYSICS_H_
