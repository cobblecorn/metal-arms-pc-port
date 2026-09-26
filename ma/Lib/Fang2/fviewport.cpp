//////////////////////////////////////////////////////////////////////////////////////
// fviewport.cpp - Fang viewport module.
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
// 01/11/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fviewport.h"
#include "fxfm.h"


static BOOL _bModuleInitialized;
fViewport_Callback FViewport_CallbackFunc=NULL;

f32 fviewport_fCullDistOverride=0.0f;

static FViewportPlanesMask_t _TestSphere_VS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_VS, FViewportPlanesMask_t nCrossesPlanesMask );



BOOL fviewport_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	_bModuleInitialized = TRUE;

	return TRUE;
}


void fviewport_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}


void fviewport_SetCallback( fViewport_Callback pCallback ) {
	FViewport_CallbackFunc = pCallback;
}


// Computes pViewport->aUnitNormIn[nSpace][] and pViewport->aPointOnPlane[nSpace][].
void fviewport_ComputeNormals( FViewport_t *pViewport, FViewportDir_e nDir, FViewportSpace_e nSpace ) {
	u32 i, nKey, *pnViewportKey;
	CFXfm *pXfm;

	FASSERT( _bModuleInitialized );

	switch( nSpace ) {
	case FVIEWPORT_SPACE_VS:
		return;

	case FVIEWPORT_SPACE_WS:
		pXfm = FXfm_pView;
		nKey = FXfm_nViewKey;
		pnViewportKey = &pViewport->anNormCalcViewKey[nDir];
		break;

	case FVIEWPORT_SPACE_MS:
		pXfm = FXfm_pModelView;
		nKey = FXfm_nModelViewKey;
		pnViewportKey = &pViewport->anNormCalcModelViewKey[nDir];
		break;

	default:
		FASSERT_NOW;
	}

	if( *pnViewportKey != nKey ) {
		*pnViewportKey = nKey;

		for( i=0; i<6; i++ ) {
			pXfm->TransformUnitDirR( pViewport->aaaUnitNorm[nDir][nSpace][i], pViewport->aaaUnitNorm[nDir][FVIEWPORT_SPACE_VS][i] );
		}

		switch( pViewport->nType ) {
		case FVIEWPORT_TYPE_PERSPECTIVE:
			pViewport->aaPointOnPlane[nSpace][0] = pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][1] = pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][2] = pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][3] = pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][4] = pXfm->m_MtxR.m_vZ.v3*pViewport->fFarZ + pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][5] = pXfm->m_MtxR.m_vZ.v3*pViewport->fNearZ + pXfm->m_MtxR.m_vPos.v3;
			break;

		case FVIEWPORT_TYPE_ORTHO3D:
			pViewport->aaPointOnPlane[nSpace][0] = pXfm->m_MtxR.m_vPos.v3 + pXfm->m_MtxR.m_vX.v3*pViewport->HalfRes.x;
			pViewport->aaPointOnPlane[nSpace][1] = pXfm->m_MtxR.m_vPos.v3 - pXfm->m_MtxR.m_vX.v3*pViewport->HalfRes.x;
			pViewport->aaPointOnPlane[nSpace][2] = pXfm->m_MtxR.m_vPos.v3 - pXfm->m_MtxR.m_vY.v3*pViewport->HalfRes.y;
			pViewport->aaPointOnPlane[nSpace][3] = pXfm->m_MtxR.m_vPos.v3 + pXfm->m_MtxR.m_vY.v3*pViewport->HalfRes.y;
			pViewport->aaPointOnPlane[nSpace][4] = pXfm->m_MtxR.m_vZ.v3*pViewport->fFarZ + pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][5] = pXfm->m_MtxR.m_vZ.v3*pViewport->fNearZ + pXfm->m_MtxR.m_vPos.v3;
			break;

		case FVIEWPORT_TYPE_ORTHO2D:
			pViewport->aaPointOnPlane[nSpace][0] = pXfm->m_MtxR.m_vPos.v3 + pXfm->m_MtxR.m_vX.v3*pViewport->Res.x;
			pViewport->aaPointOnPlane[nSpace][1] = pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][2] = pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][3] = pXfm->m_MtxR.m_vPos.v3 + pXfm->m_MtxR.m_vY.v3*pViewport->Res.y;
			pViewport->aaPointOnPlane[nSpace][4] = pXfm->m_MtxR.m_vZ.v3*pViewport->fFarZ + pXfm->m_MtxR.m_vPos.v3;
			pViewport->aaPointOnPlane[nSpace][5] = pXfm->m_MtxR.m_vZ.v3*pViewport->fNearZ + pXfm->m_MtxR.m_vPos.v3;
			break;

		default:
			FASSERT_NOW;
		}
	}
}


// Transforms the viewport planes into sphere space and tests the sphere against them.
FViewportPlanesMask_t fviewport_TestSphere_SS( FViewport_t *pViewport, FViewportSpace_e nSphereSpace, const CFSphere *pBoundSphere, FViewportPlanesMask_t nCrossesPlanesMask ) {
	FViewportPlanesMask_t nStillCrossesPlanesMask;
	u32 i, nWalkingBit;
	f32 fDot, fNegRadius_VS;
	const CFVec3 *pPlaneUnitNorm;
	CFVec3 PlanePointToSphere;

	fviewport_ComputeNormals( pViewport, FVIEWPORT_DIR_OUT, nSphereSpace );

	nStillCrossesPlanesMask = 0;
	fNegRadius_VS = -pBoundSphere->m_fRadius;

	for( i=0, nWalkingBit=1, pPlaneUnitNorm=pViewport->aaaUnitNorm[FVIEWPORT_DIR_OUT][nSphereSpace]; i<6; i++, nWalkingBit<<=1, pPlaneUnitNorm++ ) {
		if( nCrossesPlanesMask & nWalkingBit ) {
			PlanePointToSphere = pBoundSphere->m_Pos - pViewport->aaPointOnPlane[nSphereSpace][i];

			fDot = PlanePointToSphere.Dot( *pPlaneUnitNorm );

			if( fDot > pBoundSphere->m_fRadius ) {
				// Bounding sphere is completely outside this plane...
				return -1;
			}

			if( fDot >= fNegRadius_VS ) {
				// Bounding sphere intersects plane...
				nStillCrossesPlanesMask |= nWalkingBit;
			}
		}
	}

	nCrossesPlanesMask &= nStillCrossesPlanesMask;

	return nCrossesPlanesMask;
}


// Determines which planes the specified bounding sphere intersects with.
// The bounding sphere is in model space.
//
// nCrossesPlanesMask indicates which planes any larger encompassing volume intersects.
// If there is no larger encompassing volume, set nCrossesPlanesMask to FVIEWPORT_PLANESMASK_ALL.
//
// Returns 0 (FVIEWPORT_PLANESMASK_NONE) if the bounding sphere is completely inside the frustum.
// Returns -1 if the bounding sphere is completely outside the frustum.
// Returns a revised version of nCrossesPlanesMask otherwise.
FViewportPlanesMask_t fviewport_TestSphere_MS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_MS, FViewportPlanesMask_t nCrossesPlanesMask ) {
	CFSphere BoundSphere_VS;

	FASSERT( nCrossesPlanesMask==-1 || !(nCrossesPlanesMask & ~FVIEWPORT_PLANESMASK_ALL) );

	if( (pBoundSphere_MS->m_fRadius >= 0.0f) && nCrossesPlanesMask && (nCrossesPlanesMask != -1) ) {
		// Transform sphere from model space to view space...
		FXfm_pModelView->TransformSphereF( BoundSphere_VS, *pBoundSphere_MS );
		return _TestSphere_VS( pViewport, &BoundSphere_VS, nCrossesPlanesMask );
	}

	return nCrossesPlanesMask;
}


// Same as fviewport_TestSphere_MS, but the sphere is in world space.
FViewportPlanesMask_t fviewport_TestSphere_WS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_WS, FViewportPlanesMask_t nCrossesPlanesMask ) {
	CFSphere BoundSphere_VS;

	FASSERT( nCrossesPlanesMask==-1 || !(nCrossesPlanesMask & ~FVIEWPORT_PLANESMASK_ALL) );

	if( (pBoundSphere_WS->m_fRadius >= 0.0f) && nCrossesPlanesMask && (nCrossesPlanesMask != -1) ) {
		// Transform sphere from world space to view space...
		FXfm_pView->TransformSphereF( BoundSphere_VS, *pBoundSphere_WS );
		return _TestSphere_VS( pViewport, &BoundSphere_VS, nCrossesPlanesMask );
	}

	return nCrossesPlanesMask;
}


// Same as fviewport_TestSphere_MS, but the sphere is in view space.
FViewportPlanesMask_t fviewport_TestSphere_VS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_VS, FViewportPlanesMask_t nCrossesPlanesMask ) {

	FASSERT( nCrossesPlanesMask==-1 || !(nCrossesPlanesMask & ~FVIEWPORT_PLANESMASK_ALL) );

	if( (pBoundSphere_VS->m_fRadius >= 0.0f) && nCrossesPlanesMask && (nCrossesPlanesMask != -1) ) {
		return _TestSphere_VS( pViewport, pBoundSphere_VS, nCrossesPlanesMask );
	}

	return nCrossesPlanesMask;
}


static FViewportPlanesMask_t _TestSphere_VS( const FViewport_t *pViewport, const CFSphere *pBoundSphere_VS, FViewportPlanesMask_t nCrossesPlanesMask ) {
	FViewportPlanesMask_t nStillCrossesPlanesMask;
	u32 i, nWalkingBit;
	f32 fDot, fNegRadius_VS;
	const CFVec3 *pPlaneUnitNorm, *pPlanePoint;
	CFVec3 PlanePointToSphere;

	if( (nCrossesPlanesMask == 0) || (nCrossesPlanesMask == -1) ) {
		return nCrossesPlanesMask;
	}

	fNegRadius_VS = -pBoundSphere_VS->m_fRadius;

	nStillCrossesPlanesMask = 0;

	// Test near plane...
	if( nCrossesPlanesMask & FVIEWPORT_PLANESMASK_NEAR ) {
		fDot = pViewport->fNearZ - pBoundSphere_VS->m_Pos.z;

		if( fDot > pBoundSphere_VS->m_fRadius ) {
			// Bounding sphere is completely ouside this plane...
			return -1;
		}

		if( fDot >= fNegRadius_VS ) {
			// Bounding sphere intersects plane...
			nStillCrossesPlanesMask |= FVIEWPORT_PLANESMASK_NEAR;
		}
	}

	// Test far plane...
	if( nCrossesPlanesMask & FVIEWPORT_PLANESMASK_FAR ) {
		if (fviewport_fCullDistOverride)
		{
			fDot = pBoundSphere_VS->m_Pos.z - fviewport_fCullDistOverride;
		}
		else
		{
			fDot = pBoundSphere_VS->m_Pos.z - pViewport->fFarZ;
		}

		if( fDot > pBoundSphere_VS->m_fRadius ) {
			// Bounding sphere is completely ouside this plane...
			return -1;
		}

		if( fDot >= fNegRadius_VS ) {
			// Bounding sphere intersects plane...
			nStillCrossesPlanesMask |= FVIEWPORT_PLANESMASK_FAR;
		}
	}

	// Test side planes...
	pPlaneUnitNorm = pViewport->aaaUnitNorm[FVIEWPORT_DIR_OUT][FVIEWPORT_SPACE_VS];
	pPlanePoint = pViewport->aaPointOnPlane[FVIEWPORT_SPACE_VS];
	for( i=0, nWalkingBit=1; i<4; i++, nWalkingBit<<=1, pPlaneUnitNorm++, pPlanePoint++ ) {
		if( nCrossesPlanesMask & nWalkingBit ) {
			PlanePointToSphere = pBoundSphere_VS->m_Pos - *pPlanePoint;

			fDot = PlanePointToSphere.Dot( *pPlaneUnitNorm );

			if( fDot > pBoundSphere_VS->m_fRadius ) {
				// Bounding sphere is completely outside this plane...
				return -1;
			}

			if( fDot >= fNegRadius_VS ) {
				// Bounding sphere intersects plane...
				nStillCrossesPlanesMask |= nWalkingBit;
			}
		}
	}

	nCrossesPlanesMask &= nStillCrossesPlanesMask;

	return nCrossesPlanesMask;
}


