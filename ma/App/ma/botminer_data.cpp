//////////////////////////////////////////////////////////////////////////////////////
// botgrunt_data.cpp - Bot data.
//
// Author: Steve Ranck     
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
// 03/26/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botminer.h"
#include "botanim.h"
#include "fmesh.h"



//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotMiner::m_BotDef = {
	BOTRACE_DROID,							// nRace
	BOTCLASS_MINER,							// nClass
	BOTSUBCLASS_MINER_BASE,					// nSubClass
};




//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotMiner::m_aGameDataMap[] = {
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

cchar *CBotMiner::m_apszBoneNameTable[BONE_COUNT] = {
	"Groin",				// BONE_GROIN,					
	"Torso",				// BONE_TORSO,					
	"Head",					// BONE_HEAD,					
				
	"L_Shoulder",			// BONE_LEFT_SHOULDER,			
	"L_Arm_Upper",			// BONE_LEFT_ARM_UPPER,		
	"L_Arm_Elbow",			// BONE_LEFT_ARM_ELBOW,		
	"L_Arm_Lower",			// BONE_LEFT_ARM_LOWER,		
	"L_Hand",				// BONE_LEFT_HAND,				
	"Secondary_Fire",		// BONE_SECONDARY_FIRE,		
				
	"L_ThumbA",				// BONE_LEFT_THUMB_A,			
	"L_IndexFingerA",		// BONE_LEFT_INDEXFINGER_A,	
	"L_MiddleFingerA",		// BONE_LEFT_MIDDLEFINGER_A,	
	"L_PinkyFingerA",		// BONE_LEFT_PINKYFINGER_A,	
				
	"R_Shoulder",			// BONE_RIGHT_SHOULDER,		
	"R_Arm_Upper",			// BONE_RIGHT_ARM_UPPER,		
	"R_Arm_Lower",			// BONE_RIGHT_ARM_LOWER,
	"AttachPoint_Primary",	// BONE_ATTACHPOINT_PRIMARY,
				
	"Flap",					// BONE_FLAP,					
	"Backpack",				// BONE_BACKPACK,				
	"AntennaA",				// BONE_ANTENNA_A,				
	"AntennaB",				// BONE_ANTENNA_B,				
	"AntennaC",				// BONE_ANTENNA_C,				
	"AntennaD",				// BONE_ANTENNA_D,				
	"AntennaE",				// BONE_ANTENNA_E,				
	"MinerDummy",			// BONE_MINER_DUMMY,			
				
	"L_Hip",				// BONE_LEFT_HIP,				
	"L_Leg_Upper",			// BONE_LEFT_LEG_UPPER,		
	"L_Knee",				// BONE_LEFT_KNEE,				
	"L_Leg_Lower",			// BONE_LEFT_LEG_LOWER,		
	"L_Foot_Back",			// BONE_LEFT_FOOT_BACK,		
	"L_Heel",				// BONE_LEFT_HEEL,				
	"L_Foot_Front",			// BONE_LEFT_FOOT_FRONT,		
	"L_Toe",				// BONE_LEFT_TOE,				
				
	"R_Hip",				// BONE_RIGHT_HIP,				
	"R_Leg_Upper",			// BONE_RIGHT_LEG_UPPER,		
	"R_Knee",				// BONE_RIGHT_KNEE,			
	"R_Leg_Lower",			// BONE_RIGHT_LEG_LOWER,		
	"R_Foot_Back",			// BONE_RIGHT_FOOT_BACK,		
	"R_Heel",				// BONE_RIGHT_HEEL,			
	"R_Foot_Front",			// BONE_RIGHT_FOOT_FRONT,		
	"R_Toe",				// BONE_RIGHT_TOE,				
		
	//lite_spot_yellow*ID(1)*perpixel(TF_1spotlt1)
};


cchar *CBotMiner::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	// Ground locomotion:																	
	"ARDMidle001",					// ANIM_STAND
////	"ARDMtiptoe1",					// ANIM_SNEAK											
	"ARDMwalkF01",					// ANIM_WALK_FORWARD									
	"ARDMrun_F01",					// ANIM_RUN_FORWARD										
//	"zARMGrun_f02",					// ANIM_RUN_PANIC,										
//	"zARMGidle002",					// ANIM_STAND_ALERT,									
//	"zARMGwalkF02",					// ANIM_WALK_ALERT,										
//	"zARMGwalkF03",					// ANIM_TURN_IN_PLACE									
																							
	// Air locomotion:																		
	"ARDMjump_01",					// ANIM_JUMP_LAUNCH									
	"ARDMjump_03",					// ANIM_JUMP_LAND									
	"ARDMjump_02",					// ANIM_JUMP_FLY									
	"ARDMjump201",					// ANIM_JUMP_TUCK									
	"ARDMjump202",					// ANIM_JUMP_UNTUCK									
	"ARDMcable01",					// ANIM_CABLE_REACH									
	"ARDMcable02",					// ANIM_CABLE_GRASP									
	"ARDMcable03",					// ANIM_CABLE_RELEASE									
	
	//"zARMGhop_L01",					// ANIM_HOP_LEFT										
	//"zARMGhop_r01",					// ANIM_HOP_RIGHT										
	//"zARMGstart01",					// ANIM_STARTLE											
	//"zARMGdiveL01",					//ANIM_ROLL_LEFT,										
	//"zARMGdiveR01",					//ANIM_ROLL_RIGHT,										
																							
	// Weapon firing and reloading:															
	"ARDMactn101",					// ANIM_FIRE_PRIMARY									
	"ARDMactn201",					// ANIM_FIRE_SECONDARY									
	//"zARMGactn1c1",					// ANIM_FIRE_PRIMARY_ALERT								
																							
	//// Remote control:																		
	//"zARMGelect01",					// ANIM_RC_TETHERED										
	"ARDMpwrdn01",					// ANIM_RC_POWER_DOWN									
	"ARDMpwrup01",					// ANIM_RC_POWER_UP										
																							
	//// misc																					
	//"zARMGDoze_01",					// ANIM_DOZE_LOOP										
	//"zARMGWake_02",					// ANIM_NAPJERK											
	//"zARMGWake_01",					// ANIM_WAKE											
																							
	"ARDMrldP201",					// ANIM_RELOAD_CLIP_EJECT_OLD							
	"ARDMrldP202",					// ANIM_RELOAD_CLIP_GRAB_NEW							
	"ARDMrldP203",					// ANIM_RELOAD_CLIP_INSERT_NEW							
	"ARDMrldP204",					// ANIM_RELOAD_CLIP_SLAPIN_NEW		

	// Weapon switching:
	"ARDMswtchA3",					// ANIM_WEAPON_SWITCH_RELEASE_SINGLE
	"ARDMswtchB3",					// ANIM_WEAPON_SWITCH_RELEASE_DUAL
	"ARDMswtchC3",					// ANIM_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED
	"ARDMswtch_2",					// ANIM_WEAPON_SWITCH

	"ARDMrldD101",					// ANIM_RELOAD_ROCKET1_GRAB_AMMO,
	"ARDMrldD102",					// ANIM_RELOAD_ROCKET1_DROP_IN,
	"ARDMrldD103",					// ANIM_RELOAD_ROCKET1_FINISH,
	"ARDMfirD101",					// ANIM_FIRE_ROCKET1,
	"ARDMaimD101",					// ANIM_AIM_ROCKET1,
																							
	"ARDMaimpb01",					// ANIM_AIM_PILLBOX										
};																							
																							
																							
cchar *CBotMiner::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {								
	"ARMGidle001",																			
};																							
																							
																							
cchar *CBotMiner::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {					
	"stand",						// ANIMCONTROL_STAND,									
//	"stand_alert",					// ANIMCONTROL_STAND_ALERT,								
//	"turninplace",					// ANIMCONTROL_TURN_IN_PLACE,							
	"walk",							// ANIMCONTROL_WALK,									
//	"walk_alert",					// ANIMCONTROL_WALK_ALERT,								
	"run",							// ANIMCONTROL_RUN,
//	"run_panic",					// ANIMCONTROL_RUN_PANIC,

	"aim_summer",					// ANIMCONTROL_AIM_SUMMER,

	"jump_launch",					// ANIMCONTROL_JUMP_LAUNCH,
	"jump_land_L",					// ANIMCONTROL_JUMP_LAND_LOWER,
	"jump_land_U",					// ANIMCONTROL_JUMP_LAND_UPPER,
	"jump_fly",						// ANIMCONTROL_JUMP_FLY,
	"jump_tuck",					// ANIMCONTROL_JUMP_TUCK,
	"jump_untuck",					// ANIMCONTROL_JUMP_UNTUCK,
	"cable_reach",					// ANIMCONTROL_CABLE_REACH,
	"cable_grasp",					// ANIMCONTROL_CABLE_GRASP,
	"cable_release",				// ANIMCONTROL_CABLE_RELEASE,

	//"hop_l",						// ANIMCONTROL_HOP_LEFT,
	//"hop_rt",						// ANIMCONTROL_HOP_RIGHT,
	//"startle",						// ANIMCONTROL_STARTLE,
	//"roll_l",						// ANIMCONTROL_ROLL_LEFT,
	//"roll_r",						// ANIMCONTROL_ROLL_RIGHT,

	"fire1",						// ANIMCONTROL_FIRE_1,
	//"fire1_alert",					// ANIMCONTROL_FIRE_1_ALERT,
	"fire2_lower",					// ANIMCONTROL_FIRE_2_LOWER,
	"fire2_upper",					// ANIMCONTROL_FIRE_2_UPPER,

	//"tethered",						// ANIMCONTROL_RC_TETHERED,
	"power_up",						// ANIMCONTROL_RC_POWER_UP,
	"power_down",					// ANIMCONTROL_RC_POWER_DOWN,

	//"doze",							// ANIMCONTROL_DOZE_LOOP
	//"napjerk",						// ANIMCONTROL_NAPJERK
	//"wake",							// ANIMCONTROL_WAKE

	"clip_rld_eject",				// ANIMCONTROL_RELOAD_CLIP_EJECT_OLD
	"clip_rld_grab",				// ANIMCONTROL_RELOAD_CLIP_GRAB_NEW
	"clip_rld_insert",				// ANIMCONTROL_RELOAD_CLIP_INSERT_NEW
	"clip_rld_slapin",				// ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW

	"ws_release_s",					// ANIMCONTROL_WEAPON_SWITCH_RELEASE_SINGLE,
	"ws_release_d",					// ANIMCONTROL_WEAPON_SWITCH_RELEASE_DUAL,
	"ws_release_RL23",				// ANIMCONTROL_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
	"ws_switch",					// ANIMCONTROL_WEAPON_SWITCH,

	"rocket1_rld_G",				// ANIMCONTROL_RELOAD_ROCKET1_GRAB_AMMO,
	"rocket1_rld_D",				// ANIMCONTROL_RELOAD_ROCKET1_DROP_IN,
	"rocket1_rld_F",				// ANIMCONTROL_RELOAD_ROCKET1_FINISH,
	"rocket1_fire",					// ANIMCONTROL_FIRE_ROCKET1,
	"rocket1_aim",					// ANIMCONTROL_AIM_ROCKET1,


	"aim_pillbox",					// ANIMCONTROL_AIM_PILLBOX
};


cchar *CBotMiner::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",						// ANIMTAP_STAND,
	//"stand_alert",					// ANIMTAP_STAND_ALERT,
	//"turninplace",					// ANIMTAP_TURN_IN_PLACE,
	"walk",							// ANIMTAP_WALK,
	//"walk_alert",					// ANIMTAP_WALK_ALERT,
	"run",							// ANIMTAP_RUN,
	//"run_panic",					// ANIMTAP_RUN_PANIC,

	"aim_summer",					// ANIMTAP_AIM_SUMMER,

	"jump_launch",					// ANIMTAP_JUMP_LAUNCH,
	"jump_land_L",					// ANIMTAP_JUMP_LAND_LOWER,
	"jump_land_U",					// ANIMTAP_JUMP_LAND_UPPER,
	"jump_fly",						// ANIMTAP_JUMP_FLY,
	"jump_tuck",					// ANIMTAP_JUMP_TUCK,
	"jump_untuck",					// ANIMTAP_JUMP_UNTUCK,
	"cable_reach",					// ANIMTAP_CABLE_REACH,
	"cable_grasp",					// ANIMTAP_CABLE_GRASP,
	"cable_release",				// ANIMTAP_CABLE_RELEASE,

	//"hop_l",						// ANIMTAP_HOP_LEFT,
	//"hop_r",						// ANIMTAP_HOP_RIGHT,
	//"startle",						// ANIMTAP_STARTLE,
	//"roll_l",						// ANIMTAP_ROLL_LEFT,
	//"roll_r",						// ANIMTAP_ROLL_RIGHT,


	"fire1",						// ANIMTAP_FIRE_1,
	//"fire1_alert",					// ANIMTAP_FIRE_1_ALERT,
	"fire2_lower",					// ANIMTAP_FIRE_2_LOWER,
	"fire2_upper",					// ANIMTAP_FIRE_2_UPPER,

	//"tethered",						// ANIMTAP_RC_TETHERED,
	"power_up",						// ANIMTAP_RC_POWER_UP,
	"power_down",					// ANIMTAP_RC_POWER_DOWN,

	//"doze",							// ANIMTAP_DOZE_LOOP
	//"jerk",							// ANIMTAP_NAPJERK
	//"wake",							// ANIMTAP_WAKE

	"clip_rld_eject",				// ANIMTAP_RELOAD_CLIP_EJECT_OLD
	"clip_rld_grab",				// ANIMTAP_RELOAD_CLIP_GRAB_NEW
	"clip_rld_insert",				// ANIMTAP_RELOAD_CLIP_INSERT_NEW
	"clip_rld_slapin",				// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW
	
	"ws_release_s",					// ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE,
	"ws_release_d",					// ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL,
	"ws_release_RL23",				// ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
	"ws_switch",					// ANIMTAP_WEAPON_SWITCH,

	"rocket1_rld_G",				// ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO,
	"rocket1_rld_D",				// ANIMTAP_RELOAD_ROCKET1_DROP_IN,
	"rocket1_rld_F",				// ANIMTAP_RELOAD_ROCKET1_FINISH,
	"rocket1_fire",					// ANIMTAP_FIRE_ROCKET1,
	"rocket1_aim",					// ANIMTAP_AIM_ROCKET1,

	"aim_pillbox",					// ANIMTAP_AIM_PILLBOX
};


CFAnimCombinerConfig::ConfigStack_t CBotMiner::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_AIM_PILLBOX,

	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_TETHERED,

	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WAKE,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_NAPJERK,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_DOZE_LOOP,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_1,
//	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_1_ALERT,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_2_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_2_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_ROCKET1,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WEAPON_SWITCH_RELEASE_SINGLE,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WEAPON_SWITCH_RELEASE_DUAL,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WEAPON_SWITCH,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_ROCKET1_GRAB_AMMO,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_ROCKET1_DROP_IN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_ROCKET1_FINISH,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CABLE_REACH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CABLE_RELEASE,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CABLE_GRASP,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_TUCK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_UNTUCK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_FLY,

	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_HOP_LEFT,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_HOP_RIGHT,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STARTLE,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ROLL_LEFT,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ROLL_RIGHT,

	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_AIM_ROCKET1,

	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN_PANIC,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_TURN_IN_PLACE,	// really sneak
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK_ALERT,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_DOWN,
	//CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_ALERT,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotMiner::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,								ANIMCONTROL_STAND,							1,
	//ANIMTAP_STAND_ALERT,						ANIMCONTROL_STAND_ALERT,					1,
	//ANIMTAP_TURN_IN_PLACE,						ANIMCONTROL_TURN_IN_PLACE,					1,
	ANIMTAP_WALK,								ANIMCONTROL_WALK,							1,
	//ANIMTAP_WALK_ALERT,							ANIMCONTROL_WALK_ALERT,						1,
	ANIMTAP_RUN,								ANIMCONTROL_RUN,							1,
	//ANIMTAP_RUN_PANIC,							ANIMCONTROL_RUN_PANIC,						1,

	ANIMTAP_CABLE_REACH,						ANIMCONTROL_CABLE_REACH,					1,
	ANIMTAP_CABLE_GRASP,						ANIMCONTROL_CABLE_GRASP,					1,
	ANIMTAP_CABLE_RELEASE,						ANIMCONTROL_CABLE_RELEASE,					1,
	ANIMTAP_JUMP_UNTUCK,						ANIMCONTROL_JUMP_UNTUCK,					1,
	ANIMTAP_JUMP_TUCK,							ANIMCONTROL_JUMP_TUCK,						1,
	ANIMTAP_JUMP_FLY,							ANIMCONTROL_JUMP_FLY,						1,
	ANIMTAP_JUMP_LAND_LOWER,					ANIMCONTROL_JUMP_LAND_LOWER,				1,
	ANIMTAP_JUMP_LAND_UPPER,					ANIMCONTROL_JUMP_LAND_UPPER,				1,
	ANIMTAP_JUMP_LAUNCH,						ANIMCONTROL_JUMP_LAUNCH,					1,

	//ANIMTAP_HOP_LEFT,							ANIMCONTROL_HOP_LEFT,						1,
	//ANIMTAP_HOP_RIGHT,							ANIMCONTROL_HOP_RIGHT,						1,
	//ANIMTAP_STARTLE,							ANIMCONTROL_STARTLE,						1,
	//ANIMTAP_ROLL_LEFT,							ANIMCONTROL_ROLL_LEFT,						1,
	//ANIMTAP_ROLL_RIGHT,							ANIMCONTROL_ROLL_RIGHT,						1,

	ANIMTAP_AIM_SUMMER,							ANIMCONTROL_AIM_SUMMER,						1,

	ANIMTAP_FIRE_1,								ANIMCONTROL_FIRE_1,							1,
//	ANIMTAP_FIRE_1_ALERT,						ANIMCONTROL_FIRE_1_ALERT,					1,
	ANIMTAP_FIRE_2_UPPER,						ANIMCONTROL_FIRE_2_UPPER,					1,
	ANIMTAP_FIRE_2_LOWER,						ANIMCONTROL_FIRE_2_LOWER,					1,

	//ANIMTAP_RC_TETHERED,						ANIMCONTROL_RC_TETHERED,					1,
	ANIMTAP_RC_POWER_UP,						ANIMCONTROL_RC_POWER_UP,					1,
	ANIMTAP_RC_POWER_DOWN,						ANIMCONTROL_RC_POWER_DOWN,					1,

	//ANIMTAP_DOZE_LOOP,							ANIMCONTROL_DOZE_LOOP, 						1,
	//ANIMTAP_NAPJERK,							ANIMCONTROL_NAPJERK,						1,
	//ANIMTAP_WAKE,								ANIMCONTROL_WAKE,							1,

	ANIMTAP_RELOAD_CLIP_EJECT_OLD,				ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,			1,
	ANIMTAP_RELOAD_CLIP_GRAB_NEW,				ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,			1,
	ANIMTAP_RELOAD_CLIP_INSERT_NEW,				ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,			1,
	ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,				ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,			1,

	ANIMTAP_WEAPON_SWITCH,							ANIMCONTROL_WEAPON_SWITCH,							1,
	ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,	ANIMCONTROL_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,	1,
	ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL,				ANIMCONTROL_WEAPON_SWITCH_RELEASE_DUAL,				1,
	ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE,			ANIMCONTROL_WEAPON_SWITCH_RELEASE_SINGLE,			1,

	ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO,			ANIMCONTROL_RELOAD_ROCKET1_GRAB_AMMO,		1,
	ANIMTAP_RELOAD_ROCKET1_DROP_IN,				ANIMCONTROL_RELOAD_ROCKET1_DROP_IN,			1,
	ANIMTAP_RELOAD_ROCKET1_FINISH,				ANIMCONTROL_RELOAD_ROCKET1_FINISH,			1,
	ANIMTAP_FIRE_ROCKET1,						ANIMCONTROL_FIRE_ROCKET1,					1,
	ANIMTAP_AIM_ROCKET1,						ANIMCONTROL_AIM_ROCKET1,					1,

	ANIMTAP_AIM_PILLBOX,						ANIMCONTROL_AIM_PILLBOX,					1,
	255,										255,										255
};


CFAnimCombiner::AttachList_t CBotMiner::m_aAnimAttach[] = {
	ANIM_STAND,										ANIMTAP_STAND,
	//ANIM_STAND_ALERT,								ANIMTAP_STAND_ALERT,
	//ANIM_TURN_IN_PLACE,							ANIMTAP_TURN_IN_PLACE,
	ANIM_WALK_FORWARD,								ANIMTAP_WALK,
	//ANIM_WALK_ALERT,								ANIMTAP_WALK_ALERT,
	ANIM_RUN_FORWARD,								ANIMTAP_RUN,
	//ANIM_RUN_PANIC,									ANIMTAP_RUN_PANIC,

	ANIM_JUMP_LAUNCH,								ANIMTAP_JUMP_LAUNCH,
	ANIM_JUMP_LAND,									ANIMTAP_JUMP_LAND_LOWER,
	ANIM_JUMP_LAND,									ANIMTAP_JUMP_LAND_UPPER,
	ANIM_JUMP_FLY,									ANIMTAP_JUMP_FLY,
	ANIM_JUMP_TUCK,									ANIMTAP_JUMP_TUCK,
	ANIM_JUMP_UNTUCK,								ANIMTAP_JUMP_UNTUCK,
	ANIM_CABLE_REACH,								ANIMTAP_CABLE_REACH,
	ANIM_CABLE_GRASP,								ANIMTAP_CABLE_GRASP,
	ANIM_CABLE_RELEASE,								ANIMTAP_CABLE_RELEASE,

	//ANIM_HOP_LEFT,								ANIMTAP_HOP_LEFT,
	//ANIM_HOP_RIGHT,								ANIMTAP_HOP_RIGHT,
	//ANIM_STARTLE,									ANIMTAP_STARTLE,
	//ANIM_ROLL_LEFT,									ANIMTAP_ROLL_LEFT,
	//ANIM_ROLL_RIGHT,								ANIMTAP_ROLL_RIGHT,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,					ANIMTAP_AIM_SUMMER,

	ANIM_FIRE_PRIMARY,								ANIMTAP_FIRE_1,
	//ANIM_FIRE_PRIMARY_ALERT,						ANIMTAP_FIRE_1_ALERT,
	ANIM_FIRE_SECONDARY,							ANIMTAP_FIRE_2_LOWER,
	ANIM_FIRE_SECONDARY,							ANIMTAP_FIRE_2_UPPER,

	//ANIM_RC_TETHERED,								ANIMTAP_RC_TETHERED,
	ANIM_RC_POWER_UP,								ANIMTAP_RC_POWER_UP,
	ANIM_RC_POWER_DOWN,								ANIMTAP_RC_POWER_DOWN,

	//ANIM_DOZE_LOOP,									ANIMTAP_DOZE_LOOP,
	//ANIM_NAPJERK,									ANIMTAP_NAPJERK,
	//ANIM_WAKE,										ANIMTAP_WAKE,	

	ANIM_RELOAD_CLIP_EJECT_OLD,						ANIMTAP_RELOAD_CLIP_EJECT_OLD,
	ANIM_RELOAD_CLIP_GRAB_NEW,						ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	ANIM_RELOAD_CLIP_INSERT_NEW,					ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	ANIM_RELOAD_CLIP_SLAPIN_NEW,					ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

	ANIM_WEAPON_SWITCH_RELEASE_SINGLE,				ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE,
	ANIM_WEAPON_SWITCH_RELEASE_DUAL,				ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL,
	ANIM_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,	ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
	ANIM_WEAPON_SWITCH,								ANIMTAP_WEAPON_SWITCH,

	ANIM_RELOAD_ROCKET1_GRAB_AMMO,				ANIMTAP_RELOAD_ROCKET1_GRAB_AMMO,
	ANIM_RELOAD_ROCKET1_DROP_IN,				ANIMTAP_RELOAD_ROCKET1_DROP_IN,
	ANIM_RELOAD_ROCKET1_FINISH,					ANIMTAP_RELOAD_ROCKET1_FINISH,
	ANIM_FIRE_ROCKET1,							ANIMTAP_FIRE_ROCKET1,
	ANIM_AIM_ROCKET1,							ANIMTAP_AIM_ROCKET1,

	ANIM_AIM_PILLBOX,								ANIMTAP_AIM_PILLBOX,
	255,											255
};


const u8 CBotMiner::m_aBoneEnableIndices_FullBody[] = {
	BONE_GROIN,					
	BONE_TORSO,					
	BONE_HEAD,					

	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	BONE_FLAP,					
	BONE_BACKPACK,				
	BONE_ANTENNA_A,				
	BONE_ANTENNA_B,				
	BONE_ANTENNA_C,				
	BONE_ANTENNA_D,				
	BONE_ANTENNA_E,				
	BONE_MINER_DUMMY,			

	BONE_LEFT_HIP,				
	BONE_LEFT_LEG_UPPER,		
	BONE_LEFT_KNEE,				
	BONE_LEFT_LEG_LOWER,		
	BONE_LEFT_FOOT_BACK,		
	BONE_LEFT_HEEL,				
	BONE_LEFT_FOOT_FRONT,		
	BONE_LEFT_TOE,				

	BONE_RIGHT_HIP,				
	BONE_RIGHT_LEG_UPPER,		
	BONE_RIGHT_KNEE,			
	BONE_RIGHT_LEG_LOWER,		
	BONE_RIGHT_FOOT_BACK,		
	BONE_RIGHT_HEEL,			
	BONE_RIGHT_FOOT_FRONT,		
	BONE_RIGHT_TOE,				

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_AimSummer[] = {
	BONE_HEAD,
	BONE_TORSO,

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_TapFire1[] = {
	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	255
};


//const u8 CBotMiner::m_aBoneEnableIndices_TapFire1_Alert[] = {
//	BONE_RIGHT_ARM_UPPER,
//	BONE_RIGHT_ELBOW,
//	BONE_RIGHT_ARM_LOWER,
//	BONE_ATTACHPOINT_PRIMARY,
//
//	BONE_LEFT_ARM_UPPER,
//	BONE_LEFT_ELBOW,
//	BONE_LEFT_ARM_LOWER,
//	BONE_LEFT_THUMB,
//	BONE_LEFT_FINGER_A,
//	BONE_LEFT_FINGER_B,
//	BONE_LEFT_FINGER_C,
//
//	255
//};


const u8 CBotMiner::m_aBoneEnableIndices_JumpLand_Lower[] = {
	BONE_GROIN,					

	BONE_LEFT_HIP,				
	BONE_LEFT_LEG_UPPER,		
	BONE_LEFT_KNEE,				
	BONE_LEFT_LEG_LOWER,		
	BONE_LEFT_FOOT_BACK,		
	BONE_LEFT_HEEL,				
	BONE_LEFT_FOOT_FRONT,		
	BONE_LEFT_TOE,				

	BONE_RIGHT_HIP,				
	BONE_RIGHT_LEG_UPPER,		
	BONE_RIGHT_KNEE,			
	BONE_RIGHT_LEG_LOWER,		
	BONE_RIGHT_FOOT_BACK,		
	BONE_RIGHT_HEEL,			
	BONE_RIGHT_FOOT_FRONT,		
	BONE_RIGHT_TOE,				

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_JumpLand_Upper[] = {
	BONE_TORSO,					
	BONE_HEAD,					

	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	BONE_FLAP,					
	BONE_BACKPACK,				
	BONE_ANTENNA_A,				
	BONE_ANTENNA_B,				
	BONE_ANTENNA_C,				
	BONE_ANTENNA_D,				
	BONE_ANTENNA_E,				
	BONE_MINER_DUMMY,			

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_TapFire2_Lower[] = {
	BONE_GROIN,					

	BONE_LEFT_HIP,				
	BONE_LEFT_LEG_UPPER,		
	BONE_LEFT_KNEE,				
	BONE_LEFT_LEG_LOWER,		
	BONE_LEFT_FOOT_BACK,		
	BONE_LEFT_HEEL,				
	BONE_LEFT_FOOT_FRONT,		
	BONE_LEFT_TOE,				

	BONE_RIGHT_HIP,				
	BONE_RIGHT_LEG_UPPER,		
	BONE_RIGHT_KNEE,			
	BONE_RIGHT_LEG_LOWER,		
	BONE_RIGHT_FOOT_BACK,		
	BONE_RIGHT_HEEL,			
	BONE_RIGHT_FOOT_FRONT,		
	BONE_RIGHT_TOE,				

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_ReloadSingle_Arm[] = {
	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_ReloadSingle_Body[] = {
	BONE_TORSO,
	BONE_HEAD,

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_TapFire2_Upper[] = {
	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_ReloadClip[] = {
	BONE_TORSO,					
	BONE_HEAD,					

	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	BONE_FLAP,					
	BONE_BACKPACK,				
	BONE_ANTENNA_A,				
	BONE_ANTENNA_B,				
	BONE_ANTENNA_C,				
	BONE_ANTENNA_D,				
	BONE_ANTENNA_E,				
	BONE_MINER_DUMMY,			

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_UserAnim_UpperBody[] = {
	BONE_TORSO,					
	BONE_HEAD,					

	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	BONE_FLAP,					
	BONE_BACKPACK,				
	BONE_ANTENNA_A,				
	BONE_ANTENNA_B,				
	BONE_ANTENNA_C,				
	BONE_ANTENNA_D,				
	BONE_ANTENNA_E,				
	BONE_MINER_DUMMY,			

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_UserAnim_LowerBody[] = {
	BONE_LEFT_HIP,				
	BONE_LEFT_LEG_UPPER,		
	BONE_LEFT_KNEE,				
	BONE_LEFT_LEG_LOWER,		
	BONE_LEFT_FOOT_BACK,		
	BONE_LEFT_HEEL,				
	BONE_LEFT_FOOT_FRONT,		
	BONE_LEFT_TOE,				

	BONE_RIGHT_HIP,				
	BONE_RIGHT_LEG_UPPER,		
	BONE_RIGHT_KNEE,			
	BONE_RIGHT_LEG_LOWER,		
	BONE_RIGHT_FOOT_BACK,		
	BONE_RIGHT_HEEL,			
	BONE_RIGHT_FOOT_FRONT,		
	BONE_RIGHT_TOE,				

	BONE_GROIN,

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_UserAnim_UpperTorso[] = {
	BONE_TORSO,					
	BONE_FLAP,					
	BONE_BACKPACK,				
	BONE_ANTENNA_A,				
	BONE_ANTENNA_B,				
	BONE_ANTENNA_C,				
	BONE_ANTENNA_D,				
	BONE_ANTENNA_E,				
	BONE_MINER_DUMMY,			

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_UserAnim_LowerTorso[] = {
	BONE_GROIN,					

	BONE_LEFT_HIP,				
	BONE_LEFT_LEG_UPPER,		
	BONE_LEFT_KNEE,				
	BONE_LEFT_LEG_LOWER,		

	BONE_RIGHT_HIP,				
	BONE_RIGHT_LEG_UPPER,		
	BONE_RIGHT_KNEE,			
	BONE_RIGHT_LEG_LOWER,		

	BONE_GROIN,

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_UserAnim_LeftArm[] = {
	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_UserAnim_RightArm[] = {
	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_UserAnim_Head[] = {
	BONE_HEAD,

	255
};

const u8 CBotMiner::m_aBoneEnableIndices_Cable[] = {
	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_AimRocket1[] = {
	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	255
};


const u8 CBotMiner::m_aBoneEnableIndices_FireRocket1[] = {
	BONE_TORSO,					
	BONE_HEAD,					

	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	BONE_FLAP,					
	BONE_BACKPACK,				
	BONE_ANTENNA_A,				
	BONE_ANTENNA_B,				
	BONE_ANTENNA_C,				
	BONE_ANTENNA_D,				
	BONE_ANTENNA_E,			

	255
};


const u8 *CBotMiner::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND,
	//m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND_ALERT,
	//m_aBoneEnableIndices_FullBody,				// ANIMTAP_TURN_IN_PLACE,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_WALK,
	//m_aBoneEnableIndices_FullBody,				// ANIMTAP_WALK_ALERT,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RUN,
	//m_aBoneEnableIndices_FullBody,				// ANIMTAP_RUN_PANIC,

	m_aBoneEnableIndices_AimSummer,				// ANIMTAP_AIM_SUMMER,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_LAUNCH,
	m_aBoneEnableIndices_JumpLand_Lower,		// ANIMTAP_JUMP_LAND_LOWER,
	m_aBoneEnableIndices_JumpLand_Upper,		// ANIMTAP_JUMP_LAND_UPPER,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_FLY,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_TUCK,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_JUMP_UNTUCK,
	m_aBoneEnableIndices_Cable,					// ANIMTAP_CABLE_REACH,
	m_aBoneEnableIndices_Cable,					// ANIMTAP_CABLE_GRASP,
	m_aBoneEnableIndices_Cable,					// ANIMTAP_CABLE_RELEASE,

	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_HOP_LEFT,
	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_HOP_RIGHT,
	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_STARTLE
	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_ROLL_LEFT,
	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_ROLL_RIGHT,

	m_aBoneEnableIndices_TapFire1,				// ANIMTAP_FIRE_1,
	//m_aBoneEnableIndices_TapFire1_Alert,		// ANIMTAP_FIRE_1_ALERT,
	m_aBoneEnableIndices_TapFire2_Lower,		// ANIMTAP_FIRE_2_LOWER,
	m_aBoneEnableIndices_TapFire2_Upper,		// ANIMTAP_FIRE_2_UPPER,

	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_RC_TETHERED,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_DOWN
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_UP

	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_DOZE_LOOP
	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_NAPJERK
	//m_aBoneEnableIndices_FullBody,			// ANIMTAP_WAKE

	m_aBoneEnableIndices_ReloadClip,			// ANIMTAP_RELOAD_CLIP_EJECT_OLD
	m_aBoneEnableIndices_ReloadClip,			// ANIMTAP_RELOAD_CLIP_GRAB_NEW
	m_aBoneEnableIndices_ReloadClip,			// ANIMTAP_RELOAD_CLIP_INSERT_NEW
	m_aBoneEnableIndices_ReloadClip,			// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW

	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_WEAPON_SWITCH_RELEASE_SINGLE,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_WEAPON_SWITCH_RELEASE_DUAL,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_WEAPON_SWITCH_RELEASE_SHOULDER_MOUNTED,
	m_aBoneEnableIndices_UserAnim_UpperBody,	// ANIMTAP_WEAPON_SWITCH,

	m_aBoneEnableIndices_ReloadClip,		// ANIMTAP_RELOAD_ROCKET1_GRAB_A
	m_aBoneEnableIndices_ReloadClip,		// ANIMTAP_RELOAD_ROCKET1_DROP_I
	m_aBoneEnableIndices_ReloadClip,		// ANIMTAP_RELOAD_ROCKET1_FINISH
	m_aBoneEnableIndices_FireRocket1,		// ANIMTAP_FIRE_ROCKET1,
	m_aBoneEnableIndices_AimRocket1,		// ANIMTAP_AIM_ROCKET1,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_AIM_PILLBOX
};


const u8 *CBotMiner::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody,					// Idle 0
};


const u8 CBotMiner::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:

	255
};


const u8 CBotMiner::m_anEnableBoneNameIndexTableForSummer_TetherShock[] = {
	// Enable these bones to be driven by our summer:

	BONE_GROIN,					
	BONE_TORSO,					
	BONE_HEAD,					

	BONE_LEFT_SHOULDER,			
	BONE_LEFT_ARM_UPPER,		
	BONE_LEFT_ARM_ELBOW,		
	BONE_LEFT_ARM_LOWER,		
	BONE_LEFT_HAND,				
	BONE_SECONDARY_FIRE,		

	BONE_LEFT_THUMB_A,			
	BONE_LEFT_INDEXFINGER_A,	
	BONE_LEFT_MIDDLEFINGER_A,	
	BONE_LEFT_PINKYFINGER_A,	

	BONE_RIGHT_SHOULDER,		
	BONE_RIGHT_ARM_UPPER,		
	BONE_RIGHT_ARM_LOWER,		

	BONE_FLAP,					
	BONE_BACKPACK,				
	BONE_ANTENNA_A,				
	BONE_ANTENNA_B,				
	BONE_ANTENNA_C,				
	BONE_ANTENNA_D,				
	BONE_ANTENNA_E,				
	BONE_MINER_DUMMY,			

	BONE_LEFT_HIP,				
	BONE_LEFT_LEG_UPPER,		
	BONE_LEFT_KNEE,				
	BONE_LEFT_LEG_LOWER,		
	BONE_LEFT_FOOT_BACK,		
	BONE_LEFT_HEEL,				
	BONE_LEFT_FOOT_FRONT,		
	BONE_LEFT_TOE,				

	BONE_RIGHT_HIP,				
	BONE_RIGHT_LEG_UPPER,		
	BONE_RIGHT_KNEE,			
	BONE_RIGHT_LEG_LOWER,		
	BONE_RIGHT_FOOT_BACK,		
	BONE_RIGHT_HEEL,			
	BONE_RIGHT_FOOT_FRONT,		
	BONE_RIGHT_TOE,				

	255
};


const u8 CBotMiner::m_anTagPointBoneNameIndexArray[] = {
	BONE_HEAD,
	BONE_TORSO,
	BONE_LEFT_ARM_UPPER,
	BONE_RIGHT_ARM_UPPER,

	255
};


const u8 CBotMiner::m_nApproxEyePointBoneNameIndex = BONE_HEAD;


BOOL CBotMiner::_BuildAnimStackDef( void ) {
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
