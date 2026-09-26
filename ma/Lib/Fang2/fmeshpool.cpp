//////////////////////////////////////////////////////////////////////////////////////
// FMeshPool.cpp - Mesh pool module.
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
#include "fmeshpool.h"



CFMeshPool *CFMeshPool::CreatePool( u32 nMaxItemCount ) {
	CFMeshPool *pMeshPool;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	if( nMaxItemCount > 65535 ) {
		DEVPRINTF( "CFMeshPool::CreatePool(): Pool system supports a maximum of 65535 elements.\n" );
		goto _ExitWithError;
	}

	pMeshPool = fnew CFMeshPool;
	if( pMeshPool == NULL ) {
		DEVPRINTF( "CFMeshPool::CreatePool(): Not enough memory to allocate mesh pool.\n" );
		goto _ExitWithError;
	}

	pMeshPool->m_bCreated = TRUE;

	pMeshPool->m_pWorldMeshItemPool = fnew CFWorldMeshItem[nMaxItemCount];
	if( pMeshPool->m_pWorldMeshItemPool == NULL ) {
		DEVPRINTF( "CFMeshPool::CreatePool(): Not enough memory to allocate mesh pool items.\n" );
		goto _ExitWithError;
	}

	pMeshPool->m_nMaxPoolElementCount = nMaxItemCount;

	flinklist_InitRoot( &pMeshPool->m_RootFree, FANG_OFFSETOF( CFWorldMeshItem, m_Link ) );
	flinklist_InitRoot( &pMeshPool->m_RootActive, FANG_OFFSETOF( CFWorldMeshItem, m_Link ) );

	flinklist_InitPool( &pMeshPool->m_RootFree, pMeshPool->m_pWorldMeshItemPool, sizeof(CFWorldMeshItem), nMaxItemCount );

	for( i=0; i<nMaxItemCount; ++i ) {
		pMeshPool->m_pWorldMeshItemPool[i].m_nListMember = CFWorldMeshItem::LIST_MEMBER_FREE_POOL;
		pMeshPool->m_pWorldMeshItemPool[i].m_nPoolIndex = i;
	}

	return pMeshPool;

_ExitWithError:
	DestroyPool( pMeshPool );
	fres_ReleaseFrame( ResFrame );
	return NULL;
}


void CFMeshPool::DestroyPool( CFMeshPool *pMeshPool ) {
	if( pMeshPool ) {
		if( pMeshPool->IsCreated() ) {
			fdelete_array( pMeshPool->m_pWorldMeshItemPool );
			pMeshPool->m_pWorldMeshItemPool = NULL;

			pMeshPool->m_bCreated = FALSE;

			fdelete( pMeshPool );
		}
	}
}


CFWorldMeshItem *CFMeshPool::GetFromFreePool( void ) {
	FASSERT( IsCreated() );

	CFWorldMeshItem *pWorldMeshItem;

	pWorldMeshItem = (CFWorldMeshItem *)flinklist_RemoveTail( &m_RootFree );
	if( pWorldMeshItem == NULL ) {
		return NULL;
	}

	FASSERT( pWorldMeshItem->m_nListMember == CFWorldMeshItem::LIST_MEMBER_FREE_POOL );

	flinklist_AddTail( &m_RootActive, pWorldMeshItem );
	pWorldMeshItem->m_nListMember = CFWorldMeshItem::LIST_MEMBER_ACTIVE_POOL;

	return pWorldMeshItem;
}


void CFMeshPool::ReturnToFreePool( CFWorldMeshItem *pWorldMeshItem ) {
	FASSERT( IsCreated() );

	if( pWorldMeshItem == NULL ) {
		return;
	}

	if( pWorldMeshItem->m_nListMember == CFWorldMeshItem::LIST_MEMBER_FREE_POOL ) {
		return;
	}

	if( FWorld_pWorld ) {
		pWorldMeshItem->RemoveFromWorld();
	}

	FASSERT( pWorldMeshItem->m_nListMember == CFWorldMeshItem::LIST_MEMBER_ACTIVE_POOL );
	flinklist_Remove( &m_RootActive, pWorldMeshItem );
	flinklist_AddTail( &m_RootFree, pWorldMeshItem );
	pWorldMeshItem->m_nListMember = CFWorldMeshItem::LIST_MEMBER_FREE_POOL;
}


void CFMeshPool::ReturnAllToFreePool( void ) {
	FASSERT( IsCreated() );

	CFWorldMeshItem *pWorldMeshItem;

	while( pWorldMeshItem = (CFWorldMeshItem *)flinklist_GetTail( &m_RootActive ) ) {
		ReturnToFreePool( pWorldMeshItem );
	}
}




