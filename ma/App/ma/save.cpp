//////////////////////////////////////////////////////////////////////////////////////
// save.cpp - Game save module.
//
// Author: Steve Ranck     
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
// 04/13/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#if 0
#include "fang.h"
#include "save.h"
#include "fres.h"



static BOOL _bSystemInitialized;

static void *_pBuffer;
static u32 _nBufferByteCount;

static BOOL _bSaveMode;
static u8 *_pnCurrentSaveAddress;
static u32 _nCurrentSaveByteCount;
static u32 _nSavedObjectCount;



static void _ResetSaveMode( void );
static void _BufferFreedCallback( void *pResMem );




BOOL save_InitSystem( void ) {
	FASSERT( !_bSystemInitialized );

	_pBuffer = NULL;
	_nBufferByteCount = 0;

	_bSaveMode = FALSE;
	_ResetSaveMode();
	_nSavedObjectCount = 0;

	_bSystemInitialized = TRUE;

	return TRUE;
}


void save_UninitSystem( void ) {
	if( _bSystemInitialized ) {

		_pBuffer = NULL;
		_nBufferByteCount = 0;

		_bSaveMode = FALSE;
		_ResetSaveMode();
		_nSavedObjectCount = 0;

		_bSystemInitialized = FALSE;
	}
}


BOOL save_AllocateBuffer( u32 nByteCount ) {
	FASSERT( _bSystemInitialized );
	FASSERT( _pBuffer == NULL );

	_pBuffer = fres_CreateAndAlloc( SAVE_RESTYPE, NULL, _BufferFreedCallback, nByteCount, NULL );

	if( _pBuffer == NULL ) {
		// Could not create resource...
		DEVPRINTF( "save_AllocateBuffer(): Not enough memory for game save buffer (%u bytes requested).\n", nByteCount );
		return FALSE;
	}

	_nBufferByteCount = nByteCount;

	_bSaveMode = FALSE;
	_ResetSaveMode();
	_nSavedObjectCount = 0;

	return TRUE;
}


void save_BeginSaveMode( void ) {
	FASSERT( _bSystemInitialized );
	FASSERT( _pBuffer );
	FASSERT( !_bSaveMode );

	_bSaveMode = TRUE;
	_ResetSaveMode();
	_nSavedObjectCount = 0;
}


void save_EndSaveMode( void ) {
	FASSERT( _bSystemInitialized );
	FASSERT( _pBuffer );

	if( _bSaveMode ) {
		_bSaveMode = FALSE;
		_ResetSaveMode();
	}
}


BOOL save_SaveObjectState( const void *pObjectAddress, u32 nObjectByteCount ) {
	FASSERT( _bSystemInitialized );
	FASSERT( _bSaveMode );
	FASSERT( sizeof(u32) == 4 );
	FASSERT( sizeof(void *) == 4 );

	if( nObjectByteCount == 0 ) {
		return TRUE;
	}

	if( (_nCurrentSaveByteCount + nObjectByteCount + sizeof(u32) + sizeof(void *)) > _nBufferByteCount ) {
		DEVPRINTF( "save_SaveState(): Save buffer is full (currently %u bytes).\n", _nBufferByteCount );
		return FALSE;
	}

	*_pnCurrentSaveAddress++ = (u8)((nObjectByteCount >>  0) & 0xff);
	*_pnCurrentSaveAddress++ = (u8)((nObjectByteCount >>  8) & 0xff);
	*_pnCurrentSaveAddress++ = (u8)((nObjectByteCount >> 16) & 0xff);
	*_pnCurrentSaveAddress++ = (u8)((nObjectByteCount >> 24) & 0xff);

	*_pnCurrentSaveAddress++ = (u8)(((u32)pObjectAddress >>  0) & 0xff);
	*_pnCurrentSaveAddress++ = (u8)(((u32)pObjectAddress >>  8) & 0xff);
	*_pnCurrentSaveAddress++ = (u8)(((u32)pObjectAddress >> 16) & 0xff);
	*_pnCurrentSaveAddress++ = (u8)(((u32)pObjectAddress >> 24) & 0xff);

	fang_MemCopy( _pnCurrentSaveAddress, pObjectAddress, nObjectByteCount );

	_pnCurrentSaveAddress += nObjectByteCount;

	_nSavedObjectCount++;

	return TRUE;
}


void save_RestoreAllObjects( void ) {
	u8 *pnCurrentRestoreAddress;
	void *pObjectAddress;
	u32 nObjectByteCount;

	FASSERT( _bSystemInitialized );
	FASSERT( !_bSaveMode );
	FASSERT( sizeof(u32) == 4 );
	FASSERT( sizeof(void *) == 4 );

	if( _pBuffer == NULL ) {
		return;
	}

	for( pnCurrentRestoreAddress=(u8 *)_pBuffer; _nSavedObjectCount; _nSavedObjectCount-- ) {
		nObjectByteCount = (u32)pnCurrentRestoreAddress[0] | ((u32)pnCurrentRestoreAddress[1] << 8) | ((u32)pnCurrentRestoreAddress[2] << 16) | ((u32)pnCurrentRestoreAddress[3] << 24);
		pObjectAddress = (void *)((u32)pnCurrentRestoreAddress[4] | ((u32)pnCurrentRestoreAddress[5] << 8) | ((u32)pnCurrentRestoreAddress[6] << 16) | ((u32)pnCurrentRestoreAddress[7] << 24));

		pnCurrentRestoreAddress += 8;

		fang_MemCopy( pObjectAddress, pnCurrentRestoreAddress, nObjectByteCount );

		pnCurrentRestoreAddress += nObjectByteCount;
	}
}


static void _ResetSaveMode( void ) {
	_pnCurrentSaveAddress = (u8 *)_pBuffer;
	_nCurrentSaveByteCount = 0;
}


static void _BufferFreedCallback( void *pResMem ) {
	_pBuffer = NULL;
	_nBufferByteCount = NULL;

	_bSaveMode = FALSE;
	_ResetSaveMode();
	_nSavedObjectCount = 0;
}

#endif