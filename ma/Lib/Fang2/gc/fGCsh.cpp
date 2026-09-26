//////////////////////////////////////////////////////////////////////////////////////
// fGCsh.cpp - 
//
// Author: John Lafleur     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc. d
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/18/02	Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
//for acosf(), it doesn't exist in our math library.
#include <math.h>

#include "fang.h"
#include "fGC.h"
#include "fGCsh.h"
#include "fGCvid.h"
#include "fGCtex.h"
#include "fGCxfm.h"
#include "fGCtex.h"
#include "fGCshaders.h"

#include "flinklist.h"
#include "fres.h"
#include "fcolor.h"
#include "fshaders.h"
#include "fperf.h"
#include "flight.h"
#include "fclib.h"
#include "frenderer.h"
#include "fGCdisplaylist.h"

#include "fliquid.h"
#include "fworld.h"
#include "fmath_vec.h"
#include "fshadow.h"
#include "fvis.h"
#include "fres.h"

#include "fcamera.h"
#include "fliquid.h"
#include "frendersort.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _MAX_TEX_COORD_MTX_COUNT	FSH_MAX_TEX_LAYER_COUNT
#define _MAX_TEX_OVERRIDE_COUNT		FSH_MAX_TEX_LAYER_COUNT
#define MAX_SREFLECT_TARGETS 4

#define FGCSH_ALLOW_REFRACTION 0


//////////////////////////////////////////////////////////////////////////////////////
// Global variables:
//////////////////////////////////////////////////////////////////////////////////////

BOOL FSh_bSurfaceOnly=FALSE;
BOOL FSh_bAlphaOut=FALSE;
BOOL fgcsh_bColorKey=FALSE;
BOOL FSh_bUseFastPass=FALSE;
CFVec3A FSh_PlayerPos;

//FSh_RENDER_ALL
//FSh_ShaderFlags_e FSh_shaderFlags=(FSh_ShaderFlags_e)(FSh_RENDER_REFLECTIONMAP|FSh_RENDER_DETAIL|FSh_RENDER_BUMP|
//													  FSh_RENDER_MULTIPASS_LIGHTING|FSh_RENDER_MOTIF_LIGHTMAPS);

FSh_ShaderFlags_e FSh_shaderFlags=(FSh_ShaderFlags_e)(FSh_RENDER_ALL);

CFLight *FSh_pSpecularLight;

//Configuration Settings for shaders
f32 FSh_fDetailTile=4.0f;
f32 FSh_fBumpTile=1.0f;

f32 fgcsh_fProjOffs=0.0f;
f32 fgcsh_fProjRngAdj=1.0f;

BOOL FSh_bInvAlpha_Emissive=FALSE;
BOOL FSh_ZEnable = TRUE;


enum LM_MODE
{
	LM_NONE=0,
	ZMASK_EMASK_LM2,
	ZMASK_LM3,
	EMASK_LM3,
	LM4
};

u8 m_nLMMode;
u8 m_nLM;

BOOL _bFullScrActive=FALSE;

//Fog Values
f32 fsh_fFogNearZ=0.0f;
f32 fsh_fFogFarZ=500.0f;
f32 fsh_fFogConst[4];
f32 fsh_fFogMinY=30.0f;
f32 fsh_fFogMaxY=70.0f;
f32 fsh_fFogMinYIntens=0.20f;
f32 fsh_fFogDensity=1.0f;
CFColorRGB fsh_fFogColor(0.5f, 0.5f, 0.5f);
CFTexInst *fsh_pFogTex=NULL;
//

void *FSh_pProcedural=NULL;

static u32 FSh_CurDepthBias=0;

extern f32 FGCViewport_ProjDepth;

BOOL fgcsh_bWorldGeo=FALSE;

BOOL FSh_bShadowRender=FALSE;
s32 FSh_ShadowID=0;

GXColor _gxKonst[4];

//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

// Used for texture coordinate transformations (like scrolling):
typedef struct 
{
	u32 nTexLayerID;				// Texture layer ID
	const CFMtx43 *pTexCoordMtx;	// Pointer to matrix to be applied
} _TexCoordMtx_t;


// Used for texture override (like flipping):
typedef struct 
{
	u32 nTexLayerID;				// Texture layer ID
	CFTexInst *pTexInst;			// Texture to be applied
} _TexOverride_t;

typedef struct
{
	u32 nTexLayerID;
	CFTexInst *pCur, *pOrig;
} _TexOverride_Cache_t;

typedef struct
{
	CFVec3A vPos;
	GXColor GCColor;
	float fOOR_WS;
	GXTexObj *pGXShadow;
	
	CFLight *pFLight;
} PerPixelPointLight_t;

typedef struct
{
	CFVec3A vPos;
	CFMtx43 *pMtx_WS;
	CFVec3A vDir_VS;
	CFVec3A vPos_VS;
	GXColor GCColor;
	float fFOV;
	GXTexObj *pGXTex;
	GXTexObj *pGXShadow;
	
	CFLight *pFLight;
	
	f32 fRange;
} PerPixelSpotLight_t;

typedef struct
{
	CFTexInst *pTexInst;
	void *pUser;
	f32 fSize;
} fsh_ReflectTarget_t;

typedef enum
{
	PMTX_PERPIXEL_SPECULAR = GX_PTTEXMTX10,
	PMTX_NORMAL_REFLECT = GX_PTTEXMTX11,
	PMTX_PERPIXEL_SPECKILL = GX_PTTEXMTX12,
	PMTX_LDIR = GX_PTTEXMTX13,
	PMTX_PROJECTION = GX_PTTEXMTX14,
	PMTX_CLIPPLANE = GX_PTTEXMTX15,
	PMTX_PROJECTION_INVERSE = GX_PTTEXMTX16,
	PMTX_FOG_DEPTH = GX_PTTEXMTX17,
	PMTX_FOG_HEIGHT = GX_PTTEXMTX18,

	PMTX_NUM_MTX = 9
} PostXF_Mtx_e;


//////////////////////////////////////////////////////////////////////////////////////
// Local variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;
static BOOL _bShaderSystemOpen;

static FLinkRoot_t _RootRS;					// Linklist of render states
static FLinkRoot_t _RootTS;					// Linklist of texture stage states
static FLinkRoot_t _RootRI;					// Linklist of texture stage infos

static u32 _anGlobalRegisters[FSHREG_GLOBAL_COUNT];		// Array of global registers

// Buffer for textures pointers from registers
static FShTexInst_t *_apRegShTexInst[8];

static CFColorMotif _Default_GReg_Ambient;	// Default register value for global register AMBIENT
static CFColorMotif _Default_GReg_Motif00;	// Default register value for global register FSHREG_MOTIF_A0_C0
static CFColorMotif _Default_GReg_Motif01;	// Default register value for global register FSHREG_MOTIF_A0_C1
static CFColorMotif _Default_GReg_Motif10;	// Default register value for global register FSHREG_MOTIF_A1_C0
static CFColorMotif _Default_GReg_Motif11;	// Default register value for global register FSHREG_MOTIF_A1_C1


#if !FANG_PRODUCTION_BUILD
static BOOL8 _bReportedUnsupported[64];
#endif

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

static BOOL _bSetDetailScale=FALSE;
static u32 _nDetailReg = 0;

static BOOL _bFogEnabled;

static CFColorRGBA _GReg_Ambient(0, 0, 0, 0);

GXTexObj *_apGXTexBuffer[8];
static u32 _nCurrentShader = 0xffffffff;
static BOOL _bStageSwapMode[16] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, 
									FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };
static u8 _nSwapClr[8], _nSwapAlpha[8];
static FRenderMode_e _nRenderMode = FRMODE_NORMAL;

static BOOL _bFastShader = FALSE;

static int _nPerPixelPoint=0, _nPerPixelSpot = 0;
PerPixelPointLight_t _aPerPixelPointLights[8];
PerPixelSpotLight_t _aPerPixelSpotLights[4];

GXTexObj _gxAttenMap, _gxTexKill, _gxTexKill2, _gxAttenMapY, _gxFogRamp1, _gxFogRamp2, _gxDot3_2D;
GXColor _gxFogClr={0,0,0,0};

FViewport_t *_pSView=NULL;
static BOOL _bZBufferEqual=FALSE;
static BOOL _bIndLookup=FALSE;
u32 _nPosMatrix=0;
static u32 _nNrmlMatrix=0;

static _TexOverride_Cache_t _aTexOverrideCache[8];

static u32 _nNumSReflectTargets=0;
static fsh_ReflectTarget_t _aSReflectTargets[MAX_SREFLECT_TARGETS];
static CFTexInst *_pFullScrTarget, *_pFullScrLoRes;

static u32 _nLMapTC;

static BOOL _bSpotShadows=FALSE;
static BOOL _bPointShadows=FALSE;

GXLightObj GCLight;

f32 _fLayerAlpha=1.0f;
CFColorRGBA _fSurfaceColor(1.0f, 1.0f, 1.0f, 1.0f);
GXColor _gxLayerAlpha={0xff, 0xff, 0xff, 0xff};
GXColor _gxSurfaceColor={0xff, 0xff, 0xff, 0xff};

u32 _nShaderID, _nSurfaceShaderID, _nDiffuseShaderID;
static u32 _nPassIdx;
static BOOL _bUseTrans;

f32 _indMtx_Warp[2][3]=
{
	0.5f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.0f
};

//////////////////////////////////////////////////////////////////////////////////////
// Static function prototypes:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );
static BOOL _BuildGCLight( GXLightObj *pGCLight, CFLight *pLight, BOOL bPerPixelLight );

FINLINE GXColor _GXCOLOR_RGB(float r, float g, float b, float a);

static void _ResetTEVStates( void );

static void _GenerateAttenMap(void);

void _LightLookAt(MtxPtr pLookAt, CFVec3A& camPos, CFVec3A& up, CFVec3A& target);
void _LightPerspective(MtxPtr m, float fov, float fDist=0.0f);
void _ApplyInvView(Mtx44Ptr pMtx);

void _HandleProcedural(CFLiquidVolume *pLiquid);
void _CheckTexCoordMtx();
//void _CheckTexOverride();

//static void _IndirectBumpST(CFVec3A& vDir, f32 fScale, u8& nTev, u8& nInd, u8& nTexGen, u8& nTexMtx, u8& nTex);
static void _IndirectWarp(u8 nStage, f32 fScale, s8 nScaleExp, u8& nTev, u8& nInd, u8& nTexGen, u8& nTexMtx, u8& nTex, BOOL bReApply, BOOL bUseReg=TRUE);
static void _IndirectWarp_End(u8 nStage, f32 fScale, s8 nScaleExp, u8 nTex);

static BOOL FSh_WorldCallbackFunc( FWorldEvent_e nEvent );

//static void _HandleSpecularPass(void);
void _PerPixelSpec(void);
void GenerateDirMtx(CFVec3A& vDir, MtxPtr mtx);
void _PlanePerspective(MtxPtr m, f32 fovX, f32 fovY);

void _SetTexture( GXTexObj *pGCTexObj, GXTexMapID nIDNum );

void _HandleFastPass();

void fsh_FullScrCallback(void *pUserData);
void fsh_FullScrLoResCallback(void *pUserData);

void fsh_EndFrame();
void fsh_CreateFullScreenTarget();

void fsh_SetupClipPlane(CFVec3 vPlane, CFVec3 vPoint, BOOL bReflect=FALSE);
void fsh_ClearClipPlane();

BOOL FSh_bUseClipPlane=FALSE;

static void _SetupIndTypes();

BOOL FSh_WorldCallbackFunc( FWorldEvent_e nEvent )
{
	switch (nEvent)
	{
		case FWORLD_EVENT_WORLD_PRELOAD:
		case FWORLD_EVENT_WORLD_POSTDESTROY:
			fsh_ClearRenderPlanes();
			fsh_ClearClipPlane();
			break;
		case FWORLD_EVENT_WORLD_POSTLOAD:
		case FWORLD_EVENT_WORLD_PREDESTROY:
			break;
	};
	return TRUE;
}

#define _TEVKCOLORSEL(stageID, konst) GXSetTevKColorSel(_anTevStage[stageID], (GXTevKColorSel)konst)
#define _TEVKALPHASEL(stageID, konst) GXSetTevKAlphaSel(_anTevStage[stageID], (GXTevKAlphaSel)konst)
#define _TEVORDER(stageID, texCoordID, texMapID, ChannelID) GXSetTevOrder(_anTevStage[stageID], (GXTexCoordID)texCoordID, (GXTexMapID)texMapID, (GXChannelID)ChannelID)
#define _TEVCOLORIN(stageID, a, b, c, d) GXSetTevColorIn(_anTevStage[stageID], (GXTevColorArg)a, (GXTevColorArg)b, (GXTevColorArg)c, (GXTevColorArg)d)
#define _TEVALPHAIN(stageID, a, b, c, d) GXSetTevAlphaIn(_anTevStage[stageID], (GXTevAlphaArg)a, (GXTevAlphaArg)b, (GXTevAlphaArg)c, (GXTevAlphaArg)d)
#define _TEVCOLOROP(stageID, op, bias, scale, clamp, resArg) GXSetTevColorOp(_anTevStage[stageID], (GXTevOp)op, (GXTevBias)bias, (GXTevScale)scale, clamp, (GXTevRegID)resArg)
#define _TEVALPHAOP(stageID, op, bias, scale, clamp, resArg) GXSetTevAlphaOp(_anTevStage[stageID], (GXTevOp)op, (GXTevBias)bias, (GXTevScale)scale, clamp, (GXTevRegID)resArg)
#define _TEVSWAPMODE(stageID, swapColor, swapAlpha) GXSetTevSwapMode(_anTevStage[stageID], (GXTevSwapSel)swapColor, (GXTevSwapSel)swapAlpha)

//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
BOOL fsh_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );
	
#if !FANG_PRODUCTION_BUILD	
	fang_MemZero( _bReportedUnsupported, sizeof( BOOL8 ) * 64 );
#endif

	_bWindowCreated = FALSE;
	_bShaderSystemOpen = FALSE;
	
	_nTexCoordMtxMaxCount = _MAX_TEX_COORD_MTX_COUNT;
	_nTexCoordMtxCount = 0;
	_nTexOverrideMaxCount = _MAX_TEX_OVERRIDE_COUNT;
	_nTexOverrideCount = 0;
	_nPrevTexOverrideCount = 0;
	
	FSh_pSpecularLight = NULL;
	
	_ResetTEVStates();
	
	_bModuleInitialized = TRUE;
	
	_nRenderMode=FRMODE_NORMAL;
	
	_GenerateAttenMap();
	_SetupIndTypes();
	
	_bIndLookup = FALSE;
	
	FSh_PlayerPos.Set(0,0,0);
	
	return TRUE;
}


//
//
//
void fsh_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_ResetTEVStates();

	_bModuleInitialized = FALSE;
}

//
//
//
static void _ResetTEVStates( void ) 
{
	u32 i;
	_nCurrentShader = 0xffffffff;
	for ( i = 0; i < 16; i++ )
	{
		_bStageSwapMode[i] = FALSE;
	}
	
	for ( i = 0; i < 8; i++ )
	{
		_apGXTexBuffer[i] = NULL;
	}
	
	for (i=0; i<4; i++)
	{	
		_nSwapClr[i]=(GXTevSwapSel)(GX_TEV_SWAP3+1);
		_nSwapAlpha[i]=(GXTevSwapSel)(GX_TEV_SWAP3+1);
	}
	
	if ( _bModuleInitialized )
	{
		GXSetChanMatColor(GX_COLOR0, _GXCOLOR_RGB(1.0f, 1.0f, 1.0f, 1.0f));
	}
}

static void _GenerateAttenMap(void)
{
	static u8 *__pImageData_A;
	static u8 *__pImageData_A2;
	static u8 *__pImageData_L;
	static u8 *__pImageData_L2;
	static u8 *__pImageData_D1;
	static u8 *__pImageData_D3;
	s32 nS, nT;
	float fX, fY, fZ, f2O255=(2.0f/255.0f), fOO127=(1.0f/127.0f);
	
	__pImageData_A = (u8*)fres_AlignedAlloc(256*256, 32);
	__pImageData_A2 = (u8*)fres_AlignedAlloc(256*4, 32);
	__pImageData_L = (u8*)fres_AlignedAlloc(256*4, 32);
	__pImageData_L2 = (u8*)fres_AlignedAlloc(256*4, 32);
	__pImageData_D1 = (u8*)fres_AlignedAlloc(256*4, 32);
	__pImageData_D3 = (u8*)fres_AlignedAlloc(256*4, 32);
		
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<256; nT++)
		{
			fX = (float)nS;
			fX = fX*f2O255 - 1.0f;
			fY = (float)nT;
			fY = fY*f2O255 - 1.0f;
			fZ = fX*fX+fY*fY;
			if (fZ < 0.0f) fZ = 0.0f;
			if (fZ > 1.0f) fZ = 1.0f;
			__pImageData_A[nS + (nT<<8)] = (u8)(0xff - (u8)(fZ*255.0f));
		}
	}
	
	_Swizzle_I8(256, 256, __pImageData_A);
	
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			fX = (float)nS;
			fX = fX*f2O255 - 1.0f;
			fZ = fX*fX;
			if (fZ < 0.0f) fZ = 0.0f;
			if (fZ > 1.0f) fZ = 1.0f;
			__pImageData_A2[nS + (nT<<8)] = (u8)(0xff - (u8)(fZ*255.0f));
		}
	}
	
	_Swizzle_I8(256, 4, __pImageData_A2);
	
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			__pImageData_L[nS + (nT<<8)] = (u8)( (nS == 0)?(0x00):(0xff) );
		}
	}
	
	_Swizzle_I8(256, 4, __pImageData_L);
	
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			__pImageData_L2[nS + (nT<<8)] = (u8)( (nS == 0)?(0x00):(0xff-nS) );
		}
	}
	
	_Swizzle_I8(256, 4, __pImageData_L2);
	
	u8 nX;
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			nX = (u8)(nS);
			if (nX < 4) nX = 4;
			__pImageData_D1[nS + (nT<<8)] = nX;
		}
	}
	
	_Swizzle_I8(256, 4, __pImageData_D1);
	
	for (nS=0; nS<256; nS++)
	{
		for (nT=0; nT<4; nT++)
		{
			nX = (u8)(nS);
			if (nX < 128) nX = 128;
			__pImageData_D3[nS + (nT<<8)] = nX;
		}
	}
	
	_Swizzle_I8(256, 4, __pImageData_D3);
		
	GXInitTexObj(&_gxAttenMap, __pImageData_A, 256, 256, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GXInitTexObj(&_gxAttenMapY, __pImageData_A2, 256, 4, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GXInitTexObj(&_gxTexKill, __pImageData_L, 256, 4, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GXInitTexObj(&_gxTexKill2, __pImageData_L2, 256, 4, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GXInitTexObj(&_gxFogRamp1, __pImageData_D1, 256, 4, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GXInitTexObjFilter(&_gxFogRamp1, GX_LINEAR, GX_LINEAR);
	GXInitTexObj(&_gxFogRamp2, __pImageData_D3, 256, 4, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GXInitTexObjFilter(&_gxFogRamp2, GX_LINEAR, GX_LINEAR);
}


//
//
//
u32 fsh_GetGlobalRegister( FShReg_e nRegNum ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nRegNum>=FSHREG_GLOBAL_OFFSET && nRegNum<FSHREG_ONE_PAST_LAST );

	return _anGlobalRegisters[nRegNum-FSHREG_GLOBAL_OFFSET];
}


//
// Returns previous value.
//
u32 fsh_SetGlobalRegister( FShReg_e nRegNum, u32 nNewValue ) 
{
	u32 nPrevValue;

	FASSERT( _bModuleInitialized );
	FASSERT( nRegNum>=FSHREG_GLOBAL_OFFSET && nRegNum<FSHREG_ONE_PAST_LAST );

	nPrevValue = _anGlobalRegisters[nRegNum-FSHREG_GLOBAL_OFFSET];
	_anGlobalRegisters[nRegNum-FSHREG_GLOBAL_OFFSET] = nNewValue;

	return nPrevValue;
}


//
//
//
void fsh_Open( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !_bShaderSystemOpen );

	_bShaderSystemOpen = TRUE;
	
	fsh_PrepareFrame();
}


//
//
//
void fsh_Close( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	_bShaderSystemOpen = FALSE;
	
	fsh_EndFrame();
}

void _PlanePerspective(MtxPtr m, f32 fovX, f32 fovY)
{
    float angle, cotX, cotY;

    // find the cotangent of half the (YZ) field of view
    angle = fovX * 0.5f;
    cotX = 0.5f / tanf(angle);
    
    angle = fovY * 0.5f;
    cotY = 0.5f / tanf(angle);

    m[0][0] =    cotX;
    m[0][1] =    0.0f;
    m[0][2] =    -0.5f;
    m[0][3] =    0.0f;

    m[1][0] =    0.0f;
    m[1][1] =    -cotY;
    m[1][2] =    -0.5f;
    m[1][3] =    0.0f;

    m[2][0] =    0.0f;
    m[2][1] =    0.0f;
    m[2][2] =   -1.0f;
    m[2][3] =    0.0f;
}


void fsh_LoadProjMtx(f32 fovX, f32 fovY)
{
	Mtx44 proj;
	_PlanePerspective(proj, fovX, fovY);
	
	proj[3][0] = proj[3][1] = proj[3][2] = 0;
    proj[3][3] = 1;
		
	GXLoadTexMtxImm(proj, PMTX_PROJECTION, GX_MTX3x4);
	proj[1][1] = -proj[1][1];
	GXLoadTexMtxImm(proj, PMTX_PROJECTION_INVERSE, GX_MTX3x4);
}

//
//
//
void fsh_SetDepthBias( u32 nBias ) 
{
	FASSERT( _bWindowCreated );
	
	if (nBias != FSh_CurDepthBias)
	{
		//tweak the projection
		f32 pm[GX_PROJECTION_SZ];
		GXGetProjectionv(pm);
		
		pm[6] = FGCViewport_ProjDepth - (f32)(nBias)*0.001f;
		
		GXSetProjectionv(pm);
		FSh_CurDepthBias = nBias;
		//
	}
}


//
//
//
u32 fsh_GetDepthBias( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	return _nDepthBias;
}

void fsh_SetDetailMapTileFactor(f32 fTileFactor)
{
	FSh_fDetailTile = fTileFactor;
}

void fsh_SetBumpMapTileFactor(f32 fTileFactor)
{
	FSh_fBumpTile = fTileFactor;
}

f32 fsh_GetDetailMapTileFactor()
{
	return (FSh_fDetailTile);
}

f32 fsh_GetBumpMapTileFactor()
{
	return (FSh_fBumpTile);
}

void fsh_Fog_SetParam(f32 fNearZ, f32 fFarZ, f32 fMinY, f32 fMaxY, f32 fDensity, CFColorRGB& FogClr, CFTexInst *pFogTex)
{
	fsh_fFogDensity = fDensity;
	fsh_fFogNearZ = fNearZ;
	fsh_fFogFarZ = fFarZ;
	fsh_fFogMinY = fMinY;
	fsh_fFogMaxY = fMaxY;
	fsh_fFogColor = FogClr;

	//Fdepth = (Zview - NearZ)/(FarZ - NearZ)
	//Fdepth = Zview*(1/(FarZ-NearZ)) + (-NearZ/(FarZ-NearZ)) [ mad in vertex shader ASM ]
	fsh_fFogConst[0] = -fsh_fFogDensity /(fsh_fFogFarZ - fsh_fFogNearZ);
	fsh_fFogConst[1] =  fsh_fFogNearZ*fsh_fFogConst[0];
	//Fheight = 1.0 - (Yworld - MinY)/(MaxY - MinY)
	//Fheight = -Yworld*(1/(MaxY-MinY)) + (MaxY/(MaxY-MinY)) [ mad in vertex shader ASM ]
	fsh_fFogConst[2] = -1.0f/(fsh_fFogMaxY - fsh_fFogMinY);
	fsh_fFogConst[3] = -fsh_fFogMaxY*fsh_fFogConst[2];
	//mad: res(x) = x*a + b

	fsh_pFogTex = pFogTex;
}

//
//
//
void fsh_TexCoordMtx_ResetList( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	_nTexCoordMtxCount = 0;
	
	u32 i;
	for ( i=0; i<4; i++ )
	{
		_aPerPixelSpotLights[i].pGXTex = &_gxAttenMap;
		_aPerPixelSpotLights[i].pGXShadow = NULL;
	}
}


//
// Important: pTexCoordMtx must persist until no further fsh_Execute() calls require it.
//
void fsh_TexCoordMtx_Add( u32 nTexLayerID, const CFMtx43 *pTexCoordMtx ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	
	_aTexCoordMtx[_nTexCoordMtxCount].nTexLayerID = nTexLayerID;
	_aTexCoordMtx[_nTexCoordMtxCount].pTexCoordMtx = pTexCoordMtx;

	_nTexCoordMtxCount++;
}


//
//
//
u32 fsh_TexCoordMtx_GetCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexCoordMtxCount;
}


//
//
//
u32 fsh_TexCoordMtx_GetMaxCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexCoordMtxMaxCount;
}


//
//
//
void fsh_TexOverride_ResetList( void ) 
{
	FASSERT( _bWindowCreated );
	_nTexOverrideCount = 0;
}


//
// Important: pTexDef must persist until no further fsh_Execute() calls require it.
//
void fsh_TexOverride_Add( u32 nTexLayerID, CFTexInst *pTexInst ) 
{
	FASSERT( _bWindowCreated );

	if( _nTexOverrideCount >= _nTexOverrideMaxCount ) {
		DEVPRINTF( "fsh_TexOverride_Add(): Too many texture overrides specified. Max is %u.\n", _nTexOverrideMaxCount );
		return;
	}

	#if FANG_DEBUG_BUILD
		// A specific nTexLayerID should be added only once (for performance reasons)...
		for( u32 i=0; i<_nTexOverrideCount; i++ ) {
			if( nTexLayerID == _aTexOverride[i].nTexLayerID ) {
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


//
//
//
u32 fsh_TexOverride_GetCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexOverrideCount;
}


//
//
//
u32 fsh_TexOverride_GetMaxCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nTexOverrideMaxCount;
}


//
//
//
void fsh_Light_ResetList( void ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	_nLightCount = 0;
	_nPerPixelPoint=0;
	_nPerPixelSpot=0;
	
	_bSpotShadows = FALSE;
	_bPointShadows = FALSE;
}

//
// Important: pLight must persist until no further fsh_Execute() calls require it.
//
void fsh_Light_Add( CFLight *pLight, BOOL bLightPerPixel ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	
	if ( _nLightCount >= FGC_nMaxLights ) 
	{
		// No room in light list...
		return;
	}

	if ( pLight->m_nFlags & FLIGHT_FLAG_ENABLE ) 
	{
		// SER: Added this so that we can use coronas without lights (hope it doesn't cause any problems)!
		if ( !(pLight->m_nFlags & FLIGHT_FLAG_CORONA_ONLY) )
		{
			// We will light the mesh per pixel if the mesh accepts per pixel lights
			// and this light is a per pixel light...
			bLightPerPixel = (bLightPerPixel && (pLight->m_nFlags & FLIGHT_FLAG_PER_PIXEL));
			
			// ...or if this light is a spotlight and it has a projection texture and
			// the mesh is to be lit per pixel or the light does not require the mesh to be lit per pixel
			bLightPerPixel |= (pLight->m_nType == FLIGHT_TYPE_SPOT) && pLight->m_pProjectionTex && (bLightPerPixel || !(pLight->m_nFlags & FLIGHT_FLAG_MESH_MUST_BE_PER_PIXEL));
			
			if (_BuildGCLight( &GCLight, pLight, bLightPerPixel ) )
			{
				GXLoadLightObjImm( &GCLight, (GXLightID)(1 << _nLightCount) );
				_nLightCount++;
				
				FASSERT( _nLightCount <= 6 );
			}
		}
	}
}

//
//
//
static BOOL _BuildGCLight( GXLightObj *pGCLight, CFLight *pLight, BOOL bPerPixelLight ) 
{
	pLight->ComputeColor();

	GXColor GCColor;
	GCColor.r = (u8)(pLight->m_ScaledColor.fRed * 255.f);
	GCColor.g = (u8)(pLight->m_ScaledColor.fGreen * 255.f);
	GCColor.b = (u8)(pLight->m_ScaledColor.fBlue * 255.f);
	GCColor.a = (u8)(pLight->m_ScaledColor.fAlpha * 255.f);
	GXInitLightColor( pGCLight, GCColor );

	if ( pLight->m_nFlags & FLIGHT_FLAG_HASDIR )
	{
		if ( pLight->m_nXfmKey_VS != FXfm_nViewKey)
		{
			pLight->m_vUnitDir_VS = FGCXfm_mtxGCLeftToRightHandViewMtx.m44.MultDir( pLight->m_mtxOrientation_WS.m_vFront );
		}
		GXInitLightDir( pGCLight, pLight->m_vUnitDir_VS.x, pLight->m_vUnitDir_VS.y, pLight->m_vUnitDir_VS.z );
	} 
	else
	{
		GXInitLightDir( pGCLight, 0.f, 0.f, 0.f );
	}

	if ( pLight->m_nFlags & FLIGHT_FLAG_HASPOS ) 
	{
		if ( pLight->m_nXfmKey_VS != FXfm_nViewKey )
		{
			pLight->m_vPos_VS = FGCXfm_mtxGCLeftToRightHandViewMtx.m44.MultPoint( pLight->m_spInfluence_WS.m_Pos );
		}

		if (bPerPixelLight)
		{		
			if (pLight->m_nType == FLIGHT_TYPE_OMNI)
			{
				if (_nPerPixelPoint < 8)
				{
					f32 fR, fG, fB;
					fR = fmath_Sqrt(pLight->m_ScaledColor.fRed);   fR = fmath_Sqrt(fR);
					fG = fmath_Sqrt(pLight->m_ScaledColor.fGreen); fG = fmath_Sqrt(fG);
					fB = fmath_Sqrt(pLight->m_ScaledColor.fBlue);  fB = fmath_Sqrt(fB);
					
					if (pLight->m_pShadowTex&&0)
					{
						_aPerPixelPointLights[_nPerPixelPoint].pGXShadow = (GXTexObj *)pLight->m_pShadowTex->GetTexDef()->pTexData->pGCTexObj;
						_bPointShadows = TRUE;
					}
					else
					{
						_aPerPixelPointLights[_nPerPixelPoint].pGXShadow = NULL;
					}
				
					_aPerPixelPointLights[ _nPerPixelPoint ].vPos.Set(pLight->m_spInfluence_WS.m_Pos);
					_aPerPixelPointLights[ _nPerPixelPoint ].GCColor = _GXCOLOR_RGB(fR, fG, fB, 1.0f);//GCColor;
					_aPerPixelPointLights[ _nPerPixelPoint ].fOOR_WS = 0.90f*pLight->m_fOOR_WS;//*2.0f;
					
					_aPerPixelPointLights[ _nPerPixelPoint ].pFLight = pLight;
					_nPerPixelPoint++;
				}
				
				return FALSE; //per-pixel light, can't use Lighting hardware
			}
			else if (pLight->m_nType == FLIGHT_TYPE_SPOT && _nPerPixelSpot < 4 && pLight->m_spInfluence_WS.m_fRadius)//( (_bWorldGeo)?(4):(2) ))
			{
				_aPerPixelSpotLights[ _nPerPixelSpot ].vPos.Set(pLight->m_spInfluence_WS.m_Pos);
				_aPerPixelSpotLights[ _nPerPixelSpot ].pMtx_WS = &pLight->m_mtxOrientation_WS;
				_aPerPixelSpotLights[ _nPerPixelSpot ].vDir_VS.Set(pLight->m_vUnitDir_VS.x, pLight->m_vUnitDir_VS.y, pLight->m_vUnitDir_VS.z);
				_aPerPixelSpotLights[ _nPerPixelSpot ].vPos_VS.Set(pLight->m_vPos_VS.x, pLight->m_vPos_VS.y, pLight->m_vPos_VS.z);
				_aPerPixelSpotLights[ _nPerPixelSpot ].GCColor = GCColor;
				_aPerPixelSpotLights[ _nPerPixelSpot ].fFOV = pLight->m_fSpotOuterRadians;
				if (pLight->m_pProjectionTex && pLight->m_pProjectionTex->GetTexDef() )
				{
					_aPerPixelSpotLights[ _nPerPixelSpot ].pGXTex = (GXTexObj *)pLight->m_pProjectionTex->GetTexDef()->pTexData->pGCTexObj;
				}
				else
				{
					_aPerPixelSpotLights[ _nPerPixelSpot ].pGXTex = &_gxAttenMap;
				}
				
				if (pLight->m_pShadowTex&&0)
				{
					_aPerPixelSpotLights[_nPerPixelSpot].pGXShadow = (GXTexObj *)pLight->m_pShadowTex->GetTexDef()->pTexData->pGCTexObj;
					_bSpotShadows = TRUE;
				}
				else
				{
					_aPerPixelSpotLights[_nPerPixelSpot].pGXShadow = NULL;
				}
				_aPerPixelSpotLights[ _nPerPixelSpot ].pFLight = pLight;
				_aPerPixelSpotLights[ _nPerPixelSpot ].fRange = 1.0f/pLight->m_spInfluence_WS.m_fRadius;
				_nPerPixelSpot++;

				return FALSE;
			}
			return FALSE;
		}
		
		GXInitLightPos( pGCLight, pLight->m_vPos_VS.x, pLight->m_vPos_VS.y, pLight->m_vPos_VS.z );
		if (pLight->m_nType == FLIGHT_TYPE_SPOT)
		{
			GXInitLightAttn( pGCLight, pLight->m_fSpotGCK0, pLight->m_fSpotGCK1, pLight->m_fSpotGCK2, 
 					FLIGHT_ATTEN_K0,
 					(FLIGHT_ATTEN_K1 * pLight->m_fOOR_WS), 
 					(FLIGHT_ATTEN_K2 * pLight->m_fOOR2_WS) );
		}
		else
		{
			GXInitLightAttn( pGCLight, 1.f, 0.f, 0.f, 
 					FLIGHT_ATTEN_K0,
 					(FLIGHT_ATTEN_K1 * pLight->m_fOOR_WS), 
 					(FLIGHT_ATTEN_K2 * pLight->m_fOOR2_WS) );
		}
	} 
	else 
	{
		// If it doesn't have position, it must be a directional light
		GXInitLightPos( pGCLight, -pLight->m_vUnitDir_VS.x * 65536.f, -pLight->m_vUnitDir_VS.y * 65536.f, -pLight->m_vUnitDir_VS.z * 65536.f );
		GXInitLightAttn( pGCLight, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f );
	}
	
	pLight->m_nXfmKey_VS = FXfm_nViewKey;

	return TRUE;
}


//
//
//
void fgcsh_Light_Activate( BOOL bUseVertexColor, BOOL bActivateChannel2 ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( _bShaderSystemOpen );
	
	fgc_SetChannelsForShaders( bUseVertexColor, _nLightCount, bActivateChannel2, (FSh_shaderType == SHADERTYPE_SPECULAR) );
}

//
//
//
void fgcsh_SetAmbientLight(float r, float g, float b, float a)
{
	_GReg_Ambient.fRed = r;
	_GReg_Ambient.fGreen = g;
	_GReg_Ambient.fBlue = b;
	_GReg_Ambient.fAlpha = a;
}

//
//
//
void fgcsh_Light_SetAmbient( u8 nRed, u8 nGreen, u8 nBlue, u8 nAlpha )
{
	GXColor GCColor;
	f32 fOO255=1.0f/255.0f;
	
	FMATH_CLAMP( nRed, 0, 255 );
	FMATH_CLAMP( nGreen, 0, 255 );
	FMATH_CLAMP( nBlue, 0, 255 );
	FMATH_CLAMP( nAlpha, 0, 255 );
	
	GCColor.r = nRed;
	GCColor.g = nGreen;
	GCColor.b = nBlue;
	GCColor.a = nAlpha;
	
	_GReg_Ambient.fRed = (f32)nRed*fOO255;
	_GReg_Ambient.fGreen = (f32)nGreen*fOO255;
	_GReg_Ambient.fBlue = (f32)nBlue*fOO255;
	_GReg_Ambient.fAlpha = 1.0f;
	
    // Set up ambient color  THIS SHOULD NOT BE PERMANENT!!!
    GXSetChanAmbColor( GX_COLOR0A0, GCColor );
    GCColor.r = GCColor.g = GCColor.b = 255;
}


//
//
//
u32 fsh_Light_GetCount( void ) 
{
	FASSERT( _bWindowCreated );
	return _nLightCount;
}


//
//
//
u32 fsh_Light_GetMaxCount( void ) 
{
	FASSERT( _bWindowCreated );
	return FGC_nMaxLights;
}


//
//
//
void fsh_Fog_Enable( BOOL bEnable, BOOL bForce ) 
{
	FASSERT( _bWindowCreated );
	#if 0
	_bFogEnabled = bEnable;
	#else
	_bFogEnabled = FALSE;
	#endif
}


//
//
//
BOOL fsh_Fog_IsEnabled( void ) 
{
	FASSERT( _bModuleInitialized );
	return _bFogEnabled;
}


//
//
//
void fgcsh_Swap( void ) 
{
	FASSERT( _bModuleInitialized );
	_ResetTEVStates();
}

void fsh_RenderFogPass()
{
	if (!_bFogEnabled) { return; }

	CFVec3 XAxis, YAxis, ZAxis, Pos, vec;
	CFVec3 quad[4];
	CFVec2 quadUV[4];
	CFColorRGBA color;
	float fScale;

	fgc_SetBlendOp(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_INVDSTALPHA, GX_LO_SET);	
	fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
	fgc_SetZMode( TRUE, GX_ALWAYS, FALSE );

	GXSetTevKColor(GX_KCOLOR0, _GXCOLOR_RGB(fsh_fFogColor.fRed, fsh_fFogColor.fGreen, fsh_fFogColor.fBlue, 1.0f));
	GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
	
	GXSetNumTevStages(1);
	GXSetNumIndStages(0);
	GXSetTevDirect(GX_TEVSTAGE0);

	if (fsh_pFogTex)
	{
		GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);	
		
		GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		
		FTexDef_t *pTexDefLayer = fsh_pFogTex->GetTexDef();
		if (pTexDefLayer)
		{
			GXTexObj *pGCTexObjLayer = (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj;
			GXInitTexObjWrapMode( pGCTexObjLayer, GX_REPEAT, GX_REPEAT );
			_SetTexture(pGCTexObjLayer, GX_TEXMAP0);
		}
		
		GXSetNumTexGens(1);
	}
	else
	{
		GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		
		GXSetNumTexGens(0);
	}

	XAxis.x = FXfm_pView->m_MtxR.aa[0][0]; XAxis.y = FXfm_pView->m_MtxR.aa[1][0]; XAxis.z = FXfm_pView->m_MtxR.aa[2][0];
	YAxis.x = FXfm_pView->m_MtxR.aa[0][1]; YAxis.y = FXfm_pView->m_MtxR.aa[1][1]; YAxis.z = FXfm_pView->m_MtxR.aa[2][1];
	ZAxis.x = FXfm_pView->m_MtxR.aa[0][2]; ZAxis.y = FXfm_pView->m_MtxR.aa[1][2]; ZAxis.z = FXfm_pView->m_MtxR.aa[2][2];
	
	fScale = 1.0f;
	
	vec.x = -1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[0].x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x;
	quad[0].y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y;
	quad[0].z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z;

	quadUV[0].x = 0.0f;
	quadUV[0].y = 0.0f;

	vec.x = +1.0f; vec.y = -1.0f; vec.z = 0.0f;
	quad[1].x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x; 
	quad[1].y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y; 
	quad[1].z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z; 

	quadUV[1].x = 4.0f;
	quadUV[1].y = 0.0f;

	vec.x = +1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[2].x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x; 
	quad[2].y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y; 
	quad[2].z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z; 

	quadUV[2].x = 4.0f;
	quadUV[2].y = 4.0f;

	vec.x = -1.0f; vec.y = +1.0f; vec.z = 0.0f;
	quad[3].x = vec.Dot(XAxis)*fScale + FXFM_CAM_ORIG_WS.x + FXfm_pView->m_MtxR.m_vFront.x; 
	quad[3].y = vec.Dot(YAxis)*fScale + FXFM_CAM_ORIG_WS.y + FXfm_pView->m_MtxR.m_vFront.y; 
	quad[3].z = vec.Dot(ZAxis)*fScale + FXFM_CAM_ORIG_WS.z + FXfm_pView->m_MtxR.m_vFront.z;

	quadUV[3].x = 0.0f;
	quadUV[3].y = 4.0f;
	
	// Set the vertex descriptions
	fgc_ClearVtxDesc();
	fgc_SetVtxPosDesc( GX_DIRECT );
	fgc_SetVtxSTDesc( GX_VA_TEX0, GX_DIRECT );
	
	Mtx mView;
	fgcxfm_ConvertXfmToGCMtx( mView, FGCXfm_mtxGCLeftToRightHandViewMtx );
	fgcxfm_LoadGCPosMatrixImm( mView, GX_PNMTX0 );
	
	Mtx mtxTemp;
	MTXIdentity( mtxTemp );
	GXLoadNrmMtxImm( mtxTemp, GX_PNMTX0 );
	fgcxfm_SetGCCurrentMatrix( GX_PNMTX0 );
	
	fgc_SetVtxPosFormat( GX_VTXFMT7, GX_F32, 0 );
	fgc_SetVtxSTFormat( GX_VTXFMT7, GX_VA_TEX0, GX_F32, 0 );

	u32 i;
	GXBegin(GX_QUADS, GX_VTXFMT7, 4);
		for (i=0; i<4; i++)
		{
			GXPosition3f32( quad[i].x, quad[i].y, quad[i].z );
			GXTexCoord2f32( quadUV[i].x, quadUV[i].y );
		}
	GXEnd();
}


//
//
//
void _SetTexture( GXTexObj *pGCTexObj, GXTexMapID nIDNum )
{
	FASSERT( pGCTexObj );
	FASSERT( nIDNum >= 0 && nIDNum < 9 );
	
	if ( _apGXTexBuffer[nIDNum] != pGCTexObj )
	{
#if FPERF_ENABLE
		FPerf_nRawTexSwitchCount++;
#endif
		GXLoadTexObj( pGCTexObj, nIDNum );
		_apGXTexBuffer[nIDNum] = pGCTexObj;
	}
}

void *fsh_GetAttenMap()
{
	return (void *)&_gxAttenMap;
}

static void _SetupIndTypes()
{
	u32 i;
	FShaderIndirectLookup_t *pLookUp;
	FShaderIndirectStage_t *pStage;
	for (i=FSH_IND_DUDV; i<FSH_NUM_IND_TYPES; i++)
	{
		pLookUp = &_aIndLookupTypes[i];
		switch (i)
		{
			case FSH_IND_DUDV:
			{
				pLookUp->nNumIndStages = 3;
				pLookUp->nNumIndReg = 1;
				pLookUp->nScaleExp = 1;
				//IndStage 0
				pStage = &pLookUp->aIndStages[0];
				pStage->TevStage = GX_TEVSTAGE0;
				pStage->IndStage = GX_INDTEXSTAGE0;
				pStage->Format = GX_ITF_8;
				pStage->BiasSel = GX_ITB_ST;
				pStage->MatrixSel = GX_ITM_S0;
				pStage->WrapS = GX_ITW_0;
				pStage->WrapT = GX_ITW_0;
				pStage->bAddPrev = GX_FALSE;
				pStage->bUTC_Lod = GX_FALSE;
				pStage->AlphaSel = GX_ITBA_OFF;
				//IndStage 1
				pStage = &pLookUp->aIndStages[1];
				pStage->TevStage = GX_TEVSTAGE1;
				pStage->IndStage = GX_INDTEXSTAGE0;
				pStage->Format = GX_ITF_8;
				pStage->BiasSel = GX_ITB_ST;
				pStage->MatrixSel = GX_ITM_T0;
				pStage->WrapS = GX_ITW_0;
				pStage->WrapT = GX_ITW_0;
				pStage->bAddPrev = GX_TRUE;
				pStage->bUTC_Lod = GX_FALSE;
				pStage->AlphaSel = GX_ITBA_OFF;
				//IndStage 2
				pStage = &pLookUp->aIndStages[2];
				pStage->TevStage = GX_TEVSTAGE2;
				pStage->IndStage = GX_INDTEXSTAGE0;
				pStage->Format = GX_ITF_8;
				pStage->BiasSel = GX_ITB_NONE;
				pStage->MatrixSel = GX_ITM_OFF;
				pStage->WrapS = GX_ITW_OFF;
				pStage->WrapT = GX_ITW_OFF;
				pStage->bAddPrev = GX_TRUE;
				pStage->bUTC_Lod = GX_FALSE;
				pStage->AlphaSel = GX_ITBA_OFF;
			}
			break;
			case FSH_IND_WARP_ADD:
			case FSH_IND_WARP_REPLACE:
			{
				GXIndTexWrap Wrap = (i==FSH_IND_WARP_ADD)?(GX_ITW_OFF):(GX_ITW_0);
				pLookUp->nNumIndStages = 1;
				pLookUp->nNumIndReg = 1;
				pLookUp->nScaleExp = 1;
				//IndStage 0
				pStage = &pLookUp->aIndStages[0];
				pStage->TevStage = GX_TEVSTAGE0;
				pStage->IndStage = GX_INDTEXSTAGE0;
				pStage->Format = GX_ITF_8;
				pStage->BiasSel = GX_ITB_STU;
				pStage->MatrixSel = GX_ITM_0;
				pStage->WrapS = Wrap;
				pStage->WrapT = Wrap;
				pStage->bAddPrev = GX_FALSE;
				pStage->bUTC_Lod = GX_FALSE;
				pStage->AlphaSel = GX_ITBA_OFF;
			}	
			break;
			case FSH_IND_XYZ:
			{
				pLookUp->nNumIndStages = 1;
				pLookUp->nNumIndReg = 1;
				pLookUp->nScaleExp = 1;
				//IndStage 0
				pStage = &pLookUp->aIndStages[0];
				pStage->TevStage = GX_TEVSTAGE0;
				pStage->IndStage = GX_INDTEXSTAGE0;
				pStage->Format = GX_ITF_8;
				pStage->BiasSel = GX_ITB_STU;
				pStage->MatrixSel = GX_ITM_0;
				pStage->WrapS = GX_ITW_OFF;
				pStage->WrapT = GX_ITW_OFF;
				pStage->bAddPrev = GX_FALSE;
				pStage->bUTC_Lod = GX_FALSE;
				pStage->AlphaSel = GX_ITBA_OFF;
			}
			break;
		}
	}
}

static BOOL _UseTrans(u32 shaderID)
{
	if (shaderID == FSHADERS_tBASE            || shaderID == FSHADERS_etBASE) return (TRUE);
	if (shaderID == FSHADERS_etBASE_ADD_rbENV || shaderID == FSHADERS_ADD_BASE) return (TRUE);
	if (shaderID == FSHADERS_tBASE_ADD_SPEC   || shaderID == FSHADERS_etBASE_ADD_SPEC) return (TRUE);
	if (shaderID == FSHADERS_etBASE_ADD_bSPEC || shaderID == FSHADERS_tBASE_LERP_tLAYER) return (TRUE);
	
	if (shaderID == FSHADERS_pBASE            || shaderID == FSHADERS_epBASE) return (TRUE);
	if (shaderID == FSHADERS_ADD_vBASE 		  || shaderID == FSHADERS_epBASE_ADD_bSPEC) return (TRUE);
	if (shaderID == FSHADERS_pBASE_ADD_SPEC   || shaderID == FSHADERS_epBASE_ADD_SPEC) return (TRUE);
	if (shaderID == FSHADERS_epBASE_ADD_bSPEC) return (TRUE);
	
	if (shaderID == FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER) return (TRUE);
	
	if (shaderID == FSHADERS_tBASE_DETAIL     || shaderID == FSHADERS_etBASE_DETAIL) return (TRUE);
	if (shaderID == FSHADERS_etBASE_ADD_rbENV_DETAIL || shaderID == FSHADERS_ADD_BASE_DETAIL) return (TRUE);
	if (shaderID == FSHADERS_tBASE_ADD_SPEC_DETAIL   || shaderID == FSHADERS_etBASE_ADD_SPEC_DETAIL) return (TRUE);
	if (shaderID == FSHADERS_etBASE_ADD_bSPEC_DETAIL || shaderID == FSHADERS_tBASE_LERP_tLAYER_DETAIL) return (TRUE);
	if (shaderID == FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER_DETAIL) return (TRUE);
	
	if (shaderID >= FSHADERS_DECAL_AI && shaderID <= FSHADERS_INTENSITY) return (TRUE);
	return (FALSE);
}

static BOOL _CheckRenderMode(FShaderType_e nShaderType)
{
	BOOL ret=TRUE;
	
	if (_nRenderMode == FRMODE_DIFFUSE)
	{
		if (nShaderType != SHADERTYPE_DIFFUSE)
		{
			ret = FALSE;
		}
	}
	else if (_nRenderMode == FRMODE_SURFACE)
	{
		if (nShaderType != SHADERTYPE_SURFACE)
		{
			ret = FALSE;
		}
	}
	else if (_nRenderMode == FRMODE_SPECULAR)
	{
		if (nShaderType != SHADERTYPE_SPECULAR)
		{
			ret = FALSE;
		}
	}
	
	return (ret);
}

void fsh_SetLayerAlpha(f32 fAlpha)
{
	if (fAlpha != _fLayerAlpha)
	{
		_fLayerAlpha = fAlpha;
		_gxLayerAlpha = _GXCOLOR_RGB(_fLayerAlpha, _fLayerAlpha, _fLayerAlpha, _fLayerAlpha);
		
		GXSetTevKColor(GX_KCOLOR3, _gxLayerAlpha);
	}
}

void fsh_SetSurfaceColor( f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha )
{
	if ( _nShaderID == FSHADERS_ADD_BASE || _nShaderID == FSHADERS_ADD_BASE_DETAIL || _nShaderID == FSHADERS_ADD_vBASE)
	{
		fRed   *= fAlpha * _fLayerAlpha;
		fGreen *= fAlpha * _fLayerAlpha;
		fBlue  *= fAlpha * _fLayerAlpha;
	}
	else if ( _nShaderID == FSHADERS_tBASE || _nShaderID == FSHADERS_etBASE )
	{
		fAlpha *= _fLayerAlpha;
	}
	
	if (fRed != _fSurfaceColor.fRed || fGreen != _fSurfaceColor.fGreen || fBlue != _fSurfaceColor.fBlue || fAlpha != _fSurfaceColor.fAlpha)
	{
		_fSurfaceColor.fRed = fRed;
		_fSurfaceColor.fGreen = fGreen;
		_fSurfaceColor.fBlue = fBlue;
		_fSurfaceColor.fAlpha = fAlpha;
		
		_gxSurfaceColor = _GXCOLOR_RGB(fRed, fGreen, fBlue, fAlpha);
		GXSetTevKColor(GX_KCOLOR0, _gxSurfaceColor);
	}
}

u32 fsh_GetNumDiffusePasses(u32 shaderID, u32 diffuseID, BOOL *pbFastPass)
{
	u32 nPasses = 1;
	m_nLM = 0;
	m_nLMMode = LM_NONE;
	if (pbFastPass) { *pbFastPass = FALSE; }
	
	#if 0
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

	
	if (_UseTrans(shaderID)) 
	{
		return (0);
	}
	
	nPasses += ((_nPerPixelPoint==0)?(0):( (_nPerPixelPoint<=4)?(1):(2) ));
	nPasses += ((_nPerPixelSpot==0)?(0):(1));

	if (FSh_pnLightMapInputRegisters)
	{
		m_nLM = FSh_pnLightMapInputRegisters[0];
		
		if ( m_nLM > 0 && FSh_pnLightMapInputRegisters[1] && FSh_pnLightMapInputRegisters[1]<0xffffffff )
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
			m_nLM = 0;
			m_nLMMode = LM_NONE;
		}
	}
	else
	{
		m_nLM = 0;
		m_nLMMode = LM_NONE;
	}
	
	if ( !(FSh_shaderFlags&FSh_RENDER_MULTIPASS_LIGHTING) )
	{
		return 1;
	}
		
	return nPasses;
}

u32 fsh_GetNumSpecularPasses(u32 shaderID, u32 specularID)
{
	u32 nPasses = 0;
	
	if (FSh_shaderType == SHADERTYPE_SPECULAR)// && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP] > 0 && FSh_pnSurfaceInputRegisters[FSHADERS_LIGHT_REG_SMOTIF])
	{
		nPasses = 1;
	}

	return nPasses;
}

u32 fsh_GetCurrentNumPasses(BOOL *pbFastPass)
{
	u32 nRet = 1;
	m_nLMMode = LM_NONE; m_nLM = 0;
	
	if (pbFastPass) { *pbFastPass = FALSE; }
	_bFastShader = FALSE;
	
	switch (FSh_shaderType)
	{
		case SHADERTYPE_DIFFUSE:
			nRet = fsh_GetNumDiffusePasses(0, 0, pbFastPass);
		break;
		case SHADERTYPE_SURFACE:
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
		
			nRet = 1;
		break;
		case SHADERTYPE_SPECULAR:
			nRet = fsh_GetNumSpecularPasses(0, 0);
		break;
	};
	return (nRet);
}

void fgcsh_SetCurrentPosMtx( u32 nMatrixIdx )
{
	_nPosMatrix = nMatrixIdx;
}

void fgcsh_SetCurrentNrmlMtxIdx( u32 nMatrixIdx )
{
	_nNrmlMatrix = nMatrixIdx;
}

void fsh_SetPassIdx(u32 nPass)
{
	_nPassIdx = nPass;
}

BOOL fsh_SetSurfaceShader(u32 shaderID)
{
	_nSurfaceShaderID = shaderID;
	return TRUE;
}

BOOL fsh_SetShader(u32 shaderID, u32 diffuseID, u32 specID, u32 passIdx)
{
	_bUseTrans=_UseTrans(shaderID);
	u32 nshader;
	
	_nSurfaceShaderID = shaderID;
	_nPassIdx = passIdx;
	
	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	if (!_CheckRenderMode(FSh_shaderType)) return FALSE;
	#endif

	if (FSh_shaderType == SHADERTYPE_DIFFUSE)
	{
		if (_bUseTrans) return (FALSE); //no diffuse or specular passes for translucent shaders.

		nshader = diffuseID;
		_nDiffuseShaderID = diffuseID;
		_pnShaderRemap = _anDiffuseRemap;
	}
	else if (FSh_shaderType == SHADERTYPE_SURFACE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY )
	{
		nshader = shaderID;
		_pnShaderRemap = _anSurfaceRemap;
		if (_bUseTrans)
		{
			FSh_shaderType = SHADERTYPE_TRANSLUCENCY;
		}
		
		if (FShaders_aShaderRegs[shaderID].nSurfaceTypeFlags & FSHADERS_SURFACE_FLAG_DETAIL_MAP)
		{
			_bSetDetailScale = TRUE;
			_nDetailReg=0;
			while ( FShaders_aShaderRegs[shaderID].anRegType[_nDetailReg] != FSHADERS_REG_DETAILMAP_TILE_FACTOR )
			{
				_nDetailReg++;
			}	
		}
	}
	else if (FSh_shaderType == SHADERTYPE_SPECULAR)
	{
		if (_bUseTrans) return (FALSE);

		_pnShaderRemap = _anSpecularRemap;
		nshader = specID;
	}


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

void fsh_EndExecute()
{
	if (_bIndLookup)
	{
		_bIndLookup = FALSE;
		GXSetNumIndStages(0);
		GXSetTevDirect(GX_TEVSTAGE0);
		GXSetTevDirect(GX_TEVSTAGE1);
		GXSetTevDirect(GX_TEVSTAGE2);
		GXSetTevDirect(GX_TEVSTAGE3);
		
		GXSetTevDirect(GX_TEVSTAGE4);
		GXSetTevDirect(GX_TEVSTAGE5);
		GXSetTevDirect(GX_TEVSTAGE6);
		GXSetTevDirect(GX_TEVSTAGE7);
	}
	
	if (_bFogEnabled)
	{
		GXSetAlphaUpdate(FALSE);
	}
}

FINLINE GXColor _GXCOLOR_RGB(float r, float g, float b, float a)
{
	GXColor ret;
	ret.r = r*255.0f; ret.g = g*255.0f; ret.b = b*255.0f; ret.a = a*255.0f;
	return (ret);
}

FINLINE GXColorS10 _GXCOLORS10_RGB(float r, float g, float b, float a)
{
	GXColorS10 ret;
	ret.r = r*255.0f; ret.g = g*255.0f; ret.b = b*255.0f; ret.a = a*255.0f;
	return (ret);
}

void fsh_RenderMode(FRenderMode_e renderMode)
{
	_nRenderMode = renderMode;
}

static void _SetRenderStates( void )
{
	FASSERT( FSh_pnSurfaceInputRegisters );
	
	CFColorRGBA pDestColor, pDestColor2;
	u32 i;
	
	if (_nPassIdx == 0 && FSh_shaderType == SHADERTYPE_DIFFUSE && _bFogEnabled)
	{
		GXSetAlphaUpdate(TRUE);
	}
	else
	{
		GXSetAlphaUpdate(FALSE);
	}
	
	if ( (_nPassIdx > 0 || FSh_shaderType == SHADERTYPE_SPECULAR) )
	{
		fgc_SetBlendOp(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);		
		fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
		
		fgc_SetZMode( TRUE, GX_EQUAL, FALSE );

		return;
	}
	
	if (FSh_bShadowRender)
	{
		fgc_SetBlendOp(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);			
		fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
		
		fgc_SetZMode( TRUE, GX_ALWAYS, FALSE );
	}
	else
	{
		if (FSh_shaderType != SHADERTYPE_DIFFUSE && FSh_shaderType != SHADERTYPE_TRANSLUCENCY)
		{
			m_nLMMode = LM_NONE;
			m_nLM = 0;
		}
		
		if (!(_nShaderID >= FSHADERS_DECAL_AI && _nShaderID <= FSHADERS_INTENSITY))
		{
			if ( FRS_bRenderFlags&FRS_RENDER_PASS_LIGHTING )
			{
				if (_nPassIdx == 0 && (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY))
				{
					fgc_SetZMode( TRUE, GX_LEQUAL, !!(FSh_ZEnable||FSh_shaderType == SHADERTYPE_DIFFUSE) );
				}
				else
				{
					fgc_SetZMode( TRUE, GX_EQUAL, FALSE );
				}
			}
			else
			{
				fgc_SetZMode( TRUE, GX_LEQUAL, TRUE );
			}
		}

		if ( !(_nShaderID >= FSHADERS_DECAL_AI && _nShaderID <= FSHADERS_INTENSITY) )
		{
			if ( FRS_bRenderFlags&FRS_RENDER_PASS_LIGHTING )
			{
				if (_nRenderMode == FRMODE_NORMAL)
				{
					fgc_SetBlendOp( (GXBlendMode)_aShaderRenderStates[_nShaderID].bBlendMode, (GXBlendFactor)_aShaderRenderStates[_nShaderID].bSourceF,
						 	   	   (GXBlendFactor)_aShaderRenderStates[_nShaderID].bDestF, (GXLogicOp)_aShaderRenderStates[_nShaderID].bLogicOp);
				}
				else
				{
					fgc_SetBlendOp(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);		
				}
			}
			else
			{
				fgc_SetBlendOp(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);
			}
		}
		
		if (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY)
		{
			if (m_nLMMode > LM_NONE && FSh_shaderType == SHADERTYPE_DIFFUSE)
			{
				pDestColor = _GReg_Ambient;
				if (FSh_pnLightInputRegisters && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF] != NULL)
				{
					((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])->ComputeColor( &pDestColor2 );
					pDestColor += pDestColor2;
					pDestColor.Clamp1();
				}			
				GXSetTevKColor(GX_KCOLOR0, _GXCOLOR_RGB(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha));
				
				GXSetChanMatColor(GX_COLOR0, FGC_gxWhite);
			}
			else if (_nShaderID == FSHADERS_INTENSITY)
			{
				GXSetTevKColor(GX_KCOLOR0, _GXCOLOR_RGB(0.3f, 0.59f, 0.11f, 0.0f));
			}
			else
			{
				if (_aShaderRenderStates[_nShaderID].nFactorReg < 0xff)
				{
					if (FSh_pnSurfaceInputRegisters[ _aShaderRenderStates[_nShaderID].nFactorReg ] != NULL)
					{
						((const CFColorMotif *)FSh_pnSurfaceInputRegisters[ _aShaderRenderStates[_nShaderID].nFactorReg ])->ComputeColor( &pDestColor );
						GXSetTevKColor(GX_KCOLOR2, _GXCOLOR_RGB(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha));
					}
					else
					{
						GXSetTevKColor(GX_KCOLOR2, _GXCOLOR_RGB(1,1,1,1));
					}
				}
										
				if (FSh_pnLightInputRegisters && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_DMOTIF] != NULL)
				{
					((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_DMOTIF])->ComputeColor( &pDestColor );
					GXSetChanMatColor(GX_COLOR0, _GXCOLOR_RGB(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha));
				}
					
				pDestColor = _GReg_Ambient;
				if (FSh_pnLightInputRegisters && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF] != NULL)
				{
					((const CFColorMotif *)FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMOTIF])->ComputeColor( &pDestColor2 );
					pDestColor += pDestColor2;
					pDestColor.Clamp1();
				}
				//Load standard 8bit version for version where Registers get trampeled.
				_gxKonst[1] = _GXCOLOR_RGB(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha);
				GXSetTevKColor(GX_KCOLOR1, _gxKonst[1]);
				//Load 10bit signed value, allowing for negative emissiveness in some shaders.
				GXColorS10 gxClrS10;
				gxClrS10 = _GXCOLORS10_RGB(pDestColor.fRed, pDestColor.fGreen, pDestColor.fBlue, pDestColor.fAlpha);
				GXSetTevColorS10(GX_TEVREG0, gxClrS10);
			}
		}
		
		if ( !(_nShaderID >= FSHADERS_DECAL_AI && _nShaderID <= FSHADERS_INTENSITY) )
		{
			if (_nPassIdx == 0 && (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY))
			{
				if (_aShaderRenderStates[_nShaderID].bAlphaTest || FSh_bUseClipPlane)
				{
					if ( fgcsh_bColorKey )
					{	
						fgc_SetAlphaCompare(GX_GREATER, 0x20, GX_AOP_OR, GX_EQUAL, 0xff);
					}	
					else
					{
						fgc_SetAlphaCompare(GX_GEQUAL, 0x80, GX_AOP_OR, GX_EQUAL, 0xff);
					}
				}
				else
				{
					fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
				}
			}
			else
			{
				fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
			}
		}
		
		if (_aShaderRenderStates[_nShaderID].IndLookupType != FSH_IND_INVALID)
		{
			FShaderIndirectLookup_t *pLookUp = &_aIndLookupTypes[ _aShaderRenderStates[_nShaderID].IndLookupType ];
			GXSetNumIndStages(pLookUp->nNumIndReg);
			
			for (i=0; i<pLookUp->nNumIndStages; i++)
			{
				FShaderIndirectStage_t *pStage = &pLookUp->aIndStages[i];
				GXSetTevIndirect(pStage->TevStage, pStage->IndStage, pStage->Format, pStage->BiasSel, pStage->MatrixSel,
								 pStage->WrapS, pStage->WrapT, pStage->bAddPrev, pStage->bUTC_Lod, pStage->AlphaSel);
			}
			if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP] && FSh_shaderFlags&FSh_RENDER_BUMP)
			{
				FShTexInst_t *pBump = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]);
				FTexDef_t *pTexDefLayer = pBump->TexInst.GetTexDef();
				if (pTexDefLayer)
				{
					GXTexObj *pGCTexObjLayer = (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj;
					_SetTexture(pGCTexObjLayer, GX_TEXMAP1);
				}
		 		GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
		    	GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

			    GXSetIndTexMtx(GX_ITM_0, _indMtx_Warp, pLookUp->nScaleExp);
		    }
			
			_bIndLookup = TRUE;
		}
		else
		{
			_bIndLookup = FALSE;
		}
		
		// DFS -- I don't know if this is masking a deeper problem or not, but the
		// intensity shader uses KCOLOR0 for its weights, and we don't want to trash it.
		if ((_nShaderID != FSHADERS_INTENSITY) &&
			(FSh_shaderType == SHADERTYPE_SURFACE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY))
		{
			GXSetTevKColor(GX_KCOLOR0, _gxSurfaceColor);
			GXSetTevKColor(GX_KCOLOR3, _gxLayerAlpha);
		}
	}
}

FINLINE BOOL _NeedSwapChange(int stage, u8 swap_color, u8 swap_alpha)//GXTevSwapSel swap_color, GXTevSwapSel swap_alpha)
{
	BOOL ret = false;
	if (_nSwapClr[stage] != swap_color || _nSwapAlpha[stage] != swap_alpha)
	{
		ret = true;
	}	
	_nSwapClr[stage] = swap_color; _nSwapAlpha[stage] = swap_alpha;
	return (ret);
}

void _ApplyInvView(Mtx44Ptr pMtx)
{
	MTXConcat(pMtx, FXfm_GCInvView, pMtx);
}

FINLINE void _GenerateLightXZMat(PerPixelPointLight_t *pLight, MtxPtr pMtx)
{
	pMtx[0][0] = 0.5f*pLight->fOOR_WS; pMtx[0][1] = 0; pMtx[0][2] = 0; pMtx[0][3] = -pLight->vPos.x*pLight->fOOR_WS*0.5f + 0.5f;
	pMtx[1][0] = 0; pMtx[1][1] = 0; pMtx[1][2] = 0.5f*pLight->fOOR_WS; pMtx[1][3] = -pLight->vPos.z*pLight->fOOR_WS*0.5f + 0.5f;
	pMtx[2][0] = pMtx[2][1] = pMtx[2][2] = 0; pMtx[2][3] = 1.0f;
}

FINLINE void _GenerateLightYMat(PerPixelPointLight_t *pLight, MtxPtr pMtx)
{
	pMtx[0][0] = 0; pMtx[0][1] = 0.5f*pLight->fOOR_WS; pMtx[0][2] = 0; pMtx[0][3] = -pLight->vPos.y*pLight->fOOR_WS*0.5f + 0.5f;
	pMtx[1][0] = 0; pMtx[1][1] = 0; pMtx[1][2] = 0; pMtx[1][3] = 0.0f;
	pMtx[2][0] = pMtx[2][1] = pMtx[2][2] = 0; pMtx[2][3] = 1.0f;
}

FINLINE void _GenerateTexKillMat(CFVec3& N, CFVec3A& P, MtxPtr pMtx)
{
	pMtx[0][0] = N.x; pMtx[0][1] = N.y; pMtx[0][2] = N.z; pMtx[0][3] = -P.x*N.x - P.y*N.y - P.z*N.z;
	pMtx[1][0] = 0; pMtx[1][1] = 0; pMtx[1][2] = 0; pMtx[1][3] = 0.0f;
	pMtx[2][0] = pMtx[2][1] = pMtx[2][2] = 0; pMtx[2][3] = 1.0f;
}

void _LightLookAt(MtxPtr pLookAt, CFVec3A& camPos, CFVec3A& up, CFVec3A& target)
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

    pLookAt[0][0] = vRight.x;
    pLookAt[0][1] = vRight.y;
    pLookAt[0][2] = vRight.z;
    pLookAt[0][3] = -( camPos.x * vRight.x + camPos.y * vRight.y + camPos.z * vRight.z );

    pLookAt[1][0] = vUp.x;
    pLookAt[1][1] = vUp.y;
    pLookAt[1][2] = vUp.z;
    pLookAt[1][3] = -( camPos.x * vUp.x + camPos.y * vUp.y + camPos.z * vUp.z );

    pLookAt[2][0] = vLook.x;
    pLookAt[2][1] = vLook.y;
    pLookAt[2][2] = vLook.z;
    pLookAt[2][3] = -( camPos.x * vLook.x + camPos.y * vLook.y + camPos.z * vLook.z );
}

void _LightPerspective(MtxPtr m, float fov, float fDist)
{
    f32 angle, cot;

    // find the cotangent of half the (YZ) field of view
    angle = fov * 0.5f;
    
    f32 fTanAng = tanf(angle);
    f32 fNewTanAng;
    
    if (fDist)
    {
    	fNewTanAng = fTanAng / (1.0f + fDist);
    }
    else
    {
	    fNewTanAng = fTanAng;
    }
        
    cot = 0.5f / fNewTanAng;//tanf(angle);

    m[0][0] =    cot;
    m[0][1] =    0.0f;
    m[0][2] =    -0.5f;
    m[0][3] =    0.0f;

    m[1][0] =    0.0f;
    m[1][1] =    cot;
    m[1][2] =    -0.5f;
    m[1][3] =    0.0f;

    m[2][0] =    0.0f;
    m[2][1] =    0.0f;
    m[2][2] =   -1.0f;
    m[2][3] =    0.0f;
}

FINLINE void _GenerateSpotMat(PerPixelSpotLight_t *pLight, Mtx44Ptr pMtx, float fDist=0.0f);

FINLINE void _GenerateSpotMat(PerPixelSpotLight_t *pLight, Mtx44Ptr pMtx, float fDist)
{
	Mtx44 lookAt, proj;
	CFVec3A up;
	CFVec3A target;
	
    lookAt[0][0] = pLight->pMtx_WS->m_vRight.x;
    lookAt[0][1] = pLight->pMtx_WS->m_vRight.y;
    lookAt[0][2] = pLight->pMtx_WS->m_vRight.z;
    lookAt[0][3] = -( pLight->vPos.x * pLight->pMtx_WS->m_vRight.x + pLight->vPos.y * pLight->pMtx_WS->m_vRight.y + pLight->vPos.z * pLight->pMtx_WS->m_vRight.z );

    lookAt[1][0] = pLight->pMtx_WS->m_vUp.x;
    lookAt[1][1] = pLight->pMtx_WS->m_vUp.y;
    lookAt[1][2] = pLight->pMtx_WS->m_vUp.z;
    lookAt[1][3] = -( pLight->vPos.x * pLight->pMtx_WS->m_vUp.x + pLight->vPos.y * pLight->pMtx_WS->m_vUp.y + pLight->vPos.z * pLight->pMtx_WS->m_vUp.z );

    lookAt[2][0] = pLight->pMtx_WS->m_vFront.x;
    lookAt[2][1] = pLight->pMtx_WS->m_vFront.y;
    lookAt[2][2] = pLight->pMtx_WS->m_vFront.z;
    lookAt[2][3] = -( pLight->vPos.x * pLight->pMtx_WS->m_vFront.x + pLight->vPos.y * pLight->pMtx_WS->m_vFront.y + pLight->vPos.z * pLight->pMtx_WS->m_vFront.z );
    
    lookAt[3][0] = 0.0f;
    lookAt[3][1] = 0.0f;
    lookAt[3][2] = 0.0f;
    lookAt[3][3] = 1.0f;

	_LightPerspective(proj, pLight->fFOV, fDist);
		
	MTXConcat(proj, lookAt, pMtx);
	
	pMtx[3][0] = 0;
	pMtx[3][1] = 0;
	pMtx[3][2] = 0;
	pMtx[3][3] = 1;
	
	if (!fgcsh_bWorldGeo)
	{
		_ApplyInvView(pMtx);
	}
}

FINLINE void _GenerateDirMtx(CFVec3A& vDir, MtxPtr pMtx)
{
	pMtx[0][0] = -vDir.x*0.5f; pMtx[0][1] = 0; pMtx[0][2] = 0; pMtx[0][3] = 0.5f;
	pMtx[1][0] = 0; pMtx[1][1] = -vDir.y*0.5f; pMtx[1][2] = 0; pMtx[1][3] = 0.5f;	
	pMtx[2][0] = 0; pMtx[2][1] = 0; pMtx[2][2] = 0; pMtx[2][3] = 0.0f;	
}

static void _HandleLightingPass( void )
{
	PerPixelPointLight_t *pLight;
	PerPixelSpotLight_t *pSpotLight;
	s32 n, idx, i, s, s3;
	
	BOOL bHasBump=FALSE;
	
	if ( FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]&&fgcsh_bWorldGeo&& FSh_shaderFlags&FSh_RENDER_BUMP )
	{
		FShTexInst_t *pBump = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]);
		if (pBump)
		{
			FTexDef_t *pTexDefLayer = pBump->TexInst.GetTexDef();
			if (pTexDefLayer)
			{
				bHasBump = TRUE;
			}
		}
	}
	
	bHasBump = FALSE;
	
	if (_nPassIdx == 1)
	{
		n = (_nPerPixelPoint < 4)?(_nPerPixelPoint):(4);
		idx = 0;
	}
	else if (_nPassIdx == 2 && _nPerPixelPoint > 4)
	{
		n = _nPerPixelPoint - 4;
		idx = 4;
	}
	else
	{
		n = 0;
	}

	GXColor white={0xff, 0xff, 0xff, 0xff};
	Mtx xzMtx, yMtx;
	Mtx texKillMtx;
	Mtx44 spotMtx;

	if (n)
	{
		u8 nDiffExp=2;
		
		if (!bHasBump)
		{
			_SetTexture( &_gxAttenMap, GX_TEXMAP0 );
			_SetTexture( &_gxAttenMapY, GX_TEXMAP1 );
			u32 t=0;
			s32 t0_3=0, t1_3;
			for (i=0, s=0; i<n; i++, s+=(2+nDiffExp), t+=2, t0_3+=6)
			{
				pLight = &_aPerPixelPointLights[i+idx];
				GXSetTevKColor((GXTevKColorID)(GX_KCOLOR0+i), pLight->GCColor);
				//First stage		
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s), (GXTexCoordID)(GX_TEXCOORD0+t), GX_TEXMAP0, GX_COLOR_NULL );
				
				t1_3 = t0_3+3;
				
				//texgens.
				_GenerateLightXZMat(pLight, xzMtx);
				_GenerateLightYMat(pLight, yMtx);
				if (fgcsh_bWorldGeo)
				{
					GXLoadTexMtxImm(xzMtx, (GXTexMtx)(GX_TEXMTX0+t0_3), GX_MTX2x4);
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+t), GX_TG_MTX2x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX0+t0_3));
					
					GXLoadTexMtxImm(yMtx, (GXTexMtx)(GX_TEXMTX0+t1_3), GX_MTX2x4);
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+t+1), GX_TG_MTX2x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX0+t1_3));
				}
				else
				{
					_ApplyInvView(xzMtx);
					GXLoadTexMtxImm(xzMtx, (GXTexMtx)(GX_PTTEXMTX0+t0_3), GX_MTX3x4);
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+t), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, (GXTexMtx)(GX_PTTEXMTX0+t0_3));
					
					_ApplyInvView(yMtx);
					GXLoadTexMtxImm(yMtx, (GXTexMtx)(GX_PTTEXMTX0+t1_3), GX_MTX3x4);
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+t+1), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, (GXTexMtx)(GX_PTTEXMTX0+t1_3));	
				}
				//
			
				GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
				GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				//Second stage	
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s+1), (GXTexCoordID)(GX_TEXCOORD0+t+1), GX_TEXMAP1, GX_COLOR_NULL );

				GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s+1), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
				GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_CC_KONST, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0);
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
				GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_CC_ZERO, GX_CC_C0, GX_CC_C0, GX_CC_ZERO);
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
				if (i==0)
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_CC_ZERO, GX_CC_C0, GX_CC_C0, GX_CC_ZERO);
				}
				else
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_CC_ZERO, GX_CC_C0, GX_CC_C0, GX_CC_CPREV);
				}
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			}
			
			GXSetNumTevStages(n*(2+nDiffExp)); //4*4 = 16.
			GXSetNumTexGens(n*2);	//4*2 = 8. 
		}
		else
		{
			u8 nTev, nInd, nTexGen, nTexMtx, nTex, nDummy8;
			_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE);
			
			_SetTexture( &_gxAttenMap, (GXTexMapID)(GX_TEXMAP0+nTex) );
			_SetTexture( &_gxAttenMapY, (GXTexMapID)(GX_TEXMAP1+nTex) );
			u32 t=0;
			s32 t0_3=0, t1_3;
			
			if (n > 3) n = 3;
			
			t0_3 = nTexMtx*3;
			
			for (i=0, s=0; i<n; i++, s+=(2+nDiffExp), t+=2, t0_3+=6)
			{
				pLight = &_aPerPixelPointLights[i+idx];
				GXSetTevKColor((GXTevKColorID)(GX_KCOLOR0+i), pLight->GCColor);
				//First stage		
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s), (GXTexCoordID)(GX_TEXCOORD0+t+nTexGen), (GXTexMapID)(GX_TEXMAP0+nTex), GX_COLOR_NULL );
				
				t1_3 = t0_3+3;
				
				//texgens.
				_GenerateLightXZMat(pLight, xzMtx);
				_GenerateLightYMat(pLight, yMtx);
				if (fgcsh_bWorldGeo)
				{
					GXLoadTexMtxImm(xzMtx, (GXTexMtx)(GX_TEXMTX0+t0_3), GX_MTX2x4);
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+t+nTexGen), GX_TG_MTX2x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX0+t0_3));
					
					GXLoadTexMtxImm(yMtx, (GXTexMtx)(GX_TEXMTX0+t1_3), GX_MTX2x4);
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+t+1+nTexGen), GX_TG_MTX2x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX0+t1_3));
				}
				else
				{
					_ApplyInvView(xzMtx);
					GXLoadTexMtxImm(xzMtx, (GXTexMtx)(GX_PTTEXMTX0+t0_3), GX_MTX3x4);
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+t+nTexGen), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, (GXTexMtx)(GX_PTTEXMTX0+t0_3));
					
					_ApplyInvView(yMtx);
					GXLoadTexMtxImm(yMtx, (GXTexMtx)(GX_PTTEXMTX0+t1_3), GX_MTX3x4);
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+t+1+nTexGen), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, (GXTexMtx)(GX_PTTEXMTX0+t1_3));
				}
				
				if (i > 0)
				{
					_IndirectWarp(s, 0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE);
				}
				_IndirectWarp(s+1, 0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE);
				//
			
				GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
				GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				//Second stage	
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s+1), (GXTexCoordID)(GX_TEXCOORD0+t+1+nTexGen), (GXTexMapID)(GX_TEXMAP1+nTex), GX_COLOR_NULL );

				GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s+1), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
				GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_CC_KONST, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0);
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s+1), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
				GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_CC_ZERO, GX_CC_C0, GX_CC_C0, GX_CC_ZERO);
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s+2), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				
				GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
				if (i==0)
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_CC_ZERO, GX_CC_C0, GX_CC_C0, GX_CC_ZERO);
				}
				else
				{
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_CC_ZERO, GX_CC_C0, GX_CC_C0, GX_CC_CPREV);
				}
				GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
				GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s+3), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			}
			
			GXSetNumTevStages(n*(2+nDiffExp)); //4*4 = 16.
			GXSetNumTexGens(n*2+1);	//4*2 = 8. 
			GXSetNumIndStages(1);	
		
		}
	}
	else if (_nPerPixelSpot)
	{
		n = _nPerPixelSpot;
		FMATH_CLAMP( n, 0, 3 );
		
		fgc_SetChannelsForPerPixelSpot(n);
		
		u8 nTev, nInd, nTexGen, nTexMtx, nTex;
		u8 s2=0;
		u8 nDummy8;
		GXLightObj gxLight;
		
		CFVec3 vN;
		
		if (fgcsh_bWorldGeo)
		{
			if (!bHasBump)
			{
				_SetTexture(&_gxTexKill2, GX_TEXMAP0);
				
				for (i=0, s=0, s2=0; i<n; i++, s+=3, s2+=2)
				{
					pSpotLight = &_aPerPixelSpotLights[i];
					
					//Setup light for spotlight backface culling
					GXInitLightAttn( &gxLight, 1.f, 0.f, 0.f, 0.001f, 0, 0 );
 					
 					GXInitLightPos( &gxLight, pSpotLight->vPos_VS.x, pSpotLight->vPos_VS.y, pSpotLight->vPos_VS.z );
			 		
			 		GXInitLightColor( &gxLight, FGC_gxWhite );
			 		GXLoadLightObjImm( &gxLight, (GXLightID)(1<<i) );
			 		//
					
					_SetTexture(pSpotLight->pGXTex, (GXTexMapID)(GX_TEXMAP1+i));
					
					vN = pSpotLight->pMtx_WS->m_vFront;
					
					CFVec3A vPos;
					vPos.v3 = pSpotLight->vPos.v3;
					
					f32 fProjOffs = fgcsh_fProjOffs;
					pSpotLight->vPos.v3 = vPos.v3 - (vN*fProjOffs);
					
					vN.x *= pSpotLight->fRange;
					vN.y *= pSpotLight->fRange;
					vN.z *= pSpotLight->fRange;
										
					f32 fAdjDist = pSpotLight->fRange*fgcsh_fProjRngAdj;
					if (fAdjDist > 1.0f) { fAdjDist = 1.0f; }
					fAdjDist *= fProjOffs;
					
					_GenerateSpotMat(pSpotLight, spotMtx, fAdjDist);	//projection matrix for spotlight.
					_GenerateTexKillMat(vN, vPos, texKillMtx); 			//clip to light plane + distance attenuation.
					
					s3 = i*6;
					
					GXLoadTexMtxImm(spotMtx, (GXTexMtx)(GX_TEXMTX0+s3), GX_MTX3x4);
					GXLoadTexMtxImm(texKillMtx, (GXTexMtx)(GX_TEXMTX1+s3), GX_MTX2x4);
					
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+s2), GX_TG_MTX3x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX0+s3));
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+s2), GX_TG_MTX2x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX1+s3));
					
					GXSetTevKColor((GXTevKColorID)(GX_KCOLOR0+i), pSpotLight->GCColor);
					
					//First stage
					//replace i with s.
					GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s), (GXTexCoordID)(GX_TEXCOORD0+s2), (GXTexMapID)(GX_TEXMAP1+i), GX_COLOR_NULL );
					
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

					//Second stage, backface culling					
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, (i<2)?GX_COLOR0A0:GX_COLOR1A1 );
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CC_ZERO, (i==0||i==2)?GX_CC_RASC:GX_CC_RASA, GX_CC_C0, GX_CC_ZERO);
					
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					
					//Third stage	
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE2+s), (GXTexCoordID)(GX_TEXCOORD0+s2+1), GX_TEXMAP0, GX_COLOR_NULL );

					if (i==0)
					{			
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
					}
					else
					{
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_CPREV);
					}
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				}
				
				GXSetNumTevStages(n*3);
				GXSetNumTexGens(n*2);
			}
			else
			{
				_IndirectWarp(0, 0.10f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE);
				_SetTexture(&_gxTexKill2, (GXTexMapID)(GX_TEXMAP0 + nTex));
				
				s3 = nTexMtx*3;
				
				for (i=0, s=0; i<n; i++, s+=2, s3+=6)
				{
					if (i > 0)
					{
						_IndirectWarp(s, 0.10f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE);
					}
				
					pSpotLight = &_aPerPixelSpotLights[i];
					
					//Setup light for spotlight backface culling
					GXInitLightAttn( &gxLight, 1.f, 0.f, 0.f, 0.001f, 0, 0 );
 					
 					GXInitLightPos( &gxLight, pSpotLight->vPos_VS.x, pSpotLight->vPos_VS.y, pSpotLight->vPos_VS.z );
			 		
			 		GXInitLightColor( &gxLight, FGC_gxWhite );
			 		GXLoadLightObjImm( &gxLight, (GXLightID)(1<<i) );
			 		//
					
					_SetTexture(pSpotLight->pGXTex, (GXTexMapID)(GX_TEXMAP1+nTex+i));
					
					vN = pSpotLight->pMtx_WS->m_vFront;
					
					CFVec3A vPos;
					vPos.v3 = pSpotLight->vPos.v3;
					
					f32 fProjOffs = fgcsh_fProjOffs;
					pSpotLight->vPos.v3 = vPos.v3 - (vN*fProjOffs);
					
					vN.x *= pSpotLight->fRange;
					vN.y *= pSpotLight->fRange;
					vN.z *= pSpotLight->fRange;
										
					f32 fAdjDist = pSpotLight->fRange*fgcsh_fProjRngAdj;
					if (fAdjDist > 1.0f) { fAdjDist = 1.0f; }
					fAdjDist *= fProjOffs;
					
					_GenerateSpotMat(pSpotLight, spotMtx, fAdjDist);	//projection matrix for spotlight.
					_GenerateTexKillMat(vN, vPos, texKillMtx); 			//clip to light plane + distance attenuation.
					
					GXLoadTexMtxImm(spotMtx, (GXTexMtx)(GX_TEXMTX0+s3), GX_MTX3x4);
					GXLoadTexMtxImm(texKillMtx, (GXTexMtx)(GX_TEXMTX1+s3), GX_MTX2x4);
					
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+s2+nTexGen), GX_TG_MTX3x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX0+s3));
					GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+s2+nTexGen), GX_TG_MTX2x4, GX_TG_POS, (GXTexMtx)(GX_TEXMTX1+s3));
					
					GXSetTevKColor((GXTevKColorID)(GX_KCOLOR0+i), pSpotLight->GCColor);
					
					//First stage
					//replace i with s.
					GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s), (GXTexCoordID)(GX_TEXCOORD0+s2+nTexGen), (GXTexMapID)(GX_TEXMAP1+nTex+i), GX_COLOR_NULL );
					
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

					//Second stage, backface culling					
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, (i<2)?GX_COLOR0A0:GX_COLOR1A1 );
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CC_ZERO, (i==0||i==2)?GX_CC_RASC:GX_CC_RASA, GX_CC_C0, GX_CC_ZERO);
					
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					
					//Third stage	
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE2+s), (GXTexCoordID)(GX_TEXCOORD0+s2+nTexGen+1), GX_TEXMAP0, GX_COLOR_NULL );

					if (i==0)
					{			
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
					}
					else
					{
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_CPREV);
					}
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				}
				
				GXSetNumTevStages(n*3);
				GXSetNumTexGens(n*2+1);
				GXSetNumIndStages(1);
			}
		}
		else
		{
			if (!bHasBump)
			{
				_SetTexture(&_gxTexKill2, GX_TEXMAP0);
				
				for (i=0, s=0, s2=0; i<n; i++, s+=3, s2+=2)
				{
					pSpotLight = &_aPerPixelSpotLights[i];
					
					//Setup light for spotlight backface culling
					GXInitLightAttn( &gxLight, 1.f, 0.f, 0.f, 1.0f, 0, 0 );
 					
 					GXInitLightPos( &gxLight, pSpotLight->vPos_VS.x, pSpotLight->vPos_VS.y, pSpotLight->vPos_VS.z );
			 		
			 		GXInitLightColor( &gxLight, FGC_gxWhite );
			 		GXLoadLightObjImm( &gxLight, (GXLightID)(1<<i) );
			 		//
					
					_SetTexture(pSpotLight->pGXTex, (GXTexMapID)(GX_TEXMAP1+i));
					
					vN = pSpotLight->pMtx_WS->m_vFront;
					
					CFVec3A vPos;
					vPos.v3 = pSpotLight->vPos.v3;
					
					f32 fProjOffs = fgcsh_fProjOffs;
					pSpotLight->vPos.v3 = vPos.v3 - (vN*fProjOffs);
					
					vN.x *= pSpotLight->fRange;
					vN.y *= pSpotLight->fRange;
					vN.z *= pSpotLight->fRange;
										
					f32 fAdjDist = pSpotLight->fRange*fgcsh_fProjRngAdj;
					if (fAdjDist > 1.0f) { fAdjDist = 1.0f; }
					fAdjDist *= fProjOffs;
					
					_GenerateSpotMat(pSpotLight, spotMtx, fAdjDist);	//projection matrix for spotlight.
					_GenerateTexKillMat(vN, vPos, texKillMtx); 			//clip to light plane + distance attenuation.
					
					_ApplyInvView(texKillMtx);
					
					s3 = i*6;
					
					GXLoadTexMtxImm(spotMtx, (GXTexMtx)(GX_PTTEXMTX0+s3), GX_MTX3x4);
					GXLoadTexMtxImm(texKillMtx, (GXTexMtx)(GX_PTTEXMTX1+s3), GX_MTX3x4);
					
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+s2), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, GX_PTTEXMTX0+s3);
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD1+s2), GX_TG_MTX2x4, GX_TG_POS, _nPosMatrix, FALSE, GX_PTTEXMTX1+s3);
					
					GXSetTevKColor((GXTevKColorID)(GX_KCOLOR0+i), pSpotLight->GCColor);
					
					//First stage
					//replace i with s.
					GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s), (GXTexCoordID)(GX_TEXCOORD0+s2), (GXTexMapID)(GX_TEXMAP1+i), GX_COLOR_NULL );
					
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

					//Second stage, backface culling					
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, (i<2)?GX_COLOR0A0:GX_COLOR1A1 );
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CC_ZERO, (i==0||i==2)?GX_CC_RASC:GX_CC_RASA, GX_CC_C0, GX_CC_ZERO);
					
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					
					//Third stage	
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE2+s), (GXTexCoordID)(GX_TEXCOORD0+s2+1), GX_TEXMAP0, GX_COLOR_NULL );

					if (i==0)
					{			
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
					}
					else
					{
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_CPREV);
					}
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				}
				GXSetNumTevStages(n*3);
				GXSetNumTexGens(n*2);
			}
			else
			{
				_IndirectWarp(0, 0.10f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE);
				_SetTexture(&_gxTexKill2, (GXTexMapID)(GX_TEXMAP0 + nTex));
				
				s3 = nTexMtx*3;
				
				for (i=0, s=0; i<n; i++, s+=2)
				{
					if (i > 0)
					{
						_IndirectWarp(s, 0.10f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE);
					}
				
					pSpotLight = &_aPerPixelSpotLights[i];
					
					//Setup light for spotlight backface culling
					GXInitLightAttn( &gxLight, 1.f, 0.f, 0.f, 1.0f, 0, 0 );
					
					GXInitLightPos( &gxLight, pSpotLight->vPos_VS.x, pSpotLight->vPos_VS.y, pSpotLight->vPos_VS.z );
			 		
			 		GXInitLightColor( &gxLight, FGC_gxWhite );
			 		GXLoadLightObjImm( &gxLight, (GXLightID)(1<<i) );
			 		//
					
					_SetTexture(pSpotLight->pGXTex, (GXTexMapID)(GX_TEXMAP1+nTex+i));
					
					vN = pSpotLight->pMtx_WS->m_vFront;
					
					CFVec3A vPos;
					vPos.v3 = pSpotLight->vPos.v3;
					
					f32 fProjOffs = fgcsh_fProjOffs;
					pSpotLight->vPos.v3 = vPos.v3 - (vN*fProjOffs);
					
					vN.x *= pSpotLight->fRange;
					vN.y *= pSpotLight->fRange;
					vN.z *= pSpotLight->fRange;
										
					f32 fAdjDist = pSpotLight->fRange*fgcsh_fProjRngAdj;
					if (fAdjDist > 1.0f) { fAdjDist = 1.0f; }
					fAdjDist *= fProjOffs;
					
					_GenerateSpotMat(pSpotLight, spotMtx, fAdjDist);	//projection matrix for spotlight.
					_GenerateTexKillMat(vN, vPos, texKillMtx); 			//clip to light plane + distance attenuation.
					
					_ApplyInvView(texKillMtx);
					
					GXLoadTexMtxImm(spotMtx, (GXTexMtx)(GX_PTTEXMTX0+s3), GX_MTX3x4);
					GXLoadTexMtxImm(texKillMtx, (GXTexMtx)(GX_PTTEXMTX1+s3), GX_MTX2x4);
					
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+s2+nTexGen), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, GX_PTTEXMTX0+s3);
					GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD1+s2+nTexGen), GX_TG_MTX2x4, GX_TG_POS, _nPosMatrix, FALSE, GX_PTTEXMTX1+s3);
					
					GXSetTevKColor((GXTevKColorID)(GX_KCOLOR0+i), pSpotLight->GCColor);
					
					//First stage
					//replace i with s.
					GXSetTevKColorSel((GXTevStageID)(GX_TEVSTAGE0+s), (GXTevKColorSel)(GX_TEV_KCSEL_K0+i));
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE0+s), (GXTexCoordID)(GX_TEXCOORD0+s2+nTexGen), (GXTexMapID)(GX_TEXMAP1+nTex+i), GX_COLOR_NULL );
					
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE0+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE0+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

					//Second stage, backface culling					
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEXCOORD_NULL, GX_TEXMAP_NULL, (i<2)?GX_COLOR0A0:GX_COLOR1A1 );
					GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CC_ZERO, (i==0||i==2)?GX_CC_RASC:GX_CC_RASA, GX_CC_C0, GX_CC_ZERO);
					
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE1+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE1+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					
					//Third stage	
					GXSetTevOrder((GXTevStageID)(GX_TEVSTAGE2+s), (GXTexCoordID)(GX_TEXCOORD0+s2+1+nTexGen), GX_TEXMAP0, GX_COLOR_NULL );

					if (i==0)
					{			
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
					}
					else
					{
						GXSetTevColorIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_CPREV);
					}
					GXSetTevColorOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					GXSetTevAlphaIn((GXTevStageID)(GX_TEVSTAGE2+s), GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
					GXSetTevAlphaOp((GXTevStageID)(GX_TEVSTAGE2+s), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
					
					GXSetNumTevStages(n*3);
					GXSetNumTexGens(n*2+1);
					GXSetNumIndStages(1);
				}
			}
		}
	}
}

void _PerPixelSpec(void)
{
	Mtx SpecMtx;
		
    f32 fScale, fDotP, fR;
    f32 fTweak = 0.5f;
    
    Vec  vdir, vhalf, vaxis;
    Vec  vview = { 0.0f, 0.0f, 1.0f };
    
    CFLight *pDirLight = fvis_GetCurrentDirectional();
    
    if (pDirLight)
    {
		if ( pDirLight->m_nXfmKey_VS != FXfm_nViewKey)
		{
			pDirLight->m_vUnitDir_VS = FGCXfm_mtxGCLeftToRightHandViewMtx.m44.MultDir( pDirLight->m_mtxOrientation_WS.m_vFront );
		}
		pDirLight->m_nXfmKey_VS = FXfm_nViewKey;

	    // Calculate light direction
	    vdir.x = pDirLight->m_vUnitDir_VS.x;
	    vdir.y = pDirLight->m_vUnitDir_VS.y;
	    vdir.z = pDirLight->m_vUnitDir_VS.z;
	   
	    if (vdir.x || vdir.y || vdir.z)
	    {
		    // Calculate the half angle
		    fDotP = -vdir.z;
		    if ( fDotP == -1.0f )
		    {
		        // Singular potnt ( vview +  = 0 )
		        MTXScale(SpecMtx, 0.0f, 0.0f, 0.0f); // zero matrix
		    }
		    else
		    {
			    // The obtained half-angle vector directs an opposite side
			    vhalf.x = -vdir.x;
			    vhalf.y = -vdir.y;
			    vhalf.z = -vdir.z + 1.0f;
			    
			    VECNormalize(&vhalf, &vhalf);

			    if ( fDotP == 1.0f || (fmath_Abs(vhalf.y) < 0.0001f && fmath_Abs(vhalf.x) < 0.0001f) )
			    {
				    MTXIdentity(SpecMtx);
			    }
			    else
			    {
			        vaxis.x = -vhalf.y;
			        vaxis.y = vhalf.x;
			        vaxis.z = 0.0f;
			        
			        fR = acosf(-vhalf.z);
			        			        
			        if ( fmath_Abs(fR) > 0.001f )
			        {
						MTXRotAxisRad(SpecMtx, &vaxis, fR);
					}
			    }
			    	    
			    // Scaling and bias
			    fScale = 2.0f * fTweak + 1.5f;
			    
			    MTXScaleApply(SpecMtx, SpecMtx, fScale, -fScale, fScale);
			    
			    SpecMtx[0][3] = 0.5f;
			    SpecMtx[1][3] = 0.5f;
			    SpecMtx[2][0] = SpecMtx[2][1] = SpecMtx[2][2] = 0.0f;
			    SpecMtx[2][3] = 1.0f;
			}
			GXLoadTexMtxImm(SpecMtx, PMTX_PERPIXEL_SPECULAR, GX_MTX3x4);
		}
		
		SpecMtx[0][0] = -vdir.x; SpecMtx[0][1] = -vdir.y; SpecMtx[0][2] = -vdir.z; SpecMtx[0][3] = 0;
		SpecMtx[1][0] = 0; SpecMtx[1][1] = 0; SpecMtx[1][2] = 0; SpecMtx[1][3] = 0;
		SpecMtx[2][0] = 0; SpecMtx[2][1] = 0; SpecMtx[2][2] = 0; SpecMtx[2][3] = 1;
		GXLoadTexMtxImm(SpecMtx, PMTX_PERPIXEL_SPECKILL, GX_MTX3x4);
	}
}

void fsh_PrepareFrame()
{
	//once per-frame calculations for things such as per-pixel specular and specular bumpmapping.

	//Generate per-pixel specular post transform matrix (GX_PTTEXMTX10)
//	_PerPixelSpec();
	
	frenderer_Fog_ComputeColor();
	_gxFogClr = _GXCOLOR_RGB(FRenderer_FogColorRGB.fRed, FRenderer_FogColorRGB.fGreen, FRenderer_FogColorRGB.fBlue, 1.0f);
	
	_nCurrentShader = 0xffffffff;
}

void fsh_EndFrame()
{
}

void fgcsh_ResetSwapTables( void )
{
	u32 i;
	for (i=0; i<4; i++)
	{
		if (_NeedSwapChange(i, GX_TEV_SWAP0, GX_TEV_SWAP0))
		{
			GXSetTevSwapMode(_anTevStage[i], GX_TEV_SWAP0, GX_TEV_SWAP0);
		}
	}
}

static void _HandleLMapBump(void)
{
	FShTexInst_t *pBump = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]);
	FTexDef_t *pTexDefLayer = pBump->TexInst.GetTexDef();
	if (pTexDefLayer)
	{
		_SetTexture( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj, GX_TEXMAP1 );
		GXInitTexObjWrapMode( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
	}
	
	u32 nLMap = 0;
	u32 _nt = (u32)nLMap*3+1;
	FShTexInst_t *pLM = (FShTexInst_t *)(FSh_pnLightMapInputRegisters[_nt]);
	pTexDefLayer = pLM->TexInst.GetTexDef();
	if (pTexDefLayer)
	{
		_SetTexture( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj, GX_TEXMAP0 );
		GXInitTexObjWrapMode( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj, GX_CLAMP, GX_CLAMP );
	}
	GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, (GXTexGenSrc)(GX_TG_TEX0+_nLMapTC), GX_IDENTITY);
	GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	// Indirect Stage 0 -- Lookup indirect map
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);

    // The image map is four times bigger than the indirect map
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

    // Stage 0 -- Output texture color
    //
    // TEVPREV = TEXC/TEXA

    GXSetTevIndWarp(GX_TEVSTAGE0,       // tev stage
                    GX_INDTEXSTAGE0,    // indirect stage
                    GX_TRUE,            // signed offsets?
                    GX_FALSE,           // replace mode?
                    GX_ITM_0);          // ind matrix select	
                    
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    
    f32 indMtx[2][3] = { { 0.5F, 0.0F, 0.0F, },   // Indirect "identity" mtx
                         { 0.0F, 0.5F, 0.0F, } }; // (not used)
	GXSetIndTexMtx(GX_ITM_0, indMtx, 0);
    
    // Set up TEV and such...
    GXSetNumTevStages(1);
    GXSetNumIndStages(1);

    // One texture coordinate, no colors.
    GXSetNumTexGens(2);
        
    _bIndLookup = TRUE;
}

void GenerateDirMtx(CFVec3A& vDir, MtxPtr mtx)
{
	CFVec3A vLook,vRight,vUp, up;
	
	up.Set(0, 1, 0);

    // compute unit target vector
    // use negative value to look down (-Z) axis
    vLook.x = -vDir.x; vLook.y = -vDir.y; vLook.z = -vDir.z;

    // vRight = camUp x vLook
    vRight = vRight.Cross(up, vLook);
    vRight = vRight.Unitize();

    // vUp = vLook x vRight
  	vUp = vUp.Cross(vLook, vRight);
  	vUp.Unitize();

    mtx[0][0] = vRight.x * 0.5f;
    mtx[0][1] = vRight.y * 0.5f;
    mtx[0][2] = vRight.z * 0.5f;
    mtx[0][3] = 0.5f;

    mtx[1][0] = vUp.x * 0.5f;
    mtx[1][1] = vUp.y * 0.5f;
    mtx[1][2] = vUp.z * 0.5f;
    mtx[1][3] = 0.5f;

    mtx[2][0] = 0;
    mtx[2][1] = 0;
    mtx[2][2] = 0;
    mtx[2][3] = 1;
}

static void _IndirectWarp(u8 nStage, f32 fScale, s8 nScaleExp, u8& nTev, u8& nInd, u8& nTexGen, u8& nTexMtx, u8& nTex, BOOL bReApply, BOOL bUseReg)
{
	if (!bReApply)
	{
		if (bUseReg)
		{
			FShTexInst_t *pBump = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]);
			FTexDef_t *pTexDefLayer = pBump->TexInst.GetTexDef();
			if (pTexDefLayer)
			{
				_SetTexture( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj, GX_TEXMAP0 );
				GXInitTexObjWrapMode( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
			}
		}
		
		// Indirect Stage 0 -- Lookup indirect map
	    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);
	    
	    if (FSh_fBumpTile > 0.0f)
	    {
		    Mtx fTexMtx;
		    MTXIdentity(fTexMtx);
		    if (FSh_fBumpTile != 1.0f)
		    {
		    	fTexMtx[0][0] = FSh_fBumpTile;
		    	fTexMtx[1][1] = FSh_fBumpTile;
		    }
	  	    GXLoadTexMtxImm(fTexMtx, GX_TEXMTX0, GX_MTX2x4);
	  	}
	    
	    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);//GX_IDENTITY);

	    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
	     
	    f32 indMtx[2][3] = { { fScale, 0.0F, 0.0F, },   // Indirect "identity" mtx
	                         { 0.0F, fScale, 0.0F, } }; // used to scale the warp.
		GXSetIndTexMtx(GX_ITM_0, indMtx, nScaleExp);
		
		nTev = 0;
	    nInd = 1;

		nTexGen = 1;
		nTex = 1; //bump + dot3
		nTexMtx = 1;
	}
	
	GXSetTevIndWarp((GXTevStageID)(GX_TEVSTAGE0+nStage),       // tev stage
                    GX_INDTEXSTAGE0,    // indirect stage
                    GX_TRUE,            // signed offsets?
                    GX_FALSE,           // replace mode?
                    GX_ITM_0);          // ind matrix select	
	
	_bIndLookup = TRUE;
}

static void _IndirectWarp_End(GXTevStageID nStage, f32 fScale, s8 nScaleExp, u8 nTex)
{
	FShTexInst_t *pBump = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]);
	FTexDef_t *pTexDefLayer = pBump->TexInst.GetTexDef();
	if (pTexDefLayer)
	{
		_SetTexture( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj,  (GXTexMapID)(GX_TEXMAP0+nTex) );
		GXInitTexObjWrapMode( (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
	}
	
	// Indirect Stage 0 -- Lookup indirect map
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, (GXTexMapID)(GX_TEXMAP0+nTex));
    
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
     
    f32 indMtx[2][3] = { { fScale, 0.0F, 0.0F, },   // Indirect "identity" mtx
                         { 0.0F, fScale, 0.0F, } }; // used to scale the warp.
	GXSetIndTexMtx(GX_ITM_0, indMtx, nScaleExp);
	
	GXSetTevIndWarp(nStage,       // tev stage
                    GX_INDTEXSTAGE0,    // indirect stage
                    GX_TRUE,            // signed offsets?
                    GX_FALSE,           // replace mode?
                    GX_ITM_0);          // ind matrix select	
	
	_bIndLookup = TRUE;
}
CFTexInst *_pReflectTex=NULL;

void fsh_SetReflectTex(CFTexInst *pTex)
{
	_pReflectTex = pTex;
}

void fsh_ActivateFullScrTargets(BOOL bActivate)
{
	if (bActivate != _bFullScrActive)
	{
		ftex_ActivateRenderTarget(_pFullScrTarget, bActivate);
		#if FGCSH_ALLOW_REFRACTION
		ftex_ActivateRenderTarget(_pFullScrLoRes, bActivate);
		#endif
		
		_bFullScrActive = bActivate;
	}
}

void fsh_CreateFullScreenTarget()
{
	char szName[16];
	sprintf(szName, "FullScr");
	_pFullScrTarget = ftex_CreateRenderTarget_FullScreen(FTEX_RENDERTARGET_FMT_C16_D24_S8, szName, FALSE, FRES_NULLHANDLE, NULL, 512, 464);
	#if FGCSH_ALLOW_REFRACTION
	sprintf(szName, "FullScrLoRes");
	_pFullScrLoRes = ftex_CreateRenderTarget_FullScreen(FTEX_RENDERTARGET_FMT_C16_D24_S8, szName, FALSE, FRES_NULLHANDLE, NULL, 256, 232);
	ftex_AddRenderTarget(_pFullScrLoRes, fsh_FullScrLoResCallback, TRUE, 20, FALSE, TRUE, NULL);
	#endif
	
	_pFullScrTarget->ClearFlag(CFTexInst::FLAG_WRAP_S);
	_pFullScrTarget->ClearFlag(CFTexInst::FLAG_WRAP_T);
	
	#if !FGCSH_ALLOW_REFRACTION
		ftex_AddRenderTarget(_pFullScrTarget, fsh_FullScrCallback, TRUE, 0, FALSE, TRUE, NULL);
	#else
		ftex_AddRenderTarget(_pFullScrTarget, fsh_FullScrCallback, TRUE, 30, FALSE, TRUE, NULL);
	#endif
		
	_bFullScrActive=TRUE;
	fsh_ActivateFullScrTargets(FALSE);
}

void fsh_ResetFullScrRenderTarget()
{
	#if !FGCSH_ALLOW_REFRACTION
		ftex_AddRenderTarget(_pFullScrTarget, fsh_FullScrCallback, TRUE, 0, FALSE, TRUE, NULL);
	#else
		ftex_AddRenderTarget(_pFullScrTarget, fsh_FullScrCallback, TRUE, 30, FALSE, TRUE, NULL);
	#endif
	
	#if FGCSH_ALLOW_REFRACTION
	ftex_AddRenderTarget(_pFullScrLoRes, fsh_FullScrLoResCallback, TRUE, 20, FALSE, TRUE, NULL);
	#endif
	
	_bFullScrActive=TRUE;
	fsh_ActivateFullScrTargets(FALSE);
}

CFTexInst *fsh_GetFullScrTexture()
{
	return (_pFullScrTarget);
}

u16 *fsh_GetFullScrData()
{
	//ftex_ActivateRenderTarget(_pFullScrTarget, FALSE);
	fsh_ActivateFullScrTargets(FALSE);

	u16 *pRet = NULL;
	FTexDef_t *pTexDef = _pFullScrTarget->GetTexDef();
	if (pTexDef)
	{				
		GXTexObj *pTex = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
		pRet = (u16*)GXGetTexObjData((const GXTexObj*)pTex);
		
		if ( pRet == NULL )
		{
			return NULL;
		}
		
		pRet = (u16 *)OSPhysicalToCached((u32)pRet);
	}
	return (pRet);
}

void fsh_ReleaseFullScrData()
{
	fsh_ActivateFullScrTargets(TRUE);
}

void fsh_CreateSReflectTarget(void *pUserData)
{
	CFTexInst *pTexInst;
	char szName[16];
	if (_nNumSReflectTargets < MAX_SREFLECT_TARGETS)
	{
		sprintf(szName, "SReflect%d", _nNumSReflectTargets);
		pTexInst = ftex_CreateRenderTarget(128, 128, FTEX_RENDERTARGET_FMT_C16_D24_S8, szName, TRUE);
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

void fsh_ResetSReflect()
{
	u32 i;
	for (i=0; i<MAX_SREFLECT_TARGETS; i++)
	{
		_aSReflectTargets[ _nNumSReflectTargets ].fSize = 0.0f;
	}
}

CFTexInst *fsh_GetSReflectTarget(f32 fSize, void *pUser)
{
	u32 i;
	for (i=0; i<_nNumSReflectTargets; i++)
	{
		if (_aSReflectTargets[i].pUser == pUser)
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

void fsh_ClearSReflectTargets()
{
	_nNumSReflectTargets = 0;
}

typedef struct
{
	CFVec3 vPlane;
	
	CFVec3 vQuad[4];
	u32 nType;
	
	//Volumetric fog - for refractions only.
	CFColorRGB fogClr;
	f32 fFogDensity; //0.0f = no fog.
	GXColor gxClr;
	//
	
	CFVec3 vCen;
	
	f32 fGeoCullDist;
	f32 fOpacity;
	f32 fTile;
	f32 fRadius;
	
	BOOL bActive;
	
	CFTexInst *pLayer0;
	CFTexInst *pLayer1;
} fsh_Render_Plane_t;

u32 _nNumRenderPlanes;
fsh_Render_Plane_t _aRenderPlanes[MAX_RENDER_PLANES];

CFMtx43A _ReflectMtx;

void fsh_SetupClipPlane(CFVec3 vPlane, CFVec3 vPoint, BOOL bReflect)
{
	Mtx ClipMtx;
	CFVec3 vN;
	CFVec3 vOffs;
	
	vN = -FGCXfm_mtxGCLeftToRightHandViewMtx.m44.MultDir( vPlane );
	vOffs = FGCXfm_mtxGCLeftToRightHandViewMtx.m44.MultPoint( vPoint );
	
	ClipMtx[0][0] = vN.x; ClipMtx[0][1] = vN.y; ClipMtx[0][2] = vN.z; ClipMtx[0][3] = -vOffs.Dot(vN);
	ClipMtx[1][0] = 0; ClipMtx[1][1] = 0; ClipMtx[1][2] = 0; ClipMtx[1][3] = 0.0f;
	ClipMtx[2][0] = ClipMtx[2][1] = ClipMtx[2][2] = 0; ClipMtx[2][3] = 1.0f;
		
	GXLoadTexMtxImm(ClipMtx, (u32)PMTX_CLIPPLANE, GX_MTX3x4);
	//Turn on the clip plane.
	FSh_bUseClipPlane = TRUE;
	
	if (bReflect)
	{
		_ReflectMtx.Identity();
		
		vN = vPlane;
			
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
		
		fgc_FlipCullDir(TRUE);
		
		GXCullMode gxCull = fgc_GetCullMode();
		fgc_SetCullMode(gxCull);
	}
	else
	{
		FXfm_pMirrorMtx = NULL;
	}
}

void fsh_ClearClipPlane()
{
	//Turn off the clip plane.
	FSh_bUseClipPlane = FALSE;
	
	fgc_FlipCullDir(FALSE);
}

void fsh_ClearRenderPlanes()
{
	_nNumRenderPlanes = 0;
}

void fsh_ActivateRenderPlane(u32 nPlaneID, BOOL bActive)
{
	_aRenderPlanes[ nPlaneID ].bActive = bActive;
}

u32 fsh_AddRenderPlane(CFVec3 *pQuad, u32 nType, BOOL bFlip, CFColorRGB *fogClr, f32 fFogDensity, f32 fGeoCullDist, f32 fOpacity, f32 fTile, f32 fRadius, CFTexInst *pLayer0, CFTexInst *pLayer1)
{
	u32 i, nPlaneID;
	_aRenderPlanes[ _nNumRenderPlanes ].vCen.Set(0,0,0);
	for (i=0; i<4; i++)
	{
		_aRenderPlanes[ _nNumRenderPlanes ].vQuad[i] = pQuad[i];
		_aRenderPlanes[ _nNumRenderPlanes ].vCen += pQuad[i];
	}
	_aRenderPlanes[ _nNumRenderPlanes ].vCen *= 0.25f;
	
	CFVec3 vS, vT, vN;
	vS = pQuad[1] - pQuad[0];
	vS.Unitize();
	vT = pQuad[2] - pQuad[0];
	vT.Unitize();
	
	vN = vS.UnitCross(vT);
	
	if (bFlip)
	{
		vN = -vN;
	}
	
	_aRenderPlanes[ _nNumRenderPlanes ].vPlane = vN;
	
	_aRenderPlanes[ _nNumRenderPlanes ].nType = nType;
	
	if (fogClr && fFogDensity && (nType&RP_REFRACT))
	{
		_aRenderPlanes[ _nNumRenderPlanes ].fogClr = *fogClr;
		_aRenderPlanes[ _nNumRenderPlanes ].fFogDensity = fFogDensity;
	}
	else
	{
		if (fogClr)
		{
			_aRenderPlanes[ _nNumRenderPlanes ].fogClr = *fogClr;
		}
		else
		{
			_aRenderPlanes[ _nNumRenderPlanes ].fogClr = CFColorRGB(1,1,1);
		}
		_aRenderPlanes[ _nNumRenderPlanes ].fFogDensity = 0.0f;
	}
	
	_aRenderPlanes[ _nNumRenderPlanes ].gxClr.r = _aRenderPlanes[ _nNumRenderPlanes ].fogClr.fRed * 255.0f;
	_aRenderPlanes[ _nNumRenderPlanes ].gxClr.g = _aRenderPlanes[ _nNumRenderPlanes ].fogClr.fGreen * 255.0f;
	_aRenderPlanes[ _nNumRenderPlanes ].gxClr.b = _aRenderPlanes[ _nNumRenderPlanes ].fogClr.fBlue * 255.0f;
	_aRenderPlanes[ _nNumRenderPlanes ].gxClr.a = fOpacity * 255.0f;
	
	_aRenderPlanes[ _nNumRenderPlanes ].fGeoCullDist = fGeoCullDist;
	_aRenderPlanes[ _nNumRenderPlanes ].fOpacity = fOpacity;
	_aRenderPlanes[ _nNumRenderPlanes ].fTile = fTile;
	_aRenderPlanes[ _nNumRenderPlanes ].fRadius = fRadius*fRadius;
	
	_aRenderPlanes[ _nNumRenderPlanes ].pLayer0 = pLayer0;
	_aRenderPlanes[ _nNumRenderPlanes ].pLayer1 = pLayer1;
		
	nPlaneID = _nNumRenderPlanes;
	_nNumRenderPlanes++;
	
	return (nPlaneID);
}

void fsh_ChangeRenderPlane(u32 nPlaneID, CFVec3 *pQuad, CFColorRGB *fogClr, f32 fFogDensity)
{
	u32 i;
	for (i=0; i<4; i++)
	{
		_aRenderPlanes[ nPlaneID ].vQuad[i] = pQuad[i];
	}
	
	CFVec3 vS, vT, vN;
	vS = pQuad[1] - pQuad[0];
	vS.Unitize();
	vT = pQuad[2] - pQuad[0];
	vT.Unitize();
	
	vN = vS.UnitCross(vT);
	
	_aRenderPlanes[ nPlaneID ].vPlane = vN;
	
	if (fogClr && fFogDensity && (_aRenderPlanes[ nPlaneID ].nType&RP_REFRACT))
	{
		_aRenderPlanes[ nPlaneID ].fogClr = *fogClr;
		_aRenderPlanes[ nPlaneID ].fFogDensity = fFogDensity;
	}
	else
	{
		_aRenderPlanes[ nPlaneID ].fFogDensity = 0.0f;
	}
}

BOOL bRenderPlane=TRUE;

void fsh_FullScrCallback(void *pUserData)
{
	s32 i, idx=-1;
	f32 fDist, fMinDist=FMATH_MAX_FLOAT;
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
			bRenderPlane = FALSE;
			
			pCamera->SetupCameraAndViewport();
			
			fsh_SetupClipPlane(_aRenderPlanes[idx].vPlane, _aRenderPlanes[idx].vQuad[3], (_aRenderPlanes[idx].nType&RP_REFLECT));
			
			fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);
			if (FVis_pSkyBox && !(_aRenderPlanes[idx].nType&RP_REFRACT))
			{
				FSh_bUseClipPlane = FALSE;
				frenderer_Push( FRENDERER_MESH, NULL );
		 		FVis_pSkyBox->Draw( FVIEWPORT_PLANESMASK_ALL, TRUE );
				frenderer_Pop();
				FSh_bUseClipPlane = TRUE;
			}
			fviewport_fCullDistOverride = _aRenderPlanes[idx].fGeoCullDist;
			fvis_Draw( NULL, FALSE);
			fviewport_fCullDistOverride = 0;
			bRenderPlane = TRUE;
			
			fsh_ClearClipPlane();
			FXfm_pMirrorMtx = NULL;
		}
		else
		{
			fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);
		}
	}
}

void fsh_FullScrLoResCallback(void *pUserData)
{	
	s32 i, idx=-1;
	if (_nNumRenderPlanes)
	{
		for (i=0; i<_nNumRenderPlanes; i++)
		{
			if (_aRenderPlanes[i].bActive)
			{
				if (_aRenderPlanes[i].nType&RP_REFLECT && _aRenderPlanes[i].nType&RP_REFRACT)
				{
					idx = i;
					break;
				}
			}
		}
		if (idx > -1)
		{
			bRenderPlane = FALSE;
			
			CFCamera *pCamera = fcamera_GetCameraByIndex( fcamera_GetLastCameraIndex() );
			pCamera->SetupCameraAndViewport();
			
			fsh_SetupClipPlane(_aRenderPlanes[i].vPlane, _aRenderPlanes[i].vQuad[3], FALSE);
			
			fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);
			if (FVis_pSkyBox)
			{
				FSh_bUseClipPlane = FALSE;
				frenderer_Push( FRENDERER_MESH, NULL );
				FVis_pSkyBox->Draw( FVIEWPORT_PLANESMASK_ALL, TRUE );
				frenderer_Pop();
				FSh_bUseClipPlane = TRUE;
			}
			fviewport_fCullDistOverride = _aRenderPlanes[i].fGeoCullDist;
			fvis_Draw( NULL, FALSE);
			fviewport_fCullDistOverride = 0;
			bRenderPlane = TRUE;
			
			fsh_ClearClipPlane();
			FXfm_pMirrorMtx = NULL;
		}
		else
		{
			fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);
		}
	}
}

f32 _afQuadU[4] = 
{
	0.0f, 1.0f, 1.0f, 0.0f
};

f32 _afQuadV[4] = 
{
	1.0f, 1.0f, 0.0f, 0.0f
};

void fsh_RenderPlane(u32 nPlaneID, f32 fScaleU, f32 fScaleV, f32 fScrollU, f32 fScrollV, CFTexInst *pEMBM, f32 fGlowScale, f32 fBumpTile)
{
	if (!bRenderPlane) { return; }
	GXTexObj *gxEMBM, *gxReflect, *gxRefract;
	FTexDef_t *pTexDef;
	fsh_Render_Plane_t *pPlane = &_aRenderPlanes[nPlaneID];
	
	u32 fCurBumpTile = FSh_fBumpTile;
	FSh_fBumpTile = 1.0f;
	
	u32 nQuad=1;
	f32 fDeltaY=0;
	
	Mtx texMtx, texMtx1;
	
	if (pPlane->nType&RP_EMISSIVE)
	{
		texMtx[0][0] = pPlane->fTile*fScaleU; texMtx[0][1] = texMtx[0][2] = 0.0f; texMtx[0][3] = 0;
		texMtx[1][1] = pPlane->fTile*fScaleV; texMtx[1][0] = texMtx[1][2] = 0.0f; texMtx[1][3] = 0;
		
		texMtx1[0][0] = pPlane->fTile*fScaleU; texMtx1[0][1] = texMtx1[0][2] = 0.0f; texMtx1[0][3] = fScrollU;
		texMtx1[1][1] = pPlane->fTile*fScaleV; texMtx1[1][0] = texMtx1[1][2] = 0.0f; texMtx1[1][3] = fScrollV;
	}
	else
	{
		texMtx[0][0] = 0.025f*pPlane->fTile*fScaleU; texMtx[0][1] = texMtx[0][2] = 0.0f; texMtx[0][3] = fScrollU;
		texMtx[1][2] = 0.025f*pPlane->fTile*fScaleV; texMtx[1][0] = texMtx[1][1] = 0.0f; texMtx[1][3] = fScrollV;
	}

	GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
	
	if ( (pPlane->nType&RP_REFLECT) && (pPlane->nType&RP_REFRACT) ) //water like surface.
	{
		u8 nTev=0, nInd=0, nTexGen=0, nTexMtx=0, nTex=0;
		u8 nDummy8=0;
		if (pEMBM)
		{
			_IndirectWarp(0,  0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
			#if FGCSH_ALLOW_REFRACTION
			_IndirectWarp(1, -0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE, FALSE);
			#endif
			
			pTexDef = pEMBM->GetTexDef();
			if (pTexDef)
			{
				gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it wrap after done
				GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
				//
				_SetTexture(gxEMBM, GX_TEXMAP0);
			}
			
			pTexDef = _pFullScrTarget->GetTexDef();
			if (pTexDef)
			{
				gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it clamp after done
				GXInitTexObjWrapMode( gxRefract, GX_CLAMP, GX_CLAMP );
				//
				_SetTexture(gxRefract, GX_TEXMAP1);
			}
			
			#if FGCSH_ALLOW_REFRACTION
			pTexDef = _pFullScrLoRes->GetTexDef();
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it clamp after done
				GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
				//
				_SetTexture(gxReflect, GX_TEXMAP2);
			}
			#endif
			
			if (pPlane->pLayer0)
			{
				_IndirectWarp(2, 0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE, FALSE);
				#if FGCSH_ALLOW_REFRACTION
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD2+nTexGen), GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0);
				GXSetTevOrder(GX_TEVSTAGE2, (GXTexCoordID)(GX_TEXCOORD2+nTexGen), GX_TEXMAP3, GX_COLOR_NULL );
				#else
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0);
				GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
				#endif
				
				pTexDef = pPlane->pLayer0->GetTexDef();
				if (pTexDef)
				{
					GXInitTexObjWrapMode( (GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
					#if FGCSH_ALLOW_REFRACTION
					_SetTexture((GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_TEXMAP3);
					#else
					_SetTexture((GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_TEXMAP2);
					#endif
				}
			}
			
			GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
			#if FGCSH_ALLOW_REFRACTION
			GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
			#endif
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
			#if FGCSH_ALLOW_REFRACTION
			GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
			#endif
						
			GXSetNumIndStages(1);
			
			_bIndLookup = TRUE;
		}
		else
		{
			pTexDef = _pFullScrTarget->GetTexDef();
			if (pTexDef)
			{
				gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it clamp after done
				GXInitTexObjWrapMode( gxRefract, GX_CLAMP, GX_CLAMP );
				//
				_SetTexture(gxRefract, GX_TEXMAP0);
			}
			
			pTexDef = _pFullScrLoRes->GetTexDef();
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it clamp after done
				GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
				//
				_SetTexture(gxReflect, GX_TEXMAP1);
			}
			
			if (pPlane->pLayer0)
			{
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD2+nTexGen), GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0);
				GXSetTevOrder(GX_TEVSTAGE2, (GXTexCoordID)(GX_TEXCOORD2+nTexGen), GX_TEXMAP3, GX_COLOR_NULL );
				
				pTexDef = pPlane->pLayer0->GetTexDef();
				if (pTexDef)
				{
					GXInitTexObjWrapMode( (GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
					_SetTexture((GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_TEXMAP3);
				}
			}
			
			GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
			GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
			GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL );
			
			GXSetNumIndStages(0);
		}
		
		#if FGCSH_ALLOW_REFRACTION	
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_HALF, GX_CC_TEXC, GX_CC_ZERO);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
		GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_HALF, GX_CC_TEXC, GX_CC_CPREV);
		GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
		GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
		if (pPlane->pLayer0)
		{
			GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
			GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
			if (_NeedSwapChange(2, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}	
			
			GXSetNumTevStages(3);
			GXSetNumTexGens((pEMBM)?(4):(3));
		}
		else
		{
			GXSetNumTevStages(2);
			GXSetNumTexGens((pEMBM)?(3):(2));
		}
		#else

		GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1_2);
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
		if (pPlane->pLayer0)
		{
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}	
			
			GXSetNumTevStages(2);
			GXSetNumTexGens((pEMBM)?(3):(2));
		}
		else
		{
			GXSetNumTevStages(1);
			GXSetNumTexGens((pEMBM)?(2):(1));
		}
		#endif
		
		if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
		{
			GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
		}
		
		if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
		{
			GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
		}
	}
	else if ( (pPlane->nType&RP_REFLECT) || (pPlane->nType&RP_REFRACT) )
	{
		u8 nTev, nInd, nTexGen, nTexMtx, nTex, nDummy8;
		if (pEMBM)
		{
			_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
			
			pTexDef = pEMBM->GetTexDef();
			if (pTexDef)
			{
				gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it wrap after done
				GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
				//
				_SetTexture(gxEMBM, GX_TEXMAP0);
			}
			
			pTexDef = _pFullScrTarget->GetTexDef();
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it clamp after done
				GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
				//
				_SetTexture(gxReflect, GX_TEXMAP1);
			}
			
			GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
			
			if (pPlane->pLayer0)
			{
				_IndirectWarp(1, 0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE, FALSE);
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0);
				GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
				
				pTexDef = pPlane->pLayer0->GetTexDef();
				if (pTexDef)
				{
					GXInitTexObjWrapMode( (GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
					_SetTexture((GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_TEXMAP2);
				}
			}		
				
			GXSetNumIndStages(1);
			
			_bIndLookup = TRUE;
		}
		else
		{
			pTexDef = _pFullScrTarget->GetTexDef();
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it clamp after done
				GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
				//
				_SetTexture(gxReflect, GX_TEXMAP0);
			}
			GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
			
			GXSetNumIndStages(0);
		}
		
		GXSetTevKColor(GX_KCOLOR0, pPlane->gxClr);

		GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
		if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
		{
			GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
		}
		
		if (pPlane->pLayer0)
		{
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}	
			
			GXSetNumTevStages(2);
			GXSetNumTexGens((pEMBM)?(3):(2));
		}
		else
		{
			GXSetNumTevStages(1);
			GXSetNumTexGens((pEMBM)?(2):(1));
		}
	}
	else if ( pPlane->nType&RP_EMISSIVE )
	{
		GXTexObj *pTemp;
		nQuad=11;
		fDeltaY=0.1f*fGlowScale;
		
		GXLoadTexMtxImm(texMtx1, GX_TEXMTX1, GX_MTX2x4);
		
		if (pPlane->pLayer0	&& pPlane->pLayer1)
		{
			u8 nTev=0, nInd=0, nTexGen=0, nTexMtx=0, nTex=0;
			u8 nDummy8=0;
			if (pEMBM)
			{
				_IndirectWarp(1,  0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				pTexDef = pPlane->pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxRefract, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxRefract, GX_TEXMAP1);
				}
				
				pTexDef = pPlane->pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxReflect, GX_TEXMAP2);
				}
				
				pTemp = (GXTexObj *)GXGetTexObjUserData( gxReflect );
				if (pTemp)
				{
					GXInitTexObjWrapMode( pTemp, GX_REPEAT, GX_REPEAT );
					_SetTexture(pTemp, GX_TEXMAP3);
				}
				else
				{
					_SetTexture(gxReflect, GX_TEXMAP3);
				}
				
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP3, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE2, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				pTexDef = pPlane->pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxRefract, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxRefract, GX_TEXMAP0);
				}
				
				pTexDef = pPlane->pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				pTexDef = pPlane->pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				pTemp = (GXTexObj *)GXGetTexObjUserData( gxReflect );
				if (pTemp)
				{
					_SetTexture(pTemp, GX_TEXMAP2);
				}
				else
				{
					_SetTexture(gxReflect, GX_TEXMAP2);
				}
				
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL );
				
				GXSetNumIndStages(0);
			}
			
			GXSetTevKColor(GX_KCOLOR0, pPlane->gxClr);
			GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
			
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
				
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV);
			GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	

			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP2))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP2);
			}
			
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(2, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(3, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			GXSetNumTevStages(4);
			GXSetNumTexGens((pEMBM)?(3):(1));
		}
		else
		{
			u8 nTev, nInd, nTexGen, nTexMtx, nTex;
			if (pEMBM)
			{
				_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it wrap after done
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				if (pPlane->pLayer0)
				{
					pTexDef = pPlane->pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pPlane->pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				if (pPlane->pLayer0)
				{
					pTexDef = pPlane->pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pPlane->pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxReflect, GX_TEXMAP0);
				}
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				
				GXSetNumIndStages(0);
			}
			
			GXSetTevKColor(GX_KCOLOR0, pPlane->gxClr);

			GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			
			GXSetNumTevStages(1);
			GXSetNumTexGens((pEMBM)?(2):(1));
		}
	}
	else
	{
		if (pPlane->pLayer0	&& pPlane->pLayer1)
		{
			u8 nTev=0, nInd=0, nTexGen=0, nTexMtx=0, nTex=0;
			u8 nDummy8=0;
			if (pEMBM)
			{
				_IndirectWarp(0,  0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				_IndirectWarp(1, -0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it wrap after done
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				pTexDef = pPlane->pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					_SetTexture(gxRefract, GX_TEXMAP1);
				}
				
				pTexDef = pPlane->pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					_SetTexture(gxReflect, GX_TEXMAP2);
				}
				
				GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				pTexDef = pPlane->pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxRefract, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxRefract, GX_TEXMAP0);
				}
				
				pTexDef = pPlane->pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL );
								
				GXSetNumIndStages(0);
			}
				
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_HALF, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevKColor(GX_KCOLOR0, pPlane->gxClr);
			
			GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
					
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_HALF, GX_CC_TEXC, GX_CC_CPREV);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			GXSetNumTevStages(2);
			GXSetNumTexGens((pEMBM)?(3):(2));
		}
		else
		{
			u8 nTev, nInd, nTexGen, nTexMtx, nTex;
			if (pEMBM)
			{
				_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it wrap after done
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				if (pPlane->pLayer0)
				{
					pTexDef = pPlane->pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pPlane->pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				if (pPlane->pLayer0)
				{
					pTexDef = pPlane->pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pPlane->pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxReflect, GX_TEXMAP0);
				}
				GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				
				GXSetNumIndStages(0);
			}
			
			GXSetTevKColor(GX_KCOLOR0, pPlane->gxClr);

			GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			
			GXSetNumTevStages(1);
			GXSetNumTexGens((pEMBM)?(2):(1));
		}
	}

	#if !FGCSH_ALLOW_REFRACTION
	if ( (pPlane->fOpacity < 1.0f && (pPlane->nType&RP_TEXTURE)) || (pPlane->nType&RP_REFLECT && pPlane->nType&RP_REFRACT) )
	#else
	if ( pPlane->fOpacity < 1.0f && (pPlane->nType&RP_TEXTURE) )
	#endif
	{
		fgc_SetBlendOp(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);		
	}
	else
	{
		fgc_SetBlendOp(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);		
	}
	fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
	
	fgc_SetZMode( TRUE, GX_LEQUAL, TRUE );
	
	// Set the vertex descriptions
	fgc_ClearVtxDesc();
	fgc_SetVtxPosDesc( GX_DIRECT );
	fgc_SetVtxSTDesc( GX_VA_TEX0, GX_DIRECT );
	
	Mtx mView;
	fgcxfm_ConvertXfmToGCMtx( mView, FGCXfm_mtxGCLeftToRightHandViewMtx );
	fgcxfm_LoadGCPosMatrixImm( mView, GX_PNMTX0 );
	
	Mtx mtxTemp;
	MTXIdentity( mtxTemp );
	GXLoadNrmMtxImm( mtxTemp, GX_PNMTX0 );
	fgcxfm_SetGCCurrentMatrix( GX_PNMTX0 );
	
	fgc_SetVtxPosFormat( GX_VTXFMT7, GX_F32, 0 );
	fgc_SetVtxSTFormat( GX_VTXFMT7, GX_VA_TEX0, GX_F32, 0 );
	
	GXCullMode gxCull = fgc_GetCullMode();
	fgc_SetCullMode( GX_CULL_NONE );
	fgc_SetClipMode(TRUE);
	GXColor _gxClr={0x20, 0x20, 0x20, 0x20};
	_gxClr.a = pPlane->gxClr.a;
	//Submit quad.
	u32 i, n;
	f32 fYOffs=0.0f;
	
	GXBegin(GX_QUADS, GX_VTXFMT7, 4);
		for (i=0; i<4; i++)
		{
			GXPosition3f32( pPlane->vQuad[i].x, pPlane->vQuad[i].y, pPlane->vQuad[i].z );
			GXTexCoord2f32( _afQuadU[i]*fBumpTile, _afQuadV[i]*fBumpTile );
		}
		fYOffs += fDeltaY;
	GXEnd();	

	if (nQuad > 1)
	{
		if (pPlane->pLayer0	&& pPlane->pLayer1)
		{
			GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
			GXSetNumTevStages(3);
		
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
				
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		}
	
		fgc_SetBlendOp(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);		
		
		GXSetTevKColor(GX_KCOLOR0, _gxClr);
		
		GXBegin(GX_QUADS, GX_VTXFMT7, 4*(nQuad-1));
			for (n=0; n<nQuad-1; n++)
			{
				for (i=0; i<4; i++)
				{
					GXPosition3f32( pPlane->vQuad[i].x, pPlane->vQuad[i].y+fYOffs, pPlane->vQuad[i].z );
					GXTexCoord2f32( _afQuadU[i]*fBumpTile, _afQuadV[i]*fBumpTile );
				}
				fYOffs += fDeltaY;
			}
		GXEnd();	
	}
	//
	fgc_SetCullMode( gxCull );
	
	fsh_EndExecute();
	
	FSh_fBumpTile = fCurBumpTile;
}

void fsh_DynamicSReflect_Callback(void *pUserData)
{
#if FSH_DYNAMIC_SREFLECT
	static CFXfm _VXfm;
	CFWorldMesh *pMesh = (CFWorldMesh *)pUserData;
	CFVec3 vPos;
	
	if ( (pMesh->m_nFlags&FMESHINST_FLAG_DONT_DRAW) ) return;
	if ( !pMesh->WasDrawnLastFrame() && !pMesh->WasDrawnThisFrame() ) 
	{
		return;
	}
	
	pMesh->m_Xfm.TransformPointF(vPos, pMesh->m_BoundSphere_MS.m_Pos);
	CFVec3 vDir, vUp(0,1,0);
	CFVec3A vCamPos, vCamDir, vN;
	f32 fFOV, f2Dot;
	
	CFCamera *pCamera = fcamera_GetCameraByIndex(0);
	const CFXfm *pCamXfm = pCamera->GetFinalXfm();
	pCamXfm->InitStackWithView();
	
	vCamPos.Set(FXfm_pView->m_MtxR.m_vPos);
	vCamDir.Set(FXfm_pView->m_MtxR.m_vFront);
	
	vN.x = vCamPos.x - vPos.x; vN.y = vCamPos.y - vPos.y; vN.z = vCamPos.z - vPos.z;
	if (vN.x || vN.y || vN.z)
	{
		vN.Unitize();
	}
	else
	{
		vN.x = 1.0f;
	}
	
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
		
	fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);
	
	fFOV = FMATH_DEG2RAD(70.0f);
	fviewport_InitPersp(_pSView, fFOV, fFOV, 0.1f, 1000.0f);
	fviewport_SetActive(_pSView);
	
	_VXfm.BuildLookatFromDirVec(vPos, vDir, vUp);
	_VXfm.InitStackWithView();
	
	pMesh->m_nFlags |= FMESHINST_FLAG_DONT_DRAW;
	fvis_Draw( NULL, FALSE);
	pMesh->m_nFlags &= ~FMESHINST_FLAG_DONT_DRAW;
#endif
}

static void _GenerateNrmlReflect(MtxPtr mtx)
{
	MTXScale(mtx, -0.5f, -0.5f, 0.5f);
	mtx[0][3] = 0.5f;
	mtx[1][3] = 0.5f;
	mtx[2][0] = mtx[2][1] = mtx[2][2] = 0.0f;
	mtx[2][3] = 1.0f;
}

static void _rbSReflect()
{
	if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]&&FSh_shaderFlags&FSh_RENDER_BUMP)
	{
		_IndirectWarp_End(GX_TEVSTAGE2, 0.5f, 1, 3);
		GXSetNumIndStages(1);
	}

	GXSetNumTevStages(3);
	GXSetNumTexGens(2);
	
	GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0A0);
	GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV);
	GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
	GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	if (_NeedSwapChange(2, GX_TEV_SWAP0, GX_TEV_SWAP0))
	{
		GXSetTevSwapMode(_anTevStage[2], GX_TEV_SWAP0, GX_TEV_SWAP0);
	}	
	
	FShTexInst_t *pTexInst;
	FTexDef_t *pTexDef;
	
	if (_pReflectTex)
	{
		pTexInst = (FShTexInst_t *)_pReflectTex;
	}
	else
	{
		pTexInst = (FShTexInst_t *)(FSh_pnSurfaceInputRegisters[2]);
	}
	
	pTexDef = pTexInst->TexInst.GetTexDef();
	if (pTexDef)
	{				
		_SetTexture((GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_TEXMAP2);
		
		Mtx _nrmlReflectMtx;
		_GenerateNrmlReflect(_nrmlReflectMtx);
		GXLoadTexMtxImm(_nrmlReflectMtx, (u32)PMTX_NORMAL_REFLECT, GX_MTX3x4);
		
		Mtx mNrml34;
		if (fgcsh_bWorldGeo)
		{
			fgcxfm_ConvertXfmToGCMtx( mNrml34, FGCXfm_mtxGCLeftToRightHandViewMtx );
			mNrml34[0][3] = mNrml34[1][3] = mNrml34[2][3] = 0.0f;
		}
		else
		{
			mNrml34[0][0] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[0][0]; mNrml34[0][1] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[0][1]; mNrml34[0][2] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[0][2]; mNrml34[0][3] = 0;
			mNrml34[1][0] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[1][0]; mNrml34[1][1] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[1][1]; mNrml34[1][2] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[1][2]; mNrml34[1][3] = 0;
			mNrml34[2][0] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[2][0]; mNrml34[2][1] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[2][1]; mNrml34[2][2] = (FGCXfm_apNrmMtxBuffer[FGCXfm_nCurrentBuffer][ _nNrmlMatrix ])[2][2]; mNrml34[2][3] = 0;
		}
		GXLoadTexMtxImm(mNrml34, GX_TEXMTX1, GX_MTX3x4);
		GXSetTexCoordGen2( GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX1, TRUE, PMTX_NORMAL_REFLECT );
	}
}

void fgcsh_SetLMapTCStart(u32 nStart)
{
	_nLMapTC = nStart;
}

void fsh_ZWriteEnable(BOOL bEnable)
{
	FSh_ZEnable = bEnable;
}

static void _LoadFogMtx()
{
	Mtx ZviewMtx, YworldMtx;
	
	// mad Fd, Zview, fsh_fFogConst[0], fsh_fFogConst[1]
	MTXIdentity(ZviewMtx);
	ZviewMtx[0][0] = 0.0f; ZviewMtx[0][2] = fsh_fFogConst[0]; ZviewMtx[0][3] = fsh_fFogConst[1];
	ZviewMtx[1][1] = 0.0f; ZviewMtx[1][3] = 0.5f;
	ZviewMtx[2][2] = 0.0f; ZviewMtx[2][3] = 1.0f;
	
	GXLoadTexMtxImm(ZviewMtx, PMTX_FOG_DEPTH, GX_MTX3x4);
	
	// mad Fh, Yworld, fsh_fFogConst[2], fsh_fFogConst[3]
	MTXIdentity(YworldMtx);
	YworldMtx[0][0] = 0.0f; YworldMtx[0][1] = fsh_fFogConst[2]; YworldMtx[0][3] = fsh_fFogConst[3];
	YworldMtx[1][1] = 0.0f; YworldMtx[1][3] = 0.5f;
	YworldMtx[2][2] = 0.0f; YworldMtx[2][3] = 1.0f;
	//InvView to convert from view space (_nPosMatrix) to world space
	_ApplyInvView(YworldMtx); 
	GXLoadTexMtxImm(YworldMtx, PMTX_FOG_HEIGHT, GX_MTX3x4);
}

FINLINE u8 _FogFuncType()
{
	u8 nRet = 0xff;
	if (_nShaderID == _anDiffuseRemap[FSHADERS_VLIGHT_BASIC] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_VLIGHT_MASK_EMISSIVE] ||
		_nShaderID == _anDiffuseRemap[FSHADERS_LM1] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_LM1_EMASK] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_LM2] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_LM2_EMASK] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_LM3] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_LM3_EMASK] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_LM4] || 
		_nShaderID == _anDiffuseRemap[FSHADERS_LM4_EMASK])
	{
		nRet = 0x00;
	}
	else if (_nShaderID == _anDiffuseRemap[FSHADERS_VLIGHT_BASIC_CO] || 
  			 _nShaderID == _anDiffuseRemap[FSHADERS_VLIGHT_MASK_EMISSIVE_CO] || 
  			 _nShaderID == _anDiffuseRemap[FSHADERS_LM1_ZMASK] || 
  			 _nShaderID == _anDiffuseRemap[FSHADERS_LM2_ZMASK] || 
  			 _nShaderID == _anDiffuseRemap[FSHADERS_LM3_ZMASK] || 
  			 _nShaderID == _anDiffuseRemap[FSHADERS_LM4_ZMASK])
	{
		nRet = 0x01;			 
	}
	else if (_nShaderID == FSHADERS_tBASE || 
  			 _nShaderID ==  FSHADERS_etBASE || 
  			 _nShaderID ==  FSHADERS_tBASE_ADD_SPEC || 
  			 _nShaderID ==  FSHADERS_etBASE_ADD_SPEC || 
  			 _nShaderID == FSHADERS_etBASE_ADD_bSPEC || 
  			 _nShaderID == FSHADERS_tBASE_LERP_tLAYER || 
  			 _nShaderID == FSHADERS_tBASE_ADD_SPEC_LERP_tLAYER || 
  			 _nShaderID == FSHADERS_etBASE_ADD_rbENV ||
  			 _nShaderID == FSHADERS_tBASE_vALPHA)
	{
		nRet = 0x02;			 
	}
	
	return (nRet);
}

static BOOL _HandleFog(u32 nTevStages, u32 nTexGens, u32 nTexMaps)
{
	static u32 __nViewUpdateKey = 0;
	u8 nFogFuncType;
	if (FXfm_nViewKey != __nViewUpdateKey)
	{
		__nViewUpdateKey = FXfm_nViewKey;
		_LoadFogMtx();
	}
	
	GXTexMapID nTexMapID[2];
	GXTexCoordID nTexCoordID[2];
	GXTevStageID nTevStageID[4];
	
	nTexMapID[0] = (GXTexMapID)( GX_TEXMAP0+nTexMaps );
	nTexMapID[1] = (GXTexMapID)( nTexMapID[0] + 1 );
	
	nTexCoordID[0] = (GXTexCoordID)( GX_TEXCOORD0+nTexGens );
	nTexCoordID[1] = (GXTexCoordID)( nTexCoordID[0]+1 );
	
	nTevStageID[0] = (GXTevStageID)( GX_TEVSTAGE0+nTevStages );
	nTevStageID[1] = (GXTevStageID)( nTevStageID[0]+1 );
	nTevStageID[2] = (GXTevStageID)( nTevStageID[0]+2 );
	nTevStageID[3] = (GXTevStageID)( nTevStageID[0]+3 );
	
	nFogFuncType = _FogFuncType();
	
	if (nFogFuncType < 0xff) //OPAQUE MATERIALS
	{
		//Fd = Zview * fsh_fFogConst[0]  + fsh_fFogConst[1]
		//Fh = Yworld * fsh_fFogConst[2] + fsh_fFogConst[3]
		//F = Fd * Fh
		
		//stage(n+1): calc Fd				-> lookup intensity ramp
		//stage(n+2): calc Fh*stage(n+1)	-> lookup intensity ramp
		//output(alpha) = Fd * Fh
				
		//RAMP TEXTURE: tex(0.0, 0.5) = 0.0 -> tex(1.0, 0.5) = 1.0
		_SetTexture(&_gxFogRamp1, nTexMapID[0]);
		_SetTexture(&_gxFogRamp2, nTexMapID[1]);

		//STAGE1
		// mad Fd, Zview, fsh_fFogConst[0], fsh_fFogConst[1]
		if (fgcsh_bWorldGeo)
		{
			GXSetTexCoordGen2(nTexCoordID[0], GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_FOG_DEPTH);
		}
		else
		{
			GXSetTexCoordGen2(nTexCoordID[0], GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, PMTX_FOG_DEPTH);
		}
				
		//STAGE2
		// mad Fh, Yworld, fsh_fFogConst[2], fsh_fFogConst[3]
		// mul F, Fd, Fh
		if (fgcsh_bWorldGeo)
		{
			GXSetTexCoordGen2(nTexCoordID[1], GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_FOG_HEIGHT);
		}
		else
		{
			GXSetTexCoordGen2(nTexCoordID[1], GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, PMTX_FOG_HEIGHT);
		}
	}
	
	if (nFogFuncType==0x00)
	{
		GXSetNumTevStages(nTevStages+2);
		GXSetNumTexGens(nTexGens+2);
		
		//STAGE1
		GXSetTevOrder(nTevStageID[0], nTexCoordID[0], nTexMapID[0], GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[0], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[0], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[0], GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
		GXSetTevAlphaOp(nTevStageID[0], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------

		//STAGE2
		GXSetTevOrder(nTevStageID[1], nTexCoordID[1], nTexMapID[1], GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[1], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[1], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[1], GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
		GXSetTevAlphaOp(nTevStageID[1], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------
		return TRUE;
	}
	else if (nFogFuncType==0x01) //CUTOUT MATERIALS
	{
		GXSetNumTevStages(nTevStages+3);
		GXSetNumTexGens(nTexGens+2);
		
		GXColor gxRes={0xff, 0xff, 0xff, 0xff};
		GXSetTevColor(GX_TEVREG2, gxRes);
		
		fgc_SetAlphaCompare(GX_GREATER, 0x00, GX_AOP_OR, GX_EQUAL, 0xff);
		
		GXSetTevKColor(GX_KCOLOR3, _GXCOLOR_RGB(0.5f, 0.5f, 0.5f, 0.5f));
		GXSetTevKAlphaSel(nTevStageID[0], GX_TEV_KASEL_K3_A);
		
		//STAGE1
		GXSetTevOrder(nTevStageID[0], GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[0], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[0], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[0], GX_CA_APREV, GX_CA_KONST, GX_CA_A2, GX_CA_ZERO);
		GXSetTevAlphaOp(nTevStageID[0], GX_TEV_COMP_A8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------
	
		//STAGE2
		GXSetTevOrder(nTevStageID[1], nTexCoordID[0], nTexMapID[0], GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[1], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[1], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[1], GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
		GXSetTevAlphaOp(nTevStageID[1], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------

		//STAGE3
		GXSetTevOrder(nTevStageID[2], nTexCoordID[1], nTexMapID[1], GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[2], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[2], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[2], GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
		GXSetTevAlphaOp(nTevStageID[2], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------
		return TRUE;
	}
	else if (nFogFuncType==0x02) //TRANSLUCENT MATERIALS
	{
		GXSetNumTevStages(nTevStages+4);
		GXSetNumTexGens(nTexGens+2);
		
		//STAGE1 - Stuff opacity into Reg0
		//mov r0, alpha
		GXSetTevOrder(nTevStageID[0], GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[0], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[0], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[0], GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
		GXSetTevAlphaOp(nTevStageID[0], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
		//-------------------
	
		//STAGE2 - Calc. Fd
		// mad Fd, Zview, fsh_fFogConst[0], fsh_fFogConst[1]
		GXSetTevOrder(nTevStageID[1], nTexCoordID[0], nTexMapID[0], GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[1], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[1], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[1], GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
		GXSetTevAlphaOp(nTevStageID[1], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------

		//STAGE3  - Calc. Fh, and F = Fd*Fh
		// mad Fh, Yworld, fsh_fFogConst[2], fsh_fFogConst[3]
		// mul F, Fd, Fh
		GXSetTevOrder(nTevStageID[2], nTexCoordID[1], nTexMapID[1], GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[2], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[2], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[2], GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
		GXSetTevAlphaOp(nTevStageID[2], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------
		
		//STAGE3  - Final opacity = Reg0 * (1 - F)
		// mul r0, r0, 1-F
		GXSetTevOrder(nTevStageID[3], GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
		GXSetTevColorIn(nTevStageID[3], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
		GXSetTevColorOp(nTevStageID[3], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(nTevStageID[3], GX_CA_A0, GX_CA_ZERO, GX_CA_APREV, GX_CA_ZERO);
		GXSetTevAlphaOp(nTevStageID[3], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		//-------------------
		
		return TRUE;
	}
	
	return FALSE;
}

void _HandleProcedural(CFLiquidVolume *pLiquid)
{
	u32 i, j, nTexLayerID;
	f32 fBumpTile = FSh_fBumpTile;
	FSh_fBumpTile = 0;
	
	FTexDef_t *pTexDef;
	GXTexObj *gxEMBM, *gxReflect, *gxRefract, *pTemp;

	CFTexInst *pEMBM = pLiquid->GetEMBM();
	FShTexInst_t *pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[2];  //This is the reflection texture.
	FShTexInst_t *pShTexInst2;
	if (!pShTexInst)
	{
		pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[0];
	}

	for (i=0; i<2; i++)
	{
		Mtx mtxTemp;
		if( _nTexCoordMtxCount == 0 ) 
		{
			// No TC matrices...
			MTXIdentity(mtxTemp);
			
			GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+i*3, GX_MTX2x4);
		} 
		else 
		{
			nTexLayerID = pShTexInst->nTexLayerID;

			for( j=0; j<_nTexCoordMtxCount; j++ ) 
			{
				if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
				{
					fgcxfm_ConvertToGCTexMtx( mtxTemp, *_aTexCoordMtx[j].pTexCoordMtx );
						
					GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+i*3, GX_MTX2x4);
					break;
				}
			}
			if( j == _nTexCoordMtxCount ) 
			{
				// This stage's texture is not flagged for TC animation...
				MTXIdentity(mtxTemp);
				GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+i*3, GX_MTX2x4);
			}
		}
	}

	if (_nShaderID == FSHADERS_LIQUID_MOLTEN_1LAYER || _nShaderID == FSHADERS_LIQUID_MOLTEN_2LAYER)
	{
		pShTexInst = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[0];
		
		if (_nShaderID == FSHADERS_LIQUID_MOLTEN_2LAYER)
		{
			pShTexInst2 = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[2];
		
			u8 nTev=0, nInd=0, nTexGen=0, nTexMtx=0, nTex=0;
			u8 nDummy8=0;
			
			_IndirectWarp(1,  0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				
			pTexDef = pEMBM->GetTexDef();
			if (pTexDef)
			{
				gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
				_SetTexture(gxEMBM, GX_TEXMAP0);
			}
			
			pTexDef = pShTexInst->TexInst.GetTexDef();
			if (pTexDef)
			{
				gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				GXInitTexObjWrapMode( gxRefract, GX_REPEAT, GX_REPEAT );
				_SetTexture(gxRefract, GX_TEXMAP1);
			}
			
			pTexDef = pShTexInst2->TexInst.GetTexDef();
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
				_SetTexture(gxReflect, GX_TEXMAP2);
			}
			
			pTemp = (GXTexObj *)GXGetTexObjUserData( gxReflect );
			if (pTemp)
			{
				GXInitTexObjWrapMode( pTemp, GX_REPEAT, GX_REPEAT );
				_SetTexture(pTemp, GX_TEXMAP3);
			}
			else
			{
				_SetTexture(gxReflect, GX_TEXMAP3);
			}
			
			GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
			GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1);
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP3, GX_COLOR_NULL );
			GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
			GXSetTevOrder(GX_TEVSTAGE2, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
			
			GXSetNumIndStages(1);
			
			_bIndLookup = TRUE;
						
			GXSetTevKColor(GX_KCOLOR0, FGC_gxWhite);
			GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
			
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
				
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV);
			GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0);
			GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
			GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	

			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP2))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP2);
			}
			
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(2, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(3, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			GXSetNumTevStages(4);
			GXSetNumTexGens(3);
		}
		else
		{
			u8 nTev, nInd, nTexGen, nTexMtx, nTex;
			
			_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
			
			pTexDef = pEMBM->GetTexDef();
			if (pTexDef)
			{
				gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
				_SetTexture(gxEMBM, GX_TEXMAP0);
			}
			
			pTexDef = pShTexInst->TexInst.GetTexDef();
			
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
				_SetTexture(gxReflect, GX_TEXMAP1);
			}
			
			GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR0A0 );
			
			GXSetNumIndStages(1);
			
			_bIndLookup = TRUE;
						
			GXSetTevKColor(GX_KCOLOR0, FGC_gxWhite);

			GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			
			GXSetNumTevStages(1);
			GXSetNumTexGens(2);
		}
	}
	else if (_nShaderID == FSHADERS_LIQUID_TEXTURE)
	{
		u8 nTev, nInd, nTexGen, nTexMtx, nTex;
		_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
			
		pTexDef = pEMBM->GetTexDef();
		if (pTexDef)
		{
			gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
			GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
			_SetTexture(gxEMBM, GX_TEXMAP0);
		}
			
		pTexDef = pShTexInst->TexInst.GetTexDef();
		if (pTexDef)
		{
			gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
			//will make it clamp after done
			GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
			//
			_SetTexture(gxReflect, GX_TEXMAP1);
		}
			
		GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
		//First stage		
		GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR0A0 );
			
		GXSetNumIndStages(1);
		
		_bIndLookup = TRUE;
		
		GXSetTevKColor(GX_KCOLOR0, FGC_gxWhite);

		GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
		if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
		{
			GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
		}
		
		GXSetNumTevStages(1);
		GXSetNumTexGens(2);
	}
	else if (_nShaderID == FSHADERS_LIQUID_ENV || _nShaderID == FSHADERS_LIQUID_LAYER_ENV)
	{
		u8 nTev, nInd, nTexGen, nTexMtx, nTex;
		_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
			
		pTexDef = pEMBM->GetTexDef();
		if (pTexDef)
		{
			gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
			GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
			_SetTexture(gxEMBM, GX_TEXMAP0);
		}
			
		pTexDef = pShTexInst->TexInst.GetTexDef();
		if (pTexDef)
		{
			gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
			//will make it clamp after done
			GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
			//
			_SetTexture(gxReflect, GX_TEXMAP1);
		}
			
		GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
		//First stage		
		GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR0A0 );
			
		if (_nShaderID == FSHADERS_LIQUID_LAYER_ENV)
		{
			GXSetTevDirect(GX_TEVSTAGE1);
			GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
			GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
			
			pShTexInst2 = (FShTexInst_t *)FSh_pnSurfaceInputRegisters[0];
			pTexDef = pShTexInst2->TexInst.GetTexDef();
			if (pTexDef)
			{
				GXInitTexObjWrapMode( (GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
				_SetTexture((GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_TEXMAP2);
			}
		}		
				
		GXSetNumIndStages(1);
		
		_bIndLookup = TRUE;
		
		GXSetTevKColor(GX_KCOLOR0, FGC_gxWhite);

		GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
		if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
		{
			GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
		}
		
		if (_nShaderID == FSHADERS_LIQUID_LAYER_ENV)
		{
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}	
			
			GXSetNumTevStages(2);
			GXSetNumTexGens(3);
		}
		else
		{
			GXSetNumTevStages(1);
			GXSetNumTexGens(2);
		}
	}
	
	FSh_fBumpTile = fBumpTile;
}

void _HandleFastPass()
{
	s32 i, n, ntex, ntinst;
	
	u32 ii;	
	FTexDef_t *pTexDefLayer[8];
	GXTexObj *pGCTexObjLayer[8];
	GXTexObj *pTemp;
	
	n = _aShaderRenderStates[_nSurfaceShaderID].nStages+2;
	ntex = _aShaderRenderStates[_nSurfaceShaderID].nTexGens;
	ntinst = _aShaderRenderStates[_nSurfaceShaderID].nTexures;	
	
	GXSetNumTevStages(n);
	GXSetNumTexGens(ntex);
				
	FShaderStageStates_t *stage;
	
	u32 nTexLayerID, j;
	CFTexInst TexInst;
	BOOL bNrmlEnv[6]={0,0,0,0,0,0};
	BOOL bDoNrmlEnv=FALSE;

	#if FPERF_ENABLE
		FPerf_nRawShSwitchCount++;
	#endif
	for (i=0; i<n; i++)
	{
		stage = &_aShaderStageStates[_nSurfaceShaderID][i];
		
		_TEVKCOLORSEL(i, stage->nColorKonst);
		_TEVKALPHASEL(i, stage->nAlphaKonst);
		_TEVORDER(i, stage->texCoordID, stage->texMapID, stage->channelID);
		
		_TEVCOLORIN(i, stage->cA, stage->cB, stage->cC, stage->cD);
		_TEVCOLOROP(i, stage->colorOp, stage->colorBias, stage->colorScale, stage->colorClamp, stage->colorResArg);
		_TEVALPHAIN(i, stage->aA, stage->aB, stage->aC, stage->aD);
		_TEVALPHAOP(i, stage->alphaOp, stage->alphaBias, stage->alphaScale, stage->alphaClamp, stage->alphaResArg);
	
		if (_NeedSwapChange(i, stage->swapColor, stage->swapAlpha))
		{
			_TEVSWAPMODE(i, stage->swapColor, stage->swapAlpha);
		}
	}
	
	int idx=0, nmtx=0;
	BOOL bpUseUserData[6];
	BOOL bDetailScale[6];
	BOOL bEmissiveMask=FALSE;
	s8 nTexMtxIdx[8], nLMap=0;
	
	for (i=0; idx<ntinst && i<6; i++)
	{
		pGCTexObjLayer[i] = NULL;
				
		if (_aShaderTexGens[_nSurfaceShaderID][i].nTexReg > -1)
		{
			bpUseUserData[idx] = FALSE;
			if (_aShaderTexGens[_nSurfaceShaderID][i].nMask == _ZMASK)
			{
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK]);
				bpUseUserData[idx] = TRUE;
				bDetailScale[idx] = FALSE;
			}
			else if (_aShaderTexGens[_nSurfaceShaderID][i].nMask == _SMASK)
			{
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK]);
				bpUseUserData[idx] = TRUE;
				bDetailScale[idx] = FALSE;
			}
			else if (_aShaderTexGens[_nSurfaceShaderID][i].nMask == _EMASK)
			{
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK]);
				bpUseUserData[idx] = TRUE;
				bDetailScale[idx] = FALSE;
				
				bEmissiveMask=TRUE;
			}
			else
			{
				_apRegShTexInst[idx] = NULL;
			}
			
			if (!_apRegShTexInst[idx])
			{
				bpUseUserData[idx] = (_aShaderTexGens[_nSurfaceShaderID][i].nTexReg&_USER_DATA_FLAG);
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnSurfaceInputRegisters[ _aShaderTexGens[_nSurfaceShaderID][i].nTexReg&(_USER_DATA_FLAG-1) ]);
								
				if ( bpUseUserData[idx]  && !_apRegShTexInst[idx] )
				{
					_apRegShTexInst[idx] = _apRegShTexInst[0];
				}
				if (_aShaderTexGens[_nSurfaceShaderID][i].nTCIReg & (_DETAIL_SCALE) && _aShaderTexGens[_nSurfaceShaderID][i].nTCIReg != -1)
				{
					bDetailScale[idx] = TRUE;
				}
				else
				{
					bDetailScale[idx] = FALSE;
				}
			}
			idx++;
		}
	}
	
	BOOL bUseUserData=FALSE;
	Mtx mtxTemp;
	
	// Get the Tex defs and the GCTexObjs for the layer textures
	
	for (i=ntinst; i<8; i++)
	{
		_aTexOverrideCache[i].pOrig = _aTexOverrideCache[i].pCur = NULL;
		_aTexOverrideCache[i].nTexLayerID = 0xffffffff;
	}
	
	for ( i = 0; i < ntinst; i++ )
	{
		nTexMtxIdx[i] = -1;
		_aTexOverrideCache[i].pOrig = _aTexOverrideCache[i].pCur = NULL;
		_aTexOverrideCache[i].nTexLayerID = 0xffffffff;
		if ( _apRegShTexInst[i] )
		{
			if (_apRegShTexInst[i])
			{
				_aTexOverrideCache[i].pOrig = &_apRegShTexInst[i]->TexInst;
				_aTexOverrideCache[i].pCur  = &_apRegShTexInst[i]->TexInst;
				_aTexOverrideCache[i].nTexLayerID = _apRegShTexInst[i]->nTexLayerID;
			}
		
			bUseUserData = bpUseUserData[i];
			
			if( _nTexCoordMtxCount > 0 && nmtx < _nTexCoordMtxCount ) 
			{
				// There is at least one TC matrix...
				nTexLayerID = _apRegShTexInst[i]->nTexLayerID;

				for( j=0; j<_nTexCoordMtxCount; j++ ) 
				{
					if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
					{
						// This stage's texture is flagged for TC animation...
						nTexMtxIdx[i] = nmtx;
						fgcxfm_ConvertToGCTexMtx( mtxTemp, *_aTexCoordMtx[j].pTexCoordMtx );
						
						GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+nmtx*3, GX_MTX2x4);
						
						nmtx++;
						
						break;
					}
				}
			}
			
			if (bDetailScale[i])
			{
				MTXIdentity(mtxTemp);
				
				mtxTemp[0][0] = FSh_fDetailTile;
				mtxTemp[1][1] = FSh_fDetailTile;
				
				GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+nmtx*3, GX_MTX2x4);
				
				nTexMtxIdx[i] = nmtx;
				
				nmtx++;
			}
			
			// If we have any texture overrides, apply them now
			if ( _nTexOverrideCount )
			{
				pGCTexObjLayer[i] = NULL;
				pTexDefLayer[i] = NULL;
				for ( ii = 0; ii < _nTexOverrideCount; ii++ )
				{
					if ( _apRegShTexInst[i]->nTexLayerID == _aTexOverride[ii].nTexLayerID )
					{
						_aTexOverrideCache[i].pCur = _aTexOverride[ii].pTexInst;
						
						pTexDefLayer[i] = _aTexOverride[ii].pTexInst->GetTexDef();
						if (_aTexOverride[ii].pTexInst->GetTexDef()->pTexData)
						{
							pGCTexObjLayer[i] = (GXTexObj *)_aTexOverride[ii].pTexInst->GetTexDef()->pTexData->pGCTexObj;
						}
						else
						{
							pGCTexObjLayer[i] = NULL;
						}
						
						if (bUseUserData && pGCTexObjLayer[i])
						{
							pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
							if (pTemp)
							{
								pGCTexObjLayer[i] = pTemp;
							}
							else
							{
								if (_NeedSwapChange(i, _aShaderStageStates[_nSurfaceShaderID][i].swapColor, GX_TEV_SWAP0))
								{
									_TEVSWAPMODE(i, _aShaderStageStates[_nSurfaceShaderID][i].swapColor, GX_TEV_SWAP0);
								}
							}
						}
						
						if ( pGCTexObjLayer[i] )
						{
							fgctex_InitGCTexObj( &pGCTexObjLayer[i], _aTexOverride[ii].pTexInst );
						}
					}
				}
					
				if ( !pGCTexObjLayer[i] || !pTexDefLayer[i] )
				{
					pTexDefLayer[i] = _apRegShTexInst[i]->TexInst.GetTexDef();
					if (pTexDefLayer[i])
					{				
						pGCTexObjLayer[i] = (GXTexObj *)_apRegShTexInst[i]->TexInst.GetTexDef()->pTexData->pGCTexObj;
						
						if (bUseUserData)
						{
							pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
							if (pTemp)
							{
								pGCTexObjLayer[i] = pTemp;
							}
							else
							{
								if (_NeedSwapChange(i, _aShaderStageStates[_nSurfaceShaderID][i].swapColor, GX_TEV_SWAP0))
								{
									//GXSetTevSwapMode(_anTevStage[i], _aShaderStageStates[_nSurfaceShaderID][i].swapColor, GX_TEV_SWAP0);
									_TEVSWAPMODE(i, _aShaderStageStates[_nSurfaceShaderID][i].swapColor, GX_TEV_SWAP0);
								}
							}
						}
						
						if ( pGCTexObjLayer[i] )
						{
							fgctex_InitGCTexObj( &pGCTexObjLayer[i], &_apRegShTexInst[i]->TexInst );
						}
					}
				}
			}
			else
			{
				if ((GXTexObj *)_apRegShTexInst[i] == &_gxAttenMap)
				{
					pGCTexObjLayer[i] = (GXTexObj *)&_gxAttenMap;
				}
				else
				{
					pTexDefLayer[i] = _apRegShTexInst[i]->TexInst.GetTexDef();
					if (pTexDefLayer[i])
					{
						if (pTexDefLayer[i]->pTexData)
						{
							pGCTexObjLayer[i] = (GXTexObj *)pTexDefLayer[i]->pTexData->pGCTexObj;
							if (pGCTexObjLayer[i])
							{
								if (bUseUserData)
								{
									pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
									if (pTemp)
									{
										pGCTexObjLayer[i] = pTemp;
									}
									else
									{
										if (_NeedSwapChange(i, _aShaderStageStates[_nSurfaceShaderID][i].swapColor, GX_TEV_SWAP0))
										{
											_TEVSWAPMODE(i, _aShaderStageStates[_nSurfaceShaderID][i].swapColor, GX_TEV_SWAP0);
										}
									}
								}
								
								fgctex_InitGCTexObj( &pGCTexObjLayer[i], &_apRegShTexInst[i]->TexInst );
							}
						}
					}
				}
			}
		}
	}

	s8 tci, nreg, k;
	u32 nTexMtx;
	
	for (i=0; i<ntex; i++) //texgens
	{
		k = _aShaderRenderStates[_nSurfaceShaderID].texGen[i];

		if (_aShaderTexGens[_nSurfaceShaderID][k].nTCIReg != -1)
		{
			nreg = _aShaderTexGens[_nSurfaceShaderID][k].nTCIReg&(_DETAIL_SCALE-1);
		}
		else
		{
			nreg = -1;
		}
		
		tci = (u8)( nreg > -1 ? FSh_pnSurfaceInputRegisters[ nreg ] : 0 );
				
		if (nTexMtxIdx[i] > -1)
		{
			nTexMtx = GX_TEXMTX0+nTexMtxIdx[i]*3;
		}
		else
		{
			nTexMtx = _aShaderTexGens[_nSurfaceShaderID][k].mtx;
		}
		
		GXSetTexCoordGen( (GXTexCoordID)(GX_TEXCOORD0+i), (GXTexGenType)_aShaderTexGens[_nSurfaceShaderID][k].texGenType, (GXTexGenSrc)(_aShaderTexGens[_nSurfaceShaderID][k].texGenSrc+tci), nTexMtx );
	}

#if FANG_DEBUG_BUILD		
	FGCDL_ActiveTextures = ntex;
#endif	
	
	for (i=0; i<ntinst; i++) //texture instances.
	{
		if ( !pGCTexObjLayer[i] )
		{
			GXSetNumTexGens( 0 );
			GXSetNumTevStages( 1 );
		    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
			GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR0A0 );
			_nCurrentShader = 0xffffffff;
			return;
		}
		
		_SetTexture( pGCTexObjLayer[i], (GXTexMapID)(GX_TEXMAP0+i) );
	}
	
	{
		#if FSH_DYNAMIC_SREFLECT
		if (_nSurfaceShaderID == FSHADERS_oBASE_ADD_rbSREFLECT)
		{
			_rbSReflect();
		}
		#endif
	}
	
	GXSetTevKColor(GX_KCOLOR0, _gxSurfaceColor);
	GXSetTevKColor(GX_KCOLOR3, _gxLayerAlpha);
	
	_nPrevTexOverrideCount = _nTexOverrideCount;
	_nPrevTexCoordMtxCount = _nTexCoordMtxCount;
}

#define _USE_COLORKEY(idx) ( idx == 0 && fgcsh_bColorKey )

static void _SetPassGCState( void )
{
	s32 i, n, ntex, ntinst;
	
	u32 ii;	
	FTexDef_t *pTexDefLayer[8];
	GXTexObj *pGCTexObjLayer[8];
	GXTexObj *pTemp;
	
	if (_bFastShader)
	{
		_HandleFastPass();
		return;
	}
	
	BOOL bSetNumTevStages=TRUE;
	
	// Set rendering mode for no texture
	if (_nShaderID == -1)
	{
		GXSetNumTexGens( 0 );
		GXSetNumTevStages( 1 );
		GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
		GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR0A0 );
		_nCurrentShader = 0xffffffff;
		return;
	}
	
	if ( !(FSh_shaderFlags&FSh_RENDER_DETAIL) && _nShaderID >= FSHADERS_oBASE_DETAIL && _nShaderID <= FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL )
	{
		_nShaderID = _aDetailShaderRemap[ _nShaderID-FSHADERS_oBASE_DETAIL ];
	}
	if ( !(FSh_shaderFlags&FSh_RENDER_REFLECTIONMAP) && _nShaderID >= FSHADERS_oBASE_ADD_rbENV && _nShaderID <= FSHADERS_oBASE_LERP_pLAYER_ADD_rbENV_DETAIL )
	{
		_nShaderID = _aReflectionShaderRemap[ _nShaderID-FSHADERS_oBASE_ADD_rbENV ];
	}
	
	if (_nShaderID >= FSHADERS_LIQUID_ENV && _nShaderID <= FSHADERS_LIQUID_MOLTEN_2LAYER)
	{
		CFLiquidVolume *pLiquid = (CFLiquidVolume *)FSh_pProcedural;
	
		if (pLiquid)
		{	
			_HandleProcedural(pLiquid);
			_nCurrentShader = 0xffffffff;
			return;
		}
	}
	
	if (FSh_shaderType == SHADERTYPE_DIFFUSE && m_nLM > 0 && m_nLMMode > LM_NONE && !FSh_bShadowRender)
	{
		switch (m_nLMMode)
		{
			case ZMASK_EMASK_LM2:
				if (m_nLM == 1)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM1_ZMASK_EMASK];
				}
				else
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM2_ZMASK_EMASK];
				}
			break;
			case ZMASK_LM3:
				if (m_nLM == 1)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM1_ZMASK];
					
					fgc_SetAlphaCompare(GX_GEQUAL, 0x80, GX_AOP_OR, GX_EQUAL, 0xff);
				}
				else if (m_nLM == 2)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM2_ZMASK];
				}
				else
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM3_ZMASK];
				}
			break;
			case EMASK_LM3:
				if (m_nLM == 1)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM1_EMASK];
				}
				else if (m_nLM == 2)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM2_EMASK];
				}
				else
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM3_EMASK];
				}
			break;
			case LM4:
				if (m_nLM == 1)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM1];
				}
				else if (m_nLM == 2)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM2];
				}
				else if (m_nLM == 3)
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM3];
				}
				else
				{
					_nShaderID = _anDiffuseRemap[FSHADERS_LM4];
				}
			break;
		}
	}

	if ( FSh_shaderType == SHADERTYPE_TRANSLUCENCY && _nShaderID == FSHADERS_tBASE && m_nLMMode > LM_NONE)
	{
		_nShaderID = FSHADERS_tBASE_LM;
	}
	else if (FSh_shaderType == SHADERTYPE_TRANSLUCENCY && _nShaderID == FSHADERS_tBASE_DETAIL && m_nLMMode > LM_NONE)
	{
		_nShaderID = FSHADERS_tBASE_DETAIL_LM;
	}
	else if (FSh_shaderType == SHADERTYPE_TRANSLUCENCY && _nShaderID == FSHADERS_tBASE_LERP_tLAYER && m_nLMMode > LM_NONE)
	{
		_nShaderID = FSHADERS_tBASE_LERP_tLAYER_LM;
	}
	
	if ( (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY)  && _bFogEnabled && !FSh_bShadowRender && _nPassIdx == 0 )
	{
		if ( _HandleFog(_aShaderRenderStates[_nShaderID].nStages, _aShaderRenderStates[_nShaderID].nTexGens, _aShaderRenderStates[_nShaderID].nTexures) )
		{	
			bSetNumTevStages=FALSE;	
		}
	}

	if (_nPassIdx > 0)
	{
		_HandleLightingPass();
		_nCurrentShader = 0xffffffff;
		return;
	}
	
	//HANDLE DETAIL TILE & BUMP TILE
	if ( _bSetDetailScale ) //This is set in fsh_SetShader(), which also sets the register to read it from.
	{
		fsh_SetDetailMapTileFactor( *((f32 *)&FSh_pnSurfaceInputRegisters[_nDetailReg]) );
	}
	
	//Set the bumpmap tile if its the first diffuse lighting pass & the shader has a bumpmap.
	if (FSh_shaderType == SHADERTYPE_DIFFUSE && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP]&&FSh_shaderFlags&FSh_RENDER_BUMP)
	{
		fsh_SetBumpMapTileFactor( *((f32 *)&FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP_TILE_FACTOR]) );
	}
	
	n = _aShaderRenderStates[_nShaderID].nStages;
	ntex = _aShaderRenderStates[_nShaderID].nTexGens;
	ntinst = _aShaderRenderStates[_nShaderID].nTexures;
	
	u32 nAddClip;
	
	if (FSh_bShadowRender)
	{
		n = 1;
		if (_nShaderID == FSHADERS_VLIGHT_BASIC_CO || _nShaderID == FSHADERS_VLIGHT_MASK_EMISSIVE_CO || FSh_shaderType == SHADERTYPE_TRANSLUCENCY)
		{
			ntex = (ntex > 0)?(1):(0);
			ntinst = (ntinst > 0)?(1):(0);
		}
		else
		{
			ntex = 0;
			ntinst = 0;
		}
		
		nAddClip = 0;
		
		_nCurrentShader = 0xffffffff;
	}
	else
	{
		nAddClip = (FSh_bUseClipPlane && (FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY))?(1):(0);
	}
	
	BOOL bAddTex=FALSE;
	
	if (bSetNumTevStages)
	{
		GXSetNumTevStages(n + nAddClip);
		GXSetNumTexGens(ntex + nAddClip);
	}
			
	FShaderStageStates_t *stage;
	
	u32 nTexLayerID, j;
	CFTexInst TexInst;
	BOOL bNrmlEnv[6]={0,0,0,0,0,0};
	BOOL bDoNrmlEnv=FALSE;

	if (_nCurrentShader != _nShaderID || 1)
	{
#if FPERF_ENABLE
		FPerf_nRawShSwitchCount++;
#endif
		if (FSh_bShadowRender)
		{
			GXColor gxShadowID;
			
			if (FSh_ShadowID == 0)
			{
				gxShadowID.r = FSh_ShadowID;
				gxShadowID.g = FSh_ShadowID;
				gxShadowID.b = FSh_ShadowID;
				gxShadowID.a = FSh_ShadowID;
			}
			else
			{
				gxShadowID.r = 0xff;
				gxShadowID.g = 0xff;
				gxShadowID.b = 0xff;
				gxShadowID.a = 0xff;
			}
			
			GXSetTevKColor(GX_KCOLOR0, gxShadowID);
			GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
			
			if ((_nShaderID == FSHADERS_VLIGHT_BASIC_CO || _nShaderID == FSHADERS_VLIGHT_MASK_EMISSIVE_CO || FSh_shaderType == SHADERTYPE_TRANSLUCENCY))
			{
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
				GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
				GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
				GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP2))
				{
					GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP2);
				}
				
				_apRegShTexInst[0] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK]);
				
				if (!_apRegShTexInst[0])
				{
					_apRegShTexInst[0] = (FShTexInst_t *)(FSh_pnSurfaceInputRegisters[0]);
				}
				
				pTexDefLayer[0] = _apRegShTexInst[0]->TexInst.GetTexDef();
				if (pTexDefLayer[0])
				{
					if (pTexDefLayer[0]->pTexData)
					{
						pGCTexObjLayer[0] = (GXTexObj *)pTexDefLayer[0]->pTexData->pGCTexObj;
						
						if (pGCTexObjLayer[0])
						{
							GXTexObj *pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[0] );
							if (pTemp)
							{
								pGCTexObjLayer[0] = pTemp;
							}
							else
							{
								if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
								{
									GXSetTevSwapMode(_anTevStage[0], GX_TEV_SWAP0, GX_TEV_SWAP0);
								}
							}
						}
						
						if (pGCTexObjLayer[0])
						{
							fgctex_InitGCTexObj( &pGCTexObjLayer[0], &_apRegShTexInst[0]->TexInst );
						}
					}
				}
			}
			else
			{
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
				GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
				GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
				GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
				if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
				{
					GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
				}
			}
		}
		else
		{
			for (i=0; i<n; i++)
			{
				stage = &_aShaderStageStates[_nShaderID][i];
				
				_TEVKCOLORSEL(i, stage->nColorKonst);
				_TEVKALPHASEL(i, stage->nAlphaKonst);
				_TEVORDER(i, stage->texCoordID, stage->texMapID, stage->channelID);
				
				if (FSh_shaderType == SHADERTYPE_DIFFUSE)
				{
					if (i == 0 && _nDiffuseShaderID == FSHADERS_VLIGHT_MASK_EMISSIVE && m_nLM < 1)
					{
						if (FSh_bInvAlpha_Emissive)
						{
							GXSetTevColorIn(_anTevStage[i], GX_CC_KONST, GX_CC_ZERO, GX_CC_TEXA, GX_CC_ZERO);
						}
						else if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK] == NULL)
						{
							GXSetTevColorIn(_anTevStage[i], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
						}
						else
						{				
							_TEVCOLORIN(i, stage->cA, stage->cB, stage->cC, stage->cD);
						}
					}
					else if ( i == 1 && _nShaderID == _anDiffuseRemap[FSHADERS_LM1_EMASK] )
					{
						if (FSh_bInvAlpha_Emissive)
						{
							GXSetTevColorIn(_anTevStage[i], GX_CC_KONST, GX_CC_ZERO, GX_CC_TEXA, GX_CC_CPREV);
						}
						else if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK] == NULL)
						{
							GXSetTevColorIn(_anTevStage[i], GX_CC_ZERO, GX_CC_ONE, GX_CC_KONST, GX_CC_CPREV);
						}
						else
						{				
							_TEVCOLORIN(i, stage->cA, stage->cB, stage->cC, stage->cD);
						}
					}
					else
					{
						_TEVCOLORIN(i, stage->cA, stage->cB, stage->cC, stage->cD);
					}
				}
				else
				{
					_TEVCOLORIN(i, stage->cA, stage->cB, stage->cC, stage->cD);
				}
				
				_TEVCOLOROP(i, stage->colorOp, stage->colorBias, stage->colorScale, stage->colorClamp, stage->colorResArg);
				_TEVALPHAIN(i, stage->aA, stage->aB, stage->aC, stage->aD);
				_TEVALPHAOP(i, stage->alphaOp, stage->alphaBias, stage->alphaScale, stage->alphaClamp, stage->alphaResArg);
			
				if (_NeedSwapChange(i, stage->swapColor, stage->swapAlpha))
				{
					_TEVSWAPMODE(i, stage->swapColor, stage->swapAlpha);
				}
								
				if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_BUMPMAP] && stage->bEMBM&&FSh_shaderFlags&FSh_RENDER_BUMP)
				{
					_IndirectWarp_End(_anTevStage[i], 0.5f, 1, ntinst);
					bAddTex = TRUE;
				}
			}
		}
		
		_nCurrentShader = _nShaderID;
	}
	else
	{
		for (i=0; i<n; i++)
		{
			stage = &_aShaderStageStates[_nShaderID][i];
			if (_NeedSwapChange(i, stage->swapColor, stage->swapAlpha))
			{
				_TEVSWAPMODE(i, stage->swapColor, stage->swapAlpha);
			}
		}
	}
	
	if (bAddTex)
	{
		GXSetNumIndStages(1);
	}
	
	int idx=0, nmtx=0;
	u32 _nt;
	BOOL bpUseUserData[6];
	BOOL bDetailScale[6];
	BOOL bEmissiveMask=FALSE;
	s8 nTexMtxIdx[8], nLMap=0;
	if (!FSh_bShadowRender)
	{
		for (i=0; idx<ntinst && i<6; i++)
		{
			pGCTexObjLayer[i] = NULL;
			
			if (_aShaderTexGens[_nShaderID][i].nTexReg > -1)
			{
				if (_aShaderTexGens[_nShaderID][i].nMask == _ZMASK)
				{
					_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK]);
					bpUseUserData[idx] = TRUE;
					bDetailScale[idx] = FALSE;
				}
				else if (_aShaderTexGens[_nShaderID][i].nMask == _SMASK)
				{
					_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK]);
					bpUseUserData[idx] = TRUE;
					bDetailScale[idx] = FALSE;
				}
				else if (_aShaderTexGens[_nShaderID][i].nMask == _EMASK)
				{
					_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK]);
					bpUseUserData[idx] = TRUE;
					bDetailScale[idx] = FALSE;
					
					bEmissiveMask=TRUE;
				}
				else
				{
					_apRegShTexInst[idx] = NULL;
				}
				
				if (!_apRegShTexInst[idx])
				{
					bpUseUserData[idx] = (_aShaderTexGens[_nShaderID][i].nTexReg&_USER_DATA_FLAG);
					if (!bpUseUserData[idx] && m_nLMMode > LM_NONE && FSh_shaderType == SHADERTYPE_DIFFUSE)
					{
						_nt = (u32)nLMap*3+1;
						_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightMapInputRegisters[_nt]);
						nLMap++;
					}
					else if (m_nLMMode > LM_NONE && _aShaderTexGens[_nShaderID][i].nTCIReg == _ADDR_LM)
					{
						_nt = 1;
						_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightMapInputRegisters[_nt]);
						nLMap++;
					}
					else
					{
						_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnSurfaceInputRegisters[ _aShaderTexGens[_nShaderID][i].nTexReg&(_USER_DATA_FLAG-1) ]);
					}
									
					if ( bpUseUserData[idx]  && !_apRegShTexInst[idx] )
					{
						_apRegShTexInst[idx] = _apRegShTexInst[0];
					}
					if (_aShaderTexGens[_nShaderID][i].nTCIReg & (_DETAIL_SCALE) && _aShaderTexGens[_nShaderID][i].nTCIReg > -1)
					{
						bDetailScale[idx] = TRUE;
					}
					else
					{
						bDetailScale[idx] = FALSE;
					}
				}
				idx++;
			}
		}
	}
	
	BOOL bUseUserData=FALSE;
	Mtx mtxTemp;
	
	// Get the Tex defs and the GCTexObjs for the layer textures
	
	for (i=ntinst; i<8; i++)
	{
		_aTexOverrideCache[i].pOrig = _aTexOverrideCache[i].pCur = NULL;
		_aTexOverrideCache[i].nTexLayerID = 0xffffffff;
	}
	
	for ( i = 0; i < ntinst; i++ )
	{
		nTexMtxIdx[i] = -1;
		_aTexOverrideCache[i].pOrig = _aTexOverrideCache[i].pCur = NULL;
		_aTexOverrideCache[i].nTexLayerID = 0xffffffff;
		if ( _apRegShTexInst[i] )
		{
			if (_apRegShTexInst[i])
			{
				_aTexOverrideCache[i].pOrig = &_apRegShTexInst[i]->TexInst;
				_aTexOverrideCache[i].pCur  = &_apRegShTexInst[i]->TexInst;
				_aTexOverrideCache[i].nTexLayerID = _apRegShTexInst[i]->nTexLayerID;
			}
		
			bUseUserData = bpUseUserData[i];
			
			if( _nTexCoordMtxCount > 0 && nmtx < _nTexCoordMtxCount && m_nLMMode == LM_NONE) 
			{
				// There is at least one TC matrix...
				nTexLayerID = _apRegShTexInst[i]->nTexLayerID;

				for( j=0; j<_nTexCoordMtxCount; j++ ) 
				{
					if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
					{
						// This stage's texture is flagged for TC animation...
						nTexMtxIdx[i] = nmtx;
						fgcxfm_ConvertToGCTexMtx( mtxTemp, *_aTexCoordMtx[j].pTexCoordMtx );
						
						GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+nmtx*3, GX_MTX2x4);
						
						nmtx++;
						
						break;
					}
				}
			}
			
			if (bDetailScale[i] && !FSh_bShadowRender)
			{
				MTXIdentity(mtxTemp);
				
				mtxTemp[0][0] = FSh_fDetailTile;
				mtxTemp[1][1] = FSh_fDetailTile;
				
				GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+nmtx*3, GX_MTX2x4);
				
				nTexMtxIdx[i] = nmtx;
				
				nmtx++;
			}
			
			// If we have any texture overrides, apply them now
			if (!FSh_bShadowRender)
			{
				if ( _nTexOverrideCount && m_nLMMode == LM_NONE )
				{
					pGCTexObjLayer[i] = NULL;
					pTexDefLayer[i] = NULL;
					for ( ii = 0; ii < _nTexOverrideCount; ii++ )
					{
						if ( _apRegShTexInst[i]->nTexLayerID == _aTexOverride[ii].nTexLayerID )
						{
							_aTexOverrideCache[i].pCur = _aTexOverride[ii].pTexInst;
							
							pTexDefLayer[i] = _aTexOverride[ii].pTexInst->GetTexDef();
							if (_aTexOverride[ii].pTexInst->GetTexDef()->pTexData)
							{
								pGCTexObjLayer[i] = (GXTexObj *)_aTexOverride[ii].pTexInst->GetTexDef()->pTexData->pGCTexObj;
							}
							else
							{
								pGCTexObjLayer[i] = NULL;
							}
							
							if (bUseUserData && pGCTexObjLayer[i])
							{
								pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
								if (pTemp)
								{
									pGCTexObjLayer[i] = pTemp;
								}
								else
								{
									if (_NeedSwapChange(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0))
									{
										_TEVSWAPMODE(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0);
									}
								}
							}
							
							if ( pGCTexObjLayer[i] )
							{
								fgctex_InitGCTexObj( &pGCTexObjLayer[i], _aTexOverride[ii].pTexInst );
							}
						}
					}
					
					if ( !pGCTexObjLayer[i] || !pTexDefLayer[i] )
					{
						pTexDefLayer[i] = _apRegShTexInst[i]->TexInst.GetTexDef();
						if (pTexDefLayer[i])
						{				
							pGCTexObjLayer[i] = (GXTexObj *)_apRegShTexInst[i]->TexInst.GetTexDef()->pTexData->pGCTexObj;
							
							if (bUseUserData)
							{
								pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
								if (pTemp)
								{
									pGCTexObjLayer[i] = pTemp;
								}
								else
								{
									if (_NeedSwapChange(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0))
									{
										_TEVSWAPMODE(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0);
									}
								}
							}
							
							if ( pGCTexObjLayer[i] )
							{
								fgctex_InitGCTexObj( &pGCTexObjLayer[i], &_apRegShTexInst[i]->TexInst );
							}
						}
					}
				}
				else
				{
					if ((GXTexObj *)_apRegShTexInst[i] == &_gxAttenMap)
					{
						pGCTexObjLayer[i] = (GXTexObj *)&_gxAttenMap;
					}
					else
					{
						pTexDefLayer[i] = _apRegShTexInst[i]->TexInst.GetTexDef();
						if (pTexDefLayer[i])
						{
							if (pTexDefLayer[i]->pTexData)
							{
								pGCTexObjLayer[i] = (GXTexObj *)pTexDefLayer[i]->pTexData->pGCTexObj;
								if (pGCTexObjLayer[i])
								{
									if (bUseUserData)
									{
										pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
										if (pTemp)
										{
											pGCTexObjLayer[i] = pTemp;
										}
										else
										{
											if (_NeedSwapChange(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0))
											{
												_TEVSWAPMODE(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0);
											}
										}
									}
									
									fgctex_InitGCTexObj( &pGCTexObjLayer[i], &_apRegShTexInst[i]->TexInst );
								}
							}
						}
					}
				}
				
				if ( !pGCTexObjLayer[i] || !pTexDefLayer[i] )
				{
					GXSetNumTexGens( 0 );
					GXSetNumTevStages( 1 );
				    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
					GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR0A0 );
					_nCurrentShader = 0xffffffff;
					return;
				}
			}
		}
		else
		{
			pTexDefLayer[i] = NULL;
			pGCTexObjLayer[i] = NULL;
		}
	}

	s8 tci, nreg, k;
	u32 nTexMtx;
	
	for (i=0; i<ntex; i++) //texgens
	{
		k = _aShaderRenderStates[_nShaderID].texGen[i];

		if (_aShaderTexGens[_nShaderID][k].nTCIReg > -1)
		{
			nreg = _aShaderTexGens[_nShaderID][k].nTCIReg&(_DETAIL_SCALE-1);
		}
		else
		{
			nreg = _aShaderTexGens[_nShaderID][k].nTCIReg;
		}
		
		if (m_nLMMode > LM_NONE && i<m_nLM && FSh_shaderType == SHADERTYPE_DIFFUSE)
		{
			tci = _nLMapTC + i;
		}
		else if ( nreg == _ADDR_LM )
		{
			tci = 1;
		}
		else
		{
			tci = (u8)( nreg > -1 ? FSh_pnSurfaceInputRegisters[ nreg ] : 0 );
		}
		
		if (nTexMtxIdx[i] > -1)
		{
			nTexMtx = GX_TEXMTX0+nTexMtxIdx[i]*3;
		}
		else
		{
			nTexMtx = _aShaderTexGens[_nShaderID][k].mtx;
		}
		
		GXSetTexCoordGen( (GXTexCoordID)(GX_TEXCOORD0+i), (GXTexGenType)_aShaderTexGens[_nShaderID][k].texGenType, (GXTexGenSrc)(_aShaderTexGens[_nShaderID][k].texGenSrc+tci), nTexMtx );
	}

#if FANG_DEBUG_BUILD		
	FGCDL_ActiveTextures = ntex;
#endif	
	
	for (i=0; i<ntinst; i++) //texture instances.
	{
		if ( !pGCTexObjLayer[i] )
		{
			GXSetNumTexGens( 0 );
			GXSetNumTevStages( 1 );
		    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
			GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR0A0 );
			_nCurrentShader = 0xffffffff;
			return;
		}
		
		_SetTexture( pGCTexObjLayer[i], (GXTexMapID)(GX_TEXMAP0+i) );
	}
	
	if (!FSh_bShadowRender)
	{
		#if FSH_DYNAMIC_SREFLECT
		if (_nShaderID == FSHADERS_oBASE_ADD_rbSREFLECT)
		{
			_rbSReflect();
		}
		#endif
		
		if (FSh_bUseClipPlane&&(FSh_shaderType == SHADERTYPE_DIFFUSE || FSh_shaderType == SHADERTYPE_TRANSLUCENCY))
		{
			_SetTexture(&_gxTexKill, (GXTexMapID)(GX_TEXMAP0+ntinst));

			if (fgcsh_bWorldGeo)
			{
				GXSetTexCoordGen2( (GXTexCoordID)(GX_TEXCOORD0+ntex), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_CLIPPLANE );
			}
			else
			{
				GXSetTexCoordGen2( (GXTexCoordID)(GX_TEXCOORD0+ntex), GX_TG_MTX3x4, GX_TG_POS, _nPosMatrix, FALSE, PMTX_CLIPPLANE );
			}
			
			GXSetTevOrder(_anTevStage[n], (GXTexCoordID)(GX_TEXCOORD0+ntex), (GXTexMapID)(GX_TEXMAP0+ntinst), GX_COLOR_NULL);
			GXSetTevColorIn(_anTevStage[n], GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
			GXSetTevColorOp(_anTevStage[n], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			if (FSh_shaderType == SHADERTYPE_TRANSLUCENCY)
			{
				GXSetTevAlphaIn(_anTevStage[n], GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
			}
			else
			{
				GXSetTevAlphaIn(_anTevStage[n], GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			}
			GXSetTevAlphaOp(_anTevStage[n], GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			
			if (_NeedSwapChange(n, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(_anTevStage[n], GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
		}
	}
	_nPrevTexOverrideCount = _nTexOverrideCount;
	_nPrevTexCoordMtxCount = _nTexCoordMtxCount;
}

void _CheckTexCoordMtx()
{
	Mtx mtxTemp;
	s32 idx=0, nmtx=0;
	u32 i, j, nTexLayerID;
	BOOL bDetailScale[6];
	BOOL bpUseUserData[6];
	s8 nTexMtxIdx[8];
	u32 ntinst = _aShaderRenderStates[_nShaderID].nTexures;
	u32 ntex = _aShaderRenderStates[_nShaderID].nTexGens;
		
	u32 ii;	
	FTexDef_t *pTexDefLayer[8];
	GXTexObj *pGCTexObjLayer[8];
	GXTexObj *pTemp;
	
	BOOL bUseUserData=FALSE;
	
	for (i=0; idx<ntinst && i<6; i++)
	{
		pGCTexObjLayer[i] = NULL;
		
		if (_aShaderTexGens[_nShaderID][i].nTexReg > -1)
		{
			if (_aShaderTexGens[_nShaderID][i].nMask == _ZMASK)
			{
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_ZMASK]);
				bpUseUserData[idx] = TRUE;
				bDetailScale[idx] = FALSE;
			}
			else if (_aShaderTexGens[_nShaderID][i].nMask == _SMASK)
			{
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMASK]);
				bpUseUserData[idx] = TRUE;
				bDetailScale[idx] = FALSE;
			}
			else if (_aShaderTexGens[_nShaderID][i].nMask == _EMASK)
			{
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_EMASK]);
				bpUseUserData[idx] = TRUE;
				bDetailScale[idx] = FALSE;
			}
			else
			{
				_apRegShTexInst[idx] = NULL;
			}
			
			if (!_apRegShTexInst[idx])
			{
				bpUseUserData[idx] = (_aShaderTexGens[_nShaderID][i].nTexReg&_USER_DATA_FLAG);
				_apRegShTexInst[idx] = (FShTexInst_t *)(FSh_pnSurfaceInputRegisters[ _aShaderTexGens[_nShaderID][i].nTexReg&(_USER_DATA_FLAG-1) ]);
								
				if ( bpUseUserData[idx]  && !_apRegShTexInst[idx] )
				{
					_apRegShTexInst[idx] = _apRegShTexInst[0];
				}
				if (_aShaderTexGens[_nShaderID][i].nTCIReg & (_DETAIL_SCALE) && _aShaderTexGens[_nShaderID][i].nTCIReg != -1)
				{
					bDetailScale[idx] = TRUE;
				}
				else
				{
					bDetailScale[idx] = FALSE;
				}
			}
			idx++;
		}
	}
	
	for ( i = 0; i < ntinst; i++ )
	{
		nTexMtxIdx[i] = -1;
		if ( _apRegShTexInst[i] )
		{
		
			bUseUserData = bpUseUserData[i];
			
			if( _nTexCoordMtxCount > 0 && nmtx < _nTexCoordMtxCount && m_nLMMode == LM_NONE) {
				// There is at least one TC matrix...
				nTexLayerID = _apRegShTexInst[i]->nTexLayerID;

				for( j=0; j<_nTexCoordMtxCount; j++ ) 
				{
					if( nTexLayerID == _aTexCoordMtx[j].nTexLayerID ) 
					{
						// This stage's texture is flagged for TC animation...
						nTexMtxIdx[i] = nmtx;
						fgcxfm_ConvertToGCTexMtx( mtxTemp, *_aTexCoordMtx[j].pTexCoordMtx );
						
						GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+nmtx*3, GX_MTX2x4);
						
						nmtx++;
						
						break;
					}
				}
			}
			
			if (bDetailScale[i])
			{
				MTXIdentity(mtxTemp);
				
				mtxTemp[0][0] = FSh_fDetailTile;
				mtxTemp[1][1] = FSh_fDetailTile;
				
				GXLoadTexMtxImm(mtxTemp, GX_TEXMTX0+nmtx*3, GX_MTX2x4);
				
				nTexMtxIdx[i] = nmtx;
				
				nmtx++;
			}
			
			// If we have any texture overrides, apply them now
			if (1)//!FSh_bShadowRender)
			{
				if ( _nTexOverrideCount )
				{
					pGCTexObjLayer[i] = NULL;
					pTexDefLayer[i] = NULL;
					for ( ii = 0; ii < _nTexOverrideCount; ii++ )
					{
						if ( _apRegShTexInst[i]->nTexLayerID == _aTexOverride[ii].nTexLayerID )
						{
							_aTexOverrideCache[i].pCur = _aTexOverride[ii].pTexInst;
							
							pTexDefLayer[i] = _aTexOverride[ii].pTexInst->GetTexDef();
							if (_aTexOverride[ii].pTexInst->GetTexDef()->pTexData)
							{
								pGCTexObjLayer[i] = (GXTexObj *)_aTexOverride[ii].pTexInst->GetTexDef()->pTexData->pGCTexObj;
							}
							else
							{
								pGCTexObjLayer[i] = NULL;
							}
							
							if (bUseUserData && pGCTexObjLayer[i])
							{
								pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
								if (pTemp)
								{
									pGCTexObjLayer[i] = pTemp;
								}
								else
								{
									if (_NeedSwapChange(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0))
									{
										_TEVSWAPMODE(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0);
									}
								}
							}
							
							if ( pGCTexObjLayer[i] )
							{
								fgctex_InitGCTexObj( &pGCTexObjLayer[i], _aTexOverride[ii].pTexInst );
							}
						}
					}
					
					if ( !pGCTexObjLayer[i] || !pTexDefLayer[i] )
					{
						pTexDefLayer[i] = _apRegShTexInst[i]->TexInst.GetTexDef();
						if (pTexDefLayer[i])
						{				
							pGCTexObjLayer[i] = (GXTexObj *)_apRegShTexInst[i]->TexInst.GetTexDef()->pTexData->pGCTexObj;
							
							if (bUseUserData)
							{
								pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
								if (pTemp)
								{
									pGCTexObjLayer[i] = pTemp;
								}
								else
								{
									if (_NeedSwapChange(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0))
									{
										//GXSetTevSwapMode(_anTevStage[i], _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0);
										_TEVSWAPMODE(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0);
									}
								}
							}
							
							if ( pGCTexObjLayer[i] )
							{
								fgctex_InitGCTexObj( &pGCTexObjLayer[i], &_apRegShTexInst[i]->TexInst );
							}
						}
					}
				}
				else
				{
					if ((GXTexObj *)_apRegShTexInst[i] == &_gxAttenMap)
					{
						pGCTexObjLayer[i] = (GXTexObj *)&_gxAttenMap;
					}
					else
					{
						pTexDefLayer[i] = _apRegShTexInst[i]->TexInst.GetTexDef();
						if (pTexDefLayer[i])
						{
							if (pTexDefLayer[i]->pTexData)
							{
								pGCTexObjLayer[i] = (GXTexObj *)pTexDefLayer[i]->pTexData->pGCTexObj;
								if (pGCTexObjLayer[i])
								{
									if (bUseUserData)
									{
										pTemp = (GXTexObj *)GXGetTexObjUserData( pGCTexObjLayer[i] );
										if (pTemp)
										{
											pGCTexObjLayer[i] = pTemp;
										}
										else
										{
											if (_NeedSwapChange(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0))
											{
												_TEVSWAPMODE(i, _aShaderStageStates[_nShaderID][i].swapColor, GX_TEV_SWAP0);
											}
										}
									}
									
									fgctex_InitGCTexObj( &pGCTexObjLayer[i], &_apRegShTexInst[i]->TexInst );
								}
							}
						}
					}
				}
				
				if ( !pGCTexObjLayer[i] || !pTexDefLayer[i] )
				{
					GXSetNumTexGens( 0 );
					GXSetNumTevStages( 1 );
				    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
					GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR0A0 );
					_nCurrentShader = 0xffffffff;
					return;
				}
			}
		}
		else
		{
			pTexDefLayer[i] = NULL;
			pGCTexObjLayer[i] = NULL;
		}
	}

	s8 tci, nreg, k;
	u32 nTexMtx;
	
	for (i=0; i<ntex; i++) //texgens
	{
		k = _aShaderRenderStates[_nShaderID].texGen[i];

		if (_aShaderTexGens[_nShaderID][k].nTCIReg != -1)
		{
			nreg = _aShaderTexGens[_nShaderID][k].nTCIReg&(_DETAIL_SCALE-1);
		}
		else
		{
			nreg = -1;
		}
		
		tci = (u8)( nreg > -1 ? FSh_pnSurfaceInputRegisters[ nreg ] : 0 );
				
		if (nTexMtxIdx[i] > -1)
		{
			nTexMtx = GX_TEXMTX0+nTexMtxIdx[i]*3;
		}
		else
		{
			nTexMtx = _aShaderTexGens[_nShaderID][k].mtx;
		}
		
		GXSetTexCoordGen( (GXTexCoordID)(GX_TEXCOORD0+i), (GXTexGenType)_aShaderTexGens[_nShaderID][k].texGenType, (GXTexGenSrc)(_aShaderTexGens[_nShaderID][k].texGenSrc+tci), nTexMtx );
	}

#if FANG_DEBUG_BUILD		
	FGCDL_ActiveTextures = ntex;
#endif	
	
	for (i=0; i<ntinst; i++) //texture instances.
	{
		if ( !pGCTexObjLayer[i] )
		{
			GXSetNumTexGens( 0 );
			GXSetNumTevStages( 1 );
		    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
			GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEX_DISABLE, GX_COLOR0A0 );
			_nCurrentShader = 0xffffffff;
			return;
		}
		
		_SetTexture( pGCTexObjLayer[i], (GXTexMapID)(GX_TEXMAP0+i) );
	}
}

void fsh_FastSurfaceExecute()
{
	//Layer Alpha & surface color already handled correctly, no need to do anything here.
	
	//fsh_ExecuteCurrent(FALSE, 0);
	//return;
	
	_CheckTexCoordMtx();

	if (_pReflectTex && _nShaderID == FSHADERS_oBASE_ADD_rbSREFLECT)
	{
		//must set a special matrix per-mesh unfortunately....
		_rbSReflect();
	}
	
	if (_nShaderID != FSHADERS_INTENSITY)
	{
		GXSetTevKColor(GX_KCOLOR0, _gxSurfaceColor);
		GXSetTevKColor(GX_KCOLOR3, _gxLayerAlpha);
	}
}

void fsh_ExecuteCurrent(BOOL bShadowRender, s32 nShadowID)
{
	u32 _nOrigShader;
	FGC_bLazyRegisterSet = TRUE;
	FSh_bShadowRender = bShadowRender;
	FSh_ShadowID = nShadowID;
	
	if (FSh_shaderType == SHADERTYPE_DIFFUSE)
	{
		_nOrigShader = _nShaderID;
	}

	if (1)//_nShaderID != _nCurrentShader || FSh_shaderType != SHADERTYPE_SURFACE)
	{
		//Blend states, z buffering, etc. should only be set once for the surface shader.
		_SetRenderStates(); //set render states and blending depending on shader type.
	}
	
	_SetPassGCState();
	
	if (FSh_shaderType == SHADERTYPE_DIFFUSE)
	{
		_nShaderID = _nOrigShader;
	}
}

//
//
//
static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent ) 
{
	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FGCVID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FGCVID_EVENT_WINDOW_CREATED:
			_bWindowCreated = TRUE;
			_bShaderSystemOpen = FALSE;

			// Set the default material
			GXColor GCColor;
		    GCColor.r = GCColor.g = GCColor.b = GCColor.a = 255;
		    GXSetChanMatColor( GX_COLOR0A0, GCColor );
		    
			// Set up a swap mode that will use the green channel for alpha
			
			GXSetTevSwapModeTable(
				GX_TEV_SWAP1,
				GX_CH_RED,
				GX_CH_RED,
				GX_CH_RED,
				GX_CH_RED );
			
			GXSetTevSwapModeTable(
				GX_TEV_SWAP2,
				GX_CH_GREEN,
				GX_CH_GREEN,
				GX_CH_GREEN,
				GX_CH_GREEN );
				
			GXSetTevSwapModeTable(
				GX_TEV_SWAP3,
				GX_CH_BLUE,
				GX_CH_BLUE,
				GX_CH_BLUE,
				GX_CH_BLUE );
			
			// Setup light used for environment mapping
			{
				static GXLightObj __gxLight;
				GXInitLightColor(&__gxLight, FGC_gxWhite);
				GXInitLightPos(&__gxLight, 0.0f, 0.0f, 0.0f);
				GXInitLightDir(&__gxLight, 0, 0, -1);
				GXInitLightAttn(&__gxLight, 1, 0, 0, 1, 0, 0);
				GXLoadLightObjImm(&__gxLight, GX_LIGHT7);
			}
			
			Mtx ident;
			MTXIdentity(ident);
			
			GXLoadTexMtxImm(ident, GX_TEXMTX4, GX_MTX3x4);
			GXLoadTexMtxImm(ident, GX_TEXMTX5, GX_MTX3x4);
			if (!_pSView)
			{
				_pSView = fviewport_Create();
			}
			
			fsh_CreateFullScreenTarget();
			fsh_ClearRenderPlanes();
			fsh_ClearClipPlane();
			
			fworld_RegisterWorldCallbackFunction( FSh_WorldCallbackFunc );
			
			fsh_fFogConst[0] = -fsh_fFogDensity /(fsh_fFogFarZ - fsh_fFogNearZ);
			fsh_fFogConst[1] =  fsh_fFogNearZ*fsh_fFogConst[0];
			fsh_fFogConst[2] = -1.0f/(fsh_fFogMaxY - fsh_fFogMinY);
			fsh_fFogConst[3] = -fsh_fFogMaxY*fsh_fFogConst[2];
			
			fsh_pFogTex = NULL;
			
			_bFogEnabled = FALSE;
			//Initialize Post transform texture matrices

			u32 nMtx=GX_PTTEXMTX0;
			Mtx postXF;
			MTXIdentity(postXF);
			for (; nMtx<=GX_PTTEXMTX19; nMtx+=3)
			{
				GXLoadTexMtxImm(postXF, nMtx, GX_MTX3x4);
			}
			
			//
			break;

		case FGCVID_EVENT_WINDOW_DESTROYED:
			_bWindowCreated = FALSE;

			fworld_UnregisterWorldCallbackFunction( FSh_WorldCallbackFunc );
			
			break;

		case FGCVID_EVENT_PRE_RESET:
			break;

		case FGCVID_EVENT_POST_RESET:
			break;
	}

	return TRUE;
}

void fsh_DrawLiquidMesh(u32 nType, u16 nVtx, u16 nPrim, void *pMesh, u16 *pIdx, CFColorRGB *pClr, f32 fOpacity, CFVec3& vFwd, CFTexInst *pLayer0, CFTexInst *pLayer1, CFTexInst *pEMBM)
{
	GXTexObj *gxEMBM, *gxReflect, *gxRefract;
	FTexDef_t *pTexDef;
	
	LiquidFallVtx *pVtx = (LiquidFallVtx *)pMesh;
	
	u32 nQuad=1;
	f32 fDeltaY=0;
	
	GXColor gxClr;
	
	gxClr = _GXCOLOR_RGB(pClr->fRed, pClr->fGreen, pClr->fBlue, fOpacity);
	
	Mtx texMtx1;
	
	if ( (nType&RP_REFLECT) || (nType&RP_REFRACT) )
	{
		u8 nTev, nInd, nTexGen, nTexMtx, nTex, nDummy8;
		if (pEMBM)
		{
			_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
			
			pTexDef = pEMBM->GetTexDef();
			if (pTexDef)
			{
				gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
				_SetTexture(gxEMBM, GX_TEXMAP0);
			}
			
			pTexDef = _pFullScrTarget->GetTexDef();
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
				_SetTexture(gxReflect, GX_TEXMAP1);
			}
			
			GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION_INVERSE);
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
			
			if (pLayer0)
			{
				_IndirectWarp(1, 0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE, FALSE);
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
				GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
				
				pTexDef = pLayer0->GetTexDef();
				if (pTexDef)
				{
					GXInitTexObjWrapMode( (GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_REPEAT, GX_REPEAT );
					_SetTexture((GXTexObj *)pTexDef->pTexData->pGCTexObj, GX_TEXMAP2);
				}
			}		
				
			GXSetNumIndStages(1);
			
			_bIndLookup = TRUE;
		}
		else
		{
			pTexDef = _pFullScrTarget->GetTexDef();
			if (pTexDef)
			{
				gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
				//will make it clamp after done
				GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
				//
				_SetTexture(gxReflect, GX_TEXMAP0);
			}
			GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
			//First stage		
			GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
			
			GXSetNumIndStages(0);
		}
		
		GXSetTevKColor(GX_KCOLOR0, gxClr);

		GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
		GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
		GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
		GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
		GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
		if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
		{
			GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
		}
		
		if (pLayer0)
		{
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ONE, GX_CC_TEXC, GX_CC_CPREV);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
		
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}	
			
			GXSetNumTevStages(2);
			GXSetNumTexGens((pEMBM)?(3):(2));
		}
		else
		{
			GXSetNumTevStages(1);
			GXSetNumTexGens((pEMBM)?(2):(1));
		}
	}
	else if ( nType&RP_EMISSIVE )
	{
		GXTexObj *pTemp;
		nQuad=11;
		fDeltaY=0.001f;
		
		GXLoadTexMtxImm(texMtx1, GX_TEXMTX1, GX_MTX2x4);
		
		if (pLayer0	&& pLayer1)
		{
			u8 nTev=0, nInd=0, nTexGen=0, nTexMtx=0, nTex=0;
			u8 nDummy8=0;
			if (pEMBM)
			{
				_IndirectWarp(1,  0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				pTexDef = pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxRefract, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxRefract, GX_TEXMAP1);
				}
				
				pTexDef = pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxReflect, GX_TEXMAP2);
				}
				
				pTemp = (GXTexObj *)GXGetTexObjUserData( gxReflect );
				if (pTemp)
				{
					GXInitTexObjWrapMode( pTemp, GX_REPEAT, GX_REPEAT );
					_SetTexture(pTemp, GX_TEXMAP3);
				}
				else
				{
					_SetTexture(gxReflect, GX_TEXMAP3);
				}
				
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP3, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE2, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				pTexDef = pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxRefract, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxRefract, GX_TEXMAP0);
				}
				
				pTexDef = pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				pTexDef = pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				pTemp = (GXTexObj *)GXGetTexObjUserData( gxReflect );
				if (pTemp)
				{
					_SetTexture(pTemp, GX_TEXMAP2);
				}
				else
				{
					_SetTexture(gxReflect, GX_TEXMAP2);
				}
				
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL );
				
				GXSetNumIndStages(0);
			}
			
			GXSetTevKColor(GX_KCOLOR0, gxClr);
			GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
			
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
				
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_APREV, GX_CC_CPREV);
			GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	

			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP2))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP2);
			}
			
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(2, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(3, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			GXSetNumTevStages(4);
			GXSetNumTexGens((pEMBM)?(3):(1));
		}
		else
		{
			u8 nTev, nInd, nTexGen, nTexMtx, nTex;
			if (pEMBM)
			{
				_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it wrap after done
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				if (pLayer0)
				{
					pTexDef = pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				if (pLayer0)
				{
					pTexDef = pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_CLAMP, GX_CLAMP );
					//
					_SetTexture(gxReflect, GX_TEXMAP0);
				}
				GXSetTexCoordGen((GXTexCoordID)(GX_TEXCOORD0), GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				
				GXSetNumIndStages(0);
			}
			
			GXSetTevKColor(GX_KCOLOR0, gxClr);

			GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			
			GXSetNumTevStages(1);
			GXSetNumTexGens((pEMBM)?(2):(1));
		}
	}
	else
	{
		if (pLayer0	&& pLayer1)
		{
			u8 nTev=0, nInd=0, nTexGen=0, nTexMtx=0, nTex=0;
			u8 nDummy8=0;
			if (pEMBM)
			{
				_IndirectWarp(0,  0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				_IndirectWarp(1, -0.5f, 0, nDummy8, nDummy8, nDummy8, nDummy8, nDummy8, TRUE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				pTexDef = pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxRefract, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxRefract, GX_TEXMAP1);
				}
				
				pTexDef = pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxReflect, GX_TEXMAP2);
				}
				
				GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, (GXTexCoordID)(GX_TEXCOORD1+nTexGen), GX_TEXMAP2, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				pTexDef = pLayer0->GetTexDef();
				if (pTexDef)
				{
					gxRefract = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxRefract, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxRefract, GX_TEXMAP0);
				}
				
				pTexDef = pLayer1->GetTexDef();
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL );
								
				GXSetNumIndStages(0);
			}
				
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_HALF, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ONE);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevKColor(GX_KCOLOR0, gxClr);
			
			GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K0_A);
					
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_HALF, GX_CC_TEXC, GX_CC_CPREV);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			if (_NeedSwapChange(1, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}
			
			GXSetNumTevStages(2);
			GXSetNumTexGens((pEMBM)?(3):(2));
		}
		else
		{
			u8 nTev, nInd, nTexGen, nTexMtx, nTex;
			if (pEMBM)
			{
				_IndirectWarp(0, 0.5f, 0, nTev, nInd, nTexGen, nTexMtx, nTex, FALSE, FALSE);
				
				pTexDef = pEMBM->GetTexDef();
				if (pTexDef)
				{
					gxEMBM = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxEMBM, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxEMBM, GX_TEXMAP0);
				}
				
				if (pLayer0)
				{
					pTexDef = pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					_SetTexture(gxReflect, GX_TEXMAP1);
				}
				
				GXSetTexCoordGen2((GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, (GXTexCoordID)(GX_TEXCOORD0+nTexGen), GX_TEXMAP1, GX_COLOR_NULL );
				
				GXSetNumIndStages(1);
				
				_bIndLookup = TRUE;
			}
			else
			{
				if (pLayer0)
				{
					pTexDef = pLayer0->GetTexDef();
				}
				else
				{
					pTexDef = pLayer1->GetTexDef();
				}
				if (pTexDef)
				{
					gxReflect = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
					//will make it clamp after done
					GXInitTexObjWrapMode( gxReflect, GX_REPEAT, GX_REPEAT );
					//
					_SetTexture(gxReflect, GX_TEXMAP0);
				}
				GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_PNMTX0, FALSE, PMTX_PROJECTION);
				//First stage		
				GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL );
				
				GXSetNumIndStages(0);
			}
			
			GXSetTevKColor(GX_KCOLOR0, gxClr);

			GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			if (_NeedSwapChange(0, GX_TEV_SWAP0, GX_TEV_SWAP0))
			{
				GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
			}

			
			GXSetNumTevStages(1);
			GXSetNumTexGens((pEMBM)?(2):(1));
		}
	}

	if ( fOpacity < 1.0f )
	{
		fgc_SetBlendOp(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);		
	}
	else
	{
		fgc_SetBlendOp(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);		
	}
	fgc_SetAlphaCompare(GX_ALWAYS, 0x00, GX_AOP_AND, GX_ALWAYS, 0x00);
	
	fgc_SetZMode( TRUE, GX_LEQUAL, TRUE );
	
	// Set the vertex descriptions
	fgc_ClearVtxDesc();
	fgc_SetVtxPosDesc( GX_DIRECT );
	fgc_SetVtxSTDesc( GX_VA_TEX0, GX_DIRECT );
	fgc_SetVtxSTDesc( GX_VA_TEX1, GX_DIRECT );
	
	Mtx mView;
	fgcxfm_ConvertXfmToGCMtx( mView, FGCXfm_mtxGCLeftToRightHandViewMtx );
	fgcxfm_LoadGCPosMatrixImm( mView, GX_PNMTX0 );
	
	Mtx mtxTemp;
	MTXIdentity( mtxTemp );
	GXLoadNrmMtxImm( mtxTemp, GX_PNMTX0 );
	fgcxfm_SetGCCurrentMatrix( GX_PNMTX0 );
	
	fgc_SetVtxPosFormat( GX_VTXFMT7, GX_F32, 0 );
	fgc_SetVtxSTFormat( GX_VTXFMT7, GX_VA_TEX0, GX_F32, 0 );
	fgc_SetVtxSTFormat( GX_VTXFMT7, GX_VA_TEX1, GX_F32, 0 );
	
	GXCullMode gxCull = fgc_GetCullMode();
	fgc_SetCullMode( GX_CULL_NONE );
	fgc_SetClipMode(TRUE);
	GXColor _gxClr={0x20, 0x20, 0x20, 0x20};
	_gxClr.a = gxClr.a;
	//Submit quad.
	u32 i, n, nIdx;
	f32 fYOffs=0.0f;
	
	nIdx = 0;
	GXBegin(GX_TRIANGLES, GX_VTXFMT7, nPrim*3);
		for (i=0; i<nPrim; i++)
		{
			GXPosition3f32( pVtx[ pIdx[nIdx] ].vPos.x, pVtx[ pIdx[nIdx] ].vPos.y, pVtx[ pIdx[nIdx] ].vPos.z );
			GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[0].x, pVtx[ pIdx[nIdx] ].vTex[0].y );
			GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[1].x, pVtx[ pIdx[nIdx] ].vTex[1].y );
			nIdx++;
			
			GXPosition3f32( pVtx[ pIdx[nIdx] ].vPos.x, pVtx[ pIdx[nIdx] ].vPos.y, pVtx[ pIdx[nIdx] ].vPos.z );
			GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[0].x, pVtx[ pIdx[nIdx] ].vTex[0].y );
			GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[1].x, pVtx[ pIdx[nIdx] ].vTex[1].y );
			nIdx++;
			
			GXPosition3f32( pVtx[ pIdx[nIdx] ].vPos.x, pVtx[ pIdx[nIdx] ].vPos.y, pVtx[ pIdx[nIdx] ].vPos.z );
			GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[0].x, pVtx[ pIdx[nIdx] ].vTex[0].y );
			GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[1].x, pVtx[ pIdx[nIdx] ].vTex[1].y );
			nIdx++;
		}
		fYOffs += fDeltaY;
	GXEnd();	

	if (nQuad > 1)
	{
		if (pLayer0	&& pLayer1)
		{
			GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL );
			GXSetNumTevStages(3);
		
			GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
			GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
				
			GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_ZERO, GX_CC_APREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);	
			
			GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_K0);
			GXSetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_K0_A);
			GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_KONST, GX_CC_CPREV, GX_CC_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
			GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		}
	
		fgc_SetBlendOp(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_SET);		
		
		GXSetTevKColor(GX_KCOLOR0, _gxClr);
		CFVec3 vScale;
		vScale.Set(vFwd.x * fYOffs + 1.0f, vFwd.y * fYOffs + 1.0f, vFwd.z * fYOffs + 1.0f);
		
		GXBegin(GX_TRIANGLES, GX_VTXFMT7, nPrim*3*(nQuad-1));
			for (n=0; n<nQuad-1; n++)
			{
				nIdx = 0;
				for (i=0; i<nPrim; i++)
				{
					GXPosition3f32( pVtx[ pIdx[nIdx] ].vPos.x*vScale.x, pVtx[ pIdx[nIdx] ].vPos.y*vScale.y, pVtx[ pIdx[nIdx] ].vPos.z*vScale.z );
					GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[0].x, pVtx[ pIdx[nIdx] ].vTex[0].y );
					GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[1].x, pVtx[ pIdx[nIdx] ].vTex[1].y );
					nIdx++;
					
					GXPosition3f32( pVtx[ pIdx[nIdx] ].vPos.x*vScale.x, pVtx[ pIdx[nIdx] ].vPos.y*vScale.y, pVtx[ pIdx[nIdx] ].vPos.z*vScale.z );
					GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[0].x, pVtx[ pIdx[nIdx] ].vTex[0].y );
					GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[1].x, pVtx[ pIdx[nIdx] ].vTex[1].y );
					nIdx++;
					
					GXPosition3f32( pVtx[ pIdx[nIdx] ].vPos.x*vScale.x, pVtx[ pIdx[nIdx] ].vPos.y*vScale.y, pVtx[ pIdx[nIdx] ].vPos.z*vScale.z );
					GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[0].x, pVtx[ pIdx[nIdx] ].vTex[0].y );
					GXTexCoord2f32( pVtx[ pIdx[nIdx] ].vTex[1].x, pVtx[ pIdx[nIdx] ].vTex[1].y );
					nIdx++;
				}
				fYOffs += fDeltaY;
				
				vScale.x += vFwd.x*fDeltaY;
				vScale.y += vFwd.y*fDeltaY;
				vScale.z += vFwd.z*fDeltaY;
			}
		GXEnd();	
	}
	//
	fgc_SetCullMode( gxCull );
	
	fsh_EndExecute();
}
