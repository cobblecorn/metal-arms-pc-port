//////////////////////////////////////////////////////////////////////////////////////
// fxfm.h - Fang Transformation module.
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
// 09/13/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FXFM_H_
#define _FXFM_H_ 1

#include "fang.h"
#include "fmath.h"


#define FXFM_WORLD_ORIG_VS		FXfm_pView->m_MtxF.m_vPos
#define FXFM_WORLD_ORIG_MS		FXfm_pModel->m_MtxR.m_vPos
#define FXFM_CAM_ORIG_WS		FXfm_pView->m_MtxR.m_vPos
#define FXFM_CAM_ORIG_MS		FXfm_pModelView->m_MtxR.m_vPos
#define FXFM_MODEL_ORIG_WS		FXfm_pModel->m_MtxF.m_vPos
#define FXFM_MODEL_ORIG_VS		FXfm_pModelView->m_MtxF.m_vPos


// Notes:
//
// FXfm_pView->m_MtxF.m_vPos is the world's origin in view space
// FXfm_pView->m_MtxR.m_vPos is the camera's position in world space
// FXfm_pModel->m_MtxF.m_vPos is the model's position in world space
// FXfm_pModel->m_MtxR.m_vPos is the world's origin in model space
// FXfm_pModelView->m_MtxF.m_vPos is the model's origin in view space
// FXfm_pModelView->m_MtxR.m_vPos is the camera's position in model space
//
// Vec_VS = FXfm_pView->m_MtxF * Vec_WS
// Vec_WS = FXfm_pModel->m_MtxF * Vec_MS
// Vec_VS = FXfm_pModelView->m_MtxF * Vec_MS
//
// Vec_WS = FXfm_pView->m_MtxR * Vec_VS
// Vec_MS = FXfm_pModel->m_MtxR * Vec_WS
// Vec_MS = FXfm_pModelView->m_MtxR * Vec_VS




typedef enum {
	FXFM_TYPE_TRS = 0,	// General (translation, rotation, scale combo)
	FXFM_TYPE_RS,		// Rotation & scale only (no translation)
	FXFM_TYPE_S,		// Scale only (no translation, no rotation)
	FXFM_TYPE_T,		// Translation only (no scale, no rotation)

	FXFM_TYPE_COUNT,
	FXFM_FORCE_DWORD = 0x7fffffff	// Forces this enumeration to compile to 32 bits in size
} FXfmType_e;

//NOTE - PS2 asserts the fact that sizeof(CFXfm) is a multiple of 16-bytes!!
FCLASS_ALIGN_PREFIX class CFXfm {
public:
	// Data Members:

	CFMtx43A m_MtxF;	// Forward matrix (transforms a model space point to world or camera space)
	CFMtx43A m_MtxR;	// Reverse matrix (transforms a world or camera space point to model space)

	f32 m_fScaleF;		// Forward scale that's built into the MtxF matrix (fScaleF = 1.0f/fScaleR)
	f32 m_fScaleR;		// Reverse scale that's built into the MtxR matrix (fScaleR = 1.0f/fScaleF)

	// Object Interface:
	FINLINE CFXfm() {}

	FINLINE CFXfm &Identity( void );
	FINLINE CFXfm &Invert( void );
	FINLINE CFXfm &ReceiveInverseOf( const CFXfm &rSrcXfm );
	CFXfm &ReceiveProductOf( const CFXfm &rSrcXfm1, const CFXfm &rSrcXfm2 );
	CFXfm &ReceiveProductOf_InvTimesNorm( const CFXfm &rSrcXfm1, const CFXfm &rSrcXfm2 );
	CFXfm &ReceiveProductOf_NormTimesInv( const CFXfm &rSrcXfm1, const CFXfm &rSrcXfm2 );

	FINLINE CFXfm &operator = ( const CFXfm &rSrcXfm ) { fang_MemCopy( this, &rSrcXfm, sizeof(CFXfm) ); return *this; }
	CFXfm &operator *= ( const CFXfm &rXfm );
	CFXfm operator * ( const CFXfm &rSrcXfm2 ) const;

	CFXfm &BuildFromMtx( const CFMtx33 &rMtx, BOOL bComputeScale=FALSE );
	CFXfm &BuildFromMtx( const CFMtx43 &rMtx, BOOL bComputeScale=FALSE );
	CFXfm &BuildFromMtx( const CFMtx43A &rMtx, BOOL bComputeScale=FALSE );
	CFXfm &BuildFromMtx( const CFMtx33 &rMtx, f32 fScale );
	CFXfm &BuildFromMtx( const CFMtx43 &rMtx, f32 fScale );
	CFXfm &BuildFromMtx( const CFMtx43A &rMtx, f32 fScale );

	CFXfm &BuildFromInvMtx( const CFMtx43A &rMtx, BOOL bComputeScale=FALSE );

	CFXfm &BuildFromQuat( const CFQuatA &rQuat );
	CFXfm &BuildFromQuat( const CFQuatA &rQuat, f32 fScale );
	CFXfm &BuildFromQuat( const CFQuatA &rQuat, const CFVec3A &rPos );
	CFXfm &BuildFromQuat( const CFQuatA &rQuat, const CFVec3A &rPos, f32 fScale );

	CFXfm &BuildFromInvQuat( const CFQuatA &rQuat );
	CFXfm &BuildFromInvQuat( const CFQuatA &rQuat, const CFVec3A &rPos );

	CFXfm &BuildTranslation( const CFVec3 &rVec );
	CFXfm &BuildTranslation( const CFVec3A &rVec );
	CFXfm &BuildTranslation( f32 fX, f32 fY, f32 fZ );

	CFXfm &BuildScale( f32 fScale );

	CFXfm &BuildRotation( CFMtx43A &rRotMtx43A );
	CFXfm &BuildRotationX( f32 fRadiansX );
	CFXfm &BuildRotationX( f32 fRadiansX, const CFVec3 &rXlat );
	CFXfm &BuildRotationX( f32 fRadiansX, f32 fX, f32 fY, f32 fZ );
	CFXfm &BuildRotationY( f32 fRadiansY );
	CFXfm &BuildRotationY( f32 fRadiansY, const CFVec3 &rXlat );
	CFXfm &BuildRotationY( f32 fRadiansY, f32 fX, f32 fY, f32 fZ );
	CFXfm &BuildRotationZ( f32 fRadiansZ );
	CFXfm &BuildRotationZ( f32 fRadiansZ, const CFVec3 &rXlat );
	CFXfm &BuildRotationZ( f32 fRadiansZ, f32 fX, f32 fY, f32 fZ );
	CFXfm &BuildRotYXZ_XlatFromPoint( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ, f32 fX, f32 fY, f32 fZ );

	CFXfm &BuildCameraRotAndInitStack( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ, f32 fX, f32 fY, f32 fZ );
	CFXfm &BuildInvRotYXZ_XlatFromPoint( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ, f32 fX, f32 fY, f32 fZ );

	CFXfm &BuildLookat( const CFVec3 &rCamPos, const CFVec3 &rLookAtPoint, const CFVec3 &rUpVec=CFVec3(0.0f,1.0f,0.0f) );
	CFXfm &BuildLookatFromDirVec( const CFVec3 &rCamPos, const CFVec3 &rLookAtUnitDirVec, const CFVec3 &rUpVec );

	FINLINE CFSphere &TransformSphereF( CFSphere &rDestSphere, const CFSphere &rSrcSphere ) const;
	FINLINE CFSphere &TransformSphereR( CFSphere &rDestSphere, const CFSphere &rSrcSphere ) const;

	FINLINE CFVec3 &TransformPointF( CFVec3 &rDestVec, const CFVec3 &rSrcVec ) const;
	FINLINE CFVec3 &TransformPointR( CFVec3 &rDestVec, const CFVec3 &rSrcVec ) const;

	FINLINE CFVec3 &TransformDirF( CFVec3 &rDestVec, const CFVec3 &rSrcVec ) const;
	FINLINE CFVec3 &TransformDirR( CFVec3 &rDestVec, const CFVec3 &rSrcVec ) const;

	FINLINE CFVec3 &TransformUnitDirF( CFVec3 &rDestVec, const CFVec3 &rSrcVec ) const;
	FINLINE CFVec3 &TransformUnitDirR( CFVec3 &rDestVec, const CFVec3 &rSrcVec ) const;

	FINLINE void InitStackWithView( void ) const { InitStack( this ); }
	void PushModel( void ) const;

	// Static Interface:
	static void InitStack( const CFXfm *pXfmView = NULL );
	static void PopModel( u32 nPopCount = 1 );

private:
	FINLINE void _Identity3x3( void );

	FCLASS_STACKMEM_ALIGN( CFXfm );
} FCLASS_ALIGN_SUFFIX;


FINLINE CFXfm& CFXfm::Identity( void ) {
	m_MtxF.Identity();
	m_MtxR.Identity();
	m_fScaleF = 1.0f;
	m_fScaleR = 1.0f;

	return *this;
}

FINLINE CFXfm& CFXfm::Invert( void ) {
	CFMtx43A MtxF;
	f32 fScaleF;

	MtxF = m_MtxF;
	fScaleF = m_fScaleF;

	m_MtxF = m_MtxR;
	m_MtxR = MtxF;

	m_fScaleF = m_fScaleR;
	m_fScaleR = fScaleF;

	return *this;
}

FINLINE CFXfm& CFXfm::ReceiveInverseOf( const CFXfm& rSrcXfm ) {
	m_MtxR = rSrcXfm.m_MtxF;
	m_MtxF = rSrcXfm.m_MtxR;
	m_fScaleR = rSrcXfm.m_fScaleF;
	m_fScaleF = rSrcXfm.m_fScaleR;

	return *this;
}

FINLINE CFSphere& CFXfm::TransformSphereF( CFSphere &rDestSphere, const CFSphere &rSrcSphere ) const {
	rDestSphere.m_Pos = m_MtxF.m44.MultPoint( rSrcSphere.m_Pos );
	rDestSphere.m_fRadius = m_fScaleF * rSrcSphere.m_fRadius;
	return rDestSphere;
}

FINLINE CFSphere& CFXfm::TransformSphereR( CFSphere &rDestSphere, const CFSphere &rSrcSphere ) const {
	rDestSphere.m_Pos = m_MtxR.m44.MultPoint( rSrcSphere.m_Pos );
	rDestSphere.m_fRadius = m_fScaleR * rSrcSphere.m_fRadius;
	return rDestSphere;
}

FINLINE CFVec3& CFXfm::TransformPointF( CFVec3& rDestVec, const CFVec3& rSrcVec ) const {
	rDestVec = m_MtxF.m44.MultPoint( rSrcVec );
	return rDestVec;
}

FINLINE CFVec3& CFXfm::TransformPointR( CFVec3& rDestVec, const CFVec3& rSrcVec ) const {
	rDestVec = m_MtxR.m44.MultPoint( rSrcVec );
	return rDestVec;
}

FINLINE CFVec3& CFXfm::TransformDirF( CFVec3& rDestVec, const CFVec3& rSrcVec ) const {
	rDestVec = m_MtxF.m44.MultDir( rSrcVec );
	return rDestVec;
}

FINLINE CFVec3& CFXfm::TransformDirR( CFVec3& rDestVec, const CFVec3& rSrcVec ) const {
	rDestVec = m_MtxR.m44.MultDir( rSrcVec );
	return rDestVec;
}

FINLINE CFVec3& CFXfm::TransformUnitDirF( CFVec3& rDestVec, const CFVec3& rSrcVec ) const {
	rDestVec = m_MtxF.m44.MultDir( rSrcVec ) * m_fScaleR;
	return rDestVec;
}

FINLINE CFVec3& CFXfm::TransformUnitDirR( CFVec3& rDestVec, const CFVec3& rSrcVec ) const {
	rDestVec = m_MtxR.m44.MultDir( rSrcVec ) * m_fScaleF;
	return rDestVec;
}



extern CFXfm *FXfm_pView;				// View xfm (MtxF transforms from WS to VS)
extern CFXfm *FXfm_pModel;				// Concatenated model xfm (MtxF transforms from MS to WS)
extern CFXfm *FXfm_pModelView;			// Concatenated model*view xfm (MtxF transforms from MS to VS)
extern CFMtx43A *FXfm_pMirrorMtx;		// Pointer to a mirror matrix, if any

extern u32 FXfm_nViewKey;				// Incremented each time the view xfm is modified
extern u32 FXfm_nModelKey;				// Incremented each time the model xfm is modified
extern u32 FXfm_nModelViewKey;			// Incremented each time either the model or view xfm is modified

extern CFXfm FXfm_Identity;				// Identity xfm


extern void fxfm_SetViewAndWorldSpaceModelMatrices( void );
extern BOOL fxfm_ModuleStartup( void );
extern void fxfm_ModuleShutdown( void );




#endif

