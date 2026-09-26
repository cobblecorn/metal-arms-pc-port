//////////////////////////////////////////////////////////////////////////////////////
// BotZombieBoss_data.h -
//
// Author: Michael Scholz
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
// 03/24/02 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "fanim.h"
#include "BotZombieBoss.h"
#include "botanim.h"
#include "fmesh.h"



//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotZombieBoss::m_BotDef =
{
	BOTRACE_ZOMBIE,							// nRace
	BOTCLASS_ZOMBIE_BOSS,					// nClass
	BOTSUBCLASS_ZOMBIEBOSS_NORMAL,			// nSubClass
};

//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotZombieBoss::m_aGameDataMap[] =
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

	NULL
};



//--------------------------------------------------------------------------------------------------------
// Animation Stack Data:
//--------------------------------------------------------------------------------------------------------

cchar *CBotZombieBoss::m_apszBoneNameTable[BONE_COUNT] =
{
	"ZombieBoss_Dummy",
	"L_Finger_3B",
	"Groin",
	"L_Leg_Upper",
	"L_Leg_Middle",
	"L_Leg_Lower",
	"L_Foot",
	"R_Leg_Upper",
	"R_Leg_Lower",
	"R_Foot",
	"Torso",
	"R_Arm_Upper",
	"R_Arm_Lower",
	"R_Hand",
	"Chain_Reel",
	"L_Arm_Upper",
	"L_Arm_Lower",
	"L_Hand",
	"L_Finger_2A",
	"L_Finger_2B",
	"L_Finger_3A",
	"L_Finger_1A",
	"L_Finger_1B",
	"Head",
	"Jaw",
	"Junk_Groin_B",
	"Hose_Torso",
	"Pipe_Back_C",
	"Pipe_Back_B",
	"Riblet",
	"Pipe_Back_A",
	"Junk_Torso_A",
	"Junk_Torso_B",
	"Hose_Groin",
	"Junk_Groin_C",
	"Junk_Groin_A",
	"Junk_Groin_D",
	"wrecking_ball",
	"AttachPoint_Chain",
	"AttachPoint_Glitch",
	"chain_link01",
	"chain_link02",
	"chain_link03",
	"chain_link04",
	"chain_link05",
	"chain_link06",
	"chain_link07",
	"chain_link08",
	"chain_link09",
	"chain_link10",
	"chain_link11",
	"chain_link12",
	"chain_link13",
	"chain_link14",
	"chain_link15",
	"chain_link16",
	"chain_link17",
	"chain_link18",
	"chain_link19",
	"chain_link20",
	"chain_link21",
	"chain_link22",
	"chain_link23",
	"chain_link24",
	"chain_link25",
	"chain_link26",
	"chain_link27",
	"chain_link28",
	"chain_link29",
	"chain_link30",
	"ptack_head",
	"ptack_smoke01",
	"ptack_smoke02",
	"ptack_smoke03",
};

cchar *CBotZombieBoss::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = 
{
	"ARZZidle001",	//	ANIM_STAND_LOOK,				
	"ARZZidle002",	//	ANIM_STAND_SWING				
	"ARZZwalkF01",	//	ANIM_WALK						

	"ARZZactn101",	//	ANIM_CHAIN_SWING,
	"ARZZactn102",	//	ANIM_CHAIN_ROLL,				
	"ARZZactn103",	//	ANIM_CHAIN_RETURN,
	
	"ARZZroar_01",	//	ANIM_ROAR,				
	"ARZZtug_001",	//	ANIM_TUG,

	"ARZZactn201",	//	ANIM_GRAB_ATTACK,
	
	"ARZZfreeze1",	//  ANIM_FREEZE
	"ARZZdeath01",	//  ANIM_DEATH
};


cchar *CBotZombieBoss::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] =
{
	"stndlook",		// ANIMCONTROL_STAND_LOOK
	"stndswng",		// ANIMCONTROL_STAND_SWING
	"walk",			// ANIMCONTROL_WALK

	"chnswingu",	// ANIMCONTROL_CHAIN_SWING_UPPER
	"chnrollu",		// ANIMCONTROL_CHAIN_ROLL_UPPER
	"chnreturnu",	// ANIMCONTROL_CHAIN_RETURN_UPPER

	"roaru",		// ANIMCONTROL_ROAR_UPPER,				
	"tugu",			// ANIMCONTROL_TUG_UPPER,				

	"chnswingl",	// ANIMCONTROL_CHAIN_SWING_UPPER
	"chnrolll",		// ANIMCONTROL_CHAIN_ROLL_UPPER
	"chnreturnl",	// ANIMCONTROL_CHAIN_RETURN_UPPER

	"roarl",		// ANIMCONTROL_ROAR_UPPER,				
	"tugl",			// ANIMCONTROL_TUG_UPPER,				
	
	"grab",			// ANIMCONTROL_GRAB_ATTACK

	"freeze",		// ANIMCONTROL_FREEZE
	"death",		// ANIMCONTROL_DEATH
};

cchar *CBotZombieBoss::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] =
{
	"stndlook",		// ANIMTAP_STAND_LOOK
	"stndswng",		// ANIMTAP_STAND_SWING
	"walk",			// ANIMTAP_WALK

	"chnswingu",	// ANIMTAP_CHAIN_SWING_UPPER
	"chnrollu",		// ANIMTAP_CHAIN_ROLL_UPPER
	"chnreturnu",	// ANIMTAP_CHAIN_RETURN_UPPER

	"roaru",		// ANIMTAP_ROAR_UPPER,				
	"tugu",			// ANIMTAP_TUG_UPPER,				

	"chnswingl",	// ANIMTAP_CHAIN_SWING_UPPER
	"chnrolll",		// ANIMTAP_CHAIN_ROLL_UPPER
	"chnreturnl",	// ANIMTAP_CHAIN_RETURN_UPPER

	"roarl",		// ANIMTAP_ROAR_UPPER,				
	"tugl",			// ANIMTAP_TUG_UPPER,				
	
	"grab",			// ANIMTAP_GRAB_ATTACK

	"freeze",		// ANIMTAP_FREEZE
	"death",		// ANIMTAP_DEATH
};


CFAnimCombinerConfig::ConfigStack_t CBotZombieBoss::m_aAnimCombinerConfigStack[] =
{
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_DEATH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_FREEZE,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_GRAB_ATTACK,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ROAR_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_TUG_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CHAIN_SWING_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CHAIN_ROLL_UPPER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CHAIN_RETURN_UPPER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ROAR_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_TUG_LOWER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CHAIN_SWING_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CHAIN_ROLL_LOWER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_CHAIN_RETURN_LOWER,

	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_SWING,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND_LOOK,

	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotZombieBoss::m_aAnimCombinerConfigTaps[] =
{
	ANIMTAP_STAND_LOOK,						ANIMCONTROL_STAND_LOOK,			1,
	ANIMTAP_STAND_SWING,					ANIMCONTROL_STAND_SWING,		1,
	ANIMTAP_WALK,							ANIMCONTROL_WALK,				1,

	ANIMTAP_CHAIN_SWING_UPPER,				ANIMCONTROL_CHAIN_SWING_UPPER,		1,
	ANIMTAP_CHAIN_ROLL_UPPER,				ANIMCONTROL_CHAIN_ROLL_UPPER,			1,
	ANIMTAP_CHAIN_RETURN_UPPER,				ANIMCONTROL_CHAIN_RETURN_UPPER,		1,

	ANIMTAP_ROAR_UPPER,						ANIMCONTROL_ROAR_UPPER,		1,
	ANIMTAP_TUG_UPPER,						ANIMCONTROL_TUG_UPPER,		1,

	ANIMTAP_CHAIN_SWING_LOWER,				ANIMCONTROL_CHAIN_SWING_LOWER,		1,
	ANIMTAP_CHAIN_ROLL_LOWER,				ANIMCONTROL_CHAIN_ROLL_LOWER,			1,
	ANIMTAP_CHAIN_RETURN_LOWER,				ANIMCONTROL_CHAIN_RETURN_LOWER,		1,

	ANIMTAP_ROAR_LOWER,						ANIMCONTROL_ROAR_LOWER,		1,
	ANIMTAP_TUG_LOWER,						ANIMCONTROL_TUG_LOWER,		1,

	ANIMTAP_GRAB_ATTACK,					ANIMCONTROL_GRAB_ATTACK,		1,

	ANIMTAP_FREEZE,							ANIMCONTROL_FREEZE,			1,
	ANIMTAP_DEATH,							ANIMCONTROL_DEATH,		1,
	255,									255,							255
};

CFAnimCombiner::AttachList_t CBotZombieBoss::m_aAnimAttach[] =
{
	ANIM_STAND_LOOK,		ANIMTAP_STAND_LOOK,
	ANIM_STAND_SWING,		ANIMTAP_STAND_SWING,		
	ANIM_WALK,				ANIMTAP_WALK,						

	ANIM_CHAIN_SWING,		ANIMTAP_CHAIN_SWING_UPPER,		
	ANIM_CHAIN_ROLL,		ANIMTAP_CHAIN_ROLL_UPPER,	
	ANIM_CHAIN_RETURN,		ANIMTAP_CHAIN_RETURN_UPPER,

	ANIM_ROAR,				ANIMTAP_ROAR_UPPER,
	ANIM_TUG,				ANIMTAP_TUG_UPPER,

	ANIM_CHAIN_SWING,		ANIMTAP_CHAIN_SWING_LOWER,		
	ANIM_CHAIN_ROLL,		ANIMTAP_CHAIN_ROLL_LOWER,	
	ANIM_CHAIN_RETURN,		ANIMTAP_CHAIN_RETURN_LOWER,

	ANIM_ROAR,				ANIMTAP_ROAR_LOWER,
	ANIM_TUG,				ANIMTAP_TUG_LOWER,

	ANIM_GRAB_ATTACK,		ANIMTAP_GRAB_ATTACK,
	ANIM_FREEZE,			ANIMTAP_FREEZE,
	ANIM_DEATH,				ANIMTAP_DEATH,
	255,						255
};

const u8 CBotZombieBoss::m_aBoneEnableIndices_FullBody[] =
{
	BONE_ZOMBIEBOSS_DUMMY,
	BONE_L_FINGER_3B,
	BONE_GROIN,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_MIDDLE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_TORSO,
	BONE_R_ARM_UPPER,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_CHAIN_REEL,
	BONE_L_ARM_UPPER,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_FINGER_2A,
	BONE_L_FINGER_2B,
	BONE_L_FINGER_3A,
	BONE_L_FINGER_1A,
	BONE_L_FINGER_1B,
	BONE_HEAD,
	BONE_JAW,
	BONE_JUNK_GROIN_B,
	BONE_HOSE_TORSO,
	BONE_PIPE_BACK_C,
	BONE_PIPE_BACK_B,
	BONE_RIBLET,
	BONE_PIPE_BACK_A,
	BONE_JUNK_TORSO_A,
	BONE_JUNK_TORSO_B,
	BONE_HOSE_GROIN,
	BONE_JUNK_GROIN_C,
	BONE_JUNK_GROIN_A,
	BONE_JUNK_GROIN_D,
	BONE_WRECKING_BALL,
	BONE_ATTACHPOINT_CHAIN,
	BONE_ATTACHPOINT_GLITCH,
	BONE_CHAIN_LINK01,
	BONE_CHAIN_LINK02,
	BONE_CHAIN_LINK03,
	BONE_CHAIN_LINK04,
	BONE_CHAIN_LINK05,
	BONE_CHAIN_LINK06,
	BONE_CHAIN_LINK07,
	BONE_CHAIN_LINK08,
	BONE_CHAIN_LINK09,
	BONE_CHAIN_LINK10,
	BONE_CHAIN_LINK11,
	BONE_CHAIN_LINK12,
	BONE_CHAIN_LINK13,
	BONE_CHAIN_LINK14,
	BONE_CHAIN_LINK15,
	BONE_CHAIN_LINK16,
	BONE_CHAIN_LINK17,
	BONE_CHAIN_LINK18,
	BONE_CHAIN_LINK19,
	BONE_CHAIN_LINK20,
	255
};

const u8 CBotZombieBoss::m_aBoneEnableIndices_FullBodyNoArm[] =
{
	BONE_L_FINGER_3B,
	BONE_GROIN,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_MIDDLE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_TORSO,
//	BONE_R_ARM_UPPER,
//	BONE_R_ARM_LOWER,
//	BONE_R_HAND,
//	BONE_CHAIN_REEL,
	BONE_L_ARM_UPPER,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_FINGER_2A,
	BONE_L_FINGER_2B,
	BONE_L_FINGER_3A,
	BONE_L_FINGER_1A,
	BONE_L_FINGER_1B,
	BONE_HEAD,
	BONE_JAW,
	BONE_JUNK_GROIN_B,
	BONE_HOSE_TORSO,
	BONE_PIPE_BACK_C,
	BONE_PIPE_BACK_B,
	BONE_RIBLET,
	BONE_PIPE_BACK_A,
	BONE_JUNK_TORSO_A,
	BONE_JUNK_TORSO_B,
	BONE_HOSE_GROIN,
	BONE_JUNK_GROIN_C,
	BONE_JUNK_GROIN_A,
	BONE_JUNK_GROIN_D,
//	BONE_WRECKING_BALL,
//	BONE_ATTACHPOINT_CHAIN,
	BONE_ATTACHPOINT_GLITCH,
	BONE_CHAIN_LINK01,
	BONE_CHAIN_LINK02,
	BONE_CHAIN_LINK03,
	BONE_CHAIN_LINK04,
	BONE_CHAIN_LINK05,
	BONE_CHAIN_LINK06,
	BONE_CHAIN_LINK07,
	BONE_CHAIN_LINK08,
	BONE_CHAIN_LINK09,
	BONE_CHAIN_LINK10,
	BONE_CHAIN_LINK11,
	BONE_CHAIN_LINK12,
	BONE_CHAIN_LINK13,
	BONE_CHAIN_LINK14,
	BONE_CHAIN_LINK15,
	BONE_CHAIN_LINK16,
	BONE_CHAIN_LINK17,
	BONE_CHAIN_LINK18,
	BONE_CHAIN_LINK19,
	BONE_CHAIN_LINK20,
	255
};

const u8 CBotZombieBoss::m_aBoneEnableIndices_UpperBody[] =
{
	BONE_TORSO,
	BONE_R_ARM_UPPER,
	BONE_R_ARM_LOWER,
	BONE_R_HAND,
	BONE_CHAIN_REEL,
	BONE_HEAD,
	BONE_JAW,
	BONE_HOSE_TORSO,
	BONE_PIPE_BACK_C,
	BONE_PIPE_BACK_B,
	BONE_RIBLET,
	BONE_PIPE_BACK_A,
	BONE_JUNK_TORSO_A,
	BONE_JUNK_TORSO_B,
	BONE_WRECKING_BALL,
	BONE_ATTACHPOINT_CHAIN,
	BONE_ATTACHPOINT_GLITCH,
	255
};
const u8 CBotZombieBoss::m_aBoneEnableIndices_LowerBody[] =
{
	BONE_GROIN,
	BONE_L_LEG_UPPER,
	BONE_L_LEG_MIDDLE,
	BONE_L_LEG_LOWER,
	BONE_L_FOOT,
	BONE_R_LEG_UPPER,
	BONE_R_LEG_LOWER,
	BONE_R_FOOT,
	BONE_JUNK_GROIN_B,
	BONE_HOSE_GROIN,
	BONE_JUNK_GROIN_C,
	BONE_JUNK_GROIN_A,
	BONE_JUNK_GROIN_D,
	BONE_L_ARM_UPPER,
	BONE_L_ARM_LOWER,
	BONE_L_HAND,
	BONE_L_FINGER_2A,
	BONE_L_FINGER_2B,
	BONE_L_FINGER_3A,
	BONE_L_FINGER_3B,
	BONE_L_FINGER_1A,
	BONE_L_FINGER_1B,
	255
};

const u8 CBotZombieBoss::m_anEnableBoneNameIndexTableForSummer_Normal[] =
{
	// Enable these bones to be driven by our summer:
	255
};

const u8 *CBotZombieBoss::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] =
{
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_STAND_LOOK, 
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_STAND_SWING,
	m_aBoneEnableIndices_FullBodyNoArm,		// ANIMTAP_WALK,

	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_CHAIN_SWING_UPPER,
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_CHAIN_ROLL_UPPER,
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_CHAIN_RETURN_UPPER,

	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_ROAR_UPPER,
	m_aBoneEnableIndices_UpperBody,			// ANIMTAP_TUG_UPPER,

	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_CHAIN_SWING_LOWER,
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_CHAIN_ROLL_LOWER,
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_CHAIN_RETURN_LOWER,

	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_ROAR_LOWER,
	m_aBoneEnableIndices_LowerBody,			// ANIMTAP_TUG_LOWER,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_GRAB_ATTACK,

	m_aBoneEnableIndices_FullBody,			// ANIMTAP_FREEZE,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_DEATH, 

};	


const u8 CBotZombieBoss::m_anTagPointBoneNameIndexArray[] =
{
	BONE_HEAD,
	BONE_TORSO,
	BONE_L_ARM_UPPER,
	BONE_R_ARM_UPPER,
	255
};

BOOL CBotZombieBoss::_BuildAnimStackDef( void )
{
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount = CBotAnimStackDef::ANIM_USER_COUNT;
	AnimStackDefInit.nBaseAnimNameCount = ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable = m_apszBaseAnimNameTable;
	AnimStackDefInit.nIdleCount = 0;
	AnimStackDefInit.apszIdleAnimNameTable = NULL;
	AnimStackDefInit.nBaseControlCount = ANIMCONTROL_BASE_COUNT;
	AnimStackDefInit.apszBaseControlNameTable = m_apszBaseControlNameTable;
	AnimStackDefInit.nBaseTapCount = ANIMTAP_BASE_COUNT;
	AnimStackDefInit.apszBaseTapNameTable = m_apszBaseTapNameTable;
	AnimStackDefInit.nBoneCount = BONE_COUNT;
	AnimStackDefInit.apszBoneNameTable = m_apszBoneNameTable;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap = m_apnEnableBoneNameIndexTableForEachBaseTap;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap = NULL;
	AnimStackDefInit.pnEnableBoneNameIndexTableForSummer = m_anEnableBoneNameIndexTableForSummer_Normal;
	AnimStackDefInit.pBaseAnimConfigNet = NULL;
	AnimStackDefInit.pBaseAnimConfigStack = m_aAnimCombinerConfigStack;
	AnimStackDefInit.pBaseAnimConfigTap = m_aAnimCombinerConfigTaps;
	AnimStackDefInit.pBaseAnimAttachList = m_aAnimAttach;
	AnimStackDefInit.nIdleStackConnectionControlIndex = ANIMCONTROL_STAND_LOOK;
	AnimStackDefInit.nIdleStackConnectionControlInput = 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
}