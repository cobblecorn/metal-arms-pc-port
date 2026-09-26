//////////////////////////////////////////////////////////////////////////////////////
// botpred_data.cpp - Bot data.
//
// Author: Steve Ranck     
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
// 03/26/02 Ranck       Created.
// 12/10/02 Elliott		Took ownership
//////////////////////////////////////////////////////////////////////////////////////


#include "botpred.h"

const CBotDef CBotPred::m_BotDef = {
	BOTRACE_MIL,					// nRace
	BOTCLASS_PREDATOR,				// nClass
	BOTSUBCLASS_PREDATOR_NORMAL,	// nSubClass
};


cchar *CBotPred::m_apszBoneNameTable[BONE_COUNT] = {
	"Predator_Dummy",				// BONE_DUMMY
	"Torso",						// BONE_TORSO,					
	"ArmUpperR2",					// BONE_ARM_UPPER_R2,			
	"ArmLowerR2",					// BONE_ARM_LOWER_R2,			
	"GunR2",						// BONE_GUN_R2,				
	"GunBarrelR2",					// BONE_GUN_BARREL_R2,			
									
	"Primary_Fire03",				// BONE_PRIMARY_FIRE_3,		
									
	"ArmUpperR1",					// BONE_ARM_UPPER_R1,			
	"ArmLowerR1",					// BONE_ARM_LOWER_R1,			
	"GunR1",						// BONE_GUN_R1,				
	"GunBarrelR1",					// BONE_GUN_BARREL_R1,			
									
	"Primary_Fire04",				// BONE_PRIMARY_FIRE_4,		
		
	"ArmUpperL2",					// BONE_ARM_UPPER_L2,			
	"ArmLowerL2",					// BONE_ARM_LOWER_L2,			
	"GunL2",						// BONE_GUN_L2,				
	"GunBarrelL2",					// BONE_GUN_BARREL_L2,			
									
	"Primary_Fire01",				// BONE_PRIMARY_FIRE_1,		
		
	"ArmUpperL1",					// BONE_ARM_UPPER_L1,			
	"ArmLowerL1",					// BONE_ARM_LOWER_L1,			
	"GunL1",						// BONE_GUN_L1,				
	"GunBarrelL1",					// BONE_GUN_BARREL_L1,			
		
	"Primary_Fire02",				// BONE_PRIMARY_FIRE_2,		
		
	"JetPort",						// BONE_JET_PORT,				
	"Spotlite",						// BONE_SPOTLITE,				
	"AttachPoint_Light",			// BONE_SPOTLITE_ATTACH,		
	"Tether_Port",					// BONE_TETHER,				
		
	"AttachPoint_R_Streamer",		// BONE_STREAMER_R_ATTACH,		
	"AttachPoint_L_Streamer",		// BONE_STREAMER_L_ATTACH,		
	"AttachPoint_C_Streamer",		// BONE_STREAMER_C_ATTACH,

	"AttachPoint_JetBack1",			//	BONE_JETBACK1_ATTACH,
	"AttachPoint_JetBack2",			//	BONE_JETBACK2_ATTACH,
	"AttachPoint_JetDown"			//	BONE_JETDOWN_ATTACH,
};


cchar *CBotPred::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = {
	"ARMPIdle001",				//ANIM_STAND
	"ARMPelect01",				//ANIMCONTROL_RC_TETHERED,
	"ARMPpwrdn01",				//ANIMCONTROL_RC_POWER_DOWN,
	"ARMPpwrup01"				//ANIMCONTROL_RC_POWER_UP,
};


cchar *CBotPred::m_apszIdleAnimNameTable[ANIM_IDLE_COUNT] = {
	"ARMPIdle001"
};


cchar *CBotPred::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"Stand",			//ANIMCONTROL_STAND
	"tethered",		//  ANIMCONTROL_RC_TETHERED
	"power_down",	//	ANIMCONTROL_RC_POWER_DOWN,
	"power_up",		//	ANIMCONTROL_RC_POWER_UP,

};


cchar *CBotPred::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"stand",		//	ANIMTAP_STAND,
	"tethered",		//  ANIMCONTROL_RC_TETHERED
	"power_down",	//	ANIMCONTROL_RC_POWER_DOWN,
	"power_up",		//	ANIMCONTROL_RC_POWER_UP,
};


CFAnimCombinerConfig::ConfigStack_t CBotPred::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_TETHERED,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_UP,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_RC_POWER_DOWN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_STAND,
	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CBotPred::m_aAnimCombinerConfigTaps[] = {
	ANIMTAP_STAND,			ANIMCONTROL_STAND,				1,
	ANIMTAP_RC_TETHERED,	ANIMCONTROL_RC_TETHERED,		1,
	ANIMTAP_RC_POWER_UP,	ANIMCONTROL_RC_POWER_UP,		1,
	ANIMTAP_RC_POWER_DOWN,	ANIMCONTROL_RC_POWER_DOWN,		1,
	255,					255,							255
};


CFAnimCombiner::AttachList_t CBotPred::m_aAnimAttach[] = {
	ANIM_STAND,				ANIMTAP_STAND,
	ANIM_RC_TETHERED,		ANIMTAP_RC_TETHERED,
	ANIM_RC_POWER_UP,		ANIMTAP_RC_POWER_UP,
	ANIM_RC_POWER_DOWN,		ANIMTAP_RC_POWER_DOWN,

	255,					255
};

const u8 CBotPred::m_aBoneEnableIndices_FullBody[] = {
	BONE_TORSO,				
	BONE_ARM_UPPER_R2,		
	BONE_ARM_LOWER_R2,		
	BONE_GUN_R2,				
	BONE_GUN_BARREL_R2,		

	BONE_PRIMARY_FIRE_3,		

	BONE_ARM_UPPER_R1,		
	BONE_ARM_LOWER_R1,		
	BONE_GUN_R1,				
	BONE_GUN_BARREL_R1,		

	BONE_PRIMARY_FIRE_4,		

	BONE_ARM_UPPER_L2,		
	BONE_ARM_LOWER_L2,		
	BONE_GUN_L2,				
	BONE_GUN_BARREL_L2,		

	BONE_PRIMARY_FIRE_1,		

	BONE_ARM_UPPER_L1,		
	BONE_ARM_LOWER_L1,		
	BONE_GUN_L1,				
	BONE_GUN_BARREL_L1,		

	BONE_PRIMARY_FIRE_2,		

	BONE_JET_PORT,			
	BONE_SPOTLITE,			
	BONE_SPOTLITE_ATTACH,	
	BONE_TETHER,				

	BONE_STREAMER_R_ATTACH,
	BONE_STREAMER_L_ATTACH,
	BONE_STREAMER_C_ATTACH,
	255
};



const u8 *CBotPred::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_STAND,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_RC_TETHERED,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_RC_POWER_DOWN,
	m_aBoneEnableIndices_FullBody,		//	ANIMTAP_RC_POWER_UP,
};

const u8 *CBotPred::m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT] = {
	m_aBoneEnableIndices_FullBody									// Idle 0
};

const u8 CBotPred::m_anEnableBoneNameIndexTableForSummer[] = {
	255
};


//////////////////////////////////////////////
//BONE MASKS FOR USER ANIMS

const u8 CBotPred::m_aBoneEnableIndices_UserAnim_UpperBody[] = {
	BONE_TORSO,				
	BONE_ARM_UPPER_R2,		
	BONE_ARM_LOWER_R2,		
	BONE_GUN_R2,				
	BONE_GUN_BARREL_R2,		

	BONE_PRIMARY_FIRE_3,		

	BONE_ARM_UPPER_R1,		
	BONE_ARM_LOWER_R1,		
	BONE_GUN_R1,				
	BONE_GUN_BARREL_R1,		

	BONE_PRIMARY_FIRE_4,		

	BONE_ARM_UPPER_L2,		
	BONE_ARM_LOWER_L2,		
	BONE_GUN_L2,				
	BONE_GUN_BARREL_L2,		

	BONE_PRIMARY_FIRE_1,		

	BONE_ARM_UPPER_L1,		
	BONE_ARM_LOWER_L1,		
	BONE_GUN_L1,				
	BONE_GUN_BARREL_L1,		

	BONE_PRIMARY_FIRE_2,		

	BONE_JET_PORT,			
	BONE_SPOTLITE,			
	BONE_SPOTLITE_ATTACH,	
	BONE_TETHER,				

	BONE_STREAMER_R_ATTACH,
	BONE_STREAMER_L_ATTACH,
	BONE_STREAMER_C_ATTACH,
	255
};


const u8 CBotPred::m_aBoneEnableIndices_UserAnim_LowerBody[] = {
	BONE_TORSO,				
	BONE_ARM_UPPER_R2,		
	BONE_ARM_LOWER_R2,		
	BONE_GUN_R2,				
	BONE_GUN_BARREL_R2,		

	BONE_PRIMARY_FIRE_3,		

	BONE_ARM_UPPER_R1,		
	BONE_ARM_LOWER_R1,		
	BONE_GUN_R1,				
	BONE_GUN_BARREL_R1,		

	BONE_PRIMARY_FIRE_4,		

	BONE_ARM_UPPER_L2,		
	BONE_ARM_LOWER_L2,		
	BONE_GUN_L2,				
	BONE_GUN_BARREL_L2,		

	BONE_PRIMARY_FIRE_1,		

	BONE_ARM_UPPER_L1,		
	BONE_ARM_LOWER_L1,		
	BONE_GUN_L1,				
	BONE_GUN_BARREL_L1,		

	BONE_PRIMARY_FIRE_2,		

	BONE_JET_PORT,			
	BONE_SPOTLITE,			
	BONE_SPOTLITE_ATTACH,	
	BONE_TETHER,				

	BONE_STREAMER_R_ATTACH,
	BONE_STREAMER_L_ATTACH,
	BONE_STREAMER_C_ATTACH,
	255
};


const u8 CBotPred::m_aBoneEnableIndices_UserAnim_UpperTorso[] = {
	BONE_TORSO,				
	BONE_ARM_UPPER_R2,		
	BONE_ARM_LOWER_R2,		
	BONE_GUN_R2,				
	BONE_GUN_BARREL_R2,		

	BONE_PRIMARY_FIRE_3,		

	BONE_ARM_UPPER_R1,		
	BONE_ARM_LOWER_R1,		
	BONE_GUN_R1,				
	BONE_GUN_BARREL_R1,		

	BONE_PRIMARY_FIRE_4,		

	BONE_ARM_UPPER_L2,		
	BONE_ARM_LOWER_L2,		
	BONE_GUN_L2,				
	BONE_GUN_BARREL_L2,		

	BONE_PRIMARY_FIRE_1,		

	BONE_ARM_UPPER_L1,		
	BONE_ARM_LOWER_L1,		
	BONE_GUN_L1,				
	BONE_GUN_BARREL_L1,		

	BONE_PRIMARY_FIRE_2,		

	BONE_JET_PORT,			
	BONE_SPOTLITE,			
	BONE_SPOTLITE_ATTACH,	
	BONE_TETHER,				

	BONE_STREAMER_R_ATTACH,
	BONE_STREAMER_L_ATTACH,
	BONE_STREAMER_C_ATTACH,
	255
};


const u8 CBotPred::m_aBoneEnableIndices_UserAnim_LowerTorso[] = {
	BONE_TORSO,				
	BONE_ARM_UPPER_R2,		
	BONE_ARM_LOWER_R2,		
	BONE_GUN_R2,				
	BONE_GUN_BARREL_R2,		

	BONE_PRIMARY_FIRE_3,		

	BONE_ARM_UPPER_R1,		
	BONE_ARM_LOWER_R1,		
	BONE_GUN_R1,				
	BONE_GUN_BARREL_R1,		

	BONE_PRIMARY_FIRE_4,		

	BONE_ARM_UPPER_L2,		
	BONE_ARM_LOWER_L2,		
	BONE_GUN_L2,				
	BONE_GUN_BARREL_L2,		

	BONE_PRIMARY_FIRE_1,		

	BONE_ARM_UPPER_L1,		
	BONE_ARM_LOWER_L1,		
	BONE_GUN_L1,				
	BONE_GUN_BARREL_L1,		

	BONE_PRIMARY_FIRE_2,		

	BONE_JET_PORT,			
	BONE_SPOTLITE,			
	BONE_SPOTLITE_ATTACH,	
	BONE_TETHER,				

	BONE_STREAMER_R_ATTACH,
	BONE_STREAMER_L_ATTACH,
	BONE_STREAMER_C_ATTACH,
	255
};


const u8 CBotPred::m_aBoneEnableIndices_UserAnim_LeftArm[] = {
	BONE_ARM_UPPER_L2,		
	BONE_ARM_LOWER_L2,		
	BONE_GUN_L2,				
	BONE_GUN_BARREL_L2,		

	BONE_PRIMARY_FIRE_1,		

	BONE_ARM_UPPER_L1,		
	BONE_ARM_LOWER_L1,		
	BONE_GUN_L1,				
	BONE_GUN_BARREL_L1,

	BONE_PRIMARY_FIRE_2,
	255		// Bone-mask lists end with 255; without it the reader ran into the next table
};


const u8 CBotPred::m_aBoneEnableIndices_UserAnim_RightArm[] = {
	BONE_ARM_UPPER_L2,		
	BONE_ARM_LOWER_L2,		
	BONE_GUN_L2,				
	BONE_GUN_BARREL_L2,		

	BONE_PRIMARY_FIRE_1,		

	BONE_ARM_UPPER_L1,		
	BONE_ARM_LOWER_L1,		
	BONE_GUN_L1,				
	BONE_GUN_BARREL_L1,		

	BONE_PRIMARY_FIRE_2,		

	255
};


const u8 CBotPred::m_aBoneEnableIndices_UserAnim_Head[] = {
	BONE_SPOTLITE,			
	BONE_SPOTLITE_ATTACH,	
	255
};


// These must be in FIRING order... that is the order that is passed into CWeaponQuadLaser in ClassHierarchyBuild
const u32 CBotPred::m_auArmLimbTable[LIMB_TYPE_COUNT] = {
	LIMB_TYPE_TOP_LEFT_ARM,
	LIMB_TYPE_TOP_RIGHT_ARM,
	LIMB_TYPE_BOTTOM_LEFT_ARM,
	LIMB_TYPE_BOTTOM_RIGHT_ARM
};



const u8 CBotPred::m_nApproxEyePointBoneNameIndex = BONE_SPOTLITE;

const u8 CBotPred::m_anTagPointBoneNameIndexArray[] = {
	BONE_TORSO,
	BONE_SPOTLITE,
	BONE_JETBACK1_ATTACH,
	BONE_JETBACK2_ATTACH,
	BONE_JET_PORT,
	BONE_ARM_LOWER_R1,
	BONE_ARM_LOWER_R2,
	BONE_ARM_LOWER_L1,
	BONE_ARM_LOWER_L2,

	255
};

cchar* CBotPred::m_apszStreamerBones[] = {
	"AttachPoint_R_Streamer",		// BONE_STREAMER_R_ATTACH,		
	"AttachPoint_L_Streamer",		// BONE_STREAMER_L_ATTACH,		
	"AttachPoint_C_Streamer",		// BONE_STREAMER_C_ATTACH,		
	NULL
};

BOOL CBotPred::_BuildAnimStackDef( void ) {
	// this should be easy...
		
	CBotAnimStackDef::Init_t AnimStackDefInit;

	AnimStackDefInit.nUserCount					= CBotAnimStackDef::ANIM_USER_COUNT;;

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
	AnimStackDefInit.pnEnableBoneNameIndexTableForSummer		= m_anEnableBoneNameIndexTableForSummer;
	AnimStackDefInit.pBaseAnimConfigNet							= NULL;
	AnimStackDefInit.pBaseAnimConfigStack						= m_aAnimCombinerConfigStack;
	AnimStackDefInit.pBaseAnimConfigTap							= m_aAnimCombinerConfigTaps;
	AnimStackDefInit.pBaseAnimAttachList						= m_aAnimAttach;
	AnimStackDefInit.nIdleStackConnectionControlIndex			= ANIMCONTROL_STAND;
	AnimStackDefInit.nIdleStackConnectionControlInput			= 0;

	return m_AnimStackDef.Create( &AnimStackDefInit );
	
}



//--------------------------------------------------------------------------------------------------------
// Game Data Vocab:
//--------------------------------------------------------------------------------------------------------

const FGameData_TableEntry_t CBotPred::m_aBotInfoVocab_Pred[] = {

		//cchar*	pszSpotlightMesh;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//f32		fMinTargetDist;
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1000,

		//cchar*	pszStreamerTex;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//cchar*	pszParticleJetBack;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//cchar*	pszParticleJetDown;
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionBig
		FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionSecondary
		FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionTiny
		FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionFinal

		FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupHover
		FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupWind

		FGAMEDATA_VOCAB_PARTICLE,		// hParticleBurnSmoke


		// End of table:
		FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotPred::m_aGameDataMap[] = {
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"MountAim",
	m_aBotInfoVocab_MountAim,
	sizeof(m_BotInfo_MountAim),
	(void *)&m_BotInfo_MountAim,

	"Predator",
	m_aBotInfoVocab_Pred,
	sizeof(m_BotInfo_Pred),
	(void *)&m_BotInfo_Pred,

	NULL
};
