//////////////////////////////////////////////////////////////////////////////////////
// fxfm.cpp - Fang Transformation module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/15/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fxfm.h"
#include "fres.h"


CFXfm *FXfm_pView;				// View xfm
CFXfm *FXfm_pModel;				// Concatenated model xfm
CFXfm *FXfm_pModelView;			// Concatenated model*view xfm
CFMtx43A *FXfm_pMirrorMtx;		// Mirror Matrix, if applicable

u32 FXfm_nViewKey;				// Incremented each time the view xfm is modified
u32 FXfm_nModelKey;				// Incremented each time the model xfm is modified
u32 FXfm_nModelViewKey;			// Incremented each time either the model or view xfm is modified

CFXfm FXfm_Identity;			// Identity xfm

#if FANG_PLATFORM_GC
extern void fgcxfm_SetViewGCMatrix( void );
#endif







static BOOL _bModuleInitialized;

static CFXfm _ViewXfm;				// The one and only view xfm

static CFXfm *_pModelStack;			// The model xfm stack
static u32 _nMaxModelStackXfms;		// The maximum number of xfm's that the model stack can hold

static CFXfm *_pModelViewStack;		// The model-view xfm stack
static u32 _nMaxModelViewStackXfms;	// The maximum number of xfm's that the model-view stack can hold




BOOL fxfm_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );
	
	FXfm_pMirrorMtx = NULL;

	_nMaxModelStackXfms = Fang_ConfigDefs.nXfm_MaxModelStackXfms;
	if( _nMaxModelStackXfms < 1 ) {
		DEVPRINTF( "fxfm_ModuleStartup(): Fang_ConfigDefs.nXfm_MaxModelStackXfms must be at least 1.\n" );
		_nMaxModelStackXfms = 1;
	}
	_nMaxModelStackXfms++;

	_pModelStack = (CFXfm *)fres_AlignedAlloc( _nMaxModelStackXfms * sizeof(CFXfm), CFXfm::CLASS_BYTE_ALIGNMENT );
	if( _pModelStack == NULL ) {
		DEVPRINTF( "fxfm_ModuleStartup(): Could not allocate %u bytes for model Xfm stack.\n", _nMaxModelStackXfms * sizeof(CFXfm) );
		return FALSE;
	}

	_nMaxModelViewStackXfms = _nMaxModelStackXfms;
	_pModelViewStack = (CFXfm *)fres_AlignedAlloc( _nMaxModelViewStackXfms * sizeof(CFXfm), CFXfm::CLASS_BYTE_ALIGNMENT );
	if( _pModelViewStack == NULL ) {
		DEVPRINTF( "fxfm_ModuleStartup(): Could not allocate %u bytes for model-view Xfm stack.\n", _nMaxModelViewStackXfms * sizeof(CFXfm) );
		return FALSE;
	}

	FXfm_pView = &_ViewXfm;

	FXfm_nViewKey = 1;
	FXfm_nModelKey = 1;
	FXfm_nModelViewKey = 1;

	_bModuleInitialized = TRUE;

	FXfm_Identity.Identity();
	CFXfm::InitStack();

	return TRUE;
}

void fxfm_ModuleShutdown( void ) {
	FASSERT( _bModuleInitialized );

	_bModuleInitialized = FALSE;
}

void CFXfm::InitStack( const CFXfm *pXfmView ) {
	FASSERT( _bModuleInitialized );

	_pModelStack[0].Identity();
	FXfm_pModel = &_pModelStack[0];
 
	if( pXfmView ) {
		_ViewXfm = *pXfmView;
		_pModelViewStack[0] = *pXfmView;
	} else {
		_ViewXfm.Identity();
		_pModelViewStack[0].Identity();
	}

	FXfm_pModelView = &_pModelViewStack[0];
	
#if FANG_PLATFORM_GC
	fgcxfm_SetViewGCMatrix();
#endif

	FXfm_nViewKey++;
	FXfm_nModelKey++;
	FXfm_nModelViewKey++;
}

void CFXfm::PushModel( void ) const {
	FASSERT( _bModuleInitialized );
	FASSERT( FXfm_pModel < &_pModelStack[_nMaxModelStackXfms] );
	FASSERT( FXfm_pModelView < &_pModelViewStack[_nMaxModelViewStackXfms] );

	if( FXfm_pModel++ > _pModelStack ) {
		// Not the first element pushed...
		FXfm_pModel->ReceiveProductOf( *(FXfm_pModel-1), *this );
	} else {
		// This is the first element pushed...
		*FXfm_pModel = *this;
	}

	FXfm_pModelView++;
	FXfm_pModelView->ReceiveProductOf( _ViewXfm, *FXfm_pModel );

	FXfm_nModelKey++;
	FXfm_nModelViewKey++;
}

void CFXfm::PopModel( u32 nPopCount ) {
	FASSERT( _bModuleInitialized );
	FASSERT( (FXfm_pModel - nPopCount) >= _pModelStack );
	FASSERT( (FXfm_pModelView - nPopCount) >= _pModelViewStack );

	if( nPopCount ) {
		FXfm_pModel -= nPopCount;
		FXfm_pModelView -= nPopCount;

		FXfm_nModelKey++;
		FXfm_nModelViewKey++;
	}
}

CFXfm &CFXfm::BuildFromMtx( const CFMtx33 &rMtx, BOOL bComputeScale ) {
	if( !bComputeScale ) {
		// No scale...

		m_fScaleF = 1.0f;
		m_fScaleR = 1.0f;

		m_MtxF.aa[0][0] = m_MtxR.aa[0][0] = rMtx.aa[0][0];
		m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = rMtx.aa[0][1];
		m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = rMtx.aa[0][2];

		m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = rMtx.aa[1][0];
		m_MtxF.aa[1][1] = m_MtxR.aa[1][1] = rMtx.aa[1][1];
		m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = rMtx.aa[1][2];

		m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = rMtx.aa[2][0];
		m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = rMtx.aa[2][1];
		m_MtxF.aa[2][2] = m_MtxR.aa[2][2] = rMtx.aa[2][2];
	} else {
		// Compute scale...

		f32 fOOScale2;

		m_fScaleF = fmath_Sqrt( rMtx.aa[0][0]*rMtx.aa[0][0] + rMtx.aa[0][1]*rMtx.aa[0][1] + rMtx.aa[0][2]*rMtx.aa[0][2] );
		FASSERT( m_fScaleF != 0.0f );
		m_fScaleR = fmath_Inv( m_fScaleF );

		fOOScale2 = m_fScaleR * m_fScaleR;

		m_MtxF.Set( rMtx );

		m_MtxR.aa[0][0] = rMtx.aa[0][0] * fOOScale2;
		m_MtxR.aa[1][0] = rMtx.aa[0][1] * fOOScale2;	
		m_MtxR.aa[2][0] = rMtx.aa[0][2] * fOOScale2;	
			
		m_MtxR.aa[0][1] = rMtx.aa[1][0] * fOOScale2;	
		m_MtxR.aa[1][1] = rMtx.aa[1][1] * fOOScale2;	
		m_MtxR.aa[2][1] = rMtx.aa[1][2] * fOOScale2;	
			
		m_MtxR.aa[0][2] = rMtx.aa[2][0] * fOOScale2;	
		m_MtxR.aa[1][2] = rMtx.aa[2][1] * fOOScale2;	
		m_MtxR.aa[2][2] = rMtx.aa[2][2] * fOOScale2;	
	}

	m_MtxF.aa[3][0] = m_MtxR.aa[3][0] = 0.0f;
	m_MtxF.aa[3][1] = m_MtxR.aa[3][1] = 0.0f;
	m_MtxF.aa[3][2] = m_MtxR.aa[3][2] = 0.0f;

	return *this;
}

CFXfm &CFXfm::BuildFromMtx( const CFMtx43 &rMtx, BOOL bComputeScale ) {
	float x, y, z, nx;

	if( !bComputeScale ) {
		// No scale...

		m_fScaleF = 1.0f;
		m_fScaleR = 1.0f;

		m_MtxF.aa[0][0] = m_MtxR.aa[0][0] = rMtx.aa[0][0];
		m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = rMtx.aa[0][1];
		m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = rMtx.aa[0][2];

		m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = rMtx.aa[1][0];
		m_MtxF.aa[1][1] = m_MtxR.aa[1][1] = rMtx.aa[1][1];
		m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = rMtx.aa[1][2];

		m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = rMtx.aa[2][0];
		m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = rMtx.aa[2][1];
		m_MtxF.aa[2][2] = m_MtxR.aa[2][2] = rMtx.aa[2][2];
	} else {
		// Compute scale...

		f32 fOOScale2;

		m_fScaleF = fmath_Sqrt( rMtx.aa[0][0]*rMtx.aa[0][0] + rMtx.aa[0][1]*rMtx.aa[0][1] + rMtx.aa[0][2]*rMtx.aa[0][2] );
		FASSERT( m_fScaleF != 0.0f );
		m_fScaleR = fmath_Inv( m_fScaleF );

		fOOScale2 = m_fScaleR * m_fScaleR;

		m_MtxF.Set( rMtx.m33 );

		m_MtxR.aa[0][0] = rMtx.aa[0][0] * fOOScale2;
		m_MtxR.aa[1][0] = rMtx.aa[0][1] * fOOScale2;	
		m_MtxR.aa[2][0] = rMtx.aa[0][2] * fOOScale2;	

		m_MtxR.aa[0][1] = rMtx.aa[1][0] * fOOScale2;	
		m_MtxR.aa[1][1] = rMtx.aa[1][1] * fOOScale2;	
		m_MtxR.aa[2][1] = rMtx.aa[1][2] * fOOScale2;	

		m_MtxR.aa[0][2] = rMtx.aa[2][0] * fOOScale2;	
		m_MtxR.aa[1][2] = rMtx.aa[2][1] * fOOScale2;	
		m_MtxR.aa[2][2] = rMtx.aa[2][2] * fOOScale2;	
	}

	x = rMtx.m_vPos.x;
	y = rMtx.m_vPos.y;
	z = rMtx.m_vPos.z;
	nx = -x;

	m_MtxF.aa[3][0] = x;
	m_MtxF.aa[3][1] = y;
	m_MtxF.aa[3][2] = z;
	m_MtxR.aa[3][0] = nx*m_MtxR.aa[0][0] - y*m_MtxR.aa[1][0] - z*m_MtxR.aa[2][0];
	m_MtxR.aa[3][1] = nx*m_MtxR.aa[0][1] - y*m_MtxR.aa[1][1] - z*m_MtxR.aa[2][1];
	m_MtxR.aa[3][2] = nx*m_MtxR.aa[0][2] - y*m_MtxR.aa[1][2] - z*m_MtxR.aa[2][2];

	return *this;
}

CFXfm &CFXfm::BuildFromMtx( const CFMtx43A &rMtx, BOOL bComputeScale ) {
	m_MtxF = rMtx;

	if( !bComputeScale ) {
		// No scale...

		m_fScaleF = 1.0f;
		m_fScaleR = 1.0f;
	} else {
		// Compute scale...

		m_fScaleF = rMtx.m_vRight.Mag();
		FASSERT( m_fScaleF != 0.0f );
		m_fScaleR = fmath_Inv( m_fScaleF );
	}

	m_MtxR.ReceiveAffineInverse_KnowOOScale2( rMtx, m_fScaleR * m_fScaleR );

	return *this;
}

// Important: rMtx must be a matrix with unit scale.
CFXfm &CFXfm::BuildFromMtx( const CFMtx33 &rMtx, f32 fScale ) {
	f32 fOOScale2;

	FASSERT( fScale != 0.0f );

	m_fScaleF = fScale;
	m_fScaleR = fmath_Inv( m_fScaleF );

	fOOScale2 = m_fScaleR * m_fScaleR;

	m_MtxF.m_vRight.v3 = rMtx.m_vRight * m_fScaleF;
	m_MtxF.m_vUp.v3 = rMtx.m_vUp * m_fScaleF;
	m_MtxF.m_vFront.v3 = rMtx.m_vFront * m_fScaleF;

	m_MtxR.aa[0][0] = rMtx.aa[0][0] * fOOScale2;
	m_MtxR.aa[1][0] = rMtx.aa[0][1] * fOOScale2;	
	m_MtxR.aa[2][0] = rMtx.aa[0][2] * fOOScale2;	
		
	m_MtxR.aa[0][1] = rMtx.aa[1][0] * fOOScale2;	
	m_MtxR.aa[1][1] = rMtx.aa[1][1] * fOOScale2;	
	m_MtxR.aa[2][1] = rMtx.aa[1][2] * fOOScale2;	
		
	m_MtxR.aa[0][2] = rMtx.aa[2][0] * fOOScale2;	
	m_MtxR.aa[1][2] = rMtx.aa[2][1] * fOOScale2;	
	m_MtxR.aa[2][2] = rMtx.aa[2][2] * fOOScale2;	

	m_MtxF.aa[3][0] = m_MtxR.aa[3][0] = 0.0f;
	m_MtxF.aa[3][1] = m_MtxR.aa[3][1] = 0.0f;
	m_MtxF.aa[3][2] = m_MtxR.aa[3][2] = 0.0f;

	return *this;
}

// Important: rMtx must be a matrix with unit scale.
CFXfm &CFXfm::BuildFromMtx( const CFMtx43 &rMtx, f32 fScale ) {
	f32 fNegX, fOOScale2;

	FASSERT( fScale != 0.0f );

	m_fScaleF = fScale;
	m_fScaleR = fmath_Inv( m_fScaleF );

	fOOScale2 = m_fScaleR * m_fScaleR;

	// Set forward rotation components...
	m_MtxF.m_vRight.v3 = rMtx.m_vRight * m_fScaleF;
	m_MtxF.m_vUp.v3 = rMtx.m_vUp * m_fScaleF;
	m_MtxF.m_vFront.v3 = rMtx.m_vFront * m_fScaleF;

	// Set reverse rotation components...
	m_MtxR.aa[0][0] = rMtx.aa[0][0] * fOOScale2;
	m_MtxR.aa[1][0] = rMtx.aa[0][1] * fOOScale2;
	m_MtxR.aa[2][0] = rMtx.aa[0][2] * fOOScale2;

	m_MtxR.aa[0][1] = rMtx.aa[1][0] * fOOScale2;
	m_MtxR.aa[1][1] = rMtx.aa[1][1] * fOOScale2;
	m_MtxR.aa[2][1] = rMtx.aa[1][2] * fOOScale2;

	m_MtxR.aa[0][2] = rMtx.aa[2][0] * fOOScale2;
	m_MtxR.aa[1][2] = rMtx.aa[2][1] * fOOScale2;
	m_MtxR.aa[2][2] = rMtx.aa[2][2] * fOOScale2;

	// Set translation component...
	fNegX = -rMtx.m_vPos.x;

	m_MtxF.aa[3][0] = rMtx.m_vPos.x;
	m_MtxF.aa[3][1] = rMtx.m_vPos.y;
	m_MtxF.aa[3][2] = rMtx.m_vPos.z;
	m_MtxR.aa[3][0] = fNegX*m_MtxR.aa[0][0] - rMtx.m_vPos.y*m_MtxR.aa[1][0] - rMtx.m_vPos.z*m_MtxR.aa[2][0];
	m_MtxR.aa[3][1] = fNegX*m_MtxR.aa[0][1] - rMtx.m_vPos.y*m_MtxR.aa[1][1] - rMtx.m_vPos.z*m_MtxR.aa[2][1];
	m_MtxR.aa[3][2] = fNegX*m_MtxR.aa[0][2] - rMtx.m_vPos.y*m_MtxR.aa[1][2] - rMtx.m_vPos.z*m_MtxR.aa[2][2];

	return *this;
}

// Important: rMtx must be a matrix with unit scale.
CFXfm &CFXfm::BuildFromMtx( const CFMtx43A &rMtx, f32 fScale ) {
	CFVec3A ScaleVec;

	FASSERT( m_fScaleF != 0.0f );

	m_fScaleF = fScale;
	m_fScaleR = fmath_Inv( fScale );

	ScaleVec.Set( m_fScaleF );
	m_MtxF.m_vRight.Mul( rMtx.m_vRight, ScaleVec );
	m_MtxF.m_vUp.Mul( rMtx.m_vUp, ScaleVec );
	m_MtxF.m_vFront.Mul( rMtx.m_vFront, ScaleVec );
	m_MtxF.m_vPos = rMtx.m_vPos;

	m_MtxR.ReceiveAffineInverse_KnowOOScale2( m_MtxF, m_fScaleR * m_fScaleR );

	return *this;
}

CFXfm &CFXfm::BuildFromInvMtx( const CFMtx43A &rMtx, BOOL bComputeScale ) {
	m_MtxR = rMtx;

	if( !bComputeScale ) {
		// No scale...

		m_fScaleF = 1.0f;
		m_fScaleR = 1.0f;
	} else {
		// Compute scale...

		m_fScaleR= rMtx.m_vRight.Mag();
		FASSERT( m_fScaleR != 0.0f );
		m_fScaleF = fmath_Inv( m_fScaleR );
	}

	m_MtxF.ReceiveAffineInverse_KnowOOScale2( rMtx, m_fScaleR * m_fScaleR );

	return *this;
}


CFXfm &CFXfm::BuildFromQuat( const CFQuatA &rQuat ) {
	rQuat.BuildMtx( m_MtxF );

	m_MtxR.aa[0][0] = m_MtxF.aa[0][0];
	m_MtxR.aa[1][0] = m_MtxF.aa[0][1];
	m_MtxR.aa[2][0] = m_MtxF.aa[0][2];

	m_MtxR.aa[0][1] = m_MtxF.aa[1][0];
	m_MtxR.aa[1][1] = m_MtxF.aa[1][1];
	m_MtxR.aa[2][1] = m_MtxF.aa[1][2];

	m_MtxR.aa[0][2] = m_MtxF.aa[2][0];
	m_MtxR.aa[1][2] = m_MtxF.aa[2][1];
	m_MtxR.aa[2][2] = m_MtxF.aa[2][2];

	m_MtxF.aa[3][0] = m_MtxR.aa[3][0] = 0.0f;
	m_MtxF.aa[3][1] = m_MtxR.aa[3][1] = 0.0f;
	m_MtxF.aa[3][2] = m_MtxR.aa[3][2] = 0.0f;

	m_fScaleF = 1.0f;
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildFromQuat( const CFQuatA &rQuat, f32 fScale ) {
	f32 fOOScale2;

	FASSERT( fScale != 0.0f );

	rQuat.BuildMtx( m_MtxF, fScale );

	m_fScaleR = fmath_Inv( fScale );
	m_fScaleF = fScale;

	fOOScale2 = m_fScaleR * m_fScaleR;

	m_MtxR.aa[0][0] = m_MtxF.aa[0][0] * fOOScale2;
	m_MtxR.aa[1][0] = m_MtxF.aa[0][1] * fOOScale2;
	m_MtxR.aa[2][0] = m_MtxF.aa[0][2] * fOOScale2;

	m_MtxR.aa[0][1] = m_MtxF.aa[1][0] * fOOScale2;
	m_MtxR.aa[1][1] = m_MtxF.aa[1][1] * fOOScale2;
	m_MtxR.aa[2][1] = m_MtxF.aa[1][2] * fOOScale2;

	m_MtxR.aa[0][2] = m_MtxF.aa[2][0] * fOOScale2;
	m_MtxR.aa[1][2] = m_MtxF.aa[2][1] * fOOScale2;
	m_MtxR.aa[2][2] = m_MtxF.aa[2][2] * fOOScale2;

	m_MtxF.m_vPos.Zero();
	m_MtxR.m_vPos.Zero();

	return *this;
}

CFXfm &CFXfm::BuildFromQuat( const CFQuatA &rQuat, const CFVec3A &rPos ) {
	f32 fNegX;

	rQuat.BuildMtx( m_MtxF );

	m_MtxR.aa[0][0] = m_MtxF.aa[0][0];
	m_MtxR.aa[1][0] = m_MtxF.aa[0][1];
	m_MtxR.aa[2][0] = m_MtxF.aa[0][2];

	m_MtxR.aa[0][1] = m_MtxF.aa[1][0];
	m_MtxR.aa[1][1] = m_MtxF.aa[1][1];
	m_MtxR.aa[2][1] = m_MtxF.aa[1][2];

	m_MtxR.aa[0][2] = m_MtxF.aa[2][0];
	m_MtxR.aa[1][2] = m_MtxF.aa[2][1];
	m_MtxR.aa[2][2] = m_MtxF.aa[2][2];

	fNegX = -rPos.x;
	m_MtxF.m_vPos = rPos;
	m_MtxR.aa[3][0] = fNegX*m_MtxR.aa[0][0] - rPos.y*m_MtxR.aa[1][0] - rPos.z*m_MtxR.aa[2][0];
	m_MtxR.aa[3][1] = fNegX*m_MtxR.aa[0][1] - rPos.y*m_MtxR.aa[1][1] - rPos.z*m_MtxR.aa[2][1];
	m_MtxR.aa[3][2] = fNegX*m_MtxR.aa[0][2] - rPos.y*m_MtxR.aa[1][2] - rPos.z*m_MtxR.aa[2][2];

	m_fScaleF = 1.0f;
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildFromQuat( const CFQuatA &rQuat, const CFVec3A &rPos, f32 fScale ) {
	f32 fOOScale2, fNegX;

	FASSERT( fScale != 0.0f );

	rQuat.BuildMtx( m_MtxF, fScale );

	m_fScaleR = fmath_Inv( fScale );
	m_fScaleF = fScale;

	fOOScale2 = m_fScaleR * m_fScaleR;

	m_MtxR.aa[0][0] = m_MtxF.aa[0][0] * fOOScale2;
	m_MtxR.aa[1][0] = m_MtxF.aa[0][1] * fOOScale2;
	m_MtxR.aa[2][0] = m_MtxF.aa[0][2] * fOOScale2;

	m_MtxR.aa[0][1] = m_MtxF.aa[1][0] * fOOScale2;
	m_MtxR.aa[1][1] = m_MtxF.aa[1][1] * fOOScale2;
	m_MtxR.aa[2][1] = m_MtxF.aa[1][2] * fOOScale2;

	m_MtxR.aa[0][2] = m_MtxF.aa[2][0] * fOOScale2;
	m_MtxR.aa[1][2] = m_MtxF.aa[2][1] * fOOScale2;
	m_MtxR.aa[2][2] = m_MtxF.aa[2][2] * fOOScale2;

	fNegX = -rPos.x;
	m_MtxF.m_vPos = rPos;
	m_MtxR.aa[3][0] = fNegX*m_MtxR.aa[0][0] - rPos.y*m_MtxR.aa[1][0] - rPos.z*m_MtxR.aa[2][0];
	m_MtxR.aa[3][1] = fNegX*m_MtxR.aa[0][1] - rPos.y*m_MtxR.aa[1][1] - rPos.z*m_MtxR.aa[2][1];
	m_MtxR.aa[3][2] = fNegX*m_MtxR.aa[0][2] - rPos.y*m_MtxR.aa[1][2] - rPos.z*m_MtxR.aa[2][2];

	return *this;
}

CFXfm &CFXfm::BuildFromInvQuat( const CFQuatA &rQuat ) {
	rQuat.BuildMtx( m_MtxR );

	m_MtxF.aa[0][0] = m_MtxR.aa[0][0];
	m_MtxF.aa[1][0] = m_MtxR.aa[0][1];
	m_MtxF.aa[2][0] = m_MtxR.aa[0][2];

	m_MtxF.aa[0][1] = m_MtxR.aa[1][0];
	m_MtxF.aa[1][1] = m_MtxR.aa[1][1];
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2];

	m_MtxF.aa[0][2] = m_MtxR.aa[2][0];
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1];
	m_MtxF.aa[2][2] = m_MtxR.aa[2][2];

	m_MtxF.m_vPos.Zero();
	m_MtxR.m_vPos.Zero();

	m_fScaleF = 1.0f;
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildFromInvQuat( const CFQuatA &rQuat, const CFVec3A &rPos ) {
	f32 fNegX;

	rQuat.BuildMtx( m_MtxR );

	m_MtxF.aa[0][0] = m_MtxR.aa[0][0];
	m_MtxF.aa[1][0] = m_MtxR.aa[0][1];
	m_MtxF.aa[2][0] = m_MtxR.aa[0][2];

	m_MtxF.aa[0][1] = m_MtxR.aa[1][0];
	m_MtxF.aa[1][1] = m_MtxR.aa[1][1];
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2];

	m_MtxF.aa[0][2] = m_MtxR.aa[2][0];
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1];
	m_MtxF.aa[2][2] = m_MtxR.aa[2][2];

	fNegX = -rPos.x;
	m_MtxR.m_vPos = rPos;
	m_MtxF.aa[3][0] = fNegX*m_MtxF.aa[0][0] - rPos.y*m_MtxF.aa[1][0] - rPos.z*m_MtxF.aa[2][0];
	m_MtxF.aa[3][1] = fNegX*m_MtxF.aa[0][1] - rPos.y*m_MtxF.aa[1][1] - rPos.z*m_MtxF.aa[2][1];
	m_MtxF.aa[3][2] = fNegX*m_MtxF.aa[0][2] - rPos.y*m_MtxF.aa[1][2] - rPos.z*m_MtxF.aa[2][2];

	m_fScaleF = 1.0f;
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildTranslation( const CFVec3 &rVec ) {
	m_MtxF.Identity33();
	m_MtxF.m_vPos.v3 = rVec;
	m_fScaleF = 1.0f;

	m_MtxR.Identity33();
	m_MtxR.m_vPos.v3 = -rVec;
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildTranslation( const CFVec3A &rVec ) {
	m_MtxF.Identity33();
	m_MtxF.m_vPos = rVec;
	m_fScaleF = 1.0f;

	m_MtxR.Identity33();
	m_MtxR.m_vPos.ReceiveNegative( rVec );
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildTranslation( f32 fX, f32 fY, f32 fZ ) {
	m_MtxF.Identity33();
	m_MtxF.m_vPos.Set( fX, fY, fZ );
	m_fScaleF = 1.0f;

	m_MtxR.Identity33();
	m_MtxR.m_vPos.Set( fX, fY, fZ );
	m_MtxR.m_vPos.Negate();
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildScale( f32 fScale ) {
	Identity();

	m_fScaleF = fScale;
	m_fScaleR = fmath_Inv( fScale );

	m_MtxF.aa[0][0] = m_MtxF.aa[1][1] = m_MtxF.aa[2][2] = m_fScaleF;
	m_MtxR.aa[0][0] = m_MtxR.aa[1][1] = m_MtxR.aa[2][2] = m_fScaleR;

	return *this;
}

// rRotMtx43A must have unit scale.
CFXfm &CFXfm::BuildRotation( CFMtx43A &rRotMtx43A ) {
	m_MtxF = rRotMtx43A;
	m_MtxR.ReceiveAffineInverse( rRotMtx43A, FALSE );
	m_MtxF.m_vPos.Zero();
	m_MtxR.m_vPos.Zero();
	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationX( f32 fRadiansX ) {
	f32 fSin, fCos;

	Identity();

	fmath_SinCos( fRadiansX, &fSin, &fCos );

	m_MtxF.aa[1][1] = m_MtxF.aa[2][2] = m_MtxR.aa[1][1] = m_MtxR.aa[2][2] = fCos;
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = fSin;
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = -fSin;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationX( f32 fRadiansX, const CFVec3 &rXlat ) {
	f32 fSin, fCos;

	_Identity3x3();

	fmath_SinCos( fRadiansX, &fSin, &fCos );

	m_MtxF.aa[1][1] = m_MtxF.aa[2][2] = m_MtxR.aa[1][1] = m_MtxR.aa[2][2] = fCos;
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = fSin;
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = -fSin;

	m_MtxF.aa[3][0] = rXlat.x;
	m_MtxF.aa[3][1] = rXlat.y;
	m_MtxF.aa[3][2] = rXlat.z;
	m_MtxR.aa[3][0] = -rXlat.x;
	m_MtxR.aa[3][1] = -fSin*rXlat.z - fCos*rXlat.y;
	m_MtxR.aa[3][2] = fSin*rXlat.y - fCos*rXlat.z;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationX( f32 fRadiansX, f32 fX, f32 fY, f32 fZ ) {
	f32 fSin, fCos;

	_Identity3x3();

	fmath_SinCos( fRadiansX, &fSin, &fCos );

	m_MtxF.aa[1][1] = m_MtxF.aa[2][2] = m_MtxR.aa[1][1] = m_MtxR.aa[2][2] = fCos;
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = fSin;
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = -fSin;

	m_MtxF.aa[3][0] = fX;
	m_MtxF.aa[3][1] = fY;
	m_MtxF.aa[3][2] = fZ;
	m_MtxR.aa[3][0] = -fX;
	m_MtxR.aa[3][1] = -fSin*fZ - fCos*fY;
	m_MtxR.aa[3][2] = fSin*fY - fCos*fZ;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationY( f32 fRadiansY ) {
	f32 fSin, fCos;

	Identity();

	fmath_SinCos( fRadiansY, &fSin, &fCos );

	m_MtxF.aa[0][0] = m_MtxF.aa[2][2] = m_MtxR.aa[0][0] = m_MtxR.aa[2][2] = fCos;
	m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = -fSin;
	m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = fSin;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationY( f32 fRadiansY, const CFVec3 &rXlat ) {
	f32 fSin, fCos;

	_Identity3x3();

	fmath_SinCos( fRadiansY, &fSin, &fCos );

	m_MtxF.aa[0][0] = m_MtxF.aa[2][2] = m_MtxR.aa[0][0] = m_MtxR.aa[2][2] = fCos;
	m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = -fSin;
	m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = fSin;

	m_MtxF.aa[3][0] = rXlat.x;
	m_MtxF.aa[3][1] = rXlat.y;
	m_MtxF.aa[3][2] = rXlat.z;
	m_MtxR.aa[3][0] = fSin*rXlat.z - fCos*rXlat.x;
	m_MtxR.aa[3][1] = -rXlat.y;
	m_MtxR.aa[3][2] = -fSin*rXlat.x - fCos*rXlat.z;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationY( f32 fRadiansY, f32 fX, f32 fY, f32 fZ ) {
	f32 fSin, fCos;

	_Identity3x3();

	fmath_SinCos( fRadiansY, &fSin, &fCos );

	m_MtxF.aa[0][0] = m_MtxF.aa[2][2] = m_MtxR.aa[0][0] = m_MtxR.aa[2][2] = fCos;
	m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = -fSin;
	m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = fSin;

	m_MtxF.aa[3][0] = fX;
	m_MtxF.aa[3][1] = fY;
	m_MtxF.aa[3][2] = fZ;
	m_MtxR.aa[3][0] = fSin*fZ - fCos*fX;
	m_MtxR.aa[3][1] = -fY;
	m_MtxR.aa[3][2] = -fSin*fX - fCos*fZ;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationZ( f32 fRadiansZ ) {
	f32 fSin, fCos;

	Identity();

	fmath_SinCos( fRadiansZ, &fSin, &fCos );

	m_MtxF.aa[0][0] = m_MtxF.aa[1][1] = m_MtxR.aa[0][0] = m_MtxR.aa[1][1] = fCos;
	m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = fSin;
	m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = -fSin;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationZ( f32 fRadiansZ, const CFVec3 &rXlat ) {
	f32 fSin, fCos;

	_Identity3x3();

	fmath_SinCos( fRadiansZ, &fSin, &fCos );

	m_MtxF.aa[0][0] = m_MtxF.aa[1][1] = m_MtxR.aa[0][0] = m_MtxR.aa[1][1] = fCos;
	m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = fSin;
	m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = -fSin;

	m_MtxF.aa[3][0] = rXlat.x;
	m_MtxF.aa[3][1] = rXlat.y;
	m_MtxF.aa[3][2] = rXlat.z;
	m_MtxR.aa[3][0] = -fSin*rXlat.y - fCos*rXlat.x;
	m_MtxR.aa[3][1] = fSin*rXlat.x - fCos*rXlat.y;
	m_MtxR.aa[3][2] = -rXlat.z;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotationZ( f32 fRadiansZ, f32 fX, f32 fY, f32 fZ ) {
	f32 fSin, fCos;

	_Identity3x3();

	fmath_SinCos( fRadiansZ, &fSin, &fCos );

	m_MtxF.aa[0][0] = m_MtxF.aa[1][1] = m_MtxR.aa[0][0] = m_MtxR.aa[1][1] = fCos;
	m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = fSin;
	m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = -fSin;

	m_MtxF.aa[3][0] = fX;
	m_MtxF.aa[3][1] = fY;
	m_MtxF.aa[3][2] = fZ;
	m_MtxR.aa[3][0] = -fSin*fY - fCos*fX;
	m_MtxR.aa[3][1] = fSin*fX - fCos*fY;
	m_MtxR.aa[3][2] = -fZ;

	m_fScaleF = m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildRotYXZ_XlatFromPoint( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ, f32 fX, f32 fY, f32 fZ ) {
	f32 fSinX, fSinY, fSinZ;
	f32 fCosX, fCosY, fCosZ;
	f32 fSinXSinZ, fSinXCosZ;
	f32 fNegX;

	fNegX = -fX;

	fmath_SinCos( fRadiansX, &fSinX, &fCosX );
	fmath_SinCos( fRadiansY, &fSinY, &fCosY );
	fmath_SinCos( fRadiansZ, &fSinZ, &fCosZ );

	fSinXSinZ = fSinX*fSinZ;
	fSinXCosZ = fSinX*fCosZ;

	m_MtxF.aa[0][0] = m_MtxR.aa[0][0] = fCosY*fCosZ + fSinXSinZ*fSinY;
	m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = fCosX*fSinZ;
	m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = fSinXSinZ*fCosY - fSinY*fCosZ;

	m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = fSinXCosZ*fSinY - fCosY*fSinZ;
	m_MtxF.aa[1][1] = m_MtxR.aa[1][1] = fCosX*fCosZ;
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = fSinXCosZ*fCosY + fSinY*fSinZ;

	m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = fSinY*fCosX;
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = -fSinX;
	m_MtxF.aa[2][2] = m_MtxR.aa[2][2] = fCosY*fCosX;

	m_MtxF.aa[3][0] = fX;
	m_MtxF.aa[3][1] = fY;
	m_MtxF.aa[3][2] = fZ;
	m_MtxR.aa[3][0] = fNegX*m_MtxR.aa[0][0] - fY*m_MtxR.aa[1][0] - fZ*m_MtxR.aa[2][0];
	m_MtxR.aa[3][1] = fNegX*m_MtxR.aa[0][1] - fY*m_MtxR.aa[1][1] - fZ*m_MtxR.aa[2][1];
	m_MtxR.aa[3][2] = fNegX*m_MtxR.aa[0][2] - fY*m_MtxR.aa[1][2] - fZ*m_MtxR.aa[2][2];

	m_fScaleF = 1.0f;
	m_fScaleR = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildCameraRotAndInitStack( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ, f32 fX, f32 fY, f32 fZ ) {
	BuildInvRotYXZ_XlatFromPoint( fRadiansY, fRadiansX, fRadiansZ, fX, fY, fZ );
	InitStackWithView();

	return *this;
}

CFXfm &CFXfm::BuildInvRotYXZ_XlatFromPoint( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ, f32 fX, f32 fY, f32 fZ ) {
	f32 fSinX, fSinY, fSinZ;
	f32 fCosX, fCosY, fCosZ;
	f32 fSinXSinZ, fSinXCosZ;
	f32 fNegX;

	fNegX = -fX;

	fmath_SinCos( fRadiansX, &fSinX, &fCosX );
	fmath_SinCos( fRadiansY, &fSinY, &fCosY );
	fmath_SinCos( fRadiansZ, &fSinZ, &fCosZ );

	fSinXSinZ = fSinX*fSinZ;
	fSinXCosZ = fSinX*fCosZ;

	m_MtxR.aa[0][0] = m_MtxF.aa[0][0] = fCosY*fCosZ + fSinXSinZ*fSinY;
	m_MtxR.aa[0][1] = m_MtxF.aa[1][0] = fCosX*fSinZ;
	m_MtxR.aa[0][2] = m_MtxF.aa[2][0] = fSinXSinZ*fCosY - fSinY*fCosZ;

	m_MtxR.aa[1][0] = m_MtxF.aa[0][1] = fSinXCosZ*fSinY - fCosY*fSinZ;
	m_MtxR.aa[1][1] = m_MtxF.aa[1][1] = fCosX*fCosZ;
	m_MtxR.aa[1][2] = m_MtxF.aa[2][1] = fSinXCosZ*fCosY + fSinY*fSinZ;

	m_MtxR.aa[2][0] = m_MtxF.aa[0][2] = fSinY*fCosX;
	m_MtxR.aa[2][1] = m_MtxF.aa[1][2] = -fSinX;
	m_MtxR.aa[2][2] = m_MtxF.aa[2][2] = fCosY*fCosX;

	m_MtxR.aa[3][0] = fX;
	m_MtxR.aa[3][1] = fY;
	m_MtxR.aa[3][2] = fZ;
	m_MtxF.aa[3][0] = fNegX*m_MtxF.aa[0][0] - fY*m_MtxF.aa[1][0] - fZ*m_MtxF.aa[2][0];
	m_MtxF.aa[3][1] = fNegX*m_MtxF.aa[0][1] - fY*m_MtxF.aa[1][1] - fZ*m_MtxF.aa[2][1];
	m_MtxF.aa[3][2] = fNegX*m_MtxF.aa[0][2] - fY*m_MtxF.aa[1][2] - fZ*m_MtxF.aa[2][2];

	m_fScaleR = 1.0f;
	m_fScaleF = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildLookat( const CFVec3 &rCamPos, const CFVec3 &rLookAtPoint, const CFVec3 &rUpVec ) {
	CFVec3 vLook, vRight, vUp;
	f32 fNegCamX;

	fNegCamX = -rCamPos.x;

	vLook = (rLookAtPoint - rCamPos).Unit();
	vRight = rUpVec.UnitCross( vLook );
	vUp = vLook.UnitCross( vRight );

	m_MtxF.aa[0][0] = m_MtxR.aa[0][0] = vRight.a[0];
	m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = vRight.a[1];
	m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = vRight.a[2];

	m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = vUp.a[0];
	m_MtxF.aa[1][1] = m_MtxR.aa[1][1] = vUp.a[1];
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = vUp.a[2];

	m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = vLook.a[0];
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = vLook.a[1];
	m_MtxF.aa[2][2] = m_MtxR.aa[2][2] = vLook.a[2];

	m_MtxF.aa[3][0] = fNegCamX*vRight.a[0]	- rCamPos.a[1]*vRight.a[1]	- rCamPos.a[2]*vRight.a[2];
	m_MtxF.aa[3][1] = fNegCamX*vUp.a[0]		- rCamPos.a[1]*vUp.a[1]		- rCamPos.a[2]*vUp.a[2];
	m_MtxF.aa[3][2] = fNegCamX*vLook.a[0]	- rCamPos.a[1]*vLook.a[1]	- rCamPos.a[2]*vLook.a[2];
	m_MtxR.aa[3][0] = rCamPos.a[0];
	m_MtxR.aa[3][1] = rCamPos.a[1];
	m_MtxR.aa[3][2] = rCamPos.a[2];

	m_fScaleR = 1.0f;
	m_fScaleF = 1.0f;

	return *this;
}

CFXfm &CFXfm::BuildLookatFromDirVec( const CFVec3 &rCamPos, const CFVec3 &rLookAtUnitDirVec, const CFVec3 &rUpVec ) {
	CFVec3 vRight, vUp;
	f32 fNegCamX;

	fNegCamX = -rCamPos.x;

	vRight = rUpVec.UnitCross( rLookAtUnitDirVec );
	vUp = rLookAtUnitDirVec.UnitCross( vRight );

	m_MtxF.aa[0][0] = m_MtxR.aa[0][0] = vRight.a[0];
	m_MtxF.aa[1][0] = m_MtxR.aa[0][1] = vRight.a[1];
	m_MtxF.aa[2][0] = m_MtxR.aa[0][2] = vRight.a[2];

	m_MtxF.aa[0][1] = m_MtxR.aa[1][0] = vUp.a[0];
	m_MtxF.aa[1][1] = m_MtxR.aa[1][1] = vUp.a[1];
	m_MtxF.aa[2][1] = m_MtxR.aa[1][2] = vUp.a[2];

	m_MtxF.aa[0][2] = m_MtxR.aa[2][0] = rLookAtUnitDirVec.a[0];
	m_MtxF.aa[1][2] = m_MtxR.aa[2][1] = rLookAtUnitDirVec.a[1];
	m_MtxF.aa[2][2] = m_MtxR.aa[2][2] = rLookAtUnitDirVec.a[2];

	m_MtxF.aa[3][0] = fNegCamX*vRight.a[0]	- rCamPos.a[1]*vRight.a[1]	- rCamPos.a[2]*vRight.a[2];
	m_MtxF.aa[3][1] = fNegCamX*vUp.a[0]		- rCamPos.a[1]*vUp.a[1]		- rCamPos.a[2]*vUp.a[2];
	m_MtxF.aa[3][2] = fNegCamX*rLookAtUnitDirVec.a[0]	- rCamPos.a[1]*rLookAtUnitDirVec.a[1]	- rCamPos.a[2]*rLookAtUnitDirVec.a[2];
	m_MtxR.aa[3][0] = rCamPos.a[0];
	m_MtxR.aa[3][1] = rCamPos.a[1];
	m_MtxR.aa[3][2] = rCamPos.a[2];

	m_fScaleR = 1.0f;
	m_fScaleF = 1.0f;

	return *this;
}

void CFXfm::_Identity3x3( void ) {
	m_MtxF.Identity33();
	m_MtxR.Identity33();
}

CFXfm &CFXfm::ReceiveProductOf( const CFXfm &rSrcXfm1, const CFXfm &rSrcXfm2 ) {
	m_MtxF.Mul( rSrcXfm1.m_MtxF, rSrcXfm2.m_MtxF );
	m_MtxR.Mul( rSrcXfm2.m_MtxR, rSrcXfm1.m_MtxR );
	m_fScaleF = rSrcXfm1.m_fScaleF * rSrcXfm2.m_fScaleF;
	m_fScaleR = rSrcXfm1.m_fScaleR * rSrcXfm2.m_fScaleR;

	return *this;
}

CFXfm &CFXfm::ReceiveProductOf_InvTimesNorm( const CFXfm &rSrcXfm1, const CFXfm &rSrcXfm2 ) {
	m_MtxF.Mul( rSrcXfm1.m_MtxR, rSrcXfm2.m_MtxF );
	m_MtxR.Mul( rSrcXfm2.m_MtxR, rSrcXfm1.m_MtxF );
	m_fScaleF = rSrcXfm1.m_fScaleR * rSrcXfm2.m_fScaleF;
	m_fScaleR = rSrcXfm1.m_fScaleF * rSrcXfm2.m_fScaleR;

	return *this;
}

CFXfm &CFXfm::ReceiveProductOf_NormTimesInv( const CFXfm &rSrcXfm1, const CFXfm &rSrcXfm2 ) {
	m_MtxF.Mul( rSrcXfm1.m_MtxF, rSrcXfm2.m_MtxR );
	m_MtxR.Mul( rSrcXfm2.m_MtxF, rSrcXfm1.m_MtxR );
	m_fScaleF = rSrcXfm1.m_fScaleF * rSrcXfm2.m_fScaleR;
	m_fScaleR = rSrcXfm1.m_fScaleR * rSrcXfm2.m_fScaleF;

	return *this;
}

CFXfm &CFXfm::operator *= ( const CFXfm &rSrcXfm1 ) {
	CFMtx43A MtxR;

	m_MtxF.Mul( rSrcXfm1.m_MtxF );
	MtxR.Mul( rSrcXfm1.m_MtxR, m_MtxR );
	m_MtxR = MtxR;
	m_fScaleF *= rSrcXfm1.m_fScaleF;
	m_fScaleR *= rSrcXfm1.m_fScaleR;

	return *this;
}

CFXfm CFXfm::operator * ( const CFXfm &rSrcXfm2 ) const {
	CFXfm Xfm;

	Xfm.m_MtxF.Mul( m_MtxF, rSrcXfm2.m_MtxF );
	Xfm.m_MtxR.Mul( rSrcXfm2.m_MtxR, m_MtxR );
	Xfm.m_fScaleF = m_fScaleF * rSrcXfm2.m_fScaleF;
	Xfm.m_fScaleR = m_fScaleR * rSrcXfm2.m_fScaleR;

	return Xfm;
}

