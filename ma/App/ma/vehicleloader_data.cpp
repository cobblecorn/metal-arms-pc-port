//////////////////////////////////////////////////////////////////////////////////////
// vehicleloader_data.cpp -
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
// 01/14/02 Elliott 	Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "vehicleloader.h"


const CBotDef CVehicleLoader::m_BotDef = {
	BOTRACE_MIL,							// nRace
	BOTCLASS_LOADER,						// nClass
	BOTSUBCLASS_NONE,						// nSubClass
};



cchar *CVehicleLoader::m_apszBoneNameTable[BONE_COUNT] = {
	"AttachPoint_Jet_R",	//BONE_JET_R_ATTACH,	
	"Fuselage",				//BONE_FUSELAGE,		
	"L_Engine",				//BONE_ENGINE_L,		
	"AttachPoint_Jet_L",	//BONE_JET_L_ATTACH,	
	"R_Engine",				//BONE_ENGINE_R,		
	"Claw_Arm",				//BONE_CLAW_ARM,		
	"L_Claw",				//BONE_CLAW_L,		
	"R_Claw",				//BONE_CLAW_R,		
	"AttachPoint_Driver",	//BONE_DRIVER_ATTACH,	
	"Primary_Fire",			//BONE_PRIMARY_FIRE,
	"AttachPoint_Victim",	//BONE_VICTIM_ATTACH,
	"Gun",					//BONE_GUN,
};

cchar *CVehicleLoader::m_apszAnimBoneNames[ANIMBONE_COUNT] = {
	"L_Engine",				//ANIMBONE_ENGINE_LEFT,
	"R_Engine",				//ANIMBONE_ENGINE_RIGHT,
	"L_Claw",				//ANIMBONE_PINCER_LEFT,
	"R_Claw",				//ANIMBONE_PINCER_RIGHT,
	"Claw_Arm",				//ANIMBONE_PINCER_ARM,
	"Gun",					//ANIMBONE_GUN
};

const u8 CVehicleLoader::m_nApproxEyePointBoneNameIndex = BONE_DRIVER_ATTACH;

const u8 CVehicleLoader::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] = {
	BONE_FUSELAGE,
	BONE_ENGINE_L,
	BONE_ENGINE_R,
	BONE_DRIVER_ATTACH,
};



//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CVehicleLoader::m_aGameDataMap[] = {
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"MountAim",
	m_aBotInfoVocab_MountAim,
	sizeof(m_BotInfo_MountAim),
	(void *)&m_BotInfo_MountAim,

	"Loader",
	m_aBotInfoVocab_Loader,
	sizeof( m_BotInfo_Loader ),
	(void*)&m_BotInfo_Loader,

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

	NULL
};

const FGameDataMap_t CVehicleLoader::m_aSoundDataMap[] = {
	"Sounds",
	m_aBotInfoVocab_LoaderSounds,
	sizeof(m_BotInfo_LoaderSounds),
	(void *)&m_BotInfo_LoaderSounds,

	NULL
};





const FGameData_TableEntry_t CVehicleLoader::m_aBotInfoVocab_Loader[] = {
	//pszExhaustParticle;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//pszEngineStopParticle
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//fDecelBonusZ;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fDecelBonusX
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fOOGunRoundsPerSec;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_30,

	//fShotSpreadRate;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	//fShotSpreadRecRate;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fMinShotSpread;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fMaxShotSpread;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fMaxRange;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	//fMuzzleFlashZWidth;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fMuzzleFlashZHeight;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fMuzzleFlashZAlpha;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fMuzzleFlashXYScale;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fMuzzleFlashXYAlpha;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fMuzzleFlashMeshScale
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fMuzzleLightRadius;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_1000,

	//fMuzzleImpactScale;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fMuzzleImpactAlpha;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,	
	
	//fMuzzleImpactOffset;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	//fPotmarkScale;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fTracerTime;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10,

	//fTracerWidth;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_100,

	//fTracerAlpha;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_1,

	//fSoundVolume3D;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	//fSoundRadius3D;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_1000,

	//fSoundVolume2D;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_1,

	//hBulletDecal
	FGAMEDATA_VOCAB_DECAL_DEF,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameData_TableEntry_t CVehicleLoader::m_aBotInfoVocab_LoaderSounds[] = {

	//FSndFx_FxHandle_t hPowerUp;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hPowerDown;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hHoverHigh;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hHoverLow;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hClawMove;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFx_FxHandle_t hClawClose;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
	
	//FSndFx_FxHandle_t hClawCloseObject;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	//FSndFX_FxHandle_t hGun;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};