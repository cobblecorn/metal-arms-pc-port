//////////////////////////////////////////////////////////////////////////////////////
// ffile.cpp - Fang file module.
//
// Author: Steve Ranck     
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
// 05/02/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "fang.h"
#include "ffile.h"
#include "fcdrom.h"
#include "fdata.h"
#include "fclib.h"
#include "fres.h"

#if FANG_PLATFORM_WIN
	#include <time.h>
#endif

#if FANG_PLATFORM_GC
	#include "fgcfile.h"
#elif FANG_PLATFORM_DX
	#include "dx\fdx8file.h"
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#include "fps2file.h"
//ARG - <<<<<
#endif


//====================
// private definitions

#define _ENABLE_TIMING_LOADS		(TRUE & FANG_ENABLE_DEV_FEATURES)


//=================
// public variables


#if _ENABLE_TIMING_LOADS
	// Used to time loads
	#include "ftimer.h"
	f32 FFile_fSecondsInModule = 0.f;
	BOOL _bAlreadyTiming = FALSE;
	class CFFileTimer
	{
	public:
		BOOL	m_bAccumulateTime;
		CFTimer m_FunctionTimer;
		CFFileTimer( void )
		{
			m_bAccumulateTime = FALSE;
			if ( !_bAlreadyTiming )
			{
				m_bAccumulateTime = TRUE;
				m_FunctionTimer.Reset();
				_bAlreadyTiming = TRUE;
			}
		}
		~CFFileTimer( void )
		{
			if ( m_bAccumulateTime )
			{
				FFile_fSecondsInModule += m_FunctionTimer.SampleSeconds();
				_bAlreadyTiming = FALSE;
			}
		}
	};
#endif

CFPlatformFileCookie *FFile_paFileCookie;
u32 FFile_nMaxSimultaneousOpen = 0;

volatile QueuedReadToken *FFile_paMFTokenList;
volatile u16 FFile_nLastMFTokenRead;
volatile u16 FFile_nLastMFTokenUsed;

FFileHandle FFile_hMasterFile;
FFileHandle FFile_hAsyncMasterFile;

// Localization variables
char FFile_cLanguageChar;		// Default to English
char FFile_cAudioLanguageChar;	// Used for audio banks, because Musyx does not allow funky characters
char FFile_cReferenceLanguageChar;	

// Async ReadError Callback variables
FFileAsyncReadErrorCallback_t *FFile_pAsyncErrorHaltCallback;
FFileAsyncReadErrorCallback_t *FFile_pAsyncErrorResumeCallback;

//==================
// private variables

static BOOL	_bModuleInitialized;
static BOOL _bMasterMode;
static u32 _nMasterDirEntries;
static const FFileMasterEntry_t *_pMasterDir;

#if FANG_PLATFORM_WIN
	// Used for file logging
	static char _szLogFilename[ 256 ];
	static BOOL _bLogSystemOK = FALSE;
	static BOOL _bLogFiles = FALSE;
#endif

//===================
// Private Prototypes

static BOOL _ReadMasterDir( void );
static void _ResetMasterModeVars( void );
static int _QSortCompareFcn( const void *arg1, const void *arg2 );
static s32  _GetDirIndexOfRawFileName( cchar *pszRawName );
static cchar *_FindStartOfRawFileName( cchar *pszPathName );
static s32  _GetDirIndexOfFullPathName( cchar *pszPathName );

//=================
// Public Functions


//
//
//
BOOL ffile_ModuleStartup( void ) 
{
	u32 i;

	FASSERT( (sizeof(int) >= sizeof(void *)) ); // No 64-bit pointers without 64-bit ints...
	FASSERT( (sizeof(int) >= sizeof(s32)) );
	FASSERT( !_bModuleInitialized );

	FFile_pAsyncErrorHaltCallback = NULL;
	FFile_pAsyncErrorResumeCallback = NULL;

	FFile_nMaxSimultaneousOpen = Fang_ConfigDefs.nFile_MaxSimultaneousFilesOpen;
	FFile_paFileCookie = (CFPlatformFileCookie *)fres_AlignedAlloc( FFile_nMaxSimultaneousOpen * sizeof( CFPlatformFileCookie ), 4 );
	if ( !FFile_paFileCookie )
	{
		DEVPRINTF( "ffile_ModuleStartup() - ERROR - Unable to allocate file cookies for ffile system.\n" );
		return FALSE;
	}

	for ( i = 0; i < FFile_nMaxSimultaneousOpen; i++ ) 
	{
		FFile_paFileCookie[i].m_hFangHandle = i;
		FFile_paFileCookie[i].m_bUsedEntry = FALSE;
		FFile_paFileCookie[i].m_nMasterPos = 0;
		FFile_paFileCookie[i].m_pMasterDirEntry = NULL;
	}

	FFile_nLastMFTokenRead = 0;
	FFile_nLastMFTokenUsed = 0;
	FFile_paMFTokenList = (QueuedReadToken *)fres_Alloc( sizeof( QueuedReadToken ) * FFILE_MAX_MF_READ_TOKENS );
	if ( !FFile_paMFTokenList )
	{
		return FALSE;
	}
    fang_MemZero( (void *)FFile_paMFTokenList, sizeof( QueuedReadToken ) * FFILE_MAX_MF_READ_TOKENS );

	_bModuleInitialized = TRUE;

#if FANG_PLATFORM_GC
	if ( !fgcfile_ModuleStartup() )
	{
		return FALSE;
	}
#elif FANG_PLATFORM_DX
	if ( !fdx8file_ModuleStartup() )
	{
		return FALSE;
	}
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	if ( !fps2file_ModuleStartup() )
	{
		return FALSE;
	}
//ARG - <<<<<
#endif

	// init the file log stuff, default = OFF
	ffile_LogSetFilename( NULL );

	// setup either master mode or directory mode
	_bMasterMode = FALSE;
	ffile_SetupFileMode();
	
#if _ENABLE_TIMING_LOADS
	FFile_fSecondsInModule = 0.0f;
#endif
	
	// Localization variables
	FFile_cLanguageChar = 0;
	FFile_cAudioLanguageChar = 0;
	FFile_cReferenceLanguageChar = 0;

	return TRUE;
}


//
//
//
void ffile_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	_ResetMasterModeVars();

#if FANG_PLATFORM_GC
	fgcfile_ModuleShutdown();
#elif FANG_PLATFORM_DX
	fdx8file_ModuleShutdown();
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	fps2file_ModuleShutdown();
//ARG - <<<<<
#endif

	FFile_paFileCookie = NULL;
	FFile_paMFTokenList = NULL;

	_bModuleInitialized = FALSE;
}


//
//
//
BOOL ffile_IsInitialized( void ) 
{
	return _bModuleInitialized;
}


//
//
//
BOOL ffile_IsInMasterMode( void ) 
{
	FASSERT( _bModuleInitialized );

	return _bMasterMode;
}


//
// Sets up ffile into either master file mode or directory mode.
// This function keys off of the Fang_ConfigDefs.pszFile_MasterFilePathName field
// and is automatically called on ModuleStartup, but can be called again if 
// a different master file is to be used.
// Care should be taken to ensure that nothing is being loaded before calling this function.
void ffile_SetupFileMode( void ) 
{
	#if _ENABLE_TIMING_LOADS
		CFFileTimer FFileTimer;
	#endif

	FASSERT( _bModuleInitialized );

	_ResetMasterModeVars();

	if ( Fang_ConfigDefs.pszFile_MasterFilePathName ) {
		// Master file specified...
		FFile_hMasterFile = ffile_OSDir_Open( Fang_ConfigDefs.pszFile_MasterFilePathName, FFILE_OPEN_RONLY_RANDOM );

		if ( !FFILE_IS_VALID_HANDLE( FFile_hMasterFile ) ) {
			DEVPRINTF( "ffile_SetupFileMode(): Could not open master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
			DEVPRINTF( "                       Switching to Directory Mode.\n" );
		}
	} 
	else 
	{
		// Master file not specified...
		FFile_hMasterFile = FFILE_INVALID_HANDLE;
	}

	_bMasterMode = FFILE_IS_VALID_HANDLE( FFile_hMasterFile );

	if ( _bMasterMode ) 
	{
		if ( !_ReadMasterDir() )
		{
			// Trouble with master file...
			ffile_OSDir_Close( &FFile_paFileCookie[FFile_hMasterFile] );
			FFile_hMasterFile = FFILE_INVALID_HANDLE;
			_bMasterMode = FALSE;
			_nMasterDirEntries = 0;
			_pMasterDir = NULL;
		} 
		else 
		{
//			DEVPRINTF( "Using master file '%s' (%u dir entries).\n", Fang_ConfigDefs.pszFile_MasterFilePathName, _nMasterDirEntries );
		}
	}

	FFile_hAsyncMasterFile = ffile_OSDir_Open( Fang_ConfigDefs.pszFile_MasterFilePathName, FFILE_OPEN_RONLY_RANDOM, TRUE );
}


//
//
//
void ffile_ResetTimer( void ) 
{
	FASSERT( _bModuleInitialized );
	
#if _ENABLE_TIMING_LOADS
	FFile_fSecondsInModule = 0.0f;
#endif
}


//
//
//
f32 ffile_GetTime( void ) 
{
	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
 	return FFile_fSecondsInModule;
#else
	return 0.0f;
#endif
}


//
//
//
BOOL ffile_LogSetFilename( cchar *pszFilename ) 
{
#if FANG_PLATFORM_WIN
	char szBuffer[ 256 ];
	time_t Time;
	struct tm *pNewTime;

	// default to system off
	_bLogSystemOK = FALSE;
	_bLogFiles = FALSE;

	if ( !pszFilename ) 
	{
		return FALSE;
	}
	FFileHandle hFile = ffile_OSDir_Open( pszFilename, FFILE_OPEN_TRUNC_OR_CREATE_WONLY );
	if ( FFILE_IS_VALID_HANDLE( hFile ) == FALSE ) 
	{
		return FALSE;
	}
	// write a few lines out to the new file
	time( &Time );
	pNewTime = localtime( &Time );
	sprintf( szBuffer, "#FANG***DO NOT REMOVE THIS LINE***\r\n# Resource Log File, created %.24s\r\n# Used to optimize load times\r\n", asctime( pNewTime ) );

	if ( ffile_OSDir_Write( &FFile_paFileCookie[hFile], fclib_strlen( szBuffer ), szBuffer ) <= 0 ) 
	{
		ffile_OSDir_Close( &FFile_paFileCookie[hFile] );
		return FALSE;
	}
	ffile_OSDir_Close( &FFile_paFileCookie[hFile] );

	fclib_strcpy( _szLogFilename, pszFilename );
	_bLogSystemOK = TRUE;
#else
	pszFilename;
#endif
	return TRUE;  
}


//
//
//
void ffile_LogStart( cchar *pszLevelName ) 
{
#if FANG_PLATFORM_WIN
	FASSERT( _bLogSystemOK );

	_bLogFiles = TRUE;

	// write out the level name
	if( _bLogSystemOK && _bLogFiles ) 
	{
		char szBuffer[ 256 ];

		FFileHandle hFile = ffile_OSDir_Open( _szLogFilename, FFILE_OPEN_WONLY );
		if( FFILE_IS_VALID_HANDLE( hFile ) == FALSE ) 
		{
			return;
		}
		ffile_OSDir_Seek( &FFile_paFileCookie[hFile], 0, FFILE_SEEK_END );
		sprintf( szBuffer, "\r\n# %s\r\n", pszLevelName );
		ffile_OSDir_Write( &FFile_paFileCookie[hFile], fclib_strlen( szBuffer ), szBuffer );
		ffile_OSDir_Close( &FFile_paFileCookie[hFile] );		
	}
#else
	pszLevelName;
#endif
}


void ffile_AddToLogFile( cchar *pszFilename ) {
#if FANG_PLATFORM_WIN
	if( _bLogSystemOK && _bLogFiles ) {
		char szBuffer[ 256 ];

		FFileHandle hFile = ffile_OSDir_Open( _szLogFilename, FFILE_OPEN_WONLY );
		if( FFILE_IS_VALID_HANDLE( hFile ) == FALSE ) {
			return;
		}
		ffile_OSDir_Seek( &FFile_paFileCookie[hFile], 0, FFILE_SEEK_END );
		cchar *pszEntryName = _FindStartOfRawFileName( pszFilename );

		sprintf( szBuffer, "%s\r\n", pszEntryName );
		fclib_strlwr( szBuffer );
		ffile_OSDir_Write( &FFile_paFileCookie[hFile], fclib_strlen( szBuffer ), szBuffer );
		ffile_OSDir_Close( &FFile_paFileCookie[hFile] );		
	}
#else
	pszFilename;
#endif
}

//
//
//
void ffile_LogStop( void ) 
{
#if FANG_PLATFORM_WIN
	FASSERT( _bLogSystemOK );

	_bLogFiles = FALSE;
#endif
}


//
//
//
FFileHandle ffile_Open( cchar *pszFName, FFileOpenType_e nOpenMode, BOOL bBypassMasterFile ) 
{
	s32 i, nDirIndex;

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	FASSERT( _bModuleInitialized );

	ffile_AddToLogFile( pszFName );

	// If we found a master file and the caller hasn't requested bypass, look in the master file
	if ( _bMasterMode && !bBypassMasterFile ) 
	{
		// Master File mode...
		if ( nOpenMode != FFILE_OPEN_RONLY 
			&& nOpenMode != FFILE_OPEN_RONLY_SEQUENTIAL 
			&& nOpenMode != FFILE_OPEN_RONLY_RANDOM )
		{
			// Only reading is supported from a master file...
			return FFILE_INVALID_HANDLE;
		}

		cchar *pszRawFileName = _FindStartOfRawFileName( pszFName );
		if ( fclib_strlen( pszRawFileName ) >= FDATA_PRJFILE_FILENAME_LEN ) 
		{
			DEVPRINTF( "ffile_Open(): The filename '%s' is too long, max filename = %d.\n", pszRawFileName, (FDATA_PRJFILE_FILENAME_LEN-1) );
			return FFILE_INVALID_HANDLE;
		}

		for ( i = 0; i < (s32)FFile_nMaxSimultaneousOpen; i++ ) 
		{
			if ( FFile_paFileCookie[i].m_bUsedEntry == FALSE ) 
			{
				// Found unused cookie...
				nDirIndex = _GetDirIndexOfFullPathName( pszFName );
				if ( nDirIndex == -1 ) 
				{
					// File not found...
					return FFILE_INVALID_HANDLE;
				}

				// Found file...

				FFile_paFileCookie[i].m_bUsedEntry = TRUE;
				FFile_paFileCookie[i].m_nMasterPos = 0;
				FFile_paFileCookie[i].m_pMasterDirEntry = &_pMasterDir[nDirIndex];

				return (FFileHandle)i;
			}
		}

		// No cookies left in the jar....
		DEVPRINTF( "ffile_open() - ERROR - Exceeded maximum open files.\n" );
		return FFILE_INVALID_HANDLE;
	} 
	else 
	{
		FFileHandle nReturn = ffile_OSDir_Open( pszFName, nOpenMode );
				
		if ( !FFILE_IS_VALID_HANDLE(nReturn) )
		{
			return FFILE_ERROR_RET;
		}

		return (FFileHandle)nReturn;
	}
}


//
//
//
s32 ffile_Close( FFileHandle hFile ) 
{
	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	// Do we have a valid file handle?
	if ( !FFILE_IS_VALID_HANDLE( hFile ) ) 
	{
		return FFILE_ERROR_RET;
	}
	
	// Is the file already closed?
	if ( FFile_paFileCookie[hFile].m_bUsedEntry == FALSE )
	{
		return 0;
	}
	
	// If this is in the master file, no need to close	
	if ( FFile_paFileCookie[hFile].m_pMasterDirEntry ) 
	{
		// Flag the cookie as unused
		FFile_paFileCookie[hFile].m_bUsedEntry = FALSE;
		
		return 0;
	} 
	else 
	{
		// Directory mode...
		s32 nReturnValue = ffile_OSDir_Close( &FFile_paFileCookie[hFile] );
		
		// Flag the cookie as unused
		FFile_paFileCookie[hFile].m_bUsedEntry = FALSE;
		
		return nReturnValue;
	}
}


//
//
//
s32 ffile_GetFileSize( FFileHandle hFile ) 
{
	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	// Do we have a valid file handle?
	if ( !FFILE_IS_VALID_HANDLE( hFile ) ) 
	{
		return FFILE_ERROR_RET;
	}
	
	if ( FFile_paFileCookie[hFile].m_pMasterDirEntry ) 
	{
		return FFile_paFileCookie[hFile].m_pMasterDirEntry->nNumBytes;
	} 
	else 
	{
		// Directory mode...
		return ffile_OSDir_GetFileSize( &FFile_paFileCookie[hFile] );
	}
}


//
//
//
s32 ffile_Seek( FFileHandle hFile, s32 nFileOffset, FFileSeekType_e nSeekMode ) 
{
	const FFileMasterEntry_t *pDirEntry;

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	FASSERT( _bModuleInitialized );

	// Do we have a valid file handle?
	if ( !FFILE_IS_VALID_HANDLE( hFile ) ) 
	{
		return FFILE_ERROR_RET;
	}

	if ( FFile_paFileCookie[hFile].m_pMasterDirEntry ) 
	{
		// Master mode...
		pDirEntry = FFile_paFileCookie[hFile].m_pMasterDirEntry;

		switch( nSeekMode ) 
		{
			case FFILE_SEEK_SET:
				break;

			case FFILE_SEEK_CUR:
				nFileOffset = (s32)FFile_paFileCookie[hFile].m_nMasterPos + nFileOffset;
				break;

			case FFILE_SEEK_END:
				nFileOffset = (s32)pDirEntry->nNumBytes + nFileOffset;
				break;

			default:
				FASSERT_NOW;
				return FFILE_ERROR_RET;
		}

		if ( nFileOffset < 0 || (u32)nFileOffset>pDirEntry->nNumBytes ) 
		{
			// Can't set past the end of a file...
			return FFILE_ERROR_RET;
		}

		// Set new file position...
		FFile_paFileCookie[hFile].m_nMasterPos = nFileOffset;
		return 0;
	} 
	else 
	{
		// Directory mode...
		s32 nReturn = ffile_OSDir_Seek( &FFile_paFileCookie[hFile], nFileOffset, nSeekMode );
		return nReturn;
	}
}


//
//
//
s32 ffile_Tell( FFileHandle hFile ) 
{
	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	// Do we have a valid file handle?
	if ( !FFILE_IS_VALID_HANDLE( hFile ) ) 
	{
		return FFILE_ERROR_RET;
	}

	if ( FFile_paFileCookie[hFile].m_pMasterDirEntry ) 
	{
		// File was opened in the master file...
		return (s32)FFile_paFileCookie[hFile].m_nMasterPos;
	} 
	else 
	{
		// Directory mode...
		return ffile_OSDir_Tell( &FFile_paFileCookie[hFile] );
	}
}


//
//
//
const FFileMasterEntry_t* ffile_MF_OpenStream( cchar *pszFName, u32 *pFileSizeOut )
{
	s32 nDirIndex;

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	FASSERT( _bModuleInitialized );

	ffile_AddToLogFile( pszFName );

	// If we found a master file and the caller hasn't requested bypass, look in the master file
	if ( !_bMasterMode )
	{
		return NULL;
	}
	
	cchar *pszRawFileName = _FindStartOfRawFileName( pszFName );
	if ( fclib_strlen( pszRawFileName ) >= FDATA_PRJFILE_FILENAME_LEN ) 
	{
		DEVPRINTF( "ffile_Open(): The filename '%s' is too long, max filename = %d.\n", pszRawFileName, (FDATA_PRJFILE_FILENAME_LEN-1) );
		return NULL;
	}

	nDirIndex = _GetDirIndexOfFullPathName( pszFName );
	if ( nDirIndex == -1 ) 
	{
		// File not found...
		return NULL;
	}

	// Found file...

	if ( pFileSizeOut )
	{
		(*pFileSizeOut) = _pMasterDir[nDirIndex].nNumBytes;
	}
	
	return &_pMasterDir[nDirIndex];
}


//
//
//
s32 ffile_MF_ReadStream( const FFileMasterEntry_t *pDirEntry, u32 uReadStart, u32 uReadAmt, void *pDestData, FFileAsyncReadCallback_t *pCallback, void *pUser )
{
	s32 nBytesRead;

	FASSERT( _bModuleInitialized && pDirEntry && pDestData && pCallback );
	FASSERT( uReadAmt != 0 );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	// Seek to the appropriate spot in the master file
	if ( ffile_OSDir_Seek( &FFile_paFileCookie[FFile_hAsyncMasterFile], pDirEntry->nStartOffset + uReadStart, FFILE_SEEK_SET ) < 0 ) 
	{
		// Trouble seeking...
		ffile_OSDir_Seek( &FFile_paFileCookie[FFile_hAsyncMasterFile], pDirEntry->nStartOffset, FFILE_SEEK_SET );
		return FFILE_ERROR_RET;
	}

	// Attempt to read the data requested from the master file
	if ( (nBytesRead = ffile_OSDir_Read( &FFile_paFileCookie[FFile_hAsyncMasterFile], uReadAmt, pDestData, pCallback, pUser )) < 0 ) 
	{
		// Error reading...
		ffile_OSDir_Seek( &FFile_paFileCookie[FFile_hAsyncMasterFile], pDirEntry->nStartOffset + uReadStart, FFILE_SEEK_SET );
		return FFILE_ERROR_RET;
	}

	return nBytesRead;
}


//
//
//
s32 ffile_Read( FFileHandle hFile, u32 uReadAmt, void *pDestData, FFileAsyncReadCallback_t *pCallback, void *pUser ) 
{
	const FFileMasterEntry_t *pDirEntry;
	s32 nBytesRead;

	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	// Do we have a valid file handle?
	if ( !FFILE_IS_VALID_HANDLE( hFile ) ) 
	{
		return FFILE_ERROR_RET;
	}

	if ( FFile_paFileCookie[hFile].m_pMasterDirEntry ) 
	{
		// This file was opened through the master file...
		pDirEntry = FFile_paFileCookie[hFile].m_pMasterDirEntry;

		// Seek to the appropriate spot in the master file
		if ( ffile_OSDir_Seek( &FFile_paFileCookie[FFile_hMasterFile], pDirEntry->nStartOffset + FFile_paFileCookie[hFile].m_nMasterPos, FFILE_SEEK_SET ) < 0 ) 
		{
			// Trouble seeking...
			ffile_OSDir_Seek( &FFile_paFileCookie[FFile_hMasterFile], pDirEntry->nStartOffset, FFILE_SEEK_SET );
			FFile_paFileCookie[hFile].m_nMasterPos = 0;
			return FFILE_ERROR_RET;
		}

		if ( (FFile_paFileCookie[hFile].m_nMasterPos + uReadAmt) > pDirEntry->nNumBytes ) 
		{
			uReadAmt = pDirEntry->nNumBytes - FFile_paFileCookie[hFile].m_nMasterPos;
		}

		// Attempt to read the data requested from the master file
		if ( (nBytesRead = ffile_OSDir_Read( &FFile_paFileCookie[FFile_hMasterFile], uReadAmt, pDestData, pCallback, pUser )) < 0 ) 
		{
			// Error reading...
			ffile_OSDir_Seek( &FFile_paFileCookie[FFile_hMasterFile], pDirEntry->nStartOffset + FFile_paFileCookie[hFile].m_nMasterPos, FFILE_SEEK_SET );
			return FFILE_ERROR_RET;
		}

		// Success reading...
		FFile_paFileCookie[hFile].m_nMasterPos += nBytesRead;
		return nBytesRead;
	} 
	else 
	{
		// Directory mode...
		s32 nReturn = ffile_OSDir_Read( &FFile_paFileCookie[hFile], uReadAmt, pDestData, pCallback );
		return nReturn;
	}
}


//
//
//
s32 ffile_Write( FFileHandle hFile, u32 uWriteAmt, void *pSrcData ) 
{
	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	// Do we have a valid file handle?
	if ( !FFILE_IS_VALID_HANDLE( hFile ) ) 
	{
		return FFILE_ERROR_RET;
	}

	if ( FFile_paFileCookie[hFile].m_pMasterDirEntry ) 
	{
		// File was opened from the master file...
		
		// Not allowed to write in master mode...
		FASSERT_NOW;
		return FFILE_ERROR_RET;
	} 
	else 
	{
		// Directory mode...
		return ffile_OSDir_Write( &FFile_paFileCookie[hFile], uWriteAmt, pSrcData );
	}
}


//
//
//
s32 ffile_EOF( FFileHandle hFile ) 
{
	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	// Do we have a valid file handle?
	if ( !FFILE_IS_VALID_HANDLE( hFile ) ) 
	{
		// Return not EOF...
		return 0;
	}

	if ( FFile_paFileCookie[hFile].m_pMasterDirEntry ) 
	{
		// File was opened from the master file...
		return (FFile_paFileCookie[hFile].m_nMasterPos >= FFile_paFileCookie[hFile].m_pMasterDirEntry->nNumBytes);
	} 
	else 
	{
		// Directory mode...
		return ffile_OSDir_EOF( &FFile_paFileCookie[hFile] );
	}
}


//
//
//
s32 ffile_Scanf( FFileHandle hFile, const char *szFmt, ... ) 
{
	FASSERT( _bModuleInitialized );

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	char *pCurrFmt, cFmt, szNewFmt[16], szConversion[16], szBuffer[64], *pBuffer, *pszConversion;
	int *pInt, nCount, nFmt, nBytesRead, nUsefulBytes;
	s32 nCurr;
	f32 *pFloat;
	BOOL bFormatComplete;

	va_list vaArgList;
	va_start ( vaArgList, szFmt );

	pBuffer = szBuffer;
	nCount = 0;
	nFmt = 0;
	nBytesRead = 0;
	nUsefulBytes = 0;

	nBytesRead = ffile_Read( hFile, 64, szBuffer );

	for ( pCurrFmt = (char *)szFmt; *pCurrFmt; ) 
	{
		// Build the separated format string
		bFormatComplete = FALSE;
		nFmt = 0;
		while (*pCurrFmt != '\0' && !( bFormatComplete && *pCurrFmt == '%' ) ) 
		{
			szNewFmt[ nFmt++ ] = *pCurrFmt;
			if ( *pCurrFmt == '%' ) 
			{
				bFormatComplete = TRUE;
				cFmt = *(pCurrFmt + 1);
			}
			pCurrFmt++;
		}		
		szNewFmt[ nFmt ] = '\0';

		// Skip the white space.
		while( *pBuffer == '\r' || *pBuffer == '\n' || *pBuffer == ' ' ) pBuffer++;

		switch( cFmt ) 
		{
			case 'd':
				pInt = va_arg( vaArgList, int * );
				if ( sscanf( pBuffer, szNewFmt, pInt) ) {
					sprintf( szConversion, szNewFmt, *pInt );
					pBuffer += fclib_strlen( szConversion ); 
					nFmt = 0;
					nCount++;
				}				
				break;

			case 'f':
				pFloat = va_arg( vaArgList, f32 * );
				if ( sscanf( pBuffer, szNewFmt, pFloat) ) {
					sprintf( szConversion, szNewFmt, *pFloat );
					pBuffer += fclib_strlen( szConversion ); 
					nFmt = 0;
					nCount++;
				}				
				break;

			case 'c':
				if ( sscanf( pBuffer, szNewFmt, va_arg( vaArgList, char *) ) ) {
					nFmt = 0;
					nCount++;
					pBuffer += sizeof( char );
				}				
				break;

			case 's':
				pszConversion = va_arg( vaArgList, char * );
				if ( sscanf( pBuffer, szNewFmt, pszConversion) ) {
					pBuffer += fclib_strlen( pszConversion );
					nFmt = 0;
					nCount++;
				}
				break;
		}
	}

	nUsefulBytes = pBuffer - szBuffer + nFmt;
	nCurr = ffile_Tell( hFile );
	ffile_Seek( hFile, nCurr - nBytesRead + nUsefulBytes, FFILE_SEEK_SET );

	va_end( vaArgList );
	return (nCount);
}


//
//
//
s32 ffile_GetLanguageChars( char* pcLanguageChar, char* pcAudioLanguageChar )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pcLanguageChar );
	FASSERT( pcAudioLanguageChar );

	*pcLanguageChar = FFile_cLanguageChar;
	*pcAudioLanguageChar = FFile_cAudioLanguageChar;

	return 0; // SUCCESS
}

//
//
//
s32 ffile_SetLanguageChars( char cReferenceChar, char cLanguageChar, char cAudioLanguageChar )
{
	FASSERT( _bModuleInitialized );

	FASSERT( cReferenceChar );
	FASSERT( cLanguageChar );
	FASSERT( cAudioLanguageChar );
	if( !cReferenceChar || !cLanguageChar || !cAudioLanguageChar )
	{
		return -1; // FAILURE!
	}

	FFile_cReferenceLanguageChar = cReferenceChar;
	FFile_cLanguageChar = cLanguageChar;
	FFile_cAudioLanguageChar = cAudioLanguageChar;

	return 0; // SUCCESS
}


s32 ffile_LocalizeFilename( char* pszFilenameToLocalize, BOOL bConvertFilenameToLocalizedVersion ) 
{

	s32 nRetVal = 0; // Not a localized file
	if( FFile_cReferenceLanguageChar && FFile_cLanguageChar && ( FFile_cReferenceLanguageChar != FFile_cLanguageChar ) ) {
		char* pReferenceLanguageChar = fclib_strchr( pszFilenameToLocalize, FFile_cReferenceLanguageChar );
		if( pReferenceLanguageChar ) {
			if( bConvertFilenameToLocalizedVersion ) {
				*pReferenceLanguageChar = FFile_cLanguageChar;
			}
			nRetVal = 1; // This is a localized file
		}
	}

	return nRetVal; // SUCCESS
}


s32 ffile_SetAsyncReadErrorHaltCallback( FFileAsyncReadErrorCallback_t *pCallback )
{
	FFile_pAsyncErrorHaltCallback = pCallback;
	return 0;
}


s32 ffile_SetAsyncReadErrorResumeCallback( FFileAsyncReadErrorCallback_t *pCallback )
{
	FFile_pAsyncErrorResumeCallback = pCallback;
	return 0;
}


/*
//
//
//
s32 ffile_OSDir_Scanf( FFileHandle hFile, const char *szFmt, ... ) 
{
	FASSERT( _bModuleInitialized );

	char *pCurrFmt, cFmt, szNewFmt[16], szConversion[16], szBuffer[64], *pBuffer, *pszConversion;
	int *pInt, nCount, nFmt, nBytesRead, nUsefulBytes;
	s32 nCurr;
	f32 *pFloat;
	BOOL bFormatComplete;

	va_list vaArgList;
	va_start ( vaArgList, szFmt );

	pBuffer = szBuffer;
	nCount = 0;
	nFmt = 0;
	nBytesRead = 0;
	nUsefulBytes = 0;

	nBytesRead = ffile_OSDir_Read( &FFile_paFileCookie[hFile], 64, szBuffer );

	for ( pCurrFmt = (char *)szFmt; *pCurrFmt; ) 
	{
		// Build the separated format string
		bFormatComplete = FALSE;
		nFmt = 0;
		while (*pCurrFmt != '\0' && !( bFormatComplete && *pCurrFmt == '%' ) ) 
		{
			szNewFmt[ nFmt++ ] = *pCurrFmt;
			if ( *pCurrFmt == '%' ) 
			{
				bFormatComplete = TRUE;
				cFmt = *(pCurrFmt + 1);
			}
			pCurrFmt++;
		}		
		szNewFmt[ nFmt ] = '\0';

		// Skip the white space.
		while( *pBuffer == '\r' || *pBuffer == '\n' || *pBuffer == ' ' ) 
		{
			pBuffer++;
		}

		switch( cFmt ) 
		{
			case 'd':
				pInt = va_arg( vaArgList, int * );
				if ( sscanf( pBuffer, szNewFmt, pInt) ) 
				{
					sprintf( szConversion, szNewFmt, *pInt );
					pBuffer += fclib_strlen( szConversion ); 
					nFmt = 0;
					nCount++;
				}				
				break;
			case 'f':
				pFloat = va_arg( vaArgList, f32 * );
				if ( sscanf( pBuffer, szNewFmt, pFloat) ) 
				{
					sprintf( szConversion, szNewFmt, *pFloat );
					pBuffer += fclib_strlen( szConversion ); 
					nFmt = 0;
					nCount++;
				}				
				break;
			case 'c':
				if ( sscanf( pBuffer, szNewFmt, va_arg( vaArgList, char *) ) ) 
				{
					nFmt = 0;
					nCount++;
					pBuffer += sizeof( char );
				}				
				break;
			case 's':
				pszConversion = va_arg( vaArgList, char * );
				if ( sscanf( pBuffer, szNewFmt, pszConversion) ) 
				{
					pBuffer += fclib_strlen( pszConversion );
					nFmt = 0;
					nCount++;
				}
				break;
		}
	}

	nUsefulBytes = pBuffer - szBuffer + nFmt;
	nCurr = ffile_OSDir_Tell( &FFile_paFileCookie[hFile] );
	ffile_OSDir_Seek( &FFile_paFileCookie[hFile], nCurr - nBytesRead + nUsefulBytes, FFILE_SEEK_SET );

	va_end( vaArgList );
	return (nCount);
}
*/
//==================
// private functions

static BOOL _ReadMasterDir( void ) {
	FDataPrjFile_Header_t Header;

#if _ENABLE_TIMING_LOADS
	CFFileTimer FFileTimer;
#endif

	///////////////////////////////////////
	// CHECK THE MASTER FILE VERSION NUMBER
	if( ffile_OSDir_Read( &FFile_paFileCookie[FFile_hMasterFile], sizeof(FDataPrjFile_Header_t), &Header ) < 0 ) {
		// Trouble reading header...
		DEVPRINTF( "ffile _ReadMasterDir(): Could not read header from master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                        Switching to Directory Mode.\n" );
		return FALSE;
	}

	//////////////////////////////////
	// CHECK THE MASTER FILE SIGNATURE
	if( Header.Version.nSignature != FVERSION_FILE_SIGNATURE ) {
		// Signatures don't match...
		DEVPRINTF( "ffile _ReadMasterDir(): Unrecognized signature in master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                        Switching to Directory Mode.\n" );
		return FALSE;
	}

	///////////////////////////////////////////////
	// CHECK THE VARIOUS FILE TYPE VERSION NUMBERS
	u32 nTgaVersion, nApeVersion; 
#if (FANG_PLATFORM_WIN | FANG_PLATFORM_XB)
	if( Header.Version.nVersion != FDATA_PRJFILE_XB_VERSION ) {
		// Incorrect version...
		DEVPRINTF( "ffile _ReadMasterDir(): Version mismatch in master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                        Switching to Directory Mode.\n" );
		return FALSE;
	}
	nTgaVersion = FDATA_PRJFILE_XB_TGA_VERSION;
	nApeVersion = FDATA_PRJFILE_XB_APE_VERSION;
#elif FANG_PLATFORM_PS2
	if( Header.Version.nVersion != FDATA_PRJFILE_PS_VERSION ) {
		// Incorrect version...
		DEVPRINTF( "ffile _ReadMasterDir(): Version mismatch in master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                        Switching to Directory Mode.\n" );
		return FALSE;
	}
	nTgaVersion = FDATA_PRJFILE_PS_TGA_VERSION;
	nApeVersion = FDATA_PRJFILE_PS_APE_VERSION;
#elif FANG_PLATFORM_GC
	if( Header.Version.nVersion != FDATA_PRJFILE_GC_VERSION ) 
	{
		// Incorrect version...
		DEVPRINTF( "ffile_ModuleStartup(): Version mismatch in master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                       Switching to Directory Mode.\n" );
		return FALSE;
	}
	nTgaVersion = FDATA_PRJFILE_GC_TGA_VERSION;
	nApeVersion = FDATA_PRJFILE_GC_APE_VERSION;
#else
	// unknown platform
	DEVPRINTF( "ffile _ReadMasterDir(): Unknown platform trying to use master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
	DEVPRINTF( "                        Switching to Directory Mode.\n" );
	return FALSE;
#endif

	// make sure that the master file's file type compiler versions jive with the runtime version numbers
	if( Header.nFntCompilerVersion != FDATA_PRJFILE_FNT_VERSION ||
		Header.nCsvCompilerVersion != FDATA_PRJFILE_CSV_VERSION ||
		Header.nApeCompilerVersion != nApeVersion ||
		Header.nTgaCompilerVersion != nTgaVersion ||
		Header.nMtxCompilerVersion != FDATA_PRJFILE_MTX_VERSION ||
		Header.nSmaCompilerVersion != FDATA_PRJFILE_SMA_VERSION ||
		Header.nGtCompilerVersion != FDATA_PRJFILE_GT_VERSION ||
		Header.nWvbCompilerVersion != FDATA_PRJFILE_WVB_VERSION ||
		Header.nFprCompilerVersion != FDATA_PRJFILE_FPR_VERSION ||
		Header.nCamCompilerVersion != FDATA_PRJFILE_CAM_VERSION ) {
		// One or more file type compiler versions don't match...
		DEVPRINTF( "ffile_ModuleStartup(): One or more file type compiler version mismatchs in '%s', please re-PASM.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                       Switching to Directory Mode.\n" );
		return FALSE;
	}

	// Compute dir size...
	_nMasterDirEntries = Header.nNumEntries;

	/////////////////////
	// GRAB AN FMEM FRAME
	FMemFrame_t hMemFrame = fmem_GetFrame();

//ARG - >>>>>
//ARG - initialization crosses 'goto' scope
	u32 nMasterDirBytes;
	u32 i;
	FFileMasterEntry_t *pDest;
//ARG - <<<<<
	
	u32 nOrigMasterDirBytes = _nMasterDirEntries * sizeof(FDataPrjFile_Entry_t);
	FDataPrjFile_Entry_t *pEntries = (FDataPrjFile_Entry_t *)fmem_Alloc( nOrigMasterDirBytes, 4 );
	if( !pEntries ) {
		// Not enough memory...
		DEVPRINTF( "ffile _ReadMasterDir(): Not enough memory to hold directory of master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                        (%u directory entries, %u bytes).\n", _nMasterDirEntries, nOrigMasterDirBytes );
		DEVPRINTF( "                        Switching to Directory Mode.\n" );
		goto _ExitWithError;
	}

    // Read in the directory...
	if( ffile_OSDir_Read( &FFile_paFileCookie[FFile_hMasterFile], nOrigMasterDirBytes, (void *)pEntries ) < 0 ) {
		// Trouble reading directory...
		DEVPRINTF( "ffile _ReadMasterDir(): Trouble reading directory of master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                        Switching to Directory Mode.\n" );
		goto _ExitWithError;
	}
	// Allocate a smaller chunk of memory to hold the permanent directory structure...
	nMasterDirBytes = _nMasterDirEntries * sizeof(FFileMasterEntry_t);
	_pMasterDir = (const FFileMasterEntry_t *)fres_AlignedAlloc( nMasterDirBytes, 4 );
	if( !_pMasterDir ) {
		DEVPRINTF( "ffile _ReadMasterDir(): Not enough memory to hold directory of master file '%s'.\n", Fang_ConfigDefs.pszFile_MasterFilePathName );
		DEVPRINTF( "                        (%u directory entries, %u bytes).\n", _nMasterDirEntries, nMasterDirBytes );
		DEVPRINTF( "                        Switching to Directory Mode.\n" );
		goto _ExitWithError;
	}
	// copy each masterfile entry to its new smaller structure...
	pDest = (FFileMasterEntry_t *)_pMasterDir;
	for( i=0; i < _nMasterDirEntries; i++ ) {
		pDest->nNameCRC = fmath_Crc32( 0, (const u8 *)pEntries[i].szFilename, FDATA_PRJFILE_FILENAME_LEN );
		pDest->nStartOffset = pEntries[i].nStartOffset;
		pDest->nNumBytes = pEntries[i].nNumBytes;
		pDest++;
	}
	// qsort the entries by the new crc name
	fclib_QSort( (void *)_pMasterDir, _nMasterDirEntries, sizeof( FFileMasterEntry_t ), _QSortCompareFcn );

	// search for duplicate CRC name values, if so don't proceed
	for( i=0; i < (_nMasterDirEntries-1); i++ ) {
		if( _pMasterDir[i].nNameCRC == _pMasterDir[i+1].nNameCRC ) {
			// 2 master file entries CRCed to the same value, what the fuck
			FASSERT_NOW;
			DEVPRINTF( "ffile _ReadMasterDir(): At least 2 master file entries CRCed to the same value, this system doesn't work.\n" );
			goto _ExitWithError;
		}
	}

	// release our fmem frame so that the copy of the original, larger master file entries, are freed
	fmem_ReleaseFrame( hMemFrame );

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( hMemFrame );
	return FALSE;	
}

static void _ResetMasterModeVars( void ) {
	
	FASSERT( _bModuleInitialized );

	if( _bMasterMode ) {
		// Close master file...
		ffile_OSDir_Close( &FFile_paFileCookie[FFile_hMasterFile] );
		_pMasterDir = NULL;
		FFile_hMasterFile = FFILE_INVALID_HANDLE;
		_bMasterMode = FALSE;
	}
}

// return values:
// < 0 elem1 less than elem2 
// 0 elem1 equivalent to elem2 
// > 0 elem1 greater than elem2 
static int _QSortCompareFcn( const void *arg1, const void *arg2 ) {
	FFileMasterEntry_t *p1, *p2;

	p1 = (FFileMasterEntry_t *)arg1;
	p2 = (FFileMasterEntry_t *)arg2;

   // Compare all of both strings:
	if( p1->nNameCRC < p2->nNameCRC ) {
		return -1;
	} else if( p1->nNameCRC == p2->nNameCRC ) {
		return 0;
	}
	return 1;
}

static s32 _GetDirIndexOfRawFileName( cchar *pszRawName ) {
	FFileMasterEntry_t SearchFor, *pFoundEntry;
	char szFilename[FDATA_PRJFILE_FILENAME_LEN];

	// mike's bsearch 
	if( pszRawName==NULL || pszRawName[0]==0 ) {
		// Invalid name...
		return -1;
	}

	// copy the search string, make it lowercase, and then fill in the search for entry
	fclib_strncpy( szFilename, (cchar *)pszRawName, FDATA_PRJFILE_FILENAME_LEN );
	fclib_strlwr( szFilename );

	// Modify the filename to a localized filename (if need be)
	ffile_LocalizeFilename( szFilename );

	SearchFor.nNameCRC = fmath_Crc32( 0, (const u8 *)szFilename, FDATA_PRJFILE_FILENAME_LEN );
	SearchFor.nStartOffset = 0;
	SearchFor.nNumBytes = 0;
	
	// start a binary search for SearchFor
	pFoundEntry = (FFileMasterEntry_t *)bsearch( &SearchFor,
												   _pMasterDir,
												   _nMasterDirEntries,
												   sizeof( FFileMasterEntry_t ),
												   _QSortCompareFcn );
	if( pFoundEntry ) {
		return (pFoundEntry - _pMasterDir);
	}

	// File not found...
	return -1;
}

static cchar *_FindStartOfRawFileName( cchar *pszPathName ) 
{
	cchar *pszRawName;
	u32 i, nPathLen;

	nPathLen = fclib_strlen( pszPathName );
	pszRawName = pszPathName + nPathLen;

	for( i=0; i<nPathLen; i++ ) {
		pszRawName--;

		if( *pszRawName==':' || *pszRawName=='\\' || *pszRawName=='/' ) {
			return pszRawName + 1;
		}
	}

	return pszRawName;
}

static s32 _GetDirIndexOfFullPathName( cchar *pszPathName ) {
	cchar *pszRawName;

	pszRawName = _FindStartOfRawFileName( pszPathName );
	return _GetDirIndexOfRawFileName( pszRawName );
}
