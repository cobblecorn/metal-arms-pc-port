//////////////////////////////////////////////////////////////////////////////////////
// fdx8sh.cpp - 
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
// 11/02/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fdx8.h"
#include "fdx8sh.h"
#include "fdx8vid.h"
#include "fdx8tex.h"
#include "fdx8xfm.h"
#include "fdx8tex.h"
#include "fdx8vb.h"

#include "fdx8mesh.h"

#include "frendersort.h"
#include "fcolor.h"
#include "fshaders.h"
#include "fperf.h"
#include "flight.h"
#include "frenderer.h"

#include "fliquid.h"

#include "fcamera.h"
#include "fvis.h"
#include "fdx8vshader_const.h"

#include "fdx8shaders.h"			// This file defines all of Fang's shader programs
#include "fdx8shadow.h"

#include <stdio.h>
//

//*********************************************************
//LOCAL TYPEDEFs & ENUMs

#define FSTATIC		static	

//Multipass lighting: allows for per-pixel lighting and bumpmapping ( default = 1 )
#define FSH_MULTIPASS_LIGHTING 1
//SimpleShader: Bypass normal shader processing, pass thru as simple as possible ( default = 0 )
#define FSH_SIMPLE_SHADER 0
//
#define FANG_SURFACE_ONLY 0

#define _USE_GAMEPAD_ 0

#define MAX_SREFLECT_TARGETS 4
#define _MAX_TEX_COORD_MTX_COUNT	FSH_MAX_TEX_LAYER_COUNT
#define _MAX_TEX_OVERRIDE_COUNT		FSH_MAX_TEX_LAYER_COUNT

enum LM_MODE
{
	LM_NONE=0,
	ZMASK_EMASK_LM2,
	ZMASK_LM3,
	EMASK_LM3,
	LM4
};

#if FANG_LIGHTING_2X
	#define DST_BLEND D3DBLEND_SRCCOLOR
#else
	#define DST_BLEND D3DBLEND_ZERO
#endif

#define PX_PASSTHRU PSHADER_PASSTHRU

//MASKS, what masks can this shader have? (see _aShaderRenderStates[] below)
enum Masks
{
	MASK_NONE=0,
	MASK_Z=0x1,
	MASK_S=0x2,
	MASK_E=0x4
};

typedef struct 
{
	u8 nStages;		 //Number of texture stages.
	u8 nTFactorReg;  //which register does it get tfactor from (0xff == no tfactor)
	u8 nAmbientReg;  //(0xff == no ambient reg)
	u8 nEmissiveReg; //(0xff == no emissive reg)
	u8 nDiffuseReg;  //(0xff == no diffuse reg)
	u8 nSpecReg;	 //(0xff == no specular reg)
	u8 nMaterialSource[4]; //ambient, diffuse, emissive, specular sources = { D3DMCS_MATERIAL, D3DMCS_COLOR1 }
	u32 vShader;	 //vertex shader index.
	u32 pShader;	 //pixel shader index.
	u32 nBlendOp;	 //Blend op (usually ADD).
	u32 nSrcBlend;	 //Src Blend 
	u32 nDstBlend;	 //Dest Blend
	BOOL bAlphaTest; //TRUE to enable alphatesting.
	u32 nAlphaFunc;	 //Alpha compare function.
	u32 nAlphaRef;	 //Alpha compare value.
	u8 nMask;		 //Masks to use for this shader: ZMASK, EMASK, SMASK
} FShaderRenderStates_t;

typedef struct 
{
	s32 nTexReg;
} FShaderStageStates_t;

typedef struct
{
	CFVec3 vPos;
	CFVec2 vTex[4];
} RenderPlaneVtx_t;

typedef struct
{
	CFVec3 vPlane;

	RenderPlaneVtx_t QuadSec[4];
	RenderPlaneVtx_t Quad[4];
	u32 nType, nVtx;

	//Volumetric fog - for refractions only.
	CFColorRGB fogClr;
	f32 fFogDensity; //0.0f = no fog.
	u32 d3dClr;
	//
	
	f32 fGeoCullDist;
	f32 fOpacity;
	f32 fTile;
	f32 fRadius;
	
	BOOL bActive;
	
	CFTexInst *pLayer0;
	CFTexInst *pLayer1;

	CFVec3 vCen;
} fsh_Render_Plane_t;

typedef struct
{
	CFTexInst *pTexInst;
	void *pUser;
	f32 fSize;
} fsh_ReflectTarget_t;

//Per-Pixel lighting
typedef struct
{
	D3DXVECTOR3 vPos;
	D3DXVECTOR3 vColor;
	float fOOR;
} PerPixelPointLight_t;

typedef struct
{
	CFVec3A vPos;
	CFMtx43 *pMtx_WS;
	D3DXVECTOR3 vColor;
	f32 fFOV, fRange2;
	LPDIRECT3DTEXTURE8 pDXTex;
} PerPixelSpotLight_t;

typedef struct {					// Used for texture coordinate transformations (like scrolling):
	u32 nTexLayerID;				// Texture layer ID
	const CFMtx43 *pTexCoordMtx;	// Pointer to matrix to be applied
} _TexCoordMtx_t;

typedef struct {					// Used for texture override (like flipping):
	u32 nTexLayerID;				// Texture layer ID
	CFTexInst *pTexInst;			// Texture to be applied
} _TexOverride_t;

typedef struct
{
	u32 nTexLayerID;
	CFTexInst *pCur, *pOrig;
} _TexOverride_Cache_t;

//

//*********************************************************
//VARIABLES
const f32 _fOO255=(1.0f/255.0f);
static u32 _nPassIdx;
static BOOL _bUseTrans;
static u32 *_pnShaderRemap = NULL;
BOOL fdx8sh_bStream1Set = FALSE;

BOOL FSh_bUseFastPass=FALSE;

//Generic identity matrix used in vertex shaders.
static D3DXMATRIX _D3DIdentMtx( 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 );

BOOL FSh_bShadowRender=FALSE;
BOOL FSh_bUseClipPlane=FALSE;
BOOL _bFullScrActive=FALSE;
BOOL _bFastShader = FALSE;
BOOL fsh_bUseExtColorStream=FALSE;
BOOL FSh_ZEnable=TRUE;
BOOL bSetupRenderTargets=FALSE;
BOOL bRenderPlane=TRUE;
BOOL FSh_bSurfaceOnly=FALSE;
BOOL FSh_bAlphaOut=FALSE;

f32 FSh_fPerPixelFade = 1.f;
s32 FSh_ShadowID=0;
u32 _nNumRenderPlanes;
f32 _fCubeReflectHackDelta=0.1f;
fsh_Render_Plane_t _aRenderPlanes[MAX_RENDER_PLANES];

CFVec3A FSh_PlayerPos;
extern f32 _pFOVAdj;

//FSh_RENDER_ALL
//FSh_ShaderFlags_e FSh_shaderFlags=(FSh_ShaderFlags_e)(FSh_RENDER_REFLECTIONMAP|FSh_RENDER_DETAIL|FSh_RENDER_BUMP|
//													  FSh_RENDER_MULTIPASS_LIGHTING|FSh_RENDER_MOTIF_LIGHTMAPS);

FSh_ShaderFlags_e FSh_shaderFlags=(FSh_ShaderFlags_e)(FSh_RENDER_ALL);

//Fog Values
f32 fsh_fFogNearZ=0.0f;
f32 fsh_fFogFarZ=500.0f;
f32 fsh_fFogConst[4];
f32 fsh_fFogMinY=30.0f;
f32 fsh_fFogMaxY=70.0f;
f32 fsh_fFogDensity=1.0f;
CFTexInst *fsh_pFogTex=NULL;
static CFColorRGB fsh_fFogColor2;
static CFColorRGB fsh_fFogColor(0.5f, 0.5f, 0.5f);
//CFTexInst *fsh_pFogTex=NULL;
//

FViewport_t *_pSView=NULL;
D3DXMATRIX _PlaneProjMtx;

u8 m_nLMMode;
u8 m_nLM;

u32 _nNumSReflectTargets=0;
fsh_ReflectTarget_t _aSReflectTargets[MAX_SREFLECT_TARGETS];
CFTexInst *_pFullScrTarget, *_pFullScrLoRes;
LPDIRECT3DCUBETEXTURE8 _pFullScrTarget_D3DTex=NULL;
LPDIRECT3DSURFACE8 _pFullScrTarget_D3DSurf=NULL;

f32 _fLayerAlpha=1.0f;
CFColorRGBA _fSurfaceColor(1.0f, 1.0f, 1.0f, 1.0f);
CFColorRGBA _fFinalSurfaceColor(1.0f, 1.0f, 1.0f, 1.0f);

CFTexInst *_pReflectTex=NULL;

D3DXMATRIX _matProjectionSaved;
CFLight *FSh_pSpecularLight;

static int _nPerPixelPoint=0, _nPerPixelSpot = 0;
PerPixelPointLight_t _aPerPixelPointLights[8];
PerPixelSpotLight_t _aPerPixelSpotLights[4];

LPDIRECT3DTEXTURE8 _pAttenMap=NULL, _pTexKill=NULL;
LPDIRECT3DTEXTURE8 _pSpec16=NULL, _pSpec32=NULL, _pSpec48=NULL;
LPDIRECT3DCUBETEXTURE8 _pNormalizeCube=NULL;

//Configuration Settings for shaders
f32 FSh_fDetailTile=4.0f;
f32 FSh_fBumpTile=1.0f;

BOOL FSh_bInvAlpha_Emissive=FALSE;

static D3DMATRIX _D3DEnvMapMtx;
static BOOL _bRestoreClamp=FALSE;
static u32 _nLightCount;
static u32 _nDepthBias;

static u32 _nTexCoordMtxCount;
static u32 _nPrevTexCoordMtxCount;
static u32 _nTexCoordMtxMaxCount;
static _TexCoordMtx_t _aTexCoordMtx[_MAX_TEX_COORD_MTX_COUNT];

static u32 _nTexOverrideCount;
static u32 _nPrevTexOverrideCount;
static u32 _nTexOverrideMaxCount;
static _TexOverride_t _aTexOverride[_MAX_TEX_OVERRIDE_COUNT];

static BOOL _bFogEnabled;

static u8 _nPointCount, _nDirCount, _nSpotCount, _nPad;

static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;
static BOOL _bShaderSystemOpen;

static u32 _nShaderID, _nSurfaceShaderID;
static u32 _anGlobalRegisters[FSHREG_GLOBAL_COUNT];		// Array of global registers
static u32 _nCurrentShader = 0xffffffff;
static u32 _nCurrentVtxShader = 0xffffffff;
static u32 _nCurrentPixelShader = 0xffffffff;
static _TexOverride_Cache_t _aTexOverrideCache[4];

static BOOL _bSetDetailScale=FALSE;
static u32 _nDetailReg = 0;

static f32 _afPixelConst2[4]={0,0,0,0}, _afPixelConst3[4]={0,0,0,0};

static CFColorMotif _Default_GReg_Ambient;	// Default register value for global register AMBIENT
static CFColorMotif _Default_GReg_Motif00;	// Default register value for global register FSHREG_MOTIF_A0_C0
static CFColorMotif _Default_GReg_Motif01;	// Default register value for global register FSHREG_MOTIF_A0_C1
static CFColorMotif _Default_GReg_Motif10;	// Default register value for global register FSHREG_MOTIF_A1_C0
static CFColorMotif _Default_GReg_Motif11;	// Default register value for global register FSHREG_MOTIF_A1_C1

static BOOL _bVBChanged=FALSE;

//Pixel Shader Info
u32 _nNumPixelShaders=64;
u32 _anPShader_Handle[136];
//Vertex Shader Info.
u32 _nNumVertexShaders=76;
u32 _anVShader_Handle[136]; //4*136 = 544
u32 _nVertexType, _nFixedVShader;
//

//Procedural (for liquid shaders)
void *FSh_pProcedural=NULL;

//******************SHADER DEFINITION VARIABLES**********************
//Shader remapping, see fsh_SetShader()
static u32 _anSurfaceRemap[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72, 73, 74, 75, 76, 77, 78, 79
};

static u32 _anDiffuseRemap[] =
{
	FSHADERS_SHADER_COUNT, FSHADERS_SHADER_COUNT + 1, FSHADERS_SHADER_COUNT + 2,
	FSHADERS_SHADER_COUNT + 3, FSHADERS_SHADER_COUNT + 4, FSHADERS_SHADER_COUNT + 5	
};

static u32 _anSpecularRemap[] =
{
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT, 
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 1,
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 2,
	FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT + 3	
};

//Blending modes, register indexs (for things like envmap motif), vertex/pixel shader indexs, alpha test modes, and masks.
static FShaderRenderStates_t _aShaderRenderStates[] =
{
	//FSHADERS_oBASE: oBASE - 0
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_PASSTHRU, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADER_cBASE: cBASE
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADER_tBASE: tBASE
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_etBASE, tBASE
	{
		1, 0xff, 0x02, 0xff, 0x03, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_tBASE_ADD_SPEC, tBASE
	{
		1, 0xff, 0xff, 0xff, 0x02, 0x03,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_etBASE_ADD_SPEC, tBASE
	{
		1, 0xff, 0x02, 0xff, 0x03, 0x04,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_etBASE_ADD_bSPEC, tBASE
	{
		1, 0xff, 0x02, 0xff, 0x03, 0x04,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_ADD_BASE, ADD_BASE // -7
	{
		1, 0x02, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_tLAYER,// -8 oBASE_LERP_tLAYER
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_tLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_vLAYER, oBASE_LERP_vLAYER
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_vLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_pLAYER, oBASE_LERP_pLAYER - 10
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_pLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_cBASE_LERP_tLAYER, cBASE_LERP_tLAYER 
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_tLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_cBASE_LERP_vLAYER, cBASE_LERP_vLAYER
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_vLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_cBASE_LERP_pLAYER, cBASE_LERP_pLAYER
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_pLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_tBASE_LERP_tLAYER, 
	{
		3, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_tLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER, tBASE_LERP_tLAYER - 15
	{
		2, 0xff, 0xff, 0xff, 0x02, 0x03,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_oBASE_LERP_tLAYER, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_ADD_rbENV, - 
	{
		2, 0x04, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_oBASE_ADD_rbENV, PSHADER_oBASE_ADD_rbENV, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_S
	},
	//FSHADERS_etBASE_ADD_rbENV, - 
	{
		3, 0x04, 0xff, 0x02, 0x03, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		//VSHADER_BASE_oBASE_ADD_rbENV, PSHADER_oBASE_ADD_rbENV, //nstages, vshader, pshader
		VSHADER_BASE_ENV_POINT1_DIR1, PSHADER_oBASE_ADD_rbENV_LIT, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z|MASK_S
	},
	//FSHADERS_oBASE_DETAIL
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_DETAIL, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADER_cBASE_DETAIL
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADER_tBASE_DETAIL
	{
		1, 0xff, 0xff, 0xff, 0x03, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_etBASE_DETAIL
	{
		1, 0xff, 0x02, 0xff, 0x04, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_tBASE_ADD_SPEC_DETAIL
	{
		1, 0xff, 0xff, 0xff, 0x03, 0x03,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_etBASE_ADD_SPEC_DETAIL
	{
		1, 0xff, 0x02, 0xff, 0x04, 0x04,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_etBASE_ADD_bSPEC_DETAIL
	{
		1, 0xff, 0x02, 0xff, 0x04, 0x04,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_ADD_BASE_DETAIL
	{
		2, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_tLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_tLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_vLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_vLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_pLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_pLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_cBASE_LERP_tLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_tLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_cBASE_LERP_vLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_vLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_cBASE_LERP_pLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_pLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_tBASE_LERP_tLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0x03, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_tLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL
	{
		3, 0xff, 0xff, 0xff, 0x03, 0x04,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_DETAIL, PSHADER_oBASE_LERP_tLAYER_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_oBASE_ADD_rbENV_DETAIL
	{
		3, 0x06, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_oBASE_ADD_rbENV_DETAIL, PSHADER_oBASE_ADD_rbENV_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_S
	},
	//FSHADERS_etBASE_ADD_rbENV_DETAIL
	{
		3, 0x06, 0xff, 0x03, 0x04, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_oBASE_ADD_rbENV_DETAIL, PSHADER_oBASE_ADD_rbENV_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z|MASK_S
	},
	//FSHADERS_oBASE_ADD_rbSREFLECT,
	{
		2, 0x04, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_oBASE_ADD_rbSREFLECT, PSHADER_oBASE_ADD_rbENV, //nstages, vshader, pshader
		//D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_S
	},
	//FSHADERS_tBASE_vALPHA
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		//VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU_VALPHA, //nstages, vshader, pshader
		VSHADER_BASE_PASSTHRU, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_LIQUID_ENV,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_REFLECT, PSHADER_CUBE_REFLECT, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_LIQUID_LAYER_ENV,
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_REFLECT, PSHADER_CUBE_REFLECT, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_LIQUID_TEXTURE,
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_REFLECT, PSHADER_CUBE_REFLECT, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_LIQUID_MOLTEN_1LAYER,
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_REFLECT, PSHADER_CUBE_REFLECT, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_LIQUID_MOLTEN_2LAYER,
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_REFLECT, PSHADER_CUBE_REFLECT, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	{
		3, 0x05, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU_ENV, PSHADER_oBASE_LERP_tLAYER_ADD_rbENV, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	{
		3, 0x05, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU_ENV, PSHADER_oBASE_LERP_vLAYER_ADD_rbENV, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
	{
		3, 0x05, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU_ENV, PSHADER_oBASE_LERP_pLAYER_ADD_rbENV, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	{
		4, 0x07, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU_ENV, PSHADER_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	{
		4, 0x07, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU_ENV, PSHADER_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
	{
		4, 0x07, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU_ENV, PSHADER_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADER_pBASE: pBASE
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU_VTALPHA, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_epBASE, pBASE
	{
		1, 0xff, 0x02, 0xff, 0x03, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU_VTALPHA, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_pBASE_ADD_SPEC, pBASE
	{
		1, 0xff, 0xff, 0xff, 0x02, 0x03,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU_VTALPHA, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_epBASE_ADD_SPEC, pBASE
	{
		1, 0xff, 0x02, 0xff, 0x03, 0x04,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU_VTALPHA, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_epBASE_ADD_bSPEC, pBASE
	{
		1, 0xff, 0x02, 0xff, 0x03, 0x04,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU_VTALPHA, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_Z
	},
	//FSHADERS_ADD_vBASE, ADD_BASE // -7
	{
		1, 0x02, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_PASSTHRU_VAMUL, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_LIGHT_MORPH
	{
		2, 0x04, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_PASSTHRU, PSHADER_PASSTHRU, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, DST_BLEND, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_DECAL_AI,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_DECALTEX_AI, - 20
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_DECALTEX_AT,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_DIFFUSETEX_AI,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_DIFFUSETEX_AT,
	{	
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_DIFFUSETEX_AIAT,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_SPECULARTEX_AT, - 25
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_ADD,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_BLEND_AIPLUSAT,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_CT_PLUS_CIAT_AI,
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		0, 0, //nstages, tfactor reg, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADER_otBASE: tBASE with alpha from SurfaceColor only.
	{
		1, 0xff, 0xff, 0xff, 0x02, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_PASSTHRU_SALPHA, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_FULLBRIGHT
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_VCOLOR, PSHADER_COLOR, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_FULLBRIGHT_CO - 30
	{
		1, 0xff, 0xff, 0xff, 0xff, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_COLOR_MASK, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_VLIGHT_BASIC, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		1, 0xff, 0xff, 0x00, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_COLOR, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_NONE
	},
	//FSHADERS_VLIGHT_BASIC_CO, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		1, 0xff, 0xff, 0x00, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_COLOR_MASK, //vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
#if FANG_PLATFORM_XB
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
#else
		TRUE, D3DCMP_GREATER, 0x01, //alphatest, alpha func, alpharef
#endif
		MASK_Z
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		1, 0x00, 0xff, 0x00, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_COLOR_EMASK, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
		FALSE, D3DCMP_ALWAYS, 0x00, //alphatest, alpha func, alpharef
		MASK_E
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE_CO, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		1, 0x00, 0xff, 0xff, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_POINT1_DIR1, PSHADER_COLOR_EMASK, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
#if FANG_PLATFORM_XB
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
#else
		TRUE, D3DCMP_GREATER, 0x01, //alphatest, alpha func, alpharef
#endif
		MASK_Z|MASK_E
	},
	//FSHADERS_VSPEC_BASIC, - 53
	{
		1, 0x00, 0xff, 0xff, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_DIRECTIONAL1_SPECULAR, PSHADER_COLOR, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
#if FANG_PLATFORM_XB
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
#else
		TRUE, D3DCMP_GREATER, 0x01, //alphatest, alpha func, alpharef
#endif
		MASK_NONE
	},
	//FSHADERS_VSPEC_MASK,
	{
		1, 0x00, 0xff, 0xff, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_DIRECTIONAL1_SPECULAR, PSHADER_COLORALPHA_MASK, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
#if FANG_PLATFORM_XB
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
#else
		TRUE, D3DCMP_GREATER, 0x01, //alphatest, alpha func, alpharef
#endif
		MASK_S
	},
	//FSHADERS_VSPEC_BASIC_CO,
	{
		1, 0x00, 0xff, 0xff, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_DIRECTIONAL1_SPECULAR, PSHADER_COLOR, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
#if FANG_PLATFORM_XB
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
#else
		TRUE, D3DCMP_GREATER, 0x01, //alphatest, alpha func, alpharef
#endif
		MASK_NONE
	},
	//FSHADERS_VSPEC_BASIC_MASK_CO - 38
	{
		1, 0x00, 0xff, 0xff, 0x00, 0xff,
		D3DMCS_MATERIAL, D3DMCS_MATERIAL, D3DMCS_COLOR1, D3DMCS_MATERIAL,
		VSHADER_BASE_DIRECTIONAL1_SPECULAR, PSHADER_COLORALPHA_MASK, //nstages, vshader, pshader
		D3DBLENDOP_ADD, D3DBLEND_ONE, D3DBLEND_ONE, //blendop, srcblend factor, dstblend factor
#if FANG_PLATFORM_XB
		TRUE, D3DCMP_GREATER, 0x00, //alphatest, alpha func, alpharef
#else
		TRUE, D3DCMP_GREATER, 0x01, //alphatest, alpha func, alpharef
#endif
		MASK_S
	}
};

//Tells the shader system if a texture is used in each stage.
//-1 if no texture used. Note, it only looks at the first N entries where N is the number of stages for a shader.
static FShaderStageStates_t _aShaderStageStates[][4] =
{
	//FSHADER_oBASE
	{
		0, 0, 0, 0
	},
	//FSHADERS_cBASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_tBASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_etBASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_tBASE_ADD_SPEC,
	{
		0, 0, 0, 0
	},
	//FSHADERS_etBASE_ADD_SPEC,
	{
		0, 0, 0, 0
	},
	//FSHADERS_etBASE_ADD_bSPEC, -- incomplete because of bspec.
	{
		0, 0, 0, 0
	},
	//FSHADERS_ADD_BASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_oBASE_LERP_tLAYER,
	{
		0, 2, 0, 0
	},
	//FSHADERS_oBASE_LERP_vLAYER,
	{
		0, 2, 0, 0
	},
	//FSHADERS_oBASE_LERP_pLAYER,
	{
		2, 0, 0, 0
	},
	//FSHADERS_cBASE_LERP_tLAYER,
	{
		0, 2, 0, 0
	},
	//FSHADERS_cBASE_LERP_vLAYER,
	{
		0, 2, 0, 0
	},
	//FSHADERS_cBASE_LERP_pLAYER,
	{
		2, 0, 0, 0
	},
	//FSHADERS_tBASE_LERP_tLAYER,
	{
		0, 2, -1, 0
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER,
	{
		0, 2, -1, 0
	},
	//FSHADERS_oBASE_ADD_rbENV,
	{
		2, 0, 0, 0
	},
	//FSHADERS_etBASE_ADD_rbENV,
	{
		0, 2, -1, 0
	},
	//FSHADERS_oBASE_DETAIL
	{
		0, 2, 0, 0
	},
	//FSHADERS_cBASE_DETAIL
	{
		0, 0, 0, 0
	},
	//FSHADERS_tBASE_DETAIL
	{
		0, 0, 0, 0
	},
	//FSHADERS_etBASE_DETAIL,
	{
		0, 0, 0, 0
	},
	//FSHADERS_tBASE_ADD_SPEC_DETAIL,
	{
		0, 0, 0, 0
	},
	//FSHADERS_etBASE_ADD_SPEC_DETAIL,
	{
		0, 0, 0, 0
	},
	//FSHADERS_etBASE_ADD_bSPEC_DETAIL
	{
		0, 0, 0, 0
	},
	//FSHADERS_ADD_BASE_DETAIL
	{
		0, 0, 0, 0
	},
	//FSHADERS_oBASE_LERP_tLAYER_DETAIL
	{
		0, 2, 0, 0
	},
	//FSHADERS_oBASE_LERP_vLAYER_DETAIL
	{
		0, 2, 0, 0
	},
	//FSHADERS_oBASE_LERP_pLAYER_DETAIL
	{
		2, 0, 0, 0
	},
	//FSHADERS_cBASE_LERP_tLAYER_DETAIL
	{
		0, 2, 0, 0
	},
	//FSHADERS_cBASE_LERP_vLAYER_DETAIL
	{
		0, 2, 0, 0
	},
	//FSHADERS_cBASE_LERP_pLAYER_DETAIL
	{
		2, 0, 0, 0
	},
	//FSHADERS_tBASE_LERP_tLAYER_DETAIL
	{
		0, 2, -1, 0
	},
	//FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL
	{
		0, 2, -1, 0
	},
	//FSHADERS_oBASE_ADD_rbENV_DETAIL
	{
		2, 0, 0, 0
	},
	//FSHADERS_etBASE_ADD_rbENV_DETAIL
	{
		0, 2, -1, 0
	},
	//FSHADERS_oBASE_ADD_rbSREFLECT,
	{
		0, 2, -1, 0
	},
	//FSHADERS_tBASE_vALPHA,
	{
		0, 0, 0, 0
	},
	//FSHADERS_LIQUID_ENV,
	{
		0, 0, 0, 0
	},
	//FSHADERS_LIQUID_LAYER_ENV,
	{
		0, 2, 0, 0
	},
	//FSHADERS_LIQUID_TEXTURE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_LIQUID_MOLTEN_1LAYER,
	{
		0, 0, 0, 0
	},
	//FSHADERS_LIQUID_MOLTEN_2LAYER,
	{
		0, 2, 0, 0
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV,
	{
		0, 2, 4, 5
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV,
	{
		0, 2, 4, 5
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV,
	{
		0, 2, 4, 5
	},
	//FSHADERS_oBASE_LERP_tLAYER_ADD_rbENV_DETAIL,
	{
		0, 2, 4, 5
	},
	//FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV_DETAIL,
	{
		0, 2, 4, 5
	},
	//FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL,
	{
		0, 2, 4, 5
	},
	//FSHADERS_pBASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_epBASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_pBASE_ADD_SPEC,
	{
		0, 0, 0, 0
	},
	//FSHADERS_epBASE_ADD_SPEC,
	{
		0, 0, 0, 0
	},
	//FSHADERS_epBASE_ADD_bSPEC, -- incomplete because of bspec.
	{
		0, 0, 0, 0
	},
	//FSHADERS_ADD_vBASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_LIGHT_MORPH
	{
		2, 0, 0, 0
	},
	//FSHADERS_DECAL_AI,
	{
		0, 0, 0, 0
	},
	//FSHADERS_DECALTEX_AI,
	{
		0, 0, 0, 0
	},
	//FSHADERS_DECALTEX_AT,
	{
		0, 0, 0, 0
	},
	//FSHADERS_DIFFUSETEX_AI,
	{
		0, 0, 0, 0
	},
	//FSHADERS_DIFFUSETEX_AT,
	{
		0, 0, 0, 0
	},
	//FSHADERS_DIFFUSETEX_AIAT,
	{
		0, 0, 0, 0
	},
	//FSHADERS_SPECULARTEX_AT,
	{
		0, 0, 0, 0
	},
	//FSHADERS_ADD,
	{
		0, 0, 0, 0
	},
	//FSHADERS_BLEND_AIPLUSAT,
	{
		0, 0, 0, 0
	},
	//FSHADERS_CT_PLUS_CIAT_AI,
	{
		0, 0, 0, 0
	},
	//FSHADERS_otBASE,
	{
		0, 0, 0, 0
	},
	//FSHADERS_FULLBRIGHT
	{
		-1, 0, 0, 0
	},
	//FSHADERS_FULLBRIGHT_CO
	{
		0, 0, 0, 0
	},
	//FSHADERS_VLIGHT_BASIC, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		-1, 0, 0, 0
	},
	//FSHADERS_VLIGHT_BASIC_CO, // Ca + Cv + Cm[Me] + LITD(i)*Cm[Md]
	{
		0, 0, 0, 0
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		0, 0, 0, 0
	},
	//FSHADERS_VLIGHT_MASK_EMISSIVE_CO, //Ca + Cv + Cm[Me]*At0 + LITD(i)*Cm[Md]
	{
		0, 0, 0, 0
	}
};

//Alpha blend enable for each shader type.
static u32 _anSTypeAlphaEnable[] =
{
	FALSE,	//DIFFUSE
	TRUE,	//SURFACE
	TRUE,	//TRANSLUCENCY
	TRUE	//SPECULAR
};
//***********************************************************

//*********************************************************
//Function forward definitions.
FSTATIC BOOL FSh_WorldCallbackFunc( FWorldEvent_e nEvent );

FSTATIC void _SetPixelShader(u32 nShaderIdx);
FINLINE void _SetPixelShader_Fast(u32 nShaderIdx);

void fsh_ResetFullScrRenderTarget();
void fsh_CreateFullScreenTarget();
void fsh_SetupClipPlane(CFVec3 vPlane, CFVec3 vPoint, BOOL bReflect);
void fsh_ClearClipPlane();

void fsh_FullScrCallback(void *pUserData);

void _SetupSimpleShader();
FINLINE BOOL _IsProceduralShader();

FSTATIC void _GenerateAttenMap(void);
FSTATIC void _Reset( void );
FSTATIC BOOL _BuildD3DLight( D3DLIGHT8 *pD3DLight, CFLight *pLight, BOOL bPerPixelLight );
FSTATIC BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent );

FINLINE u32 F2DW( f32 f ) { return *((u32*)&f); }
//

//Reset some data on preload/postdestry and postload.
BOOL FSh_WorldCallbackFunc( FWorldEvent_e nEvent )
{
	switch (nEvent)
	{
		case FWORLD_EVENT_WORLD_PRELOAD:
			//Setup Render targets
			bSetupRenderTargets = TRUE;
			break;
		case FWORLD_EVENT_WORLD_POSTDESTROY:
			//Clears RenderPlanes (used for liquids) and per-pixel clip plane.
			fsh_ClearRenderPlanes();
			fsh_ClearClipPlane();
			break;
		case FWORLD_EVENT_WORLD_POSTLOAD:
			//Setups reflection render target(s)
			if (bSetupRenderTargets)
			{
				fsh_ResetFullScrRenderTarget();
				bSetupRenderTargets = FALSE;
			}
			break;
		case FWORLD_EVENT_WORLD_PREDESTROY:
			break;
	};
	return TRUE;
}

//Setup dynamic reflection texture.
void fsh_SetReflectTex(CFTexInst *pTex)
{
	_pReflectTex = pTex;
}

//Toggle external color stream.
void fdx8sh_ToggleExternalColorStream(BOOL bExtStream)
{
	fsh_bUseExtColorStream = bExtStream;
}

//create dynamic reflection texture used by liquids.
void fsh_CreateFullScreenTarget()
{
	char szName[16];
	sprintf(szName, "FullScr");
	_pFullScrTarget = ftex_CreateRenderTarget_FullScreen(FTEX_RENDERTARGET_FMT_C24_A8_D24_S8, szName, FALSE, FRES_NULLHANDLE, NULL, 128, 128, FTEX_CUBE);
	_pFullScrTarget_D3DTex = NULL;
	_pFullScrTarget_D3DSurf = NULL;
	if (_pFullScrTarget)
	{
		if (_pFullScrTarget->GetTexDef())
		{
			if (_pFullScrTarget->GetTexDef()->pTexData)
			{
				_pFullScrTarget_D3DTex = (LPDIRECT3DCUBETEXTURE8)_pFullScrTarget->GetTexDef()->pTexData->pD3DTexture;
				_pFullScrTarget_D3DSurf = (LPDIRECT3DSURFACE8)_pFullScrTarget->GetTexDef()->pTexData->pD3DDepthStencil;
			}
		}
	}
	_pFullScrTarget->SetFlag(CFTexInst::FLAG_NORELEASE);
}

//Activates fullscreen target (used by liquids).
void fsh_ActivateFullScrTargets(BOOL bActivate)
{
	if (_bFullScrActive != bActivate)
	{
		ftex_ActivateRenderTarget(_pFullScrTarget, bActivate);
		_bFullScrActive = bActivate;
	}
}

//Reset fullscreen targets, render targets are cleared on each load, so this puts the reflection target back.
void fsh_ResetFullScrRenderTarget()
{
#if FANG_PLATFORM_XB
	ftex_AddRenderTarget(_pFullScrTarget, fsh_FullScrCallback, TRUE, 0, FALSE, TRUE, NULL);
#endif

	_bFullScrActive = TRUE;
	fsh_ActivateFullScrTargets(FALSE);
}

//external access to fullscreen rendertarget (cubemap on XBox)
CFTexInst *fsh_GetFullScrTexture()
{
	return (_pFullScrTarget);
}

//Cannot get data on XBox (its a cubemap so what data should I get), but this is used on GC.
u16 *fsh_GetFullScrData()
{
	//ftex_ActivateRenderTarget(_pFullScrTarget, FALSE);
	fsh_ActivateFullScrTargets(FALSE);
	return (NULL);
}

//Reactivates fullscreen target.
void fsh_ReleaseFullScrData()
{
	//ftex_ActivateRenderTarget(_pFullScrTarget, TRUE);
	fsh_ActivateFullScrTargets(TRUE);
}

//Set alpha override.
void fsh_SetLayerAlpha(f32 fAlpha)
{
	_fLayerAlpha = fAlpha;
}

//Creates a dynamic spherical reflection render target,.
void fsh_CreateSReflectTarget(void *pUserData)
{
	CFTexInst *pTexInst;
	char szName[16];
	if (_nNumSReflectTargets < MAX_SREFLECT_TARGETS)
	{
		sprintf(szName, "SReflect%d", _nNumSReflectTargets);
		pTexInst = ftex_CreateRenderTarget(128, 128, FTEX_RENDERTARGET_FMT_C24_A8_D24_S8, szName, TRUE);
		if (pTexInst)
		{
			ftex_AddRenderTarget(pTexInst, fsh_DynamicSReflect_Callback, TRUE, 20, FALSE, TRUE, pUserData);

			_aSReflectTargets[ _nNumSReflectTargets ].pTexInst = pTexInst;
			_aSReflectTargets[ _nNumSReflectTargets ].pUser = pUserData;
			_aSReflectTargets[ _nNumSReflectTargets ].fSize = 0.0f;

			_nNumSReflectTargets++;
		}
	}
}

//Resets my shader cache for those things that set shaders outside of FSH.CPP (like shadows)
void fsh_ResetShaderCache( void )
{
	_nCurrentVtxShader = 0xffffffff;
	_nCurrentPixelShader = 0xffffffff;
}

//Resets my list of spherical reflection render targets.
void fsh_ResetSReflect()
{
	u32 i;
	for (i=0; i<MAX_SREFLECT_TARGETS; i++)
	{
		_aSReflectTargets[ _nNumSReflectTargets ].fSize = 0.0f;
	}
}

//Gets a spherical reflection render target, only 4 are allowed, so it picks an existing one if it cannot create a new one.
CFTexInst *fsh_GetSReflectTarget(f32 fSize, void *pUser)
{
	u32 i;
	for (i=0; i<_nNumSReflectTargets; i++)
	{
		if ( _aSReflectTargets[i].pUser == pUser)
		{
			_aSReflectTargets[i].fSize = fSize;
			return _aSReflectTargets[i].pTexInst;
		}
	}

	for (i=0; i<_nNumSReflectTargets; i++)
	{
		if (_aSReflectTargets[i].fSize == 0.0f)
		{
			_aSReflectTargets[i].pUser = pUser;
			ftex_SetUserData(_aSReflectTargets[i].pTexInst, pUser);
			_aSReflectTargets[i].fSize = fSize;
			return _aSReflectTargets[i].pTexInst;
		}
	}

	f32 fSmallSize=_aSReflectTargets[0].fSize;
	u32 nIdx=0;
	for (i=1; i<_nNumSReflectTargets; i++)
	{
		if (_aSReflectTargets[i].fSize < fSmallSize)
		{
			fSmallSize = _aSReflectTargets[i].fSize;
			nIdx = i;
		}
	}
	if (fSize > fSmallSize)
	{
		_aSReflectTargets[nIdx].pUser = pUser;
		_aSReflectTargets[nIdx].fSize = fSize;
		ftex_SetUserData(_aSReflectTargets[nIdx].pTexInst, pUser);

		return _aSReflectTargets[nIdx].pTexInst;
	}

	return (NULL);
}

//Clears out spherical render target list (just sets # to 0 since I check that first).
void fsh_ClearSReflectTargets()
{
	_nNumSReflectTargets = 0;
}

//Setup per-pixel clip plane. It generates it by setting the near clip plane to be parallel to the clip plane to
//avoid having to make custom shaders.
void fsh_SetupClipPlane(CFVec3 vPlane, CFVec3 vPoint, BOOL bReflect)
{
	// Save the view and projection matrix.
	FSh_bUseClipPlane = TRUE;
    FDX8_pDev->GetTransform( D3DTS_PROJECTION, &_matProjectionSaved );

	CFVec3 vN, vOffs, vNRes;
	vN = FXfm_pView->m_MtxF.m44.MultDir( vPlane );
	vOffs = FXfm_pView->m_MtxF.m44.MultPoint( vPoint );

	f32 fD = -vN.Dot(vOffs);

	static CFMtx43A _ReflectMtx;
	if (bReflect)
	{
		_ReflectMtx.Identity();
		
		vNRes = vN;
		vN = -vPlane;
			
		f32 fxy, fxz, fyz, fpdotn;

	    fxy   = -2.0f * vPlane.x * vN.y;
	    fxz   = -2.0f * vPlane.x * vN.z;
	    fyz   = -2.0f * vPlane.y * vN.z;
	    fpdotn = 2.0f * vPoint.Dot(vN);

		_ReflectMtx.aa[0][0] = 1.0f - 2.0f * vN.x * vN.x;
	    _ReflectMtx.aa[1][0] = fxy;
	    _ReflectMtx.aa[2][0] = fxz;
	    _ReflectMtx.aa[3][0] = fpdotn * vN.x;

	    _ReflectMtx.aa[0][1] = fxy;
	    _ReflectMtx.aa[1][1] = 1.0f - 2.0f * vN.y * vN.y;
	    _ReflectMtx.aa[2][1] = fyz;
	    _ReflectMtx.aa[3][1] = fpdotn * vN.y;

	    _ReflectMtx.aa[0][2] = fxz;
	    _ReflectMtx.aa[1][2] = fyz;
	    _ReflectMtx.aa[2][2] = (1.0f - 2.0f * vN.z * vN.z);
	    _ReflectMtx.aa[3][2] = fpdotn * vN.z;
		
		FXfm_pMirrorMtx = &_ReflectMtx;
		
		frenderer_Push( FRENDERER_MESH, NULL );
		fmesh_FlipCullDir(TRUE);
		
		FMeshCullDir_e dxCull = fmesh_GetCullDir();
		fmesh_SetCullDir(dxCull);
		frenderer_Pop();

		vN = vNRes;
	}
	else
	{
		FXfm_pMirrorMtx = NULL;
	}
    
    //
    // Adjust the projection matrix so that the near clip plane is aligned
    // with the mirror plane.
    //
	
    float zn = -fD / vN.z;
    float zf = 4500.0f;

    D3DXMATRIX matProjection;

    matProjection._11 = _matProjectionSaved._11;
    matProjection._21 = 0.0f;
    matProjection._31 = 0.0f;
    matProjection._41 = 0.0f;

    matProjection._12 = 0.0f;
    matProjection._22 = _matProjectionSaved._22;
    matProjection._32 = 0.0f;
    matProjection._42 = 0.0f;

    // z's dependance on x and y.
    float nxnz = vN.x / vN.z;
    float nynz = vN.y / vN.z;

    // Maximum z contributed by x and y at farclip.
    float maxxz = zf * fabsf(nxnz) * (1.0f / matProjection._11);
    float maxyz = zf * fabsf(nynz) * (1.0f / matProjection._22);

    // Scale factor for the z range.
    float zscale = zf / (zf + maxxz + maxyz - zn);

    matProjection._13 = nxnz * zscale;
    matProjection._23 = nynz * zscale;
    matProjection._33 = zscale;
    matProjection._43 = -zn * zscale;

    matProjection._14 = 0.0f;
    matProjection._24 = 0.0f;
    matProjection._34 = 1.0f;
    matProjection._44 = 0.0f;

//    FDX8_pDev->SetTransform( D3DTS_PROJECTION, &matProjection );
	FDX8_SetProjMatrix( &matProjection );
}

//Clears out clip plane and restores near clip plane
void fsh_ClearClipPlane()
{
//	FDX8_pDev->SetTransform( D3DTS_PROJECTION, &_matProjectionSaved );
	FDX8_SetProjMatrix( &_matProjectionSaved );
	fmesh_FlipCullDir(FALSE);

	FSh_bUseClipPlane = FALSE;
}

//Dynamic spherical reflection.
void fsh_DynamicSReflect_Callback(void *pUserData)
{
	static CFXfm _VXfm;
	//reflective mesh.w
	CFWorldMesh *pMesh = (CFWorldMesh *)pUserData;
	CFVec3 vPos;
	
	if (!pMesh) return;
	//If mesh is not drawn then it shouldn't reflect anything.
	if (pMesh->m_nFlags&FMESHINST_FLAG_DONT_DRAW) return;
	if ( !pMesh->WasDrawnLastFrame() && !pMesh->WasDrawnThisFrame() ) 
	{
		return;
	}
	
	//Transform object's position into world space.
	pMesh->m_Xfm.TransformPointF(vPos, pMesh->m_BoundSphere_MS.m_Pos);
	CFVec3 vDir, vUp(0,1,0);
	CFVec3 vCamPos, vCamDir, vN;
	f32 fFOV, f2Dot;
	
	//get camera, need to change this like fullscreen callback.
	CFCamera *pCamera = fcamera_GetCameraByIndex(0);
	const CFXfm *pCamXfm = pCamera->GetFinalXfm();
	pCamXfm->InitStackWithView();
	
	//calculate new camera position.
	vCamPos.x = FXfm_pView->m_MtxR.m_vPos.x; vCamPos.y = FXfm_pView->m_MtxR.m_vPos.y; vCamPos.z = FXfm_pView->m_MtxR.m_vPos.z;
	vCamDir.x = FXfm_pView->m_MtxR.m_vFront.x; vCamDir.y = FXfm_pView->m_MtxR.m_vFront.y; vCamDir.z = FXfm_pView->m_MtxR.m_vFront.z;
	
	vN.x = vCamPos.x - vPos.x; vN.y = vCamPos.y - vPos.y; vN.z = vCamPos.z - vPos.z;
	if (vN.x || vN.y || vN.z)
	{
		vN.Unitize();
	}
	else
	{
		vN.x = 1.0f;
	}
	
	//Calculate new direction by reflecting the view direction off the object->view vector.
	f2Dot = 2.0f * (vN.x*vCamDir.x + vN.y*vCamDir.y + vN.z*vCamDir.z);
	vDir.x = vCamDir.x - vN.x*f2Dot;
	vDir.y = vCamDir.y - vN.y*f2Dot;
	vDir.z = vCamDir.z - vN.z*f2Dot;
	
	if (vDir.x || vDir.y || vDir.z)
	{
		vDir.Unitize();
	}
	else
	{
		vDir.x = 1.0f;
	}
	
	//Create a large FOV to simulate sphere reflection.
	fFOV = FMATH_DEG2RAD(70.0f);
	fviewport_InitPersp(_pSView, fFOV, fFOV, 0.1f, 1000.0f);
	fviewport_SetActive(_pSView);

	fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 1.0f, 0.0f, 1.0f, 0);
	
	_VXfm.BuildLookatFromDirVec(vPos, vDir, vUp);
	_VXfm.InitStackWithView();

	//don't draw reflecting mesh in reflection.
	pMesh->m_nFlags |= FMESHINST_FLAG_DONT_DRAW;
	fvis_Draw( NULL, FALSE );
	pMesh->m_nFlags &= ~FMESHINST_FLAG_DONT_DRAW;
}

void fsh_LoadProjMtx(f32 fovX, f32 fovY)
{
}

BOOL fsh_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	fdx8vid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	_bWindowCreated = FALSE;
	_bModuleInitialized = TRUE;
	_bShaderSystemOpen = FALSE;
	FSh_pSpecularLight = NULL;

	_Reset();

	_matProjectionSaved._11 = 1.f;
	_matProjectionSaved._21 = 0.f;
	_matProjectionSaved._31 = 0.f;
	_matProjectionSaved._41 = 0.f;

	_matProjectionSaved._12 = 0.f;
	_matProjectionSaved._22 = 1.f;
	_matProjectionSaved._32 = 0.f;
	_matProjectionSaved._42 = 0.f;

	_matProjectionSaved._13 = 0.f;
	_matProjectionSaved._23 = 0.f;
	_matProjectionSaved._33 = 1.f;
	_matProjectionSaved._43 = 0.f;

	_matProjectionSaved._14 = 0.f;
	_matProjectionSaved._24 = 0.f;
	_matProjectionSaved._34 = 0.f;
	_matProjectionSaved._44 = 1.f;

	// Init global registers...
	_Default_GReg_Motif00.OpaqueBlack();
	fsh_SetGlobalRegister( FSHREG_MOTIF_A0_C0, (u32)&_Default_GReg_Motif00 );

	_Default_GReg_Motif01.TransparentWhite();
	fsh_SetGlobalRegister( FSHREG_MOTIF_A0_C1, (u32)&_Default_GReg_Motif01 );

	_Default_GReg_Motif10.OpaqueBlack();
	fsh_SetGlobalRegister( FSHREG_MOTIF_A1_C0, (u32)&_Default_GReg_Motif10 );

	_Default_GReg_Motif11.OpaqueWhite();
	fsh_SetGlobalRegister( FSHREG_MOTIF_A1_C1, (u32)&_Default_GReg_Motif11 );

	return TRUE;
}

void fsh_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fdx8vid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_bModuleInitialized = FALSE;
}

u32 fsh_GetGlobalRegister( FShReg_e nRegNum ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nRegNum>=FSHREG_GLOBAL_OFFSET && nRegNum<FSHREG_ONE_PAST_LAST );

	return _anGlobalRegisters[nRegNum-FSHREG_GLOBAL_OFFSET];
}

// Returns previous value.
u32 fsh_SetGlobalRegister( FShReg_e nRegNum, u32 nNewValue ) 
{
	u32 nPrevValue;

	FASSERT( _bModuleInitialized );
	FASSERT( nRegNum>=FSHREG_GLOBAL_OFFSET && nRegNum<FSHREG_ONE_PAST_LAST );

	nPrevValue = _anGlobalRegisters[nRegNum-FSHREG_GLOBAL_OFFSET];
	_anGlobalRegisters[nRegNum-FSHREG_GLOBAL_OFFSET] = nNewValue;

	return nPrevValue;
}

void fsh_Open( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !_bShaderSystemOpen );

	fdx8_SetRenderState_LOCALVIEWER( TRUE );
	fdx8_SetRenderState_ZBIAS( 0 );
	fdx8_SetRenderState_POINTSPRITEENABLE( FALSE );
	fdx8_SetRenderState_POINTSCALEENABLE( FALSE );
	if( FDX8_Caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE ) {
		fdx8_SetRenderState_COLORWRITEENABLE( D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );
	}
	fdx8_SetRenderState_STENCILENABLE( FALSE );
	fdx8_SetRenderState_STENCILFUNC( D3DCMP_ALWAYS );
	fdx8_SetRenderState_STENCILFAIL( D3DSTENCILOP_KEEP );
	fdx8_SetRenderState_STENCILZFAIL( D3DSTENCILOP_KEEP );
	fdx8_SetRenderState_STENCILPASS( D3DSTENCILOP_KEEP );
	fdx8_SetRenderState_STENCILREF( 0 );
	fdx8_SetRenderState_STENCILMASK( 0xffffffff );
	fdx8_SetRenderState_STENCILWRITEMASK( 0xffffffff );

	FDX8_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
	FDX8_pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
	FDX8_pDev->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2 );
	FDX8_pDev->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3 );

	ftex_SetTexAddress(0, TRUE, TRUE);
	ftex_SetTexAddress(1, TRUE, TRUE);
	ftex_SetTexAddress(2, TRUE, TRUE);
	ftex_SetTexAddress(3, TRUE, TRUE);

	fsh_PrepareFrame();
	_Reset();
	_bShaderSystemOpen = TRUE;

#if	FSH_SIMPLE_SHADER
	_SetupSimpleShader();
#endif
}

void fsh_Close( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	_bShaderSystemOpen = FALSE;
}

void fsh_SetDepthBias( u32 nBias ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );

	if (nBias > 0)
	{
		_nDepthBias = nBias;
	}
	
	FMATH_CLAMPMAX( nBias, FDX8_MAX_ZBIAS );
	_nDepthBias = nBias;

	fdx8_SetRenderState_ZBIAS( nBias );
}

u32 fsh_GetDepthBias( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	return _nDepthBias;
}

//Detail map tile factor, default = 4.0f
void fsh_SetDetailMapTileFactor(f32 fTileFactor)
{
	FSh_fDetailTile = fTileFactor;
	FDX8_pDev->SetVertexShaderConstant(CV_DETAIL_SCALE, D3DXVECTOR4(FSh_fDetailTile, FSh_fDetailTile, FSh_fDetailTile, FSh_fDetailTile), 1);
}

//BumpMap Tile factor, default = 1.0f
void fsh_SetBumpMapTileFactor(f32 fTileFactor)
{
	FSh_fBumpTile = fTileFactor;
	FDX8_pDev->SetVertexShaderConstant(CV_BUMP_SCALE, D3DXVECTOR4(FSh_fBumpTile, FSh_fBumpTile, FSh_fBumpTile, FSh_fBumpTile), 1);
}

f32 fsh_GetDetailMapTileFactor()
{
	return (FSh_fDetailTile);
}

f32 fsh_GetBumpMapTileFactor()
{
	return (FSh_fBumpTile);
}

//Setup fog parameters
void fsh_Fog_SetParam(f32 fNearZ, f32 fFarZ, f32 fMinY, f32 fMaxY, f32 fDensity, CFColorRGB& FogClr, CFTexInst *pFogTex)
{
	fsh_fFogDensity = fDensity;
	fsh_fFogNearZ = fNearZ;
	fsh_fFogFarZ = fFarZ;
	fsh_fFogMinY = fMinY;
	fsh_fFogMaxY = fMaxY;
	fsh_fFogColor = FogClr;
	fsh_fFogColor2 = FogClr;

	//Fdepth = (Zview - NearZ)/(FarZ - NearZ)
	//Fdepth = Zview*(1/(FarZ-NearZ)) + (-NearZ/(FarZ-NearZ)) [ mad in vertex shader ASM ]
	if ( FMATH_FABS(fFarZ - fNearZ) > 0.1f )
	{
		fsh_fFogConst[0] = fsh_fFogDensity /(fsh_fFogFarZ - fsh_fFogNearZ);
		fsh_fFogConst[1] = -fsh_fFogNearZ*fsh_fFogConst[0];
	}
	else
	{
		fsh_fFogConst[0] = 0.001f; //default to fNearZ = 0, fFarZ = 1000, density = 1.0f: 1/(1000-0)
		fsh_fFogConst[1] = 0.0f;   //default to fNearZ = 0
	}
	//Fheight = 1.0 - (Yworld - MinY)/(MaxY - MinY)
	//Fheight = -Yworld*(1/(MaxY-MinY)) + (MaxY/(MaxY-MinY)) [ mad in vertex shader ASM ]

	if ( FMATH_FABS(fMinY - fMaxY) > 0.1f )
	{
		fsh_fFogConst[2] = -1.0f/(fsh_fFogMaxY - fsh_fFogMinY);
		fsh_fFogConst[3] = -fsh_fFogMaxY*fsh_fFogConst[2];
	}
	else
	{
		fsh_fFogConst[2] = 0.0f;
		fsh_fFogConst[3] = 1.0f;
	}
	//mad: res(x) = x*a + b

	FDX8_pDev->SetVertexShaderConstant(CV_FOG_DEPTH, fsh_fFogConst, 1);

	fsh_pFogTex = pFogTex;
}

void fsh_TexCoordMtx_ResetList( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	_nTexCoordMtxCount = 0;
}

void fsh_SetSurfaceColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha)
{
	_fSurfaceColor.Set( fRed, fGreen, fBlue, fAlpha );
}

// Important: pTexCoordMtx must persist until no further fsh_Execute() calls require it.
void fsh_TexCoordMtx_Add( u32 nTexLayerID, const CFMtx43 *pTexCoordMtx ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );

	if( _nTexCoordMtxCount >= _nTexCoordMtxMaxCount ) 
	{
		DEVPRINTF( "fsh_TexCoordMtx_Add(): Too many texture coordinate matrices specified. Max is %u.\n", _nTexCoordMtxMaxCount );
		return;
	}

	#if FANG_DEBUG_BUILD
		// A specific nTexLayerID should be added only once (for performance reasons)...
		for( u32 i=0; i<_nTexCoordMtxCount; i++ ) 
		{
			if( nTexLayerID == _aTexCoordMtx[i].nTexLayerID ) 
			{
				// This ID already added...
				DEVPRINTF( "fsh_TexCoordMtx_Add(): ID %u already added. For best performance, don't add an ID more than once.\n", nTexLayerID );
				break;
			}
		}
	#endif

	_aTexCoordMtx[_nTexCoordMtxCount].nTexLayerID = nTexLayerID;
	_aTexCoordMtx[_nTexCoordMtxCount].pTexCoordMtx = pTexCoordMtx;

	_nTexCoordMtxCount++;
}

u32 fsh_TexCoordMtx_GetCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexCoordMtxCount;
}

u32 fsh_TexCoordMtx_GetMaxCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexCoordMtxMaxCount;
}

void fsh_TexOverride_ResetList( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	_nTexOverrideCount = 0;
}

// Important: pTexDef must persist until no further fsh_Execute() calls require it.
void fsh_TexOverride_Add( u32 nTexLayerID, CFTexInst *pTexInst ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );

	if( _nTexOverrideCount >= _nTexOverrideMaxCount ) 
	{
		DEVPRINTF( "fsh_TexOverride_Add(): Too many texture overrides specified. Max is %u.\n", _nTexOverrideMaxCount );
		return;
	}

	#if FANG_DEBUG_BUILD
		// A specific nTexLayerID should be added only once (for performance reasons)...
		for( u32 i=0; i<_nTexOverrideCount; i++ ) 
		{
			if( nTexLayerID == _aTexOverride[i].nTexLayerID ) 
			{
				// This ID already added...
				DEVPRINTF( "fsh_TexOverride_Add(): ID %u already added. For best performance, don't add an ID more than once.\n", nTexLayerID );
				break;
			}
		}
	#endif

	_aTexOverride[_nTexOverrideCount].nTexLayerID = nTexLayerID;
	_aTexOverride[_nTexOverrideCount].pTexInst = pTexInst;

	_nTexOverrideCount++;
}

u32 fsh_TexOverride_GetCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexOverrideCount;
}

u32 fsh_TexOverride_GetMaxCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexOverrideMaxCount;
}

void fsh_Light_ResetList( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	_nLightCount = 0;
	_nPointCount = 0;
	_nDirCount = 0;
	_nSpotCount = 0;
	_nPerPixelPoint=0;
	_nPerPixelSpot = 0;

	u32 i;
	//Setup per-pixel spot defaults.
	for (  i=0; i<8; i++ )
	{
		_aPerPixelSpotLights[i].pDXTex = _pAttenMap;
	}
}

// Important: pLight must persist until no further fsh_Execute() calls require it.
void fsh_Light_Add( CFLight *pLight, BOOL bLightPerPixel ) {
	D3DLIGHT8 D3DLight;

	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );

	if (_nLightCount == 0)
	{
		FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE, D3DXVECTOR4(0, 0, 0, 1.0f), 1);
	}

	if( pLight->m_nFlags & FLIGHT_FLAG_ENABLE ) 
	{
		// SER: Added this so that we can use coronas without lights (hope it doesn't cause any problems)!
		if ( !(pLight->m_nFlags & FLIGHT_FLAG_CORONA_ONLY) )
		{
#if 1
			// We will light the mesh per pixel if the mesh accepts per pixel lights
			// and this light is a per pixel light...
			bLightPerPixel = (bLightPerPixel && (pLight->m_nFlags & FLIGHT_FLAG_PER_PIXEL));
			
			// ...or if this light is a spotlight and it has a projection texture and
			// the mesh is to be lit per pixel or the light does not require the mesh to be lit per pixel
			bLightPerPixel |= (pLight->m_nType == FLIGHT_TYPE_SPOT) && pLight->m_pProjectionTex && (bLightPerPixel || !(pLight->m_nFlags & FLIGHT_FLAG_MESH_MUST_BE_PER_PIXEL));
#else
#pragma message( "fsh_Light_Add() - DX Lighting shaders need to fixed so they handle non-projected texture spots." )
			bLightPerPixel = (bLightPerPixel && (pLight->m_nFlags & FLIGHT_FLAG_PER_PIXEL)) || (pLight->m_nType == FLIGHT_TYPE_SPOT);
#endif			
			if ( bLightPerPixel && FSh_fPerPixelFade == 0.f )
			{
				return;
			}

			if ( _BuildD3DLight( &D3DLight, pLight, bLightPerPixel ) )
			{
				if( _nLightCount < FDX8_nMaxD3DLights ) 
				{
					fdx8_SetLight( _nLightCount++, &D3DLight );
				}
			}
		}
	}
}

u32 fsh_Light_GetCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nLightCount;
}

u32 fsh_Light_GetMaxCount( void ) 
{
	FASSERT( _bWindowCreated );
	return FDX8_nMaxD3DLights;
}

void fsh_Fog_Enable( BOOL bEnable, BOOL bForce ) 
{
	FASSERT( _bWindowCreated );
	//FASSERT( _bShaderSystemOpen );
#if FANG_PLATFORM_XB
	_bFogEnabled = bEnable;
#else
	_bFogEnabled = FALSE;
#endif

	if (_bFogEnabled && !FSh_bAlphaOut)
	{
		FDX8_pDev->SetVertexShaderConstant(CV_FOG_DEPTH, fsh_fFogConst, 1);
	}
	else
	{
		f32 fFogValues[4]={0, 0, 0, 1};
		if (FSh_bAlphaOut)
		{
			fFogValues[1] = 1.0f;
		}
		FDX8_pDev->SetVertexShaderConstant(CV_FOG_DEPTH, fFogValues, 1);
	}
}

BOOL fsh_Fog_IsEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	return _bFogEnabled;
}

//Create vertex shaders based on included data.
FSTATIC void _SetupVertexShaders()
{
	u32 i;
	for (i=0; i<_nNumVertexShaders; i++)
	{
		#if FANG_PLATFORM_XB
		if (FAILED(FDX8_pDev->CreateVertexShader((const DWORD *)_apVShaderDecl[i], (const DWORD *)_apVShaderFunc[i], (DWORD *)&_anVShader_Handle[i], 0)))//D3DUSAGE_PERSISTENTDIFFUSE|D3DUSAGE_PERSISTENTSPECULAR)))
		{
			fang_DevPrintf( "fdx8sh::_SetupVertexShaders() - CreateVertexShader Error Shader Index: %d.\n", i );
			_apVShaderFunc[i] = 0;
		}
		#else
		if (FAILED(FDX8_pDev->CreateVertexShader((const DWORD *)_apVShaderDecl[i], (const DWORD *)_apVShaderFunc[i], (DWORD *)&_anVShader_Handle[i], 0)))
		{
			fang_DevPrintf( "fdx8sh::_SetupVertexShaders() - CreateVertexShader Error Shader Index: %d.\n", i );
			_apVShaderFunc[i] = 0;
		}
		#endif
	}
	FDX8_pDev->SetVertexShaderConstant(CV_ZERO, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_ONE, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_EPS, D3DXVECTOR4(0.0001f, 0.0001f, 0.0001f, 1.0f/255.0f), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_HALF, D3DXVECTOR4(0.5f, 0.5f, 0.5f, 1.0f), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_DETAIL_SCALE, D3DXVECTOR4(4.0f, 4.0f, 4.0f, 4.0f), 1);

	//f0 = 1.0f/(far_z - near_z), f1 = -near_z/(far_z-near_z), f2 = -1.0f/(maxy-miny), f3 = maxy/(maxy-miny)
	FDX8_pDev->SetVertexShaderConstant(CV_FOG_DEPTH, fsh_fFogConst, 1);
	FDX8_pDev->SetVertexShaderConstant(CV_FOG_MIN, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.5f), 1);
}

//delete all my vertex shaders.
FSTATIC void _DeleteVertexShaders()
{
	u32 i;
	for (i=0; i<_nNumVertexShaders; i++)
	{
		if (_anVShader_Handle[i])
		{
			FDX8_pDev->DeleteVertexShader( _anVShader_Handle[i] );
		}
	}
}

//fast version of _SetVertexShader()
FINLINE void _SetVertexShader_Fast(u32 nBaseIdx)
{
	u32 nRemapIdx, nShader;

	nRemapIdx = _anVShaderRemap[nBaseIdx-1][_nVertexType];
	nShader = _anVShaderRemap[nBaseIdx-1][_nVertexType];
	if (_nCurrentVtxShader != nShader)
	{
		FDX8_SetVertexShader(nShader);
	}
}

//Sets the vertex shader, remapping if necessary.
FSTATIC void _SetVertexShader(u32 nBaseIdx)
{
	u32 nRemapIdx, nShader;
	if (nBaseIdx == VSHADER_BASE_ENV_POINT1_DIR1)
	{
		if (_nLightCount == 0)
		{
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT2_DIFFUSE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
		}
		else
		{
			switch (_nPointCount)
			{
				case 0: //Light 0
					nBaseIdx = VSHADER_BASE_ENV_POINT1_DIR1;
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHT2_DIFFUSE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
				case 1:	//Light 0, 1
					nBaseIdx = VSHADER_BASE_ENV_POINT1_DIR1;
					break;
				case 2: //Light 0, 1, 2
					nBaseIdx = VSHADER_BASE_ENV_POINT2_DIR1;
					break;
				case 3: //Light 0, 1, 2, 3
					nBaseIdx = VSHADER_BASE_ENV_POINT4_DIR1;
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHT5_DIFFUSE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
					break;
				case 4: //Light 0, 1, 2, 3, 4
					nBaseIdx = VSHADER_BASE_ENV_POINT4_DIR1;
					break;
				case 5: //Light 0, 1, 2, 3, 4, 5
				case 6: //Light 0, 1, 2, 3, 4, 5, 6
					nBaseIdx = VSHADER_BASE_ENV_POINT4_DIR1;
					break;
			}
		}

		//This is my final remapped shader handle.
		nRemapIdx = _anVShaderRemap[nBaseIdx-1][_nVertexType];

		if (fsh_bUseExtColorStream)
		{
			nRemapIdx = VSHADER_ENV_POINT1_DIR1_EXTSTREAM + (nRemapIdx-VSHADER_ENV_POINT1_DIR1);
		}

		nShader = _anVShader_Handle[nRemapIdx];
	}
	else if (nBaseIdx == VSHADER_BASE_POINT1_DIR1)
	{
		//select lighting shader here.
		if (_nLightCount == 0)
		{
			nBaseIdx = VSHADER_BASE_VCOLOR;
		}
		else
		{
			switch (_nPointCount)
			{
				case 0: //Light 0
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHT2_DIFFUSE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
				case 1:	//Light 0, 1
					nBaseIdx = VSHADER_BASE_POINT1_DIR1;
					break;
				case 2: //Light 0, 1, 2
					nBaseIdx = VSHADER_BASE_POINT2_DIR1;
					break;
				case 3: //Light 0, 1, 2, 3
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHT5_DIFFUSE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
				case 4: //Light 0, 1, 2, 3, 4
					nBaseIdx = VSHADER_BASE_POINT4_DIR1;
					break;
				case 5: //Light 0, 1, 2, 3, 4, 5
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHT7_DIFFUSE, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
				case 6: //Light 0, 1, 2, 3, 4, 5, 6
					nBaseIdx = VSHADER_BASE_POINT6_DIR1;
					break;
				case 7:
				case 8:
					nBaseIdx = VSHADER_BASE_POINT7_DIR1;
					break;
			}
		}
		//
		if (!fsh_bUseExtColorStream)
		{
			nRemapIdx = _anVShaderRemap[nBaseIdx-1][_nVertexType];
		}
		else
		{
			//If there is an external color stream, I  must use a different shader index.
			switch (nBaseIdx)
			{
				case VSHADER_BASE_VCOLOR:
					nRemapIdx = VSHADER_VCOLOR_EXTSTREAM;
				break;
				case VSHADER_BASE_POINT1_DIR1:
					nRemapIdx = VSHADER_POINT1_DIR1_EXTSTREAM;
				break;
				case VSHADER_BASE_POINT2_DIR1:
					nRemapIdx = VSHADER_POINT2_DIR1_EXTSTREAM;
				break;
				case VSHADER_BASE_POINT4_DIR1:
					nRemapIdx = VSHADER_POINT4_DIR1_EXTSTREAM;
				break;
				case VSHADER_BASE_POINT6_DIR1:
					nRemapIdx = VSHADER_POINT6_DIR1_EXTSTREAM;
				break;
				case VSHADER_BASE_POINT7_DIR1:
				case VSHADER_BASE_POINT8:
					nRemapIdx = VSHADER_POINT7_DIR1_EXTSTREAM;
				break;
				default:
					nRemapIdx = _anVShaderRemap[nBaseIdx-1][_nVertexType];
				break;
			}
		}

		//This is my final remapped shader handle.
		nShader = _anVShader_Handle[nRemapIdx];
	}
	else if (nBaseIdx >= VSHADER_BASE_LM4 && nBaseIdx <= VSHADER_BASE_EMASK_LM3)
	{
        u8 n;

		if (fsh_bUseExtColorStream)
		{
			nRemapIdx = VSHADER_VCOLOR_EXTSTREAM;
			//This is my final remapped shader handle.
			nShader = _anVShader_Handle[nRemapIdx];
		}
		else
		{
			if (nBaseIdx == VSHADER_BASE_LM4) { n = 0; }
			else if (nBaseIdx == VSHADER_BASE_ZMASK_EMASK_LM2) { n = 1; }
			else if (nBaseIdx == VSHADER_BASE_ZMASK_LM3) { n = 2; }
			else { n = 3; }

			//which lightmap shader should I use.
			nRemapIdx = _anLMVShader[n][m_nLM-1];
			//This is my final remapped shader handle.
			nShader = _anVShader_Handle[nRemapIdx];
		}
	}
	else if (nBaseIdx > VSHADER_BASE_NONE)
	{
		if (FSh_bShadowRender)
		{
			//special shader for shadows.
			nRemapIdx = 0;
		}
		else
		{
			if ( FSh_shaderFlags&FSh_RENDER_FORCEBASE_SHADER )
			{
				nBaseIdx = _BaseVShaderRemap[ nBaseIdx ];
			}
			else
			{
				if ( nBaseIdx >= VSHADER_BASE_DETAIL && nBaseIdx <= VSHADER_BASE_oBASE_ADD_rbENV_DETAIL && !(FSh_shaderFlags&FSh_RENDER_DETAIL) )
				{
					nBaseIdx = _DetailVShaderRemap[ nBaseIdx-VSHADER_BASE_DETAIL ];
				}

				if ( nBaseIdx >= VSHADER_BASE_PASSTHRU_ENV && nBaseIdx <= VSHADER_BASE_oBASE_ADD_rbSREFLECT_EMBM && !(FSh_shaderFlags&FSh_RENDER_REFLECTIONMAP) )
				{
					nBaseIdx = _ReflectionVShaderRemap[ nBaseIdx-VSHADER_BASE_PASSTHRU_ENV ];
				}
			}
			
			nRemapIdx = _anVShaderRemap[nBaseIdx-1][_nVertexType];
		}
		//This is my final remapped shader handle.
		nShader = _anVShader_Handle[nRemapIdx];
	}

	//Set my new vertex shader if it is different from my current one.
	if (_nCurrentVtxShader != nShader)
	{
		FDX8_SetVertexShader(nShader);

		FDX8_pDev->SetVertexShaderConstant(CV_ZERO, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 1);
		FDX8_pDev->SetVertexShaderConstant(CV_ONE, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);

		//This is a waste, there must be a better way...
		FDX8_pDev->SetVertexShaderConstant(CV_EPS, D3DXVECTOR4(0.0001f, 0.0001f, 0.0001f, 1.0f/255.0f), 1);
		FDX8_pDev->SetVertexShaderConstant(CV_HALF, D3DXVECTOR4(0.5f, 0.5f, 0.5f, 1.0f), 1);

		/*
		if (FSh_bAlphaOut)
		{
			f32 fFogValues[4]={0, 1, 0, 1};
			FDX8_pDev->SetVertexShaderConstant(CV_FOG_DEPTH, fFogValues, 1);
			FDX8_pDev->SetVertexShaderConstant(CV_FOG_MIN, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 1);
		}
		else
		{
			FDX8_pDev->SetVertexShaderConstant(CV_FOG_DEPTH, fsh_fFogConst, 1);
		}
		*/
				
		_nCurrentVtxShader = nShader;
	}
}

//Create pixel shaders based on included data.
FSTATIC void _SetupPixelShaders()
{
	u32 i;
	for (i=0; i<_nNumPixelShaders; i++)
	{
	#if FANG_PLATFORM_WIN
		if (FAILED(FDX8_pDev->CreatePixelShader((const DWORD *)_apPShaderFunc[i], (DWORD *)&_anPShader_Handle[i])))
		{
			fang_DevPrintf( "fdx8sh::_SetupPixelShaders() - CreatePixelShader Error Shader Index: %d.\n", i );
		}
	#else
		if (FAILED(FDX8_pDev->CreatePixelShader((const D3DPIXELSHADERDEF *)_apPShaderFunc[i], (DWORD *)&_anPShader_Handle[i])))
		{
			fang_DevPrintf( "fdx8sh::_SetupPixelShaders() - CreatePixelShader Error Shader Index: %d.\n", i );
		}
	#endif
	}
}

//Delete all my pixel shaders to make DX happy.
FSTATIC void _DeletePixelShaders()
{
	u32 i;
	for (i=0; i<_nNumPixelShaders; i++)
	{
		if (_anPShader_Handle[i])
		{
			FDX8_pDev->DeletePixelShader( _anPShader_Handle[i] );
		}
	}
}

//Set my pixel shader with no remapping.
FSTATIC void _SetPixelShader_Fast(u32 nShaderIdx)
{
	//Load only if different then what is already set.
	u32 nShader = _anPShader_Handle[nShaderIdx-1];
	if (_nCurrentPixelShader != nShader)
	{
		FDX8_pDev->SetPixelShader(nShader);	

		//I have to set my pixel shader constants if the shader changes.
		//The XBox flushes all shader constants when the shader is set.
		_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
		_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
		_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
		_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
		FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);

		_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = 1.0f;
		FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);
	}
	else
	{
		//I haven't changed shaders so I can set the constants only if they changed.
		if (_fLayerAlpha != _afPixelConst3[0])
		{
			_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = 1.0f;
			FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);
		}

		if (_afPixelConst2[0] != 1.0f || _afPixelConst2[1] != 1.0f || _afPixelConst2[2] != 1.0f || _afPixelConst2[3] != 1.0f)
		{
			_afPixelConst2[0] = 1.0f;
			_afPixelConst2[1] = 1.0f;
			_afPixelConst2[2] = 1.0f;
			_afPixelConst2[3] = 1.0f;
			FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
		}
	}
    
	_nCurrentPixelShader = nShader;
}

//Set Pixel shader with remapping if necessary.
FSTATIC void _SetPixelShader(u32 nShaderIdx)
{
	u32 nShader;
	if (nShaderIdx >= PSHADER_LM1 && nShaderIdx <= PSHADER_EMASK_LM3)
	{
		//Figure out what shader I need for the lightmapping state.
		u8 n, i;
		if (fsh_bUseExtColorStream)
		{
			if (nShaderIdx == PSHADER_LM4) { nShaderIdx = PSHADER_COLOR; }
			else if (nShaderIdx == PSHADER_ZMASK_EMASK_LM2) { nShaderIdx = PSHADER_COLOR_MASK; }
			else if (nShaderIdx == PSHADER_ZMASK_LM2) { nShaderIdx = PSHADER_COLOR_MASK; }
			else if (nShaderIdx == PSHADER_ZMASK_LM3) { nShaderIdx = PSHADER_COLOR_MASK; }
			else if (nShaderIdx == PSHADER_EMASK_LM3) { nShaderIdx = PSHADER_COLOR_MASK; }
			else 
			{ 
				nShaderIdx = PSHADER_COLOR; 
			}

			//Set only if the shader is different
			nShader = _anPShader_Handle[nShaderIdx-1];
		}
		else
		{
			if (nShaderIdx == PSHADER_LM4) { n = 0; }
			else if (nShaderIdx == PSHADER_ZMASK_EMASK_LM2) { n = 1; }
			else if (nShaderIdx == PSHADER_ZMASK_LM3) { n = 2; }
			else { n = 3; }

			if (FSh_shaderType != SHADERTYPE_DIFFUSE)
			{
				nShaderIdx = _anLMPShader_Trans[n][m_nLM-1];
			}
			else
			{
				nShaderIdx = _anLMPShader[n][m_nLM-1];
			}

			if (nShaderIdx == PSHADER_EMASK_LM1)
			{
				if (FSh_bInvAlpha_Emissive)
				{
					nShaderIdx = PSHADER_INV_EMASK_LM1;
				}
			}

			//Set only if the shader is different
			nShader = _anPShader_Handle[nShaderIdx-1];
			
			//Load lightmap motif values.
			for (i=1; i<m_nLM; i++)
			{
				f32 afData[4]={1.0f, 1.0f, 1.0f, 1.0f};
				CFColorRGB pDestColor;
				const CFColorMotif *pMotif = (const CFColorMotif *)FSh_pnLightMapInputRegisters[i*3+3];
				if (pMotif)
				{
					pMotif->ComputeColor(&pDestColor);
					afData[0] = pDestColor.fRed; afData[1] = pDestColor.fGreen; afData[2] = pDestColor.fBlue; afData[3] = 1.0f;
				}
				FDX8_pDev->SetPixelShaderConstant(i, afData, 1);
			}
		}
		if (_nCurrentPixelShader != nShader)
		{
			FDX8_pDev->SetPixelShader(nShader);
		}
	}
	else
	{
		//Remap ColorEMask shader based on InvAlpha_Emissive setting or lack of emissive mask.

		if ( FSh_shaderFlags&FSh_RENDER_FORCEBASE_SHADER )
		{
			nShader = _anPShader_Handle[ _BasePShaderRemap[ nShaderIdx ]-1 ];
			if (_nCurrentPixelShader != nShader)
			{
				FDX8_pDev->SetPixelShader(nShader);

				//I have to set my pixel shader constants if the shader changes.
				//The XBox flushes all shader constants when the shader is set.

				_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = _fLayerAlpha;
				FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);

				_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
				_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
				_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
				_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
				FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
			}
			else
			{
				//I haven't changed shaders so I can set the constants only if they changed.
				if (_fLayerAlpha != _afPixelConst3[0])
				{
					_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = _fLayerAlpha;
					FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);
				}

				if (_afPixelConst2[0] != _fFinalSurfaceColor.fRed || _afPixelConst2[1] != _fFinalSurfaceColor.fGreen || _afPixelConst2[2] != _fFinalSurfaceColor.fBlue || _afPixelConst2[3] != _fFinalSurfaceColor.fAlpha)
				{
					_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
					_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
					_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
					_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
					FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
				}
			}
		}
		else
		{
			if (nShaderIdx == PSHADER_COLOR_EMASK)
			{
				if (FSh_bInvAlpha_Emissive)
				{
					nShaderIdx = PSHADER_COLOR_EMASK_INV;
				}
				else if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK] == NULL)
				{
					nShaderIdx = PSHADER_COLOR_EMISSIVE;
				}
			}
			else if (nShaderIdx == PSHADER_EMASK_LM1)
			{
				if (FSh_bInvAlpha_Emissive)
				{
					nShaderIdx = PSHADER_INV_EMASK_LM1;
				}
			}

			if (nShaderIdx > PSHADER_NONE)
			{
				//Load only if different then what is already set.

				if ( !(FSh_shaderFlags&FSh_RENDER_DETAIL) && nShaderIdx >= PSHADER_DETAIL && nShaderIdx <= PSHADER_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL)
				{
					nShaderIdx = _DetailPShaderRemap[ nShaderIdx-PSHADER_DETAIL ];
				}

				if ( !(FSh_shaderFlags&FSh_RENDER_REFLECTIONMAP) && nShaderIdx >= PSHADER_oBASE_ADD_rbENV && nShaderIdx <= PSHADER_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL)
				{
					nShaderIdx = _ReflectionPShaderRemap[ nShaderIdx-PSHADER_oBASE_ADD_rbENV ];
				}
				
				nShader = _anPShader_Handle[nShaderIdx-1];
				if (_nCurrentPixelShader != nShader)
				{
					FDX8_pDev->SetPixelShader(nShader);

					//I have to set my pixel shader constants if the shader changes.
					//The XBox flushes all shader constants when the shader is set.

					_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = _fLayerAlpha;
					FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);

					_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
					_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
					_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
					_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
					FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
				}
				else
				{
					//I haven't changed shaders so I can set the constants only if they changed.
					if (_fLayerAlpha != _afPixelConst3[0])
					{
						_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = _fLayerAlpha;
						FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);
					}

					if (_afPixelConst2[0] != _fFinalSurfaceColor.fRed || _afPixelConst2[1] != _fFinalSurfaceColor.fGreen || _afPixelConst2[2] != _fFinalSurfaceColor.fBlue || _afPixelConst2[3] != _fFinalSurfaceColor.fAlpha)
					{
						_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
						_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
						_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
						_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
						FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
					}
				}
			}
		}
	}
	_nCurrentPixelShader = nShader;
}

//Set and save vertex type. Also sets a flag if the type has changed.
void fdx8sh_SetVertexType(u32 type)
{
	if (_nFixedVShader != type)
	{
		_bVBChanged = TRUE;
	}
	_nFixedVShader = type;
	if (type == (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)))
	{
		_nVertexType = 0;
	}
	else if ( type == (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) )
	{
		_nVertexType = 0;
	}
	else if (type == (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1)))
	{
		_nVertexType = 1;
	}
	else if (type == (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX3 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) | D3DFVF_TEXCOORDSIZE2(2)))
	{
		_nVertexType = 1;
	}
	else if (type == (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX4 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1) | D3DFVF_TEXCOORDSIZE2(2) | D3DFVF_TEXCOORDSIZE2(3)))
	{
		_nVertexType = 1;
	}
	else if (type == (D3DFVF_XYZB3 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)))
	{
		_nVertexType = 2;
	}
	else if (type == (D3DFVF_XYZB3 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1)))
	{
		_nVertexType = 3;
	}
}

//Get internal attenuation map.
void *fsh_GetAttenMap()
{
	return (void *)_pAttenMap;
}

//Build normalization cube map.
//If a texture lookup with an unnormalized per-pixel vector is made into this cube map
//the result will be normalized (per-pixel normalization).
#if !FANG_PLATFORM_XB
BOOL _CreateNormalizationCubeMap(u32 nWidth)
{
	HRESULT hr;
	
	hr = D3DXCreateCubeTexture(FDX8_pDev, nWidth, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &_pNormalizeCube);

	if(FAILED(hr))
	{
		_pNormalizeCube = NULL;
		return hr;
	}

	u32 nD3DFormat = D3DFMT_X8R8G8B8;
	u32 i, x, y;

	for (i = 0; i < 6; i++)
	{
		D3DLOCKED_RECT Locked;
		D3DXVECTOR3 Normal;
		float w,h;
		D3DSURFACE_DESC ddsdDesc;
		
		_pNormalizeCube->GetLevelDesc(0, &ddsdDesc);
		_pNormalizeCube->LockRect((D3DCUBEMAP_FACES)i, 0, &Locked, NULL, 0);


		for (y = 0; y < ddsdDesc.Height; y++)
		{
			h = (float)y / ((float)(ddsdDesc.Height - 1));
			h *= 2.0f;
			h -= 1.0f;

			for (x = 0; x < ddsdDesc.Width; x++)
			{
				w = (float)x / ((float)(ddsdDesc.Width - 1));
				w *= 2.0f;
				w -= 1.0f;

				u32* pBits = (u32*)((u8*)Locked.pBits + (y * Locked.Pitch));
				pBits += x;

				switch((D3DCUBEMAP_FACES)i)
				{
					case D3DCUBEMAP_FACE_POSITIVE_X:
						Normal = D3DXVECTOR3(1.0f, -h, -w);
						break;
					case D3DCUBEMAP_FACE_NEGATIVE_X:
						Normal = D3DXVECTOR3(-1.0f, -h, w);
						break;
					case D3DCUBEMAP_FACE_POSITIVE_Y:
						Normal = D3DXVECTOR3(w, 1.0f, h);
						break;
					case D3DCUBEMAP_FACE_NEGATIVE_Y:
						Normal = D3DXVECTOR3(w, -1.0f, -h);
						break;
					case D3DCUBEMAP_FACE_POSITIVE_Z:
						Normal = D3DXVECTOR3(w, -h, 1.0f);
						break;
					case D3DCUBEMAP_FACE_NEGATIVE_Z:
						Normal = D3DXVECTOR3(-w, -h, -1.0f);
						break;
				}

				D3DXVec3Normalize(&Normal, &Normal);

				// Scale to be a color from 0 to 255 (127 is 0)
				Normal += D3DXVECTOR3(1.0f, 1.0f, 1.0f);
				Normal *= 127.0f;

				// Store the color
				*pBits = (u32)(((u32)Normal.x << 16) | ((u32)Normal.y << 8) | ((u32)Normal.z << 0));

			}
		}
		_pNormalizeCube->UnlockRect((D3DCUBEMAP_FACES)i, 0);
	}

	return TRUE;
}
#else
BOOL _CreateNormalizationCubeMap(u32 nWidth)
{
	HRESULT hr;
	IDirect3DSurface8 *pSurface;

	FMemFrame_t MemFrame = fmem_GetFrame();
	u32 *pImageData_L = (u32*)fmem_AllocAndZero(nWidth*nWidth*4, 8);
	
	hr = D3DXCreateCubeTexture(FDX8_pDev, nWidth, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &_pNormalizeCube);

	if(FAILED(hr))
	{
		_pNormalizeCube = NULL;
		return hr;
	}

	u32 nD3DFormat = D3DFMT_X8R8G8B8;
	u32 i, x, y;
	RECT srcRect, dstRect;

	srcRect.left = 0;
	srcRect.right = nWidth;
	srcRect.top = 0;
	srcRect.bottom = nWidth;

	dstRect.left = 0;
	dstRect.right = nWidth;
	dstRect.top = 0;
	dstRect.bottom = nWidth;

	for (i = 0; i < 6; i++)
	{
		D3DXVECTOR3 Normal;
		float w,h;
		D3DSURFACE_DESC ddsdDesc;
		
		_pNormalizeCube->GetLevelDesc(0, &ddsdDesc);

		for (y = 0; y < ddsdDesc.Height; y++)
		{
			h = (float)y / ((float)(ddsdDesc.Height - 1));
			h *= 2.0f;
			h -= 1.0f;

			for (x = 0; x < ddsdDesc.Width; x++)
			{
				w = (float)x / ((float)(ddsdDesc.Width - 1));
				w *= 2.0f;
				w -= 1.0f;

				u32* pBits = (u32*)((u8*)pImageData_L + (y * nWidth * 4));
				pBits += x;

				switch((D3DCUBEMAP_FACES)i)
				{
					case D3DCUBEMAP_FACE_POSITIVE_X:
						Normal = D3DXVECTOR3(1.0f, -h, -w);
						break;
					case D3DCUBEMAP_FACE_NEGATIVE_X:
						Normal = D3DXVECTOR3(-1.0f, -h, w);
						break;
					case D3DCUBEMAP_FACE_POSITIVE_Y:
						Normal = D3DXVECTOR3(w, 1.0f, h);
						break;
					case D3DCUBEMAP_FACE_NEGATIVE_Y:
						Normal = D3DXVECTOR3(w, -1.0f, -h);
						break;
					case D3DCUBEMAP_FACE_POSITIVE_Z:
						Normal = D3DXVECTOR3(w, -h, 1.0f);
						break;
					case D3DCUBEMAP_FACE_NEGATIVE_Z:
						Normal = D3DXVECTOR3(-w, -h, -1.0f);
						break;
				}

				D3DXVec3Normalize(&Normal, &Normal);

				// Scale to be a color from 0 to 255 (127 is 0)
				Normal += D3DXVECTOR3(1.0f, 1.0f, 1.0f);
				Normal *= 127.0f;

				// Store the color
				*pBits = (u32)(((u32)Normal.x << 16) | ((u32)Normal.y << 8) | ((u32)Normal.z << 0));

			}
		}
		_pNormalizeCube->GetCubeMapSurface((D3DCUBEMAP_FACES)i, 0, &pSurface);
		D3DXLoadSurfaceFromMemory(pSurface, NULL, &dstRect, pImageData_L, (D3DFORMAT)nD3DFormat, nWidth*4, NULL, &srcRect, D3DX_FILTER_POINT, 0);
		pSurface->Release();
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;
}
#endif

//Generate attenuation maps (and other utility textures).
FSTATIC void _GenerateAttenMap(void)
{
#if FANG_PLATFORM_XB
	u8 *pImageData_A;
	u8 *pImageData_L;
	s32 nS, nT, nValue;
	float fX, fY, fZ, f2O255=(2.0f/255.0f);

	FMemFrame_t MemFrame = fmem_GetFrame();

	pImageData_A = (u8*)fmem_AllocAndZero(256*256, 8);
	pImageData_L = (u8*)fmem_AllocAndZero(256*4, 8);

	LPDIRECT3DSURFACE8 pSurface;
		
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<256; nT++)
		{
			fX = (float)nS;
			fX = fX*f2O255 - 1.0f;
			fY = (float)nT;
			fY = fY*f2O255 - 1.0f;
			fZ = fX*fX+fY*fY; //z = 1 - sqrtf(x^2 + y^2), (fX, fY, fZ)Dot(0, 0, 1) = fZ
			if (fZ < 0.0f) fZ = 0.0f;
			if (fZ > 1.0f) fZ = 1.0f;
			pImageData_A[nS + (nT<<8)] = 0xff - (u8)(fZ*255.0f); // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}
	
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			pImageData_L[nS + (nT<<8)] = (nS == 0)?(0xff):(0x00);
		}
	}

	RECT srcRect;
	
	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 256, 1, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &_pAttenMap)) )
	{
		srcRect.left = 0;
		srcRect.right = 255;
		srcRect.top = 0;
		srcRect.bottom = 255;

		_pAttenMap->GetSurfaceLevel(0, &pSurface);
		D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_A, D3DFMT_L8, 256, NULL, &srcRect, D3DX_FILTER_NONE, 0);
		pSurface->Release();
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 4, 1, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &_pTexKill)) )
	{
		srcRect.left = 0;
		srcRect.right = 255;
		srcRect.top = 0;
		srcRect.bottom = 3;

		_pTexKill->GetSurfaceLevel(0, &pSurface);
		D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_L8, 256, NULL, &srcRect, D3DX_FILTER_NONE, 0);
		pSurface->Release();
	}

	f32 fDot=0.0f;
	f32 fSpecPwr=16.0f;

	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			fDot = (f32)nS * _fOO255;
			fDot = powf(fDot, fSpecPwr);
			if (fDot < 0.0f) fDot = 0.0f;
			if (fDot > 1.0f) fDot = 1.0f;
			nValue = (u32)(fDot * 255.0f);
			pImageData_L[nS + (nT<<8)] = nValue; // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 4, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &_pSpec16)) )
	{
		srcRect.left = 0;
		srcRect.right = 256;
		srcRect.top = 0;
		srcRect.bottom = 4;

		_pSpec16->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_L8, 256, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pSpec16->Release();
			_pSpec16 = NULL;
		}
		pSurface->Release();
	}

	fSpecPwr = 32.0f;

	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			fDot = (f32)nS * _fOO255;
			fDot = (f32)pow((f32)fDot, (f32)fSpecPwr);
			if (fDot < 0.0f) fDot = 0.0f;
			if (fDot > 1.0f) fDot = 1.0f;
			nValue = (u32)(fDot * 255.0f);
			pImageData_L[nS + (nT<<8)] = nValue; // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 4, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &_pSpec32)) )
	{
		srcRect.left = 0;
		srcRect.right = 256;
		srcRect.top = 0;
		srcRect.bottom = 4;

		_pSpec32->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_L8, 256, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pSpec32->Release();
			_pSpec32 = NULL;
		}
		pSurface->Release();
	}

	fSpecPwr = 48.0f;

	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			fDot = (f32)nS * _fOO255;
			fDot = (f32)pow((f32)fDot, (f32)fSpecPwr);
			if (fDot < 0.0f) fDot = 0.0f;
			if (fDot > 1.0f) fDot = 1.0f;
			nValue = (u32)(fDot * 255.0f);
			pImageData_L[nS + (nT<<8)] = nValue; // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 4, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &_pSpec48)) )
	{
		srcRect.left = 0;
		srcRect.right = 256;
		srcRect.top = 0;
		srcRect.bottom = 4;

		_pSpec48->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_L8, 256, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pSpec48->Release();
			_pSpec48 = NULL;
		}
		pSurface->Release();
	}

	fmem_ReleaseFrame( MemFrame );
#else
	_pAttenMap = _pTexKill = NULL;
	
	u32 *pImageData_A;
	u32 *pImageData_L;
	s32 nS, nT, nValue, nValueA;
	float fX, fY, fZ, f2O255=(2.0f/255.0f);

	FMemFrame_t MemFrame = fmem_GetFrame();

	pImageData_A = (u32*)fmem_AllocAndZero(256*256*4, 8);
	pImageData_L = (u32*)fmem_AllocAndZero(256*4*4, 8);

	LPDIRECT3DSURFACE8 pSurface;
		
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<256; nT++)
		{
			fX = (float)nS;
			fX = fX*f2O255 - 1.0f;
			fY = (float)nT;
			fY = fY*f2O255 - 1.0f;
			fZ = fX*fX+fY*fY; //z = 1 - sqrtf(x^2 + y^2), (fX, fY, fZ)Dot(0, 0, 1) = fZ
			if (fZ < 0.0f) fZ = 0.0f;
			if (fZ > 1.0f) fZ = 1.0f;
			nValue = 0xff - (u32)(fZ*255.0f);
			pImageData_A[nS + (nT<<8)] = nValue | (nValue<<8) | (nValue<<16) | (nValue<<24); // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}
	
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			nValue = (u32)((nS == 0)?(0xff):(0x00));
			pImageData_L[nS + (nT<<8)] = nValue | (nValue<<8) | (nValue<<16) | (nValue<<24); // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}

	RECT srcRect;
	
	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &_pAttenMap)) )
	{
		srcRect.left = 0;
		srcRect.right = 255;
		srcRect.top = 0;
		srcRect.bottom = 255;

		_pAttenMap->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_A, D3DFMT_A8R8G8B8, 256*4, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pAttenMap->Release();
			_pAttenMap = NULL;
		}
		pSurface->Release();
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 4, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &_pTexKill)) )
	{
		srcRect.left = 0;
		srcRect.right = 255;
		srcRect.top = 0;
		srcRect.bottom = 3;

		_pTexKill->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_A8R8G8B8, 256*4, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pTexKill->Release();
			_pTexKill = NULL;
		}
		pSurface->Release();
	}

	f32 fDot=0.0f;
	f32 fSpecPwr=16.0f;

	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<1; nT++)
		{
			fDot = (f32)nS * _fOO255;
			fDot = (f32)pow((f32)fDot, (f32)fSpecPwr);
			if (fDot < 0.0f) fDot = 0.0f;
			if (fDot > 1.0f) fDot = 1.0f;
			nValue = (u32)(fDot * 255.0f);
			nValueA = (u32)((nS == 0)?(0xff):(0x00));
			pImageData_L[nS + (nT<<8)] = nValue | (nValue<<8) | (nValue<<16) | (nValueA<<24); // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &_pSpec16)) )
	{
		srcRect.left = 0;
		srcRect.right = 256;
		srcRect.top = 0;
		srcRect.bottom = 1;

		_pSpec16->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_A8R8G8B8, 256*4, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pSpec16->Release();
			_pSpec16 = NULL;
		}
		pSurface->Release();
	}

	fSpecPwr = 32.0f;

	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<1; nT++)
		{
			fDot = (f32)nS * _fOO255;
			fDot = (f32)pow((f32)fDot, (f32)fSpecPwr);
			if (fDot < 0.0f) fDot = 0.0f;
			if (fDot > 1.0f) fDot = 1.0f;
			nValue = (u32)(fDot * 255.0f);
			nValueA = (u32)((nS == 0)?(0xff):(0x00));
			pImageData_L[nS + (nT<<8)] = nValue | (nValue<<8) | (nValue<<16) | (nValueA<<24); // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &_pSpec32)) )
	{
		srcRect.left = 0;
		srcRect.right = 256;
		srcRect.top = 0;
		srcRect.bottom = 1;

		_pSpec32->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_A8R8G8B8, 256*4, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pSpec32->Release();
			_pSpec32 = NULL;
		}
		pSurface->Release();
	}

	fSpecPwr = 48.0f;

	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<1; nT++)
		{
			fDot = (f32)nS * _fOO255;
			fDot = (f32)pow((f32)fDot, (f32)fSpecPwr);
			if (fDot < 0.0f) fDot = 0.0f;
			if (fDot > 1.0f) fDot = 1.0f;
			nValue = (u32)(fDot * 255.0f);
			nValueA = (u32)((nS == 0)?(0xff):(0x00));
			pImageData_L[nS + (nT<<8)] = nValue | (nValue<<8) | (nValue<<16) | (nValueA<<24); // (fX, fY, fZ) Dot (0, 0, 1)
		}
	}

	if ( SUCCEEDED(FDX8_pDev->CreateTexture(256, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &_pSpec48)) )
	{
		srcRect.left = 0;
		srcRect.right = 256;
		srcRect.top = 0;
		srcRect.bottom = 1;

		_pSpec48->GetSurfaceLevel(0, &pSurface);
		if( FAILED( D3DXLoadSurfaceFromMemory(pSurface, NULL, NULL, pImageData_L, D3DFMT_A8R8G8B8, 256*4, NULL, &srcRect, D3DX_FILTER_NONE, 0) ) ) {
			_pSpec48->Release();
			_pSpec48 = NULL;
		}
		pSurface->Release();
	}

	fmem_ReleaseFrame( MemFrame );
#endif

	_CreateNormalizationCubeMap(64);
}



//returns TRUE is shader is translucent else FALSE.
BOOL _UseTrans(u32 shaderID)
{
	if (shaderID == FSHADERS_tBASE            || shaderID == FSHADERS_etBASE) return (TRUE);
	if (shaderID == FSHADERS_etBASE_ADD_rbENV || shaderID == FSHADERS_ADD_BASE) return (TRUE);
	if (shaderID == FSHADERS_tBASE_ADD_SPEC   || shaderID == FSHADERS_etBASE_ADD_SPEC) return (TRUE);
	if (shaderID == FSHADERS_etBASE_ADD_bSPEC || shaderID == FSHADERS_tBASE_LERP_tLAYER) return (TRUE);
	if (shaderID == FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER || shaderID == FSHADERS_otBASE) return (TRUE);

	if (shaderID == FSHADERS_pBASE            || shaderID == FSHADERS_epBASE) return (TRUE);
	if (shaderID == FSHADERS_ADD_vBASE 		  || shaderID == FSHADERS_epBASE_ADD_bSPEC) return (TRUE);
	if (shaderID == FSHADERS_pBASE_ADD_SPEC   || shaderID == FSHADERS_epBASE_ADD_SPEC) return (TRUE);
	if (shaderID == FSHADERS_epBASE_ADD_bSPEC) return (TRUE);

	if (shaderID == FSHADERS_LIQUID_ENV || shaderID == FSHADERS_LIQUID_LAYER_ENV || shaderID == FSHADERS_LIQUID_TEXTURE) return (TRUE);
	return (FALSE);
}

//Calculates the number of passes based on the shader type.
FINLINE u32 fsh_GetCurrentNumPasses(BOOL *pbFastPass)
{
	u32 nRet = 1;
	
	m_nLM = 0;
	m_nLMMode = LM_NONE;

	if (pbFastPass) { *pbFastPass = FALSE; }
	_bFastShader = FALSE;

	switch (FSh_shaderType)
	{
		case SHADERTYPE_DIFFUSE:
			nRet = fsh_GetNumDiffusePasses(0, 0, pbFastPass);
		break;
		case SHADERTYPE_SURFACE:
			//Surface shaders also happen in 1 pass.
			nRet = 1;
		break;
		case SHADERTYPE_TRANSLUCENCY:
			//Setup lightmap mode for translucency.
			//Shaders with translucency and lightmaps must be remapped at runtime based on the number
			//of lightmaps.
			m_nLM = 0;
			m_nLMMode = LM_NONE;
			if (FSh_pnLightMapInputRegisters)
			{
				m_nLM = FSh_pnLightMapInputRegisters[0];
				if (m_nLM > 0 && FSh_pnLightMapInputRegisters[1])
				{
					if ( !(FSh_shaderFlags&FSh_RENDER_MOTIF_LIGHTMAPS) )
					{
						m_nLM = 1;
					}
					if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK] && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK])
					{
						m_nLMMode = ZMASK_EMASK_LM2;
					}
					else
					{
						m_nLMMode = ZMASK_LM3;
					}
				}
				else
				{
					m_nLMMode = LM_NONE;
				}
			}

			if (_nShaderID == FSHADERS_LIQUID_LAYER_ENV) 
			{
				//The only translucent shader with more then 1 pass.
				//Note that this would not work in the general case.
				nRet = 2;
			}
			else
			{
				nRet = 1;
			}

		break;
		case SHADERTYPE_SPECULAR:
			//this will change soon.
			if ( FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP] && (FSh_shaderFlags&FSh_RENDER_BUMP) )
			{
				nRet = 0;
			}
			else
			{
				nRet = fsh_GetNumSpecularPasses(0, 0);
			}
		break;
	};
	return (nRet);
}

//Gets the number of diffuse passes which is a function of the number of per-pixel lights, whether the surface has bumpmaps
//and how many lightmaps the surface has.
u32 fsh_GetNumDiffusePasses(u32 shaderID, u32 diffuseID, BOOL *pbFastPass)
{
	u32 nPasses = 1;

#if 1
	if (FSh_bUseFastPass)
	{
		if ( !FSh_bShadowRender && pbFastPass && ( (_nPerPixelPoint == 0 && _nPerPixelSpot == 0) || !(FSh_shaderFlags&FSh_RENDER_MULTIPASS_LIGHTING) ) )
		{
			if (_nSurfaceShaderID == FSHADERS_oBASE_ADD_rbENV)
			{
				*pbFastPass = TRUE;
				_bFastShader = TRUE;

				m_nLMMode = LM_NONE;
				if (FSh_pnLightMapInputRegisters)
				{
					m_nLM = FSh_pnLightMapInputRegisters[0];

					if (m_nLM > 0 && FSh_pnLightMapInputRegisters[1])
					{
						m_nLMMode = LM4;
						m_nLM = 1;
					}
				}

				return 1;
			}
		}
	}
#endif

	m_nLM = 0;

	//Per-pixel lights cannot be used if the internal attenuation map does not exist.
	if (_pAttenMap == NULL) return (nPasses);
	//There is no seperate lighting pass for translucent materials.
	if (_UseTrans(shaderID) && shaderID > 0) return (0);

	//Calculate per-pixel lighting cost, based on the existance of a lightmap.
	if ( FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP] && (FSh_shaderFlags&FSh_RENDER_BUMP) )
	{
		nPasses += _nPerPixelPoint;
		nPasses += _nPerPixelSpot;
	}
	else
	{
		nPasses += ((_nPerPixelPoint+1)>>1);
		nPasses += ((_nPerPixelSpot+1)>>1);
	}

	//Calculate the lightmap cost based on the number of lightmaps.
	//The LMMode is also set (or cleared) here.
	m_nLMMode = LM_NONE;
	if (FSh_pnLightMapInputRegisters)
	{
		m_nLM = FSh_pnLightMapInputRegisters[0];

		if (m_nLM > 0 && FSh_pnLightMapInputRegisters[1])
		{
			if ( !(FSh_shaderFlags&FSh_RENDER_MOTIF_LIGHTMAPS) )
			{
				m_nLM = 1;
			}

			if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK] && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK])
			{
				m_nLMMode = ZMASK_EMASK_LM2;
				if (m_nLM > 2) { nPasses++; }
			}
			else if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK] || FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK])
			{
				m_nLMMode = (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK])?(ZMASK_LM3):(EMASK_LM3);
				if (m_nLM > 3) { nPasses++; }
			}
			else
			{
				m_nLMMode = LM4;
				if (m_nLM > 4) { nPasses++; }
			}
		}
		else
		{
			m_nLMMode = LM_NONE;
		}
	}

	//Multipass lighting is disabled, return.
	if ( !(FSh_shaderFlags&FSh_RENDER_MULTIPASS_LIGHTING) )
	{
		nPasses = 1;
	}
    	
	return nPasses;
}

//Get the number of specular passes. This will change in the near future.
u32 fsh_GetNumSpecularPasses(u32 shaderID, u32 specularID)
{
	u32 nPasses = 0;

	if (FSh_shaderType == SHADERTYPE_SPECULAR)//FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP] > 0 && FSh_pnSurfaceInputRegisters[FSHADERS_LIGHT_REG_SMOTIF])
	{
		if ( FSh_shaderFlags&FSh_RENDER_MULTIPASS_LIGHTING )
		{
			nPasses = 1;
		}
	}
	
	return (nPasses);
}

BOOL fsh_SetSurfaceShader(u32 shaderID)
{
	_nSurfaceShaderID = shaderID;
	return TRUE;
}

//Sets the current shader.
BOOL fsh_SetShader(u32 shaderID, u32 diffuseID, u32 specID, u32 passIdx)
{
	_bUseTrans=_UseTrans(shaderID);
	_bSetDetailScale = FALSE;
	u32 nshader;

	_nSurfaceShaderID = shaderID;
	_nPassIdx = passIdx;

	//This remaps my shader index into my global shader list.
	if (FSh_shaderType == SHADERTYPE_DIFFUSE)
	{
		if (_bUseTrans) 
		{
			return FALSE; //no diffuse or specular passes for translucent shaders.
		}

		nshader = diffuseID;
        _pnShaderRemap = _anDiffuseRemap;
	}
	else if (FSh_shaderType == SHADERTYPE_SURFACE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY )
	{
		if (FShaders_aShaderRegs[shaderID].nSurfaceTypeFlags & FSHADERS_SURFACE_FLAG_DETAIL_MAP)
		{
			_bSetDetailScale = TRUE;
			_nDetailReg=0;
			while ( FShaders_aShaderRegs[shaderID].anRegType[_nDetailReg] != FSHADERS_REG_DETAILMAP_TILE_FACTOR )
			{
				_nDetailReg++;
			}	
		}

		nshader = shaderID;
		_pnShaderRemap = _anSurfaceRemap;
		if (_bUseTrans)
		{
			FSh_shaderType = SHADERTYPE_TRANSLUCENCY;
		}
	}
	else if (FSh_shaderType == SHADERTYPE_SPECULAR)
	{
		if (_bUseTrans) return (FALSE);
		_pnShaderRemap = _anSpecularRemap;

		nshader = specID;
	}

	//This shader ID is used for setting up shader states.
	//Note that an ExecuteCurrent() must be called atleast once since shader states are lazy (executed only when needed).
	if (shaderID < FSHADERS_SHADER_COUNT)
	{
		_nShaderID = _pnShaderRemap[nshader];
	}
	else
	{
		_nShaderID = FSHADERS_oBASE;
	}
	return (TRUE);
}

void fsh_RenderMode(FRenderMode_e renderMode)
{
}

//Sets up a simple shader, primarily used for testing.
void _SetupSimpleShader()
{
	fdx8_SetRenderState_ALPHABLENDENABLE(FALSE);
	fdx8_SetRenderState_ALPHATESTENABLE(FALSE);
	fdx8_SetRenderState_ZWRITEENABLE(TRUE);
	fdx8_SetRenderState_ZFUNC(D3DCMP_LESSEQUAL);

	u32 vShader = VSHADER_BASE_VCOLOR;
	u32 pShader = PSHADER_COLOR;

	_SetVertexShader_Fast(VSHADER_BASE_PASSTHRU);
	_SetPixelShader_Fast(PSHADER_COLOR);
}

//Setup render states such as Blend mode, Alphatest, Ztest, Alpha/Color write and Material Constants
void _SetRenderStates()
{
	CFColorRGBA pDestColor, pDestColor2;

	//Default material
	D3DMATERIAL8 d3d8mat;
	D3DCOLORVALUE white={1,1,1,1}, black={0,0,0,0};
	d3d8mat.Diffuse = white;
	d3d8mat.Ambient = white;
	d3d8mat.Emissive = black;
	d3d8mat.Specular = black;

	//Allow alpha writing ONLY for pass 0 of diffuse pass, this is used for fog.
	if (_nPassIdx == 0 && FSh_shaderType == SHADERTYPE_DIFFUSE && (_bFogEnabled||FSh_bAlphaOut))
	{
		fdx8_SetRenderState_COLORWRITEENABLE(D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA);
	}
	else
	{
		fdx8_SetRenderState_COLORWRITEENABLE(D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE);
	}

	//Setup for multipass lighting
	if ( (_nPassIdx > 0 || FSh_shaderType == SHADERTYPE_SPECULAR) && !_IsProceduralShader() )
	{
		fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
		fdx8_SetRenderState_SRCBLEND(D3DBLEND_ONE);
		fdx8_SetRenderState_DESTBLEND(D3DBLEND_ONE);
		fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);
		
		fdx8_SetRenderState_ALPHATESTENABLE(FALSE);
		
		fdx8_SetRenderState_ZWRITEENABLE( FALSE );
		fdx8_SetRenderState_ZFUNC( D3DCMP_EQUAL );

		return;
	}

	if (_bRestoreClamp)
	{
		_bRestoreClamp = FALSE;
		ftex_SetTexAddress(0, TRUE, TRUE);
		ftex_SetTexAddress(1, TRUE, TRUE);
		ftex_SetTexAddress(2, TRUE, TRUE);
		ftex_SetTexAddress(3, TRUE, TRUE);
	}

    if (FSh_bShadowRender)
	{
		//Special states for shadow rendering.
		fdx8_SetRenderState_ALPHABLENDENABLE( FALSE );

		fdx8_SetRenderState_ZWRITEENABLE( FALSE );
		fdx8_SetRenderState_ZFUNC( D3DCMP_ALWAYS );

		if ( _aShaderRenderStates[_nShaderID].nSrcBlend != D3DBLEND_ONE || _aShaderRenderStates[_nShaderID].nDstBlend != D3DBLEND_ONE || 1 )
		{
			fdx8_SetRenderState_ALPHATESTENABLE( FALSE );
		}
		else
		{
			fdx8_SetRenderState_ALPHATESTENABLE( _aShaderRenderStates[_nShaderID].bAlphaTest );
			fdx8_SetRenderState_ALPHAFUNC( _aShaderRenderStates[_nShaderID].nAlphaFunc );
			fdx8_SetRenderState_ALPHAREF( _aShaderRenderStates[_nShaderID].nAlphaRef );
		}

		FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
	}
	else
	{
		if (FSh_bSurfaceOnly)
		{
			//This allows me to render only surfaces passes.
			if (FSh_shaderType == SHADERTYPE_SURFACE)
			{
				fdx8_SetRenderState_ALPHABLENDENABLE( FALSE );
			}
			else
			{
				fdx8_SetRenderState_ALPHABLENDENABLE( _anSTypeAlphaEnable[FSh_shaderType] );
			}
		}
		else
		{
			//This sets up the alpha blend mode based on the shader type.
			//Diffuse = NO_BLEND, Surface = BLEND, Specular = BLEND, Translucent = BLEND
			fdx8_SetRenderState_ALPHABLENDENABLE( _anSTypeAlphaEnable[FSh_shaderType] );
		}
		//Setup actual blend settings.
		fdx8_SetRenderState_SRCBLEND( _aShaderRenderStates[_nShaderID].nSrcBlend );
		fdx8_SetRenderState_DESTBLEND( _aShaderRenderStates[_nShaderID].nDstBlend );
		fdx8_SetRenderState_BLENDOP( _aShaderRenderStates[_nShaderID].nBlendOp );

		if (FSh_shaderType == SHADERTYPE_DIFFUSE)
		{
			//No tfactor is used to pass white.
			FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
		}
		else if (_aShaderRenderStates[_nShaderID].nTFactorReg < 0xff
				&& FSh_pnSurfaceInputRegisters[_aShaderRenderStates[_nShaderID].nTFactorReg])
		{
			//TFactor is used for things such as envmap motif. Its a generic "FACTOR" field.
			const CFColorMotif *pMotif = (const CFColorMotif *)FSh_pnSurfaceInputRegisters[_aShaderRenderStates[_nShaderID].nTFactorReg];
			if (pMotif->nMotifIndex == 0)
			{
				pMotif->ComputeColor( &pDestColor );
				FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha), 1);
			}
			else //Invalid motif index! Set to default color.
			{
				FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
			}

			m_nLMMode = LM_NONE;
		}
		else if (FSh_shaderType != SHADERTYPE_TRANSLUCENCY)
		{
			//No factor, so load white for shaders to work.
			FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
			m_nLMMode = LM_NONE;
		}

		
		if (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY)
		{ //Both diffuse and translucency types have lighting.
			if (FSh_shaderType == SHADERTYPE_TRANSLUCENCY)
			{
				((const CFColorMotif *)fsh_GetGlobalRegister(FSHREG_AMBIENT))->ComputeColor( &pDestColor );
				if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])
				{
					((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])->ComputeColor( &pDestColor2 );
					pDestColor += pDestColor2;
					pDestColor.Clamp1();
				}

				//Global/Mesh Ambient + Emissive = Total Ambient (0,0,0) - (1,1,1).
				FDX8_pDev->SetVertexShaderConstant(CV_MAT_AMBIENT, D3DXVECTOR4(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, 1.0f), 1);

				if (_aShaderRenderStates[_nShaderID].nMaterialSource[1] < 0xff)
				{
					//load diffuse color into the proper register.
					FDX8_pDev->SetVertexShaderConstant(CV_MAT_DIFFUSE, D3DXVECTOR4(d3d8mat.Diffuse.r, d3d8mat.Diffuse.g, d3d8mat.Diffuse.b, 1.0f), 1);
				}
			}
			else
			{
				if (_aShaderRenderStates[_nShaderID].nTFactorReg < 0xff)
				{ //This settings tells us that emissive motif can be used for this shader and will be masked.
					if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])
					{
						//If the emissive motif exists then load it into the proper register, note the value must be halved for 2X lighting.
						((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])->ComputeColor( &pDestColor );
	#if FANG_LIGHTING_2X
						FDX8_pDev->SetVertexShaderConstant(CV_MAT_EMISSIVE, D3DXVECTOR4(pDestColor.fRed*0.5f, pDestColor.fGreen*0.5f, pDestColor.fBlue*0.5f, pDestColor.fAlpha), 1);
	#else
						FDX8_pDev->SetVertexShaderConstant(CV_MAT_EMISSIVE, D3DXVECTOR4(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha), 1);
	#endif
					}
					else
					{
						FDX8_pDev->SetVertexShaderConstant(CV_MAT_EMISSIVE, D3DXVECTOR4(0,0,0,0), 1);
					}
				}
				else
				{
					FDX8_pDev->SetVertexShaderConstant(CV_MAT_EMISSIVE, D3DXVECTOR4(0,0,0,0), 1);
				}

				if (_aShaderRenderStates[_nShaderID].nDiffuseReg < 0xff)
				{
					if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_DMOTIF])
					{
						((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_DMOTIF])->ComputeColor( &pDestColor );
						d3d8mat.Diffuse.r = pDestColor.fRed;
						d3d8mat.Diffuse.g = pDestColor.fGreen;
						d3d8mat.Diffuse.b = pDestColor.fBlue;
					}
				}
				
				if (_aShaderRenderStates[_nShaderID].nMaterialSource[1] < 0xff)
				{
					FDX8_pDev->SetVertexShaderConstant(CV_MAT_DIFFUSE, D3DXVECTOR4(d3d8mat.Diffuse.r, d3d8mat.Diffuse.g, d3d8mat.Diffuse.b, 1.0f), 1);
				}
				
				//Calculates the mesh ambient color.
				((const CFColorMotif *)fsh_GetGlobalRegister(FSHREG_AMBIENT))->ComputeColor( &pDestColor );
				//If there is not emissive mask in this shader, then just add the emissive color to the ambient.
				if (_aShaderRenderStates[_nShaderID].nTFactorReg == 0xff)
				{
					if (_aShaderRenderStates[_nShaderID].nEmissiveReg < 0xff)
					{
						if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])
						{
							((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])->ComputeColor( &pDestColor2 );
							pDestColor += pDestColor2;
							pDestColor.Clamp1();
						}
					}
				}

				//Load the ambient color into the register.
				//Note emissive is included ONLY if there is not emissive mask.
	#if FANG_LIGHTING_2X
				FDX8_pDev->SetVertexShaderConstant(CV_MAT_AMBIENT, D3DXVECTOR4(pDestColor.fRed*0.5f, pDestColor.fGreen*0.5f, pDestColor.fBlue*0.5f, pDestColor.fAlpha), 1);
	#else
				FDX8_pDev->SetVertexShaderConstant(CV_MAT_AMBIENT, D3DXVECTOR4(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha), 1);
	#endif
			}
		}

		if (FSh_bSurfaceOnly)
		{
			//Special settings for surface pass only mode.
			fdx8_SetRenderState_ZWRITEENABLE( TRUE );
			fdx8_SetRenderState_ZFUNC( D3DCMP_LESSEQUAL );

			fdx8_SetRenderState_ALPHATESTENABLE( _aShaderRenderStates[_nShaderID].bAlphaTest );
			fdx8_SetRenderState_ALPHAFUNC( _aShaderRenderStates[_nShaderID].nAlphaFunc );
			fdx8_SetRenderState_ALPHAREF( _aShaderRenderStates[_nShaderID].nAlphaRef );
		}
		else
		{
			//Normal settings.
			if ( (_nPassIdx == 0 && FSh_shaderType == SHADERTYPE_DIFFUSE) || FSh_shaderType == SHADERTYPE_TRANSLUCENCY )
			{
				//First pass, must write z values, use lessequal z compare and can do alpha testing.
				fdx8_SetRenderState_ZWRITEENABLE( !!(FSh_ZEnable||FSh_shaderType == SHADERTYPE_DIFFUSE) );
				fdx8_SetRenderState_ZFUNC( D3DCMP_LESSEQUAL );

				fdx8_SetRenderState_ALPHATESTENABLE( _aShaderRenderStates[_nShaderID].bAlphaTest );
				fdx8_SetRenderState_ALPHAFUNC( _aShaderRenderStates[_nShaderID].nAlphaFunc );
				fdx8_SetRenderState_ALPHAREF( _aShaderRenderStates[_nShaderID].nAlphaRef );
			}
			else
			{
				//Subsequent passes do not write z values, use z equal and thus does not need to do alpha testing.
				fdx8_SetRenderState_ZWRITEENABLE( FALSE );
				fdx8_SetRenderState_ZFUNC( D3DCMP_EQUAL );

				fdx8_SetRenderState_ALPHATESTENABLE( FALSE );
			}
		}
	}
}

//Apply inverse view matrix so that:
//Given the resultant matrix as MtxRes and the original as MtxSrc then
//MtxRes * MtxView = MtxSrc since MtxRes = MtxSrc * MtxViewInv
//therefore MtxSrc * MtxViewInv * M\txView = MtxSrc
#if FANG_PLATFORM_XB
	FINLINE void _ApplyInvView(XGMATRIX *pMtx)
	{
		XGMatrixMultiply(pMtx, &FDX8_InvCurrentViewMtx, pMtx);
	}
#else
	FINLINE void _ApplyInvView(D3DXMATRIX *pMtx)
	{
		D3DXMatrixMultiply(pMtx, &FDX8_InvCurrentViewMtx, pMtx);
	}
#endif

//Generates attenuation data for lights.
FINLINE void _GeneratePerPixelPointShaderData(PerPixelPointLight_t *pLight, s32 nIdx)
{
	//Load light data into registers.
	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE_POINT_PP+nIdx, D3DXVECTOR4(pLight->vColor), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_POSITION_PP+nIdx, D3DXVECTOR4(pLight->vPos), 1);	
	//Calculate and load light attenuation.
	D3DXVECTOR4 vAtten;
	vAtten.x = -pLight->vPos.x*pLight->fOOR*0.5f + 0.5f;
	vAtten.y = -pLight->vPos.y*pLight->fOOR*0.5f + 0.5f;
	vAtten.z = -pLight->vPos.z*pLight->fOOR*0.5f + 0.5f;
	vAtten.w = 0.5f*pLight->fOOR;
	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_ATTENUATION+nIdx, vAtten, 1);	
}

//Calculates a look at matrix for lights.
void _LightLookAt(D3DXMATRIX *pLookAt, CFVec3A& camPos, CFVec3A& up, CFVec3A& target)
{
	CFVec3A vLook,vRight,vUp;

    // compute unit target vector
    // use negative value to look down (-Z) axis
    vLook.x = camPos.x - target.x;
    vLook.y = camPos.y - target.y;
    vLook.z = camPos.z - target.z;
    vLook = vLook.Unitize();
    

    // vRight = camUp x vLook
    vRight = vRight.Cross(up, vLook);
    vRight = vRight.Unitize();

    // vUp = vLook x vRight
  	vUp = vUp.Cross(vLook, vRight);

    pLookAt->m[0][0] = vRight.x;
    pLookAt->m[1][0] = vRight.y;
    pLookAt->m[2][0] = vRight.z;
    pLookAt->m[3][0] = -( camPos.x * vRight.x + camPos.y * vRight.y + camPos.z * vRight.z );

    pLookAt->m[0][1] = vUp.x;
    pLookAt->m[1][1] = vUp.y;
    pLookAt->m[2][1] = vUp.z;
    pLookAt->m[3][1] = -( camPos.x * vUp.x + camPos.y * vUp.y + camPos.z * vUp.z );

    pLookAt->m[0][2] = vLook.x;
    pLookAt->m[1][2] = vLook.y;
    pLookAt->m[2][2] = vLook.z;
    pLookAt->m[3][2] = -( camPos.x * vLook.x + camPos.y * vLook.y + camPos.z * vLook.z );

	pLookAt->m[0][3] = 0;
    pLookAt->m[1][3] = 0;
    pLookAt->m[2][3] = 0;
	pLookAt->m[3][3] = 1.0f;
}

//Calculates projection matrix for perspective lights (spotlights).
void _LightPerspective(D3DXMATRIX *pMat, f32 fov)
{
	f32 angle, cot;

    // find the cotangent of half the (YZ) field of view
    angle = fov * 0.5f;
	cot = 0.5f / tanf(angle);

    pMat->m[0][0] =    cot;
    pMat->m[1][0] =    0.0f;
    pMat->m[2][0] =   -0.5f;
    pMat->m[3][0] =    0.0f;

    pMat->m[0][1] =    0.0f;
    pMat->m[1][1] =    cot;
    pMat->m[2][1] =   -0.5f;
    pMat->m[3][1] =    0.0f;

    pMat->m[0][2] =    0.0f;
    pMat->m[1][2] =    0.0f;
    pMat->m[2][2] =   -1.0f;
    pMat->m[3][2] =    0.0f;

	pMat->m[0][3] = 0;
    pMat->m[1][3] = 0;
    pMat->m[2][3] = 1.0f;
	pMat->m[3][3] = 1.0f;
}

//Generates vertex shader data for per-pixel spotlights.
FINLINE void _GenerateSpotShaderData(PerPixelSpotLight_t *pLight, s32 nIdx)
{
	D3DXMATRIX lookAt, proj;
	D3DXMATRIX out;
	D3DXVECTOR4 vAtten;
	CFVec3A up, N;
	CFVec3A target;

	//Convert lookat matrix from light WS matrix.
	lookAt.m[0][0] = pLight->pMtx_WS->m_vRight.x;
    lookAt.m[1][0] = pLight->pMtx_WS->m_vRight.y;
    lookAt.m[2][0] = pLight->pMtx_WS->m_vRight.z;
    lookAt.m[3][0] = -( pLight->vPos.x * pLight->pMtx_WS->m_vRight.x + pLight->vPos.y * pLight->pMtx_WS->m_vRight.y + pLight->vPos.z * pLight->pMtx_WS->m_vRight.z );

    lookAt.m[0][1] = pLight->pMtx_WS->m_vUp.x;
    lookAt.m[1][1] = pLight->pMtx_WS->m_vUp.y;
    lookAt.m[2][1] = pLight->pMtx_WS->m_vUp.z;
    lookAt.m[3][1] = -( pLight->vPos.x * pLight->pMtx_WS->m_vUp.x + pLight->vPos.y * pLight->pMtx_WS->m_vUp.y + pLight->vPos.z * pLight->pMtx_WS->m_vUp.z );

    lookAt.m[0][2] = -pLight->pMtx_WS->m_vFront.x;
    lookAt.m[1][2] = -pLight->pMtx_WS->m_vFront.y;
    lookAt.m[2][2] = -pLight->pMtx_WS->m_vFront.z;
    lookAt.m[3][2] = -( pLight->vPos.x * -pLight->pMtx_WS->m_vFront.x + pLight->vPos.y * -pLight->pMtx_WS->m_vFront.y + pLight->vPos.z * -pLight->pMtx_WS->m_vFront.z );

	lookAt.m[0][3] = 0;
    lookAt.m[1][3] = 0;
    lookAt.m[2][3] = 0;
	lookAt.m[3][3] = 1.0f;

	//Calculate light perspective matrix from light cone angle.
	_LightPerspective(&proj, pLight->fFOV);
	
	//Concat LookAt and Projection to get spotlight matrix.
	D3DXMatrixMultiply(&out, &lookAt, &proj);
	
	D3DXVECTOR3 LPos = D3DXVECTOR3(pLight->vPos.x, pLight->vPos.y, pLight->vPos.z);
	D3DXVECTOR3 LDir = D3DXVECTOR3(-pLight->pMtx_WS->m_vFront.x, -pLight->pMtx_WS->m_vFront.y, -pLight->pMtx_WS->m_vFront.z );

	//Attenuation constant, does a look up into the texkill texture for clipping and used for linear distance attenuation.
	//s = (dp3 Vtx.xyz, vAtten.xyz) + (vAtten.w)
	//t = 0
	vAtten.x = LDir.x; vAtten.y = LDir.y; vAtten.z = LDir.z; vAtten.w = -(LPos.x*LDir.x + LPos.y*LDir.y + LPos.z*LDir.z);

	//Light data for shader.
	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE_PP+nIdx, D3DXVECTOR4(pLight->vColor), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIRECTION_PP+nIdx, D3DXVECTOR4(LDir), 1);
	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_ATTENUATION_PP+nIdx, vAtten, 1);
	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_POSITION_PP+nIdx, D3DXVECTOR4(LPos.x, LPos.y, LPos.z, pLight->fRange2), 1);

	//Load spotlight matrix into the correct registers.
	D3DXMATRIX LightMtx;
	D3DXMatrixTranspose(&LightMtx, &out);
	if (nIdx == 0)
	{
		FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX0_0, &LightMtx(0,0), 4);
	}
	else
	{
		FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX1_0, &LightMtx(0,0), 4);
	}
}

//This pass handles per-pixel lighting for point lights and spotlights.
//Surfaces may have bumpmaps (but it is not required).
FSTATIC void _HandleLightingPass( void )
{
    BOOL bHasBump=FALSE;
	PerPixelPointLight_t *pLight;
	PerPixelSpotLight_t *pSpotLight;
	s32 n, idx, i, s;

	//Load bumpmap if it exists.
	if ( FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP] && (FSh_shaderFlags&FSh_RENDER_BUMP) )
	{
		if ( ((CFTexInst*)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP])->GetTexDef() )
		{
			bHasBump = TRUE;
		}
	}

	if (bHasBump)
	{
		//1 per-pixel point light per pass with bumpmapping.
		n = _nPerPixelPoint - (_nPassIdx-1);
		if (n > 1) n = 1;
		if (n < 0) n = 0;
	}
	else
	{
		//2 per-pixel point lights without bumpmapping.
		//NOTE that it is per-pixel attenuation ONLY, full phong shading would require 1 pass per point light.
		//This can be faked by supplying a constant white, small bumpmap.
		n = _nPerPixelPoint - ( (_nPassIdx-1)<<1 );
		n = (n>2)?(2):(n);
		if (n < 0) n = 0;
	}

	//Clear out the texture cache.
	for (i=0; i<4; i++)
	{
		fdx8tex_SetTexture(i, NULL, i);
	}

	D3DXMATRIX xzMtx, yMtx;
	D3DXMATRIX texKillMtx;
	D3DXMATRIX spotMtx;

	//***********************************
	//PER-PIXEL POINT LIGHTS
	if (n)
	{
		//Calc light index based on the current pass #
		if (bHasBump)
		{
			idx = _nPassIdx-1;
		}
		else
		{
			idx = (_nPassIdx-1)<<1;
		}

		if (n == 1)
		{
			if (bHasBump)
			{
				_SetVertexShader(VSHADER_BASE_PERPIXELPOINT1_BUMP);
				_SetPixelShader(PSHADER_PERPIXEL_POINT1_BUMP);
			}
			else
			{
				_SetVertexShader(VSHADER_BASE_PERPIXELPOINT1);
				_SetPixelShader(PSHADER_PERPIXEL_POINT1);
			}
		}
		else //if n>1, then it must be an per-pixel attenuation only light.
		{
			_SetVertexShader(VSHADER_BASE_PERPIXELPOINT2);
			_SetPixelShader(PSHADER_PERPIXEL_POINT2);
		}

		for (i=0, s=0; i<n; i++, s+=2)
		{
			//Attenuation map, using A(x,y) + A(z)
			FDX8_pDev->SetTexture(s, _pAttenMap);
			FDX8_pDev->SetTexture(s+1, _pAttenMap);

			if (bHasBump)
			{
				//Setup bumpmap and normalization cubemap. Per-Pixel normalization is very important when using large polygons.
				ftex_SetTexAddress(s+2, TRUE, TRUE);
				fdx8tex_SetTexture(s+2, (CFTexInst *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP], s+2);
				FDX8_pDev->SetTexture(s+3, _pNormalizeCube);
			}
		
			//Generate per-pixel data such as attenuation.
			pLight = &_aPerPixelPointLights[i+idx];
			_GeneratePerPixelPointShaderData(pLight, i);

			//Attenuation textures must be clamped.
			ftex_SetTexAddress(s, FALSE, FALSE);
			ftex_SetTexAddress(s+1, FALSE, FALSE);

			//Normalization cube map must be clamped to work correctly.
			if (s == 0)
			{
				ftex_SetTexAddress(s+3, FALSE, FALSE, FALSE);
			}
		}
	}
	//***********************************
	//PER-PIXEL SPOT LIGHTS
	else if (_nPerPixelSpot)
	{
		//1 per-pixel spot light per pass with bumpmapping.
		if (bHasBump)
		{
			n = 1;

			idx = _nPassIdx - _nPerPixelPoint - 1;
		}
		//2 per-pixel spot lights per pass without bumpmapping.
		else
		{
			n = _nPerPixelSpot - ( (_nPassIdx-((_nPerPixelPoint+1)>>1)-1)<<1 );
			n = (n>2)?(2):(n);

			idx = ( (_nPassIdx-((_nPerPixelPoint+1)>>1)-1)<<1 );
		}

		if (n == 1)
		{
			if (bHasBump)
			{
				_SetVertexShader(VSHADER_BASE_PERPIXELSPOT1_BUMP);
				_SetPixelShader(PSHADER_PERPIXEL_SPOT1_BUMP);
			}
			else
			{
				_SetVertexShader(VSHADER_BASE_PERPIXELSPOT1);
				_SetPixelShader(PSHADER_PERPIXEL_SPOT1);
			}
		}
		else
		{
			_SetVertexShader(VSHADER_BASE_PERPIXELSPOT2);
			_SetPixelShader(PSHADER_PERPIXEL_SPOT2);
		}
		
		for (i=0, s=0; i<n; i++, s+=2)
		{
			pSpotLight = &_aPerPixelSpotLights[i+idx];

			//Spotlight texture.
			FDX8_pDev->SetTexture(s, pSpotLight->pDXTex);
			//Texkill texture, used to clamp to light plane defined by light direction as plane normal.
			FDX8_pDev->SetTexture(s+1, _pTexKill);

			if (bHasBump)
			{
				//Load bumpmap and normalization cubemap.
				fdx8tex_SetTexture(s+2, (CFTexInst *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP], s+2);
				FDX8_pDev->SetTexture(s+3, _pNormalizeCube);

				//bumpmap can wrap.
				ftex_SetTexAddress(s+2, TRUE, TRUE);

				//normalization cubemap must be clamped to work correctly.
				ftex_SetTexAddress(s+3, FALSE, FALSE, FALSE);

				fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( s+2, D3DTTFF_DISABLE);
				fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( s+3, D3DTTFF_DISABLE);
			}
			
			//Spot texture and TexKill texture must be clamped to work correctly.
			ftex_SetTexAddress(s, FALSE, FALSE);

			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( s, D3DTTFF_DISABLE);
			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS( s+1, D3DTTFF_DISABLE);
			ftex_SetTexAddress(s+1, FALSE, FALSE);

			#if FANG_PLATFORM_XB
			if (i == 1)
			{
				FDX8_pDev->SetPixelShaderConstant(0, &pSpotLight->vColor, 1);	
			}
			#endif

			
			//Generate spotlight matrix, texkill matrix and attenuation data.
			_GenerateSpotShaderData(pSpotLight, i);
		}
	}

	_bRestoreClamp=TRUE;
}

void _GenerateDirectionalShaderData(CFLight *pDirLight)
{
	//Load vertex shader data for directional specular light.
	D3DXVECTOR3 vDir = D3DXVECTOR3(-pDirLight->m_vUnitDir_VS.x, -pDirLight->m_vUnitDir_VS.y, -pDirLight->m_vUnitDir_VS.z);

	FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIRECTION_PP, D3DXVECTOR4(vDir), 1);
	CFColorRGBA pDestColor;
	if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMOTIF])
	{
		((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMOTIF])->ComputeColor( &pDestColor );
		FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE_PP, D3DXVECTOR4(pDirLight->m_ScaledColor.fRed*pDestColor.fRed, pDirLight->m_ScaledColor.fGreen*pDestColor.fGreen, pDirLight->m_ScaledColor.fBlue*pDestColor.fBlue, 1.0f), 1);
	}
	else
	{
		FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE_PP, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
	}
}

void fsh_PrepareFrame()
{
	//Used on GC?
}

FSTATIC void _HandleSpecularPass( void )
{
	//Special handling for specular pass.
	//Will be changing this week to have better specular support.
	if ( !FSh_pSpecularLight )
	{
		return;
	}

	LPDIRECT3DTEXTURE8 pSpecTex;
	if ( FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP] && (FSh_shaderFlags&FSh_RENDER_BUMP) )
	{
		if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP])
		{
			if (*((f32 *)&FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP]) <= 0.2f)
			{
				pSpecTex = _pSpec16;
			}
			else if (*((f32 *)&FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP]) <= 0.4f)
			{
				pSpecTex = _pSpec32;
			}
			else
			{
				pSpecTex = _pSpec48;
			}
		}
		else
		{
			pSpecTex = _pSpec32;
		}
		
		_SetVertexShader(VSHADER_BASE_PERPIXELDIRECTIONAL1_SPECBUMP);
		_SetPixelShader(PSHADER_PERPIXEL_DIRECTIONAL1_SPECBUMP);

		fdx8tex_SetTexture(0, (CFTexInst *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP], 0);
		FDX8_pDev->SetTexture(1, pSpecTex);
		FDX8_pDev->SetTexture(2, pSpecTex);
				
		ftex_SetTexAddress(0, TRUE, TRUE);

		ftex_SetTexAddress(1, FALSE, FALSE);
		ftex_SetTexAddress(2, FALSE, FALSE);

		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS(0, D3DTTFF_DISABLE);
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS(1, D3DTTFF_DISABLE);
		fdx8_SetTextureState_TEXTURETRANSFORMFLAGS(2, D3DTTFF_DISABLE);

		FDX8_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
		FDX8_pDev->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2 );
		FDX8_pDev->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3 );

		_GenerateDirectionalShaderData(FSh_pSpecularLight);
	}
	else
	{
		D3DXVECTOR3 LDir= D3DXVECTOR3(-FSh_pSpecularLight->m_vUnitDir_VS.x, -FSh_pSpecularLight->m_vUnitDir_VS.y, -FSh_pSpecularLight->m_vUnitDir_VS.z);
			
		FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIRECTION_PP, D3DXVECTOR4(LDir), 1);

		if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMOTIF])
		{
			CFColorRGBA pDestColor;
			((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMOTIF])->ComputeColor( &pDestColor );

			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE_PP, D3DXVECTOR4(FSh_pSpecularLight->m_ScaledColor.fRed*pDestColor.fRed, FSh_pSpecularLight->m_ScaledColor.fGreen*pDestColor.fGreen, FSh_pSpecularLight->m_ScaledColor.fBlue*pDestColor.fBlue, 1.0f), 1);
		}
		else
		{
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE_PP, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
		}

		f32 fPwr;
		if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP])
		{
			fPwr = *((f32 *)&FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP]);
			fPwr *= 100.0f;

			if (fPwr > 100.0f)
			{
				fPwr = 32.0f;
			}
		}
		else
		{
			fPwr = 32.0f;
		}

		FDX8_pDev->SetVertexShaderConstant(CV_MATPOWER, D3DXVECTOR4(fPwr, fPwr, fPwr, fPwr), 1);
		FDX8_pDev->SetVertexShaderConstant(CV_EYE_VECTOR, D3DXVECTOR4(0, 0, -1, 1.0f), 1);

		FDX8_pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
		FDX8_pDev->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 2 );
		FDX8_pDev->SetTextureStageState( 3, D3DTSS_TEXCOORDINDEX, 3 );

		if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK])
		{
			fdx8tex_SetTexture(0, (CFTexInst *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK], 0);

			if ( _nShaderID == (FSHADERS_VSPEC_BASIC + FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT) )
			{
				_nShaderID = (FSHADERS_VSPEC_MASK + FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT);
			}
			else if ( _nShaderID == (FSHADERS_VSPEC_BASIC_CO + FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT) )
			{
				_nShaderID = (FSHADERS_VSPEC_BASIC_MASK_CO + FSHADERS_SHADER_COUNT + FSHADERS_DIFFUSE_COUNT);
			}
		}

		_SetVertexShader(_aShaderRenderStates[_nShaderID].vShader);
		_SetPixelShader(_aShaderRenderStates[_nShaderID].pShader);
	}
}

BOOL _IsProceduralShader()
{
	if (_nShaderID >= FSHADERS_LIQUID_ENV && _nShaderID <= FSHADERS_LIQUID_MOLTEN_2LAYER)
	{
		return TRUE;
	}
	return FALSE;
}

void _HandleProcedural(u32 vShader, u32 pShader, CFLiquidVolume *pLiquid)
{
	u32 i, j, nTexLayerID;

	CFTexInst *pEMBM = pLiquid->GetEMBM();
	FShTexInst_t *pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[2];  //This is the reflection texture.
	FShTexInst_t *pShTexInst2;
	if (!pShTexInst)
	{
		pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[0];
	}

	//Setup texturecord overrides.
	for (i=0; i<2; i++)
	{
		if( _nTexCoordMtxCount == 0 ) {
			// No TC matrices...
			FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
		} else {
			D3DXMATRIX mTmp;
			// There is at least one TC matrix...

			nTexLayerID = pShTexInst->nTexLayerID;

			for( j=0; j<_nTexCoordMtxCount; j++ ) 
			{
				if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
				{
					// This stage's texture is flagged for TC animation...
					mTmp.m[0][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][0];
					mTmp.m[0][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][0];
					mTmp.m[0][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][0];
					mTmp.m[1][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][1];
					mTmp.m[1][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][1];
					mTmp.m[1][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][1];
					FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &mTmp(0,0), 2);
					break;
				}
			}
			if( j == _nTexCoordMtxCount ) 
			{
				// This stage's texture is not flagged for TC animation...
				FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
			}
		}
	}

	if (_nShaderID == FSHADERS_LIQUID_MOLTEN_1LAYER)
	{
		pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[0];

		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11,   F2DW( 0.05f ) );

		FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PASSTHRU_TC1]);
		_nCurrentVtxShader = _anVShader_Handle[VSHADER_PASSTHRU_TC1];
		_SetPixelShader(MOLTEN_1LAYER_EMBM);
		fdx8tex_SetTexture(0, pEMBM, 0);
		fdx8tex_SetTexture(1, &pShTexInst->TexInst, 1);
	}
	else if (_nShaderID == FSHADERS_LIQUID_MOLTEN_2LAYER)
	{
		pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[0];
		pShTexInst2 = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[2];

		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11,   F2DW( 0.05f ) );

		FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PASSTHRU_TC1]);
		_nCurrentVtxShader = _anVShader_Handle[VSHADER_PASSTHRU_TC1];
		_SetPixelShader(MOLTEN_2LAYER_EMBM);

		fdx8tex_SetTexture(0, pEMBM, 0);
		fdx8tex_SetTexture(1, &pShTexInst->TexInst, 1);
		fdx8tex_SetTexture(2, &pShTexInst2->TexInst, 2);
	}
	else if (_nShaderID == FSHADERS_LIQUID_TEXTURE)
	{
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10,   F2DW( 0.05f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11,   F2DW( 0.05f ) );

		FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PASSTHRU_TC1]);
		_nCurrentVtxShader = _anVShader_Handle[VSHADER_PASSTHRU_TC1];
		_SetPixelShader(MOLTEN_1LAYER_EMBM);
		fdx8tex_SetTexture(0, pEMBM, 0);
		fdx8tex_SetTexture(1, &pShTexInst->TexInst, 1);
	}
	else
	{
		//This is a multi-pass procedural surface shader. The ONLY multipass surface shader for generic meshs.
		for (i=0; i<4; i++)
		{
			FDX8_pDev->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | i);
			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS(i, D3DTTFF_DISABLE);
		}

		if ( _nPassIdx == 0 )
		{
			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_SRCALPHA);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_INVSRCALPHA);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

			fdx8tex_SetTexture(0, pEMBM, 0);

			ftex_SetTexAddress(0, TRUE, TRUE);

			fdx8tex_SetTexture(1, &pShTexInst->TexInst, 1);
			fdx8tex_SetTexture(2, &pShTexInst->TexInst, 2);
			fdx8tex_SetTexture(3, &pShTexInst->TexInst, 3);

			ftex_SetTexAddress(3, TRUE, TRUE, TRUE);
			
			CFVec4 vCen;
			vCen.Set( 0, 0, 0, 1 );//pPlane->vCen.x, pPlane->vCen.y, pPlane->vCen.z, 1.0f);
			FDX8_pDev->SetVertexShaderConstant(CV_FIXED_COLOR, &vCen, 1);
			
			FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PLANAR_REFLECT_MESH]);
			_nCurrentVtxShader = _anVShader_Handle[VSHADER_PLANAR_REFLECT_MESH];
			_SetPixelShader(PSHADER_CUBE_REFLECT_VTXALPLHA);
		}
		else
		{
			//I want do Pass0*Pass1*2, not exactly correct if the shader is translucent but it looks pretty cool.
			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_DESTCOLOR);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_SRCCOLOR);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

			FShTexInst_t *pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[0];
			if (pShTexInst)
			{
				fdx8tex_SetTexture( 0, &pShTexInst->TexInst, 0 );
			}
			
			FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PASSTHRU_TC1]);
			_nCurrentVtxShader = _anVShader_Handle[VSHADER_PASSTHRU_TC1];
			_SetPixelShader(PSHADER_PASSTHRU);
		}
	}

	//Load a white constant for the shader to use.
	f32 afData[4];
	afData[0] = 1.0f;
	afData[1] = 1.0f;
	afData[2] = 1.0f;
	afData[3] = 1.0f;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
	//
}

void _HandleFastShader()
{
	BOOL bRemap=TRUE;
	u32 vShader = _aShaderRenderStates[_nShaderID].vShader;
	u32 pShader = _aShaderRenderStates[_nSurfaceShaderID].pShader;

	//if (pShader == PSHADER_oBASE_ADD_rbENV)
	if (m_nLMMode > LM_NONE && fdx8sh_bStream1Set && FSh_pnLightMapInputRegisters[1] && !fsh_bUseExtColorStream)
	{
		bRemap = FALSE;
		vShader = VSHADER_ENV_LM;
		pShader = PSHADER_oBASE_ADD_rbENV_LIT_LM;

		FShTexInst_t *pShTexInst = (FShTexInst_t *)FSh_pnLightMapInputRegisters[1];
		if (pShTexInst)
		{
			if( (u32)pShTexInst < 0xffffffff ) 
			{
				//Make sure that my lightmap texture does not wrap. This is CRITICAL for filtering.
				pShTexInst->TexInst.ClearFlag(CFTexInst::FLAG_WRAP_S);
				pShTexInst->TexInst.ClearFlag(CFTexInst::FLAG_WRAP_T);

				ftex_SetTexAddress(2, FALSE, FALSE);
				fdx8tex_SetTexture(2, &pShTexInst->TexInst, 2);
			} 
			else 
			{
				DEVPRINTF( "Error:  Lightmap texture = 0xffffffff\n" );
			}
		}
	}
	else
	{
		vShader = VSHADER_BASE_ENV_POINT1_DIR1;
		pShader = PSHADER_oBASE_ADD_rbENV_LIT;
	}

	int i, n;
	FShTexInst_t *pShTexInst;
	BOOL bSetZMask=FALSE;
	BOOL bSetSMask=FALSE;
	
	//Number of texture stages
	n = _aShaderRenderStates[_nSurfaceShaderID].nStages;
	if (bRemap)
	{
		_SetVertexShader(vShader);
		_SetPixelShader(pShader);
	}
	else
	{
		if (_nCurrentVtxShader != vShader)
		{
			FDX8_SetVertexShader( _anVShader_Handle[vShader] );
			_nCurrentVtxShader = vShader;
		}
		if (_nCurrentPixelShader != pShader)
		{
			FDX8_pDev->SetPixelShader( _anPShader_Handle[pShader-1] );
			_nCurrentPixelShader = pShader;

			//I have to set my pixel shader constants if the shader changes.
			//The XBox flushes all shader constants when the shader is set.

			_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = _fLayerAlpha;
			FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);

			_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
			_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
			_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
			_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
			FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
		}
		else
		{
			//I haven't changed shaders so I can set the constants only if they changed.
			if (_fLayerAlpha != _afPixelConst3[0])
			{
				_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = _fLayerAlpha;
				FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);
			}

			if (_afPixelConst2[0] != _fFinalSurfaceColor.fRed || _afPixelConst2[1] != _fFinalSurfaceColor.fGreen || _afPixelConst2[2] != _fFinalSurfaceColor.fBlue || _afPixelConst2[3] != _fFinalSurfaceColor.fAlpha)
			{
				_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
				_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
				_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
				_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
				FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
			}
		}
	}

	if (_aShaderRenderStates[_nSurfaceShaderID].nTFactorReg < 0xff && FSh_pnSurfaceInputRegisters[_aShaderRenderStates[_nSurfaceShaderID].nTFactorReg])
	{
		CFColorRGBA pDestColor;
		//TFactor is used for things such as envmap motif. Its a generic "FACTOR" field.
		const CFColorMotif *pMotif = (const CFColorMotif *)FSh_pnSurfaceInputRegisters[_aShaderRenderStates[_nSurfaceShaderID].nTFactorReg];
		if (pMotif->nMotifIndex == 0)
		{
			pMotif->ComputeColor( &pDestColor );
			pDestColor = CFColorRGBA(0.10f, 0.10f, 0.10f, 0.0f);
			FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha), 1);
		}
		else //Invalid motif index! Set to default color.
		{
			FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
		}
	}

	_nCurrentShader = _nShaderID;

	u32 nTexLayerID, j;
	CFTexInst TexInst;
	D3DXMATRIX mTmp;

	//Here I have to load my textures and setup my texture matrices.
    if (pShader)
	{
		for (i=0; i<2; i++)
		{
			FDX8_pDev->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | i);
			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS(i, D3DTTFF_DISABLE);
		}

		u8 nMask=0;
		for (i=0; i<2; i++)
		{
			_aTexOverrideCache[i].pOrig = _aTexOverrideCache[i].pCur = NULL;
			_aTexOverrideCache[i].nTexLayerID = 0xffffffff;
			if (_aShaderStageStates[_nSurfaceShaderID][i].nTexReg > -1 && i < n)
			{
				//Texture.
				pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[i*2];
				
				if (pShTexInst)
				{
					_aTexOverrideCache[i].pOrig = &pShTexInst->TexInst;
					_aTexOverrideCache[i].pCur = _aTexOverrideCache[i].pOrig;
					_aTexOverrideCache[i].nTexLayerID = pShTexInst->nTexLayerID;
				}
													
				if( _nTexOverrideCount == 0 ) 
				{
					fdx8tex_SetTexture( i, &pShTexInst->TexInst, i );
					nMask++;
				} 
				else 
				{
					nTexLayerID = pShTexInst->nTexLayerID;

					for( j=0; j<_nTexOverrideCount; j++ ) 
					{
						if( nTexLayerID == _aTexOverride[j].nTexLayerID ) 
						{
							// This stage's texture is flagged for texture override...
							TexInst = pShTexInst->TexInst;
							if (_aTexOverride[j].pTexInst)
							{
								TexInst.SetTexDef( _aTexOverride[j].pTexInst->GetTexDef() );

								_aTexOverrideCache[i].pCur = _aTexOverride[j].pTexInst;
																	
								fdx8tex_SetTexture( i, &TexInst, i );

								nMask++;
							}
							break;
						}
					}
					if( j == _nTexOverrideCount ) 
					{
						// This stage's texture is not flagged for texture override...
						fdx8tex_SetTexture( i, &pShTexInst->TexInst, i );
						nMask++;
					}
				}

				//Texture Matrix.
				if( _nTexCoordMtxCount == 0 ) 
				{
					// No TC matrices...
					FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
				} 
				else 
				{
					// There is at least one TC matrix...

					nTexLayerID = pShTexInst->nTexLayerID;

					for( j=0; j<_nTexCoordMtxCount; j++ ) 
					{
						if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
						{
							// This stage's texture is flagged for TC animation...
							mTmp.m[0][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][0];
							mTmp.m[0][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][0];
							mTmp.m[0][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][0];
							mTmp.m[1][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][1];
							mTmp.m[1][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][1];
							mTmp.m[1][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][1];
							FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &mTmp(0,0), 2);
							break;
						}
					}
					if( j == _nTexCoordMtxCount ) 
					{
						// This stage's texture is not flagged for TC animation...
						FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
					}
				}
			}
		}
		//Set my z mask after the normal textures are loaded.
		if (bSetZMask && nMask < 2)
		{
			pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK];
			if (pShTexInst)
			{
				fdx8tex_SetTexture( nMask, &pShTexInst->TexInst, nMask );
				nMask++;
			}
		}
		//Set my specular/envmap mask after normal textures and z mask.
		if (bSetSMask && nMask < 2)
		{
			pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK];
			if (pShTexInst)
			{
				fdx8tex_SetTexture( nMask, &pShTexInst->TexInst, nMask );
				nMask++;
			}
		}
	}
	_nPrevTexOverrideCount = _nTexOverrideCount;
	_nPrevTexCoordMtxCount = _nTexCoordMtxCount;
}

void _SetPassDXState()
{
	if (_bFastShader&&FSh_bUseFastPass) //FAST SHADER HERE.
	{
		_HandleFastShader();
		return;
	}

	//Handle additional lighting passes seperately.
	if (_nPassIdx > 0 && FSh_shaderType == SHADERTYPE_DIFFUSE)
	{
		_HandleLightingPass();
		return;
	}
	else if (FSh_shaderType == SHADERTYPE_SPECULAR)
	{
		_HandleSpecularPass();
		return;
	}

	u32 vShader = _aShaderRenderStates[_nShaderID].vShader;
	u32 pShader = _aShaderRenderStates[_nShaderID].pShader;

	//Handle special cases here: Shadow Rendering and Procedural liquid shaders.
	if (FSh_bShadowRender)
	{
		vShader = VSHADER_BASE_PASSTHRU;
		pShader = PSHADER_COLOR;

		fdx8_SetRenderState_CULLMODE( D3DCULL_NONE );
	}
	else if (_nShaderID >= FSHADERS_LIQUID_ENV && _nShaderID <= FSHADERS_LIQUID_MOLTEN_2LAYER)
	{
		CFLiquidVolume *pLiquid=NULL;

		pLiquid = (CFLiquidVolume *)FSh_pProcedural;	
		m_nLMMode = LM_NONE;

		_HandleProcedural(vShader, pShader, pLiquid);
		return;
	}

	//HANDLE DETAIL TILE & BUMP TILE
	if ( _bSetDetailScale && (FSh_shaderFlags&FSh_RENDER_DETAIL) ) //This is set in fsh_SetShader(), which also sets the register to read it from.
	{
		fsh_SetDetailMapTileFactor( *((f32 *)&FSh_pnSurfaceInputRegisters[_nDetailReg]) );
	}

	//Set the bumpmap tile if its the first diffuse lighting pass & the shader has a bumpmap.
	if ( FSh_shaderType == SHADERTYPE_DIFFUSE && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP] && (FSh_shaderFlags&FSh_RENDER_BUMP) )
	{
		fsh_SetBumpMapTileFactor( *((f32 *)&FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP_TILE_FACTOR]) );
	}
	//
    
	//OK the main path.
	int i, n;
	FShTexInst_t *pShTexInst;
	
	//Number of texture stages
	n = _aShaderRenderStates[_nShaderID].nStages;

	if ( FSh_shaderFlags&FSh_RENDER_FORCEBASE_SHADER )
	{
		n = (n>0)?(1):(0);
	}
	else if ( _bSetDetailScale && !(FSh_shaderFlags&FSh_RENDER_DETAIL) )
	{
		n--;
	}

	//Set vertex shader, if lightmaps are present the mode will be overriden.
	if (vShader > 0)
	{
		if (m_nLMMode > LM_NONE && !FSh_bShadowRender)
		{
			switch (m_nLMMode)
			{
				case LM4:
					vShader = VSHADER_BASE_LM4;
					break;
				case ZMASK_EMASK_LM2:
					vShader = VSHADER_BASE_ZMASK_EMASK_LM2;
					break;
				case ZMASK_LM3:
                    vShader = VSHADER_BASE_ZMASK_LM3;
					break;
				case EMASK_LM3:
					vShader = VSHADER_BASE_EMASK_LM3;
					break;
			};
		}
		//load shader here.
		_SetVertexShader(vShader);
	}
	else
	{
		//If a requested vertex shader doesn't exist, fallback to the last used fixed function shader.
		//NOTE: if this happens then BAD things are occuring.
		FDX8_SetVertexShader(_nFixedVShader);
		_nCurrentVtxShader = _nFixedVShader;
	}

	BOOL bSetZMask=FALSE;
	BOOL bSetSMask=FALSE;

	//if ( _nCurrentShader != _nShaderID || 1)
	//Since some meshs have lightmaps and stuff
	{
#if FPERF_ENABLE
		FPerf_nRawShSwitchCount++;
#endif
		if (pShader > 0)
		{
			//Mesh has lightmaps.
			if (m_nLMMode > LM_NONE && !FSh_bShadowRender)
			{
				switch (m_nLMMode)
				{
					case LM4:
						pShader = PSHADER_LM4;
						break;
					case ZMASK_EMASK_LM2:
						pShader = PSHADER_ZMASK_EMASK_LM2;
						break;
					case ZMASK_LM3:
						pShader = PSHADER_ZMASK_LM3;
						break;
					case EMASK_LM3:
						pShader = PSHADER_EMASK_LM3;
						break;
				};
			}
			else
			{
				//Material has a seperate ZMask
				if ( (_aShaderRenderStates[_nShaderID].nMask & MASK_Z) && (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK]) )
				{
					if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK]!=FSh_pnSurfaceInputRegisters[0])
					{
						switch (pShader)
						{
							case PSHADER_PASSTHRU:
								pShader = PSHADER_PASSTHRU_ALPHAMASK;
								bSetZMask = TRUE;
								break;
							case PSHADER_oBASE_LERP_tLAYER:
								pShader = PSHADER_oBASE_LERP_tLAYER_ALPHAMASK;
								bSetZMask = TRUE;
								break;
							case PSHADER_oBASE_ADD_rbENV:
								{
									if ( (_aShaderRenderStates[_nShaderID].nMask & MASK_S) && (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK]) && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK]!=FSh_pnSurfaceInputRegisters[0])
									{
										pShader = PSHADER_oBASE_ADD_rbENV_SAMASK;
										bSetSMask = TRUE;
									}
									else
									{
										pShader = PSHADER_oBASE_ADD_rbENV_AMASK;
									}
									bSetZMask = TRUE;
								}
								break;
						}
					}
				}
				else if ( (_aShaderRenderStates[_nShaderID].nMask & MASK_S) && (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK]) )
				{
					//Material has a seperate Specular Mask - used with environment mapping.
					if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK]!=FSh_pnSurfaceInputRegisters[0])
					{
						switch (pShader)
						{
							case PSHADER_oBASE_ADD_rbENV:
								pShader = PSHADER_oBASE_ADD_rbENV_SMASK;
								bSetSMask = TRUE;
							break;
						};
					}
				}
			}
			//load shader here.
			_SetPixelShader(pShader);

			//Shadow creation data.
			if (FSh_bShadowRender)
			{
				f32 afData[4];
				//Object ID = green
				afData[0] = (f32)FSh_ShadowID*_fOO255;
				afData[1] = (f32)FSh_ShadowID*_fOO255;
				//Object ID > 0 = blue
				afData[2] = (FSh_ShadowID > 0)?(1.0f):(0.0f);
				afData[3] = (f32)FSh_ShadowID*_fOO255;
				FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
			}
		}
		
		_nCurrentShader = _nShaderID;
	}

	u32 nTexLayerID, j;
	CFTexInst TexInst;
	D3DXMATRIX mTmp;

	FASSERT(pShader);

	//Here I have to load my textures and setup my texture matrices.
    if (pShader)
	{
		for (i=0; i<4; i++)
		{
			FDX8_pDev->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | i);
			fdx8_SetTextureState_TEXTURETRANSFORMFLAGS(i, D3DTTFF_DISABLE);
		}

		//This must be a translucent or surface shader.
		if (FSh_shaderType != SHADERTYPE_DIFFUSE)
		{
			u8 nMask=0;
			for (i=0; i<4; i++)
			{
				_aTexOverrideCache[i].pOrig = _aTexOverrideCache[i].pCur = NULL;
				_aTexOverrideCache[i].nTexLayerID = 0xffffffff;
				if (_aShaderStageStates[_nShaderID][i].nTexReg > -1 && i < n)
				{
					//Texture.
					if (i < 3)
					{
						pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[i*2];
					}
					else
					{
						pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[5];
					}

					if (pShTexInst)
					{
						_aTexOverrideCache[i].pOrig = &pShTexInst->TexInst;
						_aTexOverrideCache[i].pCur = _aTexOverrideCache[i].pOrig;
						_aTexOverrideCache[i].nTexLayerID = pShTexInst->nTexLayerID;
					}
														
					if (_pReflectTex && i==1 && _nShaderID == FSHADERS_oBASE_ADD_rbSREFLECT)
					{
						fdx8tex_SetTexture( i, _pReflectTex, i );
						nMask++;
					}
					else if( _nTexOverrideCount == 0 ) 
					{
						fdx8tex_SetTexture( i, &pShTexInst->TexInst, i );
						nMask++;
					} 
					else 
					{
						nTexLayerID = pShTexInst->nTexLayerID;

						for( j=0; j<_nTexOverrideCount; j++ ) 
						{
							if( nTexLayerID == _aTexOverride[j].nTexLayerID ) 
							{
								// This stage's texture is flagged for texture override...
								TexInst = pShTexInst->TexInst;
								if (_aTexOverride[j].pTexInst)
								{
									TexInst.SetTexDef( _aTexOverride[j].pTexInst->GetTexDef() );

									_aTexOverrideCache[i].pCur = _aTexOverride[j].pTexInst;
																		
									fdx8tex_SetTexture( i, &TexInst, i );
									nMask++;
								}
								break;
							}
						}
						if( j == _nTexOverrideCount ) 
						{
							// This stage's texture is not flagged for texture override...
							fdx8tex_SetTexture( i, &pShTexInst->TexInst, i );
							nMask++;
						}
					}

					//Texture Matrix.
					if( _nTexCoordMtxCount == 0 ) 
					{
						// No TC matrices...
						FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
					} 
					else 
					{
						// There is at least one TC matrix...

						nTexLayerID = pShTexInst->nTexLayerID;

						for( j=0; j<_nTexCoordMtxCount; j++ ) 
						{
							if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
							{
								// This stage's texture is flagged for TC animation...
								mTmp.m[0][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][0];
								mTmp.m[0][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][0];
								mTmp.m[0][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][0];
								mTmp.m[1][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][1];
								mTmp.m[1][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][1];
								mTmp.m[1][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][1];
								FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &mTmp(0,0), 2);
								break;
							}
						}
						if( j == _nTexCoordMtxCount ) 
						{
							// This stage's texture is not flagged for TC animation...
							FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
						}
					}
				}
			}
			//Set my z mask after the normal textures are loaded.
			if (bSetZMask && nMask < 4)
			{
				pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK];
				if (pShTexInst)
				{
					fdx8tex_SetTexture( nMask, &pShTexInst->TexInst, nMask );
					nMask++;
				}
			}
			//Set my specular/envmap mask after normal textures and z mask.
			if (bSetSMask && nMask < 4)
			{
				pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK];
				if (pShTexInst)
				{
					fdx8tex_SetTexture( nMask, &pShTexInst->TexInst, nMask );
					nMask++;
				}
			}
		}
		else if (m_nLMMode == LM_NONE)
		{
			pShTexInst = NULL;
			//A Diffuse shader with no lightmaps - note that this must be the base pass.
			FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0, &_D3DIdentMtx(0,0), 2);
			FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX1_0, &_D3DIdentMtx(0,0), 2);

			//I have an emissive mask, I load it into the first texture slot (slot 0).
			if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK])
			{
				pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK];
				if (pShTexInst)
				{
					fdx8tex_SetTexture( 0, &pShTexInst->TexInst, 0 );
				}
			}
			//I have a ZMask, I load it into texture slot 1
			else if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK])
			{
				pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK];
				if ( pShTexInst )
				{
					if( _nTexOverrideCount != 0 ) 
					{
						nTexLayerID = pShTexInst->nTexLayerID;

						for( j=0; j<_nTexOverrideCount; j++ ) 
						{
							if( nTexLayerID == _aTexOverride[j].nTexLayerID ) 
							{
								// This stage's texture is flagged for texture override...
								TexInst = pShTexInst->TexInst;
								if (_aTexOverride[j].pTexInst)
								{
									TexInst.SetTexDef( _aTexOverride[j].pTexInst->GetTexDef() );
									fdx8tex_SetTexture( 0, &TexInst, 0 );
								}
								break;
							}
						}
						if( j == _nTexOverrideCount ) 
						{
							// This stage's texture is not flagged for texture override...
							fdx8tex_SetTexture( 0, &pShTexInst->TexInst, 0 );
						}
					}
					else
					{
						fdx8tex_SetTexture( 0, &pShTexInst->TexInst, 0 );
					}
				}
			}

			//Texture Matrix.
			if( _nTexCoordMtxCount > 0 )  
			{
				// There is at least one TC matrix...
				if (pShTexInst)
				{
					nTexLayerID = pShTexInst->nTexLayerID;

					for( j=0; j<_nTexCoordMtxCount; j++ ) 
					{
						if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
						{
							// This stage's texture is flagged for TC animation...
							mTmp.m[0][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][0];
							mTmp.m[0][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][0];
							mTmp.m[0][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][0];
							mTmp.m[1][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][1];
							mTmp.m[1][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][1];
							mTmp.m[1][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][1];
							FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0, &mTmp(0,0), 2);
							break;
						}
					}
					if( j == _nTexCoordMtxCount ) 
					{
						// This stage's texture is not flagged for TC animation...
						FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0, &_D3DIdentMtx(0,0), 2);
					}
				}
			}
		}
		if (m_nLMMode > LM_NONE && m_nLM > 0)// && FSh_pnLightMapInputRegisters)
		{
			//This is a diffuse shader AND I have lightmaps for this mesh.
			u32 nTex=0;
			FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0, &_D3DIdentMtx(0,0), 2);
			FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX1_0, &_D3DIdentMtx(0,0), 2);
			//Emissive mask, set in the first available texture slot.
			if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK])
			{
				pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK];
				if (pShTexInst)
				{
					fdx8tex_SetTexture( nTex, &pShTexInst->TexInst, nTex );
					nTex++;
				}
			}
			//Zmask, set in the first available texture slot after the emissive mask (if it exists).
			if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK])
			{
				pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK];
				if (pShTexInst)
				{
					fdx8tex_SetTexture( nTex, &pShTexInst->TexInst, nTex );
					nTex++;
				}
			
				//Texture Matrix.
				if ( _nTexCoordMtxCount > 0 && pShTexInst )
				{
					// There is at least one TC matrix...
					nTexLayerID = pShTexInst->nTexLayerID;

					for( j=0; j<_nTexCoordMtxCount; j++ ) 
					{
						if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
						{
							// This stage's texture is flagged for TC animation...
							mTmp.m[0][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][0];
							mTmp.m[0][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][0];
							mTmp.m[0][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][0];
							mTmp.m[1][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][1];
							mTmp.m[1][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][1];
							mTmp.m[1][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][1];
							FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0, &mTmp(0,0), 2);
							break;
						}
					}
					if( j == _nTexCoordMtxCount ) 
					{
						// This stage's texture is not flagged for TC animation...
						FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0, &_D3DIdentMtx(0,0), 2);
					}
				}
			}

			//Now load all my lightmaps, after my masks. Note that I'm limited to 4 - [EMASK?1:0] - [ZMASK?1:0]
			if ( !fsh_bUseExtColorStream )
			{
				for (i=0; i<m_nLM; i++)
				{
					pShTexInst = (FShTexInst_t *)FSh_pnLightMapInputRegisters[i*3+1];
					if (pShTexInst )
					{
						if( (u32)pShTexInst < 0xffffffff ) 
						{
							//Make sure that my lightmap texture does not wrap. This is CRITICAL for filtering.
							pShTexInst->TexInst.ClearFlag(CFTexInst::FLAG_WRAP_S);
							pShTexInst->TexInst.ClearFlag(CFTexInst::FLAG_WRAP_T);

							ftex_SetTexAddress(nTex, FALSE, FALSE);

							fdx8tex_SetTexture( nTex, &pShTexInst->TexInst, nTex );
							nTex++;
						} 
						else 
						{
							DEVPRINTF( "Error:  Lightmap texture = 0xffffffff\n" );
						}
					}
				}
			}
		}
		else if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK] && FSh_shaderType == SHADERTYPE_TRANSLUCENCY)// && FSh_shaderType != SHADERTYPE_SPECULAR )
		{
			//Make sure ZMask is set for translucencies
			pShTexInst = (FShTexInst_t *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK];
			if (pShTexInst)
			{
				fdx8tex_SetTexture( 0, &pShTexInst->TexInst, 0 );
			}
		}
	}
	_nPrevTexOverrideCount = _nTexOverrideCount;
	_nPrevTexCoordMtxCount = _nTexCoordMtxCount;
}

extern f32 FVis_fLastVPFOVDistMod;

void fsh_RenderFogPass()
{
	if (FSh_bSurfaceOnly)
	{
		return;
	}

	if (!_bFogEnabled) { return; }

	CFVec3 XAxis, YAxis, ZAxis, Pos, vec;
	RenderPlaneVtx_t quad[4];
	CFColorRGBA color;
	float fScale;

	fdx8xfm_SetViewDXMatrix( TRUE );
	fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &CFMtx43A::m_IdentityMtx, TRUE );
	
	FDX8_SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX4);
	_nCurrentVtxShader = D3DFVF_XYZ | D3DFVF_TEX4;
	FDX8_pDev->SetPixelShader(0);
	_nCurrentPixelShader = 0;

	fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
	fdx8_SetRenderState_SRCBLEND( D3DBLEND_DESTALPHA );
	fdx8_SetRenderState_DESTBLEND( D3DBLEND_INVDESTALPHA );
	fdx8_SetRenderState_BLENDOP( D3DBLENDOP_ADD );

	fdx8_SetRenderState_ALPHATESTENABLE(FALSE);
	fdx8_SetRenderState_ZFUNC( D3DCMP_ALWAYS );
	fsh_ZWriteEnable(FALSE);

	fdx8_SetRenderState_TEXTUREFACTOR(D3DCOLOR_COLORVALUE(fsh_fFogColor2.fRed, fsh_fFogColor2.fGreen, fsh_fFogColor2.fBlue, 1.0f));

	if (fsh_pFogTex)
	{
		fdx8tex_SetTexture(0, fsh_pFogTex, 0);
		ftex_SetTexAddress(0, TRUE, TRUE);

		fdx8_SetTextureState_COLOROP(   0, D3DTOP_MODULATE );
		fdx8_SetTextureState_COLORARG1( 0, D3DTA_TEXTURE );
		fdx8_SetTextureState_COLORARG2( 0, D3DTA_TFACTOR );
	}
	else
	{
		fdx8_SetTextureState_COLOROP(   0, D3DTOP_SELECTARG1 );
		fdx8_SetTextureState_COLORARG1( 0, D3DTA_TFACTOR );
	}

	fdx8_SetTextureState_ALPHAOP(   0, D3DTOP_DISABLE );
	fdx8_SetTextureState_COLOROP(   1, D3DTOP_DISABLE );

	XAxis.x = FXfm_pView->m_MtxR.aa[0][0]; XAxis.y = FXfm_pView->m_MtxR.aa[1][0]; XAxis.z = FXfm_pView->m_MtxR.aa[2][0];
	YAxis.x = FXfm_pView->m_MtxR.aa[0][1]; YAxis.y = FXfm_pView->m_MtxR.aa[1][1]; YAxis.z = FXfm_pView->m_MtxR.aa[2][1];
	ZAxis.x = FXfm_pView->m_MtxR.aa[0][2]; ZAxis.y = FXfm_pView->m_MtxR.aa[1][2]; ZAxis.z = FXfm_pView->m_MtxR.aa[2][2];

	fScale = FVis_fLastVPFOVDistMod; //1.0f
	
	vec.x = -1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[0].vPos.x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x;
	quad[0].vPos.y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y;
	quad[0].vPos.z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z;

	quad[0].vTex[0].x = 0.0f;
	quad[0].vTex[0].y = 0.0f;

	vec.x = +1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[1].vPos.x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x; 
	quad[1].vPos.y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y; 
	quad[1].vPos.z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z; 

	quad[1].vTex[0].x = 4.0f;
	quad[1].vTex[0].y = 0.0f;

	vec.x = +1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[3].vPos.x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x; 
	quad[3].vPos.y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y; 
	quad[3].vPos.z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z; 

	quad[3].vTex[0].x = 4.0f;
	quad[3].vTex[0].y = 4.0f;

	vec.x = -1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[2].vPos.x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x; 
	quad[2].vPos.y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y; 
	quad[2].vPos.z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z;

	quad[2].vTex[0].x = 0.0f;
	quad[2].vTex[0].y = 4.0f;

	FDX8_pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(RenderPlaneVtx_t));

	fdx8tex_SetTexture(0, NULL, 0);
}

void _CheckTexCoordMtx()
{
	u32 i=0, j;
	D3DXMATRIX mTmp;
	u32 nmtx = 0, nTexLayerID;
	FShTexInst_t *pShTexInst=NULL;
	u32 n = _aShaderRenderStates[_nShaderID].nStages;
	u32 nMask=0;
	CFTexInst TexInst;

	//Here I have to load my textures and setup my texture matrices.
    for (i=0; i<4; i++)
	{
		if (_aShaderStageStates[_nShaderID][i].nTexReg > -1 && i < n)
		{
			//Texture.
			if (i < 3)
			{
				pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[i*2];
			}
			else
			{
				pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[5];
			}

			if (_pReflectTex && i==1 && _nShaderID == FSHADERS_oBASE_ADD_rbSREFLECT)
			{
				fdx8tex_SetTexture( i, _pReflectTex, i );
				nMask++;
			}
			else if( _nTexOverrideCount == 0 ) 
			{
				fdx8tex_SetTexture( i, &pShTexInst->TexInst, i );
				nMask++;
			} 
			else 
			{
				nTexLayerID = pShTexInst->nTexLayerID;

				for( j=0; j<_nTexOverrideCount; j++ ) 
				{
					if( nTexLayerID == _aTexOverride[j].nTexLayerID ) 
					{
						// This stage's texture is flagged for texture override...
						TexInst = pShTexInst->TexInst;
						if (_aTexOverride[j].pTexInst)
						{
							TexInst.SetTexDef( _aTexOverride[j].pTexInst->GetTexDef() );

							fdx8tex_SetTexture( i, &TexInst, i );
							nMask++;
						}
						break;
					}
				}
				if( j == _nTexOverrideCount ) 
				{
					// This stage's texture is not flagged for texture override...
					fdx8tex_SetTexture( i, &pShTexInst->TexInst, i );
					nMask++;
				}
			}

			//Texture Matrix.
			if( _nTexCoordMtxCount == 0 ) 
			{
				// No TC matrices...
				FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
			} 
			else 
			{
				// There is at least one TC matrix...

				nTexLayerID = pShTexInst->nTexLayerID;

				for( j=0; j<_nTexCoordMtxCount; j++ ) 
				{
					if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
					{
						// This stage's texture is flagged for TC animation...
						mTmp.m[0][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][0];
						mTmp.m[0][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][0];
						mTmp.m[0][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][0];
						mTmp.m[1][0] = _aTexCoordMtx[j].pTexCoordMtx->aa[0][1];
						mTmp.m[1][1] = _aTexCoordMtx[j].pTexCoordMtx->aa[1][1];
						mTmp.m[1][2] = _aTexCoordMtx[j].pTexCoordMtx->aa[2][1];
						FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &mTmp(0,0), 2);
						break;
					}
				}
				if( j == _nTexCoordMtxCount ) 
				{
					// This stage's texture is not flagged for TC animation...
					FDX8_pDev->SetVertexShaderConstant(CV_TEXMTX0_0+i*2, &_D3DIdentMtx(0,0), 2);
				}
			}
		}
	}

	_nPrevTexCoordMtxCount = _nTexCoordMtxCount;
}

void _CheckTexOverride()
{
	u32 i, j;

	if ( _nTexOverrideCount > 0 || (_nTexOverrideCount == 0 && _nPrevTexOverrideCount > 0) )
	{
		for (i=0; i<4; i++)
		{
			if (_aTexOverrideCache[i].pOrig)
			{
				for( j=0; j<_nTexOverrideCount; j++ ) 
				{
					if( _aTexOverrideCache[i].nTexLayerID == _aTexOverride[j].nTexLayerID ) 
					{
						// This stage's texture is flagged for texture override...
						if (_aTexOverride[j].pTexInst)
						{
							_aTexOverrideCache[i].pCur = _aTexOverride[j].pTexInst;
			
							fdx8tex_SetTexture( i, _aTexOverride[j].pTexInst, i );
						}
						break;
					}
				}

				if (j == _nTexOverrideCount || _nTexOverrideCount == 0)
				{
					if (_aTexOverrideCache[i].pCur != _aTexOverrideCache[i].pOrig)
					{
						fdx8tex_SetTexture( i, _aTexOverrideCache[i].pOrig, i );
					}
				}
			}
		}
	}
	_nPrevTexOverrideCount = _nTexOverrideCount;
}

void fsh_FastSurfaceExecute()
{
	//test
	//fsh_ExecuteCurrent(FALSE, 0);
	//return;
	//
	#if FANG_SURFACE_ONLY
		FSh_bSurfaceOnly = TRUE;
	#endif
	if ( _nShaderID == FSHADERS_ADD_BASE || _nShaderID == FSHADERS_ADD_BASE_DETAIL || _nShaderID == FSHADERS_ADD_vBASE )
	{
		//Surface tint is affect by the layer alpha * surface alpha for additive shaders.
		_fFinalSurfaceColor.fRed   = _fSurfaceColor.fRed   * _fSurfaceColor.fAlpha * _fLayerAlpha;
		_fFinalSurfaceColor.fGreen = _fSurfaceColor.fGreen * _fSurfaceColor.fAlpha * _fLayerAlpha;
		_fFinalSurfaceColor.fBlue  = _fSurfaceColor.fBlue  * _fSurfaceColor.fAlpha * _fLayerAlpha;
		_fFinalSurfaceColor.fAlpha = _fSurfaceColor.fAlpha;
	}
	else
	{
		_fFinalSurfaceColor.fRed   = _fSurfaceColor.fRed;
		_fFinalSurfaceColor.fGreen = _fSurfaceColor.fGreen;
		_fFinalSurfaceColor.fBlue  = _fSurfaceColor.fBlue;
		_fFinalSurfaceColor.fAlpha = _fSurfaceColor.fAlpha;
	}

	if (_fLayerAlpha != _afPixelConst3[0])
	{
		_afPixelConst3[0] = _afPixelConst3[1] = _afPixelConst3[2] = _afPixelConst3[3] = _fLayerAlpha;
		FDX8_pDev->SetPixelShaderConstant(3, _afPixelConst3, 1);
	}

	if (_afPixelConst2[0] != _fFinalSurfaceColor.fRed || _afPixelConst2[1] != _fFinalSurfaceColor.fGreen || _afPixelConst2[2] != _fFinalSurfaceColor.fBlue || _afPixelConst2[3] != _fFinalSurfaceColor.fAlpha)
	{
		_afPixelConst2[0] = _fFinalSurfaceColor.fRed;
		_afPixelConst2[1] = _fFinalSurfaceColor.fGreen;
		_afPixelConst2[2] = _fFinalSurfaceColor.fBlue;
		_afPixelConst2[3] = _fFinalSurfaceColor.fAlpha;
		FDX8_pDev->SetPixelShaderConstant(2, _afPixelConst2, 1);
	}

	_CheckTexCoordMtx();
	//_CheckTexOverride();

	if (_pReflectTex && _nShaderID == FSHADERS_oBASE_ADD_rbSREFLECT)
	{
		fdx8tex_SetTexture( 1, _pReflectTex, 1 );
	}
}

void fsh_ExecuteCurrent(BOOL bShadowRender, s32 nShadowID)
{
	#if FANG_SURFACE_ONLY
		FSh_bSurfaceOnly = TRUE;
	#endif
	if ( _nShaderID == FSHADERS_ADD_BASE || _nShaderID == FSHADERS_ADD_BASE_DETAIL || _nShaderID == FSHADERS_ADD_vBASE )
	{
		//Surface tint is affect by the layer alpha * surface alpha for additive shaders.
		_fFinalSurfaceColor.fRed   = _fSurfaceColor.fRed   * _fSurfaceColor.fAlpha * _fLayerAlpha;
		_fFinalSurfaceColor.fGreen = _fSurfaceColor.fGreen * _fSurfaceColor.fAlpha * _fLayerAlpha;
		_fFinalSurfaceColor.fBlue  = _fSurfaceColor.fBlue  * _fSurfaceColor.fAlpha * _fLayerAlpha;
		_fFinalSurfaceColor.fAlpha = _fSurfaceColor.fAlpha;
	}
	else
	{
		_fFinalSurfaceColor.fRed   = _fSurfaceColor.fRed;
		_fFinalSurfaceColor.fGreen = _fSurfaceColor.fGreen;
		_fFinalSurfaceColor.fBlue  = _fSurfaceColor.fBlue;
		_fFinalSurfaceColor.fAlpha = _fSurfaceColor.fAlpha;
	}

#if FSH_SIMPLE_SHADER
	_SetupSimpleShader();
	return;
#else
	FSh_bShadowRender = bShadowRender;
	FSh_ShadowID = nShadowID;

	if (_nShaderID != _nCurrentShader || FSh_shaderType != SHADERTYPE_SURFACE)
	{
		//Blend states, z buffering, etc. should only be set once for the surface shader.
		_SetRenderStates(); //set render states and blending depending on shader type.
	}
	//This handles specific vertex/pixel shaders, textures and texture matrices and must be set on an per-mesh level.
	_SetPassDXState();

	if (FSh_bShadowRender)
	{
		FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
	}
#endif
}

void fsh_SetPassIdx(u32 nPass)
{
	_nPassIdx = nPass;
	if (nPass > 0)
	{
		m_nLMMode = LM_NONE;
	}
}

void fsh_ZWriteEnable(BOOL bEnable)
{
	FSh_ZEnable = bEnable;
}

void fsh_CheckVB()
{
	if (_bVBChanged && _nPassIdx == 0 && FSh_shaderType != SHADERTYPE_SPECULAR && (_nShaderID < FSHADERS_LIQUID_ENV || _nShaderID > FSHADERS_LIQUID_MOLTEN_2LAYER))
	{
		_bVBChanged = FALSE;

		if (!FSh_bShadowRender)
		{
			u32 nVShader = _aShaderRenderStates[_nShaderID].vShader;

			if (!fdx8sh_bStream1Set) { m_nLMMode = LM_NONE; }

			if (m_nLMMode > LM_NONE)
			{
				switch (m_nLMMode)
				{
					case LM4:
						nVShader = VSHADER_BASE_LM4;
						break;
					case ZMASK_EMASK_LM2:
						nVShader = VSHADER_BASE_ZMASK_EMASK_LM2;
						break;
					case ZMASK_LM3:
						nVShader = VSHADER_BASE_ZMASK_LM3;
						break;
					case EMASK_LM3:
						nVShader = VSHADER_BASE_EMASK_LM3;
						break;
				};
			}
			_SetVertexShader(nVShader);
		}
		else
		{
			_SetVertexShader(VSHADER_BASE_PASSTHRU);
			FDX8_pDev->SetVertexShaderConstant(CV_FACTOR, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 1);
		}
	}
}

void fsh_EndExecute()
{
	if (_bFogEnabled)
	{
		fdx8_SetRenderState_COLORWRITEENABLE(D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE);
	}
}
//end Jeremy

//for testing
#include "fpad.h"
//

FSTATIC void _Reset( void ) 
{
	// Default global registers...
	_Default_GReg_Ambient.OpaqueBlack();
	fsh_SetGlobalRegister( FSHREG_AMBIENT, (u32)&_Default_GReg_Ambient );

	// Reset lights...
	_nLightCount = 0;

	_nPointCount = 0;
	_nDirCount = 0;
	_nSpotCount = 0;

	// Reset texture coordinate transformations...
	_nTexCoordMtxMaxCount = _MAX_TEX_COORD_MTX_COUNT;
	_nTexCoordMtxCount = 0;
	_nTexOverrideMaxCount = _MAX_TEX_OVERRIDE_COUNT;
	_nTexOverrideCount = 0;
	_nPrevTexOverrideCount = 0;

	// Reset depth bias...
	_nDepthBias = 0;

	// Reset current shader.
	_nCurrentShader = 0xffffffff;
	_nCurrentVtxShader = 0xffffffff;
	_nCurrentPixelShader = 0xffffffff;

	//Why isn't this set somewhere else?
	if (_bModuleInitialized && _bWindowCreated)
	{
		//fdx8_SetRenderState_LIGHTING( TRUE );
		ftex_SetTexAddress(0, TRUE, TRUE);
		ftex_SetTexAddress(1, TRUE, TRUE);
		ftex_SetTexAddress(2, TRUE, TRUE);
		ftex_SetTexAddress(3, TRUE, TRUE);
		//
	}

#if _USE_GAMEPAD_
	//TEMP for visual purposes only
	extern u32 Gamepad_nDebugPortIndex;
	extern FPad_Sample_t *Gamepad_aapSample[FPADIO_MAX_DEVICES][32];

	s32 nFShDebugControllerId = Gamepad_nDebugPortIndex;

	if ( Gamepad_aapSample[nFShDebugControllerId][0] )
	{
		if ( Gamepad_aapSample[nFShDebugControllerId][0]->fCurrentState )
		{
 			FLight_fExposure += 0.01f;
		}
		else if ( Gamepad_aapSample[nFShDebugControllerId][10]->fCurrentState )
		{
			FLight_fExposure -= 0.01f;
			if (FLight_fExposure < 0.1f) FLight_fExposure = 0.1f;
		}

		if ( Gamepad_aapSample[nFShDebugControllerId][5]->fCurrentState )
		{
 			FLight_fLightRadiusAdj += 0.01f;
		}
		else if ( Gamepad_aapSample[nFShDebugControllerId][6]->fCurrentState )
		{
			FLight_fLightRadiusAdj -= 0.01f;
			if (FLight_fLightRadiusAdj < 0.1f) FLight_fLightRadiusAdj = 0.1f;
		}
	}
	//
#endif
}

FSTATIC BOOL _BuildD3DLight( D3DLIGHT8 *pD3DLight, CFLight *pLight, BOOL bPerPixelLight ) 
{
	switch( pLight->m_nType ) 
	{
		case FLIGHT_TYPE_DIR:
			pD3DLight->Type = D3DLIGHT_DIRECTIONAL;
			fsh_SetSpecularLight( pLight );
			break;

		case FLIGHT_TYPE_OMNI:
			pD3DLight->Type = D3DLIGHT_POINT;
			break;

		case FLIGHT_TYPE_SPOT:
			pD3DLight->Type = D3DLIGHT_SPOT;
			break;

		default:
			FASSERT_NOW;
	}

	pLight->ComputeColor();

	CFColorRGBA LightColor;
	LightColor = pLight->m_ScaledColor;
	LightColor.Clamp1();

	pD3DLight->Diffuse.r = pD3DLight->Specular.r = LightColor.fRed;
	pD3DLight->Diffuse.g = pD3DLight->Specular.g = LightColor.fGreen;
	pD3DLight->Diffuse.b = pD3DLight->Specular.b = LightColor.fBlue;
	pD3DLight->Diffuse.a = pD3DLight->Specular.a = LightColor.fAlpha;
	pD3DLight->Ambient.r = pD3DLight->Ambient.g = pD3DLight->Ambient.b = pD3DLight->Ambient.a = 0.0f;

	if( pLight->m_nFlags & FLIGHT_FLAG_HASDIR ) 
	{
		if ( pLight->m_nXfmKey_VS != FXfm_nViewKey )
		{
			pLight->m_vUnitDir_VS = FXfm_pView->m_MtxF.m44.MultDir( pLight->m_mtxOrientation_WS.m_vFront );
		}
	}

	if( pLight->m_nFlags & FLIGHT_FLAG_HASPOS ) 
	{
		if ( pLight->m_nXfmKey_VS != FXfm_nViewKey )
		{
			pLight->m_vPos_VS = FXfm_pView->m_MtxF.m44.MultPoint( pLight->m_spInfluence_WS.m_Pos );
		}
		pD3DLight->Position.x = pLight->m_spInfluence_WS.m_Pos.x;
		pD3DLight->Position.y = pLight->m_spInfluence_WS.m_Pos.y;
		pD3DLight->Position.z = pLight->m_spInfluence_WS.m_Pos.z;

		pD3DLight->Range = pLight->m_spInfluence_WS.m_fRadius;

		if (bPerPixelLight)
		{		
			if (pLight->m_nType == FLIGHT_TYPE_OMNI)
			{
				if (_nPerPixelPoint < 8)
				{
					D3DXVECTOR3 LPos= D3DXVECTOR3(pLight->m_vPos_VS.x, pLight->m_vPos_VS.y, pLight->m_vPos_VS.z);

					f32 fR, fG, fB;
					fR = fmath_Sqrt( pD3DLight->Diffuse.r ); fR = fmath_Sqrt(fR);
					fG = fmath_Sqrt( pD3DLight->Diffuse.g ); fG = fmath_Sqrt(fG);
					fB = fmath_Sqrt( pD3DLight->Diffuse.b ); fB = fmath_Sqrt(fB);
				
					_aPerPixelPointLights[ _nPerPixelPoint ].vPos = LPos;
					_aPerPixelPointLights[ _nPerPixelPoint ].vColor = D3DXVECTOR3(fR * FSh_fPerPixelFade, fG * FSh_fPerPixelFade, fB * FSh_fPerPixelFade);
					_aPerPixelPointLights[ _nPerPixelPoint ].fOOR = 0.90f*pLight->m_fOOR_WS;

					_nPerPixelPoint++;
				}
				
				return FALSE; //per-pixel light, can't use Lighting hardware

			} 
			else if (pLight->m_nType == FLIGHT_TYPE_SPOT && _nPerPixelSpot < 4)
			{
				D3DXVECTOR3 LPos = D3DXVECTOR3(pLight->m_spInfluence_WS.m_Pos.x, pLight->m_spInfluence_WS.m_Pos.y, pLight->m_spInfluence_WS.m_Pos.z);

				_aPerPixelSpotLights[ _nPerPixelSpot ].vPos.Set(LPos);
				_aPerPixelSpotLights[ _nPerPixelSpot ].pMtx_WS = &pLight->m_mtxOrientation_WS;
				_aPerPixelSpotLights[ _nPerPixelSpot ].vColor = D3DXVECTOR3(pD3DLight->Diffuse.r * FSh_fPerPixelFade, pD3DLight->Diffuse.g * FSh_fPerPixelFade, pD3DLight->Diffuse.b * FSh_fPerPixelFade);

				_aPerPixelSpotLights[ _nPerPixelSpot ].fRange2 = 1.0f / (pLight->m_spInfluence_WS.m_fRadius*pLight->m_spInfluence_WS.m_fRadius);
				_aPerPixelSpotLights[ _nPerPixelSpot ].fFOV = pLight->m_fSpotOuterRadians;
				if (pLight->m_pProjectionTex)
				{
					if (pLight->m_pProjectionTex->GetTexDef())
					{
						if (pLight->m_pProjectionTex->GetTexDef()->pTexData)
						{
							_aPerPixelSpotLights[ _nPerPixelSpot ].pDXTex = (LPDIRECT3DTEXTURE8)pLight->m_pProjectionTex->GetTexDef()->pTexData->pD3DTexture;
						}
					}
				}
				else
				{
					_aPerPixelSpotLights[ _nPerPixelSpot ].pDXTex = _pAttenMap;
				}

				_nPerPixelSpot++;

				return FALSE;
			}
			else if (pLight->m_nType == FLIGHT_TYPE_SPOT)
			{
				pD3DLight->Attenuation0 = FLIGHT_ATTEN_K0;
			}
		}

		pD3DLight->Attenuation0 = FLIGHT_ATTEN_K0;
		#if FLIGHT_ATTEN_NO_K1
			pD3DLight->Attenuation1 = 0.0f;
		#else
			pD3DLight->Attenuation1 = (FLIGHT_ATTEN_K1 * pLight->m_fOOR_WS);
		#endif
		pD3DLight->Attenuation2 = (FLIGHT_ATTEN_K2 * pLight->m_fOOR2_WS);

		if ( _nPointCount < 6 )
		{
			D3DXVECTOR3 LPos= D3DXVECTOR3(pLight->m_vPos_VS.x, pLight->m_vPos_VS.y, pLight->m_vPos_VS.z);
			
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT2_DIFFUSE+_nPointCount, D3DXVECTOR4(pD3DLight->Diffuse.r, pD3DLight->Diffuse.g, pD3DLight->Diffuse.b, 1.0f), 1);
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT2_POSITION+_nPointCount, D3DXVECTOR4(LPos), 1);
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT2_ATTENUATION+_nPointCount, D3DXVECTOR4(pD3DLight->Attenuation0, pD3DLight->Attenuation1, pD3DLight->Attenuation2, 1.0f), 1);

			if (_nPointCount < 4)
			{
				u32 nOffs = _nPointCount<<1;
				if (pLight->m_nType == FLIGHT_TYPE_SPOT)
				{
					//Spot Direction.
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX2_0+nOffs, D3DXVECTOR4(pLight->m_vUnitDir_VS.x, pLight->m_vUnitDir_VS.y, pLight->m_vUnitDir_VS.z, 1) , 1);
					//Spot Angular Attenuation.
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX2_1+nOffs, D3DXVECTOR4(0, -pLight->m_fSpotK1*pLight->m_fSpotK2, pLight->m_fSpotK2, 0), 1);
				}
				else
				{
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX2_0+nOffs, D3DXVECTOR4(0, 0, 0, 0), 1);
					FDX8_pDev->SetVertexShaderConstant(CV_LIGHTMTX2_1+nOffs, D3DXVECTOR4(0.0f, 1.0f, 0.0f, 0.0f), 1);
				}
			}
			_nPointCount++;
		}
	} 
	else 
	{
		pD3DLight->Position.x = 0.0f;
		pD3DLight->Position.y = 0.0f;
		pD3DLight->Position.z = 0.0f;
		pD3DLight->Range = 0.0f;
		pD3DLight->Attenuation0 = 0.0f;
		pD3DLight->Attenuation1 = 0.0f;
		pD3DLight->Attenuation2 = 0.0f;
	}

	if( pLight->m_nFlags & FLIGHT_FLAG_HASDIR ) 
	{
		pD3DLight->Direction.x = pLight->m_mtxOrientation_WS.m_vFront.x;
		pD3DLight->Direction.y = pLight->m_mtxOrientation_WS.m_vFront.y;
		pD3DLight->Direction.z = pLight->m_mtxOrientation_WS.m_vFront.z;

		if (_nDirCount < 1 && pLight->m_nType != FLIGHT_TYPE_SPOT)
		{
			D3DXVECTOR3 LDir= D3DXVECTOR3(-pLight->m_vUnitDir_VS.x, -pLight->m_vUnitDir_VS.y, -pLight->m_vUnitDir_VS.z);
			
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIFFUSE, D3DXVECTOR4(pD3DLight->Diffuse.r, pD3DLight->Diffuse.g, pD3DLight->Diffuse.b, 1.0f), 1);
			FDX8_pDev->SetVertexShaderConstant(CV_LIGHT1_DIRECTION, D3DXVECTOR4(LDir), 1);

			_nDirCount++;
		}
	} 
	else 
	{
		pD3DLight->Direction.x = 0.0f;
		pD3DLight->Direction.y = 0.0f;
		pD3DLight->Direction.z = 0.0f;
	}

	if( pLight->m_nType == FLIGHT_TYPE_SPOT ) 
	{
		pD3DLight->Falloff = 1.0f;
		pD3DLight->Theta = pLight->m_fSpotInnerRadians;
		pD3DLight->Phi = pLight->m_fSpotOuterRadians;
	} 
	else 
	{
		pD3DLight->Falloff = 0.0f;
		pD3DLight->Theta = 0.0f;
		pD3DLight->Phi = 0.0f;
	}

	pLight->m_nXfmKey_VS = FXfm_nViewKey;

	return TRUE;
}

FSTATIC BOOL _WindowCreatedCallback( FDX8VidEvent_e nEvent ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FDX8VID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FDX8VID_EVENT_WINDOW_CREATED:
			_bWindowCreated = TRUE;
			_bShaderSystemOpen = FALSE;

			_Reset();
			if (!_pAttenMap) 
			{
				_GenerateAttenMap();
			}

			if (!_pSView)
			{
				_pSView = fviewport_Create();
			}

			fsh_CreateFullScreenTarget();
			fsh_ClearRenderPlanes();
			fsh_ClearClipPlane();
				
			fworld_RegisterWorldCallbackFunction( FSh_WorldCallbackFunc );

			_SetupVertexShaders();
			_SetupPixelShaders();

			fdx8shadow_SetupVertexShaders();
			fdx8shadow_SetupPixelShaders();

			fsh_fFogConst[0] = fsh_fFogDensity /(fsh_fFogFarZ - fsh_fFogNearZ);
			fsh_fFogConst[1] = -fsh_fFogNearZ*fsh_fFogConst[0];
			fsh_fFogConst[2] = -1.0f/(fsh_fFogMaxY - fsh_fFogMinY);
			fsh_fFogConst[3] = -fsh_fFogMaxY*fsh_fFogConst[2];

			_bFogEnabled = FALSE;

			fsh_pFogTex = NULL;

			break;

		case FDX8VID_EVENT_WINDOW_DESTROYED:
			_bWindowCreated = FALSE;

			fworld_UnregisterWorldCallbackFunction( FSh_WorldCallbackFunc );

			if (_pAttenMap)
			{
				_pAttenMap->Release();
			}
			_pAttenMap = NULL;
			if (_pTexKill)
			{
				_pTexKill->Release();
			}
			_pTexKill = NULL;
			if (_pSpec16)
			{
				_pSpec16->Release();
			}
			_pSpec16=NULL;
			if (_pSpec32)
			{
				_pSpec32->Release();
			}
			_pSpec32=NULL;
			if (_pSpec48)
			{
				_pSpec48->Release();
			}
			_pSpec48=NULL;
			if (_pNormalizeCube)
			{
				_pNormalizeCube->Release();
			}
			_pNormalizeCube=NULL;

			if (_pFullScrTarget_D3DTex)
			{
				_pFullScrTarget_D3DTex->Release();
				_pFullScrTarget_D3DTex = NULL;
			}

			if (_pFullScrTarget_D3DSurf)
			{
				_pFullScrTarget_D3DSurf->Release();
				_pFullScrTarget_D3DSurf = NULL;
			}

			_DeleteVertexShaders();
			_DeletePixelShaders();

			fdx8shadow_DeleteVertexShaders();
			fdx8shadow_DeletePixelShaders();

			_pSView = NULL;
			
			fworld_UnregisterWorldCallbackFunction( FSh_WorldCallbackFunc );
			break;

		case FDX8VID_EVENT_PRE_RESET:
			break;

		case FDX8VID_EVENT_POST_RESET:
			break;
	}

	return TRUE;
}

void fsh_FullScrCallback(void *pUserData)
{
#if FANG_PLATFORM_XB
	u8 nCubeView;
	s32 i, idx=-1;
	f32 fMinDist=FMATH_MAX_FLOAT, fDist;
	if (_nNumRenderPlanes)
	{
		CFCamera *pCamera = fcamera_GetCameraByIndex( fcamera_GetLastCameraIndex() );
		if (pCamera)
		{
			const CFVec3 *pPos = pCamera->GetPos();
			
			if (pPos)
			{
				for (i=0; i<(s32)_nNumRenderPlanes; i++)
				{
					if (_aRenderPlanes[i].bActive)
					{
						if (_aRenderPlanes[i].nType&RP_REFLECT || _aRenderPlanes[i].nType&RP_REFRACT)
						{
							fDist = CFVec3(_aRenderPlanes[i].vCen - *pPos).Mag2();
							if (fDist-_aRenderPlanes[i].fRadius < fMinDist)
							{
								fMinDist = fDist-_aRenderPlanes[i].fRadius;
								idx = i;
							}
							//break;
						}
					}
				}
			}
		}

		if (idx > -1)
		{
			if ( fdx8tex_GetCubeMapParam(nCubeView) )
			{
				CFVec3 vDir, vUp;
				switch (nCubeView)
				{
					case 0: //N=(+1, 0, 0)
						vDir.Set(+1, 0, 0);
						vUp.Set(0, 1, 0);
						fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 1.0f, 0.0f, 0.0f, 1.0f, 0);
						break;
					case 1: //N=(-1, 0, 0)
						vDir.Set(-1, 0, 0);
						vUp.Set(0, 1, 0);
						fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.5f, 0.0f, 0.0f, 1.0f, 0);
						break;
					case 2: //N=( 0,+1, 0)
						vDir.Set(0, +1, 0);
						vUp.Set(0, 0, -1);
						fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 1.0f, 0.0f, 1.0f, 0);
						break;
					case 3: //N=( 0,-1, 0)
						vDir.Set(0, -1, 0);
						vUp.Set(0, 0, -1);
						fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 1.0f, 1.0f, 0.0f, 1.0f, 0);
						break;
					case 4: //N=( 0, 0,+1)
						vDir.Set(0, 0, +1);
						vUp.Set(0, 1, 0);
						fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 1.0f, 1.0f, 0);
						break;
					case 5: //N=( 0, 0,-1)
						vDir.Set(0, 0, -1);
						vUp.Set(0, 1, 0);
						fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.5f, 1.0f, 0);
						break;
				}

				static CFXfm _VXfm;
				const CFXfm *pCamXfm = pCamera->GetFinalXfm();

				pCamXfm->InitStackWithView();

				f32 fFOV = FMATH_DEG2RAD(45.0f);
				fviewport_InitPersp(_pSView, fFOV, fFOV, 0.1f, 1000.0f);
				fviewport_SetActive(_pSView);

				CFVec3 vPos;
				vPos = _aRenderPlanes[idx].vCen;
				_VXfm.BuildLookatFromDirVec( vPos, vDir, vUp );
				_VXfm.InitStackWithView();

				bRenderPlane = FALSE;
				fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);
				if (FVis_pSkyBox)
				{
					frenderer_Push( FRENDERER_MESH, NULL );
		 			FVis_pSkyBox->Draw( FVIEWPORT_PLANESMASK_ALL, TRUE );
					frenderer_Pop();
				}

				//Set the min render distance to 10 ft. to avoid problems when objects get too close to the center of reflection.
				FVis_fMinRenderDist = 10.0f;
		
				fvis_Draw( NULL, FALSE);
				bRenderPlane = TRUE;

				//Reset Min render distance.
				FVis_fMinRenderDist = 0.0f;
			}
		}
		else
		{
			fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);
		}
	}
#endif
}

CFMtx43A _ReflectMtx;

void fsh_ClearRenderPlanes()
{
	_nNumRenderPlanes = 0;
}

f32 _afQuadU[4] = 
{
	0.0f, 1.0f, 1.0f, 0.0f
};

f32 _afQuadV[4] = 
{
	1.0f, 1.0f, 0.0f, 0.0f
};

u32 _anQuadIdx[4] = 
{
	0, 1, 3, 2
};

u32 fsh_AddRenderPlane(CFVec3 *pQuad, u32 nType, BOOL bFlip, CFColorRGB *fogClr, f32 fFogDensity, f32 fGeoCullDist, f32 fOpacity, f32 fTile, f32 fRadius, CFTexInst *pLayer0, CFTexInst *pLayer1)
{
	u32 i, nPlaneID = _nNumRenderPlanes, n;
	fsh_Render_Plane_t *pPlane = &_aRenderPlanes[nPlaneID]; 
	_nNumRenderPlanes++;

	CFVec3 vS, vT, vN, dS, dT, pS, pT;
	CFVec2 vS2, vT2, dS2, dT2, pS2, pT2;

	pPlane->vCen.Set(0,0,0);
	for (i=0; i<4; i++)
	{
		pPlane->vCen += pQuad[i];
	}
	pPlane->vCen *= 0.25f;

	pPlane->nVtx = 4;
	
	for (i=0; i<4; i++)
	{
		n = _anQuadIdx[i];
		pPlane->Quad[i].vPos = pQuad[n];

		pPlane->Quad[i].vTex[0].x = _afQuadU[n];
		pPlane->Quad[i].vTex[0].y = _afQuadV[n];

		pPlane->Quad[i].vTex[1] = pPlane->Quad[i].vTex[0];
		pPlane->Quad[i].vTex[2] = pPlane->Quad[i].vTex[0];
		pPlane->Quad[i].vTex[3] = pPlane->Quad[i].vTex[0];
	}
	
	vS = pQuad[1] - pQuad[0];
	vS.Unitize();
	vT = pQuad[2] - pQuad[0];
	vT.Unitize();
	
	vN = vS.UnitCross(vT);
	
	if (bFlip)
	{
		vN = -vN;
	}
	
	pPlane->vPlane = vN;
	
	pPlane->nType = nType;
	
	if (fogClr && fFogDensity && (nType&RP_REFRACT))
	{
		pPlane->fogClr = *fogClr;
		pPlane->fFogDensity = fFogDensity;
	}
	else
	{
		if (fogClr)
		{
			pPlane->fogClr = *fogClr;
		}
		else
		{
			pPlane->fogClr = CFColorRGB(1,1,1);
		}
		pPlane->fFogDensity = 0.0f;
	}
	
	pPlane->d3dClr = D3DCOLOR_COLORVALUE(pPlane->fogClr.fRed, pPlane->fogClr.fGreen, pPlane->fogClr.fBlue, fOpacity);
		
	pPlane->fGeoCullDist = fGeoCullDist;
	pPlane->fOpacity = fOpacity;
	pPlane->fTile = fTile;
	pPlane->fRadius = fRadius*fRadius;
	
	pPlane->pLayer0 = pLayer0;
	pPlane->pLayer1 = pLayer1;

	return nPlaneID;
}

void fsh_ChangeRenderPlane(u32 nPlaneID, CFVec3 *pQuad, CFColorRGB *fogClr, f32 fFogDensity)
{
	u32 i;
	f32 fdy;
	fsh_Render_Plane_t *pPlane = &_aRenderPlanes[nPlaneID]; 
	fdy = pQuad[0].y - pPlane->Quad[0].vPos.y;
	for (i=0; i<pPlane->nVtx; i++)
	{
		pPlane->Quad[i].vPos.y += fdy;
	}

	_aRenderPlanes[ _nNumRenderPlanes ].fFogDensity = fFogDensity;
	if (fogClr)
	{
		_aRenderPlanes[ _nNumRenderPlanes ].fogClr = *fogClr;
	}
	else
	{
		_aRenderPlanes[ _nNumRenderPlanes ].fogClr = CFColorRGB(1,1,1);
	}
		
	pPlane->d3dClr = D3DCOLOR_COLORVALUE(pPlane->fogClr.fRed, pPlane->fogClr.fGreen, pPlane->fogClr.fBlue, pPlane->fOpacity);
}

BOOL _bNeedTCRestore=FALSE;

BOOL _SetupMoltenStates(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM, f32 fScrollU, f32 fScrollV)
{
	BOOL bRet = FALSE;

	f32 afData[4];
	afData[0] = afData[1] = afData[2] = afData[3] = 1.0f;
	
	FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00,   F2DW( 0.01f ) );
    FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01,   F2DW( 0.0f ) );
    FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10,   F2DW( 0.0f ) );
    FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11,   F2DW( 0.01f ) );

	if (pPlane->pLayer0	&& pPlane->pLayer1)
	{
		_SetPixelShader(MOLTEN_2LAYER_EMBM);

		fdx8tex_SetTexture(0, pEMBM, 0);
		fdx8tex_SetTexture(1, pPlane->pLayer0, 1);
		fdx8tex_SetTexture(2, pPlane->pLayer1, 2);

		u32 i;
		for (i=0; i<4; i++)
		{
			pPlane->Quad[i].vTex[2].x = (pPlane->Quad[i].vTex[0].x+fScrollU)*(pPlane->fTile);
			pPlane->Quad[i].vTex[2].y = (pPlane->Quad[i].vTex[0].y+fScrollV)*(pPlane->fTile);
		}

		bRet = TRUE;
	}
	else
	{
		if (pEMBM)
		{
			if (pPlane->pLayer0)
			{
				_SetPixelShader(MOLTEN_1LAYER_EMBM);
				fdx8tex_SetTexture(0, pEMBM, 0);
				fdx8tex_SetTexture(1, pPlane->pLayer0, 1);

				bRet = TRUE;
			}
		}
	}

	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);

	return (bRet);
}

BOOL _SetupMoltenLayers(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM, f32 fScrollX, f32 fScrollY)
{
	BOOL bRet = FALSE;
	FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00,   F2DW( 0.025f ) );
    FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01,   F2DW( 0.0f ) );
    FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10,   F2DW( 0.0f ) );
    FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11,   F2DW( 0.025f ) );

	ftex_SetTexAddress(0, TRUE, TRUE);
	ftex_SetTexAddress(1, TRUE, TRUE);
	ftex_SetTexAddress(2, TRUE, TRUE);

	FDX8_SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX4);
	_nCurrentVtxShader = D3DFVF_XYZ | D3DFVF_TEX4;

	if (pPlane->pLayer0	&& pPlane->pLayer1)
	{
		_SetPixelShader(MOLTEN_2LAYER_EMBM_GLOW);

		fdx8tex_SetTexture(0, pEMBM, 0);
		fdx8tex_SetTexture(1, pPlane->pLayer0, 1);
		fdx8tex_SetTexture(2, pPlane->pLayer1, 2);

		bRet = TRUE;
	}
	else
	{
		if (pEMBM)
		{
			if (pPlane->pLayer0)
			{
				_SetPixelShader(MOLTEN_1LAYER_EMBM);
				fdx8tex_SetTexture(0, pEMBM, 0);
				fdx8tex_SetTexture(1, pPlane->pLayer0, 1);

				bRet = TRUE;
			}
		}
	}

	f32 afData[4];
	afData[0] = afData[1] = afData[2] = afData[3] = 0.125f;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);

	return (bRet);
}

void _SetupRefTextureStates(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM)
{
	fdx8tex_SetTexture(0, pEMBM, 0);

	ftex_SetTexAddress(0, TRUE, TRUE);

	fdx8tex_SetTexture(1, pPlane->pLayer0, 1);
	fdx8tex_SetTexture(2, pPlane->pLayer0, 2);
	fdx8tex_SetTexture(3, pPlane->pLayer0, 3);

	ftex_SetTexAddress(3, TRUE, TRUE, TRUE);
	
	CFVec4 vCen;
	vCen.Set( 0, 0, 0, 1 );//pPlane->vCen.x, pPlane->vCen.y, pPlane->vCen.z, 1.0f);
	FDX8_pDev->SetVertexShaderConstant(CV_FIXED_COLOR, &vCen, 1);
	
	FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PLANAR_REFLECT]);
	_nCurrentVtxShader = _anVShader_Handle[VSHADER_PLANAR_REFLECT];
	_SetPixelShader(PSHADER_CUBE_REFLECT);

	f32 afData[4];
	afData[0] = pPlane->fogClr.fRed;
	afData[1] = pPlane->fogClr.fGreen;
	afData[2] = pPlane->fogClr.fBlue;
	afData[3] = pPlane->fOpacity;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
}

void _SetupMercuryStatesPass0(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM, f32 fScrollU, f32 fScrollV)
{
	fdx8tex_SetTexture(0, pEMBM, 0);

	ftex_SetTexAddress(0, TRUE, TRUE);

	fdx8tex_SetTexture(1, _pFullScrTarget, 1);
	fdx8tex_SetTexture(2, _pFullScrTarget, 2);
	fdx8tex_SetTexture(3, _pFullScrTarget, 3);

	ftex_SetTexAddress(3, FALSE, FALSE, FALSE);
	
	CFVec4 vCen;
	/*
	_fCubeReflectHackDelta = FMATH_FABS(pPlane->QuadSec[0].vPos.x - pPlane->QuadSec[1].vPos.x);
	_fCubeReflectHackDelta = FMATH_FABS(pPlane->QuadSec[2].vPos.y - pPlane->QuadSec[3].vPos.y)<_fCubeReflectHackDelta?FMATH_FABS(pPlane->QuadSec[2].vPos.y - pPlane->QuadSec[3].vPos.y):_fCubeReflectHackDelta;

	if (_fCubeReflectHackDelta)
	{
		_fCubeReflectHackDelta = 1.0f/_fCubeReflectHackDelta;
	}
	*/

	vCen.Set( 0, 0, 0, 1 );//pPlane->vCen.x, pPlane->vCen.y, pPlane->vCen.z, _fCubeReflectHackDelta);
	FDX8_pDev->SetVertexShaderConstant(CV_FIXED_COLOR, &vCen, 1);
	
	FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PLANAR_REFLECT]);
	_nCurrentVtxShader = _anVShader_Handle[VSHADER_PLANAR_REFLECT];
	_SetPixelShader(PSHADER_CUBE_REFLECT);

	f32 afData[4];
	afData[0] = pPlane->fogClr.fRed;
	afData[1] = pPlane->fogClr.fGreen;
	afData[2] = pPlane->fogClr.fBlue;
	afData[3] = pPlane->fOpacity;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
}

void _SetupMercuryStatesPass1(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM, f32 fScrollU, f32 fScrollV)
{
	fdx8tex_SetTexture(0, pEMBM, 0);

	ftex_SetTexAddress(0, TRUE, TRUE);

	fdx8tex_SetTexture(1, pPlane->pLayer0, 1);

	FDX8_SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX4);
	_nCurrentVtxShader = D3DFVF_XYZ | D3DFVF_TEX4;
	_SetPixelShader(MOLTEN_1LAYER_EMBM);

	f32 afData[4];
	afData[0] = afData[1] = afData[2] = afData[3] = 1.0f;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);

	u32 i;
	for (i=0; i<4; i++)
	{
		pPlane->Quad[i].vTex[1].x = (pPlane->Quad[i].vTex[0].x+fScrollU)*pPlane->fTile;
		pPlane->Quad[i].vTex[1].y = (pPlane->Quad[i].vTex[0].y+fScrollV)*pPlane->fTile;
	}
}

void _SetupWaterStatesPass0(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM, f32 fScrollU, f32 fScrollV)
{
	fdx8tex_SetTexture(0, pEMBM, 0);

	ftex_SetTexAddress(0, TRUE, TRUE);

	fdx8tex_SetTexture(1, _pFullScrTarget, 1);
	fdx8tex_SetTexture(2, _pFullScrTarget, 2);
	fdx8tex_SetTexture(3, _pFullScrTarget, 3);

	ftex_SetTexAddress(3, FALSE, FALSE, FALSE);
		
	FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PLANAR_REFLECT]);
	_nCurrentVtxShader = _anVShader_Handle[VSHADER_PLANAR_REFLECT];
	_SetPixelShader(PSHADER_CUBE_REFLECT);

	CFVec4 vCen;

	/*_fCubeReflectHackDelta = FMATH_FABS(pPlane->Quad[0].vPos.x - pPlane->Quad[1].vPos.x);
	_fCubeReflectHackDelta = FMATH_FABS(pPlane->Quad[2].vPos.y - pPlane->Quad[3].vPos.y)>_fCubeReflectHackDelta?FMATH_FABS(pPlane->Quad[2].vPos.y - pPlane->Quad[3].vPos.y):_fCubeReflectHackDelta;

	if (_fCubeReflectHackDelta)
	{
		_fCubeReflectHackDelta = 2.0f/_fCubeReflectHackDelta;
	}*/

	vCen.Set( 0, 0, 0, 0 );//pPlane->vCen.x, pPlane->vCen.y, pPlane->vCen.z, _fCubeReflectHackDelta);
	FDX8_pDev->SetVertexShaderConstant(CV_FIXED_COLOR, &vCen, 1);

	f32 afData[4];
	afData[0] = pPlane->fogClr.fRed;
	afData[1] = pPlane->fogClr.fGreen;
	afData[2] = pPlane->fogClr.fBlue;
	afData[3] = pPlane->fOpacity;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
}

void _SetupWaterStatesPass1(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM, f32 fScrollU, f32 fScrollV)
{
	fdx8tex_SetTexture(0, pEMBM, 0);

	ftex_SetTexAddress(0, TRUE, TRUE);

	fdx8tex_SetTexture(1, _pFullScrTarget, 1);
	fdx8tex_SetTexture(2, _pFullScrTarget, 2);
	fdx8tex_SetTexture(3, _pFullScrTarget, 3);

	ftex_SetTexAddress(3, FALSE, FALSE, FALSE);
		
	FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PLANAR_REFRACT]);
	_nCurrentVtxShader = _anVShader_Handle[VSHADER_PLANAR_REFRACT];
	_SetPixelShader(PSHADER_CUBE_REFLECT);

	f32 afData[4];
	afData[0] = pPlane->fogClr.fRed*0.5f;
	afData[1] = pPlane->fogClr.fGreen*0.5f;
	afData[2] = pPlane->fogClr.fBlue*0.5f;
	afData[3] = pPlane->fOpacity;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
}

void _SetupWaterStatesPass2(fsh_Render_Plane_t *pPlane, CFTexInst *pEMBM, f32 fScrollU, f32 fScrollV)
{
	fdx8tex_SetTexture(0, pEMBM, 0);
	fdx8tex_SetTexture(1, pPlane->pLayer0, 1);

	ftex_SetTexAddress(0, TRUE, TRUE);

	FDX8_SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX4);
	_nCurrentVtxShader = D3DFVF_XYZ | D3DFVF_TEX4;
	_SetPixelShader(MOLTEN_1LAYER_EMBM);

	f32 afData[4];
	afData[0] = afData[1] = afData[2] = afData[3] = 1.0f;
	FDX8_pDev->SetPixelShaderConstant(0, afData, 1);

	_bNeedTCRestore = TRUE;
	u32 i;
	for (i=0; i<4; i++)
	{
		pPlane->Quad[i].vTex[1].x = (pPlane->Quad[i].vTex[0].x+fScrollU)*pPlane->fTile;
		pPlane->Quad[i].vTex[1].y = (pPlane->Quad[i].vTex[0].y+fScrollV)*pPlane->fTile;
	}
}

void _MoveUp(f32 fYOffs)
{
	CFMtx43A OffsMtx;
	OffsMtx.Identity();
	OffsMtx.m_vPos.Set(0, fYOffs, 0);

	fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &OffsMtx, FALSE );
}

void fsh_RenderPlane(u32 nPlaneID, f32 fScaleU, f32 fScaleV, f32 fScrollU, f32 fScrollV, CFTexInst *pEMBM, f32 fGlowScale, f32 fBumpTile)
{
#if FANG_PLATFORM_XB
	if (!bRenderPlane) return;
	u32 nQuad=1, n, nPasses;
	f32 fDeltaY=0.0f;
	fsh_Render_Plane_t *pPlane = &_aRenderPlanes[nPlaneID];

	fdx8xfm_SetViewDXMatrix( TRUE );
	fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &CFMtx43A::m_IdentityMtx, TRUE );
	
	FDX8_SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX4);
	_nCurrentVtxShader = D3DFVF_XYZ | D3DFVF_TEX4;
	FDX8_pDev->SetPixelShader(0);
	_nCurrentPixelShader = 0;

	fdx8_SetRenderState_TEXTUREFACTOR(0xffffffff);//pPlane->d3dClr);
	fdx8_SetRenderState_ALPHABLENDENABLE(FALSE);
	fdx8_SetRenderState_ALPHATESTENABLE(FALSE);

	fdx8_SetRenderState_ZFUNC( D3DCMP_LESSEQUAL );

	FDX8_pDev->SetVertexShaderConstant(CV_EYE_POS_WORLD, &FXfm_pView->m_MtxR.m44.m_vPos, 1);

	ftex_SetTexAddress(0, TRUE, TRUE);

	u32 i;
	for (i=0; i<4; i++)
	{
		n = _anQuadIdx[i];
		pPlane->Quad[i].vTex[0].x = _afQuadU[n]*fBumpTile;
		pPlane->Quad[i].vTex[0].y = _afQuadV[n]*fBumpTile;
	}

	if ( pPlane->nType&RP_EMISSIVE )
	{
		_SetupMoltenStates(pPlane, pEMBM, fScrollU, fScrollV);

		nQuad=11;
		fDeltaY=0.1f * fGlowScale;

		nPasses = 1;
	}
	else if (pPlane->nType&RP_TEXTURE)
	{
		if (pPlane->fOpacity < 1.0f)
		{
			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_SRCALPHA);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_INVSRCALPHA);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);
		}
		_SetupRefTextureStates(pPlane, pEMBM);

		nPasses = 1;
	}
	else if (pPlane->nType&RP_REFLECT && pPlane->nType&RP_REFRACT)
	{
		pPlane->fOpacity = 0.75f;
		if (pPlane->fOpacity < 1.0f)
		{
			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_SRCALPHA);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_INVSRCALPHA);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);
		}
		_SetupWaterStatesPass0(pPlane, pEMBM, fScrollU, fScrollV);	
	
		if (pPlane->pLayer0)
		{
			nPasses = 2;
		}
		else
		{
			nPasses = 1;
		}
	}
	else
	{
		if (pPlane->fOpacity < 1.0f)
		{
			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_SRCALPHA);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_INVSRCALPHA);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);
		}
		_SetupMercuryStatesPass0(pPlane, pEMBM, fScrollU, fScrollV);

		if (pPlane->pLayer0)
		{
			nPasses = 2;
		}
		else
		{
			nPasses = 1;
		}
	}

	fdx8_SetRenderState_ZWRITEENABLE( (nPasses==1)?(TRUE):(FALSE) );

	#if FANG_PLATFORM_WIN
	fdx8_SetRenderState_CLIPPING( TRUE );
	#endif
	fdx8_SetRenderState_CULLMODE( D3DCULL_NONE );
	FDX8_pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pPlane->Quad, sizeof(RenderPlaneVtx_t));
	
	if (pPlane->nType&RP_REFLECT && !(pPlane->nType&RP_REFRACT) && pPlane->pLayer0)
	{
		_SetupMercuryStatesPass1(pPlane, pEMBM, fScrollU, fScrollV);

		fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
		fdx8_SetRenderState_SRCBLEND(D3DBLEND_DESTCOLOR);
		fdx8_SetRenderState_DESTBLEND(D3DBLEND_ZERO);
		fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

		fdx8_SetRenderState_ZWRITEENABLE( TRUE );

		FDX8_pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pPlane->Quad, sizeof(RenderPlaneVtx_t));
	}
	else if (pPlane->nType&RP_REFLECT && pPlane->nType&RP_REFRACT)
	{
		fdx8_SetRenderState_ZWRITEENABLE( TRUE );

		if (pPlane->pLayer0)
		{
			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_DESTCOLOR);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_SRCCOLOR);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

			_SetupWaterStatesPass2(pPlane, pEMBM, fScrollU, fScrollV);
			FDX8_pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pPlane->Quad, sizeof(RenderPlaneVtx_t));
		}
	}

	if (nQuad > 1)
	{
		if ( _SetupMoltenLayers(pPlane, pEMBM, fScrollU, fScrollV) )
		{
			f32 fYOffs = fDeltaY;

			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_ONE);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_ONE);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

			fdx8_SetRenderState_CULLMODE( D3DCULL_NONE );

			for (n=0; n<nQuad-1; n++)
			{
				_MoveUp(fYOffs);
				FDX8_pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pPlane->Quad, sizeof(RenderPlaneVtx_t));
				fYOffs += fDeltaY;
			}
		}
	}

	FDX8_pDev->SetStreamSource(0, NULL, 0);
	fdx8vb_UncacheSelected();

	fdx8_SetRenderState_CULLMODE( D3DCULL_CCW );

	fdx8tex_SetTexture(0, NULL, 0);
	fdx8tex_SetTexture(1, NULL, 1);
	fdx8tex_SetTexture(2, NULL, 2);
	fdx8tex_SetTexture(3, NULL, 3);
    
	if (_bNeedTCRestore)
	{
		_bNeedTCRestore = FALSE;
		u32 i;
		for (i=0; i<4; i++)
		{
			n = _anQuadIdx[i];
			pPlane->Quad[i].vTex[1].x = _afQuadU[n];
			pPlane->Quad[i].vTex[1].y = _afQuadV[n];
		}
	}
#endif
}

void fsh_ActivateRenderPlane(u32 nPlaneID, BOOL bActive)
{
	_aRenderPlanes[ nPlaneID ].bActive = bActive;
}

void _MoveOut(f32 fScale, CFVec3& vFwd)
{
	CFMtx43A OffsMtx;
	OffsMtx.m_vX.Set(vFwd.x*fScale+1.0f, 0.0f, 0.0f); OffsMtx.m_vY.Set(0.0f, vFwd.y*fScale+1.0f, 0.0f); OffsMtx.m_vZ.Set(0.0f, 0.0f, vFwd.z*fScale+1.0f); OffsMtx.m_vPos.Zero();

	fdx8xfm_SetCustomDXMatrix( D3DTS_WORLDMATRIX(0), &OffsMtx, TRUE );
}

#include "fliquid.h"

void fsh_DrawLiquidMesh(u32 nType, u16 nVtx, u16 nPrim, void *pMesh, u16 *pIdx, CFColorRGB *pClr, f32 fOpacity, CFVec3& vFwd, CFTexInst *pLayer0, CFTexInst *pLayer1, CFTexInst *pEMBM)
{
#if FANG_PLATFORM_XB

	if (!pEMBM) return;

	if ( !(nType&RP_EMISSIVE) )
	{
		fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
		fdx8_SetRenderState_SRCBLEND(D3DBLEND_SRCALPHA);
		fdx8_SetRenderState_DESTBLEND(D3DBLEND_INVSRCALPHA);
		fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

		fdx8tex_SetTexture(0, pEMBM, 0);

		if ( (nType&RP_TEXTURE) )
		{
			fdx8tex_SetTexture(1, pLayer0, 1);
			fdx8tex_SetTexture(2, pLayer0, 2);
			fdx8tex_SetTexture(3, pLayer0, 3);

			ftex_SetTexAddress(3, TRUE, TRUE, TRUE);
		}
		else
		{
			fdx8tex_SetTexture(1, _pFullScrTarget, 1);
			fdx8tex_SetTexture(2, _pFullScrTarget, 2);
			fdx8tex_SetTexture(3, _pFullScrTarget, 3);

			ftex_SetTexAddress(3, FALSE, FALSE, FALSE);
		}

		ftex_SetTexAddress(0, TRUE, TRUE);

		CFVec4 vCen;
		vCen.Set( 0, 0, 0, 1 );//pPlane->vCen.x, pPlane->vCen.y, pPlane->vCen.z, 1.0f);
		FDX8_pDev->SetVertexShaderConstant(CV_FIXED_COLOR, &vCen, 1);

		FDX8_SetVertexShader(_anVShader_Handle[VSHADER_PLANAR_REFLECT]);
		_nCurrentVtxShader = _anVShader_Handle[VSHADER_PLANAR_REFLECT];
		_SetPixelShader(PSHADER_CUBE_REFLECT);

		fdx8_SetRenderState_CULLMODE( D3DCULL_NONE );

		f32 afData[4];
		if (pLayer0)
		{
			afData[0] = 1;
			afData[1] = 1;
			afData[2] = 1;
		}
		else
		{
			afData[0] = pClr->fRed;
			afData[1] = pClr->fGreen;
			afData[2] = pClr->fBlue;
		}
		afData[3] = fOpacity;
		FDX8_pDev->SetPixelShaderConstant(0, afData, 1);

		fdx8_SetRenderState_TEXTUREFACTOR(D3DCOLOR_COLORVALUE(pClr->fRed, pClr->fGreen, pClr->fBlue, 1.0f));

		if ( (pLayer0 && !(nType&RP_TEXTURE)) || (pLayer1 && (nType&RP_TEXTURE)) )
		{
			fdx8_SetRenderState_ZWRITEENABLE( FALSE );
		}
		else
		{
			fdx8_SetRenderState_ZWRITEENABLE( TRUE );
		}

		FDX8_pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, nVtx, nPrim, pIdx, D3DFMT_INDEX16, pMesh, sizeof(LiquidFallVtx));

		if ( (pLayer0 && !(nType&RP_TEXTURE)) || (pLayer1 && (nType&RP_TEXTURE)) )
		{
			fdx8_SetRenderState_ZWRITEENABLE( TRUE );

			FDX8_SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX2);
			_nCurrentVtxShader = D3DFVF_XYZ | D3DFVF_TEX2;
			FDX8_pDev->SetPixelShader(0);
			_nCurrentPixelShader = 0;

			if (nType&RP_TEXTURE)
			{
				fdx8tex_SetTexture(0, pLayer1, 1);
			}
			else
			{
				fdx8tex_SetTexture(0, pLayer0, 1);
			}

			fdx8_SetTextureState_COLOROP(   0, D3DTOP_MODULATE );
			fdx8_SetTextureState_COLORARG1( 0, D3DTA_TEXTURE );
			fdx8_SetTextureState_COLORARG2( 0, D3DTA_TFACTOR );
			fdx8_SetTextureState_ALPHAOP(   0, D3DTOP_SELECTARG1 );

			fdx8_SetTextureState_COLOROP(  1, D3DTOP_DISABLE );

			fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
			fdx8_SetRenderState_SRCBLEND(D3DBLEND_ONE);
			fdx8_SetRenderState_DESTBLEND(D3DBLEND_ONE);
			fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

			FDX8_pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, nVtx, nPrim, pIdx, D3DFMT_INDEX16, pMesh, sizeof(LiquidFallVtx));
		}
	}
	else if (pLayer0)
	{
		fdx8_SetRenderState_TEXTUREFACTOR(0xffffffff);//pPlane->d3dClr);
		fdx8_SetRenderState_ALPHABLENDENABLE(FALSE);
		fdx8_SetRenderState_ALPHATESTENABLE(FALSE);

		fdx8_SetRenderState_CULLMODE( D3DCULL_NONE );

		fdx8_SetRenderState_ZFUNC( D3DCMP_LESSEQUAL );

		//molten states
		f32 afData[4];
		afData[0] = afData[1] = afData[2] = afData[3] = 1.0f;
		
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00,   F2DW( 0.01f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01,   F2DW( 0.0f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10,   F2DW( 0.0f ) );
		FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11,   F2DW( 0.01f ) );

		if (pLayer0	&& pLayer1)
		{
			_SetPixelShader(MOLTEN_2LAYER_EMBM);

			fdx8tex_SetTexture(0, pEMBM, 0);
			fdx8tex_SetTexture(1, pLayer0, 1);
			fdx8tex_SetTexture(2, pLayer1, 2);
		}
		else
		{
			if (pEMBM)
			{
				if (pLayer0)
				{
					_SetPixelShader(MOLTEN_1LAYER_EMBM);
					fdx8tex_SetTexture(0, pEMBM, 0);
					fdx8tex_SetTexture(1, pLayer0, 1);
				}
			}
		}

		FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
		//molten states

		FDX8_pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, nVtx, nPrim, pIdx, D3DFMT_INDEX16, pMesh, sizeof(LiquidFallVtx));

		u32 nQuad=11, n;
		f32 fDeltaY=0.001f;

		if (nQuad > 1)
		{
			//Setup Molten layers.
			FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00,   F2DW( 0.025f ) );
			FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01,   F2DW( 0.0f ) );
			FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10,   F2DW( 0.0f ) );
			FDX8_pDev->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11,   F2DW( 0.025f ) );

			ftex_SetTexAddress(0, TRUE, TRUE);
			ftex_SetTexAddress(1, TRUE, TRUE);
			ftex_SetTexAddress(2, TRUE, TRUE);

			FDX8_SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX4);
			_nCurrentVtxShader = D3DFVF_XYZ | D3DFVF_TEX4;

			if (pLayer0	&& pLayer1)
			{
				_SetPixelShader(MOLTEN_2LAYER_EMBM_GLOW);

				fdx8tex_SetTexture(0, pEMBM, 0);
				fdx8tex_SetTexture(1, pLayer0, 1);
				fdx8tex_SetTexture(2, pLayer1, 2);
			}
			else
			{
				if (pEMBM)
				{
					if (pLayer0)
					{
						_SetPixelShader(MOLTEN_1LAYER_EMBM);
						fdx8tex_SetTexture(0, pEMBM, 0);
						fdx8tex_SetTexture(1, pLayer0, 1);
					}
				}
			}

			f32 afData[4];
			afData[0] = afData[1] = afData[2] = afData[3] = 0.125f;
			FDX8_pDev->SetPixelShaderConstant(0, afData, 1);
			//
			{
				f32 fYOffs = fDeltaY;

				fdx8_SetRenderState_ALPHABLENDENABLE(TRUE);
				fdx8_SetRenderState_SRCBLEND(D3DBLEND_ONE);
				fdx8_SetRenderState_DESTBLEND(D3DBLEND_ONE);
				fdx8_SetRenderState_BLENDOP(D3DBLENDOP_ADD);

				for (n=0; n<nQuad-1; n++)
				{
					_MoveOut(fYOffs, vFwd);
					FDX8_pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, nVtx, nPrim, pIdx, D3DFMT_INDEX16, pMesh, sizeof(LiquidFallVtx));
					fYOffs += fDeltaY;
				}
			}
		}
	}				
#endif
}
