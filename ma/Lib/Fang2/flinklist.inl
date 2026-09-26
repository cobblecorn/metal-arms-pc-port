//////////////////////////////////////////////////////////////////////////////////////
// flinklist.inl - Fang linklist module inlined functions.
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
#include "fang.h"
#include "flinklist.h"



#if FANG_ENABLE_INLINE_CODE
	FINLINE FLinkRoot_t *flinklist_InitRoot( FLinkRoot_t *pLinkRoot, s32 nStructureOffset );
	FINLINE void *flinklist_GetNext( const FLinkRoot_t *pLinkRoot, const void *pStructure );
	FINLINE void *flinklist_GetPrev( const FLinkRoot_t *pLinkRoot, const void *pStructure );
	FINLINE void *flinklist_GetHead( const FLinkRoot_t *pLinkRoot );
	FINLINE void *flinklist_GetTail( const FLinkRoot_t *pLinkRoot );
	FINLINE FLink_t *flinklist_GetLinkPointer( const FLinkRoot_t *pLinkRoot, const void *pStructure );
	FINLINE void *flinklist_GetStructurePointer( const FLinkRoot_t *pLinkRoot, const FLink_t *pLink );
#endif



// Initializes the specified link root.
// nStructureOffset is the number of bytes from the start of the
// structure encapsulating the FLink_t field to the FLink_t field.
// Use the OFFSETOF macro in xmacros.h to compute nStructureOffset.
FINLINE FLinkRoot_t *flinklist_InitRoot( FLinkRoot_t *pLinkRoot, s32 nStructureOffset ) {
	pLinkRoot->pHeadLink = NULL;
	pLinkRoot->pTailLink = NULL;
	pLinkRoot->nStructOffset = nStructureOffset;
	pLinkRoot->nCount = 0;

	return pLinkRoot;
}

// Returns a pointer to the next structure in the list.
// If pStructure is NULL, simply calls flinklist_GetHead();
FINLINE void *flinklist_GetNext( const FLinkRoot_t *pLinkRoot, const void *pStructure ) {
	const FLink_t *pLink;

	if( pStructure ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		pLink = (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );
		return flinklist_GetStructurePointer( pLinkRoot, pLink->pNextLink );
	} else {
		return flinklist_GetHead( pLinkRoot );
	}
}

// Returns a pointer to the previous structure in the list.
// If pStructure is NULL, simply calls flinklist_GetTail();
FINLINE void *flinklist_GetPrev( const FLinkRoot_t *pLinkRoot, const void *pStructure ) {
	const FLink_t *pLink;

	if( pStructure ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		pLink = (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );
		return flinklist_GetStructurePointer( pLinkRoot, pLink->pPrevLink );
	} else {
		return flinklist_GetTail( pLinkRoot );
	}
}

// Returns a pointer to the structure at the head of the list.
FINLINE void *flinklist_GetHead( const FLinkRoot_t *pLinkRoot ) {
	s32 nHeadAddress;

	nHeadAddress = (s32)pLinkRoot->pHeadLink;
	if( nHeadAddress ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		return (void *)( nHeadAddress - pLinkRoot->nStructOffset );
	} else {
		return NULL;
	}
}

// Returns a pointer to the structure at the tail of the list.
FINLINE void *flinklist_GetTail( const FLinkRoot_t *pLinkRoot ) {
	s32 nTailAddress;

	nTailAddress = (s32)pLinkRoot->pTailLink;
	if( nTailAddress ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		return (void *)( nTailAddress - pLinkRoot->nStructOffset );
	} else {
		return NULL;
	}
}

// Returns a pointer to the current structure's FLink_t entry.
FINLINE FLink_t *flinklist_GetLinkPointer( const FLinkRoot_t *pLinkRoot, const void *pStructure ) {
	if( pStructure ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		return (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );
	} else {
		return NULL;
	}
}

// Returns a pointer to the structure containing the specified FLink_t pointer.
FINLINE void *flinklist_GetStructurePointer( const FLinkRoot_t *pLinkRoot, const FLink_t *pLink ) {
	if( pLink ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		return (void *)( (s32)pLink - pLinkRoot->nStructOffset );
	} else {
		return NULL;
	}
}

