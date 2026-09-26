//////////////////////////////////////////////////////////////////////////////////////
// botswarmerboss_data.cpp - Swarmer Boss bot data
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
// 03.31.03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#include "fang.h"
#include "fanim.h"
#include "botswarmerboss.h"
#include "botanim.h"
#include "fmesh.h"

const CBotDef CBotSwarmerBoss::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_SWARMER_BOSS,					// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CBotSwarmerBoss::m_apszBoneNameTable[BONE_COUNT] = {
	"SwarmerBoss_Dummy",		// BONE_DUMMY,			
	"Torso",					// BONE_TORSO,			
	"L_Front_Leg_A",			// BONE_LEG_LF_A,		
	"L_Front_Leg_B",			// BONE_LEG_LF_B,		
	"L_Front_Leg_C",			// BONE_LEG_LF_C,		
	"L_Front_Leg_D",			// BONE_LEG_LF_D,		
	"L_Front_Leg_E",			// BONE_LEG_LF_E,		
	"L_Front_Foot_Dummy",		// BONE_LEG_LF_DUMMY,	
										
	"R_Front_Leg_A",			// BONE_LEG_RF_A,		
	"R_Front_Leg_B",			// BONE_LEG_RF_B,		
	"R_Front_Leg_C",			// BONE_LEG_RF_C,		
	"R_Front_Leg_D",			// BONE_LEG_RF_D,		
	"R_Front_Leg_E",			// BONE_LEG_RF_E,		
	"R_Front_Foot_Dummy",		// BONE_LEG_RF_DUMMY,	
								
	"L_Diode",					// BONE_DIODE_L,		
	"R_Diode",					// BONE_DIODE_R,		
	"Diode_Front",				// BONE_DIODE_FRONT,	
	"Ass",						// BONE_ASS,			
								
	"L_Rear_Leg_A",				// BONE_LEG_LR_A,		
	"L_Rear_Leg_B",				// BONE_LEG_LR_B,		
	"L_Rear_Leg_C",				// BONE_LEG_LR_C,		
	"L_Rear_Leg_D",				// BONE_LEG_LR_D,		
	"L_Rear_Leg_E",				// BONE_LEG_LR_E,		
	"L_Rear_Foot_Dummy",		// BONE_LEG_LR_DUMMY,	
								
	"R_Rear_Leg_A",				// BONE_LEG_RR_A,		
	"R_Rear_Leg_B",				// BONE_LEG_RR_B,		
	"R_Rear_Leg_C",				// BONE_LEG_RR_C,		
	"R_Rear_Leg_D",				// BONE_LEG_RR_D,		
	"R_Rear_Leg_E",				// BONE_LEG_RR_E,		
	"R_Rear_Foot_Dummy",		// BONE_LEG_RR_DUMMY,	
								
	"Thorax_A",					// BONE_THORAX_A,		
	"Thorax_B",					// BONE_THORAX_B,		
	"Thorax_C",					// BONE_THORAX_C,		
	"Thorax_D",					// BONE_THORAX_D,		
								
	"Primary_Fire",				// BONE_PRIFIRE,		
	"Head",						// BONE_HEAD,			
	"R_Mandible",				// BONE_MANDIBLE_R,	
	"L_Mandible",				// BONE_MANDIBLE_L,	
									
	"R_Feeler_3_A",				// BONE_FEELER_R3A,	
	"R_Feeler_3_B",				// BONE_FEELER_R3B,	
	"R_Feeler_3_C",				// BONE_FEELER_R3C,	
	"R_Feeler_3_D",				// BONE_FEELER_R3D,	
	"R_Feeler_1_A",				// BONE_FEELER_R1A,	
	"R_Feeler_1_B",				// BONE_FEELER_R1B,	
	"R_Feeler_1_C",				// BONE_FEELER_R1C,	
	"R_Feeler_1_D",				// BONE_FEELER_R1D,	
	"R_Feeler_2_A",				// BONE_FEELER_R2A,	
	"R_Feeler_2_B",				// BONE_FEELER_R2B,	
	"R_Feeler_2_C",				// BONE_FEELER_R2C,	
	"R_Feeler_2_D",				// BONE_FEELER_R2D,	
	"L_Feeler_3_A",				// BONE_FEELER_L3A,	
	"L_Feeler_3_B",				// BONE_FEELER_L3B,	
	"L_Feeler_3_C",				// BONE_FEELER_L3C,	
	"L_Feeler_3_D",				// BONE_FEELER_L3D,	
	"L_Feeler_2_A",				// BONE_FEELER_L1A,	
	"L_Feeler_2_B",				// BONE_FEELER_L1B,	
	"L_Feeler_2_C",				// BONE_FEELER_L1C,	
	"L_Feeler_2_D",				// BONE_FEELER_L1D,	
	"L_Feeler_1_A",				// BONE_FEELER_L2A,	
	"L_Feeler_1_B",				// BONE_FEELER_L2B,	
	"L_Feeler_1_C",				// BONE_FEELER_L2C,	
	"L_Feeler_1_D",				// BONE_FEELER_L2D,	
};


cchar *CBotSwarmerBoss::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMAidle001",		// ANIM_STAND,		
	"ARMAwalkF01",		// ANIM_WALK,			
							   
	"ARMAjump_01",		// ANIM_JUMP_LAUNCH,	
	"ARMAjump_02",		// ANIM_JUMP_FLY,		
	"ARMAjump_03",		// ANIM_JUMP_LAND,	
						   
	"ARMAlower01",		// ANIM_LOWER,		
	"ARMAslash01",		// ANIM_SLASH,		
	"ARMAaimF_01",		// ANIM_FIRE,			
};



cchar *CBotSwarmerBoss::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",		// ANIMCONTROL_STAND,
	"walk",			// ANIMCONTROL_WALK,
		
	"jump_launch",	// ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",		// ANIMCONTROL_JUMP_FLY,
	"jump_land",	// ANIMCONTROL_JUMP_LAND,

	"lower",		// ANIMCONTROL_LOWER,
	"slash",		// ANIMCONTROL_SLASH,
	"fire",			// ANIMCONTROL_FIRE,
};


cchar *CBotSwarmerBoss::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",		// ANIMTAP_STAND,
	"walk",			// ANIMTAP_WALK,
		
	"jump_launch",	// ANIMTAP_JUMP_LAUNCH,
	"jump_fly",		// ANIMTAP_JUMP_FLY,
	"jump_land",	// ANIMTAP_JUMP_LAND,

	"lower",		// ANIMTAP_LOWER,
	"slash",		// ANIMTAP_SLASH,
	"fire",			// ANIMTAP_FIRE,
};


CFAnimCombinerConfig::ConfigStack_t CBotSwarmerBoss::m_aAnimCombinerConfigStack[] = {
	// from highest to lowest priority

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_SLASH,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_FIRE,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_LAND,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_JUMP_FLY,
	
									
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_LOWER,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_WALK,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,		255
};


CFAnimCombinerConfig::ConfigTap_t	CBotSwarmerBoss::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,				ANIMCONTROL_STAND,			1,
	ANIMTAP_WALK,				ANIMCONTROL_WALK,			1,
			
	ANIMTAP_JUMP_LAUNCH,		ANIMCONTROL_JUMP_LAUNCH,	1,
	ANIMTAP_JUMP_FLY,			ANIMCONTROL_JUMP_FLY,		1,
	ANIMTAP_JUMP_LAND,			ANIMCONTROL_JUMP_LAND,		1,

	ANIMTAP_LOWER,				ANIMCONTROL_LOWER,			1,
	ANIMTAP_SLASH,				ANIMCONTROL_SLASH,			1,
	ANIMTAP_FIRE,				ANIMCONTROL_FIRE,			1,
	
	255,						255,						255
};


CFAnimCombiner::AttachList_t	CBotSwarmerBoss::m_aAnimAttach[] = {
	ANIM_STAND,					ANIMTAP_STAND,				
	ANIM_WALK,					ANIMTAP_WALK,				
		   					
	ANIM_JUMP_LAUNCH,			ANIMTAP_JUMP_LAUNCH,		
	ANIM_JUMP_FLY,				ANIMTAP_JUMP_FLY,			
	ANIM_JUMP_LAND,				ANIMTAP_JUMP_LAND,			

	ANIM_LOWER,					ANIMTAP_LOWER,				
	ANIM_SLASH,					ANIMTAP_SLASH,				
	ANIM_FIRE,					ANIMTAP_FIRE,				

	255,						255
};


const u8* CBotSwarmerBoss::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND,			
	m_aBoneEnableIndices_MainBody,				// ANIMTAP_WALK,			
	
	m_aBoneEnableIndices_MainBody,				// ANIMTAP_JUMP_LAUNCH,	
	m_aBoneEnableIndices_MainBody,				// ANIMTAP_JUMP_FLY,		
	m_aBoneEnableIndices_MainBody,				// ANIMTAP_JUMP_LAND,		
	
	m_aBoneEnableIndices_MainBody,				// ANIMTAP_LOWER,			
	m_aBoneEnableIndices_MainBody,				// ANIMTAP_SLASH,			
	m_aBoneEnableIndices_MainBody,				// ANIMTAP_FIRE,		
};


const u8* CBotSwarmerBoss::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// Idle 0
};


const u8 CBotSwarmerBoss::m_aBoneEnableIndices_FullBody[] = {
	BONE_DUMMY,			
	BONE_TORSO,			
	BONE_LEG_LF_A,		
	BONE_LEG_LF_B,		
	BONE_LEG_LF_C,		
	BONE_LEG_LF_D,		
	BONE_LEG_LF_E,		
	BONE_LEG_LF_DUMMY,	
			
	BONE_LEG_RF_A,		
	BONE_LEG_RF_B,		
	BONE_LEG_RF_C,		
	BONE_LEG_RF_D,		
	BONE_LEG_RF_E,		
	BONE_LEG_RF_DUMMY,	

	BONE_DIODE_L,		
	BONE_DIODE_R,		
	BONE_DIODE_FRONT,	
//	BONE_ASS,			

	BONE_LEG_LR_A,		
	BONE_LEG_LR_B,		
	BONE_LEG_LR_C,		
	BONE_LEG_LR_D,		
	BONE_LEG_LR_E,		
	BONE_LEG_LR_DUMMY,	

	BONE_LEG_RR_A,		
	BONE_LEG_RR_B,		
	BONE_LEG_RR_C,		
	BONE_LEG_RR_D,		
	BONE_LEG_RR_E,		
	BONE_LEG_RR_DUMMY,	

	BONE_THORAX_A,		
	BONE_THORAX_B,		
	BONE_THORAX_C,		
	BONE_THORAX_D,		

	BONE_PRIFIRE,		
	BONE_HEAD,			
	BONE_MANDIBLE_R,	
	BONE_MANDIBLE_L,	
		
	BONE_FEELER_R3A,	
	BONE_FEELER_R3B,	
	BONE_FEELER_R3C,	
	BONE_FEELER_R3D,	
	BONE_FEELER_R1A,	
	BONE_FEELER_R1B,	
	BONE_FEELER_R1C,	
	BONE_FEELER_R1D,	
	BONE_FEELER_R2A,	
	BONE_FEELER_R2B,	
	BONE_FEELER_R2C,	
	BONE_FEELER_R2D,	
	BONE_FEELER_L3A,	
	BONE_FEELER_L3B,	
	BONE_FEELER_L3C,	
	BONE_FEELER_L3D,	
	BONE_FEELER_L1A,	
	BONE_FEELER_L1B,	
	BONE_FEELER_L1C,	
	BONE_FEELER_L1D,	
	BONE_FEELER_L2A,	
	BONE_FEELER_L2B,	
	BONE_FEELER_L2C,	
	BONE_FEELER_L2D,	

	255
};


const u8 CBotSwarmerBoss::m_aBoneEnableIndices_MainBody[] = {
	BONE_DUMMY,			
	BONE_TORSO,			
	BONE_LEG_LF_A,		
	BONE_LEG_LF_B,		
	BONE_LEG_LF_C,		
	BONE_LEG_LF_D,		
	BONE_LEG_LF_E,		
	BONE_LEG_LF_DUMMY,	
			
	BONE_LEG_RF_A,		
	BONE_LEG_RF_B,		
	BONE_LEG_RF_C,		
	BONE_LEG_RF_D,		
	BONE_LEG_RF_E,		
	BONE_LEG_RF_DUMMY,	

	BONE_DIODE_L,		
	BONE_DIODE_R,		
	BONE_DIODE_FRONT,	
//	BONE_ASS,			

	BONE_LEG_LR_A,		
	BONE_LEG_LR_B,		
	BONE_LEG_LR_C,		
	BONE_LEG_LR_D,		
	BONE_LEG_LR_E,		
	BONE_LEG_LR_DUMMY,	

	BONE_LEG_RR_A,		
	BONE_LEG_RR_B,		
	BONE_LEG_RR_C,		
	BONE_LEG_RR_D,		
	BONE_LEG_RR_E,		
	BONE_LEG_RR_DUMMY,	

	BONE_THORAX_A,		
	BONE_THORAX_B,		
	BONE_THORAX_C,		
	BONE_THORAX_D,		

	BONE_PRIFIRE,		
	BONE_HEAD,			

	255
};


const u8 CBotSwarmerBoss::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	BONE_DUMMY,			
	BONE_TORSO,			
	BONE_LEG_LF_A,		
	BONE_LEG_LF_B,		
	BONE_LEG_LF_C,		
	BONE_LEG_LF_D,		
	BONE_LEG_LF_E,		
	BONE_LEG_LF_DUMMY,	
			
	BONE_LEG_RF_A,		
	BONE_LEG_RF_B,		
	BONE_LEG_RF_C,		
	BONE_LEG_RF_D,		
	BONE_LEG_RF_E,		
	BONE_LEG_RF_DUMMY,	

	BONE_DIODE_L,		
	BONE_DIODE_R,		
	BONE_DIODE_FRONT,	
//	BONE_ASS,			

	BONE_LEG_LR_A,		
	BONE_LEG_LR_B,		
	BONE_LEG_LR_C,		
	BONE_LEG_LR_D,		
	BONE_LEG_LR_E,		
	BONE_LEG_LR_DUMMY,	

	BONE_LEG_RR_A,		
	BONE_LEG_RR_B,		
	BONE_LEG_RR_C,		
	BONE_LEG_RR_D,		
	BONE_LEG_RR_E,		
	BONE_LEG_RR_DUMMY,	

	BONE_THORAX_A,		
	BONE_THORAX_B,		
	BONE_THORAX_C,		
	BONE_THORAX_D,		

	BONE_PRIFIRE,		
	BONE_HEAD,			
	BONE_MANDIBLE_R,	
	BONE_MANDIBLE_L,	
		
	BONE_FEELER_R3A,	
	BONE_FEELER_R3B,	
	BONE_FEELER_R3C,	
	BONE_FEELER_R3D,	
	BONE_FEELER_R1A,	
	BONE_FEELER_R1B,	
	BONE_FEELER_R1C,	
	BONE_FEELER_R1D,	
	BONE_FEELER_R2A,	
	BONE_FEELER_R2B,	
	BONE_FEELER_R2C,	
	BONE_FEELER_R2D,	
	BONE_FEELER_L3A,	
	BONE_FEELER_L3B,	
	BONE_FEELER_L3C,	
	BONE_FEELER_L3D,	
	BONE_FEELER_L1A,	
	BONE_FEELER_L1B,	
	BONE_FEELER_L1C,	
	BONE_FEELER_L1D,	
	BONE_FEELER_L2A,	
	BONE_FEELER_L2B,	
	BONE_FEELER_L2C,	
	BONE_FEELER_L2D,	

	255
};


const u8 CBotSwarmerBoss::m_anTagPointBoneNameIndexArray[] = {
	BONE_TORSO,
	255
};

cchar *CBotSwarmerBoss::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMAidle001"
};



BOOL CBotSwarmerBoss::_BuildAnimStackDef( void ) {
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

const FGameDataMap_t CBotSwarmerBoss::m_aGameDataMap[] = {
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


const FGameData_TableEntry_t CBotSwarmerBoss::m_aBotInfoVocab_SwarmerBoss[] = {

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT, 0, 0, 0.0f, 0.0f
};

#endif //0