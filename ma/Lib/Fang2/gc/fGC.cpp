//////////////////////////////////////////////////////////////////////////////////////
// fGC.cpp - Master GameCube Fang module.
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
// 02/18/02 Lafleur		Created from stubbed DX version
//////////////////////////////////////////////////////////////////////////////////////

// Platform Includes
#include "fGC.h"
#include "fGCvid.h"
#include "fGCloop.h"
#include "fGCtex.h"
#include "fGCviewport.h"
#include "fGCxfm.h"
#include "fGCdraw.h"
#include "fGCmesh.h"
#include "fGCload.h"
#include "fGCshadow.h"
#include "fGCvb.h"
#include "fGCdata.h"

#include "fsh.h"
#include "fmovie2.h"
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
#include "fdev.h"
#include "fpsprite.h"
#include "fshadow.h"
#include "faudio.h"
#include "fpart.h"
#include "fstorage.h"
#include "fforce.h"
#include "fstringtable.h"
#include "fscriptsystem.h"
#include "fmotionobj.h"
#include "fcamera.h"
#include "fsndfx.h"
#include "fparticle.h"
#include "fRenderSort.h"
#include "fAMem.h"
#include "fdebris.h"
#include "fverlet.h"
#include "fliquid.h"
#include "smoketrail.h"
#include "fvtxpool.h"
#include "fexplosion.h"
#include "FCheckPoint.h"
#include "fsound.h"
#include "flightgroup.h"
#include "flightpool.h"
#include "falignedpool.h"
#include "fwire.h"
#include "fdatastreaming.h"
#include "fdecal.h"

// GameCube Includes
#include "dvd.h"
#include "os.h"
#include "vi.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#if FANGGC_BENCHMARK_BUILD
	#define _ENABLE_SOUND		FALSE
#else
	#define _ENABLE_SOUND		TRUE
#endif

typedef BOOL _ModuleStartupFcn( void );
typedef void _ModuleShutdownFcn( void );


//////////////////////////////////////////////////////////////////////////////////////
// Global Variables:
//////////////////////////////////////////////////////////////////////////////////////

// Memory heap handle
OSHeapHandle FGC_HeapHandle = -1;

GXCullMode 	FGC_CullMode;
GXBool		FGC_ClipMode;
GXBool 		FGC_bDither;
GXBool 		FGC_bZComparePriorToTEV;
GXBool 		FGC_bColorBufferUpdate;

// Zbuffer cache
GXBool 		FGC_bZRead;
GXCompare 	FGC_ZCompareMode;
GXBool 		FGC_bZWrite;

// Blend Op cache
GXBlendMode		FGC_BlendMode;
GXBlendFactor	FGC_SourceFactor;
GXBlendFactor	FGC_DestFactor;
GXLogicOp		FGC_BitLogicOp;

// Alpha Compare cache
GXCompare 		FGC_AlphaCompare1;
u8 				FGC_nAlphaCompareRef1;
GXAlphaOp 		FGC_AlphaCompareOp;
GXCompare 		FGC_AlphaCompare2;
u8 				FGC_nAlphaCompareRef2;

// Vertex Format cache
GXAttrType 		FGC_CurrPosIdxType;
GXCompType 		FGC_CurrPosType[GX_MAX_VTXFMT];
u8 				FGC_CurrPosFrac[GX_MAX_VTXFMT];
GXAttrType 		FGC_CurrNrmIdxType;
GXAttrType 		FGC_CurrNBTIdxType;
GXCompType		FGC_CurrNrmType[GX_MAX_VTXFMT];
GXCompCnt		FGC_CurrNrmCompCnt[GX_MAX_VTXFMT];
u8 				FGC_CurrNrmFrac[GX_MAX_VTXFMT];
GXAttrType		FGC_CurrSTIdxType[8];
GXCompType		FGC_SetVtxSTType[GX_MAX_VTXFMT][8];
u8				FGC_SetVtxSTFrac[GX_MAX_VTXFMT][8];
GXAttrType		FGC_CurrClrIdxType[2];


// Channel count
u32				FGC_CurrChannelCount;

// Flag that indicates that a GameCube GPU lazy register setting has been made
BOOL8		FGC_bLazyRegisterSet = TRUE;

u32			FGC_nStageCount;

u32			FGC_nMaxLights;

BOOL		FGC_bMemoryInitialized = FALSE;
BOOL		FGC_DEVKitInExtendedMemoryMode = FALSE;
BOOL 		FGC_FlipCullMode = FALSE;



Mtx44 FGC_Mtx44Identity = 
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};


Mtx FGC_MtxIdentity = 
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
};


Mtx FGC_MtxLeftToRightIdentity =
{
	1.f,	0.f,	0.f,	0.f,
	0.f,	1.f,	0.f,	0.f,
	0.f,	0.f,	-1.f,	0.f,
};


//////////////////////////////////////////////////////////////////////////////////////
// Local Variables:
//////////////////////////////////////////////////////////////////////////////////////

static BOOL _bStartedUp = FALSE;

static u32 _auAramBlocks[FGC_MAX_NUM_ARAM_BLOCKS];

static const struct 
{
	_ModuleStartupFcn *pFcnStartup;
	_ModuleShutdownFcn *pFcnShutdown;
} _aModuleArray[] = 
{
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
#if _ENABLE_SOUND
	CFSoundGroup::ModuleStartup,	CFSoundGroup::ModuleShutdown,
#endif
	fgamedata_ModuleStartup,		fgamedata_ModuleShutdown,
#if _ENABLE_SOUND
	faudio_ModuleStartup,			faudio_ModuleShutdown,
#endif
	fvid_ModuleStartup,				fvid_ModuleShutdown,
	fperf_ModuleStartup,			fperf_ModuleShutdown,
	fcolor_ModuleStartup,			fcolor_ModuleShutdown,
	ftex_ModuleStartup,				ftex_ModuleShutdown,
	fviewport_ModuleStartup,		fviewport_ModuleShutdown,
	fgcviewport_ModuleStartup,		fgcviewport_ModuleShutdown,
	frenderer_ModuleStartup,		frenderer_ModuleShutdown,
	fxfm_ModuleStartup,				fxfm_ModuleShutdown,
	fgcxfm_ModuleStartup,			fgcxfm_ModuleShutdown,
	fgcvb_ModuleStartup,			fgcvb_ModuleShutdown,
	fsh_ModuleStartup,				fsh_ModuleShutdown,
	fdraw_ModuleStartup,			fdraw_ModuleShutdown,
	fvtxpool_ModuleStartup,			fvtxpool_ModuleShutdown,
	fgcdraw_ModuleStartup,			fgcdraw_ModuleShutdown,
	fgcload_ModuleStartup,			fgcload_ModuleShutdown,
	fmesh_ModuleStartup,			fmesh_ModuleShutdown,
	fgcmesh_ModuleStartup,			fgcmesh_ModuleShutdown,
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
	fshadow_ModuleStartup,			fshadow_ModuleShutdown,
	fgcshadow_ModuleStartup,		fgcshadow_ModuleShutdown,
	ftext_ModuleStartup,			ftext_ModuleShutdown,
	CFStreamMgr::ModuleStartup,		CFStreamMgr::ModuleShutdown,
	fstorage_ModuleStartup,			fstorage_ModuleShutdown,
	CFScriptSystem::ModuleStartup,	CFScriptSystem::ModuleShutdown,
	CFMotionObj::ModuleStartup,		CFMotionObj::ModuleShutdown,
	fcamera_ModuleStartup,			fcamera_ModuleShutdown,
#if _ENABLE_SOUND
	fsndfx_ModuleStartup,			fsndfx_ModuleShutdown,
#endif
	fparticle_ModuleStartup,		fparticle_ModuleShutdown,
	frs_ModuleStartup,				frs_ModuleShutdown,
	CFWire::ModuleStartup,			CFWire::ModuleShutdown,
	CFVerlet::ModuleStartup,		CFVerlet::ModuleShutdown,
	fliquid_ModuleStartup,			fliquid_ModuleShutdown,
	fexplosion_ModuleStartup,		fexplosion_ModuleShutdown,
	CFDecal::ModuleStartup,			CFDecal::ModuleShutdown,
	CFCheckPoint::ModuleStartup,	CFCheckPoint::ModuleShutdown,
	falignedpool_ModuleStartup,		falignedpool_ModuleShutdown,
	
	NULL,							NULL
};


//////////////////////////////////////////////////////////////////////////////////////
// Static Functions:
//////////////////////////////////////////////////////////////////////////////////////

static void _ClearVariables( void );
static void _ClearStateMemory( void );


//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
void fgc_Init( void )
{
	OSInit();

	VIInit();

	//// Required by Musyx.
	//
	ARInit( _auAramBlocks, FGC_MAX_NUM_ARAM_BLOCKS );
	ARQInit();
	//
	////

}


//
//
//
BOOL fgc_Startup( void ) 
{
	s32 i;
	
	FASSERT( !_bStartedUp );

	fgc_Init();

	_ClearVariables();
	_ClearStateMemory();

	fgc_InitializeMemory();
	
	if ( FGC_DEVKitInExtendedMemoryMode )
	{
		Fang_ConfigDefs.nRes_HeapBytes += (8 * 1024 * 1024); // Add 8MB's on if the dev kit is in extended memory mode
	}		

	for( i = 0; _aModuleArray[i].pFcnStartup; i++ ) 
	{
		if( _aModuleArray[i].pFcnStartup() == FALSE ) 
		{
			for( i--; i >= 0; i-- ) 
			{
				_aModuleArray[i].pFcnShutdown();
			}
			return FALSE;
		}
	}

	_bStartedUp = TRUE;

	return TRUE;
}


//
//
//
void fgc_Shutdown( void ) 
{
	s32 i;

	FASSERT( _bStartedUp );

	for( i = 0; _aModuleArray[i].pFcnStartup; i++ )
	{
	}

	for( i--; i >= 0; i-- ) 
	{
		_aModuleArray[i].pFcnShutdown();
	}

	_ClearVariables();
	_ClearStateMemory();

	_bStartedUp = FALSE;
}



//
//
//
void fgc_InitRenderState( void ) 
{
	u32 i, ii;
	
	FGC_nStageCount = 8;
	FGC_nMaxLights = 6;
	
	// Set default z buffer modes
	FGC_bZRead = GX_TRUE;
	FGC_ZCompareMode = GX_LEQUAL;
	FGC_bZWrite = GX_TRUE;
	GXSetZMode( FGC_bZRead, FGC_ZCompareMode, FGC_bZWrite );
	
	// Set default culling (note that it is front culling
	// for GameCube because we are converting into a right-
	// handed system)
	FGC_CullMode = GX_CULL_FRONT;
	GXSetCullMode( FGC_CullMode );
	
	// Set default dithering
	FGC_bDither = GX_FALSE;
	GXSetDither( FGC_bDither );
	
	// Set Default clipping - GC Clip mode is enable = 0, disable = 1 - screwy
	FGC_ClipMode = GX_TRUE;
	GXSetClipMode( (GXClipMode)!FGC_ClipMode );
	
	// Z Compare can occur prior to or after texture stage.  But,
	// if Z Compare occurs prior to texture stage, Z values are
	// written to the z buffer without regard to alpha in the
	// texture.  So, if there is alpha in the texture, we do not
	// want to do the Z compare first.	
	FGC_bZComparePriorToTEV = GX_FALSE;
	GXSetZCompLoc( FGC_bZComparePriorToTEV );
	
	// Set default color update
	FGC_bColorBufferUpdate = GX_TRUE;
	GXSetColorUpdate( FGC_bColorBufferUpdate );

	// Set Default blend op	
	FGC_BlendMode = GX_BM_BLEND;
	FGC_SourceFactor = GX_BL_SRCALPHA;
	FGC_DestFactor = GX_BL_INVSRCALPHA;
	FGC_BitLogicOp = GX_LO_NOOP;
	GXSetBlendMode( FGC_BlendMode, FGC_SourceFactor, FGC_DestFactor, FGC_BitLogicOp );

	// Set default alpha compare mode
	FGC_AlphaCompare1 = GX_ALWAYS;
	FGC_nAlphaCompareRef1 = 0;
	FGC_AlphaCompareOp = GX_AOP_AND;
	FGC_AlphaCompare2 = GX_ALWAYS;
	FGC_nAlphaCompareRef2 = 0;
	GXSetAlphaCompare( FGC_AlphaCompare1, FGC_nAlphaCompareRef1, FGC_AlphaCompareOp, FGC_AlphaCompare2, FGC_nAlphaCompareRef2 );

	FGC_CurrPosIdxType = GX_DIRECT;
	GXSetVtxDesc( GX_VA_POS, FGC_CurrPosIdxType );
	
	for ( i = 0; i < GX_MAX_VTXFMT; i++ )
	{
		FGC_CurrPosType[i] = GX_F32;
		FGC_CurrPosFrac[i] = 0;
		GXSetVtxAttrFmt( (GXVtxFmt)i, GX_VA_POS, GX_POS_XYZ, FGC_CurrPosType[i], FGC_CurrPosFrac[i] );
	}
	
	FGC_CurrNrmIdxType = GX_DIRECT;
	GXSetVtxDesc( GX_VA_NRM, FGC_CurrNrmIdxType );
	for ( i = 0; i < GX_MAX_VTXFMT; i++ )
	{
		FGC_CurrNrmType[i] = GX_F32;
		FGC_CurrNrmFrac[i] = 0;
		FGC_CurrNrmCompCnt[i] = GX_NRM_XYZ;
		GXSetVtxAttrFmt( (GXVtxFmt)i, GX_VA_NRM, FGC_CurrNrmCompCnt[i], FGC_CurrNrmType[i], FGC_CurrNrmFrac[i] );
	}
	
	FGC_CurrNBTIdxType = GX_NONE;
	GXSetVtxDesc( GX_VA_NBT, FGC_CurrNBTIdxType );
	
	FGC_CurrClrIdxType[0] = GX_DIRECT;
	GXSetVtxDesc( GX_VA_CLR0, FGC_CurrClrIdxType[0] );
	
	FGC_CurrClrIdxType[1] = GX_DIRECT;
	GXSetVtxDesc( GX_VA_CLR1, FGC_CurrClrIdxType[1] );
	
	for ( i = 0; i < 8; i++ )
	{
		FGC_CurrSTIdxType[i] = GX_DIRECT;
		GXSetVtxDesc( (GXAttr)(GX_VA_TEX0 + i), FGC_CurrSTIdxType[i] );

		for ( ii = 0; ii < GX_MAX_VTXFMT; ii++ )
		{
			FGC_SetVtxSTType[ii][i] = GX_F32;
			FGC_SetVtxSTFrac[ii][i] = 0;
			GXSetVtxAttrFmt( (GXVtxFmt)i, (GXAttr)(GX_VA_TEX0 + i), GX_NRM_XYZ, FGC_SetVtxSTType[ii][i], FGC_SetVtxSTFrac[ii][i] );
		}
	}
	
	FGC_CurrChannelCount = 1;
	GXSetNumChans( FGC_CurrChannelCount );
}

#define _DISPLAY_GCHEAP_STATUS	(TRUE & FANG_ENABLE_DEV_FEATURES)

//
//
//
void fgc_InitializeMemory( void ) 
{
	void *pArenaLo;
	void *pArenaHi;
	
	if ( FGC_bMemoryInitialized )
	{
		return;
	}

	#if _DISPLAY_GCHEAP_STATUS
		OSReport( "Initializing GameCube Memory \n" );
	#endif
	
	pArenaLo = OSGetArenaLo();
	pArenaHi = OSGetArenaHi();
	
	if ( (u32)pArenaHi - (u32)pArenaLo == 0 )
	{
		#if _DISPLAY_GCHEAP_STATUS
			OSReport( "fgc_InitializeMemory() - FATAL ERROR!  No heap available!!!\n" );
		#endif
		return;
	}
	
	#if _DISPLAY_GCHEAP_STATUS
		OSReport( "fgc_InitializeMemory() - GC Free Memory at Startup = %d\n", (u32)pArenaHi - (u32)pArenaLo );
	#endif
	
	if ( (u32)pArenaHi - (u32)pArenaLo > (32 * (1024 * 1024)) )
	{
		FGC_DEVKitInExtendedMemoryMode = TRUE;
	}
	
	// While we're doing memory allocations, let's allocate the
	// frame buffers so that they are in contiguous space at the
	// bottom of the arena.
	FGCVid_nFrameBufferSize = VIPadFrameBufferWidth(640) * 480 * (u32)VI_DISPLAY_PIX_SZ;
	FGCVid_pFrameBuffer1 	= (void *)OSRoundUp32B((u32)pArenaLo);
	FGCVid_pFrameBuffer2 	= (void *)OSRoundUp32B((u32)FGCVid_pFrameBuffer1 + FGCVid_nFrameBufferSize);
	FGCVid_pCurrentBuffer 	= FGCVid_pFrameBuffer2;
	
	// Allocate the Graphics FIFO
	FGCVid_nGraphicsFifoSize = 224 * 1024;
	FGCVid_pGraphicsFifo	= (void *)OSRoundUp32B((u32)FGCVid_pFrameBuffer2 + FGCVid_nFrameBufferSize);

	// Need to reset the arena lo based on the the frame buffer allocations
    pArenaLo = (void*)OSRoundUp32B((u32)FGCVid_pGraphicsFifo + FGCVid_nGraphicsFifoSize);
    OSSetArenaLo( pArenaLo );
	
	// Create a heap based on the memory now available.
	pArenaLo = OSGetArenaLo();
	pArenaHi = OSGetArenaHi();
	pArenaLo = OSInitAlloc( pArenaLo, pArenaHi, 1 );
	OSSetArenaLo( pArenaLo );
	
	// Ensure that the arena boundaries are 32B aligned
	pArenaLo = (void *)OSRoundUp32B( pArenaLo );
	pArenaHi = (void *)OSRoundDown32B( pArenaHi );
	
	FGC_HeapHandle = OSCreateHeap( pArenaLo, pArenaHi );
	if ( FGC_HeapHandle == -1 )
	{
		#if _DISPLAY_GCHEAP_STATUS
			OSReport( "fgc_InitializeMemory() - Heap creation failed.\n" );
		#endif
	}
	else
	{
		// Set up the current heap
		OSSetCurrentHeap( FGC_HeapHandle );
	}
	
	#if _DISPLAY_GCHEAP_STATUS
		OSReport( "fgc_InitializeMemory() - Memory allocated for double-buffered frames = %d\n", FGCVid_nFrameBufferSize * 2 );
		OSReport( "fgc_InitializeMemory() - Memory allocated for Graphics FIFO = %d\n", FGCVid_nGraphicsFifoSize );
		OSReport( "fgc_InitializeMemory() - Resulting GC Memory Heap Size = %d\n", (u32)pArenaHi - (u32)pArenaLo );
	#endif
	
	// From here on out, we should use OSAlloc() and OSFree()
	// instead of malloc() and free()
	OSSetArenaLo( pArenaLo = pArenaHi );
	
	// We're done, so set the flag
	FGC_bMemoryInitialized = TRUE;
}


//
//
//
static void _ClearVariables( void ) 
{
}


//
//
//
static void _ClearStateMemory( void ) 
{
/*
	u32 i, j;

	FDX8_bDXTexStageArg0Supported = FALSE;
	FDX8_bDXTexStageResultArgSupported = FALSE;

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
*/
}

BOOL8 FGC_bUseVertexColor = FALSE;

static BOOL8 _bUsingChannel2 = FALSE;
static BOOL8 _bUsingChannel2ForSpec = FALSE;
static u8    _nLastLightCount = 255;
static u8    _nLastShadowCount = 255;
static u8    _nLastPerPixelSpotCount = 255;

GXColor FGC_gxWhite = { 0xff, 0xff, 0xff, 0xff };
GXColor FGC_gxBlack = { 0, 0, 0, 0 };


enum
{
	_CHANNEL_STATE_NONE = 0,
	_CHANNEL_STATE_SHADER,
	_CHANNEL_STATE_FDRAW,
	_CHANNEL_STATE_POINTSPRITE,
	_CHANNEL_STATE_SHADOWS,
	_CHANNEL_STATE_PERPIXEL_SPOT,
};

static u8    _nCurrentChannelState = _CHANNEL_STATE_NONE;

//
//
//
BOOL fgc_SetChannelsForShaders( BOOL8 bUseVertexColor, u8 nLightCount, BOOL8 bUseChannel2, BOOL8 bSpecular )
{
	if ( nLightCount > FGC_nMaxLights)
	{
		nLightCount = FGC_nMaxLights;
	}
	
	BOOL bRefreshStates = (_nCurrentChannelState != _CHANNEL_STATE_SHADER);
	
	// Set the number of channels to be used
	fgc_SetNumChannels( !!bUseChannel2 + 1 );
	if ( FGC_bUseVertexColor != bUseVertexColor || _nLastLightCount != nLightCount || bRefreshStates )
	{
		if ( bUseVertexColor )
		{ 
			GXSetChanCtrl(	GX_COLOR0,
							GX_ENABLE,		// enable Channel
							GX_SRC_VTX,		// amb source
							GX_SRC_REG,		// mat source
							(1 << nLightCount) - 1,		// light mask
							GX_DF_CLAMP,	// diffuse function
							GX_AF_SPOT );	// atten   function
		}
		else
		{
			GXSetChanCtrl(	GX_COLOR0,
							GX_ENABLE,		// enable Channel
							GX_SRC_REG,		// amb source
							GX_SRC_REG,		// mat source
							(1 << nLightCount) - 1,		// light mask
							GX_DF_CLAMP,	// diffuse function
							GX_AF_SPOT );	// atten   function
		}
		
		FGC_bUseVertexColor = bUseVertexColor;
		_nLastLightCount = nLightCount;
	}
	
	if ( bUseChannel2 )
	{
		GXSetChanCtrl( GX_COLOR1, GX_ENABLE,
					   GX_SRC_REG, GX_SRC_REG,
					   GX_LIGHT7, GX_DF_SIGN, GX_AF_SPOT );
		GXSetChanCtrl( GX_ALPHA1, GX_ENABLE,
					   GX_SRC_REG, GX_SRC_REG,
					   GX_LIGHT6, GX_DF_NONE, GX_AF_SPEC );
					   
			
		if ( bSpecular )// && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SMOTIF] && FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP])
		{
			CFLight *pDirLight = fvis_GetCurrentDirectional();
			GXLightObj gxLight;
			
			f32 fPwr;
			if (FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP])
			{
				fPwr = *((f32 *)&FSh_pnLightInputRegisters[FSHADERS_LIGHT_REG_SEXP]);
				fPwr *= 100.0f;
			}
			else
			{
				fPwr = 32.0f;
			}
			
			GXInitLightColor( &gxLight, FGC_gxWhite );

			if ( pDirLight->m_nXfmKey_VS != FXfm_nViewKey)
			{
				pDirLight->m_vUnitDir_VS = FGCXfm_mtxGCLeftToRightHandViewMtx.m44.MultDir( pDirLight->m_mtxOrientation_WS.m_vFront );
			}
			pDirLight->m_nXfmKey_VS = FXfm_nViewKey;
			
			GXInitSpecularDir(&gxLight, pDirLight->m_vUnitDir_VS.x, pDirLight->m_vUnitDir_VS.y, pDirLight->m_vUnitDir_VS.z);
			GXInitLightShininess(&gxLight, fPwr);
			
			GXLoadLightObjImm(&gxLight, GX_LIGHT6);
			
			_bUsingChannel2ForSpec = TRUE;
		}
		else if ( _bUsingChannel2ForSpec || bRefreshStates )
		{
			GXLightObj gxLight;
			GXInitSpecularDir( &gxLight, 0, 1, 0 );
			GXInitLightShininess( &gxLight, 1 );
			
			GXLoadLightObjImm( &gxLight, GX_LIGHT6 );
			
			_bUsingChannel2ForSpec = FALSE;
		}

		if ( !_bUsingChannel2 || bRefreshStates )
		{				   
			GXSetChanAmbColor( GX_COLOR1A1, FGC_gxBlack );
			GXSetChanMatColor( GX_COLOR1A1, FGC_gxWhite );
		}
		
		_bUsingChannel2 = TRUE;
	}
	else if ( _bUsingChannel2 || bRefreshStates )
	{
		GXSetChanCtrl( GX_COLOR1, GX_DISABLE,
					   GX_SRC_REG, GX_SRC_REG,
					   GX_LIGHT_NULL, GX_DF_SIGN, GX_AF_SPOT );
		GXSetChanCtrl( GX_ALPHA1, GX_DISABLE,
					   GX_SRC_REG, GX_SRC_REG,
					   GX_LIGHT_NULL, GX_DF_NONE, GX_AF_SPEC );
					   
	   _bUsingChannel2 = FALSE;
	}
	
	_nCurrentChannelState = _CHANNEL_STATE_SHADER;
	
	return TRUE;
}


//
//
//
BOOL fgc_SetChannelsForFDraw( void )
{
	if ( _nCurrentChannelState != _CHANNEL_STATE_FDRAW )
	{
		// Set rendering mode
		fgc_SetNumChannels( 1 );				// # of color channels
		
		GXSetChanCtrl(	GX_COLOR0A0,
						GX_ENABLE,		// enable Channel
						GX_SRC_VTX,		// amb source
						GX_SRC_REG,		// mat source
						GX_LIGHT_NULL,	// light mask
						GX_DF_NONE,		// diffuse function
						GX_AF_NONE  );	// atten   function
						
		GXSetChanMatColor( GX_COLOR0A0, FGC_gxWhite );
	    
		_nCurrentChannelState = _CHANNEL_STATE_FDRAW;
	}
	
    return TRUE;
}


//
//
//
BOOL fgc_SetChannelsForPointSprites( void )
{
	if ( _nCurrentChannelState != _CHANNEL_STATE_POINTSPRITE )
	{
		// Set rendering mode
		fgc_SetNumChannels( 1 );				// # of color channels
		
		GXSetChanCtrl(	GX_COLOR0A0,
						GX_ENABLE,		// enable Channel
						GX_SRC_VTX,		// amb source
						GX_SRC_REG,		// mat source
						GX_LIGHT_NULL,	// light mask
						GX_DF_CLAMP,	// diffuse function
						GX_AF_NONE  );	// atten   function
						
		_nCurrentChannelState = _CHANNEL_STATE_POINTSPRITE;
	}
	
	return TRUE;
}


//
//
//
BOOL fgc_SetChannelsForShadows( u32 nShadowCount )
{
	if ( _nCurrentChannelState != _CHANNEL_STATE_SHADOWS || nShadowCount > _nLastShadowCount )
	{
		if ( nShadowCount > 0 )
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
		if ( nShadowCount > 1 )
		{
			GXSetChanCtrl(	GX_ALPHA0,
							GX_ENABLE,		// enable Channel
							GX_SRC_REG,		// amb source
							GX_SRC_REG,		// mat source
							GX_LIGHT1,		// light mask
							GX_DF_CLAMP,	// diffuse function
							GX_AF_SPOT );	// atten   function
		}
		if ( nShadowCount > 2 )
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
			
			fgc_SetNumChannels( 2 );
		}
		else
		{
			fgc_SetNumChannels( 1 );
		}
		
		if ( nShadowCount > 3 )
		{
			GXSetChanCtrl(	GX_ALPHA1,
							GX_ENABLE,		// enable Channel
							GX_SRC_REG,		// amb source
							GX_SRC_REG,		// mat source
							GX_LIGHT3,		// light mask
							GX_DF_CLAMP,	// diffuse function
							GX_AF_SPOT );	// atten   function
		}
		
		_nLastShadowCount = nShadowCount;				
		_nCurrentChannelState = _CHANNEL_STATE_SHADOWS;
	}

	return TRUE;
}

//
//
//
BOOL fgc_SetChannelsForPerPixelSpot( u32 nPerPixelSpotCount )
{
	if ( _nCurrentChannelState != _CHANNEL_STATE_PERPIXEL_SPOT || nPerPixelSpotCount > _nLastPerPixelSpotCount )
	{
		if ( nPerPixelSpotCount > 0 )
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
		if ( nPerPixelSpotCount > 1 )
		{
			GXSetChanCtrl(	GX_ALPHA0,
							GX_ENABLE,		// enable Channel
							GX_SRC_REG,		// amb source
							GX_SRC_REG,		// mat source
							GX_LIGHT1,		// light mask
							GX_DF_CLAMP,	// diffuse function
							GX_AF_SPOT );	// atten   function
		}
		if ( nPerPixelSpotCount > 2 )
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
			
			fgc_SetNumChannels( 2 );
		}
		else
		{
			fgc_SetNumChannels( 1 );
		}
		
		if ( nPerPixelSpotCount > 3 )
		{
			GXSetChanCtrl(	GX_ALPHA1,
							GX_ENABLE,		// enable Channel
							GX_SRC_REG,		// amb source
							GX_SRC_REG,		// mat source
							GX_LIGHT3,		// light mask
							GX_DF_CLAMP,	// diffuse function
							GX_AF_SPOT );	// atten   function
		}
		
		_nLastPerPixelSpotCount = nPerPixelSpotCount;				
		_nCurrentChannelState = _CHANNEL_STATE_PERPIXEL_SPOT;
	}

	return TRUE;
}


//
//
//
BOOL fgc_DisableChannels( void )
{
	fgc_SetNumChannels( 1 );				// # of color channels
	GXSetChanCtrl(	GX_COLOR0A0,
					GX_FALSE,		// enable Channel
					GX_SRC_VTX,		// amb source
					GX_SRC_VTX,		// mat source
					GX_LIGHT_NULL,	// light mask
					GX_DF_NONE,		// diffuse function
					GX_AF_NONE );	// atten   function
					
	_nCurrentChannelState = _CHANNEL_STATE_NONE;

	return TRUE;
}
						
