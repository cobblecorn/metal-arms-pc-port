//////////////////////////////////////////////////////////////////////////////////////
// botEliteGuard_data.h -
//
// Author: Michael Scholz
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/23/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "fanim.h"
#include "BotEliteGuard.h"
#include "botanim.h"
#include "fmesh.h"



//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotEliteGuard::m_BotDef =
{
	BOTRACE_MIL,							// nRace
	BOTCLASS_ELITE_GUARD,					// nClass
	BOTSUBCLASS_GRUNT_NORMAL,				// nSubClass
};

const FGameData_TableEntry_t m_EliteInfoVocab_Dive[] = 
{
// 	f32 fMinVerticalVel
	FGAMEDATA_VOCAB_F32_UNBOUND,
// 	fMinVerticalVel
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fBoostVertical;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fBoostForward;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	fLandingSpeedMult
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	fDiveMinCosHalfAngle
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	f32 fCameraOverrideEnterLookAtX;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fCameraOverrideEnterLookAtY;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fCameraOverrideEnterLookAtZ;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fCameraOverrideEnterFollowPosX;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fCameraOverrideEnterFollowPosY;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fCameraOverrideEnterFollowPosZ;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fCameraOverrideEnterTime;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fCameraOverrideExitTime;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	f32 fAngleExitEnter;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fAngleExitFly;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fAngleExitTuck;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fAngleExitLand;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fAngleExitExit;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	f32 fEnterPitchVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fEnterBlendVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fEnterAnimVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	f32 fFlyPitchVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	f32 fFlyPitchVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
		
//	f32 fTuckPitchVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fTuckBlendVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fTuckAnimVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	f32 fLandPitchVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//	f32 fExitPitchVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fExitBlendVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//	f32 fExitAnimVel;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
	
//	f32 fTimeScale;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameData_TableEntry_t m_EliteInfoVocab_Staff[] = 
{
	// pszDamageProfileSlash:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//		f32 fSwingPush;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fSwingSpeedForward;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fSwingSpeedBackward;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fStreamerLength;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fSwingSwingForwardStreamerOn;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fSwingSwingForwardStreamerFadeOff;
	FGAMEDATA_VOCAB_F32_UNBOUND, 

//		f32 fSwingSwingForwardStreamerOff;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fSwingSwingBackwardStreamerOn;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fSwingSwingBackwardStreamerFadeOff;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fSwingSwingBackwardStreamerOff;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		f32 fStreamerAlpha;
	FGAMEDATA_VOCAB_F32_UNBOUND, 
//		FDecalDefHandle_t hDecalDef;
	FGAMEDATA_VOCAB_DECAL_DEF,

//		FParticle_DefHandle_t hLaserParticle
	FGAMEDATA_VOCAB_PARTICLE,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};
//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotEliteGuard::m_aGameDataMap[] =
{
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"MountAim",
	m_aBotInfoVocab_MountAim,
	sizeof(m_BotInfo_MountAim),
	(void *)&m_BotInfo_MountAim,

	"Walk",
	m_aBotInfoVocab_Walk,
	sizeof(m_BotInfo_Walk),
	(void *)&m_BotInfo_Walk,

	"Jump",
	m_aBotInfoVocab_Jump,
	sizeof(m_BotInfo_Jump),
	(void *)&m_BotInfo_Jump,

	"Dive",
	m_EliteInfoVocab_Dive,
	sizeof(m_DefaultEliteInfoDive),
	(void*)&m_DefaultEliteInfoDive,

	"Staff",
	m_EliteInfoVocab_Staff,
	sizeof(m_DefaultEliteInfoStaff),
	(void*)&m_DefaultEliteInfoStaff,

	NULL
};



//--------------------------------------------------------------------------------------------------------
// The bot's .aid file tells PASM to split the bot into these parts:
//
//  Part bit 1: Torso, waist, and neck
//  Part bit 2: Head
//  Part bit 3: Right arm hierarchy
//  Part bit 4: Upper left arm
//  Part bit 5: Lower left arm hierarchy
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
// Animation Stack Data:
//--------------------------------------------------------------------------------------------------------

cchar *CBotEliteGuard::m_apszBoneNameTable[BONE_COUNT] =
{
	// Staff
	"R_Arm_Upper",
	"R_Elbow",
	"R_Arm_Lower",
	"R_Hand",
	"R_ThumbA",
	"R_ThumbB",
	"R_IndexFingerA",
	"R_IndexFingerB",
	"R_PinkyFingerA",
	"R_PinkyFingerB",
	"R_MiddleFingerA",
	"R_MiddleFingerB",
	"R_Hand_Dummy",
	"Staff",
	"Primary_Fire",

	// Left arm
	"L_Arm_Upper",
	"L_Elbow",
	"L_Arm_Lower",
	"L_Hand",
	"L_Hand_Dummy",
	"L_PinkyFingerA",
	"L_PinkyFingerB",
	"L_MiddleFingerA",
	"L_MiddleFingerB",
	"L_IndexFingerA",
	"L_IndexFingerB",
	"L_ThumbA",
	"L_ThumbB",
	
	// Upper Body
	"Tether_Port",
	"Waist1",
	"Waist2",
	"Waist3",
	"Torso",
	"Neck",
	"Head",

	// Lower Body
	"Groin",
	"R_Leg_Upper",
	"R_Leg_Piston1_Top",
	"R_Leg_Piston2_Top",
	"R_Knee_Dummy",
	"R_Leg_Lower",
	"R_Foot",
	"R_Foot_Back",
	"R_Heel",
	"R_Foot_Front",
	"R_Toe",
	"R_Leg_Piston1_Bottom",
	"R_Leg_Piston3_Bottom",
	"R_Leg_Piston2_Bottom",
	"R_Leg_Piston3_Top",
	"L_Leg_Upper",
	"L_Leg_Piston2_Top",
	"L_Leg_Piston3_Top",
	"L_Knee_Dummy",
	"L_Leg_Lower",
	"L_Foot",
	"L_Foot_Back",
	"L_Heel",
	"L_Foot_Front",
	"L_Toe",
	"L_Leg_Piston3_Bottom",
	"L_Leg_Piston1_Bottom",
	"L_Leg_Piston2_Bottom",
	"L_Leg_Piston1_Top"
};



cchar *CBotEliteGuard::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMEidle001",	   // ANIM_STAND,
	"ARMEwalkf01",	   // ANIM_WALK,
	"ARMErun_F01",	   // ANIM_RUN,

	"ARMEidle003",	   // ANIM_STAND_ALERT,
	"ARMEwalkF02",	   // ANIM_WALK_ALERT,

	"ARMEjump_01",	   // ANIM_JUMP_LAUNCH,		
	"ARMEjump_02",	   // ANIM_JUMP_FLY,		
	"ARMEjump_03",	   // ANIM_JUMP_LAND_UPPER,		
	
	"ARMEcabls01",	  // ANIM_CABLE_REACH,	
	"ARMEcabls02",	  // ANIM_CABLE_GRASP,	
	"ARMEcabls03",	  // ANIM_CABLE_RELEASE,	

	//dive move separated  into 3 pieces
	"ARMEjump201",	   // ANIM_JUMP_TUCK_STAFF,		
	"ARMEjump202",	   // ANIM_JUMP_TUCK,		
	"ARMEjump203",	   // ANIM_JUMP_UNTUCK,		

	//dive,roll,getup
	"ARMEdive_01",	   // ANIM_DIVEROLL,		

	"ARMEactn101",	   // ANIM_FIRE_CHARGE,	
	"ARMEactn102",	   // ANIM_FIRE,		
	"ARMEactn103",	   // ANIM_FIRE_DONE,		

	"ARMEactn201",	   // ANIM_SWING_CHARGE,	
	"ARMEactn202",	   // ANIM_SWING_FORWARD,	
	"ARMEactn203",	   // ANIM_SWING_BACK,	
	"ARMEactn204",	   // ANIM_SWING_DONE,	
	
	"ARMEelect01",	   // ANIM_RC_TETHERED,
	"ARMEpwrdn01",	   // ANIM_RC_POWER_DOWN,				
	"ARMEpwrup01",	   // ANIM_RC_POWER_UP,
	"ARMEactn401",	   // ANIM_FIRE_STAFF,	
};
 

cchar *CBotEliteGuard::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMEidle001",
};

cchar *CBotEliteGuard::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] =
{
	"stand",				// ANIMCONTROL_STAND,		
	"walk",					// ANIMCONTROL_WALK,		
	"run",					// ANIMCONTROL_RUN,		

	"stand_alert",			// ANIMCONTROL_STAND_ALERT,		
	"walk_alert",			// ANIMCONTROL_WALK_ALERT,		

	"launch_u",				// ANIMCONTROL_JUMP_LAUNCH,		
	"fly",					// ANIMCONTROL_JUMP_FLY,		
	"land_upr",				// ANIMCONTROL_JUMP_LAND_UPPER,		
	"land_lwr",				// ANIMCONTROL_JUMP_LAND_LOWER,		

	"tuck_staff",			// ANIMCONTROL_JUMP_TUCK_STAFF,		
	"tuck",					// ANIMCONTROL_JUMP_TUCK,		
	"untuck",				// ANIMCONTROL_JUMP_UNTUCK,		

	"diveroll",				// ANIMCONTROL_DIVEROLL

	"fire0upr",				// ANIMCONTROL_FIRE_CHARGE_UPPER,	
	"fire1upr",				// ANIMCONTROL_FIRE_UPPER,		
	"fire2upr",				// ANIMCONTROL_FIRE_DONE_UPPER,	

	"swng0upr",				// ANIMCONTROL_SWING_CHARGE_UPPER,	
	"swng1upr",				// ANIMCONTROL_SWING_FORWARD_UPPER,	
	"swng2upr",				// ANIMCONTROL_SWING_BACK_UPPER,	
	"swng3upr",				// ANIMCONTROL_SWING_DONE_UPPER,	

	"fire0lwr",				// ANIMCONTROL_FIRE_CHARGE_LOWER,	
	"fire1lwr",				// ANIMCONTROL_FIRE_LOWER,		
	"fire2lwr",				// ANIMCONTROL_FIRE_DONE_LOWER,	

	"swng0lwr",				// ANIMCONTROL_SWING_CHARGE_LOWER,	
	"swng1lwr",				// ANIMCONTROL_SWING_FORWARD_LOWER,	
	"swng2lwr",				// ANIMCONTROL_SWING_BACK_LOWER,	
	"swng3lwr",				// ANIMCONTROL_SWING_DONE_LOWER,	

	"tethered",				// ANIMCONTROL_RC_TETHERED,
	"power_up",				// ANIMCONTROL_RC_POWER_UP,
	"power_down",			// ANIMCONTROL_RC_POWER_DOWN,

	"cable_reach",			// ANIMCONTROL_CABLE_REACH,
	"cable_grasp",			// ANIMCONTROL_CABLE_GRASP,
	"cable_release",		// ANIMCONTROL_CABLE_RELEASE,

	"fire_staff",			// ANIMCONTROL_FIRE_STAFF,

	"aim_summer"			// ANIMCONTROL_AIM_SUMMER,
};


cchar *CBotEliteGuard::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] =
{
	"stand",				// ANIMTAP_STAND,
	"walk",					// ANIMTAP_WALK,		
	"run",					// ANIMTAP_RUN,		

	"stand_alert",			// ANIMTAP_STAND_ALERT
	"walk_alert",			// ANIMTAP_WALK_ALERT,		

	"launch_u",				// ANIMTAP_JUMP_LAUNCH,		
	"fly",					// ANIMTAP_JUMP_FLY,		
	"land_upr",				// ANIMTAP_JUMP_LAND_UPPER,		
	"land_lwr",				// ANIMTAP_JUMP_LAND_LOWER,		

	"tuck_staff",			// ANIMTAP_JUMP_TUCK_STAFF,		
	"tuck",					// ANIMTAP_JUMP_TUCK,		
	"untuck",				// ANIMTAP_JUMP_UNTUCK,		

	"diveroll",				// ANIMTAP_DIVEROLL

	"fire0upr",				// ANIMTAP_FIRE_CHARGE_UPPER,	
	"fire1upr",				// ANIMTAP_FIRE_UPPER,		
	"fire2upr",				// ANIMTAP_FIRE_DONE_UPPER,	

	"swng0upr",				// ANIMTAP_SWING_CHARGE_UPPER,	
	"swng1upr",				// ANIMTAP_SWING_FORWARD_UPPER,	
	"swng2upr",				// ANIMTAP_SWING_BACK_UPPER,	
	"swng3upr",				// ANIMTAP_SWING_DONE_UPPER,	

	"fire0lwr",				// ANIMTAP_FIRE_CHARGE_LOWER,	
	"fire1lwr",				// ANIMTAP_FIRE_LOWER,		
	"fire2lwr",				// ANIMTAP_FIRE_DONE_LOWER,	

	"swng0lwr",				// ANIMTAP_SWING_CHARGE_LOWER,	
	"swng1lwr",				// ANIMTAP_SWING_FORWARD_LOWER,	
	"swng2lwr",				// ANIMTAP_SWING_BACK_LOWER,	
	"swng3lwr",				// ANIMTAP_SWING_DONE_LOWER,	

	"tethered",				// ANIMTAP_RC_TETHERED,
	"power_up",				// ANIMTAP_RC_POWER_UP,
	"power_down",			// ANIMTAP_RC_POWER_DOWN,

	"cable_reach",			// ANIMTAP_CABLE_REACH,
	"cable_grasp",			// ANIMTAP_CABLE_GRASP,
	"cable_release",		// ANIMTAP_CABLE_RELEASE,

	"fire_staff",			// ANIMTAP_FIRE_STAFF,

	"aim_summer"			// ANIMTAP_AIM_SUMMER,
};


CFAnimCombinerConfig::ConfigStack_t CBotEliteGuard::m_aAnimCombinerConfigStack[] =
{
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_TETHERED,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CABLE_REACH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CABLE_RELEASE,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CABLE_GRASP,
	
	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_CHARGE_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_FORWARD_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_BACK_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_DONE_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_CHARGE_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_DONE_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_STAFF,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_UNTUCK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_TUCK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_TUCK_STAFF,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_FLY,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_DIVEROLL,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK_ALERT,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_CHARGE_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_DONE_LOWER,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_CHARGE_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_FORWARD_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_BACK_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_DONE_LOWER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_DOWN,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_ALERT,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,
	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotEliteGuard::m_aAnimCombinerConfigTaps[] =
{
	ANIMTAP_STAND,								ANIMCONTROL_STAND,					1,
	ANIMTAP_RUN,								ANIMCONTROL_RUN,					1,
	ANIMTAP_WALK,								ANIMCONTROL_WALK,					1,

	ANIMTAP_STAND_ALERT,						ANIMCONTROL_STAND_ALERT,			1,
	ANIMTAP_WALK_ALERT,							ANIMCONTROL_WALK_ALERT,				1,

	ANIMTAP_CABLE_REACH,						ANIMCONTROL_CABLE_REACH,			1,
	ANIMTAP_CABLE_GRASP,						ANIMCONTROL_CABLE_GRASP,			1,
	ANIMTAP_CABLE_RELEASE,						ANIMCONTROL_CABLE_RELEASE,			1,

	ANIMTAP_JUMP_LAND_UPPER,					ANIMCONTROL_JUMP_LAND_UPPER,		1,
	ANIMTAP_JUMP_LAND_LOWER,					ANIMCONTROL_JUMP_LAND_LOWER,		1,
	ANIMTAP_JUMP_FLY,							ANIMCONTROL_JUMP_FLY,				1,

	ANIMTAP_JUMP_LAUNCH,						ANIMCONTROL_JUMP_LAUNCH,			1,
	ANIMTAP_JUMP_TUCK_STAFF,					ANIMCONTROL_JUMP_TUCK_STAFF,		1,
	ANIMTAP_JUMP_TUCK,							ANIMCONTROL_JUMP_TUCK,				1,
	ANIMTAP_JUMP_UNTUCK,						ANIMCONTROL_JUMP_UNTUCK,			1,
	
	ANIMTAP_DIVEROLL,							ANIMCONTROL_DIVEROLL,				1,

	ANIMTAP_FIRE_CHARGE_UPPER,					ANIMCONTROL_FIRE_CHARGE_UPPER,		1,
	ANIMTAP_FIRE_UPPER,							ANIMCONTROL_FIRE_UPPER,				1,
	ANIMTAP_FIRE_DONE_UPPER,					ANIMCONTROL_FIRE_DONE_UPPER,		1,
	
	ANIMTAP_SWING_CHARGE_UPPER,					ANIMCONTROL_SWING_CHARGE_UPPER,		1,
	ANIMTAP_SWING_FORWARD_UPPER,				ANIMCONTROL_SWING_FORWARD_UPPER,	1,
	ANIMTAP_SWING_BACK_UPPER,					ANIMCONTROL_SWING_BACK_UPPER,		1,
	ANIMTAP_SWING_DONE_UPPER,					ANIMCONTROL_SWING_DONE_UPPER,		1,
	
	ANIMTAP_FIRE_CHARGE_LOWER,					ANIMCONTROL_FIRE_CHARGE_LOWER,		1,
	ANIMTAP_FIRE_LOWER,							ANIMCONTROL_FIRE_LOWER,				1,
	ANIMTAP_FIRE_DONE_LOWER,					ANIMCONTROL_FIRE_DONE_LOWER,		1,
	
	ANIMTAP_SWING_CHARGE_LOWER,					ANIMCONTROL_SWING_CHARGE_LOWER,		1,
	ANIMTAP_SWING_FORWARD_LOWER,				ANIMCONTROL_SWING_FORWARD_LOWER,	1,
	ANIMTAP_SWING_BACK_LOWER,					ANIMCONTROL_SWING_BACK_LOWER,		1,
	ANIMTAP_SWING_DONE_LOWER,					ANIMCONTROL_SWING_DONE_LOWER,		1,	

	ANIMTAP_RC_TETHERED,						ANIMCONTROL_RC_TETHERED,			1,
	ANIMTAP_RC_POWER_DOWN,						ANIMCONTROL_RC_POWER_DOWN,			1,
	ANIMTAP_RC_POWER_UP,						ANIMCONTROL_RC_POWER_UP,			1,

	ANIMTAP_FIRE_STAFF,							ANIMCONTROL_FIRE_STAFF,				1,

	ANIMTAP_AIM_SUMMER,							ANIMCONTROL_AIM_SUMMER,				1,
	255,										255,								255
};


CFAnimCombiner::AttachList_t CBotEliteGuard::m_aAnimAttach[] =
{
	ANIM_RUN,						ANIMTAP_RUN,					
	ANIM_WALK,						ANIMTAP_WALK,					
	ANIM_STAND,						ANIMTAP_STAND,

	ANIM_STAND_ALERT,				ANIMTAP_STAND_ALERT,
	ANIM_WALK_ALERT,				ANIMTAP_WALK_ALERT,

	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_UPPER,				
	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_LOWER,				
	ANIM_JUMP_FLY,					ANIMTAP_JUMP_FLY,				
	ANIM_JUMP_LAUNCH,				ANIMTAP_JUMP_LAUNCH,

	ANIM_JUMP_TUCK_STAFF,			ANIMTAP_JUMP_TUCK_STAFF,				
	ANIM_JUMP_TUCK,					ANIMTAP_JUMP_TUCK,				
	ANIM_JUMP_UNTUCK,				ANIMTAP_JUMP_UNTUCK,

	ANIM_DIVEROLL,					ANIMTAP_DIVEROLL,

	ANIM_CABLE_REACH,				ANIMTAP_CABLE_REACH,
	ANIM_CABLE_GRASP,				ANIMTAP_CABLE_GRASP,
	ANIM_CABLE_RELEASE,				ANIMTAP_CABLE_RELEASE,
	
	ANIM_FIRE_CHARGE,				ANIMTAP_FIRE_CHARGE_UPPER,			
	ANIM_FIRE,						ANIMTAP_FIRE_UPPER,				
	ANIM_FIRE_DONE,					ANIMTAP_FIRE_DONE_UPPER,			
	
	ANIM_SWING_CHARGE,				ANIMTAP_SWING_CHARGE_UPPER,			
	ANIM_SWING_FORWARD,				ANIMTAP_SWING_FORWARD_UPPER,			
	ANIM_SWING_BACK,				ANIMTAP_SWING_BACK_UPPER,			
	ANIM_SWING_DONE,				ANIMTAP_SWING_DONE_UPPER,			
	
	ANIM_FIRE_CHARGE,				ANIMTAP_FIRE_CHARGE_LOWER,			
	ANIM_FIRE,						ANIMTAP_FIRE_LOWER,
	ANIM_FIRE_DONE,					ANIMTAP_FIRE_DONE_LOWER,			
	
	ANIM_SWING_CHARGE,				ANIMTAP_SWING_CHARGE_LOWER,			
	ANIM_SWING_FORWARD,				ANIMTAP_SWING_FORWARD_LOWER,			
	ANIM_SWING_BACK,				ANIMTAP_SWING_BACK_LOWER,			
	ANIM_SWING_DONE,				ANIMTAP_SWING_DONE_LOWER,			

	ANIM_RC_TETHERED,				ANIMTAP_RC_TETHERED,
	ANIM_RC_POWER_DOWN,				ANIMTAP_RC_POWER_DOWN,
	ANIM_RC_POWER_UP,				ANIMTAP_RC_POWER_UP,
	ANIM_FIRE_STAFF,				ANIMTAP_FIRE_STAFF,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMTAP_AIM_SUMMER,
	255,						255
};

const u8 CBotEliteGuard::m_nApproxEyePointBoneNameIndex = BONE_HEAD;

const u8 CBotEliteGuard::m_aBoneEnableIndices_FullBody[] =
{
	// Staff
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_HAND_DUMMY,
	BONE_STAFF,
	BONE_PRIMARY_FIRE,

	// Left arm
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,

	// Upper Body
	BONE_TETHER_PORT,
	BONE_WAIST1,
	BONE_WAIST2,
	BONE_WAIST3,
	BONE_TORSO,
	BONE_NECK,
	BONE_HEAD,

	// Lower Body
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_PISTON1_TOP,
	BONE_R_LEG_PISTON2_TOP,
	BONE_R_KNEE_DUMMY,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,
	BONE_R_LEG_PISTON1_BOTTOM,
	BONE_R_LEG_PISTON3_BOTTOM,
	BONE_R_LEG_PISTON2_BOTTOM,
	BONE_R_LEG_PISTON3_TOP,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_PISTON2_TOP,
	BONE_L_LEG_PISTON3_TOP,
	BONE_L_KNEE_DUMMY,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,
	BONE_L_LEG_PISTON3_BOTTOM,
	BONE_L_LEG_PISTON1_BOTTOM,
	BONE_L_LEG_PISTON2_BOTTOM,
	BONE_L_LEG_PISTON1_TOP,
	255
};

const u8 CBotEliteGuard::m_aBoneEnableIndices_UpperBody[] =
{
	// Staff
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_HAND_DUMMY,
	BONE_STAFF,
	BONE_PRIMARY_FIRE,

	// Left arm
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,

	// Upper Body
	BONE_TETHER_PORT,
	BONE_WAIST1,
	BONE_WAIST2,
	BONE_WAIST3,
	BONE_TORSO,
	BONE_NECK,
	BONE_HEAD,
	255
};

const u8 CBotEliteGuard::m_aBoneEnableIndices_FireStaff[] =
{
	// Staff
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_HAND_DUMMY,
	BONE_STAFF,
	BONE_PRIMARY_FIRE,

	//// Left arm
	//BONE_L_ARM_UPPER,
	//BONE_L_ELBOW,
	//BONE_L_ARM_LOWER,
	//BONE_L_HAND,
	//BONE_L_HAND_DUMMY,
	//BONE_L_PINKYFINGERA,
	//BONE_L_PINKYFINGERB,
	//BONE_L_MIDDLEFINGERA,
	//BONE_L_MIDDLEFINGERB,
	//BONE_L_INDEXFINGERA,
	//BONE_L_INDEXFINGERB,
	//BONE_L_THUMBA,
	//BONE_L_THUMBB,

	// Upper Body
	BONE_TETHER_PORT,
	BONE_WAIST1,
	BONE_WAIST2,
	BONE_WAIST3,
	BONE_TORSO,
	BONE_NECK,
	BONE_HEAD,
	255
};


const u8 CBotEliteGuard::m_aBoneEnableIndices_LowerBody[] =
{
	// Lower Body
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_PISTON1_TOP,
	BONE_R_LEG_PISTON2_TOP,
	BONE_R_KNEE_DUMMY,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,
	BONE_R_LEG_PISTON1_BOTTOM,
	BONE_R_LEG_PISTON3_BOTTOM,
	BONE_R_LEG_PISTON2_BOTTOM,
	BONE_R_LEG_PISTON3_TOP,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_PISTON2_TOP,
	BONE_L_LEG_PISTON3_TOP,
	BONE_L_KNEE_DUMMY,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,
	BONE_L_LEG_PISTON3_BOTTOM,
	BONE_L_LEG_PISTON1_BOTTOM,
	BONE_L_LEG_PISTON2_BOTTOM,
	BONE_L_LEG_PISTON1_TOP,
	255
};

const u8 CBotEliteGuard::m_aBoneEnableIndices_LeftArm[] =
{
	// Left arm
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,
	255
};

const u8 CBotEliteGuard::m_aBoneEnableIndices_AimSummer[] = 
{
	BONE_HEAD,
	BONE_TORSO,
	255
};

const u8 CBotEliteGuard::m_anEnableBoneNameIndexTableForSummer_Normal[] =
{
	// Enable these bones to be driven by our summer:
	255
};

const u8 CBotEliteGuard::m_anEnableBoneNameIndexTableForSummer_TetherShock[] = 
{
	// Staff
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_HAND_DUMMY,
	BONE_STAFF,
	BONE_PRIMARY_FIRE,

	// Left arm
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,

	// Upper Body
	BONE_TETHER_PORT,
	BONE_WAIST1,
	BONE_WAIST2,
	BONE_WAIST3,
	BONE_TORSO,
	BONE_NECK,
	BONE_HEAD,

	// Lower Body
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_PISTON1_TOP,
	BONE_R_LEG_PISTON2_TOP,
	BONE_R_KNEE_DUMMY,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,
	BONE_R_LEG_PISTON1_BOTTOM,
	BONE_R_LEG_PISTON3_BOTTOM,
	BONE_R_LEG_PISTON2_BOTTOM,
	BONE_R_LEG_PISTON3_TOP,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_PISTON2_TOP,
	BONE_L_LEG_PISTON3_TOP,
	BONE_L_KNEE_DUMMY,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,
	BONE_L_LEG_PISTON3_BOTTOM,
	BONE_L_LEG_PISTON1_BOTTOM,
	BONE_L_LEG_PISTON2_BOTTOM,
	BONE_L_LEG_PISTON1_TOP,
	255
};



/*
const u8 CBotEliteGuard::m_aBoneEnableIndices_UserAnim_UpperBody[] =
{
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_HAND_DUMMY,

	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,

	BONE_WAIST1,
	BONE_WAIST2,
	BONE_WAIST3,
	BONE_TORSO,
	BONE_NECK,
	BONE_HEAD,
	BONE_STAFF,

	255
};


const u8 CBotEliteGuard::m_aBoneEnableIndices_UserAnim_LowerBody[] =
{
	BONE_R_LEG_UPPER,
	BONE_R_LEG_PISTON1_TOP,
	BONE_R_LEG_PISTON2_TOP,
	BONE_R_KNEE_DUMMY,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,
	BONE_R_LEG_PISTON1_BOTTOM,
	BONE_R_LEG_PISTON3_BOTTOM,
	BONE_R_LEG_PISTON2_BOTTOM,
	BONE_R_LEG_PISTON3_TOP,

	BONE_L_LEG_UPPER,
	BONE_L_LEG_PISTON2_TOP,
	BONE_L_LEG_PISTON3_TOP,
	BONE_L_KNEE_DUMMY,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,
	BONE_L_LEG_PISTON3_BOTTOM,
	BONE_L_LEG_PISTON1_BOTTOM,

	BONE_GROIN,

	255
};


const u8 CBotEliteGuard::m_aBoneEnableIndices_UserAnim_UpperTorso[] =
{
	BONE_WAIST1,
	BONE_WAIST2,
	BONE_WAIST3,
	BONE_TORSO,
	BONE_NECK,
	BONE_HEAD,
	BONE_STAFF,
	255
};


const u8 CBotEliteGuard::m_aBoneEnableIndices_UserAnim_LowerTorso[] =
{
	BONE_R_LEG_UPPER,
	BONE_R_LEG_PISTON1_TOP,
	BONE_R_LEG_PISTON2_TOP,
	BONE_R_KNEE_DUMMY,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,
	BONE_R_LEG_PISTON1_BOTTOM,
	BONE_R_LEG_PISTON3_BOTTOM,
	BONE_R_LEG_PISTON2_BOTTOM,
	BONE_R_LEG_PISTON3_TOP,

	BONE_L_LEG_UPPER,
	BONE_L_LEG_PISTON2_TOP,
	BONE_L_LEG_PISTON3_TOP,
	BONE_L_KNEE_DUMMY,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,
	BONE_L_LEG_PISTON3_BOTTOM,
	BONE_L_LEG_PISTON1_BOTTOM,

	BONE_GROIN,

	255
};


const u8 CBotEliteGuard::m_aBoneEnableIndices_UserAnim_LeftArm[] =
{
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,

	255
};


const u8 CBotEliteGuard::m_aBoneEnableIndices_UserAnim_RightArm[] =
{
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_HAND_DUMMY,

	255
};


const u8 CBotEliteGuard::m_aBoneEnableIndices_UserAnim_Head[] =
{
	BONE_HEAD,

	255
};
*/

const u8 *CBotEliteGuard::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] =
{
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_WALK,			
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_RUN,			
	//m_aBoneEnableIndices_LowerBody,			// ANIMTAP_RUN

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_STAND_ALERT,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_WALK_ALERT,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_LAUNCH,		
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_FLY,		
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_JUMP_LAND_UPPER,		
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_JUMP_LAND_LOWER,	
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_JUMP_TUCK_STAFF,		
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_TUCK,		
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_UNTUCK,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_DIVEROLL	
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_FIRE_CHARGE_UPPER,	
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_FIRE_UPPER,		
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_FIRE_DONE_UPPER,	
				
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_SWING_CHARGE_UPPER,	
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_SWING_FORWARD_UPPER,	
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_SWING_BACK_UPPER,	
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_SWING_DONE_UPPER,	

	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_FIRE_CHARGE_LOWER,	
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_FIRE_LOWER,		
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_FIRE_DONE_LOWER,	
			 
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_SWING_CHARGE_LOWER,	
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_SWING_FORWARD_LOWER,	
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_SWING_BACK_LOWER,	
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_SWING_DONE_LOWER,
	
	m_aBoneEnableIndices_FullBody,			//	ANIMTAP_RC_TETHERED,
	m_aBoneEnableIndices_FullBody,			//	ANIMTAP_RC_POWER_DOWN,
	m_aBoneEnableIndices_FullBody,			//	ANIMTAP_RC_POWER_UP,

	m_aBoneEnableIndices_LeftArm,			// ANIMTAP_CABLE_REACH,				
	m_aBoneEnableIndices_LeftArm,			// ANIMTAP_CABLE_GRASP,				
	m_aBoneEnableIndices_LeftArm,			// ANIMTAP_CABLE_RELEASE,
	m_aBoneEnableIndices_FireStaff,		// ANIMTAP_FIRE_STAFF,
		
	// without anim
	m_aBoneEnableIndices_AimSummer,			// ANIMTAP_AIM_SUMMER,

};

const u8 *CBotEliteGuard::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] =
{
	m_aBoneEnableIndices_FullBody,									// Idle 0
};


const u8 CBotEliteGuard::m_anTagPointBoneNameIndexArray[] =
{
	BONE_HEAD,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_R_ARM_UPPER,

	255
};


BOOL CBotEliteGuard::_BuildAnimStackDef( void )
{
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount = CBotAnimStackDef::ANIM_USER_COUNT;
	AnimStackDefInit.nBaseAnimNameCount = ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable = m_apszBaseAnimNameTable;
	AnimStackDefInit.nIdleCount = ANIM_IDLE_COUNT;
	AnimStackDefInit.apszIdleAnimNameTable = m_apszIdleAnimNameTable;
	AnimStackDefInit.nBaseControlCount = ANIMCONTROL_BASE_COUNT;
	AnimStackDefInit.apszBaseControlNameTable = m_apszBaseControlNameTable;
	AnimStackDefInit.nBaseTapCount = ANIMTAP_BASE_COUNT;
	AnimStackDefInit.apszBaseTapNameTable = m_apszBaseTapNameTable;
	AnimStackDefInit.nBoneCount = BONE_COUNT;
	AnimStackDefInit.apszBoneNameTable = m_apszBoneNameTable;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap = m_apnEnableBoneNameIndexTableForEachBaseTap;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap = m_apnEnableBoneNameIndexTableForEachIdleTap;
	AnimStackDefInit.pnEnableBoneNameIndexTableForSummer = m_anEnableBoneNameIndexTableForSummer_Normal;
	AnimStackDefInit.pBaseAnimConfigNet = NULL;
	AnimStackDefInit.pBaseAnimConfigStack = m_aAnimCombinerConfigStack;
	AnimStackDefInit.pBaseAnimConfigTap = m_aAnimCombinerConfigTaps;
	AnimStackDefInit.pBaseAnimAttachList = m_aAnimAttach;
	AnimStackDefInit.nIdleStackConnectionControlIndex = ANIMCONTROL_STAND;
	AnimStackDefInit.nIdleStackConnectionControlInput = 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
}