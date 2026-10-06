//////////////////////////////////////////////////////////////////////////////////////
// ebox.cpp - Generic entity box class.
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
#include "ebox.h"
#include "entity.h"
#include "GoodieProps.h"
#include "meshtypes.h"
#include "fclib.h"
#include "fdraw.h"



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEBoxBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEBoxBuilder _EBoxBuilder;


void CEBoxBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_BOX, pszEntityType );

	m_HalfDim_MS.Set( 0.5f );

	// Let base class know that we're capable of being a tripwire...
	m_bEC_ClassIsTripwireCapable = TRUE;

	// Set ourself up to NOT be a Verlet collision surface (We will use the TYPE_COUNT enum to represent NONE)
	m_eVerletSurfaceType = CFVerletTackSurface::TYPE_COUNT;
	m_fVerletSurfaceFriction = 0.0f;
	m_fVerletSurfacePlaneDist = 0.0f;
	m_bVerletSurfacePreserveVelocity = FALSE;
	m_bVerletSurfaceDynamic = FALSE;  //static collision surface by default
}


BOOL CEBoxBuilder::InterpretTable( void ) {

	if(fclib_stricmp(CEntityParser::m_pszTableName, "PhysSurf") == 0)
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);

		if(fclib_stricmp(pszValue, "Plane") == 0)
		{
			m_eVerletSurfaceType = CFVerletTackSurface::TYPE_PLANE;
		}
		else if(fclib_stricmp(pszValue, "Box") == 0)
		{
			m_eVerletSurfaceType = CFVerletTackSurface::TYPE_BOX;
		}
		else if(fclib_stricmp(pszValue, "None") == 0)
		{
			m_eVerletSurfaceType = CFVerletTackSurface::TYPE_COUNT;
		}
		else
		{
			DEVPRINTF("CEBoxBuilder::InterpretTable() : Invalid value '%s' for 'PhysSurf' table.\n", pszValue);
			FASSERT_NOW;
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "PhysSurfFriction"))
	{
		CEntityParser::Interpret_F32(&m_fVerletSurfaceFriction);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "PhysSurfPlaneDist"))
	{
		CEntityParser::Interpret_F32(&m_fVerletSurfacePlaneDist);
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "PhysSurfDynamic"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bVerletSurfaceDynamic = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bVerletSurfaceDynamic = TRUE;
		}
		else
		{
			DEVPRINTF("CEBoxBuilder::InterpretTable() : Invalid value '%s' for 'PhysSurfDynamic' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "PhysSurfPreserveVelocity"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "true"))
		{
			m_bVerletSurfacePreserveVelocity = TRUE;
		}
		else if(!fclib_stricmp(pszValue, "false"))
		{
			m_bVerletSurfacePreserveVelocity = TRUE;
		}
		else
		{
			DEVPRINTF("CEBoxBuilder::InterpretTable() : Invalid value '%s' for 'PhysSurfPreserveVelocity' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	return CEntityBuilder::InterpretTable();
}


BOOL CEBoxBuilder::PostInterpretFixup( void ) {
	CFWorldShapeBox *pShapeBox;

	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	FASSERT( CEntityParser::m_pWorldShapeInit->m_nShapeType == FWORLD_SHAPETYPE_BOX );
	pShapeBox = CEntityParser::m_pWorldShapeInit->m_pBox;

	m_HalfDim_MS.Set( pShapeBox->m_fDimX, pShapeBox->m_fDimY, pShapeBox->m_fDimZ );
	m_HalfDim_MS.Mul( 0.5f );

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEBox
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CEBox::CEBox() {
	_SetDefaults();
}


CEBox::~CEBox() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CEBox::Create( f32 fDimX, f32 fDimY, f32 fDimZ, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, u32 nTripwireEntityCount ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( fDimX>=0.0f && fDimY>=0.0f && fDimZ>=0.0f );

	// Get pointer to the leaf class's builder object...
	CEBoxBuilder *pBuilder = (CEBoxBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_EBoxBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOX );
	}

	// Set our builder parameters...

	pBuilder->m_HalfDim_MS.Set( 0.5f*fDimX, 0.5f*fDimY, 0.5f*fDimZ );

	// Let base class know that we're capable of being a tripwire...
	pBuilder->m_bEC_ClassIsTripwireCapable = TRUE;

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName, nTripwireEntityCount );
}


void CEBox::GetGoodieDistributionOrigin( CFVec3A *pPt_WS ) {
	FASSERT(pPt_WS);
	fmath_RandomPointInBoxTransformed( *pPt_WS, HalfDimX_WS(), HalfDimY_WS(), HalfDimZ_WS(), *MtxToWorld() );
}


void CEBox::GetGoodieDistributionDir(CFVec3A *pVec_WS ) {
	FASSERT(pVec_WS);
	if( m_pGoodieProps != NULL ) {
		fmath_RandomDirTransformed( *pVec_WS, 1.0f, 1.0f, m_pGoodieProps->m_fDispersion, *MtxToWorld() );
		f32 fUnitControl = fmath_RandomFloat();
		pVec_WS->Mul( FMATH_FPOT(fUnitControl, m_pGoodieProps->m_afSpeed[0], m_pGoodieProps->m_afSpeed[1]) );
	} else {
		fmath_RandomDirTransformed( *pVec_WS, 1.0f, 1.0f, 0.0f, *MtxToWorld() );
	}
}


CEntityBuilder *CEBox::GetLeafClassBuilder( void ) {
	return &_EBoxBuilder;
}


BOOL CEBox::ClassHierarchyBuild( void ) {
	CFSphere Sphere;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEBoxBuilder *pBuilder = (CEBoxBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Initialize from builder object...
	m_HalfDim_MS = pBuilder->m_HalfDim_MS;
	m_HalfDim_WS.Mul( m_HalfDim_MS, pBuilder->m_fEC_Scale_WS );

	m_aCorner_WS[0].ReceiveNegative( m_HalfDim_WS );
	pBuilder->m_EC_Mtx_WS.MulPoint( m_aCorner_WS[0] );
	pBuilder->m_EC_Mtx_WS.MulPoint( m_aCorner_WS[1], m_HalfDim_WS );

	Sphere.m_Pos = pBuilder->m_EC_Mtx_WS.m_vPos.v3;
	Sphere.m_fRadius = m_HalfDim_WS.Mag();
	m_WorldUser.MoveTracker( Sphere );
	m_WorldUser.RemoveFromWorld();
	m_WorldUser.m_pUser = this;
	m_WorldUser.m_nUser = MESHTYPES_ENTITY;
	m_WorldUser.SetUserTypeBits( TypeBits() );

	// Set up tripwire handling...
	if( IsTripwire() ) {
		// We're a tripwire. It's nice to be special!

		m_pTripwire->m_BoundingSphere_MS.m_Pos.Zero();
		m_pTripwire->m_BoundingSphere_MS.m_fRadius = m_HalfDim_MS.Mag();
	}

	// Deal with verlet collision surface creation...
	if( pBuilder->m_eVerletSurfaceType != CFVerletTackSurface::TYPE_COUNT ) {
		//we need to create a verlet tachsurface object...
		m_pVerletTackSurface = fnew CFVerletTackSurface;
		if( m_pVerletTackSurface == NULL ) {
			DEVPRINTF( "CEBox::ClassHierarchyBuild: Could not allocate memory for a CFVerletTackSurface.\n" );
			goto _ExitWithError;
		}
		//At this point, the builder will know if we have a parent or not
		//set up the verlet surface... Call Init() Functions here...
		if( pBuilder->m_pszEC_ParentEntity || pBuilder->m_bVerletSurfaceDynamic ) { //we have a parent name or a dynamic override...
			//initialize the surface as dynamic
			if( pBuilder->m_eVerletSurfaceType == CFVerletTackSurface::TYPE_PLANE ) {
				m_pVerletTackSurface->Init_Plane_Dynamic( &m_MtxToWorld, FALSE, CFVerletTackSurface::PLANE_NORMAL_Z );
			} else { //initialize as a BOX
				m_pVerletTackSurface->Init_Box_Dynamic( &m_MtxToWorld, &m_HalfDim_MS, FALSE );
			}
		}
		else {
			//initialize the surface as static 
			if( pBuilder->m_eVerletSurfaceType == CFVerletTackSurface::TYPE_PLANE ) {
				m_pVerletTackSurface->Init_Plane_Static( &m_MtxToWorld.m_vPos, &m_MtxToWorld.m_vFront );
			} else { //initialize as a BOX
				m_pVerletTackSurface->Init_Box_Static( &m_MtxToWorld, &m_HalfDim_MS );
			}
		}
		//now, set up the properties for this surface...
		m_pVerletTackSurface->SetFriction( pBuilder->m_fVerletSurfaceFriction );
		m_pVerletTackSurface->SetPlaneDist( pBuilder->m_fVerletSurfacePlaneDist );
		m_pVerletTackSurface->SetPreserveVelocityFlag( pBuilder->m_bVerletSurfacePreserveVelocity );
	}
	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEBox::ClassHierarchyRelocated( void *pIdentifier ) {
	FASSERT( IsCreated() );

	CEntity::ClassHierarchyRelocated( pIdentifier );

	if( !IsInWorld() ) {
		// We're not in the world...
		return;
	}

	_Move();
}


void CEBox::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );

	CEntity::ClassHierarchyAddToWorld();

	_Move();
}


void CEBox::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );

	m_WorldUser.RemoveFromWorld();

	CEntity::ClassHierarchyRemoveFromWorld();
}


void CEBox::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = &m_WorldUser;
}


void CEBox::_Move( void ) {
	CFSphere Sphere;

	m_HalfDim_WS.Mul( m_HalfDim_MS, m_fScaleToWorld );

	m_aCorner_WS[0].ReceiveNegative( m_HalfDim_MS );
	m_MtxToWorld.MulPoint( m_aCorner_WS[0] );
	m_MtxToWorld.MulPoint( m_aCorner_WS[1], m_HalfDim_MS );

	if( IsInWorld() ) {
		Sphere.m_Pos = m_MtxToWorld.m_vPos.v3;
		Sphere.m_fRadius = m_HalfDim_WS.Mag();
		m_WorldUser.MoveTracker( Sphere );
	}
}


u32 CEBox::TripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS ) {
	u32 uRetVal = 0;
	BOOL bPrevIsInside, bNewIsInside;

//	DEVPRINTF("<%f, %f, %f> -> <%f, %f, %f>\n", pPrevPos_WS->x, pPrevPos_WS->y, pPrevPos_WS->z, pNewPos_WS->x, pNewPos_WS->y, pNewPos_WS->z);

	//First, check out if this is a killmode tripwire...
	if( m_pTripwire->m_eKillMode != CTripwire::TRIPWIRE_KILLMODE_NONE ) {
		//this is a special case tripwire... only do the necessary checking and no more!!!!
		u32 nRetFlags = 0; //no flags yet;

		if( m_pTripwire->m_eKillMode == CTripwire::TRIPWIRE_KILLMODE_VOLUME ) {
			//do standard volume checking
			bPrevIsInside = _PosInside( *pPrevPos_WS );
			bNewIsInside = _PosInside( *pNewPos_WS );
			if( !bPrevIsInside && bNewIsInside ) {
				//just entered this volume... time to die!
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

	bPrevIsInside = _PosInside( *pPrevPos_WS );
	bNewIsInside = _PosInside( *pNewPos_WS );

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

#if FANG_WINGC
	// A fast player can cross a thin box between frames without either endpoint
	// inside. Match sphere triggers by reporting both enter and exit in that case.
	// Kill volumes retain their separate tests above.
	CFVec3A vPrev, vMove, vBounds;
	vPrev.Sub( *pPrevPos_WS, m_aCorner_WS[0] );
	vMove.Sub( *pNewPos_WS, *pPrevPos_WS );
	vBounds.Sub( m_aCorner_WS[1], m_aCorner_WS[0] );
	const CFVec3A *apAxes[3] = { &m_MtxToWorld.m_vRight, &m_MtxToWorld.m_vUp, &m_MtxToWorld.m_vFront };
	f32 fEnter = 0.0f, fExit = 1.0f;
	for( u32 nAxis=0; nAxis < 3; nAxis++ ) {
		const f32 fExtent = vBounds.Dot( *apAxes[nAxis] );
		const f32 fStart = vPrev.Dot( *apAxes[nAxis] );
		const f32 fMove = vMove.Dot( *apAxes[nAxis] );
		if( fmath_Abs( fMove ) < 0.000001f ) {
			if( fStart < 0.0f || fStart > fExtent ) {
				return TRIPWIRE_COLLFLAG_NONE;
			}
			continue;
		}
		f32 fNear = -fStart / fMove;
		f32 fFar = (fExtent - fStart) / fMove;
		if( fNear > fFar ) {
			const f32 fSwap = fNear;
			fNear = fFar;
			fFar = fSwap;
		}
		FMATH_CLAMPMIN( fEnter, fNear );
		FMATH_CLAMPMAX( fExit, fFar );
		if( fEnter > fExit ) {
			return TRIPWIRE_COLLFLAG_NONE;
		}
	}
	return TRIPWIRE_COLLFLAG_ENTER_EVENT | TRIPWIRE_COLLFLAG_EXIT_EVENT;
#endif

	return TRIPWIRE_COLLFLAG_NONE;

	// JUSTIN: Temp code.
	if( _PosInside( *pNewPos_WS ) && !_PosInside( *pPrevPos_WS ) ) {
		uRetVal |= TRIPWIRE_COLLFLAG_ENTER_EVENT;
	}
/*	if( _PosInside( *pNewPos_WS ) ) {
		uRetVal |= TRIPWIRE_COLLFLAG_ENTER_EVENT;
	}*/

	return uRetVal;
#if 0
	f32 fAWdU[3], fADdU[3], fAWxDdU[3], fRhs;
	f32 fExtentX, fExtentY, fExtentZ;
	CFVec3A kSDir, kSCen, kDiff;

	kSDir.Sub( *pNewPos_WS, *pPrevPos_WS ).Mul( 0.5f );
	kSCen.Add( *pPrevPos_WS, kSDir );
	kDiff.Sub( kSCen, m_MtxToWorld.m_vPos );

	fExtentX = 2.0f * m_HalfDim_MS.x;
	fExtentY = 2.0f * m_HalfDim_MS.y;
	fExtentZ = 2.0f * m_HalfDim_MS.z;

	fAWdU[0] = fmath_Abs( kSDir.Dot( m_MtxToWorld.m_vRight ) );
	fADdU[0] = fmath_Abs( kDiff.Dot( m_MtxToWorld.m_vRight ) );
	fRhs = fExtentX + fAWdU[0];
	if( fADdU[0] > fRhs ) {
		return TRIPWIRE_COLLFLAG_NONE;
	}

	fAWdU[1] = fmath_Abs( kSDir.Dot( m_MtxToWorld.m_vUp ) );
	fADdU[1] = fmath_Abs( kDiff.Dot( m_MtxToWorld.m_vUp ) );
	fRhs = fExtentY + fAWdU[1];
	if( fADdU[1] > fRhs ) {
		return TRIPWIRE_COLLFLAG_NONE;
	}

	fAWdU[2] = fmath_Abs( kSDir.Dot( m_MtxToWorld.m_vFront ) );
	fADdU[2] = fmath_Abs( kDiff.Dot( m_MtxToWorld.m_vFront ) );
	fRhs = fExtentZ + fAWdU[2];
	if( fADdU[2] > fRhs ) {
		return TRIPWIRE_COLLFLAG_NONE;
	}

	CFVec3A kWxD;
	kWxD.Cross( kSDir, kDiff );

	fAWxDdU[0] = fmath_Abs( kWxD.Dot( m_MtxToWorld.m_vRight ) );
	fRhs = fExtentY*fAWdU[2] + fExtentZ*fAWdU[1];
	if( fAWxDdU[0] > fRhs ) {
		return TRIPWIRE_COLLFLAG_NONE;
	}

	fAWxDdU[1] = fmath_Abs( kWxD.Dot( m_MtxToWorld.m_vUp ) );
	fRhs = fExtentX*fAWdU[2] + fExtentZ*fAWdU[0];
	if( fAWxDdU[1] > fRhs ) {
		return TRIPWIRE_COLLFLAG_NONE;
	}

	fAWxDdU[2] = fmath_Abs( kWxD.Dot( m_MtxToWorld.m_vFront ) );
	fRhs = fExtentX*fAWdU[1] + fExtentY*fAWdU[0];
	if( fAWxDdU[2] > fRhs ) {
		return TRIPWIRE_COLLFLAG_NONE;
	}

	return TRUE;
#endif



// remove......xxxxxxxxxxxx
return TRIPWIRE_COLLFLAG_NONE;
#if 0
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
#endif
}



BOOL CEBox::_PosInside( const CFVec3A &vecPos ) {

	CFVec3A vecNormal;
	CFVec3A avecPlaneToPt[2];

	avecPlaneToPt[0] = vecPos;
	avecPlaneToPt[0].Sub(m_aCorner_WS[0]);
	avecPlaneToPt[1] = vecPos;
	avecPlaneToPt[1].Sub(m_aCorner_WS[1]);

	// JUSTIN: Optimizations: negate the normal, calculate only two distance to pts.

	//////////////////////////////////////////////////////////////////////
	// Check against the plane z = -1 in MS.
	vecNormal.Set(0.0f, 0.0f, -1.0f);
	m_MtxToWorld.MulDir(vecNormal);

	if(avecPlaneToPt[0].Dot(vecNormal) > 0.0f) {
		return FALSE;
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Check against the plane z = +1 in MS.
//	vecNormal.Set(0.0f, 0.0f, +1.0f);
//	m_MtxToWorld.MulDir(vecNormal);

	// Instead of negating the normal we change the sign on our comparison.
	if(avecPlaneToPt[1].Dot(vecNormal) < 0.0f) {
		return FALSE;
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Check against the plane x = -1 in MS.
	vecNormal.Set(-1.0f, 0.0f, 0.0f);
	m_MtxToWorld.MulDir(vecNormal);

	if(avecPlaneToPt[0].Dot(vecNormal) > 0.0f) {
		return FALSE;
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Check against the plane x = +1 in MS.
	// JUSTIN: Optimize this?
//	vecNormal.Set(+1.0f, 0.0f, 0.0f);
//	m_MtxToWorld.MulDir(vecNormal);

	// Instead of negating the normal we change the sign on our comparison.
	if(avecPlaneToPt[1].Dot(vecNormal) < 0.0f) {
		return FALSE;
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Check against the plane y = -1 in MS.
	vecNormal.Set(0.0f, -1.0f, 0.0f);
	m_MtxToWorld.MulDir(vecNormal);

	if(avecPlaneToPt[0].Dot(vecNormal) > 0.0f) {
		return FALSE;
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Check against the plane y = +1 in MS.
	// JUSTIN: Optimize this?
//	vecNormal.Set(0.0f, +1.0f, 0.0f);
//	m_MtxToWorld.MulDir(vecNormal);

	// Instead of negating the normal we change the sign on our comparison.
	if(avecPlaneToPt[1].Dot(vecNormal) < 0.0f) {
		return FALSE;
	}
	//
	//////////////////////////////////////////////////////////////////////

	return TRUE;
}


void CEBox::_SetDefaults( void ) {
	m_pVerletTackSurface = NULL;	// This box entity could represent a collision surface
}

BOOL CEBox::IsPointWithinBox(const CFVec3A& rTestPointWS){
	// first transform TestPoint into box local coordinates:
	CFMtx43A mtxWorldToBox;
	CFVec3A  vTestPt_BS;
	mtxWorldToBox.ReceiveAffineInverse( m_MtxToWorld, FALSE );
	mtxWorldToBox.MulPoint( vTestPt_BS, rTestPointWS );

	if( vTestPt_BS.x < -m_HalfDim_WS.x ) {
		return FALSE;
	}
	else if( vTestPt_BS.x >  m_HalfDim_WS.x ) {
		return FALSE;
	}
	else if( vTestPt_BS.y < -m_HalfDim_WS.y ) {
		return FALSE;
	}
	else if( vTestPt_BS.y >  m_HalfDim_WS.y ) {
		return FALSE;
	}
	else if( vTestPt_BS.z < -m_HalfDim_WS.z ) {
		return FALSE;
	}
	else if( vTestPt_BS.z >  m_HalfDim_WS.z ) {
		return FALSE;
	}
	else return TRUE;
}



BOOL CEBox::IsPointWithinBoxXZ(const CFVec3A& rTestPointWS){
	// first transform TestPoint into box local coordinates:
	CFMtx43A mtxWorldToBox;
	CFVec3A  vTestPt_BS;
	mtxWorldToBox.ReceiveAffineInverse( m_MtxToWorld, FALSE );
	mtxWorldToBox.MulPoint( vTestPt_BS, rTestPointWS );

	if( vTestPt_BS.x < -m_HalfDim_WS.x ) {
		return FALSE;
	}
	else if( vTestPt_BS.x >  m_HalfDim_WS.x ) {
		return FALSE;
	}
	else if( vTestPt_BS.z < -m_HalfDim_WS.z ) {
		return FALSE;
	}
	else if( vTestPt_BS.z >  m_HalfDim_WS.z ) {
		return FALSE;
	}
	else return TRUE;
}



BOOL CEBox::_PassedThroughOriginZPlane( const CFVec3A *pVecPosA, const CFVec3A *pVecPosB ) {

	CFMtx43A mtxWorldToBox;
	CFVec3A  vTestPtA_BS;
	CFVec3A  vTestPtB_BS;

	mtxWorldToBox.ReceiveAffineInverse( m_MtxToWorld, FALSE );
	mtxWorldToBox.MulPoint( vTestPtA_BS, *pVecPosA );
	mtxWorldToBox.MulPoint( vTestPtB_BS, *pVecPosB );

	//if any of the two points are ON the origin, return TRUE;
	if( vTestPtA_BS.z == 0.0f )
		return TRUE;
	if( vTestPtB_BS.z == 0.0f )
		return TRUE;

	BOOL bOnOppositeSides = FALSE;
	if( vTestPtA_BS.z > 0.0f ) {
		if( vTestPtB_BS.z <= 0.0f ) {
			bOnOppositeSides = TRUE;
		}
	} else {
		if( vTestPtB_BS.z >= 0.0f ) {
			bOnOppositeSides = TRUE;
		}
	}

	if( !bOnOppositeSides ) {
		return FALSE;
	}

	//if we are here, then we need to figure out where these two backtransformed points intersect the XY plane at Z = 0...
	//CFVec3A vecNormal;
	//vecNormal.Set(0.0f, 0.0f, 1.0f);
	f32 fADot = vTestPtA_BS.z; // since x and y are zero, then we are just interested in the z components...
	f32 fBDot = vTestPtB_BS.z;


//	f32 fT = - fADot / ( fBDot - fADot );
	f32 fT = - fmath_Div(fADot, fBDot - fADot );

	CFVec3A vIntersectionPoint;

	//vIntersectionPoint = vTestPtA_BS + ( fT * ( vTestPtB_BS - vTestPtA_BS ) );
	vIntersectionPoint.Sub( vTestPtB_BS, vTestPtA_BS );
	vIntersectionPoint.Mul( fT );
	vIntersectionPoint.Add( vTestPtA_BS );

	if( fmath_Abs( vIntersectionPoint.x ) > m_HalfDim_WS.x ) {
		return FALSE;
	}
	if( fmath_Abs( vIntersectionPoint.y ) > m_HalfDim_WS.y ) {
		return FALSE;
	}

	//if we are here, we have a point crossing the boundry, and within the bounds of the plane...
	return TRUE;
}

void CEBox::Draw( void ) {
#if !FANG_PRODUCTION_BUILD
	CFVec3A avPoints[8];
	s32 nIndex;

	for( nIndex = 0; nIndex < 8; nIndex++ )	{
		avPoints[nIndex] = m_HalfDim_WS.v3;
	}

	avPoints[0].x = -avPoints[0].x;	avPoints[0].y = -avPoints[0].y;	avPoints[0].z = -avPoints[0].z;
	avPoints[1].x = -avPoints[1].x;	avPoints[1].y = -avPoints[1].y;
	avPoints[2].x = -avPoints[2].x;									avPoints[2].z = -avPoints[2].z;
	avPoints[3].x = -avPoints[3].x;
									avPoints[4].y = -avPoints[4].y;	avPoints[4].z = -avPoints[4].z;
									avPoints[5].y = -avPoints[5].y;
																	avPoints[6].z = -avPoints[6].z;

	for( nIndex = 0; nIndex < 8; nIndex++ )	{
		m_MtxToWorld.MulPoint( avPoints[nIndex] );
	}

	fdraw_DevLine( &avPoints[0].v3, &avPoints[1].v3 );
	fdraw_DevLine( &avPoints[1].v3, &avPoints[3].v3 );
	fdraw_DevLine( &avPoints[3].v3, &avPoints[2].v3 );
	fdraw_DevLine( &avPoints[2].v3, &avPoints[0].v3 );
	fdraw_DevLine( &avPoints[0].v3, &avPoints[4].v3 );
	fdraw_DevLine( &avPoints[1].v3, &avPoints[5].v3 );
	fdraw_DevLine( &avPoints[2].v3, &avPoints[6].v3 );
	fdraw_DevLine( &avPoints[3].v3, &avPoints[7].v3 );
	fdraw_DevLine( &avPoints[4].v3, &avPoints[5].v3 );
	fdraw_DevLine( &avPoints[5].v3, &avPoints[7].v3 );
	fdraw_DevLine( &avPoints[7].v3, &avPoints[6].v3 );
	fdraw_DevLine( &avPoints[6].v3, &avPoints[4].v3 );
#endif
}
