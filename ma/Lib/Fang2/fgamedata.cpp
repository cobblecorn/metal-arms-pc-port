//////////////////////////////////////////////////////////////////////////////////////
// fgamedata.cpp - 
//
// Author: Michael Starich   
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
// 07/30/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fgamedata.h"
#include "fclib.h"
#include "fmath.h"
#include "ffile.h"
#include "fres.h"
#include "fdata.h"
#include "fresload.h"
#include "flinklist.h"
#include "fstringtable.h"
#include "fsndfx.h"
#include "ftex.h"
#include "fparticle.h"
#include "fdebris.h"
#include "fsound.h"
#include "fexplosion.h"
#include "fsndfx.h"
#include "fdecal.h"
#include "gcdata.h"



//====================
// private definitions

typedef struct {
	char szFilename[FDATA_PRJFILE_FILENAME_LEN];
	FDataGamFile_Header_t *pHeader;

	FLink_t Link;
} _FMemFile_t;

// macros to go from handles to pointers
#define _FH_2_HP( hFile )		( (FDataGamFile_Header_t *)hFile )
#define _TH_2_TP( hTable )		( (FDataGamFile_Table_t *)hTable )
// macros to go from pointers to handles
#define _HP_2_FH( pHeader )		( (FGameDataFileHandle_t)pHeader )
#define _TP_2_TH( pTable )		( (FGameDataTableHandle_t)pTable )

#define _NOT_A_KEYWORD_TABLE	-1

//=================
// public variables
f32 afDataTable[F32_DATATABLE_COUNT] = {
	0.0f,			//	F32_DATATABLE_0, 
	0.0000001f,		//  F32_DATATABLE_Pt0000001
	0.00001f,		//  F32_DATATABLE_Pt00001
	0.0001f,		//  F32_DATATABLE_Pt0001
	0.001f,			//	F32_DATATABLE_Pt001,
	0.01f,			//  F32_DATATABLE_Pt01,
	0.1f,			//  F32_DATATABLE_Pt1,
	0.95f,			//  F32_DATATABLE_Pt95
   -1.0f,			//  F32_DATATABLE_Neg1
	1.0f,			//	F32_DATATABLE_1,
	2.0f,			//	F32_DATATABLE_2,
	3.0f,			//	F32_DATATABLE_3,
	5.0f,			//	F32_DATATABLE_5,
	6.0f,			//	F32_DATATABLE_6,
	10.0f,			//	F32_DATATABLE_10,
	15.0f,			//	F32_DATATABLE_15,
	20.0f,			//	F32_DATATABLE_20,
	30.0f,			//	F32_DATATABLE_30,
	50.0f,			//	F32_DATATABLE_50,
	60.0f,			//	F32_DATATABLE_60,
   -100.0f,			//  F32_DATATABLE_Neg100,
	100.0f,			//  F32_DATATABLE_100,
	255.0f,			//  F32_DATATABLE_255
	500.0f,			//  F32_DATATABLE_500
   -1000.0f,		//  F32_DATATABLE_Neg1000,
	1000.0f,		//  F32_DATATABLE_1000,
	5000.0f,		//  F32_DATATABLE_5000,
   -10000.0f,		//  F32_DATATABLE_Neg10000
	10000.0f,		//  F32_DATATABLE_10000
	65534.0f,		//  F32_DATATABLE_65534,
	65535.0f,		//  F32_DATATABLE_65535,
	100000.0f,		//  F32_DATATABLE_100000
	10000000.0f,	//  F32_DATATABLE_10000000
   -1000000000.0f,  //	F32_DATATABLE_Neg1000000000,
	1000000000.0f,  //	F32_DATATABLE_1000000000,
	4294967295.0f,	//  F32_DATATABLE_4294967295,
	100000000000.0f //  F32_DATATABLE_100000000000
};


//==================
// private variables

static BOOL _bModuleInited = FALSE;
static FResLoadReg_t _ResLoadRegistration;
static FLinkRoot_t _FMemFilesLL;
static cchar *_pszKeyword;
static char _szFilename[FDATA_PRJFILE_FILENAME_LEN];
static FGameDataDamageConverterCallback_t *_pFcnDamageConverter;
static FGameDataArmorConverterCallback_t *_pFcnArmorConverter;

//===================
// private prototypes

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static BOOL _FixupPtrsAndValidateData( FDataGamFile_Header_t *pHeader );
static BOOL _DoFieldAndEntryTypesMatch( FDataGamFile_Field_t *pField, const FGameData_TableEntry_t *pEntry ); 
static u32 _ComputeBytesNeededForEntry( FDataGamFile_Field_t *pField, const FGameData_TableEntry_t *pEntry );
static u32 _CopyField( FDataGamFile_Field_t *pField, const FGameData_TableEntry_t *pEntry, void *pDest );
static FGameDataFileHandle_t _GetFileHandleFromTableHandle( FGameDataTableHandle_t hTableHandle );
static void _CreateFilenameString( BOOL bFresTypeString, cchar *pszFilename );
static BOOL _DoesTableNameMatchKeyword( FDataGamFile_Table_t *pTable );
static FGameDataFileHandle_t _LoadFieldStringCsvFile( FDataGamFile_Table_t *pTable, u32 nFieldIndex );
static FGameDataTableHandle_t _GetFirstNonKeywordTable( FGameDataWalker_t &rWalker );
static void _PopCurrentTableFromStack( FGameDataWalker_t &rWalker );

//=================
// public functions

/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL fgamedata_ModuleStartup( void ) {

	_pszKeyword = "include";
	flinklist_InitRoot( &_FMemFilesLL, FANG_OFFSETOF( _FMemFile_t, Link ) );

	// register our fres loader function
	fres_CopyType( _ResLoadRegistration.sResType, FGAMEDATA_RESTYPE );
	_ResLoadRegistration.pszFileExtension = "csv";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_PERM;
	_ResLoadRegistration.nAlignment = 4;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = NULL;

	if( !fresload_RegisterHandler( &_ResLoadRegistration ) ) {
		// Registration failed...
		DEVPRINTF( "fgamedata_ModuleStartup(): Could not register resource.\n" );
		return FALSE;
	}

	_pFcnDamageConverter = NULL;

	_bModuleInited = TRUE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void fgamedata_ModuleShutdown( void ) {
	_bModuleInited = FALSE;	
	_pszKeyword = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void fgamedata_SetDamageConverterCallback( FGameDataDamageConverterCallback_t *pFcnDamageConverter ) {
	_pFcnDamageConverter = pFcnDamageConverter;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataDamageConverterCallback_t *fgamedata_GetDamageConverterCallback( void ) {
	return _pFcnDamageConverter;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void fgamedata_SetArmorConverterCallback( FGameDataArmorConverterCallback_t *pFcnArmorConverter ) {
	_pFcnArmorConverter = pFcnArmorConverter;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataArmorConverterCallback_t *fgamedata_GetArmorConverterCallback( void ) {
	return _pFcnArmorConverter;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void fgamedata_MemFrameReleased( void *pReleasedMem ) {
	_FMemFile_t *pFile, *pNext;

	if( _bModuleInited ) {
		pFile = (_FMemFile_t *)flinklist_GetHead( &_FMemFilesLL );
		while( pFile ) {
			pNext = (_FMemFile_t *)flinklist_GetNext( &_FMemFilesLL, pFile );

			// see if we should remove pFile
			if( (void *)pFile >= pReleasedMem ) {
				// remove and mark as unusable
				pFile->pHeader->nFlags = 0;
				flinklist_Remove( &_FMemFilesLL, pFile );
			}
			pFile = pNext;
		}
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void fgamedata_SetTableIncludeKeyword( cchar *pszKeyword ) {
	_pszKeyword = pszKeyword;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

cchar *fgamedata_GetTableIncludeKeyworld( void ) {
	return _pszKeyword;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataFileHandle_t fgamedata_LoadFileToFMem( cchar *pszFileName ) {
	FFileHandle hFile;
	FDataGamFile_Header_t *pHeader;
	_FMemFile_t *pFile;

	if( !pszFileName ) {
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}

	_CreateFilenameString( FALSE, pszFileName );

	// see if we have already loaded this file
	pFile = (_FMemFile_t *)flinklist_GetHead( &_FMemFilesLL );
	while( pFile ) {
		// see if our names match
		if( fclib_stricmp( pFile->szFilename, _szFilename ) == 0 ) {
			// we found the file we were looking for
			return _HP_2_FH( pFile->pHeader );
		}
		pFile = (_FMemFile_t *)flinklist_GetNext( &_FMemFilesLL, pFile );
	}

	hFile = ffile_Open( _szFilename, FFILE_OPEN_RONLY );
	if( !FFILE_IS_VALID_HANDLE( hFile ) ) {
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}

	u32 nFilesize = ffile_GetFileSize( hFile );
	if( !nFilesize ) {
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}

	// grab a frame
	FMemFrame_t Frame = fmem_GetFrame();

	pFile = (_FMemFile_t *)fmem_AllocAndZero( nFilesize + sizeof( _FMemFile_t ), 4 );
	if( !pFile ) {
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}
	pHeader = (FDataGamFile_Header_t *)&pFile[1];
	
	if( ffile_Read( hFile, nFilesize, pHeader ) < 0 ) {
		fmem_ReleaseFrame( Frame );
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}
	if( ffile_Close( hFile ) < 0 ) {
		fmem_ReleaseFrame( Frame );
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}
	const char *pExt = strrchr( _szFilename, '.' );
	if( pExt && fclib_stricmp( pExt + 1, "csv" ) == 0 &&
		!gcdata_Convert( "csv", _szFilename, pHeader, nFilesize ) )
	{
		fmem_ReleaseFrame( Frame );
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}

	// make sure that there is at least 1 table in the file
	if( pHeader->nNumTables == 0 ) {
		// there are no tables in this file, fail the load
		fmem_ReleaseFrame( Frame );
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}

	// fixup all of the pointers
	if( !_FixupPtrsAndValidateData( pHeader ) ) {
		fmem_ReleaseFrame( Frame );
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}
	// set some flags
	pHeader->nFlags &= ~FDATA_GAMFILE_FLAGS_UNKNOWN_ORIGIN;
	pHeader->nFlags |= FDATA_GAMFILE_FLAGS_FMEM_LOADED;

	// add our self to the link list
	pFile->pHeader = pHeader;
	fclib_strncpy( pFile->szFilename, _szFilename, FDATA_PRJFILE_FILENAME_LEN );
	flinklist_AddTail( &_FMemFilesLL, pFile );

	return _HP_2_FH( pHeader );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataFileHandle_t fgamedata_GetHandleFromLoadedFile( void *pLoadedFile ) {

	if( !pLoadedFile ) {
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}
	
	FDataGamFile_Header_t *pHeader = (FDataGamFile_Header_t *)pLoadedFile;

	if( pHeader->nFlags & FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP ) {
		// nothing to fix up, just return our ptr
		return _HP_2_FH( pHeader );
	}

	// fixup all of the pointers
	if( !_FixupPtrsAndValidateData( pHeader ) ) {
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}

	// set some flags
	pHeader->nFlags &= ~FDATA_GAMFILE_FLAGS_FMEM_LOADED;
	pHeader->nFlags |= FDATA_GAMFILE_FLAGS_UNKNOWN_ORIGIN;

	return _HP_2_FH( pHeader );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

cchar *fgamedata_GetFileNameFromFileHandle( FGameDataFileHandle_t hFileHandle ) {
	
	// check for a valid file handle
	if( hFileHandle == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "fgamedata_GetFileNameFromFileHandle(): Invalid file handle, you must load a file and pass in the returned handle.\n" );
		return NULL;
	}

	// grab a ptr to our data header
	FDataGamFile_Header_t *pHeader = _FH_2_HP( hFileHandle );
	FASSERT( pHeader->nFlags & FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP );

	if( pHeader->nFlags & FDATA_GAMFILE_FLAGS_UNKNOWN_ORIGIN ) {
		// no name associated with this file
		return NULL;
	} else if( pHeader->nFlags & FDATA_GAMFILE_FLAGS_FMEM_LOADED ) {
		// search our fmem link list for this load
		_FMemFile_t *pFile;

		pFile = (_FMemFile_t *)flinklist_GetHead( &_FMemFilesLL );
		while( pFile ) {
			if( pFile->pHeader == pHeader ) {
				return pFile->szFilename;
			}
			pFile = (_FMemFile_t *)flinklist_GetNext( &_FMemFilesLL, pFile );
		}
	} else {
		// let fres search for this load
		FResHandle_t hRes = fres_FindHandle( pHeader );
		if( hRes != FRES_NULLHANDLE ) {
			return fres_GetName( hRes );
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataTableHandle_t fgamedata_GetFirstTableHandle( FGameDataFileHandle_t hFileHandle, cchar *pszTableName ) {
	FGameDataWalker_t Walker;
	FDataGamFile_Table_t *pTable;
	FGameDataTableHandle_t hDesiredTable = FGAMEDATA_INVALID_TABLE_HANDLE;

	if( hFileHandle == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "fgamedata_GetFirstTableHandle(): Invalid file handle, you must load a file and pass in the returned handle.\n" );
		return FGAMEDATA_INVALID_TABLE_HANDLE;
	}

	FGameDataTableHandle_t hTableHandle = fgamedata_GetFirstTable( hFileHandle, Walker );
	while( hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		pTable = _TH_2_TP( hTableHandle );
		if( fclib_stricmp( pTable->pszKeyString, pszTableName ) == 0 ) {
			hDesiredTable = _TP_2_TH( pTable );
			break;
		}
		hTableHandle = fgamedata_GetNextTable( Walker );
	}

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
		if( hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE ) {
			// Found a valid table. Let's see if it exists more than once...

			FGameDataTableHandle_t hDupCheckTable = hTableHandle;

			hDupCheckTable = fgamedata_GetNextTable( Walker );

			while( hDupCheckTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
				pTable = _TH_2_TP( hDupCheckTable );
				if( fclib_stricmp( pTable->pszKeyString, pszTableName ) == 0 ) {
					DEVPRINTF( "fgamedata_GetFirstTableHandle(): Duplicate table '%s' found in '%s'. Using first occurrence.\n", pszTableName, fgamedata_GetFileNameFromFileHandle(hFileHandle) );
					break;
				}
				hDupCheckTable = fgamedata_GetNextTable( Walker );
			}
		}
	#endif

	return hDesiredTable;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataTableHandle_t fgamedata_GetLastTableHandle( FGameDataFileHandle_t hFileHandle, cchar *pszTableName ) {
	FGameDataWalker_t Walker;
	FDataGamFile_Table_t *pTable;
	FGameDataTableHandle_t hDesiredTable = FGAMEDATA_INVALID_TABLE_HANDLE;

	if( hFileHandle == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "fgamedata_GetLastTableHandle(): Invalid file handle, you must load a file and pass in the returned handle.\n" );
		return FGAMEDATA_INVALID_TABLE_HANDLE;
	}

	FGameDataTableHandle_t hTableHandle = fgamedata_GetFirstTable( hFileHandle, Walker );
	while( hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		pTable = _TH_2_TP( hTableHandle );
		if( fclib_stricmp( pTable->pszKeyString, pszTableName ) == 0 ) {
			hDesiredTable = _TP_2_TH( pTable );
		}
		hTableHandle = fgamedata_GetNextTable( Walker );
	}

	return hDesiredTable;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

cchar *fgamedata_GetFileNameFromTableHandle( FGameDataTableHandle_t hTableHandle ) {
	// get a filehandle from our hTableHandle
	FGameDataFileHandle_t hFileHandle = _GetFileHandleFromTableHandle( hTableHandle );

	return fgamedata_GetFileNameFromFileHandle( hFileHandle );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

cchar *fgamedata_GetTableName( FGameDataTableHandle_t hTableHandle ) {
	FDataGamFile_Table_t *pTable = _TH_2_TP( hTableHandle );

	return pTable->pszKeyString;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 fgamedata_GetNumFields( FGameDataTableHandle_t hTableHandle ) {
	FDataGamFile_Table_t *pTable = _TH_2_TP( hTableHandle );

	return (u32)pTable->nNumFields;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

const void *fgamedata_GetPtrToFieldData( FGameDataTableHandle_t hTableHandle, u32 nFieldIndex,
										 FGameData_VarType_e &rnDataType ) {
	FDataGamFile_Table_t *pTable = _TH_2_TP( hTableHandle );
		
	if( nFieldIndex >= pTable->nNumFields ) {
		DEVPRINTF( "fgamedata_GetPtrToFieldData(): Invalid field index, get the number of fields by calling fgamedata_GetNumFields().\n" );
		return NULL;
	}
	
	FDataGamFile_Field_t *pField = &pTable->paFields[nFieldIndex];

	switch( pField->nDataType ) {

	case FDATA_GAMEFILE_DATA_TYPE_STRING:
		rnDataType = FGAMEDATA_VAR_TYPE_STRING;
		return pField->pszValue;
		break;

	case FDATA_GAMEFILE_DATA_TYPE_FLOAT:
		rnDataType = FGAMEDATA_VAR_TYPE_FLOAT;
		return &pField->fValue;
		break;

	case FDATA_GAMEFILE_DATA_TYPE_WIDESTRING:
		rnDataType = FGAMEDATA_VAR_TYPE_WIDESTRING;
		return pField->pwszValue;
		break;

	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL fgamedata_GetFieldFromTable( FGameDataTableHandle_t hTableHandle, u32 nFieldIndex, 
								  const FGameData_TableEntry_t *pTableEntry, void *pDest ) {
	FDataGamFile_Table_t *pTable = _TH_2_TP( hTableHandle );
		
	if( nFieldIndex >= pTable->nNumFields ) {
		DEVPRINTF( "fgamedata_GetFieldFromTable(): There is no index = %d, the table only has %d fields.\n", nFieldIndex, pTable->nNumFields );	
		return FALSE;
	}
	
	FDataGamFile_Field_t *pField = &pTable->paFields[nFieldIndex];

	// make sure our input parameters are valid
	if( !pTableEntry || !pDest ) {
		// invalid call
		DEVPRINTF( "fgamedata_GetFieldFromTable(): Invalid input parameters, one of the parameters is NULL.\n" );
		return FALSE;
	}

	// make sure that our memory vars make sense
	if( pTableEntry->nBytesForData == 0 ) {
		DEVPRINTF( "fgamedata_GetFieldFromTable(): Invalid input parameters, TableEntry->nBytesForData = %d.\n", pTableEntry->nBytesForData );
		return FALSE;
	}

	// make sure that the field and entry data types jive
	if( !_DoFieldAndEntryTypesMatch( pField, pTableEntry ) ) {
		DEVPRINTF( "fgamedata_GetFieldFromTable(): Data type mismatch, the i'th field is not of the type desired.\n", nFieldIndex );
		return FALSE;
	}

	// calculate the number of bytes needed to store the requested data
	u32 nNumBytesNeeded = _ComputeBytesNeededForEntry( pField, pTableEntry );
	if( nNumBytesNeeded > pTableEntry->nBytesForData ) {
		DEVPRINTF( "fgamedata_GetFieldFromTable(): Not enough dest memory to copy everything requested, %d bytes passed in, %d bytes needed.\n", pTableEntry->nBytesForData, nNumBytesNeeded );
		return FALSE;
	}

	// check valid min & max values
	if( pTableEntry->GetDataType() == FGAMEDATA_VAR_TYPE_FLOAT &&
		pTableEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_REQUIRE_MIN_MAX) {

		FASSERT(pTableEntry->uMinValueLookup < F32_DATATABLE_COUNT );
		FASSERT(pTableEntry->uMaxValueLookup < F32_DATATABLE_COUNT );

		if (pTableEntry->uMaxValueLookup == pTableEntry->uMinValueLookup) {
			DEVPRINTF( "fgamedata_GetFieldFromTable(): The requested entry requires valid min & max values, the current values are equal.\n" );
			return FALSE;
		}
	}

	// copy the requested field
	if( !_CopyField( pField, pTableEntry, pDest ) ) {
		DEVPRINTF( "fgamedata_GetTableData(): Problem while copying the %d'th field.\n", nFieldIndex );
		return FALSE;
	}

	return TRUE;	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

// This version doesn't require prior knowledge of the number of table entries (which makes the other
// version a serious displeasure to use). The table must be terminated with an entry type of
// FGAMEDATA_VAR_TYPE_COUNT.
BOOL fgamedata_GetTableData( FGameDataTableHandle_t hTableHandle, 
							 const FGameData_TableEntry_t *paTableEntries,
							 void *pDest, u32 nNumDestBytes, u32 nFieldStartIndex ) {
	u32 nTableEntryCount;

	for( nTableEntryCount=0; paTableEntries[nTableEntryCount].GetDataType() != FGAMEDATA_VAR_TYPE_COUNT; ++nTableEntryCount ) {}

	if( nTableEntryCount ) {
		return fgamedata_GetTableData( hTableHandle, paTableEntries, nTableEntryCount, pDest, nNumDestBytes, nFieldStartIndex );
	} else {
		return TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

// This version operates on an array of structures.
// paTableEntries defines the structure.
// pDest points to the first byte of the first element in the array to be filled.
// nStructureByteCount is the number of bytes in the structure.
// nArrayElementCount is the number of elements in the array.
BOOL fgamedata_GetTableData_ArrayOfStructures( FGameDataTableHandle_t hTableHandle, 
								   const FGameData_TableEntry_t *paTableEntries,
								   void *pDest, u32 nStructureByteCount, u32 nArrayElementCount ) {
	u32 i, nTableEntryCount, nFieldIndex;

	for( nTableEntryCount=0; paTableEntries[nTableEntryCount].GetDataType() != FGAMEDATA_VAR_TYPE_COUNT; ++nTableEntryCount ) {}

	if( nTableEntryCount == 0 ) {
		return TRUE;
	}

	nFieldIndex = 0;

	for( i=0; i<nArrayElementCount; ++i ) {
		if( !fgamedata_GetTableData( hTableHandle, paTableEntries, nTableEntryCount, pDest, nStructureByteCount, nFieldIndex ) ) {
			return FALSE;
		}

		pDest = (void *)( (u8 *)pDest + nStructureByteCount );
		nFieldIndex += nTableEntryCount;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL fgamedata_GetTableData( FGameDataTableHandle_t hTableHandle, 
							 const FGameData_TableEntry_t *paTableEntries, u32 nNumTableEntries,
							 void *pDest, u32 nNumDestBytes, u32 nFieldStartIndex ) {
	FDataGamFile_Table_t *pTable = _TH_2_TP( hTableHandle );

	// make sure our input parameters are valid
	if( !paTableEntries || !pDest ) {
		// invalid call
		DEVPRINTF( "fgamedata_GetTableData(): Invalid input parameters, one of the parameters is NULL.\n" );
		return FALSE;
	}

	// make sure that our memory vars make sense
	u32 nTableEntryIndex, nFieldIndex, nEntryMemTotal = 0;
	for( nTableEntryIndex=0; nTableEntryIndex<nNumTableEntries; nTableEntryIndex++ ) {
		nEntryMemTotal += paTableEntries[nTableEntryIndex].nBytesForData;
	}	
	if( nNumDestBytes == 0 || nEntryMemTotal > nNumDestBytes ) {
		DEVPRINTF( "fgamedata_GetTableData(): Invalid input parameters, Dest Bytes = %d, yet the sum of TableEntry->nBytesForData = %d.\n", nNumDestBytes, nEntryMemTotal );
		return FALSE;
	}

	// make sure that there are enough fields in the table
	if( (nFieldStartIndex + nNumTableEntries) > pTable->nNumFields ) {
		// our table doesn't contain enough fields to fill all of the requested entries

		if( nFieldStartIndex == 0 ) {
			DEVPRINTF( "fgamedata_GetTableData(): The table %s in file %s has only %d fields, not the requested %d.\n", fgamedata_GetTableName( hTableHandle), fgamedata_GetFileNameFromTableHandle( hTableHandle ), pTable->nNumFields, nNumTableEntries );
		} else {
			DEVPRINTF( "fgamedata_GetTableData(): The table %s in file %s has only %d remaining fields from field %d, not the requested %d.\n", fgamedata_GetTableName( hTableHandle), fgamedata_GetFileNameFromTableHandle( hTableHandle ), pTable->nNumFields-nFieldStartIndex, nFieldStartIndex, nNumTableEntries );
		}

		return FALSE;
	}

	// see if the table matches the passed in template,
	// while we are at it, let's compute the required dest bytes.
	u32 nNeededBytes, nTotalBytesNeeded = 0;
	FDataGamFile_Field_t *pField;
	const FGameData_TableEntry_t *pEntry;

	for( nFieldIndex=nFieldStartIndex, nTableEntryIndex=0; nTableEntryIndex<nNumTableEntries ; ++nFieldIndex, ++nTableEntryIndex ) {
		// grab a ptr to our field
		pField = &pTable->paFields[nFieldIndex];
		pEntry = &paTableEntries[nTableEntryIndex];
		
		// make sure that the field and entry data types jive
		if( !_DoFieldAndEntryTypesMatch( pField, pEntry ) ) {
			DEVPRINTF( "fgamedata_GetTableData(): Data type mismatch - Field %d of table '%s' is not of the type desired.\n", nFieldIndex, fgamedata_GetTableName(hTableHandle) );
			return FALSE;
		}
		
		// calculate the number of bytes needed to store the requested data
		nNeededBytes = _ComputeBytesNeededForEntry( pField, pEntry );

		if( nNeededBytes > pEntry->nBytesForData ) {
			DEVPRINTF( "fgamedata_GetTableData(): Not enough dest memory to copy everything requested, %d bytes specified in entry, %d bytes needed.\n", pEntry->nBytesForData, nNeededBytes );
			return FALSE;
		}
		nTotalBytesNeeded += nNeededBytes;

		// check valid min & max values
		if( pEntry->GetDataType() == FGAMEDATA_VAR_TYPE_FLOAT &&
			pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_REQUIRE_MIN_MAX ) {
			
			FASSERT(pEntry->uMinValueLookup < F32_DATATABLE_COUNT );
			FASSERT(pEntry->uMaxValueLookup < F32_DATATABLE_COUNT );
			
			if (pEntry->uMaxValueLookup == pEntry->uMinValueLookup ) {
				DEVPRINTF( "fgamedata_GetTableData(): The %d'th requested entry requires valid min & max values, the current values are equal.\n", nFieldIndex );
				return FALSE;
			}
		}
	}

	// do one final memory check 
	if( nTotalBytesNeeded > nNumDestBytes ||
		nTotalBytesNeeded > nEntryMemTotal ) {
		DEVPRINTF( "fgamedata_GetTableData(): Not enough dest memory to copy everything requested, %d bytes passed in, %d bytes needed.\n", nNumDestBytes, nTotalBytesNeeded );
		return FALSE;
	}

	// copy the fields
	u8 *pCurrentDest = (u8 *)pDest;
	for( nFieldIndex=nFieldStartIndex, nTableEntryIndex=0; nTableEntryIndex<nNumTableEntries ; ++nFieldIndex, ++nTableEntryIndex ) {
		// grab a ptr to our field
		pField = &pTable->paFields[nFieldIndex];
		pEntry = &paTableEntries[nTableEntryIndex];

		// copy the requested field
		if( !_CopyField( pField, pEntry, pCurrentDest ) ) {
			DEVPRINTF( "fgamedata_GetTableData(): Problem while copying the %d'th field.\n", nFieldIndex );
			return FALSE;
		}

		// advance our ptr
		pCurrentDest += pEntry->nBytesForData;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataTableHandle_t fgamedata_GetFirstTable( FGameDataFileHandle_t hFileHandle, FGameDataWalker_t &rWalker ) {
	FDataGamFile_Header_t *pHeader;
	FDataGamFile_Table_t *pTable;
	cchar *pszFilename;

	// check for a valid file handle
	if( hFileHandle == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "fgamedata_GetFirstTable(): Invalid file handle, you must load a file and pass in the returned handle.\n" );
		return FGAMEDATA_INVALID_TABLE_HANDLE;
	}
	
	// grab a ptr to our data header
	pHeader = _FH_2_HP( hFileHandle );
	FASSERT( pHeader->nFlags & FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP );

	// make sure that the file is valid (has some tables)
	if( pHeader->nNumTables == 0 ) {
		pszFilename = fgamedata_GetFileNameFromFileHandle( hFileHandle );
		DEVPRINTF( "fgamedata_GetFirstTable(): There are no tables in the file '%s'.\n", (pszFilename) ? pszFilename : "unknown" );
		return FGAMEDATA_INVALID_TABLE_HANDLE;
	}

	// grab the first table in the file
	pTable = &pHeader->paTables[0];

	// setup our stack to point the current table
	rWalker.aStack[0].hTable = _TP_2_TH( pTable );
	rWalker.aStack[0].nFieldIndex = _NOT_A_KEYWORD_TABLE;// assume that this table isn't a keyword table for now
	rWalker.nNextStackIndex = 1;

	return _GetFirstNonKeywordTable( rWalker );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

FGameDataTableHandle_t fgamedata_GetNextTable( FGameDataWalker_t &rWalker ) {
	u32 nCurIndex;
	FDataGamFile_Header_t *pHeader;
	FDataGamFile_Table_t *pTable;

	if( rWalker.nNextStackIndex == 0 ) {
		return FGAMEDATA_INVALID_TABLE_HANDLE;
	}
	
	// grab the current file from the table on the top of the stack
	nCurIndex = rWalker.nNextStackIndex - 1;
	pHeader = _FH_2_HP( _GetFileHandleFromTableHandle( rWalker.aStack[nCurIndex].hTable ) );
	FASSERT( pHeader->nFlags & FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP );

	// advance the top of the stack to the very next table that needs to be processed
	pTable = _TH_2_TP( rWalker.aStack[nCurIndex].hTable );
	if( pTable->nTableIndex >= (pHeader->nNumTables-1) ) {
		// we've reached the end of the file, pop off the current table from the stack
		_PopCurrentTableFromStack( rWalker );
		
		if( rWalker.nNextStackIndex == 0 ) {
			// nothing more in the stack
			return FGAMEDATA_INVALID_TABLE_HANDLE;
		}
	} else {
		// simply move to the next table in the file
		pTable++;
		rWalker.aStack[nCurIndex].hTable = _TP_2_TH( pTable );
		rWalker.aStack[nCurIndex].nFieldIndex = _NOT_A_KEYWORD_TABLE;// assume that this table isn't a keyword table for now
	}

	return _GetFirstNonKeywordTable( rWalker );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL fgamedata_ReadFileUsingMap( const FGameDataMap_t *pMapTable, cchar *pszFileName ) {
	FGameDataFileHandle_t hGameDataFile;
	FGameDataTableHandle_t hTableHandle;
	FGameDataWalker_t GameDataWalker;
	const FGameDataMap_t *pMap;
	cchar *pszTableName;

	FResFrame_t ResFrame = fres_GetFrame();
	FMemFrame_t MemFrame = fmem_GetFrame();

	hGameDataFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hGameDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "fgamedata_ReadFileUsingMap(): Could not open '%s'.\n", pszFileName );
		goto _ExitWithError;
	}

	hTableHandle = fgamedata_GetFirstTable( hGameDataFile, GameDataWalker );
	for( ; hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE; hTableHandle = fgamedata_GetNextTable( GameDataWalker ) ) {
		// Find the table in pMapTable...
		pszTableName = fgamedata_GetTableName( hTableHandle );
		for( pMap=pMapTable; pMap->pszTableName; ++pMap ) {
			if( !fclib_stricmp( pszTableName, pMap->pszTableName ) ) {
				// Found table...
				break;
			}
		}
		if( pMap->pszTableName == NULL ) {
			// Table not found...
			continue;
		}

		// Found table in pMapTable...

		if( !fgamedata_GetTableData( hTableHandle, pMap->pVocabTable, pMap->pDestTableData, pMap->nDestTableBytes ) ) {
			// Error...
			DEVPRINTF( "fgamedata_ReadFileUsingMap(): Error in definition of table '%s' found in '%s'.\n", pszTableName, pszFileName );
			goto _ExitWithError;
		}
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


//==================
// private functions

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName ) {
	FDataGamFile_Header_t *pHeader = (FDataGamFile_Header_t *)pLoadedBase;

	if( fgamedata_GetHandleFromLoadedFile( pHeader ) == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return FALSE;
	}
	// unset some flags that fgamedata_GetHandleFromLoadedFile() sets
	pHeader->nFlags &= ~(FDATA_GAMFILE_FLAGS_FMEM_LOADED | FDATA_GAMFILE_FLAGS_UNKNOWN_ORIGIN);

	fres_SetBase( hRes, pLoadedBase );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static BOOL _FixupPtrsAndValidateData( FDataGamFile_Header_t *pHeader ) {
	FDataGamFile_Table_t *pTable;
	FDataGamFile_Field_t *pField;
	u32 i, j, nBaseAddress;

	FASSERT( (pHeader->nFlags & FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP) == 0 );
	
	nBaseAddress = (u32)pHeader;
	// fix up the header ptrs
	pHeader->paTables = (FDataGamFile_Table_t *)( (u32)pHeader->paTables + nBaseAddress );
	
	pTable = pHeader->paTables;
	
	for( i=0; i < pHeader->nNumTables; i++ ) {
		// fix up this table's ptrs
		pTable[i].pszKeyString += nBaseAddress;
		pTable[i].paFields = (FDataGamFile_Field_t *)( (u32)pTable[i].paFields + nBaseAddress );

		for( j=0; j < pTable[i].nNumFields; j++ ) {
			pField = &pTable[i].paFields[j];
			if( pField->nDataType == FDATA_GAMEFILE_DATA_TYPE_STRING ) {
				// fix up this field's char *
				pField->pszValue += nBaseAddress;
			}
			if( pField->nDataType == FDATA_GAMEFILE_DATA_TYPE_WIDESTRING ) {
				// fix up this field's wchar *
				pField->pwszValue = (cwchar*) ( (u32)pField->pwszValue + nBaseAddress );
			}
		}		
	}

	pHeader->nFlags |= FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP;
	
	return TRUE;		
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static BOOL _DoFieldAndEntryTypesMatch( FDataGamFile_Field_t *pField, const FGameData_TableEntry_t *pEntry ) { 
	
	if( pEntry->GetDataType() == FGAMEDATA_VAR_TYPE_STRING &&
		pField->nDataType == FDATA_GAMEFILE_DATA_TYPE_STRING ) {
		return TRUE;
	}
	if( pEntry->GetDataType() == FGAMEDATA_VAR_TYPE_FLOAT &&
		pField->nDataType == FDATA_GAMEFILE_DATA_TYPE_FLOAT ) {
		return TRUE;
	}
	if( pEntry->GetDataType() == FGAMEDATA_VAR_TYPE_WIDESTRING &&
		pField->nDataType == FDATA_GAMEFILE_DATA_TYPE_WIDESTRING ) {
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

// it is assumed that _DoFieldAndEntryTypesMatch has already been called and both parameter's types match
static u32 _ComputeBytesNeededForEntry( FDataGamFile_Field_t *pField, const FGameData_TableEntry_t *pEntry ) {

	if( pEntry->GetDataType() == FGAMEDATA_VAR_TYPE_STRING ) {
		
		if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_ONLY ) {
			// we only need a char *
			return ( sizeof( char * ) );
		}
		
		if( !(pEntry->nFlags & FGAMEDATA_FLAGS_STRING_FAIL_ON_OVERFLOW) ) {
			// truncate to pEntry->nBytesForData
			if( (pField->nStringLen + 1) > pEntry->nBytesForData ) {
				return pEntry->nBytesForData;
			}
		}
		// return the size of the string plus a byte for the NULL terminator
		return (pField->nStringLen + 1);
	}
	if( pEntry->GetDataType() == FGAMEDATA_VAR_TYPE_FLOAT ) {
		u32 nTypesMaskWanted = ( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_ALL_POSSIBLE );	
		u32 nNumFloatsNeeded = fmath_CountBits( nTypesMaskWanted );

		return ( nNumFloatsNeeded * sizeof( f32 ) );
	}
	if( pEntry->GetDataType() == FGAMEDATA_VAR_TYPE_WIDESTRING ) {
		
		if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_ONLY ) {
			// we only need a wchar *
			return ( sizeof( wchar * ) );
		}
		
		if( !(pEntry->nFlags & FGAMEDATA_FLAGS_STRING_FAIL_ON_OVERFLOW) ) {
			// truncate to pEntry->nBytesForData
			if( ( ( pField->nStringLen + 1) * sizeof( u16 ) ) > pEntry->nBytesForData ) {
				return pEntry->nBytesForData;
			}
		}
		// return the size of the wide string plus the NULL terminator
		return ( pField->nStringLen + 1) * sizeof( u16 );
	}

	// unknown data type
	FASSERT_NOW;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

// returns the number of bytes written to pDest
// it is assumed that the needed bytes are available
static u32 _CopyField( FDataGamFile_Field_t *pField, const FGameData_TableEntry_t *pEntry, void *pDest ) {
	u32 nNumBytesWritten = 0;

	switch( pField->nDataType ) {
		
	case FDATA_GAMEFILE_DATA_TYPE_STRING:
		{
			if( (pEntry->nFlags & FGAMEDATA_FLAGS_STRING_NONE_TO_NULL) && !fclib_stricmp( pField->pszValue, "none" ) ) {
				// Value is "none" so set string to NULL...

				if( pEntry->nFlags & (FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_PTR_ONLY |
									  FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE | FGAMEDATA_FLAGS_STRING_TO_TEXDEF |
									  FGAMEDATA_FLAGS_STRING_TO_MESH | FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP |
									  FGAMEDATA_FLAGS_STRING_TO_EXPLODE_GROUP | FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP |
									  FGAMEDATA_FLAGS_STRING_TO_DAMAGE_PROFILE | FGAMEDATA_FLAGS_STRING_TO_ARMOR_PROFILE |
									  FGAMEDATA_FLAGS_STRING_TO_MOTIF_BASE | FGAMEDATA_FLAGS_STRING_TO_DEBRIS_MESH_SET |
									  FGAMEDATA_FLAGS_STRING_TO_DECAL_DEF ) ) {
					// Set string pointer to zero...
					FASSERT( pEntry->nBytesForData >= sizeof( char * ) );
					u32 *pnAddress = (u32 *)pDest;
					*pnAddress = NULL;
					nNumBytesWritten = sizeof( char * );
				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE ) {
					// Set string pointer to invalid handle...
					FASSERT( pEntry->nBytesForData >= sizeof( char * ) );
					u32 *pnAddress = (u32 *)pDest;
					*pnAddress = FPARTICLE_INVALID_HANDLE;
					nNumBytesWritten = sizeof( char * );
				} else {
					// Set first byte of string to NULL...
					if( pEntry->nBytesForData ) {
						char *pcAddress = (char *)pDest;
						*pcAddress = 0;
						nNumBytesWritten = sizeof( char );
					}
				}
			} else {
				// Value is not "none"...

				if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL ) {
					u32 *pnAddress = (u32 *)pDest;
					nNumBytesWritten = sizeof( char * );			

					if( pField->pszValue == NULL ) {
						*pnAddress = 0;
					} else {
						// copy the string into the "Main" string table and store the pointer
						cchar *pszMainStringTableString = CFStringTable::AddString( "Main", (cchar *)pField->pszValue );
						*pnAddress = (u32)pszMainStringTableString;
					}

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_ONLY ) {
					// store the address to the value string
					u32 *pAddress = (u32 *)pDest;
					*pAddress = (u32)pField->pszValue;
					nNumBytesWritten = sizeof( char * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE ) {
					// get a SFX handle for the string value and save the handle
					FSndFx_FxHandle_t *pHandle = (FSndFx_FxHandle_t *)pDest;
					*pHandle = fsndfx_GetFxHandle( pField->pszValue );
					nNumBytesWritten = sizeof( FSndFx_FxHandle_t );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_TEXDEF ) {
					// Get a FTexDef_t pointer for the string name and save the handle...
					FTexDef_t **ppTexDef = (FTexDef_t **)pDest;
					*ppTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, pField->pszValue );
					nNumBytesWritten = sizeof( FTexDef_t * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_MESH ) {
					// Get a FMesh_t pointer for the string name and save the handle...
					FMesh_t **ppMesh = (FMesh_t **)pDest;
					*ppMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pField->pszValue );
					nNumBytesWritten = sizeof( FMesh_t * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP ) {
					// Get a debris group pointer for the string name and save the handle...
					CFDebrisGroup **ppDebrisGroup = (CFDebrisGroup **)pDest;
					*ppDebrisGroup = CFDebrisGroup::Find( pField->pszValue );
					nNumBytesWritten = sizeof( CFDebrisGroup * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_DEBRIS_MESH_SET ) {
					// Get a debris mesh set pointer for the string name and save the handle...
					CFDebrisMeshSet **ppDebrisMeshSet = (CFDebrisMeshSet **)pDest;
					*ppDebrisMeshSet = CFDebrisMeshSet::Find( pField->pszValue );
					nNumBytesWritten = sizeof( CFDebrisMeshSet * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_EXPLODE_GROUP ) {
					// Get an explosion group pointer for the string name and save the handle...
					FExplosion_GroupHandle_t *phExplosionGroup = (FExplosion_GroupHandle_t *)pDest;
					*phExplosionGroup = fexplosion_GetExplosionGroup( pField->pszValue );
					nNumBytesWritten = sizeof( FExplosion_GroupHandle_t );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE ) {
					// Get a particle handle for the string value and save the handle...
					FParticle_DefHandle_t *pHandle = (FParticle_DefHandle_t *)pDest;
					*pHandle = (FParticle_DefHandle_t *)fresload_Load( FPARTICLE_RESTYPE, pField->pszValue );
					nNumBytesWritten = sizeof( FParticle_DefHandle_t );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_MOTIF_BASE ) {
					// Convert the motif string to a motif base index...
					u32 *pnBaseMotif = (u32 *)pDest;
					*pnBaseMotif = fcolor_ConvertStringToBaseMotif( pField->pszValue );
					nNumBytesWritten = sizeof( u32 );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP ) {
					// Get a sound group pointer for the string name and save it...
					CFSoundGroup **ppSoundGroup = (CFSoundGroup **)pDest;
					*ppSoundGroup = CFSoundGroup::RegisterGroup( pField->pszValue );
					nNumBytesWritten = sizeof( CFSoundGroup * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_DAMAGE_PROFILE ) {
					// Get a damage profile pointer for the string name and save it...

					void **ppDamageProfile = (void **)pDest;

					if( _pFcnDamageConverter ) {
						*ppDamageProfile = _pFcnDamageConverter( pField->pszValue );
					} else {
						*ppDamageProfile = NULL;
					}

					nNumBytesWritten = sizeof( void * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_ARMOR_PROFILE ) {
					// Get an armor profile pointer for the string name and save it...

					void **ppArmorProfile = (void **)pDest;

					if( _pFcnArmorConverter ) {
						*ppArmorProfile = _pFcnArmorConverter( pField->pszValue );
					} else {
						*ppArmorProfile = NULL;
					}

					nNumBytesWritten = sizeof( void * );

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_TO_DECAL_DEF ) {
					FDecalDefHandle_t *pData = (FDecalDefHandle_t*)pDest;
					*pData = CFDecal::GetDefByName( pField->pszValue );
					nNumBytesWritten = sizeof( void* );

				} else {
					// copy the string, including the NULL 
					nNumBytesWritten = pField->nStringLen + 1;
					if( !(pEntry->nFlags & FGAMEDATA_FLAGS_STRING_FAIL_ON_OVERFLOW) ) {
						// truncate the string if needed
						FMATH_CLAMPMAX( nNumBytesWritten, pEntry->nBytesForData );
					}
					fang_MemCopy( pDest, pField->pszValue, nNumBytesWritten );
				}
			}
		}
		break;

	case FDATA_GAMEFILE_DATA_TYPE_WIDESTRING:
		{
			if( (pEntry->nFlags & FGAMEDATA_FLAGS_STRING_NONE_TO_NULL) && !fclib_wcsicmp( pField->pwszValue, L"none" ) ) {
				// Value is "none" so set string to NULL...

				if( pEntry->nFlags & (FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_PTR_ONLY |
									  FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE | FGAMEDATA_FLAGS_STRING_TO_TEXDEF |
									  FGAMEDATA_FLAGS_STRING_TO_MESH | FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP |
									  FGAMEDATA_FLAGS_STRING_TO_EXPLODE_GROUP | FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP |
									  FGAMEDATA_FLAGS_STRING_TO_DAMAGE_PROFILE | FGAMEDATA_FLAGS_STRING_TO_ARMOR_PROFILE |
									  FGAMEDATA_FLAGS_STRING_TO_MOTIF_BASE | FGAMEDATA_FLAGS_STRING_TO_DEBRIS_MESH_SET ) ) {
					// Set string pointer to zero...
					FASSERT( pEntry->nBytesForData >= sizeof( u16 * ) );
					u32 *pnAddress = (u32 *)pDest;
					*pnAddress = NULL;
					nNumBytesWritten = sizeof( u16 * );
				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE ) {
					// Set string pointer to invalid handle...
					FASSERT( pEntry->nBytesForData >= sizeof( u16 * ) );
					u32 *pnAddress = (u32 *)pDest;
					*pnAddress = FPARTICLE_INVALID_HANDLE;
					nNumBytesWritten = sizeof( u16 * );
				} else {
					// Set first byte of string to NULL...
					if( pEntry->nBytesForData ) {
						u16 *pwcAddress = ( u16 *)pDest;
						*pwcAddress = 0;
						nNumBytesWritten = sizeof( u16 );
					}
				}
			} else {
				// Value is not "none"...

				if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL ) {
					u32 *pnAddress = (u32 *)pDest;
					nNumBytesWritten = sizeof( u16 * );			

					if( pField->pwszValue == NULL ) {
						*pnAddress = 0;
					} else {
						// copy the string into the "Main" string table and store the pointer
						cwchar *pwszMainStringTableString = CFStringTable::AddString( "Main", (cwchar *)pField->pwszValue );
						*pnAddress = (u32)pwszMainStringTableString;
					}

				} else if( pEntry->nFlags & FGAMEDATA_FLAGS_STRING_PTR_ONLY ) {
					// store the address to the value string
					u32 *pAddress = (u32 *)pDest;
					*pAddress = (u32)pField->pwszValue;
					nNumBytesWritten = sizeof( u16 * );

				} else if( pEntry->nFlags & ( FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE | FGAMEDATA_FLAGS_STRING_TO_TEXDEF |
											  FGAMEDATA_FLAGS_STRING_TO_MESH | FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP |
											  FGAMEDATA_FLAGS_STRING_TO_EXPLODE_GROUP | FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP |
											  FGAMEDATA_FLAGS_STRING_TO_DAMAGE_PROFILE | FGAMEDATA_FLAGS_STRING_TO_ARMOR_PROFILE |
											  FGAMEDATA_FLAGS_STRING_TO_MOTIF_BASE | FGAMEDATA_FLAGS_STRING_TO_DEBRIS_MESH_SET ) ) {
					// THESE FLAGS ARE NOT SUPPORTED FOR WIDESTRINGS!
					FASSERT_MSG( 1, "Invalid usage of FGAMEDATA_FLAGS_STRINGS_* with a wide string resource!\n");  
					FASSERT( pEntry->nBytesForData >= sizeof( u16 * ) );
					u32 *pnAddress = (u32 *)pDest;
					*pnAddress = NULL;
					nNumBytesWritten = sizeof( u16 * );

				} else {
					// copy the string, including the NULL 
					nNumBytesWritten = pField->nStringLen + sizeof( u16 );
					if( !(pEntry->nFlags & FGAMEDATA_FLAGS_STRING_FAIL_ON_OVERFLOW) ) {
						// truncate the string if needed
						FMATH_CLAMPMAX( nNumBytesWritten, pEntry->nBytesForData );
					}
					fang_MemCopy( pDest, pField->pwszValue, nNumBytesWritten );
				}
			}
		}
		break;

	case FDATA_GAMEFILE_DATA_TYPE_FLOAT:
		{
			f32 *pfFloat = (f32 *)pDest;
			f32 fX = pField->fValue;
			u32 nIndex = 0;

			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL ) {
				FASSERT(pEntry->uMinValueLookup < F32_DATATABLE_COUNT );
				FASSERT(pEntry->uMaxValueLookup < F32_DATATABLE_COUNT );

				if( fX < afDataTable[pEntry->uMinValueLookup]) {
					return 0;
				}
				if( fX > afDataTable[pEntry->uMaxValueLookup]) {
					return 0;
				}
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO ) {
				FASSERT(pEntry->uMinValueLookup < F32_DATATABLE_COUNT );
				FASSERT(pEntry->uMaxValueLookup < F32_DATATABLE_COUNT );

				FMATH_CLAMP( fX, afDataTable[pEntry->uMinValueLookup], afDataTable[pEntry->uMaxValueLookup] );
			}

			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_X ) {
				pfFloat[nIndex++] = fX;
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_OO_X ) {
				if( fX == 0.0f )
				{
					pfFloat[nIndex++] = FMATH_MAX_FLOAT;
				}
				else
				{
					pfFloat[nIndex++] = ( 1.0f / fX );
				}
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_RADS_TO_DEGS ) {
				pfFloat[nIndex++] = FMATH_RAD2DEG( fX );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS ) {
				pfFloat[nIndex++] = FMATH_DEG2RAD( fX );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT ) {
				pfFloat[nIndex] = ( fX / 100.0f );
				FMATH_CLAMP_UNIT_FLOAT( pfFloat[nIndex] );
				nIndex++;
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_UNIT_FLOAT ) {
				FASSERT(pEntry->uMinValueLookup < F32_DATATABLE_COUNT );
				FASSERT(pEntry->uMaxValueLookup < F32_DATATABLE_COUNT );
				pfFloat[nIndex++] = ( (fX - afDataTable[pEntry->uMinValueLookup]) / (afDataTable[pEntry->uMaxValueLookup] - afDataTable[pEntry->uMinValueLookup]) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_BI_POLAR_UNIT_FLOAT ) {
				FASSERT(pEntry->uMinValueLookup < F32_DATATABLE_COUNT );
				FASSERT(pEntry->uMaxValueLookup < F32_DATATABLE_COUNT );
				pfFloat[nIndex++] = ( (((fX - afDataTable[pEntry->uMinValueLookup])/(afDataTable[pEntry->uMaxValueLookup] - afDataTable[pEntry->uMinValueLookup])) - 0.5f) * 2.0f );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_SQRT_X ) {
				pfFloat[nIndex++] = ( fmath_AcuSqrt( fX ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_OO_SQRT_X ) {
				pfFloat[nIndex++] = ( fmath_AcuInvSqrt( fX ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_SIN_DEGS_TO_RADS_X ) {
				pfFloat[nIndex++] = ( fmath_Sin( FMATH_DEG2RAD( fX ) ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_COS_DEGS_TO_RADS_X ) {
				pfFloat[nIndex++] = ( fmath_Cos( FMATH_DEG2RAD( fX ) ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_X_SQUARED ) {
				pfFloat[nIndex++] = ( FMATH_SQUARE( fX ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_X_CUBED ) {
				pfFloat[nIndex++] = ( FMATH_CUBE( fX ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_ABS_X ) {
				pfFloat[nIndex++] = ( FMATH_FABS( fX ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_OO_X_SQUARED ) {
				pfFloat[nIndex++] = ( 1.0f / FMATH_SQUARE( fX ) );
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_CONVERT_TO_U32 ) {
				u32 *pnData = (u32 *)&pfFloat[nIndex];
				*pnData = (u32)(fX + 0.25f);
				++nIndex;
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_CONVERT_TO_S32 ) {
				s32 *pnData = (s32 *)&pfFloat[nIndex];
				*pnData = (s32)fX;
				++nIndex;
			}
			if( pEntry->nFlags & FGAMEDATA_FLAGS_FLOAT_UNCLAMPED_PERCENT_TO_UNIT_FLOAT ) {
				pfFloat[nIndex] = ( fX / 100.0f );
				nIndex++;
			}
			nNumBytesWritten = nIndex * sizeof( f32 );
		}
		break;

	default:
		FASSERT_NOW;
		break;
	}

	FASSERT( nNumBytesWritten <= pEntry->nBytesForData );

	return nNumBytesWritten;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static FGameDataFileHandle_t _GetFileHandleFromTableHandle( FGameDataTableHandle_t hTableHandle ) {
	FDataGamFile_Table_t *pTable = _TH_2_TP( hTableHandle );
	pTable -= pTable->nTableIndex;

	FDataGamFile_Header_t *pHeader = (FDataGamFile_Header_t *)pTable;
	pHeader--;

	return _HP_2_FH( pHeader );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static void _CreateFilenameString( BOOL bFresTypeString, cchar *pszFilename ) {

	fclib_strncpy( _szFilename, pszFilename, FDATA_PRJFILE_FILENAME_LEN ); 
	_szFilename[FDATA_PRJFILE_FILENAME_LEN-1] = 0;
	fclib_strlwr( _szFilename );
	int nLen = fclib_strlen( _szFilename );

	// is there an extension on the filename
	if( nLen > 4 &&
		_szFilename[nLen-4] == '.' &&
		_szFilename[nLen-3] == 'c' &&
		_szFilename[nLen-2] == 's' &&
		_szFilename[nLen-1] == 'v' ) {
		// there is an extension
		if( bFresTypeString ) {
			// remove the extension
			_szFilename[nLen-4] = 0;
		}
	} else {
		// there is no extension
		if( !bFresTypeString &&
			nLen <= (FDATA_PRJFILE_FILENAME_LEN-5) ) {
			// add an extension
			fclib_strcat( _szFilename, ".csv" );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static BOOL _DoesTableNameMatchKeyword( FDataGamFile_Table_t *pTable ) {

	if( !_pszKeyword ) {
		return FALSE;
	}

	if( fclib_stricmp( pTable->pszKeyString, _pszKeyword ) == 0 ) {
		// we match
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static FGameDataFileHandle_t _LoadFieldStringCsvFile( FDataGamFile_Table_t *pTable, u32 nFieldIndex ) {
	FDataGamFile_Field_t *pField;
	
	if( nFieldIndex >= pTable->nNumFields ) {
		DEVPRINTF( "fgamedata::_LoadFieldStringCsvFile(): Invalid field index (%d) into a table with only %d fields.\n", nFieldIndex, pTable->nNumFields );
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}
	pField = &pTable->paFields[nFieldIndex];

	// pField must point to a string
	if( pField->nDataType != FDATA_GAMEFILE_DATA_TYPE_STRING ) {
		DEVPRINTF( "fgamedata::_LoadFieldStringCsvFile(): Field [%d] of the table named '%s' is not a string so no file could be loaded.\n", nFieldIndex, pTable->pszKeyString );
		return FGAMEDATA_INVALID_FILE_HANDLE;
	}
	// get the header to determine where to load this file
	FGameDataFileHandle_t hFile = _GetFileHandleFromTableHandle( _TP_2_TH( pTable ) );
	FDataGamFile_Header_t *pHeader = _FH_2_HP( hFile );

	if( pHeader->nFlags & (FDATA_GAMFILE_FLAGS_FMEM_LOADED | FDATA_GAMFILE_FLAGS_UNKNOWN_ORIGIN) ) {
		// load the file into fmem memory
		hFile = fgamedata_LoadFileToFMem( pField->pszValue );
	} else {
		// load into fres memory
		_CreateFilenameString( TRUE, pField->pszValue );
		hFile = _HP_2_FH( fresload_Load( FGAMEDATA_RESTYPE, pField->pszValue ) );
	}

	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		cchar *pszFilename = fgamedata_GetFileNameFromTableHandle( _TP_2_TH( pTable ) );

		DEVPRINTF( "fgamedata::_LoadFieldStringCsvFile(): Could not load file '%s', as referenced in file '%s', table '%s', field [%d].\n",
				   pField->pszValue, pszFilename ? pszFilename : "unknown", pTable->pszKeyString, nFieldIndex );
	}
	
	return hFile;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static FGameDataTableHandle_t _GetFirstNonKeywordTable( FGameDataWalker_t &rWalker ) { 
	u32 nCurIndex;
	FGameDataFileHandle_t hNewFile;
	FDataGamFile_Header_t *pHeader;
	FDataGamFile_Table_t *pTable;
	cchar *pszFilename;
	BOOL bPreviousError = FALSE;

	if( rWalker.nNextStackIndex == 0 ) {
		return FGAMEDATA_INVALID_TABLE_HANDLE;
	}

	pTable = _TH_2_TP( rWalker.aStack[rWalker.nNextStackIndex - 1].hTable );

	// attemp to get a valid table onto the top of the stack
	while( _DoesTableNameMatchKeyword( pTable ) &&
		   rWalker.nNextStackIndex < FGAMEDATA_MAX_RECURSION_DEPTH ) {
		
		nCurIndex = rWalker.nNextStackIndex - 1;
		// mark the current stack top as a keyword table
		if( rWalker.aStack[nCurIndex].nFieldIndex == _NOT_A_KEYWORD_TABLE ) {
			// we discovered that the stack top table is a keyword table
			rWalker.aStack[nCurIndex].nFieldIndex = 0;
		}

		if( rWalker.aStack[nCurIndex].nFieldIndex >= pTable->nNumFields ) {
			// we've moved too far, there are no more fields to load, move to the next table entry
			pHeader = _FH_2_HP( _GetFileHandleFromTableHandle( rWalker.aStack[nCurIndex].hTable ) );
			if( pTable->nTableIndex >= (pHeader->nNumTables-1) ) {
				// pop
				_PopCurrentTableFromStack( rWalker );
		
				if( rWalker.nNextStackIndex == 0 ) {
					// nothing more in the stack
					return FGAMEDATA_INVALID_TABLE_HANDLE;
				}
			} else {
				// move to the next table
				pTable++;
				rWalker.aStack[nCurIndex].hTable = _TP_2_TH( pTable );
				rWalker.aStack[nCurIndex].nFieldIndex = _NOT_A_KEYWORD_TABLE;// assume that this table isn't a keyword table for now
			}
			continue;
		}
		
		// load pTable's first field as another table
		hNewFile = _LoadFieldStringCsvFile( pTable, rWalker.aStack[nCurIndex].nFieldIndex );
		if( hNewFile == FGAMEDATA_INVALID_TABLE_HANDLE ) {
			// could not load the requested field, try loading the next field of this table
			rWalker.aStack[nCurIndex].nFieldIndex++;
			bPreviousError = TRUE;
			continue;
		}

		// grab a ptr to our data header
		pHeader = _FH_2_HP( hNewFile );
		FASSERT( pHeader->nFlags & FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP );

		// make sure that the file is valid (has some tables)
		if( pHeader->nNumTables == 0 ) {
			// the loaded table doesn't have an tables, try loading the next field of this table
			pszFilename = fgamedata_GetFileNameFromFileHandle( hNewFile );
			
			DEVPRINTF( "fgamedata::_GetFirstNonKeywordTable(): There are no tables in the file '%s', skipping file.\n", (pszFilename) ? pszFilename : "unknown" );
			rWalker.aStack[nCurIndex].nFieldIndex++;
			bPreviousError = TRUE;
			continue;
		}

		// grab the first table in the new file
		pTable = &pHeader->paTables[0];
		
		// push the new table onto the stack
		rWalker.aStack[ rWalker.nNextStackIndex ].hTable = _TP_2_TH( pTable );
		rWalker.aStack[ rWalker.nNextStackIndex ].nFieldIndex = _NOT_A_KEYWORD_TABLE;// assume that this table isn't a keyword table for now
		rWalker.nNextStackIndex++;		
	}

	// if the top of the stack is still a keyword, then we've tried to recurse too far down
	nCurIndex = rWalker.nNextStackIndex - 1;
	pTable = _TH_2_TP( rWalker.aStack[nCurIndex].hTable );
	if( _DoesTableNameMatchKeyword( pTable ) ) {
		if( rWalker.nNextStackIndex >= FGAMEDATA_MAX_RECURSION_DEPTH ) {
			DEVPRINTF( "fgamedata::_GetFirstNonKeywordTable(): Tried to recurse too deep, increase the stack size from the current value of %d.\n", FGAMEDATA_MAX_RECURSION_DEPTH );		
		}
		// set the walker to empty so that fgamedata_GetNextTable() calls fail
		rWalker.nNextStackIndex = 0;

		return FGAMEDATA_INVALID_TABLE_HANDLE;
	}

	// cool, we reached a valid, non keyword table, on the top of the stack
	return rWalker.aStack[nCurIndex].hTable;
}

static void _PopCurrentTableFromStack( FGameDataWalker_t &rWalker ) {
	u32 nCurIndex;
	FDataGamFile_Header_t *pHeader;
	FDataGamFile_Table_t *pTable;

	if( rWalker.nNextStackIndex == 0 ) {
		return;
	}

	rWalker.nNextStackIndex--;
	while( rWalker.nNextStackIndex > 0 ) {
		nCurIndex = rWalker.nNextStackIndex - 1;
		pTable = _TH_2_TP( rWalker.aStack[nCurIndex].hTable );
		
		// advance to the next field in the keyword table (if it wasn't a keyword table, why would it be in the stack?)
		FASSERT( rWalker.aStack[nCurIndex].nFieldIndex != _NOT_A_KEYWORD_TABLE );
		rWalker.aStack[nCurIndex].nFieldIndex++;

		// see if we should move to the next table
		if( rWalker.aStack[nCurIndex].nFieldIndex < pTable->nNumFields ) {
			// there is another include field that needs to be processed on top of the stack, done
			break;
		} else {
			pHeader = _FH_2_HP( _GetFileHandleFromTableHandle( rWalker.aStack[nCurIndex].hTable ) );
			FASSERT( pHeader->nFlags & FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP );

			if( pTable->nTableIndex >= (pHeader->nNumTables-1) ) {
				// pop another table from the stack
				rWalker.nNextStackIndex--;
				continue;
			} else {
				pTable++;
				rWalker.aStack[nCurIndex].hTable = _TP_2_TH( pTable );
				rWalker.aStack[nCurIndex].nFieldIndex = _NOT_A_KEYWORD_TABLE;// assume that this table isn't a keyword table for now
				// there is a new table that needs to be processed on top of the stack, done
				break;
			}
		}
		// WE SHOULD NEVER GET HERE
		FASSERT_NOW;
	}
}


