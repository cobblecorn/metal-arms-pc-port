//////////////////////////////////////////////////////////////////////////////////////
// botAAgun_data.cpp -
//
// Author: Michael Starich
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
// 3/14/03 Starich       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "botAAgun.h"
#include "fanim.h"
#include "botanim.h"
#include "fmesh.h"


//--------------------------------------------------------------------------------------------------------
// Bot Definition:
//--------------------------------------------------------------------------------------------------------

const CBotDef CBotAAGun::m_BotDef = {
	BOTRACE_MIL,					// nRace
	BOTCLASS_AAGUN,					// nClass
	BOTSUBCLASS_AAGUN_NORMAL,		// nSubClass
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotAAGun::m_aGameDataMap[] = {
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"MountAim",
	m_aBotInfoVocab_MountAim,
	sizeof(m_BotInfo_MountAim),
	(void *)&m_BotInfo_MountAim,

	"Weapon",
	m_aBotInfoVocab_Weapon,
	sizeof(m_BotInfo_Weapon),
	(void *)&m_BotInfo_Weapon,

	"AAGun",
	m_aBotInfoVocab_AAGun,
	sizeof( m_BotInfo_AAGun ),
	(void*)&m_BotInfo_AAGun,

	"MortarInfo",
	m_aBotInfoVocab_Mortar,
	sizeof( m_MortarInfo ),
	(void *)&m_MortarInfo,
	
	NULL
};

const FGameData_TableEntry_t CBotAAGun::m_aBotInfoVocab_AAGun[] = {
	
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
	F32_DATATABLE_5000,

	//fMuzzleLightRadius;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_1000,

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

	//CFSoundGroup *pGunFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	

	// fMaxHeading;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMinHeading;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fHeadingPerSec;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fPitchPerSec;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fSecsBetweenBarrelFires;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_ABS_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fRecoilBack;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_ABS_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fPercentRecoilBackwards;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_UNIT_FLOAT | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	FGAMEDATA_VOCAB_DAMAGE,	// pPrimaryWeaponDamageProfile
	FGAMEDATA_VOCAB_PARTICLE,// hFlashPartDef;
	FGAMEDATA_VOCAB_PARTICLE,// hSmokePartDef;
	FGAMEDATA_VOCAB_PARTICLE,// hFlakPartDef;
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_Neg1, F32_DATATABLE_1 ),// fReticleX
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_Neg1, F32_DATATABLE_1 ),// fReticleY
	FGAMEDATA_VOCAB_F32_UNBOUND,// fEnterRadius;
	FGAMEDATA_VOCAB_F32_UNBOUND,// fEnterHalfHeight;
	FGAMEDATA_VOCAB_SOUND_GROUP,// CFSoundGroup *pRotateTuret;

	FGAMEDATA_VOCAB_DECAL_DEF,	// hBulletDecalDef

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CBotAAGun::m_aBotInfoVocab_Mortar[] = {
//	u32 uPoolCannonsBalls;
	FGAMEDATA_VAR_TYPE_FLOAT|FGAMEDATA_FLAGS_CONVERT_TO_U32,sizeof( u32 ),F32_DATATABLE_0,F32_DATATABLE_0,

//	f32 fRoundsPerSec;
//	f32 fOORoundsPerSec;
	FGAMEDATA_VOCAB_F32_BOUND_XOOX( F32_DATATABLE_1, F32_DATATABLE_60 ),

//	cchar *pszLaunchExplosion;
	FGAMEDATA_VOCAB_EXPLODE_GROUP,

//	cchar *pszDetonateExplosion;
	FGAMEDATA_VOCAB_EXPLODE_GROUP,

// f32 fMinCannonballVelocityXZ;
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_1, F32_DATATABLE_10000 ),
// f32 fMaxCannonballVelocityXZ;
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_1, F32_DATATABLE_10000 ),

//	f32	fMinRangeXZ;
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_1, F32_DATATABLE_10000 ),
//	f32	fMaxRangeXZ;
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_1, F32_DATATABLE_10000 ),

// f32 fMinShockwaveImpulse;
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_0, F32_DATATABLE_1000 ),

//	f32 fMaxShockwaveImpulse;
	FGAMEDATA_VOCAB_F32_BOUND( F32_DATATABLE_0, F32_DATATABLE_1000 ),

// CFSoundGroup* pCannonBallInAir;
	FGAMEDATA_VOCAB_SOUND_GROUP,

// CFSoundGroup *pMortarFire;
	FGAMEDATA_VOCAB_SOUND_GROUP,

// CFSoundGroup *pMortarExplode;
	FGAMEDATA_VOCAB_SOUND_GROUP,

// CFSoundGroup *pMortarNoFire;
	FGAMEDATA_VOCAB_SOUND_GROUP,

// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

//--------------------------------------------------------------------------------------------------------
// Animation Stack Data:
//--------------------------------------------------------------------------------------------------------

cchar *CBotAAGun::m_apszBoneNameTable[BONE_COUNT] = {
	"Barrel_T_R",		// BONE_BARREL_T_R
	"Barrel_B_L",		// BONE_BARREL_B_L
	"Barel_T_L",		// BONE_BARREL_T_L
	"Barrel_B_R",		// BONE_BARREL_B_R

	"Sheath_T_R",		// BONE_SHEATH_T_R
	"Sheath_B_L",		// BONE_SHEATH_B_L
	"Sheath_T_L",		// BONE_SHEATH_T_L
	"Sheath_B_R",		// BONE_SHEATH_B_R

	"Primary_fire04",	// BONE_PRIMARY_FIRE1	TR
	"Primary_fire02",	// BONE_PRIMARY_FIRE2	BL
	"Primary_fire1",	// BONE_PRIMARY_FIRE3	TL
	"Primary_fire03",	// BONE_PRIMARY_FIRE4	BR
	
	"Horizontal",		// BONE_HORIZONTAL
	"Vertical",			// BONE_VERTICAL
	
	"AttachPoint_Bot",	// BONE_ATTACHPOINT_BOT
	"Secondary",		// BONE_SECONDARY
	"Base",				// BONE_BASE
};

cchar *CBotAAGun::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"rest",		// ANIMTAP_REST
	"manual",	// ANIMTAP_AIM_SUMMER
};

cchar *CBotAAGun::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"rest",		// ANIMCONTROL_REST
	"manual",	// ANIMCONTROL_AIM_SUMMER
};

CFAnimCombinerConfig::ConfigStack_t CBotAAGun::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_SUMMER,	ANIMCONTROL_AIM_SUMMER,
	CFAnimMixer::TYPE_BLENDER,	ANIMCONTROL_REST,

	CFAnimMixer::TYPE_COUNT,	255
};

//order doesn't matter, just match left to right
CFAnimCombinerConfig::ConfigTap_t CBotAAGun::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_REST,		ANIMCONTROL_REST,		1,
	ANIMTAP_AIM_SUMMER,	ANIMCONTROL_AIM_SUMMER,	1,

	255,				255,					255
};

CFAnimCombiner::AttachList_t CBotAAGun::m_aAnimAttach[] = {
	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMCONTROL_REST,
	BOTANIM_NULL_ANIMSOURCE_ATTACH,	ANIMCONTROL_AIM_SUMMER,

	255,							255
};

const u8 *CBotAAGun::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,
};

const u8 CBotAAGun::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};

const u8 CBotAAGun::m_anTagPointBoneNameIndexArray[] = {
	BONE_HORIZONTAL,
	BONE_VERTICAL,
	BONE_BARREL_B_L,
	BONE_BARREL_B_R,
	255
};

BOOL CBotAAGun::_BuildAnimStackDef( void ) {
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount = 0;
	AnimStackDefInit.nBaseAnimNameCount = ANIM_BASE_COUNT;
	AnimStackDefInit.apszBaseAnimNameTable = NULL;
	AnimStackDefInit.nIdleCount = ANIM_IDLE_COUNT;
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
	AnimStackDefInit.nIdleStackConnectionControlIndex = ANIMCONTROL_REST;
	AnimStackDefInit.nIdleStackConnectionControlInput = 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
}