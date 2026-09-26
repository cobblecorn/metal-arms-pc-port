//////////////////////////////////////////////////////////////////////////////////////
// fGCtex.cpp - Fang texture module (GameCube version).
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
#include "fshaders.h"

#include "fGC.h"
#include "fGCtex.h"
#include "fGCvid.h"
#include "fGCviewport.h"
#include "fGCsh.h"

#include "fmath.h"
#include "fclib.h"
#include "fdata.h"
#include "fres.h"
#include "fresload.h"
#include "flinklist.h"
#include "fperf.h"
#include "fvid.h"
#include "fvis.h"
#include "fdatastreaming.h"

#include "floop.h"


//////////////////////////////////////////////////////////////////////////////////////
// Local Defines:
//////////////////////////////////////////////////////////////////////////////////////

#define _COMPRESSED_MIN_DIMENSION		8
#define _UNCOMPRESSED_MIN_DIMENSION		4
#define _MAX_RENDERTARGETS 				20


//////////////////////////////////////////////////////////////////////////////////////
// Global Variables:
//////////////////////////////////////////////////////////////////////////////////////

char FTex_szNewReflectMap[16];
char FTex_szOldReflectMap[16];

u32 FTex_nMaxTextureWidth;		// Maximum texture width allowed (for largest LOD)
u32 FTex_nMaxTextureHeight;		// Maximum texture height allowed (for largest LOD)
u32 FTex_nMaxAspect;			// Maximum allowed aspect ratio (width/height or height/width)

BOOL FTex_bMipmaps;				// TRUE=mipmaps are supported
BOOL FTex_bRenderTargets;		// TRUE=render targets are supported
BOOL FTex_bDynamicTextures;		// TRUE=dynamic textures are supported
BOOL FTex_bDynamicsCanBeLost;	// TRUE=dynamic textures can be lost on this platform (FALSE on PS2 and XB)

BOOL FTex_bAllowRenderTargetClear=TRUE;

f32 fgctex_fCopyMul=1.0f;


//////////////////////////////////////////////////////////////////////////////////////
// Local Structures:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
struct _RenderTarget
{
	CFTexInst *pTexInst;
	CFTexInst *pAlphaTex;
	ftex_RenderTarget_Callback pCallback;
	f32 fOOMaxRenderFreq;

	BOOL8 bAutoClearRGBA;
	BOOL8 bAutoClearDepth;
	BOOL8 bActive;
	BOOL8 bAllowLiquidRender;
	BOOL8 bStagger;
	BOOL8 bRenderWorld;
	u8 nPad8[2];
	
	f32 fCurrentRenderTime;
	
	void *pUserData;
};

//
//
//
struct _TexelInfo_t 
{
	FTexFmt_e nTexFmt;		// Fang texel format

	u8 nBitsPerTexel;		// Number of bits per texel
	u8 nAlphaBits;			// Number of bits in the alpha channel (0 for S3TC textures)
	u8 nRedBits;			// Number of bits in the red channel (0 for S3TC textures)
	u8 nGreenBits;			// Number of bits in the green channel (0 for S3TC textrures)
	u8 nBlueBits;			// Number of bits in the blue channel (0 for S3TC textures)
	u8 nS3TC;				// S3TC type (0=not a S3TC texture, otherwise 1=S3TC, 2=S3TCA1, and 3=S3TCx2)
	u8 nMinDim;				// Minimum texture dimension (width and height)

	GXTexFmt nGCFormat;		// The GC format used for this texture
};


//
//
//
typedef struct 
{
	void 		*pPlatformData;
	FTexDef_t	*pTexDef;
	u32 		nTCIndex;
	BOOL 		bStateCached;
} _SetTexState_t;


//////////////////////////////////////////////////////////////////////////////////////
// Static Variables:
//////////////////////////////////////////////////////////////////////////////////////

static _RenderTarget _aRenderTargets[_MAX_RENDERTARGETS];
static u32  _nNumRenderTargets = 0;
static u32  _nCurrentSlot = 0;
static BOOL _bRenderWorld = TRUE;

static FViewport_t *_TexViewport=NULL;
static FViewport_t *_pActiveViewport;
static float _fHorizScale = (640.0f/512.0f);
static FTexDef_t *_RTpTexDef=NULL;
static u16 _nX = 0;
static u16 _nY = 0;

static BOOL _bCompressedAlphaMode[ FGC_MAX_TEX_STAGES ];

static BOOL _bModuleInitialized;
static BOOL _bWindowCreated;

static FResLoadReg_t _ResLoadRegistration;

static FLinkRoot_t _RootList;

static _SetTexState_t _aSetTexState[ FGC_MAX_TEX_STAGES ];

static u32 _nTotalTextureBytes;

static u8 *_pSwizzleBuffer = NULL;

static _TexelInfo_t _aTexelInfoTable[FTEX_FMT_COUNT] = 
{
	FTEX_FMT_R8G8B8A8,	32,	8,	8,	8,	8,	0,	4,	GX_TF_RGBA8,
	FTEX_FMT_R8G8B8X8,	32,	0,	8,	8,	8,	0,	4,	GX_TF_RGBA8,
	FTEX_FMT_R4G4B4A3,	16,	3,	4,	4,	4,	0,	4,	GX_TF_RGB5A3,
	FTEX_FMT_R5G5B5X1,	16,	0,	5,	5,	5,	0,	4,	GX_TF_RGB5A3,
	FTEX_FMT_R5G6B5,	16,	0,	5,	6,	5,	0,	4,	GX_TF_RGB565,
	FTEX_FMT_S3TC,		4,	0,	0,	0,	0,	1,	8,	GX_TF_CMPR,
	FTEX_FMT_S3TCA1,	4,	0,	0,	0,	0,	3,	8,	GX_TF_CMPR,
	FTEX_FMT_S3TCx2,	4,	0,	0,	0,	0,	5,	8,	GX_TF_CMPR,
	FTEX_FMT_I8,		8,	0,	0,	0,	0,	5,	4,	GX_TF_I8,
	FTEX_FMT_IA8,		16,	8,	0,	0,	0,	0,	4,	GX_TF_IA8,
	FTEX_FMT_IA4,		8,	4,	0,	0,	0,	0,	4,	GX_TF_IA4,
	FTEX_FMT_I4,		4, 	0,  0,  0,  0, 	0,  4,  GX_TF_I4
};

static FTexFmt_e _anRenderTarget_TexFmt[]=
{
	FTEX_FMT_R5G6B5,
	FTEX_FMT_R5G6B5,
	FTEX_FMT_R5G6B5,
	FTEX_FMT_R5G6B5,
	FTEX_FMT_R5G6B5,
	FTEX_FMT_R4G4B4A3,
	FTEX_FMT_R4G4B4A3,
	FTEX_FMT_R4G4B4A3,
	FTEX_FMT_R4G4B4A3,
	FTEX_FMT_R4G4B4A3,
	FTEX_FMT_R8G8B8A8,
	FTEX_FMT_R8G8B8A8,
	FTEX_FMT_R8G8B8A8,
	FTEX_FMT_R8G8B8A8,
	FTEX_FMT_R8G8B8A8,
	FTEX_FMT_I8,
	FTEX_FMT_I4,
};

static GXTexFmt _anRenderTarget_GXTexFmt[]=
{
	GX_TF_RGB565,
	GX_TF_RGB565,
	GX_TF_RGB565,
	GX_TF_RGB565,
	GX_TF_RGB565,
	GX_TF_RGB5A3,
	GX_TF_RGB5A3,
	GX_TF_RGB5A3,
	GX_TF_RGB5A3,
	GX_TF_RGB5A3,
	GX_TF_RGBA8,
	GX_TF_RGBA8,
	GX_TF_RGBA8,
	GX_TF_RGBA8,
	GX_TF_RGBA8,
	GX_TF_I8,
	GX_TF_I4
};

CFTexInst CFTexInst::NullTexInst;


//////////////////////////////////////////////////////////////////////////////////////
// Static Functions:
//////////////////////////////////////////////////////////////////////////////////////

static void _RenderTexture_ViewportCallback( FViewport_t *pView ); 

static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _ResLoadDestroy( void *pBase );

static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent );
static void _ClearData( void );

static u32  _ComputeMipmapLevelCount( GXTexFmt nGCFormat, u32 nTexelsAcross, u32 nTexelsDown );
static u32  _ComputeTextureBytes( FTexDef_t *pTexDef );

static BOOL _CopyTextureImageToGCTex( FTexData_t *pTexData, const void *pSrcImage, BOOL bCreate );

static FTexDef_t* _SetupGCTexture( const FTexInfo_t *pTexInfo, const void *pSrcImage, FResHandle_t hRes, FResHandle_t *phRes, BOOL bCreate );
static void _SwizzleTexData( FTexFmt_e nSrcFmt, void *pImage, u32 w, u32 h );
static void _CreateNextMipMapLayer( FTexData_t *pTexData, const void *pSource, void *pDest, u32 nDestW, u32 nDestH );

static BOOL FTex_WorldCallbackFunc( FWorldEvent_e nEvent );
//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////


//
//
//
BOOL ftex_ModuleStartup( void ) 
{
	FASSERT( !_bModuleInitialized );
	
	FASSERT_MSG( !(sizeof( FTexInfo_t ) & 0x1f), "FGCTEX.CPP: FTexInfo_t is not a multiple of 32!  Fang2 GameCube will break!" );

	_bWindowCreated = FALSE;
	
//	_pSwizzleBuffer = (u8 *)OSAlloc( 512 * 512 * 4 );
	_pSwizzleBuffer = NULL;

	_ClearData();

	fgcvid_RegisterWindowCallbackFunction( _WindowCreatedCallback );

	flinklist_InitRoot( &_RootList, (s32)FANG_OFFSETOF( FTexData_t, Link ) );

	fres_CopyType( _ResLoadRegistration.sResType, FTEX_RESNAME );
	_ResLoadRegistration.pszFileExtension = "tga";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_PERM;
	_ResLoadRegistration.nAlignment = 32;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = _ResLoadDestroy;
	

	if ( !fresload_RegisterHandler( &_ResLoadRegistration ) ) 
	{
		// Registration failed...
		DEVPRINTF( "ftex_ModuleStartup(): Could not register resource.\n" );
		return FALSE;
	}

	FTex_szNewReflectMap[0] = 0;
	FTex_szOldReflectMap[0] = 0;

	_nTotalTextureBytes = 0;
	_bModuleInitialized = TRUE;

	return TRUE;
}


//
//
//
void ftex_ModuleShutdown( void ) 
{
	FASSERT( _bModuleInitialized );

	fgcvid_UnregisterWindowCallbackFunction( _WindowCreatedCallback );

	_ClearData();

	OSFree( _pSwizzleBuffer );

	_bModuleInitialized = FALSE;
}


//
//
//
u32 ftex_ComputeMipmapLevelCount( FTexFmt_e nTexelFormat, u32 nTexelsAcross, u32 nTexelsDown ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( nTexelFormat>=0 && nTexelFormat < FTEX_FMT_COUNT );
	
	FASSERT( nTexelsAcross <= FTex_nMaxTextureWidth );
	FASSERT( nTexelsDown <= FTex_nMaxTextureHeight );
	FASSERT( ((u32)nTexelsAcross/(u32)nTexelsDown <= FTex_nMaxAspect) && ((u32)nTexelsDown/(u32)nTexelsAcross) <= FTex_nMaxAspect );

	return _ComputeMipmapLevelCount( _aTexelInfoTable[nTexelFormat].nGCFormat, nTexelsAcross, nTexelsDown );
}


//
//
//
BOOL ftex_IsPointerValid( const FTexDef_t *pTexDef, BOOL bNullPointerIsValid ) 
{
	FASSERT( _bModuleInitialized );

	if ( pTexDef == NULL ) 
		return bNullPointerIsValid;

	return (fres_FindHandle( pTexDef->pTexData ) != FRES_NULLHANDLE);
}


//
//
//
FTexDef_t* ftex_Find( cchar *pszName ) 
{
	FTexData_t *pTexData;

	FASSERT( _bModuleInitialized );

	pTexData = (FTexData_t *)fres_FindBase( FTEX_RESNAME, pszName );

	if ( pTexData ) 
		return &pTexData->TexDef;

	return NULL;
}


//
//
//
FTexDef_t* ftex_CreateTexture( const FTexInfo_t *pTexInfo, const void *pSrcImage, const FTexPalette_t *pPalette, FResHandle_t hRes, FResHandle_t *phRes ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );	// Vid subsystem is not reseting
	FASSERT( pTexInfo );	// Tex info was provided
	FASSERT( pPalette == NULL );	// No pallete support on GC
	FASSERT( fclib_IsStringNullTerminated( pTexInfo->szName, FDATA_TEXNAME_LEN ) );
	FASSERT( pTexInfo->nTexFmt >= 0 && pTexInfo->nTexFmt < FTEX_FMT_COUNT );
	FASSERT( pTexInfo->nPalFmt >= 0 && pTexInfo->nPalFmt < FTEX_PALFMT_COUNT );
	FASSERT( !(pTexInfo->nFlags & FTEX_FLAG_RENDERTARGET) );
	FASSERT( pTexInfo->nLodCount > 0 );
	FASSERT( !(pTexInfo->nFlags & FTEX_FLAG_DYNAMIC) || FTex_bDynamicTextures );
	FASSERT( fmath_IsPowerOf2( pTexInfo->nTexelsAcross, FALSE ) );
	FASSERT( fmath_IsPowerOf2( pTexInfo->nTexelsDown, FALSE ) );
	FASSERT( pTexInfo->nTexelsAcross >= _aTexelInfoTable[pTexInfo->nTexFmt].nMinDim );
	FASSERT( pTexInfo->nTexelsDown >= _aTexelInfoTable[pTexInfo->nTexFmt].nMinDim );

	FASSERT( pTexInfo->nTexelsAcross <= FTex_nMaxTextureWidth );
	FASSERT( pTexInfo->nTexelsDown <= FTex_nMaxTextureHeight );
	FASSERT( ((u32)pTexInfo->nTexelsAcross / (u32)pTexInfo->nTexelsDown <= FTex_nMaxAspect) 
				&& ((u32)pTexInfo->nTexelsDown/(u32)pTexInfo->nTexelsAcross) <= FTex_nMaxAspect );

	FTexInfo_t *pActualTexInfo;
	void *pActualSrcImage;

	pActualTexInfo = (FTexInfo_t *)pSrcImage;
	pActualSrcImage = (void *)(pTexInfo + 1);

	return _SetupGCTexture( pTexInfo, pActualSrcImage, hRes, phRes, FALSE );
}


//
//
//
static GXTexObj* _CreateGCTexObj( FTexDef_t *pTexDef )
{
	GXTexObj *pTexObj = (GXTexObj *)fres_AlignedAlloc( sizeof( GXTexObj ), 32 );
	if ( !pTexObj )
	{
		return NULL;
	}

	// Initialize the user data to NULL	
	GXInitTexObjUserData( pTexObj, NULL );

	FTexData_t *pData = pTexDef->pTexData;
	
	u32 nLODCount = pTexDef->TexInfo.nLodCount;
	
	u32 bLOD = FALSE;
	if ( nLODCount > 1 )
	{
		bLOD = TRUE;
	}

	// Setup the texture properties
	GXInitTexObj( pTexObj, 
				pData->pRawTexture, 
				pData->nWidth, 
				pData->nHeight, 
				pData->nGCTexFormat, 
				GX_CLAMP, 
				GX_CLAMP, 
				bLOD );

	// Setup the LOD properties for the texture
	if ( nLODCount > 1 )
	{
		GXInitTexObjLOD( pTexObj,
				GX_LIN_MIP_LIN,
				GX_LINEAR,
				0,				// min LOD
				nLODCount - 1,	// max LOD
				0,				// LOD bias
				FALSE,			// bias clamp
				FALSE,			// edge LOD enable
				GX_ANISO_1 );	// anisotropy
	}

	// If there is a compressed texture with gradient alpha, we
	// simulate this using two compressed textures (since the GC
	// compression does not support gradient alpha)        
	if ( pTexDef->TexInfo.nTexFmt == FTEX_FMT_S3TCx2 )
	{
		GXTexObj *pAlpha;
		pAlpha = (GXTexObj *)fres_AlignedAlloc( sizeof( GXTexObj ), 32 );
		if ( !pAlpha )
		{
			return NULL;
		}
		GXInitTexObjUserData( pAlpha, NULL );

		// Setup the alpha texture properties
		GXInitTexObj( pAlpha, 
					(u8 *)pData->pRawTexture + (pData->nTextureBytes >> 1), 
					pData->nWidth, 
					pData->nHeight, 
					GX_TF_CMPR, 
					GX_CLAMP, 
					GX_CLAMP, 
					bLOD );

		// Setup the lod properties of the alpha texture					
		if ( nLODCount > 1 )
		{
			GXInitTexObjLOD( pAlpha,
					GX_LIN_MIP_LIN,
					GX_LINEAR,
					0,				// min LOD
					nLODCount - 1,	// max LOD
					0,				// LOD bias
					FALSE,			// bias clamp
					FALSE,			// edge LOD enable
					GX_ANISO_1 );	// anisotropy
		}

		// Shove the alpha channel pointer into the color image user data	        
		GXInitTexObjUserData( pTexObj, pAlpha );
	}
	
	return pTexObj;
}


//
//
//
static FTexDef_t* _SetupGCTexture( const FTexInfo_t *pTexInfo, const void *pSrcImage, FResHandle_t hRes, FResHandle_t *phRes, BOOL bCreate ) 
{
	FTexData_t *pTexData;
	FTexDef_t *pTexDef;
	FResFrame_t ResFrame;

	ResFrame = fres_GetFrame();

	// Build or retrieve our resource...
	if ( hRes == FRES_NULLHANDLE )
	{
		// Resource handle not provided...

		pTexData = (FTexData_t *)fres_CreateAndAlloc( FTEX_RESNAME, pTexInfo->szName, _ResLoadDestroy, sizeof(FTexData_t), &hRes );
		if ( pTexData == NULL ) 
		{
			// Could not create resource...
			goto _CreateTextureFailure;
		}
	} 
	else 
	{
		// Resource handle provided...

		pTexData = (FTexData_t *)fres_Alloc( sizeof(FTexData_t) );
		if ( pTexData == NULL ) 
		{
			goto _CreateTextureFailure;
		}
	}

	pTexDef = &pTexData->TexDef;
	pTexDef->TexInfo = *pTexInfo;
	pTexDef->pTexData = pTexData;
	
	pTexDef->TexInfo.nRenderTargetDepthBitCount = 0;
	pTexDef->TexInfo.nRenderTargetStencilBitCount = 0;

	pTexData->nFlags 		  = 0;
	pTexData->nWidth		  = pTexDef->TexInfo.nTexelsAcross;
	pTexData->nHeight 		  = pTexDef->TexInfo.nTexelsDown;
	pTexData->nGCTexFormat	  = _aTexelInfoTable[pTexDef->TexInfo.nTexFmt].nGCFormat;
	pTexData->nTextureBytes   = _ComputeTextureBytes( pTexDef );

	if ( bCreate )
	{
		// Compute the appropriate number of LOD's
		pTexData->nLodCount = _ComputeMipmapLevelCount( pTexData->nGCTexFormat, pTexData->nWidth, pTexData->nHeight );
		pTexData->nLodCount = FMATH_MIN( pTexData->nLodCount, pTexDef->TexInfo.nLodCount );

		if ( !_CopyTextureImageToGCTex( pTexData, pSrcImage, TRUE ) ) 
		{
			// Failed...

			if ( pTexData->nLodCount == 1 ) 
			{
				// Failed to create texture...
				goto _CreateTextureFailure;
			}

			// Failed to create mipmap.
			// Try creating texture instead...

			pTexData->nLodCount = 1;
			pTexDef->TexInfo.nLodCount = 1;

			if ( !_CopyTextureImageToGCTex( pTexData, pSrcImage, TRUE ) ) 
			{
				// Failed to create texture...
				goto _CreateTextureFailure;
			}
		}
		
		// Flush the texture so that it is out of the CPU cache
		DCFlushRange( pTexData->pRawTexture, pTexData->nTextureBytes );
	}
	else
	{
		pTexData->nLodCount = pTexDef->TexInfo.nLodCount;
		// Flush the texture so that it is out of the CPU cache
		pTexData->pRawTexture = (void *)pSrcImage;
		DCFlushRange( pTexData->pRawTexture, pTexData->nTextureBytes );
	}

	// Success!

	pTexDef->TexInfo.nLodCount = pTexData->nLodCount;
	pTexDef->TexInfo.nTexelsAcross = pTexData->nWidth;
	pTexDef->TexInfo.nTexelsDown = pTexData->nHeight;
	pTexData->nAttachedStages = 0;
	#if FPERF_ENABLE
		pTexData->nPerfKey = 0;
	#endif

	// Create the GC tex obj
	pTexData->pGCTexObj = _CreateGCTexObj( pTexDef );
	if ( !pTexData->pGCTexObj )
	{
		DEVPRINTF( "_SetupGCTexture - Unable to create the GameCube Texture object for texture %s.", pTexDef->TexInfo.szName );
		goto _CreateTextureFailure;
	}
	
	if ( FResLoad_bLoadedForStreaming )
	{
#if 0
		pTexData->pRawTexture = FDS_StreamMgr.StoreData( pTexData->pRawTexture, pTexData->nTextureBytes, FDS_LOCATION_ARAM );
#else
		char szTextureName[16];
		fclib_strcpy( szTextureName, pTexDef->TexInfo.szName );
		fclib_strcat( szTextureName, ".tga" );
		pTexData->pRawTexture = FDS_StreamMgr.StreamFromMasterfile( szTextureName, sizeof(FTexInfo_t) );
#endif
		pTexData->TexDef.TexInfo.nFlags |= FTEX_FLAG_STREAMING;
	}
	
	flinklist_AddTail( &_RootList, pTexData );

	if ( phRes ) 
	{
		*phRes = hRes;
	}

	_nTotalTextureBytes += pTexData->nTextureBytes;

#if FHEAP_TRACK_MEM_ALLOCATIONS
	if ( Fheap_nTexPoolCount < FHEAP_TEX_MEM_TRACKERS_POOL_SIZE )
	{
		TexMemTracker_t *pTexTracker = &Fheap_nTexTrackerPool[Fheap_nTexPoolCount++];
		pTexTracker->pTexInfo = &pTexData->TexDef.TexInfo;
		pTexTracker->nGCMemAllocated = pTexData->nTextureBytes;
		Fheap_nTotalGCTexMemTracked += pTexData->nTextureBytes;
#if FHEAP_DETAILED_TRACKING		
		pTexTracker->pszRequestingModuleName = Fheap_pszSecondaryFileName;
		switch ( pTexInfo->nTexFmt )
		{
			case FTEX_FMT_R8G8B8A8:
				pTexTracker->pszFormatName = "FTEX_FMT_R8G8B8A8";
				break;
				
			case FTEX_FMT_R8G8B8X8:
				pTexTracker->pszFormatName = "FTEX_FMT_R8G8B8X8";
				break;
				
			case FTEX_FMT_R4G4B4A3:
				pTexTracker->pszFormatName = "FTEX_FMT_R4G4B4A3";
				break;
				
			case FTEX_FMT_R5G5B5X1:
				pTexTracker->pszFormatName = "FTEX_FMT_R5G5B5X1";
				break;
				
			case FTEX_FMT_R5G6B5:
				pTexTracker->pszFormatName = "FTEX_FMT_R5G6B5";
				break;
				
			case FTEX_FMT_S3TC:
				pTexTracker->pszFormatName = "FTEX_FMT_S3TC";
				break;
				
			case FTEX_FMT_S3TCA1:
				pTexTracker->pszFormatName = "FTEX_FMT_S3TCA1";
				break;
				
			case FTEX_FMT_S3TCx2:
				pTexTracker->pszFormatName = "FTEX_FMT_S3TCx2";
				break;
				
			case FTEX_FMT_I8:
				pTexTracker->pszFormatName = "FTEX_FMT_I8";
				break;
			case FTEX_FMT_IA8:
				pTexTracker->pszFormatName = "FTEX_FMT_IA8";
				break;
			case FTEX_FMT_IA4:
				pTexTracker->pszFormatName = "FTEX_FMT_IA4";
				break;
				
			default:
				pTexTracker->pszFormatName = "UNKNOWN";
				break;
		}
#endif // FHEAP_DETAILED_TRACKING
	}
#endif

	return pTexDef;

_CreateTextureFailure:
	// Failure...
	fres_ReleaseFrame( ResFrame );

	if ( phRes )
		*phRes = FRES_NULLHANDLE;

	return NULL;
}


//
//
//
BOOL ftex_ModifyTexture( FTexDef_t *pTexDef, const void *pSrcImage, const FTexPalette_t *pPalette ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );
	FASSERT( ftex_IsPointerValid( pTexDef, TRUE ) );
	FASSERT( pPalette == NULL );
	FASSERT( pTexDef->pTexData->nGCTexFormat != GX_TF_CMPR );

	if ( pTexDef == NULL )
		return TRUE;

	if ( pSrcImage == NULL )
		return TRUE;

	FASSERT( pTexDef->TexInfo.nFlags & FTEX_FLAG_DYNAMIC );
	
	_CopyTextureImageToGCTex( pTexDef->pTexData, pSrcImage, FALSE );
	
	return FALSE;
}


//
//
//
void ftex_PreLoad( FTexDef_t *pTexDef ) 
{
	FASSERT( _bWindowCreated );
	FASSERT( !FGCvid_bResetting );
}

void ftex_ReleaseTex(CFTexInst *pTex)
{

}

//
//
//
CFTexInst *ftex_CreateRenderTarget_FullScreen( FTexRenderTargetFmt_e nFormat, cchar *pszName, BOOL bPow2, FResHandle_t hRes, FResHandle_t *phRes, u32 nTexelsAcross, u32 nTexelsDown, FTextureType_e nType, BOOL bDoubleBuffer )
{
	if (nTexelsAcross == 0 || nTexelsDown == 0)
	{
		nTexelsAcross = 512; nTexelsDown = 448;
	}

	if (nTexelsDown > 448 && nFormat != FTEX_RENDERTARGET_FMT_I4_D24) { nTexelsDown = 448; bPow2 = FALSE; }
	
	CFTexInst *pTexInst = ftex_CreateRenderTarget(nTexelsAcross, nTexelsDown, nFormat, pszName, bPow2, hRes, phRes, nType);
	if (pTexInst)
	{
		pTexInst->SetFlag(CFTexInst::FLAG_FULLSCREEN);
	}
	return (pTexInst);
}


//
//
//
CFTexInst *ftex_CreateDynamicEMBM( u32 nTexelsAcross, u32 nTexelsDown, cchar *pszName, FResHandle_t hRes, FResHandle_t *phRes ) 
{
	FASSERT( _bWindowCreated );	
	CFTexInst *pCFTex = NULL;
	FTexInfo_t TexInfo;
	
	fclib_strcpy(TexInfo.szName, pszName);
	TexInfo.nTexFmt = FTEX_FMT_IA8;
	//TexInfo.nFlags = FTEX_FLAG_RENDERTARGET;
	TexInfo.nFlags = FTEX_FLAG_DYNAMIC;
	TexInfo.nLodCount = 1;
	TexInfo.nRenderTargetStencilBitCount = 0;
	TexInfo.nRenderTargetDepthBitCount = 16;
	TexInfo.nTexelsAcross = nTexelsAcross;
	TexInfo.nTexelsDown = nTexelsDown;
	
	u32 nNewBytes = GXGetTexBufferSize(nTexelsAcross, nTexelsDown, GX_TF_IA8, GX_FALSE, 10);
	u32 *pnTexture = (u32 *)fres_AlignedAlloc( nNewBytes, 32 );
	fang_MemSet(pnTexture, 127, nNewBytes);
	
	DCFlushRange( pnTexture, nNewBytes );
	
	pCFTex = fnew CFTexInst;
	FTexDef_t *pTexDef = _SetupGCTexture( &TexInfo, (void *)(pnTexture), hRes, phRes, FALSE );
	pCFTex->SetTexDef(pTexDef);
	
	return pCFTex;
}


//
//
//
void ftex_HeightMapToEMBM(CFTexInst *pTex, f32 *pfData, BOOL bVec3)
{
	if (!pTex || !pfData) 
	{
		return;
	}
	
	FTexDef_t *pTexDef = pTex->GetTexDef();
	if (pTexDef)
	{
		GXTexObj *gxTex = (GXTexObj *)pTexDef->pTexData->pGCTexObj;
		if (gxTex)
		{
			u8 *pData = (u8*)GXGetTexObjData(gxTex);
			if (!pData) return;
			
			pData = (u8*)OSPhysicalToCached((u32)pData);
			
			s32 nX, nY, nOffs, nDelta, nOffsX;
			f32 fDX, fDY;
			
			f32 fT00, fT01, fT10;
			
			nDelta = pTexDef->TexInfo.nTexelsAcross*2;
			
			for (nY = 0, nOffs = 0; nY < pTexDef->TexInfo.nTexelsDown-1; nY++, nOffs += nDelta)
			{
				for (nX = 0, nOffsX = 0; nX < pTexDef->TexInfo.nTexelsAcross-1; nX++, nOffsX += 2)
				{
					fT00 = pfData[ nX + nY*pTexDef->TexInfo.nTexelsAcross ] * 0.25f;
					fT01 = pfData[nX+1+ nY*pTexDef->TexInfo.nTexelsAcross ] * 0.25f;
					fT10 = pfData[ nX +(nY+1)*pTexDef->TexInfo.nTexelsAcross] * 0.25f;
					
					fDX = (fT00 - fT01)*128.0f + 127.0f;
					FMATH_CLAMP(fDX, 0.0f, 255.0f);
					fDY = (fT00 - fT10)*128.0f + 127.0f;
					FMATH_CLAMP(fDY, 0.0f, 255.0f);
					
					if (nOffs + nOffsX > 8190)
					{
						FASSERT_NOW;
					}
				
					pData[nOffs + nOffsX + 0] = (u8)( fDX );
					pData[nOffs + nOffsX + 1] = (u8)( fDY );
				}
			}
			
			DCFlushRange( pData, pTexDef->TexInfo.nTexelsDown*pTexDef->TexInfo.nTexelsAcross*2 );
			_Swizzle_IA8(pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, pData);
			DCFlushRange( pData, pTexDef->TexInfo.nTexelsDown*pTexDef->TexInfo.nTexelsAcross*2 );
		}
	}
	FASSERT( pTex );
}


//
// These do nothing on the GC since everything uses the EFB depth buffer.
//
void ftex_CreateSharedDepthBuffer(u8 nBPP, u32 nWidth, u32 nHeight)
{
}


//
//
//
void ftex_ClearSharedDepthBuffer( void )
{
}


//
//
//
CFTexInst* ftex_CreateRenderTarget( u32 nTexelsAcross, u32 nTexelsDown, FTexRenderTargetFmt_e nFormat, cchar *pszName, BOOL bPow2, FResHandle_t hRes, FResHandle_t *phRes, FTextureType_e nType, BOOL bDoubleBuffer) 
{
	FASSERT( _bWindowCreated );
#if FANG_ENABLE_FASSERT
	if (bPow2)
	{
		FASSERT( fmath_IsPowerOf2( nTexelsAcross, FALSE ) );
		FASSERT( fmath_IsPowerOf2( nTexelsDown, FALSE ) );
	}
#endif
	CFTexInst *pCFTex = NULL;
	FTexInfo_t TexInfo;
	
	if( pszName == NULL ) 
	{
		pszName = FTEX_DEFAULT_RENDER_TARGET_NAME;
	}
	
	fclib_strcpy(TexInfo.szName, pszName);
	TexInfo.nTexFmt = _anRenderTarget_TexFmt[nFormat];
	TexInfo.nFlags = FTEX_FLAG_RENDERTARGET;
	TexInfo.nLodCount = 1;
	TexInfo.nRenderTargetStencilBitCount = 0;
	TexInfo.nRenderTargetDepthBitCount = 24;
	TexInfo.nTexelsAcross = nTexelsAcross;
	TexInfo.nTexelsDown = nTexelsDown;
	
#if FHEAP_TRACK_MEM_ALLOCATIONS
	char _szOldResourceType[8];
	fclib_strncpy(_szOldResourceType, Fheap_szResourceType, 6);
	fclib_strcpy( Fheap_szResourceType, "TXTR" );
#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
 	u32 nNewBytes = GXGetTexBufferSize(nTexelsAcross, nTexelsDown, _anRenderTarget_GXTexFmt[nFormat], GX_FALSE, 10);
	u32 *pnTexture = (u32 *)fres_AlignedAllocAndZero( nNewBytes, 32 );
	if ( pnTexture )
	{
		DCFlushRange( pnTexture, nNewBytes );
		
		pCFTex = fnew CFTexInst;
		if ( pCFTex )
		{
			FTexDef_t *pTexDef = _SetupGCTexture( &TexInfo, (void *)(pnTexture), hRes, phRes, FALSE );
			pCFTex->SetTexDef( pTexDef );
			pCFTex->SetFlag( CFTexInst::FLAG_RENDERTARGET );
			if (bPow2)
			{
				pCFTex->SetFlag( CFTexInst::FLAG_POW2 );
			}
			else
			{
				pCFTex->ClearFlag(CFTexInst::FLAG_WRAP_S);
				pCFTex->ClearFlag(CFTexInst::FLAG_WRAP_T);
			}
		}
	}	
	
#if FHEAP_TRACK_MEM_ALLOCATIONS
	fclib_strncpy(Fheap_szResourceType, _szOldResourceType, 6);
#endif // FHEAP_TRACK_MEM_ALLOCATIONS
	
	return pCFTex;
}


//
//
//
void fgctex_InitGCTexObj( GXTexObj **ppTexObj, CFTexInst *pTexInst ) 
{
	static GXTexWrapMode __nSTMap[2] = 
	{
		GX_CLAMP,
		GX_REPEAT
	};
	static GXTexFilter __nFilterMap[3] = 
	{
		GX_LIN_MIP_LIN,
		GX_LINEAR,
		GX_NEAR
	};
	
	// If this texture is in streamed remote memory, make sure it is in the cache
	FTexDef_t *pTexDef = pTexInst->GetTexDef();
	if ( pTexDef->TexInfo.nFlags & FTEX_FLAG_STREAMING )
	{
		void *pCachedData = FDS_StreamMgr.AccessData( pTexDef->pTexData->pRawTexture );
		if ( pCachedData )
		{
			if ( !GXGetTexObjData( *ppTexObj ) )
			{
				GXInitTexObjData( *ppTexObj, pCachedData );
			}
		}
		else
		{
			GXInitTexObjData( *ppTexObj, NULL );
			*ppTexObj = NULL;
			return;
		}
	}
	
	// Set the wrapping
	GXInitTexObjWrapMode( *ppTexObj, __nSTMap[!!(pTexInst->GetFlags() & CFTexInst::FLAG_WRAP_S)], 
									__nSTMap[!!(pTexInst->GetFlags() & CFTexInst::FLAG_WRAP_T)] );
/*									
	u32 nFilterIdx = !!(pTexInst->GetFlags() & CFTexInst::FLAG_MINFILTER_NO_TRILINEAR) + (!!( pTexInst->GetFlags() & CFTexInst::FLAG_MINFILTER_NO_BILINEAR ) << 1);
	FASSERT( nFilterIdx >= 0 && nFilterIdx < 3 );
	GXInitTexObjFilter( __nFilterMap[ nFilterIdx ] );
*/
}


//
//
//
void CFTexInst::SetTexDef( FTexDef_t *pTexDef ) 
{
	if ( pTexDef || m_pTexDef ) 
	{
		SetStateChangedFlag( TRUE );
	}

	m_pTexDef = pTexDef;
	
	if ( !pTexDef )
	{
		return;
	}
}


//
//
//
static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName ) 
{
	FTexDef_t *pTexDef;
	FTexInfo_t *pTexInfo;
	void *pSrcImage;

	FASSERT( _bWindowCreated );

	pTexInfo = (FTexInfo_t *)pLoadedBase;
	pSrcImage = (void *)(pTexInfo + 1);

	FASSERT( fclib_strlen( pszResName ) <= FDATA_TEXNAME_LEN );
	fclib_strcpy( pTexInfo->szName, pszResName );

	pTexDef = _SetupGCTexture( pTexInfo, pSrcImage, hRes, NULL, FALSE );
	
	// Bail if unsuccessful
	if ( pTexDef == NULL ) 
	{
		return FALSE;
	}

	// Texture created successfully...
	fres_SetBase( hRes, pTexDef->pTexData );

	return TRUE;
}


//
//
//
static void _ResLoadDestroy( void *pBase ) 
{
	FTexData_t *pTexData, *pTexDataCurrentViewport;
	FViewport_t *pViewport;

	pTexData = (FTexData_t *)pBase;
	pViewport = fviewport_GetActive();
	pTexDataCurrentViewport = (pViewport && pViewport->pTexDef) ? pViewport->pTexDef->pTexData : NULL;

	_nTotalTextureBytes -= pTexData->nTextureBytes;

	// If we're deleting a render target that's currently set as the active
	// viewport, we must disable the current viewport...
	if ( pTexData == pTexDataCurrentViewport ) 
	{
		if ( !FGCvid_bResetting ) 
		{
			fviewport_SetActive( NULL );
		}
	}

#if FHEAP_TRACK_MEM_ALLOCATIONS
	fheap_UnTrackTexMem( &(pTexData->TexDef.TexInfo) );
#endif
	
	flinklist_Remove( &_RootList, pTexData );
	
	//ftex_ClearRenderTargets();
}


//
//
//
static void _ReleaseAllRenderTargets( void ) 
{
	FTexData_t *pTexData;

	FASSERT( _bWindowCreated );

	// The video system is about to reset the device. For the reset to be successful,
	// we must Release all non-managed textures. Currently, these are only render
	// targets, all of which have been created in D3DPOOL_DEFAULT.

	for ( pTexData=(FTexData_t *)flinklist_GetTail(&_RootList); pTexData; pTexData=(FTexData_t *)flinklist_GetPrev( &_RootList, pTexData ) ) 
	{
		if ( pTexData->TexDef.TexInfo.nFlags & FTEX_FLAG_RENDERTARGET ) 
		{
			// We found a render target. Release the texture...

			FASSERT( pTexData->pRawTexture );
		}
	}
}

//
//
//
static void _RebuildAllRenderTargets( void ) 
{
}

//
//
//
static BOOL _WindowCreatedCallback( FGCVidEvent_e nEvent ) 
{
	u32 i;

	FASSERT( _bModuleInitialized );
	FASSERT( nEvent>=0 && nEvent<FGCVID_EVENT_COUNT );

	switch( nEvent ) 
	{
		case FGCVID_EVENT_WINDOW_CREATED:
			_bWindowCreated = TRUE;

			_nTotalTextureBytes = 0;

			FTex_bRenderTargets = TRUE;

			FTex_nMaxTextureWidth 	= 1024;
			FTex_nMaxTextureHeight 	= 1024;
			FTex_nMaxAspect 		= FMATH_MAX( FTex_nMaxTextureWidth, FTex_nMaxTextureHeight );
			FTex_bMipmaps 			= TRUE;
			FTex_bDynamicTextures 	= TRUE;
			FTex_bDynamicsCanBeLost = FALSE;
			FGC_nStageCount			= 8;

			// Mark the stages as no longer cached
			for ( i = 0; i < FGC_MAX_TEX_STAGES; i++ ) 
			{
				_aSetTexState[i].bStateCached = FALSE;
			}

			fang_MemSet( _bCompressedAlphaMode, 0, sizeof( BOOL ) * FGC_MAX_TEX_STAGES );
			fworld_RegisterWorldCallbackFunction( FTex_WorldCallbackFunc );

			break;

		case FGCVID_EVENT_WINDOW_DESTROYED:
			_ClearData();
			_bWindowCreated = FALSE;
			
			fworld_UnregisterWorldCallbackFunction( FTex_WorldCallbackFunc );
			break;

		case FGCVID_EVENT_PRE_RESET:
			_ReleaseAllRenderTargets();
			break;

		case FGCVID_EVENT_POST_RESET:
			_RebuildAllRenderTargets();
			break;
	}

	return TRUE;
}

BOOL FTex_WorldCallbackFunc( FWorldEvent_e nEvent )
{
	switch (nEvent)
	{
		case FWORLD_EVENT_WORLD_PRELOAD:
			ftex_ClearRenderTargets();
			break;
		case FWORLD_EVENT_WORLD_POSTDESTROY:
			ftex_ClearRenderTargets();
			break;
		case FWORLD_EVENT_WORLD_POSTLOAD:
			break;
		case FWORLD_EVENT_WORLD_PREDESTROY:
			break;
	};
	return TRUE;
}

//
//
//
static void _ClearData( void ) 
{
	FTex_nMaxTextureWidth = 0;
	FTex_nMaxTextureHeight = 0;
	FTex_nMaxAspect = 0;

	FTex_bMipmaps = FALSE;
	FTex_bRenderTargets = FALSE;
	FTex_bDynamicTextures = FALSE;
	FTex_bDynamicsCanBeLost = FALSE;
}


//
//
//
static u32 _ComputeMipmapLevelCount( GXTexFmt nGCFormat, u32 nTexelsAcross, u32 nTexelsDown ) 
{
	FASSERT( fmath_IsPowerOf2( nTexelsAcross, FALSE ) );
	FASSERT( fmath_IsPowerOf2( nTexelsDown, FALSE ) );

	// If mipmaps aren't supported, bail out
	if ( !FTex_bMipmaps ) 
		return 1;

	u32 nMin;
	if ( nGCFormat == GX_TF_CMPR )
	{
		FASSERT( nTexelsAcross 	>= _COMPRESSED_MIN_DIMENSION );
		FASSERT( nTexelsDown 	>= _COMPRESSED_MIN_DIMENSION );
		nMin = _COMPRESSED_MIN_DIMENSION;
	}
	else
	{
		FASSERT( nTexelsAcross 	>= _UNCOMPRESSED_MIN_DIMENSION );
		FASSERT( nTexelsDown 	>= _UNCOMPRESSED_MIN_DIMENSION );
		nMin = _UNCOMPRESSED_MIN_DIMENSION;
	}

	u32 nLod = 0;
	while ( nTexelsAcross >= nMin && nTexelsDown >= nMin )
	{
		nTexelsAcross >>= 1;
		nTexelsDown >>= 1;
		nLod++;
	}
	
	return nLod;
}


//
//
//
static u32 _ComputeTextureBytes( FTexDef_t *pTexDef ) 
{
//	u32 nLod, nWidth, nHeight, nBitsPerTexel, nBitCount, nMinDim, nLodCount;

	u32 nBitCount = 0;
	u32 nWidth 			= pTexDef->TexInfo.nTexelsAcross;
	u32 nHeight 		= pTexDef->TexInfo.nTexelsDown;
	u32 nLodCount 		= pTexDef->TexInfo.nLodCount;
	u32 nBitsPerTexel	= _aTexelInfoTable[ pTexDef->TexInfo.nTexFmt ].nBitsPerTexel;
	u32 nMinDim 		= _aTexelInfoTable[ pTexDef->TexInfo.nTexFmt ].nMinDim;

	for ( u32 nLod = 0; nLod < nLodCount; nLod++ ) 
	{
		nBitCount += FMATH_BYTE_ALIGN_UP( nWidth * nHeight * nBitsPerTexel, 32 );

		nWidth >>= 1;
		nHeight >>= 1;

		FMATH_CLAMPMIN( nWidth, nMinDim );
		FMATH_CLAMPMIN( nHeight, nMinDim );
	}

	if ( pTexDef->TexInfo.nTexFmt == FTEX_FMT_S3TCx2 )
		nBitCount <<= 1;
		
	return nBitCount >> 3;
}


//
//
//
static void _ResizeDimensionsIfRequired( FTexFmt_e nTexFmt, u16 *pnWidth, u16 *pnHeight ) 
{
	u32 nRatio, nWidth, nHeight, nMaxWidth, nMaxHeight, nMinDim;

	nWidth = *pnWidth;
	nHeight = *pnHeight;
	nMinDim = _aTexelInfoTable[nTexFmt].nMinDim;

	nMaxWidth = (FTex_nMaxTextureWidth < nMinDim) ? nMinDim : FTex_nMaxTextureWidth;
	nMaxHeight = (FTex_nMaxTextureHeight < nMinDim) ? nMinDim : FTex_nMaxTextureHeight;
	FASSERT( nMaxWidth );
	FASSERT( nMaxHeight );

	// Fixup width...
	nRatio = nWidth / nMaxWidth;
	if ( nRatio > 1 ) 
	{
		// Texture is too wide...
		nWidth /= nRatio;
		nHeight /= nRatio;
		FMATH_CLAMPMIN( nWidth, nMinDim );
		FMATH_CLAMPMIN( nHeight, nMinDim );
	}

	// Fixup height...
	nRatio = nHeight / nMaxHeight;
	if ( nRatio > 1 ) 
	{
		// Texture is too tall...
		nWidth /= nRatio;
		nHeight /= nRatio;
		FMATH_CLAMPMIN( nWidth, nMinDim );
		FMATH_CLAMPMIN( nHeight, nMinDim );
	}

	// Fixup aspect ratio...
	nRatio = nWidth / nHeight;
	if ( nRatio > FTex_nMaxAspect ) 
	{
		// Texture too wide...
		nWidth = nHeight * nRatio;
	} 
	else 
	{
		nRatio = nHeight / nWidth;
		if ( nRatio > FTex_nMaxAspect ) 
		{
			// Texture too tall...
			nHeight = nWidth * nRatio;
		}
	}

	*pnWidth = nWidth;
	*pnHeight = nHeight;
}


//
//
//
static BOOL _CopyTextureImageToGCTex( FTexData_t *pTexData, const void *pSrcImage, BOOL bCreate )
{
	FASSERT( pSrcImage );

	if ( bCreate )
	{
		// Allocate space for the resulting texture
		pTexData->pRawTexture = fres_AlignedAlloc( pTexData->nTextureBytes, 32 );
		if ( !pTexData->pRawTexture )
			return FALSE;
			
		// Flag this texture as having been created at runtime, not loaded	
		pTexData->nFlags |= FGCTEXFLAGS_RUNTIME;
	}

	switch ( pTexData->TexDef.TexInfo.nTexFmt )
	{
		case FTEX_FMT_R8G8B8A8: 
		case FTEX_FMT_R8G8B8X8:
		case FTEX_FMT_R4G4B4A3:
		case FTEX_FMT_R5G5B5X1:
		case FTEX_FMT_R5G6B5:
		case FTEX_FMT_I8:
		case FTEX_FMT_IA8:
		case FTEX_FMT_IA4:
		{
			u32 nTexFormat = pTexData->TexDef.TexInfo.nTexFmt;
			u32 nBytesPerTexel = _aTexelInfoTable[ nTexFormat ].nBitsPerTexel / 8;
			u32 nWidth = pTexData->nWidth;
			u32 nHeight = pTexData->nHeight;
			u8 *pData = (u8 *)pTexData->pRawTexture;
			for ( u32 i = 0; i < pTexData->nLodCount; i++ )
			{
				// Create the mipmap from the source image
				_CreateNextMipMapLayer( pTexData, pSrcImage, pData, nWidth, nHeight );
				
				_SwizzleTexData( (FTexFmt_e)nTexFormat, pData, nWidth, nHeight );
				
//				fang_MemSet( pData, 255, nWidth * nHeight * nBytesPerTexel );

				// Move on to the next LOD
				pData += FMATH_BYTE_ALIGN_UP(nWidth * nHeight * nBytesPerTexel, 32);
				nWidth = nWidth >> 1;
				nHeight = nHeight >> 1;
			}
				
			break;
		}

		case FTEX_FMT_S3TCx2:
		case FTEX_FMT_S3TC:
		case FTEX_FMT_S3TCA1:
		{
			DEVPRINTF( "FGCTEX.CPP: You cannot create compressed textures at runtime!\n" );
			FASSERT_NOW;
			break;
		}

		default:
			DEVPRINTF( "FGCTEX.CPP: Request to create unrecognized tex type.\n" );
			FASSERT_NOW;
			break;
	}
	
	return TRUE;
}


//
//	Returns 8-bit R, G, B, and A for non-swizzled textures for specified row and column
//
static void _GetTexelRGBA( u8 *pSource, u32 nSrcFmt, u32 nSrcWidth, u32 nRow, u32 nCol, u8 *r, u8 *g, u8 *b, u8 *a )
{
	switch ( nSrcFmt )
	{
		case FTEX_FMT_R8G8B8A8: 
		{
			u32 nColor = ((u32 *)pSource)[ nRow * nSrcWidth + nCol ];
			*r = (nColor & 0xff000000) >> 24;
			*g = (nColor & 0x00ff0000) >> 16;
			*b = (nColor & 0x0000ff00) >>  8;
			*a = (nColor & 0x000000ff) >>  0;
			break;
		}

		case FTEX_FMT_R8G8B8X8:
		{
			u32 nColor = ((u32 *)pSource)[ nRow * nSrcWidth + nCol ];
			*r = (nColor & 0xff000000) >> 24;
			*g = (nColor & 0x00ff0000) >> 16;
			*b = (nColor & 0x0000ff00) >>  8;
			*a = 255;
			break;
		}

		case FTEX_FMT_R4G4B4A3:
		{
			u16 nColor = ((u16 *)pSource)[ nRow * nSrcWidth + nCol ];
			*a = ((nColor & 0xf000) >> 12) * 36.4285f;
			*r = ((nColor & 0x0f00) >>  8) * 17;
			*g = ((nColor & 0x00f0) >>  4) * 17;
			*b = ((nColor & 0x000f) >>  0) * 17;
			break;
		}

		case FTEX_FMT_R5G5B5X1:
		{
			u16 nColor = ((u16 *)pSource)[ nRow * nSrcWidth + nCol ];
			*a = 255;
			*r = ((nColor & 0x7c00) >> 10) * 8.2258f;
			*g = ((nColor & 0x03e0) >>  5) * 8.2258f;
			*b = ((nColor & 0x001f) >>  0) * 8.2258f;
			break;
		}

		case FTEX_FMT_R5G6B5:
		{
			u16 nColor = ((u16 *)pSource)[ nRow * nSrcWidth + nCol ];
			*a = 255;
			*r = ((nColor & 0xf800) >> 11) * 8.2258f;
			*g = ((nColor & 0x07e0) >>  5) * 4.0476f;
			*b = ((nColor & 0x001f) >>  0) * 8.2258f;
			break;
		}

		case FTEX_FMT_S3TCx2:
		case FTEX_FMT_S3TC:
		case FTEX_FMT_S3TCA1:
		{
			FASSERT_NOW;
			break;
		}

		case FTEX_FMT_I8:
		{
			u8 nColor = pSource[ nRow * nSrcWidth + nCol ];
			*a = 255;
			*r = nColor;
			*g = nColor;
			*b = nColor;
			break;
		}

		case FTEX_FMT_IA8:
		{
			u16 nColor = ((u16 *)pSource)[ nRow * nSrcWidth + nCol ];
			*a =  nColor&0x00ff;
			*r = (nColor&0xff00)>>8;
			*g = (nColor&0xff00)>>8;
			*b = (nColor&0xff00)>>8;
			break;
		}

		case FTEX_FMT_IA4:
		{
			u8 nColor = pSource[ nRow * nSrcWidth + nCol ];
			*a = (nColor&0x000f)*17;
			*r = ((nColor&0x00f0)>>4)*17;
			*g = ((nColor&0x00f0)>>4)*17;
			*b = ((nColor&0x00f0)>>4)*17;
			break;
		}

		default:
			FASSERT_NOW;
			break;
	}
}


//
//	Sets 8-bit R, G, B, and A for non-swizzled textures for specified row and column
//
static void _SetTexelRGBA( u8 *pDest, u32 nDestFmt, u32 nDestWidth, u32 nRow, u32 nCol, u8 r, u8 g, u8 b, u8 a )
{
	switch ( nDestFmt )
	{
		case FTEX_FMT_R8G8B8A8: 
		{
			((u32 *)pDest)[ nRow * nDestWidth + nCol ] = (r << 24) | (g << 16) | (b << 8) | a;
			break;
		}

		case FTEX_FMT_R8G8B8X8:
		{
			((u32 *)pDest)[ nRow * nDestWidth + nCol ] = (r << 24) | (g << 16) | (b << 8) | 255;
			break;
		}

		case FTEX_FMT_R4G4B4A3:
		{
			((u16 *)pDest)[ nRow * nDestWidth + nCol ] =  ((a & 0xe0) << 7) | ((r & 0xf0) << 4) 
										| ((g & 0xf0) << 0) | ((b & 0xf0) >> 4);
			break;
		}

		case FTEX_FMT_R5G5B5X1:
		{
			((u16 *)pDest)[ nRow * nDestWidth + nCol ] = 0x80 | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) 
										| ((b & 0xf8) >> 3);
			break;
		}

		case FTEX_FMT_R5G6B5:
		{
			((u16 *)pDest)[ nRow * nDestWidth + nCol ] = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) 
										| ((b & 0xf8) >> 3);
			break;
		}

		case FTEX_FMT_S3TCx2:
		case FTEX_FMT_S3TC:
		case FTEX_FMT_S3TCA1:
		{
			FASSERT_NOW;
			break;
		}

		case FTEX_FMT_I8:
		{
			pDest[ nRow * nDestWidth + nCol ] = r;
			break;
		}

		case FTEX_FMT_IA8:
		{
			((u16 *)pDest)[ nRow * nDestWidth + nCol ] = (r<<8) | (a);
			break;
		}

		case FTEX_FMT_IA4:
		{
			pDest[ nRow * nDestWidth + nCol ] = ((r/17)<<4) | (a/17);
			break;
		}

		default:
			FASSERT_NOW;
			break;
	}

}


//
//	Creates a mipmap in pDest using the data in pSource.
//	Basic box-filter technique
//
static void _CreateNextMipMapLayer( FTexData_t *pTexData, const void *pSource, void *pDest, u32 nDestW, u32 nDestH )
{
	FASSERT( pSource && pDest );
	
	u8	r, g, b, a;
	f32	rAcc, gAcc, bAcc, aAcc;
	
	// Calculate the box dimensions
	u32 nBoxDimW = pTexData->nWidth / nDestW;
	u32 nBoxDimH = pTexData->nHeight / nDestH;
	f32 fOOBoxWxH = 1.f / (f32)(nBoxDimW * nBoxDimH);

	// If it's one-to-one, just copy it
	if ( nBoxDimW == 1 && nBoxDimH == 1 )
	{
		fang_MemCopy( pDest, pSource, pTexData->nTextureBytes );
		return;
	}
	
	u32 nSrcFmt = pTexData->TexDef.TexInfo.nTexFmt;
	
	// Write out the mipmap into pDest
	for ( u32 row = 0; row < nDestH; row++ )
	{
		for ( u32 col = 0; col < nDestW; col++ )
		{
			rAcc = gAcc = bAcc = aAcc = 0.f;
			
			// Average the adjacent pixels in the source, taking 
			// into account the alpha values:
			for ( u32 nSampleRow = 0; nSampleRow < nBoxDimH; nSampleRow++ )
			{
				for ( u32 nSampleCol = 0; nSampleCol < nBoxDimW; nSampleCol++ )
				{
					_GetTexelRGBA( (u8 *)pSource, nSrcFmt, pTexData->nWidth, (row * nBoxDimW) + nSampleRow, (col * nBoxDimH) + nSampleCol, &r, &g, &b, &a );
					
					rAcc += r * a;
					gAcc += g * a;
					bAcc += b * a;
					aAcc += a;
				}
			}
			
			// Assign the average to the destination
			if ( aAcc )
			{
				f32 fOOaAcc = 1.f / aAcc;
				_SetTexelRGBA( (u8 *)pDest, nSrcFmt, nDestW, row, col, 
								(u8)(rAcc * fOOaAcc + 0.5f), 
								(u8)(gAcc * fOOaAcc + 0.5f), 
								(u8)(bAcc * fOOaAcc + 0.5f), 
								(u8)(aAcc * fOOBoxWxH + 0.5f) );
			}
			else
			{
				_SetTexelRGBA( (u8 *)pDest, nSrcFmt, nDestW, row, col, 0, 0, 0, 0 );
			}
		}
	} 		
}

static void _Swizzle_RGBA8( s32 width, s32 height, void *pImage, BOOL bFullAlpha );
static void _PackTile_RGBA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pResult, BOOL bFullAlpha );

static void _Swizzle_RGB5A3( s32 width, s32 height, void *pImage, BOOL bFullAlpha );
static void _PackTile_RGB5A3( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pResult, BOOL bFullAlpha );

static void _Swizzle_R5G6B5( s32 width, s32 height, void *pImage );
static void _PackTile_R5G6B5( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pResult );

static void _Swizzle_CMPR( s32 width, s32 height, void *pImage );
static void _PackTile_CMPR( s32 width, s32 height, const void *pSource, s32 tileX, s32 tileY, u16 *pResult );

//static void _Swizzle_I8( s32 width, s32 height, void *pImage );
static void _PackTile_I8( s32 width, s32 height, const void *pSource, s32 x, s32 y, s32 bpp, u8 *pResult );

static void _PackTile_IA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr );


//
//
//
void _SwizzleTexData( FTexFmt_e nSrcFmt, void *pImage, u32 w, u32 h )
{
	switch ( nSrcFmt )
	{
		case FTEX_FMT_R8G8B8A8:
			_Swizzle_RGBA8( w, h, pImage, FALSE );
			break;

		case FTEX_FMT_R8G8B8X8:
			_Swizzle_RGBA8( w, h, pImage, TRUE );
			break;

		case FTEX_FMT_R4G4B4A3:
			_Swizzle_RGB5A3( w, h, pImage, FALSE );
			break;

		case FTEX_FMT_R5G5B5X1:
			_Swizzle_RGB5A3( w, h, pImage, TRUE );
			break;

		case FTEX_FMT_R5G6B5:
			_Swizzle_R5G6B5( w, h, pImage );
			break;

		case FTEX_FMT_S3TC:
		case FTEX_FMT_S3TCA1:
		case FTEX_FMT_S3TCx2:
			_Swizzle_CMPR( w, h, pImage );
			break;

		case FTEX_FMT_I8:
			_Swizzle_I8( w, h, pImage );
			break;

		default:
			FASSERT_NOW;
			break;
	}
}


//
//
//
void _Swizzle_RGBA8( s32 width, s32 height, void *pImage, BOOL bFullAlpha )
{
	u32 numTileRows, tileRow;	
	u32 numTileCols, tileCol;
	u8  *pDstPtr;
	
	fang_MemCopy( _pSwizzleBuffer, pImage, width * height * 4);
 	
	// Number of 4x4 texel tile cols, rows including any partial tiles
	numTileCols = ((width  + 3) >> 2);
	numTileRows = ((height + 3) >> 2);
	
	pDstPtr = (u8 *)pImage;
	
	// numTileRows, numTileCols includes any partial tiles
	for( tileRow=0; tileRow<numTileRows; tileRow++ )
	{
		for ( tileCol=0; tileCol<numTileCols; tileCol++)
		{
			_PackTile_RGBA8( width, height, _pSwizzleBuffer, (tileCol * 4), (tileRow * 4), pDstPtr, bFullAlpha );
			
			// Move to next 2 (32B) cache lines
			pDstPtr += 64;
		}	
	}
}


// 
// 4x4 tile, 16-bit texels
// x and y represent starting texel position of this tile
// pack AR in low half, GB in high half dest. buffer 
//
void _PackTile_RGBA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr, BOOL bFullAlpha )
{
	u32 row, col;

	// pDstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if ( realRows > 4 )
		realRows = 4;

	if ( realCols > 4 )
		realCols = 4;
			
	// pack 2 32B tiles
	for ( row = 0; row < realRows; row++ )
	{	
		// Pack 2 cache lines at once move 8 bytes (4 16-bit texels) per row
		// need to reset ptr each row to account for column padding
		u8 *pAR = pDstPtr +      (row * 8);
		u8 *pGB = pDstPtr + 32 + (row * 8);

		for ( col=0; col<realCols; col++ )
		{           
			u32 offset = ((y+row)*width + (x+col))*4;
			const u8* basePtr = (u8 *)pSource;
	
			u8 r = basePtr[offset];
			u8 g = basePtr[offset+1];
			u8 b = basePtr[offset+2];
			u8 a = basePtr[offset+3];

			if ( bFullAlpha )
				a = 255;

			// alpha is byte 0, red is byte 1
			*pAR       = a;	
			*(pAR + 1) = r;

			// green is byte 0, blue is byte 1
			*pGB       = g; 
			*(pGB + 1) = b;
			
			pAR += 2;
			pGB += 2;
		}
	}
}


//
// convert from layer to final hw format
// 4x4 texel tiles @ 8B per row, 32B per tile
//
//
void _Swizzle_RGB5A3( s32 width, s32 height, void *pImage, BOOL bFullAlpha )
{	
	u32 tileRow, tileCol;
 		
	fang_MemCopy( _pSwizzleBuffer, pImage, width * height * 2);
 	
	// number of 4x4 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 3) >> 2);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8 *)(pImage);
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{
		for(tileCol=0; tileCol<numTileCols; tileCol++)
		{			
			_PackTile_RGB5A3( width, height, _pSwizzleBuffer, tileCol*4, tileRow*4, pDstPtr, bFullAlpha );
			pDstPtr += 32;                  // next 32B cache line
		}			
	} 	
}


//
//	4x4 tile, 16-bit texels
//	x and y represent starting texel position of this tile
//
void _PackTile_RGB5A3( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr, BOOL bFullAlpha )
{

	u32 row, col;

	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if ( realRows > 4 )
		realRows = 4;
		
	if ( realCols > 4 )
		realCols = 4;
				     			
	// pack 32B tile 
	for ( row = 0; row < realRows; row++ )
	{
		// Move 8 bytes (4 16-bit texels) per row
		// Need to reset ptr each row to account for column padding
		u16 *pTile = (u16 *)(pDstPtr + (row * 8));

		for ( col=0; col<realCols; col++ )
		{
			const u16 *basePtr = (u16 *)pSource;
			u16 color = basePtr[(y+row)*width + (x+col)];

			// For this format, we want ARGB so convert the source
			// RGBA to ARGB and store it away.
			if ( bFullAlpha )
			{
				*pTile =	0x80						// A
						| (((color>>11) & 0xff) << 10)	// R
						| (((color>> 6) & 0xff) <<  5)	// G
						| (((color>> 1) & 0xff) <<  0);	// B
			}
			else
			{
				*pTile =  (((color>>0) & 0x8f) << 12)	// A
						| (((color>>12) & 0xff) << 8)	// R
						| (((color>>8) & 0xff) <<  4)	// G
						| (((color>> 4) & 0xff) << 0);	// B
			}

			pTile++;
		}
	}
}


//
// convert from layer to final hw format
// 4x4 texel tiles @ 8B per row, 32B per tile
//
//
void _Swizzle_R5G6B5( s32 width, s32 height, void *pImage )
{	
	u32 tileRow, tileCol;
 		
	fang_MemCopy( _pSwizzleBuffer, pImage, width * height * 2);
 	
	// number of 4x4 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 3) >> 2);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8*)(pImage);
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{
		for(tileCol=0; tileCol<numTileCols; tileCol++)
		{			
			_PackTile_R5G6B5( width, height, _pSwizzleBuffer, tileCol*4, tileRow*4, pDstPtr );
			pDstPtr += 32;                  // next 32B cache line
		}			
	} 	
}


//
//	4x4 tile, 16-bit texels
//	x and y represent starting texel position of this tile
//
void _PackTile_R5G6B5( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr )
{

	u32 row, col;

	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if ( realRows > 4 )
		realRows = 4;
		
	if ( realCols > 4 )
		realCols = 4;
				     			
	// pack 32B tile 
	for ( row = 0; row < realRows; row++ )
	{
		// Move 8 bytes (4 16-bit texels) per row
		// Need to reset ptr each row to account for column padding
		u16 *pTile = (u16 *)(pDstPtr + (row * 8));

		for ( col=0; col<realCols; col++ )
		{
			const u16 *basePtr = (u16 *)pSource;
			u16 color = basePtr[(y+row)*width + (x+col)];

			*pTile = color;

			pTile++;
		}
	}
}


//
//
//
void _Swizzle_CMPR( s32 width, s32 height, void *pImage )
{
	u32 tileRow, tileCol;
	u32 srcTileRows, srcTileCols;
	u16* pDstPtr;

	fang_MemCopy( _pSwizzleBuffer, pImage, width * height * 0.5f);
 	
	// each source tile is 4x4 texels, 8B
	srcTileRows   = ((height + 3) >> 2);
	srcTileCols   = ((width  + 3) >> 2);

	pDstPtr = (u16*)(pImage);

	// each dst tile is 2x2 source tiles, so move by 2 each iteration
	for(tileRow = 0; tileRow < srcTileRows; tileRow += 2 )
	{
		for(tileCol = 0; tileCol < srcTileCols; tileCol += 2 )
		{
			_PackTile_CMPR( width, height, _pSwizzleBuffer, tileCol, tileRow, pDstPtr );
			pDstPtr += 16; // 32B per dst tile, short ptr
		}
	}
}


//
//
//
static void _FixCMPWord( u16 *data )
{
	u16 tmp;
	tmp = *data;

	// reverse tuple order within bytes
	*data = ( (tmp & 0x3 )   << 6 ) |
			( (tmp & 0xC )   << 2 ) |
			( (tmp & 0x30)   >> 2 ) |
			( (tmp & 0xC0)   >> 6 ) |

            ( (tmp & 0x300 ) << 6 ) |
			( (tmp & 0xC00 ) << 2 ) |
			( (tmp & 0x3000) >> 2 ) |
			( (tmp & 0xC000) >> 6 ) ;
}


//
//
//
void _PackTile_CMPR( s32 width, s32 height, const void *pSource, s32 tileX, s32 tileY, u16 *pDstPtr )
{
	u32  x, y;
	u16* srcPtr;
	u16  tmp;
	u32  srcTileOffset;
	u32  subTileRows, subRowShorts;    // number of s3 4x4 tiles
	u32  srcPadWidth, srcPadHeight;
	u16* buffPtr;

	// set the padded size of the s3 source image out to a 4-texel boundary
	srcPadWidth  = ( (width  + 3) >> 2 );
	srcPadHeight = ( (height + 3) >> 2 );

	// number of bytes in a single row of 4x4 texel source tiles
	srcTileOffset = srcPadWidth * 8;

	// number of 4x4 (source) tile rows to copy ( will be 1 or 2 )
	subTileRows = 2;
	if( (srcPadHeight - tileY) < 2 )
		subTileRows = 1;

	// number of 4x4 tile cols to copy translated into number of short values
	// ( will be 4 or 8 )
	subRowShorts = 8;
	if( (srcPadWidth - tileX) < 2 )
		subRowShorts = 4;

	for( y=0; y < subTileRows; y++ )
	{
		srcPtr  = (u16*)( (u8*)(pSource) + ((tileY + y) * srcTileOffset) + (tileX*8) ); 
		buffPtr = ( pDstPtr + (y * 8) );        // 16 bytes per subRow = 8 shorts

		// process one or both 4x4 row tiles at once- 4 short each
		for( x=0; x < subRowShorts; x++ )
		{			
			switch( x )
			{

			// color table entries - switch bytes within a 16-bit world only
			case 0:	
			case 1:
			case 4:
			case 5:
				*buffPtr++ = *srcPtr++;
				break;
			
			// 2-bit color tuples;
			// reverse tuple order within bytes of a word
			case 2:
			case 3:
			case 6:
			case 7:
				tmp = *srcPtr++;
				_FixCMPWord( &tmp );
				*buffPtr++ = tmp;
				break;

			}
		}
	}
}

void _Swizzle_IA8( s32 width, s32 height, void *pImage )
{
	u32 tileRow, tileCol;
	
	FMemFrame_t Frame = fmem_GetFrame();
	
	_pSwizzleBuffer = (u8*)fmem_Alloc( width*height*2, 32 );

	fang_MemCopy( _pSwizzleBuffer, pImage, width * height * 2 );
 	
	// Number of 4x4 texel tile cols, rows including any partial tiles
	u32 numTileCols = ((width  + 3) >> 2);
	u32 numTileRows = ((height + 3) >> 2);
	
	u8 *pDstPtr = (u8*)pImage;
	
	// numTileRows, numTileCols includes any partial tiles
	for ( tileRow=0; tileRow<numTileRows; tileRow++ )
	{		
		for (tileCol=0; tileCol<numTileCols; tileCol++ )
		{						
			_PackTile_IA8( width, height, _pSwizzleBuffer, tileCol*4, tileRow*4, pDstPtr );// width, height, pSource, tileCol*4, tileRow*4, pDstPtr );
			pDstPtr += 32;                  // next 32B cache line
		}			
	}
	
	fmem_ReleaseFrame( Frame );
}


//
//
//
static void _PackTile_IA8( s32 width, s32 height, const void *pSource, s32 x, s32 y, u8 *pDstPtr )
{
	u32 row, col;
	u32 realRows, realCols;
	u16* pTile;
	u8* pData;
	

	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	realRows = height - y;
	realCols = width  - x;
	
	if( realRows > 4)    
		realRows = 4;
		
	if(realCols > 4)
		realCols = 4;
			
	// pack 32B tile 
	for(row=0; row<realRows; row++)
	{	
		pTile = (u16*)(pDstPtr + (row * 8));                       // move 8 bytes (4 16-bit texels) per row
				                                                    // need to reset ptr each row to account for
						                                            // column padding
		for(col=0; col<realCols; col++)
		{
			u32 offset = ((y+row)*width + (x+col))*2;
			const u8* basePtr = (u8 *)pSource;
	
			u8 A = basePtr[offset+0];
			u8 I = basePtr[offset+1];

			pData = (u8*)pTile;

			pData[0] = A;
			pData[1] = I;
			pTile++;
		} // end for col loop			
	} // end for row loop
}

//
//
//
void _Swizzle_I8( s32 width, s32 height, void *pImage )
{
	u32 tileRow, tileCol;
	
	FMemFrame_t Frame = fmem_GetFrame();
	
	_pSwizzleBuffer = (u8*)fmem_Alloc( 256*256, 32 );
	
	if (_pSwizzleBuffer)
	{
		
		fang_MemCopy( _pSwizzleBuffer, pImage, width * height );
	 	
		// Number of 4x8 texel tile cols, rows including any partial tiles
		u32 numTileCols = ((width  + 7) >> 3);
		u32 numTileRows = ((height + 3) >> 2);
		
		u8 *pDstPtr = (u8*)pImage;
		
		// numTileRows, numTileCols includes any partial tiles
		for ( tileRow=0; tileRow<numTileRows; tileRow++ )
		{		
			for (tileCol=0; tileCol<numTileCols; tileCol++ )
			{						
				_PackTile_I8( width, height, _pSwizzleBuffer, (tileCol * 8), (tileRow * 4), 1, pDstPtr );
				pDstPtr += 32;                  // next 32B cache line
			}			
		}
	}
	
	fmem_ReleaseFrame( Frame );
}

void ftex_SetTexAddress( u32 nStageNum, BOOL bWrapU, BOOL bWrapV, BOOL bWrapW )
{
}


//
//
//
static void _PackTile_I8( s32 width, s32 height, const void *pSource, s32 x, s32 y, s32 bpp, u8 *pDstPtr )
{
	u32 row, col;
    
	// dstPtr is already zeroed out, so this will take care of padding issue
	// 'realRows', 'realCols' represent actual source image texels remaining
	u32 realRows = height - y;
	u32 realCols = width  - x;
	
	if( realRows > 4)    
		realRows = 4;
		
	if(realCols > 8)
		realCols = 8;
		
	// pack 32B tile 
	for(row=0; row<realRows; row++)
	{	
		// Move 8 bytes (8 8-bit texels) per row.  Need 
		// to reset ptr each row to account for column padding
		u8 *pTile = pDstPtr + (row * 8);						

		for(col=0; col<realCols; col++)
		{
			u32 offset = ((y+row)*width + (x+col))*bpp;
			const u8 *basePtr = (u8 *)pSource;

			*pTile = basePtr[offset];
			pTile++;
		}				
	}				
}

//FULLSCREEN
#if 1

//
//
//
void _RenderTexture_ViewportCallback( FViewport_t *pView )
{
	FTexDef_t *pTexDef = _RTpTexDef;
	
	_pActiveViewport = pView;
	
	switch (_pActiveViewport->nType)
	{
		case FVIEWPORT_TYPE_PERSPECTIVE:
			fviewport_InitPersp( _TexViewport, pView->fHalfFOVX, pView->fHalfFOVY, pView->fNearZ, pView->fFarZ, _nX, _nY, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
		case FVIEWPORT_TYPE_ORTHO2D:
			fviewport_InitOrtho2D( _TexViewport, pView->fNearZ, pView->fFarZ, _nX, _nY, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
        case FVIEWPORT_TYPE_ORTHO3D:
			fviewport_InitOrtho3D( _TexViewport, pView->fNearZ, pView->fFarZ, _nX, _nY, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
	};
	
	fviewport_SetCallback(NULL);
	fviewport_SetActive(_TexViewport);
	fviewport_SetCallback(_RenderTexture_ViewportCallback);
}

BOOL CFTexInst::BeginRender_FullScreen()
{
	// Get Current RenderTarget and Save (will restore later).
	FTexDef_t *pTexDef = GetTexDef();
	_RTpTexDef = pTexDef;
	
	if (!pTexDef)
	{
		return (FALSE);
	}
	
	_pActiveViewport = fviewport_GetActive_Absolute();
	if( !_pActiveViewport ) {
		// _pActiveViewport can sometimes be NULL between levels - Starich
		return FALSE;
	}
	_nX=0; _nY=0;

	switch (_pActiveViewport->nType)
	{
		case FVIEWPORT_TYPE_PERSPECTIVE:
			fviewport_InitPersp( _TexViewport, _pActiveViewport->fHalfFOVX, _pActiveViewport->fHalfFOVY, _pActiveViewport->fNearZ, _pActiveViewport->fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
		case FVIEWPORT_TYPE_ORTHO2D:
			fviewport_InitOrtho2D( _TexViewport, _pActiveViewport->fNearZ, _pActiveViewport->fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
        case FVIEWPORT_TYPE_ORTHO3D:
			fviewport_InitOrtho3D( _TexViewport, _pActiveViewport->fNearZ, _pActiveViewport->fFarZ, 0, 0, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
	};

	fviewport_SetCallback(NULL);
	fviewport_SetActive(_TexViewport);
	fviewport_SetCallback(_RenderTexture_ViewportCallback);
	
	return (TRUE);
}

BOOL _bUseIntens=FALSE;

void CFTexInst::EndRender_FullScreen(BOOL bAutoClearRGBA, BOOL bAutoClearDepth)
{
	FTexDef_t *pTexDef = GetTexDef();
	_RTpTexDef = pTexDef;

	GXColor gxWhite={0xff, 0xff, 0xff, 0xff};
	GXTexObj *pTexObj = pTexDef->pTexData->pGCTexObj;
	//Restore RenderTarget to BackBuffer
	
	GXTexFmt fmt = GXGetTexObjFmt(pTexObj);
	if ( !_bUseIntens )
	{
		if (fmt == GX_TF_I8) { fmt = GX_CTF_R8; }
		if (fmt == GX_TF_I4) { fmt = GX_CTF_R4; }
	}
	
	GXSetTexCopySrc(0, 0, pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown);
	GXSetTexCopyDst(pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, fmt, FALSE);
		
	GXSetCopyClear(gxWhite, GX_MAX_Z24);
	GXCopyTex(GXGetTexObjData(pTexObj), GX_TRUE);
	
	fviewport_SetCallback(NULL);
	fviewport_SetActive(_pActiveViewport);
}

//SIDEBAR
BOOL CFTexInst::BeginRender()
{
	FTexDef_t *pTexDef = GetTexDef();
	_RTpTexDef = pTexDef;
	
	if (!pTexDef)
	{
		return (FALSE);
	}
	
	_nX = 640;//-160;
	_nY = (128*_nCurrentSlot);

	_pActiveViewport = fviewport_GetActive_Absolute();

	switch (_pActiveViewport->nType)
	{
		case FVIEWPORT_TYPE_PERSPECTIVE:
			fviewport_InitPersp( _TexViewport, _pActiveViewport->fHalfFOVX, _pActiveViewport->fHalfFOVY, _pActiveViewport->fNearZ, _pActiveViewport->fFarZ, _nX, _nY, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
		case FVIEWPORT_TYPE_ORTHO2D:
			fviewport_InitOrtho2D( _TexViewport, _pActiveViewport->fNearZ, _pActiveViewport->fFarZ, _nX, _nY, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
        case FVIEWPORT_TYPE_ORTHO3D:
			fviewport_InitOrtho3D( _TexViewport, _pActiveViewport->fNearZ, _pActiveViewport->fFarZ, _nX, _nY, pTexDef->TexInfo.nTexelsAcross*_fHorizScale, pTexDef->TexInfo.nTexelsDown );
			break;
	};

	fviewport_SetCallback(NULL);
	fviewport_SetActive(_TexViewport);
	fviewport_SetCallback(_RenderTexture_ViewportCallback);
	
	return (TRUE);
}

void CFTexInst::EndRender(BOOL bAutoClearRGBA, BOOL bAutoClearDepth)
{
	FTexDef_t *pTexDef = GetTexDef();
	_RTpTexDef = pTexDef;

	GXColor gxWhite={0xff, 0xff, 0xff, 0xff};
	GXTexObj *pTexObj = pTexDef->pTexData->pGCTexObj;
	GXSetTexCopySrc(512, (_nCurrentSlot*128), pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown);
	GXSetTexCopyDst(pTexDef->TexInfo.nTexelsAcross, pTexDef->TexInfo.nTexelsDown, GXGetTexObjFmt(pTexObj), FALSE);
	
	GXSetCopyClear(gxWhite, GX_MAX_Z24);
	GXCopyTex(GXGetTexObjData(pTexObj), GX_FALSE);

	fviewport_SetCallback(NULL);
	fviewport_SetActive(_pActiveViewport);
}

void ftex_FlushRenderTarget(CFTexInst *pTexInst)
{
}

BOOL ftex_AddRenderTarget(CFTexInst *pTexInst, ftex_RenderTarget_Callback pCallback, BOOL bRenderWorld, u32 nMaxRenderFreq, BOOL bAutoClearColor, BOOL bAutoClearDepth, void *pUserData, BOOL bAllowLiquid, BOOL bForceNoLag)
{
	BOOL ret=FALSE;
	u32 n = _nNumRenderTargets;
	if (_nNumRenderTargets < _MAX_RENDERTARGETS && pTexInst)
	{
		ret = TRUE;
		_nNumRenderTargets++;

		_aRenderTargets[n].pTexInst = pTexInst;
		_aRenderTargets[n].pAlphaTex = NULL;
		_aRenderTargets[n].pCallback = pCallback;
		_aRenderTargets[n].bRenderWorld = bRenderWorld;
		if (nMaxRenderFreq > 0)
		{
			_aRenderTargets[n].fOOMaxRenderFreq = 1.0f/(f32)nMaxRenderFreq;
		}
		else
		{
			_aRenderTargets[n].fOOMaxRenderFreq = 0.0f;
		}

		_aRenderTargets[n].bAutoClearRGBA = bAutoClearColor;
		_aRenderTargets[n].bAutoClearDepth = bAutoClearDepth;
		
		_aRenderTargets[n].bAllowLiquidRender = bAllowLiquid;

		_aRenderTargets[n].fCurrentRenderTime = 0.0f;
		_aRenderTargets[n].bStagger = FALSE;
		
		_aRenderTargets[n].bActive = TRUE;
		_aRenderTargets[n].pUserData = pUserData;
	}
	return (ret);
}

void ftex_StaggerRenderTarget()
{
	_aRenderTargets[_nNumRenderTargets-1].bStagger = TRUE;
}

void ftex_ActivateRenderTarget(CFTexInst *pTexInst, BOOL bActive)
{
	_RenderTarget *pTarget;
	u32 i;
	for (i=0; i<_nNumRenderTargets; i++)
	{
		pTarget = &_aRenderTargets[i];
		
		if (pTarget->pTexInst == pTexInst)
		{
			if (pTarget->bActive != bActive)
			{
				pTarget->bActive = bActive;
				if (bActive == TRUE)
				{
					ftex_FlushRenderTarget(pTexInst);
					pTarget->fCurrentRenderTime = 0.0f;
				}
			}
			return;
		}
	}
}

void ftex_SetUserData(CFTexInst *pTexInst, void *pUserData)
{
	_RenderTarget *pTarget;
	u32 i;
	for (i=0; i<_nNumRenderTargets; i++)
	{
		pTarget = &_aRenderTargets[i];
		
		if (pTarget->pTexInst == pTexInst)
		{
			pTarget->pUserData = pUserData;
			return;
		}
	}
}

void ftex_RenderTarget_SetAlphaTex( CFTexInst *pTexInst, CFTexInst *pAlphaTex )
{
	_RenderTarget *pTarget;
	u32 i;
	for (i=0; i<_nNumRenderTargets; i++)
	{
		pTarget = &_aRenderTargets[i];
		
		if (pTarget->pTexInst == pTexInst)
		{
			pTarget->pAlphaTex = pAlphaTex;
			
			FTexDef_t *pTexDefLayer = pTarget->pTexInst->GetTexDef();
			GXTexObj *pGCTexObjLayer;
			
			FTexDef_t *pAlphaTexDefLayer = pAlphaTex->GetTexDef();
			GXTexObj *pGCAlphaTexObjLayer;
			
			if (pTexDefLayer && pAlphaTexDefLayer)
			{
				if (pAlphaTexDefLayer->pTexData)
				{
					pGCAlphaTexObjLayer = (GXTexObj *)pAlphaTexDefLayer->pTexData->pGCTexObj;
				}
				
				if (pTexDefLayer->pTexData && pGCAlphaTexObjLayer)
				{
					pGCTexObjLayer = (GXTexObj *)pTexDefLayer->pTexData->pGCTexObj;
					
					if (pGCTexObjLayer)
					{
						GXInitTexObjUserData( pGCTexObjLayer, pGCAlphaTexObjLayer );
					}
				}
			}
			
			return;
		}
	}	
}

extern GXRenderModeObj *FGCVid_pRenderMode;

BOOL ftex_HandleRenderTargets()
{
	if (_nNumRenderTargets < 1) return TRUE;
	
	_bUseIntens = FALSE;
	
	u32 i;
	_bRenderWorld=TRUE;
	_RenderTarget *pTarget;
	BOOL bSlotFree=TRUE, bRTActive=FALSE;
	BOOL bFullScr=FALSE;
	BOOL bStaggerRender=FALSE;
	
	FVis_bInRenderTarget = TRUE;
	
	_nCurrentSlot = 0;
	
	_pActiveViewport = fviewport_GetActive_Absolute();

	if (!_TexViewport)
	{
		_TexViewport = fviewport_Create();
	}
	
	if ( fgctex_fCopyMul != 1.0f )
	{
		u8 nNewFilter[7];
		u32 i;
		for ( i = 0; i < 7; ++i )
		{
			nNewFilter[i] = u8( FGCVid_pRenderMode->vfilter[i] * fgctex_fCopyMul );
		}
		
		GXSetCopyFilter( FGCVid_pRenderMode->aa, FGCVid_pRenderMode->sample_pattern, GX_TRUE, nNewFilter );
	}
	
	for (i=0; i<_nNumRenderTargets; i++)
	{
		pTarget = &_aRenderTargets[i];
		if (pTarget->bActive)
		{
			if (pTarget->fCurrentRenderTime <= 0)// || (pTarget->pTexInst->GetFlags() & CFTexInst::FLAG_FULLSCREEN))
			{
				bRTActive = TRUE;
				if (pTarget->pTexInst->GetFlags() & CFTexInst::FLAG_FULLSCREEN)
				{
					fviewport_SetCallback(_RenderTexture_ViewportCallback);

					if (pTarget->pTexInst->BeginRender_FullScreen())
					{
						fvis_ToggleLiquidInRenderTarget( pTarget->bAllowLiquidRender );
						pTarget->pCallback(pTarget->pUserData);
						fvis_ToggleLiquidInRenderTarget( FALSE );
						if (pTarget->pAlphaTex)
						{
							_bUseIntens = TRUE;
							pTarget->pTexInst->EndRender_FullScreen(FALSE, FALSE);
							pTarget->pAlphaTex->EndRender_FullScreen(pTarget->bAutoClearRGBA, pTarget->bAutoClearDepth);
							_bUseIntens = FALSE;
						}
						else
						{
							pTarget->pTexInst->EndRender_FullScreen(pTarget->bAutoClearRGBA, pTarget->bAutoClearDepth);
						}
					}

					bSlotFree = TRUE;
					bFullScr = TRUE;
				}
				else
				{
					if (_nCurrentSlot < 4)
					{
						if ( pTarget->bStagger && !bStaggerRender )
						{
							bStaggerRender = TRUE;
						}
						else if ( pTarget->bStagger && bStaggerRender )
						{
							continue;
						}
						
						fviewport_SetCallback(_RenderTexture_ViewportCallback);

						if (pTarget->pTexInst->BeginRender())
						{
							fvis_ToggleLiquidInRenderTarget( pTarget->bAllowLiquidRender );
							pTarget->pCallback(pTarget->pUserData);
							fvis_ToggleLiquidInRenderTarget( FALSE );
							if (pTarget->pAlphaTex)
							{
								_bUseIntens = TRUE;
								pTarget->pTexInst->EndRender(FALSE, FALSE);
								pTarget->pAlphaTex->EndRender(pTarget->bAutoClearRGBA, pTarget->bAutoClearDepth);
								_bUseIntens = FALSE;
							}
							else
							{
								pTarget->pTexInst->EndRender(pTarget->bAutoClearRGBA, pTarget->bAutoClearDepth);
							}
						}

						_nCurrentSlot++;
						
						bSlotFree = TRUE;
					}
					else
					{
						bSlotFree = FALSE;
					}
				}

				if (bSlotFree)
				{
					if (!pTarget->bRenderWorld) _bRenderWorld=FALSE;
					pTarget->fCurrentRenderTime = pTarget->fOOMaxRenderFreq;
				}
			}
			else
			{
				pTarget->fCurrentRenderTime-=FLoop_fPreviousLoopSecs;
				if (pTarget->fCurrentRenderTime < 0.0f) pTarget->fCurrentRenderTime = 0.0f;
			}
		}
	}
	
	if (_nNumRenderTargets > 0 && bRTActive)
	{
		if (bFullScr)
		{
			GXPixModeSync();
		}
		
		// Restore vertical de-flicker filter mode
		if ( fgctex_fCopyMul != 1.0f )
		{
	    	GXSetCopyFilter( FGCVid_pRenderMode->aa, FGCVid_pRenderMode->sample_pattern, GX_TRUE, FGCVid_pRenderMode->vfilter );
	    }

		fviewport_SetCallback(NULL);		
		fviewport_SetActive(_pActiveViewport);
		fviewport_Clear(FVIEWPORT_CLEARFLAG_COLOR | FVIEWPORT_CLEARFLAG_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 0);
	}
	
	FVis_bInRenderTarget = FALSE;
	
	GXInvalidateTexAll();
	
	return (_bRenderWorld);
}

BOOL ftex_GetRenderWorldFlag() 
{ 
	return (_bRenderWorld); 
}

void ftex_ClearRenderTargets()
{
	if ( _nNumRenderTargets )
	{
		_nNumRenderTargets = 0;
		_bRenderWorld=TRUE;
		_TexViewport = NULL;
		
		fsh_ClearSReflectTargets();
		
		fsh_ResetFullScrRenderTarget();
	}
}
#endif
//End RenderTarget API
