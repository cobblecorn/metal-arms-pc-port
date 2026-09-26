//////////////////////////////////////////////////////////////////////////////////////
// fAMem.h - Auxiliary memory manager for fang
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

#ifndef _FAMEM_H_
#define _FAMEM_H_ 1

#include "fang.h"


#define FAMEM_MAX_ALLOCATIONS			16

#define FAMEM_INVALID_KEY				-1


//
//
typedef enum
{
	FAMEM_TYPE_FAST = 0,
	FAMEM_TYPE_SLOW,
	FAMEM_TYPE_COUNT,

	FAMEM_TYPE_INVALID = 0xff,

} FAmem_Type_e;

//
//
typedef enum
{
	FAMEM_OP_READ = 0,
	FAMEM_OP_WRITE,
	FAMEM_OP_CLEAR,

	FAMEM_OP_COUNT

} FAmem_Operation_e;

//
//
typedef enum
{
	FAMEM_ERROR_NONE = 0,
	FAMEM_ERROR_OUT_OF_BOUNDS_READ,
	FAMEM_ERROR_OUT_OF_BOUNDS_WRITE,
	FAMEM_ERROR_MEMORY_FREED,
	FAMEM_ERROR_INVALID_ALLOCATION_KEY,
	FAMEM_ERROR_INVALID_ALIGNMENT,
	FAMEM_ERROR_GENERAL,

	FAMEM_ERROR_COUNT

} FAmem_Error_e;


class CFAMemAccessor;


///////////////////////////////////////
// AMEM callback:
//
// IMPORTANT NOTE: The callback may be invoked while in one of the interface functions. For
// instance, the callback for a write may be invoked while the write request is still in famem_Write.
// Modification/Deletion of pSourceData (or the pDest for a read) is allowed in this case.
typedef void FAmemCallback_t( FAmem_Operation_e nOp, CFAMemAccessor *pAMemAccessor, void *pUser, FAmem_Error_e nError );


///////////////////////////////////////
// CFAMemAccessor:  The user interface for the auxiliary memory system
//
// The accessor provides an interface to reserving, reading and storing data
// in auxiliary memory.  It is designed to operate asynchronously though it
// can function synchronously for those clients that are not heavily concerned
// with speed.  In order to work asynchronously with a number of simultaneous
// calls, the famem system relies on the client providing aligned reads 
// and source and destination buffers.  This may introduce some complexity
// on the client side, but minimizes overallocation in the famem system.
//
// It is important to attempt to make reads and writes in as large a block
// as possible.  More numerous reads will take longer, particularly where
// the famem system has to contend with other drivers for DMA resources.
//
// Make sure you review the notes related to the callback, above.
//
// You MUST call Free() or delete the class instance prior to the famem module shutdown
//
FCLASS_NOALIGN_PREFIX class CFAMemAccessor
{
	private:
		FAmem_Type_e	m_nType;
		void			*m_pPlatformData;
		s32				m_nKey;

	public:
		CFAMemAccessor( void )
		{
			m_nType = FAMEM_TYPE_INVALID;
			m_pPlatformData = NULL;
			m_nKey = -1;
		}
		~CFAMemAccessor( void );

		// nAMemSize must be a multiple of 32/
		// If speed is not necessary, nType should be set to FAMEM_TYPE_SLOW
		BOOL Init( u32 nAMemSize, FAmem_Type_e nType );
		
		// Releases the accessor's claim to the memory that was previously earmarked.
		FAmem_Error_e Free( void );

		// Memory access functions.  If no callback is provided, the functions will be 
		// synchronous.  If the callback is provided, the function will be asynchronous.  
		//		nLocation  - offset into the initialized data (MUST be a multiple of 32)
		//		pSourceData && pDest must be 32 byte aligned.
		//		nSize - amount to write/read (MUST be in increments of 32 bytes)
		FAmem_Error_e Write( u32 nLocation, const void *pSourceData, u32 nSize, FAmemCallback_t *pCallback = NULL, void *pUser = NULL );
		FAmem_Error_e Read( u32 nLocation, void *pDest, u32 nSize, FAmemCallback_t *pCallback = NULL, void *pUser = NULL );

		// Verify is a blocking function. nLocation, pData, and nSize do not need to be aligned.
		// returns -1 if the memory is the same, otherwise it returns the index of the first differing byte
		s32 Verify( u32 nLocation, void *pData, u32 nSize );

	FCLASS_STACKMEM_NOALIGN( CFAMemAccessor )

} FCLASS_NOALIGN_SUFFIX;


///////////////////////////////////////
// Module Startup/Shutdown
extern BOOL famem_ModuleStartup( void );
extern void famem_ModuleShutdown( void );

// Module Status
extern u32 famem_GetFreeAMemBytes( FAmem_Type_e nType );


#endif // _FAMEM_H_
