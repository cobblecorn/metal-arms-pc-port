//////////////////////////////////////////////////////////////////////////////////////
// botsniper_data.cpp - Sniper
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
// 03/11/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botanim.h"
#include "fmesh.h"
#include "botsniper.h"

const CBotDef CBotSniper::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_SNIPER,						// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CBotSniper::m_apszBoneNameTable[BONE_COUNT] = {
	"SniperDummy",			// BONE_DUMMY,				
	"Groin",				// BONE_GROIN,				
	"Spine",				// BONE_SPINE,				
	"Torso",				// BONE_TORSO,				
	"Neck",					// BONE_NECK,				
	"Head",					// BONE_HEAD,				
	"Rib1",					// BONE_RIB1,				
	"Rib2",					// BONE_RIB2,				
	"Rib3",					// BONE_RIB3,				
	"Rib4",					// BONE_RIB4,				
							   
	"R_Leg_Upper",			// BONE_LEG_UPPER_R,		
	"R_Leg_Lower",			// BONE_LEG_LOWER_R,		
	"R_Ankle",				// BONE_ANKLE_R,			
	"R_Foot",				// BONE_FOOT_R,			
	"R_Leg_PistonB",		// BONE_LEG_PISTONB_R,		
	"R_Leg_PistonA",		// BONE_LEG_PISTONA_R,		
							   
	"L_Leg_Upper",			// BONE_LEG_UPPER_L,		
	"L_Leg_Lower",			// BONE_LEG_LOWER_L,		
	"L_Ankle",				// BONE_ANKLE_L,			
	"L_Foot",				// BONE_FOOT_L,			
	"L_Leg_PistonB",		// BONE_LEG_PISTONB_L,		
	"L_Leg_PistonA",		// BONE_LEG_PISTONA_L,		
							   
	"L_Arm_Upper",			// BONE_ARM_UPPER_L,		
	"L_Arm_Lower",			// BONE_ARM_LOWER_L,		
	"L_Arm_PistonB",		// BONE_ARM_PISTONB_L,		
	"L_Arm_PistonA",		// BONE_ARM_PISTONA_L,		
							   
	"R_Arm_Upper",			// BONE_ARM_UPPER_R,		
	"R_Arm_Lower",			// BONE_ARM_LOWER_R,		
	"Gun_Barrel",			// BONE_GUN_BARREL,		
	"R_Arm_PistonB",		// BONE_ARM_PISTONB_R,		
	"R_Arm_PistonA",		// BONE_ARM_PISTONA_R,	

	"Primary_Fire",			// BONE_PRIMARY_FIRE,
	"AttachPoint_Grapple",	// BONE_GRAPPLE_ATTACH,
};

cchar *CBotSniper::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMNidle001",		// ANIM_STAND,
	"ARMNwalkF01",		// ANIM_WALK,
	"ARMNrun_F01",		// ANIM_RUN,
																
	"ARMNjump_01",		// ANIM_JUMP_LAUNCH,
	"ARMNjump_02",		// ANIM_JUMP_FLY,
	"ARMNjump_04",		// ANIM_JUMP_LAND,						
	"ARMNjump_03",		// ANIM_JUMP_RECOVER					
	
	"ARMNactn202",		//  ANIM_GRAPPLE_FLY,
	"ARMNready02",		// ANIM_GRAPPLE,						
	"ARMNaim_c01",		// ANIM_HANG_CEILING,					
	"ARMNaim_W01",		// ANIM_HANG_WALL,						
	
	"armnfirep01",		// ANIM_FIRE_END,
	"ARMNactn101",		// ANIM_FIRE_START,						
	"ARMNactn102",		// ANIM_FIRE,							
	"ARMNactn103",		// ANIM_FIRE_END,						
																
	"ARMNelect01",		// ANIM_RC_TETHERED,
	"ARMNpwrdn01",		// ANIM_RC_POWER_DOWN,
	"ARMNpwrup01"		// ANIM_RC_POWER_UP,

};


cchar *CBotSniper::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",			// ANIMCONTROL_STAND,
	"walk",				// ANIMCONTROL_WALK,
	"run",				// ANIMCONTROL_RUN,
	
	"jump_launch",		// ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",			// ANIMCONTROL_JUMP_FLY,
	"jump_land_u",		// ANIMCONTROL_JUMP_LAND_UPPER,
	"jump_land_l",		// ANIMCONTROL_JUMP_LAND_LOWER,
	"jump_land_r",		// ANIMCONTROL_JUMP_RECOVER,
	
	"grapple_fly",		// ANIMCONTROL_GRAPPLE_FLY
	"grapple",			// ANIMCONTROL_GRAPPLE,
	"grapple_hang",		// ANIMCONTROL_GRAPPLE_HANG,
	"grapple_l",		// ANIMCONTROL_GRAPPLE_LOWER,
	
	"sn_summer",		// ANIMCONTROL_SUMMER,
	
	"fire_fast",		// ANIMCONTROL_FIRE_FAST,
	"fire_start",		// ANIMCONTROL_FIRE_START,
	"fire",				// ANIMCONTROL_FIRE,
	"fire_end",			// ANIMCONTROL_FIRE_END,

	"fire_fast_l",		// ANIMCONTROL_FIRE_FAST_LOWER,
	"fire_start_l",		// ANIMCONTROL_FIRE_START_LOWER,
	"fire_l",			// ANIMCONTROL_FIRE_LOWER,
	"fire_end_l",		// ANIMCONTROL_FIRE_END_LOWER,

	"tether",			// ANIMCONTROL_RC_TETHERED,
	"power_dn",			// ANIMCONTROL_RC_POWER_DOWN,
	"power_up",			// ANIMCONTROL_RC_POWER_UP,
};


// Animation taps:
cchar *CBotSniper::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",			// ANIMTAP_STAND,				
	"walk",				// ANIMTAP_WALK,
	"run",				// ANIMTAP_RUN,

	"jump_launch",		// ANIMTAP_JUMP_LAUNCH,
	"jump_fly",			// ANIMTAP_JUMP_FLY,
	"jump_land_u",		// ANIMTAP_JUMP_LAND_UPPER,
	"jump_land_l",		// ANIMTAP_JUMP_LAND_LOWER,
	"jump_land_r",		// ANIMTAP_JUMP_RECOVER,

	"grapple_fly",		// ANIMTAP_GRAPPLE_FLY
	"grapple",			// ANIMTAP_GRAPPLE,
	"grapple_hang",		// ANIMTAP_GRAPPLE_HANG,
	"grapple_l",		// ANIMTAP_GRAPPLE_LOWER,

	"sn_summer",		// ANIMTAP_SUMMER,

	"fire_fast",		// ANIMTAP_FIRE_FAST,
	"fire_start",		// ANIMTAP_FIRE_START,
	"fire",				// ANIMTAP_FIRE,
	"fire_end",			// ANIMTAP_FIRE_END,

	"fire_fast_l",		// ANIMTAP_FIRE_FAST_LOWER,
	"fire_start_l",		// ANIMTAP_FIRE_START_LOWER,
	"fire_l",			// ANIMTAP_FIRE_LOWER,
	"fire_end_l",		// ANIMTAP_FIRE_END_LOWER,

	"tether",			// ANIMTAP_RC_TETHERED,
	"power_dn",			// ANIMTAP_RC_POWER_DOWN,
	"power_up",			// ANIMTAP_RC_POWER_UP,
};


cchar *CBotSniper::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMNidle001"
};


CFAnimCombinerConfig::ConfigStack_t CBotSniper::m_aAnimCombinerConfigStack[] = {
	// from highest to lowest priority

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_RC_TETHERED,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_RC_POWER_DOWN,

	CFAnimMixer::TYPE_SUMMER,			ANIMCONTROL_SUMMER,
	
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE_FAST,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE_START,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE_END,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_GRAPPLE_HANG,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_GRAPPLE_FLY,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_GRAPPLE,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_FLY,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_LAND_RECOVER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_LAND_UPPER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_JUMP_LAND_LOWER,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_WALK,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE_FAST_LOWER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE_START_LOWER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE_LOWER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_FIRE_END_LOWER,
	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_GRAPPLE_LOWER,

	CFAnimMixer::TYPE_BLENDER,			ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,			255

};


CFAnimCombinerConfig::ConfigTap_t	CBotSniper::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,					ANIMCONTROL_STAND,				1,
	ANIMTAP_WALK,					ANIMCONTROL_WALK,				1,
	ANIMTAP_RUN,					ANIMCONTROL_RUN,				1,
										
	ANIMTAP_JUMP_LAUNCH,			ANIMCONTROL_JUMP_LAUNCH,		1,
	ANIMTAP_JUMP_FLY,				ANIMCONTROL_JUMP_FLY,			1,
	ANIMTAP_JUMP_LAND_UPPER,		ANIMCONTROL_JUMP_LAND_UPPER,	1,
	ANIMTAP_JUMP_LAND_LOWER,		ANIMCONTROL_JUMP_LAND_LOWER,	1,
	ANIMTAP_JUMP_LAND_RECOVER,		ANIMCONTROL_JUMP_LAND_RECOVER,	1,
										
	ANIMTAP_GRAPPLE_FLY,			ANIMCONTROL_GRAPPLE_FLY,		1,
	ANIMTAP_GRAPPLE,				ANIMCONTROL_GRAPPLE,			1,
	ANIMTAP_GRAPPLE_HANG,			ANIMCONTROL_GRAPPLE_HANG,		1,

	ANIMTAP_SUMMER,					ANIMCONTROL_SUMMER,				1,
										
	ANIMTAP_FIRE_FAST,				ANIMCONTROL_FIRE_FAST,			1,
	ANIMTAP_FIRE_START,				ANIMCONTROL_FIRE_START,			1,
	ANIMTAP_FIRE,					ANIMCONTROL_FIRE,				1,
	ANIMTAP_FIRE_END,				ANIMCONTROL_FIRE_END,			1,

	ANIMTAP_RC_TETHERED,			ANIMCONTROL_RC_TETHERED,		1,
	ANIMTAP_RC_POWER_DOWN,			ANIMCONTROL_RC_POWER_DOWN,		1,
	ANIMTAP_RC_POWER_UP,			ANIMCONTROL_RC_POWER_UP,		1,

	ANIMTAP_FIRE_FAST_LOWER,		ANIMCONTROL_FIRE_FAST_LOWER,	1,
	ANIMTAP_FIRE_START_LOWER,		ANIMCONTROL_FIRE_START_LOWER,	1,
	ANIMTAP_FIRE_LOWER,				ANIMCONTROL_FIRE_LOWER,			1,
	ANIMTAP_FIRE_END_LOWER,			ANIMCONTROL_FIRE_END_LOWER,		1,
	ANIMTAP_GRAPPLE_LOWER,			ANIMCONTROL_GRAPPLE_LOWER,		1,

	255,							255,							255
};


CFAnimCombiner::AttachList_t	CBotSniper::m_aAnimAttach[] = {
	ANIM_STAND,						ANIMTAP_STAND,				
	ANIM_WALK,						ANIMTAP_WALK,				
	ANIM_RUN,						ANIMTAP_RUN,				
																
	ANIM_JUMP_LAUNCH,				ANIMTAP_JUMP_LAUNCH,		
	ANIM_JUMP_FLY,					ANIMTAP_JUMP_FLY,			
	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_UPPER,	
	ANIM_JUMP_LAND,					ANIMTAP_JUMP_LAND_LOWER,
	ANIM_JUMP_RECOVER,				ANIMTAP_JUMP_LAND_RECOVER,
									
	ANIM_GRAPPLE_FLY,				ANIMTAP_GRAPPLE_FLY,
	ANIM_GRAPPLE,					ANIMTAP_GRAPPLE,
	ANIM_HANG_CEILING,				ANIMTAP_GRAPPLE_HANG,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMTAP_SUMMER,
	
	ANIM_FIRE_FAST,					ANIMTAP_FIRE_FAST,
	ANIM_FIRE_START,				ANIMTAP_FIRE_START,			
	ANIM_FIRE,						ANIMTAP_FIRE,				
	ANIM_FIRE_END,					ANIMTAP_FIRE_END,

	ANIM_RC_TETHERED,				ANIMTAP_RC_TETHERED,		
	ANIM_RC_POWER_DOWN,				ANIMTAP_RC_POWER_DOWN,		
	ANIM_RC_POWER_UP,				ANIMTAP_RC_POWER_UP,	

	ANIM_FIRE_FAST,					ANIMTAP_FIRE_FAST_LOWER,
	ANIM_FIRE_START,				ANIMTAP_FIRE_START_LOWER,
	ANIM_FIRE,						ANIMTAP_FIRE_LOWER,
	ANIM_FIRE_END,					ANIMTAP_FIRE_END_LOWER,
	ANIM_GRAPPLE,					ANIMTAP_GRAPPLE_LOWER,

	255,							255,
};


const u8 CBotSniper::m_aBoneEnableIndices_FullBody[] = {
	BONE_DUMMY,				
	BONE_GROIN,				
	BONE_SPINE,				
	BONE_TORSO,				
	BONE_NECK,				
	BONE_HEAD,				
	BONE_RIB1,				
	BONE_RIB2,				
	BONE_RIB3,				
	BONE_RIB4,				

	BONE_LEG_UPPER_R,		
	BONE_LEG_LOWER_R,		
	BONE_ANKLE_R,			
	BONE_FOOT_R,			
	BONE_LEG_PISTONB_R,		
	BONE_LEG_PISTONA_R,		

	BONE_LEG_UPPER_L,		
	BONE_LEG_LOWER_L,		
	BONE_ANKLE_L,			
	BONE_FOOT_L,			
	BONE_LEG_PISTONB_L,		
	BONE_LEG_PISTONA_L,		

	BONE_ARM_UPPER_L,		
	BONE_ARM_LOWER_L,		
	BONE_ARM_PISTONB_L,		
	BONE_ARM_PISTONA_L,		

	BONE_ARM_UPPER_R,		
	BONE_ARM_LOWER_R,		
	BONE_GUN_BARREL,		
	BONE_ARM_PISTONB_R,		
	BONE_ARM_PISTONA_R,	

	255
};


const u8 CBotSniper::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	BONE_DUMMY,				
	BONE_GROIN,				

	BONE_SPINE,				
	BONE_TORSO,				
	BONE_NECK,				
	BONE_HEAD,				
	BONE_RIB1,				
	BONE_RIB2,				
	BONE_RIB3,				
	BONE_RIB4,				

	BONE_LEG_UPPER_R,		
	BONE_LEG_LOWER_R,		
	BONE_ANKLE_R,			
	BONE_FOOT_R,			
	BONE_LEG_PISTONB_R,		
	BONE_LEG_PISTONA_R,		

	BONE_LEG_UPPER_L,		
	BONE_LEG_LOWER_L,		
	BONE_ANKLE_L,			
	BONE_FOOT_L,			
	BONE_LEG_PISTONB_L,		
	BONE_LEG_PISTONA_L,		

	BONE_ARM_UPPER_L,		
	BONE_ARM_LOWER_L,		
	BONE_ARM_PISTONB_L,		
	BONE_ARM_PISTONA_L,		

	BONE_ARM_UPPER_R,		
	BONE_ARM_LOWER_R,		
	BONE_GUN_BARREL,		
	BONE_ARM_PISTONB_R,		
	BONE_ARM_PISTONA_R,		
};



const u8* CBotSniper::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND,									
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_WALK,									
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RUN,										
																									
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_LAUNCH,								
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_FLY,								
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_JUMP_LAND_UPPER,							
	m_aBoneEnableIndices_UserAnim_LowerBody,	// ANIMTAP_JUMP_LAND_LOWER,							
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_RECOVER,							
																	
	m_aBoneEnableIndices_FullBody,				// ANIMTATP_GRAPPLE_FLY,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_GRAPPLE,									
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_GRAPPLE_HANG,							
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_GRAPPLE_LOWER,
																									
	m_aBoneEnableIndices_Summer,				// ANIMTAP_SUMMER,									
						
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_FIRE_FAST,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_FIRE_START,								
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_FIRE,									
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_FIRE_END,								
																									
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_FIRE_FAST_LOWER,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_FIRE_START_LOWER,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_FIRE_LOWER,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_FIRE_END_LOWER,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_TETHERED,								
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWERDOWN,							
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWERUP,																										
};																									
																									
																									
																									
const u8* CBotSniper::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {				
	m_aBoneEnableIndices_FullBody,				// Idle 0											
};																									


//BONE MASKS FOR USER ANIMS

const u8 CBotSniper::m_aBoneEnableIndices_UserAnim_UpperBody[] = {
	BONE_SPINE,				
	BONE_TORSO,				
	BONE_NECK,				
	BONE_HEAD,				
	BONE_RIB1,				
	BONE_RIB2,				
	BONE_RIB3,				
	BONE_RIB4,				

	BONE_ARM_UPPER_L,		
	BONE_ARM_LOWER_L,		
	BONE_ARM_PISTONB_L,		
	BONE_ARM_PISTONA_L,		

	BONE_ARM_UPPER_R,		
	BONE_ARM_LOWER_R,		
	BONE_GUN_BARREL,		
	BONE_ARM_PISTONB_R,		
	BONE_ARM_PISTONA_R,	

	255
};


const u8 CBotSniper::m_aBoneEnableIndices_UserAnim_LowerBody[] = {
	BONE_DUMMY,				
	BONE_GROIN,				

	BONE_LEG_UPPER_R,		
	BONE_LEG_LOWER_R,		
	BONE_ANKLE_R,			
	BONE_FOOT_R,			
	BONE_LEG_PISTONB_R,		
	BONE_LEG_PISTONA_R,		

	BONE_LEG_UPPER_L,		
	BONE_LEG_LOWER_L,		
	BONE_ANKLE_L,			
	BONE_FOOT_L,			
	BONE_LEG_PISTONB_L,		
	BONE_LEG_PISTONA_L,		

	255
};

const u8 CBotSniper::m_aBoneEnableIndices_UserAnim_UpperTorso[] = {
	BONE_SPINE,				
	BONE_TORSO,				
	BONE_NECK,				
	BONE_HEAD,				
	BONE_RIB1,				
	BONE_RIB2,				
	BONE_RIB3,				
	BONE_RIB4,				

	255,
};

const u8 CBotSniper::m_aBoneEnableIndices_UserAnim_LowerTorso[] = {
	BONE_DUMMY,				
	BONE_GROIN,				

	BONE_LEG_UPPER_R,		

	BONE_LEG_UPPER_L,		

	255
};

const u8 CBotSniper::m_aBoneEnableIndices_UserAnim_LeftArm[] = {
	BONE_ARM_UPPER_L,		
	BONE_ARM_LOWER_L,		
	BONE_ARM_PISTONB_L,		
	BONE_ARM_PISTONA_L,		

	255
};


const u8 CBotSniper::m_aBoneEnableIndices_UserAnim_RightArm[] = {
	BONE_ARM_UPPER_R,		
	BONE_ARM_LOWER_R,		
	BONE_GUN_BARREL,		
	BONE_ARM_PISTONB_R,		
	BONE_ARM_PISTONA_R,	

	255
};

const u8 CBotSniper::m_aBoneEnableIndices_UserAnim_Head[] = {
	BONE_HEAD,				
	255
};


const u8 CBotSniper::m_nApproxEyePointBoneNameIndex = BONE_HEAD;


const u8 CBotSniper::m_anTagPointBoneNameIndexArray[] = {
	BONE_HEAD,
	BONE_TORSO,
	BONE_ARM_UPPER_L,
	BONE_ARM_UPPER_R,
	255
};


cchar *CBotSniper::m_apszSummerBoneNameTable[ANIMBONE_COUNT] = {
	"L_Arm_Upper",			// ANIMBONE_ARM_UPPER_L,
	"L_Arm_Lower",			// ANIMBONE_ARM_LOWER_L,
	"R_Arm_Upper",			// ANIMBONE_ARM_UPPER_R,
	"Head",					// ANIMBONE_HEAD,
	"Torso",				// ANIMBONE_TORSO,
	"Rib1",					// ANIMBONE_RIB1,
	"Rib2",					// ANIMBONE_RIB2,
	"Rib3",					// ANIMBONE_RIB3,
	"Rib4",					// ANIMBONE_RIB4,	
};


const u8 CBotSniper::m_aBoneEnableIndices_Summer[] = {
	BONE_ARM_UPPER_L,
	BONE_ARM_LOWER_L,
	BONE_ARM_UPPER_R,
	BONE_TORSO,
	BONE_RIB1,
	BONE_RIB2,
	BONE_RIB3,
	BONE_RIB4,	
	255
};


BOOL CBotSniper::_BuildAnimStackDef( void ) {
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
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotSniper::m_aGameDataMap[] = {
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

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

	//"Sniper",
	//m_aBotInfoVocab_Sniper,
	//sizeof(m_BotInfo_Sniper),
	//(void*)&m_BotInfo_Sniper,

	NULL
};










