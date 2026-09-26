//////////////////////////////////////////////////////////////////////////////////////
// FMeshPool.h - Mesh pool module.
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
#ifndef _FMESHPOOL_H_
#define _FMESHPOOL_H_ 1

#include "fang.h"
#include "fworld.h"
#include "flinklist.h"


class CFMeshPool;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFWorldMeshItem
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFWorldMeshItem : public CFWorldMesh {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum ListMember_e {
		LIST_MEMBER_NONE,				// This object hasn't been created yet
		LIST_MEMBER_FREE_POOL,			// This object is currently in the free pool
		LIST_MEMBER_ACTIVE_POOL,		// This object is currently in the active pool

		LIST_MEMBER_COUNT
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FLink_t m_Link;						// Link to other objects in pool
	u16 m_nListMember;					// Which linklist this object is a member of (see ListMember_e for details)
	u16 m_nPoolIndex;					// The index into the pool of this element




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CFWorldMeshItem() { m_nListMember = LIST_MEMBER_NONE; }

	FINLINE BOOL IsCreated( void ) const { return (m_nListMember == LIST_MEMBER_ACTIVE_POOL); }

	FINLINE u32 GetPoolIndex( void ) const { return m_nPoolIndex; }


	friend class CFMeshPool;


	FCLASS_STACKMEM_ALIGN( CFWorldMeshItem );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFMeshPool
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFMeshPool {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL m_bCreated;						// TRUE once CreatePool() has been called
	FLinkRoot_t m_RootFree;					// Linklist of free objects
	FLinkRoot_t m_RootActive;				// Linklist of active objects
	u32 m_nMaxPoolElementCount;				// Number of elements in m_pMeshPool
	CFWorldMeshItem *m_pWorldMeshItemPool;	// Pool of mesh item objects




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static CFMeshPool *CreatePool( u32 nMaxItemCount );
	static void DestroyPool( CFMeshPool *pMeshPool );

	FINLINE BOOL IsCreated( void ) const { return m_bCreated; }

	CFWorldMeshItem *GetFromFreePool( void );
	void ReturnToFreePool( CFWorldMeshItem *pWorldMeshItem );
	void ReturnAllToFreePool( void );

	FINLINE u32 GetMaxItemCount( void ) const { FASSERT( IsCreated() ); return m_nMaxPoolElementCount; }

	FINLINE CFWorldMeshItem *GetFirstFreeItem( void ) const { FASSERT( IsCreated() ); return (CFWorldMeshItem *)flinklist_GetHead( &m_RootFree ); }
	FINLINE CFWorldMeshItem *GetNextFreeItem( const CFWorldMeshItem *pWorldMeshItem ) const { FASSERT( IsCreated() ); return (CFWorldMeshItem *)flinklist_GetNext( &m_RootFree, pWorldMeshItem ); }
	FINLINE CFWorldMeshItem *GetItemByIndex( u32 nItemIndex ) const { FASSERT( IsCreated() ); FASSERT( nItemIndex < m_nMaxPoolElementCount ); return &m_pWorldMeshItemPool[nItemIndex]; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FINLINE CFMeshPool() {}
	FINLINE ~CFMeshPool() {}


	FCLASS_STACKMEM_ALIGN( CFMeshPool );
} FCLASS_ALIGN_SUFFIX;



#endif
