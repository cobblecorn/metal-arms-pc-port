//////////////////////////////////////////////////////////////////////////////////////
// botscientist_data.cpp - mil scientist bot data
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
// 04/19/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botscientist.h"
#include "botanim.h"
#include "fmesh.h"

const CBotDef CBotScientist::m_BotDef = {
	BOTRACE_MIL,						// nRace
	BOTCLASS_SCIENTIST,					// nClass
	BOTSUBCLASS_SCIENTIST_GENERIC,		// nSubClass
};


cchar *CBotScientist::m_apszBoneNameTable[BONE_COUNT] = {
	"Base",					// BONE_BASE,			
	"Waist",				// BONE_WAIST,			
	"Torso",				// BONE_TORSO,			
		   
	"L_Arm_Upper",			// BONE_ARM_UPPER_L,	
	"L_Elbow",				// BONE_ARM_ELBOW_L,	
	"L_Arm_Lower",			// BONE_ARM_LOWER_L,	
	"L_Hand",				// BONE_HAND_L,		
	"L_Finger_A1",			// BONE_FINGER_LA1,		
	"L_Finger_A2",			// BONE_FINGER_LA2,		
	"L_Finger_A3",			// BONE_FINGER_LA3,		
	"L_Finger_B1",			// BONE_FINGER_LB1,		
	"L_Finger_B2",			// BONE_FINGER_LB2,		
	"L_Finger_B3",			// BONE_FINGER_LB3,		
		   
	"R_Arm_Upper",			// BONE_ARM_UPPER_R,	
	"R_Elbow",				// BONE_ARM_ELBOW_R,	
	"R_Arm_Lower",			// BONE_ARM_LOWER_R,	
	"R_Hand",				// BONE_HAND_R,		
	"R_Finger_B1",			// BONE_FINGER_RA1,		
	"R_Finger_B2",			// BONE_FINGER_RA2,		
	"R_Finger_B3",			// BONE_FINGER_RA3,		
	"R_Finger_A1",			// BONE_FINGER_RB1,		
	"R_Finger_A2",			// BONE_FINGER_RB2,		
	"R_Finger_A3",			// BONE_FINGER_RB3,

	// non anim driven
	"Target",				// BONE_TARGET,
};

cchar *CBotScientist::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
    "ARMIidle001",	// ANIM_STAND,
	"ARMIconsol1",	// ANIM_CONSOLE,
	"ARMIactn101",	// ANIM_ATTACK,
	"ARMIelect01",	// ANIM_RC_TETHERED
	"ARMIpwrdn01",	// ANIM_RC_POWER_DOWN
	"ARMIpwrup01",	// ANIM_RC_POWER_UP
};


cchar *CBotScientist::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",			// ANIMCONTROL_STAND,
	"term",				// ANIMCONTROL_CONSOLE,
	"attack",			// ANIMCONTROL_ATTACK,
	"elec",				// ANIMCONTROL_RC_TETHERED
	"pwrdn",			// ANIMCONTROL_RC_POWER_DOWN
	"pwrup",			// ANIMCONTROL_RC_POWER_UP
};	


cchar *CBotScientist::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",			// ANIMTAP_STAND,
	"term",				// ANIMTAP_CONSOLE,
	"attack",			// ANIMTAP_ATTACK,
	"elec",				// ANIMTAP_RC_TETHERED
	"pwrdn",			// ANIMTAP_RC_POWER_DOWN
	"pwrup",			// ANIMTAP_RC_POWER_UP

};


cchar *CBotScientist::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMIidle001"
};


CFAnimCombinerConfig::ConfigStack_t CBotScientist::m_aAnimCombinerConfigStack[] = {
	// from highest to lowest priority

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_TETHERED,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_RC_POWER_DOWN,

	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_ATTACK,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_CONSOLE,
	CFAnimMixer::TYPE_BLENDER,		ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,		255
};


CFAnimCombinerConfig::ConfigTap_t	CBotScientist::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,				ANIMCONTROL_STAND,					1,
	ANIMTAP_CONSOLE,			ANIMCONTROL_CONSOLE,				1,
	ANIMTAP_ATTACK,				ANIMCONTROL_ATTACK,					1,
	ANIMTAP_RC_TETHERED,		ANIMCONTROL_RC_TETHERED,			1,
	ANIMTAP_RC_POWER_UP,		ANIMCONTROL_RC_POWER_UP,			1,
	ANIMTAP_RC_POWER_DOWN,		ANIMCONTROL_RC_POWER_DOWN,			1,
	255,						255,								255
};


CFAnimCombiner::AttachList_t	CBotScientist::m_aAnimAttach[] = {
	ANIM_STAND,						ANIMTAP_STAND,			
	ANIM_CONSOLE,					ANIMTAP_CONSOLE,	
	ANIM_ATTACK,					ANIMTAP_ATTACK,
	ANIM_RC_TETHERED,				ANIMTAP_RC_TETHERED,		
	ANIM_RC_POWER_UP,				ANIMTAP_RC_POWER_UP,		
	ANIM_RC_POWER_DOWN,				ANIMTAP_RC_POWER_DOWN,		

	255,							255
};


const u8 CBotScientist::m_aBoneEnableIndices_FullBody[] = {
	BONE_BASE,		
	BONE_WAIST,		
	BONE_TORSO,		

	BONE_ARM_UPPER_L,
	BONE_ARM_ELBOW_L,
	BONE_ARM_LOWER_L,
	BONE_HAND_L,		
	BONE_FINGER_LA1,	
	BONE_FINGER_LA2,	
	BONE_FINGER_LA3,	
	BONE_FINGER_LB1,	
	BONE_FINGER_LB2,	
	BONE_FINGER_LB3,	

	BONE_ARM_UPPER_R,
	BONE_ARM_ELBOW_R,
	BONE_ARM_LOWER_R,
	BONE_HAND_R,		
	BONE_FINGER_RA1,	
	BONE_FINGER_RA2,	
	BONE_FINGER_RA3,	
	BONE_FINGER_RB1,	
	BONE_FINGER_RB2,	
	BONE_FINGER_RB3,	
	255
};


const u8 CBotScientist::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	BONE_BASE,		
	BONE_WAIST,		
	BONE_TORSO,		

	BONE_ARM_UPPER_L,
	BONE_ARM_ELBOW_L,
	BONE_ARM_LOWER_L,
	BONE_HAND_L,		
	BONE_FINGER_LA1,	
	BONE_FINGER_LA2,	
	BONE_FINGER_LA3,	
	BONE_FINGER_LB1,	
	BONE_FINGER_LB2,	
	BONE_FINGER_LB3,	

	BONE_ARM_UPPER_R,
	BONE_ARM_ELBOW_R,
	BONE_ARM_LOWER_R,
	BONE_HAND_R,		
	BONE_FINGER_RA1,	
	BONE_FINGER_RA2,	
	BONE_FINGER_RA3,	
	BONE_FINGER_RB1,	
	BONE_FINGER_RB2,	
	BONE_FINGER_RB3,	

	255
};


const u8* CBotScientist::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_STAND,			
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_CONSOLE,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_ATTACK,

	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_TETHERED,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_UP,	
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_RC_POWER_DOWN,
	
	

};


const u8* CBotScientist::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// Idle 0
};


//BONE MASKS FOR USER ANIMS
//const u8 CBotScientist::m_aBoneEnableIndices_UserAnim_UpperBody[] = {
//	BONE_GROIN,			
//	BONE_TORSO,			
//	BONE_HEAD,			
//	BONE_CHEST_TOP,		
//	BONE_CHEST_BOTTOM,	
//	BONE_CHEST_MIDDLE,	
//	BONE_ARM_UPPER_L,	
//	BONE_ARM_LOWER_L,	
//	BONE_GUN_2_L,		
//	BONE_GUN_1_L,		
//	BONE_ARM_ARMOR_L,	
//
//	BONE_ARM_UPPER_R,	
//	BONE_ARM_LOWER_R,	
//	BONE_GUN_2_R,		
//	BONE_GUN_1_R,		
//	BONE_ARM_ARMOR_R,	
//
//	BONE_WING_L,		
//	BONE_WING_R,		
//
//	255
//};
//
//
//const u8 CBotScientist::m_aBoneEnableIndices_UserAnim_LowerBody[] = {
//	BONE_GROIN,			
//
//	BONE_LEG_UPPER_L,	
//	BONE_LEG_KNEE_L,	
//	BONE_LEG_LOWER_L,	
//	BONE_LEG_ANGKLE_L,	
//	BONE_FOOT_BACK_L,	
//	BONE_FOOT_BLADE_L,	
//	BONE_FOOT_FRONT_L,	
//	BONE_TOE1_L,		
//	BONE_TOE2_L,		
//						
//
//	BONE_LEG_UPPER_R,	
//	BONE_LEG_KNEE_R,	
//	BONE_LEG_LOWER_R,	
//	BONE_LEG_ANGKLE_R,	
//	BONE_FOOT_BACK_R,	
//	BONE_FOOT_BLADE_R,	
//	BONE_FOOT_FRONT_R,	
//	BONE_TOE1_R,		
//	BONE_TOE2_R,		
//	255
//};
//
//
//const u8 CBotScientist::m_aBoneEnableIndices_UserAnim_UpperTorso[] = {
//	BONE_TORSO,			
//	BONE_CHEST_TOP,		
//	BONE_CHEST_BOTTOM,	
//	BONE_CHEST_MIDDLE,	
//
//	BONE_WING_L,		
//	BONE_WING_R,		
//	255
//};
//
//
//const u8 CBotScientist::m_aBoneEnableIndices_UserAnim_LowerTorso[] = {
//	BONE_GROIN,			
//	BONE_LEG_UPPER_L,	
//	BONE_LEG_UPPER_R,	
//	255
//};
//
//
//const u8 CBotScientist::m_aBoneEnableIndices_UserAnim_LeftArm[] = {
//	BONE_ARM_UPPER_L,	
//	BONE_ARM_LOWER_L,	
//	BONE_GUN_2_L,		
//	BONE_GUN_1_L,		
//	BONE_ARM_ARMOR_L,	
//	255
//};
//
//
//const u8 CBotScientist::m_aBoneEnableIndices_UserAnim_RightArm[] = {
//	BONE_ARM_UPPER_R,	
//	BONE_ARM_LOWER_R,	
//	BONE_GUN_2_R,		
//	BONE_GUN_1_R,		
//	BONE_ARM_ARMOR_R,	
//	255
//};
//
//
//const u8 CBotScientist::m_aBoneEnableIndices_UserAnim_Head[] = {
//	BONE_HEAD,
//	255
//};

const u8 CBotScientist::m_nApproxEyePointBoneNameIndex = BONE_TARGET;


const u8 CBotScientist::m_anTagPointBoneNameIndexArray[] = {
	BONE_TARGET,
	BONE_ARM_UPPER_L,
	BONE_ARM_UPPER_R,

	255
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameData_TableEntry_t CBotScientist::m_aBotInfoVocab_Scientist[] = {
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_0),		// fAttackTime;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_0),		// fAttackRechargeTime;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_Pt1),	// fAttackMaxRange;
	FGAMEDATA_VOCAB_F32_BOUNDMIN(F32_DATATABLE_Pt1),	// fHealMaxRange;

	FGAMEDATA_VOCAB_TEXDEF,								// pBoltTex;

	FGAMEDATA_VOCAB_DAMAGE,								// pJoltDamageProfile;
	FGAMEDATA_VOCAB_DAMAGE,								// pHealDamageProfile;

	FGAMEDATA_VOCAB_PARTICLE,							// hJoltParticleDef;
	FGAMEDATA_VOCAB_PARTICLE,							// hHealParticleDef;

	FGAMEDATA_VOCAB_SOUND_GROUP,						// pSoundShock;
	FGAMEDATA_VOCAB_SOUND_GROUP,						// pSoundHeal;
	FGAMEDATA_VOCAB_SOUND_GROUP,						// pSoundHoverLoop;
	
	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};



const FGameDataMap_t CBotScientist::m_aGameDataMap[] = {
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

	"Scientist",
	m_aBotInfoVocab_Scientist,
	sizeof(m_BotInfo_Scientist),
	(void*)&m_BotInfo_Scientist,

	NULL
};


BOOL CBotScientist::_BuildAnimStackDef( void ) {
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

