//////////////////////////////////////////////////////////////////////////////////////
// botjumper_data.cpp - Jump Trooper bot data
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
// 02/09/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botjumper.h"
#include "botanim.h"
#include "fmesh.h"

const CBotDef CBotJumper::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_JUMP_TROOPER,					// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CBotJumper::m_apszBoneNameTable[BONE_COUNT] = {
	"Groin",				// BONE_GROIN,			
	"Torso",				// BONE_TORSO,			
	"Head",					// BONE_HEAD,

	"Chest_Top",			// BONE_CHEST_TOP,		
	"Chest_Bottom",			// BONE_CHEST_BOTTOM,	
	"Chest_Middle",			// BONE_CHEST_MIDDLE,	

	"L_Arm_Upper",			// BONE_ARM_UPPER_L,	
	"L_Arm_Lower",			// BONE_ARM_LOWER_L,	
	"L_Gun_Barrel2",		// BONE_GUN_2_L		
	"L_Gun_Barrel1",		// BONE_GUN_1_L,		
	"L_Arm_Armor",			// BONE_ARM_ARMOR_L,
	"Primary_Fire_L1",		// BONE_PRIFIRE_1_L,		
	"Primary_Fire_L2",		// BONE_PRIFIRE_2_L,		


	"R_Arm_Upper",			// BONE_ARM_UPPER_R,	
	"R_Arm_Lower",			// BONE_ARM_LOWER_R,	
	"R_Gun_Barrel2",		// BONE_GUN_2_R,		
	"R_Gun_Barrel1",		// BONE_GUN_1_R,		
	"R_Arm_Armor",			// BONE_ARM_ARMOR_R,	
	"Primary_Fire_R1",		// BONE_PRIFIRE_1_R,		
	"Primary_Fire_R2",		// BONE_PRIFIRE_2_R,		

	"L_Wing",				// BONE_WING_L,		
	"R_Wing",				// BONE_WING_R,		

	"L_Leg_Upper",			// BONE_LEG_UPPER_L,	
	"L_Kneepad",			// BONE_LEG_KNEE_L,	
	"L_Leg_Lower",			// BONE_LEG_LOWER_L,	
	"L_Ankle",				// BONE_LEG_ANGKLE_L,	
	"L_Foot_Back",			// BONE_FOOT_BACK_L,	
	"Left_Foot_Blade",		// BONE_FOOT_BLADE_L,	
	"L_Foot_Front",			// BONE_FOOT_FRONT_L,	
	"L_Toe1",				// BONE_TOE1_L,		
	"L_Toe2",				// BONE_TOE2_L,		

	"R_Leg_Upper",			// BONE_LEG_UPPER_R,	
	"R_Kneepad",			// BONE_LEG_KNEE_R,	
	"R_Leg_Lower",			// BONE_LEG_LOWER_R,	
	"R_Ankle",				// BONE_LEG_ANGKLE_R,	
	"R_Foot_Front",			// BONE_FOOT_BACK_R,	
	"R_Toe1",				// BONE_FOOT_BLADE_R,	
	"R_Toe2",				// BONE_FOOT_FRONT_R,	
	"R_Foot_Back",			// BONE_TOE1_R,		
	"R_Foot_Blade",			// BONE_TOE2_R,		
	"JumpTrooperDummy",		// BONE_DUMMY
};


cchar* CBotJumper::m_apszSummerBoneNames[SUMMERBONE_COUNT] = {
	"L_Arm_Upper",	//SUMMERBONE_ARM_UPPER_L,
//	"L_Arm_Lower",	//SUMMERBONE_ARM_LOWER_L,
	"R_Arm_Upper",	//SUMMERBONE_ARM_UPPER_R,
//	"R_Arm_Lower",	//SUMMERBONE_ARM_LOWER_R,
//	"Torso",		//SUMMERBONE_TORSO,
//	"Head",			//SUMMERBONE_HEAD,
};



cchar *CBotJumper::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
    "ARMJidle001",	// ANIM_STAND,
	"ARMJwalkf01",	// ANIM_WALK,
	"ARMJrun_F02",	// ANIM_RUN,

	"ARMJjump_01",	// ANIM_JUMP_LAUNCH,
	"ARMJjump201",	// ANIM_JUMP_FLY_JETS,
	"ARMJjump_02",	// ANIM_JUMP_FLY,
	"ARMJjump_03",	// ANIM_JUMP_LAND,

	"ARMJactn101",	// ANIM_FIRE_1,
	"ARMJactn201",	// ANIM_ATTACK_START,
	"ARMJactn202",	// ANIM_ATTACK,
	"ARMJactn203",	// ANIM_ATTACK_END,

	"ARMJhop_l01",	// ANIM_HOP_LEFT,
	"ARMJhop_r01",	// ANIM_HOP_RIGHT,

	"ARMJelect01",	// ANIM_RC_TETHERED,
	"ARMJpwrdn01",	// ANIM_RC_POWER_DOWN,
	"ARMJpwrup01",	// ANIM_RC_POWER_UP,
	"ARMJpwrdn02",	// ANIMTAP_RC_POWER_DOWN_INV,
	"ARMJpwrup02",	// ANIMTAP_RC_POWER_UP_INV,

	"ARMJdrop_01",	// ANIM_DROP,
	"ARMJwake_01",	// ANIM_WAKE,
	"ARMJsleep01",	// ANIM_SLEEP,

	"ARMJdive_01",	// ANIM_DIVE,
	"ARMJdive_02",	// ANIM_DIVE_RECOVER,
};


cchar *CBotJumper::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",			// ANIMCONTROL_STAND,
	"walk",				// ANIMCONTROL_WALK,
	"run",				// ANIMCONTROL_RUN,
	
	"jump_launch",		// ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly_jets",	// ANIMCONTROL_JUMP_FLY_JETS,
	"jump_fly",			// ANIMCONTROL_JUMP_FLY,
	"jump_land_u",		// ANIMCONTROL_JUMP_LAND_UPPER,
	"jump_land_l",		// ANIMCONTROL_JUMP_LAND_LOWER,

	"aim_summer",		// ANIMCONTROL_AIM_SUMMER
	
	"fire",				// ANIMCONTROL_FIRE_1,
	"attack_summer",	// ANIMCONTROL_ATTACK_SUMMER,
	"attack_start",		// ANIMCONTROL_ATTACK_START,
	"attack",			// ANIMCONTROL_ATTACK,
	"attack_end",		// ANIMCONTROL_ATTACK_END,

	"hop_left",			// ANIMCONTROL_HOP_LEFT,
	"hop_right",		// ANIMCONTROL_HOP_RIGHT,

	"tethered_u",		// ANIMCONTROL_RC_TETHERED_UPPER,
	"tethered",			// ANIMCONTROL_RC_TETHERED,
	"powerdown",		// ANIMCONTROL_RC_POWER_DOWN,
	"powerup",			// ANIMCONTROL_RC_POWER_UP,
	"pwrdn_i",			// ANIMCONTROL_RC_POWER_DOWN_INV,
	"pwrup_i",			// ANIMCONTROL_RC_POWER_UP_INV,


	"drop",				// ANIMCONTROL_DROP,
	"aware",			// ANIMCONTROL_AWARE,
	"wake",				// ANIMCONTROL_WAKE,
	"sleep",			// ANIMCONTROL_SLEEP,

	"dive",				// ANIMCONTROL_DIVE,
	"dive_recover_u",	// ANIMCONTROL_DIVE_RECOVER_UPPER,
	"dive_recover_l",	// ANIMCONTROL_DIVE_RECOVER_LOWER,
};	


cchar *CBotJumper::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",			// ANIMTAP_STAND,
	"walk",				// ANIMTAP_WALK,
	"run",				// ANIMTAP_RUN,

	"jump_launch",		// ANIMTAP_JUMP_LAUNCH,
	"jump_fly_jets",	// ANIMTAP_JUMP_FLY_JETS,
	"jump_fly",			// ANIMTAP_JUMP_FLY,
	"jump_land_u",		// ANIMTAP_JUMP_LAND_UPPER,
	"jump_land_l",		// ANIMTAP_JUMP_LAND_LOWER,

	"aim summer",		// ANIMTAP_AIM_SUMMER
	"fire",				// ANIMTAP_FIRE_1,
	"attack_summer",	// ANIMTAP_ATTACK_SUMMER,
	"attack_start",		// ANIMTAP_ATTACK_START,
	"attack",			// ANIMTAP_ATTACK,
	"attack_end",		// ANIMTAP_ATTACK_END,
	
	"hop_left",			// ANIMTAP_HOP_LEFT,
	"hop_right",		// ANIMTAP_HOP_RIGHT,

	"tethered_u",		// ANIMTAP_RC_TETHERED_UPPER,
	"tethered",			// ANIMTAP_RC_TETHERED,
	"powerdown",		// ANIMTAP_RC_POWER_DOWN,
	"powerup",			// ANIMTAP_RC_POWER_UP,
	"pwrdn_i",			// ANIMTAP_RC_POWER_DOWN_INV,
	"pwrup_i",			// ANIMTAP_RC_POWER_UP_INV,

	"drop",				// ANIMTAP_DROP,
	"aware",			// ANIMTAP_AWARE,
	"wake",				// ANIMTAP_WAKE,
	"sleep",			// ANIMTAP_SLEEP,

	"dive",				// ANIMTAP_DIVE,
	"dive_recover_u",	// ANIMTAP_DIVE_RECOVER_UPPER,
	"dive_recover_l",	// ANIMTAP_DIVE_RECOVER_LOWER,
};


cchar *CBotJumper::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMJidle001"
};


CFAnimCombinerConfig::ConfigStack_t CBotJumper::m_aAnimCombinerConfigStack[] = {
	// from highest to lowest priority

	CFAnimMixer::TYPE_SUMMER,		ANIMCONTROL_AIM_SUMMER,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_TETHERED_UPPER,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_TETHERED,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_POWER_DOWN,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_POWER_UP_INV,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_POWER_DOWN_INV,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_DROP,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_AWARE,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_WAKE,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_SLEEP,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_HOP_LEFT,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_HOP_RIGHT,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_FIRE_1,
	CFAnimMixer::TYPE_SUMMER,		ANIMCONTROL_ATTACK_SUMMER,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_ATTACK_START,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_ATTACK,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_ATTACK_END,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_DIVE,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_DIVE_RECOVER_UPPER,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_DIVE_RECOVER_LOWER,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_FLY_JETS,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_FLY,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_LAND_UPPER,
	
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_WALK,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,		255

};


CFAnimCombinerConfig::ConfigTap_t	CBotJumper::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,				ANIMCONTROL_STAND,					1,
	ANIMTAP_WALK,				ANIMCONTROL_WALK,					1,
	ANIMTAP_RUN,				ANIMCONTROL_RUN,					1,
								
	ANIMTAP_JUMP_LAUNCH,		ANIMCONTROL_JUMP_LAUNCH,			1,
	ANIMTAP_JUMP_FLY,			ANIMCONTROL_JUMP_FLY,				1,
	ANIMTAP_JUMP_FLY_JETS,		ANIMCONTROL_JUMP_FLY_JETS,			1,
	ANIMTAP_JUMP_LAND_UPPER,	ANIMCONTROL_JUMP_LAND_UPPER,		1,
	ANIMTAP_JUMP_LAND_LOWER,	ANIMCONTROL_JUMP_LAND_LOWER,		1,
								
	ANIMTAP_AIM_SUMMER,			ANIMCONTROL_AIM_SUMMER,				1,
	ANIMTAP_FIRE_1,				ANIMCONTROL_FIRE_1,					1,
	ANIMTAP_ATTACK_SUMMER,		ANIMCONTROL_ATTACK_SUMMER,			1,
	ANIMTAP_ATTACK_START,		ANIMCONTROL_ATTACK_START,			1,
	ANIMTAP_ATTACK,				ANIMCONTROL_ATTACK,					1,
	ANIMTAP_ATTACK_END,			ANIMCONTROL_ATTACK_END,				1,

	ANIMTAP_HOP_LEFT,			ANIMCONTROL_HOP_LEFT,				1,
	ANIMTAP_HOP_RIGHT,			ANIMCONTROL_HOP_RIGHT,				1,
																	
	ANIMTAP_RC_TETHERED_UPPER,	ANIMCONTROL_RC_TETHERED_UPPER,		1,
	ANIMTAP_RC_TETHERED,		ANIMCONTROL_RC_TETHERED,			1,
	ANIMTAP_RC_POWER_DOWN,		ANIMCONTROL_RC_POWER_DOWN,			1,
	ANIMTAP_RC_POWER_UP,		ANIMCONTROL_RC_POWER_UP,			1,
	ANIMTAP_RC_POWER_DOWN_INV,	ANIMCONTROL_RC_POWER_DOWN_INV,		1,
	ANIMTAP_RC_POWER_UP_INV,	ANIMCONTROL_RC_POWER_UP_INV,		1,

	ANIMTAP_DROP,				ANIMCONTROL_DROP,					1,
	ANIMTAP_WAKE,				ANIMCONTROL_WAKE,					1,
	ANIMTAP_SLEEP,				ANIMCONTROL_SLEEP,					1,

	ANIMTAP_AWARE,				ANIMCONTROL_AWARE,					1,
	ANIMTAP_DIVE,				ANIMCONTROL_DIVE,					1,
	ANIMTAP_DIVE_RECOVER_UPPER,	ANIMCONTROL_DIVE_RECOVER_UPPER,		1,
	ANIMTAP_DIVE_RECOVER_LOWER,	ANIMCONTROL_DIVE_RECOVER_LOWER,		1,

	255,						255,								255
};


CFAnimCombiner::AttachList_t	CBotJumper::m_aAnimAttach[] = {
	ANIM_STAND,						ANIMTAP_STAND,			
	ANIM_WALK,						ANIMTAP_WALK,			
	ANIM_RUN,						ANIMTAP_RUN,			
															
	ANIM_JUMP_LAUNCH,				ANIMTAP_JUMP_LAUNCH,	
	ANIM_JUMP_FLY_JETS,				ANIMTAP_JUMP_FLY_JETS,		
	ANIM_JUMP_FLY,					ANIMTAP_JUMP_FLY,		
	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_UPPER,
	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_LOWER,
					
	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMTAP_AIM_SUMMER,

	ANIM_FIRE_1,					ANIMTAP_FIRE_1,
	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMTAP_ATTACK_SUMMER,
	ANIM_ATTACK_START,				ANIMTAP_ATTACK_START,	
	ANIM_ATTACK,					ANIMTAP_ATTACK,			
	ANIM_ATTACK_END,				ANIMTAP_ATTACK_END,
	ANIM_HOP_LEFT,					ANIMTAP_HOP_LEFT,		
	ANIM_HOP_RIGHT,					ANIMTAP_HOP_RIGHT,		
									
	ANIM_RC_TETHERED,				ANIMTAP_RC_TETHERED_UPPER,
	ANIM_RC_TETHERED,				ANIMTAP_RC_TETHERED,	
	ANIM_RC_POWER_DOWN,				ANIMTAP_RC_POWER_DOWN,	
	ANIM_RC_POWER_UP,				ANIMTAP_RC_POWER_UP,	
	ANIM_RC_POWER_DOWN_INV,			ANIMTAP_RC_POWER_DOWN_INV,	
	ANIM_RC_POWER_UP_INV,			ANIMTAP_RC_POWER_UP_INV,	
								
	ANIM_DROP,						ANIMTAP_DROP,			
	ANIM_WAKE,						ANIMTAP_WAKE,			
	ANIM_SLEEP,						ANIMTAP_SLEEP,			
	ANIM_STAND,						ANIMTAP_AWARE,

	ANIM_DIVE_RECOVER,				ANIMTAP_DIVE_RECOVER_UPPER,
	ANIM_DIVE_RECOVER,				ANIMTAP_DIVE_RECOVER_LOWER,

	ANIM_DIVE,						ANIMTAP_DIVE,
	

	255,							255
};


const u8 CBotJumper::m_aBoneEnableIndices_FullBody[] = {
	BONE_GROIN,			
	BONE_TORSO,			
	BONE_HEAD,			
	BONE_CHEST_TOP,		
	BONE_CHEST_BOTTOM,	
	BONE_CHEST_MIDDLE,	
	BONE_ARM_UPPER_L,	
	BONE_ARM_LOWER_L,	
	BONE_GUN_2_L,		
	BONE_GUN_1_L,		
	BONE_ARM_ARMOR_L,	

	BONE_ARM_UPPER_R,	
	BONE_ARM_LOWER_R,	
	BONE_GUN_2_R,		
	BONE_GUN_1_R,		
	BONE_ARM_ARMOR_R,	

	BONE_WING_L,		
	BONE_WING_R,		

	BONE_LEG_UPPER_L,	
	BONE_LEG_KNEE_L,	
	BONE_LEG_LOWER_L,	
	BONE_LEG_ANGKLE_L,	
	BONE_FOOT_BACK_L,	
	BONE_FOOT_BLADE_L,	
	BONE_FOOT_FRONT_L,	
	BONE_TOE1_L,		
	BONE_TOE2_L,		
						

	BONE_LEG_UPPER_R,	
	BONE_LEG_KNEE_R,	
	BONE_LEG_LOWER_R,	
	BONE_LEG_ANGKLE_R,	
	BONE_FOOT_BACK_R,	
	BONE_FOOT_BLADE_R,	
	BONE_FOOT_FRONT_R,	
	BONE_TOE1_R,		
	BONE_TOE2_R,		

	BONE_DUMMY,

	255
};


const u8 CBotJumper::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	BONE_GROIN,			
	BONE_TORSO,			
	BONE_HEAD,			
	BONE_CHEST_TOP,		
	BONE_CHEST_BOTTOM,	
	BONE_CHEST_MIDDLE,	
	BONE_ARM_UPPER_L,	
	BONE_ARM_LOWER_L,	
	BONE_GUN_2_L,		
	BONE_GUN_1_L,		
	BONE_ARM_ARMOR_L,	

	BONE_ARM_UPPER_R,	
	BONE_ARM_LOWER_R,	
	BONE_GUN_2_R,		
	BONE_GUN_1_R,		
	BONE_ARM_ARMOR_R,	

	BONE_WING_L,		
	BONE_WING_R,		

	BONE_LEG_UPPER_L,	
	BONE_LEG_KNEE_L,	
	BONE_LEG_LOWER_L,	
	BONE_LEG_ANGKLE_L,	
	BONE_FOOT_BACK_L,	
	BONE_FOOT_BLADE_L,	
	BONE_FOOT_FRONT_L,	
	BONE_TOE1_L,		
	BONE_TOE2_L,		
						

	BONE_LEG_UPPER_R,	
	BONE_LEG_KNEE_R,	
	BONE_LEG_LOWER_R,	
	BONE_LEG_ANGKLE_R,	
	BONE_FOOT_BACK_R,	
	BONE_FOOT_BLADE_R,	
	BONE_FOOT_FRONT_R,	
	BONE_TOE1_R,		
	BONE_TOE2_R,	
	255
};


const u8 CBotJumper::m_aBoneEnableIndices_Attack[] = {
	BONE_TORSO,			
	BONE_HEAD,			
	BONE_CHEST_TOP,		
	BONE_CHEST_BOTTOM,	
	BONE_CHEST_MIDDLE,	
	BONE_ARM_UPPER_L,	
	BONE_ARM_LOWER_L,	
	BONE_GUN_2_L,		
	BONE_GUN_1_L,		
	BONE_ARM_ARMOR_L,	

	BONE_ARM_UPPER_R,	
	BONE_ARM_LOWER_R,	
	BONE_GUN_2_R,		
	BONE_GUN_1_R,		
	BONE_ARM_ARMOR_R,	

	255
};

const u8 CBotJumper::m_aBoneEnableIndices_AimSummer[] = {
	BONE_HEAD,
	BONE_TORSO,

	255
};


const u8* CBotJumper::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND,			
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_WALK,			
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RUN,			
						
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_LAUNCH,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_FLY_JETS,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_FLY,		
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_JUMP_LAND_UPPER,
	m_aBoneEnableIndices_UserAnim_LowerBody,	// ANIMTAP_JUMP_LAND_LOWER,
		
	m_aBoneEnableIndices_AimSummer,				// ANIMTAP_AIM_SUMMER,

	m_aBoneEnableIndices_Attack,				// ANIMTAP_FIRE_1,
	m_aBoneEnableIndices_Attack,				// ANIMTAP_ATTACK_SUMMER,
	m_aBoneEnableIndices_Attack,				// ANIMTAP_ATTACK_START,	
	m_aBoneEnableIndices_Attack,				// ANIMTAP_ATTACK,			
	m_aBoneEnableIndices_Attack,				// ANIMTAP_ATTACK_END,		

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_HOP_LEFT,		
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_HOP_RIGHT,		
	
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_RC_TETHERED_UPPER,	
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_RC_TETHERED,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_DOWN,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_UP,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_DOWN_INV,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_UP_INV,	


	m_aBoneEnableIndices_FullBody,				// ANIMTAP_DROP,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_AWARE,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_WAKE,			
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_SLEEP,			
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_DIVE,			
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_DIVE_RECOVER_UPPER,			
	m_aBoneEnableIndices_UserAnim_LowerBody,	// ANIMTAP_DIVE_RECOVER_LOWER,			

};


const u8* CBotJumper::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// Idle 0
};


//BONE MASKS FOR USER ANIMS

const u8 CBotJumper::m_aBoneEnableIndices_UserAnim_UpperBody[] = {
	BONE_GROIN,			
	BONE_TORSO,			
	BONE_HEAD,			
	BONE_CHEST_TOP,		
	BONE_CHEST_BOTTOM,	
	BONE_CHEST_MIDDLE,	
	BONE_ARM_UPPER_L,	
	BONE_ARM_LOWER_L,	
	BONE_GUN_2_L,		
	BONE_GUN_1_L,		
	BONE_ARM_ARMOR_L,	

	BONE_ARM_UPPER_R,	
	BONE_ARM_LOWER_R,	
	BONE_GUN_2_R,		
	BONE_GUN_1_R,		
	BONE_ARM_ARMOR_R,	

	BONE_WING_L,		
	BONE_WING_R,		

	255
};


const u8 CBotJumper::m_aBoneEnableIndices_UserAnim_LowerBody[] = {
	BONE_GROIN,			

	BONE_LEG_UPPER_L,	
	BONE_LEG_KNEE_L,	
	BONE_LEG_LOWER_L,	
	BONE_LEG_ANGKLE_L,	
	BONE_FOOT_BACK_L,	
	BONE_FOOT_BLADE_L,	
	BONE_FOOT_FRONT_L,	
	BONE_TOE1_L,		
	BONE_TOE2_L,		
						

	BONE_LEG_UPPER_R,	
	BONE_LEG_KNEE_R,	
	BONE_LEG_LOWER_R,	
	BONE_LEG_ANGKLE_R,	
	BONE_FOOT_BACK_R,	
	BONE_FOOT_BLADE_R,	
	BONE_FOOT_FRONT_R,	
	BONE_TOE1_R,		
	BONE_TOE2_R,		
	255
};


const u8 CBotJumper::m_aBoneEnableIndices_UserAnim_UpperTorso[] = {
	BONE_TORSO,			
	BONE_CHEST_TOP,		
	BONE_CHEST_BOTTOM,	
	BONE_CHEST_MIDDLE,	

	BONE_WING_L,		
	BONE_WING_R,		
	255
};


const u8 CBotJumper::m_aBoneEnableIndices_UserAnim_LowerTorso[] = {
	BONE_GROIN,			
	BONE_LEG_UPPER_L,	
	BONE_LEG_UPPER_R,	
	255
};


const u8 CBotJumper::m_aBoneEnableIndices_UserAnim_LeftArm[] = {
	BONE_ARM_UPPER_L,	
	BONE_ARM_LOWER_L,	
	BONE_GUN_2_L,		
	BONE_GUN_1_L,		
	BONE_ARM_ARMOR_L,	
	255
};


const u8 CBotJumper::m_aBoneEnableIndices_UserAnim_RightArm[] = {
	BONE_ARM_UPPER_R,	
	BONE_ARM_LOWER_R,	
	BONE_GUN_2_R,		
	BONE_GUN_1_R,		
	BONE_ARM_ARMOR_R,	
	255
};


const u8 CBotJumper::m_aBoneEnableIndices_UserAnim_Head[] = {
	BONE_HEAD,
	255
};


const u8 CBotJumper::m_nApproxEyePointBoneNameIndex = BONE_HEAD;


const u8 CBotJumper::m_anTagPointBoneNameIndexArray[] = {
	BONE_HEAD,
	BONE_TORSO,
	BONE_ARM_UPPER_L,
	BONE_ARM_UPPER_R,

	255
};

cchar* CBotJumper::m_apszStreamerBonesRed[] = {
	"AttachPoint_R_Streamer",
	NULL
};


cchar* CBotJumper::m_apszStreamerBonesGreen[] = {
	"AttachPoint_L_Streamer",
	NULL
};


cchar* CBotJumper::m_apszStreamerBonesBladeLeft[] = {
	"off_L_Arm_Dummy",
	NULL
};


cchar* CBotJumper::m_apszStreamerBonesBladeRight[] = {
	"off_R_Arm_Dummy",
	NULL
};



//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotJumper::m_aGameDataMap[] = {
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

	"Jumper",
	m_aBotInfoVocab_Jumper,
	sizeof(m_BotInfo_Jumper),
	(void*)&m_BotInfo_Jumper,

	NULL
};


const FGameDataMap_t CBotJumper::m_aSoundDataMap[] = {
	"JumperSounds",
	m_aBotInfoVocab_JumperSounds,
	sizeof(m_BotInfo_JumperSounds),
	(void *)&m_BotInfo_JumperSounds,

	NULL
};


const FGameData_TableEntry_t CBotJumper::m_aBotInfoVocab_Jumper[] = {
	FGAMEDATA_VOCAB_DAMAGE,			// apDamageProfileGun[0]
	FGAMEDATA_VOCAB_DAMAGE,			// apDamageProfileGun[1]
	FGAMEDATA_VOCAB_DAMAGE,			// apDamageProfileMelee[0]
	FGAMEDATA_VOCAB_DAMAGE,			// apDamageProfileMelee[1]
	FGAMEDATA_VOCAB_DAMAGE,			// apDamageProfileDive[0]
	FGAMEDATA_VOCAB_DAMAGE,			// apDamageProfileDive[1]

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hGunExplosion

	FGAMEDATA_VOCAB_PARTICLE,		// hParticleDefJumpJetSmoke
	FGAMEDATA_VOCAB_PARTICLE,		// hParticleDefJumpJetFire
	FGAMEDATA_VOCAB_PARTICLE,		// hParticleDefJumpLaunch

	//fMaxAirAccel;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fDecelMult;			
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fTerminalVelocity;	
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1000,
	F32_DATATABLE_0,

	//fJumpLaunchVel;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fJumpLaunchAccel;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fJumpLaunchTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fJumpDescendVel;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	//fJumpMinInterval;	
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fJumpDiveInterval;	
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fMeleeAttackSpeed;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10,

	//uMaxTracers;		
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( u32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100,

	//fTracerSpeed;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fFireRate;			
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10,

	//fWingStreamerAlphaAI;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fWingStreamerAlphaPlayer;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fDiveMaxDistance;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fSoundVolume3D;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fSoundVolume2D;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fSoundRadius3D;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fDiveTurnDegPerSec;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS | FGAMEDATA_FLAGS_FLOAT_SIN_DEGS_TO_RADS_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameData_TableEntry_t CBotJumper::m_aBotInfoVocab_JumperSounds[] = {

	//FSndFx_FxHandle_t hMeleeAttack;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hMeleeImpact;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hJetsLaunch;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hJetsLoop;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hGunsFire;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hDive;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


BOOL CBotJumper::_BuildAnimStackDef( void ) {
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

