//////////////////////////////////////////////////////////////////////////////////////
// flinklist.h - Fang linklist module.
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
// 09/07/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FLINKLIST_H_
#define _FLINKLIST_H_ 1

#include "fang.h"


#if FANG_ENABLE_FASSERT
	// Linklist validation is extremely slow! Even though these macros are mapped to
	// functions, the functions only perform the validation check if Fang's MaxValidation
	// flag is TRUE (see fang_SetMaxValidationState() for more info).
	#define LINKLIST_VALIDATE_INTEGRITY( pLinkRoot ) flinklist_Validate_CheckLinkIntegrity(pLinkRoot);
	#define LINKLIST_VALIDATE_REDUNDANCY( pLinkRoot, pTestStructure ) flinklist_Validate_CheckForRedundantEntry( pLinkRoot, pTestStructure );
#else
	// Disable linklist validation checks...
	#define LINKLIST_VALIDATE_INTEGRITY( pLinkRoot )
	#define LINKLIST_VALIDATE_REDUNDANCY( pLinkRoot, pTestStructure )
#endif

struct _FLink_s;
typedef struct _FLink_s FLink_t;

struct _FLink_s {
	FLink_t *pPrevLink;			// Points to previous link
	FLink_t *pNextLink;			// Points to next link
};

typedef struct FLinkRoot_s{
	FLink_t *pHeadLink;			// Points to the head link
	FLink_t *pTailLink;			// Points to the tail link
	s32 nStructOffset;			// Bytes to start of the structure that encapsulates the FLink_t field
	u32 nCount;					// Number of items in list
} FLinkRoot_t;


#if !FANG_ENABLE_INLINE_CODE
	extern FLinkRoot_t *flinklist_InitRoot( FLinkRoot_t *pLinkRoot, s32 nStructureOffset );
	extern void *flinklist_GetNext( const FLinkRoot_t *pLinkRoot, const void *pStructure );
	extern void *flinklist_GetPrev( const FLinkRoot_t *pLinkRoot, const void *pStructure );
	extern void *flinklist_GetHead( const FLinkRoot_t *pLinkRoot );
	extern void *flinklist_GetTail( const FLinkRoot_t *pLinkRoot );
	extern FLink_t *flinklist_GetLinkPointer( const FLinkRoot_t *pLinkRoot, const void *pStructure );
	extern void *flinklist_GetStructurePointer( const FLinkRoot_t *pLinkRoot, const FLink_t *pLink );
#endif

extern void flinklist_InitPool( FLinkRoot_t *pLinkRoot, void *pFirstEntry, int nEntryBytes, int nNumEntries );
extern void *flinklist_AddHead( FLinkRoot_t *pLinkRoot, void *pStructure );
extern void *flinklist_AddTail( FLinkRoot_t *pLinkRoot, void *pStructure );
extern void *flinklist_AddBefore( FLinkRoot_t *pLinkRoot, void *pCurrent, void *Structure );  //add pStructure into list before pCurrent
extern void *flinklist_AddAfter( FLinkRoot_t *pLinkRoot, void *pCurrent, void *pStructure );  //add pStructure into list after pCurrent
extern void *flinklist_Remove( FLinkRoot_t *pLinkRoot, void *pStructure );
extern void *flinklist_RemoveHead( FLinkRoot_t *pLinkRoot );
extern void *flinklist_RemoveTail( FLinkRoot_t *pLinkRoot );
extern void *flinklist_MoveHeadToTail( FLinkRoot_t *pLinkRoot );
extern void flinklist_MoveLinkToTail( FLinkRoot_t *pLinkRoot, void *pStructure );
extern BOOL flinklist_IsLinkInList( const FLinkRoot_t *pLinkRoot, const FLink_t *pLink );

#if FANG_ENABLE_FASSERT
	extern void flinklist_Validate_CheckLinkIntegrity( const FLinkRoot_t *pLinkRoot );
	extern void flinklist_Validate_CheckForRedundantEntry( const FLinkRoot_t *pLinkRoot, void *pTestStructure );
#endif


#if FANG_ENABLE_INLINE_CODE
	#include "flinklist.inl"
#endif


#endif
