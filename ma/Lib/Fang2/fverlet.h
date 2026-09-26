//////////////////////////////////////////////////////////////////////////////////////
// fverlet.h - Verlet physics module.
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
// 11/21/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FVERLET_H_
#define _FVERLET_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fworld_coll.h"
#include "fexplosion.h"
#include "FCheckPoint.h"

#define FVERLET_MESH_BONE_TACK_PREFIX		"Tack_"
#define FVERLET_MESH_BONE_CTACK_PREFIX		"CTack_"
#define FVERLET_DUMMY_TACK_NAME				"<DummyTack>"


struct FMesh_t;
struct FMeshBone_t;
class CFVerlet;
class CFVerletTack;
class CFWorldMesh;
class CFWorldTracker;
class CFMeshInst;
class CFWire;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFVerletTackSurface - Used to perform cheap collision against tacks.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFVerletTackSurface {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		TYPE_BOX,					// Box (m_UnitMtx.m_vPos is in the center of the box)
		TYPE_PLANE,					// Infinite plane (m_UnitMtx.m_vPos lies on the plane)

		TYPE_COUNT
	} Type_e;


	typedef enum {
		ANCHOR_TYPE_STATIC,
		ANCHOR_TYPE_DYNAMIC,
		ANCHOR_TYPE_BONED,

		ANCHOR_TYPE_NONE
	} AnchorType_e;


	typedef enum {
		PLANE_NORMAL_X,
		PLANE_NORMAL_Y,
		PLANE_NORMAL_Z,
		PLANE_NORMAL_NX,
		PLANE_NORMAL_NY,
		PLANE_NORMAL_NZ,

		PLANE_NORMAL_COUNT
	} PlaneNormal_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		FLAG_ENABLED					= 0x01,		// Enable collision with this surface
		FLAG_PLANE_NEGATE_NORMAL		= 0x02,		// Negate the normal vector
		FLAG_MUST_UNITIZE_MTX			= 0x04,		// TRUE if we need to unitize m_pNonUnitMtx into m_UnitMtx
		FLAG_PRESERVE_VELOCITY			= 0x08,		// Sets the bPreserveVelocity flag when calling _ApplyPlaneConstraint() so that
													//  the velocity is not modified after the collision adjustment is made.

		FLAG_NONE						= 0x00
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	f32 m_fFriction;					// Friction to apply to tacks colliding with this surface
	f32 m_fPlaneDist;					// Minimum allowed distance to the plane

	u8 m_nType;							// Type of surface
	u8 m_nAnchorType;					// Type of anchor
	u8 m_nPlaneAxisIndex;				// Indexes into *m_pUnitMtx such that 0=X axis, 1=Y axis, 2=Z axis
	u8 m_nFlags;						// See FLAG_* for info

	u32 m_nUnitMtxComputedFrame;		// Snapshot of FVid_nFrameCounter of when m_UnitMtx was last computed for this surface

	const CFMtx43A *m_pUnitMtx;			// Points to the unit orientation matrix
	const CFMtx43A *m_pNonUnitMtx;		// Points to the non-unit orientation matrix
	CFMtx43A m_Mtx_BS;					// Orientation matrix relative to *m_pNonUnitMtx (might not be unit)
	CFMtx43A m_UnitMtx;					// Unit orientation matrix

	CFVec3A m_BoxHalfDim_MS;			// Box half dimensions in model space (x, y and z)
	CFVec3A m_BoxMinCorner_WS;			// (-m_BoxHalfDim_MS.x, -m_BoxHalfDim_MS.y, -m_BoxHalfDim_MS.z) in world space
	CFVec3A m_BoxMaxCorner_WS;			// ( m_BoxHalfDim_MS.x,  m_BoxHalfDim_MS.y,  m_BoxHalfDim_MS.z) in world space




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CFVerletTackSurface() { m_nType = TYPE_COUNT; }
	FINLINE BOOL IsCreated( void ) const { return (m_nType != TYPE_COUNT); }

	void Init_Plane_Static( const CFVec3A *pPointOnPlane_WS, const CFVec3A *pUnitNormal_WS );
	void Init_Plane_Dynamic( const CFMtx43A *pMtx, BOOL bMtxMayBeNonUnit, PlaneNormal_e nPlaneNormal=PLANE_NORMAL_Z );
	void Init_Plane_Boned( const CFWorldMesh *pWorldMesh, u32 nBoneIndex, PlaneNormal_e nPlaneNormal );
	void Init_Plane_Boned_WS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_WS, PlaneNormal_e nPlaneNormal=PLANE_NORMAL_Z );
	void Init_Plane_Boned_BS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_BS, PlaneNormal_e nPlaneNormal=PLANE_NORMAL_Z );

	void Init_Box_Static( const CFMtx43A *pUnitMtx, const CFVec3A *pHalfDim_MS );
	void Init_Box_Dynamic( const CFMtx43A *pMtx, const CFVec3A *pHalfDim_MS, BOOL bMtxMayBeNonUnit );
	void Init_Box_Boned( CFWorldMesh *pWorldMesh, u32 nBoneIndex, const CFVec3A *pHalfDim_MS );
	void Init_Box_Boned_WS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_WS, const CFVec3A *pHalfDim_MS );
	void Init_Box_Boned_BS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_BS, const CFVec3A *pHalfDim_MS );

	void Enable( BOOL bEnable );
	FINLINE BOOL IsEnabled( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_ENABLED; }

	FINLINE Type_e GetType( void ) const { FASSERT( IsCreated() ); return (Type_e)m_nType; }
	FINLINE AnchorType_e GetAnchorType( void ) const { FASSERT( IsCreated() ); return (AnchorType_e)m_nAnchorType; }

	FINLINE void SetFriction( f32 fFriction ) { FASSERT( IsCreated() ); m_fFriction = fFriction; }
	FINLINE f32 GetFriction( void ) const { FASSERT( IsCreated() ); return m_fFriction; }

	FINLINE void SetPreserveVelocityFlag( BOOL bPreserveVelocity );
	FINLINE BOOL IsVelocityPreserved( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_PRESERVE_VELOCITY; }

	FINLINE void SetPlaneDist( f32 fPlaneDist ) { FASSERT( IsCreated() ); m_fPlaneDist = fPlaneDist; }
	FINLINE f32 GetPlaneDist( void ) const { FASSERT( IsCreated() ); return m_fPlaneDist; }

	FINLINE const CFMtx43A *GetUnitMtx( void ) const { FASSERT( IsCreated() ); return &m_UnitMtx; }

	FINLINE const CFVec3A *Box_GetHalfDim_MS( void ) const { FASSERT( IsCreated() ); return &m_BoxHalfDim_MS; }

	FINLINE void ComputeUnitMtx( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ComputeUnitMtxForDynamicAnchor( void );
	void _ComputeUnitMtxForBonedAnchor( void );
	void _ComputeBoxWorldSpaceCorners( void );


	friend class CFVerlet;
	friend class CFVerletTack;

	FCLASS_STACKMEM_ALIGN( CFVerletTackSurface );
} FCLASS_ALIGN_SUFFIX;



FINLINE void CFVerletTackSurface::ComputeUnitMtx( void ) {
	FASSERT( IsCreated() );

	switch( m_nAnchorType ) {
	case ANCHOR_TYPE_STATIC:
		return;

	case ANCHOR_TYPE_DYNAMIC:
		if( m_nFlags & FLAG_MUST_UNITIZE_MTX ) {
			// We must unitize the matrix...

			if( m_nUnitMtxComputedFrame != FVid_nFrameCounter ) {
				// We haven't already done it this frame...

				_ComputeUnitMtxForDynamicAnchor();
			}
		}

		return;

	case ANCHOR_TYPE_BONED:
		if( m_nUnitMtxComputedFrame != FVid_nFrameCounter ) {
			// We haven't already done it this frame...

			_ComputeUnitMtxForBonedAnchor();
		}

		return;

	default:
		FASSERT_NOW;
	};
}


FINLINE void CFVerletTackSurface::SetPreserveVelocityFlag( BOOL bPreserveVelocity ) {
	FASSERT( IsCreated() );

	if( bPreserveVelocity ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_PRESERVE_VELOCITY );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_PRESERVE_VELOCITY );
	}
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFVerletTack
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFVerletTack {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		ANCHOR_TYPE_NONE,				// Tack is not anchored to anything
		ANCHOR_TYPE_POINT_STATIC,		// Tack is anchored to an immovable point that will never move
		ANCHOR_TYPE_POINT_DYNAMIC,		// Tack is anchored to an immovable point that may move by some means outside the Verlet system
		ANCHOR_TYPE_POINT_BONED,		// Tack is anchored to an immovable point that may move via a bone hierarchy
		ANCHOR_TYPE_TACK,				// Tack is anchored to another tack
	} AnchorType_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		FLAG_ANCHOR_ENABLED			= 0x01,	// TRUE to enable the anchoring of this tack to its anchor
		FLAG_POINT_CONSTRAINT		= 0x02,	// TRUE when both m_fMinDistFromTackToAnchor and m_fMaxDistFromTackToAnchor are 0.0f
		FLAG_COLLISION_TACK			= 0x04,	// TRUE if this is a collision tack
		FLAG_WIRE_GEO_COLLIDABLE	= 0x08,	// TRUE if the wire geometry should be collidable

		FLAG_NONE					= 0x00
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static u32 m_nBoneTackPrefixLength;	// fclib_strlen(FVERLET_MESH_BONE_TACK_PREFIX)
	static u32 m_nBoneCTackPrefixLength;// fclib_strlen(FVERLET_MESH_BONE_CTACK_PREFIX)


	CFVec3A m_TackPos_MS;				// The tack's at-rest (initial) position in model space
										// For boned tacks, this is in bone space

	CFVec3A m_AnchorPos_WS;				// The anchor point the tack is to be fixed to (used when m_nAnchorType is ANCHOR_TYPE_POINT_*)
	CFVec3A m_BonedAnchorPos_BS;		// If our anchor is a boned anchor, this is the anchor position in bone space (Zero otherwise)

	u32 m_nBonedAnchorComputedFrame;	// Snapshot of FVid_nFrameCounter of when m_AnchorPos_WS was last computed for this boned anchor
	const CFMtx43A *m_pBonedAnchorMtx;	// If our anchor is a boned anchor, this points into the bone matrix palette (NULL otherwise)

	union {
		const CFVec3A *m_pAnchorPos_WS;	// Points to the anchor position that the tack is to be fixed to (valid when m_nAnchorType is ANCHOR_TYPE_POINT_*)
		CFVerletTack *m_pAnchorTack;	// Points to the tack this tack is anchored to (valid when m_nAnchorType is ANCHOR_TYPE_TACK)
	};

	f32 m_fMinDistFromTackToAnchor;		// Minimum constraint distance between tack and anchor
	f32 m_fMaxDistFromTackToAnchor;		// Maximum constraint distance between tack and anchor

	CFVec4 m_Weights;					// Precomputed magic constants: a[0]=C1, a[1]=C2, a[2]=C3, a[3]=C4
	CFVec4 m_ScaledWeights;				// m_Weights / lamda, where lamda is C1*C1 + C2*C2 + C3*C3 + C4*C4

	cchar *m_pszName;					// Tack name
	CFVerlet *m_pTackOwner;				// Pointer to the CFVerlet object that owns this tack

	CFVerletTack *m_pTackAnchoredToMe;	// The tack that's anchored to this tack (NULL=none)

	f32 m_fAnchorUnitHealth;			// Health of this tack's anchor
	FExplosion_GroupHandle_t m_hExplosionGroup; // The explosiion that is to be spawned when a tack reaches zero health

	CFWire *m_pWire;					// Wire to draw (NULL=none)
	u8 m_nWireGeoIndex;					// Which wire geo to draw (-1=none)

	u8 m_nFlags;						// See FLAG_* for info
	u8 m_nAnchorType;					// See AnchorType_e for info
	s8 m_nBonedTackBoneIndex;			// If this is a boned tack, this is its parent bone index into the matrix palette (-1=not a boned tack)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	u8 m_nUser;							// User data

	void *m_pUser1;						// User data
	void *m_pUser2;						// User data
	void *m_pUser3;						// User data




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Creation:
	FINLINE CFVerletTack() { m_pTackOwner = NULL; }

	void Create_MS( CFVerlet *pTackOwner, const CFVec3A *pPos_MS, cchar *pszTackName=NULL, cchar *pszExplosionGroupName=NULL );
	void Create_WS( CFVerlet *pTackOwner, const CFVec3A *pPos_MS, cchar *pszTackName=NULL, cchar *pszExplosionGroupName=NULL );

	void Init_MS( const CFVec3A *pPos_MS, cchar *pszTackName=NULL, cchar *pszExplosionGroupName=NULL );
	void Init_WS( const CFVec3A *pPos_MS, cchar *pszTackName=NULL, cchar *pszExplosionGroupName=NULL );

	void Reposition_MS( const CFVec3A *pPos_MS );
	void Reposition_WS( const CFVec3A *pPos_WS );

	void SetTackName( cchar *pszTackName );
	FINLINE void SetExplosionGroup( FExplosion_GroupHandle_t hExplosionGroup ) { FASSERT( IsCreated() );  m_hExplosionGroup = hExplosionGroup; }


	// Info:
	FINLINE BOOL IsCreated( void ) const { return (m_pTackOwner != NULL); }
	FINLINE cchar *GetTackName( void ) const { FASSERT( IsCreated() ); return m_pszName; }
	FINLINE const CFVec3A *GetTackPos_MS( void ) const { FASSERT( IsCreated() ); return &m_TackPos_MS; }
	FINLINE const CFVec4 *GetWeights( void ) const { FASSERT( IsCreated() ); return &m_Weights; }
	FINLINE const CFVec4 *GetScaledWeights( void ) const { FASSERT( IsCreated() ); return &m_ScaledWeights; }
	FINLINE CFVerlet *GetTackOwner( void ) const { FASSERT( IsCreated() ); return m_pTackOwner; }
	FINLINE FExplosion_GroupHandle_t GetExplosionGroup( void ) const { FASSERT( IsCreated() ); return m_hExplosionGroup; }

	FINLINE void ComputePos_WS( CFVec3A *pTackPos_WS ) const;


	// Utility:
	static BOOL IsStringPrefixedWithTackName( cchar *pszName );
	static BOOL IsStringPrefixedWithCTackName( cchar *pszName );
	static BOOL IsTackBone( FMeshBone_t *pBone, cchar **ppszTackName=NULL, BOOL *pbCTack=NULL );
	static u32 DetermineTackCountFromMesh( const FMesh_t *pMesh );


	// Collision:
	FINLINE BOOL IsCollisionTack( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_COLLISION_TACK; }
	void EnableCollisionTack( BOOL bCollisionTack );


	// Anchor:
	void Anchor_Enable( BOOL bEnable );
	void Anchor_Remove( void );
	void Anchor_SetToImmovablePoint_Static( const CFVec3A *pPos_WS=NULL );
	void Anchor_SetToImmovablePoint_Dynamic( const CFVec3A *pPos_WS );
	void Anchor_SetToImmovablePoint_Boned( const CFWorldMesh *pWorldMesh, u32 nBoneIndex );
	void Anchor_SetToImmovablePoint_Boned_WS( const CFMtx43A *pBoneMtx, const CFVec3A *pPos_WS );
	void Anchor_SetToImmovablePoint_Boned_BS( const CFMtx43A *pBoneMtx, const CFVec3A *pPos_BS );
	void Anchor_SetToTack( CFVerletTack *pAnchorToThisTack );
	void Anchor_SetRange( f32 fMinConstraintDist, f32 fMaxConstraintDist );
	void Anchor_SetUnitHealth( f32 fNewUnitHealth, BOOL bSilent=FALSE );

	void Anchor_DrawWire( u8 nWireGeoIndex );
	void Anchor_EnableWireCollision( BOOL bEnableCollision );

	FINLINE u8 Anchor_GetWireGeoIndex( void ) const { FASSERT( IsCreated() ); return m_nWireGeoIndex; }
	FINLINE BOOL Anchor_IsDrawWireEnabled( void ) const { FASSERT( IsCreated() ); return (m_nWireGeoIndex != -1); }
	FINLINE BOOL Anchor_IsWireCollidable( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_WIRE_GEO_COLLIDABLE; }


	FINLINE BOOL Anchor_IsEnabled( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_ANCHOR_ENABLED; }
	FINLINE AnchorType_e Anchor_GetType( void ) const { FASSERT( IsCreated() ); return (AnchorType_e)m_nAnchorType; }
	FINLINE const CFVec3A *Anchor_GetAnchorPoint( void ) const { FASSERT( IsCreated() ); FASSERT( m_nAnchorType==ANCHOR_TYPE_POINT_STATIC || m_nAnchorType==ANCHOR_TYPE_POINT_DYNAMIC || m_nAnchorType==ANCHOR_TYPE_POINT_BONED ); return m_pAnchorPos_WS; }
	FINLINE CFVerletTack *Anchor_GetAnchorTack( void ) const { FASSERT( IsCreated() ); FASSERT( m_nAnchorType==ANCHOR_TYPE_TACK ); return m_pAnchorTack; }
	FINLINE f32 Anchor_GetMinConstraintDist( void ) const { FASSERT( IsCreated() ); return m_fMinDistFromTackToAnchor; }
	FINLINE f32 Anchor_GetMaxConstraintDist( void ) const { FASSERT( IsCreated() ); return m_fMaxDistFromTackToAnchor; }
	FINLINE CFVerletTack *Anchor_GetTackAnchoredToMe( void ) const { FASSERT( IsCreated() ); return m_pTackAnchoredToMe; }
	FINLINE BOOL Anchor_IsPointConstraint( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_POINT_CONSTRAINT; }
	FINLINE f32 Anchor_GetUnitHealth( void ) const { FASSERT( IsCreated() ); return m_fAnchorUnitHealth; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _InitTack( CFVerlet *pTackOwner, const CFVec3A *pPos_MS, cchar *pszTackName, cchar *pszExplosionGroupName );
	void _RepositionTack( const CFVec3A *pPos_MS );
	void _UpdateBonedTack( const CFMtx43A *pBoneToBoneMtx );
	BOOL _CheckPointSave( void ); //have this tack object save itself 
	BOOL _CheckPointLoad( void ); //have this tack object reload itself
	void _UpdateTackWire( void );


	friend class CFVerlet;

	FCLASS_STACKMEM_ALIGN( CFVerletTack );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFVerlet
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFVerlet {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	struct Init_t {
		f32 fMass;							// Mass of the object

		f32 fDimX;							// X dimension of the object
		f32 fDimY;							// Y dimension of the object
		f32 fDimZ;							// Z dimension of the object
	};


	struct Init2_t {
		f32 fMass;							// Mass of the object

		f32 fDimX;							// X dimension of the object
		f32 fDimY;							// Y dimension of the object
		f32 fDimZ;							// Z dimension of the object

		cchar **apszBoneNames;				// Bones in the mesh that are to be included as part of the object
		cchar **apszBoneNameTacks;			// Bones in the mesh that are to become tacks
		cchar **apszBoneNameCTacks;			// Bones in the mesh that are to become collision tacks
	};


	typedef enum {
		NET_LIST_MEMBER_INACTIVE,			// No work whatsoever will be performed on this net
		NET_LIST_MEMBER_ACTIVE,				// Work will be performed on this net each frame

		NET_LIST_MEMBER_COUNT
	} NetListMember_e;


	typedef enum {
		UNANCHORED_COLL_TYPE_NONE,			// Don't perform any collision on this object
		UNANCHORED_COLL_TYPE_TACKS,			// Perform tack/plane collision only
		UNANCHORED_COLL_TYPE_SPHERE,		// Perform sphere collision against terrain and static objects
		UNANCHORED_COLL_TYPE_CAPSULE,		// Perform capsule collision against terrain and static objects
		UNANCHORED_COLL_TYPE_KDOP,			// Perform kDOP collision against terrain and static objects

		UNANCHORED_COLL_TYPE_COUNT
	} UnanchoredCollType_e;


	typedef void MovedCallback_t( CFVerlet *pVerlet, f32 fDeltaSecs );
	typedef void ConstraintCallback_t( CFVerlet *pVerlet );
	typedef void DeactivateCallback_t( CFVerlet *pVerlet );
	typedef void ExplodeCallback_t( CFVerlet *pVerlet );
	typedef BOOL TrackerCollCallback_t( CFVerlet *pVerlet, CFWorldTracker *pTracker );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		FLAG_CREATED							= 0x00000001,	// TRUE when this CFVerlet object has been created
		FLAG_INVMTX_BUILT						= 0x00000002,	// TRUE when m_InvUnitMtx has been computed
		FLAG_IN_SCOPE							= 0x00000004,	// TRUE when this object is in scope (near the player)
		FLAG_MOVING								= 0x00000008,	// TRUE when this object is moving
		FLAG_VISITED							= 0x00000010,	// Used to determine whether we've already visited this object
		FLAG_NET_MASTER_FREEZE					= 0x00000020,	// TRUE to never to work on this object (flag used on net masters only)
		FLAG_HAS_DYNAMIC_ANCHOR					= 0x00000040,	// TRUE when this object has at least one enabled dynamic anchor
		FLAG_HAS_BONED_ANCHOR					= 0x00000080,	// TRUE when this object has at least one enabled boned anchor
		FLAG_HAS_NON_ZERO_COM					= 0x00000100,	// TRUE when this object has a non-zero center-of-mass
		FLAG_ENABLE_WORK						= 0x00000200,	// TRUE to perform work on this object, FALSE to disable work on this object
		FLAG_COLLIDE_WITH_DYN_OBJ				= 0x00000400,	// TRUE to perform collisions against dynamic objects
		FLAG_MANUAL_WORK						= 0x00000800,	// TRUE when this object will be updated by some interface calling the ManualWork function

		FLAG_NET_MASTER_ONE_SHOT				= 0x00001000,	// One-shot objects are allowed to be in motion once, and will be frozen once they stop
		FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS	= 0x00002000,	// TRUE if this object should freeze itself once it becomes motionless
		FLAG_NET_MASTER_CTACK_HIT_CPLANE		= 0x00004000,	// TRUE if any of this object's tacks have collided with any of its collision planes

		FLAG_CAN_EXPLODE						= 0x00008000,	// TRUE if object should explode when it's not dangling any more and it hits a surface
		FLAG_EXPLODE_WHEN_TACK_HITS_SURFACE		= 0x00010000,	// TRUE if we should explode this object the next time a ctack hits a collision surface
		FLAG_CALL_EXPLODE_CALLBACK				= 0x00020000,	// TRUE if we should call the explode callback

		FLAG_NONE								= 0x00000000
	};


	struct Bone_t {
		u8 nBoneIndex;						// Bone index that the boned tacks are children of
		u8 nTackCount;						// The number of consecutive boned tacks (0=none)
		u8 nTackStartIndex;					// Index into m_pTackArray of the first boned tack (unused if nTackCount is 0)
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	void *m_pUser;							// User data (not initialized nor modified in any way by the FVerlet system)
	u32 m_nUser;							// User data (not initialized nor modified in any way by the FVerlet system)




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;				// TRUE when ModuleStartup() has been successfully called
	static f32 m_fSecsUntilNextWork;				// Seconds remaining until we do another physics work loop

	static const CFVec3A m_QuarterVec;				// ( 0.25, 0.25, 0.25 )
	static const CFVec3A m_aTetraPos_MS[4];			// Tetrahedron model space positions
	static const CFVec3A m_aRegularTetraPos_MS[4];	// Regular tetrahedron model space positions

	static FLinkRoot_t m_InactiveNetsRoot;			// Linklist of inactive net masters
	static FLinkRoot_t m_ActiveNetsRoot;			// Linklist of active net masters

	static CFVerlet **m_ppVerletWorkArray;			// Array of CFVerlet object pointers we're doing work on
	static CFVerlet **m_ppNetWorkArray;				// Array of net master pointers we're doing work on
	static u32 m_nWorkArrayCount;					// Number of elements in m_ppVerletWorkArray and m_ppNetWorkArray

	static u32 m_nVerletWorkCount;					// While performing work, this is the number of elements in m_ppVerletWorkArray
	static u32 m_nNetWorkCount;						// While performing work, this is the number of elements in m_ppNetWorkArray

	static cchar **m_apszTackNameArray;				// Optional array of bone names that are to be made into tacks
	static cchar **m_apszCTackNameArray;			// Optional array of bone names that are to be made into collision tacks
	static CFVerlet* m_pCBVerlet;					// Used to identify class in callbacks

	u32 m_nNetLastWorkedFrame;					// Contains the frame number of the last time the Work function was called.  ONLY VALID FOR THE MASTER
	u32 m_nFlags;								// See FLAG_* for info

	CFVec3A m_TetraDim;							// The x, y, and z dimensions of the tetrahedron

	CFVec3A m_aTetraPos_WS[4];					// Tetrahedron's four vertices
	CFVec3A m_aTetraOldPos_WS[4];				// Tetrahedron's four vertices last frame (required for the Verlet integrator)

	CFVec3A m_aTetraForce_WS[4];				// The current force vectors in world space to be applied to the tetrahedron's four vertices
	CFVec3A m_aTetraImpulse_WS[4];				// The current impulse vectors in world space to be applied to the tetrahedron's four vertices

	CFVec3A m_CenterOfMass_MS;					// Center of mass (usually 0,0,0)
	CFVec3A m_PreWorkPos_WS;					// Used by the work function to save off the previous origin's position
	CFMtx43A m_UnitMtx;							// Unit matrix that directly positions the solid geometry in world space
	CFMtx43A m_InvUnitMtx;						// Inverse of m_UnitMtx. Valid only when FLAG_INVMTX_BUILT is set. (call _ComputeInvMtx() to compute)

	MovedCallback_t *m_pFcnMovedCallback;				// Called every time m_UnitMtx has been updated (NULL=none)
	ConstraintCallback_t *m_pFcnConstraintCallback;		// Called for user-inforced constraints (NULL=none)
	DeactivateCallback_t *m_pFcnDeactivateCallback;		// Called when the verlet object has stopped and is moved to the deactive list
	ExplodeCallback_t *m_pFcnExplodeCallback;			// Called when the verlet object is ready to be exploded
	TrackerCollCallback_t *m_pFcnTrackerCollCallback;	// Called by the collision system during collision detection

	CFWorldMesh *m_pWorldMesh;					// The world mesh this object is tied to

	f32 m_fGravity;								// Gravity to apply each frame
	f32 m_fMass;								// Mass of the object
	f32 m_fHundredOverMass;						// 100/m_fMass (used to scale external forces and impulses)

	f32 m_fPrimeSecs;							// Number of seconds to run the simulation during the prime phase
	f32 m_fUnitDampen;							// 0=none
	f32 m_fMotionlessSecsRemaining;				// Number of seconds remaining before clearing the FLAG_MOVING flag

												// In the following m_K* values, x, y, and z are afrom m_TetraDim:
	f32 m_K1;									// 1/(-12*x*y*z)
	f32 m_K2;									// 1/(12*x*y)
	f32 m_K3;									// 1/y
	f32 m_K4;									// 12*x*y
	f32 m_K5;									// 6*y*z
	f32 m_K6;									// 4*x*z
	f32 m_K7;									// -3*x*y*z
	f32 m_K8;									// -4*x
	f32 m_K9;									// 12*y
	f32 m_K10;									// 3*x*y

	f32 m_fDim_v10_v12;							// Tetrahedron dimension from v1 to v0 and from v1 to v2
	f32 m_fDim_v30_v32;							// Tetrahedron dimension from v3 to v0 and from v3 to v2
	f32 m_fDim_v31;								// Tetrahedron dimension from v3 to v1

	CFVerlet *m_pNetMaster;						// Points to this object's net master (points to ourself if we're a master)
	FLink_t m_NetMasterLink;					// Link to other net masters (used only if we're a master)

	FLinkRoot_t m_NetSlaveRoot;					// Linklist of net slaves of this net master. Always includes ourselves. Used only if this is a net master.
	FLink_t m_NetSlaveLink;						// Link to other net slaves in the master's net

	Bone_t *m_pBoneArray;						// Array of boned tacks (NULL=none)
	u8 m_nBoneCount;							// Number of elements in m_pBoneArray
	u8 m_nBaseBoneIndex;						// Index into m_pBoneArray of this object's base bone

	u8 m_nNetInScopeCount;						// Number of members in this net that have their FLAG_IN_SCOPE flag set
	u8 m_nNetMovingCount;						// Number of members in this net that have their FLAG_MOVING flag set
	u8 m_nNetManualWorkCount;					// Number of members in this net that have their FLAG_MANUAL_WORK flag set
	u8 m_nNetListMember;						// See NetListMember_e for info

	u8 m_nUnanchoredCollType;					// Collision test type to perform when not tacked (see UnanchoredCollType_e for info)
	u8 m_nCollTackCount;						// Number of collision tacks
	u8 m_nTackCount;							// Number of tacks in m_pTackArray (0=none)
	u8 m_nTackSurfaceCount;						// Number of elements in m_ppTackSurfaceArray (0=none)

	CFVerletTack *m_pTackArray;					// Array of tack points (NULL=none)
	CFVerletTackSurface **m_ppTackSurfaceArray;	// Array of tack surfaces to collide with (NULL=none)

	CFVec4 m_aRegularTetraVtxWeights[4];		// Weights for the 4 vertices of the regular tetrahedron

	//NOTE : These are NOT used by the verlet system at all, so I commented them out (memory savings baby!) -- RAF
	//f32 m_fAggravateSecs;						// Time remaining to aggravate this object
	//f32 m_fAggravateUnitIntensity;				// Aggravation unit intensity




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleInitialized( void ) { return m_bSystemInitialized; }

	static void Reset( void );
	static void WorkAll( void );
	static void PrimeAll( void );
	static void DrawAll( void );


	// Creation:
	CFVerlet();
	~CFVerlet();

	static void SetBoneNameTackTable( cchar **apszTackNameArray, cchar **apszCTackNameArray );

	BOOL Create( f32 fMass, const CFVec3A *pObjectDim, u32 nTackCount, const CFMtx43A *pUnitMtx=&CFMtx43A::m_IdentityMtx, const CFVec3A *pCenterOfMass_MS=NULL );
	BOOL Create( f32 fMass, const CFVec3A *pObjectDim, const FMesh_t *pMesh, const CFMtx43A *pUnitMtx=&CFMtx43A::m_IdentityMtx, const CFVec3A *pCenterOfMass_MS=NULL );
	BOOL Create( const Init2_t *pInit2, const FMesh_t *pMesh );
	void Destroy( void );

	void AttachedToWorldMesh( CFWorldMesh *pWorldMesh );


	void SetCenterOfMass_WS( const CFVec3A *pCenterOfMass_WS, BOOL bUpdateBonedTacks=TRUE );
	void SetCenterOfMass_MS( const CFVec3A *pCenterOfMass_MS, BOOL bUpdateBonedTacks=TRUE );
	FINLINE const CFVec3A *GetCenterOfMass_MS( void ) const { FASSERT( IsCreated() ); return &m_CenterOfMass_MS; }

	FINLINE void SetWorldMesh( CFWorldMesh *pWorldMesh ) { FASSERT( IsCreated() ); m_pWorldMesh = pWorldMesh; }
	FINLINE CFWorldMesh *GetWorldMesh( void ) const { FASSERT( IsCreated() ); return m_pWorldMesh; }

	FINLINE void SetMovedCallback( MovedCallback_t *pFcnMovedCallback ) { FASSERT( IsCreated() ); m_pFcnMovedCallback = pFcnMovedCallback; }
	FINLINE MovedCallback_t *GetMovedCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnMovedCallback; }

	FINLINE void SetConstraintCallback( ConstraintCallback_t *pFcnConstraintCallback ) { FASSERT( IsCreated() ); m_pFcnConstraintCallback = pFcnConstraintCallback; }
	FINLINE ConstraintCallback_t *GetConstraintCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnConstraintCallback; }

	FINLINE void SetDeactivateCallback( DeactivateCallback_t *pFcnDeactivateCallback ) { FASSERT( IsCreated() ); m_pFcnDeactivateCallback = pFcnDeactivateCallback; }
	FINLINE DeactivateCallback_t *GetDeactivateCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnDeactivateCallback; }

	FINLINE void SetExplodeCallback( ExplodeCallback_t *pFcnExplodeCallback ) { FASSERT( IsCreated() ); m_pFcnExplodeCallback = pFcnExplodeCallback; }
	FINLINE ExplodeCallback_t *GetExplodeCallback( void ) const { FASSERT( IsCreated() ); return m_pFcnExplodeCallback; }
	FINLINE BOOL IsExplodeCallbackBeingCalled( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_CALL_EXPLODE_CALLBACK; }

	void SetTackSurfaceArray( CFVerletTackSurface **ppTackSurfaceArray );
	void SetTackSurfaceArray( CFVerletTackSurface **ppTackSurfaceArray, u32 nTackSurfaceCount );
	FINLINE CFVerletTackSurface **GetTackSurfaceArray( void ) const { FASSERT( IsCreated() ); return m_ppTackSurfaceArray; }
	FINLINE u32 GetNumTackSurfaces( void ) const { FASSERT( IsCreated() ); return m_nTackSurfaceCount; }

	FINLINE void SetDampening( f32 fUnitDampen ) { FASSERT( IsCreated() ); m_fUnitDampen = fUnitDampen; }
	FINLINE f32 GetDampening( void ) const { FASSERT( IsCreated() ); return m_fUnitDampen; }

	FINLINE void SetPrimeSecs( f32 fPrimeSecs ) { FASSERT( IsCreated() ); m_fPrimeSecs = fPrimeSecs; }
	FINLINE f32 GetPrimeSecs( void ) const { FASSERT( IsCreated() ); return m_fPrimeSecs; }

	FINLINE void SetGravity( f32 fGravity ) { FASSERT( IsCreated() ); m_fGravity = fGravity; }
	FINLINE f32 GetGravity( void ) const { FASSERT( IsCreated() ); return m_fGravity; }

	FINLINE void SetUnanchoredCollType( UnanchoredCollType_e nCollType ) { FASSERT( IsCreated() ); m_nUnanchoredCollType = nCollType; }
	FINLINE UnanchoredCollType_e GetUnanchoredCollType( void ) const { FASSERT( IsCreated() ); return (UnanchoredCollType_e)m_nUnanchoredCollType; }

	FINLINE void SetMass( f32 fMass );
	FINLINE f32	GetMass( void ) { FASSERT( IsCreated() ); return m_fMass; };

	FINLINE void SetCollisionCallback( TrackerCollCallback_t *pFcnCallback ) { FASSERT( IsCreated() ); m_pFcnTrackerCollCallback = pFcnCallback; };
	FINLINE TrackerCollCallback_t* GetCollisionCallback( void ) { FASSERT( IsCreated() ); return m_pFcnTrackerCollCallback; };

	FINLINE void EnableDynamicObjColl( BOOL bEnable );

	void SetObjectDim( const CFVec3A &vDim );
	void InitUnitMtx( const CFMtx43A *pUnitMtx, BOOL bKickStartNet = TRUE );


	// Nets:
	FINLINE void Net_Kickstart( void ) { FASSERT( IsCreated() ); if( m_pNetMaster->m_nNetInScopeCount && m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_INACTIVE) { _Net_Kickstart( m_pNetMaster ); } }
	void Net_MoveToInactiveList( void );
	void Net_MoveToActiveList( void );
	void Net_Freeze( BOOL bFreeze );

	void Net_MakeOneShot( BOOL bOneShot );

	FINLINE BOOL Net_IsFrozen( void ) const { FASSERT( IsCreated() ); return m_pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE; }


	// Work:
	FINLINE void SetManualWork( void ) { FASSERT( IsCreated() ); FASSERT( m_pNetMaster->m_nNetInScopeCount < 255 ); m_nFlags |= FLAG_MANUAL_WORK; ++m_pNetMaster->m_nNetManualWorkCount; }
	void ManualWork( void ); //only supposed to be called for objects with the FLAG_MANUAL_WORK bit set.
	void SetInScope( BOOL bInScope );
	void Integrate( f32 fDeltaSecs, BOOL bClearForces );
	void ApplyDampening( f32 fUnitDampen, f32 fDeltaSecs );
	void SatisfyConstraints( void );
	void ComputeMtx( void );

	FINLINE void EnableWork( BOOL bEnable );
	FINLINE BOOL IsWorkEnabled( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_ENABLE_WORK; }


	// Forces:
	void StopMotion( void );

	void ClearForces( void );
	void ApplyForce( const CFVec3A *pPos_WS, const CFVec3A *pForceVec_WS );
	void ApplyForceToCOM( const CFVec3A *pImpulseVec_WS );

	void ClearImpulses( void );
	void ApplyImpulse( const CFVec3A *pPos_WS, const CFVec3A *pImpulseVec_WS );
	void ApplyImpulseToCOM( const CFVec3A *pImpulseVec_WS );

	void ApplyDisplacementImpulse_Pos( const CFVec3A *pPos_WS, const CFVec3A *pTargetPos_WS );
	void ApplyDisplacementImpulse_Pos( const CFVec4 *pScaledWeights, const CFVec3A *pPos_WS, const CFVec3A *pTargetPos_WS );
	void ApplyDisplacementImpulse_Vec( const CFVec3A *pPos_WS, const CFVec3A *pDisplacementVec_WS );
	void ApplyDisplacementImpulse_Vec( const CFVec4 *pScaledWeights, const CFVec3A *pDisplacementVec_WS );


	// Collision:
	void CollideTacksWithTackSurface( CFVerletTackSurface *pTackSurface );
	void CollideTacksWithPlane( const CFVec3A *pPointOnPlane_WS, const CFVec3A *pPlaneUnitNormal_WS, f32 fFriction=0.0f, f32 fMinDistFromPlane=0.0f, BOOL bPreserveVelocity=FALSE );
	void CollideTacksWithBox( const CFMtx43A *pUnitMtx, const CFVec3A *pMinCorner_WS, const CFVec3A *pMaxCorner_WS, f32 fFriction=0.0f, BOOL bPreserveVelocity=FALSE );


	// Utility:
	FINLINE NetListMember_e GetListMember( void ) const { FASSERT( IsCreated() ); return (NetListMember_e)m_pNetMaster->m_nNetListMember; }

	FINLINE void WS2MS_Point( CFVec3A *pPoint_MS, const CFVec3A *pPoint_WS );
	FINLINE void WS2MS_Dir( CFVec3A *pDir_MS, const CFVec3A *pDir_WS );

	void ComputeWeights_MS( CFVec4 *pWeights, const CFVec3A *pPos_MS );
	void ComputeWeights_WS( CFVec4 *pWeights, const CFVec3A *pPos_WS );
	void ComputeScaledWeights_MS( CFVec4 *pScaledWeights, const CFVec3A *pPos_MS );
	void ComputeScaledWeights_WS( CFVec4 *pScaledWeights, const CFVec3A *pPos_WS );

	static FINLINE f32 ComputeLamda( const CFVec4 *pWeights ) { return pWeights->x*pWeights->x + pWeights->y*pWeights->y + pWeights->z*pWeights->z + pWeights->w*pWeights->w; }
	static FINLINE f32 ComputeInvLamda( const CFVec4 *pWeights ) { return fmath_Inv( pWeights->x*pWeights->x + pWeights->y*pWeights->y + pWeights->z*pWeights->z + pWeights->w*pWeights->w ); }
	static FINLINE void ScaleWeightsByInvLamda( CFVec4 *pWeights ) { f32 fOOLamda=ComputeInvLamda(pWeights); pWeights->x*=fOOLamda; pWeights->y*=fOOLamda; pWeights->z*=fOOLamda; pWeights->w*=fOOLamda; }

	void ComputeWSOriginPointFromMS( CFVec3A *pPos_WS );
	void ComputeWSSurfacePointFromMS( CFVec3A *pPos_WS, const CFVec4 *pWeights );
	void ComputeWSSurfacePointFromMS( CFVec3A *pPos_WS, const CFVec3A *pPos_MS );

	void ComputeOriginVelocity_WS( CFVec3A *pLinearVelocity_WS, f32 fInvDeltaSecs );
	void ComputeSurfaceVelocity_WS( CFVec3A *pSurfaceVelocity_WS, const CFVec3A *pPos_WS, f32 fInvDeltaSecs );
	void ComputeSurfaceVelocity_WS( CFVec3A *pSurfaceVelocity_WS, const CFVec4 *pWeights, f32 fInvDeltaSecs );

	void ComputeOriginStepDist_WS( CFVec3A *pLinearStepDist_WS );
	void ComputeSurfaceStepDist_WS( CFVec3A *pSurfaceStepDist_WS, const CFVec3A *pPos_WS );
	void ComputeSurfaceStepDist_WS( CFVec3A *pSurfaceStepDist_WS, const CFVec4 *pWeights );

	BOOL CheckPointSave( void ); //have this verlet object save itself 
	BOOL CheckPointLoad( void ); //have this verlet object reload itself


	// Destruction:
	FINLINE BOOL IsExplodeFunctionalityEnabled( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_CAN_EXPLODE; }
	void EnableExplodeFunctionality( BOOL bCanExplode );


	// Info:
	FINLINE BOOL IsCreated( void ) const { return (m_nFlags & FLAG_CREATED); }
	FINLINE const CFMtx43A *GetUnitMtx( void ) const { FASSERT( IsCreated() ); return &m_UnitMtx; }
	FINLINE const CFVec3A *GetTetraVtxPos_WS( u32 nVtxIndex ) const { FASSERT( IsCreated() ); FASSERT( nVtxIndex < 4 ); return &m_aTetraPos_WS[nVtxIndex]; }
	FINLINE const CFVec3A *GetTetraVtxOldPos_WS( u32 nVtxIndex ) const { FASSERT( IsCreated() ); FASSERT( nVtxIndex < 4 ); return &m_aTetraOldPos_WS[nVtxIndex]; }

	FINLINE u8 GetBaseBoneIndex( void ) const { FASSERT( IsCreated() ); return m_nBaseBoneIndex; }
	FINLINE u8 GetContributingBoneCount( void ) const { FASSERT( IsCreated() ); return m_nBoneCount; }
	FINLINE u8 GetBoneIndex( u8 nArrayIndex ) const { FASSERT( IsCreated() ); FASSERT( nArrayIndex < m_nBoneCount ); return m_pBoneArray[nArrayIndex].nBoneIndex; }

	FINLINE BOOL IsInScope( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_IN_SCOPE; }


	// Tack Interface:
	FINLINE u32 Tack_GetCount( void ) const { FASSERT( IsCreated() ); return m_nTackCount; }
	CFVerletTack *Tack_FindByName( cchar *pszTackName );
	s32 Tack_FindIndexByName( cchar *pszTackName );
	CFVerletTack *Tack_GetFromIndex( u32 nTackIndex ) const { FASSERT( IsCreated() ); FASSERT( nTackIndex < m_nTackCount ); return &m_pTackArray[nTackIndex]; }

	void Tack_InitAllTacksFromMesh( const FMesh_t *pMesh );


	// Constraints:
	static void Constraint_Distance( CFVec3A *pPos1_WS, CFVec3A *pPos2_WS, f32 fRestLength );
	static void Constraint_Range( CFVec3A *pPos1_WS, CFVec3A *pPos2_WS, f32 fMinRestLength, f32 fMaxRestLength );
	static void Constraint_AnchorDistance( CFVec3A *pPos_WS, const CFVec3A *pAnchorPos_WS, f32 fRestLength );
	static void Constraint_AnchorRange( CFVec3A *pPos_WS, const CFVec3A *pAnchor, f32 fMinRestLength, f32 fMaxRestLength );

	void TackConstraint_AnchorPoint( CFVerletTack *pTack, const CFVec3A *pAnchorPos_WS, f32 fMinConstraintDist, f32 fMaxConstraintDist );
	void TackConstraint_AnchorTack( CFVerletTack *pTack, CFVerletTack *pAnchorTack, f32 fMinConstraintDist, f32 fMaxConstraintDist );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static f32 _ApproxCubeRoot( f32 fVal );

	static void _Net_Kickstart( CFVerlet *pNetMaster );
	static void _Net_Merge( CFVerlet *pNetMaster1, CFVerlet *pNetMaster2 );
	static void _Net_Sever( CFVerlet *pVerlet1, CFVerlet *pVerlet2 );
	static void _Net_ResetVisitedFlag( CFVerlet *pNetMaster );
	static BOOL _Net_Find( CFVerlet *pVerlet, const CFVerlet *pVerletToFind );
	static void _Net_TransferMembersToNewMaster( CFVerlet *pVerlet, CFVerlet *pNewNetMaster );
	
	static u32 _TrackerCollisionCB( CFWorldTracker *pTracker );

	static void _FillNetArray( FLinkRoot_t *pLinkRoot );
	void _FillSlaveArray( void );
	void _NetWork( f32 fDeltaSecs, f32 fInvDeltaSecs, BOOL bPrimeMode, BOOL bFirstPass, BOOL bLastPass );
	void _InactiveNetWork( void );
	BOOL _ResolveCollisionState( f32 fDeltaSecs, f32 fInvDeltaSecs );
	BOOL _HandleCollision( f32 fDeltaSecs, f32 fInvDeltaSecs );
	void _UpdateBonedTacks( void );
	static void _NetListPrime( void );

	void _UpdateExplosionDetectionFlag( void );

	void _ComputeTetraVerticesFromMtx( void );
	void _HandleMotionlessDetection( f32 fDeltaSecs, f32 fInvDeltaSecs );

	void _ApplyTetraConstraints( void );
	void _ApplyTackConstraints( void );

	void _ComputeWeightedDisplacementVectors( CFVec3A *pTetraVertexDisplacementVec, const CFVec3A *pPos_WS, const CFVec3A *pDisplacementVec );
	void _ComputeWeightedDisplacementVectors( CFVec3A *pTetraVertexDisplacementVec, const CFVec4 *pScaledWeights, const CFVec3A *pDisplacementVec );

	void _ComputeInvMtx( void );
	void _ApplyPlaneConstraint( const CFVec3A *pPos_WS, const CFVec3A *pUnitAxis, const CFVec4 *pScaledWeights, f32 fPenetrationDist, f32 fFriction=0.0f, BOOL bPreserveVelocity=FALSE );

	void _Draw( void );
	void _DrawTetra( const CFVec3A *pTetraPosArray_WS );
	void _DrawTacks( void );

	void _UpdateHasDynamicAnchorFlag( void );
	void _UpdateHasBonedAnchorFlag( void );

	void _CollideTacksWithTackSurface( CFVerletTackSurface *pTackSurface );


	friend class CFVerletTack;

	FCLASS_STACKMEM_ALIGN( CFVerlet );
} FCLASS_ALIGN_SUFFIX;




FINLINE void CFVerlet::_ComputeInvMtx( void ) {
	FASSERT( IsCreated() );

	if( !(m_nFlags & FLAG_INVMTX_BUILT) ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_INVMTX_BUILT );
		m_InvUnitMtx.ReceiveAffineInverse( m_UnitMtx, FALSE );
	}
}


FINLINE void CFVerlet::WS2MS_Point( CFVec3A *pPoint_MS, const CFVec3A *pPoint_WS ) {
	FASSERT( IsCreated() );

	_ComputeInvMtx();
	m_InvUnitMtx.MulPoint( *pPoint_MS, *pPoint_WS );
}


FINLINE void CFVerlet::WS2MS_Dir( CFVec3A *pDir_MS, const CFVec3A *pDir_WS ) {
	FASSERT( IsCreated() );

	_ComputeInvMtx();
	m_InvUnitMtx.MulPoint( *pDir_MS, *pDir_WS );
}


FINLINE void CFVerlet::EnableWork( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( bEnable ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_ENABLE_WORK );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_ENABLE_WORK );
	}
}


FINLINE void CFVerletTack::ComputePos_WS( CFVec3A *pTackPos_WS ) const {
	FASSERT( IsCreated() );

	m_pTackOwner->ComputeWSSurfacePointFromMS( pTackPos_WS, &m_Weights );
}


FINLINE void CFVerlet::SetMass( f32 fMass ) {
	FASSERT( IsCreated() );
	FASSERT( fMass > 0.001f );

	m_fMass = fMass;
	m_fHundredOverMass = 100.0f / fMass;
}


FINLINE void CFVerlet::EnableDynamicObjColl( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( bEnable ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_COLLIDE_WITH_DYN_OBJ );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_COLLIDE_WITH_DYN_OBJ );
	}
}


#endif

