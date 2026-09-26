//////////////////////////////////////////////////////////////////////////////////////
// botkrunk_data.cpp - Bot data.
//
// Author: Michael Starich
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
// 1/27/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botkrunk.h"
#include "botanim.h"
#include "fmesh.h"


cchar		**CBotKrunk::m_apszIdleAnimNameTable;
const u8	**CBotKrunk::m_apnEnableBoneNameIndexTableForEachIdleTap;

//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotKrunk::m_BotDef = {
	BOTRACE_DROID,					// nRace
	BOTCLASS_KRUNK,					// nClass
	BOTSUBCLASS_KRUNK_NORMAL,		// nSubClass
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotKrunk::m_aGameDataMap[] = {
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
// Animation Stack Data:
//--------------------------------------------------------------------------------------------------------

cchar *CBotKrunk::m_apszBoneNameTable[BONE_COUNT] = {
	"Groin",					// BONE_GROIN

	"R_Leg_Upper",				// BONE_R_LEG_UPPER
	"R_Knee",					// BONE_R_KNEE	
	"R_Leg_Lower",				// BONE_R_LEG_LOWER	
	"R_Foot_Dummy",				// BONE_R_FOOT_DUMMY	

	"L_Leg_Upper",				// BONE_L_LEG_UPPER
	"L_Knee",					// BONE_L_KNEE
	"L_Leg_Lower",				// BONE_L_LEG_LOWER
	"L_Foot",					// BONE_L_FOOT
	"Beltpack_Front_A",			// BONE_BELTPACK_FRONT_A
	"Beltpack_Front_B",			// BONE_BELTPACK_FRONT_B
	"Beltpack_Front_C",			// BONE_BELTPACK_FRONT_C
	"Beltpack_Front_D",			// BONE_BELTPACK_FRONT_D
	"Beltpack_Front_E",			// BONE_BELTPACK_FRONT_E
	"Beltpack_Back_A",			// BONE_BELTPACK_BACK_A
	"Beltpack_Back_B",			// BONE_BELTPACK_BACK_B
	"Beltpack_Back_C",			// BONE_BELTPACK_BACK_C
	"Beltpack_Back_D",			// BONE_BELTPACK_BACK_D
										
	"Torso",					// BONE_TORSO
	"Head",						// BONE_HEAD
	"L_Eyebrow",				// BONE_L_EYEBROW
	"R_Eyebrow",				// BONE_R_EYEBROW
	"Jaw",						// BONE_JAW
										
	"L_Hand",					// BONE_L_HAND
	"L_Hand_Dummy",				// BONE_L_HAND_DUMMY
	"L_ArmB_Upper",				// BONE_L_ARMB_UPPER
	"L_ArmB_Lower",				// BONE_L_ARMB_LOWER
	"L_IndexFingerB",			// BONE_L_INDEXFINGERB
	"L_PinkyFingerB",			// BONE_L_PINKYFINGERB
	"L_MiddleFingerB",			// BONE_L_MIDDLEFINGERB
	"L_ThumbB",					// BONE_L_THUMBB
												
	"L_ArmA_Upper",				// BONE_L_ARMA_UPPER
	"L_ArmA_Lower",				// BONE_L_ARMA_LOWER
	"L_IndexFingerA",			// BONE_L_INDEXFINGERA
	"L_PinkyFingerA",			// BONE_L_PINKYFINGERA
	"L_MiddleFingerA",			// BONE_L_MIDDLEFINGERA
	"L_ThumbA",					// BONE_L_THUMBA
										
	"TorchDummy",				// BONE_TORCHDUMMY
	"R_ArmA_Upper",				// BONE_R_ARMA_UPPER
	"R_ArmA_Lower",				// BONE_R_ARMA_LOWER
	"AttachPoint_Primary",		// BONE_ATTACHPOINT_PRIMARY
	"Barrel",					// BONE_BARREL
	"Primary_Fire",				// BONE_PRIMARY_FIRE
	"Handle",					// BONE_HANDLE
	"R_ArmB_Upper",				// BONE_R_ARMB_UPPER
	"R_ArmB_Lower",				// BONE_R_ARMB_LOWER
	"R_ScrewDriver_Flat",		// BONE_R_SCREWDRIVER_FLAT
	"R_Screwdriver_Philips",	// BONE_R_SCREWDRIVER_PHILIPS
	"ScrewDriver_Dummy",		// BONE_SCREWDRIVER_DUMMY
	"KrunkDummy",				// BONE_DUMMY
};

cchar *CBotKrunk::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	// Ground locomotion:	
	"ARDKidle001",	// ANIM_STAND,
	"ARDKwalkF01",	// ANIM_WALK,
	"ARDKrun_F01",	// ANIM_RUN,

	// Air locomotion:
	"ARDKJump_01",	// ANIM_JUMP_LAUNCH,
	"ARDKJump_02",	// ANIM_JUMP_FLY,
	"ARDKJump_03",	// ANIM_JUMP_LAND,
	
	// Weapon firing and reloading:
	"ARDKactn101",	// ANIM_FIRE_1,	
	"ARDKactn201",	// ANIM_FIRE_2,	

	"ARDKrldA101",	// ANIM_RELOAD_CLIP_GRAB_NEW, 
	"ARDKrldA102",	// ANIM_RELOAD_CLIP_INSERT_NEW,
	"ARDKrldA103",	// ANIM_RELOAD_CLIP_SLAPIN_NEW,

	"ARDKwrench1",	// ANIM_DISASSEMBLE
	"ARDKassemb1",	// ANIM_ASSEMBLE
};

cchar *CBotKrunk::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",						// ANIMTAP_STAND,
	"walk",							// ANIMTAP_WALK,
	"run",							// ANIMTAP_RUN,

	"jump_launch",					// ANIMTAP_JUMP_LAUNCH,
	"jump_fly",						// ANIMTAP_JUMP_FLY,
	"jump_land_L",					// ANIMTAP_JUMP_LAND_LOWER,
	"jump_land_U",					// ANIMTAP_JUMP_LAND_UPPER,

	"fire1",						// ANIMTAP_FIRE_1,
	"fire2",						// ANIMTAP_FIRE_2,

	"aim_summer",					// ANIMTAP_AIM_SUMMER,

	"clip_rld_grab",				// ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	"clip_rld_insert",				// ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	"clip_rld_slapin",				// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,
	"disassemble",					// ANIMTAP_DISASSEMBLE,
	"assemble",						// ANIMTAP_ASSEMBLE,
};

cchar *CBotKrunk::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",						// ANIMCONTROL_STAND,
	"walk",							// ANIMCONTROL_WALK,
	"run",							// ANIMCONTROL_RUN,

	"jump_launch",					// ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",						// ANIMCONTROL_JUMP_FLY,
	"jump_land_L",					// ANIMCONTROL_JUMP_LAND_LOWER,
	"jump_land_U",					// ANIMCONTROL_JUMP_LAND_UPPER,

	"fire1",						// ANIMCONTROL_FIRE_1,
	"fire2",						// ANIMCONTROL_FIRE_2,

	"aim_summer",					// ANIMCONTROL_AIM_SUMMER,

	"clip_rld_grab",				// ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
	"clip_rld_insert",				// ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
	"clip_rld_slapin",				// ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

	"disassemble",					// ANIMCONTROL_DISASSEMBLE,
	"assemble",						// ANIMCONTROL_ASSEMBLE,
};

CFAnimCombinerConfig::ConfigStack_t CBotKrunk::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ASSEMBLE,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_DISASSEMBLE,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_1,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_2,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_FLY,

	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,
	
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotKrunk::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,								ANIMCONTROL_STAND,							1,
	ANIMTAP_WALK,								ANIMCONTROL_WALK,							1,
	ANIMTAP_RUN,								ANIMCONTROL_RUN,							1,

	ANIMTAP_JUMP_FLY,							ANIMCONTROL_JUMP_FLY,						1,
	ANIMTAP_JUMP_LAND_LOWER,					ANIMCONTROL_JUMP_LAND_LOWER,				1,
	ANIMTAP_JUMP_LAND_UPPER,					ANIMCONTROL_JUMP_LAND_UPPER,				1,
	ANIMTAP_JUMP_LAUNCH,						ANIMCONTROL_JUMP_LAUNCH,					1,

	ANIMTAP_AIM_SUMMER,							ANIMCONTROL_AIM_SUMMER,						1,

	ANIMTAP_FIRE_1,								ANIMCONTROL_FIRE_1,							1,
	ANIMTAP_FIRE_2,								ANIMCONTROL_FIRE_2,							1,

	ANIMTAP_RELOAD_CLIP_GRAB_NEW,				ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,			1,
	ANIMTAP_RELOAD_CLIP_INSERT_NEW,				ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,			1,
	ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,				ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,			1,

	ANIMTAP_DISASSEMBLE,						ANIMCONTROL_DISASSEMBLE,					1,
	ANIMTAP_ASSEMBLE,							ANIMCONTROL_ASSEMBLE,						1, 

	255,										255,										255
};

CFAnimCombiner::AttachList_t CBotKrunk::m_aAnimAttach[] = {
	ANIM_STAND,									ANIMTAP_STAND,
	ANIM_WALK,									ANIMTAP_WALK,
	ANIM_RUN,									ANIMTAP_RUN,

	ANIM_JUMP_LAUNCH,							ANIMTAP_JUMP_LAUNCH,
	ANIM_JUMP_LAND,								ANIMTAP_JUMP_LAND_LOWER,
	ANIM_JUMP_LAND,								ANIMTAP_JUMP_LAND_UPPER,
	ANIM_JUMP_FLY,								ANIMTAP_JUMP_FLY,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,				ANIMTAP_AIM_SUMMER,

	ANIM_FIRE_1,								ANIMTAP_FIRE_1,
	ANIM_FIRE_2,								ANIMTAP_FIRE_2,

	ANIM_RELOAD_CLIP_GRAB_NEW,					ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	ANIM_RELOAD_CLIP_INSERT_NEW,				ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	ANIM_RELOAD_CLIP_SLAPIN_NEW,				ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

	ANIM_DISASSEMBLE,							ANIMTAP_DISASSEMBLE,
	ANIM_ASSEMBLE,								ANIMTAP_ASSEMBLE,

	255,										255
};

const u8 CBotKrunk::m_aBoneEnableIndices_AimSummer[] = {
	BONE_HEAD,
	BONE_TORSO,

	255
};

const u8 CBotKrunk::m_aBoneEnableIndices_FullBody[] = {
	BONE_GROIN,
	BONE_R_LEG_UPPER,
	BONE_R_KNEE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT_DUMMY,
	BONE_L_LEG_UPPER,
	BONE_L_KNEE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_BELTPACK_FRONT_A,
	BONE_BELTPACK_FRONT_B,
	BONE_BELTPACK_FRONT_C,
	BONE_BELTPACK_FRONT_D,
	BONE_BELTPACK_FRONT_E,
	BONE_BELTPACK_BACK_A,
	BONE_BELTPACK_BACK_B,
	BONE_BELTPACK_BACK_C,
	BONE_BELTPACK_BACK_D,			
	BONE_TORSO,
	BONE_HEAD,
	BONE_L_EYEBROW,
	BONE_R_EYEBROW,
	BONE_JAW,			
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_ARMB_UPPER,
	BONE_L_ARMB_LOWER,
	BONE_L_INDEXFINGERB,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERB,
	BONE_L_THUMBB,					
	BONE_L_ARMA_UPPER,
	BONE_L_ARMA_LOWER,
	BONE_L_INDEXFINGERA,
	BONE_L_PINKYFINGERA,
	BONE_L_MIDDLEFINGERA,
	BONE_L_THUMBA,			
	BONE_TORCHDUMMY,
	BONE_R_ARMA_UPPER,
	BONE_R_ARMA_LOWER,
	BONE_ATTACHPOINT_PRIMARY,
	BONE_BARREL,
	BONE_PRIMARY_FIRE,
	BONE_HANDLE,
	BONE_R_ARMB_UPPER,
	BONE_R_ARMB_LOWER,
	BONE_R_SCREWDRIVER_FLAT,
	BONE_R_SCREWDRIVER_PHILIPS,
	BONE_SCREWDRIVER_DUMMY,
	BONE_DUMMY,
	255
};

const u8 CBotKrunk::m_aBoneEnableIndices_UpperBody[] = {
    BONE_BELTPACK_FRONT_A,
	BONE_BELTPACK_FRONT_B,
	BONE_BELTPACK_FRONT_C,
	BONE_BELTPACK_FRONT_D,
	BONE_BELTPACK_FRONT_E,
	BONE_BELTPACK_BACK_A,
	BONE_BELTPACK_BACK_B,
	BONE_BELTPACK_BACK_C,
	BONE_BELTPACK_BACK_D,			
	BONE_TORSO,
	BONE_HEAD,
	BONE_L_EYEBROW,
	BONE_R_EYEBROW,
	BONE_JAW,			
	BONE_L_HAND,
	BONE_L_HAND_DUMMY,
	BONE_L_ARMB_UPPER,
	BONE_L_ARMB_LOWER,
	BONE_L_INDEXFINGERB,
	BONE_L_PINKYFINGERB,
	BONE_L_MIDDLEFINGERB,
	BONE_L_THUMBB,					
	BONE_L_ARMA_UPPER,
	BONE_L_ARMA_LOWER,
	BONE_L_INDEXFINGERA,
	BONE_L_PINKYFINGERA,
	BONE_L_MIDDLEFINGERA,
	BONE_L_THUMBA,			
	BONE_TORCHDUMMY,
	BONE_R_ARMA_UPPER,
	BONE_R_ARMA_LOWER,
	BONE_ATTACHPOINT_PRIMARY,
	BONE_BARREL,
	BONE_PRIMARY_FIRE,
	BONE_HANDLE,
	BONE_R_ARMB_UPPER,
	BONE_R_ARMB_LOWER,
	BONE_R_SCREWDRIVER_FLAT,
	BONE_R_SCREWDRIVER_PHILIPS,
	BONE_SCREWDRIVER_DUMMY,
	255
};

const u8 CBotKrunk::m_aBoneEnableIndices_LowerBody[] = {
	BONE_R_LEG_UPPER,
	BONE_R_KNEE,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT_DUMMY,
	BONE_L_LEG_UPPER,
	BONE_L_KNEE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_GROIN,
	255
};

const u8 CBotKrunk::m_aBoneEnableIndices_UpperRightArm[] = {
	BONE_R_ARMA_UPPER,
	BONE_R_ARMA_LOWER,
	BONE_ATTACHPOINT_PRIMARY,
	BONE_BARREL,
	BONE_PRIMARY_FIRE,
	BONE_HANDLE,
	255
};

const u8 *CBotKrunk::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,		// ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,		// ANIMTAP_WALK,
	m_aBoneEnableIndices_FullBody,		// ANIMTAP_RUN,

	m_aBoneEnableIndices_FullBody,		// ANIMTAP_JUMP_LAUNCH,
	m_aBoneEnableIndices_FullBody,		// ANIMTAP_JUMP_FLY,
	m_aBoneEnableIndices_LowerBody,		// ANIMTAP_JUMP_LAND_LOWER,
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_JUMP_LAND_UPPER,
		
	m_aBoneEnableIndices_UpperRightArm,	// ANIMTAP_FIRE_1,
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_FIRE_2,

	m_aBoneEnableIndices_FullBody,		// ANIMTAP_AIM_SUMMER,	

	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

	m_aBoneEnableIndices_FullBody,		// ANIMTAP_DISASSEMBLE,
	m_aBoneEnableIndices_FullBody,		// ANIMTAP_ASSEMBLE
};

const u8 CBotKrunk::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:

	255
};

const u8 CBotKrunk::m_anTagPointBoneNameIndexArray[] = {
	BONE_HEAD,
	BONE_TORSO,
	BONE_L_ARMA_UPPER,
	BONE_R_ARMA_UPPER,

	255
};


const u8 CBotKrunk::m_nApproxEyePointBoneNameIndex = BONE_HEAD;

BOOL CBotKrunk::_BuildAnimStackDef( void ) {
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount = CBotAnimStackDef::ANIM_USER_COUNT;
	AnimStackDefInit.nBaseAnimNameCount = ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable = m_apszBaseAnimNameTable;
	AnimStackDefInit.nIdleCount = m_nBotInfo_Idles;
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