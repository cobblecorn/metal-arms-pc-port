//////////////////////////////////////////////////////////////////////////////////////
// botslosh_data.cpp - Bot data.
//
// Author: Michael Scholz
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
// 12/18/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botslosh.h"
#include "botanim.h"
#include "fmesh.h"



//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotSlosh::m_BotDef =
{
	BOTRACE_DROID,							// nRace
	BOTCLASS_CHEMBOT,							// nClass
	BOTSUBCLASS_CHEMBOT_GENERIC,				// nSubClass
};




//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotSlosh::m_aGameDataMap[] =
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

cchar *CBotSlosh::m_apszBoneNameTable[BONE_COUNT] =
{
	"R_Leg_Upper",					// BONE_R_LEG_UPPER	
	"R_Leg_Lower",					// BONE_R_LEG_LOWER	
	"R_Foot_Back",					// BONE_R_FOOT_BACK	
	"R_Heel",						// BONE_R_HEEL		
	"R_Foot_Front",					// BONE_R_FOOT_FRONT	
	"R_Toe",						// BONE_R_TOE		
			
	"L_Leg_Upper",					// BONE_L_LEG_UPPER	
	"L_Leg_Lower",					// BONE_L_LEG_LOWER	
	"L_Foot_Back",					// BONE_L_FOOT_BACK	
	"L_Heel",						// BONE_L_HEEL		
	"L_Foot_Front",					// BONE_L_FOOT_FRONT	
	"L_Toe",						// BONE_L_TOE		
			
	"R_Arm_Upper",					// BONE_R_ARM_UPPER	
	"R_Elbow",						// BONE_R_ELBOW		
	"R_Arm_Lower",					// BONE_R_ARM_LOWER	
			
	"L_Arm_Upper",					// BONE_L_ARM_UPPER	
	"L_Elbow",						// BONE_L_ELBOW		
	"L_Arm_Lower",					// BONE_L_ARM_LOWER	
	"L_Hand",						// BONE_L_HAND		
	"L_Middlefingera",				// BONE_L_MIDDLEFINGERA	
	"L_Middlefingerb",				// BONE_L_MIDDLEFINGERB	
	"L_Pinkyfingera",				// BONE_L_PINKYFINGERA	
	"L_Pinkyfingerb",				// BONE_L_PINKYFINGERB	
	"L_Indexfingera",				// BONE_L_INDEXFINGERA	
	"L_Indexfingerb",				// BONE_L_INDEXFINGERB	
	"L_Thumba",						// BONE_L_THUMBA		
	"L_Thumbb",						// BONE_L_THUMBB
	"Secondary_Fire",				// BONE_SECONDARY_FIRE

	"Groin",						// BONE_GROIN
	"Torso",						// BONE_TORSO
	"Carapace_Rear",				// BONE_CARAPACE_REAR
	"Tank",							// BONE_TANK
	"Carapace_Middle",				// BONE_CARAPACE_MIDDLE
	"Carapace_Front",				// BONE_CARAPACE_FRONT
	"Neck",							// BONE_NECK
	"Head",							// BONE_HEAD
	"Mask",							// BONE_MASK
	"AttachPoint_Jet_R",			// BONE_ATTACHPOINT_JET_R
	"AttachPoint_Jet_L",			// BONE_ATTACHPOINT_JET_L
	"AttachPoint_Fluid",			// BONE_ATTACHPOINT_FLUID
};

		

cchar *CBotSlosh::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] =
{
	// Ground locomotion:	
	"ARDSidle001",	// ANIM_STAND
	"ARDSwalkF01",	// ANIM_SNEAK,
	"ARDSwalkF01",	// ANIM_WALK,
	"ARDSrun_F01",	// ANIM_RUN,

	// Air locomotion:
	"ARDSJump_01",	// ANIM_JUMP_LAUNCH,
	"ARDSJump_02",	// ANIM_JUMP_FLY,
	"ARDSJump201",	// ANIM_JUMP_BOOST,
	"ARDSJump_03",	// ANIM_JUMP_LAND,
	
	// Weapon firing and reloading:
	"ARDSactn201",	// ANIM_FIRE_2,
	"ARDSrldP_01",	// ANIM_RELOAD_CLIP_EJECT_OLD,
	"ARDSrldP_02",	// ANIM_RELOAD_CLIP_GRAB_NEW,
	"ARDSrldP_03",	// ANIM_RELOAD_CLIP_INSERT_NEW,
	"ARDSrldP_04"	// ANIM_RELOAD_CLIP_SLAPIN_NEW,
};


cchar		**CBotSlosh::m_apszIdleAnimNameTable;
const u8	**CBotSlosh::m_apnEnableBoneNameIndexTableForEachIdleTap;

cchar *CBotSlosh::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] =
{
	"stand",						// ANIMCONTROL_STAND,
	"sneak",						// ANIMCONTROL_SNEAK,
	"walk",							// ANIMCONTROL_WALK,
	"run",							// ANIMCONTROL_RUN,

	"jump_launch",					// ANIMCONTROL_JUMP_LAUNCH,
	"jump_fly",						// ANIMCONTROL_JUMP_FLY,
	"jump_boost",					// ANIMCONTROL_JUMP_BOOST,
	"jump_land_upr",				// ANIMCONTROL_JUMP_LAND_UPPER,
	"jump_land_lwr",				// ANIMCONTROL_JUMP_LAND_LOWER,

	"fire2",						// ANIMCONTROL_FIRE_2,

	"aim_summer",					// ANIMCONTROL_AIM_SUMMER,

	"clip_rld_eject",				// ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
	"clip_rld_grab",				// ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
	"clip_rld_insert",				// ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
	"clip_rld_slapin",				// ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,
};

cchar *CBotSlosh::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] =
{
	"stand",						// ANIMTAP_STAND,
	"sneak",						// ANIMTAP_SNEAK,
	"walk",							// ANIMTAP_WALK,
	"run",							// ANIMTAP_RUN,

	"jump_launch",					// ANIMTAP_JUMP_LAUNCH,
	"jump_fly",						// ANIMTAP_JUMP_FLY,
	"jump_boost",					// ANIMTAP_JUMP_BOOST,
	"jump_land_upr",				// ANIMTAP_JUMP_LAND_UPPER,
	"jump_land_lwr",				// ANIMTAP_JUMP_LAND_LOWER,

	"fire2",						// ANIMTAP_FIRE_2,

	"aim_summer",					// ANIMTAP_AIM_SUMMER,

	"clip_rld_eject",				// ANIMTAP_RELOAD_CLIP_EJECT_OLD,
	"clip_rld_grab",				// ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	"clip_rld_insert",				// ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	"clip_rld_slapin",				// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,
};


CFAnimCombinerConfig::ConfigStack_t CBotSlosh::m_aAnimCombinerConfigStack[] =
{
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FIRE_2,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAUNCH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_LAND_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_BOOST,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP_FLY,

	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RUN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SNEAK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,
	
	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotSlosh::m_aAnimCombinerConfigTaps[] =
{
	ANIMTAP_STAND,								ANIMCONTROL_STAND,							1,
	ANIMTAP_SNEAK,								ANIMCONTROL_SNEAK,							1,
	ANIMTAP_WALK,								ANIMCONTROL_WALK,							1,
	ANIMTAP_RUN,								ANIMCONTROL_RUN,							1,

	ANIMTAP_JUMP_FLY,							ANIMCONTROL_JUMP_FLY,						1,
	ANIMTAP_JUMP_LAND_UPPER,					ANIMCONTROL_JUMP_LAND_UPPER,				1,
	ANIMTAP_JUMP_LAND_LOWER,					ANIMCONTROL_JUMP_LAND_LOWER,				1,
	ANIMTAP_JUMP_BOOST,							ANIMCONTROL_JUMP_BOOST,						1,
	ANIMTAP_JUMP_LAUNCH,						ANIMCONTROL_JUMP_LAUNCH,					1,

	ANIMTAP_AIM_SUMMER,							ANIMCONTROL_AIM_SUMMER,						1,

	ANIMTAP_FIRE_2,								ANIMCONTROL_FIRE_2,							1,

	ANIMTAP_RELOAD_CLIP_EJECT_OLD,				ANIMCONTROL_RELOAD_CLIP_EJECT_OLD,			1,
	ANIMTAP_RELOAD_CLIP_GRAB_NEW,				ANIMCONTROL_RELOAD_CLIP_GRAB_NEW,			1,
	ANIMTAP_RELOAD_CLIP_INSERT_NEW,				ANIMCONTROL_RELOAD_CLIP_INSERT_NEW,			1,
	ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,				ANIMCONTROL_RELOAD_CLIP_SLAPIN_NEW,			1,

	255,										255,										255
};


CFAnimCombiner::AttachList_t CBotSlosh::m_aAnimAttach[] =
{
	ANIM_STAND,									ANIMTAP_STAND,
	ANIM_SNEAK,									ANIMTAP_SNEAK,
	ANIM_WALK,									ANIMTAP_WALK,
	ANIM_RUN,									ANIMTAP_RUN,

	ANIM_JUMP_LAUNCH,							ANIMTAP_JUMP_LAUNCH,
	ANIM_JUMP_LAND,								ANIMTAP_JUMP_LAND_UPPER,
	ANIM_JUMP_LAND,								ANIMTAP_JUMP_LAND_LOWER,
	ANIM_JUMP_BOOST,							ANIMTAP_JUMP_BOOST,
	ANIM_JUMP_FLY,								ANIMTAP_JUMP_FLY,

	BOTANIM_NULL_ANIMSOURCE_ATTACH,				ANIMTAP_AIM_SUMMER,

	ANIM_FIRE_2,								ANIMTAP_FIRE_2,

	ANIM_RELOAD_CLIP_EJECT_OLD,					ANIMTAP_RELOAD_CLIP_EJECT_OLD,
	ANIM_RELOAD_CLIP_GRAB_NEW,					ANIMTAP_RELOAD_CLIP_GRAB_NEW,
	ANIM_RELOAD_CLIP_INSERT_NEW,				ANIMTAP_RELOAD_CLIP_INSERT_NEW,
	ANIM_RELOAD_CLIP_SLAPIN_NEW,				ANIMTAP_RELOAD_CLIP_SLAPIN_NEW,

	255,										255
};

const u8 CBotSlosh::m_aBoneEnableIndices_AimSummer[] = 
{
	BONE_HEAD,
	BONE_TORSO,

	255
};

const u8 CBotSlosh::m_aBoneEnableIndices_FullBody[] = 
{
	BONE_R_LEG_UPPER,	
	BONE_R_LEG_LOWER,	
	BONE_R_FOOT_BACK,	
	BONE_R_HEEL		,
	BONE_R_FOOT_FRONT,	
	BONE_R_TOE		,
	BONE_L_LEG_UPPER,	
	BONE_L_LEG_LOWER,	
	BONE_L_FOOT_BACK,	
	BONE_L_HEEL		,
	BONE_L_FOOT_FRONT,	
	BONE_L_TOE		,

	BONE_R_ARM_UPPER,	
	BONE_R_ELBOW	,	
	BONE_R_ARM_LOWER,	
	BONE_L_ARM_UPPER,	
	BONE_L_ELBOW	,	
	BONE_L_ARM_LOWER,	
	BONE_L_HAND		,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_PINKYFINGERA,	
	BONE_L_PINKYFINGERB,	
	BONE_L_INDEXFINGERA,	
	BONE_L_INDEXFINGERB,	
	BONE_L_THUMBA,	
	BONE_L_THUMBB,
	BONE_SECONDARY_FIRE,
	BONE_GROIN,
	
	BONE_TORSO,
	BONE_CARAPACE_REAR,
	BONE_TANK,
	BONE_CARAPACE_MIDDLE,
	BONE_CARAPACE_FRONT,
	BONE_NECK,
	BONE_HEAD,
	BONE_MASK,
	BONE_ATTACHPOINT_JET_R,
	BONE_ATTACHPOINT_JET_L,
	BONE_ATTACHPOINT_FLUID,
	255
};



const u8 CBotSlosh::m_aBoneEnableIndices_UpperBody[] = 
{
	BONE_R_ARM_UPPER,	
	BONE_R_ELBOW,	
	BONE_R_ARM_LOWER,	
	BONE_L_ARM_UPPER,	
	BONE_L_ELBOW,	
	BONE_L_ARM_LOWER,	
	BONE_L_HAND,
	BONE_L_MIDDLEFINGERA,
	BONE_L_MIDDLEFINGERB,
	BONE_L_PINKYFINGERA,	
	BONE_L_PINKYFINGERB,	
	BONE_L_INDEXFINGERA,	
	BONE_L_INDEXFINGERB,	
	BONE_L_THUMBA,	
	BONE_L_THUMBB,
	BONE_SECONDARY_FIRE,
	BONE_TORSO,
	BONE_CARAPACE_REAR,
	BONE_TANK,
	BONE_CARAPACE_MIDDLE,
	BONE_CARAPACE_FRONT,
	BONE_NECK,
	BONE_HEAD,
	BONE_MASK,
	BONE_ATTACHPOINT_JET_R,
	BONE_ATTACHPOINT_JET_L,
	BONE_ATTACHPOINT_FLUID,

	
	255
};

const u8 CBotSlosh::m_aBoneEnableIndices_LowerBody[] = 
{
	BONE_GROIN,
	BONE_R_LEG_UPPER,	
	BONE_R_LEG_LOWER,	
	BONE_R_FOOT_BACK,	
	BONE_R_HEEL		,
	BONE_R_FOOT_FRONT,	
	BONE_R_TOE		,
	BONE_L_LEG_UPPER,	
	BONE_L_LEG_LOWER,	
	BONE_L_FOOT_BACK,	
	BONE_L_HEEL		,
	BONE_L_FOOT_FRONT,	
	BONE_L_TOE		,
	255
};

const u8 *CBotSlosh::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_SNEAK,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_WALK,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_RUN,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_LAUNCH,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_JUMP_FLY,
	m_aBoneEnableIndices_FullBody,			// ANIM_JUMP_BOOST
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_JUMP_LAND_UPPER,
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_JUMP_LAND_LOWER,
		
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_FIRE_2,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_AIM_SUMMER,

	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_EJECT_OLD
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_GRAB_NEW
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_INSERT_NEW
	m_aBoneEnableIndices_UpperBody,		// ANIMTAP_RELOAD_CLIP_SLAPIN_NEW
};

const u8 CBotSlosh::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:

	255
};



const u8 CBotSlosh::m_anTagPointBoneNameIndexArray[] = {
	BONE_HEAD,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_R_ARM_UPPER,

	255
};


const u8				CBotSlosh::m_nApproxEyePointBoneNameIndex = BONE_HEAD;
FParticle_DefHandle_t	CBotSlosh::m_hBoostParticleDef			  = FPARTICLE_INVALID_HANDLE;


BOOL CBotSlosh::_BuildAnimStackDef( void )
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