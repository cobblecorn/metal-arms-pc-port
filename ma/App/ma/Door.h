//////////////////////////////////////////////////////////////////////////////////////
// Door.h - Door object.
//
// Author: Justin Link
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
// Date		Who			Description
// --------	----------	--------------------------------------------------------------
// 04/22/02	Link		Created.
//   03/24/03 pgm		Made elevators revers directions if a bot is underneath them
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DOOR_H_
#define _DOOR_H_ 1

#include "fang.h"
#include "meshentity.h"

struct FVisPortal_t;
class CFAnimManMtx;
class CFAnimMeshRest;
class CFAnimCombiner;


class CFVec3AObj;

FCLASS_ALIGN_PREFIX class CDoorEntity : public CMeshEntity
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	/////////////////////////////////////////////////////////////
	// User type (specified in the user properties).
	enum UserType_e
	{
		USERTYPE_DOOR,
		USERTYPE_LIFT,
		USERTYPE_COUNT
	};

	/////////////////////////////////////////////////////////////
	// (local).
	enum BoneMoveType_e
	{
		MOVETYPE_NONE,
		MOVETYPE_BONETRANS,
		MOVETYPE_ANIMATION,
		MOVETYPE_COUNT
	};

	/////////////////////////////////////////////////////////////
	// Describes 
	//   a whole.			
	enum DoorMoveType_e
	{
		DOORMOVETYPE_NONE,
		DOORMOVETYPE_LINE,		  //pgm: pretty sure these are for lifts
		DOORMOVETYPE_VECOBJ,	  //pgm: pretty sure these are for lifts
		DOORMOVETYPE_COUNT
	};

	/////////////////////////////////////////////////////////////
	// Describes the action that the door is currently undertaking.
	enum ActionState_e
	{
		DOORSTATE_ZERO = 0,
		DOORSTATE_ONE,
		DOORSTATE_ZEROTOONE,
		DOORSTATE_ONETOZERO,
		DOORSTATE_COUNT
	};

	//how many places can a lift stop?
	enum
	{
		LIFTSTOP_ZERO = 0,
		LIFTSTOP_ONE,
		NUM_LIFTSTOPS
	};

	/////////////////////////////////////////////////////////////
	// How the door acts and reacts.
	enum DoorBehavior_e
	{
		DOOR_BEHAVIOR_DOOR = 0,			// Door goes to DOORSTATE_ONE when a bot is
										//   in its 'inner region', and goes to DOORSTATE_ZERO
										//   when no one is in its 'region'.
		DOOR_BEHAVIOR_OSCILLATE,		// Continually switches its position back
										//   and forth.
		DOOR_BEHAVIOR_ELEVATOR,			// If someone enters larger active region, go to
										//   that lift stop, if someone enters the smaller
										//   active region, go to the other position.
		DOOR_BEHAVIOR_OTHER,			// Doesn't take any action of its own
										//   initiative.
		DOOR_BEHAVIOR_COUNT
	};


	enum
	{
		DOORBONE_ZERO = 0,
		DOORBONE_ONE,
		NUM_DOORBONES,
	};

	/////////////////////////////////////////////////////////////
	// Flags indicating for whom the door opens when it's type is
	//   'door'.
	enum
	{
		DOOR_ACCESS_NONE	=	0,
		DOOR_ACCESS_PLAYER	=	1 << 0,
		DOOR_ACCESS_NPC		=	1 << 1,
	};

	/////////////////////////////////////////////////////////////
	// Params that go with certain behaviors
	enum
	{
		DOOR_BEHAVIOR_CTRL_AUTOCLOSE				=	1 << 0,		// Door will automaticall close(go to state 0) after specified amount of time
		DOOR_BEHAVIOR_CTRL_OPENFRONT				=	1 << 1,		// Door will open automatically if it detects someone in front of it.
		DOOR_BEHAVIOR_CTRL_OPENBACK					=	1 << 2,		// Door will open automatically if it detects someone behind it.
		DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL0		=	1 << 3,		// Elevator will not automatically get called from state 0.
		DOOR_BEHAVIOR_CTRL_DISABLE_AUTO_CALL1		=	1 << 4,		// Elevator will not automatically get called from state 1.
	};

	/////////////////////////////////////////////////////////////
	// Reason why a gotopos was issued.
	enum GotoReason_e
	{
		GOTOREASON_UNKNOWN = 0,					// We don't know why the gotopos was issued.
		GOTOREASON_PICKUP,						// We want to pick up somebody.  The difference of this from above is that
												//   we will wait a few seconds for someone to board before we will respond
												//   to any other requests.
		GOTOREASON_DESTINATION,					// We want to take someone somewhere.
		GOTOREASON_PATHBLOCKED,					// The Path was blocked, so lift hat to abort the last goto

		GOTOREASON_COUNT
	};

	enum
	{
		CHECK_FOR_BOTS_IN_FRONTANDBACK = 0,
		CHECK_FOR_BOTS_IN_FRONT,
		CHECK_FOR_BOTS_IN_BACK,
	};


	enum
	{
		ACTIVE_REGION_INNER,
		ACTIVE_REGION_OUTER,
		NUM_ACTIVE_REGIONS,
	};

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	u32 m_uBehaviorCtrlFlags;
	GotoReason_e m_eCurGotoReason;
	f32 m_fPickupCntDn;
	static const f32 m_afDefaultActiveRadius[NUM_ACTIVE_REGIONS];
	static CDoorEntity *m_pDoorListHead;		// pgm: Global list of all doors
	CDoorEntity *m_pDoorListNext;				// Next in the list of CDoorEntity's.

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bSystemInitialized;
	static u32 s_uAccessFlags;					// -> callback.
	static u32 s_uFrontBackFilter;				// -> callback.
	static CFVec3A s_vecDoorZ, s_vecDoorPos;	// -> callback.
	
	// bunched the aligned members up so that we waste less space to alignment
	CFVec3A m_vecDoorMove;
	CFMtx43A m_mtxDoorInitPos;
	CFVec3A m_avecBoneTrans[NUM_DOORBONES];					// [Bone]
	CFVec3A m_avecAIPts[NUM_LIFTSTOPS];			 //pgm: I think AI pts are pickup pts

	f32 m_afActiveRadius[NUM_ACTIVE_REGIONS];	// pgm: two radii (inner and outer) 

	// Describes the motion(pgm: aka displacement) of the door as a whole.
	DoorMoveType_e m_eDoorMoveType;
	CFVec3AObj *m_apV3OPath[2];
	f32 m_fRotateY;
	f32 m_fRotateZ;
	
	UserType_e m_eUserType;
	u32 m_uNumAIPts;
	CEntity *m_apAISwitch[2];

	DoorBehavior_e m_eBehavior;
	u32 m_uAccessFlags;

	// These are valid only if the appropriate brain mod flags are set.
	f32 m_fAutoCloseDelay;

	// Occupied means 
	BOOL m_bUnoccupiedLastFrame;

	ActionState_e m_eState;
	BOOL m_bLocked;
	BOOL m_bStartOpen;
	f32 m_fUnitPos;
	f32 m_fUnitPosMapped;
	f32 m_fTimeOpen;
	u32 m_uPos;

	// Config data.
	f32 m_afUnitPosRate[2];
	f32 m_afPosTime[2];		// 0: 0->1, 1: 1->0
	s32 m_anMeshIdx[2];		// 0: 0, 1: 1
	u32 m_anEvent[2];		// 0: ->0, 1: ->1
	// TODO: Make these a bitfield.
	BOOL m_bUseAccelMap;			// Whether or not unit position get mapped to something else.
	BOOL m_bIgnoreSteam;
	BOOL m_bIsPortal;
	FVisPortal_t *m_pPortal;

	// Used for adjusting the bounding sphere.
	f32 m_fClosedRadius;
	f32 m_fRadiusDelta;

	// Determines what to do when changing positions.
	BoneMoveType_e m_eBoneMoveType;

	// Only used when open-type is bone translation.
	CFAnimManMtx *m_pManMtx;
	CFAnimMeshRest *m_pRestMtx;
	CFAnimCombiner *m_pAnimCombiner;
	
	// Only used when open-type is animation.
	f32 m_afAnimSpeedMult[2];

	// Used for the steam.
	u32 m_uNumSteamBones;
	s32 m_anSteamBoneIdx[5];					// [Bone]

	// Used for both doors AND lifts
	FSndFx_FxHandle_t m_hSoundOpenStart;
	FSndFx_FxHandle_t m_hSoundOpenStop;
	FSndFx_FxHandle_t m_hSoundOpenLoop;
	CFAudioEmitter *m_pSoundOpenLoopEmitter;

	// Used for doors ONLY
	FSndFx_FxHandle_t m_hSoundCloseStart;
	FSndFx_FxHandle_t m_hSoundCloseStop;
	FSndFx_FxHandle_t m_hSoundCloseLoop;
	CFAudioEmitter *m_pSoundCloseLoopEmitter;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CDoorEntity();
	virtual ~CDoorEntity();
	BOOL Create();			// Boring, dumb create used for debugging purposes.

	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();
	static BOOL IsInitialized() { return(m_bSystemInitialized); }
	static CDoorEntity *AI_FindWithNearestPad(const CFVec3A &vecPos, UserType_e eUserType);

	// Commands.
	BOOL GotoPos(u32 uNewPos, GotoReason_e eGotoReason);		  //pgm:this is more of a request and there are various reasons why it might get tossed.
	BOOL ForceGotoPos(u32 uNewPos, GotoReason_e eGotoReason);	//Works regardless of current state
	void SnapToPos(u32 uNewPos);	 //called when lift/door goes to either one, or zero 

	// Being in the locked state means that the door will not respond to
	//   *anything* until it is unlocked.
	// It will, however, complete whatever action it may have been
	//   undertaking prior to its being locked.
	void SetLockState(BOOL bLocked);
	FINLINE BOOL IsLocked(void) { return (m_bLocked); }

	/////////////////////////////////////////////////////////////
	// State querying.

	// The value returned by GetUnitPos is temporal.
	f32 GetUnitPos();
	// The value returned by GetUnitPos mapped is spatial.
	f32 GetUnitPosMapped();
	FINLINE ActionState_e GetState() { return(m_eState); }
	BOOL AI_IsOpen(const CFVec3A& RelateToPos);		 //since lifts are doors too, their "openness" is relative to the end points of the lift's movement
	BOOL AI_IsOpening(const CFVec3A& FromWhere);
	FINLINE f32 AI_GetOnRadiusXZ(void) { return m_afActiveRadius[0];}
	void AI_HoldAtEndLiftFor(f32 fSecs);

	BOOL IsOccupied(void);
	FINLINE CEntity *AI_GetSwitch(u32 uSwitchNum) { return m_apAISwitch[uSwitchNum]; }
	FINLINE DoorBehavior_e GetBehavior() { return(m_eBehavior); }

	// This returns the unit position endpoint for which the CDoorEntity is closest
	//    to the specified position.
	f32 AI_GetUnitTargetPos(const CFVec3A &vecPos);

	FINLINE const CFMtx43A &AI_GetInitPos() { return(m_mtxDoorInitPos); }
	FINLINE const CFVec3A &AI_GetMoveVec() { return(m_vecDoorMove); }

	// checkpoint load/save functions
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	void DebugRender(void);

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	virtual void ClassHierarchyDestroy();
	virtual BOOL ClassHierarchyBuild();
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder();

	virtual void ClassHierarchyResolveEntityPointerFixups();
	virtual void ClassHierarchyWork();

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	// Helper functions.
	void Clear();
	void _SetDefaults();

	// uFrontBackFilter = 1 -> front only, 2 -> back only, 0 -> both sides.
	BOOL _ValidBotNearby(CFVec3A &vecPos, f32 fRadius, u32 uFrontBackFilter);

	// Action state.
	void StateTransitions();
	void StateWork();


	enum
	{
		NORMAL_ADVANCE = FALSE,
		UNDO_LAST_ADVANCE = TRUE,
	};
	void AdvanceUnitPos(BOOL bRevert = FALSE);

	// Behavior specific work.
	void DoBehaviorDoor();
	void DoBehaviorOscillate();
	void DoBehaviorOther();
	void DoBehaviorElevator();

	void AutoClose();
	void AutoOpenFront();
	void AutoOpenBack();

	// Door state movement.
	void StateMovement();

	// Specific door state movement.
	void DoBoneMovement();
	void DoAnimMovement();

	// For the steam bones.
	void SpawnSteam();
	void FindSteamBoneIndices();

	void ConvertUnitPos(u32 uOldState, u32 uNewState);
	f32 ConvertUnitPos(u32 uOldState, u32 uNewState, f32 fOldUnitPos);

	static BOOL BotCheckCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL LiftPathBlockedCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	enum
	{
		NO_COLLISION_TEST = 0,
		DO_COLLISION_TEST,
	};
	BOOL _UpdateLineDoorPosition( BOOL bCheckCollision );

	void StopDoorLoopSounds( void );

	FCLASS_STACKMEM_ALIGN(CDoorEntity);
} FCLASS_ALIGN_SUFFIX;




#endif
