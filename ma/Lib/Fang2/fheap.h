//////////////////////////////////////////////////////////////////////////////////////
// fheap.h - Fang heap module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 10/12/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FHEAP_H_
#define _FHEAP_H_ 1

#include "fang.h"


enum {
	FHEAP_LOWER		= 0,
	FHEAP_UPPER		= 1
};

#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	#define FHEAP_TRACK_MEM_ALLOCATIONS		TRUE
	#define FHEAP_DETAILED_TRACKING			TRUE
#elif FANG_RELEASE_BUILD
	#define FHEAP_TRACK_MEM_ALLOCATIONS		TRUE
	#define FHEAP_DETAILED_TRACKING			FALSE
#else
	#define FHEAP_TRACK_MEM_ALLOCATIONS		FALSE
	#define FHEAP_DETAILED_TRACKING			FALSE
#endif

typedef void *FHeapFrame_t;

#if FHEAP_TRACK_MEM_ALLOCATIONS
	extern void fheap_TrackMem( cchar *pszFileName, u32 nLineNum, u32 nBytes );
	extern void fheap_UnTrackMem( FHeapFrame_t Frame );
	extern void fheap_UnTrackTexMem( void *pTexInfo );
	
	#if FHEAP_DETAILED_TRACKING
		extern void fheap_ShowMemoryTracking( void );
		extern void fheap_ShowMemoryTracking( BOOL bShowModuleDetails, BOOL bShowByModule, BOOL bShowTexAllocs, BOOL bDumpToFile, cchar* pszFileName ); 
	#endif
	
	#define FHEAP_MEM_TRACKERS_POOL_SIZE		30000
	#define FHEAP_TEX_MEM_TRACKERS_POOL_SIZE	512
	#define FHEAP_MAX_MEM_TRACKER_NAME_LEN		16
	
	struct MemTracker_t
	{
	#if FHEAP_DETAILED_TRACKING
		cchar*			pszPrimaryFileName;
		cchar*			pszSecondaryFileName;
		u16				nPrimaryLineNumber;
		u16				nSecondaryLineNumber;
		u16				bConsidered;
	#endif
		char			szResourceType[8];
		u32				nMemAllocated;
		FHeapFrame_t	Frame;
	};
	
	struct TexMemTracker_t
	{
	#if FHEAP_DETAILED_TRACKING
		cchar*			pszFormatName;
		cchar*			pszRequestingModuleName;
	#endif
		void			*pTexInfo;
		u32				nXBMemAllocated;
		u32				nPCMemAllocated;
		u32				nGCMemAllocated;
		u32				nPS2MemAllocated;	//ARG
	};
	
	extern char Fheap_szResourceType[8];
	extern cchar* Fheap_pszSecondaryFileName;
	extern u16 Fheap_nSecondaryLineNumber;
	extern u32 Fheap_nTotalMemTracked;
	extern u32 Fheap_nTotalPCTexMemTracked;
	extern u32 Fheap_nTotalXBTexMemTracked;
	extern u32 Fheap_nTotalGCTexMemTracked;
	extern u32 Fheap_nTotalPS2TexMemTracked;	//ARG
	extern u32 Fheap_nTotalAnimMemTracked;
	extern u32 Fheap_nTotalSoundMemTracked;
	extern u32 Fheap_nTotalMeshMemTracked;
	extern TexMemTracker_t Fheap_nTexTrackerPool[FHEAP_TEX_MEM_TRACKERS_POOL_SIZE];
	extern u32 Fheap_nTexPoolCount;
	extern MemTracker_t Fheap_nTrackerPool[FHEAP_MEM_TRACKERS_POOL_SIZE];
	extern u32 Fheap_nPoolCount;
	extern BOOL Fheap_bTrackingInvalid;
	
#endif // FHEAP_TRACK_MEM_ALLOCATIONS


class CFHeap {
public:
	CFHeap();
	CFHeap( u32 nBytes, void *pMemBase=NULL );
	~CFHeap();

	BOOL Init( u32 nBytes, void *pMemBase=NULL );
	void Free( void );
	FINLINE BOOL IsInit( void );

	FINLINE u32 GetFreeBytes( void );

	FINLINE FHeapFrame_t GetFrame( u32 nHeapNum );
	FINLINE FHeapFrame_t GetFrameHeap0( void );
	FINLINE FHeapFrame_t GetFrameHeap1( void );

	void ReleaseFrame( u32 nHeapNum, FHeapFrame_t Frame );
	void ReleaseFrameHeap0( FHeapFrame_t Frame );
	void ReleaseFrameHeap1( FHeapFrame_t Frame );

	void *Alloc( u32 nHeapNum, u32 nBytes, u32 nByteAlignment );
	void *AllocAndZero( u32 nHeapNum, u32 nBytes, u32 nByteAlignment );

	void *AllocHeap0( u32 nBytes, u32 nByteAlignment );
	void *AllocHeap1( u32 nBytes, u32 nByteAlignment );
	void *AllocAndZeroHeap0( u32 nBytes, u32 nByteAlignment );
	void *AllocAndZeroHeap1( u32 nBytes, u32 nByteAlignment );

	void *AllocHeap0( u32 nBytes, u32 nByteAlignment, u32 nOffset );
	void *AllocAndZeroHeap0( u32 nBytes, u32 nByteAlignment, u32 nOffset );
	void *AllocHeap1( u32 nBytes, u32 nByteAlignment, u32 nOffset );
	void *AllocAndZeroHeap1( u32 nBytes, u32 nByteAlignment, u32 nOffset );

	void *AllocAllHeap0( u32 nByteAlignment, u32 *pnBytesAllocated );
	BOOL ResizeHeap0( void *pMemBlock, u32 nNewSizeInBytes );
	BOOL ResizeHeap1ToNewBase( void *pOldBase, void *pNewBase );

protected:
	BOOL m_bWeAlloced;		// TRUE if we allocated the memory, or FALSE if it was provided to us
	u32 m_nHeapBytes;		// Total heap size in bytes (0=heap not initialized)
	u32 m_anHeapExtents[2];	// [0]=start of heap, [1]=one byte past end of heap
	u32 m_anHeap[2];		// [0]=lower, [1]=upper
};


FINLINE BOOL CFHeap::IsInit( void ) {
	if( m_nHeapBytes ) {
		return TRUE;
	}

	return FALSE;
}

FINLINE u32 CFHeap::GetFreeBytes( void ) {
	FASSERT( m_nHeapBytes );

	return (m_anHeap[1] - m_anHeap[0]);
}

FINLINE FHeapFrame_t CFHeap::GetFrame( u32 nHeapNum ) {
	FASSERT( m_nHeapBytes );
	FASSERT( nHeapNum < 2 );

	return (FHeapFrame_t)m_anHeap[nHeapNum];
}

FINLINE FHeapFrame_t CFHeap::GetFrameHeap0( void ) {
	FASSERT( m_nHeapBytes );
	return (FHeapFrame_t)m_anHeap[0];
}

FINLINE FHeapFrame_t CFHeap::GetFrameHeap1( void ) {
	FASSERT( m_nHeapBytes );
	return (FHeapFrame_t)m_anHeap[1];
}



#endif

