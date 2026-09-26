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

#include <stdio.h>
#include <io.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//#define _DEVICE_POLL_DELAY	( 0.00f ) // full fps.
//#define _DEVICE_POLL_DELAY	( 0.05f ) // 20 fps.
//#define _DEVICE_POLL_DELAY	( 0.10f ) // 10 fps.
//#define _DEVICE_POLL_DELAY	( 0.20f ) //  5 fps.
#define _DEVICE_POLL_DELAY		( 0.25f ) //  4 fps.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Devices.
static FStorage_DeviceInfo_t _oDeviceInfo;
static u32 _uInserted;
static u64 _uDevicePollDelay, _uDevicePollTimeStamp;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e _fstorage_ReadWriteProfile( BOOL bRead, FStorage_DeviceID_e oeID, const u16 *pwszName, u32 uPosition, void *puBuff, u32 uBuffSize )
{
	FASSERT_MSG( _bModuleInstalled,                          "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                   "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                  "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > wcslen( (unsigned short *)pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( puBuff,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uBuffSize,                                  "[ FSTORAGE ] Error: Zero size !!!" );

	//// Open.
	//
	u16 au16Buff1[ MAX_PATH ];
	wsprintfW( (unsigned short *)au16Buff1, L"profile-%s", pwszName );

	FILE *pFile = _wfopen( (unsigned short *)au16Buff1, L"rb+" );
	if( ! pFile )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: _wfopen() failed for device #%u !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}
	//
	////

	//// Size.
	//
	u32 uIndex = _filelength( _fileno( pFile ) );
	if( -1 == uIndex )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: _filelength() failed for device #%u !!!\n", __LINE__, 0 );
		fclose( pFile );
		return FSTORAGE_ERROR;
	}

	if( ( uPosition + uBuffSize ) > uIndex )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: File size not adequate for operation on device #%u !!!\n", __LINE__, 0 );
		fclose( pFile );
		return FSTORAGE_ERROR;
	}
	//
	////

	//// Seek.
	//
	if( fseek( pFile, uPosition, SEEK_SET ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: fseek() failed for device #%u !!!\n", __LINE__, 0 );
		fclose( pFile );
		return FSTORAGE_ERROR;
	}
	//
	////

	if( bRead )
	{
		//// Read.
		//
		if( uBuffSize != fread( puBuff, 1, uBuffSize, pFile ) )
		{
			DEVPRINTF( "[ FSTORAGE ] Error %u: WriteFile() failed for device #%u !!!\n", __LINE__, 0 );
			fclose( pFile );
			return FSTORAGE_ERROR;
		}
		//
		////
	}
	else
	{
		//// Write.
		//
		if( uBuffSize != fwrite( puBuff, 1, uBuffSize, pFile ) )
		{
			DEVPRINTF( "[ FSTORAGE ] Error %u: WriteFile() failed for device #%u !!!\n", __LINE__, 0 );
			fclose( pFile );
			return FSTORAGE_ERROR;
		}
		//
		////
	}

	fclose( pFile );

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
	wcscpy( (unsigned short *)_oDeviceInfo.wszName, L"PC Hard Disk" );
	_oDeviceInfo.uStatus = FSTORAGE_DEVICE_STATUS_CONNECTED | FSTORAGE_DEVICE_STATUS_AVAILABLE;
	_uInserted = _oDeviceInfo.oeID = FSTORAGE_DEVICE_ID_XB_PC_HD;
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

void _fstorage_UpdateNumProfiles( void ) {
	
	_oDeviceInfo.uNumProfiles = 0;

	_wfinddata_t oFindData;
	u32 uHandle = _wfindfirst( L"profile-*", &oFindData );

	if( -1 != uHandle )
	{
		do
		{
			++( _oDeviceInfo.uNumProfiles );

		} while( 0 == _wfindnext( uHandle, &oFindData ) );

		_findclose( uHandle );
	}
}

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
	_oDeviceInfo.uBytesAvailable = 10000000;
	_oDeviceInfo.uBytesTotal = 12000000;
	_oDeviceInfo.uBytesUsed = _oDeviceInfo.uBytesTotal - _oDeviceInfo.uBytesAvailable;

	_fstorage_UpdateNumProfiles();

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
	FASSERT_MSG( *pwszName,                                    "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > lstrlenW( (unsigned short *)pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( uSize,                                        "[ FSTORAGE ] Error: Zero size !!!" );

	////
	//
	u16 au16Buff1[ MAX_PATH ];
	wsprintfW( (unsigned short *)au16Buff1, L"profile-%s", pwszName );

	FILE *pFile = _wfopen( (unsigned short *)au16Buff1, L"wb" );
	if( ! pFile )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: _wfopen() failed for device #%u !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}
	//
	////

	//// Expand.
	//
	u8 au8Buff1[ MAX_PATH ];
	au8Buff1[ 0 ] = 0;

	if( uSize != fwrite( au8Buff1, 1, uSize, pFile ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: fwrite() failed for device #%u !!!\n", __LINE__, 0 );
		fclose( pFile );
#ifdef _UNICODE
		DeleteFile( (unsigned short *)au16Buff1 );
#else
		u32 nLen = wcslen( (unsigned short *)au16Buff1 );
		WideCharToMultiByte( CP_THREAD_ACP, WC_NO_BEST_FIT_CHARS, (unsigned short *)au16Buff1, nLen, (char *)au8Buff1, sizeof( au8Buff1 ), NULL, NULL );
		au8Buff1[nLen] = 0;
		DeleteFile( (char *)au8Buff1 );
#endif
		return FSTORAGE_ERROR;
	}
	//
	////

	fclose( pFile );

	_fstorage_UpdateNumProfiles();

	return FSTORAGE_ERROR_NONE;

} // fstorage_CreateProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_DeleteProfile( FStorage_DeviceID_e oeID, const u16 *pwszName )
{
	FASSERT_MSG( _bModuleInstalled,                            "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,          "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                    "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > lstrlenW( (unsigned short *)pwszName ), "[ FSTORAGE ] Error: String too long !!!" );

	////
	//
	u16 au16Buff1[ MAX_PATH ];
	wsprintfW( (unsigned short *)au16Buff1, L"profile-%s", pwszName );

#ifdef _UNICODE
	if( ! DeleteFile( (unsigned short *)au16Buff1 ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: DeleteFile() failed for device #%u !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}
#else
	u8 au8Buff1[ MAX_PATH ];
	u32 nLen = wcslen( (unsigned short *)au16Buff1 );
	WideCharToMultiByte( CP_THREAD_ACP, WC_NO_BEST_FIT_CHARS, (unsigned short *)au16Buff1, nLen, (char *)au8Buff1, sizeof( au8Buff1 ), NULL, NULL );
	au8Buff1[nLen] = 0;
	if( ! DeleteFile( (char *)au8Buff1 ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: DeleteFile() failed for device #%u !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR;
	}
#endif
	//
	////

	_fstorage_UpdateNumProfiles();

	return FSTORAGE_ERROR_NONE;

} // fstorage_DeleteProfile

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_ValidateProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, BOOL bUpdateSignature, BOOL *pbIsValid /* = NULL */ )
{
	FASSERT_MSG( _bModuleInstalled,                            "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,          "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                    "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > lstrlenW( (unsigned short *)pwszName ), "[ FSTORAGE ] Error: String too long !!!" );

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
	if( ! _oDeviceInfo.uNumProfiles )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u has no profiles !!!\n", __LINE__, 0 );
		return FSTORAGE_ERROR_NO_PROFILES;
	}
	//
	////

	////
	//
	_wfinddata_t oFindData;
	u32 uHandle = _wfindfirst( L"profile-*", &oFindData );
	u32 nIndex = 0;

	if( -1 != uHandle )
	{
		do
		{
			if( nIndex >= nStartOffset ) {
                wcscpy( (unsigned short *)paoProfile[ *puNumProfilesReturned ].wszName, &( oFindData.name[ 8 ] ) );
				paoProfile[ *puNumProfilesReturned ].uBytesTotal = oFindData.size;

				if( ++( *puNumProfilesReturned ) >= uNumProfiles ) {
					break;
				}
			}
			nIndex++;

		} while( 0 == _wfindnext( uHandle, &oFindData ) );

		_findclose( uHandle );
	}
	//
	////

	return FSTORAGE_ERROR_NONE;

} // fstorage_GetProfileInfos

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FStorage_Error_e fstorage_SetProfileIcon( FStorage_DeviceID_e oeID, const u16 *pwszName, void *puIcon, u32 uIconSize )
{
	FASSERT_MSG( _bModuleInstalled,                            "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_PC_HD == oeID,          "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                    "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > lstrlenW( (unsigned short *)pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
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
