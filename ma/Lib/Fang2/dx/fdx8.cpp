//////////////////////////////////////////////////////////////////////////////////////
// fdx8.cpp - Master DX8 Fang module.
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
// 09/30/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

#include "fdx8.h"
#include "fdx8vid.h"
#include "fdx8loop.h"
#include "fdx8tex.h"
#include "fdx8viewport.h"
#include "fdx8xfm.h"
#include "fdx8draw.h"
#include "fdx8mesh.h"
#include "fdx8load.h"
#include "fdx8shadow.h"
#include "fsh.h"
#include "fdx8vb.h"

#include "fsysinfo.h"
#include "fmath.h"
#include "fperf.h"
#include "fres.h"
#include "fresload.h"
#include "fcolor.h"
#include "fcpu.h"
#include "ftimer.h"
#include "fboxfilter.h"
#include "fviewport.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fmesh.h"
#include "fworld.h"
#include "fanim.h"
#include "fcamanim.h"
#include "ftext.h"
#include "ffile.h"
#include "fcoll.h"
#include "fpad.h"
#include "fpadio.h"
//#include "fdev.h"
#include "fpsprite.h"
#include "fshadow.h"
#include "fsound.h"
#include "faudio.h"
#include "fstorage.h"
#include "FScriptSystem.h"
#include "FMotionObj.h"
#include "fforce.h"
#include "fcamera.h"
#include "fstringtable.h"
#include "fgamedata.h"
#include "fsndfx.h"
#include "fparticle.h"
#include "fRenderSort.h"
#include "famem.h"
#include "fwire.h"
#include "fverlet.h"
#include "smoketrail.h"
#include "fdebris.h"
#include "fmovie2.h"
#include "fliquid.h"
#include "fexception.h"
#include "fvtxpool.h"
#include "fexplosion.h"
#include "FCheckPoint.h"
#include "falignedpool.h"
#include "flightpool.h"
#include "flightgroup.h"
#include "fdatastreaming.h"
#include "fdecal.h"

// !!Nate
#if !defined(_FANGDEF_PRODUCTION_BUILD) && defined(_FANGDEF_PLATFORM_XB)
#include "fserver.h"
#endif

IDirect3D8			*FDX8_pD3D;
IDirect3DDevice8	*FDX8_pDev;
IDirect3DSurface8	*FDX8_pSurfBack;
IDirect3DSurface8	*FDX8_pSurfDepthStencil;

D3DCAPS8			FDX8_Caps;
D3DSURFACE_DESC		FDX8_BackDesc;
D3DSURFACE_DESC		FDX8_DepthStencilDesc;

BOOL				FDX8_bHardwareVertexShaders;


u32					FDX8_nStageCount;
BOOL				FDX8_bDXTexStageArg0Supported;
BOOL				FDX8_bDXTexStageResultArgSupported;

u32					FDX8_nMaxD3DLights;

u32					FDX8_uCurrentVertexShader;

// Cached matrices
#if FANG_PLATFORM_XB
	XGMATRIX		FDX8_CurrentViewMtx;
	XGMATRIX		FDX8_InvCurrentViewMtx;
	XGMATRIX		FDX8_CurrentProjMtx;
#else
	D3DXMATRIX		FDX8_CurrentViewMtx;
	D3DXMATRIX		FDX8_InvCurrentViewMtx;
	D3DXMATRIX		FDX8_CurrentProjMtx;
#endif
CFMtx43A			FDX8_CurrentFangViewMtx;
CFMtx43A			FDX8_CurrentFangProjMtx;
BOOL				FDX8_bViewMtxIsIdentity;

const D3DMATRIX FDX8_MatrixIdentity = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};


DWORD FDX8_anRenderState[ FDX8_RENDERSTATE_COUNT ];
DWORD FDX8_anTextureState[ FDX8_MAX_TEX_STAGES ][ FDX8_TEXTURESTATE_COUNT ];
BOOL FDX8_abLightEnable[ FDX8_MAX_LIGHTS ];
D3DLIGHT8 FDX8_aLight[ FDX8_MAX_LIGHTS ];
D3DMATERIAL8 FDX8_Material;


#if FANG_PLATFORM_XB
	// Global xbox information about how we were launched
	DWORD			FDX8_uLaunchType = LDT_FROM_DASHBOARD;
	LAUNCH_DATA		FDX8_LaunchData;
	char			FDX8_szXBEName[ MAX_PATH+1 ];
#endif




typedef BOOL _ModuleStartupFcn( void );
typedef void _ModuleShutdownFcn( void );


static BOOL _bStartedUp;


static const struct {
	_ModuleStartupFcn *pFcnStartup;
	_ModuleShutdownFcn *pFcnShutdown;
} _aModuleArray[] = {
	// IMPORTANT: These are in a very specific order. Rearrange at your own risk!

	fres_ModuleStartup,				fres_ModuleShutdown,
	famem_ModuleStartup,			famem_ModuleShutdown,
	CFStringTable::ModuleStartup,	CFStringTable::ModuleShutdown,
	smoketrail_InitSystem,			smoketrail_UninitSystem,
	CFDebrisDef::ModuleStartup,		CFDebrisDef::ModuleShutdown,
	CFDebrisMeshSet::ModuleStartup,	CFDebrisMeshSet::ModuleShutdown,
	CFDebrisGroup::ModuleStartup,	CFDebrisGroup::ModuleShutdown,
	fsysinfo_ModuleStartup,			fsysinfo_ModuleShutdown,
	fmath_ModuleStartup,			fmath_ModuleShutdown,
	fboxfilter_ModuleStartup,		fboxfilter_ModuleShutdown,
	fcpu_ModuleStartup,				fcpu_ModuleShutdown,
	ftimer_ModuleStartup,			ftimer_ModuleShutdown,
	ffile_ModuleStartup,			ffile_ModuleShutdown,
	fresload_ModuleStartup,			fresload_ModuleShutdown,
	CFSoundGroup::ModuleStartup,	CFSoundGroup::ModuleShutdown,
	fgamedata_ModuleStartup,		fgamedata_ModuleShutdown,
	faudio_ModuleStartup,			faudio_ModuleShutdown,
	fvid_ModuleStartup,				fvid_ModuleShutdown,
	fperf_ModuleStartup,			fperf_ModuleShutdown,
	fcolor_ModuleStartup,			fcolor_ModuleShutdown,
	ftex_ModuleStartup,				ftex_ModuleShutdown,
	fviewport_ModuleStartup,		fviewport_ModuleShutdown,
	fdx8viewport_ModuleStartup,		fdx8viewport_ModuleShutdown,
	frenderer_ModuleStartup,		frenderer_ModuleShutdown,
	fxfm_ModuleStartup,				fxfm_ModuleShutdown,
	fdx8xfm_ModuleStartup,			fdx8xfm_ModuleShutdown,
	fdx8vb_ModuleStartup,			fdx8vb_ModuleShutdown,
	fsh_ModuleStartup,				fsh_ModuleShutdown,
	fdraw_ModuleStartup,			fdraw_ModuleShutdown,
	fvtxpool_ModuleStartup,			fvtxpool_ModuleShutdown,
	fdx8draw_ModuleStartup,			fdx8draw_ModuleShutdown,
	fdx8load_ModuleStartup,			fdx8load_ModuleShutdown,
	fmesh_ModuleStartup,			fmesh_ModuleShutdown,
	fdx8mesh_ModuleStartup,			fdx8mesh_ModuleShutdown,
	fpsprite_ModuleStartup,			fpsprite_ModuleShutdown,
	fworld_ModuleStartup,			fworld_ModuleShutdown,
	CFLightPool::ModuleStartup,		CFLightPool::ModuleShutdown,
	CFLightGroupMgr::ModuleStartup,	CFLightGroupMgr::ModuleShutdown,
	fanim_ModuleStartup,			fanim_ModuleShutdown,
	fcamanim_ModuleStartup,			fcamanim_ModuleShutdown,
	fcoll_ModuleStartup,			fcoll_ModuleShutdown,
//	fdev_ModuleStartup,				fdev_ModuleShutdown,
	floop_ModuleStartup,			floop_ModuleShutdown,
	fpad_ModuleStartup,				fpad_ModuleShutdown,
	fpadio_ModuleStartup,			fpadio_ModuleShutdown,
	fforce_ModuleStartup,			fforce_ModuleShutdown,
	fmovie2_ModuleStartup,			fmovie2_ModuleShutdown,
	//Shadow stuff here.
	fshadow_ModuleStartup,			fshadow_ModuleShutdown,
	fdx8shadow_ModuleStartup,		fdx8shadow_ModuleShutdown,
	ftext_ModuleStartup,			ftext_ModuleShutdown,
	CFStreamMgr::ModuleStartup,		CFStreamMgr::ModuleShutdown,
	fexception_ModuleStartup,		fexception_ModuleShutdown,
	fstorage_ModuleStartup,			fstorage_ModuleShutdown,
	CFScriptSystem::ModuleStartup,	CFScriptSystem::ModuleShutdown,
//	CFScalarObj::ModuleStartup,		CFScalarObj::ModuleShutdown,
	CFMotionObj::ModuleStartup,		CFMotionObj::ModuleShutdown,
	fcamera_ModuleStartup,			fcamera_ModuleShutdown,
	fsndfx_ModuleStartup,			fsndfx_ModuleShutdown,
	fparticle_ModuleStartup,		fparticle_ModuleShutdown,
	frs_ModuleStartup,				frs_ModuleShutdown,
	CFWire::ModuleStartup,			CFWire::ModuleShutdown,
	CFVerlet::ModuleStartup,		CFVerlet::ModuleShutdown,
	fliquid_ModuleStartup,			fliquid_ModuleShutdown,
	fexplosion_ModuleStartup,		fexplosion_ModuleShutdown,
	CFDecal::ModuleStartup,			CFDecal::ModuleShutdown,
	CFCheckPoint::ModuleStartup,	CFCheckPoint::ModuleShutdown,
	falignedpool_ModuleStartup,		falignedpool_ModuleShutdown,

// !!Nate
#if !defined(_FANGDEF_PRODUCTION_BUILD) && defined(_FANGDEF_PLATFORM_XB)
	fserver_ModuleStartup,			fserver_ModuleShutdown,
#endif

	NULL,							NULL
};



static void _ClearVariables( void );
static void _ClearStateMemory( void );



BOOL fdx8_Startup( void ) {
	s32 i;

	FASSERT( !_bStartedUp );

	_ClearVariables();
	_ClearStateMemory();

#if FANG_PLATFORM_XB
	////
	//
	XDEVICE_PREALLOC_TYPE aoInitDeviceType[] =
	{
		{ XDEVICE_TYPE_GAMEPAD,			FPADIO_MAX_DEVICES },
		{ XDEVICE_TYPE_MEMORY_UNIT,		8 }
	};
	XInitDevices( sizeof( aoInitDeviceType ) / sizeof( XDEVICE_PREALLOC_TYPE ), aoInitDeviceType );
// Normaly we should wait for the devices to become available before continuing.
// Sleep( 500 );
	//
	////
#endif

	for( i=0; _aModuleArray[i].pFcnStartup; i++ ) {
		if( _aModuleArray[i].pFcnStartup() == FALSE ) {
			for( i--; i >= 0; i-- ) {
				_aModuleArray[i].pFcnShutdown();
			}
			return FALSE;
		}
	}

	_bStartedUp = TRUE;

	return TRUE;
}

void fdx8_Shutdown( void ) {
	s32 i;

	FASSERT( _bStartedUp );

	for( i=0; _aModuleArray[i].pFcnStartup; i++ );

	for( i--; i>=0; i-- ) {
		_aModuleArray[i].pFcnShutdown();
	}

	_ClearVariables();
	_ClearStateMemory();

	_bStartedUp = FALSE;
}


//
//
//
void fdx8_InitD3DState( void ) 
{
	f32 fZero = 0.0f;
	f32 fOne = 1.0f;
	u32 i;
	D3DLIGHT8 Light;
	D3DMATERIAL8 Mtl;

	FDX8_uCurrentVertexShader = 0xffffffff;

	#define __InitRenderState( StateName, nValue )																\
		FDX8_anRenderState[ FDX8_RENDERSTATE_##StateName ] = nValue;											\
		FDX8_pDev->SetRenderState( D3DRS_##StateName, FDX8_anRenderState[ FDX8_RENDERSTATE_##StateName ] );

	#define __InitTextureState( nStage, StateName, nValue )														\
		FDX8_anTextureState[ nStage ][ FDX8_TEXTURESTATE_##StateName ] = nValue;								\
		FDX8_pDev->SetTextureStageState( nStage, D3DTSS_##StateName, FDX8_anTextureState[ nStage ][ FDX8_TEXTURESTATE_##StateName ] );

	_ClearStateMemory();

	#if FANG_PLATFORM_XB
		// Xbox supports hardware vertex shaders...
		FDX8_bHardwareVertexShaders = TRUE;
	#else
		// Assume no hardware vertex shader support...
		FDX8_bHardwareVertexShaders = FALSE;

		if( FDX8_Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) {
			// This PC video card supports hardware TnL...

			if( FDX8_Caps.VertexShaderVersion >= D3DVS_VERSION(1,0) ) {
				// And it supports hardware vertex shaders...
				FDX8_bHardwareVertexShaders = TRUE;
			}
		}
	#endif

	#if FANG_PLATFORM_XB
		// SER: Must look into why this is required for Xbox!
		FDX8_Caps.MaxTextureBlendStages=4;
	#endif
	FDX8_nStageCount = FMATH_MIN( FDX8_Caps.MaxTextureBlendStages, FDX8_MAX_TEX_STAGES );
	FDX8_bDXTexStageArg0Supported = !!(FDX8_Caps.TextureOpCaps & (D3DTEXOPCAPS_MULTIPLYADD | D3DTEXOPCAPS_LERP));
	FDX8_bDXTexStageResultArgSupported = !!(FDX8_Caps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP);

	FDX8_nMaxD3DLights = FDX8_Caps.MaxActiveLights;
	FMATH_CLAMPMAX( FDX8_nMaxD3DLights, FDX8_MAX_LIGHTS );

	// Not used by Fang...
	//FDX8_pDev->SetRenderState( D3DRS_ZENABLE,  );
	//FDX8_pDev->SetRenderState( D3DRS_LINEPATTERN,  );
	//FDX8_pDev->SetRenderState( D3DRS_LASTPIXEL,  );
	//FDX8_pDev->SetRenderState( D3DRS_ZVISIBLE,  );
	//FDX8_pDev->SetRenderState( D3DRS_EDGEANTIALIAS,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP0,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP1,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP2,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP3,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP4,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP5,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP6,  );
	//FDX8_pDev->SetRenderState( D3DRS_WRAP7,  );
	//FDX8_pDev->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE,  );

	// Fixed for all of Fang...
	FDX8_pDev->SetRenderState( D3DRS_COLORVERTEX, TRUE );
#	if FANG_PLATFORM_WIN
		FDX8_pDev->SetRenderState( D3DRS_CLIPPLANEENABLE, 0 );
#	endif

	// Controlled by other modules of Fang...
	__InitRenderState( FILLMODE, D3DFILL_SOLID );
	__InitRenderState( SHADEMODE, D3DSHADE_GOURAUD );
	__InitRenderState( ZWRITEENABLE, TRUE );
	__InitRenderState( ALPHATESTENABLE, FALSE );
	__InitRenderState( SRCBLEND, D3DBLEND_ONE );
	__InitRenderState( DESTBLEND, D3DBLEND_ZERO );
	__InitRenderState( CULLMODE, D3DCULL_CCW );
	__InitRenderState( ZFUNC, D3DCMP_LESSEQUAL );
	__InitRenderState( ALPHAREF, 0 );
	__InitRenderState( ALPHAFUNC, D3DCMP_ALWAYS );
	__InitRenderState( DITHERENABLE, FALSE );
	__InitRenderState( ALPHABLENDENABLE, FALSE );
	__InitRenderState( FOGENABLE, FALSE );
	__InitRenderState( SPECULARENABLE, FALSE );
	__InitRenderState( FOGCOLOR, 0 );
	__InitRenderState( FOGTABLEMODE, D3DFOG_NONE );
	__InitRenderState( FOGSTART, *((DWORD*)(&fZero)) );
	__InitRenderState( FOGEND, *((DWORD*)(&fZero)) );
	__InitRenderState( FOGDENSITY, *((DWORD*)(&fZero)) );
	__InitRenderState( ZBIAS, 0 );
	__InitRenderState( RANGEFOGENABLE, FALSE );
	__InitRenderState( STENCILENABLE, FALSE );
	__InitRenderState( STENCILFAIL, D3DSTENCILOP_KEEP );
	__InitRenderState( STENCILZFAIL, D3DSTENCILOP_KEEP );
	__InitRenderState( STENCILPASS, D3DSTENCILOP_KEEP );
	__InitRenderState( STENCILFUNC, D3DCMP_ALWAYS );
	__InitRenderState( STENCILREF, 0 );
	__InitRenderState( STENCILMASK, 0xffffffff );
	__InitRenderState( STENCILWRITEMASK, 0xffffffff );
	__InitRenderState( TEXTUREFACTOR, 0 );
	__InitRenderState( LIGHTING, TRUE );
	__InitRenderState( AMBIENT, 0 );
	__InitRenderState( LOCALVIEWER, TRUE );
	__InitRenderState( NORMALIZENORMALS, FALSE );
	__InitRenderState( DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
	__InitRenderState( SPECULARMATERIALSOURCE, D3DMCS_COLOR2 );
	__InitRenderState( AMBIENTMATERIALSOURCE, D3DMCS_COLOR2 );
	__InitRenderState( EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
	__InitRenderState( VERTEXBLEND, D3DVBF_DISABLE );
	__InitRenderState( POINTSIZE, *((DWORD*)(&fZero)) );
	__InitRenderState( POINTSIZE_MIN, *((DWORD*)(&fZero)) );
	__InitRenderState( POINTSPRITEENABLE, FALSE );
	__InitRenderState( POINTSCALEENABLE, FALSE );
	__InitRenderState( POINTSCALE_A, *((DWORD*)(&fZero)) );
	__InitRenderState( POINTSCALE_B, *((DWORD*)(&fZero)) );
	__InitRenderState( POINTSCALE_C, *((DWORD*)(&fZero)) );
	__InitRenderState( PATCHEDGESTYLE, D3DPATCHEDGE_CONTINUOUS );
	__InitRenderState( PATCHSEGMENTS, *((DWORD*)(&fZero)) );
	__InitRenderState( POINTSIZE_MAX, *((DWORD*)(&fOne)) );
	if( FDX8_Caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE ) {
		__InitRenderState( COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED );
	}
	__InitRenderState( BLENDOP, D3DBLENDOP_ADD );
#	if FANG_PLATFORM_WIN
		__InitRenderState( CLIPPING, TRUE );
		__InitRenderState( FOGVERTEXMODE, D3DFOG_NONE );
		__InitRenderState( TWEENFACTOR, *((DWORD*)(&fZero)) );
#	endif

	for( i=0; i<FDX8_nStageCount; i++ ) {
		__InitTextureState( i, COLOROP, D3DTOP_DISABLE );
		__InitTextureState( i, COLORARG1, D3DTA_TEXTURE );
		__InitTextureState( i, COLORARG2, D3DTA_CURRENT );
		__InitTextureState( i, ALPHAOP, D3DTOP_SELECTARG1 );
		__InitTextureState( i, ALPHAARG1, D3DTA_TEXTURE );
		__InitTextureState( i, ALPHAARG2, D3DTA_CURRENT );
		__InitTextureState( i, BUMPENVMAT00, 0 );
		__InitTextureState( i, BUMPENVMAT01, 0 );
		__InitTextureState( i, BUMPENVMAT10, 0 );
		__InitTextureState( i, BUMPENVMAT11, 0 );
		__InitTextureState( i, BUMPENVLSCALE, 0 );
		__InitTextureState( i, BUMPENVLOFFSET, 0 );
		__InitTextureState( i, TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );

		FDX8_anTextureState[ i ][ FDX8_TEXTURESTATE_COLORARG0 ] = D3DTA_CURRENT;
		FDX8_anTextureState[ i ][ FDX8_TEXTURESTATE_ALPHAARG0 ] = D3DTA_CURRENT;
		if( FDX8_bDXTexStageArg0Supported ) {
			FDX8_pDev->SetTextureStageState( i, D3DTSS_COLORARG0, FDX8_anTextureState[ i ][ FDX8_TEXTURESTATE_COLORARG0 ] );
			FDX8_pDev->SetTextureStageState( i, D3DTSS_ALPHAARG0, FDX8_anTextureState[ i ][ FDX8_TEXTURESTATE_ALPHAARG0 ] );
		}

		FDX8_anTextureState[ i ][ FDX8_TEXTURESTATE_RESULTARG ] = D3DTA_CURRENT;
		if( FDX8_bDXTexStageResultArgSupported ) {
			FDX8_pDev->SetTextureStageState( i, D3DTSS_RESULTARG, FDX8_anTextureState[ i ][ FDX8_TEXTURESTATE_RESULTARG ] );
		}

	}

	__InitTextureState( 0, COLOROP, D3DTOP_MODULATE );

	fang_MemZero( &Light, sizeof(Light) );
	Light.Type = D3DLIGHT_DIRECTIONAL;
	Light.Direction.x = 1.0f;
	Light.Range = 1.0f;
	Light.Falloff = 1.0f;
	Light.Attenuation0 = 1.0f;
	Light.Attenuation1 = 1.0f;
	Light.Attenuation2 = 1.0f;
	Light.Theta = 0.0f;
	Light.Phi = FMATH_HALF_PI;

	for( i=0; i<FDX8_MAX_LIGHTS; i++ ) {
		FDX8_abLightEnable[i] = FALSE;
		fang_MemCopy( &FDX8_aLight[i], &Light, sizeof(Light) );

		FDX8_pDev->LightEnable( i, FDX8_abLightEnable[i] );
		FDX8_pDev->SetLight( i, &FDX8_aLight[i] );
	}

	fang_MemZero( &Mtl, sizeof(Mtl) );
	fang_MemCopy( &FDX8_Material, &Mtl, sizeof(Mtl) );
	FDX8_pDev->SetMaterial( &FDX8_Material );

	#undef __InitTextureState
	#undef __InitRenderState
}


static void _ClearVariables( void ) {
	FDX8_pD3D = NULL;
	FDX8_pDev = NULL;
	FDX8_pSurfBack = NULL;
	FDX8_pSurfDepthStencil = NULL;

	fang_MemZero( &FDX8_Caps, sizeof(FDX8_Caps) );
	fang_MemZero( &FDX8_BackDesc, sizeof(FDX8_BackDesc) );
}

static void _ClearStateMemory( void ) {
	u32 i, j;

	FDX8_bDXTexStageArg0Supported = FALSE;
	FDX8_bDXTexStageResultArgSupported = FALSE;

	FDX8_CurrentViewMtx = FDX8_MatrixIdentity;
	FDX8_InvCurrentViewMtx = FDX8_MatrixIdentity;
	FDX8_CurrentProjMtx = FDX8_MatrixIdentity;
	FDX8_CurrentFangViewMtx.Identity();
	FDX8_CurrentFangProjMtx.Identity();
	FDX8_bViewMtxIsIdentity = FALSE;

	for( i=0; i<FDX8_RENDERSTATE_COUNT; i++ ) {
		FDX8_anRenderState[i] = 0;
	}

	for( i=0; i<FDX8_MAX_TEX_STAGES; i++ ) {
		for( j=0; j<FDX8_TEXTURESTATE_COUNT; j++ ) {
			FDX8_anTextureState[i][j] = 0;
		}
	}

	for( i=0; i<FDX8_MAX_LIGHTS; i++ ) {
		FDX8_abLightEnable[i] = FALSE;
		fang_MemZero( &FDX8_aLight[i], sizeof(D3DLIGHT8) );
	}

	fang_MemZero( &FDX8_Material, sizeof(FDX8_Material) );
}

