//////////////////////////////////////////////////////////////////////////////////////
// weapon_cleaner.cpp - Cleaner weapon.
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
// 06/05/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_cleaner.h"
#include "weapon.h"
#include "eproj.h"
#include "iteminst.h"
#include "fsndfx.h"
#include "fresload.h"
#include "eproj_cleaner.h"
#include "damage.h"
#include "bot.h"



#define _USER_PROP_FILENAME		"w_cleaner.csv"



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponCleanerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponCleanerBuilder _WeaponCleanerBuilder;


void CWeaponCleanerBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONCLEANER, pszEntityType );
}


BOOL CWeaponCleanerBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponCleanerBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponCleaner
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponCleaner::_UserProps_t CWeaponCleaner::m_aUserProps[EUK_COUNT_CLEANER];
CEProjPool::PoolHandle_t CWeaponCleaner::m_ahProjPool[EUK_COUNT_CLEANER];
SmokeTrailAttrib_t CWeaponCleaner::m_CanisterSmokeTrailAttrib;
SmokeTrailAttrib_t CWeaponCleaner::m_RocketSmokeTrailAttrib;
CFTexInst CWeaponCleaner::m_StreamerTexInst;


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponCleaner::m_aUserPropVocab[] = {
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

	// fRoundsPerSec:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_1000,

	// fGrenadeCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fGrenadesInPoolCount:
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

	// fGrenadeSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fRocketMaxDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10000000,

	// fRocketMaxSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10000000,

	// fRocketMinSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10000000,

	// fRocketMaxSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10000000,

	// fRocketScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt01,
	F32_DATATABLE_10000000,

	// fRocketSwarmAmp:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000000,

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hRocketExplosionGroup

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroup
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupUse
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupBounce
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupLock
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupWhir


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponCleaner::m_aUserPropMapTable[] = {
	"Cleaner",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	NULL
};


BOOL CWeaponCleaner::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;
	FTexDef_t *pTexDef;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponCleaner::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_CLEANER; i++ ) {
		// Create a pool of projectiles...
		m_ahProjPool[i] = CEProjPool::Create( CEProj::PROJTYPE_CLEANER, m_aUserProps[i].apszMeshName[_MESH_GRENADE], (u32)m_aUserProps[i].fGrenadesInPoolCount );

		if( m_ahProjPool[i] == EPROJPOOL_NULL_HANDLE ) {
			DEVPRINTF( "CWeaponCleaner::InitSystem(): Could not create projectile pool.\n" );
			goto _ExitWithError;
		}

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_CLEANER][i];

		pInfo->nGrip = GRIP_LEFT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = 0.0f;
		pInfo->fMaxTargetAssistDist = 1000.0f;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_THROWABLE;
		pInfo->nReticleType = CReticle::TYPE_NONE;
	}

#if 1
	{
		FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
		if( pTexDef == NULL ) {
			DEVPRINTF( "CWeaponCleaner::InitSystem(): Could not load smoke trail texture.\n" );
		}

		m_RocketSmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
		m_RocketSmokeTrailAttrib.pTexDef = pTexDef;

		m_RocketSmokeTrailAttrib.fScaleMin_WS = 0.6f;
		m_RocketSmokeTrailAttrib.fScaleMax_WS = 0.8f;
		m_RocketSmokeTrailAttrib.fScaleSpeedMin_WS = 0.5f;
		m_RocketSmokeTrailAttrib.fScaleSpeedMax_WS = 0.8f;
		m_RocketSmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
		m_RocketSmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

		m_RocketSmokeTrailAttrib.fXRandSpread_WS = 0.05f;
		m_RocketSmokeTrailAttrib.fYRandSpread_WS = 0.05f;
		m_RocketSmokeTrailAttrib.fDistBetweenPuffs_WS = 0.1f;
		m_RocketSmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.025f;

		m_RocketSmokeTrailAttrib.fYSpeedMin_WS = 0.25f;
		m_RocketSmokeTrailAttrib.fYSpeedMax_WS = 0.5f;
		m_RocketSmokeTrailAttrib.fYAccelMin_WS = -0.2f;
		m_RocketSmokeTrailAttrib.fYAccelMax_WS = -0.5f;

		m_RocketSmokeTrailAttrib.fUnitOpaqueMin_WS = 0.2f;
		m_RocketSmokeTrailAttrib.fUnitOpaqueMax_WS = 0.5f;
		m_RocketSmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -1.5f;
		m_RocketSmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.5f;
		m_RocketSmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
		m_RocketSmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

		m_RocketSmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.8f, 0.15f );
		m_RocketSmokeTrailAttrib.EndColorRGB.Set( 0.8f, 0.4f, 0.0f );
		m_RocketSmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
		m_RocketSmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
		m_RocketSmokeTrailAttrib.fEndColorUnitIntensityMin = 0.75f;
		m_RocketSmokeTrailAttrib.fEndColorUnitIntensityMax = 1.0f;

		m_RocketSmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
		m_RocketSmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
		m_RocketSmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
		m_RocketSmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;



		m_CanisterSmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
		m_CanisterSmokeTrailAttrib.pTexDef = pTexDef;

		m_CanisterSmokeTrailAttrib.fScaleMin_WS = 0.4f;
		m_CanisterSmokeTrailAttrib.fScaleMax_WS = 0.8f;
		m_CanisterSmokeTrailAttrib.fScaleSpeedMin_WS = 0.5f;
		m_CanisterSmokeTrailAttrib.fScaleSpeedMax_WS = 0.8f;
		m_CanisterSmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
		m_CanisterSmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

		m_CanisterSmokeTrailAttrib.fXRandSpread_WS = 0.05f;
		m_CanisterSmokeTrailAttrib.fYRandSpread_WS = 0.05f;
		m_CanisterSmokeTrailAttrib.fDistBetweenPuffs_WS = 0.2f;
		m_CanisterSmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.025f;

		m_CanisterSmokeTrailAttrib.fYSpeedMin_WS = 0.25f;
		m_CanisterSmokeTrailAttrib.fYSpeedMax_WS = 0.5f;
		m_CanisterSmokeTrailAttrib.fYAccelMin_WS = -0.2f;
		m_CanisterSmokeTrailAttrib.fYAccelMax_WS = -0.5f;

		m_CanisterSmokeTrailAttrib.fUnitOpaqueMin_WS = 0.5f;
		m_CanisterSmokeTrailAttrib.fUnitOpaqueMax_WS = 0.9f;
		m_CanisterSmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -1.5f;
		m_CanisterSmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.5f;
		m_CanisterSmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
		m_CanisterSmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

		m_CanisterSmokeTrailAttrib.StartColorRGB.Set( 1.0f, 1.0f, 1.0f );
		m_CanisterSmokeTrailAttrib.EndColorRGB.Set( 0.7f, 0.7f, 0.7f );
		m_CanisterSmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
		m_CanisterSmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
		m_CanisterSmokeTrailAttrib.fEndColorUnitIntensityMin = 0.75f;
		m_CanisterSmokeTrailAttrib.fEndColorUnitIntensityMax = 1.0f;

		m_CanisterSmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
		m_CanisterSmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
		m_CanisterSmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
		m_CanisterSmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	}
#endif

	// Load streamer texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "trdmstrmry1" );
	m_StreamerTexInst.SetTexDef( pTexDef );
	m_StreamerTexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	if( !CEProj_Cleaner::InitSystem() )
	{
		DEVPRINTF( "CWeaponCleaner::Init(): CEProj_Cleaner::InitSystem() failed.\n" );
		goto _ExitWithError;
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponCleaner::UninitSystem( void ) {
}


CWeaponCleaner::CWeaponCleaner() {
	_ClearDataMembers();
}


CWeaponCleaner::~CWeaponCleaner() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponCleaner::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponCleanerBuilder *pBuilder = (CWeaponCleanerBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponCleanerBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONCLEANER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_CLEANER, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponCleaner::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponCleanerBuilder *pBuilder = (CWeaponCleanerBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponCleaner::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CWeapon::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponCleaner::ClassHierarchyDestroy( void ) {
	// Destroy ourselves first...
	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponCleaner::GetLeafClassBuilder( void ) {
	return &_WeaponCleanerBuilder;
}


void CWeaponCleaner::_ClearDataMembers( void ) {
	m_pResourceData = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_pProjToThrow = NULL;
	m_nTriggerDownTicks = 0;
	m_bTriggerDownLastFrame = FALSE;
	m_fUnitThrowSpeed = 0.0f;
	m_nWeaponFlags |= WEAPONFLAG_NEEDS_TARGETED_ENTITY_FOR_USE;
}


void CWeaponCleaner::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	if( m_pProjToThrow ) {
		CEProjPool::ReturnProjectileToFreePool( m_pProjToThrow );
		m_pProjToThrow = NULL;
	}
}


void CWeaponCleaner::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo );

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NONE );
	}
}


BOOL CWeaponCleaner::Throwable_TriggerWork( f32 fUnitTriggerVal ) {
	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		return FALSE;
	}

	if( m_nClipAmmo == 0 ) {
		// No rounds in clip...
		return FALSE;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return FALSE;
	}

	// Cleaners are only useable by players!
	if (GetOwner()->m_nPossessionPlayerIndex < 0)
		return FALSE;

	if( fUnitTriggerVal > 0.0f )
	{
		// trigger pressed

		if( !m_bTriggerDownLastFrame )
		{
			// just entered targeting mode

			if( m_pProjToThrow )
			{
				// Our projectile hasn't been returned yet...
				return FALSE;
			}

			// get projectile as soon as trigger is pressed
			// so that we can give it targets.
			if( !_GetProjectileFromPoolAndInit() )
			{
				// No more projectiles...
				return FALSE;
			}
		}

		if( m_pProjToThrow )
		{
			// assign targets while trigger is pressed
			CEProj_Cleaner *pCleanerProjectile = (CEProj_Cleaner*) m_pProjToThrow->GetExtensionObject( CEProj::PROJTYPE_CLEANER );

			pCleanerProjectile->AssignTarget( GetOwner()->m_nPossessionPlayerIndex,  GetOwner(), GetTargetedEntity(), m_aUserProps[m_nUpgradeLevel].pSoundGroupLock );
		}

		// we are in targeting mode
		m_bTriggerDownLastFrame = TRUE;

		return FALSE;
	}

	if( !m_bTriggerDownLastFrame )
	{
		// trigger is up and we were
		// not just in targeting mode
		return FALSE;
	}

	// if we reach this point, then trigger was just released

	m_bTriggerDownLastFrame = FALSE;

	if( !m_pProjToThrow )
	{
		return FALSE;
	}

	CEProj_Cleaner *pCleanerProjectile = (CEProj_Cleaner*) m_pProjToThrow->GetExtensionObject( CEProj::PROJTYPE_CLEANER );

	if( !pCleanerProjectile )
	{
		return FALSE;
	}

	RemoveFromClip( 1 );

	if( pCleanerProjectile->NumTargetsInList() > 0 )
	{
		m_fSecondsCountdownTimer = 2.0f;

		// Compute throw speed...
		f32 fDeltaSecs = FLoop_fSecsPerTick * (f32)(FLoop_nTotalLoopTicks - m_nTriggerDownTicks);

		m_fUnitThrowSpeed = fDeltaSecs * (1.0f / 1.0f);
		FMATH_CLAMP( m_fUnitThrowSpeed, 0.0f, 1.0f );
		m_fUnitThrowSpeed = 1.0f - m_fUnitThrowSpeed;

		PlaySound( m_aUserProps[m_nUpgradeLevel].pSoundGroupUse );

		return TRUE;
	}
	else
	{
		// no targets specified, so cancel throw and return projectile
		Throwable_ReturnCleaner();
	}

	return FALSE;
}


BOOL CWeaponCleaner::_GetProjectileFromPoolAndInit( void ) {
	// Get a free projectile...
	m_pProjToThrow = CEProjPool::GetProjectileFromFreePool( m_ahProjPool[m_nUpgradeLevel] );
	if( m_pProjToThrow == NULL ) {
		// No more projectiles...
		return FALSE;
	}

	// Init projectile...

	m_pProjToThrow->Init();
	m_pProjToThrow->SetDamager( this, m_pOwnerBot );
	m_pProjToThrow->SetExplosionGroup( m_aUserProps[m_nUpgradeLevel].hExplosionGroup );
	m_pProjToThrow->SetMaxDistCanTravel( m_aUserProps[m_nUpgradeLevel].fMaxLiveRange );

	return TRUE;
}


BOOL CWeaponCleaner::Throwable_Prime( void ) {
	FASSERT( IsCreated() );

	if( !_GetProjectileFromPoolAndInit() ) {
		// No more projectiles...
		return FALSE;
	}

	RemoveFromClip( 1 );

	return TRUE;
}


void CWeaponCleaner::Throwable_AttachGrenadeToOwnerBotBone( cchar *pszBoneName ) {
	FASSERT( IsCreated() );

	if( m_pProjToThrow && m_pOwnerBot ) {
		m_pProjToThrow->Attach_UnitMtxToParent_PS_NewScale_PS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, 1.5f, TRUE );
		m_pProjToThrow->AddToWorld();
	}
}


// Throws the active grenade toward the specified point.
void CWeaponCleaner::Throwable_ThrowGrenade_TargetPoint( const CFVec3A *pTargetPos_WS, f32 fSpeed) {
	FASSERT( IsCreated() );
	FASSERT( m_pProjToThrow );

	FASSERT_NOW;	// TBD!!!

	m_pProjToThrow->DetachFromParent();

//	m_pProjToThrow->SetLinSpeed( m_aUserProps[m_nUpgradeLevel].fGrenadeSpeed );
//	m_pProjToThrow->SetLinUnitDir_WS( pProjUnitDir_WS );
}


// Throws the active grenade toward the owner bot's mount aim direction.
void CWeaponCleaner::Throwable_ThrowGrenade_MountAimDirection( f32 fSlingSpeed, BOOL bLimbDangling ) {
	f32 fUnitAimPitch, fProjSpeed;
	CFQuatA Quat;
	CFVec3A LaunchVelocity_WS, BotAddVelocity_WS, LaunchUnitVec_WS;

	FASSERT( IsCreated() );
	FASSERT( m_pOwnerBot );
	FASSERT( m_pProjToThrow );

	// Detach from any parent...
	m_pProjToThrow->DetachFromParent();

	// Compute speed...
	fUnitAimPitch = 1.01f - fmath_Div( m_pOwnerBot->m_fMountPitch_WS - m_pOwnerBot->m_fMountPitchMin_WS, m_pOwnerBot->m_fMountPitchMax_WS - m_pOwnerBot->m_fMountPitchMin_WS );
	fUnitAimPitch = fmath_Sin( fUnitAimPitch * FMATH_HALF_PI );
	fProjSpeed = FMATH_FPOT( fUnitAimPitch, 25.0f, 40.0f );

	// Compute velocity vector...
	Quat.BuildQuat( m_pOwnerBot->MtxToWorld()->m_vRight, FMATH_DEG2RAD( -20.0f ) );
	Quat.MulPoint( LaunchVelocity_WS, m_pOwnerBot->MtxToWorld()->m_vFront );
	LaunchVelocity_WS.Mul( fProjSpeed );

	// Add in bot's initial velocity and extract unit trajectory and speed...
	BotAddVelocity_WS = m_pOwnerBot->m_Velocity_MS;
	BotAddVelocity_WS.x = 0.0f;
	m_pOwnerBot->MS2WS( BotAddVelocity_WS );

	LaunchVelocity_WS.Add( BotAddVelocity_WS );
	fProjSpeed = LaunchUnitVec_WS.SafeUnitAndMag( LaunchVelocity_WS );
	if( fProjSpeed <= 0.0f ) {
		fProjSpeed = 0.0f;
		LaunchUnitVec_WS = CFVec3A::m_UnitAxisY;
	}

	m_pProjToThrow->SetLinSpeed( fProjSpeed );
	m_pProjToThrow->SetLinUnitDir_WS( &LaunchUnitVec_WS );

	CFVec3A RotVel_WS;

	if( fmath_RandomChance( 0.5f ) ) {
		RotVel_WS.Set( -fmath_RandomFloatRange( 0.4f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	} else {
		RotVel_WS.Set( fmath_RandomFloatRange( 0.4f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	}
	RotVel_WS.Unitize();

	const _UserProps_t *pUserProps = &m_aUserProps[m_nUpgradeLevel];

	// Launch it!
	m_pProjToThrow->SetRotUnitAxis_WS( &RotVel_WS );
	m_pProjToThrow->SetRotSpeed( fmath_RandomFloatRange( FMATH_DEG2RAD( 180.0f ), FMATH_DEG2RAD( 360.0f ) ) );
	m_pProjToThrow->SetMaxLifeSecs( 4.0f );
	m_pProjToThrow->SmokeTrailOn( &m_CanisterSmokeTrailAttrib );
	m_pProjToThrow->SetSmokePos_MS( &CFVec3A( 0.0f, 0.3f, 0.0f ) );

	CEProjExt::CEProj_Cleaner_Params_t CleanerParams;
	CleanerParams.fRocketMaxDist = pUserProps->fRocketMaxDist;
	CleanerParams.fRocketMaxSecs = pUserProps->fRocketMaxSecs;
	CleanerParams.fRocketMinSpeed = pUserProps->fRocketMinSpeed;
	CleanerParams.fRocketMaxSpeed = pUserProps->fRocketMaxSpeed;
	CleanerParams.fRocketScale = pUserProps->fRocketScale;
	CleanerParams.fRocketSwarmAmp = pUserProps->fRocketSwarmAmp;
	CleanerParams.hRocketExplosionGroup = pUserProps->hRocketExplosionGroup;
	CleanerParams.pSoundGroupWhir = pUserProps->pSoundGroupWhir;
	m_pProjToThrow->SetCleanerParams( &CleanerParams );

	CFMtx43A ProjMtx;
	ProjMtx.UnitMtxFromUnitVec( m_pProjToThrow->GetLinUnitDir_WS() );
	ProjMtx.m_vPos.Set( m_pProjToThrow->MtxToWorld()->m_vPos );

	m_pProjToThrow->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, 1.0f );
	
	m_pProjToThrow->Launch();
	m_pProjToThrow = NULL;
}


void CWeaponCleaner::Throwable_ReturnCleaner( void ) {
	FASSERT( IsCreated() );

	if( m_pProjToThrow ) {
		m_pProjToThrow->DetachFromParent();
		CEProjPool::ReturnProjectileToFreePool( m_pProjToThrow );

		// clear pointer to canister that is used in canister static functions
		CEProj_Cleaner *pCleanerProjectile = (CEProj_Cleaner*) m_pProjToThrow->GetExtensionObject( CEProj::PROJTYPE_CLEANER );
		pCleanerProjectile->ClearTargetingCleaner(this);
		m_pProjToThrow = NULL;

		if( !IsClipFull() ) {
			AddToClip( 1 );
		} else if( !IsReserveFull() ) {
			AddToReserve( 1 );
		}
	}
}


// Called once per frame to update animations, timers, etc.
void CWeaponCleaner::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fSecondsCountdownTimer, 0.0f );
	}
}


void CWeaponCleaner::ClearAllWeaponTargets( void )
{
	if( m_pProjToThrow )
	{
		// untarget all of canister's targets
		CEProj_Cleaner *pCleanerProjectile = (CEProj_Cleaner*) m_pProjToThrow->GetExtensionObject( CEProj::PROJTYPE_CLEANER );
		pCleanerProjectile->InitTargetList();
	}
}

void CWeaponCleaner::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );
	
	if (m_pProjToThrow)
		m_pProjToThrow->DrawEnable(bDrawingHasBeenEnabled);

}
