//////////////////////////////////////////////////////////////////////////////////////
// FLightPool.h - Light pool module.
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
#ifndef _FLIGHTPOOL_H_
#define _FLIGHTPOOL_H_ 1

#include "fang.h"
#include "fworld.h"
#include "flinklist.h"


class CFLightPool;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFWorldLightItem
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFWorldLightItem : public CFWorldLight {
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
	ListMember_e m_nListMember;			// Which linklist this object is a member of




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CFWorldLightItem() { m_nListMember = LIST_MEMBER_NONE; }

	FINLINE BOOL IsCreated( void ) const { return (m_nListMember == LIST_MEMBER_ACTIVE_POOL); }


	friend class CFLightPool;


	FCLASS_STACKMEM_ALIGN( CFWorldLightItem );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFLightPool
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFLightPool {
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
	u32 m_nMaxPoolElementCount;				// Number of elements in m_pLightPool
	CFWorldLightItem *m_pWorldLightItemPool;	// Pool of light item objects




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL ModuleStartup();
	static void ModuleShutdown();

	static CFLightPool *CreatePool( u32 nMaxItemCount );
	static void DestroyPool( CFLightPool *pLightPool );

	FINLINE BOOL IsCreated( void ) const { return m_bCreated; }

	CFWorldLightItem *GetFromFreePool( void );
	void ReturnToFreePool( CFWorldLightItem *pWorldLightItem );
	void ReturnAllToFreePool( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FINLINE CFLightPool() {}
	FINLINE ~CFLightPool() {}


	FCLASS_STACKMEM_ALIGN( CFLightPool );
} FCLASS_ALIGN_SUFFIX;



extern CFLightPool *FLightPool_pPool;

#endif
