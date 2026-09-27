//////////////////////////////////////////////////////////////////////////////////////
// fsound.cpp - General purpose sound functions.
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
// 03/2/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fsound.h"
#include "fgamedata.h"
#include "fstringtable.h"
#include "fclib.h"




typedef struct {
	cchar *pszTagName;		// The sound tag name
	f32 fVolume2D;			// 2D volume level (0=off, 100=max)
	f32 fVolume3D;			// 3D volume level (0=off, 100=max)
	f32 fRadius3D;			// 3D radius (0=off)
	f32 fMinPitchMult;		// Minimum pitch multiplier (0=min, 255=max)
	f32 fMaxPitchMult;		// Maximum pitch multiplier (0=min, 255=max)
	f32 fRadiusAI;			// AI radius
	cchar *pszTypeAI;		// AI sound type (game specific)
} _SoundInfoData_t;


const FGameData_TableEntry_t _aUserPropVocab[] = {
	// pszTagName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fVolume2D:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// fVolume3D:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	// fRadius3D:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_1000000000,


	// fRadiusAI:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_1000000000,

	// fMinPitchMult:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_255,

	// fMaxPitchMult:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_255,

	// pszTypeAI:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFSoundGroup
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
BOOL CFSoundGroup::m_bModuleStartedUp;
CFSoundGroup *CFSoundGroup::m_pGroupBase;
u32 CFSoundGroup::m_nGroupCount;



BOOL CFSoundGroup::ModuleStartup( void ) {
	FASSERT( !IsModuleStartedUp() );

	m_pGroupBase = NULL;
	m_nGroupCount = 0;

	m_bModuleStartedUp = TRUE;

	return TRUE;
}


void CFSoundGroup::ModuleShutdown( void ) {
	if( m_bModuleStartedUp ) {
		m_pGroupBase = NULL;
		m_nGroupCount = 0;

		m_bModuleStartedUp = FALSE;
	}
}


BOOL CFSoundGroup::BuildGroupsFromFile( cchar *pszSoundFile, FSoundNotifyAICallback_t *pFcnNotifyAI, FSoundConvertAICallback_t *pFcnConvertAI ) {
	FASSERT( IsModuleStartedUp() );
	FASSERT( pszSoundFile );

	FGameDataFileHandle_t hGameDataFile;
	FGameDataTableHandle_t hTable;
	FGameDataWalker_t Walker;
	CFSoundGroup *pGroup;

	FResFrame_t ResFrame = fres_GetFrame();
	FMemFrame_t MemFrame = fmem_GetFrame();

	#if !FANG_PRODUCTION_BUILD
		u32 nFreeBytes = fres_GetFreeBytes();
	#endif

	m_pGroupBase = NULL;
	m_nGroupCount = 0;
	CFSoundInfo::m_pFcnNotifyAI = pFcnNotifyAI;

	hGameDataFile = fgamedata_LoadFileToFMem( pszSoundFile );
	if( hGameDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFSoundGroup::BuildGroupsFromFile(): Unable to load file '%s'.\n", pszSoundFile );
		goto _ExitWithError;
	}

	if( !_AllocateMemoryForGroups( hGameDataFile, pszSoundFile ) ) {
		goto _ExitWithError;
	}

	pGroup = m_pGroupBase;

	for( hTable = fgamedata_GetFirstTable( hGameDataFile, Walker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( Walker ) ) {
		// Build this group...
		if( !_BuildGroup( &pGroup, pszSoundFile, hTable, pFcnConvertAI ) ) {
			goto _ExitWithError;
		}
	}

	fmem_ReleaseFrame( MemFrame );

	#if !FANG_PRODUCTION_BUILD
		DEVPRINTF( "Sound groups used %u bytes of memory for %u sound groups.\n", nFreeBytes - fres_GetFreeBytes(), m_nGroupCount );
	#endif

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );

	m_pGroupBase = NULL;

	return FALSE;
}


BOOL CFSoundGroup::_AllocateMemoryForGroups( FGameDataFileHandle_t hGameDataFile, cchar *pszSoundFile ) {
	FGameDataTableHandle_t hTable;
	FGameDataWalker_t Walker;
	cchar *pszTableName;
	u32 nStructElementCount, nTableFieldCount, nEntryCount, nBytesToAlloc;

	m_pGroupBase = NULL;
	m_nGroupCount = 0;
	nBytesToAlloc = 0;
	
	u32 nSizeofUserProp = sizeof( _aUserPropVocab );
	u32 nSizeofTableEntry = sizeof( FGameData_TableEntry_t );

	nStructElementCount = nSizeofUserProp / nSizeofTableEntry;
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;

	for( hTable = fgamedata_GetFirstTable( hGameDataFile, Walker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( Walker ) ) {
		pszTableName = fgamedata_GetTableName( hTable );

		nTableFieldCount = fgamedata_GetNumFields( hTable );
		if( nTableFieldCount % nStructElementCount ) {
			DEVPRINTF( "CFSoundGroup::_AllocateMemoryForGroups(): Invalid number of fields table '%s' of file '%s.csv'.\n", pszTableName, pszSoundFile );
			goto _ExitWithError;
		}

		nEntryCount = nTableFieldCount / nStructElementCount;

		if( nEntryCount > 255 ) {
			DEVPRINTF( "CFSoundGroup::_AllocateMemoryForGroups(): Sound info count exceeds 255 in file '%s'.\n", pszSoundFile );
			goto _ExitWithError;
		}

		nBytesToAlloc += sizeof(CFSoundGroup) + nEntryCount*sizeof(CFSoundInfo);
		++m_nGroupCount;
	}

	m_pGroupBase = (CFSoundGroup *)fres_AllocAndZero( nBytesToAlloc );
	if( m_pGroupBase == NULL ) {
		DEVPRINTF( "CFSoundGroup::_AllocateMemoryForGroups(): Not enough memory to allocate sound groups.\n" );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}


BOOL CFSoundGroup::_BuildGroup( CFSoundGroup **ppGroup, cchar *pszSoundFile, FGameDataTableHandle_t hTable, FSoundConvertAICallback_t *pFcnConvertAI ) {
	_SoundInfoData_t *pSoundInfoData;
	CFSoundInfo *pSoundInfoArray;
	CFSoundGroup *pGroup;
	cchar *pszTableName;
	f32 fTemp;
	u32 i, nStructElementCount, nTableFieldCount, nEntryCount;

	FMemFrame_t MemFrame = fmem_GetFrame();

	pGroup = *ppGroup;

	pszTableName = fgamedata_GetTableName( hTable );

	nStructElementCount = sizeof( _aUserPropVocab ) / sizeof( FGameData_TableEntry_t );
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;

	nTableFieldCount = fgamedata_GetNumFields( hTable );
	FASSERT( (nTableFieldCount % nStructElementCount) == 0 );

	nEntryCount = nTableFieldCount / nStructElementCount;
	FASSERT( nEntryCount <= 255 );

	// Note that this is going in temp memory...
	pSoundInfoData = (_SoundInfoData_t *)fmem_Alloc( sizeof(_SoundInfoData_t) * nEntryCount );
	if( pSoundInfoData == NULL ) {
		DEVPRINTF( "CFSoundGroup::_BuildGroup(): Not enough memory to create _SoundInfoData_t array.\n" );
		goto _ExitWithError;
	}

	if( !fgamedata_GetTableData_ArrayOfStructures( hTable, _aUserPropVocab, pSoundInfoData, sizeof(_SoundInfoData_t), nEntryCount ) ) {
		DEVPRINTF( "CFSoundGroup::_BuildGroup(): Problem encountered while parsing table '%s' of file '%s.csv'.\n", pszTableName, pszSoundFile );
		goto _ExitWithError;
	}

	pGroup->m_pszGroupName = CFStringTable::AddString( NULL, pszTableName );
	if( pGroup->m_pszGroupName == NULL ) {
		DEVPRINTF( "CFSoundGroup::_BuildGroup(): Not enough memory to add string to string table.\n" );
		goto _ExitWithError;
	}

	pGroup->m_nSoundInfoCount = nEntryCount;
	pGroup->m_nLastSoundInfoIndexPlayed = (pGroup->m_nSoundInfoCount > 0) ? pGroup->m_nSoundInfoCount - 1 : 0;

	pSoundInfoArray = (CFSoundInfo *)( pGroup + 1 );

	for( i=0; i<nEntryCount; ++i ) {
		pSoundInfoArray[i].m_pszSoundTagName = pSoundInfoData[i].pszTagName;
		pSoundInfoArray[i].m_hSound = 0;
		pSoundInfoArray[i].m_nVolume2D = (u8)(pSoundInfoData[i].fVolume2D * 255.0f);
		pSoundInfoArray[i].m_nVolume3D = (u8)(pSoundInfoData[i].fVolume3D * 255.0f);

		fTemp = (pSoundInfoData[i].fRadius3D + 9.9f) * 0.1f;
		FMATH_CLAMP( fTemp, 0.0f, 255.0f );
		pSoundInfoArray[i].m_nDeciRadius3D = (u8)fTemp;

		fTemp = (pSoundInfoData[i].fRadiusAI + 9.9f) * 0.1f;
		FMATH_CLAMP( fTemp, 0.0f, 255.0f );
		pSoundInfoArray[i].m_nDeciRadiusAI = (u8)fTemp;

		pSoundInfoArray[i].m_nMinPitchPercent = (u8)(pSoundInfoData[i].fMinPitchMult);
		pSoundInfoArray[i].m_nMaxPitchPercent = (u8)(pSoundInfoData[i].fMaxPitchMult);

		if( pFcnConvertAI ) {
			pSoundInfoArray[i].m_nTypeAI = pFcnConvertAI( pSoundInfoData[i].pszTypeAI );
		} else {
			pSoundInfoArray[i].m_nTypeAI = 0;
		}
	}

	fmem_ReleaseFrame( MemFrame );

	*ppGroup = (CFSoundGroup *)( (u32)(pGroup + 1) + nEntryCount*sizeof(CFSoundInfo) );

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}


CFSoundGroup *CFSoundGroup::RegisterGroup( cchar *pszGroupName ) {
	CFSoundGroup *pGroup;

	pGroup = _FindGroup( pszGroupName );
	if( pGroup == NULL ) {
		return NULL;
	}

	CFSoundInfo *pSoundInfoArray;
	u32 i;

	pSoundInfoArray = (CFSoundInfo *)(pGroup + 1);

	for( i=0; i<pGroup->m_nSoundInfoCount; ++i ) {
		pSoundInfoArray[i].m_hSound = fsndfx_GetFxHandle( pSoundInfoArray[i].m_pszSoundTagName );

		if( pSoundInfoArray[i].m_hSound == 0 ) {
#if FANG_WINGC
			// The retail sound-group tables use 'None' for an empty slot; that one is no sound, not a
			// missing one.
			if( !fclib_stricmp( pSoundInfoArray[i].m_pszSoundTagName, "None" ) ) {
				continue;
			}
#endif
			DEVPRINTF( "CFSoundGroup::RegisterGroup(): Warning! Sound '%s' not found. Bank may not be loaded.\n", pSoundInfoArray[i].m_pszSoundTagName );
		}
	}

	pGroup->m_nLastSoundInfoIndexPlayed = (pGroup->m_nSoundInfoCount > 0) ? pGroup->m_nSoundInfoCount - 1 : 0;

	return pGroup;
}


CFSoundGroup *CFSoundGroup::_FindGroup( cchar *pszGroupName ) {
	if( m_pGroupBase == NULL ) {
		return NULL;
	}

	if( pszGroupName == NULL ) {
		return NULL;
	}

	CFSoundGroup *pGroup;
	u32 i;

	pGroup = m_pGroupBase;

	for( i=0; i<m_nGroupCount; ++i ) {
		if( !fclib_stricmp( pszGroupName, pGroup->m_pszGroupName ) ) {
			// Found the group...
			return pGroup;
		}

		// Step to the next group...

		pGroup = (CFSoundGroup *)( (u32)pGroup + sizeof(CFSoundGroup) + pGroup->m_nSoundInfoCount*sizeof(CFSoundInfo) );
	}

	return NULL;
}


CFSoundInfo *CFSoundGroup::GetSoundInfo( CFSoundGroup *pGroup, u32 nSoundInfoIndex ) {
	if( pGroup == NULL ) {
		// No sound group...
		return NULL;
	}

	if( pGroup->m_nSoundInfoCount == 0 ) {
		// No sound info's to choose from...
		return NULL;
	}

	return ((CFSoundInfo *)( (u32)( pGroup + 1 ) ));
}


FSndFx_FxHandle_t CFSoundGroup::GetSoundHandle( CFSoundGroup *pGroup, u32 nSoundInfoIndex ) {
	const CFSoundInfo *pSoundInfo = GetSoundInfo( pGroup, nSoundInfoIndex );

	if( pSoundInfo == NULL ) {
		return 0;
	}

	return pSoundInfo->m_hSound;
}


CFSoundInfo *CFSoundGroup::GetRandomSoundInfo( CFSoundGroup *pGroup, CFSoundInfo::DecompressedSoundInfo_t *pDestDecompressedSoundInfo ) {
	if( pGroup == NULL ) {
		// No sound group...
		return NULL;
	}

	if( pGroup->m_nSoundInfoCount == 0 ) {
		// No sound info's to choose from...
		return NULL;
	}

	CFSoundInfo *pSoundInfo = ((CFSoundInfo *)( (u32)( pGroup + 1 ) ));

	if( pGroup->m_nSoundInfoCount > 1 ) {
		// More than one sound info to choose from...

		u32 nSoundInfoIndex = fmath_RandomChoice( pGroup->m_nSoundInfoCount - 1 );

		if( nSoundInfoIndex >= pGroup->m_nLastSoundInfoIndexPlayed ) {
			++nSoundInfoIndex;
		}

		FASSERT( nSoundInfoIndex < pGroup->m_nSoundInfoCount );

		pSoundInfo = &pSoundInfo[ nSoundInfoIndex ];

		pGroup->m_nLastSoundInfoIndexPlayed = nSoundInfoIndex;
	}

	if( pDestDecompressedSoundInfo ) {
		pSoundInfo->Decompress( pDestDecompressedSoundInfo );
	}

	return pSoundInfo;
}


FSndFx_FxHandle_t CFSoundGroup::GetRandomSoundHandle( CFSoundGroup *pGroup ) {
	const CFSoundInfo *pSoundInfo = GetRandomSoundInfo( pGroup );

	if( pSoundInfo == NULL ) {
		return 0;
	}

	return pSoundInfo->m_hSound;
}


void CFSoundGroup::PlaySound( CFSoundGroup *pGroup, BOOL bPlay2D, const CFVec3A *pPos_WS, s32 nPlayerIndex, BOOL bInformAI, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride, CFAudioEmitter **ppUserAudioEmitter ) {
	CFSoundInfo *pSoundInfo = GetRandomSoundInfo( pGroup );

	if( pSoundInfo == NULL ) {
		return;
	}

	pSoundInfo->PlaySound( bPlay2D, pPos_WS, nPlayerIndex, bInformAI, fVolMult, fPitchMult, fRadiusOverride, ppUserAudioEmitter );
}


CFAudioEmitter *CFSoundGroup::AllocAndPlaySound( CFSoundGroup *pGroup, BOOL bPlay2D, const CFVec3A *pPos_WS, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride,
												 f32 *pfChosenVolume, f32 *pfChosenPitchMult ) {
	CFSoundInfo *pSoundInfo = GetRandomSoundInfo( pGroup );

	if( pSoundInfo == NULL ) {
		return NULL;
	}

	return pSoundInfo->AllocAndPlaySound( bPlay2D, pPos_WS, fVolMult, fPitchMult, fRadiusOverride, pfChosenVolume, pfChosenPitchMult );
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFSoundInfo
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FSoundNotifyAICallback_t *CFSoundInfo::m_pFcnNotifyAI;



void CFSoundInfo::Decompress( DecompressedSoundInfo_t *pDecompressedSoundInfo ) const {
	pDecompressedSoundInfo->m_pszSoundTagName = m_pszSoundTagName;
	pDecompressedSoundInfo->m_hSound = m_hSound;
	pDecompressedSoundInfo->m_fUnitVol2D = (f32)m_nVolume2D * (1.0f / 255.0f);
	pDecompressedSoundInfo->m_fUnitVol3D = (f32)m_nVolume3D * (1.0f / 255.0f);
	pDecompressedSoundInfo->m_fRadius3D = (f32)m_nDeciRadius3D * 10.0f;
	pDecompressedSoundInfo->m_fRadiusAI = (f32)m_nDeciRadiusAI * 10.0f;
	pDecompressedSoundInfo->m_fMinPitchMult = (f32)m_nMinPitchPercent * 0.01f;
	pDecompressedSoundInfo->m_fMaxPitchMult = (f32)m_nMaxPitchPercent * 0.01f;
	pDecompressedSoundInfo->m_nTypeAI = m_nTypeAI;
}


void CFSoundInfo::PlaySound( BOOL bPlay2D, const CFVec3A *pPos_WS, s32 nPlayerIndex, BOOL bInformAI, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride, CFAudioEmitter **ppUserAudioEmitter ) const {
	DecompressedSoundInfo_t DecompressedSoundInfo;

	Decompress( &DecompressedSoundInfo );

	if( DecompressedSoundInfo.m_hSound ) {
		// There is a sound to play...

		f32 fPitchMultiplier = fPitchMult * fmath_RandomFloatRange( DecompressedSoundInfo.m_fMinPitchMult, DecompressedSoundInfo.m_fMaxPitchMult );

		if( bPlay2D ) {
			// Play a 2D sound...

			f32 fUnitVol = DecompressedSoundInfo.m_fUnitVol2D * fVolMult;

			if( fUnitVol > 0.0f ) {
				fsndfx_Play2D( DecompressedSoundInfo.m_hSound, fUnitVol, fPitchMultiplier, FAudio_EmitterDefaultPriorityLevel, 0.0f, TRUE, ppUserAudioEmitter );
			}
		} else {
			// Play a 3D sound...

			if( pPos_WS ) {
				// A 3D position has been provided...

				f32 fUnitVol = DecompressedSoundInfo.m_fUnitVol3D * fVolMult;

				if( fUnitVol > 0.0f ) {
					f32 fRadius3D;

					if( fRadiusOverride < 0.0f ) {
						// No radius override specified...
						fRadius3D = DecompressedSoundInfo.m_fRadius3D;
					} else {
						// A radius override has been specified...
						fRadius3D = fRadiusOverride;
					}

					if( fRadius3D > 0.0f ) {
						fsndfx_Play3D( DecompressedSoundInfo.m_hSound, pPos_WS, fRadius3D, 1.0f, fUnitVol, fPitchMultiplier, FAudio_EmitterDefaultPriorityLevel, TRUE, ppUserAudioEmitter );
					}
				}
			}
		}
	}

	if( bInformAI ) {
		if( m_pFcnNotifyAI ) {
			f32 fRadiusAI;

			if( fRadiusOverride < 0.0f ) {
				// No radius override specified...
				fRadiusAI = DecompressedSoundInfo.m_fRadiusAI;
			} else {
				// A radius override has been specified...

				if( DecompressedSoundInfo.m_fRadius3D > 0.01f ) {
					// Scale the AI's radius by the same ratio the 3D radius was scaled...
					fRadiusAI = DecompressedSoundInfo.m_fRadiusAI * fmath_Div( fRadiusOverride, DecompressedSoundInfo.m_fRadius3D );
				} else {
					// Just use the override value directly...
					fRadiusAI = fRadiusOverride;
				}
			}

			if( fRadiusAI > 0.0f ) {
				m_pFcnNotifyAI( pPos_WS, nPlayerIndex, DecompressedSoundInfo.m_nTypeAI, fRadiusAI );
			}
		}
	}
}


CFAudioEmitter *CFSoundInfo::AllocAndPlaySound( BOOL bPlay2D, const CFVec3A *pPos_WS, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride,
											    f32 *pfChosenVolume, f32 *pfChosenPitchMult ) const {
	DecompressedSoundInfo_t DecompressedSoundInfo;
	CFAudioEmitter *pAudioEmitter = NULL;
	f32 fPitchMultiplier = 1.0f;
	f32 fUnitVol = 0.0f;

	Decompress( &DecompressedSoundInfo );

	if( DecompressedSoundInfo.m_hSound ) {
		// There is a sound to play...

		fPitchMultiplier = fPitchMult * fmath_RandomFloatRange( DecompressedSoundInfo.m_fMinPitchMult, DecompressedSoundInfo.m_fMaxPitchMult );

		if( bPlay2D ) {
			// Play a 2D sound...

			fUnitVol = DecompressedSoundInfo.m_fUnitVol2D * fVolMult;

			pAudioEmitter = FSNDFX_ALLOCNPLAY2D( DecompressedSoundInfo.m_hSound, fUnitVol, fPitchMultiplier, FAudio_EmitterDefaultPriorityLevel, 0.0f, TRUE );
		} else {
			// Play a 3D sound...

			if( pPos_WS ) {
				// A 3D position has been provided...

				f32 fRadius3D;

				fUnitVol = DecompressedSoundInfo.m_fUnitVol3D * fVolMult;

				if( fRadiusOverride < 0.0f ) {
					// No radius override specified...
					fRadius3D = DecompressedSoundInfo.m_fRadius3D;
				} else {
					// A radius override has been specified...
					fRadius3D = fRadiusOverride;
				}

				pAudioEmitter = FSNDFX_ALLOCNPLAY3D( DecompressedSoundInfo.m_hSound, pPos_WS, fRadius3D, fUnitVol, fPitchMultiplier, FAudio_EmitterDefaultPriorityLevel, TRUE );
			}
		}
	}

	if( pfChosenVolume ) {
		*pfChosenVolume = fUnitVol;
	}

	if( pfChosenPitchMult ) {
		*pfChosenPitchMult = fPitchMultiplier;
	}

	return pAudioEmitter;
}


