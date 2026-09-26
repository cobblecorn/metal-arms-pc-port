#ifndef _AIBRAIN_H_
#define _AIBRAIN_H_ 1

//////////////////////////////////////////////////////////////////////////////////////
// AIBrain.h 
//  classes:
//		CAIBrain - Thought State machine, and AIMover work function dispatcher for AI 
//                 units.       
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
// 03/14/02 MacKellar      Created.
// 04/14/02	MacKellar	   Changed to be more Generic form (AIBrain)
//                         Changed CAIBrainState to CAIThought and moved to AIThought.h
//////////////////////////////////////////////////////////////////////////////////////


#include "fmath.h"
#include "ftl.h"
#include "AIThought.h"	 //for ThoughtBank_AccessFunc
#include "AIKnowledge.h"						 
#include "AIEnviro.h"  //for CAISound
#include "AIFormations.h"

class CAIMover;
class CAIBrain;
class CPendingThought;
class CEntityBuilder;
class CDamageResult;
class CBot;
class CAIFormation;

typedef CNiList<CAIBrain*> BrainPtrList;
typedef CNiList<CPendingThought*> PendingList;

FCLASS_NOALIGN_PREFIX class CPendingThought
{
public:
	CAIMemorySmall* m_pParamPack;
	s8 m_nThoughtType;
	u8 m_uThoughtFlags;

	FCLASS_STACKMEM_NOALIGN(CPendingThought);
} FCLASS_NOALIGN_SUFFIX;


FCLASS_ALIGN_PREFIX class CAIBrain
{
public:
	CAIBrain(void);
	~CAIBrain(void);

	// Job Thought Types
	enum 
	{
		TT_INVALID = -2,						// unititialized data or something.
		TT_OFF = -1,							// Brain is in slave mode.  that means Perception, Knowledge and Decision Only. (No Action!)
		TT_WAIT = 0,							// Stay in one place, looking around
		TT_WANDER,								// Wander path finding graph
		TT_PATROL,								// Patrol a path
	};

	// Goal Thought Types
	enum
	{
		TT_FOLLOW = TT_PATROL+1,				// follow another brain
		TT_COMBAT,								// combat mode
		TT_SEARCH,								// search for something
		TT_GOTO,								// goto somewhere
		TT_FACEIT,								// orient to look at something
		TT_TALK,								// talk (using BotTalk module)
		NUM_THOUGHT_TYPES,
	};

	enum
	{
		TT_MAX_JOB_TYPE = TT_PATROL,
		TT_MAX_GOAL_TYPE = NUM_THOUGHT_TYPES-1,
	};

	void					Create(CAIMover *pBotMover);
	BOOL					InitFromBuilder(CEntityBuilder* pBuilder);
	void					AddToWorld(void);
	void					RemoveFromWorld(void);
	void					ClearBrain(void);
	void					Work(void); // Perception->Knowledge->Decision->Action

	// checkpoint save/restore functions
	BOOL					CheckpointSave(void);
	void					CheckpointRestore(void);
	void					CheckpointRestorePostamble(void);

	// call ConnectState() to indicate that the specified state (nStateId)
	// is implemented for this brain.  And, this brain should use PBankAccessFunc to
	// get and recycle state objects whenever it needs
	FINLINE void			ConnectThought(s16 nThoughtId, ThoughtBank_AccessFunc pBankAccessFunc)	{ m_apThoughtBankAccessFunc[nThoughtId] = pBankAccessFunc;}
	FINLINE BOOL			IsThoughtConnected(s16 nThoughtId)										{ return m_apThoughtBankAccessFunc[nThoughtId] != NULL;}
	
	BOOL					AssignJob(s8 nThoughtType, CAIMemorySmall* pParamPack = NULL);		 // will onlyc cause current thought to change if current thought is a job
	BOOL					AssignGoal(s8 nThoughtType, CAIMemorySmall* pParamPack = NULL);		 // will definitely cause current thought to change
	BOOL					AssignReact(s8 nThoughtType, CAIMemorySmall* pParamPack = NULL);	 // will definitely cause current react to change
	BOOL					AssignTalk(CAIMemorySmall* pParamPack = NULL);						 // will definitely cause current talk thought to change

	//leader, follower
	BOOL					AssignLeader(CAIBrain* pLeader);
	BOOL					AssignFollower(CAIBrain* pFollower);
	BOOL					RemoveFollower(CAIBrain* pFollower);
	BOOL					CanFollow(CAIBrain* pLeader);
	FINLINE BOOL			CanLead(CAIBrain* pFollower)	{ return TRUE;}
	FINLINE u32				GetNumFollowers(void)			{ return (u32)m_FollowerList.Size();}
	FINLINE BOOL			IsFollowing(void)				{ return m_pLeader!=NULL;}
	FINLINE BOOL			IsLeading(void)					{ return (m_FollowerList.Size() != 0);}
	s32						GetFollowerRank(u32 *puOutOfHowMany = NULL);	//if this unit is a follower, what is its "rank" according to its leader
	CAIPost*				GetFollowerPost(void);			//if this unit is a follower, what is its post
	void					StopFollowing(void);
	void					StopLeading(void);
	void					BootUnhappyFollowers(void);
	void					CleanupActiveThoughts(void);


	BOOL					ForceFailureOnAllThoughtsExceptJob(void);
	FINLINE s8				GetCurThought(void)				{ return m_nCurThought;}
	FINLINE s8				GetLastThought(void)			{ return m_nLastThought;}
	FINLINE s8				GetJobThought(void)				{ return m_nJobThought;}
	FINLINE CAIThought		*GetCurThoughtPtr(void)			{ return m_pCurThought;}
	FINLINE CAIThought		*GetTalkPtr(void)				{ return m_pCurTalk;}
	FINLINE CAIThought		*GetReactPtr(void)				{ return m_pCurReact;}
	FINLINE CAIThought		*GetCurFollowerThoughtPtr(void)	{ return m_pCurFollowerThought;}

	FINLINE CAIMover		*GetAIMover(void)				{ return m_pMover;}
	FINLINE u16				GetGUID(void)					{ return m_uBrainGUID;}
	const CFVec3A&			GetLoc(void);
	FINLINE CAIKnowledge&	GetKnowledge(void)				{ return m_Knowledge;}
	FINLINE BOOL			HasTalkThought(void)			{ return m_pCurTalk!=NULL;}	 //either about to, or is, or is finishing up the playing of a bottalk
	BOOL					IsTalking(void);
	FINLINE BOOL			IsReacting(void)				{ return m_pCurReact!=NULL;}

	enum
	{
		BRAINFLAG_NONE								= 0x00000000,
		BRAINFLAG_ACTIVE							= 0x00000001,
		BRAINFLAG_SCAN								= 0x00000002,
		BRAINFLAG_TRACKSOUNDS_HEAD					= 0x00000004,
		BRAINFLAG_TRACKSOUNDS_TORSO					= 0x00000008,
		BRAINFLAG_SCAN_DIR							= 0x00000010,
		BRAINFLAG_MECH_CTRL_INIT_FAILED				= 0x00000020,
		BRAINFLAG_BUDDY_CTRL_AUTO					= 0x00000040,
		BRAINFLAG_BUDDY_CTRL_ONACTION				= 0x00000080,
		BRAINFLAG_ATTACKS_ENEMY						= 0x00000100,
		BRAINFLAG_ATTACKS_NEUTRAL					= 0x00000200,
		BRAINFLAG_ATTACKS_FRIENDLY					= 0x00000400,
		BRAINFLAG_ATTACKS_PLAYERS					= 0x00000800,
		BRAINFLAG_ATTACKS_NPC						= 0x00001000,
		BRAINFLAG_NEW_INFREQ_RAND_TIMER				= 0x00002000,
		BRAINFLAG_DISABLE_ENTITY_WORK				= 0x00004000,
		BRAINFLAG_NEVER_UNARMED						= 0x00008000,
		BRAINFLAG_WAS_MOVING						= 0x00010000,
		BRAINFLAG_JUST_STOPPED						= 0x00020000,
		BRAINFLAG_FOLLOWER_DOESNT_NEED_POST			= 0x00040000,
		BRAINFLAG_REQUESTED_MECH_ENTRY				= 0x00080000,
		BRAINFLAG_REQUESTED_MECH_EXIT				= 0x00100000,
		BRAINFLAG_BUDDY_CTRL_NOWEAPONSTOW			= 0x00200000,

		BRTAINFLAG_ONESTOSAVE = (	BRAINFLAG_BUDDY_CTRL_AUTO | 
								BRAINFLAG_BUDDY_CTRL_ONACTION | 
								BRAINFLAG_ATTACKS_ENEMY | 
								BRAINFLAG_ATTACKS_NEUTRAL | 
								BRAINFLAG_ATTACKS_FRIENDLY | 
								BRAINFLAG_ATTACKS_PLAYERS | 
								BRAINFLAG_ATTACKS_NPC |
								BRAINFLAG_NEVER_UNARMED |
								BRAINFLAG_BUDDY_CTRL_NOWEAPONSTOW),

	};
	FINLINE void SetFlag_Active(void)						{ m_uBrainFlags |= BRAINFLAG_ACTIVE;}
	FINLINE u32 GetFlag_Active(void)						{ return (m_uBrainFlags & BRAINFLAG_ACTIVE);}
	FINLINE void ClearFlag_Active(void)						{ m_uBrainFlags &= ~BRAINFLAG_ACTIVE;}

	FINLINE void SetFlag_Scan(void)							{ m_uBrainFlags |= BRAINFLAG_SCAN;}
	FINLINE u32 GetFlag_Scan(void)							{ return (m_uBrainFlags & BRAINFLAG_SCAN);}
	FINLINE void ClearFlag_Scan(void)						{ m_uBrainFlags &= ~BRAINFLAG_SCAN;}

	FINLINE void SetFlag_DisableEntityWork(void)			{ m_uBrainFlags |= BRAINFLAG_DISABLE_ENTITY_WORK;}
	FINLINE u32 GetFlag_DisableEntityWork(void)				{ return (m_uBrainFlags & BRAINFLAG_DISABLE_ENTITY_WORK);}
	FINLINE void ClearFlag_DisableEntityWork(void)			{ m_uBrainFlags &= ~BRAINFLAG_DISABLE_ENTITY_WORK;}

	FINLINE void SetFlag_NeverUnarmed(void)					{ m_uBrainFlags |= BRAINFLAG_NEVER_UNARMED;}
	FINLINE u32 GetFlag_NeverUnarmed(void)					{ return (m_uBrainFlags & BRAINFLAG_NEVER_UNARMED);}
	FINLINE void ClearFlag_NeverUnarmed(void)				{ m_uBrainFlags &= ~BRAINFLAG_NEVER_UNARMED;}

	FINLINE void SetFlag_TrackSounds_Head(void)				{ m_uBrainFlags |= BRAINFLAG_TRACKSOUNDS_HEAD;}
	FINLINE u32 GetFlag_TrackSounds_Head(void)				{ return (m_uBrainFlags & BRAINFLAG_TRACKSOUNDS_HEAD);}
	FINLINE void ClearFlag_TrackSounds_Head(void)			{ m_uBrainFlags &= ~BRAINFLAG_TRACKSOUNDS_HEAD;}

	FINLINE void SetFlag_TrackSounds_Torso(void)			{ m_uBrainFlags |= BRAINFLAG_TRACKSOUNDS_TORSO;}
	FINLINE u32 GetFlag_TrackSounds_Torso(void)				{ return (m_uBrainFlags & BRAINFLAG_TRACKSOUNDS_TORSO);}
	FINLINE void ClearFlag_TrackSounds_Torso(void)			{ m_uBrainFlags &= ~BRAINFLAG_TRACKSOUNDS_TORSO;}
	
	FINLINE void SetFlag_MechUseageCtrlInitFailed(void)		{ m_uBrainFlags |= BRAINFLAG_MECH_CTRL_INIT_FAILED;}
	FINLINE u32 GetFlag_MechUseageCtrlInitFailed(void)		{ return (m_uBrainFlags & BRAINFLAG_MECH_CTRL_INIT_FAILED);}
	FINLINE void ClearFlag_MechUseageCtrlInitFailed(void)	{ m_uBrainFlags &= ~BRAINFLAG_MECH_CTRL_INIT_FAILED;}

	FINLINE void SetFlag_Buddy_Ctrl_Auto(void)					{ m_uBrainFlags |= BRAINFLAG_BUDDY_CTRL_AUTO;}
	FINLINE u32 GetFlag_Buddy_Ctrl_Auto(void)					{ return (m_uBrainFlags & BRAINFLAG_BUDDY_CTRL_AUTO);}
	FINLINE void ClearFlag_Buddy_Ctrl_Auto(void)				{ m_uBrainFlags &= ~BRAINFLAG_BUDDY_CTRL_AUTO;}

	FINLINE void SetFlag_Buddy_Ctrl_OnAction(void)				{ m_uBrainFlags |= BRAINFLAG_BUDDY_CTRL_ONACTION;}
	FINLINE u32 GetFlag_Buddy_Ctrl_OnAction(void)				{ return (m_uBrainFlags & BRAINFLAG_BUDDY_CTRL_ONACTION);}
	FINLINE void ClearFlag_Buddy_Ctrl_OnAction(void)			{ m_uBrainFlags &= ~BRAINFLAG_BUDDY_CTRL_ONACTION;}

	FINLINE void SetFlag_Buddy_Ctrl_NoWeaponStow(void)				{ m_uBrainFlags |= BRAINFLAG_BUDDY_CTRL_NOWEAPONSTOW;}
	FINLINE u32 GetFlag_Buddy_Ctrl_NoWeaponStow(void)				{ return (m_uBrainFlags & BRAINFLAG_BUDDY_CTRL_NOWEAPONSTOW);}
	FINLINE void ClearFlag_Buddy_Ctrl_NoWeaponStow(void)			{ m_uBrainFlags &= ~BRAINFLAG_BUDDY_CTRL_NOWEAPONSTOW;}

	FINLINE void SetFlag_Follower_Doesnt_Need_Post(void)		{ m_uBrainFlags |= BRAINFLAG_FOLLOWER_DOESNT_NEED_POST;}
	FINLINE u32 GetFlag_Follower_Doesnt_Need_Post(void)			{ return (m_uBrainFlags & BRAINFLAG_FOLLOWER_DOESNT_NEED_POST);}
	FINLINE void ClearFlag_Follower_Doesnt_Need_Post(void)		{ m_uBrainFlags &= ~BRAINFLAG_FOLLOWER_DOESNT_NEED_POST;}

	FINLINE void SetFlag_Brainflag_Requested_Mech_Entry(void)	{ m_uBrainFlags |= BRAINFLAG_REQUESTED_MECH_ENTRY;}
	FINLINE u32 GetFlag_Brainflag_Requested_Mech_Entry(void)	{ return (m_uBrainFlags & BRAINFLAG_REQUESTED_MECH_ENTRY);}
	FINLINE void ClearFlag_Brainflag_Requested_Mech_Entry(void)	{ m_uBrainFlags &= ~BRAINFLAG_REQUESTED_MECH_ENTRY;}

	FINLINE void SetFlag_Brainflag_Requested_Mech_Exit(void)	{ m_uBrainFlags |= BRAINFLAG_REQUESTED_MECH_EXIT;}
	FINLINE u32 GetFlag_Brainflag_Requested_Mech_Exit(void)		{ return (m_uBrainFlags & BRAINFLAG_REQUESTED_MECH_EXIT);}
	FINLINE void ClearFlag_Brainflag_Requested_Mech_Exit(void)	{ m_uBrainFlags &= ~BRAINFLAG_REQUESTED_MECH_EXIT;}

	FINLINE void SetFlag_ScanDir(void)						{ m_uBrainFlags |= BRAINFLAG_SCAN_DIR;}
	FINLINE u32 GetFlag_ScanDir(void)						{ return (m_uBrainFlags & BRAINFLAG_SCAN_DIR);}
	FINLINE void ClearFlag_ScanDir(void)					{ m_uBrainFlags &= ~BRAINFLAG_SCAN_DIR;}
	FINLINE void ToggleFlag_ScanDir(void)					{ m_uBrainFlags ^= BRAINFLAG_SCAN_DIR;}

	FINLINE u32 GetFlag_Just_Stopped(void)					{ return m_uBrainFlags & BRAINFLAG_JUST_STOPPED;}

	FINLINE u16 GetAttackNotifyRadius(void)					{ return m_uAttackNotifyRadius;}
	FINLINE f32 GetBrainAlertUnit(void)						{ return m_fBrainAlertUnit;}

	FINLINE void SetFlag_AttackEnemy(void)					{ m_uBrainFlags |= BRAINFLAG_ATTACKS_ENEMY; }
	FINLINE u32 GetFlag_AttackEnemy(void)					{ return (m_uBrainFlags & BRAINFLAG_ATTACKS_ENEMY); }
	FINLINE void ClearFlag_AttackEnemy(void)				{ m_uBrainFlags &= ~BRAINFLAG_ATTACKS_ENEMY; }

	FINLINE void SetFlag_AttackNeutral(void)				{ m_uBrainFlags |= BRAINFLAG_ATTACKS_NEUTRAL; }
	FINLINE u32 GetFlag_AttackNeutral(void)					{ return (m_uBrainFlags & BRAINFLAG_ATTACKS_NEUTRAL); }
	FINLINE void ClearFlag_AttackNeutral(void)				{ m_uBrainFlags &= ~BRAINFLAG_ATTACKS_NEUTRAL; }

	FINLINE void SetFlag_AttackFriendly(void)				{ m_uBrainFlags |= BRAINFLAG_ATTACKS_FRIENDLY; }
	FINLINE u32 GetFlag_AttackFriendly(void)				{ return (m_uBrainFlags & BRAINFLAG_ATTACKS_FRIENDLY); }
	FINLINE void ClearFlag_AttackFriendly(void)				{ m_uBrainFlags &= ~BRAINFLAG_ATTACKS_FRIENDLY; }

	FINLINE void SetFlag_AttackPlayer(void)					{ m_uBrainFlags |= BRAINFLAG_ATTACKS_PLAYERS; }
	FINLINE u32 GetFlag_AttackPlayer(void)					{ return (m_uBrainFlags & BRAINFLAG_ATTACKS_PLAYERS); }
	FINLINE void ClearFlag_AttackPlayer(void)				{ m_uBrainFlags &= ~BRAINFLAG_ATTACKS_PLAYERS; }

	FINLINE void SetFlag_AttackNPC(void)					{ m_uBrainFlags |= BRAINFLAG_ATTACKS_NPC; }
	FINLINE u32 GetFlag_AttackNPC(void)						{ return (m_uBrainFlags & BRAINFLAG_ATTACKS_NPC); }
	FINLINE void ClearFlag_AttackNPC(void)					{ m_uBrainFlags &= ~BRAINFLAG_ATTACKS_NPC; }

	enum
	{
		MECH_USEAGE_CTRL_NEVER  = 0,
		MECH_USEAGE_CTRL_AUTO,
		MECH_USEAGE_CTRL_INIT,
	};
	enum
	{
		MECH_USEAGE_TYPE_FLAG_NONE		= 0x00,			//this brain uses no Mech types, should have MECH_USEAGE_CTRL_NEVER
		MECH_USEAGE_TYPE_FLAG_PILL		= 0x01,			//this brain can use pillboxes
		MECH_USEAGE_TYPE_FLAG_LOADER	= 0x02,			//this brain can use loaders
		MECH_USEAGE_TYPE_FLAG_SENTINEL	= 0x04,			//this brain can use sentinels
		MECH_USEAGE_TYPE_FLAG_RAT		= 0x08,			//this brain can drive rats
		MECH_USEAGE_TYPE_FLAG_RATGUN	= 0x10,			//this brain can use rat guns
		MECH_USEAGE_TYPE_ALL			= MECH_USEAGE_TYPE_FLAG_PILL | MECH_USEAGE_TYPE_FLAG_LOADER | MECH_USEAGE_TYPE_FLAG_SENTINEL | MECH_USEAGE_TYPE_FLAG_RAT | MECH_USEAGE_TYPE_FLAG_RATGUN,
	};
	FINLINE u8 GetMechUseageRadius(void)										{ return m_uMechUseageRadius;}
	FINLINE u8 GetMechUseageCtrl(void)											{ return m_uMechUseageCtrl;}
	FINLINE u8 GetMechUseageTypeFlag(void)										{ return m_uMechUseageTypeFlags;}
	FINLINE void SetMechUseage(u8 uMechUseageCtrl, u8 uMechUseageTypeFlags, u8 uWithinDist)	{ m_uMechUseageCtrl = uMechUseageCtrl;m_uMechUseageTypeFlags = uMechUseageTypeFlags; m_uMechUseageRadius = uWithinDist;}
	FINLINE void SetMechUseageCtrl(u8 uMechUseageCtrl)							{ m_uMechUseageCtrl = uMechUseageCtrl;}
	FINLINE void SetMechUseageTypeFlags(u8 uMechUseageTypeFlags)				{ m_uMechUseageTypeFlags = uMechUseageTypeFlags;}
	FINLINE void AddMechUseageTypeFlag(u8 uMechUseageTypeFlags)					{ m_uMechUseageTypeFlags |= uMechUseageTypeFlags;}

	FINLINE CBot* GetMechLock(u8* puSeatId = NULL)				{ if (m_pMechLock && puSeatId) { *puSeatId = m_uMechLockSeatId;} return m_pMechLock; }
	void SetMechLock(CBot* pMech, u8 uSeatId);
	void ClearMechLock(void);

	void NotifyActionEnable(CEntity* pEntity);					//A player clicked "Action" on this guy!
	void NotifyDamaged(const CDamageResult *pDamageResult);		//damage was done to this brain
	void NotifyDamageDealt(const CDamageResult *pDamageResult);	//damage was done by this brain
	void NotifyRammedBy(CEntity* pEntity);						//this bot was rammed

	enum
	{
		ATTRIB_STACK_NUM_LEVELS = 4
	};

	enum
	{
		ATTRIB_AGGRESSION = 0,
		ATTRIB_COURAGE,
		ATTRIB_INTELLIGENCE,
		ATTRIB_PERCEPTORCTRL,
		ATTRIB_SPEED,
		NUM_ATTRIBS
	};

	enum
	{
		PCFLAG_EYES_ON			= 0x01,
		PCFLAG_EARS_ON			= 0x02,
		PCFLAG_TOUCH_ON			= 0x04,
		PCFLAG_RADIO_ON			= 0x08,
		PCFLAG_EYES_INUSE		= 0x10,
		PCFLAG_EARS_INUSE		= 0x20,
		PCFLAG_TOUCH_INUSE		= 0x40,
		PCFLAG_RADIO_INUSE		= 0x80
	};
	FINLINE u8				GetRace(void)							{ return m_uRace;};
	FINLINE void			SetRace(u8 uRace)						{ m_uRace = uRace;}
	void					SetBaseSpeed(u8 uBaseSpeedPct);			 //always sets attrib stack level 0
	void					PushBaseSpeedSetting(u8 uNewBaseSpeedPct); 	 //push a new speed setting on the stack
	void					PopBaseSpeedSetting(void);

	u8						GetAttrib(u8 uAttrib)					{ if (uAttrib == ATTRIB_PERCEPTORCTRL) {return GetPerceptionCtrl();} return m_aauAttribStack[uAttrib][m_auAttribStackDepth[uAttrib]];}
	void					SetAttrib(u8 uAttrib, u8 uValue)		{ m_aauAttribStack[uAttrib][m_auAttribStackDepth[uAttrib]] = uValue;}
	BOOL					AttribStack_Push(u8 uAttrib, u8 uValue);
	BOOL					AttribStack_Pop(u8 uAttrib);
	u8						AttribStack_GetDepth(u8 uAttrib)		{ return m_auAttribStackDepth[uAttrib];} 
	void					AttribStack_Init(void);
	void					AttribStack_Reset(void);
	u8						GetPerceptionCtrl(void);

	CAIBrain*				GetLeader(void)							{ return m_pLeader;};
	CAIFormation*			GetFormation(void)						{ return m_pFormation;};

	enum
	{
		GREETINGLEVEL_0 = 0,		//this means the bot has never greeted the player
		GREETINGLEVEL_1,			//this means the bot has greeted the player once
		GREETINGLEVEL_2,				
		GREETINGLEVEL_3,
		NUM_GREETINGLEVELS
	};
	void					OverrideGreetingLevel(s32 nPlayerId, u8 uGreetingLevel);

	void					DebugRender(f32 fScreenX, f32 fScreenY);

	enum
	{
		ATTACKRULESET_0 = 0,		//generic biped (probably good for guys 2-10 feet tall)
		ATTACKRULESET_1,			//nothing yet
		ATTACKRULESET_NPC,			//when NPC fight NPC
		ATTACKRULESET_TITAN0,		//attack rules for the titan
		ATTACKRULESET_PRED0,		//attack rules for the predator
		ATTACKRULESET_PROBE0,		//attack rules for the probe
		ATTACKRULESET_RAT0,			//attack rules for the probe
		ATTACKRULESET_ELITEGUARD0,	//attack rules for the elite guard
		ATTACKRULESET_ZOMBIE0,	
		ATTACKRULESET_CORROSIVE0,
		ATTACKRULESET_ZOMBIEBOSS0,
		ATTACKRULESET_JUMPER0,
		ATTACKRULESET_JUMPER1,
		ATTACKRULESET_SCIENTIST0,
		ATTACKRULESET_NONE,
		NUM_ATTACKRULESETS,
	};
	FINLINE void					SetAttackRuleSet(u8 uRuleSet)				{ m_uAttackRuleset = uRuleSet;}
	FINLINE u8						GetAttackRuleSet(void)						{ return m_uAttackRuleset;}

	//these will be called by AIBrain::Work() if AIBrain::Work() is called by the brainman because the brain is "active"
	//otherwise, they can be called directly by clients to use a CAIBrain object as a recorder of sorts
	void					DoPerceptionWork(void);  // Gather input, stick it into knowledge
	void					DoKnowledgeWork(void);   // Purge old or invalid memories from knowledge
	void					DoScanHeadWork(void);  //the head will scan back and forth
	void					DoUpdateMovementTrackingWork(void);


	//
	//	Reaction logic (AIBrainReact.cpp)
	//
	enum
	{
		REACTIONSTATE_NONE = 0,
		REACTIONSTATE_GRENADE_DODGE,
		REACTIONSTATE_GRENADE_DODGE_2,
		REACTIONSTATE_FOLLOW_SUSPICIOUS,
		REACTIONSTATE_ATTACK_IMPULSE,				 //attack reaction to danger, (no specific enemy)
		REACTIONSTATE_ATTACK,
		REACTIONSTATE_DODGE_VEHICLE,
		REACTIONSTATE_DODGE_VEHICLE_2,
		REACTIONSTATE_GOSEE_SOUND,
		REACTIONSTATE_GOSEE_SOUND_2,
		NUM_REACTIONSTATES,
	};

public:
	BOOL					IsGoalChangePending(void);

	enum
	{
		LOD_OUT									= 0x00,
		LOD_VISIBLE								= 0x01,
		LOD_ACTIVE								= 0x02,
		LOD_ON_GOAL_OVERRIDE					= 0x04,
		LOD_EARS_ON_THIS_FRAME					= 0x08,
		LOD_OVERRIDE_ALWAYS						= 0x10,	   //this means that the brain will never go inactive as a result of LOD (used by designers for important patrolling units and such)
		LOD_ALLOW_OVERRIDE_WHEN_CONVENIENT		= 0x20,	   //this means that the brain will go LOD inactive, but that the AI may wake him up occasionally if the frame rate is conducive.
		LOD_OVERRIDE_FOR_ONE_WORK				= 0x40,
		LOD_DELAY_WORK_TIL_AFTERPLAYERBOT		= 0x80
	};
	FINLINE void			SetLODOverrideAlways(void)				{ m_uLODFlags |= LOD_OVERRIDE_ALWAYS;}
	FINLINE void			SetLODOverrideForOneWork(void)			{ m_uLODFlags |= LOD_OVERRIDE_FOR_ONE_WORK;}
	FINLINE void			SetLODOverrideWhenConvenient(void)		{ m_uLODFlags |= LOD_ALLOW_OVERRIDE_WHEN_CONVENIENT;}
	FINLINE void			SetActiveAndVisibleBits(u8 uLODBits)	{ m_uLODFlags |= uLODBits;}
	FINLINE void			SetLODDelayTilAfterPlayerBot(void)		{ m_uLODFlags |= LOD_DELAY_WORK_TIL_AFTERPLAYERBOT;}
	FINLINE void			ClearLODOverrideAlways(void)			{ m_uLODFlags &= ~LOD_OVERRIDE_ALWAYS;}
	FINLINE void			ClearLODOverrideForOneWork(void)		{ m_uLODFlags &= ~LOD_OVERRIDE_FOR_ONE_WORK;}
	FINLINE void			ClearLODOverrideWhenConvenient(void)	{ m_uLODFlags &= ~LOD_ALLOW_OVERRIDE_WHEN_CONVENIENT;}
	FINLINE void			ClearLODDelayTilAfterPlayerBot(void)	{ m_uLODFlags &= ~LOD_DELAY_WORK_TIL_AFTERPLAYERBOT;}
	FINLINE void			ClearLODActiveAndVisible(void)			{ m_uLODFlags &= ~(LOD_VISIBLE |LOD_ACTIVE | LOD_ON_GOAL_OVERRIDE);}
	FINLINE BOOL			IsLODActive(void)						{ return ((m_uLODFlags & (LOD_OVERRIDE_ALWAYS | LOD_ACTIVE | LOD_VISIBLE | LOD_ON_GOAL_OVERRIDE | LOD_OVERRIDE_FOR_ONE_WORK)) ||
																			  ((m_uLODFlags & LOD_ALLOW_OVERRIDE_WHEN_CONVENIENT) && m_uLODOverrideTimeLeft > 0));  }
	FINLINE BOOL			GetLODFlags(void)						{ return m_uLODFlags;}
	BOOL					IsHearingLODActive(void)						{ return m_uLODFlags & LOD_EARS_ON_THIS_FRAME;}
	void					ForceHearingLODOnThisFrame(void)					{ m_uLODFlags |= LOD_EARS_ON_THIS_FRAME;}
	void					ClearHearingLOD(void)							{ m_uLODFlags &= ~LOD_EARS_ON_THIS_FRAME;}

	void					SetHearingMagUnalert(f32 fScanDist)					{ m_PercepUnalert.m_fHearingMag = fScanDist;}
	void					SetHearingMagAlert(f32 fScanDist)					{ m_PercepAlert.m_fHearingMag = fScanDist;}
	void					SetEyeScanDistUnalert(f32 fScanDist)					{ m_PercepUnalert.m_fEyeScanDist = fScanDist;}
	void					SetBonusEyeScanDistForPlayersUnalert(f32 fBonusDist)   { m_PercepUnalert.m_fBonusEyeScanDistForPlayers = fBonusDist;}
	void					SetEyeCosHalfFOVUnalert(f32 fFov)						{ m_PercepUnalert.m_fEyeCosHalfFOV = fFov;} 
	void					SetEyeScanDistAlert(f32 fScanDist)					{ m_PercepAlert.m_fEyeScanDist = fScanDist;}
	void					SetBonusEyeScanDistForPlayersAlert(f32 fBonusDist)   { m_PercepAlert.m_fBonusEyeScanDistForPlayers = fBonusDist;}
	void					SetEyeCosHalfFOVAlert(f32 fFov)						{ m_PercepAlert.m_fEyeCosHalfFOV = fFov;} 

	FINLINE f32				GetEyeScanDist(void)					{ return FMATH_FPOT(m_fBrainAlertUnit, m_PercepUnalert.m_fEyeScanDist, m_PercepAlert.m_fEyeScanDist);}
	FINLINE f32				GetBonusEyeScanDistForPlayers(void)		{ return FMATH_FPOT(m_fBrainAlertUnit, m_PercepUnalert.m_fBonusEyeScanDistForPlayers, m_PercepAlert.m_fBonusEyeScanDistForPlayers);}
	FINLINE f32				GetEyeCosHalfFOV(void)					{ return FMATH_FPOT(m_fBrainAlertUnit, m_PercepUnalert.m_fEyeCosHalfFOV, m_PercepAlert.m_fEyeCosHalfFOV);} 
	FINLINE f32				GetHearingMag(void)						{ return FMATH_FPOT(m_fBrainAlertUnit, m_PercepUnalert.m_fHearingMag, m_PercepAlert.m_fHearingMag);}
	void					BoostBrainAlert(f32 fUnitVal)			{ if (fUnitVal > m_fBrainAlertUnit) m_fBrainAlertUnit = fUnitVal;}


	enum
	{
		SOUND_MARK_FLAG_NONE			= 0x0000,
		SOUND_MARK_FLAG_QUICK_TURN		= 0x0010,
		SOUND_MARK_FLAG_VALID			= 0x0020
	};

	FINLINE BOOL			IsSoundMarkValid(void)					{ return (m_uSoundMarkFlags & SOUND_MARK_FLAG_VALID)!=0;}
	void					SetSoundMark(CAISound* pSound, u8 uSoundMarkFlags = 0);
	void					ClearSoundMark(void);
	const CFVec3A&			GetSoundMarkLoc(void);

	enum
	{
		SIGHT_MARK_FLAG_NONE			= 0x0000,
		SIGHT_MARK_FLAG_PROJECTILE		= 0x0001,
		SIGHT_MARK_FLAG_BOT				= 0x0002,
		SIGHT_MARK_FLAG_SECOND_STAGE	= 0x0004,
		SIGHT_MARK_FLAG_TWO_STAGE		= 0x0008,
		SIGHT_MARK_FLAG_QUICK_TURN		= 0x0010,
		SIGHT_MARK_FLAG_STOP_AND_LOOK	= 0x0020,
		SIGHT_MARK_FLAG_VALID			= 0x0040
	};

	BOOL					IsSightMarkValid(void)					{ return (m_uSightMarkFlags & SIGHT_MARK_FLAG_VALID)!=0;}
	void					SetSightMark_1Stage(const CFVec3A& SightPt1_WS,u8 uSightMarkFlags );
	void					SetSightMark_2Stage(const CFVec3A& SightPt1_WS, const CFVec3A& SightPt2_WS, u8 uSightMarkFlags );
	void					ClearSightMark(void);
	void					GetSightMark(CFVec3A* pMarkPos);

	FINLINE u32				GetReactionState(void)					{ return m_uReactionState;}
	BOOL					ChangeReactionState(u32 uReactionState);

	BOOL					ShouldFollowerAttack(CAIBrain* pLeaderBrain, CEntity* pEnemy);

	BOOL					Action_DodgeGrenade(const CFVec3A& WhereLoc);
	BOOL					Action_DodgeVehicle(CAISound* pSound);
	BOOL					Action_Greet(CAIMemorySmall* pGreetMemory, CEntity* pOtherEntity, u8 uGreetingCount);
	BOOL					Action_InvestigateSound(CAIMemoryLarge* pHeardMem, CAISound* pSound);
	BOOL					Action_Attack(CEntity* pOtherEntity);
	BOOL					Action_GoSeeSound(CAISound* pSound);

	FINLINE void			SetDodgeVehicleReactBlockerTimeOut(u16 uDodgeVehicleReactBlockerTimeOut)	{m_uDodgeVehicleReactBlockerTimeOut = uDodgeVehicleReactBlockerTimeOut;}

	FINLINE void			SetNumWeapons(u8 uNumWeapons)				{ m_uNumWeapons = uNumWeapons;}
	FINLINE u8				GetNumWeapons(void)							{ return m_uNumWeapons;}

	FINLINE void			SetWakeupRadius(u8 uWakeupRadius)				{ m_uWakeupRadius = uWakeupRadius;}
	FINLINE u8				GetWakeupRadius(void)							{ return m_uWakeupRadius;}

	FINLINE void			SetMeleeConfig(u8 uMeleeRad, u8 uMeleeTimeMin, u8 uMeleeTimeMax)			{m_uMeleeRad = uMeleeRad; m_uMeleeTimeMin = uMeleeTimeMin; m_uMeleeTimeMax = uMeleeTimeMax; }
	FINLINE void			GetMeleeConfig(u8* puMeleeRad, u8* puMeleeTimeMin, u8* puMeleeTimeMax)		{*puMeleeRad = m_uMeleeRad; *puMeleeTimeMin = m_uMeleeTimeMin; *puMeleeTimeMax = m_uMeleeTimeMax;}

	FINLINE void			SetCommGroupName(cchar* pszStringTableCommGroupName)		{ m_pszStringTableCommGroupName = pszStringTableCommGroupName;}
	FINLINE cchar*			GetCommGroupName(void)										{ return m_pszStringTableCommGroupName;}

	BOOL					HasMovedOneFootSince(f32 fTimeSecs);
	FINLINE f32				GetLastXYZChangeTime(void)				{ return m_fLastXYZChange;}
	FINLINE f32				GetWorldAddRandom(void)					{ return m_fWorldAddRandom;}
	FINLINE f32				GetInfreqCycleRandom(void)				{ return m_fInfreqCycleRandom;}
	void					UpdateInfreqCycleRandom(void);

	FINLINE BOOL			IsThoughtChangePending(void)				{ return (m_PendingThoughtList.Size() > 0);}
	BOOL					PlayBTAOnTakeDamage(CAIMemoryLarge* pDamagedByMem);
	BOOL					Rules_Job_Generic_OnPrompted(CAIMemorySmall* pPrompted);
	FINLINE u16				GetMaxJumpDist(void)						{ return m_uJumpDist;}

protected:
	FINLINE CPendingThought* GetPendingThought(void)					{ return m_PendingThoughtList.Begin().Get();}
	FINLINE CPendingThought* PopPendingThought(void)					{ return m_PendingThoughtList.PopHead();}
	BOOL					AddPendingThought(s8 nThoughtType, CAIMemorySmall* pParamPack = NULL);	 
	BOOL					AddPendingReact(s8 nThoughtType, CAIMemorySmall* pParamPack = NULL);	 
	BOOL					AddPendingTalk(CAIMemorySmall* pParamPack = NULL);

	void					DoDecisionWork_PreThought(void);     // Request thought changes based on contents of knowledge. Called before any of the current thought works are called
	void					DoDecisionWork_PostThought(void);    // Request thought changes based on contents of knowledge. Called after all of the current thought works are called


	BOOL					Rules_Job_Generic_OnSeenFriendlyPlayer(CAIMemoryLarge* pSeenMem);
	BOOL					Rules_Job_Generic_OnSeenEnemyPlayer(CAIMemoryLarge* pSeenMem);
	BOOL					Rules_Job_Generic_OnSeenSuspiciousPlayer(CAIMemoryLarge* pSeenMem, f32 *pfSuspicion);
	BOOL					Rules_Job_Generic_OnSeenFriendlyNPC(CAIMemoryLarge* pSeenMem);
	BOOL					Rules_Job_Generic_OnSeenEnemyNPC(CAIMemoryLarge* pSeenMem);
	BOOL					Rules_Job_Generic_OnHeard(CAIMemoryLarge* pHeardMem);
	BOOL					Rules_Job_Generic_OnTakeDamage(CAIMemoryLarge* pDamagedMem);
	void					Rules_Job_Generic_EnviroChangeRules(void);
	void					Rules_Job_Generic(void);
	void					DoBehaviorRules(void);	 // 

	void					DoThoughtFSMWork(void);	 // the fsm that controls current thought work function calls and changing of current thought
	void					DoLeaderWork(void);
	void					DoFollowerWork(void);
	void					SetPendingThought(void);
	void					OnPerceiveFriendly(CEntity* pEntity);
	void					OnPerceiveEnemy(CEntity* pOtherEntity);
	void					OnPerceiveDanger(const CFVec3A& Where);
	BOOL					DoVisCheckAndVerifyLOSWithEntity(CEntity* pOtherMover, CAIMemoryMedium* pVisCheckMem = NULL);
	BOOL					IntersectsVisionCone(const CFVec3A& Pt_WS, f32 fOtherRad, f32 fVisionRange, f32 *pfDot = NULL, f32 *pfDist = NULL);
	BOOL					ShouldStartle(void);
	void					SetStartleTimeOut(void);
	u16						GetAllQuiteTime(void);	//How long has it been all Quiet
	void					CanSeeIt(CEntity* pEntity, f32 fOtherRadius);


	CAIMover				*m_pMover;
	CAIBrain				*m_pLeader;
	CAIFormation			*m_pFormation;			// if there are followers, there could be a formation.

	ThoughtBank_AccessFunc	*m_apThoughtBankAccessFunc[NUM_THOUGHT_TYPES];	 // each brain has a table of function pointers for retrieving and recycling thought objects.
	CAIThought				*m_pCurThought;
	CAIThought				*m_pCurTalk;
	CAIThought				*m_pCurReact;
	CAIThought				*m_pCurFollowerThought;

	s8						m_nJobThought;
	s8						m_nGoalThought;
	s8						m_nCurThought;
	s8						m_nLastThought;
	CAIMemorySmall			*m_pJobParamPack;		// pointer to the param pack for the current Job thought... might be NULL, otherwise, it should point to a memory in the brain's knowledge
													// store it because we may need to restart our Job thought after completing a goal
	u32						m_uBrainFlags;
	u16						m_uBrainGUID;
	u16						m_uEyeReactBlockerTimeOut;
	u16						m_uStartleTimeOut;						//time at which bot will be startle-able again
	u16						m_uIgnoreNearbyGrenadeTimeOut;			//When I can dodge grenades again
	u16						m_uDodgeGrenadeTimeOut;					//When the current grenade doge reaction should timeout if no grenade explosion is heard
	u16						m_uDodgeVehicleReactBlockerTimeOut;		//When I can dodge grenades again
	u16						m_uGoSeeSoundTimeOut;
	u16						m_uDontGoSeeSoundsTimeOut;
	u16						m_uHearBTATimeOut;
	u16						m_uOuchBTATimeOut;
	u16						m_uSeeBTATimeOut;
	u16						m_uJumpDist;							//How far can the mover that goes with this brain jump when not injured

	// The various brain attributes govern when, why and how the robot makes decisions and behaves
	u8						m_aauAttribStack[NUM_ATTRIBS][ATTRIB_STACK_NUM_LEVELS];
	u8						m_auAttribStackDepth[NUM_ATTRIBS];

	u8						m_uRace;				//Race of this brain
	u8						m_uAttackRuleset;		//which set of rules does this brain follow when it is attacking?
	u8						m_uStartleMin;			//Min time between startles
	u8						m_uStartleMax;			//Max time between startles
	u8						m_uMechUseageRadius;
	u8						m_uMechUseageCtrl;
	u8						m_uMechUseageTypeFlags;
	u8						m_uMechLockSeatId;
	CBot					*m_pMechLock;

	u8						m_uNumWeapons;
	u8						m_uWakeupRadius;
	u8						m_uMeleeRad;
	u8						m_uMeleeTimeMin;
	u8						m_uMeleeTimeMax;
	cchar*					m_pszStringTableCommGroupName;			//a string table entry for the communication group name this brain is in.


	u8						m_uOddsOfIgnoringPlayerAtEyeScanDist;
	u8						m_uInnerRadiusForIgnorePlayerOdds;		//From this m_uIgnorePlayerOddInnerRadius to m_fEyeScanDist+m_fBonusEyeScanDistForPlayers, Odds of ignoring 0% -m_uOddsOfIgnoringPlayerAtEyeScanDist% will be squared distance interpolation
	u8						m_uOddsOfIgnoringNearbyGrenades;		//When a greande sound is heard, these are the odds that a grenade reaction will NOT be done
	u8						m_uOddsOfNotDodgingVehicles;			//When a vehicle comes...
	u8						m_uOddsOfNotGoingToSeeSounds;			//When a go sound is heard
	u8						m_uAttackNotifyRadius;					//How large a radius will the bot notify other bots at the time attack begins

	
	
	CAIKnowledge			m_Knowledge;			// every brain must have a knowledge object to keep track of memories.  There are many assumptions to this effect.
	CNiList<CAIPost>		m_FollowerList;			// findfix: could potentially make this an optional component of brain.  And add functions to attach follower list data to a brain.
	PendingList				m_PendingTalkList;
	PendingList				m_PendingReactList;
	PendingList				m_PendingThoughtList;

	class CPercepCtrl
	{
	public:
		f32						m_fHearingMag;
		f32						m_fEyeScanDist;
		f32						m_fBonusEyeScanDistForPlayers;
		f32						m_fEyeCosHalfFOV; 
	};
	CPercepCtrl				m_PercepUnalert;
	CPercepCtrl				m_PercepAlert;
	f32						m_fBrainAlertUnit;
	f32						m_VolSlackAtLastNapJerk;	  // for nap-jerk reaction
	f32						m_fVolSlackTimeOut;			  // for nap-jerk reaction
	f32						m_fHeadScan;
	f32						m_fWorldAddRandom;
	f32						m_fInfreqCycleRandom;
	f32						m_fSoundMarkTime;
	f32						m_fSightMarkTime;
	f32						m_fLastXYZChange;
	u32						m_uLastX;
	u32						m_uLastY;
	u32						m_uLastZ;
	u8						m_uLODFlags;
	u8						m_uLODOverrideTimeLeft;
	u8						m_uSightMarkFlags;
	u8						m_uSoundMarkFlags;
	u8						m_uSuspicionBTAPlayed;
	u8						m_uSuspicionLevel;
	u8						m_uReactionState;
	CAISound				m_SoundMark;
	CFVec3A					m_SightMark1;
	CFVec3A					m_SightMark2;


//	Entity Ptr Fixups for CheckpointRestore  findfix: having to have this data here stinks
	u32						m_uPlayerLeaderGUID;
	u32						m_uMechGUID;
	u32						m_uMechLockGUID;


//
//
//	Static
//
public:
	static cchar*			ThoughtTypeToString(s16 sThoughtType);
	static cchar*			s_aszThoughtTypesStrings[];
	static cchar*			s_apszBrainReactionStateStrings[];
	static f32				s_fDamagedByTotalDamageMultiplier;

	FINLINE static BOOL		IsJob(s16 nThoughtType)			{ return nThoughtType <= TT_MAX_JOB_TYPE;}
	FINLINE static BOOL		IsGoal(s16 nThoughtType)		{ return nThoughtType <= TT_MAX_GOAL_TYPE && nThoughtType > TT_MAX_JOB_TYPE;}

	enum
	{
		AIBRAIN_DEFAULT_GLOBAL_PENDINGTHOUGHT_POOL_COUNT = 200,
	};
	enum
	{
		AIBRAIN_DEFAULT_GLOBAL_FORMATIONPOST_POOL_COUNT = 50,
	};
	static BOOL				InitLevel(struct FLinkRoot_s* pGlobalPtrNodePool);
	static void				UninitLevel(void);
	static CNiBank<CPendingThought>* s_pPendingThoughtBank;

	static BOOL				m_bGlobalBlindDefAndDumb;
	static BOOL				m_bGlobalIgnoreMode;

	static BOOL				_SoundCollisionCB(CAISound* pSound, void* pData);
	static BOOL				_CanSeePlayer(CAIBrain* pThisBrain, CEntity* pPlayerEntity);
	static BOOL				_CanSeeNPCBrain_CB(CAIBrain* pOtherBrain, void* pData);
	static void				_BeginVisibleEntityScan(void);
	static BOOL				_CanSeeVisibleEntity_CB(CAISound* pSound, void* pData);
	static void				_EndVisibleEntityScan(CAIBrain* pThisBrain);


	friend class CAIFormation;
	friend void aibrainman_Render(u32 uRenderBits);

	
	FCLASS_STACKMEM_ALIGN(CAIBrain); 
} FCLASS_ALIGN_SUFFIX;


#endif //_AIBRAIN_H_
