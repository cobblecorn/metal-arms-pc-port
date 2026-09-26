//////////////////////////////////////////////////////////////////////////////////////
// fxbstorage.cpp - 
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
// 12/28/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fdx8.h"
#include "fclib.h"
#include "floop.h"
#include "fstorage.h"

#include <stdio.h>


#define _DEVICE_POLL_DELAY( TicksPerSecond )	( TicksPerSecond >> 2 )//  4 fps.

#define _PORT_FROM_DRIVE_LETTER( c )			( XMUPortFromDriveLetter( c ) ) 
#define _SLOT_FROM_DRIVE_LETTER( c )			( XMUSlotFromDriveLetter( c ) ) 


// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Devices.
static DWORD _dwConnected, _dwInserted, _dwRemoved;
static u32 _auDeviceMasks[ FSTORAGE_MAX_DEVICES ], _uConnected;
static u64 _uDevicePollDelay, _uDevicePollTimeStamp;
static FStorage_DeviceInfo_t _aoDeviceInfos[ FSTORAGE_MAX_DEVICES ];

static cchar *_apszDriveLetters[FSTORAGE_MAX_DEVICES] = {
	"U:\\",// FSTORAGE_DEVICE_ID_XB_PC_HD	
	"F:\\",// FSTORAGE_DEVICE_ID_XB_MC_0A
	"G:\\",// FSTORAGE_DEVICE_ID_XB_MC_0B
	"H:\\",// FSTORAGE_DEVICE_ID_XB_MC_1A
	"I:\\",// FSTORAGE_DEVICE_ID_XB_MC_1B
	"J:\\",// FSTORAGE_DEVICE_ID_XB_MC_2A
	"K:\\",// FSTORAGE_DEVICE_ID_XB_MC_2B
	"L:\\",// FSTORAGE_DEVICE_ID_XB_MC_3A
	"M:\\" // FSTORAGE_DEVICE_ID_XB_MC_3B
};


FINLINE u32 _fstorage_DeviceIDtoIndex( FStorage_DeviceID_e oeID ) {
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	u32 uIndex = 0;
	while( ! ( (u32)oeID & ( 1 << uIndex++ ) ) );

	return ( uIndex - 1 );

}


FStorage_Error_e _fstorage_ReadWriteProfile( BOOL bRead,
											FStorage_DeviceID_e oeID,
											const u16 *pwszName,
											u32 uPosition,
											void *puBuff,
											u32 uBuffSize ) {
	FASSERT_MSG( _bModuleInstalled,                          "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                   "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                  "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( puBuff,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uBuffSize,                                  "[ FSTORAGE ] Error: Zero size !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _aoDeviceInfos[ uDeviceIndex ].uStatus ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	// Open.
	u8 szString[ MAX_PATH ];
	if( ERROR_SUCCESS != XCreateSaveGame( _apszDriveLetters[uDeviceIndex], pwszName, OPEN_EXISTING, 0, (char *)szString, MAX_PATH ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: XCreateSaveGame() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	fclib_strcat( (char *)szString, "data.bin" );
	HANDLE ohFile = CreateFile( (char *)szString, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL );
	if( INVALID_HANDLE_VALUE == ohFile ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CreateFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	// Size.
	u32 uIndex = GetFileSize( ohFile, NULL );
	if( -1 == uIndex ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: GetFileSize() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		return FSTORAGE_ERROR;
	}

	if( ( sizeof( XCALCSIG_SIGNATURE ) + uPosition + uBuffSize ) > uIndex ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: File size not adequate for operation on device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		return FSTORAGE_ERROR;
	}
	
	// Seek.
	if( INVALID_SET_FILE_POINTER == SetFilePointer( ohFile, sizeof( XCALCSIG_SIGNATURE ) + uPosition, NULL, FILE_BEGIN ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: SetFilePointer() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		return FSTORAGE_ERROR;
	}
	
	u32 uTemp;

	if( bRead ) {
		// Read.
		if( ! ReadFile( ohFile, puBuff, uBuffSize, (LPDWORD)&uTemp, NULL ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: ReadFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CloseHandle( ohFile );
			return FSTORAGE_ERROR;
		}
	} else {
		// Write.
		if( ! WriteFile( ohFile, puBuff, uBuffSize, (LPDWORD)&uTemp, NULL ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: WriteFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CloseHandle( ohFile );
			return FSTORAGE_ERROR;
		}
	}

	CloseHandle( ohFile );

	return FSTORAGE_ERROR_NONE;

}

void _fstorage_UpdateNumProfiles( s32 nDeviceIndex ) {

	// assume 0 profiles
	_aoDeviceInfos[nDeviceIndex].uNumProfiles = 0;
	
	// find the first saved game
	XGAME_FIND_DATA oFindData;
	HANDLE ohFile = XFindFirstSaveGame( _apszDriveLetters[nDeviceIndex], &oFindData );

	if( ohFile != INVALID_HANDLE_VALUE ) {
		// walk all of the saved games and count how many files we find
		do {
			++_aoDeviceInfos[nDeviceIndex].uNumProfiles;
		} while( XFindNextSaveGame( ohFile, &oFindData ) );

		// close the find file handle
		XFindClose( ohFile );
	}
}


BOOL fstorage_ModuleStartup( void ) {
	FASSERT_MSG( ! _bModuleStarted, "[ FSTORAGE ] Error: System already started !!!" );

	_bModuleStarted = TRUE;
	_bModuleInstalled = FALSE;

	return TRUE;

}


void fstorage_ModuleShutdown( void ) {
	fstorage_Uninstall();

	_bModuleStarted = FALSE;

}


FStorage_Error_e fstorage_Install( const FStorage_Init_t *poInit/*=NULL*/ ) {
	FASSERT_MSG( _bModuleStarted,     "[ FSTORAGE ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled, "[ FSTORAGE ] Error: System already installed !!!" );

	fang_MemZero( _auDeviceMasks, sizeof( _auDeviceMasks ) );
	fang_MemZero( _aoDeviceInfos, sizeof( _aoDeviceInfos ) );

	_auDeviceMasks[ 1 ] = XDEVICE_PORT0_TOP_MASK;
	_auDeviceMasks[ 2 ] = XDEVICE_PORT0_BOTTOM_MASK;
	_auDeviceMasks[ 3 ] = XDEVICE_PORT1_TOP_MASK;
	_auDeviceMasks[ 4 ] = XDEVICE_PORT1_BOTTOM_MASK;
	_auDeviceMasks[ 5 ] = XDEVICE_PORT2_TOP_MASK;
	_auDeviceMasks[ 6 ] = XDEVICE_PORT2_BOTTOM_MASK;
	_auDeviceMasks[ 7 ] = XDEVICE_PORT3_TOP_MASK;
	_auDeviceMasks[ 8 ] = XDEVICE_PORT3_BOTTOM_MASK;
	
	// HD.
	wcscpy( _aoDeviceInfos[ 0 ].wszName, L"Xbox Hard Disk" );
	_aoDeviceInfos[ 0 ].uStatus = FSTORAGE_DEVICE_STATUS_CONNECTED | FSTORAGE_DEVICE_STATUS_AVAILABLE;
	_aoDeviceInfos[ 0 ].oeID = FSTORAGE_DEVICE_ID_XB_PC_HD;
	_fstorage_UpdateNumProfiles( 0 );
	GetDiskFreeSpaceEx( _apszDriveLetters[0], (PULARGE_INTEGER)&( _aoDeviceInfos[ 0 ].uBytesAvailable ), (PULARGE_INTEGER)&( _aoDeviceInfos[ 0 ].uBytesTotal ), NULL );
	
	for( u32 uIndex = 1; FSTORAGE_MAX_DEVICES > uIndex; ++uIndex ) {
		_aoDeviceInfos[ uIndex ].oeID = (FStorage_DeviceID_e)( 1 << uIndex );
	}

	_uConnected = _dwConnected = 0;

	_uDevicePollDelay     = (u64)( _DEVICE_POLL_DELAY( FLoop_nTicksPerSec ) );
	_uDevicePollTimeStamp = 0;

	_bModuleInstalled = TRUE;

	return FSTORAGE_ERROR_NONE;

}


void fstorage_Uninstall( void ) {

	if( ! _bModuleInstalled ) {
		return;
	}

	for( u32 uIndex = 1; FSTORAGE_MAX_DEVICES > uIndex; ++uIndex ) {
		if( FSTORAGE_DEVICE_STATUS_AVAILABLE & _aoDeviceInfos[ uIndex ].uStatus ) {
			XUnmountMU( _PORT_FROM_DRIVE_LETTER( *_apszDriveLetters[uIndex] ),
						_SLOT_FROM_DRIVE_LETTER( *_apszDriveLetters[uIndex] ) );
		}
	}

	_bModuleInstalled = FALSE;
}


BOOL fstorage_IsInstalled( void ) {
	return _bModuleInstalled;
}


FStorage_Error_e fstorage_CalcSaveGameSize( FStorage_DeviceID_e oeID, u32 uDataBytes, u32 *puTotalBytes ) {
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( puTotalBytes,                        "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puTotalBytes = 0;

	if( ! uDataBytes )
	{
		return FSTORAGE_ERROR_NONE;
	}

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _aoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	u32 uTemp = XGetDiskClusterSize( _apszDriveLetters[uDeviceIndex] );
	if( ! uTemp ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: XGetDiskClusterSize() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	// adding two clusters.  One for the directory, one for the savegame metadata.
	*puTotalBytes = uDataBytes + (2 * uTemp);

	return FSTORAGE_ERROR_NONE;

} // fstorage fstorage_CalcSaveGameSize


FStorage_Error_e fstorage_BytesToBlocks( FStorage_DeviceID_e oeID, u32 uBytes, u32 *puBlocks ) {
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( puBlocks,                            "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puBlocks = 0;

	if( ! uBytes ) {
		return FSTORAGE_ERROR_NONE;
	}

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _aoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	u32 uTemp = XGetDiskClusterSize( _apszDriveLetters[uDeviceIndex] );
	if( ! uTemp ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: XGetDiskClusterSize() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	*puBlocks =  ( ( uBytes / uTemp ) + ( ( uBytes % uTemp ) ? 1 : 0 ) );

	return FSTORAGE_ERROR_NONE;

}

void fstorage_UpdateDeviceInfos( u32 *puConnected, u32 *puInserted, u32 *puRemoved, BOOL bForceUpdate ) {

	// NOTE : The bForceUpdate variable is NOT currently respected in this function.

	u32 uIndex;

	FASSERT_MSG( _bModuleInstalled, "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( puConnected,       "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puInserted,        "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puRemoved,         "[ FSTORAGE ] Error: NULL pointer !!!" );

	if( _uDevicePollDelay > ( FLoop_nRealTotalLoopTicks - _uDevicePollTimeStamp ) ) {
		// haven't waited long enough to resample
		*puConnected = _uConnected;
		*puRemoved   = 0;
		*puInserted  = 0;
		return;
	}

	_uDevicePollTimeStamp = FLoop_nRealTotalLoopTicks;

	// see what devices have been inserted or removed
	if( ! XGetDeviceChanges( XDEVICE_TYPE_MEMORY_UNIT, &_dwInserted, &_dwRemoved ) ) {
		*puConnected = _uConnected;
		*puRemoved   = 0;
		*puInserted  = 0;

		return;
	}

	_dwConnected &= ( ~ _dwRemoved );
	_dwConnected |= _dwInserted;

	_uConnected = *puConnected =	(                                 ( 1 ) |   // HD
									( ( _dwConnected <<  1 ) & ( 1 << 1 ) ) |   // 0A
									( ( _dwConnected >> 14 ) & ( 1 << 2 ) ) |   // 0B
									( ( _dwConnected <<  2 ) & ( 1 << 3 ) ) |   // 1A
									( ( _dwConnected >> 13 ) & ( 1 << 4 ) ) |   // 1B
									( ( _dwConnected <<  3 ) & ( 1 << 5 ) ) |   // 2A
									( ( _dwConnected >> 12 ) & ( 1 << 6 ) ) |   // 2B
									( ( _dwConnected <<  4 ) & ( 1 << 7 ) ) |   // 3A
									( ( _dwConnected >> 11 ) & ( 1 << 8 ) ) );  // 3B

	if( _dwRemoved ) {
		*puRemoved =	(                               ( 1 ) |   // HD
						( ( _dwRemoved <<  1 ) & ( 1 << 1 ) ) |   // 0A
						( ( _dwRemoved >> 14 ) & ( 1 << 2 ) ) |   // 0B
						( ( _dwRemoved <<  2 ) & ( 1 << 3 ) ) |   // 1A
						( ( _dwRemoved >> 13 ) & ( 1 << 4 ) ) |   // 1B
						( ( _dwRemoved <<  3 ) & ( 1 << 5 ) ) |   // 2A
						( ( _dwRemoved >> 12 ) & ( 1 << 6 ) ) |   // 2B
						( ( _dwRemoved <<  4 ) & ( 1 << 7 ) ) |   // 3A
						( ( _dwRemoved >> 11 ) & ( 1 << 8 ) ) );  // 3B
	}

	if( _dwInserted ) {
		*puInserted =	(                                ( 1 ) |   // HD
						( ( _dwInserted <<  1 ) & ( 1 << 1 ) ) |   // 0A
						( ( _dwInserted >> 14 ) & ( 1 << 2 ) ) |   // 0B
						( ( _dwInserted <<  2 ) & ( 1 << 3 ) ) |   // 1A
						( ( _dwInserted >> 13 ) & ( 1 << 4 ) ) |   // 1B
						( ( _dwInserted <<  3 ) & ( 1 << 5 ) ) |   // 2A
						( ( _dwInserted >> 12 ) & ( 1 << 6 ) ) |   // 2B
						( ( _dwInserted <<  4 ) & ( 1 << 7 ) ) |   // 3A
						( ( _dwInserted >> 11 ) & ( 1 << 8 ) ) );  // 3B
	}

	
	// update the number profiles on each attached device	
	for( uIndex = 0; uIndex < FSTORAGE_MAX_DEVICES; ++uIndex ) {
		_fstorage_UpdateNumProfiles( (s32) uIndex );
	}

	FStorage_DeviceInfo_t *pDevInfo;

	// HD.
	pDevInfo = &_aoDeviceInfos[ 0 ];
	GetDiskFreeSpaceEx( _apszDriveLetters[0], (PULARGE_INTEGER)&( pDevInfo->uBytesAvailable ), (PULARGE_INTEGER)&( pDevInfo->uBytesTotal ), NULL );
	pDevInfo->uBytesUsed = pDevInfo->uBytesTotal - pDevInfo->uBytesAvailable;

	// MC.

	// Removals.
	for( uIndex = 1; FSTORAGE_MAX_DEVICES > uIndex; ++uIndex ) {

		pDevInfo = &_aoDeviceInfos[ uIndex ];

		if( ( _dwRemoved & _auDeviceMasks[ uIndex ] ) ) {
            if( FSTORAGE_DEVICE_STATUS_AVAILABLE & pDevInfo->uStatus )  {
				XUnmountMU( _PORT_FROM_DRIVE_LETTER( *_apszDriveLetters[uIndex] ),
							_SLOT_FROM_DRIVE_LETTER( *_apszDriveLetters[uIndex] ) );
			}
			pDevInfo->uStatus = FSTORAGE_DEVICE_STATUS_NONE;
		}
	}

	// Insertions.
	u8 uChar;
	s32 nIndex;
		
	for( uIndex = 1; FSTORAGE_MAX_DEVICES > uIndex; ++uIndex ) {

		pDevInfo = &_aoDeviceInfos[ uIndex ];

		if( _dwInserted & _auDeviceMasks[ uIndex ] ) {
			pDevInfo->uStatus |= ( FSTORAGE_DEVICE_STATUS_CONNECTED | ( ( ERROR_SUCCESS == XMountMU( _PORT_FROM_DRIVE_LETTER( *_apszDriveLetters[uIndex] ), _SLOT_FROM_DRIVE_LETTER( *_apszDriveLetters[uIndex] ), (char *)&uChar ) ) ? FSTORAGE_DEVICE_STATUS_AVAILABLE : FSTORAGE_DEVICE_STATUS_NONE ) );
			if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & pDevInfo->uStatus ) ) {
				DEVPRINTF( "[ FSTORAGE ] Error %u: XMountMU() failed for device #%u !!!\n", __LINE__, uIndex );
				// clear some data...
				pDevInfo->wszName[0] = 0x0;
				pDevInfo->uBytesAvailable = 0;
				pDevInfo->uBytesTotal = 0;
				pDevInfo->uNumProfiles = 0;
				continue;
			}

			for( nIndex = 0; nIndex < FSTORAGE_MAX_NAME_LEN; nIndex++ ) {
				// XBox doesn't seem to return a null-terminated wide string name
				pDevInfo->wszName[ nIndex ] = 0;
			}

			if( ERROR_SUCCESS == XMUNameFromDriveLetter( ( 'E' + uIndex ), pDevInfo->wszName, MAX_MUNAME ) ) {
				if( ! wcslen( pDevInfo->wszName ) ) {
					_snwprintf( pDevInfo->wszName, FSTORAGE_MAX_NAME_LEN, L"MU %d%ls", ((uIndex-1)>>1)+1, ( uIndex & 1 ) ? L"A" : L"B" );
				}
			} else {
				_snwprintf( pDevInfo->wszName, FSTORAGE_MAX_NAME_LEN, L"MU %d%ls", ((uIndex-1)>>1)+1, ( uIndex & 1 ) ? L"A" : L"B" );
			}

			pDevInfo->uBytesTotal = 
				pDevInfo->uBytesAvailable = 
				pDevInfo->uBytesUsed = 0;
			GetDiskFreeSpaceEx( _apszDriveLetters[uIndex], (PULARGE_INTEGER)&( pDevInfo->uBytesAvailable ), (PULARGE_INTEGER)&( pDevInfo->uBytesTotal ), NULL );
			pDevInfo->uBytesUsed = pDevInfo->uBytesTotal - pDevInfo->uBytesAvailable;

			_fstorage_UpdateNumProfiles( (s32) uIndex );
		}
	}
}


FStorage_Error_e fstorage_FormatDevice( FStorage_DeviceID_e oeID ) {
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	DEVPRINTF( "[ FSTORAGE ] Error %u: fstorage_FormatDevice() unavailable on Xbox !!!\n", __LINE__ );

	return FSTORAGE_ERROR;

}


const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( FStorage_DeviceID_e oeID ) {
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	return &( _aoDeviceInfos[ _fstorage_DeviceIDtoIndex( oeID ) ] );
}


const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( u32 uIndex ) {
	FASSERT_MSG( _bModuleInstalled,             "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( uIndex >= 0 ,                  "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( uIndex < FSTORAGE_MAX_DEVICES, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	return &( _aoDeviceInfos[ uIndex ] );

}


FStorage_Error_e fstorage_CreateProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, u32 uSize ) {
	DWORD nError;

	FASSERT_MSG( _bModuleInstalled,                          "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                   "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                  "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( uSize,                                      "[ FSTORAGE ] Error: Zero size !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );
	FStorage_DeviceInfo_t *pDevInfo = &_aoDeviceInfos[uDeviceIndex];

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & pDevInfo->uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	u8 szString[ MAX_PATH ];

	// Open.
	nError = XCreateSaveGame( _apszDriveLetters[uDeviceIndex], pwszName, CREATE_NEW, 0, (char *)szString, MAX_PATH );

	if( nError == ERROR_ALREADY_EXISTS ) {
		// hmm, why do we get this error?

		// try opening existing
		if( ERROR_SUCCESS != XCreateSaveGame( _apszDriveLetters[uDeviceIndex], pwszName, OPEN_EXISTING, 0, (char *)szString, MAX_PATH ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: XCreateSaveGame() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			return FSTORAGE_ERROR;
		}
	} else if( nError != ERROR_SUCCESS ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: XCreateSaveGame() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	fclib_strcat( (char *)szString, "data.bin" );
	HANDLE ohFile = CreateFile( (char *)szString, GENERIC_ALL, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL );
	if( INVALID_HANDLE_VALUE == ohFile ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CreateFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		XDeleteSaveGame( _apszDriveLetters[uDeviceIndex], pwszName );
		return FSTORAGE_ERROR;
	}
	
	// Seek.
	if( INVALID_SET_FILE_POINTER == SetFilePointer( ohFile, uSize + sizeof( XCALCSIG_SIGNATURE ), NULL, FILE_BEGIN ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: SetFilePointer() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		XDeleteSaveGame( _apszDriveLetters[uDeviceIndex], pwszName );
		return FSTORAGE_ERROR;
	}
	
	// Expand.
	if( ! SetEndOfFile( ohFile ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: SetEndOfFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		XDeleteSaveGame( _apszDriveLetters[uDeviceIndex], pwszName );
		return FSTORAGE_ERROR;
	}

	CloseHandle( ohFile );

	// update the number of profiles on this device
	_fstorage_UpdateNumProfiles( uDeviceIndex );

	// update the amount of memory that this device has available
	GetDiskFreeSpaceEx( _apszDriveLetters[uDeviceIndex], (PULARGE_INTEGER)&( pDevInfo->uBytesAvailable ), (PULARGE_INTEGER)&( pDevInfo->uBytesTotal ), NULL );
	pDevInfo->uBytesUsed = pDevInfo->uBytesTotal - pDevInfo->uBytesAvailable;

	return FSTORAGE_ERROR_NONE;
}


FStorage_Error_e fstorage_DeleteProfile( FStorage_DeviceID_e oeID, const u16 *pwszName ) {
	FASSERT_MSG( _bModuleInstalled,                          "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                   "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                  "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );
	FStorage_DeviceInfo_t *pDevInfo = &_aoDeviceInfos[uDeviceIndex];

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & pDevInfo->uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	if( ERROR_SUCCESS != XDeleteSaveGame( _apszDriveLetters[uDeviceIndex], pwszName ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: XDeleteSaveGame() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	// update the number of profiles on this device
	_fstorage_UpdateNumProfiles( uDeviceIndex );

	// update the amount of memory that this device has available
	GetDiskFreeSpaceEx( _apszDriveLetters[uDeviceIndex], (PULARGE_INTEGER)&( pDevInfo->uBytesAvailable ), (PULARGE_INTEGER)&( pDevInfo->uBytesTotal ), NULL );
	pDevInfo->uBytesUsed = pDevInfo->uBytesTotal - pDevInfo->uBytesAvailable;

	return FSTORAGE_ERROR_NONE;
}


FStorage_Error_e fstorage_ValidateProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, BOOL bUpdateSignature, BOOL *pbIsValid/*= NULL*/ ) {
	FASSERT_MSG( _bModuleInstalled,                          "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                   "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                  "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );

	if( pbIsValid ) {
		*pbIsValid = FALSE;
	}

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _aoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	// Open.
	u8 szString[ MAX_PATH ];
	if( ERROR_SUCCESS != XCreateSaveGame( _apszDriveLetters[uDeviceIndex], pwszName, OPEN_EXISTING, 0, (char *)szString, MAX_PATH ) ) {
//		DEVPRINTF( "[ FSTORAGE ] Error %u: XCreateSaveGame() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	fclib_strcat( (char *)szString, "data.bin" );
	HANDLE ohFile = CreateFile( (char *)szString, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL );
	if( INVALID_HANDLE_VALUE == ohFile ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CreateFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	// Size.
	u32 uIndex = GetFileSize( ohFile, NULL );
	if( -1 == uIndex ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: GetFileSize() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		return FSTORAGE_ERROR;
	}

	if( sizeof( XCALCSIG_SIGNATURE ) >= uIndex ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: File size not adequate for operation on device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		return FSTORAGE_ERROR;
	}

	// Seek.
	if( INVALID_SET_FILE_POINTER == SetFilePointer( ohFile, sizeof( XCALCSIG_SIGNATURE ), NULL, FILE_BEGIN ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: SetFilePointer() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		return FSTORAGE_ERROR;
	}

	// Calc signature.
	HANDLE ohTemp = XCalculateSignatureBegin( 0 );
	if( INVALID_HANDLE_VALUE == ohTemp ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: XCalculateSignatureBegin() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		return FSTORAGE_ERROR;
	}

	uIndex -= sizeof( XCALCSIG_SIGNATURE );

	u32 uTemp;
	XCALCSIG_SIGNATURE oSignature;
	u8 auBuff[ 8 * 1024 ];

	do {
		if( ! ReadFile( ohFile, auBuff, sizeof( auBuff ), (LPDWORD)&uTemp, NULL ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: ReadFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			XCalculateSignatureEnd( ohTemp, &oSignature );
			CloseHandle( ohFile );
			return FSTORAGE_ERROR;
		}

		XCalculateSignatureUpdate( ohTemp, auBuff, uTemp );
		uIndex -= uTemp;
	} while( uIndex );

	XCalculateSignatureEnd( ohTemp, &oSignature );

	if( bUpdateSignature ) {
		if( INVALID_SET_FILE_POINTER == SetFilePointer( ohFile, 0, NULL, FILE_BEGIN ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: SetFilePointer() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CloseHandle( ohFile );
			return FSTORAGE_ERROR;
		}

		if( ! WriteFile( ohFile, &oSignature, sizeof( oSignature ), (LPDWORD)&uTemp, NULL ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: WriteFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CloseHandle( ohFile );
			return FSTORAGE_ERROR;
		}

		if( pbIsValid ) {
			*pbIsValid = TRUE;
		}
	} else if( pbIsValid ) {
		if( INVALID_SET_FILE_POINTER == SetFilePointer( ohFile, 0, NULL, FILE_BEGIN ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: SetFilePointer() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CloseHandle( ohFile );
			return FSTORAGE_ERROR;
		}

		XCALCSIG_SIGNATURE oSignFile;

		if( ! ReadFile( ohFile, &oSignFile, sizeof( oSignFile ), (LPDWORD)&uTemp, NULL ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: ReadFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CloseHandle( ohFile );
			return FSTORAGE_ERROR;
		}

		if( ! memcmp( &oSignature, &oSignFile, sizeof( oSignFile ) ) ) {
			*pbIsValid = TRUE;
		} else {
			DEVPRINTF( "[ FSTORAGE ] Error %u: Signature doesn't match file on device #%u !!!\n", __LINE__, uDeviceIndex );
			*pbIsValid = FALSE;
		}
	}

	CloseHandle( ohFile );

	return FSTORAGE_ERROR_NONE;

}


FStorage_Error_e fstorage_GetProfileInfos( FStorage_DeviceID_e oeID, 
										  FStorage_ProfileInfo_t *paoProfile,
										  u32 uNumProfiles,
										  u32 *puNumProfilesReturned,
										  u32 nStartOffset/*=0*/ ) {
	FASSERT_MSG( _bModuleInstalled,                   "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( paoProfile,                          "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uNumProfiles,                        "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( puNumProfilesReturned,               "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puNumProfilesReturned = 0;

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );
	FStorage_DeviceInfo_t *pDevInfo = &_aoDeviceInfos[ uDeviceIndex ];

	if( !(pDevInfo->uStatus & FSTORAGE_DEVICE_STATUS_AVAILABLE) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR_DEVICE_UNAVAILABLE;
	}
	if( !pDevInfo->uNumProfiles ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u has no profiles !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR_NO_PROFILES;
	}
	if( nStartOffset >= pDevInfo->uNumProfiles ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u doesn't have the specified offset value !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR_NO_PROFILES;
	}
	
	u32 nIndex = 0;
	XGAME_FIND_DATA oFindData;
	HANDLE ohFile = XFindFirstSaveGame( _apszDriveLetters[uDeviceIndex], &oFindData );
	if( ohFile != INVALID_HANDLE_VALUE ) {
		do {
			if( nIndex >= nStartOffset ) {
				// record the name and size of this profile
				wcscpy( paoProfile[ *puNumProfilesReturned ].wszName, oFindData.szSaveGameName );
				paoProfile[ *puNumProfilesReturned ].uBytesTotal = oFindData.wfd.nFileSizeLow;

				if( ++( *puNumProfilesReturned ) >= uNumProfiles ) {
					break;
				}
			}
			nIndex++;
			if( nIndex >= pDevInfo->uNumProfiles ) {
				break;
			}
		} while( XFindNextSaveGame( ohFile, &oFindData ) );

		XFindClose( ohFile );
	}
	
	return FSTORAGE_ERROR_NONE;
}


FStorage_Error_e fstorage_SetProfileIcon( FStorage_DeviceID_e oeID, const u16 *pwszName, void *puIcon, u32 uIconSize ) {
	FASSERT_MSG( _bModuleInstalled,                          "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE     != oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_XB_MC_3B >= oeID,        "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                   "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                  "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN > wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( puIcon,                                     "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uIconSize,                                  "[ FSTORAGE ] Error: Zero size !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _aoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	// Open.
	u8 szString[ MAX_PATH ];
	if( ERROR_SUCCESS != XCreateSaveGame( _apszDriveLetters[uDeviceIndex], pwszName, CREATE_NEW, 0, (char *)szString, MAX_PATH ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: XCreateSaveGame() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	fclib_strcat( (char *)szString, "SaveImage.xbx" );
	HANDLE ohFile = CreateFile( (char *)szString, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL );
	if( INVALID_HANDLE_VALUE == ohFile ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CreateFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	// Write.
	u32 uTemp;

	if( ! WriteFile( ohFile, puIcon, uIconSize, (LPDWORD)&uTemp, NULL ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: WriteFile() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CloseHandle( ohFile );
		DeleteFile( (char *)szString );
		return FSTORAGE_ERROR;
	}

	CloseHandle( ohFile );

	return FSTORAGE_ERROR_NONE;

}


FStorage_Error_e fstorage_ReadProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, u32 uPosition, void *puBuff, u32 uBuffSize ) {
	return _fstorage_ReadWriteProfile( TRUE, oeID, pwszName, uPosition, puBuff, uBuffSize );
}


FStorage_Error_e fstorage_WriteProfile( FStorage_DeviceID_e oeID, const u16 *pwszName, u32 uPosition, void *puBuff, u32 uBuffSize ) {
	return _fstorage_ReadWriteProfile( FALSE, oeID, pwszName, uPosition, puBuff, uBuffSize );
}