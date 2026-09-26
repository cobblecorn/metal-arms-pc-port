//////////////////////////////////////////////////////////////////////////////////////
// fres.h - Fang resource manager.
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

#ifndef _FRES_H_
#define _FRES_H_ 1

#include "fang.h"
#include "fheap.h"
#include "fgamedata.h"

//------------------------------------------------------------------------------------------------------------------------------------
// fmem system:
//------------------------------------------------------------------------------------------------------------------------------------
#define FMEM_ALLOCSTRUCT( StructName ) (StructName *)fmem_Alloc( sizeof(StructName) )
#define FMEM_ALLOCSTRUCTS( StructName, nCount ) (StructName *)fmem_Alloc( sizeof(StructName) * (nCount) )
#define FMEM_ALLOCSTRUCTZ( StructName ) (StructName *)fmem_AllocAndZero( sizeof(StructName) )
#define FMEM_ALLOCSTRUCTSZ( StructName, nCount ) (StructName *)fmem_AllocAndZero( sizeof(StructName) * (nCount) )

typedef void *FMemFrame_t;


extern BOOL FRes_bModuleInitialized;
extern CFHeap FRes_CFHeap;


extern BOOL fres_ModuleStartup( void );
extern void fres_ModuleShutdown( void );


FINLINE FMemFrame_t fmem_GetFrame( void ) {
	FASSERT( FRes_bModuleInitialized );
	return (FMemFrame_t)FRes_CFHeap.GetFrameHeap0();
}

FINLINE void fmem_ReleaseFrame( FMemFrame_t Frame ) {
	FASSERT( FRes_bModuleInitialized );
	fgamedata_MemFrameReleased( (void *)Frame );
	FRes_CFHeap.ReleaseFrameHeap0( (FHeapFrame_t)Frame );
}

FINLINE void *fmem_Alloc( u32 nBytes, u32 nByteAlignment=0 ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.AllocHeap0( nBytes, nByteAlignment );
}

FINLINE void *fmem_AllocAndZero( u32 nBytes, u32 nByteAlignment=0 ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.AllocAndZeroHeap0( nBytes, nByteAlignment );
}

FINLINE void *fmem_Alloc( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.AllocHeap0( nBytes, nByteAlignment, nOffset );
}

FINLINE void *fmem_AllocAndZero( u32 nBytes, u32 nByteAlignment, u32 nOffset ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.AllocAndZeroHeap0( nBytes, nByteAlignment, nOffset );
}

FINLINE u32 fmem_GetFreeBytes( void ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.GetFreeBytes();
}

FINLINE void *fmem_AllocAll( u32 *pnBytesAllocated, u32 nByteAlignment=0 ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.AllocAllHeap0( nByteAlignment, pnBytesAllocated );
}

FINLINE BOOL fmem_Resize( void *pMemBlock, u32 nNewSizeInBytes ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.ResizeHeap0( pMemBlock, nNewSizeInBytes );
}





//------------------------------------------------------------------------------------------------------------------------------------
// fres system:
//------------------------------------------------------------------------------------------------------------------------------------
#define FRES_TYPELEN	4
#define FRES_TYPELENZ	(FRES_TYPELEN + 1)
#define FRES_NAMELEN	31	// This is a recommendation, not a limit!

#define FRES_TYPE_NONE		"NONE"
#define FRES_NAME_NONE		"noname"
#define FRES_CLASS_RESOURCE	"FNEW"

// list the different types here so that we don't collide with one another - Mike
#define FTEX_RESNAME			"TXTR"
#define FANIM_RESNAME			"ANIM"
#define FMESH_RESTYPE			"MESH"
#define FWORLD_RESTYPE			"WRLD"
#define FGAMEDATA_RESTYPE		"GDAT"
#define FAUDIOBANK_RESTYPE		"WDAT"
#define FSNDFX_RESTYPE			"SFXB"
#define FPARTICLE_RESTYPE		"PART"
#define FCAMANIM_RESTYPE		"CANM"


#define FRES_ALLOCSTRUCT( StructName ) (StructName *)fres_Alloc( sizeof(StructName) )
#define FRES_ALLOCSTRUCTS( StructName, nCount ) (StructName *)fres_Alloc( sizeof(StructName) * (nCount) )
#define FRES_ALLOCSTRUCTZ( StructName ) (StructName *)fres_AllocAndZero( sizeof(StructName) )
#define FRES_ALLOCSTRUCTSZ( StructName, nCount ) (StructName *)fres_AllocAndZero( sizeof(StructName) * (nCount) )

typedef void *FResFrame_t;
typedef void *FResHandle_t;
typedef void FResDestroyCallback_t( void *pResMem );

#define FRES_NULLHANDLE		((FResHandle_t)0)


FINLINE FResFrame_t fres_GetFrame( void ) {
	FASSERT( FRes_bModuleInitialized );
	return (FResFrame_t)FRes_CFHeap.GetFrame( 1 );
}


extern void fres_CopyType( char *psDestResType, cchar *psSrcResType );
extern BOOL fres_CompareTypes( cchar *psResType1, cchar *psResType2 );
extern BOOL fres_IsTypeValid( cchar *psResType );

extern void *fres_FindBase( cchar *psResType, cchar *pszResName );
extern void *fres_FindBase( void *pBase );
extern FResHandle_t fres_FindHandle( cchar *psResType, cchar *pszResName );
extern FResHandle_t fres_FindHandle( void *pBase );
extern FResHandle_t fres_FindHandle( void *pBase, FResDestroyCallback_t *pFcnCallback );

extern void *fres_GetBase( FResHandle_t hRes );
extern FResDestroyCallback_t *fres_GetCallback( FResHandle_t hRes );
extern void fres_GetTypeNZ( FResHandle_t hRes, char *psDestBuf );
extern void fres_GetTypeZ( FResHandle_t hRes, char *pszDestBuf );
extern cchar *fres_GetName( FResHandle_t hRes );

extern void fres_SetCallback( FResHandle_t hRes, FResDestroyCallback_t *pFcnDestroyCallback );
extern void fres_SetBase( FResHandle_t hRes, void *pBase );
extern void fres_SetBaseAndCallback( FResHandle_t hRes, void *pBase, FResDestroyCallback_t *pFcnDestroyCallback );





#if FANG_DEBUG_BUILD || FANG_TEST_BUILD

	// MACROS - Generic creation:
	#define fres_Create( psResType, pszResName ) fres_Debug_Create( __FILE__, __LINE__, psResType, pszResName )
	#define fres_CreateWithCallback( psResType, pszResName, pFcnDestroyCallback ) fres_Debug_CreateWithCallback( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback )

	// MACROS - Creation without alignment:
	#define fres_CreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes ) fres_Debug_CreateAndAlloc( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes )
	#define fres_CreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes ) fres_Debug_CreateAndAllocAndZero( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes )
	#define fres_CreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes ) fres_Debug_CreateAndAllocWithOffset( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes )
	#define fres_CreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes ) fres_Debug_CreateAndAllocAndZeroWithOffset( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes )

	// MACROS - Creation with alignment:
	#define fres_AlignedCreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes ) fres_Debug_AlignedCreateAndAlloc( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes )
	#define fres_AlignedCreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes ) fres_Debug_AlignedCreateAndAllocAndZero( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes )
	#define fres_AlignedCreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes ) fres_Debug_AlignedCreateAndAllocWithOffset( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes )
	#define fres_AlignedCreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes ) fres_Debug_AlignedCreateAndAllocAndZeroWithOffset( __FILE__, __LINE__, psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes )

	// MACROS - Allocation without alignment:
	#define fres_Alloc( nBytes ) fres_Debug_Alloc( __FILE__, __LINE__, nBytes )
	#define fres_AllocAndZero( nBytes ) fres_Debug_AllocAndZero( __FILE__, __LINE__, nBytes )
	#define fres_AllocWithOffset( nBytes, nOffset ) fres_Debug_AllocWithOffset( __FILE__, __LINE__, nBytes, nOffset )
	#define fres_AllocAndZeroWithOffset( nBytes, nOffset ) fres_Debug_AllocAndZeroWithOffset( __FILE__, __LINE__, nBytes, nOffset )

	// MACROS - Allocation with alignment:
	#define fres_AlignedAlloc( nBytes, nByteAlignment ) fres_Debug_AlignedAlloc( __FILE__, __LINE__, nBytes, nByteAlignment )
	#define fres_AlignedAllocAndZero( nBytes, nByteAlignment ) fres_Debug_AlignedAllocAndZero( __FILE__, __LINE__, nBytes, nByteAlignment )
	#define fres_AlignedAllocWithOffset( nBytes, nByteAlignment, nOffset ) fres_Debug_AlignedAllocWithOffset( __FILE__, __LINE__, nBytes, nByteAlignment, nOffset )
	#define fres_AlignedAllocAndZeroWithOffset( nBytes, nByteAlignment, nOffset ) fres_Debug_AlignedAllocAndZeroWithOffset( __FILE__, __LINE__, nBytes, nByteAlignment, nOffset )

	// MACROS - Frame release:
	#define fres_ReleaseFrame( Frame ) fres_Debug_ReleaseFrame( __FILE__, __LINE__, Frame )

	// Generic creation:
	extern FResHandle_t fres_Debug_Create( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName );
	extern FResHandle_t fres_Debug_CreateWithCallback( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback );

	// Creation without alignment:
	extern void *fres_Debug_CreateAndAlloc( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes );
	extern void *fres_Debug_CreateAndAllocAndZero( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes );
	extern void *fres_Debug_CreateAndAllocWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes );
	extern void *fres_Debug_CreateAndAllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes );

	// Creation with alignment:
	extern void *fres_Debug_AlignedCreateAndAlloc( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes );
	extern void *fres_Debug_AlignedCreateAndAllocAndZero( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes );
	extern void *fres_Debug_AlignedCreateAndAllocWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes );
	extern void *fres_Debug_AlignedCreateAndAllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes );

	// Allocation without alignment:
	extern void *fres_Debug_Alloc( cchar *pszFileName, u32 nLineNum, u32 nBytes );
	extern void *fres_Debug_AllocAndZero( cchar *pszFileName, u32 nLineNum, u32 nBytes );
	extern void *fres_Debug_AllocWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nOffset );
	extern void *fres_Debug_AllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nOffset );

	// Allocation with alignment:
	extern void *fres_Debug_AlignedAlloc( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment );
	extern void *fres_Debug_AlignedAllocAndZero( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment );
	extern void *fres_Debug_AlignedAllocWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment, u32 nOffset );
	extern void *fres_Debug_AlignedAllocAndZeroWithOffset( cchar *pszFileName, u32 nLineNum, u32 nBytes, u32 nByteAlignment, u32 nOffset );

	// Frame release:
	extern void fres_Debug_ReleaseFrame( cchar *pszFileName, u32 nLineNum, FResFrame_t Frame );

#else

	// MACROS - Generic creation:
	#define fres_Create( psResType, pszResName ) _fres_Create( psResType, pszResName )
	#define fres_CreateWithCallback( psResType, pszResName, pFcnDestroyCallback ) _fres_CreateWithCallback( psResType, pszResName, pFcnDestroyCallback )

	// MACROS - Creation without alignment:
	#define fres_CreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes ) _fres_CreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes )
	#define fres_CreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes ) _fres_CreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, phDestRes )
	#define fres_CreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes ) _fres_CreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes )
	#define fres_CreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes ) _fres_CreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nOffset, phDestRes )

	// MACROS - Creation with alignment:
	#define fres_AlignedCreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes ) _fres_AlignedCreateAndAlloc( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes )
	#define fres_AlignedCreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes ) _fres_AlignedCreateAndAllocAndZero( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, phDestRes )
	#define fres_AlignedCreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes ) _fres_AlignedCreateAndAllocWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes )
	#define fres_AlignedCreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes ) _fres_AlignedCreateAndAllocAndZeroWithOffset( psResType, pszResName, pFcnDestroyCallback, nBytes, nByteAlignment, nOffset, phDestRes )

	// MACROS - Allocation without alignment:
	#define fres_Alloc( nBytes ) _fres_Alloc( nBytes )
	#define fres_AllocAndZero( nBytes ) _fres_AllocAndZero( nBytes )
	#define fres_AllocWithOffset( nBytes, nOffset ) _fres_AllocWithOffset( nBytes, nOffset )
	#define fres_AllocAndZeroWithOffset( nBytes, nOffset ) _fres_AllocAndZeroWithOffset( nBytes, nOffset )

	// MACROS - Allocation with alignment:
	#define fres_AlignedAlloc( nBytes, nByteAlignment ) _fres_AlignedAlloc( nBytes, nByteAlignment )
	#define fres_AlignedAllocAndZero( nBytes, nByteAlignment ) _fres_AlignedAllocAndZero( nBytes, nByteAlignment )
	#define fres_AlignedAllocWithOffset( nBytes, nByteAlignment, nOffset ) _fres_AlignedAllocWithOffset( nBytes, nByteAlignment, nOffset )
	#define fres_AlignedAllocAndZeroWithOffset( nBytes, nByteAlignment, nOffset ) _fres_AlignedAllocAndZeroWithOffset( nBytes, nByteAlignment, nOffset )

	// MACROS - Frame release:
	#define fres_ReleaseFrame( Frame ) _fres_ReleaseFrame( Frame )

	// Generic creation:
	extern FResHandle_t _fres_Create( cchar *psResType, cchar *pszResName );
	extern FResHandle_t _fres_CreateWithCallback( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback );

	// Creation without alignment:
	extern void *_fres_CreateAndAlloc( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes );
	extern void *_fres_CreateAndAllocAndZero( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, FResHandle_t *phDestRes );
	extern void *_fres_CreateAndAllocWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes );
	extern void *_fres_CreateAndAllocAndZeroWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nOffset, FResHandle_t *phDestRes );

	// Creation with alignment:
	extern void *_fres_AlignedCreateAndAlloc( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes );
	extern void *_fres_AlignedCreateAndAllocAndZero( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, FResHandle_t *phDestRes );
	extern void *_fres_AlignedCreateAndAllocWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes );
	extern void *_fres_AlignedCreateAndAllocAndZeroWithOffset( cchar *psResType, cchar *pszResName, FResDestroyCallback_t *pFcnDestroyCallback, u32 nBytes, u32 nByteAlignment, u32 nOffset, FResHandle_t *phDestRes );

	// Allocation without alignment:
	extern void *_fres_Alloc( u32 nBytes );
	extern void *_fres_AllocAndZero( u32 nBytes );
	extern void *_fres_AllocWithOffset( u32 nBytes, u32 nOffset );
	extern void *_fres_AllocAndZeroWithOffset( u32 nBytes, u32 nOffset );

	// Allocation with alignment:
	extern void *_fres_AlignedAlloc( u32 nBytes, u32 nByteAlignment );
	extern void *_fres_AlignedAllocAndZero( u32 nBytes, u32 nByteAlignment );
	extern void *_fres_AlignedAllocWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );
	extern void *_fres_AlignedAllocAndZeroWithOffset( u32 nBytes, u32 nByteAlignment, u32 nOffset );

	// Frame release:
	extern void _fres_ReleaseFrame( FResFrame_t Frame );

#endif




FINLINE u32 fres_GetFreeBytes( void ) {
	FASSERT( FRes_bModuleInitialized );
	return FRes_CFHeap.GetFreeBytes();
}




#endif

