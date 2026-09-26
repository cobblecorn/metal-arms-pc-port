//////////////////////////////////////////////////////////////////////////////////////
// weapon_wrench.cpp -
//
// Author: Nathan Miller     
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
// 03/19/03 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "iteminst.h"
#include "botglitch.h"
#include "weapon_wrench.h"
#include "fsound.h"

#define _USER_PROP_FILENAME		"w_wrench.csv"

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponWrenchBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponWrenchBuilder _WeaponWrenchBuilder;


void CWeaponWrenchBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_UNSPECIFIED, pszEntityType );
}


BOOL CWeaponWrenchBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponWrenchBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponWrench
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CWeaponWrench::m_bSystemInitialized = FALSE;
CWeaponWrench::_UserProps_t CWeaponWrench::m_UserProps;

const FGameData_TableEntry_t CWeaponWrench::m_aUserPropVocab[] = {
	// pszMeshName
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
	FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_1000,

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupUse

	// fTimeInPieces
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_65534,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponWrench::m_aUserPropMapTable[] = {
	"Info",
	m_aUserPropVocab,
	sizeof(m_UserProps),
	(void *)&m_UserProps,

	NULL
};



BOOL CWeaponWrench::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );

	FResFrame_t ResFrame = fres_GetFrame();

	m_bSystemInitialized = TRUE;

	fang_MemZero( &m_UserProps, sizeof( m_UserProps ) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponWrench::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	Info_t *pInfo;

	pInfo = &m_aaInfo[WEAPON_TYPE_WRENCH][0];

	pInfo->nGrip = GRIP_LEFT_ARM;
	pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
	pInfo->nStanceType = STANCE_TYPE_STANDARD;
	pInfo->fMinTargetAssistDist = 0.0f;
	pInfo->fMaxTargetAssistDist = 0.0f;
	pInfo->fMaxLiveRange = 0.0f;
	pInfo->nClipAmmoMax = ( m_UserProps.fClipAmmoMax ) >= 0.0f ? (u16) m_UserProps.fClipAmmoMax : INFINITE_AMMO;
	pInfo->nReserveAmmoMax = 0;
	pInfo->nInfoFlags = INFOFLAG_THROWABLE;
	pInfo->nReticleType = CReticle::TYPE_NONE;

	return TRUE;

	// Error...
_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CWeaponWrench::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		m_bSystemInitialized = FALSE;
	}
}


CWeaponWrench::CWeaponWrench() {
	_ClearDataMembers();
}


CWeaponWrench::~CWeaponWrench() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponWrench::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponWrenchBuilder *pBuilder = (CWeaponWrenchBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponWrenchBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONWRENCH );
	}

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_WRENCH, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponWrench::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponWrenchBuilder *pBuilder = (CWeaponWrenchBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponWrench::ClassHierarchyBuilt( void ) {
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


void CWeaponWrench::ClassHierarchyDestroy( void ) {
	// Destroy ourselves first...
	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponWrench::GetLeafClassBuilder( void ) {
	return &_WeaponWrenchBuilder;
}


void CWeaponWrench::_ClearDataMembers( void ) {
	m_fSecondsCountdownTimer = 0.0f;
}

void CWeaponWrench::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo );

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NONE );
	}
}


BOOL CWeaponWrench::Throwable_TriggerWork( f32 fUnitTriggerVal ) {
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

	if( m_pOwnerBot == NULL ) {
		return FALSE;
	}

	if( !( m_pOwnerBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
		return FALSE;
	}

	CBotGlitch *pGlitch = (CBotGlitch *) m_pOwnerBot;

	// If already in pieces, do nothing
//	if( pGlitch->IsInPieces() ) {
//		return FALSE;
//	}
	
	pGlitch->BreakIntoPieces( m_UserProps.fTimeInPieces );

	m_fSecondsCountdownTimer = m_UserProps.fRoundsPerSec;

	PlaySound( m_UserProps.pSoundGroupUse );

	// !!Nate - Temp 
	//RemoveFromClip( 1 );

	return TRUE;
}

// Called once per frame to update animations, timers, etc.
void CWeaponWrench::ClassHierarchyWork( void ) {
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