//////////////////////////////////////////////////////////////////////////////////////
// fGCAMem.cpp - GameCube Auxiliary memory manager for fang
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

// GameCube will manage at most 16 MB's of auxiliary memory in ARAM
#define _GC_MAX_AVAILABLE_FAST_AMEM_BYTES		16777216
#define _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS		32
#define _INVALID_ARQREQUEST_OWNER				0xffffffff


//////////////////////////////////////////////////////////////////////////////////////
// Local structures:
//////////////////////////////////////////////////////////////////////////////////////

//
//
struct _MemAlloc_t
{
	void *pLocation;	// Pointer to the base of the allocated memory
	u32  nSize;			// size of the allocation
	s32  nKey;			// Used to verify that CFAmemAlloc's are still valid
	BOOL bFreed;		// If TRUE, this allocation has been freed
};


//
//
struct _MemRequest_t
{
	ARQRequest				Request;
	CFAMemAccessor			*pAMemAlloc;
	void					*pUser;
	FAmemCallback_t 		*pCallback;
};



//////////////////////////////////////////////////////////////////////////////////////
// Local variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized = FALSE;
static u8   *_pAMEMFastMemory;
static u32  _nMaxFastBytesAvailable;
static u32  _nFastBytesAllocated;

static _MemAlloc_t *_paFastHeapAllocations;
static u16 _nFastHeapAllocationIdx;

static s32 _nAllocationKey;

//
volatile _MemRequest_t *_paMemRequests;
volatile u32 _nActiveARQRequestCount;

volatile BOOL _bInShutdown;

//////////////////////////////////////////////////////////////////////////////////////
// Static functions:
//////////////////////////////////////////////////////////////////////////////////////

static void _FreeHeapAllocation( _MemAlloc_t *pAlloc );
static void _ARQCallback( u32 pPointerToARQRequest );


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
	_bInShutdown = FALSE;
	
	// If no auxiliary memory was requested, then don't activate the system
	if ( Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes + Fang_ConfigDefs.nAMem_SlowAuxiliaryMemoryBytes == 0 )
	{
		return TRUE;
	}
	
	// Make sure the ARQ is initialized
	if ( !ARCheckInit() )
	{
		FASSERT_NOW;
		DEVPRINTF( "famem_ModuleStartup() - ERROR! - ARQ driver not initialized.\n" );
		return FALSE;
	}

	// Determine bytes available
	u32 nBytesAfterAudio = _GC_MAX_AVAILABLE_FAST_AMEM_BYTES - Fang_ConfigDefs.nAudio_MaxSoundBytes - ARGetBaseAddress();
	nBytesAfterAudio = FMATH_BYTE_ALIGN_DOWN( nBytesAfterAudio, 32 );
	_nMaxFastBytesAvailable = Fang_ConfigDefs.nAMem_FastAuxiliaryMemoryBytes + Fang_ConfigDefs.nAMem_SlowAuxiliaryMemoryBytes;
	if ( _nMaxFastBytesAvailable > nBytesAfterAudio )
	{
		DEVPRINTF( "FGCAMEM.CPP - WARNING! - Fang configured to use more memory than is available in auxiliary memory.  Set to max of %d.\n", nBytesAfterAudio );
		_nMaxFastBytesAvailable = nBytesAfterAudio;
	}
	
	// Set the memory pointers for the AMEM manager (On GameCube, this comes from ARAM)
	_pAMEMFastMemory = (u8 *)(ARGetBaseAddress() + Fang_ConfigDefs.nAudio_MaxSoundBytes);
	_nFastBytesAllocated = 0;

	// Allocate structures to track memory allocations
	_nFastHeapAllocationIdx = 0;
	_paFastHeapAllocations = (_MemAlloc_t *)fres_AllocAndZero( sizeof( _MemAlloc_t ) * FAMEM_MAX_ALLOCATIONS );
	if ( !_paFastHeapAllocations )
	{
		DEVPRINTF( "FGCAMEM.CPP - ERROR! - Unable to allocate %d bytes for auxiliary memory management structures.\n", sizeof( _MemAlloc_t ) * FAMEM_MAX_ALLOCATIONS );
		_paFastHeapAllocations = NULL;
		return FALSE;
	}
	
	// Allocate memory for ARQ requests
	_nActiveARQRequestCount = 0;
	_paMemRequests = (_MemRequest_t *)fres_AllocAndZero( sizeof( _MemRequest_t ) * _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS );
	if ( !_paMemRequests )
	{
		DEVPRINTF( "FGCAMEM.CPP - ERROR! - Unable to allocate %d bytes for auxiliary memory management structures.\n", sizeof( _MemRequest_t ) * _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS );
		_paMemRequests = NULL;
		return FALSE;
	}
	
	u32 i;
	for ( i = 0; i < _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS; i++ )
	{
		_paMemRequests[i].Request.owner = i;
		_paMemRequests[i].pAMemAlloc = NULL;
		_paMemRequests[i].pCallback = NULL;
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
	
	_bInShutdown = TRUE;
	
	// Flush all pending requests
	ARQFlushQueue();
	
	// Wait for completion of in-progress requests
	while( TRUE )
	{
		if ( !ARGetDMAStatus() )
		{
			break;
		}
	}
	
	FASSERT( _nActiveARQRequestCount == 0 );
}


//
//
//
u32 famem_GetFreeAMemBytes( FAmem_Type_e nType )
{
	FASSERT( _bModuleInitialized );
	FASSERT( nType >= 0 && nType < FAMEM_TYPE_COUNT );

	return _nMaxFastBytesAvailable - _nFastBytesAllocated;
}


//
//
//
static void _FreeHeapAllocation( _MemAlloc_t *pAlloc )
{
	FASSERT( _bModuleInitialized );
	FASSERT( pAlloc );

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
				FASSERT( _paFastHeapAllocations[i + 1].pLocation == NULL );
			}

			// The top allocation has been freed, so collapse the heap
			_nFastBytesAllocated -= _paFastHeapAllocations[i].nSize;
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


//
//
//
static void _ARQCallback( u32 pPointerToARQRequest )
{
	FAmem_Operation_e nOp;
	
	ARQRequest *pARQRequest = (ARQRequest *)pPointerToARQRequest;

	// Verify that we got back a valid ARQ request
	if ( !pARQRequest || pARQRequest->owner < 0 || pARQRequest->owner >= _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS  )
	{
		FASSERT_NOW;
		return;
	}
	
	volatile _MemRequest_t *pMemRequest = &_paMemRequests[pARQRequest->owner];
	
	if ( pARQRequest->type == ARQ_TYPE_MRAM_TO_ARAM )
	{
		nOp = FAMEM_OP_WRITE;
	}
	else if ( pARQRequest->type == ARQ_TYPE_ARAM_TO_MRAM )
	{
		nOp = FAMEM_OP_READ;
	}
	
	if ( pMemRequest->pCallback && !_bInShutdown )
	{
		pMemRequest->pCallback( nOp, (CFAMemAccessor *)pMemRequest->pAMemAlloc, pMemRequest->pUser, FAMEM_ERROR_NONE );
	}
	
	pMemRequest->pAMemAlloc = NULL;
	pMemRequest->pCallback = NULL;
	
//	DEVPRINTF( "_ARQCallback() - ARAM operation complete.\n" );
	
	FASSERT( _nActiveARQRequestCount > 0 );
	_nActiveARQRequestCount--;
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
	if ( _nMaxFastBytesAvailable - _nFastBytesAllocated < nAMemSize )
	{
		DEVPRINTF( "CFAmemAlloc::Init() - ERROR! - Not enough AMEM left to allocate %d bytes.\n", nAMemSize );
		return FALSE;
	}

	m_pPlatformData = &_paFastHeapAllocations[_nFastHeapAllocationIdx];
	m_nKey = _nAllocationKey;
	m_nType = nType;

	// Perform the allocation
	if ( nType == FAMEM_TYPE_FAST || nType == FAMEM_TYPE_SLOW )
	{
		// Do not exceed the maximum number of allocations
		if ( _nFastHeapAllocationIdx == FAMEM_MAX_ALLOCATIONS )
		{
			return FALSE;
		}

		_paFastHeapAllocations[_nFastHeapAllocationIdx].pLocation = &_pAMEMFastMemory[_nFastBytesAllocated];
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nSize = nAMemSize;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].bFreed = FALSE;
		_paFastHeapAllocations[_nFastHeapAllocationIdx].nKey = _nAllocationKey;
		_nFastHeapAllocationIdx++;
		_nAllocationKey++;

		// Modify current high-water mark
		_nFastBytesAllocated += nAMemSize;
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
FAmem_Error_e CFAMemAccessor::Write( u32 nLocation, const void *pSourceData, u32 nSize, FAmemCallback_t *pCallback/*=NULL*/, void *pUser/*=NULL*/  )
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
	
	while ( TRUE )
	{
		if ( _nActiveARQRequestCount < _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS )
		{
			break;
		}
	}
	
	DCFlushRange( (void *)pSourceData, nSize );
	
	// Find the first available request
	u32 i;
	for ( i = 0; i < _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS; i++ )
	{
		if ( _paMemRequests[i].pAMemAlloc == NULL )
		{
			break;
		}		
	}
	FASSERT( i < _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS );
	
	_paMemRequests[i].pAMemAlloc = this;
	
	u32 nPriority;
	if ( m_nType == FAMEM_TYPE_SLOW )
	{
	
		nPriority = ARQ_PRIORITY_LOW;
	}
	else if ( m_nType == FAMEM_TYPE_FAST )
	{
		nPriority = ARQ_PRIORITY_HIGH;
	}
	else
	{
		FASSERT_NOW;
	}
	
	_paMemRequests[i].pCallback = pCallback;
	_paMemRequests[i].pUser = pUser;
	
//	DEVPRINTF( "CFAMemAccessor::Write() - ARAM write operation started.\n" );
	
	_nActiveARQRequestCount++;
	ARQPostRequest( (ARQRequest *)&_paMemRequests[i].Request, 			// ARQRequest
					i, 										// Owner
					ARQ_TYPE_MRAM_TO_ARAM, 					// Type
					nPriority, 								// Priority
					(u32)pSourceData,						// Source memory address
					(u32)pAlloc->pLocation + nLocation,		// Destination memory address
					nSize,									// transaction size
					_ARQCallback );							// Callback

	// If we're provided with a callback, then return immediately
	if ( pCallback )
	{
		return FAMEM_ERROR_NONE;
	}
	
	// No callback provided, so we need to block until we receive the results
	while ( TRUE )
	{
		if ( _paMemRequests[i].pAMemAlloc == NULL )
		{
			break;
		}
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
	if ( nLocation < 0 || nLocation > pAlloc->nSize || nLocation + nSize > pAlloc->nSize )
	{
		return FAMEM_ERROR_OUT_OF_BOUNDS_WRITE;
	}

	// Execute the read
	while ( TRUE )
	{
		if ( _nActiveARQRequestCount < _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS )
		{
			break;
		}
	}

	// Invalidate the current destination buffer to make sure noone is using it.	
	DCInvalidateRange( pDest, nSize );
	
	// Find the first available request
	u32 i;
	for ( i = 0; i < _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS; i++ )
	{
		if ( _paMemRequests[i].pAMemAlloc == NULL )
		{
			break;
		}		
	}
	FASSERT( i < _GC_MAX_SIMULTANEOUS_ARQ_REQUESTS );
	
	_paMemRequests[i].pAMemAlloc = this;
	
	u32 nPriority;
	if ( m_nType == FAMEM_TYPE_SLOW )
	{
		nPriority = ARQ_PRIORITY_LOW;
	}
	else if ( m_nType == FAMEM_TYPE_FAST )
	{
		nPriority = ARQ_PRIORITY_HIGH;
	}
	else
	{
		FASSERT_NOW;
	}
	
	_paMemRequests[i].pCallback = pCallback;
	_paMemRequests[i].pUser = pUser;
	
//	DEVPRINTF( "CFAMemAccessor::Read() - ARAM read operation started.\n" );
	
	_nActiveARQRequestCount++;
	ARQPostRequest( (ARQRequest *)&_paMemRequests[i].Request, 			// ARQRequest
					i, 										// Owner
					ARQ_TYPE_ARAM_TO_MRAM, 					// Type
					nPriority, 								// Priority
					(u32)pAlloc->pLocation + nLocation,		// Source memory address
					(u32)pDest,								// Destination memory address
					nSize,									// transaction size
					_ARQCallback );							// Callback
					
	// If we're provided with a callback, then return immediately
	if ( pCallback )
	{
		return FAMEM_ERROR_NONE;
	}
	
	// No callback provided, so we need to block until we receive the results
	while ( TRUE )
	{
		if ( _paMemRequests[i].pAMemAlloc == NULL )
		{
			break;
		}
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
