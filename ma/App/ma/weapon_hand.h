//////////////////////////////////////////////////////////////////////////////////////
// weapon_hand.h - Hand gun implementation.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/28/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _WEAPON_HAND_H_
#define _WEAPON_HAND_H_ 1

#include "fang.h"
#include "fworld.h"
#include "weapon.h"




FCLASS_ALIGN_PREFIX class CWeaponHand : public CWeapon {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CFWorldMesh *m_pWorldMeshArray;			// Array of world meshes: one for each upgrade level.
	CFWorldMesh *m_pWorldMesh;				// Current world mesh

	static cchar *m_apszMeshNames[EUK_COUNT_BLINK_RIGHT_HAND];




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CWeaponHand();
	virtual ~CWeaponHand();

	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);
	virtual void ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel );

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); return m_pWorldMesh; }
	virtual void HumanTargeting_FindDesiredTargetPoint_WS( CFVec3A *pTargetPoint_WS );



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain

	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );
	virtual void ClassHierarchyRelocated( void *pIdentifier );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _SetDefaults( void );
	void _ResetThisClassFromConfigData( void );


	FCLASS_STACKMEM_ALIGN( CWeaponHand );
} FCLASS_ALIGN_SUFFIX ;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponHandBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CWeaponHandBuilder : public CWeaponBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CWeaponHandBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CWeaponHandBuilder );
} FCLASS_ALIGN_SUFFIX;




#endif

