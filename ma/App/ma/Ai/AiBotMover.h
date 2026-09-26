#ifndef _AIBOTMOVER_H_
#define _AIBOTMOVER_H_ 1

//////////////////////////////////////////////////////////////////////////////////////
// AIBotMover.h 
//  classes:
//		CAIBotMover - Controls the movement of Ground Based (XZ plane) AI based robots
//				      Primary function is to act as interface that hides low-level physics
//                        implementation of robots.  Derive from CAIBotMover to create specialized
//                        versions that can control robots with unique physical abilities
//                    Provides a clean interface for doing common bot things like;
//						  Following a path
//                        Going toward a point
//                        Turning to face a point
//					  Contains default implementation of object avoidance for bots.
//                        
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
// 03/14/02 Mackellar      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fmath.h"
#include "AIMath.h"
#include "AIMover.h"

class CBot;
class CAIBrain;

FCLASS_ALIGN_PREFIX class CAIBotMover : public CAIMover
{
public:
	CAIBotMover(void);
	virtual ~CAIBotMover(void);

	virtual void Create(CEntity* pEntity, u16 uGUID);
	virtual void AddToWorld(void);
	virtual void RemoveFromWorld(void);

	virtual BOOL CalcAvoidanceTestCyln(CFCylnA* pCyln);

	//Assign the mover to a path
	virtual void AssignPath(CAIPath* pPath,	   //paths usually are obtained from the path finder (CAIGraphSearch) NULL tells Mover to clear previously assigned paths
							u32 uPathInfoFlags = CAIMover::ASSIGNPATHFLAG_NONE);
	
	//
	// Frame by Frame Control of the Mover
	//
	virtual void BeginFrame(void);	//call this at the top of every game frame. It does important stuff
	
	virtual BOOL FollowPath(void);  //FALSE means not done, TRUE means path complete

	// SimpleMoveToward a Point
	// must be called every frame
	// returns void //TRUE when Mover is very very near gotopoint_ws
	virtual void SimpleMoveToward(const CFVec3A& GotoLoc_WS);	//location to Move to
							//f32* pfVelAlgnmnt = NULL);		//If !NULL, will get set to the cos of the angle between mover's current velocity and goal!

	// MoveToward a Point
	// must be called every frame
	// returns TRUE when Mover is very very near gotopoint_ws
	virtual BOOL MoveToward(const CFVec3A& GotoLoc_WS,		//location to Move to
							f32* pfVelAlgnmnt = NULL);		//If !NULL, will get set to the cos of the angle between mover's current velocity and goal!

	// MoveToward a Point, but approach at a certain angle 
	// must be called every frame
	// returns TRUE when Mover is very very near gotopoint_ws
	virtual BOOL MoveTowardUseApproachConstraint(	const CFVec3A& GotoPoint_WS,		//location to Move to
													const CFVec3A& ApproachVecUnit,		//EntryRay to approach GotoPoint on  (Origin is at GotoPoint)
													f32 fMaxApproachDist,				//Max Distance from Approach Vector
													f32* pfVelAlgnmnt = NULL);			//If !NULL, will get set to the cos of the angle between mover's current velocity and goal!
	

	// returns TRUE when Mover's Torso is facing Loc
	virtual BOOL FaceToward(const CFVec3A& Loc_WS,			//location to aim at
							f32* pfTorsoAlgnmnt = NULL);	//If !NULL, will get set to the cos of the angle between mover's current XZ heading and goal!
	virtual BOOL HoverTowardY(f32 fY_WS)				{ return TRUE;}	//no default implementation
	
	virtual void EndFrame(void);		  //call when no more orders will be given this frame

	//
	// Talk State
	//
	virtual BOOL CanTalkNow(BOOL bRequireLowerBody = FALSE);
	BOOL IsSafeToStop(void);

	FINLINE CBot* GetBot(void)											{ FASSERT(!m_pEntity || (m_pEntity->TypeBits() & ENTITY_BIT_BOT)); return (CBot*) m_pEntity;}

	static BOOL _ActiveBrain_CB(CAIBrain* pOtherBrain, void* pData);
	static BOOL IterateEntityObstacleCBFunc(CEntity* pEntity, void* pData);

//protected:
	FINLINE BOOL HasCanHopPathFlags(void)								{ return (m_uAssignPathFlags & (ASSIGNPATHFLAG_CANHOP | ASSIGNPATHFLAG_CANHOPALONG));}
	FINLINE BOOL HasCanRollPathFlags(void)								{ return (m_uAssignPathFlags & (ASSIGNPATHFLAG_CANROLL | ASSIGNPATHFLAG_CANROLLALONG));}
	FINLINE BOOL HasHoppingPathFlags(void)								{ return (m_uAssignPathFlags & ASSIGNPATHFLAG_ISHOPPING);}
	FINLINE BOOL HasRollingPathFlags(void)								{ return (m_uAssignPathFlags & ASSIGNPATHFLAG_ISROLLING);}
	BOOL _StartHopPath(void);
	BOOL _HandleHoppingPath(void);
	BOOL _StartRollPath(void);
	BOOL _HandleRollingPath(void);

	// returns TRUE when Mover's Torso is facing Loc
	BOOL _FaceTowardXZ(	const CFVec3A& Loc_WS,				//location to face torso at
						f32* pfTorsoAlgnmnt = NULL);		//If !NULL, will get set to the cos of the angle between mover's current 3D heading and goal!

	BOOL _FaceTowardPitch(	const CFVec3A& Loc_WS);		//If !NULL, will get set to the cos of the angle between mover's current 3D heading and goal!
	
	void _FindObstacles(void);
	void _AvoidObstacles(void);

	virtual void _DoStuckWork(void);
	void _PrepareRunningJump(void);
	f32 DistToNextWay(void);
	BOOL FollowPath_NormalEdge(void);	// FALSE means not done, TRUE means path complete
	BOOL FollowPath_JumpEdge(void);		// FALSE means not done, TRUE means path complete
	BOOL FollowPath_DoorEdge(void);		// FALSE means not done, TRUE means path complete
	BOOL FollowPath_LiftEdge(void);		// FALSE means not done, TRUE means path complete
	BOOL FollowPath_MechEntry(void);
	CAIPathWaypoint* FollowPath_WayPtProgress(void);
	BOOL CalcFrontRightAlgnDotsXZ(const CFVec3A& Location, f32* pfFrontDot, f32* pfRightDot);

	//
	//	Forces are either steering forces, or movement forces
    //  They are what are ultimately fed into the AIController, and then into the Bot Controller
	//
	enum
	{
		FORCETYPE_PREVIOUS = 0, // he previously applied force
		FORCETYPE_GOAL,			// MoveToward() or FaceToward() directions are called GOAL_FORCE 
		FORCETYPE_AVOID,		// AvoidObstacles() directions are called AVOID_FORCE
		FORCETYPE_UNSTUCK,		// EndFrame() directions are called UNSTUCK_FORCE
		NUM_FORCETYPES,
	};

	//jump stages
	enum
	{
		JUMP_STAGE_NOPE = 0,
		JUMP_STAGE_LOCKEDGE,
		JUMP_STAGE_ALIGN,
		JUMP_STAGE_RUNNINGSTART,
		JUMP_STAGE_INAIR,
		JUMP_STAGE_RETRY_1,
		JUMP_STAGE_RETRY_2,
		JUMP_STAGE_RETRY_3,
		JUMP_STAGE_ARRIVALPENDING,
	};

	//door stages
	enum
	{
		DOOR_STAGE_NOPE = 0,
		DOOR_STAGE_ENROUTE,
		DOOR_STAGE_WAITING,
		DOOR_STAGE_FINDSWITCH,
		DOOR_STAGE_SWITCH,
	};

	//lift stages
	enum
	{
		LIFT_STAGE_NOPE = 0,
		LIFT_STAGE_WAITINGFOR,
		LIFT_STAGE_ENTERING,
		LIFT_STAGE_ON,
		LIFT_STAGE_EXITING,
		LIFT_STAGE_FINDSWITCH,
	};

	//stages for when on a path to enter a mech
	enum
	{
		MECH_ENTRY_STAGE_NOPE = 0,
		MECH_ENTRY_STAGE_ATBASE,
		MECH_ENTRY_STAGE_INAIR,
		MECH_ENTRY_STAGE_SUCKTOWARDSEAT,
		MECH_ENTRY_STAGE_DONE,
		MECH_ENTRY_STAGE_FAILED,
	};
	
	// when following a path, there
	// are separate work functions that get called
	// depending on the type of edge
	enum
	{
		EF_EDGE_NORMAL		= 0x00,
		EF_EDGE_JUMP		= 0x01,
		EF_EDGE_DOOR		= 0x02,
		EF_EDGE_LIFT		= 0x04,
		EF_EDGE_JUMPCREVACE	= 0x08,
		EF_EDGE_HAS_SWITCH	= 0x10,
		EF_EDGE_USE_SWITCH1	= 0x20,
		EF_EDGE_HOVER		= 0x40,
	};


//#ifdef DEBUG
	CAIBrain			*m_pBrain;		// the brain that is currently controlling this mover.. findfix: only here for debugging!
	CBot				*m_pBot;		// Bot layer defines the physical interface to bot.. Not really needed since m_pBot==AIMover::m_pEntity
//#endif
	f32					m_afRotForce[NUM_FORCETYPES];	// requested steering(rotation) forces for the current frame
	f32					m_afDXForce[NUM_FORCETYPES];	// requested World X movement forces 
	f32					m_afDZForce[NUM_FORCETYPES];	// requested World Z movement forces
	f32					m_fAvoidForceRotation;			// avoidance force rotation (this happens if the bot is trying to avoid, but is stuck)
	s8					m_nNumRotForces;				// how many steering influences are there this frame?
	s8					m_nNumDXForces;					// how many driving (world x axis) influences are there this frame?
	s8					m_nNumDZForces;					// how many driving (world z axis) influences are there this frame?

	enum
	{
		BOTMOVERFLAG_RECEIVED_EXTERNAL_TORSO_GOAL		= 0x01,
		BOTMOVERFLAG_RECEIVED_EXTERNAL_MOVE_GOAL		= 0x02,
		BOTMOVERFLAG_HAS_RECENT_TORSO_GOAL				= 0x04,
		BOTMOVERFLAG_RECEIVED_HOVER_GOAL				= 0x08
	};
	u8					m_uBotMoverFlags;
	u8					m_uEdgeType;

	//jumping
	u8					m_uJumpStage;
	u8					m_uHazardId;
	u8					m_uDoorStage;
	u8					m_uLiftStage;
	u8					m_uMechEntryStage;

	//hopping & rolling
	u8					m_uHopMin;				//Min time between hop moves
	u8					m_uHopMax;				//Max time between hop moves
	u16					m_uHopTimeOut;

	u8					m_uRollMin;				//Min time between Roll moves
	u8					m_uRollMax;				//Max time between Roll moves
	u16					m_uRollTimeOut;

	
	FCLASS_STACKMEM_ALIGN( CAIBotMover );
} FCLASS_ALIGN_SUFFIX;



FCLASS_ALIGN_PREFIX class CAIHoverBotMover : public CAIBotMover
{
	public:
	CAIHoverBotMover(void);
	virtual ~CAIHoverBotMover(void);

	virtual void BeginFrame(void);	//call this at the top of every game frame. It does important stuff
	virtual void EndFrame(void);	//call when no more orders will be given this frame

	BOOL HoverTowardY(f32 fY_WS);

	f32					m_afDYForce[NUM_FORCETYPES];	// requested World Y movement forces 
	s8					m_nNumDYForces;					// how many hovering influences are there this frame?
	f32					m_fMinAltitude;

};


#endif _AIBOTMOVER_H_
