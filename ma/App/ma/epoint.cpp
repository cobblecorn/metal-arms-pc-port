//////////////////////////////////////////////////////////////////////////////////////
// epoint.cpp - Generic entity point class.
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
// 04/30/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "epoint.h"
#include "entity.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEPointBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEPointBuilder _EPointBuilder;


void CEPointBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_POINT, pszEntityType );
}


BOOL CEPointBuilder::InterpretTable( void ) {
	return CEntityBuilder::InterpretTable();
}


BOOL CEPointBuilder::PostInterpretFixup( void ) {
	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEPoint
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CEPoint::CEPoint() {
}


CEPoint::~CEPoint() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CEPoint::Create( cchar *pszEntityName, const CFMtx43A *pMtx ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );

	// Get pointer to the leaf class's builder object...
	CEPointBuilder *pBuilder = (CEPointBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_EPointBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_POINT );
	}

	// Set our builder parameters...

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx );
}


CEntityBuilder *CEPoint::GetLeafClassBuilder( void ) {
	return &_EPointBuilder;
}


BOOL CEPoint::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEPointBuilder *pBuilder = (CEPointBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_SetDefaults();

	// Initialize from builder object...

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEPoint::_SetDefaults( void ) {
}

