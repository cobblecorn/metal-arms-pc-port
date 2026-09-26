//////////////////////////////////////////////////////////////////////////////////////
// weapon_hand.cpp - Hand implementation.
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

#include "fang.h"
#include "weapon_hand.h"
#include "weapon.h"
#include "fworld.h"
#include "fresload.h"
#include "meshtypes.h"



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponHandBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponHandBuilder _WeaponHandBuilder;


void CWeaponHandBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONHAND, pszEntityType );
}


BOOL CWeaponHandBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponHandBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponHand
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

cchar *CWeaponHand::m_apszMeshNames[EUK_COUNT_BLINK_RIGHT_HAND] = {
	"GWDGwpnP001",
	"GWDMwpnP001",
	"GWMGwpnP001"
};


CWeaponHand::CWeaponHand() {
	m_pWorldMeshArray = NULL;
}


CWeaponHand::~CWeaponHand() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponHand::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponHandBuilder *pBuilder = (CWeaponHandBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponHandBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONHAND );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_BLINK_RIGHT_HAND, pszEntityName, pMtx, pszAIBuilderName );
}


void CWeaponHand::ClassHierarchyDestroy( void ) {
	fdelete_array( m_pWorldMeshArray );
	m_pWorldMeshArray = NULL;

	CWeapon::ClassHierarchyDestroy();
}


BOOL CWeaponHand::ClassHierarchyBuild( void ) {
	Info_t *pInfo;
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	u32 i;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponHandBuilder *pBuilder = (CWeaponHandBuilder *)GetLeafClassBuilder();

	// Set input parameters for CBot creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_SetDefaults();

	// Initialize from builder object...

	// Create our world mesh array (one world mesh for each upgrade level)...
	m_pWorldMeshArray = fnew CFWorldMesh[EUK_COUNT_BLINK_RIGHT_HAND];
	if( m_pWorldMeshArray == NULL ) {
		DEVPRINTF( "CWeaponHand::Create(): Not enough memory to create m_pWorldMeshArray.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<EUK_COUNT_BLINK_RIGHT_HAND; i++ ) {
		// Load mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_apszMeshNames[i] );
		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponHand::Create(): Could not find mesh '%s'\n", m_apszMeshNames[i] );
			goto _ExitWithError;
		}

		// Init the world mesh...
		MeshInit.pMesh = pMesh;
		MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist = 200.0f;
		MeshInit.Mtx.Identity();
		MeshInit.Mtx.SetRotationY( 0.0f );
		MeshInit.Mtx.m_vPos.Set( 0.0f, 0.0f, 0.0f );
		m_pWorldMeshArray[i].Init( &MeshInit );
		m_pWorldMeshArray[i].RemoveFromWorld();
		m_pWorldMeshArray[i].SetCollisionFlag( TRUE );
		m_pWorldMeshArray[i].m_nUser = MESHTYPES_ENTITY;
		m_pWorldMeshArray[i].m_pUser = this;
		m_pWorldMeshArray[i].SetUserTypeBits( TypeBits() );

		pInfo = &m_aaInfo[WEAPON_TYPE_BLINK_RIGHT_HAND][i];
		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = 0.0f;
		pInfo->fMaxTargetAssistDist = 0.0f;
		pInfo->fMaxLiveRange = 0.0f;
		pInfo->nClipAmmoMax = INFINITE_AMMO;
		pInfo->nReserveAmmoMax = INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_NO_AMMO;
		pInfo->nReticleType = CReticle::TYPE_NONE;
	}

	m_pInfo = &m_aaInfo[WEAPON_TYPE_BLINK_RIGHT_HAND][0];
	m_pWorldMesh = &m_pWorldMeshArray[0];


	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


CEntityBuilder *CWeaponHand::GetLeafClassBuilder( void ) {
	return &_WeaponHandBuilder;
}


void CWeaponHand::_SetDefaults( void ) {
	FASSERT( IsCreated() );

	m_pOwnerBot = NULL;
	m_nUpgradeLevel = 0;
	m_nClipAmmo = 0;
	m_nReserveAmmo = 0;
}


void CWeaponHand::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();
	m_pWorldMesh->AddToWorld();
}


void CWeaponHand::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	m_pWorldMesh->RemoveFromWorld();
	CWeapon::ClassHierarchyRemoveFromWorld();
}


void CWeaponHand::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

//	if( IsInWorld() ) 
	{
		u32 i;

		if( bDrawingHasBeenEnabled ) {
			// Enable drawing of this weapon...

			for( i=0; i<EUK_COUNT_BLINK_RIGHT_HAND; ++i ) {
				FMATH_CLEARBITMASK( m_pWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		} else {
			// Disable drawing of this weapon...
			for( i=0; i<EUK_COUNT_BLINK_RIGHT_HAND; ++i ) {
				FMATH_SETBITMASK( m_pWorldMeshArray[i].m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}
	}
}


void CWeaponHand::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );

	if( IsInWorld() ) {
		m_pWorldMesh->UpdateTracker();
	}
}


void CWeaponHand::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}


void CWeaponHand::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
   	FASSERT( !IsTransitionState( CurrentState() ) );
	FASSERT( m_nUpgradeLevel < EUK_COUNT_BLINK_RIGHT_HAND );
	FASSERT( nPreviousUpgradeLevel < EUK_COUNT_BLINK_RIGHT_HAND );

	CFWorldMesh *pOldMesh = &m_pWorldMeshArray[nPreviousUpgradeLevel];
	pOldMesh->RemoveFromWorld();

	m_pInfo = m_pInfo = &m_aaInfo[WEAPON_TYPE_BLINK_RIGHT_HAND][m_nUpgradeLevel];
	m_pWorldMesh = &m_pWorldMeshArray[m_nUpgradeLevel];

	if( IsInWorld() ) {
		m_pWorldMesh->m_Xfm = pOldMesh->m_Xfm;
		m_pWorldMesh->UpdateTracker();
	}

}


void CWeaponHand::HumanTargeting_FindDesiredTargetPoint_WS( CFVec3A *pTargetPoint_WS ) {
	FASSERT( GetOwner() != NULL );
	pTargetPoint_WS->Mul( GetOwner()->m_MountUnitFrontXZ_WS, 1000.0f ).Add( GetOwner()->MtxToWorld()->m_vPos );
}

