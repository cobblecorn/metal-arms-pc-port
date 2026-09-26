//////////////////////////////////////////////////////////////////////////////////////
// botzom_data.cpp -  Zombie bot data.
//
// Author: Steve Ranck     
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
// 01/27/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botzom.h"
#include "botanim.h"



//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotZom::m_BotDef = {
	BOTRACE_ZOMBIE,								// nRace
	BOTCLASS_ZOMBIE,							// nClass
	BOTSUBCLASS_ZOMBIE_NORMAL,					// nSubClass
};




//--------------------------------------------------------------------------------------------------------
// Game Data Vocab:
//--------------------------------------------------------------------------------------------------------

const FGameData_TableEntry_t CBotZom::m_aBotInfoVocab_Zom[] = {
	FGAMEDATA_VOCAB_TEXDEF,		// pSlashStreamerTexDef

	// fSlashStreamerAlpha:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fSlashStreamerWidth:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000000000,

	FGAMEDATA_VOCAB_DAMAGE,		// pDamageProfileSlash

	// pszClawDamageSphereBoneName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fClawDamageSphereRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100000000000,

	// nPartGroupCount
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100,

	// nDeathCount
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100,

	// fDeathStayAsleepSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fReformInterruptionStayAsleepSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	FGAMEDATA_VOCAB_PARTICLE,	// hBreathParticle

	// fBreathTimeMin:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// fBreathTimeMax:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hPartExplosion

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupSlash
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAssemble
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCrumbleApart
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupBlownApart
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupBreathPuff
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupHitDamageable
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupHitNonDamageable
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupClatter


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};




//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotZom::m_aGameDataMap[] = {
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

	"Zom",
	m_aBotInfoVocab_Zom,
	sizeof(m_BotInfo_Zom),
	(void *)&m_BotInfo_Zom,

	NULL
};




//--------------------------------------------------------------------------------------------------------
// Animation Stack Data:
//--------------------------------------------------------------------------------------------------------

cchar *CBotZom::m_apszBoneNameTable[BONE_COUNT] = {
	"Eye",							// BONE_EYE
	"Groin",						// BONE_GROIN
	"R_Leg_Upper",					// BONE_R_LEG_UPPER
	"R_Knee",						// BONE_R_KNEE
	"R_Leg_Middle",					// BONE_R_LEG_MIDDLE
	"R_Leg_Lower",					// BONE_R_LEG_LOWER
	"R_Foot",						// BONE_R_FOOT
	"off_R_Foot_Dummy",				// BONE_R_TOE
	"L_Leg_Upper",					// BONE_L_LEG_UPPER
	"L_Leg_Lower",					// BONE_L_LEG_LOWER
	"off_L_Foot_Dummy",				// BONE_L_TOE
	"Waist",						// BONE_WAIST
	"Waist_ScrapB",					// BONE_WAIST_SCRAPB
	"Waist_ScrapA",					// BONE_WAIST_SCRAPA
	"Waist_ScrapC",					// BONE_WAIST_SCRAPC
	"Torso",						// BONE_TORSO
	"Neck_A",						// BONE_NECK_A
	"Neck_B",						// BONE_NECK_B
	"Neck_C",						// BONE_NECK_C
	"Neck_D",						// BONE_NECK_D
	"Neck_E",						// BONE_NECK_E
	"Neck_F",						// BONE_NECK_F
	"EyeStalk",						// BONE_EYESTALK
	"Back_Junk_D",					// BONE_BACK_JUNK_D
	"Back_Junk_C",					// BONE_BACK_JUNK_C
	"Back_Junk_A",					// BONE_BACK_JUNK_A
	"Back_Junk_B",					// BONE_BACK_JUNK_B
	"R_Arm_Upper",					// BONE_R_ARM_UPPER
	"L_Arm_Upper",					// BONE_L_ARM_UPPER
	"L_Arm_Middle",					// BONE_L_ARM_MIDDLE
	"L_Arm_Lower",					// BONE_L_ARM_LOWER
	"L_Hand",						// BONE_L_HAND
	"Thumb_A",						// BONE_THUMB_A
	"Thumb_B",						// BONE_THUMB_B
	"L_Finger_1A",					// BONE_L_FINGER_1A
	"L_Finger_1B",					// BONE_L_FINGER_1B
	"L_Finger_1_Tip",				// BONE_L_FINGER_1_TIP
	"L_Finger_2A",					// BONE_L_FINGER_2A
	"L_Finger_2B",					// BONE_L_FINGER_2B
	"L_Finger_2_Tip",				// BONE_L_FINGER_2_TIP
	"Hose_A",						// BONE_HOSE_A
};

		

cchar *CBotZom::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	// Formation:
	"ARZAformA01",	// ANIM_FORM

	// Ground locomotion:	
	"ARZAidle001",	// ANIM_STAND,
	"ARZAwalkF01",	// ANIM_WALK,
	"ARZArun_F01",	// ANIM_RUN,

	// Air locomotion:
	"ARZAjump_01",	// ANIM_JUMP_LAUNCH,
	"ARZAjump_02",	// ANIM_JUMP_FLY,
	"ARZAjump_03",	// ANIM_JUMP_LAND,

	// Weapon firing and reloading:
	"ARZAactn201",	// ANIM_SLASH_F_FROM_REST,
	"ARZAactn202",	// ANIM_SLASH_F,
	"ARZAactn203",	// ANIM_SLASH_F_TO_REST,

	"ARZAactn204",	// ANIM_SLASH_B_FROM_REST,
	"ARZAactn205",	// ANIM_SLASH_B,
	"ARZAactn206",	// ANIM_SLASH_B_TO_REST,

	"ARZAactn207",	// ANIM_SLASH_D_FROM_REST,
	"ARZAactn208",	// ANIM_SLASH_D,
	"ARZAactn209",	// ANIM_SLASH_D_TO_REST,
};


cchar *CBotZom::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARZAidle002",
};


cchar *CBotZom::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"form",							// ANIMCONTROL_FORM,

	"stand",						// ANIMCONTROL_STAND,
	"walk",							// ANIMCONTROL_WALK,
	"run",							// ANIMCONTROL_RUN,

	"jump_launch",					// ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",						// ANIMCONTROL_JUMP_FLY,
	"jump_land_L",					// ANIMCONTROL_JUMP_LAND_LOWER,
	"jump_land_U",					// ANIMCONTROL_JUMP_LAND_UPPER,

	"slash_d",						// ANIMCONTROL_SLASH_D,
	"slash_fb",						// ANIMCONTROL_SLASH_FB,
	"slash_d_l",					// ANIMCONTROL_SLASH_D_LOWER,
	"slash_fb_l",					// ANIMCONTROL_SLASH_FB_LOWER,

	"aim_summer",					// ANIMCONTROL_AIM_SUMMER,
};


cchar *CBotZom::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"form",							// ANIMTAP_FORM,

	"stand",						// ANIMTAP_STAND,
	"walk",							// ANIMTAP_WALK,
	"run",							// ANIMTAP_RUN,

	"jump_launch",					// ANIMTAP_JUMP_LAUNCH,
	"jump_fly",						// ANIMTAP_JUMP_FLY,
	"jump_land_L",					// ANIMTAP_JUMP_LAND_LOWER,
	"jump_land_U",					// ANIMTAP_JUMP_LAND_UPPER,

	"slash_d",						// ANIMTAP_SLASH_D,
	"slash_fb",						// ANIMTAP_SLASH_FB,
	"slash_d_l",					// ANIMTAP_SLASH_D_LOWER,
	"slash_fb_l",					// ANIMTAP_SLASH_FB_LOWER,

	"aim_summer",					// ANIMTAP_AIM_SUMMER,
};


CFAnimCombinerConfig::ConfigStack_t CBotZom::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FORM,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SLASH_D,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SLASH_FB,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_FLY,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SLASH_D_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SLASH_FB_LOWER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,					255
};


// Order doesn't matter, just match left to right.
CFAnimCombinerConfig::ConfigTap_t CBotZom::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_FORM,								ANIMCONTROL_FORM,							1,

	ANIMTAP_STAND,								ANIMCONTROL_STAND,							1,
	ANIMTAP_WALK,								ANIMCONTROL_WALK,							1,
	ANIMTAP_RUN,								ANIMCONTROL_RUN,							1,

	ANIMTAP_JUMP_LAUNCH,						ANIMCONTROL_JUMP_LAUNCH,					1,
	ANIMTAP_JUMP_FLY,							ANIMCONTROL_JUMP_FLY,						1,
	ANIMTAP_JUMP_LAND_LOWER,					ANIMCONTROL_JUMP_LAND_LOWER,				1,
	ANIMTAP_JUMP_LAND_UPPER,					ANIMCONTROL_JUMP_LAND_UPPER,				1,

	ANIMTAP_AIM_SUMMER,							ANIMCONTROL_AIM_SUMMER,						1,

	ANIMTAP_SLASH_D,							ANIMCONTROL_SLASH_D,						1,
	ANIMTAP_SLASH_FB,							ANIMCONTROL_SLASH_FB,						1,
	ANIMTAP_SLASH_D_LOWER,						ANIMCONTROL_SLASH_D_LOWER,					1,
	ANIMTAP_SLASH_FB_LOWER,						ANIMCONTROL_SLASH_FB_LOWER,					1,

	255,										255,										255
};


CFAnimCombiner::AttachList_t CBotZom::m_aAnimAttach[] = {
	ANIM_FORM,									ANIMTAP_FORM,

	ANIM_STAND,									ANIMTAP_STAND,
	ANIM_WALK,									ANIMTAP_WALK,
	ANIM_RUN,									ANIMTAP_RUN,

	ANIM_JUMP_LAUNCH,							ANIMTAP_JUMP_LAUNCH,
	ANIM_JUMP_FLY,								ANIMTAP_JUMP_FLY,
	ANIM_JUMP_LAND,								ANIMTAP_JUMP_LAND_LOWER,
	ANIM_JUMP_LAND,								ANIMTAP_JUMP_LAND_UPPER,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,				ANIMTAP_AIM_SUMMER,

	ANIM_SLASH_D_FROM_REST,						ANIMTAP_SLASH_D,
	ANIM_SLASH_F_FROM_REST,						ANIMTAP_SLASH_FB,
	ANIM_SLASH_D_FROM_REST,						ANIMTAP_SLASH_D_LOWER,
	ANIM_SLASH_F_FROM_REST,						ANIMTAP_SLASH_FB_LOWER,

	255,										255
};


const u8 CBotZom::m_aBoneEnableIndices_AimSummer[] = {
	BONE_NECK_F,
	BONE_TORSO,

	255
};


const u8 CBotZom::m_aBoneEnableIndices_FullBody[] = {
	BONE_EYE,
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_KNEE,
	BONE_R_LEG_MIDDLE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_TOE,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_LOWER,
	BONE_L_TOE,
	BONE_WAIST,
	BONE_WAIST_SCRAPB,
	BONE_WAIST_SCRAPA,
	BONE_WAIST_SCRAPC,
	BONE_TORSO,
	BONE_NECK_A,
	BONE_NECK_B,
	BONE_NECK_C,
	BONE_NECK_D,
	BONE_NECK_E,
	BONE_NECK_F,
	BONE_EYESTALK,
	BONE_BACK_JUNK_D,
	BONE_BACK_JUNK_C,
	BONE_BACK_JUNK_A,
	BONE_BACK_JUNK_B,
	BONE_R_ARM_UPPER,
	BONE_L_ARM_UPPER,
	BONE_L_ARM_MIDDLE,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_THUMB_A,
	BONE_THUMB_B,
	BONE_L_FINGER_1A,
	BONE_L_FINGER_1B,
	BONE_L_FINGER_1_TIP,
	BONE_L_FINGER_2A,
	BONE_L_FINGER_2B,
	BONE_L_FINGER_2_TIP,
	BONE_HOSE_A,

	255
};


const u8 CBotZom::m_aBoneEnableIndices_UpperBody[] = {
	BONE_EYE,
	BONE_WAIST,
	BONE_WAIST_SCRAPB,
	BONE_WAIST_SCRAPA,
	BONE_WAIST_SCRAPC,
	BONE_TORSO,
	BONE_NECK_A,
	BONE_NECK_B,
	BONE_NECK_C,
	BONE_NECK_D,
	BONE_NECK_E,
	BONE_NECK_F,
	BONE_EYESTALK,
	BONE_BACK_JUNK_D,
	BONE_BACK_JUNK_C,
	BONE_BACK_JUNK_A,
	BONE_BACK_JUNK_B,
	BONE_R_ARM_UPPER,
	BONE_L_ARM_UPPER,
	BONE_L_ARM_MIDDLE,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_THUMB_A,
	BONE_THUMB_B,
	BONE_L_FINGER_1A,
	BONE_L_FINGER_1B,
	BONE_L_FINGER_1_TIP,
	BONE_L_FINGER_2A,
	BONE_L_FINGER_2B,
	BONE_L_FINGER_2_TIP,
	BONE_HOSE_A,

	255
};


const u8 CBotZom::m_aBoneEnableIndices_LowerBody[] = {
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_KNEE,
	BONE_R_LEG_MIDDLE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_TOE,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_LOWER,
	BONE_L_TOE,

	255
};


const u8 CBotZom::m_aBoneEnableIndices_JumpLand_Lower[] = {
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_KNEE,
	BONE_R_LEG_MIDDLE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_R_TOE,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_LOWER,
	BONE_L_TOE,

	255
};


const u8 CBotZom::m_aBoneEnableIndices_JumpLand_Upper[] = {
	BONE_EYE,
	BONE_WAIST,
	BONE_WAIST_SCRAPB,
	BONE_WAIST_SCRAPA,
	BONE_WAIST_SCRAPC,
	BONE_TORSO,
	BONE_NECK_A,
	BONE_NECK_B,
	BONE_NECK_C,
	BONE_NECK_D,
	BONE_NECK_E,
	BONE_NECK_F,
	BONE_EYESTALK,
	BONE_BACK_JUNK_D,
	BONE_BACK_JUNK_C,
	BONE_BACK_JUNK_A,
	BONE_BACK_JUNK_B,
	BONE_R_ARM_UPPER,
	BONE_L_ARM_UPPER,
	BONE_L_ARM_MIDDLE,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_THUMB_A,
	BONE_THUMB_B,
	BONE_L_FINGER_1A,
	BONE_L_FINGER_1B,
	BONE_L_FINGER_1_TIP,
	BONE_L_FINGER_2A,
	BONE_L_FINGER_2B,
	BONE_L_FINGER_2_TIP,
	BONE_HOSE_A,

	255
};


const u8 *CBotZom::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_FORM,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_WALK,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_RUN,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_LAUNCH,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_FLY,
	m_aBoneEnableIndices_JumpLand_Lower,	// ANIMTAP_JUMP_LAND_LOWER,
	m_aBoneEnableIndices_JumpLand_Upper,	// ANIMTAP_JUMP_LAND_UPPER,

	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_SLASH_D,
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_SLASH_FB,
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_SLASH_D_LOWER,
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_SLASH_FB_LOWER,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_AIM_SUMMER,
};


const u8 *CBotZom::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody,			// Idle 0
};


const u8 CBotZom::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:

	255
};


const u8 CBotZom::m_anTagPointBoneNameIndexArray[] = {
	BONE_EYESTALK,
	BONE_BACK_JUNK_A,
	BONE_WAIST,
	BONE_L_ARM_LOWER,
	BONE_R_ARM_UPPER,

	255
};


const u8 CBotZom::m_nApproxEyePointBoneNameIndex = BONE_EYESTALK;


BOOL CBotZom::_BuildAnimStackDef( void ) {
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


