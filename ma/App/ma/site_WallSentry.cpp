//////////////////////////////////////////////////////////////////////////////////////
// site_WallSentry.cpp
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
// 11/05/02 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "MeshTypes.h"
#include "site_WallSentry.h"
#include "weapon.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "FCheckPoint.h"
#include "potmark.h"
#include "explosion.h"
#include "eproj.h"
#include "ftext.h"

static cchar _WALLSENTRYINFO_FILENAME[] = 	"sw_wall";
static cchar _apszSentryMeshFilename[] = "GRMSsentry2";
static cchar _apszSentryMeshDeadFilename[] = "GRMSsent_D2";

CWallSentry::_WallSpewProps_t	CWallSentry::m_pSpewProps;
CDamageProfile*		CWallSentry::m_apDamageProfile;
// This table describes to fgamedata how our user property table is to be interpreted:

const FGameData_TableEntry_t CWallSentry::m_aSpewPropVocab[] = 
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

	// FDecalDefHandle_t hBulletDecal;
	FGAMEDATA_VOCAB_DECAL_DEF,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CWallSentry::m_aGameDataMap[] = {
	"GunProps",
	m_aSpewPropVocab,
	sizeof(CWallSentry::m_pSpewProps),
	(void *)&CWallSentry::m_pSpewProps,

	NULL
};

cchar *CWallSentry::m_apszBoneNameList[BONE_COUNT] = 
{
	"Swivel",	//	BONE_SWIVEL,	
	"Arm_U",	//	BONE_ARM_U,  	
	"Arm_L",	//	BONE_ARM_L,    
	"Torso",	//	BONE_TORSO,	
	"Barrel_L",	//	BONE_BARREL_L,
	"Barrel_R",	//	BONE_BARREL_R,
	"Primary_Fire_L",	//	BONE_PRIMARY_FIRE_L,	
	"Primary_Fire_R"	//	BONE_PRIMARY_FIRE_R,	
};

const u8 CWallSentry::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] = 
{
	BONE_TORSO,
	BONE_SWIVEL,
	BONE_BARREL_L,
	BONE_BARREL_R,
};

cchar *CWallSentry::m_apszBoneNameList2[4] = 
{
	"Swivel",	//	BONE_SWIVEL,	
	"Arm_U",	//	BONE_ARM_U,  	
	"Arm_L",	//	BONE_ARM_L,    
	"Torso",	//	BONE_TORSO,	
};

cchar *CWallSentry::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = 
{
	// Ground locomotion:
	"ARMS2pdwn01",					// ANIM_POWER_DOWN
	"ARMS2cdie01",					// ANIM_DEATH
};

cchar *CWallSentry::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = 
{
	"rest",		//	ANIMCONTROL_REST,
	"powerdn",	//	ANIMCONTROL_POWER_DOWN,
	"death",	//	ANIMCONTROL_DEATH,
	"manual",   //  ANIMCONTROL_AIM_SUMMER,
};


cchar *CWallSentry::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = 
{
	"rest",		//	ANIMTAP_REST,
	"powerdn",	//	ANIMTAP_POWER_DOWN,
	"death"	,   //  ANIMTAP_DEATH
	"manual",   //  ANIMTAP_AIM_SUMMER
};

CFAnimCombinerConfig::ConfigStack_t CWallSentry::m_aAnimCombinerConfigStack[] = 
{
	// From highest to lowest priority:
	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_POWER_DOWN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_DEATH,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_REST,
	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CWallSentry::m_aAnimCombinerConfigTaps[] = 
{
	ANIMCONTROL_REST,			ANIMTAP_REST,				1,
	ANIMCONTROL_POWER_DOWN,		ANIMTAP_POWER_DOWN,			1,
	ANIMCONTROL_DEATH,			ANIMTAP_DEATH,				1,
	ANIMCONTROL_AIM_SUMMER,		ANIMTAP_AIM_SUMMER,			1,
	255,						255,						255
};


CFAnimCombiner::AttachList_t CWallSentry::m_aAnimAttach[] = {
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_REST,
	ANIM_POWER_DOWN,						ANIMCONTROL_POWER_DOWN,
	ANIM_DEATH,								ANIMCONTROL_DEATH,
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_AIM_SUMMER,
	255,									255
};

const u8 CWallSentry::m_aBoneEnableIndices_Death[] = {
	BONE_ARM_U,  	
	BONE_ARM_L,    
	BONE_TORSO,
	255
};


const u8 *CWallSentry::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,									// ANIMTAP_REST,
	NULL,									// ANIMTAP_POWER_DOWN
	m_aBoneEnableIndices_Death,				// ANIMTAP_DEATH,
	NULL,									// ANIMTAP_AIM_SUMMER,
};

const u8 CWallSentry::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};

CBotAnimStackDef CWallSentry::m_DeadAnimStackDef;
CBotAnimStackDef CWallSentry::m_AliveAnimStackDef;

CWallSentry::~CWallSentry()
{

}

void CWallSentry::Destroy(void)
{
	_KillAudioEmitters();
	m_DeadAnim.Destroy();
	m_pOwnerBot->m_Anim.Destroy();
	m_AliveMeshRestAnimSource.Destroy();
	m_DeadMeshRestAnimSource.Destroy();
	m_AliveAnimManMtxAim.Destroy();
	m_DeadAnimManMtxAim.Destroy();
}

BOOL CWallSentry::InitSystem( void )
{
	if (CWallSentry::m_AliveAnimStackDef.IsCreated()==FALSE)
	{
		// Build our animation stack and load animations...
		CBotAnimStackDef::Init_t AnimStackDefInit;
		AnimStackDefInit.nUserCount					= 0; // (my guns don't have user overrides) CBotAnimStackDef::ANIM_USER_COUNT;
		AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
		AnimStackDefInit.apszBaseAnimNameTable		= m_apszBaseAnimNameTable;
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
	if (CWallSentry::m_DeadAnimStackDef.IsCreated()==FALSE)
	{
		// Build our animation stack and load animations...
		CBotAnimStackDef::Init_t AnimStackDefInit;
		AnimStackDefInit.nUserCount					= 0; // (my guns don't have user overrides) CBotAnimStackDef::ANIM_USER_COUNT;
		AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
		AnimStackDefInit.apszBaseAnimNameTable		= m_apszBaseAnimNameTable;
		AnimStackDefInit.nIdleCount					= ANIM_IDLE_COUNT;
		AnimStackDefInit.apszIdleAnimNameTable		= NULL; // m_apszIdleAnimNameTable;
		AnimStackDefInit.nBaseControlCount			= ANIMCONTROL_BASE_COUNT;
		AnimStackDefInit.apszBaseControlNameTable	= m_apszBaseControlNameTable;
		AnimStackDefInit.nBaseTapCount				= ANIMTAP_BASE_COUNT;
		AnimStackDefInit.apszBaseTapNameTable		= m_apszBaseTapNameTable;
		AnimStackDefInit.nBoneCount					= 4;
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
		if( !CWallSentry::m_DeadAnimStackDef.Create( &AnimStackDefInit ) ) 
		{
			goto _ExitWithError;
		}
	}
	
	if( !fgamedata_ReadFileUsingMap( m_aGameDataMap, _WALLSENTRYINFO_FILENAME ) ) 
	{
		DEVPRINTF( "CWallSentryRocket::Create(): Could not create .\n" );
		goto _ExitWithError;
	}
	m_apDamageProfile = CDamage::FindDamageProfile( m_pSpewProps.pszDamageProfile );
	return TRUE;

_ExitWithError:
	return TRUE;
}

void CWallSentry::UninitSystem( void )
{
	m_AliveAnimStackDef.Destroy();
	m_DeadAnimStackDef.Destroy();
}


BOOL CWallSentry::Create( CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 uUserTypeBits )
{
	m_pOwnerBot = (CBot*)pUser;
	m_pData = pData;

	FMeshInit_t MeshInit;

	// actions are how we take over the Bot
	m_pOwnerBot->SetActionable( FALSE );

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
	m_pAliveMesh->SetLineOfSightFlag( FALSE );

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
	m_pDeadMesh->SetLineOfSightFlag( FALSE );

	m_pDeadMesh->UpdateTracker();
	m_pDeadMesh->RemoveFromWorld();

	m_pActiveMesh = &m_AliveMesh;
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

	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_DEATH, 0.0f );
	m_DeadAnim.BaseAnim_SetControlValue(  ANIMCONTROL_DEATH, 0.0f );
	
	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_AIM_SUMMER, 1.0f );
	m_DeadAnim.BaseAnim_SetControlValue(  ANIMCONTROL_AIM_SUMMER, 1.0f );

	m_MtxRecoil = CFMtx43A::m_IdentityMtx;

	m_bFirePrimaryThisFrame = FALSE;
	
	m_nSpewClipAmmo = CWeapon::INFINITE_AMMO;
	m_nSpewReserveAmmo = CWeapon::INFINITE_AMMO;

	m_fSpewUnitShotSpread  = 0.0f;
    m_fSpewSecondsUntilNextTracer  = 0.0f;
	m_fSpewSecondsCountdownTimer  = 0.0f;

	m_fSecsUntilNextSound = 0.0f;

	m_fRecoil0 = 0.0f;
	m_bFireLeftBarrel = TRUE;

	m_fRecoilDistance = .6f;
	m_fRecoilReturnRate = 2 * m_fRecoilDistance * m_pSpewProps.fRoundsPerSec;

	// Find tag points...
	for(s32 i=0; i<TAG_POINT_COUNT; i++ ) 
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

	cchar* pszMuzzleNames[] = 
	{
		"Primary_Fire_L",	//	BONE_PRIMARY_FIRE_L,	
		"Primary_Fire_R"	//	BONE_PRIMARY_FIRE_R,	
	};

	s32 fireBone;
	for (s32 nMuzzle = 0; nMuzzle < 2; nMuzzle++)
	{
		fireBone = m_AliveMesh.FindBone(pszMuzzleNames[nMuzzle]);
		FASSERT(fireBone>=0);
		m_apMuzzlePts[nMuzzle] = m_AliveMesh.GetBoneMtxPalette()[fireBone];
	}
	m_pData->m_pMtxAimOrientation_WS = m_apMuzzlePts[0];

	for(s32 i=0; i<MAX_SIMULTANEOUS_FIRE_SOUNDS; ++i ) 
	{
		m_apAudioEmitterFire[i] = NULL;
	}
	m_nAudioEmitterIndex = 0;

	s32 nTorsoBone		   = m_DeadMesh.FindBone("Torso");
	FASSERT(nTorsoBone>=0);
	m_pTorsoBone = m_DeadMesh.GetBoneMtxPalette()[nTorsoBone];
	return TRUE;
}

void CWallSentry::AddToWorld(void )
{
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
	{
		m_DeadMesh.UpdateTracker(TRUE);
	}
	else
	{
		m_AliveMesh.UpdateTracker(TRUE);
	}
}


void CWallSentry::RemoveFromWorld(void )
{
	m_AliveMesh.RemoveFromWorld();
	m_DeadMesh.RemoveFromWorld();
}

CFMtx43A* CWallSentry::AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName )
{
	return 0;
}

u32	CWallSentry::TriggerWorkPrimary( f32 fUnitTriggerVal1)
{
//	s32 nPlayerIndex;

	m_bFirePrimaryThisFrame = FALSE;
	
	if( fUnitTriggerVal1 < 0.25f ) 
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

			//				if( IsOwnedByPlayer() ) {
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
		m_fSpewUnitShotSpread += FLoop_fPreviousLoopSecs * m_pSpewProps.fShotSpreadSpeed;
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

	if( IsOwnedByPlayer() ) 
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
	return 1;

_ExitWithoutTriggerPressed:
	if( m_fSpewUnitShotSpread > 0.0f ) 
	{
		m_fSpewUnitShotSpread -= FLoop_fPreviousLoopSecs * m_pSpewProps.fShotSpreadRecoverySpeed;
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
u32	CWallSentry::TriggerWorkSecondary( f32 fUnitTriggerVal2)
{
	return 0;
}

void CWallSentry::_SetNewWorldMesh( CFWorldMesh *pSomeMesh )
{
	if (m_pActiveMesh == pSomeMesh)
		return;

	FASSERT(pSomeMesh);
	FASSERT(m_pActiveMesh);

	pSomeMesh->m_Xfm = m_pActiveMesh->m_Xfm;
	m_pActiveMesh->RemoveFromWorld();
	m_pActiveMesh = pSomeMesh;

	m_pActiveMesh->AddToWorld();
	m_pOwnerBot->m_pWorldMesh = m_pActiveMesh;

	for(CEntity* pChild=m_pOwnerBot->GetFirstChild(); pChild; pChild=m_pOwnerBot->GetNextChild(pChild) ) 
	{
		if( pChild->GetAttachBoneName() ) 
		{
			// Child is attached to one of our bones...
			s32 nBoneIndex = pSomeMesh->FindBone( pChild->GetAttachBoneName() );
			if( nBoneIndex >= 0 ) 
			{
				// The same bone exists in the new mesh. Lets use it...
				pChild->m_pParentMtxToWorld = pSomeMesh->GetBoneMtxPalette()[nBoneIndex];
			} 
			else 
			{
				// The same bone doesn't exist in the new mesh. Let's use our model matrix...
				pChild->m_pParentMtxToWorld = &pSomeMesh->m_Xfm.m_MtxF;
			}
		}
	}
}

void CWallSentry::_HandleFireThisFrame(void)
{
	if( m_bFirePrimaryThisFrame ) 
	{

		m_fRecoil0 = -m_fRecoilDistance; // recoil me;

		CFVec3A PerturbVec;
		f32 fJitterFactor, fBestSpread, fWorstSpread;

		fBestSpread = m_pSpewProps.fBestShotSpreadFactor;
		fWorstSpread = m_pSpewProps.fWorstShotSpreadFactor;

		fJitterFactor = FMATH_FPOT( m_fSpewUnitShotSpread, fBestSpread, fWorstSpread );

		PerturbVec.Set(
			fmath_RandomFloatRange( -fJitterFactor, fJitterFactor ),
			fmath_RandomFloatRange( -fJitterFactor, fJitterFactor ),
			fmath_RandomFloatRange( -fJitterFactor, fJitterFactor )
			);

		CFVec3A vFireUnitDir = m_apMuzzlePts[0]->m_vFront;
		vFireUnitDir.Add( PerturbVec );
		vFireUnitDir.Unitize();

		if(fmath_RandomRange(0, 9) == 0)
			m_fSpewSecondsCountdownTimer += m_pSpewProps.fOORoundsPerSec + fmath_RandomFloatRange(0.3f, 0.6f);
		else
			m_fSpewSecondsCountdownTimer += m_pSpewProps.fOORoundsPerSec;
 
		CFVec3A ApproxMuzzlePoint = CFVec3A::m_Null;
		CFVec3A MuzzlePoint;
		s32 nMuzzle = (m_bFireLeftBarrel) ? 0 : 1;
		//for (;;)
		{
			MuzzlePoint = m_apMuzzlePts[nMuzzle]->m_vPos;
			
			f32 fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );

			f32 fDistanceScale = 3.0f;
			f32 fOODistanceMax = .015f;
            fDistanceScale = fOODistanceMax * m_pData->m_fDistanceToTarget;
			FMATH_CLAMPMAX(fDistanceScale, 3.0f);
			fRandomScale += fDistanceScale;

			CMuzzleFlash::AddFlash_CardPosterZ(
				CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
				MuzzlePoint,
				vFireUnitDir,
				m_pSpewProps.fMuzzleWidth_Z * fRandomScale,
				m_pSpewProps.fMuzzleHeight_Z * fRandomScale,
				m_pSpewProps.fMuzzleAlpha_Z
				);
			
			CMuzzleFlash::AddFlash_CardPosterXY(
				CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
				MuzzlePoint,
				m_pSpewProps.fMuzzleScale_XY * fRandomScale,
				m_pSpewProps.fMuzzleAlpha_XY
				);
			
			CMuzzleFlash::AddFlash_Mesh3D(
				CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_3D_STAR_1 + fmath_RandomRange( 0, 1 ) ],
				MuzzlePoint,
				vFireUnitDir,
				m_pSpewProps.fMuzzleScale_Mesh * fRandomScale,
				m_pSpewProps.fMuzzleRot_Mesh
				);
			
			CFVec3A RayEnd;
			FCollImpact_t CollImpact;

			RayEnd.Mul( vFireUnitDir, m_pSpewProps.fMaxLiveRange ).Add( MuzzlePoint );

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

				ImpactFlashPos.Mul( CollImpact.UnitFaceNormal, m_pSpewProps.fMuzzleOffset_Impact ).Add( CollImpact.ImpactPoint );

				if( pCollMaterial->IsImpactFlashEnabled() ) 
				{
					CMuzzleFlash::AddFlash_CardPosterXY(
						CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
						ImpactFlashPos,
						m_pSpewProps.fMuzzleScale_Impact,
						m_pSpewProps.fMuzzleAlpha_Impact
						);
				}

				CEntity* pEntity = CGColl::ExtractEntity( &CollImpact );

				CFDecal::Create( m_pSpewProps.hBulletDecal, &CollImpact, &vFireUnitDir );

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
						pDamageForm->m_pDamageProfile = m_apDamageProfile;
						pDamageForm->m_Damager.pWeapon = NULL;
						pDamageForm->m_Damager.pBot = m_pOwnerBot;
						pDamageForm->m_Damager.nDamagerPlayerIndex = GetPlayerIndex();
						pDamageForm->m_pDamageeEntity = pDamagee;
						pDamageForm->InitTriDataFromCollImpact( pWorldMesh, &CollImpact, &vFireUnitDir );

						CDamage::SubmitDamageForm( pDamageForm );
					}
				}
			}
			BOOL bMakeTracer = FALSE;

			bMakeTracer = fmath_RandomChance( 0.8f );
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
			ApproxMuzzlePoint.Add(MuzzlePoint);
		}
		ApproxMuzzlePoint.Mul(0.50f);
		CMuzzleFlash::MuzzleLight( &ApproxMuzzlePoint, 20.0f );

		// Play firing sound...
		if( m_apAudioEmitterFire[m_nAudioEmitterIndex] ) {
			m_apAudioEmitterFire[m_nAudioEmitterIndex]->Destroy();
			m_apAudioEmitterFire[m_nAudioEmitterIndex] = NULL;
		}

		m_pOwnerBot->PlaySound( CBotSiteWeapon::GetSiteWeaponSndHandle(CBotSiteWeapon::SITESND_FIRE), 1.0f, 1.0f, -1.0f, &m_apAudioEmitterFire[m_nAudioEmitterIndex] );

		if( ++m_nAudioEmitterIndex >= MAX_SIMULTANEOUS_FIRE_SOUNDS ) {
			m_nAudioEmitterIndex = 0;
		}
		
		m_bFireLeftBarrel = !m_bFireLeftBarrel; // toggle muzzles to fire
	}

	if( m_fSpewSecondsCountdownTimer > 0.0f ) 
	{
		m_fSpewSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}

#if 0 
	CFColorRGBA red;red.OpaqueRed();
	CFVec3A Start, End, Dir;
	Start = m_apMuzzlePts[0]->m_vPos;
	Dir = m_apMuzzlePts[0]->m_vFront;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&red,&red);


	CFColorRGBA blue;blue.OpaqueBlue();
	Start = m_apMuzzlePts[1]->m_vPos;
	Dir =m_apMuzzlePts[1]->m_vFront;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&blue,&blue);
#endif
}

void CWallSentry::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	pApproxMuzzlePoint_WS->Zero();
	for (s32 nPt=0;nPt<2;nPt++)
	{
		pApproxMuzzlePoint_WS->Add(m_apMuzzlePts[nPt]->m_vPos);
	}
	pApproxMuzzlePoint_WS->Mul(0.5f);
}


void 	CWallSentry::Work(void)								// my tick
{
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_UP)
	{
		return _PowerUpWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_DOWN)
	{
		return _PowerDownWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
	{
		return _DeathWork();
	}

	if(m_fRecoil0 < 0.0f)
	{
		m_fRecoil0 += m_fRecoilReturnRate * FLoop_fPreviousLoopSecs;
	}
	if(m_fRecoil0 > 0.0f)
	{
		m_fRecoil0 = 0.0f;
	}

	m_AliveAnimManMtxAim.UpdateFrame(BONE_TORSO,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_SWIVEL,m_pData->m_MtxAimY_MS);

	m_MtxRecoil.m_vPos.z	= m_fRecoil0;
	if(!m_bFireLeftBarrel) // set to false after they fire, therefore update this set
		m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_L, m_MtxRecoil);
	else
        m_AliveAnimManMtxAim.UpdateFrame(BONE_BARREL_R, m_MtxRecoil);
	
	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
	ComputeApproxMuzzlePoint_WS(&m_pData->m_vAimingOrigin_WS);

	_HandleFireThisFrame();
}

void 	CWallSentry::_PowerUpWork(void)						// my other tick
{
	m_pOwnerBot->m_Anim.BaseAnim_DeltaTime( ANIMTAP_POWER_DOWN, -FLoop_fPreviousLoopSecs, TRUE); 
	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated

	if (m_pOwnerBot->m_Anim.BaseAnim_GetUnitTime( ANIMTAP_POWER_DOWN) == 0.0f)
	{
		if (m_pData->m_fPitchWS <= 0.0f)
		{
			m_pData->m_fPitchWS = 0.0f;
		}
		else
		{
			m_pData->m_fPitchWS -= 0.01f;
		}
		m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
		m_pData->m_MtxAimY_MS.SetRotationY( (m_pData->m_fYawWS- m_pData->m_fYawAdjustWS2MS) );
		m_AliveAnimManMtxAim.UpdateFrame(BONE_TORSO, m_pData->m_MtxAimX_MS);
		m_AliveAnimManMtxAim.UpdateFrame(BONE_SWIVEL,m_pData->m_MtxAimY_MS);
	}
}

void 	CWallSentry::_PowerDownWork(void)					// my other other tick
{
	if (m_pData->m_fPitchWS < 0.10f)
	{
		m_pData->m_fPitchWS += 0.01f;
		FMATH_CLAMPMAX(m_pData->m_fPitchWS,0.10f);
	}
	else if (m_pData->m_fPitchWS > 0.10f)
	{
		m_pData->m_fPitchWS -= 0.01f;
		FMATH_CLAMPMIN(m_pData->m_fPitchWS,0.10f);
	}

	m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
	m_pData->m_MtxAimY_MS.SetRotationY( (m_pData->m_fYawWS- m_pData->m_fYawAdjustWS2MS) );
	m_AliveAnimManMtxAim.UpdateFrame(BONE_TORSO, m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_SWIVEL,m_pData->m_MtxAimY_MS);

	m_pOwnerBot->m_Anim.BaseAnim_DeltaTime( ANIMTAP_POWER_DOWN, FLoop_fPreviousLoopSecs, TRUE); 

	// means that at 1/3d pt. of animation, blend is at 100%	
	f32 fUnitTime = 3.0f * fmath_UnitLinearToSCurve( m_pOwnerBot->m_Anim.BaseAnim_GetUnitTime( ANIMTAP_POWER_DOWN ) );
	FMATH_CLAMPMAX( fUnitTime, 1.0f );
	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_POWER_DOWN, fUnitTime );
	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
}

void 	CWallSentry::_DeathWork(void)						// my other other other tick
{
	if (m_DeadAnim.BaseAnim_GetUnitTime(ANIMTAP_DEATH) <1.0f)
	{
		if (m_pData->m_fPitchWS <= 0)
		{
			m_pData->m_fPitchWS = 0;
		}
		else
		{
			m_pData->m_fPitchWS -= FLoop_fPreviousLoopSecs;
		}
		m_DeadAnim.BaseAnim_DeltaTime( ANIMTAP_DEATH, FLoop_fPreviousLoopSecs,  TRUE); 
	}
	
	m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
	m_DeadAnimManMtxAim.UpdateFrame(BONE_TORSO, m_pData->m_MtxAimX_MS);
	m_DeadAnim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
}

void 	CWallSentry::CueNewState(void)							
{
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_UP)
	{
		return _CuePowerUp();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_DOWN)
	{
		return _CuePowerDown();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
	{
		return _CueDeath();
	}
}

void 	CWallSentry::_CuePowerUp(void)							// turn me on
{
	
}

void 	CWallSentry::_CuePowerDown(void)		// WHIRRRWhiRRwwwhir.
{
	
}

void 	CWallSentry::_CueDeath(void)			// WHIRRRWhiRRwwwhir.
{
	_SetNewWorldMesh(&m_DeadMesh);
	m_DeadAnim.BaseAnim_SetControlValue( ANIMCONTROL_DEATH, 1.0f );
	m_DeadAnim.BaseAnim_UpdateTime( ANIMCONTROL_DEATH, 0.0f );
	m_DeadAnimManMtxAim.UpdateFrame(BONE_SWIVEL,m_pData->m_MtxAimY_MS);
}


BOOL CWallSentry::CheckpointSave( void )
{
	f32 fAnimTime = m_pOwnerBot->m_Anim.BaseAnim_GetTime(ANIMTAP_POWER_DOWN);
	f32 fControlValue = m_DeadAnim.BaseAnim_GetControlValue(ANIMCONTROL_POWER_DOWN);
	CFCheckPoint::SaveData( (u32&) m_pActiveMesh );
	CFCheckPoint::SaveData( (u32&) fAnimTime );
	CFCheckPoint::SaveData( (u32&) fControlValue );
	return TRUE;
}

void CWallSentry::CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData )
{
	_KillAudioEmitters();
	CFWorldMesh *pWorldMesh;
	f32 fAnimTime;
	f32 fControlValue;
	CFCheckPoint::LoadData( (u32&) pWorldMesh );
	CFCheckPoint::LoadData( (u32&) fAnimTime );
	CFCheckPoint::LoadData( (u32&) fControlValue );
	_SetNewWorldMesh(pWorldMesh);
	m_pOwnerBot->m_Anim.BaseAnim_UpdateTime( ANIMTAP_POWER_DOWN, fAnimTime);
	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_POWER_DOWN, fControlValue);
}

u32 CWallSentry::GetTagPointCount( void ) const
{
	return TAG_POINT_COUNT;
}
const CFVec3A *CWallSentry::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT( nTagPointIndex < TAG_POINT_COUNT );
	return m_apTagPoint_WS[nTagPointIndex];

}

void CWallSentry::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( (nTrackerSkipListCount + 2) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = &m_AliveMesh;
	apTrackerSkipList[nTrackerSkipListCount++] = &m_DeadMesh;
	if (m_pData->m_pDriverBot)
	{
		m_pData->m_pDriverBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

BOOL CWallSentry::CanOccupyStation( CBot * pBot)
{
	// no, you can't get in a wall sentry!
	return FALSE;
}

void CWallSentry::_KillAudioEmitters( void ) 
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