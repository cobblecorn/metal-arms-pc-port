//////////////////////////////////////////////////////////////////////////////////////
// Damage.cpp - Damage module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/06/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "damage.h"
#include "flinklist.h"
#include "fres.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fgamedata.h"
#include "meshtypes.h"
#include "entity.h"
#include "bot.h"
#include "weapon.h"
#include "gstring.h"
#include "ai\aienviro.h"
#include "fclib.h"
#include "MultiplayerMgr.h"
#include "Difficulty.h"


#define _DAMAGE_CSV_FILENAME		"damage.csv"	// Holds all damage profiles
#define _ARMOR_CSV_FILENAME			"armor.csv"		// Holds all armor profiles

#define _DAMAGE_DATA_POOL_COUNT		40


static CDamageData _DamageData;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamage
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CDamage::m_bSystemInitialized;
BOOL CDamage::m_bBusyProcessingInstantForms;
CDamageForm *CDamage::m_pPoolFreeDamageForm;
FLinkRoot_t CDamage::m_RootFreeDamageForm;
FLinkRoot_t CDamage::m_RootQueuedDamageForm;
FLinkRoot_t CDamage::m_RootInstantDamageForm;

CDamageProfile *CDamage::m_pDamageProfileArray;
CArmorProfile *CDamage::m_pArmorProfileArray;
u32 CDamage::m_nDamageProfileCount;
u32 CDamage::m_nArmorProfileCount;
BOOL CDamage::m_bDamageDataSwapped;

CDamageProfile CDamage::m_DamageProfileNone;
CArmorProfile CDamage::m_ArmorProfileNone;
CArmorProfile CDamage::m_ArmorProfileInfinite;

LastEntityDamage_t CDamage::m_LastEntityDamaged = {
	NULL,
	DAMAGE_HITPOINT_TYPE_PROJECTILE,
	NULL,
	0,
	0xffffffff
};

CFWorldTracker**	CDamageForm::m_apEntityDamageQueue;
u32					CDamageForm::m_uEntityDamageQueueCount;

const FGameData_TableEntry_t CDamage::m_aVocab_DamageProfile[] = {
	// m_pszDamageCause:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// m_fDamagePeriod:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_afUnitHitpoints[DAMAGE_HITPOINT_TYPE_PROJECTILE]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_afUnitHitpoints[DAMAGE_HITPOINT_TYPE_BLAST]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_afUnitHitpoints[DAMAGE_HITPOINT_TYPE_FLAME]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_aHitpointRange::m_fInnerRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_HitpointRange::m_fInnerValue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_UNCLAMPED_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// m_HitpointRange::m_fOuterRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_HitpointRange::m_fOuterValue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_UNCLAMPED_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// m_ImpulseRange::m_fInnerRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_ImpulseRange::m_fInnerValue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_Neg1000000000,
	F32_DATATABLE_1000000000,

	// m_ImpulseRange::m_fOuterRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_ImpulseRange::m_fOuterValue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_Neg1000000000,
	F32_DATATABLE_1000000000,

	// m_RumbleRange::m_fInnerRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_RumbleRange::m_fInnerValue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_RumbleRange::m_fOuterRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_RumbleRange::m_fOuterValue:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_fRumbleSecs:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// m_fAIAudibleRadius:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1000000000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameData_TableEntry_t CDamage::m_aVocab_ArmorProfile[] = {
	// m_afUnitProtection[DAMAGE_HITPOINT_TYPE_PROJECTILE]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_afUnitProtection[DAMAGE_HITPOINT_TYPE_BLAST]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_afUnitProtection[DAMAGE_HITPOINT_TYPE_FLAME]:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_fUnitProtectionFromImpulse:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_fUnitProtectionFromRumble:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// m_fMinBlastHitpointFilter:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};




BOOL CDamage::InitSystem( void ) {
	u32 i;

	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;
	m_bBusyProcessingInstantForms = FALSE;

	FResFrame_t ResFrame = fres_GetFrame();

	// Init several useful profiles...
	fang_MemZero( &m_DamageProfileNone, sizeof(m_DamageProfileNone) );
	m_DamageProfileNone.m_pszName = "None";
	m_DamageProfileNone.m_nDamageCause = DAMAGE_CAUSE_IMPACT;
	m_DamageProfileNone.m_abZeroInnerAndOuterValues[0] = TRUE;
	m_DamageProfileNone.m_abZeroInnerAndOuterValues[1] = TRUE;
	m_DamageProfileNone.m_abZeroInnerAndOuterValues[2] = TRUE;

	fang_MemZero( &m_ArmorProfileNone, sizeof(m_ArmorProfileNone) );
	m_ArmorProfileNone.m_pszName = "None";

	fang_MemZero( &m_ArmorProfileInfinite, sizeof(m_ArmorProfileInfinite) );
	m_ArmorProfileInfinite.m_pszName = "Infinite";
	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		m_ArmorProfileInfinite.m_afUnitProtection[i] = 1.0f;
	}
	m_ArmorProfileInfinite.m_fUnitProtectionFromImpulse = 1.0f;
	m_ArmorProfileInfinite.m_fUnitProtectionFromRumble = 1.0f;

	// Build our pools...
	m_pPoolFreeDamageForm = fnew CDamageForm[_DAMAGE_DATA_POOL_COUNT];
	if( m_pPoolFreeDamageForm == NULL ) {
		DEVPRINTF( "CDamage::InitSystem(): Not enough memory to allocate pool of CDamageForm objects.\n" );
		goto _ExitWithError;
	}

	// Init our pools...
	_InitPools();

	CDamageForm::m_apEntityDamageQueue = (CFWorldTracker**)fres_AllocAndZero(sizeof(CFWorldTracker*) * _DAMAGE_ENTITY_POOL_COUNT);
	if( CDamageForm::m_apEntityDamageQueue == NULL ) {
		DEVPRINTF( "CDamage::InitSystem(): Not enough memory to allocate pool of CFWorldTracker* objects.\n" );
		goto _ExitWithError;
	}
	CDamageForm::m_uEntityDamageQueueCount = 0; // means there's no entities in the damage queue

	// Load damage and armor data...
	if( !_LoadDamageAndArmorData() ) {
		// Error message has already been printed...
		goto _ExitWithError;
	}

	fgamedata_SetDamageConverterCallback( _GameDataDamageConverterCallback );
	fgamedata_SetArmorConverterCallback( _GameDataArmorConverterCallback );

	return TRUE;

	// Error:
_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CDamage::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		fdelete_array( m_pPoolFreeDamageForm );
		m_pPoolFreeDamageForm = NULL;

		fdelete_array( m_pDamageProfileArray );
		m_pDamageProfileArray = NULL;
		m_nDamageProfileCount = 0;
		m_bDamageDataSwapped = FALSE;

		fdelete_array( m_pArmorProfileArray );
		m_pArmorProfileArray = NULL;
		m_nArmorProfileCount = 0;

		m_bSystemInitialized = FALSE;
	}
}


void CDamage::SwapMultiplayerData( BOOL bIsMultiPlayer )
{
	// If the current swap state matches what we want, we don't need to
	// do anything.
	bIsMultiPlayer = !!bIsMultiPlayer;	// just in case...
	if ( bIsMultiPlayer == m_bDamageDataSwapped )
		return;

	// Flip the swapped switch
	m_bDamageDataSwapped = !m_bDamageDataSwapped;

	// OK, we need to swap data. Since most entries probably will not have a
	// multiplayer version, we find each MP entry and look for the corresponding
	// non-MP entry to swap them.
	u32 i;

	// Damage first...
	for ( i = 0; i < m_nDamageProfileCount; i++ ) {

		cchar* pszMPName = m_pDamageProfileArray[i].m_pszName;
		if (fclib_strlen( pszMPName ) < 4)
			continue;

		// If the name begins with MP_ then it is a multiplayer profile
		if ( (fclib_toupper(pszMPName[0]) == 'M') && (fclib_toupper(pszMPName[1]) == 'P') && (pszMPName[2] == '_') ) {

			// Found a MP entry. Look for the corresponding non-MP entry
			cchar* pszNonMPName = &pszMPName[3];
			for ( u32 j = 0; j < m_nDamageProfileCount; j++ ) {
				if ( !fclib_stricmp( m_pDamageProfileArray[j].m_pszName, pszNonMPName ) ) {
					// Found it! Now swap the two entries
					CDamageProfile tmp = m_pDamageProfileArray[i];
					m_pDamageProfileArray[i] = m_pDamageProfileArray[j];
					m_pDamageProfileArray[j] = tmp;

					// Now switch the names back, to avoid finding this entry again
					cchar* pcTmp = m_pDamageProfileArray[i].m_pszName;
					m_pDamageProfileArray[i].m_pszName = m_pDamageProfileArray[j].m_pszName;
					m_pDamageProfileArray[j].m_pszName = pcTmp;

					break;
				}
			}
		}
	}

	// Now armor...
	for ( i = 0; i < m_nArmorProfileCount; i++ ) {

		cchar* pszMPName = m_pArmorProfileArray[i].m_pszName;
		if (fclib_strlen( pszMPName ) < 4)
			continue;

		// If the name begins with MP_ then it is a multiplayer profile
		if ( (fclib_toupper(pszMPName[0]) == 'M') && (fclib_toupper(pszMPName[1]) == 'P') && (pszMPName[2] == '_') ) {

			// Found a MP entry. Look for the corresponding non-MP entry
			cchar* pszNonMPName = &pszMPName[3];
			for ( u32 j = 0; j < m_nArmorProfileCount; j++ ) {
				if ( !fclib_stricmp( m_pArmorProfileArray[j].m_pszName, pszNonMPName ) ) {
					// Found it! Now swap the two entries
					CArmorProfile tmp = m_pArmorProfileArray[i];
					m_pArmorProfileArray[i] = m_pArmorProfileArray[j];
					m_pArmorProfileArray[j] = tmp;

					// Now switch the names back, to avoid finding this entry again
					cchar* pcTmp = m_pArmorProfileArray[i].m_pszName;
					m_pArmorProfileArray[i].m_pszName = m_pArmorProfileArray[j].m_pszName;
					m_pArmorProfileArray[j].m_pszName = pcTmp;

					break;
				}
			}
		}
	}
}


void *CDamage::_GameDataDamageConverterCallback( cchar *pszDamageProfileName ) {
	return FindDamageProfile( pszDamageProfileName );
}


void *CDamage::_GameDataArmorConverterCallback( cchar *pszArmorProfileName ) {
	return FindArmorProfile( pszArmorProfileName );
}


void CDamage::InitLevel( void ) {
	while( m_RootQueuedDamageForm.nCount ) {
		KillDamageForm( (CDamageForm *)flinklist_GetHead( &m_RootQueuedDamageForm ) );

		DEVPRINTF( "CDamage::InitLevel\n" );
	}

	// ASSERT if there are still entities in the damage queue...
	FASSERT( CDamageForm::m_uEntityDamageQueueCount == 0 );
}


// Returns a pointer to an empty, initialized CDamageForm class.
// Returns NULL if an empty form could not be retrieved.
CDamageForm *CDamage::GetEmptyDamageFormFromPool( void ) {
	FASSERT( m_bSystemInitialized );

	// Get a CDamageForm from the free pool...
	CDamageForm *pDamageForm = (CDamageForm *)flinklist_RemoveTail( &m_RootFreeDamageForm );
	if( pDamageForm == NULL ) {
		// No more in the free pool...
		return NULL;
	}

	FASSERT( pDamageForm->m_nListMember == CDamageForm::LIST_MEMBER_FREE_POOL );

	// Initialize the CDamageForm to defaults...
	pDamageForm->Init();

	// Return it to caller...
	return pDamageForm;
}


// Constructs a new CDamageForm object in memory.
// Returns a pointer to an empty, initialized CDamageForm class.
// Returns NULL if there was insufficient memory.
CDamageForm *CDamage::BuildNewEmptyDamageForm( void ) {
	FASSERT( m_bSystemInitialized );

	// Construct a new CDamageForm...
	CDamageForm *pDamageForm = fnew CDamageForm;

	if( pDamageForm == NULL ) {
		DEVPRINTF( "CDamage::BuildNewEmptyDamageForm(): Insufficient memory to build new CDamageForm.\n" );
		return NULL;
	}

	// Initialize the CDamageForm to defaults...
	pDamageForm->Init();

	// Return it to caller...
	return pDamageForm;
}


// Constructs an array of new CDamageForm objects in memory.
// Returns a pointer to an empty, initialized CDamageForm class.
// Returns NULL if there was insufficient memory.
CDamageForm *CDamage::BuildNewEmptyDamageForm( u32 nArrayCount ) {
	FASSERT( m_bSystemInitialized );

	if( nArrayCount == 0 ) {
		return NULL;
	}

	// Construct a new CDamageForm...
	CDamageForm *pDamageForm = fnew CDamageForm[nArrayCount];

	if( pDamageForm == NULL ) {
		DEVPRINTF( "CDamage::BuildNewEmptyDamageForm(): Insufficient memory to build a new CDamageForm array.\n" );
		return NULL;
	}

	u32 i;

	// Initialize the CDamageForm objects to defaults...
	for( i=0; i<nArrayCount; ++i ) {
		pDamageForm[i].Init();
	}

	// Return it to caller...
	return pDamageForm;
}


// Set fDuration to 0.0f for a single-frame damage infliction.
// Set fDuration to <0.0f to inflict damage forever.
void CDamage::SubmitDamageForm( CDamageForm *pDamageForm, f32 fDuration ) {
	FASSERT( m_bSystemInitialized );

	if( pDamageForm == NULL ) {
		return;
	}

	FASSERT( pDamageForm->m_nListMember == CDamageForm::LIST_MEMBER_NONE );

	if( !pDamageForm->_PrepareForSubmission( fDuration ) ) {
		// Ignore form...

		if( pDamageForm->m_nFlags & CDamageForm::DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL ) {
			pDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_FREE_POOL;
			flinklist_AddTail( &m_RootFreeDamageForm, pDamageForm );
		}

		return;
	}

	if( fDuration != 0.0f ) {
		// Queue this form for processing later...

		pDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_QUEUED_POOL;
		flinklist_AddTail( &m_RootQueuedDamageForm, pDamageForm );

		return;
	}

	// Process this form immediately...
	pDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_INSTANT_POOL;
	flinklist_AddTail( &m_RootInstantDamageForm, pDamageForm );

	if( m_bBusyProcessingInstantForms ) {
		// We're already in this function. This form will be
		// processed by the original caller of this function...

		return;
	}

	m_bBusyProcessingInstantForms = TRUE;

	CDamageForm *pWorkDamageForm;

	while( pWorkDamageForm = (CDamageForm *)flinklist_RemoveHead( &m_RootInstantDamageForm ) ) {
		pWorkDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_NONE;

		pWorkDamageForm->_Work();

		// Retire the form...

		if( pWorkDamageForm->m_nFlags & CDamageForm::DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL ) {
			pWorkDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_FREE_POOL;
			flinklist_AddTail( &m_RootFreeDamageForm, pWorkDamageForm );
		}
	}

	m_bBusyProcessingInstantForms = FALSE;
}


// Removes the form from the queued list and returns it to the free pool if that's
// where it originated. If the form is already in the free list, this function does
// nothing.
void CDamage::KillDamageForm( CDamageForm *pDamageForm ) {
	CancelDamageForm( pDamageForm );

	if( pDamageForm->m_nListMember != CDamageForm::LIST_MEMBER_FREE_POOL ) {
		if( pDamageForm->m_nFlags & CDamageForm::DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL ) {
			pDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_FREE_POOL;
			flinklist_AddTail( &m_RootFreeDamageForm, pDamageForm );
		}
	}
}


// Removes the form from the queued or instant list, but will not return it to the free pool.
// Returns TRUE if the form was a member of the queued or instant pool.
// Returns FALSE otherwise.
BOOL CDamage::CancelDamageForm( CDamageForm *pDamageForm ) {
	switch( pDamageForm->m_nListMember ) {
	case CDamageForm::LIST_MEMBER_QUEUED_POOL:
		// Form is in queued pool...
		pDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_NONE;
		flinklist_Remove( &m_RootQueuedDamageForm, pDamageForm );
		return TRUE;

	case CDamageForm::LIST_MEMBER_INSTANT_POOL:
		// Form is in instant pool...
		pDamageForm->m_nListMember = CDamageForm::LIST_MEMBER_NONE;
		flinklist_Remove( &m_RootInstantDamageForm, pDamageForm );
		return TRUE;
	};

	return FALSE;
}


// Cancels and then re-submits the form.
// Returns TRUE if the cancel/resubmit operation was performed.
// Returns FALSE if the form was not in the queued or instant list
//  (in this case, the form will not be submitted).
BOOL CDamage::ResubmitDamageForm( CDamageForm *pDamageForm, f32 fDuration ) {
	if( CancelDamageForm( pDamageForm ) ) {
		SubmitDamageForm( pDamageForm, fDuration );
		return TRUE;
	}

	return FALSE;
}


void CDamage::Work( void ) {
	static CDamageForm *__apDamageFormArray[_DAMAGE_DATA_POOL_COUNT];
	u32 i, nQueuedFormCount;

	// Add all queued forms to our array...
	nQueuedFormCount = 0;
	CDamageForm *pDamageForm;
	for( pDamageForm=(CDamageForm *)flinklist_GetHead( &m_RootQueuedDamageForm ); pDamageForm; pDamageForm=(CDamageForm *)flinklist_GetNext( &m_RootQueuedDamageForm, pDamageForm ) ) {
		FASSERT( pDamageForm->m_nListMember == CDamageForm::LIST_MEMBER_QUEUED_POOL );
		__apDamageFormArray[nQueuedFormCount++] = pDamageForm;
	}

	// Call each form's work function...
	for( i=0; i<nQueuedFormCount; ++i ) {
		if( __apDamageFormArray[i]->_Work() ) {
			// This form is ready to be moved back in the free pool...

			FASSERT( __apDamageFormArray[i]->m_nListMember == CDamageForm::LIST_MEMBER_QUEUED_POOL );

			__apDamageFormArray[i]->m_nListMember = CDamageForm::LIST_MEMBER_NONE;
			flinklist_Remove( &m_RootQueuedDamageForm, __apDamageFormArray[i] );

			if( __apDamageFormArray[i]->m_nFlags & CDamageForm::DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL ) {
				__apDamageFormArray[i]->m_nListMember = CDamageForm::LIST_MEMBER_FREE_POOL;
				flinklist_AddTail( &m_RootFreeDamageForm, __apDamageFormArray[i] );
			}
		}
	}
}


CDamageProfile *CDamage::FindDamageProfile( cchar *pszProfileName, BOOL bReturnValidPointerIfNotFound, BOOL bDisplayErrorIfNotFound ) {
	FASSERT( m_bSystemInitialized );

	if( pszProfileName ) {
		u32 i;

		for( i=0; i<m_nDamageProfileCount; ++i ) {
			if( !fclib_stricmp( m_pDamageProfileArray[i].m_pszName, pszProfileName ) ) {
				// Found profile...
				return &m_pDamageProfileArray[i];
			}
		}

		// Profile not found...

		if( bDisplayErrorIfNotFound ) {
			DEVPRINTF( "CDamage::FindDamageProfile(): Damage profile '%s' doesn't exist. No damage will be generated.\n", pszProfileName );
		}
	}

	if( bReturnValidPointerIfNotFound ) {
		return &m_DamageProfileNone;
	}

	return NULL;
}


CArmorProfile *CDamage::FindArmorProfile( cchar *pszProfileName, BOOL bReturnValidPointerIfNotFound, BOOL bDisplayErrorIfNotFound ) {
	FASSERT( m_bSystemInitialized );

	if( pszProfileName ) {
		u32 i;

		for( i=0; i<m_nArmorProfileCount; ++i ) {
			if( !fclib_stricmp( m_pArmorProfileArray[i].m_pszName, pszProfileName ) ) {
				// Found profile...
				return &m_pArmorProfileArray[i];
			}
		}

		// Profile not found...

		if( bDisplayErrorIfNotFound ) {
			DEVPRINTF( "CDamage::FindDamageProfile(): Armor profile '%s' doesn't exist. No armor will be used.\n", pszProfileName );
		}
	}

	if( bReturnValidPointerIfNotFound ) {
		return &m_ArmorProfileInfinite;
	}

	return NULL;
}


void CDamage::_InitPools( void ) {
	u32 i;

	FASSERT( m_bSystemInitialized );

	flinklist_InitRoot( &m_RootFreeDamageForm, FANG_OFFSETOF( CDamageForm, m_Link ) );
	flinklist_InitRoot( &m_RootQueuedDamageForm, FANG_OFFSETOF( CDamageForm, m_Link ) );
	flinklist_InitRoot( &m_RootInstantDamageForm, FANG_OFFSETOF( CDamageForm, m_Link ) );

	flinklist_InitPool( &m_RootFreeDamageForm, m_pPoolFreeDamageForm, sizeof(CDamageForm), _DAMAGE_DATA_POOL_COUNT );

	for( i=0; i<_DAMAGE_DATA_POOL_COUNT; ++i ) {
		m_pPoolFreeDamageForm[i].Init();
		m_pPoolFreeDamageForm[i].m_nFlags = CDamageForm::DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL;
		m_pPoolFreeDamageForm[i].m_nListMember = CDamageForm::LIST_MEMBER_FREE_POOL;
	}
}


BOOL CDamage::_LoadDamageAndArmorData( void ) {
	FMemFrame_t MemFrame;
	FGameDataFileHandle_t hGameDataFile;
	FGameDataWalker_t TableWalker;
	FGameDataTableHandle_t hTable;
	CDamageProfileDef DummyDamageProfile;
	CArmorProfileDef DummyArmorProfile;
	cchar *pszProfileName;
	u32 i, j;

	static const struct { cchar *pszDamageCause; DamageCause_e nDamageCause; } __aDamageCauseMapTable[] = {
		DAMAGE_CAUSE_STRING_IMPACT,		DAMAGE_CAUSE_IMPACT,
		DAMAGE_CAUSE_STRING_BLAST,		DAMAGE_CAUSE_BLAST,
		DAMAGE_CAUSE_STRING_HEAT,		DAMAGE_CAUSE_HEAT,
		DAMAGE_CAUSE_STRING_SEVER,		DAMAGE_CAUSE_SEVER,
		DAMAGE_CAUSE_STRING_COLLISION,	DAMAGE_CAUSE_COLLISION,
		DAMAGE_CAUSE_STRING_CRUSH,		DAMAGE_CAUSE_CRUSH,
		DAMAGE_CAUSE_STRING_AMBIENT,	DAMAGE_CAUSE_AMBIENT,

		// Terminate table:
		NULL,							DAMAGE_CAUSE_IMPACT
	};


	FResFrame_t ResFrame = fres_GetFrame();

	// Load damage CSV file...
	m_pDamageProfileArray = NULL;
	m_nDamageProfileCount = 0;
	m_bDamageDataSwapped = FALSE;

	MemFrame = fmem_GetFrame();
	hGameDataFile = fgamedata_LoadFileToFMem( _DAMAGE_CSV_FILENAME );
	if( hGameDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Could not load damage file '%s'. No damage profiles have been defined.\n", _DAMAGE_CSV_FILENAME );
	} else {
		// Count the number of tables in the file...
		for( hTable = fgamedata_GetFirstTable( hGameDataFile, TableWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( TableWalker ) ) {
			if( !fgamedata_GetTableData( hTable, m_aVocab_DamageProfile, &DummyDamageProfile, sizeof(CDamageProfileDef) ) ) {
				DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Trouble parsing item '%s' in file '%s'. Profile skipped.\n", fgamedata_GetTableName(hTable), _DAMAGE_CSV_FILENAME );
				continue;
			}

			++m_nDamageProfileCount;
		}

		if( m_nDamageProfileCount ) {
			// Allocate array to hold our profiles...
			m_pDamageProfileArray = fnew CDamageProfile[m_nDamageProfileCount];
			if( m_pDamageProfileArray == NULL ) {
				DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Not enough memory to allocate m_pDamageProfileArray.\n" );
				goto _ExitWithError;
			}

			// Build profile data...
			i = 0;
			for( hTable = fgamedata_GetFirstTable( hGameDataFile, TableWalker ); i<m_nDamageProfileCount; hTable = fgamedata_GetNextTable( TableWalker ) ) {
				FASSERT( hTable != FGAMEDATA_INVALID_TABLE_HANDLE );

				pszProfileName = fgamedata_GetTableName( hTable );

				if( !fgamedata_GetTableData( hTable, m_aVocab_DamageProfile, &m_pDamageProfileArray[i], sizeof(CDamageProfileDef) ) ) {
					continue;
				}

				// Skip if profile name already exists...
				if( !fclib_stricmp( pszProfileName, "NULL" ) ) {
					DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Profile name 'NULL' in file '%s' is reserved. Profile skipped.\n", _DAMAGE_CSV_FILENAME );
					m_pDamageProfileArray[i] = m_DamageProfileNone;
					++i;
					continue;
				}

				for( j=0; j<i; ++j ) {
					if( !fclib_stricmp( pszProfileName, m_pDamageProfileArray[j].m_pszName ) ) {
						DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Profile name '%s' in file '%s' already exists. Profile skipped.\n", pszProfileName, _DAMAGE_CSV_FILENAME );
						m_pDamageProfileArray[i] = m_DamageProfileNone;
						++i;
						continue;
					}
				}

				// Store profile name...
				m_pDamageProfileArray[i].m_pszName = gstring_Main.AddString( pszProfileName );

				// Compute damage cause...
				for( j=0; __aDamageCauseMapTable[j].pszDamageCause; ++j ) {
					if( !fclib_stricmp( m_pDamageProfileArray[i].m_pszDamageCause, __aDamageCauseMapTable[j].pszDamageCause ) ) {
						m_pDamageProfileArray[i].m_nDamageCause = __aDamageCauseMapTable[j].nDamageCause;
						break;
					}
				}
				if( __aDamageCauseMapTable[j].pszDamageCause == NULL ) {
					DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Profile name '%s' in file '%s' specifies unknown damage cause '%s'. Using 'Impact'.\n", pszProfileName, _DAMAGE_CSV_FILENAME, __aDamageCauseMapTable[j].pszDamageCause );
					m_pDamageProfileArray[i].m_nDamageCause = DAMAGE_CAUSE_IMPACT;
				}

				// Compute additional fields...
				m_pDamageProfileArray[i].ComputeValuesFromProfileDef();

				++i;
			}
		}
	}

	// Release temporary memory...
	fmem_ReleaseFrame( MemFrame );

	// Load armor CSV file...
	m_pArmorProfileArray = NULL;
	m_nArmorProfileCount = 0;

	MemFrame = fmem_GetFrame();
	hGameDataFile = fgamedata_LoadFileToFMem( _ARMOR_CSV_FILENAME );
	if( hGameDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Could not load armor file '%s'. No armor profiles have been defined.\n", _ARMOR_CSV_FILENAME );
	} else {
		// Count the number of tables in the file...
		for( hTable = fgamedata_GetFirstTable( hGameDataFile, TableWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( TableWalker ) ) {
			if( !fgamedata_GetTableData( hTable, m_aVocab_ArmorProfile, &DummyArmorProfile, sizeof(CArmorProfileDef) ) ) {
				DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Trouble parsing item '%s' in file '%s'. Profile skipped.\n", fgamedata_GetTableName(hTable), _ARMOR_CSV_FILENAME );
				continue;
			}

			++m_nArmorProfileCount;
		}

		if( m_nArmorProfileCount ) {
			// Allocate array to hold our profiles...
			m_pArmorProfileArray = fnew CArmorProfile[m_nArmorProfileCount];
			if( m_pArmorProfileArray == NULL ) {
				DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Not enough memory to allocate m_pArmorProfileArray.\n" );
				goto _ExitWithError;
			}

			// Build profile data...
			i = 0;
			for( hTable = fgamedata_GetFirstTable( hGameDataFile, TableWalker ); i<m_nArmorProfileCount; hTable = fgamedata_GetNextTable( TableWalker ) ) {
				FASSERT( hTable != FGAMEDATA_INVALID_TABLE_HANDLE );

				pszProfileName = fgamedata_GetTableName( hTable );

				if( !fgamedata_GetTableData( hTable, m_aVocab_ArmorProfile, &m_pArmorProfileArray[i], sizeof(CArmorProfileDef) ) ) {
					continue;
				}

				// Skip if profile name already exists...
				if( !fclib_stricmp( pszProfileName, "NULL" ) ) {
					DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Profile name 'NULL' in file '%s' is reserved. Profile skipped.\n", _ARMOR_CSV_FILENAME );
					m_pArmorProfileArray[i] = m_ArmorProfileInfinite;
					++i;
					continue;
				}

				for( j=0; j<i; ++j ) {
					if( !fclib_stricmp( pszProfileName, m_pArmorProfileArray[j].m_pszName ) ) {
						DEVPRINTF( "CDamage::_LoadDamageAndArmorData(): Profile name '%s' in file '%s' already exists. Profile skipped.\n", pszProfileName, _ARMOR_CSV_FILENAME );
						m_pArmorProfileArray[i] = m_ArmorProfileInfinite;
						++i;
						continue;
					}
				}

				// Store profile name...
				m_pArmorProfileArray[i].m_pszName = gstring_Main.AddString( fgamedata_GetTableName(hTable) );

				++i;
			}
		}
	}

	// Release temporary memory...
	fmem_ReleaseFrame( MemFrame );

	return TRUE;

	// Not enough memory...
_ExitWithError:
	fdelete_array( m_pDamageProfileArray );
	m_pDamageProfileArray = NULL;
	m_nDamageProfileCount = 0;
	m_bDamageDataSwapped = FALSE;

	fdelete_array( m_pArmorProfileArray );
	m_pArmorProfileArray = NULL;
	m_nArmorProfileCount = 0;

	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


f32 CDamage::ComputeRadialIntensity_Rumble( f32 fDist, const CDamageProfile *pDamageProfile, f32 fNormIntensity ) {
	f32 fIntenstiy;

	fIntenstiy = ComputeRadialIntensity(
						fDist,
						pDamageProfile->m_RumbleRange.m_fInnerValue,
						pDamageProfile->m_RumbleRange.m_fOuterValue,
						pDamageProfile->m_RumbleRange.m_fInnerRadius,
						pDamageProfile->m_RumbleRange.m_fOuterRadius,
						pDamageProfile->m_fOODeltaRadius_Rumble,
						TRUE,
						fNormIntensity
					);

	return fIntenstiy;
}


f32 CDamage::ComputeRadialIntensity_Impulse( f32 fDist, const CDamageProfile *pDamageProfile, f32 fNormIntensity ) {
	f32 fIntenstiy;

	fIntenstiy = ComputeRadialIntensity(
						fDist,
						pDamageProfile->m_ImpulseRange.m_fInnerValue,
						pDamageProfile->m_ImpulseRange.m_fOuterValue,
						pDamageProfile->m_ImpulseRange.m_fInnerRadius,
						pDamageProfile->m_ImpulseRange.m_fOuterRadius,
						pDamageProfile->m_fOODeltaRadius_Impulse,
						FALSE,
						fNormIntensity
					);

	return fIntenstiy;
}


f32 CDamage::ComputeRadialIntensity_Hitpoint( f32 fDist, const CDamageProfile *pDamageProfile, f32 fNormIntensity ) {
	f32 fIntenstiy;

	fIntenstiy = ComputeRadialIntensity(
						fDist,
						pDamageProfile->m_HitpointRange.m_fInnerValue,
						pDamageProfile->m_HitpointRange.m_fOuterValue,
						pDamageProfile->m_HitpointRange.m_fInnerRadius,
						pDamageProfile->m_HitpointRange.m_fOuterRadius,
						pDamageProfile->m_fOODeltaRadius_Hitpoint,
						FALSE,
						fNormIntensity
					);

	return fIntenstiy;
}


f32 CDamage::ComputeRadialIntensity( f32 fDist, f32 fInnerValue, f32 fOuterValue, f32 fInnerRadius, f32 fOuterRadius, f32 fOODeltaRadius, BOOL bClampToUnitFloat, f32 fNormIntensity ) {
	f32 fResultNormIntensity, fUnit;

	if( fDist >= fOuterRadius ) {
		// Distance from epicenter to entity is farther than outer radius...
		fResultNormIntensity = 0.0f;
	} else if( (fOODeltaRadius == 0.0f) || (fDist <= fInnerRadius) ) {
		// Distance from epicenter to entity is closer than inner radius...
		fResultNormIntensity = fInnerValue;
	} else {
		// Distance from epicenter to entity is between inner and outer radii...
		fUnit = (fDist - fInnerRadius) * fOODeltaRadius;

		fUnit = 1.0f - fUnit;
		fUnit *= fUnit;
		fUnit = 1.0f - fUnit;
		FMATH_CLAMP_UNIT_FLOAT( fUnit );

		fResultNormIntensity = FMATH_FPOT( fUnit, fInnerValue, fOuterValue );
	}

	fResultNormIntensity *= fNormIntensity;

	if( bClampToUnitFloat ) {
		FMATH_CLAMP_UNIT_FLOAT( fResultNormIntensity );
	}

	return fResultNormIntensity;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageForm
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CDamageForm::Damager_t CDamageForm::m_TempDamager;
CEntity *CDamageForm::m_pOneSpecificEntity;
CFVec3A CDamageForm::m_aBlastPoint[_BLAST_POINT_COUNT];


// Initializes all public data members to a default value, except for
// m_nFlags::DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL and m_Link.
// m_nListMember is set to LIST_MEMBER_NONE.
void CDamageForm::Init( void ) {
	m_pFcnDamageBeingInflicted = NULL;
	m_pUserArgument = NULL;
	m_nDamageLocale = DAMAGE_LOCALE_AMBIENT;
	m_nDamageDelivery = DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
	m_pDamageProfile = NULL;
	m_fNormIntensity = 1.0f;

	m_Epicenter_WS.Zero();

	m_Damager.pEntity = NULL;
	m_Damager.pWeapon = NULL;
	m_Damager.pBot = NULL;
	m_Damager.nDamagerPlayerIndex = -1;

	m_pDamageeEntity = NULL;

	m_TriData.ImpactUnitNormal_WS = CFVec3A::m_UnitAxisZ;

	m_TriData.aTriVtx_WS[0].Set( 0.0f, 0.0f, 0.0f );
	m_TriData.aTriVtx_WS[1].Set( 0.5f, 0.5f, 0.0f );
	m_TriData.aTriVtx_WS[2].Set( 1.0f, 0.0f, 0.0f );

	m_TriData.nDamageeBoneIndex = -1;
	m_TriData.pDamageeWorldMesh = NULL;
	m_TriData.eSurfaceType = GCOLL_SURF_TYPE_NORMAL;

	m_AttackUnitDir_WS = CFVec3A::m_UnitAxisZ;
	m_ImpactPoint_WS.Zero();

	m_nListMember = LIST_MEMBER_NONE;
	m_nFlags &= DAMAGE_FLAG_FORM_ORIG_FROM_FREE_POOL;

	m_nAISoundType = AISOUNDTYPE_WEAPON;

	m_fSecsUntilFormRetired = 0.0f;
	m_fSecsUntilIssuingNextDamage = 0.0f;
	m_fLargestOuterRadius = 0.0f;
}


// If pImpactedWorldMesh is NULL, this function simply returns.
void CDamageForm::InitTriDataFromCollImpact( CFWorldMesh *pImpactedWorldMesh, const FCollImpact_t *pImpact, const CFVec3A *pAttackUnitDir_WS ) {
	if( pImpactedWorldMesh ) {
		m_TriData.ImpactUnitNormal_WS = pImpact->UnitFaceNormal;
		m_TriData.aTriVtx_WS[0] = pImpact->aTriVtx[0];
		m_TriData.aTriVtx_WS[1] = pImpact->aTriVtx[1];
		m_TriData.aTriVtx_WS[2] = pImpact->aTriVtx[2];
		m_TriData.nDamageeBoneIndex = pImpact->nBoneIndex;
		m_TriData.pDamageeWorldMesh = pImpactedWorldMesh;
		m_TriData.eSurfaceType = CGColl::GetSurfaceType( pImpact );

		m_ImpactPoint_WS = pImpact->ImpactPoint;
		m_AttackUnitDir_WS = *pAttackUnitDir_WS;
	}
}


void CDamageData::InitCollImpactFromTriData( FCollImpact_t *pImpact ) const {
	if( pImpact ) {
		if( m_pTriData ) {
			pImpact->UnitFaceNormal = m_pTriData->ImpactUnitNormal_WS;
			pImpact->aTriVtx[0] = m_pTriData->aTriVtx_WS[0];
			pImpact->aTriVtx[1] = m_pTriData->aTriVtx_WS[1];
			pImpact->aTriVtx[2] = m_pTriData->aTriVtx_WS[2];
			pImpact->nBoneIndex = m_pTriData->nDamageeBoneIndex;
		} else {
			pImpact->UnitFaceNormal.Set( CFVec3A::m_UnitAxisY );
			pImpact->aTriVtx[0].Set( CFVec3A::m_Null );
			pImpact->aTriVtx[1].Set( CFVec3A::m_Null );
			pImpact->aTriVtx[2].Set( CFVec3A::m_Null );
			pImpact->nBoneIndex = -1;
		}

		pImpact->ImpactPoint = m_ImpactPoint_WS;
	}
}


// Returns FALSE if the form should be ignored.
BOOL CDamageForm::_PrepareForSubmission( f32 fDuration ) {
	static const u32 __anLocaleToAISoundTypeMapTable[DAMAGE_LOCALE_COUNT] = {
		AISOUNDTYPE_WEAPON,			// DAMAGE_LOCALE_AMBIENT	
		AISOUNDTYPE_EXPLOSION,		// DAMAGE_LOCALE_BLAST		
		AISOUNDTYPE_WEAPON,			// DAMAGE_LOCALE_IMPACT		
	};


	FASSERT( m_nDamageLocale>=0 && m_nDamageLocale<DAMAGE_LOCALE_COUNT );
	FASSERT( m_nDamageDelivery>=0 && m_nDamageDelivery<DAMAGE_DELIVERY_COUNT );
	FASSERT( m_pDamageProfile != NULL );

	#if !FANG_PRODUCTION_BUILD
		if( m_nDamageLocale == DAMAGE_LOCALE_IMPACT ) {
			if( m_nDamageDelivery != DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY ) {
				// Invalid form...
				FASSERT_NOW_MSG( "DAMAGE_LOCALE_IMPACT requires DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY." );
			}
		}
	#endif

	if( m_pDamageProfile == NULL ) {
		return FALSE;
	} else {
		switch( m_nDamageDelivery ) {
		case CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY:
			if( m_pDamageeEntity == NULL ) {
				return FALSE;
			}

			break;

		case CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE:
			if( m_pTripwireEntity == NULL ) {
				return FALSE;
			}

			break;

		case CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_VOLUME:
			if( m_pVisVolume == NULL ) {
				return FALSE;
			}

			break;
		}
	}

	m_nAISoundType = __anLocaleToAISoundTypeMapTable[m_nDamageLocale];

	if( (m_nDamageLocale == DAMAGE_LOCALE_BLAST) || (m_nDamageDelivery == DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS) ) {
		// Find largest blast radius...

		if( m_pDamageProfile->m_HitpointRange.m_fOuterRadius > m_pDamageProfile->m_ImpulseRange.m_fOuterRadius ) {
			if( m_pDamageProfile->m_HitpointRange.m_fOuterRadius > m_pDamageProfile->m_RumbleRange.m_fOuterRadius ) {
				// Hitpoint has largest outer radius...
				m_fLargestOuterRadius = m_pDamageProfile->m_HitpointRange.m_fOuterRadius;
			} else {
				// Rumble has largest outer radius...
				m_fLargestOuterRadius = m_pDamageProfile->m_RumbleRange.m_fOuterRadius;
			}
		} else {
			if( m_pDamageProfile->m_ImpulseRange.m_fOuterRadius > m_pDamageProfile->m_RumbleRange.m_fOuterRadius ) {
				// Impulse has largest outer radius...
				m_fLargestOuterRadius = m_pDamageProfile->m_ImpulseRange.m_fOuterRadius;
			} else {
				// Rumble has largest outer radius...
				m_fLargestOuterRadius = m_pDamageProfile->m_RumbleRange.m_fOuterRadius;
			}
		}
	}

	m_fSecsUntilFormRetired = fDuration;

	return TRUE;
}


// Returns TRUE when the form has retired.
BOOL CDamageForm::_Work( void ) {
	BOOL bDealDamage = TRUE;

	if( m_fSecsUntilFormRetired != 0.0f ) {
		// See if it's time to issue more damage...

		m_fSecsUntilIssuingNextDamage -= FLoop_fPreviousLoopSecs;

		if( m_fSecsUntilIssuingNextDamage > 0.0f ) {
			// Not time to issue more damage...

			bDealDamage = FALSE;
		} else {
			// It's time to issue more damage...

			m_fSecsUntilIssuingNextDamage += m_pDamageProfile->m_fDamagePeriod;
			FMATH_CLAMPMIN( m_fSecsUntilIssuingNextDamage, 0.0f );
		}
	}

	if( bDealDamage ) {
		if( m_fNormIntensity != 0.0f ) {
			switch( m_nDamageDelivery ) {
			case DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY:
				if( !m_pDamageeEntity->IsInvincible() ) {
					_Work_OneSpecificEntity();
				}

				break;

			case DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS:
				_Work_ProfileRadius();

				break;

			case DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_VOLUME:
				_Work_SpecificVolume();
				break;

			case DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE:
				_Work_SpecificTripwire();
				break;

			default:
				FASSERT_NOW;
			}
		}
	}

	// Handle form retirement...

	if( m_fSecsUntilFormRetired == 0.0f ) {
		// Form queued for this frame only...
		return TRUE;
	}

	if( m_fSecsUntilFormRetired > 0.0f ) {
		// Timed form...

		m_fSecsUntilFormRetired -= FLoop_fPreviousLoopSecs;

		if( m_fSecsUntilFormRetired <= 0.0f ) {
			// Time to retire this form...

			m_fSecsUntilFormRetired = 0.0f;

			return TRUE;
		}
	}

	// Keep the form alive...

	return FALSE;
}


void CDamageForm::_Work_OneSpecificEntity( void ) {
	switch( m_nDamageLocale ) {
	case DAMAGE_LOCALE_AMBIENT:
		_InflictDamage_Ambient( m_pDamageeEntity, NULL );
		break;

	case DAMAGE_LOCALE_IMPACT:
		_InflictDamage_Impact( m_pDamageeEntity );
		break;

	case DAMAGE_LOCALE_BLAST:
		{
			CFSphere Sphere;

			Sphere.m_Pos = m_Epicenter_WS.v3;
			Sphere.m_fRadius = m_fLargestOuterRadius;

			// Find all intersecting entities...
			m_pOneSpecificEntity = m_pDamageeEntity;

			u32 nCurrentQueueCount = m_uEntityDamageQueueCount;
			fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_MESH, _FindTrackersCallback_OneSpecificEntity_Blast, 
											0, NULL, NULL, MESHTYPES_ENTITY, 0x7fffffffffffffff );
			while( m_uEntityDamageQueueCount > nCurrentQueueCount ) {
				--m_uEntityDamageQueueCount;
				_InflictDamage_Blast( m_apEntityDamageQueue[m_uEntityDamageQueueCount] );
			}
		}

		break;

	default:
		FASSERT_NOW;
		break;
	}
}


void CDamageForm::_Work_ProfileRadius( void ) {
	CFSphere Sphere;
	u32 nCurrentQueueCount;

	m_fLargestOuterRadius = FMATH_MAX( m_fLargestOuterRadius, m_pDamageProfile->m_HitpointRange.m_fOuterRadius );
	// sanity check, remove if necessary
	FASSERT(m_fLargestOuterRadius < 10000.f);

	Sphere.m_Pos = m_Epicenter_WS.v3;
	Sphere.m_fRadius = m_fLargestOuterRadius;

	// Find all intersecting entities...
	switch( m_nDamageLocale ) {
	case DAMAGE_LOCALE_BLAST:
		nCurrentQueueCount = m_uEntityDamageQueueCount;

		fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_MESH, _FindTrackersCallback_Blast, 0, NULL, NULL, MESHTYPES_ENTITY, 0x7fffffffffffffff );
		fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_USER, _FindTrackersCallback_Blast, 0, NULL, NULL, MESHTYPES_ENTITY, 0x7fffffffffffffff );

		while( m_uEntityDamageQueueCount > nCurrentQueueCount ) {
			--m_uEntityDamageQueueCount;
			_InflictDamage_Blast( m_apEntityDamageQueue[m_uEntityDamageQueueCount] );
		}

		break;

	case DAMAGE_LOCALE_AMBIENT:
		nCurrentQueueCount = m_uEntityDamageQueueCount;

		fworld_FindTrackersIntersectingSphere( &Sphere, FWORLD_TRACKERTYPE_MESH, _FindTrackersCallback_Ambient, 
											0, NULL, NULL, MESHTYPES_ENTITY, 0x7fffffffffffffff );
		
		// sanity check, remove if necessary
		FASSERT(Sphere.m_fRadius < 10000.f);
		
//		if ("bDebugAmbientDamage")
//		{
//		CFVec3A vSpherePos;
//		vSpherePos.v3 = Sphere.m_Pos;
//		fdraw_DevSphere(&vSpherePos,Sphere.m_fRadius);
//		}

		while( m_uEntityDamageQueueCount > nCurrentQueueCount ) {
			--m_uEntityDamageQueueCount;
			_InflictDamage_Ambient( (CEntity *)m_apEntityDamageQueue[m_uEntityDamageQueueCount]->m_pUser, &m_Epicenter_WS );
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CDamageForm::_Work_SpecificVolume( void ) {
	if( m_pVisVolume == NULL ) {
		return;
	}

	u32 nCurrentQueueCount;

	switch( m_nDamageLocale ) {
	case DAMAGE_LOCALE_BLAST:
		nCurrentQueueCount = m_uEntityDamageQueueCount;

		fworld_FindTrackersIntersectingVolume( m_pVisVolume, _FindTrackersCallback_Blast, FWORLD_TRACKERTYPE_MESH );

		while( m_uEntityDamageQueueCount > nCurrentQueueCount ) {
			--m_uEntityDamageQueueCount;
			_InflictDamage_Blast( m_apEntityDamageQueue[m_uEntityDamageQueueCount] );
		}

		break;

	case DAMAGE_LOCALE_AMBIENT:
		nCurrentQueueCount = m_uEntityDamageQueueCount;

		fworld_FindTrackersIntersectingVolume( m_pVisVolume, _FindTrackersCallback_Ambient, FWORLD_TRACKERTYPE_MESH );

		while( m_uEntityDamageQueueCount > nCurrentQueueCount ) {
			--m_uEntityDamageQueueCount;
			_InflictDamage_Ambient( (CEntity *)m_apEntityDamageQueue[m_uEntityDamageQueueCount]->m_pUser, &m_Epicenter_WS );
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CDamageForm::_Work_SpecificTripwire( void ) {
	if( m_pTripwireEntity == NULL ) {
		return;
	}

	if( !m_pTripwireEntity->TripwireDamage_IsEnabled() ) {
		// Tripwire damage is disabled...
		return;
	}

	// Tripwire damage is enabled...

#if 0
	u32 i, nContainedTripper, nCurrentQueueCount;

	for( i=0; i<

	m_pDamageForm = this;

	switch( m_nDamageLocale ) {
	case DAMAGE_LOCALE_BLAST:
		nCurrentQueueCount = m_uEntityDamageQueueCount;

		fworld_FindTrackersIntersectingVolume( m_pVisVolume, _FindTrackersCallback_Blast, FWORLD_TRACKERTYPE_MESH );

		while( m_uEntityDamageQueueCount > nCurrentQueueCount ) {
			--m_uEntityDamageQueueCount;
			_InflictDamage_Blast( m_apEntityDamageQueue[m_uEntityDamageQueueCount] );
		}

		break;

	case DAMAGE_LOCALE_AMBIENT:
		nCurrentQueueCount = m_uEntityDamageQueueCount;

		fworld_FindTrackersIntersectingVolume( m_pVisVolume, _FindTrackersCallback_Ambient, FWORLD_TRACKERTYPE_MESH );

		while( m_uEntityDamageQueueCount > nCurrentQueueCount ) {
			--m_uEntityDamageQueueCount;
			_InflictDamage_Blast( m_apEntityDamageQueue[m_uEntityDamageQueueCount] );
		}

		break;

	default:
		FASSERT_NOW;
	}
#endif
}


BOOL CDamageForm::_FindTrackersCallback_OneSpecificEntity_Blast( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	FASSERT( pTracker->m_nUser == MESHTYPES_ENTITY );

	// This is an entity...

	if( ((CEntity *)pTracker->m_pUser)->IsInvincible() ) {
		// Not a damageable entity...
		return TRUE;
	}

	if( (CEntity *)pTracker->m_pUser != m_pOneSpecificEntity ) {
		// Not the entity we're looking for...
		return TRUE;
	}

	// Found the entity we're looking for...
	_AddTrackerToDamageQueue(pTracker);
	return TRUE;
}


BOOL CDamageForm::_FindTrackersCallback_Blast( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	FASSERT( pTracker->m_nUser == MESHTYPES_ENTITY );

	// This is an entity...

	if( ((CEntity *)pTracker->m_pUser)->IsInvincible() ) {
		// Not a damageable entity...
		return TRUE;
	}

	_AddTrackerToDamageQueue( pTracker );

	return TRUE;
}


BOOL CDamageForm::_FindTrackersCallback_Ambient( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not a CEntity...
		return TRUE;
	}

	// This is an entity...
	_AddTrackerToDamageQueue(pTracker);
	return TRUE;
}


// Inflicts impact damage upon the specified entity.
void CDamageForm::_InflictDamage_Impact( CEntity *pEntity ) {
	f32 fIntensity;
	u32 i;

	_DamageData.m_nDamageLocale = DAMAGE_LOCALE_IMPACT;
	_DamageData.m_pDamageeEntity = pEntity;
	_DamageData.m_pDamageProfile = m_pDamageProfile;
	_DamageData.m_pEpicenter_WS = NULL;
	_DamageData.m_ImpactPoint_WS = m_ImpactPoint_WS;
	_DamageData.m_AttackUnitDir_WS = m_AttackUnitDir_WS;
	_DamageData.m_pTriData = &m_TriData;
	_DamageData.m_fDamageFormOrigNormIntensity = m_fNormIntensity;
	_DamageData.m_fLargestOuterRadius = m_fLargestOuterRadius;

	_SetupDamagerInfo();

	fIntensity = m_fNormIntensity * m_pDamageProfile->m_HitpointRange.m_fInnerValue;
	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		_DamageData.m_afDeltaHitpoints[i] = fIntensity * m_pDamageProfile->m_afUnitHitpoints[i];
	}

	_DamageData.m_fImpulseMag = m_fNormIntensity * m_pDamageProfile->m_ImpulseRange.m_fInnerValue;

	_DamageData.m_fRumbleUnitIntensity = m_fNormIntensity * m_pDamageProfile->m_RumbleRange.m_fInnerValue;
	FMATH_CLAMP_UNIT_FLOAT( _DamageData.m_fRumbleUnitIntensity );

	_DamageData.m_fAIAudibleRadius = m_fNormIntensity * m_pDamageProfile->m_fAIAudibleRadius;

	if( _DoesDamageDataHaveNonZeroIntensity() ) {
		BOOL bDoDamage = TRUE;

		if( m_pFcnDamageBeingInflicted ) {
			bDoDamage = m_pFcnDamageBeingInflicted( this, m_pDamageeEntity, m_pUserArgument );
		}

		if( bDoDamage ) {
			m_pDamageeEntity->InflictDamage( &_DamageData );

			//Fill out data for the last entity damaged.
			CDamage::m_LastEntityDamaged.fDeltaHitPoints = _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_PROJECTILE] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_BLAST] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_FLAME];
			CDamage::m_LastEntityDamaged.pEntity = m_pDamageeEntity;
			CDamage::m_LastEntityDamaged.eDamageType = DAMAGE_HITPOINT_TYPE_PROJECTILE;
			CDamage::m_LastEntityDamaged.pWeapon = NULL;
			CDamage::m_LastEntityDamaged.nTimeStamp = FVid_nFrameCounter;
		}
	}
}


// Inflicts ambient damage upon the specified entity.
void CDamageForm::_InflictDamage_Ambient( CEntity *pEntity, const CFVec3A *pEpicenter_WS ) {
	if( pEntity->IsInvincible() ) {
		// Not a damageable entity...
		return;
	}

	f32 fIntensity;
	u32 i;

	_DamageData.m_nDamageLocale = DAMAGE_LOCALE_AMBIENT;
	_DamageData.m_pDamageeEntity = pEntity;
	_DamageData.m_pDamageProfile = m_pDamageProfile;
	_DamageData.m_fDamageFormOrigNormIntensity = m_fNormIntensity;
	_DamageData.m_fLargestOuterRadius = m_fLargestOuterRadius;

	if( m_nDamageDelivery == DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS ) {
		_DamageData.m_pEpicenter_WS = &m_Epicenter_WS;
	} else {
		_DamageData.m_pEpicenter_WS = NULL;
	}

	_DamageData.m_ImpactPoint_WS = pEntity->MtxToWorld()->m_vPos;
	_DamageData.m_AttackUnitDir_WS = CFVec3A::m_UnitAxisZ;
	_DamageData.m_pTriData = NULL;

	_SetupDamagerInfo();

	fIntensity = m_pDamageProfile->m_HitpointRange.m_fInnerValue * m_fNormIntensity;
	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		_DamageData.m_afDeltaHitpoints[i] = fIntensity * m_pDamageProfile->m_afUnitHitpoints[i];
	}

	_DamageData.m_fImpulseMag = 0.0f;

	_DamageData.m_fRumbleUnitIntensity = m_fNormIntensity * m_pDamageProfile->m_RumbleRange.m_fInnerValue;
	FMATH_CLAMP_UNIT_FLOAT( _DamageData.m_fRumbleUnitIntensity );

	_DamageData.m_fAIAudibleRadius = m_fNormIntensity * m_pDamageProfile->m_fAIAudibleRadius;

	if( _DoesDamageDataHaveNonZeroIntensity() ) {
		BOOL bDoDamage = TRUE;

		if( m_pFcnDamageBeingInflicted ) {
			bDoDamage = m_pFcnDamageBeingInflicted( this, _DamageData.m_pDamageeEntity, m_pUserArgument );
		}

		if( bDoDamage ) {
			_DamageData.m_pDamageeEntity->InflictDamage( &_DamageData );
		}
	}
}


// Inflicts blast-locale damage upon the specified entity.
// Determines blast point and performs LOS tests for hitpoints and impulse.
void CDamageForm::_InflictDamage_Blast( CFWorldTracker *pTracker ) {
	FASSERT( pTracker->m_nUser == MESHTYPES_ENTITY );

	CEntity *pEntity = (CEntity *)pTracker->m_pUser;

	if( pEntity->IsInvincible() ) {
		// Not a damageable entity...
		return;
	}

	// This is a damageable entity...

	if( pTracker == m_TriData.pDamageeWorldMesh ) {
		// We directly hit this world mesh...

		_DamageData.m_nDamageLocale = DAMAGE_LOCALE_BLAST;
		_DamageData.m_pDamageeEntity = pEntity;
		_DamageData.m_pDamageProfile = m_pDamageProfile;
		_DamageData.m_pEpicenter_WS = &m_Epicenter_WS;
		_DamageData.m_pTriData = &m_TriData;
		_DamageData.m_fDamageFormOrigNormIntensity = m_fNormIntensity;
		_DamageData.m_fLargestOuterRadius = m_fLargestOuterRadius;

		_SetupDamagerInfo();

		_DamageData.m_fImpulseMag = _ComputeRadialIntensity_Impulse( 0.0f );

		_DamageData.m_ImpactPoint_WS = m_Epicenter_WS;
		_DamageData.m_AttackUnitDir_WS.ReceiveNegative( m_TriData.ImpactUnitNormal_WS );

		// Set up hitpoint damage data...
		u32 i;
		f32 fIntensity = _ComputeRadialIntensity_Hitpoint( 0.0f );

		for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
			_DamageData.m_afDeltaHitpoints[i] = fIntensity * m_pDamageProfile->m_afUnitHitpoints[i];
		}

		_DamageData.m_fRumbleUnitIntensity = _ComputeRadialIntensity_Rumble( 0.0f );
		_DamageData.m_fAIAudibleRadius = m_fNormIntensity * m_pDamageProfile->m_fAIAudibleRadius;

		if( _DoesDamageDataHaveNonZeroIntensity() ) {
			BOOL bDoDamage = TRUE;

			if( m_pFcnDamageBeingInflicted ) {
				bDoDamage = m_pFcnDamageBeingInflicted( this, pEntity, m_pUserArgument );
			}

			if( bDoDamage ) {
				pEntity->InflictDamage( &_DamageData );

				//Fill out data for the last entity damaged.
				CDamage::m_LastEntityDamaged.fDeltaHitPoints = _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_PROJECTILE] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_BLAST] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_FLAME];
				CDamage::m_LastEntityDamaged.pEntity = pEntity;
				CDamage::m_LastEntityDamaged.eDamageType = DAMAGE_HITPOINT_TYPE_BLAST;
				CDamage::m_LastEntityDamaged.pWeapon = NULL;
				CDamage::m_LastEntityDamaged.nTimeStamp = FVid_nFrameCounter;
			}
		}

		return;
	}

	CFVec3A SafeEpicenter_WS, EntityBoundSphereCenter_WS, EpicenterToBoundSphereCenter_WS;
	f32 fDistFromEpicenterToBoundSphereCenter;
	BOOL bSetImpactPointAndAttackDir;
	u32 i;

	// The safe epicenter will not be coincident with the bounding sphere center...
	SafeEpicenter_WS = m_Epicenter_WS;

	// Fetch entity bounding sphere center...
	EntityBoundSphereCenter_WS.Set( pTracker->GetBoundingSphere().m_Pos );

	// Compute vector from epicenter to entity bounding sphere center...
	EpicenterToBoundSphereCenter_WS.Sub( EntityBoundSphereCenter_WS, SafeEpicenter_WS );

	// If epicenter is coincident with bounding sphere, move epicenter...
	fDistFromEpicenterToBoundSphereCenter = EpicenterToBoundSphereCenter_WS.MagSq();
	if( fDistFromEpicenterToBoundSphereCenter >= 0.00001f ) {
		fDistFromEpicenterToBoundSphereCenter = fmath_Sqrt( fDistFromEpicenterToBoundSphereCenter );
	} else {
		SafeEpicenter_WS.y -= 0.1f;
		EpicenterToBoundSphereCenter_WS.Sub( EntityBoundSphereCenter_WS, SafeEpicenter_WS );
		fDistFromEpicenterToBoundSphereCenter = EpicenterToBoundSphereCenter_WS.Mag();
	}

	_DamageData.m_nDamageLocale = DAMAGE_LOCALE_BLAST;
	_DamageData.m_pDamageeEntity = pEntity;
	_DamageData.m_pDamageProfile = m_pDamageProfile;
	_DamageData.m_pEpicenter_WS = &SafeEpicenter_WS;
	_DamageData.m_pTriData = NULL;
	_DamageData.m_fDamageFormOrigNormIntensity = m_fNormIntensity;
	_DamageData.m_fLargestOuterRadius = m_fLargestOuterRadius;

	_SetupDamagerInfo();

	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		_DamageData.m_afDeltaHitpoints[i] = 0.0f;
	}

	_DamageData.m_fImpulseMag = 0.0f;

	if( pEntity->DoWeUseSphereTestForBlastDamage() ) {
		// Instead of using the blast point test, use a simple sphere test
		// to determine whether the entity has been affected by the blast...

		CFSphere BlastSphere;

		BlastSphere.m_Pos = SafeEpicenter_WS.v3;
		BlastSphere.m_fRadius = m_fLargestOuterRadius;

		if( !BlastSphere.IsIntersecting( pTracker->GetBoundingSphere() ) ) {
			// Blast sphere does not intersect tracker sphere...
			return;
		}

		// Blast sphere intersects tracker sphere!

		f32 fIntensity, fDistanceFromEpicenterToChosenImpactPoint;

		_DamageData.m_AttackUnitDir_WS.Div( EpicenterToBoundSphereCenter_WS, fDistFromEpicenterToBoundSphereCenter );

		if( !pTracker->GetBoundingSphere().IsIntersecting( SafeEpicenter_WS.v3 ) ) {
			// Epicenter lies outside the tracker sphere...

			fDistanceFromEpicenterToChosenImpactPoint = fDistFromEpicenterToBoundSphereCenter - pTracker->GetBoundingSphere().m_fRadius;

			_DamageData.m_ImpactPoint_WS.Mul( _DamageData.m_AttackUnitDir_WS, pTracker->GetBoundingSphere().m_fRadius );
			_DamageData.m_ImpactPoint_WS.RevSub( EntityBoundSphereCenter_WS );
		} else {
			// Epicenter lies inside the tracker sphere...

			fDistanceFromEpicenterToChosenImpactPoint = 0.0f;

			_DamageData.m_ImpactPoint_WS.Add( SafeEpicenter_WS, EntityBoundSphereCenter_WS ).Mul( CFVec3A::m_HalfVec );
		}

		// Set up hitpoint damage data...
		fIntensity = _ComputeRadialIntensity_Hitpoint( fDistanceFromEpicenterToChosenImpactPoint );
		for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
			_DamageData.m_afDeltaHitpoints[i] = fIntensity * m_pDamageProfile->m_afUnitHitpoints[i];
		}

		// Set up impulse damage data...
		_DamageData.m_fImpulseMag = _ComputeRadialIntensity_Impulse( fDistanceFromEpicenterToChosenImpactPoint );

		_DamageData.m_fRumbleUnitIntensity = _ComputeRadialIntensity_Rumble( fDistFromEpicenterToBoundSphereCenter );
		FMATH_CLAMP_UNIT_FLOAT( _DamageData.m_fRumbleUnitIntensity );

		_DamageData.m_fAIAudibleRadius = m_fNormIntensity * m_pDamageProfile->m_fAIAudibleRadius;

		if( _DoesDamageDataHaveNonZeroIntensity() ) {
			BOOL bDoDamage = TRUE;

			if( m_pFcnDamageBeingInflicted ) {
				bDoDamage = m_pFcnDamageBeingInflicted( this, pEntity, m_pUserArgument );
			}

			if( bDoDamage ) {
				pEntity->InflictDamage( &_DamageData );

				//Fill out data for the last entity damaged.
				CDamage::m_LastEntityDamaged.fDeltaHitPoints = _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_PROJECTILE] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_BLAST] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_FLAME];
				CDamage::m_LastEntityDamaged.pEntity = pEntity;
				CDamage::m_LastEntityDamaged.eDamageType = DAMAGE_HITPOINT_TYPE_BLAST;
				CDamage::m_LastEntityDamaged.pWeapon = NULL;
				CDamage::m_LastEntityDamaged.nTimeStamp = FVid_nFrameCounter;
			}
		}

		return;
	}

	bSetImpactPointAndAttackDir = FALSE;

	if( !m_pDamageProfile->m_abZeroInnerAndOuterValues[0] || !m_pDamageProfile->m_abZeroInnerAndOuterValues[1] ) {
		// Either the profile's hitpoint or impulse radial values are non-zero...

		f32 afOuterRadius2[2], fMag2, fDistanceFromEpicenterToChosenBlastPoint;
		u32 nBlastPointCount, nBlastPointIndex, nBlastRadiusIndex;
		s32 nUnobstructedBlastPointIndex;
		BOOL bBuiltTrackerSkipList, abTransformedToWS[_BLAST_POINT_COUNT], abComputedLineOfSite[_BLAST_POINT_COUNT], abLineOfSiteObstructed[_BLAST_POINT_COUNT];
		CFSphereA BlastPointSphere;
		CFMtx43A BlastSphereMtx;

		bBuiltTrackerSkipList = FALSE;
		nUnobstructedBlastPointIndex = -1;

		for( nBlastPointIndex=0; nBlastPointIndex<_BLAST_POINT_COUNT; ++nBlastPointIndex ) {
			abTransformedToWS[nBlastPointIndex] = FALSE;
			abComputedLineOfSite[nBlastPointIndex] = FALSE;
		}

		// Build blast point sphere...
		BlastPointSphere.m_fRadius = pTracker->GetBoundingSphere().m_fRadius * 0.5f + 0.5f;
		BlastPointSphere.m_Pos.Set( EntityBoundSphereCenter_WS );

		// Determine if epicenter lies within the blast point sphere...
		if( !BlastPointSphere.IsIntersecting( SafeEpicenter_WS ) ) {
			// Epicenter lies outside the blast point sphere.
			// Build the blast points around the sphere...

			// Generate a matrix whose XY plane faces the epicenter and whose Z axis points away from the epicenter.
			// The position of the matrix will be the tracker's bounding sphere center...
			BlastSphereMtx.m_vPos = EntityBoundSphereCenter_WS;
			BlastSphereMtx.m_vX.CrossYWithVec( EpicenterToBoundSphereCenter_WS );
			fMag2 = BlastSphereMtx.m_vX.MagSq();
			if( fMag2 >= 0.00001f ) {
				BlastSphereMtx.m_vX.Mul( fmath_InvSqrt( fMag2 ) );
				BlastSphereMtx.m_vZ.Div( EpicenterToBoundSphereCenter_WS, fDistFromEpicenterToBoundSphereCenter );
				BlastSphereMtx.m_vY.Cross( BlastSphereMtx.m_vZ, BlastSphereMtx.m_vX );
			} else {
				if( EpicenterToBoundSphereCenter_WS.y >= 0.0f ) {
					BlastSphereMtx.m_vX.Set( 1.0f, 0.0f, 0.0f );
					BlastSphereMtx.m_vY.Set( 0.0f, 0.0f, -1.0f );
					BlastSphereMtx.m_vZ.Set( 0.0f, 1.0f, 0.0f );
				} else {
					BlastSphereMtx.m_vX.Set( 1.0f, 0.0f, 0.0f );
					BlastSphereMtx.m_vY.Set( 0.0f, 0.0f, 1.0f );
					BlastSphereMtx.m_vZ.Set( 0.0f, -1.0f, 0.0f );
				}
			}

			// Form our blast detection points in matrix space...
			nBlastPointCount = _BLAST_POINT_COUNT;

			BlastPointSphere.m_fRadius = pTracker->GetBoundingSphere().m_fRadius * 0.5f;
			m_aBlastPoint[0].Set( 0.0f, 0.0f, -BlastPointSphere.m_fRadius );		// Front (always closest to epicenter)
			m_aBlastPoint[1].Set( -BlastPointSphere.m_fRadius, 0.0f, 0.0f );		// Left (same distance to epicenter as top, right, and bottom)
			m_aBlastPoint[2].Set( 0.0f, BlastPointSphere.m_fRadius, 0.0f );			// Top
			m_aBlastPoint[3].Set( BlastPointSphere.m_fRadius, 0.0f, 0.0f );			// Right
			m_aBlastPoint[4].Set( 0.0f, -BlastPointSphere.m_fRadius, 0.0f );		// Bottom
		} else {
			// Epicenter lies within the blast point sphere.
			// Let's just make our blast point half way between the epicenter and the blast point sphere center...

			nBlastPointCount = 1;
			m_aBlastPoint[0].Add( SafeEpicenter_WS, EntityBoundSphereCenter_WS ).Mul( CFVec3A::m_HalfVec );
			abTransformedToWS[0] = TRUE;
		}

		// Compute outer-radius-squared for both impulse and hitpoint ranges...
		afOuterRadius2[0] = m_pDamageProfile->m_HitpointRange.m_fOuterRadius * m_pDamageProfile->m_HitpointRange.m_fOuterRadius;
		afOuterRadius2[1] = m_pDamageProfile->m_ImpulseRange.m_fOuterRadius * m_pDamageProfile->m_ImpulseRange.m_fOuterRadius;

		for( nBlastRadiusIndex=0; nBlastRadiusIndex<2; ++nBlastRadiusIndex ) {
			if( m_pDamageProfile->m_abZeroInnerAndOuterValues[nBlastRadiusIndex] ) {
				// This blast radius doesn't exist...

				continue;
			}

			// Blast radius exists. Test blast points to find the closest one that's within
			// the blast radius and whose path to the epicenter is unobstructed...

			for( nBlastPointIndex=0; nBlastPointIndex<nBlastPointCount; ++nBlastPointIndex ) {
				if( !abTransformedToWS[nBlastPointIndex] ) {
					// Convert our blast point to world space...
					BlastSphereMtx.MulPoint( m_aBlastPoint[nBlastPointIndex] );
					abTransformedToWS[nBlastPointIndex] = TRUE;
				}

				if( nBlastPointIndex < 2 ) {
					// Compute the distance squared from the epicenter to the blast point...
					fMag2 = m_aBlastPoint[nBlastPointIndex].DistSq( SafeEpicenter_WS );

					// Check if the blast sphere reaches this blast point...
					if( fMag2 >= afOuterRadius2[nBlastRadiusIndex] ) {
						// Blast sphere doesn't reach this blast point,
						// and so it won't reach the others, either...

						break;
					}
				}

				// Blast sphere reaches this blast point.
				// Determine if the path from the epicenter to the blast point is unobstructed...

				u32 Damage_nTrackerSkipListCount=0;
				CFWorldTracker *Damage_apTrackerSkipList[FWORLD_MAX_SKIPLIST_ENTRIES];

				if( !bBuiltTrackerSkipList ) {
					// Build the tracker skip list...

					pEntity->AppendTrackerSkipList(Damage_nTrackerSkipListCount,Damage_apTrackerSkipList);

					if( m_Damager.pEntity ) {
						m_Damager.pEntity->AppendTrackerSkipList(Damage_nTrackerSkipListCount,Damage_apTrackerSkipList);
					}
					if( m_Damager.pBot && (m_Damager.pBot != m_Damager.pEntity) ) {
						m_Damager.pBot->AppendTrackerSkipList(Damage_nTrackerSkipListCount,Damage_apTrackerSkipList);
					}
					if( m_Damager.pWeapon && (m_Damager.pWeapon != m_Damager.pEntity) && ((CEntity *)m_Damager.pWeapon != (CEntity *)m_Damager.pBot) ) {
						m_Damager.pWeapon->AppendTrackerSkipList(Damage_nTrackerSkipListCount,Damage_apTrackerSkipList);
					}

					bBuiltTrackerSkipList = TRUE;
				}

				if( !abComputedLineOfSite[nBlastPointIndex] ) {
					abComputedLineOfSite[nBlastPointIndex] = TRUE;

					abLineOfSiteObstructed[nBlastPointIndex] = fworld_IsLineOfSightObstructed(
																	&SafeEpicenter_WS,
																	&m_aBlastPoint[nBlastPointIndex],
																	Damage_nTrackerSkipListCount,
																	Damage_apTrackerSkipList,
																	NULL,
																	-1,
																	FCOLL_MASK_OBSTRUCT_SPLASH_DAMAGE,
																	FCOLL_LOD_HIGHEST
																);
				}

				if( !abLineOfSiteObstructed[nBlastPointIndex] ) {
					// Line of site is not obstructed...

					nUnobstructedBlastPointIndex = nBlastPointIndex;
					fDistanceFromEpicenterToChosenBlastPoint = fmath_Sqrt( fMag2 );

					break;
				}
			}

			if( nUnobstructedBlastPointIndex >= 0 ) {
				// We found an unobstructed blast point that lies within the blast sphere...

				break;
			}
		}

		if( nUnobstructedBlastPointIndex >= 0 ) {
			f32 fIntensity;

			_DamageData.m_ImpactPoint_WS = m_aBlastPoint[nUnobstructedBlastPointIndex];
			_DamageData.m_AttackUnitDir_WS.Sub( _DamageData.m_ImpactPoint_WS, SafeEpicenter_WS ).Unitize();
			bSetImpactPointAndAttackDir = TRUE;

			// Set up hitpoint damage data...
			fIntensity = _ComputeRadialIntensity_Hitpoint( fDistanceFromEpicenterToChosenBlastPoint );
			for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
				_DamageData.m_afDeltaHitpoints[i] = fIntensity * m_pDamageProfile->m_afUnitHitpoints[i];
			}

			// Set up impulse damage data...
			_DamageData.m_fImpulseMag = _ComputeRadialIntensity_Impulse( fDistanceFromEpicenterToChosenBlastPoint );
		}
	}

	_DamageData.m_fRumbleUnitIntensity = _ComputeRadialIntensity_Rumble( fDistFromEpicenterToBoundSphereCenter );
	FMATH_CLAMP_UNIT_FLOAT( _DamageData.m_fRumbleUnitIntensity );

	_DamageData.m_fAIAudibleRadius = m_fNormIntensity * m_pDamageProfile->m_fAIAudibleRadius;

	if( _DoesDamageDataHaveNonZeroIntensity() ) {
		if( !bSetImpactPointAndAttackDir ) {
			// Impact point and attack unit direction haven't been computed yet. Do it now...

			CFSphereA BlastPointSphere;

			BlastPointSphere.m_fRadius = pTracker->GetBoundingSphere().m_fRadius * 0.5f + 0.5f;
			BlastPointSphere.m_Pos.Set( EntityBoundSphereCenter_WS );

			// Determine if epicenter lies within the blast point sphere...
			if( !BlastPointSphere.IsIntersecting( SafeEpicenter_WS ) ) {
				// Epicenter lies outside the blast point sphere.
				// We'll use the blast point that lies on the line between the epicenter and the bounding sphere center...

				_DamageData.m_ImpactPoint_WS.Div( EpicenterToBoundSphereCenter_WS, fDistFromEpicenterToBoundSphereCenter );
				_DamageData.m_ImpactPoint_WS.Mul( pTracker->GetBoundingSphere().m_fRadius * 0.5f );
				_DamageData.m_ImpactPoint_WS.RevSub( EntityBoundSphereCenter_WS );
			} else {
				// Epicenter lies within the blast point sphere.
				// Let's just make our blast point half way between the epicenter and the blast point sphere center...

				_DamageData.m_ImpactPoint_WS.Add( SafeEpicenter_WS, EntityBoundSphereCenter_WS ).Mul( CFVec3A::m_HalfVec );
			}

			_DamageData.m_AttackUnitDir_WS.Sub( _DamageData.m_ImpactPoint_WS, SafeEpicenter_WS ).Unitize();
		}

		BOOL bDoDamage = TRUE;

		if( m_pFcnDamageBeingInflicted ) {
			bDoDamage = m_pFcnDamageBeingInflicted( this, pEntity, m_pUserArgument );
		}

		if( bDoDamage ) {
			pEntity->InflictDamage( &_DamageData );

			//Fill out data for the last entity damaged.
			CDamage::m_LastEntityDamaged.fDeltaHitPoints = _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_PROJECTILE] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_BLAST] + _DamageData.m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_FLAME];
			CDamage::m_LastEntityDamaged.pEntity = pEntity;
			CDamage::m_LastEntityDamaged.eDamageType = DAMAGE_HITPOINT_TYPE_BLAST;
			CDamage::m_LastEntityDamaged.pWeapon = NULL;
			CDamage::m_LastEntityDamaged.nTimeStamp = FVid_nFrameCounter;
		}
	}
}


BOOL CDamageForm::_DoesDamageDataHaveNonZeroIntensity( void ) {
	if( _DamageData.m_fImpulseMag > 0.0f ) {
		return TRUE;
	}

	if( _DamageData.m_fRumbleUnitIntensity > 0.0f ) {
		return TRUE;
	}

	u32 i;

	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		if( _DamageData.m_afDeltaHitpoints[i] > 0.0f ) {
			return TRUE;
		}
	}

	return FALSE;
}


void CDamageForm::_SetupDamagerInfo( void ) {
	switch( m_nDamageDelivery ) {
	case DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY:
		_DamageData.m_pDamagerTripwireEntity = NULL;
		_DamageData.m_pDamagerVisVolume = NULL;
		break;

	case DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS:
		_DamageData.m_pDamagerTripwireEntity = NULL;
		_DamageData.m_pDamagerVisVolume = NULL;
		break;

	case DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_VOLUME:
		_DamageData.m_pDamagerTripwireEntity = NULL;
		_DamageData.m_pDamagerVisVolume = m_pVisVolume;
		break;

	case DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE:
		_DamageData.m_pDamagerTripwireEntity = m_pTripwireEntity;
		_DamageData.m_pDamagerVisVolume = NULL;
		break;

	default:
		FASSERT_NOW;
	}

	_DamageData.m_Damager.pEntity = m_Damager.pEntity;
	_DamageData.m_Damager.pWeapon = m_Damager.pWeapon;
	_DamageData.m_Damager.pBot = m_Damager.pBot;
	_DamageData.m_Damager.nDamagerPlayerIndex = m_Damager.nDamagerPlayerIndex;
}


f32 CDamageForm::_ComputeRadialIntensity_Rumble( f32 fDist ) {
	return CDamage::ComputeRadialIntensity_Rumble( fDist, m_pDamageProfile, m_fNormIntensity );
}


f32 CDamageForm::_ComputeRadialIntensity_Impulse( f32 fDist ) {
	return CDamage::ComputeRadialIntensity_Impulse( fDist, m_pDamageProfile, m_fNormIntensity );
}


f32 CDamageForm::_ComputeRadialIntensity_Hitpoint( f32 fDist ) {
	return CDamage::ComputeRadialIntensity_Hitpoint( fDist, m_pDamageProfile, m_fNormIntensity );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageData - Class given to a CEntity by the damage system to inflict damage onto the entity.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


// Given the specified armor profile, this function computes the damage results and
// returns them in *pDamageResult.
//
// The armor modifier can either attenuate or amplify the armor like this:
//    0: No change to armor values from profile.
//    -1 to 0: Attenuate affect of armor (-1=no armor, 0=no modification of armor).
//    0 to +1: Amplify affect of armor (0=no modification of armor, +1=infinite armor).
//
// If pArmorProfile is NULL, no armor is used.
void CDamageData::ComputeDamageResult( const CArmorProfile *pArmorProfile, CDamageResult *pDamageResult, f32 fArmorModifier, BOOL bDontApplyArmorModifierToHitpointsIfHitpointProtectionIsInfinite, f32 fDamageNormIntensity ) const {
	if( pDamageResult == NULL ) {
		return;
	}

	FMATH_CLAMP( fArmorModifier, -1.0f, 1.0f );

	if( pArmorProfile == NULL ) {
		pArmorProfile = &CDamage::m_ArmorProfileNone;
	}

	f32 afUnitSusceptibility[DAMAGE_HITPOINT_TYPE_COUNT], fUnitSusceptabilityImpulse, fUnitSusceptabilityRumble;
	u32 i;

	pDamageResult->m_pDamageData = this;
	pDamageResult->m_pArmorProfile = pArmorProfile;

	if( fArmorModifier == 0.0f ) {
		// No modification to armor profile...

		for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
			afUnitSusceptibility[i] = 1.0f - pArmorProfile->m_afUnitProtection[i];
		}

		fUnitSusceptabilityImpulse = 1.0f - pArmorProfile->m_fUnitProtectionFromImpulse;
		fUnitSusceptabilityRumble = 1.0f - pArmorProfile->m_fUnitProtectionFromRumble;
	} else if( fArmorModifier < 0.0f ) {
		// Attenuate armor...

		f32 fUnitAtten = 1.0f + fArmorModifier;

		for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
			if( !bDontApplyArmorModifierToHitpointsIfHitpointProtectionIsInfinite || (pArmorProfile->m_afUnitProtection[i] < 1.0f) ) {
				afUnitSusceptibility[i] = 1.0f - fUnitAtten * pArmorProfile->m_afUnitProtection[i];
			} else {
				afUnitSusceptibility[i] = 0.0f;
			}
		}

		fUnitSusceptabilityImpulse = 1.0f - fUnitAtten * pArmorProfile->m_fUnitProtectionFromImpulse;
		fUnitSusceptabilityRumble = 1.0f - fUnitAtten * pArmorProfile->m_fUnitProtectionFromRumble;
	} else {
		// Amplify armor...

		f32 fUnitAtten = 1.0f - fArmorModifier;

		for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
			if( !bDontApplyArmorModifierToHitpointsIfHitpointProtectionIsInfinite || (pArmorProfile->m_afUnitProtection[i] < 1.0f) ) {
				afUnitSusceptibility[i] = fUnitAtten * (1.0f - pArmorProfile->m_afUnitProtection[i]);
			} else {
				afUnitSusceptibility[i] = 0.0f;
			}
		}

		fUnitSusceptabilityImpulse = fUnitAtten * (1.0f - pArmorProfile->m_fUnitProtectionFromImpulse);
		fUnitSusceptabilityRumble = fUnitAtten * (1.0f - pArmorProfile->m_fUnitProtectionFromRumble);
	}

	// Apply difficulty biasing...
	if( m_pDamageeEntity && (m_pDamageeEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		// Receiver of damage is a bot...

		if( MultiplayerMgr.IsSinglePlayer() ) {
			// Not a multiplayer game...

			f32 fDeltaUnitSusceptabilityFromDifficulty = 0.0f;

			if( m_Damager.nDamagerPlayerIndex >= 0 ) {
				// Damager is a player...

				if( ((CBot *)m_pDamageeEntity)->m_nPossessionPlayerIndex < 0 ) {
					// Receiver of damage is an NPC bot...

					fDeltaUnitSusceptabilityFromDifficulty = CDifficulty::GetInfo()->m_fDeltaUnitSusceptabilityNPC;
				}
			} else {
				// Damager is an NPC bot...

				if( ((CBot *)m_pDamageeEntity)->m_nPossessionPlayerIndex >= 0 ) {
					// Receiver of damage is a player bot...

					fDeltaUnitSusceptabilityFromDifficulty = CDifficulty::GetInfo()->m_fDeltaUnitSusceptabilityPlayer;
				}
			}

			// Modify susceptibility...
			if( fDeltaUnitSusceptabilityFromDifficulty != 0.0f ) {
				for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
					afUnitSusceptibility[i] += afUnitSusceptibility[i] * fDeltaUnitSusceptabilityFromDifficulty;
				}
			}
		}
	}

	// Clamp susceptability values...
	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		FMATH_CLAMP_UNIT_FLOAT( afUnitSusceptibility[i] );
	}
	FMATH_CLAMP_UNIT_FLOAT( fUnitSusceptabilityImpulse );
	FMATH_CLAMP_UNIT_FLOAT( fUnitSusceptabilityRumble );

	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		pDamageResult->m_afDeltaHitpoints[i] = fDamageNormIntensity * m_afDeltaHitpoints[i] * afUnitSusceptibility[i];
	}

	if( pDamageResult->m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_BLAST] < pArmorProfile->m_fMinBlastHitpointFilter ) {
		pDamageResult->m_afDeltaHitpoints[DAMAGE_HITPOINT_TYPE_BLAST] = 0.0f;
	}

	pDamageResult->m_fDeltaHitpoints = 0.0f;
	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		pDamageResult->m_fDeltaHitpoints += pDamageResult->m_afDeltaHitpoints[i];
	}

	pDamageResult->m_fImpulseMag = m_fImpulseMag * fUnitSusceptabilityImpulse;
	pDamageResult->m_Impulse_WS.Mul( m_AttackUnitDir_WS, pDamageResult->m_fImpulseMag );
	pDamageResult->m_fRumbleUnitIntensity = m_fRumbleUnitIntensity * fUnitSusceptabilityRumble;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageRange - Utility class for ranged, interpolated data.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CDamageRange::GenerateFromLerp( f32 fUnitVal, const CDamageRange *pDamageRange0, const CDamageRange *pDamageRange1 ) {
	m_fInnerRadius = FMATH_FPOT( fUnitVal, pDamageRange0->m_fInnerRadius, pDamageRange1->m_fInnerRadius );
	m_fInnerValue = FMATH_FPOT( fUnitVal, pDamageRange0->m_fInnerValue, pDamageRange1->m_fInnerValue );
	m_fOuterRadius = FMATH_FPOT( fUnitVal, pDamageRange0->m_fOuterRadius, pDamageRange1->m_fOuterRadius );
	m_fOuterValue = FMATH_FPOT( fUnitVal, pDamageRange0->m_fOuterValue, pDamageRange1->m_fOuterValue );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageProfileDef - CSV data used to build a CDamageProfile.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CDamageProfileDef::GenerateFromLerp( f32 fUnitVal, const CDamageProfileDef *pDamageProfileDef0, const CDamageProfileDef *pDamageProfileDef1 ) {
	u32 i;

	m_nDamageCause = pDamageProfileDef0->m_nDamageCause;

	m_fDamagePeriod = FMATH_FPOT( fUnitVal, pDamageProfileDef0->m_fDamagePeriod, pDamageProfileDef1->m_fDamagePeriod );

	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		m_afUnitHitpoints[i] = FMATH_FPOT( fUnitVal, pDamageProfileDef0->m_afUnitHitpoints[i], pDamageProfileDef1->m_afUnitHitpoints[i] );
	}

	m_HitpointRange.GenerateFromLerp( fUnitVal, &pDamageProfileDef0->m_HitpointRange, &pDamageProfileDef1->m_HitpointRange );
	m_ImpulseRange.GenerateFromLerp( fUnitVal, &pDamageProfileDef0->m_ImpulseRange, &pDamageProfileDef1->m_ImpulseRange );
	m_RumbleRange.GenerateFromLerp( fUnitVal, &pDamageProfileDef0->m_RumbleRange, &pDamageProfileDef1->m_RumbleRange );

	m_fRumbleSecs = FMATH_FPOT( fUnitVal, pDamageProfileDef0->m_fRumbleSecs, pDamageProfileDef1->m_fRumbleSecs );
	m_fAIAudibleRadius = FMATH_FPOT( fUnitVal, pDamageProfileDef0->m_fAIAudibleRadius, pDamageProfileDef1->m_fAIAudibleRadius );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDamageProfile
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CDamageProfile::ComputeValuesFromProfileDef( void ) {
	if( m_HitpointRange.m_fOuterRadius > (m_HitpointRange.m_fInnerRadius + 0.01f) ) {
		m_fOODeltaRadius_Hitpoint = 1.0f / (m_HitpointRange.m_fOuterRadius - m_HitpointRange.m_fInnerRadius);
	} else {
		m_HitpointRange.m_fOuterRadius = m_HitpointRange.m_fInnerRadius;
		m_fOODeltaRadius_Hitpoint = 0.0f;
	}

	if( m_ImpulseRange.m_fOuterRadius > (m_ImpulseRange.m_fInnerRadius + 0.01f) ) {
		m_fOODeltaRadius_Impulse = 1.0f / (m_ImpulseRange.m_fOuterRadius - m_ImpulseRange.m_fInnerRadius);
	} else {
		m_ImpulseRange.m_fOuterRadius = m_ImpulseRange.m_fInnerRadius;
		m_fOODeltaRadius_Impulse = 0.0f;
	}

	if( m_RumbleRange.m_fOuterRadius > (m_RumbleRange.m_fInnerRadius + 0.01f) ) {
		m_fOODeltaRadius_Rumble = 1.0f / (m_RumbleRange.m_fOuterRadius - m_RumbleRange.m_fInnerRadius);
	} else {
		m_RumbleRange.m_fOuterRadius = m_RumbleRange.m_fInnerRadius;
		m_fOODeltaRadius_Rumble = 0.0f;
	}

	if( (m_HitpointRange.m_fInnerValue == 0.0f) && (m_HitpointRange.m_fOuterValue == 0.0f) ) {
		m_abZeroInnerAndOuterValues[0] = TRUE;
	} else {
		m_abZeroInnerAndOuterValues[0] = FALSE;
	}

	if( (m_ImpulseRange.m_fInnerValue == 0.0f) && (m_ImpulseRange.m_fOuterValue == 0.0f) ) {
		m_abZeroInnerAndOuterValues[1] = TRUE;
	} else {
		m_abZeroInnerAndOuterValues[1] = FALSE;
	}

	if( (m_RumbleRange.m_fInnerValue == 0.0f) && (m_RumbleRange.m_fOuterValue == 0.0f) ) {
		m_abZeroInnerAndOuterValues[2] = TRUE;
	} else {
		m_abZeroInnerAndOuterValues[2] = FALSE;
	}
}


void CDamageProfile::GenerateFromLerp( f32 fUnitVal, const CDamageProfile *pDamageProfile0, const CDamageProfile *pDamageProfile1 ) {
	m_pszName = pDamageProfile0->m_pszName;

	CDamageProfileDef::GenerateFromLerp( fUnitVal, pDamageProfile0, pDamageProfile1 );
	ComputeValuesFromProfileDef();
}


