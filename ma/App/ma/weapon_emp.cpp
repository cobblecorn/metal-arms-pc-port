//////////////////////////////////////////////////////////////////////////////////////
// weapon_emp.cpp - EMP grenade weapon.
//
// Author: Nathan Miller    
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
// 11/18/02 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "weapon_emp.h"
#include "weapon.h"
#include "eproj.h"
#include "iteminst.h"
#include "fresload.h"
#include "fsndfx.h"
#include "bot.h"
#include "ai/aigameutils.h"


#define _USER_PROP_FILENAME		"w_emp.csv"



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponEMPBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponEMPBuilder _WeaponGrenBuilder;


void CWeaponEMPBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONEMP, pszEntityType );
}


BOOL CWeaponEMPBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponEMPBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponEMP
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponEMP::_UserProps_t CWeaponEMP::m_aUserProps[EUK_COUNT_EMP];
CEProjPool::PoolHandle_t CWeaponEMP::m_ahProjPool[EUK_COUNT_EMP];
SmokeTrailAttrib_t CWeaponEMP::m_SmokeTrailAttrib;
CFTexInst CWeaponEMP::m_StreamerTexInst;


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponEMP::m_aUserPropVocab[] = {
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

	// fGrenadeLife:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fGrenadeMeshScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_100000,

	// fFatRadiusDelta
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDetonationRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDetonationLife
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDetonationTime
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroup
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupUse
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupBounce
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEffect


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponEMP::m_aUserPropMapTable[] = {
	"Gren",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	/*"GrenMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"GrenMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],*/

	NULL
};



BOOL CWeaponEMP::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;
	FTexDef_t *pTexDef;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponEMP::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_EMP; i++ ) {
		// Create a pool of projectiles...
		m_ahProjPool[i] = CEProjPool::Create( CEProj::PROJTYPE_GRENADE, m_aUserProps[i].apszMeshName[_MESH_GRENADE], (u32)m_aUserProps[i].fGrenadesInPoolCount );

		if( m_ahProjPool[i] == EPROJPOOL_NULL_HANDLE ) {
			DEVPRINTF( "CWeaponEMP::InitSystem(): Could not create projectile pool.\n" );
			goto _ExitWithError;
		}

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_EMP][i];

		pInfo->nGrip = GRIP_LEFT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = 0.0f;
		pInfo->fMaxTargetAssistDist = 0.0f;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_THROWABLE;
		pInfo->nReticleType = CReticle::TYPE_NONE;
	}

	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tf_plasma01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponEMP::InitSystem(): Could not load smoke trail texture.\n" );
	}

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_ADDITIVE_BLEND_MODE;//SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 0.5f;
	m_SmokeTrailAttrib.fScaleMax_WS = 1.0f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = -0.5f;//0.5f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = -0.8f;//0.8f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.25f;//0.05f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.25f;//0.05f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.2f;//0.1f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.025f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.25f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 0.5f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.2f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -2.5f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 0.16f, 0.54f, 1.0f );
	m_SmokeTrailAttrib.EndColorRGB.Set( 1.0f, 1.0f, 1.0f );
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 0.5f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.7f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.0f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.1f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	// Load streamer texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "trdmstrmry1" );
	m_StreamerTexInst.SetTexDef( pTexDef );
	m_StreamerTexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponEMP::UninitSystem( void ) {
}


CWeaponEMP::CWeaponEMP() {
	_ClearDataMembers();
}


CWeaponEMP::~CWeaponEMP() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponEMP::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponEMPBuilder *pBuilder = (CWeaponEMPBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponGrenBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONEMP );
	}

	// Set our builder parameters...
	
	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_EMP, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponEMP::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponEMPBuilder *pBuilder = (CWeaponEMPBuilder *)GetLeafClassBuilder();

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


BOOL CWeaponEMP::ClassHierarchyBuilt( void ) {
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


void CWeaponEMP::ClassHierarchyDestroy( void ) {
	// Destroy ourselves first...
	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponEMP::GetLeafClassBuilder( void ) {
	return &_WeaponGrenBuilder;
}


void CWeaponEMP::_ClearDataMembers( void ) {
	m_pResourceData = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_pProjToThrow = NULL;
}


void CWeaponEMP::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	if( m_pProjToThrow ) {
		CEProjPool::ReturnProjectileToFreePool( m_pProjToThrow );
		m_pProjToThrow = NULL;
	}
}


void CWeaponEMP::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo );

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NONE );
	}
}


BOOL CWeaponEMP::Throwable_TriggerWork( f32 fUnitTriggerVal ) {
	if( fUnitTriggerVal <= 0.0f ) {
		// Trigger not down...
		return FALSE;
	}

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

	if( !_GetProjectileFromPoolAndInit() ) {
		// No more projectiles...
		return FALSE;
	}

	m_fSecondsCountdownTimer = 2.0f;

	return TRUE;
}


BOOL CWeaponEMP::_GetProjectileFromPoolAndInit( void ) {
	if( m_pProjToThrow ) {
		// Our projectile hasn't been returned yet...
		return FALSE;
	}

	// Get a free projectile...
	m_pProjToThrow = CEProjPool::GetProjectileFromFreePool( m_ahProjPool[m_nUpgradeLevel] );
	if( m_pProjToThrow == NULL ) {
		// No more projectiles...
		return FALSE;
	}

	const _UserProps_t *pUserProps = &m_aUserProps[m_nUpgradeLevel];

	// Init projectile...
	CEProjExt::CEProj_Grenade_Params_t GrenadeParams;

	GrenadeParams.nGrenadeType = CEProjExt::CEPROJ_GRENADE_TYPE_EMP;
	GrenadeParams.FloorDampening.Set( 0.85f, 0.65f, 0.85f );
	GrenadeParams.WallDampening.Set( 0.95f, 0.95f, 0.95f );
	GrenadeParams.bUseAngularWallDampening = TRUE;
	GrenadeParams.afAngularWallDampeningRange[0] = 0.2f;
	GrenadeParams.afAngularWallDampeningRange[1] = 0.4f;
	GrenadeParams.fRadiusPercentUse = 0.6f;
	GrenadeParams.fFatRadiusDelta = pUserProps->fFatRadiusDelta;
	GrenadeParams.fDetonationRadius = pUserProps->fDetonationRadius;
	GrenadeParams.fDetonationLife = pUserProps->fDetonationLife;
	GrenadeParams.fShutdownTime = pUserProps->fShutdownTime;
	GrenadeParams.pSoundGroupBounce = pUserProps->pSoundGroupBounce;
	GrenadeParams.pSoundGroupEMPEffect = pUserProps->pSoundGroupEffect;

	m_pProjToThrow->Init();
	m_pProjToThrow->SetDamager( this, m_pOwnerBot );
	m_pProjToThrow->SetMaxDistCanTravel( pUserProps->fMaxLiveRange );
	m_pProjToThrow->SetExplosionGroup( pUserProps->hExplosionGroup );
	m_pProjToThrow->SetGrenadeParams( &GrenadeParams );
	m_pProjToThrow->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &CFMtx43A::m_IdentityMtx, pUserProps->fGrenadeMeshScale, FALSE );

	return TRUE;
}


BOOL CWeaponEMP::Throwable_Prime( void ) {
	FASSERT( IsCreated() );

	if( !_GetProjectileFromPoolAndInit() ) {
		// No more projectiles...
		return FALSE;
	}

	return TRUE;
}


void CWeaponEMP::Throwable_AttachGrenadeToOwnerBotBone( cchar *pszBoneName ) {
	FASSERT( IsCreated() );

	if( m_pOwnerBot ) {
		if( m_pProjToThrow ) {
			m_pProjToThrow->Attach_UnitMtxToParent_PS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, FALSE );
			m_pProjToThrow->AddToWorld();
		}
	}
}


// Throws the active grenade toward the specified point.
void CWeaponEMP::Throwable_ThrowGrenade_TargetPoint( const CFVec3A *pTargetPos_WS, f32 fSpeed ) {
	FASSERT( IsCreated() );
	FASSERT( m_pProjToThrow );

	CFVec3A vVel;

	aiutils_ComputeTrajectoryFromXZVel( vVel, m_pProjToThrow->MtxToWorld()->m_vPos, *pTargetPos_WS, fSpeed, 32.0f );
	Throwable_ThrowGrenade( m_pProjToThrow, vVel, m_aUserProps[m_nUpgradeLevel].pSoundGroupUse );
	m_pProjToThrow = NULL;
}


// Throws the active grenade toward the owner bot's mount aim direction.
void CWeaponEMP::Throwable_ThrowGrenade_MountAimDirection( f32 fSlingSpeed, BOOL bLimbDangling ) {
	f32 fUnitAimPitch, fProjSpeed;
	CFQuatA Quat;
	CFVec3A LaunchVelocity_WS, BotAddVelocity_WS, LaunchUnitVec_WS;

	FASSERT( IsCreated() );
	FASSERT( m_pOwnerBot );
	FASSERT( m_pProjToThrow );

	// Detach from any parent...
	m_pProjToThrow->DetachFromParent();

	if( fSlingSpeed < 0.0f ) {
		f32 fRandomSpeedMult = 1.0f;
		f32 fRandomAngleSum = 0.0f;

		if( bLimbDangling ) {
			fRandomSpeedMult = fmath_RandomFloatRange( 0.5f, 0.75f );
			fRandomAngleSum = fmath_RandomFloatRange( FMATH_DEG2RAD(-40.0f), FMATH_DEG2RAD(40.0f) );
		}

		// Compute speed...
		fUnitAimPitch = 1.01f - fmath_Div( m_pOwnerBot->m_fMountPitch_WS - m_pOwnerBot->m_fMountPitchMin_WS, m_pOwnerBot->m_fMountPitchMax_WS - m_pOwnerBot->m_fMountPitchMin_WS );
		fUnitAimPitch = fmath_Sin( fUnitAimPitch * FMATH_HALF_PI );
		fProjSpeed = fRandomSpeedMult * FMATH_FPOT( fUnitAimPitch, 25.0f, 60.0f );

		// Compute velocity vector...
		Quat.BuildQuat( m_pOwnerBot->MtxToWorld()->m_vRight, fRandomAngleSum + FMATH_DEG2RAD( -20.0f ) );
		Quat.MulPoint( LaunchVelocity_WS, m_pOwnerBot->MtxToWorld()->m_vFront );
		LaunchVelocity_WS.Mul( fProjSpeed );
	} else {
		fProjSpeed = fSlingSpeed;

		Quat.BuildQuat( m_pOwnerBot->MtxToWorld()->m_vRight, FMATH_DEG2RAD( -10.0f ) );
		Quat.MulPoint( LaunchVelocity_WS, m_pOwnerBot->MtxToWorld()->m_vFront );
		LaunchVelocity_WS.Mul( fProjSpeed );

		m_pProjToThrow->SetDetonateOnImpact( TRUE );
	}

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
		RotVel_WS.Set( -fmath_RandomFloatRange( 0.1f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	} else {
		RotVel_WS.Set( fmath_RandomFloatRange( 0.1f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	}
	RotVel_WS.Unitize();

	// Launch it!
	m_pProjToThrow->SetRotUnitAxis_WS( &RotVel_WS );
	m_pProjToThrow->SetRotSpeed( fmath_RandomFloatRange( FMATH_DEG2RAD( 180.0f ), FMATH_DEG2RAD( 360.0f ) ) );
	m_pProjToThrow->SetMaxLifeSecs( m_aUserProps[m_nUpgradeLevel].fGrenadeLife );

	if( fSlingSpeed < 0.0f ) {
		m_pProjToThrow->SmokeTrailOn( &m_SmokeTrailAttrib );
		m_pProjToThrow->SetSmokePos_MS( &CFVec3A( 0.0f, 0.5f, 0.0f ) );
	} else {
		m_pProjToThrow->StreamerOn( &m_StreamerTexInst, 0.15f, CFXStreamerEmitter::USE_RIGHT_AXIS, 1.0f, 10 );
	}

	m_pProjToThrow->SetSkipListCallback( _BuildSkipList );
	m_pProjToThrow->Launch();
	m_pProjToThrow = NULL;

	// Remove the round we launched from the clip...
	RemoveFromClip( 1 );

	PlaySound( m_aUserProps[m_nUpgradeLevel].pSoundGroupUse );
}


void CWeaponEMP::Throwable_ReturnGrenade( void ) {
	FASSERT( IsCreated() );

	if( m_pProjToThrow ) {
		m_pProjToThrow->DetachFromParent();
		CEProjPool::ReturnProjectileToFreePool( m_pProjToThrow );
		m_pProjToThrow = NULL;
	}
}


// Called once per frame to update animations, timers, etc.
void CWeaponEMP::ClassHierarchyWork( void ) {
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


void CWeaponEMP::_BuildSkipList( CEProj *pProj ) {
	CWeaponEMP *pWeaponGren = (CWeaponEMP *)pProj->GetDamagerWeapon();
	pWeaponGren->m_pOwnerBot->AppendTrackerSkipList();
}

void CWeaponEMP::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );
	
	if (m_pProjToThrow)
		m_pProjToThrow->DrawEnable(bDrawingHasBeenEnabled);

}
