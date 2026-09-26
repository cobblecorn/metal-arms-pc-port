//////////////////////////////////////////////////////////////////////////////////////
// weapon_rocket.cpp - Rocket launcher weapon.
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
#include "weapon_rocket.h"
#include "fgamedata.h"
#include "fresload.h"
#include "fclib.H"
#include "fworld.h"
#include "fmesh.h"
#include "fanim.h"
#include "meshtypes.h"
#include "floop.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "explosion.h"
#include "iteminst.h"
#include "fsndfx.h"
#include "AI/AIEnviro.h"
#include "weapon_scope.h"
#include "damage.h"
#include "LightPool.h"
#include "fsound.h"



#define _USER_PROP_FILENAME		"w_rocket.csv"



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRocketBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponRocketBuilder _WeaponRocketBuilder;


void CWeaponRocketBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONROCKET, pszEntityType );
}


BOOL CWeaponRocketBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponRocketBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRocket
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponRocket::_UserProps_t CWeaponRocket::m_aUserProps[EUK_COUNT_ROCKET_LAUNCHER];
CDamageProfile *CWeaponRocket::m_apDamageProfile[EUK_COUNT_ROCKET_LAUNCHER];
CFTexInst CWeaponRocket::m_aCoronaTexInst[EUK_COUNT_ROCKET_LAUNCHER];
CFMtx43A *CWeaponRocket::m_pCartridgeSlotMtxArray;
CFMtx43A CWeaponRocket::m_RL23_MtxRotZ;
CWeaponRocket *CWeaponRocket::m_pRL23_CallbackThis;
SmokeTrailAttrib_t CWeaponRocket::m_aSmokeTrailAttrib[EUK_COUNT_ROCKET_LAUNCHER];
//SmokeTrailAttrib_t CWeaponRocket::m_RearTubeSmokeTrailAttrib;

// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponRocket::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_WEAPON]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszMeshName[_MESH_ROCKET]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszAnimName[_ANIM_RELOAD_OPEN]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszAnimName[_ANIM_RELOAD_CLOSE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszAnimName[_ANIM_DEPLOY]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszAnimName[_ANIM_STOW]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fTimeMulReloadOpen:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	// fTimeMulReloadClose:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	// fTimeMulDeploy:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	// fTimeMulStow:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

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

	// fWeaponCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fShotSpreadFactor:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// afScopeSpreadMult[0]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// afScopeSpreadMult[1]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

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
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fMinTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMaxTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// pszTubeBoneName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszRL23_CartridgeBoneName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fRL23_AngleBetweenCartridgeSlots:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fRL23_NoAmmoBaseAngle:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fRL23_CartridgeRocketScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_100000,

	// fRL23_CartridgeRocketPosOffsetZ:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fRL23_CartridgeRocketPosRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fRL23_ReloadTimeForOneRound:
	// fRL23_OOReloadTimeForOneRound:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_Pt01,
	F32_DATATABLE_60,

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroupImpact
	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroupFire

	// pszCoronaTexName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fCoronaScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_100000,

	// fCoronaUnitScreenspaceScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_100000,

	// fCoronaColorRed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fCoronaColorGreen:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fCoronaColorBlue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fLightColorRed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fLightColorGreen:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fLightColorBlue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fLightRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_1000,

	// fProjectileMeshScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_1000,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupReload
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupLoop

	EPROJ_MERV_STATICPARAMS_GAMEDATA_VOCAB,	// MervStaticParams

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponRocket::m_aUserPropMapTable[] = {
	"RocketL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"RocketL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"RocketL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"RocketMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],

	"RocketMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[4],

	"Rocket_Buddy",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[5],

	NULL
};



BOOL CWeaponRocket::InitSystem( void ) {
	Info_t *pInfo;
	f32 fFireRecoveryTime;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	m_RL23_MtxRotZ.Identity();

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponRocket::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		return FALSE;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_ROCKET_LAUNCHER; i++ ) {
		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_ROCKET_LAUNCHER][i];

		if( i==1 || i==2 ) {
			if( m_aUserProps[i].fClipAmmoMax > 0 ) {
				FMATH_CLAMPMAX( m_aUserProps[i].fClipAmmoMax, 10.0f );
			} else {
				m_aUserProps[i].fClipAmmoMax = 4.0f;
			}

			// RL2 and RL3...
			pInfo->nGrip = GRIP_RIGHT_ARM;
			pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
			pInfo->nStanceType = STANCE_TYPE_SHOULDER_MOUNT;

			// Fixup reload time stuff for RL2 and RL3...
			fFireRecoveryTime = m_aUserProps[i].fOORoundsPerSec;
			if( m_aUserProps[i].fRL23_ReloadTimeForOneRound < (fFireRecoveryTime + 0.001f) ) {
				m_aUserProps[i].fRL23_ReloadTimeForOneRound = 0.001f;
			} else {
				m_aUserProps[i].fRL23_ReloadTimeForOneRound -= fFireRecoveryTime;
			}
			m_aUserProps[i].fRL23_OOReloadTimeForOneRound = 1.0f / m_aUserProps[i].fRL23_ReloadTimeForOneRound;
		} else {
			// RL1...
			pInfo->nGrip = GRIP_BOTH_ARMS;
			pInfo->nReloadType = RELOAD_TYPE_ROCKET1;
			pInfo->nStanceType = STANCE_TYPE_TWO_HANDED_TORSO_TWIST;
		}

		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = (pInfo->nStanceType == STANCE_TYPE_TWO_HANDED_TORSO_TWIST) ? INFOFLAG_LEFT_HAND_RELOADS : INFOFLAG_SCOPE_ENABLED;
		pInfo->nInfoFlags |= INFOFLAG_THICK_TARGETING | INFOFLAG_DISPLAY_AMMO_TEXT;
		pInfo->nReticleType = CReticle::TYPE_DROID_STANDARD;

		// Load corona texture...
		m_aCoronaTexInst[i].SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_aUserProps[i].pszCoronaTexName ) );
	}

	// RL2 and RL3 have visual rockets on their clips.
	// Allocate an array of matrices to hold the rocket orientations within the cartridge...
	m_pCartridgeSlotMtxArray = fnew CFMtx43A[ m_aaInfo[WEAPON_TYPE_ROCKET_LAUNCHER][1].nClipAmmoMax ];
	if( m_pCartridgeSlotMtxArray == NULL ) {
		DEVPRINTF( "CWeaponRocket::InitSystem(): Not enough memory to allocate m_pCartridgeSlotMtxArray.\n" );
		goto _ExitWithError;
	}

	if( !_RL23_ComputeCartridgeRocketMatrices() ) {
		goto _ExitWithError;
	}

	_RL1_SetSmokeTrailAttributes( 0 );
	_RL2_SetSmokeTrailAttributes();
	_RL3_SetSmokeTrailAttributes();
	_RL1_SetSmokeTrailAttributes( 3 );
	_RL1_SetSmokeTrailAttributes( 4 );

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponRocket::UninitSystem( void ) {
}


void CWeaponRocket::_RL1_SetSmokeTrailAttributes( u32 uUpgradeLevel ) {
	FASSERT( uUpgradeLevel < EUK_COUNT_ROCKET_LAUNCHER );

	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRocket::_RL1_SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	m_aSmokeTrailAttrib[uUpgradeLevel].nFlags = SMOKETRAIL_FLAG_NONE;
	m_aSmokeTrailAttrib[uUpgradeLevel].pTexDef = pTexDef;

	m_aSmokeTrailAttrib[uUpgradeLevel].fScaleMin_WS = 0.5f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fScaleMax_WS = 0.6f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fScaleSpeedMin_WS = 1.2f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fScaleSpeedMax_WS = 1.5f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fScaleAccelMin_WS = -1.0f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fScaleAccelMax_WS = -1.5f;

	m_aSmokeTrailAttrib[uUpgradeLevel].fXRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fYRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fDistBetweenPuffs_WS = 0.4f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_aSmokeTrailAttrib[uUpgradeLevel].fYSpeedMin_WS = 0.5f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fYSpeedMax_WS = 1.0f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fYAccelMin_WS = -0.2f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fYAccelMax_WS = -0.5f;

	m_aSmokeTrailAttrib[uUpgradeLevel].fUnitOpaqueMin_WS = 0.5f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fUnitOpaqueMax_WS = 0.7f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fUnitOpaqueSpeedMin_WS = -0.5f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fUnitOpaqueSpeedMax_WS = -0.9f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fUnitOpaqueAccelMin_WS = 0.0f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fUnitOpaqueAccelMax_WS = 0.0f;

	m_aSmokeTrailAttrib[uUpgradeLevel].StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_aSmokeTrailAttrib[uUpgradeLevel].EndColorRGB.White();
	m_aSmokeTrailAttrib[uUpgradeLevel].fStartColorUnitIntensityMin = 1.0f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fStartColorUnitIntensityMax = 0.9f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fEndColorUnitIntensityMin = 0.2f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fEndColorUnitIntensityMax = 0.6f;

	m_aSmokeTrailAttrib[uUpgradeLevel].fColorUnitSliderSpeedMin = 3.5f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fColorUnitSliderSpeedMax = 4.5f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fColorUnitSliderAccelMin = 0.0f;
	m_aSmokeTrailAttrib[uUpgradeLevel].fColorUnitSliderAccelMax = 0.0f;
}


void CWeaponRocket::_RL2_SetSmokeTrailAttributes( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRocket::_RL1_SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	m_aSmokeTrailAttrib[1].nFlags = SMOKETRAIL_FLAG_NONE;
	m_aSmokeTrailAttrib[1].pTexDef = pTexDef;

	m_aSmokeTrailAttrib[1].fScaleMin_WS = 0.5f;
	m_aSmokeTrailAttrib[1].fScaleMax_WS = 0.6f;
	m_aSmokeTrailAttrib[1].fScaleSpeedMin_WS = 1.2f;
	m_aSmokeTrailAttrib[1].fScaleSpeedMax_WS = 1.5f;
	m_aSmokeTrailAttrib[1].fScaleAccelMin_WS = -1.0f;
	m_aSmokeTrailAttrib[1].fScaleAccelMax_WS = -1.5f;

	m_aSmokeTrailAttrib[1].fXRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib[1].fYRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib[1].fDistBetweenPuffs_WS = 0.4f;
	m_aSmokeTrailAttrib[1].fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_aSmokeTrailAttrib[1].fYSpeedMin_WS = 0.5f;
	m_aSmokeTrailAttrib[1].fYSpeedMax_WS = 1.0f;
	m_aSmokeTrailAttrib[1].fYAccelMin_WS = -0.2f;
	m_aSmokeTrailAttrib[1].fYAccelMax_WS = -0.5f;

	m_aSmokeTrailAttrib[1].fUnitOpaqueMin_WS = 0.5f;
	m_aSmokeTrailAttrib[1].fUnitOpaqueMax_WS = 0.7f;
	m_aSmokeTrailAttrib[1].fUnitOpaqueSpeedMin_WS = -0.5f;
	m_aSmokeTrailAttrib[1].fUnitOpaqueSpeedMax_WS = -0.9f;
	m_aSmokeTrailAttrib[1].fUnitOpaqueAccelMin_WS = 0.0f;
	m_aSmokeTrailAttrib[1].fUnitOpaqueAccelMax_WS = 0.0f;

	m_aSmokeTrailAttrib[1].StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_aSmokeTrailAttrib[1].EndColorRGB.White();
	m_aSmokeTrailAttrib[1].fStartColorUnitIntensityMin = 1.0f;
	m_aSmokeTrailAttrib[1].fStartColorUnitIntensityMax = 0.9f;
	m_aSmokeTrailAttrib[1].fEndColorUnitIntensityMin = 0.2f;
	m_aSmokeTrailAttrib[1].fEndColorUnitIntensityMax = 0.6f;

	m_aSmokeTrailAttrib[1].fColorUnitSliderSpeedMin = 3.5f;
	m_aSmokeTrailAttrib[1].fColorUnitSliderSpeedMax = 4.5f;
	m_aSmokeTrailAttrib[1].fColorUnitSliderAccelMin = 0.0f;
	m_aSmokeTrailAttrib[1].fColorUnitSliderAccelMax = 0.0f;

#if 0
	// Tube smoke:
	m_RearTubeSmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_RearTubeSmokeTrailAttrib.pTexDef = pTexDef;

	m_RearTubeSmokeTrailAttrib.fScaleMin_WS = 0.25f;
	m_RearTubeSmokeTrailAttrib.fScaleMax_WS = 0.3f;
	m_RearTubeSmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_RearTubeSmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_RearTubeSmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_RearTubeSmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_RearTubeSmokeTrailAttrib.fXRandSpread_WS = 0.1f;
	m_RearTubeSmokeTrailAttrib.fYRandSpread_WS = 0.1f;
	m_RearTubeSmokeTrailAttrib.fDistBetweenPuffs_WS = 0.1f;
	m_RearTubeSmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.0f;

	m_RearTubeSmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_RearTubeSmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_RearTubeSmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_RearTubeSmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_RearTubeSmokeTrailAttrib.fUnitOpaqueMin_WS = 0.5f;
	m_RearTubeSmokeTrailAttrib.fUnitOpaqueMax_WS = 0.7f;
	m_RearTubeSmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_RearTubeSmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_RearTubeSmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_RearTubeSmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_RearTubeSmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_RearTubeSmokeTrailAttrib.EndColorRGB.White();
	m_RearTubeSmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_RearTubeSmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_RearTubeSmokeTrailAttrib.fEndColorUnitIntensityMin = 0.2f;
	m_RearTubeSmokeTrailAttrib.fEndColorUnitIntensityMax = 0.6f;

	m_RearTubeSmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_RearTubeSmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_RearTubeSmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_RearTubeSmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
#endif
}


void CWeaponRocket::_RL3_SetSmokeTrailAttributes( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRocket::_RL1_SetSmokeTrailAttributes(): Could not load smoke trail texture.\n" );
	}

	m_aSmokeTrailAttrib[2].nFlags = SMOKETRAIL_FLAG_NONE;
	m_aSmokeTrailAttrib[2].pTexDef = pTexDef;

	m_aSmokeTrailAttrib[2].fScaleMin_WS = 0.5f;
	m_aSmokeTrailAttrib[2].fScaleMax_WS = 0.6f;
	m_aSmokeTrailAttrib[2].fScaleSpeedMin_WS = 1.2f;
	m_aSmokeTrailAttrib[2].fScaleSpeedMax_WS = 1.5f;
	m_aSmokeTrailAttrib[2].fScaleAccelMin_WS = -1.0f;
	m_aSmokeTrailAttrib[2].fScaleAccelMax_WS = -1.5f;

	m_aSmokeTrailAttrib[2].fXRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib[2].fYRandSpread_WS = 0.2f;
	m_aSmokeTrailAttrib[2].fDistBetweenPuffs_WS = 0.4f;
	m_aSmokeTrailAttrib[2].fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_aSmokeTrailAttrib[2].fYSpeedMin_WS = 0.5f;
	m_aSmokeTrailAttrib[2].fYSpeedMax_WS = 1.0f;
	m_aSmokeTrailAttrib[2].fYAccelMin_WS = -0.2f;
	m_aSmokeTrailAttrib[2].fYAccelMax_WS = -0.5f;

	m_aSmokeTrailAttrib[2].fUnitOpaqueMin_WS = 0.5f;
	m_aSmokeTrailAttrib[2].fUnitOpaqueMax_WS = 0.7f;
	m_aSmokeTrailAttrib[2].fUnitOpaqueSpeedMin_WS = -0.5f;
	m_aSmokeTrailAttrib[2].fUnitOpaqueSpeedMax_WS = -0.9f;
	m_aSmokeTrailAttrib[2].fUnitOpaqueAccelMin_WS = 0.0f;
	m_aSmokeTrailAttrib[2].fUnitOpaqueAccelMax_WS = 0.0f;

	m_aSmokeTrailAttrib[2].StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_aSmokeTrailAttrib[2].EndColorRGB.White();
	m_aSmokeTrailAttrib[2].fStartColorUnitIntensityMin = 1.0f;
	m_aSmokeTrailAttrib[2].fStartColorUnitIntensityMax = 0.9f;
	m_aSmokeTrailAttrib[2].fEndColorUnitIntensityMin = 0.2f;
	m_aSmokeTrailAttrib[2].fEndColorUnitIntensityMax = 0.6f;

	m_aSmokeTrailAttrib[2].fColorUnitSliderSpeedMin = 3.5f;
	m_aSmokeTrailAttrib[2].fColorUnitSliderSpeedMax = 4.5f;
	m_aSmokeTrailAttrib[2].fColorUnitSliderAccelMin = 0.0f;
	m_aSmokeTrailAttrib[2].fColorUnitSliderAccelMax = 0.0f;
}


BOOL CWeaponRocket::_RL23_ComputeCartridgeRocketMatrices( void ) {
	f32 fAngle, fOffsetDownCartridge, fRadius, fSin, fCos;
	u32 i;
	CFMtx43A ScaleMtx;

	fRadius = m_aUserProps[1].fRL23_CartridgeRocketPosRadius;
	fOffsetDownCartridge = m_aUserProps[1].fRL23_CartridgeRocketPosOffsetZ;

	ScaleMtx.Identity();
	ScaleMtx.Mul( m_aUserProps[1].fRL23_CartridgeRocketScale );

	for( i=0, fAngle=0.0f; i<m_aaInfo[WEAPON_TYPE_ROCKET_LAUNCHER][1].nClipAmmoMax; ++i, fAngle+=m_aUserProps[1].fRL23_AngleBetweenCartridgeSlots ) {
		fmath_SinCos( fAngle, &fSin, &fCos );

		m_pCartridgeSlotMtxArray[i].Identity();
		m_pCartridgeSlotMtxArray[i].SetRotationZ( fAngle );
		m_pCartridgeSlotMtxArray[i].m_vPos.Set( fRadius*fCos, fRadius*fSin, fOffsetDownCartridge );
		m_pCartridgeSlotMtxArray[i].Mul( ScaleMtx );
	}

	return TRUE;
}


CWeaponRocket::CWeaponRocket() {
	_ClearDataMembers();
}


CWeaponRocket::~CWeaponRocket() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponRocket::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponRocketBuilder *pBuilder = (CWeaponRocketBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponRocketBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONROCKET );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_ROCKET_LAUNCHER, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponRocket::ClassHierarchyBuild( void ) {
	u32 i, j;
	FAnim_t *pAnim;
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	_ResourceData_t *pResourceData;
	Info_t *pInfo;
	u32 uMeshEUK;
	u32 uResEUK;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponRocketBuilder *pBuilder = (CWeaponRocketBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_ROCKET_LAUNCHER; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_ROCKET_LAUNCHER );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		pInfo = &m_aaInfo[WEAPON_TYPE_ROCKET_LAUNCHER][i];

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponRocket::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );

			pMesh = m_pErrorMesh;
			if( pMesh == NULL ) {
				goto _ExitWithError;
			}
		}

		// Init the world mesh...
		MeshInit.pMesh = pMesh;
		MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist = m_aUserProps[i].fWeaponCullDist;
		MeshInit.Mtx = *MtxToWorld();
		
		uResEUK = i;
		if( !CreateSharedEUKData( &MeshInit, &uResEUK, &(pResourceData->m_pWorldMesh), &(pResourceData->m_pAnimCombiner), NULL ) ) {
			DEVPRINTF( "CWeaponRocket::ClassHierarchyBuild():  Error creating shared EUK data\n" );
			goto _ExitWithError;
		}

		if( uResEUK == i ) {	// we just loaded the data, init everything
			pResourceData->m_pWorldMesh->RemoveFromWorld();
			pResourceData->m_pWorldMesh->SetCollisionFlag( TRUE );
			pResourceData->m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
			pResourceData->m_pWorldMesh->m_pUser = this;
			pResourceData->m_pWorldMesh->SetUserTypeBits( TypeBits() );
			pResourceData->m_pWorldMesh->SetLineOfSightFlag(FALSE); //pgm added this so that weapons never block los tests

			if( !pResourceData->m_pAnimCombiner->CreateSimple( NULL, pResourceData->m_pWorldMesh ) ) {
				DEVPRINTF( "CWeaponRocket::Create(): Could not create animation combiner.\n" );
				goto _ExitWithError;
			}

			// RL2 and RL3 require a bone anim callback...
			if( pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
				pResourceData->m_pAnimCombiner->SetBoneCallback( _CartridgeBoneCallback );
				pResourceData->m_pAnimCombiner->DisableAllBoneCallbacks();
				pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[i].pszRL23_CartridgeBoneName );
			}
		}

		
		pResourceData->m_nCombinerTapID = pResourceData->m_pAnimCombiner->GetTapID( FANIM_SIMPLE_COMBINER_CONFIG_TAPNAME );
		if( pResourceData->m_nCombinerTapID == -1 ) {
			DEVPRINTF( "CWeaponRocket::Create(): Could not obtain combiner tap ID.\n" );
			goto _ExitWithError;
		}

		// Load our animations...
		for( j=0; j<_ANIM_COUNT; ++j ) {
			// Load the animation resource...
			if( m_aUserProps[i].apszAnimName[j]==NULL || m_aUserProps[i].apszAnimName[j][0]==0 || !fclib_stricmp( m_aUserProps[i].apszAnimName[j], "none" ) ) {
				// No animation provided...
				pAnim = NULL;
			} else {
				pAnim = (FAnim_t *)fresload_Load( FANIM_RESNAME, m_aUserProps[i].apszAnimName[j] );
				if( pAnim == NULL ) {
					DEVPRINTF( "CWeaponRocket::Create(): Could not find anim '%s'.\n", m_aUserProps[i].apszAnimName[j] );
				}
			}

			// Create our animation instance...
			if( pAnim ) {
				pResourceData->m_apAnimInstArray[j] = fnew CFAnimInst;
				if( pResourceData->m_apAnimInstArray[j] == NULL ) {
					DEVPRINTF( "CWeaponRocket::Create(): Not enough memory to create m_apAnimInstArray.\n" );
					goto _ExitWithError;
				}

				if( !pResourceData->m_apAnimInstArray[j]->Create( pAnim ) ) {
					DEVPRINTF( "CWeaponRocket::Create(): Could not create animation instance.\n" );
					goto _ExitWithError;
				}
			}
		}

		// RL2 and RL3 have visual rockets on its clip.
		// Create the rockets...
//		if( pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		if( (uMeshEUK == 1) || (uMeshEUK == 2) ) {
   			FASSERT( pInfo->nClipAmmoMax );

			// Create an array of world meshes (one world mesh per cartridge rocket)...
			pResourceData->m_pCartridgeRocketWorldMeshArray = fnew CFWorldMesh[ pInfo->nClipAmmoMax ];
			if( pResourceData->m_pCartridgeRocketWorldMeshArray == NULL ) {
				DEVPRINTF( "CWeaponRocket::Create(): Not enough memory to create pResourceData->m_pCartridgeRocketWorldMeshArray.\n", i );
				goto _ExitWithError;
			}

			pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[i].apszMeshName[_MESH_ROCKET] );
			if( pMesh == NULL ) {
				DEVPRINTF( "CWeaponRocket::Create(): Could not find mesh '%s'.\n", m_aUserProps[i].apszMeshName[_MESH_ROCKET] );

				pMesh = m_pErrorMesh;
				if( pMesh == NULL ) {
					goto _ExitWithError;
				}
			}

			MeshInit.pMesh = pMesh;
			MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
			MeshInit.fCullDist = m_aUserProps[i].fRocketCullDist;
			MeshInit.Mtx.Identity();

			for( j=0; j<pInfo->nClipAmmoMax; ++j ) {
				// Init the world mesh...
				pResourceData->m_pCartridgeRocketWorldMeshArray[j].Init( &MeshInit );
				pResourceData->m_pCartridgeRocketWorldMeshArray[j].RemoveFromWorld();
				pResourceData->m_pCartridgeRocketWorldMeshArray[j].SetCollisionFlag( FALSE );
			}

			// Find cartridge bone index...
			m_nRL23_CartridgeBoneIndex = m_aResourceData[i].m_pWorldMesh->FindBone( m_aUserProps[i].pszRL23_CartridgeBoneName );
			if( m_nRL23_CartridgeBoneIndex < 0 ) {
				DEVPRINTF( "CWeaponRocket::Create(): Could not find bone '%s' on rocket launcher L%u.\n", m_aUserProps[i].pszRL23_CartridgeBoneName, i );
				goto _ExitWithError;
			}
		}

		// Find tube bone index...
		m_aResourceData[i].m_nTubeBoneIndex = m_aResourceData[i].m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].pszTubeBoneName );
		if( m_aResourceData[i].m_nTubeBoneIndex < 0 ) {
			DEVPRINTF( "CWeaponRocket::Create(): Could not find bone '%s' on rocket launcher L%u.\n", m_aUserProps[uMeshEUK].pszTubeBoneName, i );
			goto _ExitWithError;
		}
	}

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	fforce_NullHandle( &m_hForce );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponRocket::ClassHierarchyBuilt( void ) {
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


void CWeaponRocket::ClassHierarchyDestroy( void ) {
	u32 i, j;

	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	for( i=0; i<EUK_COUNT_ROCKET_LAUNCHER; ++i ) {
		DestroySharedEUKData( i );

		for( j=0; j<_ANIM_COUNT; ++j ) {
			fdelete( m_aResourceData[i].m_apAnimInstArray[j] );
		}

		fdelete_array( m_aResourceData[i].m_pCartridgeRocketWorldMeshArray );
	}

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponRocket::GetLeafClassBuilder( void ) {
	return &_WeaponRocketBuilder;
}


void CWeaponRocket::_ClearDataMembers( void ) {
	u32 i, j;

	for( i=0; i<EUK_COUNT_ROCKET_LAUNCHER; ++i ) {
		m_aResourceData[i].m_pWorldMesh = NULL;
		m_aResourceData[i].m_pAnimCombiner = NULL;

		for( j=0; j<_ANIM_COUNT; ++j ) {
			m_aResourceData[i].m_apAnimInstArray[j] = NULL;
		}

		m_aResourceData[i].m_nCombinerTapID = -1;
		m_aResourceData[i].m_nCombinerTapID2 = -1;
		m_aResourceData[i].m_pCartridgeRocketWorldMeshArray = NULL;
		m_aResourceData[i].m_nTubeBoneIndex = -1;
	}

	fforce_NullHandle( &m_hForce );

	m_pResourceData = NULL;
	m_pAnimInstCurrentlyPlaying = NULL;
	m_fAnimInstTimeMul = 1.0f;
	m_fSecondsCountdownTimer = 0.0f;

	m_pRL23_CallbackThis = NULL;
	m_nRL23_CartridgeBoneIndex = -1;
	m_nRL23_CartMode = _RL23_CARTMODE_IDLE;
	m_fRL23_CartridgeRotationAngle = 0.0f;
	m_fRL23_CartridgeRotAngleStart = 0.0f;
	m_fRL23_CartridgeRotAngleEnd = 0.0f;
	m_fRL23_CartridgeSwingAngle = 0.0f;
	m_bRL23_ReloadRequest = FALSE;
//	m_hTubeSmokeTrail = SMOKETRAIL_NULLHANDLE;
//	m_fRL23_TubeSmokeSecs = 0.0f;
}


void CWeaponRocket::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponRocket::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst , bUpdateItemInstAmmoFromWeaponAmmo);

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NUMERIC );
	}
}


void CWeaponRocket::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pWorldMesh->UpdateTracker();
}


void CWeaponRocket::ClassHierarchyRemoveFromWorld( void ) {
	u32 i;

	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();

	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		// Remove all cartridge rockets from the world for RL2 and RL3...

		FASSERT( m_pResourceData->m_pCartridgeRocketWorldMeshArray );

		for( i=0; i<m_pInfo->nClipAmmoMax; ++i ) {
			m_pResourceData->m_pCartridgeRocketWorldMeshArray[i].RemoveFromWorld();
		}
	}
}


void CWeaponRocket::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	u32 i;

	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this weapon...

		for( i=0; i<EUK_COUNT_ROCKET_LAUNCHER; ++i ) {
			FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		// Enable drawing of rockets...
		if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
			FASSERT( m_pResourceData->m_pCartridgeRocketWorldMeshArray );
			for( i=0; i<m_pInfo->nClipAmmoMax; ++i ) {
				FMATH_CLEARBITMASK( m_pResourceData->m_pCartridgeRocketWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}
	} else {
		// Disable drawing of this weapon...
		for( i=0; i<EUK_COUNT_ROCKET_LAUNCHER; ++i ) {
			FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		// Disable drawing of rockets...
		if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
			FASSERT( m_pResourceData->m_pCartridgeRocketWorldMeshArray );
			for( i=0; i<m_pInfo->nClipAmmoMax; ++i ) {
				FMATH_SETBITMASK( m_pResourceData->m_pCartridgeRocketWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}
	}
}


void CWeaponRocket::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		_ComputeMtxPalette();
	}
}


void CWeaponRocket::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;

	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		// For RL2 and RL3, add our cartridge rockets...

		FASSERT( m_pResourceData->m_pCartridgeRocketWorldMeshArray );
		for( u32 i=0; i<GetClipAmmo(); ++i ) {
			FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
			apTrackerSkipList[nTrackerSkipListCount++] = &m_pResourceData->m_pCartridgeRocketWorldMeshArray[i];
		}
	}
}


void CWeaponRocket::_BuildRocketSkipList( CEProj *pProj ) {
	CWeaponRocket *pWeaponRocket = (CWeaponRocket *)pProj->GetDamagerWeapon();

	if( pWeaponRocket->m_pOwnerBot ) {
		pWeaponRocket->m_pOwnerBot->AppendTrackerSkipList();
	}
}


// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponRocket::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	u32 i;

	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_ROCKET_LAUNCHER][nPreviousUpgradeLevel];

	// Remove old mesh from world...
	pOldResourceData->m_pWorldMesh->RemoveFromWorld();

	if( pOldResourceData->m_pCartridgeRocketWorldMeshArray ) {
		// Remove all RL2 and RL3 cartridge rockets from world...
		FASSERT( m_pResourceData->m_pCartridgeRocketWorldMeshArray );

		for( i=0; i<pOldInfo->nClipAmmoMax; ++i ) {
			pOldResourceData->m_pCartridgeRocketWorldMeshArray[i].RemoveFromWorld();
		}
	}

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	TransferFromReserveToClip( INFINITE_AMMO, FALSE );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}
}


CFMtx43A *CWeaponRocket::_GetTubeMtx( void ) const {
	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_pResourceData->m_nTubeBoneIndex ];
}


void CWeaponRocket::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	const CFMtx43A *pTubeBoneMtx = _GetTubeMtx();

	pMuzzlePoint_WS->Mul( pTubeBoneMtx->m_vZ, m_aUserProps[m_nUpgradeLevel].fDistFromWeaponOrigToMuzzle ).Add( pTubeBoneMtx->m_vPos );
}

void CWeaponRocket::ComputeMuzzleUnitDir_WS( CFVec3A *pMuzzleUnitDir_WS ) const {
	FASSERT( IsCreated() );

	const CFMtx43A *pTubeBoneMtx = _GetTubeMtx();

	*pMuzzleUnitDir_WS = pTubeBoneMtx->m_vFront;
}


void CWeaponRocket::_InitProjectile( CEProj *pProj, const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bNoDynamicLight ) {
	CFMtx43A ProjMtx;
	const _UserProps_t *pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	ProjMtx.UnitMtxFromUnitVec( pUnitDir_WS );
	ProjMtx.m_vPos = *pPos_WS;

	pProj->Init();
	pProj->SetDamager( this, m_pOwnerBot );
	pProj->SetDamageProfile( m_apDamageProfile[ m_nUpgradeLevel ] );
	pProj->SetSkipListCallback( _BuildRocketSkipList );
	pProj->SetMaxDistCanTravel( pUserProps->fMaxLiveRange );
	pProj->SetLinSpeed( pUserProps->fRocketSpeed );
	pProj->SetLinUnitDir_WS( pUnitDir_WS );
	pProj->SmokeTrailOn( &m_aSmokeTrailAttrib[m_nUpgradeLevel] );
	pProj->SetExplosionGroup( pUserProps->hExplosionGroupImpact );
	pProj->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &ProjMtx, pUserProps->fProjectileMeshScale );
	pProj->LoopingSoundOn( pUserProps->pSoundGroupLoop );

	// Create corona...
	CFWorldLightItem *pWorldLightItem = CLightPool::GetFromFreePool();
	if( pWorldLightItem ) {
		pWorldLightItem->m_Light.InitOmniLight( &ProjMtx.m_vPos.v3, pUserProps->fLightRadius );

		FMATH_SETBITMASK( pWorldLightItem->m_Light.m_nFlags, FLIGHT_FLAG_CORONA | FLIGHT_FLAG_CORONA_WORLDSPACE | FLIGHT_FLAG_CORONA_ONLY );

		pWorldLightItem->m_Light.m_pCoronaTex = &m_aCoronaTexInst[ m_nUpgradeLevel ];
		pWorldLightItem->m_Light.m_fCoronaScale = pUserProps->fCoronaScale;
		pWorldLightItem->m_Light.m_fDeltaScale = pUserProps->fCoronaUnitScreenspaceScale;

		pWorldLightItem->m_Light.SetColor( pUserProps->fCoronaColorRed, pUserProps->fCoronaColorGreen, pUserProps->fCoronaColorBlue );
		pWorldLightItem->m_Light.SetMotif( fcolor_GetRandomMotif(FCOLORMOTIF_ROCKET0) );
		pProj->SetAttachedLight( pWorldLightItem );
	}

	// Create dynamic light...
	if( !bNoDynamicLight ) {
		CFWorldLightItem *pWorldLightItem = CLightPool::GetFromFreePool();
		if( pWorldLightItem ) {
			pWorldLightItem->m_Light.InitOmniLight( &ProjMtx.m_vPos.v3, pUserProps->fLightRadius );

			FMATH_SETBITMASK( pWorldLightItem->m_Light.m_nFlags, FLIGHT_FLAG_PER_PIXEL );

			pWorldLightItem->m_Light.SetColor( pUserProps->fLightColorRed, pUserProps->fLightColorGreen, pUserProps->fLightColorBlue );
			pWorldLightItem->m_Light.SetMotif( fcolor_GetRandomMotif(FCOLORMOTIF_ROCKET0) );
			pProj->SetAttachedLight2( pWorldLightItem );
		}
	}
}


#if 0
void CWeaponRocket::_SpawnTubeEffects( void ) {
	_KillTubeEffects();

	// Spawn a smoke trail for the muzzle effect...
	m_hTubeSmokeTrail = smoketrail_GetFromFreePoolAndSetAttributes( &m_RearTubeSmokeTrailAttrib );

	if( m_hTubeSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
		CFVec3A TubeSmokeEndPos_WS;
		const CFMtx43A *pTubeBoneMtx = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_pResourceData->m_nTubeBoneIndex ];

		TubeSmokeEndPos_WS.Mul( pTubeBoneMtx->m_vZ, -4.0f ).Add( pTubeBoneMtx->m_vPos );

		smoketrail_Puff( m_hTubeSmokeTrail, &TubeSmokeEndPos_WS );
		smoketrail_Puff( m_hTubeSmokeTrail, &pTubeBoneMtx->m_vPos );

		m_fRL23_TubeSmokeSecs = 0.125f;
	}
}


void CWeaponRocket::_KillTubeEffects( void ) {
	if( m_hTubeSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
		smoketrail_ReturnToFreePool( m_hTubeSmokeTrail, TRUE );
		m_hTubeSmokeTrail = SMOKETRAIL_NULLHANDLE;
	}

	m_fRL23_TubeSmokeSecs = 0.0f;
}


void CWeaponRocket::_TubeEffectsWork( void ) {
	if( m_hTubeSmokeTrail != SMOKETRAIL_NULLHANDLE ) {
		const CFMtx43A *pTubeBoneMtx = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_pResourceData->m_nTubeBoneIndex ];

		smoketrail_Puff( m_hTubeSmokeTrail, &pTubeBoneMtx->m_vPos );

		if( m_fRL23_TubeSmokeSecs > 0.0f ) {
			m_fRL23_TubeSmokeSecs -= FLoop_fPreviousLoopSecs;

			if( m_fRL23_TubeSmokeSecs <= 0.0f ) {
				_KillTubeEffects();
			}
		}
	}
}
#endif


// This function is called to let the weapon know what the trigger values are.
// It should be called whether or not the triggers are pressed because this function
// is responsible for firing and reloading of the weapon.
//
// Return value:
//   Bit 0: Trigger #1 fired
//   Bit 1: Trigger #2 fired
u32 CWeaponRocket::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );

	if( fUnitTriggerVal1 < 0.25f ) {
		// Trigger not down...
		return 0;
	}

	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		return 0;
	}

	if( m_nClipAmmo == 0 ) {
		// No rounds in clip...
		return 0;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return 0;
	}

	// Get a free projectile...
	CEProj *pProj = CEProjPool::GetProjectileFromFreePool( m_ahProjPool[PROJ_POOL_TYPE_SWARMER_ROCKET] );
	if( pProj == NULL ) {
		// No more projectiles...
		return 0;
	}

	// Let's fire a round...

	CFVec3A ProjPos_WS, ProjUnitDir_WS, FireUnitDir_WS, MuzzleUnitDir_WS;
	f32 fJitterFactor;
	const _UserProps_t *pUserProps;

	pUserProps = &m_aUserProps[m_nUpgradeLevel];

	// Compute muzzle point...
	ComputeMuzzlePoint_WS( &ProjPos_WS );
	ComputeMuzzleUnitDir_WS( &MuzzleUnitDir_WS );
	ComputeFireUnitDir( &FireUnitDir_WS, &ProjPos_WS, &MuzzleUnitDir_WS, pvTargetPos_WS, FALSE );

	// Add in a little inaccuracy...
	fJitterFactor = pUserProps->fShotSpreadFactor;
	if( m_pAttachedScope && m_pAttachedScope->IsZoomEnabled() ) {
		fJitterFactor *= pUserProps->afScopeSpreadMult[ m_pAttachedScope->GetUpgradeLevel() ];
	}

	fmath_ScatterUnitVec( &ProjUnitDir_WS, &FireUnitDir_WS, fJitterFactor );

	// Initialize the rocket projectile...
	_InitProjectile( pProj, &ProjPos_WS, &ProjUnitDir_WS, FALSE );

	if( m_nUpgradeLevel == 2 ) {
		CEProjExt::CEProj_Swarmer_Params_t SwarmerParams;

		SwarmerParams.pMervStaticParams = &m_aUserProps[2].MervStaticParams;
		SwarmerParams.fSwarmAmplitude = 0.0f;
		SwarmerParams.fSwarmingBlendInInvDist = 0.0f;
		SwarmerParams.fSwarmingSpiralSpeed = 0.0f;

		pProj->SetSwarmerParams( &SwarmerParams );
	}

	pProj->Launch();

//	_SpawnTubeEffects();

	if( pBuddyFirePos_WS != NULL ) {
		// Get a free projectile...
		CEProj *pProj = CEProjPool::GetProjectileFromFreePool( m_ahProjPool[PROJ_POOL_TYPE_SWARMER_ROCKET] );
		if( pProj ) {
			_InitProjectile( pProj, pBuddyFirePos_WS, &ProjUnitDir_WS, TRUE );
			pProj->Launch();
		}
	}

	if( pUserProps->hExplosionGroupFire ) {
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			FExplosionSpawnParams_t SpawnParams;

			SpawnParams.InitToDefaults();

			SpawnParams.uFlags |= FEXPLOSION_SPAWN_EXPLOSION_FOLLOW_UNIT_DIR;

			SpawnParams.Pos_WS = ProjPos_WS;
			SpawnParams.UnitDir = ProjUnitDir_WS;
			SpawnParams.uSurfaceType = 0;

			CExplosion2::SpawnExplosion( hSpawner, pUserProps->hExplosionGroupFire, &SpawnParams );
		}
	}

	// Set timer until the next round is ready...
	m_fSecondsCountdownTimer = pUserProps->fOORoundsPerSec;

	if( IsOwnedByPlayer()) {
		// Human player...

		s32 nPlayerIndex = GetOwner()->m_nPossessionPlayerIndex;

		// Generate force feedback...
		fforce_Kill( &m_hForce );

		if( m_pInfo->nStanceType != STANCE_TYPE_SHOULDER_MOUNT ) {
			fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_LIGHT, &m_hForce );
		} else {
			fforce_Play( Player_aPlayer[nPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROCKET_THRUST_HEAVY, &m_hForce );
		}

		// Generate camera shake...
		CFCamera *pCamera = fcamera_GetCameraByIndex( nPlayerIndex );
		pCamera->ShakeCamera( 0.05f, 0.15f );

		// Play 2D launch sound...
		CFSoundGroup::PlaySound( pUserProps->pSoundGroupFire, TRUE, &ProjPos_WS, nPlayerIndex, TRUE );
	} else {
		// Play 3D launch sound...
		CFSoundGroup::PlaySound( pUserProps->pSoundGroupFire, FALSE, &ProjPos_WS );
	}

	// Set up cartridge rotation for RL2 and RL3...
	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		m_nRL23_CartMode = _RL23_CARTMODE_FIRE;
		m_fRL23_CartridgeRotAngleStart = m_aUserProps[1].fRL23_AngleBetweenCartridgeSlots * (f32)(GetMaxClipAmmo() - GetClipAmmo());
		m_fRL23_CartridgeRotAngleEnd = m_fRL23_CartridgeRotAngleStart + m_aUserProps[1].fRL23_AngleBetweenCartridgeSlots;
		m_fRL23_CartridgeRotationAngle = m_fRL23_CartridgeRotAngleStart;
	}

	// Remove the round we fired from the clip...
	RemoveFromClip( 1 );

	// Indicate to caller that trigger #1 fired the weapon...
	return 1;
}


void CWeaponRocket::_RL1_Work( void ) {
	if( m_fSecondsCountdownTimer > 0.0f ) {
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fSecondsCountdownTimer, 0.0f );
	}
}


void CWeaponRocket::_RL23_Work( void ) {
	f32 fUnit, fVal;

	#define __RELOAD_RECOVERY_UNIT_TIME		0.2f
	#define __SWINGDOWN_TIME				5.0f
	#define __SWINGUP_TIME					0.5f
	#define __SWINGDOWN_BOUNCE_UNIT_TIME	0.15f

	switch( m_nRL23_CartMode ) {
	case _RL23_CARTMODE_IDLE:
		if( !HasAmmo() ) {
			// No ammo...

			if( CurrentState() == STATE_RELOAD_OPENED ) {
				m_nRL23_CartMode = _RL23_CARTMODE_SWINGDOWN;
				m_fSecondsCountdownTimer = __SWINGDOWN_TIME;
			}

		} else if( m_bRL23_ReloadRequest  ) {
			// We have ammo and there's a pending reload request...

			m_bRL23_ReloadRequest = FALSE;

			// Make sure it can be reloaded, just in case...
			if( CanClipBeReloaded() ) {
				m_nRL23_CartMode = _RL23_CARTMODE_RELOAD;

				m_fSecondsCountdownTimer = m_aUserProps[ m_nUpgradeLevel ].fRL23_ReloadTimeForOneRound;

				m_fRL23_CartridgeRotAngleStart = m_aUserProps[1].fRL23_AngleBetweenCartridgeSlots * (f32)(GetMaxClipAmmo() - GetClipAmmo());
				m_fRL23_CartridgeRotAngleEnd = m_fRL23_CartridgeRotAngleStart - m_aUserProps[1].fRL23_AngleBetweenCartridgeSlots;
				m_fRL23_CartridgeRotationAngle = m_fRL23_CartridgeRotAngleStart;
				TransferFromReserveToClip();

				PlaySound( m_aUserProps[m_nUpgradeLevel].pSoundGroupReload );
			}
		}

		break;

	case _RL23_CARTMODE_FIRE:
		// We're in the process of firing off a shot.
		// Advance cartridge rotation...

		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		if( m_fSecondsCountdownTimer <= 0.0f ) {
			// We're done firing...

			m_fSecondsCountdownTimer = 0.0f;
			m_nRL23_CartMode = _RL23_CARTMODE_IDLE;
		}

		fUnit = 1.0f - m_fSecondsCountdownTimer * m_aUserProps[ m_nUpgradeLevel ].fRoundsPerSec;
		m_fRL23_CartridgeRotationAngle = FMATH_FPOT( fmath_UnitLinearToSCurve(fUnit), m_fRL23_CartridgeRotAngleStart, m_fRL23_CartridgeRotAngleEnd );

		break;

	case _RL23_CARTMODE_RELOAD:
		// We're in the process of reloading.
		// Advance cartridge rotation...

		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		if( m_fSecondsCountdownTimer <= 0.0f ) {
			// We'one reloading...

			m_fSecondsCountdownTimer = 0.0f;
			m_nRL23_CartMode = _RL23_CARTMODE_IDLE;
			ReloadComplete();
		}

		fUnit = 1.0f - m_fSecondsCountdownTimer * m_aUserProps[ m_nUpgradeLevel ].fRL23_OOReloadTimeForOneRound;

		if( fUnit < (1.0f - __RELOAD_RECOVERY_UNIT_TIME) ) {
			fUnit *= (1.0f / (1.0f - __RELOAD_RECOVERY_UNIT_TIME));

			// Make the cool reload rotation motion...
			fVal = 2.5f * fmath_Sin( FMATH_HALF_PI * fUnit );
			fVal = FMATH_FPOT( fUnit, fVal, fUnit );
			fVal *= fVal;

			m_fRL23_CartridgeRotationAngle = m_fRL23_CartridgeRotAngleStart + fVal * (m_fRL23_CartridgeRotAngleEnd - m_fRL23_CartridgeRotAngleStart);
		}

		break;

	case _RL23_CARTMODE_SWINGDOWN:
		fUnit = 1.0f;

		if( m_fSecondsCountdownTimer > 0.0f ) {
			m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fSecondsCountdownTimer, 0.0f );

			fUnit = 1.0f - m_fSecondsCountdownTimer * (1.0f / __SWINGDOWN_TIME);

			m_fRL23_CartridgeRotationAngle = -FMATH_2PI * fmath_Sin( fUnit * FMATH_HALF_PI );

			if( fUnit < __SWINGDOWN_BOUNCE_UNIT_TIME ) {
				fUnit *= (1.0f / __SWINGDOWN_BOUNCE_UNIT_TIME);

				fVal = fmath_Cos( 4.712f * fUnit * fUnit * fUnit * fUnit );
				fVal = fmath_Abs( fVal ) * (1.0f - fUnit);

				m_fRL23_CartridgeSwingAngle = (1.0f - fVal) * -FMATH_HALF_PI;
			}
		}

		if( fUnit >= 0.15f ) {
			if( HasAmmo() || (CurrentState() != STATE_RELOAD_OPENED) ) {
				m_nRL23_CartMode = _RL23_CARTMODE_SWINGUP;
				m_fRL23_CartridgeRotAngleStart = m_fRL23_CartridgeRotationAngle;
				m_fRL23_CartridgeRotAngleEnd = (fUnit == 1.0f) ? m_fRL23_CartridgeRotAngleStart : 0.0f;
				m_fSecondsCountdownTimer = __SWINGUP_TIME;
			}
		}

		break;

	case _RL23_CARTMODE_SWINGUP:
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		if( m_fSecondsCountdownTimer <= 0.0f ) {
			m_fSecondsCountdownTimer = 0.0f;
			m_nRL23_CartMode = _RL23_CARTMODE_IDLE;
		}

		fUnit = 1.0f - m_fSecondsCountdownTimer * (1.0f / __SWINGUP_TIME);

		m_fRL23_CartridgeRotationAngle = FMATH_FPOT( fUnit, m_fRL23_CartridgeRotAngleStart, m_fRL23_CartridgeRotAngleEnd );
		m_fRL23_CartridgeSwingAngle = -FMATH_HALF_PI * fmath_UnitLinearToSCurve( 1.0f - fUnit );

		break;

	default:
		FASSERT_NOW;
		break;
	}

	#undef __RELOAD_RECOVERY_UNIT_TIME
	#undef __SWINGDOWN_TIME
	#undef __SWINGUP_TIME
	#undef __SWINGDOWN_BOUNCE_UNIT_TIME
}


// Called once per frame to update animations, timers, etc.
void CWeaponRocket::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	// Do EUK-level-specific work...
	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		_RL23_Work();
	} else {
		_RL1_Work();
	}

	if( m_pAnimInstCurrentlyPlaying ) {
		// We're animating between states...

		if( m_pAnimInstCurrentlyPlaying->DeltaTime( FLoop_fPreviousLoopSecs * m_fAnimInstTimeMul, TRUE ) ) {
			// Animation done playing...
			m_pAnimInstCurrentlyPlaying = NULL;
			StateChangeComplete();
		}
	}

	_ComputeMtxPalette();

//	_TubeEffectsWork();
}


void CWeaponRocket::BeginReload( void ) {
	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		m_bRL23_ReloadRequest = TRUE;
	} else {
		TransferFromReserveToClip();
		ReloadComplete();

		PlaySound( m_aUserProps[m_nUpgradeLevel].pSoundGroupReload );
	}
}


// Starts changing to state CWeapon::CurrentState().
void CWeaponRocket::BeginStateChange( void ) {
	_Anim_e nAnimIndex;

	if( (CurrentState() == STATE_PRESENT) || (CurrentState() == STATE_RELOADING) ) {
		StateChangeComplete();
		return;
	}

	nAnimIndex = _TransitionStateToAnimIndex( CurrentState() );

	if( nAnimIndex == _ANIM_COUNT ) {
		// No animation to play for this state. Jump to state...

		m_pAnimInstCurrentlyPlaying = NULL;
		StateChangeComplete();
	} else {
		// Play animation...

		m_pAnimInstCurrentlyPlaying = m_pResourceData->m_apAnimInstArray[nAnimIndex];
		m_pResourceData->m_pAnimCombiner->AttachToTap( m_pResourceData->m_nCombinerTapID, m_pAnimInstCurrentlyPlaying );

		m_fAnimInstTimeMul = m_aUserProps[ m_nUpgradeLevel ].afTimeMul[nAnimIndex];

		if( m_fAnimInstTimeMul >= 0.0f ) {
			m_pAnimInstCurrentlyPlaying->UpdateTime( 0.0f );
		} else {
			m_pAnimInstCurrentlyPlaying->UpdateUnitTime( 1.0f );
		}
	}
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponRocket::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_fSecondsCountdownTimer = 0.0f;

	m_nRL23_CartMode = _RL23_CARTMODE_IDLE;
	m_bRL23_ReloadRequest = FALSE;
	m_fRL23_CartridgeSwingAngle = 0.0f;
	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		m_fRL23_CartridgeRotationAngle = m_aUserProps[1].fRL23_AngleBetweenCartridgeSlots * (f32)(GetMaxClipAmmo() - GetClipAmmo());
	}

	_SetAnimToCurrentFinalState();
}


void CWeaponRocket::_SetAnimToCurrentFinalState( void ) {
	_Anim_e nAnimIndex;
	f32 fAnimTimeUnitStartValue;

	nAnimIndex = _FinalStateToAnimInfo( CurrentState(), &fAnimTimeUnitStartValue );

	if( nAnimIndex != _ANIM_COUNT ) {
		m_pResourceData->m_apAnimInstArray[nAnimIndex]->UpdateUnitTime( fAnimTimeUnitStartValue );
		m_pResourceData->m_pAnimCombiner->AttachToTap( m_pResourceData->m_nCombinerTapID, m_pResourceData->m_apAnimInstArray[nAnimIndex] );
	}

	_ComputeMtxPalette();

	m_pAnimInstCurrentlyPlaying = NULL;
}


void CWeaponRocket::_CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	if( (m_pRL23_CallbackThis->m_nRL23_CartMode != _RL23_CARTMODE_SWINGDOWN) && (m_pRL23_CallbackThis->m_nRL23_CartMode != _RL23_CARTMODE_SWINGUP) ) {
		m_RL23_MtxRotZ.SetRotationZ( m_pRL23_CallbackThis->m_fRL23_CartridgeRotationAngle + m_aUserProps[1].fRL23_NoAmmoBaseAngle );
		m_RL23_MtxRotZ.m_vPos.Zero();
		rNewMtx.Mul( m_RL23_MtxRotZ );
	} else {
		m_RL23_MtxRotZ.SetRotationZ( m_pRL23_CallbackThis->m_fRL23_CartridgeSwingAngle );
		m_RL23_MtxRotZ.m_vPos.Set( 0.0f, -0.2f, 0.0f );

		rNewMtx.Mul( m_RL23_MtxRotZ );

		m_RL23_MtxRotZ.SetRotationZ( m_pRL23_CallbackThis->m_fRL23_CartridgeRotationAngle + m_aUserProps[1].fRL23_NoAmmoBaseAngle );
		m_RL23_MtxRotZ.m_vPos.Set( 0.0f, 0.2f, 0.0f );
		rNewMtx.Mul( m_RL23_MtxRotZ );
	}
}


void CWeaponRocket::_ComputeMtxPalette( void ) {
	u32 i;

#define __ROCKET_DELAY_UNIT_TIME	0.1f
#define __ROCKET_SPIN_UNIT_TIME		0.8f
#define __ROCKET_SPIN_ANGLE_START	(FMATH_PI * 0.25f)
#define __ROCKET_SPIN_DELTA			(FMATH_PI * 0.25f)
#define __ROCKET_DISP_RECOIL_ANGLE	2.0f
#define __ROCKET_DISP_DIST			0.5f

	m_pRL23_CallbackThis = this;
	m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );

	// Move rockets attached to cartridge for RL2 and RL3...
	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		CFMtx43A *pCartridgeBoneMtx = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_nRL23_CartridgeBoneIndex ];
		CFMtx43A RocketMtx;

		FASSERT( m_pResourceData->m_pCartridgeRocketWorldMeshArray );

		for( i=0; i<GetClipAmmo(); ++i ) {
			RocketMtx.Mul( *pCartridgeBoneMtx, m_pCartridgeSlotMtxArray[i] );
			m_pResourceData->m_pCartridgeRocketWorldMeshArray[i].m_Xfm.BuildFromMtx( RocketMtx );
			m_pResourceData->m_pCartridgeRocketWorldMeshArray[i].UpdateTracker();
		}
	}

	RelocateAllChildren();

	m_pRL23_CallbackThis = NULL;
}


void CWeaponRocket::NotifyAmmoMightHaveChanged( void ) {
	s32 i;

	if( m_pInfo->nStanceType == STANCE_TYPE_SHOULDER_MOUNT ) {
		FASSERT( m_pResourceData->m_pCartridgeRocketWorldMeshArray );
		for( i=GetClipAmmo(); i<m_pInfo->nClipAmmoMax; ++i ) {
			m_pResourceData->m_pCartridgeRocketWorldMeshArray[i].RemoveFromWorld();
		}
	}
}


// Returns _ANIM_COUNT if an animation does not exist for the specified transition state.
CWeaponRocket::_Anim_e CWeaponRocket::_TransitionStateToAnimIndex( State_e nState ) const {
	_Anim_e nReturnState = _ANIM_COUNT;

	switch( nState ) {
	case STATE_STOWING:
		nReturnState = _ANIM_STOW;
		break;

	case STATE_DEPLOYING:
		nReturnState = _ANIM_DEPLOY;
		break;

	case STATE_CLOSING_RELOAD:
		nReturnState = _ANIM_RELOAD_CLOSE;
		break;

	case STATE_OPENING_RELOAD:
		nReturnState = _ANIM_RELOAD_OPEN;
		break;

	}

	if( nReturnState != _ANIM_COUNT ) {
		if( m_pResourceData->m_apAnimInstArray[nReturnState] == NULL ) {
			nReturnState = _ANIM_COUNT;
		}
	}

	return nReturnState;
}


// Returns _ANIM_COUNT if an animation does not exist for the specified final state.
CWeaponRocket::_Anim_e CWeaponRocket::_FinalStateToAnimInfo( State_e nState, f32 *pfAnimTimeUnitStartValue ) const {
	_Anim_e nReturnState = _ANIM_COUNT;

	switch( nState ) {
	case STATE_PRESENT:
		if( m_pResourceData->m_apAnimInstArray[_ANIM_STOW] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_STOW;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_DEPLOY] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_DEPLOY;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_CLOSE] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_RELOAD_CLOSE;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_OPEN] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_RELOAD_OPEN;
			break;
		}

		break;

	case STATE_STOWED:
		if( m_pResourceData->m_apAnimInstArray[_ANIM_STOW] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_STOW;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_DEPLOY] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_DEPLOY;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_CLOSE] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_RELOAD_CLOSE;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_OPEN] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_RELOAD_OPEN;
			break;
		}

		break;

	case STATE_DEPLOYED:
		if( m_pResourceData->m_apAnimInstArray[_ANIM_DEPLOY] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_DEPLOY;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_STOW] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_STOW;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_CLOSE] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_RELOAD_CLOSE;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_OPEN] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_RELOAD_OPEN;
			break;
		}

		break;

	case STATE_RELOAD_OPENED:
		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_OPEN] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_RELOAD_OPEN;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_RELOAD_CLOSE] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_RELOAD_CLOSE;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_DEPLOY] ) {
			*pfAnimTimeUnitStartValue = 1.0f;
			nReturnState = _ANIM_DEPLOY;
			break;
		}

		if( m_pResourceData->m_apAnimInstArray[_ANIM_STOW] ) {
			*pfAnimTimeUnitStartValue = 0.0f;
			nReturnState = _ANIM_STOW;
			break;
		}

		break;

	}

	if( nReturnState != _ANIM_COUNT ) {
		if( m_aUserProps[ m_nUpgradeLevel ].afTimeMul[nReturnState] < 0.0f ) {
			*pfAnimTimeUnitStartValue = 1.0f - *pfAnimTimeUnitStartValue;
		}
	}

	return nReturnState;
}

