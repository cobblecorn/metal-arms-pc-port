//////////////////////////////////////////////////////////////////////////////////////
// weapon_blaster.cpp - Blaster launcher weapon.
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
#include "weapon_blaster.h"
#include "fgamedata.h"
#include "fres.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fresload.h"
#include "fmesh.h"
#include "fanim.h"
#include "meshtypes.h"
#include "floop.h"
#include "fclib.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "explosion.h"
#include "iteminst.h"
#include "potmark.h"
#include "AI/AIEnviro.h"
#include "damage.h"
#include "fsound.h"
#include "eparticlepool.h"


static const char *_USER_PROP_FILENAME	= "w_blaster.csv";
#define _SMOKE_TEXTURE					"tfp1steam01"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponBlasterBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponBlasterBuilder _WeaponBlasterBuilder;
CWeaponBlaster *CWeaponBlaster::m_pCallbackThis = NULL;


void CWeaponBlasterBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONBLASTER, pszEntityType );
}


BOOL CWeaponBlasterBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponBlasterBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponBlaster
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
CWeaponBlaster::_UserProps_t	CWeaponBlaster::m_aUserProps[EUK_COUNT_BLASTER];
CWeaponBlaster::_SystemData_t	CWeaponBlaster::m_aSystemData[EUK_COUNT_BLASTER];
//SmokeTrailAttrib_t CWeaponBlaster::m_SmokeTrailAttrib;

#if FANG_WINGC
static BOOL _bUserPropsAvailable = FALSE;
static u32 _nAvailableBlasterLevels;
// Retail fields 38/39 are retained until their runtime semantics are established.
static f32 _aRetailExtraProps[CWeapon::EUK_COUNT_BLASTER][2];
#endif


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponBlaster::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_WEAPON]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszMeshName[_MESH_SHELL]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszBoneName[_BONE_PRIMARY_FIRE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszBoneName[_BONE_CARTRIDGE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// apszBoneName[_BONE_BARREL]:
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

	// nPelletCountPerRound:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100,

	// fShotSpreadFactor:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_Pt95,

	// fWeaponCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fShellCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,
	
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

	// fCartridgeXRange
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// fCartridgeXPos
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	// fShellXPos
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,

	// fShellScale
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	// fDoubleStacked
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fShellStackYOffset
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fReloadTimeForOneRound
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// f32 fBarrel2DelayTime
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// pszTracerTexName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszMuzzleFlashTexName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

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

	// fShowShellsOnCartridge
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fUnitCamVibration:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	FGAMEDATA_VOCAB_PARTICLE,		// hParticleSmoke

	// fSmokeUnitIntensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fSmokeCoolDownSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_OO_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000,

	FGAMEDATA_VOCAB_DAMAGE,			// pDamageProfile

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupFire
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEmpty
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupCock
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupRound

	FGAMEDATA_VOCAB_DECAL_DEF,		// hDecalDef


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponBlaster::m_aUserPropMapTable[] = {
	"BlasterL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"BlasterL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"BlasterL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"BlasterMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],

	"BlasterMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[4],

	NULL
};


u32 CWeaponBlaster::GetAvailableLevelCount( void ) {
#if FANG_WINGC
	return _nAvailableBlasterLevels;
#else
	return EUK_COUNT_BLASTER;
#endif
}

#if FANG_WINGC
BOOL CWeaponBlaster::_ReadRetailProperties( void ) {
	FMemFrame_t frame = fmem_GetFrame();
	FGameDataFileHandle_t file = fgamedata_LoadFileToFMem( _USER_PROP_FILENAME );
	BOOL ok = file != FGAMEDATA_INVALID_FILE_HANDLE;
	_nAvailableBlasterLevels = 0;
	for( u32 level=0; ok && level<EUK_COUNT_BLASTER; ++level ) {
		FGameDataTableHandle_t table = fgamedata_GetFirstTableHandle( file, m_aUserPropMapTable[level].pszTableName );
		if( table == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			// Retail has three player levels and no military variants. Require
			// the player levels, and reject a gap before a later military level.
			if( level < 3 ) ok = FALSE;
			continue;
		}
		if( level != _nAvailableBlasterLevels ) { ok = FALSE; break; }
		u32 count = fgamedata_GetNumFields( table );
		if( count != 43 && count != 45 ) { ok = FALSE; break; }
		u8 *dest = (u8 *)&m_aUserProps[level];
		u32 bytes = 0;
		for( u32 field=0; ok && field<43; ++field ) {
			const FGameData_TableEntry_t *entry = &m_aUserPropVocab[field];
			if( entry->nBytesForData > sizeof(_UserProps_t)-bytes ) { ok = FALSE; break; }
			ok = fgamedata_GetFieldFromTable( table, field + ((count==45 && field>=38) ? 2 : 0), entry, dest+bytes );
			bytes += entry->nBytesForData;
		}
		if( bytes != sizeof(_UserProps_t) ) ok = FALSE;
		if( ok && count == 45 ) {
			FGameData_TableEntry_t extra = { FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_FLOAT_X,
				sizeof(f32), F32_DATATABLE_0, F32_DATATABLE_0 };
			ok = fgamedata_GetFieldFromTable( table, 38, &extra, &_aRetailExtraProps[level][0] ) &&
				fgamedata_GetFieldFromTable( table, 39, &extra, &_aRetailExtraProps[level][1] );
		}
		if( ok && (!(m_aUserProps[level].fClipAmmoMax >= 1.0f) ||
			!(m_aUserProps[level].fReloadTimeForOneRound > 0.0f)) ) ok = FALSE;
		if( ok ) ++_nAvailableBlasterLevels;
	}
	fmem_ReleaseFrame( frame );
	if( !ok ) _nAvailableBlasterLevels = 0;
	return ok;
}
#endif

BOOL CWeaponBlaster::InitSystem( void ) 
{
	Info_t *pInfo;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );
#if FANG_WINGC
	_bUserPropsAvailable = FALSE;
#endif

	// Read the user properties for all EUK levels of this weapon...
#if FANG_WINGC
	if( !_ReadRetailProperties() )
#else
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) )
#endif
	{
		DEVPRINTF( "CWeaponBlaster::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
#if FANG_WINGC
		DEVPRINTF( "CWeaponBlaster::InitSystem(): Disabling blaster entities because the retail GameCube table uses an unsupported schema.\n" );
		fres_ReleaseFrame( ResFrame );
		return TRUE;
#else
		goto _ExitWithError;
#endif
	}

#if FANG_WINGC
	_bUserPropsAvailable = TRUE;
	DEVPRINTF( "CWeaponBlaster: loaded %u variants; retail extra fields retained without behavior mapping.\n", GetAvailableLevelCount() );
#endif

	// Do this for each EUK level...
	for( i=0; i<GetAvailableLevelCount(); i++ )
	{
		//fixup the user props
		m_aSystemData[i].bDoubleStacked = m_aUserProps[i].fDoubleStacked == 0.0f ? 0 : 1;
		m_aSystemData[i].bShowShellsOnCartridge = m_aUserProps[i].fShowShellsOnCartridge == 0.0f ? 0 : 1;		

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_BLASTER][i];

		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_SHELL;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->fUnitRecoil = m_aUserProps[i].fUnitRecoil;
		pInfo->nClipAmmoMax = (u16) m_aUserProps[i].fClipAmmoMax;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_DISPLAY_AMMO_TEXT;
		pInfo->nReticleType = CReticle::TYPE_BLASTER;

		// Allocate an array of matrices to hold the shell orientations within the cartridge...
		FASSERT( pInfo->nClipAmmoMax );

		m_aSystemData[i].paCartridgeSlotMtxArray = NULL;
		if (m_aSystemData[i].bShowShellsOnCartridge)
		{
			m_aSystemData[i].paCartridgeSlotMtxArray = fnew CFMtx43A[ pInfo->nClipAmmoMax ];
			if( m_aSystemData[i].paCartridgeSlotMtxArray == NULL ) {
				DEVPRINTF( "CWeaponBlaster::InitSystem(): Not enough memory to allocate m_aSystemData[%u].paCartridgeSlotMtxArray.\n", i );
				goto _ExitWithError;
			}
		}
		m_aSystemData[i].fOOReloadTimeForOneRound =	1.0f / m_aUserProps[i].fReloadTimeForOneRound;
	}

	// Init smoke trail attributes...
	_SetSmokeTrailAttributes();

	_ComputeCartridgeShellMatrices();

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponBlaster::UninitSystem( void ) {
}


void CWeaponBlaster::_SetSmokeTrailAttributes( void )
{
#if 0
	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _SMOKE_TEXTURE );

	m_SmokeTrailAttrib.fScaleMin_WS = 0.17f;
	m_SmokeTrailAttrib.fScaleMax_WS = 0.28f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.1f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.02f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.25f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 0.50f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.45f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.65f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.75f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -1.2f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.White();
	m_SmokeTrailAttrib.EndColorRGB.White();
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 0.25f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.5f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.25f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.5f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 1.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 2.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;
#endif
}


void CWeaponBlaster::_ComputeCartridgeShellMatrices(void)
{
	u32 i,ii;
	CFMtx43A ScaleMtx;
	Info_t *pInfo = NULL;
	_UserProps_t *pUserProps;

	for( i=0, pUserProps=m_aUserProps; i<GetAvailableLevelCount(); ++i, ++pUserProps )
	{ // Each EUK level...

		pInfo = &m_aaInfo[WEAPON_TYPE_BLASTER][i];
		m_aSystemData[i].fShellSlotDist = pUserProps->fCartridgeXRange / (pInfo->nClipAmmoMax>>m_aSystemData[i].bDoubleStacked);

		if (m_aSystemData[i].bShowShellsOnCartridge)
		{
			ScaleMtx.Identity();
			ScaleMtx.Mul(pUserProps->fShellScale);


			for( ii=0; ii <pInfo->nClipAmmoMax; ++ii)
			{
				f32 fPos = pUserProps->fShellXPos - m_aSystemData[i].fShellSlotDist * (f32)(ii>>m_aSystemData[i].bDoubleStacked);
				m_aSystemData[i].paCartridgeSlotMtxArray[ii].Identity();
				m_aSystemData[i].paCartridgeSlotMtxArray[ii].m_vPos.Set( fPos, 0.07f*(f32)m_aSystemData[i].bDoubleStacked*(ii&1),0.0f);
				m_aSystemData[i].paCartridgeSlotMtxArray[ii].Mul( ScaleMtx );
			}
		}
	}
}


CWeaponBlaster::CWeaponBlaster() {
	_ClearDataMembers();
}


CWeaponBlaster::~CWeaponBlaster() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponBlaster::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	#if FANG_WINGC
		if( !_bUserPropsAvailable ) {
			DEVPRINTF( "CWeaponBlaster::Create(): Skipping '%s'; compatible blaster properties are unavailable.\n", pszEntityName ? pszEntityName : "<unnamed>" );
			return FALSE;
		}
	#endif

	// Get pointer to the leaf class's builder object...
	CWeaponBlasterBuilder *pBuilder = (CWeaponBlasterBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponBlasterBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONBLASTER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_BLASTER, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponBlaster::ClassHierarchyBuild( void ) 
{
	u32 i,j;
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
	CWeaponBlasterBuilder *pBuilder = (CWeaponBlasterBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<GetAvailableLevelCount(); ++i, ++pResourceData )
	{
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			if( (u32)m_nSingleMeshForEUKs >= GetAvailableLevelCount() ) goto _ExitWithError;
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		pInfo = &m_aaInfo[WEAPON_TYPE_BLASTER][i];

		// Load the mesh resource...
		pMesh = NULL;
		//if (!m_aUserProps[i].apszMeshName[_MESH_WEAPON] ||
		//	fclib_stricmp(m_aUserProps[i].apszMeshName[_MESH_WEAPON], "null"))
		//{

		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponBlaster::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );

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
		if( !CreateSharedEUKData( &MeshInit, &uResEUK, &(pResourceData->m_pWorldMesh), &(pResourceData->m_pAnimCombiner), &(pResourceData->m_pAnimInstRest) ) ) {
			DEVPRINTF( "CWeaponBlaster::ClassHierarchyBuild():  Error creating shared EUK data\n" );
			goto _ExitWithError;
		}

		if( uResEUK == i ) {
			pResourceData->m_pWorldMesh->RemoveFromWorld();
			pResourceData->m_pWorldMesh->SetCollisionFlag( TRUE );
			pResourceData->m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
			pResourceData->m_pWorldMesh->m_pUser = this;
			pResourceData->m_pWorldMesh->SetUserTypeBits( TypeBits() );
			pResourceData->m_pWorldMesh->SetLineOfSightFlag(FALSE); //pgm added this so that weapons never block los tests
		//}

			if( !pResourceData->m_pAnimCombiner->CreateSimple( NULL, pResourceData->m_pWorldMesh ) ) {
				DEVPRINTF( "CWeaponBlaster::Create(): Could not create animation combiner.\n" );
				goto _ExitWithError;
			}

			// catridge requires another bone anim callback...
			pResourceData->m_pAnimCombiner->SetBoneCallback( _CartridgeBoneCallback );
			pResourceData->m_pAnimCombiner->DisableAllBoneCallbacks();
			pResourceData->m_pAnimCombiner->EnableBoneCallback( m_aUserProps[i].apszBoneName[_BONE_CARTRIDGE] );
		}

		pResourceData->m_nCombinerTapID = pResourceData->m_pAnimCombiner->GetTapID( FANIM_SIMPLE_COMBINER_CONFIG_TAPNAME );
		if( pResourceData->m_nCombinerTapID == -1 ) {
			DEVPRINTF( "CWeaponBlaster::Create(): Could not obtain combiner tap ID.\n" );
			goto _ExitWithError;
		}

		// Find bone index for the primary fire 
		m_aSystemData[i].nPrimFireBoneIndex = pResourceData->m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE] );
		if( m_aSystemData[i].nPrimFireBoneIndex < 0 ) {
			DEVPRINTF( "CWeaponBlaster::Create(): Could not find bone '%s' on blaster launcher L%d.\n", m_aUserProps[uMeshEUK].apszBoneName[_BONE_PRIMARY_FIRE], i );
			goto _ExitWithError;
		}

		// Find bone index for the cartridge
		m_aSystemData[i].nCartridgeBoneIndex = pResourceData->m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].apszBoneName[_BONE_CARTRIDGE] );
		if( m_aSystemData[i].nCartridgeBoneIndex < 0 ) {
			DEVPRINTF( "CWeaponBlaster::Create(): Could not find bone '%s' on blaster launcher L%d.\n", m_aUserProps[uMeshEUK].apszBoneName[_BONE_CARTRIDGE], i );
			goto _ExitWithError;
		}

		// Find bone index for the barrel 
		m_aSystemData[i].nBarrelBoneIndex = pResourceData->m_pWorldMesh->FindBone( m_aUserProps[uMeshEUK].apszBoneName[_BONE_BARREL] );
		if( m_aSystemData[i].nBarrelBoneIndex < 0 ) {
			DEVPRINTF( "CWeaponBlaster::Create(): Could not find bone '%s' on blaster launcher L%d.\n", m_aUserProps[uMeshEUK].apszBoneName[_BONE_BARREL], i );
			goto _ExitWithError;
		}

		// shells on its clip
		// Create them
		FASSERT( pInfo->nClipAmmoMax );

		// Create an array of world meshes for the shells that stick to the cartridge
		if (m_aSystemData[i].bShowShellsOnCartridge)	//some euk Levels (3) don't have shells stuck to the barrel
		{
			pResourceData->m_paShellWorldMeshArray = fnew CFWorldMesh[ pInfo->nClipAmmoMax ];
			if( pResourceData->m_paShellWorldMeshArray == NULL ) {
				DEVPRINTF( "CWeaponBlaster::Create(): Not enough memory to create pResourceData->m_paShellWorldMeshArray.\n", i );
				goto _ExitWithError;
			}

			pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_SHELL] );
			if( pMesh == NULL ) {
				DEVPRINTF( "CWeaponBlaster::Create(): Could not find mesh '%s'.\n", m_aUserProps[uMeshEUK].apszMeshName[_MESH_SHELL] );

				pMesh = m_pErrorMesh;
				if( pMesh == NULL ) {
					goto _ExitWithError;
				}
			}

			MeshInit.pMesh = pMesh;
			MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
			MeshInit.fCullDist = m_aUserProps[uMeshEUK].fShellCullDist;
			MeshInit.Mtx.Identity();

			for( j=0; j<pInfo->nClipAmmoMax; ++j ) {
				// Init the world mesh...
				pResourceData->m_paShellWorldMeshArray[j].Init( &MeshInit );
				pResourceData->m_paShellWorldMeshArray[j].RemoveFromWorld();
				pResourceData->m_paShellWorldMeshArray[j].SetCollisionFlag( FALSE );
			}
		}
	}

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	fforce_NullHandle( &m_hForce );

	m_nBlasterState = BLASTERSTATE_NORMAL;
	m_fSecondsCountdownTimer = 0.0f;

	m_bFireThisFrame = FALSE;

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponBlaster::BeginReload( void )
{
	//someday, kick off reload state and do these when the reload anim is done

	m_bReloadRequested = TRUE;
}

	
BOOL CWeaponBlaster::ClassHierarchyBuilt( void ) {
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


void CWeaponBlaster::ClassHierarchyDestroy( void ) {
	u32 i;

	fforce_Kill( &m_hForce );

	// Destroy ourselves first...
	for( i=0; i<EUK_COUNT_BLASTER; ++i ) {
		DestroySharedEUKData( i );

		m_aResourceData[i].m_pWorldMesh = NULL;
		m_aResourceData[i].m_pAnimCombiner = NULL;
		fdelete_array( m_aResourceData[i].m_paShellWorldMeshArray );  m_aResourceData[i].m_paShellWorldMeshArray = NULL;
		m_aResourceData[i].m_pAnimInstRest = NULL;
	}

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();


}


CEntityBuilder *CWeaponBlaster::GetLeafClassBuilder( void ) {
	return &_WeaponBlasterBuilder;
}


void CWeaponBlaster::_ClearDataMembers( void ) {
	u32 i;

	for( i=0; i<EUK_COUNT_BLASTER; ++i ) {
		m_aResourceData[i].m_pWorldMesh = NULL;
		m_aResourceData[i].m_pAnimCombiner = NULL;
		m_aResourceData[i].m_nCombinerTapID = -1;
		m_aResourceData[i].m_paShellWorldMeshArray = NULL;
		m_aResourceData[i].m_pAnimInstRest = NULL;
	}

	m_pResourceData = NULL;
	m_pUserProps = NULL;
	m_pAnimInstCurrentlyPlaying = NULL;
	m_fAnimInstTimeMul = 1.0f;
	m_fSecondsCountdownTimer = 0.0f;
	m_fFireAgainTime = 0.0f;	 //delay for second barrel
	m_bReloadRequested = FALSE;
	m_fReloadCockTimerSecs = 0.f;

	m_nBlasterState = BLASTERSTATE_NORMAL;
	m_fSecondsCountdownTimer = 0.0f;

	m_bBoolBarrel2Fire = FALSE;
	m_bFireThisFrame = FALSE;
	m_vTargetPos_WS.Zero();
	m_pvFireBuddyPos_WS = NULL;
	m_fUnitTriggerVal1 = 0.0f;

	m_pParticleSmoke = NULL;
	m_fSmokeUnitIntensity = 0.0f;
}


void CWeaponBlaster::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponBlaster::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NUMERIC );
	}
}


void CWeaponBlaster::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pWorldMesh->UpdateTracker();
}


void CWeaponBlaster::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();

	if (m_pResourceData->m_paShellWorldMeshArray)
	{
		for( s32 i=0; i < m_pInfo->nClipAmmoMax; ++i )
		{
			m_pResourceData->m_paShellWorldMeshArray[i].RemoveFromWorld();
		}
	}

	// Remove smoke from world...
	_KillSmoke();
}


void CWeaponBlaster::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled )
{
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	u32 i;

	if( bDrawingHasBeenEnabled )
	{
		// Enable drawing of this weapon...

		for( i=0; i<EUK_COUNT_BLASTER; ++i )
		{
			if( m_aResourceData[i].m_pWorldMesh ) FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		if (m_pResourceData->m_paShellWorldMeshArray)
		{
			for( i=0; i<m_pInfo->nClipAmmoMax; ++i )
			{
				FMATH_CLEARBITMASK( m_pResourceData->m_paShellWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}
	}
	else
	{
		// Disable drawing of this weapon...
		for( i=0; i<EUK_COUNT_BLASTER; ++i )
		{
			if( m_aResourceData[i].m_pWorldMesh ) FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}

		if (m_pResourceData->m_paShellWorldMeshArray)
		{
			for( i=0; i<m_pInfo->nClipAmmoMax; ++i )
			{
				FMATH_SETBITMASK( m_pResourceData->m_paShellWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}

		_KillSmoke();
	}
}


void CWeaponBlaster::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();

		m_pCallbackThis = this;
		_ComputeMtxPalette();
		m_pCallbackThis = NULL;
	}
}


CFMtx43A *CWeaponBlaster::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ) {
	s32 nBoneIndex;

	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pResourceData->m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CWeaponBlaster::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CWeaponBlaster::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;
}

void CWeaponBlaster::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_fSecondsCountdownTimer = 0.0f;
	m_bReloadRequested = FALSE;
	m_nBlasterState = BLASTERSTATE_NORMAL;
	m_fCartridgeOffset = 0.0f;
}

// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponBlaster::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	u32 i;

	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_BLASTER][nPreviousUpgradeLevel];

	_KillSmoke();

	// Remove old mesh from world...
	pOldResourceData->m_pWorldMesh->RemoveFromWorld();

	if (pOldResourceData->m_paShellWorldMeshArray)	{
		for( i=0; i<pOldInfo->nClipAmmoMax; ++i ) {
			pOldResourceData->m_paShellWorldMeshArray[i].RemoveFromWorld();
		}
	}

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];
	m_pUserProps = &m_aUserProps[ m_nUpgradeLevel ];

	TransferFromReserveToClip( INFINITE_AMMO, FALSE );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}

	TransferFromReserveToClip( INFINITE_AMMO, FALSE );
}


void CWeaponBlaster::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	*pMuzzlePoint_WS = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_aSystemData[m_nUpgradeLevel].nPrimFireBoneIndex ]->m_vPos;
}


void CWeaponBlaster::_UpdateSmoke( void )
{
	if( !IsDrawEnabled() || (m_fSmokeUnitIntensity <= 0.0f) )
	{
		// Smoke should be off...

		_KillSmoke();
	}
	else
	{
		if( m_pUserProps->hParticleSmoke )
		{
			// Smoke should be on...

			f32 fUnitIntensity = m_fSmokeUnitIntensity * m_pUserProps->fSmokeUnitIntensity;

			if( m_pParticleSmoke )
			{
				// Smoke is already on. Update intensity...
				m_pParticleSmoke->SetUnitIntensity( fUnitIntensity );
			}
			else
			{
				// Smoke is not on yet. Turn it on...
				m_pParticleSmoke = eparticlepool_GetEmitter();

				if( m_pParticleSmoke )
				{
					const CFMtx43A *pBoneMtx_WS;
					CFMtx43A Mtx;

					pBoneMtx_WS = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_aSystemData[m_nUpgradeLevel].nPrimFireBoneIndex ];

					m_pParticleSmoke->StartEmission( m_pUserProps->hParticleSmoke, fUnitIntensity );

					m_pParticleSmoke->Relocate_RotXlatFromScaledMtx_WS_NewScale_WS( pBoneMtx_WS, 0.0f, 1.0f, FALSE );
					m_pParticleSmoke->Attach_ToParent_WS( this, m_pUserProps->apszBoneName[_BONE_PRIMARY_FIRE], FALSE );
				}
			}
		}
	}
}


void CWeaponBlaster::_KillSmoke( void )
{
	if( m_pParticleSmoke && m_pParticleSmoke->IsCreated() )
	{
		m_pParticleSmoke->DetachFromParent();
		m_pParticleSmoke->StopEmission();
		m_pParticleSmoke = NULL;
	}
}


void CWeaponBlaster::_ScatterBlast(const CFVec3A& BlastOrigin, const CFVec3A& BlastDirUnit, u32 uNumPellets, f32 fJitterFactor)
{
	CEntity *pHitEntity;
	const CGCollMaterial *pCollMaterial;
	FCollImpact_t CollImpact;
	CFVec3A PelletUnitDir_WS, RayEnd_WS, ImpactFlashPos_WS;

	// Build tracker skip list...
	FWorld_nTrackerSkipListCount = 0;
	if( m_pOwnerBot )
	{
		m_pOwnerBot->AppendTrackerSkipList();
	}

	for (u32 i = 0; i < uNumPellets; i++)
	{
		//compute the pellet direction...
		fmath_ScatterUnitVec( &PelletUnitDir_WS, &BlastDirUnit, fJitterFactor );

		fmath_RandomInt32();

		//compute the pellet extent point
		RayEnd_WS.Mul( PelletUnitDir_WS, m_pUserProps->fMaxLiveRange ).Add( BlastOrigin );

		if (fworld_FindClosestImpactPointToRayStart( &CollImpact, &BlastOrigin, &RayEnd_WS, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, m_pResourceData->m_pWorldMesh, -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ))
		{
			//extract the material we hit
			pCollMaterial = CGColl::GetMaterial( &CollImpact );

			//draw an impact flash
			if( pCollMaterial->IsImpactFlashEnabled() )
			{
				ImpactFlashPos_WS.Mul( CollImpact.UnitFaceNormal, m_pUserProps->fMuzzleOffset_Impact ).Add( CollImpact.ImpactPoint );

				CMuzzleFlash::AddFlash_CardPosterXY(
					m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BALL_POSTER_XY_1],
					ImpactFlashPos_WS,
					m_pUserProps->fMuzzleScale_Impact,
					m_pUserProps->fMuzzleAlpha_Impact
				);
			}

			CFDecal::Create( m_pUserProps->hDecalDef, &CollImpact, &PelletUnitDir_WS );
			////make pockmark
			//if( pCollMaterial->IsPockMarkEnabled( &CollImpact ) )
			//{
			//	potmark_NewPotmark( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.4f, POTMARKTYPE_BULLET );
			//}

			//draw effects
#if 0
			pCollMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_SPARKS_BURST, &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.1f );
			pCollMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_BITS, &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.1f );
			pCollMaterial->DrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST, &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 0.1f );
#else
			pCollMaterial->DrawAllDrawableParticles( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, FALSE, 0.1f, 0.1f, 0.1f );
#endif

			pHitEntity = CGColl::ExtractEntity( &CollImpact );
			if( pHitEntity )
			{
				// Get an empty damage form...
				CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

				if( pDamageForm )
				{
					// Fill out the form...

					pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
					pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
					pDamageForm->m_pDamageProfile = m_pUserProps->pDamageProfile;
					pDamageForm->m_Damager.pWeapon = this;
					pDamageForm->m_Damager.pBot = GetOwner();
					pDamageForm->m_Damager.nDamagerPlayerIndex = GetOwner() ? GetOwner()->m_nPossessionPlayerIndex : -1;
					pDamageForm->m_pDamageeEntity = pHitEntity;
					pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh *)CollImpact.pTag, &CollImpact, &PelletUnitDir_WS );

					CDamage::SubmitDamageForm( pDamageForm );
				}
			}

			//make an AI impact sound in case any AI are listening
			if( GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1 && m_fSecsUntilNextSound <= 0.0f)
			{
				AIEnviro_AddSound( CollImpact.ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, this );
				m_fSecsUntilNextSound = 0.5f;
			}
		}
	}
}


u32 CWeaponBlaster::_L1_TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPoint_WS, const CFVec3A *pBuddyFirePos_WS ) 
{
	if( fUnitTriggerVal1 < 0.25f ) {
		// Trigger not down...

		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

		return 0;
	}

	if( m_nClipAmmo == 0 ) {
		// No rounds in clip...

		if( m_nReserveAmmo == 0 ) {
			// Completely out of ammo. Make empty click sound...

			if( !(m_nWeaponFlags & WEAPONFLAG_MADE_EMPTY_CLICK_SOUND) ) {
				FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

				PlaySound( m_pUserProps->pSoundGroupEmpty );
			}
		}

		return 0;
	}

	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		return 0;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return 0;
	}

#if 0
	m_vTargetPos_WS.Set( *pvTargetPoint_WS );
	if ( m_pOwnerBot && m_pOwnerBot->m_nPossessionPlayerIndex >= 0 )
	{
		m_pOwnerBot->FocusHumanTargetPoint_WS( &m_vTargetPos_WS, this );
	}
#else
	m_vTargetPos_WS = *pvTargetPoint_WS;
#endif

	m_bFireThisFrame = TRUE;

	return 1;
}

void CWeaponBlaster::_L1_Fire( const CFVec3A &vUnitFireDir ) {
	CFVec3A BlastOrigin;

	ComputeMuzzlePoint_WS( &BlastOrigin );

	_DrawMuzzleEffects( &BlastOrigin, &vUnitFireDir, FALSE );
	_ScatterBlast(BlastOrigin, vUnitFireDir, m_pUserProps->nPelletCountPerRound, m_pUserProps->fShotSpreadFactor);

	if(m_pvFireBuddyPos_WS != NULL)
	{
		_ScatterBlast(*m_pvFireBuddyPos_WS, vUnitFireDir, m_pUserProps->nPelletCountPerRound, m_pUserProps->fShotSpreadFactor);
		_DrawMuzzleEffects( m_pvFireBuddyPos_WS, &vUnitFireDir, TRUE );
	}
												 
#if 0
	CFVec3A BlastWing;
	CFVec3A DebrisVelocity;
	BlastWing = MtxToWorld()->m_vRight;
	BlastWing.Mul(fmath_RandomFloatRange( 3.0f, 5.0f ));
	DebrisVelocity = CFVec3A::m_UnitAxisY;
	DebrisVelocity.Mul(fmath_RandomFloatRange( 8.0f, 10.0f ));
	DebrisVelocity.Add(BlastWing);
	DebrisVelocity.Add(m_pOwnerBot->m_Velocity_WS);

	DebrisInitSettings_t _debriSettings;
	_debriSettings.bSampleDynamicLights = TRUE;
	_debriSettings.fMinRotVel = 0.65f;
	_debriSettings.fMaxRotVel = 1.15f;
	_debriSettings.fCullDist2 = (85.0f * 85.0f);
	_debriSettings.fLinearFrictionPerSec = 40.0f;
	_debriSettings.fRotFrictionPerSec = 20.0f;
	_debriSettings.fGravityPerSec = 40.0f;
	_debriSettings.fShrinkPerSec = 0.75f;
	_debriSettings.pStateChangeCBFunc = NULL;

	if (m_pResourceData->m_paShellWorldMeshArray)
	{
		CDebris::CreateDebris(&(MtxToWorld()->m_vPos), &DebrisVelocity, &(m_pResourceData->m_paShellWorldMeshArray[0].m_pMesh), 1, 1.0f, &_debriSettings);
	}
#endif

	// Set timer until the next round is ready...
	m_fSecondsCountdownTimer = m_pUserProps->fOORoundsPerSec;

	if( IsOwnedByPlayer() ) {
		// Human player...
		fforce_Kill( &m_hForce );
		fforce_Play( Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE, &m_hForce );

		CFCamera *pCamera = fcamera_GetCameraByIndex( GetOwner()->m_nPossessionPlayerIndex );
		pCamera->ShakeCamera( m_pUserProps->fUnitCamVibration, 0.05f );
	}

	PlaySound( m_pUserProps->pSoundGroupFire );

	// Remove the round we fired from the clip...
	_ConsumeFiredRound();
}


u32 CWeaponBlaster::_L23_TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPoint_WS, const CFVec3A *pBuddyFirePos_WS ) {

	m_bBoolBarrel2Fire = FALSE;
	f32 fNowTime = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;
	if (m_fFireAgainTime != 0.0f && 
		m_fFireAgainTime < fNowTime)
	{
		m_bBoolBarrel2Fire = TRUE;
		m_fFireAgainTime = 0.0f;
	}

	if( !m_bBoolBarrel2Fire && (fUnitTriggerVal1 < 0.25f) ) {
		// Trigger not down...

		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

		return 0;
	}

	if( m_nClipAmmo == 0 ) {
		// No rounds in clip...

		if( m_nReserveAmmo == 0 ) {
			// Completely out of ammo. Make empty click sound...

			if( !(m_nWeaponFlags & WEAPONFLAG_MADE_EMPTY_CLICK_SOUND) ) {
				FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );

				PlaySound( m_pUserProps->pSoundGroupEmpty );
			}
		}

		return 0;
	}

	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		return 0;
	}

	if( !m_bBoolBarrel2Fire && m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return 0;
	}

	m_vTargetPos_WS = *pvTargetPoint_WS;
	m_bFireThisFrame = TRUE;

	return 1;
}


void CWeaponBlaster::_L23_Fire( const CFVec3A &vUnitFireDir ) {
	CFVec3A BlastWing, BlastOrigin, BlastDir, DebrisVelocity;

	ComputeMuzzlePoint_WS( &BlastOrigin );

#if 0
	DebrisInitSettings_t _debriSettings;
	_debriSettings.bSampleDynamicLights = TRUE;
	_debriSettings.fMinRotVel = 0.65f;
	_debriSettings.fMaxRotVel = 1.15f;
	_debriSettings.fCullDist2 = (85.0f * 85.0f);
	_debriSettings.fLinearFrictionPerSec = 40.0f;
	_debriSettings.fRotFrictionPerSec = 20.0f;
	_debriSettings.fGravityPerSec = 40.0f;
	_debriSettings.fShrinkPerSec = 0.75f;
	_debriSettings.pStateChangeCBFunc = NULL;
#endif

	if (!m_bBoolBarrel2Fire)
	{
		_ScatterBlast(BlastOrigin, vUnitFireDir, m_pUserProps->nPelletCountPerRound, m_pUserProps->fShotSpreadFactor);
		_DrawMuzzleEffects( &BlastOrigin, &vUnitFireDir, FALSE );

		if(m_pvFireBuddyPos_WS)
		{
			_ScatterBlast(*m_pvFireBuddyPos_WS, vUnitFireDir, m_pUserProps->nPelletCountPerRound >> 1, m_pUserProps->fShotSpreadFactor);
			_DrawMuzzleEffects( m_pvFireBuddyPos_WS, &vUnitFireDir, TRUE );
		}
		m_fFireAgainTime = (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick)+m_pUserProps->fBarrel2DelayTime;

		//kick out a shell
		BlastWing = MtxToWorld()->m_vRight;
		BlastWing.Mul(fmath_RandomFloatRange( 2.0f, 4.0f ));
		DebrisVelocity = CFVec3A::m_UnitAxisY;
		DebrisVelocity.Mul(fmath_RandomFloatRange( 8.0f, 10.0f ));
		DebrisVelocity.Add(BlastWing);
		DebrisVelocity.Add(m_pOwnerBot->m_Velocity_WS);

#if 0
		if (m_aResourceData[0].m_paShellWorldMeshArray)
		{
			CDebris::CreateDebris(&(MtxToWorld()->m_vPos), &DebrisVelocity, &(m_aResourceData[0].m_paShellWorldMeshArray[0].m_pMesh), 1, 1.0f, &_debriSettings);
		}
#endif
	}
	else
	{
		_ScatterBlast(BlastOrigin, vUnitFireDir, m_pUserProps->nPelletCountPerRound, m_pUserProps->fShotSpreadFactor);
		_DrawMuzzleEffects( &BlastOrigin, &vUnitFireDir, FALSE );

		if(m_pvFireBuddyPos_WS)
		{
			_ScatterBlast(*m_pvFireBuddyPos_WS, vUnitFireDir, m_pUserProps->nPelletCountPerRound >> 1, m_pUserProps->fShotSpreadFactor);
			_DrawMuzzleEffects( m_pvFireBuddyPos_WS, &vUnitFireDir, TRUE );
		}

		//debris
		BlastWing = MtxToWorld()->m_vRight;
		BlastWing.Mul(fmath_RandomFloatRange( 3.0f, 5.0f ));
		DebrisVelocity = CFVec3A::m_UnitAxisY;
		DebrisVelocity.Mul(fmath_RandomFloatRange( 7.0f, 9.0f ));
		DebrisVelocity.Add(BlastWing);
		DebrisVelocity.Add(m_pOwnerBot->m_Velocity_WS);

#if 0
		if (m_aResourceData[0].m_paShellWorldMeshArray)
		{
			CDebris::CreateDebris(&(MtxToWorld()->m_vPos), &DebrisVelocity, &(m_aResourceData[0].m_paShellWorldMeshArray[0].m_pMesh), 1, 1.0f, &_debriSettings);
		}
#endif
	}

	m_fSecondsCountdownTimer = m_pUserProps->fOORoundsPerSec;

	if( IsOwnedByPlayer() ) {
		// Human player...
		fforce_Kill( &m_hForce );
		fforce_Play( Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_ROUGH_RUMBLE, &m_hForce );

		CFCamera *pCamera = fcamera_GetCameraByIndex( GetOwner()->m_nPossessionPlayerIndex );
		pCamera->ShakeCamera( m_pUserProps->fUnitCamVibration, 0.05f );
	}

	PlaySound( m_pUserProps->pSoundGroupFire );

	// Remove the round we fired from the clip...
	_ConsumeFiredRound();
}



// Shell meshes represent remaining rounds, not shots fired. Infinite ammo leaves
// the clip unchanged; indexing that full count would access past the shell array.
void CWeaponBlaster::_ConsumeFiredRound( void ) {
	u16 nBefore = GetClipAmmo();
	RemoveFromClip( 1 );
	u16 nAfter = GetClipAmmo();
	if( m_pResourceData->m_paShellWorldMeshArray && nAfter < nBefore && nAfter < m_pInfo->nClipAmmoMax ) {
		m_pResourceData->m_paShellWorldMeshArray[nAfter].RemoveFromWorld();
	}
}

void CWeaponBlaster::_DrawMuzzleEffects( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS, BOOL bBuddy )
{
	f32 fRandomScale = fmath_RandomFloatRange( 0.8f, 1.0f );

	CMuzzleFlash::AddFlash_Card3D(
		m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_BLASTER],
		*pPos_WS,
		4.0f,
		3.0f,
		fmath_RandomFloatRange( 0.0f, FMATH_DEG2RAD( 360.0f ) )
	);

	CMuzzleFlash::AddFlash_CardPosterZ(
		m_ahMuzzleFlashGroup[ MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1 + fmath_RandomRange( 0, 1 ) ],
		*pPos_WS,
		*pUnitDir_WS,
		m_pUserProps->fMuzzleWidth_Z * fRandomScale,
		m_pUserProps->fMuzzleHeight_Z * fRandomScale,
		m_pUserProps->fMuzzleAlpha_Z
	);

#if 0
	// Make a small smoke trail...
	smoketrail_SpawnMuzzlePuff(
		&m_SmokeTrailAttrib,
		pPos_WS,
		pUnitDir_WS,
		3.0f
	);
#endif

	if( !bBuddy ) {
		f32 fMuzzleLightRadius = IsOwnedByPlayer() ? 20.0f : 15.0f;

		// Spawn muzzle light...
		CMuzzleFlash::MuzzleLight( pPos_WS, fMuzzleLightRadius, fmath_RandomFloatRange( 0.5f, 1.0f ) );
	}
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
u32 CWeaponBlaster::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPoint_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );

	m_bFireThisFrame = FALSE;
	m_fUnitTriggerVal1 = fUnitTriggerVal1;
	m_pvFireBuddyPos_WS = pBuddyFirePos_WS;

	if( m_nUpgradeLevel == 0 ) {
		// EUK 1...
		return _L1_TriggerWork( fUnitTriggerVal1, fUnitTriggerVal2, pvTargetPoint_WS, pBuddyFirePos_WS );
	} else {
		// All other versions...
		return _L23_TriggerWork( fUnitTriggerVal1, fUnitTriggerVal2, pvTargetPoint_WS, pBuddyFirePos_WS );
	}
}


// Called once per frame to update animations, timers, etc.
void CWeaponBlaster::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	f32 fUnit, fVal;
	BOOL bReloadComplete;

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( m_bFireThisFrame ) {
		CFVec3A vFireUnitDir;
		CFVec3A vMuzzlePt;

		ComputeMuzzlePoint_WS( &vMuzzlePt );
		ComputeFireUnitDir( &vFireUnitDir, &vMuzzlePt, &m_MtxToWorld.m_vFront, &m_vTargetPos_WS, FALSE );

		if( m_nUpgradeLevel == 0 ) {
			// EUK 1...
			_L1_Fire( vFireUnitDir );
		} else {
			// All other versions...
			_L23_Fire( vFireUnitDir );
		}

		m_fSmokeUnitIntensity = 1.0f;

		FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND );
	} else {
		if( m_fSmokeUnitIntensity > 0.0f ) {
			m_fSmokeUnitIntensity -= m_pUserProps->fSmokeCoolDownSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fSmokeUnitIntensity, 0.0f );
		}
	}

	_UpdateSmoke();

	if( m_fSecondsCountdownTimer > 0.0f ) {
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fSecondsCountdownTimer, 0.0f );
	}

	#define __RELOAD_RECOVERY_UNIT_TIME		0.2f
	#define __SWINGDOWN_TIME				5.0f
	#define __SWINGUP_TIME					0.5f
	#define __SWINGDOWN_BOUNCE_UNIT_TIME	0.15f

	switch (m_nBlasterState)
	{
		case BLASTERSTATE_NORMAL:

			if ( m_fReloadCockTimerSecs != 0.f ) {
				m_fReloadCockTimerSecs -= FLoop_fPreviousLoopSecs;
				if ( m_fReloadCockTimerSecs <= 0.f ) {
					// If timer is expired, sound the cock
					m_fReloadCockTimerSecs = 0.f;
					PlaySound( m_pUserProps->pSoundGroupCock );
				}
			}

			if ( m_bReloadRequested  )
			{
				m_bReloadRequested = FALSE;

				// Make sure it can be reloaded, just in case...
				if( CanClipBeReloaded() ) {
					m_nBlasterState = BLASTERSTATE_RELOADING;

					m_fSecondsCountdownTimer = m_pUserProps->fReloadTimeForOneRound;

					// Create sound for the rounds loading
					PlaySound( m_pUserProps->pSoundGroupRound );

					m_fCartridgeOffsetStart = 0;
					m_fCartridgeOffset = 0;
					m_fCartridgeOffsetEnd = m_aSystemData[m_nUpgradeLevel].fShellSlotDist;
				}
			}
			break;
		case BLASTERSTATE_RELOADING:

			bReloadComplete = FALSE;

			m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
			if( m_fSecondsCountdownTimer <= 0.0f ) {
				// We're done reloading...

				m_fSecondsCountdownTimer = 0.0f;
				m_nBlasterState = BLASTERSTATE_NORMAL;
				TransferFromReserveToClip((u16) (1+m_aSystemData[m_nUpgradeLevel].bDoubleStacked));

				// Start timer for cock sound
				if ( !CanClipBeReloaded() ){
					m_fReloadCockTimerSecs = 0.06f;
				}

				ReloadComplete();

				bReloadComplete = TRUE;
			}

			fUnit = 1.0f - m_fSecondsCountdownTimer * m_aSystemData[m_nUpgradeLevel].fOOReloadTimeForOneRound;

			if( fUnit < (1.0f - __RELOAD_RECOVERY_UNIT_TIME) ) {
				fUnit *= (1.0f / (1.0f - __RELOAD_RECOVERY_UNIT_TIME));

				// Make the cool reload rotation motion...
				fVal = 2.5f * fmath_Sin( FMATH_HALF_PI * fUnit );
				fVal = FMATH_FPOT( fUnit, fVal, fUnit );
				fVal *= fVal;

				m_fCartridgeOffset = m_fCartridgeOffsetStart + fVal * (m_fCartridgeOffsetEnd - m_fCartridgeOffsetStart);
			}

			if( bReloadComplete ) {
				// Reset firing timer...
				m_fSecondsCountdownTimer = 0.5f * m_pUserProps->fOORoundsPerSec;
			}

			break;
	};
}


void CWeaponBlaster::_ComputeMtxPalette( void )
{
	m_pResourceData->m_pAnimCombiner->ComputeMtxPalette( FALSE );

	if (m_aSystemData[m_nUpgradeLevel].nBarrelBoneIndex > -1)
	{
//		CFMtx43A *pBarrelBoneMtx = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_aSystemData[m_nUpgradeLevel].nBarrelBoneIndex];

		//do barrell recoil
	}

	// Move  any shells attached to the cartridge
	if (m_aSystemData[m_nUpgradeLevel].bShowShellsOnCartridge)
	{	   //level 3 draws no shells
		CFMtx43A *pCartridgeBoneMtx = m_pResourceData->m_pWorldMesh->GetBoneMtxPalette()[ m_aSystemData[m_nUpgradeLevel].nCartridgeBoneIndex];
		CFMtx43A ShellMtx;
		CFMtx43A LoadMtx;
		for( s32 i=0; i<GetClipAmmo(); ++i )
		{
			LoadMtx = m_aSystemData[m_nUpgradeLevel].paCartridgeSlotMtxArray[i];

			ShellMtx.Mul( *pCartridgeBoneMtx, LoadMtx );
			m_pResourceData->m_paShellWorldMeshArray[i].m_Xfm.BuildFromMtx( ShellMtx );
			m_pResourceData->m_paShellWorldMeshArray[i].UpdateTracker();
		}
	}

	RelocateAllChildren();
}


void CWeaponBlaster::_CartridgeBoneCallback( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	rNewMtx.Mul( rParentMtx, rBoneMtx );

	CFMtx43A tmp;
	tmp.Identity();
	f32 fPos = m_aUserProps[m_pCallbackThis->m_nUpgradeLevel].fCartridgeXPos + -(1.0f - (f32)m_pCallbackThis->GetClipAmmo() / m_pCallbackThis->GetMaxClipAmmo())*m_aUserProps[m_pCallbackThis->m_nUpgradeLevel].fCartridgeXRange;
	if (m_pCallbackThis->m_nBlasterState == BLASTERSTATE_RELOADING)
	{
		fPos += m_pCallbackThis->m_fCartridgeOffset;
	}
	tmp.m_vPos.Set( fPos, 0.0f, 0.0f );
	rNewMtx.Mul(tmp);
}



