//////////////////////////////////////////////////////////////////////////////////////
// AIBrainMan.h - 
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
#ifndef _AIBRAINMAN_H_
#define _AIBRAINMAN_H_ 1

class CAIBrain;
class CEntity;
class CEntityBuilder;
class CDamageData;
class CDamageResult;

BOOL aibrainman_InitSys(void);											//called once at beginning of level before any units are created
void aibrainman_UninitSys(void);										//called once at end of level.  (Hopefully after all units are removed from world?) findfix:
void aibrainman_CheckpointRestoreSys(void);
void aibrainman_Work(void);												//called once per game frame... Work for all aibrains and their bots will be called.
void aibrainman_Render(u32 uRenderFlags);								//called once to draw debug info for bots
CAIBrain* aibrainman_Create(CEntity* pEntity, CEntityBuilder* pBuilder = NULL);	//to be called whenever an entity that needs AI is created.
void aibrainman_Destroy(CEntity* pEntity);								//to be called whenver an entity that needs AI is destroyed
void aibrainman_AddToWorld(CEntity* pEntity,BOOL bActivate = TRUE);		//to be called whenver an entity that needs AI is created.
void aibrainman_RemoveFromWorld(CEntity* pEntity);						//to be called whenver an entity that needs AI is created.
void aibrainman_CheckpointSave(CEntity* pEntity);						//save an entity's ai state for a checkpoint
void aibrainman_CheckpointRestore(CEntity* pEntity);					//restore an entity's ai state for a checkpoint
void aibrainman_CheckpointRestorePostamble(CEntity* pEntity);			//postamble to the restore
void aibrainman_StopAttacking(void);									//restore all AI to their Jobs

BOOL aibrainman_Activate(CAIBrain* pBrain);				//the brain manager will start calling CBrain::Work every frame
#define DO_CLEAR_BRAIN TRUE
#define DONT_CLEAR_BRAIN FALSE
BOOL aibrainman_Deactivate(CAIBrain* pBrain, BOOL bClearBrain = DO_CLEAR_BRAIN); //the brain manager will stop calling CBrain::Work every frame
BOOL aibrainman_IsActive(CAIBrain* pBrain);
BOOL aibrainman_PlayerBotBrainWork(CAIBrain* pBrain, u32 uPlayerId);
BOOL aibrainman_ConfigurePlayerBotBrain(CAIBrain* pBrain, u32 uPlayerId);
BOOL aibrainman_ConfigurePostPossessionBotBrain(CAIBrain* pBrain);
BOOL aibrainman_ConfigureNPCVehicleBrain(CAIBrain* pVehicleBrain, CAIBrain* pDriverBrain);	  //this is a vechile brain that has somebody in it
BOOL aibrainman_ConfigurePostNPCVehicleBrain(CAIBrain* pVehBrain);		//this is a vechile brain that has nobody in it
void aibrainman_NotifyActionEnable(CAIBrain* pBrain, CEntity* pEntity);							//A player clicked "Action" on this guy!
void aibrainman_DamagedNotify(CAIBrain* pDamagee, const CDamageResult *pDamageResult);	//A Damage was done to a brain
void aibrainman_DamageDealtNotify(CAIBrain* pDamager, const CDamageResult *pDamageResult);	//A Damage was done to a brain
void aibrainman_RammedByNotify(CAIBrain* pBrain, CEntity* pEntity);
void aibrainman_NotifyMechEject(CAIBrain* pBrain);	//this brain got ejected from its mech
void aibrainman_SetLODOverrideForOneWork(CAIBrain* pBrain);
void aibrainman_SetDisableEntityWorkFlag(CAIBrain* pBrain);
void aibrainman_ClearDisableEntityWorkFlag(CAIBrain* pBrain);

//
//	 A CAIBrain is a container
//

//BrainType dictates which thoughts are available to the brain
enum BrainTypes
{
	BRAINTYPE_GENERICBIPED = 0,
	BRAINTYPE_GENERICTREADED,
	BRAINTYPE_GENERICHOVERCRAFT,
	BRAINTYPE_PILL,
	BRAINTYPE_SCOUT,
	BRAINTYPE_MORTAR,
	BRAINTYPE_CORROSIVE,
	BRAINTYPE_ZOMBIEBOSS,
	NUM_BRAINTYPES,
};


// Mover Class dictates which type of object is moved
enum AIMoverClasses
{
	MOVERCLASS_XZ = 0,			//stick to the ground
	MOVERCLASS_3D,				//fly around
	MOVERCLASS_CAR,				//stick to the ground, but drive like a car (no turn in place)
	MOVERCLASS_XZANDHOVER,		//stick to the ground, or hover above it
	NUM_MOVERCLASSES,
};


enum BrainDebugRenderFlags
{
	AIBM_DISPLAY_BRAIN_STATES		= 0x0001,
	AIBM_DISPLAY_MOVER_INFO			= 0x0002,
	AIBM_DISPLAY_CURTHOUGHT_DATA	= 0x0004,
	AIBM_DISPLAY_NEXT_WAYPOINT		= 0x0008,
	AIBM_DISPLAY_KNOWLEDGE_INFO		= 0x0010,
	AIBM_DISPLAY_PATROLPATHS		= 0x0020,
	AIBM_DISPLAY_BRAIN_NAME			= 0x0040,
	AIBM_DISPLAY_HEARING			= 0x0080,
	AIBM_DISPLAY_VISION				= 0x0100,
	AIBM_DISPLAY_WEAPON_INFO		= 0x0200,
	AIBM_DISPLAY_ALLNPCBRAINMASK	= 0x0fff,
	AIBM_DISPLAY_ACTIVE_LIST		= 0x1000,
	AIBM_DISPLAY_PLAYER_BRAIN		= 0x2000,
};

typedef BOOL ActiveBrainCBFunc(CAIBrain* pBrain, void* pData);
#define AIBRAINMAN_STOP_ITERATION 1
#define AIBRAINMAN_CONTINUE_ITERATION 0
void aibrainman_IterateActiveBrains(ActiveBrainCBFunc* pCBFunc, void* pData);
void aibrainman_CountActiveBrains(void);
void aibrainman_DeactivateNoDelete(CAIBrain* pBrain);

#endif

