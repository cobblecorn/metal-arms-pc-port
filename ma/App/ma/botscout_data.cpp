//////////////////////////////////////////////////////////////////////////////////////
// botscout_data.cpp - Jump Trooper bot data
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
// 02/09/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botscout.h"
#include "botanim.h"
#include "fmesh.h"

const CBotDef CBotScout::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_SCOUT,							// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CBotScout::m_apszBoneNameTable[BONE_COUNT] = {
	"Head",			// BONE_HEAD,
	"Torso",		// BONE_TORSO,			
	"Neck1",		// BONE_NECK1,
	"Neck2",		// BONE_NECK2,
	"Neck3",		// BONE_NECK3,
	"Neck4",		// BONE_NECK4,
	"Antenna1",		// BONE_ANTENNA1
	"Antenna2",		// BONE_ANTENNA2
	"Antenna3",		// BONE_ANTENNA3
	"Antenna4",		// BONE_ANTENNA4
	"AttachPoint_Beam", // BONE_ATTACHPOINT_BEAM
	"ScoutDummy",	// BONE_DUMMY
};

cchar *CBotScout::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMUscan_01",	// ANIM_SCAN,
    "ARMUheadup1",	// ANIM_HEADUP,
};

cchar *CBotScout::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"scan",			//	ANIMCONTROL_SCAN,
	"headup",		//	ANIMCONTROL_HEADUP,
};


cchar *CBotScout::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"scan",			//	ANIMTAP_SCAN,
	"headup",		//	ANIMTAP_HEADUP,
};


CFAnimCombinerConfig::ConfigStack_t CBotScout::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_HEADUP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_SCAN,
	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotScout::m_aAnimCombinerConfigTaps[] = {
	ANIMCONTROL_SCAN,			ANIMTAP_SCAN,				1,
	ANIMCONTROL_HEADUP,			ANIMTAP_HEADUP,				1,
	255,									255,			255
};


CFAnimCombiner::AttachList_t CBotScout::m_aAnimAttach[] = {
	ANIM_SCAN,				ANIMTAP_SCAN,
	ANIM_HEADUP,			ANIMTAP_HEADUP,
	255,					255
};

const u8 CBotScout::m_aBoneEnableIndices_FullBody[] = {
	BONE_HEAD,				// "Groin",						
	BONE_TORSO,				// "Torso",
	BONE_NECK1,				// "Neck1"
	BONE_NECK2,				// "Neck2"
	BONE_NECK3,				// "Neck3"
	BONE_NECK4,				// "Neck4"
	BONE_ANTENNA1,			// "Antenna1"
	BONE_ANTENNA2,			// "Antenna2"
	BONE_ANTENNA3,			// "Antenna3"
	BONE_ANTENNA4,			// "Antenna4"
	BONE_DUMMY,				// "ScoutDummy"

	255
};


const u8 *CBotScout::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_SCAN,
	m_aBoneEnableIndices_FullBody,				// ANIMTAP_HEADUP,
};

const u8 CBotScout::m_nApproxEyePointBoneNameIndex = BONE_HEAD;

const u8 CBotScout::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] = {
	BONE_TORSO,
	BONE_HEAD,
	BONE_ANTENNA3,
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameData_TableEntry_t CBotScout::m_aBotInfoVocab_Scout[] = {

	// hSmokeParticle:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDamageSlowdownAmount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// apszScanConeMeshName[0]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fScanConeMinScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// fScanConeScaleRange:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// fScanConeScaleFrequency:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// fScanConeLightFlicker:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAlertChirp
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupScanning
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupServo1
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupServo2
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupTreads
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupAlarmZap
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupTick
#if FANG_WINGC
	// Retail vocabulary (main.dol): a siren sound before the damage profile and a debris group after it.
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupSiren
	FGAMEDATA_VOCAB_DAMAGE,			// pScoutDetDamageProfile:
	FGAMEDATA_VOCAB_DEBRIS_GROUP,	// pDebrisGroupGuts
#else
	FGAMEDATA_VOCAB_DAMAGE,			// pScoutDetDamageProfile:
#endif

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CBotScout::m_aGameDataMap[] = {
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"Walk",
	m_aBotInfoVocab_Walk,
	sizeof(m_BotInfo_Walk),
	(void *)&m_BotInfo_Walk,

	"Weapon",
	m_aBotInfoVocab_Weapon,
	sizeof(m_BotInfo_Weapon),
	(void *)&m_BotInfo_Weapon,

	"Scout",
	m_aBotInfoVocab_Scout,
	sizeof(m_BotInfo_Scout),
	(void *)&m_BotInfo_Scout,
	NULL
};


BOOL CBotScout::_BuildAnimStackDef( void ) {
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount					= CBotAnimStackDef::ANIM_USER_COUNT;
	AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable		= m_apszBaseAnimNameTable;
	AnimStackDefInit.nIdleCount					= 0;
	AnimStackDefInit.apszIdleAnimNameTable		= NULL;
	AnimStackDefInit.nBaseControlCount			= ANIMCONTROL_BASE_COUNT;
	AnimStackDefInit.apszBaseControlNameTable	= m_apszBaseControlNameTable;
	AnimStackDefInit.nBaseTapCount				= ANIMTAP_BASE_COUNT;
	AnimStackDefInit.apszBaseTapNameTable		= m_apszBaseTapNameTable;
	AnimStackDefInit.nBoneCount					= BONE_COUNT;
	AnimStackDefInit.apszBoneNameTable			= m_apszBoneNameTable;

	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap	= m_apnEnableBoneNameIndexTableForEachBaseTap;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap	= NULL;
	AnimStackDefInit.pnEnableBoneNameIndexTableForSummer		= m_aBoneEnableIndices_FullBody;

	AnimStackDefInit.pBaseAnimConfigNet					= NULL;
	AnimStackDefInit.pBaseAnimConfigStack				= m_aAnimCombinerConfigStack;
	AnimStackDefInit.pBaseAnimConfigTap					= m_aAnimCombinerConfigTaps;
	AnimStackDefInit.pBaseAnimAttachList				= m_aAnimAttach;
	AnimStackDefInit.nIdleStackConnectionControlIndex	= ANIMCONTROL_SCAN;	// must index last entry in config stack
	AnimStackDefInit.nIdleStackConnectionControlInput	= 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
}

