//////////////////////////////////////////////////////////////////////////////////////
// botprobe_data.cpp - Bot data.
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
// 12/30/02 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "botprobe.h"

//--------------------------------------------------------------------------------------------------------
// Game Data Vocab:
//--------------------------------------------------------------------------------------------------------

const FGameData_TableEntry_t CBotProbe::m_aBotInfoVocab_Probe[] = {
	// fGrabCollisionSphereBumpX
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fGrabCollisionSphereBumpY
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fGrabCollisionSphereBumpZ
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fGrabCollisionSphereRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinDeathZoneX_MS
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinDeathZoneY_MS
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinDeathZoneZ_MS
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSpinCollisionSphereRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMagnetOuterRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMagnetYRange
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszJetParticleName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszSparkParticleName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszPuffParticleName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszDamageProfilePinch
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszDamageProfileShake
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszDamageProfileSpin
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotProbe::m_aGameDataMap[] = {
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"MountAim",
	m_aBotInfoVocab_MountAim,
	sizeof(m_BotInfo_MountAim),
	(void *)&m_BotInfo_MountAim,

	"Probe",
	m_aBotInfoVocab_Probe,
	sizeof(m_BotInfo_Probe),
	(void *)&m_BotInfo_Probe,

	NULL
};

const CBotDef CBotProbe::m_BotDef = {
	BOTRACE_MIL,					// nRace
	BOTCLASS_PROBE,					// nClass
	BOTSUBCLASS_PROBE_NORMAL,		// nSubClass
};

cchar *CBotProbe::m_apszBoneNameTable[BONE_COUNT] = {
	"Torso",			// BONE_TORSO,
	"Head",				// BONE_HEAD,
	"Leg1_A",			// BONE_LEG1_A,
	"Leg1_B",			// BONE_LEG1_B,
    "Leg1_C",			// BONE_LEG1_C,
	"Leg1_D",			// BONE_LEG1_D,
	"Leg1_E",			// BONE_LEG1_E,
	"Leg2_A",			// BONE_LEG2_A,
	"Leg2_B",			// BONE_LEG2_B,
    "Leg2_C",			// BONE_LEG2_C,
	"Leg2_D",			// BONE_LEG2_D,
	"Leg2_E",			// BONE_LEG2_E,
	"Leg3_A",			// BONE_LEG3_A,
	"Leg3_B",			// BONE_LEG3_B,
    "Leg3_C",			// BONE_LEG3_C,
	"Leg3_D",			// BONE_LEG3_D,
	"Leg3_E",			// BONE_LEG3_E,
	"AttachPoint_Jet",	// BONE_ATTACH_POINT_JET
};

cchar *CBotProbe::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMLidle001",	// ANIM_STAND,
	"ARMLactn101",	// ANIM_ATTACK_LEECH,
	"ARMLactn201",	// ANIM_ATTACK_SPIN,
	"ARMLfling01",  // ANIM_ATTACK_FLING,
	"ARMLpwrdn01",  // ANIM_POWER_DOWN,
	"ARMLpwrup01",  // ANIM_POWER_UP,
};

cchar *CBotProbe::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMLidle001"
};


cchar *CBotProbe::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"stand",		// ANIMCONTROL_STAND,
	"attack_leech", // ANIMCONTROL_ATTACK_LEECH,
	"attack_spin",	// ANIMCONTROL_ATTACK_SPIN,
	"attack_fling", // ANIMCONTROL_ATTACK_FLING,
	"power_down",	// ANIMCONTROL_POWER_DOWN,
	"power_up",		// ANIMCONTROL_POWER_UP,
};



cchar *CBotProbe::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",		// ANIMTAP_STAND,
	"attack_leech",	// ANIMTAP_ATTACK_LEECH,
	"attack_spin",	// ANIMTAP_ATTACK_SPIN,
	"attack_fling", // ANIMTAP_ATTACK_FLING
	"power_down",	// ANIMTAP_POWER_DOWN,
	"power_up",		// ANIMTAP_POWER_UP,
};


CFAnimCombinerConfig::ConfigStack_t CBotProbe::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ATTACK_FLING,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ATTACK_LEECH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_ATTACK_SPIN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_POWER_DOWN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,

	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CBotProbe::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,								ANIMCONTROL_STAND,				1,
	ANIMTAP_ATTACK_LEECH,						ANIMCONTROL_ATTACK_LEECH,		1,
	ANIMTAP_ATTACK_SPIN,						ANIMCONTROL_ATTACK_SPIN,		1,
	ANIMTAP_ATTACK_FLING,						ANIMCONTROL_ATTACK_FLING,		1,
	ANIMTAP_POWER_DOWN,							ANIMCONTROL_POWER_DOWN,			1,
	ANIMTAP_POWER_UP,							ANIMCONTROL_POWER_UP,			1,
	255,										255,							255
};


CFAnimCombiner::AttachList_t CBotProbe::m_aAnimAttach[] = {
	ANIM_STAND,									ANIMTAP_STAND,
	ANIM_ATTACK_LEECH,							ANIMTAP_ATTACK_LEECH,
	ANIM_ATTACK_SPIN,							ANIMTAP_ATTACK_SPIN,
	ANIM_ATTACK_FLING,							ANIMTAP_ATTACK_FLING,
	ANIM_POWER_DOWN,							ANIMTAP_POWER_DOWN,
	ANIM_POWER_UP,								ANIMTAP_POWER_UP,
	255,										255
};

const u8 CBotProbe::m_anTagPointBoneNameIndexArray[] = {
	BONE_TORSO,
	BONE_HEAD,
	BONE_LEG1_E,
	BONE_LEG2_E,
	BONE_LEG3_E,

	255
};

const u8 *CBotProbe::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

const u8 *CBotProbe::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	NULL,									// Idle 0
};


const u8 CBotProbe::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	BONE_TORSO,
	BONE_HEAD,
	BONE_LEG1_A,
	BONE_LEG1_B,
	BONE_LEG1_C,
	BONE_LEG1_D,
	BONE_LEG1_E,
	BONE_LEG2_A,
	BONE_LEG2_B,
	BONE_LEG2_C,
	BONE_LEG2_D,
	BONE_LEG2_E,
	BONE_LEG3_A,
	BONE_LEG3_B,
	BONE_LEG3_C,
	BONE_LEG3_D,
	BONE_LEG3_E,
	BONE_ATTACH_POINT_JET,
	255
};

const u8 CBotProbe::m_nApproxEyePointBoneNameIndex = BONE_HEAD;

BOOL CBotProbe::_BuildAnimStackDef( void ) {
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