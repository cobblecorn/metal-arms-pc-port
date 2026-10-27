//////////////////////////////////////////////////////////////////////////////////////
// weapon_laser.cpp - Mining Laser weapon.
//
// Author: Justin Link
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
// 06/19/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_laser.h"
#include "fgamedata.h"
#include "fworld.h"
#include "fmesh.h"
#include "fanim.h"
#include "fresload.h"
#include "meshtypes.h"
#include "floop.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "explosion.h"
#include "potmark.h"
#include "fcamera.h"
#include "ItemInst.h"
#include "LaserBeam.h"
#include "fsndfx.h"
#include "fsound.h"
#include "AI/AIEnviro.h"
#include "EShield.h"
#include "damage.h"
#include "gcoll.h"
#include "fdecal.h"
#if FANG_WINGC
#include "eparticlepool.h"
#include "pc_mods.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif
#endif

#include "sas_user.h"


#define _USER_PROP_FILENAME		"w_laser.csv"


static const u32 _uFizzleThreshhold = 25;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponLaserBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponLaserBuilder _WeaponLaserBuilder;


void CWeaponLaserBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONLASER, pszEntityType );
}


BOOL CWeaponLaserBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponLaserBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponLaser
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponLaser::_UserProps_t CWeaponLaser::m_aUserProps[EUK_COUNT_LASER];
CDamageProfile *CWeaponLaser::m_apDamageProfile[EUK_COUNT_LASER];
TracerDef_t CWeaponLaser::m_aoTracerDef[EUK_COUNT_LASER];
CFTexInst CWeaponLaser::m_aTracerTexInst[EUK_COUNT_LASER];

#if FANG_WINGC
static BOOL _bUserPropsAvailable = FALSE;
CWeaponLaser::_RetailCharge_t CWeaponLaser::m_aRetailCharge[EUK_COUNT_LASER];
static const u16 _CHARGED_TRACER = 0x8000;

// Retail values without a slot in _UserProps_t (see _ReadRetailProperties).
static f32 _aRetailTracerColor[CWeapon::EUK_COUNT_LASER][4];
static cchar *_apszRetailMuzzleBone[CWeapon::EUK_COUNT_LASER];
// Retail field 65: the firing sound group ("LaserFire"), resolved against the level's banks when a
// laser is built.
static cchar *_apszRetailFireGroup[CWeapon::EUK_COUNT_LASER];
static CFSoundGroup *_apRetailFireGroup[CWeapon::EUK_COUNT_LASER];
#endif

// The source's third laser level is a continuous beam. The retail GameCube L3 fires
// tracers through the same path as the other levels, so the WINGC build does too.
static FINLINE BOOL _UsesL3Beam( u32 nLevel ) {
#if FANG_WINGC
	return FALSE;
#else
	return nLevel == 2;
#endif
}


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponLaser::m_aUserPropVocab[] = {
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

	// fWeaponCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

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

	// fUnitRecoil:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fRocketCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fRechargeRate:
	// fOORechargeRate:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fRocketSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fTracerLength
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_500,

	// fTracerWidth
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

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

	// pszL12_TexName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszMuzzleTex
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMuzzleWidth
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleHeight
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// pszDamageProfile:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hFiringSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_DECAL_DEF,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponLaser::m_aUserPropMapTable[] = {
	"LaserL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"LaserL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"LaserL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"LaserMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],

	"LaserMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[4],

	NULL
};


#if FANG_WINGC
// The retail w_laser.csv tables have 73 fields (charge/burst redesign) where this source
// expects 23. Their meaning comes from the retail main.dol: its FGameDataMap_t "LaserL1"
// vocabulary gives each field's type and range, and retail CWeaponLaser code shows where
// the values go (tools/dol_vocab.py, tools/dol_xref.py). Confirmed uses:
//   0 mesh, 1 muzzle bone, 2 tracer texture, 3-6 tracer RGBA, 7 clip (reserve is forced
//   infinite), 8 rounds/sec, 10/11/12/13 tracer speed/length/width/max range,
//   14 max and 46 min target-assist distance, 43 weapon cull distance, 44 recoil.
// 59 and 71 are the damage profile and decal by vocabulary type; 65 is the firing sound group. The
// PC hold/release uses retail charge timing, projectile data, damage and sounds below.
BOOL CWeaponLaser::_ReadRetailProperties( void ) {
	// Retail source field for each entry of m_aUserPropVocab, or -1 if it has none.
	static const s8 anRetailField[] = {
		0,	// apszMeshName[_MESH_WEAPON]
		7,	// fClipAmmoMax
		-1,	// fReserveAmmoMax: retail InitSystem stores INFINITE_AMMO
		8,	// fRoundsPerSec, fOORoundsPerSec
		43,	// fWeaponCullDist: retail ClassHierarchyBuild MeshInit.fCullDist
		-1,	// fRocketsInPoolCount: unused by this runtime
		13,	// fMaxLiveRange: Info_t and TracerDef_t::fMaxTailDist_WS
		44,	// fUnitRecoil: Info_t
		-1,	// fRocketCullDist: unused by this runtime
		-1,	// fRechargeRate, fOORechargeRate: derived below
		10,	// fRocketSpeed: TracerDef_t::fSpeed_WS
		11,	// fTracerLength: TracerDef_t::fLength_WS
		12,	// fTracerWidth: TracerDef_t::fWidth_WS
		-1,	// fDistFromWeaponOrigToMuzzle: retail uses the field 1 bone
		46,	// fMinTargetAssistDist: Info_t
		14,	// fMaxTargetAssistDist: Info_t
		2,	// pszL12_TexName: retail tracer texture
		-1,	// pszMuzzleTex: unused by this runtime
		-1,	// fMuzzleWidth: retail muzzle flashes are particles (fields 47-50)
		-1,	// fMuzzleHeight
		59,	// pszDamageProfile
		-1,	// hFiringSound: retail uses sound groups (fields 65-70)
		71,	// hDecalDef
	};
	static const FGameData_TableEntry_t RetailString = {
		FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL, sizeof(char *), F32_DATATABLE_0, F32_DATATABLE_0 };
	static const FGameData_TableEntry_t RetailFloat = {
		FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_FLOAT_X, sizeof(f32), F32_DATATABLE_0, F32_DATATABLE_0 };

	FMemFrame_t Frame = fmem_GetFrame();
	FGameDataFileHandle_t hFile = fgamedata_LoadFileToFMem( _USER_PROP_FILENAME );
	BOOL bOK = hFile != FGAMEDATA_INVALID_FILE_HANDLE;

	for( u32 nLevel=0; bOK && nLevel<EUK_COUNT_LASER; ++nLevel ) {
		FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( hFile, m_aUserPropMapTable[nLevel].pszTableName );
		if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE || fgamedata_GetNumFields( hTable ) != 73 ) {
			bOK = FALSE;
			break;
		}

		_UserProps_t *pProps = &m_aUserProps[nLevel];
		if( !fgamedata_GetTableDataRemapped( hTable, m_aUserPropVocab, anRetailField, sizeof(anRetailField) / sizeof(anRetailField[0]),
											 pProps, sizeof(_UserProps_t) ) ) {
			bOK = FALSE;
			break;
		}

		// Retail extras: muzzle bone, tracer color, and the energy refill step (every
		// field 39 seconds, add field 40 of a full unit) used to derive the recharge rate.
		f32 fRefillSecs = 0.0f, fRefillUnit = 0.0f;
		bOK = fgamedata_GetFieldFromTable( hTable, 1, &RetailString, &_apszRetailMuzzleBone[nLevel] ) &&
			fgamedata_GetFieldFromTable( hTable, 65, &RetailString, &_apszRetailFireGroup[nLevel] ) &&
			fgamedata_GetFieldFromTable( hTable, 39, &RetailFloat, &fRefillSecs ) &&
			fgamedata_GetFieldFromTable( hTable, 40, &RetailFloat, &fRefillUnit );
		for( u32 i=0; bOK && i<4; ++i ) {
			bOK = fgamedata_GetFieldFromTable( hTable, 3 + i, &RetailFloat, &_aRetailTracerColor[nLevel][i] );
		}
		if( !bOK ) {
			break;
		}

		_RetailCharge_t *pCharge = &m_aRetailCharge[nLevel];
		static const FGameData_TableEntry_t ParticleField = { FGAMEDATA_VOCAB_PARTICLE };
		bOK = fgamedata_GetFieldFromTable( hTable, 33, &RetailFloat, &pCharge->fDelay ) &&
			fgamedata_GetFieldFromTable( hTable, 34, &RetailFloat, &pCharge->fChargeSeconds ) &&
			fgamedata_GetFieldFromTable( hTable, 35, &RetailFloat, &pCharge->fPitchStart ) &&
			fgamedata_GetFieldFromTable( hTable, 36, &RetailFloat, &pCharge->fPitchEnd ) &&
			fgamedata_GetFieldFromTable( hTable, 48, &ParticleField, &pCharge->hChargeParticle ) &&
			fgamedata_GetFieldFromTable( hTable, 49, &ParticleField, &pCharge->hBurstParticle ) &&
			fgamedata_GetFieldFromTable( hTable, 50, &ParticleField, &pCharge->hEndParticle ) &&
			fgamedata_GetFieldFromTable( hTable, 37, &RetailFloat, &pCharge->fReadyEnergy ) &&
			fgamedata_GetFieldFromTable( hTable, 38, &RetailFloat, &pCharge->fEnergyAfterBurst ) &&
			fgamedata_GetFieldFromTable( hTable, 20, &RetailFloat, &pCharge->fSpeed ) &&
			fgamedata_GetFieldFromTable( hTable, 21, &RetailFloat, &pCharge->fLength ) &&
			fgamedata_GetFieldFromTable( hTable, 22, &RetailFloat, &pCharge->fWidth ) &&
			fgamedata_GetFieldFromTable( hTable, 23, &RetailFloat, &pCharge->fRange ) &&
			fgamedata_GetFieldFromTable( hTable, 66, &RetailString, &pCharge->pszChargeSound ) &&
			fgamedata_GetFieldFromTable( hTable, 67, &RetailString, &pCharge->pszBurstSound ) &&
			fgamedata_GetFieldFromTable( hTable, 68, &RetailString, &pCharge->pszBurstSound2 ) &&
			fgamedata_GetFieldFromTable( hTable, 69, &RetailString, &pCharge->pszRicochetSound ) &&
			fgamedata_GetFieldFromTable( hTable, 70, &RetailString, &pCharge->pszReadySound ) &&
			fgamedata_GetFieldFromTable( hTable, 63, &RetailString, &pCharge->pszImpact ) &&
			fgamedata_GetFieldFromTable( hTable, 64, &RetailString, &pCharge->pszRicochetImpact ) &&
			fgamedata_GetFieldFromTable( hTable, 72, &RetailString, &pCharge->pszDecal );
		for( u32 i=0; bOK && i<3; ++i ) {
			bOK = fgamedata_GetFieldFromTable( hTable, 60 + i, &RetailString, &pCharge->apszDamage[i] ) &&
				fgamedata_GetFieldFromTable( hTable, 29 + i, &RetailFloat, &pCharge->afColor[i] );
		}
		if( !bOK || !(pCharge->fDelay >= 0.0f) || !(pCharge->fChargeSeconds > 0.0f) ||
			!(pCharge->fReadyEnergy > 0.0f && pCharge->fReadyEnergy < 1.0f) ||
			!(pCharge->fEnergyAfterBurst >= 0.0f && pCharge->fEnergyAfterBurst < pCharge->fReadyEnergy) ||
			!(pCharge->fSpeed > 0.0f && pCharge->fLength > 0.0f && pCharge->fWidth > 0.0f && pCharge->fRange > 0.0f) ) {
			bOK = FALSE;
			break;
		}

		pProps->fReserveAmmoMax = -1.0f;
		pProps->fRocketsInPoolCount = 30.0f;		// retail tracer group size
		pProps->fRocketCullDist = pProps->fWeaponCullDist;
		pProps->fDistFromWeaponOrigToMuzzle = 0.0f;
		pProps->pszMuzzleTex = NULL;
		pProps->fMuzzleWidth = 0.0f;
		pProps->fMuzzleHeight = 0.0f;
		pProps->hFiringSound = FSNDFX_INVALID_FX_HANDLE;
		if( fRefillSecs > 0.0f && fRefillUnit > 0.0f ) {
			pProps->fRechargeRate = pProps->fClipAmmoMax * fRefillUnit / fRefillSecs;
		}

		// These feed divisions and countdown loops; reject values that would stall them.
		if( !(pProps->fClipAmmoMax >= 1.0f) || !(pProps->fRoundsPerSec > 0.0f) || !(pProps->fRechargeRate > 0.0f) ||
			!(pProps->fRocketSpeed > 0.0f) || !(pProps->fMaxLiveRange > 0.0f) || !pProps->apszMeshName[_MESH_WEAPON] ) {
			bOK = FALSE;
			break;
		}
		pProps->fOORechargeRate = 1.0f / pProps->fRechargeRate;
	}

	fmem_ReleaseFrame( Frame );
	if( bOK ) {
		DEVPRINTF( "CWeaponLaser: loaded %d retail variants with player hold/release charge data.\n", EUK_COUNT_LASER );
	}
	return bOK;
}
#endif


BOOL CWeaponLaser::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;
	FTexDef_t *pTexDef;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );
#if FANG_WINGC
	_bUserPropsAvailable = FALSE;
#endif

	// Read the user properties for all EUK levels of this weapon...
#if FANG_WINGC
	if( !_ReadRetailProperties() ) {
#else
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
#endif
		DEVPRINTF( "CWeaponLaser::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
#if FANG_WINGC
		DEVPRINTF( "CWeaponLaser::InitSystem(): Disabling laser entities because the retail GameCube table could not be mapped.\n" );
		fres_ReleaseFrame( ResFrame );
		return TRUE;
#else
		goto _ExitWithError;
		return FALSE;
#endif
	}

#if FANG_WINGC
	_bUserPropsAvailable = TRUE;
#endif

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_LASER; i++ ) {
		if( !_UsesL3Beam( i ) )
		{
			m_aoTracerDef[i].pUser = NULL;				// This will get set at the point of spawning the tracers.
			m_aoTracerDef[i].pFcnKillCallback = _TracerKilledCallback;
			m_aoTracerDef[i].fWidth_WS = m_aUserProps[i].fTracerWidth;
			m_aoTracerDef[i].fLength_WS = m_aUserProps[i].fTracerLength;
			m_aoTracerDef[i].fSpeed_WS = m_aUserProps[i].fRocketSpeed;
			m_aoTracerDef[i].fMaxTailDist_WS = m_aUserProps[i].fMaxLiveRange;
			m_aoTracerDef[i].fBeginDeathUnitFade_WS = 0.25f;
#if FANG_WINGC
			// The retail tracer texture (TF_1shot_01) is a glow on black with no alpha, and the
			// retail color is tinted with full alpha: it only looks right drawn additively, as the
			// other energy tracers are (elite guard, snarq, quad laser primary).
			m_aoTracerDef[i].uFlags = TRACERFLAG_DRAW_ADDITIVE;
#else
			m_aoTracerDef[i].uFlags = TRACERFLAG_NONE;
#endif

			pTexDef = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, m_aUserProps[i].pszL12_TexName));
			if(pTexDef == NULL)
			{
				DEVPRINTF("CWeaponLaser::InitSystem() : Could not load tracer texture '%s'.\n", m_aUserProps[i].pszL12_TexName);
			}

			m_aTracerTexInst[i].SetTexDef( pTexDef );
		}

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_LASER][i];

		// Do any special limitations on the values we got from the CSV file.

		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->fUnitRecoil = m_aUserProps[i].fUnitRecoil;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_NONE;
		pInfo->nReticleType = CReticle::TYPE_DROID_STANDARD;

		m_apDamageProfile[i] = CDamage::FindDamageProfile( m_aUserProps[i].pszDamageProfile );
#if FANG_WINGC
		for( u32 n=0; n<3; ++n ) {
			cchar *pszName = m_aRetailCharge[i].apszDamage[n];
			m_aRetailCharge[i].apDamage[n] = pszName && fclib_stricmp( pszName, "None" ) ? CDamage::FindDamageProfile( pszName ) : NULL;
		}
#endif
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponLaser::UninitSystem( void ) {
}



CWeaponLaser::CWeaponLaser() {
	_ClearDataMembers();
}



CWeaponLaser::~CWeaponLaser() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}



BOOL CWeaponLaser::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	#if FANG_WINGC
		if( !_bUserPropsAvailable ) {
			DEVPRINTF( "CWeaponLaser::Create(): Skipping '%s'; compatible laser properties are unavailable.\n", pszEntityName ? pszEntityName : "<unnamed>" );
			return FALSE;
		}
	#endif

	// Get pointer to the leaf class's builder object...
	CWeaponLaserBuilder *pBuilder = (CWeaponLaserBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponLaserBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONLASER );
	}

	m_pSoundEmitter = NULL;

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_LASER, pszEntityName, pMtx, pszAIBuilderName );
}



BOOL CWeaponLaser::ClassHierarchyBuild( void ) {
	u32 i;//, j;
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
	CWeaponLaserBuilder *pBuilder = (CWeaponLaserBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

#if FANG_WINGC
	// The firing sound is a retail sound group; the level's sound banks are loaded by now.
	for( i=0; i<EUK_COUNT_LASER; ++i ) {
		_apRetailFireGroup[i] = NULL;
		if( _apszRetailFireGroup[i] && fclib_stricmp( _apszRetailFireGroup[i], "None" ) ) {
			_apRetailFireGroup[i] = CFSoundGroup::RegisterGroup( _apszRetailFireGroup[i] );
		}
		m_aUserProps[i].hFiringSound = _apRetailFireGroup[i] ? CFSoundGroup::GetRandomSoundHandle( _apRetailFireGroup[i] ) : FSNDFX_INVALID_FX_HANDLE;
		_RetailCharge_t &Charge = m_aRetailCharge[i];
		Charge.pChargeSound = CFSoundGroup::RegisterGroup( Charge.pszChargeSound );
		Charge.pBurstSound = CFSoundGroup::RegisterGroup( Charge.pszBurstSound );
		Charge.pBurstSound2 = CFSoundGroup::RegisterGroup( Charge.pszBurstSound2 );
		Charge.pRicochetSound = CFSoundGroup::RegisterGroup( Charge.pszRicochetSound );
		Charge.pReadySound = CFSoundGroup::RegisterGroup( Charge.pszReadySound );
	}
#endif

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_LASER; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_LASER );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		pInfo = &m_aaInfo[WEAPON_TYPE_LASER][i];

		// Create the world mesh...
		pResourceData->m_pWorldMesh = fnew CFWorldMesh;
		if( pResourceData->m_pWorldMesh == NULL ) {
			DEVPRINTF( "CWeaponLaser::Create(): Not enough memory to create m_pWorldMesh.\n" );
			goto _ExitWithError;
		}

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );


		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponLaser::Create(): Could not find mesh '%s'.\n", m_aUserProps[i].apszMeshName[_MESH_WEAPON] );

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
		if( !CreateSharedEUKData( &MeshInit, &uResEUK, &(pResourceData->m_pWorldMesh) ) ) {
			DEVPRINTF( "CWeaponLaser::ClassHierarchyBuild():  Error creating EUK shared data\n" );
			goto _ExitWithError;
		}

		if( uResEUK == i ) {
			pResourceData->m_pWorldMesh->Init( &MeshInit );
			pResourceData->m_pWorldMesh->RemoveFromWorld();
			pResourceData->m_pWorldMesh->SetCollisionFlag( TRUE );
			pResourceData->m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
			pResourceData->m_pWorldMesh->m_pUser = this;
			pResourceData->m_pWorldMesh->SetUserTypeBits( TypeBits() );
			pResourceData->m_pWorldMesh->SetLineOfSightFlag(FALSE); //pgm added this so that weapons never block los tests
		}

#if FANG_WINGC
		m_anMuzzleBoneIndex[i] = -1;
		if( _apszRetailMuzzleBone[uMeshEUK] && pResourceData->m_pWorldMesh->m_pMesh ) {
			m_anMuzzleBoneIndex[i] = pResourceData->m_pWorldMesh->FindBone( _apszRetailMuzzleBone[uMeshEUK] );
			if( m_anMuzzleBoneIndex[i] < 0 ) {
				DEVPRINTF( "CWeaponLaser::ClassHierarchyBuild(): Muzzle bone '%s' not found in L%d mesh; firing from the weapon origin.\n", _apszRetailMuzzleBone[uMeshEUK], i + 1 );
			}
		}
#endif

		if( _UsesL3Beam( uMeshEUK ) )
		{
			m_apL3_LaserBeam[0] = CLaserBeam::GetLaserBeam();
			m_apL3_LaserBeam[1] = CLaserBeam::GetLaserBeam();
			if((m_apL3_LaserBeam[0] == NULL) || (m_apL3_LaserBeam[1] == NULL))
			{
				DEVPRINTF("CWeaponLaser::ClassHierarchyBuild(): Could not get laser beam object.\n");
				goto _ExitWithError;
			}
		}
	}

	m_hTracerGroup = tracer_CreateGroup(&m_aTracerTexInst[m_nUpgradeLevel], 18);
	if(m_hTracerGroup == TRACER_NULLGROUPHANDLE)
	{
		DEVPRINTF( "CWeaponLaser::ClassHierarchyBuild(): Could not create tracer group.\n" );
		goto _ExitWithError;
	}

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	fforce_NullHandle( &m_hForce );
	m_bFiredLastFrame = FALSE;
	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponLaser::ClassHierarchyBuilt( void ) {
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


void CWeaponLaser::ClassHierarchyDestroy( void ) {
	// Destroy ourselves first...
#if FANG_WINGC
	CancelPlayerCharge();
#endif

	fdelete( m_apL3_LaserBeam[0] );
	fdelete( m_apL3_LaserBeam[1] );

	fforce_Kill( &m_hForce );

	tracer_DestroyGroup(m_hTracerGroup);
	m_hTracerGroup = TRACER_NULLGROUPHANDLE;


	for( u32 i=0; i<EUK_COUNT_LASER; i++ ) {
		DestroySharedEUKData( i );
	}

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponLaser::GetLeafClassBuilder( void ) {
	return &_WeaponLaserBuilder;
}


void CWeaponLaser::_ClearDataMembers( void ) {
	u32 i;//, j;

	for( i=0; i<EUK_COUNT_LASER; ++i )
	{
		m_aResourceData[i].m_pWorldMesh = NULL;
	}

	m_pResourceData = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_fRechargeCntDn = 0.0f;
	m_L3_fDrainCntDn = 0.0f;
	m_bFireThisFrame = FALSE;
	m_bRechargeThisFrame = FALSE;
	m_bFiredLastFrame = FALSE;
#if FANG_WINGC
	m_PlayerCharge.Cancel( false );
	m_pChargeSoundEmitter = NULL;
	m_bChargedShotThisFrame = FALSE;
	m_bChargeReadySoundPlayed = FALSE;
	m_bSecondaryTapThisFrame = FALSE;
	m_nChargedShotTier = 0;
	m_hChargeParticle = FPARTICLE_INVALID_HANDLE;
	m_vChargeMuzzle_WS.Zero();
	m_fChargePitchBase = 1.0f;
#endif
	m_hTracerGroup = TRACER_NULLGROUPHANDLE;
	m_pvecBuddyFirePos_WS = NULL;

	m_vTargetPos_WS.Zero();
	m_vecL3_EndPoint.Zero();
	m_apL3_LaserBeam[0] = NULL;
	m_apL3_LaserBeam[1] = NULL;
#if FANG_WINGC
	for( u32 i=0; i<EUK_COUNT_LASER; ++i ) {
		m_anMuzzleBoneIndex[i] = -1;
	}
#endif
}


void CWeaponLaser::_TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
#if FANG_WINGC
	if( pTracerDef->nUser & _CHARGED_TRACER ) {
		_ChargedTracerCallback( pTracerDef, nKillReason, pImpact );
		return;
	}
#endif
	if( nKillReason == TRACER_KILLREASON_HIT_GEO ) {
		FASSERT(pTracerDef->pUser != NULL);
		CWeaponLaser *pLaser = (CWeaponLaser *)(pTracerDef->pUser);


		const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );

		pCollMaterial->DrawAllDrawableParticles(
			&pImpact->ImpactPoint,
			&pImpact->UnitFaceNormal,
			TRUE,
			fmath_RandomFloatRange( 0.0f, 0.0f ),
			fmath_RandomFloatRange( 0.0f, 0.1f ),
			fmath_RandomFloatRange( 0.0f, 0.1f )
		);

//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	fforce_Kill( &pLaser->m_hForce );
//
//	FDecalDefHandle_t hDecalDef;
//	hDecalDef = CFDecal::GetDefByName( "explosion" );
//	CFDecalDef decaldef = *(CFDecalDef*)hDecalDef;
//	decaldef.fMinRadius = decaldef.fMaxRadius = 20.0f;
//	CFDecal::Create( &decaldef, pImpact, &pTracerDef->UnitDir_WS );
//
//#endif

		//FDecalDefHandle_t hDecal = CFDecal::GetDefByName( "rocket" );

		CFDecal::Create( m_aUserProps[pLaser->m_nUpgradeLevel].hDecalDef, pImpact, &pTracerDef->UnitDir_WS );
		//CFDecal::Create( hDecal, pImpact );

		if(pImpact->pTag == NULL)
		{
//#if !SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//			if( pCollMaterial->IsPockMarkEnabled( pImpact ) )
//			{
//				potmark_NewPotmark(&pImpact->ImpactPoint, &pImpact->UnitFaceNormal);
//			}
//#endif
		
		}


		else
		{
			CFWorldMesh *pHitWorldMesh = (CFWorldMesh *)(pImpact->pTag);

			// See if the world mesh that we found is an entity...
			if(pHitWorldMesh->m_nUser == MESHTYPES_ENTITY)
			{
				// It's an Entity!
				CEntity *pEDamagee = (CEntity *)(pHitWorldMesh->m_pUser);

				//if( pCollMaterial->IsPockMarkEnabled( pImpact ) )
				//{
				//	if( !(pEDamagee->TypeBits() & ENTITY_BIT_BOT) && !(pEDamagee->TypeBits() & ENTITY_BIT_SHIELD) && !(pEDamagee->TypeBits() & ENTITY_BIT_PROJECTILE)) {
				//		potmark_NewPotmark( &pImpact->ImpactPoint, &pImpact->UnitFaceNormal );
				//	}
				//}

				// Get an empty damage form...
				CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

				if( pDamageForm )
				{
					// Fill out the form...

					pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
					pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
					pDamageForm->m_pDamageProfile = pLaser->m_apDamageProfile[pLaser->m_nUpgradeLevel];
					pDamageForm->m_Damager.pWeapon = pLaser;
					pDamageForm->m_Damager.pBot = pLaser->GetOwner();
					pDamageForm->m_Damager.nDamagerPlayerIndex = pLaser->GetOwner() ? pLaser->GetOwner()->m_nPossessionPlayerIndex : -1;
					pDamageForm->m_pDamageeEntity = pEDamagee;
					pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pImpact, &pTracerDef->UnitDir_WS );

					CDamage::SubmitDamageForm( pDamageForm );
				}
			}
		}

		//make an AI impact sound in case any AI are listening
		if( pLaser->m_pOwnerBot->m_nPossessionPlayerIndex > -1 && pLaser->m_fSecsUntilNextSound <= 0.0f)
		{
			AIEnviro_AddSound( pImpact->ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, pLaser );
			pLaser->m_fSecsUntilNextSound = 0.5f;
		}
	}
}



void CWeaponLaser::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponLaser::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst(pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);
	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType(CItemInst::AMMODISPLAY_METER, CItemInst::AMMODISPLAY_METER);
	}
}



void CWeaponLaser::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pWorldMesh->UpdateTracker();
}


void CWeaponLaser::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );
#if FANG_WINGC
	CancelPlayerCharge();
#endif

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();
}


void CWeaponLaser::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );
#if FANG_WINGC
	if( !bDrawingHasBeenEnabled ) CancelPlayerCharge();
#endif

//	if( IsInWorld() ) 
	{
		u32 i;

		if( bDrawingHasBeenEnabled ) {
			// Enable drawing of this weapon...

			for( i=0; i<EUK_COUNT_LASER; ++i ) {
				FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		} else {
			// Disable drawing of this weapon...

			for( i=0; i<EUK_COUNT_LASER; ++i ) {
				FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}
	}
}


void CWeaponLaser::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}
}


void CWeaponLaser::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;
}

// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponLaser::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_LASER][nPreviousUpgradeLevel];

	// Remove old mesh from world...
	pOldResourceData->m_pWorldMesh->RemoveFromWorld();

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}

	tracer_SetNewTexture( m_hTracerGroup, &m_aTracerTexInst[m_nUpgradeLevel] );
}



void CWeaponLaser::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

#if FANG_WINGC
	// The weapon mesh is created without a bone palette, so use the at-rest muzzle bone.
	const s32 nBone = m_anMuzzleBoneIndex[m_nUpgradeLevel];
	if( nBone >= 0 ) {
		m_MtxToWorld.MulPoint( *pMuzzlePoint_WS, m_pResourceData->m_pWorldMesh->m_pMesh->pBoneArray[nBone].AtRestBoneToModelMtx.m_vPos );
		return;
	}
#endif
	pMuzzlePoint_WS->Mul( m_MtxToWorld.m_vFront, m_aUserProps[m_nUpgradeLevel].fDistFromWeaponOrigToMuzzle ).Add( m_MtxToWorld.m_vPos );
}



// This function is called to let the weapon know what the trigger values are.
// It should be called whether or not the triggers are pressed because this function
// is responsible for firing and reloading of the weapon.
//
// pProjUnitDir_WS must point in the direction that the weapon will fire.
//
// Return value:
//   Bit 0: Trigger #1 fired
//   Bit 1: Trigger #2 fired
#if FANG_WINGC
void CWeaponLaser::_StopChargeSound( void ) {
	if( m_pChargeSoundEmitter ) {
		m_pChargeSoundEmitter->Destroy();
		m_pChargeSoundEmitter = NULL;
	}
	m_bChargeReadySoundPlayed = FALSE;
}

void CWeaponLaser::CancelPlayerCharge( void ) {
	m_PlayerCharge.Cancel();
	_StopChargeSound();
	_StopChargeParticle();
	m_bFireThisFrame = m_bChargedShotThisFrame = FALSE;
	m_bSecondaryTapThisFrame = FALSE;
}

static BOOL _SuperLaserEnabled() {
    static const BOOL enabled=pcmods_Feature("super_mining_lasers");
    return enabled;
}

BOOL CWeaponLaser::SupportsPlayerCharge( void ) const {
	return IsOwnedByPlayer() && m_aRetailCharge[m_nUpgradeLevel].apDamage[0] &&
		m_aRetailCharge[m_nUpgradeLevel].apDamage[1] && m_aRetailCharge[m_nUpgradeLevel].apDamage[2];
}

BOOL CWeaponLaser::UsesMouseChargeControls( void ) const {
#if defined(MA_PC_INPUT)
	const s32 player = GetSafeOwnerPlayerIndex();
	return player >= 0 && player < MAX_PLAYERS && pcinput_UsesMouseWeaponControls(Player_aPlayer[player].m_nControllerIndex);
#else
	return FALSE;
#endif
}

u16 CWeaponLaser::_PlayerChargeCost( void ) const {
	// User-selected PC energy rule: four charged bursts per full bar. Round up
	// for nonstandard clip sizes, and never permit a zero-cost burst.
	return (u16)FMATH_MAX( 1u, ((u32)m_pInfo->nClipAmmoMax + 3u) / 4u );
}

u32 CWeaponLaser::_PlayerChargeTrigger( f32 fTrigger, const CFVec3A *pTargetPos_WS, const CFVec3A *pBuddyFirePos_WS ) {
	const _RetailCharge_t &Charge = m_aRetailCharge[m_nUpgradeLevel];
	const BOOL bControlled = CurrentState() == STATE_DEPLOYED && GetOwner() && !GetOwner()->IsDeadOrDying();
	// Each remaining quarter-bar can charge without waiting for a full refill.
	const BOOL bReady = m_nClipAmmo >= _PlayerChargeCost() && m_fSecondsCountdownTimer <= 0.0f;
	// Retail charges energy from 0.5 to 1 at 0.5 / field34 per second.
	const PCLaserCharge::Result Shot = m_PlayerCharge.Sample( fTrigger, bControlled != FALSE, bReady != FALSE,
		FLoop_fPreviousLoopSecs, Charge.fDelay, Charge.fChargeSeconds, !UsesMouseChargeControls() );
	m_bRechargeThisFrame = !m_PlayerCharge.IsCharging();
	if( !m_PlayerCharge.IsCharging() ) _StopChargeSound();
	m_bSecondaryTapThisFrame = Shot.shot == PCLaserCharge::SECONDARY_TAP;
	if( Shot.shot != PCLaserCharge::CHARGED || m_nClipAmmo == 0 || m_fSecondsCountdownTimer > 0.0f ) return 0;
	m_bChargedShotThisFrame = Shot.shot == PCLaserCharge::CHARGED;
	m_nChargedShotTier = Shot.power >= 0.95f ? 2 : (Shot.power >= 0.5f ? 1 : 0);
	m_bRechargeThisFrame = FALSE;
	m_vTargetPos_WS.Set( *pTargetPos_WS );
	GetOwner()->FocusHumanTargetPoint_WS( &m_vTargetPos_WS, this );
	m_pvecBuddyFirePos_WS = pBuddyFirePos_WS;
	m_bFireThisFrame = TRUE;
	return 1;
}

void CWeaponLaser::_ConfigureNormalTracer( TracerDef_t *pTracer ) const {
    // Super Mining Lasers replaces the ordinary impact callback and launcher,
    // retaining the ordinary projectile dimensions and thin collision mask.
    if(!_SuperLaserEnabled() || !m_aRetailCharge[m_nUpgradeLevel].apDamage[0]) return;
    pTracer->nUser=(u16)(_CHARGED_TRACER | m_nUpgradeLevel);
    pTracer->pFcnKillCallback=_ChargedTracerCallback;
    pTracer->pFcnBuildSkipList=_ChargedSkipList;
}

void CWeaponLaser::_ConfigureChargedTracer( TracerDef_t *pTracer ) const {
	const _RetailCharge_t &Charge = m_aRetailCharge[m_nUpgradeLevel];
	pTracer->nUser = (u16)(_CHARGED_TRACER | (m_nChargedShotTier << 3) | m_nUpgradeLevel);
	pTracer->uFlags |= TRACERFLAG_THICK_PROJECTILE | TRACERFLAG_DEFER_FIRST_WORK;
	pTracer->pFcnKillCallback = _ChargedTracerCallback;
	pTracer->pFcnBuildSkipList = _ChargedSkipList;
	pTracer->fSpeed_WS = Charge.fSpeed;
	pTracer->fLength_WS = Charge.fLength;
	pTracer->fWidth_WS = Charge.fWidth;
	pTracer->fMaxTailDist_WS = Charge.fRange;
	pTracer->ColorRGBA.Set( Charge.afColor[0], Charge.afColor[1], Charge.afColor[2], 1.0f );
}

CDamageProfile *CWeaponLaser::_ChargedDamageProfile( u16 nShot ) {
	const u32 nLevel = nShot & 7, nTier = (nShot >> 3) & 3;
	if( !(nShot & _CHARGED_TRACER) || nLevel >= EUK_COUNT_LASER || nTier >= 3 ) return NULL;
	return m_aRetailCharge[nLevel].apDamage[nTier];
}

void CWeaponLaser::_ChargedSkipList( void *pWeapon ) {
	CWeaponLaser *pLaser = (CWeaponLaser *)pWeapon;
	if( pLaser->GetOwner() ) pLaser->GetOwner()->AppendTrackerSkipList();
	else pLaser->AppendTrackerSkipList();
}

void CWeaponLaser::_ChargeParticleKilled( FParticle_EmitterHandle_t hEmitter, void *pUser ) {
	CWeaponLaser *pLaser = (CWeaponLaser *)pUser;
	if( pLaser->m_hChargeParticle == hEmitter ) pLaser->m_hChargeParticle = FPARTICLE_INVALID_HANDLE;
}

void CWeaponLaser::_StopChargeParticle( void ) {
	if( m_hChargeParticle ) {
		// The emitter follows a member position. Kill it before that member can be freed.
		fparticle_KillEmitter( m_hChargeParticle );
		m_hChargeParticle = FPARTICLE_INVALID_HANDLE;
	}
}

// Retail burst collision: main.dol 801fa9c4, 801fafb8 and 801fb18c.
// The shot snapshots its upgrade, charge band and damager; later weapon switches
// and other players cannot change an in-flight shot's damage or ownership.
void CWeaponLaser::_ChargedTracerCallback( TracerDef_t *pTracer, TracerKillReason_e nReason, const FCollImpact_t *pImpact ) {
	CDamageProfile *pDamage = _ChargedDamageProfile( pTracer->nUser );
	if( !pDamage || !pTracer->pUser ) return;
	CWeaponLaser *pLaser = (CWeaponLaser *)pTracer->pUser;
	const _RetailCharge_t &Charge = m_aRetailCharge[pTracer->nUser & 7];
	const f32 fIntensity = 0.5f * (f32)((pTracer->nUser >> 3) & 3);
	if( nReason == TRACER_KILLREASON_REACHED_MAX_DIST ) {
		if( Charge.hEndParticle ) {
			CFVec3A End;
			End.Mul( pTracer->UnitDir_WS, Charge.fRange ).Add( pTracer->TailPos_WS );
			fparticle_SpawnEmitter( Charge.hEndParticle, End.v3, &pTracer->UnitDir_WS.v3, fIntensity );
		}
		return;
	}
	if( nReason != TRACER_KILLREASON_HIT_GEO || !pImpact ) return;

	TracerDef_t Next = *pTracer;
	Next.pFcnBuildSkipList = NULL; // A reflected shot can return to its firer, as in retail.
	Next.uFlags |= TRACERFLAG_DEFER_FIRST_WORK;
	if( CGColl::ExtractWire( pImpact ) ) {
		Next.TailPos_WS.Mul( Next.UnitDir_WS, 0.01f ).Add( pImpact->ImpactPoint );
		tracer_NewTracer( pLaser->m_hTracerGroup, &Next, 0.0f, FALSE, FALSE, &pTracer->Damager );
		return;
	}

	const u8 nReaction = CGColl::ComputeProjectileReaction( pImpact );
	const BOOL bReflectOnly = nReaction == CEntity::PROJECTILE_REACTION_RICOCHET || nReaction == CEntity::PROJECTILE_REACTION_SHIELD;
	FExplosion_GroupHandle_t hGroup = CExplosion2::GetExplosionGroup( bReflectOnly ? Charge.pszRicochetImpact : Charge.pszImpact );
	if( hGroup ) {
		FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();
		if( hSpawner ) {
			FExplosionSpawnParams_t Params;
			Params.InitToDefaults();
			Params.uFlags = FEXPLOSION_SPAWN_NO_SMOKE; // Retail passes 0x40; burst damage is a separate direct hit.
			Params.Pos_WS = pImpact->ImpactPoint;
			Params.UnitDir = pImpact->UnitFaceNormal;
			Params.uSurfaceType = pImpact->nUserType;
			CExplosion2::SpawnExplosion( hSpawner, hGroup, &Params );
		}
	}

	Next.UnitDir_WS.ReceiveReflection( pTracer->UnitDir_WS, pImpact->UnitFaceNormal );
	Next.UnitDir_WS.Unitize();
	Next.TailPos_WS.Mul( Next.UnitDir_WS, 0.01f ).Add( pImpact->ImpactPoint );
	tracer_NewTracer( pLaser->m_hTracerGroup, &Next, 0.0f, FALSE, FALSE, &pTracer->Damager );
	if( bReflectOnly ) {
		CFSoundGroup::PlaySound( Charge.pRicochetSound, FALSE, &pImpact->ImpactPoint );
	} else {
		FDecalDefHandle_t hDecal = CFDecal::GetDefByName( Charge.pszDecal );
		if( hDecal ) CFDecal::Create( hDecal, pImpact, &pTracer->UnitDir_WS );
		const CGCollMaterial *pMaterial = CGColl::GetMaterial( pImpact );
		pMaterial->DrawAllDrawableParticles( &pImpact->ImpactPoint, &Next.UnitDir_WS, TRUE, fIntensity, fIntensity, fIntensity );
	}

	CEntity *pHit = CGColl::ExtractEntity( pImpact );
	if( pHit ) {
		CDamageForm *pForm = CDamage::GetEmptyDamageFormFromPool();
		if( pForm ) {
			pForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
			pForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pForm->m_pDamageProfile = pDamage;
			pForm->m_Damager = pTracer->Damager;
			pForm->m_pDamageeEntity = pHit;
			pForm->InitTriDataFromCollImpact( (CFWorldMesh *)pImpact->pTag, pImpact, &pTracer->UnitDir_WS );
			CDamage::SubmitDamageForm( pForm );
		}
	}
}

#endif

u32 CWeaponLaser::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );

	m_bFireThisFrame = FALSE;
#if FANG_WINGC
	m_bChargedShotThisFrame = FALSE;
	m_bSecondaryTapThisFrame = FALSE;
	// Primary fire retains rapid shots. A secondary tap uses the selected item;
	// holding secondary reserves the press for a charge and never throws on release.
	if( SupportsPlayerCharge() ) {
		const BOOL bMouse = UsesMouseChargeControls();
        const f32 fChargeTrigger = bMouse ? fUnitTriggerVal2 : fUnitTriggerVal1;
        const u32 nChargedFire = _PlayerChargeTrigger( fChargeTrigger, pvTargetPos_WS, pBuddyFirePos_WS );
        if( nChargedFire || (bMouse ? fChargeTrigger >= 0.25f :
            ((fChargeTrigger > 0.0f && fChargeTrigger < 1.0f) || m_PlayerCharge.IsCharging())) ) return nChargedFire;
	}
#endif

	if(fUnitTriggerVal1 < 0.25f)
	{
		// Trigger not down...
		m_bRechargeThisFrame = TRUE;
		return(0);
	}

	m_bRechargeThisFrame = FALSE;

	if(CurrentState() != STATE_DEPLOYED)
	{
		// Not in a state to fire...
		return(0);
	}

	u32 uLowerThreshold;
	if( _UsesL3Beam( m_nUpgradeLevel ) && (!m_bFiredLastFrame))
	{
		uLowerThreshold = 10;
	}
	else
	{
		uLowerThreshold = 0;
	}

	if(m_nClipAmmo <= uLowerThreshold)
	{
		// No rounds in clip...
		return(0);
	}

	if( !_UsesL3Beam( m_nUpgradeLevel ) && (m_fSecondsCountdownTimer > 0.0f))
	{
		// Can't fire another round yet...
		return(0);
	}

	m_vTargetPos_WS.Set( *pvTargetPos_WS );
	if ( m_pOwnerBot && m_pOwnerBot->m_nPossessionPlayerIndex >= 0 )
	{
		m_pOwnerBot->FocusHumanTargetPoint_WS( &m_vTargetPos_WS, this );
	}

	// We passed all the tests, let's go ahead and fire off a round.
	m_bFireThisFrame = TRUE;
	m_pvecBuddyFirePos_WS = pBuddyFirePos_WS;

	return(1);
}


f32 CWeaponLaser::GetProjectileSpeed(void) const
{
	return m_aoTracerDef[m_nUpgradeLevel].fSpeed_WS;
}



static void _TrackPlayerLaser(void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied)
{
	CWeaponLaser* pLaser = (CWeaponLaser*) pUserData;

	if (bDied)
	{
//		pLaser->m_uAISoundHandle = 0;
	}
	else
	{
		if( pLaser->m_uAISoundHandle &&
			!AIEnviro_ModifySound( pLaser->m_uAISoundHandle,  NewPos_WS, AIEnviro_fProjectileVisualRadius, 0.1f, AISOUNDTYPE_PROJECTILE, pLaser->GetOwner()) )
		{
			pLaser->m_uAISoundHandle = 0;
		}
	}

}
// Called once per frame to update animations, timers, etc.
void CWeaponLaser::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	static const f32 _fPerturbFactor = 0.01f;
#if FANG_WINGC
	if( CurrentState() != STATE_DEPLOYED || !GetOwner() || GetOwner()->IsDeadOrDying() || !IsOwnedByPlayer() ) {
		// Keep the original AI fire request, but discard a player's pending charge.
		if( m_PlayerCharge.IsCharging() || m_bChargedShotThisFrame ) CancelPlayerCharge();
	}
	if( m_PlayerCharge.IsCharging() ) {
		const _RetailCharge_t &Charge = m_aRetailCharge[m_nUpgradeLevel];
		CFVec3A Muzzle;
		ComputeMuzzlePoint_WS( &Muzzle );
		if( !m_pChargeSoundEmitter && Charge.pChargeSound ) {
			m_pChargeSoundEmitter = CFSoundGroup::AllocAndPlaySound( Charge.pChargeSound, TRUE, &Muzzle, 1.0f, 1.0f, -1.0f, NULL, &m_fChargePitchBase );
		}
		if( m_pChargeSoundEmitter ) m_pChargeSoundEmitter->SetFrequencyFactor( m_fChargePitchBase * FMATH_FPOT( m_PlayerCharge.Power(), Charge.fPitchStart, Charge.fPitchEnd ) );
		m_vChargeMuzzle_WS = Muzzle;
		if( !m_hChargeParticle && Charge.hChargeParticle && IsDrawEnabled() ) {
			m_hChargeParticle = fparticle_SpawnEmitter( Charge.hChargeParticle, &m_vChargeMuzzle_WS.v3, &m_MtxToWorld.m_vFront.v3, NULL, m_PlayerCharge.Power() );
			if( m_hChargeParticle ) fparticle_SetKillCallback( m_hChargeParticle, _ChargeParticleKilled, this );
		}
		if( m_hChargeParticle ) fparticle_SetIntensity( m_hChargeParticle, m_PlayerCharge.Power() );
		if( m_PlayerCharge.Power() >= 1.0f && !m_bChargeReadySoundPlayed ) {
			if( Charge.pReadySound ) CFSoundGroup::PlaySound( Charge.pReadySound, TRUE, &Muzzle, GetSafeOwnerPlayerIndex() );
			m_bChargeReadySoundPlayed = TRUE;
		}
		if( IsDrawEnabled() && !Charge.hChargeParticle ) {
			const f32 fSize = Charge.fWidth * (0.25f + 0.75f * m_PlayerCharge.Power());
			CMuzzleFlash::AddFlash_Card3D( m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_YELLOW], Muzzle, fSize, fSize, 0.0f );
		}
	} else {
		_StopChargeParticle();
	}
#endif

	if(m_bFireThisFrame)
	{
		//////////////////////////////////////////////////////////////////////
		// Do the effects for the firing.
		BOOL bPlayForce = FALSE;
		CFVec3A vecPerturb;
		CFVec3A vFireUnitDir;
		CFVec3A vecMuzzlePoint;

		
		//vFireUnitDir.Sub( m_vTargetPos_WS, vecMuzzlePoint );
		//f32 fMag2 = vFireUnitDir.MagSq();
		//if( fMag2 < 0.001f ) {
		//	// too close
		//	vFireUnitDir = m_MtxToWorld.m_vFront;
		//}
		//
		//vFireUnitDir.Mul( fmath_InvSqrt( fMag2 ) );

		ComputeMuzzlePoint_WS(&vecMuzzlePoint);
		ComputeFireUnitDir( &vFireUnitDir, &vecMuzzlePoint, &m_MtxToWorld.m_vFront, &m_vTargetPos_WS, FALSE );

		vecPerturb.Set(fmath_RandomFloatRange(-_fPerturbFactor, _fPerturbFactor), fmath_RandomFloatRange(-_fPerturbFactor, _fPerturbFactor), fmath_RandomFloatRange(-_fPerturbFactor, _fPerturbFactor));
		vFireUnitDir.Add(vecPerturb);
		vFireUnitDir.Unitize();


		// Levels other than the L3 beam share the tracer case.
		switch( _UsesL3Beam( m_nUpgradeLevel ) ? 2 : 0 )
		{
			case 2:
			{
				if((m_nClipAmmo >= _uFizzleThreshhold) || (fmath_RandomChance(1.0f - (f32)(m_nClipAmmo) * (1.0f / (f32)(_uFizzleThreshhold)))))
				{
					m_apL3_LaserBeam[0]->SetStartPosAndDir(vecMuzzlePoint, vFireUnitDir, this, m_apDamageProfile[2]);

					if( IsDrawEnabled() ) {
						CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], vecMuzzlePoint, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, 0.5f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
						CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], vecMuzzlePoint, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, -0.4f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
					}

					if(m_pvecBuddyFirePos_WS != NULL)
					{
						m_apL3_LaserBeam[1]->SetStartPosAndDir(*m_pvecBuddyFirePos_WS, vFireUnitDir, this, m_apDamageProfile[2]);

						if( IsDrawEnabled() ) {
							CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], *m_pvecBuddyFirePos_WS, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, 0.5f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
							CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], *m_pvecBuddyFirePos_WS, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, -0.4f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
						}
					}

					if ( IsOwnedByPlayer() )
					{
						if ( !m_pSoundEmitter )
						{
							m_pSoundEmitter = FSNDFX_ALLOCNPLAY2D( m_aUserProps[m_nUpgradeLevel].hFiringSound, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
						}
					}
					else
					{
						if ( !m_pSoundEmitter )
						{
							m_pSoundEmitter = FSNDFX_ALLOCNPLAY3D( m_aUserProps[m_nUpgradeLevel].hFiringSound, &vecMuzzlePoint, m_aUserProps[m_nUpgradeLevel].fMaxLiveRange, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
						}
					}

					if (GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1)
					{
						AIEnviro_BoostPlayerSoundTo(GetOwner()->m_nPossessionPlayerIndex, 30.0f);
					}

					bPlayForce = TRUE;
				}
				else if ( m_pSoundEmitter )
				{
					m_pSoundEmitter->Destroy();
					m_pSoundEmitter = NULL;
				}
				break;

			}
			case 0:
			case 1:
			case 3:
			default:
			{
				m_aoTracerDef[m_nUpgradeLevel].pUser = this;
#if FANG_WINGC
				m_aoTracerDef[m_nUpgradeLevel].ColorRGBA.Set( _aRetailTracerColor[m_nUpgradeLevel][0], _aRetailTracerColor[m_nUpgradeLevel][1],
															  _aRetailTracerColor[m_nUpgradeLevel][2], _aRetailTracerColor[m_nUpgradeLevel][3] );
#else
				m_aoTracerDef[m_nUpgradeLevel].ColorRGBA.OpaqueWhite();
#endif
				m_aoTracerDef[m_nUpgradeLevel].UnitDir_WS = vFireUnitDir;
				m_aoTracerDef[m_nUpgradeLevel].TailPos_WS = vecMuzzlePoint;
//				m_aoTracerDef[m_nUpgradeLevel].ppCollSkipTrackers = &m_pOwnerBot->m_pWorldMesh;

				CFCamera *pCamera = fcamera_GetCameraByIndex(CPlayer::m_pCurrent->m_nPlayerIndex);

				if(m_nUpgradeLevel == 1)
				{
					if( IsDrawEnabled() ) {
						CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_YELLOW], vecMuzzlePoint, m_aUserProps[1].fMuzzleWidth, m_aUserProps[1].fMuzzleHeight, 0.5f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
					}
				}

				ConstructDamagerData();
				TracerDef_t ShotTracer = m_aoTracerDef[m_nUpgradeLevel];
				f32 fHeadDistanceOut = 0.2f;
#if FANG_WINGC
                const BOOL bSuperLaser=_SuperLaserEnabled() && m_aRetailCharge[m_nUpgradeLevel].apDamage[0];
                _ConfigureNormalTracer(&ShotTracer);
				// A local copy prevents charged properties leaking to the next normal shot
				// or another co-op player's laser of the same upgrade level.
				if( m_bChargedShotThisFrame ) {
					_ConfigureChargedTracer( &ShotTracer );
					fHeadDistanceOut = 0.0f;
					const _RetailCharge_t &Charge = m_aRetailCharge[m_nUpgradeLevel];
					if( Charge.hBurstParticle && IsDrawEnabled() ) fparticle_SpawnEmitter( Charge.hBurstParticle, vecMuzzlePoint.v3, &vFireUnitDir.v3, 0.5f * (f32)m_nChargedShotTier );
				}
#endif
				if (tracer_NewTracer(m_hTracerGroup, &ShotTracer, fHeadDistanceOut, FALSE, FALSE, &CDamageForm::m_TempDamager))
				{

					if (m_pOwnerBot->m_nPossessionPlayerIndex > -1)
					{
						if( !m_uAISoundHandle || !AIEnviro_SoundHandleToPtr(m_uAISoundHandle)) {
							m_uAISoundHandle = AIEnviro_AddSound( vecMuzzlePoint, AIEnviro_fProjectileVisualRadius, 0.1f, AISOUNDTYPE_PROJECTILE, AISOUNDCTRL_VISIBLE_ENTITY, GetOwner());
						}
						tracer_AddExtraParamsToLastTracer(_TrackPlayerLaser, (void*) this);
					}
				
				}
				if(m_pvecBuddyFirePos_WS != NULL)
				{
					ShotTracer.UnitDir_WS = vFireUnitDir;
					ShotTracer.TailPos_WS = *m_pvecBuddyFirePos_WS;

					ConstructDamagerData();
					tracer_NewTracer(m_hTracerGroup, &ShotTracer, fHeadDistanceOut, FALSE, FALSE, &CDamageForm::m_TempDamager);
				}

#if FANG_WINGC
                if(bSuperLaser && !m_bChargedShotThisFrame) {
                    const _RetailCharge_t &charge=m_aRetailCharge[m_nUpgradeLevel];
                    if(charge.hBurstParticle && IsDrawEnabled()) fparticle_SpawnEmitter(charge.hBurstParticle,vecMuzzlePoint.v3,&vFireUnitDir.v3,0.0f);
                }
                if(m_bChargedShotThisFrame || bSuperLaser)
                    CFSoundGroup::PlaySound(m_aRetailCharge[m_nUpgradeLevel].pBurstSound2,IsOwnedByPlayer(),&vecMuzzlePoint,GetSafeOwnerPlayerIndex());
                if(bSuperLaser && IsOwnedByPlayer()) {
                    CFCamera *camera=fcamera_GetCameraByIndex(GetSafeOwnerPlayerIndex());
                    if(camera) camera->ShakeCamera(0.05f,0.1f);
                }
				CFSoundGroup *pShotSound = (m_bChargedShotThisFrame || bSuperLaser) ? m_aRetailCharge[m_nUpgradeLevel].pBurstSound : _apRetailFireGroup[m_nUpgradeLevel];
				if( pShotSound ) {
					// the group carries the retail volume, pitch and radius for the shot
					if( IsOwnedByPlayer() ) {
						CFSoundGroup::PlaySound( pShotSound, TRUE, &vecMuzzlePoint, GetOwner() ? GetOwner()->m_nPossessionPlayerIndex : -1 );
					} else {
						CFSoundGroup::PlaySound( pShotSound, FALSE, &vecMuzzlePoint );
					}
				} else
#endif
				if ( IsOwnedByPlayer() )
				{
					fsndfx_Play2D( m_aUserProps[m_nUpgradeLevel].hFiringSound,
									0.75f);	//fVolumeMultiplier  =
				}
				else
				{
					fsndfx_Play3D( m_aUserProps[m_nUpgradeLevel].hFiringSound,
									&vecMuzzlePoint,
									450.0f,
									-1.f,
									0.6f);//fVolumeMultiplier);
				}

				if (GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1)
				{
					AIEnviro_BoostPlayerSoundTo(GetOwner()->m_nPossessionPlayerIndex, 30.0f);
				}
	
				bPlayForce = TRUE;

				break;
			}
		}

		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Force feedback effects.
		if(bPlayForce && IsOwnedByPlayer())
		{
			// Human player...
			fforce_Kill(&m_hForce);
			fforce_Play(Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_LIGHT_PULSE, &m_hForce);
		}
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Update timer variables and do any appropriate energy depletion.
		switch( _UsesL3Beam( m_nUpgradeLevel ) ? 2 : 0 )
		{
			case 2:
			{
				// See if 
				m_L3_fDrainCntDn -= FLoop_fPreviousLoopSecs;
				while(m_L3_fDrainCntDn <= 0.0f)
				{
					// JUSTIN: Have this come from the CSV file.
					m_L3_fDrainCntDn += m_aUserProps[m_nUpgradeLevel].fOORoundsPerSec;
					RemoveFromClip(1);
				}
				break;
			}
			case 0:
			case 1:
			case 3:
			default:
			{
				// Set timer until the next round is ready...
#if FANG_WINGC
				if( m_bChargedShotThisFrame ) {
					const _RetailCharge_t &Charge = m_aRetailCharge[m_nUpgradeLevel];
					m_fSecondsCountdownTimer += Charge.fDelay;
					RemoveFromClip( _PlayerChargeCost() );
				} else
#endif
				{
				m_fSecondsCountdownTimer += m_aUserProps[m_nUpgradeLevel].fOORoundsPerSec;
				RemoveFromClip(1);
				}
				break;
			}
		}
		//
		//////////////////////////////////////////////////////////////////////

		m_bFiredLastFrame = TRUE;
	}
	else
	{
		m_bFiredLastFrame = FALSE;

		if ( m_pSoundEmitter )
		{
			m_pSoundEmitter->Destroy();
			m_pSoundEmitter = NULL;
		}
	}

	if(m_bRechargeThisFrame)
	{
		// Replenish the energy.
		m_fRechargeCntDn -= FLoop_fPreviousLoopSecs;

		while(m_fRechargeCntDn <= 0.0f)
		{
			m_fRechargeCntDn += m_aUserProps[m_nUpgradeLevel].fOORechargeRate;			// JUSTIN: Get this from a CSV.
			SetClipAmmo(GetClipAmmo() + 1);
		}
	}

	if(m_fSecondsCountdownTimer > 0.0f)
	{
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}

	// Do EUK-level-specific work...
	switch( m_nUpgradeLevel )
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	default:
		break;
	}
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponLaser::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();
#if FANG_WINGC
	CancelPlayerCharge();
#endif

	m_fSecondsCountdownTimer = 0.0f;
	m_L3_fDrainCntDn = 0.0f;
	m_fRechargeCntDn = 0.0f;
	m_bFiredLastFrame = FALSE;
}



void CWeaponLaser::NotifyAmmoMightHaveChanged( void ) {

}


