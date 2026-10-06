//////////////////////////////////////////////////////////////////////////////////////
// MAScriptTypes.h - Mettle Arms script functions.
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
// 05/01/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MASCRIPTTYPES_H_
#define _MASCRIPTTYPES_H_ 1

#include "fang.h"
#include "fres.h"
#include "fCamAnim.h"
#include "amx.h"

#define NUM_MA_ETIMERS 10

class CMeshEntity;
class CCamManualInfo;
class CCamCutsceneInfo;
class CFAnimInst;
class CBot;
class CEntityControl;
class CESpline;

extern AMX_NATIVE_INFO MAScriptTypes_aMAEventNatives[];

class CMAScriptTypes
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();
#if FANG_WINGC
	static void RefreshCoopScriptPlayers();
#endif
};

class CMAST_EntityWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static cell AMX_NATIVE_CALL E_Find(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_FindInWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_FindOutWorld(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL E_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsNamed(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsBot(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsDoor(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsMesh(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsSwitch(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL E_EnableTripwire(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL E_AddToWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_RemoveFromWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsInWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsTargetable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsHittable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsActionable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsTripwire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_SetNormHealth(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_GetNormHealth(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_SetTargetable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_SnapToE(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL E_SetExplodArmor(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_SetFlameArmor(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_SetBulletArmor(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL E_GetPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_TouchOfDeath(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_SetDamageable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsDamageable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_Blast(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL E_DistanceBetween(AMX *pAMX, cell *aParams);
};
const u32 CMAST_CMeshEntityWrapper_uMaxPoolSize = 100;


class CMAST_AlarmNetWrapper
{
public:
	static cell AMX_NATIVE_CALL E_AlarmNetFind(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_TurnOnAlarmNet(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_TurnOffAlarmNet(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_IsAlarmNetOn(AMX *pAMX, cell *aParams);
};


class CMAST_SpawnSysWrapper
{
public:
	static cell AMX_NATIVE_CALL E_SpawnNetFind(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_TurnOnSpawnNet(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_TurnOffSpawnNet(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_GetSpawnNetDefCon(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL E_SetSpawnNetDefCon(AMX *pAMX, cell *aParams);
};


class CMAST_MeshEntityWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static cell AMX_NATIVE_CALL ME_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_Find(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_AddToWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_RemoveFromWorld(AMX *pAMX, cell *aParams);
	// This init takes only a mesh as a parameter.
	static cell AMX_NATIVE_CALL ME_Init1(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_Init2(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_AnimSelect(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_SetAnimSpeed(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_GetAnimSpeed(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_SetAnimClamp(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_EnableAnim(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_GetUnitAnimPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_GetAnimPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_SetUnitAnimPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_SetAnimPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL ME_SetAlwaysWork(AMX *pAMX, cell *aParams);

private:
	static u32 m_uNumAcquired;
	static CMeshEntity **m_papPool;	
//	static s32 m_nNextInPool;
//	static CMeshEntity *m_aPool;
};

class CMAST_BotWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static void Work();
	static BOOL InitLevel();
	static void UninitLevel();

	static cell AMX_NATIVE_CALL Bot_Find(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_AddToWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_RemoveFromWorld(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_GetPlayer(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_IsDeadOrDying(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_IsDead(AMX *pAMX, cell *aParams);
	// Natives imported by retail scripts that this source lacked (behavior from retail main.dol):
	static cell AMX_NATIVE_CALL Bot_IsPosessed(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_IsRecruited(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_Recruit(AMX *pAMX, cell *aParams);

	//
	// AI functions
	//
	static cell AMX_NATIVE_CALL Bot_JobWait(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_JobWaitAndLook(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_JobWander(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_JobPatrol(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_GoalGotoPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_GotoE(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_GotoEO(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_Attack(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_FacePt(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_FaceE(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_FacePlayer(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_LoadTalk(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_Talk(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Bot_FollowLeader(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_SetBaseSpeed(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_ClrAttackSpecs(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_SetAttackSpecs( AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_TurnOnPercep(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_TurnOffPercep(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_UsePercep(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_IgnorePercep(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_IsPercepOn(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_IsPercepInUse(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Ai_EnableUnarmedPlayerRule(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Ai_DisableUnarmedPlayerRule(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Ai_IsUnarmedPlayerRuleEnabled(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Ai_CountEnemiesNear(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Ai_CountFriendsNear(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_CanSeeBot(AMX *pAMX, cell *aParams);

	//
	//  functions
	//
	static cell AMX_NATIVE_CALL Bot_GetAnimPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_FreezePlayer(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_UnfreezePlayer(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Bot_AttachAnim(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_DetachAnim(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_MaskUBody(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_MaskLBody(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Bot_LoadAnim(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Bot_PlayAnim(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Bot_TurnAround( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL Bot_IsTurnedAround( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL Bot_SpotLightOn( AMX *pAMX, cell *aParams );

	static cell AMX_NATIVE_CALL Bot_ZomStartForming( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL Bot_ZomFloatPieces( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL Bot_ZomCollapse( AMX *pAMX, cell *aParams );

	static cell AMX_NATIVE_CALL Bot_PossessExit( AMX *pAMX, cell *aParams );

	static cell AMX_NATIVE_CALL BotBlink_Acquire(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotBlink_Release(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotBlink_Init1(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotBlink_AbortScope(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL BotGlitch_FallDown(AMX *pAMX, cell *aParams);

private:
	static CFAnimInst *m_pAnimInst;
	static BOOL m_bIsAnimating;
	static CBot *m_pbotAnimating;
	static CEntityControl *m_pBotControls; 
};

class CMAST_BTIWrapper
{
public:
	static cell AMX_NATIVE_CALL BTI_AcquireBTI( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL BTI_ReleaseBTI( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL BTI_InitBTI( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL BTI_PlayBTI( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL BTI_IsBTIPlaying( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL BTI_SetAudio2D( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL BTI_SetAudioRadius( AMX *pAMX, cell *aParams );
	static cell AMX_NATIVE_CALL BTI_DuckAudWhenPlay( AMX *pAMX, cell *aParams );
};

class CMAST_DoorWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static cell AMX_NATIVE_CALL Door_Find(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Door_IsValid(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Door_GotoPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Door_GetUnitPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Door_GetUnitPosMap(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Door_SetLock(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Door_SnapToPos(AMX *pAMX, cell *aParams);
};


class CMAST_Vec3ObjWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static cell AMX_NATIVE_CALL FV3Points1_Init1(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL FV3Points2_Init1(AMX *pAMX, cell *aParams);
};

class CMAST_QuatObjWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static cell AMX_NATIVE_CALL FQuatSLERP_Init1(AMX *pAMX, cell *aParams);
};

//This is a spline based wrapper system
class CMAST_CamWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	static cell AMX_NATIVE_CALL Cam_Init(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Cam_Activate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Cam_Deactivate(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Cam_Shake(AMX *pAMX, cell *aParams);

private:
	static CCamManualInfo *m_pCamInfo;
	static BOOL m_bCamInitted;
};


//This is a max exported based wrapper system
class CMAST_CamAnimWrapper
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();
	static void Work();

	static cell AMX_NATIVE_CALL CamAnim_GetHandle(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamAnim_Start(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamAnim_StartEx(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamAnim_StartEx2(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL CamAnim_End(AMX *pAMX, cell *aParams);
//	static cell AMX_NATIVE_CALL CamAnim_Shake(AMX *pAMX, cell *aParams);

private:
	static void CamAnim_EndCutscene();
	static void CamAnim_Start( CFCamAnimInst* pCamAnimInst, BOOL bLetterBoxImmediate, BOOL bDisableAIAttackMode );

	static CCamCutsceneInfo *m_pCamInfo;
};

// TIMER CLASS
typedef struct {
	u16 uTimerStats;	
	u16 uTriggerCount; //number of times this timer has triggered an event. 
	f32 fTimer;
} CMAST_ETimer_t;

class CMAST_Timer
{
public:
	static BOOL InitLevel();
	static void UninitLevel();
	static void Work();

	static void CheckPointSave( void );
	static void CheckPointRestore( void );

	static cell AMX_NATIVE_CALL Timer_Set(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Timer_IsDone(AMX *pAMX, cell *aParams);
	//new functions for multiple event driver timers
	static cell AMX_NATIVE_CALL Timer_AcquireETimer( AMX *pAMX, cell *aparams );
	static cell AMX_NATIVE_CALL Timer_ReleaseETimer( AMX *pAMX, cell *aparams );
	static cell AMX_NATIVE_CALL Timer_SetETimer( AMX *pAMX, cell *aparams );
	static cell AMX_NATIVE_CALL Timer_ShowIcon( AMX *pAMX, cell *aparams );
	static cell AMX_NATIVE_CALL Timer_HideIcon( AMX *pAMX, cell *aparams );
	
private:

	enum {
		_ETIMER_FLAGS_ACQUIRED				= 0x0001,	// The ETimer is acquired by the scripts.
		_ETIMER_FLAGS_ACTIVE				= 0x0002,	// The ETimer is currently active (timing)
		_ETIMER_FLAGS_NONE					= 0x0000
	};


	static f32 m_fTimer;
	static CMAST_ETimer_t m_ETimers[NUM_MA_ETIMERS];
	static u32 m_nTimerEvent;
	static CMAST_ETimer_t *m_pIconTimer;

};

class CMAST_Hud
{
public:
	static BOOL InitSystem();
	static void UninitSystem();

	static cell AMX_NATIVE_CALL HUD_StartCAMsg(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL HUD_RadioStartBank(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL HUD_RadioStartStrm(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL HUD_RadioStop(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL HUD_RadioIsPlaying(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL HUD_SetMissionText(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL HUD_GetMissionIndex(AMX *pAMX, cell *aParams);

private:
};

class CMAST_GoodieBag
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();

	// GI stands for goodie instance.
	// These operate on the goodie bag as a whole.
	static cell AMX_NATIVE_CALL GB_GetNumGIs(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_GetProb(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_SetProb(AMX *pAMX, cell *aParams);

	// These operate on specific goodie instances.
	static cell AMX_NATIVE_CALL GB_GetGoodieType(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_SetGoodieType(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_GetGoodieQuant1(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_SetGoodieQuant1(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_GetGoodieQuant2(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_SetGoodieQuant2(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_GetGoodieProb(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL GB_SetGoodieProb(AMX *pAMX, cell *aParams);
};

class CMAST_Music
{
public:
	static cell AMX_NATIVE_CALL Music_Play(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Music_PlayEx(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Music_PlayDefault(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Music_PlayDefaultEx(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Music_Stop(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL StreamingSfx_Play(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL StreamingSfx_PlayEx(AMX *pAMX, cell *aParams);	
	
	static cell AMX_NATIVE_CALL Speech_Play(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Speech_Stop(AMX *pAMX, cell *aParams);	
};

//These functions operate on an entity, so I would think they should reside 
//in the entity wrapper class.  However, this wrapper already existed for
//the ambient audio play and stop functions, so I'll put the rest of the 
//audio functions in here as well, even though they operate on an entity.
class CMAST_Audio
{
public:
	static cell AMX_NATIVE_CALL Audio_GetSFXHandle(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_SetAmbientSFX(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_GetAmbientSFX(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_SetAmbientVol(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_GetAmbientVol(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_SetAmbientRad(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_GetAmbientRad(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_PlayAmbient(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_StopAmbient(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_Play2DSound(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Audio_Play2DSoundEx(AMX *pAMX, cell *aParams);	// retail script native
};


class CMAST_Movie
{
public:
	static cell AMX_NATIVE_CALL Movie_GetHandle(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Movie_Play(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Movie_PlayEx(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Movie_Stop(AMX *pAMX, cell *aParams);
};


class CMAST_Particle
{
public:
	static cell AMX_NATIVE_CALL Particle_Find(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Particle_Start(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Particle_Stop(AMX *pAMX, cell *aParams);
};

class CMAST_Checkpoint
{
public:
	static cell AMX_NATIVE_CALL Checkpoint_Save(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Checkpoint_Save2(AMX *pAMX, cell *aParams);		// retail script native
};

#if 0
class CMAST_DestructEntityWrapper
{
public:
	static cell AMX_NATIVE_CALL DE_StartDestruct(AMX *pAMX, cell *aParams);
};
#endif

class CMAST_GameWrapper
{
public:
	static cell AMX_NATIVE_CALL Game_BeginCutScene(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Game_EndCutScene(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Game_WinLevel(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Game_DoPauseMode(AMX *pAMX, cell *aParams);
	// Retail script natives:
	static cell AMX_NATIVE_CALL Console_Enable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Misc_SetValue(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Misc_GetValue(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Misc_GetDifficulty(AMX *pAMX, cell *aParams);
};

class CMAST_TackWrapper
{
public:
	static cell AMX_NATIVE_CALL Tack_Find(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Tack_GetHealth(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_SetHealth(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Tack_SetArmorProf(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Tack_Enable(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_Remove(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_ToEntityStat(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_ToEntityDyn(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_ToPos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_SetRange(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Tack_IsEnabled(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_GetRangeMin(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_GetRangeMax(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Tack_GetSnapped2Me(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Tack_GetPhysEntity(AMX *pAMX, cell *aParams);
};

class CMAST_PhysWrapper
{
public:
	static cell AMX_NATIVE_CALL Phys_Reset(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_Freeze(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_Kickstart(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_Sleep(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_SetDampening(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_SetPrimeSecs(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_SetGravity(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Phys_IsFrozen(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_IsSleeping(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_GetDampening(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_GetPrimeSecs(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_GetGravity(AMX *pAMX, cell *aParams);

	static cell AMX_NATIVE_CALL Phys_GetTackCount(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL Phys_GetTack(AMX *pAMX, cell *aParams);
};


class CMAST_SplineActorWrapper
{
public:
	//Spline Actor support functions -- These work on all Spline Actor types
	static cell AMX_NATIVE_CALL SActor_Init(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL SActor_SetTimePos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL SActor_GetTimePos(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL SActor_Reset(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL SActor_Start(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL SActor_Stop(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL SActor_IsDone(AMX *pAMX, cell *aParams);
	//GSActor (General Spline Actor) specific functions
	static cell AMX_NATIVE_CALL GSActor_Acquire(AMX *pAMX, cell *aParams);
	//HSActor (Horizontal Spline Actor) specific functions
	static cell AMX_NATIVE_CALL HSActor_Acquire(AMX *pAMX, cell *aParams);
	//SSActor (Swinging Spline Actor) specific functions
	static cell AMX_NATIVE_CALL SSActor_Acquire(AMX *pAMX, cell *aParams);
	//SOSActor (Static Orientation Spline Actor) specific functions
	static cell AMX_NATIVE_CALL SOSActor_Acquire(AMX *pAMX, cell *aParams);
};

class CMAST_BarterWrapper
{
public:
	static cell AMX_NATIVE_CALL Barter_MoveToPoint(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Barter_Remove(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Barter_SetAttract(AMX *pAMX, cell *aParams);	
};

class CMAST_VehicleWrapper
{
public:
	static cell AMX_NATIVE_CALL Vehicle_SetPath(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_ClearPath(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_SetPlayer(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_PlayerDrive(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_Driveable(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_MaxSpeed(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_PlayerGun(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_EjectDriver(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_EjectGunner(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_GetSpeed(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_DriverExit(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Vehicle_GunnerExit(AMX *pAMX, cell *aParams);	
};

class CMAST_FXWrapper
{
public:
	static cell AMX_NATIVE_CALL FX_Explosion_Find(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL FX_Explosion(AMX *pAMX, cell *aParams);
	static cell AMX_NATIVE_CALL FX_StompRing(AMX *pAMX, cell *aParams);	// retail script native
};

class CMAST_GoodieWrapper
{
public:
	static cell AMX_NATIVE_CALL G_GiveToGlitch(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL G_NotifyGoodieUsed(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL G_IsInInventory(AMX *pAMX, cell *aParams);	
};

class CMAST_DebrisWrapper
{
public:
	static cell AMX_NATIVE_CALL Deb_RumbleShake(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Deb_GetRumbleShake(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Deb_SetAutoShake(AMX *pAMX, cell *aParams);	
	static cell AMX_NATIVE_CALL Deb_Shake(AMX *pAMX, cell *aParams);	
};

#endif
