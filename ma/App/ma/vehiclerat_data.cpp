//////////////////////////////////////////////////////////////////////////////////////
// vehiclerat_data.cpp - Bot data.
//
// Author: Chris MacDonald
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
// 11/07/02 MacDonald	Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "vehiclerat.h"
#include "botanim.h"
#include "fmesh.h"


const CBotDef CVehicleRat::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_RAT,							// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CVehicleRat::m_apszBoneNameTable[BONE_COUNT] = {
	"RATDummy",
	"Chassis",
	"crate_B",
	"crate_D",
	"crate_C",
	"crate_A",
	"R_Hatch",
	"L_Hatch",
	"crate_E",
	"L_Axle_Rear",
	"L_Hubcap_Rear",
	"L_Wheel_Rear",
	"L_Axle_Middle",
	"L_Hubcap_Middle",
	"L_Wheel_Middle",
	"L_Axle_Front",
	"L_Hubcap_Front",
	"L_Wheel_Front",
	"R_Axle_Rear",
	"R_Hubcap_Rear",
	"R_Wheel_Rear",
	"R_Axle_Middle",
	"R_Hubcap_Middle",
	"R_Wheel_Middle",
	"R_Axle_Front",
	"R_Hubcap_Front",
	"R_Wheel_Front",
	"AttachPoint_Turret",
	"AttachPoint_Driver",
	"AttachPoint_Gunner",
};

cchar *CVehicleRat::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"rest",		//	ANIMCONTROL_REST,
};


cchar *CVehicleRat::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"rest",		//	ANIMTAP_REST,
};


CFAnimCombinerConfig::ConfigStack_t CVehicleRat::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					CVehicleRat::ANIMCONTROL_REST,
	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CVehicleRat::m_aAnimCombinerConfigTaps[] = {
	CVehicleRat::ANIMCONTROL_REST,			CVehicleRat::ANIMTAP_REST,				1,
	255,									255,							255
};


CFAnimCombiner::AttachList_t CVehicleRat::m_aAnimAttach[] = {
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			CVehicleRat::ANIMCONTROL_REST,
	255,									255
};



const u8 *CVehicleRat::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,
};

const u8 CVehicleRat::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};


const u8 CVehicleRat::m_nApproxEyePointBoneNameIndex = BONE_R_HATCH;

const u8 CVehicleRat::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] = {
	BONE_RATDUMMY,
	BONE_CHASSIS,
	BONE_R_HATCH,
	BONE_L_WHEEL_FRONT,
	BONE_R_WHEEL_FRONT,
	BONE_L_WHEEL_MIDDLE,
	BONE_R_WHEEL_MIDDLE,
	BONE_L_WHEEL_REAR,
	BONE_R_WHEEL_REAR,
};

BOOL CVehicleRat::_BuildAnimStackDef( void )
{
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount					= 0;
	AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable		= NULL;
	AnimStackDefInit.nIdleCount					= ANIM_IDLE_COUNT;
	AnimStackDefInit.apszIdleAnimNameTable		= NULL;
	AnimStackDefInit.nBaseControlCount			= ANIMCONTROL_BASE_COUNT;
	AnimStackDefInit.apszBaseControlNameTable	= m_apszBaseControlNameTable;
	AnimStackDefInit.nBaseTapCount				= ANIMTAP_BASE_COUNT;
	AnimStackDefInit.apszBaseTapNameTable		= m_apszBaseTapNameTable;
	AnimStackDefInit.nBoneCount					= BONE_COUNT;
	AnimStackDefInit.apszBoneNameTable			= m_apszBoneNameTable;

	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap	= m_apnEnableBoneNameIndexTableForEachBaseTap;
	AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap	= NULL;
	AnimStackDefInit.pnEnableBoneNameIndexTableForSummer		= m_anEnableBoneNameIndexTableForSummer_Normal;

	AnimStackDefInit.pBaseAnimConfigNet					= NULL;
	AnimStackDefInit.pBaseAnimConfigStack				= m_aAnimCombinerConfigStack;
	AnimStackDefInit.pBaseAnimConfigTap					= m_aAnimCombinerConfigTaps;
	AnimStackDefInit.pBaseAnimAttachList				= m_aAnimAttach;
	AnimStackDefInit.nIdleStackConnectionControlIndex	= ANIMCONTROL_REST;
	AnimStackDefInit.nIdleStackConnectionControlInput	= 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
}

//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------
const FGameData_TableEntry_t CVehicleRat::m_aBotInfoVocab_Rat[] =
{
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupHatchClank
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupBoost
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupSkid

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CVehicleRat::m_aGameDataMap[] =
{
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"VehiclePhysics",
	m_aBotInfoVocab_VehiclePhysics,
	sizeof( m_BotInfo_VehiclePhysics ),
	(void*)&m_BotInfo_VehiclePhysics,

	"Vehicle",
	m_aBotInfoVocab_Vehicle,
	sizeof( m_BotInfo_Vehicle ),
	(void*)&m_BotInfo_Vehicle,

	"DriverCamera",
	CVehicleCamera::m_aBotInfoVocab_VehicleCamera,
	sizeof( m_BotInfo_DriverCamera ),
	(void*)&m_BotInfo_DriverCamera,

	"MPDriverCamera",
	CVehicleCamera::m_aBotInfoVocab_VehicleCamera,
	sizeof( m_BotInfo_MPDriverCamera ),
	(void*)&m_BotInfo_MPDriverCamera,

	"GunnerCamera",
	CVehicleCamera::m_aBotInfoVocab_VehicleCamera,
	sizeof( m_BotInfo_GunnerCamera ),
	(void*)&m_BotInfo_GunnerCamera,

	"MGGunnerCamera",
	CVehicleCamera::m_aBotInfoVocab_VehicleCamera,
	sizeof( m_BotInfo_MGGunnerCamera ),
	(void*)&m_BotInfo_MGGunnerCamera,

	"Rat",
	m_aBotInfoVocab_Rat,
	sizeof( m_BotInfo_Rat ),
	(void*)&m_BotInfo_Rat,

	"Engine",
	m_aBotInfoVocab_Engine,
	sizeof( m_BotInfo_Engine ),
	(void*)&m_BotInfo_Engine,

	NULL
};

