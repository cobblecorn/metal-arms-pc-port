//////////////////////////////////////////////////////////////////////////////////////
// fstorage.h - Fang storage (memory unit / harddisk).
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
// 06/29/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef _FSTORAGE_H_
#define _FSTORAGE_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
Notes on using FStorage:

- First call should be fstorage_ModuleStartup().
- Last call should be fstorage_ModuleShutdown().
- Nothing can be called until fstorage_ModuleStartup() is called or after fstorage_ModuleShutdown() is called.

- First call after fstorage_Install() should be fstorage_UpdateDeviceInfos().

- The rest should be pretty self-explanatory, otherwise please consult the author and current maintainer:

Albert Yale
ayale@swinginape.com
949-716-8209 x 206
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define FSTORAGE_MAX_NAME_LEN		32

#if FANG_PLATFORM_XB
	#define FSTORAGE_MAX_DEVICES	9
#elif FANG_PLATFORM_GC
	#define FSTORAGE_MAX_DEVICES	2
#else
	#define FSTORAGE_MAX_DEVICES	1
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum
{
	FSTORAGE_DEVICE_STATUS_CONNECTED			=	0x00000001,	// ( 1 << 0 )
	FSTORAGE_DEVICE_STATUS_AVAILABLE			=	0x00000002,	// ( 1 << 1 )

	FSTORAGE_DEVICE_STATUS_BROKEN				=	0x00000004,	// ( 1 << 2 )
	FSTORAGE_DEVICE_STATUS_CORRUPTED			=	0x00000008,	// ( 1 << 3 )
	FSTORAGE_DEVICE_STATUS_UNSUPPORTED			=	0x00000010,	// ( 1 << 4 )
	FSTORAGE_DEVICE_STATUS_UNFORMATED			=	0x00000020,	// ( 1 << 5 )
	FSTORAGE_DEVICE_STATUS_WRONG_ENCODING		=	0x00000040,	// ( 1 << 6 )
	FSTORAGE_DEVICE_STATUS_NO_FREE_MEM			=	0x00000080,	// ( 1 << 7 )
	FSTORAGE_DEVICE_STATUS_WRONG_DEVICE			=	0x00000100,	// ( 1 << 8 )
	FSTORAGE_DEVICE_STATUS_DAMAGED				= 	0x00000200, // ( 1 << 9 )
	FSTORAGE_DEVICE_STATUS_NO_FREE_FILE_ENTRIES = 	0x00000400, // ( 1 << 10 )
	
	// handy mask combinations
	FSTORAGE_DEVICE_READY_FOR_USE			= (FSTORAGE_DEVICE_STATUS_CONNECTED | FSTORAGE_DEVICE_STATUS_AVAILABLE),

	FSTORAGE_DEVICE_STATUS_NONE				=	0x00000000
} FStorage_DeviceStatus_e;

typedef enum
{
	FSTORAGE_DEVICE_ID_NONE					=	0x00000000,

	//// PC and XB.
	FSTORAGE_DEVICE_ID_XB_PC_HD				=	0x00000001,	// ( 1 << 0 )

	//// XB.
	FSTORAGE_DEVICE_ID_XB_MC_0A				=	0x00000002,	// ( 1 << 1 )
	FSTORAGE_DEVICE_ID_XB_MC_0B				=	0x00000004,	// ( 1 << 2 )
	FSTORAGE_DEVICE_ID_XB_MC_1A				=	0x00000008,	// ( 1 << 3 )
	FSTORAGE_DEVICE_ID_XB_MC_1B				=	0x00000010,	// ( 1 << 4 )
	FSTORAGE_DEVICE_ID_XB_MC_2A				=	0x00000020,	// ( 1 << 5 )
	FSTORAGE_DEVICE_ID_XB_MC_2B				=	0x00000040,	// ( 1 << 6 )
	FSTORAGE_DEVICE_ID_XB_MC_3A				=	0x00000080,	// ( 1 << 7 )
	FSTORAGE_DEVICE_ID_XB_MC_3B				=	0x00000100,	// ( 1 << 8 )

	//// GC.
	FSTORAGE_DEVICE_ID_GC_MC_LEFT			=	0x00000001,	// ( 1 << 0 )
	FSTORAGE_DEVICE_ID_GC_MC_RIGHT			=	0x00000002,	// ( 1 << 1 )

	// CPS Added 5.20.03 -->  
	//// PS2.
	FSTORAGE_DEVICE_ID_PS2_MC_LEFT			=	0x00000001,	// ( 1 << 0 )
	FSTORAGE_DEVICE_ID_PS2_MC_RIGHT			=	0x00000002	// ( 1 << 1 )
	// <-- CPS Added 5.20.03
	
} FStorage_DeviceID_e;

typedef enum
{
	FSTORAGE_ERROR_NONE						=	 0,
	FSTORAGE_ERROR							=	-1,

	FSTORAGE_ERROR_BROKEN					=	-2,
	FSTORAGE_ERROR_CORRUPTED				=	-3,
	FSTORAGE_ERROR_UNSUPPORTED				=	-4,
	FSTORAGE_ERROR_UNFORMATED				=	-5,
	FSTORAGE_ERROR_WRONG_ENCODING			=	-6,
	FSTORAGE_ERROR_NO_FREE_MEM				=	-7,
	FSTORAGE_ERROR_NO_PROFILES				=	-8,
	FSTORAGE_ERROR_DEVICE_UNAVAILABLE		=   -9,

/*
no more file system entries available on device?
no space left on device?
[...]
*/

} FStorage_Error_e;

typedef struct
{
	wchar wszName[ FSTORAGE_MAX_NAME_LEN ]; // Unicode.
	u64 uBytesTotal, uBytesAvailable, uBytesUsed;
	FStorage_DeviceID_e oeID;
	u32 uStatus; // See FStorage_DeviceStatus_e.
	u32 uNumProfiles;

} FStorage_DeviceInfo_t;

typedef struct
{
	wchar wszName[ FSTORAGE_MAX_NAME_LEN ]; // Unicode.
	u32 uBytesTotal;

} FStorage_ProfileInfo_t;

typedef struct
{
// Note: On the Xbox, the default app icon is provided through the project settings.
// Note: On the GameCube, the app icon data is loaded at fstorage_install time and persists until Uninstall.
	cchar *GCONLY_pszIconName;		// [GC]  Name of app icon data .RDG file [else] ignore.
	cchar *GCONLY_pszBannerName;	// [ GC (optional) ]  to app banner data [else] ignore.
	cchar *GCONLY_pszApplicationName; // Used for the comment field

} FStorage_Init_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//// Module.
extern BOOL fstorage_ModuleStartup( void );
extern void fstorage_ModuleShutdown( void );

extern FStorage_Error_e fstorage_Install( const FStorage_Init_t *poInit = NULL ); // if GC: poInit can't be NULL.
extern void fstorage_Uninstall( void );
extern BOOL fstorage_IsInstalled( void );

extern FStorage_Error_e fstorage_CalcSaveGameSize( FStorage_DeviceID_e oeID, u32 uDataBytes, u32 *puTotalBytes );
extern FStorage_Error_e fstorage_BytesToBlocks( FStorage_DeviceID_e oeID, u32 uBytes, u32 *puBlocks ); //
extern void fstorage_UpdateDeviceInfos( u32 *puConnected, u32 *puInserted, u32 *puRemoved, BOOL bForceUpdate = FALSE ); // See FStorage_DeviceID_e.
extern FStorage_Error_e fstorage_FormatDevice( FStorage_DeviceID_e oeID ); // GC only.
extern const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( FStorage_DeviceID_e oeID );
extern const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( u32 uIndex );

extern FStorage_Error_e fstorage_CreateProfile( FStorage_DeviceID_e oeID,
											   cwchar *pwszName,
											   u32 uSize );
extern FStorage_Error_e fstorage_DeleteProfile( FStorage_DeviceID_e oeID,
											   cwchar *pwszName );
extern FStorage_Error_e fstorage_ValidateProfile( FStorage_DeviceID_e oeID,
												 cwchar *pwszName,
												 BOOL bUpdateSignature,
												 BOOL *pbIsValid = NULL );
extern FStorage_Error_e fstorage_GetProfileInfos( FStorage_DeviceID_e oeID,
												 FStorage_ProfileInfo_t *paoProfile,
												 u32 uNumProfiles,
												 u32 *puNumProfilesReturned,
												 u32 nStartOffset=0 );
extern FStorage_Error_e fstorage_SetProfileIcon( FStorage_DeviceID_e oeID,
												cwchar *pwszName,
												void *puIcon,
												u32 uIconSize );
extern FStorage_Error_e fstorage_ReadProfile( FStorage_DeviceID_e oeID,
											 cwchar *pwszName,
											 u32 uPosition,
											 void *puBuff,
											 u32 uBuffSize );
extern FStorage_Error_e fstorage_WriteProfile( FStorage_DeviceID_e oeID,
											  cwchar *pwszName,
											  u32 uPosition,
											  void *puBuff,
											  u32 uBuffSize );

#if FANG_WINGC
// PC local co-op campaigns: one whole file per name (player 1's profile name) in the Co-op folder
// beside the profiles. Read returns FALSE when there is no such save or it is larger than uBuffSize.
extern BOOL fstorage_PcCoopRead( cwchar *pwszName, void *puBuff, u32 uBuffSize, u32 *puBytesRead );
extern BOOL fstorage_PcCoopWrite( cwchar *pwszName, const void *puBuff, u32 uBuffSize );
#endif

// Determines if it's possible to save a new profile on a memory device...
// If FSTORAGE_ERROR is returned, then there was an error trying to determine if the card can be used
//    and puRetStatus is meaningless.
// if FSTORAGE_ERROR_NONE is returned, then puRetStatus will contain some of the following flags:
//		FSTORAGE_DEVICE_READY_FOR_USE : Indicates that the memory status CAN be saved
//		FSTORAGE_DEVICE_STATUS_NO_FREE_MEM : Not enough memory files on the card
//		FSTORAGE_DEVICE_STATUS_NO_FREE_FILE_ENTRIES : No free file entries on the card
extern FStorage_Error_e fstorage_CanSaveNewProfile( FStorage_DeviceID_e oeID, u32 uBytes, u32 *puRetStatus );

FINLINE FStorage_DeviceID_e fstorage_IndexToDeviceID( u32 uIndex )
{
	FASSERT_MSG( uIndex < FSTORAGE_MAX_DEVICES, "[ FSTORAGE ] Error: Invalid device Index" );
	switch( uIndex )
	{
		case 0:
			return FSTORAGE_DEVICE_ID_XB_PC_HD;
		case 1:
			return FSTORAGE_DEVICE_ID_XB_MC_0A;
		case 2:
			return FSTORAGE_DEVICE_ID_XB_MC_0B;
		case 3:
			return FSTORAGE_DEVICE_ID_XB_MC_1A;
		case 4:
			return FSTORAGE_DEVICE_ID_XB_MC_1B;
		case 5:
			return FSTORAGE_DEVICE_ID_XB_MC_2A;
		case 6:
			return FSTORAGE_DEVICE_ID_XB_MC_2B;
		case 7:
			return FSTORAGE_DEVICE_ID_XB_MC_3A;
		case 8:
			return FSTORAGE_DEVICE_ID_XB_MC_3B;
		default:
			FASSERT_NOW;
	}
	return FSTORAGE_DEVICE_ID_NONE;
}

FINLINE s32 fStorage_DeviceIDToIndex( FStorage_DeviceID_e device ) {
	//FASSERT_MSG( device < FSTORAGE_MAX_DEVICES, "[ FSTORAGE ] Error: Invalid device Index" );
	switch( device )
	{
	case FSTORAGE_DEVICE_ID_XB_PC_HD:
		return 0;
	case FSTORAGE_DEVICE_ID_XB_MC_0A:
		return 1;
	case FSTORAGE_DEVICE_ID_XB_MC_0B:
		return 2;
	case FSTORAGE_DEVICE_ID_XB_MC_1A:
		return 3;
	case FSTORAGE_DEVICE_ID_XB_MC_1B:
		return 4;
	case FSTORAGE_DEVICE_ID_XB_MC_2A:
		return 5;
	case FSTORAGE_DEVICE_ID_XB_MC_2B:
		return 6;
	case FSTORAGE_DEVICE_ID_XB_MC_3A:
		return 7;
	case FSTORAGE_DEVICE_ID_XB_MC_3B:
		return 8;
	default:
		return -1;
	}
	
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // _FSTORAGE_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
