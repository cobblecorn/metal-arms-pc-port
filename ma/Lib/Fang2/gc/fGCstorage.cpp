//////////////////////////////////////////////////////////////////////////////////////
// fgcstorage.cpp - 
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
// 02/10/02 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <dolphin.h>
#include <wchar.h>

#include <texpalette.h>

#include "fang.h"
#include "fgc.h"
#include "fmath.h"
#include "fres.h"
#include "fclib.h"
#include "floop.h"
#include "fstorage.h"
#include "ffile.h"
#include "fclib.h"

#define _DEVICE_POLL_DELAY					( 0.25f ) //  4 fps.

#define _SUPPORTED_SECTOR_SIZE_IN_BYTES		( 8 * 1024 )
#define _SLOT_NAME_CSV_FILENAME				"fgcstorage$"
#define _SLOT_NAME_TABLE_NAME				"slotnames"
#define _APPLICATION_STRING_SIZE			( CARD_COMMENT_SIZE >> 1 )

typedef enum {
	_TPL_LOAD_ICON_MODE,
	_TPL_LOAD_BANNER_MODE
} _TPLLoadMode_e;

typedef struct {
////////////////////////////////////////////////////////////
// THESE FIELDS MUST BE IN THE FIRST 512 BYTES OF THE FILE
	u32 uCrc;// CRC of the stored data
	u32 uPayloadAndHeaderSize;// total size of header plus stored data
	u8 auComment[ CARD_COMMENT_SIZE ];// REQUIRED
///////////////////////////////////////////////////////////////	
	
//////////////////////////////////////////////////////////////////////////////////
// the banner data and CLUT must be immediately followed by the Icon data and CLUT
	u8 auBannerC8andCLUT[ ( 96 * 32 ) + ( 2 * 256 ) ];
	u8 auIconC8andCLUT[ ( 32 * 32 ) + ( 2 * 256 ) ];
	
} _FileHeader_t;

///////////////
// private vars

// Module.
static BOOL _bModuleStarted = FALSE;
static BOOL _bModuleInstalled = FALSE;

// Application Name
static char _szApplicationName[ _APPLICATION_STRING_SIZE + 1 ]; // This will the the first line on the comment

// Devices.
static u32 _uConnected, _uInserted, _auSectorSizes[ FSTORAGE_MAX_DEVICES ];
static volatile u32 _uRemoved;
static FStorage_DeviceInfo_t *_paoDeviceInfos;
static u64 _uDevicePollDelay, _uDevicePollTimeStamp;

// Icons.
static u32 _uIconSize, _uBannerSize;
static void *_puIcon, *_puBanner;

// Buffers.
static u8 *_pauTempBuff;
static u8 *_paauCardWorkArea;// work area for the mount call

///////////////
// Prototypes.
FINLINE u32 _fstorage_DeviceIDtoIndex( FStorage_DeviceID_e oeID );
FINLINE void _fstorage_ConvertCommentToName( const u8 *puSource, u16 *puDestination );
FINLINE void _fstorage_ConvertNameToComment( cwchar *puSource, u8 *puDestination );
FINLINE void _fstorage_UnmountCallback( s32 nPort, s32 nResult );
static void _fstorage_UpdateNumProfiles( s32 nDeviceID );
static BOOL _fstorage_LoadTPLFile( cchar *pszFilename, void** ppRawData, u32 *puDataSize, _TPLLoadMode_e eLoadMode );
static BOOL _fstorage_LoadSlotNameCSV( void );

FINLINE u32 _fstorage_DeviceIDtoIndex( FStorage_DeviceID_e oeID ) {
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	return (u32)oeID - 1;

}


// puDestination must point to FSTORAGE_MAX_NAME_LEN * sizeof( u16 ) bytes
FINLINE void _fstorage_ConvertCommentToName( const u8 *puSource, u16 *puDestination ) {
	FASSERT_MSG( puSource,      "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puDestination, "[ FSTORAGE ] Error: NULL pointer !!!" );

	// zero out our destination memory
	fang_MemZero( puDestination, ( sizeof( u16 ) * FSTORAGE_MAX_NAME_LEN ) );

	// NEW SYSTEM
	// Skip past the application name....
	cchar* pPlayerName = (cchar*) &puSource[ _APPLICATION_STRING_SIZE ];

	// get the length of the src string
	u32 nSrcLen = fclib_strlen( pPlayerName );	
	FMATH_CLAMPMAX( nSrcLen, (FSTORAGE_MAX_NAME_LEN-1) );

	// copy the src string to the destination string
	u32 i;
	for( i=0; i < nSrcLen; i++ ) {
		puDestination[i] = (u16)pPlayerName[i];
	}
}


// puDestination must point FSTORAGE_MAX_NAME_LEN bytes
FINLINE void _fstorage_ConvertNameToComment( cwchar *puSource, u8 *puDestination ) {
	FASSERT_MSG( puSource,      "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puDestination, "[ FSTORAGE ] Error: NULL pointer !!!" );

	// zero out the destination memory
    fang_MemZero( puDestination, CARD_COMMENT_SIZE );

	// get the length of the src string
	u32 nSrcLen = fclib_wcslen( puSource );
	FMATH_CLAMPMAX( nSrcLen, (FSTORAGE_MAX_NAME_LEN-1) );

	sprintf( (char*)puDestination, "%s", _szApplicationName );

	char *pDestination = (char*) &puDestination[ fclib_strlen( (cchar*)puDestination ) ];
	// copy the src string to the destination string
	u32 i;
	for( i=0; i < nSrcLen; i++ ) {
		pDestination[i] = (u8)puSource[i];
	}
}


static void _fstorage_UpdateNumProfiles( s32 nDeviceID ) {
	CARDStat CardStats;
	u32 uCount;
	s32 nRes;

	// grab the dvd disk id so that we can compare to the file stats info
	DVDDiskID *pDiskID = DVDGetCurrentDiskID();
	FASSERT( pDiskID );

	_paoDeviceInfos[nDeviceID].uNumProfiles = 0;

	// walk all of the possible files on the card
	for( uCount = 0; uCount < CARD_MAX_FILE; uCount++ ) {
		// get file status of the uCount'th file
		nRes = CARDGetStatus( nDeviceID, uCount, &CardStats );
		if( nRes == CARD_RESULT_READY &&
			fang_IsMemIdentical( pDiskID->gameName, CardStats.gameName, sizeof( pDiskID->gameName ) ) &&
			fang_IsMemIdentical( pDiskID->company, CardStats.company, sizeof( pDiskID->company ) ) ) {
			// this profile is one from our application, count it
			_paoDeviceInfos[nDeviceID].uNumProfiles++;
		}
	}
}


BOOL fstorage_ModuleStartup( void ) {
	FASSERT_MSG( ! _bModuleStarted, "[ FSTORAGE ] Error: System already started !!!" );

	CARDInit();

	_bModuleStarted = TRUE;
	_bModuleInstalled = FALSE;

	return TRUE;

}


void fstorage_ModuleShutdown( void ) {
	fstorage_Uninstall();

	_bModuleStarted = FALSE;

}


FStorage_Error_e fstorage_Install( const FStorage_Init_t *poInit /* = NULL */ ) {
	FASSERT_MSG( _bModuleStarted,                                                                                   "[ FSTORAGE ] Error: System not started !!!" );
	FASSERT_MSG( ! _bModuleInstalled,                                                                               "[ FSTORAGE ] Error: System already installed !!!" );
	FASSERT_MSG( poInit,                                                                                            "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( poInit->GCONLY_pszIconName,                                                                        "[ FSTORAGE ] Error: NULL pointer !!!" );
//	FASSERT_MSG( poInit->GCONLY_puBanner,                                                                           "[ FSTORAGE ] Error: NULL pointer !!!" );

	FResFrame_t oResFrame = fres_GetFrame();
	
	// allocate our system memory
	_paoDeviceInfos   = (FStorage_DeviceInfo_t *)fres_AlignedAllocAndZero( ( FSTORAGE_MAX_DEVICES * sizeof( FStorage_DeviceInfo_t ) ), 32 );
	_pauTempBuff      = (u8 *)fres_AlignedAlloc( ( 2 * _SUPPORTED_SECTOR_SIZE_IN_BYTES ), 32 );
	_paauCardWorkArea = (u8 *)fres_AlignedAlloc( FSTORAGE_MAX_DEVICES * CARD_WORKAREA_SIZE, 32 );
	if( ! ( _paoDeviceInfos && _pauTempBuff && _paauCardWorkArea ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: fres_AlignedAlloc() failed !!!\n", __LINE__ );
		fres_ReleaseFrame( oResFrame );
		return FSTORAGE_ERROR;
	}

	// get the length of the passed in Application Name String
	u32 nSrcLen = fclib_strlen( poInit->GCONLY_pszApplicationName );	
	FMATH_CLAMPMAX( nSrcLen, _APPLICATION_STRING_SIZE );
	// copy the provided application name string to the storage application name string
	u32 i;
	for( i=0; i < nSrcLen; i++ ) {
		_szApplicationName[i] = poInit->GCONLY_pszApplicationName[i];
	}

	// Now, copy in spaces for the rest of the name (because this name will appear on the first comment line)
	for( i=nSrcLen; i < _APPLICATION_STRING_SIZE; i++ ) {
		_szApplicationName[i] = ' ';
	}
	_szApplicationName[	_APPLICATION_STRING_SIZE ] = 0x00; // NULL TERMINATE the application string name
	
	// Set these names up as default -- We will attempt to load localized names for the cards shortly...
    wcscpy( _paoDeviceInfos[ 0 ].wszName, L"Left - Slot A" );//GameCube Memory Card A" );
	_paoDeviceInfos[ 0 ].oeID = FSTORAGE_DEVICE_ID_GC_MC_LEFT;

	wcscpy( _paoDeviceInfos[ 1 ].wszName, L"Right - Slot B" );//GameCube Memory Card B" );
	_paoDeviceInfos[ 1 ].oeID = FSTORAGE_DEVICE_ID_GC_MC_RIGHT;

	if( !_fstorage_LoadSlotNameCSV() ) {
		DEVPRINTF( "[ FSTORAGE ] WARNING: Could not load the Localized SlotNames CSV File %s -- Will proceede with NON-LOCALIZED slot names!\n", _SLOT_NAME_CSV_FILENAME );
	}
	
	// Load the icon and (optional) banner information here
	if( !_fstorage_LoadTPLFile( poInit->GCONLY_pszIconName, &_puIcon, &_uIconSize, _TPL_LOAD_ICON_MODE ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Could not load the SaveGame Icon!\n", __LINE__ );
		return FSTORAGE_ERROR;
	}

	if( poInit->GCONLY_pszBannerName && 
	    !_fstorage_LoadTPLFile( poInit->GCONLY_pszBannerName, &_puBanner, &_uBannerSize, _TPL_LOAD_BANNER_MODE ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Could not load the SaveGame Icon!\n", __LINE__ );
		return FSTORAGE_ERROR;
	}

	_uConnected = _uInserted = _uRemoved = 0;

	_uDevicePollDelay     = (u64)( _DEVICE_POLL_DELAY * FLoop_fTicksPerSec );
	_uDevicePollTimeStamp = 0;

	_bModuleInstalled = TRUE;

	return FSTORAGE_ERROR_NONE;

}

void fstorage_Uninstall( void ) {

	if( ! _bModuleInstalled ) {
		return;
	}

	CARDUnmount( 0 );
	CARDUnmount( 1 );

	_bModuleInstalled = FALSE;

}


BOOL fstorage_IsInstalled( void ) {
	return _bModuleInstalled;

}


// This routine will load a CSV (if its present) that will contain the localized names
// of the memory cards.  If this CSV is not present, the system will use the default
// non localized file names.
static BOOL _fstorage_LoadSlotNameCSV( void ) {

	FMemFrame_t hMemFrame;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hTable;
	FGameData_VarType_e nDataType;
	
	u32 nNumEntriesInTable;

	cwchar *pwszSlotAName;
	cwchar *pwszSlotBName;
			
	// grab an fmem frame
	hMemFrame = fmem_GetFrame();

	//////////////////////////////////////////////////
	// load the csv file to temp memory (fmem)
	hFile = fgamedata_LoadFileToFMem( _SLOT_NAME_CSV_FILENAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "[ FSTORAGE ] NOTICE: Could not find filename %s containing localized Memory card slot names!!!\n", _SLOT_NAME_CSV_FILENAME );
		goto _ExitWithError;
	}
	
	// Load the crate groups table
	hTable = fgamedata_GetFirstTableHandle( hFile, _SLOT_NAME_TABLE_NAME );
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "[ FSTORAGE ] NOTICE: Could not find tablename %s containing localized Memory card slot names!!!\n", _SLOT_NAME_TABLE_NAME );
		goto _ExitWithError;
	}

	// Get the number of fields (which translates to the number of crate names) in the table
	nNumEntriesInTable = fgamedata_GetNumFields( hTable );
	if( nNumEntriesInTable != 2 ) {
		DEVPRINTF( "[ FSTORAGE ] NOTICE: This table %s expects EXACTLY two entries.  Found %d entries!\n", _SLOT_NAME_TABLE_NAME, nNumEntriesInTable );
		goto _ExitWithError;
	}

	// Now, read in the name of card slot A.
	pwszSlotAName = (cwchar *)fgamedata_GetPtrToFieldData( hTable, 0, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING)
	{
		DEVPRINTF("[ FSTORAGE ] NOTICE: Slot Name A in table %s is not a WIDESTRING format!\n", _SLOT_NAME_TABLE_NAME );
		goto _ExitWithError;
	}

	// Now, read in the name of card slot B.
	pwszSlotBName = (cwchar *)fgamedata_GetPtrToFieldData( hTable, 1, nDataType );
	if(nDataType != FGAMEDATA_VAR_TYPE_WIDESTRING)
	{
		DEVPRINTF("[ FSTORAGE ] NOTICE: Slot Name B in table %s is not a WIDESTRING format!\n", _SLOT_NAME_TABLE_NAME );
		goto _ExitWithError;
	}
	
	// If we are here, then we successfully parsed the CSV, we just need to exit out
    wcscpy( _paoDeviceInfos[ 0 ].wszName, pwszSlotAName );
	wcscpy( _paoDeviceInfos[ 1 ].wszName, pwszSlotBName );
	
	fmem_ReleaseFrame( hMemFrame );
	return TRUE;
		
	
_ExitWithError:
	fmem_ReleaseFrame( hMemFrame );
	
	return FALSE;
}

// This routine will load a TPL file thats encoded into the Master file as a an .RDG file.
// A TPL File is a gamecube specific file format for storing texture / animation data.
static BOOL _fstorage_LoadTPLFile( cchar *pszFilename, void** ppRawData, u32 *puDataSize, _TPLLoadMode_e eLoadMode ) {

	FFileHandle ohFile = FFILE_INVALID_HANDLE;
	char szFilename[ 32 ];

	// First, load the RDG file...
	if( !pszFilename ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Null filename passed in !!!\n", __LINE__ );
		return FALSE;
	}
	
	fclib_strcpy( szFilename, pszFilename );
	char* pszRDG = fclib_strchr( pszFilename, '.' );
	if( pszRDG ) {
		fclib_strcpy( pszRDG, ".rdg" );
	} else {
		fclib_strcat( szFilename, ".rdg" );
	}
	
	ohFile = ffile_Open( szFilename, FFILE_OPEN_RONLY_SEQUENTIAL, FALSE );

	if( FFILE_INVALID_HANDLE == ohFile )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: Could not open %s file !!!\n", __LINE__, szFilename );
		return FALSE;
	}

	u32 uSize = (u32)ffile_GetFileSize( ohFile );
	if( ! uSize )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: %s file invalid !!!\n", szFilename );
		ffile_Close( ohFile );
		return FALSE;
	}

	FMemFrame_t hMemFrame = fmem_GetFrame();
	
	// now, grab a temporary memory pointer to load this data into...
	void *pTempData = fmem_Alloc( uSize );
	if( ! pTempData )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: fmem_Alloc() failed !!!\n", __LINE__ );
		ffile_Close( ohFile );
		return FALSE;
	}

	if( uSize != ffile_Read( ohFile, uSize, pTempData ) )
	{
		DEVPRINTF( "[ FSTORAGE ] Error %u: ffile_Read() failed !!!\n", __LINE__ );
		ffile_Close( ohFile );
		fmem_ReleaseFrame( hMemFrame );
		return FALSE;
	}

	ffile_Close( ohFile );
	
	#define __FIXUP_POINTER( pointer, type )	if (pointer) pointer = (type *)((u32)pTempData + (u32)pointer)
	
	// Now, parse the loaded file...
	TEXPalette *pPaletteHeader = ( TEXPalette* ) pTempData;
	__FIXUP_POINTER( pPaletteHeader->descriptorArray, TEXDescriptor );

	// First, ensure that it's a a .TPL File.
	if(pPaletteHeader->versionNumber != 0x0020AF30 ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: %s is not a valid .TPL file! !!!\n", __LINE__, szFilename );
		fmem_ReleaseFrame( hMemFrame );
		return FALSE;
	}	
	
	// Check the load mode... If this is a banner file, it should only have one descriptor
	if( ( eLoadMode == _TPL_LOAD_BANNER_MODE ) && ( pPaletteHeader->numDescriptors != 1 ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: %s does not have ONLY 1 texture defined in it!  Bannners can only have 1 texture !!!\n", __LINE__, szFilename );
		fmem_ReleaseFrame( hMemFrame );
		return FALSE;
	}

	u32 i;
	// Loop through the TPL file structure and
	//   1) Fix up all pointers
	//   2) Verify the texture format is correct for banner and icon files
	//   3) Verify that if it's a multiple texture animation, we use the same tex format for every frame
	//	 4) Verify the texture size is correct for what we are loading (icon or banner)
	//   5) Figure out how much memory we need to allocate
	u32 nFormat = 0xFFFFFFFF;
	u32 nTotalMem = 0;
	for( i = 0; i < pPaletteHeader->numDescriptors; i++ ) {

		// Get Descriptor from the descriptor array
		TEXDescriptor *pTexDescriptor = &pPaletteHeader->descriptorArray[i];
		// Now, fix up the Descriptor pointers
		__FIXUP_POINTER( pTexDescriptor->textureHeader, TEXHeader );
		__FIXUP_POINTER( pTexDescriptor->CLUTHeader, CLUTHeader );
		
		TEXHeader *pTexHeader = pTexDescriptor->textureHeader;
		CLUTHeader *pCLUTHeader = pTexDescriptor->CLUTHeader;
		__FIXUP_POINTER( pTexHeader->data, char );
		__FIXUP_POINTER( pCLUTHeader->data, char );
		
		// Now, check what format this texture is...
		if( pTexHeader->format != GX_TF_RGB5A3 && pTexHeader->format != 0x09 ) { // 0x09 is the palettized format
			DEVPRINTF( "[ FSTORAGE ] Error %u: %s is not the proper texture format!  Needs to be RGB5A3 or Ci8  !!!\n", __LINE__, szFilename );
			fmem_ReleaseFrame( hMemFrame );
			return FALSE;
		}
		
		// If the CLUT pointer is valid, we should ensure that it's texture format is GX_TF_RGB5A3
		if( pCLUTHeader && ( pCLUTHeader->format != GX_TL_RGB5A3 ) ) {
			// Invalid CLUT format
			DEVPRINTF( "[ FSTORAGE ] Error %u: %s is a palettized image, but it's CLUT is not a RGB5A3 format!!!\n", __LINE__, szFilename );
			fmem_ReleaseFrame( hMemFrame );
			return FALSE;
		}
		
		// Make sure all the textures are the same format
		if( nFormat == 0xFFFFFFFF ) { // format of the first texture has not yet been defined
			// Store off the format
			nFormat = pTexHeader->format;
		} else if( nFormat != pTexHeader->format ) { // Verify that this texture is the same format as the previous ones
			DEVPRINTF( "[ FSTORAGE ] Error %u: %s has multiple texture formats stored in it!  All animation textures need to be the same format!!!\n", __LINE__, szFilename );
			fmem_ReleaseFrame( hMemFrame );
			return FALSE;
		}
		
		// Verify the texture size is correct based on what type of file we are loading
		if( eLoadMode == _TPL_LOAD_BANNER_MODE ) {
			// We are loading a banner, so it should be 96 x 32
			if( ( pTexHeader->width != 96 ) || ( pTexHeader->height != 32 ) ) {
				// Invalid banner size!
				DEVPRINTF( "[ FSTORAGE ] Error %u: %s is the wrong texture size!  Banners must be 96x32!!!\n", __LINE__, szFilename );
				fmem_ReleaseFrame( hMemFrame );
				return FALSE;
			}
		} else {
			// We are loading an icon
			if( ( pTexHeader->width != 32 ) || ( pTexHeader->height != 32 ) ) {
				// Invalid icon size!
				DEVPRINTF( "[ FSTORAGE ] Error %u: %s is the wrong texture size!  Icons must be 32x32!!!\n", __LINE__, szFilename );
				fmem_ReleaseFrame( hMemFrame );
				return FALSE;
			}
		}		
		// Keep track of how much memory we need to allocate
		if( pTexHeader->format == GX_TF_RGB5A3 ) {
			nTotalMem += pTexHeader->width * pTexHeader->height * sizeof( u16 );
		} else {
			// This is a palettized format
			nTotalMem += pTexHeader->width * pTexHeader->height;
			if( i == ( pPaletteHeader->numDescriptors - 1 ) ) {
				// Since this is a palettized format, all Textures will SHARE the same lookup table
				// Only add the memory required to store the CLUT once...
				nTotalMem += pCLUTHeader->numEntries * sizeof( u16 );
			}
		}
	}

	#undef __FIXUP_POINTER
		
	// If we are here, then we have ensured that all is well with the texture file, and it's time
	// To allocate memory and copy the texture into the permenant buffer!
	FResFrame_t hResFrame = fres_GetFrame;

	*puDataSize = nTotalMem;
	u8* pRawData = (u8*) fres_Alloc( *puDataSize );
	if( !pRawData ) {
		fres_ReleaseFrame( hResFrame );
		fmem_ReleaseFrame( hMemFrame );
		DEVPRINTF( "[ FSTORAGE ] Error %u: fres_Alloc() failed !!!\n", __LINE__ );
		return FALSE;
	}
	
	*ppRawData = pRawData;
	
	// Now, copy the data (icon first, then CLUT if it exists)
	u32 nBytesCopied = 0;
	for( i = 0; i < pPaletteHeader->numDescriptors; i++ ) {
		TEXDescriptor *pTexDescriptor = &pPaletteHeader->descriptorArray[i];
		TEXHeader *pTexHeader = pTexDescriptor->textureHeader;
		CLUTHeader *pCLUTHeader = pTexDescriptor->CLUTHeader;

		u32 nBytesToCopy;
		if( pTexHeader->format == GX_TF_RGB5A3 ) {
			nBytesToCopy = pTexHeader->width * pTexHeader->height * sizeof( u16 );
		} else {
			nBytesToCopy = pTexHeader->width * pTexHeader->height;
		}

		fang_MemCopy( &pRawData[ nBytesCopied ], pTexHeader->data, nBytesToCopy ); // copy in the texture palette
		nBytesCopied += nBytesToCopy;
		
		// Check to see if we should copy off the CLUT.
		if( i == ( pPaletteHeader->numDescriptors - 1 ) && pCLUTHeader ) {
			// Copy off the color palette
			nBytesToCopy = pCLUTHeader->numEntries * sizeof( u16 );
			fang_MemCopy( &pRawData[ nBytesCopied ], pCLUTHeader->data, nBytesToCopy ); // copy in the texture palette
			nBytesCopied += nBytesToCopy;			
		}
	}
	
	FASSERT( nBytesCopied == *puDataSize );

	// We are done!!! woo hoo!
		
	return TRUE;
}

FStorage_Error_e fstorage_CalcSaveGameSize( FStorage_DeviceID_e oeID, u32 uDataBytes, u32 *puTotalBytes ) {
	FASSERT_MSG( _bModuleInstalled,                      "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( puTotalBytes,                           "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puTotalBytes = 0;

	if( ! uDataBytes )
	{
		return FSTORAGE_ERROR_NONE;
	}

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	////
	//
	u32 nBlocks;
	fstorage_BytesToBlocks( oeID, uDataBytes + sizeof( _FileHeader_t ), &nBlocks ); // calculate the size of the file + the size of the header
	FASSERT( nBlocks );
	*puTotalBytes = _auSectorSizes[ uDeviceIndex ] * nBlocks;
	//
	////

	return FSTORAGE_ERROR_NONE;
} // fstorage fstorage_CalcSaveGameSize


FStorage_Error_e fstorage_BytesToBlocks( FStorage_DeviceID_e oeID, u32 uBytes, u32 *puBlocks ) {
	FASSERT_MSG( _bModuleInstalled,                      "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( puBlocks,                               "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puBlocks = 0;

	if( ! uBytes ) {
		return FSTORAGE_ERROR_NONE;
	}

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	*puBlocks = ( ( uBytes / _auSectorSizes[ uDeviceIndex ] ) + ( ( uBytes % _auSectorSizes[ uDeviceIndex ] ) ? 1 : 0 ) );

	return FSTORAGE_ERROR_NONE;
}


FINLINE void _fstorage_UnmountCallback( s32 nPort, s32 nResult ) {
#pragma unused( nResult )

	_uRemoved |= ( 1 << nPort );

}


void fstorage_UpdateDeviceInfos( u32 *puConnected, u32 *puInserted, u32 *puRemoved, BOOL bForceUpdate ) {
	FASSERT_MSG( _bModuleInstalled, "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( puConnected,       "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puInserted,        "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( puRemoved,         "[ FSTORAGE ] Error: NULL pointer !!!" );

	// only poll the device on a low frame rate
	if( !bForceUpdate && ( _uDevicePollDelay > ( FLoop_nRealTotalLoopTicks - _uDevicePollTimeStamp ) ) ) {
		*puConnected = _uConnected;
		*puRemoved   = 0;
		*puInserted  = 0;

		return;
	}
	_uDevicePollTimeStamp = FLoop_nRealTotalLoopTicks;
	
	u32 uIndex, uTemp, uCount;
	CARDFileInfo oFileInfo;
	s32 nReturnValue;
	BOOL bContinue;	
	s32 nBytesS32;	

	for( uIndex = 0; uIndex < FSTORAGE_MAX_DEVICES; uIndex++ ) {
		uTemp = (u32)( 1 << uIndex );

		if( CARD_RESULT_READY == CARDProbeEx( (s32)uIndex,
											&nBytesS32,
											NULL ) ) {
			// convert Mega bits to # bytes
			_paoDeviceInfos[ uIndex ].uBytesTotal = (u32)nBytesS32;
			_paoDeviceInfos[ uIndex ].uBytesTotal *= ((1024 * 1024) >> 3);
			
			if( uTemp & _uConnected ) {
				_uInserted &= ~uTemp; // OFF (still connected).
			} else {
				_uConnected |= uTemp; // ON (connected).
				_uInserted  |= uTemp; // ON (connected).
			}
			// update the number of profiles on this card
			if( _paoDeviceInfos[ uIndex ].uStatus == FSTORAGE_DEVICE_STATUS_AVAILABLE ) {
				_fstorage_UpdateNumProfiles( (s32) uIndex );			
			}
		} else {
			_uConnected &= ~uTemp; // OFF (disconnected).
			_uInserted  &= ~uTemp; // OFF (disconnected).
			
			// since the card isn't connected, there can't be any profiles on it
			_paoDeviceInfos[ uIndex ].uNumProfiles = 0;
		}
	}
	
	*puConnected = _uConnected;
	*puInserted  = _uInserted;
	*puRemoved   = _uRemoved;
	
	if( ! ( _uInserted || _uRemoved ) ) {
		return;
	}

	for( uIndex = 0; uIndex < FSTORAGE_MAX_DEVICES; uIndex++ ) {
		uTemp = (u32)( 1 << uIndex );

		// Removals.
		if( uTemp & _uRemoved ) {
			_paoDeviceInfos[ uIndex ].uStatus = FSTORAGE_DEVICE_STATUS_NONE;
			// clear the removed bit
			_uRemoved &= ~uTemp;
		}
		
		// Insertions.
		if( uTemp & _uInserted ) {
			// the device is connected
			_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_CONNECTED;

			// mount the card
			nReturnValue = CARDMount( (s32)uIndex, &_paauCardWorkArea[uIndex * CARD_WORKAREA_SIZE], _fstorage_UnmountCallback );

			// deal with the mount return codes
			switch( nReturnValue ) {

			case CARD_RESULT_BROKEN:
				_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_BROKEN;
				bContinue = TRUE;
				break;

			case CARD_RESULT_ENCODING:
				_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_WRONG_ENCODING;
				bContinue = TRUE;
				break;

			case CARD_RESULT_READY:
				_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_AVAILABLE;
				bContinue = TRUE;
				break;

			case CARD_RESULT_WRONGDEVICE:
				_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_WRONG_DEVICE;
				bContinue = FALSE;
				break;

			case CARD_RESULT_IOERROR:
				_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_DAMAGED;
				bContinue = FALSE;
				break;
				
			default:
				bContinue = FALSE;	
				break;
			}

			if( !bContinue ) {
				continue;
			}

			// check the file system
			if( CARDCheck( (s32)uIndex ) != CARD_RESULT_READY ) {
				_paoDeviceInfos[ uIndex ].uStatus &= ~FSTORAGE_DEVICE_STATUS_AVAILABLE;
				DEVPRINTF( "[ FSTORAGE ] Error %u: CARDCheck() failed for device #%u !!!\n", __LINE__, uIndex );
				continue;
			}
			
			// get the amount of free memory
			if( CARD_RESULT_READY != CARDFreeBlocks( (s32)uIndex,
												&nBytesS32,
												(s32 *)&uCount ) ) {
				_paoDeviceInfos[ uIndex ].uStatus &= ~FSTORAGE_DEVICE_STATUS_AVAILABLE;
				DEVPRINTF( "[ FSTORAGE ] Error %u: CARDFreeBlocks() failed for device #%u !!!\n", __LINE__, uIndex );
				continue;
			}
			_paoDeviceInfos[ uIndex ].uBytesAvailable = (u32)nBytesS32;
			if( !uCount || !_paoDeviceInfos[ uIndex ].uBytesAvailable ) {
				// there are no free directory entries or bytes on the card, so the card must be full
				_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_NO_FREE_MEM;
				_paoDeviceInfos[ uIndex ].uBytesAvailable = 0;
			}

			// get the sector size
			if( CARD_RESULT_READY != CARDGetSectorSize( (s32)uIndex, &_auSectorSizes[ uIndex ] ) ) {
				_paoDeviceInfos[ uIndex ].uStatus &= ~FSTORAGE_DEVICE_STATUS_AVAILABLE;
				DEVPRINTF( "[ FSTORAGE ] Error %u: CARDGetSectorSize() failed for device #%u !!!\n", __LINE__, uIndex );
				continue;
			}
			FASSERT_MSG( ! ( _auSectorSizes[ uIndex ] % CARD_READ_SIZE ), "[ FSTORAGE ] Error: Invalid assumption !!!" );
			
            if( _SUPPORTED_SECTOR_SIZE_IN_BYTES != _auSectorSizes[ uIndex ] ) {
				DEVPRINTF( "[ FSTORAGE ] Warning %u: CARDGetSectorSize() returned an unsupported sector size for device #%u !!!\n", __LINE__, uIndex );
				_paoDeviceInfos[ uIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_UNSUPPORTED;
				continue;
			}
			
			// compute the used byte count
			_paoDeviceInfos[ uIndex ].uBytesUsed = _paoDeviceInfos[ uIndex ].uBytesTotal - _paoDeviceInfos[ uIndex ].uBytesAvailable;
			
			// count the number of profiles on the card
			_fstorage_UpdateNumProfiles( uIndex );
		}
	}
}


const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( FStorage_DeviceID_e oeID ) {
	FASSERT_MSG( _bModuleInstalled,                      "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	return &_paoDeviceInfos[ _fstorage_DeviceIDtoIndex( oeID ) ];

}


const FStorage_DeviceInfo_t *fstorage_GetDeviceInfo( u32 uIndex ) {
	FASSERT_MSG( _bModuleInstalled,             "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( uIndex >= 0,                   "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( uIndex < FSTORAGE_MAX_DEVICES, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	return &_paoDeviceInfos[ uIndex ];

}


FStorage_Error_e fstorage_CreateProfile( FStorage_DeviceID_e oeID, cwchar *pwszName, u32 uSize ) {
	FASSERT_MSG( _bModuleInstalled,                           "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                    "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                   "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN >= wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( uSize,                                       "[ FSTORAGE ] Error: Zero size !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
 
	if( FSTORAGE_DEVICE_STATUS_NO_FREE_MEM & _paoDeviceInfos[ uDeviceIndex ].uStatus ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u out of free memory !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR_NO_FREE_MEM;
	}
	
	// make sure that there is enough room free on the card
	u32 nBlocks;
	fstorage_BytesToBlocks( oeID, uSize + sizeof( _FileHeader_t ), &nBlocks ); // calculate the size of the file + the size of the header
	FASSERT( nBlocks );
	u32 nTotalBytesNeeded = _auSectorSizes[ uDeviceIndex ] * nBlocks;
	if( nTotalBytesNeeded > _paoDeviceInfos[ uDeviceIndex ].uBytesAvailable ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u not enough free memory !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR_NO_FREE_MEM;
	}
	
	// create the file name
	u8 szFileName[ FSTORAGE_MAX_NAME_LEN ];
	fang_MemSet(szFileName, 0, FSTORAGE_MAX_NAME_LEN);
	sprintf( (char *)szFileName, "%08X", fmath_Crc32( 0, (u8 *)pwszName, ( wcslen( pwszName ) << 1 ) ) );

	// create the file on disk
	CARDFileInfo oFileInfo;
	oFileInfo.length = nTotalBytesNeeded;
	s32 nCardError = CARDCreate( (s32)uDeviceIndex, (char *)szFileName, nTotalBytesNeeded, &oFileInfo );
	if( nCardError != CARD_RESULT_READY ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDCreate() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	// grab the current file stats
	_FileHeader_t *pFileHeader = (_FileHeader_t *)_pauTempBuff;
	CARDStat oStat;
	fang_MemZero( &oStat, sizeof( oStat ) );
	fang_MemZero( _pauTempBuff, _auSectorSizes[ uDeviceIndex ] );
	
	if( CARD_RESULT_READY != CARDGetStatus( (s32)uDeviceIndex, oFileInfo.fileNo, &oStat ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDGetStatus() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		CARDFastDelete( (s32)uDeviceIndex, oFileInfo.fileNo );
		return FSTORAGE_ERROR;
	}
	
	// compute the size of the file with header
	pFileHeader->uPayloadAndHeaderSize = uSize + sizeof( _FileHeader_t );
	
	// save the filename as the comment
	_fstorage_ConvertNameToComment( (wchar *)pwszName, pFileHeader->auComment );
		
	// copy the icon data
	fang_MemCopy( pFileHeader->auIconC8andCLUT, _puIcon, _uIconSize );
	
	// copy the banner data
	if( _puBanner ) {
		fang_MemCopy( pFileHeader->auBannerC8andCLUT, _puBanner, _uBannerSize );
	}
	
	CARDSetCommentAddress( &oStat, FANG_OFFSETOF( _FileHeader_t, auComment ) );

	CARDSetIconAddress( &oStat, FANG_OFFSETOF( _FileHeader_t, auBannerC8andCLUT ) );
	CARDSetBannerFormat( &oStat, CARD_STAT_BANNER_C8 );
	CARDSetIconAnim( &oStat, CARD_STAT_ANIM_LOOP );
	CARDSetIconFormat( &oStat, 0, CARD_STAT_ICON_C8 );	
	CARDSetIconSpeed( &oStat, 0, CARD_STAT_SPEED_SLOW );
	CARDSetIconSpeed( &oStat, 1, CARD_STAT_SPEED_END);
	
	// write out the first sector size of data which will contain the banner, icons and a comment string
	if( CARD_RESULT_READY != CARDWrite( &oFileInfo, _pauTempBuff, (s32)_auSectorSizes[ uDeviceIndex ], 0 ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDWrite() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		CARDFastDelete( (s32)uDeviceIndex, oFileInfo.fileNo );
		return FSTORAGE_ERROR;
	}
	
	CARDClose( &oFileInfo );
	
	// set the status info for the file
	if( CARD_RESULT_READY != CARDSetStatus( (s32)uDeviceIndex, oFileInfo.fileNo, &oStat ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDSetStatus() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		CARDFastDelete( (s32)uDeviceIndex, oFileInfo.fileNo );
		return FSTORAGE_ERROR;
	}
	
	// update the amount of free space and used space
	u32 nBytes, nDirs;
	if( CARDFreeBlocks( (s32)uDeviceIndex,
						(s32 *)&nBytes,
						(s32 *)&nDirs ) != CARD_RESULT_READY ) {
		_paoDeviceInfos[ uDeviceIndex ].uStatus &= ~FSTORAGE_DEVICE_STATUS_AVAILABLE;
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDFreeBlocks() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	_paoDeviceInfos[ uDeviceIndex ].uBytesAvailable = nBytes;
	if( !nDirs || !_paoDeviceInfos[ uDeviceIndex ].uBytesAvailable ) {
		// there are no free directory entries or bytes on the card, so the card must be full
		_paoDeviceInfos[ uDeviceIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_NO_FREE_MEM;
		_paoDeviceInfos[ uDeviceIndex ].uBytesAvailable = 0;
	}
	// compute the used byte count
	_paoDeviceInfos[ uDeviceIndex ].uBytesUsed = _paoDeviceInfos[ uDeviceIndex ].uBytesTotal - _paoDeviceInfos[ uDeviceIndex ].uBytesAvailable;	
	
	// update the number of profiles on this device
	_fstorage_UpdateNumProfiles( uDeviceIndex );

	return FSTORAGE_ERROR_NONE;

}


FStorage_Error_e fstorage_DeleteProfile( FStorage_DeviceID_e oeID, cwchar *pwszName ) {
	FASSERT_MSG( _bModuleInstalled,                           "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                    "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                   "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN >= wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	u8 szFileName[ FSTORAGE_MAX_NAME_LEN ];
	fang_MemSet(szFileName, 0, FSTORAGE_MAX_NAME_LEN);
	sprintf( (char *)szFileName, "%08X", fmath_Crc32( 0, (u8 *)pwszName, ( wcslen( pwszName ) << 1 ) ) );

	if( CARD_RESULT_READY != CARDDelete( (s32)uDeviceIndex,  (char *)szFileName ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDDelete() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	// update the amount of free space and used space
	u32 nBytes, nDirs;
	if( CARDFreeBlocks( (s32)uDeviceIndex,
						(s32 *)&nBytes,
						(s32 *)&nDirs ) != CARD_RESULT_READY ) {
		_paoDeviceInfos[ uDeviceIndex ].uStatus &= ~FSTORAGE_DEVICE_STATUS_AVAILABLE;
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDFreeBlocks() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	_paoDeviceInfos[ uDeviceIndex ].uBytesAvailable = nBytes;
	if( !nDirs || !_paoDeviceInfos[ uDeviceIndex ].uBytesAvailable ) {
		// there are no free directory entries or bytes on the card, so the card must be full
		_paoDeviceInfos[ uDeviceIndex ].uStatus |= FSTORAGE_DEVICE_STATUS_NO_FREE_MEM;
		_paoDeviceInfos[ uDeviceIndex ].uBytesAvailable = 0;
	}
	// compute the used byte count
	_paoDeviceInfos[ uDeviceIndex ].uBytesUsed = _paoDeviceInfos[ uDeviceIndex ].uBytesTotal - _paoDeviceInfos[ uDeviceIndex ].uBytesAvailable;
	
	// update the number of profiles on this device
	_fstorage_UpdateNumProfiles( uDeviceIndex );

	return FSTORAGE_ERROR_NONE;

}


FStorage_Error_e fstorage_GetProfileInfos( FStorage_DeviceID_e oeID,
										  FStorage_ProfileInfo_t *paoProfile,
										  u32 uNumProfiles,
										  u32 *puNumProfilesReturned,
										  u32 nStartOffset/*=0*/ ) {
	FASSERT_MSG( _bModuleInstalled,                      "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( paoProfile,                             "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uNumProfiles,                           "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( puNumProfilesReturned,                  "[ FSTORAGE ] Error: NULL pointer !!!" );

	*puNumProfilesReturned = 0;

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );
	FStorage_DeviceInfo_t *pDevInfo = &_paoDeviceInfos[ uDeviceIndex ];

	if( !( pDevInfo->uStatus & FSTORAGE_DEVICE_STATUS_AVAILABLE) ) {
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
	
	CARDFileInfo oFileInfo;
	_FileHeader_t *pFileHeader;
	s32 nReturn;
	CARDStat Stat;
	u32 nProfileCount = 0;
	pFileHeader = (_FileHeader_t *)_pauTempBuff;

	// grab the dvd disk id so that we can compare to the file stats info
	DVDDiskID *pDiskID = DVDGetCurrentDiskID();
	FASSERT( pDiskID ); 
	
	// walk all of the possible files on the card
	for( u32 uIndex = 0; uIndex < CARD_MAX_FILE; uIndex++ ) {
	
		nReturn = CARDGetStatus( (s32)uDeviceIndex, (s32)uIndex, &Stat );
		if( nReturn == CARD_RESULT_READY &&
			fang_IsMemIdentical( pDiskID->gameName, Stat.gameName, sizeof( pDiskID->gameName ) ) &&
			fang_IsMemIdentical( pDiskID->company, Stat.company, sizeof( pDiskID->company ) ) ) {
			
			if( nProfileCount >= nStartOffset ) {
			
				// this is one of our profiles, open it for reading
				if( CARDFastOpen( (s32)uDeviceIndex, (s32)uIndex, &oFileInfo ) == CARD_RESULT_READY ) {
					// read the file (we only need the first readable section to get the info we need)
					if( CARDRead( &oFileInfo, pFileHeader, CARD_READ_SIZE, 0 ) != CARD_RESULT_READY ) {
						DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
						CARDClose( &oFileInfo );
						break;
					}

	                // record the name and size of this profile
					_fstorage_ConvertCommentToName( pFileHeader->auComment,
												(u16 *)paoProfile[ *puNumProfilesReturned ].wszName );
					paoProfile[ *puNumProfilesReturned ].uBytesTotal = pFileHeader->uPayloadAndHeaderSize - sizeof( _FileHeader_t );

					// close the file
					CARDClose( &oFileInfo );

					if( ++( *puNumProfilesReturned ) >= uNumProfiles ) {
						break;
					}
				} else {
					// couldn't open one of our files, this shouldn't happen
					FASSERT_NOW;
					break;
				}	
			}
			nProfileCount++;		
		}
	}
	
	return FSTORAGE_ERROR_NONE;
}


FStorage_Error_e fstorage_ReadProfile( FStorage_DeviceID_e oeID,
									  cwchar *pwszName,
									  u32 uPosition,
									  void *puBuff,
									  u32 uBuffSize ) {
	FASSERT_MSG( _bModuleInstalled,                           "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                    "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                   "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN >= wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( puBuff,                                      "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uBuffSize,                                   "[ FSTORAGE ] Error: Zero size !!!" );
	
	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	CARDFileInfo oFileInfo;
	u8 szFileName[ FSTORAGE_MAX_NAME_LEN ];
	fang_MemSet(szFileName, 0, FSTORAGE_MAX_NAME_LEN);
	// Open.
	sprintf( (char *)szFileName, "%08X", fmath_Crc32( 0, (u8 *)pwszName, ( wcslen( pwszName ) << 1 ) ) );
	
	s32 nCardError = CARDOpen( (s32)uDeviceIndex, (char *)szFileName, &oFileInfo );
	if( nCardError == CARD_RESULT_NOFILE ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDOpen() failed for device #%u - Error = CARD_RESULT_NOFILE!!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	if( nCardError != CARD_RESULT_READY ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDOpen() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	
	u32 nLastVRByte = (uPosition + uBuffSize);
	s32 nSectorSize = (s32)_auSectorSizes[ uDeviceIndex ];
	_FileHeader_t *pFileHeader = (_FileHeader_t *)&_pauTempBuff[nSectorSize];
	u8 *pn1stVRByte = (u8 *)&pFileHeader[1];
	
	// first read the first sector of the file so that we can gather info on the internal file size 	
	if( CARDRead( &oFileInfo, 
				pFileHeader,
				nSectorSize,
				0 ) !=  CARD_RESULT_READY ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		return FSTORAGE_ERROR;
	}
	if( nLastVRByte > (pFileHeader->uPayloadAndHeaderSize - sizeof( _FileHeader_t ) ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Can't read beyound end of file on device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		return FSTORAGE_ERROR;
	}
	
	// see if we have already read enough of the file
	
	if( (nLastVRByte + sizeof( _FileHeader_t )) <= nSectorSize ) {
		// copy the bytes that we've already read
		fang_MemCopy( puBuff, &pn1stVRByte[uPosition], uBuffSize );
		CARDClose( &oFileInfo );
		
		// done
		return FSTORAGE_ERROR_NONE;	
	}
	
	// figure out how many read iterations we will need to get all of the data
	u32 nStartingOffset = ( sizeof( _FileHeader_t ) + uPosition );
	u32 uCurrentSector = ( ( nStartingOffset / nSectorSize ) * nSectorSize );
	u32 uFileOffset = nStartingOffset % nSectorSize;
	u32 uTotalSizeXfr = 0;
	u32 uSizeXfr;

	do {
		if( CARDRead( &oFileInfo,
					_pauTempBuff,
					nSectorSize,
					(s32)uCurrentSector ) != CARD_RESULT_READY ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CARDClose( &oFileInfo );
			return FSTORAGE_ERROR;
		}

		uSizeXfr = FMATH_MIN( uBuffSize, ( nSectorSize - uFileOffset ) );

		fang_MemCopy( ((u8 *)puBuff) + uTotalSizeXfr, _pauTempBuff + uFileOffset, uSizeXfr );

		uFileOffset = 0;
		uTotalSizeXfr += uSizeXfr;
		uCurrentSector += nSectorSize;
		uBuffSize -= uSizeXfr;

	} while( uBuffSize );

	CARDClose( &oFileInfo );

	return FSTORAGE_ERROR_NONE;

} // fstorage_ReadProfile

FStorage_Error_e fstorage_WriteProfile( FStorage_DeviceID_e oeID,
									   cwchar *pwszName,
									   u32 uPosition,
									   void *puBuff,
									   u32 uBuffSize ) {
	FASSERT_MSG( _bModuleInstalled,                           "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                    "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                   "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN >= wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( puBuff,                                      "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( uBuffSize,                                   "[ FSTORAGE ] Error: Zero size !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	CARDFileInfo oFileInfo;
	u8 szFileName[ FSTORAGE_MAX_NAME_LEN ];
	fang_MemSet(szFileName, 0, FSTORAGE_MAX_NAME_LEN);
	// Open.
	sprintf( (char *)szFileName, "%08X", fmath_Crc32( 0, (u8 *)pwszName, ( wcslen( pwszName ) << 1 ) ) );

	if( CARD_RESULT_READY != CARDOpen( (s32)uDeviceIndex, (char *)szFileName, &oFileInfo ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDOpen() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	// Size.
	if( CARD_RESULT_READY != CARDRead( &oFileInfo, 
									&_pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ],
									(s32)_auSectorSizes[ uDeviceIndex ],
									0 ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		return FSTORAGE_ERROR;
	}

	if( ( uPosition + uBuffSize ) > ( (*((_FileHeader_t *)( &( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ) ))).uPayloadAndHeaderSize - sizeof( _FileHeader_t ) ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Can't write beyound end of file on device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		return FSTORAGE_ERROR;
	}

	u32 uTemp = ( sizeof( _FileHeader_t ) + uPosition );

	u32 uCurrentSector = ( ( uTemp / _auSectorSizes[ uDeviceIndex ] ) * _auSectorSizes[ uDeviceIndex ] );
	u32 uFileOffset    = ( uTemp % _auSectorSizes[ uDeviceIndex ] );
	u32 uTotalSizeXfr  = 0;

	if( ( 0 == uCurrentSector ) && ( 0 == ( ( uTemp + uBuffSize ) / _auSectorSizes[ uDeviceIndex ] ) ) ) {
		fang_MemCopy( ( &( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ) + uTemp ), puBuff, uBuffSize );

		if( CARD_RESULT_READY != CARDWrite( &oFileInfo, &( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ), (s32)_auSectorSizes[ uDeviceIndex ], 0 ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: CARDWrite() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CARDClose( &oFileInfo );
			return FSTORAGE_ERROR;
		}

		CARDClose( &oFileInfo );
		return FSTORAGE_ERROR_NONE;
	}

	u32 uSizeXfr;

	do {
		uSizeXfr = FMATH_MIN( uBuffSize, ( _auSectorSizes[ uDeviceIndex ] - uFileOffset ) );

		if( uFileOffset || ( ( uSizeXfr == uBuffSize ) && ( uSizeXfr < _auSectorSizes[ uDeviceIndex ] ) ) ) {
			if( CARD_RESULT_READY != CARDRead( &oFileInfo, _pauTempBuff, (s32)_auSectorSizes[ uDeviceIndex ], (s32)uCurrentSector ) ) {
				DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
				CARDClose( &oFileInfo );
				return FSTORAGE_ERROR;
			}
		}

		fang_MemCopy( ( _pauTempBuff + uFileOffset ), ( ((u8 *)puBuff) + uTotalSizeXfr ), uSizeXfr );

		if( CARD_RESULT_READY != CARDWrite( &oFileInfo, _pauTempBuff, (s32)_auSectorSizes[ uDeviceIndex ], (s32)uCurrentSector ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: CARDWrite() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CARDClose( &oFileInfo );
			return FSTORAGE_ERROR;
		}

		uFileOffset = 0;
		uTotalSizeXfr += uSizeXfr;
		uCurrentSector += _auSectorSizes[ uDeviceIndex ];
		uBuffSize -= uSizeXfr;

	} while( uBuffSize );

	CARDClose( &oFileInfo );

	return FSTORAGE_ERROR_NONE;

} // fstorage_WriteProfile








///////////////////////////////////
// Not used yet
//////////////////////////////////

FStorage_Error_e fstorage_ValidateProfile( FStorage_DeviceID_e oeID,
										  cwchar *pwszName,
										  BOOL bUpdateSignature,
										  BOOL *pbIsValid/*=NULL*/ ) {
	FASSERT_MSG( _bModuleInstalled,                           "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                    "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                   "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN >= wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );

	if( pbIsValid ) {
		*pbIsValid = FALSE;
	}

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	CARDFileInfo oFileInfo;
	u8 szFileName[ FSTORAGE_MAX_NAME_LEN ];
	fang_MemSet(szFileName, 0, FSTORAGE_MAX_NAME_LEN);
	// Open.
	sprintf( (char *)szFileName, "%08X", fmath_Crc32( 0, (u8 *)pwszName, ( wcslen( pwszName ) << 1 ) ) );

	if( CARD_RESULT_READY != CARDOpen( (s32)uDeviceIndex, (char *)szFileName, &oFileInfo ) ) {
//		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDOpen() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	// Calc signature.
	u32 uCrcSignature = 0, uCurrentSector = 0, uCount, uSizeXfr;

	do {
		if( 0 == uCurrentSector ) {
			if( CARD_RESULT_READY != CARDRead( &oFileInfo,
											&( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ),
											(s32)_auSectorSizes[ uDeviceIndex ],
											0 ) ) {
				DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
				CARDClose( &oFileInfo );
				return FSTORAGE_ERROR;
			}

			uCount = (*((_FileHeader_t *)( &( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ) ))).uPayloadAndHeaderSize;
			uSizeXfr = FMATH_MIN( uCount, _auSectorSizes[ uDeviceIndex ] );
			uCrcSignature = fmath_Crc32( uCrcSignature, ( &( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ) + ( FANG_OFFSETOF( _FileHeader_t, uCrc ) + sizeof( u32 ) ) ), ( uSizeXfr - ( FANG_OFFSETOF( _FileHeader_t, uCrc ) + sizeof( u32 ) ) ) );
			uCurrentSector = _auSectorSizes[ uDeviceIndex ];
			uCount -= uSizeXfr;
		} else {
			if( CARD_RESULT_READY != CARDRead( &oFileInfo, _pauTempBuff, (s32)_auSectorSizes[ uDeviceIndex ], (s32)uCurrentSector ) ) {
				DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
				CARDClose( &oFileInfo );
				return FSTORAGE_ERROR;
			}

			uSizeXfr = FMATH_MIN( uCount, _auSectorSizes[ uDeviceIndex ] );
			uCrcSignature = fmath_Crc32( uCrcSignature, _pauTempBuff, uSizeXfr );
			uCount -= uSizeXfr;
			uCurrentSector += _auSectorSizes[ uDeviceIndex ];
		}

	} while( uCount );

	if( bUpdateSignature ) {
		(*((_FileHeader_t *)( &( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ) ))).uCrc = uCrcSignature;

		if( CARD_RESULT_READY != CARDWrite( &oFileInfo,
											&( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ),
											(s32)_auSectorSizes[ uDeviceIndex ],
											0 ) ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: CARDWrite() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
			CARDClose( &oFileInfo );
			return FSTORAGE_ERROR;
		}

		if( pbIsValid ) {
			*pbIsValid = TRUE;
		}
	} else if( pbIsValid ) {
		*pbIsValid = ( uCrcSignature == (*((_FileHeader_t *)( &( _pauTempBuff[ _auSectorSizes[ uDeviceIndex ] ] ) ))).uCrc );

		if( ! *pbIsValid ) {
			DEVPRINTF( "[ FSTORAGE ] Error %u: Signature doesn't match file on device #%u !!!\n", __LINE__, uDeviceIndex );
		}
	}

	CARDClose( &oFileInfo );

	return FSTORAGE_ERROR_NONE;

} // fstorage_ValidateProfile

FStorage_Error_e fstorage_SetProfileIcon( FStorage_DeviceID_e oeID,
										 cwchar *pwszName,
										 void *puIcon,
										 u32 uIconSize ) {
	FASSERT_MSG( _bModuleInstalled,                           "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID,      "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( pwszName,                                    "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( *pwszName,                                   "[ FSTORAGE ] Error: Zero length !!!" );
	FASSERT_MSG( FSTORAGE_MAX_NAME_LEN >= wcslen( pwszName ), "[ FSTORAGE ] Error: String too long !!!" );
	FASSERT_MSG( puIcon,                                      "[ FSTORAGE ] Error: NULL pointer !!!" );
	FASSERT_MSG( ( ( 32 * 32 ) + ( 2 * 256 ) ) == uIconSize,  "[ FSTORAGE ] Error: Wrong size !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_AVAILABLE & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u unavailable !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	CARDFileInfo oFileInfo;
	u8 szFileName[ FSTORAGE_MAX_NAME_LEN ];
	fang_MemSet(szFileName, 0, FSTORAGE_MAX_NAME_LEN);
	// Open.
	sprintf( (char *)szFileName, "%08X", fmath_Crc32( 0, (u8 *)pwszName, ( wcslen( pwszName ) << 1 ) ) );

	if( CARD_RESULT_READY != CARDOpen( (s32)uDeviceIndex, (char *)szFileName, &oFileInfo ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDOpen() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	 
	if( CARD_RESULT_READY != CARDRead( &oFileInfo, _pauTempBuff, (s32)_auSectorSizes[ uDeviceIndex ], 0 ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDRead() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		return FSTORAGE_ERROR;
	}
	
	fang_MemCopy( (*((_FileHeader_t *)(_pauTempBuff))).auIconC8andCLUT, puIcon, uIconSize );
	
	if( CARD_RESULT_READY != CARDWrite( &oFileInfo, _pauTempBuff, (s32)_auSectorSizes[ uDeviceIndex ], 0 ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDWrite() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		CARDClose( &oFileInfo );
		return FSTORAGE_ERROR;
	}

	CARDClose( &oFileInfo );

	return FSTORAGE_ERROR_NONE;

} // fstorage_SetProfileIcon

FStorage_Error_e fstorage_FormatDevice( FStorage_DeviceID_e oeID ) {
	FASSERT_MSG( _bModuleInstalled,                      "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );

	if( ! ( FSTORAGE_DEVICE_STATUS_CONNECTED & _paoDeviceInfos[ uDeviceIndex ].uStatus ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: Device #%u not connected !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	if( CARD_RESULT_READY != CARDFormat( (s32)uDeviceIndex ) ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDFormat() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}
	
	// Clear this cards connected status, so it will re-initialize it's status variables
	u32 uTemp = (u32)( 1 << uDeviceIndex );
	FMATH_CLEARBITMASK( _uConnected, uTemp );
	
	return FSTORAGE_ERROR_NONE;

} // fstorage_FormatDevice


// This function checks to see if it's possible to save a new profile on a particular device....
// It will check:
//	a) If there are enough bytes free on the memory device
//	b) If there are any free file entries on the memory device
extern FStorage_Error_e fstorage_CanSaveNewProfile( FStorage_DeviceID_e oeID, u32 uBytes, u32 *puRetStatus ) {

	*puRetStatus = 0;

	FASSERT_MSG( _bModuleInstalled,                      "[ FSTORAGE ] Error: System not installed !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_NONE        != oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );
	FASSERT_MSG( FSTORAGE_DEVICE_ID_GC_MC_RIGHT >= oeID, "[ FSTORAGE ] Error: Invalid device ID !!!" );

	u32 uDeviceIndex = _fstorage_DeviceIDtoIndex( oeID );
	u32 uTotalBytesNeeded;
	if( fstorage_CalcSaveGameSize( oeID, uBytes, &uTotalBytesNeeded ) != FSTORAGE_ERROR_NONE ) {
		return FSTORAGE_ERROR;
	}

	// update the amount of free space and used space
	u32 nBytes, nDirs;
	if( CARDFreeBlocks( (s32)uDeviceIndex,
						(s32 *)&nBytes,
						(s32 *)&nDirs ) != CARD_RESULT_READY ) {
		DEVPRINTF( "[ FSTORAGE ] Error %u: CARDFreeBlocks() failed for device #%u !!!\n", __LINE__, uDeviceIndex );
		return FSTORAGE_ERROR;
	}

	if( uTotalBytesNeeded > nBytes ) {
		*puRetStatus |= FSTORAGE_DEVICE_STATUS_NO_FREE_MEM;
	}
	if( nDirs == 0 ) {
		*puRetStatus |= FSTORAGE_DEVICE_STATUS_NO_FREE_FILE_ENTRIES;
	}

	if( *puRetStatus ) { // We are all good...
		*puRetStatus = FSTORAGE_DEVICE_READY_FOR_USE;
	}
	
	return FSTORAGE_ERROR_NONE;
}



	