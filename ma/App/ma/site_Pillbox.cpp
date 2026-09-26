//////////////////////////////////////////////////////////////////////////////////////
// site_Pillbox.cpp
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
#include "site_Pillbox.h"
#include "weapon.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "FCheckPoint.h"
#include "potmark.h"
#include "explosion.h"
#include "eproj.h"
#include "ftext.h"
#include "hud2.h"
#include "player.h"
#include "botpart.h"
#include "vehicle.h"
#include "AI\AIBrainman.h"
#include "AI\AIEnviro.h"

static s32 _nRED_LIGHT_ID = 1;
static s32 _nYELLOW_LIGHT_ID = 2;

static cchar _apszPillboxInfo_Filename[]	= "sw_pillbox";
static cchar _apszSentryMeshFilename[]		= "GOM_Pboxa00";
static cchar _apszSentryMeshDeadFilename[]	= "GOM_Pboxad1";
static cchar _apszSearchLightMeshFilename[] = "GRMGlite_01";
static cchar _apszShellMeshFilename[]		= "gwmgshell01";

static cchar _BOTPART_FILENAME[]			= "bp_pillbox";
static s32   _PART_INSTANCE_COUNT_PER_TYPE	= 1;
static s32	 _LIMB_TYPE_COUNT				= 1;
static CFVec3A m_vCameraOffset;

CPillbox::_PillboxSpewProps_t	CPillbox::m_aSpewProps[2];

CDamageProfile*			CPillbox::m_aDamageProfile[2];
CFDebrisGroup*			CPillbox::m_pDebrisGroup;
FMesh_t*				CPillbox::m_pShellMesh;
CFDebrisMesh			CPillbox::m_DebrisShellMesh;
CBotPartPool*			CPillbox::m_pPartPool;

// This table describes to fgamedata how our user property table is to be interpreted:

const FGameData_TableEntry_t CPillbox::m_aSpewPropVocab[] =
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

	// FDecalDefHandle_t hBulletDecal;
	FGAMEDATA_VOCAB_DECAL_DEF,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameDataMap_t CPillbox::m_aGameDataMap[] = {
	"GunPropsMil",
	m_aSpewPropVocab,
	sizeof(_PillboxSpewProps_t),
	(void *)&CPillbox::m_aSpewProps[0],

	"GunPropsPossessed",
	m_aSpewPropVocab,
	sizeof(_PillboxSpewProps_t),
	(void *)&CPillbox::m_aSpewProps[1],

	NULL
};

cchar *CPillbox::m_apszBoneNameListAlive[BONE_COUNT] =
{
	"Pillbox_Dummy",
	"Base",
	"Swivel",
	"searchlight",
	"Gun",
	"gun barrel",
	"Primary_Fire",
	"AttachPoint_Bot",
	"AttachPoint_Light",
};

const u8 CPillbox::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] = 
{
	BONE_BASE,
	BONE_SWIVEL,
	BONE_SEARCHLIGHT,
	BONE_GUN_BARREL,
};

cchar *CPillbox::m_apszBoneNameListDead[1] =
{
	"Base",
};

cchar *CPillbox::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = {
	"rest",		//	ANIMCONTROL_REST,
	"manual",   //  ANIMCONTROL_AIM_SUMMER,
};


cchar *CPillbox::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = {
	"rest",		//	ANIMTAP_REST,
	"manual",   //  ANIMTAP_AIM_SUMMER
};

CFAnimCombinerConfig::ConfigStack_t CPillbox::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_REST,
	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CPillbox::m_aAnimCombinerConfigTaps[] = {
	ANIMCONTROL_REST,			ANIMTAP_REST,				1,
	ANIMCONTROL_AIM_SUMMER,		ANIMTAP_AIM_SUMMER,			1,
	255,						255,						255
};


CFAnimCombiner::AttachList_t CPillbox::m_aAnimAttach[] = {
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_REST,
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_AIM_SUMMER,
	255,									255
};

const u8 *CPillbox::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	NULL,									// ANIMTAP_REST,
	NULL,									// ANIMTAP_AIM_SUMMER,
};

const u8 CPillbox::m_anEnableBoneNameIndexTableForSummer_Normal[] = {
	// Enable these bones to be driven by our summer:
	255
};

CBotAnimStackDef CPillbox::m_DeadAnimStackDef;
CBotAnimStackDef CPillbox::m_AliveAnimStackDef;

CPillbox::~CPillbox()
{

}

void CPillbox::Destroy(void)
{
	_KillAudioEmitters();
	m_pOwnerBot->m_Anim.Destroy();
	m_DeadAnim.Destroy();
	m_AliveMeshRestAnimSource.Destroy();
	m_DeadMeshRestAnimSource.Destroy();
	m_AliveAnimManMtxAim.Destroy();
	m_DeadAnimManMtxAim.Destroy();
}

BOOL CPillbox::InitSystem( void )
{
	if (CPillbox::m_AliveAnimStackDef.IsCreated()==FALSE)
	{
		// Build our animation stack and load animations...
		CBotAnimStackDef::Init_t AnimStackDefInit;
		AnimStackDefInit.nUserCount					= 0; // (my guns don't have user overrides) CBotAnimStackDef::ANIM_USER_COUNT;
		AnimStackDefInit.nBaseAnimNameCount			= ANIM_BASE_COUNT;
		AnimStackDefInit.apszBaseAnimNameTable		= NULL;  // m_apszBaseAnimNameTable;
		AnimStackDefInit.nIdleCount					= ANIM_IDLE_COUNT;
		AnimStackDefInit.apszIdleAnimNameTable		= NULL; // m_apszIdleAnimNameTable;
		AnimStackDefInit.nBaseControlCount			= ANIMCONTROL_BASE_COUNT;
		AnimStackDefInit.apszBaseControlNameTable	= m_apszBaseControlNameTable;
		AnimStackDefInit.nBaseTapCount				= ANIMTAP_BASE_COUNT;
		AnimStackDefInit.apszBaseTapNameTable		= m_apszBaseTapNameTable;
		AnimStackDefInit.nBoneCount					= BONE_COUNT;
		AnimStackDefInit.apszBoneNameTable			= m_apszBoneNameListAlive;

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
	if (CPillbox::m_DeadAnimStackDef.IsCreated()==FALSE)
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
		AnimStackDefInit.nBoneCount					= 1;
		AnimStackDefInit.apszBoneNameTable			= m_apszBoneNameListDead;

		AnimStackDefInit.ppnEnableBoneNameIndexTableForEachBaseTap	= m_apnEnableBoneNameIndexTableForEachBaseTap;
		AnimStackDefInit.ppnEnableBoneNameIndexTableForEachIdleTap	= NULL;// ?? m_apnEnableBoneNameIndexTableForEachIdleTap;
		AnimStackDefInit.pnEnableBoneNameIndexTableForSummer		= m_anEnableBoneNameIndexTableForSummer_Normal;

		AnimStackDefInit.pBaseAnimConfigNet					= NULL;
		AnimStackDefInit.pBaseAnimConfigStack				= m_aAnimCombinerConfigStack;
		AnimStackDefInit.pBaseAnimConfigTap					= m_aAnimCombinerConfigTaps;
		AnimStackDefInit.pBaseAnimAttachList				= m_aAnimAttach;
		AnimStackDefInit.nIdleStackConnectionControlIndex	= ANIMCONTROL_REST;
		AnimStackDefInit.nIdleStackConnectionControlInput	= 0;
		if( !CPillbox::m_DeadAnimStackDef.Create( &AnimStackDefInit ) )
		{
			goto _ExitWithError;
		}
	}

#if FANG_WINGC
	if( !CBotSiteWeapon::ReadRetailGunTables( m_aGameDataMap, m_aSpewPropVocab, _apszPillboxInfo_Filename ) )
#else
	if( !fgamedata_ReadFileUsingMap( m_aGameDataMap, _apszPillboxInfo_Filename) )
#endif
	{
		DEVPRINTF( "CPillboxRocket::Create(): Could not create .\n" );
		goto _ExitWithError;
	}
	
	m_aDamageProfile[0] = CDamage::FindDamageProfile( m_aSpewProps[0].pszDamageProfile );
	m_aDamageProfile[1] = CDamage::FindDamageProfile( m_aSpewProps[1].pszDamageProfile );

	m_pShellMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE,_apszShellMeshFilename);
	if( m_pShellMesh == NULL ) 
	{
			DEVPRINTF( "CPillbox::InitSystem(): Could not load shell mesh '%s'.\n", _apszShellMeshFilename );
			goto _ExitWithError;
	}
	m_DebrisShellMesh.m_pMesh = m_pShellMesh;
	m_DebrisShellMesh.m_fScale = 1.0f;		
	m_DebrisShellMesh.m_nFlags = CFDebrisMesh::FLAG_CAN_KICKUP_DUST;
	m_DebrisShellMesh.m_nTintRed = 255;
	m_DebrisShellMesh.m_nTintGreen = 255;
	m_DebrisShellMesh.m_nTintBlue = 255;

	m_pDebrisGroup = CFDebrisGroup::Find("SiteWeaponShell");
	return TRUE;

_ExitWithError:
	return TRUE;
}

void CPillbox::UninitSystem( void )
{
	m_AliveAnimStackDef.Destroy();
	m_DeadAnimStackDef.Destroy();
}


BOOL CPillbox::Create( CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 uUserTypeBits )
{
	m_pOwnerBot = (CBot*)pUser;
	m_pData = pData;

	FMeshInit_t MeshInit;

	m_pSpewProps=&m_aSpewProps[0];
	m_pDamageProfile=m_aDamageProfile[0];

	// actions are how we take over the Bot
	m_pOwnerBot->SetActionable( TRUE );

	// Load mesh resource...
	FMesh_t		*pAliveMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszSentryMeshFilename);
	if( pAliveMesh == NULL )
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyBuild(): Could not find mesh '%s'\n", _apszSentryMeshFilename );
		return FALSE;
	}
	// Init the world mesh...
	MeshInit.pMesh		= pAliveMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( m_pData->m_MtxPosition_WS );
	m_AliveMesh.Init( &MeshInit );
	m_AliveMesh.m_nUser = MESHTYPES_ENTITY;
	m_AliveMesh.m_pUser = pUser;
	m_AliveMesh.SetUserTypeBits( uUserTypeBits );
	m_AliveMesh.m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_AliveMesh.SetCollisionFlag( TRUE );
	m_AliveMesh.SetLineOfSightFlag( TRUE );

	m_AliveMesh.m_BoundSphere_MS.m_Pos = m_AliveMesh.m_BoundSphere_MS.m_Pos;
	m_AliveMesh.m_BoundSphere_MS.m_fRadius *= 1.15f;

    m_AliveMesh.UpdateTracker();
	m_AliveMesh.RemoveFromWorld();

	FMesh_t		*pDeadMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszSentryMeshDeadFilename );
	if( pDeadMesh == NULL )
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyBuild(): Could not find mesh '%s'\n", _apszSentryMeshDeadFilename );
		return FALSE;
	}
	// Init the dead world mesh...
	MeshInit.pMesh		= pDeadMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist  = FMATH_MAX_FLOAT;
	MeshInit.Mtx.Set( m_pData->m_MtxPosition_WS );
	m_DeadMesh.Init( &MeshInit );
	m_DeadMesh.m_nUser = MESHTYPES_ENTITY;
	m_DeadMesh.m_pUser = pUser;
	m_DeadMesh.SetUserTypeBits( uUserTypeBits );
	m_DeadMesh.m_nFlags &= ~(FMESHINST_FLAG_DONT_DRAW | FMESHINST_FLAG_NOCOLLIDE);
	m_DeadMesh.SetCollisionFlag( TRUE );
	m_DeadMesh.SetLineOfSightFlag( TRUE );
	m_DeadMesh.UpdateTracker();
	m_DeadMesh.RemoveFromWorld();

	m_pAliveMesh = &m_AliveMesh;
	m_pDeadMesh = &m_DeadMesh;
	
	m_pOwnerBot->m_pWorldMesh = &m_AliveMesh;

	if( !m_pOwnerBot->m_Anim.Create( &m_AliveAnimStackDef, &m_AliveMesh ) )
	{
		return FALSE;
	}

	if( !m_DeadAnim.Create( &m_DeadAnimStackDef, &m_DeadMesh ) )
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

	m_fSecsUntilNextSound = 0.0f;

	m_fRecoil0 = 0.0f;

	m_fRecoilDistance = 0.75f;
	m_fRecoilReturnRate = 2.0f * m_fRecoilDistance * m_pSpewProps->fRoundsPerSec;

	s32 i;
	// Find tag points...
	for( i=0; i<TAG_POINT_COUNT; i++ ) 
	{
		s32 nBoneIndex = m_AliveMesh.FindBone( m_apszBoneNameListAlive[ m_anTagPointBoneNameIndexArray[i] ] );
		if( nBoneIndex < 0 ) 
		{
			DEVPRINTF( "CBotSiteWeapons::ClassHierarchyBuild(): Could not locate TagPoint bone '%s'.\n", m_apszBoneNameListAlive[ m_anTagPointBoneNameIndexArray[i] ] );
		} else 
		{
			m_apTagPoint_WS[i] = &m_AliveMesh.GetBoneMtxPalette()[nBoneIndex]->m_vPos;
		}
	}
	
	for(s32 i=0; i<MAX_SIMULTANEOUS_FIRE_SOUNDS; ++i ) 
	{
		m_apAudioEmitterFire[i] = NULL;
	}
	m_nAudioEmitterIndex = 0;


	s32 fireBone = m_AliveMesh.FindBone("Primary_Fire");
	FASSERT(fireBone>=0);
	m_pMuzzlePtArray[0] = m_AliveMesh.GetBoneMtxPalette()[fireBone];
	m_pData->m_pMtxAimOrientation_WS = m_pMuzzlePtArray[0];

//	s32 nBarrelBone = m_AliveMesh.FindBone("gun barrel");
	s32 nBarrelBone = m_AliveMesh.FindBone("gun");
	FASSERT(nBarrelBone>=0);
	pmtxCameraAttachPtWS = m_AliveMesh.GetBoneMtxPalette()[nBarrelBone];
	
	m_nAttachBotPtBoneIdx = m_AliveMesh.FindBone("AttachPoint_Bot");
	FASSERT(m_nAttachBotPtBoneIdx>=0);

	m_nShellEjectPtBoneIdx = m_AliveMesh.FindBone("Shell_Eject_L");
	FASSERT(m_nShellEjectPtBoneIdx>=0);

	// find the search light matrix
	s32 nSeachLightBone = m_AliveMesh.FindBone("AttachPoint_Light");
	FASSERT(nSeachLightBone>=0);
	m_pSearchLightMtx = m_AliveMesh.GetBoneMtxPalette()[nSeachLightBone];
	
//	if (!m_LightMeshEntity.Create(_apszSearchLightMeshFilename))
//	{
//		DEVPRINTF( "CBotPillbox::ClassHierarchyBuild(): Could not create spotlight\n" );
//		return FALSE;
//	}
//	m_LightMeshEntity.SetMeshInstFlags( FMESHINST_FLAG_POSTER_Y);
//	m_LightMeshEntity.RemoveFromWorld(); // is added later

	CFWorldAttachedLight* pRedWorldLight = m_AliveMesh.GetAttachedLightByID(_nRED_LIGHT_ID);
	CFWorldAttachedLight* pYellowWorldLight = m_AliveMesh.GetAttachedLightByID(_nYELLOW_LIGHT_ID);
	if ( !pRedWorldLight || !pYellowWorldLight )
	{
		FASSERT(!"You should be able to ignore this assert. But your pillbox mesh is broken. light missing");
		return FALSE;
	}
	pRedWorldLight->UpdateTracker();
	pYellowWorldLight->UpdateTracker();
	pYellowWorldLight->m_Light.m_Motif.ComputeColor(&m_vYellowLightColor);
	pRedWorldLight->m_Light.m_Motif.ComputeColor(&m_vRedLightColor);

	pRedWorldLight->m_Light.Enable(FALSE);
	pYellowWorldLight->m_Light.Enable(FALSE);
	m_pWorldLight =	pRedWorldLight;
	m_pWorldLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION );
	m_pWorldLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_POSITION );
	if (m_pData->m_fLightRadiusOverride >= 0.0f)
	{
		m_pWorldLight->m_Light.m_spInfluence_WS.m_fRadius = m_pData->m_fLightRadiusOverride;
	}

	m_vCameraOffset.x = 0.f;
	m_vCameraOffset.y = 1.5f;
	m_vCameraOffset.z = 1.0f;

	m_fCurrentLightAlpha=1.0f;

	m_ePreviousState = m_pData->m_eSiteWeaponState;
	m_bPossessed = FALSE;

	m_fIdlingVelocity					= 0;
	m_fIdlingVelocityDamp				= .50f;						// how much velocity is left after hitting an extrema
	m_fIdlingAcceleration				= 4.0f;					    // downward pull from gravity
	fforce_NullHandle( &m_hForce );

	// set up part mgr...
	if( !m_pOwnerBot->m_pPartMgr->Create( m_pOwnerBot, &m_pPartPool, _BOTPART_FILENAME, _PART_INSTANCE_COUNT_PER_TYPE, _LIMB_TYPE_COUNT ) ) 
	{
		DEVPRINTF("problem aprsing file %s", _BOTPART_FILENAME);
	}

	return TRUE;
}

void CPillbox::AddToWorld(void )
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

void CPillbox::RemoveFromWorld(void )
{
	m_AliveMesh.RemoveFromWorld();
	m_DeadMesh.RemoveFromWorld();
}

CFMtx43A* CPillbox::AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName )
{
	return 0;
}

u32	CPillbox::TriggerWorkPrimary( f32 fUnitTriggerVal1 )
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
u32	CPillbox::TriggerWorkSecondary( f32 fUnitTriggerVal2 )
{
	return 0;
}

void CPillbox::_DeadMeshDraw( BOOL bDrawDeadMesh)
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

void CPillbox::_HandleFireThisFrame(void)
{
	if( m_bFirePrimaryThisFrame )
	{
		if (m_fRecoil0 == 0.0f) // top barrel ready to fire
		{
			m_fRecoil0 = -m_fRecoilDistance; // recoil me;
		}

		FASSERT(m_pData->m_pDriverBot);
		CFVec3A vFireUnitDir = m_pMuzzlePtArray[0]->m_vFront;

		if( IsOwnedByPlayer() )
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

		CFVec3A MuzzlePoint;
		MuzzlePoint = m_pMuzzlePtArray[0]->m_vPos;

		_EjectShell();

		f32 fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );
		if (!IsOwnedByPlayer())
		{
			f32 fDistanceScale = 3.0f;
			f32 fOODistanceMax = .015f;
            fDistanceScale = fOODistanceMax * m_pData->m_fDistanceToTarget;
			FMATH_CLAMPMAX(fDistanceScale, 3.0f);
			fRandomScale += fDistanceScale;
		}

		CMuzzleFlash::AddFlash_CardPosterZ(
			CWeapon::m_ahMuzzleFlashGroup[ CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
			MuzzlePoint,
			vFireUnitDir,
			m_pSpewProps->fMuzzleWidth_Z * fRandomScale,
			m_pSpewProps->fMuzzleHeight_Z * fRandomScale,
			m_pSpewProps->fMuzzleAlpha_Z
			);

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

		CMuzzleFlash::MuzzleLight( &MuzzlePoint, 20.0f );
		if( IsOwnedByPlayer() )
		{
			fforce_Kill( &m_hForce );
			fforce_Play( Player_aPlayer[GetPlayerIndex()].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MAX, &m_hForce );
		}
		
		m_fSpewSecondsCountdownTimer += m_pSpewProps->fOORoundsPerSec;
		//		RemoveFromClip(1);

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
					pDamageForm->m_pDamageProfile = m_pDamageProfile;
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

		if(IsOwnedByPlayer()==FALSE) 
		{
			// AI...

			bMakeTracer = fmath_RandomChance( 0.8f );
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
	}
	if( m_fSpewSecondsCountdownTimer > 0.0f )
	{
		m_fSpewSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}
}

void CPillbox::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	*pApproxMuzzlePoint_WS = m_pMuzzlePtArray[0]->m_vPos;
}

void CPillbox::_ComputeCameraMatrix_WS( void )
{
	// SER: These three lines are all that are needed. Why waste a ton of cycles on
	//      doing a bunch of vector math and two expensive atans???
	m_pData->m_MtxCam_WS.UnitMtxFromUnitVec( &pmtxCameraAttachPtWS->m_vZ );
	m_pData->m_MtxCam_WS.m_vPos = pmtxCameraAttachPtWS->m_vPos;
	m_pData->m_MtxCam_WS.MulPoint( m_pData->m_MtxCam_WS.m_vPos, m_vCameraOffset );

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

void 	CPillbox::Work(void)								// my tick
{
	m_fNormHealth = m_pOwnerBot->NormHealth();
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_IDLING)
	{
		return _IdlingWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_DOWN)
	{
		return _PowerDownWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
	{
		return _DeathWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_ENTERING_POSSESSED)
	{
		return _EnteringPossessedWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWERING_UP_POSSESSED)
	{
		_PossessedPowerUpWork();
	}

	if(m_fRecoil0 < 0.0f)
	{
		m_fRecoil0 += m_fRecoilReturnRate * FLoop_fPreviousLoopSecs;
	}
	if(m_fRecoil0 > 0.0f)
	{
		m_fRecoil0 = 0.0f;
	}
	m_MtxRecoil.m_vPos.z	= m_fRecoil0;
	m_AliveAnimManMtxAim.UpdateFrame(BONE_GUN_BARREL, m_MtxRecoil);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_GUN,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_SWIVEL,m_pData->m_MtxAimY_MS);
	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();

//	m_pWorldLight->UpdateTracker(m_pSearchLightMtx);
	
	// move to attach point
	CFVec3A vAttachPtBot = (m_AliveMesh.GetBoneMtxPalette()[m_nAttachBotPtBoneIdx])->m_vPos;
	CFMtx43A mtxAttachBot;
	mtxAttachBot = *m_pData->m_pMtxAimOrientation_WS;
	mtxAttachBot.m_vPos = vAttachPtBot;
	FASSERT(m_pData->m_pDriverBot);
	m_pData->m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS(&mtxAttachBot,FALSE,this);

#if 0
	CFColorRGBA red;red.OpaqueRed();
	CFVec3A Start, End, Dir;
	Start = m_pMuzzlePtArray[0]->m_vPos;
	Dir =m_pMuzzlePtArray[0]->m_vFront;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&red,&red);
#endif
	
	ComputeApproxMuzzlePoint_WS(&m_pData->m_vAimingOrigin_WS);	// update gun's position
	if (m_bPossessed)
	{
		_ComputeCameraMatrix_WS();								// update camera's matrix
		if (m_pData->m_pDriverBot->IsCapableOfAimingPB())
		{
			f32 fPitchRange = m_pData->m_fMaximumPitchWS - m_pData->m_fMinimumPitchWS;
            f32 fPitchPercent = fmath_Div((m_pData->m_fPitchWS - m_pData->m_fMinimumPitchWS),fPitchRange);
			m_pData->m_pDriverBot->UpdateTime(CBot::ASI_AIM_PILLBOX, 1.0f - fPitchPercent);
		}
		
	}
	_HandleFireThisFrame();
	_UpdateLight();
}

void 	CPillbox::_IdlingWork(void)						// my other tick
{
	if ( (FMATH_FABS(m_fIdlingVelocity) > 0.05) ||	// keep going as long as velocity is big
		(m_pData->m_fPitchWS < m_pData->m_fMaximumPitchWS) )				//	or going up
	{
		m_pData->m_fPitchWS = m_pData->m_fPitchWS + ((m_fIdlingVelocity) * FLoop_fPreviousLoopSecs);
		if (m_pData->m_fPitchWS >= m_pData->m_fMaximumPitchWS) // bounce bottom
		{
			m_pData->m_fPitchWS = m_pData->m_fMaximumPitchWS;
			m_fIdlingVelocity *= (-m_fIdlingVelocityDamp);
		}
		m_fIdlingVelocity += (m_fIdlingAcceleration * FLoop_fPreviousLoopSecs);
		m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
		m_AliveAnimManMtxAim.UpdateFrame(BONE_GUN,  m_pData->m_MtxAimX_MS);
		m_AliveAnimManMtxAim.UpdateFrame(BONE_SWIVEL,m_pData->m_MtxAimY_MS);
		m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
		m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
	}
}


void 	CPillbox::_EnteringPossessedWork(void)
{
	if(IsOwnedByPlayer() == TRUE) // don't play anim in possessed cases
		return;
	
	FASSERT(m_pData->m_fTimeToEnter > 0.0f);
	f32 fBlendPercent = fmath_Div(m_pData->m_fTimeSinceStateChanged,m_pData->m_fTimeToEnter);
	FASSERT(m_pData->m_pDriverBot->IsCapableOfAimingPB());

	m_pData->m_pDriverBot->SetControlValue( CBot::ASI_AIM_PILLBOX, fBlendPercent );

	CFVec3A vAttachPtBot = (m_AliveMesh.GetBoneMtxPalette()[m_nAttachBotPtBoneIdx])->m_vPos;
	CFMtx43A mtxAttachBot;
	mtxAttachBot = *m_pData->m_pMtxAimOrientation_WS;
	mtxAttachBot.m_vPos = vAttachPtBot;
	FASSERT(m_pData->m_pDriverBot);
	m_pData->m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS(&mtxAttachBot,FALSE,this);

	f32 fPitchRange = m_pData->m_fMaximumPitchWS - m_pData->m_fMinimumPitchWS;
    f32 fPitchPercent = fmath_Div((m_pData->m_fPitchWS - m_pData->m_fMinimumPitchWS),fPitchRange);
	m_pData->m_pDriverBot->UpdateTime(CBot::ASI_AIM_PILLBOX, 1.0f - fPitchPercent);
}

void 	CPillbox::_PossessedPowerUpWork(void)			// my other tick
{
	if (m_pData->m_fPitchWS <= 0.0f)
	{
		m_pData->m_fPitchWS = 0.0f;
	}
	else
	{
		m_pData->m_fPitchWS -= FLoop_fPreviousLoopSecs;
	}

	m_pData->m_MtxAimY_MS.SetRotationY(m_pData->m_fYawWS-m_pData->m_fYawAdjustWS2MS);
	m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_GUN,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_SWIVEL,m_pData->m_MtxAimY_MS);
//	m_pWorldLight->UpdateTracker(m_pSearchLightMtx);

	f32 fPercentStateChanged = fmath_Div(m_pData->m_fTimeSinceStateChanged,m_pData->m_fTimeToPowerUp);
	FMATH_CLAMPMAX(fPercentStateChanged,1.0f);
	m_fCurrentLightAlpha = fPercentStateChanged;
}

void 	CPillbox::_PowerDownWork(void)					// my other other tick
{
}

void 	CPillbox::_DeathWork(void)						// my other other other tick
{
}

void 	CPillbox::CueNewState(void)							
{
	if (m_ePreviousState == CBotSiteWeapon::SITE_ENTERING_POSSESSED)
	{
		if (m_pData->m_pDriverBot->IsInWorld())
		{
			if (m_pData->m_pDriverBot->IsPlayerBot())
			{
				m_pData->m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
				m_pData->m_pDriverBot->SetNoCollideStateAir( FALSE );
			}

			if ( (m_pData->m_eSiteWeaponState != CBotSiteWeapon::SITE_POWERING_UP_POSSESSED) &&
				 (m_pData->m_eSiteWeaponState != CBotSiteWeapon::SITE_POSSESSED) )
			{
				FASSERT(!m_bPossessed); // not possessed yet, so won't miss the ptr

				m_pData->m_pDriverBot = NULL; // we were entering, so we have the pointer,
				// but you can't keep it, entering was interrupted
			}
		}
	}
	
	m_ePreviousState = m_pData->m_eSiteWeaponState;
	
	switch (m_pData->m_eSiteWeaponState)
	{
	case CBotSiteWeapon::SITE_IDLING:
		if (m_bPossessed)
		{
			_CuePossessed(FALSE);
		}
		m_fIdlingVelocity = 0;
		return;
	case CBotSiteWeapon::SITE_POWER_UP:
		return _CuePowerUp();
	case CBotSiteWeapon::SITE_POWER_DOWN:
		return _CuePowerDown();
	case CBotSiteWeapon::SITE_DEAD:
		if (m_bPossessed)
		{
			_CuePossessed(FALSE);
		}
		return _CueDeath();
	case CBotSiteWeapon::SITE_POWERING_UP_POSSESSED:
		return _CuePossessed(TRUE);
	case CBotSiteWeapon::SITE_ENTERING_POSSESSED:
		if (m_pData->m_pDriverBot->IsPlayerBot())
		{
			CFVec3A vJumpVel;
			CFVec3A vEntryPt;
			BOOL bOK = ((CBotSiteWeapon*)m_pOwnerBot)->GetStationEntryPoint(&vEntryPt);
			FASSERT(bOK);

			m_pData->m_fTimeToEnter = CVehicle::ComputeEntryTrajectory( m_pData->m_pDriverBot->MtxToWorld()->m_vPos,	// start pos
														vEntryPt,									// end pos							
														3.0f,								// minimum jump height, 
														m_pData->m_pDriverBot->m_pBotInfo_Gen->fGravity * m_pData->m_pDriverBot->m_fGravityMultiplier,	// gravity
														vJumpVel );							// output velocity

			m_pData->m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
			m_pData->m_pDriverBot->SetNoCollideStateAir( TRUE );
			m_pData->m_pDriverBot->StartVelocityJump( &vJumpVel );
			return;
		}
		else
		{
			m_pData->m_fTimeToEnter = 0.5f;
		}
	case CBotSiteWeapon::SITE_POSSESSED:
		{
			s32 nIdx = GetPlayerIndex();
			if (nIdx > -1)
			{
				CReticle *pReticle = &Player_aPlayer[ nIdx ].m_Reticle;
				pReticle->EnableDraw(TRUE, TRUE);
			}
			m_fCurrentLightAlpha=1.0f;
		}
		break;
	}
}

void 	CPillbox::_CuePowerUp(void)							// turn me on
{
}

void 	CPillbox::_CuePowerDown(void)				// WHIRRRWhiRRwwwhir.
{
}

void 	CPillbox::_CueDeath(void)					// WHIRRRWhiRRwwwhir.
{
	_Light(FALSE);
	_DeadMeshDraw( TRUE );
}

void 	CPillbox::_CuePossessed(BOOL bGoPosed)		// transform!() 
{
	FASSERT(m_pOwnerBot->m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_PILLBOX);
	if (bGoPosed)
	{
		if (m_bPossessed) // already did this
			return;
		FASSERT(m_pData->m_pDriverBot);
		// turn the light on
		_Light(TRUE);

		// control the gun
		m_pOwnerBot->SetControls( m_pData->m_pDriverBot->Controls());
		
		
		// attach to point
 		m_pData->m_pDriverBot->Attach_ToParent_WithGlue_WS( m_pOwnerBot,"Swivel");
		m_pData->m_pDriverBot->SetControls(NULL);
//		m_pData->m_pDriverBot->EnteringMechWork(m_pOwnerBot);

		if (m_pData->m_pDriverBot->IsCapableOfAimingPB())
		{
			m_pData->m_pDriverBot->SetControlValue( CBot::ASI_AIM_PILLBOX, 1.0f ); // full strength, in case of no blend case...
		}
		m_bPossessed = TRUE;
		s32 nIdx = GetPlayerIndex();
		if (nIdx > -1)
		{
			CReticle *pReticle = &Player_aPlayer[ nIdx ].m_Reticle;
			CHud2* pHud = CHud2::GetHudForPlayer(nIdx);

			// remember previous values
			m_fOldNormX = pReticle->GetNormOriginX();
			m_fOldNormY = pReticle->GetNormOriginY();
			m_OldHudMode = pHud->m_eCurHudMode;
			m_OldReticleType = pReticle->GetType();
			pReticle->SetNormOrigin(0.0f,0.0f);
			pReticle->SetType( CReticle::TYPE_FLOORSENTRY );
			pReticle->SetUnitScale(0.0f);
			pReticle->SetReticleOverride();
			pReticle->EnableDraw(FALSE, TRUE);

			m_uOldHudFlags = pHud->SetDrawFlags(CHud2::DRAW_ENABLEOVERRIDE | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR);
			CFColorRGBA startColor, endColor;

			startColor.Set( 1.0f, 1.0f, 0.0f, 0.5f );
			endColor.Set( 0.0f, 1.0f, 0.0f, 0.5f );

			//pHud->OverrideHealthBar(TRUE,&m_fNormHealth,(u32)m_pOwnerBot->HealthContainerCount(), startColor, endColor );
			m_pSpewProps=&m_aSpewProps[1];
			m_pDamageProfile=m_aDamageProfile[1];
			
			_ComputeCameraMatrix_WS();
			gamecam_SwitchPlayerToManualCamera(PLAYER_CAM(nIdx),&m_pData->m_CamManInfo);

			if (m_pOwnerBot->AIBrain())
			{
				//when players take "possession" of bots with brains, call this
				//because the brain will be repurposed during that time.
				//see exit vehicle for the undoing of this.
				aibrainman_ConfigurePlayerBotBrain( m_pOwnerBot->AIBrain(), m_pOwnerBot->m_nPossessionPlayerIndex );	 //turns this bot's brain into a player brain
			}
		}
	}
	else
	{
		if (m_pData->m_pDriverBot &&  //pgm added this to stop a null access. don't care to know why it happened.
			m_pData->m_pDriverBot->IsInWorld()) // now, can get called on quit, and driver gets popped first
		{
			s32 nIdx = GetPlayerIndex();
			if (nIdx > -1)
			{
				gamecam_SwitchPlayerTo3rdPersonCamera(PLAYER_CAM(nIdx),m_pData->m_pDriverBot);

				m_pSpewProps=&m_aSpewProps[0];
				m_pDamageProfile=m_aDamageProfile[0];

				CReticle *pReticle = &Player_aPlayer[ nIdx ].m_Reticle;
				pReticle->SetNormOrigin(m_fOldNormX,m_fOldNormY);
				pReticle->SetType(m_OldReticleType);

				CHud2* pHud = CHud2::GetHudForPlayer(nIdx);
				pHud->SetHudMode( m_OldHudMode );
				pHud->SetDrawFlags( m_uOldHudFlags );
				pReticle->ClearReticleOverride();
				pReticle->SetUnitScale(0.0f);
				pReticle->SetType(CReticle::TYPE_NONE);
				pReticle->EnableDraw(FALSE);
			}
			// turn the light off
			_Light(FALSE);

			CEntityControl* pCtl = m_pOwnerBot->Controls();
			m_pOwnerBot->SetControls( NULL ); 
//			m_pData->m_pDriverBot->ExitingMechWork();

			if (IsOwnedByPlayer())
			{
				CPlayer *pPlayer = &Player_aPlayer[ GetPlayerIndex() ];
				pPlayer->ZeroControls();
				aibrainman_ConfigurePostPossessionBotBrain(m_pOwnerBot->AIBrain());
			}
//			if (!m_pData->m_pDriverBot->AIBrain()->GetFlag_Active()) // fix for checkpoint restore during possessed grunt possession
			if (!aibrainman_IsActive(m_pData->m_pDriverBot->AIBrain())) //pgm switch to this function 5/3
			{
				m_pData->m_pDriverBot->SetControls (pCtl);
			}

			m_pData->m_pDriverBot->DetachFromParent();

			if (m_pData->m_pDriverBot->IsCapableOfAimingPB())
			{
				m_pData->m_pDriverBot->SetControlValue( CBot::ASI_AIM_PILLBOX, 0.0f );
			}
		}
		m_pData->m_pDriverBot = 0;
		m_bPossessed = FALSE;
	}

	if (m_pData->m_eSiteWeaponState!=CBotSiteWeapon::SITE_DEAD)
	{
		CFMtx43A mtxRecoil = CFMtx43A::m_IdentityMtx;
		m_AliveAnimManMtxAim.UpdateFrame(BONE_PRIMARY_FIRE, m_MtxRecoil);
		m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
		m_pOwnerBot->RelocateAllChildren();
	}
}

BOOL CPillbox::CheckpointSave( void )
{
	BOOL bDead = m_DeadMesh.IsAddedToWorld();
	CFCheckPoint::SaveData( (u32&)bDead  );
	return TRUE;
}

void CPillbox::CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData )
{
	_KillAudioEmitters();
	BOOL bDeadMeshDraw;
	CFCheckPoint::LoadData( (u32&) bDeadMeshDraw );
	_DeadMeshDraw(bDeadMeshDraw);
	_Light(FALSE);

	m_AliveAnimManMtxAim.UpdateFrame(BONE_GUN,  rSiteData.m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_SWIVEL,rSiteData.m_MtxAimY_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_PRIMARY_FIRE, CFMtx43A::m_IdentityMtx);
	m_pOwnerBot->m_Anim.m_pAnimCombiner->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();
}

void CPillbox::_Light(BOOL bLight)
{
	if (!m_pData->m_bEnableLight)
		return;

	if (bLight)
	{
		FASSERT(m_pData->m_pDriverBot);
		
        if (m_pData->m_pDriverBot->m_pBotDef->m_nRace == BOTRACE_MIL) 
		{
			m_pWorldLight->m_Light.m_Motif.Set(m_vRedLightColor);
			m_pWorldLight->m_Light.Enable(TRUE);
		}
		else 
		{
			m_pWorldLight->m_Light.m_Motif.Set(m_vYellowLightColor);
			m_pWorldLight->m_Light.Enable(TRUE);
		}
	}
	else
	{
		m_pWorldLight->m_Light.Enable(FALSE);
	}

	m_pOwnerBot->PlaySound(CBotSiteWeapon::GetSiteWeaponSndHandle(CBotSiteWeapon::SITESND_LIGHTSWITCH));
}

void CPillbox::_UpdateLight(void)
{
	// reposition the light to emanate from camera's position
	CFMtx43 mtxOrientation_WS(	m_pData->m_pMtxAimOrientation_WS->m_vRight.v3,
								m_pData->m_pMtxAimOrientation_WS->m_vUp.v3,
								m_pData->m_pMtxAimOrientation_WS->m_vFront.v3);

	m_pWorldLight->SetWSOverrideVars(&m_pData->m_vAimingOrigin_WS.v3,&mtxOrientation_WS);
	m_pWorldLight->m_Light.SetIntensity(m_fCurrentLightAlpha);
}

void CPillbox::_EjectShell( void ) 
{
	// init our debris settings
	CFDebrisSpawner DebrisSpawner;
	DebrisSpawner.InitToDefaults();
	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
	DebrisSpawner.m_pDebrisGroup = m_pDebrisGroup;
	FASSERT(DebrisSpawner.m_pDebrisGroup);
	DebrisSpawner.m_fMinSpeed = 7.0f;
	DebrisSpawner.m_fMaxSpeed = 12.0f;
	DebrisSpawner.m_pDebrisMesh = &m_DebrisShellMesh;
	DebrisSpawner.m_fUnitDirSpread = 0.2f;
	DebrisSpawner.m_nMinDebrisCount = 1;
	DebrisSpawner.m_nMaxDebrisCount = 1;
	DebrisSpawner.m_fScaleMul = 1.0f;
	DebrisSpawner.m_fGravityMul = 1.0f;
	DebrisSpawner.m_fRotSpeedMul = 1.0f;

	CFMtx43A* pEjectMtx = m_AliveMesh.GetBoneMtxPalette()[m_nShellEjectPtBoneIdx];
	DebrisSpawner.m_Mtx.m_vPos = pEjectMtx->m_vPos;
	DebrisSpawner.m_Mtx.m_vZ = pEjectMtx->m_vFront;

	FMATH_SETBITMASK( DebrisSpawner.m_nFlags, CFDebrisSpawner::FLAG_PLAY_IMPACT_SOUND );

	DebrisSpawner.Spawn();
}

u32 CPillbox::GetTagPointCount( void ) const
{
	return TAG_POINT_COUNT;
}
const CFVec3A *CPillbox::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT( nTagPointIndex < TAG_POINT_COUNT );
	return m_apTagPoint_WS[nTagPointIndex];

}

void CPillbox::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( (nTrackerSkipListCount + 2) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = &m_AliveMesh;
	apTrackerSkipList[nTrackerSkipListCount++] = &m_DeadMesh;
	if (m_pData->m_pDriverBot)
	{
		m_pData->m_pDriverBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

BOOL CPillbox::CanOccupyStation( CBot * pBot)
{
	if (pBot->m_nPossessionPlayerIndex > -1 && pBot->IsInAir())	  //pgm: don't require ai bots to be on the ground please, they're sometimes in the air for stranger reasons that you might think.
		return FALSE;
	
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

void CPillbox::_KillAudioEmitters( void ) 
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