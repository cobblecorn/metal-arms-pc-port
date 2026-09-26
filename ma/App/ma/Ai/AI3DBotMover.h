#ifndef _AI3DBOTMOVER_H_
#define _AI3DBOTMOVER_H_ 1

//////////////////////////////////////////////////////////////////////////////////////
// AI3DBotMover.h 
//  classes:
//		CAI3DBotMover - High-Level 3D movement controls of CBots.
//				      Primary function is to act as a wrapper that hides low-level physics/animation
//                        details of CBot.  Derive from CAI3DBotMover to create specialized
//                        versions that can control robots with unique physical abilities
//                    Provides a simple, clean interface for;
//						  Following a path
//                        Moving toward a point
//                        Turning to face a point
//					  Contains default implementation of object avoidance for bots.
//					  Tuneable params dictate torso-alignment and strafing abilities used during movement
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
// 06/11/02 Mackellar      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fmath.h"
#include "AIMath.h"
#include "AIMover.h"

class CBot;
class CAIBrain;

FCLASS_ALIGN_PREFIX class CAI3DBotMover : public CAIMover
{
public:
	CAI3DBotMover(void);
	virtual ~CAI3DBotMover(void);

	virtual void Create(CEntity* pEntity, u16 uGUID);
	virtual void AddToWorld(void);
	virtual void RemoveFromWorld(void);

	virtual void AssignPath(CAIPath* pPath, u32 uPathInfoFlags = 0);
	
	//
	// Frame by Frame Control of the Mover
	//
	virtual void BeginFrame(void);	  //call this at the top of every game frame. It does important stuff
	
	//give the mover something to do
	virtual BOOL FollowPath(void);  //FALSE means not done, TRUE means path complete

	virtual BOOL MoveToward(const CFVec3A& GotoPoint_WS,		//location to Move to
							f32* pfVelAlgnmnt = NULL);			//If !NULL, will get set to the cos of the angle between mover's current velocity and goal!

	virtual BOOL MoveTowardUseApproachConstraint(	const CFVec3A& GotoPoint_WS,	  //location to Move to
								const CFVec3A& EntryVecUnit,  //EntryRay to approach GotPoint on
								f32 fMaxEntryVecDist,		  //Max Distance from Ray to move Freely within
								f32* pfVelAlgnmnt = NULL);	  //If NotNull, will get set to the cos of the angle between mover's current velocity and goal!
	
	virtual BOOL FaceToward(const CFVec3A& Location,			//location to face torso at
							f32* pfTorsoAlgnmnt = NULL);		//If !NULL, will get set to the cos of the angle between mover's current 3D heading and goal!
	
	virtual void EndFrame(void);		  //call when no more orders will be given this frame

	virtual BOOL CalcAvoidanceTestCyln(CFCylnA* pCyln);
	//
	// Talk State
	//
	virtual BOOL CanTalkNow(BOOL bRequireLowerBody = FALSE);

	inline CBot* GetBot(void)							{ return (CBot*) m_pEntity;}
	inline const CFVec3A& GetUnitFacingVec(void)		{ return m_HeadingUnit;}

	static BOOL _ActiveBrain_CB(CAIBrain* pOtherBrain, void* pData);

//protected:
	void FindObstacles(void);
	void AvoidObstacles(void);

	void DoStuckWork(void);
	f32 DistToNextWay(void);
	virtual CAIPathWaypoint* FollowPath_WayPtProgress(void);
	BOOL FollowPath_NormalEdge(void);	// FALSE means not done, TRUE means path complete
	// returns TRUE when Mover is moving directly at GotoPoint_WS
	BOOL MoveTowardXZ(	const CFVec3A& GotoPoint_WS,		//location to Move to
						f32* pfVelAlgnmnt = NULL);			//If !NULL, will get set to the cos of the angle between mover's current velocity and goal!

	// returns TRUE when Mover's Torso is facing Loc
	BOOL FaceTowardXZ(	const CFVec3A& Location,			//location to face torso at
						f32* pfTorsoAlgnmnt = NULL);		//If !NULL, will get set to the cos of the angle between mover's current 3D heading and goal!

	BOOL FaceTowardPitch(	const CFVec3A& Location,			//location to pitch and look at
						f32* pfTorsoAlgnmnt = NULL);		//If !NULL, will get set to the cos of the angle between mover's current 3D heading and goal!

	enum
	{
		FORCETYPE_PREVIOUS = 0, // he previously applied force
		FORCETYPE_GOAL,			// MoveToward() or FaceToward() directions are called GOAL_FORCE 
		FORCETYPE_AVOID,		// AvoidObstacles() directions are called AVOID_FORCE
		FORCETYPE_UNSTUCK,		// EndFrame() directions are called UNSTUCK_FORCE
		NUM_FORCETYPES,
	};

	CFVec3A				m_HeadingUnit;

//#ifdef DEBUG
	CAIBrain			*m_pBrain;		// the brain that is currently controlling this mover.. findfix: only here for debugging!
	CBot				*m_pBot;		// Bot layer defines the physical interface to bot.. Not really needed since m_pBot==AIMover::m_pEntity
//#endif
	f32					m_afYawForce[NUM_FORCETYPES];	// requested steering(y rotation) forces for the current frame
	f32					m_afPitchForce[NUM_FORCETYPES];	// requested steering(x rotation) forces for the current frame
	f32					m_afDXForce[NUM_FORCETYPES];	// requested World X movement forces 
	f32					m_afDYForce[NUM_FORCETYPES];	// requested World Y movement forces
	f32					m_afDZForce[NUM_FORCETYPES];	// requested World Z movement forces
	s16					m_nNumPitchForces;				// how many pitch steering influences are there this frame?
	s16					m_nNumYawForces;				// how many yaw steering influences are there this frame?
	s16					m_nNumDXForces;					// how many driving (X Axis) influences are there this frame?
	s16					m_nNumDYForces;					// how many driving (Y Axis) influences are there this frame?
	s16					m_nNumDZForces;					// how many driving (Z Axis) influences are there this frame?

	enum
	{
		RECEIVED_EXTERNAL_TORSO_GOAL					= 0x0001,
		RECEIVED_EXTERNAL_MOVE_GOAL						= 0x0002,
		HAS_RECENT_TORSO_GOAL							= 0x0004,
		MODEL_ORIGIN_IS_MORE_NEAR_BOTTOM_OF_CYLN		= 0x0008,
		MODEL_ORIGIN_IS_MORE_NEAR_MIDDLE_OF_CYLN		= 0x0010,
	};
	#define MODEL_ORIGIN_FLAGS_MASK  (MODEL_ORIGIN_IS_MORE_NEAR_BOTTOM_OF_CYLN|MODEL_ORIGIN_IS_MORE_NEAR_MIDDLE_OF_CYLN)

	u16					m_u3DBotMoverFlags;

	// when following a path, there
	// are separate work functions that get called
	// depending on the type of edge
	enum
	{
		EF_EDGE_NORMAL		= 0x00,
	};

	u8					m_uEdgeType;

	
	FCLASS_STACKMEM_ALIGN( CAI3DBotMover );
} FCLASS_ALIGN_SUFFIX;

#endif _AIBOTMOVER_H_
