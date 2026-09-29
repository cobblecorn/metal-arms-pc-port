//////////////////////////////////////////////////////////////////////////////////////
// fdx8shadow.cpp - 
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
// 07/10/01 Ranck       Created.
// 02/03/03 Chernobieff Took ownership.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdx8shadow.h"
#include "fshadow.h"
#include "fdx8vid.h"
#include "fdx8mesh.h"
#include "fdx8xfm.h"
#include "fdx8viewport.h"
#include "fsh.h"
#include "fcamera.h"
#include "fRenderSort.h"
#include "frenderer.h"
#include "fdx8tex.h"

#include "fvis.h"
#include "fdraw.h"

#include "fworld.h"

#include "fdx8vshader_const.h"

//shadow pixel shaders
#if FANG_PLATFORM_XB
#include "CompiledPShaderfxbPShadow1.h"
#include "CompiledPShaderfxbPShadow2.h"
#include "CompiledPShaderfxbPShadow3.h"
#include "CompiledPShaderfxbPShadow4.h"
#else
#include "CompiledPShaderfdx8PShadow1.h"
#include "CompiledPShaderfdx8PShadow2.h"
#include "CompiledPShaderfdx8PShadow3.h"
#include "CompiledPShaderfdx8PShadow4.h"
#endif

//shadow vertex shaders
#if FANG_PLATFORM_XB
#include "CompiledVShaderfxbVShadows1.h"
#include "CompiledVShaderfxbVShadows2.h"
#include "CompiledVShaderfxbVShadows3.h"
#include "CompiledVShaderfxbVShadows4.h"
#else
#include "CompiledVShaderfdx8VShadows1.h"
#include "CompiledVShaderfdx8VShadows2.h"
#include "CompiledVShaderfdx8VShadows3.h"
#include "CompiledVShaderfdx8VShadows4.h"
#endif

static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );

static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

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
static u32  _nNextMap=0;

enum
{
	NUM_512=1,
	NUM_256=2,
	NUM_128=8,
	NUM_SHADOW_BUFFERS=NUM_512+NUM_256+NUM_128,
};

#include <stdio.h>

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

CFTexInst *_pShadowBuffers[NUM_SHADOW_BUFFERS];
//Store DX interfaces so I can directly release them
LPDIRECT3DTEXTURE8 _pDXTexResource[NUM_SHADOW_BUFFERS];
LPDIRECT3DSURFACE8 _pDXSurfResource[3];
//
Shadow_Buffer_Data_t aShadowData[NUM_SHADOW_BUFFERS];
Shadow_Buffer_Data_t aShadowLOD[100];
Shadow_Light_t _aShadowSubmit[100];
u32 _nLightSubmit=0;
u32 _nShadowBufUsed=0;
u32 _nShadowLOD=0;

u32 fdx8shadow_nNumVertexShaders=4;
u32 fdx8shadow_nNumPixelShaders=4;

u32 fdx8shadow_anVShader_Handle[4];
u32 fdx8shadow_anPShader_Handle[4];

BOOL FShadow_bActive=TRUE;

u32 fdx8shadow_VShaderDecl[] =
{
	D3DVSD_STREAM(0),
	D3DVSD_REG(0, D3DVSDT_FLOAT3),    //Pos(xyz)
	D3DVSD_REG(1, D3DVSDT_FLOAT3),    //Normal(xyz)
	D3DVSD_END()
};

#if FANG_PLATFORM_WIN
u32 *fdx8shadow_apVShaderFunc[] =
{
	(u32*)dwFdx8VShadows1VertexShader, //0
	(u32*)dwFdx8VShadows2VertexShader, //1
	(u32*)dwFdx8VShadows3VertexShader, //2
	(u32*)dwFdx8VShadows4VertexShader  //3
};
#else
u32 *fdx8shadow_apVShaderFunc[] =
{
	(u32*)dwCompiledVShaderfxbVShadows1VertexShader, //0
	(u32*)dwCompiledVShaderfxbVShadows2VertexShader, //1
	(u32*)dwCompiledVShaderfxbVShadows3VertexShader, //2
	(u32*)dwCompiledVShaderfxbVShadows4VertexShader  //3
};
#endif

#if FANG_PLATFORM_WIN
u32 *fdx8shadow_apPShaderFunc[] =
{
	(u32*)dwFdx8PShadow1PixelShader,
	(u32*)dwFdx8PShadow2PixelShader,
	(u32*)dwFdx8PShadow3PixelShader,
	(u32*)dwFdx8PShadow4PixelShader
};
#else
D3DPIXELSHADERDEF *fdx8shadow_apPShaderFunc[] =
{
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPShadow1PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPShadow2PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPShadow3PixelShader,
	(D3DPIXELSHADERDEF*)dwCompiledPShaderfxbPShadow4PixelShader
};
#endif

void fshadow_ShadowCallback(void *pUserData);
f32 _CheckRangeFromCurCamera(CFMeshInst *pMesh);

BOOL fdx8shadow_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );
	_bModuleInitialized = TRUE;

	memset(_pShadowBuffers, 0, 4*NUM_SHADOW_BUFFERS);

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	return TRUE;
}

void fdx8shadow_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}

BOOL bSetupShadowBuffers=FALSE;
void fshadow_ResetShadowBuffers();
void fshadow_CreateShadowBuffers();

void fdx8shadow_SetupVertexShaders()
{
	u32 i;
	for (i=0; i<fdx8shadow_nNumVertexShaders; i++)
	{
		#if FANG_PLATFORM_XB
			if (FAILED(FDX8_pDev->CreateVertexShader((const DWORD *)fdx8shadow_VShaderDecl, (const DWORD *)fdx8shadow_apVShaderFunc[i], (DWORD *)&fdx8shadow_anVShader_Handle[i], D3DUSAGE_PERSISTENTDIFFUSE)))
			{
				fang_DevPrintf( "fdx8shadow::_SetupVertexShaders() - CreateVertexShader Error Shader Index: %d.\n", i );
			}
		#else
			if (FAILED(FDX8_pDev->CreateVertexShader((const DWORD *)fdx8shadow_VShaderDecl, (const DWORD *)fdx8shadow_apVShaderFunc[i], (DWORD *)&fdx8shadow_anVShader_Handle[i], 0)))
			{
				fang_DevPrintf( "fdx8shadow::_SetupVertexShaders() - CreateVertexShader Error Shader Index: %d.\n", i );
			}
		#endif
	}
}

void fdx8shadow_DeleteVertexShaders()
{
	u32 i;
	for (i=0; i<fdx8shadow_nNumVertexShaders; i++)
	{
		FDX8_pDev->DeleteVertexShader(fdx8shadow_anVShader_Handle[i]);
	}
}

void fdx8shadow_SetupPixelShaders()
{
	u32 i;
	for (i=0; i<fdx8shadow_nNumPixelShaders; i++)
	{
	#if FANG_PLATFORM_WIN
		if (FAILED(FDX8_pDev->CreatePixelShader((const DWORD *)fdx8shadow_apPShaderFunc[i], (DWORD *)&fdx8shadow_anPShader_Handle[i])))
		{
			fang_DevPrintf( "fdx8shadow::_SetupPixelShaders() - CreatePixelShader Error Shader Index: %d.\n", i );
		}
	#else
		if (FAILED(FDX8_pDev->CreatePixelShader((const D3DPIXELSHADERDEF *)fdx8shadow_apPShaderFunc[i], (DWORD *)&fdx8shadow_anPShader_Handle[i])))
		{
			fang_DevPrintf( "fdx8shadow::_SetupPixelShaders() - CreatePixelShader Error Shader Index: %d.\n", i );
		}
	#endif
	}
}

void fdx8shadow_DeletePixelShaders()
{
	u32 i;
	for (i=0; i<fdx8shadow_nNumVertexShaders; i++)
	{
		FDX8_pDev->DeletePixelShader(fdx8shadow_anPShader_Handle[i]);
	}
}

void fshadow_DrawTest()
{
	CFVec3 quad[4];
	CFColorRGBA color(1.0f, 1.0f, 1.0f, 1.0f);

	quad[0].Set(-29.0f, 10.0f, -400.0f);

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

	//quad[0].Set(-29.0f, 10.0f, -400.0f+10.0f);
	quad[0].Set(33.0f, 3.0f, -10.0f);

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

	quad[0].Set(33.0f, 3.0f, -10.0f);

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

	quad[0].Set(43.0f, 3.0f, -10.0f);

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

	quad[0].Set(53.0f, 3.0f, -10.0f);

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

	quad[0].Set(63.0f, 3.0f, -10.0f);

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

extern D3DXMATRIX _matProjectionSaved;
extern CFLight *FMesh_CurrentLight;
BOOL bPrevRenderFlags;

#if FANG_PLATFORM_XB
D3DRECT _rect;
#endif

void fshadow_ShadowCallback(void *pUserData)
{
	if (_nShadowBufUsed < 1) return;

	CFVec3 vLookAt, vPos, vDir, vUp(0,1,0);
	u32 nShadowIdx = (u32)pUserData;
	f32 fZFar=500.0f;

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
#if FANG_PLATFORM_XB
					_rect.x1 = 2; _rect.y1 = 2;
					_rect.x2 = _pShadowBuffers[nShadowIdx]->GetTexDef()->TexInfo.nTexelsAcross-3;
					_rect.y2 = _pShadowBuffers[nShadowIdx]->GetTexDef()->TexInfo.nTexelsDown-3;

					FDX8_pDev->SetScissors(1, FALSE, &_rect);
#endif

					frenderer_Push( FRENDERER_MESH );
					aShadowData[nShadowIdx].pMesh->Draw( FVIEWPORT_PLANESMASK_NONE, TRUE );
					frenderer_Pop();
				}
				break;
		}
		FMesh_bRenderShadows = FALSE;
		FMesh_CurrentLight = NULL;
	}
}

// Shadow buffer sizes. The console-era buffers were 256 (the "512" pool too) and 128 texels; the PC
// renders them at four and two times that for sharper character shadows. Each pool's shared depth
// buffer is created at the same size, and render-target viewports follow the texture size.
#if FANG_WINGC
	#define _PORT_SHADOW_512	1024
	#define _PORT_SHADOW_256	512
	#define _PORT_SHADOW_128	256
#else
	#define _PORT_SHADOW_512	256
	#define _PORT_SHADOW_256	256
	#define _PORT_SHADOW_128	128
#endif

void fshadow_CreateShadowBuffers()
{
	int i, n=0;
	char szName[16];
	FTexData_t *pTexDef;

	ftex_CreateSharedDepthBuffer(16, _PORT_SHADOW_512, _PORT_SHADOW_512);
	for (i=0; i<NUM_512; i++, n++)
	{
		sprintf(szName, "SBuf_%d", n);
		_pShadowBuffers[n] = ftex_CreateRenderTarget_FullScreen(FTEX_RENDERTARGET_FMT_C16_D16, szName, FALSE, FRES_NULLHANDLE, NULL, _PORT_SHADOW_512, _PORT_SHADOW_512, FTEX_2D, FALSE);
         
		_pShadowBuffers[n]->SetFlag(CFTexInst::FLAG_NORELEASE);

		if (!_pShadowBuffers[n]) return;

		//Store DX interfaces so I can release them later.
		pTexDef = _pShadowBuffers[n]->GetTexDef()->pTexData;
		_pDXTexResource[n] = pTexDef->pD3DTexture;

		if (i == 0)
		{
			_pDXSurfResource[0] = pTexDef->pD3DDepthStencil;
		}
	}

	ftex_CreateSharedDepthBuffer(16, _PORT_SHADOW_256, _PORT_SHADOW_256);
	for (i=0; i<NUM_256; i++, n++)
	{
		sprintf(szName, "SBuf_%d", n);
		_pShadowBuffers[n] = ftex_CreateRenderTarget_FullScreen(FTEX_RENDERTARGET_FMT_C16_D16, szName, FALSE, FRES_NULLHANDLE, NULL, _PORT_SHADOW_256, _PORT_SHADOW_256, FTEX_2D, FALSE);

		_pShadowBuffers[n]->SetFlag(CFTexInst::FLAG_NORELEASE);

		//Store DX interfaces so I can release them later.
		pTexDef = _pShadowBuffers[n]->GetTexDef()->pTexData;
		_pDXTexResource[n] = pTexDef->pD3DTexture;

		if (i == 0)
		{
			_pDXSurfResource[1] = pTexDef->pD3DDepthStencil;
		}
	}
	
	ftex_CreateSharedDepthBuffer(16, _PORT_SHADOW_128, _PORT_SHADOW_128);
	for (i=0; i<NUM_128; i++, n++)
	{
		sprintf(szName, "SBuf_%d", n);
		_pShadowBuffers[n] = ftex_CreateRenderTarget_FullScreen(FTEX_RENDERTARGET_FMT_C16_D16, szName, FALSE, FRES_NULLHANDLE, NULL, _PORT_SHADOW_128, _PORT_SHADOW_128, FTEX_2D, FALSE);

		_pShadowBuffers[n]->SetFlag(CFTexInst::FLAG_NORELEASE);

		//Store DX interfaces so I can release them later.
		pTexDef = _pShadowBuffers[n]->GetTexDef()->pTexData;
		_pDXTexResource[n] = pTexDef->pD3DTexture;

		if (i == 0)
		{
			_pDXSurfResource[2] = pTexDef->pD3DDepthStencil;
		}
	}

	ftex_ClearSharedDepthBuffer();
}

void fshadow_DeleteShadowBuffers()
{
	s32 i, n=0;
	if (!_pShadowBuffers[n]) return;

	//Release DX interfaces, application memory is taken care of by the heap system.
	for (i=0; i<NUM_SHADOW_BUFFERS; i++)
	{
		if (_pDXTexResource[i])
		{
			_pDXTexResource[i]->Release();
		}
	}

	for (i=0; i<3; i++)
	{
		if (_pDXSurfResource[i])
		{
			_pDXSurfResource[i]->Release();
		}
	}
}

void fshadow_ClearLightList()
{
	_nNextMap=0;

	s32 nCamIdx = fcamera_GetLastCameraIndex();

	CFCamera *pCamera;
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
		ftex_AddRenderTarget(_pShadowBuffers[i], fshadow_ShadowCallback, TRUE, 0, FALSE, TRUE, (void*)i, FALSE, TRUE);
	}

	fshadow_ClearLightList();
	fshadow_ClearVolumeList();
}

f32 _CalcLightValue(CFLight *pLight)
{
	if (pLight->m_nType == FLIGHT_TYPE_DIR)
	{
		return 1000.0f;
	}

	CFVec3 vOffs;
	f32 fD2;
	f32 fR2;
	
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

//
//
//
void fshadow_SubmitLight( CFLight *pLight, CFMeshInst *pMesh )
{
	//Only these light types can cast shadows. Ambient lights cannot.
	if (pLight->m_nType == FLIGHT_TYPE_DIR || pLight->m_nType == FLIGHT_TYPE_OMNI || pLight->m_nType == FLIGHT_TYPE_SPOT)
	{
		Shadow_Light_t *pSubmit = &_aShadowSubmit[_nLightSubmit];
		
		if ( fshadow_LightCastShadow(pLight) )
		{
			pSubmit->nNumShadowMeshs = fvis_GetNumShadows( pLight->m_nIndex );
			if (pSubmit->nNumShadowMeshs > 0)
			{
				pSubmit->fValue = _CalcLightValue( pLight );
				pSubmit->pLight = pLight;
				_nLightSubmit++;
			}
		}
	}
}

#define _DIR_DIST 1000.0f;

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
		pData->vViewPos.x = vPos.x - pData->pLight->m_mtxOrientation_WS.m_vFront.x*_DIR_DIST;
		pData->vViewPos.y = vPos.y - pData->pLight->m_mtxOrientation_WS.m_vFront.y*_DIR_DIST;
		pData->vViewPos.z = vPos.z - pData->pLight->m_mtxOrientation_WS.m_vFront.z*_DIR_DIST;
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
	if (nVolumeLightCount < 1) return;

	if (_nVolumeCount >= MAX_VOLUMES) return; 

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
					nBufIdx = _FindShadowBuffer_LOD(aShadowLOD[0].pLight, nMeshIdx);
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
		nRet += ((nPasses-1)>>1);
		nRet++;
	}

	FASSERT( ((s32)nPasses>0&&(s32)nRet>0) || nShadowLights < 1 );
			
	return (nRet);
}

extern void _LightLookAt(D3DXMATRIX *pLookAt, CFVec3A& camPos, CFVec3A& up, CFVec3A& target);
extern void _LightPerspective(D3DXMATRIX *pLookAt, float fov);
extern void _ApplyInvView(D3DXMATRIX *pLookAt);
extern void fdx8tex_SetTexture( u32 nStageNum, CFTexInst *pTexInst, u32 nTCIndex );

extern LPDIRECT3DTEXTURE8 _pAttenMap;
extern LPDIRECT3DTEXTURE8 _pTexKill;

u32 _nCurShaderIdx = 0xff;

void fshadow_BeginShadowRender()
{
	fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
	fdx8_SetRenderState_SRCBLEND(D3DBLEND_ZERO);
	fdx8_SetRenderState_DESTBLEND(D3DBLEND_SRCCOLOR);
	fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);
	
	fdx8_SetRenderState_ALPHATESTENABLE(FALSE);
	
	fdx8_SetRenderState_ZWRITEENABLE( FALSE );
	fdx8_SetRenderState_ZFUNC( D3DCMP_EQUAL );

	fsh_ResetShaderCache();

	FDX8_pDev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU|0);
	FDX8_pDev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU|1);
	FDX8_pDev->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU|2);
	FDX8_pDev->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU|3);

	FDX8_pDev->SetVertexShaderConstant(CV_LARGE, &D3DXVECTOR4(256, 256, 256, 256), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_ZERO, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_ONE, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);

	//Setup texture clamping/border for shadows
	u32 i;
	for (i=0; i<4; i++)
	{
		#if FANG_PLATFORM_XB	//XBox uses a scissor box to avoid leakage, so all I do here is CLAMP
			ftex_SetTexAddress(i, FALSE, FALSE);
		#else	//DX8.1 on PC doesn't have scissor box cap (DX9 does, finally) so we must use the border color which limits compatibility.
			//Special mode, fake it by saying clamp.
			ftex_SetTexAddress(i, FALSE, FALSE);

			FDX8_pDev->SetTextureStageState(i, D3DTSS_BORDERCOLOR, 0x0);
            FDX8_pDev->SetTextureStageState(i, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
			FDX8_pDev->SetTextureStageState(i, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		#endif
	}

	_nCurShaderIdx = 0xff;
}

void fshadow_EndShadowRender()
{
	fdx8shadow_ClearTextures();

	#if !FANG_PLATFORM_XB
		u32 i;
		for (i=0; i<4; i++)
		{
			FDX8_pDev->SetTextureStageState(i, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			FDX8_pDev->SetTextureStageState(i, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		}
	#endif
}

BOOL fshadow_SetupShadowPass(u32 nPass, BOOL bWorldGeo)
{
	BOOL bReturn = TRUE;
	u32 nStart, nEnd, i;
	s32 nShaderIdx;
	CFTexInst *pTexBuffer;
	
	nStart = nPass<<1; nEnd = nStart + 2;

	if (nEnd > nShadowLights) 
	{ 
		nEnd = nShadowLights; 
	}
	
	if (nEnd - nStart == 0)
	{
		return bReturn;
	}
	nShaderIdx = nEnd-nStart-1;
	if (nShaderIdx < 0) { nShaderIdx = 0; }
	if (nShaderIdx > 1) { nShaderIdx = 1; }

	HRESULT hr;
	if (nShaderIdx != _nCurShaderIdx)
	{
		FDX8_SetVertexShader(fdx8shadow_anVShader_Handle[nShaderIdx]);
		if ( FAILED(hr = FDX8_pDev->SetPixelShader(fdx8shadow_anPShader_Handle[nShaderIdx])) )
		{
			fang_DevPrintf( "fdx8shadow::SetPixelShader() - FAILED: %d\n", hr );
		}
		_nCurShaderIdx = nShaderIdx;
	}

	ftex_SetTexAddress((nEnd-nStart), FALSE, FALSE);
	ftex_SetTexAddress((nEnd-nStart)+1, FALSE, FALSE);

	for (i=nStart; i<nEnd; i++)
	{
		if ( pShadowLights[i]->pLight->m_nType != FLIGHT_TYPE_DIR )
		{
			bReturn = FALSE;
		}
			
		// LOD lights use the attenuation map; don't read an unset (or previous light's) buffer.
		pTexBuffer = NULL;
		if (!pShadowLights[i]->bLOD)
		{
			pTexBuffer = _pShadowBuffers[ pShadowLights[i]->nIdx ];
		}

		if (pTexBuffer || pShadowLights[i]->bLOD)
		{
			if (pShadowLights[i]->bLOD)
			{
				FDX8_pDev->SetTexture(i-nStart, _pAttenMap);
			}
			else
			{
				FDX8_pDev->SetTexture(i-nStart, (LPDIRECT3DTEXTURE8)pTexBuffer->GetTexDef()->pTexData->pD3DTexture);
			}

			CFVec3A vTarget, vUp, vPos;
			CFVec3 vUp3, vDir3, vPos3;
			
			D3DXMATRIX lookAtMtx, projMtx, shadowMtx, out;
			
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
			
			_LightLookAt( &lookAtMtx, vPos, vUp, vTarget);
			_LightPerspective( &projMtx, fFov*2.0f );
			D3DXMatrixMultiply(&out, &lookAtMtx, &projMtx);

			D3DXMatrixTranspose(&shadowMtx, &out);

			switch (i-nStart)
			{
				case 0:
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX0_0, &shadowMtx(0,0), 4);
				break;
				case 1:
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX1_0, &shadowMtx(0,0), 4);
				break;
				default:
					fang_DevPrintf( "Invalid shadow stage!\n" );
				break;
			}
						
			f32 fI = 0.39f * pShadowLights[i]->pLight->m_ScaledColor.fRed + 0.59f * pShadowLights[i]->pLight->m_ScaledColor.fGreen + 0.11f * pShadowLights[i]->pLight->m_ScaledColor.fBlue;
			if (fI > 1.0f) { fI = 1.0f; }
			if (fI < 0.5f) { fI = 0.5f; }

			f32 fShadowI = fI * pShadowLights[i]->fShadowFade;
			
			if (fShadowI < 0.0f) 
			{
				fShadowI = 0.0f; 
			}
			if (fShadowI > 0.5f) 
			{ 
				fShadowI = 0.5f; 
			}

			fShadowI *= pShadowLights[i]->pMesh->GetShadowIntensityAdjust();

			f32 afData[4];
			afData[0] = afData[1] = afData[2] = afData[3] = fShadowI;
			FDX8_pDev->SetPixelShaderConstant(i-nStart, afData, 1);

			f32 fD;

			if (pShadowLights[i]->pLight->m_nType == FLIGHT_TYPE_SPOT)
			{
				FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIRECTION+(i-nStart), &D3DXVECTOR4(pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront.x, pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront.y, pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront.z, 1), 1);

				//Get the worldspace center.
				fD = -pShadowLights[i]->pSphere->m_Pos.Dot(pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront);

				FDX8_pDev->SetVertexShaderConstant(CV_PLANE1+(i-nStart), &D3DXVECTOR4(pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront.x, pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront.y, pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront.z, fD), 1);

				CFVec3 end = pShadowLights[i]->pSphere->m_Pos;
				end = end + (pShadowLights[i]->pLight->m_mtxOrientation_WS.m_vFront*50.0f);
#if !FANG_PRODUCTION_BUILD
				fdraw_DevSphere(&pShadowLights[i]->pSphere->m_Pos, 10.0f, &FColor_MotifBlue, 2, 2, 2);
				fdraw_DevLine(&pShadowLights[i]->pSphere->m_Pos, &end, &FColor_MotifRed, &FColor_MotifGreen);
#endif
			}
			else
			{
				FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIRECTION+(i-nStart), &D3DXVECTOR4(-pShadowLights[i]->vViewDir.x, -pShadowLights[i]->vViewDir.y, -pShadowLights[i]->vViewDir.z, 1), 1);
				fD = -pShadowLights[i]->pSphere->m_Pos.Dot(pShadowLights[i]->vViewDir);

				FDX8_pDev->SetVertexShaderConstant(CV_PLANE1+(i-nStart), &D3DXVECTOR4(pShadowLights[i]->vViewDir.x, pShadowLights[i]->vViewDir.y, pShadowLights[i]->vViewDir.z, fD), 1);
			}

			if (pShadowLights[i]->pLight->m_nType == FLIGHT_TYPE_DIR)
			{
				FDX8_pDev->SetVertexShaderConstant(CV_LIGHT5_POSITION+(i-nStart), &D3DXVECTOR4(pShadowLights[i]->vViewPos.x, pShadowLights[i]->vViewPos.y, pShadowLights[i]->vViewPos.z, 0.00000004f), 1);
			}
			else
			{
				FDX8_pDev->SetVertexShaderConstant(CV_LIGHT5_POSITION+(i-nStart), &D3DXVECTOR4(pShadowLights[i]->vViewPos.x, pShadowLights[i]->vViewPos.y, pShadowLights[i]->vViewPos.z, pShadowLights[i]->pLight->m_fOOR2_WS*0.75f), 1);
			}

			f32 fSpot0, fSpot1;

			if (pShadowLights[i]->pLight->m_nType == FLIGHT_TYPE_SPOT)
			{
				fSpot0 = pShadowLights[i]->pLight->m_fSpotK2;
				fSpot1 = -pShadowLights[i]->pLight->m_fSpotK1 * fSpot0;
			}
			else
			{
				fSpot0 = -1.0f; 
				fSpot1 =  0.0f;
			}
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE+(i-nStart), &D3DXVECTOR4(1.0f, fSpot1, fSpot0, 0.0f), 1);
		}
	}

	return bReturn;
}

void fdx8shadow_ClearTextures()
{
	fdx8tex_SetTexture( 0, NULL, 0 );
	fdx8tex_SetTexture( 1, NULL, 1 );
	fdx8tex_SetTexture( 2, NULL, 2 );
	fdx8tex_SetTexture( 3, NULL, 3 );
}

void fshadow_EndSubmit()
{
	u32 i, j, nLOD, nBuf=NUM_SHADOW_BUFFERS;
	
	if (_nLightSubmit)
	{
		f32 fHighestValue;
		f32 fShadowFade;
		f32 fFade2;
		s32 nHighestIdx = -1;
		nBuf = NUM_SHADOW_BUFFERS;
		
		for (j=0; j<_nLightSubmit; j++)
		{
			if (_aShadowSubmit[j].pLight)
			{
				_aShadowSubmit[j].bActive = TRUE;
			}
			else
			{
				_aShadowSubmit[j].bActive = FALSE;
			}
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
					//fFade2 = 0.0f;
					if (i < nBuf)
					{
						if ( fShadowFade > 0.0f )
						{
							aShadowData[i].pLight = _aShadowSubmit[nHighestIdx].pLight;
							if (aShadowData[i].pLight)
							{
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
					}
					else
					{
						fFade2 = 0.5f;
					}

					if ( fFade2 > 0.0f )
					{
						if (_aShadowSubmit[nHighestIdx].pLight)
						{
							aShadowLOD[nLOD].pLight = _aShadowSubmit[nHighestIdx].pLight;
							aShadowLOD[nLOD].pMesh = _aShadowSubmit[nHighestIdx].pLight->m_papShadowMeshInst[j];
							aShadowLOD[nLOD].pSphere = &_aShadowSubmit[nHighestIdx].pLight->m_paShadowBoundingSphere[j];
							aShadowLOD[nLOD].nIdx = nLOD;
							aShadowLOD[nLOD].fShadowFade = fFade2*2.0f;
							
							aShadowLOD[nLOD].bLOD = TRUE;
									
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

static BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) {
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) {
	case FDX8VID_EVENT_WINDOW_CREATED:
		_bWindowCreated = TRUE;

		fshadow_CreateShadowBuffers();
		fworld_RegisterWorldCallbackFunction( FShadow_WorldCallbackFunc );
		break;

	case FDX8VID_EVENT_WINDOW_DESTROYED:
		_bWindowCreated = FALSE;

		fshadow_DeleteShadowBuffers();
		fworld_UnregisterWorldCallbackFunction( FShadow_WorldCallbackFunc );
		break;

	case FDX8VID_EVENT_PRE_RESET:
		break;

	case FDX8VID_EVENT_POST_RESET:
		break;
	}

	return TRUE;
}
