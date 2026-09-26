//////////////////////////////////////////////////////////////////////////////////////
// botmozer_data.cpp -
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
// 02/25/03 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botmozer.h"
#include "botanim.h"
#include "fmesh.h"



//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotMozer::m_BotDef =
{
	BOTRACE_DROID,						// nRace
	BOTCLASS_COLOSSUS,					// nClass
	BOTSUBCLASS_COLOSSUS_MOZER,			// nSubClass
};

const FGameData_TableEntry_t CBotMozer::m_aBotInfoVocab_Mozer[] = 
{
	//fMaxShockwaveImpulse
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//f32 fMinShockwaveImpulse;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CBotMozer::m_aBotInfoVocab_Girder[] = 
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
//		f32 fTimeScale;
	FGAMEDATA_VOCAB_F32_UNBOUND,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};
//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotMozer::m_aGameDataMap[] =
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

	"Weapon",
	m_aBotInfoVocab_Weapon,
	sizeof(m_BotInfo_Weapon),
	(void *)&m_BotInfo_Weapon,

	"Mozer",
	m_aBotInfoVocab_Mozer,
	sizeof(m_BotInfo_Mozer),
	(void*)&m_BotInfo_Mozer,

	"Girder",
	m_aBotInfoVocab_Girder,
	sizeof(m_BotInfo_Girder),
	(void*)&m_BotInfo_Girder,

	NULL
};



cchar			**CBotMozer::m_apszIdleAnimNameTable;
const u8		**CBotMozer::m_apnEnableBoneNameIndexTableForEachIdleTap;

//--------------------------------------------------------------------------------------------------------
// Animation Stack Data:
//--------------------------------------------------------------------------------------------------------

cchar *CBotMozer::m_apszBoneNameTable[BONE_COUNT] =
{
	// lower body
	"Groin",
	"R_Leg_Upper",
	"R_Leg_Lower",
	"R_Foot_Front",
	"R_Toe",
	"R_Foot_Back",
	"R_Heel",
	"L_Leg_Upper",
	"L_Leg_Lower",
	"L_Foot_Front",
	"L_Toe",
	"L_Foot_Back",
	"L_Heel",
	"ColossusDummy",
	"off_R_Foot",
	"off_L_Foot",
	"off_R_Hand",
	"off_L_Leg_IK",
	"off_R_Leg_IK",

	// upper body
	"L_Arm_Upper",
	"Torso",
	"R_Arm_Upper",
	"R_Arm_Elbow",
	"Gun",
	"L_Arm_Elbow",
	"L_Arm_Lower",
	"L_Hand",
	"Girder",
	"Secondary_Fire",
	"BackB",
	"BackA",
	"R_Jowl",
	"L_Jowl",
	"BackC",
	"BackD",
	"Cylinder01",
	"Head",
	"Jaw",
	"R_Flap",
	"L_Flap",
	"AttachPoint_Primary",
	"off_L_Hand",
	"off_L_Arm_IK",
	"off_R_Arm_IK"
};



cchar *CBotMozer::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] =
{
	"ARDCidle001",	// ANIM_STAND,
	"ARDCwalkF01",	// ANIM_WALK,
	"ARDCrun_F01",	// ANIM_RUN,

	"ARDCjump_01",	// ANIM_JUMP_LAUNCH,
	"ARDCjump_02",	// ANIM_JUMP_FLY,
	"ARDCjump_03",	// ANIM_JUMP_LAND,

	"ARDCjump201",	// ANIM_JUMP2_FLY,
	"ARDCjump202",	// ANIM_JUMP2_LAND,

	"ARDCready01",	// ANIM_FIRE_PRIMARY,

	"ARDCactn2a0",	// ANIM_SWING_WINDUP,
	"ARDCactn2a1",	// ANIM_SWING_FORWARD,
	"ARDCactn2b1",	// ANIM_SWING_BACK,
	"ARDCactn2a2",	// ANIM_SWING_FORWARD_DONE,
	"ARDCactn2b2",	// ANIM_SWING_BACK_DONE,
	
	"ARDCrldP_01",	// ANIM_RELOAD_CLIP_EJECT_OLD,
	"ARDCrldP_02",	// ANIM_RELOAD_CLIP_GRAB_NEW,
	"ARDCrldP_03",	// ANIM_RELOAD_CLIP_INSERT_NEW,
	"ARDCrldP_04"	// ANIM_RELOAD_CLIP_SLAPIN_NEW,
};


cchar *CBotMozer::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] =
{
	"stand",		//	ANIMCONTROL_STAND,
	"walk",			//	ANIMCONTROL_WALK,
	"run",			//	ANIMCONTROL_RUN,

	"jump_launch",	//	ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",		//	ANIMCONTROL_JUMP_FLY,
	"jump_lnd_upr",	//	ANIMCONTROL_JUMP_LAND_UPPER,
	"jump_lnd_lwr",	//	ANIMCONTROL_JUMP_LAND_LOWER,

	"JUMP2_FLY",	//	ANIMCONTROL_JUMP2_FLY,
	"JUMP2_LAND",	//	ANIMCONTROL_JUMP2_LAND,

	"fire_upr",		//	ANIMCONTROL_FIRE_1_UPPER,
	"fire_lwr",		//	ANIMCONTROL_FIRE_1_LOWER,

	"windup_upr",	//	ANIMCONTROL_SWING_WINDUP_UPPER,
	"forward_upr",	//	ANIMCONTROL_SWING_FORWARD_UPPER,
	"back_upr",		//	ANIMCONTROL_SWING_BACK_UPPER,
	"fwddone_upr",	//	ANIMCONTROL_SWING_FORWARD_DONE_UPPER,
	"backdone_upr",	//	ANIMCONTROL_SWING_BACK_DONE_UPPER,

	"windup_lwr",	//	ANIMCONTROL_SWING_WINDUP_LOWER,
	"forward_lwr",	//	ANIMCONTROL_SWING_FORWARD_LOWER,
	"back_lwr",		//	ANIMCONTROL_SWING_BACK_LOWER,
	"fwddone_lwr",	//	ANIMCONTROL_SWING_FORWARD_DONE_LOWER,
	"backdone_lwr",	//	ANIMCONTROL_SWING_BACK_DONE_LOWER,

	"clip_rld_eject",				// ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
	"clip_rld_grab",				// ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
	"clip_rld_insert",				// ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
	"clip_rld_slapin",				// ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

	"recoil_summer",// 	ANIMCONTROL_RECOIL_SUMMER,
};

cchar *CBotMozer::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] =
{
	"stand",		//	ANIMTAP_STAND,
	"walk",			//	ANIMTAP_WALK,
	"run",			//	ANIMTAP_RUN,

	"jump_launch",	//	ANIMTAP_JUMP_LAUNCH,
	"jump_fly",		//	ANIMTAP_JUMP_FLY,
	"jump_lnd_upr",	//	ANIMTAP_JUMP_LAND_UPPER,
	"jump_lnd_lwr",	//	ANIMTAP_JUMP_LAND_LOWER,

	"JUMP2_FLY",	//	ANIMTAP_JUMP2_FLY,
	"JUMP2_LAND",	//	ANIMTAP_JUMP2_LAND,

	"fire_upr",		//	ANIMTAP_FIRE_1_UPPER,
	"fire_lwr",		//	ANIMTAP_FIRE_1_LOWER,

	"windup_upr",	//	ANIMTAP_SWING_WINDUP_UPPER,
	"forward_upr",	//	ANIMTAP_SWING_FORWARD_UPPER,
	"back_upr",		//	ANIMTAP_SWING_BACK_UPPER,
	"fwddone_upr",	//	ANIMTAP_SWING_FORWARD_DONE_UPPER,
	"backdone_upr",	//	ANIMTAP_SWING_BACK_DONE_UPPER,

	"windup_lwr",	//	ANIMTAP_SWING_WINDUP_LOWER,
	"forward_lwr",	//	ANIMTAP_SWING_FORWARD_LOWER,
	"back_lwr",		//	ANIMTAP_SWING_BACK_LOWER,
	"fwddone_lwr",	//	ANIMTAP_SWING_FORWARD_DONE_LOWER,
	"backdone_lwr",	//	ANIMTAP_SWING_BACK_DONE_LOWER,

	"clip_rld_eject",				// ANIMTAP_RELOAD_CLIP_EJECT_OLD,
	"clip_rld_grab",				// ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	"clip_rld_insert",				// ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	"clip_rld_slapin",				// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

	"recoil_summer",// 	ANIMTAP_RECOIL_SUMMER,
};


CFAnimCombinerConfig::ConfigStack_t CBotMozer::m_aAnimCombinerConfigStack[] =
{
	// From highest to lowest priority:
	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_RECOIL_SUMMER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_1_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_WINDUP_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_FORWARD_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_BACK_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_FORWARD_DONE_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_BACK_DONE_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAUNCH,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP2_FLY,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP2_LAND,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_FLY,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_WINDUP_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_FORWARD_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_BACK_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_FORWARD_DONE_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SWING_BACK_DONE_LOWER,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_1_LOWER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,
	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotMozer::m_aAnimCombinerConfigTaps[] =
{
	ANIMTAP_STAND,						ANIMCONTROL_STAND,					1,
	ANIMTAP_WALK,						ANIMCONTROL_WALK,					1,
	ANIMTAP_RUN,						ANIMCONTROL_RUN,					1,
	ANIMTAP_JUMP_LAUNCH,				ANIMCONTROL_JUMP_LAUNCH,			1,
	ANIMTAP_JUMP_FLY,					ANIMCONTROL_JUMP_FLY,				1,
	ANIMTAP_JUMP_LAND_UPPER,			ANIMCONTROL_JUMP_LAND_UPPER,		1,
	ANIMTAP_JUMP_LAND_LOWER,			ANIMCONTROL_JUMP_LAND_LOWER,		1,
	ANIMTAP_JUMP2_FLY,					ANIMCONTROL_JUMP2_FLY,				1,
	ANIMTAP_JUMP2_LAND,					ANIMCONTROL_JUMP2_LAND,				1,
	ANIMTAP_FIRE_1_UPPER,				ANIMCONTROL_FIRE_1_UPPER,			1,
	ANIMTAP_FIRE_1_LOWER,				ANIMCONTROL_FIRE_1_LOWER,			1,
	ANIMTAP_SWING_WINDUP_UPPER,			ANIMCONTROL_SWING_WINDUP_UPPER,		1,
	ANIMTAP_SWING_FORWARD_UPPER,		ANIMCONTROL_SWING_FORWARD_UPPER,	1,
	ANIMTAP_SWING_BACK_UPPER,			ANIMCONTROL_SWING_BACK_UPPER,		1,
	ANIMTAP_SWING_FORWARD_DONE_UPPER,	ANIMCONTROL_SWING_FORWARD_DONE_UPPER,	1,
	ANIMTAP_SWING_BACK_DONE_UPPER,		ANIMCONTROL_SWING_BACK_DONE_UPPER,	1,
	ANIMTAP_SWING_WINDUP_LOWER,			ANIMCONTROL_SWING_WINDUP_LOWER,		1,
	ANIMTAP_SWING_FORWARD_LOWER,		ANIMCONTROL_SWING_FORWARD_LOWER,	1,
	ANIMTAP_SWING_BACK_LOWER,			ANIMCONTROL_SWING_BACK_LOWER,		1,
	ANIMTAP_SWING_FORWARD_DONE_LOWER,	ANIMCONTROL_SWING_FORWARD_DONE_LOWER,	1,
	ANIMTAP_SWING_BACK_DONE_LOWER,		ANIMCONTROL_SWING_BACK_DONE_LOWER,	1,

	ANIMTAP_RELOAD_CLIP_EJECT_OLD,				ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,			1,
	ANIMTAP_RELOAD_CLIP_GRAB_NEW,				ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,			1,
	ANIMTAP_RELOAD_CLIP_INSERT_NEW,				ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,			1,
	ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,				ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,			1,

	ANIMTAP_RECOIL_SUMMER,				ANIMCONTROL_RECOIL_SUMMER,			1,
	255,								255,								255
};

CFAnimCombiner::AttachList_t CBotMozer::m_aAnimAttach[] =
{
	ANIM_STAND,						ANIMTAP_STAND,
	ANIM_WALK,						ANIMTAP_WALK,
	ANIM_RUN,						ANIMTAP_RUN,
	ANIM_JUMP_LAUNCH,				ANIMTAP_JUMP_LAUNCH,
	ANIM_JUMP_FLY,					ANIMTAP_JUMP_FLY,
	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_UPPER,
	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_LOWER,
	ANIM_JUMP2_FLY,					ANIMTAP_JUMP2_FLY,
	ANIM_JUMP2_LAND,				ANIMTAP_JUMP2_LAND,
	ANIM_FIRE_PRIMARY,				ANIMTAP_FIRE_1_UPPER,
	ANIM_FIRE_PRIMARY,				ANIMTAP_FIRE_1_LOWER,
	ANIM_SWING_WINDUP,				ANIMTAP_SWING_WINDUP_UPPER,
	ANIM_SWING_FORWARD,				ANIMTAP_SWING_FORWARD_UPPER,
	ANIM_SWING_BACK,				ANIMTAP_SWING_BACK_UPPER,
	ANIM_SWING_FORWARD_DONE,		ANIMTAP_SWING_FORWARD_DONE_UPPER,
	ANIM_SWING_BACK_DONE,			ANIMTAP_SWING_BACK_DONE_UPPER,
	ANIM_SWING_WINDUP,				ANIMTAP_SWING_WINDUP_LOWER,
	ANIM_SWING_FORWARD,				ANIMTAP_SWING_FORWARD_LOWER,
	ANIM_SWING_BACK,				ANIMTAP_SWING_BACK_LOWER,
	ANIM_SWING_FORWARD_DONE,		ANIMTAP_SWING_FORWARD_DONE_LOWER,
  	ANIM_SWING_BACK_DONE,			ANIMTAP_SWING_BACK_DONE_LOWER,

	ANIM_RELOAD_CLIP_EJECT_OLD,					ANIMTAP_RELOAD_CLIP_EJECT_OLD,
	ANIM_RELOAD_CLIP_GRAB_NEW,					ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	ANIM_RELOAD_CLIP_INSERT_NEW,				ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	ANIM_RELOAD_CLIP_SLAPIN_NEW,				ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMTAP_RECOIL_SUMMER,
	255,						255
};

const u8				CBotMozer::m_nApproxEyePointBoneNameIndex = BONE_HEAD;

const u8 CBotMozer::m_aBoneEnableIndices_FullBody[] =
{
	// lower body				
	BONE_GROIN,				
	BONE_R_LEG_UPPER,			
	BONE_R_LEG_LOWER,			
	BONE_R_FOOT_FRONT,			
	BONE_R_TOE,				
	BONE_R_FOOT_BACK,			
	BONE_R_HEEL,				
	BONE_L_LEG_UPPER,			
	BONE_L_LEG_LOWER,			
	BONE_L_FOOT_FRONT,			
	BONE_L_TOE,				
	BONE_L_FOOT_BACK,			
	BONE_L_HEEL,				
	BONE_COLOSSUSDUMMY,			
	BONE_OFF_R_FOOT,			
	BONE_OFF_L_FOOT,			
	BONE_OFF_R_HAND,			
	BONE_OFF_L_LEG_IK,			
	BONE_OFF_R_LEG_IK,			
	// upper body				
	BONE_L_ARM_UPPER,			
	BONE_TORSO,				
	BONE_R_ARM_UPPER,			
	BONE_R_ARM_ELBOW,			
	BONE_GUN,				
	BONE_L_ARM_ELBOW,			
	BONE_L_ARM_LOWER,			
	BONE_L_HAND,				
	BONE_GIRDER,				
	BONE_SECONDARY_FIRE,			
	BONE_BACKB,				
	BONE_BACKA,				
	BONE_R_JOWL,				
	BONE_L_JOWL,				
	BONE_BACKC,				
	BONE_BACKD,				
	BONE_CYLINDER01,			
	BONE_HEAD,				
	BONE_JAW,				
	BONE_R_FLAP,				
	BONE_L_FLAP,				
	BONE_ATTACHPOINT_PRIMARY,		
	BONE_OFF_L_HAND,			
	BONE_OFF_L_ARM_IK,			
	BONE_OFF_R_ARM_IK,
	255
};

const u8 CBotMozer::m_aBoneEnableIndices_UpperBody[] =
{
	// upper body
	BONE_L_ARM_UPPER,			
	BONE_TORSO,				
	BONE_R_ARM_UPPER,			
	BONE_R_ARM_ELBOW,			
	BONE_GUN,				
	BONE_L_ARM_ELBOW,			
	BONE_L_ARM_LOWER,			
	BONE_L_HAND,				
	BONE_GIRDER,				
	BONE_SECONDARY_FIRE,			
	BONE_BACKB,				
	BONE_BACKA,				
	BONE_R_JOWL,				
	BONE_L_JOWL,				
	BONE_BACKC,				
	BONE_BACKD,				
	BONE_CYLINDER01,			
	BONE_HEAD,				
	BONE_JAW,				
	BONE_R_FLAP,				
	BONE_L_FLAP,				
	BONE_ATTACHPOINT_PRIMARY,		
	BONE_OFF_L_HAND,			
	BONE_OFF_L_ARM_IK,			
	BONE_OFF_R_ARM_IK,
	255
};

const u8 CBotMozer::m_aBoneEnableIndices_LowerBody[] =
{
	// lower body
	BONE_GROIN,				
	BONE_R_LEG_UPPER,			
	BONE_R_LEG_LOWER,			
	BONE_R_FOOT_FRONT,			
	BONE_R_TOE,				
	BONE_R_FOOT_BACK,			
	BONE_R_HEEL,				
	BONE_L_LEG_UPPER,			
	BONE_L_LEG_LOWER,			
	BONE_L_FOOT_FRONT,			
	BONE_L_TOE,				
	BONE_L_FOOT_BACK,			
	BONE_L_HEEL,				
	BONE_COLOSSUSDUMMY,			
	BONE_OFF_R_FOOT,			
	BONE_OFF_L_FOOT,			
	BONE_OFF_R_HAND,			
	BONE_OFF_L_LEG_IK,			
	BONE_OFF_R_LEG_IK,			
	255
};

const u8 CBotMozer::m_aBoneEnableIndices_PriAttack[] =
{
	BONE_L_ARM_UPPER,			
		BONE_R_ARM_UPPER,			
		BONE_R_ARM_ELBOW,			
		BONE_GUN,				
		BONE_R_FLAP,				
		BONE_ATTACHPOINT_PRIMARY,		
		255
};


const u8 CBotMozer::m_anEnableBoneNameIndexTableForSummer_Normal[] =
{
	// Enable these bones to be driven by our summer:
	255
};

const u8 CBotMozer::m_aBoneEnableIndices_RecoilSummer[] = {
	BONE_R_ARM_UPPER,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_COLOSSUSDUMMY,
	255
};

cchar *CBotMozer::m_apszRecoilBoneNameTable[ANIMBONE_RECOIL_COUNT] = {
	"R_Arm_Upper",
	"Torso",
	"L_Arm_Upper",
	"ColossusDummy"
};

/*
const u8 CBotMozer::m_aBoneEnableIndices_UserAnim_UpperBody[] =
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


const u8 CBotMozer::m_aBoneEnableIndices_UserAnim_LowerBody[] =
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


const u8 CBotMozer::m_aBoneEnableIndices_UserAnim_UpperTorso[] =
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


const u8 CBotMozer::m_aBoneEnableIndices_UserAnim_LowerTorso[] =
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


const u8 CBotMozer::m_aBoneEnableIndices_UserAnim_LeftArm[] =
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


const u8 CBotMozer::m_aBoneEnableIndices_UserAnim_RightArm[] =
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


const u8 CBotMozer::m_aBoneEnableIndices_UserAnim_Head[] =
{
	BONE_HEAD,

	255
};
*/

const u8 *CBotMozer::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] =
{
	m_aBoneEnableIndices_FullBody,			// 	ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,			// 	ANIMTAP_WALK,
	m_aBoneEnableIndices_FullBody,			// 	ANIMTAP_RUN,
	m_aBoneEnableIndices_FullBody,			// 	ANIMTAP_JUMP_LAUNCH,
	m_aBoneEnableIndices_FullBody,			// 	ANIMTAP_JUMP_FLY,
	m_aBoneEnableIndices_UpperBody,			// 	ANIMTAP_JUMP_LAND_UPPER,
	m_aBoneEnableIndices_LowerBody,			// 	ANIMTAP_JUMP_LAND_LOWER,
	m_aBoneEnableIndices_FullBody,			// 	ANIMTAP_JUMP2_FLY,
	m_aBoneEnableIndices_FullBody,			// 	ANIMTAP_JUMP2_LAND,
	m_aBoneEnableIndices_PriAttack,			// 	ANIMTAP_FIRE_1_UPPER,
	m_aBoneEnableIndices_LowerBody,			// 	ANIMTAP_FIRE_1_LOWER,
	m_aBoneEnableIndices_UpperBody,			// 	ANIMTAP_SWING_WINDUP_UPPER,
	m_aBoneEnableIndices_UpperBody,			// 	ANIMTAP_SWING_FORWARD_UPPER,
	m_aBoneEnableIndices_UpperBody,			// 	ANIMTAP_SWING_BACK_UPPER,
	m_aBoneEnableIndices_UpperBody,			// 	ANIMTAP_SWING_FORWARD_DONE_UPPER,
	m_aBoneEnableIndices_UpperBody,			// 	ANIMTAP_SWING_BACK_DONE_UPPER,
	m_aBoneEnableIndices_LowerBody,			// 	ANIMTAP_SWING_WINDUP_LOWER,
	m_aBoneEnableIndices_LowerBody,			// 	ANIMTAP_SWING_FORWARD_LOWER,
	m_aBoneEnableIndices_LowerBody,			// 	ANIMTAP_SWING_BACK_LOWER,
	m_aBoneEnableIndices_LowerBody,			// 	ANIMTAP_SWING_FORWARD_DONE_LOWER,
	m_aBoneEnableIndices_LowerBody,			//  ANIMTAP_SWING_BACK_DONE_LOWER,
	
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_EJECT_OLD
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_GRAB_NEW
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_INSERT_NEW
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW
	// without anim
	m_aBoneEnableIndices_RecoilSummer,		// ANIMTAP_RECOIL_SUMMER,
};

const u8 CBotMozer::m_anTagPointBoneNameIndexArray[] =
{
	BONE_HEAD,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_R_ARM_UPPER,

	255
};


BOOL CBotMozer::_BuildAnimStackDef( void )
{
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount = CBotAnimStackDef::ANIM_USER_COUNT;
	AnimStackDefInit.nBaseAnimNameCount = ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable = m_apszBaseAnimNameTable;
	AnimStackDefInit.nIdleCount =  m_nBotInfo_Idles;
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