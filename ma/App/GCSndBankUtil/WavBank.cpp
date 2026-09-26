//////////////////////////////////////////////////////////////////////////////////////
// WavBank.cpp - 
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
// 06/06/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "WavBank.h"
#include "fclib.h"
#include "WavFile.h"


CWavBank::CWavBank() {
	fang_MemZero( &m_Header, sizeof( WavBank_Header_t ) );
	m_apWaves.RemoveAll();

	// go ahead and create an empty file by default
	CreateEmpty();
}

CWavBank::~CWavBank() {
	FreeData();
}

BOOL CWavBank::SetFileSettings( WavBank_ToolSettings_t *pSettings ) {

	// simply copy the new settings over the old ones
	fang_MemCopy( &m_Header.Settings, pSettings, sizeof( WavBank_ToolSettings_t ) );

	return TRUE;	
}

WavBank_ToolSettings_t *CWavBank::GetFileSettings() {
	return &m_Header.Settings;
}

BOOL CWavBank::CreateEmpty() {

	// free any data that may be contained
	FreeData();
	
	// a few fields need to be setup in the header
	m_Header.nSignature = FVERSION_FILE_SIGNATURE;
	m_Header.nFileVersion = WAV_BANK_VERSION_NUM;
	m_Header.nNumBytes = sizeof( WavBank_Header_t );
	
	// record the name of who created this file
	DWORD nLen = WAV_BANK_STRING_LEN;
	if( !GetComputerName( m_Header.szCreatedBy, &nLen ) ) {
		fclib_strncpy( m_Header.szCreatedBy, "unknown user", WAV_BANK_STRING_LEN );
	}
	return TRUE;
}

BOOL CWavBank::IsEmpty() {

	if( m_Header.nNumWaves ) {
		return FALSE;
	}
	return TRUE;
}

u32 CWavBank::GetBankSize() {
	return m_Header.nNumBytes;
}

BOOL CWavBank::Read( cchar *pszFilename ) {

	CreateEmpty();

	if( !pszFilename ) {
		return FALSE;
	}
	
	// open the file for reading
	FILE *pFileStream = fopen( pszFilename, "rb" );
	if( !pFileStream ) {
		return FALSE;
	}

	// read the header in
	if( fread( &m_Header, sizeof( WavBank_Header_t ), 1, pFileStream ) != 1 ) {
		fclose( pFileStream );
		return FALSE;
	}

	// check the version and signature
	if( m_Header.nFileVersion != WAV_BANK_VERSION_NUM ||
		m_Header.nSignature != FVERSION_FILE_SIGNATURE ) {
		fang_MemZero( &m_Header, sizeof( WavBank_Header_t ) );
		return FALSE;
	}

	// check the filesize
	fseek( pFileStream, 0, SEEK_END );
	if( ftell( pFileStream ) != (int)m_Header.nNumBytes ) {
		// the filesize in the header does not match the actual filesize
		fclose( pFileStream );
		fang_MemZero( &m_Header, sizeof( WavBank_Header_t ) );
		return FALSE;
	}
	fseek( pFileStream, sizeof( WavBank_Header_t ), SEEK_SET );
	
	u32 i, nFileWavCount;
	WavBank_Entry_t *pEntry;

	if( m_Header.nNumWaves ) {
		nFileWavCount = m_Header.nNumWaves;
		m_Header.nNumWaves = 0;

		// read in each entry
		for( i=0; i < nFileWavCount; i++ ) {
			// allocate memory for an entry
			pEntry = (WavBank_Entry_t *)malloc( sizeof( WavBank_Entry_t ) );
			if( !pEntry ) {
				fclose( pFileStream );
				CreateEmpty();
				return FALSE;
			}

			// read in the entry
			if( fread( pEntry, sizeof( WavBank_Entry_t ), 1, pFileStream ) != 1 ) {
				free( pEntry );
				fclose( pFileStream );
				CreateEmpty();
				return FALSE;
			}

			// go ahead and allocate memory to store the wav file (which we will read in just a bit)
			if( pEntry->nNumBytes ) {
				pEntry->pData = (u8 *)malloc( pEntry->nNumBytes );
				if( !pEntry->pData ) {
					free( pEntry );
					fclose( pFileStream );
					CreateEmpty();
					return FALSE;
				}
			} else {
				pEntry->pData = NULL;
			}

			// add this entry to our entry list
			m_apWaves.Add( pEntry );
			m_Header.nNumWaves++;
		}

		FASSERT( m_Header.nNumWaves == nFileWavCount );

		// read in each wav file
		for( i=0; i < nFileWavCount; i++ ) {
			pEntry = (WavBank_Entry_t *)m_apWaves[i];

			if( pEntry->nNumBytes ) {
				if( fread( pEntry->pData, pEntry->nNumBytes, 1, pFileStream ) != 1 ) {
					fclose( pFileStream );
					CreateEmpty();
					return FALSE;
				}
			}
		}
	}

	// close the file
	fclose( pFileStream );

	return TRUE;
}

BOOL CWavBank::ReadEntriesOnly( cchar *pszFilename ) {
	CreateEmpty();

	if( !pszFilename ) {
		return FALSE;
	}
	
	// open the file for reading
	FILE *pFileStream = fopen( pszFilename, "rb" );
	if( !pFileStream ) {
		return FALSE;
	}

	// read the header in
	if( fread( &m_Header, sizeof( WavBank_Header_t ), 1, pFileStream ) != 1 ) {
		fclose( pFileStream );
		return FALSE;
	}

	// check the version and signature
	if( m_Header.nFileVersion != WAV_BANK_VERSION_NUM ||
		m_Header.nSignature != FVERSION_FILE_SIGNATURE ) {
		fang_MemZero( &m_Header, sizeof( WavBank_Header_t ) );
		return FALSE;
	}

	// check the filesize
	fseek( pFileStream, 0, SEEK_END );
	if( ftell( pFileStream ) != (int)m_Header.nNumBytes ) {
		// the filesize in the header does not match the actual filesize
		fclose( pFileStream );
		fang_MemZero( &m_Header, sizeof( WavBank_Header_t ) );
		return FALSE;
	}
	fseek( pFileStream, sizeof( WavBank_Header_t ), SEEK_SET );

	u32 i, nFileWavCount;
	WavBank_Entry_t *pEntry;

	if( m_Header.nNumWaves ) {
		nFileWavCount = m_Header.nNumWaves;
		m_Header.nNumWaves = 0;

		// read in each entry
		for( i=0; i < nFileWavCount; i++ ) {
			// allocate memory for an entry
			pEntry = (WavBank_Entry_t *)malloc( sizeof( WavBank_Entry_t ) );
			if( !pEntry ) {
				fclose( pFileStream );
				CreateEmpty();
				return FALSE;
			}

			// read in the entry
			if( fread( pEntry, sizeof( WavBank_Entry_t ), 1, pFileStream ) != 1 ) {
				free( pEntry );
				fclose( pFileStream );
				CreateEmpty();
				return FALSE;
			}

			// since we won't be reading in the wav data, just NULL out the ptr
			pEntry->pData = NULL;

			// add this entry to our entry list
			m_apWaves.Add( pEntry );
			m_Header.nNumWaves++;
		}

		FASSERT( m_Header.nNumWaves == nFileWavCount );		
	}

	// close the file
	fclose( pFileStream );

	return TRUE;
}

BOOL CWavBank::Write( cchar *pszFilename ) {
	
	if( !pszFilename ) {
		return FALSE;
	}

	FILE *pFileStream = fopen( pszFilename, "wb" );
	if( !pFileStream ) {
		return FALSE;
	}
	
	// update our header
	CTime CurrentTime = CTime::GetCurrentTime();
	m_Header.nModifiedTime = (u32)CurrentTime.GetTime();
	DWORD nLen = WAV_BANK_STRING_LEN;
	if( !GetComputerName( m_Header.szLastModifiedBy, &nLen ) ) {
		fclib_strncpy( m_Header.szLastModifiedBy, "unknown user", WAV_BANK_STRING_LEN );
	}
	u32 nByteOffset = 0;

	// write out our header
	fwrite( &m_Header, sizeof( WavBank_Header_t ), 1, pFileStream );
	nByteOffset = sizeof( WavBank_Header_t );

	// write out each entry
	nByteOffset += sizeof( WavBank_Entry_t ) * m_Header.nNumWaves;
	u32 i;
	u8 *pData;
	WavBank_Entry_t *pEntry;
	for( i=0; i < m_Header.nNumWaves; i++ ) {
		pEntry = (WavBank_Entry_t *)m_apWaves[i];
		
		// fix pEntry->nDataOffset
		pEntry->nDataOffset = nByteOffset;
		nByteOffset += pEntry->nNumBytes;
		
		// cache the data ptr
		pData = pEntry->pData;
		pEntry->pData = NULL;

		// write the entry to disk
		fwrite( pEntry, sizeof( WavBank_Entry_t ), 1, pFileStream );

		// restore the data ptr
		pEntry->pData = pData;
	}

	// sanity check
	FASSERT( nByteOffset == m_Header.nNumBytes );
	
	// write out each wav file
	for( i=0; i < m_Header.nNumWaves; i++ ) {
		pEntry = (WavBank_Entry_t *)m_apWaves[i];
		
		if( pEntry->nNumBytes ) {
			fwrite( pEntry->pData, pEntry->nNumBytes, 1, pFileStream );
		}
	}

	// close the file
	fclose( pFileStream );

	return TRUE;
}

u32 CWavBank::GetNumEntries() {
	return m_Header.nNumWaves;
}

BOOL CWavBank::DeleteEntryByIndex( u32 nIndex ) {

	WavBank_Entry_t *pEntry = GetEntryByIndex( nIndex );
	if( !pEntry ) {
		return FALSE;
	}
	
	RemoveEntry( pEntry, nIndex );

	return TRUE;
}

BOOL CWavBank::DeleteEntry( WavBank_Entry_t *pEntry ) {

	if( !pEntry ) {
		return FALSE;
	}
	// find the index of pEntry
	u32 i;
	for( i=0; i < m_Header.nNumWaves; i++ ) {
		if( (u32)pEntry == (u32)m_apWaves[i] ) {
			break;
		}
	}
	if( i == m_Header.nNumWaves ) {
		return FALSE;
	}

	RemoveEntry( pEntry, i );

	return TRUE;
}

WavBank_Entry_t *CWavBank::GetEntryFromFileInfo( CFileInfo *pFileInfo ) {
	
	if( !pFileInfo ) {
		return NULL;
	}
	return GetEntryByName( pFileInfo->GetFileTitle() );
}

WavBank_Entry_t *CWavBank::GetEntryByIndex( u32 nIndex ) {
	
	if( nIndex >= m_Header.nNumWaves ) {
		return NULL;
	}
	return (WavBank_Entry_t *)m_apWaves[nIndex];
}

// retrieves an entry by name (pszFilename should not include any
// path or extension info, only the file title)
WavBank_Entry_t *CWavBank::GetEntryByName( cchar *pszFilename ) {
	
	if( !pszFilename ) {
		return NULL;
	}
	// search our entry array for a match
	u32 i;
	WavBank_Entry_t *pEntry;
	for( i=0; i < m_Header.nNumWaves; i++ ) {
		pEntry = (WavBank_Entry_t *)m_apWaves[i];
		if( fclib_stricmp( pEntry->szFilename, pszFilename ) == 0 ) {
			// we match, return this entry
			return pEntry;
		}
	}

	return NULL;
}

// will create a new entry if one does not already exist,
// if one already exists with the same name, it will be replaced
// by pFileInfo
WavBank_Entry_t *CWavBank::AddEntry( CFileInfo *pFileInfo ) {
	
	if( !pFileInfo ) {
		return NULL;
	}
	WavBank_Entry_t *pEntry;

	// see if an entry already exists (remember no extensions)
	pEntry = GetEntryByName( pFileInfo->GetFileTitle() );
	if( pEntry ) {
		if( UpdateEntry( pFileInfo, pEntry ) ) {
			return pEntry;
		} else {
			return NULL;
		}
	}

	// check the passed in wav format
	f32 fSecs;
	u32 nNumChannels, nSamplesPerSec;
	if( !IsAValidWaveFile( pFileInfo, &fSecs, &nNumChannels, &nSamplesPerSec ) ) {
		// not a valid wav, or the wrong format
		return FALSE;
	}

	// the entry doesn't already exist, try to create one

	// allocate the entry
	pEntry = (WavBank_Entry_t *)malloc( sizeof( WavBank_Entry_t ) );
	if( !pEntry ) {
		return NULL;
	}

	// allocate memory to hold the file
	pEntry->pData = (u8 *)malloc( pFileInfo->GetLength() );
	if( !pEntry->pData ) {
		free( pEntry );
		return NULL;
	}

	// copy pFileInfo into our memory buffer
	if( !CopyFileToMemory( pEntry->pData, pFileInfo ) ) {
		free( pEntry->pData );
		free( pEntry );
		return NULL;
	}
	
	// update pEntry
	pEntry->nDataOffset = 0;// unknown till we write out the file
	pEntry->nModifiedTime = (u32)pFileInfo->GetLastWriteTime().GetTime();
	pEntry->nNumBytes = pFileInfo->GetLength();
	fclib_strncpy( pEntry->szFilename, pFileInfo->GetFileTitle(), WAV_BANK_MAX_WAV_FILENAME_LEN );
	pEntry->fSecsOfAudio = fSecs;
	pEntry->nNumChannels = nNumChannels;
	pEntry->nSamplesPerSec = nSamplesPerSec;

	// update the header filesize
	m_Header.nNumBytes += pEntry->nNumBytes;
	m_Header.nNumBytes += sizeof( WavBank_Entry_t );
	m_Header.nNumWaves++;

	// finally add pEntry to our entry list
	m_apWaves.Add( pEntry );

	return pEntry;
}

// returns TRUE if pEntry was updated, FALSE if there was a problem
// pEntry is unchanged if FALSE is returned.
BOOL CWavBank::UpdateEntry( CFileInfo *pFileInfo, WavBank_Entry_t *pEntry ) {
	
	if( !pFileInfo || !pEntry ) {
		return FALSE;
	}

	// check the passed in wav format
	f32 fSecs;
	u32 nNumChannels, nSamplesPerSec;
	if( !IsAValidWaveFile( pFileInfo, &fSecs, &nNumChannels, &nSamplesPerSec ) ) {
		// not a valid wav, or the wrong format
		return FALSE;
	}

	// allocate memory to hold pFileInfo
	u8 *pData;
	pData = (u8 *)malloc( pFileInfo->GetLength() );
	if( !pData ) {
		return FALSE;
	}

	// copy pFileInfo into our memory buffer
	if( !CopyFileToMemory( pData, pFileInfo ) ) {
		free( pData );
		return FALSE;
	}

	// release the currently held wav file
	free( pEntry->pData );
	m_Header.nNumBytes -= pEntry->nNumBytes;

	// update pEntry
	pEntry->nDataOffset = 0;// unknown till we write out the file
	pEntry->nModifiedTime = (u32)pFileInfo->GetLastWriteTime().GetTime();
	pEntry->nNumBytes = pFileInfo->GetLength();
	pEntry->pData = pData;
	fclib_strncpy( pEntry->szFilename, pFileInfo->GetFileTitle(), WAV_BANK_MAX_WAV_FILENAME_LEN );
	pEntry->fSecsOfAudio = fSecs;
	pEntry->nNumChannels = nNumChannels;
	pEntry->nSamplesPerSec = nSamplesPerSec;

	// update the header filesize
	m_Header.nNumBytes += pEntry->nNumBytes;
	
	return TRUE;
}

// returns TRUE if pEntry and pFileInfo match and do not need updating.
BOOL CWavBank::DoesFileMatchEntry( CFileInfo *pFileInfo, WavBank_Entry_t *pEntry ) {
	
	if( !pFileInfo || !pEntry ) {
		return FALSE;
	}
	// check the names
	if( fclib_stricmp( pFileInfo->GetFileTitle(), pEntry->szFilename ) != 0 ) {
		return FALSE;
	}
	// check the size
	if( pFileInfo->GetLength() != pEntry->nNumBytes ) {
		return FALSE;
	}
	// check the modified time
	CTime ModTime = (time_t)pEntry->nModifiedTime;
	if( pFileInfo->GetLastWriteTime() != ModTime ) {
		return FALSE;
	}

	return TRUE;
}

void CWavBank::FreeData() {

	while( m_Header.nNumWaves ) {
		DeleteEntryByIndex( 0 );
	}
	m_apWaves.RemoveAll();
	fang_MemZero( &m_Header, sizeof( WavBank_Header_t ) );
}

BOOL CWavBank::CopyFileToMemory( u8 *pDest, CFileInfo *pFileInfo ) {
	FILE *pFileStream;

	pFileStream = fopen( pFileInfo->GetFilePath(), "rb" );
	if( !pFileStream ) {
		return FALSE;
	}
	// seek to the beginning of the file
	fseek( pFileStream, 0, SEEK_SET );

	// read the file into pDest
	if( fread( pDest, pFileInfo->GetLength(), 1, pFileStream ) != 1 ) {
		fclose( pFileStream );
		return FALSE;
	}

	fclose( pFileStream );

	return TRUE;
}

BOOL CWavBank::IsAValidWaveFile( CFileInfo *pFileInfo, f32 *pfFileSecs, u32 *pnNumChannels, u32 *pnSamplesPerSec ) {
	CWaveFile WavFile;

	if( !WavFile.Open( (char *)((const char *)(pFileInfo->GetFilePath())), NULL, WAVEFILE_READ ) ) {
		return FALSE;
	}
	if( WavFile.m_pwfx->wBitsPerSample != 16 ||
		(WavFile.m_pwfx->nChannels != 1 && WavFile.m_pwfx->nChannels != 2) ||			
		WavFile.m_pwfx->wFormatTag != WAVE_FORMAT_PCM ) {
		
		WavFile.Close();
		return FALSE;
	}
	// record the wav secs
	if( pfFileSecs ) {
		*pfFileSecs = WavFile.m_fSecsOfData;
	}
	// record the number of channels
	if ( pnNumChannels ) {
		*pnNumChannels = WavFile.m_pwfx->nChannels;
	}
	// record the sample rate
	if( pnSamplesPerSec ) {
		*pnSamplesPerSec = WavFile.m_pwfx->nSamplesPerSec;
	}

	WavFile.Close();

	return TRUE;
}

void CWavBank::RemoveEntry( WavBank_Entry_t *pEntry, u32 nIndex ) {
	
	// recompute the num of bytes contained in the file
	m_Header.nNumBytes -= pEntry->nNumBytes;
	m_Header.nNumBytes -= sizeof( WavBank_Entry_t );

	// free the wav data memory
	if( pEntry->pData ) {
		free( pEntry->pData );
		pEntry->pData = NULL;
	}

	// free the entry memory
	free( pEntry );
	m_apWaves.RemoveAt( nIndex, 1 );

	// reduce the number of waves contained in the bank by 1
	m_Header.nNumWaves--;
}
