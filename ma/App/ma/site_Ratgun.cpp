//////////////////////////////////////////////////////////////////////////////////////
// site_RatGun.cpp
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
// 1/12/03 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "MeshTypes.h"
#include "site_RatGun.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "FCheckPoint.h"
#include "fsound.h"
#include "ftext.h"
#include "weapon.h"
#include "potmark.h"
#include "explosion.h"
#include "eproj.h"
#include "hud2.h"
#include "player.h"
#include "AI\AIEnviro.h"
#include "AI\AIBrainman.h"
#include "vehiclerat.h"

#define _ANTENNA_SEGMENTS			( 5 )
#define _ANTENNA_FREQUENCY			( 60.0f )
#define _ANTENNA_DAMPING			( 2.5f )
#define _ANTENNA_AMPLITUDE			( 10.0f )

static cchar _apszRatGunInfo_Filename[]		= "sw_RatGun";
static cchar _apszRatMeshFilename[]			= "grmrturret1";
static cchar _apszDroidRatMeshFilename[]	= "grdrturret1";
static cchar _apszRatMeshDeadFilename[]		= "grmrturret1";
static cchar _apszShellMeshFilename[]		= "gwmgshell01";
static cchar _apszDustParticleDefName[]		= "e_ratimpct";
static CFVec3A m_vCameraOffset;


CRatGun::_RatGunSpewProps_t	CRatGun::m_aSpewProps[2];
CDamageProfile*		CRatGun::m_aDamageProfile[2];

CFDebrisGroup*			CRatGun::m_pDebrisGroup;
FMesh_t*				CRatGun::m_pShellMesh;
CFDebrisMesh			CRatGun::m_DebrisShellMesh;
s32						CRatGun::m_nBoneIndexSwivel;

// This table describes to fgamedata how our user property table is to be interpreted:

const FGameData_TableEntry_t CRatGun::m_aSpewPropVocab[] =
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

const FGameDataMap_t CRatGun::m_aGameDataMap[] = {
	"GunPropsMil",
	m_aSpewPropVocab,
	sizeof(_RatGunSpewProps_t),
	(void *)&CRatGun::m_aSpewProps[0],

	"GunPropsPossessed",
	m_aSpewPropVocab,
	sizeof(_RatGunSpewProps_t),
	(void *)&CRatGun::m_aSpewProps[1],

	NULL
};

cchar *CRatGun::m_apszBoneNameList[BONE_COUNT] =
{
	"BallTurretBase",
	"BallTurret",
	"gun barrel",
	"Primary_Fire",
	"attachPoint_Gunner",
	"BallTurretRibs",
	"Shell_Eject",
};

//ARG - >>>>> moved from below to fix table order problem when compiled on PS2
cchar *CRatGun::m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT] = 
{
	"rest",		//	ANIMCONTROL_REST,
	"open",		//  ANIMCONTROL_OPEN,
	"manual",   //  ANIMCONTROL_AIM_SUMMER,
};


cchar *CRatGun::m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT] = 
{
	"rest",		//	ANIMTAP_REST,
	"open",		//  ANIMTAP_OPEN,
	"manual",   //  ANIMTAP_AIM_SUMMER
};

cchar *CRatGun::m_apszBaseAnimNameTable[ANIM_BASE_COUNT] = 
{
	"armrturret1",
};
//ARG - <<<<<

//ARG - the PS2 compiler will not maintain the order of or group like tables, it also aligns each table to an 8 byte
//ARG	boundry, so the code below is very bad and needs to be fixed.

const u8 CRatGun::m_aBoneEnableIndices_FullBody[] = 
{
	BONE_BALLTURRETBASE,
	BONE_BALLTURRET,
	BONE_GUN_BARREL,
	BONE_PRIMARY_FIRE,
	BONE_ATTACHPOINT_GUNNER,
	BONE_BALLTURRETRIBS,
	BONE_SHELL_EJECT,
	255		// Bone-mask lists end with 255; without it the reader ran into the next table
};
const u8 CRatGun::m_aBoneEnableIndices_OpenClose[] =
{
	BONE_BALLTURRETRIBS,
	255
};
const u8 CRatGun::m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT] =
{
	BONE_BALLTURRETBASE,
	BONE_BALLTURRET,
	BONE_PRIMARY_FIRE,
	BONE_GUN_BARREL,
};

CFAnimCombinerConfig::ConfigStack_t CRatGun::m_aAnimCombinerConfigStack[] = {
	// From highest to lowest priority:
	CFAnimMixer::TYPE_SUMMER,					ANIMCONTROL_AIM_SUMMER,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_OPEN,
	CFAnimMixer::TYPE_BLENDER,					ANIMCONTROL_REST,
	CFAnimMixer::TYPE_COUNT,					255
};


CFAnimCombinerConfig::ConfigTap_t CRatGun::m_aAnimCombinerConfigTaps[] = {
	ANIMCONTROL_REST,			ANIMTAP_REST,				1,
	ANIMCONTROL_AIM_SUMMER,		ANIMTAP_AIM_SUMMER,			1,
	ANIMCONTROL_OPEN,			ANIMTAP_OPEN,				1,
	255,						255,						255
};


CFAnimCombiner::AttachList_t CRatGun::m_aAnimAttach[] = {
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_REST,
	BOTANIM_NULL_ANIMSOURCE_ATTACH,			ANIMCONTROL_AIM_SUMMER,
	ANIM_OPEN,								ANIMCONTROL_OPEN,
	255,									255
};

const u8 *CRatGun::m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT] = {
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_REST,
	m_aBoneEnableIndices_FullBody,			// ANIMTAP_AIM_SUMMER,
	m_aBoneEnableIndices_OpenClose,			// ANIMTAP_OPEN
};

const u8 CRatGun::m_anEnableBoneNameIndexTableForSummer_Normal[] = 
{
	// Enable these bones to be driven by our summer:
	BONE_GUN_BARREL,
	BONE_BALLTURRET, 
	BONE_BALLTURRETBASE,
	255
};

CBotAnimStackDef CRatGun::m_AliveAnimStackDef;

CRatGun::~CRatGun()
{

}

void CRatGun::Destroy(void)
{
	_KillAudioEmitters();
	m_pOwnerBot->m_Anim.Destroy();
	m_AliveMeshRestAnimSource.Destroy();
	m_AliveAnimManMtxAim.Destroy();
}

BOOL CRatGun::InitSystem( void )
{
	if (CRatGun::m_AliveAnimStackDef.IsCreated()==FALSE)
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
			DEVPRINTF("CRatGun: failed to Create m_AliveAnimStackDef\n");
			goto _ExitWithError;
		}
	}
#if FANG_WINGC
	if( !CBotSiteWeapon::ReadRetailGunTables( m_aGameDataMap, m_aSpewPropVocab, _apszRatGunInfo_Filename ) )
#else
	if( !fgamedata_ReadFileUsingMap( m_aGameDataMap, _apszRatGunInfo_Filename) )
#endif
	{
		DEVPRINTF( "CRatGunRocket::Create(): Could not create .\n" );
		goto _ExitWithError;
	}

	m_aDamageProfile[0] = CDamage::FindDamageProfile( m_aSpewProps[0].pszDamageProfile );
	m_aDamageProfile[1] = CDamage::FindDamageProfile( m_aSpewProps[1].pszDamageProfile );

	m_pShellMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE,_apszShellMeshFilename);
	if( m_pShellMesh == NULL )
	{
			DEVPRINTF( "CRatGun::InitSystem(): Could not load shell mesh '%s'.\n", _apszShellMeshFilename );
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

void CRatGun::UninitSystem( void )
{
	m_AliveAnimStackDef.Destroy();
}


BOOL CRatGun::Create( CBotSiteWeapon::SiteWeaponData_t* pData, void* pUser, const u64 uUserTypeBits )
{
	FMesh_t *pAliveMesh;

	m_pOwnerBot = (CBot*)pUser;
	m_pData = pData;
	FMeshInit_t MeshInit;

	m_pSpewProps=&m_aSpewProps[0];
	m_pDamageProfile=m_aDamageProfile[0];

	// actions are how we take over the Bot
	m_pOwnerBot->SetActionable( TRUE );

	// Load mesh resource...
	if( CVehicleRat::m_bLoadingDroidRat )
	{
		pAliveMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszDroidRatMeshFilename);
	}
	else
	{
		pAliveMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _apszRatMeshFilename);
	}

	if( pAliveMesh == NULL )
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyBuild(): Could not find mesh '%s'\n", _apszRatMeshFilename );
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
    m_AliveMesh.UpdateTracker();
	m_AliveMesh.RemoveFromWorld();

	m_pAliveMesh = &m_AliveMesh;
	m_pDeadMesh = &m_AliveMesh;

	m_pOwnerBot->m_pWorldMesh = &m_AliveMesh;

	if( !m_pOwnerBot->m_Anim.Create( &m_AliveAnimStackDef, &m_AliveMesh ) )
	{
		return FALSE;
	}

	if( !m_AliveMeshRestAnimSource.Create(m_AliveMesh.m_pMesh))
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyCreate(): Could not create m_MeshRestAnimSource.\n" );
		return FALSE;
	}

	m_pOwnerBot->m_Anim.BaseAnim_Attach(ANIMTAP_REST, &m_AliveMeshRestAnimSource);

	if( !m_AliveAnimManMtxAim.Create( m_AliveAnimStackDef.m_nBoneCount, m_AliveAnimStackDef.m_apszBoneNameTable ) )
	{
		DEVPRINTF( "CBotSentry::ClassHierarchyCreate(): Could not create m_AliveAnimManFrameAim.\n" );
		return FALSE;
	}

	m_pOwnerBot->m_Anim.BaseAnim_Attach(ANIMTAP_AIM_SUMMER, &m_AliveAnimManMtxAim);
	m_AliveAnimManMtxAim.Reset();
	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_REST, 1.0f );
	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_AIM_SUMMER, 1.0f );
	
	m_MtxRecoil = CFMtx43A::m_IdentityMtx;

	m_bFirePrimaryThisFrame = FALSE;

	m_nSpewClipAmmo = CWeapon::INFINITE_AMMO;
	m_nSpewReserveAmmo = CWeapon::INFINITE_AMMO;

	m_fSpewUnitShotSpread  = 0.0f;
    m_fSpewSecondsUntilNextTracer  = 0.0f;
	m_fSpewSecondsCountdownTimer  = 0.0f;
	
	m_fSecsUntilNextSound = 0.0f;

	m_fRecoil0 = 0.0f;
	m_fOpenVelocity = 0.0f;

	m_fRecoilDistance = 0.75f;
	m_fRecoilReturnRate = 2.0f * m_fRecoilDistance * m_pSpewProps->fRoundsPerSec;

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
	
	for(s32 i=0; i<MAX_SIMULTANEOUS_FIRE_SOUNDS; ++i ) 
	{
		m_apAudioEmitterFire[i] = NULL;
	}
	m_nAudioEmitterIndex = 0;


	s32 fireBone = m_AliveMesh.FindBone("Primary_Fire");
	FASSERT(fireBone>=0);
	m_pMuzzlePtArray[0] = m_AliveMesh.GetBoneMtxPalette()[fireBone];
	m_pData->m_pMtxAimOrientation_WS = m_pMuzzlePtArray[0];
	
	s32 nBarrelBone = m_AliveMesh.FindBone("gun barrel");
	FASSERT(nBarrelBone>=0);
	pmtxCameraAttachPtWS = m_AliveMesh.GetBoneMtxPalette()[nBarrelBone];

	m_nAttachBotPtBoneIdx = m_AliveMesh.FindBone(m_apszBoneNameList[BONE_ATTACHPOINT_GUNNER]);
	FASSERT(m_nAttachBotPtBoneIdx>=0);

	m_nShellEjectPtBoneIdx = m_AliveMesh.FindBone(m_apszBoneNameList[BONE_SHELL_EJECT]);
	FASSERT(m_nShellEjectPtBoneIdx>=0);

	m_nBoneIndexSwivel = m_AliveMesh.FindBone(m_apszBoneNameList[BONE_BALLTURRETBASE]);
	FASSERT(m_nBoneIndexSwivel>=0);

	m_vCameraOffset.x = 0.f;
	m_vCameraOffset.y = 1.f;
	m_vCameraOffset.z = -2.5f;

	m_ePreviousState = m_pData->m_eSiteWeaponState;
	m_bPossessed = FALSE;

	m_fIdlingVelocity					= 0;
	m_fIdlingVelocityDamp				= .50f;						// how much velocity is left after hitting an extrema
	m_fIdlingAcceleration				= 4.0f;					    // downward pull from gravity
	fforce_NullHandle( &m_hForce[0] );
	fforce_NullHandle( &m_hForce[1] );
	m_pOwnerBot->m_Anim.BaseAnim_SetControlValue( ANIMCONTROL_OPEN, 1.0f );

	// set up antenna
	{
		s32 anBones[_ANTENNA_SEGMENTS];
		u32 uIndex;
		m_Antenna.Create( &m_AliveMesh, _ANTENNA_SEGMENTS, _ANTENNA_FREQUENCY, _ANTENNA_DAMPING, _ANTENNA_AMPLITUDE );
		anBones[0] = m_AliveMesh.FindBone( "AntennaA" );
		anBones[1] = m_AliveMesh.FindBone( "AntennaB" );
		anBones[2] = m_AliveMesh.FindBone( "AntennaC" );
		anBones[3] = m_AliveMesh.FindBone( "AntennaD" );
		anBones[4] = m_AliveMesh.FindBone( "AntennaE" );
		for( uIndex = 0; uIndex < _ANTENNA_SEGMENTS; uIndex++ )
		{
			if( anBones[uIndex] < 0 )
			{
				DEVPRINTF( "CRatGun::Create: Antenna Bone %d doesn't exist in object.\n", uIndex );
			}
		}

		m_Antenna.RegisterBones( anBones );
	}
	m_pOwnerBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( "AntennaA" );
	m_pOwnerBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( "AntennaB" );
	m_pOwnerBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( "AntennaC" );
	m_pOwnerBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( "AntennaD" );
	m_pOwnerBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( "AntennaE" );
	m_pOwnerBot->m_Anim.m_pAnimCombiner->SetBoneCallback( &CBotSiteWeapon::AnimBoneCallback );

	m_bResettingTurret = FALSE;

	m_hDustParticle = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _apszDustParticleDefName );
	if( m_hDustParticle == FPARTICLE_INVALID_HANDLE ) 
	{
		DEVPRINTF( "CRatGun::Create(): Could not find particle definition '%s'.\n", _apszDustParticleDefName );
	}

	return TRUE;
}


void CRatGun::AddToWorld(void )
{
	m_AliveMesh.UpdateTracker(TRUE);
	// snap antenna to new position
	m_Antenna.SetPos();
	m_pOwnerBot->AtRestMatrixPalette();
}

void CRatGun::RemoveFromWorld(void )
{
	m_AliveMesh.RemoveFromWorld();
}

CFMtx43A* CRatGun::AttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName )
{
	return 0;
}

u32	CRatGun::TriggerWorkPrimary( f32 fUnitTriggerVal1 )
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
u32	CRatGun::TriggerWorkSecondary( f32 fUnitTriggerVal2 )
{
	return 0;
}

void CRatGun::_HandleFireThisFrame(void)
{
	if( !m_pData->m_pDriverBot )
	{
		return;
	}

	if( m_bFirePrimaryThisFrame )
	{
		CVehicleRat *pRat = NULL;
		if( m_pOwnerBot->GetParent() && (m_pOwnerBot->GetParent()->TypeBits() & ENTITY_BIT_VEHICLERAT) )
		{
			pRat = (CVehicleRat*) m_pOwnerBot->GetParent();
		}


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
#if 0
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
#endif

		CFVec3A MuzzlePoint;
		MuzzlePoint = m_pMuzzlePtArray[0]->m_vPos;

		_EjectShell();

		f32 fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );
		if( IsOwnedByPlayer() )
		{
			fRandomScale *= 3.0f;
		}
		else
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
			fforce_Kill( &m_hForce[0] );
			fforce_Play( Player_aPlayer[GetPlayerIndex()].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_MED, &m_hForce[0] );
		}
		
		// send rumble to human driver, if any
		if( pRat && pRat->GetDriverBot() && pRat->GetDriverBot()->m_nPossessionPlayerIndex >= 0 )
		{
			fforce_Kill( &m_hForce[1] );
			fforce_Play( Player_aPlayer[pRat->GetDriverBot()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE_LOW, &m_hForce[1] );
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
				TRUE,
				fmath_RandomFloatRange( 0.5f, 1.0f ),
				fmath_RandomFloatRange( 0.5f, 1.0f ),
				fmath_RandomFloatRange( 0.5f, 1.0f )
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

			if( pCollMaterial->CanDrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST ) && pCollMaterial->m_nMtlID != 0 /*exclude metal*/ )
			{
				if( m_hDustParticle != FPARTICLE_INVALID_HANDLE )
				{
					fparticle_SpawnEmitter( m_hDustParticle, CollImpact.ImpactPoint.v3, &CollImpact.UnitFaceNormal.v3, fmath_RandomFloatRange( 0.1f, 1.0f ) );
				}
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

//		if( IsOwnedByPlayer() == FALSE ) 
		{
			// AI...

			bMakeTracer = fmath_RandomChance( 0.8f );
		} 
//		else 
//		{
//			// Human-controlled...
//
//			if( m_fSpewSecondsUntilNextTracer <= 0.0f ) 
//			{
//				m_fSpewSecondsUntilNextTracer = fmath_RandomFloatRange( 0.1f, 0.25f );
//				bMakeTracer = TRUE;
//			}
//		}

		if( bMakeTracer ) 
		{
			// Make a tracer...

			f32 fTracerLen = MuzzlePoint.Dist( RayEnd );

			CMuzzleFlash::AddFlash_CardPosterZ(
				CWeapon::m_ahMuzzleFlashGroup[CWeapon::MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1],
				MuzzlePoint,
				vFireUnitDir,
//				0.15f,
				1.5f,
				fTracerLen,
				0.2f
				);
		}

		// Spawn muzzle light...
		f32 fMuzzleLightRadius = (IsOwnedByPlayer() ? 15.0f : 10.f);
		CMuzzleFlash::MuzzleLight( &m_pData->m_vAimingOrigin_WS, fMuzzleLightRadius, fmath_RandomFloatRange( 0.5f, 1.0f ) );

		s32 nPlayer = m_pOwnerBot->m_nPossessionPlayerIndex;
		if( nPlayer < 0 && pRat && pRat->GetDriverBot() )
		{
			nPlayer = pRat->GetDriverBot()->m_nPossessionPlayerIndex;
		}

		// Play firing sound...
		if( m_apAudioEmitterFire[m_nAudioEmitterIndex] ) {
			m_apAudioEmitterFire[m_nAudioEmitterIndex]->Destroy();
			m_apAudioEmitterFire[m_nAudioEmitterIndex] = NULL;
		}
		if( nPlayer >= 0 )
		{
			// either gunner or driver is human, play 2D
			CFSoundGroup::PlaySound(	CBotSiteWeapon::GetSiteWeaponSndHandle(CBotSiteWeapon::SITESND_FIRE), 
										TRUE,		//play 2d
										&m_pOwnerBot->MtxToWorld()->m_vPos, 
										nPlayer,	// player index
										TRUE,		// inform ai
										fmath_RandomFloatRange( 0.8f, 1.0f ),/*volume*/
										1.0f, /*pitch */
										-1.0f, /* fRadiusOverride */
										&m_apAudioEmitterFire[m_nAudioEmitterIndex]);
		} 
		else
		{
			// play 3D
			CFSoundGroup::PlaySound(	CBotSiteWeapon::GetSiteWeaponSndHandle(CBotSiteWeapon::SITESND_FIRE), 
										FALSE,		// play 2d
										&m_pOwnerBot->MtxToWorld()->m_vPos, 
										-1,			// player index
										FALSE,		// inform ai
										fmath_RandomFloatRange( 0.8f, 1.0f ),/*volume*/ 
										1.0f, /*pitch */
										-1.0f, /* fRadiusOverride */
										&m_apAudioEmitterFire[m_nAudioEmitterIndex]);
		}

	}
	if( m_fSpewSecondsCountdownTimer > 0.0f )
	{
		m_fSpewSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}
}

void CRatGun::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	*pApproxMuzzlePoint_WS = m_pMuzzlePtArray[0]->m_vPos;
}

void 	CRatGun::Work(void)								// my tick
{
	m_fNormHealth = m_pOwnerBot->NormHealth();
	if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_IDLING)
	{
		_IdlingWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWER_DOWN)
	{
		_PowerDownWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
	{
		_DeathWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_ENTERING_POSSESSED)
	{
		_EnteringPossessedWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POWERING_UP_POSSESSED)
	{
		_PossessedPowerUpWork();
	}
	else if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_POSSESSED)
	{
		if( m_pData->m_pDriverBot )
		{
			m_pData->m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS( m_AliveMesh.GetBoneMtxPalette()[m_nAttachBotPtBoneIdx] );
		}
	}

	// update m_fYawAdjustWS2MS every frame
	if( m_pOwnerBot->GetParent() )
	{
		CEntity *pEntity;
		pEntity = (CEntity*) m_pOwnerBot->GetParent();
		m_pData->m_fYawAdjustWS2MS = fmath_Atan( pEntity->MtxToWorld()->m_vFront.x, pEntity->MtxToWorld()->m_vFront.z );
	}
	// update m_fPitchAdjustWS2MS every frame
	{
		m_pData->m_fPitchAdjustWS2MS = fmath_Atan( -m_AliveMesh.GetBoneMtxPalette()[m_nBoneIndexSwivel]->m_vFront.y, m_AliveMesh.GetBoneMtxPalette()[m_nBoneIndexSwivel]->m_vFront.MagXZ() );
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

	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRET,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRETBASE,m_pData->m_MtxAimY_MS);
	m_pOwnerBot->m_Anim.BaseAnim_DeltaUnitTime(ANIMTAP_OPEN,m_fOpenVelocity * FLoop_fPreviousLoopSecs,TRUE);
	m_pOwnerBot->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();

	m_Antenna.Work();

	// move to attach point
	const CFMtx43A* pmtxAttachBot = (m_AliveMesh.GetBoneMtxPalette()[m_nAttachBotPtBoneIdx]);

#if 0
	CFColorRGBA red;red.OpaqueRed();
	CFVec3A Start, End, Dir;
	Start = pmtxAttachBot->m_vPos;
	Dir =pmtxAttachBot->m_vFront;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&red,&red);

	CFColorRGBA cyan;cyan.OpaqueCyan();
	Start = pmtxAttachBot->m_vPos;
	Dir =pmtxAttachBot->m_vUp;
	End.Add(Start,Dir.Mul(100.0f));
	fdraw_DevLine(&Start,&End,&cyan,&cyan);

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
		if (m_pData->m_pDriverBot->IsCapableOfAimingPB())
		{
			m_pData->m_pDriverBot->UpdateTime(CBot::ASI_AIM_PILLBOX, 0.20f);
		}

	}
	_HandleFireThisFrame();
}

void 	CRatGun::_IdlingWork(void)						// my other tick
{
	// remap gun yaw to +/- 180 degrees if necessary
	if( m_pData->m_fYawWS > FMATH_DEG2RAD( 180.0f ) )
	{
		m_pData->m_fYawWS -= FMATH_DEG2RAD( 360.0f );
	}
	else if( m_pData->m_fYawWS < FMATH_DEG2RAD( -180.0f ) )
	{
		m_pData->m_fYawWS += FMATH_DEG2RAD( 360.0f );
	}

	// return gun pitch to neutral position
	if( m_pData->m_fPitchWS > 0.0f )
	{
		m_pData->m_fPitchWS -= FLoop_fPreviousLoopSecs;
		if( m_pData->m_fPitchWS < 0.0f )
		{
			m_pData->m_fPitchWS = 0.0f;
		}
	}
	else if( m_pData->m_fPitchWS < 0.0f )
	{
		m_pData->m_fPitchWS += FLoop_fPreviousLoopSecs;
		if( m_pData->m_fPitchWS > 0.0f )
		{
			m_pData->m_fPitchWS = 0.0f;
		}
	}

	// return gun yaw to face forward relative to RAT
	if( m_bResettingTurret )
	{
		f32 fVel = FLoop_fPreviousLoopSecs;
		f32 fDelta = m_pData->m_fYawAdjustWS2MS - m_pData->m_fYawWS;
		if( fDelta < 0.0f )
		{
			// turn left if delta < 0
			fVel = -fVel;
		}
		
		if( FMATH_FABS( fDelta ) > FMATH_DEG2RAD( 180.0f ) )
		{
			// turn the shortest way by turning the other way if simple delta is > 180 degrees
			fVel = -fVel;
		}

		if( FMATH_FABS( fDelta ) > FMATH_FABS( fVel ) )
		{
			m_pData->m_fYawWS += fVel;
		}
		else
		{
			m_bResettingTurret = FALSE;
			// close enough, set yaw to equal RAT body yaw
			m_pData->m_fYawWS = m_pData->m_fYawAdjustWS2MS;
		}
	}
	else
	{
		m_pData->m_fYawWS = m_pData->m_fYawAdjustWS2MS;
	}

	m_pData->m_MtxAimY_MS.SetRotationY(m_pData->m_fYawWS - m_pData->m_fYawAdjustWS2MS);
	m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS-m_pData->m_fPitchAdjustWS2MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRET,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRETBASE,m_pData->m_MtxAimY_MS);

	m_pOwnerBot->m_Anim.BaseAnim_DeltaUnitTime(ANIMTAP_OPEN,m_fOpenVelocity * FLoop_fPreviousLoopSecs, TRUE);
	m_pOwnerBot->ComputeMtxPalette( FALSE ); // outside if to update positions

	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
}


void 	CRatGun::_EnteringPossessedWork(void)
{
	if(IsOwnedByPlayer() == TRUE) // don't play anim in possessed cases
		return;

	FASSERT(m_pData->m_fTimeToEnter > 0.0f);		
	f32 fBlendPercent = fmath_Div(m_pData->m_fTimeSinceStateChanged,m_pData->m_fTimeToEnter);
	FASSERT(m_pData->m_pDriverBot->IsCapableOfAimingPB());

	m_pData->m_pDriverBot->SetControlValue( CBot::ASI_AIM_PILLBOX, fBlendPercent );

	const CFMtx43A* pmtxAttachBot = (m_AliveMesh.GetBoneMtxPalette()[m_nAttachBotPtBoneIdx]);
	FASSERT(m_pData->m_pDriverBot);
	m_pData->m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS(pmtxAttachBot,FALSE);

	m_pData->m_pDriverBot->UpdateTime(CBot::ASI_AIM_PILLBOX, 0.20f);
}

void 	CRatGun::_PossessedPowerUpWork(void)			// my other tick
{
	m_pData->m_MtxAimY_MS.SetRotationY(m_pData->m_fYawWS-m_pData->m_fYawAdjustWS2MS);
	m_pData->m_MtxAimX_MS.SetRotationX(m_pData->m_fPitchWS-m_pData->m_fPitchAdjustWS2MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRET,  m_pData->m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRETBASE,m_pData->m_MtxAimY_MS);
	m_pOwnerBot->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();	// SER: This needs to be here so that attached entities (like an L3 arrow) will be relocated
}

void 	CRatGun::_PowerDownWork(void)					// my other other tick
{
}

void 	CRatGun::_DeathWork(void)						// my other other other tick
{
}

void 	CRatGun::CueNewState(void)
{
	if (m_ePreviousState == CBotSiteWeapon::SITE_ENTERING_POSSESSED)
	{
		if (m_pData->m_pDriverBot->IsInWorld())
		{
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
		m_bResettingTurret = TRUE;
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
	case CBotSiteWeapon::SITE_ENTERING_POSSESSED:
		if (m_pData->m_pDriverBot->IsPlayerBot())
			m_pData->m_fTimeToEnter = 0.0f;
		else
			m_pData->m_fTimeToEnter = 0.5f;
		return;

	case CBotSiteWeapon::SITE_POWERING_UP_POSSESSED:
		return _CuePossessed(TRUE);
	case CBotSiteWeapon::SITE_POSSESSED:
		{
			s32 nIdx = GetPlayerIndex();
			if( nIdx >= 0 )
			{
				CReticle *pReticle = &Player_aPlayer[ nIdx ].m_Reticle;
				pReticle->EnableDraw(TRUE, TRUE);
			}
		}
		break;
	}
}

void 	CRatGun::_CuePowerUp(void)							// turn me on
{
}

void 	CRatGun::_CuePowerDown(void)				// WHIRRRWhiRRwwwhir.
{
}

void 	CRatGun::_CueDeath(void)					// WHIRRRWhiRRwwwhir.
{
	
}

void 	CRatGun::_CuePossessed(BOOL bGoPosed)		// transform!()
{
	FASSERT(m_pOwnerBot->m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN);
	if (bGoPosed)
	{
		if (m_bPossessed) // already did this
			return;
		FASSERT(m_pData->m_pDriverBot);

		// control the gun
		m_pOwnerBot->SetControls( m_pData->m_pDriverBot->Controls());
		// relocate to inside
		const CFMtx43A* pmtxAttachBot = (m_AliveMesh.GetBoneMtxPalette()[m_nAttachBotPtBoneIdx]);
		FASSERT(m_pData->m_pDriverBot);
		m_pData->m_pDriverBot->Relocate_RotXlatFromUnitMtx_WS(pmtxAttachBot,FALSE);

		// attach to point
 		m_pData->m_pDriverBot->Attach_ToParent_WS( m_pOwnerBot,m_apszBoneNameList[BONE_ATTACHPOINT_GUNNER]);
		m_pData->m_pDriverBot->SetControls(NULL);
//		m_pData->m_pDriverBot->EnteringMechWork(m_pOwnerBot);


		// make driving bot non-collidable
//		m_pData->m_pDriverBot->m_pWorldMesh->SetCollisionFlag(FALSE);
// we want RAT gunner to be collidable for weapon damage -cjm

		// enter aiming anim
		if (m_pData->m_pDriverBot->IsCapableOfAimingPB())
		{
			m_pData->m_pDriverBot->SetControlValue( CBot::ASI_AIM_PILLBOX, 1.0f );
		}

		m_bPossessed = TRUE;
		if (IsOwnedByPlayer()==TRUE)
		{
			CReticle *pReticle = &Player_aPlayer[ GetPlayerIndex() ].m_Reticle;
			CHud2* pHud = CHud2::GetHudForPlayer(GetPlayerIndex());

			// remember previous values
			m_fOldNormX = pReticle->GetNormOriginX();
			m_fOldNormY = pReticle->GetNormOriginY();
			m_OldHudMode = pHud->m_eCurHudMode;
			m_OldReticleType = pReticle->GetType();
			pReticle->SetNormOrigin(0.0f,0.0f);
			pReticle->SetType( CReticle::TYPE_FLOORSENTRY );
			pReticle->SetUnitScale(0.0f);
			pReticle->SetReticleOverride();
			pReticle->EnableDraw(FALSE,TRUE);

			m_uOldHudFlags = pHud->SetDrawFlags(CHud2::DRAW_ENABLEOVERRIDE | CHud2::DRAW_BATTERIES | CHud2::DRAW_RADAR);
			CFColorRGBA startColor, endColor;

			startColor.Set( 1.0f, 1.0f, 0.0f, 0.5f );
			endColor.Set( 0.0f, 1.0f, 0.0f, 0.5f );

			m_pSpewProps=&m_aSpewProps[1];
			m_pDamageProfile=m_aDamageProfile[1];

//			if (m_pOwnerBot->AIBrain())
//			{
//				//when players take "possession" of bots with brains, call this
//				//because the brain will be repurposed during that time.
//				//see exit vehicle for the undoing of this.
//				aibrainman_ConfigurePlayerBotBrain( m_pOwnerBot->AIBrain(), m_pOwnerBot->m_nPossessionPlayerIndex );	 //turns this bot's brain into a player brain
//			}


		}
	}
	else
	{
		if (m_pData->m_pDriverBot && m_pData->m_pDriverBot->IsInWorld())// now, can get called on quit, and driver gets popped first
		{
			s32 nIdx = GetPlayerIndex();
			if (nIdx > -1)
			{
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

			CEntityControl* pCtl = m_pOwnerBot->Controls();
			m_pOwnerBot->SetControls( NULL );
//			m_pData->m_pDriverBot->ExitingMechWork();

			//
			//  Note, anytime a mech kicks its owner out, it is good to call notify it
			//
			if (m_pData->m_eSiteWeaponState == CBotSiteWeapon::SITE_DEAD)
			{
				if (m_pData->m_pDriverBot->AIBrain())
				{
					aibrainman_NotifyMechEject(m_pData->m_pDriverBot->AIBrain());
				}
			}
//			m_pData->m_pDriverBot->m_pWorldMesh->SetCollisionFlag(TRUE);
// no longer setting non-collidable upon entry - cjm
			if (IsOwnedByPlayer())
			{
				CPlayer *pPlayer = &Player_aPlayer[ GetPlayerIndex() ];
				pPlayer->ZeroControls();
			}

			m_pData->m_pDriverBot->SetControls (pCtl);
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
		m_pOwnerBot->ComputeMtxPalette( FALSE );
		m_pOwnerBot->RelocateAllChildren();
	}
}

BOOL CRatGun::CheckpointSave( void )
{
	return TRUE;
}

void CRatGun::CheckpointRestore( const CBotSiteWeapon::SiteWeaponData_t& rSiteData )
{
	_KillAudioEmitters();
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRET,  rSiteData.m_MtxAimX_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_BALLTURRETBASE,rSiteData.m_MtxAimY_MS);
	m_AliveAnimManMtxAim.UpdateFrame(BONE_PRIMARY_FIRE, CFMtx43A::m_IdentityMtx);
	m_pOwnerBot->ComputeMtxPalette( FALSE );
	m_pOwnerBot->RelocateAllChildren();
}


void CRatGun::_EjectShell( void )
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

u32 CRatGun::GetTagPointCount( void ) const
{
	return TAG_POINT_COUNT;
}
const CFVec3A *CRatGun::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT( nTagPointIndex < TAG_POINT_COUNT );
	return m_apTagPoint_WS[nTagPointIndex];

}

BOOL CRatGun::GetOpenOrClosed(void)
{
	f32 fAnimTime = m_pOwnerBot->m_Anim.BaseAnim_GetUnitTime(ANIMTAP_OPEN);
	if (fAnimTime<.99f)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CRatGun::SetOpenOrClosed(BOOL bOpen, f32 fHowLongItShouldTakeSecs )
{
	FMATH_CLAMP(fHowLongItShouldTakeSecs, .05f,10.f);
	f32 fOOHowLongItShouldTakeSecs = 1.0f/fHowLongItShouldTakeSecs;
	
	m_fOpenVelocity = (bOpen ?  fOOHowLongItShouldTakeSecs : -fOOHowLongItShouldTakeSecs);

	return TRUE;
}

void CRatGun::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList)
{
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = &m_AliveMesh;

	if (m_pData->m_pDriverBot)
	{
		m_pData->m_pDriverBot->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	}
}

BOOL CRatGun::CanOccupyStation( CBot * pBot)
{
	// it's up to rat to make this determination, not i
	return TRUE;
}

void CRatGun::_KillAudioEmitters( void ) 
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