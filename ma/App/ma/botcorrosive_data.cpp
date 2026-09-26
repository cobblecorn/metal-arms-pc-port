//////////////////////////////////////////////////////////////////////////////////////
// botcorrosive_data.cpp - General Corrosive
//
// Author: Russell Foushee     
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
// 03/13/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botanim.h"
#include "fmesh.h"
#include "botcorrosive.h"

const CBotDef CBotCorrosive::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_CORROSIVE,						// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CBotCorrosive::m_apszBoneNameTable[BONE_COUNT] = {
	"CorrosiveDummy",		// BONE_DUMMY				
	"Groin",				// BONE_GROIN
	"R_Leg_Upper",			// BONE_R_LEG_UPPPER
	"R_Leg_Middle",			// BONE_R_LEG_MIDDLE
	"R_Leg_Lower",			// BONE_R_LEG_LOWER
	"R_Foot",				// BONE_R_FOOT
	"R_Foot_Back",			// BONE_R_FOOT_BACK
	"R_Heel",				// BONE_R_HEEL
	"R_Foot_Front",			// BONE_R_FOOT_FRONT
	"R_Toe",				// BONE_R_TOE
	"L_Leg_Upper",			// BONE_L_LEG_UPPER
	"L_Leg_Middle",			// BONE_L_LEG_MIDDLE
	"L_Leg_Lower",			// BONE_L_LEG_LOWER
	"L_Foot",				// BONE_L_FOOT
	"L_Foot_Back",			// BONE_L_FOOT_BACK
	"L_Heel",				// BONE_L_HEEL
	"L_Foot_Front",			// BONE_L_FOOT_FRONT
	"L_Toe",				// BONE_L_TOE
	"WaistA",				// BONE_WAISTA
	"WaistB",				// BONE_WAISTB
	"WaistC",				// BONE_WAISTC
	"Torso",				// BONE_TORSO
	"L_Arm_Upper",			// BONE_L_ARM_UPPER
	"L_Elbow",				// BONE_L_ELBOW
	"L_Arm_Lower",			// BONE_L_ARM_LOWER
	"L_Hand",				// BONE_L_HAND
	"L_IndexFingerA",		// BONE_L_INDEXFINGERA
	"L_IndexFingerB",		// BONE_L_INDEXFINGERB
	"L_MiddleFingerA",		// BONE_L_MIDDLEFINGERA
	"L_MiddleFingerB",		// BONE_L_MIDDLEFINGERB
	"L_PinkyFingerA",		// BONE_L_PINKYFINGERA
	"L_PinkyFingerB",		// BONE_L_PINKYFINGERB
	"L_ThumbA",				// BONE_L_THUMBA
	"L_ThumbB",				// BONE_L_THUMBB
	"L_Hand_Dummy",			// BONE_L_HAND_DUMMY
	"R_Arm_Upper",			// BONE_R_ARM_UPPER
	"R_Elbow",				// BONE_R_ELBOW
	"R_Arm_Lower",			// BONE_R_ARM_LOWER
	"R_Hand",				// BONE_R_HAND
	"R_ThumbA",				// BONE_R_THUMBA
	"R_ThumbB",				// BONE_R_THUMBB
	"R_MiddleFingerA",		// BONE_R_MIDDLEFINGERA
	"R_MiddleFingerB",		// BONE_R_MIDDLEFINGERB
	"R_PinkyFingerA",		// BONE_R_PINKYFINGERA
	"R_PinkyFingerB",		// BONE_R_PINKYFINGERB
	"R_IndexFingerA",		// BONE_R_INDEXFINGERA
	"R_IndexFingerB",		// BONE_R_INDEXFINGERB
	"R_Hand_Dummy",			// BONE_R_HAND_DUMMY
	"R_ShoulderB",			// BONE_R_SHOULDERB
	"R_ShoulderA",			// BONE_R_SHOULDERA
	"L_ShoulderA",			// BONE_L_SHOULDERA
	"L_ShoulderB",			// BONE_L_SHOULDERB
	"Neck_Upper",			// BONE_NECK_UPPER
	"Head",					// BONE_HEAD
	"L_Jaw_Piston_A",		// BONE_L_JAW_PISTON_A
	"R_Jaw_Piston_A",		// BONE_R_JAW_PISTON_A
	"Neck_PistonB",			// BONE_NECK_PISTONB
	"Jaw",					// BONE_JAW
	"L_Jaw_Piston_B",		// BONE_L_JAW_PISTON_B
	"R_Jaw_Piston_B",		// BONE_R_JAW_PISTON_B
	"Neck_PistonA",			// BONE_NECK_PISTONA
	"Chest_DoorA",			// BONE_CHEST_DOORA
	"Chest_DoorB",			// BONE_CHEST_DOORB
	"Chest_DoorC",			// BONE_CHEST_DOORC
	"Chest_DoorD",			// BONE_CHEST_DOORD
	"Chest_DoorE",			// BONE_CHEST_DOORE
	"Primary_Fire",			// BONE_PRIMARY_FIRE
	"WaistD",				// BONE_WAIST_D

	"Head_Wound01",			// BONE_HEAD_WOUND01			
	"Head_Wound02",			// BONE_HEAD_WOUND02			
	"R_Arm_Upper_Wound01",	// BONE_R_ARM_UPPER_WOUND01	
	"R_Foot_Front_Wound01",	// BONE_R_FOOT_FRONT_WOUND01	
	"R_Leg_Upper_Wound01",	// BONE_R_LEG_UPPER_WOUND01	
	"R_Leg_Lower_Wound01",	// BONE_R_LEG_LOWER_WOUND01	
	"L_Arm_Upper_Wound01",	// BONE_L_ARM_UPPER_WOUND01	
	"L_Arm_Lower_Wound01",	// BONE_L_ARM_LOWER_WOUND01	
	"Torso_Wound01",		// BONE_TORSO_WOUND01			
	"Torso_Wound02",		// BONE_TORSO_WOUND02			
	"Torso_Wound03",		// BONE_TORSO_WOUND03			
	"Torso_Wound04",		// BONE_TORSO_WOUND04			
	"Torso_Wound05",		// BONE_TORSO_WOUND05			
	"Torso_Wound06",		// BONE_TORSO_WOUND06			
	"Torso_Wound07",		// BONE_TORSO_WOUND07			
	"Torso_Wound08",		// BONE_TORSO_WOUND08			
	"Torso_Wound09",		// BONE_TORSO_WOUND09			
};

cchar *CBotCorrosive::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMZidle001",		// ANIM_STAND,
	"ARMZidle002",		// ANIM_STAND_LOOK,
	"ARMZwalkF01",		// ANIM_WALK,
	"ARMZwalkF02",		// ANIM_WALK_DAMAGED

	"ARMZjump_01",		// ANIM_JUMP_LAUNCH,
	"ARMZjump_02",		// ANIM_JUMP_FLY,
	"ARMZjump_03",		// ANIM_JUMP_LAND

	"ARMZstomp01",		// ANIM_STOMP,
	"ARMZstomp02",		// ANIM_FORWARD_STOMP,
	"ARMZactn201",		// ANIM_FIST_SMASH,
	"ARMZswat_01",		// ANIM_SWAT,
	"ARMZactn101",		// ANIM_BOT_TOSS,
	"ARMZangry01",		// ANIM_BEAT_CHEST,
	"ARMZflick01",		// ANIM_FINGER_FLICK,

	"ARMZunder01",		// ANIM_PEER_UNDER,
	"ARMZchest01",		// ANIM_OPEN_CHEST_DOORS,

	"ARMZidleB01",		// ANIM_STAND_LIMP_LEFT,
	"ARMZhop_l01",		// ANIM_LIMP_LEFT,

	"ARMZdeath01",		// ANIM_DEATH,
	"ARMZpwrdn01",		// ANIM_RC_POWER_DOWN,
	"ARMZpwrup01",		// ANIM_RC_POWER_UP,
};


cchar *CBotCorrosive::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",			// ANIMCONTROL_STAND,
	"stand_look",		// ANIMCONTROL_STAND_LOOK,
	"walk",				// ANIMCONTROL_WALK,
	"damaged_walk",		// ANIMCONTROL_DAMAGED_WALK,

	"jump_launch",		// ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",			// ANIMCONTROL_JUMP_FLY,
	"jump_land_u",		// ANIMCONTROL_JUMP_LAND_UPPER,
	"jump_land_l",		// ANIMCONTROL_JUMP_LAND_LOWER,

	"stomp",			// ANIMCONTROL_STOMP,
	"fwd_stomp",		// ANIMCONTROL_FORWARD_STOMP,
	"fist_smash_u",		// ANIMCONTROL_FIST_SMASH_UPPER,
	"fist_smash_l",		// ANIMCONTROL_FIST_SMASH_LOWER,
	"swat",				// ANIMCONTROL_SWAT,
	"bot_toss",			// ANIMCONTROL_BOT_TOSS,
	"beat_chest",		// ANIMCONTROL_BEAT_CHEST,
	"finger_flick",		// ANIMCONTROL_FINGER_FLICK,

	"peer_under",		// ANIMCONTROL_PEER_UNDER,
	"open_c_doors",		// ANIMCONTROL_OPEN_CHEST_DOORS,

	"stand_limp_l",		// ANIMCONTROL_STAND_LIMP_LEFT,
	"limp_l",			// ANIMCONTROL_LIMP_LEFT,

	"death",			// ANIMCONTROL_DEATH,
	"power_down",		// ANIMCONTROL_RC_POWER_DOWN,
	"power_up",			// ANIMCONTROL_RC_POWER_UP,

	"aim_summer",		// ANIMCONTROL_AIM_SUMMER,
};


// Animation taps:
cchar *CBotCorrosive::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",			// ANIMTAP_STAND,
	"stand_look",		// ANIMTAP_STAND_LOOK,
	"walk",				// ANIMTAP_WALK,
	"damged_walk",		// ANIMTAP_DAMAGED_WALK,

	"jump_launch",		// ANIMTAP_JUMP_LAUNCH,
	"jump_fly",			// ANIMTAP_JUMP_FLY,
	"jump_land_u",		// ANIMTAP_JUMP_LAND_UPPER,
	"jump_land_l",		// ANIMTAP_JUMP_LAND_LOWER,

	"stomp",			// ANIMTAP_STOMP,
	"fwd_stomp",		// ANIMTAP_FORWARD_STOMP,
	"fist_smash_u",		// ANIMTAP_FIST_SMASH_UPPER,
	"fist_smash_l",		// ANIMTAP_FIST_SMASH_LOWER,
	"swat",				// ANIMTAP_SWAT,
	"bot_toss",			// ANIMTAP_BOT_TOSS,
	"beat_chest",		// ANIMTAP_BEAT_CHEST,
	"finger_flick",		// ANIMTAP_FINGER_FLICK,

	"peer_under",		// ANIMTAP_PEER_UNDER,
	"open_c_doors",		// ANIMTAP_OPEN_CHEST_DOORS,

	"stand_limp_l",		// ANIMTAP_STAND_LIMP_LEFT,
	"limp_l",			// ANIMTAP_LIMP_LEFT,

	"death",			// ANIMTAP_DEATH,
	"power_down",		// ANIMTAP_RC_POWER_DOWN,
	"power_up",			// ANIMTAP_RC_POWER_UP,

	"aim_summer",		// ANIMTAP_AIM_SUMMER,
};


cchar *CBotCorrosive::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMZidle001"
};


CFAnimCombinerConfig::ConfigStack_t CBotCorrosive::m_aAnimCombinerConfigStack[] = {
	// from highest to lowest priority

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_DEATH,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_RC_POWER_DOWN,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_PEER_UNDER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FINGER_FLICK,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIST_SMASH_UPPER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_BOT_TOSS,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_SWAT,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_OPEN_CHEST_DOORS,

	CFAnimMixer::TYPE_SUMMER,			ANIMCONTROL_AIM_SUMMER,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_STOMP,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FORWARD_STOMP,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_BEAT_CHEST,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_LAND_UPPER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_FLY,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_LIMP_LEFT,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_DAMAGED_WALK,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_WALK,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIST_SMASH_LOWER,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_STAND_LIMP_LEFT,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_STAND_LOOK,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,			255
};


CFAnimCombinerConfig::ConfigTap_t	CBotCorrosive::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,					ANIMCONTROL_STAND,				1,
	ANIMTAP_STAND_LOOK,				ANIMCONTROL_STAND_LOOK,			1,
	ANIMTAP_WALK,					ANIMCONTROL_WALK,				1,
	ANIMTAP_DAMAGED_WALK,			ANIMCONTROL_DAMAGED_WALK,		1,

	ANIMTAP_JUMP_LAUNCH,			ANIMCONTROL_JUMP_LAUNCH,		1,
	ANIMTAP_JUMP_FLY,				ANIMCONTROL_JUMP_FLY,			1,
	ANIMTAP_JUMP_LAND_UPPER,		ANIMCONTROL_JUMP_LAND_UPPER,	1,
	ANIMTAP_JUMP_LAND_LOWER,		ANIMCONTROL_JUMP_LAND_LOWER,	1,

	ANIMTAP_STOMP,					ANIMCONTROL_STOMP,				1,
	ANIMTAP_FORWARD_STOMP,			ANIMCONTROL_FORWARD_STOMP,		1,
	ANIMTAP_FIST_SMASH_UPPER,		ANIMCONTROL_FIST_SMASH_UPPER,	1,
	ANIMTAP_FIST_SMASH_LOWER,		ANIMCONTROL_FIST_SMASH_LOWER,	1,
	ANIMTAP_SWAT,					ANIMCONTROL_SWAT,				1,
	ANIMTAP_BOT_TOSS,				ANIMCONTROL_BOT_TOSS,			1,
	ANIMTAP_BEAT_CHEST,				ANIMCONTROL_BEAT_CHEST,			1,
	ANIMTAP_FINGER_FLICK,			ANIMCONTROL_FINGER_FLICK,		1,

	ANIMTAP_PEER_UNDER,				ANIMCONTROL_PEER_UNDER,			1,
	ANIMTAP_OPEN_CHEST_DOORS,		ANIMCONTROL_OPEN_CHEST_DOORS,	1,

	ANIMTAP_STAND_LIMP_LEFT,		ANIMCONTROL_STAND_LIMP_LEFT,	1,
	ANIMTAP_LIMP_LEFT,				ANIMCONTROL_LIMP_LEFT,			1,

	ANIMTAP_DEATH,					ANIMCONTROL_DEATH,				1,
	ANIMTAP_RC_POWER_UP,			ANIMCONTROL_RC_POWER_UP,		1,
	ANIMTAP_RC_POWER_DOWN,			ANIMCONTROL_RC_POWER_DOWN,		1,

	ANIMTAP_AIM_SUMMER,				ANIMCONTROL_AIM_SUMMER,			1,

	255,							255,							255
};


CFAnimCombiner::AttachList_t	CBotCorrosive::m_aAnimAttach[] = {
	ANIM_STAND,				ANIMTAP_STAND,
	ANIM_STAND_LOOK,		ANIMTAP_STAND_LOOK,
	ANIM_WALK,				ANIMTAP_WALK,
	ANIM_DAMAGED_WALK,		ANIMTAP_DAMAGED_WALK,

	ANIM_JUMP_LAUNCH,		ANIMTAP_JUMP_LAUNCH,		
	ANIM_JUMP_FLY,			ANIMTAP_JUMP_FLY,			
	ANIM_JUMP_LAND,			ANIMTAP_JUMP_LAND_UPPER,	
	ANIM_JUMP_LAND,			ANIMTAP_JUMP_LAND_LOWER,

	ANIM_STOMP,				ANIMTAP_STOMP,
	ANIM_FORWARD_STOMP,		ANIMTAP_FORWARD_STOMP,
	ANIM_FIST_SMASH,		ANIMTAP_FIST_SMASH_UPPER,
	ANIM_FIST_SMASH,		ANIMTAP_FIST_SMASH_LOWER,
	ANIM_SWAT,				ANIMTAP_SWAT,
	ANIM_BOT_TOSS,			ANIMTAP_BOT_TOSS,
	ANIM_BEAT_CHEST,		ANIMTAP_BEAT_CHEST,
	ANIM_FINGER_FLICK,		ANIMTAP_FINGER_FLICK,

	ANIM_PEER_UNDER,		ANIMTAP_PEER_UNDER,
	ANIM_OPEN_CHEST_DOORS,	ANIMTAP_OPEN_CHEST_DOORS,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMTAP_AIM_SUMMER,

	ANIM_STAND_LIMP_LEFT,	ANIMTAP_STAND_LIMP_LEFT,	
	ANIM_LIMP_LEFT,			ANIMTAP_LIMP_LEFT,			

	ANIM_DEATH,				ANIMTAP_DEATH,			
	ANIM_RC_POWER_UP,		ANIMTAP_RC_POWER_UP,
	ANIM_RC_POWER_DOWN,		ANIMTAP_RC_POWER_DOWN,

	255,					255,
};


const u8 CBotCorrosive::m_aBoneEnableIndices_FullBody[] = {
	BONE_DUMMY,
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_MIDDLE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_MIDDLE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,
	BONE_WAISTA,
	BONE_WAISTB,
	BONE_WAISTC,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,
	BONE_L_HAND_DUMMY,
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_HAND_DUMMY,
	BONE_R_SHOULDERB,
	BONE_R_SHOULDERA,
	BONE_L_SHOULDERA,
	BONE_L_SHOULDERB,
	BONE_NECK_UPPER,
	BONE_HEAD,
	BONE_L_JAW_PISTON_A,
	BONE_R_JAW_PISTON_A,
	BONE_NECK_PISTONB,
	BONE_JAW,
	BONE_L_JAW_PISTON_B,
	BONE_R_JAW_PISTON_B,
	BONE_NECK_PISTONA,
	BONE_CHEST_DOORA,
	BONE_CHEST_DOORB,
	BONE_CHEST_DOORC,
	BONE_CHEST_DOORD,
	BONE_CHEST_DOORE,
	//BONE_PRIMARY_FIRE,
	BONE_WAISTD,

	255
};


const u8 CBotCorrosive::m_anEnableBoneNameIndexTableForSummer_Normal[] = {

	BONE_DUMMY,
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_MIDDLE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_MIDDLE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,
	BONE_WAISTA,
	BONE_WAISTB,
	BONE_WAISTC,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,
	BONE_L_HAND_DUMMY,
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_HAND_DUMMY,
	BONE_R_SHOULDERB,
	BONE_R_SHOULDERA,
	BONE_L_SHOULDERA,
	BONE_L_SHOULDERB,
	BONE_NECK_UPPER,
	BONE_HEAD,
	BONE_L_JAW_PISTON_A,
	BONE_R_JAW_PISTON_A,
	BONE_NECK_PISTONB,
	BONE_JAW,
	BONE_L_JAW_PISTON_B,
	BONE_R_JAW_PISTON_B,
	BONE_NECK_PISTONA,
	BONE_CHEST_DOORA,
	BONE_CHEST_DOORB,
	BONE_CHEST_DOORC,
	BONE_CHEST_DOORD,
	BONE_CHEST_DOORE,
	//BONE_PRIMARY_FIRE,
	BONE_WAISTD,
};



const u8* CBotCorrosive::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND_LOOK,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_WALK,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_DAMAGED_WALK,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_LAUNCH,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_FLY,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_JUMP_LAND_UPPER,
	m_aBoneEnableIndices_UserAnim_LowerBody,	// ANIMTAP_JUMP_LAND_LOWER,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STOMP,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_FORWARD_STOMP,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_FIST_SMASH_UPPER,
	m_aBoneEnableIndices_UserAnim_LowerBody,	// ANIMTAP_FIST_SMASH_LOWER,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_SWAT,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_BOT_TOSS,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_BEAT_CHEST,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_FINGER_FLICK,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_PEER_UNDER,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_OPEN_CHEST_DOORS,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND_LIMP_LEFT,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_LIMP_LEFT,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_DEATH,
	m_aBoneEnableIndices_FullBody,				//	ANIMTAP_RC_POWER_DOWN,
	m_aBoneEnableIndices_FullBody,				//	ANIMTAP_RC_POWER_UP,

	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_AIM_SUMMER,
};												   
												   


const u8* CBotCorrosive::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// Idle 0
};



const u8 CBotCorrosive::m_aBoneEnableIndices_AimSummer[] = 
{
	BONE_HEAD,
	BONE_TORSO,
	255
};


//BONE MASKS FOR USER ANIMS

const u8 CBotCorrosive::m_aBoneEnableIndices_UserAnim_UpperBody[] = {
	BONE_WAISTA,
	BONE_WAISTB,
	BONE_WAISTC,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,
	BONE_L_HAND_DUMMY,
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_HAND_DUMMY,
	BONE_R_SHOULDERB,
	BONE_R_SHOULDERA,
	BONE_L_SHOULDERA,
	BONE_L_SHOULDERB,
	BONE_NECK_UPPER,
	BONE_CHEST_DOORA,
	BONE_CHEST_DOORB,
	BONE_CHEST_DOORC,
	BONE_CHEST_DOORD,
	BONE_CHEST_DOORE,
	//BONE_PRIMARY_FIRE,
	BONE_WAISTD,

	255
};


const u8 CBotCorrosive::m_aBoneEnableIndices_UserAnim_LowerBody[] = {
	BONE_DUMMY,				
	BONE_GROIN,				

	BONE_L_LEG_UPPER,
	BONE_L_LEG_MIDDLE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,

	BONE_R_LEG_UPPER,
	BONE_R_LEG_MIDDLE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,

	255
};

const u8 CBotCorrosive::m_aBoneEnableIndices_UserAnim_UpperTorso[] = {
	BONE_TORSO,				
	BONE_R_SHOULDERB,
	BONE_R_SHOULDERA,
	BONE_L_SHOULDERA,
	BONE_L_SHOULDERB,
	BONE_NECK_UPPER,
	BONE_NECK_PISTONA,
	BONE_CHEST_DOORA,

	BONE_CHEST_DOORB,
	BONE_CHEST_DOORC,
	BONE_CHEST_DOORD,
	BONE_CHEST_DOORE,
	//BONE_PRIMARY_FIRE,

	255,
};

const u8 CBotCorrosive::m_aBoneEnableIndices_UserAnim_LowerTorso[] = {
	BONE_DUMMY,				
	BONE_GROIN,				

	BONE_R_LEG_UPPER,
	BONE_L_LEG_UPPER,

	BONE_WAISTA,
	BONE_WAISTB,
	BONE_WAISTC,
	BONE_WAISTD,

	255
};

const u8 CBotCorrosive::m_aBoneEnableIndices_UserAnim_LeftArm[] = {
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_INDEXFINGERA,
	BONE_L_INDEXFINGERB,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_PINKYFINGERA,
	BONE_L_PINKYFINGERB,
	BONE_L_THUMBA,
	BONE_L_THUMBB,
	BONE_L_HAND_DUMMY,

	255
};


const u8 CBotCorrosive::m_aBoneEnableIndices_UserAnim_RightArm[] = {
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_R_THUMBA,
	BONE_R_THUMBB,
	BONE_R_MIDDLEFINGERA,
	BONE_R_MIDDLEFINGERB,
	BONE_R_PINKYFINGERA,
	BONE_R_PINKYFINGERB,
	BONE_R_INDEXFINGERA,
	BONE_R_INDEXFINGERB,
	BONE_R_HAND_DUMMY,

	255
};

const u8 CBotCorrosive::m_aBoneEnableIndices_UserAnim_Head[] = {
	BONE_HEAD,
	BONE_L_JAW_PISTON_A,
	BONE_R_JAW_PISTON_A,
	BONE_NECK_PISTONB,
	BONE_JAW,
	BONE_L_JAW_PISTON_B,
	BONE_R_JAW_PISTON_B,
	255
};

const u8 CBotCorrosive::m_nApproxEyePointBoneNameIndex = BONE_HEAD;


const u8 CBotCorrosive::m_anTagPointBoneNameIndexArray[] = {
	BONE_HEAD,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_R_ARM_UPPER,

	255
};


BOOL CBotCorrosive::_BuildAnimStackDef( void ) {
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount					= CBotAnimStackDef::ANIM_USER_COUNT;
	AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable		= m_apszBaseAnimNameTable;
	AnimStackDefInit.nIdleCount					= ANIM_IDLE_COUNT;
	AnimStackDefInit.apszIdleAnimNameTable		= m_apszIdleAnimNameTable;
	AnimStackDefInit.nBaseControlCount			= ANIMCONTROL_BASE_COUNT;
	AnimStackDefInit.apszBaseControlNameTable	= m_apszBaseControlNameTable;
	AnimStackDefInit.nBaseTapCount				= ANIMTAP_BASE_COUNT;
	AnimStackDefInit.apszBaseTapNameTable		= m_apszBaseTapNameTable;
	AnimStackDefInit.nBoneCount					= BONE_COUNT;
	AnimStackDefInit.apszBoneNameTable			= m_apszBoneNameTable;

	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap	= m_apnEnableBoneNameIndexTableForEachBaseTap;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap	= m_apnEnableBoneNameIndexTableForEachIdleTap;
	AnimStackDefInit.pnEnableBoneNameIndexTableForSummer		= m_anEnableBoneNameIndexTableForSummer_Normal;

	AnimStackDefInit.pBaseAnimConfigNet					= NULL;
	AnimStackDefInit.pBaseAnimConfigStack				= m_aAnimCombinerConfigStack;
	AnimStackDefInit.pBaseAnimConfigTap					= m_aAnimCombinerConfigTaps;
	AnimStackDefInit.pBaseAnimAttachList				= m_aAnimAttach;
	AnimStackDefInit.nIdleStackConnectionControlIndex	= ANIMCONTROL_STAND;
	AnimStackDefInit.nIdleStackConnectionControlInput	= 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
}


//--------------------------------------------------------------------------------------------------------
// Game Data Vocab:
//--------------------------------------------------------------------------------------------------------

const FGameData_TableEntry_t CBotCorrosive::m_aBotInfoVocab_Corrosive[] = {

	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileRocketAI
	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileRocketPlayer
	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileFistSmash
	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileFootStep
	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileFootStomp
	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileFootStepRumble
	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileFootStompRumble
	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileDeathCrashRumble

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hRocketExplosion
	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hLimbExplosion
	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hDeathExplosion

	//
	//
	////ROCKET PARAMS
	//cchar *pszRocketSmokeTex;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//f322	fRocketReleaseAnimTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32 fRocketRange;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32	fRocketSpeed;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// pszCoronaTexName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fCoronaScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_100000,

	// fCoronaUnitScreenspaceScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_100000,

	// fCoronaColorRed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fCoronaColorGreen:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fCoronaColorBlue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fLightRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_1000,

	// fProjectileMeshScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_1000,

	EPROJ_MERV_STATICPARAMS_GAMEDATA_VOCAB,	// MervStaticParams

	// fFistSmashDamageSphereRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_1000,

	// fSwatDamageSphereRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_1000,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAngryRoar
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupLaugh
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFistSmashGrunt
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGrouppSmashCrash
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupDeath1
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupDeath2
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupDeathCrash
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupRocketLauncher

	// pszBotTossParticleFXResName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fBotTossFXIntensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_100000,

	// pszVSpotHitParticleFXResName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotCorrosive::m_aGameDataMap[] = {
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

	"Corrosive",
	m_aBotInfoVocab_Corrosive,
	sizeof(m_BotInfo_Corrosive),
	(void*)&m_BotInfo_Corrosive,

	NULL
};
