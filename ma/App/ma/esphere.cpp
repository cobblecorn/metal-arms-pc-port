//////////////////////////////////////////////////////////////////////////////////////
// esphere.cpp - Generic entity sphere class.
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
#include "esphere.h"
#include "entity.h"
#include "GoodieProps.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CESphereBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CESphereBuilder _ESphereBuilder;


void CESphereBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_SPHERE, pszEntityType );

	m_fRadius = 2.0f;

	// Let base class know that we're capable of being a tripwire...
	m_bEC_ClassIsTripwireCapable = TRUE;
}


BOOL CESphereBuilder::InterpretTable( void ) {
	return CEntityBuilder::InterpretTable();
}


BOOL CESphereBuilder::PostInterpretFixup( void ) {
	CFWorldShapeSphere *pShapeSphere;

	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	FASSERT( CEntityParser::m_pWorldShapeInit->m_nShapeType == FWORLD_SHAPETYPE_SPHERE );
	pShapeSphere = CEntityParser::m_pWorldShapeInit->m_pSphere;

	m_fRadius = pShapeSphere->m_fRadius;

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CESphere
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CESphere::CESphere() {
}


CESphere::~CESphere() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CESphere::Create( f32 fRadius, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, u32 nTripwireEntityCount ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( fRadius >= 0.0f );

	// Get pointer to the leaf class's builder object...
	CESphereBuilder *pBuilder = (CESphereBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_ESphereBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_SPHERE );
	}

	// Set our builder parameters...

	pBuilder->m_fRadius = fRadius;

	// Let base class know that we're capable of being a tripwire...
	pBuilder->m_bEC_ClassIsTripwireCapable = TRUE;

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName, nTripwireEntityCount );
}


void CESphere::GetGoodieDistributionOrigin(CFVec3A *pPt_WS) {
	FASSERT(pPt_WS);
	fmath_RandomPointInSphereTransformed(*pPt_WS, Radius_MS(), *MtxToWorld());
}


void CESphere::GetGoodieDistributionDir(CFVec3A *pVec_WS) {
	FASSERT(pVec_WS);
	if( m_pGoodieProps != NULL ) {
		fmath_RandomDirTransformed( *pVec_WS, 1.0f, 1.0f, m_pGoodieProps->m_fDispersion, *MtxToWorld() );
		f32 fUnitControl = fmath_RandomFloat();
		pVec_WS->Mul(FMATH_FPOT( fUnitControl, m_pGoodieProps->m_afSpeed[0], m_pGoodieProps->m_afSpeed[1]) );
	} else {
		fmath_RandomDirTransformed( *pVec_WS, 1.0f, 1.0f, 0.0f, *MtxToWorld() );
	}
}


CEntityBuilder *CESphere::GetLeafClassBuilder( void ) {
	return &_ESphereBuilder;
}


BOOL CESphere::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CESphereBuilder *pBuilder = (CESphereBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_SetDefaults();

	// Initialize from builder object...
	m_fRadius_MS = pBuilder->m_fRadius;

	m_SphereA_WS.m_Pos = pBuilder->m_EC_Mtx_WS.m_vPos;
	m_SphereA_WS.m_fRadius = m_fRadius_MS * pBuilder->m_fEC_Scale_WS;

	// Set up tripwire handling...
	if( IsTripwire() ) {
		// We're a tripwire. It's nice to be special!

		m_pTripwire->m_BoundingSphere_MS.m_Pos.Zero();
		m_pTripwire->m_BoundingSphere_MS.m_fRadius = m_fRadius_MS;
	}

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CESphere::_SetDefaults( void ) {
	m_fRadius_MS = 2.0f;
	m_SphereA_WS.Set( CFVec3A::m_Null, m_fRadius_MS );
}


void CESphere::ClassHierarchyRelocated( void *pIdentifier ) {
	FASSERT( IsCreated() );

	CEntity::ClassHierarchyRelocated( pIdentifier );

	m_SphereA_WS.m_Pos = m_MtxToWorld.m_vPos;
	m_SphereA_WS.m_fRadius = m_fRadius_MS * m_fScaleToWorld;
}



u32 CESphere::TripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS ) {

	//First, check out if this is a killmode tripwire...
	if( m_pTripwire->m_eKillMode != CTripwire::TRIPWIRE_KILLMODE_NONE ) {
		//this is a special case tripwire... only do the necessary checking and no more!!!!
		u32 nRetFlags = 0; //no flags yet;

		if( m_pTripwire->m_eKillMode == CTripwire::TRIPWIRE_KILLMODE_VOLUME ) {
			//do standard volume checking
			u32 nIntersectionFlags = _PerformTripwireCollisionTest( pPrevPos_WS, pNewPos_WS );
			if( nIntersectionFlags & ( TRIPWIRE_COLLFLAG_ENTER_EVENT | TRIPWIRE_COLLFLAG_NEWPOS_INSIDE ) ) {
				//we are inside the bounding volume...  Time to die!
				nRetFlags = TRIPWIRE_COLLFLAG_KILL_NOW;
			}
		} else {
			//This is a Kill-Plane tripwire object...
			if( _PassedThroughOriginZPlane( pPrevPos_WS, pNewPos_WS ) ) {
				// passed through the origin. tiem to die!
				nRetFlags = TRIPWIRE_COLLFLAG_KILL_NOW;
			}
		}
		//check to see if we are supposed to die this frame....
		//if so, check to see if we should spawn our death effects
		if( nRetFlags && m_pTripwire->m_bKillModeSpawnDeathEffects ) {
			nRetFlags |= TRIPWIRE_COLLFLAG_SPAWN_DEATH_EFFECTS;
		}
		return nRetFlags;
	}

	//if we are here, this is a traditional tripwire, do the traditional thing!
	return _PerformTripwireCollisionTest( pPrevPos_WS, pNewPos_WS );
}


u32 CESphere::_PerformTripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS ) {
	CFVec3A P1P2, P1ToCenter, P2ToCenter;
	f32 fV, fOOP1P2Dist2, fD2;
	BOOL bPrevIsInside, bNewIsInside;

	bPrevIsInside = bNewIsInside = FALSE;

	if( m_pTripwire->m_BoundingSphere_WS.IsIntersecting( *pPrevPos_WS ) ) {
		bPrevIsInside = TRUE;
	}

	if( m_pTripwire->m_BoundingSphere_WS.IsIntersecting( *pNewPos_WS ) ) {
		bNewIsInside = TRUE;
	}

	if( bPrevIsInside && bNewIsInside ) {
		// Previous and new points are both inside...
		return TRIPWIRE_COLLFLAG_NEWPOS_INSIDE;
	}

	if( bNewIsInside ) {
		// Only the new point is inside...
		return TRIPWIRE_COLLFLAG_NEWPOS_INSIDE | TRIPWIRE_COLLFLAG_ENTER_EVENT;
	}

	if( bPrevIsInside ) {
		// Only the previous point is inside...
		return TRIPWIRE_COLLFLAG_EXIT_EVENT;
	}

	// Both points are outside. We must perform a more expensive test...

	P1P2.Sub( *pNewPos_WS, *pPrevPos_WS );
	P1ToCenter.Sub( m_pTripwire->m_BoundingSphere_WS.m_Pos, *pPrevPos_WS );

	fV = P1ToCenter.Dot( P1P2 );
	if( fV <= 0.0f ) {
		// P1P2 is pointing away from the sphere (or P1 & P2 are the same point)...
		return TRIPWIRE_COLLFLAG_NONE;
	}

	P2ToCenter.Sub( m_pTripwire->m_BoundingSphere_WS.m_Pos, *pNewPos_WS );
	if( P2ToCenter.Dot( P1P2 ) >= 0.0f ) {
		// P1P2 is pointing away from the sphere...
		return TRIPWIRE_COLLFLAG_NONE;
	}

	fOOP1P2Dist2 = P1P2.InvMagSq();
	fD2 = m_pTripwire->m_BoundingSphere_WS.m_fRadius*m_pTripwire->m_BoundingSphere_WS.m_fRadius - ( P1ToCenter.MagSq() - fV*fV*fOOP1P2Dist2 );

	if( fD2 <= 0.0f ) {
		// Infinite line colinear with P1P2 doesn't intersect the sphere...
		return TRIPWIRE_COLLFLAG_NONE;
	}

	// Line segment intersects the sphere...

	return TRIPWIRE_COLLFLAG_ENTER_EVENT | TRIPWIRE_COLLFLAG_EXIT_EVENT;
}


BOOL CESphere::_PassedThroughOriginZPlane( const CFVec3A *pVecPosA, const CFVec3A *pVecPosB ) {

	CFMtx43A mtxWorldToMS;
	CFVec3A  vTestPtA_MS;
	CFVec3A  vTestPtB_MS;

	//back transfrom the two points into Model Space...
	mtxWorldToMS.ReceiveAffineInverse( m_MtxToWorld, FALSE );
	mtxWorldToMS.MulPoint( vTestPtA_MS, *pVecPosA );
	mtxWorldToMS.MulPoint( vTestPtB_MS, *pVecPosB );

	//if any of the two points are ON the origin, return TRUE;
	if( vTestPtA_MS.z == 0.0f )
		return TRUE;
	if( vTestPtB_MS.z == 0.0f )
		return TRUE;

	BOOL bOnOppositeSides = FALSE;
	if( vTestPtA_MS.z > 0.0f ) {
		if( vTestPtB_MS.z <= 0.0f ) {
			bOnOppositeSides = TRUE;
		}
	} else {
		if( vTestPtB_MS.z >= 0.0f ) {
			bOnOppositeSides = TRUE;
		}
	}

	if( !bOnOppositeSides ) {
		return FALSE;
	}

	//if we are here, then we need to figure out where these two backtransformed points intersect the XY plane at Z = 0...
//	CFVec3A vecNormal;
//	vecNormal.Set(0.0f, 0.0f, 1.0f);
	f32 fADot = vTestPtA_MS.z; // since x and y are zero, then we are just interested in the z components...
	f32 fBDot = vTestPtB_MS.z;

	f32 fT = - fmath_Div(fADot, fBDot - fADot );

	CFVec3A vIntersectionPoint;

	//vIntersectionPoint = vTestPtA_BS + ( fT * ( vTestPtB_BS - vTestPtA_BS ) );
	vIntersectionPoint.Sub( vTestPtB_MS, vTestPtA_MS );
	vIntersectionPoint.Mul( fT );
	vIntersectionPoint.Add( vTestPtA_MS );

	//now, see if we are within the radius...  Remember, we are in model space, so the origin of the sphere is 0,0,0!
	if( vIntersectionPoint.MagSq() > ( m_pTripwire->m_BoundingSphere_WS.m_fRadius * m_pTripwire->m_BoundingSphere_WS.m_fRadius ) ) {
		return FALSE;
	}

	return TRUE;
}
