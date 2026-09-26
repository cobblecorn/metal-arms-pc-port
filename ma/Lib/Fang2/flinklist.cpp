//////////////////////////////////////////////////////////////////////////////////////
// linklist.c - General purpose linklist functions.
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

#if !FANG_ENABLE_INLINE_CODE
	#include "flinklist.inl"
#endif

//====================
// private definitions

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

static void _RemoveLink( FLinkRoot_t *pLinkRoot, FLink_t *pLink );

//=================
// public functions

// Initializes a pool of linklist entries and its root.
// pLinkRoot points to the linklist root, not initialized by this call.
// pFirstEntry points to the start of the buffer to initialize.
// nEntryBytes specifies the size of each entry.
// nNumEntries specifies the number of entries to initialize.
// The buffer pointed to by pFirstEntry must have a minimum size of nEntryBytes*nNumEntries.
void flinklist_InitPool( FLinkRoot_t *pLinkRoot, void *pFirstEntry, int nEntryBytes, int nNumEntries ) {
	u8 *pBuf;

	for( pBuf = (u8 *)pFirstEntry; nNumEntries--; pBuf += nEntryBytes ) {
		flinklist_AddTail( pLinkRoot, pBuf );
	}
}

// Adds the specified structure to the head of the list.
// If pStructure is NULL, does nothing.
// Returns pStructure.
void *flinklist_AddHead( FLinkRoot_t *pLinkRoot, void *pStructure ) {
	FLink_t *pLink;

	FASSERT( (u32)pStructure );

	if( pStructure ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		LINKLIST_VALIDATE_REDUNDANCY( pLinkRoot, pStructure );

		pLink = (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );
		FASSERT(pLink != pLinkRoot->pHeadLink);

		pLink->pPrevLink = NULL;
		if( (pLink->pNextLink = pLinkRoot->pHeadLink) ) {
			pLinkRoot->pHeadLink->pPrevLink = pLink;
		} else {
			pLinkRoot->pTailLink = pLink;
		}
		pLinkRoot->pHeadLink = pLink;
		pLinkRoot->nCount++;
	}

	return pStructure;
}

//add new item into list before the current item
void *flinklist_AddBefore( FLinkRoot_t *pLinkRoot, void *pCurrent, void *pStructure )
{
	FASSERT( (u32)pCurrent );
	FASSERT( (u32)pStructure );

	if (pStructure)
	{
		FLink_t *pNew = flinklist_GetLinkPointer(pLinkRoot, pStructure);
		if (pCurrent)
		{
			//FASSERT( pCurrent better be in the list...!)
			FLink_t *pCur = flinklist_GetLinkPointer(pLinkRoot, pCurrent);
			LINKLIST_VALIDATE_REDUNDANCY( pLinkRoot, pStructure );

			pNew->pNextLink = pCur;
			pNew->pPrevLink = pCur->pPrevLink;
			pCur->pPrevLink = pNew;
			if (pNew->pPrevLink)
				pNew->pPrevLink->pNextLink = pNew;

			//ME:  was:  if (pCurrent == (FLink_t *) ( (s32)pLinkRoot->pHeadLink + pLinkRoot->nStructOffset))
			if( pCur == pLinkRoot->pHeadLink )
			{
				pLinkRoot->pHeadLink = pNew;
			}

			pLinkRoot->nCount++;
		}
		else
		{  //must be an empty list, why else pass NULL to AddAfter?
			FASSERT(pLinkRoot->nCount == 0 && pLinkRoot->pTailLink == NULL && pLinkRoot->pHeadLink);
			flinklist_AddHead(pLinkRoot, pStructure);
		}
	}
	return pStructure;
}

//add new into list after another one
void *flinklist_AddAfter( FLinkRoot_t *pLinkRoot, void *pAfterStructure, void *pStructure )
{
	FASSERT( (u32)pStructure );
	FASSERT( (u32)pAfterStructure || !pLinkRoot->nCount);

	if (pStructure)
	{
		FLink_t *pNew = flinklist_GetLinkPointer(pLinkRoot, pStructure);
		if (pAfterStructure)
		{
			//FASSERT( pAfterStructure better be in the list...!)
			FLink_t *pCur = flinklist_GetLinkPointer(pLinkRoot, pAfterStructure);

			pNew->pNextLink = pCur->pNextLink;
			pNew->pPrevLink = pCur;
			pCur->pNextLink = pNew;
			if (pNew->pNextLink)
				pNew->pNextLink->pPrevLink = pNew;

			//ME:  was  ( (s32)pLinkRoot->pTailLink + pLinkRoot->nStructOffset))
			if (pAfterStructure == (FLink_t *) ( (s32)pLinkRoot->pTailLink - pLinkRoot->nStructOffset))
			{
				pLinkRoot->pTailLink = pNew;
			}
			pLinkRoot->nCount++;
		}
		else
		{  //must be an empty list, why else pass NULL to AddAfter?
			FASSERT(pLinkRoot->nCount == 0 && pLinkRoot->pTailLink == NULL && pLinkRoot->pHeadLink);
			flinklist_AddHead(pLinkRoot, pStructure);
		}
	}
	return pStructure;
}

// Adds the specified structure to the tail of the list.
// If pStructure is NULL, does nothing.
// Returns pStructure.
void *flinklist_AddTail( FLinkRoot_t *pLinkRoot, void *pStructure ) {
	FLink_t *pLink;

	FASSERT( (u32)pStructure );

	if( pStructure ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		LINKLIST_VALIDATE_REDUNDANCY( pLinkRoot, pStructure );

		pLink = (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );

		FASSERT(pLink != pLinkRoot->pTailLink);
		pLink->pNextLink = NULL;
		if( (pLink->pPrevLink = pLinkRoot->pTailLink) ) {
			pLinkRoot->pTailLink->pNextLink = pLink;
		} else {
			pLinkRoot->pHeadLink = pLink;
		}
		pLinkRoot->pTailLink = pLink;
		pLinkRoot->nCount++;
	}

	return pStructure;
}

// Removes the specified structure from the list.
// If pStructure is NULL, does nothing.
// Returns pStructure.
void *flinklist_Remove( FLinkRoot_t *pLinkRoot, void *pStructure ) {

	if( pStructure ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		_RemoveLink( pLinkRoot, (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset ) );
	}

	return pStructure;
}

// Removes the structure which is at the head of the list.
// Returns a pointer to the structure, or NULL if the list was empty.
void *flinklist_RemoveHead( FLinkRoot_t *pLinkRoot ) {
	FLink_t *pLink;

	if( (pLink=pLinkRoot->pHeadLink) ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		_RemoveLink( pLinkRoot, pLink );
		return (void *)( (s32)pLink - pLinkRoot->nStructOffset );
	}

	return NULL;
}

// Removes the structure which is at the tail of the list.
// Returns a pointer to the structure, or NULL if the list was empty.
void *flinklist_RemoveTail( FLinkRoot_t *pLinkRoot ) {
	FLink_t *pLink;

	if( (pLink = pLinkRoot->pTailLink) ) {
		LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
		_RemoveLink( pLinkRoot, pLink );
		return (void *)( (s32)pLink - pLinkRoot->nStructOffset );
	}

	return NULL;
}

// Moves the first link to the end and returns the (new)last link...
void *flinklist_MoveHeadToTail( FLinkRoot_t *pLinkRoot ) {	
	FLink_t		*pLink;
	FLink_t		*pTempLink;
	
	LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
	
	pLink = pLinkRoot->pHeadLink;
	if( pLink == 0 ) {
		return 0;
	}
	
	if( (pTempLink = pLinkRoot->pTailLink) == pLink ) {
		// already there...
		return (void *)( (s32)pLink - pLinkRoot->nStructOffset );
	}
	
	pTempLink->pNextLink = pLink;
	pLink->pPrevLink = pTempLink;
	pTempLink = pLink->pNextLink;
	
	pLinkRoot->pHeadLink = pTempLink;
	pTempLink->pPrevLink = NULL;
	
	pLink->pNextLink = NULL;
	pLinkRoot->pTailLink = pLink;
	
	return (void *)( (s32)pLink - pLinkRoot->nStructOffset );
}

// Moves any link to the end...
void flinklist_MoveLinkToTail( FLinkRoot_t *pLinkRoot, void *pStructure ) {	
	FLink_t		*pLink;
	
	LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
	
	pLink = (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );
	
	if( pLinkRoot->pHeadLink == pLink ) {
		// call a specialized version...
		flinklist_MoveHeadToTail( pLinkRoot );
		return;
	}
	
	if( pLinkRoot->pHeadLink == 0 ) {
		return;
	}
	
	if( pLinkRoot->pTailLink == pLink ) {
		// already done...
		return;
	}
	
	pLink->pPrevLink->pNextLink = pLink->pNextLink;
	pLink->pNextLink->pPrevLink = pLink->pPrevLink;
	pLink->pNextLink = NULL;
	pLink->pPrevLink = pLinkRoot->pTailLink;
	pLinkRoot->pTailLink->pNextLink = pLink;
	pLinkRoot->pTailLink = pLink;
	
	return;
}

// returns TRUE if pLink is in pLinkRoot list.
BOOL flinklist_IsLinkInList( const FLinkRoot_t *pLinkRoot, const FLink_t *pLink ) {
	FLink_t *pL;
	BOOL bLinkIsInList = FALSE;

	pL = pLinkRoot->pHeadLink;
	while( pL )
	{
		if( pL == pLink )
		{
			bLinkIsInList = TRUE;
			break;
		}

		pL = pL->pNextLink;
	}

	return bLinkIsInList;
}


#if FANG_ENABLE_FASSERT

// Validates the link integrity by walking the list and counting the elements
void flinklist_Validate_CheckLinkIntegrity( const FLinkRoot_t *pLinkRoot ) {
	void *pStructure;
	FLink_t *pLink = NULL;
	u32 nCount;

	if( !fang_GetValidationState() ) {
		// MaxValidation state not set...
		return;
	}

	// Check to see if count matches the number of entries...
	nCount = 0;
	if( pLinkRoot->pHeadLink ) {
		pStructure = (void *)( (s32)pLinkRoot->pHeadLink - pLinkRoot->nStructOffset );
		while( pStructure ) {
			FASSERT( nCount < pLinkRoot->nCount );
			nCount++;
			pLink = (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );
			if( pLink->pNextLink ) {
				pStructure = (void *)( (s32)pLink->pNextLink - pLinkRoot->nStructOffset );
			} else {
				pStructure = NULL;
			}
		}
	}

	FASSERT( nCount == pLinkRoot->nCount );
}

// Checks the link list for duplicates
void flinklist_Validate_CheckForRedundantEntry( const FLinkRoot_t *pLinkRoot, void *pTestStructure ) {
	void *pStructure;
	FLink_t *pLink;

	if( !fang_GetValidationState() ) {
		// MaxValidation state not set...
		return;
	}

	if( pLinkRoot->pHeadLink ) {
		pStructure = (void *)( (s32)pLinkRoot->pHeadLink - pLinkRoot->nStructOffset );
		while( pStructure ) {
			FASSERT( pStructure != pTestStructure );

			pLink = (FLink_t *)( (s32)pStructure + pLinkRoot->nStructOffset );
			if( pLink->pNextLink ) {
				pStructure = (void *)( (s32)pLink->pNextLink - pLinkRoot->nStructOffset );
			} else {
				pStructure = NULL;
			}
		}
	}
}
#endif

//==================
// private functions

// Removes the specified link from the list.
// Returns a pointer to the structure.
static void _RemoveLink( FLinkRoot_t *pLinkRoot, FLink_t *pLink ) {

	if( fang_GetValidationState() ) {
		FLink_t *pL;
		BOOL bLinkIsInList = FALSE;

		pL = pLinkRoot->pHeadLink;
		while( pL )
		{
			if( pL == pLink )
			{
				bLinkIsInList = TRUE;
				break;
			}

			pL = pL->pNextLink;
		}

		FASSERT( bLinkIsInList );
	}

	FASSERT((pLinkRoot->nCount == 1 && pLinkRoot->pHeadLink == pLink) || (pLink->pNextLink || pLink->pPrevLink));
	if( pLinkRoot->pHeadLink ) {
		if( pLink->pPrevLink ) {
			pLink->pPrevLink->pNextLink = pLink->pNextLink;
		} else {
			FASSERT(pLinkRoot->pHeadLink == pLink);
			pLinkRoot->pHeadLink = pLink->pNextLink;
		}

		if( pLink->pNextLink ) {
			pLink->pNextLink->pPrevLink = pLink->pPrevLink;
		} else {
			FASSERT(pLinkRoot->pTailLink == pLink);
			pLinkRoot->pTailLink = pLink->pPrevLink;
		}

		pLinkRoot->nCount--;

		FASSERT((pLinkRoot->nCount != 1) || (pLinkRoot->pHeadLink == pLinkRoot->pTailLink));
		FASSERT((pLinkRoot->nCount != 0) || (!pLinkRoot->pHeadLink && !pLinkRoot->pTailLink));
		FASSERT((pLinkRoot->nCount < 2) || (pLinkRoot->pHeadLink != pLinkRoot->pTailLink));
	}

	pLink->pNextLink = NULL;
	pLink->pPrevLink = NULL;

	LINKLIST_VALIDATE_INTEGRITY( pLinkRoot );
}



