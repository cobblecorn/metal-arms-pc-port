//////////////////////////////////////////////////////////////////////////////////////
// ffile.h - Fang file module.
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

#ifndef _FFILE_H_
#define _FFILE_H_

#include "fang.h"


#define FFILE_MAX_SIMULTANEOUS_ASYNC_READS		64
#define FFILE_INVALID_HANDLE					( (FFileHandle)-1 )
#define FFILE_ERROR_RET							( -1 )

// Defines the maximum number of queued read requests before blocking occurs
#define FFILE_MAX_MF_READ_TOKENS				32


typedef enum {							// (Note: There is no 'text' file mode. Caller must handle CR/LF itself)
	FFILE_OPEN_RONLY_SEQUENTIAL,		// Will fail if the file does not exist
	FFILE_OPEN_RONLY_RANDOM,			// Will fail if the file does not exist
	FFILE_OPEN_RONLY,					// Will fail if the file does not exist
	FFILE_OPEN_WONLY,					// Will fail if the file does not exist
	FFILE_OPEN_TRUNC_OR_CREATE_WONLY,	// Will always create a new file

	FFILE_OPEN_COUNT
} FFileOpenType_e;

typedef enum {
	FFILE_ASYNC_READ_ERROR_DVD_COVER_OPEN,		// These five are used with the halt callback is invoked.
	FFILE_ASYNC_READ_ERROR_DVD_NO_DISK,
	FFILE_ASYNC_READ_ERROR_DVD_WRONG_DISK,
	FFILE_ASYNC_READ_ERROR_DVD_RETRY,
	FFILE_ASYNC_READ_ERROR_DVD_FATAL_ERROR,
	FFILE_ASYNC_READ_ERROR_NONE					// Used when the resume callback is invoked.
} FFileAsyncReadErrorCode_e;


// The ffile_Seek function moves the file pointer (if any) associated with stream to a new location that is 
//   offset bytes from origin. The next operation on the stream takes place at the new location. On a stream
//   open for update, the next operation can be either a read or a write. The argument origin must be one of
//   the following constants:

// FFILE_SEEK_CUR : Current position of file pointer

// FFILE_SEEK_END : End of file

// FFILE_SEEK_SET : Beginning of file



typedef enum {			// Don't change the order of these. They match the OS definitions!
	FFILE_SEEK_SET,
	FFILE_SEEK_CUR,
	FFILE_SEEK_END,

	FFILE_SEEK_COUNT
} FFileSeekType_e;


//
//	File Read status returns
//
enum {
	FFILE_STATUS_IDLE,
	FFILE_STATUS_ASYNC_READ_PENDING,

	FFILE_STATUS_COUNT
};


// A valid fang file handle is an index into FFile_paFileCookie array
typedef s32 FFileHandle;

typedef struct {
	u32 nNameCRC;			// The CRC of the lowercase name, with extension
	u32 nStartOffset;		// Offset to the first byte of data for this entry (from the beginning of the file)
	u32 nNumBytes;			// Total number of bytes in this entry
} FFileMasterEntry_t;

//
//	This is the base file cookie structure.  The platform file cookie
//	will inherit this base and add additional members on as it needs.
//
class CFFileCookie
{
	public:
		BOOL 		m_bUsedEntry;		// This entry is used
		FFileHandle m_hFangHandle;		// The fang file handle for the cookie (it's index in the cookie array)
		u32  		m_nMasterPos;		// Master Mode Only: Current position within master file
		const FFileMasterEntry_t  *m_pMasterDirEntry;	// Pointer to the directory entry
	
};

class CFPlatformFileCookie;


extern CFPlatformFileCookie *FFile_paFileCookie;
extern u32 FFile_nMaxSimultaneousOpen;


#define FFILE_IS_VALID_HANDLE( hFile )			( (s32)(hFile) >= 0 && (s32)(hFile) < (s32)FFile_nMaxSimultaneousOpen )

// Notes on return values:
//   1. On all functions, a return value of zero or positive is always success (ffile_Open() can return 0 on success)
//   2. On all functions, a negative reutrn value is always failure
//   3. When reading, EOF is not checked for. An error will be returned for either EOF or a file error


extern BOOL ffile_ModuleStartup( void );
extern void ffile_ModuleShutdown( void );

// Returns whether or not the module is initialized
extern BOOL ffile_IsInitialized( void );

extern BOOL ffile_IsInMasterMode( void );

// Sets up ffile into either master file mode or directory mode.
// This function keys off of the Fang_ConfigDefs.pszFile_MasterFilePathName field
// and is automatically called on ModuleStartup, but can be called again if 
// a different master file is to be used.
// Care should be taken to ensure that nothing is being loaded before calling this function.
extern void ffile_SetupFileMode( void );

// used to time the load times
extern void ffile_ResetTimer( void );
extern f32 ffile_GetTime( void );

// used to log files during load
extern BOOL ffile_LogSetFilename( cchar *pszFilename );
extern void ffile_LogStart( cchar *pszLevelName );
extern void ffile_AddToLogFile( cchar *pszFilename );// way for fresload_load to add already loaded items to the list
extern void ffile_LogStop( void );

// Asynchronous read callback - a non zero error code indicates a read error
typedef void FFileAsyncReadCallback_t( s32 nErrorCode, FFileHandle hFile, void *pDestBuffer, u32 uReadAmt, void *pUser );

// Asynchronous read Error callback
typedef void FFileAsyncReadErrorCallback_t( FFileAsyncReadErrorCode_e eErrorCode, BOOL bHandleSwap );

// These functions access files that either reside in the standard OS directory hierarchy,
// or reside in a master file:
extern FFileHandle ffile_Open( cchar *pszFName, FFileOpenType_e nOpenMode, BOOL bBypassMasterFile = FALSE );
extern s32 ffile_Close( FFileHandle hFile );
extern s32 ffile_GetFileSize( FFileHandle hFile );
extern s32 ffile_Seek( FFileHandle hFile, s32 nFileOffset, FFileSeekType_e nSeekMode );
extern s32 ffile_Tell( FFileHandle hFile );

// Rules for Asynchronous reads:
//	- Destination buffers and bytes to be read must be multiples of 32.
//	- Seek Positions must be 4 byte aligned.
// Les rgles pour asynchrone lit: 
//	- les amortisseurs et les bytes de destination lire doivent tre des multiples de 32. 
//	- les positions de recherche doivent tre le byte 4 align. 
extern s32 ffile_Read( FFileHandle hFile, u32 uReadAmt, void *pDestData, FFileAsyncReadCallback_t *pCallback = NULL, void *pUser = NULL );

extern s32 ffile_Write( FFileHandle hFile, u32 uWriteAmt, void *pSrcData );
extern s32 ffile_EOF( FFileHandle hFile );
extern s32 ffile_Scanf( FFileHandle hFile, const char *szFmt, ... );
extern s32 ffile_CheckFileReadStatus( FFileHandle hFile, BOOL bFlushCallbacks );
extern s32 ffile_CancelFileIO( FFileHandle hFile );

// Async Error callbacks -- Invoked when async errors occur and normal APP processing will halt
// or be resumed.
extern s32 ffile_SetAsyncReadErrorHaltCallback( FFileAsyncReadErrorCallback_t *pCallback );
extern s32 ffile_SetAsyncReadErrorResumeCallback( FFileAsyncReadErrorCallback_t *pCallback );

// This function is used to inform the app that an Async Read error has been resolved,
// and the App should do anything necessary because normal app processing is going to resume.

// Functions for setting up streaming from the master file
extern const FFileMasterEntry_t* ffile_MF_OpenStream( cchar *pszFName, u32 *pFileSizeOut );
extern s32 ffile_MF_ReadStream( const FFileMasterEntry_t *pDirEntry, u32 uReadStart, u32 uReadAmt, void *pDestData, FFileAsyncReadCallback_t *pCallback, void *pUser = NULL );

// Functions for setting up the internationalization properties

// This routine will take a filename, and determine if it's a file that can be localized.
// Optionally, it will convert the filename (if necessary) to the appropriate localized filename -- this is the default behavior
// A return value of 0 means the filename was not modified,
// A return value of 1 means the filename was modified to a localized version.
extern s32 ffile_LocalizeFilename( char* pcFilenameToLocalize, BOOL bConvertFilenameToLocalizedVersion = TRUE );
extern s32 ffile_GetLanguageChars( char* pcLanguageChar, char* pcAudioLanguageChar );
extern s32 ffile_SetLanguageChars( char cReferenceChar, char cLanguageChar, char cAudioLanguageChar ); // any reference characters are replaced with language characters in ffile_Open calls.

extern void ffile_Work( void ); // Does any platform specific PER FRAME work.

//
//
//
struct QueuedReadToken
{
	void	*pDestination;
	u32		nStartPosition;
	u32		nReadAmount;
	void	*pUser;
	FFileAsyncReadCallback_t *pCallback;
};


#endif

