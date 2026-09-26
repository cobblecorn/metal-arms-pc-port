//////////////////////////////////////////////////////////////////////////////////////
// MasterFileCompile.cpp - 
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
// 05/21/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "MasterFileCompile.h"
#include "FileInfo.h"
#include "fmath.h"
#include "fclib.h"
#include "fdata.h"
#include "fversion.h"

// private defines
#define _BUFFER_SIZE	( 128 * 1024 )// how many bytes of a file should we read/write at a time

// private vars
static char _cBuffer[_BUFFER_SIZE];

// private prototypes
static int _QSortCompareFcn( const void *arg1, const void *arg2 );


///////////////////////////
// INIT & CLEANUP FUNCTIONS

CMasterFileCompile::CMasterFileCompile( void *pUserData/*=NULL*/,
									    MasterFileCompile_ProgressBarInitFcn_t *pProgressInit/*=NULL*/,
									    MasterFileCompile_ProgressBarStepFcn_t *pProgressStep/*=NULL*/ ) {
	m_pBinFileStream = NULL;
	m_pAllocatedMem = NULL;
	m_paEntries = NULL;
	m_paSupportEntries = NULL;

	m_pProgressInitFcn = pProgressInit;
	m_pProgressStepFcn = pProgressStep;
	m_pUserData = pUserData;

	fang_MemZero( _cBuffer, _BUFFER_SIZE );
}

CMasterFileCompile::~CMasterFileCompile() {
	Reset();
}

// closes any open files and frees any allocated memory
void CMasterFileCompile::Reset() {
	if( m_pBinFileStream ) {
		fclose( m_pBinFileStream );
		m_pBinFileStream = NULL;
	}
	if( m_pAllocatedMem ) {
		delete [] m_pAllocatedMem;
		m_pAllocatedMem = NULL;
	}
	m_paEntries = NULL;
	m_paSupportEntries = NULL;
}

//////////////////////////////////////
// DATA EXTRACTION & UTILITY FUNCTIONS

// returns TRUE if rsEntryName is valid, FALSE otherwise
BOOL CMasterFileCompile::IsEntryNameValid( const CString &rsEntryName ) {
	
	// make sure that th entry name has at least 1 char
	if( rsEntryName.IsEmpty() ) {
		// filename is too short (at least 1 char is required)
		return FALSE;
	}
	// make sure that the entry name is not too long
	if( rsEntryName.GetLength() > FDATA_PRJFILE_FILENAME_LEN ) {
		// filename is too long
		return FALSE;
	}
	return TRUE;
}

// will reset and then open pszfilename, allocate memory and read in the 
// entry table and the support entry table.  Also checks the signature
// and version info
MasterFileCompile_ErrorCodes_e CMasterFileCompile::ReadHeaderAndEntries( cchar *pszFilename, BOOL bReadOnly, BOOL bAutoClose ) {
	u32 nBytesToAllocate;
	BOOL bSwapEndian;
	MasterFileCompile_ErrorCodes_e nRetCode;

	Reset();

	m_pBinFileStream = fopen( pszFilename, bReadOnly ? "rb" : "r+b" );
	if( !m_pBinFileStream ) {
		Reset();
		return MASTER_FILE_COMPILE_ERROR_OPENING;
	}
	// read in the current header
	fseek( m_pBinFileStream, 0, SEEK_SET );
	nRetCode = ReadHeader( bSwapEndian );
	if( nRetCode != MASTER_FILE_COMPILE_ERROR_NONE ) {
		Reset();
		return nRetCode;
	}
	// check the version info
	if( ( FVERSION_GET_MAJOR_NUMBER( m_Header.Version.nVersion ) != FDATA_PRJFILE_MAJOR ) ||
		( FVERSION_GET_MINOR_NUMBER( m_Header.Version.nVersion ) != FDATA_PRJFILE_MINOR ) ||
		( FVERSION_GET_SUB_NUMBER( m_Header.Version.nVersion ) != FDATA_PRJFILE_SUB ) ) {
		// the master file version is not correct
		Reset();
		return MASTER_FILE_COMPILE_ERROR_WRONG_VERSION;
	}
	// check the filesize
	fseek( m_pBinFileStream, 0, SEEK_END );
	if( ftell( m_pBinFileStream ) != (int)m_Header.nBytesInFile ) {
		// the filesize in the header does not match the actual filesize
		Reset();
		return MASTER_FILE_COMPILE_ERROR_WRONG_FILESIZE;
	}
	fseek( m_pBinFileStream, sizeof( FDataPrjFile_Header_t ), SEEK_SET );

	// read in the entries (first allocate enough memory to hold them all
	nBytesToAllocate = (m_Header.nNumEntries + m_Header.nNumFreeEntries) * sizeof( FDataPrjFile_Entry_t );
	nBytesToAllocate += (m_Header.nNumSupportEntries + m_Header.nNumFreeSupportEntries) * sizeof( FDataPrjFile_SupportEntry_t );
	m_pAllocatedMem = new u8[nBytesToAllocate];
	if( !m_pAllocatedMem ) {
		// could not allocate enough memory for all of the entries
		Reset();
		return MASTER_FILE_COMPILE_ERROR_ALLOCATING_MEMORY;
	}
	u8 *pDest = m_pAllocatedMem;
	nRetCode = ReadEntryArray( pDest,
							   m_Header.nNumEntries + m_Header.nNumFreeEntries,
							   m_Header.nNumEntries,
							   bSwapEndian );
	if( nRetCode != MASTER_FILE_COMPILE_ERROR_NONE ) {
		// could not read the file
		Reset();
		return nRetCode;
	}
	pDest += (m_Header.nNumEntries + m_Header.nNumFreeEntries) * sizeof( FDataPrjFile_Entry_t );
	nRetCode = ReadSupportEntryArray( pDest, 
									  m_Header.nNumSupportEntries + m_Header.nNumFreeSupportEntries,
									  m_Header.nNumSupportEntries,
									  bSwapEndian );
	if( nRetCode != MASTER_FILE_COMPILE_ERROR_NONE ) {
		// could not read the file
		Reset();
		return nRetCode;
	}
	m_paEntries = (FDataPrjFile_Entry_t *)m_pAllocatedMem;
	m_paSupportEntries = (FDataPrjFile_SupportEntry_t *)&m_paEntries[ m_Header.nNumEntries + m_Header.nNumFreeEntries ];

	if( bAutoClose ) {
		fclose( m_pBinFileStream );
		m_pBinFileStream = NULL;
	}
	return MASTER_FILE_COMPILE_ERROR_NONE;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the tga version from the master file header
u32 CMasterFileCompile::GetTgaVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nTgaCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old tga version from the master file header
u32 CMasterFileCompile::SetTgaVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nTgaCompilerVersion;
	m_Header.nTgaCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the ape version from the master file header
u32 CMasterFileCompile::GetApeVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nApeCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old ape version from the master file header
u32 CMasterFileCompile::SetApeVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nApeCompilerVersion;
	m_Header.nApeCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the mtx version from the master file header
u32 CMasterFileCompile::GetMtxVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nMtxCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old mtx version from the master file header
u32 CMasterFileCompile::SetMtxVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nMtxCompilerVersion;
	m_Header.nMtxCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the csv version from the master file header
u32 CMasterFileCompile::GetCsvVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nCsvCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old csv version from the master file header
u32 CMasterFileCompile::SetCsvVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nCsvCompilerVersion;
	m_Header.nCsvCompilerVersion = nVersion;
	return nOldVersion;
}


// returns 0 if a master file is not currently loaded
// otherwise returns the fnt version from the master file header
u32 CMasterFileCompile::GetFntVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nFntCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old fnt version from the master file header
u32 CMasterFileCompile::SetFntVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nFntCompilerVersion;
	m_Header.nFntCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the sma version from the master file header
u32 CMasterFileCompile::GetSmaVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nSmaCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old sma version from the master file header
u32 CMasterFileCompile::SetSmaVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nSmaCompilerVersion;
	m_Header.nSmaCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the gt version from the master file header
u32 CMasterFileCompile::GetGtVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nGtCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old gt version from the master file header
u32 CMasterFileCompile::SetGtVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nGtCompilerVersion;
	m_Header.nGtCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the wvb version from the master file header
u32 CMasterFileCompile::GetWvbVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nWvbCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old wvb version from the master file header
u32 CMasterFileCompile::SetWvbVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nWvbCompilerVersion;
	m_Header.nWvbCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the fpr version from the master file header
u32 CMasterFileCompile::GetFprVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nFprCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old fpr version from the master file header
u32 CMasterFileCompile::SetFprVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nFprCompilerVersion;
	m_Header.nFprCompilerVersion = nVersion;
	return nOldVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the cam version from the master file header
u32 CMasterFileCompile::GetCamVersionFromHeader() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nCamCompilerVersion;
}

// returns 0 if a master file is not currently loaded
// otherwise returns the old cam version from the master file header
u32 CMasterFileCompile::SetCamVersion( u32 nVersion ) {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	u32 nOldVersion = m_Header.nCamCompilerVersion;
	m_Header.nCamCompilerVersion = nVersion;
	return nOldVersion;
}


u32 CMasterFileCompile::GetNumEntries() {

	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nNumEntries;
}

const FDataPrjFile_Entry_t *CMasterFileCompile::GetEntryByIndex( u32 nIndex ) {
	
	if( !m_pAllocatedMem || (nIndex >= m_Header.nNumEntries) ) {
		return NULL;
	}
	return &m_paEntries[nIndex];
}

// searches m_paEntries for an entry that has pszFilename, if not found then NULL is returned
FDataPrjFile_Entry_t *CMasterFileCompile::GetEntryByName( cchar *pszFilename ) {
	FDataPrjFile_Entry_t SearchFor;
		
	if( !pszFilename || !m_pAllocatedMem || (m_Header.nNumEntries == 0) ) {
		return NULL;
	}

	// create an entry for pszFilename
	CString sFilename( pszFilename );
	sFilename.MakeLower();
	if( !IsEntryNameValid( sFilename ) ) {
		// filename is too long or too short
		return NULL;
	}
	// fill in the entry
	fclib_strncpy( SearchFor.szFilename, (cchar *)sFilename, FDATA_PRJFILE_FILENAME_LEN );
	SearchFor.nStartOffset = 0;
	SearchFor.nNumBytes = 0;
	SearchFor.nModifiedTime = 0;

	// start a binary search for SearchFor
	FDataPrjFile_Entry_t *pFoundEntry = (FDataPrjFile_Entry_t *)bsearch( &SearchFor,
																		 m_paEntries,
																		 m_Header.nNumEntries,
																		 sizeof( FDataPrjFile_Entry_t ),
																		 _QSortCompareFcn );
	// may be NULL if SearchFor wasn't found
	return pFoundEntry;	
}

BOOL CMasterFileCompile::DoModifiedTimesMatch( const FDataPrjFile_Entry_t *pEntry, const CFileInfo *pFileInfo ) {
	
	CTime FileTime( pFileInfo->GetLastWriteTime() );
	CTime EntryTime( (time_t)pEntry->nModifiedTime );
	if( EntryTime != FileTime ) {
		// the times are different
		return FALSE;
	}
	return TRUE;
}

u32 CMasterFileCompile::GetNumSupportEntries() {

	if( !m_pAllocatedMem ) {
		return 0;
	}
	return m_Header.nNumSupportEntries;
}

u8 CMasterFileCompile::GetPlatformMask() {
	if( !m_pAllocatedMem ) {
		return 0;
	}
	return (u8)( FVERSION_GET_PLATFORM_MASK( m_Header.Version.nVersion ) );
}

const FDataPrjFile_SupportEntry_t *CMasterFileCompile::GetSupportEntryByIndex( u32 nIndex ) {
	
	if( !m_pAllocatedMem || (nIndex >= m_Header.nNumSupportEntries) ) {
		return NULL;
	}
	return &m_paSupportEntries[nIndex];
}

// searches m_paSupportEntries for an entry that has pszFilename, if not found then NULL is returned
FDataPrjFile_SupportEntry_t *CMasterFileCompile::GetSupportEntryByName( cchar *pszFilename ) {
			
	if( !pszFilename || !m_pAllocatedMem || (m_Header.nNumSupportEntries == 0) ) {
		return NULL;
	}

	CString sFilename( pszFilename );
	sFilename.MakeLower();
	if( !IsEntryNameValid( sFilename ) ) {
		// filename is too long or too short
		return NULL;
	}
	CString sEntryname;
	for( u32 i=0; i < m_Header.nNumSupportEntries; i++ ) {
		sEntryname = m_paSupportEntries[i].szFilename;
		if( sEntryname == sFilename ) {
			return &m_paSupportEntries[i];
		}
	}
	return NULL;
}

BOOL CMasterFileCompile::DoModifiedTimesMatch( const FDataPrjFile_SupportEntry_t *pEntry, const CFileInfo *pFileInfo ) {
	
	CTime FileTime( pFileInfo->GetLastWriteTime() );
	CTime EntryTime( (time_t)pEntry->nModifiedTime );
	if( EntryTime != FileTime ) {
		// the times are different
		return FALSE;
	}
	return TRUE;
}

// copys pSrcFile to pDstFile by copying _BUFFER_SIZE number of bytes at a time.
// it is the callers responsibility to position pDstFile to the desired place in the file
// Returns TRUE if successful, FALSE if there was a problem
BOOL CMasterFileCompile::CopyFile( FILE *pDstFile, FILE *pSrcFile, u32 nSrcBytes/*=0*/ ) {
	u32 nBytesAtATime;

	if( !pDstFile || !pSrcFile ) {
		// no file to copy
		return FALSE;
	}
	if( nSrcBytes == 0 ) {
		// figure out how many bytes pSrcFile contains
		fseek( pSrcFile, 0, SEEK_END );
		nSrcBytes = ftell( pSrcFile );
		fseek( pSrcFile, 0, SEEK_SET );
	}

	while( nSrcBytes > 0 ) {
		nBytesAtATime = FMATH_MIN( nSrcBytes, _BUFFER_SIZE );

		if( fread( _cBuffer, nBytesAtATime, 1, pSrcFile ) != 1 ) {
			// could not read the file, skip this file
			return FALSE;
		}
		// write out our data to our dst file
		if( fwrite( _cBuffer, nBytesAtATime, 1, pDstFile ) != 1 ) {
			// could not write to the file, skip this file
			return FALSE;
		}
		nSrcBytes -= nBytesAtATime;
	}
	return TRUE;
}

// writes nNumBytes of Zeros wherever pDstFile is currently positioned,
// this may expand the filesize or overwrite currently written data, it
// is up to the caller to use this function properly
// Returns TRUE if successful, FALSE if there was a problem
BOOL CMasterFileCompile::PadFileWithZeros( FILE *pDstFile, u32 nNumBytes ) {
	
	if( !pDstFile ) {
		return FALSE;
	}
	if( !nNumBytes ) {
		// nothing to pad
		return TRUE;
	}

	if( nNumBytes <= _BUFFER_SIZE ) {
		fang_MemZero( _cBuffer, nNumBytes ); 
		if( fwrite( _cBuffer, nNumBytes, 1, pDstFile ) != 1 ) {
			return FALSE;
		}
	} else {
		fang_MemZero( _cBuffer, _BUFFER_SIZE );
		u32 nBytesToWrite;
		while( nNumBytes ) {
			nBytesToWrite = ( nNumBytes > _BUFFER_SIZE ) ? _BUFFER_SIZE : nNumBytes;
			if( fwrite( _cBuffer, nBytesToWrite, 1, pDstFile ) != 1 ) {
				return FALSE;
			}
			nNumBytes -= nBytesToWrite;
		}
	}
	return TRUE;
}

// inserts zeros at nInsertByteOffset, and moves all of the current data in the file downward so that no data
// is lost.  The file will be nNumBytes larger after calling this function.
// Returns TRUE if successful, FALSE if there was a problem
BOOL CMasterFileCompile::InsertBytesIntoAFile( FILE *pDstFile, u32 nInsertByteOffset, u32 nNumBytesToInsert ) {
	u32 nBytesInOrigFile, nBytesToCopy, nSrcOffset, nDstOffset, nBytesAtATime;

	if( !pDstFile ) {
		return FALSE;
	}

	// find out how large the file is, do this
	// by moving to the end of the file and getting
	// the file offset
	fseek( pDstFile, 0, SEEK_END );
	nBytesInOrigFile = ftell( pDstFile );

	// if the insert offset is greater than the size of the file, 
	// add nNumBytesToInsert of bytes to the end of the file
	if( nInsertByteOffset >= nBytesInOrigFile ) {
		return PadFileWithZeros( pDstFile, nNumBytesToInsert );
	}

	// calculate how many bytes we have to move down in the file
	nBytesToCopy = nBytesInOrigFile - nInsertByteOffset;
	// set our src and dst offset to the last byte in each region
	nSrcOffset = nBytesInOrigFile;
	nDstOffset = nBytesInOrigFile + nNumBytesToInsert;
	
	// expand the end of the file
	if( !PadFileWithZeros( pDstFile, nNumBytesToInsert ) ) {
		return FALSE;
	}

	while( nBytesToCopy ) {
		nBytesAtATime = FMATH_MIN( nBytesToCopy, _BUFFER_SIZE );
		// adjust our file offsets
		nSrcOffset -= nBytesAtATime;
		FASSERT( nSrcOffset >= nInsertByteOffset );
		nDstOffset -= nBytesAtATime;
		// move our file pointer to the source offset
		fseek( pDstFile, nSrcOffset, SEEK_SET );
		// read in nBytesAtATime bytes into memory
		if( fread( _cBuffer, nBytesAtATime, 1, pDstFile ) != 1 ) {
			// could not read the file, error
			return FALSE;
		}
		// move our file pointer to the dest offset
		fseek( pDstFile, nDstOffset, SEEK_SET );
		// write out our data to our dst file
		if( fwrite( _cBuffer, nBytesAtATime, 1, pDstFile ) != 1 ) {
			// could not write to the file, error
			return FALSE;
		}

		nBytesToCopy -= nBytesAtATime;
	}
	// zero out nNumBytesToInsert bytes at nInsertByteOffset
	fseek( pDstFile, nInsertByteOffset, SEEK_SET );
	if( !PadFileWithZeros( pDstFile, nNumBytesToInsert ) ) {
		return FALSE;
	}
	
	return TRUE;
}

// will rearrange the master file to have the file listed in pDesiredOrder first and 
// any left over files last.  If pDesiredOrder == NULL, then the newly produced master
// file will have all of its entries in the same order as the old file, but will be compacted.
// This function works in a temp file and will not delete the original, until the new file
// has been safely written and closed.
BOOL CMasterFileCompile::RearrangeMasterFile( cchar *pszFilename,
											 const CStringList *pDesiredOrder/*=NULL*/,
											 BOOL bIncludeOnlyLoggedFilesInNewMasterFile/*=FALSE*/ ) {
	CMasterFileCompile NewMasterFile;
	u32 i;
	int nNumStrings;
	FILE *pDestFile;
	FDataPrjFile_Entry_t *pOldEntry;
	
	// read the header and entries, also verifies that the file exists
	if( ReadHeaderAndEntries( pszFilename ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
		return FALSE;
	}

	nNumStrings = (pDesiredOrder) ? pDesiredOrder->GetCount() : 0;
	
	// setup the progress bar
	if( m_pProgressInitFcn ) {
		m_pProgressInitFcn( 1 + nNumStrings + m_Header.nNumEntries, m_pUserData );
	}

	// create a new empty master file (this is a temp till we are done)
	CString sNewFilename( pszFilename );
	sNewFilename += ".tmp";
	if( !NewMasterFile.BeginCompilation( sNewFilename, 
										 m_Header.nNumEntries, 
										 FVERSION_GET_PLATFORM_MASK( m_Header.Version.nVersion ),
										 TRUE ) ) {
		return FALSE;
	}
	
	// copy header data into the new master file
	NewMasterFile.m_Header.Version = m_Header.Version;
	NewMasterFile.m_Header.nTgaCompilerVersion = m_Header.nTgaCompilerVersion;
	NewMasterFile.m_Header.nApeCompilerVersion = m_Header.nApeCompilerVersion;
	NewMasterFile.m_Header.nMtxCompilerVersion = m_Header.nMtxCompilerVersion;

	// copy the support entries into the new master file
	for( i=0; i < m_Header.nNumSupportEntries; i++ ) {
		NewMasterFile.m_paSupportEntries[i] = m_paSupportEntries[i];
	}
	NewMasterFile.m_Header.nNumSupportEntries = m_Header.nNumSupportEntries;
	NewMasterFile.m_Header.nNumFreeSupportEntries -= m_Header.nNumSupportEntries;

	if( nNumStrings > 0 ) {
		// walk the string list compiling those files into our new master file
		POSITION Pos;
		CString sEntryName;
		for( Pos = pDesiredOrder->GetHeadPosition(); Pos != NULL; ) {
			sEntryName = pDesiredOrder->GetNext( Pos );
			// find sEntryName in the old master file
			pOldEntry = GetEntryByName( sEntryName );
			if( pOldEntry ) {
				// sEntryName was found in the old master file, create an entry in the new master file for it
				pDestFile = NewMasterFile.GetCompilationDataFilePtr( pOldEntry );
				if( !pDestFile ) {
					// could not create a new entry 
					goto _NEW_MASTER_FILE_ERROR;
				}
				fseek( m_pBinFileStream, pOldEntry->nStartOffset, SEEK_SET );
				if( !CopyFile( pDestFile, m_pBinFileStream, pOldEntry->nNumBytes ) ) {
					// could not copy the data to the new master file
					goto _NEW_MASTER_FILE_ERROR;
				}
			}
			// update our progress bar
			if( m_pProgressStepFcn ) {
				m_pProgressStepFcn( m_pUserData );
			}
		}
	}

	if( !bIncludeOnlyLoggedFilesInNewMasterFile ) {
		// walk the old entries adding entries that were not in the string list
		for( i=0; i < m_Header.nNumEntries; i++ ) {
			pOldEntry = &m_paEntries[i];
			// see if this entry is already in the new master file
			if( !NewMasterFile.GetEntryByName( pOldEntry->szFilename ) ) {
				// this entry needs to be added to the new master file
				pDestFile = NewMasterFile.GetCompilationDataFilePtr( pOldEntry );
				if( !pDestFile ) {
					// could not create a new entry 
					goto _NEW_MASTER_FILE_ERROR;
				}
				fseek( m_pBinFileStream, pOldEntry->nStartOffset, SEEK_SET );
				if( !CopyFile( pDestFile, m_pBinFileStream, pOldEntry->nNumBytes ) ) {
					// could not copy the data to the new master file
					goto _NEW_MASTER_FILE_ERROR;
				}
			}
			// update our progress bar
			if( m_pProgressStepFcn ) {
				m_pProgressStepFcn( m_pUserData );
			}
		}
	}

	// finish the compilation
	if( !NewMasterFile.EndCompilation() ) {
		// could not finish the compilation process
		goto _NEW_MASTER_FILE_ERROR;
	}
	
	// set both files, that way they are both closed
	Reset();
	NewMasterFile.Reset();

	// delete the old master file
	if( !DeleteFile( pszFilename ) ) {
		// could not delete the old master file
		return FALSE;
	}
	// rename the temp master file to the original file's name
	if( rename( sNewFilename, pszFilename ) != 0 ) {
		// could not rename the temp file 
		return FALSE;
	}
	// finally read the header again
	//if( ReadHeaderAndEntries( pszFilename ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
	// !!Nate
	if( ReadHeaderAndEntries( pszFilename, FALSE, TRUE ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
		return FALSE;
	}

	// update our progress bar
	if( m_pProgressStepFcn ) {
		m_pProgressStepFcn( m_pUserData );
	}

	return TRUE;

_NEW_MASTER_FILE_ERROR:
	NewMasterFile.Reset();
	// delete the temp file
	DeleteFile( sNewFilename );
	
	return FALSE;	
}

u32 CMasterFileCompile::LogAllRegularFileEntries( CStringList &rLog, cchar *pszExtToLog/*=NULL*/ ) {

	if( !m_pAllocatedMem ) {
		return 0;
	}

	rLog.RemoveAll();

	BOOL bExtOnly = (pszExtToLog) ? TRUE : FALSE;

	CString sExt = pszExtToLog;
	sExt.MakeLower();

	CString sName;

	u32 i;
	for( i=0; i < m_Header.nNumEntries; i++ ) {
		if( bExtOnly ) {
			sName = m_paEntries[i].szFilename;
			if( sName.Find( sExt ) >= 0 ) {
				rLog.AddTail( sName );
			}
		} else {
            rLog.AddTail( m_paEntries[i].szFilename );
		}
	}

	return rLog.GetCount();
}

u32 CMasterFileCompile::LogAllUnusedRegularFileEntries( const CStringList &rLog, CStringList &rUsedEntries ) {

	if( !m_pAllocatedMem ) {
		return 0;
	}

	rUsedEntries.RemoveAll();

	// walk all of the entries in the masterfile, and if they are not in the used entries list, log them
	u32 i;
	CString sTemp;
	for( i=0; i < m_Header.nNumEntries; i++ ) {
		sTemp = m_paEntries[i].szFilename;
		if( rLog.Find( sTemp ) == NULL ) {
			rUsedEntries.AddTail( sTemp );
		}
	}

	return rUsedEntries.GetCount();
}

/////////////////////////////
// INTERFACE FOR MEMORY FILES

BOOL CMasterFileCompile::BeginCompilation( cchar *pszFilename, u32 nNumFilesToCompile, u32 nPlatformFlag, BOOL bRebuildAll/*=FALSE*/ ) {
	u32 i, nAddedBytes, nBytesAtATime, nElementsToAdd;
	BOOL bReadSwapEndian, bWriteSwapEndian;

	Reset();

	if( bRebuildAll ) {
		nNumFilesToCompile = FMATH_MAX( FDATA_PRJFILE_MIN_ENTRY_COUNT, nNumFilesToCompile );
		goto _REBUILD_NEW_HEADER;
	} else {
		// open an existing file named pszFilename for edit
		m_pBinFileStream = fopen( pszFilename, "r+b" );
		if( !m_pBinFileStream ) {
			// file must not already exist, create a brand new one
			nNumFilesToCompile = FMATH_MAX( FDATA_PRJFILE_MIN_ENTRY_COUNT, nNumFilesToCompile );
			goto _REBUILD_NEW_HEADER;
		}	
		// read in the current header
		fseek( m_pBinFileStream, 0, SEEK_SET );
		if( ReadHeader( bReadSwapEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
			// could not read the header
			Reset();
			return FALSE;
		}
		// check the master file platform type
		if( (FVERSION_GET_PLATFORM_MASK( m_Header.Version.nVersion ) & nPlatformFlag) == 0 ) {
			// the master file opened is not the correct platform
			Reset();
			return FALSE;
		}
		// check the master file version info
		if( ( FVERSION_GET_MAJOR_NUMBER( m_Header.Version.nVersion ) != FDATA_PRJFILE_MAJOR ) ||
			( FVERSION_GET_MINOR_NUMBER( m_Header.Version.nVersion ) != FDATA_PRJFILE_MINOR ) ||
			( FVERSION_GET_SUB_NUMBER( m_Header.Version.nVersion ) != FDATA_PRJFILE_SUB ) ) {
			// the master file version is not correct
			Reset();
			return FALSE;
		}		
		// are there enough open spots for these new files?
		if( m_Header.nNumFreeEntries < nNumFilesToCompile ||
			m_Header.nNumFreeSupportEntries < nNumFilesToCompile ) {
			/////////////////////////////////////////////////////////////
			// not enough open spots, we need to add more available spots
			/////////////////////////////////////////////////////////////

			////////////////////////////////////////////////////////////
			// allocate memory to hold the entry and support entry array
			nBytesAtATime = (m_Header.nNumEntries * sizeof( FDataPrjFile_Entry_t )) + 
							(m_Header.nNumSupportEntries * sizeof( FDataPrjFile_SupportEntry_t ));
			m_pAllocatedMem = new u8[nBytesAtATime];
			if( !m_pAllocatedMem ) {
				// could not allocate enough memory for the entries
				Reset();
				return FALSE;
			}
			fang_MemZero( m_pAllocatedMem, nBytesAtATime );

			m_paEntries = (FDataPrjFile_Entry_t *)m_pAllocatedMem;
			m_paSupportEntries = (FDataPrjFile_SupportEntry_t *)&m_paEntries[m_Header.nNumEntries];

			///////////////////////
			// read the old entries
			fseek( m_pBinFileStream, sizeof( FDataPrjFile_Header_t ), SEEK_SET );
			if( ReadEntryArray( (u8 *)m_paEntries, m_Header.nNumEntries, m_Header.nNumEntries, bReadSwapEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
				// could not read the entry array
				Reset();
				return FALSE;
			}

			///////////////////////////////
			// read the old support entries
			fseek( m_pBinFileStream, 
				   sizeof( FDataPrjFile_Header_t ) + (sizeof( FDataPrjFile_Entry_t ) * (m_Header.nNumEntries + m_Header.nNumFreeEntries) ),
				   SEEK_SET );
			if( ReadSupportEntryArray( (u8 *)m_paSupportEntries, m_Header.nNumSupportEntries, m_Header.nNumSupportEntries, bReadSwapEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
				// could not read the support entry array
				Reset();
				return FALSE;
			}

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// compute the number of bytes to add, remember to keep the data offset aligned by FDATA_PRJFILE_FILE_ALIGNMENT
			nElementsToAdd = nNumFilesToCompile << 1;
			if( nElementsToAdd < 1024 ) {
				// if we are going to do this, let's make it worth the effort
				nElementsToAdd = 1024;
			}
			nAddedBytes = nElementsToAdd * sizeof( FDataPrjFile_Entry_t );
			nAddedBytes += nElementsToAdd * sizeof( FDataPrjFile_SupportEntry_t );
			nAddedBytes = FMATH_BYTE_ALIGN_UP( nAddedBytes, FDATA_PRJFILE_FILE_ALIGNMENT );

			/////////////////////////////////////////////////////
			// insert bytes into the file, moving everything down
			if( !InsertBytesIntoAFile( m_pBinFileStream, m_Header.nDataOffset, nAddedBytes ) ) {
				Reset();
				return FALSE;	
			}
			
			/////////////////////
			// fixup the m_Header
			m_Header.nBytesInFile += nAddedBytes;
			m_Header.nDataOffset += nAddedBytes;
			m_Header.nNumFreeEntries += nElementsToAdd;
			m_Header.nNumFreeSupportEntries += nElementsToAdd;

			//////////////////////////////////////
			// adjust each entry's starting offset
			for( i=0; i < m_Header.nNumEntries; i++ ) {
				m_paEntries[i].nStartOffset += nAddedBytes;
			}

			////////////////////////////////////////////////////////////////
			// zero out everything in master file upto the start of the data
			fseek( m_pBinFileStream, 0, SEEK_SET );
			if( !PadFileWithZeros( m_pBinFileStream, (m_Header.nDataOffset - nAddedBytes) ) ) {
				Reset();
				return FALSE;
			}

			// cache some key values we will need to write out the arrays
			u32 nNumEntries = m_Header.nNumEntries;
			u32 nSupportEntryByteOffset = sizeof( FDataPrjFile_Header_t ) + 
										  (sizeof( FDataPrjFile_Entry_t ) * (m_Header.nNumEntries + m_Header.nNumFreeEntries) );
			u32 nNumSupportEntries = m_Header.nNumSupportEntries;
			/////////////////////////////
			// write out the new m_Header		
			fseek( m_pBinFileStream, 0, SEEK_SET );
			if( WriteHeader( bWriteSwapEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
				Reset();
				return FALSE;
			}
			FASSERT( bWriteSwapEndian == bReadSwapEndian );// these should be equal!!!

			////////////////////////////////
			// write out the new entry array
			if( WriteEntryArray( m_paEntries, nNumEntries, bWriteSwapEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
				Reset();
				return FALSE;
			}
			
			////////////////////////////////////////
			// write out the new support entry array
			fseek( m_pBinFileStream, nSupportEntryByteOffset, SEEK_SET );
			if( WriteSupportEntryArray( m_paSupportEntries, nNumSupportEntries, bWriteSwapEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
				Reset();
				return FALSE;
			}
			
			////////////////////////
			// free our temp memory
			delete [] m_pAllocatedMem;
			m_pAllocatedMem = NULL;
			m_paEntries = NULL;
			m_paSupportEntries = NULL;
		}
	}
	// read in the entries
	if( ReadHeaderAndEntries( pszFilename ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
		Reset();
		return FALSE;
	}

	return TRUE;

_REBUILD_NEW_HEADER:
	
	// open a new file named pszFilename for writting
	m_pBinFileStream = fopen( pszFilename, "wb" );
	if( m_pBinFileStream ) {
		if( !WriteNewEmptyMasterFile( m_pBinFileStream, nPlatformFlag, nNumFilesToCompile ) ) {
			Reset();
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CMasterFileCompile::DoesFileInfoNeedCompiling( const CFileInfo *pFileInfo, const CString &rsEntryFilename ) {
	const FDataPrjFile_Entry_t *pEntry;

	pEntry = GetEntryByName( rsEntryFilename );
	if( pEntry ) {
		if( DoModifiedTimesMatch( pEntry, pFileInfo ) ) {
			return FALSE;
		}
	}
	return TRUE;
}

// Will get/create an entry named rsEntryFilename using data contained by pFileInfo.
// If nNumBytesNeeded is 0, then the entry will be set to pFileInfo->GetLength() number 
// of bytes.  It is assumed that pFileInfo needs to be compiled, the caller should 
// call DoesFileInfoNeedCompiling() before calling this function.  If an entry 
// already exists then if nNumBytesNeeded will fit into the current spot, then it 
// will be zeroed out and a pointer returned to it, otherwise the data will be zeroed out
// and a newly created (and zeroed out) buffer at the end of the file will be created, handling the 
// alignment issues.
// It is further assumed that BeginCompilation has been called before calling this function.
// Returns NULL if an error occurred and a file pointer could not be gotten, otherwise
// the file pointer returned should be written to with NO MORE than nNumBytesNeeded bytes.
FILE *CMasterFileCompile::GetCompilationDataFilePtr( const CFileInfo *pFileInfo, const CString &rsEntryFilename, u32 nNumBytesNeeded/*=0*/, u32 nCRC/*=0*/, u16 nEntryFlags/*=0*/ ) {
	u32 nBytesUsed, nMaxBytes;
	BOOL bNewEntry, bUpdateFileBytes;

	if( !m_pBinFileStream || !pFileInfo || !m_pAllocatedMem || rsEntryFilename.IsEmpty() ) {
		// BeginCompilation needs to be called before calling this function
		return NULL;
	}

	// make sure that the entry name is not too long
	CString sEntryName( rsEntryFilename );
	sEntryName.MakeLower();
	if( !IsEntryNameValid( sEntryName ) ) {
		// filename is too long or too short
		return NULL;
	}

	if( !nNumBytesNeeded ) {
		// set nNumBytesNeeded to the filesize
		nNumBytesNeeded = pFileInfo->GetLength();
		if( !nNumBytesNeeded ) {
			return NULL;
		}
	}
	CTime ModifiedTime( pFileInfo->GetLastWriteTime() );

	FDataPrjFile_Entry_t *pEntry = GetEntryByName( sEntryName );
	if( pEntry ) {
		// the file exists, will it fit into the old spot, or should we create a new one
		bNewEntry = FALSE;

		nMaxBytes = pEntry->nStartOffset + pEntry->nNumBytes;
		nMaxBytes = FMATH_BYTE_ALIGN_UP( nMaxBytes, FDATA_PRJFILE_FILE_ALIGNMENT );
		nMaxBytes -= pEntry->nStartOffset;
						
		if( nNumBytesNeeded <= nMaxBytes ) {
			// write the new data in the old data's spot
			bUpdateFileBytes = FALSE;
			fseek( m_pBinFileStream, pEntry->nStartOffset, SEEK_SET );			
		} else {
			// write the new data at the end of the file
			bUpdateFileBytes = TRUE;

			// zero out the old data location
			fseek( m_pBinFileStream, pEntry->nStartOffset, SEEK_SET );
			PadFileWithZeros( m_pBinFileStream, pEntry->nNumBytes );// not the end of the world if this fails...just the old data will still be there, oh well
			
			// position the file pointer
			fseek( m_pBinFileStream, 0, SEEK_END );
		}
	} else {
		if( !m_Header.nNumFreeEntries ) {
			return NULL;
		}
		// add a new entry to the end of the file
		bNewEntry = TRUE;
		bUpdateFileBytes = TRUE;

		pEntry = &m_paEntries[m_Header.nNumEntries];
		fseek( m_pBinFileStream, 0, SEEK_END );		
	}

	// fill in the entry
	pEntry->nFlags = nEntryFlags;
	pEntry->nModifiedTime = (u32)ModifiedTime.GetTime();
	pEntry->nCRC = nCRC;
	pEntry->nNumBytes = nNumBytesNeeded;
	pEntry->nStartOffset = ftell( m_pBinFileStream );
	fclib_strncpy( pEntry->szFilename, (cchar *)sEntryName, FDATA_PRJFILE_FILENAME_LEN );
			
	// compute number of total bytes used
	nBytesUsed = (pEntry->nStartOffset + pEntry->nNumBytes);
	nBytesUsed = FMATH_BYTE_ALIGN_UP( nBytesUsed, FDATA_PRJFILE_FILE_ALIGNMENT );
	nBytesUsed -= pEntry->nStartOffset;

	// write out nBytesUsed at the current file position		
	if( !PadFileWithZeros( m_pBinFileStream, nBytesUsed ) ) {
		// couldn't write out zeros at the current file position, delete the entry
		if( !bNewEntry ) {
			DeleteExistingEntry( pEntry );
		} else {
			fang_MemZero( pEntry, sizeof( FDataPrjFile_Entry_t ) );	
		}
		return NULL;
	}
	
	// position the file pointer to the start of pEntry
	// DO NOT MOVE, BECAUSE AFTER SORTING THE HEADER pEntry COULD POINT TO AN OLD ENTRY
	fseek( m_pBinFileStream, pEntry->nStartOffset, SEEK_SET );

	// update the header vars
	if( bNewEntry ) {
		m_Header.nNumEntries++;
		m_Header.nNumFreeEntries--;

		// we must resort that that future lookups work correctly
		if( m_Header.nNumEntries > 1 ) {
			fclib_QSort( m_paEntries, m_Header.nNumEntries, sizeof( FDataPrjFile_Entry_t ), _QSortCompareFcn );
		}
	}
	if( bUpdateFileBytes ) {
		m_Header.nBytesInFile += nBytesUsed;	
	}

	return m_pBinFileStream;
}

// deletes an entry and zeros out the data that the entry used to use
void CMasterFileCompile::DeleteEntry( const CString &rsEntryFilename ) {
		
	if( !m_pBinFileStream || !m_pAllocatedMem || rsEntryFilename.IsEmpty() ) {
		// BeginCompilation needs to be called before calling this function
		return;
	}

	// make sure that the entry name is not too long
	CString sEntryName( rsEntryFilename );
	sEntryName.MakeLower();
	if( !IsEntryNameValid( sEntryName ) ) {
		// filename is too long
		return;
	}
	
	FDataPrjFile_Entry_t *pEntry = GetEntryByName( sEntryName );
	if( pEntry ) {
		DeleteExistingEntry( pEntry );
	}
}

// pEntry must be a valid used entry obtained by calling GetEntryByIndex() or GetEntryByName()
void CMasterFileCompile::DeleteExistingEntry( FDataPrjFile_Entry_t *pEntry ) {
	FDataPrjFile_Entry_t *pLastEntry;

	if( !m_pBinFileStream || !m_pAllocatedMem || !pEntry ) {
		// BeginCompilation needs to be called before calling this function
		return;
	}
	// zero out the data held in the last location (this will allow us to zip up the master file better)
	fseek( m_pBinFileStream, pEntry->nStartOffset, SEEK_SET );
	PadFileWithZeros( m_pBinFileStream, pEntry->nNumBytes );

	// grab the last entry
	pLastEntry = &m_paEntries[m_Header.nNumEntries-1];
	if( pEntry != pLastEntry ) {
		// copy pLastEntry into pEntry
		*pEntry = *pLastEntry;								
	}
	// zero out the last entry
	fang_MemZero( (void *)pLastEntry, sizeof( FDataPrjFile_Entry_t ) );

	// reduce the header entry counts
	m_Header.nNumEntries--;
	m_Header.nNumFreeEntries++;

	// resort the header entries so that future lookups work
	if( m_Header.nNumEntries > 1 ) {
		fclib_QSort( m_paEntries, m_Header.nNumEntries, sizeof( FDataPrjFile_Entry_t ), _QSortCompareFcn );
	}
}

// pFileInfo must not be NULL.
// This function will see if a support entry exists for pFileInfo, if
// so, then if will update it (if needed) to match the time contained in
// pFileInfo.  If pFileInfo does not have a support entry then one will be 
// created for it (if possible).
void CMasterFileCompile::UpdateOrAddSupportEntry( const CFileInfo *pFileInfo ) {
		
	if( !m_pBinFileStream || !m_pAllocatedMem || !pFileInfo || !m_paSupportEntries ) {
		// BeginCompilation needs to be called before calling this function
		return;
	}
	CString sEntryname( pFileInfo->GetFileName() );
	sEntryname.MakeLower();
	FDataPrjFile_SupportEntry_t *pSupportEntry = GetSupportEntryByName( sEntryname );

	if( pSupportEntry ) {
		// the entry exists, see if our time matches
		if( !DoModifiedTimesMatch( pSupportEntry, pFileInfo ) ) {
			// the times do not match, an update is needed
			CTime FileTime( pFileInfo->GetLastWriteTime() );
			pSupportEntry->nModifiedTime = (u32)FileTime.GetTime();
		}
	} else {
		// an entry doesn't already exist, create one
		if( m_Header.nNumFreeSupportEntries ) {
			pSupportEntry = &m_paSupportEntries[ m_Header.nNumSupportEntries ];
			
			fclib_strncpy( pSupportEntry->szFilename, sEntryname, FDATA_PRJFILE_FILENAME_LEN );
			
			CTime FileTime( pFileInfo->GetLastWriteTime() );
			pSupportEntry->nModifiedTime = (u32)FileTime.GetTime();

			m_Header.nNumFreeSupportEntries--;
			m_Header.nNumSupportEntries++;
		}
	}
}

// writes out the header and the entries to disk, m_pBinFileStream should already be open
// to a valid master file and the header and entries should be ready to be written to disk.
// closes the open file upon completion
BOOL CMasterFileCompile::EndCompilation() {
	
	if( !m_pBinFileStream || !m_pAllocatedMem ) {
		return FALSE;
	}
	// cache a few vars, just in case we switch the Endian format during writing
	BOOL bSwitchEndian;
	u32 nNumEntries, nNumFreeEntries, nNumSupportEntries;
	nNumEntries = m_Header.nNumEntries;
	nNumFreeEntries = m_Header.nNumFreeEntries;
	nNumSupportEntries = m_Header.nNumSupportEntries;

	// write out our new m_Header
	fseek( m_pBinFileStream, 0, SEEK_SET );
	if( WriteHeader( bSwitchEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
		return FALSE;
	}
	
	// qsort the entry table
	if( nNumEntries ) {
		if( nNumEntries > 1 ) {
			fclib_QSort( m_paEntries, nNumEntries, sizeof( FDataPrjFile_Entry_t ), _QSortCompareFcn );
		}
	
		// write out our new entry table
		if( WriteEntryArray( m_paEntries, nNumEntries, bSwitchEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
			return FALSE;
		}		
	}

	// write out our new support entry table
	if( nNumSupportEntries ) {
		fseek( m_pBinFileStream, 
			   sizeof( FDataPrjFile_Header_t ) + (sizeof( FDataPrjFile_Entry_t ) * (nNumEntries + nNumFreeEntries)),
			   SEEK_SET );
		if( WriteSupportEntryArray( m_paSupportEntries, nNumSupportEntries, bSwitchEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
			return FALSE;
		}
	}
	fclose( m_pBinFileStream );
	m_pBinFileStream = NULL;

	return TRUE;
}

/////////////////////////////
// PRIVATE SUPPORT FUNCTIONS

// returns TRUE if a new empty file was written to disk
// this new master file will have nMinEntries number of 
// unused entries in it.
// this function, if successfull, will set the global header and 
// m_paEntries to their correct values
BOOL CMasterFileCompile::WriteNewEmptyMasterFile( FILE *pFileStream, u32 nPlatformFlag, u32 nMinEntries ) {
	u32 nFileOffset, nBytes;
	
	// fill in our m_Header
	fang_MemZero( &m_Header, sizeof( FDataPrjFile_Header_t ) );

	// fill the header's version info
	m_Header.Version.nSignature = FVERSION_FILE_SIGNATURE;
	if( nPlatformFlag & FVERSION_PLATFORM_FLAG_XBOX ) {
		// this is a xbox platform
		m_Header.Version.nVersion = FDATA_PRJFILE_XB_VERSION;
		m_Header.nTgaCompilerVersion = FDATA_PRJFILE_XB_TGA_VERSION;
		m_Header.nApeCompilerVersion = FDATA_PRJFILE_XB_APE_VERSION;

	} else if( nPlatformFlag & FVERSION_PLATFORM_FLAG_GC ) {
		// this is a gc platform
		m_Header.Version.nVersion = FDATA_PRJFILE_GC_VERSION;
		m_Header.nTgaCompilerVersion = FDATA_PRJFILE_GC_TGA_VERSION;
		m_Header.nApeCompilerVersion = FDATA_PRJFILE_GC_APE_VERSION;

#ifdef _MMI_TARGET_PS2
    } else if( nPlatformFlag & FVERSION_PLATFORM_FLAG_PS2 ) {
		// this is a PS2 platform
		m_Header.Version.nVersion = FDATA_PRJFILE_PS_VERSION;
		m_Header.nTgaCompilerVersion = FDATA_PRJFILE_PS_TGA_VERSION;
		m_Header.nApeCompilerVersion = FDATA_PRJFILE_PS_APE_VERSION;
#endif
	} else {
		FASSERT_NOW;
		return FALSE;
	}
	m_Header.nMtxCompilerVersion = FDATA_PRJFILE_MTX_VERSION;
	m_Header.nCsvCompilerVersion = FDATA_PRJFILE_CSV_VERSION;
	m_Header.nFntCompilerVersion = FDATA_PRJFILE_FNT_VERSION;
	m_Header.nSmaCompilerVersion = FDATA_PRJFILE_SMA_VERSION;
	m_Header.nGtCompilerVersion = FDATA_PRJFILE_GT_VERSION;
	m_Header.nWvbCompilerVersion = FDATA_PRJFILE_WVB_VERSION;
	m_Header.nFprCompilerVersion = FDATA_PRJFILE_FPR_VERSION;
	m_Header.nCamCompilerVersion = FDATA_PRJFILE_CAM_VERSION;

	// calculate the starting file offset for the first file
	nFileOffset = sizeof( FDataPrjFile_Header_t );
	nFileOffset += (sizeof( FDataPrjFile_Entry_t ) * nMinEntries);
	nFileOffset += (sizeof( FDataPrjFile_SupportEntry_t ) * nMinEntries);
	nFileOffset = FMATH_BYTE_ALIGN_UP( nFileOffset, FDATA_PRJFILE_FILE_ALIGNMENT );
	
	// finsh setting up the header struct
	m_Header.nBytesInFile = nFileOffset;
	m_Header.nDataOffset = nFileOffset;
	m_Header.nNumFreeEntries = nMinEntries; 
	m_Header.nNumFreeSupportEntries = nMinEntries;
	
	// write out our m_Header to our file
	fseek( pFileStream, 0, SEEK_SET );
	BOOL bSwitchEndian;
	if( WriteHeader( bSwitchEndian ) != MASTER_FILE_COMPILE_ERROR_NONE ) {
		// could not write to the file, return an error
		return FALSE;
	}
	if( bSwitchEndian ) {
		// we switched the endian format before writing out the header, switch it back so that PASM can use it
		m_Header.ChangeEndian();
	}
		
	// allocate memory to hold all of the entries
	nBytes = nFileOffset - sizeof( FDataPrjFile_Header_t );
	m_pAllocatedMem = new u8[nBytes];
	if( !m_pAllocatedMem ) {
		// couldn't allocate enough memory for the entry table
		return FALSE;
	}
	fang_MemZero( m_pAllocatedMem, nBytes );
	m_paEntries = (FDataPrjFile_Entry_t *)m_pAllocatedMem;
	m_paSupportEntries = (FDataPrjFile_SupportEntry_t *)&m_paEntries[m_Header.nNumFreeEntries];

	// write out the entry table to our file
	if( fwrite( m_pAllocatedMem, nBytes, 1, pFileStream ) != 1 ) {
		// free our memory
		delete [] m_pAllocatedMem;
		m_pAllocatedMem = NULL;
		m_paEntries = NULL;
		m_paSupportEntries = NULL;
		// could not write to the file, return an error
		return FALSE;
	}

	return TRUE;
}

// Will create a new entry based on the data contained in pEntry
// It is assumed that pEntry doesn't already exist but the following field must be set:
//		szFilename
//		nNumBytes
//		nModifiedTime
// It is assumed that BeginCompilation has been called before calling this function.
// Returns NULL if an error occurred and a file pointer could not be gotten, otherwise
// the file pointer returned should be written to with NO MORE than pEntry->nNumBytes bytes.
FILE *CMasterFileCompile::GetCompilationDataFilePtr( const FDataPrjFile_Entry_t *pEntry ) {
	u32 nBytesUsed;
	FDataPrjFile_Entry_t *pNewEntry;
	
	if( !m_pBinFileStream || 
		!pEntry || 
		!m_pAllocatedMem ) {
		// BeginCompilation needs to be called before calling this function
		return NULL;
	}
	// make sure that pEntry's required fields have been filled in
	if( pEntry->nNumBytes == 0 ||
		pEntry->szFilename[0] == 0 ||
		pEntry->nModifiedTime == 0 ) {
		// pEntry doesn't have the necessary fields field in
		return NULL;
	}
	
	// make sure that there is a free entry
	if( !m_Header.nNumFreeEntries ) {
		return NULL;
	}
	// add a new entry to the end of the file
	pNewEntry = &m_paEntries[m_Header.nNumEntries];
	fseek( m_pBinFileStream, 0, SEEK_END );		
	
	// fill in the entry
	fclib_strncpy( pNewEntry->szFilename, pEntry->szFilename, FDATA_PRJFILE_FILENAME_LEN );
	pNewEntry->nStartOffset = ftell( m_pBinFileStream );
	pNewEntry->nFlags = pEntry->nFlags;
	pNewEntry->nNumBytes = pEntry->nNumBytes;
	pNewEntry->nModifiedTime = pEntry->nModifiedTime;
	pNewEntry->nCRC = pEntry->nCRC;
	
	// compute number of total bytes used
	nBytesUsed = (pNewEntry->nStartOffset + pNewEntry->nNumBytes);
	nBytesUsed = FMATH_BYTE_ALIGN_UP( nBytesUsed, FDATA_PRJFILE_FILE_ALIGNMENT );
	nBytesUsed -= pNewEntry->nStartOffset;

	// write out nBytesUsed at the current file position		
	if( !PadFileWithZeros( m_pBinFileStream, nBytesUsed ) ) {
		// couldn't write out zeros at the current file position, delete the entry
		fang_MemZero( pNewEntry, sizeof( FDataPrjFile_Entry_t ) );	
		return NULL;
	}
	
	// position the file pointer to the start of pEntry
	// DO NOT MOVE, BECAUSE AFTER SORTING THE HEADER pEntry COULD POINT TO AN OLD ENTRY
	fseek( m_pBinFileStream, pNewEntry->nStartOffset, SEEK_SET );

	// update the header vars
	m_Header.nNumEntries++;
	m_Header.nNumFreeEntries--;
	m_Header.nBytesInFile += nBytesUsed;

	// we must resort that that future lookups work correctly
	if( m_Header.nNumEntries > 1 ) {
		fclib_QSort( m_paEntries, m_Header.nNumEntries, sizeof( FDataPrjFile_Entry_t ), _QSortCompareFcn );
	}
		
	return m_pBinFileStream;
}

// reads the header struct (assumes that the file is already opened and positioned to the correct
// location).  This function will test the signature and will switch the data to little endian if 
// necessary.
MasterFileCompile_ErrorCodes_e CMasterFileCompile::ReadHeader( BOOL &rbSwitchedEndian ) {
	
	if( fread( &m_Header, sizeof( FDataPrjFile_Header_t ), 1, m_pBinFileStream ) != 1 ) {
		return MASTER_FILE_COMPILE_ERROR_READING;	
	}
	rbSwitchedEndian = FALSE;

	// check the master file signature
	if( m_Header.Version.nSignature != FVERSION_FILE_SIGNATURE ) {
		// the master file opened is not a valid fang master file, see if our data is big endian
		m_Header.ChangeEndian();
		rbSwitchedEndian = TRUE;
		
		// retest the signature
		if( m_Header.Version.nSignature != FVERSION_FILE_SIGNATURE ) {
			return MASTER_FILE_COMPILE_ERROR_INVALID_SIGNATURE;
		}
		if( !( FVERSION_GET_PLATFORM_MASK( m_Header.Version.nVersion ) & FVERSION_PLATFORM_FLAG_GC ) ) {
			// only a gc master file should be big endian, something is wrong here
			return MASTER_FILE_COMPILE_ERROR_INVALID_SIGNATURE;
		}
	}

	return MASTER_FILE_COMPILE_ERROR_NONE;
}

MasterFileCompile_ErrorCodes_e CMasterFileCompile::ReadEntryArray( u8 *pDest, u32 nEntriesToRead,
																   u32 nNumUsedEntries, BOOL bSwitchEndian ) {
	u32 i;
	FDataPrjFile_Entry_t *pEntry;

	if( fread( pDest, sizeof( FDataPrjFile_Entry_t ), nEntriesToRead, m_pBinFileStream ) != nEntriesToRead ) {
		return MASTER_FILE_COMPILE_ERROR_READING;	
	}
	pEntry = (FDataPrjFile_Entry_t *)pDest;

	if( bSwitchEndian ) {
		FASSERT( nNumUsedEntries <= nEntriesToRead );
		for( i=0; i < nNumUsedEntries; i++ ) {
			pEntry[i].ChangeEndian();
		}
	}
	return MASTER_FILE_COMPILE_ERROR_NONE;
}

MasterFileCompile_ErrorCodes_e CMasterFileCompile::ReadSupportEntryArray( u8 *pDest, u32 nEntriesToRead, 
																		  u32 nNumUsedEntries, BOOL bSwitchEndian ) {
	u32 i;
	FDataPrjFile_SupportEntry_t *pEntry;

	if( fread( pDest, sizeof( FDataPrjFile_SupportEntry_t ), nEntriesToRead, m_pBinFileStream ) != nEntriesToRead ) {
		return MASTER_FILE_COMPILE_ERROR_READING;	
	}
	pEntry = (FDataPrjFile_SupportEntry_t *)pDest;

	if( bSwitchEndian ) {
		FASSERT( nNumUsedEntries <= nEntriesToRead );
		for( i=0; i < nNumUsedEntries; i++ ) {
			pEntry[i].ChangeEndian();
		}
	}
	return MASTER_FILE_COMPILE_ERROR_NONE;
}

MasterFileCompile_ErrorCodes_e CMasterFileCompile::WriteHeader( BOOL &rbSwitchedEndian ) {

	// see if we should switch the endian format
	if( FVERSION_GET_PLATFORM_MASK( m_Header.Version.nVersion ) & FVERSION_PLATFORM_FLAG_GC ) {
		rbSwitchedEndian = TRUE;
		m_Header.ChangeEndian();
	} else {
		rbSwitchedEndian = FALSE;
	}
	if( fwrite( &m_Header, sizeof( FDataPrjFile_Header_t ), 1, m_pBinFileStream ) != 1 ) {
		return MASTER_FILE_COMPILE_ERROR_WRITING;	
	}
	return MASTER_FILE_COMPILE_ERROR_NONE;
}

MasterFileCompile_ErrorCodes_e CMasterFileCompile::WriteEntryArray( FDataPrjFile_Entry_t *pEntry, u32 nEntriesToWrite, BOOL bSwitchEndian ) {
	u32 i;

	if( bSwitchEndian ) {
		for( i=0; i < nEntriesToWrite; i++ ) {
			pEntry[i].ChangeEndian();
		}
	}
	if( fwrite( pEntry, sizeof( FDataPrjFile_Entry_t ), nEntriesToWrite, m_pBinFileStream ) != nEntriesToWrite ) {
		return MASTER_FILE_COMPILE_ERROR_WRITING;	
	}
	return MASTER_FILE_COMPILE_ERROR_NONE;
}

MasterFileCompile_ErrorCodes_e CMasterFileCompile::WriteSupportEntryArray( FDataPrjFile_SupportEntry_t *pEntry, u32 nEntriesToWrite, BOOL bSwitchEndian ) {
	u32 i;

	if( bSwitchEndian ) {
		for( i=0; i < nEntriesToWrite; i++ ) {
			pEntry[i].ChangeEndian();
		}
	}
	if( fwrite( pEntry, sizeof( FDataPrjFile_SupportEntry_t ), nEntriesToWrite, m_pBinFileStream ) != nEntriesToWrite ) {
		return MASTER_FILE_COMPILE_ERROR_WRITING;	
	}
	return MASTER_FILE_COMPILE_ERROR_NONE;
}

//==================
// private functions

// return values:
// < 0 elem1 less than elem2 
// 0 elem1 equivalent to elem2 
// > 0 elem1 greater than elem2 
static int _QSortCompareFcn( const void *arg1, const void *arg2 ) {
	FDataPrjFile_Entry_t *p1, *p2;

	p1 = (FDataPrjFile_Entry_t *)arg1;
	p2 = (FDataPrjFile_Entry_t *)arg2;

   // Compare all of both strings:
   return fclib_stricmp( p1->szFilename, p2->szFilename );
}