//////////////////////////////////////////////////////////////////////////////////////
// fGCshadow.cpp - 
//
// Author: John Lafleur
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
// 02/18/02	Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fgcvid.h"
#include "fgcshadow.h"
#include "fgcmesh.h"
#include "fgcxfm.h"
#include "fgcviewport.h"
#include "fsh.h"
#include "fcamera.h"
#include "fRenderSort.h"
#include "frenderer.h"

#include "fvis.h"
#include "fdraw.h"

#include "fworld.h"
#include "fgctex.h"

#include "fshadow.h"
#include <stdio.h>

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );
static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

u32 _nNextMap=0;
BOOL bSetupShadowBuffers=FALSE;
BOOL FShadow_bActive=TRUE;

enum
{
	NUM_512=1,
	NUM_256=2,
	NUM_128=8,
	NUM_SHADOW_BUFFERS=NUM_512+NUM_256+NUM_128,
};

typedef struct
{
	BOOL bActive;
	BOOL bLOD;
	u8 nIdx;
	CFLight *pLight;
	CFMeshInst *pMesh;
	CFSphere *pSphere;
	
	CFVec3 vViewPos;
	CFVec3 vViewDir;	//view direction for light.
	CFVec3 vViewUp;		//up direction for light.
	f32 fHalfAngRad; 	//half angle for light frustum.
	
	f32 fShadowFade;
} Shadow_Buffer_Data_t;

typedef struct
{
	f32 fValue;
	CFLight *pLight;
	BOOL bActive;
	u8 nNumShadowMeshs;
} Shadow_Light_t;

enum
{
	MAX_VOLUMES=10
};

typedef struct
{
	u32 nLightCount;
	CFLight *pLight[64];
	CFMeshInst *pMesh;
} VolumeLightList_t;

VolumeLightList_t _aVolumeLights[MAX_VOLUMES];
volatile u32 _nVolumeCount=0;

CFTexInst *_pShadowBuffers[NUM_SHADOW_BUFFERS];
Shadow_Buffer_Data_t aShadowData[NUM_SHADOW_BUFFERS];
Shadow_Buffer_Data_t aShadowLOD[NUM_SHADOW_BUFFERS*4];
Shadow_Light_t _aShadowSubmit[100];
u32 _nLightSubmit=0;
u32 _nShadowBufUsed=0;
u32 _nShadowLOD=0;

BOOL FShadow_WorldCallbackFunc( FWorldEvent_e nEvent );
void fshadow_ShadowCallback(void *pUserData);
void fshadow_ResetShadowBuffers();
void fshadow_CreateShadowBuffers();

f32 _CalcLightValue(CFLight *pLight);
BOOL _BuildShadowMtx(Shadow_Buffer_Data_t *pData, u32 nIdx, u32 nType);
s32 _FindShadowBuffer(CFLight *pLight, u32 nMeshIdx);
s32 _FindShadowBuffer_LOD(CFLight *pLight, u32 nMeshIdx);
f32 _CheckRangeFromCurCamera(CFMeshInst *pMesh);
void _SetupLights( u32 nNumShadows );
void _SetupLight( CFLight *pLight, u32 nLightIdx, GXColor gxClr );

BOOL fgcshadow_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );
	_bModuleInitialized = TRUE;

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	return TRUE;
}

void fgcshadow_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}

void fshadow_DrawTest()
{
	CFVec3 quad[4];
	CFColorRGBA color(1.0f, 1.0f, 1.0f, 1.0f);

	quad[0].Set(32.0f, 1.0f, -8.0f);

	quad[1].x = quad[0].x + 10.0f;
	quad[1].y = quad[0].y;
	quad[1].z = quad[2].z = quad[3].z = quad[0].z;

	quad[2].x = quad[0].x + 10.0f;
	quad[2].y = quad[0].y - 10.0f;

	quad[3].x = quad[0].x;
	quad[3].y = quad[0].y - 10.0f;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	fdraw_SetTexture(_pShadowBuffers[0]);
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);

	quad[0].Set(22.0f, 1.0f, -8.0f);

	quad[1].x = quad[0].x + 10.0f;
	quad[1].y = quad[0].y;
	quad[1].z = quad[2].z = quad[3].z = quad[0].z;

	quad[2].x = quad[0].x + 10.0f;
	quad[2].y = quad[0].y - 10.0f;

	quad[3].x = quad[0].x;
	quad[3].y = quad[0].y - 10.0f;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	fdraw_SetTexture(_pShadowBuffers[1]);
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);

	quad[0].Set(12.0f, 1.0f, -8.0f);

	quad[1].x = quad[0].x + 10.0f;
	quad[1].y = quad[0].y;
	quad[1].z = quad[2].z = quad[3].z = quad[0].z;

	quad[2].x = quad[0].x + 10.0f;
	quad[2].y = quad[0].y - 10.0f;

	quad[3].x = quad[0].x;
	quad[3].y = quad[0].y - 10.0f;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	fdraw_SetTexture(_pShadowBuffers[2]);
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);

	quad[0].Set(2.0f, 1.0f, -8.0f);

	quad[1].x = quad[0].x + 10.0f;
	quad[1].y = quad[0].y;
	quad[1].z = quad[2].z = quad[3].z = quad[0].z;

	quad[2].x = quad[0].x + 10.0f;
	quad[2].y = quad[0].y - 10.0f;

	quad[3].x = quad[0].x;
	quad[3].y = quad[0].y - 10.0f;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	fdraw_SetTexture(_pShadowBuffers[3]);
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);

	quad[0].Set(-8.0f, 1.0f, -8.0f);

	quad[1].x = quad[0].x + 10.0f;
	quad[1].y = quad[0].y;
	quad[1].z = quad[2].z = quad[3].z = quad[0].z;

	quad[2].x = quad[0].x + 10.0f;
	quad[2].y = quad[0].y - 10.0f;

	quad[3].x = quad[0].x;
	quad[3].y = quad[0].y - 10.0f;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	fdraw_SetTexture(_pShadowBuffers[4]);
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);

	quad[0].Set(-18.0f, 1.0f, -8.0f);

	quad[1].x = quad[0].x + 10.0f;
	quad[1].y = quad[0].y;
	quad[1].z = quad[2].z = quad[3].z = quad[0].z;

	quad[2].x = quad[0].x + 10.0f;
	quad[2].y = quad[0].y - 10.0f;

	quad[3].x = quad[0].x;
	quad[3].y = quad[0].y - 10.0f;

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECALTEX_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
	fdraw_SetTexture(_pShadowBuffers[5]);
	fdraw_TexQuad(&quad[0], &quad[1], &quad[2], &quad[3], &color);
}

static CFXfm _VXfm;
extern FViewport_t *_pSView;

extern CFLight *FMesh_CurrentLight;
BOOL bPrevRenderFlags;

f32 _pFOVAdj=1.5f;

void fshadow_ShadowCallback(void *pUserData)
{
	CFVec3 vLookAt, vPos, vDir, vUp(0,1,0);
	u32 nShadowIdx = (u32)pUserData;
	f32 fZFar=500.0f;
	
	if (_nShadowBufUsed < 1) return;

	if (aShadowData[nShadowIdx].bActive)
	{
		FMesh_bRenderShadows = TRUE;
		FMesh_CurrentLight = aShadowData[nShadowIdx].pLight;

		aShadowData[nShadowIdx].pLight->m_pShadowTex = _pShadowBuffers[nShadowIdx];

		switch (aShadowData[nShadowIdx].pLight->m_nType)
		{
			case FLIGHT_TYPE_DIR:
				fZFar = 2000.0f;
			case FLIGHT_TYPE_SPOT:
			case FLIGHT_TYPE_OMNI:
				{
					vPos = aShadowData[nShadowIdx].vViewPos;
					vDir = aShadowData[nShadowIdx].vViewDir;
					vUp = aShadowData[nShadowIdx].vViewUp;
					
					f32 fFOV = aShadowData[nShadowIdx].fHalfAngRad;
					fviewport_InitPersp(_pSView, fFOV, fFOV, 1.0f, fZFar);
					fviewport_SetActive(_pSView);
					
					fviewport_Clear(FVIEWPORT_CLEARFLAG_COLOR | FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0);
					
					_VXfm.BuildLookatFromDirVec(vPos, vDir, vUp);
					_VXfm.InitStackWithView();
					
					//Do scissor here.
					u32 nXO, nYO, nDx, nDy, nA, nB;
					GXGetScissor(&nXO, &nYO, &nDx, &nDy);
					nA = 512+2; nB = nYO+2;
					GXSetScissor(nA, nB, _pShadowBuffers[nShadowIdx]->GetTexDef()->TexInfo.nTexelsAcross-3, _pShadowBuffers[nShadowIdx]->GetTexDef()->TexInfo.nTexelsDown-3);
					FASSERT( nA + _pShadowBuffers[nShadowIdx]->GetTexDef()->TexInfo.nTexelsAcross-3 <= 640 );
					//

					frenderer_Push( FRENDERER_MESH );
					aShadowData[nShadowIdx].pMesh->Draw( FVIEWPORT_PLANESMASK_NONE, TRUE );
					frenderer_Pop();
					
					//Restore scissor here.
					GXSetScissor(nXO, nYO, nDx, nDy);
					FASSERT( nXO + nDx <= 640 );
					//
				}
				break;
		}
		FMesh_bRenderShadows = FALSE;
		FMesh_CurrentLight = NULL;
	}
}

void fshadow_CreateShadowBuffers()
{
	int i, n=0;
	char szName[16];
	
	for (i=0; i<NUM_512; i++, n++)
	{
		sprintf(szName, "SBuf_%d", n);
		_pShadowBuffers[n] = ftex_CreateRenderTarget(128, 128, FTEX_RENDERTARGET_FMT_I4_D24, szName, FALSE, FRES_NULLHANDLE, NULL, FTEX_2D, TRUE);
 
		_pShadowBuffers[n]->SetFlag(CFTexInst::FLAG_NORELEASE);
	}

	for (i=0; i<NUM_256; i++, n++)
	{
		sprintf(szName, "SBuf_%d", n);
		_pShadowBuffers[n] = ftex_CreateRenderTarget(128, 128, FTEX_RENDERTARGET_FMT_I4_D24, szName, FALSE, FRES_NULLHANDLE, NULL, FTEX_2D, TRUE);

		_pShadowBuffers[n]->SetFlag(CFTexInst::FLAG_NORELEASE);
	}
	
	for (i=0; i<NUM_128; i++, n++)
	{
		sprintf(szName, "SBuf_%d", n);
		_pShadowBuffers[n] = ftex_CreateRenderTarget(64, 64, FTEX_RENDERTARGET_FMT_I4_D24, szName, FALSE, FRES_NULLHANDLE, NULL, FTEX_2D, TRUE);

		_pShadowBuffers[n]->SetFlag(CFTexInst::FLAG_NORELEASE);
	}
}

void fshadow_ClearLightList()
{
	_nNextMap=0;
	
	s32 nCamIdx = fcamera_GetLastCameraIndex();
	CFCamera *pCamera=NULL;
	
	if (nCamIdx > -1)
	{
		pCamera = fcamera_GetCameraByIndex( nCamIdx );
	}
	else
	{
		pCamera = NULL;
	}

	if (pCamera)
	{
		const CFXfm *pCamXfm = pCamera->GetFinalXfm();
		FMesh_vShadowCam.Set( pCamXfm->m_MtxR.m_vPos.x, pCamXfm->m_MtxR.m_vPos.y, pCamXfm->m_MtxR.m_vPos.z );
	}
	else
	{
		FMesh_vShadowCam.Set( 0, 0, 0 );
	}
	
	_nLightSubmit=0;
	_nShadowBufUsed = 0;
	_nShadowLOD = 0;
}

void fshadow_ClearVolumeList()
{
	_nVolumeCount = 0;
}


void fshadow_ResetShadowBuffers()
{
	int i;
	
	for (i=0; i<NUM_SHADOW_BUFFERS; i++)
	{
		ftex_AddRenderTarget(_pShadowBuffers[i], fshadow_ShadowCallback, TRUE, 30, FALSE, TRUE, (void*)i, FALSE, TRUE);
	}
	
	fshadow_ClearLightList();
	fshadow_ClearVolumeList();
}

f32 _CalcLightValue(CFLight *pLight)
{
	if (pLight->m_nType == FLIGHT_TYPE_DIR)
	{
		return ( 1000.0f );
	}
	CFVec3 vOffs;
	
	f32 fD2, fR2;

	if ( !FShadow_bMultiplayer )
	{
		vOffs = FMesh_vShadowCam - pLight->m_spInfluence_WS.m_Pos;
		fD2 = vOffs.InvMag2();
		fR2 = 0.0f;
		if (pLight->m_fOOR2_WS > 0.1f)
		{
			fR2 = 1.0f/pLight->m_fOOR2_WS;
		}
	}
	else
	{
		fR2 = fD2 = 1.0f;
	}

	f32 fI = 0.39f * pLight->m_ScaledColor.fRed + 0.59f * pLight->m_ScaledColor.fGreen + 0.11f * pLight->m_ScaledColor.fBlue;
	
	return ( fI * fR2 * fD2 );
}

void fshadow_SubmitLight(CFLight *pLight, CFMeshInst *pMesh)
{
	if (!FShadow_bActive) { return; }
	
	//Only these light types can cast shadows. Ambient lights cannot.
	if (pLight->m_nType == FLIGHT_TYPE_DIR || pLight->m_nType == FLIGHT_TYPE_OMNI || pLight->m_nType == FLIGHT_TYPE_SPOT)
	{
		Shadow_Light_t *pSubmit = &_aShadowSubmit[_nLightSubmit];
		
		if ( fshadow_LightCastShadow(pLight) )
		{
			pSubmit->nNumShadowMeshs = fvis_GetNumShadows(pLight->m_nIndex);
			if (pSubmit->nNumShadowMeshs > 0)
			{
				pSubmit->fValue = _CalcLightValue(pLight);
				pSubmit->pLight = pLight;
				_nLightSubmit++;
			}
		}
	}
}

f32 fDirDist = 1000.0f;

BOOL _BuildShadowMtx(Shadow_Buffer_Data_t *pData, u32 nIdx, u32 nType)
{
	u32 i, nMeshUse=0;

	if (pData->pMesh->m_BoundSphere_MS.m_fRadius < 0.01f)
	{
		return FALSE;
	}

	if (nIdx > 0)
	{
		for (i=0; i<nIdx; i++)
		{
			if (nType == 0)
			{
				if (aShadowData[i].pMesh == pData->pMesh)
				{
					nMeshUse++;
					if (nMeshUse > 1) { return FALSE; }
				}
			}
			else
			{
				if (aShadowLOD[i].pMesh == pData->pMesh)
				{
					nMeshUse++;
					if (nMeshUse > 1) { return FALSE; }
				}
			}
		}
		if (nMeshUse > 1) { return FALSE; }
	}

	CFVec3A vPos;
	vPos.Set(pData->pMesh->m_BoundSphere_MS.m_Pos.x, pData->pMesh->m_BoundSphere_MS.m_Pos.y, pData->pMesh->m_BoundSphere_MS.m_Pos.z);
	vPos = pData->pMesh->m_Xfm.m_MtxF.MulPoint(vPos);
	
	pData->vViewDir.Set(vPos.x, vPos.y, vPos.z);
	
	if (pData->pLight->m_nType == FLIGHT_TYPE_DIR)
	{
		pData->vViewPos.x = vPos.x - pData->pLight->m_mtxOrientation_WS.m_vFront.x*fDirDist;
		pData->vViewPos.y = vPos.y - pData->pLight->m_mtxOrientation_WS.m_vFront.y*fDirDist;
		pData->vViewPos.z = vPos.z - pData->pLight->m_mtxOrientation_WS.m_vFront.z*fDirDist;
	}
	else
	{
		pData->vViewPos.Set(pData->pLight->m_spInfluence_WS.m_Pos.x, pData->pLight->m_spInfluence_WS.m_Pos.y, pData->pLight->m_spInfluence_WS.m_Pos.z);
	}
	
	if ( FMATH_FABS(pData->vViewDir.x-pData->vViewPos.x)<0.1f && FMATH_FABS(pData->vViewDir.y-pData->vViewPos.y)<0.1f && FMATH_FABS(pData->vViewDir.z-pData->vViewPos.z)<0.1f)
	{
		return FALSE;
	}

	pData->vViewDir = pData->vViewDir - pData->vViewPos;

	if ( FMATH_FABS(pData->vViewDir.x)<0.1f && FMATH_FABS(pData->vViewDir.y)<0.1f && FMATH_FABS(pData->vViewDir.z)<0.1f)
	{
		return FALSE;
	}

	f32 fMag = pData->vViewDir.Mag();
	pData->vViewDir.Unitize();
	
	pData->vViewUp.Set(0, 1, 0);
	if ( FMATH_FABS(pData->vViewDir.y) > 0.90f )
	{
		pData->vViewUp.Set(1, 0, 0);
	}
	
	pData->fHalfAngRad = fmath_Atan( pData->pMesh->m_BoundSphere_MS.m_fRadius, fMag ) * 1.5f;

	if (pData->fHalfAngRad < 0.000001f)
	{
		return FALSE;
	}

	return TRUE;
}

f32 fshadow_GetShadowMtx(CFLight *pLight, CFVec3 *pPos, CFVec3* pDir, CFVec3* pUp, CFMeshInst *pMesh/*=NULL*/)
{
	u32 i;
	for (i=0; i<NUM_SHADOW_BUFFERS; i++)
	{
		if (aShadowData[i].pLight == pLight && (aShadowData[i].pMesh == pMesh || pMesh == NULL) )
		{
			*pDir = aShadowData[i].vViewDir;
			
			if ( FMATH_FABS(pDir->x) < 0.5f && FMATH_FABS(pDir->y) < 0.5f && FMATH_FABS(pDir->z) < 0.5f )
			{
				pDir->y = -1.0f;
			}
			
			*pUp = aShadowData[i].vViewUp;
			
			*pPos = aShadowData[i].vViewPos;
			
			return aShadowData[i].fHalfAngRad;
		}
	}
	pDir->x = pDir->z = 0.0f;
	pDir->y = -1.0f;
	
	pUp->y = pUp->z = 0.0f;
	pUp->x = 1.0f;
	
	pPos->x = pPos->y = pPos->z = 0.0f;
	return 0;
}

Shadow_Buffer_Data_t *pShadowLights[50];
u32 nShadowLights;

f32 _CheckRangeFromCurCamera(CFMeshInst *pMesh)
{
	f32 fRet = 1.0f;

	CFVec3 vOffs;
	CFVec3 vOrig;
		
	vOffs.x = pMesh->m_Xfm.m_MtxF.m_vPos.x - FXFM_CAM_ORIG_WS.x;
	vOffs.y = pMesh->m_Xfm.m_MtxF.m_vPos.y - FXFM_CAM_ORIG_WS.y;
	vOffs.z = pMesh->m_Xfm.m_MtxF.m_vPos.z - FXFM_CAM_ORIG_WS.z;

	if ( FMATH_FABS(vOffs.x) > FShadow_ShadowDist_MP || FMATH_FABS(vOffs.y) > FShadow_ShadowDist_MP || FMATH_FABS(vOffs.z) > FShadow_ShadowDist_MP )
	{
		fRet = 0.0f;
	}
	else
	{
		fRet = 1.0f - (vOffs.Mag2() - FShadow_fShadowStartFade_MP2)*FShadow_fOOShadowFadeDist_MP2;
	}

	return fRet;
}

s32 _FindShadowBuffer(CFLight *pLight, u32 nMeshIdx)
{
	s32 i, nIdx=0;
	for (i=0; i<(s32)_nShadowBufUsed; i++)
	{
		if (aShadowData[i].pLight == pLight && nIdx == nMeshIdx)
		{
			if ( FShadow_bMultiplayer)
			{
				aShadowData[i].fShadowFade = _CheckRangeFromCurCamera(aShadowData[i].pMesh);
				if ( !aShadowData[i].fShadowFade )
				{
					return (-1);
				}
			}
			return (i);
		}
		else if (aShadowData[i].pLight == pLight)
		{
			nIdx++;
		}
	}
	return (-1);
}

s32 _FindShadowBuffer_LOD(CFLight *pLight, u32 nMeshIdx)
{
	s32 i, nIdx=0;
	for (i=0; i<(s32)_nShadowLOD; i++)
	{
		if (aShadowLOD[i].pLight == pLight && nIdx == nMeshIdx)
		{
			return (i);
		}
		else if (aShadowLOD[i].pLight == pLight)
		{
			nIdx++;
		}
	}
	return (-1);
}

void fshadow_SaveVolumeLightList(CFWorldLight *apVolumeLights[], u32 nVolumeLightCount, CFMeshInst *pWorldGeo)
{
	if (!FShadow_bActive) { return; }
	if (nVolumeLightCount < 1) return;
	
	if ( _nVolumeCount >= MAX_VOLUMES )
	{
		return;
	}

	u32 i;
	VolumeLightList_t *pVolume;

	pVolume = &_aVolumeLights[_nVolumeCount];
	
	pVolume->nLightCount = 0;
	pVolume->pMesh = pWorldGeo;

	for (i=0; i<nVolumeLightCount; i++)
	{
		if (apVolumeLights[i]->m_Light.m_nFlags&FLIGHT_FLAG_CAST_SHADOWS)
		{
			pVolume->pLight[ pVolume->nLightCount++ ] = &apVolumeLights[i]->m_Light;
		}
	}

	if (pVolume->nLightCount)
	{
		_nVolumeCount++;
	}
}

u32 fshadow_GetNumShadowPasses(FMesh_RenderLight_t *pRenderLights, u8 nRenderLightCount, CFMeshInst *pMesh, BOOL bVolumeGeo)
{
	if (!FShadow_bActive) { return 0; }
	
	u32 nPasses=0, i, nMesh, nMeshIdx;
	u32 nRet;
	s32 nBufIdx;
	BOOL bNoZero=FALSE;
	nShadowLights = 0;
	
	if (aShadowData[0].pLight || aShadowLOD[0].pLight)
	{
		if (aShadowData[0].pLight)
		{
			if (aShadowData[0].pLight->m_nType == FLIGHT_TYPE_DIR)
			{
				nMeshIdx = 0;
				if ( FShadow_bMultiplayer )
				{
					u32 i;
					for (i=0; i<_nShadowBufUsed; i++)
					{
						nBufIdx = _FindShadowBuffer(aShadowData[0].pLight, nMeshIdx);
						if (nBufIdx > -1)
						{
							nPasses++;
							pShadowLights[nShadowLights++] = &aShadowData[nBufIdx];
							bNoZero = TRUE;
						}
						nMeshIdx++;
					}
				}
				else
				{
					do
					{
						nBufIdx = _FindShadowBuffer(aShadowData[0].pLight, nMeshIdx);
						if (nBufIdx > -1)
						{
							nPasses++;
							pShadowLights[nShadowLights++] = &aShadowData[nBufIdx];
							bNoZero = TRUE;
						}
						nMeshIdx++;
					} while (nBufIdx > -1);
				}
			}
		}
		if (aShadowLOD[0].pLight)
		{	
			if (aShadowLOD[0].pLight->m_nType == FLIGHT_TYPE_DIR)
			{
				nMeshIdx = 0;
				do
				{
					nBufIdx = _FindShadowBuffer_LOD(aShadowData[0].pLight, nMeshIdx);
					if (nBufIdx > -1)
					{
						nPasses++;
						pShadowLights[nShadowLights++] = &aShadowLOD[nBufIdx];
						bNoZero = TRUE;
					}
					nMeshIdx++;
				} while (nBufIdx > -1);
			}
		}
	}
	
	if (bVolumeGeo)
	{
		if (_nVolumeCount)
		{
			VolumeLightList_t *pVolume=NULL;
			for (i=0; i<_nVolumeCount; i++)
			{
				pVolume = &_aVolumeLights[i];

				if (pVolume->pMesh == pMesh)
				{
                    break;
				}
			}
			if (i < _nVolumeCount && pVolume)
			{
				for (i=0; i<pVolume->nLightCount; i++)
				{	
					nMesh = fvis_GetNumShadows(pVolume->pLight[i]->m_nIndex);
					if (nMesh > 0)
					{
						nMeshIdx = 0;
						do
						{
							nBufIdx = _FindShadowBuffer(pVolume->pLight[i], nMeshIdx);
							if (nBufIdx > -1 && (pVolume->pLight[i]->m_nType != FLIGHT_TYPE_DIR || bNoZero==FALSE))
							{
								nPasses++;
								pShadowLights[nShadowLights++] = &aShadowData[nBufIdx];
							}
							nMeshIdx++;
						} while (nBufIdx > -1);
					}
				}
			}
		}
	}
	else
	{
		for (i=0; i<nRenderLightCount; i++)
		{
			nMesh = fvis_GetNumShadows(pRenderLights[i].pLight->m_nIndex);
			if ((pRenderLights[i].pLight->m_nFlags&FLIGHT_FLAG_CAST_SHADOWS) && nMesh > 0)
			{
				nMeshIdx = 0;
				do
				{
					nBufIdx = _FindShadowBuffer(pRenderLights[i].pLight, nMeshIdx);
					if (nBufIdx > -1 && (pRenderLights[i].pLight->m_nType != FLIGHT_TYPE_DIR || bNoZero==FALSE))
					{
						nPasses++;
						pShadowLights[nShadowLights++] = &aShadowData[nBufIdx];
					}
					nMeshIdx++;
				} while (nBufIdx > -1);
			}
		}
	}

	nRet = 0;
	if (nPasses > 0)
	{
		nRet += ((nPasses-1)>>2);
		nRet++;
	}
			
	return (nRet);
}

extern void _LightLookAt(MtxPtr pLookAt, CFVec3A& camPos, CFVec3A& up, CFVec3A& target);
extern void _LightPerspective(MtxPtr m, float fov, float fDist=0.0f);
extern void _ApplyInvView(Mtx44Ptr pMtx);
extern void _SetTexture( GXTexObj *pGCTexObj, GXTexMapID nIDNum );
extern u32 _nPosMatrix;

extern GXTexObj _gxAttenMap;
extern GXTexObj _gxTexKill;

u32 _nCurShaderIdx = 0xff;

void fshadow_BeginShadowRender()
{
	if (!FShadow_bActive) { return; }

	fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
	fgc_SetZMode( TRUE, GX_EQUAL, FALSE );
	fgc_SetBlendOp(GX_BM_BLEND, GX_BL_ZERO, GX_BL_SRCCLR, GX_LO_SET);

	_nCurShaderIdx = 0xff;
}

void fshadow_EndShadowRender()
{
	if (!FShadow_bActive) { return; }

	fgc_DisableChannels();
/*	
	
	GXSetChanCtrl(	GX_COLOR0A0,
						GX_ENABLE,		// enable Channel
						GX_SRC_VTX,		// amb source
						GX_SRC_REG,		// mat source
						GX_LIGHT_NULL,		// light mask
						GX_DF_CLAMP,	// diffuse function
						GX_AF_SPOT );	// atten   function
	GXSetChanCtrl(	GX_COLOR1,
					GX_DISABLE,		// enable Channel
					GX_SRC_REG,		// amb source
					GX_SRC_REG,		// mat source
					GX_LIGHT_NULL,		// light mask
					GX_DF_CLAMP,	// diffuse function
					GX_AF_SPOT );	// atten   function
	GXSetChanCtrl(	GX_ALPHA1,
					GX_DISABLE,		// enable Channel
					GX_SRC_REG,		// amb source
					GX_SRC_REG,		// mat source
					GX_LIGHT_NULL,		// light mask
					GX_DF_CLAMP,	// diffuse function
					GX_AF_SPOT );	// atten   function
*/
}

void _SetupLights( u32 nNumShadows )
{
	fgc_SetChannelsForShadows( nNumShadows );
/*
	if (nNumShadows > 0)
	{
		GXSetChanCtrl(	GX_COLOR0,
						GX_ENABLE,		// enable Channel
						GX_SRC_REG,		// amb source
						GX_SRC_REG,		// mat source
						GX_LIGHT0,		// light mask
						GX_DF_CLAMP,	// diffuse function
						GX_AF_SPOT );	// atten   function
						
		GXSetChanAmbColor( GX_COLOR0A0, FGC_gxBlack );
		GXSetChanMatColor( GX_COLOR0A0, FGC_gxWhite );
	}
	if (nNumShadows > 1)
	{
		GXSetChanCtrl(	GX_ALPHA0,
						GX_ENABLE,		// enable Channel
						GX_SRC_REG,		// amb source
						GX_SRC_REG,		// mat source
						GX_LIGHT1,		// light mask
						GX_DF_CLAMP,	// diffuse function
						GX_AF_SPOT );	// atten   function
	}
	if (nNumShadows > 2)
	{
		GXSetChanCtrl(	GX_COLOR1,
						GX_ENABLE,		// enable Channel
						GX_SRC_REG,		// amb source
						GX_SRC_REG,		// mat source
						GX_LIGHT2,		// light mask
						GX_DF_CLAMP,	// diffuse function
						GX_AF_SPOT );	// atten   function
						
		GXSetChanAmbColor( GX_COLOR1A1, FGC_gxBlack );
		GXSetChanMatColor( GX_COLOR1A1, FGC_gxWhite );
	}
	if (nNumShadows > 3)
	{
		GXSetChanCtrl(	GX_ALPHA1,
						GX_ENABLE,		// enable Channel
						GX_SRC_REG,		// amb source
						GX_SRC_REG,		// mat source
						GX_LIGHT3,		// light mask
						GX_DF_CLAMP,	// diffuse function
						GX_AF_SPOT );	// atten   function
	}
*/
}

GXLightID _anLightID[]=
{
	GX_LIGHT0,
	GX_LIGHT1,
	GX_LIGHT2,
	GX_LIGHT3
};

void _SetupLight( CFLight *pLight, u32 nLightIdx, GXColor gxClr )
{
	GXLightObj gxLight;
	
	if ( pLight->m_nType == FLIGHT_TYPE_SPOT || pLight->m_nType == FLIGHT_TYPE_OMNI )
	{
		GXInitLightAttn( &gxLight, 1.f, 0.f, 0.f, 
 					FLIGHT_ATTEN_K0,
 					(FLIGHT_ATTEN_K1 * pLight->m_fOOR_WS * 0.25f), 
 					(FLIGHT_ATTEN_K2 * pLight->m_fOOR2_WS * 0.25f) );
 		GXInitLightPos( &gxLight, pLight->m_vPos_VS.x, pLight->m_vPos_VS.y, pLight->m_vPos_VS.z );

		u32 nR = gxClr.r;
		nR <<= 2; FMATH_CLAMP(nR, 0x00, 0xff);
		gxClr.r = gxClr.g = gxClr.b = gxClr.a = nR;
 		GXInitLightColor( &gxLight, gxClr );
	}
	else
	{
		GXInitLightAttn( &gxLight, 1.f, 0.f, 0.f, 
 					FLIGHT_ATTEN_K0, 0, 0 );
 					
 		GXInitLightDir( &gxLight, pLight->m_vUnitDir_VS.x, pLight->m_vUnitDir_VS.y, pLight->m_vUnitDir_VS.z );
 		GXInitLightPos( &gxLight, -pLight->m_vUnitDir_VS.x * 65536.f, -pLight->m_vUnitDir_VS.y * 65536.f, -pLight->m_vUnitDir_VS.z * 65536.f );
 		
 		GXInitLightColor( &gxLight, gxClr );
	}
		
	GXLoadLightObjImm(&gxLight, _anLightID[ nLightIdx ]);		
}


//
// Returns TRUE if the only light casting a shadow is a directional light
//
BOOL fshadow_SetupShadowPass(u32 nPass, BOOL bWorldGeo)
{
	if (!FShadow_bActive) { return FALSE; }

	BOOL bSetStates=FALSE, bReturn = TRUE;
	u32 nStart, nEnd, i, N0, N1, T0, I;
	GXColor LIntens;
	GXTexObj *pShadow;
	CFTexInst *pTexBuffer;
	
	nStart = nPass<<2; nEnd = nStart + 4;
	if (nEnd > nShadowLights) { nEnd = nShadowLights; }
	
	if (nEnd - nStart == 0)
	{
		return bReturn;
	}
	
	FGC_bLazyRegisterSet = TRUE;

	if ( _nCurShaderIdx != nEnd-nStart )
	{
		_nCurShaderIdx = nEnd - nStart;
		bSetStates = TRUE;
		
		_SetupLights( nEnd-nStart );
	}
	
	_SetTexture(&_gxTexKill, GX_TEXMAP0);
	
	for (i=nStart; i<nEnd; i++)
	{
		if ( pShadowLights[i]->pLight->m_nType != FLIGHT_TYPE_DIR )
		{
			bReturn = FALSE;
		}
			
		I = i-nStart;
	
		N0 = (i-nStart)<<1;
		N1 = N0+1;
		
		T0 = N0*3;
		
		if (!pShadowLights[i]->bLOD)
		{
			pTexBuffer = _pShadowBuffers[ pShadowLights[i]->nIdx ];
		}
		
		if (pTexBuffer || pShadowLights[i]->bLOD)
		{
			if (pShadowLights[i]->bLOD)
			{
				pShadow = &_gxAttenMap;
			}
			else
			{
				pShadow = (GXTexObj *)pTexBuffer->GetTexDef()->pTexData->pGCTexObj;
			}
		
			CFVec3A vTarget, vUp, vPos;
			CFVec3 vUp3, vDir3, vPos3;
			
			Mtx lookAtMtx, projMtx, shadowMtx;
			
			f32 fFov = pShadowLights[i]->fHalfAngRad;
			
			if (pShadowLights[i]->bLOD)
			{
				fFov *= 0.5f;
			}
			
			vDir3 = pShadowLights[i]->vViewDir;
			vUp3 = pShadowLights[i]->vViewUp;
			vPos3 = pShadowLights[i]->vViewPos;
			
			vPos.Set(vPos3.x, vPos3.y, vPos3.z);
			
			vUp.Set(-vUp3.x, -vUp3.y, -vUp3.z);
			vTarget.Set(vPos);
			vTarget.x += vDir3.x*10.0f;
			vTarget.y += vDir3.y*10.0f;
			vTarget.z += vDir3.z*10.0f;
			
			_LightLookAt( lookAtMtx, vPos, vUp, vTarget);
			_LightPerspective( projMtx, fFov*2.0f );
			MTXConcat(projMtx, lookAtMtx, shadowMtx);
			
			f32 fI = 0.39f * pShadowLights[i]->pLight->m_ScaledColor.fRed + 0.59f * pShadowLights[i]->pLight->m_ScaledColor.fGreen + 0.11f * pShadowLights[i]->pLight->m_ScaledColor.fBlue;
			fI *= 2.0f;
			
			if (fI > 1.0f) { fI = 1.0f; }
			if (fI < 0.5f) { fI = 0.5f; }
			
			f32 fShadowI = fI * 255.0f * pShadowLights[i]->fShadowFade;
			FMATH_CLAMP(fShadowI, 0.0f, 255.0f);
			
			fShadowI *= pShadowLights[i]->pMesh->GetShadowIntensityAdjust();

			LIntens.r = (u8)( fShadowI );
			LIntens.g = LIntens.b = LIntens.a = LIntens.r;
			
			if (bWorldGeo)
			{
				GXLoadTexMtxImm(shadowMtx, GX_TEXMTX0+T0, GX_MTX3x4);
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+N0), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0+T0);
			}
			else
			{
				_ApplyInvView(shadowMtx);
				GXLoadTexMtxImm(shadowMtx, GX_PTTEXMTX0+T0, GX_MTX3x4);
				GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+N0), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, GX_PTTEXMTX0+T0);
			}
			
			GXInitTexObjWrapMode(pShadow, GX_CLAMP, GX_CLAMP );
			GXInitTexObjFilter( pShadow, GX_LINEAR, GX_LINEAR );
			_SetTexture(pShadow, (GXTexMapID)(GX_TEXMAP1+I) );
			
			GXSetTevKColor((GXTevKColorID)(GX_KCOLOR0+I), LIntens);
			_SetupLight( pShadowLights[i]->pLight, I, LIntens );

			if ( bSetStates )
			{
				GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+N0), (GXTevKColorSel)(GX_TEV_KCSEL_K0+I));
				GXSetTevKAlphaSel((GXTevStageID)(GX_TEVSTAGE0+N0), GX_TEV_KASEL_1);
				GXSetTevDirect( (GXTevStageID)(GX_TEVSTAGE0+N0) );
							
				if (I == 0)
				{		
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+N0), GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+N0), (GXTexCoordID)(GX_TEXCOORD0+N0), (GXTexMapID)(GX_TEXMAP1+I), GX_COLOR0A0 );
				}
				else if (I == 1)
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+N0), GX_CC_ZERO, GX_CC_RASA, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+N0), (GXTexCoordID)(GX_TEXCOORD0+N0), (GXTexMapID)(GX_TEXMAP1+I), GX_COLOR0A0 );
				}
				else if (I == 2)
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+N0), GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+N0), (GXTexCoordID)(GX_TEXCOORD0+N0), (GXTexMapID)(GX_TEXMAP1+I), GX_COLOR1A1 );
				}
				else
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+N0), GX_CC_ZERO, GX_CC_RASA, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+N0), (GXTexCoordID)(GX_TEXCOORD0+N0), (GXTexMapID)(GX_TEXMAP1+I), GX_COLOR1A1 );
				}
				
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+N0), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+N0), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+N0), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				
				Mtx ClipMtx;
				CFVec3 vN;
				CFVec3 vOffs;
	
				vN = vDir3;
				vOffs = pShadowLights[i]->pSphere->m_Pos;
				
				ClipMtx[0][0] = vN.x; ClipMtx[0][1] = vN.y; ClipMtx[0][2] = vN.z; ClipMtx[0][3] = -vOffs.Dot(vN);
				ClipMtx[1][0] = 0; ClipMtx[1][1] = 0; ClipMtx[1][2] = 0; ClipMtx[1][3] = 0.0f;
				ClipMtx[2][0] = ClipMtx[2][1] = ClipMtx[2][2] = 0; ClipMtx[2][3] = 1.0f;
								
				//Setup Clip Plane here, should be through the center of the shadow casters bounding sphere.
				if (bWorldGeo)
				{
					GXLoadTexMtxImm(ClipMtx, GX_TEXMTX1+T0, GX_MTX3x4);
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+N1), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1+T0);
				}
				else
				{
					_ApplyInvView(ClipMtx);
					GXLoadTexMtxImm(ClipMtx, GX_PTTEXMTX1+T0, GX_MTX3x4);
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+N1), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, GX_PTTEXMTX1+T0);
				}
				
				GXSetTevDirect( (GXTevStageID)(GX_TEVSTAGE0+N1) );
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+N1), (GXTexCoordID)(GX_TEXCOORD0+N1), GX_TEXMAP0, GX_COLOR_NULL );
				
				if (I == 0)
				{		
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+N1), GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
				}
				else
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+N1), GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_CPREV);
				}
				
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+N1), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+N1), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+N1), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			}
		}
	}
	
	if ( bSetStates )
	{
		N0 = (nEnd-nStart)<<1;
	
		GXSetTevDirect( (GXTevStageID)(GX_TEVSTAGE0+N0) );		
		GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+N0), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
		GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+N0), GX_CC_ONE, GX_CC_ZERO, GX_CC_CPREV, GX_CC_ZERO);
		GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+N0), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+N0), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
		GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+N0), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		
		GXSetNumTevStages(N0+1);
		GXSetNumTexGens(N0);
		GXSetNumIndStages(0);
	}
	
	return bReturn;
}

void fshadow_EndSubmit()
{
	static BOOL __bLastActive = TRUE;
	u32 i, j, nLOD, nBuf=NUM_SHADOW_BUFFERS;
	
	if (FShadow_bActive != __bLastActive) 
	{ 
		if (!FShadow_bActive)
		{
			for (i=0; i<NUM_SHADOW_BUFFERS; i++)
			{
				ftex_ActivateRenderTarget(_pShadowBuffers[i], FALSE);
			}
			return; 
		}
		__bLastActive = FShadow_bActive;
	}

	if (_nLightSubmit)
	{
		f32 fHighestValue;
		f32 fShadowFade;
		f32 fFade2;
		s32 nHighestIdx = -1;
		nBuf = NUM_SHADOW_BUFFERS;
		
		for (j=0; j<_nLightSubmit; j++)
		{
			_aShadowSubmit[j].bActive = TRUE;
		}
		
		i = 0; nLOD = 0;
		do
		{
			nHighestIdx = -1;
			fHighestValue = -100.0f;
			for (j=0; j<_nLightSubmit; j++)
			{
				if (_aShadowSubmit[j].fValue > fHighestValue && _aShadowSubmit[j].bActive)
				{
					fHighestValue = _aShadowSubmit[j].fValue;
					nHighestIdx = j;
				}
			}
			if (nHighestIdx < 0) { break; }
			
			_aShadowSubmit[nHighestIdx].bActive = FALSE;
			
			for (j=0; j<_aShadowSubmit[nHighestIdx].nNumShadowMeshs; j++)
			{
				if (_aShadowSubmit[nHighestIdx].pLight->m_papShadowMeshInst[j])
				{
					fShadowFade = fshadow_CheckObjDist(_aShadowSubmit[nHighestIdx].pLight->m_papShadowMeshInst[j], fFade2);
					if (i < nBuf)
					{
						if ( fShadowFade > 0.0f )
						{
							aShadowData[i].bActive = TRUE;
							aShadowData[i].pLight = _aShadowSubmit[nHighestIdx].pLight;
							aShadowData[i].pMesh = aShadowData[i].pLight->m_papShadowMeshInst[j];
							aShadowData[i].pSphere = &aShadowData[i].pLight->m_paShadowBoundingSphere[j];
							aShadowData[i].nIdx = i;
							aShadowData[i].fShadowFade = fShadowFade;
							
							aShadowData[i].bLOD = FALSE;
									
							if ( _BuildShadowMtx(&aShadowData[i], i, 0) )
							{
								aShadowData[i].bActive = TRUE;
								aShadowData[i].pLight->m_pShadowTex = NULL;
								_nShadowBufUsed++;
								i++;
							}
						}
					}
					else
					{
						fFade2 = 0.5f;
					}
					if ( fFade2 > 0.0f )
					{
						aShadowLOD[nLOD].pLight = _aShadowSubmit[nHighestIdx].pLight;
						aShadowLOD[nLOD].pMesh = _aShadowSubmit[nHighestIdx].pLight->m_papShadowMeshInst[j];
						aShadowLOD[nLOD].pSphere = &_aShadowSubmit[nHighestIdx].pLight->m_paShadowBoundingSphere[j];
						aShadowLOD[nLOD].nIdx = nLOD;
						aShadowLOD[nLOD].fShadowFade = fFade2*2.0f;
						
						aShadowLOD[nLOD].bLOD = TRUE;
						
						if (aShadowLOD[nLOD].pLight)
						{
							if ( _BuildShadowMtx(&aShadowLOD[nLOD], nLOD, 1) )
							{
								aShadowLOD[nLOD].bActive = TRUE;
								aShadowLOD[nLOD].pLight->m_pShadowTex = NULL;
								_nShadowLOD++;
								nLOD++;
							}
						}
					}
				}
			}
		} while (i < nBuf);
	}
	
	for (i=0; i<_nShadowBufUsed; i++)
	{
		ftex_ActivateRenderTarget(_pShadowBuffers[i], aShadowData[i].bActive);
	}
	for (i=_nShadowBufUsed; i<NUM_SHADOW_BUFFERS; i++)
	{
		ftex_ActivateRenderTarget(_pShadowBuffers[i], FALSE);
	}
	
	_nLightSubmit = 0;
}

BOOL FShadow_WorldCallbackFunc( FWorldEvent_e nEvent )
{
	switch (nEvent)
	{
		case FWORLD_EVENT_WORLD_PRELOAD:
			bSetupShadowBuffers = TRUE;
			break;
		case FWORLD_EVENT_WORLD_POSTDESTROY:
			break;
		case FWORLD_EVENT_WORLD_POSTLOAD:
			if (bSetupShadowBuffers)
			{
				fshadow_ResetShadowBuffers();
				bSetupShadowBuffers = FALSE;
			}
			break;
		case FWORLD_EVENT_WORLD_PREDESTROY:
			break;
	};
	return TRUE;
}

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent ) {
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FGCVID_EVENT_COUNT );

	switch( nEvent ) {
	case FGCVID_EVENT_WINDOW_CREATED:
		_bWindowCreated = TRUE;

		fshadow_CreateShadowBuffers();
		fworld_RegisterWorldCallbackFunction( FShadow_WorldCallbackFunc );
		break;

	case FGCVID_EVENT_WINDOW_DESTROYED:
		_bWindowCreated = FALSE;

		fworld_UnregisterWorldCallbackFunction( FShadow_WorldCallbackFunc );
		break;

	case FGCVID_EVENT_PRE_RESET:
		break;

	case FGCVID_EVENT_POST_RESET:
		break;
	}

	return TRUE;
}
