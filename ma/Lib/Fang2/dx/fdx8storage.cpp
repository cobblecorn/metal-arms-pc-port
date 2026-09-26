//////////////////////////////////////////////////////////////////////////////////////
// fdx8storage.cpp - 
//
// Author: Albert Yale
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
// 02/11/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fdx8.h"
#include "fclib.h"
#include "floop.h"
#include "fstorage.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// The PC has one storage device, a directory holding one file per profile.
//
// The directory is MA_PORT_SAVE_DIR (set by the launcher's -save-dir), else
// %APPDATA%\Metal Arms PC Port\Saves, else "saves" in the working directory.
//
// Profile names can hold characters Windows rejects in file names, differ only by case, or end
// in a space, so each UTF-16 unit of the name is stored as four hex digits: "Bob" is saved as
// profile-0042006F0062.sav. Writes go to a .tmp copy that replaces the profile once complete,
// so a crash or a full disk mid-save leaves the previous profile intact.

//#define _DEVICE_POLL_DELAY	( 0.00f ) // full fps.
//#define _DEVICE_POLL_DELAY	( 0.05f ) // 20 fps.
//#define _DEVICE_POLL_DELAY	( 0.10f ) // 10 fps.
//#define _DEVICE_POLL_DELAY	( 0.20f ) //  5 fps.
#define _DEVICE_POLL_DELAY		( 0.25f ) //  4 fps.

#define _BYTES_AVAILABLE		( 10000000 )
#define _BYTES_TOTAL			( 12000000 )

#define _SAVE_DIR_ENV			L"MA_PORT_SAVE_DIR"
#define _SAVE_DIR_APPDATA		L"\\Metal Arms PC Port\\Saves"
#define _SAVE_DIR_FALLBACK		L"saves"

#define _PROFILE_PREFIX			L"profile-"
#define _PROFILE_PREFIX_LEN		( 8 )
#define _PROFILE_EXT			L".sav"
#define _PROFILE_EXT_LEN		( 4 )
#define _TEMP_EXT				L".tmp"

// Earlier port builds saved raw profiles as "profile-<name>" in the working directory; the game
// writes gamesave.h's PROFILE_SIGNATURE first.
#define _LEGACY_PROFILE_SIGNATURE	( 0x55501234 )

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Devices.
static FStorage_DeviceInfo_t _oDeviceInfo;
static u32 _uInserted;
static u64 _uDevicePollDelay, _uDevicePollTimeStamp;

// Absolute save directory, without a trailing separator.
static WCHAR _awszSaveDir[ MAX_PATH ];

static const u8 _au8Zeros[ 4096 ] = { 0 };

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Appends pwszSrc to the string of length *puLen. Returns FALSE, leaving the string as it was,
// when the result would not fit in uMaxChars (terminator included).
static BOOL _AppendW( WCHAR *pwszDst, u32 uMaxChars, u32 *puLen, const WCHAR *pwszSrc )
{
	u32 uLen = *puLen;

	while( *pwszSrc )
	{
		if( uLen + 1 >= uMaxChars )
		{
			pwszDst[ *puLen ] = 0;
			return FALSE;
		}
		pwszDst[ uLen++ ] = *pwszSrc++;
	}

	pwszDst[ uLen ] = 0;
	*puLen = uLen;

	return TRUE;

} // _AppendW

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Returns the length of a usable profile name, or 0 for NULL, empty or too long.
static u32 _ProfileNameLength( cwchar *pwszName )
{
	if( ! pwszName )
	{
		return 0;
	}

	u32 uLen = 0;
	while( pwszName[ uLen ] )
	{
		if( ++uLen >= FSTORAGE_MAX_NAME_LEN )
		{
			return 0;
		}
	}

	return uLen;

} // _ProfileNameLength

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Builds <save dir>\profile-<hex name>.sav, plus .tmp when bTemp.
static BOOL _BuildProfilePath( cwchar *pwszName, BOOL bTemp, WCHAR *pwszPath )
{
	u32 uNameLen = _ProfileNameLength( pwszName );
	if( ! uNameLen )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Invalid profile name !!!\n", __LINE__ );
		return FALSE;
	}

	u32 uLen = 0;
	pwszPath[ 0 ] = 0;
	BOOL bFits = _AppendW( pwszPath, MAX_PATH, &uLen, _awszSaveDir ) &&
				 _AppendW( pwszPath, MAX_PATH, &uLen, L"\\" _PROFILE_PREFIX );

	for( u32 i = 0; bFits && i < uNameLen; ++i )
	{
		WCHAR awszHex[ 5 ];
		for( u32 uDigit = 0; uDigit < 4; ++uDigit )
		{
			u32 uNibble = ( (u32)pwszName[ i ] >> ( 12 - 4 * uDigit ) ) & 0xF;
			awszHex[ uDigit ] = (WCHAR)( uNibble < 10 ? ( L'0' + uNibble ) : ( L'A' + uNibble - 10 ) );
		}
		awszHex[ 4 ] = 0;
		bFits = _AppendW( pwszPath, MAX_PATH, &uLen, awszHex );
	}

	bFits = bFits && _AppendW( pwszPath, MAX_PATH, &uLen, _PROFILE_EXT );
	if( bTemp )
	{
		bFits = bFits && _AppendW( pwszPath, MAX_PATH, &uLen, _TEMP_EXT );
	}

	if( ! bFits )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Profile path too long for save directory '%ls' !!!\n", __LINE__, _awszSaveDir );
		return FALSE;
	}

	return TRUE;

} // _BuildProfilePath

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static s32 _HexDigitValue( WCHAR wc )
{
	if( wc >= L'0' && wc <= L'9' ) return wc - L'0';
	if( wc >= L'A' && wc <= L'F' ) return wc - L'A' + 10;
	if( wc >= L'a' && wc <= L'f' ) return wc - L'a' + 10;
	return -1;

} // _HexDigitValue

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Compares uLen characters, ignoring ASCII case (Windows file names ignore case).
static BOOL _MatchIgnoringCase( const WCHAR *pwszA, const WCHAR *pwszB, u32 uLen )
{
	for( u32 i = 0; i < uLen; ++i )
	{
		WCHAR wcA = pwszA[ i ], wcB = pwszB[ i ];
		if( wcA >= L'A' && wcA <= L'Z' ) wcA = (WCHAR)( wcA - L'A' + L'a' );
		if( wcB >= L'A' && wcB <= L'Z' ) wcB = (WCHAR)( wcB - L'A' + L'a' );
		if( wcA != wcB )
		{
			return FALSE;
		}
	}

	return TRUE;

} // _MatchIgnoringCase

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Recovers the profile name from a file name written by _BuildProfilePath (no directory).
// pwszName must hold FSTORAGE_MAX_NAME_LEN characters.
static BOOL _DecodeProfileFileName( const WCHAR *pwszFileName, wchar *pwszName )
{
	u32 uLen = (u32)lstrlenW( pwszFileName );
	if( uLen <= _PROFILE_PREFIX_LEN + _PROFILE_EXT_LEN )
	{
		return FALSE;
	}

	u32 uHexLen = uLen - _PROFILE_PREFIX_LEN - _PROFILE_EXT_LEN;
	if( ( uHexLen & 3 ) || ( uHexLen / 4 ) >= FSTORAGE_MAX_NAME_LEN ||
		! _MatchIgnoringCase( pwszFileName, _PROFILE_PREFIX, _PROFILE_PREFIX_LEN ) ||
		! _MatchIgnoringCase( pwszFileName + uLen - _PROFILE_EXT_LEN, _PROFILE_EXT, _PROFILE_EXT_LEN ) )
	{
		return FALSE;
	}

	const WCHAR *pwszHex = pwszFileName + _PROFILE_PREFIX_LEN;
	u32 uNameLen = uHexLen / 4;

	for( u32 i = 0; i < uNameLen; ++i )
	{
		u32 uUnit = 0;
		for( u32 uDigit = 0; uDigit < 4; ++uDigit )
		{
			s32 nValue = _HexDigitValue( *pwszHex++ );
			if( nValue < 0 )
			{
				return FALSE;
			}
			uUnit = ( uUnit << 4 ) | (u32)nValue;
		}
		if( ! uUnit )
		{
			return FALSE;
		}
		pwszName[ i ] = (wchar)uUnit;
	}
	pwszName[ uNameLen ] = 0;

	return TRUE;

} // _DecodeProfileFileName

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Counts the profiles in the save directory, in directory order. When paoProfile is given, also
// returns up to uMaxProfiles of them starting with the uStartOffset'th.
static u32 _ScanProfiles( FStorage_ProfileInfo_t *paoProfile, u32 uMaxProfiles, u32 uStartOffset, u32 *puNumReturned )
{
	u32 uNumReturned = 0;
	u32 uCount = 0;

	WCHAR awszPattern[ MAX_PATH ];
	u32 uLen = 0;
	awszPattern[ 0 ] = 0;

	if( _AppendW( awszPattern, MAX_PATH, &uLen, _awszSaveDir ) &&
		_AppendW( awszPattern, MAX_PATH, &uLen, L"\\" _PROFILE_PREFIX L"*" ) )
	{
		WIN32_FIND_DATAW oFindData;
		HANDLE hFind = FindFirstFileW( awszPattern, &oFindData );

		if( INVALID_HANDLE_VALUE != hFind )
		{
			wchar awszName[ FSTORAGE_MAX_NAME_LEN ];

			do
			{
				if( ( oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ||
					! _DecodeProfileFileName( oFindData.cFileName, awszName ) )
				{
					continue;
				}

				if( paoProfile && uCount >= uStartOffset && uNumReturned < uMaxProfiles )
				{
					fclib_wcscpy( paoProfile[ uNumReturned ].wszName, awszName );
					paoProfile[ uNumReturned ].uBytesTotal = oFindData.nFileSizeLow;
					++uNumReturned;
				}
				++uCount;

			} while( FindNextFileW( hFind, &oFindData ) );

			FindClose( hFind );
		}
	}

	if( puNumReturned )
	{
		*puNumReturned = uNumReturned;
	}

	return uCount;

} // _ScanProfiles

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _fstorage_UpdateNumProfiles( void )
{
	_oDeviceInfo.uNumProfiles = _ScanProfiles( NULL, 0, 0, NULL );

} // _fstorage_UpdateNumProfiles

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Creates every missing directory in pwszDir (which is modified and restored along the way).
static BOOL _CreateDirectoryTree( WCHAR *pwszDir )
{
	if( ! pwszDir[ 0 ] )
	{
		return FALSE;
	}

	for( WCHAR *pwc = pwszDir + 1; *pwc; ++pwc )
	{
		if( ( *pwc == L'\\' || *pwc == L'/' ) && pwc[ -1 ] != L':' && pwc[ -1 ] != L'\\' && pwc[ -1 ] != L'/' )
		{
			WCHAR wcSeparator = *pwc;
			*pwc = 0;
			CreateDirectoryW( pwszDir, NULL ); // Failures show up in the final check.
			*pwc = wcSeparator;
		}
	}
	CreateDirectoryW( pwszDir, NULL );

	DWORD dwAttributes = GetFileAttributesW( pwszDir );
	return ( INVALID_FILE_ATTRIBUTES != dwAttributes ) && ( dwAttributes & FILE_ATTRIBUTE_DIRECTORY );

} // _CreateDirectoryTree

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void _StripTrailingSeparators( WCHAR *pwszDir )
{
	u32 uLen = (u32)lstrlenW( pwszDir );
	while( uLen > 1 && ( pwszDir[ uLen - 1 ] == L'\\' || pwszDir[ uLen - 1 ] == L'/' ) && pwszDir[ uLen - 2 ] != L':' )
	{
		pwszDir[ --uLen ] = 0;
	}

} // _StripTrailingSeparators

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static BOOL _InitSaveDir( void )
{
	WCHAR awszDir[ MAX_PATH ];
	_awszSaveDir[ 0 ] = 0;

	DWORD dwLen = GetEnvironmentVariableW( _SAVE_DIR_ENV, awszDir, MAX_PATH );
	if( dwLen >= MAX_PATH )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: MA_PORT_SAVE_DIR is too long !!!\n", __LINE__ );
		return FALSE;
	}

	if( ! dwLen )
	{
		u32 uLen = GetEnvironmentVariableW( L"APPDATA", awszDir, MAX_PATH );
		if( ! uLen || uLen >= MAX_PATH || ! _AppendW( awszDir, MAX_PATH, &uLen, _SAVE_DIR_APPDATA ) )
		{
			fclib_wcscpy( awszDir, _SAVE_DIR_FALLBACK );
		}
	}

	_StripTrailingSeparators( awszDir );

	if( ! awszDir[ 0 ] || ! _CreateDirectoryTree( awszDir ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Could not create save directory '%ls' !!!\n", __LINE__, awszDir );
		return FALSE;
	}

	dwLen = GetFullPathNameW( awszDir, MAX_PATH, _awszSaveDir, NULL );
	if( ! dwLen || dwLen >= MAX_PATH )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Could not resolve save directory '%ls' !!!\n", __LINE__, awszDir );
		_awszSaveDir[ 0 ] = 0;
		return FALSE;
	}
	_StripTrailingSeparators( _awszSaveDir );

	return TRUE;

} // _InitSaveDir

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Copies profiles saved by earlier port builds (see _LEGACY_PROFILE_SIGNATURE) from the working
// directory into the save directory. Existing profiles are not overwritten and the originals
// are left in place.
static void _CopyLegacyProfiles( void )
{
	WIN32_FIND_DATAW oFindData;
	HANDLE hFind = FindFirstFileW( _PROFILE_PREFIX L"*", &oFindData );

	if( INVALID_HANDLE_VALUE == hFind )
	{
		return;
	}

	wchar awszName[ FSTORAGE_MAX_NAME_LEN ];
	WCHAR awszPath[ MAX_PATH ];

	do
	{
		const WCHAR *pwszLegacyName = oFindData.cFileName + _PROFILE_PREFIX_LEN;

		// Skip directories, and profiles already in the new format (the working directory can be
		// the save directory).
		if( ( oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ||
			_DecodeProfileFileName( oFindData.cFileName, awszName ) ||
			! _ProfileNameLength( (cwchar *)pwszLegacyName ) )
		{
			continue;
		}

		u32 uSignature = 0;
		DWORD dwRead = 0;
		HANDLE hFile = CreateFileW( oFindData.cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( INVALID_HANDLE_VALUE == hFile )
		{
			continue;
		}
		BOOL bIsProfile = ReadFile( hFile, &uSignature, sizeof( uSignature ), &dwRead, NULL ) &&
						  dwRead == sizeof( uSignature ) && uSignature == _LEGACY_PROFILE_SIGNATURE;
		CloseHandle( hFile );

		if( ! bIsProfile || ! _BuildProfilePath( (cwchar *)pwszLegacyName, FALSE, awszPath ) )
		{
			continue;
		}

		if( CopyFileW( oFindData.cFileName, awszPath, TRUE ) )
		{
			DEVPRINTF( "[ FSTORAGE ] Copied profile '%ls' from the working directory into the save directory.\n", pwszLegacyName );
		}
		else if( ERROR_FILE_EXISTS != GetLastError() )
		{
			DEVPRINTF( "[ FSTORAGE ] Error %u: Could not copy profile '%ls' into the save directory (error %u) !!!\n", __LINE__, pwszLegacyName, GetLastError() );
		}

	} while( FindNextFileW( hFind, &oFindData ) );

	FindClose( hFind );

} // _CopyLegacyProfiles

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Reads or writes uBuffSize bytes at uPosition, which must lie within the file.
static BOOL _ReadWriteAt( HANDLE hFile, BOOL bRead, u32 uPosition, void *puBuff, u32 uBuffSize )
{
	DWORD dwSizeHigh = 0;
	DWORD dwSize = GetFileSize( hFile, &dwSizeHigh );
	if( INVALID_FILE_SIZE == dwSize && NO_ERROR != GetLastError() )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: GetFileSize() failed (error %u) !!!\n", __LINE__, GetLastError() );
		return FALSE;
	}

	u64 uFileBytes = ( (u64)dwSizeHigh << 32 ) | dwSize;
	if( (u64)uPosition + uBuffSize > uFileBytes )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: File size not adequate for operation !!!\n", __LINE__ );
		return FALSE;
	}

	if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, (LONG)uPosition, NULL, FILE_BEGIN ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: SetFilePointer() failed (error %u) !!!\n", __LINE__, GetLastError() );
		return FALSE;
	}

	DWORD dwDone = 0;
	BOOL bOk = bRead ? ReadFile( hFile, puBuff, uBuffSize, &dwDone, NULL )
					 : WriteFile( hFile, puBuff, uBuffSize, &dwDone, NULL );
	if( ! bOk || dwDone != uBuffSize )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: %s failed (error %u) !!!\n", __LINE__, bRead ? "ReadFile()" : "WriteFile()", GetLastError() );
		return FALSE;
	}

	return TRUE;

} // _ReadWriteAt

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Replaces the profile with its completed .tmp copy.
static BOOL _CommitTempFile( const WCHAR *pwszTemp, const WCHAR *pwszPath )
{
	if( ! MoveFileExW( pwszTemp, pwszPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: MoveFileEx() failed (error %u) !!!\n", __LINE__, GetLastError() );
		return FALSE;
	}

	return TRUE;

} // _CommitTempFile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static FStorage_Error_e _fstorage_ReadWriteProfile( BOOL bRead, FStorage_DeviceID_e oeID, cwchar *pwszName, u32 uPosition, void *puBuff, u32 uBuffSize )
{
	FASSERT_MSG( _bModuleInstalled,                          "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                   "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puBuff,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uBuffSize,                                  "[ FSTORAGE ] Error: Zero size !!!" );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}

	WCHAR awszPath[ MAX_PATH ];
	if( ! puBuff || ! uBuffSize || ! _BuildProfilePath( pwszName, FALSE, awszPath ) )
	{
		return FSTORAGE_ERROR;
	}

	if( bRead )
	{
		HANDLE hFile = CreateFileW( awszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( INVALID_HANDLE_VALUE == hFile )
		{
			// The menus read a name to find out whether it is taken, so a missing profile is expected.
			DWORD dwError = GetLastError();
			if( ERROR_FILE_NOT_FOUND != dwError )
			{
				DEVPRINTF( "[ FSTORAGE ] Error %u: CreateFile() failed for device #%u (error %u) !!!\n", __LINE__, 0, dwError );
			}
			return FSTORAGE_ERROR;
		}

		BOOL bOk = _ReadWriteAt( hFile, TRUE, uPosition, puBuff, uBuffSize );
		CloseHandle( hFile );

		return bOk ? FSTORAGE_ERROR_NONE : FSTORAGE_ERROR;
	}

	WCHAR awszTemp[ MAX_PATH ];
	if( ! _BuildProfilePath( pwszName, TRUE, awszTemp ) )
	{
		return FSTORAGE_ERROR;
	}

	// Writing requires an existing profile, as it did with the original fopen( "rb+" ).
	if( ! CopyFileW( awszPath, awszTemp, FALSE ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: CopyFile() failed for device #%u (error %u) !!!\n", __LINE__, 0, GetLastError() );
		return FSTORAGE_ERROR;
	}

	HANDLE hFile = CreateFileW( awszTemp, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: CreateFile() failed for device #%u (error %u) !!!\n", __LINE__, 0, GetLastError() );
		DeleteFileW( awszTemp );
		return FSTORAGE_ERROR;
	}

	BOOL bOk = _ReadWriteAt( hFile, FALSE, uPosition, puBuff, uBuffSize );
	if( bOk && ! FlushFileBuffers( hFile ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: FlushFileBuffers() failed for device #%u (error %u) !!!\n", __LINE__, 0, GetLastError() );
		bOk = FALSE;
	}
	CloseHandle( hFile );

	if( ! bOk || ! _CommitTempFile( awszTemp, awszPath ) )
	{
		DeleteFileW( awszTemp );
		return FSTORAGE_ERROR;
	}

	return FSTORAGE_ERROR_NONE;

} // _fstorage_ReadWriteProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fstorage_ModuleStartup( void )
{
	FASSERT_MSG( ! _bModuleStarted, "[ FSTORAGE ] Error: System already started !!!" );

	_bModuleStarted = TRUE;
	_bModuleInstalled = FALSE;

	return TRUE;

} // fstorage_ModuleStartup

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fstorage_ModuleShutdown( void )
{
	fstorage_Uninstall();

	_bModuleStarted = FALSE;

} // fstorage_ModuleShutdown

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_Install( const FStorage_Init_t *poInit /* = NULL */ )
{
	FASSERT_MSG( _bModuleStarted,     "[ FSTORAGE ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled, "[ FSTORAGE ] Error: System already installed !!!" );

	fang_MemZero( &_oDeviceInfo, sizeof( _oDeviceInfo ) );

	////
	//
	fclib_wcscpy( _oDeviceInfo.wszName, L"PC Hard Disk" );
	_uInserted = _oDeviceInfo.oeID = FSTORAGE_DEVICE_ID_XB_PC_HD;

	// A save directory that cannot be used leaves the device connected but unavailable, so the
	// menus report it instead of the game failing to start.
	if( _InitSaveDir() )
	{
		_oDeviceInfo.uStatus = FSTORAGE_DEVICE_STATUS_CONNECTED | FSTORAGE_DEVICE_STATUS_AVAILABLE;
		_oDeviceInfo.uBytesAvailable = _BYTES_AVAILABLE;
		_oDeviceInfo.uBytesTotal = _BYTES_TOTAL;
		_oDeviceInfo.uBytesUsed = _oDeviceInfo.uBytesTotal - _oDeviceInfo.uBytesAvailable;

		DEVPRINTF( "[ FSTORAGE ] Saving profiles in '%ls'.\n", _awszSaveDir );
		_CopyLegacyProfiles();
		_fstorage_UpdateNumProfiles();
	}
	else
	{
		_oDeviceInfo.uStatus = FSTORAGE_DEVICE_STATUS_CONNECTED;
		DEVPRINTF( "[ FSTORAGE ] Error %u: No usable save directory; profiles cannot be saved !!!\n", __LINE__ );
	}
	//
	////

	_uDevicePollDelay     = (u64)( _DEVICE_POLL_DELAY * FLoop_fTicksPerSec );
	_uDevicePollTimeStamp = 0;

	_bModuleInstalled = TRUE;

	return FSTORAGE_ERROR_NONE;

} // fstorage_Install

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fstorage_Uninstall( void )
{
	if( ! _bModuleInstalled )
	{
		return;
	}

	_bModuleInstalled = FALSE;

} // fstorage_Uninstall

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BOOL fstorage_IsInstalled( void )
{
	return _bModuleInstalled;

} // fstorage_IsInstalled

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FStorage_Error_e fstorage_CalcSaveGameSize( FStorage_DeviceID_e oeID, u32 uDataBytes, u32 *puTotalBytes ) {
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( puTotalBytes,                        "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puTotalBytes = 0;

	if( ! uDataBytes )
	{
		return FSTORAGE_ERROR_NONE;
	}

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, FSTORAGE_DEVICE_ID_XB_PC_HD );
		return FSTORAGE_ERROR;
	}

	////
	//
	*puTotalBytes = uDataBytes;
	//
	////

	return FSTORAGE_ERROR_NONE;
} // fstorage fstorage_CalcSaveGameSize


FStorage_Error_e fstorage_BytesToBlocks( FStorage_DeviceID_e oeID, u32 uBytes, u32 *puBlocks )
{
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( puBlocks,                            "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puBlocks = 0;

	if( ! uBytes )
	{
		return FSTORAGE_ERROR_NONE;
	}

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, FSTORAGE_DEVICE_ID_XB_PC_HD );
		return FSTORAGE_ERROR;
	}

	////
	//
	*puBlocks =  ( ( uBytes / 10240 ) + ( ( uBytes % 10240 ) ? 1 : 0 ) );
	//
	////

	return FSTORAGE_ERROR_NONE;

} // fstorage_BytesToBlocks

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void fstorage_UpdateDeviceInfos( u32 *puConnected, u32 *puInserted, u32 *puRemoved, BOOL bForceUpdate )
{
	FASSERT_MSG( _bModuleInstalled, "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( puConnected,       "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puInserted,        "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puRemoved,         "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puConnected = FSTORAGE_DEVICE_ID_XB_PC_HD;
	*puInserted = _uInserted;
	*puRemoved = 0;

	////
	//
	if( _uDevicePollDelay > ( FLoop_nRealTotalLoopTicks - _uDevicePollTimeStamp ) )
	{
		return;
	}

	_uDevicePollTimeStamp = FLoop_nRealTotalLoopTicks;
	//
	////

	//// HD.
	//
	if( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus )
	{
		_fstorage_UpdateNumProfiles();
	}

	_uInserted = 0;

} // fstorage_UpdateDeviceInfos

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_FormatDevice( FStorage_DeviceID_e oeID )
{
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	DEVPRINTF( "[ FSTORAGE ] Error %u: fstorage_FormatDevice() unavailable on PC !!!\n", __LINE__ );

	return FSTORAGE_ERROR;

} // fstorage_FormatDevice

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( FStorage_DeviceID_e oeID )
{
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	return &_oDeviceInfo;

} // fstorage_GetDeviceInfo

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( u32 uIndex )
{
	FASSERT_MSG( _bModuleInstalled,             "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( uIndex >= 0,                   "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( uIndex < FSTORAGE_MAX_DEVICES, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	return &_oDeviceInfo;

} // fstorage_GetDeviceInfo


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_CreateProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, u32 uSize )
{
	FASSERT_MSG( _bModuleInstalled,                            "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,          "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uSize,                                        "[ FSTORAGE ] Error: Zero size !!!" );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}

	WCHAR awszPath[ MAX_PATH ], awszTemp[ MAX_PATH ];
	if( ! uSize || ! _BuildProfilePath( pwszName, FALSE, awszPath ) || ! _BuildProfilePath( pwszName, TRUE, awszTemp ) )
	{
		return FSTORAGE_ERROR;
	}

	//// Create a zero-filled profile, replacing any existing one with this name.
	//
	HANDLE hFile = CreateFileW( awszTemp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: CreateFile() failed for device #%u (error %u) !!!\n", __LINE__, 0, GetLastError() );
		return FSTORAGE_ERROR;
	}

	BOOL bOk = TRUE;
	for( u32 uLeft = uSize; bOk && uLeft; )
	{
		DWORD dwChunk = ( uLeft < sizeof( _au8Zeros ) ) ? uLeft : sizeof( _au8Zeros );
		DWORD dwDone = 0;
		bOk = WriteFile( hFile, _au8Zeros, dwChunk, &dwDone, NULL ) && dwDone == dwChunk;
		uLeft -= dwChunk;
	}
	if( ! bOk || ! FlushFileBuffers( hFile ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: WriteFile() failed for device #%u (error %u) !!!\n", __LINE__, 0, GetLastError() );
		bOk = FALSE;
	}
	CloseHandle( hFile );

	if( ! bOk || ! _CommitTempFile( awszTemp, awszPath ) )
	{
		DeleteFileW( awszTemp );
		return FSTORAGE_ERROR;
	}
	//
	////

	_fstorage_UpdateNumProfiles();

	return FSTORAGE_ERROR_NONE;

} // fstorage_CreateProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_DeleteProfile( FStorage_DeviceID_e oeID, const u16 *pwszName )
{
	FASSERT_MSG( _bModuleInstalled,                            "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,          "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}

	////
	//
	WCHAR awszPath[ MAX_PATH ];
	if( ! _BuildProfilePath( pwszName, FALSE, awszPath ) )
	{
		return FSTORAGE_ERROR;
	}

	if( ! DeleteFileW( awszPath ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: DeleteFile() failed for device #%u (error %u) !!!\n", __LINE__, 0, GetLastError() );
		return FSTORAGE_ERROR;
	}
	//
	////

	_fstorage_UpdateNumProfiles();

	return FSTORAGE_ERROR_NONE;

} // fstorage_DeleteProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Succeeds when the profile exists; CPlayerProfile::IsOnCard() depends on that, as on the Xbox.
// The PC keeps no save signature: the game's own header and CRC are checked when a profile loads.
FStorage_Error_e fstorage_ValidateProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, BOOL bUpdateSignature, BOOL *pbIsValid /* = NULL */ )
{
	FASSERT_MSG( _bModuleInstalled,                            "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,          "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );

	if( pbIsValid )
	{
		*pbIsValid = FALSE;
	}

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}

	WCHAR awszPath[ MAX_PATH ];
	WIN32_FILE_ATTRIBUTE_DATA oData;
	if( ! _BuildProfilePath( pwszName, FALSE, awszPath ) ||
		! GetFileAttributesExW( awszPath, GetFileExInfoStandard, &oData ) ||
		( oData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ||
		( ! oData.nFileSizeLow && ! oData.nFileSizeHigh ) )
	{
		return FSTORAGE_ERROR;
	}

	if( pbIsValid )
	{
		*pbIsValid = TRUE;
	}

	return FSTORAGE_ERROR_NONE;

} // fstorage_ValidateProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_GetProfileInfos( FStorage_DeviceID_e oeID,
										  FStorage_ProfileInfo_t *paoProfile,
										  u32 uNumProfiles,
										  u32 *puNumProfilesReturned,
										  u32 nStartOffset/*=0*/ )
{
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( paoProfile,                          "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uNumProfiles,                        "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( puNumProfilesReturned,               "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puNumProfilesReturned = 0;

	////
	//
	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _oDeviceInfo.uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR_DEVICE_UNAVAILABLE;
	}

	if( ! _oDeviceInfo.uNumProfiles )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u has no profiles !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR_NO_PROFILES;
	}

	if( nStartOffset >= _oDeviceInfo.uNumProfiles )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u doesn't have the specified offset value !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR_NO_PROFILES;
	}
	//
	////

	_ScanProfiles( paoProfile, uNumProfiles, nStartOffset, puNumProfilesReturned );

	// The files can change outside the game; don't hand back entries that were not filled in.
	if( ! *puNumProfilesReturned )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u has no profile at offset %u !!!\n", __LINE__, 0, nStartOffset );
		return FSTORAGE_ERROR_NO_PROFILES;
	}

	return FSTORAGE_ERROR_NONE;

} // fstorage_GetProfileInfos

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_SetProfileIcon( FStorage_DeviceID_e oeID, const u16 *pwszName, void *puIcon, u32 uIconSize )
{
	FASSERT_MSG( _bModuleInstalled,                            "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,          "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( _ProfileNameLength( pwszName ),               "[ FSTORAGE ] Error: Invalid profile name !!!" );
	FASSERT_MSG( puIcon,                                       "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uIconSize,                                    "[ FSTORAGE ] Error: Zero size !!!" );

	return FSTORAGE_ERROR_NONE;

} // fstorage_SetProfileIcon

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_ReadProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, u32 uPosition, void *puBuff, u32 uBuffSize )
{
	return _fstorage_ReadWriteProfile( TRUE, oeID, pwszName, uPosition, puBuff, uBuffSize );

} // fstorage_ReadProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_WriteProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, u32 uPosition, void *puBuff, u32 uBuffSize )
{
	return _fstorage_ReadWriteProfile( FALSE, oeID, pwszName, uPosition, puBuff, uBuffSize );

} // fstorage_WriteProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
