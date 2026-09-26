//////////////////////////////////////////////////////////////////////////////////////
// botsnarq_data.cpp - mil snarq bot data
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
// 04/19/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "botsnarq.h"
#include "botanim.h"
#include "fmesh.h"

const CBotDef CBotSnarq::m_BotDef = {
	BOTRACE_MIL,					// nRace
	BOTCLASS_SNARQ,					// nClass
	BOTSUBCLASS_BOTSNARQ_NORMAL,	// nSubClass
};


cchar *CBotSnarq::m_apszBoneNameTable[BONE_COUNT] = {
	"SnarqDummy",					// BONE_DUMMY,				
		   
	"AttachPoint_JetBack1",			// BONE_JETBACK1,			
	"AttachPoint_JetBack2",			// BONE_JETBACK2,			
	"AttachPoint_L_JetDown",		// BONE_JETDOWN_L,			
	"AttachPoint_R_JetDown",		// BONE_JETDOWN_R,			
		   
	"AttachPoint_L_Streamer",		// BONE_STREAMER_L,		
	"AttachPoint_R_Streamer",		// BONE_STREAMER_R,		
		   
	"Primary_Fire_L",				// PRIFIRE_L,				
	"Primary_Fire_R",				// PRIFIRE_R,				
};


const u8 CBotSnarq::m_nApproxEyePointBoneNameIndex = BONE_DUMMY;


const u8 CBotSnarq::m_anTagPointBoneNameIndexArray[] = {
	BONE_DUMMY,
	BONE_PRIFIRE_L,
	BONE_PRIFIRE_R,

	255
};


//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

const FGameDataMap_t CBotSnarq::m_aGameDataMap[] = {
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

	"Jump",
	m_aBotInfoVocab_Jump,
	sizeof(m_BotInfo_Jump),
	(void *)&m_BotInfo_Jump,

	"Weapon",
	m_aBotInfoVocab_Weapon,
	sizeof(m_BotInfo_Weapon),
	(void *)&m_BotInfo_Weapon,

	"Snarq",
	m_aBotInfoVocab_Snarq,
	sizeof(m_BotInfo_Snarq),
	(void*)&m_BotInfo_Snarq,

	NULL
};


const FGameData_TableEntry_t CBotSnarq::m_aBotInfoVocab_Snarq[] = {

	FGAMEDATA_VOCAB_F32_BOUNDMIN( F32_DATATABLE_1 ),		//	f32 fMaxXZVel;
	FGAMEDATA_VOCAB_F32_BOUNDMIN( F32_DATATABLE_1 ),		//	f32 fMaxXZAccel;
	FGAMEDATA_VOCAB_F32_BOUNDMIN( F32_DATATABLE_1 ),		//	f32 fMaxYVel;
	FGAMEDATA_VOCAB_F32_BOUNDMIN( F32_DATATABLE_1 ),		//	f32 fMaxYAccel;

	FGAMEDATA_VOCAB_U32_BOUND( F32_DATATABLE_1, F32_DATATABLE_100 ),		//	u32 uMaxTracers
	FGAMEDATA_VOCAB_F32_MIN0,					//	f32 fTracerWidth;		
	FGAMEDATA_VOCAB_F32_MIN0,					//	f32 fTracerLength;	
	FGAMEDATA_VOCAB_F32_MIN0,					// f32 fTracerSpeed;	
	FGAMEDATA_VOCAB_F32_MIN0,					// f32 fTracerTailDist;

	FGAMEDATA_VOCAB_U32_BOUND( F32_DATATABLE_1, F32_DATATABLE_30 ),			// u32	uStreamerDepth;
	FGAMEDATA_VOCAB_F32_UNIT,					// f32 fStreamerAlpha; 
	FGAMEDATA_VOCAB_F32_MIN0,					// f32 fStreamerSize;
	FGAMEDATA_VOCAB_F32_MIN0,					// 	f32 fStreamerSamples;
	FGAMEDATA_VOCAB_TEXDEF,						// FTexDef_t *pStreamerTex;

	//f32 fOOFireRate;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt1,
	F32_DATATABLE_100,

	//f32 fOOFireSoundRate;
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt1,
	F32_DATATABLE_100,

	FGAMEDATA_VOCAB_F32_UNIT,					// f32 fEngineResponseRate;				// how fast the particles/sounds respond to changes in accel
	FGAMEDATA_VOCAB_ARMOR,						//	CDamageProfile *pGunDamageProfile;
	FGAMEDATA_VOCAB_SOUND_GROUP,				// CFSoundGroup *pHoverSound;
	FGAMEDATA_VOCAB_SOUND_GROUP,				// CFSoundGroup *pGunSound;
	FGAMEDATA_VOCAB_PARTICLE,					// FParticle_DefHandle_t hExhaustParticle;
	FGAMEDATA_VOCAB_EXPLODE_GROUP,				// FExplosion_GroupHandle_t hDeathExplosion1;
	FGAMEDATA_VOCAB_EXPLODE_GROUP,				// FExplosion_GroupHandle_t hDeathExplosion2;

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


cchar* CBotSnarq::m_apszStreamerBones[] = {
	m_apszBoneNameTable[BONE_STREAMER_L],
	m_apszBoneNameTable[BONE_STREAMER_R],
	NULL
};


