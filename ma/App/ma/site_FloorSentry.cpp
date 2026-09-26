//////////////////////////////////////////////////////////////////////////////////////
// site_FloorSentry.cpp
//
// Author: Michael Scholz
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
// 11/26/02 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "MeshTypes.h"
#include "site_FloorSentry.h"
#include "weapon.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "FCheckPoint.h"
#include "potmark.h"
#include "explosion.h"
#include "hud2.h"
#include "player.h"
#include "botpart.h"
#include "AI\AIEnviro.h"
#include "MultiplayerMgr.h"
//#include "miscdraw.h" 

static cchar _FLOORSENTRYINFO_FILENAME[] = 	"sw_floor";
static cchar _apszSentryMeshFilename[] = "GRMSsentry1";
static cchar _apszSentryMeshDeadFilename[] = "GRMSsent_D1";
static cchar _apszSentryBarrelFilename[] = "GRMSbarrel1";
static cchar _apszSearchLightMeshFilename[] = "GRMGlite_01";
static u32 _uSearchLightMatID			= 38;
static u32 _uPanelMatID					= 5;
static s32 _nRED_LIGHT_ID = 1;
static s32 _nYELLOW_LIGHT_ID = 2;

CFloorSentry::_FloorSpewProps_t		CFloorSentry::m_aSpewProps[2];
CFloorSentry::_FloorRocketProps_t	CFloorSentry::m_aRocketProps[2];

SmokeTrailAttrib_t			CFloorSentry::m_aSmokeTrailAttrib;
CDamageProfile *			CFloorSentry::m_aSpewDamageProfile[2];
CDamageProfile *			CFloorSentry::m_aRocketDamageProfile[2];
CBotPartPool*				CFloorSentry::m_pPartPool;

static cchar _BOTPART_FILENAME[]			= "bp_sentry";
static s32   _PART_INSTANCE_COUNT_PER_TYPE	= 1;
static s32	 _LIMB_TYPE_COUNT				= 1;


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CFloorSentry::m_aSpewPropVocab[] = 
{
		// fClipAmmoMax:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_Neg1,
		F32_DATATABLE_65534,

		// fReserveAmmoMax:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_Neg1,
		F32_DATATABLE_65534,

		// fRoundsPerSec:
		// fOORoundsPerSec:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
		sizeof( f32 ) * 2,
		F32_DATATABLE_Pt001,
		F32_DATATABLE_1000,

		// fMaxLiveRange:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_1,
		F32_DATATABLE_100000,

		// fDistFromWeaponOrigToMuzzle:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fBestShotSpreadFactor:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fWorstShotSpreadFactor:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fShotSpreadSpeed:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fShotSpreadRecoverySpeed:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fMuzzleWidth_Z:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fMuzzleHeight_Z:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fMuzzleAlpha_Z:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fMuzzleScale_XY:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fMuzzleAlpha_XY:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fMuzzleScale_Mesh:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fMuzzleRot_Mesh:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		// fMuzzleScale_Impact:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fMuzzleAlpha_Impact:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fMuzzleOffset_Impact:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		// fBulletDamage:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fExplosionDamage:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fFlameDamage:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// pszDamageProfile:
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		// fSoundRadius:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10000000,

		// fCameraShakeIntensity:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		// fCameraShakeDuration:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		//hBulletDecal
		FGAMEDATA_VOCAB_DECAL_DEF,

		// End of table:
		FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CFloorSentry::m_aRocketPropVocab[] = {
	// apszMeshName[_MESH_WEAPON]:
	FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		// fClipAmmoMax:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_Neg1,
		F32_DATATABLE_65534,

		// fReserveAmmoMax:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_Neg1,
		F32_DATATABLE_65534,

		// fRoundsPerSec:
		// fOORoundsPerSec:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
		sizeof( f32 ) * 2,
		F32_DATATABLE_Pt001,
		F32_DATATABLE_1000,

		// fShotSpreadFactor:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fRocketsInPoolCount:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_1,
		F32_DATATABLE_1000,

		// fMaxLiveRange:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_1,
		F32_DATATABLE_100000,

		// fRocketCullDist:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fRocketSpeed:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_1,
		F32_DATATABLE_100000,

		// fDistFromWeaponOrigToMuzzle:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_100000,

		// fBulletDamage
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fExplosionDamage
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// fFlameDamage
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_1,

		// pszDamageProfile:
		FGAMEDATA_VAR_TYPE_STRING|
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
		sizeof( char * ),
		F32_DATATABLE_0,
		F32_DATATABLE_0,

		// fSoundRadius:
		FGAMEDATA_VAR_TYPE_FLOAT|
		FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
		sizeof( f32 ),
		F32_DATATABLE_0,
		F32_DATATABLE_10000000,

		// FExplosion_GroupHandle_t hExplosionGroup
		FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroup

		// End of table:
		FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CFloorSentry::m_aGameDataMap[] = {
		"GunPropsMil",
		m_aSpewPropVocab,
		sizeof(_FloorSpewProps_t),
		(void *)&CFloorSentry::m_aSpewProps[0],

		"RocketPropsMil",
		m_aRocketPropVocab,
		sizeof(_FloorRocketProps_t),
		(void *)&CFloorSentry::m_aRocketProps[0],

		"GunPropsPossessed",
		m_aSpewPropVocab,
		sizeof(_FloorSpewProps_t),
		(void *)&CFloorSentry::m_aSpewProps[1],

		"RocketPropsPossessed",
		m_aRocketPropVocab,
		sizeof(_FloorRocketProps_t),
		(void *)&CFloorSentry::m_aRocketProps[1],
		NULL
};

cchar *CFloorSentry::m_apszBoneNameList[BONE_COUNT] = 
{
	"horizontal",
		"vertical",
		"barrel_t_r", 
		"barrel_b_r", 
		"barrel_t_l", 
		"barrel_b_l",	
		"AttachPoint_Bot",	//	BONE_ATTACH_POINT,
		"primary_fire1",	//	BONE_PRIMARY_FIRE1,	
		"primary_fire2",	//	BONE_PRIMARY_FIRE2,	
		"primary_fire3",	//	BONE_PRIMARY_FIRE3,	
		"primary_fire4",	//	BONE_PRIMARY_FIRE4,	
		"AttachPoint_Light",
};

const u8 CFloorSentry::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] = 
{
	BONE_HORIZONTAL,
	BONE_VERTICAL,
	BONE_BARREL_B_L,
	BONE_BARREL_B_R,
};

cchar *CFloorSentry::m_apszBoneNameList2[2] = 
{
	"horizontal", 
	"vertical"
};

cchar *CFloorSentry::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"rest",		//	ANIMCONTROL_REST,
	"manual",   //  ANIMCONTROL_AIM_SUMMER
};


cchar *CFloorSentry::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"rest",		//	ANIMTAP_REST,
	"manual",   //  ANIMTAP_AIM_SUMMER
};

CFAnimCombinerConfig::ConfigStack_t CFloorSentry::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_REST,
	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CFloorSentry::m_aAnimCombinerConfigTaps[] = {
	ANIMCONTROL_REST,			ANIMTAP_REST,				1,
	ANIMCONTROL_AIM_SUMMER,		ANIMTAP_AIM_SUMMER,			1,
	255,						255,						255
};


CFAnimCombiner::AttachList_t CFloorSentry::m_aAnimAttach[] = {
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_REST,
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_AIM_SUMMER,
	255,									255
};

const u8 *CFloorSentry::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,
};

const u8 CFloorSentry::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};

CBotAnimStackDef CFloorSentry::m_DeadAnimStackDef;
CBotAnimStackDef CFloorSentry::m_AliveAnimStackDef;

CFloorSentry::~CFloorSentry()
{
	// pBoneMatrix
}

void CFloorSentry::Destroy(void)
{
	_KillAudioEmitters();
	m_DeadAnim.Destroy();
	m_pOwnerBot->m_Anim.Destroy();
	m_AliveMeshRestAnimSource.Destroy();
	m_DeadMeshRestAnimSource.Destroy();
	m_AliveAnimManMtxAim.Destroy();
	m_DeadAnimManMtxAim.Destroy();
	m_DummyRecruiter.Destroy();
}

BOOL CFloorSentry::InitSystem( void )
{
	if (CFloorSentry::m_AliveAnimStackDef.IsCreated()==FALSE)
	{
		// Build our animation stack and load animations...
		CBotAnimStackDef::Init_t AnimStackDefInit;
		AnimStackDefInit.nUserCount					= 0; // (my guns don't have user overrides) CBotAnimStackDef::ANIM_USER_COUNT;
		AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
		AnimStackDefInit.apszBaseAnimNameTable		= NULL; // m_apszBaseAnimNameTable;
		AnimStackDefInit.nIdleCount					= ANIM_IDLE_COUNT;
		AnimStackDefInit.apszIdleAnimNameTable		= NULL; // m_apszIdleAnimNameTable;
		AnimStackDefInit.nBaseControlCount			= ANIMCONTROL_BASE_COUNT;
		AnimStackDefInit.apszBaseControlNameTable	= m_apszBaseControlNameTable;
		AnimStackDefInit.nBaseTapCount				= ANIMTAP_BASE_COUNT;
		AnimStackDefInit.apszBaseTapNameTable		= m_apszBaseTapNameTable;
		AnimStackDefInit.nBoneCount					= BONE_COUNT;
		AnimStackDefInit.apszBoneNameTable			= m_apszBoneNameList;

		AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap	= m_apnEnableBoneNameIndexTableForEachBaseTap;
		AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap	= NULL;// ?? m_apnEnableBoneNameIndexTableForEachIdleTap;
		AnimStackDefInit.pnEnableBoneNameIndexTableForSummer		= m_anEnableBoneNameIndexTableForSummer_Normal;

		AnimStackDefInit.pBaseAnimConfigNet					= NULL;
		AnimStackDefInit.pBaseAnimConfigStack				= m_aAnimCombinerConfigStack;
		AnimStackDefInit.pBaseAnimConfigTap					= m_aAnimCombinerConfigTaps;
		AnimStackDefInit.pBaseAnimAttachList				= m_aAnimAttach;
		AnimStackDefInit.nIdleStackConnectionControlIndex	= ANIMCONTROL_REST;
		AnimStackDefInit.nIdleStackConnectionControlInput	= 0;
		if( !m_AliveAnimStackDef.Create( &AnimStackDefInit ) ) 
		{
			goto _ExitWithError;
		}
	}
	if (CFloorSentry::m_DeadAnimStackDef.IsCreated()==FALSE)
	{
		// Build our animation stack and load animations...
		CBotAnimStackDef::Init_t AnimStackDefInit;
		AnimStackDefInit.nUserCount					= 0; // (my guns don't have user overrides) CBotAnimStackDef::ANIM_USER_COUNT;
		AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
		AnimStackDefInit.apszBaseAnimNameTable		= NULL; // m_apszBaseAnimNameTable;
		AnimStackDefInit.nIdleCount					= ANIM_IDLE_COUNT;
		AnimStackDefInit.apszIdleAnimNameTable		= NULL; // m_apszIdleAnimNameTable;
		AnimStackDefInit.nBaseControlCount			= ANIMCONTROL_BASE_COUNT;
		AnimStackDefInit.apszBaseControlNameTable	= m_apszBaseControlNameTable;
		AnimStackDefInit.nBaseTapCount				= ANIMTAP_BASE_COUNT;
		AnimStackDefInit.apszBaseTapNameTable		= m_apszBaseTapNameTable;
		AnimStackDefInit.nBoneCount					= 2;
		AnimStackDefInit.apszBoneNameTable			= m_apszBoneNameList2;

		AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap	= m_apnEnableBoneNameIndexTableForEachBaseTap;
		AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap	= NULL;// ?? m_apnEnableBoneNameIndexTableForEachIdleTap;
		AnimStackDefInit.pnEnableBoneNameIndexTableForSummer		= m_anEnableBoneNameIndexTableForSummer_Normal;

		AnimStackDefInit.pBaseAnimConfigNet					= NULL;
		AnimStackDefInit.pBaseAnimConfigStack				= m_aAnimCombinerConfigStack;
		AnimStackDefInit.pBaseAnimConfigTap					= m_aAnimCombinerConfigTaps;
		AnimStackDefInit.pBaseAnimAttachList				= m_aAnimAttach;
		AnimStackDefInit.nIdleStackConnectionControlIndex	= ANIMCONTROL_REST;
		AnimStackDefInit.nIdleStackConnectionControlInput	= 0;
		if( !CFloorSentry::m_DeadAnimStackDef.Create( &AnimStackDefInit ) ) 
		{
			goto _ExitWithError;
		}
	}

	if( !fgamedata_ReadFileUsingMap( m_aGameDataMap, _FLOORSENTRYINFO_FILENAME ) ) 
	{
		DEVPRINTF( "CFloorSentryRocket::Create(): Could not create .\n" );
		goto _ExitWithError;
	}
	
	m_aSpewDamageProfile[0]	= CDamage::FindDamageProfile( m_aSpewProps[0].pszDamageProfile );
	m_aRocketDamageProfile[0] = CDamage::FindDamageProfile( m_aRocketProps[0].pszDamageProfile );
	m_aSpewDamageProfile[1]	= CDamage::FindDamageProfile( m_aSpewProps[1].pszDamageProfile );
	m_aRocketDamageProfile[1] = CDamage::FindDamageProfile( m_aRocketProps[1].pszDamageProfile );

	_SetSmokeTrailAttributes();

	return TRUE;

_ExitWithError:
	return FALSE;
}

void CFloorSentry::UninitSystem( void )
{
	m_AliveAnimStackDef.Destroy();
	m_DeadAnimStackDef.Destroy();
}

void CFloorSentry::_SetSmokeTrailAttributes( void ) 
{
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRocket::_RL1_SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	m_aSmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_aSmokeTrailAttrib.pTexDef = pTexDef;

	m_aSmokeTrailAttrib.fScaleMin_WS = 0.5f;
	m_aSmokeTrailAttrib.fScaleMax_WS = 0.6f;
	m_aSmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_aSmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_aSmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_aSmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_aSmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib.fDistBetweenPuffs_WS = 0.4f;
	m_aSmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_aSmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_aSmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_aSmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_aSmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_aSmokeTrailAttrib.fUnitOpaqueMin_WS = 0.5f;
	m_aSmokeTrailAttrib.fUnitOpaqueMax_WS = 0.7f;
	m_aSmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_aSmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_aSmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_aSmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_aSmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_aSmokeTrailAttrib.EndColorRGB.White();
	m_aSmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_aSmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_aSmokeTrailAttrib.fEndColorUnitIntensityMin = 0.2f;
	m_aSmokeTrailAttrib.fEndColorUnitIntensityMax = 0.6f;

	m_aSmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_aSmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_aSmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_aSmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
}


BOOL CFloorSentry::Create( CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 uUserTypeBits )
{
	m_pOwnerBot = (CBot*)pUser;
	m_pData = pData;

	FMeshInit_t MeshInit;
	
	m_fUnitTimeUntilRocketReady = 0.0f;
	m_pSpewProps=&m_aSpewProps[0];
	m_pRocketProps=&m_aRocketProps[0];
	m_pSpewDamageProfile=m_aSpewDamageProfile[0];
	m_pRocketDamageProfile=m_aRocketDamageProfile[0];
	
	// actions are how we take over the Bot
	m_pOwnerBot->SetActionable( TRUE );

	cchar * pszMeshFilename = NULL;
	// Load mesh resource...
	pszMeshFilename = _apszSentryMeshFilename;
	FMesh_t		*pAliveMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pszMeshFilename);

	if( pAliveMesh == NULL )
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		return FALSE;
	}

	// Create world mesh...
	m_pAliveMesh = &m_AliveMesh;

	// Init the world mesh...
	MeshInit.pMesh		= pAliveMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( m_pData->m_MtxPosition_WS );
	m_pAliveMesh->Init( &MeshInit );

	m_pAliveMesh->m_nUser = MESHTYPES_ENTITY;
	m_pAliveMesh->m_pUser = pUser;
	m_pAliveMesh->SetUserTypeBits( uUserTypeBits );
	m_pAliveMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pAliveMesh->SetCollisionFlag( TRUE );
	m_pAliveMesh->SetLineOfSightFlag( TRUE );

	m_pAliveMesh->UpdateTracker();
	m_pAliveMesh->RemoveFromWorld();

	FMesh_t		*pDeadMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszSentryMeshDeadFilename );
	if( pDeadMesh == NULL )
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyBuild(): Could not find mesh '%s'\n", pszMeshFilename );
		return FALSE;
	}

	// Create world mesh...
	m_pDeadMesh = &m_DeadMesh;

	// Init the dead world mesh...
	MeshInit.pMesh		= pDeadMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( m_pData->m_MtxPosition_WS );
	m_pDeadMesh->Init( &MeshInit );

	m_pDeadMesh->m_nUser = MESHTYPES_ENTITY;
	m_pDeadMesh->m_pUser = pUser;
	m_pDeadMesh->SetUserTypeBits( uUserTypeBits );
	m_pDeadMesh->m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_pDeadMesh->SetCollisionFlag( TRUE );
	m_pDeadMesh->SetLineOfSightFlag( TRUE );

	m_pDeadMesh->UpdateTracker();
	m_pDeadMesh->RemoveFromWorld();

	m_pOwnerBot->m_pWorldMesh = &m_AliveMesh;

	if( !m_pOwnerBot->m_Anim.Create( &m_AliveAnimStackDef, m_pAliveMesh ) ) 
	{
		return FALSE;
	}

	if( !m_DeadAnim.Create( &m_DeadAnimStackDef, m_pDeadMesh ) ) 
	{
		return FALSE;
	}

	if( !m_AliveMeshRestAnimSource.Create(m_AliveMesh.m_pMesh))
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyCreate(): Could not create m_MeshRestAnimSource.\n" );
		return FALSE;
	}

	if( !m_DeadMeshRestAnimSource.Create(m_DeadMesh.m_pMesh))
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyCreate(): Could not create m_MeshRestAnimSource.\n" );
		return FALSE;
	}

	m_pOwnerBot->m_Anim.BaseAnim_Attach(ANIMTAP_REST, &m_AliveMeshRestAnimSource);
	m_DeadAnim.BaseAnim_Attach(ANIMTAP_REST, &m_DeadMeshRestAnimSource);

	if( !m_AliveAnimManMtxAim.Create( m_AliveAnimStackDef.m_nBoneCount, m_AliveAnimStackDef.m_apszBoneNameTable ) ) 
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyCreate(): Could not create m_AliveAnimManFrameAim.\n" );
		return FALSE;
	}

	if( !m_DeadAnimManMtxAim.Create( m_DeadAnimStackDef.m_nBoneCount, m_DeadAnimStackDef.m_apszBoneNameTable) ) 
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyCreate(): Could not create m_AliveAnimManFrameAim.\n" );
		return FALSE;
	}

	m_pOwnerBot->m_Anim.BaseAnim_Attach(ANIMTAP_AIM_SUMMER, &m_AliveAnimManMtxAim);
	m_DeadAnim.BaseAnim_Attach(ANIMTAP_AIM_SUMMER, &m_DeadAnimManMtxAim);

	m_AliveAnimManMtxAim.Reset();	
	m_DeadAnimManMtxAim.Reset();	

	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_REST, 1.0f );
	m_DeadAnim.BaseAnim_SetControlValue(  ANIMCONTROL_REST, 1.0f );

	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_AIM_SUMMER, 1.0f );
	m_DeadAnim.BaseAnim_SetControlValue(  ANIMCONTROL_AIM_SUMMER, 1.0f );

	m_MtxRecoil = CFMtx43A::m_IdentityMtx;

	m_bFirePrimaryThisFrame = FALSE;

	m_nSpewClipAmmo = CWeapon::INFINITE_AMMO;
	m_nSpewReserveAmmo = CWeapon::INFINITE_AMMO;

	m_fSpewUnitShotSpread  = 0.0f;
	m_fSpewSecondsUntilNextTracer  = 0.0f;
	m_fSpewSecondsCountdownTimer  = 0.0f;
	m_fRocketSecondsCountdownTimer = 0.0f;

	m_fSecsUntilNextSound = 0.0f;

	m_fRecoil0 = 0.0f;
	m_bFireTopBarrels = TRUE;
	m_ePreviousState = m_pData->m_eSiteWeaponState;

	m_fRecoilDistanceNormal = 1.0f;
	m_fRecoilDistanceCustom = 1.0f;
	m_fRecoilReturnRate = 2 * m_fRecoilDistanceNormal * m_pSpewProps->fRoundsPerSec;
	
	s32 i;
	
	// Find tag points...
	for( i=0; i<TAG_POINT_COUNT; i++ ) 
	{
		s32 nBoneIndex = m_AliveMesh.FindBone( m_apszBoneNameList[ m_anTagPointBoneNameIndexArray[i] ] );
		if( nBoneIndex < 0 ) 
		{
			DEVPRINTF( "CBotSiteWeapons::ClassHierarchyBuild(): Could not locate TagPoint bone '%s'.\n", m_apszBoneNameList[ m_anTagPointBoneNameIndexArray[i] ] );
		} else 
		{
			m_apTagPoint_WS[i] = &m_AliveMesh.GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		}
	}

//	m_fCountPlayerRockets = m_aRocketProps[1].fClipAmmoMax;
	cchar* pszMuzzleNames[] = 
	{
		"primary_fire1",
		"primary_fire4",
		"primary_fire2",
		"primary_fire3",
		"Secondary_Fire",
	};

	s32 nFireBone;
	for (s32 nMuzzle = 0; nMuzzle < 5; nMuzzle++)
	{
		nFireBone = m_AliveMesh.FindBone(pszMuzzleNames[nMuzzle]);
		FASSERT(nFireBone >=0);
		m_apMuzzlePts[nMuzzle] = m_AliveMesh.GetBoneMtxPalette()[nFireBone];
	}
	m_pData->m_pMtxAimOrientation_WS = m_apMuzzlePts[0];

	// Init the custom barrels ...
	FMesh_t		*pCustomBarrel = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszSentryBarrelFilename );
	if( pCustomBarrel == NULL )
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyBuild(): Could not find mesh '%s'\n", _apszSentryBarrelFilename );
		return FALSE;
	}
	for (s32 nMuzzle = 0; nMuzzle < 4; nMuzzle++)
	{
		MeshInit.pMesh		= pCustomBarrel;
		MeshInit.nFlags		= 0;
		MeshInit.fCullDist  = FMATH_MAX_FLOAT;
		MeshInit.Mtx.Set( *m_apMuzzlePts[nMuzzle] );
		
		m_apCustomBarrels[nMuzzle].Init( &MeshInit );
		m_apCustomBarrels[nMuzzle].m_nUser = MESHTYPES_ENTITY;
		m_apCustomBarrels[nMuzzle].m_pUser = pUser;
		m_apCustomBarrels[nMuzzle].SetUserTypeBits( uUserTypeBits );
		m_apCustomBarrels[nMuzzle].m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
		m_apCustomBarrels[nMuzzle].SetCollisionFlag( FALSE );
		m_apCustomBarrels[nMuzzle].SetLineOfSightFlag( FALSE );
		m_apCustomBarrels[nMuzzle].UpdateTracker();
		m_apCustomBarrels[nMuzzle].RemoveFromWorld();
	}

	for(s32 i=0; i<MAX_SIMULTANEOUS_FIRE_SOUNDS; ++i ) 
	{
		m_apAudioEmitterFire[i] = NULL;
	}
	m_nAudioEmitterIndex = 0;

	// find the search light matrix
//	s32 nSeachLightBone = m_AliveMesh.FindBone("AttachPoint_Light");
//	FASSERT(nSeachLightBone>=0);
//	m_pSearchLightMtx = m_AliveMesh.GetBoneMtxPalette()[nSeachLightBone];
	
	m_eLightPage = RED;
	
	CFWorldAttachedLight* pRedWorldLight = m_AliveMesh.GetAttachedLightByID(_nRED_LIGHT_ID);
	CFWorldAttachedLight* pYellowWorldLight = m_AliveMesh.GetAttachedLightByID(_nYELLOW_LIGHT_ID);
	FASSERT(pRedWorldLight);
	FASSERT(pYellowWorldLight);
	pRedWorldLight->UpdateTracker();
	pYellowWorldLight->UpdateTracker();
	pYellowWorldLight->m_Light.m_Motif.ComputeColor(&m_vYellowLightColor);
	pRedWorldLight->m_Light.m_Motif.ComputeColor(&m_vRedLightColor);

	pRedWorldLight->m_Light.Enable(FALSE);
	pYellowWorldLight->m_Light.Enable(FALSE);
	m_pWorldLight =	pRedWorldLight;
	m_pWorldLight->m_Light.Enable(TRUE && m_pData->m_bEnableLight);
	m_pWorldLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION );
	m_pWorldLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_POSITION );
	if (m_pData->m_fLightRadiusOverride >= 0.0f)
	{
		m_pWorldLight->m_Light.m_spInfluence_WS.m_fRadius = m_pData->m_fLightRadiusOverride;
	}
	// zero the saved parameters as not helpful
//	m_pWorldLight->ResetLightToIdentity();
	
	m_fCurrentLightAlpha=1.0f;

	m_fDeathRandomAngularVelocityRange	= FMATH_PI;		// the R.A.V will be between 0 and here (ever want neg??)
	m_fDeathRandomAngularVelocity		= -fmath_RandomFloatRange(0.0f,m_fDeathRandomAngularVelocityRange);
	m_fDeathVelocityDamp				= .75f;						// how much velocity is left after hitting an extrema
	m_fDeathAcceleration				= 2.2f;					    // downward pull from gravity

	m_fPowerDownVelocity					= 0;
	m_fPowerDownVelocityDamp				= .50f;						// how much velocity is left after hitting an extrema
	m_fPowerDownAcceleration				= 4.0f;					    // downward pull from gravity

	m_fOOSparksPerSecond				= 1.0f/2.0f;
	m_fSparkCountDown					= 0.0f;
	
	m_bPossessed = FALSE;
	fforce_NullHandle( &m_hForce );

	// check for light's texture handle
	hLightLayerHandle = m_AliveMesh.GetTexLayerHandle(_uSearchLightMatID);
	FASSERT(m_AliveMesh.GetTexFlip(hLightLayerHandle));
	m_AliveMesh.TexFlip_AnimateFlip(hLightLayerHandle, FALSE);

	hPanelLayerHandle = m_AliveMesh.GetTexLayerHandle(_uPanelMatID);
	FASSERT( hPanelLayerHandle != FMESH_TEXLAYERHANDLE_INVALID);

	// set up part mgr...
	if( !m_pOwnerBot->m_pPartMgr->Create( m_pOwnerBot, &m_pPartPool, _BOTPART_FILENAME, _PART_INSTANCE_COUNT_PER_TYPE, _LIMB_TYPE_COUNT ) ) 
	{
		DEVPRINTF("problem aprsing file %s", _BOTPART_FILENAME);
	}

	m_DummyRecruiter.Create();

	return TRUE;
	// Error:
}

void CFloorSentry::AddToWorld(void )
{
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
	{
		m_DeadMesh.UpdateTracker(TRUE);
	}
	else
	{
		m_AliveMesh.UpdateTracker(TRUE);
	}
	m_DummyRecruiter.AddToWorld();
	m_uAIVisibleSoundHandle = 0;
}

void CFloorSentry::RemoveFromWorld(void )
{
	m_AliveMesh.RemoveFromWorld();
	m_DeadMesh.RemoveFromWorld();
	m_DummyRecruiter.RemoveFromWorld();
}

CFMtx43A* CFloorSentry::AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName )
{
	return 0;
}

u32	CFloorSentry::TriggerWorkPrimary( f32 fUnitTriggerVal1) 
{
	m_bFirePrimaryThisFrame = FALSE;

	if( fUnitTriggerVal1 < 0.01f ) 
	{
		// Trigger not down...

		//		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

		goto _ExitWithoutTriggerPressed;
	}

	if( m_nSpewClipAmmo == 0 ) 
	{
		// No rounds in clip...
		if( m_nSpewReserveAmmo == 0 ) 
		{
			// Completely out of ammo. Make empty click sound...

			//			if( !(m_nWeaponFlags & WEAPONFLAG_MADE_EMPTY_CLICK_SOUND) ) 
			//			{
			//				FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

			//				if( IsOwnedByPlayer()) {
			//					fsndfx_Play2D( m_pUserProps->hEmptyClickSound );
			//				} else {
			//					fsndfx_Play3D( m_pUserProps->hEmptyClickSound, &m_MtxToWorld.m_vPos, m_pUserProps->fSoundRadius );
			//				}
			//			}
		}
		goto _ExitWithoutTriggerPressed;
	}
	/*
	if( CurrentState() != STATE_DEPLOYED ) 
	{
	// Not in a state to fire...
	goto _ExitWithoutTriggerPressed;
	}
	*/
	// if( !(m_nWeaponFlags & WEAPONFLAG_CLIP_INSTALLED) ) 
	// {
	// 	// The clip is not installed...
	// 	goto _ExitWithoutTriggerPressed;
	// }

	// if( m_fDebrisSecondsCountdownTimer > 0.0f ) 
	// {
	// 	m_fDebrisSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	// }

	if( m_fSpewUnitShotSpread < 1.0f ) 
	{
		m_fSpewUnitShotSpread += FLoop_fPreviousLoopSecs * m_pSpewProps->fShotSpreadSpeed;
		FMATH_CLAMPMAX( m_fSpewUnitShotSpread, 1.0f );
	}
	/*
	if( m_nUpgradeLevel == 2 ) 
	{
	if( m_fCartridgeRotSpeed < m_pUserProps->fL3_CartridgeMaxRevsPerSec ) 
	{
	m_fCartridgeRotSpeed += FLoop_fPreviousLoopSecs * m_pUserProps->fL3_CartridgeRotAccel;
	FMATH_CLAMPMAX( m_fCartridgeRotSpeed, m_pUserProps->fL3_CartridgeMaxRevsPerSec );
	}
	}

	if(  IsOwnedByPlayer() ) 
	{
	CFCamera *pCamera = fcamera_GetCameraByIndex( GetOwner()->m_nPossessionPlayerIndex );
	pCamera->SetCamVibrationForOneFrame( m_pUserProps->fUnitCamVibration );
	}
	*/
	if( m_fSpewSecondsUntilNextTracer > 0.0f ) 
	{
		m_fSpewSecondsUntilNextTracer -= FLoop_fPreviousLoopSecs;
	}

	if( m_fSpewSecondsCountdownTimer > 0.0f ) 
	{
		// Can't fire another round yet...
		return 0;
	}

	m_bFirePrimaryThisFrame = TRUE;
	
	if (IsOwnedByPlayer())
	{
		AIEnviro_BoostPlayerSoundTo(GetPlayerIndex(), 30.0f);
	}



	return 1;

_ExitWithoutTriggerPressed:
	if( m_fSpewUnitShotSpread > 0.0f ) 
	{
		m_fSpewUnitShotSpread -= FLoop_fPreviousLoopSecs * m_pSpewProps->fShotSpreadRecoverySpeed;
		FMATH_CLAMPMIN( m_fSpewUnitShotSpread, 0.0f );
	}
	/*
	if( m_nUpgradeLevel == 2 ) 
	{
	if( m_fCartridgeRotSpeed > 0.0f ) 
	{
	m_fCartridgeRotSpeed -= FLoop_fPreviousLoopSecs * m_pUserProps->fL3_CartridgeRotDecel;
	FMATH_CLAMPMIN( m_fCartridgeRotSpeed, 0.0f );
	}
	}
	*/
	return 0;
}

u32	CFloorSentry::TriggerWorkSecondary( f32 fUnitTriggerVal) 
{
	if( fUnitTriggerVal < 0.25f ) {
		// Trigger not down...
		return 0;
	}

//	if( m_bPossessed && m_fCountPlayerRockets == 0.0f ) 
//	{
//	// No rounds in clip...
//	return 0;
//	}

	if( m_fRocketSecondsCountdownTimer > 0.0f ) 
	{
		// Can't fire another round yet...
		return 0;
	}

	s32 m_nUpgradeLevel = 0;

	// Get a free projectile...
	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( CWeapon::m_ahProjPool[CWeapon::PROJ_POOL_TYPE_SWARMER_ROCKET] );
	if( pProj == NULL ) {
		// No more projectiles...
		return 0;
	}

	// Let's fire a round...

	CFMtx43A ProjMtx;
	CFVec3A ProjUnitDir_WS;
	f32 fJitterFactor;

	fJitterFactor = m_pRocketProps->fShotSpreadFactor;
	//	if( m_pAttachedScope && m_pAttachedScope->IsZoomEnabled() ) {
	//		fJitterFactor *= m_aUserProps[m_nUpgradeLevel].afScopeSpreadMult[ m_pAttachedScope->GetUpgradeLevel() ];
	//	}
	
	CFVec3A* pProjUnitDir_WS = &m_RocketAimUnitVec_WS;
	fmath_ScatterUnitVec( &ProjUnitDir_WS, pProjUnitDir_WS, fJitterFactor );

	ProjMtx.m_vFront = ProjUnitDir_WS;
	ProjMtx.m_vRight.UnitCrossYWithVec( ProjUnitDir_WS );
	ProjMtx.m_vUp.Cross( ProjMtx.m_vFront, ProjMtx.m_vRight );
	ProjMtx.m_vPos = m_apMuzzlePts[4]->m_vPos;

	pProj->Init();

	pProj->SetDamager( NULL, m_pOwnerBot );
	pProj->SetDamageProfile( m_pRocketDamageProfile );
	pProj->SetSkipListCallback( _BuildRocketSkipList );
	pProj->SetDetonateCallback( _RL12_ProjDetonateCallback );
	pProj->SetMaxDistCanTravel( m_pRocketProps->fMaxLiveRange );
	pProj->SetLinSpeed( m_pRocketProps->fRocketSpeed );
	pProj->SetLinUnitDir_WS( &ProjUnitDir_WS );
	pProj->SmokeTrailOn( &CFloorSentry::m_aSmokeTrailAttrib);
	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, 1.5f );
	pProj->SetExplosionGroup( m_pRocketProps->hExplosionGroup );
	pProj->Launch();

	if (IsOwnedByPlayer())
	{
		// Human player...
		fforce_Kill( &m_hForce );
		fforce_Play( Player_aPlayer[GetPlayerIndex()].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );
	}
	m_pOwnerBot->PlaySound(CBotSiteWeapon::GetSiteWeaponSndHandle(CBotSiteWeapon::SITESND_ROCKET));

	// Set timer until the next round is ready...
	m_fRocketSecondsCountdownTimer = m_pRocketProps->fOORoundsPerSec;
	
	
	/*
	if( IsOwnedByPlayer()) {

	// Human player...
	fforce_Kill( &m_hForce );
	fforce_Play( Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );

	fsndfx_Play2D( m_aUserProps[m_nUpgradeLevel].hFiringSound );
	} else {
	fsndfx_Play3D( m_aUserProps[m_nUpgradeLevel].hFiringSound, &ProjMtx.m_vPos, m_aUserProps[m_nUpgradeLevel].fMaxLiveRange, 1.0f );
	}
	*/
	/*
	// Set up cartridge rotation for RL2...
	if( m_nUpgradeLevel == 1 ) {
	m_nRL2_CartMode = _RL2_CARTMODE_FIRE;
	m_fRL2_CartridgeRotAngleStart = m_aUserProps[1].fRL2_AngleBetweenCartridgeSlots * (f32)(GetMaxClipAmmo() - GetClipAmmo());
	m_fRL2_CartridgeRotAngleEnd = m_fRL2_CartridgeRotAngleStart + m_aUserProps[1].fRL2_AngleBetweenCartridgeSlots;
	m_fRL2_CartridgeRotationAngle = m_fRL2_CartridgeRotAngleStart;
	}
*/
	// Remove the round we fired from the clip...
//	if (m_pOwnerBot->IsPlayerBot())
//		m_fCountPlayerRockets = (m_fCountPlayerRockets-1.0f);
/*
	if (GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1)
	{
	AIEnviro_BoostPlayerSoundTo(GetOwner()->m_nPossessionPlayerIndex, 25.0f);
	}
	*/
	// Indicate to caller that trigger #1 fired the weapon...
	return 1;
}

void CFloorSentry::_DeadMeshDraw( BOOL bDrawDeadMesh)
{
	if (bDrawDeadMesh == FALSE)
	{
		m_DeadMesh.RemoveFromWorld();
	}
	else
	{
		m_DeadMesh.m_Xfm.BuildFromMtx(m_pData->m_MtxPosition_WS);
		m_DeadMesh.UpdateTracker();
	
		for(CEntity* pChild=m_pOwnerBot->GetFirstChild(); pChild; pChild=m_pOwnerBot->GetNextChild(pChild) ) 
		{
			if( pChild->GetAttachBoneName() ) 
			{
				// Child is attached to one of our bones...
				s32 nBoneIndex = m_DeadMesh.FindBone( pChild->GetAttachBoneName() );
				if( nBoneIndex >= 0 ) 
				{
					// The same bone exists in the new mesh. Lets use it...
					pChild->m_pParentMtxToWorld = m_DeadMesh.GetBoneMtxPalette()[nBoneIndex];
				} 
				else 
				{
					// The same bone doesn't exist in the new mesh. Let's use our model matrix...
					pChild->m_pParentMtxToWorld = &m_DeadMesh.m_Xfm.m_MtxF;
				}
			}
		}
	}
}

void CFloorSentry::_HandleFireThisFrame(void)
{
	if( m_bFirePrimaryThisFrame ) 
	{
		if (m_pData->m_eSiteWeaponState != CBotSiteWeapon::SITE_POSSESSED)
			m_fRecoil0 = -m_fRecoilDistanceNormal; // recoil me;
		else
			m_fRecoil0 = -m_fRecoilDistanceCustom; // recoil me;

		CFVec3A vFireUnitDir = m_apMuzzlePts[0]->m_vFront;
		if( m_pData->m_pDriverBot && GetPlayerIndex() >= 0 ) 
		{
			CFCamera *pCamera = fcamera_GetCameraByIndex( GetPlayerIndex() );
			pCamera->SetCamVibrationForOneFrame( 1.0f );			
			m_pOwnerBot->FocusHumanTargetPoint_WS(&m_pOwnerBot->m_TargetedPoint_WS, NULL);
			vFireUnitDir.Sub(m_pOwnerBot->m_TargetedPoint_WS, m_pData->m_vAimingOrigin_WS);
			vFireUnitDir.Unitize();
		}

		CFVec3A PerturbVec;
		f32 fJitterFactor, fBestSpread, fWorstSpread;

		fBestSpread = m_pSpewProps->fBestShotSpreadFactor;
		fWorstSpread = m_pSpewProps->fWorstShotSpreadFactor;

		fJitterFactor = FMATH_FPOT( m_fSpewUnitShotSpread, fBestSpread, fWorstSpread );

		PerturbVec.Set(
			fmath_RandomFloatRange( -fJitterFactor, fJitterFactor ),
			fmath_RandomFloatRange( -fJitterFactor, fJitterFactor ),
			fmath_RandomFloatRange( -fJitterFactor, fJitterFactor )
			);

		
		vFireUnitDir.Add( PerturbVec );
		vFireUnitDir.Unitize();

		if(!m_bPossessed && fmath_RandomRange(0, 9) == 0)
			m_fSpewSecondsCountdownTimer += m_pSpewProps->fOORoundsPerSec + fmath_RandomFloatRange(0.2f, 0.3f);
		else
			m_fSpewSecondsCountdownTimer += m_pSpewProps->fOORoundsPerSec;

		s32 nBeginMuzzle,nEndMuzzle;
		float fBarrelOffset; 
		if (m_bFireTopBarrels)
		{
			nBeginMuzzle=0;
			nEndMuzzle=1;
			fBarrelOffset = 2.20f;
		}
		else
		{
			nBeginMuzzle=2;
			nEndMuzzle=3;
			fBarrelOffset = 1.20f;
		}

		CFVec3A ApproxMuzzlePoint = CFVec3A::m_Null;
		CFVec3A MuzzlePoint;
		for (s32 nMuzzle = nBeginMuzzle;nMuzzle<=nEndMuzzle;nMuzzle++)
		{
			if (!m_bPossessed)
			{
				MuzzlePoint = m_apMuzzlePts[nMuzzle]->m_vPos;
			}
			else
			{
				CFVec3A vMediate;
				vMediate.Mul(m_apMuzzlePts[nMuzzle]->m_vFront,fBarrelOffset );
				MuzzlePoint.Add(m_apMuzzlePts[nMuzzle]->m_vPos,vMediate);
			}


			f32 fRandomScale;
			if (!m_bPossessed)
			{
				fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );
				f32 fDistanceScale = 3.0f;
				f32 fOODistanceMax = .015f;
				fDistanceScale = fOODistanceMax * m_pData->m_fDistanceToTarget;
				FMATH_CLAMPMAX(fDistanceScale, 3.0f);
				fRandomScale += fDistanceScale;
			}
			else
				fRandomScale = fmath_RandomFloatRange( 0.4f, .5f );


			CMuzzleFlash::AddFlash_CardPosterZ(
				CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
				MuzzlePoint,
				vFireUnitDir,
				m_pSpewProps->fMuzzleWidth_Z * fRandomScale,
				m_pSpewProps->fMuzzleHeight_Z * fRandomScale,
				m_pSpewProps->fMuzzleAlpha_Z
				);

			if (!m_bPossessed)
			{
				CMuzzleFlash::AddFlash_CardPosterXY(
					CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
					MuzzlePoint,
					m_pSpewProps->fMuzzleScale_XY * fRandomScale,
					m_pSpewProps->fMuzzleAlpha_XY
					);

				CMuzzleFlash::AddFlash_Mesh3D(
					CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_3D_STAR_1 + fmath_RandomRange( 0, 1 ) ],
					MuzzlePoint,
					vFireUnitDir,
					m_pSpewProps->fMuzzleScale_Mesh * fRandomScale,
					m_pSpewProps->fMuzzleRot_Mesh
					);
			}
			else
			{
				FASSERT(IsOwnedByPlayer());
				fforce_Kill( &m_hForce );
				fforce_Play( Player_aPlayer[GetPlayerIndex()].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce );
			}
			
			CFVec3A RayEnd;
			FCollImpact_t CollImpact;

			RayEnd.Mul( vFireUnitDir, m_pSpewProps->fMaxLiveRange ).Add( MuzzlePoint );

			FWorld_nTrackerSkipListCount = 0;
			m_pOwnerBot->AppendTrackerSkipList();
			if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &MuzzlePoint, &RayEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) 
			{
				// Hit something...

				RayEnd = CollImpact.ImpactPoint;

				CFVec3A ImpactFlashPos;

				const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( &CollImpact );

				pCollMaterial->DrawAllDrawableParticles(
					&CollImpact.ImpactPoint,
					&CollImpact.UnitFaceNormal,
					FALSE,
					fmath_RandomFloatRange( 0.2f, 1.0f ),
					fmath_RandomFloatRange( 0.2f, 0.5f ),
					fmath_RandomFloatRange( 0.1f, 0.3f )
					);

				ImpactFlashPos.Mul( CollImpact.UnitFaceNormal, m_pSpewProps->fMuzzleOffset_Impact ).Add( CollImpact.ImpactPoint );

				if( pCollMaterial->IsImpactFlashEnabled() ) 
				{
					CMuzzleFlash::AddFlash_CardPosterXY(
						CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
						ImpactFlashPos,
						m_pSpewProps->fMuzzleScale_Impact,
						m_pSpewProps->fMuzzleAlpha_Impact
						);
				}
				CEntity* pEntity = CGColl::ExtractEntity( &CollImpact );

				CFDecal::Create( m_pSpewProps->hBulletDecal, &CollImpact, &vFireUnitDir );

				if( pEntity == NULL ) 
				{
					// Hit world geo...
					//if( pCollMaterial->IsPockMarkEnabled( &CollImpact ) ) 
					//{
					//	potmark_NewPotmark( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.2f, POTMARKTYPE_BULLET );
					//}
				} 
				else 
				{
					// It's an Entity!
					CEntity *pDamagee = pEntity;
					CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;
					// Get an empty damage form...
					CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

					if( pDamageForm ) 
					{
						// Fill out the form...

						pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
						pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
						pDamageForm->m_pDamageProfile = m_pSpewDamageProfile;
						pDamageForm->m_Damager.pWeapon = NULL;
						pDamageForm->m_Damager.pBot = m_pOwnerBot;
						pDamageForm->m_Damager.nDamagerPlayerIndex = GetPlayerIndex();
						pDamageForm->m_pDamageeEntity = pDamagee;
						pDamageForm->InitTriDataFromCollImpact( pWorldMesh, &CollImpact, &vFireUnitDir );

						CDamage::SubmitDamageForm( pDamageForm );
					}
				}
			
				// Make an AI impact sound in case any AI are listening...
				if( IsOwnedByPlayer() && m_fSecsUntilNextSound <= 0.0f ) 
				{
					AIEnviro_AddSound( CollImpact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, m_pOwnerBot );
					m_fSecsUntilNextSound = 0.5f;
				}
			}

			BOOL bMakeTracer = FALSE;

			if( IsOwnedByPlayer() == FALSE) 
			{
				// AI...

				bMakeTracer = fmath_RandomChance( 0.4f );
			} 
			else 
			{
				// Human-controlled...

				if( m_fSpewSecondsUntilNextTracer <= 0.0f ) 
				{
					m_fSpewSecondsUntilNextTracer = fmath_RandomFloatRange( 0.1f, 0.25f );
					bMakeTracer = TRUE;
				}
			}

			if( bMakeTracer ) 
			{
				// Make a tracer...

				f32 fTracerLen = MuzzlePoint.Dist( RayEnd );

				CMuzzleFlash::AddFlash_CardPosterZ(
					CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1],
					MuzzlePoint,
					vFireUnitDir,
					0.15f,
					fTracerLen,
					0.2f
					);
			}
		} // end for firing muzzles

		// Spawn muzzle light...
		f32 fMuzzleLightRadius = (IsOwnedByPlayer() ? 15.0f : 10.f);
		CMuzzleFlash::MuzzleLight( &m_pData->m_vAimingOrigin_WS, fMuzzleLightRadius, fmath_RandomFloatRange( 0.5f, 1.0f ) );

		// Play firing sound...
		if( m_apAudioEmitterFire[m_nAudioEmitterIndex] ) {
			m_apAudioEmitterFire[m_nAudioEmitterIndex]->Destroy();
			m_apAudioEmitterFire[m_nAudioEmitterIndex] = NULL;
		}

		m_pOwnerBot->PlaySound( CBotSiteWeapon::GetSiteWeaponSndHandle(CBotSiteWeapon::SITESND_FIRE), 1.0f, 1.0f, -1.0f, &m_apAudioEmitterFire[m_nAudioEmitterIndex] );

		if( ++m_nAudioEmitterIndex >= MAX_SIMULTANEOUS_FIRE_SOUNDS ) {
			m_nAudioEmitterIndex = 0;
		}

		m_bFireTopBarrels = !m_bFireTopBarrels; // toggle muzzles to fire
	}
	if( m_fSpewSecondsCountdownTimer > 0.0f ) 
	{
		m_fSpewSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}
	if( m_fRocketSecondsCountdownTimer > 0.0f ) 
	{
		m_fRocketSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}
	else
	{
		m_fRocketSecondsCountdownTimer = 0.0f;
	}
	m_fUnitTimeUntilRocketReady = 1.0f - fmath_Div(m_fRocketSecondsCountdownTimer,m_pRocketProps->fOORoundsPerSec);
	FMATH_CLAMP_UNIT_FLOAT(m_fUnitTimeUntilRocketReady);
}

void CFloorSentry::_BuildRocketSkipList( CEProj *pProj )
{
	CBot* pBot = (CBot*)pProj->GetDamagerBot();
	pBot->AppendTrackerSkipList();
}

BOOL CFloorSentry::_RL12_ProjDetonateCallback( CEProj *pProj, BOOL bMakeEffect, CEProj::Event_e nEvent, const FCollImpact_t *pImpact ) 
{
	if( bMakeEffect )
	{
		if( pImpact ) 
		{
			// Get an empty damage form...
			CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

			if( pDamageForm ) 
			{
				// Fill out the form...
				pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
				pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
				pDamageForm->m_pDamageProfile = pProj->GetDamageProfile();
				pDamageForm->m_Epicenter_WS = pProj->MtxToWorld()->m_vPos;
				pDamageForm->m_Damager = *pProj->GetDamager();

				CDamage::SubmitDamageForm( pDamageForm );
			}
		}
	}

	return TRUE;
}

void CFloorSentry::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	pApproxMuzzlePoint_WS->Zero();
	for (s32 nPt=0;nPt<4;nPt++)
	{
		pApproxMuzzlePoint_WS->Add(m_apMuzzlePts[nPt]->m_vPos);
	}
	pApproxMuzzlePoint_WS->Mul(0.25f);
}

void CFloorSentry::Work(void)								// my tick
{
	// Call into alternate work functions
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_UP) 
	{
		_PowerUpWork();
		m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
		m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
		_UpdateLight();
		return;
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_DOWN)
	{
		_UpdateLight();
		return _PowerDownWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
	{
		_UpdateLight();
		return _DeathWork();
	}
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWERING_UP_POSSESSED) 
	{
		_PowerUpWork(); // plus the barrel positioning!
	}

	// Recruitment test. If we have been recruited, and our recruiter has died,
	// we automagically get unrecruited.
	if (MultiplayerMgr.IsMultiplayer() && m_pOwnerBot->Recruit_IsRecruited())
	{
		// A little circuitous here; see if the original player bot has been
		// killed.
		CBot* pPlayerBot = (CBot*)Player_aPlayer[m_pOwnerBot->Recruit_GetRecruiter()].m_pEntityOrig;
		FASSERT(pPlayerBot->TypeBits() & ENTITY_BIT_BOT);
		if (pPlayerBot->IsDeadOrDying())
		{
			m_pOwnerBot->Unrecruit();
			m_pOwnerBot->Power(FALSE);
		}
	}

	// update the barrels
	if(m_fRecoil0 < 0.0f)
	{
		m_fRecoil0 += m_fRecoilReturnRate * FLoop_fPreviousLoopSecs;
	}
	if(m_fRecoil0 > 0.0f)
	{
		m_fRecoil0 = 0.0f;
	}
	
	m_AliveAnimManMtxAim.UpdateFrame(BONE_VERTICAL,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_HORIZONTAL,m_pData->m_MtxAimY_MS);

	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated

	m_MtxRecoil.m_vPos.z	= m_fRecoil0;

	// DFS -- Always update the third-person barrels for multiplayer
	if ( !m_bPossessed || (CPlayer::m_nPlayerCount > 1) )
	{
		if(!m_bFireTopBarrels) // set to false after they fire, therefore update this set
		{
			m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_T_R, m_MtxRecoil);
			m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_T_L, m_MtxRecoil);
		}
		else
		{
			m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_B_R, m_MtxRecoil);
			m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_B_L, m_MtxRecoil);
		}
	}
	
	// We only have to update the first-person barrels if someone is in there
	if (m_bPossessed)
	{
		CFMtx43A WorkMtx;
		CFVec3A WorkVec;
		f32 fRecoilTop=0.f;
		f32 fRecoilBottom=0.f;
		if(!m_bFireTopBarrels) // set to false after they fire, therefore update this set
		{
			fRecoilTop = m_fRecoil0;
		}
		else
		{
			fRecoilBottom = m_fRecoil0;
		}
		// unrolled for per barrel positioning.
		// Note: UpdateTracker is now called from PreUserRender; don't call it
		// here. Or if you do, follow it with RemoveFromWorld().
		s32 nMuzzle = 0;
		WorkMtx = *m_apMuzzlePts[nMuzzle];
		WorkVec.Mul(WorkMtx.m_vFront,(-2.5f + fRecoilTop));
		WorkMtx.m_vPos.Add(WorkVec);
		m_apCustomBarrels[nMuzzle].m_Xfm.BuildFromMtx(WorkMtx,TRUE);
		
		nMuzzle = 1;
		WorkMtx = *m_apMuzzlePts[nMuzzle];
		WorkVec.Mul(WorkMtx.m_vFront,(-2.5f + fRecoilTop) );
		WorkMtx.m_vPos.Add(WorkVec);
		m_apCustomBarrels[nMuzzle].m_Xfm.BuildFromMtx(WorkMtx,TRUE);

		nMuzzle = 2;
		WorkMtx = *m_apMuzzlePts[nMuzzle];
		WorkVec.Mul(WorkMtx.m_vFront,(-3.5f + fRecoilBottom));
		WorkMtx.m_vPos.Add(WorkVec);
		m_apCustomBarrels[nMuzzle].m_Xfm.BuildFromMtx(WorkMtx,TRUE);

		nMuzzle = 3;
		WorkMtx = *m_apMuzzlePts[nMuzzle];
		WorkVec.Mul(WorkMtx.m_vFront,(-3.5f + fRecoilBottom));
		WorkMtx.m_vPos.Add(WorkVec);
		m_apCustomBarrels[nMuzzle].m_Xfm.BuildFromMtx(WorkMtx,TRUE);
	}

//	m_pWorldLight->UpdateTracker(m_pSearchLightMtx);

#if 0
	CFColorRGBA red;red.OpaqueRed();
	CFVec3A Start, End, Dir;
	Start = m_pSearchLightMtx->m_vPos;
	Dir =m_pSearchLightMtx->m_vFront;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&red,&red);

	CFColorRGBA red;red.OpaqueRed();
	CFVec3A Start, End, Dir;
	Start = m_apMuzzlePts[0]->m_vPos;
	Dir =m_apMuzzlePts[0]->m_vFront;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&red,&red);
#endif
    ComputeApproxMuzzlePoint_WS(&m_pData->m_vAimingOrigin_WS);	// update gun's position
	
//	CDebugDraw::Sphere(m_pData->m_vAimingOrigin_WS, .8f);

	if (m_bPossessed)
	{
		_ComputeCameraMatrix_WS();	// update camera's matrix 
		_ComputeRocketDirection(100.0f); // the parameter isn't actually used anymore for possessed cases
	}
	else
	{
		if (m_pData->m_pEnemyTarget) // only update the rocket if we've got a target
			_ComputeRocketDirection(m_pData->m_fDistanceToTarget);
	}
	_HandleFireThisFrame();
	_UpdateLight();


	//
	//	notify AI system of our presence
	//  if we are from a player.
	//
	if(!m_bPossessed)
	{
		if (!m_uAIVisibleSoundHandle || !AIEnviro_ModifySound( m_uAIVisibleSoundHandle,  m_pOwnerBot->MtxToWorld()->m_vPos, m_pOwnerBot->m_fCollCylinderRadius_WS, 0.3f, AISOUNDTYPE_NONE, m_pOwnerBot) )
		{
			m_uAIVisibleSoundHandle = AIEnviro_AddSound( m_pOwnerBot->MtxToWorld()->m_vPos, m_pOwnerBot->m_fCollCylinderRadius_WS, 0.3f, AISOUNDTYPE_NONE, AISOUNDCTRL_VISIBLE_ENTITY, m_pOwnerBot );
		}
	}
}

void 	CFloorSentry::CueNewState(void)								// my tick
{
	if ((m_ePreviousState == CBotSiteWeapon::SITE_POWER_UP) || 
		(m_ePreviousState == CBotSiteWeapon::SITE_POWERING_UP_POSSESSED))
	{
		m_fCurrentLightAlpha=1.0f;
	}
	if (m_ePreviousState == CBotSiteWeapon::SITE_POSSESSED)
	{
		_CuePossessed(FALSE);
	}
	
	m_ePreviousState = m_pData->m_eSiteWeaponState;
	
	switch (m_pData->m_eSiteWeaponState)
	{
	case CBotSiteWeapon::SITE_POWERING_UP_POSSESSED:
		return _CuePossessedPowerUp();
	case CBotSiteWeapon::SITE_POWER_UP:
		return _CuePowerUp();
	case CBotSiteWeapon::SITE_POWER_DOWN:
		return _CuePowerDown();
	case CBotSiteWeapon::SITE_DEAD:
		return _CueDeath();
	case CBotSiteWeapon::SITE_POSSESSED:
		return _CuePossessed(TRUE);
	}
}


void 	CFloorSentry::_PowerUpWork(void)						// my other tick
{
	if (m_pData->m_fPitchWS <= 0.0f)
	{
		m_pData->m_fPitchWS = 0;
	}
	else
	{
		m_pData->m_fPitchWS -= FLoop_fPreviousLoopSecs;
	}

	m_pData->m_MtxAimY_MS.SetRotationY(m_pData->m_fYawWS-m_pData->m_fYawAdjustWS2MS);
	m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_VERTICAL,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_HORIZONTAL,m_pData->m_MtxAimY_MS);
//	m_pWorldLight->UpdateTracker(m_pSearchLightMtx);

	f32 fPercentStateChanged = fmath_Div(m_pData->m_fTimeSinceStateChanged,m_pData->m_fTimeToPowerUp);
	FMATH_CLAMPMAX(fPercentStateChanged,1.0f);
	m_fCurrentLightAlpha = fPercentStateChanged;
}

void 	CFloorSentry::_PowerDownWork(void)					// my other other tick
{
	if ( (FMATH_FABS(m_fPowerDownVelocity) > 0.05) ||	// keep going as long as velocity is big
		(m_pData->m_fPitchWS < m_pData->m_fMaximumPitchWS) )				//	or going up
	{
		m_pData->m_fPitchWS = m_pData->m_fPitchWS + ((m_fPowerDownVelocity) * FLoop_fPreviousLoopSecs);
		if (m_pData->m_fPitchWS >= m_pData->m_fMaximumPitchWS) // bounce bottom
		{
			m_pData->m_fPitchWS = m_pData->m_fMaximumPitchWS;
			m_fPowerDownVelocity *= (-m_fPowerDownVelocityDamp);
		}
		m_fPowerDownVelocity += (m_fPowerDownAcceleration * FLoop_fPreviousLoopSecs);
	}
	m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_VERTICAL,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_HORIZONTAL,m_pData->m_MtxAimY_MS);
	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
//	m_pWorldLight->UpdateTracker(m_pSearchLightMtx);

	f32 fPercentStateChanged = fmath_Div(m_pData->m_fTimeSinceStateChanged,m_pData->m_fTimeToPowerDown);
	FMATH_CLAMPMAX(fPercentStateChanged,1.0f);
	m_fCurrentLightAlpha = 1.0f - fPercentStateChanged;
	m_eLightPage = OFF;
}

void CFloorSentry::_DeathWork()						// my other tick
{
// dying:
	if ( (FMATH_FABS(m_fDeathRandomAngularVelocity) > 0.05) ||	// keep going as long as velocity is big
		(m_pData->m_fPitchWS < m_pData->m_fMaximumPitchWS) )				//	or going up
	{
		m_pData->m_fPitchWS = m_pData->m_fPitchWS + ((m_fDeathRandomAngularVelocity) * FLoop_fPreviousLoopSecs);
		if (m_pData->m_fPitchWS >= m_pData->m_fMaximumPitchWS) // bounce bottom
		{
			m_pData->m_fPitchWS = m_pData->m_fMaximumPitchWS;
			m_fDeathRandomAngularVelocity *= (-m_fDeathVelocityDamp);
			m_pData->m_fYawVelocity *= m_fDeathVelocityDamp;
		}

		if (m_pData->m_fPitchWS <= m_pData->m_fMinimumPitchWS) // bounce top
		{
			m_pData->m_fPitchWS = m_pData->m_fMinimumPitchWS;
			m_fDeathRandomAngularVelocity *= (-1.0f); // elastic collision 'ere
		}
		m_fDeathRandomAngularVelocity += (m_fDeathAcceleration * FLoop_fPreviousLoopSecs);
		
		m_pData->m_fYawWS += (m_pData->m_fYawVelocity * FLoop_fPreviousLoopSecs);
		
		m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
		m_pData->m_MtxAimY_MS.SetRotationY(m_pData->m_fYawWS - m_pData->m_fYawAdjustWS2MS);

		m_DeadAnimManMtxAim.UpdateFrame(BONE_VERTICAL,  m_pData->m_MtxAimX_MS);
		m_DeadAnimManMtxAim.UpdateFrame(BONE_HORIZONTAL,m_pData->m_MtxAimY_MS);
		m_DeadAnim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
		m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
	}
	CFVec3A pos =  m_pDeadMesh->m_Xfm.m_MtxF.m_vPos;
	CFVec3A up  =  m_pDeadMesh->m_Xfm.m_MtxF.m_vUp;
}

void 	CFloorSentry::_CuePossessedPowerUp(void)
{
	_CuePossessed(TRUE);
}


void 	CFloorSentry::_CuePowerUp(void)							// turn me on
{
}

void 	CFloorSentry::_CuePowerDown()	// WHIRRRWhiRRwwwhir.
{
	m_fRecoil0 =0.0f;
	m_MtxRecoil = CFMtx43A::m_IdentityMtx;
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_T_R, m_MtxRecoil);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_B_R, m_MtxRecoil);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_T_L, m_MtxRecoil);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_B_L, m_MtxRecoil);
	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
//	m_pWorldLight->UpdateTracker(m_pSearchLightMtx);
	m_eLightPage = OFF;
}

void 	CFloorSentry::_CuePossessed(BOOL bGoPosed)		// transform!() 
{
	FASSERT(m_pOwnerBot->m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY);

	if (bGoPosed)
	{
		if (m_bPossessed) // already did this
			return;

		// Index of the player, better be glitch, eh?
		FASSERT(m_pData->m_pDriverBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
		s32 nPlayerIdx = m_pData->m_pDriverBot->m_nOwnerPlayerIndex;


		m_pOwnerBot->SetControls( m_pData->m_pDriverBot->Controls());

		// Attach the bot to the gun
		s32 nAttach = CBotSiteWeapon::m_nBoneIndexFloorSentryAttachPt;
		FASSERT(nAttach >= 0);
		CFMtx43A* pMtx = m_pAliveMesh->GetBoneMtxPalette()[nAttach];
		m_pData->m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS(pMtx);
		m_pData->m_pDriverBot->Attach_ToParent_WithGlue_WS( m_pOwnerBot,"AttachPoint_Bot");

		m_pData->m_pDriverBot->SetControls(NULL);
//		m_pData->m_pDriverBot->EnteringMechWork(m_pOwnerBot);  //tell the driver that we are driving

		// Hmmm... The player bot is pretty exposed, so let's let him be hit in multiplayer
		if (MultiplayerMgr.IsSinglePlayer()) {
			m_pData->m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
			Player_aPlayer[nPlayerIdx].SetHiddenFromSelf(TRUE);
			m_pData->m_pDriverBot->SetInvincible(TRUE); // no damage taken while in floor sentry 
		}

		m_eLightPage = YELLOW;
		
		CReticle *pReticle = &Player_aPlayer[ nPlayerIdx ].m_Reticle;
		m_fOldNormX = pReticle->GetNormOriginX();
		m_fOldNormY = pReticle->GetNormOriginY();
		m_bOldReticleIsDrawn = pReticle->IsDrawEnabled();
		pReticle->SetNormOrigin(0.0f,0.0f);
		m_OldReticleType = pReticle->GetType();
		pReticle->SetType( CReticle::TYPE_FLOORSENTRY );
		pReticle->SetUnitScale(0.0f);
		pReticle->SetReticleOverride();
		pReticle->EnableDraw(TRUE, TRUE);

		// Hud for the possessing player
		CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIdx);
		pHud->SetHudMode( HUDMODE_MIL );
		pHud->SetDrawFlags(CHud2::DRAW_ENABLEOVERRIDE | /*CHud2::DRAW_LEFT_AMMOBOX |*/ CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR | CHud2::DRAW_RIGHT_AMMOBOX);
//		pHud->OverrideAmmoData(CHud2::LEFT,CHud2::OVERRIDE_COUNTER_MIL,&m_fCountPlayerRockets,FALSE);
		CFColorRGBA startColor, endColor;
		startColor.OpaqueRed();
		endColor.OpaqueGreen();
		pHud->OverrideAmmoData( CHud2::RIGHT, CHud2::OVERRIDE_METER_MIL, &m_fUnitTimeUntilRocketReady, TRUE, &startColor, &endColor );

		m_bPossessed = TRUE;
		
		m_pSpewProps=&m_aSpewProps[1];
		m_pRocketProps=&m_aRocketProps[1];
		m_pSpewDamageProfile=m_aSpewDamageProfile[1];
		m_pRocketDamageProfile=m_aRocketDamageProfile[1];

		_ComputeCameraMatrix_WS();
		gamecam_SwitchPlayerToManualCamera(PLAYER_CAM(nPlayerIdx),&m_pData->m_CamManInfo);
	}
	else
	{
		if (m_pData->m_pDriverBot->IsInWorld()) // now, can get called on quit, and driver gets popped first
		{
			// Index of the player, better be glitch, eh?
			FASSERT(m_pData->m_pDriverBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
			s32 nPlayerIdx = m_pData->m_pDriverBot->m_nOwnerPlayerIndex;

			// Hud for the possessing player
			CHud2* pHud = CHud2::GetHudForPlayer(nPlayerIdx);

			gamecam_SwitchPlayerTo3rdPersonCamera(PLAYER_CAM(nPlayerIdx),m_pData->m_pDriverBot);

			m_pSpewProps=&m_aSpewProps[0];
			m_pRocketProps=&m_aRocketProps[0];
			m_pSpewDamageProfile=m_aSpewDamageProfile[0];
			m_pRocketDamageProfile=m_aRocketDamageProfile[0];

			CEntityControl* pCtl = m_pOwnerBot->Controls();
			m_pOwnerBot->SetControls( NULL ); 
			//		m_pData->m_pDriverBot->ExitingMechWork();
			if (IsOwnedByPlayer())
			{
				CPlayer *pPlayer = &Player_aPlayer[ GetPlayerIndex() ];
				pPlayer->ZeroControls();
			}
			m_pData->m_pDriverBot->SetControls (pCtl);
			m_pData->m_pDriverBot->DetachFromParent();
			m_pData->m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
			Player_aPlayer[nPlayerIdx].SetHiddenFromSelf(FALSE);
			m_pData->m_pDriverBot->SetInvincible(FALSE); 
//			m_pData->m_pDriverBot->ExitingMechWork();  //tell the driver that we are exiting

			CReticle *pReticle = &Player_aPlayer[ nPlayerIdx ].m_Reticle;
			pReticle->ClearReticleOverride();
			pReticle->SetNormOrigin(m_fOldNormX,m_fOldNormY);
			pReticle->SetType(m_OldReticleType);
			pHud->SetHudMode( HUDMODE_GLITCH );
			//pReticle->EnableDraw(m_bOldReticleIsDrawn,TRUE);
			pReticle->EnableDraw(FALSE);
			pReticle->SetUnitScale(0.0f);
		}
		m_pData->m_pDriverBot = 0;
		m_bPossessed = FALSE;
	}
}


void CFloorSentry::_CueDeath(void)
{
	_DeadMeshDraw( TRUE );
	m_fCurrentLightAlpha=0.0f;
}


BOOL CFloorSentry::CheckpointSave( void )
{
	BOOL bDead = m_DeadMesh.IsAddedToWorld();
	CFCheckPoint::SaveData( (u32&)bDead  );
	CFCheckPoint::SaveData( (u32&) m_eLightPage );
//	CFCheckPoint::SaveData( (u32&) m_fCountPlayerRockets );
	CFCheckPoint::SaveData( (u32&) m_fCurrentLightAlpha );
	return TRUE;
}

void CFloorSentry::CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData )
{
	_KillAudioEmitters();
	BOOL bDeadMeshDraw;
	CFCheckPoint::LoadData( (u32&) bDeadMeshDraw );
	_DeadMeshDraw(bDeadMeshDraw);
	CFCheckPoint::LoadData( (u32&) m_eLightPage );
//	CFCheckPoint::LoadData( (u32&) m_fCountPlayerRockets );
	CFCheckPoint::LoadData( (u32&) m_fCurrentLightAlpha );
}

void CFloorSentry::PreUserRender( void )
{
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
		return;

	m_apCustomBarrels[0].UpdateTracker();
	m_apCustomBarrels[1].UpdateTracker();
	m_apCustomBarrels[2].UpdateTracker();
	m_apCustomBarrels[3].UpdateTracker();
}

void CFloorSentry::PostUserRender( void )
{
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
		return;

	m_apCustomBarrels[0].RemoveFromWorld();
	m_apCustomBarrels[1].RemoveFromWorld();
	m_apCustomBarrels[2].RemoveFromWorld();
	m_apCustomBarrels[3].RemoveFromWorld();
}

void CFloorSentry::_ComputeRocketDirection(f32 fTargetDist)
{
	CFVec3A vTargetDirection;
	if (m_bPossessed)
	{
		m_pOwnerBot->FocusHumanTargetPoint_WS(&m_pOwnerBot->m_TargetedPoint_WS, NULL);
		vTargetDirection.Sub(m_pOwnerBot->m_TargetedPoint_WS, m_pData->m_vAimingOrigin_WS);
		vTargetDirection.Unitize();
	}
	else // do some leadin'
	{
		CFVec3A vTargetDistance;
		CFVec3A vGazingDirection;
		CFVec3A vUnitMuzzleFront;
		vUnitMuzzleFront.ReceiveUnit(m_pData->m_pMtxAimOrientation_WS->m_vFront);

		vGazingDirection.Sub(m_pOwnerBot->m_TargetedPoint_WS,m_apMuzzlePts[4]->m_vPos);
		vTargetDistance.Mul(vUnitMuzzleFront,fTargetDist);
		vGazingDirection.Add(vTargetDistance);
		vGazingDirection.Unitize();

		FASSERT(m_pData->m_pEnemyTarget);
		CFVec3A vTargetVelocity = m_pData->m_pEnemyTarget->m_Velocity_WS;
		f32 fRocketTimeToTarget = fmath_Div(fTargetDist,m_pRocketProps->fRocketSpeed);
		vTargetVelocity.Mul(fRocketTimeToTarget);
		CFVec3A vTargetPt;
		vTargetPt.Set(m_pData->m_pEnemyTarget->MtxToWorld()->m_vPos);
		vTargetPt.Add(vTargetVelocity);
		vTargetPt.y = fmath_RandomFloatRange(vTargetPt.y,m_pOwnerBot->m_TargetedPoint_WS.y); // somewhere from head to toe

		vTargetDistance.Mul(vUnitMuzzleFront, fTargetDist);
		vTargetDirection.Sub(vTargetPt,m_apMuzzlePts[4]->m_vPos);
		vTargetDirection.Unitize();

		CFVec3A vLeftTargetDirCap,vRightTargetDirCap;
		CFVec3A vLeftTargetDirCapPlus90,vRightTargetDirCapPlus90;
		vLeftTargetDirCapPlus90.y=0.0f;
		vRightTargetDirCapPlus90.y=0.0f;
		vLeftTargetDirCap.ReceiveRotationY(vGazingDirection,FMATH_DEG2RAD(-12.f));
		vRightTargetDirCap.ReceiveRotationY(vGazingDirection,FMATH_DEG2RAD(20.f));
		vLeftTargetDirCapPlus90.x  = vLeftTargetDirCap.z;
		vLeftTargetDirCapPlus90.z  = -vLeftTargetDirCap.x;
		vRightTargetDirCapPlus90.x = vRightTargetDirCap.z;
		vRightTargetDirCapPlus90.z = -vRightTargetDirCap.x;

		f32 fCosTestLeft = vTargetDirection.Dot(vLeftTargetDirCapPlus90);
		if (fCosTestLeft < 0.0f) // out of cap left, set to left dir
		{
			vTargetDirection = vLeftTargetDirCap;
		}
		else
		{
			f32 fCosTestRight = vTargetDirection.Dot(vRightTargetDirCapPlus90);
			if (fCosTestRight > 0.0f) // out of cap right, set to right dir
			{
				vTargetDirection = vRightTargetDirCap;
			}
		}
	}

	
	m_RocketAimUnitVec_WS = vTargetDirection;

#if 0
	CFColorRGBA cyan;
	cyan.OpaqueCyan();
	CFVec3A Start, End, Dir;

	Start = m_apMuzzlePts[4]->m_vPos;
	Dir = m_RocketAimUnitVec_WS;
	End.Add(Start,Dir.Mul(100.0f));

	fdraw_DevLine(&Start,&End,&cyan,&cyan);
#endif
}

void CFloorSentry::_ComputeCameraMatrix_WS(void)
{
	m_pData->m_MtxCam_WS.UnitMtxFromNonUnitVec( &m_pData->m_pMtxAimOrientation_WS->m_vFront );
	m_pData->m_MtxCam_WS.m_vPos = m_pData->m_vAimingOrigin_WS;

#if 0 
	CFColorRGBA blue;blue.OpaqueBlue();
	CFColorRGBA white;white.OpaqueWhite();
	CFColorRGBA red;red.OpaqueRed();
	CFVec3A camStart, camEnd, camDir;

	camStart = m_pData->m_MtxCam_WS.m_vPos;
	camStart.y -= 0.5f;
	camDir = m_pData->m_MtxCam_WS.m_vFront;
	camEnd.Add(camStart,camDir.Mul(100.0f));

	fdraw_DevLine(&camStart,&camEnd,&blue,&blue);
#endif

}
void CFloorSentry::_UpdateLight(void)
{
	if (!m_pData->m_bEnableLight)
		return;
	
	f32 fMinEmissive = -.45f;
	f32 fMaxEmissive = 1.0f;

	m_eLightPage = ( (m_pOwnerBot->m_pBotDef->m_nRace == BOTRACE_MIL) ? RED : YELLOW);
	
	FASSERT(m_AliveMesh.GetTexFlip(hLightLayerHandle));

	if (m_fCurrentLightAlpha==0.0f)
	{
		m_pWorldLight->m_Light.Enable(FALSE);
		m_AliveMesh.LayerEmissive_Set(hPanelLayerHandle,fMinEmissive,fMinEmissive,fMinEmissive);
		m_AliveMesh.TexFlip_SetFlipPage(hLightLayerHandle,(u32)OFF);
	}
	else
	{
		FASSERT(m_pData->m_bEnableLight);

		m_pWorldLight->m_Light.Enable(TRUE);
		if (m_eLightPage==RED)
		{
			m_pWorldLight->m_Light.m_Motif.Set(m_vRedLightColor);
			m_AliveMesh.TexFlip_SetFlipPage(hLightLayerHandle,(u32)RED);
		}
		else if (m_eLightPage==YELLOW)
		{
			m_pWorldLight->m_Light.m_Motif.Set(m_vYellowLightColor);
			m_AliveMesh.TexFlip_SetFlipPage(hLightLayerHandle,(u32)YELLOW);
		}

		m_pWorldLight->m_Light.SetIntensity(m_fCurrentLightAlpha);
		f32 fEmissive = FMATH_FPOT(m_fCurrentLightAlpha, fMinEmissive, fMaxEmissive);
		m_AliveMesh.LayerEmissive_Set(hPanelLayerHandle,fEmissive,fEmissive,fEmissive);
	}
	
	// reposition the light to emanate from camera's position
	CFMtx43 mtxOrientation_WS(	m_pData->m_pMtxAimOrientation_WS->m_vRight.v3,
								m_pData->m_pMtxAimOrientation_WS->m_vUp.v3,
								m_pData->m_pMtxAimOrientation_WS->m_vFront.v3);
	mtxOrientation_WS.m_vRight.Unitize();
	mtxOrientation_WS.m_vFront.Unitize();
	mtxOrientation_WS.m_vUp.Unitize();
	m_pWorldLight->SetWSOverrideVars(&m_pData->m_vAimingOrigin_WS.v3,&mtxOrientation_WS);

#if 0
	CFColorRGBA cyan;cyan.OpaqueCyan();
	CFVec3A Start, End, Dir;
	Start = m_pWorldLight->m_Light.m_Sphere_WS.m_Pos;
	Dir = m_pWorldLight->m_Light.m_mtxOrientation_WS.m_vFront;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&cyan,&cyan);
#endif
}

u32 CFloorSentry::GetTagPointCount( void ) const
{
	return TAG_POINT_COUNT;
}
const CFVec3A *CFloorSentry::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT( nTagPointIndex < TAG_POINT_COUNT );
	return m_apTagPoint_WS[nTagPointIndex];

}

void CFloorSentry::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( (nTrackerSkipListCount + 2) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = &m_AliveMesh;
	apTrackerSkipList[nTrackerSkipListCount++] = &m_DeadMesh;
	if (m_pData->m_pDriverBot)
	{
		m_pData->m_pDriverBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

BOOL CFloorSentry::CanOccupyStation( CBot * pBot)
{
	if (!(pBot->TypeBits() & ENTITY_BIT_BOTGLITCH) )
		return FALSE;

	if (pBot->m_nPossessionPlayerIndex > -1 && pBot->IsInAir())	  //pgm: don't require ai bots to be on the ground please, they're sometimes in the air for stranger reasons that you might think.
		return FALSE;
	
	// Multiplayer: if this gun has been recruited, only let the recruiting player in
	if (MultiplayerMgr.IsMultiplayer() && m_pOwnerBot->Recruit_IsRecruited())
	{
		if (m_pOwnerBot->Recruit_GetRecruiter() != pBot->m_nPossessionPlayerIndex)
			return FALSE;
	}

	if( pBot->m_nPossessionPlayerIndex >= 0 )
	{
		if( CHud2::GetHudForPlayer(pBot->m_nPossessionPlayerIndex)->GetUnitSelfDestruct() > 0.0f )
		{
			// don't allow entry while player is holding "X" to exit possessed bot.
			return FALSE;
		}
	}

	return TRUE;
}

void CFloorSentry::_KillAudioEmitters( void ) 
{
	u32 i;

	for( i=0; i<MAX_SIMULTANEOUS_FIRE_SOUNDS; ++i ) 
	{
		if( m_apAudioEmitterFire[i] ) 
		{
			m_apAudioEmitterFire[i]->Destroy();
			m_apAudioEmitterFire[i] = NULL;
		}
	}
}
