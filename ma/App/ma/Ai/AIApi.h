//////////////////////////////////////////////////////////////////////////////////////
// Ai.h - 
//
// Author: Pat MacKellar 
//
//
//	 API for controlling the behavior of AI Units
//
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
// 05/10/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AI_H_
#define _AI_H_ 1

#include "fmath.h"
#include "bot.h"
class CAIBrain;
class CEntity;

//typedef enum NPCWeapon_e;


////////////////////////////////////////////////// Jobs ////////////////////////////////////
//
// Wait
//	 - Unit stands in place, facing a place or thing, and occasionally looking around.
BOOL ai_AssignJob_Wait(	CAIBrain* pBrain,
						BOOL bSpecifyLookAt,		// TRUE/FALSE  (True means use pLookAtObj or LookAtLoc) FALSE means no lookAtgoal
						const CFVec3A& LookAtLoc,	// A position to look at (Ignored if LookAtObjGUID > 0)	
						u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
						u8 uLookAroundChance);		// (0-100) Percent Chance that obj will decide to look around. (evaluated several times a minute)

//
// Wander 
//	 - Unit moves around randomly within specifications	 (radius, or list of points)
BOOL ai_AssignJob_Wander(	CAIBrain* pBrain,
							f32 fRadius,				// (0.0 - Inf)	Radius (in feet) to stay within while wandering
							BOOL bStayInRoom,			// (0/1)		Use radius, but always stay in the original room
							u8 uSpeedPctMin,			// (0-100)		Min Pct of this unit's ability that it should travel at	 (0 means wander should never change units speed)
							u8 uSpeedPctMax,			// (0-100)		Max             "      "								 (0 means wander should never change units speed)
							u8 uSociable,				// (0-100)		How likely it is to stop and talk to other bots
							u8 uCuriosity,				// (0-100)		Breaks will last 5 seconds + random(curiosity)
							u8 uTakeBreakChance,		// (0-100)		Percent Chance that that bot will take a break. (evaluated at every grid point)
							cchar *pszPathName = NULL);	// String table Name of espline entity that contains the path points


//
// Patrol
//	- Unit moves along a path, potentially doing something special in custom behavior zones
///////////////////////
BOOL ai_AssignJob_Patrol(	CAIBrain *pBrain,
							cchar *pszPathName,					// String table Name of espline entity that contains the path points
							u8 uPatrolSpeed,					// (0-100)	Pct of speed patrolling unit should travel at  (0 means patrol should never change units speed)
							u8 uDirection,						// (0,1)	0= Clockwise, 1 = CounterClockWise
							u8 uNumIgnoreZones = 0,				// how many elements in the next parameter
							cchar** paszIgonoreZones = NULL);	// array of names of patrol zones to ignore




//////////////////////////////////////////////////// Goals ////////////////////////////////////

//
// Goto
//	 - AI Brain will goto the specified location using pathfinder and object avoidance
enum
{
	GOTOFLAG_NONE							= 0x0000,
	GOTOFLAG_LOOKAT							= 0x0001,
	GOTOFLAG_3DBOT							= 0x0002,
	GOTOFLAG_BLIND_OK						= 0x0004,
	GOTOFLAG_DID_LOOKAT						= 0x0008,
	GOTOFLAG_CAN_ROLL						= 0x0010,
	GOTOFLAG_FORCE_ROLL						= 0x0020,
	GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END	= 0x0040,
	GOTOFLAG_DID_SET_SPEED					= 0x0080,
	GOTOFLAG_USE_JOB_REACT_RULES			= 0x0100,
	GOTOFLAG_VICT_BTA_AT_END				= 0x0200,
	GOTOFLAG_POWERDOWN_AT_END				= 0x0400,
	GOTOFLAG_RETURN_FIRE_OK					= 0x0800
};

#define GOTOFLAGS_ALLEXTERNAL (GOTOFLAG_RETURN_FIRE_OK | GOTOFLAG_POWERDOWN_AT_END | GOTOFLAG_USE_JOB_REACT_RULES | GOTOFLAG_CAN_ROLL | GOTOFLAG_FORCE_ROLL | GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END | GOTOFLAG_BLIND_OK | GOTOFLAG_LOOKAT | GOTOFLAG_VICT_BTA_AT_END)
BOOL ai_AssignGoal_Goto(CAIBrain* pBrain,
						const CFVec3A& GotoLoc,			// Where
						u8 uFudgeDest = 0,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
						u8 uSpeedPct = 0,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
						u16	uGotoFlags = 0);			// GOTOFLAG_*


//
// Goto With LookAt
//
//	 - Unit will goto the specified location using pathfinder and object avoidance
//   - while looking at a something
BOOL ai_AssignGoal_GotoWithLookAt(	CAIBrain* pBrain,
									const CFVec3A& GotoLoc,			// Where
									const CFVec3A& LookAtLoc,		// A pt in world space to aim torso and head at.(only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
									u32	uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means none).
									u8 uPctToLookAt,				// (0-100) Percent to Goal that look at order will turn on  
									u8 uFudgeDest = 0,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
									u8 uSpeedPct = 0,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
									u16	uGotoFlags = 0);			// GOTOFLAG_*
//
// Attack
//	 - Unit will Attack the specified Entity
BOOL ai_AssignGoal_Attack(	CAIBrain* pBrain,
							u32 uEnemyGUID,				// The baddie
							s16 nCompletionEvent);		// A scripting event to call apon completion or failure

//
// FaceIt
//	 - Look at a place or thing
BOOL ai_AssignGoal_FaceIt(	CAIBrain* pBrain,
							const CFVec3A& LookAtLoc,	// A position to look at (Ignored if LookAtObjGUID > 0)	
							u32	uLookAtObjGUID,			// The GUID of an entity to keep looking at (0 means ignore)
							u8 uTrack,					// (0-100) How hard to try to track LookAt it if moving
							u8 uTimeOut,				// (0-255) Even if the thing is there, quit after this number of seconds. Time starts when alignment is pretty close. (0 means never quit)
							s16 nCompletionEvent);		// A scripting event to call apon completion or failure


//
// Follow
//	 - Follow another Unit.
BOOL ai_FollowLeader(	CAIBrain* pBrain,
						u32	uLeaderGUID,			// The GUID of an entity to follow
						s16 nCompletionEvent);		// A scripting event to call apon completion or failure

//
// Notify that the AIs weapon has changed, set the new firing defaults.
//
BOOL ai_WeaponChanged( CAIBrain *pBrain, CBotBuilder::NPCWeapon_e eWpn );


///////////////////////////////////////////////  Talking/Gesturing  //////////////////////////////////////////////////


enum TalkModeCBControl
{
	TMCB_READY,
	TMCB_DONE,
};
typedef BOOL TalkModeCallback(u32 uTalkModeCBControl, void *pvData1, void *pvData2);
BOOL ai_TalkModeBegin(	CAIBrain* pBrain,
						TalkModeCallback *pCB,		// A Callback function.  Will be called when the AI Actually begins talking, or disregards the request to begin talking.
						void *pvData1,				// data to be passed to pFunc
						void *pvData2,				// data to be passed to pFunc
						u8 uTimeOutSecs,			// How long this request is valid.
						BOOL bNeedtoStop,			// The ai must stop moving before calling the callback func
						BOOL bNeedToFace,			// The ai must face the specified entity before calling the callback func
						u32 uLookAtObjGUID);		// The GUID of an entity to keep looking at (0 means ignore)
BOOL ai_TalkModeEnd(CAIBrain* pBrain);
BOOL ai_IsTalking(CAIBrain* pBrain);
BOOL ai_IsAttacking(CAIBrain* pBrain);
CBot* ai_GetMechLock(CAIBrain* pBrain);

//////////////////////////////////////////////////  Perception Control ///////////////////////////////////////////
// Note from Justin to Pat: If you change/add/remove any of these enums, please make sure to change
//   the appropriate enums in MAScriptTypes.inc (a script include file).  Also, if you have to change the
//   enum there, you will also *have* to recompile all script files that use that enum, or else they will
//   already be compiled to be using the old constants.
enum
{
	AI_PERCEPTOR_EYES = 0,
	AI_PERCEPTOR_EARS,
	AI_PERCEPTOR_TOUCH,
	AI_PERCEPTOR_RADIO,
	AI_NUM_PERCEPTORS
};

void ai_TurnOnPerceptor(CAIBrain* pBrain, u8 uPerceptorId); 	// On means recording input
void ai_TurnOffPerceptor(CAIBrain* pBrain, u8 uPerceptorId); 	// Off means brain might as well not have it
void ai_UsePerceptor(CAIBrain* pBrain, u8 uPerceptorId); 		// Use means do things based on the input from this perceptor
void ai_IgnorePerceptor(CAIBrain* pBrain, u8 uPerceptorId); 	// Ignore means DON"T do things base on the input from this perceptor
BOOL ai_IsPerceptorOn(CAIBrain* pBrain, u8 uPerceptorId);
BOOL ai_IsPerceptorInUse(CAIBrain* pBrain, u8 uPerceptorId);
f32 ai_GetBrainAlertUnit(CAIBrain* pBrain);					

//////////////////////////////////////////////////  Brain Attributes ///////////////////////////////////////////////
enum
{
	AIRACE_MIL = 0,
	AIRACE_DROID,
	AIRACE_ZOMBIE,
	AIRACE_AMBIENT,
	NUM_AIRACES
};

//
// Race
//
u8 ai_GetRace(CAIBrain* pBrain);	  // uAIRace is from enum above
void ai_SetRace(CAIBrain* pBrain, u8 uAIRace);	  // uAIRace is from enum above

void ai_SetBaseSpeed(	CAIBrain* pBrain,
						u8 uSpeedPct);	// (0-100) base speed	(0 means stop!)

// remove any attack specs
void ai_ClearAttackSpecs( CAIBrain* pBrain,
						  BOOL bResetCurrentAttack);		// if brain is currently on attack, do you want to reset the attack to use default attack specs?		

// change or specify new attack specs
void ai_SetAttackSpecs( CAIBrain* pBrain,
						u8 uNormalRuleSet,			// which ruleset the brain will use when attacking
						u8 uAlternateRuleSet,			// an alternate one
						u8 uAlternateRuleSetOdds,		// (0-100) pct chance that the alternate will be used
						u16 fAttackMaxDistFromOrigin,		// Limit the distance traveled from location at the time attack mode turns on
						u16 fStayPutDist,					// stay local until enemy is atleast this close
						u8 uAttackSpeedMin,					// (0-100) how fast to move while in attack mode (min)
						u8 uAttackSpeedMax,					// (0-100) how fast to move while in attack mode (max)
						BOOL bResetCurrentAttack);		// if brain is currently on attack, do you want to reset the attack to use default attack specs?		


void ai_NotifyCutSceneBegin(void);
void ai_NotifyCutSceneEnd(void);

//
//	AI Script Event Enums
//
enum
{
	AI_EVENTDATA1_GOAL_COMPLETE = 0,
	AI_EVENTDATA1_NO_ATTACKERS = 1,
	AI_EVENTDATA1_ATTACK_INIT = 2
};

void ai_EnableUnarmedPlayerRule(void);
void ai_DisableUnarmedPlayerRule(void);
BOOL ai_IsUnarmedPlayerRuleEnabled(void);

//
// Follow
//	 - Follow another Unit.
CEntity* ai_GetLastEnemySeen( CAIBrain* pBrain,
							  CFVec3A* pLastSeenLoc,
							  BOOL *pbNewlySeen = NULL,
							  u16 *puWhenLastSeenSec = NULL);


//
//	AI Buddy Ctrl
//
enum
{
	AI_BUDDY_CTRL_NEVER = 0,
	AI_BUDDY_CTRL_AUTO,
	AI_BUDDY_CTRL_ONACTION
};

void ai_ChangeBuddyCtrl(CAIBrain* pBrain, u8 uBuddyCtrl);	  //change buddy ctrl.... will break formation or connect up follow leader if appropriate
void ai_ResetToJob(CAIBrain* pBrain);

//
// Grenade Useage Ctrl
//
enum
{
	AI_GRENADE_USE_NEVER				= 0x00,
	AI_GRENADE_USE_ENEMYINSIGHT			= 0x01,
	AI_GRENADE_USE_SELF_DEFENSE			= 0x02,
	AI_GRENADE_USE_FIRST_CONTACT		= 0x04,
	AI_GRENADE_USE_ENEMY_EVASIVE		= 0x08,
	AI_GRENADE_USE_RETURN_GRENADE_FIRE	= 0x10,
	AI_GRENADE_USE_WHENEVER				= (AI_GRENADE_USE_ENEMYINSIGHT | AI_GRENADE_USE_SELF_DEFENSE | AI_GRENADE_USE_FIRST_CONTACT | AI_GRENADE_USE_ENEMY_EVASIVE | AI_GRENADE_USE_RETURN_GRENADE_FIRE),
	AI_GRENADE_USE_SMART				= (AI_GRENADE_USE_ENEMYINSIGHT | AI_GRENADE_USE_SELF_DEFENSE | AI_GRENADE_USE_ENEMY_EVASIVE)
};


#endif