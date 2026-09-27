//////////////////////////////////////////////////////////////////////////////////////
// MAScriptTypes.cpp - Mettle Arms script functions.
//
// Author: Justine Link
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
#include "fang.h"
#include "MAScriptTypes.h"
#include "AI/AiApi.h"
#include "AI\AIBrain.h"
#include "AI/AIBrainMan.h"
#include "AI/AIBrainmems.h"
#include "AI/AIBrainUtils.h"
#include "ai/aigroup.h"
#include "AlarmSys.h"
#include "bartersystem.h"
#include "bottalkinst.h"
#include "CamManual.h"
#include "CamCutscene.h"
#include "damage.h"
#include "eboomer.h"
#include "Door.h"
#include "eparticle.h"
#include "espline.h"
#include "esphere.h"
#include "fclib.h"
#include "FNativeUtil.h"
#include "FPointPath1.h"
#include "FPointPath2.h"
#include "FQuatSLERP.h"
#include "fresload.h"
#include "FScriptSystem.h"
#include "fverlet.h"
#include "game.h"
#include "gamecam.h"
#include "gamepad.h"
#include "gamesave.h"
#include "Hud2.h"
#include "letterbox.h"
#include "level.h"
#include "meshentity.h"
#include "player.h"
#include "SplineActor.h"
#include "spawnsys.h"
#include "vehicle.h"
#include "botglitch.h"
#include "explosion.h"
#include "botzom.h"
#include "Collectable.h"
#include "edebris.h"
#include "gstring.h"
#include "econsole.h"
#include "difficulty.h"
#include "fxshockwave.h"

cell AMX_NATIVE_CALL Bot_SetBuddyCtrl(AMX *pAMX, cell *aParams);
extern BOOL AIBrain_TalkModeCB( u32 uTalkModeCBControl, void *pvData1, void *pvData2 );


AMX_NATIVE_INFO MAScriptTypes_aMAEventNatives[] =
{
	// CEntity functions...
	"E_Find",							&CMAST_EntityWrapper::E_Find,
	"E_FindInWorld",					&CMAST_EntityWrapper::E_FindInWorld,
	"E_FindOutWorld",					&CMAST_EntityWrapper::E_FindOutWorld,

	"E_IsValid",						&CMAST_EntityWrapper::E_IsValid,
	"E_IsNamed",						&CMAST_EntityWrapper::E_IsNamed,
	"E_IsBot",							&CMAST_EntityWrapper::E_IsBot,
	"E_IsDoor",							&CMAST_EntityWrapper::E_IsDoor,
	"E_IsMesh",							&CMAST_EntityWrapper::E_IsMesh,
	"E_IsSwitch",						&CMAST_EntityWrapper::E_IsSwitch,

	"E_EnableTripwire",					&CMAST_EntityWrapper::E_EnableTripwire,

	"E_AddToWorld",						&CMAST_EntityWrapper::E_AddToWorld,
	"E_RemoveFromWorld",				&CMAST_EntityWrapper::E_RemoveFromWorld,
	"E_IsInWorld",						&CMAST_EntityWrapper::E_IsInWorld,
	"E_IsTargetable",					&CMAST_EntityWrapper::E_IsTargetable,
	"E_IsHittable",						&CMAST_EntityWrapper::E_IsHittable,
	"E_IsActionable",					&CMAST_EntityWrapper::E_IsActionable,
	"E_IsTripwire",						&CMAST_EntityWrapper::E_IsTripwire,
	"E_SetNormHealth",					&CMAST_EntityWrapper::E_SetNormHealth,
	"E_GetNormHealth",					&CMAST_EntityWrapper::E_GetNormHealth,
	"E_SetTargetable",					&CMAST_EntityWrapper::E_SetTargetable,
	"E_SnapToE",						&CMAST_EntityWrapper::E_SnapToE,

	"E_SetExplodArmor",					&CMAST_EntityWrapper::E_SetExplodArmor,
	"E_SetFlameArmor",					&CMAST_EntityWrapper::E_SetFlameArmor,
	"E_SetBulletArmor",					&CMAST_EntityWrapper::E_SetBulletArmor,

	"E_GetPos",							&CMAST_EntityWrapper::E_GetPos,
	"E_TouchOfDeath",					&CMAST_EntityWrapper::E_TouchOfDeath,
	"E_SetDamageable",					&CMAST_EntityWrapper::E_SetDamageable,
	"E_IsDamageable",					&CMAST_EntityWrapper::E_IsDamageable,
	"E_Blast",							&CMAST_EntityWrapper::E_Blast,

	"E_DistanceBetween",				&CMAST_EntityWrapper::E_DistanceBetween,

	// CMeshEntity functions...
	"ME_Acquire",						&CMAST_MeshEntityWrapper::ME_Acquire,
	"ME_Release",						&CMAST_MeshEntityWrapper::ME_Release,
	"ME_Find",							&CMAST_MeshEntityWrapper::ME_Find,
	"ME_IsValid",						&CMAST_MeshEntityWrapper::ME_IsValid,
	"ME_Init1",							&CMAST_MeshEntityWrapper::ME_Init1,
	"ME_Init2",							&CMAST_MeshEntityWrapper::ME_Init2,
	"ME_AddToWorld",					&CMAST_MeshEntityWrapper::ME_AddToWorld,
	"ME_RemoveFromWorld",				&CMAST_MeshEntityWrapper::ME_RemoveFromWorld,
	"ME_AnimSelect",					&CMAST_MeshEntityWrapper::ME_AnimSelect,
	"ME_SetAnimSpeed",					&CMAST_MeshEntityWrapper::ME_SetAnimSpeed,
	"ME_GetAnimSpeed",					&CMAST_MeshEntityWrapper::ME_GetAnimSpeed,
	"ME_SetAnimClamp",					&CMAST_MeshEntityWrapper::ME_SetAnimClamp,
	"ME_EnableAnim",					&CMAST_MeshEntityWrapper::ME_EnableAnim,
	"ME_GetUnitAnimPos",				&CMAST_MeshEntityWrapper::ME_GetUnitAnimPos,
	"ME_GetAnimPos",					&CMAST_MeshEntityWrapper::ME_GetAnimPos,
	"ME_SetUnitAnimPos",				&CMAST_MeshEntityWrapper::ME_SetUnitAnimPos,
	"ME_SetAnimPos",					&CMAST_MeshEntityWrapper::ME_SetAnimPos,
	"ME_SetAlwaysWork",					&CMAST_MeshEntityWrapper::ME_SetAlwaysWork,

	// Bot functions...
	"Bot_Find",							&CMAST_BotWrapper::Bot_Find,
	"Bot_IsValid",						&CMAST_BotWrapper::Bot_IsValid,
	"Bot_AddToWorld",					&CMAST_BotWrapper::Bot_AddToWorld,
	"Bot_RemoveFromWorld",				&CMAST_BotWrapper::Bot_RemoveFromWorld,
	"Bot_IsDeadOrDying",				&CMAST_BotWrapper::Bot_IsDeadOrDying,
	"Bot_IsDead",						&CMAST_BotWrapper::Bot_IsDead,
	"Bot_IsPosessed",					&CMAST_BotWrapper::Bot_IsPosessed,		// retail spelling
	"Bot_IsRecruited",					&CMAST_BotWrapper::Bot_IsRecruited,
	"Bot_Recruit",						&CMAST_BotWrapper::Bot_Recruit,

	"Bot_JobWander",					&CMAST_BotWrapper::Bot_JobWander,
	"Bot_GetPlayer",					&CMAST_BotWrapper::Bot_GetPlayer,
	"Bot_GoalGotoPos",					&CMAST_BotWrapper::Bot_GoalGotoPos,
	"Bot_GotoE",						&CMAST_BotWrapper::Bot_GotoE,
	"Bot_GotoEO",						&CMAST_BotWrapper::Bot_GotoEO,
	"Bot_JobWait",						&CMAST_BotWrapper::Bot_JobWait,
	"Bot_JobWaitAndLook",				&CMAST_BotWrapper::Bot_JobWaitAndLook,
	"Bot_JobPatrol",					&CMAST_BotWrapper::Bot_JobPatrol,
	"Bot_Attack",						&CMAST_BotWrapper::Bot_Attack,
	"Bot_FacePt",						&CMAST_BotWrapper::Bot_FacePt,
	"Bot_FaceE",						&CMAST_BotWrapper::Bot_FaceE,
	"Bot_FacePlayer",					&CMAST_BotWrapper::Bot_FacePlayer,
	"Bot_LoadTalk",						&CMAST_BotWrapper::Bot_LoadTalk,
	"Bot_Talk",							&CMAST_BotWrapper::Bot_Talk,

	"Bot_FollowLeader",					&CMAST_BotWrapper::Bot_FollowLeader,
	"Bot_SetBuddyCtrl",					&Bot_SetBuddyCtrl,
	"Bot_SetBaseSpeed",					&CMAST_BotWrapper::Bot_SetBaseSpeed,

	"Bot_ClrAttackSpecs",				&CMAST_BotWrapper::Bot_ClrAttackSpecs,
	"Bot_SetAttackSpecs",				&CMAST_BotWrapper::Bot_SetAttackSpecs,

	"Bot_TurnOnPercep",					&CMAST_BotWrapper::Bot_TurnOnPercep,
	"Bot_TurnOffPercep",				&CMAST_BotWrapper::Bot_TurnOffPercep,
	"Bot_UsePercep",					&CMAST_BotWrapper::Bot_UsePercep,
	"Bot_IgnorePercep",					&CMAST_BotWrapper::Bot_IgnorePercep,
	"Bot_IsPercepOn",					&CMAST_BotWrapper::Bot_IsPercepOn,
	"Bot_IsPercepInUse",				&CMAST_BotWrapper::Bot_IsPercepInUse,

	"Ai_EnableUPRule",					&CMAST_BotWrapper::Ai_EnableUnarmedPlayerRule,
	"Ai_DisableUPRule",					&CMAST_BotWrapper::Ai_DisableUnarmedPlayerRule,
	"Ai_IsUPREnabled",					&CMAST_BotWrapper::Ai_IsUnarmedPlayerRuleEnabled,

	"Ai_CountEnemiesNear",				&CMAST_BotWrapper::Ai_CountEnemiesNear,
	"Ai_CountFriendsNear",				&CMAST_BotWrapper::Ai_CountFriendsNear,

	"Bot_CanSeeBot",					&CMAST_BotWrapper::Bot_CanSeeBot,

	"Bot_LoadAnim",						&CMAST_BotWrapper::Bot_LoadAnim,
	"Bot_PlayAnim",						&CMAST_BotWrapper::Bot_PlayAnim,
	"Bot_GetAnimPos",					&CMAST_BotWrapper::Bot_GetAnimPos,
	"Bot_FreezePlayer",					&CMAST_BotWrapper::Bot_FreezePlayer,
	"Bot_UnfreezePlayer",				&CMAST_BotWrapper::Bot_UnfreezePlayer,

	"Bot_AttachAnim",					&CMAST_BotWrapper::Bot_AttachAnim,
	"Bot_DetachAnim",					&CMAST_BotWrapper::Bot_DetachAnim,
	"Bot_MaskUBody",					&CMAST_BotWrapper::Bot_MaskUBody,
	"Bot_MaskLBody",					&CMAST_BotWrapper::Bot_MaskLBody,

	"Bot_TurnAround",					&CMAST_BotWrapper::Bot_TurnAround,
	"Bot_IsTurnedAround",				&CMAST_BotWrapper::Bot_IsTurnedAround,
	"Bot_SpotLightOn",					&CMAST_BotWrapper::Bot_SpotLightOn,

	"Bot_ZomStartForming",				&CMAST_BotWrapper::Bot_ZomStartForming,
	"Bot_ZomFloatPieces",				&CMAST_BotWrapper::Bot_ZomFloatPieces,
	"Bot_ZomCollapse",					&CMAST_BotWrapper::Bot_ZomCollapse,

	"Bot_PossessExit",					&CMAST_BotWrapper::Bot_PossessExit,

	// bot talk instance functions
	"BTI_AcquireBTI",					&CMAST_BTIWrapper::BTI_AcquireBTI,
	"BTI_ReleaseBTI",					&CMAST_BTIWrapper::BTI_ReleaseBTI,
	"BTI_InitBTI",						&CMAST_BTIWrapper::BTI_InitBTI,
	"BTI_PlayBTI",						&CMAST_BTIWrapper::BTI_PlayBTI,
	"BTI_IsBTIPlaying",					&CMAST_BTIWrapper::BTI_IsBTIPlaying,
	"BTI_SetAudio2D",					&CMAST_BTIWrapper::BTI_SetAudio2D,
	"BTI_SetAudioRadius",				&CMAST_BTIWrapper::BTI_SetAudioRadius,
	"BTI_DuckAudWhenPlay",				&CMAST_BTIWrapper::BTI_DuckAudWhenPlay,

	// BotBlink functions...
	"BotBlink_Acquire",					&CMAST_BotWrapper::BotBlink_Acquire,
	"BotBlink_Release",					&CMAST_BotWrapper::BotBlink_Release,
	"BotBlink_Init1",					&CMAST_BotWrapper::BotBlink_Init1,
	"BotBlink_AbortScope",				&CMAST_BotWrapper::BotBlink_AbortScope,
	"BotGlitch_FallDown",				&CMAST_BotWrapper::BotGlitch_FallDown,

	// Door functions...
	"Door_Find",						&CMAST_DoorWrapper::Door_Find,
	"Door_IsValid",						&CMAST_DoorWrapper::Door_IsValid,
	"Door_GotoPos",						&CMAST_DoorWrapper::Door_GotoPos,
	"Door_GetUnitPos",					&CMAST_DoorWrapper::Door_GetUnitPos,
	"Door_GetUnitPosMap",				&CMAST_DoorWrapper::Door_GetUnitPosMap,
	"Door_SetLock",						&CMAST_DoorWrapper::Door_SetLock,
	"Door_SnapToPos",					&CMAST_DoorWrapper::Door_SnapToPos,

	// Special FVec3Obj functions...
	"FV3Points1_Init1",					&CMAST_Vec3ObjWrapper::FV3Points1_Init1,
	"FV3Points2_Init1",					&CMAST_Vec3ObjWrapper::FV3Points2_Init1,

	// Special FQuatObj functions...
	"FQuatSLERP_Init1",					&CMAST_QuatObjWrapper::FQuatSLERP_Init1,

	"Cam_Init",							&CMAST_CamWrapper::Cam_Init,
	"Cam_Activate",						&CMAST_CamWrapper::Cam_Activate,
	"Cam_Deactivate",					&CMAST_CamWrapper::Cam_Deactivate,
	"Cam_Shake",						&CMAST_CamWrapper::Cam_Shake,

	//Camera Animation cutscene functions
	"CamAnim_GetHandle",				&CMAST_CamAnimWrapper::CamAnim_GetHandle,
	"CamAnim_Start",					&CMAST_CamAnimWrapper::CamAnim_Start,
	"CamAnim_StartEx",					&CMAST_CamAnimWrapper::CamAnim_StartEx,
	"CamAnim_StartEx2",					&CMAST_CamAnimWrapper::CamAnim_StartEx2,
	"CamAnim_End",						&CMAST_CamAnimWrapper::CamAnim_End,

	"E_AlarmNetFind",					&CMAST_AlarmNetWrapper::E_AlarmNetFind,
	"E_TurnOnAlarmNet",					&CMAST_AlarmNetWrapper::E_TurnOnAlarmNet,
	"E_TurnOffAlarmNet",				&CMAST_AlarmNetWrapper::E_TurnOffAlarmNet,
	"E_IsAlarmNetOn",					&CMAST_AlarmNetWrapper::E_IsAlarmNetOn,

	"E_SpawnNetFind",					&CMAST_SpawnSysWrapper::E_SpawnNetFind,
	"E_TurnOnSpawnNet",					&CMAST_SpawnSysWrapper::E_TurnOnSpawnNet,
	"E_TurnOffSpawnNet",				&CMAST_SpawnSysWrapper::E_TurnOffSpawnNet,
	"E_GetSpawnNetDefCon",				&CMAST_SpawnSysWrapper::E_GetSpawnNetDefCon,
	"E_SetSpawnNetDefCon",				&CMAST_SpawnSysWrapper::E_SetSpawnNetDefCon,

	"Timer_Set",						&CMAST_Timer::Timer_Set,
	"Timer_IsDone",						&CMAST_Timer::Timer_IsDone,
	"Timer_AcquireETimer",				&CMAST_Timer::Timer_AcquireETimer,
	"Timer_ReleaseETimer",				&CMAST_Timer::Timer_ReleaseETimer,
	"Timer_SetETimer",					&CMAST_Timer::Timer_SetETimer,
	"Timer_ShowIcon",					&CMAST_Timer::Timer_ShowIcon,
	"Timer_HideIcon",					&CMAST_Timer::Timer_HideIcon,

	// HUD Functions...
	"HUD_StartCAMsg",					&CMAST_Hud::HUD_StartCAMsg,
	"HUD_RadioStartBank",				&CMAST_Hud::HUD_RadioStartBank,
	"HUD_RadioStartStrm",				&CMAST_Hud::HUD_RadioStartStrm,
	"HUD_RadioStop",					&CMAST_Hud::HUD_RadioStop,
	"HUD_RadioIsPlaying",				&CMAST_Hud::HUD_RadioIsPlaying,
	"HUD_SetMissionText",				&CMAST_Hud::HUD_SetMissionText,
	"HUD_GetMissionIndex",				&CMAST_Hud::HUD_GetMissionIndex,

	// MUSIC Functions...
	"Music_Play",						&CMAST_Music::Music_Play,
	"Music_PlayEx",						&CMAST_Music::Music_PlayEx,
	"Music_PlayDefault",				&CMAST_Music::Music_PlayDefault,
	"Music_PlayDefaultEx",				&CMAST_Music::Music_PlayDefaultEx,
	"Music_Stop",						&CMAST_Music::Music_Stop,
	"StreamingSfx_Play",				&CMAST_Music::StreamingSfx_Play,
	"StreamingSfx_PlayEx",				&CMAST_Music::StreamingSfx_PlayEx,
	"Speech_Play",						&CMAST_Music::Speech_Play,
	"Speech_Stop",						&CMAST_Music::Speech_Stop,
	

	// ENTITY Ambient Audio Functions
	"Audio_GetSFXHandle",				&CMAST_Audio::Audio_GetSFXHandle,
	"Audio_SetAmbientSFX",				&CMAST_Audio::Audio_SetAmbientSFX,
	"Audio_GetAmbientSFX",				&CMAST_Audio::Audio_GetAmbientSFX,
	"Audio_SetAmbientVol",				&CMAST_Audio::Audio_SetAmbientVol,
	"Audio_GetAmbientVol",				&CMAST_Audio::Audio_GetAmbientVol,
	"Audio_SetAmbientRad",				&CMAST_Audio::Audio_SetAmbientRad,
	"Audio_GetAmbientRad",				&CMAST_Audio::Audio_GetAmbientRad,
	"Audio_PlayAmbient",				&CMAST_Audio::Audio_PlayAmbient,
	"Audio_StopAmbient",				&CMAST_Audio::Audio_StopAmbient,
	"Audio_Play2DSound",				&CMAST_Audio::Audio_Play2DSound,
	"Audio_Play2DSoundEx",				&CMAST_Audio::Audio_Play2DSoundEx,

	"Movie_GetHandle",					&CMAST_Movie::Movie_GetHandle,
	"Movie_Play",						&CMAST_Movie::Movie_Play,
	"Movie_PlayEx",						&CMAST_Movie::Movie_PlayEx,
	"Movie_Stop",						&CMAST_Movie::Movie_Stop,

	"Particle_Find",					&CMAST_Particle::Particle_Find,
	"Particle_Start",					&CMAST_Particle::Particle_Start,
	"Particle_Stop",					&CMAST_Particle::Particle_Stop,

	"Checkpoint_Save",					&CMAST_Checkpoint::Checkpoint_Save,
	"Checkpoint_Save2",					&CMAST_Checkpoint::Checkpoint_Save2,

//	"DE_StartDestruct",					&CMAST_DestructEntityWrapper::DE_StartDestruct,

	"Game_BeginCutScene",				&CMAST_GameWrapper::Game_BeginCutScene,
	"Game_EndCutScene",					&CMAST_GameWrapper::Game_EndCutScene,
	"Game_WinLevel",					&CMAST_GameWrapper::Game_WinLevel,
	"Game_DoPauseMode",					&CMAST_GameWrapper::Game_DoPauseMode,
	"Console_Enable",					&CMAST_GameWrapper::Console_Enable,
	"Misc_SetValue",					&CMAST_GameWrapper::Misc_SetValue,
	"Misc_GetValue",					&CMAST_GameWrapper::Misc_GetValue,
	"Misc_GetDifficulty",				&CMAST_GameWrapper::Misc_GetDifficulty,

	// Tack functions...
	"Tack_Find",						&CMAST_TackWrapper::Tack_Find,

	"Tack_GetHealth",					&CMAST_TackWrapper::Tack_GetHealth,
	"Tack_SetHealth",					&CMAST_TackWrapper::Tack_SetHealth,

	"Tack_SetArmorProf",				&CMAST_TackWrapper::Tack_SetArmorProf,

	"Tack_Enable",						&CMAST_TackWrapper::Tack_Enable,
	"Tack_Remove",						&CMAST_TackWrapper::Tack_Remove,
	"Tack_ToEntityStat",				&CMAST_TackWrapper::Tack_ToEntityStat,
	"Tack_ToEntityDyn",					&CMAST_TackWrapper::Tack_ToEntityDyn,
	"Tack_ToPos",						&CMAST_TackWrapper::Tack_ToPos,
	"Tack_SetRange",					&CMAST_TackWrapper::Tack_SetRange,

	"Tack_IsEnabled",					&CMAST_TackWrapper::Tack_IsEnabled,
	"Tack_GetRangeMin",					&CMAST_TackWrapper::Tack_GetRangeMin,
	"Tack_GetRangeMax",					&CMAST_TackWrapper::Tack_GetRangeMax,
	"Tack_GetSnapped2Me",				&CMAST_TackWrapper::Tack_GetSnapped2Me,

	"Tack_GetPhysEntity",				&CMAST_TackWrapper::Tack_GetPhysEntity,

	// Phys functions...
	"Phys_Reset",						&CMAST_PhysWrapper::Phys_Reset,
	"Phys_Freeze",						&CMAST_PhysWrapper::Phys_Freeze,
	"Phys_Kickstart",					&CMAST_PhysWrapper::Phys_Kickstart,
	"Phys_Sleep",						&CMAST_PhysWrapper::Phys_Sleep,
	"Phys_SetDampening",				&CMAST_PhysWrapper::Phys_SetDampening,
	"Phys_SetPrimeSecs",				&CMAST_PhysWrapper::Phys_SetPrimeSecs,
	"Phys_SetGravity",					&CMAST_PhysWrapper::Phys_SetGravity,

	"Phys_IsFrozen",					&CMAST_PhysWrapper::Phys_IsFrozen,
	"Phys_IsSleeping",					&CMAST_PhysWrapper::Phys_IsSleeping,
	"Phys_GetDampening",				&CMAST_PhysWrapper::Phys_GetDampening,
	"Phys_GetPrimeSecs",				&CMAST_PhysWrapper::Phys_GetPrimeSecs,
	"Phys_GetGravity",					&CMAST_PhysWrapper::Phys_GetGravity,

	"Pnys_GetTackCount",				&CMAST_PhysWrapper::Phys_GetTackCount,
	"Phys_GetTack",						&CMAST_PhysWrapper::Phys_GetTack,

	// SplineActor functions...
	"SActor_Init",						&CMAST_SplineActorWrapper::SActor_Init,
	"SActor_SetTimePos",				&CMAST_SplineActorWrapper::SActor_SetTimePos,
	"SActor_GetTimePos",				&CMAST_SplineActorWrapper::SActor_GetTimePos,
	"SActor_Reset",						&CMAST_SplineActorWrapper::SActor_Reset,
	"SActor_Start",						&CMAST_SplineActorWrapper::SActor_Start,
	"SActor_Stop",						&CMAST_SplineActorWrapper::SActor_Stop,
	"SActor_IsDone",					&CMAST_SplineActorWrapper::SActor_IsDone,
	"GSActor_Acquire",					&CMAST_SplineActorWrapper::GSActor_Acquire,
	"HSActor_Acquire",					&CMAST_SplineActorWrapper::HSActor_Acquire,
	"SSActor_Acquire",					&CMAST_SplineActorWrapper::SSActor_Acquire,
	"SOSActor_Acquire",					&CMAST_SplineActorWrapper::SOSActor_Acquire,

	// Barter functions...
	"Barter_MoveToPoint",				&CMAST_BarterWrapper::Barter_MoveToPoint,
	"Barter_Remove",					&CMAST_BarterWrapper::Barter_Remove,
	"Barter_SetAttract",				&CMAST_BarterWrapper::Barter_SetAttract,

	"Vehicle_SetPath",					&CMAST_VehicleWrapper::Vehicle_SetPath,
	"Vehicle_ClearPath",				&CMAST_VehicleWrapper::Vehicle_ClearPath,
	"Vehicle_SetPlayer",				&CMAST_VehicleWrapper::Vehicle_SetPlayer,
	"Vehicle_PlayerDrive",				&CMAST_VehicleWrapper::Vehicle_PlayerDrive,
	"Vehicle_Driveable",				&CMAST_VehicleWrapper::Vehicle_Driveable,
	"Vehicle_PlayerGun",				&CMAST_VehicleWrapper::Vehicle_PlayerGun,
	"Vehicle_MaxSpeed",					&CMAST_VehicleWrapper::Vehicle_MaxSpeed,
	"Vehicle_EjectDriver",				&CMAST_VehicleWrapper::Vehicle_EjectDriver,
	"Vehicle_EjectGunner",				&CMAST_VehicleWrapper::Vehicle_EjectGunner,
	"Vehicle_GetSpeed",					&CMAST_VehicleWrapper::Vehicle_GetSpeed,
	"Vehicle_DriverExit",				&CMAST_VehicleWrapper::Vehicle_DriverExit,
	"Vehicle_GunnerExit",				&CMAST_VehicleWrapper::Vehicle_GunnerExit,

	// Effects funcitons...
	"FX_Explosion_Find",				&CMAST_FXWrapper::FX_Explosion_Find,
	"FX_Explosion",						&CMAST_FXWrapper::FX_Explosion,
	"FX_StompRing",						&CMAST_FXWrapper::FX_StompRing,

	// Goodie functions...
	"G_GiveToGlitch",					&CMAST_GoodieWrapper::G_GiveToGlitch,
	"G_NotifyGoodieUsed",				&CMAST_GoodieWrapper::G_NotifyGoodieUsed,
	"G_IsInInventory",					&CMAST_GoodieWrapper::G_IsInInventory,

	// Debris functions...
	"Deb_RumbleShake",					&CMAST_DebrisWrapper::Deb_RumbleShake,
	"Deb_GetRumbleShake",				&CMAST_DebrisWrapper::Deb_GetRumbleShake,
	"Deb_SetAutoShake",					&CMAST_DebrisWrapper::Deb_SetAutoShake,
	"Deb_Shake",						&CMAST_DebrisWrapper::Deb_Shake,

	NULL,								NULL
};

// =============================================================================================================

BOOL CMAScriptTypes::InitSystem()
{
	AMX_NATIVE_INFO *pCurNative = &(MAScriptTypes_aMAEventNatives[0]);
	while(pCurNative->func != NULL)
	{
		FASSERT(pCurNative->name && fclib_strlen(pCurNative->name) < 20);  //HEHEHE 19 seems to be the magic number not to exceed.
		CFScriptSystem::RegisterNative(pCurNative->name, pCurNative->func);
		++pCurNative;
	}

	if(!CMAST_MeshEntityWrapper::InitSystem())
	{
		return(FALSE);
	}
	if(!CMAST_BotWrapper::InitSystem())
	{
		return(FALSE);
	}
	if(!CMAST_DoorWrapper::InitSystem())
	{
		return(FALSE);
	}
	if(!CMAST_CamWrapper::InitSystem())
	{
		return(FALSE);
	}
	if(!CMAST_CamAnimWrapper::InitSystem())
	{
		return(FALSE);
	}
	if(!CMAST_GoodieBag::InitSystem())
	{
		return(FALSE);
	}

	return(TRUE);
}

void CMAScriptTypes::UninitSystem()
{
	CMAST_GoodieBag::UninitSystem();
	CMAST_CamWrapper::UninitSystem();
	CMAST_CamAnimWrapper::UninitSystem();
	CMAST_DoorWrapper::UninitSystem();
	CMAST_BotWrapper::UninitSystem();
	CMAST_MeshEntityWrapper::UninitSystem();

	return;
}

BOOL CMAScriptTypes::InitLevel()
{
	if(!CMAST_MeshEntityWrapper::InitLevel())
	{
		return(FALSE);
	}
	if(!CMAST_BotWrapper::InitLevel())
	{
		return(FALSE);
	}
	if(!CMAST_DoorWrapper::InitLevel())
	{
		return(FALSE);
	}
	if(!CMAST_CamWrapper::InitLevel())
	{
		return(FALSE);
	}
	if(!CMAST_CamAnimWrapper::InitLevel())
	{
		return(FALSE);
	}
	if(!CMAST_GoodieBag::InitLevel())
	{
		return(FALSE);
	}
	if(!CMAST_Timer::InitLevel())
	{
		return(FALSE);
	}

	return(TRUE);
}

void CMAScriptTypes::UninitLevel()
{
	CMAST_GoodieBag::UninitLevel();
	CMAST_CamWrapper::UninitLevel();
	CMAST_CamAnimWrapper::UninitLevel();
	CMAST_DoorWrapper::UninitLevel();
	CMAST_BotWrapper::UninitLevel();
	CMAST_MeshEntityWrapper::UninitLevel();
	CMAST_Timer::UninitLevel();
}

// =============================================================================================================

/////////////////////////////////////////////////////////////////////////////////////////////
// CEntity functions.

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_Find(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_Find", 1 );
	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[1]);
	CEntity *pE = CEntity::Find(szEntityName);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_Find() : Could not find entity named '%s'.", szEntityName);
	} 
	else 
	{
		if( pE->TypeBits() & ENTITY_BIT_GOODIE )
		{
			CCollectable *pCollect = (CCollectable *) pE;
			pCollect->SetUsedInScript( TRUE );
		}
	}

	return((cell)(pE));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_FindInWorld(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_FindInWorld", 1 );
	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[1]);
	CEntity *pE = CEntity::FindInWorld(szEntityName);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_FindInWorld() : Could not find entity named '%s'.", szEntityName);
	}
	else 
	{
		if( pE->TypeBits() & ENTITY_BIT_GOODIE )
		{
			CCollectable *pCollect = (CCollectable *) pE;
			pCollect->SetUsedInScript( TRUE );
		}
	}
	return((cell)(pE));

}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_FindOutWorld(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_FindOutWorld", 1 );
	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[1]);
	CEntity *pE = CEntity::FindOutOfWorld(szEntityName);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_FindOutWorld() : Could not find entity named '%s'.", szEntityName);
	}
	else 
	{
		if( pE->TypeBits() & ENTITY_BIT_GOODIE )
		{
			CCollectable *pCollect = (CCollectable *) pE;
			pCollect->SetUsedInScript( TRUE );
		}
	}
	return((cell)(pE));

}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsValid(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsValid", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	return((cell)(pE != NULL));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsNamed(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsNamed", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsNamed() : Passed in NULL entity.");
		return((cell)(0));
	}

	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[2]);

	return(pE->Name() && fclib_stricmp(pE->Name(), szEntityName) == 0);
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsBot(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsBot", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsBot() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)((pE->TypeBits() & ENTITY_BIT_BOT) != 0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsDoor(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsDoor", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsDoor() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)((pE->TypeBits() & ENTITY_BIT_DOOR) != 0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsMesh(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsMesh", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsMesh() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)((pE->TypeBits() & ENTITY_BIT_MESHENTITY) != 0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsSwitch(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsSwitch", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsSwitch() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)((pE->TypeBits() & ENTITY_BIT_SWITCH) != 0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_EnableTripwire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_EnableTripwire", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_EnableTripwire() : Passed in NULL entity.");
		return((cell)(0));
	}

	pE->ArmTripwire(aParams[2]);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_AddToWorld(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_AddToWorld", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_AddToWorld() : Passed in NULL entity.");
		return((cell)(0));
	}

	if (pE->TypeBits() & ENTITY_BIT_BOT && pE->IsInWorld() && ((CBot*) pE)->IsDead())
	{
		((CBot*) pE)->RemoveFromWorld();
	}
	
	if (pE->IsCreated())
	{
		pE->AddToWorld();
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_RemoveFromWorld(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_RemoveFromWorld", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_RemoveFromWorld() : Passed in NULL entity.");
		return((cell)(0));
	}

	pE->RemoveFromWorld();

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsInWorld(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsInWorld", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsInWorld() : Passed in NULL entity.");
		return((cell)(0));
	}

	BOOL bIs = (pE->IsInWorld() && (!(pE->TypeBits() & ENTITY_BIT_BOT) || !((CBot*) pE)->IsDeadOrDying()) );

	return((cell)bIs);
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsTargetable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsTargetable", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsTargetable() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)(pE->IsTargetable()));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsHittable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsHittable", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsHittable() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)(!pE->IsInvincible()));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsActionable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsActionable", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsActionable() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)(pE->IsActionable()));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsTripwire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsTripwire", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsTripwire() : Passed in NULL entity.");
		return((cell)(0));
	}

	return((cell)(pE->IsTripper()));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_SetNormHealth(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetNormHealth", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_SetNormHealth() : Passed in NULL entity.");
		return((cell)(0));
	}

	f32 fNewHealth = ConvertCellToF32(aParams[2]);
	pE->SetNormHealth(fNewHealth);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_GetNormHealth(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_GetNormHealth", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_GetNormHealth() : Passed in NULL entity.");
		return((cell)(0));
	}

	return( ConvertF32ToCell( pE->NormHealth() ) );
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_SetTargetable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetTargetable", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_SetTargetable() : Passed in NULL entity.");
		return((cell)(0));
	}

	BOOL bTargetable = !!(aParams[2]);
	pE->SetTargetable(bTargetable);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_SnapToE(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SnapToE", 2 );
	CEntity *pE1 = (CEntity *)(aParams[1]);
	if(pE1 == NULL)
	{
		SCRIPT_ERROR("E_SnapToE() : Passed in NULL entity #1.");
		return((cell)(0));
	}

	CEntity *pE2 = (CEntity *)(aParams[2]);
	if(pE2 == NULL)
	{
		SCRIPT_ERROR("CMAST::E_SnapToE() : Passed in NULL entity #2.");
		return((cell)(0));
	}

	pE1->Relocate_RotXlatFromUnitMtx_WS(pE2->MtxToWorld());

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_GetPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_GetPos", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	CFVec3A *pVec = (CFVec3A *)(aParams[2]);
	if(pVec == NULL)
	{
		return((cell)(0));
	}

	*pVec = pE->MtxToWorld()->m_vPos;

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_TouchOfDeath(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_TouchOfDeath", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);	  //the obj
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_TouchOfDeat() : Passed in null entity.");
		return((cell)(0));
	}
	pE->Die( TRUE, (BOOL) aParams[2]);			 //param 2 (or 3) whatever is true/false for spawn goodies or not

	return((cell)(0));
}

// allows bot to be damaged or not.
// first parameter is handle to entity.
// if second parameter is TRUE, entity can be damaged,
// if FALSE, entity cannot be damaged.
cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_SetDamageable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetDamageable", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);	  //the obj
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_SetDamageable() : Passed in null entity.");
		return((cell)(0));
	}

	pE->SetInvincible(!(BOOL) aParams[2]);

	return((cell)(0));
}


// tells if entity can be damaged or not.
// first parameter is handle to entity.
// returns TRUE if entity can be damaged, FALSE if entity cannot be damaged.
cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_IsDamageable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsDamageable", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);	  //the obj
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_IsDamageable() : Passed in null entity.");
		return((cell)(0));
	}

	return((cell)!pE->IsInvincible());
}


// Generates blast damage from the origin of the entity.
// The entity's current Damage Profile will be used.
//
// norm_intensity is the normalized intensity which gets
// multiplied by the intensity in the damage profile.
// 0=no damage, 1=max damage, >1=more than max damage.
//
// E_Blast entity_handle norm_intensity
cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_Blast(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_Blast", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);	  //the obj
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_Blast() : Passed in null entity.");
		return((cell)(0));
	}

	// Get an empty damage form...
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	if( pDamageForm ) {
		// Fill out the form...

		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
		pDamageForm->m_pDamageProfile = pE->GetDamageProfile();
		pDamageForm->m_Epicenter_WS = pE->MtxToWorld()->m_vPos;
		pDamageForm->m_Damager.pEntity = pE;

		if( pE->TypeBits() & ENTITY_BIT_BOT ) {
			pDamageForm->m_Damager.pBot = (CBot *)pE;
		}
		if( pE->TypeBits() & ENTITY_BIT_WEAPON ) {
			pDamageForm->m_Damager.pWeapon = (CWeapon *)pE;
		}

		pDamageForm->m_fNormIntensity = ConvertCellToF32(aParams[2]);

		CDamage::SubmitDamageForm( pDamageForm );
	}

	return((cell)(0));
}


cell CMAST_EntityWrapper::E_DistanceBetween(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_DistanceBetween", 2 );

	CEntity *pEA = (CEntity *)(aParams[1]);	
	if(pEA == NULL)
	{
		SCRIPT_ERROR("E_DistanceBetween() : first bot handle Passed in is null .");
		return((cell)(-1));
	}

	CEntity *pEB = (CEntity *)(aParams[2]);	
	if(pEB == NULL)
	{
		SCRIPT_ERROR("E_DistanceBetween() : second bot handle Passed in is null .");
		return((cell)(-1));
	}

	f32 fDist = -1.0f;
	if (pEA->IsInWorld() && pEB->IsInWorld())
	{
		fDist = pEA->MtxToWorld()->m_vPos.Dist(pEB->MtxToWorld()->m_vPos);
	}
	
	return (cell) fDist;
}


cell AMX_NATIVE_CALL CMAST_AlarmNetWrapper::E_AlarmNetFind(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_AlarmNetFind", 1 );

	char pszAlarmNetName[100];
	FillStringFromCell(pszAlarmNetName, 99, pAMX, aParams[1]);
	if(pszAlarmNetName[0] == '\0')
	{
		SCRIPT_ERROR("E_AlarmNetFind() : Passed in null alarm net name.");
		return((cell)(0));
	}

	CAlarmNet* pNet = NULL;
	pNet = CAlarmNet::FindAlarmNet(pszAlarmNetName);
	if(	!pNet)
	{
		SCRIPT_ERROR("E_AlarmNetFind() : Passed in unknown alarm net name (%s)", pszAlarmNetName);
		return((cell)(0));
	}
	return (cell) pNet;
}



cell AMX_NATIVE_CALL CMAST_AlarmNetWrapper::E_TurnOnAlarmNet(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_TurnOnAlarm", 2 );
	CAlarmNet* pNet = (CAlarmNet*) (aParams[1]);	  //the alarmNetname
	if(pNet == NULL)
	{
		SCRIPT_ERROR("E_TurnOnAlarm() : Passed in null alarm net handle.");
		return((cell)(0));
	}

	CFVec3A* pLoc = NULL;
	if (aParams[2])
	{
		pLoc = &(((CEntity*) aParams[2])->MtxToWorld()->m_vPos);
	}

	pNet->TurnOn((CEntity*) aParams[2], pLoc);
	return((cell)(0));

}


cell AMX_NATIVE_CALL CMAST_AlarmNetWrapper::E_TurnOffAlarmNet(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_TurnOffAlarmNet", 1 );
	CAlarmNet* pNet = (CAlarmNet*) (aParams[1]);	  //the alarmNetname
	if(pNet == NULL)
	{
		SCRIPT_ERROR("E_TurnOnAlarm() : Passed in null alarm net handle.");
		return((cell)(0));
	}

	pNet->TurnOff();

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_AlarmNetWrapper::E_IsAlarmNetOn(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_IsAlarmOn", 1 );
	CAlarmNet* pNet = (CAlarmNet*) (aParams[1]);	  //the alarmNetname
	if(pNet == NULL)
	{
		SCRIPT_ERROR("E_IsAlarmNetOn() : Passed in null alarm net handle.");
		return((cell)(0));
	}

	return((cell)(pNet->IsOn()));
}


cell AMX_NATIVE_CALL CMAST_SpawnSysWrapper::E_SpawnNetFind(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SpawnSysFindNet", 1 );

	char pszNetName[100];
	FillStringFromCell(pszNetName, 99, pAMX, aParams[1]);


	if(pszNetName[0] == '\0')
	{
		SCRIPT_ERROR("E_SpawnSysFindNet() : Passed in null spawn net name.");
		return((cell)(0));
	}

	CSpawnNet* pNet = NULL;
	pNet = CSpawnSys::FindSpawnNet(pszNetName);
	if(	!pNet)
	{
		SCRIPT_ERROR("E_SpawnSysFindNet() : Passed in unknown spawn net name (%s)", pszNetName);
		return (cell) 0;
	}
	return((cell)(pNet));
}


cell AMX_NATIVE_CALL CMAST_SpawnSysWrapper::E_TurnOnSpawnNet(AMX *pAMX, cell *aParams)
{

	return (cell) 0;
}


 cell AMX_NATIVE_CALL CMAST_SpawnSysWrapper::E_TurnOffSpawnNet(AMX *pAMX, cell *aParams)
{
	return (cell) 0;
}


cell AMX_NATIVE_CALL CMAST_SpawnSysWrapper::E_GetSpawnNetDefCon(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetSpawnNetDefCon", 2 );
	CSpawnNet* pNet = (CSpawnNet*) (aParams[1]);	  //the Net
	if(pNet == NULL)
	{
		SCRIPT_ERROR("E_SetSpawnNetDefCon() : Passed in null spawn net handle.");
		return((cell)(0));
	}

	return (cell) pNet->m_uDefConLevel;
}


cell AMX_NATIVE_CALL CMAST_SpawnSysWrapper::E_SetSpawnNetDefCon(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetSpawnNetDefCon", 2 );
	CSpawnNet* pNet = (CSpawnNet*) (aParams[1]);	  //the Net
	if(pNet == NULL)
	{
		SCRIPT_ERROR("E_SetSpawnNetDefCon() : Passed in null spawn net handle.");
		return((cell)(0));
	}

	u32 uDefcon = (u32) aParams[2];
	if ( uDefcon >= CSpawnNet::NUM_SPANNET_DEFCONS)
	{
		uDefcon = CSpawnNet::SPAWNNET_DEFCON_3_INTRUDER;
	}

	pNet->SetDefconLevel(uDefcon);
	return (cell) 0;
}



cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_SetExplodArmor(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetExplodArmor", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_SetExplodArmor() : Passed in null entity.");
		return((cell)(0));
	}

	f32 fUnitArmor = (f32)(aParams[2]) * (0.01f);
	// SER: Fix this
//	pE->SetUnitArmor(CEntity::ARMOR_TYPE_EXPLOSION, fUnitArmor);
	
	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_SetFlameArmor(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetFlameArmor", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_SetFlameArmor() : Passed in null entity.");
		return((cell)(0));
	}

	f32 fUnitArmor = (f32)(aParams[2]) * (0.01f);
	// SER: Fix this
//	pE->SetUnitArmor(CEntity::ARMOR_TYPE_FLAME, fUnitArmor);

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_EntityWrapper::E_SetBulletArmor(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "E_SetBulletArmor", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("E_SetBulletArmor() : Passed in null entity.");
		return((cell)(0));
	}

	f32 fUnitArmor = (f32)(aParams[2]) * (0.01f);
	// SER: Fix this
//	pE->SetUnitArmor(CEntity::ARMOR_TYPE_BULLET, fUnitArmor);

	return((cell)(0));
}

//
/////////////////////////////////////////////////////////////////////////////////////////////

// =============================================================================================================

//// CMeshEntity functions.
//

CMeshEntity **CMAST_MeshEntityWrapper::m_papPool = NULL;
u32 CMAST_MeshEntityWrapper::m_uNumAcquired = 0;

BOOL CMAST_MeshEntityWrapper::InitSystem()
{
	m_papPool = (CMeshEntity **)(fres_AllocAndZero(sizeof(CMeshEntity *) * CMAST_CMeshEntityWrapper_uMaxPoolSize));
	if(m_papPool == NULL)
	{
		SCRIPT_ERROR("CFST_MeshEntityWrapper::InitSystem() : Could not allocate pool.");
		m_uNumAcquired = 0;
		return(TRUE);
	}

	return(TRUE);
}

void CMAST_MeshEntityWrapper::UninitSystem()
{
	m_papPool = NULL;
}

BOOL CMAST_MeshEntityWrapper::InitLevel()
{
	FASSERT(m_uNumAcquired == 0);

	return(TRUE);
}

void CMAST_MeshEntityWrapper::UninitLevel()
{
	u32 uCurMEIdx;
	for(uCurMEIdx = 0; uCurMEIdx < m_uNumAcquired; ++uCurMEIdx)
	{
		fdelete(m_papPool[uCurMEIdx]);
		m_papPool[uCurMEIdx] = NULL;
	}
	m_uNumAcquired = 0;
}

cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_Acquire", 0 );
	CMeshEntity *pME = fnew CMeshEntity;
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_Acquire : Could not create mesh entity.");
		return(NULL);
	}

	m_papPool[m_uNumAcquired] = pME;
	++m_uNumAcquired;

	return((cell)(pME));
}

cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_Release(AMX *pAMX, cell *aParams)
{
	// REPAIR: This should be getting returned to the pool.
	return(0);
}

// looks like this call is not exposed to .inc files and is not used -cjm
cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_Find(AMX *pAMX, cell *aParams)
{
	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[1]);
	return((cell)(CEntity::Find(szEntityName)));
}

// looks like this call is not exposed to .inc files and is not used -cjm
cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_IsValid(AMX *pAMX, cell *aParams)
{
	// This simply checks to see if the CMeshEntity pointer is NULL.
	return((CMeshEntity *)(aParams[1]) != NULL);
}

// looks like this call is not exposed to .inc files and is not used -cjm
cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_AddToWorld(AMX *pAMX, cell *aParams)
{
	return CMAST_EntityWrapper::E_AddToWorld(pAMX, aParams);
}

// looks like this call is not exposed to .inc files and is not used -cjm
cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_RemoveFromWorld(AMX *pAMX, cell *aParams)
{
	return CMAST_EntityWrapper::E_RemoveFromWorld(pAMX, aParams);
}

cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_Init1(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_Init1", 2 );
	// MeshEntities init by mesh name, so we need to get the name from the mesh
	//   that the script passed in.
	FMesh_t *pMesh = (FMesh_t *)(aParams[2]);
	CFMtx43A mtxTemp;
	mtxTemp.Identity();

	if( pMesh == NULL )
	{
		SCRIPT_ERROR("ME_Init1() : Second parameter was null." );
	}


	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if( pME == NULL )
	{
		SCRIPT_ERROR("ME_Init1() : First parameter was null." );
	}

	if(pME && pMesh && !pME->Create(pMesh->szName, NULL, NULL, &mtxTemp))
	{
		SCRIPT_ERROR("ME_Init1() : Error trying to create MeshEntity.");
	}

	return(0);
}


// This init takes a mesh and an animation name as parameters.
cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_Init2(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_Init2", 3 );
	// MeshEntities init by mesh name, so we need to get the name from the mesh
	//   that the script passed in.
	FMesh_t *pMesh = (FMesh_t *)(aParams[2]);
	CFMtx43A mtxTemp;
	mtxTemp.Identity();

	if( pMesh == NULL )
	{
		SCRIPT_ERROR("ME_Init2() : Second parameter was null." );
	}

	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if( pME == NULL )
	{
		SCRIPT_ERROR("ME_Init2() : First parameter was null." );
	}

	if (pME && pMesh && aParams[3])
	{
		char szAnimName[20];
		FillStringFromCell(szAnimName, 19, pAMX, aParams[3]);

		if(!pME->Create(pMesh->szName, szAnimName, NULL, &mtxTemp))
		{
			SCRIPT_ERROR("ME_Init2() : Error trying to create MeshEntity.");
		}

	}
	return(0);
}




cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_AnimSelect(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_AnimSelect", 2 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_AnimSelect() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_AnimSelect() : Mesh entity not yet created.");
		return((cell)(0));
	}

	s32 nAnimIdx = (s32)(aParams[2]);
	pME->UserAnim_Select(nAnimIdx);
	pME->UserAnim_UpdateTime(0.0f);

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_SetAnimSpeed(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_SetAnimSpeed", 2 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_SetAnimSpeed() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_SetAnimSpeed() : Mesh entity not yet created.");
		return((cell)(0));
	}

	f32 fAnimSpeed = ConvertCellToF32(aParams[2]);
	pME->UserAnim_SetSpeedMult(fAnimSpeed);

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_GetAnimSpeed(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_GetAnimSpeed", 1 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_GetAnimSpeed() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_GetAnimSpeed() : Mesh entity not yet created.");
		return((cell)(0));
	}

	f32 fSpeed = 0.0f;
	fSpeed = pME->UserAnim_GetAnimSpeedMult();

	return(ConvertF32ToCell(fSpeed));
}


cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_SetAnimClamp(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_SetAnimClamp", 2 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_SetAnimClamp() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_SetAnimClamp() : Mesh entity not yet created.");
		return((cell)(0));
	}

	BOOL bAnimClamp = !!(aParams[2]);
	pME->UserAnim_SetClampMode(bAnimClamp);

	return((cell)(0));
}


// ME_EnableAnim( mesh_entity, TRUE/FALSE )
cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_EnableAnim(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_EnableAnim", 2 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_EnableAnim() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_EnableAnim() : Mesh entity not yet created.");
		return((cell)(0));
	}

	BOOL bAnimClamp = !!(aParams[2]);
	pME->DriveMeshWithAnim(bAnimClamp);

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_GetUnitAnimPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_GetUnitAnimPos", 1 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_GetUnitAnimPos() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_GetUnitAnimPos() : Mesh entity not yet created.");
		return((cell)(0));
	}

	CFAnimInst *pAI = NULL;

	pAI = pME->UserAnim_GetCurrentInst();

	if(pAI == NULL)
	{
		SCRIPT_ERROR("ME_GetUnitAnimPos() : Mesh entity has no anim inst." );
		return((cell)(0));
	}

	return(ConvertF32ToCell(pAI->GetUnitTime()));
}



cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_GetAnimPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_GetAnimPos", 1 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_GetAnimPos() : Null ME passed in.");
		return((cell)(0));
	}

	CFAnimInst *pAI = NULL;
	
	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_GetAnimPos() : Mesh entity not yet created.");
		return((cell)(0));
	}

	pAI = pME->UserAnim_GetCurrentInst();
	if(pAI == NULL)
	{
		SCRIPT_ERROR("ME_GetAnimPos() : Mesh entity has no anim inst." );
		return((cell)(0));
	}

	return(ConvertF32ToCell(pAI->GetTime()));
}



cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_SetUnitAnimPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_SetUnitAnimPos", 2 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_SetUnitAnimPos() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_SetUnitAnimPos() : Mesh entity not yet created.");
		return((cell)(0));
	}

	CFAnimInst *pAI = pME->UserAnim_GetCurrentInst();
	if(pAI == NULL)
	{
		SCRIPT_ERROR("ME_SetUnitAnimPos() : Mesh entity has no anim inst." );
		return((cell)(0));
	}

	pAI->UpdateUnitTime(ConvertCellToF32(aParams[2]));

	return(cell(0));
}



cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_SetAnimPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_SetAnimPos", 2 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_SetAnimPos() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_SetAnimPos() : Mesh entity not yet created.");
		return((cell)(0));
	}

	CFAnimInst *pAI = pME->UserAnim_GetCurrentInst();
	if(pAI == NULL)
	{
		return((cell)(0));
	}

	pAI->UpdateTime(ConvertCellToF32(aParams[2]));

	return(cell(0));
}

// allows a mesh entity to remain "awake" (call it's work function) even when not in an active volume.
// first parameter is pointer to mesh entity.
// if second parameter is TRUE, mesh entity will remain awake, 
// if FALSE, mesh entity will stop when outside active volume (default behavior).
cell AMX_NATIVE_CALL CMAST_MeshEntityWrapper::ME_SetAlwaysWork(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "ME_SetAnimPos", 2 );
	CMeshEntity *pME = (CMeshEntity *)(aParams[1]);
	BOOL bAlways = (BOOL) !!(aParams[2]);

	if(pME == NULL)
	{
		SCRIPT_ERROR("ME_SetAlwaysWork() : Null ME passed in.");
		return((cell)(0));
	}

	if (!pME->IsCreated())
	{
		SCRIPT_ERROR("ME_SetAlwaysWork() : Mesh entity not yet created.");
		return((cell)(0));
	}

	pME->AlwaysWork( bAlways );

	return(cell(0));
}


// =============================================================================================================

CFAnimInst *CMAST_BotWrapper::m_pAnimInst = NULL;
BOOL CMAST_BotWrapper::m_bIsAnimating = FALSE;
CBot *CMAST_BotWrapper::m_pbotAnimating = NULL;
CEntityControl *CMAST_BotWrapper::m_pBotControls;

BOOL CMAST_BotWrapper::InitSystem()
{
	m_bIsAnimating = FALSE;

	return(TRUE);
}

void CMAST_BotWrapper::UninitSystem()
{
	m_bIsAnimating = FALSE;
}

void CMAST_BotWrapper::Work()
{
   //stuck this here since it is the only place I could find that was script related and got called every frame and wasn't in fang.
#if !FANG_PRODUCTION_BUILD
	u8 uControllerId = Gamepad_nDebugPortIndex;
	if ((CPlayer::m_nPlayerCount == 1) && (Gamepad_aapSample[uControllerId][GAMEPAD_MAIN_FIRE_SECONDARY]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK))
	{
		CFScriptSystem::SetMonitorsOn(1-CFScriptSystem::GetMonitorsOn());
	}
#endif
	if(m_bIsAnimating)
	{
		FASSERT(m_pAnimInst != NULL);
		m_pAnimInst->DeltaTime(FLoop_fPreviousLoopSecs, TRUE);

		f32 fAnimUnitTime = m_pAnimInst->GetUnitTime();

		if(fAnimUnitTime >= 0.999f)
		{
			m_bIsAnimating = FALSE;
			m_pbotAnimating->UserAnim_Attach(0, NULL);
			m_pbotAnimating->UserAnim_SetControlValue(0, 0.0f);
			m_pbotAnimating->UserAnim_DisableAllBones(0);
			m_pbotAnimating = NULL;
		}
		else if(fAnimUnitTime > 0.9f)
		{
			m_pbotAnimating->UserAnim_SetControlValue(0, 10.0f * (1.0f - fAnimUnitTime));
		}
		else if(fAnimUnitTime < 0.1f)
		{
			m_pbotAnimating->UserAnim_SetControlValue(0, 10.0f * fAnimUnitTime);
		}
	}
}

BOOL CMAST_BotWrapper::InitLevel()
{
	m_bIsAnimating = FALSE;

	return(TRUE);
}

void CMAST_BotWrapper::UninitLevel()
{
	fdelete(m_pAnimInst);
	m_pAnimInst = NULL;
	m_bIsAnimating = FALSE;
}

/////////////////////////////////////////////////////////////
// General CBot functions.
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_Find(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_Find", 1 );
	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[1]);

	CEntity *pEntity = CEntity::Find(szEntityName);

	if(pEntity == NULL)
	{
		SCRIPT_ERROR("Bot_Find() : Could not find object named '%s'.", szEntityName);
		return((cell)(NULL));
	}

	if(!(pEntity->TypeBits() & ENTITY_BIT_BOT))
	{
		SCRIPT_ERROR("Bot_Find() : Object named '%s' is not a bot.", szEntityName);
		return((cell)(NULL));
	}

	return((cell)(pEntity));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsValid(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsValid", 1 );
	return((cell)(aParams[1] != NULL));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_AddToWorld(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_AddToWorld", 1 );
	return CMAST_EntityWrapper::E_AddToWorld(pAMX, aParams);
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_RemoveFromWorld(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_RemoveFromWorld", 1 );
	return CMAST_EntityWrapper::E_RemoveFromWorld(pAMX, aParams);
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsDeadOrDying(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsDeadOrDying", 1 );
	CBot *pBot = (CBot *)(aParams[1]);
	

	return((cell) pBot->IsDeadOrDying());
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsDead(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsDead", 1 );
	CBot *pBot = (CBot *)(aParams[1]);

	return((cell) pBot->IsDead());
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_GetPlayer(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_GetPlayer", 0 );
	CEntity *pE = CPlayer::m_pCurrent->m_pEntityCurrent;
	FASSERT(pE->TypeBits() & ENTITY_BIT_BOT);
	CBot *pBot = (CBot *)(pE);
	return((cell)(pE));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_JobWait(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_JobWait", 2 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_JobWait() : Passed in null bot.");
		return((cell)(0));
	}

	if (pBot->IsInWorld())
	{
		ai_AssignJob_Wait(pBot->AIBrain(), FALSE, CFVec3A::m_Null, 0, (u8)(aParams[2]));
	}
	else
	{
		SCRIPT_ERROR("Bot_JobWait() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_JobWaitAndLook(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_JobWaitAndLook", 3 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_JobWait() : Passed in null bot.");
		return((cell)(0));
	}

	if (pBot->IsInWorld())
	{
		CEntity* pLookAtEntity = (CEntity*) aParams[2];
		u32 uLookAtEntityGUID = 0;
		if (pLookAtEntity)
		{
			uLookAtEntityGUID = pLookAtEntity->Guid();
		}
		ai_AssignJob_Wait(	pBot->AIBrain(),
							TRUE,
							CFVec3A::m_Null,
							uLookAtEntityGUID,
							(u8)(aParams[3]));
	}
	else
	{
		SCRIPT_ERROR("Bot_JobWaitAndLook() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_JobWander(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_JobWander", 8 );
	CBot *pBot = (CBot *)(aParams[1]);

	if (pBot->IsInWorld())
	{
		ai_AssignJob_Wander(pBot->AIBrain(), (f32)(aParams[2]), aParams[3], (u8) (aParams[4]), (u8)(aParams[5]), (u8)(aParams[6]), (u8)(aParams[7]), (u8)(aParams[8]));
	}
	else
	{
		SCRIPT_ERROR("Bot_JobWander() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_JobPatrol(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_JobPatrol", 4 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_JobPatrol() : Passed in null bot.");
		return((cell)(0));
	}

	char szPathName[100];
	FillStringFromCell(szPathName, 99, pAMX, aParams[2]);

	if (pBot->IsInWorld())
	{
		ai_AssignJob_Patrol(pBot->AIBrain(), szPathName, (u8)(aParams[3]), (u8)(aParams[4]), 0, NULL);
	}
	else
	{
		SCRIPT_ERROR("Bot_JobPatrol() : Bot named %s not in world", pBot->Name() );
	}
	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_GoalGotoPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_GoalGotoPos", 5 );
	CBot *pBot = (CBot *)(aParams[1]);

	u8 uDoReactions = FALSE;
	u8 uGotoFlags = GOTOFLAG_NONE;

	if (aParams[4] > 0)
	{
		uGotoFlags |= GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END;
	}

	if (aParams[0]/4 > 4)
	{
		if (aParams[5] == 1)
		{
			uGotoFlags |= GOTOFLAG_USE_JOB_REACT_RULES;
		}
		else if (aParams[5] == 2)
		{
			uGotoFlags |= GOTOFLAG_RETURN_FIRE_OK;
		}
	}

	if (pBot->IsInWorld())
	{													 
		ai_AssignGoal_Goto(pBot->AIBrain(),				  
							*((CFVec3A *)(aParams[2])),	  // Where                                                                                       
							2,	   //fudge
							(u8)(aParams[3]),			  // (0-100) Pct speed this unit should travel at (0 means goto should never change units speed) 
							uGotoFlags);		
	}
	else
	{
		SCRIPT_ERROR("Bot_GoalGotoPos() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}

//native Bot_GotoE(	Bot:this,			1	//Bot to move.
//					Entity:hEDest,		2	//go to the center of this entity
//					BOOL bEventWhenDone,3	//(DONT_TRIGGER,DO_TRIGGER) engine should trigger the ai script event to be called when bot is within a couple feet of goal
//					nSpeedPercent,		4	//percent of speed bot is cabable of going (0 means use bot's base speed as specified in user props or Bot_SetbaseSpeed(..)
//					Entity:hELookAt,	5	//Entity to look at while on goto
//					nWhenToStartLook,	6	//(0-100) percent of the way to goal that the bot will begin looking at hELookAt
//					BOOL nReactions);	7	//(DISABLE_AI_REACTIONS,ENABLE_AI_REACTIONS,ENABLE_ATTACK_ON_THE_WAY)

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_GotoE(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_GotoE", 7 );
	CBot *pBot = (CBot *)(aParams[1]);
	CEntity *pEDest = (CEntity *)(aParams[2]);
	CEntity *pELookAt = (CEntity *)(aParams[5]);

	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_GotoE() : Passed in null bot.");
		return((cell)(0));
	}
	if(pEDest == NULL)
	{
		SCRIPT_ERROR("Bot_GotoE() : Passed in null destination entity.");
		return((cell)(0));
	}

	if(aParams[3] < 0 || aParams[3] > 1 )
	{
		SCRIPT_ERROR("Bot_GotoE() : bEventWhenDone should be DONT_TRIGGER or DO_TRIGGER." );
		aParams[3] = 0;
	}


	u8 uDoReactions = FALSE;
	if (aParams[0]/4 > 6)
	{
		uDoReactions = (u8) aParams[7];
	}

	u16 uGotoFlags = GOTOFLAG_NONE;
	if (uDoReactions == 1)
	{
		uGotoFlags |= GOTOFLAG_USE_JOB_REACT_RULES;
	}
	else if (uDoReactions == 2)
	{
		uGotoFlags |= GOTOFLAG_RETURN_FIRE_OK;
	}
	if (aParams[3])
	{
		uGotoFlags |= GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END;
	}

	if (pBot->IsInWorld())
	{
		if(pELookAt)
		{
			ai_AssignGoal_GotoWithLookAt(pBot->AIBrain(),					//	CAIBrain* pBrain,
										pEDest->MtxToWorld()->m_vPos,		//	const CFVec3A& GotoLoc,
										CFVec3A::m_Null,					//	const CFVec3A& LookAtLoc
										pELookAt->Guid(),					//	u32 uLookAtObjGUID,
										(u8)(aParams[6]),					//	u8 uPctToLookat
										2,									//  u8 uFudgeDest
										(u8)(aParams[4]),					//	u8 uSpeedPct
										uGotoFlags);						//  u16 uGotoFlags
		}
		else
		{
			ai_AssignGoal_Goto(	pBot->AIBrain(),			   //	CAIBrain* pBrain,          
								pEDest->MtxToWorld()->m_vPos,  //	const CFVec3A& GotoLoc,
								2,							   //   u8 uFudgeDest
								(u8)(aParams[4]),			   //	u8 uSpeedPct,
								uGotoFlags);				   //   u16 uGotoFlags
		}
	}
	else
	{
		SCRIPT_ERROR("Bot_GotoE() : Bot named %s not in world", pBot->Name() );
	}
	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_GotoEO(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_GotoEO", 6 );
	CBot *pBot = (CBot *)(aParams[1]);
	CEntity *pEDest = (CEntity *)(aParams[2]);

	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_GotoEO() : Passed in null bot.");
		return((cell)(0));
	}
	if(pEDest == NULL)
	{
		SCRIPT_ERROR("Bot_GotoEO() : Passed in null destination entity.");
		return((cell)(0));
	}

	u16 uGotoFlags = GOTOFLAG_NONE;
	if (aParams[6] == 1)
	{
		uGotoFlags |= GOTOFLAG_USE_JOB_REACT_RULES;
	}
	else if (aParams[6] == 2)
	{
		uGotoFlags |= GOTOFLAG_RETURN_FIRE_OK;
	}
	if (aParams[3])
	{
		uGotoFlags |= GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END;
	}
	
	uGotoFlags |= GOTOFLAG_LOOKAT;


	CFVec3A vecLookAtPt = pEDest->MtxToWorld()->m_vPos;
	vecLookAtPt.Add(pEDest->MtxToWorld()->m_vZ);
	if (pBot->IsInWorld())
	{
		ai_AssignGoal_GotoWithLookAt(	pBot->AIBrain(),				//	CAIBrain* pBrain,
										pEDest->MtxToWorld()->m_vPos,	//	const CFVec3A& GotoLoc,
										vecLookAtPt,					//	const CFVec3A& LookAtLoc
										0,								//	u32 uLookAtObjGUID,
										(u8)(aParams[5]),				//	u8 uPctToLookat
										2,								//  u8 uFudgeDest
										(u8)(aParams[4]),				//	u8 uSpeedPct
										uGotoFlags);				    //  u16 uGotoFlags												
	}
	else
	{
		SCRIPT_ERROR("Bot_GotoEO() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_Attack(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_Attack", 3 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_Attack() : Passed in null bot.");
		return((cell)(0));
	}

	CEntity *pAttackee = (CEntity *)(aParams[2]);
	if(pAttackee == NULL)
	{
		SCRIPT_ERROR("Bot_Attack() : Passed in null bot.");
		return((cell)(0));
	}

	if (pBot->IsInWorld())
	{
		ai_AssignGoal_Attack(pBot->AIBrain(), pAttackee->Guid(), (u8)(aParams[3]));
	}
	else
	{
		SCRIPT_ERROR("Bot_Attack() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_FacePt(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_FacePt", 5 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_FacePt() : Passed in null bot.");
		return((cell)(0));
	}

	CFVec3A *pvecPos = (CFVec3A *)(aParams[2]);
	if(pvecPos == NULL)
	{
		SCRIPT_ERROR("Bot_FacePt() : Passed in null face pos.");
		return((cell)(0));
	}

	if (pBot->IsInWorld())
	{
		ai_AssignGoal_FaceIt(pBot->AIBrain(), *pvecPos, 0, (u8)(aParams[3]), (u8)(aParams[4]), (s16)(aParams[5]));
	}
	else
	{
		SCRIPT_ERROR("Bot_FacePt() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_FaceE(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_FaceE", 5 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_FaceE() : Passed in null bot.");
		return((cell)(0));
	}

	CEntity *pELookAt = (CEntity *)(aParams[2]);
	if(pELookAt == NULL)
	{
		SCRIPT_ERROR("Bot_FaceE() : Passed in null entity to look at.");
		return((cell)(0));
	}

	if (pBot->IsInWorld())
	{
		ai_AssignGoal_FaceIt(pBot->AIBrain(), CFVec3A::m_Null, pELookAt->Guid(), (u8)(aParams[3]), (u8)(aParams[4]), (s16)(aParams[5]));
	}
	else
	{
		SCRIPT_ERROR("Bot_FaceE() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_LoadTalk(AMX *pAMX, cell *aParams)
{
	// Retail scripts pass three more values, which the retail native hands to a talk-pool
	// setting this source lacks after BTIPool_Init. They are accepted and ignored.
	if( (aParams[0] >> 2) != 5 ) {
		SCRIPT_CHECK_NUM_PARAMS( "Bot_LoadTalk", 2 );
	}

	char szBTA[100];
	cchar* pszStaticString= NULL;
	if (aParams[1])
	{
		FillStringFromCell(szBTA, 99, pAMX, aParams[1]);
		pszStaticString = gstring_Main.AddString( szBTA );
	}


	CTalkSystem2::BTIPool_Init(pszStaticString, aParams[2]);
	return 0;
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_Talk(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_Talk", 4 );

	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL || !pBot->AIBrain())
	{
		SCRIPT_ERROR("Bot_Talk() : Passed in null bot.");
		return((cell)(0));
	}

	char szBTA[100];
	cchar* pszStaticString = NULL;
	if (aParams[2])
	{
		FillStringFromCell(szBTA, 99, pAMX, aParams[2]);
		pszStaticString = gstring_Main.FindString(szBTA);
	}

	CEntity *pELookAt = (CBot *)(aParams[3]);	

	BOOL bStopFirst = aParams[4]!=0;

	if (!pBot->AIBrain()->IsTalking())
	{
		ai_TalkModeBegin(pBot->AIBrain(),
						AIBrain_TalkModeCB,
						(void*) pszStaticString,
						pBot,
						1,//kuAttackMOdeTalkRequestTimeOutSecs,
						bStopFirst, //STOP
						pELookAt!=NULL, //DON"T LOOK
						pELookAt ? pELookAt->Guid() : 0);
	}
	return (cell)0;
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_FacePlayer(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_FacePlayer", 4 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_FacePlayer() : Passed in null bot.");
		return((cell)(0));
	}

	u32 uPlayerId = (u32) (aParams[2]);

	if (pBot->IsInWorld() && Player_aPlayer[uPlayerId].m_pEntityCurrent)
	{
		ai_AssignGoal_FaceIt(	pBot->AIBrain(),
								CFVec3A::m_Null,
								Player_aPlayer[uPlayerId].m_pEntityCurrent->Guid(),
								TRUE,
								(u8) (aParams[3]),
								(BOOL) (aParams[4]));
	}
	else
	{
		SCRIPT_ERROR("Bot_FacePlayer() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_FollowLeader(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_FollowLeader", 3 );
	CBot *pBot = (CBot *)(aParams[1]);	
	CEntity *pLeader = (CBot *)(aParams[2]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_FollowLeader() : Passed in null bot.");
		return((cell)(0));
	}

	if (pBot->IsInWorld())
	{
		ai_FollowLeader(pBot->AIBrain(), pLeader ? pLeader->Guid():0, (s16) (aParams[3]));
		pBot->AIBrain()->SetFlag_Buddy_Ctrl_OnAction();
	}
	else
	{
		SCRIPT_ERROR("Bot_FollowLeader() : Bot named %s not in world", pBot->Name() );
	}

	return ((cell)(0));
}


cell AMX_NATIVE_CALL Bot_SetBuddyCtrl(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_SetBuddyCtrl", 2 );
	CBot *pBot = (CBot *)(aParams[1]);	
	if(pBot == NULL)
	{
		return((cell)(0));
	}

	if (pBot && pBot->AIBrain())  //bot need not be in the world for this
	{
		switch (aParams[2])
		{
			case AI_BUDDY_CTRL_NEVER:
			case AI_BUDDY_CTRL_AUTO:
			case AI_BUDDY_CTRL_ONACTION:
				ai_ChangeBuddyCtrl(pBot->AIBrain(), (u8)aParams[2]);
				break;
			default:
				SCRIPT_ERROR("Bot_SetBuddyCtrl() : No comprende your ctrl param (0 = never, 1 = auto, 2 = onaction");
				break;
		}
	}

	return ((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_SetBaseSpeed(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_SetBaseSpeed", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_SetBaseSpeed() : Passed in null bot.");
		return((cell)(0));
	}

	if (pBot->IsInWorld())
	{
		ai_SetBaseSpeed(pBot->AIBrain(), (u8)(aParams[2]));
	}
	else
	{
		SCRIPT_ERROR("Bot_SetBaseSpeed() : Bot named %s not in world", pBot->Name() );
	}


	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_TurnOnPercep(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_TurnOnPercep", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_TurnOnPercep() : Passed in null bot.");
		return((cell)(0));
	}

	if(pBot->IsInWorld())
	{
		ai_TurnOnPerceptor(pBot->AIBrain(), (u8)(aParams[2]));
	}
	else
	{
		SCRIPT_ERROR("Bot_TurnOnPercep() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_TurnOffPercep(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_TurnOffPercep", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_TurnOffPercep() : Passed in null bot.");
		return((cell)(0));
	}

	if(pBot->IsInWorld())
	{
		ai_TurnOffPerceptor(pBot->AIBrain(), (u8)(aParams[2]));
	}
	else
	{
		SCRIPT_ERROR("Bot_TurnOffPercep() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_UsePercep(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_UsePercep", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_UsePercep() : Passed in null bot.");
		return((cell)(0));
	}

	if(pBot->IsInWorld())
	{
		ai_UsePerceptor(pBot->AIBrain(), (u8)(aParams[2]));
	}
	else
	{
		SCRIPT_ERROR("Bot_UsePercep() : Bot named %s not in world", pBot->Name() );
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IgnorePercep(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IgnorePercep", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_IgnorePercep() : Passed in null bot.");
		return((cell)(0));
	}

	if(pBot->IsInWorld())
	{
		ai_IgnorePerceptor(pBot->AIBrain(), (u8)(aParams[2]));
	}
	else
	{
		SCRIPT_ERROR("Bot_IgnorePercep() : Bot not in world.'%s'", pBot->Name() ? pBot->Name() : "NONAME");
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsPercepOn(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsPercepOn",2  );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_IsPercepOn() : Passed in null bot.");
		return((cell)(0));
	}

	if(pBot->IsInWorld())
	{
		return((cell)(ai_IsPerceptorOn(pBot->AIBrain(), (u8)(aParams[2]))));
	}
	else
	{
		SCRIPT_ERROR("Bot_IsPercepOn() : Bot not in world.", pBot->Name() ? pBot->Name() : "NONAME");
	}

	return((cell)(0));
}



cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsPercepInUse(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsPercepInUse", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_IsPercepInUse() : Passed in null bot.");
		return((cell)(0));
	}

	if(pBot->IsInWorld())
	{
		return((cell)(ai_IsPerceptorInUse(pBot->AIBrain(), (u8)(aParams[2]))));
	}
	else
	{
		SCRIPT_ERROR("Bot_IsPercepInUse() : Bot not in world.", pBot->Name() ? pBot->Name() : "NONAME");
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Ai_EnableUnarmedPlayerRule(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Ai_EnableUnarmedPlayerRule", 0 );
	ai_EnableUnarmedPlayerRule();
	return (cell) 0;
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Ai_DisableUnarmedPlayerRule(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Ai_DisableUnarmedPlayerRule", 0 );
	ai_DisableUnarmedPlayerRule();
	return (cell) 0;
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Ai_IsUnarmedPlayerRuleEnabled(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Ai_IsUnarmedPlayerRuleEnabled", 0 );
	return (cell) ai_IsUnarmedPlayerRuleEnabled();
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Ai_CountEnemiesNear(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Ai_CountEnemiesNear", 3 );

	CEntity *pB = (CEntity *)(aParams[1]);	
	if((pB == NULL) || !pB->AIBrain())
	{
		SCRIPT_ERROR("Ai_CountEnemiesNear() : bot handle Passed in is not valid.");
		return((cell)(-1));
	}

	CEntity *pE = (CEntity *)(aParams[2]);	
	if((pE == NULL))
	{
		SCRIPT_ERROR("Ai_CountEnemiesNear() : Near Entity handle Passed in is not valid.");
		return((cell)(-1));
	}

	return (cell) aigroup_CountEnemiesNearPt(pB, pE->MtxToWorld()->m_vPos, (f32) aParams[3]);
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Ai_CountFriendsNear(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Ai_CountFriendsNear", 3 );

	CEntity *pB = (CEntity *)(aParams[1]);	
	if((pB == NULL) || !pB->AIBrain())
	{
		SCRIPT_ERROR("Ai_CountFriendsNear() : bot handle Passed in is not valid.");
		return((cell)(-1));
	}

	CEntity *pE = (CEntity *)(aParams[2]);	
	if((pE == NULL))
	{
		SCRIPT_ERROR("Ai_CountFriendsNear() : Near Entity handle Passed in is not valid.");
		return((cell)(-1));
	}

	return (cell) aigroup_CountAlliesNearPt(pB, pE->MtxToWorld()->m_vPos, (f32) aParams[3]);
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_CanSeeBot(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_CanSeeBot", 2 );

	CBot *pBotA = (CBot *)(aParams[1]);	
	if(pBotA == NULL)
	{
		SCRIPT_ERROR("Bot_CanSeeBot() : first bot handle Passed in is null .");
		return((cell)(0));
	}

	CEntity *pBotB = (CBot *)(aParams[2]);	
	if(pBotB == NULL)
	{
		SCRIPT_ERROR("Bot_CanSeeBot() : second bot handle Passed in is null .");
		return((cell)(0));
	}

	if (!(pBotA->TypeBits() & ENTITY_BIT_BOT) ||
		!pBotA->AIBrain() ||
		!(pBotB->TypeBits() & ENTITY_BIT_BOT) ||
		!pBotA->AIBrain())
	{
		SCRIPT_ERROR("Bot_CanSeeBot() : one of the bots is not a bot with a brain .");
		return((cell)(0));
	}

	BOOL bCanSee = FALSE;
	if (pBotA->IsInWorld() && 
		pBotB->IsInWorld())
	{
		if (pBotA->AIBrain()->GetKnowledge().CanRememberEntity(MEMORY_LARGE_SEEN, pBotB))
		{
			bCanSee = TRUE;
		}
	}
	
	return (cell) bCanSee;
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_ClrAttackSpecs(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_ClrAttackSpecs", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL || pBot->AIBrain() == NULL)
	{
		SCRIPT_ERROR("Bot_ClrAttackSpecs() : Passed in null bot, or brainless bot.");
		return((cell)(0));
	}

	ai_ClearAttackSpecs(pBot->AIBrain(), (BOOL) aParams[2]);

	return (cell) 0;
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_SetAttackSpecs( AMX *pAMX, cell *aParams)
{
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL ||  pBot->AIBrain()==NULL)
	{
		SCRIPT_ERROR("Bot_SetAttackSpecs() : Passed in null bot, or brainless bot.");
		return((cell)(0));
	}

	ai_SetAttackSpecs(	pBot->AIBrain(),
						(u8) aParams[2],		// which ruleset the brain will use when attacking
						(u8) aParams[3],		// an alternate one
						(u8) aParams[4],		// (0-100) pct chance that the alternate will be used
						(u8) aParams[5],		// Limit the distance traveled from location at the time attack mode turns on
						(u8) aParams[6],		// stay local until enemy is atleast this close
						(u8) aParams[7],		// (0-100) how fast to move while in attack mode (min)
						(u8) aParams[8],		// (0-100) how fast to move while in attack mode (max)
						(u8) aParams[9]);	// if brain is currently on attack, do you want to reset the attack to use default attack specs?
	return (cell) 0;
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_LoadAnim(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_LoadAnim", 1 );
	char szAnimName[100];
	FillStringFromCell(szAnimName, 99, pAMX, aParams[1]);

	SCRIPT_ERROR( "Scripts should not be using Bot_LoadAnim or Bot_PlayAnim!" );
	FAnim_t *pAnim = (FAnim_t *)(fresload_Load(FANIM_RESNAME, szAnimName));
	if(pAnim == NULL)
	{
		SCRIPT_ERROR("Bot_LoadAnim() : Could not load anim file '%s'.", szAnimName);
		return((cell)(0));
	}
	m_pAnimInst = fnew CFAnimInst;
	if(m_pAnimInst == NULL)
	{
		SCRIPT_ERROR("Bot_LoadAnim() : Could not create CFAnimInst for '%s'.", szAnimName);
		return((cell)(0));
	}
	if(!m_pAnimInst->Create(pAnim))
	{
		SCRIPT_ERROR("Bot_LoadAnim() : Error Create()ing CFAnimInst from FAnim_t for '%s'.", szAnimName);
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_PlayAnim(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_PlayAnim", 1 );
	SCRIPT_ERROR( "Scripts should not be using Bot_LoadAnim or Bot_PlayAnim!" );
	if(m_pAnimInst == NULL)
	{
		SCRIPT_ERROR("Bot_PlayAnim() : Cannot play animation when none has been loaded.");
		return((cell)(0));
	}
	m_bIsAnimating = TRUE;
	m_pbotAnimating = (CBot *)(aParams[1]);
	if(m_pbotAnimating->UserAnim_GetCount() == 0)
	{
		SCRIPT_ERROR("Bot_PlayAnim() : Huh?  No room left to load an animation.");
		return((cell)(0));
	}
	m_pbotAnimating->UserAnim_Attach(0, m_pAnimInst);
	m_pbotAnimating->UserAnim_SetControlValue(0, 1.0f);
	m_pbotAnimating->UserAnim_EnableAllBones(0);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_GetAnimPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_GetAnimPos", 0 );
	return(ConvertF32ToCell(m_pAnimInst->GetUnitTime()));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_FreezePlayer(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_FreezePlayer", 0 );
	CBot *pPlayer = (CBot *)(CPlayer::m_pCurrent->m_pEntityCurrent);

	m_pBotControls = pPlayer->Controls();
	pPlayer->SetControls(NULL);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_UnfreezePlayer(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_UnfreezePlayer", 0 );
	CBot *pPlayer = (CBot *)(CPlayer::m_pCurrent->m_pEntityCurrent);
	
	FASSERT(m_pBotControls != NULL);
	pPlayer->SetControls(m_pBotControls);
	m_pBotControls = NULL;

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_AttachAnim(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_AttachAnim", 3 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_AttachAnim() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_AttachAnim() : not a bot." );
		return((cell)(0));
	}

	CFAnimInst *pAnimInst = (CFAnimInst *)(aParams[2]);
	if(pAnimInst == NULL)
	{
		SCRIPT_ERROR("Bot_AttachAnim() : Passed in null AnimInst.");
		return((cell)(0));
	}

	u32 uAnimIdx = (u32)(aParams[3]);

	pBot->UserAnim_Attach(uAnimIdx, pAnimInst);
	pBot->UserAnim_SetControlValue(uAnimIdx, 1.0f);
	pBot->UserAnim_EnableAllBones(uAnimIdx);

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_DetachAnim(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_DetachAnim", 2 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_DetachAnim() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_DetachAnim() : not a bot." );
		return((cell)(0));
	}

	u32 uAnimIdx = (u32)(aParams[2]);

	pBot->UserAnim_DisableAllBones(uAnimIdx);
	pBot->UserAnim_SetControlValue(uAnimIdx, 0.0f);
	pBot->UserAnim_Attach(uAnimIdx, NULL);

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_MaskUBody(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_MaskUBody", 3 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_MaskUBody() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_MaskUBody() : not a bot." );
		return((cell)(0));
	}

	u32 uAnimIdx = (u32)(aParams[2]);

	pBot->UserAnim_BatchUpdateTapBoneMask_Open( uAnimIdx, aParams[3] );
	pBot->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_UPPER_BODY );
	pBot->UserAnim_BatchUpdateTapBoneMask_Close();

	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_MaskLBody(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_MaskLBody", 3 );
	CBot *pBot = (CBot *)(aParams[1]);
	if(pBot == NULL)
	{
		SCRIPT_ERROR("Bot_MaskLBody() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_MaskLBody() : not a bot." );
		return((cell)(0));
	}

	u32 uAnimIdx = (u32)(aParams[2]);

	pBot->UserAnim_BatchUpdateTapBoneMask_Open( uAnimIdx, aParams[3] );
	pBot->UserAnim_BatchUpdateTapBoneMask( CBot::UABONEMASK_ENABLE_LOWER_BODY );
	pBot->UserAnim_BatchUpdateTapBoneMask_Close();

	return((cell)(0));
}

// This function will cause Blink to turn around and look at the camera.
// This is used in preparation to play back a BTR.
// The first parameter is a handle to Blink.  Use Bot_GetPlayer() to get a handle to the Blink character
// The second parameter should be TRUE to turn Blink backwards, and FALSE to turn him back forwards.
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_TurnAround( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_TurnAround", 2 );
	CBot *pBot = (CBot *)(aParams[1]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("Bot_TurnAround() : Passed in null bot.");
		return((cell)(0));
	}
	FASSERT(pBot != NULL);

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_TurnAround() : not a bot." );
		return((cell)(0));
	}

	BOOL bReverse = (BOOL)(aParams[2]);

	if( pBot->IsReverseFacingMode() && !bReverse )
	{
		pBot->EndReverseFacingMode();
	}
	else if( !pBot->IsReverseFacingMode() && bReverse )
	{
		pBot->StartReverseFacingMode();
	}

	return((cell)(0));
}

// This function returns TRUE when the bot is fully turned around facing backwards,
// otherwise it returns FALSE.
// The first parameter is a handle to the Bot.
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsTurnedAround( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsTurnedAround", 1 );
	CBot *pBot = (CBot *)(aParams[1]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("Bot_IsTurnedAround() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_IsTurnedAround() : not a bot." );
		return((cell)(0));
	}

	if( pBot->IsReverseFacingMode() && !pBot->IsReverseFacingForward() && !pBot->IsReverseTransition() )
	{
		return((cell)(1));
	}

	return((cell)(0));
}

// turns on or off the spotlight on a bot.
// first parameter is a handle to a bot.
// second parameter is true or false.  True = light on, false = light off.
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_SpotLightOn( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_SpotLightOn", 2 );
	CBot *pBot = (CBot *)(aParams[1]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("Bot_SpotLightOn() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_SpotLightOn() : not a bot." );
		return((cell)(0));
	}

	if( (BOOL)aParams[2] )
	{
		pBot->SetSpotLightOn();
	}
	else
	{
		pBot->SetSpotLightOff();
	}

	return((cell)(0));
}

// Tells a ZombieBot to start forming.
// Returns TRUE if successful.
//
// Bot_ZomStartForming( Bot )
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_ZomStartForming( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_ZomStartForming", 1 );
	CBot *pBot = (CBot *)(aParams[1]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("Bot_ZomStartForming() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_ZomStartForming() : not a bot." );
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOTZOM) )
	{
		SCRIPT_ERROR("Bot_ZomStartForming() : not a ZombieBot." );
		return((cell)(0));
	}

	CBotZom *pBotZom = (CBotZom *)pBot;

	if( pBotZom->StartForming() ) {
        return((cell)(1));
	} else {
        return((cell)(0));
	}
}

// Tells a ZombieBot to float apart into pieces (EMP effect).
// Returns TRUE if successful.
//
// Bot_ZomFloatPieces( Bot )
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_ZomFloatPieces( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_ZomFloatPieces", 1 );
	CBot *pBot = (CBot *)(aParams[1]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("Bot_ZomFloatPieces() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_ZomFloatPieces() : not a bot." );
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOTZOM) )
	{
		SCRIPT_ERROR("Bot_ZomFloatPieces() : not a ZombieBot." );
		return((cell)(0));
	}

	CBotZom *pBotZom = (CBotZom *)pBot;

	if( pBotZom->SpreadIntoHoveringPieces() ) {
        return((cell)(1));
	} else {
        return((cell)(0));
	}
}

// Tells a ZombieBot to collapse into debris.
// Returns TRUE if successful.
//
// Bot_ZomCollapse( Bot )
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_ZomCollapse( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_ZomCollapse", 1 );
	CBot *pBot = (CBot *)(aParams[1]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("Bot_ZomCollapse() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_ZomCollapse() : not a bot." );
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOTZOM) )
	{
		SCRIPT_ERROR("Bot_ZomCollapse() : not a ZombieBot." );
		return((cell)(0));
	}

	CBotZom *pBotZom = (CBotZom *)pBot;

	if( pBotZom->CollapseIntoDebris() ) {
        return((cell)(1));
	} else {
        return((cell)(0));
	}
}


// Sets the ability of a player to exit a possessed bot.  Can be called on bot any time,
// not just when it is possessed.
// First parameter is handle to the bot.
// Second parameter is true or false.  true = bot can be exited, false = can't be exited.
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_PossessExit( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_PossessExit", 2 );
	CBot *pBot = (CBot *)(aParams[1]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("Bot_PossessExit() : Passed in null bot.");
		return((cell)(0));
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR("Bot_PossessExit() : not a bot." );
		return((cell)(0));
	}

	pBot->SetPossessionExitable( !!(aParams[2]) );

	return 0;
}

/////////////////////////////////////////////////////////////
// Bot Talk Instance functions

// acquire a handle to a bot talk instance.
// no parameters.
// returns a handle to a bot talk instance, or NULL if failure.
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_AcquireBTI( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_AcquireBTI", 0 );
	CBotTalkInst *pBotTalk = fnew CBotTalkInst;
	if(pBotTalk == NULL)
	{
		SCRIPT_ERROR("BTI_AcquireBTI : Could not create bot talk instance.");
		return(NULL);
	}

	return((cell) pBotTalk );
}

// free a handle to a bot talk instance.
// first parameter is handle to instance.
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_ReleaseBTI( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_ReleaseBTI", 1 );
	CBotTalkInst *pBotTalk = (CBotTalkInst*) aParams[1];
	if(pBotTalk != NULL)
	{
		fdelete( pBotTalk );
	}
	else
	{
		SCRIPT_ERROR("BTI_ReleaseBTI : Releasing NULL bot talk instance.");
	}

	return((cell) 0 );
}

// init a bot talk instance with a bot talk filename.
// first parameter is handle to bot talk instance.
// second parameter is handle to the bot to which bot talk will be applied.
// third parameter is name of bot talk file.
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_InitBTI( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_InitBTI", 3 );
	CBotTalkInst *pBotTalk = (CBotTalkInst*) aParams[1];
	CBot *pBot = (CBot *)(aParams[2]);
	char szBotTalkName[100];
	FillStringFromCell(szBotTalkName, 99, pAMX, aParams[3]);

	if( pBot == NULL )
	{
		SCRIPT_ERROR("BTI_InitBTI : bot handle was NULL.");
		return((cell) 0 );
	}

	if(pBotTalk != NULL)
	{
		if( !pBotTalk->Init( szBotTalkName ) )
		{
			SCRIPT_ERROR("BTI_InitBTI : bot talk %s init failed.", szBotTalkName );
			return((cell) 0 );
		}
		pBotTalk->SetTalkReason(TALKACTIVITY2_AMBIENT);
	}
	else
	{
		SCRIPT_ERROR("BTI_InitBTI : bot talk %s handle was NULL.", szBotTalkName );
		return((cell) 0 );
	}

	return((cell) 1 );
}

// This function plays a bot talk animation on a bot.
// The first parameter is a handle to the bot talk instance.
// second parameter is handle to the bot to which bot talk will be applied.
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_PlayBTI( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_PlayBTI", 2 );
	CBotTalkInst *pBotTalk = (CBotTalkInst*) aParams[1];
	CBot *pBot = (CBot *)(aParams[2]);

	if(pBotTalk == NULL)
	{
		SCRIPT_ERROR("BTI_PlayBTI : bot talk instance was NULL.");
		return((cell) 0 );
	}

	if( pBot == NULL )
	{
		SCRIPT_ERROR("BTI_PlayBTI : bot handle was NULL.");
		return((cell) 0 );
	}

	// mask out legs?
	CTalkSystem2::SubmitTalkRequest( pBotTalk, pBot );

	return((cell)(0));
}

// returns TRUE if bot talk instance is current animating, false otherwise.
// The first parameter is a handle to the bot talk instance.
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_IsBTIPlaying( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_IsBTIPlaying", 1 );
	CBotTalkInst *pBotTalk = (CBotTalkInst*) aParams[1];

	if(pBotTalk == NULL)
	{
		SCRIPT_ERROR("BTI_IsBTIPlaying : bot talk instance was NULL.");
		return((cell) 0 );
	}

	return((cell) pBotTalk->IsWorking() );
}

// Sets BTI audio to play in 2D.  
// Must be called before calling BTI_PlayBTI().
// First parameter is a handle to a bot talk instance.
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_SetAudio2D( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_SetAudio2D", 1 );
	CBotTalkInst *pBotTalk = (CBotTalkInst*) aParams[1];

	if(pBotTalk == NULL)
	{
		SCRIPT_ERROR("BTI_SetAudio2D : bot talk instance was NULL.");
		return((cell) 0 );
	}

	pBotTalk->Force2dAudio();

	return((cell)(0));
}

// Sets audible radius of BTI audio (when audio is 3D).  
// Must be called before calling BTI_PlayBTI().
// First parameter is a handle to a bot talk instance.
// Second parameter is audible radius.
// Second parameter is floating point value, scripters must use f32_SetFromString() or f32_SetFromInt().
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_SetAudioRadius( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_SetAudioRadius", 2 );
	CBotTalkInst *pBotTalk = (CBotTalkInst*) aParams[1];

	if(pBotTalk == NULL)
	{
		SCRIPT_ERROR("BTI_SetAudioRadius : bot talk instance was NULL.");
		return((cell) 0 );
	}

	pBotTalk->SetAudioRadius( ConvertCellToF32(aParams[2]) );

	return((cell)(0));
}

// Ducks the audio levels of all duckable sounds when
// this sound plays.  This sound will NOT be ducked
// Must be called before calling BTI_PlayBTI().
// First parameter is a handle to a bot talk instance.
// Second parameter BOOL flag: TRUE = Duck audio -- FALSE = Don't Duck Audio.
cell AMX_NATIVE_CALL CMAST_BTIWrapper::BTI_DuckAudWhenPlay( AMX *pAMX, cell *aParams )
{
	SCRIPT_CHECK_NUM_PARAMS( "BTI_DuckAudWhenPly", 2 );
	CBotTalkInst *pBotTalk = (CBotTalkInst*) aParams[1];

	if(pBotTalk == NULL)
	{
		SCRIPT_ERROR("BTI_DuckAudWhenPly : bot talk instance was NULL.");
		return((cell) 0 );
	}

	pBotTalk->DuckAudioWhenPlaying( aParams[2] );

	return((cell)(0));
}

//
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// CBotGlitch functions.
cell AMX_NATIVE_CALL CMAST_BotWrapper::BotBlink_Acquire(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "BotBlink_Acquire", 0 );
	CBotGlitch *pBot = fnew CBotGlitch;
	if(pBot == NULL)
	{
		SCRIPT_ERROR("BotBlink_Acquire() : WHOAWHOAWHOAWHOA! Could not create BotBlink!");
		return((cell)(NULL));
	}

	return((cell)(pBot));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::BotBlink_Release(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "BotBlink_Release", 1 );
	CBotGlitch *pBot = (CBotGlitch *)(aParams[1]);
	fdelete(pBot);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::BotBlink_Init1(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "BotBlink_Init1", 2 );
	CBotGlitch *pBot = (CBotGlitch *)(aParams[1]);
	FASSERT(pBot != NULL);

	CFMtx43A mtxTemp;
	mtxTemp.Identity();

	CFVec3A *pvecPos = (CFVec3A *)(aParams[2]);
	mtxTemp.m_vPos = *pvecPos;

	pBot->Create(-1, FALSE, "ScriptBot", &mtxTemp, "default");

	return((cell)(0));
}

// aborts the scope zoom mode for all players
// first parameter is boolean, TRUE = abort immediately.
cell AMX_NATIVE_CALL CMAST_BotWrapper::BotBlink_AbortScope(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "BotBlink_AbortScope", 0 );
	BOOL bImmediate = (BOOL) aParams[1];
	CPlayer::AbortAllPlayerScopeZoomMode( bImmediate );
	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_BotWrapper::BotGlitch_FallDown(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "BotGlitch_FallDown", 0 );
	FASSERT(CPlayer::m_nCurrent < MAX_PLAYERS);
	CEntity* pEntity = Player_aPlayer[CPlayer::m_nCurrent].m_pEntityOrig;
	FASSERT(pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOTGLITCH));
	CBotGlitch* pGlitch = (CBotGlitch*)pEntity;
	pGlitch->FallDown(1.0f);
	return((cell)(0));
}

//
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Door functions.
BOOL CMAST_DoorWrapper::InitSystem()
{
	return(TRUE);
}

void CMAST_DoorWrapper::UninitSystem()
{
}

BOOL CMAST_DoorWrapper::InitLevel()
{
	return(TRUE);
}

void CMAST_DoorWrapper::UninitLevel()
{
}

cell AMX_NATIVE_CALL CMAST_DoorWrapper::Door_Find(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Door_Find", 1 );
	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[1]);
	CEntity *pDE = CEntity::Find(szEntityName);

	if(pDE == NULL)
	{
		SCRIPT_ERROR("Door_Find() : Could not find object named '%s'.", szEntityName);
		return((cell)(NULL));
	}

	if(!(pDE->TypeBits() & ENTITY_BIT_DOOR))
	{
		SCRIPT_ERROR("Door_Find() : Found object named '%s' that was not a door.", szEntityName);
		return((cell)(NULL));
	}

	return((cell)(pDE));
}

cell AMX_NATIVE_CALL CMAST_DoorWrapper::Door_IsValid(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Door_IsValid", 1 );
	return((cell)(aParams[1] != NULL));
}

cell AMX_NATIVE_CALL CMAST_DoorWrapper::Door_GotoPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Door_GotoPos", 2 );
	CDoorEntity *pDE = (CDoorEntity *)(aParams[1]);
	if(pDE == NULL)
	{
		SCRIPT_ERROR("Door_GotoPos() : Passed in null door.");
		return((cell)(0));
	}

	pDE->GotoPos(aParams[2], CDoorEntity::GOTOREASON_DESTINATION);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_DoorWrapper::Door_GetUnitPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Door_GetUnitPos", 1 );
	CDoorEntity *pDE = (CDoorEntity *)(aParams[1]);
	if(pDE == NULL)
	{
		SCRIPT_ERROR("Door_GetUnitPos() : Passed in null door.");
		return((cell)(0));
	}

	return(ConvertF32ToCell(pDE->GetUnitPos()));
}

cell AMX_NATIVE_CALL CMAST_DoorWrapper::Door_GetUnitPosMap(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Door_GetUnitPosMap", 1 );
	CDoorEntity *pDE = (CDoorEntity *)(aParams[1]);
	if(pDE == NULL)
	{
		SCRIPT_ERROR("Door_GetUnitPosMap() : Passed in null door.");
		return((cell)(0));
	}

	return(ConvertF32ToCell(pDE->GetUnitPosMapped()));
}

cell AMX_NATIVE_CALL CMAST_DoorWrapper::Door_SetLock(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Door_SetLock", 2 );
	CDoorEntity *pDE = (CDoorEntity *)(aParams[1]);
	if(pDE == NULL)
	{
		SCRIPT_ERROR("Door_SetLock() : Passed in null door.");
		return((cell)(0));
	}

	pDE->SetLockState(aParams[2]);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_DoorWrapper::Door_SnapToPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Door_SnapToPos", 2 );
	CDoorEntity *pDE = (CDoorEntity *)(aParams[1]);
	if(pDE == NULL)
	{
		SCRIPT_ERROR("Door_SnapToPos() : Passed in null door.");
		return((cell)(0));
	}

	pDE->SnapToPos(aParams[2]);

	return((cell)(0));
}

//
/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Special CFVec3Obj functions... (they require knowledge of entities)

cell AMX_NATIVE_CALL CMAST_Vec3ObjWrapper::FV3Points1_Init1(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Points1_Init1", 4 );
	char szPathName[20];
	FillStringFromCell(szPathName, 19, pAMX, aParams[2]);

	CEntity *pE = CEntity::Find(szPathName);
	if(pE == NULL)
	{
		SCRIPT_ERROR("FV3Points1_Init1() : Could not find entity named '%s'.", szPathName);
		return((cell)(0));
	}
	if(!(pE->TypeBits() & ENTITY_BIT_SPLINE))
	{
		SCRIPT_ERROR("FV3Points1_Init1() : Found object named '%s' that was not a spline.", szPathName);
		return((cell)(0));
	}
	CESpline *pSE = (CESpline *)(pE);

	CFV3OPointPath1 *pVO = (CFV3OPointPath1 *)(aParams[1]);
	FASSERT(pVO != NULL);

	f32 fTotalTime = ConvertCellToF32(aParams[3]);

	if( fTotalTime <= 0.0f )
	{
		SCRIPT_ERROR("FV3Points1_Init1() : time must be greater than zero.", szPathName);
		return((cell)(0));
	}

	pVO->Init(pSE->PointArray(), pSE->PointCount(), fTotalTime, TRUE, aParams[4]);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_Vec3ObjWrapper::FV3Points2_Init1(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FV3Points2_Init1", 4 );
	char szPathName[20];
	FillStringFromCell(szPathName, 19, pAMX, aParams[2]);

	CEntity *pE = CEntity::Find(szPathName);
	if(pE == NULL)
	{
		SCRIPT_ERROR("FV3Points2_Init1() : Could not find entity named '%s'.", szPathName);
		return((cell)(0));
	}
	if(!(pE->TypeBits() & ENTITY_BIT_SPLINE))
	{
		SCRIPT_ERROR("FV3Points2_Init1() : Found object named '%s' that was not a spline.", szPathName);
		return((cell)(0));
	}
	CESpline *pSE = (CESpline *)(pE);

	CFV3OPointPath2 *pVO = (CFV3OPointPath2 *)(aParams[1]);
	FASSERT(pVO != NULL);

	f32 fTotalTime = ConvertCellToF32(aParams[3]);

	if( fTotalTime <= 0.0f )
	{
		SCRIPT_ERROR("FV3Points2_Init1() : time must be greater than zero.", szPathName);
		return((cell)(0));
	}

	pVO->Init(pSE->PointArray(), pSE->PointCount(), fTotalTime, aParams[4]);

	return((cell)(0));
}

//
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Special CFQuatObj functions... (they require knowledge of entities)
cell AMX_NATIVE_CALL CMAST_QuatObjWrapper::FQuatSLERP_Init1(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FQuatSLERP_Init1", 5 );
	// Get 
	CFQuatSLERP *pQO = (CFQuatSLERP *)(aParams[1]);
	if(pQO == NULL)
	{
		SCRIPT_ERROR("FQuatSLERP_Init1() : Passed in NULL FQuatSLERP.");
		return((cell)(0));
	}

	// Get the starting entity...
	CEntity *pE1 = (CEntity *)(aParams[2]);
	if(pE1 == NULL)
	{
		SCRIPT_ERROR("FQuatSLERP_Init1() : Passed in NULL entity #1.");
		return((cell)(0));
	}

	// Get the ending entity...
	CEntity *pE2 = (CEntity *)(aParams[3]);
	if(pE2 == NULL)
	{
		SCRIPT_ERROR("FQuatSLERP_Init1() : Passed in NULL entity #2.");
		return((cell)(0));
	}

	// Get the total time.
	f32 fTotalTime = ConvertCellToF32(aParams[4]);
	if(fTotalTime <= 0.0f)
	{
		SCRIPT_ERROR("FQuatSLERP_Init1() : Invalid total time : %f.", fTotalTime);
		return((cell)(0));
	}

	CFQuatA qQuat1, qQuat2;
	qQuat1.BuildQuat(*pE1->MtxToWorld());
	qQuat2.BuildQuat(*pE2->MtxToWorld());

	pQO->Init(&qQuat1, &qQuat2, fTotalTime, aParams[5]);

	return((cell)(0));
}
//
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Camera functions...

CCamManualInfo *CMAST_CamWrapper::m_pCamInfo = NULL;
BOOL CMAST_CamWrapper::m_bCamInitted = FALSE;

BOOL CMAST_CamWrapper::InitSystem()
{
	FASSERT(m_pCamInfo == NULL);
	m_pCamInfo = fnew CCamManualInfo;
	if(m_pCamInfo == NULL)
	{
		SCRIPT_ERROR("CMAST_CamWrapper::InitSystem() : Could not create manual camera info.");
		return(TRUE);
	}
	m_bCamInitted = FALSE;

	return(TRUE);
}

void CMAST_CamWrapper::UninitSystem()
{
	m_bCamInitted = FALSE;

	fdelete(m_pCamInfo);
	m_pCamInfo = NULL;
}

BOOL CMAST_CamWrapper::InitLevel()
{
	m_bCamInitted = FALSE;

	return(TRUE);
}

void CMAST_CamWrapper::UninitLevel()
{
	m_bCamInitted = FALSE;
}

cell AMX_NATIVE_CALL CMAST_CamWrapper::Cam_Init(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Cam_Init", 3 );

	if (m_bCamInitted)
	{
		SCRIPT_ERROR("CAM_INIT called twice. Call CAM_Deactivate first");
	}

	m_pCamInfo->m_pPosObj = (CFVec3AObj *)(aParams[1]);
	m_pCamInfo->m_pQuatObj = (CFQuatObj *)(aParams[2]);
	m_pCamInfo->m_fHalfFOV = FMATH_DEG2RAD(ConvertCellToF32(aParams[3]));

	m_bCamInitted = TRUE;

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_CamWrapper::Cam_Activate(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Cam_Activate", 0 );
	if(!m_bCamInitted)
	{
		SCRIPT_ERROR("Cam_Activate() : Tried to activate camera that has not been initted.");
		return((cell)(0));
	}

	gamecam_SwitchPlayerToManualCamera(GAME_CAM_PLAYER_1, m_pCamInfo);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_CamWrapper::Cam_Deactivate(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Cam_Deactivate", 0 );
	if(!m_bCamInitted)
	{
		SCRIPT_ERROR("Cam_Deactivate() : Tried to deactivate camera that has not been initted.");
		return((cell)(0));
	}

	CEntity *pE = CPlayer::m_pCurrent->m_pEntityCurrent;
	FASSERT(pE->TypeBits() & ENTITY_BIT_BOT);
	CBot *pBot = (CBot *)(pE);
	gamecam_SwitchPlayerTo3rdPersonCamera(GAME_CAM_PLAYER_1, pBot);
	
	m_bCamInitted = FALSE;

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_CamWrapper::Cam_Shake(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Cam_Shake", 2 );
	f32 fIntensity = ConvertCellToF32(aParams[1]);
	f32 fDuration = ConvertCellToF32(aParams[2]);

	gamecam_GetActiveCamera()->ShakeCamera( fIntensity * 0.25f, fDuration );

	return((cell)(0));
}

//
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
// Camera Animation functions...

CCamCutsceneInfo *CMAST_CamAnimWrapper::m_pCamInfo = NULL;

BOOL CMAST_CamAnimWrapper::InitSystem()
{
	FASSERT(m_pCamInfo == NULL);
	m_pCamInfo = fnew CCamCutsceneInfo;
	if(m_pCamInfo == NULL)
	{
		SCRIPT_ERROR("CMAST_CamAnimWrapper::InitSystem() : Could not create cutscene camera info.");
		return(TRUE);
	}
	return(TRUE);
}

void CMAST_CamAnimWrapper::UninitSystem()
{
	fdelete(m_pCamInfo);
	m_pCamInfo = NULL;
}

BOOL CMAST_CamAnimWrapper::InitLevel()
{
	return(TRUE);
}

void CMAST_CamAnimWrapper::UninitLevel()
{
}



void CMAST_CamAnimWrapper::Work() {
	if( ( m_pCamInfo ) && ( m_pCamInfo->GetCamAnimInst() ) ) {
		//check to see if the current animation is done...
		if( m_pCamInfo->IsDone() ) {
			CamAnim_EndCutscene();
		}
	}
}

cell AMX_NATIVE_CALL CMAST_CamAnimWrapper::CamAnim_GetHandle(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "CamAnim_GetHandle", 1 );

	char szCamAnimName[32];
	FillStringFromCell(szCamAnimName, 31, pAMX, aParams[1]);

	//create a CFCamAnimInst Instance...
	CFCamAnimInst* pCamAnimInst = CFCamAnimInst::Load( szCamAnimName );
	if(!pCamAnimInst)
	{
		SCRIPT_ERROR( "CamAnim_GetHandle() : Failure creating camera animation %s.", szCamAnimName );
		return((cell)(0));
	}

	return((cell) pCamAnimInst );
}



cell AMX_NATIVE_CALL CMAST_CamAnimWrapper::CamAnim_Start(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "CamAnim_Start", 1 );

	CFCamAnimInst* pCamAnimInst = ( CFCamAnimInst* ) aParams[1];
	if( !pCamAnimInst ) {
		SCRIPT_ERROR("CamAnim_Start() : Passed in a NULL CamAnimHandle.");
		return((cell)(0));
	}

	if( !m_pCamInfo ) {
		SCRIPT_ERROR("CamAnim_Start() : Do not have a valid CamInfo Structure.");
		return((cell)(0));
	}

	CamAnim_Start( pCamAnimInst, FALSE, TRUE );
	return((cell)(0));
}


// This call is identical to the one above, except that you can specify is you want
// the letterboxes to scroll on immediately
cell AMX_NATIVE_CALL CMAST_CamAnimWrapper::CamAnim_StartEx(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "CamAnim_Start", 2 );

	CFCamAnimInst* pCamAnimInst = ( CFCamAnimInst* ) aParams[1];
	if( !pCamAnimInst ) {
		SCRIPT_ERROR("CamAnim_Start() : Passed in a NULL CamAnimHandle.");
		return((cell)(0));
	}

	if( !m_pCamInfo ) {
		SCRIPT_ERROR("CamAnim_Start() : Do not have a valid CamInfo Structure.");
		return((cell)(0));
	}

	CamAnim_Start( pCamAnimInst, aParams[2], TRUE );
	return((cell)(0));
}


// This call is identical to the one above, except that you can specify if you want
// the letterboxes to scroll on immediately, and WHETHER OR NOT TO DISABLE THE AI ATTACK LOGIC
cell AMX_NATIVE_CALL CMAST_CamAnimWrapper::CamAnim_StartEx2(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "CamAnim_Start", 3 );

	CFCamAnimInst* pCamAnimInst = ( CFCamAnimInst* ) aParams[1];
	if( !pCamAnimInst ) {
		SCRIPT_ERROR("CamAnim_Start() : Passed in a NULL CamAnimHandle.");
		return((cell)(0));
	}

	if( !m_pCamInfo ) {
		SCRIPT_ERROR("CamAnim_Start() : Do not have a valid CamInfo Structure.");
		return((cell)(0));
	}

	CamAnim_Start( pCamAnimInst, aParams[2], aParams[3] );
	return((cell)(0));
}


void CMAST_CamAnimWrapper::CamAnim_Start( CFCamAnimInst* pCamAnimInst, BOOL bLetterBoxImmediate, BOOL bDisableAIAttackMode ) {

	pCamAnimInst->Reset(); //set to the begining of the animiation...
	m_pCamInfo->SetCamAnimInst( pCamAnimInst );

	gamecam_SwitchPlayerToCutsceneCamera(GAME_CAM_PLAYER_1, m_pCamInfo);

	//go into letterbox mode... (same code as game begin cutscene)
	CPlayer::m_pCurrent->DisableEntityControl();
	CAIBrain* pBrain = Player_aPlayer[0].m_pEntityCurrent->AIBrain();
	if (Player_aPlayer[0].m_pEntityCurrent ->TypeBits() & ENTITY_BIT_BOT)
	{
		((CBot*)Player_aPlayer[0].m_pEntityCurrent)->HeadStopLook();
	}
	aibrainman_Activate(pBrain);
	if( bDisableAIAttackMode ) { 
		// This is normally done for most ingame cutscenes
		// The only desire to skip this step would be for ingame cutscene creation modes for BINK movies...
		ai_NotifyCutSceneBegin();
		CBot::DisableBotDamageGlobally();
	}
	CBot::SetCutscenePlaying( TRUE );
	game_EnterLetterbox( pCamAnimInst->GetCamAnim()->szName, bLetterBoxImmediate );
}


cell AMX_NATIVE_CALL CMAST_CamAnimWrapper::CamAnim_End(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "CamAnim_End", 0 );

	CamAnim_EndCutscene();
	return((cell)(0));
}


void CMAST_CamAnimWrapper::CamAnim_EndCutscene( void ) {

	if( !m_pCamInfo ) {
		SCRIPT_ERROR("CamAnim_Start() : Do not have a valid CamInfo Structure.");
		return;
	}

	if( m_pCamInfo->GetCamAnimInst() ) { //a cutscene is currently active...
		
		m_pCamInfo->SetCamAnimInst( NULL );
		CEntity *pE = CPlayer::m_pCurrent->m_pEntityCurrent;
		FASSERT(pE->TypeBits() & ENTITY_BIT_BOT);
		CBot *pBot = (CBot *)(pE);
		gamecam_SwitchPlayerTo3rdPersonCamera(GAME_CAM_PLAYER_1, pBot);

		//disable the letterbox mode...
		aibrainman_Deactivate(Player_aPlayer[0].m_pEntityCurrent->AIBrain());
		CPlayer::m_pCurrent->EnableEntityControl();
		aibrainman_ConfigurePlayerBotBrain(Player_aPlayer[0].m_pEntityCurrent->AIBrain(), 0);
		ai_NotifyCutSceneEnd();
		if (Player_aPlayer[0].m_pEntityCurrent ->TypeBits() & ENTITY_BIT_BOT)
		{
			((CBot*)Player_aPlayer[0].m_pEntityCurrent)->HeadLook();
		}
		CBot::EnableBotDamageGlobally();
		CBot::SetCutscenePlaying( FALSE );
		game_LeaveLetterbox();
	}
}

//
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
//

BOOL CMAST_GoodieBag::InitSystem()
{
	return(TRUE);
}


void CMAST_GoodieBag::UninitSystem()
{
}


BOOL CMAST_GoodieBag::InitLevel()
{
	return(TRUE);
}


void CMAST_GoodieBag::UninitLevel()
{

}




f32 CMAST_Timer::m_fTimer = 0.0f;
u32 CMAST_Timer::m_nTimerEvent = 0;
CMAST_ETimer_t CMAST_Timer::m_ETimers[ NUM_MA_ETIMERS ];
CMAST_ETimer_t *CMAST_Timer::m_pIconTimer;

BOOL CMAST_Timer::InitLevel() {
	//reset the timer variables and get the nTimerEvent
	fang_MemSet(&m_ETimers, 0, sizeof( CMAST_ETimer_t ) * NUM_MA_ETIMERS );
	m_nTimerEvent = CFScriptSystem::GetEventNumFromName("etimer");
	m_pIconTimer = NULL;
	if( m_nTimerEvent == -1 ) {
		DEVPRINTF("[ CMAST_TIMER::InitLevel ] -- NO TIMER EVENT ID FOUND!!!\n");
		return FALSE;
	}
	return TRUE;		 
}

void CMAST_Timer::UninitLevel() {
	//clear the timer variables
	fang_MemSet(&m_ETimers, 0, sizeof( CMAST_ETimer_t ) * NUM_MA_ETIMERS );
}



void CMAST_Timer::Work()
{
	if(m_fTimer >= 0.0f)
	{
		m_fTimer -= FLoop_fPreviousLoopSecs;
	}

	//now manage the ETIMER structures
	u32 uCurETimerIdx;
	for(uCurETimerIdx = 0; uCurETimerIdx < NUM_MA_ETIMERS; ++uCurETimerIdx) {
		if( ( m_ETimers[ uCurETimerIdx ].uTimerStats & _ETIMER_FLAGS_ACTIVE ) ) {
			//this is an active, acquired timer
			m_ETimers[ uCurETimerIdx ].fTimer -= FLoop_fPreviousLoopSecs;
			if( m_ETimers[ uCurETimerIdx ].fTimer <= 0.0f ) {
				//this timer just struck midnight, increment it's trigger count and trigger an event.
				m_ETimers[ uCurETimerIdx ].uTriggerCount++;
				m_ETimers[ uCurETimerIdx ].uTimerStats &= ~_ETIMER_FLAGS_ACTIVE; 
				CFScriptSystem::TriggerEvent( m_nTimerEvent, ( u32 ) &m_ETimers[ uCurETimerIdx ], 
					( u32 ) m_ETimers[ uCurETimerIdx ].uTriggerCount, 0 );
			}
		}
	}

	// set draw flags every frame to avoid problems with draw flag save/restore on vehicle entry/exit.
	if( m_pIconTimer )
	{
		CPlayer::m_pCurrent->m_Hud.AddDrawFlags( CHud2::DRAW_ICON_TIMER );
	}
	else
	{
		CPlayer::m_pCurrent->m_Hud.ClearDrawFlags( CHud2::DRAW_ICON_TIMER );
	}
}

cell AMX_NATIVE_CALL CMAST_Timer::Timer_Set(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Timer_Set", 1 );
	m_fTimer = ConvertCellToF32(aParams[1]);

	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_Timer::Timer_IsDone(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Timer_IsDone", 0 );
	return(m_fTimer <= 0.0f);
}


cell AMX_NATIVE_CALL CMAST_Timer::Timer_AcquireETimer( AMX *pAMX, cell *aParams ) {
	u32 uCurETimerIdx;
	CMAST_ETimer_t *pTimerToAllocate = NULL;
	for(uCurETimerIdx = 0; uCurETimerIdx < NUM_MA_ETIMERS; ++uCurETimerIdx) {
		if( !( m_ETimers[ uCurETimerIdx ].uTimerStats & _ETIMER_FLAGS_ACQUIRED ) ) {
			pTimerToAllocate = &m_ETimers[ uCurETimerIdx ];
			break;
		}
	}

	//check to see if we have a free timer
	if( !pTimerToAllocate ) {
		SCRIPT_ERROR("Timer_AcquireETimer : No more timers available!");
		return NULL;
	}

	//mark the timer as acquired, and reset it's trigger count and timer value to zero
	pTimerToAllocate->uTimerStats = _ETIMER_FLAGS_ACQUIRED;
	pTimerToAllocate->uTriggerCount = 0;
	pTimerToAllocate->fTimer = 0.0f;
	return((cell)(pTimerToAllocate));
}


cell AMX_NATIVE_CALL CMAST_Timer::Timer_ReleaseETimer( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "Timer_ReleaseETimer", 1 );
	CMAST_ETimer_t *pETimer = ( CMAST_ETimer_t *) aParams[1];

	if( pETimer ) {
		//make sure this timer is acquired
		if( ! (pETimer->uTimerStats & _ETIMER_FLAGS_ACQUIRED ) ) {
			SCRIPT_ERROR("Timer_ReleaseETimer : Releasing an ETimer thats not acquired");
		} else {
			pETimer->uTimerStats = _ETIMER_FLAGS_NONE;
			pETimer->uTriggerCount = 0;
			pETimer->fTimer = 0.0f;
		}
	} else {
		SCRIPT_ERROR("Timer_ReleaseETimer : Releasing a NULL ETimer");
	}
	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_Timer::Timer_SetETimer( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "Timer_SetETimer", 2 );
	CMAST_ETimer_t *pETimer = ( CMAST_ETimer_t *) aParams[1];
	f32 fDuration = ConvertCellToF32(aParams[2]);

	if( pETimer ) {
		//make sure the timer is acquired
		if( ! (pETimer->uTimerStats & _ETIMER_FLAGS_ACQUIRED ) ) {
			SCRIPT_ERROR("Timer_SetETimer : Setting an ETimer that has not been acquired");
		} else {
			pETimer->fTimer = fDuration;
			pETimer->uTimerStats |= _ETIMER_FLAGS_ACTIVE;
		}
	} else {
		SCRIPT_ERROR("Timer_SetETimer : Setting a NULL ETimer");
	}
	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_Timer::Timer_ShowIcon( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "Timer_ShowIcon", 1 );
	CMAST_ETimer_t *pETimer = ( CMAST_ETimer_t *) aParams[1];

	if( pETimer ) {
		//make sure the timer is acquired
		if( ! (pETimer->uTimerStats & _ETIMER_FLAGS_ACQUIRED ) ) {
			SCRIPT_ERROR("Timer_ShowIcon : ETimer has not been acquired");
		} else {
			m_pIconTimer = pETimer;
			CPlayer::m_pCurrent->m_Hud.SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_RACE, TRUE /*bDraw*/, &m_pIconTimer->fTimer );
		}
	} else {
		SCRIPT_ERROR("Timer_ShowIcon : NULL ETimer handle");
	}
	return((cell)(0));
}

cell AMX_NATIVE_CALL CMAST_Timer::Timer_HideIcon( AMX *pAMX, cell *aParams ) {
	m_pIconTimer = NULL;
	CPlayer::m_pCurrent->m_Hud.SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_RACE, FALSE );
	return((cell)(0));
}

//serialize the current state of the Script timer variables
void CMAST_Timer::CheckPointSave( void ) {
	u32 uCurETimerIdx;

	//save off the value of the global timer
	CFCheckPoint::SaveData( m_fTimer );
	CFCheckPoint::SaveData( (u32&) m_pIconTimer );

	//now, save off the ETimer Data.
	for(uCurETimerIdx = 0; uCurETimerIdx < NUM_MA_ETIMERS; uCurETimerIdx++) {
		CFCheckPoint::SaveData( (void*) &m_ETimers[ uCurETimerIdx ], sizeof( CMAST_ETimer_t ) ); 
	}
}

//restore the script timers to a previous serialized state
void CMAST_Timer::CheckPointRestore( void ) {
	u32 uCurETimerIdx;

	//restore the value of the global timer
	CFCheckPoint::LoadData( m_fTimer );
	CFCheckPoint::LoadData( (u32&) m_pIconTimer );

	//now, restore the ETimer Data.
	for(uCurETimerIdx = 0; uCurETimerIdx < NUM_MA_ETIMERS; uCurETimerIdx++) {
		CFCheckPoint::LoadData( (void*) &m_ETimers[ uCurETimerIdx ], sizeof( CMAST_ETimer_t ) ); 
	}

	// restore the icon timer
	if( m_pIconTimer ) {
		CPlayer::m_pCurrent->m_Hud.SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_RACE, TRUE /*bDraw*/, &m_pIconTimer->fTimer );
	} else {
		CPlayer::m_pCurrent->m_Hud.SetIconTimerDraw( CHud2::ICON_TIMER_TYPE_RACE, FALSE );
	}
}

//
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// CGoodieBag functions.
// goodie bag functions don't appear to be exposed to scripters... -cjm
cell AMX_NATIVE_CALL GB_GetNumGIs(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}
	if(pE->m_pGoodieBag == NULL)
	{
		return((cell)(0));
	}
	else
	{
		return((cell)(pE->m_pGoodieBag->NumGoodies()));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_GetProb(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}
	if(pE->m_pGoodieBag == NULL)
	{
		return((ConvertF32ToCell(0.0f)));
	}
	else
	{
//		return((ConvertF32ToCell(pE->m_pGoodieBag->m_fProb)));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_SetProb(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_GetGoodieType(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_SetGoodieType(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_GetGoodieQuant1(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_SetGoodieQuant1(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_GetGoodieQuant2(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_SetGoodieQuant2(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_GetGoodieProb(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

cell AMX_NATIVE_CALL GB_SetGoodieProb(AMX *pAMX, cell *aParams)
{
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		return((cell)(0));
	}

	return((cell)(0));
}

//
/////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////
// CHud2 Functions.

BOOL CMAST_Hud::InitSystem()
{
	return(TRUE);
}

void CMAST_Hud::UninitSystem()
{
}

cell AMX_NATIVE_CALL CMAST_Hud::HUD_StartCAMsg(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "HUD_StartCAMsg", 1 );
	FSndFx_FxHandle_t hWave = (FSndFx_FxHandle_t)(aParams[1]);

	CHud2::GetCurrentHud()->TransmissionMsg_Start( CHud2::TRANSMISSION_AUTHOR_COLONEL_ALLOY, hWave, 1.0f, TRUE );

	return((cell)(0));
}


// HUD_RadioStartBank SoundHandle, nAuthorCode, fUnitVolume, bAbortWithCutScene
//
// Returns 1 if successful, or 0 if not.
cell AMX_NATIVE_CALL CMAST_Hud::HUD_RadioStartBank(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "HUD_RadioStartBank", 4 );

	FSndFx_FxHandle_t hSound = (FSndFx_FxHandle_t)(aParams[1]);
	if( hSound == FSNDFX_INVALID_FX_HANDLE )
	{
		SCRIPT_ERROR("HUD_RadioStartBank : NULL sound handle passed in.");
		return (cell)0;
	}

	CHud2::TransmissionAuthor_e nAuthorCode = (CHud2::TransmissionAuthor_e)aParams[2];
	if( nAuthorCode<0 || nAuthorCode>=CHud2::TRANSMISSION_AUTHOR_COUNT )
	{
		SCRIPT_ERROR("HUD_RadioStartBank : Invalid author code passed in.");
		return (cell)0;
	}

	f32 fUnitVolume = ConvertCellToF32( aParams[3] );
	FMATH_CLAMP( fUnitVolume, 0.0f, 1.0f );

	BOOL bAbortWithCutScene = (BOOL)aParams[4];

	CHud2::GetCurrentHud()->TransmissionMsg_Start( nAuthorCode, hSound, fUnitVolume, bAbortWithCutScene );

	return (cell)1;
}


// HUD_RadioStartStrm pszStreamingFileName, nAuthorCode, fUnitVolume, bAbortWithCutScene
//
// Returns 1 if successful, or 0 if not.
cell AMX_NATIVE_CALL CMAST_Hud::HUD_RadioStartStrm(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "HUD_RadioStartStrm", 4 );

	char szFileName[100];
	FillStringFromCell( szFileName, 99, pAMX, aParams[1] );
	if( szFileName[0] == 0 )
	{
		SCRIPT_ERROR("HUD_RadioStartStrm : NULL file name passed in.");
		return (cell)0;
	}

	CHud2::TransmissionAuthor_e nAuthorCode = (CHud2::TransmissionAuthor_e)aParams[2];
	if( nAuthorCode<0 || nAuthorCode>=CHud2::TRANSMISSION_AUTHOR_COUNT )
	{
		SCRIPT_ERROR("HUD_RadioStartStrm : Invalid author code passed in.");
		return (cell)0;
	}

	f32 fUnitVolume = ConvertCellToF32( aParams[3] );
	FMATH_CLAMP( fUnitVolume, 0.0f, 1.0f );

	BOOL bAbortWithCutScene = (BOOL)aParams[4];

	CHud2::GetCurrentHud()->TransmissionMsg_Start( nAuthorCode, szFileName, fUnitVolume, bAbortWithCutScene );

	return (cell)1;
}


// HUD_RadioStop bFadeOut
cell AMX_NATIVE_CALL CMAST_Hud::HUD_RadioStop(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "HUD_RadioStop", 1 );

	BOOL bFadeOut = (BOOL)aParams[1];

	CHud2::GetCurrentHud()->TransmissionMsg_Stop( bFadeOut );

	return (cell)0;
}


// HUD_RadioIsPlaying
//
// Returns 1 if the radio is playing, or 0 otherwise.
cell AMX_NATIVE_CALL CMAST_Hud::HUD_RadioIsPlaying(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "HUD_RadioIsPlaying", 0 );

	return (cell)!CHud2::GetCurrentHud()->TransmissionMsg_IsDonePlaying();
}


// HUD_SetMissionText nMissionTextIndex
//
// Sets the mission text displayed in the pause info screen.
// Returns TRUE if successful.
// Returns FALSE if an invalid index was passed in (in this case, the current mission will not change).
cell AMX_NATIVE_CALL CMAST_Hud::HUD_SetMissionText( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "HUD_SetMissionText", 1 );

	return (cell)level_SetCurrentMissionTextIndex( (BOOL)aParams[1] );
}


// HUD_GetMissionIndex
//
// Returns the index of the current mission text entry (0 is the first entry, 1 is the next, etc.).
// If there are no entries, returns 0.
cell AMX_NATIVE_CALL CMAST_Hud::HUD_GetMissionIndex( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "HUD_GetMissionIndex", 0 );

	return (cell)level_GetCurrentMissionTextIndex();
}

//
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Music functions

cell AMX_NATIVE_CALL CMAST_Music::Music_Play(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Music_Play", 1 );
	char szSongName[64];
	FillStringFromCell(szSongName, 63, pAMX, aParams[1]);

	level_PlayMusic( szSongName, 1.0f, FALSE );

	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::Music_PlayEx(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Music_PlayEx", 3 );
	char szSongName[64];
	u32 uVol = (u32) (aParams[2]);
	BOOL bFadeIn = (BOOL) (aParams[3]);
	FillStringFromCell(szSongName, 63, pAMX, aParams[1]);

	level_PlayMusic( szSongName, uVol*0.01f, bFadeIn );

	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::Music_PlayDefault(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Music_PlayDefault", 0 );
	level_PlayDefaultStreamingTrack( 1.0f, FALSE );

	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::Music_PlayDefaultEx(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Music_PlayDefaultEx", 1 );
	u32 uVol = (u32) (aParams[1]);
	BOOL bFadeIn = (BOOL) (aParams[2]);

	level_PlayDefaultStreamingTrack( uVol*0.01f, bFadeIn );

	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::Music_Stop(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Music_Stop", 0 );
	level_StopMusic();
	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::StreamingSfx_Play(AMX *pAMX, cell *aParams) {
	SCRIPT_CHECK_NUM_PARAMS( "StreamingSfx_Play", 1 );
	char szSongName[64];
	FillStringFromCell(szSongName, 63, pAMX, aParams[1]);

	level_PlaySfxStream( szSongName, 1.0f, FALSE );

	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::StreamingSfx_PlayEx(AMX *pAMX, cell *aParams) {
	SCRIPT_CHECK_NUM_PARAMS( "StreamingSfx_PlayEx", 3 );
	char szSongName[64];
	u32 uVol = (u32) (aParams[2]);
	BOOL bFadeIn = (BOOL) (aParams[3]);
	FillStringFromCell(szSongName, 63, pAMX, aParams[1]);

	level_PlaySfxStream( szSongName, uVol*0.01f, bFadeIn );

	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::Speech_Play(AMX *pAMX, cell *aParams) {
	SCRIPT_CHECK_NUM_PARAMS( "Speech_Play", 3 );
	
	char szSongName[64];
	u32 uVol = (u32) (aParams[2]);
	u32 uLoops = (BOOL) (aParams[3]);
	FillStringFromCell(szSongName, 63, pAMX, aParams[1]);

	level_PlayStreamingSpeech(szSongName, uVol*0.01f, uLoops);

	return( (cell) 0 );
}

cell AMX_NATIVE_CALL CMAST_Music::Speech_Stop(AMX *pAMX, cell *aParams) {
	SCRIPT_CHECK_NUM_PARAMS( "Speech_Stop", 0 );
	
	// assumes that StreamingSfx plays within the speech channel;
	level_StopStreamingSpeech();

	return( (cell) 0 );
}

//
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
// Audio functions

//gets a SFX handle from a character string
cell AMX_NATIVE_CALL CMAST_Audio::Audio_GetSFXHandle(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_GetSFXHandle", 1 );

	char szSFXName[64];
	FillStringFromCell(szSFXName, 63, pAMX, aParams[1]);

	FSndFx_FxHandle_t hSFX = fsndfx_GetFxHandle( szSFXName );
	if( hSFX == FSNDFX_INVALID_FX_HANDLE && faudio_IsInstalled() ) {
		SCRIPT_ERROR("Audio_GetSFXHandle() : Invalid SFX Handle returned." );
	}
	return( (cell) hSFX );
}

//sets an entities Ambient sound SFX handle to the passed in handle
cell AMX_NATIVE_CALL CMAST_Audio::Audio_SetAmbientSFX(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_SetAmbientSFX", 2 );

	CEntity *pE = ( CEntity * )(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Audio_SetAmbientSFX : Passed in NULL entity.");
		return((cell)(0));
	}

	FSndFx_FxHandle_t hSFX = (FSndFx_FxHandle_t)(aParams[2]);
	if( ( hSFX == NULL ) || ( hSFX == FSNDFX_INVALID_FX_HANDLE ) )
	{
		// No-audio test runs intentionally skip every SFX bank. Scripts still resolve
		// their optional ambient sounds; treat the resulting zero handle as a no-op.
		if( !faudio_IsInstalled() ) return((cell)(0));
		SCRIPT_ERROR("Audio_SetAmbientSFX() : Invalid SFX Handle provided." );
		return((cell)(0));
	}

	pE->SetAmbientSFXHandle( hSFX );
	return( (cell)(0) );
}

//Gets an entities Ambient sound SFX handle
cell AMX_NATIVE_CALL CMAST_Audio::Audio_GetAmbientSFX(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_GetAmbientSFX", 1 );

	CEntity *pE = (CEntity *)(aParams[1]);
	if( pE == NULL )
	{
		SCRIPT_ERROR("Audio_GetAmbientSFX : Passed in NULL entity.");
		return((cell)(0));
	}

	return( (cell) pE->GetAmbientSFXHandle() );
}

//set the volume of an Entities ambient sound
//first parameter is entity
//second parameter is integer volume from 0 to 100
cell AMX_NATIVE_CALL CMAST_Audio::Audio_SetAmbientVol(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_SetAmbientVol", 2 );

	CEntity *pE = ( CEntity * )(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Audio_SetAmbientVol : Passed in NULL entity.");
		return((cell)(0));
	}

	u32 uVolume = aParams[2];
	
	pE->SetAmbientVolume( uVolume*0.01f );
	return( (cell)(0) );
}


//Gets an entities Ambient sound volume
//first parameter is entity
cell AMX_NATIVE_CALL CMAST_Audio::Audio_GetAmbientVol(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_GetAmbientVol", 1 );

	CEntity *pE = (CEntity *)(aParams[1]);
	if( pE == NULL )
	{
		SCRIPT_ERROR("Audio_GetAmbientVol : Passed in NULL entity.");
		return((cell)(0));
	}

	return( (cell) ( u32 ) ( pE->GetAmbientVolume() * 100 ) );
}


//set the radius of an Entities ambient sound
//first parameter is entity
//second parameter is f32 radius from 0.0 to Whatever
cell AMX_NATIVE_CALL CMAST_Audio::Audio_SetAmbientRad(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_SetAmbientRad", 2 );

	CEntity *pE = ( CEntity * )(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Audio_SetAmbientRad : Passed in NULL entity.");
		return((cell)(0));
	}

	f32 fRadius = ConvertCellToF32( aParams[2] );
	
	pE->SetAmbientRadius( fRadius );
	return( (cell)(0) );
}


//Gets an entities f32 Ambient sound radius
//first parameter is entity
cell AMX_NATIVE_CALL CMAST_Audio::Audio_GetAmbientRad(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_GetAmbientRad", 1 );

	CEntity *pE = (CEntity *)(aParams[1]);
	if( pE == NULL )
	{
		SCRIPT_ERROR("Audio_GetAmbientRad : Passed in NULL entity.");
		return((cell)(0));
	}

	return ConvertF32ToCell( pE->GetAmbientRadius() );
}

// Starts the ambient sound for the indicated entity.
// First parameter is handle to entity.
cell AMX_NATIVE_CALL CMAST_Audio::Audio_PlayAmbient(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_PlayAmbient", 1 );

	CEntity *pEntity = (CEntity *)(aParams[1]);
	if(pEntity == NULL)
	{
		SCRIPT_ERROR("Audio_PlayAmbient() : Entity handle null." );
		return( (cell) 0 );
	}

	pEntity->PlayAmbientSFX();

	return( (cell) 0 );
}

// stops the ambient sound for the indicated entity
cell AMX_NATIVE_CALL CMAST_Audio::Audio_StopAmbient(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_StopAmbient", 1 );

	CEntity *pEntity = (CEntity *)(aParams[1]);
	if(pEntity == NULL)
	{
		SCRIPT_ERROR("Audio_StopAmbient() : Entity handle Null." );
		return( (cell) 0 );
	}

	pEntity->StopAmbientSFX();

	return( (cell) 0 );
}


// Plays a fire and forget 2D sound at the specified volume
// First parameter is the handle to the sfx to be played
// Second parameter is an integer volume level ranging from 0 (silence) to 100 (Full Vol)
cell AMX_NATIVE_CALL CMAST_Audio::Audio_Play2DSound(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_Play2DSound", 2 );

	FSndFx_FxHandle_t hSFX = (FSndFx_FxHandle_t)(aParams[1]);
	if( ( hSFX == NULL ) || ( hSFX == FSNDFX_INVALID_FX_HANDLE ) )
	{
		if( !faudio_IsInstalled() ) return((cell)(0));
		SCRIPT_ERROR("Audio_Play2DSound : Invalid SFX Handle provided." );
		return((cell)(0));
	}

	u32 uVolume = aParams[2];
	f32 fVolume = 0.01f * ( (f32) uVolume );
	FMATH_CLAMP_UNIT_FLOAT( fVolume );

	// Play the sound
	fsndfx_Play2D(hSFX, fVolume );

	return( (cell)(0) );
}


/////////////////////////////////////////////////////////////////
// Movie functions

//gets a movie handle from a character string
cell AMX_NATIVE_CALL CMAST_Movie::Movie_GetHandle(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Movie_GetHandle", 1 );

	char szMovieName[64];
	FillStringFromCell(szMovieName, 63, pAMX, aParams[1]);

	cutscene_Handle_t hMovie = cutscene_AcquireHandle( szMovieName, TRUE );
	if( hMovie == CUTSCENE_INVALID_HANDLE ) {
		SCRIPT_ERROR("Movie_GetHandle() : Invalid Handle returned." );
	}
	return( (cell) hMovie );
}


//plays a movie from a handle aquired by Movie_GetHandle()
cell AMX_NATIVE_CALL CMAST_Movie::Movie_Play(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Movie_Play", 1 );

	cutscene_Handle_t hMovie = aParams[1];
	if( hMovie == CUTSCENE_INVALID_HANDLE ) {
		SCRIPT_ERROR("Movie_Play() : Invalid Handle provided" );
		return 0;
	}
	cutscene_Start( hMovie );

	return( (cell) 0 );
}


//plays a movie from a handle aquired by Movie_GetHandle()
cell AMX_NATIVE_CALL CMAST_Movie::Movie_PlayEx(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Movie_Play", 2 );

	cutscene_Handle_t hMovie = aParams[1];
	s32 nVolumeLevel = aParams[2];

	if( hMovie == CUTSCENE_INVALID_HANDLE ) {
		SCRIPT_ERROR("Movie_Play() : Invalid Handle provided" );
		return 0;
	}

	f32 fNormalizedVolume = ( f32 ) nVolumeLevel * 0.01f;
	cutscene_Start( hMovie, fNormalizedVolume );

	return( (cell) 0 );
}


//stops a movie thats currently being played
cell AMX_NATIVE_CALL CMAST_Movie::Movie_Stop(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Movie_Stop", 0 );

	cutscene_End();

	return( (cell) 0 );
}

/////////////////////////////////////////////////////////////////
// Particle functions

cell AMX_NATIVE_CALL CMAST_Particle::Particle_Find(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Particle_Find", 1 );
	char szEntityName[100];
	FillStringFromCell(szEntityName, 99, pAMX, aParams[1]);
	CEntity *pEntity = CEntity::Find(szEntityName);

	if(pEntity == NULL)
	{
		SCRIPT_ERROR("Particle_Find() : Could not find object named '%s'.", szEntityName);
		return((cell)(NULL));
	}

	if(!(pEntity->TypeBits() & ENTITY_BIT_PARTICLE))
	{
		SCRIPT_ERROR("Particle_Find() : Object named '%s' is not a particle emitter.", szEntityName);
		return((cell)(NULL));
	}

	return((cell)(pEntity));
}

cell AMX_NATIVE_CALL CMAST_Particle::Particle_Start(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Particle_Start", 1 );
	CEntity *pEntity = (CEntity *)(aParams[1]);
	if(pEntity == NULL)
	{
		SCRIPT_ERROR("Particle_Start() : Entity handle null." );
		return( (cell) 0 );
	}

	if(!(pEntity->TypeBits() & ENTITY_BIT_PARTICLE))
	{
		SCRIPT_ERROR("Particle_Start() : Object is not a particle emitter.");
		return( (cell) 0 );
	}

	CEParticle *pParticle = (CEParticle *) pEntity;

	pParticle->EnableEmission( TRUE );

	return( (cell) 1 );
}

cell AMX_NATIVE_CALL CMAST_Particle::Particle_Stop(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Particle_Stop", 1 );
	CEntity *pEntity = (CEntity *)(aParams[1]);
	if(pEntity == NULL)
	{
		SCRIPT_ERROR("Particle_Stop() : Entity handle null." );
		return( (cell) 0 );
	}

	if(!(pEntity->TypeBits() & ENTITY_BIT_PARTICLE))
	{
		SCRIPT_ERROR("Particle_Stop() : Object is not a particle emitter.");
		return( (cell) 0 );
	}

	CEParticle *pParticle = (CEParticle *) pEntity;

	pParticle->EnableEmission( FALSE );

	return( (cell) 1 );
}

/////////////////////////////////////////////////////////////////
// Checkpoint functions

cell AMX_NATIVE_CALL CMAST_Checkpoint::Checkpoint_Save(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Checkpoint_Save", 0 );
	BOOL bResult;
	bResult = checkpoint_Save( 1, TRUE );
	return( (cell) bResult );
}


/////////////////////////////////////////////////////////////////
// destruct entity functions

cell AMX_NATIVE_CALL CMAST_GameWrapper::Game_BeginCutScene(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Game_BeginCutScene", 1 );
	char szTitle[100] = "";
	if (aParams[1])
	{
		FillStringFromCell(szTitle, 99, pAMX, aParams[1]);
		game_BeginCutScene(szTitle);
	}
	else
	{
		game_BeginCutScene();
	}
	
	return((cell)(0));
}


cell AMX_NATIVE_CALL CMAST_GameWrapper::Game_EndCutScene(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Game_EndCutScene", 0 );

	game_EndCutScene();

	return((cell)(0));
}

// exits current level and loads the level named in the first parameter.
cell AMX_NATIVE_CALL CMAST_GameWrapper::Game_WinLevel(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Game_WinLevel", 1 );
	char szCommand[100] = "";
	if (aParams[1])
	{
		FillStringFromCell(szCommand, 99, pAMX, aParams[1]);
	}

	game_GotoLevel( szCommand );

	return((cell)(0));
}


// Enters pause mode -- USED ONLY FOR THE CREATION OF CUTSCENES!
cell AMX_NATIVE_CALL CMAST_GameWrapper::Game_DoPauseMode(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Game_DoPauseMode", 0 );
	Game_bEnterPauseModeImmediatelyForCutsceneCreation = TRUE;

	return((cell)(0));
}


//----------------------------------------------------------------------------------------------------------------------------
// PhysWrapper
//----------------------------------------------------------------------------------------------------------------------------


// Attempts to locate the specified tack name on the specified entity.
// If found, returns a tack handle. If not, returns NULL.
//
// Tack_Find entity_handle, sName
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_Find(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_Find", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Tack_Find() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Tack_Find() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	char szTackName[100];
	FillStringFromCell(szTackName, 99, pAMX, aParams[2]);
	CFVerletTack *pTack = pE->GetVerlet()->Tack_FindByName(szTackName);
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_Find() : Tack '%s' not in entity '%s'.\n", szTackName, pE->Name());
	}

	return(cell)pTack;
}


// Returns the health of the tack specified by the given handle.
// Health typically ranges from 0 to 1, but can go beyond if desired.
//
// Tack_GetHealth tack_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_GetHealth(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_GetHealth", 1 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_GetHealth() : Passed in NULL tack.");
		return(cell)1;
	}

	return(ConvertF32ToCell(pTack->Anchor_GetUnitHealth()));
}


// Sets the health of the tack specified by the given handle.
// Health typically ranges from 0 to 1, but can go beyond if desired.
//
// Tack_SetHealth tack_handle health
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_SetHealth(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_SetHealth", 2 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_SetHealth() : Passed in NULL tack.");
		return(cell)0;
	}

	pTack->Anchor_SetUnitHealth( ConvertCellToF32(aParams[2]) );

	return((cell)(0));
}


// Sets the armor profile of the tack specified by the given handle.
//
// Tack_SetArmorProf tack_handle, sArmorProfileName
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_SetArmorProf(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_SetArmorProf", 2 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_SetArmorProf() : Passed in NULL tack.");
		return(cell)0;
	}

	char szArmorProfileName[100];
	FillStringFromCell(szArmorProfileName, 99, pAMX, aParams[2]);
	const CArmorProfile *pArmorProfile = CDamage::FindArmorProfile( szArmorProfileName, FALSE, FALSE );
	if( pArmorProfile == NULL ) {
		SCRIPT_ERROR("Tack_SetArmorProf() : Armor profile '%s' not found.", szArmorProfileName);
		return(cell)0;
	}

	pTack->m_pUser3 = (void *)pArmorProfile;

	return((cell)(0));
}


// Enables/disables the anchoring of this tack to something.
// 1 to enable, 0 to disable.
//
// Tack_Enable tack_handle, bool
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_Enable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_Enable", 2 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_Enable() : Passed in NULL tack.");
		return(cell)0;
	}

	pTack->Anchor_Enable( (BOOL)aParams[2] );

	return((cell)(0));
}


// Removes a tack's anchor.
//
// Tack_Remove tack_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_Remove(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_Remove", 1 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_Remove() : Passed in NULL tack.");
		return(cell)0;
	}

	pTack->Anchor_Remove();

	return((cell)(0));
}


// Anchors the tack to the current position of the specified entity.
// If the entity moves, the anchor point will not.
// The entity does not need to be a physics object.
//
// Tack_ToEntityStat tack_handle, entity_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_ToEntityStat(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_ToEntityStat", 2 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_ToEntityStat() : Passed in NULL tack.");
		return(cell)0;
	}

	CEntity *pE = (CEntity *)(aParams[2]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Tack_ToEntityStat() : Passed in NULL entity.");
		return(cell)0;
	}

	pTack->Anchor_SetToImmovablePoint_Static( &pE->MtxToWorld()->m_vPos );

	return((cell)(0));
}


// Anchors the tack to the dynamic position of the specified entity.
// If the entity moves, the anchor point will also.
// The entity does not need to be a physics object.
//
// Tack_ToEntityDyn tack_handle, entity_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_ToEntityDyn(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_ToEntityDyn", 2 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_ToEntityDyn() : Passed in NULL tack.");
		return(cell)0;
	}

	CEntity *pE = (CEntity *)(aParams[2]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Tack_ToEntityDyn() : Passed in NULL entity.");
		return(cell)0;
	}

	pTack->Anchor_SetToImmovablePoint_Dynamic( &pE->MtxToWorld()->m_vPos );

	return((cell)(0));
}


// Anchors the tack to the XYZ world point.
//
// Tack_ToPos tack_handle, x, y, z
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_ToPos(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_ToPos", 4 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_ToPos() : Passed in NULL tack.");
		return(cell)0;
	}

	CFVec3A Pos_WS;

	Pos_WS.x = ConvertCellToF32(aParams[2]);
	Pos_WS.y = ConvertCellToF32(aParams[3]);
	Pos_WS.z = ConvertCellToF32(aParams[4]);

	pTack->Anchor_SetToImmovablePoint_Static( &Pos_WS );

	return((cell)(0));
}


// Sets the min/max range of the tack's anchor.
//
// Tack_SetRange tack_handle, min, max
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_SetRange(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_SetRange", 3 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_SetRange() : Passed in NULL tack.");
		return(cell)0;
	}

	f32 fRangeMin = ConvertCellToF32(aParams[2]);
	f32 fRangeMax = ConvertCellToF32(aParams[3]);

	pTack->Anchor_SetRange( fRangeMin, fRangeMax );

	return((cell)(0));
}


// Returns 1 if the tack is anchored, or 0 if it's not.
//
// Tack_IsEnabled tack_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_IsEnabled(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_IsEnabled", 1 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_IsEnabled() : Passed in NULL tack.");
		return(cell)0;
	}

	return(cell)(u32)pTack->Anchor_IsEnabled();
}


// Returns the current minimum anchor range of the tack.
//
// Tack_GetRangeMin tack_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_GetRangeMin(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_GetRangeMin", 1 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_GetRangeMin() : Passed in NULL tack.");
		return(cell)0;
	}

	return(cell)(u32)pTack->Anchor_GetMinConstraintDist();
}


// Returns the current maximum anchor range of the tack.
//
// Tack_GetRangeMax tack_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_GetRangeMax(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_GetRangeMax", 1 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_GetRangeMax() : Passed in NULL tack.");
		return(cell)0;
	}

	return(cell)(u32)pTack->Anchor_GetMaxConstraintDist();
}


// Returns the tack that is anchored to the specified tack.
// Will return NULL if nothing is anchored to the specified tack.
//
// Tack_GetSnapped2Me tack_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_GetSnapped2Me(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_GetSnapped2Me", 1 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_GetSnapped2Me() : Passed in NULL tack.");
		return(cell)0;
	}

	return(cell)(u32)pTack->Anchor_GetTackAnchoredToMe();
}


// Returns the entity that this tack belongs to.
//
// Tack_GetPhysEntity tack_handle
cell AMX_NATIVE_CALL CMAST_TackWrapper::Tack_GetPhysEntity(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Tack_GetPhysEntity", 1 );
	CFVerletTack *pTack = (CFVerletTack *)aParams[1];
	if(pTack == NULL)
	{
		SCRIPT_ERROR("Tack_GetPhysEntity() : Passed in NULL tack.");
		return(cell)0;
	}

	return(cell)pTack->GetTackOwner()->m_pUser;
}




//----------------------------------------------------------------------------------------------------------------------------
// PhysWrapper
//----------------------------------------------------------------------------------------------------------------------------


// Resets the specified physics object to a motionless state.
// Also updates position and orientation of physics object based on position of entity
// Phys_Reset entity_handle bool
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_Reset(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_Reset", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_Reset() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_Reset() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	pE->GetVerlet()->InitUnitMtx( pE->MtxToWorld(), (BOOL)aParams[2] );

	return((cell)(0));
}


// Freezes/unfreezes the specified physics object.
//
// Phys_Freeze entity_handle bool
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_Freeze(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_Freeze", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_Freeze() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_Freeze() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	pE->GetVerlet()->Net_Freeze( (BOOL)aParams[2] );

	return((cell)(0));
}


// If the physics object is nearby, this will allow it to start moving.
// If the object won't move for a second or two, it will be put back
// to sleep.
//
// Phys_Kickstart entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_Kickstart(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_Kickstart", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_Kickstart() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_Kickstart() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	pE->GetVerlet()->Net_Kickstart();

	return((cell)(0));
}


// Puts the object to sleep. The object will be woken up by anything
// that might make it move (explosion nearby, etc.).
//
// Phys_Sleep entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_Sleep(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_Sleep", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_Sleep() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_Sleep() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	pE->GetVerlet()->Net_MoveToInactiveList();

	return((cell)(0));
}


// Sets the dampening factor to help calm the object.
// 0 is no dampening. Typical values are very small, like
// 0.02 or 0.03.
//
// Phys_SetDampening entity_handle dampen
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_SetDampening(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_SetDampening", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_SetDampening() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_SetDampening() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	pE->GetVerlet()->SetDampening( ConvertCellToF32(aParams[2]) );

	return((cell)(0));
}


// Sets the amount of time to run the physics simulation before the
// game starts. This is so that when the player first encounters
// the physics object, it will be close to its rest position.
//
// The larger the values, the longer it will take the level to load.
// For most physics object, you can leave this 0. For some, try
// a value of 1 second. If it's a bunch of objects strung together,
// you may need a larger value like 5 seconds, but try to avoid this.
//
// Phys_SetPrimeSecs entity_handle seconds
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_SetPrimeSecs(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_SetPrimeSecs", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_SetPrimeSecs() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_SetPrimeSecs() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	pE->GetVerlet()->SetPrimeSecs( ConvertCellToF32(aParams[2]) );

	return((cell)(0));
}


// Sets the gravity. The default is -64. Large values might
// make the physics go unstable.
//
// You can set this to a + value for interesting effects like
// something being blown upward by a steam vent.
//
// Phys_SetGravity entity_handle gravity
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_SetGravity(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_SetGravity", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_SetGravity() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_SetGravity() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	pE->GetVerlet()->SetGravity( ConvertCellToF32(aParams[2]) );

	return((cell)(0));
}


// Returns 1 if the physics entity is frozen.
// Otherwise, returns 0.
//
// Phys_IsFrozen entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_IsFrozen(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_IsFrozen", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_IsFrozen() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_IsFrozen() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	return(cell)!!pE->GetVerlet()->Net_IsFrozen();
}


// Returns 1 if the physics entity is sleeping.
// Otherwise, returns 0.
//
// Phys_IsSleeping entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_IsSleeping(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_IsSleeping", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_IsSleeping() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_IsSleeping() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	return(cell)(pE->GetVerlet()->GetListMember() == CFVerlet::NET_LIST_MEMBER_INACTIVE);
}


// Returns the current dampening value.
//
// Phys_GetDampening entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_GetDampening(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_GetDampening", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_GetDampening() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_GetDampening() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	return(ConvertF32ToCell(pE->GetVerlet()->GetDampening()));
}


// Returns the current prime seconds value.
//
// Phys_GetPrimeSecs entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_GetPrimeSecs(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_GetPrimeSecs", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_GetPrimeSecs() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_GetPrimeSecs() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	return(ConvertF32ToCell(pE->GetVerlet()->GetPrimeSecs()));
}


// Returns the current gravity value.
//
// Phys_GetDampening entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_GetGravity(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_GetGravity", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_GetGravity() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_GetGravity() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	return(ConvertF32ToCell(pE->GetVerlet()->GetGravity()));
}


// Returns the number of tacks on the specified physics object.
//
// Phys_GetTackCount entity_handle
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_GetTackCount(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_GetTackCount", 1 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_GetTackCount() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_GetTackCount() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	return(cell)pE->GetVerlet()->Tack_GetCount();
}


// Returns the tack handle of the specified tack index.
//
// Phys_GetTack entity_handle tack_index
cell AMX_NATIVE_CALL CMAST_PhysWrapper::Phys_GetTack(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Phys_GetTack", 2 );
	CEntity *pE = (CEntity *)(aParams[1]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("Phys_GetTack() : Passed in NULL entity.");
		return(cell)0;
	}

	if(!pE->GetVerlet())
	{
		SCRIPT_ERROR("Phys_GetTack() : Entity '%s' is not a physics object.", pE->Name());
		return(cell)0;
	}

	return(cell)pE->GetVerlet()->Tack_GetFromIndex( (u32)aParams[2] );
}


//----------------------------------------------------------------------------------------------------------------------------
// Spline Wrappers
//----------------------------------------------------------------------------------------------------------------------------

cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SActor_Init(AMX *pAMX, cell *aParams)
{

	SCRIPT_CHECK_NUM_PARAMS( "SActor_Init", 5);

	//FIRST, VALIDATE ALL THE PARAMETERS
	CSplineActor *pSplineActor = ( CSplineActor* ) aParams[1];
	if( !pSplineActor )
	{
		SCRIPT_ERROR("SActor_Init -- Passed in NULL SplineActor Handle");
		return(cell)0;
	}

	char szSplineName[20];
	FillStringFromCell(szSplineName, 19, pAMX, aParams[2]);

	f32 fTotalTime = ConvertCellToF32(aParams[3]);
	if( fTotalTime <= 0.0f )
	{
		SCRIPT_ERROR("SActor_Init() : time must be greater than zero.");
		return((cell)(0));
	}

	CMeshEntity *pME = (CMeshEntity *)(aParams[4]);
	if(pME == NULL)
	{
		SCRIPT_ERROR("SActor_Init(): Passed in a NULL meshentity.");
		return((cell)(0));
	}
	BOOL bAutoStart = aParams[5];

	//WE HAVE VALID PARAMS, INITIALIZE SPLINE
	if( !pSplineActor->Init(szSplineName, fTotalTime, pME, bAutoStart) ) {
		SCRIPT_ERROR("SActor_Init(): CSplineActor initialization failure!");
		return((cell)(0));
	}

	return (cell) 0;
}


cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SActor_SetTimePos(AMX *pAMX, cell *aParams)
{

	SCRIPT_CHECK_NUM_PARAMS( "SActor_SetTimePos", 2);

	//FIRST, VALIDATE ALL THE PARAMETERS
	CSplineActor *pSplineActor = ( CSplineActor* ) aParams[1];
	if( !pSplineActor )
	{
		SCRIPT_ERROR("SActor_SetTimePos -- Passed in NULL SplineActor Handle");
		return(cell)0;
	}

	f32 fTimePos = ConvertCellToF32(aParams[2]);
	if( fTimePos <= 0.0f )
	{
		SCRIPT_ERROR("HSActor_SetTimePos() : time must be greater than zero.");
		return((cell)(0));
	}
	pSplineActor->SetTimePos( fTimePos );

	return (cell) 0;
}


cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SActor_GetTimePos(AMX *pAMX, cell *aParams)
{

	SCRIPT_CHECK_NUM_PARAMS( "SActor_GetTimePos", 1);

	//FIRST, VALIDATE ALL THE PARAMETERS
	CSplineActor *pSplineActor = ( CSplineActor* ) aParams[1];
	if( !pSplineActor )
	{
		SCRIPT_ERROR("SActor_GetTimePos -- Passed in NULL SplineActor Handle");
		return(cell)0;
	}

	return ConvertF32ToCell( pSplineActor->GetTimePos() );
}


cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SActor_Reset(AMX *pAMX, cell *aParams)
{

	SCRIPT_CHECK_NUM_PARAMS( "HSActor_Reset", 2);

	//FIRST, VALIDATE ALL THE PARAMETERS
	CSplineActor *pSplineActor = ( CSplineActor* ) aParams[1];
	if( !pSplineActor )
	{
		SCRIPT_ERROR("SActor_Reset -- Passed in NULL SplineActor Handle");
		return(cell)0;
	}

	BOOL bAutoStart = aParams[2];

	pSplineActor->Reset( bAutoStart );

	return  (cell) 0;
}



cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SActor_Start(AMX *pAMX, cell *aParams)
{

	SCRIPT_CHECK_NUM_PARAMS( "HSActor_Start", 1);

	//FIRST, VALIDATE ALL THE PARAMETERS
	CSplineActor *pSplineActor = ( CSplineActor* ) aParams[1];
	if( !pSplineActor )
	{
		SCRIPT_ERROR("SActor_Start -- Passed in NULL SplineActor Handle");
		return(cell)0;
	}

	pSplineActor->Start();

	return  (cell) 0;
}



cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SActor_Stop(AMX *pAMX, cell *aParams)
{

	SCRIPT_CHECK_NUM_PARAMS( "HSActor_Stop", 1);

	//FIRST, VALIDATE ALL THE PARAMETERS
	CSplineActor *pSplineActor = ( CSplineActor* ) aParams[1];
	if( !pSplineActor )
	{
		SCRIPT_ERROR("SActor_Stop -- Passed in NULL SplineActor Handle");
		return(cell)0;
	}

	pSplineActor->Stop();

	return  (cell) 0;
}



cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SActor_IsDone(AMX *pAMX, cell *aParams)
{

	SCRIPT_CHECK_NUM_PARAMS( "HSActor_Start", 1);

	//FIRST, VALIDATE ALL THE PARAMETERS
	CSplineActor *pSplineActor = ( CSplineActor* ) aParams[1];
	if( !pSplineActor )
	{
		SCRIPT_ERROR("SActor_IsDone -- Passed in NULL SplineActor Handle");
		return(cell)0;
	}

	//now, return if it's done or not....
	return (cell) pSplineActor->IsDone();
}


cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::GSActor_Acquire(AMX *pAMX, cell *aParams)
{
	CGeneralSplineActor* pGSA = CGeneralSplineActor::Acquire();
	if( !pGSA ) {
		SCRIPT_ERROR("GSActor_Acquire : Could not Create an GSActor Object.");
	}

	return (cell) pGSA;

}


cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::HSActor_Acquire(AMX *pAMX, cell *aParams)
{
	CHorizontalSplineActor* pHSA = CHorizontalSplineActor::Acquire();
	if( !pHSA ) {
		SCRIPT_ERROR("HSActor_Acquire : Could not Create an HSActor Object.");
	}

	return (cell) pHSA;

}


cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SSActor_Acquire(AMX *pAMX, cell *aParams)
{
	CSwingingSplineActor* pSSA = CSwingingSplineActor::Acquire();
	if( !pSSA ) {
		SCRIPT_ERROR("SSActor_Acquire : Could not Create an SSActor Object.");
	}

	return (cell) pSSA;

}


cell AMX_NATIVE_CALL CMAST_SplineActorWrapper::SOSActor_Acquire(AMX *pAMX, cell *aParams)
{
	CStaticOrientationSplineActor* pSOSA = CStaticOrientationSplineActor::Acquire();
	if( !pSOSA ) {
		SCRIPT_ERROR("SOSActor_Acquire : Could not Create an SOSActor Object.");
	}

	return (cell) pSOSA;

}


//----------------------------------------------------------------------------------------------------------------------------
// Barter Wrappers
//----------------------------------------------------------------------------------------------------------------------------


cell AMX_NATIVE_CALL CMAST_BarterWrapper::Barter_MoveToPoint(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Barter_MoveToPoint", 1 );
	char szPointName[100];
	FillStringFromCell(szPointName, 99, pAMX, aParams[1]);

	BOOL bReturn = bartersystem_PlaceInWorldAndStartAttracting( szPointName );
	if( !bReturn ) {
		SCRIPT_MESSAGE( "Barter_MoveToPoint() : Could not move to point %s", szPointName );
	}

	return bReturn;
}


// Barter_Remove( BOOL bFadeMusic )
cell AMX_NATIVE_CALL CMAST_BarterWrapper::Barter_Remove(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Barter_Remove", 1 );

	bartersystem_RemoveFromWorld( (BOOL)aParams[1] );

	return TRUE;
}

// Barter_SetAttractRadius(u32 uWhichTable, f32 fAttractMusicRadius, f32 fAttractSpeechRadius)
cell AMX_NATIVE_CALL CMAST_BarterWrapper::Barter_SetAttract(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Barter_SetAttract", 3 );
	s32 uTable = (s32)aParams[1];
	uTable--;
	FMATH_CLAMPMIN(uTable,0);
	BOOL bOK = bartersystem_SetAttractRadius((u32)uTable, ConvertCellToF32( aParams[2] ), ConvertCellToF32( aParams[3] ));

	if (bOK)
        return TRUE;
	else
	{
		SCRIPT_ERROR( "Barter_SetAttract : Bad table number passed" );
		return FALSE;
	}
}

//----------------------------------------------------------------------------------------------------------------------------
// Vehicle Wrappers
//----------------------------------------------------------------------------------------------------------------------------


// Assigns a spline path to a vehicle for use by AI drivers.
// First parameter is handle to vehicle.
// Second parameter is handle to spline.
// Returns true on success, false otherwise.
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_SetPath(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_SetPath", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];
	CESpline *pSpline = (CESpline *) aParams[2];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_SetPath : Vehicle handle was NULL" );
		return FALSE;
	}

	if( pSpline == NULL )
	{
		SCRIPT_ERROR( "Vehicle_SetPath : Path handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_SetPath : vehicle handle is not a vehicle" );
		return FALSE;
	}

	if( !(pSpline->TypeBits() & ENTITY_BIT_SPLINE) )
	{
		SCRIPT_ERROR( "Vehicle_SetPath : Path handle is not a path" );
		return FALSE;
	}

	pVehicle->SetSplinePath( pSpline );

	return TRUE;
}

// clears a vehicle's path.  Vehicle will stop if it is currently travelling on a path.
// First parameter is handle to vehicle.
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_ClearPath(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_ClearPath", 1 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_SetPath : Vehicle handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_SetPath : vehicle handle is not a vehicle" );
		return FALSE;
	}

	pVehicle->SetSplinePath( NULL );

	return TRUE;
}

// puts player into vehicle for the duration of a level (i.e. racing level)
// First parameter is handle to vehicle.
// Second parameter is handle to player (from Bot_GetPlayer()).
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_SetPlayer(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_SetPlayer", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];
	CBot *pBot = (CBot*) aParams[2];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_SetPlayer : Vehicle handle was NULL" );
		return FALSE;
	}

	if( pBot == NULL )
	{
		SCRIPT_ERROR( "Vehicle_SetPlayer : Player handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_SetPlayer : vehicle handle is not a vehicle" );
		return FALSE;
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR( "Vehicle_SetPlayer : Player handle is not a bot" );
		return FALSE;
	}
	if( pBot->m_nPossessionPlayerIndex < 0 || pBot->m_nPossessionPlayerIndex >= MAX_PLAYERS )
	{
		SCRIPT_ERROR( "Vehicle_SetPlayer : Player handle is not a player" );
		return FALSE;
	}

	CPlayer *pPlayer = &Player_aPlayer[ pBot->m_nPossessionPlayerIndex ];
	pPlayer->SetInVehicle( pVehicle, CVehicle::STATION_DRIVER );
	return TRUE;
}

// puts player into gunner position for the duration of a level (i.e. racing level)
// First parameter is handle to vehicle.
// Second parameter is handle to player (from Bot_GetPlayer()).
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_PlayerGun(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_PlayerGun", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];
	CBot *pBot = (CBot*) aParams[2];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_PlayerGun : Vehicle handle was NULL" );
		return FALSE;
	}

	if( pBot == NULL )
	{
		SCRIPT_ERROR( "Vehicle_PlayerGun : Player handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_PlayerGun : vehicle handle is not a vehicle" );
		return FALSE;
	}

	if( !(pBot->TypeBits() & ENTITY_BIT_BOT) )
	{
		SCRIPT_ERROR( "Vehicle_PlayerGun : Player handle is not a bot" );
		return FALSE;
	}
	if( pBot->m_nPossessionPlayerIndex < 0 || pBot->m_nPossessionPlayerIndex >= MAX_PLAYERS )
	{
		SCRIPT_ERROR( "Vehicle_PlayerGun : Player handle is not a player" );
		return FALSE;
	}

	CPlayer *pPlayer = &Player_aPlayer[ pBot->m_nPossessionPlayerIndex ];
	pPlayer->SetInVehicle( pVehicle, CVehicle::STATION_GUNNER );
	return TRUE;
}


// Sets vehicle as player-driveable, or not-player-driveable.  AI driving is not affected.
// First parameter is handle to vehicle.
// Second parameter is TRUE for driveable, FALSE for not-driveable.
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_PlayerDrive(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_PlayerDrive", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];
	BOOL bDriveable = (BOOL) !!aParams[2];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_PlayerDrive : Vehicle handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_PlayerDrive : vehicle handle is not a vehicle" );
		return FALSE;
	}

	pVehicle->SetPlayerDriveable( bDriveable );
	return TRUE;
}

// Sets vehicle as driveable or not driveable by any bot, player or AI controlled.
// First parameter is handle to vehicle.
// Second parameter is TRUE for driveable, FALSE for not-driveable.
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_Driveable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_Driveable", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];
	BOOL bDriveable = (BOOL) !!aParams[2];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_Driveable : Vehicle handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_Driveable : vehicle handle is not a vehicle" );
		return FALSE;
	}

	pVehicle->SetDriveable( bDriveable );
	return TRUE;
}

// Sets the maximum speed of the vehicle as a percentage of its normal maximum speed.
// First parameter is handle to vehicle.
// Second parameter is speed percentage, from 1 to 100 (ordinary number, not floating point)
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_MaxSpeed(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_MaxSpeed", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];
	s32 nSpeed = (s32) aParams[2];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_MaxSpeed : Vehicle handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_MaxSpeed : vehicle handle is not a vehicle" );
		return FALSE;
	}

	if( nSpeed < 0 || nSpeed > 100 )
	{
		SCRIPT_ERROR( "Vehicle_MaxSpeed : speed range is 0 to 100" );
	}
	FMATH_CLAMP( nSpeed, 0, 100 );

	f32 fSpeed;
	fSpeed = (f32)nSpeed * 0.01f;
	pVehicle->SetMaxUnitSpeed( fSpeed, FALSE /*bInternalOnly*/ );
	return TRUE;
}

// causes the vehicle's driver to jump out.
// first parameter is handle to vehicle.
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_EjectDriver(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_EjectDriver", 1 );
	CVehicle::StationStatus_e eStatus;

	CVehicle *pVehicle = (CVehicle *) aParams[1];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_EjectDriver : Vehicle handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_EjectDriver : vehicle handle is not a vehicle" );
		return FALSE;
	}

	if( pVehicle->GetDriverBot() == NULL )
	{
		SCRIPT_MESSAGE( "Vehicle_EjectDriver : vehicle has no driver" );
		return FALSE;
	}

	eStatus = pVehicle->ExitStation( pVehicle->GetDriverBot(), FALSE );

	if( eStatus == CVehicle::STATION_STATUS_EXITING )
	{
		return TRUE;
	}

	SCRIPT_ERROR( "Vehicle_EjectDriver : ejection failed! Reason code %d", eStatus );
	return FALSE;
}

// causes the vehicle's Gunner to jump out.
// first parameter is handle to vehicle.
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_EjectGunner(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_EjectGunner", 1 );
	CVehicle::StationStatus_e eStatus;

	CVehicle *pVehicle = (CVehicle *) aParams[1];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_EjectGunner : Vehicle handle was NULL" );
		return FALSE;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_EjectGunner : vehicle handle is not a vehicle" );
		return FALSE;
	}

	if( pVehicle->GetGunnerBot() == NULL )
	{
		SCRIPT_MESSAGE( "Vehicle_EjectGunner : vehicle has no gunner" );
		return FALSE;
	}

	eStatus = pVehicle->ExitStation( pVehicle->GetGunnerBot(), FALSE );
	if( eStatus == CVehicle::STATION_STATUS_EXITING )
	{
		return TRUE;
	}

	SCRIPT_ERROR( "Vehicle_EjectGunner : ejection failed! Reason code %d", eStatus );
	return FALSE;
}

// returns the speed of the vehicle as a percentage of its maximum speed.
// return value range is 0 to 100 (integer, not floating point)
// first parameter is handle to vehicle.
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_GetSpeed(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_GetSpeed", 1 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_GetSpeed : Vehicle handle was NULL" );
		return 0;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_GetSpeed : vehicle handle is not a vehicle" );
		return 0;
	}

	s32 nSpeed;
	nSpeed = (s32) (pVehicle->GetUnitSpeed() * 100.0f);
	FMATH_CLAMP( nSpeed, 0, 100 );

	return nSpeed;
}

// sets a vehicle's driving station as being exitable
// first parameter is handle to vehicle
// second parameter is "true" for exitable, "false" for not-exitable
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_DriverExit(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_DriverExit", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_DriverExit : Vehicle handle was NULL" );
		return 0;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_DriverExit : vehicle handle is not a vehicle" );
		return 0;
	}

	pVehicle->SetDriverExitable( !!aParams[1] );
	return 1;
}

// sets a vehicle's gunning station as being exitable
// first parameter is handle to vehicle
// second parameter is "true" for exitable, "false" for not-exitable
cell AMX_NATIVE_CALL CMAST_VehicleWrapper::Vehicle_GunnerExit(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Vehicle_GunnerExit", 2 );

	CVehicle *pVehicle = (CVehicle *) aParams[1];

	if( pVehicle == NULL )
	{
		SCRIPT_ERROR( "Vehicle_GunnerExit : Vehicle handle was NULL" );
		return 0;
	}

	if( !(pVehicle->TypeBits() & ENTITY_BIT_VEHICLE) )
	{
		SCRIPT_ERROR( "Vehicle_GunnerExit : vehicle handle is not a vehicle" );
		return 0;
	}

	pVehicle->SetGunnerExitable( !!aParams[1] );
	return 1;
}


//----------------------------------------------------------------------------------------------------------------------------
// FX Wrappers
//----------------------------------------------------------------------------------------------------------------------------

// Attempts to locate the specified explosion.
// If found, returns an explosion handle. If not, returns NULL.
//
// Usage: FX_Explosion_Find sExplosionName
//
// sExplosionName is the name of the explosion group in the explosion group CSV.
cell AMX_NATIVE_CALL CMAST_FXWrapper::FX_Explosion_Find(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FX_Explosion_Find", 1 );
	char szExplosionGroupName[100];
	FillStringFromCell(szExplosionGroupName, 99, pAMX, aParams[1]);
	FExplosion_GroupHandle_t hExplosionGroup = CExplosion2::GetExplosionGroup(szExplosionGroupName);
	if(hExplosionGroup == FEXPLOSION_INVALID_HANDLE)
	{
		SCRIPT_ERROR("FX_Explosion_Find() : Could not find entity named '%s'.", szExplosionGroupName);
	}
	return((cell)(hExplosionGroup));
}


// Generates an explosion at the position of the given entity.
//
// Usage: FX_Explosion ExplosionHandle EntityHandle
//
// ExplosionHandle is handle of the explosion returned by FX_Explosion_Find.
// EntityHandle is the handle of the entity returned by E_Find.
cell AMX_NATIVE_CALL CMAST_FXWrapper::FX_Explosion(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FX_Explosion", 2 );

	FExplosion_GroupHandle_t hExplosionGroup = (FExplosion_GroupHandle_t)(aParams[1]);
	if(hExplosionGroup == FEXPLOSION_INVALID_HANDLE)
	{
		SCRIPT_ERROR("FX_Explosion() : Passed in null explosion handle.");
		return((cell)(0));
	}

	CEntity *pE = (CEntity *)(aParams[2]);
	if(pE == NULL)
	{
		SCRIPT_ERROR("FX_Explosion() : Passed in null entity.");
		return((cell)(0));
	}

	FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

	if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
		FExplosionSpawnParams_t SpawnParams;

		SpawnParams.InitToDefaults();

		SpawnParams.Pos_WS = pE->MtxToWorld()->m_vPos;
		SpawnParams.UnitDir.Zero();
		SpawnParams.uSurfaceType = 0;

		CExplosion2::SpawnExplosion( hSpawner, hExplosionGroup, &SpawnParams );
	}

	return((cell)(0));
}

//----------------------------------------------------------------------------------------------------------------------------
// Goodie Wrappers
//----------------------------------------------------------------------------------------------------------------------------

// Lets the goodie system know that the passed goodie name may be used in a script.
//
// Usage: G_NotifyGoodieUsed sGoodieName
//
// sGoodie name is the name of the goodie from CSV/GOODIES/goodies.csv.
cell AMX_NATIVE_CALL CMAST_GoodieWrapper::G_NotifyGoodieUsed(AMX *pAMX, cell *aParams) 
{
	SCRIPT_CHECK_NUM_PARAMS( "G_NotifyGoodieUsed", 1 );
	char szGoodieName[100];

	FillStringFromCell(szGoodieName, 99, pAMX, aParams[1]);

	CCollectable::NotifyCollectableUsedInWorld( szGoodieName );

	return((cell)1);
}

// Attempts to give the passed goodie name to Glitch.  
// If the goodie is given to Glitch, returns 1.  If not, returns NULL.
//
// Usage: G_GiveToGlitch sGoodieName
//
// sGoodie name is the name of the goodie from CSV/GOODIES/goodies.csv.
cell AMX_NATIVE_CALL CMAST_GoodieWrapper::G_GiveToGlitch(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "G_GiveToGlitch", 1 );
	char szGoodieName[100];
	CEntity *pGlitch = NULL;

	pGlitch = Player_aPlayer[0].m_pEntityOrig;

	if( pGlitch == NULL ) {
		SCRIPT_ERROR("G_GiveToGlitch() : Not able to get current entity.");
		return ((cell) NULL);
	}

	if( !( pGlitch->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
		SCRIPT_ERROR("G_GiveToGlitch() : Current bot not Glitch.");
		return ((cell) NULL);
	}

	FillStringFromCell(szGoodieName, 99, pAMX, aParams[1]);

	if( !CCollectable::GiveToPlayer( (CBotGlitch *) pGlitch, szGoodieName, 1.0f ) ) {
		SCRIPT_ERROR("G_GiveToGlitch() : Unable to give goodie '%s' to player.", szGoodieName);
		return ((cell) NULL);
	}

	return((cell)1);
}

// Returns TRUE if Glitch has the passed item in his inventory or FALSE otherwise
//
// Usage: G_IsInInventory sGoodieName
//
// sGoodie name is the name of an item from the itemlist.csv.
cell AMX_NATIVE_CALL CMAST_GoodieWrapper::G_IsInInventory(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "G_IsInInventory", 1 );
	char szGoodieName[100];

	if( Player_aPlayer[0].m_pEntityOrig == NULL ) {
		SCRIPT_ERROR("G_IsInInventory() : Not able to get current entity.");
		return ((cell) FALSE);
	}

	if( !( Player_aPlayer[0].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
		SCRIPT_ERROR("G_IsInInventory() : Current bot not Glitch.");
		return ((cell) FALSE);
	}

	CBot *pGlitch = (CBot *) Player_aPlayer[0].m_pEntityOrig;

	if( !pGlitch->m_pInventory ) {
		SCRIPT_ERROR("G_IsInInventory() : Current bot does not have inventory.");
		return ((cell) FALSE);
	}

	FillStringFromCell(szGoodieName, 99, pAMX, aParams[1]);

	CItemInst *pItem = pGlitch->m_pInventory->IsItemInInventory( szGoodieName );
	
	if( !pItem ) {
		return ((cell) FALSE);
	}

	if( pItem->m_nClipAmmo > 0 ) {
		return((cell) TRUE);
	} 

	return ((cell) FALSE);
}

//----------------------------------------------------------------------------------------------------------------------------
// Debris Wrappers
//----------------------------------------------------------------------------------------------------------------------------

// BOOL Deb_RumbleShake( hDebrisEntity, bEnableRumbleShake )
//
// If bEnableRumbleShake is TRUE, the debris entity will shake when it receives rumble.
// If bEnableRumbleShake is FALSE, the debris entity will not shake when it receives rumble.
cell AMX_NATIVE_CALL CMAST_DebrisWrapper::Deb_RumbleShake( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "Deb_RumbleShake", 2 );

	CEntity *pE = (CEntity *)(aParams[1]);

	if( pE == NULL ) {
		SCRIPT_ERROR( "Deb_RumbleShake() : Passed in NULL entity." );
		return FALSE;
	}

	if( !(pE->TypeBits() & ENTITY_BIT_DEBRIS) ) {
		SCRIPT_ERROR( "Deb_RumbleShake() : Provided entity is not a Debris Entity." );
		return FALSE;
	}

	((CEDebris *)pE)->EnableRumbleShake( aParams[2] );

	return TRUE;
}


// f32 Deb_GetRumbleShake( hDebrisEntity )
//
// Returns TRUE if rumble shake is enabled.
// Returns FALSE if rumble shake is disabled.
cell AMX_NATIVE_CALL CMAST_DebrisWrapper::Deb_GetRumbleShake( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "Deb_GetRumbleShake", 1 );

	CEntity *pE = (CEntity *)(aParams[1]);

	if( pE == NULL ) {
		SCRIPT_ERROR( "Deb_GetRumbleShake() : Passed in NULL entity." );
		return FALSE;
	}

	if( !(pE->TypeBits() & ENTITY_BIT_DEBRIS) ) {
		SCRIPT_ERROR( "Deb_GetRumbleShake() : Provided entity is not a Debris Entity." );
		return FALSE;
	}

	return ((CEDebris *)pE)->IsRumbleShakeEnabled();
}


// BOOL Deb_SetAutoShake( hDebrisEntity, fAutoShakeSecondsMin, fAutoShakeSecondsMax )
//
// If fAutoShakeSecondsMin or Max is 0, auto-shaking is disabled.
// Otherwise, the two parameters specify a time range in which auto-spawing will occur.
cell AMX_NATIVE_CALL CMAST_DebrisWrapper::Deb_SetAutoShake( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "Deb_SetAutoShake", 3 );

	CEntity *pE = (CEntity *)(aParams[1]);

	if( pE == NULL ) {
		SCRIPT_ERROR( "Deb_SetAutoShake() : Passed in NULL entity." );
		return FALSE;
	}

	if( !(pE->TypeBits() & ENTITY_BIT_DEBRIS) ) {
		SCRIPT_ERROR( "Deb_SetAutoShake() : Provided entity is not a Debris Entity." );
		return FALSE;
	}

	((CEDebris *)pE)->SetAutoShake( ConvertCellToF32( aParams[2] ), ConvertCellToF32( aParams[3] ) );

	return TRUE;
}


// BOOL Deb_Shake( hDebrisEntity, fUnitIntensity )
//
// Shakes the debris entity at the given unit intensity.
cell AMX_NATIVE_CALL CMAST_DebrisWrapper::Deb_Shake( AMX *pAMX, cell *aParams ) {
	SCRIPT_CHECK_NUM_PARAMS( "Deb_Shake", 2 );

	CEntity *pE = (CEntity *)(aParams[1]);

	if( pE == NULL ) {
		SCRIPT_ERROR( "Deb_Shake() : Passed in NULL entity." );
		return FALSE;
	}

	if( !(pE->TypeBits() & ENTITY_BIT_DEBRIS) ) {
		SCRIPT_ERROR( "Deb_Shake() : Provided entity is not a Debris Entity." );
		return FALSE;
	}

	((CEDebris *)pE)->Shake( ConvertCellToF32( aParams[2] ) );

	return TRUE;
}





/////////////////////////////////////////////////////////////////
// Natives imported by retail GameCube scripts that this source snapshot did not have.
// Each follows the retail implementation in main.dol (see tools/dol_xref.py).

cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsPosessed(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsPosessed", 1 );
	CBot *pBot = (CBot *)(aParams[1]);
	if( pBot == NULL || !(pBot->TypeBits() & ENTITY_BIT_BOT) ) {
		SCRIPT_ERROR( "Bot_IsPosessed() : Passed in an invalid bot." );
		return (cell)0;
	}
	return (cell)( pBot->IsCreated() && pBot->IsPossessed() );
}


cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_IsRecruited(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_IsRecruited", 1 );
	CBot *pBot = (CBot *)(aParams[1]);
	if( pBot == NULL || !(pBot->TypeBits() & ENTITY_BIT_BOT) ) {
		SCRIPT_ERROR( "Bot_IsRecruited() : Passed in an invalid bot." );
		return (cell)0;
	}
	return (cell)( pBot->IsCreated() && pBot->Recruit_IsRecruited() );
}


// Bot_Recruit( hRecruiterBot, hBot ): hRecruiterBot recruits hBot. Always returns 0.
cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_Recruit(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Bot_Recruit", 2 );
	CBot *pRecruiter = (CBot *)(aParams[1]);
	CBot *pBot = (CBot *)(aParams[2]);
	if( pRecruiter && pBot && (pRecruiter->TypeBits() & ENTITY_BIT_BOT) && (pBot->TypeBits() & ENTITY_BIT_BOT) ) {
		pBot->Recruit( pRecruiter );
	}
	return (cell)0;
}


// Audio_Play2DSoundEx( hSFX, nVolumePercent, nExtra ): like Audio_Play2DSound, but not
// duckable. Retail passes nExtra to a play parameter this source's fsndfx_Play2D lacks.
cell AMX_NATIVE_CALL CMAST_Audio::Audio_Play2DSoundEx(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Audio_Play2DSoundEx", 3 );

	FSndFx_FxHandle_t hSFX = (FSndFx_FxHandle_t)(aParams[1]);
	if( ( hSFX == NULL ) || ( hSFX == FSNDFX_INVALID_FX_HANDLE ) ) {
		if( !faudio_IsInstalled() ) return (cell)0;
		SCRIPT_ERROR( "Audio_Play2DSoundEx : Invalid SFX Handle provided." );
		return (cell)0;
	}

	f32 fVolume = 0.01f * (f32)(u32)aParams[2];
	FMATH_CLAMP_UNIT_FLOAT( fVolume );
	fsndfx_Play2D( hSFX, fVolume, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.0f, FALSE );

	return (cell)0;
}


// Checkpoint_Save2( hSphere ): saves checkpoint 1 with the sphere as the restart location.
cell AMX_NATIVE_CALL CMAST_Checkpoint::Checkpoint_Save2(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Checkpoint_Save2", 1 );
	CEntity *pEntity = (CEntity *)(aParams[1]);
	if( pEntity == NULL || !(pEntity->TypeBits() & ENTITY_BIT_SPHERE) ) {
		return (cell)0;
	}
	return (cell)checkpoint_Save( 1, TRUE, (CESphere *)pEntity );
}


// Console_Enable( hConsole, bEnable ): retail consoles can be switched off by script.
cell AMX_NATIVE_CALL CMAST_GameWrapper::Console_Enable(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Console_Enable", 2 );
	CEntity *pEntity = (CEntity *)(aParams[1]);
	if( pEntity == NULL || !(pEntity->TypeBits() & ENTITY_BIT_CONSOLE) ) {
		return (cell)0;
	}
	((CEConsole *)pEntity)->SetScriptEnabled( aParams[2] != 0 );
	return (cell)1;
}


// A single script-visible value shared by all scripts (retail keeps one global cell).
static cell _nMiscValue;

cell AMX_NATIVE_CALL CMAST_GameWrapper::Misc_SetValue(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Misc_SetValue", 1 );
	_nMiscValue = aParams[1];
	return (cell)1;
}


cell AMX_NATIVE_CALL CMAST_GameWrapper::Misc_GetValue(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Misc_GetValue", 0 );
	return _nMiscValue;
}


// Returns the 0-based difficulty level (0 is easiest).
cell AMX_NATIVE_CALL CMAST_GameWrapper::Misc_GetDifficulty(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "Misc_GetDifficulty", 0 );
	return (cell)CDifficulty::GetLevel();
}


// FX_StompRing( hEntity, fAmplitude, fSpeed ): a visual stomp ring at the entity; it
// pushes nothing (retail passes no entity flags and zero push effects).
cell AMX_NATIVE_CALL CMAST_FXWrapper::FX_StompRing(AMX *pAMX, cell *aParams)
{
	SCRIPT_CHECK_NUM_PARAMS( "FX_StompRing", 3 );
	CEntity *pEntity = (CEntity *)(aParams[1]);
	if( pEntity == NULL ) {
		return (cell)0;
	}
	CFXShockwave::AddShockwave( pEntity->MtxToWorld()->m_vPos, ConvertCellToF32( aParams[2] ), ConvertCellToF32( aParams[3] ), 0, 0.0f, 0.0f );
	return (cell)1;
}
