//////////////////////////////////////////////////////////////////////////////////////
// PSPool.cpp - Point sprite pool.
//
// Author: Michael Starich   
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
// 03/30/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "PSPool.h"
#include "fDataPool.h"


static CFDataPool _PSPool;
static BOOL _bSystemInitialized = FALSE;
static u32 _nMaxNumUsed = 0;
static u32 _nNumPSprites = PS_POOL_COUNT;


BOOL pspool_InitSystem( void ) {
	FASSERT( !_bSystemInitialized );

	_nNumPSprites = PS_POOL_COUNT;

	// Allocate a point-sprite pool...
	_bSystemInitialized = _PSPool.Create( sizeof( FPSprite_t ), _nNumPSprites, FPSPRITE_BYTE_ALIGNMENT, 16 );
	if( !_bSystemInitialized ) {
		DEVPRINTF( "pspool_InitSystem(): Could not allocate a pool of %u point sprites.\n", _nNumPSprites );
	}
	_nNumPSprites = _PSPool.GetSize();
	_nMaxNumUsed = 0;

	return _bSystemInitialized;
}


void pspool_UninitSystem( void ) {
	if( _bSystemInitialized ) {
		_bSystemInitialized = FALSE;
		DEVPRINTF( "pspool : %d of %d FPSprite_t's were used at any one time, you can save %d bytes by lowering the count.\n",
			_nMaxNumUsed, _nNumPSprites, sizeof( FPSprite_t ) * (_nNumPSprites - _nMaxNumUsed) );
	}
}


FPSprite_t *pspool_GetArray( u32 nNumVerts ) {
	if( !_bSystemInitialized ) {
		return NULL;
	}

	// See if there are enough elements in our pool to allocate
	// the requested amount (plus 1 to store the count)...

	++nNumVerts;
	if( _PSPool.GetNumFree() < nNumVerts ) {
		DEVPRINTF( "pspool_GetArray(): Could not allocate %u point sprites from pool. Enlarge the pool size.\n", nNumVerts );
		return NULL;
	}

	FPSprite_t *pPS = (FPSprite_t *)_PSPool.GetArray( nNumVerts );
	if( pPS ) {
		// Use the first element to store the count...
		u32 *pnAddressToStoreCount = (u32 *)pPS;
		*pnAddressToStoreCount = nNumVerts;

		++pPS;

		u32 nNumUsed = _PSPool.GetNumUsed();
		if( nNumUsed > _nMaxNumUsed ) {
			_nMaxNumUsed = nNumUsed;
		}

		return pPS;
	}

	return NULL;
}


void pspool_ReturnArray( FPSprite_t *pPS ) {
	
	if( !_bSystemInitialized || !pPS ) {
		return;
	}

	// We stored the element count before the array...
	FPSprite_t *pRealHead = pPS--;
	u32 *pnAddressOfStoredCount = (u32 *)pPS;
	_PSPool.ReturnToPool( pPS, *pnAddressOfStoredCount );
}

