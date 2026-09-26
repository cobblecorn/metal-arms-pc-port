//////////////////////////////////////////////////////////////////////////////////////
// SplineActor.cpp - Wrapper class meant to simplify MeshEntity spline based scripting
//
// Author: Russell Foushee
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/18/03 Miller		You can now pass in a NULL mesh entity, but still get a position
//						and orientation.
// 01/20/03 Foushee		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "SplineActor.h"
#include "fres.h"
#include "fscriptsystem.h"

// NKM
// The code to build the matrix here was taken from Actor.cpp in the CMeshActor::Work() function.
// If it breaks here, you'll need to change it in CMeshActor as well.
static FINLINE void _GetMatrixFromPointsAndQuat( CFMtx43A *pMtx, CFVec3AObj *pPoints, CFQuatObj *pQuat ) {
	FASSERT( pMtx );
	FASSERT( pPoints );

	pMtx->Identity();

	if( !pPoints->IsInitted() ) {
		return;
	}

	if( pQuat && pQuat->IsInitted() ) {
		CFQuatA tmpA = pQuat->GetValue();
		f32 fMag = tmpA.v.MagSq();
		FASSERT(fMag > 0.0f);
		fMag = 1.0f/fmath_AcuSqrt(fMag);
		tmpA.x *= fMag;
		tmpA.y *= fMag;
		tmpA.z *= fMag;
		tmpA.w *= fMag;
		tmpA.BuildMtx( *pMtx );
	}

	pMtx->m_vPos = pPoints->GetValue();
}


BOOL CSplineActor::AcquireMemberObjects( void ) {

	//create all the objects we currently need
	if( !( m_poPointPath2 = CFV3OPointPath2::GetAvailable() ) ){
		DEVPRINTF("CSplineActor::Acquire : Could not get a CFV3OPointPath2 Object.");
		return FALSE;
	}

	//get a MeshActor object
	if( !( m_poMeshActor = CMeshActor::Acquire() ) ){
		DEVPRINTF("CSplineActor::Acquire : Could not get a CMeshActor Object.");
		return FALSE;
	}

	return TRUE;
}



void CSplineActor::SetTimePos( f32 fNewTimePos ) {
	FMATH_CLAMPMIN( fNewTimePos, 0.0f );
	FMATH_CLAMPMAX( fNewTimePos, m_poPointPath2->GetTotalTime() );

	m_poPointPath2->SetTimePos( fNewTimePos );
}


CESpline* CSplineActor::FindCESplineFromString( cchar *pszSplineName ) {

	CEntity *pE = CEntity::Find(pszSplineName);
	if(pE == NULL)
	{
		DEVPRINTF("CSplineActor::FindCESplineFromString : Could not find entity named '%s'.", pszSplineName);
		return NULL;
	}
	if(!(pE->TypeBits() & ENTITY_BIT_SPLINE))
	{
		SCRIPT_ERROR("CSplineActor::FindCESplineFromString : Found object named '%s' that was not a spline.", pszSplineName);
		return NULL;
	}
	CESpline *pSE = (CESpline *)(pE);
	return pSE;
}


CStaticOrientationSplineActor* CStaticOrientationSplineActor::Acquire( void ) {
	CStaticOrientationSplineActor* pNewSplineActor = fnew CStaticOrientationSplineActor;

	if( pNewSplineActor == NULL ) {
		DEVPRINTF("CStaticOrientationSplineActor::Acquire() : NULL fnew -- Class creation FAILED\n");
		return NULL;
	}

	if( !pNewSplineActor->AcquireMemberObjects() ) {
		return NULL;
	}

	return pNewSplineActor;
}


BOOL CStaticOrientationSplineActor::AcquireMemberObjects( void ) {

	//this class does not need to get any additional objects.
	//just call the base class AcquireMemberObjects().

	return CSplineActor::AcquireMemberObjects();
}



BOOL CStaticOrientationSplineActor::Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity *pME/* = NULL*/, BOOL bAutoStart ) {

	CESpline *pSE = FindCESplineFromString( pszSplineName );
	if( !pSE ) {
		return FALSE;
	}

	//initialize the CFV3OPointPath2 object. 
	m_poPointPath2->Init( pSE->PointArray(), pSE->PointCount(), fTotalTime, bAutoStart );

	//initialize the MeshActor object if we need it, otherwise return it
	if( pME ) {
		m_poMeshActor->Init(pME, m_poPointPath2, NULL);
	
		if( bAutoStart ){
			m_poMeshActor->Activate();
		}
	} else {
		m_poMeshActor->Release();
		m_poMeshActor = NULL;
	}

	return TRUE;
}



void CStaticOrientationSplineActor::Reset( BOOL bAutoStart ) {
	m_poPointPath2->Reset( bAutoStart );
}


void CStaticOrientationSplineActor::Start ( void ) {
	m_poPointPath2->Start();

	if( m_poMeshActor ) {
		m_poMeshActor->Activate();
	}
}


void CStaticOrientationSplineActor::Stop ( void ) {
	m_poPointPath2->Pause();

	if( m_poMeshActor ) {
		m_poMeshActor->Deactivate();
	}
}

void CStaticOrientationSplineActor::GetMatrix( CFMtx43A *pMtx ) {
	_GetMatrixFromPointsAndQuat( pMtx, m_poPointPath2, NULL );
}


CGeneralSplineActor* CGeneralSplineActor::Acquire( void ) {
	CGeneralSplineActor* pNewSplineActor = fnew CGeneralSplineActor;

	if( pNewSplineActor == NULL ) {
		DEVPRINTF("CGeneralSplineActor::Acquire() : NULL fnew -- Class creation FAILED\n");
		return NULL;
	}

	if( !pNewSplineActor->AcquireMemberObjects() ) {
		return NULL;
	}

	return pNewSplineActor;
}


BOOL CGeneralSplineActor::AcquireMemberObjects( void ) {

	//get a FQOTang1 object
	if( !( m_poQuatTang1 = CFQOTang1::GetAvailable() ) ){
		DEVPRINTF("CGeneralSplineActor::AcquireMemberObjects : Could not get a CFQOTang1 Object.");
		return FALSE;
	}

	return CSplineActor::AcquireMemberObjects();
}



BOOL CGeneralSplineActor::Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity *pME/* = NULL*/, BOOL bAutoStart ) {

	CESpline *pSE = FindCESplineFromString( pszSplineName );
	if( !pSE ) {
		return FALSE;
	}

	//initialize the CFV3OPointPath2 object. 
	m_poPointPath2->Init( pSE->PointArray(), pSE->PointCount(), fTotalTime, bAutoStart );

	//initizlie the m_poQuatTang object...
	m_poQuatTang1->Init( m_poPointPath2, bAutoStart );

	//initialize the MeshActor object if we need it, otherwise return it
	if( pME ) {
		m_poMeshActor->Init(pME, m_poPointPath2, m_poQuatTang1);
	
		if( bAutoStart ){
			m_poMeshActor->Activate();
		}
	} else {
		m_poMeshActor->Release();
		m_poMeshActor = NULL;
	}

	return TRUE;
}



void CGeneralSplineActor::Reset( BOOL bAutoStart ) {
	m_poPointPath2->Reset( bAutoStart );
	m_poQuatTang1->Reset( bAutoStart );
}


void CGeneralSplineActor::Start ( void ) {
	m_poPointPath2->Start();
	m_poQuatTang1->Start();

	if( m_poMeshActor ) {
		m_poMeshActor->Activate();
	}
}


void CGeneralSplineActor::Stop ( void ) {
	m_poPointPath2->Pause();
	m_poQuatTang1->Pause();

	if( m_poMeshActor ) {
		m_poMeshActor->Deactivate();
	}
}

void CGeneralSplineActor::GetMatrix( CFMtx43A *pMtx ) {
	_GetMatrixFromPointsAndQuat( pMtx, m_poPointPath2, m_poQuatTang1 );
}



CHorizontalSplineActor* CHorizontalSplineActor::Acquire( void ) {
	CHorizontalSplineActor* pNewSplineActor = fnew CHorizontalSplineActor;

	if( pNewSplineActor == NULL ) {
		DEVPRINTF("CHorizontalSplineActor::Acquire() : NULL fnew -- Class creation FAILED\n");
		return NULL;
	}

	if( !pNewSplineActor->AcquireMemberObjects() ) {
		return NULL;
	}

	return pNewSplineActor;
}


BOOL CHorizontalSplineActor::AcquireMemberObjects( void ) {

	//get a FQuatTang2 object
	if( !( m_poQuatTang2 = CFQuatTang2::GetAvailable() ) ){
		DEVPRINTF("CHorizontalSplineActor::AcquireMemberObjects : Could not get a CFQuatTang2 Object.");
		return FALSE;
	}

	return CSplineActor::AcquireMemberObjects();
}



BOOL CHorizontalSplineActor::Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity *pME/* = NULL*/, BOOL bAutoStart ) {

	CESpline *pSE = FindCESplineFromString( pszSplineName );
	if( !pSE ) {
		return FALSE;
	}

	//initialize the CFV3OPointPath2 object. 
	m_poPointPath2->Init( pSE->PointArray(), pSE->PointCount(), fTotalTime, bAutoStart );

	//initizlie the m_poQuatTang2 object...
	m_poQuatTang2->Init( m_poPointPath2, bAutoStart );

	//initialize the MeshActor object if we need it, otherwise return it
	if( pME ) {
		m_poMeshActor->Init(pME, m_poPointPath2, m_poQuatTang2);
	
		if( bAutoStart ){
			m_poMeshActor->Activate();
		}
	} else {
		m_poMeshActor->Release();
		m_poMeshActor = NULL;
	}

	return TRUE;
}



void CHorizontalSplineActor::Reset( BOOL bAutoStart ) {
	m_poPointPath2->Reset( bAutoStart);
	m_poQuatTang2->Reset( bAutoStart );
}


void CHorizontalSplineActor::Start ( void ) {
	m_poPointPath2->Start();
	m_poQuatTang2->Start();

	if( m_poMeshActor ) {
		m_poMeshActor->Activate();
	}
}


void CHorizontalSplineActor::Stop ( void ) {
	m_poPointPath2->Pause();
	m_poQuatTang2->Pause();

	if( m_poMeshActor ) {
		m_poMeshActor->Deactivate();
	}
}

void CHorizontalSplineActor::GetMatrix( CFMtx43A *pMtx ) {
	_GetMatrixFromPointsAndQuat( pMtx, m_poPointPath2, m_poQuatTang2 );
}




CSwingingSplineActor* CSwingingSplineActor::Acquire( void ) {
	CSwingingSplineActor* pNewSplineActor = fnew CSwingingSplineActor;

	if( pNewSplineActor == NULL ) {
		DEVPRINTF("CSwingingSplineActor::Acquire() : NULL fnew -- Class creation FAILED\n");
		return NULL;
	}

	if( !pNewSplineActor->AcquireMemberObjects() ) {
		return NULL;
	}

	return pNewSplineActor;
}


BOOL CSwingingSplineActor::AcquireMemberObjects( void ) {

	//get a FQuatTang2 (orientation) object
	if( !( m_poQuatTang2 = CFQuatTang2::GetAvailable() ) ){
		DEVPRINTF("CSwingingSplineActor::AcquireMemberObjects : Could not get a CFQuatTang2 Object.");
		return FALSE;
	}

	//get a FQuatTang3 (acceleration) object
	if( !( m_poQuatTang3 = CFQuatTang3::GetAvailable() ) ){
		DEVPRINTF("CSwingingSplineActor::AcquireMemberObjects : Could not get a CFQuatTang3 Object.");
		return FALSE;
	}

	//get a FQuatComp (composition) object
	if( !( m_poQuatComp = CFQuatComp::GetAvailable() ) ){
		DEVPRINTF("CSwingingSplineActor::AcquireMemberObjects : Could not get a CFQuatComp Object.");
		return FALSE;
	}

	return CSplineActor::AcquireMemberObjects();
}



BOOL CSwingingSplineActor::Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity *pME/* = NULL*/, BOOL bAutoStart ) {

	CESpline *pSE = FindCESplineFromString( pszSplineName );
	if( !pSE ) {
		return FALSE;
	}

	//initialize the CFV3OPointPath2 object. 
	m_poPointPath2->Init( pSE->PointArray(), pSE->PointCount(), fTotalTime, bAutoStart );

	//initizlie the m_poQuatTang2 object...
	m_poQuatTang2->Init( m_poPointPath2, bAutoStart );

	//initizlie the m_poQuatTang3 object...
	m_poQuatTang3->Init( m_poPointPath2, bAutoStart );

	//initizlie the m_poQuatComp object...
	m_poQuatComp->Init( m_poQuatTang2, m_poQuatTang3, bAutoStart );

	//initialize the MeshActor object if we need it, otherwise return it
	if( pME ) {
		m_poMeshActor->Init(pME, m_poPointPath2, m_poQuatComp);
	
		if( bAutoStart ){
			m_poMeshActor->Activate();
		}
	} else {
		m_poMeshActor->Release();
		m_poMeshActor = NULL;
	}

	return TRUE;
}



void CSwingingSplineActor::Reset( BOOL bAutoStart ) {
	m_poPointPath2->Reset( bAutoStart );
	m_poQuatTang2->Reset( bAutoStart );
	m_poQuatTang3->Reset( bAutoStart );
	m_poQuatComp->Reset( bAutoStart );
}


void CSwingingSplineActor::Start ( void ) {
	m_poPointPath2->Start();
	m_poQuatTang2->Start();
	m_poQuatTang3->Start();
	m_poQuatComp->Start();

	if( m_poMeshActor ) {
		m_poMeshActor->Activate();
	}
}


void CSwingingSplineActor::Stop ( void ) {
	m_poPointPath2->Pause();
	m_poQuatTang2->Pause();
	m_poQuatTang3->Pause();
	m_poQuatComp->Pause();

	if( m_poMeshActor ) {
		m_poMeshActor->Deactivate();
	}
}

void CSwingingSplineActor::GetMatrix( CFMtx43A *pMtx ) {
	_GetMatrixFromPointsAndQuat( pMtx, m_poPointPath2, m_poQuatComp );
}