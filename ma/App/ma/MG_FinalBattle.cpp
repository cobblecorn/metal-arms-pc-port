//////////////////////////////////////////////////////////////////////////////////////
// MGFinalBattle.cpp - 
//
// Author: Russell Foushee     
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
// 05/09/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "MG_FinalBattle.h"
#include "fworld.h"
#include "ftext.h"
#include "floop.h"
#include "player.h"
#include "sas_user.h"
#include "fscriptsystem.h"

#define _GAME_CSV_FILENAME			"MGFinalBatl"

#define _CORROSIVE_ENTITY_NAME		"corrosive"

// General gamplay defines
#define _NUM_GENERAL_GAMEPLAY_FIELDS 7 
#define _MISC_GAMEPLAY_TABLE_NAME	"MiscGameplay"

CMGFinalBattle*	CMGFinalBattle::m_pMGFinalBattle	= NULL;

/////////////////////////
//STATICS

BOOL CMGFinalBattle::LoadLevel( LevelEvent_e eEvent ) {
	// Only load the data on LEVEL_EVENT_PRE_ENTITY_FIXUP
	if( eEvent != LEVEL_EVENT_PRE_ENTITY_FIXUP ) {
		return TRUE;
	}

	// What we want to do here is create our class, and then load our necessary data
	FASSERT( !m_pMGFinalBattle );
	FASSERT( FWorld_pWorld );

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CMGFinalBattle::LoadLevel(): Beginning level load\n" );
#endif

	FResFrame_t frame = fres_GetFrame();

	m_pMGFinalBattle = fnew CMGFinalBattle;
	if( !m_pMGFinalBattle ) {
		DEVPRINTF( "CMGFinalBattle::LoadLevel(): ERROR:  Unable to allocate game data\n" );
		goto _ExitWithError;
	}

	if( !m_pMGFinalBattle->_Create() ) {
		DEVPRINTF( "CMGFinalBattle::LoadLevel(): ERROR:  Unable to init level\n" );
		goto _ExitWithError;
	}

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CMGFinalBattle::LoadLevel(): Level loaded\n" );
#endif

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	UnloadLevel();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CMGFinalBattle::UnloadLevel( void ) {
#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CMGFinalBattle::UnloadLevel(): Beginning level unload\n" );
#endif
	if( m_pMGFinalBattle ) {
        m_pMGFinalBattle->_Destroy();
		fdelete( m_pMGFinalBattle );
		m_pMGFinalBattle = NULL;
	}	


#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CMGFinalBattle::UnloadLevel(): Level unloaded\n" );
#endif
}


/////////////////////////
//PRIVATES

CMGFinalBattle::CMGFinalBattle() {
	m_bCreated = FALSE;
	_ClearDataMembers();
}


void CMGFinalBattle::_ClearDataMembers( void ) {

	m_pBotCorrosive = NULL;
	m_pBotGlitch = NULL;

	m_fAggressionFactor = 0.0f;
	m_fAggressionTimer = 0.0f;
}


BOOL CMGFinalBattle::_Create( void ) {
	FASSERT( !m_bCreated );
	m_bCreated = TRUE;

	_ClearDataMembers();

	FResFrame_t frame = fres_GetFrame();

	//ARG - GOTO problem
	CEntity* pTempEntity;

	if( !_InitFromCSV() ) {
		goto _ExitWithError;
	}

	// Find Corrosive:
	pTempEntity = CEntity::Find( _CORROSIVE_ENTITY_NAME );
	if( !pTempEntity ) {
		DEVPRINTF( "CMGFinalBattle::_Create(): Could not find the entity named %s\n", _CORROSIVE_ENTITY_NAME );
		return FALSE;
	}

	// Verify that the corrosive entity is actually a CBotCorrosive
	if( !( pTempEntity->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		DEVPRINTF( "CMGFinalBattle::_Create(): The entity named '%s' is NOT a CBotCorrosive!\n", _CORROSIVE_ENTITY_NAME );
		return FALSE;
	}
	m_pBotCorrosive = (CBotCorrosive*) pTempEntity;

	// Find Glitch:
	pTempEntity = CPlayer::m_pCurrent->m_pEntityCurrent;

	// Verify that the entity is actually a CBotGlitch
	if( !( pTempEntity->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
		DEVPRINTF( "CMGFinalBattle::_Create(): The current player entity named is NOT a CBotGlitch!\n" );
		return FALSE;
	}
	m_pBotGlitch = (CBotGlitch*) pTempEntity;

	//Now, apply the armor modifier to glitch...
	m_pBotGlitch->SetArmorModifier( m_fGlitchArmorModifier );

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	_Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CMGFinalBattle::_Destroy( void ) {
	if( !m_bCreated ) {
		return;
	}
}


// This function loads the game csv and 
// Initializes game variables from data
// contained within the CSV
BOOL CMGFinalBattle::_InitFromCSV( void ) {
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
		
	// grab an fres and fmem frame
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the game csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _GAME_CSV_FILENAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CMGFinalBattle::_Init() : Could not load the game init csv file '%s'.\n", _GAME_CSV_FILENAME );
		goto _ExitWithError;
	}

	// Load the Gameplay params
	// NOTE : This must be loaded before the crate groups are read in, as one of the params
	// is a chip scale, which is used for the chips that are randomly placed into the level...
	if( !_LoadMiscGameplayParams( hFile ) ) {
		goto _ExitWithError;
	}

	////////////////////////////////	    
	// done with the loaded csv file
	fmem_ReleaseFrame( hMemFrame );

	return TRUE;

_ExitWithError:

	fmem_ReleaseFrame( hMemFrame );

	return FALSE;
}




// This function loads misc gameplay parameters
//
// NOTE: If any fields are added to this gameplay table, the _NUM_GENERAL_GAMEPLAY_FIELDS define 
// at the top of this source file needs to be updated as well!
//
BOOL CMGFinalBattle::_LoadMiscGameplayParams( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nGameplayParamsInCSV;
	u32 nField;

	hTable = fgamedata_GetFirstTableHandle( hFile, _MISC_GAMEPLAY_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CMGFinalBattle::_LoadMiscGameplayParams() : Could not find the Gameplay parameters table named '%s'.\n", _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nGameplayParamsInCSV = fgamedata_GetNumFields( hTable );
	if( nGameplayParamsInCSV != _NUM_GENERAL_GAMEPLAY_FIELDS ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "CMGFinalBattle::_LoadMiscGameplayParams() : Error! -- Expecting %d fields in the table %s.  Found %d fields in table instead.\n",
			       _NUM_GENERAL_GAMEPLAY_FIELDS, _MISC_GAMEPLAY_TABLE_NAME, nGameplayParamsInCSV );
		return FALSE;
	}

	// Now, start loading the table...

	nField = 0;
	// Minimum Aggression level
	m_fMinAggressionLevel = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CMGFinalBattle::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Maximum aggression level
	m_fMaxAggressionLevel = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CMGFinalBattle::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Aggression per second increment factor
	m_fAggressionPerSecondIncrementFactor = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CMGFinalBattle::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Damage taken aggression summer
	m_fDamageTakenAggressionSummer = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CMGFinalBattle::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Bot toss level start delay
	m_fBotTossLevelStartDelay = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CMGFinalBattle::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Bot toss damage delay override
	m_fBotTossDamageDelayOverride = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CMGFinalBattle::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Glitch armor modifier value
	m_fGlitchArmorModifier = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CMGFinalBattle::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Success
	return TRUE;

_ExitWithError:

	// Failure
	return FALSE;
}


void CMGFinalBattle::Save( s32 nCheckpointID ) {
}


void CMGFinalBattle::Restore( s32 nCheckpointID ) {
	// There is no mid checkpoint in this level, so reset variables here...
	if( m_pMGFinalBattle ) {
		m_pMGFinalBattle->m_fAggressionTimer = 0.0f;
	}
}


void CMGFinalBattle::_Work( void ) {

	// Gradually increment corrosives aggression factor...  Calculate the time factor
	m_fAggressionTimer += FLoop_fPreviousLoopSecs;
	m_fAggressionFactor = m_fAggressionPerSecondIncrementFactor * m_fAggressionTimer;

	// Now, add in the amount of damage taken...
	f32 fDamageAmount = 1.0f - m_pBotCorrosive->NormHealth();
	FMATH_CLAMP_UNIT_FLOAT( fDamageAmount );

	// Sum it all up
	f32 fSummedAggression = m_fMinAggressionLevel + m_fAggressionFactor + fDamageAmount* m_fDamageTakenAggressionSummer;
	FMATH_CLAMP( fSummedAggression, m_fMinAggressionLevel, m_fMaxAggressionLevel );
	CCorrosiveCombat::SetAggressiveness( fSummedAggression );

	// Check to see if we should forbid or allow corrosives AI to toss bots...
	if( ( Level_fSecsInLevel >= m_fBotTossLevelStartDelay ) || ( fDamageAmount >= m_fBotTossDamageDelayOverride ) ) {
		// Toss away man! (Bots, that is)...
		CCorrosiveCombat::AllowBotToss( TRUE );
	} else {
		// Sorry charlie, not yet.
		CCorrosiveCombat::AllowBotToss( FALSE );
	}

	//ftext_DebugPrintf( 0.4f, 0.33f, "~w1Health:  %0.2f  Aggression:  %0.3f", m_pBotCorrosive->NormHealth(), CCorrosiveCombat::GetAggressiveness() );

	return;

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	_DebugFn();
#endif
}


#if SAS_ACTIVE_USER == SAS_USER_RUSS
void CMGFinalBattle::_DebugFn( void ) {
}
#endif


