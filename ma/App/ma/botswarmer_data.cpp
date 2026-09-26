//////////////////////////////////////////////////////////////////////////////////////
// botswarmer_data.cpp - Bot data.
//
// Author: Nathan Miller
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
// 01/22/03 Miller     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botswarmer.h"
#include "botanim.h"

const CBotDef CBotSwarmer::m_BotDef = {
	BOTRACE_MIL,					// nRace
	BOTCLASS_SWARMER,				// nClass
	BOTSUBCLASS_SWARMER_NORMAL,		// nSubClass
};

cchar *CBotSwarmer::m_apszBoneNameTable[BONE_COUNT] = {
	"Torso",
	"LF_Leg_A",
	"LB_Leg_A",
	"RF_Leg_A",
	"RB_Leg_A",
};


cchar *CBotSwarmer::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMWwalkF01",
	"ARMWidle001",
	"ARMWjump101",
	"ARMWactn101"
};

cchar *CBotSwarmer::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMWidle001"
};


cchar *CBotSwarmer::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"walk",
	"stand", 
	"jump",
	"attack"
};



cchar *CBotSwarmer::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"walk",
	"stand", 
	"jump",
	"attack"
};


CFAnimCombinerConfig::ConfigStack_t CBotSwarmer::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ATTACK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_JUMP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CBotSwarmer::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_WALK,							ANIMCONTROL_WALK,				1,
	ANIMTAP_STAND,							ANIMCONTROL_STAND,				1,
	ANIMTAP_JUMP,							ANIMCONTROL_JUMP,				1,
	ANIMTAP_ATTACK,							ANIMCONTROL_ATTACK,				1,

	255,									255,							255
};


CFAnimCombiner::AttachList_t CBotSwarmer::m_aAnimAttach[] = {
	ANIM_WALK,							ANIMTAP_WALK,
	ANIM_STAND,							ANIMTAP_STAND,
	ANIM_JUMP,							ANIMTAP_JUMP,
	ANIM_ATTACK,						ANIMTAP_ATTACK,

	255,								255
};

const u8 *CBotSwarmer::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,
	NULL,
	NULL,
};

const u8 *CBotSwarmer::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	NULL,
};

const u8 CBotSwarmer::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};	

const u8 CBotSwarmer::m_nApproxEyePointBoneNameIndex = BONE_TORSO;

BOOL CBotSwarmer::_BuildAnimStackDef( void ) {
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