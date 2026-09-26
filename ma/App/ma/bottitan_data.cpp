//////////////////////////////////////////////////////////////////////////////////////
// bottitan_data.cpp - Bot data.
//
// Author: Mike Elliott  
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
// 10/14/02 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "bottitan.h"
#include "botanim.h"
#include "fmesh.h"
#include "eproj.h"


const CBotDef CBotTitan::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_TITAN,							// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CBotTitan::m_apszBoneNameTable[BONE_COUNT] = {
	"Groin",					// BONE_GROIN
	"L_Leg_Upper",				// BONE_L_LEG_UPPER,
	"L_Leg_Lower",				// BONE_L_LEG_LOWER,
	"L_Foot_Back",				// BONE_L_FOOT_BACK
	"L_Heel",					// BONE_L_HEEL,
	"L_Foot_Front",				// BONE_L_FOOT_FRONT
	"L_Toe",					// BONE_L_TOE

	"R_Leg_Upper",				// BONE_R_LEG_UPPER
	"R_Leg_Lower",				// BONE_R_LEG_LOWER
	"R_Foot_Back",				// BONE_R_FOOT_BACK
	"R_Heel",					// BONE_R_HEEL
	"R_Foot_Front",				// BONE_R_FOOT_FRONT,
	"R_Toe",					// BONE_R_TOE,

	"Torso",					// BONE_TORSO,

	"R_Rib_Middle",				// BONE_R_RIB_MIDDLE,
	"R_Rib_Bottom",				// BONE_R_RIB_BOTTOM,
	"R_Rib_Top",				// BONE_R_RIB_TOP,

	"Head",						// BONE_HEAD,
	"Reactor",					// BONE_REACTOR,

	"RocketLauncher",			// BONE_ROCKETLAUNCHER,
	"RocketCartridge",			// BONE_ROCKETCARTRIDGE,
	"Secondary_Fire",			// BONE_SECONDARY_FIRE,

	"R_Arm_Upper",				// BONE_R_ARM_UPPER,
	"R_Elbow",					// BONE_R_ELBOW,
	"R_Arm_Lower",				// BONE_R_ARM_LOWER,

	"L_Arm_Upper",				// BONE_L_ARM_UPPER,
	"L_Elbow",					// BONE_L_ELBOW,
	"L_Arm_Lower",				// BONE_L_ARM_LOWER

	"L_Rib_Middle",				// BONE_L_RIB_MIDDLE,
	"L_Rib_Top",				// BONE_L_RIB_TOP,
	"L_Rib_Bottom",				// BONE_L_RIB_BOTTOM,

	"Tether_Port",				// BONE_TETHER_PORT
	"Spotlight",				// BONE_SPOTLIGHT,
	"AttachPoint_Light",		// BONE_ATTACHPOINT_LIGHT;
};


cchar *CBotTitan::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMTidle001",	// ANIM_STAND,
	"ARMTidle002",	// ANIM_STAND_STAMP,
	"ARMTidle003",	// ANIM_STAND_LOOK,
	"ARMTwalkF01",	// ANIM_WALK,
	"ARMTrun_F01",  // ANIM_RUN,

	"ARMTjump_01",	// ANIM_JUMP_LAUNCH,
	"ARMTjump_02",	// ANIM_JUMP_FLY,
	"ARMTjump_03",	// ANIM_JUMP_LAND,
		
	"ARMTjump201",	// ANIMCONTROL_JUMP2_FLY,
	"ARMTjump202",	// ANIMCONTROL_JUMP2_LAND,

	"ARMTactn101",	// ANIM_FIRE_1,
	"ARMTactn201",	// ANIM_FIRE_2,

	"ARMTelect01",	// ANIM_RC_TETHERED
	"ARMTpwrdn01",	// ANIM_RC_POWER_DOWN,
	"ARMTpwrup01",	// ANIM_RC_POWER_UP,
	"ARMTidlel01",	// ANIM_STAND_LIMP_LEFT,
	"ARMTidler01",	// ANIM_STAND_LIMP_RIGHT,
	"ARMThop_l01",	// ANIM_LIMP_LEFT,
	"ARMThop_r01",	// ANIM_LIMP_RIGHT,

};

cchar *CBotTitan::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMTidle001"
};


cchar *CBotTitan::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",		//	ANIMCONTROL_STAND,
	"stand_stamp",	//	ANIMCONTROL_STAND_STAMP,
	"stand_look",	//	ANIMCONTROL_STAND_LOOK,
	"walk",			//	ANIMCONTROL_WALK,
	"run",			//	ANIMCONTROL_RUN,

	"aim_summer",	//	ANIMCONTROL_AIM_SUMMER

	"jump_launch",	//	ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",		//	ANIMCONTROL_JUMP_FLY,
	"jump_land_L",	//	ANIMCONTROL_JUMP_LAND_LOWER,
	"jump_land_U",	//	ANIMCONTROL_JUMP_LAND_UPPER,
		
	"jump2_launch",	//	ANIMCONTROL_JUMP2_FLY,
	"jump2_land",	//	ANIMCONTROL_JUMP2_LAND,

	"fire1",		//	ANIMCONTROL_FIRE_1,
	"fire2",		//	ANIMCONTROL_FIRE_2,

	"tethered",		//  ANIMCONTROL_RC_TETHERED
	"power_down",	//	ANIMCONTROL_RC_POWER_DOWN,
	"power_up",		//	ANIMCONTROL_RC_POWER_UP,

	"stoop",		//  ANIMCONTROL_STOOP_SUMMER

	"stand_limp_l",	// ANIM_STAND_LIMP_LEFT,
	"stand_limp_r",	// ANIM_STAND_LIMP_RIGHT,
	"limp_l",		// ANIM_LIMP_LEFT,
	"limp_r",		// ANIM_LIMP_RIGHT,
};



cchar *CBotTitan::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",		//	ANIMTAP_STAND,
	"stand_stamp",	//	ANIMTAP_STAND_STAMP,
	"stand_look",	//	ANIMTAP_STAND_LOOK,
	"walk",			//	ANIMTAP_WALK,
	"run",			//	ANIMTAP_RUN,

	"aim_summer",	//  ANIMTAP_AIM_SUMMER

	"jump_launch",	//	ANIMTAP_JUMP_LAUNCH,
	"jump_fly",		//	ANIMTAP_JUMP_FLY,
	"jump_land_L",	//	ANIMTAP_JUMP_LAND_LOWER,
	"jump_land_U",	//	ANIMTAP_JUMP_LAND_UPPER,
		
	"jump2_launch",	//	ANIMTAP_JUMP2_FLY,
	"jump2_land",	//	ANIMTAP_JUMP2_LAND,

	"fire1",		//	ANIMTAP_FIRE_1,
	"fire2",		//	ANIMTAP_FIRE_2,

	"tethered",		//  ANIMTAP_RC_TETHERED"
	"power_down",	//	ANIMTAP_RC_POWER_DOWN,
	"power_up",		//	ANIMTAP_RC_POWER_UP,

	"stoop",		//  ANIMTAP_STOOP_SUMMER

	"stand_limp_l",	// ANIM_STAND_LIMP_LEFT,
	"stand_limp_r",	// ANIM_STAND_LIMP_RIGHT,
	"limp_l",		// ANIM_LIMP_LEFT,
	"limp_r",		// ANIM_LIMP_RIGHT,
};


CFAnimCombinerConfig::ConfigStack_t CBotTitan::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_TETHERED,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_DOWN,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_1,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_2,

	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_STOOP_SUMMER,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAUNCH,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP2_FLY,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP2_LAND,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_FLY,

	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_LIMP_LEFT,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_LIMP_RIGHT,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_LIMP_LEFT,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_LIMP_RIGHT,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_STAMP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_LOOK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CBotTitan::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_FIRE_1,							ANIMCONTROL_FIRE_1,				1,
	ANIMTAP_FIRE_2,							ANIMCONTROL_FIRE_2,				1,

	ANIMTAP_STOOP_SUMMER,					ANIMCONTROL_STOOP_SUMMER,		1,
	
	ANIMTAP_JUMP_LAUNCH,					ANIMCONTROL_JUMP_LAUNCH,		1,
	ANIMTAP_JUMP_LAND_LOWER,				ANIMCONTROL_JUMP_LAND_LOWER,	1,
	ANIMTAP_JUMP_LAND_UPPER,				ANIMCONTROL_JUMP_LAND_UPPER,	1,

	ANIMTAP_JUMP2_LAND,						ANIMCONTROL_JUMP2_LAND,			1,
	ANIMTAP_JUMP2_FLY,						ANIMCONTROL_JUMP2_FLY,			1,
	ANIMTAP_JUMP_FLY,						ANIMCONTROL_JUMP_FLY,			1,

	ANIMTAP_AIM_SUMMER,						ANIMCONTROL_AIM_SUMMER,			1,

	ANIMTAP_RUN,							ANIMCONTROL_RUN,				1,
	ANIMTAP_WALK,							ANIMCONTROL_WALK,				1,
	ANIMTAP_RC_TETHERED,					ANIMCONTROL_RC_TETHERED,		1,
	ANIMTAP_RC_POWER_UP,					ANIMCONTROL_RC_POWER_UP,		1,
	ANIMTAP_RC_POWER_DOWN,					ANIMCONTROL_RC_POWER_DOWN,		1,
	ANIMTAP_STAND_STAMP,					ANIMCONTROL_STAND_STAMP,		1,
	ANIMTAP_STAND_LOOK,						ANIMCONTROL_STAND_LOOK,			1,
	ANIMTAP_STAND,							ANIMCONTROL_STAND,				1,
	ANIMTAP_STAND_LIMP_LEFT,				ANIMCONTROL_STAND_LIMP_LEFT,	1,
	ANIMTAP_STAND_LIMP_RIGHT,				ANIMCONTROL_STAND_LIMP_RIGHT,	1,
	ANIMTAP_LIMP_LEFT,						ANIMCONTROL_LIMP_LEFT,			1,
	ANIMTAP_LIMP_RIGHT,						ANIMCONTROL_LIMP_RIGHT,			1,

	255,									255,							255
};


CFAnimCombiner::AttachList_t CBotTitan::m_aAnimAttach[] = {
	ANIM_FIRE_1,							ANIMTAP_FIRE_1,
	ANIM_FIRE_2,							ANIMTAP_FIRE_2,
	
	ANIM_JUMP_LAUNCH,						ANIMTAP_JUMP_LAUNCH,
	ANIM_JUMP_LAND,							ANIMTAP_JUMP_LAND_LOWER,
	ANIM_JUMP_LAND,							ANIMTAP_JUMP_LAND_UPPER,
	ANIM_JUMP2_FLY,							ANIMTAP_JUMP2_FLY,
	ANIM_JUMP2_LAND,						ANIMTAP_JUMP2_LAND,
	ANIM_JUMP_FLY,							ANIMTAP_JUMP_FLY,
    
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMTAP_STOOP_SUMMER,
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMTAP_AIM_SUMMER,

	ANIM_RUN,								ANIMTAP_RUN,
	ANIM_WALK,								ANIMTAP_WALK,
	ANIM_RC_TETHERED,						ANIMTAP_RC_TETHERED,
	ANIM_RC_POWER_UP,						ANIMTAP_RC_POWER_UP,
	ANIM_RC_POWER_DOWN,						ANIMTAP_RC_POWER_DOWN,
	ANIM_STAND_STAMP,						ANIMTAP_STAND_STAMP,
	ANIM_STAND_LOOK,						ANIMTAP_STAND_LOOK,
	ANIM_STAND,								ANIMTAP_STAND,
	ANIM_STAND_LIMP_LEFT,					ANIMTAP_STAND_LIMP_LEFT,	
	ANIM_STAND_LIMP_RIGHT,					ANIMTAP_STAND_LIMP_RIGHT,	
	ANIM_LIMP_LEFT,							ANIMTAP_LIMP_LEFT,			
	ANIM_LIMP_RIGHT,						ANIMTAP_LIMP_RIGHT,			

	255,									255
};


const u8 CBotTitan::m_aBoneEnableIndices_FullBody[] = {
	BONE_GROIN,

	BONE_L_LEG_UPPER,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,

	BONE_R_LEG_UPPER,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,

	BONE_TORSO,

	BONE_R_RIB_MIDDLE,
	BONE_R_RIB_BOTTOM,
	BONE_R_RIB_TOP,

	BONE_HEAD,
	BONE_REACTOR,

	BONE_ROCKETLAUNCHER,
	BONE_ROCKETCARTRIDGE,
	BONE_SECONDARY_FIRE,
	
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,

	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,

	BONE_L_RIB_MIDDLE,
	BONE_L_RIB_TOP,
	BONE_L_RIB_BOTTOM,

	BONE_TETHER_PORT,
	BONE_SPOTLIGHT,
	BONE_ATTACHPOINT_LIGHT,

	255 
};


const u8 CBotTitan::m_aBoneEnableIndices_AimSummer[] = { 
	BONE_HEAD, 
	BONE_TORSO, 
	BONE_R_ELBOW,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	BONE_R_ARM_LOWER,
	255 
};


const u8 *CBotTitan::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_STAND_STAMP,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_STAND_LOOK,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_WALK,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_RUN,
	m_anEnableBonesUpperBody,			//  ANIMTAP_AIM_SUMMER
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_JUMP_LAUNCH,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_JUMP_FLY,
	m_anEnableBonesLand_Lower,			//	ANIMTAP_JUMP_LAND_LOWER,
	m_anEnableBonesLand_Upper,			//	ANIMTAP_JUMP_LAND_UPPER,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_JUMP2_FLY,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_JUMP2_LAND,
	m_anEnableBonesFire,				//	ANIMTAP_FIRE_1,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_FIRE_2,
	m_aBoneEnableIndices_FullBody,		//  ANIMTAP_RC_TETHERED"
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_RC_POWER_DOWN,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_RC_POWER_UP,
	m_aBoneEnableIndices_FullBody,		//  ANIMTAP_STOOP_SUMMER
};


cchar *CBotTitan::m_papszStoopSummerBones[] = {
	"Torso",		 		//_STOOP_BONE_TORSO,			
	"Reactor",		 		//_STOOP_BONE_REACTOR,		
	"L_Leg_Upper",	 		//_STOOP_BONE_LLEG_UPPER,		
	"R_Leg_Upper",	 		//_STOOP_BONE_RLEG_UPPER,		
	"L_Leg_Lower",	 		//_STOOP_BONE_LLEG_LOWER,		
	"R_Leg_Lower",	 		//_STOOP_BONE_RLEG_LOWER,		
	"Groin",		 		//_STOOP_BONE_GROIN,		
	"RocketLauncher",		//_STOOP_BONE_ROCKETLAUNCHER,
	"L_Arm_Upper",			//_STOOP_BONE_LARM_UPPER,
	"R_Arm_Upper",			//_STOOP_BONE_RARM_UPPER,
	"R_Foot_Front",			//_STOOP_BONE_RFOOT_FRONT,
	"R_Foot_Back",			//_STOOP_BONE_RFOOT_BACK,
	"R_Heel",				//_STOOP_BONE_RHEEL
	"L_Foot_Front",			//_STOOP_BONE_LFOOT_FRONT,		
	"L_Foot_Back",			//_STOOP_BONE_LFOOT_BACK,
	"L_Heel",				//_STOOP_BONE_LHEEL,
	"Spotlight",			//_STOOP_BONE_SPOTLIGHT,
	"Head",					//_STOOP_BONE_HEAD
	NULL
};


const u8 *CBotTitan::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// Idle 0
};

const u8 CBotTitan::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};

const u8 CBotTitan::m_anEnableBonesLand_Upper[] = {
	BONE_TORSO,

	BONE_R_RIB_MIDDLE,
	BONE_R_RIB_BOTTOM,
	BONE_R_RIB_TOP,

	BONE_HEAD,
	BONE_REACTOR,

	BONE_ROCKETLAUNCHER,
	BONE_ROCKETCARTRIDGE,
	BONE_SECONDARY_FIRE,

	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,

	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,

	BONE_L_RIB_MIDDLE,
	BONE_L_RIB_TOP,
	BONE_L_RIB_BOTTOM,

	255
};

const u8 CBotTitan::m_anEnableBonesLand_Lower[] = {
	BONE_GROIN,

	BONE_L_LEG_UPPER,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,

	BONE_R_LEG_UPPER,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,

//	BONE_TETHER_PORT,
	255
};

const u8 CBotTitan::m_anEnableBonesUpperBody[] = {
	BONE_TORSO,

//	BONE_GROIN,

	BONE_R_RIB_MIDDLE,
	BONE_R_RIB_BOTTOM,
	BONE_R_RIB_TOP,

	BONE_HEAD,
	BONE_REACTOR,

	BONE_ROCKETLAUNCHER,
	BONE_ROCKETCARTRIDGE,
	BONE_SECONDARY_FIRE,

	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,

	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,

	BONE_L_RIB_MIDDLE,
	BONE_L_RIB_TOP,
	BONE_L_RIB_BOTTOM,

	255
};

const u8 CBotTitan::m_anEnableBonesFire[] = {
	BONE_R_ARM_UPPER,
	BONE_L_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_L_ELBOW,
	BONE_R_ARM_LOWER,
	BONE_L_ARM_LOWER,
	255
};
//const u8 CBotTitan::m_anEnableBoneNameIndexTableForSummer_TetherShock[] = { 255 };

const u8 CBotTitan::m_anEnableBoneNameIndexTableForSummer_TetherShock[] = {
	// Enable these bones to be driven by our summer:

	BONE_GROIN,				
	BONE_L_LEG_UPPER,
	BONE_L_LEG_LOWER,		
	BONE_L_FOOT_BACK,		
	BONE_L_HEEL,			
	BONE_L_FOOT_FRONT,		
	BONE_L_TOE,
							
	BONE_R_LEG_UPPER,
	BONE_R_LEG_LOWER,		
	BONE_R_FOOT_BACK,		
	BONE_R_HEEL,				
	BONE_R_FOOT_FRONT,		
	BONE_R_TOE,	
							
	BONE_TORSO,
							
	BONE_R_RIB_MIDDLE,
	BONE_R_RIB_BOTTOM,		
	BONE_R_RIB_TOP,			
							
	BONE_HEAD,
	BONE_REACTOR,			
							
	BONE_ROCKETLAUNCHER,
	BONE_ROCKETCARTRIDGE,	
	BONE_SECONDARY_FIRE,	
							
	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,			
	BONE_R_ARM_LOWER,		
							
	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,			
	BONE_L_ARM_LOWER,		
							
	BONE_L_RIB_MIDDLE,
	BONE_L_RIB_TOP,			
	BONE_L_RIB_BOTTOM,		
							
//	BONE_TETHER_PORT,
	BONE_SPOTLIGHT,			
	BONE_ATTACHPOINT_LIGHT,

	255
};



//BONE MASKS FOR USER ANIMS

const u8 CBotTitan::m_aBoneEnableIndices_UserAnim_UpperBody[] = {

	BONE_TORSO,

	BONE_R_RIB_MIDDLE,
	BONE_R_RIB_BOTTOM,
	BONE_R_RIB_TOP,

	BONE_HEAD,
	BONE_REACTOR,

	BONE_ROCKETLAUNCHER,
	BONE_ROCKETCARTRIDGE,
	BONE_SECONDARY_FIRE,

	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,

	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,

	BONE_L_RIB_MIDDLE,
	BONE_L_RIB_TOP,
	BONE_L_RIB_BOTTOM,

	BONE_SPOTLIGHT,
	BONE_ATTACHPOINT_LIGHT,
	255
};


const u8 CBotTitan::m_aBoneEnableIndices_UserAnim_LowerBody[] = {
	BONE_GROIN,

	BONE_L_LEG_UPPER,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT_BACK,
	BONE_L_HEEL,
	BONE_L_FOOT_FRONT,
	BONE_L_TOE,

	BONE_R_LEG_UPPER,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT_BACK,
	BONE_R_HEEL,
	BONE_R_FOOT_FRONT,
	BONE_R_TOE,

//	BONE_TETHER_PORT,
	255
};


const u8 CBotTitan::m_aBoneEnableIndices_UserAnim_UpperTorso[] = {
	
	BONE_TORSO,

	BONE_R_RIB_MIDDLE,
	BONE_R_RIB_BOTTOM,
	BONE_R_RIB_TOP,

	BONE_REACTOR,

	BONE_ROCKETLAUNCHER,
	BONE_ROCKETCARTRIDGE,
	BONE_SECONDARY_FIRE,

	BONE_SPOTLIGHT,
	BONE_ATTACHPOINT_LIGHT,
	255
};


const u8 CBotTitan::m_aBoneEnableIndices_UserAnim_LowerTorso[] = {
	BONE_GROIN,
	BONE_L_LEG_UPPER,
	BONE_R_LEG_UPPER,
//	BONE_TETHER_PORT,
	255
};


const u8 CBotTitan::m_aBoneEnableIndices_UserAnim_LeftArm[] = {

	BONE_L_ARM_UPPER,
	BONE_L_ELBOW,
	BONE_L_ARM_LOWER,
	255
};


const u8 CBotTitan::m_aBoneEnableIndices_UserAnim_RightArm[] = {

	BONE_R_ARM_UPPER,
	BONE_R_ELBOW,
	BONE_R_ARM_LOWER,
	255
};


const u8 CBotTitan::m_aBoneEnableIndices_UserAnim_Head[] = {
	BONE_HEAD,
	255
};



const u8 CBotTitan::m_nApproxEyePointBoneNameIndex = BONE_HEAD;

const u8 CBotTitan::m_anTagPointBoneNameIndexArray[] = {
	BONE_HEAD,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_R_ARM_UPPER,

	255
};

//const u32 CBotTitan::m_auRocketTexLayerIDs[ROCKET_NUM_CHAMBERS] = {
//	80,
//	81,
//	82
//};




//--------------------------------------------------------------------------------------------------------
// Game Data Vocab:
//--------------------------------------------------------------------------------------------------------

const FGameData_TableEntry_t CBotTitan::m_aBotInfoVocab_Titan[] = {
	// pPCRocketDamageProfile:
	FGAMEDATA_VOCAB_DAMAGE,

	// pNPCRocketDamageProfile:
	FGAMEDATA_VOCAB_DAMAGE,
	
	// pszFootstepDamageProfile;
	FGAMEDATA_VOCAB_DAMAGE,

	// pStompDamageProfile
	FGAMEDATA_VOCAB_DAMAGE,

	//pszParticleSparks;	
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//pszParticleFootDust;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//cchar *pszSpotlightMesh;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//f32	fUnitTopSpeedWhileFiring;			// Top speed while the bot is firing guns or stooping
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//f32 fStompGravityMultiplier;			// Gravity multiplier to pull bot down while stomping
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//
	//
	////ROCKET PARAMS
	//cchar *pszRocketMesh;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//cchar *pszRocketSmokeTex;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//cchar *pszRocketExplosionImpact;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//cchar *pszRocketExplosionFire;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//u32	nRocketsInPool;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32	fOORocketReloadTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32 fRocketRippleDelay;					// time to wait between firing multiple rockets
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32 fRocketFullReloadTime;				// time it takes to reload after you fire multiple rockets
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

	//f32	fRocketAimTime, fOORocketAimTime;	// time it takes to orient the rl at the target
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_10000,
	
	//f32 fRocketAimOffTime, fOORocketAimOffTime;		// time it takes to revert after firing
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32 fShieldImpactSoundVolume;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//f32 fShieldImpactSoundInterval;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//f32 fLightInactivityTime;				// max time the titan can be inactive before the light shuts down
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//// shield params
	//f32 fShieldRechargeTime;				// time it takes to charge the shield
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32 fShieldRechargeDelay;				// time to wait after taking damage before recharging
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32	fShieldScale;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//cchar *pszShieldArmorProfile;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//f32 fMinPowerCycle;						// slowest speed the reactor cycles (cycles per second)
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	//f32 fMaxPowerCycle;						// fastest speed
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,
	
	//f32 fReactorSoundLevel;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//f32 fSoundRadius;				
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,


	//fUnitInPossessionCamShake;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

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

	// cchar *pszDebrisPileMeshExplosion;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,


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

	FGAMEDATA_VOCAB_SOUND_GROUP,			// pSndGrpRocketLauncher
	FGAMEDATA_VOCAB_SOUND_GROUP,			// pSndGrpRocketLoop

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};



const FGameData_TableEntry_t CBotTitan::m_aBotInfoVocab_TitanSounds[] = {
	//FSndFx_FxHandle_t hSndRLReady;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hSndRLLaunch;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hSndStomp;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hSndPlantLoop;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hSndShieldHit;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hSndShieldUp;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
	
	//FSndFx_FxHandle_t hSndShieldDown;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t	hSndPowerUp;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t	hSndPowerDown;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t	hSndServo;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};





//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotTitan::m_aGameDataMap[] = {
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

	"Titan",
	m_aBotInfoVocab_Titan,
	sizeof(m_BotInfo_Titan),
	(void *)&m_BotInfo_Titan,

	NULL
};

const FGameDataMap_t CBotTitan::m_aSoundDataMap[] = {
	"Sounds",
	m_aBotInfoVocab_TitanSounds,
	sizeof(m_BotInfo_TitanSounds), 
	(void *)&m_BotInfo_TitanSounds,
	
	NULL,
};



BOOL CBotTitan::_BuildAnimStackDef( void ) {
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