//////////////////////////////////////////////////////////////////////////////////////
// GeneralCorrosiveGame.cpp - 
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
// 04/07/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////



#include "fang.h"
#include "GeneralCorrosiveGame.h"
#include "fworld.h"
#include "ftext.h"
#include "floop.h"
#include "player.h"
#include "sas_user.h"
#include "collectable.h"
#include "ai\\aienviro.h"
#include "fscriptsystem.h"

#if SAS_ACTIVE_USER == SAS_USER_RUSS
#define _DRAW_DEBUG_SPHERES 1
#else
#define _DRAW_DEBUG_SPHERES 0
#endif

#define _GAME_CSV_FILENAME					"GenCorrGame"
#define _CRATE_GROUPS_TABLE_NAME			"CrateGroupsNames"
#define _TRENCH_TRIPWIRE_TABLE_NAME			"Trenchtripwirenames"
#define _BRIDGE_TRIPWIRE_TABLE_NAME			"Bridgetripwirenames"

#define _CORROSIVE_ENTITY_NAME				"corrosive"
#define _CONSOLE_ENTITY_NAME				"corswitch"
#define _CHECKPOINT_TRIPWIRE_NAME			"trig_checkpoint1"
#define _LIFT_ENTITY_NAME					"alift"
#define _SWITCH_TARGET_LOCATION_ENTITY		"pressme"

#define _CENTRAL_TOWER_RADIUS_SQ			( 77.0f * 77.0f )
#define _CENTRAL_TOWER_RANGEATTACK_ALTITUDE	55.0f // This is the height Glitch should be in the central tower radius to force the Range Attack
#define _REPAIR_BAY_FLOOR_HEIGHT			0.0f

// General gamplay defines
#define _NUM_GENERAL_GAMEPLAY_FIELDS		9 
#define _MISC_GAMEPLAY_TABLE_NAME			"MiscGameplay"

static cchar* _pszMGSounds[] = 
{
	"CA_40i_030",	//  MGSOUND_ALLOYNODAMAGE
};


CGeneralCorrosiveGame*	CGeneralCorrosiveGame::m_pGeneralCorrosiveGame	= NULL;
FSndFx_FxHandle_t CGeneralCorrosiveGame::m_hSounds[ CGeneralCorrosiveGame::MGSOUND_COUNT ]; 

/////////////////////////
//STATICS

BOOL CGeneralCorrosiveGame::LoadLevel( LevelEvent_e eEvent ) {
	// Only load the data on LEVEL_EVENT_PRE_ENTITY_FIXUP
	if( eEvent != LEVEL_EVENT_PRE_ENTITY_FIXUP ) {
		return TRUE;
	}

	// What we want to do here is create our class, and then load our necessary data
	FASSERT( !m_pGeneralCorrosiveGame );
	FASSERT( FWorld_pWorld );

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CGeneralCorrosiveGame::LoadLevel(): Beginning level load\n" );
#endif

	FResFrame_t frame = fres_GetFrame();

	m_pGeneralCorrosiveGame = fnew CGeneralCorrosiveGame;
	if( !m_pGeneralCorrosiveGame ) {
		DEVPRINTF( "CGeneralCorrosiveGame::LoadLevel(): ERROR:  Unable to allocate game data\n" );
		goto _ExitWithError;
	}

	if( !m_pGeneralCorrosiveGame->_Create() ) {
		DEVPRINTF( "CGeneralCorrosiveGame::LoadLevel(): ERROR:  Unable to init level\n" );
		goto _ExitWithError;
	}

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CGeneralCorrosiveGame::LoadLevel(): Level loaded\n" );
#endif

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	UnloadLevel();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CGeneralCorrosiveGame::UnloadLevel( void ) {
#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CGeneralCorrosiveGame::UnloadLevel(): Beginning level unload\n" );
#endif
	if( m_pGeneralCorrosiveGame ) {
        m_pGeneralCorrosiveGame->_Destroy();
		fdelete( m_pGeneralCorrosiveGame );
		m_pGeneralCorrosiveGame = NULL;
	}	
	// Clear the collectable callback
	CCollectable::SetCollectionCallback( NULL );


#if SAS_ACTIVE_USER == SAS_USER_RUSS
	DEVPRINTF( "**RF:  CGeneralCorrosiveGame::UnloadLevel(): Level unloaded\n" );
#endif
}


/////////////////////////
//PRIVATES

CGeneralCorrosiveGame::CGeneralCorrosiveGame() {
	m_bCreated = FALSE;
	_ClearDataMembers();
}


void CGeneralCorrosiveGame::_ClearDataMembers( void ) {

	u32 i;

	m_fChipScale = 1.0f;
	m_pBotCorrosive = NULL;
	m_pBotGlitch = NULL;
	m_pConsole = NULL;
	m_pCheckpointTripwire = NULL;
	m_pLift = NULL;
	for( i = 0; i < 4; i++ ) {
		m_apChipCrates[ i ] = NULL;
	}

	m_nNumTrenchTripwireInfos = 0;
	m_paTrenchTripwireInfo = NULL;
	m_pTrenchTripwireInfoGlitchIsInside = NULL;

	m_nNumBridgeTripwireInfos = 0;
	m_paBridgeTripwireInfo = NULL;

	m_nChipsCollected = 0;
	m_bGlitchInPossessionOfCorrosive = FALSE;
	m_nNumTimesCorrosiveHasBeenHit = 0;
}


BOOL CGeneralCorrosiveGame::_Create( void ) {
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
		DEVPRINTF( "CGeneralCorrosiveGame::_Create(): Could not find the entity named %s\n", _CORROSIVE_ENTITY_NAME );
		return FALSE;
	}

	// Verify that the corrosive entity is actually a CBotCorrosive
	if( !( pTempEntity->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_Create(): The entity named '%s' is NOT a CBotCorrosive!\n", _CORROSIVE_ENTITY_NAME );
		return FALSE;
	}
	m_pBotCorrosive = (CBotCorrosive*) pTempEntity;

	// Find Glitch:
	pTempEntity = CPlayer::m_pCurrent->m_pEntityCurrent;

	// Verify that the entity is actually a CBotGlitch
	if( !( pTempEntity->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_Create(): The current player entity named is NOT a CBotGlitch!\n" );
		return FALSE;
	}
	m_pBotGlitch = (CBotGlitch*) pTempEntity;

	// Find the console entity:
	pTempEntity = CEntity::Find( _CONSOLE_ENTITY_NAME );
	if( !pTempEntity ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_Create(): Could not find the entity named %s!\n", _CONSOLE_ENTITY_NAME );
		return FALSE;
	}
	m_pConsole = pTempEntity;

	// Find the checkpoint save tripwire entity:
	pTempEntity = CEntity::Find( _CHECKPOINT_TRIPWIRE_NAME );
	if( !pTempEntity ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_Create(): Could not find the checkpoint save tripwire named %s!\n", _CHECKPOINT_TRIPWIRE_NAME );
		return FALSE;
	}
	m_pCheckpointTripwire = pTempEntity;

	// Find the Lift entity:
	pTempEntity = CEntity::Find( _LIFT_ENTITY_NAME );
	if( !pTempEntity ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_Create(): Could not find the lift entity named %s!\n", _LIFT_ENTITY_NAME );
		return FALSE;
	}
	m_pLift = pTempEntity;

	//load the minigame sounds.
	MiniGameSounds_e iSndIdx;
	for ( iSndIdx = MGSOUND_ALLOYNODAMAGE; iSndIdx < MGSOUND_COUNT; iSndIdx = ( MiniGameSounds_e )( iSndIdx + 1 ) ) {
		m_hSounds[ iSndIdx ] = fsndfx_GetFxHandle( _pszMGSounds[ iSndIdx ] );
		if ( !m_hSounds[ iSndIdx ] ) {
			DEVPRINTF( "CGeneralCorrosiveGame::_Create(): Could not load sound effect '%s'\n", _pszMGSounds[ iSndIdx ] );
		}
	}


	// Register our callbacks
	CCollectable::SetCollectionCallback( _CollectableCallback );
	if( !_RegisterForScriptEvents() ) {
		goto _ExitWithError;
	}
	CCorrosiveCombat::SetBehaviorRequestCallback( _CorrosiveBehaviorRequestCallback );
	CBotCorrosive::SetInflictDamageCallback( _CorrosiveDamageCallback );

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	_Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CGeneralCorrosiveGame::_Destroy( void ) {
	if( !m_bCreated ) {
		return;
	}
}


// This function loads the game csv and 
// Initializes game variables from data
// contained within the CSV
BOOL CGeneralCorrosiveGame::_InitFromCSV( void ) {
	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	
	u32 nNumEntriesInTable;	//ARG - GOTO problem
		
	// grab an fres and fmem frame
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the game csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _GAME_CSV_FILENAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_Init() : Could not load the game init csv file '%s'.\n", _GAME_CSV_FILENAME );
		goto _ExitWithError;
	}

	// Load the Gameplay params
	// NOTE : This must be loaded before the crate groups are read in, as one of the params
	// is a chip scale, which is used for the chips that are randomly placed into the level...
	if( !_LoadMiscGameplayParams( hFile ) ) {
		goto _ExitWithError;
	}

	// Load the crate groups table
	hTable = fgamedata_GetFirstTableHandle( hFile, _CRATE_GROUPS_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_InitFromCSV() : Could not find the group table named '%s'.\n", _CRATE_GROUPS_TABLE_NAME );
		goto _ExitWithError;
	}
	// Get the number of fields (which translates to the number of crate names) in the table
	nNumEntriesInTable = fgamedata_GetNumFields( hTable );
	if( nNumEntriesInTable % 2 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_InitFromCSV() : This table expects two entries per group!\n", _CRATE_GROUPS_TABLE_NAME );
		goto _ExitWithError;
	}

	u32 i;
	for( i=0; i<nNumEntriesInTable >> 1; i++ ) {

		// First, read in the crate group table name...
		cchar* pszCrateGroupName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * 2 + 0, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_InitFromCSV() : Field %d in table %s is not a STRING format!\n", i * 2 + 0, pszCrateGroupName );
			goto _ExitWithError;
		}

		// Next, read in the goodie group table name...
		cchar* pszGoodieGroupName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * 2 + 1, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_InitFromCSV() : Field %d in table %s is not a STRING format!\n", i * 2 + 1, pszGoodieGroupName );
			goto _ExitWithError;
		}

		// Now, load each crate group, select one at random, and place a chip goodie bag in it.  
		if( !_PlaceGoodiesInCrateGroups( hFile, pszCrateGroupName, pszGoodieGroupName ) ) {
			goto _ExitWithError;
		}
	}

	// Load our Trench tripwire hint regions
	if( !_LoadTrenchTripwireRegions( hFile ) ) {
		goto _ExitWithError;
	}

	// Load our Bridge Attack tripwire hint regions
	if( !_LoadBridgeTripwireRegions( hFile ) ) {
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
BOOL CGeneralCorrosiveGame::_LoadMiscGameplayParams( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	u32 nGameplayParamsInCSV;
	u32 nField;

	hTable = fgamedata_GetFirstTableHandle( hFile, _MISC_GAMEPLAY_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Could not find the Gameplay parameters table named '%s'.\n", _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Get the number of fields in this table, and make sure they are equal to the number of fields that we are supposed to load.
	nGameplayParamsInCSV = fgamedata_GetNumFields( hTable );
	if( nGameplayParamsInCSV != _NUM_GENERAL_GAMEPLAY_FIELDS ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Error! -- Expecting %d fields in the table %s.  Found %d fields in table instead.\n",
			       _NUM_GENERAL_GAMEPLAY_FIELDS, _MISC_GAMEPLAY_TABLE_NAME, nGameplayParamsInCSV );
		return FALSE;
	}

	// Now, start loading the table...

	nField = 0;
	// Minimum Aggression level
	m_fMinAggressionLevel = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Maximum aggression level
	m_fMaxAggressionLevel = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Aggression per second increment factor
	m_fAggressionPerSecondIncrementFactor = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Aggresion per chip aquired increment factor
	m_fAggressionPerChipAquiredIncrementFactor = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Aggresion per chip exposed increment factor
	m_fAggressionPerChipExposedIncrementFactor = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Skip Peer aggression threshold
	m_fSkipPeerAggressionThreshold = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Num No Damage Callbacks
	m_uNoDamageNumCallbacks = ( u32 ) ( *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType ) );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Skip Peer aggression threshold
	m_fChipScale = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}

	// Minimum number of seconds that muss pass before the alloy no damage message is played
	m_fAlloyMessageMinDelay = *( f32* ) fgamedata_GetPtrToFieldData( hTable, nField++ , nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_LoadMiscGameplayParams() : Field %d in table %s is not a FLOAT format!\n", nField - 1, _MISC_GAMEPLAY_TABLE_NAME );
		goto _ExitWithError;
	}


	// Success
	return TRUE;

_ExitWithError:

	// Failure
	return FALSE;
}


// This function loads a crate group table,
// loads the goodie group table, 
// and then places the goodies in the crates at random.
BOOL CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups( FGameDataFileHandle_t hFile, cchar *pszCrateTableName,
													    cchar *pszGoodieTableName ) {
	FMemFrame_t hMemFrame;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;

//ARG - >>>>> GOTO problem
	u32 nNumCratesInGroup;
	CEntity** papCrateEntities;
	u32 nNumEntriesInTable;
	u32 nNumGoodiesInTable;
	u32 nNumCratesWithoutGoodies;
//ARG - <<<<<

	// grab an fmem frame
	hMemFrame = fmem_GetFrame();

	// Load the crate table
	hTable = fgamedata_GetFirstTableHandle( hFile, pszCrateTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Could not find the group table named '%s'.\n", pszCrateTableName );
		goto _ExitWithError;
	}
	// Get the number of fields (which translates to the number of crate names) in the table
	nNumCratesInGroup = fgamedata_GetNumFields( hTable );
	if( nNumCratesInGroup < 1 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Expect at least one entry in the %s table name.\n", pszCrateTableName );
		goto _ExitWithError;
	}

	// Now, lets create a temporary memory structure to store the crate entity pointers in.
	// We can then use that structure to recurse through later...
	papCrateEntities = ( CEntity** ) fmem_AllocAndZero( nNumCratesInGroup * sizeof( CEntity* ) );
	if( !papCrateEntities ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Could not not allocate a temporary crate entity array structure!\n" );
		goto _ExitWithError;
	}

	// Now, recurse through each entry in the field, find the Crate Entity, and store it's pointer off
	// in the temporary array
	u32 i;
	for( i = 0; i < nNumCratesInGroup; i++ ) {
		cchar* pszCrateName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Field %d in table %s is not a STRING format!\n", i, pszCrateTableName );
			goto _ExitWithError;
		}

		// Now, find the entity with this name...  It already exists because the world has just been loaded.
		CEntity *pE = CEntity::Find( pszCrateName );
		if( !pE ) {
			DEVPRINTF("CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Entity %s in table %s NOT FOUND!\n", pszCrateName, pszCrateTableName );
			goto _ExitWithError;
		}
		papCrateEntities[i] = pE;
	}

	// Now that we have read in all the crates and found their pointers, 
	// we need to read in the goodie table and place goodies inside those
	// crates...

	// Load the goodie table
	hTable = fgamedata_GetFirstTableHandle( hFile, pszGoodieTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Could not find the group table named '%s'.\n", pszCrateTableName );
		goto _ExitWithError;
	}
	// Get the number of fields (which translates to the number of goodies / 3) in the table
	nNumEntriesInTable = fgamedata_GetNumFields( hTable );
	if( nNumEntriesInTable % 3 ) {
		// This table EXPECTS 3 entries per defined goodie...
		DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Expect 3 entries for every defined goodie in the table %s.\n", pszGoodieTableName );
		goto _ExitWithError;
	}

	// If we are here, we have validated that the table is cool, so lets rock on baby!
	nNumGoodiesInTable = nNumEntriesInTable / 3;
	nNumCratesWithoutGoodies = nNumCratesInGroup;
	for( i = 0; i < nNumGoodiesInTable; i++ ) {
		// First, read in the goodie we need to place...
		cchar* pszGoodieName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i * 3 + 0, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Field %d in table %s is not a STRING format!\n", i * 3 + 0, pszGoodieTableName );
			goto _ExitWithError;
		}

		// Get the appropriate goodie type for the chip
		CollectableType_e eGoodieType = CCollectable::GetCollectableTypeFromString( pszGoodieName );
		if( eGoodieType == -1 ) {
			DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Could not get the GoodieType_e value for the %s collectable\n", pszGoodieName );
			goto _ExitWithError;
		}

		// Notify the collectable system we will be using this collectable...
		CCollectable::NotifyCollectableUsedInWorld( eGoodieType );

		// Next, read in the quantity we want
		u32 uQuantity = (u32) ( *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * 3 + 1, nDataType ) );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Field %d in table %s is not a FLOAT format!\n", i * 3 + 1, pszGoodieTableName );
			goto _ExitWithError;
		}

		// Finally, read in the probability we want
		f32 fProb = *( f32* ) fgamedata_GetPtrToFieldData( hTable, i * 3 + 2, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_FLOAT)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Field %d in table %s is not a FLOAT format!\n", i * 3 + 2, pszGoodieTableName );
			goto _ExitWithError;
		}

		// Now, we want to place these goodies in a random crate!
		u32 j;
		for ( j = 0; j < uQuantity; j++ ) {
			if( nNumCratesWithoutGoodies > 0 ) {

				// Generate a random number and use that as the index into the crate array.
				u32 nRandomCrateID = fmath_RandomChoice( nNumCratesWithoutGoodies );
				CEntity *pCrateEntity = NULL;
				u32 nEmptyCount = 0;
				u32 k;
				for ( k = 0; k < nNumCratesInGroup; k++ ) {
					if( !papCrateEntities[k]->m_pGoodieBag ) {
						if ( nEmptyCount == nRandomCrateID ) {
							pCrateEntity = papCrateEntities[k];
							break;
						}
						nEmptyCount++;
					}
				}

				if( !pCrateEntity ) {
					DEVPRINTF( "Searched for a random empty crate and didn't find it.  This shouldn't happen!\n" );
					goto _ExitWithError;
				}

				// Now, we have our crate that we want to put the goodie bag in...
				// Create the goodie bag and set up the entity!
				CGoodieBag* pGoodieBag = fnew CGoodieBag;
				if( pGoodieBag == NULL ) {
					DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Not enough memory to allocate a pGoodieBag for crate group %s.\n", pszCrateTableName );
					goto _ExitWithError;
				}

				// Now tell the goodie bag we want one collectable in it.
				if( !pGoodieBag->AddGoodie( eGoodieType, 1 ) ) {
					DEVPRINTF( "CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Error adding '%s' goodie to a crate.\n", pszGoodieName );
					goto _ExitWithError;
				}

				// Now, assign the goodie bag to the entity and create the goodies contained within the bag.
				// Also, set the probability of this goodie bag spawning.
				pCrateEntity->m_pGoodieBag = pGoodieBag;
				pGoodieBag->SetProb( fProb );

				if( eGoodieType == COLLECTABLE_CHIP ) {
					pGoodieBag->SetGoodieScale( m_fChipScale );
				}

				pGoodieBag->CreateEntities();

				if( eGoodieType == COLLECTABLE_CHIP ) {
					// Save off this crate in our crate array.  Find an empty slot in the array
					u32 k;
					for( k = 0; k < 4; k++ ) {
						if( !m_apChipCrates[ k ] ) {
							break;
						}
					}
					FASSERT( k < 4 );
					m_apChipCrates[ k ] = pCrateEntity;
				}

				// Lastly, decrease the count of the amount of crates without goodies
				nNumCratesWithoutGoodies--;
			}
			else {
				DEVPRINTF("CGeneralCorrosiveGame::_PlaceGoodiesInCrateGroups() : Attempting to place a goodie bag in a crate, but there are no empty crates left!" );
				goto _ExitWithError;
			}
		}
	}

	// Success
	fmem_ReleaseFrame( hMemFrame );
	return TRUE;

_ExitWithError:

	// Failure
	fmem_ReleaseFrame( hMemFrame );
	return FALSE;
}



// This function loads a tripwire table, which in turn loads the entity names associated with the tripwire zone
BOOL CGeneralCorrosiveGame::_LoadTrenchTripwireRegions( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;

	u32 nNumTripwires;	//ARG - GOTO problem
	
	hTable = fgamedata_GetFirstTableHandle( hFile, _TRENCH_TRIPWIRE_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_LoadTrenchTripwireRegions() : Could not find the tripwire table named '%s'.\n", _TRENCH_TRIPWIRE_TABLE_NAME );
		goto _ExitWithError;
	}

	// Get the number of fields (which translates to the number of tripwire entities) in the table
	nNumTripwires = fgamedata_GetNumFields( hTable );
	if( nNumTripwires == 0 ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "CGeneralCorrosiveGame::_LoadTrenchTripwireRegions() : WARNING! -- No tripwire entity names found in tripwire table!\n" ) ;
		return FALSE;
	}
	else {
		m_nNumTrenchTripwireInfos = nNumTripwires;

		// Allocate the array of _Tripwire_Info_t structures
		m_paTrenchTripwireInfo = ( _TrenchTripwireInfo_t* )fres_AllocAndZero( nNumTripwires * sizeof( _TrenchTripwireInfo_t ) );
		if( !m_paTrenchTripwireInfo ) {
			DEVPRINTF( "CGeneralCorrosiveGame::_LoadTrenchTripwireRegions() : Could not allocate Tripwire info array!\n" );
			goto _ExitWithError;
		}
	}

	// Now, recurse through the tripwires and create the AI hint structures
	for( u32 i = 0; i < nNumTripwires; i++ ) {
		cchar* pszTripwireName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_LoadTrenchTripwireRegions() : Field %d in table %s is not a STRING format!\n", i, _TRENCH_TRIPWIRE_TABLE_NAME );
			goto _ExitWithError;
		}
			
		// Now, lets find the table with the same name as the entity in the CSV, so we can read in the grate entity names
		if( !_ParseTrenchTripwireTable( hFile, pszTripwireName, &m_paTrenchTripwireInfo[ i ] ) ) {
			goto _ExitWithError;
		}
	}

	// Success
	return TRUE;

_ExitWithError:

	// Failure
	return FALSE;
}


BOOL CGeneralCorrosiveGame::_ParseTrenchTripwireTable( FGameDataFileHandle_t hFile, cchar *pszTripwireTableName, 
													   _TrenchTripwireInfo_t *pTrenchTripwireInfo ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;

	u32 nNumGrates;	//ARG - GOTO problem

	// First, find the tripwire entity with the same name as the tripwire table
	CEntity *pTripwireEntity = CEntity::Find( pszTripwireTableName );
	if( !pTripwireEntity ) {
		DEVPRINTF("CGeneralCorrosiveGame::_ParseTrenchTripwireTable() : Triwire entity %s in table %s NOT FOUND!\n", pszTripwireTableName, _TRENCH_TRIPWIRE_TABLE_NAME );
		goto _ExitWithError;
	}
	pTrenchTripwireInfo->pTripwireEntity = pTripwireEntity;

	// Now, find the table with the same name as the tripwire
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTripwireTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		// While this COULD indicate an error, it's also a legal condition to have a tripwire that doesn't have
		// any tripwires associated with it, so print a warning and move on.
		DEVPRINTF( "CGeneralCorrosiveGame::_ParseTrenchTripwireTable() : WARNING! : Could not find the tripwire table named '%s'.\n", pszTripwireTableName );
		return TRUE;
	}
	// Get the number of fields (which translates to the number of grate entity names) in the table
	nNumGrates = fgamedata_GetNumFields( hTable );
	if( nNumGrates < 0 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_ParseTrenchTripwireTable() : Expect at least on entry in the %s table name.\n", pszTripwireTableName );
		goto _ExitWithError;
	}

	// Allocate the entity array memory in the tripwire info structure
	pTrenchTripwireInfo->papGrateEntities = ( CEBoomer** )fres_AllocAndZero( nNumGrates * sizeof( CEntity* ) );
	if( !pTrenchTripwireInfo->papGrateEntities ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_ParseTrenchTripwireTable() : Could not allocate Tripwire grate entity array!\n" );
		goto _ExitWithError;
	}
	pTrenchTripwireInfo->nGrateEntities = nNumGrates;

	// Now, recurse through the entries in the table, finding the entity pointers and
	// filling out the pTrenchTripwireInfo structure as we go along.
	for( u32 i = 0; i < nNumGrates; i++ ) {
		cchar* pszGrateName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_ParseTrenchTripwireTable() : Field %d in table %s is not a STRING format!\n", i, pszTripwireTableName );
			goto _ExitWithError;
		}
			
		// Find the entity pointer that has this name.
		CEntity *pGrateEntity = CEntity::Find( pszGrateName );

		if( !pGrateEntity ) {
			DEVPRINTF("CGeneralCorrosiveGame::_ParseTrenchTripwireTable() : Grate entity %s in table %s NOT FOUND!\n", pszGrateName, pszTripwireTableName );
			goto _ExitWithError;
		}

		// Verify that this is a DESTRUCT entity
		if( !( pGrateEntity->TypeBits() & ENTITY_BIT_BOOMER ) ) {
			DEVPRINTF("CGeneralCorrosiveGame::_ParseTrenchTripwireTable() : Grate entity %s in table %s is not a DESTRUCT entity!!\n", pszGrateName, pszTripwireTableName );
			goto _ExitWithError;
		}

		pTrenchTripwireInfo->papGrateEntities[ i ] = ( CEBoomer* ) pGrateEntity;
	}

	// Success
	return TRUE;

_ExitWithError:

	// Failure
	return FALSE;
}

	

// This function loads a tripwire table, which in turn loads the entity names associated with the tripwire zone
BOOL CGeneralCorrosiveGame::_LoadBridgeTripwireRegions( FGameDataFileHandle_t hFile ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;

	u32 nNumTripwires;	//ARG - GOTO problem

	hTable = fgamedata_GetFirstTableHandle( hFile, _BRIDGE_TRIPWIRE_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_LoadBridgeTripwireRegions() : Could not find the tripwire table named '%s'.\n", _BRIDGE_TRIPWIRE_TABLE_NAME );
		goto _ExitWithError;
	}

	// Get the number of fields (which translates to the number of tripwire entities) in the table
	nNumTripwires = fgamedata_GetNumFields( hTable );
	if( nNumTripwires == 0 ) {
		// Print a warning, because that's kind of bad news
		DEVPRINTF( "CGeneralCorrosiveGame::_LoadBridgeTripwireRegions() : WARNING! -- No tripwire entity names found in tripwire table!\n" ) ;
		return FALSE;
	}
	else {
		m_nNumBridgeTripwireInfos = nNumTripwires;

		// Allocate the array of _BridgeTripwireInfo_t structures
		m_paBridgeTripwireInfo = ( _BridgeTripwireInfo_t* )fres_AllocAndZero( nNumTripwires * sizeof( _BridgeTripwireInfo_t ) );
		if( !m_paBridgeTripwireInfo ) {
			DEVPRINTF( "CGeneralCorrosiveGame::_LoadBridgeTripwireRegions() : Could not allocate Tripwire info array!\n" );
			goto _ExitWithError;
		}
	}

	// Now, recurse through the tripwires and fill the bridge tripwire structures
	for( u32 i = 0; i < nNumTripwires; i++ ) {
		cchar* pszTripwireName = (cchar *)fgamedata_GetPtrToFieldData( hTable, i, nDataType );
		if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
		{
			DEVPRINTF("CGeneralCorrosiveGame::_LoadBridgeTripwireRegions() : Field %d in table %s is not a STRING format!\n", i, _BRIDGE_TRIPWIRE_TABLE_NAME );
			goto _ExitWithError;
		}
			
		// Now, lets find the table with the same name as the entity in the CSV, so we can read in the grate entity names
		if( !_ParseBridgeTripwireTable( hFile, pszTripwireName, &m_paBridgeTripwireInfo[ i ] ) ) {
			goto _ExitWithError;
		}
	}

	// Success
	return TRUE;

_ExitWithError:

	// Failure
	return FALSE;
}


// This function will load a bridge tripwire table.  The table associates a tripwire with an attack location with the entity
// that will be attacked.  The attack location can be moved around and tuned by the LD's.
BOOL CGeneralCorrosiveGame::_ParseBridgeTripwireTable( FGameDataFileHandle_t hFile, cchar *pszTripwireTableName, 
													   _BridgeTripwireInfo_t *pBridgeTripwireInfo ) {

	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;

//ARG - >>>>> GOTO problem
	cchar* pszGizmoName;
	CEntity *pGizmoEntity;
	cchar* pszBridgeSectionName;
	CEntity *pBridgeSectionEntity;
//ARG - <<<<<

	// First, find the tripwire entity with the same name as the tripwire table
	CEntity *pTripwireEntity = CEntity::Find( pszTripwireTableName );
	if( !pTripwireEntity ) {
		DEVPRINTF("CGeneralCorrosiveGame::_ParseBridgeTripwireTable() : Triwire entity %s in table %s NOT FOUND!\n", pszTripwireTableName, _BRIDGE_TRIPWIRE_TABLE_NAME );
		goto _ExitWithError;
	}
	pBridgeTripwireInfo->pTripwireEntity = pTripwireEntity;

	// Now, find the table with the same name as the tripwire
	hTable = fgamedata_GetFirstTableHandle( hFile, pszTripwireTableName );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_ParseBridgeTripwireTable() : Could not find the tripwire table named '%s'.\n", pszTripwireTableName );
		goto _ExitWithError;
	}

	// verify the number of fields to be 2 in the table
	if( fgamedata_GetNumFields( hTable ) != 2 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_ParseBridgeTripwireTable() : Expecting exactly 2 fields in the %s table.\n", pszTripwireTableName );
		goto _ExitWithError;
	}

	// Get the Attack location gizmo name from the table.
	pszGizmoName = (cchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_ParseBridgeTripwireTable() : The Attack Location gizmo field (field 0) in table %s is not a STRING format!\n", pszTripwireTableName );
		goto _ExitWithError;
	}
		
	// Find the entity pointer that has this name.
	pGizmoEntity = CEntity::Find( pszGizmoName );

	if( !pGizmoEntity ) {
		DEVPRINTF("CGeneralCorrosiveGame::_ParseBridgeTripwireTable() : Attack location entity %s in table %s NOT FOUND!\n", pszGizmoName, pszTripwireTableName );
		goto _ExitWithError;
	}
	pBridgeTripwireInfo->pAttackLocationEntity = pGizmoEntity;

	// Get the Bridge section name from the table.
	pszBridgeSectionName = (cchar *)fgamedata_GetPtrToFieldData( hTable, 1, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_STRING)
	{
		DEVPRINTF("CGeneralCorrosiveGame::_ParseBridgeTripwireTable() : The Bridge Section name field (field 1) in table %s is not a STRING format!\n", pszTripwireTableName );
		goto _ExitWithError;
	}
		
	// Find the entity pointer that has this name.
	pBridgeSectionEntity = CEntity::Find( pszBridgeSectionName );

// RAFHACK -- Commented out for the short term because the level currently does not contain independent damagable bridge sections!
//	if( !pBridgeSectionEntity ) {
//		DEVPRINTF("CGeneralCorrosiveGame::_ParseBridgeTripwireTable() : Bridge Section entity %s in table %s NOT FOUND!\n", pszBridgeSectionName, pszTripwireTableName );
//		goto _ExitWithError;
//	}
	pBridgeTripwireInfo->pBridgeSectionEntity = pBridgeSectionEntity;

	// Success
	return TRUE;

_ExitWithError:

	// Failure
	return FALSE;
}


BOOL CGeneralCorrosiveGame::_RegisterForScriptEvents( void ) {
	// This function should register ourself with the scripting event system
	// for different types of events.

	// First, register for a tripwire event
	s32 nTriggerEvent = CFScriptSystem::GetEventNumFromName( "tripwire" );
	if( nTriggerEvent == -1 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not find tripwire event.\n" );
		return FALSE;
	}

	u64 uEventFlags = fevent_Bit( nTriggerEvent );

	if( !CFScriptSystem::RegisterEventListener( _TripwireEventCallback, NULL, &uEventFlags, (u32) nTriggerEvent ) ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not register Tripwire listener callback.\n" );
		return FALSE;
	}

	// next, register for a possession event
	nTriggerEvent = CFScriptSystem::GetEventNumFromName( "possess" );
	if( nTriggerEvent == -1 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not find possess event.\n" );
		return FALSE;
	}

	uEventFlags = fevent_Bit( nTriggerEvent );

	if( !CFScriptSystem::RegisterEventListener( _PossessEventCallback, NULL, &uEventFlags, (u32) nTriggerEvent ) ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not register possession listener callback.\n" );
		return FALSE;
	}

	// Register for a switch event
	nTriggerEvent = CFScriptSystem::GetEventNumFromName( "switch" );
	if( nTriggerEvent == -1 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not find switch event.\n" );
		return FALSE;
	}

	uEventFlags = fevent_Bit( nTriggerEvent );

	if( !CFScriptSystem::RegisterEventListener( _SwitchEventCallback, NULL, &uEventFlags, (u32) nTriggerEvent ) ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not register switch listener callback.\n" );
		return FALSE;
	}

	// Register for a destruct event
	nTriggerEvent = CFScriptSystem::GetEventNumFromName( "destruct" );
	if( nTriggerEvent == -1 ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not find destruct event.\n" );
		return FALSE;
	}

	uEventFlags = fevent_Bit( nTriggerEvent );

	if( !CFScriptSystem::RegisterEventListener( _DestructEventCallback, NULL, &uEventFlags, (u32) nTriggerEvent ) ) {
		DEVPRINTF( "CGeneralCorrosiveGame::_RegisterForScriptEvents() : Could not register destruct listener callback.\n" );
		return FALSE;
	}

	return TRUE;
}	


// This function gets called every time a collectable is picked up by any bot in the game
// Corrosive's AI is interested in this because if the collectable is a chip, then he should get really mad!
void CGeneralCorrosiveGame::_CollectableCallback( CollectableType_e eType, CBot *pPickupBot ) {

	// We are only interested in the COLLECTABLE_CHIP type
	if( eType == COLLECTABLE_CHIP ) {
		m_pGeneralCorrosiveGame->m_nChipsCollected++;

		// Check to see if we should add the checkpoint tripwire back into the world...
		if( m_pGeneralCorrosiveGame->m_nChipsCollected == 4 ) {
			m_pGeneralCorrosiveGame->m_pCheckpointTripwire->AddToWorld();
		}

		CCorrosiveCombat::SetRoarThisFrame( TRUE );
		f32 fAggressiveness = CCorrosiveCombat::GetAggressiveness();
		fAggressiveness += m_pGeneralCorrosiveGame->m_fAggressionPerChipAquiredIncrementFactor;
		FMATH_CLAMP( fAggressiveness, m_pGeneralCorrosiveGame->m_fMinAggressionLevel, m_pGeneralCorrosiveGame->m_fMaxAggressionLevel );
		CCorrosiveCombat::SetAggressiveness( fAggressiveness );
	}
}


void CGeneralCorrosiveGame::_TripwireEventCallback( s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3 ) {

	// Decode the EventData parameters into more useful classes:

	CEntity *pTripwireEntity = (CEntity*) uEventData2;
	CEntity *pTripperEntity = (CEntity*) uEventData3;
	if( ( pTripwireEntity == NULL ) || ( pTripperEntity == NULL ) ) {
		return;
	}

	// Give the Player location hint callback first shot at this callback...
	if( m_pGeneralCorrosiveGame->_GlitchHintLocEvaluateTripwireCallback( pTripwireEntity, pTripperEntity, uEventData1 ) ) {
		return;
	}

	// Next, give the corrosive Bridge Attack tripwires a chance to check out this tripwire
	if( m_pGeneralCorrosiveGame->_CorrosiveAttackBridgeEvaluateTripwireCallback( pTripwireEntity, pTripperEntity, uEventData1 ) ) {
		return;
	}

	//Otherwise, we don't care about this tripwire going off...
	return;
}


// This function is used to detect when corrosive is unpossessed.  When he is, just power him down for good.
void CGeneralCorrosiveGame::_PossessEventCallback( s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3 ) {
	FASSERT( m_pGeneralCorrosiveGame );

	// Decode the EventData parameters into more useful variables:

	CBot *pPossessedBot = (CBot*) uEventData2;
	BOOL bPossessing = ( BOOL ) uEventData3;

	if( pPossessedBot == (CBot*)m_pGeneralCorrosiveGame->m_pBotCorrosive ) {
		// Corrosive is being possessed / unpossessed
		if( bPossessing ) {
			m_pGeneralCorrosiveGame->m_bGlitchInPossessionOfCorrosive = TRUE;
			m_pGeneralCorrosiveGame->m_pLift->SetWalkable( FALSE );
		} else if( m_pGeneralCorrosiveGame->m_bGlitchInPossessionOfCorrosive ) {
			// Corrosive is being unpossessed, and it's not a level restart 
			// The possession event comes after the checkpoint restore code, so check the
			// m_bGlitchInPossessionOfCorrosive flag to see if glitch is currently in possession
			// of corrosive... If he is, then power him down... If not, then this is a checkpoint
			// restore, and we don't want to power down corrosive!
			
			// Power down corrosive
			m_pGeneralCorrosiveGame->m_pBotCorrosive->Power_SetState( FALSE );
			m_pGeneralCorrosiveGame->m_bGlitchInPossessionOfCorrosive = FALSE;
			m_pGeneralCorrosiveGame->m_pLift->SetWalkable( TRUE );
		}
	}
	if( ( pPossessedBot == (CBot*)m_pGeneralCorrosiveGame->m_pBotCorrosive ) && !bPossessing ) {
		// Corrosive is being unpossessed... Just power him down for good at this point...
	}
}


// This function is used to determine when the console switch is depressed.
// When it gets a valid switch event, then kick off the corrosive button press animation 
void CGeneralCorrosiveGame::_SwitchEventCallback( s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3 ) {
	FASSERT( m_pGeneralCorrosiveGame );

	// Get the switch entity
	CEntity *pSwitchEntity = ( CEntity* ) uEventData2;

	// Get the bot triggering the switch
	CBot *pTriggeringBot = ( CBot* ) uEventData3;

	// Ensure the following conditions:
	//	1) The switch being actioned is the console switch
	//	2) The entity doing the actioning is General Corrosive
	if( ( pSwitchEntity == m_pGeneralCorrosiveGame->m_pConsole ) && ( pTriggeringBot == m_pGeneralCorrosiveGame->m_pBotCorrosive ) ) {
		// Here, we should kick off the button press animation.
	}
}



// This function is invoked when we get a destruct event.
// We want to look for when one of the crates containing our chips is destroyed.  When it is,
// increase corrosives aggression and make him roar...
void CGeneralCorrosiveGame::_DestructEventCallback( s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3 ) {
	FASSERT( m_pGeneralCorrosiveGame );

	u32 i;

	// Get the entity that has been destroyed, and see if it's one of our chip crates
	BOOL bWasAChipCrate = FALSE;
	CEntity *pDestructedEntity = ( CEntity* ) uEventData2;
	for( i = 0; i < 4; i++ ) {
		if( pDestructedEntity == m_pGeneralCorrosiveGame->m_apChipCrates[ i ] ) {
			bWasAChipCrate = TRUE;
			break;
		}
	}

	if( bWasAChipCrate ) {
		// Increase corrosives aggression
		CCorrosiveCombat::SetRoarThisFrame( TRUE );
		f32 fAggressiveness = CCorrosiveCombat::GetAggressiveness();
		fAggressiveness += m_pGeneralCorrosiveGame->m_fAggressionPerChipExposedIncrementFactor;
		FMATH_CLAMP( fAggressiveness, m_pGeneralCorrosiveGame->m_fMinAggressionLevel, m_pGeneralCorrosiveGame->m_fMaxAggressionLevel );
		CCorrosiveCombat::SetAggressiveness( fAggressiveness );
	}
}




// This function gets invoked when corrosive gets hit
void CGeneralCorrosiveGame::_CorrosiveDamageCallback( void ) {
	if( m_pGeneralCorrosiveGame ) {
		m_pGeneralCorrosiveGame->m_nNumTimesCorrosiveHasBeenHit++;

		// Check to see if the minimum number of seconds has passed to play the message
		if( Level_fSecsInLevel < m_pGeneralCorrosiveGame->m_fAlloyMessageMinDelay ) {
			// We can't play this message yet, but limit our damage to 1 minus the threshhold that will
			// trigger the message...
			if( m_pGeneralCorrosiveGame->m_nNumTimesCorrosiveHasBeenHit >= m_pGeneralCorrosiveGame->m_uNoDamageNumCallbacks ) {
				m_pGeneralCorrosiveGame->m_nNumTimesCorrosiveHasBeenHit = m_pGeneralCorrosiveGame->m_uNoDamageNumCallbacks - 1;
			}
		}
		else if( ( m_pGeneralCorrosiveGame->m_nNumTimesCorrosiveHasBeenHit == m_pGeneralCorrosiveGame->m_uNoDamageNumCallbacks ) &&
		    ( !m_pGeneralCorrosiveGame->m_pBotGlitch->IsDeadOrDying() ) ) {
			// Play the colAlloy message
			CHud2::GetCurrentHud()->TransmissionMsg_Start( CHud2::TRANSMISSION_AUTHOR_COLONEL_ALLOY, m_hSounds[ MGSOUND_ALLOYNODAMAGE ], 1.0f, FALSE );
		}
	}
}


// This function will evaluate the tripwire event and see if its interested in this event.  If it IS interested in this event
// it will return true and further processing on this tripwire event will ceases...
BOOL CGeneralCorrosiveGame::_GlitchHintLocEvaluateTripwireCallback( CEntity* pTripwireEntity, CEntity* pTripperEntity, u32 uEventData1 ) {

	// First, we are only interested in tripwires that bots enter and exits...
	if ( !( pTripperEntity->TypeBits() & ENTITY_BIT_BOT ) ) {
		// This entity is NOT a bot.. bail out
		return FALSE;
	}

	// Safe to cast this guy as a bot
	CBot *pBot = (CBot*) pTripperEntity;


	if( pBot->m_nPossessionPlayerIndex < 0 ) {
		// This bot is not player controlled... bail out
		return FALSE;
	}

	// Next, we want to see if this is a tripwire in our list...
	_TrenchTripwireInfo_t *pTripwireInfo = NULL;
	for( u32 i = 0; i < m_nNumTrenchTripwireInfos; i++ ) {
		if( m_paTrenchTripwireInfo[i].pTripwireEntity == pTripwireEntity ) {
			pTripwireInfo = &m_paTrenchTripwireInfo[i];
			break;
		}
	}

	if( !pTripwireInfo ) {
		// This is NOT a tripwire that we are interested in.  Bail out
		return FALSE;
	}

	// The tripwire that was triggered is one of the tripwires we are interested in,
	// so lets do some further processing on it.
	if( uEventData1 == 0 ) {
		// Glitch is entering this tripwire.  Record off the tripwire info structure in the mini game class
		m_pTrenchTripwireInfoGlitchIsInside = pTripwireInfo;
		// When glitch is in these trenches, have corrosive go to range attack only mode...
		//AIEnviro_BoostPlayerSoundTo( pBot->m_nPossessionPlayerIndex, 0.00f ); // Make him silent in the corridors...
	} else if ( uEventData1 == 1 ) {
		// Glitch is LEAVING the tripwire... Set the tripwire info structure to null 
		// if the current Info structure pointer == pTripwireInfo.
		if( m_pTrenchTripwireInfoGlitchIsInside == pTripwireInfo ) {
			m_pTrenchTripwireInfoGlitchIsInside = NULL;
		}
	}
	
	return TRUE;
}



// This function will evaluate the tripwire event and see if its interested in this event.  If it IS interested in this event
// it will return true and further processing on this tripwire event will ceases...
BOOL CGeneralCorrosiveGame::_CorrosiveAttackBridgeEvaluateTripwireCallback( CEntity* pTripwireEntity, CEntity* pTripperEntity, u32 uEventData1 ) {

	// First, we are only interested in tripwires that corrosive enters
	if ( !( pTripperEntity->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) ) {
		// This entity is NOT Corrosive.. bail out
		return FALSE;
	}

	// Safe to cast this guy as a bot
	CBotCorrosive *pBotCorrosive = (CBotCorrosive*) pTripperEntity;

	// Next, verify that we are only looking at tripwire ENTER events...
	if( uEventData1 != 0 ) {
		return FALSE;
	}

    // Next, we want to see if this is a tripwire in our list...
	_BridgeTripwireInfo_t *pTripwireInfo = NULL;
	for( u32 i = 0; i < m_nNumBridgeTripwireInfos; i++ ) {
		if( m_paBridgeTripwireInfo[i].pTripwireEntity == pTripwireEntity ) {
			pTripwireInfo = &m_paBridgeTripwireInfo[i];
			break;
		}
	}

	if( !pTripwireInfo ) {
		// This is NOT a tripwire that we are interested in.  Bail out
		return FALSE;
	}

	CCorrosiveCombat::AttackThisLocation( &pTripwireInfo->pAttackLocationEntity->MtxToWorld()->m_vPos, FALSE );
	return TRUE;
}



BOOL CGeneralCorrosiveGame::_CorrosiveBehaviorRequestCallback( AIBehaviorReason_e eReason ) {
	// In this call, Corrosive's ai is looking for a hint as to what to do next.
	// Evaluate the reason of the request, and perhaps issue a commanded behavior

	BOOL bRetVal = FALSE;

	switch( eReason ) {
		case AI_BEHAVIOR_REASON_LOST_LOS:
			bRetVal = m_pGeneralCorrosiveGame->_CommandPeerIfGlitchInTrenches();
			break;

		case AI_BEHAVIOR_REASON_ATTACK_COMPLETE:
			// Let him revert back to his base attack state...
			// He'll be requesting a hint soon enough...
			bRetVal = FALSE;
			break;

		case AI_BEHAVIOR_REASON_PEER_COMPLETE:
			// The peer has been completed... We should have corrosive
			// stomp that location now...
			CCorrosiveCombat::AttackThisLocation( &m_pGeneralCorrosiveGame->m_vPeerLocation, TRUE );
			bRetVal = TRUE;
			break;
	}

	return bRetVal;
}


BOOL CGeneralCorrosiveGame::_CommandPeerIfGlitchInTrenches( void ) { 
	// If glitch is in one of the tripwires, then we will return the position of the tripwire

	CFVec3A vPosition;

	if( !m_pGeneralCorrosiveGame->m_pTrenchTripwireInfoGlitchIsInside ) {
		// Glitch is not inside one of the tripwires, so just return FALSE.
		return FALSE;
	}

	// If we are here, Glitch is currently inside one of the tripwires, so find a location to look

	u32 i;
	u32 nNonDestructedGrates = 0;
	for( i = 0; i < m_pTrenchTripwireInfoGlitchIsInside->nGrateEntities; i++ ) {
		if( m_pTrenchTripwireInfoGlitchIsInside->papGrateEntities[ i ]->NormHealth() > 0.0f ) {
			nNonDestructedGrates++;
		}
	}

	if( nNonDestructedGrates == 0 ) {
		// All grates are destroyed with this guy, so don't command a behavior.
		return FALSE;
	}

	// Now, we have the total number of non-destructed grates...  Generate a random number and pick the appropriate one.
	u32 nRandomGrate = fmath_RandomChoice( nNonDestructedGrates );
	u32 nCurrentNonDestructedGrate = 0;
	CEBoomer *pNonDestructedGrate = NULL;
	for( i = 0; i < m_pTrenchTripwireInfoGlitchIsInside->nGrateEntities; i++ ) {
		if( m_pTrenchTripwireInfoGlitchIsInside->papGrateEntities[ i ]->NormHealth() > 0.0f ) {
			if( nCurrentNonDestructedGrate == nRandomGrate ) {
				pNonDestructedGrate = m_pTrenchTripwireInfoGlitchIsInside->papGrateEntities[ i ];
				break;
			}
			nCurrentNonDestructedGrate++;
		}
	}

	// Use this pointer as the position to return to corrosive.
	if( pNonDestructedGrate ) {
		m_vPeerLocation = pNonDestructedGrate->MtxToWorld()->m_vPos;
		// Check Corrosives aggression factor here... if it's larger than _SKIP_PEER_AGGRESSION_FACTOR, 
		// skip the peer and just get pissed...!
		f32 fAggression = CCorrosiveCombat::GetAggressiveness();
		if( fAggression > m_fSkipPeerAggressionThreshold ) {
			CCorrosiveCombat::AttackThisLocation( &m_vPeerLocation, TRUE );
		} else {
			CCorrosiveCombat::PeerAtThisLocation( &m_vPeerLocation, TRUE );
		}
	}

	return TRUE;
}


void CGeneralCorrosiveGame::Save( s32 nCheckpointID ) {
	if( m_pGeneralCorrosiveGame ) {
		m_pGeneralCorrosiveGame->m_hSaveData[ nCheckpointID ] = CFCheckPoint::CreateObjectDataHandle();

		CFCheckPoint::SaveData( m_pGeneralCorrosiveGame->m_nChipsCollected );
		CFCheckPoint::SaveData( (u32 &)m_pGeneralCorrosiveGame->m_pTrenchTripwireInfoGlitchIsInside );
		CFCheckPoint::SaveData( m_pGeneralCorrosiveGame->m_bGlitchInPossessionOfCorrosive );
		CFCheckPoint::SaveData( m_pGeneralCorrosiveGame->m_nNumTimesCorrosiveHasBeenHit );
		CFCheckPoint::SaveData( CCorrosiveCombat::GetAggressiveness() );
	}
}


void CGeneralCorrosiveGame::Restore( s32 nCheckpointID ) {
	f32 fTempAggression;
	if( m_pGeneralCorrosiveGame ) {
		CFCheckPoint::SetObjectDataHandle( m_pGeneralCorrosiveGame->m_hSaveData[ nCheckpointID ] );

		CFCheckPoint::LoadData( m_pGeneralCorrosiveGame->m_nChipsCollected );
		CFCheckPoint::LoadData( (u32 &)m_pGeneralCorrosiveGame->m_pTrenchTripwireInfoGlitchIsInside );
		CFCheckPoint::LoadData( m_pGeneralCorrosiveGame->m_bGlitchInPossessionOfCorrosive );
		CFCheckPoint::LoadData( m_pGeneralCorrosiveGame->m_nNumTimesCorrosiveHasBeenHit );
		CFCheckPoint::LoadData( fTempAggression );

		// Reset corrosives aggression factor
		CCorrosiveCombat::SetAggressiveness( fTempAggression );
	}
}


void CGeneralCorrosiveGame::_Work( void ) {

	// Gradually increment corrosives aggression factor... 
	CFVec3A vGlitchPos;
	f32 fAggression = CCorrosiveCombat::GetAggressiveness();
	fAggression += m_fAggressionPerSecondIncrementFactor * FLoop_fPreviousLoopSecs;
	FMATH_CLAMP( fAggression, m_fMinAggressionLevel, m_fMaxAggressionLevel );
	CCorrosiveCombat::SetAggressiveness( fAggression );

	// Next, check to see if we should be forcing corrosive to be in a range attack...
	// Conditions for forcing corrosive into a range attack:
	// 1) Glitch is in a trench, or
	// 2) Glitch in the central tower or on it's catwalks, and at least Mid catwalk height up

	vGlitchPos.Set( m_pBotGlitch->MtxToWorld()->m_vPos );
	if( m_pTrenchTripwireInfoGlitchIsInside || 
		( ( vGlitchPos.MagSq() < ( _CENTRAL_TOWER_RADIUS_SQ ) ) && ( vGlitchPos.y >= _CENTRAL_TOWER_RANGEATTACK_ALTITUDE ) ) ) {
		CCorrosiveCombat::ForceRangeAttack( TRUE );
	} else {
		CCorrosiveCombat::ForceRangeAttack( FALSE );
	}

#if _DRAW_DEBUG_SPHERES == 1
	u32 i;
	for( i = 0; i < 4; i++ ) {
		fdraw_DevSphere( &m_apChipCrates[ i ]->MtxToWorld()->m_vPos.v3, 7.0f, &FColor_MotifRed );
	}
#endif

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	//ftext_DebugPrintf( 0.2f, 0.6f, "~w1SBG working: %d", FLoop_nTotalLoopSecs );
#endif
	return;

#if SAS_ACTIVE_USER == SAS_USER_RUSS
	_DebugFn();
#endif
}


#if SAS_ACTIVE_USER == SAS_USER_RUSS
void CGeneralCorrosiveGame::_DebugFn( void ) {
}
#endif


