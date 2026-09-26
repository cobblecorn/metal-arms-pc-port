//////////////////////////////////////////////////////////////////////////////////////
// fdx8file.cpp - Fang file module for DX platforms.
//
// Author: John Lafleur
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
// 06/11/02 Lafleur		Transferred from ffile
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "fmath.h"
#include "ffile.h"
#include "fdx8file.h"
#include "fclib.h"

//////////////////////////////////////////////////////////////////////////////////////
// External Dependencies:
//////////////////////////////////////////////////////////////////////////////////////

extern volatile u16 FFile_nLastMFTokenRead;
extern volatile u16 FFile_nLastMFTokenUsed;
extern volatile QueuedReadToken *FFile_paMFTokenList;
extern FFileHandle FFile_hAsyncMasterFile;

extern char FFile_cLanguageChar;
extern char FFile_cReferenceLanguageChar;	


//////////////////////////////////////////////////////////////////////////////////////
// Local Variables:
//////////////////////////////////////////////////////////////////////////////////////

volatile BOOL _bModuleInitialized = FALSE;
volatile u32 _nAsyncPendingReadCount;
volatile CFPlatformFileCookie *_apPendingAsyncReads[FFILE_MAX_SIMULTANEOUS_ASYNC_READS];
static HANDLE _hCancelledFile = INVALID_HANDLE_VALUE;

volatile BOOL _bChangingQueue;
volatile u32 _LastMFTokenSubmitted;

static HANDLE _hAsyncReadThread;
static HANDLE _hPendingCountSemaphore;


//////////////////////////////////////////////////////////////////////////////////////
// Local Functions:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _NextMFRead( u32 nReadIdx );
static void CALLBACK _AsyncReadCallback( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped );
static DWORD WINAPI AsyncFileThread( LPVOID lpParam );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fdx8file_ModuleStartup( void ) 
{
	u32 i;

	_bModuleInitialized = FALSE;

	_nAsyncPendingReadCount = 0;
	_LastMFTokenSubmitted = 0;
	
	for ( i = 0; i < FFILE_MAX_SIMULTANEOUS_ASYNC_READS; i++ )
	{
		_apPendingAsyncReads[i] = NULL;
	}

	// Clear the platform specific section of the cookies
	for ( i = 0; i < FFile_nMaxSimultaneousOpen; i++ )
	{
		FFile_paFileCookie[i].m_bUsedEntry = FALSE;
		FFile_paFileCookie[i].m_pMasterDirEntry = NULL;
		FFile_paFileCookie[i].m_hFile = INVALID_HANDLE_VALUE;
		FFile_paFileCookie[i].m_nPosition = 0;
		FFile_paFileCookie[i].m_nSeekPosition = -1;
		FFile_paFileCookie[i].m_pCallback  = NULL;
		FFile_paFileCookie[i].m_bAsyncReadInProgress = FALSE;
		FFile_paFileCookie[i].m_bAsyncReadReady = FALSE;
		FFile_paFileCookie[i].m_pUser = NULL;
	}

	_hPendingCountSemaphore = CreateSemaphore( NULL, 0, FFILE_MAX_MF_READ_TOKENS, "PendingCount" );
	if ( _hPendingCountSemaphore == NULL ) 
	{
		OutputDebugString( "CreateSemaphore failed.\r\n" );
		return FALSE;
	}
   
    DWORD dwThreadId, dwThrdParam = 1;
    _hAsyncReadThread = CreateThread( 
        NULL,					// (this parameter is ignored)
        0,						// use default stack size  
        AsyncFileThread,		// thread function 
        &dwThrdParam,			// argument to thread function 
        0,						// use default creation flags 
        &dwThreadId );			// returns the thread identifier 
 
	// Check the return value for success. 
	if ( _hAsyncReadThread == NULL ) 
	{
		OutputDebugString( "CreateThread failed.\r\n" );
		return FALSE;
	}

	_hCancelledFile = INVALID_HANDLE_VALUE;
	
	_bModuleInitialized = TRUE;
	_bChangingQueue = FALSE;
	
	return TRUE;
}


//
//
//
BOOL fdx8file_ModuleShutdown( void )
{
	u32 i;
	for ( i = 0; i < FFile_nMaxSimultaneousOpen; i++ )
	{
		FFile_paFileCookie[i].m_bUsedEntry = FALSE;
		FFile_paFileCookie[i].m_pMasterDirEntry = NULL;
		FFile_paFileCookie[i].m_hFile = INVALID_HANDLE_VALUE;
		FFile_paFileCookie[i].m_pLastDestBuffer  = NULL;
		FFile_paFileCookie[i].m_nPosition = 0;
		FFile_paFileCookie[i].m_nSeekPosition = -1;
		FFile_paFileCookie[i].m_pCallback  = NULL;
		FFile_paFileCookie[i].m_bAsyncReadInProgress = FALSE;
		FFile_paFileCookie[i].m_bAsyncReadReady = FALSE;
		FFile_paFileCookie[i].m_pUser = NULL;
	}
	
	for ( i = 0; i < FFILE_MAX_SIMULTANEOUS_ASYNC_READS; i++ )
	{
		_apPendingAsyncReads[i] = NULL;
	}

	_hCancelledFile = INVALID_HANDLE_VALUE;
	
	_bModuleInitialized = FALSE;

	CloseHandle( _hAsyncReadThread );
	CloseHandle( _hPendingCountSemaphore );

	return TRUE;
}

//
//
//
// We are not doing anything usefull in the work function for this platform
void ffile_Work( void ) {
}



//
//
//
FFileHandle ffile_OSDir_Open( cchar *pszFName, FFileOpenType_e nOpenMode, BOOL bAsyncAndAligned/*=FALSE*/ ) 
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	char szFileName[ 255 ];
	const char* pszFileName = pszFName;

	FASSERT( _bModuleInitialized );

	if( FFile_cReferenceLanguageChar && FFile_cLanguageChar )
	{
		// Copy this string into the local string path and remap the character pointer
		fclib_strcpy( szFileName, pszFName );
		pszFileName = szFileName;
		ffile_LocalizeFilename( szFileName );
	}

	s32 nFileIndex;
	for ( nFileIndex = 0; nFileIndex < (s32)FFile_nMaxSimultaneousOpen; nFileIndex++ ) 
	{
		if ( FFile_paFileCookie[nFileIndex].m_bUsedEntry == FALSE ) 
		{
			break;
		}
	}

	if ( nFileIndex == (s32)FFile_nMaxSimultaneousOpen )
	{
		DEVPRINTF( "ffile_OSDir_Open() : Maximum number of files (%d) already opened.\n", FFile_nMaxSimultaneousOpen );
		return FFILE_ERROR_RET;
	}

	if ( bAsyncAndAligned )
	{
		hFile = CreateFileA( pszFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL );
	}
	else
	{
		switch( nOpenMode ) 
		{
			case FFILE_OPEN_RONLY_SEQUENTIAL:
				hFile = CreateFileA( pszFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
				break;

			case FFILE_OPEN_RONLY_RANDOM:
				hFile = CreateFileA( pszFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL );
				break;

			case FFILE_OPEN_RONLY:
				hFile = CreateFileA( pszFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
				break;

			case FFILE_OPEN_WONLY:
				hFile = CreateFileA( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
				break;

			case FFILE_OPEN_TRUNC_OR_CREATE_WONLY:
				hFile = CreateFileA( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				break;

			default:
				FASSERT_NOW;
		}
	}

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		DWORD nLastError = GetLastError();
		return FFILE_ERROR_RET;
	}

	// Setup the cookie
	FFile_paFileCookie[nFileIndex].m_hFile = hFile;
	FFile_paFileCookie[nFileIndex].m_pLastDestBuffer  = NULL;
	FFile_paFileCookie[nFileIndex].m_bUsedEntry = TRUE;
	FFile_paFileCookie[nFileIndex].m_pMasterDirEntry = NULL;
	FFile_paFileCookie[nFileIndex].m_nPosition = 0;
	FFile_paFileCookie[nFileIndex].m_nSeekPosition = -1;
	FFile_paFileCookie[nFileIndex].m_pCallback  = NULL;
	FFile_paFileCookie[nFileIndex].m_bAsyncReadInProgress = FALSE;
	FFile_paFileCookie[nFileIndex].m_bAsyncReadReady = FALSE;

	return (FFileHandle)nFileIndex;
}


//
//
//
s32 ffile_OSDir_Close( CFPlatformFileCookie *pCookie )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie && pCookie->m_hFile != INVALID_HANDLE_VALUE );

	if ( pCookie->m_bAsyncReadInProgress )
	{
		ffile_CancelFileIO( pCookie->m_hFangHandle );
	}

	if ( CloseHandle( pCookie->m_hFile ) == 0 )
	{
		return FFILE_ERROR_RET;
	}

	// Clear the cookie settings
	pCookie->m_bUsedEntry = FALSE;
	pCookie->m_pMasterDirEntry = NULL;
	pCookie->m_hFile = INVALID_HANDLE_VALUE;
	pCookie->m_pLastDestBuffer  = NULL;
	pCookie->m_nPosition = 0;
	pCookie->m_nSeekPosition = -1;
	pCookie->m_pCallback  = NULL;
	pCookie->m_bAsyncReadInProgress = FALSE;
	pCookie->m_bAsyncReadReady = FALSE;

	return 0;
}


//
//
//
s32 ffile_OSDir_GetFileSize( CFPlatformFileCookie *pCookie ) 
{
	u32 nUpperDWORD;
	u32 nLowerDWORD;

	FASSERT( _bModuleInitialized );
	FASSERT( pCookie && pCookie->m_hFile != INVALID_HANDLE_VALUE );

	nLowerDWORD = GetFileSize( pCookie->m_hFile, (LPDWORD)&nUpperDWORD );
  
	if ( nLowerDWORD == -1 )
	{
		return FFILE_ERROR_RET;
	}

	return nLowerDWORD;
}


//
//	Seek to a position in the file.  Returns the resulting file position.
//
s32 ffile_OSDir_Seek( CFPlatformFileCookie *pCookie, s32 nFileOffset, FFileSeekType_e nSeekMode ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie && pCookie->m_hFile != INVALID_HANDLE_VALUE );

	// We are going to cue the seek so that we don't take the seek time hit unnecessarily
	switch ( nSeekMode )
	{
		case FFILE_SEEK_SET:
			pCookie->m_nSeekPosition = nFileOffset;
//			nReturn = SetFilePointer( pCookie->m_hFile, nFileOffset, NULL, FILE_BEGIN );
			break;

		case FFILE_SEEK_CUR:
			pCookie->m_nSeekPosition += nFileOffset;
//			nReturn = SetFilePointer( pCookie->m_hFile, nFileOffset, NULL, FILE_CURRENT );
			break;

		case FFILE_SEEK_END:
			pCookie->m_nSeekPosition = ffile_OSDir_GetFileSize( pCookie ) - nFileOffset;
//			nReturn = SetFilePointer( pCookie->m_hFile, nFileOffset, NULL, FILE_END );
			break;

		default:
			FASSERT_NOW;
			return FFILE_ERROR_RET;
	}

	return pCookie->m_nSeekPosition;
}


//
//
//
s32 ffile_OSDir_Tell( CFPlatformFileCookie *pCookie ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie && pCookie->m_hFile != INVALID_HANDLE_VALUE );

	return pCookie->m_nSeekPosition;
}


//
//
//
DWORD WINAPI AsyncFileThread( LPVOID lpParam ) 
{ 
	u32 i;

	while( TRUE )
	{
		WaitForSingleObjectEx( _hPendingCountSemaphore, INFINITE, TRUE );

		for ( i = 0; i < FFILE_MAX_SIMULTANEOUS_ASYNC_READS; i++ )
		{
			if ( _apPendingAsyncReads[i] == NULL )
			{
				break;
			}
		}

		if ( _LastMFTokenSubmitted == FFile_nLastMFTokenRead && FFile_nLastMFTokenRead != FFile_nLastMFTokenUsed )
		{
			// File system is caught up with us, so send out this read
			_NextMFRead( i );
		}

		if ( _nAsyncPendingReadCount )
		{
			for ( i = 0; i < FFILE_MAX_SIMULTANEOUS_ASYNC_READS; i++ )
			{
				if ( _apPendingAsyncReads[i] && _apPendingAsyncReads[i]->m_bAsyncReadReady )
				{
					_apPendingAsyncReads[i]->m_bAsyncReadReady = FALSE;
					if ( ReadFileEx( _apPendingAsyncReads[i]->m_hFile, 
									 _apPendingAsyncReads[i]->m_pLastDestBuffer, 
									 _apPendingAsyncReads[i]->m_nAsyncReadAmount, 
									 (OVERLAPPED *)&_apPendingAsyncReads[i]->m_Overlapped, 
									 _AsyncReadCallback ) == 0 )
					{
						_apPendingAsyncReads[i]->m_pCallback( FFILE_ERROR_RET, _apPendingAsyncReads[i]->m_hFangHandle, _apPendingAsyncReads[i]->m_pLastDestBuffer, -1, _apPendingAsyncReads[i]->m_pUser );
						_apPendingAsyncReads[i]->m_bAsyncReadInProgress = FALSE;
						_apPendingAsyncReads[i]->m_pCallback = NULL;
						_apPendingAsyncReads[i] = NULL;
						_nAsyncPendingReadCount--;
					}
				}
			}
		}
	}

    return 0; 
} 


//
//
//
static BOOL _NextMFRead( u32 nReadIdx )
{
	FASSERT( FFile_nLastMFTokenRead != FFile_nLastMFTokenUsed );
	
	while ( TRUE )
	{
		u16 nNextToRead = FFile_nLastMFTokenRead + 1;
		if ( nNextToRead == FFILE_MAX_MF_READ_TOKENS )
		{
			nNextToRead = 0;
		}

		// Get the next token in the token link list
		QueuedReadToken *pToken = (QueuedReadToken *)&FFile_paMFTokenList[nNextToRead];

		FASSERT( ((u32)pToken->nStartPosition & 2047) == 0 );
		FASSERT( ((u32)pToken->nReadAmount & 2047) == 0 );
		FASSERT( ((u32)pToken->pDestination & 31) == 0 );
		
		_apPendingAsyncReads[nReadIdx] = &FFile_paFileCookie[FFile_hAsyncMasterFile];
		_apPendingAsyncReads[nReadIdx]->m_pCallback = pToken->pCallback;
		_apPendingAsyncReads[nReadIdx]->m_Overlapped.Offset = pToken->nStartPosition;
		_apPendingAsyncReads[nReadIdx]->m_Overlapped.OffsetHigh = 0;
		_apPendingAsyncReads[nReadIdx]->m_Overlapped.hEvent = (HANDLE)nReadIdx;  // ReadFileEx does not use the handle, so put the Async read index there
		_apPendingAsyncReads[nReadIdx]->m_bAsyncReadInProgress = TRUE;
		_apPendingAsyncReads[nReadIdx]->m_pUser = pToken->pUser;
		_apPendingAsyncReads[nReadIdx]->m_pLastDestBuffer = pToken->pDestination;
		
//		DEVPRINTF( "_NextMFRead() - Attempting to read %x.\n", pToken->pUser );
		if ( ReadFileEx( _apPendingAsyncReads[nReadIdx]->m_hFile, 
						 pToken->pDestination, 
						 FMATH_BYTE_ALIGN_UP( pToken->nReadAmount, 2048 ), 
						 (OVERLAPPED *)&_apPendingAsyncReads[nReadIdx]->m_Overlapped, 
						 _AsyncReadCallback ) != 0 )
		{
			_LastMFTokenSubmitted = nNextToRead;
			_nAsyncPendingReadCount++;
			return TRUE;
		}

		DEVPRINTF( "_NextMFRead() - Master file async read submission failed.\n" );
		_apPendingAsyncReads[nReadIdx]->m_pCallback( FFILE_ERROR_RET, _apPendingAsyncReads[nReadIdx]->m_hFangHandle, _apPendingAsyncReads[nReadIdx]->m_pLastDestBuffer, -1, _apPendingAsyncReads[nReadIdx]->m_pUser );
		_apPendingAsyncReads[nReadIdx]->m_bAsyncReadInProgress = FALSE;
		_apPendingAsyncReads[nReadIdx]->m_pCallback = NULL;
		_apPendingAsyncReads[nReadIdx] = NULL;
//		FFile_nLastMFTokenRead = nNextToRead;
		
		if ( nNextToRead == FFile_nLastMFTokenUsed )
		{
			break;
		}
	}
	
	return FALSE;
}


//
//
//
static void CALLBACK _AsyncReadCallback( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped )
{
	u32 nAsyncIndex = (u32)lpOverlapped->hEvent;

	if ( nAsyncIndex < 0 || nAsyncIndex >= FFILE_MAX_SIMULTANEOUS_ASYNC_READS )
	{
		FASSERT_NOW;
		return;
	}

	// If this was a cancelled file, ignore the callback
	if ( _apPendingAsyncReads[nAsyncIndex]->m_hFile == _hCancelledFile )
	{
		return;
	}

	_nAsyncPendingReadCount--;
	
	if ( &FFile_paFileCookie[FFile_hAsyncMasterFile] == _apPendingAsyncReads[nAsyncIndex] )
	{
		FASSERT( _LastMFTokenSubmitted >= 0 && _LastMFTokenSubmitted < FFILE_MAX_MF_READ_TOKENS );

		if ( dwErrorCode == 0 )
		{
			_apPendingAsyncReads[nAsyncIndex]->m_pCallback( 0, _apPendingAsyncReads[nAsyncIndex]->m_hFangHandle, FFile_paMFTokenList[_LastMFTokenSubmitted].pDestination, FFile_paMFTokenList[_LastMFTokenSubmitted].nReadAmount, FFile_paMFTokenList[_LastMFTokenSubmitted].pUser );
			_apPendingAsyncReads[nAsyncIndex]->m_nPosition = lpOverlapped->Offset + dwNumberOfBytesTransfered;
		}
		else
		{
			DEVPRINTF( "FFILE.CPP _AsyncReadCallback() - Async read failed.\n" );
			_apPendingAsyncReads[nAsyncIndex]->m_pCallback( FFILE_ERROR_RET, _apPendingAsyncReads[nAsyncIndex]->m_hFangHandle, FFile_paMFTokenList[_LastMFTokenSubmitted].pDestination, -1, FFile_paMFTokenList[_LastMFTokenSubmitted].pUser );
		}

		FFile_nLastMFTokenRead = _LastMFTokenSubmitted;
		
		if ( FFile_nLastMFTokenRead != FFile_nLastMFTokenUsed )
		{
			_NextMFRead( nAsyncIndex );
			return;
		}
	}
	else
	{
		if ( dwErrorCode == 0 )
		{
			_apPendingAsyncReads[nAsyncIndex]->m_pCallback( 0, _apPendingAsyncReads[nAsyncIndex]->m_hFangHandle, _apPendingAsyncReads[nAsyncIndex]->m_pLastDestBuffer, dwNumberOfBytesTransfered, _apPendingAsyncReads[nAsyncIndex]->m_pUser );
			_apPendingAsyncReads[nAsyncIndex]->m_nPosition = lpOverlapped->Offset + dwNumberOfBytesTransfered;
		}
		else
		{
			_apPendingAsyncReads[nAsyncIndex]->m_pCallback( FFILE_ERROR_RET, _apPendingAsyncReads[nAsyncIndex]->m_hFangHandle, _apPendingAsyncReads[nAsyncIndex]->m_pLastDestBuffer, -1, _apPendingAsyncReads[nAsyncIndex]->m_pUser );
		}
	}

	_apPendingAsyncReads[nAsyncIndex]->m_pCallback = NULL;
	_apPendingAsyncReads[nAsyncIndex]->m_bAsyncReadInProgress = FALSE;
	_apPendingAsyncReads[nAsyncIndex]->m_bAsyncReadReady = FALSE;
	_apPendingAsyncReads[nAsyncIndex] = NULL;
}


//
//
//
s32 ffile_OSDir_Read( CFPlatformFileCookie *pCookie, u32 uReadAmt, void *pDestData, FFileAsyncReadCallback_t *pCallback/*=NULL*/, void *pUser/*=NULL*/ ) 
{
	u32 nBytesRead = 0;

	FASSERT( _bModuleInitialized );
	FASSERT( pCookie && pCookie->m_hFile != INVALID_HANDLE_VALUE );
	
	if ( uReadAmt == 0 ) 
	{
		return 0;
	}

	// Read the data
	if ( pCallback )
	{
		u32 i;

		// Block until there is an open slot
		while ( _nAsyncPendingReadCount == FFILE_MAX_SIMULTANEOUS_ASYNC_READS )
		{
			DEVPRINTF( "ffile_OSDir_Read() - WARNING - Blocking while awaiting an open async read slot.\n" );
		}

		for ( i = 0; i < FFILE_MAX_SIMULTANEOUS_ASYNC_READS; i++ )
		{
			if ( _apPendingAsyncReads[i] == NULL )
			{
				break;
			}
		}

		FASSERT( i != FFILE_MAX_SIMULTANEOUS_ASYNC_READS );
		
		// Are we reading from the masterfile?
		if ( &FFile_paFileCookie[FFile_hAsyncMasterFile] == pCookie )
		{
			// We are reading from the async master file, so we can put this read in the queue
			u16 nNextToken = FFile_nLastMFTokenUsed + 1;
			if ( nNextToken == FFILE_MAX_MF_READ_TOKENS )
			{
				nNextToken = 0;
			}
			
			while ( nNextToken == _LastMFTokenSubmitted )
			{
				// Pause while we wait for the I/O to catch up
//				DEVPRINTF( "ffile_OSDir_Read() - Waiting for a free master file token.\n" );
			}
			
//			DEVPRINTF( "ffile_OSDir_Read() - Placing read %x into token slot %d.\n", pUser, nNextToken );

			// Put this read request into the master file's ring buffer read queue
			QueuedReadToken *pToken = (QueuedReadToken *)&FFile_paMFTokenList[nNextToken];

			pToken->nReadAmount = uReadAmt;
			pToken->nStartPosition = pCookie->m_nSeekPosition;
			pToken->pCallback = pCallback;
			pToken->pDestination = pDestData;
			pToken->pUser = pUser;

			pCookie->m_nSeekPosition = -1;  // Seek comes with the ReadFileEx

			FFile_nLastMFTokenUsed = nNextToken;
		}
		else
		{
			// Block until the file is not being accessed (we only handle one file read at a time for non-masterfile reads)
			while ( pCookie->m_bAsyncReadInProgress == TRUE )
			{
				// Flush the callbacks
				ffile_CheckFileReadStatus( FFILE_INVALID_HANDLE, TRUE );
				DEVPRINTF( "ffile_OSDir_Read() - WARNING - Blocking while waiting for file to become accessible.\n" );
			}
			
			_nAsyncPendingReadCount++;

			_apPendingAsyncReads[i] = pCookie;
			_apPendingAsyncReads[i]->m_pCallback = pCallback;
			if ( pCookie->m_nSeekPosition > -1 )
			{
				_apPendingAsyncReads[i]->m_Overlapped.Offset = pCookie->m_nSeekPosition;
			}
			else
			{
				_apPendingAsyncReads[i]->m_Overlapped.Offset = pCookie->m_nPosition;
			}
			_apPendingAsyncReads[i]->m_Overlapped.OffsetHigh = 0;
			_apPendingAsyncReads[i]->m_Overlapped.hEvent = (HANDLE)i;  // ReadFileEx does not use the handle, so put the Async read index there
			_apPendingAsyncReads[i]->m_bAsyncReadInProgress = TRUE;
			_apPendingAsyncReads[i]->m_pUser = pUser;
			_apPendingAsyncReads[i]->m_nAsyncReadAmount = uReadAmt;
			_apPendingAsyncReads[i]->m_bAsyncReadReady = TRUE;
		
			pCookie->m_nSeekPosition = -1;  // Seek comes with the ReadFileEx
			pCookie->m_pLastDestBuffer  = pDestData;
		}

		ReleaseSemaphore( _hPendingCountSemaphore, 1, NULL );

		return 0;
	}
	else
	{
		// Check to see if there is a pending seek
		if ( pCookie->m_nSeekPosition != -1 )
		{
			if ( SetFilePointer( pCookie->m_hFile, pCookie->m_nSeekPosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER )
			{
				return FFILE_ERROR_RET;
			}
			pCookie->m_nPosition = pCookie->m_nSeekPosition;
			pCookie->m_nSeekPosition = -1;
		}

		pCookie->m_pLastDestBuffer  = pDestData;
		if ( ReadFile( pCookie->m_hFile, pDestData, uReadAmt, (LPDWORD)&nBytesRead, NULL ) == 0 )
		{
			return FFILE_ERROR_RET;
		}
	}

	pCookie->m_nPosition += nBytesRead;

	return nBytesRead;
}


//
//
//
s32 ffile_OSDir_Write( CFPlatformFileCookie *pCookie, u32 uWriteAmt, void *pSrcData ) 
{
	u32 nBytesWritten;

	FASSERT( _bModuleInitialized );
	FASSERT( pCookie && pCookie->m_hFile != INVALID_HANDLE_VALUE );

	// Check to see if there is a pending seek
	if ( pCookie->m_nSeekPosition != -1 )
	{
		if ( SetFilePointer( pCookie->m_hFile, pCookie->m_nSeekPosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER )
		{
			return FFILE_ERROR_RET;
		}
		pCookie->m_nPosition = pCookie->m_nSeekPosition;
		pCookie->m_nSeekPosition = -1;
	}

	if ( uWriteAmt == 0 ) 
	{
		return 0;
	}

	if ( WriteFile( pCookie->m_hFile, pSrcData, uWriteAmt, (LPDWORD)&nBytesWritten, NULL ) == 0 )
	{
		return FFILE_ERROR_RET;
	}

	pCookie->m_nPosition += nBytesWritten;

	return nBytesWritten;
}


//
//
//
s32 ffile_OSDir_EOF( CFPlatformFileCookie *pCookie ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie && pCookie->m_hFile != INVALID_HANDLE_VALUE );

	s32 nSize = ffile_OSDir_GetFileSize( pCookie );

	if ( nSize == FFILE_ERROR_RET )
	{
		return 0;  // Not the end of file
	}

	return (pCookie->m_nPosition >= (u32)nSize);
}


//
//
//
s32 ffile_CheckFileReadStatus( FFileHandle hFangHandle, BOOL bFlushCallbacks ) 
{
	FASSERT( _bModuleInitialized );
/*
	if ( bFlushCallbacks )
	{
		while( SleepEx( 0, TRUE ) == WAIT_IO_COMPLETION )
		{
		};
	}
*/
	if ( !FFILE_IS_VALID_HANDLE( hFangHandle ) )
	{
		return FFILE_STATUS_IDLE;
	}
	
	FASSERT( FFile_paFileCookie[hFangHandle].m_hFile != INVALID_HANDLE_VALUE );

	if ( FFile_paFileCookie[hFangHandle].m_bAsyncReadInProgress )
	{
		return FFILE_STATUS_ASYNC_READ_PENDING;
	}

	return FFILE_STATUS_IDLE;
}


//
//
//
s32 ffile_CancelFileIO( FFileHandle hFangHandle ) 
{
	FASSERT( _bModuleInitialized );

	if ( !FFILE_IS_VALID_HANDLE( hFangHandle ) )
	{
		return FFILE_ERROR_RET;
	}
	
	FASSERT( FFile_paFileCookie[hFangHandle].m_hFile != INVALID_HANDLE_VALUE );

	// Flush all of the pending async requests, ignoring those for this file
	_hCancelledFile = FFile_paFileCookie[hFangHandle].m_hFile;
	while( SleepEx( 0, TRUE ) == WAIT_IO_COMPLETION )
	{
	};

	_hCancelledFile = INVALID_HANDLE_VALUE;

	if ( CancelIo( FFile_paFileCookie[hFangHandle].m_hFile ) == 0 )
	{
		return FFILE_ERROR_RET;
	}

	return 0;
}
