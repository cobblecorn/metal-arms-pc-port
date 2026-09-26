//////////////////////////////////////////////////////////////////////////////////////
// vehiclesentinel_data.cpp - Bot data.
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
#include "vehiclesentinel.h"
#include "botanim.h"
#include "fmesh.h"


const CBotDef CVehicleSentinel::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_SENTINEL,						// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};


cchar *CVehicleSentinel::m_apszBoneNameTable[BONE_COUNT] = {
	"TankDummy",
	"Torso",
	"R_Wheel_E",
	"L_Wheel_E",
	"L_Wheel_D",
	"L_Wheel_C",
	"L_Wheel_B",
	"L_Wheel_A",
	"R_Wheel_D",
	"R_Wheel_A",
	"R_Wheel_B",
	"R_Wheel_C",
	"Turret",
	"CannonBase",
	"Cannon",
	"Primary_Fire",
	"MachineGun",
	"Secondary_Fire",
	"Spotlight",
	"AttachPoint_Lite",
	"L_Joystick",
	"R_Joystick",
	"AttachPoint_Driver",
	"Stuff_D",
	"Stuff_C",
	"Stuff_A",
	"Stuff_B",
	"CollBoneR1",
	"CollBoneL1",
	"AntennaA",
	"AntennaB",
	"AntennaC",
	"AntennaD",
	"AntennaE",
};

cchar *CVehicleSentinel::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"rest",		//	ANIMCONTROL_REST,
};


cchar *CVehicleSentinel::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"rest",		//	ANIMTAP_REST,
};


CFAnimCombinerConfig::ConfigStack_t CVehicleSentinel::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					CVehicleSentinel::ANIMCONTROL_REST,
	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CVehicleSentinel::m_aAnimCombinerConfigTaps[] = {
	CVehicleSentinel::ANIMCONTROL_REST,			CVehicleSentinel::ANIMTAP_REST,				1,
	255,									255,							255
};


CFAnimCombiner::AttachList_t CVehicleSentinel::m_aAnimAttach[] = {
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			CVehicleSentinel::ANIMCONTROL_REST,
	255,									255
};



const u8 *CVehicleSentinel::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,
};

const u8 CVehicleSentinel::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};


const u8 CVehicleSentinel::m_nApproxEyePointBoneNameIndex = BONE_TURRET;

const u8 CVehicleSentinel::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] = {
	BONE_TANKDUMMY,
	BONE_TORSO,
	BONE_L_WHEEL_A,
	BONE_R_WHEEL_A,
	BONE_L_WHEEL_C,
	BONE_R_WHEEL_C,
	BONE_L_WHEEL_E,
	BONE_R_WHEEL_E,
	BONE_PRIMARY_FIRE,
	BONE_SECONDARY_FIRE,
	BONE_TURRET,
	BONE_ATTACHPOINT_DRIVER,
	BONE_COLLBONE_R1,
	BONE_COLLBONE_L1,
};

BOOL CVehicleSentinel::_BuildAnimStackDef( void )
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

const FGameDataMap_t CVehicleSentinel::m_aGameDataMap[] = {
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

	"Engine",
	m_aBotInfoVocab_Engine,
	sizeof( m_BotInfo_Engine ),
	(void*)&m_BotInfo_Engine,

	NULL
};

