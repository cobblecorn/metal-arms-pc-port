//////////////////////////////////////////////////////////////////////////////////////
// fvtxpool.cpp - 
//
// Author: Michael Starich / Russell Foushee   
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
// 01/28/03 Foushee		Created and used VtxPool as a basis 
// 03/30/02 Starich     VtxPool Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fvtxpool.h"
#include "fDataPool.h"
#include "fdraw.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

static CFDataPool _VtxPool;
static BOOL _bSystemOK = FALSE;
static u32 _nVtxPoolCount = 0;
static u32 _nMaxNumUsed = 0;

//===================
// private prototypes

//=================
// public functions

BOOL fvtxpool_ModuleStartup( void ) {

	FASSERT( !_bSystemOK );

	// allocate an fdraw vtx pool
	// examine the fang configdefs and determine the size of the pool we need to allocate
	_nVtxPoolCount = Fang_ConfigDefs.nVtxPool_VertexCount;
/*
	if( Fang_ConfigDefs.nText_MaxCharsPerFrame * 6 > _nVtxPoolCount ) {
		_nVtxPoolCount = Fang_ConfigDefs.nText_MaxCharsPerFrame * 6;
		DEVPRINTF( "fvtxpool_ModuleStartup(): Increasing fvtxpool vertex count from %d to %d due to ftext max characters per frame requirements.\n",
			Fang_ConfigDefs.nVtxPool_VertexCount, _nVtxPoolCount );
	}
*/

	_bSystemOK = _VtxPool.Create( sizeof( FDrawVtx_t ), _nVtxPoolCount, 4, 16 );
	if( !_bSystemOK ) {
		DEVPRINTF( "fvtxpool_ModuleStartup(): Could not allocate a %d FDrawVtx pool.\n", _nVtxPoolCount );
	}
	_nVtxPoolCount = _VtxPool.GetSize();
	_nMaxNumUsed = 0;

	return _bSystemOK;
}

void fvtxpool_ModuleShutdown( void ) {
	if( _bSystemOK ) {
		_bSystemOK = FALSE;
		DEVPRINTF( "fvtxpool : %d of %d FDrawVtx_t's were used at any one time, you can save %d bytes by lowering the count.\n",
			_nMaxNumUsed, _nVtxPoolCount, sizeof( FDrawVtx_t ) * (_nVtxPoolCount - _nMaxNumUsed) );
	}
}

FDrawVtx_t *fvtxpool_GetArray( u32 nNumVerts, BOOL bSuppressNotEnoughVerticesWarning ) {
	
	if( !_bSystemOK ) {
		return NULL;
	}
	// see if there are enough verts in our pool to allocate the requested amount (plus 1 to store the count)
	nNumVerts++;
	if( _VtxPool.GetNumFree() < nNumVerts ) {
		if( !bSuppressNotEnoughVerticesWarning ) {
			DEVPRINTF( "fvtxpool_GetArray(): Not enough verts to allocate %d, enlarge the pool size.\n", nNumVerts );
		}
		return NULL;
	}
	FDrawVtx_t *pVtx = (FDrawVtx_t *)_VtxPool.GetArray( nNumVerts );
	if( pVtx ) {
		// use the 1st vert to store the count
		u32 *pnPlaceToStoreCount = (u32 *)pVtx;
		*pnPlaceToStoreCount = nNumVerts;
		++pVtx;

		u32 nNumUsed = _VtxPool.GetNumUsed();
		if( nNumUsed > _nMaxNumUsed ) {
			_nMaxNumUsed = nNumUsed;
			FASSERT( _nMaxNumUsed <= _nVtxPoolCount );
		}
		return pVtx;
	}
	
	return NULL;
}


void fvtxpool_ReturnArray( FDrawVtx_t *pVtx ) {
	
	if( !_bSystemOK || !pVtx ) {
		return;
	}
	// we stored the vtx count before the array
	FDrawVtx_t *pRealHead = pVtx--;
	u32 *pnPlaceToStoreCount = (u32 *)pVtx;
	_VtxPool.ReturnToPool( pVtx, *pnPlaceToStoreCount );
}

//==================
// private functions



