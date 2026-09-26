//////////////////////////////////////////////////////////////////////////////////////
// fDX8AMem.cpp - DirectX Auxiliary memory manager for fang
//
// Author: John Lafleur
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
// 10/21/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fang.h"
#include "fAMem.h"
#include "fmath.h"
#include "fres.h"



//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

// DirectX will manage at most 8 MB's of auxiliary memory in main RAM
#define _DX_MAX_AVAILABLE_FAST_AMEM_BYTES		8192000

// Slow AMEM memory can eventually come from the HD on Xbox
#define _DX_MAX_AVAILABLE_SLOW_AMEM_BYTES		0


//////////////////////////////////////////////////////////////////////////////////////
// Local structures:
//////////////////////////////////////////////////////////////////////////////////////

//
//
struct _MemAlloc_t
{
	void *pLocation;	// Pointer to the base of the allocated memory
	u32  nType;			// Allocation type
	u32  nSize;			// size of the allocation
	s32  nKey;			// Used to verify that CFAmemAlloc's are still valid
	BOOL bFreed;		// If TRUE, this allocation has been freed
};


//////////////////////////////////////////////////////////////////////////////////////
// Local variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized = FALSE;
static void *_pAMEMFastMemory;
static void *_pAMEMFastCurrentHWM;
static u32  _nAMEMFastAllocated;
static u32  _nAMEMSlowAllocated;

static _MemAlloc_t *_paFastHeapAllocations;
static u16 _nFastHeapAllocationIdx;

static s32 _nAllocationKey;


//////////////////////////////////////////////////////////////////////////////////////
// Static functions:
//////////////////////////////////////////////////////////////////////////////////////

static void _FreeHeapAllocation( _MemAlloc_t *pAlloc );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL famem_ModuleStartup( void )
{
	_bModuleInitialized = FALSE;

	_nAllocationKey = 1;

	// If no auxiliary memory was requested, then don't activate the system
	if ( Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes + Fang_ConfigDefs.nAMem_SlowAuxiliaryMemoryBytes == 0 )
	{
		return TRUE;
	}
	
	// Allocate memory for the AMEM manager (On DX, this comes from heap memory)
	_nAMEMFastAllocated = Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes + Fang_ConfigDefs.nAMem_SlowAuxiliaryMemoryBytes;
	if ( _nAMEMFastAllocated > _DX_MAX_AVAILABLE_FAST_AMEM_BYTES )
	{
		DEVPRINTF( "FDX8AMEM.CPP - WARNING! - Fang configured to use more memory than is available in auxiliary memory.  Set to max of %d.\n", _DX_MAX_AVAILABLE_FAST_AMEM_BYTES );
		_nAMEMFastAllocated = _DX_MAX_AVAILABLE_FAST_AMEM_BYTES;
	}
	_pAMEMFastMemory = fres_AlignedAllocAndZero( _nAMEMFastAllocated, 16 );
	if ( !_pAMEMFastMemory )
	{
		DEVPRINTF( "FDX8AMEM.CPP - ERROR! - Unable to allocate %d bytes for auxiliary memory.\n", _nAMEMFastAllocated );
		_pAMEMFastMemory = NULL;
		_pAMEMFastCurrentHWM = _pAMEMFastMemory;
		return FALSE;
	}
	_pAMEMFastCurrentHWM = _pAMEMFastMemory;

	// Allocate structures to track memory allocations
	_nFastHeapAllocationIdx = 0;
	_paFastHeapAllocations = (_MemAlloc_t *)fres_AllocAndZero( sizeof( _MemAlloc_t ) * FAMEM_MAX_ALLOCATIONS );
	if ( !_paFastHeapAllocations )
	{
		DEVPRINTF( "FDX8AMEM.CPP - ERROR! - Unable to allocate %d bytes for auxiliary memory management structures.\n", sizeof( _MemAlloc_t ) * FAMEM_MAX_ALLOCATIONS );
		_paFastHeapAllocations = NULL;
		return FALSE;
	}

	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void famem_ModuleShutdown( void )
{
	FASSERT( _nFastHeapAllocationIdx == 0 );

	_bModuleInitialized = FALSE;
}


//
//
//
u32 famem_GetFreeAMEMBytes( FAmem_Type_e nType )
{
	FASSERT( _bModuleInitialized );
	FASSERT( nType >= 0 && nType < FAMEM_TYPE_COUNT );
	u32 nFree;

	if ( nType == FAMEM_TYPE_FAST )
	{
		// Calculate the free fast AMEM
		nFree = _nAMEMFastAllocated - ((u32)_pAMEMFastCurrentHWM - (u32)_pAMEMFastMemory);
	}
	else if ( nType == FAMEM_TYPE_SLOW )
	{
		// Until we actually have slow amem, all queries will use the fast amem
		nFree = _nAMEMFastAllocated - ((u32)_pAMEMFastCurrentHWM - (u32)_pAMEMFastMemory);
	}
	else
	{
		FASSERT_NOW;
		nFree = 0;
	}

	return nFree;
}


//
//
//
static void _FreeHeapAllocation( _MemAlloc_t *pAlloc )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pAlloc );

	if ( pAlloc->nType == FAMEM_TYPE_FAST )
	{
		u32 nIndex = ((u32)pAlloc - (u32)_paFastHeapAllocations) / sizeof(_MemAlloc_t);
		pAlloc->bFreed = TRUE;

		// If this memory was on the top of the heap, we can collapse the high water mark
		s32 i;
		for ( i = _nFastHeapAllocationIdx - 1; i >= 0; i-- )
		{
			if ( _paFastHeapAllocations[i].bFreed == TRUE )
			{
				// Verify that the next location is NULL
				if ( i < FAMEM_MAX_ALLOCATIONS )
				{
					FASSERT( _paFastHeapAllocations[i+1].pLocation == NULL );
				}

				// The top allocation has been freed, so collapse the heap
				_pAMEMFastCurrentHWM = _paFastHeapAllocations[i].pLocation;
				_paFastHeapAllocations[i].pLocation = NULL;
				_paFastHeapAllocations[i].nSize = 0;
				_nFastHeapAllocationIdx--;
			}
			else
			{
				// We reached an allocation that has not yet been freed, so we
				// must stop collapsing the heap.
				break;
			}
		}
	}
	else if ( pAlloc->nType == FAMEM_TYPE_SLOW )
	{
		// Until we have a slow heap, slow allocations come from the fast heap
		FASSERT_NOW;
	}
	else
	{
		FASSERT_NOW;
	}
}


//
//
//
BOOL CFAMemAccessor::Init( u32 nAMemSize, FAmem_Type_e nType )
{
	FASSERT( _bModuleInitialized );
	FASSERT( nType >= 0 && nType < FAMEM_TYPE_COUNT );

	// Verify that some size was requested
	if ( nAMemSize == 0 )
	{
		return FALSE;
	}

	// Cannot allocate memory if this accessor already has memory
	if ( m_nType != FAMEM_TYPE_INVALID )
	{
		return FAMEM_ERROR_GENERAL;
	}
	
	// Memory allocation must be in multiples of 32 bytes	
	if ( nAMemSize & 31 )
	{
		FASSERT_NOW;
		return FALSE;
	}

	// Make sure we have enough memory available
	u32 nAvailable = famem_GetFreeAMEMBytes( nType );
	if ( nAvailable < nAMemSize )
	{
		DEVPRINTF( "CFAmemAlloc::Init() - ERROR! - Not enough AMEM left to allocate %d bytes.\n", nAMemSize );
		return FALSE;
	}

	// Perform the allocation
	if ( nType == FAMEM_TYPE_FAST )
	{
		// Do not exceed the maximum number of allocations
		if ( _nFastHeapAllocationIdx == FAMEM_MAX_ALLOCATIONS )
		{
			return FALSE;
		}

		m_pPlatformData = &_paFastHeapAllocations[_nFastHeapAllocationIdx];
		m_nKey = _nAllocationKey;
		m_nType = nType;

		_paFastHeapAllocations[_nFastHeapAllocationIdx].pLocation = _pAMEMFastCurrentHWM;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nSize = nAMemSize;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].bFreed = FALSE;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nKey = _nAllocationKey;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nType = FAMEM_TYPE_FAST;
		_nFastHeapAllocationIdx++;
		_nAllocationKey++;

		// Modify current high-water mark
		_pAMEMFastCurrentHWM = (void *)FMATH_BYTE_ALIGN_UP( ((u32)_pAMEMFastCurrentHWM + nAMemSize ), 32 );
	}
	else if ( nType == FAMEM_TYPE_SLOW )
	{
		// Until we actually have slow amem, all queries will use the fast amem

		// Do not exceed the maximum number of allocations
		if ( _nFastHeapAllocationIdx == FAMEM_MAX_ALLOCATIONS )
		{
			return FALSE;
		}

		m_pPlatformData = &_paFastHeapAllocations[_nFastHeapAllocationIdx];
		m_nKey = _nAllocationKey;
		m_nType = nType;

		_paFastHeapAllocations[_nFastHeapAllocationIdx].pLocation = _pAMEMFastCurrentHWM;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nSize = nAMemSize;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].bFreed = FALSE;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nKey = _nAllocationKey;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nType = FAMEM_TYPE_FAST;
		_nFastHeapAllocationIdx++;
		_nAllocationKey++;

		// Modify current high-water mark
		_pAMEMFastCurrentHWM = (void *)FMATH_BYTE_ALIGN_UP( ((u32)_pAMEMFastCurrentHWM + nAMemSize ), 32 );
	}
	else
	{
		FASSERT_NOW;
		return FALSE;
	}

	return TRUE;
}


//
//
//
FAmem_Error_e CFAMemAccessor::Write( u32 nLocation, const void *pSourceData, u32 nSize, FAmemCallback_t *pCallback/*=NULL*/, void *pUser/*=NULL*/ )
{
	FASSERT( _bModuleInitialized );
	FASSERT( m_nType >= 0 && m_nType < FAMEM_TYPE_COUNT );

	// FMem operations must be multiples of 32 bytes
	if ( (nSize & 31) || ((u32)pSourceData & 31) || (nLocation & 31) )
	{
		FASSERT_NOW;
		return FAMEM_ERROR_INVALID_ALIGNMENT;
	}

	// Get a pointer to the allocation
	_MemAlloc_t *pAlloc = (_MemAlloc_t *)m_pPlatformData;

	// Verify that the key still matches
	if ( !pAlloc || pAlloc->nKey != m_nKey )
	{
		return FAMEM_ERROR_INVALID_ALLOCATION_KEY;
	}

	// Verify that the memory has not been freed
	if ( pAlloc->bFreed )
	{
		return FAMEM_ERROR_MEMORY_FREED;
	}

	// Verify that the operation is in bounds
	if ( nLocation < 0 || nLocation > pAlloc->nSize 
		|| nLocation + nSize > pAlloc->nSize )
	{
		return FAMEM_ERROR_OUT_OF_BOUNDS_WRITE;
	}

	// Execute the write
	fang_MemCopy( (u8 *)pAlloc->pLocation + nLocation, pSourceData, nSize );

	if ( pCallback )
	{
		// On DX, the write is instantaneous, so we can immediately call the callback
		pCallback( FAMEM_OP_WRITE, this, pUser, FAMEM_ERROR_NONE );
	}

	return FAMEM_ERROR_NONE;
}


//
//
//
FAmem_Error_e CFAMemAccessor::Read( u32 nLocation, void *pDest, u32 nSize, FAmemCallback_t *pCallback/*=NULL*/, void *pUser/*=NULL*/ )
{
	FASSERT( _bModuleInitialized );
	FASSERT( m_nType >= 0 && m_nType < FAMEM_TYPE_COUNT );

	// FMem operations must be multiples of 32 bytes
	if ( (nSize & 31) || ((u32)pDest & 31) || (nLocation & 31) )
	{
		FASSERT_NOW;
		return FAMEM_ERROR_INVALID_ALIGNMENT;
	}

	// Get a pointer to the allocation
	_MemAlloc_t *pAlloc = (_MemAlloc_t *)m_pPlatformData;

	// Verify that the key still matches
	if ( !pAlloc || pAlloc->nKey != m_nKey )
	{
		return FAMEM_ERROR_INVALID_ALLOCATION_KEY;
	}

	// Verify that the memory has not been freed
	if ( pAlloc->bFreed )
	{
		return FAMEM_ERROR_MEMORY_FREED;
	}

	// Verify that the operation is in bounds
	if ( nLocation < 0 || nLocation > pAlloc->nSize 
		|| nLocation + nSize > pAlloc->nSize )
	{
		return FAMEM_ERROR_OUT_OF_BOUNDS_WRITE;
	}

	// Execute the read
	fang_MemCopy( pDest, (u8 *)pAlloc->pLocation + nLocation, nSize );

	if ( pCallback )
	{
		// On DX, the read is instantaneous, so we can immediately call the callback
		pCallback( FAMEM_OP_READ, this, pUser, FAMEM_ERROR_NONE );
	}

	return FAMEM_ERROR_NONE;
}


//
//
//
s32 CFAMemAccessor::Verify( u32 nLocation, void *pData, u32 nSize )
{
/*
	#define __MAX_READ	128
	u32 nStart = FMATH_BYTE_ALIGN_DOWN( nLocation, 32 );
	u32 nReadDiff = nLocation - nStart;
	u32 nRemainingBytes = nSize + nReadDiff;
	u32 nBytesCompared = 0;
	u8  nStoredData[__MAX_READ];

	while ( nRemainingBytes > 0 )
	{
		if ( nRemainingBytes > __MAX_READ )
		{
			nReadSize = __MAX_READ;
		}
		else
		{
			nReadSize = FMATH_BYTE_ALIGN_UP( nRemainingBytes );
		}

		Read( nStart, &nStoredData, nReadSize );

		for ( i = nReadDiff; i < nReadSize && nBytesCompared < nSize; i++ )
		{
			if ( nStoredData[i] != ((u8 *)pData)[nBytesCompared] )
			{
				return nBytesCompared;
			}

			nBytesCompared++;
		}

		if ( nReadDiff != 0 )
		{
			nReadDiff = 0;
		}

		nRemainingBytes -= nReadSize;
	}

	#undef __MAX_READ
*/
	return -1;
}


//
//
//
FAmem_Error_e CFAMemAccessor::Free( void )
{
	FASSERT( _bModuleInitialized );
	FASSERT( (m_nType >= 0 && m_nType < FAMEM_TYPE_COUNT) || m_nType == FAMEM_TYPE_INVALID );

	if ( m_nType == FAMEM_TYPE_INVALID )
	{
		return FAMEM_ERROR_NONE;
	}
	
	// Get a pointer to the allocation
	_MemAlloc_t *pAlloc = (_MemAlloc_t *)m_pPlatformData;

	// Verify that the key still matches
	if ( !pAlloc || pAlloc->nKey != m_nKey )
	{
		return FAMEM_ERROR_INVALID_ALLOCATION_KEY;
	}

	// Verify that the memory has not been freed
	if ( pAlloc->bFreed )
	{
		return FAMEM_ERROR_NONE;
	}

	_FreeHeapAllocation( pAlloc );

	m_nType = FAMEM_TYPE_INVALID;

	return FAMEM_ERROR_NONE;
}


//
//
//
CFAMemAccessor::~CFAMemAccessor( void )
{
	// Get a pointer to the allocation
	_MemAlloc_t *pAlloc = (_MemAlloc_t *)m_pPlatformData;

	if ( pAlloc )
	{
		_FreeHeapAllocation( pAlloc );
	}
}
