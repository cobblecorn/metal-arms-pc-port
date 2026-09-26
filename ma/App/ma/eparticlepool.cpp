//////////////////////////////////////////////////////////////////////////////////////
// EParticlePool.cpp - a generic pool of CEParticles
//
// Author: Mike Starich      
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
// 02/06/03 Starich        Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "eparticlepool.h"
#include "eparticle.h"
#include "fres.h"

typedef enum {
	_INIT_STATE_NOT_READY = 0,
	_INIT_STATE_SYSTEM_OK,
	_INIT_STATE_LEVEL_OK,

	_INIT_STATE_COUNT 
} _InitState_e;

static _InitState_e _nSystemState = _INIT_STATE_NOT_READY;
static CEParticle *_paPartPool = NULL;
static u32 _nNextStartIndex = 0;

static s32 _FindFirstAvailableIndex( void );


BOOL eparticlepool_InitSystem( void ) {
	FASSERT( _nSystemState == _INIT_STATE_NOT_READY );

	FMemFrame_t hResFrame = fmem_GetFrame();

	// allocate the pool array
	_paPartPool = fnew CEParticle[EPARTICLE_POOL_COUNT];
	if( !_paPartPool ) {
		goto _EXIT_WITH_ERROR;
	}
	
	_nNextStartIndex = 0;

	// we are inited
	_nSystemState = _INIT_STATE_SYSTEM_OK;

	return TRUE;

_EXIT_WITH_ERROR:
	fdelete_array( _paPartPool );
	_paPartPool = NULL;
	fmem_ReleaseFrame( hResFrame );
	return FALSE;
}

void eparticlepool_UninitSystem( void ) {
	if( _nSystemState > _INIT_STATE_NOT_READY ) {
		eparticlepool_UninitLevel();

		fdelete_array( _paPartPool );
		_paPartPool = NULL;
		
		_nSystemState = _INIT_STATE_NOT_READY;
	}
}

// called before a level, will Create each entry in the pool
BOOL eparticlepool_InitLevel( void ) {
	u32 i;
	
	FASSERT( _nSystemState == _INIT_STATE_SYSTEM_OK );

	_nSystemState = _INIT_STATE_LEVEL_OK;
	_nNextStartIndex = 0;

	for( i=0; i < EPARTICLE_POOL_COUNT; i++ ) {
		if( !_paPartPool[i].Create() ) {
			eparticlepool_UninitLevel();// will change the state to _INIT_STATE_SYSTEM_OK
			return FALSE;
		}		
	}

	return TRUE;
}

// called after the level is finished, will Destroy all emitters and return them to the pool
void eparticlepool_UninitLevel( void ) {
	u32 i;

	if( _nSystemState == _INIT_STATE_LEVEL_OK ) {
		for( i=0; i < EPARTICLE_POOL_COUNT; i++ ) {
			if( _paPartPool[i].IsCreated() ) {
				_paPartPool[i].Destroy();
			}
		}

		_nSystemState = _INIT_STATE_SYSTEM_OK;
	}
}

// called to get a temp emitter, once this emitter has stopped emitting particles it will be reused automatically
CEParticle *eparticlepool_GetEmitter( void ) {
	s32 nIndex;

	FASSERT( _nSystemState == _INIT_STATE_LEVEL_OK );

	nIndex = _FindFirstAvailableIndex();
	if( nIndex < 0 ) {
		DEVPRINTF( "eparticlepool_GetEmitter() : No emitters available, the size of the pool needs to be increased (currently size %d).\n", EPARTICLE_POOL_COUNT );
		return NULL;
	}
	return &_paPartPool[nIndex];
}

// will start at _nNextStartIndex and will search for an available slot that can
// be distributed.  Will not mark the returned index as used, because it doesn't 
// know how it will beused.  Coming out of this function, _nNextStartIndex will be
// set to the next index after the returned one (except if -1 is returned)
static s32 _FindFirstAvailableIndex( void ) {
	u32 i, nNextIndex;
	
	for( i=0; i < EPARTICLE_POOL_COUNT; i++ ) {

		// compute the next index
		nNextIndex = _nNextStartIndex + 1;
		if( nNextIndex >= EPARTICLE_POOL_COUNT ) {
			nNextIndex = 0;
		}

		if( !_paPartPool[_nNextStartIndex].IsEmitting() ) {
			// before returning, update the next place to start looking
			i = _nNextStartIndex;
			_nNextStartIndex = nNextIndex;
			return i;
		}

		// advance to the next particle emitter
		_nNextStartIndex = nNextIndex;
	}

	return -1;
}