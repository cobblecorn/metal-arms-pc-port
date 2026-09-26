//////////////////////////////////////////////////////////////////////////////////////
// botmortar_data.cpp -
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
// 02/25/03 Scholz       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botmortar.h"
#include "fanim.h"
#include "botanim.h"
#include "fmesh.h"



//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotMortar::m_BotDef =
{
	BOTRACE_MIL,							// nRace
	BOTCLASS_MORTAR,						// nClass
	BOTSUBCLASS_MORTAR_GENERIC,				// nSubClass
};


const FGameData_TableEntry_t m_MortarInfoVocab[] = 
{
//	u32 uPoolCannonsBalls;
	FGAMEDATA_VAR_TYPE_FLOAT|FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//	f32 fOORoundsPerSec;
//	f32 fRoundsPerSec;
	FGAMEDATA_VOCAB_F32_BOUND_XOOX(F32_DATATABLE_1,F32_DATATABLE_60),

//	f32 m_fFiringTime;
//	f32 m_fOOFiringTime;
	FGAMEDATA_VOCAB_F32_BOUND_XOOX(F32_DATATABLE_Pt1,F32_DATATABLE_10),
		
//	f32 m_fUnrecoilTime;
//	f32 m_fOOUnrecoilTime;
	FGAMEDATA_VOCAB_F32_BOUND_XOOX(F32_DATATABLE_Pt1,F32_DATATABLE_10),

//		f32 m_fVisionDistance;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_Pt1,F32_DATATABLE_10000),

//		f32 m_fAlertVisionDistance;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_Pt1,F32_DATATABLE_10000),

//	cchar *pszLaunchExplosion;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//	cchar *pszDetonateExplosion;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

// f32 fMinCannonballVelocityXZ;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_1,F32_DATATABLE_10000),
// f32 fMaxCannonballVelocityXZ;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_1,F32_DATATABLE_10000),

//	f32	fMinRangeXZ;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_1,F32_DATATABLE_10000),
//	f32	fMaxRangeXZ;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_1,F32_DATATABLE_10000),

	//	f32 fRadiansMaxDeviate;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_0,F32_DATATABLE_10),

	// f32 fRetargetTime;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_0,F32_DATATABLE_100),

	// f32 fMinShockwaveImpulse;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_0,F32_DATATABLE_1000),

	//	f32 fMaxShockwaveImpulse;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_0,F32_DATATABLE_1000),

	// f32 fTargetRadius;
	FGAMEDATA_VOCAB_F32_BOUND(F32_DATATABLE_0,F32_DATATABLE_100),
	
	// CFSoundGroup* pCannonBallInAir;
	FGAMEDATA_VOCAB_SOUND_GROUP,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0,
};

//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotMortar::m_aGameDataMap[] =
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

	"MortarInfo",
	m_MortarInfoVocab,
	sizeof(m_MortarInfoVocab),
	(void *)&m_MortarInfo,
	NULL
};



//--------------------------------------------------------------------------------------------------------
// Animation Stack Data:
//--------------------------------------------------------------------------------------------------------

cchar *CBotMortar::m_apszBoneNameTable[BONE_COUNT] =
{
	"Mortardummy",
	"base",
	"r_front_ankle",
	"r_front_foot",
	"r_f_foot_dummy",
	"r_back_ankle",
	"r_back_foot",
	"r_b_foot_dummy",
	"l_front_ankle",
	"l_front_foot",
	"l_f_foot_dummy",
	"l_back_ankle",
	"l_back_foot",
	"l_b_foot_dummy",
	"swivel",
	"barrel_base",
	"barrel_mid",
	"barrel_top",
	"fire_primary",
};



cchar *CBotMortar::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] =
{
	"ARMMidle001",
	"ARMMwalkF01",
	"ARMMfire_01",
	"ARMMfire_02",
	"ARMMpwrdn01",
	"ARMMpwrup01",
};


cchar *CBotMortar::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] =
{
	"stand", 	// 	ANIMCONTROL_STAND,				// ARMMidle001
	"walk",		//	ANIMCONTROL_WALK,				// ARMMwalkF01
	"recoil",	//	ANIMCONTROL_RECOIL,				// ARMMfire_01
	"unrecoil",	//	ANIMCONTROL_UNRECOIL,			// ARMMfire_02
	"powerdn",	//	ANIMCONTROL_POWERDOWN,			
	"powerup",	//	ANIMCONTROL_POWERUP,			
};

cchar *CBotMortar::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] =
{
	"stand", 	// 	ANIMCONTROL_STAND,				// ARMMidle001
	"walk",		//	ANIMCONTROL_WALK,				// ARMMwalkF01
	"recoil",	//	ANIMCONTROL_RECOIL,				// ARMMfire_01
	"unrecoil",	//	ANIMCONTROL_UNRECOIL,			// ARMMfire_02
	"powerdn",	//	ANIMCONTROL_POWERDOWN,			
	"powerup",	//	ANIMCONTROL_POWERUP,			
};


CFAnimCombinerConfig::ConfigStack_t CBotMortar::m_aAnimCombinerConfigStack[] =
{
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_POWER_DOWN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RECOIL,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_UNRECOIL,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_WALK,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,
	CFAnimMixer::TYPE_COUNT,					255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotMortar::m_aAnimCombinerConfigTaps[] =
{
	ANIMTAP_STAND,								ANIMCONTROL_STAND,					1,
	ANIMTAP_WALK,								ANIMCONTROL_WALK,					1,
	ANIMTAP_RECOIL,								ANIMCONTROL_RECOIL,					1,
	ANIMTAP_UNRECOIL,							ANIMCONTROL_UNRECOIL,				1,
	ANIMTAP_POWER_DOWN,							ANIMCONTROL_POWER_DOWN,				1,
	ANIMTAP_POWER_UP,							ANIMCONTROL_POWER_UP,				1,
	255,										255,								255
};

CFAnimCombiner::AttachList_t CBotMortar::m_aAnimAttach[] =
{
	ANIM_STAND,						ANIMTAP_STAND,
	ANIM_WALK,						ANIMTAP_WALK,
	ANIM_RECOIL,					ANIMTAP_RECOIL,
	ANIM_UNRECOIL,					ANIMTAP_UNRECOIL,
	ANIM_POWER_DOWN,				ANIMTAP_POWER_DOWN,
	ANIM_POWER_UP,					ANIMTAP_POWER_UP,
	255,						255
};

const u8 CBotMortar::m_aBoneEnableIndices_FullBody[] =
{
	BONE_MORTARDUMMY,
	BONE_BASE,
	BONE_R_FRONT_ANKLE,
	BONE_R_FRONT_FOOT,
	BONE_R_F_FOOT_DUMMY,
	BONE_R_BACK_ANKLE,
	BONE_R_BACK_FOOT,
	BONE_R_B_FOOT_DUMMY,
	BONE_L_FRONT_ANKLE,
	BONE_L_FRONT_FOOT,
	BONE_L_F_FOOT_DUMMY,
	BONE_L_BACK_ANKLE,
	BONE_L_BACK_FOOT,
	BONE_L_B_FOOT_DUMMY,
	BONE_SWIVEL,
	BONE_BARREL_BASE,
	BONE_BARREL_MID,
	BONE_BARREL_TOP,
	BONE_FIRE_PRIMARY,
	255
};

const u8 CBotMortar::m_aBoneEnableIndices_FullBodyExceptAimSummer[] =
{
	BONE_MORTARDUMMY,
	BONE_BASE,
	BONE_R_FRONT_ANKLE,
	BONE_R_FRONT_FOOT,
	BONE_R_F_FOOT_DUMMY,
	BONE_R_BACK_ANKLE,
	BONE_R_BACK_FOOT,
	BONE_R_B_FOOT_DUMMY,
	BONE_L_FRONT_ANKLE,
	BONE_L_FRONT_FOOT,
	BONE_L_F_FOOT_DUMMY,
	BONE_L_BACK_ANKLE,
	BONE_L_BACK_FOOT,
	BONE_L_B_FOOT_DUMMY,
	BONE_BARREL_MID,
	BONE_BARREL_TOP,
	BONE_FIRE_PRIMARY,
	255
};


const u8 CBotMortar::m_aBoneEnableIndices_AimSummer[] =
{
	BONE_SWIVEL,
	BONE_BARREL_BASE,
	255
};

const u8 *CBotMortar::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] =
{
	m_aBoneEnableIndices_FullBody,							//	ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,							//	ANIMTAP_WALK,
	m_aBoneEnableIndices_FullBodyExceptAimSummer,			//	ANIMTAP_RECOIL,
	m_aBoneEnableIndices_FullBodyExceptAimSummer,			//	ANIMTAP_UNRECOIL,
	m_aBoneEnableIndices_FullBody,							//	ANIMTAP_POWERDOWN,
	m_aBoneEnableIndices_FullBody,							//	ANIMTAP_POWERUP,
};


const u8 CBotMortar::m_anTagPointBoneNameIndexArray[] =
{
	BONE_BASE,
	BONE_BARREL_MID,
	BONE_R_FRONT_ANKLE,
	BONE_L_FRONT_ANKLE,

	255
};


BOOL CBotMortar::_BuildAnimStackDef( void )
{
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount = CBotAnimStackDef::ANIM_USER_COUNT;
	AnimStackDefInit.nBaseAnimNameCount = ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable = m_apszBaseAnimNameTable;
	AnimStackDefInit.nIdleCount = ANIM_IDLE_COUNT;
	AnimStackDefInit.apszIdleAnimNameTable = NULL;//m_apszIdleAnimNameTable;
	AnimStackDefInit.nBaseControlCount = ANIMCONTROL_BASE_COUNT;
	AnimStackDefInit.apszBaseControlNameTable = m_apszBaseControlNameTable;
	AnimStackDefInit.nBaseTapCount = ANIMTAP_BASE_COUNT;
	AnimStackDefInit.apszBaseTapNameTable = m_apszBaseTapNameTable;
	AnimStackDefInit.nBoneCount = BONE_COUNT;
	AnimStackDefInit.apszBoneNameTable = m_apszBoneNameTable;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap = m_apnEnableBoneNameIndexTableForEachBaseTap;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap = NULL;// m_apnEnableBoneNameIndexTableForEachIdleTap;
	AnimStackDefInit.pnEnableBoneNameIndexTableForSummer = NULL;//m_anEnableBoneNameIndexTableForSummer_Normal;
	AnimStackDefInit.pBaseAnimConfigNet = NULL;
	AnimStackDefInit.pBaseAnimConfigStack = m_aAnimCombinerConfigStack;
	AnimStackDefInit.pBaseAnimConfigTap = m_aAnimCombinerConfigTaps;
	AnimStackDefInit.pBaseAnimAttachList = m_aAnimAttach;
	AnimStackDefInit.nIdleStackConnectionControlIndex = ANIMCONTROL_STAND;
	AnimStackDefInit.nIdleStackConnectionControlInput = 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
}