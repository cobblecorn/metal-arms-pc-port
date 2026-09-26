//////////////////////////////////////////////////////////////////////////////////////
// FLightPool.cpp - Light pool module.
//
// Author: Steve Ranck   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 12/04/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "flightpool.h"


////////////////////////////////////////
// global vars
CFLightPool *FLightPool_pPool = NULL;


/////////////////////////////
// CFLightPool methods

BOOL CFLightPool::ModuleStartup() {
	FASSERT( !FLightPool_pPool );

	if( Fang_ConfigDefs.nLightPool_nNumLights ) {
		FLightPool_pPool = CFLightPool::CreatePool( Fang_ConfigDefs.nLightPool_nNumLights );
		if( !FLightPool_pPool ) {
			DEVPRINTF( "CFLightPool::ModuleStartup() : Could not create the global light pool of %d lights.\n", Fang_ConfigDefs.nLightPool_nNumLights );
			return FALSE;
		}
	}
	
	return TRUE;
}

void CFLightPool::ModuleShutdown( void ) {
	
	if( FLightPool_pPool ) {
		CFLightPool::DestroyPool( FLightPool_pPool );
		FLightPool_pPool = NULL;
	}
}

CFLightPool *CFLightPool::CreatePool( u32 nMaxItemCount ) {
	CFLightPool *pLightPool;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	pLightPool = fnew CFLightPool;
	if( pLightPool == NULL ) {
		DEVPRINTF( "CFLightPool::CreatePool(): Not enough memory to allocate light pool.\n" );
		goto _ExitWithError;
	}

	pLightPool->m_bCreated = TRUE;

	pLightPool->m_pWorldLightItemPool = fnew CFWorldLightItem[nMaxItemCount];
	if( pLightPool->m_pWorldLightItemPool == NULL ) {
		DEVPRINTF( "CFLightPool::CreatePool(): Not enough memory to allocate light pool items.\n" );
		goto _ExitWithError;
	}

	pLightPool->m_nMaxPoolElementCount = nMaxItemCount;

	flinklist_InitRoot( &pLightPool->m_RootFree, FANG_OFFSETOF( CFWorldLightItem, m_Link ) );
	flinklist_InitRoot( &pLightPool->m_RootActive, FANG_OFFSETOF( CFWorldLightItem, m_Link ) );

	flinklist_InitPool( &pLightPool->m_RootFree, pLightPool->m_pWorldLightItemPool, sizeof(CFWorldLightItem), nMaxItemCount );

	for( i=0; i<nMaxItemCount; ++i ) {
		pLightPool->m_pWorldLightItemPool[i].m_nListMember = CFWorldLightItem::LIST_MEMBER_FREE_POOL;
	}

	return pLightPool;

_ExitWithError:
	DestroyPool( pLightPool );
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


void CFLightPool::DestroyPool( CFLightPool *pLightPool ) {
	if( pLightPool->IsCreated() ) {
		fdelete_array( pLightPool->m_pWorldLightItemPool );
		pLightPool->m_pWorldLightItemPool = NULL;

		pLightPool->m_bCreated = FALSE;

		fdelete( pLightPool );
	}
}


CFWorldLightItem *CFLightPool::GetFromFreePool( void ) {
	FASSERT( IsCreated() );

	CFWorldLightItem *pWorldLightItem;

	pWorldLightItem = (CFWorldLightItem *)flinklist_RemoveTail( &m_RootFree );
	if( pWorldLightItem == NULL ) {
		return NULL;
	}

	FASSERT( pWorldLightItem->m_nListMember == CFWorldLightItem::LIST_MEMBER_FREE_POOL );

	flinklist_AddTail( &m_RootActive, pWorldLightItem );
	pWorldLightItem->m_nListMember = CFWorldLightItem::LIST_MEMBER_ACTIVE_POOL;

	return pWorldLightItem;
}


void CFLightPool::ReturnToFreePool( CFWorldLightItem *pWorldLightItem ) {
	if( pWorldLightItem == NULL ) {
		return;
	}

	FASSERT( IsCreated() );

	if( pWorldLightItem->m_nListMember == CFWorldLightItem::LIST_MEMBER_FREE_POOL ) {
		return;
	}

	if( FWorld_pWorld ) {
		pWorldLightItem->RemoveFromWorld();
	}

	FASSERT( pWorldLightItem->m_nListMember == CFWorldLightItem::LIST_MEMBER_ACTIVE_POOL );
	flinklist_Remove( &m_RootActive, pWorldLightItem );
	flinklist_AddTail( &m_RootFree, pWorldLightItem );
	pWorldLightItem->m_nListMember = CFWorldLightItem::LIST_MEMBER_FREE_POOL;
}


void CFLightPool::ReturnAllToFreePool( void ) {
	FASSERT( IsCreated() );

	CFWorldLightItem *pWorldLightItem;

	while( pWorldLightItem = (CFWorldLightItem *)flinklist_GetHead( &m_RootActive ) ) {
		ReturnToFreePool( pWorldLightItem );
	}
}




