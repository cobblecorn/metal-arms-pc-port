//////////////////////////////////////////////////////////////////////////////////////
// WavBank.h - 
//
// Author: Michael Starich   
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
// 06/05/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _WAVBANK_H_
#define _WAVBANK_H_ 1

#include "fang.h"
#include "fversion.h"
#include "FileInfo.h"

#define WAV_BANK_STRING_LEN				256
#define WAV_BANK_MAX_WAV_FILENAME_LEN	12// includes NULL and excludes any extension
#define WAV_BANK_VERSION_NUM			0x00000002

typedef struct {
	char szLastDirUsed[WAV_BANK_STRING_LEN];
	BOOL bRecurseSubDirs;
	u32 nFileSortColumn;
	BOOL bFileAscendingSort;
	u32 nBankSortColumn;
	BOOL bBankAscendingSort;
} WavBank_ToolSettings_t;

typedef struct {
	u32 nSignature;		// should always == FVERSION_FILE_SIGNATURE
	u32 nFileVersion;	// should always == WAV_BANK_VERSION_NUM
	u32 nNumBytes;
	u32 nNumWaves;
	u32 nModifiedTime;	// Really the file's time_t structure containing the file's last write time

	// user names to keep track of who created and modified this bank
	char szCreatedBy[WAV_BANK_STRING_LEN];
	char szLastModifiedBy[WAV_BANK_STRING_LEN];
	
	// settings used solely by the WavBanker Tool to restore the last settings used to create this file
	WavBank_ToolSettings_t Settings;	
} WavBank_Header_t;

typedef struct {
	u32 nNumBytes;
	u32 nDataOffset;	// in bytes from the head of the bank file
	u8 *pData;			// points to the actual wav file to be copies into the bank
	u32 nModifiedTime;
	char szFilename[WAV_BANK_MAX_WAV_FILENAME_LEN];// the wav filename, without extension
	f32 fSecsOfAudio;
	u32 nNumChannels;
	u32 nSamplesPerSec;
} WavBank_Entry_t;

class CWavBank
{
public:
	CWavBank();
	~CWavBank();

	BOOL SetFileSettings( WavBank_ToolSettings_t *pSettings );
	WavBank_ToolSettings_t *GetFileSettings();

	BOOL CreateEmpty();
	BOOL IsEmpty();
	u32 GetBankSize();

	BOOL Read( cchar *pszFilename );
	BOOL ReadEntriesOnly( cchar *pszFilename );
	BOOL Write( cchar *pszFilename );
	
	u32 GetNumEntries();

	BOOL DeleteEntryByIndex( u32 nIndex );
	BOOL DeleteEntry( WavBank_Entry_t *pEntry );

	WavBank_Entry_t *GetEntryFromFileInfo( CFileInfo *pFileInfo );
	WavBank_Entry_t *GetEntryByIndex( u32 nIndex );
	WavBank_Entry_t *GetEntryByName( cchar *pszFilename );
	WavBank_Entry_t *AddEntry( CFileInfo *pFileInfo );
	BOOL UpdateEntry( CFileInfo *pFileInfo, WavBank_Entry_t *pEntry );

	BOOL DoesFileMatchEntry( CFileInfo *pFileInfo, WavBank_Entry_t *pEntry );
	

private:
	void FreeData();
	void RemoveEntry( WavBank_Entry_t *pEntry, u32 nIndex );

protected:
	WavBank_Header_t m_Header;
	CPtrArray m_apWaves;// each element is a WavBank_Entry_t *

	BOOL CopyFileToMemory( u8 *pDest, CFileInfo *pFileInfo );
	BOOL IsAValidWaveFile( CFileInfo *pFileInfo, f32 *pfFileSecs, u32 *pnNumChannels, u32 *pnSamplesPerSec );
};

#endif

