//////////////////////////////////////////////////////////////////////////////////////
// AIMover.h - 
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
// 04/15/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIMOVER_H_
#define _AIMOVER_H_ 1

#include "fmath.h"
#include "ftl.h"
#include "AIMath.h"
#include "AIControlGoals.h"
#include "AIPath.h"
#include "AIWeaponCtrl.h"

class CAIPath;
class CEntity;
class CBot;


#define ASSIGNPATHFLAG_HOPCOUNTSHIFT 24
#define ASSIGNPATHFLAG_ROLLCOUNTSHIFT 24

FCLASS_ALIGN_PREFIX class CAIMover
{
public:
						CAIMover(void);
	virtual				~CAIMover(void);

	virtual void		Create(CEntity* pEntity, u16 uGUID);
	virtual void		AddToWorld(void);
	virtual void		RemoveFromWorld(void);

	virtual void		BeginFrame(void);	  //call this at the top of every game frame. It does important stuff

	enum
	{
		MOVERFLAG_NONE									= 0x00000000,
		MOVERFLAG_TORSO_ALIGNMENT_CONSTRAINT_IGNORABLE  = 0x00000001,			   //can the m_fTorsoAlignmentSpeedConstraint be ignored when given conflicting FaceGoals and MoveGoals
		MOVERFLAG_TALKING								= 0x00000002,			   //Is a talk animation going on?
		MOVERFLAG_RECEIVED_EXTERNAL_HEADLOOK_GOAL		= 0x00000004,
		MOVERFLAG_DONT_AVOID_PLAYER_BOTS				= 0x00000008,
		MOVERFLAG_STUCK_WHILE_AVOIDING_LAST_FRAME		= 0x00000010,		   //Was it stuck last frame due to avoiding something
		MOVERFLAG_DISABLE_OBJECT_AVOIDANCE				= 0x00000020,
		MOVERFLAG_CAN_HOVER								= 0x00000040,
		MOVERFLAG_CAN_FLY								= 0x00000080
	};

	//owners of AIMovers can look at these structures to get 
	//summary information about internal state of the mover
	enum
	{
		FEEDBACK_NONE						= 0x00000000,
		FEEDBACK_PATH_OBSTRUCTED			= 0x00000001,
		FEEDBACK_PATH_SLOWPROGRESS			= 0x00000002,
		FEEDBACK_PATH_COMPLETE				= 0x00000004,
		FEEDBACK_STOPPED					= 0x00000008	 //the unit is currently not moving in any direction
	};

	
	//
	// Frame by Frame Control of the Mover
	//
	enum
	{
		ASSIGNPATHFLAG_NONE					= 0x00000000,
		ASSIGNPATHFLAG_CANHOP				= 0x00000001,
		ASSIGNPATHFLAG_CANHOPALONG			= 0x00000002,
		ASSIGNPATHFLAG_ISHOPPING			= 0x00000004,
		ASSIGNPATHFLAG_CANROLL				= 0x00000008,
		ASSIGNPATHFLAG_CANROLLALONG			= 0x00000010,
		ASSIGNPATHFLAG_ISROLLING			= 0x00000020,
		ASSIGNPATHFLAG_HOPCOUNTMASK			= 0xff000000,
		ASSIGNPATHFLAG_ROLLCOUNTMASK		= 0xff000000,
		ASSIGNPATHFLAG_PRECISION_GOAL		= 0x00000040,
		ASSIGNPATHFLAG_ENTERMECH_PATH		= 0x00000080  
	};
	virtual void		AssignPath(CAIPath* pPath, u32 uAssignPathFlags = 0)	{ }


	virtual BOOL		FollowPath(void)								{ return TRUE;}	// returns 0 to mean NOT done, 1 to mean path completed
	virtual BOOL		IsFollowingPath(void)							{ return m_pPath != NULL;}
	
	// SimpleMoveToward a Point
	// must be called every frame
	// returns void //TRUE when Mover is very very near gotopoint_ws
	virtual void SimpleMoveToward(const CFVec3A& GotoLoc_WS)	//location to Move to
							//f32* pfVelAlgnmnt = NULL);		//If !NULL, will get set to the cos of the angle between mover's current velocity and goal!
							{ return;}
	// MoveToward (a Point)
	// must be called every frame
	// returns TRUE when Mover is very very near gotopoint_ws
	virtual BOOL		MoveToward(	const CFVec3A& GotoLoc_WS,		//location to Move to
									f32* pfVelAlgnmnt = NULL)		//If !NULL, will get set to the cos of the angle between mover's current velocity and goal!
									{ return TRUE;}

	// MoveToward a Point, but approach at a certain angle 
	// must be called every frame
	// returns TRUE when Mover is very very near gotopoint_ws
	virtual BOOL		MoveTowardUseApproachConstraint(const CFVec3A& GotoPoint_WS,	//location to Move to
														const CFVec3A& EntryVecUnit,	//EntryRay to approach GotPoint on
														f32 fMaxEntryVecDist,			//Max Distance from Ray to move Freely within
														f32* pfVelAlgnmnt = NULL)		//If !NULL, will get set to the cos of the angle between mover's velocity current and goal!
														{ return TRUE;}

	// returns TRUE when Mover's Torso is facing Loc
	virtual BOOL		FaceToward(	const CFVec3A& Location,			//location to face torso at
									f32* pfTorsoAlgnmnt = NULL)			//If !NULL, will get set to the cos of the angle between mover's velocity current and goal!
									{ return TRUE;}

	virtual void		EndFrame(void) { }						//call when no more orders will be given this frame

	virtual f32			GetAvoidSteeringInfluence(void)					{ return 0.0f;}  //different AIMovers might store this information differently
	virtual BOOL		CalcAvoidanceTestCyln(CFCylnA* pCyln)			{ return FALSE;}

	void				StopAndClearCmds(void);

	enum
	{
		MECH_ENTRY_NOPROXIMITYCHECK	=0,
		MECH_ENTRY_PROXIMITYCHECK = 1
	};
	void				RequestMechEntry(CEntity* pMech, BOOL bProximityCheck = MECH_ENTRY_PROXIMITYCHECK);
	void				RequestMechExit(CEntity* pMech);


	//
	// Some status info about this mover that AI might need to care about
	//
	enum
	{
		MOVERCAP_NONE					= 0x0000,
		MOVERCAP_CAN_MOVE_XZ			= 0x0001,
		MOVERCAP_CAN_MOVE_Y				= 0x0002,
		MOVERCAP_CAN_JUMP				= 0x0004,
		MOVERCAP_CAN_ROTATE_YAW			= 0x0008,
		MOVERCAP_CAN_ROTATE_PITCH		= 0x0010,
		MOVERCAP_CAN_FIRE_PRIMARY		= 0x0020,	
		MOVERCAP_CAN_FIRE_SECONDARY		= 0x0040,
		MOVERCAP_CAN_AIM_PRIMARY		= 0x0080,	
		MOVERCAP_CAN_AIM_SECONDARY		= 0x0100,
		MOVERCAP_CAN_SEE				= 0x0200,
		MOVERCAP_CAN_ALL				= 0xffff
	};
	FINLINE u16			GetMoverCaps(void)									{ return m_uMoverCaps;}
	FINLINE BOOL		CanMove(void)										{ return (m_uMoverCaps & (MOVERCAP_CAN_MOVE_XZ | MOVERCAP_CAN_MOVE_Y));}
	FINLINE BOOL		CanRotate(void)										{ return (m_uMoverCaps & (MOVERCAP_CAN_ROTATE_YAW | MOVERCAP_CAN_ROTATE_PITCH));}
	FINLINE BOOL		CanJump(void)										{ return m_uMoverCaps & MOVERCAP_CAN_JUMP;}
	FINLINE BOOL		CanSee(void)										{ return m_uMoverCaps & MOVERCAP_CAN_SEE;}
	FINLINE BOOL		IsSleeping(void);
	BOOL				WakeUp(void);
	FINLINE BOOL		CanFirePrimary(void)								{ return m_uMoverCaps & MOVERCAP_CAN_FIRE_PRIMARY;}
	FINLINE BOOL		CanFireSecondary(void)								{ return m_uMoverCaps & MOVERCAP_CAN_FIRE_SECONDARY;}
	FINLINE BOOL		CanAimPrimary(void)									{ return m_uMoverCaps & MOVERCAP_CAN_AIM_PRIMARY;}
	FINLINE BOOL		CanAimSecondary(void)								{ return m_uMoverCaps & MOVERCAP_CAN_AIM_SECONDARY;}
	BOOL				CanDriveMech(void);
	BOOL				CanHover(void)										{ return m_uMoverFlags & MOVERFLAG_CAN_HOVER;}
	BOOL				CanFly(void)										{ return m_uMoverFlags & MOVERFLAG_CAN_FLY;}

	//
	// Talk State
	//
	BOOL				IsTalkingWithOnlyAudio(void);
	BOOL				IsTalkingWithAnim(void);						//if the bot is currently talking and playing animation
	BOOL				IsTalkingWithLowerBodyAnim(void);				//if the bot is currently using its lower body in an animation... means bot should not move
	virtual BOOL		CanTalkNow(BOOL bRequireLowerBody = FALSE)		{ return FALSE;}
	virtual BOOL		IsSafeToStop(void)								{ return TRUE;}

	//
	// AIMovers have a weapon controller
	//
	FINLINE u32				GetNumWeaponCtrls(void)							{ return m_uNumWeaponCtrls;}
	FINLINE CAIWeaponCtrl*	GetWeaponCtrlPtr(u32 uWeaponIndex)				{ if (uWeaponIndex >= 0 && uWeaponIndex < m_uNumWeaponCtrls) { return m_paWeaponCtrl+uWeaponIndex;} else return NULL;}
	
	//
	//  Info about the entity being controlled!
	//
	FINLINE CEntity*		GetEntity(void)								{ return m_pEntity;}
	FINLINE const CFVec3A&	GetLoc(void) const;
	FINLINE f32				GetRadius(void) const;
	FINLINE f32				GetRadiusXZ(void) const;
	FINLINE f32				GetHeight(void) const;
	FINLINE f32				GetTallHeight(void) const;
	FINLINE f32				GetMinHeight(void) const;
	FINLINE s16				GetCurRoomId(void)							{ return m_nRoomId;}
	FINLINE CEntity*		GetAvoidThis(void)							{ return m_pAvoidThis;}  //get the entity that this object is currently avoiding, NULL if none
	FINLINE void			SetHeadLookGoal_WS(const CFVec3A& Pt_WS);
	FINLINE const CFVec3A&	GetHeadLookGoal_WS(void)					{ return m_HeadLookGoal_WS;}
	FINLINE f32				GetMoveSpeed(void);
	FINLINE const CFVec3A&	GetMoveDirXZ(void);
	FINLINE f32				GetMoveSpeedXZ(void);
	FINLINE BOOL			HasTargetLock(void);
	const CFVec3A &			GetEyeLookAt(void);
	const CFVec3A &			GetEyePos(void);
	FINLINE const CFVec3A&	GetTorsoLookAtXZ(void)						{ return m_TorsoLookAtXZ;}
	FINLINE const CFVec3A&	GetTorsoLookAt(void);
	FINLINE const CFVec3A&  GetEarLoc(void) const;
	FINLINE CBot*			GetCurMech(void);


	FINLINE BOOL		IsStuck(f32 fStuckForSecs)						{ return (BOOL)(m_fStuckForSecs > fStuckForSecs);}
	FINLINE BOOL		IsStopped(void)									{ return (BOOL)(m_nClientFeedback & FEEDBACK_STOPPED);}
	FINLINE BOOL		IsPathCompleted(void)							{ return (BOOL)(m_nClientFeedback & FEEDBACK_PATH_COMPLETE);}
	FINLINE BOOL		IsDeadOrDying(void);

	void				GetPathFindingOrigin(CFVec3A* pOrigin);

	FINLINE void		SetLastGraphVertId(u16 uLastGraphVertId)		{ m_uLastGraphVertId = uLastGraphVertId;}
	FINLINE void		SetLastGraphEdgeId(u16 uLastGraphEdgeId)		{ m_uLastGraphEdgeId = uLastGraphEdgeId;}

	FINLINE u16 		GetLastGraphVertId(void)						{ return m_uLastGraphVertId;}
	FINLINE u16 		GetLastGraphEdgeId(void)						{ return m_uLastGraphEdgeId;}

//protected

	void				UpdateMoverCaps(void);

	//speed controls
	void				SetForwardGoalSpeedPctCapable(f32 fScale);
	void				SetForwardGoalSpeed(f32 fGoalSpeed);
	void				SetForwardAccDcc(f32 fAcc, f32 fDcc)			{ m_fForwardMaxAcc = fAcc, m_fForwardMaxDcc = fDcc;}
	f32					GetForwardGoalSpeed(void)						{ return m_fForwardGoalSpeed;}
	f32					GetForwardSpeed(void)							{ return m_fForwardGoalSpeed;}
	f32					CalcSpeedPctFromAction(f32 fGoalSpeed);

	void				SetMaxXZTurnSpeedPct(f32 fMaxPct)				{ m_fMaxXZTurnSpeedPct = fMaxPct;};
	f32					GetMaxXZTurnSpeedPct(void)						{ return m_fMaxXZTurnSpeedPct;};

	//movement style controls
	void				SetMovementStyle(	f32 fAlgnmntVelConstraint, f32 fAlgnmntSpeedConstraint,	BOOL bCanOverrideConstraint);


	CFVec3A				m_TorsoLookAtXZ;
	CFVec3A				m_TorsoLookAt;
	CFVec3A				m_HeadLookGoal_WS;						//a place for the head to look at if it is reasonable based on torso direction	CFVec3A
	CEntity*			m_pEntity;
	CAIControlGoals		m_Controls;								//Entity this mover is controlling should be made to point to this when under control AI control
	s16					m_nRoomId;
	u16					m_uMoverCaps;							//see MOVERCAP_*
	f32					m_fForwardGoalSpeed;
	f32					m_fForwardMaxAcc;
	f32					m_fForwardMaxDcc;
	f32					m_fMaxXZTurnSpeedPct;
	f32					m_fTorsoAlignmentSpeedConstraint;	   //0.0f-1.0f How much speed is limited by poor "forward" torso alignment
	f32					m_fTorsoAlignmentVelConstraint;		   //0.0f-1.0f How much the direction of movement is limited by poor "forward" torso alignment
	u32					m_uMoverFlags;
	CAIPath				*m_pPath;								//Path the mover is currently following. NULL if none.
	u32					m_uAssignPathFlags;						//any flags related to the current path
	CAIPathWalker		m_PathWalker;							//if following a path, this tracks progress via waypoints
	//data for keeping track of the current command
	u32					m_nClientFeedback;
	f32					m_fLinkStartTime;
	f32					m_fLinkEstimateTime;	// estimate of how long it should take to get down the current link.. based on bot max speed
	f32					m_fStuckForSecs;		// How long mover has been stopped
	f32					m_fUnStuckForSecs;		// How long since mover got unstuck

	//Vbls this mover needs in order to avoid things
	CEntity*			m_pAvoidThis;	     // the entity that I most need to avoid
	f32					m_fLastAvoidSteeringChange;
	f32					m_fLastOverrideSteeringChange;
	u16					m_uLastGraphVertId;
	u16					m_uLastGraphEdgeId;

	static CNiList<CEntity*>	m_AvoidEntityList;	 // list of the candidates for me to avoid (Filled and Cleared withing the context of CAIMover::AvoidObstacles)
	
	//Weapon
	CAIWeaponCtrl*		m_paWeaponCtrl;
	u8					m_uNumWeaponCtrls;

	FCLASS_STACKMEM_ALIGN( CAIMover );	   //potential findfix:  can't put this macro on interface only classes?
} FCLASS_ALIGN_SUFFIX;

#if FANG_ENABLE_INLINE_CODE
#include "AIMover.inl"
#endif


//
//   Bonus functions!
//
extern f32 AIMover_DistApartXZ(const CAIMover* pA, const CAIMover* pB);
extern f32 AIMover_DistApart(const CAIMover* pA, const CAIMover* pB);



#endif //_AIMOVER_H_

