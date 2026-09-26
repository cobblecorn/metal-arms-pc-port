//////////////////////////////////////////////////////////////////////////////////////
// fres.cpp - Fang resource manager.
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
// 10/25/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fres.h"
#include "fheap.h"
#include "fclib.h"
#include "fstringtable.h"
#include "ffile.h"
#include "fmath.h"
#if FANG_PLATFORM_GC
	#include "fgc.h"
#endif

#if !FANG_PRODUCTION_BUILD
#include <stdio.h>
#endif

#if FANG_PLATFORM_XB
#include <xtl.h>
#endif


#define _FANG_MAX_NEWDEL_FILENAME	100



BOOL FRes_bModuleInitialized = FALSE;
CFHeap FRes_CFHeap;

BOOL _FRes_bTestForDestructor = FALSE;
BOOL _FRes_bHasDestructor = FALSE;

#if FHEAP_TRACK_MEM_ALLOCATIONS
static const char *_pszAllocFileName = NULL;
static u32 _nAllocLineNumber = 0;
#endif


#if FANG_DEBUG_BUILD || FANG_TEST_BUILD

	FResDebugStack_t FRes_aDebugStack[FRES_DEBUG_STACK_SIZE];
	u32 FRes_nDebugStackCount;

	void *_fres_Alloc( u32 nBytes );
	void *_fres_AllocAndZero( u32 nBytes );
	void *_fres_AllocWithOffset( u32 nBytes, u32 nOffset );
	void *_fres_AllocAndZeroWithOffset( u32 nBytes, u32 nOffset );

	void *_fres_AlignedAlloc( u32 nBytes, u32 nByteAlignment );
	void *_fres_AlignedAllocAndZero( u32 nBytes, u32 nByteAlignment );
	void *_fres_AlignedAllocWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );
	void *_fres_AlignedAllocAndZeroWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );

	void _fres_ReleaseFrame( FResFrame_t Frame );

#endif



#define _LOWERCASE( cChar ) ((cChar>='A' && cChar<='Z') ? (char)(cChar + ('a' - 'A')) : cChar)


class CFRes {
public:
	CFRes *pPrevResLink;					// Link to previous resources (must be the first field in CFRes)
	char sType[FRES_TYPELEN];				// Resource type (no NULL terminator)
	void *pBase;							// Pointer to resource base
	FResDestroyCallback_t *pFcnCallback;	// Callback function to call when destroying resource (NULL=none)

	// Note: The resource ASCIIZ name immediately follows this structure in memory!
};



static CFRes *_pLinkListTail;				// Points to the last CFRes in the linklist

#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	static u32 _nDeletedButNotReleasedCount;
	static u32 _nSupressStrandedObjectWarnings;
	static char _szNewDelResourceName[_FANG_MAX_NEWDEL_FILENAME+1];
	static void _CheckForStrandedClasses( cchar *pszFileName, u32 nLineNum );
	static void _BuildResourceString( cchar *pszClassName, cchar *pszFileName, u32 nLineNum );
	static void _DebugClassReleased( void *pResource );
#endif

// Local function prototypes
FResHandle_t _fres_Create( cchar *psResType, cchar *pszResName );
FResHandle_t _fres_CreateWithCallback( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback );
void *_fres_CreateAndAlloc( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes );
void *_fres_CreateAndAllocAndZero( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes );
void *_fres_CreateAndAllocWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes );
void *_fres_CreateAndAllocAndZeroWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes );
void *_fres_AlignedCreateAndAlloc( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes );
void *_fres_AlignedCreateAndAllocAndZero( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes );
void *_fres_AlignedCreateAndAllocWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes );
void *_fres_AlignedCreateAndAllocAndZeroWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes );


static CFRes *_Find( cchar *psType, cchar *pszName );
static CFRes *_Find( void *pBase );
static CFRes *_Find( void *pBase, FResDestroyCallback_t *pFcnCallback );


static FResFrame_t _FirstFrame;


BOOL fres_ModuleStartup( void ) {
	FASSERT( !FRes_bModuleInitialized );
	FASSERT_MSG( FANG_OFFSETOF( CFRes, pPrevResLink ) == 0, "fres_ModuleStartup(): CFRes::pPrevResLink must be the first field in CFRes." );

	if( Fang_ConfigDefs.nRes_HeapBytes == 0 ) {
		DEVPRINTF( "fres_ModuleStartup(): Fang_ConfigDefs.nRes_HeapBytes cannot be 0.\n" );
		return FALSE;
	}

	if( !FRes_CFHeap.Init( Fang_ConfigDefs.nRes_HeapBytes, Fang_ConfigDefs.pRes_HeapStart ) ) {
		DEVPRINTF( "fres_ModuleStartup(): FRes_CFHeap.Init() failed.\n" );
		return FALSE;
	}

#if FANG_PLATFORM_XB
#if !FANG_PRODUCTION_BUILD
	MEMORYSTATUS stat;
	GlobalMemoryStatus( &stat );

	char szTmp[256];
	sprintf(szTmp, "Free Memory on XB after CHeap::Init(): %d\n", stat.dwAvailPhys);
	OutputDebugString( szTmp );
#endif
#elif FANG_PLATFORM_GC
	s32 nBytesFree = OSCheckHeap( FGC_HeapHandle );
	OSReport( "Free Memory on GC after CHeap::Init(): %d\n", nBytesFree);
#endif

	_pLinkListTail = NULL;

	_FRes_bTestForDestructor = FALSE;
	_FRes_bHasDestructor = FALSE;

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
		FRes_nDebugStackCount = 0;
		_nDeletedButNotReleasedCount = 0;
		_nSupressStrandedObjectWarnings = 0;
	#endif
	
	FRes_bModuleInitialized = TRUE;

	#if FHEAP_TRACK_MEM_ALLOCATIONS
		BOOL _bAllocationRecorded = FALSE;
	#endif

	_FirstFrame = fres_GetFrame();
	
	return TRUE;
}


void fres_ModuleShutdown( void ) {
	FASSERT( FRes_bModuleInitialized );

	fres_ReleaseFrame( _FirstFrame );

	FASSERT_MSG( _pLinkListTail == NULL, "fres_ModuleShutdown(): All FRes resources must be freed before module is shut down." );
	
	FRes_CFHeap.Free();

	FRes_bModuleInitialized = FALSE;
}


//--------------------------------------------------------------------------------------------------------------------
// Resource Memory (upper block):
//--------------------------------------------------------------------------------------------------------------------

void fres_CopyType( char *psDestResType, cchar *psSrCFResType ) {
	u32 i;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psSrCFResType) );

	if( psDestResType ) {
		if( psSrCFResType == NULL ) {
			psSrCFResType = FRES_TYPE_NONE;
		}

		for( i=0; i<FRES_TYPELEN; i++ ) {
			psDestResType[i] = _LOWERCASE(psSrCFResType[i]);
		}
	}
}

BOOL fres_CompareTypes( cchar *psResType1, cchar *psResType2 ) {
	u32 i;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType1) );
	FASSERT( fres_IsTypeValid(psResType2) );

	if( psResType1 == NULL ) {
		psResType1 = FRES_TYPE_NONE;
	}
	if( psResType2 == NULL ) {
		psResType2 = FRES_TYPE_NONE;
	}

	for( i=0; i<FRES_TYPELEN; i++ ) {
		if( _LOWERCASE(psResType1[i]) != _LOWERCASE(psResType2[i]) ) {
			return FALSE;
		}
	}

	return TRUE;
}

BOOL fres_IsTypeValid( cchar *psResType ) {
	u32 i;

	FASSERT( FRes_bModuleInitialized );

	if( psResType == NULL ) {
		return TRUE;
	}

	for( i=0; i<FRES_TYPELEN; i++ ) {
		if( psResType[i] == 0 ) {
			// Type string is too short...
			return FALSE;
		}
	}

	// Valid...
	return TRUE;
}

void *fres_FindBase( cchar *psResType, cchar *pszResName ) {
	CFRes *pRes;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	pRes = _Find( psResType, pszResName );
	if( pRes ) {
		return pRes->pBase;
	}

	return NULL;
}

void *fres_FindBase( void *pBase ) {
	CFRes *pRes;

	FASSERT( FRes_bModuleInitialized );

	pRes = _Find( pBase );
	if( pRes ) {
		return pRes->pBase;
	}

	return NULL;
}

FResHandle_t fres_FindHandle( cchar *psResType, cchar *pszResName ) {
	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );
	return (FResHandle_t)_Find( psResType, pszResName );
}

FResHandle_t fres_FindHandle( void *pBase ) {
	FASSERT( FRes_bModuleInitialized );
	return (FResHandle_t)_Find( pBase );
}

FResHandle_t fres_FindHandle( void *pBase, FResDestroyCallback_t *pFcnCallback ) {
	FASSERT( FRes_bModuleInitialized );
	return (FResHandle_t)_Find( pBase, pFcnCallback );
}

void *fres_GetBase( FResHandle_t hRes ) {
	FASSERT( FRes_bModuleInitialized );

	if( hRes ) {
		return ((CFRes *)hRes)->pBase;
	} else {
		return NULL;
	}
}

FResDestroyCallback_t *fres_GetCallback( FResHandle_t hRes ) {
	FASSERT( FRes_bModuleInitialized );

	if( hRes ) {
		return ((CFRes *)hRes)->pFcnCallback;
	} else {
		return NULL;
	}
}

void fres_GetTypeNZ( FResHandle_t hRes, char *psDestBuf ) {
	FASSERT( FRes_bModuleInitialized );

	if( hRes ) {
		fres_CopyType( psDestBuf, ((CFRes *)hRes)->sType );
	} else {
		fang_MemSet( psDestBuf, ' ', FRES_TYPELEN );
	}
}

void fres_GetTypeZ( FResHandle_t hRes, char *pszDestBuf ) {
	FASSERT( FRes_bModuleInitialized );

	if( hRes ) {
		fres_CopyType( pszDestBuf, ((CFRes *)hRes)->sType );
		pszDestBuf[FRES_TYPELEN] = 0;
	} else {
		fang_MemSet( pszDestBuf, ' ', FRES_TYPELEN );
	}

	pszDestBuf[FRES_TYPELEN] = 0;
}

cchar *fres_GetName( FResHandle_t hRes ) {
	cchar *pszResName;

	FASSERT( FRes_bModuleInitialized );

	if( hRes ) {
		pszResName = (cchar *)( ((CFRes *)hRes) + 1 );

		if( *pszResName == 0 ) {
			return NULL;
		} else {
			return pszResName;
		}
	} else {
		return NULL;
	}
}

void fres_SetCallback( FResHandle_t hRes, FResDestroyCallback_t *pFcnDestroyCallback ) {
	FASSERT( FRes_bModuleInitialized );
	((CFRes *)hRes)->pFcnCallback = pFcnDestroyCallback;
}

void fres_SetBase( FResHandle_t hRes, void *pBase ) {
	FASSERT( FRes_bModuleInitialized );
	((CFRes *)hRes)->pBase = pBase;
}

void fres_SetBaseAndCallback( FResHandle_t hRes, void *pBase, FResDestroyCallback_t *pFcnDestroyCallback ) {
	FASSERT( FRes_bModuleInitialized );
	((CFRes *)hRes)->pBase = pBase;
	((CFRes *)hRes)->pFcnCallback = pFcnDestroyCallback;
}

FResHandle_t _fres_Create( cchar *psResType, cchar *pszResName ) {
	CFRes *pRes;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	if( _Find( psResType, pszResName ) ) {
		// Name already exists...
		DEVPRINTF( "fres_Create(): Resource '%s:%s' already exists. Cannot create another.\n", psResType, pszResName );
		return FRES_NULLHANDLE;
	}

	if( pszResName==NULL || *pszResName==0 ) {
		// No name specified...

		pRes = (CFRes *)_fres_Alloc( sizeof(CFRes) + 1 );
		if( pRes == NULL ) {
			// Not enough memory...
			return FRES_NULLHANDLE;
		}

		// No resource ASCIIZ name. Just store the terminator...
		*(char *)(pRes + 1) = 0;
	} else {
		pRes = (CFRes *)_fres_Alloc( sizeof(CFRes) + fclib_strlen( pszResName ) + 1 );
		if( pRes == NULL ) {
			// Not enough memory...
			return FRES_NULLHANDLE;
		}

		fclib_strcpy( (char *)(pRes + 1), pszResName );
	}

	pRes->pPrevResLink = _pLinkListTail;
	_pLinkListTail = pRes;

	pRes->pBase = NULL;
	pRes->pFcnCallback = NULL;
	fres_CopyType( pRes->sType, psResType );

	return (FResHandle_t)pRes;
}

FResHandle_t _fres_CreateWithCallback( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback ) {
	FResHandle_t hRes;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	hRes = _fres_Create( psResType, pszResName );
	if( hRes != FRES_NULLHANDLE ) {
		fres_SetCallback( hRes, pFcnDestroyCallback );
	}

	return hRes;
}

void *_fres_CreateAndAlloc( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_Alloc( nBytes );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_CreateAndAllocAndZero( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_AllocAndZero( nBytes );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_CreateAndAllocWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_AllocWithOffset( nBytes, nOffset );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_CreateAndAllocAndZeroWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_AllocAndZeroWithOffset( nBytes, nOffset );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_AlignedCreateAndAlloc( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_AlignedAlloc( nBytes, nByteAlignment );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_AlignedCreateAndAllocAndZero( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_AlignedAllocAndZero( nBytes, nByteAlignment );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_AlignedCreateAndAllocWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_AlignedAllocWithOffset( nBytes, nByteAlignment, nOffset );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_AlignedCreateAndAllocAndZeroWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes ) {
	FResFrame_t ResFrame;
	FResHandle_t hRes;
	void *pBase;

	FASSERT( FRes_bModuleInitialized );
	FASSERT( fres_IsTypeValid(psResType) );

	ResFrame = fres_GetFrame();

	hRes = _fres_Create( psResType, pszResName );
	if( hRes == FRES_NULLHANDLE ) {
		return NULL;
	}

	pBase = _fres_AlignedAllocAndZeroWithOffset( nBytes, nByteAlignment, nOffset );
	if( pBase == NULL ) {
		fres_ReleaseFrame( ResFrame );
		return NULL;
	}

	fres_SetBase( hRes, pBase );
	fres_SetCallback( hRes, pFcnDestroyCallback );

	if( phDestRes ) {
		*phDestRes = hRes;
	}

	return pBase;
}

void *_fres_Alloc( u32 nBytes ) {
	FASSERT( FRes_bModuleInitialized );
	
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return FRes_CFHeap.AllocHeap1( nBytes, 4 );
}

void *_fres_AllocAndZero( u32 nBytes ) {
	FASSERT( FRes_bModuleInitialized );
	
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return FRes_CFHeap.AllocAndZeroHeap1( nBytes, 4 );
}

void *_fres_AllocWithOffset( u32 nBytes, u32 nOffset ) {
	FASSERT( FRes_bModuleInitialized );
	
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return FRes_CFHeap.AllocHeap1( nBytes, 4, nOffset );
}

void *_fres_AllocAndZeroWithOffset( u32 nBytes, u32 nOffset ) {
	FASSERT( FRes_bModuleInitialized );
	
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return FRes_CFHeap.AllocAndZeroHeap1( nBytes, 4, nOffset );
}

void *_fres_AlignedAlloc( u32 nBytes, u32 nByteAlignment ) {
	FASSERT( FRes_bModuleInitialized );

	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	void *pMem = FRes_CFHeap.AllocHeap1( nBytes, nByteAlignment );

	if( pMem == NULL ) {
		return NULL;
	}

#if FANG_PLATFORM_XB || FANG_PLATFORM_WIN
	// This is a hack that gets around a compiler bug in the XDK.
	//
	// For aligned classes, the compiler handles new [] correctly
	// by adding 16 bytes to the number of bytes passed to new [],
	// storing the # array elements in the address returned by new [],
	// and then returning the address plus 16 bytes.
	//
	// However, delete [] doesn't work properly. The compiler generates
	// code that backs up only 4 bytes from the address passed to
	// delete [], and uses that address to extract the number of array
	// elements. This is uninitialized memory, so random behavior results.
	// The compiler examines the value pulled from this location and
	// calls "::free()" if it's zero, or the class's delete [] operator
	// if the value is not zero. Luckily, the preamble for the class's
	// delete [] operator correctly subtracts 16 from the address passed
	// to delete [] and so iterates through the correct number of
	// destructors. However, if "::free()" is called (and it should never
	// be for our memory management system in Fang), a GPF is thrown
	// because new [] didn't allocate memory in a way that's compatible
	// with "::free()".
	//
	// This hack simply initializes the first 16 bytes of the memory
	// block so that when the compiler examines the value that's 4 bytes
	// prior to the address passed to delete [], it finds a non-zero
	// value, which is just what we need to avoid the compiler calling
	// "::free()".
	//
	// This problem occurs only with Test, Release, and Production builds,

	if( nByteAlignment==16 && nBytes>=16 ) {
		fang_MemSet( pMem, 0xff, 16 );
	}
#endif

	return pMem;
}

void *_fres_AlignedAllocAndZero( u32 nBytes, u32 nByteAlignment ) {
	FASSERT( FRes_bModuleInitialized );
	
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
		_pszAllocFileName = NULL;
		_nAllocLineNumber = 0;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return FRes_CFHeap.AllocAndZeroHeap1( nBytes, nByteAlignment );
}

void *_fres_AlignedAllocWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	FASSERT( FRes_bModuleInitialized );
	
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
		_pszAllocFileName = NULL;
		_nAllocLineNumber = 0;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return FRes_CFHeap.AllocHeap1( nBytes, nByteAlignment, nOffset );
}

void *_fres_AlignedAllocAndZeroWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	FASSERT( FRes_bModuleInitialized );
	
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_TrackMem( _pszAllocFileName, _nAllocLineNumber, nBytes );
		_pszAllocFileName = NULL;
		_nAllocLineNumber = 0;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return FRes_CFHeap.AllocAndZeroHeap1( nBytes, nByteAlignment, nOffset );
}


void _fres_ReleaseFrame( FResFrame_t Frame ) {
	CFRes *pRes;
	u32 nFrameMemAddress;

	FASSERT( FRes_bModuleInitialized );

	if( Frame == NULL ) {
		return;
	}

	nFrameMemAddress = (u32)Frame;

	CFStringTable::ResFrameReleased( (void *)nFrameMemAddress );
	
	while( pRes = _pLinkListTail ) {
		if( (u32)pRes >= nFrameMemAddress ) {
			break;
		}

		if( pRes->pFcnCallback ) {
			pRes->pFcnCallback( pRes->pBase );
		}

		_pLinkListTail = pRes->pPrevResLink;
	}

	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_UnTrackMem( Frame );
	#endif

	FRes_CFHeap.ReleaseFrameHeap1( (FHeapFrame_t)Frame );
}



#if FANG_DEBUG_BUILD || FANG_TEST_BUILD

FResHandle_t fres_Debug_Create( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_Create( psResType, pszResName );
}

FResHandle_t fres_Debug_CreateWithCallback( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_CreateWithCallback( psResType, pszResName, pFcnDestroyCallback );
}

void *fres_Debug_CreateAndAlloc( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_CreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes );
}

void *fres_Debug_CreateAndAllocAndZero( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_CreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes );
}

void *fres_Debug_CreateAndAllocWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_CreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes );
}

void *fres_Debug_CreateAndAllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_CreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes );
}


void *fres_Debug_AlignedCreateAndAlloc( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedCreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes );
}

void *fres_Debug_AlignedCreateAndAllocAndZero( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedCreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes );
}

void *fres_Debug_AlignedCreateAndAllocWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedCreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes );
}

void *fres_Debug_AlignedCreateAndAllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedCreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes );
}


void *fres_Debug_Alloc( cchar *pszFileName, u32 nLineNum, u32 nBytes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_Alloc( nBytes );
}

void *fres_Debug_AllocAndZero( cchar *pszFileName, u32 nLineNum, u32 nBytes ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AllocAndZero( nBytes );
}

void *fres_Debug_AllocWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nOffset ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AllocWithOffset( nBytes, nOffset );
}

void *fres_Debug_AllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nOffset ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AllocAndZeroWithOffset( nBytes, nOffset );
}


void *fres_Debug_AlignedAlloc( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedAlloc( nBytes, nByteAlignment );
}

void *fres_Debug_AlignedAllocAndZero( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedAllocAndZero( nBytes, nByteAlignment );
}

void *fres_Debug_AlignedAllocWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedAllocWithOffset( nBytes, nByteAlignment, nOffset );
}

void *fres_Debug_AlignedAllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	_CheckForStrandedClasses( pszFileName, nLineNum );
	return _fres_AlignedAllocAndZeroWithOffset( nBytes, nByteAlignment, nOffset );
}

void fres_Debug_ReleaseFrame( cchar *pszFileName, u32 nLineNum, FResFrame_t Frame ) {
	CFRes *pRes;
	u32 nFrameMemAddress;

	FASSERT( FRes_bModuleInitialized );

	if( Frame == NULL ) {
		return;
	}

	nFrameMemAddress = (u32)Frame;

	CFStringTable::ResFrameReleased( (void *)nFrameMemAddress );

	for( pRes=_pLinkListTail; pRes; pRes=pRes->pPrevResLink ) {
		if( (u32)pRes >= nFrameMemAddress ) {
			break;
		}

		if( pRes->pFcnCallback ) {
			pRes->pFcnCallback( pRes->pBase );
		}
	}

	while( pRes = _pLinkListTail ) {
		if( (u32)pRes >= nFrameMemAddress ) {
			break;
		}

		if( pRes->pFcnCallback == NULL ) {
			if( fres_CompareTypes( FRES_CLASS_RESOURCE, pRes->sType ) ) {
				// We found a C++ memory block for a class that has a destructor
				// and that has not been deleted but is now being released...

				DEVPRINTF( "------------------------------------------------------------------------------------------------------\n" );
				DEVPRINTF( "FRES: Undeleted C++ class detected!  Data corruption is possible.\n" );
				DEVPRINTF( "Use the 'delete' operator on this C++ object:\n" );
				DEVPRINTF( "  %s\n", (cchar *)(pRes + 1) );
				DEVPRINTF( "before the fres_ReleaseFrame() call is made on line %u of file '%s'.\n", nLineNum, fclib_FindFileNameInPath(pszFileName) );
				DEVPRINTF( "------------------------------------------------------------------------------------------------------\n" );
			}
		}

		_pLinkListTail = pRes->pPrevResLink;
	}

	#if FHEAP_TRACK_MEM_ALLOCATIONS
		fheap_UnTrackMem( Frame );
	#endif

	FRes_CFHeap.ReleaseFrameHeap1( (FHeapFrame_t)Frame );
}


// Checks for C++ classes (with destructors) that have been deleted but not released.
static void _CheckForStrandedClasses( cchar *pszFileName, u32 nLineNum ) {
	if( _nDeletedButNotReleasedCount == 0 ) {
		// No stranded classes...
		return;
	}

	// We found at least one stranded class...

	CFRes *pRes;

	for( pRes=_pLinkListTail; pRes; pRes=pRes->pPrevResLink ) {
		if( fres_CompareTypes( FRES_CLASS_RESOURCE, pRes->sType ) ) {
			// Found C++ object resource...

			if( pRes->pFcnCallback ) {
				// We found a C++ memory block for a class that has a destructor
				// and that has been deleted but not released...

				DEVPRINTF( "------------------------------------------------------------------------------------------------------\n" );
				DEVPRINTF( "FRES: Stranded C++ class detected!  Possible waste of memory.\n" );
				DEVPRINTF( "Use fres_ReleaseFrame() to release this C++ object:\n" );
				DEVPRINTF( "  %s\n", (cchar *)(pRes + 1) );
				DEVPRINTF( "before the allocation is made on line %u of file '%s'.\n", nLineNum, fclib_FindFileNameInPath(pszFileName) );
				DEVPRINTF( "------------------------------------------------------------------------------------------------------\n" );

				FASSERT( _nDeletedButNotReleasedCount > 0 );
				_nDeletedButNotReleasedCount--;
				pRes->pFcnCallback = NULL;
				fres_CopyType( pRes->sType, FRES_TYPE_NONE );
			}
		}
	}
}

static void _BuildResourceString( cchar *pszClassName, cchar *pszFileName, u32 nLineNum ) {
	if( pszFileName ) {
#if FANG_PLATFORM_GC
		snprintf( _szNewDelResourceName, _FANG_MAX_NEWDEL_FILENAME, "Class '%s', File '%s', Line %u", pszClassName, fclib_FindFileNameInPath(pszFileName), nLineNum );
#else
		_snprintf( _szNewDelResourceName, _FANG_MAX_NEWDEL_FILENAME, "Class '%s', File '%s', Line %u", pszClassName, fclib_FindFileNameInPath(pszFileName), nLineNum );
#endif
		_szNewDelResourceName[_FANG_MAX_NEWDEL_FILENAME] = 0;
	} else {
		_szNewDelResourceName[0] = 0;
	}
}

static void _DebugClassReleased( void *pResource ) {
	FASSERT( _nDeletedButNotReleasedCount );
	_nDeletedButNotReleasedCount--;
}

void *fres_DebugNewWithDestructor( u32 nBytesToAlloc, u32 nByteAlignment, cchar *pszClassName, u32 nClassByteCount, cchar *pszFileName, u32 nLineNum ) {
	FResFrame_t ResFrame = fres_GetFrame();
	FResHandle_t hRes;
	void *pBase;

	FASSERT( fmath_IsPowerOf2( nByteAlignment, FALSE ) );

	_CheckForStrandedClasses( pszFileName, nLineNum );
	_nSupressStrandedObjectWarnings++;

	_BuildResourceString( pszClassName, pszFileName, nLineNum );

	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
#if !FANG_PLATFORM_GC // There is no means to determine if a class has a destructor on GC, so there is no reason to allocate a resource to track it	
	hRes = _fres_Create( FRES_CLASS_RESOURCE, _szNewDelResourceName );
	if( hRes == FRES_NULLHANDLE ) {
		goto _ExitDebugNewWithError;
	}
#endif // !FANG_PLATFORM_GC

	pBase = _fres_AlignedAlloc( nBytesToAlloc, nByteAlignment );
	if( pBase == NULL ) {
		goto _ExitDebugNewWithError;
	}

#if !FANG_PLATFORM_GC // There is no means to determine if a class has a destructor on GC, so there is no reason to allocate a resource to track it	
	// Success:
	fres_SetBase( hRes, pBase );
#endif // !FANG_PLATFORM_GC
	
	_nSupressStrandedObjectWarnings--;

	return pBase;

	// Error:
_ExitDebugNewWithError:
	fres_ReleaseFrame( ResFrame );
	_nSupressStrandedObjectWarnings--;
	return FALSE;
}

void *fres_DebugNewArrayWithDestructor( u32 nBytesToAlloc, u32 nByteAlignment, u32 nByteOffset, cchar *pszClassName, u32 nClassByteCount, cchar *pszFileName, u32 nLineNum ) {
	FASSERT( fmath_IsPowerOf2( nByteAlignment, FALSE ) );

	FResFrame_t ResFrame = fres_GetFrame();
	FResHandle_t hRes;
	void *pBase;

	_CheckForStrandedClasses( pszFileName, nLineNum );
	_nSupressStrandedObjectWarnings++;

	_BuildResourceString( pszClassName, pszFileName, nLineNum );

	#if FHEAP_TRACK_MEM_ALLOCATIONS
		_pszAllocFileName = pszFileName;
		_nAllocLineNumber = nLineNum;
	#endif // FHEAP_TRACK_MEM_ALLOCATIONS

#if !FANG_PLATFORM_GC // There is no means to determine if a class has a destructor on GC, so there is no reason to allocate a resource to track it	
	hRes = _fres_Create( FRES_CLASS_RESOURCE, _szNewDelResourceName );
	if( hRes == FRES_NULLHANDLE ) {
		goto _ExitDebugNewWithError;
	}
#endif	

	pBase = _fres_AlignedAllocWithOffset( nBytesToAlloc, nByteAlignment, nByteOffset );
	if( pBase == NULL ) {
		goto _ExitDebugNewWithError;
	}

#if FANG_PLATFORM_XB || FANG_PLATFORM_WIN
	// This is a hack that gets around a compiler bug in the XDK.
	// See comment for _fres_AlignedAlloc().

	if( nByteAlignment==16 && nBytesToAlloc>=16 ) {
		fang_MemSet( pBase, 0xff, 16 );
	}
#endif

#if !FANG_PLATFORM_GC // There is no means to determine if a class has a destructor on GC, so there is no reason to allocate a resource to track it	
	// Success:
	fres_SetBase( hRes, pBase );
#endif

	_nSupressStrandedObjectWarnings--;

	return pBase;

	// Error:
_ExitDebugNewWithError:
	fres_ReleaseFrame( ResFrame );
	_nSupressStrandedObjectWarnings--;
	return FALSE;
}

void fres_DebugDelete( void *pBase, u32 nByteAlignment, cchar *pszClassName, u32 nClassByteCount ) {
#if !FANG_PLATFORM_GC
	CFRes *pRes;
	if( pBase == NULL ) {
		return;
	}

	pRes = _Find( pBase );
	if( pRes == NULL ) {
		// This memory block has not been registered as a resource. This class has no destructor.
		return;
	}

	FASSERT( fres_CompareTypes( pRes->sType, FRES_CLASS_RESOURCE ) );

	// A destructor for this class exists...

	if( pRes->pFcnCallback ) {
		// This memory block has already been deleted...
		DEVPRINTF( "------------------------------------------------------------------------------------------------------\n" );
		DEVPRINTF( "FRES: Attempt to 'delete' a C++ object more than once!\n" );
		DEVPRINTF( "Object allocation: %s\n", (cchar *)(pRes + 1) );

		if( FRes_nDebugStackCount && FRes_aDebugStack[FRes_nDebugStackCount-1].pszFilename ) {
			DEVPRINTF( "2nd 'delete' is on line %u of '%s'.\n", FRes_aDebugStack[FRes_nDebugStackCount-1].nLineNum, fclib_FindFileNameInPath(FRes_aDebugStack[FRes_nDebugStackCount-1].pszFilename) );
		}

		DEVPRINTF( "------------------------------------------------------------------------------------------------------\n" );
		return;
	}

	pRes->pFcnCallback = _DebugClassReleased;

	_nDeletedButNotReleasedCount++;
#endif
}
#endif


static CFRes *_Find( cchar *psType, cchar *pszName ) {
	CFRes *pRes;

	FASSERT( fres_IsTypeValid(psType) );

	if( pszName==NULL || *pszName==0 ) {
		return NULL;
	}

	if( fres_CompareTypes( psType, FRES_CLASS_RESOURCE ) ) {
		return NULL;
	}

	for( pRes=_pLinkListTail; pRes; pRes=pRes->pPrevResLink ) {
		if( fres_CompareTypes( psType, pRes->sType ) ) {
			// Found matching type...

			if( !fclib_stricmp( pszName, (cchar *)(pRes + 1) ) ) {
				// Found matching name...
				return pRes;
			}
		}
	}

	return NULL;
}

static CFRes *_Find( void *pBase ) {
	CFRes *pRes;

	for( pRes=_pLinkListTail; pRes; pRes=pRes->pPrevResLink ) {
		if( pRes->pBase == pBase ) {
			// Found matching base pointer...
			return pRes;
		}
	}

	return NULL;
}

static CFRes *_Find( void *pBase, FResDestroyCallback_t *pFcnCallback ) {
	CFRes *pRes;

	for( pRes=_pLinkListTail; pRes; pRes=pRes->pPrevResLink ) {
		if( pRes->pBase==pBase && pRes->pFcnCallback==pFcnCallback ) {
			// Found matching base and callback pointers...
			return pRes;
		}
	}

	return NULL;
}

