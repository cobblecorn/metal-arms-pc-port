//////////////////////////////////////////////////////////////////////////////////////
// Difficulty.cpp - Difficulty module.
//
// Author: Steve Ranck     
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
// 06/02/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "difficulty.h"
#include "fgamedata.h"


#define _CSV_FILENAME	"Difficulty"
#define _CSV_TABLE_NAME	"Diff"


u32 CDifficulty::m_nDifficultyLevel = CDifficulty::DEFAULT_DIFFICULTY_LEVEL;
CDifficulty::CInfo CDifficulty::m_aInfo[CDifficulty::DIFFICULTY_COUNT];
CDifficulty::CInfo *CDifficulty::m_pInfo = &m_aInfo[CDifficulty::DEFAULT_DIFFICULTY_LEVEL];

#if !FANG_PRODUCTION_BUILD
	cchar *CDifficulty::m_apszDevModeEnglishName[DIFFICULTY_COUNT] = {
		"Easy",
		"Normal",
		"Hard",
		"Nuts of Steel",
	};
#endif


const FGameData_TableEntry_t CDifficulty::m_aGameDataVocab[] = {
	// m_fDeltaUnitSusceptabilityPlayer:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_1,

	// m_fDeltaUnitSusceptabilityNPC:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_1,


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


BOOL CDifficulty::LoadData( void ) {
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	u32 nStructElementCount, nTableFieldCount, nArrayElementCount;

	CDifficulty::Reset();

	FMemFrame_t MemFrame = fmem_GetFrame();

	hFile = fgamedata_LoadFileToFMem( _CSV_FILENAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CDifficulty::LoadData(): Could not load difficulty file '%s.csv'.\n", _CSV_FILENAME );
		goto _ExitWithError;
	}

	hTable = fgamedata_GetFirstTableHandle( hFile, _CSV_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CDifficulty::LoadData(): Could not find table '%s' in '%s.csv'.\n", _CSV_TABLE_NAME, _CSV_FILENAME );
		goto _ExitWithError;
	}

	nStructElementCount = sizeof( m_aGameDataVocab ) / sizeof( FGameData_TableEntry_t );
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;

	nTableFieldCount = fgamedata_GetNumFields( hTable );
	if( (nTableFieldCount % nStructElementCount) ) {
		DEVPRINTF( "CDifficulty::LoadData(): Invalid number of fields in '%s.csv'. Must be a multiple of %u.\n", _CSV_FILENAME, nStructElementCount );
		goto _ExitWithError;
	}

	nArrayElementCount = nTableFieldCount / nStructElementCount;

	if( nArrayElementCount != DIFFICULTY_COUNT ) {
		DEVPRINTF( "CDifficulty::LoadData(): Invalid number of difficulty levels in '%s.csv'. Must be %u.\n", _CSV_FILENAME, DIFFICULTY_COUNT );
		goto _ExitWithError;
	}

	if( !fgamedata_GetTableData_ArrayOfStructures( hTable, m_aGameDataVocab, m_aInfo, sizeof(CInfo), nArrayElementCount ) ) {
		DEVPRINTF( "CDifficulty::LoadData(): Trouble parsing difficulty info data for '%s.csv'.\n", _CSV_FILENAME );
		goto _ExitWithError;
	}

	// Success...

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:
	_InitAllDefaultInfoData();
	fmem_ReleaseFrame( MemFrame );

	return FALSE;
}


void CDifficulty::_InitAllDefaultInfoData( void ) {
	u32 i;

	fang_MemZero( m_aInfo, sizeof(CInfo) * DIFFICULTY_COUNT );

	for( i=0; i<DIFFICULTY_COUNT; ++i ) {
		_InitDefaultInfoData( &m_aInfo[i] );
	}
}


void CDifficulty::_InitDefaultInfoData( CInfo *pInfo ) {
	// Add non-zero defaults here...
}


void CDifficulty::Reset( void ) {
	FASSERT( DEFAULT_DIFFICULTY_LEVEL < DIFFICULTY_COUNT );

	m_nDifficultyLevel = DEFAULT_DIFFICULTY_LEVEL;
	m_pInfo = &m_aInfo[m_nDifficultyLevel];
}


void CDifficulty::SetLevel( u32 nDifficultyLevel ) {
	FASSERT( nDifficultyLevel < DIFFICULTY_COUNT );

	m_nDifficultyLevel = nDifficultyLevel;
	m_pInfo = &m_aInfo[m_nDifficultyLevel];
}

