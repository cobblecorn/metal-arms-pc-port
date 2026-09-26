//////////////////////////////////////////////////////////////////////////////////////
// eproj_linear.cpp - Linear projectiles.
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
// 06/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eproj_linear.h"
#include "floop.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fcoll.h"
#include "explosion.h"
#include "meshtypes.h"




void CEProj_Linear::Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact ) {
	if( bMakeEffect ) {
		if( pCollImpact ) {
			explosion_Spawn( EXPLOSION_TYPE_ROCKET, pCollImpact->ImpactPoint, 0.4f, &pCollImpact->UnitFaceNormal, FALSE, 200.f );
		} else {
			explosion_Spawn( EXPLOSION_TYPE_ROCKET, pProj->MtxToWorld()->m_vPos, 0.4f, &pProj->m_LinUnitDir_WS, FALSE, 200.f );
		}
	}
}


BOOL CEProj_Linear::Work( CEProj *pProj ) {
	CFVec3A StartCollPoint_WS, EndCollPoint_WS;
	CFMtx43A NewMtx;
	f32 fDistTraveledThisFrame;
	FCollImpact_t CollImpact;

	// Compute distance traveled this frame...
	fDistTraveledThisFrame = pProj->GetLinSpeed() * FLoop_fPreviousLoopSecs;
	pProj->m_fDistTraveled_WS += fDistTraveledThisFrame;

	// Compute new matrix...
	NewMtx.m_vRight = pProj->MtxToWorld()->m_vRight;
	NewMtx.m_vUp = pProj->MtxToWorld()->m_vUp;
	NewMtx.m_vFront = pProj->MtxToWorld()->m_vFront;

	NewMtx.m_vPos.Mul( *pProj->GetLinUnitDir_WS(), fDistTraveledThisFrame );
	NewMtx.m_vPos.Add( pProj->MtxToWorld()->m_vPos );

	// Perform collision...
	StartCollPoint_WS = pProj->MtxToWorld()->m_vPos;
	EndCollPoint_WS = NewMtx.m_vPos;

	pProj->Relocate_RotXlatFromUnitMtx_WS( &NewMtx );

	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &StartCollPoint_WS, &EndCollPoint_WS, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES ) ) {
		// Found collision...

		NewMtx.m_vPos.Mul( *pProj->GetLinUnitDir_WS(), -0.01f ).Add( CollImpact.ImpactPoint );
		pProj->Relocate_Xlat_WS( &NewMtx.m_vPos );

		pProj->Detonate( TRUE, CEProj::EVENT_HIT_GEO, &CollImpact );

		return TRUE;
	}

	return FALSE;
}


