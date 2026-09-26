//////////////////////////////////////////////////////////////////////////////////////
// fGCfile.cpp - Fang GameCube file module.
//
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
// 03/4/02 Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include <stdlib.h>

#include "fang.h"
#include "ffile.h"
#include "fcdrom.h"
#include "fdata.h"
#include "fclib.h"

#include "fgcfile.h"


#define _USE_TEMP_READ_BUFFER	TRUE

//////////////////////////////////////////////////////////////////////////////////////
// External Dependencies:
//////////////////////////////////////////////////////////////////////////////////////

extern volatile u16 FFile_nLastMFTokenRead;
extern volatile u16 FFile_nLastMFTokenUsed;
extern volatile QueuedReadToken *FFile_paMFTokenList;
extern FFileHandle FFile_hMasterFile;
extern FFileHandle FFile_hAsyncMasterFile;

extern char FFile_cLanguageChar;
extern char FFile_cReferenceLanguageChar;	

extern FFileAsyncReadErrorCallback_t *FFile_pAsyncErrorHaltCallback;
extern FFileAsyncReadErrorCallback_t *FFile_pAsyncErrorResumeCallback;

//////////////////////////////////////////////////////////////////////////////////////
// Local Variables:
//////////////////////////////////////////////////////////////////////////////////////

volatile BOOL _bModuleInitialized = FALSE;
volatile u32 _nAsyncPendingReadCount;
volatile CFPlatformFileCookie *_apPendingAsyncReads[FFILE_MAX_SIMULTANEOUS_ASYNC_READS];

volatile u32 _LastMFTokenSubmitted;
volatile BOOL _bAsyncBlockReadDone;
volatile s32  _nAsyncBlockReadResult;

static OSSemaphore	_PendingCountSemaphore;
static OSThread		AsyncThread;
static u8			AsyncThreadStack[4 * 1024];

static s32 _nLastDriveStatus = DVD_STATE_END;
static FFileAsyncReadErrorCode_e _eDVDErrorCode = FFILE_ASYNC_READ_ERROR_NONE;
static FFileAsyncReadErrorCode_e _eLastDVDErrorCode = FFILE_ASYNC_READ_ERROR_NONE;
static BOOL _bInvokedHaltCallback = FALSE;


#if _USE_TEMP_READ_BUFFER
	// Because GC has the following requirements for it's DVD access - read buffers
	// must be 32 byte aligned, start read positions must be 4-byte aligned, read
	// amounts must be 32-byte aligned, we create a buffer to read data into, first,
	// and then copy it over to the submitted buffer (unless the submitted buffer,
	// read position and amount satisfy the above).  This should only be used until
	// a game data file is created which will be loaded, in its entirety, into memory.
	#define _READ_BUFFER_SIZE		32768
	static void *_pTempReadBuffer	= NULL;
#endif // _USE_TEMP_READ_BUFFER


//////////////////////////////////////////////////////////////////////////////////////
// Local Functions:
//////////////////////////////////////////////////////////////////////////////////////

static void _AsyncReadCallback( s32 nResult, DVDFileInfo *pFileInfo );
static BOOL _NextMFRead( u32 nReadIdx );
static void* AsyncFileThread( void *pParam );
static BOOL _DetectDVDErrors( void );

//
//
//
BOOL fgcfile_ModuleStartup( void ) 
{
	u32 i;

	DVDInit();

	_nAsyncPendingReadCount = 0;
	
#if _USE_TEMP_READ_BUFFER
	// We use OSAlloc so as not to mess with the game allocations
//	_pTempReadBuffer = OSAlloc( _READ_BUFFER_SIZE );
	_pTempReadBuffer = fres_AlignedAlloc( _READ_BUFFER_SIZE, 32 );
#endif // _USE_TEMP_READ_BUFFER

	for ( i = 0; i < FFILE_MAX_SIMULTANEOUS_ASYNC_READS; i++ )
	{
		_apPendingAsyncReads[i] = NULL;
	}

	OSInitSemaphore( &_PendingCountSemaphore, 0 );

	if ( !OSCreateThread(
		&AsyncThread,								// ptr to the thread to init
		AsyncFileThread,							// ptr to the start routine
		NULL,										// param passed to start routine
		AsyncThreadStack + sizeof AsyncThreadStack,	// initial stack address
		sizeof AsyncThreadStack,					// stack size
		31,											// scheduling priority
		OS_THREAD_ATTR_DETACH ) )					// detached by default
	{
		DEVPRINTF( "CreateThread failed.\r\n" );
		return FALSE;
	}

	OSResumeThread( &AsyncThread );

	// Clear the platform specific section of the cookies
	for ( i = 0; i < FFile_nMaxSimultaneousOpen; i++ )
	{
		FFile_paFileCookie[i].m_nEntryNum  = -1;
		FFile_paFileCookie[i].m_nSeekPosition  = -1;
		FFile_paFileCookie[i].m_nPosition  = 0;
		FFile_paFileCookie[i].m_nSize 	 = -1;
		FFile_paFileCookie[i].m_pCallback  = NULL;
		FFile_paFileCookie[i].m_pLastDestBuffer  = NULL;
	}
	

	_LastMFTokenSubmitted = 0;
	
	_bModuleInitialized = TRUE;
	
	return TRUE;
}


//
//
//
BOOL fgcfile_ModuleShutdown( void )
{
	u32 i;
	for ( i = 0; i < FFile_nMaxSimultaneousOpen; i++ )
	{
		FFile_paFileCookie[i].m_nEntryNum = -1;
		FFile_paFileCookie[i].m_nSeekPosition = -1;
		FFile_paFileCookie[i].m_nPosition = 0;
		FFile_paFileCookie[i].m_nSize 	= -1;
		FFile_paFileCookie[i].m_pCallback = NULL;
		FFile_paFileCookie[i].m_pLastDestBuffer  = NULL;
	}
	
	DVDCancelAll();

	for ( i = 0; i < FFILE_MAX_SIMULTANEOUS_ASYNC_READS; i++ )
	{
		_apPendingAsyncReads[i] = NULL;
	}

	_LastMFTokenSubmitted = 0;
	
	_bModuleInitialized = FALSE;
	
	return TRUE;
}



//
//
//
void ffile_Work( void ) {
	// Check to see if an error condition has occured from any ASYNC file activity
	fgcfile_HandleDVDErrors( FALSE, FALSE );
}



//
//
//
FFileHandle ffile_OSDir_Open( cchar *pszFName, FFileOpenType_e nOpenMode, BOOL bAsyncAndAligned/*=FALSE*/ ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( pszFName );

	s32 nFileIndex;
	for ( nFileIndex = 0; nFileIndex < FFile_nMaxSimultaneousOpen; nFileIndex++ ) 
	{
		if ( FFile_paFileCookie[nFileIndex].m_bUsedEntry == FALSE ) 
		{
			break;
		}
	}

	if ( nFileIndex == FFile_nMaxSimultaneousOpen )
	{
		DEVPRINTF( "ffile_OSDir_Open() : Maximum number of files (%d) already opened.\n", FFile_nMaxSimultaneousOpen );
		return FFILE_ERROR_RET;
	}

	// Directory mode...

	switch( nOpenMode ) 
	{
		case FFILE_OPEN_RONLY_SEQUENTIAL:
		case FFILE_OPEN_RONLY_RANDOM:
		case FFILE_OPEN_RONLY:
			break;

		case FFILE_OPEN_WONLY:
			DEVPRINTF( "FGCFILE.CPP : File writing not supported on GameCube.\n" );
			return FFILE_ERROR_RET;
			break;

		case FFILE_OPEN_TRUNC_OR_CREATE_WONLY:
			DEVPRINTF( "FGCFILE.CPP : File created not supported on GameCube.\n" );
			return FFILE_ERROR_RET;
			break;

		default:
			FASSERT_NOW;
	}

	// The GameCube will convert paths to DVD entry numbers.  By
	// passing this entry number back to the requesting function,
	// we can treat the entry number much like a file pointer since
	// it will not change.
	char szFileName[ 128 ];

	fclib_strcpy( szFileName, pszFName );

	// Modify the filename to a localized version (if need be)
	ffile_LocalizeFilename( szFileName );

	s32 s32EntryNum = DVDConvertPathToEntrynum( szFileName );
	
	if ( s32EntryNum == -1 )
	{
		DEVPRINTF( "FGCFILE.CPP : Unable to locate file %s\n", pszFName );
		return FFILE_ERROR_RET;
	}

	if ( !DVDFastOpen( s32EntryNum, &FFile_paFileCookie[nFileIndex].m_FileInfo ) )
	{
		DEVPRINTF( "FGCFILE.CPP : Unable to locate file %s\n", pszFName );
		return FFILE_ERROR_RET;
	}
	
	FFile_paFileCookie[nFileIndex].m_bUsedEntry = TRUE;
	FFile_paFileCookie[nFileIndex].m_pMasterDirEntry = NULL;
	FFile_paFileCookie[nFileIndex].m_nEntryNum  = s32EntryNum;
	FFile_paFileCookie[nFileIndex].m_nPosition  = 0;
	FFile_paFileCookie[nFileIndex].m_nSeekPosition  = -1;
	FFile_paFileCookie[nFileIndex].m_nSize	  = -1;
	FFile_paFileCookie[nFileIndex].m_pCallback  = NULL;
	FFile_paFileCookie[nFileIndex].m_pLastDestBuffer  = NULL;

	return nFileIndex;
}


//
//
//
s32 ffile_OSDir_Close( CFPlatformFileCookie *pCookie )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie );

	// Make sure the file is open	
	if ( pCookie->m_bUsedEntry == FALSE )
	{
		DEVPRINTF( "ffile_OSDir_Close() - Calling function provided an unused cookie.\n" );
		return FFILE_ERROR_RET;
	}
	
	ffile_CancelFileIO( pCookie->m_hFangHandle );
	
	// Block on any pending async reads
	while ( DVDGetFileInfoStatus( &pCookie->m_FileInfo ) == DVD_FILEINFO_BUSY )
	{
	}
	
	// Close the file
	if ( !DVDClose( &pCookie->m_FileInfo ) )
	{
		DEVPRINTF( "ffile_OSDir_Close() - DVDClose() returned an error.\n" );
		return FFILE_ERROR_RET;
	}

	// Clear the cookie info
	pCookie->m_bUsedEntry = FALSE;
	pCookie->m_nEntryNum = -1;
	pCookie->m_nPosition = 0;
	pCookie->m_nSeekPosition = -1;
	pCookie->m_nSize 	= -1;
	pCookie->m_pLastDestBuffer  = NULL;

	return 0;
}


//
//
//
s32 ffile_OSDir_GetFileSize( CFPlatformFileCookie *pCookie )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie );

	if ( pCookie->m_bUsedEntry == FALSE || pCookie->m_nEntryNum == -1 )
	{
		return FFILE_ERROR_RET;
	}

	if ( pCookie->m_nSize != -1 )
	{
		return pCookie->m_nSize;
	}

	pCookie->m_nSize = DVDGetLength( &pCookie->m_FileInfo );
	
	// Round this up to a multiple of 32 because this value is most
	// likely going to be used to set up a read buffer.
	pCookie->m_nSize = OSRoundUp32B( pCookie->m_nSize );
	
	return pCookie->m_nSize;
}


//
//
//
s32 ffile_OSDir_Seek( CFPlatformFileCookie *pCookie, s32 nFileOffset, FFileSeekType_e nSeekMode )
{
	FASSERT( _bModuleInitialized );
	FASSERT( nSeekMode >= 0 && nSeekMode < FFILE_SEEK_COUNT );
	FASSERT( pCookie );
	
	if ( !pCookie->m_bUsedEntry || pCookie->m_nEntryNum == -1 )
		return FFILE_ERROR_RET;

	// If we don't already have the size for this file, get it
	if ( pCookie->m_nSize == -1 )
	{
		ffile_OSDir_GetFileSize( pCookie );
	}
	
	switch ( nSeekMode )
	{
		case FFILE_SEEK_SET:
		{
			if ( nFileOffset > pCookie->m_nSize || nFileOffset < 0 )
				return FFILE_ERROR_RET;
				
#if !_USE_TEMP_READ_BUFFER		
			if ( nFileOffset & 0x03 != 0 )
			{
				DEVPRINTF( "FGCFILE.CPP : Seek positions for GC must be multiples of 4.\n" );
				return FFILE_ERROR_RET;
			}
#endif // !_USE_TEMP_READ_BUFFER
				
			pCookie->m_nSeekPosition = nFileOffset;
			
			break;
		}
		
		case FFILE_SEEK_CUR:
		{
			s32 nNewPos = pCookie->m_nPosition + nFileOffset;
			if ( nNewPos > pCookie->m_nSize || nNewPos < 0 )
			{
				return FFILE_ERROR_RET;
			}
				
#if !_USE_TEMP_READ_BUFFER
			if ( nNewPos & 0x03 != 0 )
			{
				DEVPRINTF( "FGCFILE.CPP : Seek positions for GC must be multiples of 4.\n" );
				return FFILE_ERROR_RET;
			}
#endif // !_USE_TEMP_READ_BUFFER
				
			pCookie->m_nSeekPosition = nNewPos;
			
			break;
		}
			
		case FFILE_SEEK_END:
		{
			s32 nNewPos = pCookie->m_nSize - nFileOffset;
			if ( nNewPos < 0 )
			{
				return FFILE_ERROR_RET;
			}
			
#if !_USE_TEMP_READ_BUFFER
			if ( nNewPos & 0x03 != 0 )
			{
				DEVPRINTF( "FGCFILE.CPP : Seek positions for GC must be multiples of 4.\n" );
				return FFILE_ERROR_RET;
			}
#endif // !_USE_TEMP_READ_BUFFER
				
			pCookie->m_nSeekPosition = nNewPos;
			
			break;
		}
		
		default:
			break;
	}

	return 0;
}


//
//
//
s32 ffile_OSDir_Tell( CFPlatformFileCookie *pCookie )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie );

	if ( !pCookie->m_bUsedEntry || pCookie->m_nEntryNum == -1 )
	{
		return FFILE_ERROR_RET;
	}

	return pCookie->m_nSeekPosition;
}


static void* AsyncFileThread( void *pParam ) 
{ 
	#pragma unused( pParam )

	while( TRUE )
	{
		OSWaitSemaphore( &_PendingCountSemaphore );
		BOOL bExitLoop = FALSE;
		do {
			bExitLoop = _DetectDVDErrors();
			if( !bExitLoop ) {
				// Sleep the thread until next Vsync
				VIWaitForRetrace();
			}
			VIWaitForRetrace();
		} while ( !bExitLoop );
	}

	DEVPRINTF( "EXITING ASYNC DETECTION THREAD!" );
    return 0; 
} 


//
//
//
static void _AsyncBlockOnReadCallback( s32 nResult, DVDFileInfo *pFileInfo )
{
	_bAsyncBlockReadDone = TRUE;
	_nAsyncBlockReadResult = nResult;
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
		
		// This should never be the case
		FASSERT( DVDGetFileInfoStatus( &FFile_paFileCookie[FFile_hAsyncMasterFile].m_FileInfo ) == DVD_FILEINFO_READY );
		
		// Get the next token in the token link list
		QueuedReadToken *pToken = (QueuedReadToken *)&FFile_paMFTokenList[nNextToRead];
		_apPendingAsyncReads[nReadIdx] = &FFile_paFileCookie[FFile_hAsyncMasterFile];
		_apPendingAsyncReads[nReadIdx]->m_pCallback = pToken->pCallback;
		_apPendingAsyncReads[nReadIdx]->m_pUser = pToken->pUser;
		_apPendingAsyncReads[nReadIdx]->m_pLastDestBuffer = pToken->pDestination;
		_apPendingAsyncReads[nReadIdx]->m_nPosition = pToken->nStartPosition;
		DVDSetUserData( (DVDCommandBlock *)&_apPendingAsyncReads[nReadIdx]->m_FileInfo, (void *)nReadIdx );
		if ( DVDReadAsync( (DVDFileInfo *)&_apPendingAsyncReads[nReadIdx]->m_FileInfo, 
							  _apPendingAsyncReads[nReadIdx]->m_pLastDestBuffer, 
							  pToken->nReadAmount, 
							  _apPendingAsyncReads[nReadIdx]->m_nPosition, 
							  _AsyncReadCallback ) != FALSE ) // used to be == TRUE
		{
			OSSignalSemaphore( &_PendingCountSemaphore ); // have the error detection thread do it's thing...
			_LastMFTokenSubmitted = nNextToRead;
			_nAsyncPendingReadCount++;
			return TRUE;
		}
		
		DEVPRINTF( "_NextMFRead() - Master file async read submission failed.\n" );
		
		FFile_nLastMFTokenRead = nNextToRead;
		
		if ( nNextToRead == FFile_nLastMFTokenUsed )
		{
			break;
		}
	}
	
	_apPendingAsyncReads[nReadIdx]->m_pCallback = NULL;
	_apPendingAsyncReads[nReadIdx] = NULL;
	
	return FALSE;
}


//
//
//
static void _AsyncReadCallback( s32 nResult, DVDFileInfo *pFileInfo )
{
	s32 nReadIdx = (u32)DVDGetUserData( (DVDCommandBlock *)pFileInfo );
	FASSERT( nReadIdx >= 0 && nReadIdx < FFILE_MAX_SIMULTANEOUS_ASYNC_READS );

	volatile CFPlatformFileCookie *pAsyncRead = _apPendingAsyncReads[nReadIdx];
	FASSERT( pAsyncRead );
	
	if ( nResult > 0 )
	{
		pAsyncRead->m_nPosition += nResult;
		pAsyncRead->m_pCallback( 0, pAsyncRead->m_hFangHandle, pAsyncRead->m_pLastDestBuffer, nResult, pAsyncRead->m_pUser );
	}
	else
	{
		pAsyncRead->m_pCallback( FFILE_ERROR_RET, pAsyncRead->m_hFangHandle, pAsyncRead->m_pLastDestBuffer, nResult, pAsyncRead->m_pUser );
	}
	
	_nAsyncPendingReadCount--;
	
	if ( &FFile_paFileCookie[FFile_hAsyncMasterFile] == _apPendingAsyncReads[nReadIdx] )
	{
		FFile_nLastMFTokenRead++;
		if ( FFile_nLastMFTokenRead == FFILE_MAX_MF_READ_TOKENS )
		{
			FFile_nLastMFTokenRead = 0;
		}
		
		if ( FFile_nLastMFTokenRead != FFile_nLastMFTokenUsed )
		{
			_NextMFRead( nReadIdx );
			return;
		}
	}
	
	pAsyncRead->m_pCallback = NULL;
	_apPendingAsyncReads[nReadIdx] = NULL;
}


//
//
//
s32 ffile_OSDir_Read( CFPlatformFileCookie *pCookie, u32 uReadAmt, void *pDestData, FFileAsyncReadCallback_t *pCallback, void *pUser )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie );

	if ( !pCookie->m_bUsedEntry || pCookie->m_nEntryNum == -1 )
	{
		return FFILE_ERROR_RET;
	}

	s32 nRead = 0;
	
	if ( pCookie->m_nSeekPosition == -1 )
	{
		pCookie->m_nSeekPosition = pCookie->m_nPosition;
	}
	
	// If no amount was requested, bail	
	if ( uReadAmt == 0 ) 
	{
		return 0;
	}
	else if ( uReadAmt + pCookie->m_nSeekPosition > pCookie->m_nSize )
	{
		uReadAmt = pCookie->m_nSize - pCookie->m_nSeekPosition;
	}

#if !_USE_TEMP_READ_BUFFER
	if ( ((u32)pDestData & 0x1f) != 0 )
	{
		DEVPRINTF( "FGCFILE.CPP : Pointers for destination data on GC must be 32 byte aligned.\n" );
		return FFILE_ERROR_RET;
	}

	if ( (uReadAmt & 0x1f) != 0 )
	{
		DEVPRINTF( "FGCFILE.CPP : Bytes to be read from DVD on GC must be multiples of 32.\n" );
		return FFILE_ERROR_RET;
	}
	
	if ( (pCookie->m_nSeekPosition & 0x03) != 0 )
	{
		DEVPRINTF( "FGCFILE.CPP : Seek positions for GC must be multiples of 4.\n" );
		return FFILE_ERROR_RET;
	}
	
	// Read the data into the buffer
	nRead = DVDRead( &pCookie->m_FileInfo, pDestData, uReadAmt, pCookie->m_nSeekPosition );
	pCookie->m_nPosition = pCookie->m_nSeekPosition;
	pCookie->m_nSeekPosition = -1;

	if ( nRead < 0 )
	{
		return FFILE_ERROR_RET;
	}

	pCookie->m_pLastDestBuffer = pDestData;
	pCookie->m_nPosition += nRead;
	
#else // !_USE_TEMP_READ_BUFFER

	// Check alignment of data
	u32  bBadAlignment =  ((u32)pDestData & 0x0000001f)
			 				| (pCookie->m_nSeekPosition & 0x00000003)
							| (uReadAmt & 0x1f);

	if ( bBadAlignment )
	{
		if ( pCallback )
		{
			DEVPRINTF( "FGCFILE.CPP : ERROR - Improper asynchronous read parameters.\n" );
			DEVPRINTF( "FGCFILE.CPP : Destination buffers and bytes to be read must be multiples of 32.\n" );
			DEVPRINTF( "FGCFILE.CPP : Seek Positions must be 4 byte aligned.\n" );
			DEVPRINTF( "FGCFILE.CPP : Current Request:\n" );
			DEVPRINTF( "FGCFILE.CPP : Destination Buffer: %X\n", (u32)pDestData );
			DEVPRINTF( "FGCFILE.CPP : Bytes To Be Read: %d\n", uReadAmt );
			DEVPRINTF( "FGCFILE.CPP : Seek Address: %d\n", pCookie->m_nSeekPosition );
			FASSERT_NOW;
			return FFILE_ERROR_RET;
		}
		
		// Calculate aligned Seek Start
		u32 nSeekStart = (pCookie->m_nSeekPosition & (~3));
		s32 nSeekRoundDown = pCookie->m_nSeekPosition - nSeekStart;
		
		// Calculate aligned amount to read
		s32 nAmountRoundUp = ((uReadAmt + 31) & (~31)) - uReadAmt;
		s32 nBytesLeft = uReadAmt + nAmountRoundUp;
		
		u32 nBytesToRead = 0;
		s32 nBytesActuallyRead = -1;

		// Loop until we've collected all of the data from the DVD
		// We need to do this if the amount of data requested exceeds
		// the size of the temp buffer.
		while ( nBytesLeft )
		{
			// Calculate the amount to read into the temp buffer this time
			if ( nBytesLeft <= _READ_BUFFER_SIZE )
			{
				nBytesToRead = nBytesLeft;
			}
			else
			{
				nBytesToRead = _READ_BUFFER_SIZE;
			}

#if 0
			// RAF -- Commented out because we need to do async reads to query
			// the status of the drive...
			// Read from the file into the temp buffer
			nBytesActuallyRead = DVDRead( &pCookie->m_FileInfo, _pTempReadBuffer, nBytesToRead, nSeekStart );
			// If we got an error, bail									
			if ( nBytesActuallyRead < 0 )
			{
				return FFILE_ERROR_RET;
			}
#else
			_bAsyncBlockReadDone = FALSE;
			BOOL bSuccess = DVDReadAsync( &pCookie->m_FileInfo, _pTempReadBuffer, nBytesToRead, nSeekStart, _AsyncBlockOnReadCallback );
			fgcfile_HandleDVDErrors( TRUE, TRUE ); //Deal with any dvd errors that may crop up...
			// Block until the file is done reading
			while ( 1 )
			{
				if ( _bAsyncBlockReadDone )
				{
					if ( _nAsyncBlockReadResult <= 0 )
					{
						return FFILE_ERROR_RET;
					}
					
					nBytesActuallyRead = _nAsyncBlockReadResult;
					break;
				}
			}
#endif
			// Adjust the seek and bytes left for next time through
			nSeekStart += nBytesActuallyRead;
			nBytesLeft -= nBytesActuallyRead;
			
			// Calculate relevant bytes read
			if ( nBytesLeft <= 0 )
			{
				nBytesActuallyRead = nBytesActuallyRead - nSeekRoundDown - nAmountRoundUp;
			}
			else
			{
				nBytesActuallyRead = nBytesActuallyRead - nSeekRoundDown;
			}

			FASSERT( nRead + nBytesActuallyRead <= uReadAmt );

			// Copy the relevant data from the temp buffer to the submitted buffer
			fang_MemCopy( &((u8 *)pDestData)[nRead], ((u8 *)_pTempReadBuffer) + nSeekRoundDown, nBytesActuallyRead );
			
			// Increment relevant bytes read
			nRead += nBytesActuallyRead;
			
			// Only apply seek round down adjustments the first time through
			nSeekRoundDown = 0;			
		}
		
		pCookie->m_pLastDestBuffer = pDestData;
		pCookie->m_nPosition = pCookie->m_nSeekPosition + nRead;
		pCookie->m_nSeekPosition = -1;
	}
	else
	{
		// Read the data into the submitted buffer because alignment is a-okay
		if ( pCallback )
		{
			u32 i;
			
			// Block until there is an open slot
			while ( _nAsyncPendingReadCount == FFILE_MAX_SIMULTANEOUS_ASYNC_READS )
			{
				DEVPRINTF( "ffile_OSDir_Read() - WARNING - Blocking while awaiting an open async read slot.\n" );
			}

			// Find the first available async read slot
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
				u16 nNextToken = FFile_nLastMFTokenUsed + 1;
				if ( nNextToken == FFILE_MAX_MF_READ_TOKENS )
				{
					nNextToken = 0;
				}
				
				while ( nNextToken == _LastMFTokenSubmitted )
				{
					// Pause while we wait for the I/O to catch up
					DEVPRINTF( "ffile_OSDir_Read() - Waiting for a free master file token.\n" );
				}
				
				// Put this read request into the master file's ring buffer read queue
				QueuedReadToken *pToken = (QueuedReadToken *)&FFile_paMFTokenList[nNextToken];

				pToken->nReadAmount = uReadAmt;
				pToken->nStartPosition = pCookie->m_nSeekPosition;
				pToken->pCallback = pCallback;
				pToken->pDestination = pDestData;
				pToken->pUser = pUser;

				pCookie->m_nSeekPosition = -1;  // Seek comes with the ReadFileEx

				if ( FFile_nLastMFTokenUsed == FFile_nLastMFTokenRead )
				{
					// File system is caught up with us, so send out this read
					FFile_nLastMFTokenUsed = nNextToken;
					_NextMFRead( i );
				}
				else
				{
					FFile_nLastMFTokenUsed = nNextToken;
				}
				
				return 0;
			}
			
			// Block until the file is not being accessed
			s32 nStatus;
			BOOL bWaiting = TRUE;
			#if !FANG_PRODUCTION_BUILD
				BOOL bWarningIssued = FALSE;
			#endif
			while ( bWaiting )
			{
				nStatus = DVDGetCommandBlockStatus( (DVDCommandBlock *)&pCookie->m_FileInfo );

				switch ( nStatus )
				{
					case DVD_STATE_END:
					case DVD_STATE_CANCELED:
						// File is ready for reading
						bWaiting = FALSE;
						break;
						
					case DVD_STATE_FATAL_ERROR:
						DEVPRINTF( "ffile_OSDir_Read() - ERROR - Fatal error occurred while attempting to access file status.\n" );
						return FFILE_ERROR_RET;	
						
					case DVD_STATE_NO_DISK:
						DEVPRINTF( "ffile_OSDir_Read() - ERROR - No DVD in the drive when attempting to access file status.\n" );
						return FFILE_ERROR_RET;	
						
					case DVD_STATE_COVER_OPEN:
						DEVPRINTF( "ffile_OSDir_Read() - ERROR - Drive cover open when attempting to access file status.\n" );
						return FFILE_ERROR_RET;
						
					case DVD_STATE_WRONG_DISK:
						DEVPRINTF( "ffile_OSDir_Read() - ERROR - Wrong DVD in drive when attempting to access file status.\n" );
						return FFILE_ERROR_RET;
						
					default:
						break;
				}
				
				#if !FANG_PRODUCTION_BUILD
					if ( !bWarningIssued && bWaiting )
					{
						DEVPRINTF( "ffile_OSDir_Read() - WARNING - Blocking while waiting for file to become accessible.\n" );
						bWarningIssued = TRUE;
					}
				#endif
			}
			
			_apPendingAsyncReads[i] = pCookie;
			_apPendingAsyncReads[i]->m_pCallback = pCallback;
			_apPendingAsyncReads[i]->m_pUser = pUser;
			DVDSetUserData( (DVDCommandBlock *)&pCookie->m_FileInfo, (void *)i );
			
			_nAsyncPendingReadCount++;

			pCookie->m_pLastDestBuffer = pDestData;
			pCookie->m_nPosition = pCookie->m_nSeekPosition;
			pCookie->m_nSeekPosition = -1;
			DVDReadAsync( &pCookie->m_FileInfo, 
								pDestData, 
								uReadAmt, 
								pCookie->m_nPosition,
								_AsyncReadCallback );
			OSSignalSemaphore( &_PendingCountSemaphore ); // have the error detection thread do it's thing...

			return 0;
		}
		else
		{
			_bAsyncBlockReadDone = FALSE;
			pCookie->m_pLastDestBuffer = pDestData;
			pCookie->m_nPosition = pCookie->m_nSeekPosition;
			pCookie->m_nSeekPosition = -1;
			nRead = DVDReadAsync( &pCookie->m_FileInfo, pDestData, uReadAmt, pCookie->m_nPosition, _AsyncBlockOnReadCallback );
			fgcfile_HandleDVDErrors( TRUE, TRUE ); // Deal with any errors that may crop up...			
			
			// Block until the file is done reading
			while ( 1 )
			{
				if ( _bAsyncBlockReadDone )
				{
					if ( _nAsyncBlockReadResult <= 0 )
					{
						return FFILE_ERROR_RET;
					}
					
					nRead = _nAsyncBlockReadResult;
					break;
				}
			}

			pCookie->m_nPosition += nRead;
		}
	}
		
#endif // !_USE_TEMP_READ_BUFFER

	return nRead;
}


//
//
//
s32 ffile_OSDir_Write( CFPlatformFileCookie *pCookie, u32 uWriteAmt, void *pSrcData )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pCookie );
	
	// If no amount was requested, bail	
	if( uWriteAmt == 0 ) 
	{
		return 0;
	}

	// Can't write on GC
	return FFILE_ERROR_RET;
}


//
//
//
s32 ffile_OSDir_EOF( CFPlatformFileCookie *pCookie )
{
	FASSERT( pCookie );
	FASSERT( _bModuleInitialized );

	// Make sure the file is valid and open
	if ( !pCookie->m_bUsedEntry || pCookie->m_nEntryNum == -1 )
	{
		return FFILE_ERROR_RET;
	}

	// If we don't already have the size for this file, get it
	if ( pCookie->m_nSize == -1 )
	{
		ffile_OSDir_GetFileSize( pCookie );
	}
	
	if ( pCookie->m_nPosition < pCookie->m_nSize )
	{
		// Return not-end-of-file...
		return 0;
	}

	return TRUE;
}


//
//
//
s32 ffile_CheckFileReadStatus( FFileHandle hFangHandle, BOOL bFlushCallbacks ) 
{
	FASSERT( _bModuleInitialized );
	
	if ( !FFILE_IS_VALID_HANDLE( hFangHandle ) )
	{
		return FFILE_STATUS_IDLE;
	}
	
	// Make sure the file is valid and open
	if ( !FFile_paFileCookie[hFangHandle].m_bUsedEntry || FFile_paFileCookie[hFangHandle].m_nEntryNum == -1 )
	{
		return FFILE_ERROR_RET;
	}

	if ( DVDGetFileInfoStatus( &FFile_paFileCookie[hFangHandle].m_FileInfo ) == DVD_FILEINFO_BUSY )
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
	
	// Make sure the file is valid and open
	if ( !FFile_paFileCookie[hFangHandle].m_bUsedEntry || FFile_paFileCookie[hFangHandle].m_nEntryNum == -1 )
	{
		return FFILE_ERROR_RET;
	}

	if ( DVDCancel((DVDCommandBlock*)&FFile_paFileCookie[hFangHandle].m_FileInfo) == -1 )
	{
		return FFILE_ERROR_RET;
	}

	return 0;
}

// This routine WILL BLOCK if an error is detected
BOOL fgcfile_HandleDVDErrors( BOOL bDetectError, BOOL bBlockWhileError ) {

	BOOL bWasError = FALSE;

	BOOL bExitCondition = TRUE;
	
	do {
		if( bDetectError ) {
			bExitCondition = _DetectDVDErrors();
		}
		
		if( _eDVDErrorCode != FFILE_ASYNC_READ_ERROR_NONE ) {
			bWasError = TRUE;
			if( FFile_pAsyncErrorHaltCallback ) {
				FFile_pAsyncErrorHaltCallback( _eDVDErrorCode, bBlockWhileError );
				_bInvokedHaltCallback = TRUE;
			}
		}
		
		_eLastDVDErrorCode = _eDVDErrorCode;
		  
	} while ( bBlockWhileError  && ( _eDVDErrorCode != FFILE_ASYNC_READ_ERROR_NONE || !bExitCondition ) );

	// Now, we should resume!
	if( ( _eDVDErrorCode == FFILE_ASYNC_READ_ERROR_NONE ) && _bInvokedHaltCallback && FFile_pAsyncErrorResumeCallback ) {
		FFile_pAsyncErrorResumeCallback( FFILE_ASYNC_READ_ERROR_NONE, bBlockWhileError );
		_bInvokedHaltCallback = FALSE;
	}
	return bWasError;
}



// Returns true when:
//	The drive status is DVD_STATE_END or DVD_STATE_FATAL_ERROR

static BOOL _DetectDVDErrors( void ) {
	BOOL bDriveOKOrFatalError = FALSE;
	s32 nDriveStatus;

	nDriveStatus = DVDGetDriveStatus();
	if( nDriveStatus != _nLastDriveStatus ) {
		switch ( nDriveStatus ) {
			case DVD_STATE_END:
				_eDVDErrorCode = FFILE_ASYNC_READ_ERROR_NONE;
			break;
			
			case DVD_STATE_COVER_OPEN:
				_eDVDErrorCode = FFILE_ASYNC_READ_ERROR_DVD_COVER_OPEN;
				DEVPRINTF( "ffile_OSDir_Read() - ERROR - Drive cover open.\n" );
			break;
			case DVD_STATE_NO_DISK:
				_eDVDErrorCode = FFILE_ASYNC_READ_ERROR_DVD_NO_DISK;
				DEVPRINTF( "ffile_OSDir_Read() - ERROR - No Disk in DVD.\n" );
			break;
			case DVD_STATE_WRONG_DISK:
				_eDVDErrorCode = FFILE_ASYNC_READ_ERROR_DVD_WRONG_DISK;
				DEVPRINTF( "ffile_OSDir_Read() - ERROR - Wrong Disk in DVD.\n" );
			break;
			case DVD_STATE_RETRY:
				_eDVDErrorCode = FFILE_ASYNC_READ_ERROR_DVD_RETRY;
				DEVPRINTF( "ffile_OSDir_Read() - ERROR - DVD Retry.\n" );
			break;
			case DVD_STATE_FATAL_ERROR:
				_eDVDErrorCode = FFILE_ASYNC_READ_ERROR_DVD_FATAL_ERROR;
				DEVPRINTF( "ffile_OSDir_Read() - ERROR - FATAL DVD ERROR.\n" );
			break;
		}			
	}
	_nLastDriveStatus = nDriveStatus;

	return ( ( nDriveStatus == DVD_STATE_END ) || ( nDriveStatus == DVD_STATE_FATAL_ERROR ) );
}

