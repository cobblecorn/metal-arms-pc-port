//////////////////////////////////////////////////////////////////////////////////////
// espline.cpp - Generic entity spline class.
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
#include "espline.h"
#include "entity.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CESplineBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CESplineBuilder _ESplineBuilder;


void CESplineBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_SPLINE, pszEntityType );

	m_nPointType = POINTTYPE_CFVEC3;
	m_nPointCount = 0;
	m_pPointArray = NULL;
	m_bAppendFirstPoint = FALSE;
}


BOOL CESplineBuilder::InterpretTable( void ) {
	return CEntityBuilder::InterpretTable();
}


BOOL CESplineBuilder::PostInterpretFixup( void ) {
	CFWorldShapeSpline *pShapeSpline;

	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	FASSERT( CEntityParser::m_pWorldShapeInit->m_nShapeType == FWORLD_SHAPETYPE_SPLINE );
	pShapeSpline = CEntityParser::m_pWorldShapeInit->m_pSpline;

	m_nPointType = POINTTYPE_CFVEC3;
	m_bAppendFirstPoint = !!pShapeSpline->m_bClosedSpline;
	m_nPointCount = pShapeSpline->m_nPointCount;
	m_pPointArray = pShapeSpline->m_pPtArray;

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CESpline
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CESpline::CESpline() {
	m_pPointArray = NULL;
}


CESpline::~CESpline() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


void CESpline::ClassHierarchyDestroy( void ) {
	fdelete_array( (CFVec3A *)m_pPointArray );
	m_pPointArray = NULL;
	CEntity::ClassHierarchyDestroy();
}


BOOL CESpline::CreateFromCopy( u32 nPointCount, const CFVec3 *pTempPointArray, BOOL bAppendFirstPoint, cchar *pszEntityName ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );

	return _Create( CESplineBuilder::POINTTYPE_CFVEC3, nPointCount, pTempPointArray, bAppendFirstPoint, pszEntityName );
}


BOOL CESpline::CreateFromCopy( u32 nPointCount, const CFVec3A *pTempPointArray, BOOL bAppendFirstPoint, cchar *pszEntityName ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );

	return _Create( CESplineBuilder::POINTTYPE_CFVEC3A, nPointCount, pTempPointArray, bAppendFirstPoint, pszEntityName );
}


BOOL CESpline::CreateInPlace( u32 nPointCount, const CFVec3A *pPermPointArray, BOOL bClosedSpline, cchar *pszEntityName ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );

	return _Create( CESplineBuilder::POINTTYPE_CFVEC3A_INPLACE, nPointCount, pPermPointArray, bClosedSpline, pszEntityName );
}


BOOL CESpline::_Create( CESplineBuilder::PointType_e nPointType, u32 nPointCount, const void *pPointArray, BOOL bAppendFirstPoint, cchar *pszEntityName ) {
	// Get pointer to the leaf class's builder object...
	CESplineBuilder *pBuilder = (CESplineBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_ESplineBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_SPLINE );
	}

	// Set our builder parameters...

	pBuilder->m_nPointType = nPointType;
	pBuilder->m_nPointCount = nPointCount;
	pBuilder->m_pPointArray = pPointArray;
	pBuilder->m_bAppendFirstPoint = bAppendFirstPoint;

	// Create an entity...
	return CEntity::Create( pszEntityName, &CFMtx43A::m_IdentityMtx );
}


CEntityBuilder *CESpline::GetLeafClassBuilder( void ) {
	return &_ESplineBuilder;
}


BOOL CESpline::ClassHierarchyBuild( void ) {
	u32 i;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CESplineBuilder *pBuilder = (CESplineBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_SetDefaults();

	// Initialize from builder object...

	m_nPointCount = pBuilder->m_nPointCount;

	if( m_nPointCount ) {
		m_bClosedSpline = !!pBuilder->m_bAppendFirstPoint;

		if( pBuilder->m_nPointType == CESplineBuilder::POINTTYPE_CFVEC3A_INPLACE ) {
			// Source vector array is provided in-place...
			m_pPointArray = (const CFVec3A *)pBuilder->m_pPointArray;
		} else {
			// Source vector array is to be copied to permanent memory...

			if( m_bClosedSpline ) {
				// Closed splines require one additional point...
				++m_nPointCount;
			}

			// Allocate point array...
			m_pPointArray = fnew CFVec3A [m_nPointCount];

			if( m_pPointArray == NULL ) {
				DEVPRINTF( "CESpline::ClassHierarchyBuild(): Not enough memory to build spline object.\n" );
				goto _ExitWithError;
			}

			// Copy source array to permanent memory...
			switch( pBuilder->m_nPointType ) {
			case CESplineBuilder::POINTTYPE_CFVEC3:

				for( i=0; i<m_nPointCount; i++ ) {
					if( i < pBuilder->m_nPointCount ) {
						((CFVec3A *)m_pPointArray)[i].Set( ((CFVec3 *)pBuilder->m_pPointArray)[i] );
					} else {
						((CFVec3A *)m_pPointArray)[i].Set( ((CFVec3 *)pBuilder->m_pPointArray)[0] );
					}
				}

				break;

			case CESplineBuilder::POINTTYPE_CFVEC3A:

				for( i=0; i<m_nPointCount; i++ ) {
					if( i < pBuilder->m_nPointCount ) {
						((CFVec3A *)m_pPointArray)[i].Set( ((CFVec3A *)pBuilder->m_pPointArray)[i] );
					} else {
						((CFVec3A *)m_pPointArray)[i].Set( ((CFVec3A *)pBuilder->m_pPointArray)[0] );
					}
				}

				break;
			}
		}
	}

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CESpline::_SetDefaults( void ) {
	m_bClosedSpline = FALSE;
	m_nPointCount = 0;
	m_pPointArray = NULL;
}

