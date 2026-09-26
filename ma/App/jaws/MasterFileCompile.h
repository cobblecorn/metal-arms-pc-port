//////////////////////////////////////////////////////////////////////////////////////
// MasterFileCompile.h - 
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
#ifndef _MASTER_FILE_COMPILE_H_
#define _MASTER_FILE_COMPILE_H_ 1

#include "fang.h"
#include "FileInfo.h"
#include "fdata.h"

typedef void MasterFileCompile_ProgressBarInitFcn_t( u32 nNumElements, void *pUserData );
typedef void MasterFileCompile_ProgressBarStepFcn_t( void *pUserData );

typedef enum {
	MASTER_FILE_COMPILE_ERROR_NONE = 0,
	MASTER_FILE_COMPILE_ERROR_OPENING,
	MASTER_FILE_COMPILE_ERROR_READING,
	MASTER_FILE_COMPILE_ERROR_WRITING,
	MASTER_FILE_COMPILE_ERROR_INVALID_SIGNATURE,
	MASTER_FILE_COMPILE_ERROR_WRONG_VERSION,
	MASTER_FILE_COMPILE_ERROR_ALLOCATING_MEMORY,
	MASTER_FILE_COMPILE_ERROR_WRONG_FILESIZE

} MasterFileCompile_ErrorCodes_e;

class CMasterFileCompile {
public:
	///////////////////////////
	// INIT & CLEANUP FUNCTIONS
	CMasterFileCompile( void *pUserData=NULL, 
						MasterFileCompile_ProgressBarInitFcn_t *pProgressInit=NULL, 
						MasterFileCompile_ProgressBarStepFcn_t *pProgressStep=NULL );
	~CMasterFileCompile();
	void Reset();

	u8 GetPlatformMask();

	//////////////////////////////////////
	// DATA EXTRACTION & UTILITY FUNCTIONS
	static BOOL IsEntryNameValid( const CString &rsEntryName );
	//MasterFileCompile_ErrorCodes_e ReadHeaderAndEntries( cchar *pszFilename, BOOL bReadOnly=FALSE );
	// !!Nate
	MasterFileCompile_ErrorCodes_e ReadHeaderAndEntries( cchar *pszFilename, BOOL bReadOnly=FALSE, BOOL bAutoClose=FALSE );
	u32 GetTgaVersionFromHeader();
	u32 SetTgaVersion( u32 nVersion );
	
	u32 GetApeVersionFromHeader();
	u32 SetApeVersion( u32 nVersion );
	
	u32 GetMtxVersionFromHeader();
	u32 SetMtxVersion( u32 nVersion );

	u32 GetCsvVersionFromHeader();
	u32 SetCsvVersion( u32 nVersion );

	u32 GetFntVersionFromHeader();
	u32 SetFntVersion( u32 nVersion );

	u32 GetSmaVersionFromHeader();
	u32 SetSmaVersion( u32 nVersion );

	u32 GetGtVersionFromHeader();
	u32 SetGtVersion( u32 nVersion );

	u32 GetWvbVersionFromHeader();
	u32 SetWvbVersion( u32 nVersion );

	u32 GetFprVersionFromHeader();
	u32 SetFprVersion( u32 nVersion );

	u32 GetCamVersionFromHeader();
	u32 SetCamVersion( u32 nVersion );

	// regular entries
	u32 GetNumEntries();
	const FDataPrjFile_Entry_t *GetEntryByIndex( u32 nIndex );
	FDataPrjFile_Entry_t *GetEntryByName( cchar *pszFilename );
	static BOOL DoModifiedTimesMatch( const FDataPrjFile_Entry_t *pEntry, const CFileInfo *pFileInfo );
	
	// support entries
	u32 GetNumSupportEntries();
	const FDataPrjFile_SupportEntry_t *GetSupportEntryByIndex( u32 nIndex );
	FDataPrjFile_SupportEntry_t *GetSupportEntryByName( cchar *pszFilename );
	static BOOL DoModifiedTimesMatch( const FDataPrjFile_SupportEntry_t *pEntry, const CFileInfo *pFileInfo );
	
	// file functions
	static BOOL CopyFile( FILE *pDstFile, FILE *pSrcFile, u32 nSrcBytes=0 );	
	static BOOL PadFileWithZeros( FILE *pFileStream, u32 nNumBytes );
	static BOOL InsertBytesIntoAFile( FILE *pDstFile, u32 nInsertByteOffset, u32 nNumBytesToInsert );
	
	// optimization functions
	BOOL RearrangeMasterFile( cchar *pszFilename, const CStringList *pDesiredOrder=NULL, BOOL bIncludeOnlyLoggedFilesInNewMasterFile=FALSE );

	// logging functions
	u32 LogAllRegularFileEntries( CStringList &rLog, cchar *pszExtToLog=NULL );
	u32 LogAllUnusedRegularFileEntries( const CStringList &rLog, CStringList &rUsedEntries );

	// compilation functions
	BOOL BeginCompilation( cchar *pszFilename, u32 nNumFilesToCompile, u32 nPlatformFlag, BOOL bRebuildAll=FALSE );
	BOOL DoesFileInfoNeedCompiling( const CFileInfo *pFileInfo, const CString &rsEntryFilename );
	FILE *GetCompilationDataFilePtr( const CFileInfo *pFileInfo, const CString &rsEntryFilename, u32 nNumBytesNeeded=0, u32 nCRC=0, u16 nEntryFlags=0 );
	void DeleteEntry( const CString &rsEntryFilename );
	void DeleteExistingEntry( FDataPrjFile_Entry_t *pEntry );
	void UpdateOrAddSupportEntry( const CFileInfo *pFileInfo );
	BOOL EndCompilation();
		
private:
	///////////////
	// PRIVATE VARS
	MasterFileCompile_ProgressBarInitFcn_t *m_pProgressInitFcn;
	MasterFileCompile_ProgressBarStepFcn_t *m_pProgressStepFcn;
	void *m_pUserData;
	FILE *m_pBinFileStream;
	u8 *m_pAllocatedMem;
	FDataPrjFile_Entry_t *m_paEntries;
	FDataPrjFile_SupportEntry_t *m_paSupportEntries;
	FDataPrjFile_Header_t m_Header;
	
	/////////////////////////////
	// PRIVATE SUPPORT FUNCTIONS
	BOOL WriteNewEmptyMasterFile( FILE *pFileStream, u32 nPlatformFlag, u32 nMinEntries );
	FILE *GetCompilationDataFilePtr( const FDataPrjFile_Entry_t *pEntry );

	//////////////////////////////
	// ENDIAN CONVERSION FUNCTIONS
	MasterFileCompile_ErrorCodes_e ReadHeader( BOOL &rbSwitchedEndian );
	MasterFileCompile_ErrorCodes_e ReadEntryArray( u8 *pDest, u32 nEntriesToRead, u32 nNumUsedEntries, BOOL bSwitchEndian );
	MasterFileCompile_ErrorCodes_e ReadSupportEntryArray( u8 *pDest, u32 nEntriesToRead, u32 nNumUsedEntries, BOOL bSwitchEndian );

	MasterFileCompile_ErrorCodes_e WriteHeader( BOOL &rbSwitchedEndian );
	MasterFileCompile_ErrorCodes_e WriteEntryArray( FDataPrjFile_Entry_t *pEntry, u32 nEntriesToWrite, BOOL bSwitchEndian );
	MasterFileCompile_ErrorCodes_e WriteSupportEntryArray( FDataPrjFile_SupportEntry_t *pEntry, u32 nEntriesToWrite, BOOL bSwitchEndian );
		
protected:
		
};




#endif

