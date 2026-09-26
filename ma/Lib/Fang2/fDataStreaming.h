//////////////////////////////////////////////////////////////////////////////////////
// fDataStreaming.h - Fang module used to stream data from storage systems
//
// Author: John Lafleur   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/04/03 Lafleur		Created
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDATASTREAMING_H_
#define _FDATASTREAMING_H_ 1

#include "fang.h"
#include "famem.h"
#include "fvid.h"
#include "ffile.h"


#define FDS_DEFAULT_LOCAL_CACHE_SIZE	(2.0 * (1024 * 1024))
#if FANG_PLATFORM_XB
	#define FDS_MINIMUM_LOCAL_CACHE_SIZE	(1.7 * (1024 * 1024))
#else
	#define FDS_MINIMUM_LOCAL_CACHE_SIZE	(1.6 * (1024 * 1024))
#endif


class CFStreamMgr;
class CFAllocHandle;


//
//
//
struct MemBlock_t
{
	u8		nFlags;
	u8		__PAD[3];
	u8		*pStartAddress;
	u32		nMemoryInChain;	// How much memory is in this memory chain

	MemBlock_t *pPriorListBlock;
	MemBlock_t *pNextListBlock;

	MemBlock_t *pPriorMemBlock;
	MemBlock_t *pNextMemBlock;
};


//
//
//
FCLASS_NOALIGN_PREFIX class CFBlockMgr
{
	protected:
		enum
		{
			BLOCK_FLAG_NONE = 0x00,
			BLOCK_FLAG_USED = 0x01,
		};
		BOOL	m_bInitialized;
			
		u8  	*m_pMemory;
		u32		m_nManagedBytes;
		u32		m_nBytesInCache;
		u32		m_nMinBlockSize;
		u16 	m_nBlockCount;
		
		u16		m_nUnusedBlockHeaderCount;
		u16		m_nFreeBlockCount;
		u16		m_nAllocatedBlockCount;
		MemBlock_t *m_pFreeBlocks;
		MemBlock_t *m_pAllocatedBlocks;
		MemBlock_t *m_pUnusedBlockHeaders;
		MemBlock_t *m_pMemoryBlocks;
		
		MemBlock_t *m_paBlocks;
		
	protected:
		CFBlockMgr( void )
		{
			m_bInitialized = FALSE;
			m_pMemory = NULL;
			m_paBlocks = NULL;
			m_pFreeBlocks = NULL;
			m_pAllocatedBlocks = NULL;
			m_pMemoryBlocks = NULL;
			m_pUnusedBlockHeaders = NULL;
		}
		BOOL  InitLocalCache( u32 nMinBlockSize, u32 nMaxAllocations );
		void  UninitLocalCache( void );
		void  ResetMemoryBlocks( void );
		void  FreeMemory( MemBlock_t *pAllocation );
		MemBlock_t* AllocMemory( u32 nBytes );
		
		virtual void FlushCache( void );
		void DrawStatus( u32 nTotalRemoteBytes = 0, u32 nAllocatedRemoteBytes = 0 );
		
	private:
		
		void VerifyAllLists( void );
		
		FINLINE void RemoveMemBlockFromList( MemBlock_t **ppListHead, MemBlock_t *pBlock );
		FINLINE void AppendMemBlockToList( MemBlock_t **ppListHead, MemBlock_t *pBlock );
		FINLINE void InsertMemBlockInList( MemBlock_t **ppListHead, MemBlock_t *pChainHead, MemBlock_t *pInsertBefore );
		
	FCLASS_STACKMEM_NOALIGN( CFBlockMgr )
} FCLASS_NOALIGN_SUFFIX;


//
//
//
FCLASS_NOALIGN_PREFIX class CFAllocHandle
{
	friend class CFStreamMgr;
	private:
		enum
		{
			FLAG_STREAMING_IN	= 0x01,
			FLAG_IN_CACHE		= 0x02,
		};

		u8		nFlags;
		u8		nLocation;
		u16		nStreamStartOffset;	// For streaming from the master file, this represents the offset from the start of the file
		u32		nAllocationSize;
		u32		nLastAccessFrame;
		MemBlock_t*	pLocalMemory;
		
		union
		{
			u32	nARAMAddress;	// For streaming from auxilliary memory
			const FFileMasterEntry_t	*pMFFileEntry;	// For streaming from the master file
		};

		CFAllocHandle *pNext;

	FCLASS_STACKMEM_NOALIGN( CFAllocHandle )
} FCLASS_NOALIGN_SUFFIX;


//
//
//
typedef enum
{
	FDS_LOCATION_ARAM = 0,		// Remote data is streamed in from auxilliary memory
	FDS_LOCATION_MASTERFILE,	// Remote data is streamed in from the master file

} FDS_Remote_Location_e;


//
//
//
FCLASS_NOALIGN_PREFIX class CFStreamMgr : public CFBlockMgr
{
	public:
		CFStreamMgr( void )
		{
			m_bMgrInitialized = FALSE;
			m_nAMEMBytesTotal = 0;
			m_nAMEMBytesAllocated = 0;
			m_paAllocationHandles = NULL;
			m_nUsedHandleCount = 0;
			m_pAccessor = NULL;
			m_pLocalListHead = m_pLocalListTail = NULL;
			m_nLocalListCount = 0;
			m_nPostPauseFlushDelay = 0;
		}
		
		// Maintenance functions
		static BOOL ModuleStartup( void );
		static void ModuleShutdown( void );
		static void BlockOnStreaming( void );

//		BOOL  SetCacheSize( u32 nCacheSizeInBytes );
		BOOL  Init( u32 nCacheSizeInBytes );
		void  Uninit( void );
		void  Update( void );
		void  FlushCache( void );
		void  DrawStatus( void )
		{
			CFBlockMgr::DrawStatus( m_nAMEMBytesTotal, m_nAMEMBytesAllocated );
		}			
		
		// Functions to make use of the cache for temporary memory allocations
		BOOL  LockCache( void );
		BOOL  UnlockCache( void );
		void* AllocFromCache( cu32 nBytes );
		
		// Functions used to stream data to and from auxiliary memory
		BOOL  StoreRawData( const void *pDataPointer, u32 nDataSize, FDS_Remote_Location_e nLocation );
		CFAllocHandle* StoreData( const void *pDataPointer, u32 nDataSize, FDS_Remote_Location_e nLocation );
		CFAllocHandle* GetDataHandle( u32 nRemoteMemoryOffset, u32 nDataSize, FDS_Remote_Location_e nLocation );
		CFAllocHandle* StreamFromMasterfile( cchar *pszFilename, u32 nOffsetFromStart, s32 nBytesToStream = -1 );
		void CacheData( void *pDataPointer ) { AccessData( pDataPointer ); }
		BOOL DataIsCached (void *pDataPointer ) 
		{
			if ( ! pDataPointer )
			{
				return FALSE;
			}
			
			FASSERT( (u32)pDataPointer >= (u32)m_paAllocationHandles 
						&& (u32)pDataPointer <(u32)m_paAllocationHandles + (sizeof( CFAllocHandle ) * m_nMaxHandles) );
						
			return !!(((CFAllocHandle *)pDataPointer)->nFlags & CFAllocHandle::FLAG_IN_CACHE);
		}
		void* AccessData( void *pDataPointer )
		{
			if ( !pDataPointer )
			{
				return NULL;
			}
			
			FASSERT( (u32)pDataPointer >= (u32)m_paAllocationHandles 
						&& (u32)pDataPointer <(u32)m_paAllocationHandles + (sizeof( CFAllocHandle ) * m_nMaxHandles) );

			CFAllocHandle *pHandle = (CFAllocHandle *)pDataPointer;
			pHandle->nLastAccessFrame = FVid_nFrameCounter;
			if ( pHandle->pLocalMemory )
			{
				if ( !(pHandle->nFlags & CFAllocHandle::FLAG_STREAMING_IN) )
				{
					// Data is already local, so just return the pointer to the block
					return pHandle->pLocalMemory->pStartAddress;
				}
				
				// Data is still streaming in from remote location
				return NULL;
			}
			
			StartStreaming( pHandle );
			return NULL;
		}
		
	private:
		BOOL			m_bMgrInitialized;
		BOOL			m_bCacheLocked;
		u32				m_nPostPauseFlushDelay;

		// AMEM memory vars
		u32 			m_nAMEMBytesTotal;
		u32				m_nAMEMBytesAllocated;
		u32				m_nAMEMBytesDeniedAlloc;

		// Allocation Handle management vars
		u32 			m_nMaxHandles;
		CFAllocHandle	*m_pLocalListHead;
		CFAllocHandle	*m_pLocalListTail;
		u32				m_nLocalListCount;
		CFAllocHandle	*m_paAllocationHandles;
		u32				m_nUsedHandleCount;
		CFAMemAccessor 	*m_pAccessor;

	private:		
		static void AMemCallback( FAmem_Operation_e nOp, CFAMemAccessor *pAMemAccessor, void *pUser, FAmem_Error_e nError );
		static void FFileCallback( s32 nErrorCode, FFileHandle hFile, void *pDestBuffer, u32 uReadAmt, void *pUser );
		
		void StartStreaming( CFAllocHandle *pHandle );
		
	FCLASS_STACKMEM_NOALIGN( CFStreamMgr )
} FCLASS_NOALIGN_SUFFIX;


// The single instance of the stream manager
extern CFStreamMgr FDS_StreamMgr;


#endif //_FDATASTREAMING_H_

