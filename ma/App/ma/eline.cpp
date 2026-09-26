//////////////////////////////////////////////////////////////////////////////////////
// eline.cpp - Generic entity line class.
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
#include "eline.h"
#include "entity.h"
#include "GoodieProps.h"



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELineBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CELineBuilder _ELineBuilder;


void CELineBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_LINE, pszEntityType );

	m_fLength_MS = 1.0f;
}


BOOL CELineBuilder::InterpretTable( void ) {
	return CEntityBuilder::InterpretTable();
}


BOOL CELineBuilder::PostInterpretFixup( void ) {
	CFWorldShapeLine *pShapeLine;

	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	FASSERT( CEntityParser::m_pWorldShapeInit->m_nShapeType == FWORLD_SHAPETYPE_LINE );
	pShapeLine = CEntityParser::m_pWorldShapeInit->m_pLine;

	m_fLength_MS = pShapeLine->m_fLength;

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELine
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CELine::CELine() {
}


CELine::~CELine() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CELine::Create( f32 fLength_MS, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, u32 nTripwireEntityCount ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( fLength_MS > 0.0f );

	// Get pointer to the leaf class's builder object...
	CELineBuilder *pBuilder = (CELineBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_ELineBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_LINE );
	}

	// Set our builder parameters...

	pBuilder->m_fLength_MS = fLength_MS;

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName, nTripwireEntityCount );
}


void CELine::GetGoodieDistributionOrigin(CFVec3A *pPt_WS)
{
	FASSERT(pPt_WS);
	fmath_RandomPointOnLineSegTransformed(*pPt_WS, Length_WS(), *MtxToWorld());
}


void CELine::GetGoodieDistributionDir(CFVec3A *pVec_WS)
{
	FASSERT(pVec_WS);

	if(m_pGoodieProps != NULL)
	{
		fmath_RandomDirTransformed(*pVec_WS, 1.0f, 1.0f, m_pGoodieProps->m_fDispersion, *MtxToWorld());
		f32 fUnitControl = fmath_RandomFloat();
		pVec_WS->Mul(FMATH_FPOT(fUnitControl, m_pGoodieProps->m_afSpeed[0], m_pGoodieProps->m_afSpeed[1]));
	}
	else
	{
		fmath_RandomDirTransformed(*pVec_WS, 1.0f, 1.0f, 0.0f, *MtxToWorld());
	}
}


CEntityBuilder *CELine::GetLeafClassBuilder( void ) {
	return &_ELineBuilder;
}


BOOL CELine::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CELineBuilder *pBuilder = (CELineBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Initialize from builder object...
	m_fLength_MS = pBuilder->m_fLength_MS;
	m_fLength_WS = pBuilder->m_fLength_MS * pBuilder->m_fEC_Scale_WS;

	m_aPoint_WS[0] = pBuilder->m_EC_Mtx_WS.m_vPos;
	m_aPoint_WS[1].Mul( pBuilder->m_EC_Mtx_WS.m_vFront, m_fLength_WS ).Add( m_aPoint_WS[0] );

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CELine::ClassHierarchyRelocated( void *pIdentifier ) {
	FASSERT( IsCreated() );

	CEntity::ClassHierarchyRelocated( pIdentifier );

	if( !IsInWorld() ) {
		// We're not in the world...
		return;
	}

	_Move();
}


void CELine::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );

	CEntity::ClassHierarchyAddToWorld();

	_Move();
}


void CELine::_Move( void ) {
	m_fLength_WS = m_fLength_MS * m_fScaleToWorld;

	m_aPoint_WS[0] = m_MtxToWorld.m_vPos;
	m_aPoint_WS[1].Mul( m_MtxToWorld.m_vFront, m_fLength_WS ).Add( m_aPoint_WS[0] );
}

